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

RET_CODE CreateVVCNALContext(INALVVCContext** ppNALCtx)
{
	if (ppNALCtx == NULL)
		return RET_CODE_INVALID_PARAMETER;

	auto pCtx = new BST::H266Video::VideoBitstreamCtx();
	pCtx->AddRef();
	*ppNALCtx = (INALVVCContext*)pCtx;
	return RET_CODE_SUCCESS;
}

namespace BST
{
	namespace H266Video
	{
		int read_extension_and_trailing_bits(AMBst in_bst, SYNTAX_MAP_STATUS& map_status, 
											 CAMBitArray& extension_data_flag, RBSP_TRAILING_BITS& rbsp_trailing_bits)
		{
			int left_bits = 0;
			int bit_pos = AMBst_Tell(in_bst, &left_bits);
			int i = 0;
			for (; i < left_bits - 8; i++) {
				bsrbarray(in_bst, extension_data_flag, i);
				bit_pos++;
			}

			left_bits = left_bits < 8 ? left_bits : 8;

			if (left_bits > 0)
			{
				int zero_bit_count = 0;
				uint8_t final_part = (uint8_t)AMBst_GetBits(in_bst, left_bits);
				for (int j = 0; j < left_bits; j++) {
					if (!(final_part&(1 << j)))
						zero_bit_count++;
				}

				// Can't find the stop bit, it is an incompatible buffer
				if (zero_bit_count >= left_bits)
				{
					return RET_CODE_BUFFER_NOT_COMPATIBLE;
				}

				if (left_bits > zero_bit_count + 1)
				{
					for (int j = 0; j < left_bits - (zero_bit_count + 1); j++, i++) {
						if ((final_part >> (left_bits - j - 1)) & 0x1)
							extension_data_flag.BitSet(i);
						else
							extension_data_flag.BitClear(i);
						map_status.number_of_fields++;
						bit_pos++;
					}

					left_bits = zero_bit_count + 1;
				}

				rbsp_trailing_bits.bit_pos = bit_pos;
				rbsp_trailing_bits.bit_end_pos = bit_pos + zero_bit_count + 1;

				for (int j = 0; j < zero_bit_count + 1; j++) {
					if ((final_part >> (zero_bit_count - j)) & 0x01)
						rbsp_trailing_bits.rbsp_trailing_bits.BitSet(j);
					else
						rbsp_trailing_bits.rbsp_trailing_bits.BitClear(j);
				}
			}

			return RET_CODE_SUCCESS;
		}

		RET_CODE VideoBitstreamCtx::SetNUFilters(std::initializer_list<uint8_t> NU_type_filters)
		{
			nal_unit_type_filters = NU_type_filters;
			return RET_CODE_SUCCESS;
		}

		RET_CODE VideoBitstreamCtx::GetNUFilters(std::vector<uint8_t>& NU_type_filters)
		{
			NU_type_filters = nal_unit_type_filters;

			return RET_CODE_SUCCESS;
		}

		bool VideoBitstreamCtx::IsNUFiltered(uint8_t nal_unit_type)
		{
			return nal_unit_type_filters.empty() ||
				std::find(nal_unit_type_filters.cbegin(), nal_unit_type_filters.cend(), nal_unit_type) != nal_unit_type_filters.cend();
		}

		void VideoBitstreamCtx::Reset()
		{
			nal_unit_type_filters.clear();
			sps_seq_parameter_set_id.clear();
			prev_vps_video_parameter_set_id = -1;
			sp_prev_nal_unit = nullptr;
			m_active_nu_type = -1;
		}

		H266_NU VideoBitstreamCtx::GetVVCDCI()
		{
			return m_sp_dci;
		}

		H266_NU VideoBitstreamCtx::GetVVCOPI()
		{
			return m_sp_opi;
		}

		H266_NU VideoBitstreamCtx::GetVVCVPS(uint8_t vps_id)
		{
			auto iter = m_sp_vpses.find(vps_id);
			if (iter != m_sp_vpses.end())
				return iter->second;

			return std::shared_ptr<NAL_UNIT>();
		}

		H266_NU VideoBitstreamCtx::GetVVCSPS(uint8_t sps_id)
		{
			auto iter = m_sp_spses.find(sps_id);
			if (iter != m_sp_spses.end())
				return iter->second;

			return std::shared_ptr<NAL_UNIT>();
		}

		H266_NU VideoBitstreamCtx::GetVVCPPS(uint8_t pps_id)
		{
			auto iter = m_sp_ppses.find(pps_id);
			if (iter != m_sp_ppses.end())
				return iter->second;

			return std::shared_ptr<NAL_UNIT>();
		}

		H266_NU VideoBitstreamCtx::GetVVCPH() {
			return m_ph;
		}

		RET_CODE VideoBitstreamCtx::UpdateVVCDCI(H266_NU dci_nu)
		{
			m_sp_dci = dci_nu;
			return RET_CODE_SUCCESS;
		}

		RET_CODE VideoBitstreamCtx::UpdateVVCOPI(H266_NU opi_nu)
		{
			m_sp_opi = opi_nu;
			return RET_CODE_SUCCESS;
		}

		RET_CODE VideoBitstreamCtx::UpdateVVCVPS(H266_NU vps_nu)
		{
			if (!vps_nu || vps_nu->nal_unit_header.nal_unit_type != VPS_NUT || vps_nu->ptr_video_parameter_set_rbsp == nullptr)
				return RET_CODE_INVALID_PARAMETER;

			m_sp_vpses[vps_nu->ptr_video_parameter_set_rbsp->vps_video_parameter_set_id] = vps_nu;
			return RET_CODE_SUCCESS;
		}

		RET_CODE VideoBitstreamCtx::UpdateVVCSPS(H266_NU sps_nu)
		{
			if (!sps_nu || sps_nu->nal_unit_header.nal_unit_type != SPS_NUT || sps_nu->ptr_seq_parameter_set_rbsp == nullptr)
				return RET_CODE_INVALID_PARAMETER;

			m_sp_spses[sps_nu->ptr_seq_parameter_set_rbsp->sps_seq_parameter_set_id] = sps_nu;
			return RET_CODE_SUCCESS;
		}

		RET_CODE VideoBitstreamCtx::UpdateVVCPPS(H266_NU pps_nu)
		{
			if (!pps_nu || pps_nu->nal_unit_header.nal_unit_type != PPS_NUT || pps_nu->ptr_pic_parameter_set_rbsp == nullptr)
				return RET_CODE_INVALID_PARAMETER;

			m_sp_ppses[pps_nu->ptr_pic_parameter_set_rbsp->pps_pic_parameter_set_id] = pps_nu;
			return RET_CODE_SUCCESS;
		}

		RET_CODE VideoBitstreamCtx::UpdateVVCPH(H266_NU ph_nu) {
			m_ph = ph_nu;
			return RET_CODE_SUCCESS;
		}

		H266_NU VideoBitstreamCtx::CreateVVCNU()
		{
			auto ptr_VVC_NU = new NAL_UNIT;
			ptr_VVC_NU->UpdateCtx(this);
			return std::shared_ptr<NAL_UNIT>(ptr_VVC_NU);
		}

		int8_t 	VideoBitstreamCtx::GetActiveSPSID()
		{
			return m_active_sps_id;
		}

		RET_CODE VideoBitstreamCtx::ActivateSPS(int8_t sps_id)
		{
			m_active_sps_id = sps_id;
			return RET_CODE_SUCCESS;
		}

		RET_CODE VideoBitstreamCtx::DetactivateSPS()
		{
			m_active_sps_id = -1;
			return RET_CODE_SUCCESS;
		}

		RET_CODE VideoBitstreamCtx::GetNumRefIdxActive(int8_t num_ref_idx_active[2])
		{
			num_ref_idx_active[0] = NumRefIdxActive[0];
			num_ref_idx_active[1] = NumRefIdxActive[1];
			return RET_CODE_SUCCESS;
		}

		RET_CODE VideoBitstreamCtx::GetRplsIdx(int8_t rpls_idx[2])
		{
			rpls_idx[0] = RplsIdx[0];
			rpls_idx[1] = RplsIdx[1];
			return RET_CODE_SUCCESS;
		}

		//
		//	access_unit_delimiter_rbsp()
		//
		int NAL_UNIT::ACCESS_UNIT_DELIMITER_RBSP::Map(AMBst in_bst)
		{
			SYNTAX_BITSTREAM_MAP::Map(in_bst);
			try
			{
				uint8_t idx = 0;
				MAP_BST_BEGIN(0);

				bsrb1(in_bst, aud_irap_or_gdr_flag, 1);
				bsrb1(in_bst, aud_pic_type, 3);

				nal_read_obj(in_bst, rbsp_tailing_bits);

				MAP_BST_END();
			}
			catch (AMException e)
			{
				return e.RetCode();
			}

			return RET_CODE_SUCCESS;
		}

		int NAL_UNIT::ACCESS_UNIT_DELIMITER_RBSP::Unmap(AMBst out_bst)
		{
			UNREFERENCED_PARAMETER(out_bst);
			return RET_CODE_ERROR_NOTIMPL;
		}

		DECLARE_FIELDPROP_BEGIN1(NAL_UNIT::ACCESS_UNIT_DELIMITER_RBSP)
			NAV_WRITE_TAG_BEGIN_WITH_ALIAS("access_unit_delimiter_rbsp", "access_unit_delimiter_rbsp()", "the start of an AU");
			BST_FIELD_PROP_BOOL(aud_irap_or_gdr_flag, "an IRAP or GDR AU", "NOT an IRAP or GDR AU");
			BST_FIELD_PROP_NUMBER1(aud_pic_type, 3, aud_pic_type==0?"I":(aud_pic_type==1?"P,I":(aud_pic_type == 2?"B,P,I":"Unknown")));
			BST_FIELD_PROP_OBJECT(rbsp_tailing_bits);
			NAV_WRITE_TAG_END("access_unit_delimiter_rbsp");
		DECLARE_FIELDPROP_END()

		//
		// general_constraints_info()
		//
		int NAL_UNIT::GENERAL_CONSTRAINTS_INFO::Map(AMBst in_bst)
		{
			SYNTAX_BITSTREAM_MAP::Map(in_bst);
			try
			{
				uint8_t idx = 0;
				MAP_BST_BEGIN(0);
				bsrb1(in_bst, gci_present_flag, 1);
				if (gci_present_flag) {
					bsrb1(in_bst, gci_intra_only_constraint_flag, 1);
					bsrb1(in_bst, gci_all_layers_independent_constraint_flag, 1);
					bsrb1(in_bst, gci_one_au_only_constraint_flag, 1);
					bsrb1(in_bst, gci_sixteen_minus_max_bitdepth_constraint_idc, 4);
					bsrb1(in_bst, gci_three_minus_max_chroma_format_constraint_idc, 2);
					bsrb1(in_bst, gci_no_mixed_nalu_types_in_pic_constraint_flag, 1);
					bsrb1(in_bst, gci_no_trail_constraint_flag, 1);
					bsrb1(in_bst, gci_no_stsa_constraint_flag, 1);
					bsrb1(in_bst, gci_no_rasl_constraint_flag, 1);
					bsrb1(in_bst, gci_no_radl_constraint_flag, 1);
					bsrb1(in_bst, gci_no_idr_constraint_flag, 1);
					bsrb1(in_bst, gci_no_cra_constraint_flag, 1);
					bsrb1(in_bst, gci_no_gdr_constraint_flag, 1);
					bsrb1(in_bst, gci_no_aps_constraint_flag, 1);
					bsrb1(in_bst, gci_no_idr_rpl_constraint_flag, 1);
					bsrb1(in_bst, gci_one_tile_per_pic_constraint_flag, 1);
					bsrb1(in_bst, gci_pic_header_in_slice_header_constraint_flag, 1);
					bsrb1(in_bst, gci_one_slice_per_pic_constraint_flag, 1);
					bsrb1(in_bst, gci_no_rectangular_slice_constraint_flag, 1);
					bsrb1(in_bst, gci_one_slice_per_subpic_constraint_flag, 1);
					bsrb1(in_bst, gci_no_subpic_info_constraint_flag, 1);
					bsrb1(in_bst, gci_three_minus_max_log2_ctu_size_constraint_idc, 2);
					bsrb1(in_bst, gci_no_partition_constraints_override_constraint_flag, 1);
					bsrb1(in_bst, gci_no_mtt_constraint_flag, 1);
					bsrb1(in_bst, gci_no_qtbtt_dual_tree_intra_constraint_flag, 1);
					bsrb1(in_bst, gci_no_palette_constraint_flag, 1);
					bsrb1(in_bst, gci_no_ibc_constraint_flag, 1);
					bsrb1(in_bst, gci_no_isp_constraint_flag, 1);
					bsrb1(in_bst, gci_no_mrl_constraint_flag, 1);
					bsrb1(in_bst, gci_no_mip_constraint_flag, 1);
					bsrb1(in_bst, gci_no_cclm_constraint_flag, 1);
					bsrb1(in_bst, gci_no_ref_pic_resampling_constraint_flag, 1);
					bsrb1(in_bst, gci_no_res_change_in_clvs_constraint_flag, 1);
					bsrb1(in_bst, gci_no_weighted_prediction_constraint_flag, 1);
					bsrb1(in_bst, gci_no_ref_wraparound_constraint_flag, 1);
					bsrb1(in_bst, gci_no_temporal_mvp_constraint_flag, 1);
					bsrb1(in_bst, gci_no_sbtmvp_constraint_flag, 1);
					bsrb1(in_bst, gci_no_amvr_constraint_flag, 1);
					bsrb1(in_bst, gci_no_bdof_constraint_flag, 1);
					bsrb1(in_bst, gci_no_smvd_constraint_flag, 1);
					bsrb1(in_bst, gci_no_dmvr_constraint_flag, 1);
					bsrb1(in_bst, gci_no_mmvd_constraint_flag, 1);
					bsrb1(in_bst, gci_no_affine_motion_constraint_flag, 1);
					bsrb1(in_bst, gci_no_prof_constraint_flag, 1);
					bsrb1(in_bst, gci_no_bcw_constraint_flag, 1);
					bsrb1(in_bst, gci_no_ciip_constraint_flag, 1);
					bsrb1(in_bst, gci_no_gpm_constraint_flag, 1);
					bsrb1(in_bst, gci_no_luma_transform_size_64_constraint_flag, 1);
					bsrb1(in_bst, gci_no_transform_skip_constraint_flag, 1);
					bsrb1(in_bst, gci_no_bdpcm_constraint_flag, 1);
					bsrb1(in_bst, gci_no_mts_constraint_flag, 1);
					bsrb1(in_bst, gci_no_lfnst_constraint_flag, 1);
					bsrb1(in_bst, gci_no_joint_cbcr_constraint_flag, 1);
					bsrb1(in_bst, gci_no_sbt_constraint_flag, 1);
					bsrb1(in_bst, gci_no_act_constraint_flag, 1);
					bsrb1(in_bst, gci_no_explicit_scaling_list_constraint_flag, 1);
					bsrb1(in_bst, gci_no_dep_quant_constraint_flag, 1);
					bsrb1(in_bst, gci_no_sign_data_hiding_constraint_flag, 1);
					bsrb1(in_bst, gci_no_cu_qp_delta_constraint_flag, 1);
					bsrb1(in_bst, gci_no_chroma_qp_offset_constraint_flag, 1);
					bsrb1(in_bst, gci_no_sao_constraint_flag, 1);
					bsrb1(in_bst, gci_no_alf_constraint_flag, 1);
					bsrb1(in_bst, gci_no_ccalf_constraint_flag, 1);
					bsrb1(in_bst, gci_no_lmcs_constraint_flag, 1);
					bsrb1(in_bst, gci_no_ladf_constraint_flag, 1);
					bsrb1(in_bst, gci_no_virtual_boundaries_constraint_flag, 1);

					bsrb1(in_bst, gci_num_reserved_bits, 8);
					for (int i = 0; i < gci_num_reserved_bits; i++) {
						bsrbarray(in_bst, gci_reserved_zero_bit, i);
					}
				}

				idx = 0;
				while (!AMBst_IsAligned(in_bst)) {
					bsrbarray(in_bst, gci_alignment_zero_bit, idx);
					idx++;
				}

				MAP_BST_END();
			}
			catch (AMException e)
			{
				return e.RetCode();
			}

			return RET_CODE_SUCCESS;
		}

		int NAL_UNIT::GENERAL_CONSTRAINTS_INFO::Unmap(AMBst out_bst)
		{
			UNREFERENCED_PARAMETER(out_bst);
			return RET_CODE_ERROR_NOTIMPL;
		}

		DECLARE_FIELDPROP_BEGIN1(NAL_UNIT::GENERAL_CONSTRAINTS_INFO)
			BST_FIELD_PROP_BOOL(gci_present_flag, "GCI syntax elements are present", "GCI fields are not present");
			if (gci_present_flag)
			{
				BST_FIELD_PROP_BOOL(gci_intra_only_constraint_flag, 
					"sh_slice_type for all slices in OlsInScope shall be equal to 2", 
					"sh_slice_type for some slices in OlsInScope may NOT be equal to 2");
				BST_FIELD_PROP_BOOL(gci_all_layers_independent_constraint_flag,
					"vps_all_independent_layers_flag for all pictures in OlsInScope shall be equal to 1",
					"vps_all_independent_layers_flag for some pictures in OlsInScope may NOT be equal to 1");
				BST_FIELD_PROP_BOOL(gci_one_au_only_constraint_flag,
					"only one AU in OlsInScope",
					"More than one AU may be in OlsInScope");
				if (gci_sixteen_minus_max_bitdepth_constraint_idc > 0) {
					BST_FIELD_PROP_2NUMBER1_F(gci_sixteen_minus_max_bitdepth_constraint_idc, 4, 
						"sps_bitdepth_minus8 plus 8 for all pictures in OlsInScope shall be in the range of 0 to %d, inclusive.",
						(16 - gci_sixteen_minus_max_bitdepth_constraint_idc));
				}
				else
				{
					BST_FIELD_PROP_2NUMBER1(gci_sixteen_minus_max_bitdepth_constraint_idc, 4, "");
				}

				if (gci_three_minus_max_chroma_format_constraint_idc > 0) {
					BST_FIELD_PROP_2NUMBER1_F(gci_three_minus_max_chroma_format_constraint_idc, 4, 
						"sps_chroma_format_idc for all pictures in OlsInScope shall be in the range of 0 to %d, inclusive",
						(3 - gci_three_minus_max_chroma_format_constraint_idc));
				}
				else
				{
					BST_FIELD_PROP_2NUMBER1(gci_three_minus_max_chroma_format_constraint_idc, 4, "");
				}
				BST_FIELD_PROP_BOOL(gci_no_mixed_nalu_types_in_pic_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_trail_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_stsa_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_rasl_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_radl_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_idr_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_cra_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_gdr_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_aps_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_idr_rpl_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_one_tile_per_pic_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_pic_header_in_slice_header_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_one_slice_per_pic_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_rectangular_slice_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_one_slice_per_subpic_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_subpic_info_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_three_minus_max_log2_ctu_size_constraint_idc, "", "");
				BST_FIELD_PROP_BOOL(gci_no_partition_constraints_override_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_mtt_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_qtbtt_dual_tree_intra_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_palette_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_ibc_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_isp_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_mrl_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_mip_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_cclm_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_ref_pic_resampling_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_res_change_in_clvs_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_weighted_prediction_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_ref_wraparound_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_temporal_mvp_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_sbtmvp_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_amvr_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_bdof_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_smvd_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_dmvr_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_mmvd_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_affine_motion_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_prof_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_bcw_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_ciip_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_gpm_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_luma_transform_size_64_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_transform_skip_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_bdpcm_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_mts_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_lfnst_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_joint_cbcr_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_sbt_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_act_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_explicit_scaling_list_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_dep_quant_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_sign_data_hiding_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_cu_qp_delta_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_chroma_qp_offset_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_sao_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_alf_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_ccalf_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_lmcs_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_ladf_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_no_virtual_boundaries_constraint_flag, "", "");
				BST_FIELD_PROP_BOOL(gci_num_reserved_bits, "", "");

				NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "for(i=0;i&lt;gci_num_reserved_bits;i++)", "");
				for (i = 0; i < gci_num_reserved_bits; i++) {
					BST_ARRAY_FIELD_PROP_NUMBER1(gci_reserved_zero_bit, i, 1, "");
				}
				NAV_WRITE_TAG_END("Tag0");

				BST_FIELD_PROP_NUMBER_BITARRAY1(gci_alignment_zero_bit, "");
			}
		DECLARE_FIELDPROP_END()

		//
		// profile_tier_level()
		//
		int NAL_UNIT::PROFILE_TIER_LEVEL::Map(AMBst in_bst)
		{
			SYNTAX_BITSTREAM_MAP::Map(in_bst);
			try
			{
				uint8_t idx = 0;
				MAP_BST_BEGIN(0);

				if (m_profileTierPresentFlag) {
					bsrb1(in_bst, general_profile_idc, 7);
					bsrb1(in_bst, general_tier_flag, 1);
				}

				bsrb1(in_bst, general_level_idc, 8);
				bsrb1(in_bst, ptl_frame_only_constraint_flag, 1);
				bsrb1(in_bst, ptl_multilayer_enabled_flag, 1);

				if (m_profileTierPresentFlag) {
					nal_read_obj(in_bst, general_constraints_info);
				}

				for (int i = m_MaxNumSubLayersMinus1 - 1; i >= 0; i--) {
					bsrbarray(in_bst, ptl_sublayer_level_present_flag, i);
				}

				idx = 0;
				while (!AMBst_IsAligned(in_bst)) {
					bsrbarray(in_bst, ptl_reserved_zero_bit, idx);
					idx++;
				}

				sublayer_level_idc.resize((size_t)((int64_t)m_MaxNumSubLayersMinus1 + 1));
				sublayer_level_idc[m_MaxNumSubLayersMinus1] = general_level_idc;

				for (int i = m_MaxNumSubLayersMinus1 - 1; i >= 0; i--) {
					if (ptl_sublayer_level_present_flag[i]) {
						bsrb1(in_bst, sublayer_level_idc[i], 8);
					}
					else
					{
						sublayer_level_idc[i] = sublayer_level_idc[i + 1];
					}
				}

				if (m_profileTierPresentFlag) {
					bsrb1(in_bst, ptl_num_sub_profiles, 8);
					if (ptl_num_sub_profiles > 0)
					{
						general_sub_profile_idc.resize(ptl_num_sub_profiles);
						for (int i = 0; i < ptl_num_sub_profiles; i++) {
							bsrb1(in_bst, general_sub_profile_idc[i], 32);
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

		int NAL_UNIT::PROFILE_TIER_LEVEL::Unmap(AMBst out_bst)
		{
			UNREFERENCED_PARAMETER(out_bst);
			return RET_CODE_ERROR_NOTIMPL;
		}

		DECLARE_FIELDPROP_BEGIN1(NAL_UNIT::PROFILE_TIER_LEVEL)
			char szLevelName[32] = { 0 };
			if (m_profileTierPresentFlag)
			{
				BST_FIELD_PROP_2NUMBER1(general_profile_idc, 7, vvc_profile_name[GetVVCProfile()]);
				BST_FIELD_PROP_NUMBER1(general_tier_flag, 1, general_tier_flag ? "High tier" : "Main tier");
			}

			BST_FIELD_PROP_2NUMBER1_F(general_level_idc, 8, "Level %d.%d", general_level_idc / 16, general_level_idc % 16 / 3);
			BST_FIELD_PROP_BOOL(ptl_frame_only_constraint_flag, 
				"sps_field_seq_flag for all pictures in OlsInScope shall be equal to 0", 
				"sps_field_seq_flag for some pictures in OlsInScope shall NOT be equal to 0");
			BST_FIELD_PROP_BOOL(ptl_multilayer_enabled_flag, 
				"the CVSs of OlsInScope might contain more than one layer", 
				"all slices in OlsInScope shall have the same value of nuh_layer_id");
			if (m_profileTierPresentFlag)
			{
				NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "general_constraints_info()", "");
				BST_FIELD_PROP_OBJECT(general_constraints_info);
				NAV_WRITE_TAG_END("Tag0");
			}

			if (m_MaxNumSubLayersMinus1 > 0) {
				NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("Tag0", "for(i=MaxNumSubLayersMinus1(%d)-1;i&gt;=0;i--)", "", m_MaxNumSubLayersMinus1);
				for (i = m_MaxNumSubLayersMinus1 - 1; i >= 0; i--) {
					BST_ARRAY_FIELD_PROP_NUMBER1(ptl_sublayer_level_present_flag, i, 1, "");
				}
				NAV_WRITE_TAG_END("Tag0");
			}

			NAV_WRITE_TAG_BEGIN("ptl_reserved_zero_bit");
			for (i = 0; i <= ptl_reserved_zero_bit.UpperBound(); i++) {
				BST_ARRAY_FIELD_PROP_NUMBER1(ptl_reserved_zero_bit, i, 1, "");
			}
			NAV_WRITE_TAG_END("ptl_reserved_zero_bit");

			MBCSPRINTF_S(szLevelName, _countof(szLevelName), "Level %d.%d", sublayer_level_idc[m_MaxNumSubLayersMinus1]/16, sublayer_level_idc[m_MaxNumSubLayersMinus1]%16/3);
			NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag00", "sublayer_level_idc[%d]", sublayer_level_idc[m_MaxNumSubLayersMinus1], szLevelName, m_MaxNumSubLayersMinus1);
			if (m_MaxNumSubLayersMinus1 > 0) {
				NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("Tag0", "for(i=MaxNumSubLayersMinus1(%d)-1;i&gt;=0;i--)", "", m_MaxNumSubLayersMinus1);
				for (i = m_MaxNumSubLayersMinus1 - 1; i >= 0; i--) {
					MBCSPRINTF_S(szLevelName, _countof(szLevelName), "Level %d.%d", sublayer_level_idc[i] / 16, sublayer_level_idc[i] % 16 / 3);
					if (ptl_sublayer_level_present_flag[i]) {
						BST_ARRAY_FIELD_PROP_NUMBER1(sublayer_level_idc, i, 1, "");
					}
					else
					{
						NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag00", "sublayer_level_idc[%d]", sublayer_level_idc[i], "", i);
					}
				}
				NAV_WRITE_TAG_END("Tag0");
			}

			if (m_profileTierPresentFlag) {
				BST_FIELD_PROP_2NUMBER1(ptl_num_sub_profiles, 8, "the number of the general_sub_profile_idc syntax elements");
				if (ptl_num_sub_profiles > 0) {
					NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "for(i=0;i&lt;ptl_num_sub_profiles;i++)", "");
					for (i = 0; i < ptl_num_sub_profiles; i++) {
						BST_ARRAY_FIELD_PROP_NUMBER1(general_sub_profile_idc, i, 32, vvc_profile_name[GetVVCProfile(general_sub_profile_idc[i])]);
					}
					NAV_WRITE_TAG_END("Tag0");
				}
			}

		DECLARE_FIELDPROP_END()

		//
		// dpb_parameters()
		//
		int NAL_UNIT::DPB_PARAMETERS::Map(AMBst in_bst)
		{
			int iRet = RET_CODE_SUCCESS;
			SYNTAX_BITSTREAM_MAP::Map(in_bst);

			try
			{
				MAP_BST_BEGIN(0);
				dpb_max_dec_pic_buffering_minus1.resize((size_t)m_MaxSubLayersMinus1 + 1);
				dpb_max_num_reorder_pics.resize((size_t)m_MaxSubLayersMinus1 + 1);
				dpb_max_latency_increase_plus1.resize((size_t)m_MaxSubLayersMinus1 + 1);
				if (m_subLayerInfoFlag)
				{
					for (int i = 0; i <= m_MaxSubLayersMinus1; i++) {
						nal_read_ue(in_bst, dpb_max_dec_pic_buffering_minus1[i], uint32_t);
						nal_read_ue(in_bst, dpb_max_num_reorder_pics[i], uint32_t);
						nal_read_ue(in_bst, dpb_max_latency_increase_plus1[i], uint32_t);
					}
				}
				else
				{
					nal_read_ue(in_bst, dpb_max_dec_pic_buffering_minus1[m_MaxSubLayersMinus1], uint32_t);
					nal_read_ue(in_bst, dpb_max_num_reorder_pics[m_MaxSubLayersMinus1], uint32_t);
					nal_read_ue(in_bst, dpb_max_latency_increase_plus1[m_MaxSubLayersMinus1], uint32_t);
				}
				MAP_BST_END();
			}
			catch (AMException e)
			{
				return e.RetCode();
			}

			return RET_CODE_SUCCESS;
		}

		int NAL_UNIT::DPB_PARAMETERS::Unmap(AMBst out_bst)
		{
			UNREFERENCED_PARAMETER(out_bst);
			return RET_CODE_ERROR_NOTIMPL;
		}

		DECLARE_FIELDPROP_BEGIN1(NAL_UNIT::DPB_PARAMETERS)
		NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "for(i=(subLayerInfoFlag?0:MaxSubLayersMinus1);i&lt;=MaxSubLayersMinus1;i++)", "");
		for (i = (m_subLayerInfoFlag ? 0 : m_MaxSubLayersMinus1); i <= m_MaxSubLayersMinus1; i++) {
			BST_ARRAY_FIELD_PROP_UE(dpb_max_dec_pic_buffering_minus1, i, "plus 1 specifies the maximum required size of the DPB");
			BST_ARRAY_FIELD_PROP_UE(dpb_max_num_reorder_pics, i, "the maximum allowed number of pictures of the OLS that can precede any picture in the OLS in decoding order");
			BST_ARRAY_FIELD_PROP_UE(dpb_max_latency_increase_plus1, i, "");
			if (dpb_max_latency_increase_plus1[i] > 0) {
				NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("MaxLatencyPictures", "MaxLatencyPictures[%d]", 
					dpb_max_num_reorder_pics[i] + dpb_max_latency_increase_plus1[i] - 1, "", i);
			}
		}
		NAV_WRITE_TAG_END("Tag0");
		DECLARE_FIELDPROP_END()

		//
		// general_timing_hrd_parameters()
		//
		int NAL_UNIT::GENERAL_TIMING_HRD_PARAMETERS::Map(AMBst in_bst)
		{
			int iRet = RET_CODE_SUCCESS;
			SYNTAX_BITSTREAM_MAP::Map(in_bst);

			try
			{
				MAP_BST_BEGIN(0);
				bsrb1(in_bst, num_units_in_tick, 32);
				bsrb1(in_bst, time_scale, 32);
				bsrb1(in_bst, general_nal_hrd_params_present_flag, 1);
				bsrb1(in_bst, general_vcl_hrd_params_present_flag, 1);
				if (general_nal_hrd_params_present_flag || general_vcl_hrd_params_present_flag) {
					bsrb1(in_bst, general_same_pic_timing_in_all_ols_flag, 1);
					bsrb1(in_bst, general_du_hrd_params_present_flag, 1);

					if (general_du_hrd_params_present_flag) {
						bsrb1(in_bst, tick_divisor_minus2, 8);
					}

					bsrb1(in_bst, bit_rate_scale, 4);
					bsrb1(in_bst, cpb_size_scale, 4);

					if (general_du_hrd_params_present_flag) {
						bsrb1(in_bst, cpb_size_du_scale, 4);
					}

					nal_read_ue(in_bst, hrd_cpb_cnt_minus1, uint8_t);
				}
				MAP_BST_END();
			}
			catch (AMException e)
			{
				return e.RetCode();
			}

			return RET_CODE_SUCCESS;
		}

		int NAL_UNIT::GENERAL_TIMING_HRD_PARAMETERS::Unmap(AMBst out_bst)
		{
			UNREFERENCED_PARAMETER(out_bst);
			return RET_CODE_ERROR_NOTIMPL;
		}

		DECLARE_FIELDPROP_BEGIN1(NAL_UNIT::GENERAL_TIMING_HRD_PARAMETERS)
			BST_FIELD_PROP_2NUMBER1(num_units_in_tick, 32, "");
			BST_FIELD_PROP_2NUMBER1(time_scale, 32, "");

			BST_FIELD_PROP_BOOL(general_nal_hrd_params_present_flag, "", "");
			BST_FIELD_PROP_BOOL(general_vcl_hrd_params_present_flag, "", "");
			if (general_nal_hrd_params_present_flag || general_vcl_hrd_params_present_flag) {
				BST_FIELD_PROP_BOOL(general_same_pic_timing_in_all_ols_flag, "", "");
				BST_FIELD_PROP_BOOL_BEGIN(general_du_hrd_params_present_flag, "", "");
					BST_FIELD_PROP_2NUMBER1(tick_divisor_minus2, 8, "");
				BST_FIELD_PROP_BOOL_END("general_du_hrd_params_present_flag");

				BST_FIELD_PROP_2NUMBER1(bit_rate_scale, 4, "");
				BST_FIELD_PROP_2NUMBER1(cpb_size_scale, 4, "");

				if (general_du_hrd_params_present_flag) {
					BST_FIELD_PROP_2NUMBER1(cpb_size_du_scale, 4, "");
				}
				
				BST_FIELD_PROP_UE(hrd_cpb_cnt_minus1, "plus 1 specifies the number of alternative CPB delivery schedules");
			}
		DECLARE_FIELDPROP_END()

		//
		// sublayer_hrd_parameters()
		//
		int NAL_UNIT::SUBLAYER_HRD_PARAMETERS::Map(AMBst in_bst)
		{
			int iRet = RET_CODE_SUCCESS;
			SYNTAX_BITSTREAM_MAP::Map(in_bst);

			try
			{
				MAP_BST_BEGIN(0);
				parameters.resize((size_t)m_pTimingHRDParameters->hrd_cpb_cnt_minus1 + 1);
				for (int j = 0; j <= m_pTimingHRDParameters->hrd_cpb_cnt_minus1; j++) {
					nal_read_ue(in_bst, parameters[j].bit_rate_value_minus1, uint32_t);
					nal_read_ue(in_bst, parameters[j].cpb_size_value_minus1, uint32_t);

					if (m_pTimingHRDParameters->general_du_hrd_params_present_flag)
					{
						nal_read_ue(in_bst, parameters[j].cpb_size_du_value_minus1, uint32_t);
						nal_read_ue(in_bst, parameters[j].bit_rate_du_value_minus1, uint32_t);
					}

					bsrb1(in_bst, parameters[j].cbr_flag, 1);
				}
				MAP_BST_END();
			}
			catch (AMException e)
			{
				return e.RetCode();
			}

			return RET_CODE_SUCCESS;
		}

		int NAL_UNIT::SUBLAYER_HRD_PARAMETERS::Unmap(AMBst out_bst)
		{
			UNREFERENCED_PARAMETER(out_bst);
			return RET_CODE_ERROR_NOTIMPL;
		}

		DECLARE_FIELDPROP_BEGIN1(NAL_UNIT::SUBLAYER_HRD_PARAMETERS)
		NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("Tag0", "for(j=0;j&lt;=hrd_cpb_cnt_minus1:%d;j++ )", "", m_pTimingHRDParameters->hrd_cpb_cnt_minus1);
		for (int j = 0; j <= m_pTimingHRDParameters->hrd_cpb_cnt_minus1; j++) {
			BST_ARRAY_FIELD_PROP_NUMBER("bit_rate_value_minus1", j,
				(long long)quick_log2(parameters[j].bit_rate_value_minus1 + 1) * 2 + 1, parameters[j].bit_rate_value_minus1, "")
			NAV_WRITE_TAG_WITH_VALUEFMTSTR("BitRate", "%sbps", "", 
				GetReadableNum(((uint64_t)parameters[j].bit_rate_value_minus1 + 1) << (6 + m_pTimingHRDParameters->bit_rate_scale)).c_str());
			BST_ARRAY_FIELD_PROP_NUMBER("cpb_size_value_minus1", j,
				(long long)quick_log2(parameters[j].cpb_size_value_minus1 + 1) * 2 + 1, parameters[j].cpb_size_value_minus1, "")
			NAV_WRITE_TAG_WITH_VALUEFMTSTR("CpbSize", "%sbit", "", 
				GetReadableNum(((uint64_t)parameters[j].cpb_size_value_minus1 + 1) << (4 + m_pTimingHRDParameters->cpb_size_scale)).c_str());
			if (m_pTimingHRDParameters->general_du_hrd_params_present_flag)
			{
				BST_ARRAY_FIELD_PROP_NUMBER("cpb_size_du_value_minus1", j,
					(long long)quick_log2(parameters[j].cpb_size_du_value_minus1 + 1) * 2 + 1, parameters[j].cpb_size_du_value_minus1, "")
				BST_ARRAY_FIELD_PROP_NUMBER("bit_rate_du_value_minus1", j,
					(long long)quick_log2(parameters[j].bit_rate_du_value_minus1 + 1) * 2 + 1, parameters[j].bit_rate_du_value_minus1, "")
			}
			BST_FIELD_PROP_BOOL1(parameters[i], cbr_flag, "", "");
		}
		NAV_WRITE_TAG_END("Tag0");
		DECLARE_FIELDPROP_END()

		REF_PIC_LIST_STRUCT::REF_PIC_LIST_STRUCT(uint8_t listIdx, uint8_t rplsIdx, void* seq_parameter_set_rbsp)
			: ltrp_in_header_flag(1), m_listIdx(listIdx), m_rplsIdx(rplsIdx) {

			NAL_UNIT::SEQ_PARAMETER_SET_RBSP* ptr_seq_parameter_set_rbsp = (NAL_UNIT::SEQ_PARAMETER_SET_RBSP*)seq_parameter_set_rbsp;

			current_sps_num_ref_pic_list = ptr_seq_parameter_set_rbsp->sps_num_ref_pic_lists[m_listIdx];
			sps_long_term_ref_pics_flag = ptr_seq_parameter_set_rbsp->sps_long_term_ref_pics_flag;
			sps_inter_layer_prediction_enabled_flag = ptr_seq_parameter_set_rbsp->sps_inter_layer_prediction_enabled_flag;
			sps_weighted_pred_flag = ptr_seq_parameter_set_rbsp->sps_weighted_pred_flag;
			sps_weighted_bipred_flag = ptr_seq_parameter_set_rbsp->sps_weighted_bipred_flag;
			sps_log2_max_pic_order_cnt_lsb_minus4 = ptr_seq_parameter_set_rbsp->sps_log2_max_pic_order_cnt_lsb_minus4;
		}

		int REF_PIC_LIST_STRUCT::Map(AMBst in_bst)
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

							for (i = 0, m_pCtx->NumLtrpEntries[m_listIdx][m_rplsIdx] = 0; i < num_ref_entries; i++) {
								if (!inter_layer_ref_pic_flag[i] && !st_ref_pic_flag[i])
									m_pCtx->NumLtrpEntries[m_listIdx][m_rplsIdx]++;
							}
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
						else if (!ltrp_in_header_flag)
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

		int REF_PIC_LIST_STRUCT::Unmap(AMBst out_bst)
		{
			UNREFERENCED_PARAMETER(out_bst);
			return RET_CODE_ERROR_NOTIMPL;
		}

		DECLARE_FIELDPROP_BEGIN1(REF_PIC_LIST_STRUCT)
			BST_FIELD_PROP_UE(num_ref_entries, "the number of entries in the ref_pic_list_struct");
			if (sps_long_term_ref_pics_flag && m_rplsIdx < current_sps_num_ref_pic_list && num_ref_entries > 0) {
				BST_FIELD_PROP_BOOL(ltrp_in_header_flag, "POC LSBs of the LTRP entries are not present", "POC LSBs of the LTRP entries are present");
			}
			if (num_ref_entries > 0) {
				NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "for(i=0,j=0;i&lt;num_ref_entries;i++)", "");
				for (int i = 0, j = 0; i < num_ref_entries; i++) {
					BST_ARRAY_FIELD_PROP_NUMBER1(inter_layer_ref_pic_flag, i, 1, inter_layer_ref_pic_flag[i] ? "an ILRP entry" : "not an ILRP entry");
					if (!inter_layer_ref_pic_flag[i]) {
						if (sps_long_term_ref_pics_flag) {
							BST_ARRAY_FIELD_PROP_NUMBER1(st_ref_pic_flag, i, 1, "");
						}
						if (st_ref_pic_flag[i]) {
							BST_ARRAY_FIELD_PROP_UE(abs_delta_poc_st, i, "");
						
							uint32_t AbsDeltaPocSt = (uint32_t)abs_delta_poc_st[i] + 1;
							if ((sps_weighted_pred_flag || sps_weighted_bipred_flag) && i != 0)
								AbsDeltaPocSt = (uint32_t)abs_delta_poc_st[i];

							if (AbsDeltaPocSt > 0) {
								BST_ARRAY_FIELD_PROP_NUMBER1(strp_entry_sign_flag, i, 1,
									strp_entry_sign_flag[i] ? "DeltaPocValSt[listIdx][rplsIdx] is less than 0"
															: "DeltaPocValSt[listIdx][rplsIdx] is greater than or equal to 0");
							}
						}
						else if (!ltrp_in_header_flag) {
							BST_ARRAY_FIELD_PROP_NUMBER1(rpls_poc_lsb_lt, j, (long long)sps_log2_max_pic_order_cnt_lsb_minus4 + 4, 
								"the value of the picture order count modulo MaxPicOrderCntLsb of the picture referred to by the i-th entry");
							j++;
						}
						else
						{
							BST_ARRAY_FIELD_PROP_UE(ilrp_idx, i, "the index, to the list of the direct reference layers, of the ILRP entry of the i-th entry");
						}
					}
				}
				NAV_WRITE_TAG_END("Tag0");
			}
		DECLARE_FIELDPROP_END()

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

					layerIncludedInOlsFlag[i].resize((size_t)vps_max_layers_minus1 + 1);
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
							vps_ols_output_layer_flag.resize((size_t)vps_num_output_layer_sets_minus2 + 2);
							for (int i = 1; i <= (int)vps_num_output_layer_sets_minus2 + 1; i++){
								for (int j = 0; j <= (int)vps_max_layers_minus1; j++){
									bsrbarray(in_bst, vps_ols_output_layer_flag[i], j);
								}
							}
						}
					}

					bsrb1(in_bst, vps_num_ptls_minus1, 8);
				}

				vps_ptl_max_tid.resize((size_t)vps_num_ptls_minus1 + 1);
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
						if (AMP_FAILED(iRet = read_extension_and_trailing_bits(in_bst, map_status, vps_extension_data_flag, rbsp_trailing_bits)))
						{
							return iRet;
						}
					}
					else
					{
						nal_read_obj(in_bst, rbsp_trailing_bits);
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

		int NAL_UNIT::VIDEO_PARAMETER_SET_RBSP::Unmap(AMBst out_bst)
		{
			UNREFERENCED_PARAMETER(out_bst);
			return RET_CODE_ERROR_NOTIMPL;
		}

		DECLARE_FIELDPROP_BEGIN1(NAL_UNIT::VIDEO_PARAMETER_SET_RBSP)

		DECLARE_FIELDPROP_END()

		//
		// seq_parameter_set_rbsp()
		//
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
			, sps_timing_hrd_params_present_flag(0)
			, sps_sublayer_cpb_params_present_flag(0)
		{
		}

		NAL_UNIT::SEQ_PARAMETER_SET_RBSP::~SEQ_PARAMETER_SET_RBSP() {
			AMP_SAFEDEL2(profile_tier_level);
			AMP_SAFEDEL2(dpb_parameters);
			AMP_SAFEDEL2(general_timing_hrd_parameters);
			AMP_SAFEDEL2(ols_timing_hrd_parameters);
			AMP_SAFEDEL2(vui_payload);
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
					sps_subpic_ctu_top_left_x.resize((size_t)sps_num_subpics_minus1 + 1);
					sps_subpic_ctu_top_left_y.resize((size_t)sps_num_subpics_minus1 + 1);
					sps_subpic_width_minus1.resize((size_t)sps_num_subpics_minus1 + 1);
					sps_subpic_height_minus1.resize((size_t)sps_num_subpics_minus1 + 1);
					for (int i = 0; sps_num_subpics_minus1 > 0 && i <= sps_num_subpics_minus1; i++) {
						if (!sps_subpic_same_size_flag || i == 0) {
							if (i > 0 && sps_pic_width_max_in_luma_samples > CtbSizeY) {
								bsrb1(in_bst, sps_subpic_ctu_top_left_x[i], quick_ceil_log2(tmpWidthVal));
							}
							else
								sps_subpic_ctu_top_left_x[i] = 0;

							if (i > 0 && sps_pic_height_max_in_luma_samples > CtbSizeY) {
								bsrb1(in_bst, sps_subpic_ctu_top_left_y[i], quick_ceil_log2(tmpHeightVal));
							}
							else
								sps_subpic_ctu_top_left_y[i] = 0;

							if (i < sps_num_subpics_minus1 && sps_pic_width_max_in_luma_samples > CtbSizeY) {
								bsrb1(in_bst, sps_subpic_width_minus1[i], quick_ceil_log2(tmpWidthVal));
							}
							else
								sps_subpic_width_minus1[i] = tmpWidthVal - sps_subpic_ctu_top_left_x[i] - 1;

							if (i < sps_num_subpics_minus1 && sps_pic_height_max_in_luma_samples > CtbSizeY) {
								bsrb1(in_bst, sps_subpic_height_minus1[i], quick_ceil_log2(tmpHeightVal));
							}
							else
								sps_subpic_height_minus1[i] = tmpHeightVal - sps_subpic_ctu_top_left_y[i] - 1;
						}
						else
						{
							uint32_t numSubpicCols = tmpWidthVal / (sps_subpic_width_minus1[0] + 1);
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
							sps_subpic_id.resize((size_t)sps_num_subpics_minus1 + 1);
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
				for (int i = 0; i < (int)((size_t)sps_num_extra_ph_bytes * 8); i++) {
					bsrbarray(in_bst, sps_extra_ph_bit_present_flag, i);
					if (sps_extra_ph_bit_present_flag[i])
						NumExtraPhBits++;
				}
				
				bsrb1(in_bst, sps_num_extra_sh_bytes, 2);

				NumExtraShBits = 0;
				for (int i = 0; i < (int)((uint32_t)sps_num_extra_sh_bytes * 8); i++) {
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
					for (uint8_t j = 0; j < (uint8_t)sps_num_ref_pic_lists[i]; j++) {
						REF_PIC_LIST_STRUCT* refListStructure = nullptr;
						bsrbreadref(in_bst, refListStructure, REF_PIC_LIST_STRUCT, i, j, this);
						ref_pic_list_struct[i][j] = std::shared_ptr<REF_PIC_LIST_STRUCT>(refListStructure);
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
					while (!AMBst_IsAligned(in_bst)) {
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
					if (AMP_FAILED(iRet = read_extension_and_trailing_bits(in_bst, map_status, sps_extension_data_flag, rbsp_trailing_bits)))
					{
						return iRet;
					}
				}
				else
				{
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

		int NAL_UNIT::SEQ_PARAMETER_SET_RBSP::Unmap(AMBst out_bst)
		{
			UNREFERENCED_PARAMETER(out_bst);
			return RET_CODE_ERROR_NOTIMPL;
		}

		DECLARE_FIELDPROP_BEGIN1(NAL_UNIT::SEQ_PARAMETER_SET_RBSP)
		NAV_WRITE_TAG_BEGIN_WITH_ALIAS("seq_parameter_set_rbsp", "seq_parameter_set_rbsp()", "");
			BST_FIELD_PROP_2NUMBER1(sps_seq_parameter_set_id, 4, 
				"an identifier for the SPS for reference by other syntax elements");
			BST_FIELD_PROP_2NUMBER1(sps_video_parameter_set_id, 4,
				sps_video_parameter_set_id > 0 ? "the value of vps_video_parameter_set_id for the VPS referred to by the SPS" : "");
			BST_FIELD_PROP_2NUMBER1(sps_max_sublayers_minus1, 3, 
				"plus 1 specifies the maximum number of temporal sublayers that could be present in each CLVS referring to the SPS");
			BST_FIELD_PROP_2NUMBER1(sps_chroma_format_idc, 2, 
				sps_chroma_format_idc == 0 ? "Monochrome" : (
				sps_chroma_format_idc == 1 ? "4:2:0" : (
				sps_chroma_format_idc == 2 ? "4:2:2" : (
				sps_chroma_format_idc == 3 ? "4:4:4" : "Unknown"))));
			BST_FIELD_PROP_2NUMBER1(sps_log2_ctu_size_minus5, 2, "plus 5 specifies the luma coding tree block size of each CTU");
			uint8_t CtbLog2SizeY = sps_log2_ctu_size_minus5 + 5;
			uint32_t CtbSizeY = 1 << CtbLog2SizeY;
			NAV_WRITE_TAG_WITH_NUMBER_VALUE1(CtbLog2SizeY, "");
			NAV_WRITE_TAG_WITH_NUMBER_VALUE1(CtbSizeY, "");
			BST_FIELD_PROP_BOOL(sps_ptl_dpb_hrd_params_present_flag, 
				"a profile_tier_level() syntax structure and a dpb_parameters() syntax structure are present in the SPS, "
				"and a general_timing_hrd_parameters() syntax structure and an ols_timing_hrd_parameters() syntax structure could also be present in the SPS", 
				"none of these four syntax structures is present in the SPS");

			if (sps_ptl_dpb_hrd_params_present_flag) {
				NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("profile_tier_level", "profile_tier_level(1, sps_max_sublayers_minus1(%d))", "", sps_max_sublayers_minus1);
				BST_FIELD_PROP_REF(profile_tier_level);
				NAV_WRITE_TAG_END("profile_tier_level");
			}

			BST_FIELD_PROP_BOOL(sps_gdr_enabled_flag,
				"GDR pictures are enabled and could be present in the CLVS",
				"GDR pictures are disabled and not present in the CLVS");
			BST_FIELD_PROP_BOOL(sps_ref_pic_resampling_enabled_flag, 
				"reference picture resampling is enabled", 
				"reference picture resampling is disabled");

			if (sps_ref_pic_resampling_enabled_flag) {
				BST_FIELD_PROP_BOOL(sps_res_change_in_clvs_allowed_flag,
					"the picture spatial resolution might change within a CLVS",
					"the picture spatial resolution does not change within any CLVS");
			}

			BST_FIELD_PROP_UE(sps_pic_width_max_in_luma_samples, "the maximum width, in units of luma samples, of each decoded picture");
			BST_FIELD_PROP_UE(sps_pic_height_max_in_luma_samples, "the maximum height, in units of luma samples, of each decoded picture");

			BST_FIELD_PROP_BOOL_BEGIN(sps_conformance_window_flag, 
				"the conformance cropping window offset parameters follow", 
				"the conformance cropping window offset parameters are not present");

			if (sps_conformance_window_flag) {
				BST_FIELD_PROP_UE(sps_conf_win_left_offset, "");
				BST_FIELD_PROP_UE(sps_conf_win_right_offset, "");
				BST_FIELD_PROP_UE(sps_conf_win_top_offset, "");
				BST_FIELD_PROP_UE(sps_conf_win_bottom_offset, "");
			}

			BST_FIELD_PROP_BOOL_END("sps_conformance_window_flag");

			BST_FIELD_PROP_BOOL_BEGIN(sps_subpic_info_present_flag,
				"subpicture information is present for the CLVS",
				"subpicture information is not present for the CLVS");

			if (sps_subpic_info_present_flag) {
				BST_FIELD_PROP_UE(sps_num_subpics_minus1, "plus 1 specifies the number of subpictures in each picture in the CLVS");
				if (sps_num_subpics_minus1 > 0) {
					BST_FIELD_PROP_BOOL(sps_independent_subpics_flag, 
						"all subpicture boundaries in the CLVS are treated as picture boundaries and there is no loop filtering across the subpicture boundaries", 
						"");
					BST_FIELD_PROP_BOOL(sps_subpic_same_size_flag, 
						"all subpictures in the CLVS have the same width specified by sps_subpic_width_minus1[0] and the same height specified by sps_subpic_height_minus1[0]", 
						"");
				}

				uint32_t tmpWidthVal = sps_pic_width_max_in_luma_samples + CtbSizeY - 1;
				uint32_t tmpHeightVal = (sps_pic_height_max_in_luma_samples + CtbSizeY - 1) / CtbSizeY;
				if (sps_num_subpics_minus1 > 0) {
					NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0",
						"for(i=0;sps_num_subpics_minus1&gt;0 && i&lt;=sps_num_subpics_minus1;i++)", "");
					for (i = 0; sps_num_subpics_minus1 > 0 && i <= sps_num_subpics_minus1; i++) {
						if (!sps_subpic_same_size_flag || i == 0) {
							if (i > 0 && sps_pic_width_max_in_luma_samples > CtbSizeY) {
								BST_ARRAY_FIELD_PROP_NUMBER1(sps_subpic_ctu_top_left_x, i, quick_ceil_log2(tmpWidthVal), "");
							}

							if (i > 0 && sps_pic_height_max_in_luma_samples > CtbSizeY) {
								BST_ARRAY_FIELD_PROP_NUMBER1(sps_subpic_ctu_top_left_y, i, quick_ceil_log2(tmpHeightVal), "");
							}

							if (i < sps_num_subpics_minus1 && sps_pic_width_max_in_luma_samples > CtbSizeY) {
								BST_ARRAY_FIELD_PROP_NUMBER1(sps_subpic_width_minus1, i, quick_ceil_log2(tmpWidthVal), "");
							}

							if (i < sps_num_subpics_minus1 && sps_pic_height_max_in_luma_samples > CtbSizeY) {
								BST_ARRAY_FIELD_PROP_NUMBER1(sps_subpic_height_minus1, i, quick_ceil_log2(tmpHeightVal), "");
							}
						}

						if (!sps_independent_subpics_flag) {
							BST_ARRAY_FIELD_PROP_NUMBER1(sps_subpic_treated_as_pic_flag, i, 1, "");
							BST_ARRAY_FIELD_PROP_NUMBER1(sps_loop_filter_across_subpic_enabled_flag, i, 1, "");
						}
					}
					NAV_WRITE_TAG_END("Tag0");

					BST_FIELD_PROP_UE(sps_subpic_id_len_minus1, "plus 1 specifies the number of bits used to represent the syntax element sps_subpic_id");

					BST_FIELD_PROP_BOOL_BEGIN(sps_subpic_id_mapping_explicitly_signalled_flag, 
						"the subpicture ID mapping is explicitly signalled", 
						"the subpicture ID mapping is not explicitly signalled for the CLVS");

					if (sps_subpic_id_mapping_explicitly_signalled_flag) {
						BST_FIELD_PROP_BOOL_BEGIN(sps_subpic_id_mapping_present_flag, 
							"the subpicture ID mapping is signalled in the SPS", 
							"subpicture ID mapping is signalled in the PPSs referred to by coded pictures of the CLVS");

						NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag00", "for(i=0;i&lt;=sps_num_subpics_minus1;i++)", "");
						for (i = 0; i <= sps_num_subpics_minus1; i++) {
							BST_ARRAY_FIELD_PROP_NUMBER1(sps_subpic_id, i, (long long)sps_subpic_id_len_minus1 + 1, "");
						}
						NAV_WRITE_TAG_END("Tag00");

						BST_FIELD_PROP_BOOL_END("sps_subpic_id_mapping_present_flag");
					}

					BST_FIELD_PROP_BOOL_END("sps_subpic_id_mapping_explicitly_signalled_flag");
				}
			}

			BST_FIELD_PROP_UE_F(sps_bitdepth_minus8, "BitDepth: %d", sps_bitdepth_minus8 + 8);

			BST_FIELD_PROP_BOOL_END("sps_subpic_info_present_flag");

			BST_FIELD_PROP_BOOL(sps_entropy_coding_sync_enabled_flag, "", "");
			BST_FIELD_PROP_BOOL(sps_entry_point_offsets_present_flag, "", "");
			BST_FIELD_PROP_2NUMBER1_F(sps_log2_max_pic_order_cnt_lsb_minus4, 4, 
				"MaxPicOrderCntLsb: %" PRIu32 "", (uint32_t)(1U<<(sps_log2_max_pic_order_cnt_lsb_minus4 + 4)));

			BST_FIELD_PROP_BOOL_BEGIN(sps_poc_msb_cycle_flag, 
				"the ph_poc_msb_cycle_present_flag syntax element is present in PH syntax structures", 
				"the ph_poc_msb_cycle_present_flag syntax element is not present in PH syntax structures");

			if (sps_poc_msb_cycle_flag) {
				BST_FIELD_PROP_UE(sps_poc_msb_cycle_len_minus1, "plus 1 specifies the length, in bits, of the ph_poc_msb_cycle_val syntax elements")
			}

			BST_FIELD_PROP_BOOL_END("sps_poc_msb_cycle_flag");

			BST_FIELD_PROP_NUMBER_BEGIN("sps_num_extra_ph_bytes", 2, sps_num_extra_ph_bytes, "");
			for (i = 0; i < (int)((uint64_t)sps_num_extra_ph_bytes * 8); i++) {
				BST_ARRAY_FIELD_PROP_NUMBER1(sps_extra_ph_bit_present_flag, i, 1, "");
			}
			BST_FIELD_PROP_NUMBER_END("sps_num_extra_ph_bytes");

			BST_FIELD_PROP_NUMBER_BEGIN("sps_num_extra_sh_bytes", 2, sps_num_extra_sh_bytes, "");
			for (i = 0; i < (int)((uint64_t)sps_num_extra_sh_bytes * 8); i++) {
				BST_ARRAY_FIELD_PROP_NUMBER1(sps_extra_sh_bit_present_flag, i, 1, "");
			}
			BST_FIELD_PROP_NUMBER_END("sps_num_extra_sh_bytes");

			BST_FIELD_PROP_BOOL_BEGIN(sps_ptl_dpb_hrd_params_present_flag, "", "");
			if (sps_ptl_dpb_hrd_params_present_flag) {
				if (sps_max_sublayers_minus1 > 0) {
					BST_FIELD_PROP_BOOL(sps_sublayer_dpb_params_flag, "", "");
				}

				NAV_WRITE_TAG_BEGIN_WITH_ALIAS("dpb_parameters", "dpb_parameters(sps_max_sublayers_minus1, sps_sublayer_dpb_params_flag)", "");
				BST_FIELD_PROP_REF(dpb_parameters);
				NAV_WRITE_TAG_END("dpb_parameters");
			}
			BST_FIELD_PROP_BOOL_END("sps_ptl_dpb_hrd_params_present_flag");

			BST_FIELD_PROP_UE(sps_log2_min_luma_coding_block_size_minus2, "plus 2 specifies the minimum luma coding block size");

			uint8_t SubWidthC = (sps_chroma_format_idc == 0 || sps_chroma_format_idc == 3) ? 1 : 2;
			uint8_t SubHeightC = sps_chroma_format_idc == 1 ? 2 : 1;
			uint8_t MinCbLog2SizeY = sps_log2_min_luma_coding_block_size_minus2 + 2;
			uint32_t CtbWidthC = CtbSizeY / SubWidthC;
			uint32_t CtbHeightC = CtbSizeY / SubHeightC;

			NAV_WRITE_TAG_WITH_NUMBER_VALUE("MinCbLog2SizeY", MinCbLog2SizeY, "");
			NAV_WRITE_TAG_WITH_NUMBER_VALUE("MinCbSizeY", 1 << MinCbLog2SizeY, "");
			NAV_WRITE_TAG_WITH_NUMBER_VALUE("IbcBufWidthY", 256 * 128 / MinCbLog2SizeY, "");
			NAV_WRITE_TAG_WITH_NUMBER_VALUE("IbcBufWidthC", 256 * 128 / MinCbLog2SizeY / SubWidthC, "");
			NAV_WRITE_TAG_WITH_NUMBER_VALUE("VSize", AMP_MIN(64, CtbSizeY), "");
			NAV_WRITE_TAG_WITH_NUMBER_VALUE("CtbWidthC", CtbWidthC, "");
			NAV_WRITE_TAG_WITH_NUMBER_VALUE("CtbHeightC", CtbHeightC, "");

			BST_FIELD_PROP_BOOL(sps_partition_constraints_override_enabled_flag, "", "");
			BST_FIELD_PROP_UE(sps_log2_diff_min_qt_min_cb_intra_slice_luma, "");

			uint32_t MinQtLog2SizeIntraY = sps_log2_diff_min_qt_min_cb_intra_slice_luma + MinCbLog2SizeY;
			NAV_WRITE_TAG_WITH_NUMBER_VALUE("MinQtLog2SizeIntraY", MinQtLog2SizeIntraY, "");

			BST_FIELD_PROP_UE(sps_max_mtt_hierarchy_depth_intra_slice_luma, "");
			if (sps_max_mtt_hierarchy_depth_intra_slice_luma != 0) {
				BST_FIELD_PROP_UE(sps_log2_diff_max_bt_min_qt_intra_slice_luma, "");
				BST_FIELD_PROP_UE(sps_log2_diff_max_tt_min_qt_intra_slice_luma, "");
			}

			if (sps_chroma_format_idc != 0) {
				BST_FIELD_PROP_BOOL(sps_qtbtt_dual_tree_intra_flag, "", "");
			}

			if (sps_qtbtt_dual_tree_intra_flag) {
				BST_FIELD_PROP_UE(sps_log2_diff_min_qt_min_cb_intra_slice_chroma, "");
				uint32_t MinQtLog2SizeIntraC = sps_log2_diff_min_qt_min_cb_intra_slice_chroma + MinCbLog2SizeY;
				NAV_WRITE_TAG_WITH_NUMBER_VALUE("MinQtLog2SizeIntraC", MinQtLog2SizeIntraC, "");

				BST_FIELD_PROP_UE(sps_max_mtt_hierarchy_depth_intra_slice_chroma, "");
				if (sps_max_mtt_hierarchy_depth_intra_slice_chroma != 0) {
					BST_FIELD_PROP_UE(sps_log2_diff_max_bt_min_qt_intra_slice_chroma, "");
					BST_FIELD_PROP_UE(sps_log2_diff_max_tt_min_qt_intra_slice_chroma, "");
				}
			}

			BST_FIELD_PROP_UE(sps_log2_diff_min_qt_min_cb_inter_slice, "");
			uint32_t MinQtLog2SizeInterY = sps_log2_diff_min_qt_min_cb_inter_slice + MinCbLog2SizeY;
			NAV_WRITE_TAG_WITH_NUMBER_VALUE("MinQtLog2SizeInterY", MinQtLog2SizeInterY, "");

			BST_FIELD_PROP_UE(sps_max_mtt_hierarchy_depth_inter_slice, "");
			if (sps_max_mtt_hierarchy_depth_inter_slice != 0) {
				BST_FIELD_PROP_UE(sps_log2_diff_max_bt_min_qt_inter_slice, "");
				BST_FIELD_PROP_UE(sps_log2_diff_max_tt_min_qt_inter_slice, "");
			}

			if (CtbSizeY > 32) {
				BST_FIELD_PROP_BOOL(sps_max_luma_transform_size_64_flag, "", "");
			}

			BST_FIELD_PROP_BOOL_BEGIN(sps_transform_skip_enabled_flag, "", "");
			if (sps_transform_skip_enabled_flag) {
				BST_FIELD_PROP_UE(sps_log2_transform_skip_max_size_minus2, "");
				BST_FIELD_PROP_BOOL(sps_bdpcm_enabled_flag, "", "");
			}
			BST_FIELD_PROP_BOOL_END("sps_transform_skip_enabled_flag");

			BST_FIELD_PROP_BOOL_BEGIN(sps_mts_enabled_flag, "", "");
			if (sps_mts_enabled_flag) {
				BST_FIELD_PROP_BOOL(sps_explicit_mts_intra_enabled_flag, 
					"mts_idx could be present in the intra coding unit syntax of the CLVS", 
					"mts_idx is not present in the intra coding unit syntax of the CLVS");
				BST_FIELD_PROP_BOOL(sps_explicit_mts_inter_enabled_flag, 
					"mts_idx could be present in the inter coding unit syntax of the CLVS", 
					"mts_idx is not present in the inter coding unit syntax of the CLVS");
			}
			BST_FIELD_PROP_BOOL_END("sps_mts_enabled_flag");

			BST_FIELD_PROP_BOOL(sps_lfnst_enabled_flag, 
				"lfnst_idx could be present in intra coding unit syntax", 
				"lfnst_idx is not present in intra coding unit syntax");
			if (sps_chroma_format_idc != 0) {
				BST_FIELD_PROP_BOOL(sps_joint_cbcr_enabled_flag, "the joint coding of chroma residuals is enabled for the CLVS", 
					"the joint coding of chroma residuals is disabled for the CLVS");
				BST_FIELD_PROP_BOOL(sps_same_qp_table_for_chroma_flag, 
					"only one chroma QP mapping table is signalled and this table applies to Cb and Cr residuals and additionally to joint Cb-Cr residuals", 
					"chroma QP mapping tables, two for Cb and Cr, and one additional for joint Cb-Cr");
				uint8_t numQpTables = sps_same_qp_table_for_chroma_flag ? 1 : (sps_joint_cbcr_enabled_flag ? 3 : 2);
				NAV_WRITE_TAG_WITH_NUMBER_VALUE("numQpTables", numQpTables, "");
				NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag1", "for(i=0;i&lt;numQpTables;i++)", "");
				for (i = 0; i < numQpTables; i++) {
					BST_ARRAY_FIELD_PROP_SE(sps_qp_table_start_minus26, i, 
						"plus 26 specifies the starting luma and chroma QP used to describe the i-th chroma QP mapping table");
					BST_ARRAY_FIELD_PROP_SE(sps_num_points_in_qp_table_minus1, i, 
						"plus 1 specifies the number of points used to describe the i-th chroma QP mapping table");
					NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag10", "for(j=0;j&lt;=sps_num_points_in_qp_table_minus1[ i ];j++)", "");
					for (uint32_t j = 0; j <= sps_num_points_in_qp_table_minus1[i]; j++) {
						BST_2ARRAY_FIELD_PROP_UE(sps_delta_qp_in_val_minus1, i, j, 
							"a delta value used to derive the input coordinate of the j-th pivot point of the i-th chroma QP mapping table");
						BST_2ARRAY_FIELD_PROP_UE(sps_delta_qp_diff_val, i, j, 
							"a delta value used to derive the output coordinate of the j-th pivot point of the i-th chroma QP mapping table");
					}
					NAV_WRITE_TAG_END("Tag10");
				}
				NAV_WRITE_TAG_END("Tag1");
			}

			BST_FIELD_PROP_BOOL(sps_sao_enabled_flag, "SAO is enabled for the CLVS", "SAO is disabled for the CLVS");
			BST_FIELD_PROP_BOOL(sps_alf_enabled_flag, "ALF is enabled for the CLVS", "ALF is disabled for the CLVS");
			if (sps_alf_enabled_flag && sps_chroma_format_idc != 0) {
				BST_FIELD_PROP_BOOL(sps_ccalf_enabled_flag, "CCALF is enabled for the CLVS", "CCALF is disabled for the CLVS");
			}

			BST_FIELD_PROP_BOOL(sps_lmcs_enabled_flag, "LMCS is enabled for the CLVS", "LMCS is disabled for the CLVS");
			BST_FIELD_PROP_BOOL(sps_weighted_pred_flag, "weighted prediction might be applied to P slices", "weighted prediction is not applied to P slices");
			BST_FIELD_PROP_BOOL(sps_weighted_bipred_flag, 
				"explicit weighted prediction might be applied to B slices", 
				"explicit weighted prediction is not applied to B slices");
			BST_FIELD_PROP_BOOL(sps_long_term_ref_pics_flag, 
				"LTRPs might be used for inter prediction of one or more coded pictures in the CLVS", 
				"no LTRP is used for inter prediction of any coded picture in the CLVS");

			if (sps_video_parameter_set_id > 0) {
				BST_FIELD_PROP_BOOL(sps_inter_layer_prediction_enabled_flag, 
					"inter-layer prediction is enabled for the CLVS and ILRPs might be used for inter prediction of one or more coded pictures in the CLVS", 
					"inter-layer prediction is disabled for the CLVS and no ILRP is used for inter prediction of any coded picture in the CLVS");
			}

			BST_FIELD_PROP_BOOL(sps_idr_rpl_present_flag, 
				"RPL syntax elements could be present in slice headers of slices with nal_unit_type equal to IDR_N_LP or IDR_W_RADL", 
				"RPL syntax elements are not present in slice headers of slices with nal_unit_type equal to IDR_N_LP or IDR_W_RADL");
			BST_FIELD_PROP_BOOL(sps_rpl1_same_as_rpl0_flag, 
				"the syntax element sps_num_ref_pic_lists[1] and the syntax structure ref_pic_list_struct(1, rplsIdx) are not present", 
				"");

			NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag2", "for(i=0;i&lt;(sps_rpl1_same_as_rpl0_flag?1:2);i++)", "");
			for (i = 0; i < (sps_rpl1_same_as_rpl0_flag ? 1 : 2); i++) {
				BST_ARRAY_FIELD_PROP_UE(sps_num_ref_pic_lists, i, "");
				for (int j = 0; j < sps_num_ref_pic_lists[i]; j++) {
					NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("Tag20", "ref_pic_list_struct(%d, %d)", "", i, j);
					BST_FIELD_PROP_REF(ref_pic_list_struct[i][j]);
					NAV_WRITE_TAG_END("Tag20");
				}
			}
			NAV_WRITE_TAG_END("Tag2");

			BST_FIELD_PROP_BOOL(sps_ref_wraparound_enabled_flag, 
				"horizontal wrap-around motion compensation is enabled for the CLVS", 
				"horizontal wrap-around motion compensation is disabled for the CLVS");
			BST_FIELD_PROP_BOOL(sps_temporal_mvp_enabled_flag, 
				"temporal motion vector predictors are enabled for the CLVS", 
				"temporal motion vector predictors are disabled for the CLVS");
			if (sps_temporal_mvp_enabled_flag) {
				BST_FIELD_PROP_BOOL(sps_sbtmvp_enabled_flag,
					"temporal motion vector predictors are enabled for the CLVS",
					"temporal motion vector predictors are disabled for the CLVS");
			}
			else
			{
				NAV_WRITE_TAG_WITH_NUMBER_VALUE1(sps_sbtmvp_enabled_flag,
					sps_sbtmvp_enabled_flag ? "temporal motion vector predictors are enabled for the CLVS"
											: "temporal motion vector predictors are disabled for the CLVS");
			}
			BST_FIELD_PROP_BOOL(sps_amvr_enabled_flag,
				"adaptive motion vector difference resolution is enabled for the CVLS",
				"adaptive motion vector difference resolution is disabled for the CLVS");
			BST_FIELD_PROP_BOOL(sps_bdof_enabled_flag,
				"the bi-directional optical flow inter prediction is enabled for the CLVS",
				"the bi-directional optical flow inter prediction is disabled for the CLVS");
			if (sps_bdof_enabled_flag) {
				BST_FIELD_PROP_BOOL(sps_bdof_control_present_in_ph_flag, "ph_bdof_disabled_flag could be present in PH", "ph_bdof_disabled_flag is not present in PH");
			}
			else {
				NAV_WRITE_TAG_WITH_NUMBER_VALUE1(sps_bdof_control_present_in_ph_flag, 
					sps_bdof_control_present_in_ph_flag ? "ph_bdof_disabled_flag could be present in PH":"ph_bdof_disabled_flag is not present in PH");
			}

			BST_FIELD_PROP_BOOL(sps_smvd_enabled_flag, "symmetric motion vector difference is enabled for the CLVS", 
				"symmetric motion vector difference is disabled for the CLVS");
			BST_FIELD_PROP_BOOL(sps_dmvr_enabled_flag, "decoder motion vector refinement based inter bi-prediction is enabled for the CLVS", 
				"decoder motion vector refinement based inter bi-prediction is disabled for the CLVS");
			if (sps_dmvr_enabled_flag) {
				BST_FIELD_PROP_BOOL(sps_dmvr_control_present_in_ph_flag, "ph_dmvr_disabled_flag could be present in PH", "ph_dmvr_disabled_flag is not present in PH");
			}
			else
			{
				NAV_WRITE_TAG_WITH_NUMBER_VALUE1(sps_dmvr_control_present_in_ph_flag, 
					sps_dmvr_control_present_in_ph_flag?"ph_dmvr_disabled_flag could be present in PH":"ph_dmvr_disabled_flag is not present in PH");
			}

			BST_FIELD_PROP_BOOL(sps_mmvd_enabled_flag, 
				"merge mode with motion vector difference is enabled for the CLVS",
				"merge mode with motion vector difference is disabled for the CLVS");
			if (sps_mmvd_enabled_flag) {
				BST_FIELD_PROP_BOOL(sps_mmvd_fullpel_only_enabled_flag, 
					"the merge mode with motion vector difference using only integer sample precision is enabled for the CLVS", 
					"the merge mode with motion vector difference using only integer sample precision is disabled for the CLVS");
			}
			else
			{
				NAV_WRITE_TAG_WITH_NUMBER_VALUE1(sps_mmvd_fullpel_only_enabled_flag,
					sps_mmvd_fullpel_only_enabled_flag ? "the merge mode with motion vector difference using only integer sample precision is enabled for the CLVS"
														: "the merge mode with motion vector difference using only integer sample precision is disabled for the CLVS");
			}

			BST_FIELD_PROP_UE(sps_six_minus_max_num_merge_cand, "the maximum number of MVP candidates supported in the SPS subtracted from 6");
			BST_FIELD_PROP_BOOL(sps_sbt_enabled_flag, "", "");
			BST_FIELD_PROP_BOOL_BEGIN(sps_affine_enabled_flag, "", "");
				BST_FIELD_PROP_UE(sps_five_minus_max_num_subblock_merge_cand, 
						"the maximum number of subblock-based merging motion vector prediction candidates supported in the SPS subtracted from 5");
				BST_FIELD_PROP_BOOL(sps_6param_affine_enabled_flag, "", "");
				if (sps_amvr_enabled_flag) {
					BST_FIELD_PROP_BOOL(sps_affine_amvr_enabled_flag, "", "");
				}
				else
				{
					NAV_WRITE_TAG_WITH_NUMBER_VALUE1(sps_affine_amvr_enabled_flag, "");
				}
				BST_FIELD_PROP_BOOL(sps_affine_prof_enabled_flag, "", "");
				if (sps_affine_prof_enabled_flag) {
					BST_FIELD_PROP_BOOL(sps_prof_control_present_in_ph_flag, "", "");
				}
				else
				{
					NAV_WRITE_TAG_WITH_NUMBER_VALUE1(sps_prof_control_present_in_ph_flag, "");
				}
			BST_FIELD_PROP_BOOL_END("sps_affine_enabled_flag");

			BST_FIELD_PROP_BOOL(sps_bcw_enabled_flag, "", "");
			BST_FIELD_PROP_BOOL(sps_ciip_enabled_flag, "", "");

			uint8_t MaxNumMergeCand = (uint8_t)(6 - sps_six_minus_max_num_merge_cand);
			if (MaxNumMergeCand >= 2) {
				BST_FIELD_PROP_BOOL(sps_gpm_enabled_flag, "", "");
				if (sps_gpm_enabled_flag && MaxNumMergeCand >= 3) {
					BST_FIELD_PROP_UE(sps_max_num_merge_cand_minus_max_num_gpm_cand, "");
				}
			}

			BST_FIELD_PROP_UE(sps_log2_parallel_merge_level_minus2, "");
			BST_FIELD_PROP_BOOL(sps_isp_enabled_flag, "", "");
			BST_FIELD_PROP_BOOL(sps_mrl_enabled_flag, "", "");
			BST_FIELD_PROP_BOOL(sps_mip_enabled_flag, "", "");

			if (sps_chroma_format_idc != 0) {
				BST_FIELD_PROP_BOOL(sps_cclm_enabled_flag, "", "");
			}
			if (sps_chroma_format_idc == 1) {
				BST_FIELD_PROP_BOOL(sps_chroma_horizontal_collocated_flag, "", "");
				BST_FIELD_PROP_BOOL(sps_chroma_vertical_collocated_flag, "", "");
			}
			BST_FIELD_PROP_BOOL(sps_palette_enabled_flag, "", "");
			if (sps_chroma_format_idc == 3 && !sps_max_luma_transform_size_64_flag) {
				BST_FIELD_PROP_BOOL(sps_act_enabled_flag, "", "");
			}

			if (sps_transform_skip_enabled_flag || sps_palette_enabled_flag) {
				BST_FIELD_PROP_UE(sps_min_qp_prime_ts, "");
			}

			BST_FIELD_PROP_BOOL_BEGIN(sps_ibc_enabled_flag, "", "");
				BST_FIELD_PROP_UE(sps_six_minus_max_num_ibc_merge_cand, "");
			BST_FIELD_PROP_BOOL_END("sps_ibc_enabled_flag");

			BST_FIELD_PROP_BOOL_BEGIN(sps_ladf_enabled_flag, "", "");
			if (sps_ladf_enabled_flag) {
				BST_FIELD_PROP_2NUMBER1(sps_num_ladf_intervals_minus2, 3, "");
				BST_FIELD_PROP_SE(sps_ladf_lowest_interval_qp_offset, "");

				NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag3", "for(i=0;i&lt;sps_num_ladf_intervals_minus2+1;i++)", "");
				for (i = 0; i < (int)((int64_t)sps_num_ladf_intervals_minus2 + 1); i++) {
					BST_ARRAY_FIELD_PROP_SE(sps_ladf_qp_offset, i, "");
					BST_ARRAY_FIELD_PROP_UE(sps_ladf_delta_threshold_minus1, i, "");
				}
				NAV_WRITE_TAG_END("Tag3");
			}
			BST_FIELD_PROP_BOOL_END("sps_ladf_enabled_flag");

			BST_FIELD_PROP_BOOL(sps_explicit_scaling_list_enabled_flag, "", "");
			if (sps_lfnst_enabled_flag && sps_explicit_scaling_list_enabled_flag) {
				BST_FIELD_PROP_BOOL(sps_scaling_matrix_for_lfnst_disabled_flag, "", "");
			}

			if (sps_act_enabled_flag && sps_explicit_scaling_list_enabled_flag) {
				BST_FIELD_PROP_BOOL(sps_scaling_matrix_for_alternative_colour_space_disabled_flag, "", "");
			}

			if (sps_scaling_matrix_for_alternative_colour_space_disabled_flag) {
				BST_FIELD_PROP_BOOL(sps_scaling_matrix_designated_colour_space_flag, "", "");
			}

			BST_FIELD_PROP_BOOL(sps_dep_quant_enabled_flag, "", "");
			BST_FIELD_PROP_BOOL(sps_sign_data_hiding_enabled_flag, "", "");
			BST_FIELD_PROP_BOOL_BEGIN(sps_virtual_boundaries_enabled_flag, "", "");
				BST_FIELD_PROP_BOOL_BEGIN(sps_virtual_boundaries_present_flag, "", "");
					BST_FIELD_PROP_UE(sps_num_ver_virtual_boundaries, "");
					if (sps_num_ver_virtual_boundaries > 0) {
						NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag4", "for(i=0; i&lt;sps_num_ver_virtual_boundaries;i++)", "");
						for (i = 0; i < sps_num_ver_virtual_boundaries; i++) {
							BST_ARRAY_FIELD_PROP_UE(sps_virtual_boundary_pos_x_minus1, i, "");
						}
						NAV_WRITE_TAG_END("Tag4");
					}

					BST_FIELD_PROP_UE(sps_num_hor_virtual_boundaries, "");
					if (sps_num_hor_virtual_boundaries > 0) {
						NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag5", "for(i=0; i&lt;sps_num_hor_virtual_boundaries;i++)", "");
						for (i = 0; i < sps_num_hor_virtual_boundaries; i++) {
							BST_ARRAY_FIELD_PROP_UE(sps_virtual_boundary_pos_y_minus1, i, "");
						}
						NAV_WRITE_TAG_END("Tag5");
					}
				BST_FIELD_PROP_BOOL_END("sps_virtual_boundaries_present_flag");
			BST_FIELD_PROP_BOOL_END("sps_virtual_boundaries_enabled_flag");

			if (sps_ptl_dpb_hrd_params_present_flag) {
				BST_FIELD_PROP_BOOL_BEGIN(sps_timing_hrd_params_present_flag, "", "");
				if (sps_timing_hrd_params_present_flag) {
					BST_FIELD_PROP_REF1_1(general_timing_hrd_parameters, "");
					if (sps_max_sublayers_minus1 > 0) {
						BST_FIELD_PROP_BOOL(sps_sublayer_cpb_params_present_flag, "", "");
					}
					uint8_t firstSubLayer = sps_sublayer_cpb_params_present_flag ? 0 : sps_max_sublayers_minus1;
					NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("Tag6", 
						"ols_timing_hrd_parameters(firstSubLayer:%d, sps_max_sublayers_minus1:%d)", "",
						firstSubLayer, sps_max_sublayers_minus1);
					BST_FIELD_PROP_REF(ols_timing_hrd_parameters);
					NAV_WRITE_TAG_END("Tag6");
				}
				BST_FIELD_PROP_BOOL_END("sps_timing_hrd_params_present_flag");
			}

			BST_FIELD_PROP_BOOL(sps_field_seq_flag, "the CLVS conveys pictures that represent fields", "the CLVS conveys pictures that represent frames");
			BST_FIELD_PROP_BOOL_BEGIN(sps_vui_parameters_present_flag, "", "");
			BST_FIELD_PROP_UE(sps_vui_payload_size_minus1, "plus 1 specifies the number of RBSP bytes in the vui_payload");
			BST_FIELD_PROP_NUMBER_BITARRAY1(sps_vui_alignment_zero_bit, "");
			NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("Tag8", "vui_payload( sps_vui_payload_size_minus1 + 1: %d)", "", (int)sps_vui_payload_size_minus1 + 1);
				BST_FIELD_PROP_REF(vui_payload);
			NAV_WRITE_TAG_END("Tag8");

			BST_FIELD_PROP_BOOL_END("sps_vui_parameters_present_flag");

			BST_FIELD_PROP_BOOL(sps_extension_flag, "", "");
			if (sps_extension_flag) {
				BST_FIELD_PROP_NUMBER_BITARRAY1(sps_extension_data_flag, "");
			}

			BST_FIELD_PROP_OBJECT(rbsp_trailing_bits);

		NAV_WRITE_TAG_END("seq_parameter_set_rbsp");
		DECLARE_FIELDPROP_END()

		//
		// decoding_capability_information_rbsp()
		//
		NAL_UNIT::DECODING_CAPABILITY_INFORMATION_RBSP::DECODING_CAPABILITY_INFORMATION_RBSP(){

		}

		NAL_UNIT::DECODING_CAPABILITY_INFORMATION_RBSP::~DECODING_CAPABILITY_INFORMATION_RBSP() {
			for (auto p : profile_tier_level) {
				AMP_SAFEDEL2(p);
			}
		}

		int NAL_UNIT::DECODING_CAPABILITY_INFORMATION_RBSP::Map(AMBst in_bst)
		{
			int iRet = RET_CODE_SUCCESS;
			SYNTAX_BITSTREAM_MAP::Map(in_bst);

			try
			{
				MAP_BST_BEGIN(0);
				bsrb1(in_bst, dci_reserved_zero_4bits, 4);
				bsrb1(in_bst, dci_num_ptls_minus1, 4);
				profile_tier_level.resize((size_t)dci_num_ptls_minus1 + 1);
				for (int i = 0; i < (int)dci_num_ptls_minus1 + 1; i++) {
					bsrbreadref(in_bst, profile_tier_level[i], PROFILE_TIER_LEVEL, 1, 0);
				}

				bsrb1(in_bst, dci_extension_flag, 1);
				if (dci_extension_flag) {
					if (AMP_FAILED(iRet = read_extension_and_trailing_bits(in_bst, map_status, dci_extension_data_flag, rbsp_trailing_bits)))
					{
						return iRet;
					}
				}
				else
				{
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

		int NAL_UNIT::DECODING_CAPABILITY_INFORMATION_RBSP::Unmap(AMBst out_bst)
		{
			UNREFERENCED_PARAMETER(out_bst);
			return RET_CODE_ERROR_NOTIMPL;
		}

		DECLARE_FIELDPROP_BEGIN1(NAL_UNIT::DECODING_CAPABILITY_INFORMATION_RBSP)
		NAV_WRITE_TAG_BEGIN_WITH_ALIAS("decoding_capability_information_rbsp", "decoding_capability_information_rbsp()", "");
		BST_FIELD_PROP_2NUMBER1(dci_reserved_zero_4bits, 4, "shall be equal to 0");
		BST_FIELD_PROP_2NUMBER1(dci_num_ptls_minus1, 4, "plus 1 specifies the number of profile_tier_level() syntax structures");

		NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "for(i=0;i&lt;=dci_num_ptls_minus1;i++ )", "");
		for (i = 0; i < (int)dci_num_ptls_minus1 + 1; i++) {
			NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag00", "profile_tier_level( 1, 0 )", "no dci_extension_data_flag syntax elements are present");
			BST_FIELD_PROP_REF(profile_tier_level[i]);
			NAV_WRITE_TAG_END("Tag00");
		}
		NAV_WRITE_TAG_END("Tag0");

		BST_FIELD_PROP_BOOL(dci_extension_flag, "dci_extension_data_flag syntax elements might be present", "");

		if (dci_extension_flag)
		{
			NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag1", "dci_extension_data_flags", "");
			for (i = 0; i <= dci_extension_data_flag.UpperBound(); i++) {
				BST_ARRAY_FIELD_PROP_NUMBER("dci_extension_data_flag", i, 1, dci_extension_data_flag[i], "");
			}
			NAV_WRITE_TAG_END("Tag1");
		}

		BST_FIELD_PROP_OBJECT(rbsp_trailing_bits);

		NAV_WRITE_TAG_END("decoding_capability_information_rbsp");
		DECLARE_FIELDPROP_END()

		//
		// operating_point_information_rbsp()
		//
		NAL_UNIT::OPERATING_POINT_INFORMATION_RBSP::OPERATING_POINT_INFORMATION_RBSP(){
		}

		NAL_UNIT::OPERATING_POINT_INFORMATION_RBSP::~OPERATING_POINT_INFORMATION_RBSP()
		{
		}

		int NAL_UNIT::OPERATING_POINT_INFORMATION_RBSP::Map(AMBst in_bst)
		{
			int iRet = RET_CODE_SUCCESS;
			SYNTAX_BITSTREAM_MAP::Map(in_bst);

			try
			{
				MAP_BST_BEGIN(0);
				bsrb1(in_bst, opi_ols_info_present_flag, 1);
				bsrb1(in_bst, opi_htid_info_present_flag, 1);

				if (opi_ols_info_present_flag) {
					nal_read_ue1(in_bst, opi_ols_idx);
				}

				if (opi_htid_info_present_flag) {
					bsrb1(in_bst, opi_htid_plus1, 3);
				}

				bsrb1(in_bst, opi_extension_flag, 1);
				if (opi_extension_flag)
				{
					if (AMP_FAILED(iRet = read_extension_and_trailing_bits(in_bst, map_status, opi_extension_data_flag, rbsp_trailing_bits)))
					{
						return iRet;
					}
				}
				else
				{
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

		int NAL_UNIT::OPERATING_POINT_INFORMATION_RBSP::Unmap(AMBst out_bst)
		{
			UNREFERENCED_PARAMETER(out_bst);
			return RET_CODE_ERROR_NOTIMPL;
		}

		DECLARE_FIELDPROP_BEGIN1(NAL_UNIT::OPERATING_POINT_INFORMATION_RBSP)
			NAV_WRITE_TAG_BEGIN_WITH_ALIAS("operating_point_information_rbsp", "operating_point_information_rbsp()", "");
				BST_FIELD_PROP_BOOL(opi_ols_info_present_flag, "opi_ols_idx is present", "opi_ols_idx is not present");
				BST_FIELD_PROP_BOOL(opi_htid_info_present_flag, "opi_htid_plus1 is present", "opi_htid_plus1 is not present");
				if (opi_ols_info_present_flag) {
					BST_FIELD_PROP_UE(opi_ols_idx, "Do not contain any other layers than those included in the OLS with OLS index equal to this op_ols_idx");
				}
				if (opi_htid_info_present_flag) {
					BST_FIELD_PROP_2NUMBER1(opi_htid_plus1, 3, "provided in an OPI NAL unit have TemporalId less than opi_htid_plus1");
				}
				BST_FIELD_PROP_BOOL(opi_extension_flag, "opi_extension_data_flag syntax elements might be present", "no opi_extension_data_flag syntax elements are present");
				if (opi_extension_flag) {
					NAV_WRITE_TAG_BEGIN("opi_extension_data_flags");
					for (i = 0; i < opi_extension_data_flag.UpperBound(); i++) {
						BST_ARRAY_FIELD_PROP_NUMBER1(opi_extension_data_flag, i, 1, "");
					}
					NAV_WRITE_TAG_END("opi_extension_data_flags");
				}
			NAV_WRITE_TAG_END("operating_point_information_rbsp");
		DECLARE_FIELDPROP_END()

		//
		// adaptation_parameter_set_rbsp()
		//
		NAL_UNIT::ADAPTATION_PARAMETER_SET_RBSP::ALF_DATA::ALF_DATA(bool apsChromaPresentFlag)
			: alf_chroma_filter_signal_flag(0)
			, alf_cc_cb_filter_signal_flag(0)
			, alf_cc_cr_filter_signal_flag(0)
			, aps_chroma_present_flag(apsChromaPresentFlag){
		}

		int NAL_UNIT::ADAPTATION_PARAMETER_SET_RBSP::ALF_DATA::Map(AMBst in_bst)
		{
			int iRet = RET_CODE_SUCCESS;
			SYNTAX_BITSTREAM_MAP::Map(in_bst);

			try
			{
				MAP_BST_BEGIN(0);
				bsrb1(in_bst, alf_luma_filter_signal_flag, 1);
				if (aps_chroma_present_flag)
				{
					bsrb1(in_bst, alf_chroma_filter_signal_flag, 1);
					bsrb1(in_bst, alf_cc_cb_filter_signal_flag, 1);
					bsrb1(in_bst, alf_cc_cr_filter_signal_flag, 1);
				}

				if (alf_luma_filter_signal_flag) {
					bsrb1(in_bst, alf_luma_clip_flag, 1);
					nal_read_ue1(in_bst, alf_luma_num_filters_signalled_minus1);

					uint8_t nBits = quick_ceil_log2(alf_luma_num_filters_signalled_minus1 + 1);
					if (alf_luma_num_filters_signalled_minus1 > 0)
					{
						alf_luma_coeff_delta_idx.resize(NumAlfFilters);
						for (uint8_t filtIdx = 0; filtIdx < NumAlfFilters; filtIdx++) {
							bsrb1(in_bst, alf_luma_coeff_delta_idx[filtIdx], nBits);
						}
					}

					for (size_t sfIdx = 0; sfIdx <= alf_luma_num_filters_signalled_minus1; sfIdx++) {
						for (uint8_t j = 0; j < 12; j++) {
							nal_read_ue1(in_bst, alf_luma_coeff_abs[sfIdx][j]);
							if (alf_luma_coeff_abs[sfIdx][j]) {
								bsrbarray(in_bst, alf_luma_coeff_sign[sfIdx], j);
							}
						}
					}

					if (alf_luma_clip_flag) {
						for (size_t sfIdx = 0; sfIdx <= alf_luma_num_filters_signalled_minus1; sfIdx++) {
							for (uint8_t j = 0; j < 12; j++) {
								bsrb1(in_bst, alf_luma_clip_idx[sfIdx][j], 2);
							}
						}
					}
				}

				if (alf_chroma_filter_signal_flag) {
					bsrb1(in_bst, alf_chroma_clip_flag, 1);
					nal_read_ue1(in_bst, alf_chroma_num_alt_filters_minus1);
					for (size_t altIdx = 0; altIdx <= alf_chroma_num_alt_filters_minus1; altIdx++) {
						for (uint8_t j = 0; j < 6; j++) {
							nal_read_ue1(in_bst, alf_chroma_coeff_abs[altIdx][j]);
							if (alf_chroma_coeff_abs[altIdx][j] > 0) {
								bsrbarray(in_bst, alf_chroma_coeff_sign[altIdx], j);
							}
						}

						if (alf_chroma_clip_flag) {
							for (uint8_t j = 0; j < 6; j++) {
								bsrb1(in_bst, alf_chroma_clip_idx[altIdx][j], 2);
							}
						}
					}
				}

				if (alf_cc_cb_filter_signal_flag) {
					nal_read_ue1(in_bst, alf_cc_cb_filters_signalled_minus1);
					for (size_t k = 0; k < (size_t)alf_cc_cb_filters_signalled_minus1 + 1; k++) {
						for (uint8_t j = 0; j < 7; j++) {
							bsrb1(in_bst, alf_cc_cb_mapped_coeff_abs[k][j], 3);
							if (alf_cc_cb_mapped_coeff_abs[k][j]) {
								bsrbarray(in_bst, alf_cc_cb_coeff_sign[k], j);
							}
						}
					}
				}

				if (alf_cc_cr_filter_signal_flag) {
					nal_read_ue1(in_bst, alf_cc_cr_filters_signalled_minus1);
					for (size_t k = 0; k < (size_t)alf_cc_cr_filters_signalled_minus1 + 1; k++) {
						for (uint8_t j = 0; j < 7; j++) {
							bsrb1(in_bst, alf_cc_cr_mapped_coeff_abs[k][j], 3);
							if (alf_cc_cr_mapped_coeff_abs[k][j]) {
								bsrbarray(in_bst, alf_cc_cr_coeff_sign[k], j);
							}
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

		int NAL_UNIT::ADAPTATION_PARAMETER_SET_RBSP::ALF_DATA::Unmap(AMBst out_bst)
		{
			UNREFERENCED_PARAMETER(out_bst);
			return RET_CODE_ERROR_NOTIMPL;
		}

		NAL_UNIT::ADAPTATION_PARAMETER_SET_RBSP::LMCS_DATA::LMCS_DATA(bool apsChromaPresentFlag)
			: aps_chroma_present_flag(apsChromaPresentFlag){
		}

		int NAL_UNIT::ADAPTATION_PARAMETER_SET_RBSP::LMCS_DATA::Map(AMBst in_bst)
		{
			int iRet = RET_CODE_SUCCESS;
			SYNTAX_BITSTREAM_MAP::Map(in_bst);

			try
			{
				MAP_BST_BEGIN(0);

				nal_read_ue1(in_bst, lmcs_min_bin_idx);
				nal_read_ue1(in_bst, lmcs_delta_max_bin_idx);
				nal_read_ue1(in_bst, lmcs_delta_cw_prec_minus1);

				int LmcsMaxBinIdx = 15 - lmcs_delta_max_bin_idx;
				for (int i = lmcs_min_bin_idx; i <= LmcsMaxBinIdx; i++) {
					bsrb1(in_bst, lmcs_delta_abs_cw[i], lmcs_delta_cw_prec_minus1 + 1);
					if (lmcs_delta_abs_cw[i] > 0){
						bsrbarray(in_bst, lmcs_delta_sign_cw_flag, i);
					}
				}

				if (aps_chroma_present_flag) {
					bsrb1(in_bst, lmcs_delta_abs_crs, 3);
					if (lmcs_delta_abs_crs > 0) {
						bsrb1(in_bst, lmcs_delta_sign_crs_flag, 1);
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

		int NAL_UNIT::ADAPTATION_PARAMETER_SET_RBSP::LMCS_DATA::Unmap(AMBst out_bst)
		{
			UNREFERENCED_PARAMETER(out_bst);
			return RET_CODE_ERROR_NOTIMPL;
		}

		NAL_UNIT::ADAPTATION_PARAMETER_SET_RBSP::SCALING_LIST_DATA::SCALING_LIST_DATA(bool apsChromaPresentFlag) 
			: aps_chroma_present_flag(apsChromaPresentFlag) {
			UpdateDiagScanOrder();
		}

		int NAL_UNIT::ADAPTATION_PARAMETER_SET_RBSP::SCALING_LIST_DATA::Map(AMBst in_bst)
		{
			int iRet = RET_CODE_SUCCESS;
			SYNTAX_BITSTREAM_MAP::Map(in_bst);

			try
			{
				MAP_BST_BEGIN(0);

				for (uint8_t id = 0; id < 28; id++) {
					uint8_t matrixSize = id < 2 ? 2 : (id < 8 ? 4 : 8);
					if (aps_chroma_present_flag || id % 3 == 2 || id == 27) {
						bsrbarray(in_bst, scaling_list_copy_mode_flag, id);
						if (!scaling_list_copy_mode_flag[id]) {
							bsrbarray(in_bst, scaling_list_pred_mode_flag, id);
						}

						if ((scaling_list_copy_mode_flag[id] || scaling_list_pred_mode_flag[id]) && id != 0 && id != 2 && id != 8) {
							nal_read_ue1(in_bst, scaling_list_pred_id_delta[id]);
						}

						if (!scaling_list_copy_mode_flag[id]) {
							int nextCoef = 0;
							if (id > 13) {
								nal_read_se1(in_bst, scaling_list_dc_coef[id - 14]);
								nextCoef += scaling_list_dc_coef[id - 14];
							}

							uint8_t matrixSize = (id < 2) ? 2 : ((id < 8) ? 4 : 8);
							for (uint16_t i = 0; i < matrixSize * matrixSize; i++) {
								uint8_t x = DiagScanOrder[3][3][i][0];
								uint8_t y = DiagScanOrder[3][3][i][1];
								if (!(id > 25 && x >= 4 && y >= 4)) {
									nal_read_se1(in_bst, scaling_list_delta_coef[id][i]);
									nextCoef += scaling_list_delta_coef[id][i];
								}

								ScalingList[id][i] = nextCoef;
							}
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

		int NAL_UNIT::ADAPTATION_PARAMETER_SET_RBSP::SCALING_LIST_DATA::Unmap(AMBst out_bst)
		{
			UNREFERENCED_PARAMETER(out_bst);
			return RET_CODE_ERROR_NOTIMPL;
		}

		uint8_t NAL_UNIT::ADAPTATION_PARAMETER_SET_RBSP::SCALING_LIST_DATA::DiagScanOrder[4][4][64][2] = { {{{0}}} };

		void NAL_UNIT::ADAPTATION_PARAMETER_SET_RBSP::SCALING_LIST_DATA::UpdateDiagScanOrder() {
			for (unsigned log2BlockWidth = 0; log2BlockWidth < 4; log2BlockWidth++)
			{
				for (unsigned log2BlockHeight = 0; log2BlockHeight < 4; log2BlockHeight++)
				{
					auto blkWidth = 1u << log2BlockWidth;
					auto blkHeight = 1u << log2BlockHeight;

					uint8_t i = 0, x = 0, y = 0;
					bool stopLoop = false;
					while (!stopLoop) {
						while (y >= 0) {
							if (x < blkWidth && y < blkHeight) {
								DiagScanOrder[log2BlockWidth][log2BlockHeight][i][0] = x;
								DiagScanOrder[log2BlockWidth][log2BlockHeight][i][1] = y;
								i++;
							}
							y--;
							x++;
						}
						y = x;
						x = 0;
						if (i >= blkWidth * blkHeight)
							stopLoop = true;
					}
				}
			}
		}

		NAL_UNIT::ADAPTATION_PARAMETER_SET_RBSP::ADAPTATION_PARAMETER_SET_RBSP(uint8_t NUType)
			: nu_type(NUType) {
		}

		NAL_UNIT::ADAPTATION_PARAMETER_SET_RBSP::~ADAPTATION_PARAMETER_SET_RBSP() {
			if (aps_params_type == ALF_APS){
				AMP_SAFEDEL2(alf_data);
			}
			else if (aps_params_type == LMCS_APS){
				AMP_SAFEDEL2(lmcs_data);
			}
			else if (aps_params_type == SCALING_APS){
				AMP_SAFEDEL2(scaling_list_data);
			}
		}

		int NAL_UNIT::ADAPTATION_PARAMETER_SET_RBSP::Map(AMBst in_bst)
		{
			int iRet = RET_CODE_SUCCESS;
			SYNTAX_BITSTREAM_MAP::Map(in_bst);

			try
			{
				MAP_BST_BEGIN(0);
				bsrb1(in_bst, aps_params_type, 3);
				bsrb1(in_bst, aps_adaptation_parameter_set_id, 5);
				bsrb1(in_bst, aps_chroma_present_flag, 1);

				if (aps_params_type == ALF_APS)
				{
					bsrbreadref(in_bst, alf_data, ALF_DATA, aps_chroma_present_flag);
				}
				else if (aps_params_type == LMCS_APS)
				{
					bsrbreadref(in_bst, lmcs_data, LMCS_DATA, aps_chroma_present_flag);
				}
				else if (aps_params_type == SCALING_APS)
				{
					bsrbreadref(in_bst, scaling_list_data, SCALING_LIST_DATA, aps_chroma_present_flag);
				}

				bsrb1(in_bst, aps_extension_flag, 1);
				if (aps_extension_flag)
				{
					if (AMP_FAILED(iRet = read_extension_and_trailing_bits(in_bst, map_status, aps_extension_data_flag, rbsp_trailing_bits)))
					{
						return iRet;
					}
				}
				else
				{
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

		int NAL_UNIT::ADAPTATION_PARAMETER_SET_RBSP::Unmap(AMBst out_bst)
		{
			UNREFERENCED_PARAMETER(out_bst);
			return RET_CODE_ERROR_NOTIMPL;
		}

		DECLARE_FIELDPROP_BEGIN1(NAL_UNIT::ADAPTATION_PARAMETER_SET_RBSP)

		DECLARE_FIELDPROP_END()

		//
		// pic_parameter_set_rbsp()
		//
		NAL_UNIT::PIC_PARAMETER_SET_RBSP::PIC_PARAMETER_SET_RBSP(INALVVCContext* pCtx)
			: pps_loop_filter_across_tiles_enabled_flag(0), pps_rect_slice_flag(1)
			, pps_single_slice_per_subpic_flag(1), pps_tile_idx_delta_present_flag(0)
			, pps_loop_filter_across_slices_enabled_flag(0), pps_joint_cbcr_qp_offset_present_flag(0)
			, pps_cb_qp_offset(0), pps_cr_qp_offset(0), pps_joint_cbcr_qp_offset_value(0)
			, pps_deblocking_filter_override_enabled_flag(0), pps_deblocking_filter_disabled_flag(0), pps_dbf_info_in_ph_flag(0)
			, pps_luma_beta_offset_div2(0), pps_luma_tc_offset_div2(0)
			, pps_rpl_info_in_ph_flag(0), pps_sao_info_in_ph_flag(0), pps_alf_info_in_ph_flag(0), pps_wp_info_in_ph_flag(0)
			, pps_qp_delta_info_in_ph_flag(0), m_pCtx(pCtx)
		{
		}

		NAL_UNIT::PIC_PARAMETER_SET_RBSP::~PIC_PARAMETER_SET_RBSP()
		{

		}

		int NAL_UNIT::PIC_PARAMETER_SET_RBSP::Map(AMBst in_bst)
		{
			int iRet = RET_CODE_SUCCESS;
			SYNTAX_BITSTREAM_MAP::Map(in_bst);
			H266_NU sps;

			try
			{
				MAP_BST_BEGIN(0);

				bsrb1(in_bst, pps_pic_parameter_set_id, 6);
				bsrb1(in_bst, pps_seq_parameter_set_id, 4);
				bsrb1(in_bst, pps_mixed_nalu_types_in_pic_flag, 1);
				nal_read_ue1(in_bst, pps_pic_width_in_luma_samples);
				nal_read_ue1(in_bst, pps_pic_height_in_luma_samples);
				bsrb1(in_bst, pps_conformance_window_flag, 1);

				if (m_pCtx == nullptr || 
					(sps = m_pCtx->GetVVCSPS(pps_seq_parameter_set_id)) == nullptr ||
					sps->ptr_seq_parameter_set_rbsp == nullptr)
					return RET_CODE_ERROR_NOTIMPL;

				uint8_t CtbLog2SizeY = sps->ptr_seq_parameter_set_rbsp->sps_log2_ctu_size_minus5 + 5;
				uint32_t CtbSizeY = 1 << CtbLog2SizeY;
				uint8_t MinCbLog2SizeY = sps->ptr_seq_parameter_set_rbsp->sps_log2_min_luma_coding_block_size_minus2 + 2;
				uint32_t MinCbSizeY = 1 << MinCbLog2SizeY;
				uint8_t SubWidthC = (sps->ptr_seq_parameter_set_rbsp->sps_chroma_format_idc == 1 || 
									 sps->ptr_seq_parameter_set_rbsp->sps_chroma_format_idc == 2)?2:1;
				uint8_t SubHeightC = sps->ptr_seq_parameter_set_rbsp->sps_chroma_format_idc == 1 ? 2 : 1;

				PicWidthInCtbsY = (pps_pic_width_in_luma_samples + CtbSizeY -1) / CtbSizeY;
				PicHeightInCtbsY = (pps_pic_height_in_luma_samples + CtbSizeY - 1) / CtbSizeY;
				PicSizeInCtbsY = PicWidthInCtbsY * PicHeightInCtbsY;
				PicWidthInMinCbsY = pps_pic_width_in_luma_samples / MinCbSizeY;
				PicHeightInMinCbsY = pps_pic_height_in_luma_samples / MinCbSizeY;
				PicSizeInMinCbsY = PicWidthInMinCbsY * PicHeightInMinCbsY;
				PicSizeInSamplesY = pps_pic_width_in_luma_samples * pps_pic_height_in_luma_samples;
				PicWidthInSamplesC = pps_pic_width_in_luma_samples / SubWidthC;
				PicHeightInSamplesC = pps_pic_height_in_luma_samples / SubHeightC;

				if (pps_conformance_window_flag)
				{
					nal_read_ue1(in_bst, pps_conf_win_left_offset);
					nal_read_ue1(in_bst, pps_conf_win_right_offset);
					nal_read_ue1(in_bst, pps_conf_win_top_offset);
					nal_read_ue1(in_bst, pps_conf_win_bottom_offset);
				}

				bsrb1(in_bst, pps_scaling_window_explicit_signalling_flag, 1);
				if (pps_scaling_window_explicit_signalling_flag) {
					nal_read_se1(in_bst, pps_scaling_win_left_offset);
					nal_read_se1(in_bst, pps_scaling_win_right_offset);
					nal_read_se1(in_bst, pps_scaling_win_top_offset);
					nal_read_se1(in_bst, pps_scaling_win_bottom_offset);
				}

				bsrb1(in_bst, pps_output_flag_present_flag, 1);
				bsrb1(in_bst, pps_no_pic_partition_flag, 1);
				bsrb1(in_bst, pps_subpic_id_mapping_present_flag, 1);

				if (pps_subpic_id_mapping_present_flag) {
					if (!pps_no_pic_partition_flag) {
						nal_read_ue1(in_bst, pps_num_subpics_minus1);
					}
					nal_read_ue1(in_bst, pps_subpic_id_len_minus1);
					pps_subpic_id.resize((size_t)pps_num_subpics_minus1 + 1);
					for (int i = 0; i <= pps_num_subpics_minus1; i++) {
						bsrb1(in_bst, pps_subpic_id[i], pps_subpic_id_len_minus1 + 1);
					}

					SubpicIdVal.resize((size_t)sps->ptr_seq_parameter_set_rbsp->sps_num_subpics_minus1 + 1);
					for (int i = 0; i <= sps->ptr_seq_parameter_set_rbsp->sps_num_subpics_minus1; i++) {
						if (sps->ptr_seq_parameter_set_rbsp->sps_subpic_id_mapping_explicitly_signalled_flag)
							SubpicIdVal[i] = pps_subpic_id_mapping_present_flag ? pps_subpic_id[i] : sps->ptr_seq_parameter_set_rbsp->sps_subpic_id[i];
						else
							SubpicIdVal[i] = i;
					}
				}

				if (!pps_no_pic_partition_flag) {
					bsrb1(in_bst, pps_log2_ctu_size_minus5, 2);
					nal_read_ue1(in_bst, pps_num_exp_tile_columns_minus1);
					nal_read_ue1(in_bst, pps_num_exp_tile_rows_minus1);

					pps_tile_column_width_minus1.resize((size_t)pps_num_exp_tile_columns_minus1 + 1);
					for (uint32_t i = 0; i <= pps_num_exp_tile_columns_minus1; i++) {
						nal_read_ue1(in_bst, pps_tile_column_width_minus1[i]);
					}
					pps_tile_row_height_minus1.resize((size_t)pps_num_exp_tile_rows_minus1 + 1);
					for (uint32_t i = 0; i <= pps_num_exp_tile_rows_minus1; i++) {
						nal_read_ue1(in_bst, pps_tile_row_height_minus1[i]);
					}

					UpdateNumTileColumnsRows();

					if (NumTileColumns*NumTileRows > 1) {
						bsrb1(in_bst, pps_loop_filter_across_tiles_enabled_flag, 1);
						bsrb1(in_bst, pps_rect_slice_flag, 1);
					}

					if (pps_rect_slice_flag) {
						bsrb1(in_bst, pps_single_slice_per_subpic_flag, 1);
					}

					TileScanning();

					if (pps_rect_slice_flag && !pps_single_slice_per_subpic_flag) {
						nal_read_ue1(in_bst, pps_num_slices_in_pic_minus1);
						if (pps_num_slices_in_pic_minus1 > 1) {
							bsrb1(in_bst, pps_tile_idx_delta_present_flag, 1);
						}
						pps_slice_width_in_tiles_minus1.resize(pps_num_slices_in_pic_minus1);
						pps_slice_height_in_tiles_minus1.resize(pps_num_slices_in_pic_minus1);
						pps_num_exp_slices_in_tile.resize(pps_num_slices_in_pic_minus1);
						pps_exp_slice_height_in_ctus_minus1.resize(pps_num_slices_in_pic_minus1);
						pps_tile_idx_delta_val.resize(pps_num_slices_in_pic_minus1);
						for (int i = 0; i < pps_num_slices_in_pic_minus1; i++) {
							if (SliceTopLeftTileIdx[i] % NumTileColumns != NumTileColumns - 1) {
								nal_read_ue1(in_bst, pps_slice_width_in_tiles_minus1[i]);
							}
							else
								pps_slice_width_in_tiles_minus1[i] = 0;

							if (SliceTopLeftTileIdx[i] / NumTileColumns != NumTileRows - 1 &&
								(pps_tile_idx_delta_present_flag || SliceTopLeftTileIdx[i] % NumTileColumns == 0)) {
								nal_read_ue1(in_bst, pps_slice_height_in_tiles_minus1[i]);
							}
							else if (SliceTopLeftTileIdx[i] / NumTileColumns == NumTileRows - 1)
								pps_slice_height_in_tiles_minus1[i] = 0;
							else if (i > 0)
								pps_slice_height_in_tiles_minus1[i] = pps_slice_height_in_tiles_minus1[(size_t)i - 1];

							if (pps_slice_width_in_tiles_minus1[i] == 0 && pps_slice_height_in_tiles_minus1[i] == 0 &&
								RowHeightVal[SliceTopLeftTileIdx[i] / NumTileColumns] > 1) {
								nal_read_ue1(in_bst, pps_num_exp_slices_in_tile[i]);

								if (pps_num_exp_slices_in_tile[i] > 0) {
									pps_exp_slice_height_in_ctus_minus1[i].resize(pps_num_exp_slices_in_tile[i]);
									for (uint32_t j = 0; j < pps_num_exp_slices_in_tile[i]; j++) {
										nal_read_ue1(in_bst, pps_exp_slice_height_in_ctus_minus1[i][j]);
									}
								}

								i += NumSlicesInTile[i] - 1;
							}
							else
								pps_num_exp_slices_in_tile[i] = 0;

							if (pps_tile_idx_delta_present_flag && i < pps_num_slices_in_pic_minus1) {
								nal_read_se1(in_bst, pps_tile_idx_delta_val[i]);
							}
							else
								pps_tile_idx_delta_val[i] = 0;
						}
					}

					if (!pps_rect_slice_flag || pps_single_slice_per_subpic_flag || pps_num_slices_in_pic_minus1 > 0)
					{
						bsrb1(in_bst, pps_loop_filter_across_slices_enabled_flag, 1);
					}
				}

				bsrb1(in_bst, pps_cabac_init_present_flag, 1);
				for (uint8_t i = 0; i < 2; i++) {
					nal_read_ue1(in_bst, pps_num_ref_idx_default_active_minus1[i]);
				}

				bsrb1(in_bst, pps_rpl1_idx_present_flag, 1);
				bsrb1(in_bst, pps_weighted_pred_flag, 1);
				bsrb1(in_bst, pps_weighted_bipred_flag, 1);
				bsrb1(in_bst, pps_ref_wraparound_enabled_flag, 1);

				if (pps_ref_wraparound_enabled_flag) {
					nal_read_ue1(in_bst, pps_pic_width_minus_wraparound_offset);
				}

				nal_read_se1(in_bst, pps_init_qp_minus26);

				bsrb1(in_bst, pps_cu_qp_delta_enabled_flag, 1);
				bsrb1(in_bst, pps_chroma_tool_offsets_present_flag, 1);

				if (pps_chroma_tool_offsets_present_flag) {
					nal_read_se1(in_bst, pps_cb_qp_offset);
					nal_read_se1(in_bst, pps_cr_qp_offset);
					bsrb1(in_bst, pps_joint_cbcr_qp_offset_present_flag, 1);

					if (pps_joint_cbcr_qp_offset_present_flag) {
						nal_read_se1(in_bst, pps_joint_cbcr_qp_offset_value);
					}

					bsrb1(in_bst, pps_slice_chroma_qp_offsets_present_flag, 1);
					bsrb1(in_bst, pps_cu_chroma_qp_offset_list_enabled_flag, 1);

					if (pps_cu_chroma_qp_offset_list_enabled_flag) {
						nal_read_ue1(in_bst, pps_chroma_qp_offset_list_len_minus1);

						pps_cb_qp_offset_list.resize((size_t)pps_chroma_qp_offset_list_len_minus1 + 1);
						pps_cr_qp_offset_list.resize((size_t)pps_chroma_qp_offset_list_len_minus1 + 1);
						pps_joint_cbcr_qp_offset_list.resize((size_t)pps_chroma_qp_offset_list_len_minus1 + 1);
						for (uint8_t i = 0; i <= pps_chroma_qp_offset_list_len_minus1; i++) {
							nal_read_se1(in_bst, pps_cb_qp_offset_list[i]);
							nal_read_se1(in_bst, pps_cr_qp_offset_list[i]);
							if (pps_joint_cbcr_qp_offset_present_flag) {
								nal_read_se1(in_bst, pps_joint_cbcr_qp_offset_list[i]);
							}
							else
								pps_joint_cbcr_qp_offset_list[i] = 0;
						}
					}
				}

				bsrb1(in_bst, pps_deblocking_filter_control_present_flag, 1);
				if (pps_deblocking_filter_control_present_flag) {
					bsrb1(in_bst, pps_deblocking_filter_override_enabled_flag, 1);
					bsrb1(in_bst, pps_deblocking_filter_disabled_flag, 1);
					if (!pps_no_pic_partition_flag && pps_deblocking_filter_override_enabled_flag) {
						bsrb1(in_bst, pps_dbf_info_in_ph_flag, 1);
					}

					if (!pps_deblocking_filter_disabled_flag) {
						nal_read_se1(in_bst, pps_luma_beta_offset_div2);
						nal_read_se1(in_bst, pps_luma_tc_offset_div2);

						if (pps_chroma_tool_offsets_present_flag) {
							nal_read_se1(in_bst, pps_cb_beta_offset_div2);
							nal_read_se1(in_bst, pps_cb_tc_offset_div2);
							nal_read_se1(in_bst, pps_cr_beta_offset_div2);
							nal_read_se1(in_bst, pps_cr_tc_offset_div2);
						}
						else
						{
							pps_cb_beta_offset_div2 = pps_luma_beta_offset_div2;
							pps_cb_tc_offset_div2 = pps_luma_tc_offset_div2;
							pps_cr_beta_offset_div2 = pps_luma_beta_offset_div2;
							pps_cr_tc_offset_div2 = pps_luma_tc_offset_div2;
						}
					}
				}

				if (!pps_no_pic_partition_flag) {
					bsrb1(in_bst, pps_rpl_info_in_ph_flag, 1);
					bsrb1(in_bst, pps_sao_info_in_ph_flag, 1);
					bsrb1(in_bst, pps_alf_info_in_ph_flag, 1);

					if ((pps_weighted_pred_flag || pps_weighted_bipred_flag) && pps_rpl_info_in_ph_flag) {
						bsrb1(in_bst, pps_wp_info_in_ph_flag, 1);
					}

					bsrb1(in_bst, pps_qp_delta_info_in_ph_flag, 1);
				}

				bsrb1(in_bst, pps_picture_header_extension_present_flag, 1);
				bsrb1(in_bst, pps_slice_header_extension_present_flag, 1);
				bsrb1(in_bst, pps_extension_flag, 1);

				if (pps_extension_flag) {
					if (AMP_FAILED(iRet = read_extension_and_trailing_bits(in_bst, map_status, pps_extension_data_flag, rbsp_trailing_bits)))
					{
						return iRet;
					}
				}
				else
				{
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

		int NAL_UNIT::PIC_PARAMETER_SET_RBSP::Unmap(AMBst out_bst)
		{
			UNREFERENCED_PARAMETER(out_bst);
			return RET_CODE_ERROR_NOTIMPL;
		}

		int NAL_UNIT::PIC_PARAMETER_SET_RBSP::UpdateNumTileColumnsRows()
		{
			uint32_t i = 0, j =  0;
			int remainingWidthInCtbsY = PicWidthInCtbsY;
			std::vector<uint32_t> ColWidthVal;
			ColWidthVal.reserve((size_t)pps_num_exp_tile_columns_minus1 + 1);
			for(i = 0; i <= pps_num_exp_tile_columns_minus1; i++)
			{
				ColWidthVal.push_back(pps_tile_column_width_minus1[i] + 1);
				remainingWidthInCtbsY -= ColWidthVal[i];
			}

			uint32_t uniformTileColWidth = pps_tile_column_width_minus1[pps_num_exp_tile_columns_minus1] + 1;
			while (remainingWidthInCtbsY >= (int)uniformTileColWidth) {
				ColWidthVal.push_back(uniformTileColWidth);
				i++;
				remainingWidthInCtbsY -= uniformTileColWidth;
			}
			if (remainingWidthInCtbsY > 0) {
				ColWidthVal.push_back(remainingWidthInCtbsY);
				i++;
			}
			NumTileColumns = i;

			int remainingHeightInCtbsY = PicHeightInCtbsY;
			RowHeightVal.reserve((size_t)pps_num_exp_tile_rows_minus1 + 1);
			for (j = 0; j <= pps_num_exp_tile_rows_minus1; j++) {
				RowHeightVal.push_back(pps_tile_row_height_minus1[j] + 1);
				remainingHeightInCtbsY -= RowHeightVal[j];
			}
			uint32_t uniformTileRowHeight = pps_tile_row_height_minus1[pps_num_exp_tile_rows_minus1] + 1;
			while (remainingHeightInCtbsY >= (int)uniformTileRowHeight) {
				RowHeightVal.push_back(uniformTileRowHeight);
				j++;
				remainingHeightInCtbsY -= uniformTileRowHeight;
			}

			if (remainingHeightInCtbsY > 0) {
				RowHeightVal.push_back(remainingHeightInCtbsY);
				j++;
			}
			
			NumTileRows = j;

			TileColBdVal.resize((size_t)NumTileColumns + 1);
			for (TileColBdVal[0] = 0, i = 0; i < NumTileColumns; i++)
				TileColBdVal[i + 1] = (uint32_t)((uint64_t)TileColBdVal[i] + ColWidthVal[i]);

			TileRowBdVal.resize((size_t)NumTileRows + 1);
			for (TileRowBdVal[0] = 0, j = 0; j < NumTileRows; j++)
				TileRowBdVal[j + 1] = (uint32_t)((uint64_t)TileRowBdVal[j] + RowHeightVal[j]);

			uint32_t tileX = 0;
			if (PicWidthInCtbsY >= 0) {
				 CtbToTileColBd.resize((size_t)((int64_t)PicWidthInCtbsY + 1));
				ctbToTileColIdx.resize((size_t)((int64_t)PicWidthInCtbsY + 1));
			}
			for (int32_t ctbAddrX = 0; ctbAddrX <= PicWidthInCtbsY; ctbAddrX++) {
				if (ctbAddrX == (int32_t)TileColBdVal[(size_t)tileX + 1])
					tileX++;
				CtbToTileColBd[ctbAddrX] = TileColBdVal[tileX]; 
				ctbToTileColIdx[ctbAddrX] = tileX;
			}

			uint32_t tileY = 0;
			if (PicHeightInCtbsY >= 0)
			{
				 CtbToTileRowBd.resize((size_t)((int64_t)PicHeightInCtbsY + 1));
				ctbToTileRowIdx.resize((size_t)((int64_t)PicHeightInCtbsY + 1));
			}
			for (int32_t ctbAddrY = 0; ctbAddrY <= PicHeightInCtbsY; ctbAddrY++) {
				if (ctbAddrY == (int32_t)TileRowBdVal[(size_t)tileY + 1])
					tileY++;
				CtbToTileRowBd[ctbAddrY] = TileRowBdVal[tileY];
				ctbToTileRowIdx[ctbAddrY] = tileY;
			}

			H266_NU sps;
			if (m_pCtx == nullptr ||
				(sps = m_pCtx->GetVVCSPS(pps_seq_parameter_set_id)) == nullptr ||
				sps->ptr_seq_parameter_set_rbsp == nullptr)
				return RET_CODE_ERROR_NOTIMPL;

			SubpicWidthInTiles.resize((size_t)sps->ptr_seq_parameter_set_rbsp->sps_num_subpics_minus1 + 1);
			SubpicHeightInTiles.resize((size_t)sps->ptr_seq_parameter_set_rbsp->sps_num_subpics_minus1 + 1);
			subpicHeightLessThanOneTileFlag.resize((size_t)sps->ptr_seq_parameter_set_rbsp->sps_num_subpics_minus1 + 1);
			for (i = 0; i <= sps->ptr_seq_parameter_set_rbsp->sps_num_subpics_minus1; i++) {
				uint32_t leftX = sps->ptr_seq_parameter_set_rbsp->sps_subpic_ctu_top_left_x[i];
				uint32_t rightX = leftX + sps->ptr_seq_parameter_set_rbsp->sps_subpic_width_minus1[i];
				SubpicWidthInTiles[i] = ctbToTileColIdx[rightX] + 1 - ctbToTileColIdx[leftX]; 
				uint32_t topY = sps->ptr_seq_parameter_set_rbsp->sps_subpic_ctu_top_left_y[i];
				uint32_t bottomY = topY + sps->ptr_seq_parameter_set_rbsp->sps_subpic_height_minus1[i];
				SubpicHeightInTiles[i] = ctbToTileRowIdx[bottomY] + 1 - ctbToTileRowIdx[topY]; 
				if (SubpicHeightInTiles[i] == 1 && sps->ptr_seq_parameter_set_rbsp->sps_subpic_height_minus1[i] + 1 < RowHeightVal[ctbToTileRowIdx[topY]])
					subpicHeightLessThanOneTileFlag[i] = 1;
				else
					subpicHeightLessThanOneTileFlag[i] = 0;
			}

			return RET_CODE_SUCCESS;
		}

		int NAL_UNIT::PIC_PARAMETER_SET_RBSP::TileScanning()
		{
			H266_NU sps;
			if (m_pCtx == nullptr ||
				(sps = m_pCtx->GetVVCSPS(pps_seq_parameter_set_id)) == nullptr ||
				sps->ptr_seq_parameter_set_rbsp == nullptr)
				return RET_CODE_ERROR_NOTIMPL;

			NumCtusInSlice.resize((size_t)pps_num_slices_in_pic_minus1 + 1);
			if (pps_single_slice_per_subpic_flag) 
			{
				if (!sps->ptr_seq_parameter_set_rbsp->sps_subpic_info_present_flag) /* There is no subpicture info and only one slice in a picture. */
				{
					for (uint32_t j = 0; j < NumTileRows; j++) {
						for (uint32_t i = 0; i < NumTileColumns; i++) {
							AddCtbsToSlice(0, TileColBdVal[i], TileColBdVal[i + 1], TileRowBdVal[j], TileRowBdVal[j + 1]);
						}
					}
				}
				else 
				{
					for (uint32_t i = 0; i <= sps->ptr_seq_parameter_set_rbsp->sps_num_subpics_minus1; i++) {
						NumCtusInSlice[i] = 0;
						if (subpicHeightLessThanOneTileFlag[i]) /* The slice consists of a set of CTU rows in a tile. */
						{
							AddCtbsToSlice(i, 
								sps->ptr_seq_parameter_set_rbsp->sps_subpic_ctu_top_left_x[i],
								sps->ptr_seq_parameter_set_rbsp->sps_subpic_ctu_top_left_x[i] + sps->ptr_seq_parameter_set_rbsp->sps_subpic_width_minus1[i] + 1,
								sps->ptr_seq_parameter_set_rbsp->sps_subpic_ctu_top_left_y[i],
								sps->ptr_seq_parameter_set_rbsp->sps_subpic_ctu_top_left_y[i] + sps->ptr_seq_parameter_set_rbsp->sps_subpic_height_minus1[i] + 1);
						}
						else 
						{ 
							/* The slice consists of a number of complete tiles covering a rectangular region. */ 
							uint32_t tileX = ctbToTileColIdx[sps->ptr_seq_parameter_set_rbsp->sps_subpic_ctu_top_left_x[i]];
							uint32_t tileY = ctbToTileRowIdx[sps->ptr_seq_parameter_set_rbsp->sps_subpic_ctu_top_left_y[i]];
							for (uint32_t j = 0; j < SubpicHeightInTiles[i]; j++) {
								for (uint32_t k = 0; k < SubpicWidthInTiles[i]; k++) {
									AddCtbsToSlice(i, TileColBdVal[tileX + k], TileColBdVal[tileX + k + 1], TileRowBdVal[tileY + j], TileRowBdVal[tileY + j + 1]);
								}
							}
						}
					}
				}
			}
			else
			{
				uint32_t tileIdx = 0;
				for (uint32_t i = 0; i <= pps_num_slices_in_pic_minus1; i++)
					NumCtusInSlice[i] = 0;

				SliceTopLeftTileIdx.resize((size_t)pps_num_slices_in_pic_minus1 + 1);
				sliceWidthInTiles.resize((size_t)pps_num_slices_in_pic_minus1 + 1);
				sliceHeightInTiles.resize((size_t)pps_num_slices_in_pic_minus1 + 1);
				NumSlicesInTile.resize((size_t)pps_num_slices_in_pic_minus1 + 1);
				sliceHeightInCtus.resize((size_t)pps_num_slices_in_pic_minus1 + 1);

				for (uint16_t i = 0; i <= pps_num_slices_in_pic_minus1; i++) 
				{
					SliceTopLeftTileIdx[i] = tileIdx;
					uint16_t tileX = tileIdx % NumTileColumns; 
					uint16_t tileY = tileIdx / NumTileColumns;
					if (i < pps_num_slices_in_pic_minus1) {
						sliceWidthInTiles[i] = pps_slice_width_in_tiles_minus1[i] + 1;
						sliceHeightInTiles[i] = pps_slice_height_in_tiles_minus1[i] + 1;
					}
					else {
						sliceWidthInTiles[i] = NumTileColumns - tileX; 
						sliceHeightInTiles[i] = NumTileRows - tileY; 
						NumSlicesInTile[i] = 1;
					}

					if (sliceWidthInTiles[i] == 1 && sliceHeightInTiles[i] == 1) {
						if (pps_num_exp_slices_in_tile[i] == 0) {
							NumSlicesInTile[i] = 1;
							sliceHeightInCtus[i] = RowHeightVal[SliceTopLeftTileIdx[i] / NumTileColumns];
						}
						else 
						{
							uint32_t remainingHeightInCtbsY = RowHeightVal[SliceTopLeftTileIdx[i] / NumTileColumns];
							uint16_t j = 0;
							sliceHeightInCtus.resize((size_t)pps_num_slices_in_pic_minus1 + 1 + pps_num_exp_slices_in_tile[i]);
							for (j = 0; j < pps_num_exp_slices_in_tile[i]; j++) {
								sliceHeightInCtus[(size_t)i + j] = (uint32_t)pps_exp_slice_height_in_ctus_minus1[i][j] + 1;
								remainingHeightInCtbsY -= sliceHeightInCtus[(size_t)i + j];
							}
							uint32_t uniformSliceHeight = sliceHeightInCtus[(size_t)i + j - 1];
							while (remainingHeightInCtbsY >= uniformSliceHeight) {
								sliceHeightInCtus.push_back(uniformSliceHeight);
								remainingHeightInCtbsY  -= uniformSliceHeight;
								j++;
							}
							if (remainingHeightInCtbsY > 0) {
								sliceHeightInCtus.push_back(remainingHeightInCtbsY);
								j++;
							}

							NumSlicesInTile[i] = j;
						}
						uint32_t ctbY = TileRowBdVal[tileY];
						sliceWidthInTiles.resize((size_t)pps_num_slices_in_pic_minus1 + 1 + NumSlicesInTile[i]);
						sliceHeightInTiles.resize((size_t)pps_num_slices_in_pic_minus1 + 1 + NumSlicesInTile[i]);
						for (uint32_t j = 0; j < NumSlicesInTile[i]; j++) {
							AddCtbsToSlice((size_t)i + j,
								TileColBdVal[(size_t)tileX], TileColBdVal[(size_t)tileX + 1], ctbY, (uint32_t)((uint64_t)ctbY + sliceHeightInCtus[i + j]));
							ctbY += sliceHeightInCtus[(size_t)i + j]; 
							sliceWidthInTiles[(size_t)i + j] = 1;
							sliceHeightInTiles[(size_t)i + j] = 1;
						}
						i += NumSlicesInTile[i] - 1;
					}
					else
					{
						for (uint16_t j = 0; j < sliceHeightInTiles[i]; j++) {
							for (uint16_t k = 0; k < sliceWidthInTiles[i]; k++) {
								AddCtbsToSlice(i, TileColBdVal[(size_t)tileX + k], TileColBdVal[(size_t)tileX + k + 1], 
									TileRowBdVal[(size_t)tileY + j], TileRowBdVal[(size_t)tileY + j + 1]);
							}
						}
						if (i < pps_num_slices_in_pic_minus1) {
							if (pps_tile_idx_delta_present_flag)
								tileIdx += pps_tile_idx_delta_val[i];
							else {
								tileIdx += sliceWidthInTiles[i];
								if (tileIdx % NumTileColumns == 0)
									tileIdx += (sliceHeightInTiles[i] - 1) * NumTileColumns;
							}
						}
					}
				}
			}

			std::vector<uint16_t> SubpicIdxForSlice, SubpicLevelSliceIdx;
			NumSlicesInSubpic.resize((size_t)sps->ptr_seq_parameter_set_rbsp->sps_num_subpics_minus1 + 1);
			for (uint16_t i = 0; i <= sps->ptr_seq_parameter_set_rbsp->sps_num_subpics_minus1; i++) {
				NumSlicesInSubpic[i] = 0;
				SubpicIdxForSlice.resize((size_t)pps_num_slices_in_pic_minus1 + 1);
				SubpicLevelSliceIdx.resize((size_t)pps_num_slices_in_pic_minus1 + 1);
				for (uint16_t j = 0; j <= pps_num_slices_in_pic_minus1; j++) {
					uint32_t posX = CtbAddrInSlice[j][0] % PicWidthInCtbsY;
					uint32_t posY = CtbAddrInSlice[j][0] / PicWidthInCtbsY;
					if ((posX >= sps->ptr_seq_parameter_set_rbsp->sps_subpic_ctu_top_left_x[i]) &&
						(posX  < sps->ptr_seq_parameter_set_rbsp->sps_subpic_ctu_top_left_x[i] + sps->ptr_seq_parameter_set_rbsp->sps_subpic_width_minus1[i] + 1) &&
						(posY >= sps->ptr_seq_parameter_set_rbsp->sps_subpic_ctu_top_left_y[i]) &&
						(posY < sps->ptr_seq_parameter_set_rbsp->sps_subpic_ctu_top_left_y[i] + sps->ptr_seq_parameter_set_rbsp->sps_subpic_height_minus1[i] + 1)) {
						SubpicIdxForSlice[j] = i;
						SubpicLevelSliceIdx[j] = NumSlicesInSubpic[i];
						NumSlicesInSubpic[i]++;
					}
				}
			}

			return RET_CODE_SUCCESS;
		}

		DECLARE_FIELDPROP_BEGIN1(NAL_UNIT::PIC_PARAMETER_SET_RBSP)

		DECLARE_FIELDPROP_END()

		//
		// pred_weight_table()
		//
		int NAL_UNIT::PRED_WEIGHT_TABLE::Map(AMBst in_bst)
		{
			int iRet = RET_CODE_SUCCESS;
			SYNTAX_BITSTREAM_MAP::Map(in_bst);
			H266_NU sps, pps;
			int8_t NumWeightsL0, NumWeightsL1;

			try
			{
				MAP_BST_BEGIN(0);
				nal_read_ue1(in_bst, luma_log2_weight_denom);

				if (m_pCtx == nullptr ||
					(pps = m_pCtx->GetVVCPPS(m_pic_parameter_set_id)) == nullptr ||
					pps->ptr_pic_parameter_set_rbsp == nullptr ||
					(sps = m_pCtx->GetVVCSPS(pps->ptr_pic_parameter_set_rbsp->pps_seq_parameter_set_id)) == nullptr) {
					return RET_CODE_ERROR_NOTIMPL;
				}

				int8_t NumRefIdxActive[2] = { -1, - 1 };
				int8_t RplsIdx[2] = { -1, -1 };
				m_pCtx->GetNumRefIdxActive(NumRefIdxActive);
				m_pCtx->GetRplsIdx(RplsIdx);

				if (sps->ptr_seq_parameter_set_rbsp->sps_chroma_format_idc != 0) {
					nal_read_se1(in_bst, delta_chroma_log2_weight_denom);
				}

				if (pps->ptr_pic_parameter_set_rbsp->pps_wp_info_in_ph_flag) {
					nal_read_ue1(in_bst, num_l0_weights);
					NumWeightsL0 = num_l0_weights;
				}
				else
					NumWeightsL0 = NumRefIdxActive[0];

				for (int8_t i = 0; i < NumWeightsL0; i++) {
					bsrbarray(in_bst, luma_weight_l0_flag, i);
				}

				if (sps->ptr_seq_parameter_set_rbsp->sps_chroma_format_idc != 0) {
					for (int8_t i = 0; i < NumWeightsL0; i++) {
						bsrbarray(in_bst, chroma_weight_l0_flag, i);
					}
				}

				delta_luma_weight_l0.resize(NumWeightsL0);
				luma_offset_l0.resize(NumWeightsL0);
				delta_chroma_weight_l0.resize(NumWeightsL0);
				delta_chroma_offset_l0.resize(NumWeightsL0);
				for (int8_t i = 0; i < NumWeightsL0; i++) {
					if (luma_weight_l0_flag[i]) {
						nal_read_se1(in_bst, delta_luma_weight_l0[i]);
						nal_read_se1(in_bst, luma_offset_l0[i]);
					}
					else
					{
						delta_luma_weight_l0[i] = (1 << luma_log2_weight_denom);
						luma_offset_l0[i] = 0;
					}

					if (chroma_weight_l0_flag[i]) {
						for (uint8_t j = 0; j < 2; j++) {
							nal_read_se1(in_bst, delta_chroma_weight_l0[i][j]);
							nal_read_se1(in_bst, delta_chroma_offset_l0[i][j]);
						}
					}
					else
					{
						delta_chroma_weight_l0[i][0] = delta_chroma_weight_l0[i][1] = 1 << delta_chroma_log2_weight_denom;
						delta_chroma_offset_l0[i][0] = delta_chroma_offset_l0[i][1] = 0;
					}
				}

				uint8_t num_ref_entries = m_pCtx->ref_pic_list_struct[1][RplsIdx[1]] 
					? m_pCtx->ref_pic_list_struct[1][RplsIdx[1]]->num_ref_entries : 0;
				if (pps->ptr_pic_parameter_set_rbsp->pps_weighted_bipred_flag && 
					pps->ptr_pic_parameter_set_rbsp->pps_wp_info_in_ph_flag && num_ref_entries > 0)
				{
					nal_read_ue1(in_bst, num_l1_weights);
				}

				if (!pps->ptr_pic_parameter_set_rbsp->pps_weighted_bipred_flag ||
					(pps->ptr_pic_parameter_set_rbsp->pps_wp_info_in_ph_flag && num_ref_entries == 0))
					NumWeightsL1 = 0;
				else if (pps->ptr_pic_parameter_set_rbsp->pps_wp_info_in_ph_flag)
					NumWeightsL1 = num_l1_weights;
				else
					NumWeightsL1 = NumRefIdxActive[1];

				for (int8_t i = 0; i < NumWeightsL1; i++) {
					bsrbarray(in_bst, luma_weight_l1_flag, i);
				}

				if (sps->ptr_seq_parameter_set_rbsp->sps_chroma_format_idc != 0) {
					for (int8_t i = 0; i < NumWeightsL1; i++) {
						bsrbarray(in_bst, chroma_weight_l1_flag, i);
					}
				}

				delta_luma_weight_l1.resize(NumWeightsL1);
				luma_offset_l1.resize(NumWeightsL1);
				delta_chroma_weight_l1.resize(NumWeightsL1);
				delta_chroma_offset_l1.resize(NumWeightsL1);
				for (int8_t i = 0; i < NumWeightsL1; i++) {
					if (luma_weight_l1_flag[i]) {
						nal_read_se1(in_bst, delta_luma_weight_l1[i]);
						nal_read_se1(in_bst, luma_offset_l1[i]);
					}
					else
					{
						delta_luma_weight_l1[i] = (1 << luma_log2_weight_denom);
						luma_offset_l1[i] = 0;
					}

					if (chroma_weight_l1_flag[i]) {
						for (uint8_t j = 0; j < 2; j++) {
							nal_read_se1(in_bst, delta_chroma_weight_l1[i][j]);
							nal_read_se1(in_bst, delta_chroma_offset_l1[i][j]);
						}
					}
					else
					{
						delta_chroma_weight_l1[i][0] = delta_chroma_weight_l1[i][1] = 1 << delta_chroma_log2_weight_denom;
						delta_chroma_offset_l1[i][0] = delta_chroma_offset_l1[i][1] = 0;
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

		int NAL_UNIT::PRED_WEIGHT_TABLE::Unmap(AMBst out_bst)
		{
			UNREFERENCED_PARAMETER(out_bst);
			return RET_CODE_ERROR_NOTIMPL;
		}

		int NAL_UNIT::REF_PIC_LISTS::Map(AMBst in_bst)
		{
			int iRet = RET_CODE_SUCCESS;
			SYNTAX_BITSTREAM_MAP::Map(in_bst);
			H266_NU sps, pps;

			if (m_pCtx == nullptr ||
				(pps = m_pCtx->GetVVCPPS(pic_parameter_set_id)) == nullptr ||
				(pps->ptr_pic_parameter_set_rbsp == nullptr) ||
				(sps = m_pCtx->GetVVCSPS(pps->ptr_pic_parameter_set_rbsp->pps_seq_parameter_set_id)) == nullptr ||
				(sps->ptr_seq_parameter_set_rbsp == nullptr))
				return RET_CODE_ERROR_NOTIMPL;

			try
			{
				MAP_BST_BEGIN(0);
				for (uint8_t i = 0; i < 2; i++) {
					if (sps->ptr_seq_parameter_set_rbsp->sps_num_ref_pic_lists[i] > 0 &&
						(i == 0 || (i == 1 && pps->ptr_pic_parameter_set_rbsp->pps_rpl1_idx_present_flag)))
					{
						bsrbarray(in_bst, rpl_sps_flag, i);
					}
					else if (sps->ptr_seq_parameter_set_rbsp->sps_num_ref_pic_lists[i] == 0)
						rpl_sps_flag.BitClear(i);
					else
						rpl_sps_flag[0] ? rpl_sps_flag.BitSet(1) : rpl_sps_flag.BitClear(1);

					if (rpl_sps_flag[i]) {
						if (sps->ptr_seq_parameter_set_rbsp->sps_num_ref_pic_lists[i] > 1 &&
							(i == 0 || (i == 1 && pps->ptr_pic_parameter_set_rbsp->pps_rpl1_idx_present_flag))) {
							bsrb1(in_bst, rpl_idx[i], quick_ceil_log2(sps->ptr_seq_parameter_set_rbsp->sps_num_ref_pic_lists[i]));

							m_pCtx->RplsIdx[i] = rpl_sps_flag[i] ? rpl_idx[i] : sps->ptr_seq_parameter_set_rbsp->sps_num_ref_pic_lists[i];
						}
					}
					else
					{
						REF_PIC_LIST_STRUCT* refPicListStruct = nullptr;
						bsrbreadref(in_bst, 
							refPicListStruct,
							REF_PIC_LIST_STRUCT, i, 
							sps->ptr_seq_parameter_set_rbsp->sps_num_ref_pic_lists[i], 
							sps->ptr_seq_parameter_set_rbsp);

						ref_pic_list_struct[i] = std::shared_ptr<REF_PIC_LIST_STRUCT>(refPicListStruct);
						m_pCtx->ref_pic_list_struct[i][sps->ptr_seq_parameter_set_rbsp->sps_num_ref_pic_lists[i]] = ref_pic_list_struct[i];
					}

					poc_lsb_lt[i].resize(m_pCtx->NumLtrpEntries[i][m_pCtx->RplsIdx[i]]);
					delta_poc_msb_cycle_lt[i].resize(m_pCtx->NumLtrpEntries[i][m_pCtx->RplsIdx[i]]);
					for (uint8_t j = 0; j < m_pCtx->NumLtrpEntries[i][m_pCtx->RplsIdx[i]]; j++) {
						if (sps->ptr_seq_parameter_set_rbsp->ref_pic_list_struct[i][m_pCtx->RplsIdx[i]]->ltrp_in_header_flag) {
							bsrb1(in_bst, poc_lsb_lt[i][j],
								sps->ptr_seq_parameter_set_rbsp->sps_log2_max_pic_order_cnt_lsb_minus4 + 4);
						}
						bsrbarray(in_bst, delta_poc_msb_cycle_present_flag[i], j);
						if (delta_poc_msb_cycle_present_flag[i][j]) {
							nal_read_ue1(in_bst, delta_poc_msb_cycle_lt[i][j]);
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

		int NAL_UNIT::REF_PIC_LISTS::Unmap(AMBst out_bst)
		{
			UNREFERENCED_PARAMETER(out_bst);
			return RET_CODE_ERROR_NOTIMPL;
		}

		//
		// picture_header_rbsp()
		//
		NAL_UNIT::PICTURE_HEADER_STRUCTURE::PICTURE_HEADER_STRUCTURE(VideoBitstreamCtx* pCtx)
			: ph_gdr_pic_flag(0), ph_intra_slice_allowed_flag(1), ph_alf_enabled_flag(0)
			, ph_alf_cb_enabled_flag(0), ph_alf_cr_enabled_flag(0), ph_alf_cc_cb_enabled_flag(0)
			, ph_virtual_boundaries_present_flag(0), ph_num_ver_virtual_boundaries(0)
			, ph_pic_output_flag(1), ph_partition_constraints_override_flag(0)
			, ph_cu_qp_delta_subdiv_intra_slice(0), ph_cu_chroma_qp_offset_subdiv_intra_slice(0)
			, ph_cu_qp_delta_subdiv_inter_slice(0), ph_cu_chroma_qp_offset_subdiv_inter_slice(0)
			, ph_collocated_ref_idx(0), ph_temporal_mvp_enabled_flag(0), ph_collocated_from_l0_flag(1)
			, ph_mmvd_fullpel_only_flag(0), ph_mvd_l1_zero_flag(1), ph_bdof_disabled_flag(1), ph_dmvr_disabled_flag(1)
			, ph_prof_disabled_flag(1), ph_sao_luma_enabled_flag(0), ph_sao_chroma_enabled_flag(0)
			, ph_deblocking_params_present_flag(0), m_pCtx(pCtx) {
		}

		int NAL_UNIT::PICTURE_HEADER_STRUCTURE::Map(AMBst in_bst)
		{
			int iRet = RET_CODE_SUCCESS;
			SYNTAX_BITSTREAM_MAP::Map(in_bst);
			H266_NU sps, pps;

			if (m_pCtx == nullptr)
				return RET_CODE_ERROR_NOTIMPL;

			try
			{
				MAP_BST_BEGIN(0);

				bsrb1(in_bst, ph_gdr_or_irap_pic_flag, 1);
				bsrb1(in_bst, ph_non_ref_pic_flag, 1);
				if (ph_gdr_or_irap_pic_flag) {
					bsrb1(in_bst, ph_gdr_pic_flag, 1);
				}
				bsrb1(in_bst, ph_inter_slice_allowed_flag, 1);
				if (ph_inter_slice_allowed_flag) {
					bsrb1(in_bst, ph_intra_slice_allowed_flag, 1);
				}
				nal_read_ue1(in_bst, ph_pic_parameter_set_id);

				if ((pps = m_pCtx->GetVVCPPS(ph_pic_parameter_set_id)) == nullptr ||
					pps->ptr_pic_parameter_set_rbsp == nullptr ||
					(sps = m_pCtx->GetVVCSPS(pps->ptr_pic_parameter_set_rbsp->pps_seq_parameter_set_id)) == nullptr ||
					sps->ptr_seq_parameter_set_rbsp == nullptr)
					return RET_CODE_ERROR_NOTIMPL;

				bsrb1(in_bst, ph_pic_order_cnt_lsb, sps->ptr_seq_parameter_set_rbsp->sps_log2_max_pic_order_cnt_lsb_minus4 + 4);
				if (ph_gdr_pic_flag) {
					nal_read_ue1(in_bst, ph_recovery_poc_cnt);
				}

				for (int i = 0; i < sps->ptr_seq_parameter_set_rbsp->NumExtraPhBits; i++) {
					bsrbarray(in_bst, ph_extra_bit, i);
				}

				if (sps->ptr_seq_parameter_set_rbsp->sps_poc_msb_cycle_flag) {
					bsrb1(in_bst, ph_poc_msb_cycle_present_flag, 1);
					if (ph_poc_msb_cycle_present_flag) {
						bsrb1(in_bst, ph_poc_msb_cycle_val, sps->ptr_seq_parameter_set_rbsp->sps_poc_msb_cycle_len_minus1 + 1);
					}
				}

				if (sps->ptr_seq_parameter_set_rbsp->sps_alf_enabled_flag && pps->ptr_pic_parameter_set_rbsp->pps_alf_info_in_ph_flag) {
					bsrb1(in_bst, ph_alf_enabled_flag, 1);
					if (ph_alf_enabled_flag) {
						bsrb1(in_bst, ph_num_alf_aps_ids_luma, 3);
						ph_alf_aps_id_luma.resize(ph_num_alf_aps_ids_luma);
						for (int i = 0; i < ph_num_alf_aps_ids_luma; i++){
							bsrb1(in_bst, ph_alf_aps_id_luma[i], 3);
						}
						if (sps->ptr_seq_parameter_set_rbsp->sps_chroma_format_idc != 0) {
							bsrb1(in_bst, ph_alf_cb_enabled_flag, 1);
							bsrb1(in_bst, ph_alf_cr_enabled_flag, 1);
						}

						if (ph_alf_cb_enabled_flag || ph_alf_cr_enabled_flag) {
							bsrb1(in_bst, ph_alf_aps_id_chroma, 3);
						}

						if (sps->ptr_seq_parameter_set_rbsp->sps_ccalf_enabled_flag) {
							bsrb1(in_bst, ph_alf_cc_cb_enabled_flag, 1);
							if (ph_alf_cc_cb_enabled_flag) {
								bsrb1(in_bst, ph_alf_cc_cb_aps_id, 3);
							}
							bsrb1(in_bst, ph_alf_cc_cr_enabled_flag, 1);
							if (ph_alf_cc_cr_enabled_flag) {
								bsrb1(in_bst, ph_alf_cc_cr_aps_id, 3);
							}
						}
					}
				}

				if (sps->ptr_seq_parameter_set_rbsp->sps_lmcs_enabled_flag) {
					bsrb1(in_bst, ph_lmcs_enabled_flag, 1);
					if (ph_lmcs_enabled_flag) {
						bsrb1(in_bst, ph_lmcs_aps_id, 2);
						if (sps->ptr_seq_parameter_set_rbsp->sps_chroma_format_idc != 0) {
							bsrb1(in_bst, ph_chroma_residual_scale_flag, 1);
						}
					}
				}

				if (sps->ptr_seq_parameter_set_rbsp->sps_explicit_scaling_list_enabled_flag) {
					bsrb1(in_bst, ph_explicit_scaling_list_enabled_flag, 1);
					if (ph_explicit_scaling_list_enabled_flag) {
						bsrb1(in_bst, ph_scaling_list_aps_id, 3);
					}
				}

				if (sps->ptr_seq_parameter_set_rbsp->sps_virtual_boundaries_enabled_flag && !sps->ptr_seq_parameter_set_rbsp->sps_virtual_boundaries_present_flag) {
					bsrb1(in_bst, ph_virtual_boundaries_present_flag, 1);
					if (ph_virtual_boundaries_present_flag) {
						nal_read_ue1(in_bst, ph_num_ver_virtual_boundaries);
						ph_virtual_boundary_pos_x_minus1.resize(ph_num_ver_virtual_boundaries);
						for (int i = 0; i < ph_num_ver_virtual_boundaries; i++) {
							nal_read_ue1(in_bst, ph_virtual_boundary_pos_x_minus1[i]);
						}

						nal_read_ue1(in_bst, ph_num_hor_virtual_boundaries);
						ph_virtual_boundary_pos_y_minus1.resize(ph_num_hor_virtual_boundaries);
						for (int i = 0; i < ph_num_hor_virtual_boundaries; i++) {
							nal_read_ue1(in_bst, ph_virtual_boundary_pos_y_minus1[i]);
						}
					}
				}

				if (pps->ptr_pic_parameter_set_rbsp->pps_output_flag_present_flag && !ph_non_ref_pic_flag) {
					bsrb1(in_bst, ph_pic_output_flag, 1);
				}

				if (pps->ptr_pic_parameter_set_rbsp->pps_rpl_info_in_ph_flag) {
					bsrbreadref(in_bst, ref_pic_lists, REF_PIC_LISTS, m_pCtx, ph_pic_parameter_set_id);
				}

				if (sps->ptr_seq_parameter_set_rbsp->sps_partition_constraints_override_enabled_flag) {
					bsrb1(in_bst, ph_partition_constraints_override_flag, 1);
				}

				ph_log2_diff_min_qt_min_cb_intra_slice_luma = sps->ptr_seq_parameter_set_rbsp->sps_log2_diff_min_qt_min_cb_intra_slice_luma;
				ph_max_mtt_hierarchy_depth_intra_slice_luma = sps->ptr_seq_parameter_set_rbsp->sps_max_mtt_hierarchy_depth_intra_slice_luma;
				ph_log2_diff_max_bt_min_qt_intra_slice_luma = sps->ptr_seq_parameter_set_rbsp->sps_log2_diff_max_bt_min_qt_intra_slice_luma;
				ph_log2_diff_max_tt_min_qt_intra_slice_luma = sps->ptr_seq_parameter_set_rbsp->sps_log2_diff_max_tt_min_qt_intra_slice_luma;
				ph_log2_diff_min_qt_min_cb_intra_slice_chroma = sps->ptr_seq_parameter_set_rbsp->sps_log2_diff_min_qt_min_cb_intra_slice_chroma;
				ph_max_mtt_hierarchy_depth_intra_slice_chroma = sps->ptr_seq_parameter_set_rbsp->sps_max_mtt_hierarchy_depth_intra_slice_chroma;
				ph_log2_diff_max_bt_min_qt_intra_slice_chroma = sps->ptr_seq_parameter_set_rbsp->sps_log2_diff_max_bt_min_qt_intra_slice_chroma;
				ph_log2_diff_max_tt_min_qt_intra_slice_chroma = sps->ptr_seq_parameter_set_rbsp->sps_log2_diff_max_tt_min_qt_intra_slice_chroma;
				if (ph_intra_slice_allowed_flag) {
					if (ph_partition_constraints_override_flag) {
						nal_read_ue1(in_bst, ph_log2_diff_min_qt_min_cb_intra_slice_luma);
						nal_read_ue1(in_bst, ph_max_mtt_hierarchy_depth_intra_slice_luma);
						if (ph_max_mtt_hierarchy_depth_intra_slice_luma != 0) {
							nal_read_ue1(in_bst, ph_log2_diff_max_bt_min_qt_intra_slice_luma);
							nal_read_ue1(in_bst, ph_log2_diff_max_tt_min_qt_intra_slice_luma);
						}

						if (sps->ptr_seq_parameter_set_rbsp->sps_qtbtt_dual_tree_intra_flag) {
							nal_read_ue1(in_bst, ph_log2_diff_min_qt_min_cb_intra_slice_chroma);
							nal_read_ue1(in_bst, ph_log2_diff_max_bt_min_qt_intra_slice_chroma);
							if (ph_max_mtt_hierarchy_depth_intra_slice_chroma != 0) {
								nal_read_ue1(in_bst, ph_log2_diff_max_bt_min_qt_intra_slice_chroma);
								nal_read_ue1(in_bst, ph_log2_diff_max_tt_min_qt_intra_slice_chroma);
							}
						}
					}

					if (pps->ptr_pic_parameter_set_rbsp->pps_cu_qp_delta_enabled_flag) {
						nal_read_ue1(in_bst, ph_cu_qp_delta_subdiv_intra_slice);
					}

					if (pps->ptr_pic_parameter_set_rbsp->pps_cu_chroma_qp_offset_list_enabled_flag) {
						nal_read_ue1(in_bst, ph_cu_chroma_qp_offset_subdiv_intra_slice);
					}
				}

				ph_log2_diff_min_qt_min_cb_inter_slice = sps->ptr_seq_parameter_set_rbsp->sps_log2_diff_min_qt_min_cb_inter_slice;
				ph_max_mtt_hierarchy_depth_inter_slice = sps->ptr_seq_parameter_set_rbsp->sps_max_mtt_hierarchy_depth_inter_slice;
				ph_log2_diff_max_bt_min_qt_inter_slice = sps->ptr_seq_parameter_set_rbsp->sps_log2_diff_max_bt_min_qt_inter_slice;
				ph_log2_diff_max_tt_min_qt_inter_slice = sps->ptr_seq_parameter_set_rbsp->sps_log2_diff_max_tt_min_qt_inter_slice;
				if (ph_inter_slice_allowed_flag) {
					if (ph_partition_constraints_override_flag) {
						nal_read_ue1(in_bst, ph_log2_diff_min_qt_min_cb_inter_slice);
						nal_read_ue1(in_bst, ph_max_mtt_hierarchy_depth_inter_slice);
						if (ph_max_mtt_hierarchy_depth_inter_slice != 0) {
							nal_read_ue1(in_bst, ph_log2_diff_max_bt_min_qt_inter_slice);
							nal_read_ue1(in_bst, ph_log2_diff_max_tt_min_qt_inter_slice);
						}
					}

					if (pps->ptr_pic_parameter_set_rbsp->pps_cu_qp_delta_enabled_flag) {
						nal_read_ue1(in_bst, ph_cu_qp_delta_subdiv_inter_slice);
					}

					if (pps->ptr_pic_parameter_set_rbsp->pps_cu_chroma_qp_offset_list_enabled_flag) {
						nal_read_ue1(in_bst, ph_cu_chroma_qp_offset_subdiv_inter_slice);
					}

					uint8_t cur_num_ref_entries_0 = m_pCtx->RplsIdx[0] >= 0 && m_pCtx->ref_pic_list_struct[0][m_pCtx->RplsIdx[0]]
						? m_pCtx->ref_pic_list_struct[0][m_pCtx->RplsIdx[0]]->num_ref_entries : 0;
					uint8_t cur_num_ref_entries_1 = m_pCtx->RplsIdx[1] >= 0 && m_pCtx->ref_pic_list_struct[1][m_pCtx->RplsIdx[1]]
						? m_pCtx->ref_pic_list_struct[1][m_pCtx->RplsIdx[1]]->num_ref_entries : 0;
					if (sps->ptr_seq_parameter_set_rbsp->sps_temporal_mvp_enabled_flag) {
						bsrb1(in_bst, ph_temporal_mvp_enabled_flag, 1);
						if (ph_temporal_mvp_enabled_flag && pps->ptr_pic_parameter_set_rbsp->pps_rpl_info_in_ph_flag) {
							if (cur_num_ref_entries_1 > 0){
								bsrb1(in_bst, ph_collocated_from_l0_flag, 1);
							}

							if ((ph_collocated_from_l0_flag && cur_num_ref_entries_0 > 1) ||
								(!ph_collocated_from_l0_flag && cur_num_ref_entries_1 > 1)) {
								nal_read_ue1(in_bst, ph_collocated_ref_idx);
							}
						}
					}

					if (sps->ptr_seq_parameter_set_rbsp->sps_mmvd_fullpel_only_enabled_flag) {
						bsrb1(in_bst, ph_mmvd_fullpel_only_flag, 1);
					}

					uint8_t presenceFlag = 0;
					/* This condition is intentionally not merged into the next, to avoid possible interpretation of RplsIdx[ i ] not having a specified value. */
					if (!pps->ptr_pic_parameter_set_rbsp->pps_rpl_info_in_ph_flag) {
						presenceFlag = 1;
					}
					else if (cur_num_ref_entries_1 > 0){
						presenceFlag = 1;
					}

					if (presenceFlag) {
						bsrb1(in_bst, ph_mvd_l1_zero_flag, 1);
						if (sps->ptr_seq_parameter_set_rbsp->sps_bdof_control_present_in_ph_flag) {
							bsrb1(in_bst, ph_bdof_disabled_flag, 1);
						}
						else
							ph_bdof_disabled_flag = 1 - sps->ptr_seq_parameter_set_rbsp->sps_bdof_enabled_flag;

						if (sps->ptr_seq_parameter_set_rbsp->sps_dmvr_control_present_in_ph_flag) {
							bsrb1(in_bst, ph_dmvr_disabled_flag, 1);
						}
						else
							ph_dmvr_disabled_flag = 1 - sps->ptr_seq_parameter_set_rbsp->sps_dmvr_enabled_flag;
					}
					
					if (sps->ptr_seq_parameter_set_rbsp->sps_prof_control_present_in_ph_flag) {
						bsrb1(in_bst, ph_prof_disabled_flag, 1);
					}
					else
						ph_prof_disabled_flag = sps->ptr_seq_parameter_set_rbsp->sps_affine_prof_enabled_flag ? 0 : 1;

					if ((pps->ptr_pic_parameter_set_rbsp->pps_weighted_pred_flag || pps->ptr_pic_parameter_set_rbsp->pps_weighted_bipred_flag) && 
						 pps->ptr_pic_parameter_set_rbsp->pps_wp_info_in_ph_flag)
					{
						bsrbreadref(in_bst, pred_weight_table, PRED_WEIGHT_TABLE, m_pCtx, ph_pic_parameter_set_id);
					}
				}

				if (pps->ptr_pic_parameter_set_rbsp->pps_qp_delta_info_in_ph_flag) {
					nal_read_se1(in_bst, ph_qp_delta);
				}

				if (sps->ptr_seq_parameter_set_rbsp->sps_joint_cbcr_enabled_flag) {
					bsrb1(in_bst, ph_joint_cbcr_sign_flag, 1);
				}

				if (sps->ptr_seq_parameter_set_rbsp->sps_sao_enabled_flag && pps->ptr_pic_parameter_set_rbsp->pps_sao_info_in_ph_flag) {
					bsrb1(in_bst, ph_sao_luma_enabled_flag, 1);
					if (sps->ptr_seq_parameter_set_rbsp->sps_chroma_format_idc != 0) {
						bsrb1(in_bst, ph_sao_chroma_enabled_flag, 1);
					}
				}

				ph_deblocking_filter_disabled_flag = pps->ptr_pic_parameter_set_rbsp->pps_deblocking_filter_disabled_flag;
				if(pps->ptr_pic_parameter_set_rbsp->pps_chroma_tool_offsets_present_flag)
				{
					ph_cb_beta_offset_div2 = ph_cr_beta_offset_div2 = pps->ptr_pic_parameter_set_rbsp->pps_cb_beta_offset_div2;
					ph_cb_tc_offset_div2   = ph_cr_tc_offset_div2   = pps->ptr_pic_parameter_set_rbsp->pps_cb_tc_offset_div2;
				}

				if (pps->ptr_pic_parameter_set_rbsp->pps_dbf_info_in_ph_flag) {
					bsrb1(in_bst, ph_deblocking_params_present_flag, 1);
					if (ph_deblocking_params_present_flag) {
						if (!pps->ptr_pic_parameter_set_rbsp->pps_deblocking_filter_disabled_flag) {
							bsrb1(in_bst, ph_deblocking_filter_disabled_flag, 1);
						}
						else
							ph_deblocking_filter_disabled_flag = 0;

						ph_luma_beta_offset_div2 = pps->ptr_pic_parameter_set_rbsp->pps_luma_beta_offset_div2;
						ph_luma_tc_offset_div2 = pps->ptr_pic_parameter_set_rbsp->pps_luma_tc_offset_div2;
						if (!ph_deblocking_filter_disabled_flag) {
							nal_read_se1(in_bst, ph_luma_beta_offset_div2);
							nal_read_se1(in_bst, ph_luma_tc_offset_div2);
							if (pps->ptr_pic_parameter_set_rbsp->pps_chroma_tool_offsets_present_flag) {
								nal_read_se1(in_bst, ph_cb_beta_offset_div2);
								nal_read_se1(in_bst, ph_cb_tc_offset_div2);
								nal_read_se1(in_bst, ph_cr_beta_offset_div2);
								nal_read_se1(in_bst, ph_cr_tc_offset_div2);
							}
							else
							{
								ph_cb_beta_offset_div2 = ph_cr_beta_offset_div2 = ph_luma_beta_offset_div2;
								ph_cb_tc_offset_div2 = ph_cr_tc_offset_div2 = ph_luma_tc_offset_div2;
							}
						}
					}
				}

				if (pps->ptr_pic_parameter_set_rbsp->pps_picture_header_extension_present_flag) {
					nal_read_ue1(in_bst, ph_extension_length);
					if (ph_extension_length > 0)
					{
						ph_extension_data_byte.resize(ph_extension_length);
						for (int i = 0; i < ph_extension_length; i++) {
							bsrb1(in_bst, ph_extension_data_byte[i], 8);
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

		int NAL_UNIT::PICTURE_HEADER_STRUCTURE::Unmap(AMBst out_bst)
		{
			UNREFERENCED_PARAMETER(out_bst);
			return RET_CODE_ERROR_NOTIMPL;
		}

		int NAL_UNIT::PICTURE_HEADER_RBSP::Map(AMBst in_bst)
		{
			int iRet = RET_CODE_SUCCESS;
			SYNTAX_BITSTREAM_MAP::Map(in_bst);

			try
			{
				MAP_BST_BEGIN(0);
				nal_read_obj(in_bst, picture_header_structure);
				MAP_BST_END();
			}
			catch (AMException e)
			{
				return e.RetCode();
			}

			return RET_CODE_SUCCESS;
		}

		int NAL_UNIT::PICTURE_HEADER_RBSP::Unmap(AMBst out_bst)
		{
			UNREFERENCED_PARAMETER(out_bst);
			return RET_CODE_ERROR_NOTIMPL;
		}

		DECLARE_FIELDPROP_BEGIN1(NAL_UNIT::PICTURE_HEADER_RBSP)

		DECLARE_FIELDPROP_END()

		//
		// slice_layer_rbsp()
		//
		NAL_UNIT::SLICE_HEADER::SLICE_HEADER(VideoBitstreamCtx* pCtx, int8_t nu_type)
			: sh_slice_address(0), sh_num_tiles_in_slice_minus1(0), sh_cu_chroma_qp_offset_enabled_flag(0)
			, sh_deblocking_params_present_flag(0), sh_dep_quant_used_flag(0), sh_deblocking_filter_disabled_flag(0)
			, sh_cb_qp_offset(0), sh_cr_qp_offset(0), sh_joint_cbcr_qp_offset(0)
			, sh_sign_data_hiding_used_flag(0), sh_ts_residual_coding_disabled_flag(0), m_pCtx(pCtx), nal_unit_type(nu_type){
		}

		NAL_UNIT::SLICE_HEADER::~SLICE_HEADER() {

		}

		int NAL_UNIT::SLICE_HEADER::Map(AMBst in_bst)
		{
			int iRet = RET_CODE_SUCCESS;
			SYNTAX_BITSTREAM_MAP::Map(in_bst);
			H266_NU ph_nu, pps_nu, sps_nu;
			PICTURE_HEADER_STRUCTURE* pPHStructure = nullptr;

			try
			{
				MAP_BST_BEGIN(0);
				bsrb1(in_bst, sh_picture_header_in_slice_header_flag, 1);
				if (sh_picture_header_in_slice_header_flag)
				{
					bsrbreadref(in_bst, picture_header_structure, PICTURE_HEADER_STRUCTURE, m_pCtx);
					pPHStructure = picture_header_structure;
				}
				else
				{
					ph_nu = m_pCtx->GetVVCPH();
					if (ph_nu == nullptr || ph_nu->ptr_picture_header_rbsp == nullptr)
						return RET_CODE_ERROR_NOTIMPL;

					pPHStructure = &ph_nu->ptr_picture_header_rbsp->picture_header_structure;
				}

				if ((pps_nu = m_pCtx->GetVVCPPS(pPHStructure->ph_pic_parameter_set_id)) == nullptr ||
					(pps_nu->ptr_pic_parameter_set_rbsp == nullptr) ||
					(sps_nu = m_pCtx->GetVVCSPS(pps_nu->ptr_pic_parameter_set_rbsp->pps_seq_parameter_set_id)) == nullptr ||
					(sps_nu->ptr_seq_parameter_set_rbsp == nullptr))
				{
					return RET_CODE_ERROR_NOTIMPL;
				}

				if (sps_nu->ptr_seq_parameter_set_rbsp->sps_subpic_info_present_flag) {
					bsrb1(in_bst, sh_subpic_id, sps_nu->ptr_seq_parameter_set_rbsp->sps_subpic_id_len_minus1 + 1);
				}

				int CurrSubpicIdx = 0;
				if (sps_nu->ptr_seq_parameter_set_rbsp->sps_subpic_info_present_flag) {
					uint32_t sh_subpic_id = (uint32_t)AMBst_GetBits(in_bst, sps_nu->ptr_seq_parameter_set_rbsp->sps_subpic_id_len_minus1 + 1);

					for (; CurrSubpicIdx <= sps_nu->ptr_seq_parameter_set_rbsp->sps_num_subpics_minus1; CurrSubpicIdx++)
						if (pps_nu->ptr_pic_parameter_set_rbsp->SubpicIdVal[CurrSubpicIdx] == sh_subpic_id)
							break;
				}

				uint32_t NumTilesInPic = pps_nu->ptr_pic_parameter_set_rbsp->NumTileRows*pps_nu->ptr_pic_parameter_set_rbsp->NumTileColumns;
				if ((pps_nu->ptr_pic_parameter_set_rbsp->pps_rect_slice_flag && 
					 pps_nu->ptr_pic_parameter_set_rbsp->NumSlicesInSubpic.size() > 0 && 
					 pps_nu->ptr_pic_parameter_set_rbsp->NumSlicesInSubpic[CurrSubpicIdx] > 1)
					|| (!pps_nu->ptr_pic_parameter_set_rbsp->pps_rect_slice_flag && NumTilesInPic > 1))
				{
					if (!pps_nu->ptr_pic_parameter_set_rbsp->pps_rect_slice_flag) {
						bsrb1(in_bst, sh_slice_address, quick_ceil_log2(NumTilesInPic));
					}
					else
					{
						bsrb1(in_bst, sh_slice_address, quick_ceil_log2(pps_nu->ptr_pic_parameter_set_rbsp->NumSlicesInSubpic[CurrSubpicIdx]));
					}
				}

				for (int i = 0; i < sps_nu->ptr_seq_parameter_set_rbsp->NumExtraShBits; i++) {
					bsrbarray(in_bst, sh_extra_bit, i);
				}

				if (!pps_nu->ptr_pic_parameter_set_rbsp->pps_rect_slice_flag && NumTilesInPic - sh_slice_address > 1) {
					nal_read_ue1(in_bst, sh_num_tiles_in_slice_minus1);
				}

				if (pPHStructure->ph_inter_slice_allowed_flag) {
					nal_read_ue1(in_bst, sh_slice_type);
				}
				else
					sh_slice_type = VVC_I_SLICE;

				if (nal_unit_type == IDR_W_RADL || nal_unit_type == IDR_N_LP || nal_unit_type == CRA_NUT || nal_unit_type == GDR_NUT) {
					bsrb1(in_bst, sh_no_output_of_prior_pics_flag, 1);
				}

				if (pps_nu->ptr_pic_parameter_set_rbsp->pps_rect_slice_flag) 
				{
					uint32_t picLevelSliceIdx = sh_slice_address;
					if (pps_nu->ptr_pic_parameter_set_rbsp->NumSlicesInSubpic.size() > 0)
					{
						for (int j = 0; j < CurrSubpicIdx; j++)
							picLevelSliceIdx += pps_nu->ptr_pic_parameter_set_rbsp->NumSlicesInSubpic[j];

						NumCtusInCurrSlice = pps_nu->ptr_pic_parameter_set_rbsp->NumCtusInSlice[picLevelSliceIdx];
						CtbAddrInCurrSlice.resize(NumCtusInCurrSlice);
						for (int i = 0; i < NumCtusInCurrSlice; i++)
							CtbAddrInCurrSlice[i] = pps_nu->ptr_pic_parameter_set_rbsp->CtbAddrInSlice[picLevelSliceIdx][i];
					}
				}
				else {
					NumCtusInCurrSlice = 0;
					for (uint32_t tileIdx = sh_slice_address; tileIdx <= sh_slice_address + sh_num_tiles_in_slice_minus1; tileIdx++) {
						uint16_t tileX = tileIdx % pps_nu->ptr_pic_parameter_set_rbsp->NumTileColumns;
						uint16_t tileY = tileIdx / pps_nu->ptr_pic_parameter_set_rbsp->NumTileColumns;
						for (uint32_t ctbY = pps_nu->ptr_pic_parameter_set_rbsp->TileRowBdVal[tileY]; 
									  ctbY < pps_nu->ptr_pic_parameter_set_rbsp->TileRowBdVal[(size_t)tileY + 1]; ctbY++)
						{
							for (uint32_t ctbX = pps_nu->ptr_pic_parameter_set_rbsp->TileColBdVal[tileX];
										  ctbX < pps_nu->ptr_pic_parameter_set_rbsp->TileColBdVal[(size_t)tileX + 1]; ctbX++) 
							{
								CtbAddrInCurrSlice.push_back(ctbY * pps_nu->ptr_pic_parameter_set_rbsp->PicWidthInCtbsY + ctbX);
								NumCtusInCurrSlice++;
							}
						}
					}
				}

				uint8_t MinCbLog2SizeY = sps_nu->ptr_seq_parameter_set_rbsp->sps_log2_min_luma_coding_block_size_minus2 + 2;
				if (sh_slice_type == VVC_I_SLICE)
				{
					MinQtLog2SizeY = MinCbLog2SizeY + pPHStructure->ph_log2_diff_min_qt_min_cb_intra_slice_luma;
					MinQtLog2SizeC = MinCbLog2SizeY + pPHStructure->ph_log2_diff_min_qt_min_cb_intra_slice_chroma;
					MaxBtSizeY = 1 << (MinQtLog2SizeY + pPHStructure->ph_log2_diff_max_bt_min_qt_intra_slice_luma);
					MaxBtSizeC = 1 << (MinQtLog2SizeC + pPHStructure->ph_log2_diff_max_bt_min_qt_intra_slice_chroma);
					MaxTtSizeY = 1 << (MinQtLog2SizeY + pPHStructure->ph_log2_diff_max_tt_min_qt_intra_slice_luma);
					MaxTtSizeC = 1 << (MinQtLog2SizeC + pPHStructure->ph_log2_diff_max_tt_min_qt_intra_slice_chroma);
					MaxMttDepthY = pPHStructure->ph_max_mtt_hierarchy_depth_intra_slice_luma;
					MaxMttDepthC = pPHStructure->ph_max_mtt_hierarchy_depth_intra_slice_chroma;
					CuQpDeltaSubdiv = pPHStructure->ph_cu_qp_delta_subdiv_intra_slice;
					CuChromaQpOffsetSubdiv = pPHStructure->ph_cu_chroma_qp_offset_subdiv_intra_slice;
				}
				else
				{
					MinQtLog2SizeY = MinCbLog2SizeY + pPHStructure->ph_log2_diff_min_qt_min_cb_inter_slice;
					MinQtLog2SizeC = MinCbLog2SizeY + pPHStructure->ph_log2_diff_min_qt_min_cb_inter_slice;
					MaxBtSizeY = 1 << (MinQtLog2SizeY + pPHStructure->ph_log2_diff_max_bt_min_qt_inter_slice);
					MaxBtSizeC = 1 << (MinQtLog2SizeC + pPHStructure->ph_log2_diff_max_bt_min_qt_inter_slice);
					MaxTtSizeY = 1 << (MinQtLog2SizeY + pPHStructure->ph_log2_diff_max_tt_min_qt_inter_slice);
					MaxTtSizeC = 1 << (MinQtLog2SizeC + pPHStructure->ph_log2_diff_max_tt_min_qt_inter_slice);
					MaxMttDepthY = pPHStructure->ph_max_mtt_hierarchy_depth_inter_slice;
					MaxMttDepthC = pPHStructure->ph_max_mtt_hierarchy_depth_inter_slice;
					CuQpDeltaSubdiv = pPHStructure->ph_cu_qp_delta_subdiv_inter_slice;
					CuChromaQpOffsetSubdiv = pPHStructure->ph_cu_chroma_qp_offset_subdiv_inter_slice;
				}

				MinQtSizeY = 1 << MinQtLog2SizeY;
				MinQtSizeC = 1 << MinQtLog2SizeC;
				MinBtSizeY = 1 << MinCbLog2SizeY;
				MinTtSizeY = 1 << MinCbLog2SizeY;

				if (sps_nu->ptr_seq_parameter_set_rbsp->sps_alf_enabled_flag && !pps_nu->ptr_pic_parameter_set_rbsp->pps_alf_info_in_ph_flag) {
					bsrb1(in_bst, sh_alf_enabled_flag, 1);
					if (sh_alf_enabled_flag) {
						bsrb1(in_bst, sh_num_alf_aps_ids_luma, 3);
						for (uint16_t i = 0; i < sh_num_alf_aps_ids_luma; i++) {
							bsrb1(in_bst, sh_alf_aps_id_luma[i], 3);
						}

						if (sps_nu->ptr_seq_parameter_set_rbsp->sps_chroma_format_idc != 0) {
							bsrb1(in_bst, sh_alf_cb_enabled_flag, 1);
							bsrb1(in_bst, sh_alf_cr_enabled_flag, 1);
						}
						else
						{
							sh_alf_cb_enabled_flag = pPHStructure->ph_alf_cb_enabled_flag;
							sh_alf_cr_enabled_flag = pPHStructure->ph_alf_cr_enabled_flag;
						}

						if (sh_alf_cb_enabled_flag || sh_alf_cr_enabled_flag) {
							bsrb1(in_bst, sh_alf_aps_id_chroma, 3);
						}
						else
							sh_alf_aps_id_chroma = pPHStructure->ph_alf_aps_id_chroma;

						if (sps_nu->ptr_seq_parameter_set_rbsp->sps_ccalf_enabled_flag) {
							bsrb1(in_bst, sh_alf_cc_cb_enabled_flag, 1);
							if (sh_alf_cc_cb_enabled_flag) {
								bsrb1(in_bst, sh_alf_cc_cb_aps_id, 3);
							}
							else
								sh_alf_cc_cb_aps_id = pPHStructure->ph_alf_cc_cb_aps_id;

							bsrb1(in_bst, sh_alf_cc_cr_enabled_flag, 1);
							if (sh_alf_cc_cr_enabled_flag) {
								bsrb1(in_bst, sh_alf_cc_cr_aps_id, 3);
							}
							else
								sh_alf_cc_cr_aps_id = pPHStructure->ph_alf_cc_cr_aps_id;
						}
					}
				}
				else
					sh_alf_enabled_flag = pPHStructure->ph_alf_enabled_flag;

				if (pPHStructure->ph_lmcs_enabled_flag && !sh_picture_header_in_slice_header_flag)
				{
					bsrb1(in_bst, sh_lmcs_used_flag, 1);
				}
				else
					sh_lmcs_used_flag = sh_picture_header_in_slice_header_flag ? pPHStructure->ph_lmcs_enabled_flag : 0;

				if (pPHStructure->ph_explicit_scaling_list_enabled_flag && !sh_picture_header_in_slice_header_flag) {
					bsrb1(in_bst, sh_explicit_scaling_list_used_flag, 1);
				}
				else
					sh_explicit_scaling_list_used_flag = sh_picture_header_in_slice_header_flag ? pPHStructure->ph_explicit_scaling_list_enabled_flag : 0;

				if (!pps_nu->ptr_pic_parameter_set_rbsp->pps_rpl_info_in_ph_flag
					&& ((nal_unit_type != IDR_W_RADL && nal_unit_type != IDR_N_LP) || sps_nu->ptr_seq_parameter_set_rbsp->sps_idr_rpl_present_flag)) {
					bsrbreadref(in_bst, ref_pic_lists, REF_PIC_LISTS, m_pCtx, pps_nu->ptr_pic_parameter_set_rbsp->pps_pic_parameter_set_id);
				}

				std::shared_ptr<REF_PIC_LIST_STRUCT> curr_ref_pic_list[2] = 
					{ m_pCtx->ref_pic_list_struct[0][m_pCtx->RplsIdx[0]], m_pCtx->ref_pic_list_struct[1][m_pCtx->RplsIdx[1]] };
				if ((sh_slice_type != VVC_I_SLICE && curr_ref_pic_list[0] && curr_ref_pic_list[0]->num_ref_entries > 1) || 
					(sh_slice_type == VVC_B_SLICE && curr_ref_pic_list[1] && curr_ref_pic_list[1]->num_ref_entries > 1)) {
					bsrb1(in_bst, sh_num_ref_idx_active_override_flag, 1);
				}
				else
					sh_num_ref_idx_active_override_flag = 1;

				if (sh_num_ref_idx_active_override_flag) {
					for (uint8_t i = 0; i < (sh_slice_type == VVC_B_SLICE ? 2 : 1); i++) {
						if (curr_ref_pic_list[i] && curr_ref_pic_list[i]->num_ref_entries > 1) {
							nal_read_ue1(in_bst, sh_num_ref_idx_active_minus1[i]);
						}
					}
				}

				if (sh_slice_type != VVC_I_SLICE) {
					if (pps_nu->ptr_pic_parameter_set_rbsp->pps_cabac_init_present_flag) {
						bsrb1(in_bst, sh_cabac_init_flag, 1);
					}
					else
						sh_cabac_init_flag = 0;

					if (sh_slice_type == VVC_B_SLICE)
						sh_collocated_from_l0_flag = pPHStructure->ph_collocated_from_l0_flag;
					else
						sh_collocated_from_l0_flag = 1;

					if (pPHStructure->ph_temporal_mvp_enabled_flag && !pps_nu->ptr_pic_parameter_set_rbsp->pps_rpl_info_in_ph_flag) {
						if (sh_slice_type == VVC_B_SLICE) {
							bsrb1(in_bst, sh_collocated_from_l0_flag, 1);
						}

						if (pps_nu->ptr_pic_parameter_set_rbsp->pps_rpl_info_in_ph_flag)
							sh_collocated_ref_idx = pPHStructure->ph_collocated_ref_idx;
						else
							sh_collocated_ref_idx = 0;
					
						if ((sh_collocated_from_l0_flag && m_pCtx->NumRefIdxActive[0] > 1) || (!sh_collocated_from_l0_flag && m_pCtx->NumRefIdxActive[1] > 1)) {
							nal_read_ue1(in_bst, sh_collocated_ref_idx);
						}
					}

					if (!pps_nu->ptr_pic_parameter_set_rbsp->pps_wp_info_in_ph_flag && ((pps_nu->ptr_pic_parameter_set_rbsp->pps_weighted_pred_flag && sh_slice_type == VVC_P_SLICE) ||
						(pps_nu->ptr_pic_parameter_set_rbsp->pps_weighted_bipred_flag && sh_slice_type == VVC_B_SLICE)))
					{
						bsrbreadref(in_bst, pred_weight_table, PRED_WEIGHT_TABLE, m_pCtx, pps_nu->ptr_pic_parameter_set_rbsp->pps_pic_parameter_set_id);
					}
				}

				if (!pps_nu->ptr_pic_parameter_set_rbsp->pps_qp_delta_info_in_ph_flag) {
					nal_read_se1(in_bst, sh_qp_delta);
				}

				if (pps_nu->ptr_pic_parameter_set_rbsp->pps_slice_chroma_qp_offsets_present_flag) {
					nal_read_se1(in_bst, sh_cb_qp_offset);
					nal_read_se1(in_bst, sh_cr_qp_offset);
					if (sps_nu->ptr_seq_parameter_set_rbsp->sps_joint_cbcr_enabled_flag) {
						nal_read_se1(in_bst, sh_joint_cbcr_qp_offset);
					}
				}

				if (pps_nu->ptr_pic_parameter_set_rbsp->pps_cu_chroma_qp_offset_list_enabled_flag) {
					bsrb1(in_bst, sh_cu_chroma_qp_offset_enabled_flag, 1);
				}

				sh_sao_luma_used_flag = pPHStructure->ph_sao_luma_enabled_flag;
				sh_sao_chroma_used_flag = pPHStructure->ph_sao_chroma_enabled_flag;
				if (sps_nu->ptr_seq_parameter_set_rbsp->sps_sao_enabled_flag && !pps_nu->ptr_pic_parameter_set_rbsp->pps_sao_info_in_ph_flag) {
					bsrb1(in_bst, sh_sao_luma_used_flag, 1);
					if (sps_nu->ptr_seq_parameter_set_rbsp->sps_chroma_format_idc != 0) {
						bsrb1(in_bst, sh_sao_chroma_used_flag, 1);
					}
				}

				if (pps_nu->ptr_pic_parameter_set_rbsp->pps_deblocking_filter_override_enabled_flag && 
					!pps_nu->ptr_pic_parameter_set_rbsp->pps_dbf_info_in_ph_flag) {
					bsrb1(in_bst, sh_deblocking_params_present_flag, 1);
				}

				sh_luma_beta_offset_div2 = pPHStructure->ph_luma_beta_offset_div2;
				sh_luma_tc_offset_div2 = pPHStructure->ph_luma_tc_offset_div2;

				if (pps_nu->ptr_pic_parameter_set_rbsp->pps_chroma_tool_offsets_present_flag) {
					sh_cb_beta_offset_div2 = pPHStructure->ph_cb_beta_offset_div2;
					sh_cb_tc_offset_div2 = pPHStructure->ph_cb_tc_offset_div2;
					sh_cr_beta_offset_div2 = pPHStructure->ph_cr_beta_offset_div2;
					sh_cr_tc_offset_div2 = pPHStructure->ph_cr_tc_offset_div2;
				}

				if (sh_deblocking_params_present_flag) {
					if (!pps_nu->ptr_pic_parameter_set_rbsp->pps_deblocking_filter_disabled_flag) {
						bsrb1(in_bst, sh_deblocking_filter_disabled_flag, 1);
					}
					else
						sh_deblocking_filter_disabled_flag = 0;

					if (!sh_deblocking_filter_disabled_flag) {
						nal_read_se1(in_bst, sh_luma_beta_offset_div2);
						nal_read_se1(in_bst, sh_luma_tc_offset_div2);

						if (pps_nu->ptr_pic_parameter_set_rbsp->pps_chroma_tool_offsets_present_flag) {
							nal_read_se1(in_bst, sh_cb_beta_offset_div2);
							nal_read_se1(in_bst, sh_cb_tc_offset_div2);
							nal_read_se1(in_bst, sh_cr_beta_offset_div2);
							nal_read_se1(in_bst, sh_cr_tc_offset_div2);
						}
						else
						{
							sh_cb_beta_offset_div2 = sh_cr_beta_offset_div2 = sh_luma_beta_offset_div2;
							sh_cb_tc_offset_div2 = sh_cr_tc_offset_div2 = sh_luma_tc_offset_div2;
						}
					}
				}
				else
					sh_deblocking_filter_disabled_flag = pPHStructure->ph_deblocking_filter_disabled_flag;

				if (sps_nu->ptr_seq_parameter_set_rbsp->sps_dep_quant_enabled_flag) {
					bsrb1(in_bst, sh_dep_quant_used_flag, 1);
				}

				if (sps_nu->ptr_seq_parameter_set_rbsp->sps_sign_data_hiding_enabled_flag && !sh_dep_quant_used_flag) {
					bsrb1(in_bst, sh_sign_data_hiding_used_flag, 1);
				}

				if (sps_nu->ptr_seq_parameter_set_rbsp->sps_transform_skip_enabled_flag && !sh_dep_quant_used_flag && !sh_sign_data_hiding_used_flag) {
					bsrb1(in_bst, sh_ts_residual_coding_disabled_flag, 1);
				}

				if (pps_nu->ptr_pic_parameter_set_rbsp->pps_slice_header_extension_present_flag) {
					nal_read_ue1(in_bst, sh_slice_header_extension_length);
					sh_slice_header_extension_data_byte.resize(sh_slice_header_extension_length);
					for(int i = 0; i < sh_slice_header_extension_length; i++){
						bsrb1(in_bst, sh_slice_header_extension_data_byte[i], 8);
					}
				}

				int NumEntryPoints = 0; 
				if (sps_nu->ptr_seq_parameter_set_rbsp->sps_entry_point_offsets_present_flag){
					for (uint16_t i = 1; i < NumCtusInCurrSlice; i++) {
						uint32_t ctbAddrX = CtbAddrInCurrSlice[i] % pps_nu->ptr_pic_parameter_set_rbsp->PicWidthInCtbsY;
						uint32_t ctbAddrY = CtbAddrInCurrSlice[i] / pps_nu->ptr_pic_parameter_set_rbsp->PicWidthInCtbsY;
						uint32_t prevCtbAddrX = CtbAddrInCurrSlice[i - 1] % pps_nu->ptr_pic_parameter_set_rbsp->PicWidthInCtbsY;
						uint32_t prevCtbAddrY = CtbAddrInCurrSlice[i - 1] / pps_nu->ptr_pic_parameter_set_rbsp->PicWidthInCtbsY;
						if (pps_nu->ptr_pic_parameter_set_rbsp->CtbToTileRowBd[ctbAddrY] != pps_nu->ptr_pic_parameter_set_rbsp->CtbToTileRowBd[prevCtbAddrY]
							|| pps_nu->ptr_pic_parameter_set_rbsp->CtbToTileColBd[ctbAddrX] != pps_nu->ptr_pic_parameter_set_rbsp->CtbToTileColBd[prevCtbAddrX]
							|| (ctbAddrY != prevCtbAddrY && sps_nu->ptr_seq_parameter_set_rbsp->sps_entropy_coding_sync_enabled_flag))
							NumEntryPoints++;
					}
				}

				if (NumEntryPoints > 0)
				{
					nal_read_ue1(in_bst, sh_entry_offset_len_minus1);
					sh_entry_point_offset_minus1.resize(NumEntryPoints);
					for (int i = 0; i < NumEntryPoints; i++) {
						bsrb1(in_bst, sh_entry_point_offset_minus1[i], sh_entry_offset_len_minus1);
					}
				}

				AMBst_Realign(in_bst);

				MAP_BST_END();
			}
			catch (AMException e)
			{
				return e.RetCode();
			}

			return RET_CODE_SUCCESS;
		}

		int NAL_UNIT::SLICE_HEADER::Unmap(AMBst out_bst)
		{
			UNREFERENCED_PARAMETER(out_bst);
			return RET_CODE_ERROR_NOTIMPL;
		}

		int NAL_UNIT::SLICE_LAYER_RBSP::Map(AMBst in_bst)
		{
			int iRet = RET_CODE_SUCCESS;
			SYNTAX_BITSTREAM_MAP::Map(in_bst);

			int left_bits = 0;

			try
			{
				MAP_BST_BEGIN(0);
				nal_read_obj(in_bst, slice_header);

				// TODO...
				// read the slice_data

				AMBst_Tell(in_bst, &left_bits);

				MAP_BST_END();
			}
			catch (AMException e)
			{
				return e.RetCode();
			}

			return RET_CODE_SUCCESS;
		}

		int NAL_UNIT::SLICE_LAYER_RBSP::Unmap(AMBst out_bst)
		{
			UNREFERENCED_PARAMETER(out_bst);
			return RET_CODE_ERROR_NOTIMPL;
		}

		DECLARE_FIELDPROP_BEGIN1(NAL_UNIT::SLICE_LAYER_RBSP)

		DECLARE_FIELDPROP_END()

		NAL_UNIT::~NAL_UNIT() {
			switch (nal_unit_header.nal_unit_type)
			{
			case TRAIL_NUT:
			case STSA_NUT:
			case RADL_NUT:
			case RASL_NUT:
			case IDR_W_RADL:
			case IDR_N_LP:
			case CRA_NUT:
			case GDR_NUT:
				UNMAP_STRUCT_POINTER5(ptr_slice_layer_rbsp);
				break;
			case RSV_VCL_4:
			case RSV_VCL_5:
			case RSV_VCL_6:
			case RSV_IRAP_11:
				break;
			case OPI_NUT:
				UNMAP_STRUCT_POINTER5(ptr_operating_point_information_rbsp);
				break;
			case DCI_NUT:
				UNMAP_STRUCT_POINTER5(ptr_decoding_capability_information_rbsp);
				break;
			case VPS_NUT:
				UNMAP_STRUCT_POINTER5(ptr_video_parameter_set_rbsp);
				break;
			case SPS_NUT:
				UNMAP_STRUCT_POINTER5(ptr_seq_parameter_set_rbsp);
				break;
			case PPS_NUT:
				UNMAP_STRUCT_POINTER5(ptr_pic_parameter_set_rbsp);
				break;
			case PREFIX_APS_NUT:
			case SUFFIX_APS_NUT:
				UNMAP_STRUCT_POINTER5(ptr_adaptation_parameter_set_rbsp);
				break;
			case PH_NUT:
				UNMAP_STRUCT_POINTER5(ptr_picture_header_rbsp);
				break;
			case AUD_NUT:
				UNMAP_STRUCT_POINTER5(ptr_access_unit_delimiter_rbsp);
				break;
			case EOS_NUT:
				break;
			case EOB_NUT:
				break;
			case PREFIX_SEI_NUT:
			case SUFFIX_SEI_NUT:
				UNMAP_STRUCT_POINTER5(ptr_sei_rbsp);
				break;
			case FD_NUT:
				break;
			case RSV_NVCL_26:
				break;
			case RSV_NVCL_27:
				break;
			case UNSPEC_28:
				break;
			case UNSPEC_29:
				break;
			case UNSPEC_30:
				break;
			case UNSPEC_31:
				break;
			}
		}

		int NAL_UNIT::Map(AMBst bst)
		{
			int iRet = RET_CODE_SUCCESS;
			SYNTAX_BITSTREAM_MAP::Map(bst);

			if (AMP_FAILED(iRet = nal_unit_header.Map(bst)))
				return iRet;

			//printf("[H266] %s.\n", vvc_nal_unit_type_descs[nal_unit_header.nal_unit_type]);

			if (ptr_ctx_video_bst != NULL &&
				ptr_ctx_video_bst->IsNUFiltered(nal_unit_header.nal_unit_type) == false)
				return RET_CODE_NOTHING_TODO;

			if (AMP_FAILED(iRet = AMBst_SetRBSPType(bst, BST_RBSP_NAL_UNIT)))
				return iRet;

			try
			{
				MAP_BST_BEGIN(1);
				switch (nal_unit_header.nal_unit_type)
				{
				case TRAIL_NUT:
				case STSA_NUT:
				case RADL_NUT:
				case RASL_NUT:
				case IDR_W_RADL:
				case IDR_N_LP:
				case CRA_NUT:
				case GDR_NUT:
					nal_read_ref(bst, ptr_slice_layer_rbsp, SLICE_LAYER_RBSP, ptr_ctx_video_bst, nal_unit_header.nal_unit_type);
					break;
				case RSV_VCL_4:
				case RSV_VCL_5:
				case RSV_VCL_6:
				case RSV_IRAP_11:
					break;
				case OPI_NUT:
					nal_read_ref(bst, ptr_operating_point_information_rbsp, OPERATING_POINT_INFORMATION_RBSP);
					break;
				case DCI_NUT:
					nal_read_ref(bst, ptr_decoding_capability_information_rbsp, DECODING_CAPABILITY_INFORMATION_RBSP);
					break;
				case VPS_NUT:
					nal_read_ref(bst, ptr_video_parameter_set_rbsp, VIDEO_PARAMETER_SET_RBSP);
					break;
				case SPS_NUT:
					nal_read_ref(bst, ptr_seq_parameter_set_rbsp, SEQ_PARAMETER_SET_RBSP);

					// Try to update ref_pic_struct in the context
					if (ptr_ctx_video_bst != nullptr)
					{
						for (int i = 0; i < 2; i++) {
							for (int j = 0; j < 64; j++) {
								ptr_ctx_video_bst->ref_pic_list_struct[i][j] =
									ptr_seq_parameter_set_rbsp->ref_pic_list_struct[i][j];
							}
						}
					}

					break;
				case PPS_NUT:
					nal_read_ref(bst, ptr_pic_parameter_set_rbsp, PIC_PARAMETER_SET_RBSP, ptr_ctx_video_bst);
					break;
				case PREFIX_APS_NUT:
				case SUFFIX_APS_NUT:
					nal_read_ref(bst, ptr_adaptation_parameter_set_rbsp, ADAPTATION_PARAMETER_SET_RBSP, (uint8_t)nal_unit_header.nal_unit_type);
					break;
				case PH_NUT:
					nal_read_ref(bst, ptr_picture_header_rbsp, PICTURE_HEADER_RBSP, ptr_ctx_video_bst);
					break;
				case AUD_NUT:
					nal_read_ref(bst, ptr_access_unit_delimiter_rbsp, ACCESS_UNIT_DELIMITER_RBSP);
					break;
				case EOS_NUT:
					break;
				case EOB_NUT:
					break;
				case PREFIX_SEI_NUT:
				case SUFFIX_SEI_NUT:
					nal_read_ref(bst, ptr_sei_rbsp, SEI_RBSP, nal_unit_header.nal_unit_type, ptr_ctx_video_bst);
					break;
				case FD_NUT:
					break;
				case RSV_NVCL_26:
					break;
				case RSV_NVCL_27:
					break;
				case UNSPEC_28:
					break;
				case UNSPEC_29:
					break;
				case UNSPEC_30:
					break;
				case UNSPEC_31:
					break;
				}

				MAP_BST_END();
			}
			catch (AMException e)
			{
				AMBst_SetRBSPType(bst, BST_RBSP_SEQUENCE);
				return e.RetCode();
			}

			AMBst_SetRBSPType(bst, BST_RBSP_SEQUENCE);
			SYNTAX_BITSTREAM_MAP::EndMap(bst);

			return RET_CODE_SUCCESS;
		}

		int NAL_UNIT::Unmap(AMBst out_bst)
		{
			UNREFERENCED_PARAMETER(out_bst);
			return RET_CODE_ERROR_NOTIMPL;
		}

		DECLARE_FIELDPROP_BEGIN1(NAL_UNIT)
			NAV_WRITE_TAG_BEGIN2_1("NAL_UNIT", vvc_nal_unit_type_descs[nal_unit_header.nal_unit_type]);
			BST_FIELD_PROP_OBJECT(nal_unit_header);
			switch (nal_unit_header.nal_unit_type)
			{
			case TRAIL_NUT:
			case STSA_NUT:
			case RADL_NUT:
			case RASL_NUT:
			case IDR_W_RADL:
			case IDR_N_LP:
			case CRA_NUT:
			case GDR_NUT:
				NAV_FIELD_PROP_REF(ptr_slice_layer_rbsp);
				break;
			case RSV_VCL_4:
			case RSV_VCL_5:
			case RSV_VCL_6:
			case RSV_IRAP_11:
				break;
			case OPI_NUT:
				NAV_FIELD_PROP_REF(ptr_operating_point_information_rbsp);
				break;
			case DCI_NUT:
				NAV_FIELD_PROP_REF(ptr_decoding_capability_information_rbsp);
				break;
			case VPS_NUT:
				NAV_FIELD_PROP_REF(ptr_video_parameter_set_rbsp);
				break;
			case SPS_NUT:
				NAV_FIELD_PROP_REF(ptr_seq_parameter_set_rbsp);
				break;
			case PPS_NUT:
				NAV_FIELD_PROP_REF(ptr_pic_parameter_set_rbsp);
				break;
			case PREFIX_APS_NUT:
			case SUFFIX_APS_NUT:
				NAV_FIELD_PROP_REF(ptr_adaptation_parameter_set_rbsp);
				break;
			case PH_NUT:
				NAV_FIELD_PROP_REF(ptr_picture_header_rbsp);
				break;
			case AUD_NUT:
				NAV_FIELD_PROP_REF(ptr_access_unit_delimiter_rbsp);
				break;
			case EOS_NUT:
				break;
			case EOB_NUT:
				break;
			case PREFIX_SEI_NUT:
			case SUFFIX_SEI_NUT:
				NAV_FIELD_PROP_REF(ptr_sei_rbsp);
				break;
			case FD_NUT:
				break;
			case RSV_NVCL_26:
				break;
			case RSV_NVCL_27:
				break;
			case UNSPEC_28:
				break;
			case UNSPEC_29:
				break;
			case UNSPEC_30:
				break;
			case UNSPEC_31:
				break;
			}
			NAV_WRITE_TAG_END2("NAL_UNIT");
		DECLARE_FIELDPROP_END()
	}
}

