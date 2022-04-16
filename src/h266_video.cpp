/*

MIT License

Copyright (c) 2021 Ravin.Wang(wangf1978@hotmail.com)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/
#include "platcomm.h"
#include "h266_video.h"
#include "AMException.h"
#include "AMRingBuffer.h"

#include "NAL.h"
#include "DataUtil.h"
#include "nal_com.h"

const char* vvc_profile_name[6] = {
	"Main 10",
	"Multilayer Main 10",
	"Main 10 4:4:4",
	"Multilayer Main 10 4:4:4",
	"Main 10 Still Picture",
	"Main 10 4:4:4 Still Picture"
};

const char* vvc_nal_unit_type_names[64] = {
	/*00*/ "VCL::TRAIL_N",
	/*01*/ "VCL::STSA_NUT",
	/*02*/ "VCL::RADL_NUT",
	/*03*/ "VCL::RASL_NUT",
	/*04*/ "VCL::RSV_VCL_4",
	/*05*/ "VCL::RSV_VCL_5",
	/*06*/ "VCL::RSV_VCL_6",
	/*07*/ "VCL::IDR_W_RADL",
	/*08*/ "VCL::IDR_N_LP",
	/*09*/ "VCL::CRA_NUT",
	/*10*/ "VCL::GDR_NUT",
	/*11*/ "VCL::RSV_IRAP_11",
	/*12*/ "non-VCL::OPI_NUT",
	/*13*/ "non-VCL::DCI_NUT",
	/*14*/ "non-VCL::VPS_NUT",
	/*15*/ "non-VCL::SPS_NUT",
	/*16*/ "non-VCL::PPS_NUT",
	/*17*/ "non-VCL::PREFIX_APS_NUT",
	/*18*/ "non-VCL::SUFFIX_APS_NUT",
	/*19*/ "non-VCL::PH_NUT",
	/*20*/ "non-VCL::AUD_NUT",
	/*21*/ "non-VCL::EOS_NUT",
	/*22*/ "non-VCL::EOB_NUT",
	/*23*/ "non-VCL::PREFIX_SEI_NUT",
	/*24*/ "non-VCL::SUFFIX_SEI_NUT",
	/*25*/ "non-VCL::FD_NUT",
	/*26*/ "non-VCL::RSV_NVCL_26",
	/*27*/ "non-VCL::RSV_NVCL_27",
	/*28*/ "non-VCL::UNSPEC_28",
	/*29*/ "non-VCL::UNSPEC_29",
	/*30*/ "non-VCL::UNSPEC_30",
	/*31*/ "non-VCL::UNSPEC_31",
};

const char* vvc_nal_unit_type_short_names[32] = {
	/*00*/ "TRAIL_N",
	/*01*/ "STSA_NUT",
	/*02*/ "RADL_NUT",
	/*03*/ "RASL_NUT",
	/*04*/ "RSV_VCL_4",
	/*05*/ "RSV_VCL_5",
	/*06*/ "RSV_VCL_6",
	/*07*/ "IDR_W_RADL",
	/*08*/ "IDR_N_LP",
	/*09*/ "CRA_NUT",
	/*10*/ "GDR_NUT",
	/*11*/ "RSV_IRAP_11",
	/*12*/ "OPI_NUT",
	/*13*/ "DCI_NUT",
	/*14*/ "VPS_NUT",
	/*15*/ "SPS_NUT",
	/*16*/ "PPS_NUT",
	/*17*/ "PREFIX_APS_NUT",
	/*18*/ "SUFFIX_APS_NUT",
	/*19*/ "PH_NUT",
	/*20*/ "AUD_NUT",
	/*21*/ "EOS_NUT",
	/*22*/ "EOB_NUT",
	/*23*/ "PREFIX_SEI_NUT",
	/*24*/ "SUFFIX_SEI_NUT",
	/*25*/ "FD_NUT",
	/*26*/ "RSV_NVCL_26",
	/*27*/ "RSV_NVCL_27",
	/*28*/ "UNSPEC_28",
	/*29*/ "UNSPEC_29",
	/*30*/ "UNSPEC_30",
	/*31*/ "UNSPEC_31",
};

const char* vvc_nal_unit_type_descs[32] = {
	/*00*/ "Coded slice of a trailing picture or subpicture slice_layer_rbsp()",
	/*01*/ "Coded slice of an STSA picture or subpicture slice_layer_rbsp()",
	/*02*/ "Coded slice of a RADL picture or subpicture slice_layer_rbsp()",
	/*03*/ "Coded slice of a RASL picture or subpicture slice_layer_rbsp()",
	/*04*/ "Reserved non-IRAP VCL NAL unit types",
	/*05*/ "Reserved non-IRAP VCL NAL unit types",
	/*06*/ "Reserved non-IRAP VCL NAL unit types",
	/*07*/ "Coded slice of an IDR picture or subpicture* slice_layer_rbsp()",
	/*08*/ "Coded slice of an IDR picture or subpicture* slice_layer_rbsp()",
	/*09*/ "Coded slice of a CRA picture or subpicture slice_layer_rbsp()",
	/*10*/ "Coded slice of a GDR picture or subpicture slice_layer_rbsp()",
	/*11*/ "Reserved IRAP VCL NAL unit type",
	/*12*/ "Operating point information operating_point_information_rbsp()",
	/*13*/ "Decoding capability information decoding_capability_information_rbsp()",
	/*14*/ "Video parameter set video_parameter_set_rbsp()",
	/*15*/ "Sequence parameter set seq_parameter_set_rbsp()",
	/*16*/ "Picture parameter set pic_parameter_set_rbsp()",
	/*17*/ "Adaptation parameter set adaptation_parameter_set_rbsp()",
	/*18*/ "Adaptation parameter set adaptation_parameter_set_rbsp()",
	/*19*/ "Picture header picture_header_rbsp()",
	/*20*/ "AU delimiter access_unit_delimiter_rbsp()",
	/*21*/ "End of sequence end_of_seq_rbsp()",
	/*22*/ "End of bitstream end_of_bitstream_rbsp()",
	/*23*/ "Supplemental enhancement information sei_rbsp()",
	/*24*/ "Supplemental enhancement information sei_rbsp()",
	/*25*/ "Filler data filler_data_rbsp()",
	/*26*/ "Reserved non-VCL NAL unit types",
	/*27*/ "Reserved non-VCL NAL unit types",
	/*28*/ "Unspecified non-VCL NAL unit types",
	/*29*/ "Unspecified non-VCL NAL unit types",
	/*30*/ "Unspecified non-VCL NAL unit types",
	/*31*/ "Unspecified non-VCL NAL unit types",
};

namespace BST
{
	namespace H266Video
	{
		int NAL_UNIT::VIDEO_PARAMETER_SET_RBSP::UpdateReferenceLayers()
		{
			uint8_t dependencyFlag[64][64] = { {0} };
			for (uint16_t i = 0; i <= vps_max_layers_minus1; i++) {
				for (uint16_t j = 0; j <= vps_max_layers_minus1; j++) {
					dependencyFlag[i][j] = vps_direct_ref_layer_flag[i][j];
					for (uint16_t k = 0; k < i; k++)
						if (vps_direct_ref_layer_flag[i][k] && dependencyFlag[k][j])
							dependencyFlag[i][j] = 1;
				}
				LayerUsedAsRefLayerFlag[i] = 0;
			}
			for (uint16_t i = 0; i <= vps_max_layers_minus1; i++) {
				uint16_t j = 0, d = 0, r = 0;
				for (j = 0, d = 0, r = 0; j <= vps_max_layers_minus1; j++) {
					if (vps_direct_ref_layer_flag[i][j]) {
						DirectRefLayerIdx[i][d++] = (uint8_t)j;
						LayerUsedAsRefLayerFlag[j] = 1;
					}
					if (dependencyFlag[i][j])
						ReferenceLayerIdx[i][r++] = (uint8_t)j;
				}
				NumDirectRefLayers[i] = (uint8_t)d;
				NumRefLayers[i] = (uint8_t)r;
			}
			return RET_CODE_SUCCESS;
		}

		int NAL_UNIT::VIDEO_PARAMETER_SET_RBSP::UpdateOLSInfo()
		{
			if (!vps_each_layer_is_an_ols_flag)
				olsModeIdc = vps_ols_mode_idc;
			else
				olsModeIdc = 4;

			if (olsModeIdc == 4 || olsModeIdc == 0 || olsModeIdc == 1)
				TotalNumOlss = vps_max_layers_minus1 + 1;				// maximum value: 64
			else if (olsModeIdc == 2)
				TotalNumOlss = vps_num_output_layer_sets_minus2 + 2;	// maximum value: 255 + 2

			std::vector<std::vector<bool>> layerIncludedInOlsFlag;

			NumOutputLayersInOls.resize(TotalNumOlss);
			OutputLayerIdInOls.resize(TotalNumOlss);
			NumSubLayersInLayerInOLS.resize(TotalNumOlss);
			layerIncludedInOlsFlag.resize(TotalNumOlss);

			NumOutputLayersInOls[0] = 1;
			OutputLayerIdInOls[0][0] = vps_layer_id[0];
			NumSubLayersInLayerInOLS[0][0] = vps_ptl_max_tid[vps_ols_ptl_idx[0]] + 1;
			LayerUsedAsOutputLayerFlag.BitSet(0);
			for (int i = 1; i <= (int)vps_max_layers_minus1; i++) {
				if (olsModeIdc == 4 || olsModeIdc < 2)
					LayerUsedAsOutputLayerFlag.BitSet(i);
				else if (vps_ols_mode_idc == 2)
					LayerUsedAsOutputLayerFlag.BitClear(i);
			}
			for (int i = 1; i < (int)TotalNumOlss; i++) {
				if (olsModeIdc == 4 || olsModeIdc == 0) {
					NumOutputLayersInOls[i] = 1;
					OutputLayerIdInOls[i][0] = vps_layer_id[i];
					if (vps_each_layer_is_an_ols_flag)
						NumSubLayersInLayerInOLS[i][0] = vps_ptl_max_tid[vps_ols_ptl_idx[i]] + 1;
					else
					{
						NumSubLayersInLayerInOLS[i][i] = vps_ptl_max_tid[vps_ols_ptl_idx[i]] + 1;
						for (int k = i - 1; k >= 0; k--) {
							NumSubLayersInLayerInOLS[i][k] = 0;
							for (int m = k + 1; m <= i; m++) {
								uint16_t maxSublayerNeeded = AMP_MIN(NumSubLayersInLayerInOLS[i][m], vps_max_tid_il_ref_pics_plus1[m][k]);
								if (vps_direct_ref_layer_flag[m][k] && NumSubLayersInLayerInOLS[i][k] < maxSublayerNeeded)
									NumSubLayersInLayerInOLS[i][k] = maxSublayerNeeded;
							}
						}
					}
				}
				else if (vps_ols_mode_idc == 1) {
					NumOutputLayersInOls[i] = i + 1;
					for (int j = 0; j < (int)NumOutputLayersInOls[i]; j++) {
						OutputLayerIdInOls[i][j] = vps_layer_id[j];
						NumSubLayersInLayerInOLS[i][j] = vps_ptl_max_tid[vps_ols_ptl_idx[i]] + 1;
					}
				}
				else if (vps_ols_mode_idc == 2) {

					layerIncludedInOlsFlag[i].resize(vps_max_layers_minus1 + 1);
					for (int j = 0; j <= vps_max_layers_minus1; j++) {
						layerIncludedInOlsFlag[i][j] = 0;
						NumSubLayersInLayerInOLS[i][j] = 0;
					}
					uint16_t highestIncludedLayer = 0;
					uint8_t OutputLayerIdx[64] = { 0 };
					uint16_t k, j;
					for (k = 0, j = 0; k <= vps_max_layers_minus1; k++) {
						if (vps_ols_output_layer_flag[i][k]) {
							layerIncludedInOlsFlag[i][k] = 1;
							highestIncludedLayer = k;
							LayerUsedAsOutputLayerFlag.BitSet(k);
							OutputLayerIdx[j] = (uint8_t)k;
							OutputLayerIdInOls[i][j++] = vps_layer_id[k];
							NumSubLayersInLayerInOLS[i][k] = vps_ptl_max_tid[vps_ols_ptl_idx[i]] + 1;
						}
					}
					NumOutputLayersInOls[i] = j;
					for (j = 0; j < NumOutputLayersInOls[i]; j++) {
						uint8_t idx = OutputLayerIdx[j];
						for (k = 0; k < NumRefLayers[idx]; k++) {
							if (!layerIncludedInOlsFlag[i][ReferenceLayerIdx[idx][k]])
								layerIncludedInOlsFlag[i][ReferenceLayerIdx[idx][k]] = 1;
						}
					}
					for (int k = highestIncludedLayer - 1; k >= 0; k--)
					{
						if (layerIncludedInOlsFlag[i][k] && !vps_ols_output_layer_flag[i][k])
						{
							for (uint16_t m = k + 1; m <= highestIncludedLayer; m++) {
								uint16_t maxSublayerNeeded = AMP_MIN(NumSubLayersInLayerInOLS[i][m], vps_max_tid_il_ref_pics_plus1[m][k]);
								if (vps_direct_ref_layer_flag[m][k] && layerIncludedInOlsFlag[i][m] && NumSubLayersInLayerInOLS[i][k] < maxSublayerNeeded)
									NumSubLayersInLayerInOLS[i][k] = maxSublayerNeeded;
							}
						}
					}
				}
			}

			NumLayersInOls.resize(TotalNumOlss);
			LayerIdInOls.resize(TotalNumOlss);
			MultiLayerOlsIdx.resize(TotalNumOlss);
			NumLayersInOls[0] = 1;
			for (int i = 1; i < TotalNumOlss; i++) {
				if (vps_each_layer_is_an_ols_flag) {
					NumLayersInOls[i] = 1;
					LayerIdInOls[i][0] = vps_layer_id[i];
				}
				else if (vps_ols_mode_idc == 0 || vps_ols_mode_idc == 1) {
					NumLayersInOls[i] = i + 1;
					for (int j = 0; j < NumLayersInOls[i]; j++)
						LayerIdInOls[i][j] = vps_layer_id[j];
				}
				else if (vps_ols_mode_idc == 2) {
					for (int k = 0, j = 0; k <= vps_max_layers_minus1; k++) {
						if (layerIncludedInOlsFlag[i][k])
							LayerIdInOls[i][j++] = vps_layer_id[k];
						NumLayersInOls[i] = j;
					}

				}
				if (NumLayersInOls[i] > 1) {
					MultiLayerOlsIdx[i] = NumMultiLayerOlss;
					NumMultiLayerOlss++;
				}
			}
			
			return RET_CODE_SUCCESS;
		}

		int NAL_UNIT::REF_PIC_LIST_STRUCT::Map(AMBst in_bst)
		{
			int iRet = RET_CODE_SUCCESS;
			SYNTAX_BITSTREAM_MAP::Map(in_bst);

			try
			{
				int idx = 0;
				MAP_BST_BEGIN(0);
				nal_read_ue(in_bst, num_ref_entries, uint8_t);
				if (sps_long_term_ref_pics_flag &&
					m_rplsIdx < current_sps_num_ref_pic_list && num_ref_entries > 0)
					bsrb1(in_bst, ltrp_in_header_flag, 1);
				abs_delta_poc_st.resize(num_ref_entries);
				ilrp_idx.resize(num_ref_entries);
				for (uint16_t i = 0, j = 0; i < num_ref_entries; i++)
				{
					if (sps_inter_layer_prediction_enabled_flag) {
						bsrbarray(in_bst, inter_layer_ref_pic_flag, i);
					}
					else
						inter_layer_ref_pic_flag.BitClear(i);

					if (!inter_layer_ref_pic_flag[i]) {
						if (sps_long_term_ref_pics_flag) {
							bsrbarray(in_bst, st_ref_pic_flag, i);
						}
						else
							st_ref_pic_flag.BitSet(i);

						if (st_ref_pic_flag[i]) {
							nal_read_ue(in_bst, abs_delta_poc_st[i], uint16_t);
							uint32_t AbsDeltaPocSt = (uint32_t)abs_delta_poc_st[i] + 1;
							if ((sps_weighted_pred_flag || sps_weighted_bipred_flag) && i != 0)
								AbsDeltaPocSt = (uint32_t)abs_delta_poc_st[i];

							if (AbsDeltaPocSt > 0) {
								bsrbarray(in_bst, strp_entry_sign_flag, i);
							}
							else
								strp_entry_sign_flag.BitClear(i);
						}
						else if(!ltrp_in_header_flag)
						{
							uint16_t rpls_poc_lsb_lt_value;
							bsrb1(in_bst, rpls_poc_lsb_lt_value, sps_log2_max_pic_order_cnt_lsb_minus4 + 4);
							rpls_poc_lsb_lt[j++] = rpls_poc_lsb_lt_value;
						}
						else
						{
							nal_read_ue(in_bst, ilrp_idx[i], uint16_t);
						}
					}
				}

				MAP_BST_END();
			}
			catch (AMException e)
			{
				return e.RetCode();
			}

			return RET_CODE_SUCCESS;
		}

		int NAL_UNIT::VIDEO_PARAMETER_SET_RBSP::Map(AMBst in_bst)
		{
			int iRet = RET_CODE_SUCCESS;
			SYNTAX_BITSTREAM_MAP::Map(in_bst);

			try
			{
				int idx = 0;
				MAP_BST_BEGIN(0);
				bsrb1(in_bst, vps_video_parameter_set_id, 4);
				bsrb1(in_bst, vps_max_layers_minus1, 6);
				bsrb1(in_bst, vps_max_sublayers_minus1, 3);

				if (vps_max_layers_minus1 > 0 && vps_max_sublayers_minus1 > 0) {
					bsrb1(in_bst, vps_default_ptl_dpb_hrd_max_tid_flag, 1);
				}
						
				if (vps_max_layers_minus1 > 0) {
					bsrb1(in_bst, vps_all_independent_layers_flag, 1);
				}

				memset(vps_max_tid_il_ref_pics_plus1, (uint8_t)((uint32_t)vps_max_sublayers_minus1 + 1), sizeof(vps_max_tid_il_ref_pics_plus1));
				for (int i = 0; i <= vps_max_layers_minus1; i++) {
					bsrb1(in_bst, vps_layer_id[i], 6);
					// When not present, the value of vps_independent_layer_flag[ i ] is inferred to be equal to 1
					vps_independent_layer_flag.BitSet(i);
					if (i > 0 && !vps_all_independent_layers_flag) {
						bsrbarray(in_bst, vps_independent_layer_flag, i);
						if (!vps_independent_layer_flag[i]) {
							bsrbarray(in_bst, vps_max_tid_ref_present_flag, i);
							for (int j = 0; j < i; j++) {
								bsrbarray(in_bst, vps_direct_ref_layer_flag[i], j);
								if (vps_max_tid_ref_present_flag[i] && vps_direct_ref_layer_flag[i][j]) {
									bsrb1(in_bst, vps_max_tid_il_ref_pics_plus1[i][j], 3);
								}
							}
						}
					}
				}

				UpdateReferenceLayers();

				if (vps_max_layers_minus1 > 0) {
					if (vps_all_independent_layers_flag) {
						bsrb1(in_bst, vps_each_layer_is_an_ols_flag, 1);
					}

					if (!vps_each_layer_is_an_ols_flag) {
						if (!vps_all_independent_layers_flag) {
							bsrb1(in_bst, vps_ols_mode_idc, 2);
						}
						else
							vps_ols_mode_idc = 2;

						if (vps_ols_mode_idc == 2) {
							bsrb1(in_bst, vps_num_output_layer_sets_minus2, 8);
							vps_ols_output_layer_flag.resize(vps_num_output_layer_sets_minus2 + 2);
							for (int i = 1; i <= (int)vps_num_output_layer_sets_minus2 + 1; i++){
								for (int j = 0; j <= (int)vps_max_layers_minus1; j++){
									bsrbarray(in_bst, vps_ols_output_layer_flag[i], j);
								}
							}
						}
					}

					bsrb1(in_bst, vps_num_ptls_minus1, 8);
				}

				vps_ptl_max_tid.resize(vps_num_ptls_minus1 + 1);
				for (int i = 0; i <= vps_num_ptls_minus1; i++) {
					if (i > 0) {
						bsrbarray(in_bst, vps_pt_present_flag, i);
					}
					else
						vps_pt_present_flag.BitClear(0);

					if (!vps_default_ptl_dpb_hrd_max_tid_flag) {
						bsrb1(in_bst, vps_ptl_max_tid[i], 3);
					}
					else
						vps_ptl_max_tid[i] = vps_max_sublayers_minus1;
				}

				UpdateOLSInfo();

				while (!AMBst_IsAligned(in_bst)) {
					bsrbarray(in_bst, vps_ptl_alignment_zero_bit, idx);
					idx++;
				}

				profile_tier_level.resize((size_t)vps_num_ptls_minus1 + 1);
				for (int i = 0; i <= (int)vps_num_ptls_minus1; i++) {
					bsrbreadref(in_bst, profile_tier_level[i], PROFILE_TIER_LEVEL, vps_pt_present_flag[i], vps_ptl_max_tid[i]);
				}

				vps_ols_ptl_idx.resize(TotalNumOlss);
				for (int i = 0; i < TotalNumOlss; i++) {
					if (vps_num_ptls_minus1 > 0 && vps_num_ptls_minus1 + 1 != TotalNumOlss) {
						bsrb1(in_bst, vps_ols_ptl_idx[i], 8);
					}
					else if (vps_num_ptls_minus1 == 0)
						vps_ols_ptl_idx[i] = 0;
					else
						vps_ols_ptl_idx[i] = i;
				}

				if (!vps_each_layer_is_an_ols_flag) {
					nal_read_ue(in_bst, vps_num_dpb_params_minus1, uint32_t);
					if (vps_max_sublayers_minus1 > 0) {
						bsrb1(in_bst, vps_sublayer_dpb_params_present_flag, 1);
					}

					int nVpsNumDpbParams = VpsNumDpbParams();
					if (nVpsNumDpbParams > 0)
					{
						vps_dpb_max_tid.resize((size_t)nVpsNumDpbParams);
						dpb_parameters.resize((size_t)nVpsNumDpbParams);
						for (int i = 0; i < nVpsNumDpbParams; i++) {
							if (!vps_default_ptl_dpb_hrd_max_tid_flag) {
								bsrb1(in_bst, vps_dpb_max_tid[i], 3);
							}
							else
								vps_dpb_max_tid[i] = vps_max_sublayers_minus1;
							bsrbreadref(in_bst, dpb_parameters[i], DPB_PARAMETERS, vps_dpb_max_tid[i], vps_sublayer_dpb_params_present_flag);
						}
					}
							
					if (NumMultiLayerOlss > 0)
					{
						vps_ols_dpb.resize(NumMultiLayerOlss);
						for (uint32_t i = 0; i < NumMultiLayerOlss; i++)
						{
							nal_read_ue(in_bst, vps_ols_dpb[i].pic_width, uint32_t);
							nal_read_ue(in_bst, vps_ols_dpb[i].pic_height, uint32_t);
							bsrb1(in_bst, vps_ols_dpb[i].chroma_format, 2);
							nal_read_ue(in_bst, vps_ols_dpb[i].bitdepth_minus8, uint32_t);
							if (nVpsNumDpbParams > 1 && nVpsNumDpbParams != NumMultiLayerOlss) {
								nal_read_ue(in_bst, vps_ols_dpb[i].params_idx, uint32_t);
							}
							else if (nVpsNumDpbParams == 1)
								vps_ols_dpb[i].params_idx = 0;
							else
								vps_ols_dpb[i].params_idx = i;
						}
					}

					bsrb1(in_bst, vps_timing_hrd_params_present_flag, 1);
					if (vps_timing_hrd_params_present_flag) {
						bsrbreadref(in_bst, general_timing_hrd_parameters, GENERAL_TIMING_HRD_PARAMETERS);
						if (vps_max_sublayers_minus1 > 0) {
							bsrb1(in_bst, vps_sublayer_cpb_params_present_flag, 1);
						}
						else
							vps_sublayer_cpb_params_present_flag = 0;
						nal_read_ue(in_bst, vps_num_ols_timing_hrd_params_minus1, uint16_t);

						vps_hrd_max_tid.resize((size_t)vps_num_ols_timing_hrd_params_minus1 + 1);
						ols_timing_hrd_parameters.resize((size_t)vps_num_ols_timing_hrd_params_minus1 + 1);
						for (uint32_t i = 0; i <= vps_num_ols_timing_hrd_params_minus1; i++)
						{
							if (!vps_default_ptl_dpb_hrd_max_tid_flag) {
								bsrb1(in_bst, vps_hrd_max_tid[i], 3);
							}

							uint8_t firstSubLayer = vps_sublayer_cpb_params_present_flag ? 0 : vps_hrd_max_tid[i];
							bsrbreadref(in_bst, ols_timing_hrd_parameters[i], OLS_TIMING_HRD_PARAMETERS, 
								firstSubLayer, vps_hrd_max_tid[i], general_timing_hrd_parameters);
						}

						if (vps_num_ols_timing_hrd_params_minus1 > 0 && vps_num_ols_timing_hrd_params_minus1 + 1 != NumMultiLayerOlss){
							vps_ols_timing_hrd_idx.resize(NumMultiLayerOlss);
							for (uint32_t i = 0; i < NumMultiLayerOlss; i++) {
								nal_read_ue(in_bst, vps_ols_timing_hrd_idx[i], uint32_t);
							}
						}
					}

					bsrb1(in_bst, vps_extension_flag, 1);
					if (vps_extension_flag) {
						idx = 0;
						while (AMBst_NAL_more_rbsp_data(in_bst)) {
							bsrbarray(in_bst, vps_extension_data_flag, idx);
							idx++;
						}
					}
					nal_read_obj(in_bst, rbsp_trailing_bits);
				}
							
				MAP_BST_END();
			}
			catch (AMException e)
			{
				return e.RetCode();
			}

			return RET_CODE_SUCCESS;
		}

		int NAL_UNIT::VIDEO_PARAMETER_SET_RBSP::Unmap(AMBst out_bst)
		{
			UNREFERENCED_PARAMETER(out_bst);
			return RET_CODE_ERROR_NOTIMPL;
		}

		DECLARE_FIELDPROP_BEGIN1(NAL_UNIT::VIDEO_PARAMETER_SET_RBSP)

		DECLARE_FIELDPROP_END()

		int NAL_UNIT::REF_PIC_LIST_STRUCT::Unmap(AMBst out_bst)
		{
			UNREFERENCED_PARAMETER(out_bst);
			return RET_CODE_ERROR_NOTIMPL;
		}

		DECLARE_FIELDPROP_BEGIN1(NAL_UNIT::REF_PIC_LIST_STRUCT)

		DECLARE_FIELDPROP_END()

		NAL_UNIT::SEQ_PARAMETER_SET_RBSP::SEQ_PARAMETER_SET_RBSP()
			: sps_res_change_in_clvs_allowed_flag(0)
			, sps_independent_subpics_flag(1)
			, sps_subpic_same_size_flag(0)
			, sps_subpic_id_mapping_explicitly_signalled_flag(0)
			, sps_conf_win_left_offset(0)
			, sps_conf_win_right_offset(0)
			, sps_conf_win_top_offset(0)
			, sps_conf_win_bottom_offset(0)
			, sps_num_subpics_minus1(0)
			, sps_sublayer_dpb_params_flag(0)
			, sps_qtbtt_dual_tree_intra_flag(0)
			, sps_log2_diff_max_bt_min_qt_intra_slice_chroma(0)
			, sps_log2_diff_max_tt_min_qt_intra_slice_chroma(0)
			, sps_log2_diff_max_bt_min_qt_intra_slice_luma(0)
			, sps_log2_diff_max_tt_min_qt_intra_slice_luma(0)
			, sps_log2_diff_max_bt_min_qt_inter_slice(0)
			, sps_log2_diff_max_tt_min_qt_inter_slice(0)
			, sps_max_luma_transform_size_64_flag(0)
			, sps_bdpcm_enabled_flag(0)
			, sps_explicit_mts_intra_enabled_flag(0)
			, sps_explicit_mts_inter_enabled_flag(0)
			, sps_joint_cbcr_enabled_flag(0)
			, sps_same_qp_table_for_chroma_flag(1)
			, sps_qp_table_start_minus26{ 0 }
			, sps_num_points_in_qp_table_minus1{ 0 }
			, sps_delta_qp_in_val_minus1{ {0} }
			, sps_ccalf_enabled_flag(0)
			, sps_inter_layer_prediction_enabled_flag(0)
			, sps_sbtmvp_enabled_flag(0)
			, sps_bdof_control_present_in_ph_flag(0)
			, sps_dmvr_control_present_in_ph_flag(0)
			, sps_mmvd_fullpel_only_enabled_flag(0)
			, sps_6param_affine_enabled_flag(0)
			, sps_affine_amvr_enabled_flag(0)
			, sps_affine_prof_enabled_flag(0)
			, sps_prof_control_present_in_ph_flag(0)
			, sps_gpm_enabled_flag(0)
			, sps_cclm_enabled_flag(0)
			, sps_chroma_horizontal_collocated_flag(1)
			, sps_chroma_vertical_collocated_flag(1)
			, sps_act_enabled_flag(0)
			, sps_ibc_enabled_flag(0)
			, sps_scaling_matrix_for_alternative_colour_space_disabled_flag(0)
			, sps_virtual_boundaries_present_flag(0)
			, sps_num_ver_virtual_boundaries(0)
			, sps_num_hor_virtual_boundaries(0)
			, sps_sublayer_cpb_params_present_flag(0)
		{
		}

		int NAL_UNIT::SEQ_PARAMETER_SET_RBSP::Map(AMBst in_bst)
		{
			int iRet = RET_CODE_SUCCESS;
			SYNTAX_BITSTREAM_MAP::Map(in_bst);

			try
			{
				int idx = 0;
				MAP_BST_BEGIN(0);
				bsrb1(in_bst, sps_seq_parameter_set_id, 4);
				bsrb1(in_bst, sps_video_parameter_set_id, 4);
				bsrb1(in_bst, sps_max_sublayers_minus1, 3);
				bsrb1(in_bst, sps_chroma_format_idc, 2);
				bsrb1(in_bst, sps_log2_ctu_size_minus5, 2);
				bsrb1(in_bst, sps_ptl_dpb_hrd_params_present_flag, 1);

				uint8_t CtbLog2SizeY = sps_log2_ctu_size_minus5 + 5;
				uint8_t CtbSizeY = (uint8_t)(1 << CtbLog2SizeY);

				if (sps_ptl_dpb_hrd_params_present_flag) {
					bsrbreadref(in_bst, profile_tier_level, PROFILE_TIER_LEVEL, 1, sps_max_sublayers_minus1);
				}

				bsrb1(in_bst, sps_gdr_enabled_flag, 1);
				bsrb1(in_bst, sps_ref_pic_resampling_enabled_flag, 1);

				if (sps_ref_pic_resampling_enabled_flag) {
					bsrb1(in_bst, sps_res_change_in_clvs_allowed_flag, 1);
				}

				nal_read_ue1(in_bst, sps_pic_width_max_in_luma_samples);
				nal_read_ue1(in_bst, sps_pic_height_max_in_luma_samples);
				bsrb1(in_bst, sps_conformance_window_flag, 1);
				if (sps_conformance_window_flag) {
					nal_read_ue1(in_bst, sps_conf_win_left_offset);
					nal_read_ue1(in_bst, sps_conf_win_right_offset);
					nal_read_ue1(in_bst, sps_conf_win_top_offset);
					nal_read_ue1(in_bst, sps_conf_win_bottom_offset);
				}

				bsrb1(in_bst, sps_subpic_info_present_flag, 1);
				if (sps_subpic_info_present_flag) {
					nal_read_ue1(in_bst, sps_num_subpics_minus1);
					if (sps_num_subpics_minus1 > 0) {
						bsrb1(in_bst, sps_independent_subpics_flag, 1);
						bsrb1(in_bst, sps_subpic_same_size_flag, 1);
					}

					uint32_t tmpWidthVal = sps_pic_width_max_in_luma_samples + CtbSizeY - 1;
					uint32_t tmpHeightVal = (sps_pic_height_max_in_luma_samples + CtbSizeY - 1) / CtbSizeY;
					uint32_t numSubpicCols = tmpWidthVal / (sps_subpic_width_minus1[0] + 1);
					sps_subpic_ctu_top_left_x.resize(sps_num_subpics_minus1 + 1);
					sps_subpic_ctu_top_left_y.resize(sps_num_subpics_minus1 + 1);
					sps_subpic_width_minus1.resize(sps_num_subpics_minus1 + 1);
					sps_subpic_height_minus1.resize(sps_num_subpics_minus1 + 1);
					for (int i = 0; sps_num_subpics_minus1 > 0 && i <= sps_num_subpics_minus1; i++) {
						if (!sps_subpic_same_size_flag || i == 0) {
							if (i > 0 && sps_pic_width_max_in_luma_samples > CtbSizeY) {
								bsrb1(in_bst, sps_subpic_ctu_top_left_x[i], quick_ceil_log2(tmpWidthVal));
							}
							else
								sps_subpic_ctu_top_left_x[i] = 0;

							if (i > 0 && sps_pic_height_max_in_luma_samples > CtbSizeY) {
								bsrb1(in_bst, sps_subpic_ctu_top_left_y[i], quick_ceil_log2(tmpWidthVal));
							}
							else
								sps_subpic_ctu_top_left_y[i] = 0;

							if (i < sps_num_subpics_minus1 && sps_pic_width_max_in_luma_samples > CtbSizeY) {
								bsrb1(in_bst, sps_subpic_width_minus1[i], quick_ceil_log2(tmpWidthVal));
							}
							else
								sps_subpic_width_minus1[i] = tmpWidthVal - sps_subpic_ctu_top_left_x[i] - 1;

							if (i < sps_num_subpics_minus1 && sps_pic_height_max_in_luma_samples > CtbSizeY) {
								bsrb1(in_bst, sps_subpic_height_minus1[i], quick_ceil_log2(tmpWidthVal));
							}
							else
								sps_subpic_height_minus1[i] = tmpHeightVal - sps_subpic_ctu_top_left_y[i] - 1;
						}
						else
						{
							sps_subpic_ctu_top_left_x[i] = (i%numSubpicCols)*(sps_subpic_width_minus1[0] + 1);
							sps_subpic_ctu_top_left_y[i] = (i / numSubpicCols)* (sps_subpic_height_minus1[0] + 1);
							  sps_subpic_width_minus1[i] = sps_subpic_width_minus1[0];
							 sps_subpic_height_minus1[i] = sps_subpic_height_minus1[0];
						}

						if (!sps_independent_subpics_flag) {
							bsrbarray(in_bst, sps_subpic_treated_as_pic_flag, i);
							bsrbarray(in_bst, sps_loop_filter_across_subpic_enabled_flag, 1);
						}
						else
						{
							sps_subpic_treated_as_pic_flag.BitSet(i);
							sps_loop_filter_across_subpic_enabled_flag.BitClear(i);
						}
					}

					nal_read_ue1(in_bst, sps_subpic_id_len_minus1);
					bsrb1(in_bst, sps_subpic_id_mapping_explicitly_signalled_flag, 1);

					if (sps_subpic_id_mapping_explicitly_signalled_flag) {
						bsrb1(in_bst, sps_subpic_id_mapping_present_flag, 1);
						if (sps_subpic_id_mapping_present_flag){
							sps_subpic_id.resize(sps_num_subpics_minus1 + 1);
							for (size_t i = 0; i <= sps_num_subpics_minus1; i++) {
								bsrb1(in_bst, sps_subpic_id[i], sps_subpic_id_len_minus1 + 1);
							}
						}
					}
				}

				nal_read_ue1(in_bst, sps_bitdepth_minus8);
				bsrb1(in_bst, sps_entropy_coding_sync_enabled_flag, 1);
				bsrb1(in_bst, sps_entry_point_offsets_present_flag, 1);
				bsrb1(in_bst, sps_log2_max_pic_order_cnt_lsb_minus4, 4);
				bsrb1(in_bst, sps_poc_msb_cycle_flag, 1);

				if (sps_poc_msb_cycle_flag) {
					nal_read_ue1(in_bst, sps_poc_msb_cycle_len_minus1);
				}

				bsrb1(in_bst, sps_num_extra_ph_bytes, 2);

				NumExtraPhBits = 0;
				for (size_t i = 0; i < (size_t)(sps_num_extra_ph_bytes * 8); i++) {
					bsrbarray(in_bst, sps_extra_ph_bit_present_flag, i);
					if (sps_extra_ph_bit_present_flag[i])
						NumExtraPhBits++;
				}
				
				bsrb1(in_bst, sps_num_extra_sh_bytes, 2);

				NumExtraShBits = 0;
				for (size_t i = 0; i < (sps_num_extra_sh_bytes * 8); i++) {
					bsrbarray(in_bst, sps_extra_sh_bit_present_flag, i);
					if (sps_extra_sh_bit_present_flag[i])
						NumExtraShBits++;
				}

				if (sps_ptl_dpb_hrd_params_present_flag) {
					if (sps_max_sublayers_minus1 > 0) {
						bsrb1(in_bst, sps_sublayer_dpb_params_flag, 1);
					}
					bsrbreadref(in_bst, dpb_parameters, DPB_PARAMETERS, sps_max_sublayers_minus1, sps_sublayer_dpb_params_flag);
				}

				nal_read_ue1(in_bst, sps_log2_min_luma_coding_block_size_minus2);
				bsrb1(in_bst, sps_partition_constraints_override_enabled_flag, 1);

				nal_read_ue1(in_bst, sps_log2_diff_min_qt_min_cb_intra_slice_luma);
				nal_read_ue1(in_bst, sps_max_mtt_hierarchy_depth_intra_slice_luma);

				if (sps_max_mtt_hierarchy_depth_intra_slice_luma != 0) {
					nal_read_ue1(in_bst, sps_log2_diff_max_bt_min_qt_intra_slice_luma);
					nal_read_ue1(in_bst, sps_log2_diff_max_tt_min_qt_intra_slice_luma);
				}

				if (sps_chroma_format_idc != 0) {
					bsrb1(in_bst, sps_qtbtt_dual_tree_intra_flag, 1);	
				}

				if (sps_qtbtt_dual_tree_intra_flag) {
					nal_read_ue1(in_bst, sps_log2_diff_min_qt_min_cb_intra_slice_chroma);
					nal_read_ue1(in_bst, sps_max_mtt_hierarchy_depth_intra_slice_chroma);
					if (sps_max_mtt_hierarchy_depth_intra_slice_chroma != 0) {
						nal_read_ue1(in_bst, sps_log2_diff_max_bt_min_qt_intra_slice_chroma);
						nal_read_ue1(in_bst, sps_log2_diff_max_tt_min_qt_intra_slice_chroma);
					}
				}

				nal_read_ue1(in_bst, sps_log2_diff_min_qt_min_cb_inter_slice);
				nal_read_ue1(in_bst, sps_max_mtt_hierarchy_depth_inter_slice);
				if (sps_max_mtt_hierarchy_depth_inter_slice != 0) {
					nal_read_ue1(in_bst, sps_log2_diff_max_bt_min_qt_inter_slice);
					nal_read_ue1(in_bst, sps_log2_diff_max_tt_min_qt_inter_slice);
				}

				if (CtbSizeY > 32) {
					bsrb1(in_bst, sps_max_luma_transform_size_64_flag, 1);
				}
				bsrb1(in_bst, sps_transform_skip_enabled_flag, 1);
				if (sps_transform_skip_enabled_flag) {
					nal_read_ue1(in_bst, sps_log2_transform_skip_max_size_minus2);
					bsrb1(in_bst, sps_bdpcm_enabled_flag, 1);
				}
				bsrb1(in_bst, sps_mts_enabled_flag, 1);
				if (sps_mts_enabled_flag) {
					bsrb1(in_bst, sps_explicit_mts_intra_enabled_flag, 1);
					bsrb1(in_bst, sps_explicit_mts_inter_enabled_flag, 1);
				}
				bsrb1(in_bst, sps_lfnst_enabled_flag, 1);

				if (sps_chroma_format_idc != 0) {
					bsrb1(in_bst, sps_joint_cbcr_enabled_flag, 1);
					bsrb1(in_bst, sps_same_qp_table_for_chroma_flag, 1);

					uint8_t numQpTables = sps_same_qp_table_for_chroma_flag ? 1 : (sps_joint_cbcr_enabled_flag ? 3 : 2);
					for (uint8_t i = 0; i < numQpTables; i++) {
						nal_read_se1(in_bst, sps_qp_table_start_minus26[i]);
						nal_read_ue1(in_bst, sps_num_points_in_qp_table_minus1[i]);
						for (uint16_t j = 0; j <= sps_num_points_in_qp_table_minus1[i]; j++) {
							nal_read_ue1(in_bst, sps_delta_qp_in_val_minus1[i][j]);
							nal_read_ue1(in_bst, sps_delta_qp_diff_val[i][j]);
						}
					}
				}

				bsrb1(in_bst, sps_sao_enabled_flag, 1);
				bsrb1(in_bst, sps_alf_enabled_flag, 1);
				if (sps_alf_enabled_flag && sps_chroma_format_idc != 0) {
					bsrb1(in_bst, sps_ccalf_enabled_flag, 1);
				}

				bsrb1(in_bst, sps_lmcs_enabled_flag, 1);
				bsrb1(in_bst, sps_weighted_pred_flag, 1);
				bsrb1(in_bst, sps_weighted_bipred_flag, 1);
				bsrb1(in_bst, sps_long_term_ref_pics_flag, 1);

				if (sps_video_parameter_set_id > 0) {
					bsrb1(in_bst, sps_inter_layer_prediction_enabled_flag, 1);
				}

				bsrb1(in_bst, sps_idr_rpl_present_flag, 1);
				bsrb1(in_bst, sps_rpl1_same_as_rpl0_flag, 1);

				for (uint8_t i = 0; i < (sps_rpl1_same_as_rpl0_flag ? 1 : 2); i++) {
					nal_read_ue1(in_bst, sps_num_ref_pic_lists[i]);
					for (uint16_t j = 0; j < sps_num_ref_pic_lists[i]; j++) {
						bsrbreadref(in_bst, ref_pic_list_struct[i][j], REF_PIC_LIST_STRUCT, i, j, this);
					}
				}

				bsrb1(in_bst, sps_ref_wraparound_enabled_flag, 1);
				bsrb1(in_bst, sps_temporal_mvp_enabled_flag, 1);

				if (sps_temporal_mvp_enabled_flag) {
					bsrb1(in_bst, sps_sbtmvp_enabled_flag, 1);
				}

				bsrb1(in_bst, sps_amvr_enabled_flag, 1);
				bsrb1(in_bst, sps_bdof_enabled_flag, 1);
				if (sps_bdof_enabled_flag) {
					bsrb1(in_bst, sps_bdof_control_present_in_ph_flag, 1);
				}
				bsrb1(in_bst, sps_smvd_enabled_flag, 1);
				bsrb1(in_bst, sps_dmvr_enabled_flag, 1);
				if (sps_dmvr_enabled_flag) {
					bsrb1(in_bst, sps_dmvr_control_present_in_ph_flag, 1);
				}
				bsrb1(in_bst, sps_mmvd_enabled_flag, 1);
				if (sps_mmvd_enabled_flag) {
					bsrb1(in_bst, sps_mmvd_fullpel_only_enabled_flag, 1);
				}
				nal_read_ue1(in_bst, sps_six_minus_max_num_merge_cand);
				bsrb1(in_bst, sps_sbt_enabled_flag, 1);
				bsrb1(in_bst, sps_affine_enabled_flag, 1);

				uint8_t MaxNumMergeCand = (uint8_t)(6 - sps_six_minus_max_num_merge_cand);

				if (sps_affine_enabled_flag) {
					nal_read_ue1(in_bst, sps_five_minus_max_num_subblock_merge_cand);
					bsrb1(in_bst, sps_6param_affine_enabled_flag, 1);
					if (sps_amvr_enabled_flag) {
						bsrb1(in_bst, sps_affine_amvr_enabled_flag, 1);
					}
					bsrb1(in_bst, sps_affine_prof_enabled_flag, 1);
					if (sps_affine_prof_enabled_flag) {
						bsrb1(in_bst, sps_prof_control_present_in_ph_flag, 1);
					}
				}

				bsrb1(in_bst, sps_bcw_enabled_flag, 1);
				bsrb1(in_bst, sps_ciip_enabled_flag, 1);

				if (MaxNumMergeCand >= 2) {
					bsrb1(in_bst, sps_gpm_enabled_flag, 1);
					if (sps_gpm_enabled_flag && MaxNumMergeCand >= 3) {
						nal_read_ue1(in_bst, sps_max_num_merge_cand_minus_max_num_gpm_cand);
					}
				}

				nal_read_ue1(in_bst, sps_log2_parallel_merge_level_minus2);

				bsrb1(in_bst, sps_isp_enabled_flag, 1);
				bsrb1(in_bst, sps_mrl_enabled_flag, 1);
				bsrb1(in_bst, sps_mip_enabled_flag, 1);

				if (sps_chroma_format_idc != 0) {
					bsrb1(in_bst, sps_cclm_enabled_flag, 1);
				}

				if (sps_chroma_format_idc == 1) {
					bsrb1(in_bst, sps_chroma_horizontal_collocated_flag, 1);
					bsrb1(in_bst, sps_chroma_vertical_collocated_flag, 1);
				}

				bsrb1(in_bst, sps_palette_enabled_flag, 1);
				if (sps_chroma_format_idc == 3 && !sps_max_luma_transform_size_64_flag) {
					bsrb1(in_bst, sps_act_enabled_flag, 1);
				}

				if (sps_transform_skip_enabled_flag || sps_palette_enabled_flag) {
					nal_read_ue1(in_bst, sps_min_qp_prime_ts);
				}

				bsrb1(in_bst, sps_ibc_enabled_flag, 1);
				uint8_t MaxNumIbcMergeCand = 0;
				if (sps_ibc_enabled_flag) {
					nal_read_ue1(in_bst, sps_six_minus_max_num_ibc_merge_cand);
					MaxNumIbcMergeCand = 6 - sps_six_minus_max_num_ibc_merge_cand;
				}

				bsrb1(in_bst, sps_ladf_enabled_flag, 1);

				if (sps_ladf_enabled_flag) {
					bsrb1(in_bst, sps_num_ladf_intervals_minus2, 2);
					nal_read_se1(in_bst, sps_ladf_lowest_interval_qp_offset);

					for (uint32_t i = 0; i < ((uint32_t)sps_num_ladf_intervals_minus2 + 1); i++) {
						nal_read_se1(in_bst, sps_ladf_qp_offset[i]);
						nal_read_ue1(in_bst, sps_ladf_delta_threshold_minus1[i]);
					}
				}

				bsrb1(in_bst, sps_explicit_scaling_list_enabled_flag, 1);
				if (sps_lfnst_enabled_flag && sps_explicit_scaling_list_enabled_flag) {
					bsrb1(in_bst, sps_scaling_matrix_for_lfnst_disabled_flag, 1);
				}

				if (sps_act_enabled_flag && sps_explicit_scaling_list_enabled_flag) {
					bsrb1(in_bst, sps_scaling_matrix_for_alternative_colour_space_disabled_flag, 1);
				}

				if (sps_scaling_matrix_for_alternative_colour_space_disabled_flag) {
					bsrb1(in_bst, sps_scaling_matrix_designated_colour_space_flag, 1);
				}

				bsrb1(in_bst, sps_dep_quant_enabled_flag, 1);
				bsrb1(in_bst, sps_sign_data_hiding_enabled_flag, 1);
				bsrb1(in_bst, sps_virtual_boundaries_enabled_flag, 1);

				if (sps_virtual_boundaries_enabled_flag) {
					bsrb1(in_bst, sps_virtual_boundaries_present_flag, 1);
					if (sps_virtual_boundaries_present_flag) {
						nal_read_ue1(in_bst, sps_num_ver_virtual_boundaries);
						for (int i = 0; i < sps_num_ver_virtual_boundaries; i++) {
							nal_read_ue1(in_bst, sps_virtual_boundary_pos_x_minus1[i]);
						}
						nal_read_ue1(in_bst, sps_num_hor_virtual_boundaries);
						for (int i = 0; i < sps_num_hor_virtual_boundaries; i++) {
							nal_read_ue1(in_bst, sps_virtual_boundary_pos_y_minus1[i]);
						}
					}
				}

				if (sps_ptl_dpb_hrd_params_present_flag) {
					bsrb1(in_bst, sps_timing_hrd_params_present_flag, 1);
					if (sps_timing_hrd_params_present_flag) {
						bsrbreadref(in_bst, general_timing_hrd_parameters, GENERAL_TIMING_HRD_PARAMETERS);
						if (sps_max_sublayers_minus1 > 0) {
							bsrb1(in_bst, sps_sublayer_cpb_params_present_flag, 1);
							uint8_t firstSubLayer = sps_sublayer_cpb_params_present_flag ? 0 : sps_max_sublayers_minus1;
							bsrbreadref(in_bst, ols_timing_hrd_parameters, OLS_TIMING_HRD_PARAMETERS, 
								firstSubLayer, sps_max_sublayers_minus1, general_timing_hrd_parameters);
						}
					}
				}

				bsrb1(in_bst, sps_field_seq_flag, 1);
				bsrb1(in_bst, sps_vui_parameters_present_flag, 1);

				if (sps_vui_parameters_present_flag) {
					nal_read_ue1(in_bst, sps_vui_payload_size_minus1);
					idx = 0;
					while (AMBst_IsAligned(in_bst)) {
						bsrbarray(in_bst, sps_vui_alignment_zero_bit, idx);
						idx++;
					}

					// Convert EBSP into RBSP
					uint8_t vui_payload_buf[1024] = { 0 };
					AMBst_GetBytes(in_bst, vui_payload_buf, sps_vui_payload_size_minus1 + 1);

					vui_payload = new VUI_PAYLOAD(sps_vui_payload_size_minus1 + 1);
					vui_payload->Map(vui_payload_buf, sps_vui_payload_size_minus1 + 1);
				}

				bsrb1(in_bst, sps_extension_flag, 1);

				if (sps_extension_flag)
				{
					idx = 0;
					while (AMBst_NAL_more_rbsp_data(in_bst)) {
						bsrbarray(in_bst, sps_extension_data_flag, idx);
						idx++;
					}
				}

				nal_read_obj(in_bst, rbsp_trailing_bits);

				MAP_BST_END();
			}
			catch (AMException e)
			{
				return e.RetCode();
			}

			return RET_CODE_SUCCESS;
		}

	}
}

