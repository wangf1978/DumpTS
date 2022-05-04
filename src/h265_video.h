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
#ifndef _H265_VIDEO_H_
#define _H265_VIDEO_H_

#include "sei.h"
#include <vector>
#include <unordered_map>
#include "DataUtil.h"

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

extern const char* hevc_nal_unit_type_names[64];
extern const char* hevc_nal_unit_type_short_names[64];
extern const char* hevc_nal_unit_type_descs[64];
extern const char* sample_aspect_ratio_descs[256];
extern const char* vui_video_format_names[7];
extern const char* vui_colour_primaries_names[256];
extern const char* vui_transfer_characteristics_names[256];
extern const char* vui_matrix_coeffs_descs[256];
extern const char* get_hevc_profile_name(int profile);

struct GENERAL_TIER_AND_LEVEL_LIMIT
{
	uint32_t	MaxLumaPs;
	uint32_t	MaxCPB_High_tier;
	uint32_t	MaxCPB_Main_tier;
	uint16_t	MaxSliceSegmentsPerPicture;
	uint8_t		MaxTileRows;
	uint8_t		MaxTileCols;

	uint32_t	MaxLumaSr;
	uint32_t	MaxBr_High_tier;
	uint32_t	MaxBr_Main_tier;
	uint8_t		MinCrBase_High_tier;
	uint8_t		MinCrBase_Main_tier;
};

struct HEVC_PROFILE_FACTOR
{
	uint16_t	CpbVclFactor;
	uint16_t	CpbNalFactor;
	float		FormatCapabilityFactor;
	float		MinCrScaleFactor;
};

extern const std::map<uint8_t, GENERAL_TIER_AND_LEVEL_LIMIT> general_tier_and_level_limits;
extern const HEVC_PROFILE_FACTOR hevc_profile_factors[36];

extern GENERAL_TIER_AND_LEVEL_LIMIT get_hevc_tier_and_level_limit(uint8_t level_idc);

namespace BST {

	namespace H265Video {

		enum HEVC_PROFILE
		{
			HEVC_PROFILE_UNKNOWN = -1,
			HEVC_PROFILE_MONOCHROME = 0,
			HEVC_PROFILE_MONOCHROME_10,
			HEVC_PROFILE_MONOCHROME_12,
			HEVC_PROFILE_MONOCHROME_16,
			HEVC_PROFILE_MAIN,
			HEVC_PROFILE_SCREEN_EXTENDED_MAIN,
			HEVC_PROFILE_MAIN_10,
			HEVC_PROFILE_SCREEN_EXTENDED_MAIN_10,
			HEVC_PROFILE_MAIN_12,
			HEVC_PROFILE_MAIN_STILL_PICTURE,
			HEVC_PROFILE_MAIN_10_STILL_PICTURE,
			HEVC_PROFILE_MAIN_422_10,
			HEVC_PROFILE_MAIN_422_12,
			HEVC_PROFILE_MAIN_444,
			HEVC_PROFILE_HIGH_THROUGHPUT_444,
			HEVC_PROFILE_SCREEN_EXTENDED_MAIN_444,
			HEVC_PROFILE_SCREEN_EXTENDED_HIGH_THROUGHPUT_444,
			HEVC_PROFILE_MAIN_444_10,
			HEVC_PROFILE_HIGH_THROUGHPUT_444_10,
			HEVC_PROFILE_SCREEN_EXTENDED_MAIN_444_10,
			HEVC_PROFILE_SCREEN_EXTENDED_HIGH_THROUGHPUT_444_10,
			HEVC_PROFILE_MAIN_444_12,
			HEVC_PROFILE_HIGH_THROUGHPUT_444_14,
			HEVC_PROFILE_SCREEN_EXTENDED_HIGH_THROUGHPUT_444_14,
			HEVC_PROFILE_MAIN_INTRA,
			HEVC_PROFILE_MAIN_10_INTRA,
			HEVC_PROFILE_MAIN_12_INTRA,
			HEVC_PROFILE_MAIN_422_10_INTRA,
			HEVC_PROFILE_MAIN_422_12_INTRA,
			HEVC_PROFILE_MAIN_444_INTRA,
			HEVC_PROFILE_MAIN_444_10_INTRA,
			HEVC_PROFILE_MAIN_444_12_INTRA,
			HEVC_PROFILE_MAIN_444_16_INTRA,
			HEVC_PROFILE_MAIN_444_STILL_PICTURE,
			HEVC_PROFILE_MAIN_444_16_STILL_PICTURE,
			HEVC_PROFILE_HIGH_THROUGHPUT_444_16_INTRA,
		};

		enum HEVC_TIER
		{
			HEVC_TIER_UNKNOWN = -1,
			HEVC_TIER_MAIN,
			HEVC_TIER_HIGH
		};

		enum HEVC_LEVEL
		{
			HEVC_LEVEL_UNKNOWN = -1,
			HEVC_LEVEL_1	= 30,	// 1
			HEVC_LEVEL_2	= 60,	// 2
			HEVC_LEVEL_2_1	= 63,	// 2.1
			HEVC_LEVEL_3	= 90,	// 3
			HEVC_LEVEL_3_1	= 93,	// 3.1
			HEVC_LEVEL_4	= 120,	// 4
			HEVC_LEVEL_4_1	= 123,	// 4.1
			HEVC_LEVEL_5	= 150,	// 5
			HEVC_LEVEL_5_1	= 153,	// 5.1
			HEVC_LEVEL_5_2	= 156,	// 5.2
			HEVC_LEVEL_6	= 180,	// 6
			HEVC_LEVEL_6_1	= 183,	// 6.1
			HEVC_LEVEL_6_2	= 186,	// 6.2
		};

		enum NAL_UNIT_TYPE
		{
			TRAIL_N				= 0,
			TRAIL_R				= 1,
			TSA_N				= 2,
			TSA_R				= 3,
			STSA_N				= 4,
			STSA_R				= 5,
			RADL_N				= 6,
			RADL_R				= 7,
			RASL_N				= 8,
			RASL_R				= 9,
			RSV_VCL_N10			= 10,
			RSV_VCL_N12			= 12,
			RSV_VCL_N14			= 14,
			RSV_VCL_R11			= 11,
			RSV_VCL_R13			= 13,
			RSV_VCL_R15			= 15,
			BLA_W_LP			= 16,
			BLA_W_RADL			= 17,
			BLA_N_LP			= 18,
			IDR_W_RADL			= 19,
			IDR_N_LP			= 20,
			CRA_NUT				= 21,
			RSV_IRAP_VCL22 		= 22,
			RSV_IRAP_VCL23		= 23,
			RSV_VCL24			= 24,
			RSV_VCL31			= 31,
			VPS_NUT				= 32,
			SPS_NUT				= 33,
			PPS_NUT				= 34,
			AUD_NUT				= 35,
			EOS_NUT				= 36,
			EOB_NUT				= 37,
			FD_NUT				= 38,
			PREFIX_SEI_NUT		= 39,
			SUFFIX_SEI_NUT		= 40,
			RSV_NVCL41			= 41,
			RSV_NVCL47			= 47,
			UNSPEC48			= 48,
			UNSPEC63			= 63,
		};

		const uint8_t B_SLICE = 0;
		const uint8_t P_SLICE = 1;
		const uint8_t I_SLICE = 2;

		struct NAL_UNIT;

		class VideoBitstreamCtx : public CComUnknown, public INALHEVCContext
		{
		public:
			DECLARE_IUNKNOWN

			HRESULT NonDelegatingQueryInterface(REFIID uuid, void** ppvObj)
			{
				if (ppvObj == NULL)
					return E_POINTER;

				if (uuid == IID_INALContext)
					return GetCOMInterface((INALContext*)this, ppvObj);
				else if (uuid == IID_INALHEVCContext)
					return GetCOMInterface((INALHEVCContext*)this, ppvObj);

				return CComUnknown::NonDelegatingQueryInterface(uuid, ppvObj);
			}

			NAL_CODING					GetNALCoding() { return NAL_CODING_HEVC; }
			RET_CODE					SetNUFilters(std::initializer_list<uint8_t> NU_type_filters);
			RET_CODE					GetNUFilters(std::vector<uint8_t>& NU_type_filters);
			bool						IsNUFiltered(uint8_t nal_unit_type);
			RET_CODE					SetActiveNUType(int8_t nu_type) { m_active_nu_type = nu_type; return RET_CODE_SUCCESS; }
			int8_t						GetActiveNUType() { return m_active_nu_type; }
			void						Reset();
			H265_NU						GetHEVCVPS(uint8_t vps_id);
			H265_NU						GetHEVCSPS(uint8_t sps_id);
			H265_NU						GetHEVCPPS(uint8_t pps_id);
			RET_CODE					UpdateHEVCVPS(H265_NU vps_nu);
			RET_CODE					UpdateHEVCSPS(H265_NU sps_nu);
			RET_CODE					UpdateHEVCPPS(H265_NU pps_nu);
			H265_NU						CreateHEVCNU();

			int8_t						GetActiveSPSID();
			RET_CODE					ActivateSPS(int8_t sps_id);
			RET_CODE					DetactivateSPS();

		public:
			std::vector<uint8_t>		nal_unit_type_filters;
			int8_t						in_scanning = 0;
			std::unordered_map<uint8_t, int8_t>
										sps_seq_parameter_set_id;
			int8_t						prev_vps_video_parameter_set_id =  -1;
			std::map<uint8_t, H265_NU>	sp_h265_vpses;		// the smart pointers of VPS for the current h265 bitstream
			std::map<uint8_t, H265_NU>	sp_h265_spses;		// the smart pointers of SPS for the current h265 bitstream
			std::map<uint8_t, H265_NU>	sp_h265_ppses;		// the smart pointers of PPS for the current h265 bitstream
			H265_NU						sp_prev_nal_unit;
			int8_t						m_active_sps_id = -1;
			int8_t						m_active_nu_type = -1;
		};
		
		struct NAL_UNIT : public SYNTAX_BITSTREAM_MAP
		{
			struct NAL_UNIT_HEADER : public SYNTAX_BITSTREAM_MAP
			{
				uint16_t	forbidden_zero_bit:1;
				uint16_t	nal_unit_type:6;
				uint16_t	nuh_layer_id:6;
				uint16_t	nuh_temporal_id_plus1:3;

				NAL_UNIT_HEADER()
					: forbidden_zero_bit(0), nal_unit_type(0), nuh_layer_id(0), nuh_temporal_id_plus1(0){
				}

				int Map(AMBst in_bst)
				{
					SYNTAX_BITSTREAM_MAP::Map(in_bst);
					try
					{
						MAP_BST_BEGIN(0);
						nal_read_f(in_bst, forbidden_zero_bit, 1, uint16_t);
						nal_read_u(in_bst, nal_unit_type, 6, uint16_t);
						nal_read_u(in_bst, nuh_layer_id, 6, uint16_t);
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
					BST_FIELD_PROP_NUMBER1 (forbidden_zero_bit, 1, "shall be equal to 0.")
					BST_FIELD_PROP_2NUMBER1(nal_unit_type, 6, hevc_nal_unit_type_descs[nal_unit_type])
					BST_FIELD_PROP_2NUMBER1(nuh_layer_id, 6, nal_unit_type == EOB_NUT?"shall be equal to 0.":"the identifier of the layer to which a VCL NAL unit belongs or the identifier of a layer to which a non-VCL NAL unit applies")
					BST_FIELD_PROP_2NUMBER1(nuh_temporal_id_plus1, 3, (nal_unit_type >= BLA_W_LP && nal_unit_type <= RSV_IRAP_VCL23) || 
																		(nal_unit_type == TSA_R || nal_unit_type == TSA_N) ||
																		((nal_unit_type == STSA_R || nal_unit_type == STSA_N) && nuh_layer_id == 0) ||
																		(nal_unit_type == VPS_NUT || nal_unit_type == SPS_NUT || nal_unit_type == EOS_NUT || nal_unit_type == EOB_NUT)?
																		"Should be 1":
																		"TemporalId = nuh_temporal_id_plus1 - 1")
				DECLARE_FIELDPROP_END()

			}PACKED;

			struct HRD_PARAMETERS : public SYNTAX_BITSTREAM_MAP
			{
				struct SUB_LAYER_INFO: public SYNTAX_BITSTREAM_MAP
				{
					struct SUB_LAYER_HRD_PARAMETERS : public SYNTAX_BITSTREAM_MAP
					{
						struct SUB_LAYER_HRD_PARAMETER : public SYNTAX_BITSTREAM_MAP
						{
							uint32_t	bit_rate_value_minus1 = 0;
							uint32_t	cpb_size_value_minus1 = 0;
							uint32_t	cpb_size_du_value_minus1 = 0;
							uint32_t	bit_rate_du_value_minus1 = 0;
							uint8_t		cbr_flag = 0;
							SUB_LAYER_HRD_PARAMETERS*
										m_ptr_sub_layer_hrd_parameters;

							SUB_LAYER_HRD_PARAMETER(SUB_LAYER_HRD_PARAMETERS* ptr_sub_layer_hrd_parameters) 
								: m_ptr_sub_layer_hrd_parameters(ptr_sub_layer_hrd_parameters) {}

							int Map(AMBst bst)
							{
								int iRet = RET_CODE_SUCCESS;
								SYNTAX_BITSTREAM_MAP::Map(bst);

								try
								{
									MAP_BST_BEGIN(0);
									nal_read_ue(bst, bit_rate_value_minus1, uint32_t);
									nal_read_ue(bst, cpb_size_value_minus1, uint32_t);
									if (m_ptr_sub_layer_hrd_parameters->m_sub_layer_info->m_ptr_hdr_parameters->sub_pic_hrd_params_present_flag) {
										nal_read_ue(bst, cpb_size_du_value_minus1, uint32_t);
										nal_read_ue(bst, bit_rate_du_value_minus1, uint32_t);
									}
									nal_read_u(bst, cbr_flag, 1, uint8_t);
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
								BST_FIELD_PROP_2NUMBER1(bit_rate_value_minus1, (long long)quick_log2(bit_rate_du_value_minus1 + 1) * 2 + 1, "")
								BST_FIELD_PROP_2NUMBER1(cpb_size_value_minus1, (long long)quick_log2(cpb_size_value_minus1 + 1) * 2 + 1, "")
								if (m_ptr_sub_layer_hrd_parameters->m_sub_layer_info->m_ptr_hdr_parameters->sub_pic_hrd_params_present_flag)
								{
									BST_FIELD_PROP_2NUMBER1(cpb_size_du_value_minus1, (long long)quick_log2(cpb_size_du_value_minus1 + 1) * 2 + 1, "")
									BST_FIELD_PROP_2NUMBER1(bit_rate_du_value_minus1, (long long)quick_log2(bit_rate_du_value_minus1 + 1) * 2 + 1, "")
								}
								BST_FIELD_PROP_2NUMBER1(cbr_flag, 1, cbr_flag ? "the HSS operates in a constant bit rate (CBR) mode" : "the HSS operates in an intermittent bit rate mode")

								if (m_ptr_sub_layer_hrd_parameters->m_sub_layer_info->m_ptr_hdr_parameters &&
									m_ptr_sub_layer_hrd_parameters->m_sub_layer_info->m_ptr_hdr_parameters->m_commonInfPresentFlag)
								{
									uint8_t bit_rate_scale = m_ptr_sub_layer_hrd_parameters->m_sub_layer_info->m_ptr_hdr_parameters->bit_rate_scale;
									uint8_t cpb_size_scale = m_ptr_sub_layer_hrd_parameters->m_sub_layer_info->m_ptr_hdr_parameters->cpb_size_scale;
									NAV_WRITE_TAG_WITH_VALUEFMTSTR("BitRate", "%sbps", "The bit rate in bits per second", 
										GetReadableNum(((uint64_t)bit_rate_value_minus1 + 1) << (6 + bit_rate_scale)).c_str());
									NAV_WRITE_TAG_WITH_VALUEFMTSTR("CpbSize", "%sbit", "The CPB size in bits", 
										GetReadableNum(((uint64_t)cpb_size_value_minus1 + 1) << (4 + cpb_size_scale)).c_str());
								}
							DECLARE_FIELDPROP_END()
						}PACKED;

						SUB_LAYER_HRD_PARAMETER** sub_layer_hrd_parameters;
						SUB_LAYER_INFO*	m_sub_layer_info;

						SUB_LAYER_HRD_PARAMETERS(SUB_LAYER_INFO* ptr_sub_layer_info) : m_sub_layer_info(ptr_sub_layer_info){
							if (m_sub_layer_info->cpb_cnt_minus1 >= 0)
							{
								sub_layer_hrd_parameters = new SUB_LAYER_HRD_PARAMETER*[(size_t)m_sub_layer_info->cpb_cnt_minus1 + 1];
								for (unsigned long i = 0; i <= m_sub_layer_info->cpb_cnt_minus1; i++)
									sub_layer_hrd_parameters[i] = NULL;
							}
							else
								sub_layer_hrd_parameters = NULL;
						}

						virtual ~SUB_LAYER_HRD_PARAMETERS() {
							for (unsigned long i = 0; i <= m_sub_layer_info->cpb_cnt_minus1 && sub_layer_hrd_parameters != NULL; i++) {
								AMP_SAFEDEL2(sub_layer_hrd_parameters[i]);
							}
							AMP_SAFEDELA3(sub_layer_hrd_parameters);
						}

						int Map(AMBst bst)
						{
							int iRet = RET_CODE_SUCCESS;
							SYNTAX_BITSTREAM_MAP::Map(bst);

							try
							{
								MAP_BST_BEGIN(0);
								for (unsigned long i = 0; i <= m_sub_layer_info->cpb_cnt_minus1; i++) {
									nal_read_ref(bst, sub_layer_hrd_parameters[i], SUB_LAYER_HRD_PARAMETER, this);
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
						for (i = 0; i <= m_sub_layer_info->cpb_cnt_minus1; i++) {
							NAV_FIELD_PROP_REF3(sub_layer_hrd_parameters[i], "sub_layer_hrd_parameters", i);
						}
						DECLARE_FIELDPROP_END()
					}PACKED;

					uint8_t		fixed_pic_rate_general_flag : 1;
					uint8_t		fixed_pic_rate_within_cvs_flag : 1;
					uint8_t		low_delay_hrd_flag : 1;
					uint8_t		reserved : 5;

					uint16_t	elemental_duration_in_tc_minus1 = 0;
					uint8_t		cpb_cnt_minus1 = 0;

					SUB_LAYER_HRD_PARAMETERS*
								ptr_nal_sub_layer_hrd_parameters;
					SUB_LAYER_HRD_PARAMETERS*
								ptr_vcl_sub_layer_hrd_parameters;

					HRD_PARAMETERS*
								m_ptr_hdr_parameters;

					SUB_LAYER_INFO(HRD_PARAMETERS* ptr_hdr_parameters)
						: fixed_pic_rate_general_flag(0)
						, fixed_pic_rate_within_cvs_flag(0)
						, low_delay_hrd_flag(0)
						, reserved(0)
						, ptr_nal_sub_layer_hrd_parameters(NULL)
						, ptr_vcl_sub_layer_hrd_parameters(NULL)
						, m_ptr_hdr_parameters(ptr_hdr_parameters) {}

					virtual ~SUB_LAYER_INFO() {
						UNMAP_STRUCT_POINTER5(ptr_nal_sub_layer_hrd_parameters);
						UNMAP_STRUCT_POINTER5(ptr_vcl_sub_layer_hrd_parameters);
					}

					int Map(AMBst bst)
					{
						int iRet = RET_CODE_SUCCESS;
						SYNTAX_BITSTREAM_MAP::Map(bst);

						try
						{
							MAP_BST_BEGIN(0);
							nal_read_u(bst, fixed_pic_rate_general_flag, 1, uint8_t);
							if (!fixed_pic_rate_general_flag) {
								nal_read_u(bst, fixed_pic_rate_within_cvs_flag, 1, uint8_t);
							}
							else
								fixed_pic_rate_within_cvs_flag = (uint8_t)1;

							if (fixed_pic_rate_within_cvs_flag) {
								nal_read_ue(bst, elemental_duration_in_tc_minus1, uint16_t);
								low_delay_hrd_flag = 0;
							} else {
								nal_read_u(bst, low_delay_hrd_flag, 1, uint8_t);
							}

							if (!low_delay_hrd_flag) {
								nal_read_ue(bst, cpb_cnt_minus1, uint8_t);
							}
							else
								cpb_cnt_minus1 = 0;

							nal_read_ref1(bst, m_ptr_hdr_parameters->nal_hrd_parameters_present_flag, ptr_nal_sub_layer_hrd_parameters,
								SUB_LAYER_HRD_PARAMETERS, this);

							nal_read_ref1(bst, m_ptr_hdr_parameters->vcl_hrd_parameters_present_flag, ptr_vcl_sub_layer_hrd_parameters,
								SUB_LAYER_HRD_PARAMETERS, this);

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
						BST_FIELD_PROP_NUMBER1(fixed_pic_rate_general_flag, 1, "")
						if (!fixed_pic_rate_general_flag) {
							BST_FIELD_PROP_NUMBER1(fixed_pic_rate_within_cvs_flag, 1, "")
						}
						if (fixed_pic_rate_within_cvs_flag) {
							BST_FIELD_PROP_2NUMBER1(elemental_duration_in_tc_minus1, (long long)quick_log2(elemental_duration_in_tc_minus1 + 1) * 2 + 1, "")
						}
						else {
							BST_FIELD_PROP_NUMBER1(low_delay_hrd_flag, 1, "")
						}
						if (!low_delay_hrd_flag) {
							BST_FIELD_PROP_2NUMBER1(cpb_cnt_minus1, (long long)quick_log2(cpb_cnt_minus1 + 1) * 2 + 1, "the number of alternative CPB specifications in the bitstream of the CVS when HighestTid is equal to i, value should be in the range 0	to 31")
						}

						if (m_ptr_hdr_parameters->nal_hrd_parameters_present_flag) {
							BST_FIELD_PROP_REF2(ptr_nal_sub_layer_hrd_parameters, "nal_sub_layer_hrd_parameters");
						}

						if (m_ptr_hdr_parameters->vcl_hrd_parameters_present_flag) {
							BST_FIELD_PROP_REF2(ptr_vcl_sub_layer_hrd_parameters, "vcl_sub_layer_hrd_parameters");
						}
					DECLARE_FIELDPROP_END()
							
				}PACKED;

				uint8_t		nal_hrd_parameters_present_flag : 1;
				uint8_t		vcl_hrd_parameters_present_flag : 1;
				uint8_t		sub_pic_hrd_params_present_flag : 1;
				uint8_t		reserved_0 : 5;

				uint8_t		tick_divisor_minus2;
				uint8_t		du_cpb_removal_delay_increment_length_minus1 : 5;
				uint8_t		sub_pic_cpb_params_in_pic_timing_sei_flag : 1;
				uint8_t		reserved_1 : 2;
				uint8_t		dpb_output_delay_du_length_minus1;
			
				uint8_t		bit_rate_scale : 4;
				uint8_t		cpb_size_scale : 4;

				uint8_t		cpb_size_du_scale;
				uint8_t		initial_cpb_removal_delay_length_minus1;
				uint8_t		au_cpb_removal_delay_length_minus1;
				uint8_t		dpb_output_delay_length_minus1;

				SUB_LAYER_INFO**
							sub_layer_infos;

				BOOL		m_commonInfPresentFlag;
				uint8_t		m_maxNumSubLayersMinus1;

				HRD_PARAMETERS(BOOL commonInfPresentFlag, uint8_t maxNumSubLayersMinus1) 
					: au_cpb_removal_delay_length_minus1(23)
					, dpb_output_delay_length_minus1(23)
					, m_commonInfPresentFlag(commonInfPresentFlag)
					, m_maxNumSubLayersMinus1(maxNumSubLayersMinus1){
					sub_layer_infos = new SUB_LAYER_INFO*[(size_t)maxNumSubLayersMinus1 + 1];
				}

				virtual ~HRD_PARAMETERS() {
					for (int i = 0; i <= m_maxNumSubLayersMinus1; i++) {
						AMP_SAFEDEL2(sub_layer_infos[i]);
					}
					AMP_SAFEDELA3(sub_layer_infos);
				}


				int Map(AMBst bst)
				{
					int iRet = RET_CODE_SUCCESS;
					SYNTAX_BITSTREAM_MAP::Map(bst);

					try
					{
						MAP_BST_BEGIN(0);

						if (m_commonInfPresentFlag) {
							nal_read_u(bst, nal_hrd_parameters_present_flag, 1, uint8_t);
							nal_read_u(bst, vcl_hrd_parameters_present_flag, 1, uint8_t);

							if (nal_hrd_parameters_present_flag || vcl_hrd_parameters_present_flag)
							{
								nal_read_u(bst, sub_pic_hrd_params_present_flag, 1, uint8_t);

								if (sub_pic_hrd_params_present_flag) {
									nal_read_u(bst, tick_divisor_minus2, 8, uint8_t);
									nal_read_u(bst, du_cpb_removal_delay_increment_length_minus1, 5, uint8_t);
									nal_read_u(bst, sub_pic_cpb_params_in_pic_timing_sei_flag, 1, uint8_t);
									nal_read_u(bst, dpb_output_delay_du_length_minus1, 5, uint8_t);
								}

								nal_read_u(bst, bit_rate_scale, 4, uint8_t);
								nal_read_u(bst, cpb_size_scale, 4, uint8_t);

								if (sub_pic_hrd_params_present_flag) {
									nal_read_u(bst, cpb_size_du_scale, 4, uint8_t);
								}

								nal_read_u(bst, initial_cpb_removal_delay_length_minus1, 5, uint8_t);
								nal_read_u(bst, au_cpb_removal_delay_length_minus1, 5, uint8_t);
								nal_read_u(bst, dpb_output_delay_length_minus1, 5, uint8_t);
							}
							else
							{
								sub_pic_hrd_params_present_flag = 0;
							}
						}

						for (int i = 0; i <= m_maxNumSubLayersMinus1; i++)
						{
							nal_read_ref(bst, sub_layer_infos[i], SUB_LAYER_INFO, this);
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
				if (m_commonInfPresentFlag)
				{
					BST_FIELD_PROP_NUMBER1(nal_hrd_parameters_present_flag, 1, "")
					BST_FIELD_PROP_NUMBER1(vcl_hrd_parameters_present_flag, 1, "")
					if (nal_hrd_parameters_present_flag || vcl_hrd_parameters_present_flag)
					{
						BST_FIELD_PROP_NUMBER1(sub_pic_hrd_params_present_flag, 1, "")
						if (sub_pic_hrd_params_present_flag) {
							BST_FIELD_PROP_2NUMBER1(tick_divisor_minus2, 8, "")
							BST_FIELD_PROP_2NUMBER1(du_cpb_removal_delay_increment_length_minus1, 5, "")
							BST_FIELD_PROP_NUMBER1(sub_pic_cpb_params_in_pic_timing_sei_flag, 1, "")
							BST_FIELD_PROP_2NUMBER1(dpb_output_delay_du_length_minus1, 5, "")
						}

						BST_FIELD_PROP_2NUMBER1(bit_rate_scale, 4, "")
						BST_FIELD_PROP_2NUMBER1(cpb_size_scale, 4, "")

						if (sub_pic_hrd_params_present_flag) {
							BST_FIELD_PROP_2NUMBER1(cpb_size_du_scale, 4, "")
						}

						BST_FIELD_PROP_2NUMBER1(initial_cpb_removal_delay_length_minus1, 5, "")
						BST_FIELD_PROP_2NUMBER1(au_cpb_removal_delay_length_minus1, 5, "")
						BST_FIELD_PROP_2NUMBER1(dpb_output_delay_length_minus1, 5, "")
					}
				}

				for (i = 0; i <= m_maxNumSubLayersMinus1; i++) {
					BST_FIELD_PROP_REF3(sub_layer_infos[i], "SubLayer", i);
				}

				DECLARE_FIELDPROP_END()

			}PACKED;

			struct PROFILE_TIER_LEVEL : public SYNTAX_BITSTREAM_MAP
			{
				struct PROFILE_LEVEL : public SYNTAX_BITSTREAM_MAP
				{
					uint8_t		sub_layer : 1;
					uint8_t		profile_present_flag : 1;
					uint8_t		level_present_flag : 1;
					uint8_t		padding_bits : 5;

					uint64_t	profile_space : 2;
					uint64_t	tier_flag : 1;
					uint64_t	profile_idc : 5;

					uint64_t	progressive_source_flag : 1;
					uint64_t	interlaced_source_flag : 1;
					uint64_t	non_packed_constraint_flag : 1;
					uint64_t	frame_only_constraint_flag : 1;
					uint64_t	max_12bit_constraint_flag : 1;
					uint64_t	max_10bit_constraint_flag : 1;
					uint64_t	max_8bit_constraint_flag : 1;
					uint64_t	max_422chroma_constraint_flag : 1;

					uint64_t	max_420chroma_constraint_flag : 1;
					uint64_t	max_monochrome_constraint_flag : 1;
					uint64_t	intra_constraint_flag : 1;
					uint64_t	one_picture_only_constraint_flag : 1;
					uint64_t	lower_bit_rate_constraint_flag : 1;
					uint64_t	max_14bit_constraint_flag : 1;
					uint64_t	reserved_zero_33bits : 33;
					uint64_t	inbld_flag : 1;

					uint64_t	level_idc : 8;

					CAMBitArray	profile_compatibility_flag;

					int Map(AMBst in_bst)
					{
						SYNTAX_BITSTREAM_MAP::Map(in_bst);
						try
						{
							MAP_BST_BEGIN(0);
							if (profile_present_flag)
							{
								nal_read_u(in_bst, profile_space, 2, uint64_t);
								nal_read_u(in_bst, tier_flag, 1, uint64_t);
								nal_read_u(in_bst, profile_idc, 5, uint64_t);

								for (int i = 0; i < 32; i++) {
									nal_read_bitarray(in_bst, profile_compatibility_flag, i);
								}

								nal_read_u(in_bst, progressive_source_flag, 1, uint64_t);
								nal_read_u(in_bst, interlaced_source_flag, 1, uint64_t);
								nal_read_u(in_bst, non_packed_constraint_flag, 1, uint64_t);
								nal_read_u(in_bst, frame_only_constraint_flag, 1, uint64_t);
								nal_read_u(in_bst, max_12bit_constraint_flag, 1, uint64_t);
								nal_read_u(in_bst, max_10bit_constraint_flag, 1, uint64_t);
								nal_read_u(in_bst, max_8bit_constraint_flag, 1, uint64_t);
								nal_read_u(in_bst, max_422chroma_constraint_flag, 1, uint64_t);

								nal_read_u(in_bst, max_420chroma_constraint_flag, 1, uint64_t);
								nal_read_u(in_bst, max_monochrome_constraint_flag, 1, uint64_t);
								nal_read_u(in_bst, intra_constraint_flag, 1, uint64_t);
								nal_read_u(in_bst, one_picture_only_constraint_flag, 1, uint64_t);
								nal_read_u(in_bst, lower_bit_rate_constraint_flag, 1, uint64_t);
								//if (profile_idc == 5 || profile_compatibility_flag[5] ||
								//	profile_idc == 9 || profile_compatibility_flag[9] ||
								//	profile_idc == 10 || profile_compatibility_flag[10] ||
								//	profile_idc == 11 || profile_compatibility_flag[11])
								//{
								//}
								nal_read_u(in_bst, max_14bit_constraint_flag, 1, uint64_t);
								nal_read_u(in_bst, reserved_zero_33bits, 33, uint64_t);
								nal_read_u(in_bst, inbld_flag, 1, uint64_t);
							}

							if (level_present_flag) {
								nal_read_u(in_bst, level_idc, 8, uint64_t);
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
					if (profile_present_flag) {
						BST_FIELD_PROP_2NUMBER(sub_layer ? "sub_layer_profile_space" : "general_profile_space", 2, profile_space, "")
						BST_FIELD_PROP_2NUMBER(sub_layer ? "sub_layer_tier_flag" : "general_tier_flag", 1, tier_flag, "")
						BST_FIELD_PROP_2NUMBER(sub_layer ? "sub_layer_profile_idc" : "general_profile_idc", 5, profile_idc, "")

						NAV_WRITE_TAG_BEGIN2_1("profile_compatibility_flag", "");
						for (i = 0; i < 32; i++) {
							BST_ARRAY_FIELD_PROP_NUMBER(sub_layer ? "sub_layer_profile_compatibility_flag" : "general_profile_compatibility_flag", i, 1, profile_compatibility_flag[i], "")
						}
						NAV_WRITE_TAG_END2("profile_compatibility_flag");

						BST_FIELD_PROP_NUMBER(sub_layer ? "sub_layer_progressive_source_flag" : "general_progressive_source_flag", 1, progressive_source_flag, "")
						BST_FIELD_PROP_NUMBER(sub_layer ? "sub_layer_interlaced_source_flag" : "general_interlaced_source_flag", 1, interlaced_source_flag, "")
						BST_FIELD_PROP_NUMBER(sub_layer ? "sub_layer_non_packed_constraint_flag" : "general_non_packed_constraint_flag", 1, non_packed_constraint_flag, "")
						BST_FIELD_PROP_NUMBER(sub_layer ? "sub_layer_frame_only_constraint_flag" : "general_frame_only_constraint_flag", 1, frame_only_constraint_flag, "")
						if (profile_idc == 4 || profile_compatibility_flag[4] ||
							profile_idc == 5 || profile_compatibility_flag[5] ||
							profile_idc == 6 || profile_compatibility_flag[6] ||
							profile_idc == 7 || profile_compatibility_flag[7]) {
							/* The number of bits in this syntax structure is not affected by this condition */
							BST_FIELD_PROP_NUMBER(sub_layer ? "sub_layer_max_12bit_constraint_flag" : "general_max_12bit_constraint_flag", 1, max_12bit_constraint_flag, "")
							BST_FIELD_PROP_NUMBER(sub_layer ? "sub_layer_max_10bit_constraint_flag" : "general_max_10bit_constraint_flag", 1, max_10bit_constraint_flag, "")
							BST_FIELD_PROP_NUMBER(sub_layer ? "sub_layer_max_8bit_constraint_flag" : "general_max_8bit_constraint_flag", 1, max_8bit_constraint_flag, "")
							BST_FIELD_PROP_NUMBER(sub_layer ? "sub_layer_max_422chroma_constraint_flag" : "general_max_422chroma_constraint_flag", 1, max_422chroma_constraint_flag, "")

							BST_FIELD_PROP_NUMBER(sub_layer ? "sub_layer_max_420chroma_constraint_flag" : "general_max_420chroma_constraint_flag", 1, max_420chroma_constraint_flag, "")
							BST_FIELD_PROP_NUMBER(sub_layer ? "sub_layer_max_monochrome_constraint_flag" : "general_max_monochrome_constraint_flag", 1, max_monochrome_constraint_flag, "")
							BST_FIELD_PROP_NUMBER(sub_layer ? "sub_layer_intra_constraint_flag" : "general_intra_constraint_flag", 1, intra_constraint_flag, "")
							BST_FIELD_PROP_NUMBER(sub_layer ? "sub_layer_one_picture_only_constraint_flag" : "general_one_picture_only_constraint_flag", 1, one_picture_only_constraint_flag, "")
							BST_FIELD_PROP_NUMBER(sub_layer ? "sub_layer_lower_bit_rate_constraint_flag" : "general_lower_bit_rate_constraint_flag", 1, lower_bit_rate_constraint_flag, "")
							if (profile_idc == 5 || profile_compatibility_flag[5] ||
								profile_idc == 9 || profile_compatibility_flag[9] ||
								profile_idc == 10 || profile_compatibility_flag[10] ||
								profile_idc == 11 || profile_compatibility_flag[11])
							{
								BST_FIELD_PROP_NUMBER(sub_layer ? "sub_layer_max_14bit_constraint_flag" : "general_max_14bit_constraint_flag", 1, max_14bit_constraint_flag, "")
								BST_FIELD_PROP_2NUMBER(sub_layer ? "sub_layer_reserved_zero_34bits" : "general_reserved_zero_33bits", 33, reserved_zero_33bits, "")
							}
							else
							{
								uint64_t reserved_zero_34bits = ((uint64_t)max_14bit_constraint_flag << 33) | reserved_zero_33bits;
								BST_FIELD_PROP_2NUMBER(sub_layer ? "sub_layer_reserved_zero_34bits" : "general_reserved_zero_34bits", 34, reserved_zero_34bits, "")
							}
						}
						else
						{
							BST_FIELD_PROP_2NUMBER(sub_layer ? "sub_layer_reserved_zero_43bits" : "general_reserved_zero_43bits", 43,
								(((uint64_t)max_12bit_constraint_flag << 42) |
								 ((uint64_t)max_10bit_constraint_flag << 41) |
								 ((uint64_t)max_8bit_constraint_flag << 40) |
								 ((uint64_t)max_422chroma_constraint_flag << 39) |
								 ((uint64_t)max_420chroma_constraint_flag << 38) |
								 ((uint64_t)max_monochrome_constraint_flag << 37) |
								 ((uint64_t)intra_constraint_flag << 36) |
								 ((uint64_t)one_picture_only_constraint_flag << 35) |
								 ((uint64_t)lower_bit_rate_constraint_flag << 34) |
								 ((uint64_t)max_14bit_constraint_flag << 33) |
								reserved_zero_33bits), "")
						}

						if ((profile_idc >= 1 && profile_idc <= 5) ||
							profile_compatibility_flag[1] || profile_compatibility_flag[2] ||
							profile_compatibility_flag[3] || profile_compatibility_flag[4] ||
							profile_compatibility_flag[5]) {
							/* The number of bits in this syntax structure is not affected by this condition */
							BST_FIELD_PROP_NUMBER(sub_layer ? "sub_layer_inbld_flag" : "general_inbld_flag", 1, inbld_flag, "")
						}
						else {
							BST_FIELD_PROP_NUMBER(sub_layer ? "sub_layer_reserved_zero_bit" : "general_reserved_zero_bit", 1, inbld_flag, "")
						}
					}

					if (level_present_flag) {
						MBCSPRINTF_S(szTemp4, TEMP4_SIZE, "Level %d.%d", (int)(level_idc / 30), (int)(level_idc % 30 / 3));
						BST_FIELD_PROP_2NUMBER(sub_layer ? "sub_layer_level_idc" : "general_level_idc", 8, level_idc, szTemp4)
					}
					DECLARE_FIELDPROP_END()
				};

				int				max_num_sub_layers;

				PROFILE_LEVEL	general_profile_level;

				CAMBitArray		sub_layer_profile_present_flag;
				CAMBitArray		sub_layer_level_present_flag;
				PROFILE_LEVEL	sub_layer_profile_level[8];


				PROFILE_TIER_LEVEL(bool profilePresentFlag, int maxNumSubLayersMinus1) : max_num_sub_layers(maxNumSubLayersMinus1 + 1) {
					general_profile_level.profile_present_flag = profilePresentFlag ? 1 : 0;
					general_profile_level.level_present_flag = 1;
					general_profile_level.sub_layer = 0;
				}

				HEVC_PROFILE GetHEVCProfile() {
					if (general_profile_level.profile_idc == 3 || general_profile_level.profile_compatibility_flag[3])
						return HEVC_PROFILE_MAIN_STILL_PICTURE;
					else if (general_profile_level.profile_idc == 1 || general_profile_level.profile_compatibility_flag[1])
						return HEVC_PROFILE_MAIN;
					else if (general_profile_level.profile_idc == 2 || general_profile_level.profile_compatibility_flag[2])
						return general_profile_level.one_picture_only_constraint_flag ? HEVC_PROFILE_MAIN_10_STILL_PICTURE : HEVC_PROFILE_MAIN_10;
					else if (general_profile_level.profile_idc == 4 || general_profile_level.profile_compatibility_flag[4])
					{
						if (general_profile_level.max_12bit_constraint_flag && general_profile_level.max_10bit_constraint_flag && general_profile_level.max_8bit_constraint_flag &&
							general_profile_level.max_422chroma_constraint_flag && general_profile_level.max_420chroma_constraint_flag && general_profile_level.max_monochrome_constraint_flag &&
							!general_profile_level.intra_constraint_flag && !general_profile_level.one_picture_only_constraint_flag && general_profile_level.lower_bit_rate_constraint_flag)
							return HEVC_PROFILE_MONOCHROME;
						if (general_profile_level.max_12bit_constraint_flag && general_profile_level.max_10bit_constraint_flag && !general_profile_level.max_8bit_constraint_flag &&
							general_profile_level.max_422chroma_constraint_flag && general_profile_level.max_420chroma_constraint_flag && general_profile_level.max_monochrome_constraint_flag &&
							!general_profile_level.intra_constraint_flag && !general_profile_level.one_picture_only_constraint_flag && general_profile_level.lower_bit_rate_constraint_flag)
							return HEVC_PROFILE_MONOCHROME_10;
						else if (general_profile_level.max_12bit_constraint_flag && !general_profile_level.max_10bit_constraint_flag && !general_profile_level.max_8bit_constraint_flag &&
							general_profile_level.max_422chroma_constraint_flag && general_profile_level.max_420chroma_constraint_flag && general_profile_level.max_monochrome_constraint_flag &&
							!general_profile_level.intra_constraint_flag && !general_profile_level.one_picture_only_constraint_flag && general_profile_level.lower_bit_rate_constraint_flag)
							return HEVC_PROFILE_MONOCHROME_12;
						else if (!general_profile_level.max_12bit_constraint_flag && !general_profile_level.max_10bit_constraint_flag && !general_profile_level.max_8bit_constraint_flag &&
							general_profile_level.max_422chroma_constraint_flag && general_profile_level.max_420chroma_constraint_flag && general_profile_level.max_monochrome_constraint_flag &&
							!general_profile_level.intra_constraint_flag && !general_profile_level.one_picture_only_constraint_flag && general_profile_level.lower_bit_rate_constraint_flag)
							return HEVC_PROFILE_MONOCHROME_16;
						else if (general_profile_level.max_12bit_constraint_flag && !general_profile_level.max_10bit_constraint_flag && !general_profile_level.max_8bit_constraint_flag &&
							general_profile_level.max_422chroma_constraint_flag && general_profile_level.max_420chroma_constraint_flag && !general_profile_level.max_monochrome_constraint_flag &&
							!general_profile_level.intra_constraint_flag && !general_profile_level.one_picture_only_constraint_flag && general_profile_level.lower_bit_rate_constraint_flag)
							return HEVC_PROFILE_MAIN_12;
						else if (general_profile_level.max_12bit_constraint_flag && general_profile_level.max_10bit_constraint_flag && !general_profile_level.max_8bit_constraint_flag &&
							general_profile_level.max_422chroma_constraint_flag && !general_profile_level.max_420chroma_constraint_flag && !general_profile_level.max_monochrome_constraint_flag &&
							!general_profile_level.intra_constraint_flag && !general_profile_level.one_picture_only_constraint_flag && general_profile_level.lower_bit_rate_constraint_flag)
							return HEVC_PROFILE_MAIN_422_10;
						else if (general_profile_level.max_12bit_constraint_flag && !general_profile_level.max_10bit_constraint_flag && !general_profile_level.max_8bit_constraint_flag &&
							general_profile_level.max_422chroma_constraint_flag && !general_profile_level.max_420chroma_constraint_flag && !general_profile_level.max_monochrome_constraint_flag &&
							!general_profile_level.intra_constraint_flag && !general_profile_level.one_picture_only_constraint_flag && general_profile_level.lower_bit_rate_constraint_flag)
							return HEVC_PROFILE_MAIN_422_12;
						else if (general_profile_level.max_12bit_constraint_flag && general_profile_level.max_10bit_constraint_flag && general_profile_level.max_8bit_constraint_flag &&
							!general_profile_level.max_422chroma_constraint_flag && !general_profile_level.max_420chroma_constraint_flag && !general_profile_level.max_monochrome_constraint_flag &&
							!general_profile_level.intra_constraint_flag && !general_profile_level.one_picture_only_constraint_flag && general_profile_level.lower_bit_rate_constraint_flag)
							return HEVC_PROFILE_MAIN_444;
						else if (general_profile_level.max_12bit_constraint_flag && general_profile_level.max_10bit_constraint_flag && !general_profile_level.max_8bit_constraint_flag &&
							!general_profile_level.max_422chroma_constraint_flag && !general_profile_level.max_420chroma_constraint_flag && !general_profile_level.max_monochrome_constraint_flag &&
							!general_profile_level.intra_constraint_flag && !general_profile_level.one_picture_only_constraint_flag && general_profile_level.lower_bit_rate_constraint_flag)
							return HEVC_PROFILE_MAIN_444_10;
						else if (general_profile_level.max_12bit_constraint_flag && !general_profile_level.max_10bit_constraint_flag && !general_profile_level.max_8bit_constraint_flag &&
							!general_profile_level.max_422chroma_constraint_flag && !general_profile_level.max_420chroma_constraint_flag && !general_profile_level.max_monochrome_constraint_flag &&
							!general_profile_level.intra_constraint_flag && !general_profile_level.one_picture_only_constraint_flag && general_profile_level.lower_bit_rate_constraint_flag)
							return HEVC_PROFILE_MAIN_444_12;
						else if (general_profile_level.max_12bit_constraint_flag && general_profile_level.max_10bit_constraint_flag && general_profile_level.max_8bit_constraint_flag &&
							general_profile_level.max_422chroma_constraint_flag && general_profile_level.max_420chroma_constraint_flag && !general_profile_level.max_monochrome_constraint_flag &&
							general_profile_level.intra_constraint_flag && !general_profile_level.one_picture_only_constraint_flag)
							return HEVC_PROFILE_MAIN_INTRA;
						else if (general_profile_level.max_12bit_constraint_flag && general_profile_level.max_10bit_constraint_flag && !general_profile_level.max_8bit_constraint_flag &&
							general_profile_level.max_422chroma_constraint_flag && general_profile_level.max_420chroma_constraint_flag && !general_profile_level.max_monochrome_constraint_flag &&
							general_profile_level.intra_constraint_flag && !general_profile_level.one_picture_only_constraint_flag)
							return HEVC_PROFILE_MAIN_10_INTRA;
						else if (general_profile_level.max_12bit_constraint_flag && !general_profile_level.max_10bit_constraint_flag && !general_profile_level.max_8bit_constraint_flag &&
							general_profile_level.max_422chroma_constraint_flag && general_profile_level.max_420chroma_constraint_flag && !general_profile_level.max_monochrome_constraint_flag &&
							general_profile_level.intra_constraint_flag && !general_profile_level.one_picture_only_constraint_flag)
							return HEVC_PROFILE_MAIN_12_INTRA;
						else if (general_profile_level.max_12bit_constraint_flag && general_profile_level.max_10bit_constraint_flag && !general_profile_level.max_8bit_constraint_flag &&
							general_profile_level.max_422chroma_constraint_flag && !general_profile_level.max_420chroma_constraint_flag && !general_profile_level.max_monochrome_constraint_flag &&
							general_profile_level.intra_constraint_flag && !general_profile_level.one_picture_only_constraint_flag)
							return HEVC_PROFILE_MAIN_422_10_INTRA;
						else if (general_profile_level.max_12bit_constraint_flag && !general_profile_level.max_10bit_constraint_flag && !general_profile_level.max_8bit_constraint_flag &&
							general_profile_level.max_422chroma_constraint_flag && !general_profile_level.max_420chroma_constraint_flag && !general_profile_level.max_monochrome_constraint_flag &&
							general_profile_level.intra_constraint_flag && !general_profile_level.one_picture_only_constraint_flag)
							return HEVC_PROFILE_MAIN_422_12_INTRA;
						else if (general_profile_level.max_12bit_constraint_flag && general_profile_level.max_10bit_constraint_flag && general_profile_level.max_8bit_constraint_flag &&
							!general_profile_level.max_422chroma_constraint_flag && !general_profile_level.max_420chroma_constraint_flag && !general_profile_level.max_monochrome_constraint_flag &&
							general_profile_level.intra_constraint_flag && !general_profile_level.one_picture_only_constraint_flag)
							return HEVC_PROFILE_MAIN_444_INTRA;
						else if (general_profile_level.max_12bit_constraint_flag && general_profile_level.max_10bit_constraint_flag && !general_profile_level.max_8bit_constraint_flag &&
							!general_profile_level.max_422chroma_constraint_flag && !general_profile_level.max_420chroma_constraint_flag && !general_profile_level.max_monochrome_constraint_flag &&
							general_profile_level.intra_constraint_flag && !general_profile_level.one_picture_only_constraint_flag)
							return HEVC_PROFILE_MAIN_444_10_INTRA;
						else if (general_profile_level.max_12bit_constraint_flag && !general_profile_level.max_10bit_constraint_flag && !general_profile_level.max_8bit_constraint_flag &&
							!general_profile_level.max_422chroma_constraint_flag && !general_profile_level.max_420chroma_constraint_flag && !general_profile_level.max_monochrome_constraint_flag &&
							general_profile_level.intra_constraint_flag && !general_profile_level.one_picture_only_constraint_flag)
							return HEVC_PROFILE_MAIN_444_12_INTRA;
						else if (!general_profile_level.max_12bit_constraint_flag && !general_profile_level.max_10bit_constraint_flag && !general_profile_level.max_8bit_constraint_flag &&
							!general_profile_level.max_422chroma_constraint_flag && !general_profile_level.max_420chroma_constraint_flag && !general_profile_level.max_monochrome_constraint_flag &&
							general_profile_level.intra_constraint_flag && !general_profile_level.one_picture_only_constraint_flag)
							return HEVC_PROFILE_MAIN_444_16_INTRA;
						else if (general_profile_level.max_12bit_constraint_flag && general_profile_level.max_10bit_constraint_flag && general_profile_level.max_8bit_constraint_flag &&
							!general_profile_level.max_422chroma_constraint_flag && !general_profile_level.max_420chroma_constraint_flag && !general_profile_level.max_monochrome_constraint_flag &&
							general_profile_level.intra_constraint_flag && general_profile_level.one_picture_only_constraint_flag)
							return HEVC_PROFILE_MAIN_444_STILL_PICTURE;
						else if (!general_profile_level.max_12bit_constraint_flag && !general_profile_level.max_10bit_constraint_flag && !general_profile_level.max_8bit_constraint_flag &&
							!general_profile_level.max_422chroma_constraint_flag && !general_profile_level.max_420chroma_constraint_flag && !general_profile_level.max_monochrome_constraint_flag &&
							general_profile_level.intra_constraint_flag && general_profile_level.one_picture_only_constraint_flag)
							return HEVC_PROFILE_MAIN_444_16_STILL_PICTURE;
					}
					else if (general_profile_level.profile_idc == 5 || general_profile_level.profile_compatibility_flag[5])
					{
						if (general_profile_level.max_14bit_constraint_flag && general_profile_level.max_12bit_constraint_flag && general_profile_level.max_10bit_constraint_flag &&
							general_profile_level.max_8bit_constraint_flag && !general_profile_level.max_422chroma_constraint_flag && !general_profile_level.max_420chroma_constraint_flag &&
							!general_profile_level.max_monochrome_constraint_flag && !general_profile_level.intra_constraint_flag && !general_profile_level.one_picture_only_constraint_flag &&
							general_profile_level.lower_bit_rate_constraint_flag)
							return HEVC_PROFILE_HIGH_THROUGHPUT_444;
						else if (general_profile_level.max_14bit_constraint_flag && general_profile_level.max_12bit_constraint_flag && general_profile_level.max_10bit_constraint_flag &&
							!general_profile_level.max_8bit_constraint_flag && !general_profile_level.max_422chroma_constraint_flag && !general_profile_level.max_420chroma_constraint_flag &&
							!general_profile_level.max_monochrome_constraint_flag && !general_profile_level.intra_constraint_flag && !general_profile_level.one_picture_only_constraint_flag &&
							general_profile_level.lower_bit_rate_constraint_flag)
							return HEVC_PROFILE_HIGH_THROUGHPUT_444_10;
						else if (general_profile_level.max_14bit_constraint_flag && !general_profile_level.max_12bit_constraint_flag && !general_profile_level.max_10bit_constraint_flag &&
							!general_profile_level.max_8bit_constraint_flag && !general_profile_level.max_422chroma_constraint_flag && !general_profile_level.max_420chroma_constraint_flag &&
							!general_profile_level.max_monochrome_constraint_flag && !general_profile_level.intra_constraint_flag && !general_profile_level.one_picture_only_constraint_flag &&
							general_profile_level.lower_bit_rate_constraint_flag)
							return HEVC_PROFILE_HIGH_THROUGHPUT_444_14;
						else if (!general_profile_level.max_14bit_constraint_flag && !general_profile_level.max_12bit_constraint_flag && !general_profile_level.max_10bit_constraint_flag &&
							!general_profile_level.max_8bit_constraint_flag && !general_profile_level.max_422chroma_constraint_flag && !general_profile_level.max_420chroma_constraint_flag &&
							!general_profile_level.max_monochrome_constraint_flag && general_profile_level.intra_constraint_flag && !general_profile_level.one_picture_only_constraint_flag /*&&
							general_profile_level.lower_bit_rate_constraint_flag*/)
							return HEVC_PROFILE_HIGH_THROUGHPUT_444_16_INTRA;
					}
					else if (general_profile_level.profile_idc == 9 || general_profile_level.profile_compatibility_flag[9])
					{
						if (general_profile_level.max_14bit_constraint_flag && general_profile_level.max_12bit_constraint_flag && general_profile_level.max_10bit_constraint_flag &&
							general_profile_level.max_8bit_constraint_flag && !general_profile_level.max_422chroma_constraint_flag && general_profile_level.max_420chroma_constraint_flag &&
							!general_profile_level.max_monochrome_constraint_flag && !general_profile_level.intra_constraint_flag && !general_profile_level.one_picture_only_constraint_flag &&
							general_profile_level.lower_bit_rate_constraint_flag)
							return HEVC_PROFILE_SCREEN_EXTENDED_MAIN;
						else if (general_profile_level.max_14bit_constraint_flag && general_profile_level.max_12bit_constraint_flag && general_profile_level.max_10bit_constraint_flag &&
							!general_profile_level.max_8bit_constraint_flag && !general_profile_level.max_422chroma_constraint_flag && general_profile_level.max_420chroma_constraint_flag &&
							!general_profile_level.max_monochrome_constraint_flag && !general_profile_level.intra_constraint_flag && !general_profile_level.one_picture_only_constraint_flag &&
							general_profile_level.lower_bit_rate_constraint_flag)
							return HEVC_PROFILE_SCREEN_EXTENDED_MAIN_10;
						else if (general_profile_level.max_14bit_constraint_flag && general_profile_level.max_12bit_constraint_flag && general_profile_level.max_10bit_constraint_flag &&
							general_profile_level.max_8bit_constraint_flag && !general_profile_level.max_422chroma_constraint_flag && !general_profile_level.max_420chroma_constraint_flag &&
							!general_profile_level.max_monochrome_constraint_flag && !general_profile_level.intra_constraint_flag && !general_profile_level.one_picture_only_constraint_flag &&
							general_profile_level.lower_bit_rate_constraint_flag)
							return HEVC_PROFILE_SCREEN_EXTENDED_MAIN_444;
						else if (general_profile_level.max_14bit_constraint_flag && general_profile_level.max_12bit_constraint_flag && general_profile_level.max_10bit_constraint_flag &&
							!general_profile_level.max_8bit_constraint_flag && !general_profile_level.max_422chroma_constraint_flag && !general_profile_level.max_420chroma_constraint_flag &&
							!general_profile_level.max_monochrome_constraint_flag && !general_profile_level.intra_constraint_flag && !general_profile_level.one_picture_only_constraint_flag &&
							general_profile_level.lower_bit_rate_constraint_flag)
							return HEVC_PROFILE_SCREEN_EXTENDED_MAIN_444_10;
					}
					else if (general_profile_level.profile_idc == 11 || general_profile_level.profile_compatibility_flag[11])
					{
						if (general_profile_level.max_14bit_constraint_flag && general_profile_level.max_12bit_constraint_flag && general_profile_level.max_10bit_constraint_flag &&
							general_profile_level.max_8bit_constraint_flag && !general_profile_level.max_422chroma_constraint_flag && !general_profile_level.max_420chroma_constraint_flag &&
							!general_profile_level.max_monochrome_constraint_flag && !general_profile_level.intra_constraint_flag && !general_profile_level.one_picture_only_constraint_flag &&
							general_profile_level.lower_bit_rate_constraint_flag)
							return HEVC_PROFILE_SCREEN_EXTENDED_HIGH_THROUGHPUT_444;
						else if (general_profile_level.max_14bit_constraint_flag && general_profile_level.max_12bit_constraint_flag && general_profile_level.max_10bit_constraint_flag &&
							!general_profile_level.max_8bit_constraint_flag && !general_profile_level.max_422chroma_constraint_flag && !general_profile_level.max_420chroma_constraint_flag &&
							!general_profile_level.max_monochrome_constraint_flag && !general_profile_level.intra_constraint_flag && !general_profile_level.one_picture_only_constraint_flag &&
							general_profile_level.lower_bit_rate_constraint_flag)
							return HEVC_PROFILE_SCREEN_EXTENDED_HIGH_THROUGHPUT_444_10;
						else if (general_profile_level.max_14bit_constraint_flag && !general_profile_level.max_12bit_constraint_flag && !general_profile_level.max_10bit_constraint_flag &&
							!general_profile_level.max_8bit_constraint_flag && !general_profile_level.max_422chroma_constraint_flag && !general_profile_level.max_420chroma_constraint_flag &&
							!general_profile_level.max_monochrome_constraint_flag && !general_profile_level.intra_constraint_flag && !general_profile_level.one_picture_only_constraint_flag &&
							general_profile_level.lower_bit_rate_constraint_flag)
							return HEVC_PROFILE_SCREEN_EXTENDED_HIGH_THROUGHPUT_444_14;
					}

					return HEVC_PROFILE_UNKNOWN;
				}

				int Map(AMBst in_bst)
				{
					int iRet = RET_CODE_SUCCESS;

					SYNTAX_BITSTREAM_MAP::Map(in_bst);

					MAP_BST_BEGIN(0);
					if (AMP_FAILED(iRet = general_profile_level.Map(in_bst)))
						printf("[HEVC] Failed to map general_profile_level. {retcode: %d}.\n", iRet);

					if (iRet == RET_CODE_NO_MORE_DATA) goto done;

					try
					{
						for (int i = 0; i < 8 && max_num_sub_layers > 1; i++)
						{
							nal_read_bitarray(in_bst, sub_layer_profile_present_flag, i);
							sub_layer_profile_level[i].profile_present_flag = sub_layer_profile_present_flag[i];
							nal_read_bitarray(in_bst, sub_layer_level_present_flag, i);
							sub_layer_profile_level[i].level_present_flag = sub_layer_level_present_flag[i];
						}
					}
					catch (AMException e)
					{
						return e.RetCode();
					}

					for (int i = 0; i < max_num_sub_layers - 1; i++) {
						if (AMP_FAILED(iRet = sub_layer_profile_level[i].Map(in_bst)))
							printf("[HEVC] Failed to map sub_layer_profile_level#%d. (retcode: %d).\n", i, iRet);

						if (iRet == RET_CODE_NO_MORE_DATA) goto done;
					}

					MAP_BST_END();
				done:
					return iRet;
				}

				int Unmap(AMBst out_bst)
				{
					UNREFERENCED_PARAMETER(out_bst);
					return RET_CODE_ERROR_NOTIMPL;
				}

				DECLARE_FIELDPROP_BEGIN()
					NAV_FIELD_PROP_OBJECT(general_profile_level)
					for (i = 0; i < 8 && max_num_sub_layers > 1; i++) {
						if (i < max_num_sub_layers - 1)
						{
							NAV_ARRAY_FIELD_PROP_NUMBER("sub_layer_profile_present_flag", i, 1, sub_layer_profile_present_flag[i], "")
								NAV_ARRAY_FIELD_PROP_NUMBER("sub_layer_level_present_flag", i, 1, sub_layer_level_present_flag[i], "")
						}
						else
						{
							int reserved_zero_2bits = (sub_layer_profile_present_flag[i] ? 2 : 0) |
								(sub_layer_level_present_flag[i] ? 1 : 0);
							NAV_ARRAY_FIELD_PROP_NUMBER("reserved_zero_2bits", i, 2, reserved_zero_2bits, "")
						}
					}
					for (i = 0; i < max_num_sub_layers - 1; i++)
					{
						NAV_FIELD_PROP_OBJECT(sub_layer_profile_level[i]);
					}
				DECLARE_FIELDPROP_END()
			};

			struct SCALING_LIST_DATA : public SYNTAX_BITSTREAM_MAP
			{
				CAMBitArray		scaling_list_pred_mode_flags;
				uint8_t			scaling_list_pred_matrix_id_delta[4][6] = { {0} };
				uint8_t			scaling_list_dc_coef[2][6] = { {0} };
				int8_t			scaling_list_delta_coef[4][6][64] = { {{0}} };

				int Map(AMBst in_bst)
				{
					int iRet = RET_CODE_SUCCESS;
					SYNTAX_BITSTREAM_MAP::Map(in_bst);

					try
					{
						MAP_BST_BEGIN(0);
						for (int sizeId = 0; sizeId < 4; sizeId++)
						{
							for (int matrixId = 0; matrixId < 6; matrixId += (sizeId == 3) ? 3 : 1)
							{
								int idx = sizeId * 6 + matrixId;
								nal_read_bitarray(in_bst, scaling_list_pred_mode_flags, idx);

								if (!scaling_list_pred_mode_flags[idx])
								{
									nal_read_ue(in_bst, scaling_list_pred_matrix_id_delta[sizeId][matrixId], uint8_t);
								}
								else
								{
									if (sizeId > 1) {
										int16_t iTmp;
										nal_read_se(in_bst, iTmp, int16_t);
										if (iTmp < -7 || iTmp > 247)
											printf("[H265] scaling_list_dc_coef_minus8(%d) should be in the range -7 to 247, inclusive.\n", iTmp);
										scaling_list_dc_coef[sizeId - 2][matrixId] = (uint8_t)(iTmp + 8);
									}

									int coefNum = AMP_MIN(64, (1 << (4 + (sizeId << 1))));
									for (int i = 0; i < coefNum; i++) {
										nal_read_se(in_bst, scaling_list_delta_coef[sizeId][matrixId][i], int8_t);
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

				int Unmap(AMBst out_bst)
				{
					UNREFERENCED_PARAMETER(out_bst);
					return RET_CODE_ERROR_NOTIMPL;
				}

				DECLARE_FIELDPROP_BEGIN()
					NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Node0", "for(sizeId=0;sizeId&lt;4;sizeId++)", "");
					NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Node1", "for(matrixId=0;matrixId&lt;6;matrixId+=(sizeId==3)?3:1)", "");
						for (int sizeId = 0; sizeId < 4; sizeId++)
						{
							for (int matrixId = 0; matrixId < 6; matrixId += (sizeId == 3) ? 3 : 1)
							{
								int idx = sizeId * 6 + matrixId;
								BST_2ARRAY_FIELD_PROP_NUMBER("scaling_list_pred_mode_flag", sizeId, matrixId, 1, scaling_list_pred_mode_flags[idx], "");
								if (!scaling_list_pred_mode_flags[idx])
								{
									BST_2ARRAY_FIELD_PROP_NUMBER("scaling_list_pred_matrix_id_delta", sizeId, matrixId, (long long)quick_log2(scaling_list_pred_matrix_id_delta[sizeId][matrixId] + 1) * 2 + 1, scaling_list_pred_matrix_id_delta[sizeId][matrixId], "");
								}
								else
								{
									int nextCoef = 8;
									int coefNum = AMP_MIN(64, (1 << (4 + (sizeId << 1))));
									NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag0", "nextCoef = 8", 8, "");
									NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag1", "coefNum = Min(64, (1 &lt;&lt; (4 + (sizeId &lt;&lt; 1))))", coefNum, "");
									NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag2", "if(sizeId&gt;1)", "");
									if (sizeId > 1)
									{
										int16_t scaling_list_dc_coef_minus8 = scaling_list_dc_coef[sizeId - 2][matrixId] - 8;
										BST_2ARRAY_FIELD_PROP_NUMBER("scaling_list_dc_coef_minus8", sizeId, matrixId, (long long)quick_log2((scaling_list_dc_coef_minus8 >= 0 ? scaling_list_dc_coef_minus8 : (-scaling_list_dc_coef_minus8 + 1)) + 1) * 2 + 1, scaling_list_dc_coef_minus8, "")
										NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag20", "nextCoef = scaling_list_dc_coef_minus8[sizeId-2][matrixId] + 8", scaling_list_dc_coef[sizeId - 2][matrixId], "");
									}
									NAV_WRITE_TAG_END("Tag2");
								
									NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag3", "for(i = 0; i &lt; coefNum; i++)", "");
										for (i = 0; i < coefNum; i++) {
											unsigned long field_bits = scaling_list_delta_coef[sizeId][matrixId][i] >= 0 ? scaling_list_delta_coef[sizeId][matrixId][i] : (-scaling_list_delta_coef[sizeId][matrixId][i]) + 1;
											BST_3ARRAY_FIELD_PROP_SIGN_NUMBER("scaling_list_delta_coef", sizeId, matrixId, i, (long long)quick_log2(field_bits + 1) * 2 + 1, scaling_list_delta_coef[sizeId][matrixId][i], "")
											nextCoef = (nextCoef + scaling_list_delta_coef[sizeId][matrixId][i] + 256) % 256;
											NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag30", "nextCoef[%d][%d][%d] = (nextCoef + scaling_list_delta_coef[%d][%d][%d] + 256)%%256", nextCoef, "", sizeId, matrixId, i, sizeId, matrixId, i);
											NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag31", "nScalingList[%d][%d][%d] = nextCoef[%d][%d][%d]", nextCoef, "", sizeId, matrixId, i, sizeId, matrixId, i);
										}
									NAV_WRITE_TAG_END("Tag3");
								}
							}
						}
					NAV_WRITE_TAG_END("Node1");
					NAV_WRITE_TAG_END("Node0");
				DECLARE_FIELDPROP_END()
			};

			struct ST_REF_PIC_SETS : public SYNTAX_BITSTREAM_MAP
			{
				struct ST_REF_PIC_SET : public SYNTAX_BITSTREAM_MAP
				{
					ST_REF_PIC_SETS*
								st_ref_pic_sets;
					uint8_t		stRpsIdx;

					uint8_t		inter_ref_pic_set_prediction_flag : 1;
					uint8_t		delta_rps_sign : 1;
					uint8_t		reserved_0 : 6;

					uint8_t		delta_idx_minus1 = 0;
					uint16_t	abs_delta_rps_minus1 = 0;

					CAMBitArray	used_by_curr_pic_flags;
					CAMBitArray	use_delta_flags;

					uint8_t		NumNegativePics = 0;
					uint8_t		NumPositivePics = 0;

					uint16_t	delta_poc_s0_minus1[16] = { 0 };
					uint16_t	delta_poc_s1_minus1[16] = { 0 };
					CAMBitArray	used_by_curr_pic_s0_flags;
					CAMBitArray	used_by_curr_pic_s1_flags;

					ST_REF_PIC_SET(ST_REF_PIC_SETS* ptr_st_ref_pic_sets, uint8_t param_stRpsIdx)
						: st_ref_pic_sets(ptr_st_ref_pic_sets)
						, stRpsIdx(param_stRpsIdx)
						, inter_ref_pic_set_prediction_flag(0)
						, delta_rps_sign(0)
						, reserved_0(0)
						, delta_idx_minus1(0) {
						inter_ref_pic_set_prediction_flag = (uint8_t)0;
					}

					virtual ~ST_REF_PIC_SET()
					{
					}

					int32_t GetDeltaPocS(uint8_t i, uint8_t RefRpsIdx, bool bNegative) {
						if (i == 0)
							return bNegative?-(st_ref_pic_sets->st_ref_pic_set[RefRpsIdx]->delta_poc_s0_minus1[i] + 1):(st_ref_pic_sets->st_ref_pic_set[RefRpsIdx]->delta_poc_s1_minus1[i] + 1);

						return GetDeltaPocS(i - 1, RefRpsIdx, bNegative) + (bNegative?-(st_ref_pic_sets->st_ref_pic_set[RefRpsIdx]->delta_poc_s0_minus1[i] + 1): (st_ref_pic_sets->st_ref_pic_set[RefRpsIdx]->delta_poc_s1_minus1[i] + 1));
					}

					void SetDeltaPocS(uint8_t i, bool bNegative, int32_t dPoc) {
						if (i == 0)
						{
							if (bNegative)
								delta_poc_s0_minus1[i] = -dPoc - 1;
							else
								delta_poc_s1_minus1[i] = dPoc - 1;

							return;
						}

						if (bNegative)
							delta_poc_s0_minus1[i] = -(dPoc - GetDeltaPocS(i - 1, stRpsIdx, bNegative)) - 1;
						else
							delta_poc_s1_minus1[i] = dPoc - GetDeltaPocS(i - 1, stRpsIdx, bNegative) - 1;
						return;
					}

					int Map(AMBst in_bst)
					{
						int iRet = RET_CODE_SUCCESS;
						SYNTAX_BITSTREAM_MAP::Map(in_bst);

						try
						{
							MAP_BST_BEGIN(0);
							if (stRpsIdx != 0) {
								nal_read_u(in_bst, inter_ref_pic_set_prediction_flag, 1, uint8_t);
							}

							if (inter_ref_pic_set_prediction_flag)
							{
								if (stRpsIdx == st_ref_pic_sets->num_short_term_ref_pic_sets) {
									nal_read_ue(in_bst, delta_idx_minus1, uint8_t);
								}

								nal_read_u(in_bst, delta_rps_sign, 1, uint8_t);
								nal_read_ue(in_bst, abs_delta_rps_minus1, uint16_t);

								uint8_t RefRpsIdx = stRpsIdx - (delta_idx_minus1 + 1);
								uint8_t RefNumDeltaPocs = st_ref_pic_sets->st_ref_pic_set[RefRpsIdx]->NumNegativePics + st_ref_pic_sets->st_ref_pic_set[RefRpsIdx]->NumPositivePics;
								for (int j = 0; j <= RefNumDeltaPocs; j++)
								{
									uint8_t b_used_by_curr_pic_flag;
									nal_read_u(in_bst, b_used_by_curr_pic_flag, 1, uint8_t);
									b_used_by_curr_pic_flag ? used_by_curr_pic_flags.BitSet(j) : used_by_curr_pic_flags.BitClear(j);
									if (!b_used_by_curr_pic_flag) {
										uint8_t b_use_delta_flag;
										nal_read_u(in_bst, b_use_delta_flag, 1, uint8_t);
										b_use_delta_flag ? use_delta_flags.BitSet(j) : use_delta_flags.BitClear(j);
									}
									else
										use_delta_flags.BitSet(j);
								}

								// Calculate the NumNegativePics
								uint8_t i = 0;
								int32_t deltaRps = (1 - 2 * delta_rps_sign) * (abs_delta_rps_minus1 + 1);
								for (int16_t j = st_ref_pic_sets->st_ref_pic_set[RefRpsIdx]->NumPositivePics - 1; j >= 0; j--) {
									int32_t DeltaPocS1 = GetDeltaPocS((uint8_t)j, RefRpsIdx, false);
									int32_t dPoc = DeltaPocS1 + deltaRps;
									if (dPoc < 0 && use_delta_flags[st_ref_pic_sets->st_ref_pic_set[RefRpsIdx]->NumNegativePics + j])
									{
										SetDeltaPocS(i, true, dPoc);
										used_by_curr_pic_flags[st_ref_pic_sets->st_ref_pic_set[RefRpsIdx]->NumNegativePics + j] ? used_by_curr_pic_s0_flags.BitSet(i) : used_by_curr_pic_s0_flags.BitClear(i);
										i++;
									}
								}

								if (deltaRps < 0 && use_delta_flags[RefNumDeltaPocs])
								{
									SetDeltaPocS(i, true, deltaRps);
									used_by_curr_pic_flags[RefNumDeltaPocs] ? used_by_curr_pic_s0_flags.BitSet(i) : used_by_curr_pic_s0_flags.BitClear(i);
									i++;
								}

								for (uint8_t j = 0; j < st_ref_pic_sets->st_ref_pic_set[RefRpsIdx]->NumNegativePics; j++)
								{
									int32_t DeltaPocS0 = GetDeltaPocS(j, RefRpsIdx, true);
									int32_t dPoc = DeltaPocS0 + deltaRps;
									if (dPoc < 0 && use_delta_flags[j])
									{
										SetDeltaPocS(i, true, dPoc);
										used_by_curr_pic_flags[j] ? used_by_curr_pic_s0_flags.BitSet(i) : used_by_curr_pic_s0_flags.BitClear(i);
										i++;
									}
								}

								NumNegativePics = i;

								// Calculate NumPositivePics
								i = 0;
								for (int16_t j = st_ref_pic_sets->st_ref_pic_set[RefRpsIdx]->NumNegativePics - 1; j >= 0; j--)
								{
									int32_t DeltaPocS0 = GetDeltaPocS((uint8_t)j, RefRpsIdx, true);
									int32_t dPoc = DeltaPocS0 + deltaRps;
									if (dPoc > 0 && use_delta_flags[j])
									{
										SetDeltaPocS(i, false, dPoc);
										used_by_curr_pic_flags[j] ? used_by_curr_pic_s1_flags.BitSet(i) : used_by_curr_pic_s1_flags.BitClear(i);
										i++;
									}
								}

								if (deltaRps > 0 && use_delta_flags[RefNumDeltaPocs])
								{
									SetDeltaPocS(i, false, deltaRps);
									used_by_curr_pic_flags[RefNumDeltaPocs] ? used_by_curr_pic_s0_flags.BitSet(i) : used_by_curr_pic_s0_flags.BitClear(i);
									i++;
								}

								for (uint8_t j = 0; j < st_ref_pic_sets->st_ref_pic_set[RefRpsIdx]->NumPositivePics; j++)
								{
									int32_t DeltaPocS1 = GetDeltaPocS(j, RefRpsIdx, false);
									int32_t dPoc = DeltaPocS1 + deltaRps;
									if (dPoc > 0 && use_delta_flags[st_ref_pic_sets->st_ref_pic_set[RefRpsIdx]->NumNegativePics + j])
									{
										SetDeltaPocS(i, false, dPoc);
										used_by_curr_pic_flags[st_ref_pic_sets->st_ref_pic_set[RefRpsIdx]->NumNegativePics + j] ? used_by_curr_pic_s1_flags.BitSet(i) : used_by_curr_pic_s1_flags.BitClear(i);
										i++;
									}
								}

								NumPositivePics = i;
							}
							else
							{
								nal_read_ue(in_bst, NumNegativePics, uint8_t);
								nal_read_ue(in_bst, NumPositivePics, uint8_t);

								if (NumNegativePics + NumPositivePics > 16)
								{
									printf("[H265] NumNegativePics(%d) + NumPositivePics(%d) should be not greater than 16.\n", NumNegativePics, NumPositivePics);
									return RET_CODE_BUFFER_NOT_COMPATIBLE;
								}

								if (NumNegativePics > 0)
								{
									for (uint8_t i = 0; i < NumNegativePics; i++)
									{
										uint8_t b_used_by_curr_pic_s0_flag;
										nal_read_ue(in_bst, delta_poc_s0_minus1[i], uint16_t);
										nal_read_u(in_bst, b_used_by_curr_pic_s0_flag, 1, uint8_t);
										b_used_by_curr_pic_s0_flag ? used_by_curr_pic_s0_flags.BitSet(i) : used_by_curr_pic_s0_flags.BitClear(i);
										use_delta_flags.BitSet(i);
									}
								}

								if (NumPositivePics > 0)
								{
									for (uint8_t i = 0; i < NumPositivePics; i++)
									{
										uint8_t b_used_by_curr_pic_s1_flag;
										nal_read_ue(in_bst, delta_poc_s1_minus1[i], uint16_t);
										nal_read_u(in_bst, b_used_by_curr_pic_s1_flag, 1, uint8_t);
										b_used_by_curr_pic_s1_flag ? used_by_curr_pic_s1_flags.BitSet(i) : used_by_curr_pic_s1_flags.BitClear(i);
										use_delta_flags.BitSet(i + (NumNegativePics>0? NumNegativePics:0));
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
						if (stRpsIdx != 0)
						{
							NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "if(stRpsIdx!=0)", "");
							BST_FIELD_PROP_NUMBER1(inter_ref_pic_set_prediction_flag, 1, "the stRpsIdx-th candidate short-term RPS is predicted from another candidate short-term RPS, which is referred to as the source candidate short-term RPS");
							NAV_WRITE_TAG_END("Tag0");
						}
						if (inter_ref_pic_set_prediction_flag) {
							NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag10", "if(stRpsIdx==num_short_term_ref_pic_sets)", "");
							if (stRpsIdx == st_ref_pic_sets->num_short_term_ref_pic_sets) {
								BST_FIELD_PROP_2NUMBER1(delta_idx_minus1, (long long)quick_log2(delta_idx_minus1 + 1) * 2 + 1, "RefRpsIdx=stRpsIdx-(delta_idx_minus1+1)");
							}
							NAV_WRITE_TAG_END("Tag10");
							uint8_t RefRpsIdx = stRpsIdx - (delta_idx_minus1 + 1);
							uint8_t RefNumDeltaPocs = st_ref_pic_sets->st_ref_pic_set[RefRpsIdx]->NumNegativePics + st_ref_pic_sets->st_ref_pic_set[RefRpsIdx]->NumPositivePics;
							BST_FIELD_PROP_NUMBER1(delta_rps_sign, 1, "deltaRps=(1-2*delta_rps_sign)*(abs_delta_rps_minus1+1)");
							BST_FIELD_PROP_2NUMBER1(abs_delta_rps_minus1, (long long)quick_log2(abs_delta_rps_minus1 + 1) * 2 + 1, "deltaRps=(1-2*delta_rps_sign)*(abs_delta_rps_minus1+1)");
							MBCSPRINTF_S(szTemp2, TEMP2_SIZE, "NumDeltaPocs[%d]=%d", RefRpsIdx, RefNumDeltaPocs);
							NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag11", "for(j=0;j&lt;=NumDeltaPocs[RefRpsIdx];j++)", szTemp2);
							for (int j = 0; j <= RefNumDeltaPocs; j++) {
								BST_ARRAY_FIELD_PROP_NUMBER("used_by_curr_pic_flag", j, 1, used_by_curr_pic_flags[j], "");
								if (!used_by_curr_pic_flags[j]) {
									BST_ARRAY_FIELD_PROP_NUMBER("use_delta_flag", j, 1, use_delta_flags[j], "");
								}
							}
							NAV_WRITE_TAG_END("Tag11");
							NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("num_negative_pics", "num_negative_pics", NumNegativePics, "");
							NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("num_positive_pics", "num_positive_pics", NumPositivePics, "");
							if (NumNegativePics > 0)
							{
								NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag12", "for(i=0;i&lt;num_negative_pics;i++)", "");
								for (uint8_t i = 0; i < NumNegativePics; i++) {
									NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("delta_poc_s0_minus1", "delta_poc_s0_minus1[%d]", delta_poc_s0_minus1[i], "", i);
									NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("used_by_curr_pic_s0_flag", "used_by_curr_pic_s0_flag[%d]", used_by_curr_pic_s0_flags[i], "", i);
								}
								NAV_WRITE_TAG_END("Tag12");
							}

							if (NumPositivePics > 0)
							{
								NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag13", "for(i=0;i&lt;num_positive_pics;i++)", "");
								for (uint8_t i = 0; i < NumPositivePics; i++) {
									NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("delta_poc_s1_minus1", "delta_poc_s1_minus1[%d]", delta_poc_s1_minus1[i], "", i);
									NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("used_by_curr_pic_s1_flag", "used_by_curr_pic_s1_flag[%d]", used_by_curr_pic_s1_flags[i], "", i);
								}
								NAV_WRITE_TAG_END("Tag13");
							}
						}
						else
						{
							BST_FIELD_PROP_2NUMBER("num_negative_pics", (long long)quick_log2(NumNegativePics + 1) * 2 + 1, NumNegativePics, "");
							BST_FIELD_PROP_2NUMBER("num_positive_pics", (long long)quick_log2(NumPositivePics + 1) * 2 + 1, NumPositivePics, "");
							if (NumNegativePics > 0)
							{
								NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag12", "for(i=0;i&lt;num_negative_pics;i++)", "");
								for (uint8_t i = 0; i < NumNegativePics; i++) {
									BST_ARRAY_FIELD_PROP_NUMBER("delta_poc_s0_minus1", i, (long long)quick_log2(delta_poc_s0_minus1[i] + 1) * 2 + 1, delta_poc_s0_minus1[i], "");
									BST_ARRAY_FIELD_PROP_NUMBER("used_by_curr_pic_s0_flag", i, 1, used_by_curr_pic_s0_flags[i], "");
								}
								NAV_WRITE_TAG_END("Tag12");
							}

							if (NumPositivePics > 0)
							{
								NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag13", "for(i=0;i&lt;num_positive_pics;i++)", "");
								for (uint8_t i = 0; i < NumPositivePics; i++) {
									BST_ARRAY_FIELD_PROP_NUMBER("delta_poc_s1_minus1", i, (long long)quick_log2(delta_poc_s1_minus1[i] + 1) * 2 + 1, delta_poc_s1_minus1[i], "");
									BST_ARRAY_FIELD_PROP_NUMBER("used_by_curr_pic_s1_flag", i, 1, used_by_curr_pic_s1_flags[i], "");
								}
								NAV_WRITE_TAG_END("Tag13");
							}
						}
					DECLARE_FIELDPROP_END()

				};

				uint8_t		num_short_term_ref_pic_sets;
				ST_REF_PIC_SET**
							st_ref_pic_set;

				ST_REF_PIC_SETS(uint8_t param_num_short_term_ref_pic_sets) : num_short_term_ref_pic_sets(param_num_short_term_ref_pic_sets) {
					AMP_NEW0(st_ref_pic_set, ST_REF_PIC_SET*, (size_t)num_short_term_ref_pic_sets + 1);
				}

				virtual ~ST_REF_PIC_SETS() {
					if (st_ref_pic_set != NULL)
					{
						for (uint8_t i = 0; i < num_short_term_ref_pic_sets; i++) {
							AMP_SAFEDEL2(st_ref_pic_set[i]);
						}
						AMP_SAFEDELA3(st_ref_pic_set);
					}
				}

				int Map(AMBst in_bst)
				{
					int iRet = RET_CODE_SUCCESS;
					SYNTAX_BITSTREAM_MAP::Map(in_bst);

					try
					{
						MAP_BST_BEGIN(0);
						for (int i = 0; i < num_short_term_ref_pic_sets; i++) {
							nal_read_ref(in_bst, st_ref_pic_set[i], ST_REF_PIC_SET, this, i);
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
					NAV_WRITE_TAG_BEGIN_WITH_ALIAS("TAG0", "for(i=0;i&lt;num_short_term_ref_pic_sets;i++)", "");
					for (i = 0; i < num_short_term_ref_pic_sets; i++)
					{
						NAV_WRITE_TAG_ARRAY_BEGIN0("st_ref_pic_set", i, "");
						BST_FIELD_PROP_REF(st_ref_pic_set[i]);
						NAV_WRITE_TAG_END("st_ref_pic_set");
					}
					NAV_WRITE_TAG_END("TAG0");
				DECLARE_FIELDPROP_END()

			}PACKED;

			struct VIDEO_PARAMETER_SET_RBSP : public SYNTAX_BITSTREAM_MAP
			{
				struct VPS_ORDERING_INFO
				{
					uint16_t	vps_max_dec_pic_buffering_minus1;
					uint16_t	vps_max_num_reorder_pics;
					uint32_t	vps_max_latency_increase_plus1;
				}PACKED;

				struct VPS_HRD_PARAMETER : public SYNTAX_BITSTREAM_MAP
				{
					uint16_t		hrd_layer_set_idx : 15;
					uint16_t		cprms_present_flag : 1;
					HRD_PARAMETERS*	hrd_parameters = nullptr;
					VIDEO_PARAMETER_SET_RBSP*
									ptr_parent;
					uint16_t		m_idx;

					VPS_HRD_PARAMETER(VIDEO_PARAMETER_SET_RBSP* pParent, uint16_t i)
						: hrd_layer_set_idx(0), cprms_present_flag(0), ptr_parent(pParent), m_idx(i){
					}

					virtual ~VPS_HRD_PARAMETER() {
						UNMAP_STRUCT_POINTER5(hrd_parameters);
					}

					int Map(AMBst in_bst)
					{
						int iRet = RET_CODE_SUCCESS;
						SYNTAX_BITSTREAM_MAP::Map(in_bst);

						try
						{
							MAP_BST_BEGIN(0);
							nal_read_ue(in_bst, hrd_layer_set_idx, uint16_t);
							if (m_idx > 0) {
								nal_read_u(in_bst, cprms_present_flag, 1, uint16_t);
							}
							else
								cprms_present_flag = 1;
							nal_read_ref(in_bst, hrd_parameters, HRD_PARAMETERS, cprms_present_flag, ptr_parent->vps_max_sub_layers_minus1);
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
						BST_FIELD_PROP_2NUMBER1(hrd_layer_set_idx, (long long)quick_log2(hrd_layer_set_idx + 1) * 2 + 1, "")
						if (m_idx > 0) {
							BST_FIELD_PROP_NUMBER1(cprms_present_flag, 1, cprms_present_flag?"the HRD parameters that are common for all sub-layers are present":"the HRD parameters that are common for all sub-layers are not present")
						}
						BST_FIELD_PROP_REF(hrd_parameters)
					DECLARE_FIELDPROP_END()

				} PACKED;

				uint16_t	vps_video_parameter_set_id : 4;
				uint16_t	vps_base_layer_internal_flag : 1;
				uint16_t	vps_base_layer_available_flag : 1;
				uint16_t	vps_max_layers_minus1 : 6;
				uint16_t	vps_max_sub_layers_minus1 : 3;
				uint16_t	vps_temporal_id_nesting_flag : 1;

				uint16_t	vps_reserved_0xffff_16bits = 0;

				PROFILE_TIER_LEVEL*
							profile_tier_level;

				uint8_t		vps_sub_layer_ordering_info_present_flag = 0;
				VPS_ORDERING_INFO*
							vps_ordering_info;

				uint8_t		vps_max_layer_id = 0;
				uint16_t	vps_num_layer_sets_minus1 = 0;
				CAMBitArray*
							layer_id_included_flag;
				uint8_t		vps_timing_info_present_flag = 0;

				uint32_t	vps_num_units_in_tick = 0;
				uint32_t	vps_time_scale = 0;
				uint8_t		vps_poc_proportional_to_timing_flag = 0;
				uint32_t	vps_num_ticks_poc_diff_one_minus1 = 0;
				uint16_t	vps_num_hrd_parameters = 0;
				VPS_HRD_PARAMETER**
							vps_hrd_parameters;
				uint8_t		vps_extension_flag = 0;
				RBSP_TRAILING_BITS
							rbsp_trailing_bits;

				VideoBitstreamCtx*
							ptr_ctx_video_bst;

				VIDEO_PARAMETER_SET_RBSP(VideoBitstreamCtx* ctx=NULL)
					: vps_video_parameter_set_id(0)
					, vps_base_layer_internal_flag(0)
					, vps_base_layer_available_flag(0)
					, vps_max_layers_minus1(0)
					, vps_max_sub_layers_minus1(0)
					, vps_temporal_id_nesting_flag(0)
					, profile_tier_level(NULL)
					, vps_ordering_info(NULL)
					, layer_id_included_flag(NULL)
					, vps_hrd_parameters(NULL)
					, ptr_ctx_video_bst(ctx){
				}

				virtual ~VIDEO_PARAMETER_SET_RBSP() {
					if (vps_hrd_parameters != NULL) {
						for (uint16_t i = 0; i < vps_num_hrd_parameters; i++) {
							UNMAP_STRUCT_POINTER5(vps_hrd_parameters[i])
						}

						AMP_SAFEDELA3(vps_hrd_parameters);
					}

					if (vps_num_layer_sets_minus1 >= 1) {
						AMP_SAFEDELA2(layer_id_included_flag);
					}

					AMP_SAFEDELA2(vps_ordering_info);

					UNMAP_STRUCT_POINTER5(profile_tier_level);
				}

				int Map(AMBst bst)
				{
					SYNTAX_BITSTREAM_MAP::Map(bst);
					try
					{
						MAP_BST_BEGIN(0);
						nal_read_u(bst, vps_video_parameter_set_id, 4, uint16_t);

						if (ptr_ctx_video_bst != NULL)
						{
							if (ptr_ctx_video_bst->in_scanning &&
								ptr_ctx_video_bst->prev_vps_video_parameter_set_id == vps_video_parameter_set_id)
							{
								return RET_CODE_NOTHING_TODO;
							}
							else
							{
								ptr_ctx_video_bst->prev_vps_video_parameter_set_id = vps_video_parameter_set_id;
								ptr_ctx_video_bst->sps_seq_parameter_set_id[vps_video_parameter_set_id] = -1;
							}
						}

						nal_read_u(bst, vps_base_layer_internal_flag, 1, uint16_t);
						nal_read_u(bst, vps_base_layer_available_flag, 1, uint16_t);
						nal_read_u(bst, vps_max_layers_minus1, 6, uint16_t);
						nal_read_u(bst, vps_max_sub_layers_minus1, 3, uint16_t);
						nal_read_u(bst, vps_temporal_id_nesting_flag, 1, uint16_t);
						nal_read_u(bst, vps_reserved_0xffff_16bits, 16, uint16_t);

						nal_read_ref(bst, profile_tier_level, PROFILE_TIER_LEVEL, 1, vps_max_sub_layers_minus1);

						nal_read_u(bst, vps_sub_layer_ordering_info_present_flag, 1, uint8_t);

						int number_of_vps_ordering_info = vps_sub_layer_ordering_info_present_flag ? (vps_max_sub_layers_minus1 + 1) : 1;
						AMP_NEW(vps_ordering_info, VPS_ORDERING_INFO, number_of_vps_ordering_info);
						if (vps_ordering_info == nullptr)
							throw AMException(RET_CODE_OUTOFMEMORY);

						for (int i = 0; i < number_of_vps_ordering_info; i++) {
							nal_read_ue(bst, vps_ordering_info[i].vps_max_dec_pic_buffering_minus1, uint16_t);
							nal_read_ue(bst, vps_ordering_info[i].vps_max_num_reorder_pics, uint16_t);
							nal_read_ue(bst, vps_ordering_info[i].vps_max_latency_increase_plus1, uint32_t);
						}
					
						nal_read_u(bst, vps_max_layer_id, 6, uint8_t);
						nal_read_ue(bst, vps_num_layer_sets_minus1, uint16_t);

						if (vps_num_layer_sets_minus1 >= 1)
						{
							layer_id_included_flag = new CAMBitArray[vps_num_layer_sets_minus1 + 1];
							for (int i = 1; i <= vps_num_layer_sets_minus1; i++) {
								for (int j = 0; j <= vps_max_layer_id; j++)
								{
									nal_read_bitarray(bst, layer_id_included_flag[i], j);
								}
							}
						}

						nal_read_u(bst, vps_timing_info_present_flag, 1, uint8_t);
						if (vps_timing_info_present_flag) {
							nal_read_u(bst, vps_num_units_in_tick, 32, uint32_t);
							nal_read_u(bst, vps_time_scale, 32, uint32_t);
							nal_read_u(bst, vps_poc_proportional_to_timing_flag, 1, uint8_t);
							if (vps_poc_proportional_to_timing_flag)
								nal_read_ue(bst, vps_num_ticks_poc_diff_one_minus1, uint8_t);

							nal_read_ue(bst, vps_num_hrd_parameters, uint16_t);
							AMP_NEW(vps_hrd_parameters, VPS_HRD_PARAMETER*, vps_num_hrd_parameters);
							if (vps_hrd_parameters == nullptr)
								throw AMException(RET_CODE_OUTOFMEMORY);

							for (uint16_t i = 0; i < vps_num_hrd_parameters; i++) {
								MAP_MEM_TO_STRUCT_POINTER5(1, vps_hrd_parameters[i], VPS_HRD_PARAMETER, this, i)
							}
						}

						nal_read_u(bst, vps_extension_flag, 1, uint8_t);

						nal_read_obj(bst, rbsp_trailing_bits);

						SYNTAX_BITSTREAM_MAP::EndMap(bst);

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
					BST_FIELD_PROP_2NUMBER1(vps_video_parameter_set_id, 4, "identifies the VPS for reference by other syntax elements")
					BST_FIELD_PROP_NUMBER1(vps_base_layer_internal_flag, 1, "")
					BST_FIELD_PROP_NUMBER1(vps_base_layer_available_flag, 1, "")
					BST_FIELD_PROP_2NUMBER1(vps_max_layers_minus1, 6, "plus 1 specifies the maximum allowed number of layers in each CVS referring to the VPS")
					BST_FIELD_PROP_2NUMBER1(vps_max_sub_layers_minus1, 3, "plus 1 specifies the maximum number of temporal sub-layers that may be present in each CVS referring to the VPS")
					BST_FIELD_PROP_NUMBER1(vps_temporal_id_nesting_flag, 1, "when vps_max_sub_layers_minus1 is greater than 0, specifies whether inter prediction is additionally restricted for CVSs referring to the VPS. When vps_max_sub_layers_minus1 is equal to 0, vps_temporal_id_nesting_flag shall be equal to 1")
					BST_FIELD_PROP_2NUMBER1(vps_reserved_0xffff_16bits, 16, "shall be equal to 0xFFFF")

					const char* szProfileName = get_hevc_profile_name(profile_tier_level->GetHEVCProfile());
					NAV_WRITE_TAG_BEGIN_WITH_ALIAS("profile_tier_level", "profile_tier_level(1, vps_max_sub_layers_minus1)", szProfileName);
					BST_FIELD_PROP_REF(profile_tier_level)
					NAV_WRITE_TAG_END("profile_tier_level");

					BST_FIELD_PROP_NUMBER1(vps_sub_layer_ordering_info_present_flag, 1, vps_sub_layer_ordering_info_present_flag
						? "specifies that vps_max_dec_pic_buffering_minus1[ i ], vps_max_num_reorder_pics[ i ] and vps_max_latency_increase_plus1[ i ] are present for vps_max_sub_layers_minus1 + 1 sub-layers"
						: "specifies that the values of vps_max_dec_pic_buffering_minus1[ vps_max_sub_layers_minus1 ], vps_max_num_reorder_pics[ vps_max_sub_layers_minus1 ] and vps_max_latency_increase_plus1[ vps_max_sub_layers_minus1 ] apply to all sub-layers")

					NAV_WRITE_TAG_BEGIN_WITH_ALIAS("vps_ordering_infos", "for(i = ( vps_sub_layer_ordering_info_present_flag ? 0 : vps_max_sub_layers_minus1 ); i &lt;= vps_max_sub_layers_minus1; i++ ) {", "");
					for (i = (vps_sub_layer_ordering_info_present_flag ? 0 : vps_max_sub_layers_minus1); i <= vps_max_sub_layers_minus1; i++) {
						BST_ARRAY_FIELD_PROP_NUMBER("vps_max_dec_pic_buffering_minus1", i, (long long)quick_log2(vps_ordering_info[i].vps_max_dec_pic_buffering_minus1 + 1) * 2 + 1,
							vps_ordering_info[i].vps_max_dec_pic_buffering_minus1, "plus 1 specifies the maximum required size of the decoded picture buffer for the CVS in units of picture storage buffers")
						BST_ARRAY_FIELD_PROP_NUMBER("vps_max_num_reorder_pics", i, (long long)quick_log2(vps_ordering_info[i].vps_max_num_reorder_pics + 1) * 2 + 1,
							vps_ordering_info[i].vps_max_num_reorder_pics, "indicates the maximum allowed number of pictures with PicOutputFlag equal to 1 that can precede any picture with PicOutputFlag equal to 1 in the CVS in decoding order and follow that picture with PicOutputFlag equal to 1 in output order")
						BST_ARRAY_FIELD_PROP_NUMBER("vps_max_latency_increase_plus1", i, (long long)quick_log2(vps_ordering_info[i].vps_max_latency_increase_plus1 + 1) * 2 + 1,
							vps_ordering_info[i].vps_max_latency_increase_plus1, vps_ordering_info[i].vps_max_latency_increase_plus1?"VpsMaxLatencyPictures[i] = vps_max_num_reorder_pics[i] + vps_max_latency_increase_plus1[i] - 1":"no corresponding limit is expressed")
					}
					NAV_WRITE_TAG_END("vps_ordering_infos");

					BST_FIELD_PROP_2NUMBER1(vps_max_layer_id, 6, "specifies the maximum allowed value of nuh_layer_id of all NAL units in each CVS referring to the VPS")
					BST_FIELD_PROP_2NUMBER1(vps_num_layer_sets_minus1, (long long)quick_log2(vps_num_layer_sets_minus1 + 1) * 2 + 1, "plus 1 specifies the number of layer sets that are specified by the VPS")

					if (vps_num_layer_sets_minus1 >= 1)
					{
						NAV_WRITE_TAG_BEGIN_WITH_ALIAS("vps_layer_set_bits", "for( i = 1; i &lt;= vps_num_layer_sets_minus1; i++ )", "");
						NAV_WRITE_TAG_BEGIN_WITH_ALIAS("vps_layer_bits", "for( j = 0; j &lt;= vps_max_layer_id; j++ )", "");
						for (i = 1; i <= vps_num_layer_sets_minus1; i++) {
							for (int j = 0; j <= vps_max_layer_id; j++) {
								BST_2ARRAY_FIELD_PROP_NUMBER("layer_id_included_flag", i, j, 1, layer_id_included_flag[i][j], "")
							}
						}
						NAV_WRITE_TAG_END("vps_layer_bits");
						NAV_WRITE_TAG_END("vps_layer_set_bits");
					}

					BST_FIELD_PROP_NUMBER1(vps_timing_info_present_flag, 1, vps_timing_info_present_flag?"specifies that vps_num_units_in_tick, vps_time_scale, vps_poc_proportional_to_timing_flag and vps_num_hrd_parameters are present in the VPS"
						:"specifies that vps_num_units_in_tick, vps_time_scale, vps_poc_proportional_to_timing_flag and vps_num_hrd_parameters are not present in the VPS")
					if (vps_timing_info_present_flag) {
						BST_FIELD_PROP_2NUMBER1(vps_num_units_in_tick, 32, "")
						BST_FIELD_PROP_2NUMBER1(vps_time_scale, 32, "")
						BST_FIELD_PROP_NUMBER1(vps_poc_proportional_to_timing_flag, 1, "")
						if (vps_poc_proportional_to_timing_flag) {
							BST_FIELD_PROP_2NUMBER1(vps_num_ticks_poc_diff_one_minus1, (long long)quick_log2(vps_num_ticks_poc_diff_one_minus1 + 1)*2 + 1, "plus 1 specifies the number of clock ticks corresponding to a difference of picture order count values equal to 1")
						}
						BST_FIELD_PROP_2NUMBER1(vps_num_hrd_parameters, (long long)quick_log2(vps_num_hrd_parameters + 1) * 2 + 1, "specifies the number of hrd_parameters( ) syntax structures present in the VPS RBSP before the vps_extension_flag syntax element")
						for (i = 0; i < vps_num_hrd_parameters; i++) {
							BST_FIELD_PROP_REF3(vps_hrd_parameters[i], "vps_hrd_parameters", i);
						}
					}
					BST_FIELD_PROP_NUMBER1(vps_extension_flag, 1, "")
					BST_FIELD_PROP_OBJECT(rbsp_trailing_bits)
				DECLARE_FIELDPROP_END()
			};

			struct SEQ_PARAMETER_SET_RBSP : public SYNTAX_BITSTREAM_MAP
			{
				struct SPS_ORDERING_INFO
				{
					uint16_t	sps_max_dec_pic_buffering_minus1;
					uint16_t	sps_max_num_reorder_pics;
					uint32_t	sps_max_latency_increase_plus1;
				}PACKED;

				struct VUI_PARAMETERS : public SYNTAX_BITSTREAM_MAP
				{
					uint8_t		aspect_ratio_info_present_flag : 1;
					uint8_t		overscan_info_present_flag : 1;
					uint8_t		overscan_appropriate_flag : 1;
					uint8_t		video_signal_type_present_flag : 1;
					uint8_t		video_format : 3;
					uint8_t		video_full_range_flag : 1;

					uint8_t		aspect_ratio_idc;
					uint16_t	sar_width;
					uint16_t	sar_height;

					uint8_t		colour_description_present_flag : 1;
					uint8_t		chroma_loc_info_present_flag : 1;
					uint8_t		neutral_chroma_indication_flag : 1;
					uint8_t		field_seq_flag : 1;
					uint8_t		frame_field_info_present_flag : 1;
					uint8_t		default_display_window_flag : 1;
					uint8_t		vui_timing_info_present_flag : 1;
					uint8_t		reserved_0 : 1;

					uint8_t		colour_primaries;
					uint8_t		transfer_characteristics;
					uint8_t		matrix_coeffs;

					uint8_t		chroma_sample_loc_type_top_field;
					uint8_t		chroma_sample_loc_type_bottom_field;

					uint32_t	def_disp_win_left_offset;
					uint32_t	def_disp_win_right_offset;
					uint32_t	def_disp_win_top_offset;
					uint32_t	def_disp_win_bottom_offset;

					uint32_t	vui_num_units_in_tick;
					uint32_t	vui_time_scale;

					uint8_t		vui_poc_proportional_to_timing_flag : 1;
					uint8_t		vui_hrd_parameters_present_flag : 1;
					uint8_t		reserved_1 : 6;

					uint32_t	vui_num_ticks_poc_diff_one_minus1;

					HRD_PARAMETERS*
								hrd_parameters;

					uint8_t		bitstream_restriction_flag : 1;
					uint8_t		tiles_fixed_structure_flag : 1;
					uint8_t		motion_vectors_over_pic_boundaries_flag : 1;
					uint8_t		restricted_ref_pic_lists_flag : 1;

					uint16_t	min_spatial_segmentation_idc;
					uint8_t		max_bytes_per_pic_denom;
					uint8_t		max_bits_per_min_cu_denom;
					uint8_t		log2_max_mv_length_horizontal;
					uint8_t		log2_max_mv_length_vertical;

					SEQ_PARAMETER_SET_RBSP*
								seq_parameter_set_rbsp;

					VUI_PARAMETERS(SEQ_PARAMETER_SET_RBSP* ptr_seq_parameter_set_rbsp) 
						: def_disp_win_left_offset(0)
						, def_disp_win_right_offset(0)
						, def_disp_win_top_offset(0)
						, def_disp_win_bottom_offset(0)
						, hrd_parameters(NULL)
						, min_spatial_segmentation_idc(0)
						, max_bytes_per_pic_denom(2)
						, max_bits_per_min_cu_denom(1)
						, log2_max_mv_length_horizontal(15)
						, log2_max_mv_length_vertical(15)
						, seq_parameter_set_rbsp(ptr_seq_parameter_set_rbsp){}

					virtual ~VUI_PARAMETERS() {
						AMP_SAFEDEL2(hrd_parameters);
					}

					int Map(AMBst in_bst)
					{
						int iRet = RET_CODE_SUCCESS;
						SYNTAX_BITSTREAM_MAP::Map(in_bst);

						try
						{
							MAP_BST_BEGIN(0);
							nal_read_u(in_bst, aspect_ratio_info_present_flag, 1, uint8_t);
							if (aspect_ratio_info_present_flag) {
								nal_read_u(in_bst, aspect_ratio_idc, 8, uint8_t);
								if (aspect_ratio_idc == 255) {
									nal_read_u(in_bst, sar_width, 16, uint16_t);
									nal_read_u(in_bst, sar_height, 16, uint16_t);
								}
							}

							nal_read_u(in_bst, overscan_info_present_flag, 1, uint8_t);
							if (overscan_info_present_flag) {
								nal_read_u(in_bst, overscan_appropriate_flag, 1, uint8_t);
							}

							nal_read_u(in_bst, video_signal_type_present_flag, 1, uint8_t);
							if (video_signal_type_present_flag) {
								nal_read_u(in_bst, video_format, 3, uint8_t);
								nal_read_u(in_bst, video_full_range_flag, 1, uint8_t);
								nal_read_u(in_bst, colour_description_present_flag, 1, uint8_t);

								if (colour_description_present_flag) {
									nal_read_u(in_bst, colour_primaries, 8, uint8_t);
									nal_read_u(in_bst, transfer_characteristics, 8, uint8_t);
									nal_read_u(in_bst, matrix_coeffs, 8, uint8_t);
								}
							}

							nal_read_u(in_bst, chroma_loc_info_present_flag, 1, uint8_t);
							if (chroma_loc_info_present_flag) {
								nal_read_ue(in_bst, chroma_sample_loc_type_top_field, uint8_t);
								nal_read_ue(in_bst, chroma_sample_loc_type_bottom_field, uint8_t);
							}

							nal_read_u(in_bst, neutral_chroma_indication_flag, 1, uint8_t);
							nal_read_u(in_bst, field_seq_flag, 1, uint8_t);
							nal_read_u(in_bst, frame_field_info_present_flag, 1, uint8_t);
							nal_read_u(in_bst, default_display_window_flag, 1, uint8_t);

							if (default_display_window_flag) {
								nal_read_ue(in_bst, def_disp_win_left_offset, uint32_t);
								nal_read_ue(in_bst, def_disp_win_right_offset, uint32_t);
								nal_read_ue(in_bst, def_disp_win_top_offset, uint32_t);
								nal_read_ue(in_bst, def_disp_win_bottom_offset, uint32_t);
							}

							nal_read_u(in_bst, vui_timing_info_present_flag, 1, uint8_t);
							if (vui_timing_info_present_flag)
							{
								nal_read_u(in_bst, vui_num_units_in_tick, 32, uint32_t);
								nal_read_u(in_bst, vui_time_scale, 32, uint32_t);

								nal_read_u(in_bst, vui_poc_proportional_to_timing_flag, 1, uint8_t);
								if (vui_poc_proportional_to_timing_flag)
								{
									nal_read_ue(in_bst, vui_num_ticks_poc_diff_one_minus1, uint32_t);
								}

								nal_read_u(in_bst, vui_hrd_parameters_present_flag, 1, uint8_t);
								if (vui_hrd_parameters_present_flag)
								{
									nal_read_ref(in_bst, hrd_parameters, HRD_PARAMETERS, 1, seq_parameter_set_rbsp->sps_max_sub_layers_minus1);
								}
							}

							nal_read_u(in_bst, bitstream_restriction_flag, 1, uint8_t);
							if (bitstream_restriction_flag)
							{
								nal_read_u(in_bst, tiles_fixed_structure_flag, 1, uint8_t);
								nal_read_u(in_bst, motion_vectors_over_pic_boundaries_flag, 1, uint8_t);
								nal_read_u(in_bst, restricted_ref_pic_lists_flag, 1, uint8_t);

								nal_read_ue(in_bst, min_spatial_segmentation_idc, uint16_t);
								nal_read_ue(in_bst, max_bytes_per_pic_denom, uint8_t);
								nal_read_ue(in_bst, max_bits_per_min_cu_denom, uint8_t);
								nal_read_ue(in_bst, log2_max_mv_length_horizontal, uint8_t);
								nal_read_ue(in_bst, log2_max_mv_length_vertical, uint8_t);
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
						BST_FIELD_PROP_NUMBER1(aspect_ratio_info_present_flag, 1, aspect_ratio_info_present_flag?"aspect_ratio_idc is present":"aspect_ratio_idc is not present")
						if (aspect_ratio_info_present_flag)
						{
							BST_FIELD_PROP_NUMBER1(aspect_ratio_idc, 8, sample_aspect_ratio_descs[aspect_ratio_idc]);
							if (aspect_ratio_idc == 255) {
								BST_FIELD_PROP_2NUMBER1(sar_width, 16, "the horizontal size of the sample aspect ratio (in arbitrary units)");
								BST_FIELD_PROP_2NUMBER1(sar_height, 16, "the vertical size of the sample aspect ratio (in the same arbitrary units as sar_width).");
							}
						}

						BST_FIELD_PROP_NUMBER1(overscan_info_present_flag, 1, overscan_info_present_flag ? "the overscan_appropriate_flag is present" : "the preferred display method for the video signal is unspecified");
						if (overscan_info_present_flag) {
							BST_FIELD_PROP_NUMBER1(overscan_appropriate_flag, 1, overscan_appropriate_flag ? "indicates that the cropped decoded pictures output are suitable for display using overscan" 
								: "indicates that the cropped decoded pictures output contain visually important information in the entire region");
						}
						BST_FIELD_PROP_NUMBER1(video_signal_type_present_flag, 1, video_signal_type_present_flag ? "video_format, video_full_range_flag and colour_description_present_flag are present" 
							: "specify that video_format, video_full_range_flag and colour_description_present_flag are not present.");
						if (video_signal_type_present_flag)
						{
							BST_FIELD_PROP_NUMBER1(video_format, 3, vui_video_format_names[video_format]);
							BST_FIELD_PROP_NUMBER1(video_full_range_flag, 1, "indicates the black level and range of the luma and chroma signals");
							BST_FIELD_PROP_NUMBER1(colour_description_present_flag, 1, colour_description_present_flag?"specifies that colour_primaries, transfer_characteristics and matrix_coeffs are present"
								:"specifies that colour_primaries, transfer_characteristics and matrix_coeffs are not present.");

							if (colour_description_present_flag)
							{
								BST_FIELD_PROP_2NUMBER1(colour_primaries, 8, vui_colour_primaries_names[colour_primaries]);
								BST_FIELD_PROP_2NUMBER1(transfer_characteristics, 8, vui_transfer_characteristics_names[transfer_characteristics]);
								BST_FIELD_PROP_2NUMBER1(matrix_coeffs, 8, vui_matrix_coeffs_descs[matrix_coeffs]);
							}
						}

						BST_FIELD_PROP_NUMBER1(chroma_loc_info_present_flag, 1, chroma_loc_info_present_flag ? "specifies that chroma_sample_loc_type_top_field and chroma_sample_loc_type_bottom_field are present" 
							: "specifies that chroma_sample_loc_type_top_field and chroma_sample_loc_type_bottom_field are not present");
						if (chroma_loc_info_present_flag)
						{
							BST_FIELD_PROP_2NUMBER1(chroma_sample_loc_type_top_field, (long long)quick_log2(chroma_sample_loc_type_top_field + 1)*2 + 1, "")
							BST_FIELD_PROP_2NUMBER1(chroma_sample_loc_type_bottom_field, (long long)quick_log2(chroma_sample_loc_type_bottom_field + 1) * 2 + 1, "")
						}

						BST_FIELD_PROP_NUMBER1(neutral_chroma_indication_flag, 1, neutral_chroma_indication_flag ? "indicates that the value of all decoded chroma samples is equal to 1<<(BitDepthC-1)"
							: "provides no indication of decoded chroma sample values.");
						BST_FIELD_PROP_NUMBER1(field_seq_flag, 1, field_seq_flag ? "indicates that the CVS conveys pictures that represent fields, and specifies that a picture timing SEI message shall be present in every access unit of the current CVS"
							: "indicates that the CVS conveys pictures that represent frames and that a picture timing SEI message may or may not be present in any access unit of the current CVS");
						BST_FIELD_PROP_NUMBER1(frame_field_info_present_flag, 1, frame_field_info_present_flag ? "specifies that picture timing SEI messages are present for every picture and include the pic_struct, source_scan_type and duplicate_flag syntax elements"
							: "specifies that the pic_struct syntax element is not present in picture timing SEI messages");
						BST_FIELD_PROP_NUMBER1(default_display_window_flag, 1, default_display_window_flag ? "indicates that the default display window parameters follow next in the VUI."
							: "indicates that the default display window parameters are not present");

						if (default_display_window_flag)
						{
							BST_FIELD_PROP_2NUMBER1(def_disp_win_left_offset, (long long)quick_log2(def_disp_win_left_offset + 1) * 2 + 1, "leftOffset = conf_win_left_offset + def_disp_win_left_offset");
							BST_FIELD_PROP_2NUMBER1(def_disp_win_right_offset, (long long)quick_log2(def_disp_win_right_offset + 1) * 2 + 1, "rightOffset = conf_win_right_offset + def_disp_win_right_offset");
							BST_FIELD_PROP_2NUMBER1(def_disp_win_top_offset, (long long)quick_log2(def_disp_win_top_offset + 1) * 2 + 1, "topOffset = conf_win_top_offset + def_disp_win_top_offset (E");
							BST_FIELD_PROP_2NUMBER1(def_disp_win_bottom_offset, (long long)quick_log2(def_disp_win_bottom_offset + 1) * 2 + 1, "bottomOffset = conf_win_bottom_offset + def_disp_win_bottom_offset");
						}

						BST_FIELD_PROP_NUMBER1(vui_timing_info_present_flag, 1, vui_timing_info_present_flag ? "specifies that vui_num_units_in_tick, vui_time_scale, vui_poc_proportional_to_timing_flag and vui_hrd_parameters_present_flag are present"
							: "specifies that vui_num_units_in_tick, vui_time_scale, vui_poc_proportional_to_timing_flag and vui_hrd_parameters_present_flag are not present");
						if (vui_timing_info_present_flag)
						{
							BST_FIELD_PROP_2NUMBER1(vui_num_units_in_tick, 32, "the number of time units of a clock operating at the frequency vui_time_scale Hz that corresponds to one increment (called a clock tick) of a clock tick counter.");
							BST_FIELD_PROP_2NUMBER1(vui_time_scale, 32, "the number of time units that pass in one second.");
							BST_FIELD_PROP_NUMBER1(vui_poc_proportional_to_timing_flag, 1, vui_poc_proportional_to_timing_flag ? "indicates that the picture order count value for each picture in the CVS that is not the first picture in the CVS, in decoding order, is proportional to the output time of the picture relative to the output time of the first picture in the CVS."
								: "indicates that the picture order count value for each picture in the CVS that is not the first picture in the CVS, in decoding order, may or may not be proportional to the output time of the picture relative to the output time of the first picture in the CVS.");
							if (vui_poc_proportional_to_timing_flag)
							{
								BST_FIELD_PROP_2NUMBER1(vui_num_ticks_poc_diff_one_minus1, (long long)quick_log2(vui_num_ticks_poc_diff_one_minus1 + 1) * 2 + 1, "plus 1 specifies the number of clock ticks corresponding to a difference of picture order count values equal to 1");
							}
							BST_FIELD_PROP_NUMBER1(vui_hrd_parameters_present_flag, 1, vui_hrd_parameters_present_flag ? "specifies that the syntax structure hrd_parameters( ) is present."
								: "specifies that the syntax structure hrd_parameters( ) is not present.");

							if (vui_hrd_parameters_present_flag)
							{
								BST_FIELD_PROP_REF1(hrd_parameters);
							}
						}

						BST_FIELD_PROP_NUMBER1(bitstream_restriction_flag, 1, bitstream_restriction_flag ? "specifies that the bitstream restriction parameters for the CVS are present"
							: "specifies that the bitstream restriction parameters for the CVS are not present.");
						if (bitstream_restriction_flag)
						{
							BST_FIELD_PROP_NUMBER1(tiles_fixed_structure_flag, 1, tiles_fixed_structure_flag ? "indicates that each PPS that is active in the CVS has the same value of the syntax elements num_tile_columns_minus1, num_tile_rows_minus1, uniform_spacing_flag, column_width_minus1[i], row_height_minus1[i] and loop_filter_across_tiles_enabled_flag, when present."
								: "indicates that tiles syntax elements in different PPSs may or may not have the same value.");
							BST_FIELD_PROP_NUMBER1(motion_vectors_over_pic_boundaries_flag, 1, motion_vectors_over_pic_boundaries_flag ? "indicates that no sample outside the picture boundaries and no sample at a fractional sample position for which the sample value is derived using one or more samples outside the picture boundaries is used for inter prediction of any sample."
								: "indicates that one or more samples outside the picture boundaries may be used in inter prediction.");
							BST_FIELD_PROP_NUMBER1(restricted_ref_pic_lists_flag, 1, "");
							BST_FIELD_PROP_2NUMBER1(min_spatial_segmentation_idc, (long long)quick_log2(min_spatial_segmentation_idc + 1) * 2 + 1, "when not equal to 0, establishes a bound on the maximum possible size of distinct coded spatial segmentation regions in the pictures of the CVS.");
							BST_FIELD_PROP_2NUMBER1(max_bytes_per_pic_denom, (long long)quick_log2(max_bytes_per_pic_denom + 1) * 2 + 1, "indicates a number of bytes not exceeded by the sum of the sizes of the VCL NAL units associated with any coded picture in the CVS.");
							BST_FIELD_PROP_2NUMBER1(max_bits_per_min_cu_denom, (long long)quick_log2(max_bits_per_min_cu_denom + 1) * 2 + 1, "indicates an upper bound for the number of coded bits of coding_unit( ) data for any coding block in any picture of the CVS.");
							BST_FIELD_PROP_2NUMBER1(log2_max_mv_length_horizontal, (long long)quick_log2(log2_max_mv_length_horizontal + 1) * 2 + 1, "indicate the maximum absolute value of a decoded horizontal motion vector component, respectively, in quarter luma sample units, for all pictures in the CVS");
							BST_FIELD_PROP_2NUMBER1(log2_max_mv_length_vertical, (long long)quick_log2(log2_max_mv_length_vertical + 1) * 2 + 1, "indicate the maximum absolute value of a decoded vertical motion vector component, respectively, in quarter luma sample units, for all pictures in the CVS");
						}
					DECLARE_FIELDPROP_END()

				}PACKED;

				struct SPS_RANGE_EXTENSION
				{
					uint8_t		transform_skip_rotation_enabled_flag : 1;
					uint8_t		transform_skip_context_enabled_flag : 1;
					uint8_t		implicit_rdpcm_enabled_flag : 1;
					uint8_t		explicit_rdpcm_enabled_flag : 1;
					uint8_t		extended_precision_processing_flag : 1;
					uint8_t		intra_smoothing_disabled_flag : 1;
					uint8_t		high_precision_offsets_enabled_flag : 1;
					uint8_t		persistent_rice_adaptation_enabled_flag : 1;

					uint8_t		cabac_bypass_alignment_enabled_flag : 1;
					uint8_t		reserved : 7;
				};

				struct SPS_MULTILAYER_EXTENSION
				{
					uint8_t		inter_view_mv_vert_constraint_flag : 1;
					uint8_t		reserved : 7;
				};

				struct SPS_3D_EXTENSION
				{
					uint8_t		iv_di_mc_enabled_flag0 : 1;
					uint8_t		iv_mv_scal_enabled_flag0 : 1;
					uint8_t		iv_res_pred_enabled_flag : 1;
					uint8_t		depth_ref_enabled_flag : 1;
					uint8_t		vsp_mc_enabled_flag : 1;
					uint8_t		dbbp_enabled_flag : 1;
					uint8_t		reserved_0 : 2;
					uint8_t		log2_ivmc_sub_pb_size_minus3;

					uint8_t		iv_di_mc_enabled_flag1 : 1;
					uint8_t		iv_mv_scal_enabled_flag1 : 1;
					uint8_t		tex_mc_enabled_flag : 1;
					uint8_t		intra_contour_enabled_flag : 1;
					uint8_t		intra_dc_only_wedge_enabled_flag : 1;
					uint8_t		cqt_cu_part_pred_enabled_flag : 1;
					uint8_t		inter_dc_only_enabled_flag : 1;
					uint8_t		skip_intra_enabled_flag : 1;
					uint8_t		log2_texmc_sub_pb_size_minus3;
				};

				uint8_t		sps_video_parameter_set_id : 4;
				uint8_t		sps_max_sub_layers_minus1 : 3;
				uint8_t		sps_temporal_id_nesting_flag : 1;

				PROFILE_TIER_LEVEL*
							profile_tier_level;

				uint8_t		sps_seq_parameter_set_id;
				uint8_t		chroma_format_idc;
				uint8_t		separate_colour_plane_flag;

				uint32_t	pic_width_in_luma_samples;
				uint32_t	pic_height_in_luma_samples;

				uint8_t		conformance_window_flag;

				uint32_t	conf_win_left_offset;
				uint32_t	conf_win_right_offset;
				uint32_t	conf_win_top_offset;
				uint32_t	conf_win_bottom_offset;

				uint8_t		bit_depth_luma_minus8;
				uint8_t		bit_depth_chroma_minus8;
				uint8_t		log2_max_pic_order_cnt_lsb_minus4;

				uint8_t		sps_sub_layer_ordering_info_present_flag;

				SPS_ORDERING_INFO*
							sps_ordering_info;

				uint16_t	log2_min_luma_coding_block_size_minus3;
				uint16_t	log2_diff_max_min_luma_coding_block_size;

				uint16_t	log2_min_luma_transform_block_size_minus2;
				uint16_t	log2_diff_max_min_luma_transform_block_size;

				uint16_t	max_transform_hierarchy_depth_inter;
				uint16_t	max_transform_hierarchy_depth_intra;

				uint8_t		scaling_list_enabled_flag : 1;
				uint8_t		sps_scaling_list_data_present_flag : 1;
				uint8_t		amp_enabled_flag : 1;
				uint8_t		sample_adaptive_offset_enabled_flag : 1;
				uint8_t		pcm_enabled_flag : 1;
				uint8_t		pcm_loop_filter_disabled_flag : 1;
				uint8_t		reserved_0 : 2;

				SCALING_LIST_DATA*
							scaling_list_data;

				uint8_t		pcm_sample_bit_depth_luma_minus1 : 4;
				uint8_t		pcm_sample_bit_depth_chroma_minus1 : 4;

				uint16_t	log2_min_pcm_luma_coding_block_size_minus3;
				uint16_t	log2_diff_max_min_pcm_luma_coding_block_size;

				uint8_t		num_short_term_ref_pic_sets;
				ST_REF_PIC_SETS*
							st_ref_pic_sets;

				uint8_t		long_term_ref_pics_present_flag : 1;
				uint8_t		sps_temporal_mvp_enabled_flag : 1;
				uint8_t		strong_intra_smoothing_enabled_flag : 1;
				uint8_t		vui_parameters_present_flag : 1;
				uint8_t		sps_extension_present_flag : 1;
				uint8_t		reserved_1 : 3;

				uint8_t		num_long_term_ref_pics_sps;

				uint16_t	lt_ref_pic_poc_lsb_sps[33];
				CAMBitArray	used_by_curr_pic_lt_sps_flag;

				VUI_PARAMETERS*
							vui_parameters;

				uint8_t		sps_range_extension_flag : 1;
				uint8_t		sps_multilayer_extension_flag : 1;
				uint8_t		sps_3d_extension_flag : 1;
				uint8_t		sps_scc_extension_flag : 1;
				uint8_t		sps_extension_4bits : 4;

				SPS_RANGE_EXTENSION
							sps_range_extension;
				SPS_MULTILAYER_EXTENSION
							sps_multilayer_extension;
				SPS_3D_EXTENSION
							sps_3d_extension;
				RBSP_TRAILING_BITS
							rbsp_trailing_bits;

				VideoBitstreamCtx*
							ptr_ctx_video_bst;

				uint32_t	MinCbLog2SizeY = 0;
				uint32_t	CtbLog2SizeY = 0;
				uint32_t	MinCbSizeY = 0;
				uint32_t	CtbSizeY = 0;
				uint32_t	PicWidthInMinCbsY = 0;
				uint32_t	PicWidthInCtbsY = 0;
				uint32_t	PicHeightInMinCbsY = 0;
				uint32_t	PicHeightInCtbsY = 0;
				uint32_t	PicSizeInMinCbsY = 0;
				uint32_t	PicSizeInCtbsY = 0;
				uint32_t	PicSizeInSamplesY = 0;
				uint32_t	PicWidthInSamplesC = 0;
				uint32_t	PicHeightInSamplesC = 0;
				uint8_t		SubWidthC = 0;
				uint8_t		SubHeightC = 0;
				uint32_t	CtbWidthC = 0;
				uint32_t	CtbHeightC = 0;
				uint32_t	display_width = 0;
				uint32_t	display_height = 0;

				SEQ_PARAMETER_SET_RBSP(VideoBitstreamCtx* ctx = NULL)
					: profile_tier_level(NULL)
					, separate_colour_plane_flag(0)
					, conf_win_left_offset(0)
					, conf_win_right_offset(0)
					, conf_win_top_offset(0)
					, conf_win_bottom_offset(0)
					, sps_ordering_info(NULL)
					, scaling_list_data(NULL)
					, st_ref_pic_sets(NULL)
					, vui_parameters(NULL)
					, sps_range_extension_flag(0)
					, sps_multilayer_extension_flag(0)
					, sps_3d_extension_flag(0)
					, sps_scc_extension_flag(0)
					, sps_extension_4bits(0)
					, ptr_ctx_video_bst(ctx)
				{
				}

				virtual ~SEQ_PARAMETER_SET_RBSP()
				{
					AMP_SAFEDEL2(vui_parameters);
					AMP_SAFEDEL2(st_ref_pic_sets);
					AMP_SAFEDEL2(scaling_list_data);
					AMP_SAFEDELA2(sps_ordering_info);
					AMP_SAFEDEL2(profile_tier_level);
				}

				int Map(AMBst in_bst)
				{
					int iRet = RET_CODE_SUCCESS;
					SYNTAX_BITSTREAM_MAP::Map(in_bst);

					try
					{
						MAP_BST_BEGIN(0);
						nal_read_u(in_bst, sps_video_parameter_set_id, 4, uint8_t);
						nal_read_u(in_bst, sps_max_sub_layers_minus1, 3, uint8_t);
						nal_read_u(in_bst, sps_temporal_id_nesting_flag, 1, uint8_t);

						nal_read_ref(in_bst, profile_tier_level, PROFILE_TIER_LEVEL, 1, sps_max_sub_layers_minus1);

						nal_read_ue(in_bst, sps_seq_parameter_set_id, uint8_t);
						nal_read_ue(in_bst, chroma_format_idc, uint8_t);

						if (ptr_ctx_video_bst != NULL)
						{
							if (ptr_ctx_video_bst->in_scanning &&
								ptr_ctx_video_bst->sps_seq_parameter_set_id[sps_video_parameter_set_id] == sps_seq_parameter_set_id)
							{
								return RET_CODE_NOTHING_TODO;
							}
							else
								ptr_ctx_video_bst->sps_seq_parameter_set_id[sps_video_parameter_set_id] = sps_seq_parameter_set_id;
						}

						if (chroma_format_idc == 3) {
							nal_read_u(in_bst, separate_colour_plane_flag, 1, uint8_t);
						}

						nal_read_ue(in_bst, pic_width_in_luma_samples, uint32_t);
						nal_read_ue(in_bst, pic_height_in_luma_samples, uint32_t);

						nal_read_u(in_bst, conformance_window_flag, 1, uint8_t);

						display_width = pic_width_in_luma_samples;
						display_height = pic_height_in_luma_samples;
						if (conformance_window_flag)
						{
							nal_read_ue(in_bst, conf_win_left_offset, uint32_t);
							nal_read_ue(in_bst, conf_win_right_offset, uint32_t);
							nal_read_ue(in_bst, conf_win_top_offset, uint32_t);
							nal_read_ue(in_bst, conf_win_bottom_offset, uint32_t);

							uint32_t sub_width_c = ((1 == chroma_format_idc) || (2 == chroma_format_idc)) && (0 == separate_colour_plane_flag) ? 2 : 1;
							uint32_t sub_height_c = (1 == chroma_format_idc) && (0 == separate_colour_plane_flag) ? 2 : 1;
							display_width -= sub_width_c * (conf_win_left_offset + conf_win_right_offset);
							display_height -= sub_height_c * (conf_win_top_offset + conf_win_bottom_offset);
						}

						nal_read_ue(in_bst, bit_depth_luma_minus8, uint8_t);
						nal_read_ue(in_bst, bit_depth_chroma_minus8, uint8_t);
						nal_read_ue(in_bst, log2_max_pic_order_cnt_lsb_minus4, uint8_t);

						nal_read_u(in_bst, sps_sub_layer_ordering_info_present_flag, 1, uint8_t);

						AMP_NEW(sps_ordering_info, SPS_ORDERING_INFO, (size_t)sps_max_sub_layers_minus1 + 1);
						if (sps_ordering_info == nullptr)
							throw AMException(RET_CODE_OUTOFMEMORY);

						for (int i = sps_sub_layer_ordering_info_present_flag ? 0 : sps_max_sub_layers_minus1; i <= sps_max_sub_layers_minus1; i++) {
							nal_read_ue(in_bst, sps_ordering_info[i].sps_max_dec_pic_buffering_minus1, uint16_t);
							nal_read_ue(in_bst, sps_ordering_info[i].sps_max_num_reorder_pics, uint16_t);
							nal_read_ue(in_bst, sps_ordering_info[i].sps_max_latency_increase_plus1, uint32_t);
						}

						nal_read_ue(in_bst, log2_min_luma_coding_block_size_minus3, uint16_t);
						MinCbLog2SizeY = log2_min_luma_coding_block_size_minus3 + 3;

						nal_read_ue(in_bst, log2_diff_max_min_luma_coding_block_size, uint16_t);
						CtbLog2SizeY = MinCbLog2SizeY + log2_diff_max_min_luma_coding_block_size;

						MinCbSizeY = 1 << MinCbLog2SizeY;
						CtbSizeY = 1 << CtbLog2SizeY;
						PicWidthInMinCbsY = pic_width_in_luma_samples / MinCbSizeY;
						PicWidthInCtbsY = (pic_width_in_luma_samples + CtbSizeY -1) / CtbSizeY;
						PicHeightInMinCbsY = pic_height_in_luma_samples / MinCbSizeY;
						PicHeightInCtbsY = (pic_height_in_luma_samples + CtbSizeY -1) / CtbSizeY;
						PicSizeInMinCbsY = PicWidthInMinCbsY * PicHeightInMinCbsY;
						PicSizeInCtbsY = PicWidthInCtbsY * PicHeightInCtbsY;
						PicSizeInSamplesY = pic_width_in_luma_samples * pic_height_in_luma_samples;
						SubWidthC = (chroma_format_idc == 1 || chroma_format_idc == 2) ? 2 : 1;
						SubHeightC = (chroma_format_idc == 1) ? 2 : 1;
						PicWidthInSamplesC = pic_width_in_luma_samples / SubWidthC;
						PicHeightInSamplesC = pic_height_in_luma_samples / SubHeightC;
						if (chroma_format_idc != 0 && separate_colour_plane_flag != 1)
						{
							CtbWidthC = CtbSizeY / SubWidthC;
							CtbHeightC = CtbSizeY / SubHeightC;
						}

						nal_read_ue(in_bst, log2_min_luma_transform_block_size_minus2, uint16_t);
						nal_read_ue(in_bst, log2_diff_max_min_luma_transform_block_size, uint16_t);

						nal_read_ue(in_bst, max_transform_hierarchy_depth_inter, uint16_t);
						nal_read_ue(in_bst, max_transform_hierarchy_depth_intra, uint16_t);

						nal_read_u(in_bst, scaling_list_enabled_flag, 1, uint8_t);
						if (scaling_list_enabled_flag) {
							nal_read_u(in_bst, sps_scaling_list_data_present_flag, 1, uint8_t);
							if (sps_scaling_list_data_present_flag) {
								nal_read_ref(in_bst, scaling_list_data, SCALING_LIST_DATA);
							}
						}

						nal_read_u(in_bst, amp_enabled_flag, 1, uint8_t);
						nal_read_u(in_bst, sample_adaptive_offset_enabled_flag, 1, uint8_t);
						nal_read_u(in_bst, pcm_enabled_flag, 1, uint8_t);

						if (pcm_enabled_flag)
						{
							nal_read_u(in_bst, pcm_sample_bit_depth_luma_minus1, 4, uint8_t);
							nal_read_u(in_bst, pcm_sample_bit_depth_chroma_minus1, 4, uint8_t);

							nal_read_ue(in_bst, log2_min_pcm_luma_coding_block_size_minus3, uint16_t);
							nal_read_ue(in_bst, log2_diff_max_min_pcm_luma_coding_block_size, uint16_t);

							nal_read_u(in_bst, pcm_loop_filter_disabled_flag, 1, uint8_t);
						}

						nal_read_ue(in_bst, num_short_term_ref_pic_sets, uint8_t);
						nal_read_ref(in_bst, st_ref_pic_sets, ST_REF_PIC_SETS, num_short_term_ref_pic_sets);

						nal_read_u(in_bst, long_term_ref_pics_present_flag, 1, uint8_t);
						if (long_term_ref_pics_present_flag)
						{
							nal_read_ue(in_bst, num_long_term_ref_pics_sps, uint8_t);
							if (num_long_term_ref_pics_sps > 32)
								throw AMException(RET_CODE_OUT_OF_RANGE, _T("num_long_term_ref_pics_sps should be in the range 0 to 32, inclusive"));
							for (int i = 0; i < num_long_term_ref_pics_sps;i++){
								nal_read_u(in_bst, lt_ref_pic_poc_lsb_sps[i], log2_max_pic_order_cnt_lsb_minus4 + 4, uint16_t);
								nal_read_bitarray(in_bst, used_by_curr_pic_lt_sps_flag, i);
							}
						}

						nal_read_u(in_bst, sps_temporal_mvp_enabled_flag, 1, uint8_t);
						nal_read_u(in_bst, strong_intra_smoothing_enabled_flag, 1, uint8_t);
						nal_read_u(in_bst, vui_parameters_present_flag, 1, uint8_t);

						if (vui_parameters_present_flag)
						{
							nal_read_ref(in_bst, vui_parameters, VUI_PARAMETERS, this);
						}

						nal_read_u(in_bst, sps_extension_present_flag, 1, uint8_t);
						if (sps_extension_present_flag)
						{
							nal_read_u(in_bst, sps_range_extension_flag, 1, uint8_t);
							nal_read_u(in_bst, sps_multilayer_extension_flag, 1, uint8_t);
							nal_read_u(in_bst, sps_3d_extension_flag, 1, uint8_t);
							nal_read_u(in_bst, sps_scc_extension_flag, 1, uint8_t);
							nal_read_u(in_bst, sps_extension_4bits, 4, uint8_t);
						}

						if (sps_range_extension_flag)
						{
							nal_read_u(in_bst, sps_range_extension.transform_skip_rotation_enabled_flag, 1, uint8_t);
							nal_read_u(in_bst, sps_range_extension.transform_skip_context_enabled_flag, 1, uint8_t);
							nal_read_u(in_bst, sps_range_extension.implicit_rdpcm_enabled_flag, 1, uint8_t);
							nal_read_u(in_bst, sps_range_extension.explicit_rdpcm_enabled_flag, 1, uint8_t);
							nal_read_u(in_bst, sps_range_extension.extended_precision_processing_flag, 1, uint8_t);
							nal_read_u(in_bst, sps_range_extension.intra_smoothing_disabled_flag, 1, uint8_t);
							nal_read_u(in_bst, sps_range_extension.high_precision_offsets_enabled_flag, 1, uint8_t);
							nal_read_u(in_bst, sps_range_extension.persistent_rice_adaptation_enabled_flag, 1, uint8_t);
							nal_read_u(in_bst, sps_range_extension.cabac_bypass_alignment_enabled_flag, 1, uint8_t);
						}

						if (sps_multilayer_extension_flag)
						{
							nal_read_u(in_bst, sps_multilayer_extension.inter_view_mv_vert_constraint_flag, 1, uint8_t);
						}

						if (sps_3d_extension_flag)
						{
							nal_read_u(in_bst, sps_3d_extension.iv_di_mc_enabled_flag0, 1, uint8_t);
							nal_read_u(in_bst, sps_3d_extension.iv_mv_scal_enabled_flag0, 1, uint8_t);
							nal_read_ue(in_bst, sps_3d_extension.log2_ivmc_sub_pb_size_minus3, uint8_t);
							nal_read_u(in_bst, sps_3d_extension.iv_res_pred_enabled_flag, 1, uint8_t);
							nal_read_u(in_bst, sps_3d_extension.depth_ref_enabled_flag, 1, uint8_t);
							nal_read_u(in_bst, sps_3d_extension.vsp_mc_enabled_flag, 1, uint8_t);
							nal_read_u(in_bst, sps_3d_extension.dbbp_enabled_flag, 1, uint8_t);

							nal_read_u(in_bst, sps_3d_extension.iv_di_mc_enabled_flag1, 1, uint8_t);
							nal_read_u(in_bst, sps_3d_extension.iv_mv_scal_enabled_flag1, 1, uint8_t);
							nal_read_u(in_bst, sps_3d_extension.tex_mc_enabled_flag, 1, uint8_t);
							nal_read_ue(in_bst, sps_3d_extension.log2_texmc_sub_pb_size_minus3, uint8_t);
							nal_read_u(in_bst, sps_3d_extension.intra_contour_enabled_flag, 1, uint8_t);
							nal_read_u(in_bst, sps_3d_extension.intra_dc_only_wedge_enabled_flag, 1, uint8_t);
							nal_read_u(in_bst, sps_3d_extension.cqt_cu_part_pred_enabled_flag, 1, uint8_t);
							nal_read_u(in_bst, sps_3d_extension.inter_dc_only_enabled_flag, 1, uint8_t);
							nal_read_u(in_bst, sps_3d_extension.skip_intra_enabled_flag, 1, uint8_t);
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

				int Unmap(AMBst out_bst)
				{
					UNREFERENCED_PARAMETER(out_bst);
					return RET_CODE_ERROR_NOTIMPL;
				}

				DECLARE_FIELDPROP_BEGIN()
					BST_FIELD_PROP_2NUMBER1(sps_video_parameter_set_id, 4, "specifies the value of the vps_video_parameter_set_id of the active VPS.")
					BST_FIELD_PROP_2NUMBER1(sps_max_sub_layers_minus1, 3, "plus 1 specifies the maximum number of temporal sub-layers that may be present in each CVS referring to the SPS.")
					BST_FIELD_PROP_NUMBER1(sps_temporal_id_nesting_flag, 1, sps_max_sub_layers_minus1?"specifies whether inter prediction is additionally restricted for CVSs referring to the SPS.":"shall be equal to 1.")

					const char* szProfileName = get_hevc_profile_name(profile_tier_level->GetHEVCProfile());
					NAV_WRITE_TAG_BEGIN_WITH_ALIAS("profile_tier_level", "profile_tier_level(1, sps_max_sub_layers_minus1)", szProfileName);
					BST_FIELD_PROP_REF(profile_tier_level)
					NAV_WRITE_TAG_END("profile_tier_level");

					BST_FIELD_PROP_2NUMBER1(sps_seq_parameter_set_id, (long long)quick_log2(sps_seq_parameter_set_id + 1) * 2 + 1, "provides an identifier for the SPS for reference by other syntax elements.")
					BST_FIELD_PROP_2NUMBER1(chroma_format_idc, (long long)quick_log2(chroma_format_idc + 1) * 2 + 1, chroma_format_idc == 0 ? "Monochrome" : (
																											chroma_format_idc == 1 ? "4:2:0" : (
																											chroma_format_idc == 2 ? "4:2:2" : (
																											chroma_format_idc == 3 ? "4:4:4" : "Unknown"))));

					if (chroma_format_idc == 3)
					{
						BST_FIELD_PROP_NUMBER1(separate_colour_plane_flag, 1, separate_colour_plane_flag ? "the three colour planes are separately processed as monochrome sampled pictures."
							: "each of the two chroma arrays has the same height and width as the luma array.");
					}

					BST_FIELD_PROP_2NUMBER1(pic_width_in_luma_samples, (long long)quick_log2(pic_width_in_luma_samples + 1) * 2 + 1, "the width of each decoded picture in units of luma samples.");
					BST_FIELD_PROP_2NUMBER1(pic_height_in_luma_samples, (long long)quick_log2(pic_height_in_luma_samples + 1) * 2 + 1, "the height of each decoded picture in units of luma samples.");
					BST_FIELD_PROP_NUMBER1(conformance_window_flag, 1, conformance_window_flag ? "indicates that the conformance cropping window offset parameters follow next in the SPS."
						: "indicates that the conformance cropping window offset parameters are not present.");

					uint32_t display_width = pic_width_in_luma_samples, display_height = pic_height_in_luma_samples;
					if (conformance_window_flag)
					{
						BST_FIELD_PROP_2NUMBER1(conf_win_left_offset, (long long)quick_log2(conf_win_left_offset + 1) * 2 + 1, "in terms of a rectangular region specified in picture coordinates for output.");
						BST_FIELD_PROP_2NUMBER1(conf_win_right_offset, (long long)quick_log2(conf_win_right_offset + 1) * 2 + 1, "in terms of a rectangular region specified in picture coordinates for output.");
						BST_FIELD_PROP_2NUMBER1(conf_win_top_offset, (long long)quick_log2(conf_win_top_offset + 1) * 2 + 1, "in terms of a rectangular region specified in picture coordinates for output.");
						BST_FIELD_PROP_2NUMBER1(conf_win_bottom_offset, (long long)quick_log2(conf_win_bottom_offset + 1) * 2 + 1, "in terms of a rectangular region specified in picture coordinates for output.");

						uint32_t sub_width_c = ((1 == chroma_format_idc) || (2 == chroma_format_idc)) && (0 == separate_colour_plane_flag) ? 2 : 1;
						uint32_t sub_height_c = (1 == chroma_format_idc) && (0 == separate_colour_plane_flag) ? 2 : 1;
						display_width -= sub_width_c*(conf_win_left_offset + conf_win_right_offset);
						display_height -= sub_height_c*(conf_win_top_offset + conf_win_bottom_offset);
					}

					NAV_WRITE_TAG_WITH_NUMBER_VALUE1(display_width, "The display width");
					NAV_WRITE_TAG_WITH_NUMBER_VALUE1(display_height, "The display height");

					BST_FIELD_PROP_2NUMBER1(bit_depth_luma_minus8, (long long)quick_log2(bit_depth_luma_minus8 + 1) * 2 + 1, "BitDepthY = 8 + bit_depth_luma_minus8/QpBdOffsetY = 6 * bit_depth_luma_minus8");
					BST_FIELD_PROP_2NUMBER1(bit_depth_chroma_minus8, (long long)quick_log2(bit_depth_chroma_minus8 + 1) * 2 + 1, "BitDepthC = 8 + bit_depth_chroma_minus8/QpBdOffsetC = 6 * bit_depth_chroma_minus8 (7");
					BST_FIELD_PROP_2NUMBER1(log2_max_pic_order_cnt_lsb_minus4, (long long)quick_log2(log2_max_pic_order_cnt_lsb_minus4 + 1) * 2 + 1, "MaxPicOrderCntLsb = 2^(log2_max_pic_order_cnt_lsb_minus4 + 4)");

					BST_FIELD_PROP_NUMBER1(sps_sub_layer_ordering_info_present_flag, 1, sps_sub_layer_ordering_info_present_flag ? "specifies that sps_max_dec_pic_buffering_minus1[i], sps_max_num_reorder_pics[i] and sps_max_latency_increase_plus1[i] are present for sps_max_sub_layers_minus1 + 1 sub-layers."
						: "specifies that the values of sps_max_dec_pic_buffering_minus1[sps_max_sub_layers_minus1], sps_max_num_reorder_pics[sps_max_sub_layers_minus1] and sps_max_latency_increase_plus1[sps_max_sub_layers_minus1] apply to all sub-layers.");
					NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "for(i=(sps_sub_layer_ordering_info_present_flag?0:sps_max_sub_layers_minus1);i&lt;=sps_max_sub_layers_minus1; i++ )", "");
					for (i = (sps_sub_layer_ordering_info_present_flag ? 0 : sps_max_sub_layers_minus1); i <= sps_max_sub_layers_minus1; i++)
					{
						BST_ARRAY_FIELD_PROP_NUMBER("sps_max_dec_pic_buffering_minus1", i, (long long)quick_log2(sps_ordering_info[i].sps_max_dec_pic_buffering_minus1)*2 + 1, sps_ordering_info[i].sps_max_dec_pic_buffering_minus1,
							"plus 1 specifies the maximum required size of the decoded picture buffer for the CVS in units of picture storage buffers when HighestTid is equal to i.");
						BST_ARRAY_FIELD_PROP_NUMBER("sps_max_num_reorder_pics", i, (long long)quick_log2(sps_ordering_info[i].sps_max_num_reorder_pics) * 2 + 1, sps_ordering_info[i].sps_max_num_reorder_pics,
							"indicates the maximum allowed number of pictures with PicOutputFlag equal to 1");
						BST_ARRAY_FIELD_PROP_NUMBER("sps_max_latency_increase_plus1", i, (long long)quick_log2(sps_ordering_info[i].sps_max_latency_increase_plus1) * 2 + 1, sps_ordering_info[i].sps_max_latency_increase_plus1, 
							sps_ordering_info[i].sps_max_latency_increase_plus1?"SpsMaxLatencyPictures[i] = sps_max_num_reorder_pics[i] + sps_max_latency_increase_plus1[i] - 1":"SpsMaxLatencyPictures[i] = sps_max_num_reorder_pics[i]");
					}
					NAV_WRITE_TAG_END("Tag0");

					BST_FIELD_PROP_2NUMBER1(log2_min_luma_coding_block_size_minus3, (long long)quick_log2(log2_min_luma_coding_block_size_minus3 + 1) * 2 + 1, "");
					BST_FIELD_PROP_2NUMBER1(log2_diff_max_min_luma_coding_block_size, (long long)quick_log2(log2_diff_max_min_luma_coding_block_size + 1) * 2 + 1, "");
					BST_FIELD_PROP_2NUMBER1(log2_min_luma_transform_block_size_minus2, (long long)quick_log2(log2_min_luma_transform_block_size_minus2 + 1) * 2 + 1, "");
					BST_FIELD_PROP_2NUMBER1(log2_diff_max_min_luma_transform_block_size, (long long)quick_log2(log2_diff_max_min_luma_transform_block_size + 1) * 2 + 1, "");
					BST_FIELD_PROP_2NUMBER1(max_transform_hierarchy_depth_inter, (long long)quick_log2(max_transform_hierarchy_depth_inter + 1) * 2 + 1, "");
					BST_FIELD_PROP_2NUMBER1(max_transform_hierarchy_depth_intra, (long long)quick_log2(max_transform_hierarchy_depth_intra + 1) * 2 + 1, "");

					uint32_t MaxDpbSize;
					uint32_t maxDpbPicBuf = 6;
					uint32_t MaxLumaPs = get_hevc_tier_and_level_limit(profile_tier_level->general_profile_level.level_idc).MaxLumaPs;

					if (MaxLumaPs == (uint32_t)-1)
						MaxDpbSize = maxDpbPicBuf;
					else if (PicSizeInSamplesY <= (MaxLumaPs >> 2))
						MaxDpbSize = AMP_MIN(4*maxDpbPicBuf, 16);
					else if (PicSizeInSamplesY <= (MaxLumaPs >> 1))
						MaxDpbSize = AMP_MIN(2*maxDpbPicBuf, 16);
					else if (PicSizeInSamplesY <= ((3 * MaxLumaPs) >> 2))
						MaxDpbSize = AMP_MIN((4*maxDpbPicBuf) / 3, 16);
					else
						MaxDpbSize = maxDpbPicBuf;

					NAV_WRITE_TAG_WITH_NUMBER_VALUE1(MinCbLog2SizeY, "MinCbLog2SizeY = log2_min_luma_coding_block_size_minus3+3, the minimum luma coding block size.");
					NAV_WRITE_TAG_WITH_NUMBER_VALUE1(CtbLog2SizeY, "CtbLog2SizeY = MinCbLog2SizeY+log2_diff_max_min_luma_coding_block_size, the difference between the maximum and minimum luma coding block size");
					NAV_WRITE_TAG_WITH_NUMBER_VALUE1(MinCbSizeY, "MinCbSizeY = 1 &lt;&lt; MinCbLog2SizeY");
					NAV_WRITE_TAG_WITH_NUMBER_VALUE1(CtbSizeY, "CtbSizeY= 1 &lt;&lt; CtbLog2SizeY");
					NAV_WRITE_TAG_WITH_NUMBER_VALUE1(PicWidthInMinCbsY, "PicWidthInMinCbsY = pic_width_in_luma_samples / MinCbSizeY");
					NAV_WRITE_TAG_WITH_NUMBER_VALUE1(PicWidthInCtbsY, "PicWidthInCtbsY = Ceil( pic_width_in_luma_samples / CtbSizeY )");
					NAV_WRITE_TAG_WITH_NUMBER_VALUE1(PicHeightInMinCbsY, "PicHeightInMinCbsY = pic_height_in_luma_samples / MinCbSizeY");
					NAV_WRITE_TAG_WITH_NUMBER_VALUE1(PicHeightInCtbsY, "PicHeightInCtbsY = Ceil( pic_height_in_luma_samples / CtbSizeY )");
					NAV_WRITE_TAG_WITH_NUMBER_VALUE1(PicSizeInMinCbsY, "PicSizeInCtbsY = PicWidthInMinCbsY * PicHeightInMinCbsY");
					NAV_WRITE_TAG_WITH_NUMBER_VALUE1(PicSizeInCtbsY, "PicWidthInCtbsY * PicHeightInCtbsY");
					NAV_WRITE_TAG_WITH_NUMBER_VALUE1(PicSizeInSamplesY, "PicSizeInSamplesY = pic_width_in_luma_samples * pic_height_in_luma_samples");
					NAV_WRITE_TAG_WITH_NUMBER_VALUE1(PicWidthInSamplesC, "PicWidthInSamplesC = pic_width_in_luma_samples / SubWidthC");
					NAV_WRITE_TAG_WITH_NUMBER_VALUE1(PicHeightInSamplesC, "PicHeightInSamplesC = pic_height_in_luma_samples / SubHeightC");
					NAV_WRITE_TAG_WITH_NUMBER_VALUE1(CtbWidthC, "CtbWidthC = CtbSizeY / SubWidthC");
					NAV_WRITE_TAG_WITH_NUMBER_VALUE1(CtbHeightC, "CtbHeightC = CtbSizeY / SubHeightC");
					NAV_WRITE_TAG_WITH_NUMBER_VALUE1(MaxDpbSize, "Maximum Decoded picture buffer size");

					BST_FIELD_PROP_NUMBER1(scaling_list_enabled_flag, 1, scaling_list_enabled_flag ? "specifies that a scaling list is used for the scaling process for transform coefficients."
						: "specifies that scaling list is not used for the scaling process for transform coefficients.");

					if (scaling_list_enabled_flag)
					{
						BST_FIELD_PROP_NUMBER1(sps_scaling_list_data_present_flag, 1, sps_scaling_list_data_present_flag ? "specifies that the scaling_list_data( ) syntax structure is present."
							: "specifies that the scaling_list_data( ) syntax structure is not present.");
						if (sps_scaling_list_data_present_flag)
						{
							BST_FIELD_PROP_REF1(scaling_list_data);
						}
					}

					BST_FIELD_PROP_NUMBER1(amp_enabled_flag, 1, amp_enabled_flag ? "specifies that asymmetric motion partitions may be used in coding tree blocks."
						: "specifies that asymmetric motion partitions cannot be used in coding tree blocks.");
					BST_FIELD_PROP_NUMBER1(sample_adaptive_offset_enabled_flag, 1, sample_adaptive_offset_enabled_flag ? "specifies that the sample adaptive offset process is applied to the reconstructed picture after the deblocking filter process."
						: "specifies that the sample adaptive offset process is not applied to the reconstructed picture after the deblocking filter process.");
					BST_FIELD_PROP_NUMBER1(pcm_enabled_flag, 1, "");

					if (pcm_enabled_flag)
					{
						BST_FIELD_PROP_NUMBER1(pcm_sample_bit_depth_luma_minus1, 4, "the number of bits used to represent each of PCM sample values of the luma component");
						BST_FIELD_PROP_NUMBER1(pcm_sample_bit_depth_chroma_minus1, 4, "specifies the number of bits used to represent each of PCM sample values of the chroma components");
						BST_FIELD_PROP_NUMBER1(log2_min_pcm_luma_coding_block_size_minus3, (long long)quick_log2(log2_min_pcm_luma_coding_block_size_minus3 + 1)*2 + 1, 
							"plus 3 specifies the minimum size of coding blocks");
						BST_FIELD_PROP_NUMBER1(log2_diff_max_min_pcm_luma_coding_block_size, (long long)quick_log2(log2_diff_max_min_pcm_luma_coding_block_size + 1) * 2 + 1,
							"specifies the difference between the maximum and minimum size of coding blocks");
						BST_FIELD_PROP_NUMBER1(pcm_loop_filter_disabled_flag, 1, "specifies whether the loop filter process is disabled on reconstructed samples in a coding unit");
					}

					BST_FIELD_PROP_NUMBER1(num_short_term_ref_pic_sets, (long long)quick_log2(num_short_term_ref_pic_sets + 1) * 2 + 1, "the number of st_ref_pic_set( ) syntax structures included in the SPS");
					if (num_short_term_ref_pic_sets > 0)
					{
						BST_FIELD_PROP_REF(st_ref_pic_sets);
					}

					BST_FIELD_PROP_NUMBER1(long_term_ref_pics_present_flag, 1, long_term_ref_pics_present_flag?"specifies that long-term reference pictures may be used for inter prediction of one or more coded pictures in the CVS"
						:"no long-term reference picture is used for inter prediction of any coded picture in the CVS.");
					if (long_term_ref_pics_present_flag)
					{
						BST_FIELD_PROP_2NUMBER1(num_long_term_ref_pics_sps, (long long)quick_log2(num_long_term_ref_pics_sps + 1) * 2 + 1, "the number of candidate long-term reference pictures that are specified in the SPS");
						NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag1", "for(i=0;i&lt;num_long_term_ref_pics_sps;i++)", "");
						for (i = 0; i < num_long_term_ref_pics_sps; i++)
						{
							BST_ARRAY_FIELD_PROP_NUMBER("lt_ref_pic_poc_lsb_sps", i, (long long)quick_log2(lt_ref_pic_poc_lsb_sps[i] + 1) * 2 + 1, lt_ref_pic_poc_lsb_sps[i], "");
							BST_ARRAY_FIELD_PROP_NUMBER("used_by_curr_pic_lt_sps_flag", i, 1, used_by_curr_pic_lt_sps_flag[i], "equal to 0 specifies that the i-th candidate long-term reference picture specified in the SPS is not used for reference by a picture that includes in its long-term reference picture set (RPS) the i-th candidate long-term reference picture specified in the SPS.");
						}
						NAV_WRITE_TAG_END("Tag1");
					}

					BST_FIELD_PROP_NUMBER1(sps_temporal_mvp_enabled_flag, 1, sps_temporal_mvp_enabled_flag ? "specifies that slice_temporal_mvp_enabled_flag is present in the slice headers of non-IDR pictures in the CVS."
						: "specifies that slice_temporal_mvp_enabled_flag is not present in slice headers and that temporal motion vector predictors are not used in the CVS.");
					BST_FIELD_PROP_NUMBER1(strong_intra_smoothing_enabled_flag, 1, strong_intra_smoothing_enabled_flag ? "specifies that bi-linear interpolation is conditionally used in the intra prediction filtering process in the CVS as specified in clause 8.4.4.2.3."
						: "specifies that the bi-linear interpolation is not used in the CVS.");
					BST_FIELD_PROP_NUMBER1(vui_parameters_present_flag, 1, vui_parameters_present_flag ? "specifies that the vui_parameters( ) syntax structure as specified in Annex E is present."
						: "specifies that the vui_parameters( ) syntax structure as specified in Annex E is not present.");

					if (vui_parameters_present_flag)
					{
						BST_FIELD_PROP_REF1(vui_parameters);
					}

					BST_FIELD_PROP_NUMBER1(sps_extension_present_flag, 1, 
						sps_extension_present_flag ? "the syntax elements sps_range_extension_flag, sps_multilayer_extension_flag, sps_3d_extension_flag, sps_scc_extension_flag, and sps_extension_4bits are present."
													: "the syntax elements sps_range_extension_flag, sps_multilayer_extension_flag, sps_3d_extension_flag, sps_scc_extension_flag, and sps_extension_4bits are NOT present.");
					if (vui_parameters_present_flag)
					{
						BST_FIELD_PROP_NUMBER1(sps_range_extension_flag, 1, sps_range_extension_flag ? "specifies that the syntax elements sps_range_extension_flag, sps_multilayer_extension_flag, sps_3d_extension_flag and sps_extension_5bits are present in the SPS RBSP syntax structure."
							: "specifies that these syntax elements are not present.");
						BST_FIELD_PROP_NUMBER1(sps_multilayer_extension_flag, 1, sps_multilayer_extension_flag ? "specifies that the sps_range_extension( ) syntax structure is present in the SPS RBSP syntax structure. sps_range_extension_flag equal to 0 specifies that this syntax structure is not present."
							: "specifies that this syntax structure is not present.");
						BST_FIELD_PROP_NUMBER1(sps_3d_extension_flag, 1, sps_3d_extension_flag ? ""
							: "specifies that no sps_extension_data_flag syntax elements are present.");
						BST_FIELD_PROP_NUMBER1(sps_scc_extension_flag, 1, sps_scc_extension_flag ? "the sps_scc_extension() syntax structure is present."
							: "the sps_scc_extension() syntax structure is NOT present.");
						BST_FIELD_PROP_2NUMBER1(sps_extension_4bits, 4, "reserved for future use by ITU-T | ISO/IEC.");
					}

					if (sps_range_extension_flag)
					{
						NAV_WRITE_TAG_BEGIN("sps_range_extension");
						BST_FIELD_PROP_BOOL1(sps_range_extension, transform_skip_rotation_enabled_flag, "specifies that a rotation is applied to the residual data block for intra 4x4 blocks coded using a transform skip operation", "specifies that this rotation is not applied.");
						BST_FIELD_PROP_BOOL1(sps_range_extension, transform_skip_context_enabled_flag, "specifies that a particular context is used for the parsing of the sig_coeff_flag for transform blocks with a skipped transform", "specifies that the presence or absence of transform skipping or a transform bypass for transform blocks is not used in the context selection for this flag");
						BST_FIELD_PROP_BOOL1(sps_range_extension, implicit_rdpcm_enabled_flag, "specifies that the residual modification process for blocks using a transform bypass may be used for intra blocks in the CVS", "specifies that the residual modification process is not used for intra blocks in the CVS");
						BST_FIELD_PROP_BOOL1(sps_range_extension, explicit_rdpcm_enabled_flag, "specifies that the residual modification process for blocks using a transform bypass may be used for inter blocks in the CVS", "specifies that the residual modification process is not used for inter blocks in the CVS");
						BST_FIELD_PROP_BOOL1(sps_range_extension, extended_precision_processing_flag, "specifies that an extended dynamic range is used for coefficient parsing and inverse transform processing", "specifies that the extended dynamic range is not used");
						BST_FIELD_PROP_BOOL1(sps_range_extension, intra_smoothing_disabled_flag, "specifies that the filtering process of neighboring samples is unconditionally disabled for intra prediction", "specifies that the filtering process of neighboring samples is not disabled");
						BST_FIELD_PROP_BOOL1(sps_range_extension, high_precision_offsets_enabled_flag, "specifies that weighted prediction offset values are signalled using a bit-depth-dependent precision", "specifies that weighted prediction offset values are signalled with a precision equivalent to eight bit processing");
						BST_FIELD_PROP_BOOL1(sps_range_extension, persistent_rice_adaptation_enabled_flag, "specifies that the Rice parameter derivation for the binarization of coeff_abs_level_remaining[ ] is initialized at the start of each sub-block using mode dependent statistics accumulated from previous sub-blocks.", "specifies that no previous sub-block state is used in Rice parameter derivation");
						BST_FIELD_PROP_BOOL1(sps_range_extension, cabac_bypass_alignment_enabled_flag, "specifies that a context-based adaptive binary arithmetic coding (CABAC) alignment process is used prior to bypass decoding of the syntax elements coeff_sign_flag[ ] and coeff_abs_level_remaining[ ]", "specifies that no CABAC alignment process is used prior to bypass decoding");
						NAV_WRITE_TAG_END("sps_range_extension");
					}

					if (sps_multilayer_extension_flag)
					{
						NAV_WRITE_TAG_BEGIN("sps_multiplayer_extension_flag");
						BST_FIELD_PROP_BOOL1(sps_multilayer_extension, inter_view_mv_vert_constraint_flag, "indicates that vertical component of motion vectors used for inter-layer prediction are constrained in the layers for which this SPS RBSP is the active SPS RBSP", "no constraint on the vertical component of the motion vectors used for inter-layer prediction is signalled by this flag");
						NAV_WRITE_TAG_END("sps_multiplayer_extension_flag");
					}

					if (sps_3d_extension_flag)
					{
						NAV_WRITE_TAG_BEGIN("sps_3d_extension");
						BST_ARRAY_FIELD_PROP_NUMBER("iv_di_mc_enabled_flag", 0, 1, sps_3d_extension.iv_di_mc_enabled_flag0, 
							sps_3d_extension.iv_di_mc_enabled_flag0?"specifies that the derivation process for inter-view predicted merging candidates and the derivation process for disparity information merging candidates may be used in the decoding process of layers with DepthFlag equal to 0.":
							"specifies that derivation process for inter-view predicted merging candidates and the derivation process for disparity information merging candidates is not used in the decoding process of layers with DepthFlag equal to 0");
						BST_ARRAY_FIELD_PROP_NUMBER("iv_mv_scal_enabled_flag", 0, 1, sps_3d_extension.iv_mv_scal_enabled_flag0, 
							sps_3d_extension.iv_mv_scal_enabled_flag0?"specifies that motion vectors used for inter-view prediction may be scaled based on view_id_val values in the decoding process of layers with DepthFlag equal to 0"
							:"specifies that motion vectors used for inter-view prediction are not scaled based on view_id_val values in the decoding process of layers with DepthFlag equal to 0");

						BST_FIELD_PROP_2NUMBER("log2_ivmc_sub_pb_size_minus3", (long long)quick_log2(sps_3d_extension.log2_ivmc_sub_pb_size_minus3 + 1) * 2 + 1, 
							sps_3d_extension.log2_ivmc_sub_pb_size_minus3, sps_3d_extension.iv_di_mc_enabled_flag0?
							"derive the minimum size of sub-block partitions used in the derivation process for sub-block partition motion vectors for an inter-layer predicted merging candidate in the decoding process of layers with DepthFlag equal to 0":
							"");
						BST_FIELD_PROP_BOOL1(sps_3d_extension, iv_res_pred_enabled_flag, "specifies that the iv_res_pred_weight_idx syntax element may be present in coding units of layers with DepthFlag equal to 0", 
							"specifies that the iv_res_pred_weight_idx syntax element is not present coding units of layers with DepthFlag equal to 0");
						BST_FIELD_PROP_BOOL1(sps_3d_extension, depth_ref_enabled_flag, "specifies that the derivation process for a view synthesis prediction merging candidate may be used in the decoding process of layers with DepthFlag equal to 0", 
							"specifies that the derivation process for a view synthesis prediction merging candidate is not used the decoding process of layers with DepthFlag equal to 0");
						BST_FIELD_PROP_BOOL1(sps_3d_extension, vsp_mc_enabled_flag, "specifies that the dbbp_flag syntax element may be present in coding units of layers with DepthFlag equal to 0", 
							"specifies that the dbbp_flag syntax element is not present coding units of layers with DepthFlag equal to 0");
						BST_FIELD_PROP_BOOL1(sps_3d_extension, dbbp_enabled_flag, "specifies that the derivation process for a depth or disparity sample array from a depth picture may be used in the derivation process for a disparity vector for texture layers in the decoding process of layers with DepthFlag equal to 0", 
							"specifies that derivation process for a depth or disparity sample array from a depth picture is not used in the derivation process for a disparity vector for texture layers in the decoding process of layers with DepthFlag equal to 0");

						BST_ARRAY_FIELD_PROP_NUMBER("iv_di_mc_enabled_flag", 1, 1, sps_3d_extension.iv_di_mc_enabled_flag1, 
							sps_3d_extension.iv_di_mc_enabled_flag1 ? "specifies that the derivation process for inter-view predicted merging candidates and the derivation process for disparity information merging candidates may be used in the decoding process of layers with DepthFlag equal to 1.": 
							"specifies that derivation process for inter-view predicted merging candidates and the derivation process for disparity information merging candidates is not used in the decoding process of layers with DepthFlag equal to 1");
						BST_ARRAY_FIELD_PROP_NUMBER("iv_mv_scal_enabled_flag", 1, 1, sps_3d_extension.iv_mv_scal_enabled_flag1, 
							sps_3d_extension.iv_mv_scal_enabled_flag1 ? "specifies that motion vectors used for inter-view prediction may be scaled based on view_id_val values in the decoding process of layers with DepthFlag equal to 1":
							"specifies that motion vectors used for inter-view prediction are not scaled based on view_id_val values in the decoding process of layers with DepthFlag equal to 1");

						BST_FIELD_PROP_BOOL1(sps_3d_extension, tex_mc_enabled_flag, "specifies that the derivation process for motion vectors for the texture merge candidate may be used in the decoding process of layers with DepthFlag equal to 1", 
							"specifies that the derivation process for motion vectors for the texture merge candidate is not used in the decoding process of layers with DepthFlag equal to 1");
						BST_FIELD_PROP_2NUMBER("log2_texmc_sub_pb_size_minus3", (long long)quick_log2(sps_3d_extension.log2_texmc_sub_pb_size_minus3 + 1) * 2 + 1, sps_3d_extension.log2_texmc_sub_pb_size_minus3, sps_3d_extension.tex_mc_enabled_flag?
							"derive the minimum size of sub-block partitions used in the derivation process for sub-block partition motion vectors for an inter-layer predicted merging candidate in the decoding process of layers with DepthFlag equal to 1":"");
						BST_FIELD_PROP_BOOL1(sps_3d_extension, intra_contour_enabled_flag, "specifies that the intra prediction mode INTRA_CONTOUR using depth intra contour prediction may be used in the decoding process of layers with DepthFlag equal to 1", 
							"specifies that the intra prediction mode INTRA_CONTOUR using depth intra contour prediction is not used in the decoding process of layers with DepthFlag equal to 1");
						BST_FIELD_PROP_BOOL1(sps_3d_extension, intra_dc_only_wedge_enabled_flag, "specifies that the dc_only_flag syntax element may be present in coding units coded in an intra prediction mode of layers with DepthFlag equal to 1, and that the intra prediction mode INTRA_WEDGE may be used in the decoding process of layers with DepthFlag equal to 1", 
							"specifies that the dc_only_flag syntax element is not present in coding units coded in an intra prediction mode of layers with DepthFlag equal to 1 and that the intra prediction mode INTRA_WEDGE is not used in the decoding process of layers with DepthFlag equal to 1");
						BST_FIELD_PROP_BOOL1(sps_3d_extension, cqt_cu_part_pred_enabled_flag, "specifies that coding quad-tree and coding unit partitioning information may be inter-component predicted in the decoding process of layers with DepthFlag equal to 1", 
							"specifies that coding quad-tree and coding unit partitioning information are not inter-component predicted in the decoding process of layers with DepthFlag equal to 1");
						BST_FIELD_PROP_BOOL1(sps_3d_extension, inter_dc_only_enabled_flag, "the dc_only_flag syntax element may be present in coding units coded an in inter prediction mode of layers with DepthFlag equal to 1", 
							"specifies that the dc_only_flag syntax element is not present in coding units coded in an inter prediction mode of layers with DepthFlag equal to 1");
						BST_FIELD_PROP_BOOL1(sps_3d_extension, skip_intra_enabled_flag, "specifies that the skip_intra_flag syntax element may be present in coding units of layers with DepthFlag equal to 1", 
							"specifies that the skip_intra_flag syntax element is not present in coding units of layers with DepthFlag equal to 1");

						NAV_WRITE_TAG_END("sps_3d_extension");
					}

					BST_FIELD_PROP_OBJECT(rbsp_trailing_bits)

				DECLARE_FIELDPROP_END()

			};

			struct ACCESS_UNIT_DELIMITER_RBSP : public SYNTAX_BITSTREAM_MAP
			{
				uint8_t					pic_type = 0;
				RBSP_TRAILING_BITS		rbsp_trailing_bits;

				int Map(AMBst in_bst)
				{
					int iRet = RET_CODE_SUCCESS;
					SYNTAX_BITSTREAM_MAP::Map(in_bst);

					try
					{
						MAP_BST_BEGIN(0);
						nal_read_u(in_bst, pic_type, 3, uint8_t);
						nal_read_obj(in_bst, rbsp_trailing_bits);
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
					BST_FIELD_PROP_2NUMBER1(pic_type, 3, pic_type==0?"I":(pic_type==1?"P, I":(pic_type==2?"B, P, I":"Unknown")))
					BST_FIELD_PROP_OBJECT(rbsp_trailing_bits)
				DECLARE_FIELDPROP_END()

			};

			struct PIC_PARAMETER_SET_RBSP : public SYNTAX_BITSTREAM_MAP
			{
				struct PPS_RANGE_EXTENSION : public SYNTAX_BITSTREAM_MAP
				{
					uint8_t		log2_max_transform_skip_block_size_minus2 = 0;
				
					uint8_t		cross_component_prediction_enabled_flag : 1;
					uint8_t		chroma_qp_offset_list_enabled_flag : 1;
					uint8_t		reserved_0 : 6;

					uint8_t		diff_cu_chroma_qp_offset_depth = 0;
					uint8_t		chroma_qp_offset_list_len_minus1 = 0;

					int8_t		cb_qp_offset_list[6] = { 0 };
					int8_t		cr_qp_offset_list[6] = { 0 };

					uint8_t		log2_sao_offset_scale_luma = 0;
					uint8_t		log2_sao_offset_scale_chroma = 0;

					PIC_PARAMETER_SET_RBSP*
								pic_parameter_set_rbsp;

					PPS_RANGE_EXTENSION(PIC_PARAMETER_SET_RBSP* ptr_pic_parameter_set_rbsp) 
						: cross_component_prediction_enabled_flag(0)
						, chroma_qp_offset_list_enabled_flag(0)
						, reserved_0(0)
						, pic_parameter_set_rbsp(ptr_pic_parameter_set_rbsp) {
					}

					int Map(AMBst in_bst)
					{
						int iRet = RET_CODE_SUCCESS;
						SYNTAX_BITSTREAM_MAP::Map(in_bst);

						try
						{
							MAP_BST_BEGIN(0);
							if (pic_parameter_set_rbsp->transform_skip_enabled_flag) {
								nal_read_ue(in_bst, log2_max_transform_skip_block_size_minus2, uint8_t);
							}

							nal_read_u(in_bst, cross_component_prediction_enabled_flag, 1, uint8_t);
							nal_read_u(in_bst, chroma_qp_offset_list_enabled_flag, 1, uint8_t);

							if (chroma_qp_offset_list_enabled_flag) {
								nal_read_ue(in_bst, diff_cu_chroma_qp_offset_depth, uint8_t);
								nal_read_ue(in_bst, chroma_qp_offset_list_len_minus1, uint8_t);

								for (int i = 0; i <= chroma_qp_offset_list_len_minus1; i++) {
									nal_read_se(in_bst, cb_qp_offset_list[i], int8_t);
									nal_read_se(in_bst, cr_qp_offset_list[i], int8_t);
								}
							}

							nal_read_ue(in_bst, log2_sao_offset_scale_luma, uint8_t);
							nal_read_ue(in_bst, log2_sao_offset_scale_chroma, uint8_t);

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
						if (pic_parameter_set_rbsp->transform_skip_enabled_flag) {
							BST_FIELD_PROP_UE(log2_max_transform_skip_block_size_minus2, "plus 2 specifies the maximum transform block size for which transform_skip_flag may be present in coded pictures referring to the PPS");
						}

						BST_FIELD_PROP_BOOL(cross_component_prediction_enabled_flag, "specifies that log2_res_scale_abs_plus1 and res_scale_sign_flag may be present in the transform unit syntax for pictures referring to the PPS", 
							"specifies that log2_res_scale_abs_plus1 and res_scale_sign_flag are not present for pictures referring to the PPS");
						BST_FIELD_PROP_BOOL(chroma_qp_offset_list_enabled_flag, "specifies that the cu_chroma_qp_offset_flag may be present in the transform unit syntax", 
							"specifies that the cu_chroma_qp_offset_flag is not present in the transform unit syntax");

						if (chroma_qp_offset_list_enabled_flag) {
							NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "if( chroma_qp_offset_list_enabled_flag )", "");
							BST_FIELD_PROP_UE(diff_cu_chroma_qp_offset_depth, "specifies the difference between the luma coding tree block size and the minimum luma coding block size of coding units that convey cu_chroma_qp_offset_flag");
							BST_FIELD_PROP_UE(chroma_qp_offset_list_len_minus1, "plus 1 specifies the number of cb_qp_offset_list[i] and cr_qp_offset_list[i] syntax elements that are present in the PPS");
							NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag00", "for( i = 0; i &lt;= chroma_qp_offset_list_len_minus1; i++ )", "");
							for (int i = 0; i < chroma_qp_offset_list_len_minus1; i++) {
								BST_ARRAY_FIELD_PROP_SE(cb_qp_offset_list, i, "offsets used in the derivation of Qp'Cb");
								BST_ARRAY_FIELD_PROP_SE(cr_qp_offset_list, i, "offsets used in the derivation of Qp'Cr");
							}
							NAV_WRITE_TAG_END("Tag00");
							NAV_WRITE_TAG_END("Tag0");
						}

						BST_FIELD_PROP_UE(log2_sao_offset_scale_luma, "the base 2 logarithm of the scaling parameter that is used to scale sample adaptive offset (SAO) offset values for luma samples");
						BST_FIELD_PROP_UE(log2_sao_offset_scale_chroma, "the base 2 logarithm of the scaling parameter that is used to scale SAO offset values for chroma samples");
					DECLARE_FIELDPROP_END()
				};

				struct PPS_MULTILAYER_EXTENSION : public SYNTAX_BITSTREAM_MAP
				{
					struct COLOUR_MAPPING_TABLE : public SYNTAX_BITSTREAM_MAP
					{
						struct COLOUR_MAPPING_OCTANTS : public SYNTAX_BITSTREAM_MAP
						{
							uint8_t		split_octant_flag;

							COLOUR_MAPPING_OCTANTS*
								colour_mapping_octants[2][2][2];

							CAMBitArray	coded_res_flag;
							uint32_t	res_coeff_q[8][4][3];
							uint32_t	res_coeff_r[8][4][3];
							CAMBitArray	res_coeff_s;

							uint8_t		m_inpDepth;
							uint16_t	m_idxY;
							uint16_t	m_idxCb;
							uint16_t	m_idxCr;
							uint16_t	m_inpLength;
							COLOUR_MAPPING_TABLE*
										ptr_colour_mapping_table;

							COLOUR_MAPPING_OCTANTS(COLOUR_MAPPING_TABLE* pColourMappingTable, uint8_t inpDepth, uint16_t idxY, uint16_t idxCb, uint16_t idxCr, uint16_t inpLength)
								: split_octant_flag(0)
								, m_inpDepth(inpDepth)
								, m_idxY(idxY)
								, m_idxCb(idxCb)
								, m_idxCr(idxCr)
								, m_inpLength(inpLength)
								, ptr_colour_mapping_table(pColourMappingTable) {
								memset(colour_mapping_octants, 0, sizeof(colour_mapping_octants));
							}

							virtual ~COLOUR_MAPPING_OCTANTS() {
								for (int k = 0; k < 2; k++)
									for (int m = 0; m < 2; m++)
										for (int n = 0; n < 2; n++) {
											UNMAP_STRUCT_POINTER5(colour_mapping_octants[k][m][n]);
										}
							}

							int Map(AMBst in_bst)
							{
								int iRet = RET_CODE_SUCCESS;
								SYNTAX_BITSTREAM_MAP::Map(in_bst);

								try
								{
									MAP_BST_BEGIN(0);
									if (m_inpDepth < ptr_colour_mapping_table->cm_octant_depth) {
										nal_read_u(in_bst, split_octant_flag, 1, uint8_t);
									}

									uint8_t BitDepthCmInputY = 8 + ptr_colour_mapping_table->chroma_bit_depth_cm_input_minus8;
									uint8_t BitDepthCmOutputY = 8 + ptr_colour_mapping_table->chroma_bit_depth_cm_output_minus8;
									uint8_t PartNumY = 1 << ptr_colour_mapping_table->cm_y_part_num_log2;
									uint8_t CMResLSBits = (uint8_t)AMP_MAX(0, (int)(10 - BitDepthCmInputY - BitDepthCmOutputY - ptr_colour_mapping_table->cm_res_quant_bits - (ptr_colour_mapping_table->cm_delta_flc_bits_minus1 + 1)));

									if (split_octant_flag)
									{
										for (int k = 0; k < 2; k++)
											for (int m = 0; m < 2; m++)
												for (int n = 0; n < 2; n++) {
													nal_read_ref(in_bst, colour_mapping_octants[k][m][n], COLOUR_MAPPING_OCTANTS, ptr_colour_mapping_table, 
														m_inpDepth + 1, m_idxY + PartNumY*k*m_inpLength / 2,
														m_idxCb + m*m_inpLength / 2, m_idxCr + n*m_inpLength / 2, m_inpLength / 2);
												}
									}
									else
									{
										for (int i = 0; i < PartNumY; i++)
										{
											uint8_t idxShiftY = m_idxY + ((ptr_colour_mapping_table->cm_octant_depth - m_inpDepth));
											for (int j = 0; j < 4; j++)
											{
												uint8_t b_coded_res_flag;
												nal_read_u(in_bst, b_coded_res_flag, 1, uint8_t);
												b_coded_res_flag ? coded_res_flag.BitSet(i*PartNumY + j) : coded_res_flag.BitClear(i*PartNumY + j);
												for (int c = 0; b_coded_res_flag && c < 3; c++)
												{
													nal_read_ue(in_bst, res_coeff_q[i][j][c], uint32_t);
													if (CMResLSBits > 0)
													{
														nal_read_u(in_bst, res_coeff_r[i][j][c], CMResLSBits, uint32_t);
													}
													else
														res_coeff_r[i][j][c] = 0;

													if (res_coeff_q[i][j][c] || res_coeff_r[i][j][c]) {
														nal_read_bitarray(in_bst, res_coeff_s, i*PartNumY + j * 4 + c);
													}
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

							int Unmap(AMBst out_bst)
							{
								UNREFERENCED_PARAMETER(out_bst);
								return RET_CODE_ERROR_NOTIMPL;
							}

							DECLARE_FIELDPROP_BEGIN()
								if (m_inpDepth < ptr_colour_mapping_table->cm_octant_depth) {
									BST_FIELD_PROP_BOOL(split_octant_flag, "specifies that the current colour mapping octant is further split into eight octants with half length in each of the three dimensions",
										"specifies that the current colour mapping octant is not further split into eight octants");

									uint8_t BitDepthCmInputY = 8 + ptr_colour_mapping_table->chroma_bit_depth_cm_input_minus8;
									uint8_t BitDepthCmOutputY = 8 + ptr_colour_mapping_table->chroma_bit_depth_cm_output_minus8;
									uint8_t PartNumY = 1 << ptr_colour_mapping_table->cm_y_part_num_log2;
									uint8_t CMResLSBits = (uint8_t)AMP_MAX(0, (int)(10 - BitDepthCmInputY - BitDepthCmOutputY - ptr_colour_mapping_table->cm_res_quant_bits - (ptr_colour_mapping_table->cm_delta_flc_bits_minus1 + 1)));

									if (split_octant_flag)
									{
										NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "for( k = 0; k &lt; 2; k++ )", "");
										NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag00", "for( m = 0; m &lt; 2; m++ )", "");
										NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag000", "for( n = 0; n &lt; 2; n++ )", "");
										for (int k = 0; k < 2; k++)
											for (int m = 0; m < 2; m++)
												for (int n = 0; n < 2; n++) {
													MBCSPRINTF_S(szTemp4, TEMP4_SIZE, "colour_mapping_octants(%d, %d, %d, %d, %d)",
														m_inpDepth + 1, m_idxY + PartNumY*k*m_inpLength / 2, m_idxCb + m*m_inpLength / 2, m_idxCr + n*m_inpLength / 2, m_inpLength / 2);
													NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0000", szTemp4, "");
													BST_FIELD_PROP_REF(colour_mapping_octants[k][m][n]);
													NAV_WRITE_TAG_END("Tag0000");
												}
										NAV_WRITE_TAG_END("Tag000");
										NAV_WRITE_TAG_END("Tag00");
										NAV_WRITE_TAG_END("Tag0");
									}
									else
									{
										NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "for( i = 0; i &lt; PartNumY; i++ )", "");
										for (int i = 0; i < PartNumY; i++)
										{
											uint8_t idxShiftY = m_idxY + ((ptr_colour_mapping_table->cm_octant_depth - m_inpDepth));
											NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("idxShiftY", "idxShiftY = idxY + ((i &lt;&lt; (cm_octant_depth - inpDepth))", idxShiftY, "");
											NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag00", "for (int j = 0; j &lt; 4; j++)", "");
											for (int j = 0; j < 4; j++)
											{
												MBCSPRINTF_S(szTemp2, TEMP2_SIZE, "coded_res_flag[%d][%d][%d][%d]", idxShiftY, m_idxCb, m_idxCr, j);
												BST_FIELD_PROP_NUMBER(szTemp2, 1, coded_res_flag[i*PartNumY + j], "");
												if (coded_res_flag[i*PartNumY + j]) {
													NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag000", "for( c = 0; c &lt; 3; c++ )", "");
													for (int c = 0; c < 3; c++)
													{
														MBCSPRINTF_S(szTemp2, TEMP2_SIZE, "res_coeff_q[%d][%d][%d][%d][%d]", idxShiftY, m_idxCb, m_idxCr, j, c);
														BST_FIELD_PROP_2NUMBER(szTemp2, (long long)quick_log2(res_coeff_q[i][j][c] + 1) * 2 + 1, res_coeff_q[i][j][c], "specifies the quotient of the residual for the j-th colour mapping coefficient of the c-th colour component of the octant with octant index equal to (idxShiftY, idxCb, idxCr)");
														if (CMResLSBits > 0)
														{
															MBCSPRINTF_S(szTemp2, TEMP2_SIZE, "res_coeff_r[%d][%d][%d][%d][%d]", idxShiftY, m_idxCb, m_idxCr, j, c);
															BST_FIELD_PROP_2NUMBER(szTemp2, CMResLSBits, res_coeff_r[i][j][c], "specifies the remainder of the residual for the j-th colour mapping coefficient of the c-th colour component of the octant with octant index equal to (idxShiftY, idxCb, idxCr)");
														}

														if (res_coeff_q[i][j][c] || res_coeff_r[i][j][c]) {
															MBCSPRINTF_S(szTemp2, TEMP2_SIZE, "res_coeff_s[%d][%d][%d][%d][%d]", idxShiftY, m_idxCb, m_idxCr, j, c);
															BST_FIELD_PROP_NUMBER(szTemp2, 1, res_coeff_s[i*PartNumY + j * 4 + c], "the sign of the residual for the j-th colour mapping coefficient of the c-th colour component of the octant with octant index equal to (idxShiftY, idxCb, idxCr)");
														}

														int32_t cmResCoeff = (res_coeff_s[i*PartNumY + j * 4 + c]?-1:1) * (((res_coeff_q[i][j][c] << CMResLSBits) + res_coeff_r[i][j][c]) << ptr_colour_mapping_table->cm_res_quant_bits);
														NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag0000", "cmResCoeff[%d][%d][%d][%d][%d]", cmResCoeff, "", idxShiftY, m_idxCb, m_idxCr, j, c);
													}
													NAV_WRITE_TAG_END("Tag000");
												}
											}
											NAV_WRITE_TAG_END("Tag00");
										}
										NAV_WRITE_TAG_END("Tag0");
									}
								}
							
							DECLARE_FIELDPROP_END()

						};

						uint8_t		num_cm_ref_layers_minus1 = 0;
						uint8_t		cm_ref_layer_id[62] = { 0 };

						uint8_t		cm_octant_depth : 2;
						uint8_t		cm_y_part_num_log2 : 2;
						uint8_t		cm_res_quant_bits : 2;
						uint8_t		cm_delta_flc_bits_minus1 : 2;

						uint8_t		luma_bit_depth_cm_input_minus8 = 0;
						uint8_t		chroma_bit_depth_cm_input_minus8 = 0;
						uint8_t		luma_bit_depth_cm_output_minus8 = 0;
						uint8_t		chroma_bit_depth_cm_output_minus8 = 0;

						int32_t		cm_adapt_threshold_u_delta = 0;
						int32_t		cm_adapt_threshold_v_delta = 0;

						COLOUR_MAPPING_OCTANTS*
									colour_mapping_octants;

						COLOUR_MAPPING_TABLE()
							: cm_octant_depth(0)
							, cm_y_part_num_log2(0)
							, cm_res_quant_bits(0)
							, cm_delta_flc_bits_minus1(0)
							, colour_mapping_octants(NULL){
						}

						virtual ~COLOUR_MAPPING_TABLE() {
							UNMAP_STRUCT_POINTER5(colour_mapping_octants);
						}

						int Map(AMBst in_bst)
						{
							int iRet = RET_CODE_SUCCESS;
							SYNTAX_BITSTREAM_MAP::Map(in_bst);

							try
							{
								MAP_BST_BEGIN(0);

								nal_read_ue(in_bst, num_cm_ref_layers_minus1, uint8_t);
								if (num_cm_ref_layers_minus1 > 61)
									return RET_CODE_BUFFER_NOT_COMPATIBLE;

								for (int i = 0; i < num_cm_ref_layers_minus1; i++) {
									nal_read_u(in_bst, cm_ref_layer_id[i], 6, uint8_t);
								}

								nal_read_u(in_bst, cm_octant_depth, 2, uint8_t);
								nal_read_u(in_bst, cm_y_part_num_log2, 2, uint8_t);

								nal_read_ue(in_bst, luma_bit_depth_cm_input_minus8, uint8_t);
								nal_read_ue(in_bst, chroma_bit_depth_cm_input_minus8, uint8_t);
								nal_read_ue(in_bst, luma_bit_depth_cm_output_minus8, uint8_t);
								nal_read_ue(in_bst, chroma_bit_depth_cm_output_minus8, uint8_t);

								nal_read_u(in_bst, cm_res_quant_bits, 2, uint8_t);
								nal_read_u(in_bst, cm_delta_flc_bits_minus1, 2, uint8_t);

								if (cm_octant_depth == 1) {
									nal_read_se(in_bst, cm_adapt_threshold_u_delta, int32_t);
									nal_read_se(in_bst, cm_adapt_threshold_v_delta, int32_t);
								}

								nal_read_ref(in_bst, colour_mapping_octants, COLOUR_MAPPING_OCTANTS, this, 0, 0, 0, 0, 1 << cm_octant_depth);

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
						BST_FIELD_PROP_UE(num_cm_ref_layers_minus1, "the number of the following cm_ref_layer_id[ i ] syntax elements");
						if (num_cm_ref_layers_minus1 > 0) {
							NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "for(i = 0; i &lt;= num_cm_ref_layers_minus1; i++)", "");
							for (i = 0; i < num_cm_ref_layers_minus1; i++) {
								BST_ARRAY_FIELD_PROP_NUMBER("cm_ref_layer_id", i, 6, cm_ref_layer_id[i], "");
							}
							NAV_WRITE_TAG_END("Tag0");
							BST_FIELD_PROP_2NUMBER1(cm_octant_depth, 2, "the maximal split depth of the colour mapping table");
							BST_FIELD_PROP_2NUMBER1(cm_y_part_num_log2, 2, "the number of partitions of the smallest colour mapping table octant for the luma component");

							uint32_t OctantNumC = 1 << cm_octant_depth;
							uint32_t OctantNumY = 1 << (cm_octant_depth + cm_y_part_num_log2);
							uint32_t PartNumY = 1 << cm_y_part_num_log2;
							NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag1", "OctantNumC = 1 &lt;&lt; cm_octant_depth", OctantNumC, "");
							NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag2", "OctantNumY = 1 &lt;&lt; (cm_octant_depth + cm_y_part_num_log2)", OctantNumY, "");
							NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag3", "PartNumY = 1 &lt;&lt; cm_y_part_num_log2", PartNumY, "");

							BST_FIELD_PROP_UE(luma_bit_depth_cm_input_minus8, "the sample bit depth of the input luma component of the colour mapping process");
							BST_FIELD_PROP_UE(chroma_bit_depth_cm_input_minus8, "the sample bit depth of the input chroma components of the colour mapping process");
							BST_FIELD_PROP_UE(luma_bit_depth_cm_output_minus8, "the sample bit depth of the output luma component of the colour mapping process");
							BST_FIELD_PROP_UE(chroma_bit_depth_cm_output_minus8, "the sample bit depth of the output chroma components of the colour mapping process");

							BST_FIELD_PROP_2NUMBER1(cm_res_quant_bits, 2, "the number of least significant bits to be added to the colour mapping coefficient residual values res_coeff_q and res_coeff_r");
							BST_FIELD_PROP_2NUMBER1(cm_delta_flc_bits_minus1, 2, "the delta value used to derive the number of bits used to code the syntax element res_coeff_r");

							if (cm_octant_depth == 1)
							{
								NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag4", "if( cm_octant_depth = = 1 )", "");
								BST_FIELD_PROP_SE(cm_adapt_threshold_u_delta, "the partitioning threshold of the Cb component used in colour mapping process");
								BST_FIELD_PROP_SE(cm_adapt_threshold_v_delta, "the partitioning threshold of the Cr component used in colour mapping process");
								NAV_WRITE_TAG_END("Tag4");
							}

							NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag5", "colour_mapping_octants(0, 0, 0, 0, 1 &lt;&lt; cm_octant_depth)", "");
							BST_FIELD_PROP_REF(colour_mapping_octants);
							NAV_WRITE_TAG_END("Tag5");
						}
						DECLARE_FIELDPROP_END()
					} PACKED;

					struct REF_LOC_INFO
					{
						uint8_t		ref_loc_offset_layer_id : 6;
						uint8_t		scaled_ref_layer_offset_present_flag : 1;
						uint8_t		reserved_0 : 1;

						int16_t		scaled_ref_layer_left_offset;
						int16_t		scaled_ref_layer_top_offset;
						int16_t		scaled_ref_layer_right_offset;
						int16_t		scaled_ref_layer_bottom_offset;

						uint8_t		ref_region_offset_present_flag;
						uint16_t	ref_region_left_offset;
						uint16_t	ref_region_top_offset;
						uint16_t	ref_region_right_offset;
						uint16_t	ref_region_bottom_offset;

						uint8_t		resample_phase_set_present_flag;
						uint8_t		phase_hor_luma;
						uint8_t		phase_ver_luma;
						uint8_t		phase_hor_chroma_plus8;
						uint8_t		phase_ver_chroma_plus8;
					}PACKED;

					uint8_t		poc_reset_info_present_flag : 1;
					uint8_t		pps_infer_scaling_list_flag : 1;
					uint8_t		pps_scaling_list_ref_layer_id : 6;

					uint8_t		num_ref_loc_offsets = 0;
					REF_LOC_INFO*
								ref_loc_info = nullptr;

					uint8_t		colour_mapping_enabled_flag = 0;
					COLOUR_MAPPING_TABLE*
								colour_mapping_table = nullptr;

					PPS_MULTILAYER_EXTENSION()
						: poc_reset_info_present_flag(0)
						, pps_infer_scaling_list_flag(0)
						, pps_scaling_list_ref_layer_id(0){
					}

					virtual ~PPS_MULTILAYER_EXTENSION() {
						UNMAP_STRUCT_POINTER5(colour_mapping_table);
						AMP_SAFEDELA2(ref_loc_info);
					}

					int Map(AMBst in_bst)
					{
						int iRet = RET_CODE_SUCCESS;
						SYNTAX_BITSTREAM_MAP::Map(in_bst);

						try
						{
							MAP_BST_BEGIN(0);

							nal_read_u(in_bst, poc_reset_info_present_flag, 1, uint8_t);
							nal_read_u(in_bst, pps_infer_scaling_list_flag, 1, uint8_t);

							if (pps_infer_scaling_list_flag) {
								nal_read_u(in_bst, pps_scaling_list_ref_layer_id, 6, uint8_t);
							}
						
							nal_read_ue(in_bst, num_ref_loc_offsets, uint8_t);
							if (num_ref_loc_offsets > 0)
							{
								AMP_NEW0(ref_loc_info, REF_LOC_INFO, num_ref_loc_offsets);
								if (ref_loc_info == nullptr)
									throw AMException(RET_CODE_OUTOFMEMORY);

								for (int i = 0; i < num_ref_loc_offsets; i++)
								{
									nal_read_u(in_bst, ref_loc_info[i].ref_loc_offset_layer_id, 6, uint8_t);
									nal_read_u(in_bst, ref_loc_info[i].scaled_ref_layer_offset_present_flag, 1, uint8_t);
									if (ref_loc_info[i].scaled_ref_layer_offset_present_flag){
										nal_read_se(in_bst, ref_loc_info[i].scaled_ref_layer_left_offset, int16_t);
										nal_read_se(in_bst, ref_loc_info[i].scaled_ref_layer_top_offset, int16_t);
										nal_read_se(in_bst, ref_loc_info[i].scaled_ref_layer_right_offset, int16_t);
										nal_read_se(in_bst, ref_loc_info[i].scaled_ref_layer_bottom_offset, int16_t);
									}

									nal_read_u(in_bst, ref_loc_info[i].ref_region_offset_present_flag, 1, uint8_t);
									if (ref_loc_info[i].ref_region_offset_present_flag){
										nal_read_se(in_bst, ref_loc_info[i].ref_region_left_offset, int16_t);
										nal_read_se(in_bst, ref_loc_info[i].ref_region_top_offset, int16_t);
										nal_read_se(in_bst, ref_loc_info[i].ref_region_right_offset, int16_t);
										nal_read_se(in_bst, ref_loc_info[i].ref_region_bottom_offset, int16_t);
									}

									nal_read_u(in_bst, ref_loc_info[i].resample_phase_set_present_flag, 1, uint8_t);
									if (ref_loc_info[i].resample_phase_set_present_flag) {
										nal_read_ue(in_bst, ref_loc_info[i].phase_hor_luma, uint8_t);
										nal_read_ue(in_bst, ref_loc_info[i].phase_ver_luma, uint8_t);
										nal_read_ue(in_bst, ref_loc_info[i].phase_hor_chroma_plus8, uint8_t);
										nal_read_ue(in_bst, ref_loc_info[i].phase_ver_chroma_plus8, uint8_t);
									}

									nal_read_u(in_bst, colour_mapping_enabled_flag, 1, uint8_t);
									if (colour_mapping_enabled_flag) {
										nal_read_ref(in_bst, colour_mapping_table, COLOUR_MAPPING_TABLE);
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
						BST_FIELD_PROP_BOOL(poc_reset_info_present_flag, "the syntax element poc_reset_idc is present in the slice segment headers of the slices referring to the PPS", 
							"the syntax element poc_reset_idc is not present in the slice segment headers of the slices referring to the PPS");
						BST_FIELD_PROP_BOOL(pps_infer_scaling_list_flag, "the syntax elements of the scaling list data syntax structure of the PPS are inferred to be equal to those of the PPS that is active for the layer with nuh_layer_id equal to pps_scaling_list_ref_layer_id",
							"the syntax elements of the scaling list data syntax structure of the PPS are not inferred");

						if (pps_infer_scaling_list_flag) {
							BST_FIELD_PROP_2NUMBER1(pps_scaling_list_ref_layer_id, 6, "the value of nuh_layer_id of the layer for which the active PPS has the same scaling list data as the current PPS");
						}

						BST_FIELD_PROP_UE(num_ref_loc_offsets, "the number of reference layer location offsets that are present in the PPS");
						NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "for(i = 0; i &lt; num_ref_loc_offsets; i++)", "");
						for (i = 0; i < num_ref_loc_offsets; i++) {
							BST_ARRAY_FIELD_PROP_NUMBER("ref_loc_offset_layer_id", i, 6, ref_loc_info[i].ref_loc_offset_layer_id, "the nuh_layer_id value for which the i-th reference layer location offset parameters are specified");

							BST_ARRAY_FIELD_PROP_NUMBER("scaled_ref_layer_offset_present_flag", i, 1, ref_loc_info[i].scaled_ref_layer_offset_present_flag, 
								ref_loc_info[i].scaled_ref_layer_offset_present_flag?"the i-th scaled reference layer offset parameters are present in the PPS":
								"the i-th scaled reference layer offset parameters are not present in the PPS");
							if (ref_loc_info[i].scaled_ref_layer_offset_present_flag) {
								BST_ARRAY_ARRAY_FIELD_PROP_SE(scaled_ref_layer_left_offset, ref_loc_offset_layer_id, i, ref_loc_info[i].scaled_ref_layer_left_offset, "");
								BST_ARRAY_ARRAY_FIELD_PROP_SE(scaled_ref_layer_top_offset, ref_loc_offset_layer_id, i, ref_loc_info[i].scaled_ref_layer_top_offset, "");
								BST_ARRAY_ARRAY_FIELD_PROP_SE(scaled_ref_layer_right_offset, ref_loc_offset_layer_id, i, ref_loc_info[i].scaled_ref_layer_right_offset, "");
								BST_ARRAY_ARRAY_FIELD_PROP_SE(scaled_ref_layer_bottom_offset, ref_loc_offset_layer_id, i, ref_loc_info[i].scaled_ref_layer_bottom_offset, "");
							}

							BST_ARRAY_FIELD_PROP_NUMBER("ref_region_offset_present_flag", i, 1, ref_loc_info[i].ref_region_offset_present_flag, 
								ref_loc_info[i].ref_region_offset_present_flag?"the i-th reference region offset parameters are present in the PPS":
								"the i-th reference region offset parameters are not present in the PPS");
							if (ref_loc_info[i].ref_region_offset_present_flag) {
								BST_ARRAY_ARRAY_FIELD_PROP_SE(ref_region_left_offset, ref_loc_offset_layer_id, i, ref_loc_info[i].ref_region_left_offset, "");
								BST_ARRAY_ARRAY_FIELD_PROP_SE(ref_region_top_offset, ref_loc_offset_layer_id, i, ref_loc_info[i].ref_region_top_offset, "");
								BST_ARRAY_ARRAY_FIELD_PROP_SE(ref_region_right_offset, ref_loc_offset_layer_id, i, ref_loc_info[i].ref_region_right_offset, "");
								BST_ARRAY_ARRAY_FIELD_PROP_SE(ref_region_bottom_offset, ref_loc_offset_layer_id, i, ref_loc_info[i].ref_region_bottom_offset, "");
							}

							BST_ARRAY_FIELD_PROP_NUMBER("resample_phase_set_present_flag", i, 1, ref_loc_info[i].resample_phase_set_present_flag, 
								ref_loc_info[i].resample_phase_set_present_flag?"the i-th resampling phase set is present in the PPS":
								"the i-th resampling phase set is not present in the PPS");
							if (ref_loc_info[i].resample_phase_set_present_flag) {
								BST_ARRAY_ARRAY_FIELD_PROP_SE(phase_hor_luma, ref_loc_offset_layer_id, i, ref_loc_info[i].phase_hor_luma, "");
								BST_ARRAY_ARRAY_FIELD_PROP_SE(phase_ver_luma, ref_loc_offset_layer_id, i, ref_loc_info[i].phase_ver_luma, "");
								BST_ARRAY_ARRAY_FIELD_PROP_SE(phase_hor_chroma_plus8, ref_loc_offset_layer_id, i, ref_loc_info[i].phase_hor_chroma_plus8, "");
								BST_ARRAY_ARRAY_FIELD_PROP_SE(phase_ver_chroma_plus8, ref_loc_offset_layer_id, i, ref_loc_info[i].phase_ver_chroma_plus8, "");
							}
						}
						NAV_WRITE_TAG_END("Tag0");

						BST_FIELD_PROP_BOOL(colour_mapping_enabled_flag, "the colour mapping process may be applied to the inter-layer reference pictures for the coded pictures referring to the PPS", 
							"the colour mapping process is not applied to the inter-layer reference pictures for the coded pictures referring to the PPS");
						if (colour_mapping_enabled_flag) {
							BST_FIELD_PROP_REF1(colour_mapping_table);
						}
					DECLARE_FIELDPROP_END()

				}PACKED;

				struct PPS_3D_EXTENSION : public SYNTAX_BITSTREAM_MAP
				{
					struct DELTA_DLT : public SYNTAX_BITSTREAM_MAP
					{
						uint32_t		num_val_delta_dlt =  0;
						uint32_t		max_diff = 0;
						uint32_t		min_diff_minus1 = 0;
						uint32_t		delta_dlt_val0 = 0;
						uint32_t*		delta_val_diff_minus_min = nullptr;

						PPS_3D_EXTENSION*
										pps_3d_extension;

						DELTA_DLT(PPS_3D_EXTENSION* ptr_pps_3d_extension): pps_3d_extension(ptr_pps_3d_extension){}

						virtual ~DELTA_DLT() {
							AMP_SAFEDELA(delta_val_diff_minus_min);
						}

						int Map(AMBst in_bst)
						{
							int iRet = RET_CODE_SUCCESS;
							SYNTAX_BITSTREAM_MAP::Map(in_bst);

							try
							{
								MAP_BST_BEGIN(0);
								uint8_t read_length = pps_3d_extension->pps_bit_depth_for_depth_layers_minus8 + 8;
								nal_read_u(in_bst, num_val_delta_dlt, read_length, uint32_t);

								if (num_val_delta_dlt > 0) {
									if (num_val_delta_dlt > 1) {
										nal_read_u(in_bst, max_diff, read_length, uint32_t);
									}
									else
										max_diff = 0;

									if (num_val_delta_dlt > 2 && max_diff > 0) {
										uint8_t read_length_1 = (uint8_t)quick_log2(max_diff + 1);
										read_length_1 += ((uint32_t)(1 << read_length_1)) < (max_diff + 1) ? 1 : 0;
										nal_read_u(in_bst, min_diff_minus1, read_length_1, uint8_t);
									}
									else
										min_diff_minus1 = (uint32_t)(max_diff - 1);

									nal_read_u(in_bst, delta_dlt_val0, read_length, uint32_t);
									if (max_diff > (min_diff_minus1 + 1)) {
										AMP_NEW0(delta_val_diff_minus_min, uint32_t, num_val_delta_dlt);
										if (delta_val_diff_minus_min == nullptr)
											throw AMException(RET_CODE_OUTOFMEMORY);

										for (uint32_t k = 1; k < num_val_delta_dlt; k++) {
											uint8_t read_length_2 = (uint8_t)quick_log2(max_diff - (uint32_t)(min_diff_minus1 + 1) + 1);
											nal_read_u(in_bst, delta_val_diff_minus_min[k], read_length_2, uint32_t);
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
						uint8_t read_length = pps_3d_extension->pps_bit_depth_for_depth_layers_minus8 + 8;
						BST_FIELD_PROP_2NUMBER1(num_val_delta_dlt, read_length, "the number of elements in the list deltaList");
						if (num_val_delta_dlt > 0) {
							NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "if(num_val_delta_dlt &gt; 0)", "");
							if (num_val_delta_dlt > 1){
								NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag00", "if(num_val_delta_dlt &gt; 1)", "");
								BST_FIELD_PROP_2NUMBER1(max_diff, read_length, "the maximum difference between two consecutive elements in the list deltaList");
								NAV_WRITE_TAG_END("Tag00");
							}

							if (num_val_delta_dlt > 2 && max_diff > 0) {
								uint8_t read_length_1 = (uint8_t)quick_log2(max_diff + 1);
								read_length_1 += ((uint32_t)(1 << read_length_1)) < (max_diff + 1) ? 1 : 0;
								NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag01", "if (num_val_delta_dlt &gt; 2 && max_diff > 0)", "");
								BST_FIELD_PROP_2NUMBER1(min_diff_minus1, read_length_1, "the minimum difference between two consecutive elements in the list deltaList");
								NAV_WRITE_TAG_END("Tag01");
							}

							BST_FIELD_PROP_2NUMBER1(delta_dlt_val0, read_length, "the 0-th element in the list deltaList");

							NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag02", "if(max_diff &gt; (min_diff_minus1 + 1))", "");
							NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag020", "for(k = 1; k &lt; num_val_delta_dlt; k++)", "");

							uint32_t curr_delta_list = delta_dlt_val0;
							for (uint32_t k = 0; k < num_val_delta_dlt; k++) {
								if (k == 0)
								{
									NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag0200", "deltaList[%d]", curr_delta_list, "", (int)k);
								}
								else
								{
									curr_delta_list += delta_val_diff_minus_min[k] + (uint32_t)(min_diff_minus1 + 1);
									uint8_t read_length_2 = (uint8_t)quick_log2(max_diff - (uint32_t)(min_diff_minus1 + 1) + 1);
									BST_ARRAY_FIELD_PROP_NUMBER("delta_val_diff_minus_min", k, read_length_2, delta_val_diff_minus_min[k], "plus minDiff specifies the difference between the k-th element and the (k-1)-th element in the list deltaList");
									NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag0200", "deltaList[%d]", curr_delta_list, "", (int)k);
								}
							}
							NAV_WRITE_TAG_END("Tag020");
							NAV_WRITE_TAG_END("Tag02");

							NAV_WRITE_TAG_END("Tag0");
						}
						DECLARE_FIELDPROP_END()
							
					}PACKED;

					uint8_t			dlts_present_flag;
					uint8_t			pps_depth_layers_minus1;
					uint8_t			pps_bit_depth_for_depth_layers_minus8;
					CAMBitArray		dlt_flag;
					CAMBitArray		dlt_pred_flag;
					CAMBitArray		dlt_val_flags_present_flag;
					CAMBitArray		dlt_value_flag;
					DELTA_DLT*		delta_dlt[64];

					PPS_3D_EXTENSION() {
						memset(delta_dlt, 0, sizeof(delta_dlt));
					}

					virtual ~PPS_3D_EXTENSION() {
						for (int i = 0; i < 64; i++) {
							UNMAP_STRUCT_POINTER5(delta_dlt[i]);
						}
					}

					int Map(AMBst in_bst)
					{
						int iRet = RET_CODE_SUCCESS;
						SYNTAX_BITSTREAM_MAP::Map(in_bst);

						try
						{
							MAP_BST_BEGIN(0);
							nal_read_u(in_bst, dlts_present_flag, 1, uint8_t);
							if (dlts_present_flag) {
								nal_read_u(in_bst, pps_depth_layers_minus1, 6, uint8_t);
								nal_read_u(in_bst, pps_bit_depth_for_depth_layers_minus8, 4, uint8_t);

								uint32_t depthMaxValue = (1 << (pps_bit_depth_for_depth_layers_minus8 + 8)) - 1;
								for (int i = 0; i <= pps_depth_layers_minus1; i++) {
									nal_read_bitarray(in_bst, dlt_flag, i);
									if (dlt_flag[i]) {
										nal_read_bitarray(in_bst, dlt_pred_flag, i);

										if (!dlt_pred_flag[i]) {
											nal_read_bitarray(in_bst, dlt_val_flags_present_flag, i);
										}
										else
											dlt_val_flags_present_flag.BitClear(i);

										if (dlt_val_flags_present_flag[i]) {
											for (uint32_t j = 0; j <= depthMaxValue; j++) {
												nal_read_bitarray(in_bst, dlt_value_flag, i*(depthMaxValue + 1) + j);
											}
										}
										else
										{
											nal_read_ref(in_bst, delta_dlt[i], DELTA_DLT, this);
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

					int Unmap(AMBst out_bst)
					{
						UNREFERENCED_PARAMETER(out_bst);
						return RET_CODE_ERROR_NOTIMPL;
					}

					DECLARE_FIELDPROP_BEGIN()
					BST_FIELD_PROP_BOOL(dlts_present_flag, "syntax elements for the derivation of depth look-up tables are present in the PPS", "syntax elements for the derivation of depth look-up tables are not present in the PPS");
					if (dlts_present_flag){
						uint32_t depthMaxValue = (1 << (pps_bit_depth_for_depth_layers_minus8 + 8)) - 1;
						NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "if( dlts_present_flag )", "");
						BST_FIELD_PROP_2NUMBER1(pps_depth_layers_minus1, 6, "");
						BST_FIELD_PROP_2NUMBER1(pps_bit_depth_for_depth_layers_minus8, 4, "");
						NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag00", "for(i = 0; i &lt;= pps_depth_layers_minus1; i++)", "");
						for (i = 0; i <= pps_depth_layers_minus1; i++) {
							BST_ARRAY_FIELD_PROP_NUMBER("dlt_flag", i, 1, dlt_flag[i], "");
							NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag000", "if( dlt_flag[i])", "");
							BST_ARRAY_FIELD_PROP_NUMBER("dlt_pred_flag", i, 1, dlt_pred_flag[i], "");
							if (!dlt_pred_flag[i]) {
								BST_ARRAY_FIELD_PROP_NUMBER("dlt_val_flags_present_flag", i, 1, dlt_val_flags_present_flag[i], "");
							}
							if (dlt_val_flags_present_flag[i]) {
								NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0000", "for(j = 0; j &lt;= depthMaxValue; j++)", "");
								for (uint32_t j = 0; j <= depthMaxValue; j++) {
									NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("dlt_value_flag", "dlt_value_flag[%d][%d]", "", i, j);
									BST_FIELD_PROP_BOOL(dlt_value_flag[j], "an entry in the depth look-up table of the layer", "not an entry in the depth look-up table of the layer");
									NAV_WRITE_TAG_END("dlt_value_flag");
								}
								NAV_WRITE_TAG_END("Tag0000");
							}
							else
							{
								NAV_WRITE_TAG_ARRAY_BEGIN0("delta_dlt", i, "");
								BST_FIELD_PROP_REF(delta_dlt[i]);
								NAV_WRITE_TAG_END("delta_dlt");
							}
							NAV_WRITE_TAG_END("Tag000");
						}
						NAV_WRITE_TAG_END("Tag00");
						NAV_WRITE_TAG_END("Tag0");
					}
					DECLARE_FIELDPROP_END()

				};

				uint8_t		pps_pic_parameter_set_id;
				uint8_t		pps_seq_parameter_set_id;

				uint8_t		dependent_slice_segments_enabled_flag : 1;
				uint8_t		output_flag_present_flag : 1;
				uint8_t		num_extra_slice_header_bits : 3;
				uint8_t		sign_data_hiding_enabled_flag : 1;
				uint8_t		cabac_init_present_flag : 1;
				uint8_t		reserved_0 : 1;

				uint8_t		num_ref_idx_l0_default_active_minus1 : 4;
				uint8_t		num_ref_idx_l1_default_active_minus1 : 4;
				int8_t		init_qp_minus26;

				uint8_t		constrained_intra_pred_flag : 1;
				uint8_t		transform_skip_enabled_flag : 1;
				uint8_t		cu_qp_delta_enabled_flag : 1;
				uint8_t		reserved_1 : 5;

				uint8_t		diff_cu_qp_delta_depth;

				int8_t		pps_cb_qp_offset;
				int8_t		pps_cr_qp_offset;

				uint8_t		pps_slice_chroma_qp_offsets_present_flag : 1;
				uint8_t		weighted_pred_flag : 1;
				uint8_t		weighted_bipred_flag : 1;
				uint8_t		transquant_bypass_enabled_flag : 1;
				uint8_t		tiles_enabled_flag : 1;
				uint8_t		entropy_coding_sync_enabled_flag : 1;
				uint8_t		uniform_spacing_flag : 1;
				uint8_t		loop_filter_across_tiles_enabled_flag : 1;

				uint16_t	num_tile_columns_minus1;
				uint16_t	num_tile_rows_minus1;
				uint16_t*	column_width_minus1;
				uint16_t*	row_height_minus1;

				uint8_t		pps_loop_filter_across_slices_enabled_flag : 1;
				uint8_t		deblocking_filter_control_present_flag : 1;
				uint8_t		deblocking_filter_override_enabled_flag : 1;
				uint8_t		pps_deblocking_filter_disabled_flag : 1;
				uint8_t		pps_scaling_list_data_present_flag : 1;
				uint8_t		lists_modification_present_flag : 1;
				uint8_t		slice_segment_header_extension_present_flag : 1;
				uint8_t		pps_extension_present_flag : 1;

				int8_t		pps_beta_offset_div2;
				int8_t		pps_tc_offset_div2;

				SCALING_LIST_DATA*
							scaling_list_data;

				uint8_t		log2_parallel_merge_level_minus2;

				uint8_t		pps_range_extension_flag : 1;
				uint8_t		pps_multilayer_extension_flag : 1;
				uint8_t		pps_3d_extension_flag : 1;
				uint8_t		pps_extension_5bits : 5;

				PPS_RANGE_EXTENSION*
							pps_range_extension;
				PPS_MULTILAYER_EXTENSION*
							pps_multilayer_extension;
				PPS_3D_EXTENSION*
							pps_3d_extension;
				RBSP_TRAILING_BITS
							rbsp_trailing_bits;

				PIC_PARAMETER_SET_RBSP()
					: column_width_minus1(NULL)
					, row_height_minus1(NULL)
					, deblocking_filter_override_enabled_flag(0)
					, pps_deblocking_filter_disabled_flag(0)
					, scaling_list_data(NULL)
					, pps_range_extension_flag(0)
					, pps_multilayer_extension_flag(0)
					, pps_3d_extension_flag(0)
					, pps_extension_5bits(0)
					, pps_range_extension(NULL)
					, pps_multilayer_extension(NULL)
					, pps_3d_extension(NULL)
				{
				}

				virtual ~PIC_PARAMETER_SET_RBSP() {
					UNMAP_STRUCT_POINTER5(pps_3d_extension);
					UNMAP_STRUCT_POINTER5(pps_multilayer_extension);
					UNMAP_STRUCT_POINTER5(pps_range_extension);
					UNMAP_STRUCT_POINTER5(scaling_list_data);
					AMP_SAFEDELA2(column_width_minus1);
					AMP_SAFEDELA2(row_height_minus1);
				}

				int Map(AMBst in_bst)
				{
					int iRet = RET_CODE_SUCCESS;
					SYNTAX_BITSTREAM_MAP::Map(in_bst);

					try
					{
						MAP_BST_BEGIN(0);

						nal_read_ue(in_bst, pps_pic_parameter_set_id, uint8_t);
						nal_read_ue(in_bst, pps_seq_parameter_set_id, uint8_t);
						nal_read_u(in_bst, dependent_slice_segments_enabled_flag, 1, uint8_t);
						nal_read_u(in_bst, output_flag_present_flag, 1, uint8_t);
						nal_read_u(in_bst, num_extra_slice_header_bits, 3, uint8_t);
						nal_read_u(in_bst, sign_data_hiding_enabled_flag, 1, uint8_t);
						nal_read_u(in_bst, cabac_init_present_flag, 1, uint8_t);

						nal_read_ue(in_bst, num_ref_idx_l0_default_active_minus1, uint8_t);
						nal_read_ue(in_bst, num_ref_idx_l1_default_active_minus1, uint8_t);

						nal_read_se(in_bst, init_qp_minus26, int8_t);

						nal_read_u(in_bst, constrained_intra_pred_flag, 1, uint8_t);
						nal_read_u(in_bst, transform_skip_enabled_flag, 1, uint8_t);
						nal_read_u(in_bst, cu_qp_delta_enabled_flag, 1, uint8_t);

						if (cu_qp_delta_enabled_flag) {
							nal_read_ue(in_bst, diff_cu_qp_delta_depth, uint8_t);
						}

						nal_read_se(in_bst, pps_cb_qp_offset, int8_t);
						nal_read_se(in_bst, pps_cr_qp_offset, int8_t);

						nal_read_u(in_bst, pps_slice_chroma_qp_offsets_present_flag, 1, uint8_t);
						nal_read_u(in_bst, weighted_pred_flag, 1, uint8_t);
						nal_read_u(in_bst, weighted_bipred_flag, 1, uint8_t);
						nal_read_u(in_bst, transquant_bypass_enabled_flag, 1, uint8_t);
						nal_read_u(in_bst, tiles_enabled_flag, 1, uint8_t);
						nal_read_u(in_bst, entropy_coding_sync_enabled_flag, 1, uint8_t);

						if (tiles_enabled_flag) {
							nal_read_ue(in_bst, num_tile_columns_minus1, uint16_t);
							nal_read_ue(in_bst, num_tile_rows_minus1, uint16_t);
							nal_read_u(in_bst, uniform_spacing_flag, 1, uint8_t);

							if (!uniform_spacing_flag) {
								if (num_tile_columns_minus1 > 0)
								{
									AMP_NEW0(column_width_minus1, uint16_t, num_tile_columns_minus1);
									for (int i = 0; i < num_tile_columns_minus1; i++) {
										nal_read_ue(in_bst, column_width_minus1[i], uint16_t);
									}
								}

								if (num_tile_rows_minus1 > 0)
								{
									AMP_NEW0(row_height_minus1, uint16_t, num_tile_rows_minus1);
									for (int i = 0; i < num_tile_rows_minus1; i++) {
										nal_read_ue(in_bst, row_height_minus1[i], uint16_t);
									}
								}
							}

							nal_read_u(in_bst, loop_filter_across_tiles_enabled_flag, 1, uint8_t);
						}

						nal_read_u(in_bst, pps_loop_filter_across_slices_enabled_flag, 1, uint8_t);
						nal_read_u(in_bst, deblocking_filter_control_present_flag, 1, uint8_t);

						if (deblocking_filter_control_present_flag) {
							nal_read_u(in_bst, deblocking_filter_override_enabled_flag, 1, uint8_t);
							nal_read_u(in_bst, pps_deblocking_filter_disabled_flag, 1, uint8_t);

							if (!pps_deblocking_filter_disabled_flag) {
								nal_read_se(in_bst, pps_beta_offset_div2, int8_t);
								nal_read_se(in_bst, pps_tc_offset_div2, int8_t);
							}
						}

						nal_read_u(in_bst, pps_scaling_list_data_present_flag, 1, uint8_t);
						if (pps_scaling_list_data_present_flag) {
							nal_read_ref(in_bst, scaling_list_data, SCALING_LIST_DATA);
						}

						nal_read_u(in_bst, lists_modification_present_flag, 1, uint8_t);
						nal_read_ue(in_bst, log2_parallel_merge_level_minus2, uint8_t);
						nal_read_u(in_bst, slice_segment_header_extension_present_flag, 1, uint8_t);
						nal_read_u(in_bst, pps_extension_present_flag, 1, uint8_t);

						if (pps_extension_present_flag)
						{
							nal_read_u(in_bst, pps_range_extension_flag, 1, uint8_t);
							nal_read_u(in_bst, pps_multilayer_extension_flag, 1, uint8_t);
							nal_read_u(in_bst, pps_3d_extension_flag, 1, uint8_t);
							nal_read_u(in_bst, pps_extension_5bits, 5, uint8_t);
						}

						nal_read_ref1(in_bst, pps_range_extension_flag, pps_range_extension, PPS_RANGE_EXTENSION, this);
						nal_read_ref1(in_bst, pps_multilayer_extension_flag, pps_multilayer_extension, PPS_MULTILAYER_EXTENSION);
						nal_read_ref1(in_bst, pps_3d_extension_flag, pps_3d_extension, PPS_3D_EXTENSION);

						nal_read_obj(in_bst, rbsp_trailing_bits);

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
					BST_FIELD_PROP_UE(pps_pic_parameter_set_id, "identifies the PPS for reference by other syntax elements");
					BST_FIELD_PROP_UE(pps_seq_parameter_set_id, "the value of sps_seq_parameter_set_id for the active SPS");
					BST_FIELD_PROP_BOOL(dependent_slice_segments_enabled_flag, "specifies the presence of the syntax element dependent_slice_segment_flag in the slice segment headers for coded pictures referring to the PPS", 
						"specifies the absence of the syntax element dependent_slice_segment_flag in the slice segment headers for coded pictures referring to the PPS");

					BST_FIELD_PROP_BOOL(output_flag_present_flag, "indicates that the pic_output_flag syntax element is present in the associated slice headers", "indicates that the pic_output_flag syntax element is not present in the associated slice headers");
					BST_FIELD_PROP_2NUMBER1(num_extra_slice_header_bits, 3, "the number of extra slice header bits that are present in the slice header RBSP for coded pictures referring to the PPS");
					BST_FIELD_PROP_BOOL(sign_data_hiding_enabled_flag, "specifies that sign bit hiding is enabled", "specifies that sign bit hiding is disabled");
					BST_FIELD_PROP_BOOL(cabac_init_present_flag, "specifies that cabac_init_flag is present in slice headers referring to the PPS", "specifies that cabac_init_flag is not present in slice headers referring to the PPS");

					BST_FIELD_PROP_UE(num_ref_idx_l0_default_active_minus1, "the inferred value of num_ref_idx_l0_active_minus1 for P and B slices with num_ref_idx_active_override_flag equal to 0");
					BST_FIELD_PROP_UE(num_ref_idx_l1_default_active_minus1, "the inferred value of num_ref_idx_l1_active_minus1 with num_ref_idx_active_override_flag equal to 0");

					BST_FIELD_PROP_SE(init_qp_minus26, "plus 26 specifies the initial value of SliceQpY for each slice referring to the PPS");

					BST_FIELD_PROP_BOOL(constrained_intra_pred_flag, "specifies constrained intra prediction, in which case intra prediction only uses residual data and decoded samples from neighbouring coding blocks coded using intra prediction modes", 
						"specifies that intra prediction allows usage of residual data and decoded samples of neighbouring coding blocks coded using either intra or inter prediction modes");
					BST_FIELD_PROP_BOOL(transform_skip_enabled_flag, "specifies that transform_skip_flag may be present in the residual coding syntax", "specifies that transform_skip_flag is not present in the residual coding syntax");
					BST_FIELD_PROP_BOOL(cu_qp_delta_enabled_flag, "specifies that the diff_cu_qp_delta_depth syntax element is present in the PPS and that cu_qp_delta_abs may be present in the transform unit syntax", 
						"specifies that the diff_cu_qp_delta_depth syntax element is not present in the PPS and that cu_qp_delta_abs is not present in the transform unit syntax");

					if (cu_qp_delta_enabled_flag){
						NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "if( cu_qp_delta_enabled_flag )", "");
						BST_FIELD_PROP_UE(diff_cu_qp_delta_depth, "specifies the difference between the luma coding tree block size and the minimum luma coding block size of coding units that convey cu_qp_delta_abs and cu_qp_delta_sign_flag");
						NAV_WRITE_TAG_END("Tag0");
					}

					BST_FIELD_PROP_SE(pps_cb_qp_offset, "specify the offsets to the luma quantization parameter Qp'Y used for deriving Qp'Cb");
					BST_FIELD_PROP_SE(pps_cr_qp_offset, "specify the offsets to the luma quantization parameter Qp'Y used for deriving Qp'Cr");

					BST_FIELD_PROP_BOOL(pps_slice_chroma_qp_offsets_present_flag, "the slice_cb_qp_offset and slice_cr_qp_offset syntax elements are present in the associated slice headers", "these syntax elements are not present in the associated slice headers");
					BST_FIELD_PROP_BOOL(weighted_pred_flag, "specifies that weighted prediction is applied to P slices", "specifies that weighted prediction is not applied to P slices");
					BST_FIELD_PROP_BOOL(weighted_bipred_flag, "specifies that weighted prediction is applied to B slices", "specifies that the default weighted prediction is applied to B slices");
					BST_FIELD_PROP_BOOL(transquant_bypass_enabled_flag, "specifies that cu_transquant_bypass_flag is present", "specifies that cu_transquant_bypass_flag is not present");
					BST_FIELD_PROP_BOOL(tiles_enabled_flag, "specifies that there is more than one tile in each picture referring to the PPS", "specifies that there is only one tile in each picture referring to the PPS");
					BST_FIELD_PROP_BOOL(entropy_coding_sync_enabled_flag, "specifies that a specific synchronization process for context variables is invoked before decoding the coding tree unit which includes the first coding tree block of a row of coding tree blocks in each tile in each picture referring to the PPS, and a specific storage process for context variables is invoked after decoding the coding tree unit which includes the second coding tree block of a row of coding tree blocks in each tile in each picture referring to the PPS", 
						"specifies that no specific synchronization process for context variables is required to be invoked before decoding the coding tree unit which includes the first coding tree block of a row of coding tree blocks in each tile in each picture referring to the PPS, and no specific storage process for context variables is required to be invoked after decoding the coding tree unit which includes the second coding tree block of a row of coding tree blocks in each tile in each picture referring to the PPS");

					if (tiles_enabled_flag) {
						NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag1", "if( tiles_enabled_flag )", "");
						BST_FIELD_PROP_UE(num_tile_columns_minus1, "plus 1 specifies the number of tile columns partitioning the picture");
						BST_FIELD_PROP_UE(num_tile_rows_minus1, "plus 1 specifies the number of tile rows partitioning the picture");
						BST_FIELD_PROP_BOOL(uniform_spacing_flag, "specifies that tile column boundaries and likewise tile row boundaries are distributed uniformly across the picture", 
							"specifies that tile column boundaries and likewise tile row boundaries are not distributed uniformly across the picture but signalled explicitly using the syntax elements column_width_minus1[i] and row_height_minus1[i]");
						if (!uniform_spacing_flag) {
							NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag10", "if( !uniform_spacing_flag )", "");
							NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag100", "for(i=0;i&lt;num_tile_columns_minus1;i++)", "");
							for (int i = 0; i < num_tile_columns_minus1; i++) {
								BST_ARRAY_FIELD_PROP_NUMBER("column_width_minus1", i, (long long)quick_log2(column_width_minus1[i] + 1) * 2 + 1, column_width_minus1[i], "the width of the i-th tile column in units of coding tree blocks");
							}
							NAV_WRITE_TAG_END("Tag100");
							NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag101", "for( i=0;i&lt;num_tile_rows_minus1;i++)", "");
							for (int i = 0; i < num_tile_rows_minus1; i++) {
								BST_ARRAY_FIELD_PROP_NUMBER("row_height_minus1", i, (long long)quick_log2(row_height_minus1[i] + 1) * 2 + 1, row_height_minus1[i], "the height of the i-th tile row in units of coding tree blocks");
							}
							NAV_WRITE_TAG_END("Tag101");
							NAV_WRITE_TAG_END("Tag10");
						}
						BST_FIELD_PROP_BOOL(loop_filter_across_tiles_enabled_flag, "specifies that in-loop filtering operations may be performed across tile boundaries in pictures referring to the PPS", 
							"specifies that in-loop filtering operations are not performed across tile boundaries in pictures referring to the PPS");
						NAV_WRITE_TAG_END("Tag1");
					}

					BST_FIELD_PROP_BOOL(pps_loop_filter_across_slices_enabled_flag, "specifies that in-loop filtering operations may be performed across left and upper boundaries of slices referring to the PPS", 
						"specifies that in-loop filtering operations are not performed across left and upper boundaries of slices referring to the PPS");
					BST_FIELD_PROP_BOOL(deblocking_filter_control_present_flag, "specifies the presence of deblocking filter control syntax elements in the PPS",
						"specifies the absence of deblocking filter control syntax elements in the PPS");

					if (deblocking_filter_control_present_flag) {
						NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag2", "if( deblocking_filter_control_present_flag )", "");
						BST_FIELD_PROP_BOOL(deblocking_filter_override_enabled_flag, "specifies the presence of deblocking_filter_override_flag in the slice headers for pictures referring to the PPS", 
							"specifies the absence of deblocking_filter_override_flag in the slice headers for pictures referring to the PPS");
						BST_FIELD_PROP_BOOL(pps_deblocking_filter_disabled_flag, "specifies that the operation of deblocking filter is not applied for slices referring to the PPS in which slice_deblocking_filter_disabled_flag is not present", 
							"specifies that the operation of the deblocking filter is applied for slices referring to the PPS in which slice_deblocking_filter_disabled_flag is not present");
						if (!pps_deblocking_filter_disabled_flag) {
							NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag20", "if( !pps_deblocking_filter_disabled_flag )", "");
							BST_FIELD_PROP_SE(pps_beta_offset_div2, "specify the default deblocking parameter offsets for beta");
							BST_FIELD_PROP_SE(pps_tc_offset_div2, "specify the default deblocking parameter offsets for tc");
							NAV_WRITE_TAG_END("Tag20");
						}
						NAV_WRITE_TAG_END("Tag2");
					}

					BST_FIELD_PROP_BOOL(pps_scaling_list_data_present_flag, "specifies that parameters are present in the PPS to modify the scaling lists specified by the active SPS", 
						"specifies that the scaling list data used for the pictures referring to the PPS are inferred to be equal to those specified by the active SPS");
					if (pps_scaling_list_data_present_flag) {
						BST_FIELD_PROP_REF1(scaling_list_data);
					}
					BST_FIELD_PROP_BOOL(lists_modification_present_flag, "specifies that the syntax structure ref_pic_lists_modification() is present in the slice segment header", 
						"specifies that the syntax structure ref_pic_lists_modification( ) is not present in the slice segment header");
					BST_FIELD_PROP_UE(log2_parallel_merge_level_minus2, "plus 2 specifies the value of the variable Log2ParMrgLevel, which is used in the derivation process for luma motion vectors for merge mode as specified in clause 8.5.3.2.2 and the derivation process for spatial merging candidates as specified in clause 8.5.3.2.3");

					BST_FIELD_PROP_BOOL(slice_segment_header_extension_present_flag, "specifies that slice segment header extension syntax elements are present in the slice segment headers for coded pictures referring to the PPS", 
						"specifies that no slice segment header extension syntax elements are present in the slice segment headers for coded pictures referring to the PPS");
					BST_FIELD_PROP_BOOL(pps_extension_present_flag, "specifies that the syntax elements pps_range_extension_flag, pps_multilayer_extension_flag, pps_3d_extension_flag and pps_extension_5bits are present in the picture parameter set RBSP syntax structure", 
						"specifies that these syntax elements are not present");

					if (pps_extension_present_flag) {
						NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag3", "if( pps_extension_present_flag )", "");
						BST_FIELD_PROP_BOOL(pps_range_extension_flag, "specifies that the pps_range_extension( ) syntax structure is present", "specifies that this syntax structure is not present");
						BST_FIELD_PROP_BOOL(pps_multilayer_extension_flag, "specifies that the pps_multilayer_extension( ) syntax structure is present", "specifies that the pps_multilayer_extension() syntax structure is not present");
						BST_FIELD_PROP_BOOL(pps_3d_extension_flag, "specifies that the pps_3d_extension() syntax structure (specified in Annex I) is present", "specifies that the pps_3d_extension() syntax structure is not present");
						BST_FIELD_PROP_2NUMBER1(pps_extension_5bits, 5, "");
						NAV_WRITE_TAG_END("Tag3");
					}

					if (pps_range_extension_flag)
					{
						NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag4", "if( pps_range_extension_flag )", "");
						BST_FIELD_PROP_REF1(pps_range_extension);
						NAV_WRITE_TAG_END("Tag4");
					}

					if (pps_multilayer_extension_flag)
					{
						NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag5", "if( pps_multilayer_extension_flag )", "");
						BST_FIELD_PROP_REF1(pps_multilayer_extension);
						NAV_WRITE_TAG_END("Tag5");
					}

					if (pps_3d_extension_flag)
					{
						NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag6", "if( pps_3d_extension_flag )", "");
						BST_FIELD_PROP_REF1(pps_3d_extension);
						NAV_WRITE_TAG_END("Tag6");
					}

					BST_FIELD_PROP_OBJECT(rbsp_trailing_bits)

				DECLARE_FIELDPROP_END()

			};

			struct SLICE_SEGMENT_LAYER_RBSP : public SYNTAX_BITSTREAM_MAP
			{
				struct SLICE_SEGMENT_HEADER : public SYNTAX_BITSTREAM_MAP
				{
					struct REF_PIC_LISTS_MODIFICATION : public SYNTAX_BITSTREAM_MAP
					{
						uint8_t				ref_pic_list_modification_flag[2];
						uint32_t*			list_entry[2];

						SLICE_SEGMENT_HEADER*
							ptr_slice_header;

						REF_PIC_LISTS_MODIFICATION(SLICE_SEGMENT_HEADER* pSliceHdr) : ptr_slice_header(pSliceHdr) {
							memset(ref_pic_list_modification_flag, 0, sizeof(ref_pic_list_modification_flag));
							memset(list_entry, 0, sizeof(list_entry));
						}

						virtual ~REF_PIC_LISTS_MODIFICATION()
						{
							for (int i = 0; i < 2; i++) {
								AMP_SAFEDELA2(list_entry[i]);
							}
						}

						int Map(AMBst in_bst)
						{
							int iRet = RET_CODE_SUCCESS;
							SYNTAX_BITSTREAM_MAP::Map(in_bst);

							try
							{
								MAP_BST_BEGIN(0);
								nal_read_u(in_bst, ref_pic_list_modification_flag[0], 1, uint8_t);

								uint8_t index_bits = quick_ceil_log2(ptr_slice_header->NumPicTotalCurr);
								if (ref_pic_list_modification_flag[0])
								{
									list_entry[0] = new uint32_t[(size_t)ptr_slice_header->num_ref_idx_l0_active_minus1 + 1];
									memset(list_entry[0], 0, (size_t)ptr_slice_header->num_ref_idx_l0_active_minus1 + 1);

									for (uint8_t rps_idx = 0; rps_idx < ptr_slice_header->num_ref_idx_l0_active_minus1; rps_idx++)
										nal_read_u(in_bst, list_entry[0][rps_idx], index_bits, uint32_t);
								}

								if (ptr_slice_header->slice_type == B_SLICE)
								{
									nal_read_u(in_bst, ref_pic_list_modification_flag[1], 1, uint8_t);
									if (ref_pic_list_modification_flag[1])
									{
										list_entry[1] = new uint32_t[(size_t)ptr_slice_header->num_ref_idx_l1_active_minus1 + 1];
										memset(list_entry[0], 0, (size_t)ptr_slice_header->num_ref_idx_l1_active_minus1 + 1);

										for (uint8_t rps_idx = 0; rps_idx < ptr_slice_header->num_ref_idx_l1_active_minus1; rps_idx++)
											nal_read_u(in_bst, list_entry[1][rps_idx], index_bits, uint32_t);
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
						NAV_FIELD_PROP_WITH_ALIAS_BEGIN("Tag0", "ref_pic_list_modification_flag_l0", 1, ref_pic_list_modification_flag[0] ? "1" : "0",
							ref_pic_list_modification_flag[0]
							? "that reference picture list 0 is specified explicitly by a list of list_entry_l0[i] values"
							: "that reference picture list 0 is determined implicitly", bit_offset ? *bit_offset : -1LL, "I");

						uint8_t index_bits = quick_ceil_log2(ptr_slice_header->NumPicTotalCurr);
						if (ref_pic_list_modification_flag[0])
						{
							NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag00", "for(i=0;i&lt;=num_ref_idx_l0_active_minus1;i++)", "");
							for (uint8_t rps_idx = 0; rps_idx < ptr_slice_header->num_ref_idx_l0_active_minus1; rps_idx++)
							{
								NAV_ARRAY_FIELD_PROP_2NUMBER("list_entry_l0", rps_idx, index_bits, list_entry[0][rps_idx], "the index of the reference picture in RefPicListTemp0 to be placed at the current position of reference picture list 0");
							}
							NAV_WRITE_TAG_END("Tag00");
						}

						NAV_WRITE_TAG_END("Tag0");

						if (ptr_slice_header->slice_type == B_SLICE)
						{
							NAV_FIELD_PROP_WITH_ALIAS_BEGIN("Tag1", "ref_pic_list_modification_flag_l1", 1, ref_pic_list_modification_flag[1] ? "1" : "0",
								ref_pic_list_modification_flag[1]
								? "that reference picture list 1 is specified explicitly by a list of list_entry_l1[i] values"
								: "that reference picture list 1 is determined implicitly", bit_offset ? *bit_offset : -1LL, "I");

							uint8_t index_bits = quick_ceil_log2(ptr_slice_header->NumPicTotalCurr);
							if (ref_pic_list_modification_flag[1])
							{
								NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag10", "for(i=0;i&lt;=num_ref_idx_l1_active_minus1;i++)", "");
								for (uint8_t rps_idx = 0; rps_idx < ptr_slice_header->num_ref_idx_l1_active_minus1; rps_idx++)
								{
									NAV_ARRAY_FIELD_PROP_2NUMBER("list_entry_l1", rps_idx, index_bits, list_entry[1][rps_idx], "the index of the reference picture in RefPicListTemp1 to be placed at the current position of reference picture list 1");
								}
								NAV_WRITE_TAG_END("Tag10");
							}

							NAV_WRITE_TAG_END("Tag1");
						}

						DECLARE_FIELDPROP_END()

					}PACKED;

					struct PRED_WEIGHT_TABLE : public SYNTAX_BITSTREAM_MAP
					{
						struct WEIGHTING_FACTORS
						{
							uint8_t				luma_weight_flag;
							int8_t				delta_luma_weight;
							int8_t				luma_offset;
							uint8_t				chroma_weight_flag;
							int8_t				delta_chroma_weight[2];
							int8_t				delta_chroma_offset[2];
						}PACKED;

						uint8_t				luma_log2_weight_denom;
						int8_t				delta_chroma_log2_weight_denom;

						WEIGHTING_FACTORS*		weight_factors[2];
						SLICE_SEGMENT_HEADER*	ptr_slice_header;
						uint8_t					m_ChromaArrayType;

						PRED_WEIGHT_TABLE(SLICE_SEGMENT_HEADER *pSliceHdr, uint8_t ChromaArrayType)
							: ptr_slice_header(pSliceHdr)
							, m_ChromaArrayType(ChromaArrayType) {
							memset(weight_factors, 0, sizeof(weight_factors));
						}

						virtual ~PRED_WEIGHT_TABLE() {
							AMP_SAFEDELA2(weight_factors[0]);
							AMP_SAFEDELA2(weight_factors[1]);
						}

						int Map(AMBst in_bst)
						{
							int iRet = RET_CODE_SUCCESS;
							SYNTAX_BITSTREAM_MAP::Map(in_bst);

							try
							{
								MAP_BST_BEGIN(0);
								nal_read_ue(in_bst, luma_log2_weight_denom, uint8_t);
								if (m_ChromaArrayType != 0) {
									nal_read_se(in_bst, delta_chroma_log2_weight_denom, int8_t);
								}

								AMP_NEW0(weight_factors[0], WEIGHTING_FACTORS, (size_t)ptr_slice_header->num_ref_idx_l0_active_minus1 + 1);
								if (weight_factors[0] == nullptr)
									throw AMException(RET_CODE_OUTOFMEMORY);

								for (uint16_t i = 0; i <= ptr_slice_header->num_ref_idx_l0_active_minus1; i++) {
									nal_read_u(in_bst, weight_factors[0][i].luma_weight_flag, 1, uint8_t);
								}
								if (m_ChromaArrayType != 0) {
									for (uint16_t i = 0; i <= ptr_slice_header->num_ref_idx_l0_active_minus1; i++) {
										nal_read_u(in_bst, weight_factors[0][i].chroma_weight_flag, 1, uint8_t);
									}
								}
								for (uint16_t i = 0; i <= ptr_slice_header->num_ref_idx_l0_active_minus1; i++)
								{
									if (weight_factors[0][i].luma_weight_flag)
									{
										nal_read_se(in_bst, weight_factors[0][i].delta_luma_weight, int8_t);
										nal_read_se(in_bst, weight_factors[0][i].luma_offset, int8_t);
									}

									if (weight_factors[0][i].chroma_weight_flag)
									{
										for (uint8_t j = 0; j < 2; j++)
										{
											nal_read_se(in_bst, weight_factors[0][i].delta_chroma_weight[j], int8_t);
											nal_read_se(in_bst, weight_factors[0][i].delta_chroma_offset[j], int8_t);
										}
									}
								}

								if (ptr_slice_header->slice_type == 0)
								{
									AMP_NEW0(weight_factors[1], WEIGHTING_FACTORS, (size_t)ptr_slice_header->num_ref_idx_l1_active_minus1 + 1);
									if (weight_factors[1] == nullptr)
										throw AMException(RET_CODE_OUTOFMEMORY);

									for (uint16_t i = 0; i <= ptr_slice_header->num_ref_idx_l1_active_minus1; i++) {
										nal_read_u(in_bst, weight_factors[1][i].luma_weight_flag, 1, uint8_t);
									}

									if (m_ChromaArrayType != 0) {
										for (uint16_t i = 0; i <= ptr_slice_header->num_ref_idx_l1_active_minus1; i++) {
											nal_read_u(in_bst, weight_factors[1][i].chroma_weight_flag, 1, uint8_t);
										}
									}

									for (uint16_t i = 0; i <= ptr_slice_header->num_ref_idx_l1_active_minus1; i++)
									{
										if (weight_factors[1][i].luma_weight_flag)
										{
											nal_read_se(in_bst, weight_factors[1][i].delta_luma_weight, int8_t);
											nal_read_se(in_bst, weight_factors[1][i].luma_offset, int8_t);
										}

										if (weight_factors[1][i].chroma_weight_flag)
										{
											for (uint8_t j = 0; j < 2; j++)
											{
												nal_read_se(in_bst, weight_factors[1][i].delta_chroma_weight[j], int8_t);
												nal_read_se(in_bst, weight_factors[1][i].delta_chroma_offset[j], int8_t);
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

						int Unmap(AMBst out_bst)
						{
							UNREFERENCED_PARAMETER(out_bst);
							return RET_CODE_ERROR_NOTIMPL;
						}

						DECLARE_FIELDPROP_BEGIN()
						BST_FIELD_PROP_UE(luma_log2_weight_denom, "the base 2 logarithm of the denominator for all luma weighting factors");
						if (m_ChromaArrayType != 0)
						{
							BST_FIELD_PROP_UE(delta_chroma_log2_weight_denom, "the difference of the base 2 logarithm of the denominator for all chroma weighting factors");
						}

						NAV_WRITE_TAG_BEGIN_WITH_ALIAS_DESC_F("list", "for(i=0;i&lt;=num_ref_idx_l0_active_minus1;i++)", "%u luma_weight_l0_flags in list", ptr_slice_header->num_ref_idx_l0_active_minus1);
						for (uint16_t ref_idx = 0; i <= ptr_slice_header->num_ref_idx_l0_active_minus1; i++)
						{
							MBCSPRINTF_S(szTemp4, TEMP4_SIZE, "luma_weight_l0_flag[%u]", ref_idx);
							BST_FIELD_PROP_NUMBER_WITH_ALIAS_BEGIN("luma_weight_l0_flag", szTemp4, 1, weight_factors[0][ref_idx].luma_weight_flag,
								"weighting factors for the luma component of list 0 prediction are present or not");
							BST_FIELD_PROP_NUMBER_END("luma_weight_l0_flag");
						}
						NAV_WRITE_TAG_END("list");

						if (m_ChromaArrayType != 0) {
							NAV_WRITE_TAG_BEGIN_WITH_ALIAS_DESC_F("list", "for(i=0;i&lt;=num_ref_idx_l0_active_minus1;i++)", "%u chroma_weight_l0_flags in list", ptr_slice_header->num_ref_idx_l0_active_minus1);
							for (uint16_t ref_idx = 0; i <= ptr_slice_header->num_ref_idx_l0_active_minus1; i++)
							{
								MBCSPRINTF_S(szTemp4, TEMP4_SIZE, "chroma_weight_l0_flag[%u]", ref_idx);
								BST_FIELD_PROP_NUMBER_WITH_ALIAS_BEGIN("chroma_weight_l0_flag", szTemp4, 1, weight_factors[0][ref_idx].chroma_weight_flag,
									"weighting factors for the chroma prediction values of list 0 prediction are present or not");
								BST_FIELD_PROP_NUMBER_END("chroma_weight_l0_flag");
							}
							NAV_WRITE_TAG_END("list");
						}

						NAV_WRITE_TAG_BEGIN_WITH_ALIAS_DESC_F("list", "for(i=0;i&lt;=num_ref_idx_l0_active_minus1;i++)", "%u luma_weight_l0_flags in list", ptr_slice_header->num_ref_idx_l0_active_minus1);
						for (uint16_t ref_idx = 0; i <= ptr_slice_header->num_ref_idx_l0_active_minus1; i++)
						{
							if (weight_factors[0][ref_idx].luma_weight_flag)
							{
								long long field_bits = 0;
								MBCSPRINTF_S(szTemp4, TEMP4_SIZE, "delta_luma_weight_l0[%u]", ref_idx);
								field_bits = (long long)quick_log2((weight_factors[0][ref_idx].delta_luma_weight >= 0 ? weight_factors[0][ref_idx].delta_luma_weight : ((-weight_factors[0][ref_idx].delta_luma_weight) + 1)) + 1) * 2 + 1;
								BST_FIELD_PROP_2NUMBER_WITH_ALIAS(szTemp4, field_bits, weight_factors[0][ref_idx].delta_luma_weight,
									"the difference of the weighting factor applied to the luma prediction value for list 0 prediction using RefPicList0[i]");
								MBCSPRINTF_S(szTemp4, TEMP4_SIZE, "luma_offset_l0[%u]", ref_idx);
								field_bits = (long long)quick_log2((weight_factors[0][ref_idx].luma_offset >= 0 ? weight_factors[0][ref_idx].luma_offset : ((-weight_factors[0][ref_idx].luma_offset) + 1)) + 1) * 2 + 1;
								BST_FIELD_PROP_2NUMBER_WITH_ALIAS(szTemp4, field_bits, weight_factors[0][ref_idx].luma_offset,
									"the additive offset applied to the luma prediction value for list 0 prediction using RefPicList0[i]");
							}

							if (weight_factors[0][ref_idx].chroma_weight_flag)
							{
								for (uint8_t j = 0; j < 2; j++)
								{
									long long field_bits = 0;
									MBCSPRINTF_S(szTemp4, TEMP4_SIZE, "delta_chroma_weight_l0[%u][%u]", ref_idx, j);
									field_bits = (long long)quick_log2((weight_factors[0][ref_idx].delta_chroma_weight[j] >= 0 ? weight_factors[0][ref_idx].delta_chroma_weight[j] : ((-weight_factors[0][ref_idx].delta_chroma_weight[j]) + 1)) + 1) * 2 + 1;
									BST_FIELD_PROP_2NUMBER_WITH_ALIAS(szTemp4, field_bits,
										weight_factors[0][ref_idx].delta_chroma_weight[j], j == 0 ?
										"the difference of the weighting factor applied to the chroma prediction values for list 0 prediction using RefPicList0[i] for Cb" :
										"the difference of the weighting factor applied to the chroma prediction values for list 0 prediction using RefPicList0[i] for Cr");
									MBCSPRINTF_S(szTemp4, TEMP4_SIZE, "delta_chroma_offset_l0[%u][%u]", ref_idx, j);
									field_bits = (long long)quick_log2((weight_factors[0][ref_idx].delta_chroma_offset[j] >= 0 ? weight_factors[0][ref_idx].delta_chroma_offset[j] : ((-weight_factors[0][ref_idx].delta_chroma_offset[j]) + 1)) + 1) * 2 + 1;
									BST_FIELD_PROP_2NUMBER_WITH_ALIAS(szTemp4, field_bits,
										weight_factors[0][ref_idx].delta_chroma_offset[j], j == 0 ?
										"the difference of the additive offset applied to the chroma prediction values for list 0 prediction using RefPicList0[i] for Cb" :
										"the difference of the additive offset applied to the chroma prediction values for list 0 prediction using RefPicList0[i] for Cr");
								}
							}
						}
						NAV_WRITE_TAG_END("list");

						if (ptr_slice_header->slice_type == 0)
						{
							NAV_WRITE_TAG_BEGIN_WITH_ALIAS_DESC_F("list", "for(i=0;i&lt;=num_ref_idx_l1_active_minus1;i++)", "%u luma_weight_l1_flags in list", ptr_slice_header->num_ref_idx_l1_active_minus1);
							for (uint16_t ref_idx = 0; i <= ptr_slice_header->num_ref_idx_l1_active_minus1; i++)
							{
								MBCSPRINTF_S(szTemp4, TEMP4_SIZE, "luma_weight_l1_flag[%u]", ref_idx);
								BST_FIELD_PROP_NUMBER_WITH_ALIAS_BEGIN("luma_weight_l1_flag", szTemp4, 1, weight_factors[1][ref_idx].luma_weight_flag,
									"weighting factors for the luma component of list 1 prediction are present or not");
								BST_FIELD_PROP_NUMBER_END("luma_weight_l0_flag");
							}
							NAV_WRITE_TAG_END("list");

							if (m_ChromaArrayType != 0) {
								NAV_WRITE_TAG_BEGIN_WITH_ALIAS_DESC_F("list", "for(i=0;i&lt;=num_ref_idx_l1_active_minus1;i++)", "%u chroma_weight_l1_flags in list", ptr_slice_header->num_ref_idx_l1_active_minus1);
								for (uint16_t ref_idx = 0; i <= ptr_slice_header->num_ref_idx_l1_active_minus1; i++)
								{
									MBCSPRINTF_S(szTemp4, TEMP4_SIZE, "chroma_weight_l1_flag[%u]", ref_idx);
									BST_FIELD_PROP_NUMBER_WITH_ALIAS_BEGIN("chroma_weight_l1_flag", szTemp4, 1, weight_factors[1][ref_idx].chroma_weight_flag,
										"weighting factors for the chroma prediction values of list 1 prediction are present or not");
									BST_FIELD_PROP_NUMBER_END("chroma_weight_l1_flag");
								}
								NAV_WRITE_TAG_END("list");
							}

							NAV_WRITE_TAG_BEGIN_WITH_ALIAS_DESC_F("list", "for(i=0;i&lt;=num_ref_idx_l1_active_minus1;i++)", "%u luma_weight_l1_flags in list", ptr_slice_header->num_ref_idx_l1_active_minus1);
							for (uint16_t ref_idx = 0; i <= ptr_slice_header->num_ref_idx_l1_active_minus1; i++)
							{
								if (weight_factors[1][ref_idx].luma_weight_flag)
								{
									long long field_bits = 0;
									MBCSPRINTF_S(szTemp4, TEMP4_SIZE, "delta_luma_weight_l1[%u]", ref_idx);
									field_bits = (long long)quick_log2((weight_factors[1][ref_idx].delta_luma_weight >= 0 ? weight_factors[1][ref_idx].delta_luma_weight : ((-weight_factors[1][ref_idx].delta_luma_weight) + 1)) + 1) * 2 + 1;
									BST_FIELD_PROP_2NUMBER_WITH_ALIAS(szTemp4, field_bits, weight_factors[1][ref_idx].delta_luma_weight,
										"the difference of the weighting factor applied to the luma prediction value for list 1 prediction using RefPicList0[i]");
									MBCSPRINTF_S(szTemp4, TEMP4_SIZE, "luma_offset_l1[%u]", ref_idx);
									field_bits = (long long)quick_log2((weight_factors[1][ref_idx].luma_offset >= 0 ? weight_factors[1][ref_idx].luma_offset : ((-weight_factors[1][ref_idx].luma_offset) + 1)) + 1) * 2 + 1;
									BST_FIELD_PROP_2NUMBER_WITH_ALIAS(szTemp4, field_bits, weight_factors[1][ref_idx].luma_offset,
										"the additive offset applied to the luma prediction value for list 1 prediction using RefPicList0[i]");
								}

								if (weight_factors[1][ref_idx].chroma_weight_flag)
								{
									for (uint8_t j = 0; j < 2; j++)
									{
										long long field_bits = 0;
										MBCSPRINTF_S(szTemp4, TEMP4_SIZE, "delta_chroma_weight_l1[%u][%u]", ref_idx, j);
										field_bits = (long long)quick_log2((weight_factors[1][ref_idx].delta_chroma_weight[j] >= 0 ? weight_factors[1][ref_idx].delta_chroma_weight[j] : ((-weight_factors[1][ref_idx].delta_chroma_weight[j]) + 1)) + 1) * 2 + 1;
										BST_FIELD_PROP_2NUMBER_WITH_ALIAS(szTemp4, field_bits,
											weight_factors[1][ref_idx].delta_chroma_weight[j], j == 0 ?
											"the difference of the weighting factor applied to the chroma prediction values for list 1 prediction using RefPicList0[i] for Cb" :
											"the difference of the weighting factor applied to the chroma prediction values for list 1 prediction using RefPicList0[i] for Cr");
										MBCSPRINTF_S(szTemp4, TEMP4_SIZE, "delta_chroma_offset_l1[%u][%u]", ref_idx, j);
										field_bits = (long long)quick_log2((weight_factors[1][ref_idx].delta_chroma_offset[j] >= 0 ? weight_factors[1][ref_idx].delta_chroma_offset[j] : ((-weight_factors[1][ref_idx].delta_chroma_offset[j]) + 1)) + 1) * 2 + 1;
										BST_FIELD_PROP_2NUMBER_WITH_ALIAS(szTemp4, field_bits,
											weight_factors[1][ref_idx].delta_chroma_offset[j], j == 0 ?
											"the difference of the additive offset applied to the chroma prediction values for list 1 prediction using RefPicList0[i] for Cb" :
											"the difference of the additive offset applied to the chroma prediction values for list 1 prediction using RefPicList0[i] for Cr");
									}
								}
							}
							NAV_WRITE_TAG_END("list");
						}
						DECLARE_FIELDPROP_END()
					}PACKED;

					uint8_t		first_slice_segment_in_pic_flag : 1;
					uint8_t		no_output_of_prior_pics_flag : 1;
					uint8_t		slice_pic_parameter_set_id : 6;

					uint8_t		slice_type;

					uint8_t		pic_output_flag : 1;
					uint8_t		colour_plane_id : 2;
					uint8_t		short_term_ref_pic_set_sps_flag : 1;
					uint8_t		byte_align_0 : 4;

					uint8_t		short_term_ref_pic_set_idx;

					uint32_t	dependent_slice_segment_flag : 1;
					uint32_t	slice_segment_address : 31;		// Ceil(Log2(PicSizeInCtbsY)) bits

					CAMBitArray	slice_reserved_flags;
					ST_REF_PIC_SETS::ST_REF_PIC_SET*
								st_ref_pic_set = NULL;

					uint8_t		num_long_term_sps;

					uint8_t		num_long_term_pics;

					uint8_t		slice_temporal_mvp_enabled_flag : 1;
					uint8_t		slice_sao_luma_flag : 1;
					uint8_t		slice_sao_chroma_flag : 1;
					uint8_t		num_ref_idx_active_override_flag : 1;
					uint8_t		mvd_l1_zero_flag : 1;
					uint8_t		cabac_init_flag : 1;
					uint8_t		collocated_from_l0_flag : 1;
					uint8_t		byte_align_1 : 1;

					uint8_t		num_ref_idx_l0_active_minus1 : 4;
					uint8_t		num_ref_idx_l1_active_minus1 : 4;

					uint32_t	slice_pic_order_cnt_lsb;

					std::vector<uint32_t>
								lt_idx_sps;
					std::vector<uint32_t>
								poc_lsb_lt;
					CAMBitArray	used_by_curr_pic_lt_flag;
					CAMBitArray	delta_poc_msb_present_flag;
					std::vector<uint32_t>
								delta_poc_msb_cycle_lt;

					REF_PIC_LISTS_MODIFICATION*
								ref_pic_lists_modification = NULL;
					PRED_WEIGHT_TABLE*
								pred_weight_table = NULL;

					uint8_t		collocated_ref_idx : 4;
					uint8_t		five_minus_max_num_merge_cand : 4;
					int8_t		slice_qp_delta = 0;
					int8_t		slice_cb_qp_offset = 0;
					int8_t		slice_cr_qp_offset = 0;

					uint8_t		cu_chroma_qp_offset_enabled_flag : 1;
					uint8_t		deblocking_filter_override_flag : 1;
					uint8_t		slice_deblocking_filter_disabled_flag : 1;
					uint8_t		slice_loop_filter_across_slices_enabled_flag : 1;
					uint8_t		byte_align_2 : 4;

					int8_t		slice_beta_offset_div2;
					int8_t		slice_tc_offset_div2;

					uint8_t		offset_len_minus1;

					uint32_t	num_entry_point_offsets;

					std::vector<uint32_t>
								entry_point_offset_minus1;

					uint16_t	slice_segment_header_extension_length;
					std::vector<uint8_t>
								slice_segment_header_extension_data_byte;

					H265_NU		sp_pps;
					NAL_UNIT*	ptr_nal_unit = NULL;
					uint8_t		CurrRpsIdx;
					uint8_t		NumPicTotalCurr;

					SLICE_SEGMENT_HEADER(NAL_UNIT* pNALUnit)
						: pic_output_flag(1)
						, short_term_ref_pic_set_idx(0)
						, dependent_slice_segment_flag(0)
						, slice_segment_address(0)
						, num_long_term_sps(0)
						, num_long_term_pics(0)
						, cabac_init_flag(0)
						, collocated_from_l0_flag(1)
						, slice_pic_order_cnt_lsb(0)
						, collocated_ref_idx(0)
						, slice_cb_qp_offset(0)
						, slice_cr_qp_offset(0)
						, cu_chroma_qp_offset_enabled_flag(0)
						, deblocking_filter_override_flag(0)
						, num_entry_point_offsets(0)
						, slice_segment_header_extension_length(0)
						, ptr_nal_unit(pNALUnit)
					{
					}

					~SLICE_SEGMENT_HEADER()
					{
						AMP_SAFEDEL2(ref_pic_lists_modification);
						AMP_SAFEDEL2(pred_weight_table);
						AMP_SAFEDEL2(st_ref_pic_set);
					}

					int Map(AMBst in_bst)
					{
						int iRet = RET_CODE_SUCCESS;
						SYNTAX_BITSTREAM_MAP::Map(in_bst);

						try
						{
							MAP_BST_BEGIN(0);

							nal_read_u(in_bst, first_slice_segment_in_pic_flag, 1, uint8_t);

							if (ptr_nal_unit->nal_unit_header.nal_unit_type >= BLA_W_LP && ptr_nal_unit->nal_unit_header.nal_unit_type <= RSV_IRAP_VCL23)
								nal_read_u(in_bst, no_output_of_prior_pics_flag, 1, uint8_t);

							nal_read_ue(in_bst, slice_pic_parameter_set_id, uint8_t);

							sp_pps = ptr_nal_unit->ptr_ctx_video_bst->GetHEVCPPS(slice_pic_parameter_set_id);
							if (!sp_pps || sp_pps->ptr_pic_parameter_set_rbsp == nullptr)
							{
								printf("[H265] Picture Parameter Set with slice_pic_parameter_set_id: %d is not available.\n", slice_pic_parameter_set_id);
								throw AMException(RET_CODE_ERROR_NOTIMPL);
							}

							auto ptr_pps = sp_pps->ptr_pic_parameter_set_rbsp;
							num_ref_idx_l0_active_minus1 = ptr_pps->num_ref_idx_l0_default_active_minus1;
							num_ref_idx_l1_active_minus1 = ptr_pps->num_ref_idx_l1_default_active_minus1;

							assert(ptr_pps->pps_seq_parameter_set_id >= 0 && ptr_pps->pps_seq_parameter_set_id <= 15);
							ptr_nal_unit->ptr_ctx_video_bst->ActivateSPS((int8_t)ptr_pps->pps_seq_parameter_set_id);

							auto sp_sps = ptr_nal_unit->ptr_ctx_video_bst->GetHEVCSPS(ptr_pps->pps_seq_parameter_set_id);
							if (!sp_sps || sp_sps->ptr_seq_parameter_set_rbsp == nullptr)
							{
								printf("[H265] Sequence Parameter Set with pps_seq_parameter_set_id: %d is not available.\n", ptr_pps->pps_seq_parameter_set_id);
								throw AMException(RET_CODE_ERROR_NOTIMPL);
							}

							auto ptr_sps = sp_sps->ptr_seq_parameter_set_rbsp;
							uint8_t ChromaArrayType = ptr_sps->separate_colour_plane_flag == 0 ? ptr_sps->chroma_format_idc : 0;
							if (!first_slice_segment_in_pic_flag)
							{
								if (ptr_pps->dependent_slice_segments_enabled_flag)
								{
									nal_read_u(in_bst, dependent_slice_segment_flag, 1, uint32_t);
								}
								nal_read_u(in_bst, slice_segment_address, quick_ceil_log2(ptr_sps->PicSizeInCtbsY), uint32_t);
							}

							if (!dependent_slice_segment_flag)
							{
								for (int i = 0; i < ptr_pps->num_extra_slice_header_bits; i++)
									nal_read_bitarray(in_bst, slice_reserved_flags, i);
								nal_read_ue(in_bst, slice_type, uint8_t);
								if (ptr_pps->output_flag_present_flag)
									nal_read_u(in_bst, pic_output_flag, 1, uint8_t);

								if (ptr_sps->separate_colour_plane_flag)
									nal_read_u(in_bst, colour_plane_id, 2, uint8_t);

								if (ptr_nal_unit->nal_unit_header.nal_unit_type != IDR_W_RADL && ptr_nal_unit->nal_unit_header.nal_unit_type != IDR_N_LP) {
									nal_read_u(in_bst, slice_pic_order_cnt_lsb, ptr_sps->log2_max_pic_order_cnt_lsb_minus4 + 4, uint32_t);
									nal_read_u(in_bst, short_term_ref_pic_set_sps_flag, 1, uint8_t);

									if (!short_term_ref_pic_set_sps_flag) {
										nal_read_ref(in_bst, ptr_sps->st_ref_pic_sets->st_ref_pic_set[ptr_sps->num_short_term_ref_pic_sets],
											ST_REF_PIC_SETS::ST_REF_PIC_SET, ptr_sps->st_ref_pic_sets, ptr_sps->num_short_term_ref_pic_sets);
										st_ref_pic_set = ptr_sps->st_ref_pic_sets->st_ref_pic_set[ptr_sps->num_short_term_ref_pic_sets];
									}
									else if (ptr_sps->num_short_term_ref_pic_sets > 1) {
										nal_read_u(in_bst, short_term_ref_pic_set_idx, quick_ceil_log2(ptr_sps->num_short_term_ref_pic_sets), uint8_t);
									}

									if (ptr_sps->long_term_ref_pics_present_flag) {
										if (ptr_sps->num_long_term_ref_pics_sps > 0)
											nal_read_ue(in_bst, num_long_term_sps, uint8_t);

										nal_read_ue(in_bst, num_long_term_pics, uint8_t);

										lt_idx_sps.resize((size_t)num_long_term_sps + num_long_term_pics, 0);
										poc_lsb_lt.resize((size_t)num_long_term_sps + num_long_term_pics, 0);
										delta_poc_msb_cycle_lt.resize((size_t)num_long_term_sps + num_long_term_pics, 0);
										for (uint16_t i = 0; i < (uint16_t)num_long_term_sps + num_long_term_pics; i++) {
											if (i < num_long_term_sps) {
												if (ptr_sps->num_long_term_ref_pics_sps > 1)
													nal_read_u(in_bst, lt_idx_sps[i], quick_ceil_log2(ptr_sps->num_long_term_ref_pics_sps), uint32_t);
											}
											else {
												nal_read_u(in_bst, poc_lsb_lt[i], ptr_sps->log2_max_pic_order_cnt_lsb_minus4 + 4, uint32_t);
												nal_read_bitarray(in_bst, used_by_curr_pic_lt_flag, i);
											}
											nal_read_bitarray(in_bst, delta_poc_msb_present_flag, i);
											if (delta_poc_msb_present_flag[i])
												nal_read_ue(in_bst, delta_poc_msb_cycle_lt[i], uint32_t);
										}
									} // if (ptr_sps->long_term_ref_pics_present_flag)

									if (ptr_sps->sps_temporal_mvp_enabled_flag)
										nal_read_u(in_bst, slice_temporal_mvp_enabled_flag, 1, uint8_t);
								} // if (ptr_nal_unit->nal_unit_header.nal_unit_type != IDR_W_RADL && ptr_nal_unit->nal_unit_header.nal_unit_type != IDR_N_LP)

								if (ptr_sps->sample_adaptive_offset_enabled_flag) {
									nal_read_u(in_bst, slice_sao_luma_flag, 1, uint8_t);
									if (ChromaArrayType != 0)
										nal_read_u(in_bst, slice_sao_chroma_flag, 1, uint8_t);
								}

								if (slice_type == P_SLICE || slice_type == B_SLICE)
								{
									nal_read_u(in_bst, num_ref_idx_active_override_flag, 1, uint8_t);
									if (num_ref_idx_active_override_flag)
									{
										nal_read_ue(in_bst, num_ref_idx_l0_active_minus1, uint8_t);
										if (slice_type == B_SLICE)
											nal_read_ue(in_bst, num_ref_idx_l1_active_minus1, uint8_t);
									}

									if (short_term_ref_pic_set_sps_flag)
										CurrRpsIdx = short_term_ref_pic_set_idx;
									else
										CurrRpsIdx = ptr_sps->num_short_term_ref_pic_sets;

									NumPicTotalCurr = 0;
									auto ref_pic_set = ptr_sps->st_ref_pic_sets->st_ref_pic_set[CurrRpsIdx];
									for (uint8_t i = 0; i < ref_pic_set->NumNegativePics; i++)
										if (ref_pic_set->used_by_curr_pic_s0_flags[i])
											NumPicTotalCurr++;

									for (uint8_t i = 0; i < ref_pic_set->NumPositivePics; i++)
										if (ref_pic_set->used_by_curr_pic_s1_flags[i])
											NumPicTotalCurr++;

									for (uint8_t i = 0; i < num_long_term_sps + num_long_term_pics; i++)
									{
										BOOL UsedByCurrPicLt;
										if (i < num_long_term_sps)
											UsedByCurrPicLt = ptr_sps->used_by_curr_pic_lt_sps_flag[lt_idx_sps[i]];
										else
											UsedByCurrPicLt = used_by_curr_pic_lt_flag[i];
										if (UsedByCurrPicLt)
											NumPicTotalCurr++;
									}

									if (ptr_pps->lists_modification_present_flag && NumPicTotalCurr > 1)
										nal_read_ref(in_bst, ref_pic_lists_modification, REF_PIC_LISTS_MODIFICATION, this);

									if (slice_type == B_SLICE)
										nal_read_u(in_bst, mvd_l1_zero_flag, 1, uint8_t);
									if (ptr_pps->cabac_init_present_flag)
										nal_read_u(in_bst, cabac_init_flag, 1, uint8_t);
									if (slice_temporal_mvp_enabled_flag)
									{
										if (slice_type == B_SLICE)
											nal_read_u(in_bst, collocated_from_l0_flag, 1, uint8_t);

										if ((collocated_from_l0_flag && num_ref_idx_l0_active_minus1 > 0) ||
											(!collocated_from_l0_flag && num_ref_idx_l1_active_minus1 > 0))
											nal_read_ue(in_bst, collocated_ref_idx, uint8_t);
									}

									if ((ptr_pps->weighted_pred_flag && slice_type == P_SLICE) || (ptr_pps->weighted_bipred_flag && slice_type == B_SLICE))
										nal_read_ref(in_bst, pred_weight_table, PRED_WEIGHT_TABLE, this, ChromaArrayType);

									nal_read_ue(in_bst, five_minus_max_num_merge_cand, uint8_t);

								} // if (slice_type == P_SLICE || slice_type == B_SLICE)

								nal_read_se(in_bst, slice_qp_delta, int8_t);
								if (ptr_pps->pps_slice_chroma_qp_offsets_present_flag) {
									nal_read_se(in_bst, slice_cb_qp_offset, int8_t);
									nal_read_se(in_bst, slice_cr_qp_offset, int8_t);
								}

								if (ptr_pps->pps_range_extension && ptr_pps->pps_range_extension->chroma_qp_offset_list_enabled_flag)
									nal_read_u(in_bst, cu_chroma_qp_offset_enabled_flag, 1, uint8_t);

								if (ptr_pps->deblocking_filter_override_enabled_flag)
									nal_read_u(in_bst, deblocking_filter_override_flag, 1, uint8_t);

								if (ptr_pps->deblocking_filter_control_present_flag)
								{
									nal_read_u(in_bst, slice_deblocking_filter_disabled_flag, 1, uint8_t);
									if (!slice_deblocking_filter_disabled_flag)
									{
										nal_read_se(in_bst, slice_beta_offset_div2, int8_t);
										nal_read_se(in_bst, slice_tc_offset_div2, int8_t);
									}
									else
									{
										slice_beta_offset_div2 = ptr_pps->pps_beta_offset_div2;
										slice_tc_offset_div2 = ptr_pps->pps_tc_offset_div2;
									}
								}
								else
								{
									slice_deblocking_filter_disabled_flag = ptr_pps->pps_deblocking_filter_disabled_flag;
									slice_beta_offset_div2 = ptr_pps->pps_beta_offset_div2;
									slice_tc_offset_div2 = ptr_pps->pps_tc_offset_div2;
								}

								if (ptr_pps->pps_loop_filter_across_slices_enabled_flag && (slice_sao_luma_flag || slice_sao_chroma_flag || !slice_deblocking_filter_disabled_flag)) {
									nal_read_u(in_bst, slice_loop_filter_across_slices_enabled_flag, 1, uint8_t);
								}
								else {
									slice_loop_filter_across_slices_enabled_flag = ptr_pps->pps_loop_filter_across_slices_enabled_flag;
								}
							} // if (!dependent_slice_segment_flag)

							if (ptr_pps->tiles_enabled_flag || ptr_pps->entropy_coding_sync_enabled_flag)
							{
								nal_read_ue(in_bst, num_entry_point_offsets, uint32_t);
								if (num_entry_point_offsets > 0)
								{
									entry_point_offset_minus1.resize(num_entry_point_offsets, 0);
									nal_read_ue(in_bst, offset_len_minus1, uint8_t);
									for (uint32_t i = 0; i < num_entry_point_offsets; i++)
										nal_read_u(in_bst, entry_point_offset_minus1[i], offset_len_minus1 + 1, uint32_t);
								}
							} // if (ptr_pps->tiles_enabled_flag || ptr_pps->entropy_coding_sync_enabled_flag)

							if (ptr_pps->slice_segment_header_extension_present_flag)
							{
								nal_read_ue(in_bst, slice_segment_header_extension_length, uint16_t);
								slice_segment_header_extension_data_byte.resize(slice_segment_header_extension_length, 0);
								for (uint16_t i = 0; i < slice_segment_header_extension_length; i++)
									nal_read_u(in_bst, slice_segment_header_extension_data_byte[i], 8, uint8_t);
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

					int Unmap(AMBst out_bst)
					{
						UNREFERENCED_PARAMETER(out_bst);
						return RET_CODE_ERROR_NOTIMPL;
					}

					DECLARE_FIELDPROP_BEGIN()
					BST_FIELD_PROP_BOOL(first_slice_segment_in_pic_flag,
						"the slice segment is the first slice segment of the picture in decoding order",
						"the slice segment is not the first slice segment of the picture in decoding order");

					if (ptr_nal_unit->nal_unit_header.nal_unit_type >= BLA_W_LP && ptr_nal_unit->nal_unit_header.nal_unit_type <= RSV_IRAP_VCL23)
					{
						BST_FIELD_PROP_NUMBER1(no_output_of_prior_pics_flag, 1, "affects the output of previously-decoded pictures in the decoded picture buffer after the decoding of an IDR or a BLA picture that is not the first picture in the bitstream as specified in Annex C");
					}

					BST_FIELD_PROP_UE(slice_pic_parameter_set_id, "the value of pps_pic_parameter_set_id for the PPS in use");

					H265_NU sp_sps;
					if (!sp_pps || sp_pps->ptr_pic_parameter_set_rbsp == nullptr)
						return cbRequired;

					auto ptr_pps = sp_pps->ptr_pic_parameter_set_rbsp;

					sp_sps = ptr_nal_unit->ptr_ctx_video_bst->GetHEVCSPS(ptr_pps->pps_seq_parameter_set_id);
					if (!sp_sps || sp_sps->ptr_seq_parameter_set_rbsp == NULL)
						return cbRequired;

					auto ptr_sps = sp_sps->ptr_seq_parameter_set_rbsp;
					uint8_t ChromaArrayType = ptr_sps->separate_colour_plane_flag == 0 ? ptr_sps->chroma_format_idc : 0;
					if (!first_slice_segment_in_pic_flag)
					{
						if (ptr_pps->dependent_slice_segments_enabled_flag)
						{
							BST_FIELD_PROP_BOOL(dependent_slice_segment_flag,
								"the value of each slice segment header syntax element that is not present is inferred to be equal to the value of the corresponding slice segment header syntax element in the slice header",
								"the value of each slice segment header syntax element that is present");
						}

						BST_FIELD_PROP_2NUMBER1(slice_segment_address, quick_ceil_log2(ptr_sps->PicSizeInCtbsY), "the address of the first coding tree block in the slice segment, in coding tree block raster scan of a picture");
					}

					if (!dependent_slice_segment_flag)
					{
						NAV_WRITE_TAG_BEGIN_WITH_ALIAS_DESC_F("Tag0", "for(i=0;i&lt;num_extra_slice_header_bits;i++)", "num_extra_slice_header_bits: %d", ptr_pps->num_extra_slice_header_bits);
						for (int i = 0; i < ptr_pps->num_extra_slice_header_bits; i++) {
							BST_ARRAY_FIELD_PROP_NUMBER1(slice_reserved_flags, i, 1, "has semantics and values that are reserved for future use by ITU-T | ISO/IEC.");
						}
						NAV_WRITE_TAG_END("Tag0");

						BST_FIELD_PROP_UE(slice_type, slice_type == 0 ? "B-Slice" : (slice_type == 1?"P-Slice": (slice_type == 2?"I-Slice":"Unknown Slice")));

						if (ptr_pps->output_flag_present_flag) {
							BST_FIELD_PROP_NUMBER1(pic_output_flag, 1, "affects the decoded picture output and removal processes as specified in Annex C");
						}
						else {
							NAV_WRITE_TAG_WITH_NUMBER_VALUE("pic_output_flag", pic_output_flag, "affects the decoded picture output and removal processes as specified in Annex C");
						}

						if (ptr_pps->output_flag_present_flag) {
							BST_FIELD_PROP_NUMBER1(colour_plane_id, 2, colour_plane_id==0?"Y colour plane":(colour_plane_id==1?"Cb colour plane":(colour_plane_id==2?"Cr colour plane":"Unknown")));
						}

						if (ptr_nal_unit->nal_unit_header.nal_unit_type != IDR_W_RADL && ptr_nal_unit->nal_unit_header.nal_unit_type != IDR_N_LP) {
							BST_FIELD_PROP_2NUMBER1(slice_pic_order_cnt_lsb, (long long)ptr_sps->log2_max_pic_order_cnt_lsb_minus4 + 4, "specifies the picture order count modulo MaxPicOrderCntLsb for the current picture");
							BST_FIELD_PROP_BOOL(short_term_ref_pic_set_sps_flag, 
								"specifies that the short-term RPS of the current picture is derived based on one of the st_ref_pic_set() syntax structures in the active SPS that is identified by the syntax element short_term_ref_pic_set_idx in the slice header", 
								"specifies that the short-term RPS of the current picture is derived based on the st_ref_pic_set() syntax structure that is directly included in the slice headers of the current picture");

							if (!short_term_ref_pic_set_sps_flag) {
								NAV_WRITE_TAG_BEGIN_WITH_ALIAS_DESC_F("Tag1", "st_ref_pic_set(num_short_term_ref_pic_sets)", "num_short_term_ref_pic_sets = %d", ptr_sps->num_short_term_ref_pic_sets);
								BST_FIELD_PROP_REF(st_ref_pic_set);
								NAV_WRITE_TAG_END("Tag1");
							}
							else if (ptr_sps->num_short_term_ref_pic_sets > 1) {
								BST_FIELD_PROP_2NUMBER1(short_term_ref_pic_set_idx, quick_ceil_log2(ptr_sps->num_short_term_ref_pic_sets), 
									"specifies the index, into the list of the st_ref_pic_set() syntax structures included in the active SPS, of the st_ref_pic_set() syntax structure that is used for derivation of the short-term RPS of the current picture");
							}

							if (ptr_sps->long_term_ref_pics_present_flag) {
								if (ptr_sps->num_long_term_ref_pics_sps > 0)
								{
									BST_FIELD_PROP_UE(num_long_term_sps, 
										"the number of entries in the long-term RPS of the current picture that are derived based on the candidate long-term reference pictures specified in the active SPS");
								}

								BST_FIELD_PROP_UE(num_long_term_pics, "the number of entries in the long-term RPS of the current picture that are directly signalled in the slice header");

								NAV_WRITE_TAG_BEGIN_WITH_ALIAS_DESC_F("Tag1", "for(i=0;i&lt;num_long_term_sps+num_long_term_pics;i++)", "num_long_term_sps + num_long_term_pics = %d", num_long_term_sps + num_long_term_pics);
								for (uint16_t i = 0; i < num_long_term_sps + num_long_term_pics; i++) {
									if (i < num_long_term_sps) {
										if (ptr_sps->num_long_term_ref_pics_sps > 1)
										{
											BST_ARRAY_FIELD_PROP_NUMBER1_F(lt_idx_sps, i, quick_ceil_log2(ptr_sps->num_long_term_ref_pics_sps), 
												"specifies an index, into the list of candidate long-term reference pictures specified in the active SPS, of the %d-th entry in the long-term RPS of the current picture",
												i);
										}
									}
									else {
										BST_ARRAY_FIELD_PROP_NUMBER1_F(poc_lsb_lt, i, (long long)ptr_sps->log2_max_pic_order_cnt_lsb_minus4 + 4, 
											"specifies the value of the picture order count modulo MaxPicOrderCntLsb of the %d-th entry in the long-term RPS of the current picture",
											i);
										BST_ARRAY_FIELD_PROP_NUMBER1_F(used_by_curr_pic_lt_flag, i, 1,
											used_by_curr_pic_lt_flag[i]
											? "that the %d-th entry in the long-term RPS of the current picture is used for reference by the current picture"
											: "that the %d-th entry in the long-term RPS of the current picture is not used for reference by the current picture",
											i);
									}
									BST_ARRAY_FIELD_PROP_NUMBER1_F(delta_poc_msb_present_flag, i, 1, 
										delta_poc_msb_present_flag[i]
										? "specifies that delta_poc_msb_cycle_lt[%d] is present"
										: "specifies that delta_poc_msb_cycle_lt[%d] is not present",
										i);
									if (delta_poc_msb_present_flag[i])
									{
										BST_ARRAY_FIELD_PROP_UE(delta_poc_msb_cycle_lt, i, 
											"is used to determine the value of the most significant bits of the picture order count value of the i-th entry in the long-term RPS of the current picture");
									}
								}
								NAV_WRITE_TAG_END("Tag1");
							} // if (ptr_sps->long_term_ref_pics_present_flag)

							if (ptr_sps->sps_temporal_mvp_enabled_flag) {
								BST_FIELD_PROP_BOOL(slice_temporal_mvp_enabled_flag,
									"no temporal motion vector predictor is used in decoding of the current picture", 
									"temporal motion vector predictors may be used in decoding of the current picture");
							}
						} // if (ptr_nal_unit->nal_unit_header.nal_unit_type != IDR_W_RADL && ptr_nal_unit->nal_unit_header.nal_unit_type != IDR_N_LP)

						if (ptr_sps->sample_adaptive_offset_enabled_flag) {
							BST_FIELD_PROP_BOOL(slice_sao_luma_flag, 
								"SAO is enabled for the luma component in the current slice", "SAO is disabled for the luma component in the current slice");
							if (ChromaArrayType != 0)
							{
								BST_FIELD_PROP_BOOL(slice_sao_chroma_flag, 
									"SAO is enabled for the chroma component in the current slice", "SAO is disabled for the chroma component in the current slice");
							}
						}

						if (slice_type == P_SLICE || slice_type == B_SLICE)
						{
							BST_FIELD_PROP_BOOL(num_ref_idx_active_override_flag,
								"the syntax element num_ref_idx_l0_active_minus1 is present for P and B slices and that the syntax element num_ref_idx_l1_active_minus1 is present for B slices", 
								"the syntax elements num_ref_idx_l0_active_minus1 and num_ref_idx_l1_active_minus1 are not present");

							if (num_ref_idx_active_override_flag)
							{
								BST_FIELD_PROP_UE(num_ref_idx_l0_active_minus1, "the maximum reference index for reference picture list 0 that may be used to decode the slice");
								if (slice_type == B_SLICE) {
									BST_FIELD_PROP_UE(num_ref_idx_l1_active_minus1, "the maximum reference index for reference picture list 1 that may be used to decode the slice");
								}
							}

							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(CurrRpsIdx, "Current Reference Picture Set index");
							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(NumPicTotalCurr, "");

							if (ptr_pps->lists_modification_present_flag && NumPicTotalCurr > 1) {
								BST_FIELD_PROP_REF1_1(ref_pic_lists_modification, "Reference picture list modification");
							}

							if (slice_type == B_SLICE) {
								BST_FIELD_PROP_BOOL(mvd_l1_zero_flag, 
									"the mvd_coding( x0, y0, 1 ) syntax structure is not parsed and MvdL1[ x0 ][ y0 ][ compIdx ] is set equal to 0 for compIdx = 0..1", 
									"the mvd_coding( x0, y0, 1 ) syntax structure is parsed");
							}

							if (ptr_pps->cabac_init_present_flag) {
								BST_FIELD_PROP_NUMBER1(cabac_init_flag, 1, "the method for determining the initialization table used in the initialization process for context variables");
							}

							if (slice_temporal_mvp_enabled_flag) {
								if (slice_type == B_SLICE){
									BST_FIELD_PROP_BOOL(collocated_from_l0_flag, 
										"the collocated picture used for temporal motion vector prediction is derived from reference picture list 0", 
										"the collocated picture used for temporal motion vector prediction is derived from reference picture list 1");
								}

								if ((collocated_from_l0_flag && num_ref_idx_l0_active_minus1 > 0) ||
									(!collocated_from_l0_flag && num_ref_idx_l1_active_minus1 > 0))
								{
									BST_FIELD_PROP_UE(collocated_ref_idx, "the reference index of the collocated picture used for temporal motion vector prediction");
								}
							}

							if ((ptr_pps->weighted_pred_flag && slice_type == P_SLICE) || (ptr_pps->weighted_bipred_flag && slice_type == B_SLICE)){
								BST_FIELD_PROP_REF1_1(pred_weight_table, "Weighted prediction parameters semantics");
							}

							BST_FIELD_PROP_UE(five_minus_max_num_merge_cand, "the maximum number of merging motion vector prediction (MVP) candidates supported in the slice subtracted from 5");
						} // if (slice_type == P_SLICE || slice_type == B_SLICE)

						BST_FIELD_PROP_SE(slice_qp_delta, "specifies the initial value of QpY to be used for the coding blocks in the slice until modified by the value of CuQpDeltaVal in the coding unit layer");
						NAV_WRITE_TAG_WITH_NUMBER_VALUE("SliceQpY", 26 + ptr_pps->init_qp_minus26 + slice_qp_delta, "SliceQpY = 26 + init_qp_minus26 + slice_qp_delta");

						if (ptr_pps->pps_slice_chroma_qp_offsets_present_flag) {
							BST_FIELD_PROP_SE(slice_cb_qp_offset, "a difference to be added to the value of pps_cb_qp_offset when determining the value of the Qp'Cb quantization parameter");
							BST_FIELD_PROP_SE(slice_cr_qp_offset, "a difference to be added to the value of pps_cr_qp_offset when determining the value of the Qp'Cr quantization parameter");
						}

						if (ptr_pps->pps_range_extension && ptr_pps->pps_range_extension->chroma_qp_offset_list_enabled_flag) {
							BST_FIELD_PROP_BOOL(cu_chroma_qp_offset_enabled_flag, 
								"the cu_chroma_qp_offset_flag may be present in the transform unit syntax", 
								"the cu_chroma_qp_offset_flag is not present in the transform unit syntax");
						}

						if (ptr_pps->deblocking_filter_override_enabled_flag) {
							BST_FIELD_PROP_BOOL(deblocking_filter_override_flag, 
								"specifies that deblocking parameters are present in the slice header", "specifies that deblocking parameters are not present in the slice header");
						}

						if (ptr_pps->deblocking_filter_control_present_flag) {
							BST_FIELD_PROP_BOOL(slice_deblocking_filter_disabled_flag,
								"specifies that the operation of the deblocking filter is not applied for the current slice", 
								"specifies that the operation of the deblocking filter is applied for the current slice");
							if (!slice_deblocking_filter_disabled_flag) {
								BST_FIELD_PROP_SE(slice_beta_offset_div2, "the deblocking parameter offsets for beta (divided by 2) for the current slice");
								BST_FIELD_PROP_SE(slice_tc_offset_div2, "the deblocking parameter offsets for tC (divided by 2) for the current slice");
							}
							else
							{
								NAV_WRITE_TAG_WITH_1NUMBER_VALUE1(slice_beta_offset_div2, "the deblocking parameter offsets for beta (divided by 2) for the current slice");
								NAV_WRITE_TAG_WITH_1NUMBER_VALUE1(slice_tc_offset_div2, "the deblocking parameter offsets for tC (divided by 2) for the current slice");
							}
						}
						else
						{
							NAV_WRITE_TAG_WITH_1NUMBER_VALUE1(slice_deblocking_filter_disabled_flag, slice_deblocking_filter_disabled_flag
								? "specifies that the operation of the deblocking filter is not applied for the current slice"
								: "specifies that the operation of the deblocking filter is applied for the current slice");
							NAV_WRITE_TAG_WITH_1NUMBER_VALUE1(slice_beta_offset_div2, "the deblocking parameter offsets for beta (divided by 2) for the current slice");
							NAV_WRITE_TAG_WITH_1NUMBER_VALUE1(slice_tc_offset_div2, "the deblocking parameter offsets for tC (divided by 2) for the current slice");
						}

						if (ptr_pps->pps_loop_filter_across_slices_enabled_flag && (slice_sao_luma_flag || slice_sao_chroma_flag || !slice_deblocking_filter_disabled_flag))
						{
							BST_FIELD_PROP_BOOL(slice_loop_filter_across_slices_enabled_flag, 
								"specifies that in-loop filtering operations may be performed across the left and upper boundaries of the current slice", 
								"specifies that in-loop operations are not performed across left and upper boundaries of the current slice");
						}
						else {
							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(slice_loop_filter_across_slices_enabled_flag, slice_loop_filter_across_slices_enabled_flag
								? "specifies that in-loop filtering operations may be performed across the left and upper boundaries of the current slice"
								: "specifies that in-loop operations are not performed across left and upper boundaries of the current slice");
						}
					} // if (!dependent_slice_segment_flag)

					if (ptr_pps->tiles_enabled_flag || ptr_pps->entropy_coding_sync_enabled_flag)
					{
						BST_FIELD_PROP_UE(num_entry_point_offsets, "the number of entry_point_offset_minus1[i] syntax elements in the slice header");
						if (num_entry_point_offsets > 0)
						{
							BST_FIELD_PROP_UE(offset_len_minus1, "plus 1 specifies the length, in bits, of the entry_point_offset_minus1[i] syntax elements");
							NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag2", "for(i=0;&lt;num_entry_point_offsets;i++)", "");
							for (uint32_t i = 0; i < num_entry_point_offsets; i++) {
								BST_ARRAY_FIELD_PROP_NUMBER1_F(entry_point_offset_minus1, i, (long long)offset_len_minus1 + 1, "plus 1 specifies the %d-th entry point offset in bytes", i);
							}
							NAV_WRITE_TAG_END("Tag2");
						}
					}

					if (ptr_pps->slice_segment_header_extension_present_flag)
					{
						BST_FIELD_PROP_UE(slice_segment_header_extension_length, "specifies the length of the slice segment header extension data in bytes, not including the bits used for signalling slice_segment_header_extension_length itself");
						NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag3", "for(i=0;&lt;slice_segment_header_extension_length;i++)", "");
						for (uint16_t i = 0; i < slice_segment_header_extension_length; i++) {
							BST_ARRAY_FIELD_PROP_NUMBER1(slice_segment_header_extension_data_byte, i, 8, "");
						}
						NAV_WRITE_TAG_END("Tag3");
					}

					DECLARE_FIELDPROP_END()

				};

				SLICE_SEGMENT_HEADER*	ptr_slice_header = NULL;

				NAL_UNIT*				ptr_nal_unit;

				SLICE_SEGMENT_LAYER_RBSP(NAL_UNIT* pNALUnit) : ptr_nal_unit(pNALUnit){}

				virtual ~SLICE_SEGMENT_LAYER_RBSP()
				{
					AMP_SAFEDEL2(ptr_slice_header);
				}

				int Map(AMBst in_bst)
				{
					int iRet = RET_CODE_SUCCESS;
					SYNTAX_BITSTREAM_MAP::Map(in_bst);

					try
					{
						MAP_BST_BEGIN(0);
						nal_read_ref(in_bst, ptr_slice_header, SLICE_SEGMENT_HEADER, ptr_nal_unit);
						MAP_BST_END();
					}
					catch (AMException e)
					{
						iRet = e.RetCode();
					}

					// After a slice is parsed or decoded the st_ref_pic_sets[num_short_term_ref_pic_sets] is not used any more
					// It is better to unlink it, otherwise it may affect the other slices
					if (ptr_slice_header != NULL && ptr_slice_header->sp_pps)
					{
						auto sp_sps = ptr_nal_unit->ptr_ctx_video_bst->GetHEVCSPS(ptr_slice_header->sp_pps->ptr_pic_parameter_set_rbsp->pps_seq_parameter_set_id);
						if (sp_sps && sp_sps->ptr_seq_parameter_set_rbsp)
						{
							auto ptr_sps = sp_sps->ptr_seq_parameter_set_rbsp;
							ptr_sps->st_ref_pic_sets->st_ref_pic_set[ptr_sps->num_short_term_ref_pic_sets] = NULL;
						}
					}

					return iRet;
				}

				int Unmap(AMBst out_bst)
				{
					UNREFERENCED_PARAMETER(out_bst);
					return RET_CODE_ERROR_NOTIMPL;
				}

				DECLARE_FIELDPROP_BEGIN()
					NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag1", "slice_segment_header", "");
					BST_FIELD_PROP_REF(ptr_slice_header);
					NAV_WRITE_TAG_END("Tag1");
				DECLARE_FIELDPROP_END()

			}PACKED;

			NAL_UNIT_HEADER		nal_unit_header;
			union
			{
				void*						ptr_rbsp = nullptr;
				VIDEO_PARAMETER_SET_RBSP*	ptr_video_parameter_set_rbsp;
				SEQ_PARAMETER_SET_RBSP*		ptr_seq_parameter_set_rbsp;
				ACCESS_UNIT_DELIMITER_RBSP*	ptr_access_unit_delimiter_rbsp;
				PIC_PARAMETER_SET_RBSP*		ptr_pic_parameter_set_rbsp;
				SEI_RBSP*					ptr_sei_rbsp;
				SLICE_SEGMENT_LAYER_RBSP*	ptr_slice_segment_layer_rbsp;
			};
			VideoBitstreamCtx*				ptr_ctx_video_bst = nullptr;

			virtual ~NAL_UNIT() {
				switch (nal_unit_header.nal_unit_type)
				{
				case TRAIL_N:
				case TRAIL_R:
				case TSA_N:
				case TSA_R:
				case STSA_N:
				case STSA_R:
				case RADL_N:
				case RADL_R:
				case RASL_N:
				case RASL_R:
					UNMAP_STRUCT_POINTER5(ptr_slice_segment_layer_rbsp);
					break;
				case RSV_VCL_N10:
					break;
				case RSV_VCL_N12:
					break;
				case RSV_VCL_N14:
					break;
				case RSV_VCL_R11:
					break;
				case RSV_VCL_R13:
					break;
				case RSV_VCL_R15:
					break;
				case BLA_W_LP:
				case BLA_W_RADL:
				case BLA_N_LP:
				case IDR_W_RADL:
				case IDR_N_LP:
				case CRA_NUT:
					UNMAP_STRUCT_POINTER5(ptr_slice_segment_layer_rbsp);
					break;
				case RSV_IRAP_VCL22:
					break;
				case RSV_IRAP_VCL23:
					break;
				case RSV_VCL24:
					break;
				case RSV_VCL31:
					break;
				case VPS_NUT:
					UNMAP_STRUCT_POINTER5(ptr_video_parameter_set_rbsp)
					break;
				case SPS_NUT:
					UNMAP_STRUCT_POINTER5(ptr_seq_parameter_set_rbsp)
					break;
				case PPS_NUT:
					UNMAP_STRUCT_POINTER5(ptr_pic_parameter_set_rbsp);
					break;
				case AUD_NUT:
					UNMAP_STRUCT_POINTER5(ptr_access_unit_delimiter_rbsp)
					break;
				case EOS_NUT:
					break;
				case EOB_NUT:
					break;
				case FD_NUT:
					break;
				case PREFIX_SEI_NUT:
				case SUFFIX_SEI_NUT:
					UNMAP_STRUCT_POINTER5(ptr_sei_rbsp);
					break;
				case RSV_NVCL41:
					break;
				case RSV_NVCL47:
					break;
				case UNSPEC48:
					break;
				case UNSPEC63:
					break;
				}
			}

			void UpdateCtx(VideoBitstreamCtx* ctx)
			{
				ptr_ctx_video_bst = ctx;
			}

			int Map(AMBst bst)
			{
				int iRet = RET_CODE_SUCCESS;
				SYNTAX_BITSTREAM_MAP::Map(bst);

				if (AMP_FAILED(iRet = nal_unit_header.Map(bst)))
					return iRet;

				//printf("[H265] %s.\n", hevc_nal_unit_type_descs[nal_unit_header.nal_unit_type]);

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
					case TRAIL_N:
					case TRAIL_R:
					case TSA_N:
					case TSA_R:
					case STSA_N:
					case STSA_R:
					case RADL_N:
					case RADL_R:
					case RASL_N:
					case RASL_R:
					case BLA_W_LP:
					case BLA_W_RADL:
					case BLA_N_LP:
					case IDR_W_RADL:
					case IDR_N_LP:
					case CRA_NUT:
						nal_read_ref(bst, ptr_slice_segment_layer_rbsp, SLICE_SEGMENT_LAYER_RBSP, this);
						break;
					case VPS_NUT:
						nal_read_ref(bst, ptr_video_parameter_set_rbsp, VIDEO_PARAMETER_SET_RBSP, ptr_ctx_video_bst);
						break;
					case SPS_NUT:
						nal_read_ref(bst, ptr_seq_parameter_set_rbsp, SEQ_PARAMETER_SET_RBSP, ptr_ctx_video_bst);
						break;
					case PPS_NUT:
						nal_read_ref(bst, ptr_pic_parameter_set_rbsp, PIC_PARAMETER_SET_RBSP);
						break;
					case AUD_NUT:
						nal_read_ref(bst, ptr_access_unit_delimiter_rbsp, ACCESS_UNIT_DELIMITER_RBSP);
						break;
					case EOS_NUT:
						break;
					case EOB_NUT:
						break;
					case FD_NUT:
						break;
					case PREFIX_SEI_NUT:
					case SUFFIX_SEI_NUT:
						nal_read_ref(bst, ptr_sei_rbsp, SEI_RBSP, nal_unit_header.nal_unit_type, ptr_ctx_video_bst);
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

			int Unmap(AMBst out_bst)
			{
				UNREFERENCED_PARAMETER(out_bst);
				return RET_CODE_ERROR_NOTIMPL;
			}

			DECLARE_FIELDPROP_BEGIN()
				NAV_WRITE_TAG_BEGIN2_1("NAL_UNIT", hevc_nal_unit_type_descs[nal_unit_header.nal_unit_type]);
				BST_FIELD_PROP_OBJECT(nal_unit_header);
				switch (nal_unit_header.nal_unit_type)
				{
				case TRAIL_N:
				case TRAIL_R:
				case TSA_N:
				case TSA_R:
				case STSA_N:
				case STSA_R:
				case RADL_N:
				case RADL_R:
				case RASL_N:
				case RASL_R:
					NAV_FIELD_PROP_REF(ptr_slice_segment_layer_rbsp);
					break;
				case RSV_VCL_N10:
					break;
				case RSV_VCL_N12:
					break;
				case RSV_VCL_N14:
					break;
				case RSV_VCL_R11:
					break;
				case RSV_VCL_R13:
					break;
				case RSV_VCL_R15:
					break;
				case BLA_W_LP:
				case BLA_W_RADL:
				case BLA_N_LP:
				case IDR_W_RADL:
				case IDR_N_LP:
				case CRA_NUT:
					NAV_FIELD_PROP_REF(ptr_slice_segment_layer_rbsp);
					break;
				case RSV_IRAP_VCL22:
					break;
				case RSV_IRAP_VCL23:
					break;
				case RSV_VCL24:
					break;
				case RSV_VCL31:
					break;
				case VPS_NUT:
					NAV_FIELD_PROP_REF(ptr_video_parameter_set_rbsp)
					break;
				case SPS_NUT:
					NAV_FIELD_PROP_REF(ptr_seq_parameter_set_rbsp);
					break;
				case PPS_NUT:
					NAV_FIELD_PROP_REF(ptr_pic_parameter_set_rbsp);
					break;
				case AUD_NUT:
					NAV_FIELD_PROP_REF(ptr_access_unit_delimiter_rbsp)
					break;
				case EOS_NUT:
					break;
				case EOB_NUT:
					break;
				case FD_NUT:
					break;
				case PREFIX_SEI_NUT:
				case SUFFIX_SEI_NUT:
					NAV_FIELD_PROP_REF(ptr_sei_rbsp);
					break;
				case RSV_NVCL41:
					break;
				case RSV_NVCL47:
					break;
				case UNSPEC48:
					break;
				case UNSPEC63:
					break;
				}
				NAV_WRITE_TAG_END2("NAL_UNIT");
			DECLARE_FIELDPROP_END()
		}PACKED;

		struct BYTE_STREAM_NAL_UNIT : public SYNTAX_BITSTREAM_MAP
		{
			std::vector<uint8_t>		leading_zero_8bits;
			uint8_t						has_zero_byte = 0;
			uint8_t						zero_byte = 0;
			uint32_t					start_code_prefix_one_3bytes = 0;
			NAL_UNIT					nal_unit;
			std::vector<uint8_t>		trailing_zero_8bits;
			VideoBitstreamCtx*			ctx_video_bst;
			uint32_t					NAL_Length_Delimiter_Size;
			uint32_t					NAL_Length = 0;

			BYTE_STREAM_NAL_UNIT(VideoBitstreamCtx* ctx = NULL, uint32_t cbNALLengthDelimiterSize=0) 
				: ctx_video_bst(ctx), NAL_Length_Delimiter_Size(cbNALLengthDelimiterSize) {
				nal_unit.UpdateCtx(ctx);
			}

			int Map(AMBst in_bst)
			{
				int iRet = RET_CODE_SUCCESS;
				uint8_t leading_zero_byte = 0;

				SYNTAX_BITSTREAM_MAP::Map(in_bst);

				try
				{
					if (AMP_FAILED(iRet = AMBst_Realign(in_bst)))
					{
						printf("error happened or data is insufficient in bitstream {retcode: %d}.\n", iRet);
						return iRet;
					}

					MAP_BST_BEGIN(0);

					if (NAL_Length_Delimiter_Size == 0)
					{
						uint32_t next_bits = (uint32_t)AMBst_PeekBits(in_bst, 32);
						while ((next_bits >> 8) != 0x000001 && next_bits != 0x00000001)
						{
							nal_read_f(in_bst, leading_zero_byte, 8, uint8_t);
							leading_zero_8bits.push_back(leading_zero_byte);
							next_bits = (uint32_t)AMBst_PeekBits(in_bst, 32);
						}

						if ((next_bits >> 8) != 0x000001)
						{
							nal_read_f(in_bst, zero_byte, 8, uint8_t);
							has_zero_byte = true;
						}

						nal_read_f(in_bst, start_code_prefix_one_3bytes, 24, uint32_t);
					}
					else
					{
						nal_read_b(in_bst, NAL_Length, NAL_Length_Delimiter_Size<<3, uint32_t);
					}

					if (AMP_FAILED(iRet = nal_unit.Map(in_bst)))
					{
						printf("[H265] Failed to map nal_unit {retcode: %d}.\n", iRet);
						// No more data in the current bit-stream, the current mapping is terminated, and deem it as "success"
						if (iRet == RET_CODE_NO_MORE_DATA) goto done;
					}
					
					map_status.number_of_fields++;
				}
				catch (AMException e)
				{
					return e.RetCode();
				}

				if (NAL_Length_Delimiter_Size == 0 && (ctx_video_bst == NULL || !ctx_video_bst->in_scanning))
				{
					// Process trailing zero bytes
					try
					{
						while ((uint32_t)AMBst_PeekBits(in_bst, 24) != 0x000001 && (uint32_t)AMBst_PeekBits(in_bst, 32) != 0x00000001)
						{
							nal_read_f(in_bst, leading_zero_byte, 8, uint8_t);
							if (iRet != RET_CODE_NOTHING_TODO)
								trailing_zero_8bits.push_back(leading_zero_byte);
						}
					}
					catch (AMException e)
					{
						// No more data in the current bit-stream, the current mapping is terminated, and deem it as "success"
						if (e.RetCode() == RET_CODE_NO_MORE_DATA)
						{
							MAP_BST_END();
						}
						return e.RetCode();
					}
				}

			done:
				MAP_BST_END();
				return iRet;
			}

			int Unmap(AMBst out_bst)
			{
				UNREFERENCED_PARAMETER(out_bst);
				return RET_CODE_ERROR_NOTIMPL;
			}

			DECLARE_FIELDPROP_BEGIN()
			if (NAL_Length_Delimiter_Size == 0)
			{
				NAV_WRITE_TAG_BEGIN2_1("byte_stream_nal_unit", (map_status.status == 0 || map_status.number_of_fields >= (int)leading_zero_8bits.size() + (has_zero_byte ? 1 : 0) + 4) ? hevc_nal_unit_type_descs[nal_unit.nal_unit_header.nal_unit_type] : "");
				if (leading_zero_8bits.size() > 0)
				{
					NAV_FIELD_PROP_FIXSIZE_BINSTR("leading_zero_8bits", 8 * (unsigned long)leading_zero_8bits.size(), leading_zero_8bits.data(), (unsigned long)leading_zero_8bits.size(), "equal to 0x00");
				}
				if (has_zero_byte)
				{
					NAV_FIELD_PROP_2NUMBER("zero_byte", 8, zero_byte, "equal to 0x00");
				}
				NAV_FIELD_PROP_2NUMBER("start_code_prefix_one_3bytes", 24, start_code_prefix_one_3bytes, "equal to 0x000001");
				NAV_FIELD_PROP_OBJECT(nal_unit);
				if (trailing_zero_8bits.size() > 0)
				{
					NAV_FIELD_PROP_FIXSIZE_BINSTR("trailing_zero_8bits", 8 * (unsigned long)trailing_zero_8bits.size(), trailing_zero_8bits.data(), (unsigned long)trailing_zero_8bits.size(), "equal to 0x00");
				}
				NAV_WRITE_TAG_END2("byte_stream_nal_unit");
			}
			else
			{
				NAV_FIELD_PROP_2NUMBER1(NAL_Length, NAL_Length_Delimiter_Size, "Indicates the length in bytes of the following NAL unit. The length field can be configured to be of 1, 2, or 4 bytes.");
				NAV_FIELD_PROP_OBJECT(nal_unit);
			}
			DECLARE_FIELDPROP_END()

		};

		struct CVideoBitstream : public SYNTAX_BITSTREAM_MAP
		{
			std::vector<BYTE_STREAM_NAL_UNIT*>		bst_nal_units;
			VideoBitstreamCtx						ctx_video_bst;

			// used by scanning process
			std::vector<int8_t>						nal_unit_types;
			uint32_t								nal_length_delimiter_size = 0;

			CVideoBitstream() {}
			CVideoBitstream(std::initializer_list<uint8_t> unit_type_filters){
				ctx_video_bst.nal_unit_type_filters = unit_type_filters;
				ctx_video_bst.prev_vps_video_parameter_set_id = -1;
			}
			virtual ~CVideoBitstream()
			{
				for (size_t i = 0; i<bst_nal_units.size(); i++)
				{
					AMP_SAFEDEL(bst_nal_units[i]);
				}
			}

			bool ExistBDMVKeyFrame()
			{
				bool bVPSExist = false, bSPSExist = false, bIDRExist = false, bCRAExist = false;
				for (size_t i = 0; i < nal_unit_types.size(); i++)
				{
					if (nal_unit_types[i] == VPS_NUT)
						bVPSExist = true;

					if (nal_unit_types[i] == SPS_NUT)
						bSPSExist = true;

					if (nal_unit_types[i] == IDR_W_RADL || nal_unit_types[i] == IDR_N_LP)
						bIDRExist = true;

					if (nal_unit_types[i] == CRA_NUT)
						bCRAExist = true;
				}
				return bVPSExist && bSPSExist && (bIDRExist || bCRAExist);
			}

			int Map(unsigned char* pBuf, unsigned long cbSize)
			{
				int iRet = RET_CODE_ERROR;
				unsigned char* pStartBuf = pBuf;
				unsigned long  cbBufSize = cbSize;

				AMBst bst = AMBst_CreateFromBuffer(pStartBuf, cbBufSize);

				while (cbSize >= 3)
				{
					// Find the start_code_prefix_one_3bytes
					while (cbSize >= 3 && !(pBuf[0] == 0 && pBuf[1] == 0 && pBuf[2] == 1))
					{
						cbSize--;
						pBuf++;
					}

					// 3 bytes (start code prefix) + 3 bytes (nal_header + 1 byte vps id)
					if (cbSize < 6)
						break;

					// check the nal_unit_type
					int8_t nal_unit_type = (pBuf[3] >> 1) & 0x3F;
					nal_unit_types.push_back(nal_unit_type);
					if (ctx_video_bst.nal_unit_type_filters.size() > 0)
					{
						if (std::find(ctx_video_bst.nal_unit_type_filters.begin(), ctx_video_bst.nal_unit_type_filters.end(), nal_unit_type) ==
							ctx_video_bst.nal_unit_type_filters.end())
						{
							cbSize -= 5;
							pBuf += 5;
							continue;
						}
					}

					AMP_NEWT1(ptr_bst_nal_unit, BYTE_STREAM_NAL_UNIT, &ctx_video_bst);
					AMBst_Seek(bst, (cbBufSize - cbSize) * 8);
					if (AMP_FAILED(iRet = ptr_bst_nal_unit->Map(bst)))
						printf("[H265] Failed to map byte stream NAL unit {retcode: %d}.\n", iRet);

					bst_nal_units.push_back(ptr_bst_nal_unit);
					ptr_bst_nal_unit = NULL;

					if (iRet == RET_CODE_NO_MORE_DATA)
					{
						AMBst_Destroy(bst);
						iRet = RET_CODE_SUCCESS;
						break;
					}

					int CurPos = AMBst_Tell(bst);
					if (CurPos < 0)
						break;

					CurPos = (CurPos + 7) / 8;

					if (cbSize < (unsigned long)CurPos)
						break;

					cbSize = cbBufSize - CurPos;
					pBuf = pStartBuf + CurPos;
				}
				AMBst_Destroy(bst);

				return RET_CODE_SUCCESS;
			}

			int MapNALLengthDelimiterBitstream(unsigned char* pBuf, unsigned long cbSize, uint32_t cbNALLengthDelimiterSize)
			{
				int iRet = RET_CODE_ERROR;
				unsigned char* pStartBuf = pBuf;
				unsigned long  cbBufSize = cbSize;

				if (cbNALLengthDelimiterSize != 1 && cbNALLengthDelimiterSize != 2 && cbNALLengthDelimiterSize != 4)
					return RET_CODE_INVALID_PARAMETER;

				nal_length_delimiter_size = cbNALLengthDelimiterSize;
				AMBst bst = AMBst_CreateFromBuffer(pStartBuf, cbBufSize);

				while (cbSize >= cbNALLengthDelimiterSize)
				{
					uint32_t nal_length = 0;
					for (uint32_t i = 0; i < cbNALLengthDelimiterSize; i++)
						nal_length = (nal_length << 8) | pBuf[i];

					if (cbSize < nal_length + cbNALLengthDelimiterSize)
					{
						printf("[H265] The actual NAL bytes(%" PRIu32 ") are less than the specified NAL length(%" PRIu32 ").\n", (uint32_t)(cbSize - cbNALLengthDelimiterSize), nal_length);
						break;
					}

					// check the nal_unit_type
					int8_t nal_unit_type = (pBuf[cbNALLengthDelimiterSize] >> 1) & 0x3F;
					nal_unit_types.push_back(nal_unit_type);
					if (ctx_video_bst.nal_unit_type_filters.size() > 0)
					{
						if (std::find(ctx_video_bst.nal_unit_type_filters.begin(), ctx_video_bst.nal_unit_type_filters.end(), nal_unit_type) ==
							ctx_video_bst.nal_unit_type_filters.end())
						{
							cbSize -= cbNALLengthDelimiterSize + 2;
							pBuf += (ptrdiff_t)cbNALLengthDelimiterSize + 2;
							continue;
						}
					}

					AMP_NEWT1(ptr_bst_nal_unit, BYTE_STREAM_NAL_UNIT, &ctx_video_bst, cbNALLengthDelimiterSize);
					AMBst_Seek(bst, (cbBufSize - cbSize) * 8);
					if (AMP_FAILED(iRet = ptr_bst_nal_unit->Map(bst)))
						printf("[H265] Failed to map byte stream NAL unit {retcode: %d}.\n", iRet); 

					bst_nal_units.push_back(ptr_bst_nal_unit);
					ptr_bst_nal_unit = NULL;

					if (iRet == RET_CODE_NO_MORE_DATA)
					{
						AMBst_Destroy(bst);
						iRet = RET_CODE_SUCCESS;
						break;
					}

					cbSize -= cbNALLengthDelimiterSize + nal_length;
					pBuf += (ptrdiff_t)cbNALLengthDelimiterSize + nal_length;
				}
				AMBst_Destroy(bst);

				return RET_CODE_SUCCESS;
			}

			int Map(AMBst in_bst)
			{
				int iRet = RET_CODE_ERROR;
				SYNTAX_BITSTREAM_MAP::Map(in_bst);

				BYTE_STREAM_NAL_UNIT* ptr_bst_nal_unit = NULL;

				MAP_BST_BEGIN(0);

				do {
					AMP_NEWT1(ptr_bst_nal_unit, BYTE_STREAM_NAL_UNIT, &ctx_video_bst);
					if (AMP_FAILED(iRet = ptr_bst_nal_unit->Map(in_bst)))
						printf("[H265] Failed to map byte stream NAL unit {retcode: %d}.\n", iRet);

					bst_nal_units.push_back(ptr_bst_nal_unit);
					ptr_bst_nal_unit = NULL;

					if (iRet == RET_CODE_NO_MORE_DATA)
					{
						iRet = RET_CODE_SUCCESS;
						break;
					}
				} while (AMP_SUCCEEDED(iRet));

				MAP_BST_END();
				return iRet;
			}

			int Unmap(AMBst out_bst)
			{
				return RET_CODE_ERROR_NOTIMPL;
			}

			DECLARE_FIELDPROP_BEGIN()
			NAV_WRITE_TAG_BEGIN_1("VideoBitstream", "H265 video bitstream", 1);
				for (i = 0; i<(int)bst_nal_units.size(); i++)
				{
					NAV_FIELD_PROP_REF(bst_nal_units[i]);
				}
			NAV_WRITE_TAG_END("VideoBitstream");
			DECLARE_FIELDPROP_END()
		};
	}	// namespace H265Video

}	// namespace BST


#ifdef _WIN32
#pragma pack(pop)
#pragma warning(pop)
#endif
#undef PACKED

#endif
