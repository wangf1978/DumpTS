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
#include <array>

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

#define ALF_APS			0
#define LMCS_APS		1
#define SCALING_APS		2

extern const char* vvc_profile_name[6];
extern const char* vvc_nal_unit_type_names[64];
extern const char* vvc_nal_unit_type_short_names[32];
extern const char* vvc_nal_unit_type_descs[32];
extern const char* sample_aspect_ratio_descs[256];

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

		const uint8_t B_SLICE = 0;
		const uint8_t P_SLICE = 1;
		const uint8_t I_SLICE = 2;

		int read_extension_and_trailing_bits(AMBst in_bst, SYNTAX_MAP_STATUS& map_status, 
											 CAMBitArray& extension_data_flag, RBSP_TRAILING_BITS& rbsp_trailing_bits);

		struct REF_PIC_LIST_STRUCT;

		class VideoBitstreamCtx : public CComUnknown, public INALVVCContext
		{
		public:
			DECLARE_IUNKNOWN

			HRESULT NonDelegatingQueryInterface(REFIID uuid, void** ppvObj)
			{
				if (ppvObj == NULL)
					return E_POINTER;

				if (uuid == IID_INALContext)
					return GetCOMInterface((INALContext*)this, ppvObj);
				else if (uuid == IID_INALVVCContext)
					return GetCOMInterface((INALVVCContext*)this, ppvObj);

				return CComUnknown::NonDelegatingQueryInterface(uuid, ppvObj);
			}

			NAL_CODING					GetNALCoding() { return NAL_CODING_VVC; }
			RET_CODE					SetNUFilters(std::initializer_list<uint8_t> NU_type_filters);
			RET_CODE					GetNUFilters(std::vector<uint8_t>& NU_type_filters);
			bool						IsNUFiltered(uint8_t nal_unit_type);
			RET_CODE					SetActiveNUType(int8_t nu_type) { m_active_nu_type = nu_type; return RET_CODE_SUCCESS; }
			int8_t						GetActiveNUType() { return m_active_nu_type; }
			void						Reset();

			//
			// Interface INALVVCContext
			//
			H266_NU						GetVVCDCI();
			H266_NU						GetVVCOPI();
			H266_NU						GetVVCVPS(uint8_t vps_id);
			H266_NU						GetVVCSPS(uint8_t sps_id);
			H266_NU						GetVVCPPS(uint8_t pps_id);
			H266_NU						GetVVCPH();
			RET_CODE					UpdateVVCDCI(H266_NU dci_nu);
			RET_CODE					UpdateVVCOPI(H266_NU opi_nu);
			RET_CODE					UpdateVVCVPS(H266_NU vps_nu);
			RET_CODE					UpdateVVCSPS(H266_NU sps_nu);
			RET_CODE					UpdateVVCPPS(H266_NU pps_nu);
			RET_CODE					UpdateVVCPH(H266_NU ph_nu);
			H266_NU						CreateVVCNU();
			int8_t						GetActiveSPSID();
			RET_CODE					ActivateSPS(int8_t sps_id);
			RET_CODE					DetactivateSPS();

			RET_CODE					GetNumRefIdxActive(int8_t num_ref_idx_active[2]);
			RET_CODE					GetRplsIdx(int8_t rpls_idx[2]);

		public:
			std::vector<uint8_t>		nal_unit_type_filters;
			std::unordered_map<uint8_t, int8_t>
										sps_seq_parameter_set_id;
			int8_t						prev_vps_video_parameter_set_id =  -1;
			H266_NU						sp_prev_nal_unit;
			int8_t						m_active_sps_id = -1;
			int8_t						m_active_nu_type = -1;
			H266_NU						m_sp_dci;
			H266_NU						m_sp_opi;
			std::map<uint8_t, H266_NU>	m_sp_vpses;		// the smart pointers of VPS for the current h266 bitstream
			std::map<uint8_t, H266_NU>	m_sp_spses;		// the smart pointers of SPS for the current h266 bitstream
			std::map<uint8_t, H266_NU>	m_sp_ppses;		// the smart pointers of PPS for the current h266 bitstream
			H266_NU						m_ph;			// the life-cycle is in the scope of AU

			int8_t						RplsIdx[2] = { -1, -1 };
			int8_t						NumRefIdxActive[2] = { -1, -1 };
			uint8_t						NumLtrpEntries[2][65] = { {0} };
			std::shared_ptr<REF_PIC_LIST_STRUCT>
										ref_pic_list_struct[2][65];
		};

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

			// Variables inherited from the current SPS
			uint8_t			current_sps_num_ref_pic_list;
			uint8_t			sps_long_term_ref_pics_flag:1;
			uint8_t			sps_inter_layer_prediction_enabled_flag:1;
			uint8_t			sps_weighted_pred_flag:1;
			uint8_t			sps_weighted_bipred_flag:1;
			uint8_t			sps_log2_max_pic_order_cnt_lsb_minus4:4;

			VideoBitstreamCtx*
							m_pCtx;

			REF_PIC_LIST_STRUCT(uint8_t listIdx, uint8_t rplsIdx, void* seq_parameter_set_rbsp);
			int Map(AMBst in_bst);
			int Unmap(AMBst out_bst);
			DECLARE_FIELDPROP();
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
					BST_FIELD_PROP_NUMBER1(forbidden_zero_bit, 1,		"shall be equal to 0.");
					BST_FIELD_PROP_NUMBER1(nuh_reserved_zero_bit, 1,	"shall be equal to 0.");
					BST_FIELD_PROP_2NUMBER1(nuh_layer_id, 6,			"the identifier of the layer to which a VCL NAL unit belongs or the identifier of a layer to which a non-VCL NAL unit applies");
					BST_FIELD_PROP_2NUMBER1(nal_unit_type, 5,			vvc_nal_unit_type_descs[nal_unit_type]);
					BST_FIELD_PROP_2NUMBER1(nuh_temporal_id_plus1, 3,	(nal_unit_type >= IDR_W_RADL && nal_unit_type <= RSV_IRAP_11)
																		? "Should be 1" : "TemporalId = nuh_temporal_id_plus1 - 1");
				DECLARE_FIELDPROP_END()

			}PACKED;

			struct ACCESS_UNIT_DELIMITER_RBSP : public SYNTAX_BITSTREAM_MAP
			{
				uint8_t			aud_irap_or_gdr_flag : 1;
				uint8_t			aud_pic_type : 3;
				uint8_t			byte_align0 : 4;

				RBSP_TRAILING_BITS
								rbsp_tailing_bits;

				ACCESS_UNIT_DELIMITER_RBSP() : aud_irap_or_gdr_flag(0), aud_pic_type(0), byte_align0(0){}

				int Map(AMBst in_bst);
				int Unmap(AMBst out_bst);
				DECLARE_FIELDPROP();
			};

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

				int Map(AMBst in_bst);
				int Unmap(AMBst out_bst);
				DECLARE_FIELDPROP();
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

				INLINE VVC_PROFILE GetVVCProfile() {
					if (m_profileTierPresentFlag) {
						return GetVVCProfile(general_profile_idc);
					}

					return VVC_PROFILE_UNKNOWN;
				}

				INLINE VVC_PROFILE GetVVCProfile(uint32_t profile_idc) {
					switch (profile_idc) {
					case 1: return VVC_PROFILE_MAIN_10;
					case 65: return VVC_PROFILE_MAIN_10_STILL_PICTURE;
					case 33: return VVC_PROFILE_MAIN_10_444;
					case 97: return VVC_PROFILE_MAIN_10_444_STILL_PICTURE;
					case 17: return VVC_PROFILE_MULTILAYER_MAIN_10;
					case 49: return VVC_PROFILE_MULTILAYER_MAIN_10_444;
					}

					return VVC_PROFILE_UNKNOWN;
				}

				int Map(AMBst in_bst);
				int Unmap(AMBst out_bst);
				DECLARE_FIELDPROP();
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

				int Map(AMBst in_bst);
				int Unmap(AMBst out_bst);
				DECLARE_FIELDPROP();
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

				int Map(AMBst in_bst);
				int Unmap(AMBst out_bst);
				DECLARE_FIELDPROP();
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

				int Map(AMBst in_bst);
				int Unmap(AMBst out_bst);
				DECLARE_FIELDPROP();
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

					OLS_TIMING_HRD_PARAMETER()
						:fixed_pic_rate_general_flag(0), fixed_pic_rate_within_cvs_flag(1)
						, elemental_duration_in_tc_minus1(0), low_delay_hrd_flag(0), word_align_0(0){
					}
				};

				std::vector<OLS_TIMING_HRD_PARAMETER>
									parameters;

				uint8_t				m_firstSubLayer;
				uint8_t				m_MaxSubLayersVal;
				GENERAL_TIMING_HRD_PARAMETERS* 
									m_pTimingHRDParameters;

				OLS_TIMING_HRD_PARAMETERS(uint8_t firstSubLayer, uint8_t MaxSubLayersVal, GENERAL_TIMING_HRD_PARAMETERS* pTimingHRDParameters)
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
							parameters.resize((size_t)m_MaxSubLayersVal - m_firstSubLayer + 1);
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
				NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("Tag0", "for(i=firstSubLayer(%d);i&lt;=MaxSubLayersVal(%d);i++)", "", m_firstSubLayer, m_MaxSubLayersVal);
				for (i = 0; i <= m_MaxSubLayersVal - m_firstSubLayer; i++) {
					NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("Tag00", "ols_timing_hrd_parameter[%d]", "", i);
						BST_FIELD_PROP_BOOL1(parameters[i], fixed_pic_rate_general_flag, "", "");
						if (!parameters[i].fixed_pic_rate_general_flag) {
							BST_FIELD_PROP_BOOL1(parameters[i], fixed_pic_rate_within_cvs_flag, "", "");
						}
						if (parameters[i].fixed_pic_rate_within_cvs_flag) {
							BST_FIELD_PROP_2NUMBER("elemental_duration_in_tc_minus1", 
								(long long)quick_log2(parameters[i].elemental_duration_in_tc_minus1 + 1) * 2 + 1, parameters[i].elemental_duration_in_tc_minus1, "")
						}
						else if ((m_pTimingHRDParameters->general_nal_hrd_params_present_flag || 
								  m_pTimingHRDParameters->general_vcl_hrd_params_present_flag) && m_pTimingHRDParameters->hrd_cpb_cnt_minus1 == 0) {
							BST_FIELD_PROP_BOOL1(parameters[i], low_delay_hrd_flag, "", "");
						}

						if (m_pTimingHRDParameters->general_nal_hrd_params_present_flag) {
							NAV_FIELD_PROP_REF2_1(parameters[i].sublayer_nal_hrd_parameters, "sublayer_nal_hrd_parameters", "");
						}

						if (m_pTimingHRDParameters->general_vcl_hrd_params_present_flag) {
							NAV_FIELD_PROP_REF2_1(parameters[i].sublayer_vcl_hrd_parameters, "sublayer_vcl_hrd_parameters", "");
						}

					NAV_WRITE_TAG_END("Tag00");
				}
				NAV_WRITE_TAG_END("Tag0");
				DECLARE_FIELDPROP_END()
			};

			struct VUI_PARAMETERS : public SYNTAX_BITSTREAM_MAP
			{
				uint8_t			vui_progressive_source_flag : 1;
				uint8_t			vui_interlaced_source_flag : 1;
				uint8_t			vui_non_packed_constraint_flag : 1;
				uint8_t			vui_non_projected_constraint_flag : 1;
				uint8_t			vui_aspect_ratio_info_present_flag : 1;
				uint8_t			vui_aspect_ratio_constant_flag : 1;
				uint8_t			vui_overscan_info_present_flag : 1;
				uint8_t			vui_overscan_appropriate_flag : 1;

				uint8_t			vui_aspect_ratio_idc;
				uint16_t		vui_sar_width;
				uint16_t		vui_sar_height;

				uint8_t			vui_colour_description_present_flag : 1;
				uint8_t			vui_full_range_flag : 1;
				uint8_t			vui_chroma_loc_info_present_flag : 1;
				uint8_t			byte_align0 : 5;


				uint8_t			vui_colour_primaries;
				uint8_t			vui_transfer_characteristics;
				uint8_t			vui_matrix_coeffs;

				uint8_t			vui_chroma_sample_loc_type_frame;
				uint8_t			vui_chroma_sample_loc_type_top_field;
				uint8_t			vui_chroma_sample_loc_type_bottom_field;

				uint16_t		m_payloadSize;

				VUI_PARAMETERS(uint16_t payloadSize)
					: vui_aspect_ratio_info_present_flag(0), m_payloadSize(payloadSize) {
				}

				int Map(AMBst in_bst)
				{
					int iRet = RET_CODE_SUCCESS;
					SYNTAX_BITSTREAM_MAP::Map(in_bst);

					try
					{
						int idx = 0;
						MAP_BST_BEGIN(0);
						bsrb1(in_bst, vui_progressive_source_flag, 1);
						bsrb1(in_bst, vui_interlaced_source_flag, 1);
						bsrb1(in_bst, vui_non_packed_constraint_flag, 1);
						bsrb1(in_bst, vui_non_projected_constraint_flag, 1);
						bsrb1(in_bst, vui_aspect_ratio_info_present_flag, 1);

						if (vui_aspect_ratio_info_present_flag) {
							bsrb1(in_bst, vui_aspect_ratio_constant_flag, 1);

							bsrb1(in_bst, vui_aspect_ratio_idc, 8);
							if (vui_aspect_ratio_idc == 255)
							{
								bsrb1(in_bst, vui_sar_width, 16);
								bsrb1(in_bst, vui_sar_height, 16);
							}
						}

						bsrb1(in_bst, vui_overscan_info_present_flag, 1);
						if (vui_overscan_info_present_flag) {
							bsrb1(in_bst, vui_overscan_appropriate_flag, 1);
						}
						bsrb1(in_bst, vui_colour_description_present_flag, 1);
						if (vui_colour_description_present_flag)
						{
							bsrb1(in_bst, vui_colour_primaries, 8);
							bsrb1(in_bst, vui_transfer_characteristics, 8);
							bsrb1(in_bst, vui_matrix_coeffs, 8);
							bsrb1(in_bst, vui_full_range_flag, 1);
						}

						bsrb1(in_bst, vui_chroma_loc_info_present_flag, 1);
						if (vui_chroma_loc_info_present_flag) {
							if (vui_progressive_source_flag && !vui_interlaced_source_flag)
							{
								nal_read_ue(in_bst, vui_chroma_sample_loc_type_frame, uint8_t);
							}
							else
							{
								nal_read_ue(in_bst, vui_chroma_sample_loc_type_top_field, uint8_t);
								nal_read_ue(in_bst, vui_chroma_sample_loc_type_bottom_field, uint8_t);
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
					BST_FIELD_PROP_BOOL(vui_progressive_source_flag, "", "");
					BST_FIELD_PROP_BOOL(vui_interlaced_source_flag, "", "");
					BST_FIELD_PROP_BOOL(vui_non_packed_constraint_flag, "", "");
					BST_FIELD_PROP_BOOL(vui_non_projected_constraint_flag, "", "");
					BST_FIELD_PROP_BOOL_BEGIN(vui_aspect_ratio_info_present_flag, "", "");
						BST_FIELD_PROP_BOOL(vui_aspect_ratio_constant_flag, "", "");
						BST_FIELD_PROP_2NUMBER1(vui_aspect_ratio_idc, 8, sample_aspect_ratio_descs[vui_aspect_ratio_idc]);
						if (vui_aspect_ratio_idc == 255) {
							BST_FIELD_PROP_2NUMBER1(vui_sar_width, 16, "the horizontal size of the sample aspect ratio (in arbitrary units)");
							BST_FIELD_PROP_2NUMBER1(vui_sar_height, 16, "the vertical size of the sample aspect ratio (in the same arbitrary units as sar_width).");
						}
					BST_FIELD_PROP_BOOL_END("vui_aspect_ratio_info_present_flag");

					BST_FIELD_PROP_NUMBER1(vui_overscan_info_present_flag, 1, 
						vui_overscan_info_present_flag ? "the overscan_appropriate_flag is present" : "the preferred display method for the video signal is unspecified");
					if (vui_overscan_info_present_flag) {
						BST_FIELD_PROP_NUMBER1(vui_overscan_appropriate_flag, 1, vui_overscan_appropriate_flag 
							? "indicates that the cropped decoded pictures output are suitable for display using overscan"
							: "indicates that the cropped decoded pictures output contain visually important information in the entire region");
					}

					BST_FIELD_PROP_BOOL_BEGIN(vui_colour_description_present_flag, 
						"specifies that colour_primaries, transfer_characteristics and matrix_coeffs are present",
						"specifies that colour_primaries, transfer_characteristics and matrix_coeffs are not present.");

					if (vui_colour_description_present_flag)
					{
						BST_FIELD_PROP_2NUMBER1(vui_colour_primaries, 8, vui_colour_primaries_names[vui_colour_primaries]);
						BST_FIELD_PROP_2NUMBER1(vui_transfer_characteristics, 8, vui_transfer_characteristics_names[vui_transfer_characteristics]);
						BST_FIELD_PROP_2NUMBER1(vui_matrix_coeffs, 8, vui_matrix_coeffs_descs[vui_matrix_coeffs]);
						BST_FIELD_PROP_BOOL(vui_full_range_flag, "", "");
					}

					BST_FIELD_PROP_BOOL_END("vui_colour_description_present_flag");

					BST_FIELD_PROP_BOOL_BEGIN(vui_chroma_loc_info_present_flag, "", "");
					if (vui_chroma_loc_info_present_flag) {
						if (vui_progressive_source_flag && !vui_interlaced_source_flag) {
							BST_FIELD_PROP_UE(vui_chroma_sample_loc_type_frame, "");
						}
						else
						{
							BST_FIELD_PROP_UE(vui_chroma_sample_loc_type_top_field, "");
							BST_FIELD_PROP_UE(vui_chroma_sample_loc_type_bottom_field, "");
						}
					}
					BST_FIELD_PROP_BOOL_END("vui_chroma_loc_info_present_flag");
				DECLARE_FIELDPROP_END()
			};

			struct VUI_PAYLOAD : public SYNTAX_BITSTREAM_MAP
			{
				VUI_PARAMETERS	vui_parameters;

				uint8_t			vui_payload_bit_equal_to_one : 1;
				uint8_t			byte_align0 : 7;
				CAMBitArray		vui_reserved_payload_extension_data;
				CAMBitArray		vui_payload_bit_equal_to_zero;

				VUI_PAYLOAD(uint16_t payloadSize) : vui_parameters(payloadSize), vui_payload_bit_equal_to_one(0), byte_align0(0){}

				int Map(AMBst in_bst) {
					return SYNTAX_BITSTREAM_MAP::Map(in_bst);
				}

				int Map(uint8_t* pVUIPayloadBuf, uint16_t cbVUIPayloadBuf)
				{
					if (pVUIPayloadBuf == nullptr || cbVUIPayloadBuf <= 0)
						return RET_CODE_INVALID_PARAMETER;

					// Check the last byte to find the stop bit
					uint8_t pLastByte = *(pVUIPayloadBuf + cbVUIPayloadBuf - 1);
					if (pLastByte == 0)
						return RET_CODE_BUFFER_NOT_COMPATIBLE;

					uint8_t nPayloadZeroBits = 0;
					for (uint8_t i = 0; i < 8; i++) {
						if (pLastByte&(1 << i))
							break;

						nPayloadZeroBits++;
					}

					AMBst bst = AMBst_CreateFromBuffer(pVUIPayloadBuf, cbVUIPayloadBuf);
					if (bst == nullptr)
						return RET_CODE_OUTOFMEMORY;

					MAP_BST_BEGIN(0);
					nal_read_obj(bst, vui_parameters);

					int left_bits_in_bst = 0;
					int nEarlierBits = AMBst_Tell(bst, &left_bits_in_bst);

					if (left_bits_in_bst > nPayloadZeroBits + 1) {
						for (int i = 0; i < left_bits_in_bst - (nPayloadZeroBits + 1); i++) {
							bsrbarray(bst, vui_reserved_payload_extension_data, i);
						}
					}

					bsrb1(bst, vui_payload_bit_equal_to_one, 1);
					for (int i = 0; i < nPayloadZeroBits; i++) {
						bsrbarray(bst, vui_payload_bit_equal_to_zero, i);
					}
					MAP_BST_END();

					return RET_CODE_SUCCESS;
				}

				int Unmap(AMBst out_bst)
				{
					UNREFERENCED_PARAMETER(out_bst);
					return RET_CODE_ERROR_NOTIMPL;
				}

				DECLARE_FIELDPROP_BEGIN()
					BST_FIELD_PROP_OBJECT(vui_parameters);
					BST_FIELD_PROP_NUMBER_BITARRAY1(vui_reserved_payload_extension_data, "");
					BST_FIELD_PROP_BOOL(vui_payload_bit_equal_to_one, "Should be 1", "Should be 1")
					BST_FIELD_PROP_NUMBER_BITARRAY1(vui_payload_bit_equal_to_zero, "Should be all 0");
				DECLARE_FIELDPROP_END()
			};

			struct DECODING_CAPABILITY_INFORMATION_RBSP : public SYNTAX_BITSTREAM_MAP
			{
				uint8_t			dci_reserved_zero_4bits : 4;
				uint8_t			dci_num_ptls_minus1 : 4;

				uint8_t			dci_extension_flag : 1;
				uint8_t			byte_align0 : 7;

				std::vector<PROFILE_TIER_LEVEL*>
								profile_tier_level;

				CAMBitArray		dci_extension_data_flag;
				RBSP_TRAILING_BITS
								rbsp_trailing_bits;

				DECODING_CAPABILITY_INFORMATION_RBSP();
				~DECODING_CAPABILITY_INFORMATION_RBSP();
				int Map(AMBst in_bst);
				int Unmap(AMBst out_bst);
				DECLARE_FIELDPROP();
			};

			struct OPERATING_POINT_INFORMATION_RBSP : public SYNTAX_BITSTREAM_MAP
			{
				uint8_t			opi_ols_info_present_flag : 1;
				uint8_t			opi_htid_info_present_flag : 1;
				uint8_t			opi_htid_plus1 : 3;
				uint8_t			opi_extension_flag : 1;
				uint8_t			byte_align0 : 2;

				uint8_t			bytes_align[3] = { 0 };

				uint32_t		opi_ols_idx;

				CAMBitArray		opi_extension_data_flag;

				RBSP_TRAILING_BITS
								rbsp_trailing_bits;

				OPERATING_POINT_INFORMATION_RBSP();
				~OPERATING_POINT_INFORMATION_RBSP();
				int Map(AMBst in_bst);
				int Unmap(AMBst out_bst);
				DECLARE_FIELDPROP();
			};

			struct ADAPTATION_PARAMETER_SET_RBSP : public SYNTAX_BITSTREAM_MAP
			{
				struct ALF_DATA : public SYNTAX_BITSTREAM_MAP
				{
					uint8_t			alf_luma_filter_signal_flag : 1;
					uint8_t			alf_chroma_filter_signal_flag : 1;
					uint8_t			alf_cc_cb_filter_signal_flag : 1;
					uint8_t			alf_cc_cr_filter_signal_flag : 1;
					uint8_t			alf_luma_clip_flag : 1;
					uint8_t			alf_chroma_clip_flag : 1;
					uint8_t			alf_cc_cb_filters_signalled_minus1 : 2;

					uint8_t			alf_chroma_num_alt_filters_minus1 : 3;
					uint8_t			alf_cc_cr_filters_signalled_minus1 : 2;

					uint8_t			alf_luma_num_filters_signalled_minus1;

					std::vector<uint8_t>
									alf_luma_coeff_delta_idx;
					uint8_t			alf_luma_coeff_abs[25][12] = { {0} };
					CAMBitArray		alf_luma_coeff_sign[25];
					uint8_t			alf_luma_clip_idx[25][12] = { {0} };
					uint8_t			alf_chroma_coeff_abs[25][6] = { {0} };
					CAMBitArray		alf_chroma_coeff_sign[25];
					uint8_t			alf_chroma_clip_idx[25][6] = { {0} };

					uint8_t			alf_cc_cb_mapped_coeff_abs[4][7] = { {0} };
					CAMBitArray		alf_cc_cb_coeff_sign[4];

					uint8_t			alf_cc_cr_mapped_coeff_abs[4][7] = { {0} };
					CAMBitArray		alf_cc_cr_coeff_sign[4];

					bool			aps_chroma_present_flag;
					const uint8_t	NumAlfFilters = 25;

					ALF_DATA(bool apsChromaPresentFlag);
					int Map(AMBst in_bst);
					int Unmap(AMBst out_bst);
				};

				struct LMCS_DATA : public SYNTAX_BITSTREAM_MAP
				{
					uint8_t			lmcs_min_bin_idx : 4;
					uint8_t			lmcs_delta_max_bin_idx : 4;

					uint8_t			lmcs_delta_cw_prec_minus1 : 4;
					uint8_t			lmcs_delta_abs_crs : 3;
					uint8_t			lmcs_delta_sign_crs_flag : 1;

					uint16_t		lmcs_delta_abs_cw[16];
					CAMBitArray		lmcs_delta_sign_cw_flag;

					bool			aps_chroma_present_flag;

					LMCS_DATA(bool apsChromaPresentFlag);
					int Map(AMBst in_bst);
					int Unmap(AMBst out_bst);
				};

				struct SCALING_LIST_DATA : public SYNTAX_BITSTREAM_MAP
				{
					CAMBitArray		scaling_list_copy_mode_flag;
					CAMBitArray		scaling_list_pred_mode_flag;
					uint8_t			scaling_list_pred_id_delta[28] = { 0 };
					int8_t			scaling_list_dc_coef[14] = { 0 };
					int8_t			scaling_list_delta_coef[28][64] = { {0} };
					int				ScalingList[28][64] = { {0} };

					bool			aps_chroma_present_flag;
					static uint8_t	DiagScanOrder[4][4][64][2];

					SCALING_LIST_DATA(bool apsChromaPresentFlag);
					int Map(AMBst in_bst);
					int Unmap(AMBst out_bst);

					static void UpdateDiagScanOrder();
				};

				uint8_t			aps_params_type : 3;
				uint8_t			aps_adaptation_parameter_set_id : 5;

				uint8_t			aps_chroma_present_flag : 1;
				uint8_t			aps_extension_flag : 1;
				uint8_t			byte_align0 : 6;

				CAMBitArray		aps_extension_data_flag;

				union
				{
					void*			ptr_void = nullptr;
					ALF_DATA*		alf_data;
					LMCS_DATA*		lmcs_data;
					SCALING_LIST_DATA*	
									scaling_list_data;
				};

				RBSP_TRAILING_BITS
								rbsp_trailing_bits;

				uint8_t			nu_type;

				ADAPTATION_PARAMETER_SET_RBSP(uint8_t NUType);
				~ADAPTATION_PARAMETER_SET_RBSP();
				int Map(AMBst in_bst);
				int Unmap(AMBst out_bst);
				DECLARE_FIELDPROP();
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
				int Map(AMBst in_bst);
				int Unmap(AMBst out_bst);
				DECLARE_FIELDPROP();
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
				CAMBitArray		sps_subpic_treated_as_pic_flag;
				CAMBitArray		sps_loop_filter_across_subpic_enabled_flag;

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

				DPB_PARAMETERS*	dpb_parameters = nullptr;

				uint16_t		sps_log2_diff_min_qt_min_cb_intra_slice_chroma : 3;
				uint16_t		sps_max_mtt_hierarchy_depth_intra_slice_chroma : 4;
				uint16_t		sps_log2_diff_max_bt_min_qt_intra_slice_chroma : 3;
				uint16_t		sps_log2_diff_max_tt_min_qt_intra_slice_chroma : 3;

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

				CAMBitArray		sps_extra_ph_bit_present_flag;
				CAMBitArray		sps_extra_sh_bit_present_flag;

				uint8_t			NumExtraPhBits = 0;
				uint8_t			NumExtraShBits = 0;

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
				uint8_t			sps_affine_enabled_flag : 1;
				uint8_t			sps_five_minus_max_num_subblock_merge_cand : 3;
				uint8_t			sps_6param_affine_enabled_flag : 1;
				uint8_t			sps_affine_amvr_enabled_flag : 1;
				uint8_t			sps_affine_prof_enabled_flag : 1;

				uint8_t			sps_num_ref_pic_lists[2];
				std::shared_ptr<REF_PIC_LIST_STRUCT>
								ref_pic_list_struct[2][64];

				uint32_t		sps_prof_control_present_in_ph_flag : 1;
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
				uint32_t		dword_align1 : 3;

				int8_t			sps_ladf_lowest_interval_qp_offset;
				int8_t			sps_ladf_qp_offset[4];
				uint32_t		sps_ladf_delta_threshold_minus1[4];

				uint8_t			sps_explicit_scaling_list_enabled_flag : 1;
				uint8_t			sps_scaling_matrix_for_lfnst_disabled_flag : 1;
				uint8_t			sps_scaling_matrix_for_alternative_colour_space_disabled_flag : 1;
				uint8_t			sps_scaling_matrix_designated_colour_space_flag : 1;
				uint8_t			sps_dep_quant_enabled_flag : 1;
				uint8_t			sps_sign_data_hiding_enabled_flag : 1;
				uint8_t			sps_virtual_boundaries_enabled_flag : 1;
				uint8_t			sps_virtual_boundaries_present_flag : 1;

				uint8_t			sps_num_ver_virtual_boundaries : 2;
				uint8_t			sps_num_hor_virtual_boundaries : 2;
				uint8_t			sps_timing_hrd_params_present_flag : 1;
				uint8_t			sps_sublayer_cpb_params_present_flag: 1;
				uint8_t			sps_field_seq_flag : 1;
				uint8_t			sps_vui_parameters_present_flag : 1;

				uint32_t		sps_virtual_boundary_pos_x_minus1[3];
				uint32_t		sps_virtual_boundary_pos_y_minus1[3];

				GENERAL_TIMING_HRD_PARAMETERS*
								general_timing_hrd_parameters = nullptr;
				OLS_TIMING_HRD_PARAMETERS*
								ols_timing_hrd_parameters = nullptr;

				uint16_t		sps_vui_payload_size_minus1 : 10;
				uint16_t		sps_extension_flag : 1;
				uint16_t		word_align0 : 5;

				CAMBitArray		sps_vui_alignment_zero_bit;

				VUI_PAYLOAD*	vui_payload = nullptr;

				CAMBitArray		sps_extension_data_flag;

				RBSP_TRAILING_BITS
								rbsp_trailing_bits;

				SEQ_PARAMETER_SET_RBSP();
				~SEQ_PARAMETER_SET_RBSP();

				int Map(AMBst in_bst);
				int Unmap(AMBst out_bst);

				DECLARE_FIELDPROP();
			};

			struct PIC_PARAMETER_SET_RBSP : public SYNTAX_BITSTREAM_MAP
			{
				uint16_t		pps_pic_parameter_set_id : 6;
				uint16_t		pps_seq_parameter_set_id : 4;
				uint16_t		pps_mixed_nalu_types_in_pic_flag : 1;
				uint16_t		pps_conformance_window_flag:1;
				uint16_t		pps_scaling_window_explicit_signalling_flag : 1;
				uint16_t		pps_output_flag_present_flag;
				uint16_t		pps_no_pic_partition_flag;
				uint16_t		pps_subpic_id_mapping_present_flag;


				uint32_t		pps_pic_width_in_luma_samples = 0;
				uint32_t		pps_pic_height_in_luma_samples = 0;

				uint32_t		pps_conf_win_left_offset = 0;
				uint32_t		pps_conf_win_right_offset = 0;
				uint32_t		pps_conf_win_top_offset = 0;
				uint32_t		pps_conf_win_bottom_offset = 0;

				int32_t			pps_scaling_win_left_offset;
				int32_t			pps_scaling_win_right_offset;
				int32_t			pps_scaling_win_top_offset;
				int32_t			pps_scaling_win_bottom_offset;

				uint16_t		pps_num_subpics_minus1 = 0;
				uint8_t			pps_subpic_id_len_minus1;

				uint8_t			pps_log2_ctu_size_minus5 : 2;
				uint8_t			pps_loop_filter_across_tiles_enabled_flag : 1;
				uint8_t			pps_rect_slice_flag : 1;
				uint8_t			pps_single_slice_per_subpic_flag : 1;
				uint8_t			pps_tile_idx_delta_present_flag : 1;
				uint8_t			pps_loop_filter_across_slices_enabled_flag : 1;
				uint8_t			pps_cabac_init_present_flag : 1;

				std::vector<uint16_t>
								pps_subpic_id;
				std::vector<uint16_t>
								SubpicIdVal;

				uint32_t		pps_num_exp_tile_columns_minus1;
				uint32_t		pps_num_exp_tile_rows_minus1;
				std::vector<uint32_t>
								pps_tile_column_width_minus1;
				std::vector<uint32_t>
								pps_tile_row_height_minus1;

				uint16_t		pps_num_slices_in_pic_minus1;
				std::vector<uint32_t>
								pps_slice_width_in_tiles_minus1;
				std::vector<uint32_t>
								pps_slice_height_in_tiles_minus1;
				std::vector<uint32_t>
								pps_num_exp_slices_in_tile;
				std::vector<std::vector<uint16_t>>
								pps_exp_slice_height_in_ctus_minus1;
				std::vector<int32_t>
								pps_tile_idx_delta_val;

				uint8_t			pps_num_ref_idx_default_active_minus1[2] = { 0 };

				uint8_t			pps_rpl1_idx_present_flag : 1;
				uint8_t			pps_weighted_pred_flag : 1;
				uint8_t			pps_weighted_bipred_flag : 1;
				uint8_t			pps_ref_wraparound_enabled_flag : 1;
				uint8_t			pps_cu_qp_delta_enabled_flag : 1;
				uint8_t			pps_chroma_tool_offsets_present_flag : 1;
				uint8_t			pps_joint_cbcr_qp_offset_present_flag : 1;
				uint8_t			pps_slice_chroma_qp_offsets_present_flag : 1;

				int8_t			pps_init_qp_minus26;

				uint32_t		pps_pic_width_minus_wraparound_offset;

				int8_t			pps_cb_qp_offset;
				int8_t			pps_cr_qp_offset;
				int8_t			pps_joint_cbcr_qp_offset_value;

				uint8_t			pps_cu_chroma_qp_offset_list_enabled_flag : 1;
				uint8_t			pps_chroma_qp_offset_list_len_minus1 : 3;
				uint8_t			pps_deblocking_filter_control_present_flag : 1;
				uint8_t			pps_deblocking_filter_override_enabled_flag : 1;
				uint8_t			pps_deblocking_filter_disabled_flag : 1;
				uint8_t			pps_dbf_info_in_ph_flag : 1;

				std::vector<int8_t>
								pps_cb_qp_offset_list;
				std::vector<int8_t>
								pps_cr_qp_offset_list;
				std::vector<int8_t>
								pps_joint_cbcr_qp_offset_list;

				int8_t			pps_luma_beta_offset_div2;
				int8_t			pps_luma_tc_offset_div2;
				int8_t			pps_cb_beta_offset_div2;
				int8_t			pps_cb_tc_offset_div2;
				int8_t			pps_cr_beta_offset_div2;
				int8_t			pps_cr_tc_offset_div2;

				uint8_t			pps_rpl_info_in_ph_flag : 1;
				uint8_t			pps_sao_info_in_ph_flag : 1;
				uint8_t			pps_alf_info_in_ph_flag : 1;
				uint8_t			pps_wp_info_in_ph_flag : 1;
				uint8_t			pps_qp_delta_info_in_ph_flag : 1;
				uint8_t			pps_picture_header_extension_present_flag : 1;
				uint8_t			pps_slice_header_extension_present_flag : 1;
				uint8_t			pps_extension_flag : 1;

				CAMBitArray		pps_extension_data_flag;

				RBSP_TRAILING_BITS
								rbsp_trailing_bits;

				//
				// Concluded values
				//
				int32_t			PicWidthInCtbsY = -1;
				int32_t			PicHeightInCtbsY = -1;
				int32_t			PicSizeInCtbsY = -1;
				int32_t			PicWidthInMinCbsY = -1;
				int32_t			PicHeightInMinCbsY = -1;
				int32_t			PicSizeInMinCbsY = -1;
				int32_t			PicSizeInSamplesY = -1;
				int32_t			PicWidthInSamplesC = -1;
				int32_t			PicHeightInSamplesC = -1;

				uint32_t		NumTileColumns = 0;
				uint32_t		NumTileRows = 0;
				std::vector<uint32_t>
								TileColBdVal;
				std::vector<uint32_t>
								TileRowBdVal;
				std::vector<uint32_t>
								CtbToTileColBd;
				std::vector<uint32_t>
								ctbToTileColIdx;
				std::vector<uint32_t>
								CtbToTileRowBd;
				std::vector<uint32_t>
								ctbToTileRowIdx;
				std::vector<uint32_t>
								SubpicWidthInTiles;
				std::vector<uint32_t>
								SubpicHeightInTiles;
				std::vector<uint8_t>
								subpicHeightLessThanOneTileFlag;
				std::vector<uint32_t>
								NumCtusInSlice;
				std::vector<std::vector<uint32_t>>
								CtbAddrInSlice;
				std::vector<uint16_t>
								SliceTopLeftTileIdx;
				std::vector<uint16_t>
								sliceWidthInTiles;
				std::vector<uint16_t>
								sliceHeightInTiles;
				std::vector<uint16_t>
								NumSlicesInTile;
				std::vector<uint32_t> 
								RowHeightVal;
				std::vector<uint16_t>
								sliceHeightInCtus;
				std::vector<uint16_t>
								NumSlicesInSubpic;

				INALVVCContext*	m_pCtx = nullptr;

				PIC_PARAMETER_SET_RBSP(INALVVCContext*	pCtx);
				~PIC_PARAMETER_SET_RBSP();
				int Map(AMBst bst);
				int Unmap(AMBst out_bst);
				int UpdateNumTileColumnsRows();
				int TileScanning();
				inline void AddCtbsToSlice(uint32_t sliceIdx, uint32_t startX, uint32_t stopX, uint32_t startY, uint32_t stopY) {
					for (uint32_t ctbY = startY; ctbY < stopY; ctbY++)
						for (uint32_t ctbX = startX; ctbX < stopX; ctbX++) {
							CtbAddrInSlice[sliceIdx][NumCtusInSlice[sliceIdx]] = ctbY * PicWidthInCtbsY + ctbX;
							NumCtusInSlice[sliceIdx]++;
						}
				}
				DECLARE_FIELDPROP();
			};

			struct PRED_WEIGHT_TABLE : public SYNTAX_BITSTREAM_MAP
			{
				uint8_t			luma_log2_weight_denom : 3;
				uint8_t			byte_align0 : 5;

				int8_t			delta_chroma_log2_weight_denom;
				uint8_t			num_l0_weights : 4;
				uint8_t			num_l1_weights : 4;
				CAMBitArray		luma_weight_l0_flag;
				CAMBitArray		chroma_weight_l0_flag;

				std::vector<int8_t>
								delta_luma_weight_l0;
				std::vector<int8_t>
								luma_offset_l0;
					
				std::vector<std::array<int8_t, 2>>
								delta_chroma_weight_l0;
				std::vector<std::array<int8_t, 2>>
								delta_chroma_offset_l0;
				
				CAMBitArray		luma_weight_l1_flag;
				CAMBitArray		chroma_weight_l1_flag;

				std::vector<int8_t>
								delta_luma_weight_l1;
				std::vector<int8_t>
								luma_offset_l1;

				std::vector<std::array<int8_t, 2>>
								delta_chroma_weight_l1;
				std::vector<std::array<int8_t, 2>>
								delta_chroma_offset_l1;

				VideoBitstreamCtx*
								m_pCtx;
				// pred_weight_table may exist in either picture header or slice header
				uint8_t			m_pic_parameter_set_id;

				PRED_WEIGHT_TABLE(VideoBitstreamCtx* pCtx, uint8_t pps_id)
					: delta_chroma_log2_weight_denom(0), m_pCtx(pCtx), m_pic_parameter_set_id(pps_id){
				}

				int Map(AMBst bst);
				int Unmap(AMBst out_bst);
			};

			struct REF_PIC_LISTS : public SYNTAX_BITSTREAM_MAP
			{
				CAMBitArray		rpl_sps_flag;
				uint8_t			rpl_idx[2] = { 0, 0 };

				std::shared_ptr<REF_PIC_LIST_STRUCT>
								ref_pic_list_struct[2];

				std::vector<uint16_t>
								poc_lsb_lt[2];
				CAMBitArray		delta_poc_msb_cycle_present_flag[2];
				std::vector<uint32_t>
								delta_poc_msb_cycle_lt[2];

				VideoBitstreamCtx*	
								m_pCtx;
				uint8_t			pic_parameter_set_id;

				REF_PIC_LISTS(VideoBitstreamCtx* pCtx, uint8_t pps_id)
					: m_pCtx(pCtx), pic_parameter_set_id(pps_id) {
				}

				int Map(AMBst bst);
				int Unmap(AMBst out_bst);
			};

			struct PICTURE_HEADER_STRUCTURE : public SYNTAX_BITSTREAM_MAP
			{
				uint16_t		ph_gdr_or_irap_pic_flag : 1;
				uint16_t		ph_non_ref_pic_flag : 1;
				uint16_t		ph_gdr_pic_flag : 1;
				uint16_t		ph_inter_slice_allowed_flag : 1;
				uint16_t		ph_intra_slice_allowed_flag : 1;
				uint16_t		ph_pic_parameter_set_id : 6;
				uint16_t		ph_poc_msb_cycle_present_flag : 1;
				uint16_t		ph_alf_enabled_flag : 1;
				uint16_t		ph_num_alf_aps_ids_luma : 3;

				uint16_t		ph_pic_order_cnt_lsb;
				uint16_t		ph_recovery_poc_cnt;

				CAMBitArray		ph_extra_bit;

				uint32_t		ph_poc_msb_cycle_val;

				std::vector<uint8_t>
								ph_alf_aps_id_luma;

				uint16_t		ph_alf_cb_enabled_flag : 1;
				uint16_t		ph_alf_cr_enabled_flag : 1;
				uint16_t		ph_alf_aps_id_chroma : 3;
				uint16_t		ph_alf_cc_cb_enabled_flag : 1;
				uint16_t		ph_alf_cc_cb_aps_id : 3;
				uint16_t		ph_alf_cc_cr_enabled_flag : 1;
				uint16_t		ph_alf_cc_cr_aps_id : 3;
				uint16_t		ph_lmcs_enabled_flag : 1;
				uint16_t		ph_lmcs_aps_id : 2;

				uint8_t			ph_chroma_residual_scale_flag : 1;
				uint8_t			ph_explicit_scaling_list_enabled_flag : 1;
				uint8_t			ph_scaling_list_aps_id : 3;
				uint8_t			ph_virtual_boundaries_present_flag : 1;
				uint8_t			ph_num_ver_virtual_boundaries : 2;

				uint8_t			ph_num_hor_virtual_boundaries : 2;
				uint8_t			ph_pic_output_flag : 1;
				uint8_t			ph_partition_constraints_override_flag : 1;
				uint8_t			ph_log2_diff_min_qt_min_cb_intra_slice_luma : 3;
				uint8_t			byte_align0 : 1;

				std::vector<uint32_t>
								ph_virtual_boundary_pos_x_minus1;
					
				std::vector<uint32_t>
								ph_virtual_boundary_pos_y_minus1;
					
				REF_PIC_LISTS*	ref_pic_lists;

				uint8_t			ph_max_mtt_hierarchy_depth_intra_slice_luma;
				uint8_t			ph_log2_diff_max_bt_min_qt_intra_slice_luma;

				uint8_t			ph_log2_diff_max_tt_min_qt_intra_slice_luma : 4;
				uint8_t			ph_log2_diff_min_qt_min_cb_intra_slice_chroma : 4;

				uint8_t			ph_max_mtt_hierarchy_depth_intra_slice_chroma;
				uint8_t			ph_log2_diff_max_bt_min_qt_intra_slice_chroma : 4;
				uint8_t			ph_log2_diff_max_tt_min_qt_intra_slice_chroma : 4;

				uint8_t			ph_cu_qp_delta_subdiv_intra_slice;
				uint8_t			ph_cu_chroma_qp_offset_subdiv_intra_slice;

				uint8_t			ph_log2_diff_min_qt_min_cb_inter_slice:4;
				uint8_t			ph_max_mtt_hierarchy_depth_inter_slice:4;
				uint8_t			ph_log2_diff_max_bt_min_qt_inter_slice : 4;
				uint8_t			ph_log2_diff_max_tt_min_qt_inter_slice : 4;

				uint8_t			ph_cu_qp_delta_subdiv_inter_slice;
				uint8_t			ph_cu_chroma_qp_offset_subdiv_inter_slice;

				uint8_t			ph_collocated_ref_idx;

				uint8_t			ph_temporal_mvp_enabled_flag : 1;
				uint8_t			ph_collocated_from_l0_flag : 1;
				uint8_t			ph_mmvd_fullpel_only_flag : 1;
				uint8_t			ph_mvd_l1_zero_flag : 1;
				uint8_t			ph_bdof_disabled_flag : 1;
				uint8_t			ph_dmvr_disabled_flag : 1;
				uint8_t			ph_prof_disabled_flag : 1;
				uint8_t			byte_align1 : 1;
				
				PRED_WEIGHT_TABLE*
								pred_weight_table = nullptr;

				int8_t			ph_qp_delta;
				
				uint8_t			ph_joint_cbcr_sign_flag : 1;
				uint8_t			ph_sao_luma_enabled_flag : 1;
				uint8_t			ph_sao_chroma_enabled_flag : 1;
				uint8_t			ph_deblocking_params_present_flag : 1;
				uint8_t			ph_deblocking_filter_disabled_flag : 1;
				uint8_t			byte_align2 : 3;

				int8_t			ph_luma_beta_offset_div2;
				int8_t			ph_luma_tc_offset_div2;
				int8_t			ph_cb_beta_offset_div2;
				int8_t			ph_cb_tc_offset_div2;
				int8_t			ph_cr_beta_offset_div2;
				int8_t			ph_cr_tc_offset_div2;
				uint16_t		ph_extension_length;

				std::vector<uint8_t>
								ph_extension_data_byte;

				VideoBitstreamCtx*
								m_pCtx;

				PICTURE_HEADER_STRUCTURE(VideoBitstreamCtx* pCtx);
				int Map(AMBst bst);
				int Unmap(AMBst out_bst);
			};

			struct PICTURE_HEADER_RBSP : public SYNTAX_BITSTREAM_MAP
			{
				PICTURE_HEADER_STRUCTURE
								picture_header_structure;

				PICTURE_HEADER_RBSP(VideoBitstreamCtx* pCtx) :
					picture_header_structure(pCtx) {
				}

				int Map(AMBst bst);
				int Unmap(AMBst out_bst);
				DECLARE_FIELDPROP();
			};

			struct SLICE_HEADER : public SYNTAX_BITSTREAM_MAP
			{
				uint16_t		sh_picture_header_in_slice_header_flag : 1;
				uint16_t		sh_slice_type : 3;
				uint16_t		sh_no_output_of_prior_pics_flag : 1;
				uint16_t		sh_alf_enabled_flag : 1;
				uint16_t		sh_num_alf_aps_ids_luma : 3;
				uint16_t		sh_alf_cb_enabled_flag : 1;
				uint16_t		sh_alf_cr_enabled_flag : 1;
				uint16_t		sh_alf_aps_id_chroma : 3;
				uint16_t		sh_alf_cc_cb_enabled_flag : 1;
				uint16_t		sh_alf_cc_cr_enabled_flag : 1;

				uint16_t		sh_subpic_id;

				uint32_t		sh_slice_address;
				CAMBitArray		sh_extra_bit;
				uint16_t		sh_num_tiles_in_slice_minus1;

				uint8_t			sh_alf_aps_id_luma[8];
				
				uint8_t			sh_alf_cc_cb_aps_id : 3;
				uint8_t			sh_alf_cc_cr_aps_id : 3;
				uint8_t			sh_lmcs_used_flag : 1;
				uint8_t			sh_explicit_scaling_list_used_flag : 1;

				uint8_t			sh_num_ref_idx_active_override_flag : 1;
				uint8_t			sh_cabac_init_flag : 1;
				uint8_t			sh_collocated_from_l0_flag: 1;
				uint8_t			sh_cu_chroma_qp_offset_enabled_flag : 1;
				uint8_t			sh_sao_luma_used_flag : 1;
				uint8_t			sh_sao_chroma_used_flag : 1;
				uint8_t			sh_deblocking_params_present_flag : 1;
				uint8_t			sh_dep_quant_used_flag : 1;

				uint8_t			sh_collocated_ref_idx:7;
				uint8_t			sh_deblocking_filter_disabled_flag : 1;

				uint8_t			sh_num_ref_idx_active_minus1[2] = { 0 , 0 };

				int8_t			sh_qp_delta;
				int8_t			sh_cb_qp_offset;
				int8_t			sh_cr_qp_offset;
				int8_t			sh_joint_cbcr_qp_offset;

				int8_t			sh_luma_beta_offset_div2;
				int8_t			sh_luma_tc_offset_div2;
				int8_t			sh_cb_beta_offset_div2;
				int8_t			sh_cb_tc_offset_div2;
				int8_t			sh_cr_beta_offset_div2;
				int8_t			sh_cr_tc_offset_div2;

				uint16_t		sh_sign_data_hiding_used_flag : 1;
				uint16_t		sh_ts_residual_coding_disabled_flag : 1;
				uint16_t		sh_slice_header_extension_length : 9;
				uint16_t		sh_entry_offset_len_minus1 : 5;

				std::vector<uint8_t>
								sh_slice_header_extension_data_byte;

				std::vector<uint32_t>
								sh_entry_point_offset_minus1;

				PICTURE_HEADER_STRUCTURE*
								picture_header_structure = nullptr;

				REF_PIC_LISTS*	ref_pic_lists = nullptr;

				PRED_WEIGHT_TABLE*
								pred_weight_table = nullptr;

				VideoBitstreamCtx*
								m_pCtx;
				int8_t			nal_unit_type;

				uint16_t		NumCtusInCurrSlice = 0;
				std::vector<uint32_t>
								CtbAddrInCurrSlice;

				uint8_t			MinQtLog2SizeY = 0;
				uint8_t			MinQtLog2SizeC = 0;
				uint32_t		MaxBtSizeY = 0;
				uint32_t		MaxBtSizeC = 0;
				uint32_t		MaxTtSizeY = 0;
				uint32_t		MaxTtSizeC = 0;
				uint8_t			MaxMttDepthY = 0;
				uint8_t			MaxMttDepthC = 0;
				uint8_t			CuQpDeltaSubdiv = 0;
				uint8_t			CuChromaQpOffsetSubdiv = 0;

				uint32_t		MinQtSizeY = 0;
				uint32_t		MinQtSizeC = 0;
				uint32_t		MinBtSizeY = 0;
				uint32_t		MinTtSizeY = 0;

				SLICE_HEADER(VideoBitstreamCtx* pCtx, int8_t nu_type);
				~SLICE_HEADER();
				int Map(AMBst bst);
				int Unmap(AMBst out_bst);
			};

			struct SLICE_LAYER_RBSP : public SYNTAX_BITSTREAM_MAP
			{
				SLICE_HEADER	slice_header;

				RBSP_TRAILING_BITS
								rbsp_trailing_bits;

				// If there is a valid rbsp_cabac_zero_word, the upper 16bit should be all zero.
				uint32_t
								rbsp_cabac_zero_word = UINT32_MAX;

				VideoBitstreamCtx*
								m_pCtx;
				int8_t			nal_unit_type;

				SLICE_LAYER_RBSP(VideoBitstreamCtx* pCtx, int8_t nu_type)
					: slice_header(pCtx, nu_type), m_pCtx(pCtx), nal_unit_type(nu_type){
				}

				int Map(AMBst bst);
				int Unmap(AMBst out_bst);
				DECLARE_FIELDPROP();
			};

			NAL_UNIT_HEADER		nal_unit_header;
			union
			{
				void*						ptr_rbsp = nullptr;
				DECODING_CAPABILITY_INFORMATION_RBSP*
											ptr_decoding_capability_information_rbsp;
				OPERATING_POINT_INFORMATION_RBSP*
											ptr_operating_point_information_rbsp;
				ADAPTATION_PARAMETER_SET_RBSP*
											ptr_adaptation_parameter_set_rbsp;
				VIDEO_PARAMETER_SET_RBSP*	ptr_video_parameter_set_rbsp;
				SEQ_PARAMETER_SET_RBSP*		ptr_seq_parameter_set_rbsp;
				PIC_PARAMETER_SET_RBSP*		ptr_pic_parameter_set_rbsp;
				ACCESS_UNIT_DELIMITER_RBSP*	ptr_access_unit_delimiter_rbsp;
				PICTURE_HEADER_RBSP*		ptr_picture_header_rbsp;
				SEI_RBSP*					ptr_sei_rbsp;
				SLICE_LAYER_RBSP*			ptr_slice_layer_rbsp;
			};

			VideoBitstreamCtx*				ptr_ctx_video_bst = nullptr;

			~NAL_UNIT();

			void UpdateCtx(VideoBitstreamCtx* ctx)
			{
				ptr_ctx_video_bst = ctx;
			}

			int Map(AMBst bst);
			int Unmap(AMBst out_bst);

			DECLARE_FIELDPROP();
		};
	}
}

#ifdef _WIN32
#pragma pack(pop)
#pragma warning(pop)
#endif
#undef PACKED

#endif
