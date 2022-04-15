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
#ifndef _H266_VIDEO_H_
#define _H266_VIDEO_H_

#include "sei.h"
#include <vector>
#include <unordered_map>

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4100)
#pragma warning(disable:4127)
#pragma warning(disable:4200)
#pragma warning(disable:4201)
#pragma pack(push,1)
#define PACKED
#else
#define PACKED __attribute__ ((__packed__))
#endif

extern const char* vvc_profile_name[6];
extern const char* vvc_nal_unit_type_names[64];
extern const char* vvc_nal_unit_type_short_names[32];
extern const char* vvc_nal_unit_type_descs[32];

namespace BST {

	namespace H266Video {

		enum VVC_PROFILE
		{
			VVC_PROFILE_UNKNOWN = -1,
			VVC_PROFILE_MAIN_10,					// profile_idc 1,
			VVC_PROFILE_MULTILAYER_MAIN_10,			// profile_idc 17,
			VVC_PROFILE_MAIN_10_444,				// profile_idc 33,
			VVC_PROFILE_MULTILAYER_MAIN_10_444,		// profile_idc 49
			VVC_PROFILE_MAIN_10_STILL_PICTURE,		// profile_idc 65
			VVC_PROFILE_MAIN_10_444_STILL_PICTURE,	// profile_idc:97
		};

		enum VVC_TIER
		{
			VVC_TIER_UNKNOWN = -1,
			VVC_TIER_MAIN,
			VVC_TIER_HIGH
		};

		enum VVC_LEVEL
		{
			VVC_LEVEL_UNKNOWN = -1,
			VVC_LEVEL_1						= 16,	// 1
			VVC_LEVEL_2						= 32,	// 2
			VVC_LEVEL_2_1					= 35,	// 2.1
			VVC_LEVEL_3						= 48,	// 3
			VVC_LEVEL_3_1					= 51,	// 3.1
			VVC_LEVEL_4						= 64,	// 4
			VVC_LEVEL_4_1					= 67,	// 4.1
			VVC_LEVEL_5						= 80,	// 5
			VVC_LEVEL_5_1					= 83,	// 5.1
			VVC_LEVEL_5_2					= 86,	// 5.2
			VVC_LEVEL_6						= 96,	// 6
			VVC_LEVEL_6_1					= 99,	// 6.1
			VVC_LEVEL_6_2					= 102,	// 6.2
		};

		enum NAL_UNIT_TYPE
		{
			TRAIL_NUT = 0,
			STSA_NUT = 1,
			RADL_NUT = 2,
			RASL_NUT = 3,
			RSV_VCL_4 = 4,
			RSV_VCL_5 = 5,
			RSV_VCL_6 = 6,
			IDR_W_RADL = 7,
			IDR_N_LP = 8,
			CRA_NUT = 9,
			GDR_NUT = 10,
			RSV_IRAP_11 = 11,
			OPI_NUT = 12,
			DCI_NUT = 13,
			VPS_NUT = 14,
			SPS_NUT = 15,
			PPS_NUT = 16,
			PREFIX_APS_NUT = 17,
			SUFFIX_APS_NUT = 18,
			PH_NUT = 19,
			AUD_NUT = 20,
			EOS_NUT = 21,
			EOB_NUT = 22,
			PREFIX_SEI_NUT = 23,
			SUFFIX_SEI_NUT = 24,
			FD_NUT = 25,
			RSV_NVCL_26 = 26,
			RSV_NVCL_27 = 27,
			UNSPEC_28 = 28,
			UNSPEC_29 = 29,
			UNSPEC_30 = 30,
			UNSPEC_31 = 31,
		};

		class VideoBitstreamCtx 
		{
		public:
			std::vector<uint8_t>		nal_unit_type_filters;
		};

		struct PICTURE_HEADER_STRUCTURE : public SYNTAX_BITSTREAM_MAP
		{
			uint8_t			ph_gdr_or_irap_pic_flag : 1;
			uint8_t			ph_non_ref_pic_flag : 1;
			uint8_t			ph_gdr_pic_flag : 1;
			uint8_t			ph_inter_slice_allowed_flag : 1;
			uint8_t			ph_intra_slice_allowed_flag : 1;
			uint8_t			byte_align_0 : 3;

			uint32_t		ph_pic_parameter_set_id : 6;
			uint32_t		ph_pic_order_cnt_lsb : 20;
			uint32_t		dword_align_0 : 6;

			uint32_t		ph_recovery_poc_cnt;
			
			CAMBitArray		ph_extra_bit;


			int Map(AMBst in_bst)
			{
				SYNTAX_BITSTREAM_MAP::Map(in_bst);
				try
				{

				}
				catch (AMException e)
				{
					return e.RetCode();
				}

				return RET_CODE_SUCCESS;
			}
		};

		struct NAL_UNIT : public SYNTAX_BITSTREAM_MAP
		{
			struct NAL_UNIT_HEADER : public SYNTAX_BITSTREAM_MAP
			{
				uint16_t	forbidden_zero_bit : 1;
				uint16_t	nuh_reserved_zero_bit : 1;
				uint16_t	nuh_layer_id:6;
				uint16_t	nal_unit_type : 5;
				uint16_t	nuh_temporal_id_plus1 : 3;

				NAL_UNIT_HEADER()
					: forbidden_zero_bit(0), nuh_reserved_zero_bit(0), nuh_layer_id(0), nal_unit_type(0), nuh_temporal_id_plus1(0){
				}

				int Map(AMBst in_bst)
				{
					SYNTAX_BITSTREAM_MAP::Map(in_bst);
					try
					{
						MAP_BST_BEGIN(0);
						nal_read_f(in_bst, forbidden_zero_bit, 1, uint16_t);
						nal_read_f(in_bst, nuh_reserved_zero_bit, 1, uint16_t);
						nal_read_u(in_bst, nuh_layer_id, 6, uint16_t);
						nal_read_u(in_bst, nal_unit_type, 5, uint16_t);
						nal_read_u(in_bst, nuh_temporal_id_plus1, 3, uint16_t);
						MAP_BST_END();
					}
					catch(AMException e)
					{
						return e.RetCode();
					}

					return RET_CODE_SUCCESS;
				}

				int Unmap(AMBst out_bst)
				{
					UNREFERENCED_PARAMETER(out_bst);
					return RET_CODE_ERROR_NOTIMPL;
				}

				DECLARE_FIELDPROP_BEGIN()
					BST_FIELD_PROP_NUMBER1(forbidden_zero_bit, 1, "shall be equal to 0.");
					BST_FIELD_PROP_NUMBER1(nuh_reserved_zero_bit, 1, "shall be equal to 0.");
					BST_FIELD_PROP_2NUMBER1(nuh_layer_id, 6, "the identifier of the layer to which a VCL NAL unit belongs or the identifier of a layer to which a non-VCL NAL unit applies");
					BST_FIELD_PROP_2NUMBER1(nal_unit_type, 5, vvc_nal_unit_type_descs[nal_unit_type]);
					BST_FIELD_PROP_2NUMBER1(nuh_temporal_id_plus1, 3, (nal_unit_type >= IDR_W_RADL && nal_unit_type <= RSV_IRAP_11)
																		?"Should be 1"
																		:"TemporalId = nuh_temporal_id_plus1 - 1");
				DECLARE_FIELDPROP_END()

			}PACKED;

			struct GENERAL_CONSTRAINTS_INFO : public SYNTAX_BITSTREAM_MAP
			{
				uint8_t		gci_present_flag : 1;
				/* general */
				uint8_t		gci_intra_only_constraint_flag : 1;
				uint8_t		gci_all_layers_independent_constraint_flag : 1;
				uint8_t		gci_one_au_only_constraint_flag : 1;
				/* picture format */
				uint8_t		gci_sixteen_minus_max_bitdepth_constraint_idc : 4;

				uint8_t		gci_three_minus_max_chroma_format_constraint_idc : 2;
				/* NAL unit type related */
				uint8_t		gci_no_mixed_nalu_types_in_pic_constraint_flag : 1;
				uint8_t		gci_no_trail_constraint_flag : 1;
				uint8_t		gci_no_stsa_constraint_flag : 1;
				uint8_t		gci_no_rasl_constraint_flag : 1;
				uint8_t		gci_no_radl_constraint_flag : 1;
				uint8_t		gci_no_idr_constraint_flag : 1;

				uint8_t		gci_no_cra_constraint_flag : 1;
				uint8_t		gci_no_gdr_constraint_flag : 1;
				uint8_t		gci_no_aps_constraint_flag : 1;
				uint8_t		gci_no_idr_rpl_constraint_flag : 1;
				/* tile, slice, subpicture partitioning */
				uint8_t		gci_one_tile_per_pic_constraint_flag : 1;
				uint8_t		gci_pic_header_in_slice_header_constraint_flag : 1;
				uint8_t		gci_one_slice_per_pic_constraint_flag : 1;
				uint8_t		gci_no_rectangular_slice_constraint_flag : 1;

				uint8_t		gci_one_slice_per_subpic_constraint_flag : 1;
				uint8_t		gci_no_subpic_info_constraint_flag : 1;
				/* CTU and block partitioning */
				uint8_t		gci_three_minus_max_log2_ctu_size_constraint_idc : 2;
				uint8_t		gci_no_partition_constraints_override_constraint_flag : 1;
				uint8_t		gci_no_mtt_constraint_flag : 1;
				uint8_t		gci_no_qtbtt_dual_tree_intra_constraint_flag : 1;
				/* intra */
				uint8_t		gci_no_palette_constraint_flag : 1;

				uint8_t		gci_no_ibc_constraint_flag : 1;
				uint8_t		gci_no_isp_constraint_flag : 1;
				uint8_t		gci_no_mrl_constraint_flag : 1;
				uint8_t		gci_no_mip_constraint_flag : 1;
				uint8_t		gci_no_cclm_constraint_flag : 1;
				/* inter */
				uint8_t		gci_no_ref_pic_resampling_constraint_flag : 1;
				uint8_t		gci_no_res_change_in_clvs_constraint_flag : 1;
				uint8_t		gci_no_weighted_prediction_constraint_flag : 1;

				uint8_t		gci_no_ref_wraparound_constraint_flag : 1;
				uint8_t		gci_no_temporal_mvp_constraint_flag : 1;
				uint8_t		gci_no_sbtmvp_constraint_flag : 1;
				uint8_t		gci_no_amvr_constraint_flag : 1;
				uint8_t		gci_no_bdof_constraint_flag : 1;
				uint8_t		gci_no_smvd_constraint_flag : 1;
				uint8_t		gci_no_dmvr_constraint_flag : 1;
				uint8_t		gci_no_mmvd_constraint_flag : 1;

				uint8_t		gci_no_affine_motion_constraint_flag : 1;
				uint8_t		gci_no_prof_constraint_flag : 1;
				uint8_t		gci_no_bcw_constraint_flag : 1;
				uint8_t		gci_no_ciip_constraint_flag : 1;
				uint8_t		gci_no_gpm_constraint_flag : 1;
				/* transform, quantization, residual */
				uint8_t		gci_no_luma_transform_size_64_constraint_flag : 1;
				uint8_t		gci_no_transform_skip_constraint_flag : 1;
				uint8_t		gci_no_bdpcm_constraint_flag : 1;

				uint8_t		gci_no_mts_constraint_flag : 1;
				uint8_t		gci_no_lfnst_constraint_flag : 1;
				uint8_t		gci_no_joint_cbcr_constraint_flag : 1;
				uint8_t		gci_no_sbt_constraint_flag : 1;
				uint8_t		gci_no_act_constraint_flag : 1;
				uint8_t		gci_no_explicit_scaling_list_constraint_flag : 1;
				uint8_t		gci_no_dep_quant_constraint_flag : 1;
				uint8_t		gci_no_sign_data_hiding_constraint_flag : 1;

				uint8_t		gci_no_cu_qp_delta_constraint_flag : 1;
				uint8_t		gci_no_chroma_qp_offset_constraint_flag : 1;
				/* loop filter */
				uint8_t		gci_no_sao_constraint_flag : 1;
				uint8_t		gci_no_alf_constraint_flag : 1;
				uint8_t		gci_no_ccalf_constraint_flag : 1;
				uint8_t		gci_no_lmcs_constraint_flag : 1;
				uint8_t		gci_no_ladf_constraint_flag : 1;
				uint8_t		gci_no_virtual_boundaries_constraint_flag : 1;

				uint8_t		gci_num_reserved_bits;

				CAMBitArray	gci_reserved_zero_bit;

				CAMBitArray	gci_alignment_zero_bit;

				int Map(AMBst in_bst)
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

				int Unmap(AMBst out_bst)
				{
					UNREFERENCED_PARAMETER(out_bst);
					return RET_CODE_ERROR_NOTIMPL;
				}

				DECLARE_FIELDPROP_BEGIN()
				DECLARE_FIELDPROP_END()
			};

			struct PROFILE_TIER_LEVEL : public SYNTAX_BITSTREAM_MAP
			{
				uint8_t			general_profile_idc : 7;
				uint8_t			general_tier_flag : 1;

				uint8_t			general_level_idc;
				
				uint8_t			ptl_frame_only_constraint_flag : 1;
				uint8_t			ptl_multilayer_enabled_flag : 1;
				uint8_t			byte_align_0 : 6;

				uint8_t			dword_align_0;

				GENERAL_CONSTRAINTS_INFO
								general_constraints_info;

				CAMBitArray		ptl_sublayer_level_present_flag;
				CAMBitArray		ptl_reserved_zero_bit;

				std::vector<uint8_t>
								sublayer_level_idc;

				uint8_t			ptl_num_sub_profiles;
				std::vector<uint32_t>
								general_sub_profile_idc;

				bool			m_profileTierPresentFlag;
				int				m_MaxNumSubLayersMinus1;

				PROFILE_TIER_LEVEL(bool profileTierPresentFlag, int MaxNumSubLayersMinus1) 
					: m_profileTierPresentFlag(profileTierPresentFlag)
					, m_MaxNumSubLayersMinus1(MaxNumSubLayersMinus1){
				}

				VVC_PROFILE GetVVCProfile() {
					if (m_profileTierPresentFlag) {
						switch (general_profile_idc) {
						case 1: return VVC_PROFILE_MAIN_10;
						case 65: return VVC_PROFILE_MAIN_10_STILL_PICTURE;
						case 33: return VVC_PROFILE_MAIN_10_444;
						case 97: return VVC_PROFILE_MAIN_10_444_STILL_PICTURE;
						case 17: return VVC_PROFILE_MULTILAYER_MAIN_10;
						case 49: return VVC_PROFILE_MULTILAYER_MAIN_10_444;
						}
					}

					return VVC_PROFILE_UNKNOWN;
				}

				int Map(AMBst in_bst)
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

						for (int i = m_MaxNumSubLayersMinus1 - 1; i >= 0; i--){
							bsrbarray(in_bst, ptl_sublayer_level_present_flag, i);
						}

						idx = 0;
						while (AMBst_IsAligned(in_bst)) {
							bsrbarray(in_bst, ptl_reserved_zero_bit, idx);
							idx++;
						}

						if (m_MaxNumSubLayersMinus1 > 0)
							sublayer_level_idc.resize(m_MaxNumSubLayersMinus1);

						for (int i = m_MaxNumSubLayersMinus1 - 1; i >= 0; i--) {
							if (ptl_sublayer_level_present_flag[i]) {
								bsrb1(in_bst, sublayer_level_idc[i], 8);
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

				int Unmap(AMBst out_bst)
				{
					UNREFERENCED_PARAMETER(out_bst);
					return RET_CODE_ERROR_NOTIMPL;
				}

				DECLARE_FIELDPROP_BEGIN()
				DECLARE_FIELDPROP_END()
			};

			struct DPB_PARAMETERS : public SYNTAX_BITSTREAM_MAP
			{
				std::vector<uint32_t>
								dpb_max_dec_pic_buffering_minus1;
				std::vector<uint32_t>
								dpb_max_num_reorder_pics;
				std::vector<uint32_t>
								dpb_max_latency_increase_plus1;

				int				m_MaxSubLayersMinus1;
				bool			m_subLayerInfoFlag;

				DPB_PARAMETERS(int MaxSubLayersMinus1, bool subLayerInfoFlag)
					: m_MaxSubLayersMinus1(MaxSubLayersMinus1), m_subLayerInfoFlag(subLayerInfoFlag){
				}

				int Map(AMBst in_bst)
				{
					int iRet = RET_CODE_SUCCESS;
					SYNTAX_BITSTREAM_MAP::Map(in_bst);

					try
					{
						MAP_BST_BEGIN(0);
						dpb_max_dec_pic_buffering_minus1.resize((m_subLayerInfoFlag?m_MaxSubLayersMinus1:0) + 1);
						dpb_max_num_reorder_pics.resize((m_subLayerInfoFlag ? m_MaxSubLayersMinus1 : 0) + 1);
						dpb_max_latency_increase_plus1.resize((m_subLayerInfoFlag ? m_MaxSubLayersMinus1 : 0) + 1);
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
							nal_read_ue(in_bst, dpb_max_dec_pic_buffering_minus1[0], uint32_t);
							nal_read_ue(in_bst, dpb_max_num_reorder_pics[0], uint32_t);
							nal_read_ue(in_bst, dpb_max_latency_increase_plus1[0], uint32_t);
						}
						MAP_BST_END();
					}
					catch (AMException e)
					{
						return e.RetCode();
					}

					return RET_CODE_SUCCESS;
				}

				int Unmap(AMBst out_bst)
				{
					UNREFERENCED_PARAMETER(out_bst);
					return RET_CODE_ERROR_NOTIMPL;
				}

				DECLARE_FIELDPROP_BEGIN()

				DECLARE_FIELDPROP_END()
			};

			struct GENERAL_TIMING_HRD_PARAMETERS : public SYNTAX_BITSTREAM_MAP
			{
				uint32_t		num_units_in_tick;
				uint32_t		time_scale;

				uint8_t			general_nal_hrd_params_present_flag : 1;
				uint8_t			general_vcl_hrd_params_present_flag : 1;
				uint8_t			general_same_pic_timing_in_all_ols_flag : 1;
				uint8_t			general_du_hrd_params_present_flag : 1;
				uint8_t			cpb_size_du_scale : 4;

				uint8_t			tick_divisor_minus2;

				uint8_t			bit_rate_scale : 4;
				uint8_t			cpb_size_scale : 4;

				uint8_t			hrd_cpb_cnt_minus1;

				GENERAL_TIMING_HRD_PARAMETERS()
					: general_nal_hrd_params_present_flag(0)
					, general_vcl_hrd_params_present_flag(0) {
				}

				int Map(AMBst in_bst)
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

				int Unmap(AMBst out_bst)
				{
					UNREFERENCED_PARAMETER(out_bst);
					return RET_CODE_ERROR_NOTIMPL;
				}

				DECLARE_FIELDPROP_BEGIN()

				DECLARE_FIELDPROP_END()
			};

			struct SUBLAYER_HRD_PARAMETERS : public SYNTAX_BITSTREAM_MAP
			{
				struct SUBLAYER_HRD_PARAMETER
				{
					uint32_t		bit_rate_value_minus1;
					uint32_t		cpb_size_value_minus1;
					uint32_t		cpb_size_du_value_minus1;
					uint32_t		bit_rate_du_value_minus1;
					uint8_t			cbr_flag : 1;
					uint8_t			byte_align_0 : 7;
				}PACKED;

				std::vector<SUBLAYER_HRD_PARAMETER>
									parameters;

				GENERAL_TIMING_HRD_PARAMETERS* 
									m_pTimingHRDParameters;

				SUBLAYER_HRD_PARAMETERS(GENERAL_TIMING_HRD_PARAMETERS* pTimingHRDParameters) 
					: m_pTimingHRDParameters(pTimingHRDParameters) {
				}

				int Map(AMBst in_bst)
				{
					int iRet = RET_CODE_SUCCESS;
					SYNTAX_BITSTREAM_MAP::Map(in_bst);

					try
					{
						MAP_BST_BEGIN(0);
						parameters.resize(m_pTimingHRDParameters->hrd_cpb_cnt_minus1 + 1);
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

				int Unmap(AMBst out_bst)
				{
					UNREFERENCED_PARAMETER(out_bst);
					return RET_CODE_ERROR_NOTIMPL;
				}

				DECLARE_FIELDPROP_BEGIN()

				DECLARE_FIELDPROP_END()
			};

			struct OLS_TIMING_HRD_PARAMETERS : public SYNTAX_BITSTREAM_MAP
			{
				struct OLS_TIMING_HRD_PARAMETER
				{
					uint16_t		fixed_pic_rate_general_flag : 1;
					uint16_t		fixed_pic_rate_within_cvs_flag : 1;
					uint16_t		elemental_duration_in_tc_minus1 : 12;
					uint16_t		low_delay_hrd_flag : 1;
					uint16_t		word_align_0 : 1;

					SUBLAYER_HRD_PARAMETERS*
									sublayer_nal_hrd_parameters = nullptr;

					SUBLAYER_HRD_PARAMETERS*
									sublayer_vcl_hrd_parameters = nullptr;

				};

				std::vector< OLS_TIMING_HRD_PARAMETER>
									parameters;

				int					m_firstSubLayer;
				int					m_MaxSubLayersVal;
				GENERAL_TIMING_HRD_PARAMETERS* 
									m_pTimingHRDParameters;

				OLS_TIMING_HRD_PARAMETERS(int firstSubLayer, int MaxSubLayersVal, GENERAL_TIMING_HRD_PARAMETERS* pTimingHRDParameters)
					: m_firstSubLayer(firstSubLayer)
					, m_MaxSubLayersVal(MaxSubLayersVal)
					, m_pTimingHRDParameters(pTimingHRDParameters){
				}

				int Map(AMBst in_bst)
				{
					int iRet = RET_CODE_SUCCESS;
					SYNTAX_BITSTREAM_MAP::Map(in_bst);

					try
					{
						MAP_BST_BEGIN(0);
						if (m_MaxSubLayersVal >= m_firstSubLayer)
						{
							parameters.resize(m_MaxSubLayersVal - m_firstSubLayer + 1);
							for (int i = 0; i <= m_MaxSubLayersVal - m_firstSubLayer; i++)
							{
								bsrb1(in_bst, parameters[i].fixed_pic_rate_general_flag, 1);
								if (!parameters[i].fixed_pic_rate_general_flag) {
									bsrb1(in_bst, parameters[i].fixed_pic_rate_within_cvs_flag, 1);
								}

								if (parameters[i].fixed_pic_rate_within_cvs_flag) {
									nal_read_ue(in_bst, parameters[i].elemental_duration_in_tc_minus1, uint16_t);
								}
								else if((m_pTimingHRDParameters->general_nal_hrd_params_present_flag ||
										 m_pTimingHRDParameters->general_vcl_hrd_params_present_flag) && 
										 m_pTimingHRDParameters->hrd_cpb_cnt_minus1 == 0)
								{
									bsrb1(in_bst, parameters[i].low_delay_hrd_flag, 1);
								}

								if (m_pTimingHRDParameters->general_nal_hrd_params_present_flag){
									bsrbreadref(in_bst, parameters[i].sublayer_nal_hrd_parameters, SUBLAYER_HRD_PARAMETERS, m_pTimingHRDParameters);
								}

								if (m_pTimingHRDParameters->general_vcl_hrd_params_present_flag) {
									bsrbreadref(in_bst, parameters[i].sublayer_vcl_hrd_parameters, SUBLAYER_HRD_PARAMETERS, m_pTimingHRDParameters);
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

				int Unmap(AMBst out_bst)
				{
					UNREFERENCED_PARAMETER(out_bst);
					return RET_CODE_ERROR_NOTIMPL;
				}

				DECLARE_FIELDPROP_BEGIN()

				DECLARE_FIELDPROP_END()

			};

			struct VIDEO_PARAMETER_SET_RBSP : public SYNTAX_BITSTREAM_MAP
			{
				struct VPS_OLS_DPB_PARAMS
				{
					uint32_t		pic_width;
					uint32_t		pic_height;
					uint32_t		chroma_format : 2;
					uint32_t		bitdepth_minus8 : 4;
					uint32_t		params_idx : 26;
				}PACKED;

				uint16_t		vps_video_parameter_set_id : 4;
				uint16_t		vps_max_layers_minus1 : 6;
				uint16_t		vps_max_sublayers_minus1 : 3;
				uint16_t		vps_default_ptl_dpb_hrd_max_tid_flag : 1;
				uint16_t		vps_all_independent_layers_flag : 1;
				uint16_t		word_align_0 : 1;

				uint8_t			vps_each_layer_is_an_ols_flag : 1;
				uint8_t			vps_ols_mode_idc : 2;
				uint8_t			byte_align_0 : 5;

				uint8_t			vps_num_output_layer_sets_minus2;

				uint8_t			vps_layer_id[64] = { 0 };
				CAMBitArray		vps_independent_layer_flag;
				CAMBitArray		vps_max_tid_ref_present_flag;
				CAMBitArray		vps_direct_ref_layer_flag[64];
				uint8_t			vps_max_tid_il_ref_pics_plus1[64][64] = { {0} };

				std::vector<CAMBitArray>		
								vps_ols_output_layer_flag;
				
				uint8_t			vps_num_ptls_minus1;

				CAMBitArray		vps_pt_present_flag;
				std::vector<uint8_t>
								vps_ptl_max_tid;

				CAMBitArray		vps_ptl_alignment_zero_bit;
				std::vector<PROFILE_TIER_LEVEL*>
								profile_tier_level;
				std::vector<uint8_t>
								vps_ols_ptl_idx;

				uint32_t		vps_num_dpb_params_minus1;
				uint8_t			vps_sublayer_dpb_params_present_flag : 1;
				uint8_t			vps_timing_hrd_params_present_flag : 1;
				uint8_t			vps_sublayer_cpb_params_present_flag : 1;
				uint8_t			vps_extension_flag : 1;
				uint8_t			byte_align_1 : 4;

				std::vector<uint8_t>
								vps_dpb_max_tid;
				std::vector<DPB_PARAMETERS*>
								dpb_parameters;

				std::vector< VPS_OLS_DPB_PARAMS>
								vps_ols_dpb;

				GENERAL_TIMING_HRD_PARAMETERS*
								general_timing_hrd_parameters = nullptr;

				uint16_t		vps_num_ols_timing_hrd_params_minus1;
				std::vector<uint8_t>
								vps_hrd_max_tid;

				std::vector<OLS_TIMING_HRD_PARAMETERS*>
								ols_timing_hrd_parameters;

				std::vector<uint32_t>
								vps_ols_timing_hrd_idx;

				CAMBitArray		vps_extension_data_flag;

				RBSP_TRAILING_BITS
								rbsp_trailing_bits;

				// The variables NumDirectRefLayers[i], DirectRefLayerIdx[i][d], NumRefLayers[i]
				// ReferenceLayerIdx[i][r], and LayerUsedAsRefLayerFlag[ j ] are derived as follows
				uint8_t			NumDirectRefLayers[64] = { 0 };
				uint8_t			DirectRefLayerIdx[64][64] = { {0} };
				uint8_t			NumRefLayers[64] = { 0 };
				uint8_t			ReferenceLayerIdx[64][64] = { {0} };
				std::vector<bool>
								LayerUsedAsRefLayerFlag;

				uint8_t			olsModeIdc;
				uint16_t		TotalNumOlss;
				// The variable NumOutputLayersInOls[i], specifying the number of output layers in the i-th OLS
				std::vector<uint16_t>
								NumOutputLayersInOls;
				// the variable NumSubLayersInLayerInOLS[i][j], specifying the number of sublayers in the j-th layer in the i-th OLS
				std::vector<std::vector<uint16_t>>
								NumSubLayersInLayerInOLS;
				// the variable OutputLayerIdInOls[i][j], specifying the nuh_layer_id value of the j-th output layer in the i-th OLS
				std::vector<std::vector<uint8_t>>
								OutputLayerIdInOls;
				// the variable LayerUsedAsOutputLayerFlag[ k ], specifying whether the k-th layer is used as an output layer in at least one OLS
				CAMBitArray		LayerUsedAsOutputLayerFlag;
				// The variable NumLayersInOls[ i ], specifying the number of layers in the i-th OLS
				std::vector<uint16_t>
								NumLayersInOls;
				// the variable LayerIdInOls[ i ][ j ], specifying the nuh_layer_id value of the j-th layer in the i-th OLS
				std::vector<std::vector<uint8_t>>
								LayerIdInOls;
				// the variable NumMultiLayerOlss, specifying the number of multi-layer OLSs (i.e., OLSs that contain more than one layer)
				uint16_t		NumMultiLayerOlss;
				// the variable MultiLayerOlsIdx[i], specifying the index to the list of multi-layer OLSs for the i-th OLS when NumLayersInOls[i] is greater than 0
				std::vector<uint16_t>
								MultiLayerOlsIdx;

				VIDEO_PARAMETER_SET_RBSP()
					: vps_default_ptl_dpb_hrd_max_tid_flag(1)
					, vps_all_independent_layers_flag(1)
					, vps_each_layer_is_an_ols_flag(0)
					, vps_num_ptls_minus1(0)
					, vps_sublayer_dpb_params_present_flag(0)
					, LayerUsedAsRefLayerFlag(64){
				}

				~VIDEO_PARAMETER_SET_RBSP() {
					for (auto v : profile_tier_level) { AMP_SAFEDEL2(v); }
					for (auto v : dpb_parameters) {AMP_SAFEDEL2(v);}
					AMP_SAFEDEL2(general_timing_hrd_parameters);
					for (auto v : ols_timing_hrd_parameters) {AMP_SAFEDEL2(v);}
				}

				INLINE int VpsNumDpbParams()
				{
					return vps_each_layer_is_an_ols_flag ? 0 : (vps_num_dpb_params_minus1 + 1);
				}

				int UpdateReferenceLayers();
				int UpdateOLSInfo();

				int Map(AMBst in_bst)
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
								while (AMBst_more_data(in_bst)) {
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

				int Unmap(AMBst out_bst)
				{
					UNREFERENCED_PARAMETER(out_bst);
					return RET_CODE_ERROR_NOTIMPL;
				}

				DECLARE_FIELDPROP_BEGIN()

				DECLARE_FIELDPROP_END()
			};

			struct SEQ_PARAMETER_SET_RBSP;

			struct REF_PIC_LIST_STRUCT : public SYNTAX_BITSTREAM_MAP
			{
				uint8_t			num_ref_entries : 7;
				uint8_t			ltrp_in_header_flag : 1;

				CAMBitArray		inter_layer_ref_pic_flag;
				CAMBitArray		st_ref_pic_flag;
				std::vector<uint16_t>
								abs_delta_poc_st;
				CAMBitArray		strp_entry_sign_flag;
				std::vector<uint16_t>
								rpls_poc_lsb_lt;
				std::vector<uint16_t>
								ilrp_idx;

				uint8_t			m_listIdx;	// 0,1
				uint8_t			m_rplsIdx;	// 0~64
				SEQ_PARAMETER_SET_RBSP* 
								m_seq_parameter_set_rbsp;

				REF_PIC_LIST_STRUCT(uint8_t listIdx, uint8_t rplsIdx, SEQ_PARAMETER_SET_RBSP* seq_parameter_set_rbsp)
					: ltrp_in_header_flag(1), m_listIdx(listIdx), m_rplsIdx(rplsIdx), m_seq_parameter_set_rbsp(seq_parameter_set_rbsp){
				}

				int Map(AMBst in_bst);
				int Unmap(AMBst out_bst);
				size_t ProduceDesc(_Out_writes_(cbLen) char* szOutXml, size_t cbLen, bool bPrint = false, long long* bit_offset = NULL);
			};

			struct SEQ_PARAMETER_SET_RBSP : public SYNTAX_BITSTREAM_MAP
			{
				uint8_t			sps_seq_parameter_set_id : 4;
				uint8_t			sps_video_parameter_set_id : 4;
				uint8_t			sps_max_sublayers_minus1 : 3;
				uint8_t			sps_chroma_format_idc : 2;
				uint8_t			sps_log2_ctu_size_minus5 : 2;
				uint8_t			sps_ptl_dpb_hrd_params_present_flag : 1;

				PROFILE_TIER_LEVEL*
								profile_tier_level = nullptr;

				uint8_t			sps_gdr_enabled_flag : 1;
				uint8_t			sps_ref_pic_resampling_enabled_flag : 1;
				uint8_t			sps_res_change_in_clvs_allowed_flag : 1;
				uint8_t			sps_conformance_window_flag : 1;

				uint8_t			sps_subpic_info_present_flag : 1;
				uint8_t			sps_independent_subpics_flag : 1;
				uint8_t			sps_subpic_same_size_flag : 1;
				uint8_t			sps_subpic_id_mapping_explicitly_signalled_flag : 1;

				uint32_t		sps_pic_width_max_in_luma_samples = 0;
				uint32_t		sps_pic_height_max_in_luma_samples = 0;
				uint32_t		sps_conf_win_left_offset = 0;
				uint32_t		sps_conf_win_right_offset = 0;
				uint32_t		sps_conf_win_top_offset = 0;
				uint32_t		sps_conf_win_bottom_offset = 0;

				uint16_t		sps_num_subpics_minus1 = 0;

				std::vector<uint32_t>
								sps_subpic_ctu_top_left_x;
				std::vector<uint32_t>
								sps_subpic_ctu_top_left_y;
				std::vector<uint32_t>
								sps_subpic_width_minus1;
				std::vector<uint32_t>
								sps_subpic_height_minus1;
				std::vector<bool>
								sps_subpic_treated_as_pic_flag;
				std::vector<bool>
								sps_loop_filter_across_subpic_enabled_flag;

				uint8_t			sps_subpic_id_len_minus1;

				std::vector<uint16_t>
								sps_subpic_id;

				uint32_t		sps_bitdepth_minus8 : 2;
				uint32_t		sps_subpic_id_mapping_present_flag : 1;
				uint32_t		sps_entropy_coding_sync_enabled_flag : 1;
				uint32_t		sps_entry_point_offsets_present_flag : 1;
				uint32_t		sps_log2_max_pic_order_cnt_lsb_minus4 : 4;
				uint32_t		sps_poc_msb_cycle_flag : 1;

				uint32_t		sps_poc_msb_cycle_len_minus1 : 5;

				uint32_t		sps_num_extra_ph_bytes : 2;
				uint32_t		sps_num_extra_sh_bytes : 2;
				uint32_t		sps_sublayer_dpb_params_flag : 1;
				uint32_t		sps_log2_min_luma_coding_block_size_minus2 : 3;
				uint32_t		sps_log2_diff_min_qt_min_cb_intra_slice_luma : 3;
				uint32_t		sps_max_mtt_hierarchy_depth_intra_slice_luma : 4;
				uint32_t		sps_partition_constraints_override_enabled_flag : 1;
				uint32_t		sps_qtbtt_dual_tree_intra_flag : 1;

				uint32_t		sps_log2_diff_max_bt_min_qt_intra_slice_luma : 3;
				uint32_t		sps_log2_diff_max_tt_min_qt_intra_slice_luma : 3;
				uint32_t		sps_log2_diff_min_qt_min_cb_inter_slice : 3;
				uint32_t		sps_max_mtt_hierarchy_depth_inter_slice : 3;
				uint32_t		sps_log2_diff_max_bt_min_qt_inter_slice : 3;
				uint32_t		sps_log2_diff_max_tt_min_qt_inter_slice : 3;
				uint32_t		sps_max_luma_transform_size_64_flag : 1;
				uint32_t		sps_transform_skip_enabled_flag : 1;
				uint32_t		sps_log2_transform_skip_max_size_minus2 : 2;
				uint32_t		sps_bdpcm_enabled_flag : 1;
				uint32_t		sps_mts_enabled_flag : 1;
				uint32_t		sps_explicit_mts_intra_enabled_flag : 1;
				uint32_t		sps_explicit_mts_inter_enabled_flag : 1;
				uint32_t		sps_lfnst_enabled_flag : 1;
				uint32_t		sps_joint_cbcr_enabled_flag : 1;
				uint32_t		sps_same_qp_table_for_chroma_flag : 1;
				uint32_t		dword_align_0 : 3;

				std::vector<bool>
								sps_extra_ph_bit_present_flag;
				std::vector<bool>
								sps_extra_sh_bit_present_flag;

				int8_t			sps_qp_table_start_minus26[3];
				uint8_t			sps_num_points_in_qp_table_minus1[3];
				uint16_t		sps_delta_qp_in_val_minus1[3][64];
				uint16_t		sps_delta_qp_diff_val[3][64];

				uint8_t			sps_sao_enabled_flag : 1;
				uint8_t			sps_alf_enabled_flag : 1;
				uint8_t			sps_ccalf_enabled_flag : 1;
				uint8_t			sps_lmcs_enabled_flag : 1;
				uint8_t			sps_weighted_pred_flag : 1;
				uint8_t			sps_weighted_bipred_flag : 1;
				uint8_t			sps_long_term_ref_pics_flag : 1;
				uint8_t			sps_inter_layer_prediction_enabled_flag : 1;

				uint8_t			sps_idr_rpl_present_flag : 1;
				uint8_t			sps_rpl1_same_as_rpl0_flag : 1;
				uint8_t			sps_ref_wraparound_enabled_flag : 1;
				uint8_t			sps_temporal_mvp_enabled_flag : 1;
				uint8_t			sps_sbtmvp_enabled_flag : 1;
				uint8_t			sps_amvr_enabled_flag : 1;
				uint8_t			sps_bdof_enabled_flag : 1;
				uint8_t			sps_bdof_control_present_in_ph_flag : 1;

				uint8_t			sps_smvd_enabled_flag : 1;
				uint8_t			sps_dmvr_enabled_flag : 1;
				uint8_t			sps_dmvr_control_present_in_ph_flag : 1;
				uint8_t			sps_mmvd_enabled_flag : 1;
				uint8_t			sps_mmvd_fullpel_only_enabled_flag : 1;
				uint8_t			sps_six_minus_max_num_merge_cand : 3;

				uint8_t			sps_sbt_enabled_flag : 1;
				uint8_t			sps_affine_enabled_flag : 3;
				uint8_t			sps_6param_affine_enabled_flag : 1;
				uint8_t			sps_affine_amvr_enabled_flag : 1;
				uint8_t			sps_affine_prof_enabled_flag : 1;
				uint8_t			sps_prof_control_present_in_ph_flag : 1;

				uint8_t			sps_num_ref_pic_lists[2];

				uint32_t		sps_bcw_enabled_flag : 1;
				uint32_t		sps_ciip_enabled_flag : 1;
				uint32_t		sps_gpm_enabled_flag : 1;
				uint32_t		sps_max_num_merge_cand_minus_max_num_gpm_cand : 3;
				uint32_t		sps_log2_parallel_merge_level_minus2 : 3;
				uint32_t		sps_isp_enabled_flag : 1;
				uint32_t		sps_mrl_enabled_flag : 1;
				uint32_t		sps_mip_enabled_flag : 1;
				uint32_t		sps_cclm_enabled_flag : 1;
				uint32_t		sps_chroma_horizontal_collocated_flag : 1;
				uint32_t		sps_chroma_vertical_collocated_flag : 1;
				uint32_t		sps_palette_enabled_flag : 1;
				uint32_t		sps_act_enabled_flag : 1;
				uint32_t		sps_min_qp_prime_ts : 4;
				uint32_t		sps_ibc_enabled_flag : 1;
				uint32_t		sps_six_minus_max_num_ibc_merge_cand : 3;
				uint32_t		sps_ladf_enabled_flag : 1;
				uint32_t		sps_num_ladf_intervals_minus2 : 2;
				uint32_t		dword_align1 : 4;

				int8_t			sps_ladf_lowest_interval_qp_offset;

			};
		};
	}
}

#ifdef _WIN32
#pragma pack(pop)
#pragma warning(pop)
#endif
#undef PACKED

#endif
