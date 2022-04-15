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
			uint8_t dependencyFlag[64][64] = { 0 };
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
				if (m_seq_parameter_set_rbsp->sps_long_term_ref_pics_flag &&
					m_rplsIdx < m_seq_parameter_set_rbsp->sps_num_ref_pic_lists[m_listIdx] && num_ref_entries > 0)
					bsrb1(in_bst, ltrp_in_header_flag, 1);
				abs_delta_poc_st.resize(num_ref_entries);
				ilrp_idx.resize(num_ref_entries);
				for (uint16_t i = 0, j = 0; i < num_ref_entries; i++)
				{
					if (m_seq_parameter_set_rbsp->sps_inter_layer_prediction_enabled_flag) {
						bsrbarray(in_bst, inter_layer_ref_pic_flag, i);
					}
					else
						inter_layer_ref_pic_flag.BitClear(i);

					if (!inter_layer_ref_pic_flag[i]) {
						if (m_seq_parameter_set_rbsp->sps_long_term_ref_pics_flag) {
							bsrbarray(in_bst, st_ref_pic_flag, i);
						}
						else
							st_ref_pic_flag.BitSet(i);

						if (st_ref_pic_flag[i]) {
							nal_read_ue(in_bst, abs_delta_poc_st[i], uint16_t);
							uint32_t AbsDeltaPocSt = (uint32_t)abs_delta_poc_st[i] + 1;
							if ((m_seq_parameter_set_rbsp->sps_weighted_pred_flag || m_seq_parameter_set_rbsp->sps_weighted_bipred_flag) && i != 0)
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
							bsrb1(in_bst, rpls_poc_lsb_lt_value, m_seq_parameter_set_rbsp->sps_log2_max_pic_order_cnt_lsb_minus4 + 4);
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

		int NAL_UNIT::REF_PIC_LIST_STRUCT::Unmap(AMBst out_bst)
		{
			UNREFERENCED_PARAMETER(out_bst);
			return RET_CODE_ERROR_NOTIMPL;
		}

		DECLARE_FIELDPROP_BEGIN1(NAL_UNIT::REF_PIC_LIST_STRUCT)

		DECLARE_FIELDPROP_END()
	}
}

