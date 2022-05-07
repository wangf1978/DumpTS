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
#ifndef _H264_VIDEO_H_
#define _H264_VIDEO_H_

#include "sei.h"
#include "AMArray.h"
#include <vector>
#include <map>
#include <math.h>
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

extern const char* avc_nal_unit_type_names[32];
extern const char* avc_nal_unit_type_short_names[32];
extern const char* avc_nal_unit_type_descs[32];
extern const std::tuple<int16_t, int16_t> sample_aspect_ratios[18];
extern const char* sample_aspect_ratio_descs[256];
extern const char* vui_video_format_names[7];
extern const char* vui_colour_primaries_names[256];
extern const char* vui_transfer_characteristics_names[256];
extern const char* vui_matrix_coeffs_descs[256];
extern const char* chroma_format_idc_names[4];
extern const char* h264_slice_type_names[10];
extern const char* h264_primary_pic_type_names[8];

extern const char* get_h264_profile_name(int profile);
extern const char* get_h264_level_name(int level);

struct AVC_LEVEL_LIMIT
{
	uint32_t	MaxMBPS;
	uint32_t	MaxFS;
	uint32_t	MaxDpbMbs;
	uint32_t	MaxBR;
	uint32_t	MaxCPB;
	uint16_t	MaxVmvR;
	uint8_t		MinCR;
	uint8_t		MaxMvsPer2Mb;
};

#define MEMORY_MANAGEMENT_CONTROL_OPERATION_DESC(v)	(\
	(v) == 0 ? "End memory_management_control_operation syntax element loop" : (\
	(v) == 1 ? "Mark a short-term reference picture as &quot;unused for reference&quot;" : (\
	(v) == 2 ? "Mark a long-term reference picture as &quot;unused for reference&quot;" : (\
	(v) == 3 ? "Mark a short-term reference picture as &quot;used for long-term reference&quot; and assign a long-term frame index to it" : (\
	(v) == 4 ? "Specify the maximum long-term frame index and mark all long-term reference pictures having long-term frame indices greater than the maximum value as &quot;unused for reference&quot;" : (\
	(v) == 5 ? "Mark all reference pictures as &quot;unused for reference&quot; and set the MaxLongTermFrameIdx variable to &quot;no long-term frame indices&quot;" : (\
	(v) == 6 ? "Mark the current picture as &quot;used for long-term reference&quot; and assign a long-term frame index to it" : "Unknown")))))))

#define PIC_STRUCT_MEANING(s)	(\
	(s) == 0? "progressive frame":(\
	(s) == 1? "top field":(\
	(s) == 2? "bottom field":(\
	(s) == 3? "top/bottom fields":(\
	(s) == 4? "bottom/top fields":(\
	(s) == 5? "top-bottom-top fields":(\
	(s) == 6? "bottom-field-bottom fields":(\
	(s) == 7? "frame doubling":(\
	(s) == 8? "frame tripling":"unknown")))))))))

extern const std::map<int, AVC_LEVEL_LIMIT> avc_level_limits;

namespace BST {

	namespace H264Video {

		enum NAL_UNIT_TYPE
		{
			CS_NON_IDR_PIC	= 1,	// Coded slice of a non-IDR picture
			CSD_PARTITION_A = 2,	// Coded slice data partition A
			CSD_PARTITION_B = 3,	// Coded slice data partition B
			CSD_PARTITION_C = 4,	// Coded slice data partition C
			CS_IDR_PIC		= 5,	// Coded slice of an IDR picture
			SEI_NUT			= 6,	// Supplemental enhancement information
			SPS_NUT			= 7,	// Sequence	parameter set
			PPS_NUT			= 8,	// Picture parameter set
			AUD_NUT			= 9,	// Access unit delimiter
			EOS_NUT			= 10,	// End of Sequence
			EOB_NUT			= 11,	// End of stream
			FD_NUT			= 12,	// Filler data
			SPS_EXT_NUT		= 13,	// Sequence parameter set extension
			PREFIX_NUT		= 14,	// Prefix NAL unit
			SUB_SPS_NUT		= 15,	// Subset sequence parameter set
			DPS_NUT			= 16,	// Depth parameter set
			SL_WO_PARTITION	= 19,	// Coded slice of an auxiliary coded picture without partitioning
			SL_EXT			= 20,	// Coded slice extension
			SL_EXT_DVIEW	= 21,	// Coded slice extension for depth view components
		};

		enum AVC_PROFILE
		{
			AVC_PROFILE_UNKNOWN = -1,
			AVC_PROFILE_BASELINE = 66,
			AVC_PROFILE_CONSTRAINED_BASELINE,
			AVC_PROFILE_MAIN = 77,
			AVC_PROFILE_EXTENDED = 88,
			AVC_PROFILE_HIGH = 100,
			AVC_PROFILE_PROGRESSIVE_HIGH,
			AVC_PROFILE_CONSTRAINED_HIGH,
			AVC_PROFILE_HIGH_10 = 110,
			AVC_PROFILE_HIGH_10_INTRA,
			AVC_PROFILE_PROGRESSIVE_HIGH_10,
			AVC_PROFILE_HIGH_422 = 122,
			AVC_PROFILE_HIGH_422_INTRA,
			AVC_PROFILE_HIGH_444_PREDICTIVE = 244,
			AVC_PROFILE_HIGH_444_INTRA,
			AVC_PROFILE_CAVLC_444_INTRA_PROFIILE = 44,
			AVC_PROFILE_MULTIVIEW_HIGH = 118,
			AVC_PROFILE_STEREO_HIGH = 128,
			AVC_PROFILE_SCALABLE_BASELINE = 83,
			AVC_PROFILE_SCALABLE_CONSTRAINED_BASELINE,
			AVC_PROFILE_SCALABLE_HIGH = 86,
			AVC_PROFILE_SCALABLE_CONSTRAINED_HIGH,
			AVC_PROFILE_SCALABLE_HIGH_INTRA = 89,
			AVC_PROFILE_MULTIVIEW_DEPTH_HIGH = 138,
		};

		enum AVC_LEVEL
		{
			AVC_LEVEL_UNKNOWN = -1,
			AVC_LEVEL_1 = 100,
			AVC_LEVEL_1b,
			AVC_LEVEL_1_1 = 110,
			AVC_LEVEL_1_2 = 120,
			AVC_LEVEL_1_3 = 130,
			AVC_LEVEL_2 = 200,
			AVC_LEVEL_2_1 = 210,
			AVC_LEVEL_2_2 = 220,
			AVC_LEVEL_3 = 300,
			AVC_LEVEL_3_1 = 310,
			AVC_LEVEL_3_2 = 320,
			AVC_LEVEL_4 = 400,
			AVC_LEVEL_4_1 = 410,
			AVC_LEVEL_4_2 = 420,
			AVC_LEVEL_5 = 500,
			AVC_LEVEL_5_1 = 510,
			AVC_LEVEL_5_2 = 520,
			AVC_LEVEL_6 = 600,
			AVC_LEVEL_6_1 = 610,
			AVC_LEVEL_6_2 = 620,
		};

		struct NAL_UNIT;

		class VideoBitstreamCtx: public CComUnknown, public INALAVCContext
		{
		public:
			DECLARE_IUNKNOWN

			HRESULT NonDelegatingQueryInterface(REFIID uuid, void** ppvObj)
			{
				if (ppvObj == NULL)
					return E_POINTER;

				if (uuid == IID_INALContext)
					return GetCOMInterface((INALContext*)this, ppvObj);
				else if (uuid == IID_INALAVCContext)
					return GetCOMInterface((INALAVCContext*)this, ppvObj);

				return CComUnknown::NonDelegatingQueryInterface(uuid, ppvObj);
			}

			NAL_CODING					GetNALCoding() { return NAL_CODING_AVC; }
			RET_CODE					SetNUFilters(std::initializer_list<uint8_t> NU_type_filters);
			RET_CODE					GetNUFilters(std::vector<uint8_t>& NU_type_filters);
			bool						IsNUFiltered(uint8_t nal_unit_type);
			RET_CODE					SetActiveNUType(int8_t nu_type) { m_active_nu_type = nu_type; return RET_CODE_SUCCESS; }
			int8_t						GetActiveNUType() { return m_active_nu_type; }
			void						Reset();
			H264_NU						GetAVCSPS(uint8_t sps_id);
			H264_NU						GetAVCPPS(uint8_t pps_id);
			RET_CODE					UpdateAVCSPS(H264_NU sps_nu);
			RET_CODE					UpdateAVCPPS(H264_NU pps_nu);
			H264_NU						CreateAVCNU();
			int8_t						GetActiveSPSID();
			RET_CODE					ActivateSPS(int8_t sps_id);
			RET_CODE					DetactivateSPS();

		public:
			std::vector<uint8_t>		nal_unit_type_filters;
			int8_t						in_scanning;
			int8_t						prev_seq_parameter_set_id;
			std::map<uint8_t, H264_NU>	sp_h264_spses;		// the smart pointers of SPS for the current h264 bitstream
			std::map<uint8_t, H264_NU>	sp_h264_ppses;		// the smart pointers of PPS for the current h264 bitstream
			H264_NU						sp_prev_nal_unit;
			int8_t						m_active_sps_id = -1;
			int8_t						m_active_nu_type = -1;

			VideoBitstreamCtx(): in_scanning(0), prev_seq_parameter_set_id(-1) {
			}

			~VideoBitstreamCtx();
		};

		struct NAL_UNIT : public SYNTAX_BITSTREAM_MAP
		{
			struct NAL_UNIT_HEADER : public SYNTAX_BITSTREAM_MAP
			{
				uint8_t		forbidden_zero_bit : 1;
				uint8_t		nal_ref_idc : 2;
				uint8_t		nal_unit_type : 5;

				NAL_UNIT_HEADER() : forbidden_zero_bit(0), nal_ref_idc(0), nal_unit_type(0) {
				}

				int Map(AMBst in_bst)
				{
					SYNTAX_BITSTREAM_MAP::Map(in_bst);
					try
					{
						MAP_BST_BEGIN(0);
						nal_read_f(in_bst, forbidden_zero_bit, 1, uint8_t);
						nal_read_u(in_bst, nal_ref_idc, 2, uint8_t);
						nal_read_u(in_bst, nal_unit_type, 5, uint8_t);
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
				BST_FIELD_PROP_NUMBER1(forbidden_zero_bit, 1, "shall be equal to 0.");
				BST_FIELD_PROP_2NUMBER1(nal_ref_idc, 2, "");
				BST_FIELD_PROP_2NUMBER1(nal_unit_type, 5, avc_nal_unit_type_descs[nal_unit_type]);
				DECLARE_FIELDPROP_END()

			}PACKED;

			struct NAL_UNIT_HEADER_SVC_EXTENSION : public SYNTAX_BITSTREAM_MAP
			{
				uint32_t			idr_flag : 1;
				uint32_t			priority_id : 6;
				uint32_t			no_inter_layer_pred_flag : 1;
				uint32_t			dependency_id : 3;
				uint32_t			quality_id : 4;
				uint32_t			temporal_id : 3;
				uint32_t			use_ref_base_pic_flag : 1;
				uint32_t			discardable_flag : 1;
				uint32_t			output_flag : 1;
				uint32_t			reserved_three_2bits : 2;
				uint32_t			rserved : 9;

				NAL_UNIT_HEADER_SVC_EXTENSION()
					: idr_flag(0)
					, priority_id(0)
					, no_inter_layer_pred_flag(0)
					, dependency_id(0)
					, quality_id(0)
					, temporal_id(0)
					, use_ref_base_pic_flag(0)
					, discardable_flag(0)
					, output_flag(0)
					, reserved_three_2bits(0)
					, rserved(0) {
				}

				int Map(AMBst in_bst)
				{
					int iRet = RET_CODE_SUCCESS;
					SYNTAX_BITSTREAM_MAP::Map(in_bst);

					try
					{
						MAP_BST_BEGIN(0);
						nal_read_u(in_bst, idr_flag, 1, uint32_t);
						nal_read_u(in_bst, priority_id, 6, uint32_t);
						nal_read_u(in_bst, no_inter_layer_pred_flag, 1, uint32_t);
						nal_read_u(in_bst, dependency_id, 3, uint32_t);
						nal_read_u(in_bst, quality_id, 4, uint32_t);
						nal_read_u(in_bst, temporal_id, 3, uint32_t);
						nal_read_u(in_bst, use_ref_base_pic_flag, 1, uint32_t);
						nal_read_u(in_bst, discardable_flag, 1, uint32_t);
						nal_read_u(in_bst, output_flag, 1, uint32_t);
						nal_read_u(in_bst, reserved_three_2bits, 2, uint32_t);
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
				BST_FIELD_PROP_BOOL(idr_flag, "the current coded picture is an IDR picture when the value of dependency_id for the NAL unit is equal to the maximum value of dependency_id in the coded picture",
					"the current coded picture is not an IDR picture when the value of dependency_id for the NAL unit is equal to the maximum value of dependency_id in the coded picture");
				BST_FIELD_PROP_2NUMBER1(priority_id, 6, "specifies a priority identifier for the NAL unit");
				BST_FIELD_PROP_BOOL(no_inter_layer_pred_flag, "inter-layer prediction is not used for decoding the coded slice", "inter-layer prediction may be used for decoding the coded slice as signaled in the macro-block layer");
				BST_FIELD_PROP_2NUMBER1(dependency_id, 3, "specifies a dependency identifier for the NAL unit.");
				BST_FIELD_PROP_2NUMBER1(quality_id, 4, "specifies a quality identifier for the NAL unit.");
				BST_FIELD_PROP_2NUMBER1(temporal_id, 3, "specifies a temporal identifier for the NAL unit.");
				BST_FIELD_PROP_BOOL(use_ref_base_pic_flag, "specifies that reference base pictures (when present) and decoded pictures (when reference base pictures are not present) are used as reference pictures for inter prediction",
					"specifies that reference base pictures are not used as reference pictures for inter prediction");
				BST_FIELD_PROP_BOOL(discardable_flag, "specifies that the current NAL unit is not used for decoding dependency representations", "specifies that the current NAL unit may be used for decoding dependency representations");
				BST_FIELD_PROP_BOOL(output_flag, "", "");
				BST_FIELD_PROP_2NUMBER1(reserved_three_2bits, 2, "shall be equal to 3");
				DECLARE_FIELDPROP_END()
			}PACKED;

			struct NAL_UNIT_HEADER_MVC_EXTENSION : public SYNTAX_BITSTREAM_MAP
			{
				uint32_t	non_idr_flag : 1;
				uint32_t	priority_id : 6;
				uint32_t	view_id : 10;
				uint32_t	temporal_id : 3;
				uint32_t	anchor_pic_flag : 1;
				uint32_t	inter_view_flag : 1;
				uint32_t	reserved_one_bit : 1;
				uint32_t	reserved : 9;

				NAL_UNIT_HEADER_MVC_EXTENSION()
					: non_idr_flag(0)
					, priority_id(0)
					, view_id(10)
					, temporal_id(0)
					, anchor_pic_flag(0)
					, inter_view_flag(0)
					, reserved_one_bit(0)
					, reserved(0){
					}

				int Map(AMBst in_bst)
				{
					int iRet = RET_CODE_SUCCESS;
					SYNTAX_BITSTREAM_MAP::Map(in_bst);

					try
					{
						MAP_BST_BEGIN(0);
						nal_read_u(in_bst, non_idr_flag, 1, uint32_t);
						nal_read_u(in_bst, priority_id, 6, uint32_t);
						nal_read_u(in_bst, view_id, 10, uint32_t);
						nal_read_u(in_bst, temporal_id, 3, uint32_t);
						nal_read_u(in_bst, anchor_pic_flag, 1, uint32_t);
						nal_read_u(in_bst, inter_view_flag, 1, uint32_t);
						nal_read_u(in_bst, reserved_one_bit, 1, uint32_t);
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
				BST_FIELD_PROP_BOOL(non_idr_flag, "", "specifies that the current access unit is an IDR access unit");
				BST_FIELD_PROP_2NUMBER1(priority_id, 6, "specifies a priority identifier for the NAL unit");
				BST_FIELD_PROP_2NUMBER1(view_id, 10, "specifies a view identifier for the NAL unit");
				BST_FIELD_PROP_2NUMBER1(temporal_id, 3, "specifies a temporal identifier for the NAL unit");
				BST_FIELD_PROP_BOOL(anchor_pic_flag, "specifies that the current access unit is an anchor access unit", "");
				BST_FIELD_PROP_BOOL(inter_view_flag, "the current view component may be used for inter-view prediction by other view components in the current access unit",
					"the current view component is not used for inter-view prediction by any other view component in the current access unit");
				BST_FIELD_PROP_NUMBER1(reserved_one_bit, 1, "shall be equal to 1");
				DECLARE_FIELDPROP_END()
			}PACKED;

			struct SCALING_LIST : public SYNTAX_BITSTREAM_MAP
			{
				int				m_sizeOfScalingList = 0;
				bool			m_useDefaultScalingMatrixFlag = false;
				std::vector<uint8_t>
								scalingList;
				std::vector<int8_t>
								delta_scale;

				SCALING_LIST(int sizeOfScalingList) : m_sizeOfScalingList(sizeOfScalingList) {
					if (sizeOfScalingList > 0) {
						scalingList.resize(sizeOfScalingList);
						delta_scale.resize(sizeOfScalingList);
					}
				}

				bool useDefaultScalingMatrixFlag() {
					return m_useDefaultScalingMatrixFlag;
				}

				int Map(AMBst in_bst)
				{
					int iRet = RET_CODE_SUCCESS;
					SYNTAX_BITSTREAM_MAP::Map(in_bst);

					uint8_t lastScale = 8, nextScale = 8;

					try
					{
						MAP_BST_BEGIN(0);
						for (int j = 0; j < m_sizeOfScalingList; j++) {
							if (nextScale != 0) {
								nal_read_se(in_bst, delta_scale[j], int8_t);
								nextScale = (256 + lastScale + delta_scale[j]) % 256;
								m_useDefaultScalingMatrixFlag = (j == 0 && nextScale == 0) ? true : false;
							}

							scalingList[j] = (nextScale == 0) ? lastScale : nextScale;
							lastScale = scalingList[j];
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
				uint8_t lastScale = 8, nextScale = 8;
				NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag0", "lastScale = 8", 8, "");
				NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag1", "nextScale = 8", 8, "");
				NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag2", "for(j=0; j&lt;sizeOfScalingList; j++)", "");
				for (int j = 0; j < m_sizeOfScalingList; j++) {
					NAV_WRITE_TAG_ARRAY_BEGIN0("ScalingList", j, "");
					if (nextScale != 0)
					{
						NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag20", "if(nextScale != 0)", "");
						BST_ARRAY_FIELD_PROP_SE(delta_scale, j, "");
						nextScale = (256 + lastScale + delta_scale[j]) % 256;
						NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag200", "nextScale = (lastScale(%d) + delta_scale(%d) + 256) %% 256", nextScale, "", lastScale, delta_scale[j]);
						NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag201", "useDefaultScalingMatrixFlag = (j == 0 &amp;&amp; nextScale == 0)", (j == 0 && nextScale == 0), "");
						NAV_WRITE_TAG_END("Tag20");
					}

					NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag3", "scalingList[%d] = (nextScale(%d)==0)?lastScale(%d):nextScale(%d)", scalingList[j], "", j, nextScale, lastScale, nextScale);
					lastScale = scalingList[j];
					NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag4", "lastScale = scalingList[%d]", scalingList[j], "", j);
					NAV_WRITE_TAG_END("ScalingList");
				}
				NAV_WRITE_TAG_END("Tag2");
				DECLARE_FIELDPROP_END()

			};

			struct HRD_PARAMETERS : public SYNTAX_BITSTREAM_MAP
			{
				uint8_t			cpb_cnt_minus1 = 0;
				uint8_t			bit_rate_scale : 4;
				uint8_t			cpb_size_scale : 4;

				std::vector<uint32_t>
								bit_rate_value_minus1;
				std::vector<uint32_t>
								cpb_size_value_minus1;
				CAMBitArray		cbr_flag;

				uint8_t			initial_cpb_removal_delay_length_minus1 = 0;
				uint8_t			cpb_removal_delay_length_minus1 = 0;
				uint8_t			dpb_output_delay_length_minus1 = 0;
				uint8_t			time_offset_length = 0;

				HRD_PARAMETERS() : bit_rate_scale(0), cpb_size_scale(0) {
				}

				int Map(AMBst in_bst)
				{
					int iRet = RET_CODE_SUCCESS;
					SYNTAX_BITSTREAM_MAP::Map(in_bst);

					try
					{
						MAP_BST_BEGIN(0);
						nal_read_ue(in_bst, cpb_cnt_minus1, uint8_t);
						nal_read_u(in_bst, bit_rate_scale, 4, uint8_t);
						nal_read_u(in_bst, cpb_size_scale, 4, uint8_t);

						bit_rate_value_minus1.resize((size_t)cpb_cnt_minus1 + 1);
						cpb_size_value_minus1.resize((size_t)cpb_cnt_minus1 + 1);

						for (int i = 0; i <= cpb_cnt_minus1; i++) {
							nal_read_ue(in_bst, bit_rate_value_minus1[i], uint32_t);
							nal_read_ue(in_bst, cpb_size_value_minus1[i], uint32_t);
							nal_read_bitarray(in_bst, cbr_flag, i);
						}

						nal_read_u(in_bst, initial_cpb_removal_delay_length_minus1, 5, uint8_t);
						nal_read_u(in_bst, cpb_removal_delay_length_minus1, 5, uint8_t);
						nal_read_u(in_bst, dpb_output_delay_length_minus1, 5, uint8_t);
						nal_read_u(in_bst, time_offset_length, 5, uint8_t);

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
				BST_FIELD_PROP_UE(cpb_cnt_minus1, "plus 1 specifies the number of alternative CPB specifications in the bitstream");
				BST_FIELD_PROP_2NUMBER1(bit_rate_scale, 4, "(together with bit_rate_value_minus1[ SchedSelIdx ]) specifies the maximum input bit rate of the SchedSelIdx-th CPB.");
				BST_FIELD_PROP_2NUMBER1(cpb_size_scale, 4, "(together with cpb_size_value_minus1[ SchedSelIdx ]) specifies the CPB size of the SchedSelIdx-th CPB.");

				NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "for(SchedSelIdx=0;SchedSelIdx&lt;=cpb_cnt_minus1;SchedSelIdx++)", "");
				for (i = 0; i <= cpb_cnt_minus1; i++) {
					BST_ARRAY_FIELD_PROP_UE(bit_rate_value_minus1, i, "(together with bit_rate_scale) specifies the maximum input bit rate for the SchedSelIdx-th CPB.");
					BST_ARRAY_FIELD_PROP_UE(cpb_size_value_minus1, i, "is used together with cpb_size_scale to specify the SchedSelIdx-th CPB size.");
					BST_ARRAY_FIELD_PROP_NUMBER("cbr_flag", i, 1, cbr_flag[i] ? 1 : 0, cbr_flag[i] ? "specifies that the HSS operates in a constant bit rate (CBR) mode." :
						"specifies that to decode this bitstream by the HRD using the SchedSelIdx-th CPB specification, the hypothetical stream delivery scheduler (HSS) operates in an intermittent bit rate mode.");
					
					NAV_WRITE_TAG_WITH_VALUEFMTSTR("BitRate", "%sbps", "The bit rate in bits per second",
						GetReadableNum(((uint64_t)bit_rate_value_minus1[i] + 1) << (6 + bit_rate_scale)).c_str());
					NAV_WRITE_TAG_WITH_VALUEFMTSTR("CpbSize", "%sbit", "The CPB size in bits",
						GetReadableNum(((uint64_t)cpb_size_value_minus1[i] + 1) << (4 + cpb_size_scale)).c_str());
				}
				NAV_WRITE_TAG_END("Tag0");

				BST_FIELD_PROP_2NUMBER1(initial_cpb_removal_delay_length_minus1, 5, "the length in bits of the initial_cpb_removal_delay[SchedSelIdx] and initial_cpb_removal_delay_offset[SchedSelIdx] syntax elements of the buffering period SEI message");
				BST_FIELD_PROP_2NUMBER1(cpb_removal_delay_length_minus1, 5, "the length in bits of the cpb_removal_delay syntax element");
				BST_FIELD_PROP_2NUMBER1(dpb_output_delay_length_minus1, 5, "the length in bits of the dpb_output_delay syntax element");
				BST_FIELD_PROP_2NUMBER1(time_offset_length, 5, "greater than 0 specifies the length in bits of the time_offset syntax element");
				DECLARE_FIELDPROP_END()
			};

			struct ACCESS_UNIT_DELIMITER_RBSP : public SYNTAX_BITSTREAM_MAP
			{
				uint8_t					primary_pic_type = 0;
				RBSP_TRAILING_BITS		rbsp_trailing_bits;

				int Map(AMBst in_bst)
				{
					int iRet = RET_CODE_SUCCESS;
					SYNTAX_BITSTREAM_MAP::Map(in_bst);

					try
					{
						MAP_BST_BEGIN(0);
						nal_read_u(in_bst, primary_pic_type, 3, uint8_t);
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
				BST_FIELD_PROP_2NUMBER1(primary_pic_type, 3, h264_primary_pic_type_names[primary_pic_type]);
				BST_FIELD_PROP_OBJECT(rbsp_trailing_bits);
				DECLARE_FIELDPROP_END()

			};

			struct SEQ_PARAMETER_SET_DATA : public SYNTAX_BITSTREAM_MAP
			{
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
					uint8_t		timing_info_present_flag : 1;
					uint8_t		fixed_frame_rate_flag : 1;
					uint8_t		nal_hrd_parameters_present_flag : 1;
					uint8_t		vcl_hrd_parameters_present_flag : 1;

					uint8_t		colour_primaries;
					uint8_t		transfer_characteristics;
					uint8_t		matrix_coeffs;

					uint8_t		chroma_sample_loc_type_top_field;
					uint8_t		chroma_sample_loc_type_bottom_field;

					uint32_t	num_units_in_tick;
					uint32_t	time_scale;

					HRD_PARAMETERS*
								nal_hrd_parameters;

					uint8_t		low_delay_hrd_flag : 1;
					uint8_t		pic_struct_present_flag : 1;
					uint8_t		bitstream_restriction_flag : 1;
					uint8_t		motion_vectors_over_pic_boundaries_flag : 1;
					uint8_t		reserved : 4;

					HRD_PARAMETERS*
								vcl_hrd_parameters;

					uint8_t		max_bytes_per_pic_denom;
					uint8_t		max_bits_per_mb_denom;
					uint8_t		log2_max_mv_length_horizontal;
					uint8_t		log2_max_mv_length_vertical;
					uint8_t		max_num_reorder_frames;
					uint8_t		max_dec_frame_buffering;

					SEQ_PARAMETER_SET_DATA*
						seq_parameter_set_data;

					VUI_PARAMETERS(SEQ_PARAMETER_SET_DATA* ptr_seq_parameter_set_data)
						: aspect_ratio_info_present_flag(0)
						, overscan_info_present_flag(0)
						, video_signal_type_present_flag(0)
						, colour_description_present_flag(0)
						, chroma_loc_info_present_flag(0)
						, timing_info_present_flag(0)
						, nal_hrd_parameters_present_flag(0)
						, vcl_hrd_parameters_present_flag(0)
						, nal_hrd_parameters(NULL)
						, vcl_hrd_parameters(NULL)
						, seq_parameter_set_data(ptr_seq_parameter_set_data) {}

					virtual ~VUI_PARAMETERS() {
						AMP_SAFEDEL2(vcl_hrd_parameters);
						AMP_SAFEDEL2(nal_hrd_parameters);
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

							nal_read_u(in_bst, timing_info_present_flag, 1, uint8_t);
							if (timing_info_present_flag)
							{
								nal_read_u(in_bst, num_units_in_tick, 32, uint32_t);
								nal_read_u(in_bst, time_scale, 32, uint32_t);

								nal_read_u(in_bst, fixed_frame_rate_flag, 1, uint8_t);
							}

							nal_read_u(in_bst, nal_hrd_parameters_present_flag, 1, uint8_t);
							if (nal_hrd_parameters_present_flag)
							{
								nal_read_ref(in_bst, nal_hrd_parameters, HRD_PARAMETERS);
							}

							nal_read_u(in_bst, vcl_hrd_parameters_present_flag, 1, uint8_t);
							if (vcl_hrd_parameters_present_flag)
							{
								nal_read_ref(in_bst, vcl_hrd_parameters, HRD_PARAMETERS);
							}

							if (nal_hrd_parameters_present_flag || vcl_hrd_parameters_present_flag)
							{
								nal_read_u(in_bst, low_delay_hrd_flag, 1, uint8_t);
							}
							else
								low_delay_hrd_flag = 1 - fixed_frame_rate_flag;

							nal_read_u(in_bst, pic_struct_present_flag, 1, uint8_t);
							nal_read_u(in_bst, bitstream_restriction_flag, 1, uint8_t);

							if (bitstream_restriction_flag) {
								nal_read_u(in_bst, motion_vectors_over_pic_boundaries_flag, 1, uint8_t);

								nal_read_ue(in_bst, max_bytes_per_pic_denom, uint8_t);
								nal_read_ue(in_bst, max_bits_per_mb_denom, uint8_t);
								nal_read_ue(in_bst, log2_max_mv_length_horizontal, uint8_t);
								nal_read_ue(in_bst, log2_max_mv_length_vertical, uint8_t);
								nal_read_ue(in_bst, max_num_reorder_frames, uint8_t);
								nal_read_ue(in_bst, max_dec_frame_buffering, uint8_t);
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
					BST_FIELD_PROP_NUMBER1(aspect_ratio_info_present_flag, 1, aspect_ratio_info_present_flag ? "aspect_ratio_idc is present" : "aspect_ratio_idc is not present")
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
						BST_FIELD_PROP_NUMBER1(colour_description_present_flag, 1, colour_description_present_flag ? "specifies that colour_primaries, transfer_characteristics and matrix_coeffs are present"
							: "specifies that colour_primaries, transfer_characteristics and matrix_coeffs are not present.");

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
						BST_FIELD_PROP_2NUMBER1(chroma_sample_loc_type_top_field, (long long)quick_log2(chroma_sample_loc_type_top_field + 1) * 2 + 1, "");
						BST_FIELD_PROP_2NUMBER1(chroma_sample_loc_type_bottom_field, (long long)quick_log2(chroma_sample_loc_type_bottom_field + 1) * 2 + 1, "");
					}

					BST_FIELD_PROP_NUMBER1(timing_info_present_flag, 1, timing_info_present_flag ? "specifies that vui_num_units_in_tick, vui_time_scale, vui_poc_proportional_to_timing_flag and vui_hrd_parameters_present_flag are present"
						: "specifies that vui_num_units_in_tick, vui_time_scale, vui_poc_proportional_to_timing_flag and vui_hrd_parameters_present_flag are not present");
					if (timing_info_present_flag)
					{
						BST_FIELD_PROP_2NUMBER1(num_units_in_tick, 32, "the number of time units of a clock operating at the frequency vui_time_scale Hz that corresponds to one increment (called a clock tick) of a clock tick counter.");
						BST_FIELD_PROP_2NUMBER1(time_scale, 32, "the number of time units that pass in one second.");
						BST_FIELD_PROP_BOOL(fixed_frame_rate_flag, "the temporal distance between the HRD output times of any two consecutive pictures in output order is constrained as follows",
							"no such constraints apply to the temporal distance between the HRD output times of any two consecutive pictures in output order");
					}

					BST_FIELD_PROP_BOOL(nal_hrd_parameters_present_flag, "NAL HRD parameters (pertaining to Type II bitstream conformance) are present", "NAL HRD parameters are not present");
					if (nal_hrd_parameters_present_flag) {
						BST_FIELD_PROP_REF4(nal_hrd_parameters, "nal_hrd_parameters", "NAL HRD parameters");
					}

					BST_FIELD_PROP_BOOL(vcl_hrd_parameters_present_flag, "VCL HRD parameters (clauses E.1.2 and E.2.2) immediately follow the flag", "VCL HRD parameters are not present");
					if (vcl_hrd_parameters_present_flag) {
							BST_FIELD_PROP_REF4(vcl_hrd_parameters, "vcl_hrd_parameters", "VCL HRD parameters");
					}

					BST_FIELD_PROP_BOOL(low_delay_hrd_flag, "", "");
					BST_FIELD_PROP_BOOL(pic_struct_present_flag, "picture timing SEI messages (clause D.2.2) are present that include the pic_struct syntax element", "the pic_struct syntax element is not present");
					BST_FIELD_PROP_BOOL(bitstream_restriction_flag, "the following coded video sequence bitstream restriction parameters are present", "the following coded video sequence bitstream restriction parameters are not present");

					if (bitstream_restriction_flag)
					{
						BST_FIELD_PROP_BOOL(motion_vectors_over_pic_boundaries_flag, "one or more samples outside picture boundaries may be used in inter prediction",
							"no sample outside the picture boundaries and no sample at a fractional sample position for which the sample value is derived using one or more samples outside the picture boundaries is used for inter prediction of any sample");

						BST_FIELD_PROP_UE(max_bytes_per_pic_denom, "a number of bytes not exceeded by the sum of the sizes of the VCL NAL units associated with any coded picture in the coded video sequence");
						BST_FIELD_PROP_UE(max_bits_per_mb_denom, "an upper bound for the number of coded bits of macroblock_layer( ) data for any macroblock in any picture of the coded video sequence");
						BST_FIELD_PROP_UE(log2_max_mv_length_horizontal, "the maximum absolute value of a decoded horizontal motion vector component, in 1/4 luma sample units");
						BST_FIELD_PROP_UE(log2_max_mv_length_vertical, "the maximum absolute value of a decoded vertical motion vector component, in 1/4 luma sample units");
						BST_FIELD_PROP_UE(max_num_reorder_frames, "an upper bound for the number of frames buffers, in the decoded picture buffer(DPB), that are required for storing frames, complementary field pairs, and non-paired fields before output");
						BST_FIELD_PROP_UE(max_dec_frame_buffering, "the required size of the HRD decoded picture buffer (DPB) in units of frame buffers");
					}
					DECLARE_FIELDPROP_END()

				}PACKED;

				uint8_t			profile_idc;
				uint8_t			constraint_set0_flag : 1;
				uint8_t			constraint_set1_flag : 1;
				uint8_t			constraint_set2_flag : 1;
				uint8_t			constraint_set3_flag : 1;
				uint8_t			constraint_set4_flag : 1;
				uint8_t			constraint_set5_flag : 1;
				uint8_t			reserved_zero_2bits : 2;

				uint8_t			level_idc;
				uint8_t			seq_parameter_set_id;

				uint16_t		chroma_format_idc : 2;
				uint16_t		separate_colour_plane_flag : 1;
				uint16_t		bit_depth_luma_minus8 : 3;
				uint16_t		bit_depth_chroma_minus8 : 3;
				uint16_t		qpprime_y_zero_transform_bypass_flag : 1;
				uint16_t		seq_scaling_matrix_present_flag : 1;
				uint16_t		log2_max_frame_num_minus4 : 4;
				uint16_t		gaps_in_frame_num_value_allowed_flag : 1;

				CAMBitArray		seq_scaling_list_present_flag;
				std::vector<SCALING_LIST*>
								scaling_list;

				uint8_t			pic_order_cnt_type : 2;
				uint8_t			log2_max_pic_order_cnt_lsb_minus4 : 4;
				uint8_t			delta_pic_order_always_zero_flag : 1;
				uint8_t			reserved_1 : 1;

				int32_t			offset_for_non_ref_pic;
				int32_t			offset_for_top_to_bottom_field;
				uint8_t			num_ref_frames_in_pic_order_cnt_cycle;
				std::vector<int32_t>
								offset_for_ref_frame;

				uint8_t			max_num_ref_frames;
				uint16_t		pic_width_in_mbs_minus1;
				uint16_t		pic_height_in_map_units_minus1;

				uint8_t			frame_mbs_only_flag : 1;
				uint8_t			mb_adaptive_frame_field_flag : 1;
				uint8_t			direct_8x8_inference_flag : 1;
				uint8_t			frame_cropping_flag : 1;
				uint8_t			vui_parameters_present_flag : 1;
				uint8_t			reserved_2 : 3;

				uint16_t		frame_crop_left_offset;
				uint16_t		frame_crop_right_offset;
				uint16_t		frame_crop_top_offset;
				uint16_t		frame_crop_bottom_offset;

				VUI_PARAMETERS*	vui_parameters;
				VideoBitstreamCtx*
								ptr_ctx_video_bst;

				//
				// Concluded values
				//
				uint8_t			SubWidthC;
				uint8_t			SubHeightC;
				uint8_t			BitDepthY;
				uint8_t			BitDepthC;
				uint16_t		QpBdOffsetY;
				uint16_t		QpBdOffsetC;
				uint8_t			MbWidthC;
				uint8_t			MbHeightC;
				uint16_t		RawMbBits;
				uint32_t		MaxFrameNum;
				uint32_t		MaxPicOrderCntLsb;
				uint16_t		PicWidthInMbs;
				uint32_t		PicWidthInSamplesL;
				uint32_t		PicWidthInSamplesC;
				uint16_t		PicHeightInMapUnits;
				uint32_t		PicSizeInMapUnits;
				uint16_t		FrameHeightInMbs;
				uint8_t			ChromaArrayType;
				uint8_t			CropUnitX;
				uint8_t			CropUnitY;
				uint32_t		frame_buffer_width;
				uint32_t		frame_buffer_height;
				uint32_t		display_width;
				uint32_t		display_height;

				SEQ_PARAMETER_SET_DATA()
					: separate_colour_plane_flag(0)
					, vui_parameters(NULL)
					, ptr_ctx_video_bst(NULL) {
				}

				virtual ~SEQ_PARAMETER_SET_DATA() {
					AMP_SAFEDEL(vui_parameters);
					for (size_t i = 0; i < scaling_list.size(); i++) {
						UNMAP_STRUCT_POINTER5(scaling_list[i]);
					}
				}

				void UpdateCtx(VideoBitstreamCtx* ctx)
				{
					ptr_ctx_video_bst = ctx;
				}

				AVC_PROFILE GetH264Profile();
				AVC_LEVEL	GetH264Level();
				void		UpdateConcludedValues();

				int Map(AMBst in_bst)
				{
					int iRet = RET_CODE_SUCCESS;
					SYNTAX_BITSTREAM_MAP::Map(in_bst);

					try
					{
						MAP_BST_BEGIN(0);
						nal_read_u(in_bst, profile_idc, 8, uint8_t);
						nal_read_u(in_bst, constraint_set0_flag, 1, uint8_t);
						nal_read_u(in_bst, constraint_set1_flag, 1, uint8_t);
						nal_read_u(in_bst, constraint_set2_flag, 1, uint8_t);
						nal_read_u(in_bst, constraint_set3_flag, 1, uint8_t);
						nal_read_u(in_bst, constraint_set4_flag, 1, uint8_t);
						nal_read_u(in_bst, constraint_set5_flag, 1, uint8_t);
						nal_read_u(in_bst, reserved_zero_2bits, 2, uint8_t);

						nal_read_u(in_bst, level_idc, 8, uint8_t);
						nal_read_ue(in_bst, seq_parameter_set_id, uint8_t);

						if (ptr_ctx_video_bst != NULL)
						{
							if (ptr_ctx_video_bst->in_scanning &&
								ptr_ctx_video_bst->prev_seq_parameter_set_id == seq_parameter_set_id)
							{
								return RET_CODE_NOTHING_TODO;
							}
							else
								ptr_ctx_video_bst->prev_seq_parameter_set_id = seq_parameter_set_id;
						}

						if (profile_idc == 100 || profile_idc == 110 ||
							profile_idc == 122 || profile_idc == 244 || profile_idc == 44 ||
							profile_idc == 83 || profile_idc == 86 || profile_idc == 118 ||
							profile_idc == 128 || profile_idc == 138) {
							nal_read_ue(in_bst, chroma_format_idc, uint16_t);

							if (chroma_format_idc == 3) {
								nal_read_u(in_bst, separate_colour_plane_flag, 1, uint16_t);
							}
							else
								separate_colour_plane_flag = 0;

							nal_read_ue(in_bst, bit_depth_luma_minus8, uint16_t);
							nal_read_ue(in_bst, bit_depth_chroma_minus8, uint16_t);

							nal_read_u(in_bst, qpprime_y_zero_transform_bypass_flag, 1, uint16_t);
							nal_read_u(in_bst, seq_scaling_matrix_present_flag, 1, uint16_t);

							if (seq_scaling_matrix_present_flag) {
								scaling_list.resize((chroma_format_idc != 3) ? 8 : 12);
								for (int i = 0; i < ((chroma_format_idc != 3) ? 8 : 12); i++) {
									nal_read_bitarray(in_bst, seq_scaling_list_present_flag, i);
									if (seq_scaling_list_present_flag[i]) {
										if (i < 6) {
											nal_read_ref(in_bst, scaling_list[i], SCALING_LIST, 16);
										}
										else {
											nal_read_ref(in_bst, scaling_list[i], SCALING_LIST, 64);
										}
									}
								}
							}
						}
						else
						{
							chroma_format_idc = profile_idc == 183 ? 0 : 1;
							qpprime_y_zero_transform_bypass_flag = 0;
							seq_scaling_matrix_present_flag = 0;
						}

						nal_read_ue(in_bst, log2_max_frame_num_minus4, uint16_t);
						nal_read_ue(in_bst, pic_order_cnt_type, uint8_t);

						if (pic_order_cnt_type == 0) {
							nal_read_ue(in_bst, log2_max_pic_order_cnt_lsb_minus4, uint8_t);
						}
						else if (pic_order_cnt_type == 1) {
							nal_read_u(in_bst, delta_pic_order_always_zero_flag, 1, uint8_t);
							nal_read_se(in_bst, offset_for_non_ref_pic, int32_t);
							nal_read_se(in_bst, offset_for_top_to_bottom_field, int32_t);
							nal_read_ue(in_bst, num_ref_frames_in_pic_order_cnt_cycle, uint8_t);

							if (num_ref_frames_in_pic_order_cnt_cycle > 0) {
								offset_for_ref_frame.resize(num_ref_frames_in_pic_order_cnt_cycle);
							}

							for (int i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++) {
								nal_read_se(in_bst, offset_for_ref_frame[i], int32_t);
							}
						}

						nal_read_ue(in_bst, max_num_ref_frames, uint8_t);
						nal_read_u(in_bst, gaps_in_frame_num_value_allowed_flag, 1, uint16_t);
						nal_read_ue(in_bst, pic_width_in_mbs_minus1, uint16_t);
						nal_read_ue(in_bst, pic_height_in_map_units_minus1, uint16_t);

						nal_read_u(in_bst, frame_mbs_only_flag, 1, uint8_t);
						if (!frame_mbs_only_flag) {
							nal_read_u(in_bst, mb_adaptive_frame_field_flag, 1, uint8_t);
						}
						else
							mb_adaptive_frame_field_flag = 0;

						nal_read_u(in_bst, direct_8x8_inference_flag, 1, uint8_t);
						nal_read_u(in_bst, frame_cropping_flag, 1, uint8_t);
						if (frame_cropping_flag) {
							nal_read_ue(in_bst, frame_crop_left_offset, uint16_t);
							nal_read_ue(in_bst, frame_crop_right_offset, uint16_t);
							nal_read_ue(in_bst, frame_crop_top_offset, uint16_t);
							nal_read_ue(in_bst, frame_crop_bottom_offset, uint16_t);
						}

						UpdateConcludedValues();

						nal_read_u(in_bst, vui_parameters_present_flag, 1, uint8_t);
						if (vui_parameters_present_flag) {
							nal_read_ref(in_bst, vui_parameters, VUI_PARAMETERS, this);
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
				const char* szProfileName = get_h264_profile_name(GetH264Profile());
				BST_FIELD_PROP_2NUMBER1(profile_idc, 8, szProfileName);
				BST_FIELD_PROP_NUMBER1(constraint_set0_flag, 1, "");
				BST_FIELD_PROP_NUMBER1(constraint_set1_flag, 1, "");
				BST_FIELD_PROP_NUMBER1(constraint_set2_flag, 1, "");
				BST_FIELD_PROP_NUMBER1(constraint_set3_flag, 1, "");
				BST_FIELD_PROP_NUMBER1(constraint_set4_flag, 1, "");
				BST_FIELD_PROP_NUMBER1(constraint_set5_flag, 1, "");
				BST_FIELD_PROP_2NUMBER1(reserved_zero_2bits, 2, "");

				BST_FIELD_PROP_2NUMBER1(level_idc, 8, get_h264_level_name(GetH264Level()));
				BST_FIELD_PROP_UE(seq_parameter_set_id, "identifies the sequence parameter set that is referred to by the picture parameter set");

				if (profile_idc == 100 || profile_idc == 110 ||
					profile_idc == 122 || profile_idc == 244 || profile_idc == 44 ||
					profile_idc == 83 || profile_idc == 86 || profile_idc == 118 ||
					profile_idc == 128 || profile_idc == 138) {
					BST_FIELD_PROP_UE(chroma_format_idc, chroma_format_idc_names[chroma_format_idc]);

					if (chroma_format_idc == 3) {
						BST_FIELD_PROP_BOOL(separate_colour_plane_flag, "the three colour components of the 4:4:4 chroma format are coded separately", "the colour components are not coded separately");
					}

					BST_FIELD_PROP_UE(bit_depth_luma_minus8, "");
					BST_FIELD_PROP_UE(bit_depth_chroma_minus8, "");

					BST_FIELD_PROP_BOOL(qpprime_y_zero_transform_bypass_flag, "when QP'Y is equal to 0, a transform bypass operation for the transform coefficient decoding process and picture construction process prior to deblocking filter process as specified in clause 8.5 shall be applied",
						"the transform coefficient decoding process and picture construction process prior to deblocking filter process shall not use the transform bypass operation");
					BST_FIELD_PROP_BOOL(seq_scaling_matrix_present_flag, "the flags seq_scaling_list_present_flag[ i ] for i = 0..7 or i = 0..11 are present",
						"the flags seq_scaling_list_present_flag[ i ] for i = 0..7 or i = 0..11 are not present");

					if (seq_scaling_matrix_present_flag) {
						NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "for(i=0;i&lt;((chroma_format_idc!=3)?8:12);i++)", "");
						for (int i = 0; i < ((chroma_format_idc != 3) ? 8 : 12); i++) {
							MBCSPRINTF_S(szTagName, TAGNAME_SIZE, "seq_scaling_list_present_flag[%d]", i);
							MBCSPRINTF_S(szTemp4, TEMP4_SIZE, seq_scaling_list_present_flag[i] ? "the syntax structure for scaling list#%d is present" : "the syntax structure for scaling list#%d is not present", i);
							NAV_FIELD_PROP_WITH_ALIAS_BEGIN("Tag00", szTagName, 1, seq_scaling_list_present_flag[i] ? "1" : "0", szTemp4, bit_offset ? *bit_offset : -1LL, "I")
								if (seq_scaling_list_present_flag[i]) {
									if (i < 6)
									{
										MBCSPRINTF_S(szTemp4, TEMP4_SIZE, "scaling_list(ScalingList4x4[%d], 16,UseDefaultScalingMatrix4x4Flag[%d])", i, i);
										NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag00", szTemp4, "");
										BST_FIELD_PROP_REF(scaling_list[i]);
										NAV_WRITE_TAG_END("Tag00");
									}
									else {
										MBCSPRINTF_S(szTemp4, TEMP4_SIZE, "scaling_list(ScalingList8x8[%d],64,UseDefaultScalingMatrix8x8Flag[i%d])", i - 6, i - 6);
										NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag01", szTemp4, "");
										BST_FIELD_PROP_REF(scaling_list[i]);
										NAV_WRITE_TAG_END("Tag01");
									}
								}
							NAV_FIELD_PROP_END("Tag00");
						}
						NAV_WRITE_TAG_END("Tag0");
					}
				}

				BST_FIELD_PROP_UE(log2_max_frame_num_minus4, "specifies the value of the variable MaxFrameNum that is used in frame_num related derivations as follows: MaxFrameNum = 2^(log2_max_frame_num_minus4 + 4)");
				BST_FIELD_PROP_UE(pic_order_cnt_type, "specifies the method to decode picture order count");

				int32_t ExpectedDeltaPerPicOrderCntCycle = 0;
				if (pic_order_cnt_type == 0) {
					BST_FIELD_PROP_UE(log2_max_pic_order_cnt_lsb_minus4, "the value of the variable MaxPicOrderCntLsb that is used in the decoding process for picture order count as specified in clause 8.2.1 as follows:MaxPicOrderCntLsb = 2^(log2_max_pic_order_cnt_lsb_minus4 + 4)");
				}
				else if (pic_order_cnt_type == 1) {
					BST_FIELD_PROP_BOOL(delta_pic_order_always_zero_flag, "delta_pic_order_cnt[0] and delta_pic_order_cnt[1] are not present in the slice headers of the sequence and shall be inferred to be equal to 0",
						"delta_pic_order_cnt[0] is present in the slice headers of the sequence and delta_pic_order_cnt[1] may be present in the slice headers of the sequence");
					BST_FIELD_PROP_SE(offset_for_non_ref_pic, "is used to calculate the picture order count of a non-reference picture as specified in clause 8.2.1");
					BST_FIELD_PROP_SE(offset_for_top_to_bottom_field, "is used to calculate the picture order count of a bottom field as specified in clause 8.2.1");
					BST_FIELD_PROP_UE(num_ref_frames_in_pic_order_cnt_cycle, "is used in the decoding process for picture order count as specified in clause 8.2.1");

					if (num_ref_frames_in_pic_order_cnt_cycle > 0) {
						NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag1", "for( i = 0; i &lt; num_ref_frames_in_pic_order_cnt_cycle; i++)", "");
						for (int i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++) {
							BST_ARRAY_FIELD_PROP_SE(offset_for_ref_frame, i, "");
							ExpectedDeltaPerPicOrderCntCycle += offset_for_ref_frame[i];
						}
						NAV_WRITE_TAG_END("Tag1");
					}
				}

				BST_FIELD_PROP_UE(max_num_ref_frames, "the maximum number of short-term and long-term reference frames, complementary reference field pairs, and non-paired reference fields that may be used by the decoding process for inter prediction of any picture in the coded video sequence");
				BST_FIELD_PROP_BOOL(gaps_in_frame_num_value_allowed_flag, "", "");

				BST_FIELD_PROP_UE(pic_width_in_mbs_minus1, "plus 1 specifies the width of each decoded picture in units of macroblocks");
				BST_FIELD_PROP_UE(pic_height_in_map_units_minus1, "plus 1 specifies the height in slice group map units of a decoded frame or field");
				BST_FIELD_PROP_BOOL(frame_mbs_only_flag, "every coded picture of the coded video sequence is a coded frame containing only frame macroblocks",
					"coded pictures of the coded video sequence may either be coded fields or coded frames");
				if (!frame_mbs_only_flag) {
					BST_FIELD_PROP_BOOL(mb_adaptive_frame_field_flag, "the possible use of switching between frame and field macroblocks within frames", "no switching between frame and field macroblocks within a picture");
				}

				BST_FIELD_PROP_BOOL(direct_8x8_inference_flag, "", "");
				BST_FIELD_PROP_BOOL(frame_cropping_flag, "the frame cropping offset parameters follow next in the sequence parameter set", "the frame cropping offset parameters are not present");
				if (frame_cropping_flag) {
					BST_FIELD_PROP_UE(frame_crop_left_offset, "");
					BST_FIELD_PROP_UE(frame_crop_right_offset, "");
					BST_FIELD_PROP_UE(frame_crop_top_offset, "");
					BST_FIELD_PROP_UE(frame_crop_bottom_offset, "");
				}

				NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "Concluded Values", "");
				NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag0", "SubWidthC", SubWidthC, "");
				NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag1", "SubHeightC", SubHeightC, "");
				NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag2", "BitDepthY=8+bit_depth_luma_minus8", BitDepthY, "");
				NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag3", "QpBdOffsetY=6*bit_depth_luma_minus8", QpBdOffsetY, "");
				NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag4", "BitDepthC=8+bit_depth_chroma_minus8", BitDepthC, "");
				NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag5", "QpBdOffsetC=6*bit_depth_chroma_minus8", QpBdOffsetC, "");
				NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag6", "MbWidthC", MbWidthC, "");
				NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag7", "MbHeightC", MbHeightC, "");
				NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag8", "RawMbBits=256*BitDepthY + 2*MbWidthC*MbHeightC*BitDepthC", RawMbBits, "");
				NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag9", "MaxFrameNum=2^(log2_max_frame_num_minus4 + 4)", MaxFrameNum, "");
				if (pic_order_cnt_type == 0) {
					NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("TagA", "MaxPicOrderCntLsb=2^(log2_max_pic_order_cnt_lsb_minus4 + 4)", MaxPicOrderCntLsb, "");
				}
				else if (pic_order_cnt_type == 1) {
					NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("TagB", "ExpectedDeltaPerPicOrderCntCycle", ExpectedDeltaPerPicOrderCntCycle, "");
				}
				NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("TagC", "PicWidthInMbs=pic_width_in_mbs_minus1 + 1", PicWidthInMbs, "the picture width in units of macroblocks");
				NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("TagD", "PicWidthInSamplesL=PicWidthInMbs * 16", PicWidthInSamplesL, "picture width for the luma component");
				NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("TagE", "PicWidthInSamplesC=PicWidthInMbs * MbWidthC", PicWidthInSamplesC, "picture width for the chroma components");
				NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("TagF", "PicHeightInMapUnits=pic_height_in_map_units_minus1 + 1", PicHeightInMapUnits, "");
				NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("TagG", "PicSizeInMapUnits=PicWidthInMbs * PicHeightInMapUnits", PicSizeInMapUnits, "");
				NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("TagH", "FrameHeightInMbs=(2-frame_mbs_only_flag)*PicHeightInMapUnits", FrameHeightInMbs, "");
				NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("TagI", "ChromaArrayType", ChromaArrayType, "");
				if (frame_cropping_flag) {
					NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("TagJ", "CropUnitX", CropUnitX, "");
					NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("TagK", "CropUnitY", CropUnitY, "");
				}

				NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("TagL", "BufferWidth", frame_buffer_width, "The width of frame buffer");
				NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("TagM", "BufferHeight", frame_buffer_height, "The height of frame buffer");
				NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("TagN", "DisplayWidth", display_width, "The display width");
				NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("TagO", "DisplayHeight", display_height, "The display height");
				NAV_WRITE_TAG_END("Tag0");

				BST_FIELD_PROP_BOOL(vui_parameters_present_flag, "the vui_parameters() syntax structure is present", "the vui_parameters() syntax structure is not present");
				if (vui_parameters_present_flag) {
					BST_FIELD_PROP_REF1_1(vui_parameters, "");
				}

				DECLARE_FIELDPROP_END()

				uint8_t GetChromaArrayType() {
					return separate_colour_plane_flag == 0 ? chroma_format_idc : 0;
				}

			};

			struct SEQ_PARAMETER_SET_RBSP : public SYNTAX_BITSTREAM_MAP
			{
				SEQ_PARAMETER_SET_DATA	seq_parameter_set_data;
				RBSP_TRAILING_BITS		rbsp_trailing_bits;
				VideoBitstreamCtx*		ptr_ctx_video_bst;

				SEQ_PARAMETER_SET_RBSP(VideoBitstreamCtx* ctx = NULL) : ptr_ctx_video_bst(ctx) {
					seq_parameter_set_data.UpdateCtx(ctx);
				}

				int Map(AMBst in_bst)
				{
					int iRet = RET_CODE_SUCCESS;
					SYNTAX_BITSTREAM_MAP::Map(in_bst);

					try
					{
						MAP_BST_BEGIN(0);
						nal_read_obj(in_bst, seq_parameter_set_data);
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
				BST_FIELD_PROP_OBJECT(seq_parameter_set_data);
				BST_FIELD_PROP_OBJECT(rbsp_trailing_bits);
				DECLARE_FIELDPROP_END()
			};

			struct SEQ_PARAMETER_SET_SVC_EXTENSION : public SYNTAX_BITSTREAM_MAP
			{
				uint16_t		inter_layer_deblocking_filter_control_present_flag : 1;
				uint16_t		extended_spatial_scalability_idc : 2;
				uint16_t		chroma_phase_x_plus1_flag : 1;
				uint16_t		chroma_phase_y_plus1 : 2;
				uint16_t		seq_ref_layer_chroma_phase_x_plus1_flag : 1;
				uint16_t		seq_ref_layer_chroma_phase_y_plus1 : 2;
				uint16_t		seq_tcoeff_level_prediction_flag : 1;
				uint16_t		adaptive_tcoeff_level_prediction_flag : 1;
				uint16_t		slice_header_restriction_flag : 1;
				uint16_t		reserved : 4;

				int16_t			seq_scaled_ref_layer_left_offset = 0;
				int16_t			seq_scaled_ref_layer_top_offset = 0;
				int16_t			seq_scaled_ref_layer_right_offset = 0;
				int16_t			seq_scaled_ref_layer_bottom_offset = 0;

				SEQ_PARAMETER_SET_DATA*
								ptr_seq_parameter_set_data;

				SEQ_PARAMETER_SET_SVC_EXTENSION(SEQ_PARAMETER_SET_DATA* seq_parameter_set_data)
					: inter_layer_deblocking_filter_control_present_flag(0)
					, extended_spatial_scalability_idc(0)
					, chroma_phase_x_plus1_flag(0)
					, chroma_phase_y_plus1(0)
					, seq_ref_layer_chroma_phase_x_plus1_flag(0)
					, seq_ref_layer_chroma_phase_y_plus1(0)
					, seq_tcoeff_level_prediction_flag(0)
					, adaptive_tcoeff_level_prediction_flag(0)
					, slice_header_restriction_flag(0)
					, reserved(0)					
					, ptr_seq_parameter_set_data(seq_parameter_set_data) {
				}

				int Map(AMBst in_bst)
				{
					int iRet = RET_CODE_SUCCESS;
					SYNTAX_BITSTREAM_MAP::Map(in_bst);

					try
					{
						MAP_BST_BEGIN(0);
						nal_read_u(in_bst, inter_layer_deblocking_filter_control_present_flag, 1, uint16_t);
						nal_read_u(in_bst, extended_spatial_scalability_idc, 2, uint16_t);
						uint8_t ChromaArrayType = ptr_seq_parameter_set_data->GetChromaArrayType();
						if (ChromaArrayType == 1 || ChromaArrayType == 2) {
							nal_read_u(in_bst, chroma_phase_x_plus1_flag, 1, uint16_t);
						}
						else
							chroma_phase_x_plus1_flag = 1;

						if (ChromaArrayType == 1) {
							nal_read_u(in_bst, chroma_phase_y_plus1, 2, uint16_t);
						}
						else
							chroma_phase_y_plus1 = 1;

						if (extended_spatial_scalability_idc == 1)
						{
							if (ChromaArrayType > 0) {
								nal_read_u(in_bst, seq_ref_layer_chroma_phase_x_plus1_flag, 1, uint16_t);
								nal_read_u(in_bst, seq_ref_layer_chroma_phase_y_plus1, 2, uint16_t);
							}
							else
							{
								seq_ref_layer_chroma_phase_x_plus1_flag = chroma_phase_x_plus1_flag;
								seq_ref_layer_chroma_phase_y_plus1 = chroma_phase_y_plus1;
							}

							nal_read_se(in_bst, seq_scaled_ref_layer_left_offset, int16_t);
							nal_read_se(in_bst, seq_scaled_ref_layer_top_offset, int16_t);
							nal_read_se(in_bst, seq_scaled_ref_layer_right_offset, int16_t);
							nal_read_se(in_bst, seq_scaled_ref_layer_bottom_offset, int16_t);
						}

						nal_read_u(in_bst, seq_tcoeff_level_prediction_flag, 1, uint16_t);
						if (seq_tcoeff_level_prediction_flag) {
							nal_read_u(in_bst, adaptive_tcoeff_level_prediction_flag, 1, uint16_t);
						}
						else
							adaptive_tcoeff_level_prediction_flag = 0;

						nal_read_u(in_bst, slice_header_restriction_flag, 1, uint16_t);
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
				BST_FIELD_PROP_BOOL(inter_layer_deblocking_filter_control_present_flag, "a set of syntax elements controlling the characteristics of the deblocking filter for inter-layer prediction is present in the slice header",
					"the set of syntax elements controlling the characteristics of the deblocking filter for inter-layer prediction is not present in the slice headers and their inferred values are in effect");
				BST_FIELD_PROP_2NUMBER1(extended_spatial_scalability_idc, 2, "the presence of syntax elements related to geometrical parameters for the resampling processes");
				uint8_t ChromaArrayType = ptr_seq_parameter_set_data->GetChromaArrayType();

				if (ChromaArrayType == 1 || ChromaArrayType == 2) {
					BST_FIELD_PROP_BOOL(chroma_phase_x_plus1_flag, "", "");
				}

				if (ChromaArrayType == 1) {
					BST_FIELD_PROP_2NUMBER1(chroma_phase_y_plus1, 2, "the vertical phase shift of the chroma components in units of half luma samples of a frame or layer frame");
				}

				if (extended_spatial_scalability_idc == 1)
				{
					if (ChromaArrayType > 0) {
						BST_FIELD_PROP_2NUMBER1(seq_ref_layer_chroma_phase_x_plus1_flag, 1, "the horizontal phase shift of the chroma components in units of half luma samples of a layer frame for the layer pictures that may be used for inter-layer prediction");
						BST_FIELD_PROP_2NUMBER1(seq_ref_layer_chroma_phase_y_plus1, 2, "the vertical phase shift of the chroma components in units of half luma samples of a layer frame for the layer pictures that may be used for inter-layer prediction");
					}

					BST_FIELD_PROP_SE(seq_scaled_ref_layer_left_offset, "the horizontal offset between the upper-left luma sample of a resampled layer picture used for inter-layer prediction and the upper-left luma sample of the current picture or current layer picture in units of two luma samples");
					BST_FIELD_PROP_SE(seq_scaled_ref_layer_top_offset, "the vertical offset between the upper-left luma sample of a resampled layer picture used for inter-layer prediction and the upper-left luma sample of the current picture or current layer picture.");
					BST_FIELD_PROP_SE(seq_scaled_ref_layer_right_offset, "the horizontal offset between the bottom-right luma sample of a resampled layer picture used for inter-layer prediction and the bottom-right luma sample of the current picture or current layer picture in units of two luma samples");
					BST_FIELD_PROP_SE(seq_scaled_ref_layer_bottom_offset, "the vertical offset between the bottom-right luma sample of a resampled layer picture used for inter-layer prediction and the bottom-right luma sample of the current picture or current layer picture");
				}

				BST_FIELD_PROP_BOOL(seq_tcoeff_level_prediction_flag, "adaptive_tcoeff_level_prediction_flag is present", "adaptive_tcoeff_level_prediction_flag is not present");
				if (seq_tcoeff_level_prediction_flag) {
					BST_FIELD_PROP_2NUMBER1(adaptive_tcoeff_level_prediction_flag, 1, "specifies the presence of tcoeff_level_prediction_flag in slice headers that refer to the subset sequence parameter set");
				}
				BST_FIELD_PROP_2NUMBER1(slice_header_restriction_flag, 1, "specifies the presence of syntax elements in slice headers that refer to the subset sequence parameter set");
				DECLARE_FIELDPROP_END()

			}PACKED;

			struct SVC_VUI_PARAMETERS_EXTENSION : public SYNTAX_BITSTREAM_MAP
			{
				struct VUI_EXT_ENTRY
				{
					uint16_t		vui_ext_dependency_id : 3;
					uint16_t		vui_ext_quality_id : 4;
					uint16_t		vui_ext_temporal_id : 3;
					uint16_t		vui_ext_timing_info_present_flag : 1;
					uint16_t		vui_ext_fixed_frame_rate_flag : 1;
					uint16_t		vui_ext_nal_hrd_parameters_present_flag : 1;
					uint16_t		vui_ext_vcl_hrd_parameters_present_flag : 1;
					uint16_t		vui_ext_low_delay_hrd_flag : 1;
					uint16_t		vui_ext_pic_struct_present_flag : 1;

					uint32_t		vui_ext_num_units_in_tick;
					uint32_t		vui_ext_time_scale;

					HRD_PARAMETERS*	nal_hrd_parameters;
					HRD_PARAMETERS*	vcl_hrd_parameters;

				}PACKED;

				uint16_t		vui_ext_num_entries_minus1 = 0;
				std::vector<VUI_EXT_ENTRY>
								vui_ext_entries;

				int Map(AMBst in_bst)
				{
					int iRet = RET_CODE_SUCCESS;
					SYNTAX_BITSTREAM_MAP::Map(in_bst);

					try
					{
						MAP_BST_BEGIN(0);
						nal_read_ue(in_bst, vui_ext_num_entries_minus1, uint16_t);
						vui_ext_entries.resize((size_t)vui_ext_num_entries_minus1 + 1);
						for (int i = 0; i <= vui_ext_num_entries_minus1; i++) {
							nal_read_u(in_bst, vui_ext_entries[i].vui_ext_dependency_id, 3, uint16_t);
							nal_read_u(in_bst, vui_ext_entries[i].vui_ext_quality_id, 4, uint16_t);
							nal_read_u(in_bst, vui_ext_entries[i].vui_ext_temporal_id, 3, uint16_t);
							nal_read_u(in_bst, vui_ext_entries[i].vui_ext_timing_info_present_flag, 1, uint16_t);

							if (vui_ext_entries[i].vui_ext_timing_info_present_flag) {
								nal_read_u(in_bst, vui_ext_entries[i].vui_ext_num_units_in_tick, 32, uint32_t);
								nal_read_u(in_bst, vui_ext_entries[i].vui_ext_time_scale, 32, uint32_t);
								nal_read_u(in_bst, vui_ext_entries[i].vui_ext_fixed_frame_rate_flag, 1, uint16_t);
							}

							nal_read_u(in_bst, vui_ext_entries[i].vui_ext_nal_hrd_parameters_present_flag, 1, uint16_t);
							if (vui_ext_entries[i].vui_ext_nal_hrd_parameters_present_flag) {
								nal_read_ref(in_bst, vui_ext_entries[i].nal_hrd_parameters, HRD_PARAMETERS);
							}

							nal_read_u(in_bst, vui_ext_entries[i].vui_ext_vcl_hrd_parameters_present_flag, 1, uint16_t);
							if (vui_ext_entries[i].vui_ext_vcl_hrd_parameters_present_flag) {
								nal_read_ref(in_bst, vui_ext_entries[i].vcl_hrd_parameters, HRD_PARAMETERS);
							}

							if (vui_ext_entries[i].vui_ext_nal_hrd_parameters_present_flag ||
								vui_ext_entries[i].vui_ext_vcl_hrd_parameters_present_flag) {
								nal_read_u(in_bst, vui_ext_entries[i].vui_ext_low_delay_hrd_flag, 1, uint16_t);
							}

							nal_read_u(in_bst, vui_ext_entries[i].vui_ext_pic_struct_present_flag, 1, uint16_t);
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
				BST_FIELD_PROP_UE(vui_ext_num_entries_minus1, "plus 1 specifies the number of information entries that are present");
				for (i = 0; i <= vui_ext_num_entries_minus1; i++) {
					BST_ARRAY_FIELD_PROP_NUMBER("vui_ext_dependency_id", i, 3, vui_ext_entries[i].vui_ext_dependency_id, "");
					BST_ARRAY_FIELD_PROP_NUMBER("vui_ext_quality_id", i, 4, vui_ext_entries[i].vui_ext_quality_id, "");
					BST_ARRAY_FIELD_PROP_NUMBER("vui_ext_temporal_id", i, 3, vui_ext_entries[i].vui_ext_temporal_id, "the maximum value of temporal_id for the i-th subset of coded video sequences");
					BST_ARRAY_FIELD_PROP_NUMBER("vui_ext_timing_info_present_flag", i, 1, vui_ext_entries[i].vui_ext_timing_info_present_flag,
						vui_ext_entries[i].vui_ext_timing_info_present_flag
						? "vui_ext_num_units_in_tick[i], vui_ext_time_scale[i], and vui_ext_fixed_frame_rate_flag[i] for the i-th subset of coded video sequences are present"
						: "vui_ext_num_units_in_tick[i], vui_ext_time_scale[i], and vui_ext_fixed_frame_rate_flag[i] for the i-th subset of coded video sequences are not present");

					if (vui_ext_entries[i].vui_ext_timing_info_present_flag) {
						BST_ARRAY_FIELD_PROP_NUMBER("vui_ext_num_units_in_tick", i, 32, vui_ext_entries[i].vui_ext_num_units_in_tick, "the number of time units of a clock operating at the frequency time_scale Hz");
						BST_ARRAY_FIELD_PROP_NUMBER("vui_ext_time_scale", i, 32, vui_ext_entries[i].vui_ext_time_scale, "the number of time units that pass in one second");
						BST_ARRAY_FIELD_PROP_NUMBER("vui_ext_fixed_frame_rate_flag", i, 1, vui_ext_entries[i].vui_ext_fixed_frame_rate_flag, "");
					}

					BST_ARRAY_FIELD_PROP_NUMBER("vui_ext_nal_hrd_parameters_present_flag", i, 1, vui_ext_entries[i].vui_ext_nal_hrd_parameters_present_flag, "");
					if (vui_ext_entries[i].vui_ext_nal_hrd_parameters_present_flag) {
						NAV_WRITE_TAG_ARRAY_BEGIN0("nal_hrd_parameters", i, "");
						BST_FIELD_PROP_REF(vui_ext_entries[i].nal_hrd_parameters);
						NAV_WRITE_TAG_END("nal_hrd_parameters");
					}

					BST_ARRAY_FIELD_PROP_NUMBER("vui_ext_vcl_hrd_parameters_present_flag", i, 1, vui_ext_entries[i].vui_ext_vcl_hrd_parameters_present_flag, "");
					if (vui_ext_entries[i].vui_ext_vcl_hrd_parameters_present_flag) {
						NAV_WRITE_TAG_ARRAY_BEGIN0("vcl_hrd_parameters", i, "");
						BST_FIELD_PROP_REF(vui_ext_entries[i].vcl_hrd_parameters);
						NAV_WRITE_TAG_END("vcl_hrd_parameters");
					}

					if (vui_ext_entries[i].vui_ext_nal_hrd_parameters_present_flag ||
						vui_ext_entries[i].vui_ext_vcl_hrd_parameters_present_flag) {
						BST_ARRAY_FIELD_PROP_NUMBER("vui_ext_low_delay_hrd_flag", i, 1, vui_ext_entries[i].vui_ext_low_delay_hrd_flag, "");
					}

					BST_ARRAY_FIELD_PROP_NUMBER("vui_ext_pic_struct_present_flag", i, 1, vui_ext_entries[i].vui_ext_pic_struct_present_flag, "");
				}
				DECLARE_FIELDPROP_END()
			};

			struct SEQ_PARAMETER_SET_MVC_EXTENSION : public SYNTAX_BITSTREAM_MAP
			{
				uint16_t		num_views_minus1 = 0;
				uint16_t*		view_id;

				uint8_t*		num_anchor_refs_l0;
				uint16_t**		anchor_ref_l0;
				uint8_t*		num_anchor_refs_l1;
				uint16_t**		anchor_ref_l1;

				uint8_t*		num_non_anchor_refs_l0;
				uint16_t**		non_anchor_ref_l0;
				uint8_t*		num_non_anchor_refs_l1;
				uint16_t**		non_anchor_ref_l1;

				uint8_t			num_level_values_signalled_minus1 = 0;
				uint8_t*		level_idc;
				uint16_t*		num_applicable_ops_minus1;
				uint8_t**		applicable_op_temporal_id;
				uint16_t**		applicable_op_num_target_views_minus1;
				uint16_t***		applicable_op_target_view_id;
				uint16_t**		applicable_op_num_views_minus1;

				SEQ_PARAMETER_SET_MVC_EXTENSION()
					: view_id(NULL)
					, num_anchor_refs_l0(NULL), anchor_ref_l0(NULL), num_anchor_refs_l1(NULL), anchor_ref_l1(NULL)
					, num_non_anchor_refs_l0(NULL), non_anchor_ref_l0(NULL), num_non_anchor_refs_l1(NULL), non_anchor_ref_l1(NULL)
					, level_idc(NULL), num_applicable_ops_minus1(NULL), applicable_op_temporal_id(NULL), applicable_op_num_target_views_minus1(NULL), applicable_op_target_view_id(NULL), applicable_op_num_views_minus1(NULL) {
				}

				virtual ~SEQ_PARAMETER_SET_MVC_EXTENSION() {
					AMP_SAFEDELA2(view_id);

					for (int i = 1; i <= num_views_minus1; i++) {
						AMP_SAFEDELA2(anchor_ref_l0[i]);
						AMP_SAFEDELA2(anchor_ref_l1[i]);
						AMP_SAFEDELA2(non_anchor_ref_l0[i]);
						AMP_SAFEDELA2(non_anchor_ref_l1[i]);
					}

					AMP_SAFEDELA2(num_anchor_refs_l0);
					AMP_SAFEDELA2(anchor_ref_l0);
					AMP_SAFEDELA2(num_anchor_refs_l1);
					AMP_SAFEDELA2(anchor_ref_l1);

					AMP_SAFEDELA2(num_non_anchor_refs_l0);
					AMP_SAFEDELA2(non_anchor_ref_l0);
					AMP_SAFEDELA2(num_non_anchor_refs_l1);
					AMP_SAFEDELA2(non_anchor_ref_l1);

					for (int i = 0; i <= num_level_values_signalled_minus1; i++) {
						for (int j = 0; j <= num_applicable_ops_minus1[i]; j++) {
							AMP_SAFEDELA2(applicable_op_target_view_id[i][j]);
						}

						AMP_SAFEDELA2(applicable_op_temporal_id[i]);
						AMP_SAFEDELA2(applicable_op_num_target_views_minus1[i]);
						AMP_SAFEDELA2(applicable_op_target_view_id[i]);
						AMP_SAFEDELA2(applicable_op_num_views_minus1[i]);
					}

					AMP_SAFEDELA2(level_idc);
					AMP_SAFEDELA2(num_applicable_ops_minus1);
					AMP_SAFEDELA2(applicable_op_temporal_id);
					AMP_SAFEDELA2(applicable_op_num_target_views_minus1);
					AMP_SAFEDELA2(applicable_op_target_view_id);
					AMP_SAFEDELA2(applicable_op_num_views_minus1);
				}

				int Map(AMBst in_bst)
				{
					int iRet = RET_CODE_SUCCESS;
					SYNTAX_BITSTREAM_MAP::Map(in_bst);

					try
					{
						MAP_BST_BEGIN(0);
						nal_read_ue(in_bst, num_views_minus1, uint16_t);
						AMP_NEW0(view_id, uint16_t, (size_t)num_views_minus1 + 1);
						if (view_id == nullptr)
							return RET_CODE_OUTOFMEMORY;

						for (int i = 0; i <= num_views_minus1; i++) {
							nal_read_ue(in_bst, view_id[i], uint16_t);
						}

						AMP_NEW0(num_anchor_refs_l0,		uint8_t,	(size_t)num_views_minus1 + 1);
						AMP_NEW0(anchor_ref_l0,				uint16_t*,	(size_t)num_views_minus1 + 1);
						AMP_NEW0(num_anchor_refs_l1,		uint8_t,	(size_t)num_views_minus1 + 1);
						AMP_NEW0(anchor_ref_l1,				uint16_t*,	(size_t)num_views_minus1 + 1);
						AMP_NEW0(num_non_anchor_refs_l0,	uint8_t,	(size_t)num_views_minus1 + 1);
						AMP_NEW0(non_anchor_ref_l0,			uint16_t*,	(size_t)num_views_minus1 + 1);
						AMP_NEW0(num_non_anchor_refs_l1,	uint8_t,	(size_t)num_views_minus1 + 1);
						AMP_NEW0(non_anchor_ref_l1,			uint16_t*,	(size_t)num_views_minus1 + 1);

						if (num_anchor_refs_l0 == nullptr ||
							anchor_ref_l0 == nullptr ||
							num_anchor_refs_l1 == nullptr ||
							anchor_ref_l1 == nullptr ||
							num_non_anchor_refs_l0 == nullptr ||
							non_anchor_ref_l0 == nullptr ||
							num_non_anchor_refs_l1 == nullptr ||
							non_anchor_ref_l1 == nullptr)
						{
							return RET_CODE_OUTOFMEMORY;
						}

						for (int i = 1; i <= num_views_minus1; i++)
						{
							nal_read_ue(in_bst, num_anchor_refs_l0[i], uint8_t);
							AMP_NEW0(anchor_ref_l0[i], uint16_t, num_anchor_refs_l0[i]);
							if (anchor_ref_l0[i] == nullptr)
								return RET_CODE_OUTOFMEMORY;

							for (int j = 0; j < num_anchor_refs_l0[i]; j++) {
								nal_read_ue(in_bst, anchor_ref_l0[i][j], uint16_t);
							}

							nal_read_ue(in_bst, num_anchor_refs_l1[i], uint8_t);
							AMP_NEW0(anchor_ref_l1[i], uint16_t, num_anchor_refs_l1[i]);
							if (anchor_ref_l1[i] == nullptr)
								return RET_CODE_OUTOFMEMORY;

							for (int j = 0; j < num_anchor_refs_l1[i]; j++) {
								nal_read_ue(in_bst, anchor_ref_l1[i][j], uint16_t);
							}
						}

						for (int i = 1; i <= num_views_minus1; i++)
						{
							nal_read_ue(in_bst, num_non_anchor_refs_l0[i], uint8_t);
							AMP_NEW0(non_anchor_ref_l0[i], uint16_t, num_non_anchor_refs_l0[i]);
							if (non_anchor_ref_l0[i] == nullptr)
								return RET_CODE_OUTOFMEMORY;

							for (int j = 0; j < num_non_anchor_refs_l0[i]; j++) {
								nal_read_ue(in_bst, non_anchor_ref_l0[i][j], uint16_t);
							}

							nal_read_ue(in_bst, num_non_anchor_refs_l1[i], uint8_t);
							AMP_NEW0(non_anchor_ref_l1[i], uint16_t, num_non_anchor_refs_l1[i]);
							if (non_anchor_ref_l1[i] == nullptr)
								return RET_CODE_OUTOFMEMORY;

							for (int j = 0; j < num_non_anchor_refs_l1[i]; j++) {
								nal_read_ue(in_bst, non_anchor_ref_l1[i][j], uint16_t);
							}
						}

						nal_read_ue(in_bst, num_level_values_signalled_minus1, uint8_t);

						AMP_NEW0(level_idc,								uint8_t,	((size_t)num_level_values_signalled_minus1 + 1));
						AMP_NEW0(num_applicable_ops_minus1,				uint16_t,	((size_t)num_level_values_signalled_minus1 + 1));
						AMP_NEW0(applicable_op_temporal_id,				uint8_t*,	((size_t)num_level_values_signalled_minus1 + 1));
						AMP_NEW0(applicable_op_num_target_views_minus1, uint16_t*,	((size_t)num_level_values_signalled_minus1 + 1));
						AMP_NEW0(applicable_op_target_view_id,			uint16_t**, ((size_t)num_level_values_signalled_minus1 + 1));
						AMP_NEW0(applicable_op_num_views_minus1,		uint16_t*,	((size_t)num_level_values_signalled_minus1 + 1));

						if (level_idc == nullptr ||
							num_applicable_ops_minus1 == nullptr ||
							applicable_op_temporal_id == nullptr ||
							applicable_op_num_target_views_minus1 == nullptr ||
							applicable_op_target_view_id == nullptr ||
							applicable_op_num_views_minus1 == nullptr)
							return RET_CODE_OUTOFMEMORY;

						for (int i = 0; i <= num_level_values_signalled_minus1; i++)
						{
							nal_read_u(in_bst, level_idc[i], 8, uint8_t);
							nal_read_ue(in_bst, num_applicable_ops_minus1[i], uint16_t);

							AMP_NEW0(applicable_op_temporal_id[i],				uint8_t,	((size_t)num_applicable_ops_minus1[i] + 1));
							AMP_NEW0(applicable_op_num_target_views_minus1[i],	uint16_t,	((size_t)num_applicable_ops_minus1[i] + 1));
							AMP_NEW0(applicable_op_target_view_id[i],			uint16_t*,	((size_t)num_applicable_ops_minus1[i] + 1));
							AMP_NEW0(applicable_op_num_views_minus1[i],			uint16_t,	((size_t)num_applicable_ops_minus1[i] + 1));

							if (applicable_op_temporal_id[i] == nullptr ||
								applicable_op_num_target_views_minus1[i] == nullptr ||
								applicable_op_target_view_id[i] == nullptr ||
								applicable_op_num_views_minus1[i] == nullptr)
								return RET_CODE_OUTOFMEMORY;

							for (int j = 0; j <= num_applicable_ops_minus1[i]; j++) {
								nal_read_u(in_bst, applicable_op_temporal_id[i][j], 3, uint8_t);
								nal_read_ue(in_bst, applicable_op_num_target_views_minus1[i][j], uint16_t);

								AMP_NEW0(applicable_op_target_view_id[i][j], uint16_t, ((size_t)applicable_op_num_target_views_minus1[i][j] + 1));
								if (applicable_op_target_view_id[i][j] == nullptr)
									return RET_CODE_OUTOFMEMORY;

								for (int k = 0; k <= applicable_op_num_target_views_minus1[i][j]; k++) {
									nal_read_ue(in_bst, applicable_op_target_view_id[i][j][k], uint16_t);
								}

								nal_read_ue(in_bst, applicable_op_num_views_minus1[i][j], uint16_t);
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
				BST_FIELD_PROP_UE(num_views_minus1, "plus 1 specifies the maximum number of coded views in the coded video sequence");
				NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "for(i=0;i&lt;=num_views_minus1;i++)", "");
				for (i = 1; i <= num_views_minus1; i++) {
					BST_ARRAY_FIELD_PROP_UE(view_id, i, "the view_id of the view with VOIdx equal to i");
				}
				NAV_WRITE_TAG_END("Tag0");

				NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "for(i=1;i&lt;=num_views_minus1;i++)", "");
				for (i = 1; i <= num_views_minus1; i++) {
					BST_ARRAY_FIELD_PROP_UE(num_anchor_refs_l0, i, "the number of view components for inter-view prediction in the initial reference picture list RefPicList0 in decoding anchor view components with VOIdx equal to i");
					NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag00", "for(j=0;j&lt;num_anchor_refs_l0[i];j++)", "");
					for (int j = 0; j < num_anchor_refs_l0[i]; j++) {
						BST_2ARRAY_FIELD_PROP_UE(anchor_ref_l0, i, j, "the view_id of the j-th view component for inter-view prediction in the initial reference picture list RefPicList0 in decoding an anchor view components with VOIdx equal to i");
					}
					NAV_WRITE_TAG_END("Tag00");

					BST_ARRAY_FIELD_PROP_UE(num_anchor_refs_l1, i, "the number of view components for inter-view prediction in the initial reference picture list RefPicList1 in decoding anchor view components with VOIdx equal to i");
					NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag01", "for(j=0;j&lt;num_anchor_refs_l1[i];j++)", "");
					for (int j = 0; j < num_anchor_refs_l1[i]; j++) {
						BST_2ARRAY_FIELD_PROP_UE(anchor_ref_l1, i, j, "the view_id of the j-th view component for inter-view prediction in the initial reference picture list RefPicList1 in decoding an anchor view component with VOIdx equal to i");
					}
					NAV_WRITE_TAG_END("Tag01");
				}
				NAV_WRITE_TAG_END("Tag0");

				NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag1", "for(i=1;i&lt;=num_views_minus1;i++)", "");
				for (i = 1; i <= num_views_minus1; i++) {
					BST_ARRAY_FIELD_PROP_UE(num_non_anchor_refs_l0, i, "the number of view components for inter-view prediction in the initial reference picture list RefPicList0 in decoding non-anchor view components with VOIdx equal to i");
					NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag10", "for(j=0;j&lt;num_non_anchor_refs_l0[i];j++)", "");
					for (int j = 0; j < num_non_anchor_refs_l0[i]; j++) {
						BST_2ARRAY_FIELD_PROP_UE(non_anchor_ref_l0, i, j, "the view_id of the j-th view component for inter-view prediction in the initial reference picture list RefPicList0 in decoding non-anchor view components with VOIdx equal to i");
					}
					NAV_WRITE_TAG_END("Tag10");

					BST_ARRAY_FIELD_PROP_UE(num_non_anchor_refs_l1, i, "the number of view components for inter-view prediction in the initial reference picture list RefPicList1 in decoding non-anchor view components with VOIdx equal to i");
					NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag11", "for(j=0;j&lt;num_non_anchor_refs_l1[i];j++)", "");
					for (int j = 0; j < num_non_anchor_refs_l1[i]; j++) {
						BST_2ARRAY_FIELD_PROP_UE(non_anchor_ref_l1, i, j, "the view_id of the j-th view component for inter-view prediction in the initial reference picture list RefPicList1 in decoding non-anchor view components with VOIdx equal to i");
					}
					NAV_WRITE_TAG_END("Tag11");
				}
				NAV_WRITE_TAG_END("Tag1");

				BST_FIELD_PROP_UE(num_level_values_signalled_minus1, "plus 1 specifies the number of level values signaled for the coded video sequence");
				NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag2", "for(i=0;i&lt;=num_level_values_signalled_minus1;i++)", "");
				for (i = 0; i <= num_level_values_signalled_minus1; i++) {
					BST_ARRAY_FIELD_PROP_NUMBER("level_idc", i, 8, level_idc[i], "the i-th level value signaled for the coded video sequence");
					BST_ARRAY_FIELD_PROP_UE(num_applicable_ops_minus1, i, "plus 1 specifies the number of operation points to which the level indicated by level_idc[i] applies");

					MBCSPRINTF_S(szTemp4, TEMP4_SIZE, "for(j=0;j&lt;=num_applicable_ops_minus1[%d];j++)", i);
					NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag20", szTemp4, "");
					for (int j = 0; j <= num_applicable_ops_minus1[i]; j++) {
						BST_2ARRAY_FIELD_PROP_NUMBER("applicable_op_temporal_id", i, j, 3, applicable_op_temporal_id[i][j], "the temporal_id of the j-th operation point to which the level indicated by level_idc[i] applies");
						BST_2ARRAY_FIELD_PROP_UE(applicable_op_num_target_views_minus1, i, j, "plus 1 specifies the number of target output views for the j-th operation point to which the level indicated by level_idc[i] applies");
						MBCSPRINTF_S(szTemp4, TEMP4_SIZE, "for(k=0;k&lt;=applicable_op_num_target_views_minus1[%d][%d];k++)", i, j);
						NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag200", szTemp4, "");
						for (int k = 0; k <= applicable_op_num_target_views_minus1[i][j]; k++) {
							BST_3ARRAY_FIELD_UE(applicable_op_target_view_id, i, j, k, "the k-th target output view for the j-th operation point to which the level indicated by level_idc[i] applies");
						}
						NAV_WRITE_TAG_END("Tag200");
						BST_2ARRAY_FIELD_PROP_UE(applicable_op_num_views_minus1, i, j, "plus 1 specifies the number of views required for decoding the target output views corresponding to the j-th operation point to which the level indicated by level_idc[i] applies");
					}
					NAV_WRITE_TAG_END("Tag20");
				}
				NAV_WRITE_TAG_END("Tag2");

				DECLARE_FIELDPROP_END()
			}PACKED;

			struct MVC_VUI_PARAMETERS_EXTENSION : public SYNTAX_BITSTREAM_MAP
			{
				struct VUI_MVC_OP : public SYNTAX_BITSTREAM_MAP
				{
					uint16_t	vui_mvc_temporal_id : 3;
					uint16_t	vui_mvc_num_target_output_views_minus1 : 12;
					uint16_t	vui_mvc_timing_info_present_flag : 1;

					std::vector<uint16_t>
								vui_mvc_view_id;

					uint32_t	vui_mvc_num_units_in_tick;
					uint32_t	vui_mvc_time_scale;

					uint8_t		vui_mvc_fixed_frame_rate_flag : 1;
					uint8_t		vui_mvc_nal_hrd_parameters_present_flag : 1;
					uint8_t		vui_mvc_vcl_hrd_parameters_present_flag : 1;
					uint8_t		vui_mvc_low_delay_hrd_flag : 1;
					uint8_t		vui_mvc_pic_struct_present_flag : 1;
					uint8_t		reserved : 3;

					HRD_PARAMETERS*
								nal_hrd_parameters;
					HRD_PARAMETERS*
								vcl_hrd_parameters;

					VUI_MVC_OP()
						: vui_mvc_low_delay_hrd_flag(1), nal_hrd_parameters(NULL), vcl_hrd_parameters(NULL) {
					}

					virtual ~VUI_MVC_OP() {
						UNMAP_STRUCT_POINTER5(nal_hrd_parameters);
						UNMAP_STRUCT_POINTER5(vcl_hrd_parameters);
					}

					int Map(AMBst in_bst)
					{
						int iRet = RET_CODE_SUCCESS;
						SYNTAX_BITSTREAM_MAP::Map(in_bst);

						try
						{
							MAP_BST_BEGIN(0);
							nal_read_u(in_bst, vui_mvc_temporal_id, 3, uint16_t);
							nal_read_ue(in_bst, vui_mvc_num_target_output_views_minus1, uint16_t);

							vui_mvc_view_id.resize((size_t)vui_mvc_num_target_output_views_minus1 + 1);
							for (int i = 0; i <= vui_mvc_num_target_output_views_minus1; i++) {
								nal_read_ue(in_bst, vui_mvc_view_id[i], uint16_t);
							}

							nal_read_u(in_bst, vui_mvc_timing_info_present_flag, 1, uint16_t);
							if (vui_mvc_timing_info_present_flag) {
								nal_read_u(in_bst, vui_mvc_num_units_in_tick, 32, uint32_t);
								nal_read_u(in_bst, vui_mvc_time_scale, 32, uint32_t);
								nal_read_u(in_bst, vui_mvc_fixed_frame_rate_flag, 1, uint8_t);
							}

							nal_read_u(in_bst, vui_mvc_nal_hrd_parameters_present_flag, 1, uint8_t);
							if (vui_mvc_nal_hrd_parameters_present_flag) {
								nal_read_ref(in_bst, nal_hrd_parameters, HRD_PARAMETERS);
							}

							nal_read_u(in_bst, vui_mvc_vcl_hrd_parameters_present_flag, 1, uint8_t);
							if (vui_mvc_vcl_hrd_parameters_present_flag) {
								nal_read_ref(in_bst, vcl_hrd_parameters, HRD_PARAMETERS);
							}

							if (vui_mvc_nal_hrd_parameters_present_flag ||
								vui_mvc_vcl_hrd_parameters_present_flag) {
								nal_read_u(in_bst, vui_mvc_low_delay_hrd_flag, 1, uint8_t);
							}

							nal_read_u(in_bst, vui_mvc_pic_struct_present_flag, 1, uint8_t);

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
					BST_FIELD_PROP_2NUMBER1(vui_mvc_temporal_id, 3, "the maximum value of temporal_id for all VCL NAL units in the representation of the i-th operation point");
					BST_FIELD_PROP_UE(vui_mvc_num_target_output_views_minus1, "plus one specifies the number of target output views for the i-th operation point");

					NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "for(j=0;j&lt;=vui_mvc_num_target_output_views_minus1[i];j++)", "");
					for (int j = 0; j <= vui_mvc_num_target_output_views_minus1; j++) {
						BST_ARRAY_FIELD_PROP_UE(vui_mvc_view_id, j, "indicates the j-th target output view in the i-th operation point");
					}
					NAV_WRITE_TAG_END("Tag0");

					BST_FIELD_PROP_BOOL(vui_mvc_timing_info_present_flag, "vui_mvc_num_units_in_tick[i], vui_mvc_time_scale[i], and vui_mvc_fixed_frame_rate_flag[i] for the i-th sub-bitstream are present",
						"vui_mvc_num_units_in_tick[i], vui_mvc_time_scale[i], and vui_mvc_fixed_frame_rate_flag[i] for the i-th sub-bitstream are not present");
					if (vui_mvc_timing_info_present_flag) {
						BST_FIELD_PROP_2NUMBER1(vui_mvc_num_units_in_tick, 32, "the number of time units of a clock operating at the frequency time_scale Hz that corresponds to one increment (called a clock tick) of a clock tick counter");
						BST_FIELD_PROP_2NUMBER1(vui_mvc_time_scale, 32, "the number of time units that pass in one second");
						BST_FIELD_PROP_BOOL(vui_mvc_fixed_frame_rate_flag, "", "");
					}

					BST_FIELD_PROP_BOOL(vui_mvc_nal_hrd_parameters_present_flag, "NAL HRD parameters are present", "NAL HRD parameters are not present");
					if (vui_mvc_nal_hrd_parameters_present_flag) {
						BST_FIELD_PROP_REF1_1(nal_hrd_parameters, "nal_hrd_parameters()");
					}

					BST_FIELD_PROP_BOOL(vui_mvc_vcl_hrd_parameters_present_flag, "VCL HRD parameters are present", "VCL HRD parameters are not present");
					if (vui_mvc_vcl_hrd_parameters_present_flag) {
						BST_FIELD_PROP_REF1_1(vcl_hrd_parameters, "vcl_hrd_parameters()");
					}

					if (vui_mvc_nal_hrd_parameters_present_flag || vui_mvc_vcl_hrd_parameters_present_flag) {
						BST_FIELD_PROP_BOOL(vui_mvc_low_delay_hrd_flag, "&quot;big pictures&quot; that violate the nominal CPB removal times due to the number of bits used by an access unit are permitted",
							"&quot;big pictures&quot; that violate the nominal CPB removal times due to the number of bits used by an access unit are not permitted");
					}

					BST_FIELD_PROP_BOOL(vui_mvc_pic_struct_present_flag, "picture timing SEI messages (clause D.2.2) are present that include the pic_struct syntax element", "the pic_struct syntax element is not present");

					DECLARE_FIELDPROP_END()

				};

				uint16_t		vui_mvc_num_ops_minus1 = 0;
				std::vector<VUI_MVC_OP>
								vui_mvc_ops;

				int Map(AMBst in_bst)
				{
					int iRet = RET_CODE_SUCCESS;
					SYNTAX_BITSTREAM_MAP::Map(in_bst);

					try
					{
						MAP_BST_BEGIN(0);
						nal_read_ue(in_bst, vui_mvc_num_ops_minus1, uint16_t);
						vui_mvc_ops.resize((size_t)vui_mvc_num_ops_minus1 + 1);
						for (int i = 0; i <= vui_mvc_num_ops_minus1; i++) {
							nal_read_obj(in_bst, vui_mvc_ops[i]);
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
				BST_FIELD_PROP_UE(vui_mvc_num_ops_minus1, "plus 1 specifies the number of operation points for which timing information, NAL HRD parameters, VCL HRD parameters, and the pic_struct_present_flag may be present");
				NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "for(i=0;i&lt;=vui_mvc_num_ops_minus1;i++)", "");
				for (i = 0; i <= vui_mvc_num_ops_minus1; i++) {
					BST_ARRAY_FIELD_PROP_OBJECT(vui_mvc_ops, i, "");
				}
				NAV_WRITE_TAG_END("Tag0");
				DECLARE_FIELDPROP_END()

			};

			struct MVCD_VUI_PARAMETERS_EXTENSION : public SYNTAX_BITSTREAM_MAP
			{
				struct VUI_MVCD_OP : public SYNTAX_BITSTREAM_MAP
				{
					uint16_t	vui_mvcd_temporal_id : 3;
					uint16_t	vui_mvcd_num_target_output_views_minus1 : 12;
					uint16_t	vui_mvcd_timing_info_present_flag : 1;

					std::vector<uint16_t>
								vui_mvcd_view_id;
					CAMBitArray	vui_mvcd_depth_flag;
					CAMBitArray	vui_mvcd_texture_flag;

					uint32_t	vui_mvcd_num_units_in_tick;
					uint32_t	vui_mvcd_time_scale;

					uint8_t		vui_mvcd_fixed_frame_rate_flag : 1;
					uint8_t		vui_mvcd_nal_hrd_parameters_present_flag : 1;
					uint8_t		vui_mvcd_vcl_hrd_parameters_present_flag : 1;
					uint8_t		vui_mvcd_low_delay_hrd_flag : 1;
					uint8_t		vui_mvcd_pic_struct_present_flag : 1;
					uint8_t		reserved : 3;

					HRD_PARAMETERS*
								nal_hrd_parameters;
					HRD_PARAMETERS*
								vcl_hrd_parameters;

					VUI_MVCD_OP()
						: vui_mvcd_low_delay_hrd_flag(1), nal_hrd_parameters(NULL), vcl_hrd_parameters(NULL) {
					}

					virtual ~VUI_MVCD_OP() {
						UNMAP_STRUCT_POINTER5(nal_hrd_parameters);
						UNMAP_STRUCT_POINTER5(vcl_hrd_parameters);
					}

					int Map(AMBst in_bst)
					{
						int iRet = RET_CODE_SUCCESS;
						SYNTAX_BITSTREAM_MAP::Map(in_bst);

						try
						{
							MAP_BST_BEGIN(0);
							nal_read_u(in_bst, vui_mvcd_temporal_id, 3, uint16_t);
							nal_read_ue(in_bst, vui_mvcd_num_target_output_views_minus1, uint16_t);

							vui_mvcd_view_id.resize((size_t)vui_mvcd_num_target_output_views_minus1 + 1);
							for (int i = 0; i <= vui_mvcd_num_target_output_views_minus1; i++) {
								nal_read_ue(in_bst, vui_mvcd_view_id[i], uint16_t);

								nal_read_bitarray(in_bst, vui_mvcd_depth_flag, i);
								nal_read_bitarray(in_bst, vui_mvcd_texture_flag, i);
							}

							nal_read_u(in_bst, vui_mvcd_timing_info_present_flag, 1, uint16_t);
							if (vui_mvcd_timing_info_present_flag) {
								nal_read_u(in_bst, vui_mvcd_num_units_in_tick, 32, uint32_t);
								nal_read_u(in_bst, vui_mvcd_time_scale, 32, uint32_t);
								nal_read_u(in_bst, vui_mvcd_fixed_frame_rate_flag, 1, uint8_t);
							}

							nal_read_u(in_bst, vui_mvcd_nal_hrd_parameters_present_flag, 1, uint8_t);
							if (vui_mvcd_nal_hrd_parameters_present_flag) {
								nal_read_ref(in_bst, nal_hrd_parameters, HRD_PARAMETERS);
							}

							nal_read_u(in_bst, vui_mvcd_vcl_hrd_parameters_present_flag, 1, uint8_t);
							if (vui_mvcd_vcl_hrd_parameters_present_flag) {
								nal_read_ref(in_bst, vcl_hrd_parameters, HRD_PARAMETERS);
							}

							if (vui_mvcd_nal_hrd_parameters_present_flag ||
								vui_mvcd_vcl_hrd_parameters_present_flag) {
								nal_read_u(in_bst, vui_mvcd_low_delay_hrd_flag, 1, uint8_t);
							}

							nal_read_u(in_bst, vui_mvcd_pic_struct_present_flag, 1, uint8_t);

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
					BST_FIELD_PROP_2NUMBER1(vui_mvcd_temporal_id, 3, "the maximum value of temporal_id for all VCL NAL units in the representation of the i-th operation point");
					BST_FIELD_PROP_UE(vui_mvcd_num_target_output_views_minus1, "plus one specifies the number of target output views for the i-th operation point");

					NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "for(j=0;j&lt;=vui_mvc_num_target_output_views_minus1[i];j++)", "");
					for (int j = 0; j <= vui_mvcd_num_target_output_views_minus1; j++) {
						BST_ARRAY_FIELD_PROP_UE(vui_mvcd_view_id, j, "indicates the j-th target output view in the i-th operation point");
						BST_ARRAY_FIELD_PROP_NUMBER("vui_mvcd_depth_flag", j, 1, vui_mvcd_depth_flag[j], "");
						BST_ARRAY_FIELD_PROP_NUMBER("vui_mvcd_texture_flag", j, 1, vui_mvcd_texture_flag[j], "");
					}
					NAV_WRITE_TAG_END("Tag0");

					BST_FIELD_PROP_BOOL(vui_mvcd_timing_info_present_flag, "vui_mvc_num_units_in_tick[i], vui_mvc_time_scale[i], and vui_mvc_fixed_frame_rate_flag[i] for the i-th sub-bitstream are present",
						"vui_mvc_num_units_in_tick[i], vui_mvc_time_scale[i], and vui_mvc_fixed_frame_rate_flag[i] for the i-th sub-bitstream are not present");
					if (vui_mvcd_timing_info_present_flag) {
						BST_FIELD_PROP_2NUMBER1(vui_mvcd_num_units_in_tick, 32, "the number of time units of a clock operating at the frequency time_scale Hz that corresponds to one increment (called a clock tick) of a clock tick counter");
						BST_FIELD_PROP_2NUMBER1(vui_mvcd_time_scale, 32, "the number of time units that pass in one second");
						BST_FIELD_PROP_BOOL(vui_mvcd_fixed_frame_rate_flag, "", "");
					}

					BST_FIELD_PROP_BOOL(vui_mvcd_nal_hrd_parameters_present_flag, "NAL HRD parameters are present", "NAL HRD parameters are not present");
					if (vui_mvcd_nal_hrd_parameters_present_flag) {
						BST_FIELD_PROP_REF1_1(nal_hrd_parameters, "nal_hrd_parameters()");
					}

					BST_FIELD_PROP_BOOL(vui_mvcd_vcl_hrd_parameters_present_flag, "VCL HRD parameters are present", "VCL HRD parameters are not present");
					if (vui_mvcd_vcl_hrd_parameters_present_flag) {
						BST_FIELD_PROP_REF1_1(vcl_hrd_parameters, "vcl_hrd_parameters()");
					}

					if (vui_mvcd_nal_hrd_parameters_present_flag || vui_mvcd_vcl_hrd_parameters_present_flag) {
						BST_FIELD_PROP_BOOL(vui_mvcd_low_delay_hrd_flag, "&quot;big pictures&quot; that violate the nominal CPB removal times due to the number of bits used by an access unit are permitted",
							"&quot;big pictures&quot; that violate the nominal CPB removal times due to the number of bits used by an access unit are not permitted");
					}

					BST_FIELD_PROP_BOOL(vui_mvcd_pic_struct_present_flag, "picture timing SEI messages (clause D.2.2) are present that include the pic_struct syntax element", "the pic_struct syntax element is not present");

					DECLARE_FIELDPROP_END()
				};

				uint16_t						vui_mvcd_num_ops_minus1;
				std::vector<VUI_MVCD_OP>		vui_mvcd_ops;

				int Map(AMBst in_bst)
				{
					int iRet = RET_CODE_SUCCESS;
					SYNTAX_BITSTREAM_MAP::Map(in_bst);

					try
					{
						MAP_BST_BEGIN(0);
						nal_read_ue(in_bst, vui_mvcd_num_ops_minus1, uint16_t);
						vui_mvcd_ops.resize((size_t)vui_mvcd_num_ops_minus1 + 1);
						for (int i = 0; i <= vui_mvcd_num_ops_minus1; i++) {
							nal_read_obj(in_bst, vui_mvcd_ops[i]);
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
				BST_FIELD_PROP_UE(vui_mvcd_num_ops_minus1, "plus 1 specifies the number of operation points for which timing information, NAL HRD parameters, VCL HRD parameters, and the pic_struct_present_flag may be present");
				NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "for(i=0;i&lt;=vui_mvc_num_ops_minus1;i++)", "");
				for (i = 0; i <= vui_mvcd_num_ops_minus1; i++) {
					BST_ARRAY_FIELD_PROP_OBJECT(vui_mvcd_ops, i, "");
				}
				NAV_WRITE_TAG_END("Tag0");
				DECLARE_FIELDPROP_END()

			};

			struct SEQ_PARAMETER_SET_MVCD_EXTENSION : public SYNTAX_BITSTREAM_MAP
			{
				struct APPLICABLE_OP_TARGET_VIEW {
					uint16_t	applicable_op_target_view_id : 14;
					uint16_t	applicable_op_depth_flag : 1;
					uint16_t	applicable_op_texture_flag : 1;
				}PACKED;

				uint16_t		num_views_minus1 = 0;
				uint16_t*		view_id;
				CAMBitArray		depth_view_present_flag;
				CAMBitArray		texture_view_present_flag;

				uint8_t*		num_anchor_refs_l0;
				uint16_t**		anchor_ref_l0;
				uint8_t*		num_anchor_refs_l1;
				uint16_t**		anchor_ref_l1;

				uint8_t*		num_non_anchor_refs_l0;
				uint16_t**		non_anchor_ref_l0;
				uint8_t*		num_non_anchor_refs_l1;
				uint16_t**		non_anchor_ref_l1;

				uint8_t			num_level_values_signalled_minus1 = 0;
				uint8_t*		level_idc;
				uint16_t*		num_applicable_ops_minus1;
				uint8_t**		applicable_op_temporal_id;
				uint16_t**		applicable_op_num_target_views_minus1;
				APPLICABLE_OP_TARGET_VIEW***
								applicable_op_target_view;
				uint16_t**		applicable_op_num_texture_views_minus1;
				uint16_t**		applicable_op_num_depth_views;

				uint8_t			mvcd_vui_parameters_present_flag : 1;
				uint8_t			texture_vui_parameters_present_flag : 1;
				uint8_t			reserved : 6;

				MVCD_VUI_PARAMETERS_EXTENSION*
								mvcd_vui_parameters_extension;
				MVC_VUI_PARAMETERS_EXTENSION*
								mvc_vui_parameters_extension;

				SEQ_PARAMETER_SET_MVCD_EXTENSION()
					: view_id(NULL)
					, num_anchor_refs_l0(NULL), anchor_ref_l0(NULL), num_anchor_refs_l1(NULL), anchor_ref_l1(NULL)
					, num_non_anchor_refs_l0(NULL), non_anchor_ref_l0(NULL), num_non_anchor_refs_l1(NULL), non_anchor_ref_l1(NULL)
					, level_idc(NULL), num_applicable_ops_minus1(NULL), applicable_op_temporal_id(NULL), applicable_op_num_target_views_minus1(NULL), applicable_op_target_view(NULL)
					, applicable_op_num_texture_views_minus1(NULL), applicable_op_num_depth_views(NULL)
					, mvcd_vui_parameters_present_flag(0), texture_vui_parameters_present_flag(0), reserved(0)
					, mvcd_vui_parameters_extension(NULL), mvc_vui_parameters_extension(NULL) {
				}

				virtual ~SEQ_PARAMETER_SET_MVCD_EXTENSION() {
					AMP_SAFEDELA2(view_id);

					for (int i = 1; i <= num_views_minus1; i++) {
						AMP_SAFEDELA2(anchor_ref_l0[i]);
						AMP_SAFEDELA2(anchor_ref_l1[i]);
						AMP_SAFEDELA2(non_anchor_ref_l0[i]);
						AMP_SAFEDELA2(non_anchor_ref_l1[i]);
					}

					AMP_SAFEDELA2(num_anchor_refs_l0);
					AMP_SAFEDELA2(anchor_ref_l0);
					AMP_SAFEDELA2(num_anchor_refs_l1);
					AMP_SAFEDELA2(anchor_ref_l1);

					AMP_SAFEDELA2(num_non_anchor_refs_l0);
					AMP_SAFEDELA2(non_anchor_ref_l0);
					AMP_SAFEDELA2(num_non_anchor_refs_l1);
					AMP_SAFEDELA2(non_anchor_ref_l1);

					for (int i = 0; i <= num_level_values_signalled_minus1; i++) {
						for (int j = 0; j <= num_applicable_ops_minus1[i]; j++) {
							AMP_SAFEDELA2(applicable_op_target_view[i][j]);
						}

						AMP_SAFEDELA2(applicable_op_temporal_id[i]);
						AMP_SAFEDELA2(applicable_op_num_target_views_minus1[i]);
						AMP_SAFEDELA2(applicable_op_target_view[i]);
						AMP_SAFEDELA2(applicable_op_num_texture_views_minus1[i]);
						AMP_SAFEDELA2(applicable_op_num_depth_views[i]);
					}

					AMP_SAFEDELA2(applicable_op_temporal_id);
					AMP_SAFEDELA2(applicable_op_num_target_views_minus1);
					AMP_SAFEDELA2(applicable_op_num_texture_views_minus1);
					AMP_SAFEDELA2(applicable_op_num_depth_views);
					AMP_SAFEDELA2(applicable_op_target_view);
					AMP_SAFEDELA2(level_idc);
					AMP_SAFEDELA2(num_applicable_ops_minus1);
				}

				int Map(AMBst in_bst)
				{
					int iRet = RET_CODE_SUCCESS;
					SYNTAX_BITSTREAM_MAP::Map(in_bst);

					try
					{
						MAP_BST_BEGIN(0);
						nal_read_ue(in_bst, num_views_minus1, uint16_t);
						AMP_NEW0(view_id, uint16_t, (size_t)num_views_minus1 + 1);
						if (view_id == nullptr)
							return RET_CODE_OUTOFMEMORY;

						for (int i = 0; i <= num_views_minus1; i++) {
							nal_read_ue(in_bst, view_id[i], uint16_t);
							nal_read_bitarray(in_bst, depth_view_present_flag, i);
							nal_read_bitarray(in_bst, texture_view_present_flag, i);
						}

						AMP_NEW0(num_anchor_refs_l0,	uint8_t,	(size_t)num_views_minus1 + 1);
						AMP_NEW0(anchor_ref_l0,			uint16_t*,	(size_t)num_views_minus1 + 1);
						AMP_NEW0(num_anchor_refs_l1,	uint8_t,	(size_t)num_views_minus1 + 1);
						AMP_NEW0(anchor_ref_l1,			uint16_t*,	(size_t)num_views_minus1 + 1);
						AMP_NEW0(num_non_anchor_refs_l0,uint8_t,	(size_t)num_views_minus1 + 1);
						AMP_NEW0(non_anchor_ref_l0,		uint16_t*,	(size_t)num_views_minus1 + 1);
						AMP_NEW0(num_non_anchor_refs_l1,uint8_t,	(size_t)num_views_minus1 + 1);
						AMP_NEW0(non_anchor_ref_l1,		uint16_t*,	(size_t)num_views_minus1 + 1);

						if (num_anchor_refs_l0 == nullptr ||
							anchor_ref_l0 == nullptr ||
							num_anchor_refs_l1 == nullptr ||
							anchor_ref_l1 == nullptr ||
							num_non_anchor_refs_l0 == nullptr ||
							non_anchor_ref_l0 == nullptr ||
							num_non_anchor_refs_l1 == nullptr ||
							non_anchor_ref_l1 == nullptr)
							return RET_CODE_OUTOFMEMORY;

						for (int i = 1; i <= num_views_minus1; i++)
						{
							if (!depth_view_present_flag[i])
								continue;
							nal_read_ue(in_bst, num_anchor_refs_l0[i], uint8_t);
							AMP_NEW0(anchor_ref_l0[i], uint16_t, num_anchor_refs_l0[i]);
							if (anchor_ref_l0[i] == nullptr)
								return RET_CODE_OUTOFMEMORY;

							for (int j = 0; j < num_anchor_refs_l0[i]; j++) {
								nal_read_ue(in_bst, anchor_ref_l0[i][j], uint16_t);
							}

							nal_read_ue(in_bst, num_anchor_refs_l1[i], uint8_t);
							AMP_NEW0(anchor_ref_l1[i], uint16_t, num_anchor_refs_l1[i]);
							if (anchor_ref_l1[i] == nullptr)
								return RET_CODE_OUTOFMEMORY;

							for (int j = 0; j < num_anchor_refs_l1[i]; j++) {
								nal_read_ue(in_bst, anchor_ref_l1[i][j], uint16_t);
							}
						}

						for (int i = 1; i <= num_views_minus1; i++)
						{
							if (!depth_view_present_flag[i])
								continue;
							nal_read_ue(in_bst, num_non_anchor_refs_l0[i], uint8_t);
							AMP_NEW0(non_anchor_ref_l0[i], uint16_t, num_non_anchor_refs_l0[i]);
							if (non_anchor_ref_l0[i] == nullptr)
								return RET_CODE_OUTOFMEMORY;

							for (int j = 0; j < num_non_anchor_refs_l0[i]; j++) {
								nal_read_ue(in_bst, non_anchor_ref_l0[i][j], uint16_t);
							}

							nal_read_ue(in_bst, num_non_anchor_refs_l1[i], uint8_t);
							AMP_NEW0(non_anchor_ref_l1[i], uint16_t, num_non_anchor_refs_l1[i]);
							if (non_anchor_ref_l1[i] == nullptr)
								return RET_CODE_OUTOFMEMORY;

							for (int j = 0; j < num_non_anchor_refs_l1[i]; j++) {
								nal_read_ue(in_bst, non_anchor_ref_l1[i][j], uint16_t);
							}
						}

						nal_read_ue(in_bst, num_level_values_signalled_minus1, uint8_t);

						AMP_NEW0(level_idc,					uint8_t,	((size_t)num_level_values_signalled_minus1 + 1));
						AMP_NEW0(num_applicable_ops_minus1, uint16_t,	((size_t)num_level_values_signalled_minus1 + 1));
						AMP_NEW0(applicable_op_temporal_id, uint8_t*,	((size_t)num_level_values_signalled_minus1 + 1));
						AMP_NEW0(applicable_op_num_target_views_minus1, 
															uint16_t*,	((size_t)num_level_values_signalled_minus1 + 1));
						AMP_NEW0(applicable_op_target_view, APPLICABLE_OP_TARGET_VIEW**, 
																		((size_t)num_level_values_signalled_minus1 + 1));
						AMP_NEW0(applicable_op_num_texture_views_minus1, 
															uint16_t*,	((size_t)num_level_values_signalled_minus1 + 1));
						AMP_NEW0(applicable_op_num_depth_views, 
															uint16_t*,	((size_t)num_level_values_signalled_minus1 + 1));

						if (level_idc == nullptr || num_applicable_ops_minus1 == nullptr || applicable_op_temporal_id == nullptr ||
							applicable_op_num_target_views_minus1 == nullptr || applicable_op_target_view == nullptr ||
							applicable_op_num_texture_views_minus1 == nullptr || applicable_op_num_depth_views == nullptr)
							return RET_CODE_OUTOFMEMORY;

						for (int i = 0; i <= num_level_values_signalled_minus1; i++)
						{
							nal_read_u(in_bst, level_idc[i], 8, uint8_t);
							nal_read_ue(in_bst, num_applicable_ops_minus1[i], uint16_t);

							AMP_NEW0(applicable_op_temporal_id[i],				uint8_t,	((size_t)num_applicable_ops_minus1[i] + 1));
							AMP_NEW0(applicable_op_num_target_views_minus1[i],	uint16_t,	((size_t)num_applicable_ops_minus1[i] + 1));
							AMP_NEW0(applicable_op_target_view[i],				APPLICABLE_OP_TARGET_VIEW*, 
																							((size_t)num_applicable_ops_minus1[i] + 1));
							AMP_NEW0(applicable_op_num_texture_views_minus1[i], uint16_t,	((size_t)num_applicable_ops_minus1[i] + 1));
							AMP_NEW0(applicable_op_num_depth_views[i],			uint16_t,	((size_t)num_applicable_ops_minus1[i] + 1));

							if (applicable_op_temporal_id[i] == nullptr ||
								applicable_op_num_target_views_minus1[i] == nullptr ||
								applicable_op_target_view[i] == nullptr ||
								applicable_op_num_texture_views_minus1[i] == nullptr ||
								applicable_op_num_depth_views[i] == nullptr)
								return RET_CODE_OUTOFMEMORY;

							for (int j = 0; j <= num_applicable_ops_minus1[i]; j++) {
								nal_read_u(in_bst, applicable_op_temporal_id[i][j], 3, uint8_t);
								nal_read_ue(in_bst, applicable_op_num_target_views_minus1[i][j], uint16_t);

								AMP_NEW0(applicable_op_target_view[i][j], APPLICABLE_OP_TARGET_VIEW, ((size_t)applicable_op_num_target_views_minus1[i][j] + 1));
								if (applicable_op_target_view[i][j] == nullptr)
									return RET_CODE_OUTOFMEMORY;

								for (int k = 0; k <= applicable_op_num_target_views_minus1[i][j]; k++) {
									nal_read_ue(in_bst, applicable_op_target_view[i][j][k].applicable_op_target_view_id, uint16_t);
									nal_read_u(in_bst, applicable_op_target_view[i][j][k].applicable_op_depth_flag, 1, uint16_t);
									nal_read_u(in_bst, applicable_op_target_view[i][j][k].applicable_op_texture_flag, 1, uint16_t);
								}

								nal_read_ue(in_bst, applicable_op_num_texture_views_minus1[i][j], uint16_t);
								nal_read_ue(in_bst, applicable_op_num_depth_views[i][j], uint16_t);
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
				BST_FIELD_PROP_UE(num_views_minus1, "plus 1 specifies the maximum number of coded views in the coded video sequence");
				NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "for(i=0;i&lt;=num_views_minus1;i++)", "");
				for (i = 1; i <= num_views_minus1; i++) {
					BST_ARRAY_FIELD_PROP_UE(view_id, i, "the view_id of the view with VOIdx equal to i");
				}
				NAV_WRITE_TAG_END("Tag0");

				NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "for(i=1;i&lt;=num_views_minus1;i++)", "");
				for (i = 1; i <= num_views_minus1; i++) {
					BST_ARRAY_FIELD_PROP_UE(num_anchor_refs_l0, i, "the number of view components for inter-view prediction in the initial reference picture list RefPicList0 in decoding anchor view components with VOIdx equal to i");
					NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag00", "for(j=0;j&lt;num_anchor_refs_l0[i];j++)", "");
					for (int j = 0; j < num_anchor_refs_l0[i]; j++) {
						BST_2ARRAY_FIELD_PROP_UE(anchor_ref_l0, i, j, "the view_id of the j-th view component for inter-view prediction in the initial reference picture list RefPicList0 in decoding an anchor view components with VOIdx equal to i");
					}
					NAV_WRITE_TAG_END("Tag00");

					BST_ARRAY_FIELD_PROP_UE(num_anchor_refs_l1, i, "the number of view components for inter-view prediction in the initial reference picture list RefPicList1 in decoding anchor view components with VOIdx equal to i");
					NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag01", "for(j=0;j&lt;num_anchor_refs_l1[i];j++)", "");
					for (int j = 0; j < num_anchor_refs_l1[i]; j++) {
						BST_2ARRAY_FIELD_PROP_UE(anchor_ref_l1, i, j, "the view_id of the j-th view component for inter-view prediction in the initial reference picture list RefPicList1 in decoding an anchor view component with VOIdx equal to i");
					}
					NAV_WRITE_TAG_END("Tag01");
				}
				NAV_WRITE_TAG_END("Tag0");

				NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag1", "for(i=1;i&lt;=num_views_minus1;i++)", "");
				for (i = 1; i <= num_views_minus1; i++) {
					BST_ARRAY_FIELD_PROP_UE(num_non_anchor_refs_l0, i, "the number of view components for inter-view prediction in the initial reference picture list RefPicList0 in decoding non-anchor view components with VOIdx equal to i");
					NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag10", "for(j=0;j&lt;num_non_anchor_refs_l0[i];j++)", "");
					for (int j = 0; j < num_non_anchor_refs_l0[i]; j++) {
						BST_2ARRAY_FIELD_PROP_UE(non_anchor_ref_l0, i, j, "the view_id of the j-th view component for inter-view prediction in the initial reference picture list RefPicList0 in decoding non-anchor view components with VOIdx equal to i");
					}
					NAV_WRITE_TAG_END("Tag10");

					BST_ARRAY_FIELD_PROP_UE(num_non_anchor_refs_l1, i, "the number of view components for inter-view prediction in the initial reference picture list RefPicList1 in decoding non-anchor view components with VOIdx equal to i");
					NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag11", "for(j=0;j&lt;num_non_anchor_refs_l1[i];j++)", "");
					for (int j = 0; j < num_non_anchor_refs_l1[i]; j++) {
						BST_2ARRAY_FIELD_PROP_UE(non_anchor_ref_l1, i, j, "the view_id of the j-th view component for inter-view prediction in the initial reference picture list RefPicList1 in decoding non-anchor view components with VOIdx equal to i");
					}
					NAV_WRITE_TAG_END("Tag11");
				}
				NAV_WRITE_TAG_END("Tag1");

				BST_FIELD_PROP_UE(num_level_values_signalled_minus1, "plus 1 specifies the number of level values signaled for the coded video sequence");
				NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag2", "for(i=0;i&lt;=num_level_values_signalled_minus1;i++)", "");
				for (i = 0; i <= num_level_values_signalled_minus1; i++) {
					BST_ARRAY_FIELD_PROP_NUMBER("level_idc", i, 8, level_idc[i], "the i-th level value signaled for the coded video sequence");
					BST_ARRAY_FIELD_PROP_UE(num_applicable_ops_minus1, i, "plus 1 specifies the number of operation points to which the level indicated by level_idc[i] applies");

					MBCSPRINTF_S(szTemp4, TEMP4_SIZE, "for(j=0;j&lt;=num_applicable_ops_minus1[%d];j++)", i);
					NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag20", szTemp4, "");
					for (int j = 0; j <= num_applicable_ops_minus1[i]; j++) {
						BST_2ARRAY_FIELD_PROP_NUMBER("applicable_op_temporal_id", i, j, 3, applicable_op_temporal_id[i][j], "the temporal_id of the j-th operation point to which the level indicated by level_idc[i] applies");
						BST_2ARRAY_FIELD_PROP_UE(applicable_op_num_target_views_minus1, i, j, "plus 1 specifies the number of target output views for the j-th operation point to which the level indicated by level_idc[i] applies");
						MBCSPRINTF_S(szTemp4, TEMP4_SIZE, "for(k=0;k&lt;=applicable_op_num_target_views_minus1[%d][%d];k++)", i, j);
						NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag200", szTemp4, "");
						for (int k = 0; k <= applicable_op_num_target_views_minus1[i][j]; k++) {
							BST_3ARRAY_FIELD_UE1(applicable_op_target_view, applicable_op_target_view_id, i, j, k, "the k-th target output view for the j-th operation point to which the level indicated by level_idc[i] applies");
							BST_3ARRAY_FIELD_BOOL1(applicable_op_target_view, applicable_op_depth_flag, i, j, k,
								"indicates that the depth view with view_id equal to applicable_op_target_view_id[i][j][k] is included in the j-th operation point",
								"indicates that the depth view with view_id equal to applicable_op_target_view_id[i][j][k] is not included in the j-th operation point");
							BST_3ARRAY_FIELD_BOOL1(applicable_op_target_view, applicable_op_texture_flag, i, j, k,
								"indicates that the texture view with view_id equal to applicable_op_target_view_id[i][j][k] is included in the j-th operation point",
								"indicates that the texture view with view_id equal to applicable_op_target_view_id[i][j][k] is not included in the j-th operation point");
						}
						NAV_WRITE_TAG_END("Tag200");
						BST_2ARRAY_FIELD_PROP_UE(applicable_op_num_texture_views_minus1, i, j, "plus 1 specifies the number of views required for decoding the target output views corresponding to the j-th operation point to which the level indicated by level_idc[i] applies");
						BST_2ARRAY_FIELD_PROP_UE(applicable_op_num_depth_views, i, j, "the number of depth views required for decoding the target output views corresponding to the j-th operation point to which the level indicated by level_idc[i] applies");
					}
					NAV_WRITE_TAG_END("Tag20");
				}
				NAV_WRITE_TAG_END("Tag2");

				DECLARE_FIELDPROP_END()
			};

			struct SUBSET_SEQ_PARAMETER_SET_RBSP : public SYNTAX_BITSTREAM_MAP
			{
				SEQ_PARAMETER_SET_DATA		seq_parameter_set_data;

				union {
					struct {
						SEQ_PARAMETER_SET_SVC_EXTENSION*
							seq_parameter_set_svc_extension;
						uint8_t				svc_vui_parameters_present_flag;
						SVC_VUI_PARAMETERS_EXTENSION*
							svc_vui_parameters_extension;
					};
					struct {
						uint8_t				bit_equal_to_one_0;
						SEQ_PARAMETER_SET_MVC_EXTENSION*
							seq_parameter_set_mvc_extension;
						uint8_t				mvc_vui_parameters_present_flag;
						MVC_VUI_PARAMETERS_EXTENSION*
							mvc_vui_parameters_extension;
					};
					struct {
						uint8_t				bit_equal_to_one_1;
						SEQ_PARAMETER_SET_MVCD_EXTENSION*
							seq_parameter_set_mvcd_extension;
					};
					void*	pointers[3];
				};

				uint8_t						additional_extension2_flag;
				RBSP_TRAILING_BITS			rbsp_trailing_bits;

				SUBSET_SEQ_PARAMETER_SET_RBSP() {
					memset(pointers, 0, sizeof(pointers));
				}

				virtual ~SUBSET_SEQ_PARAMETER_SET_RBSP() {
					if (seq_parameter_set_data.profile_idc == 83 || seq_parameter_set_data.profile_idc == 86) {
						UNMAP_STRUCT_POINTER5(seq_parameter_set_svc_extension);
						UNMAP_STRUCT_POINTER5(svc_vui_parameters_extension);
					}
					else if (seq_parameter_set_data.profile_idc == 118 || seq_parameter_set_data.profile_idc == 128)
					{
						UNMAP_STRUCT_POINTER5(seq_parameter_set_mvc_extension);
						UNMAP_STRUCT_POINTER5(mvc_vui_parameters_extension);
					}
					else if (seq_parameter_set_data.profile_idc == 138)
					{
						UNMAP_STRUCT_POINTER5(seq_parameter_set_mvcd_extension);
					}
				}

				int Map(AMBst in_bst)
				{
					int iRet = RET_CODE_SUCCESS;
					SYNTAX_BITSTREAM_MAP::Map(in_bst);

					try
					{
						MAP_BST_BEGIN(0);
						nal_read_obj(in_bst, seq_parameter_set_data);

						if (seq_parameter_set_data.profile_idc == 83 || seq_parameter_set_data.profile_idc == 86) {
							nal_read_ref(in_bst, seq_parameter_set_svc_extension, SEQ_PARAMETER_SET_SVC_EXTENSION, &seq_parameter_set_data);
							nal_read_u(in_bst, svc_vui_parameters_present_flag, 1, uint8_t);
							if (svc_vui_parameters_present_flag) {
								nal_read_ref(in_bst, svc_vui_parameters_extension, SVC_VUI_PARAMETERS_EXTENSION);
							}
						}
						else if (seq_parameter_set_data.profile_idc == 118 || seq_parameter_set_data.profile_idc == 128)
						{
							nal_read_u(in_bst, bit_equal_to_one_0, 1, uint8_t);
							nal_read_ref(in_bst, seq_parameter_set_mvc_extension, SEQ_PARAMETER_SET_MVC_EXTENSION);
							nal_read_u(in_bst, mvc_vui_parameters_present_flag, 1, uint8_t);
							if (mvc_vui_parameters_present_flag) {
								nal_read_ref(in_bst, mvc_vui_parameters_extension, MVC_VUI_PARAMETERS_EXTENSION);
							}
						}
						else if (seq_parameter_set_data.profile_idc == 138)
						{
							nal_read_u(in_bst, bit_equal_to_one_1, 1, uint8_t);
							nal_read_ref(in_bst, seq_parameter_set_mvcd_extension, SEQ_PARAMETER_SET_MVCD_EXTENSION);
						}

						nal_read_u(in_bst, additional_extension2_flag, 1, uint8_t);

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
				NAV_WRITE_TAG_BEGIN_WITH_ALIAS("seq_parameter_set_data", "seq_parameter_set_data()", "");
				BST_FIELD_PROP_OBJECT(seq_parameter_set_data);
				NAV_WRITE_TAG_END("seq_parameter_set_data");

				if (seq_parameter_set_data.profile_idc == 83 || seq_parameter_set_data.profile_idc == 86) {
					BST_FIELD_PROP_REF1_1(seq_parameter_set_svc_extension, "");
					BST_FIELD_PROP_BOOL(svc_vui_parameters_present_flag, "that the syntax structure svc_vui_parameters_extension() is present", "the syntax structure svc_vui_parameters_extension() is not present");
					if (svc_vui_parameters_present_flag) {
						BST_FIELD_PROP_REF1_1(svc_vui_parameters_extension, "");
					}
				}
				else if (seq_parameter_set_data.profile_idc == 118 || seq_parameter_set_data.profile_idc == 128)
				{
					BST_FIELD_PROP_2NUMBER("bit_equal_to_one", 1, bit_equal_to_one_0, "should be 1");
					BST_FIELD_PROP_REF1_1(seq_parameter_set_mvc_extension, "");
					BST_FIELD_PROP_BOOL(mvc_vui_parameters_present_flag, "the syntax structure mvc_vui_parameters_extension() is present", "the syntax structure mvc_vui_parameters_extension() is not present");
					if (mvc_vui_parameters_present_flag) {
						BST_FIELD_PROP_REF1_1(mvc_vui_parameters_extension, "");
					}
				}
				else if (seq_parameter_set_data.profile_idc == 138)
				{
					BST_FIELD_PROP_2NUMBER("bit_equal_to_one", 1, bit_equal_to_one_1, "should be 1");
					BST_FIELD_PROP_REF1_1(seq_parameter_set_mvcd_extension, "");
				}

				BST_FIELD_PROP_BOOL(additional_extension2_flag, "", "");
				BST_FIELD_PROP_OBJECT(rbsp_trailing_bits);

				DECLARE_FIELDPROP_END()
			};

			struct SEQ_PARAMETER_SET_EXTENSION_RBSP : public SYNTAX_BITSTREAM_MAP
			{
				uint8_t			seq_parameter_set_id : 5;
				uint8_t			aux_format_idc : 3;

				uint8_t			bit_depth_aux_minus8 : 3;
				uint8_t			alpha_incr_flag : 1;
				uint8_t			additional_extension_flag : 1;
				uint8_t			reserved : 3;

				uint16_t		alpha_opaque_value;
				uint16_t		alpha_transparent_value;

				RBSP_TRAILING_BITS
								rbsp_trailing_bits;

				int Map(AMBst in_bst)
				{
					int iRet = RET_CODE_SUCCESS;
					SYNTAX_BITSTREAM_MAP::Map(in_bst);

					try
					{
						MAP_BST_BEGIN(0);
						nal_read_ue(in_bst, seq_parameter_set_id, uint8_t);
						nal_read_ue(in_bst, aux_format_idc, uint8_t);

						if (aux_format_idc != 0) {
							nal_read_ue(in_bst, bit_depth_aux_minus8, uint8_t);
							nal_read_u(in_bst, alpha_incr_flag, 1, uint8_t);
							int v = bit_depth_aux_minus8 + 9;
							nal_read_u(in_bst, alpha_opaque_value, v, uint16_t);
							nal_read_u(in_bst, alpha_transparent_value, v, uint16_t);
						}

						nal_read_u(in_bst, additional_extension_flag, 1, uint8_t);

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
				BST_FIELD_PROP_UE(seq_parameter_set_id, "identifies the sequence parameter set associated with the sequence parameter set extension");
				BST_FIELD_PROP_UE(aux_format_idc, aux_format_idc == 0 ? "there are no auxiliary coded pictures in the coded video sequence." : (
					aux_format_idc == 1 ? "exactly one auxiliary coded picture is present in each access unit of the coded video sequence, and that for alpha blending purposes the decoded samples of the associated primary coded picture in each access unit should be multiplied by the interpretation sample values of the auxiliary coded picture in the access unit in the display process after output from the decoding process" : (
						aux_format_idc == 2 ? "exactly one auxiliary coded picture exists in each access unit of the coded video sequence, and that for alpha blending purposes the decoded samples of the associated primary coded picture in each access unit should not be multiplied by the interpretation sample values of the auxiliary coded picture in the access unit in the display process after output from the decoding process" : (
							aux_format_idc == 3 ? "exactly one auxiliary coded picture exists in each access unit of the coded video sequence, and that the usage of the auxiliary coded pictures is unspecified" : ""))));

				if (aux_format_idc != 0) {
					BST_FIELD_PROP_UE(bit_depth_aux_minus8, "the bit depth of the samples of the sample array of the auxiliary coded picture.");
					BST_FIELD_PROP_BOOL(alpha_incr_flag, "for purposes of alpha blending, after decoding the auxiliary coded picture samples, any auxiliary coded picture sample value that is greater than Min(alpha_opaque_value, alpha_transparent_value) should be increased by one to obtain the interpretation sample value for the auxiliary coded picture sample, and any auxiliary coded picture sample value that is less than or equal to Min(alpha_opaque_value, alpha_transparent_value) should be used without alteration as the interpretation sample value for the decoded auxiliary coded picture sample value",
						"the interpretation sample value for each decoded auxiliary coded picture sample value is equal to the decoded auxiliary coded picture sample value for purposes of alpha blending");
					int v = bit_depth_aux_minus8 + 9;
					BST_FIELD_PROP_2NUMBER1(alpha_opaque_value, v, "the interpretation sample value of an auxiliary coded picture sample for which the associated luma and chroma samples of the same access unit are considered opaque for purposes of alpha blending");
					BST_FIELD_PROP_2NUMBER1(alpha_transparent_value, v, "the interpretation sample value of an auxiliary coded picture sample for which the associated luma and chroma samples of the same access unit are considered transparent for purposes of alpha blending");
				}

				BST_FIELD_PROP_BOOL(additional_extension_flag, "additional_extension_flag is reserved", "no additional data follows");
				DECLARE_FIELDPROP_END()

			};

			struct PIC_PARAMETER_SET_RBSP : public SYNTAX_BITSTREAM_MAP
			{
				uint8_t			pic_parameter_set_id;

				uint8_t			seq_parameter_set_id : 5;
				uint8_t			entropy_coding_mode_flag : 1;
				uint8_t			bottom_field_pic_order_in_frame_present_flag : 1;
				uint8_t			reserved_0 : 1;

				uint8_t			num_slice_groups_minus1 : 4;
				uint8_t			slice_group_map_type : 4;

				std::vector<uint16_t>
								run_length_minus1;
				std::vector<uint16_t>
								top_left;
				std::vector<uint16_t>
								bottom_right;

				uint8_t			slice_group_change_direction_flag;
				uint16_t		slice_group_change_rate_minus1;

				uint16_t		pic_size_in_map_units_minus1;
				std::vector<uint8_t>
								slice_group_id;

				uint16_t		num_ref_idx_l0_default_active_minus1 : 5;
				uint16_t		num_ref_idx_l1_default_active_minus1 : 5;
				uint16_t		weighted_pred_flag : 1;
				uint16_t		weighted_bipred_idc : 2;
				uint16_t		deblocking_filter_control_present_flag : 1;
				uint16_t		constrained_intra_pred_flag : 1;
				uint16_t		redundant_pic_cnt_present_flag : 1;

				int8_t			pic_init_qp_minus26;
				int8_t			pic_init_qs_minus26;
				int8_t			chroma_qp_index_offset;

				BOOL			m_bMoreData;
				uint8_t			chroma_format_idc;

				uint8_t			transform_8x8_mode_flag : 1;
				uint8_t			pic_scaling_matrix_present_flag : 1;
				uint8_t			reserved_1 : 6;

				CAMBitArray		pic_scaling_list_present_flag;
				std::vector<SCALING_LIST*>
								scaling_list;

				int8_t			second_chroma_qp_index_offset;

				RBSP_TRAILING_BITS
								rbsp_trailing_bits;
				VideoBitstreamCtx*
								ptr_ctx_video_bst;

				PIC_PARAMETER_SET_RBSP(VideoBitstreamCtx* pCtxVideoBst)
					: m_bMoreData(FALSE)
					, chroma_format_idc(0)
					, ptr_ctx_video_bst(pCtxVideoBst)
				{
				}

				virtual ~PIC_PARAMETER_SET_RBSP()
				{
					for (size_t i = 0; i < scaling_list.size(); i++) {
						UNMAP_STRUCT_POINTER5(scaling_list[i]);
					}
				}

				int Map(AMBst in_bst);

				int Unmap(AMBst out_bst)
				{
					UNREFERENCED_PARAMETER(out_bst);
					return RET_CODE_ERROR_NOTIMPL;
				}

				DECLARE_FIELDPROP_BEGIN()
				BST_FIELD_PROP_UE(pic_parameter_set_id, "identifies the picture parameter set that is referred to in the slice header");
				BST_FIELD_PROP_UE(seq_parameter_set_id, "refers to the active sequence parameter set");
				BST_FIELD_PROP_BOOL(entropy_coding_mode_flag, "CABAC", "CAVLC");
				BST_FIELD_PROP_BOOL(bottom_field_pic_order_in_frame_present_flag, "the syntax elements delta_pic_order_cnt_bottom and delta_pic_order_cnt[1] are present in the slice headers.", "the syntax elements delta_pic_order_cnt_bottom and delta_pic_order_cnt[1] are not present in the slice headers.");
				BST_FIELD_PROP_UE(num_slice_groups_minus1, "plus 1 specifies the number of slice groups for a picture");
				if (num_slice_groups_minus1 > 0)
				{
					BST_FIELD_PROP_UE(slice_group_map_type, slice_group_map_type == 0 ? "interleaved slice groups" : (
						slice_group_map_type == 1 ? "a dispersed slice group mapping" : (
							slice_group_map_type == 2 ? "one or more &quot;foreground&quot; slice groups and a &quot;leftover&quot; slice group" : (
								slice_group_map_type == 3 || slice_group_map_type == 4 || slice_group_map_type == 5 ? "" : (
									slice_group_map_type == 6 ? "" : "Unknown")))));
					if (slice_group_map_type == 0)
					{
						for (uint8_t iGroup = 0; iGroup <= num_slice_groups_minus1; iGroup++)
						{
							BST_ARRAY_FIELD_PROP_UE(run_length_minus1, iGroup, "the number of consecutive slice group map units to be assigned to the i-th slice group in raster scan order of slice group map units");
						}
					}
					else if (slice_group_map_type == 2)
					{
						for (uint8_t iGroup = 0; iGroup < num_slice_groups_minus1; iGroup++)
						{
							BST_ARRAY_FIELD_PROP_UE(top_left, iGroup, "the top-left corners of a rectangle, slice group map unit positions in a raster scan of the picture for the slice group map units.");
							BST_ARRAY_FIELD_PROP_UE(bottom_right, iGroup, "the bottom-right corners of a rectangle, slice group map unit positions in a raster scan of the picture for the slice group map units.");
						}
					}
					else if (slice_group_map_type == 3 ||
						slice_group_map_type == 4 ||
						slice_group_map_type == 5)
					{
						BST_FIELD_PROP_BOOL(slice_group_change_direction_flag, "", "");
						BST_FIELD_PROP_UE(slice_group_change_rate_minus1, "the multiple in number of slice group map units by which the size of a slice group can change from one picture to the next");
					}
					else if (slice_group_map_type == 6)
					{
						BST_FIELD_PROP_UE(pic_size_in_map_units_minus1, "the number of slice group map units in the picture");
						NAV_WRITE_TAG_BEGIN2("slice_group_ids");
						for (uint16_t i = 0; i <= pic_size_in_map_units_minus1; i++)
						{
							BST_ARRAY_FIELD_PROP_UE(slice_group_id, i, "identifies a slice group of the i-th slice group map unit in raster scan order");
						}
						NAV_WRITE_TAG_END("slice_group_ids");
					}
				}

				BST_FIELD_PROP_UE(num_ref_idx_l0_default_active_minus1, "specifies how num_ref_idx_l0_active_minus1 is inferred for P, SP, and B slices with num_ref_idx_active_override_flag equal to 0");
				BST_FIELD_PROP_UE(num_ref_idx_l1_default_active_minus1, "specifies how num_ref_idx_l1_active_minus1 is inferred for B slices with num_ref_idx_active_override_flag equal to 0");
				BST_FIELD_PROP_BOOL(weighted_pred_flag, "explicit weighted prediction shall be applied to P and SP slices", "the default weighted prediction shall be applied to P and SP slices");
				BST_FIELD_PROP_2NUMBER1(weighted_bipred_idc, 2, weighted_bipred_idc == 0 ? "the default weighted prediction shall be applied to B slices" : (
					weighted_bipred_idc == 1 ? "explicit weighted prediction shall be applied to B slices" : (
						weighted_bipred_idc == 2 ? "implicit weighted prediction shall be applied to B slices" : "Unknown")));
				BST_FIELD_PROP_SE(pic_init_qp_minus26, "the initial value minus 26 of SliceQPY for each slice");
				BST_FIELD_PROP_SE(pic_init_qs_minus26, "the initial value minus 26 of SliceQSY for all macroblocks in SP or SI slices");
				BST_FIELD_PROP_SE(chroma_qp_index_offset, "the offset that shall be added to QPY and QSY for addressing the table of QPC values for the Cb chroma component");
				BST_FIELD_PROP_BOOL(deblocking_filter_control_present_flag, "a set of syntax elements controlling the characteristics of the deblocking filter is present in the slice header",
					"the set of syntax elements controlling the characteristics of the deblocking filter is not present in the slice headers and their inferred values are in effect");
				BST_FIELD_PROP_BOOL(constrained_intra_pred_flag, "specifies constrained intra prediction, in which case prediction of macroblocks coded using Intra macroblock prediction modes only uses residual data and decoded samples from I or SI macroblock types",
					"specifies that intra prediction allows usage of residual data and decoded samples of neighbouring macroblocks coded using Inter macroblock prediction modes for the prediction of macroblocks coded using Intra macroblock prediction modes");
				BST_FIELD_PROP_BOOL(redundant_pic_cnt_present_flag, "specifies that the redundant_pic_cnt syntax element is present in all slice headers, coded slice data partition B NAL units, and coded slice data partition C NAL units that refer (either directly or by association with a corresponding coded slice data partition A NAL unit) to the picture parameter set",
					"specifies that the redundant_pic_cnt syntax element is not present in slice headers, coded slice data partition B NAL units, and coded slice data partition C NAL units that refer (either directly or by association with a corresponding coded slice data partition A NAL unit) to the picture parameter set");

				if (m_bMoreData)
				{
					BST_FIELD_PROP_BOOL(transform_8x8_mode_flag, "specifies that the 8x8 transform decoding process may be in use", "specifies that the 8x8 transform decoding process is not in use");
					BST_FIELD_PROP_BOOL(pic_scaling_matrix_present_flag, "specifies that parameters are present to modify the scaling lists specified in the sequence parameter set",
						"specifies that the scaling lists used for the picture shall be inferred to be equal to those specified by the sequence parameter set");

					if (pic_scaling_matrix_present_flag)
					{
						NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "for(i=0;i&lt;6+((chroma_format_idc!=3)?2:6)*transform_8x8_mode_flag;i++)", "");
						for (uint8_t i = 0; i < 6 + ((chroma_format_idc != 3) ? 2 : 6)*transform_8x8_mode_flag; i++)
						{
							BST_ARRAY_FIELD_PROP_NUMBER1(pic_scaling_list_present_flag, i, 1, "");
							if (pic_scaling_list_present_flag[i])
							{
								BST_ARRAY_FIELD_PROP_REF(scaling_list, i, "Scaling List");
							}
						}
						NAV_WRITE_TAG_END("Tag0");
					}

					BST_FIELD_PROP_SE(second_chroma_qp_index_offset, "the offset that shall be added to QPY and QSY for addressing the table of QPC values for the Cr chroma component");
				}

				BST_FIELD_PROP_OBJECT(rbsp_trailing_bits);

				DECLARE_FIELDPROP_END()
			};

			struct SLICE_HEADER : public SYNTAX_BITSTREAM_MAP
			{
				struct REF_PIC_LIST_MODIFICATION : public SYNTAX_BITSTREAM_MAP
				{
					struct MODIFICATION_OF_PIC_NUM
					{
						int32_t				modification_of_pic_nums_idc;

						union
						{
							int32_t				abs_diff_pic_num_minus1;
							int32_t				long_term_pic_idx;
						};
					};

					int8_t				ref_pic_list_modification_flag[2];
					MODIFICATION_OF_PIC_NUM*
										modification_of_pic_num[2];
					SLICE_HEADER*		ptr_slice_header;

					REF_PIC_LIST_MODIFICATION(SLICE_HEADER* pSliceHdr) : ptr_slice_header(pSliceHdr) {
						memset(ref_pic_list_modification_flag, 0, sizeof(ref_pic_list_modification_flag));
						memset(modification_of_pic_num, 0, sizeof(modification_of_pic_num));
					}

					virtual ~REF_PIC_LIST_MODIFICATION()
					{
						for (int i = 0; i < 2; i++)
						{
							AMP_SAFEDEL2(modification_of_pic_num[i]);
						}
					}

					int Map(AMBst in_bst)
					{
						int iRet = RET_CODE_SUCCESS;
						SYNTAX_BITSTREAM_MAP::Map(in_bst);

						try
						{
							MAP_BST_BEGIN(0);
							if (ptr_slice_header->slice_type % 5 != 2 && ptr_slice_header->slice_type % 5 != 4) {
								nal_read_u(in_bst, ref_pic_list_modification_flag[0], 1, int8_t);
								if (ref_pic_list_modification_flag[0])
								{
									AMP_NEW(modification_of_pic_num[0], MODIFICATION_OF_PIC_NUM, (size_t)ptr_slice_header->num_ref_idx_l0_active_minus1 + 2);
									if (modification_of_pic_num[0] == NULL)
										return RET_CODE_OUTOFMEMORY;

									int val, i = 0;
									do
									{
										nal_read_ue(in_bst, val, int32_t);
										modification_of_pic_num[0][i].modification_of_pic_nums_idc = val;

										if (val == 0 || val == 1)
										{
											nal_read_ue(in_bst, modification_of_pic_num[0][i].abs_diff_pic_num_minus1, int32_t);
										}
										else if (val == 2)
										{
											nal_read_ue(in_bst, modification_of_pic_num[0][i].long_term_pic_idx, int32_t);
										}

										i++;
									} while (val != 3 && i <= ptr_slice_header->num_ref_idx_l0_active_minus1 + 1);
								}
							}

							if (ptr_slice_header->slice_type % 5 == 1)
							{
								nal_read_u(in_bst, ref_pic_list_modification_flag[1], 1, int8_t);
								if (ref_pic_list_modification_flag[1])
								{
									AMP_NEW(modification_of_pic_num[1], MODIFICATION_OF_PIC_NUM, (size_t)ptr_slice_header->num_ref_idx_l1_active_minus1 + 2);
									if (modification_of_pic_num[1] == NULL)
										return RET_CODE_OUTOFMEMORY;

									int val, i = 0;
									do
									{
										nal_read_ue(in_bst, val, int32_t);
										modification_of_pic_num[1][i].modification_of_pic_nums_idc = val;

										if (val == 0 || val == 1)
										{
											nal_read_ue(in_bst, modification_of_pic_num[1][i].abs_diff_pic_num_minus1, int32_t);
										}
										else if (val == 2)
										{
											nal_read_ue(in_bst, modification_of_pic_num[1][i].long_term_pic_idx, int32_t);
										}

										i++;
									} while (val != 3 && i <= ptr_slice_header->num_ref_idx_l1_active_minus1 + 1);
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
					NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "if(slice_type%5!=2 &amp;&amp; slice_type%5!=4)", "");
					NAV_FIELD_PROP_WITH_ALIAS_BEGIN("Tag00", "ref_pic_list_modification_flag_l0", 1, ref_pic_list_modification_flag[0] ? "1" : "0",
						ref_pic_list_modification_flag[0]
						? "modification_of_pic_nums_idc is present for specifying reference picture list 0"
						: "modification_of_pic_nums_idc is NOT present for specifying reference picture list 0", bit_offset ? *bit_offset : -1LL, "I");
					if (ref_pic_list_modification_flag[0])
					{
						for (uint16_t idx = 0; idx <= ptr_slice_header->num_ref_idx_l0_active_minus1; idx++)
						{
							BST_ARRAY_FIELD_PROP_UE2(modification_of_pic_nums_idc, idx,
								modification_of_pic_num[0][idx].modification_of_pic_nums_idc, "specifies which of the reference pictures or inter-view only reference components are re-mapped");
							if (modification_of_pic_num[0][idx].modification_of_pic_nums_idc == 0 ||
								modification_of_pic_num[0][idx].modification_of_pic_nums_idc == 1)
							{
								BST_ARRAY_FIELD_PROP_UE2(abs_diff_pic_num_minus1, idx, modification_of_pic_num[0][idx].abs_diff_pic_num_minus1, "plus 1 specifies the absolute difference between the picture number of the picture being moved to the current index in the list and the picture number prediction value");
							}
							else if (modification_of_pic_num[0][idx].modification_of_pic_nums_idc == 2)
							{
								BST_ARRAY_FIELD_PROP_UE2(long_term_pic_idx, idx, modification_of_pic_num[0][idx].long_term_pic_idx, "specifies the long-term picture number of the picture being moved to the current index in the list");
							}
						}
					}
					NAV_FIELD_PROP_END("Tag00");
					NAV_WRITE_TAG_END("Tag0");
					NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag1", "if(slice_type%5!==1)", "");
					NAV_FIELD_PROP_WITH_ALIAS_BEGIN("Tag10", "ref_pic_list_modification_flag_l1", 1, ref_pic_list_modification_flag[1] ? "1" : "0",
						ref_pic_list_modification_flag[1]
						? "modification_of_pic_nums_idc is present for specifying reference picture list 1"
						: "modification_of_pic_nums_idc is NOT present for specifying reference picture list 1", bit_offset ? *bit_offset : -1LL, "I");
					if (ref_pic_list_modification_flag[1])
					{
						for (uint16_t idx = 0; idx <= ptr_slice_header->num_ref_idx_l0_active_minus1; idx++)
						{
							BST_ARRAY_FIELD_PROP_UE2(modification_of_pic_nums_idc, idx,
								modification_of_pic_num[1][idx].modification_of_pic_nums_idc, "specifies which of the reference pictures or inter-view only reference components are re-mapped");
							if (modification_of_pic_num[1][idx].modification_of_pic_nums_idc == 0 ||
								modification_of_pic_num[1][idx].modification_of_pic_nums_idc == 1)
							{
								BST_ARRAY_FIELD_PROP_UE2(abs_diff_pic_num_minus1, idx, modification_of_pic_num[1][idx].abs_diff_pic_num_minus1, "plus 1 specifies the absolute difference between the picture number of the picture being moved to the current index in the list and the picture number prediction value");
							}
							else if (modification_of_pic_num[1][idx].modification_of_pic_nums_idc == 2)
							{
								BST_ARRAY_FIELD_PROP_UE2(long_term_pic_idx, idx, modification_of_pic_num[1][idx].long_term_pic_idx, "specifies the long-term picture number of the picture being moved to the current index in the list");
							}
						}
					}
					NAV_FIELD_PROP_END("Tag10");
					NAV_WRITE_TAG_END("Tag1");
					DECLARE_FIELDPROP_END()

				}PACKED;

				struct REF_PIC_LIST_MVC_MODIFICATION : public SYNTAX_BITSTREAM_MAP
				{
					struct MODIFICATION_OF_PIC_NUM
					{
						int32_t				modification_of_pic_nums_idc;

						union
						{
							int32_t				abs_diff_pic_num_minus1;
							int32_t				long_term_pic_idx;
							int32_t				abs_diff_view_idx_minus1;
						};
					};

					int8_t				ref_pic_list_modification_flag[2];
					MODIFICATION_OF_PIC_NUM*
						modification_of_pic_num[2];
					SLICE_HEADER*		ptr_slice_header;

					REF_PIC_LIST_MVC_MODIFICATION(SLICE_HEADER* pSliceHdr) : ptr_slice_header(pSliceHdr) {
						memset(ref_pic_list_modification_flag, 0, sizeof(ref_pic_list_modification_flag));
						memset(modification_of_pic_num, 0, sizeof(modification_of_pic_num));
					}

					virtual ~REF_PIC_LIST_MVC_MODIFICATION()
					{
						for (int i = 0; i < 2; i++)
						{
							AMP_SAFEDELA2(modification_of_pic_num[i]);
						}
					}

					int Map(AMBst in_bst)
					{
						int iRet = RET_CODE_SUCCESS;
						SYNTAX_BITSTREAM_MAP::Map(in_bst);

						try
						{
							MAP_BST_BEGIN(0);
							if (ptr_slice_header->slice_type % 5 != 2 && ptr_slice_header->slice_type % 5 != 4) {
								nal_read_u(in_bst, ref_pic_list_modification_flag[0], 1, int8_t);
								if (ref_pic_list_modification_flag[0])
								{
									AMP_NEW(modification_of_pic_num[0], MODIFICATION_OF_PIC_NUM, (size_t)ptr_slice_header->num_ref_idx_l0_active_minus1 + 1);
									if (modification_of_pic_num[0] == NULL)
										return RET_CODE_OUTOFMEMORY;

									int32_t val, i = 0;
									do
									{
										nal_read_ue(in_bst, val, int32_t);
										if (i < ptr_slice_header->num_ref_idx_l0_active_minus1 + 1)
											modification_of_pic_num[0][i].modification_of_pic_nums_idc = val;

										if (val == 0 || val == 1)
										{
											nal_read_ue(in_bst, val, int32_t);
											if (i < ptr_slice_header->num_ref_idx_l0_active_minus1 + 1)
												modification_of_pic_num[0][i].abs_diff_pic_num_minus1 = val;
										}
										else if (val == 2)
										{
											nal_read_ue(in_bst, val, int32_t);
											if (i < ptr_slice_header->num_ref_idx_l0_active_minus1 + 1)
												modification_of_pic_num[0][i].long_term_pic_idx = val;
										}
										else if (val == 4 || val == 5)
										{
											nal_read_ue(in_bst, val, int32_t);
											if (i < ptr_slice_header->num_ref_idx_l0_active_minus1 + 1)
												modification_of_pic_num[0][i].abs_diff_view_idx_minus1 = val;
										}

										i++;
									} while (val != 3);
								}
							}

							if (ptr_slice_header->slice_type % 5 == 1)
							{
								nal_read_u(in_bst, ref_pic_list_modification_flag[1], 1, int8_t);
								if (ref_pic_list_modification_flag[1])
								{
									AMP_NEW(modification_of_pic_num[1], MODIFICATION_OF_PIC_NUM, (size_t)ptr_slice_header->num_ref_idx_l1_active_minus1 + 1);
									if (modification_of_pic_num[1] == NULL)
										return RET_CODE_OUTOFMEMORY;

									int val, i = 0;
									do
									{
										nal_read_ue(in_bst, val, int32_t);
										if (i < ptr_slice_header->num_ref_idx_l1_active_minus1 + 1)
											modification_of_pic_num[1][i].modification_of_pic_nums_idc = val;

										if (val == 0 || val == 1)
										{
											nal_read_ue(in_bst, val, int32_t);
											if (i < ptr_slice_header->num_ref_idx_l1_active_minus1 + 1)
												modification_of_pic_num[1][i].abs_diff_pic_num_minus1 = val;
										}
										else if (val == 2)
										{
											nal_read_ue(in_bst, val, int32_t);
											if (i < ptr_slice_header->num_ref_idx_l1_active_minus1 + 1)
												modification_of_pic_num[1][i].long_term_pic_idx = val;
										}
										else if (val == 4 || val == 5)
										{
											nal_read_ue(in_bst, val, int32_t);
											if (i < ptr_slice_header->num_ref_idx_l1_active_minus1 + 1)
												modification_of_pic_num[1][i].abs_diff_view_idx_minus1 = val;
										}

										i++;
									} while (val != 3);
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
					NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "if(slice_type%5!=2 &amp;&amp; slice_type%5!=4)", "");
					NAV_FIELD_PROP_WITH_ALIAS_BEGIN("Tag00", "ref_pic_list_modification_flag_l0", 1, ref_pic_list_modification_flag[0] ? "1" : "0",
						ref_pic_list_modification_flag[0]
						? "modification_of_pic_nums_idc is present for specifying reference picture list 0"
						: "modification_of_pic_nums_idc is NOT present for specifying reference picture list 0", bit_offset ? *bit_offset : -1LL, "I");
					if (ref_pic_list_modification_flag[0])
					{
						for (uint16_t idx = 0; idx <= ptr_slice_header->num_ref_idx_l0_active_minus1; idx++)
						{
							BST_ARRAY_FIELD_PROP_UE2(modification_of_pic_nums_idc, idx,
								modification_of_pic_num[0][idx].modification_of_pic_nums_idc,
								"specifies which of the reference pictures or inter-view only reference components are re-mapped");
							if (modification_of_pic_num[0][idx].modification_of_pic_nums_idc == 0 ||
								modification_of_pic_num[0][idx].modification_of_pic_nums_idc == 1)
							{
								BST_ARRAY_FIELD_PROP_UE2(abs_diff_pic_num_minus1, idx, modification_of_pic_num[0][idx].abs_diff_pic_num_minus1,
									"plus 1 specifies the absolute difference between the picture number of the picture being moved to the current index in the list and the picture number prediction value");
							}
							else if (modification_of_pic_num[0][idx].modification_of_pic_nums_idc == 2)
							{
								BST_ARRAY_FIELD_PROP_UE2(long_term_pic_idx, idx, modification_of_pic_num[0][idx].long_term_pic_idx,
									"specifies the long-term picture number of the picture being moved to the current index in the list");
							}
							else if (modification_of_pic_num[0][idx].modification_of_pic_nums_idc == 4 ||
								modification_of_pic_num[0][idx].modification_of_pic_nums_idc == 5)
							{
								BST_ARRAY_FIELD_PROP_UE2(abs_diff_view_idx_minus1, idx, modification_of_pic_num[0][idx].abs_diff_view_idx_minus1,
									"plus 1 specifies the absolute difference between the reference view index to put to the current index in the reference picture list and the prediction value of the reference view index");
							}
						}
					}
					NAV_FIELD_PROP_END("Tag00");
					NAV_WRITE_TAG_END("Tag0");
					NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag1", "if(slice_type%5!==1)", "");
					NAV_FIELD_PROP_WITH_ALIAS_BEGIN("Tag10", "ref_pic_list_modification_flag_l1", 1, ref_pic_list_modification_flag[1] ? "1" : "0",
						ref_pic_list_modification_flag[1]
						? "modification_of_pic_nums_idc is present for specifying reference picture list 1"
						: "modification_of_pic_nums_idc is NOT present for specifying reference picture list 1", bit_offset ? *bit_offset : -1LL, "I");
					if (ref_pic_list_modification_flag[1])
					{
						for (uint16_t idx = 0; idx <= ptr_slice_header->num_ref_idx_l0_active_minus1; idx++)
						{
							BST_ARRAY_FIELD_PROP_UE2(modification_of_pic_nums_idc, idx,
								modification_of_pic_num[1][idx].modification_of_pic_nums_idc,
								"specifies which of the reference pictures or inter-view only reference components are re-mapped");
							if (modification_of_pic_num[1][idx].modification_of_pic_nums_idc == 0 ||
								modification_of_pic_num[1][idx].modification_of_pic_nums_idc == 1)
							{
								BST_ARRAY_FIELD_PROP_UE2(abs_diff_pic_num_minus1, idx, modification_of_pic_num[1][idx].abs_diff_pic_num_minus1,
									"plus 1 specifies the absolute difference between the picture number of the picture being moved to the current index in the list and the picture number prediction value");
							}
							else if (modification_of_pic_num[1][idx].modification_of_pic_nums_idc == 2)
							{
								BST_ARRAY_FIELD_PROP_UE2(long_term_pic_idx, idx, modification_of_pic_num[1][idx].long_term_pic_idx,
									"specifies the long-term picture number of the picture being moved to the current index in the list");
							}
							else if (modification_of_pic_num[1][idx].modification_of_pic_nums_idc == 4 ||
								modification_of_pic_num[1][idx].modification_of_pic_nums_idc == 5)
							{
								BST_ARRAY_FIELD_PROP_UE2(abs_diff_view_idx_minus1, idx, modification_of_pic_num[1][idx].abs_diff_view_idx_minus1,
									"plus 1 specifies the absolute difference between the reference view index to put to the current index in the reference picture list and the prediction value of the reference view index");
							}
						}
					}
					NAV_FIELD_PROP_END("Tag10");
					NAV_WRITE_TAG_END("Tag1");
					DECLARE_FIELDPROP_END()

				}PACKED;

				struct PRED_WEIGHT_TABLE : public SYNTAX_BITSTREAM_MAP
				{
					struct WEIGHTING_FACTORS
					{
						uint8_t				luma_weight_flag;
						int8_t				luma_weight;
						int8_t				luma_offset;
						uint8_t				chroma_weight_flag;
						int8_t				chroma_weight[2];
						int8_t				chroma_offset[2];
					}PACKED;

					uint8_t				luma_log2_weight_denom : 4;
					uint8_t				chroma_log2_weight_denom : 4;

					WEIGHTING_FACTORS	*weight_factors[2];
					SLICE_HEADER		*ptr_slice_header;
					uint8_t				m_ChromaArrayType;

					PRED_WEIGHT_TABLE(SLICE_HEADER *pSliceHdr, uint8_t ChromaArrayType)
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
								nal_read_ue(in_bst, chroma_log2_weight_denom, uint8_t);
							}

							AMP_NEW(weight_factors[0], WEIGHTING_FACTORS, (size_t)ptr_slice_header->num_ref_idx_l0_active_minus1 + 1);
							if (weight_factors[0] == NULL)
								return RET_CODE_OUTOFMEMORY;

							for (uint16_t i = 0; i <= ptr_slice_header->num_ref_idx_l0_active_minus1; i++)
							{
								nal_read_u(in_bst, weight_factors[0][i].luma_weight_flag, 1, uint8_t);
								if (weight_factors[0][i].luma_weight_flag)
								{
									nal_read_se(in_bst, weight_factors[0][i].luma_weight, int8_t);
									nal_read_se(in_bst, weight_factors[0][i].luma_offset, int8_t);
								}
								else
								{
									weight_factors[0][i].luma_weight = (int8_t)pow(2, luma_log2_weight_denom);
									weight_factors[0][i].luma_offset = 0;
								}

								if (m_ChromaArrayType != 0) {
									nal_read_u(in_bst, weight_factors[0][i].chroma_weight_flag, 1, uint8_t);
									if (weight_factors[0][i].chroma_weight_flag)
									{
										for (uint8_t j = 0; j < 2; j++)
										{
											nal_read_se(in_bst, weight_factors[0][i].chroma_weight[j], int8_t);
											nal_read_se(in_bst, weight_factors[0][i].chroma_offset[j], int8_t);
										}
									}
									else
									{
										weight_factors[0][i].chroma_weight[0] = weight_factors[0][i].chroma_weight[1] = (int8_t)pow(2, chroma_log2_weight_denom);
										weight_factors[0][i].chroma_offset[0] = weight_factors[0][i].chroma_offset[1] = 0;
									}
								}
							}

							if (ptr_slice_header->slice_type % 5 == 1)
							{
								AMP_NEW(weight_factors[1], WEIGHTING_FACTORS, (size_t)ptr_slice_header->num_ref_idx_l1_active_minus1 + 1);
								if (weight_factors[1] == NULL)
									return RET_CODE_OUTOFMEMORY;

								for (uint16_t i = 0; i <= ptr_slice_header->num_ref_idx_l1_active_minus1; i++)
								{
									nal_read_u(in_bst, weight_factors[1][i].luma_weight_flag, 1, uint8_t);
									if (weight_factors[1][i].luma_weight_flag)
									{
										nal_read_se(in_bst, weight_factors[1][i].luma_weight, int8_t);
										nal_read_se(in_bst, weight_factors[1][i].luma_offset, int8_t);
									}
									else
									{
										weight_factors[1][i].luma_weight = (int8_t)pow(2, luma_log2_weight_denom);
										weight_factors[1][i].luma_offset = 0;
									}

									if (m_ChromaArrayType != 0) {
										nal_read_u(in_bst, weight_factors[1][i].chroma_weight_flag, 1, uint8_t);
										if (weight_factors[1][i].chroma_weight_flag)
										{
											for (uint8_t j = 0; j < 2; j++)
											{
												nal_read_se(in_bst, weight_factors[1][i].chroma_weight[j], int8_t);
												nal_read_se(in_bst, weight_factors[1][i].chroma_offset[j], int8_t);
											}
										}
										else
										{
											weight_factors[1][i].chroma_weight[0] = weight_factors[1][i].chroma_weight[1] = (int8_t)pow(2, chroma_log2_weight_denom);
											weight_factors[1][i].chroma_offset[0] = weight_factors[1][i].chroma_offset[1] = 0;
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
						BST_FIELD_PROP_UE(chroma_log2_weight_denom, "the base 2 logarithm of the denominator for all chroma weighting factors");
					}

					NAV_WRITE_TAG_BEGIN_WITH_ALIAS_DESC_F("list", "ref_list0", "%" PRIu16 " weight factor in list", ptr_slice_header->num_ref_idx_l0_active_minus1);
					for (uint16_t ref_idx = 0; i <= ptr_slice_header->num_ref_idx_l0_active_minus1; i++)
					{
						MBCSPRINTF_S(szTemp4, TEMP4_SIZE, "luma_weight_l0_flag[%u]", ref_idx);
						BST_FIELD_PROP_NUMBER_WITH_ALIAS_BEGIN("luma_weight_l0_flag", szTemp4, 1, weight_factors[0][ref_idx].luma_weight_flag,
							"weighting factors for the luma component of list 0 prediction are present or not");

						if (weight_factors[0][ref_idx].luma_weight_flag)
						{
							MBCSPRINTF_S(szTemp4, TEMP4_SIZE, "luma_weight_l0[%u]", ref_idx);
							BST_FIELD_PROP_2NUMBER_WITH_ALIAS(szTemp4,
								(long long)quick_log2((weight_factors[0][ref_idx].luma_weight >= 0 ? weight_factors[0][ref_idx].luma_weight : ((-weight_factors[0][ref_idx].luma_weight) + 1)) + 1) * 2 + 1,
								weight_factors[0][ref_idx].luma_weight,
								"the weighting factor applied to the luma prediction value for list 0 prediction using RefPicList0[i]");
							MBCSPRINTF_S(szTemp4, TEMP4_SIZE, "luma_offset_l0[%u]", ref_idx);
							BST_FIELD_PROP_2NUMBER_WITH_ALIAS(szTemp4,
								(long long)quick_log2((weight_factors[0][ref_idx].luma_offset >= 0 ? weight_factors[0][ref_idx].luma_offset : ((-weight_factors[0][ref_idx].luma_offset) + 1)) + 1) * 2 + 1,
								weight_factors[0][ref_idx].luma_offset,
								"the additive offset applied to the luma prediction value for list 0 prediction using RefPicList0[i]");
						}

						BST_FIELD_PROP_NUMBER_END("luma_weight_l0_flag");

						if (m_ChromaArrayType != 0)
						{
							MBCSPRINTF_S(szTemp4, TEMP4_SIZE, "chroma_weight_l0_flag[%u]", ref_idx);
							BST_FIELD_PROP_NUMBER_WITH_ALIAS_BEGIN("chroma_weight_l0_flag", szTemp4, 1, weight_factors[0][ref_idx].chroma_weight_flag,
								"weighting factors for the chroma prediction values of list 0 prediction are present or not");

							if (weight_factors[0][ref_idx].chroma_weight_flag)
							{
								for (uint8_t j = 0; j < 2; j++)
								{
									MBCSPRINTF_S(szTemp4, TEMP4_SIZE, "chroma_weight_l0[%u][%u]", ref_idx, j);
									BST_FIELD_PROP_2NUMBER_WITH_ALIAS(szTemp4,
										(long long)quick_log2((weight_factors[0][ref_idx].chroma_weight[j] >= 0 ? weight_factors[0][ref_idx].chroma_weight[j] : ((-weight_factors[0][ref_idx].chroma_weight[j]) + 1)) + 1) * 2 + 1,
										weight_factors[0][ref_idx].chroma_weight[j], j == 0 ?
										"the weighting factor applied to the chroma prediction values for list 0 prediction using RefPicList0[i] for Cb" :
										"the weighting factor applied to the chroma prediction values for list 0 prediction using RefPicList0[i] for Cr");
									MBCSPRINTF_S(szTemp4, TEMP4_SIZE, "chroma_offset_l0[%u][%u]", ref_idx, j);
									BST_FIELD_PROP_2NUMBER_WITH_ALIAS(szTemp4,
										(long long)quick_log2((weight_factors[0][ref_idx].chroma_offset[j] >= 0 ? weight_factors[0][ref_idx].chroma_offset[j] : ((-weight_factors[0][ref_idx].chroma_offset[j]) + 1)) + 1) * 2 + 1,
										weight_factors[0][ref_idx].chroma_offset[j], j == 0 ?
										"the additive offset applied to the chroma prediction values for list 0 prediction using RefPicList0[i] for Cb" :
										"the additive offset applied to the chroma prediction values for list 0 prediction using RefPicList0[i] for Cr");
								}
							}

							BST_FIELD_PROP_NUMBER_END("chroma_weight_l0_flag");
						}
					}
					NAV_WRITE_TAG_END("list");

					if (ptr_slice_header->slice_type % 5 == 1)
					{
						NAV_WRITE_TAG_BEGIN_WITH_ALIAS_DESC_F("list", "ref_list1", "%" PRIu16 " weight factor in list", ptr_slice_header->num_ref_idx_l1_active_minus1);
						for (uint16_t ref_idx = 0; i <= ptr_slice_header->num_ref_idx_l0_active_minus1; i++)
						{
							MBCSPRINTF_S(szTemp4, TEMP4_SIZE, "luma_weight_l1_flag[%u]", ref_idx);
							BST_FIELD_PROP_NUMBER_WITH_ALIAS_BEGIN("luma_weight_l1_flag", szTemp4, 1, weight_factors[1][ref_idx].luma_weight_flag,
								"weighting factors for the luma component of list 1 prediction are present or not");

							if (weight_factors[1][ref_idx].luma_weight_flag)
							{
								MBCSPRINTF_S(szTemp4, TEMP4_SIZE, "luma_weight_l1[%u]", ref_idx);
								BST_FIELD_PROP_2NUMBER_WITH_ALIAS(szTemp4,
									(long long)quick_log2((weight_factors[1][ref_idx].luma_weight >= 0 ? weight_factors[1][ref_idx].luma_weight : ((-weight_factors[1][ref_idx].luma_weight) + 1)) + 1) * 2 + 1,
									weight_factors[1][ref_idx].luma_weight,
									"the weighting factor applied to the luma prediction value for list 1 prediction using RefPicList0[i]");
								MBCSPRINTF_S(szTemp4, TEMP4_SIZE, "luma_offset_l1[%u]", ref_idx);
								BST_FIELD_PROP_2NUMBER_WITH_ALIAS(szTemp4,
									(long long)quick_log2((weight_factors[1][ref_idx].luma_offset >= 0 ? weight_factors[1][ref_idx].luma_offset : ((-weight_factors[1][ref_idx].luma_offset) + 1)) + 1) * 2 + 1,
									weight_factors[1][ref_idx].luma_offset,
									"the additive offset applied to the luma prediction value for list 1 prediction using RefPicList0[i]");
							}

							BST_FIELD_PROP_NUMBER_END("luma_weight_l1_flag");

							if (m_ChromaArrayType != 0)
							{
								MBCSPRINTF_S(szTemp4, TEMP4_SIZE, "chroma_weight_l1_flag[%u]", ref_idx);
								BST_FIELD_PROP_NUMBER_WITH_ALIAS_BEGIN("chroma_weight_l1_flag", szTemp4, 1, weight_factors[1][ref_idx].chroma_weight_flag,
									"weighting factors for the chroma prediction values of list 1 prediction are present or not");

								if (weight_factors[1][ref_idx].chroma_weight_flag)
								{
									for (uint8_t j = 0; j < 2; j++)
									{
										MBCSPRINTF_S(szTemp4, TEMP4_SIZE, "chroma_weight_l1[%u][%u]", ref_idx, j);
										BST_FIELD_PROP_2NUMBER_WITH_ALIAS(szTemp4,
											(long long)quick_log2((weight_factors[1][ref_idx].chroma_weight[j] >= 0 ? weight_factors[1][ref_idx].chroma_weight[j] : ((-weight_factors[1][ref_idx].chroma_weight[j]) + 1)) + 1) * 2 + 1,
											weight_factors[1][ref_idx].chroma_weight[j], j == 0 ?
											"the weighting factor applied to the chroma prediction values for list 1 prediction using RefPicList0[i] for Cb" :
											"the weighting factor applied to the chroma prediction values for list 1 prediction using RefPicList0[i] for Cr");
										MBCSPRINTF_S(szTemp4, TEMP4_SIZE, "chroma_offset_l1[%u][%u]", ref_idx, j);
										BST_FIELD_PROP_2NUMBER_WITH_ALIAS(szTemp4,
											(long long)quick_log2((weight_factors[1][ref_idx].chroma_offset[j] >= 0 ? weight_factors[1][ref_idx].chroma_offset[j] : ((-weight_factors[1][ref_idx].chroma_offset[j]) + 1)) + 1) * 2 + 1,
											weight_factors[1][ref_idx].chroma_offset[j], j == 0 ?
											"the additive offset applied to the chroma prediction values for list 1 prediction using RefPicList0[i] for Cb" :
											"the additive offset applied to the chroma prediction values for list 1 prediction using RefPicList0[i] for Cr");
									}
								}

								BST_FIELD_PROP_NUMBER_END("chroma_weight_l1_flag");
							}
						}

						NAV_WRITE_TAG_END("list1");
					}
					DECLARE_FIELDPROP_END()
				}PACKED;

				struct DEC_REF_PIC_MARKING : public SYNTAX_BITSTREAM_MAP
				{
					uint8_t				nal_unit_type : 5;
					uint8_t				no_output_of_prior_pics_flag : 1;
					uint8_t				long_term_reference_flag : 1;
					uint8_t				adaptive_ref_pic_marking_mode_flag : 1;

					struct ADAPTIVE_REF_PIC_MARKING
					{
						uint8_t				memory_management_control_operation;
						uint32_t			difference_of_pic_nums_minus1;
						uint32_t			long_term_pic_num;
						uint32_t			long_term_frame_idx;
						uint32_t			max_long_term_frame_idx_plus1;

						ADAPTIVE_REF_PIC_MARKING* pNext;
					}PACKED;

					ADAPTIVE_REF_PIC_MARKING*	adaptive_ref_pic_marking;
					uint8_t						m_IdrPicFlag;

					DEC_REF_PIC_MARKING(uint8_t IdrPicFlag)
						: nal_unit_type(0)
						, no_output_of_prior_pics_flag(0)
						, long_term_reference_flag(0)
						, adaptive_ref_pic_marking_mode_flag(0)
						, adaptive_ref_pic_marking(NULL)
						, m_IdrPicFlag(IdrPicFlag){
					}

					virtual ~DEC_REF_PIC_MARKING()
					{
						// Free the link list
						while (adaptive_ref_pic_marking != NULL)
						{
							ADAPTIVE_REF_PIC_MARKING* pMarking = adaptive_ref_pic_marking;
							adaptive_ref_pic_marking = adaptive_ref_pic_marking->pNext;
							AMP_SAFEDEL4(pMarking);
						}
					}

					int Map(AMBst in_bst)
					{
						int iRet = RET_CODE_SUCCESS;
						SYNTAX_BITSTREAM_MAP::Map(in_bst);

						try
						{
							MAP_BST_BEGIN(0);

							if (m_IdrPicFlag)
							{
								nal_read_u(in_bst, no_output_of_prior_pics_flag, 1, uint8_t);
								nal_read_u(in_bst, long_term_reference_flag, 1, uint8_t);
							}
							else
							{
								nal_read_u(in_bst, adaptive_ref_pic_marking_mode_flag, 1, uint8_t);
								if (adaptive_ref_pic_marking_mode_flag)
								{
									uint8_t memory_management_control_operation = 0;
									ADAPTIVE_REF_PIC_MARKING* pLastMarking = NULL;
									do
									{
										nal_read_ue(in_bst, memory_management_control_operation, uint8_t);
										if (memory_management_control_operation != 0)
										{
											AMP_NEWT1(pMarking, ADAPTIVE_REF_PIC_MARKING);
											if (pMarking == nullptr)
												throw AMException(RET_CODE_OUTOFMEMORY);

											pMarking->memory_management_control_operation = memory_management_control_operation;

											if (memory_management_control_operation == 1 ||
												memory_management_control_operation == 3)
											{
												nal_read_ue(in_bst, pMarking->difference_of_pic_nums_minus1, uint32_t);
											}

											if (memory_management_control_operation == 2)
											{
												nal_read_ue(in_bst, pMarking->long_term_pic_num, uint32_t);
											}

											if (memory_management_control_operation == 3 ||
												memory_management_control_operation == 6)
											{
												nal_read_ue(in_bst, pMarking->long_term_frame_idx, uint32_t);
											}

											if (memory_management_control_operation == 4)
											{
												nal_read_ue(in_bst, pMarking->max_long_term_frame_idx_plus1, uint32_t);
											}

											pMarking->pNext = NULL;

											if (adaptive_ref_pic_marking == NULL)
												adaptive_ref_pic_marking = pMarking;

											if (pLastMarking != NULL)
												pLastMarking->pNext = pMarking;

											pLastMarking = pMarking;
										}
									} while (memory_management_control_operation != 0);
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
					if (m_IdrPicFlag)
					{
						BST_FIELD_PROP_2NUMBER1(no_output_of_prior_pics_flag, 1, "specifies how the previously-decoded pictures in the decoded picture buffer are treated after decoding of an IDR picture");
						BST_FIELD_PROP_BOOL(long_term_reference_flag, "", "");
					}
					else
					{
						BST_FIELD_PROP_BOOL(adaptive_ref_pic_marking_mode_flag,
							"the MaxLongTermFrameIdx variable is set equal to 0 and that the current IDR picture is marked &quot;used for long - term reference&quot; and is assigned LongTermFrameIdx equal to 0.",
							"the MaxLongTermFrameIdx variable is set equal to &quot;no long-term frame indices&quot; and that the IDR picture is marked as &quot;used for short-term reference&quot; ");
						if (adaptive_ref_pic_marking_mode_flag)
						{
							i = 0;
							ADAPTIVE_REF_PIC_MARKING* pMarking = adaptive_ref_pic_marking;
							while (pMarking != NULL)
							{
								NAV_WRITE_TAG_ARRAY_BEGIN("mode", i, MEMORY_MANAGEMENT_CONTROL_OPERATION_DESC(pMarking->memory_management_control_operation));
								MBCSPRINTF_S(szTemp4, TEMP4_SIZE, "memory_management_control_operation[%d]", i);
								BST_FIELD_PROP_2NUMBER_WITH_ALIAS(szTemp4, (long long)quick_log2(pMarking->memory_management_control_operation + 1) * 2 + 1,
									pMarking->memory_management_control_operation, "specifies a control operation to be applied to affect the reference picture marking");

								if (pMarking->memory_management_control_operation == 1 ||
									pMarking->memory_management_control_operation == 3)
								{
									MBCSPRINTF_S(szTemp4, TEMP4_SIZE, "difference_of_pic_nums_minus1[%d]", i);
									BST_FIELD_PROP_2NUMBER_WITH_ALIAS(szTemp4, (long long)quick_log2(pMarking->difference_of_pic_nums_minus1 + 1) * 2 + 1,
										pMarking->difference_of_pic_nums_minus1,
										"assign a long-term frame index to a short-term reference picture or to mark a short-term reference picture as &quot;unused for reference&quot;, the resulting picture number derived from difference_of_pic_nums_minus1 shall be a picture number assigned to one of the reference pictures marked as &quot;used for reference&quot; and not previously assigned to a long-term frame index");
								}

								if (pMarking->memory_management_control_operation == 2)
								{
									MBCSPRINTF_S(szTemp4, TEMP4_SIZE, "long_term_pic_num[%d]", i);
									BST_FIELD_PROP_2NUMBER_WITH_ALIAS(szTemp4, (long long)quick_log2(pMarking->long_term_pic_num + 1) * 2 + 1,
										pMarking->long_term_pic_num,
										"mark a long-term reference picture as &quot;unused for reference&quot;, a long - term picture number assigned to one of the reference pictures that is currently marked as &quot; used for long - term reference&quot; ");
								}

								if (pMarking->memory_management_control_operation == 3 ||
									pMarking->memory_management_control_operation == 6)
								{
									MBCSPRINTF_S(szTemp4, TEMP4_SIZE, "long_term_frame_idx[%d]", i);
									BST_FIELD_PROP_2NUMBER_WITH_ALIAS(szTemp4, (long long)quick_log2(pMarking->long_term_frame_idx + 1) * 2 + 1,
										pMarking->long_term_frame_idx,
										"assign a long-term frame index to a picture");
								}

								if (pMarking->memory_management_control_operation == 4)
								{
									MBCSPRINTF_S(szTemp4, TEMP4_SIZE, "max_long_term_frame_idx_plus1[%d]", i);
									BST_FIELD_PROP_2NUMBER_WITH_ALIAS(szTemp4, (long long)quick_log2(pMarking->max_long_term_frame_idx_plus1 + 1) * 2 + 1,
										pMarking->max_long_term_frame_idx_plus1,
										"minus 1 specifies the maximum value of long-term frame index allowed for long-term reference pictures (until receipt of another value of max_long_term_frame_idx_plus1)");
								}
								NAV_WRITE_TAG_END("mode");

								pMarking = pMarking->pNext;
								i++;
							}
						}
					}
					DECLARE_FIELDPROP_END()

				}PACKED;

				uint32_t			first_mb_in_slice;
				uint8_t				slice_type;
				uint8_t				pic_parameter_set_id;
				uint8_t				colour_plane_id;
				uint16_t			frame_num;

				uint8_t				field_pic_flag : 1;
				uint8_t				bottom_field_flag : 1;
				uint8_t				aligned_byte_0 : 6;

				uint16_t			idr_pic_id;
				uint16_t			pic_order_cnt_lsb;
				int32_t				delta_pic_order_cnt_bottom;

				int32_t				delta_pic_order_cnt[2];
				uint8_t				redundant_pic_cnt;

				uint16_t			direct_spatial_mv_pred_flag : 1;
				uint16_t			num_ref_idx_active_override_flag : 1;
				uint16_t			num_ref_idx_l0_active_minus1 : 5;
				uint16_t			num_ref_idx_l1_active_minus1 : 5;
				uint16_t			aligned_word_1 : 4;

				uint8_t				cabac_init_idc : 2;
				uint8_t				sp_for_switch_flag : 1;
				uint8_t				disable_deblocking_filter_idc : 2;
				uint8_t				aligned_byte_2 : 3;

				int8_t				slice_qp_delta;
				int8_t				slice_qs_delta;
				int8_t				slice_alpha_c0_offset_div2;
				int8_t				slice_beta_offset_div2;

				uint32_t			slice_group_change_cycle;

				union
				{
					REF_PIC_LIST_MVC_MODIFICATION*
						ptr_ref_pic_list_mvc_modification;
					REF_PIC_LIST_MODIFICATION*
						ptr_ref_pic_list_modification;
				};

				PRED_WEIGHT_TABLE*	ptr_pred_weight_table;
				DEC_REF_PIC_MARKING*
									ptr_dec_ref_pic_marking;

				std::shared_ptr<NAL_UNIT>
									sp_pps;
				NAL_UNIT*			ptr_nal_unit;

				SLICE_HEADER(NAL_UNIT* pNALUnit)
					: field_pic_flag(0)
					, redundant_pic_cnt(0)
					, ptr_ref_pic_list_modification(NULL)
					, ptr_pred_weight_table(NULL)
					, ptr_dec_ref_pic_marking(NULL)
					, ptr_nal_unit(pNALUnit)
				{
				}

				virtual ~SLICE_HEADER()
				{
					if (ptr_nal_unit->nal_unit_header.nal_unit_type == 20 || ptr_nal_unit->nal_unit_header.nal_unit_type == 21)
					{
						AMP_SAFEDEL2(ptr_ref_pic_list_mvc_modification);
					}
					else
					{
						AMP_SAFEDEL2(ptr_ref_pic_list_modification);
					}
					AMP_SAFEDEL2(ptr_pred_weight_table);
					AMP_SAFEDEL2(ptr_dec_ref_pic_marking);
				}

				int Map(AMBst in_bst)
				{
					int iRet = RET_CODE_SUCCESS;
					SYNTAX_BITSTREAM_MAP::Map(in_bst);

					try
					{
						MAP_BST_BEGIN(0);

						nal_read_ue(in_bst, first_mb_in_slice, uint32_t);
						nal_read_ue(in_bst, slice_type, uint8_t);
						nal_read_ue(in_bst, pic_parameter_set_id, uint8_t);

						if (ptr_nal_unit == NULL || ptr_nal_unit->ptr_ctx_video_bst == NULL)
						{
							printf("[H264] Picture Parameter Set with pic_parameter_set_id: %d is not available.\n", pic_parameter_set_id);
							throw AMException(RET_CODE_ERROR_NOTIMPL);
						}

						sp_pps = ptr_nal_unit->ptr_ctx_video_bst->GetAVCPPS(pic_parameter_set_id);
						if (!sp_pps || sp_pps->ptr_pic_parameter_set_rbsp == NULL)
						{
							printf("[H264] Picture Parameter Set with pic_parameter_set_id: %d is not available.\n", pic_parameter_set_id);
							throw AMException(RET_CODE_ERROR_NOTIMPL);
						}

						auto sps_unit = ptr_nal_unit->ptr_ctx_video_bst->GetAVCSPS(sp_pps->ptr_pic_parameter_set_rbsp->seq_parameter_set_id);
						if (!sps_unit || sps_unit->ptr_seq_parameter_set_rbsp == nullptr)
						{
							printf("[H264] Sequence Parameter Set is not available.\n");
							throw AMException(RET_CODE_ERROR_NOTIMPL);
						}

						auto sps = sps_unit->ptr_seq_parameter_set_rbsp;

						if (sps->seq_parameter_set_data.separate_colour_plane_flag)
							nal_read_u(in_bst, colour_plane_id, 2, uint8_t);

						nal_read_u(in_bst, frame_num, sps->seq_parameter_set_data.log2_max_frame_num_minus4 + 4, uint16_t);

						if (!sps->seq_parameter_set_data.frame_mbs_only_flag)
						{
							nal_read_u(in_bst, field_pic_flag, 1, uint8_t);
							if (field_pic_flag)
								nal_read_u(in_bst, bottom_field_flag, 1, uint8_t);
						}

						if (ptr_nal_unit->nal_unit_header.nal_unit_type == 5)
							nal_read_ue(in_bst, idr_pic_id, uint16_t);

						if (sps->seq_parameter_set_data.pic_order_cnt_type == 0)
						{
							nal_read_u(in_bst, pic_order_cnt_lsb, sps->seq_parameter_set_data.log2_max_pic_order_cnt_lsb_minus4 + 4, uint16_t);
							if (sp_pps->ptr_pic_parameter_set_rbsp->bottom_field_pic_order_in_frame_present_flag && !field_pic_flag)
								nal_read_se(in_bst, delta_pic_order_cnt_bottom, int32_t);
						}

						if (sps->seq_parameter_set_data.pic_order_cnt_type == 1 && !sps->seq_parameter_set_data.delta_pic_order_always_zero_flag)
						{
							nal_read_se(in_bst, delta_pic_order_cnt[0], int32_t);
							if (sp_pps->ptr_pic_parameter_set_rbsp->bottom_field_pic_order_in_frame_present_flag && !field_pic_flag)
								nal_read_se(in_bst, delta_pic_order_cnt[1], int32_t);
						}

						if (sp_pps->ptr_pic_parameter_set_rbsp->redundant_pic_cnt_present_flag)
							nal_read_ue(in_bst, redundant_pic_cnt, uint8_t);

						if (slice_type % 5 == 1)
							nal_read_u(in_bst, direct_spatial_mv_pred_flag, 1, uint16_t);

						num_ref_idx_l0_active_minus1 = sp_pps->ptr_pic_parameter_set_rbsp->num_ref_idx_l0_default_active_minus1;
						num_ref_idx_l1_active_minus1 = sp_pps->ptr_pic_parameter_set_rbsp->num_ref_idx_l1_default_active_minus1;

						if (slice_type % 5 == 0 || slice_type % 5 == 1 || slice_type % 5 == 3)
						{
							nal_read_u(in_bst, num_ref_idx_active_override_flag, 1, uint16_t);

							if (num_ref_idx_active_override_flag)
							{
								nal_read_ue(in_bst, num_ref_idx_l0_active_minus1, uint16_t);
								if (slice_type % 5 == 1)
									nal_read_ue(in_bst, num_ref_idx_l1_active_minus1, uint16_t);
							}
						}

						if (ptr_nal_unit->nal_unit_header.nal_unit_type == 20 || ptr_nal_unit->nal_unit_header.nal_unit_type == 21)
						{
							nal_read_ref(in_bst, ptr_ref_pic_list_mvc_modification, REF_PIC_LIST_MVC_MODIFICATION, this);
						}
						else
						{
							nal_read_ref(in_bst, ptr_ref_pic_list_modification, REF_PIC_LIST_MODIFICATION, this);
						}

						uint8_t ChromaArrayType = 0;
						if (sps->seq_parameter_set_data.separate_colour_plane_flag == 0)
							ChromaArrayType = sps->seq_parameter_set_data.chroma_format_idc;

						if ((sp_pps->ptr_pic_parameter_set_rbsp->weighted_pred_flag && (slice_type % 5 == 0 || slice_type % 5 == 3)) ||
							(sp_pps->ptr_pic_parameter_set_rbsp->weighted_bipred_idc == 1 && slice_type % 5 == 1))
						{
							nal_read_ref(in_bst, ptr_pred_weight_table, PRED_WEIGHT_TABLE, this, ChromaArrayType);
						}

						if (ptr_nal_unit->nal_unit_header.nal_ref_idc != 0)
						{
							nal_read_ref(in_bst, ptr_dec_ref_pic_marking, DEC_REF_PIC_MARKING, ptr_nal_unit->nal_unit_header.nal_unit_type == 5 ? 1 : 0);
						}

						if (sp_pps->ptr_pic_parameter_set_rbsp->entropy_coding_mode_flag && slice_type % 5 != 2 && slice_type % 5 != 4)
						{
							nal_read_ue(in_bst, cabac_init_idc, uint8_t);
						}

						nal_read_se(in_bst, slice_qp_delta, int8_t);

						if (slice_type % 5 == 3 || slice_type % 5 == 4)
						{
							if (slice_type % 5 == 3)
							{
								nal_read_u(in_bst, sp_for_switch_flag, 1, uint8_t);
							}

							nal_read_se(in_bst, slice_qs_delta, int8_t);
						}

						if (sp_pps->ptr_pic_parameter_set_rbsp->deblocking_filter_control_present_flag)
						{
							nal_read_ue(in_bst, disable_deblocking_filter_idc, uint8_t);
							if (disable_deblocking_filter_idc != 1)
							{
								nal_read_se(in_bst, slice_alpha_c0_offset_div2, int8_t);
								nal_read_se(in_bst, slice_beta_offset_div2, int8_t);
							}
						}

						if (sp_pps->ptr_pic_parameter_set_rbsp->num_slice_groups_minus1 > 0 &&
							sp_pps->ptr_pic_parameter_set_rbsp->slice_group_map_type >= 3 && sp_pps->ptr_pic_parameter_set_rbsp->slice_group_map_type <= 5)
						{
							uint16_t PicWidthInMbs = sps->seq_parameter_set_data.pic_width_in_mbs_minus1 + 1;
							uint16_t PicHeightInMapUnits = sps->seq_parameter_set_data.pic_height_in_map_units_minus1 + 1;
							uint32_t PicSizeInMapUnits = PicWidthInMbs*PicHeightInMapUnits;
							uint16_t SliceGroupChangeRate = sp_pps->ptr_pic_parameter_set_rbsp->slice_group_change_rate_minus1 + 1;

							uint8_t number_of_bits = quick_ceil_log2(PicSizeInMapUnits / SliceGroupChangeRate + 1);

							nal_read_u(in_bst, slice_group_change_cycle, number_of_bits, uint32_t);
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
				BST_FIELD_PROP_UE(first_mb_in_slice, "specifies the address of the first macroblock in the slice");
				BST_FIELD_PROP_UE(slice_type, h264_slice_type_names[slice_type]);
				BST_FIELD_PROP_UE(pic_parameter_set_id, "specifies the picture parameter set in use");

				H264_NU sp_sps_nu;
				SEQ_PARAMETER_SET_RBSP* sps = NULL;
				uint8_t ChromaArrayType = 0;

				if (!sp_pps || sp_pps->ptr_pic_parameter_set_rbsp)
					goto done;

				sp_sps_nu = ptr_nal_unit->ptr_ctx_video_bst->GetAVCSPS(sp_pps->ptr_pic_parameter_set_rbsp->seq_parameter_set_id);
				if (!sp_sps_nu || sp_sps_nu->ptr_seq_parameter_set_rbsp == NULL)
					goto done;

				sps = sp_sps_nu->ptr_seq_parameter_set_rbsp;
				if (sps->seq_parameter_set_data.separate_colour_plane_flag)
				{
					MBCSPRINTF_S(szTemp2, TEMP2_SIZE, "specifies the %s colour plane associated with the current slice RBSP", colour_plane_id == 0 ? "Y" : (colour_plane_id == 1 ? "Cb" : (colour_plane_id == 2 ? "Cr" : "Unknown")));
					BST_FIELD_PROP_2NUMBER1(colour_plane_id, 2, szTemp2);
				}

				BST_FIELD_PROP_2NUMBER1(frame_num, (long long)sps->seq_parameter_set_data.log2_max_frame_num_minus4 + 4, "used as an identifier for pictures");

				if (!sps->seq_parameter_set_data.frame_mbs_only_flag)
				{
					static int MbWidthC[4] = { 0, 8, 8,  16 };
					static int MbHeightC[4] = { 0, 8, 16, 16 };
					uint16_t PicHeightInMapUnits = sps->seq_parameter_set_data.pic_height_in_map_units_minus1 + 1;
					uint16_t FrameHeightInMbs = (2 - sps->seq_parameter_set_data.frame_mbs_only_flag) * PicHeightInMapUnits;
					uint16_t PicHeightInMbs = FrameHeightInMbs / (1 + field_pic_flag);
					uint16_t PicWidthInMbs = sps->seq_parameter_set_data.pic_width_in_mbs_minus1 + 1;
					BST_FIELD_PROP_BOOL(field_pic_flag, "the slice is a slice of a coded field", "the slice is a slice of a coded frame");
					NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag1", "MbaffFrameFlag", (sps->seq_parameter_set_data.mb_adaptive_frame_field_flag && !field_pic_flag), "Macroblock-Adaptive Frame/Field Coding flag");
					NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag2", "PicHeightInMbs", PicHeightInMbs, "the picture height in units of macroblocks");
					NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag3", "PicHeightInSamples_L", PicHeightInMbs * 16, "picture height for the luma component");
					NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag4", "PicHeightInSamples_C", PicHeightInMbs * MbWidthC[sps->seq_parameter_set_data.chroma_format_idc], "picture height for the luma component");
					NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag5", "PicSizeInMbs", PicWidthInMbs * PicHeightInMbs, "picture height for the luma component");
					if (field_pic_flag)
						BST_FIELD_PROP_BOOL(bottom_field_flag, "the slice is part of a coded bottom field", "the picture is a coded top field");
				}

				if (ptr_nal_unit->nal_unit_header.nal_unit_type == 5)
					BST_FIELD_PROP_UE(idr_pic_id, "identifies an IDR picture. The values of idr_pic_id in all the slices of an IDR picture shall remain unchanged");

				if (sps->seq_parameter_set_data.pic_order_cnt_type == 0)
				{
					BST_FIELD_PROP_2NUMBER1(pic_order_cnt_lsb, (long long)sps->seq_parameter_set_data.log2_max_pic_order_cnt_lsb_minus4 + 4, "specifies the picture order count modulo MaxPicOrderCntLsb for the top field of a coded frame or for a coded field");
					if (sp_pps->ptr_pic_parameter_set_rbsp->bottom_field_pic_order_in_frame_present_flag && !field_pic_flag)
					{
						BST_FIELD_PROP_SE(delta_pic_order_cnt_bottom, "specifies the picture order count difference between the bottom field and the top field of a coded frame");
					}
				}

				if (sps->seq_parameter_set_data.pic_order_cnt_type == 1 && !sps->seq_parameter_set_data.delta_pic_order_always_zero_flag)
				{
					BST_ARRAY_FIELD_PROP_SE(delta_pic_order_cnt, 0, "specifies the picture order count difference from the expected picture order count for the top field of a coded frame or for a coded field as specified in clause 8.2.1");

					if (sp_pps->ptr_pic_parameter_set_rbsp->bottom_field_pic_order_in_frame_present_flag && !field_pic_flag)
						BST_ARRAY_FIELD_PROP_SE(delta_pic_order_cnt, 1, "specifies the picture order count difference from the expected picture order count for the bottom field of a coded frame specified in clause 8.2.1");
				}

				if (sp_pps->ptr_pic_parameter_set_rbsp->redundant_pic_cnt_present_flag)
					BST_FIELD_PROP_UE(redundant_pic_cnt,
						"shall be equal to 0 for slices and slice data partitions belonging to the primary coded picture. The value of redundant_pic_cnt shall be greater than 0 for coded slices or coded slice data partitions of a redundant coded picture.");

				if (slice_type % 5 == 1)
					BST_FIELD_PROP_BOOL(direct_spatial_mv_pred_flag,
						"the derivation process for luma motion vectors for B_Skip, B_Direct_16x16, and B_Direct_8x8 in clause 8.4.1.2 shall use spatial direct mode prediction as specified in clause 8.4.1.2.2.",
						"the derivation process for luma motion vectors for B_Skip, B_Direct_16x16, and B_Direct_8x8 in clause 8.4.1.2 shall use temporal direct mode prediction as specified in clause 8.4.1.2.3");

				if (slice_type % 5 == 0 || slice_type % 5 == 1 || slice_type % 5 == 3)
				{
					BST_FIELD_PROP_BOOL(num_ref_idx_active_override_flag,
						"specifies that the syntax element num_ref_idx_l0_active_minus1 is present for P, SP, and B slices and that the syntax element num_ref_idx_l1_active_minus1 is present for B slices",
						"specifies that the syntax elements num_ref_idx_l0_active_minus1 and num_ref_idx_l1_active_minus1 are not present");

					if (num_ref_idx_active_override_flag)
					{
						BST_FIELD_PROP_UE(num_ref_idx_l0_active_minus1, "specifies the maximum reference index for reference picture list 0 that shall be used to decode the slice");
						if (slice_type % 5 == 1)
							BST_FIELD_PROP_UE(num_ref_idx_l0_active_minus1, "specifies the maximum reference index for reference picture list 1 that shall be used to decode the slice");
					}
				}

				if (ptr_nal_unit->nal_unit_header.nal_unit_type == 20 || ptr_nal_unit->nal_unit_header.nal_unit_type == 21) {
					NAV_WRITE_TAG_BEGIN("ref_pic_list_mvc_modification");
					BST_FIELD_PROP_REF1(ptr_ref_pic_list_mvc_modification);
					NAV_WRITE_TAG_END("ref_pic_list_mvc_modification");
				}
				else
				{
					NAV_WRITE_TAG_BEGIN("ref_pic_list_modification");
					BST_FIELD_PROP_REF1(ptr_ref_pic_list_modification);
					NAV_WRITE_TAG_END("ref_pic_list_modification");
				}

				if (sps->seq_parameter_set_data.separate_colour_plane_flag == 0)
					ChromaArrayType = sps->seq_parameter_set_data.chroma_format_idc;

				if ((sp_pps->ptr_pic_parameter_set_rbsp->weighted_pred_flag && (slice_type % 5 == 0 || slice_type % 5 == 3)) ||
					(sp_pps->ptr_pic_parameter_set_rbsp->weighted_bipred_idc == 1 && slice_type % 5 == 1))
				{
					NAV_WRITE_TAG_BEGIN("pred_weight_table");
					BST_FIELD_PROP_REF1(ptr_pred_weight_table);
					NAV_WRITE_TAG_END("pred_weight_table");
				}

				if (ptr_nal_unit->nal_unit_header.nal_ref_idc != 0)
				{
					NAV_WRITE_TAG_BEGIN("dec_ref_pic_marking");
					BST_FIELD_PROP_REF1(ptr_dec_ref_pic_marking);
					NAV_WRITE_TAG_END("dec_ref_pic_marking");
				}

				if (sp_pps->ptr_pic_parameter_set_rbsp->entropy_coding_mode_flag && slice_type % 5 != 2 && slice_type % 5 != 4)
					BST_FIELD_PROP_UE(cabac_init_idc, "specifies the index for determining the initialization table used in the initialization process for context variables");

				BST_FIELD_PROP_SE(slice_qp_delta, "specifies the initial value of QPY to be used for all the macroblocks in the slice until modified by the value of mb_qp_delta in the macroblock layer");
				NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag6", "SliceQPY", 26 + sp_pps->ptr_pic_parameter_set_rbsp->pic_init_qp_minus26 + slice_qp_delta, "");

				if (slice_type % 5 == 3 || slice_type % 5 == 4)
				{
					if (slice_type % 5 == 3)
						BST_FIELD_PROP_BOOL(sp_for_switch_flag,
							"the P macroblocks in the SP slice shall be decoded using the SP and SI decoding process for switching pictures as specified in clause 8.6.2",
							"the P macroblocks in the SP slice shall be decoded using the SP decoding process for non-switching pictures as specified in clause 8.6.1");

					BST_FIELD_PROP_SE(slice_qs_delta, "specifies the value of QSY for all the macroblocks in SP and SI slices");
					NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag7", "QSY", 26 + sp_pps->ptr_pic_parameter_set_rbsp->pic_init_qs_minus26 + slice_qs_delta, "");
				}

				if (sp_pps->ptr_pic_parameter_set_rbsp->deblocking_filter_control_present_flag)
				{
					BST_FIELD_PROP_UE(disable_deblocking_filter_idc, "");
					if (disable_deblocking_filter_idc != 1)
					{
						BST_FIELD_PROP_SE(slice_alpha_c0_offset_div2, "specifies the offset used in accessing the alpha and tc0 deblocking filter tables for filtering operations controlled by the macroblocks within the slice");
						BST_FIELD_PROP_SE(slice_beta_offset_div2, "specifies the offset used in accessing the beta deblocking filter table for filtering operations controlled by the macroblocks within the slice");
					}
				}

				if (sp_pps->ptr_pic_parameter_set_rbsp->num_slice_groups_minus1 > 0 &&
					sp_pps->ptr_pic_parameter_set_rbsp->slice_group_map_type >= 3 && sp_pps->ptr_pic_parameter_set_rbsp->slice_group_map_type <= 5)
				{
					uint16_t PicWidthInMbs = sps->seq_parameter_set_data.pic_width_in_mbs_minus1 + 1;
					uint16_t PicHeightInMapUnits = sps->seq_parameter_set_data.pic_height_in_map_units_minus1 + 1;
					uint32_t PicSizeInMapUnits = PicWidthInMbs*PicHeightInMapUnits;
					uint16_t SliceGroupChangeRate = sp_pps->ptr_pic_parameter_set_rbsp->slice_group_change_rate_minus1 + 1;
					NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag8", "PicWidthInMbs", PicWidthInMbs, "");
					NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag9", "PicHeightInMapUnits", PicHeightInMapUnits, "");
					NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag10", "PicSizeInMapUnits", PicSizeInMapUnits, "");
					NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag11", "SliceGroupChangeRate", SliceGroupChangeRate, "");

					uint8_t number_of_bits = quick_ceil_log2(PicSizeInMapUnits / SliceGroupChangeRate + 1);

					BST_FIELD_PROP_2NUMBER1(slice_group_change_cycle, number_of_bits, "used to derive the number of slice group map units in slice group 0 when slice_group_map_type is equal to 3, 4, or 5");
				}

			done:
				DECLARE_FIELDPROP_END()

			};

			struct SLICE_LAYER_WITHOUT_PARTITIONING_RBSP : public SYNTAX_BITSTREAM_MAP
			{
				SLICE_HEADER*			ptr_slice_header;

				NAL_UNIT*				ptr_nal_unit;

				SLICE_LAYER_WITHOUT_PARTITIONING_RBSP(NAL_UNIT* nalunit)
					: ptr_slice_header(NULL)
					, ptr_nal_unit(nalunit){
				}

				virtual ~SLICE_LAYER_WITHOUT_PARTITIONING_RBSP(){
					AMP_SAFEDEL2(ptr_slice_header);
				}

				int Map(AMBst in_bst)
				{
					SYNTAX_BITSTREAM_MAP::Map(in_bst);
					try
					{
						MAP_BST_BEGIN(0);
						nal_read_ref(in_bst, ptr_slice_header, SLICE_HEADER, ptr_nal_unit);
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
				NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag1", "slice_header", "");
				BST_FIELD_PROP_REF(ptr_slice_header);
				NAV_WRITE_TAG_END("Tag1");
				DECLARE_FIELDPROP_END()

			}PACKED;

			struct SLICE_DATA_PARTITION_A_LAYER_RBSP : public SYNTAX_BITSTREAM_MAP
			{
				SLICE_HEADER*			ptr_slice_header;
				uint32_t				slice_id = 0;

				NAL_UNIT*				ptr_nal_unit;

				SLICE_DATA_PARTITION_A_LAYER_RBSP(NAL_UNIT* nalunit)
					: ptr_slice_header(NULL)
					, ptr_nal_unit(nalunit) {
				}

				virtual ~SLICE_DATA_PARTITION_A_LAYER_RBSP() {
					AMP_SAFEDEL2(ptr_slice_header);
				}

				int Map(AMBst in_bst)
				{
					SYNTAX_BITSTREAM_MAP::Map(in_bst);
					try
					{
						MAP_BST_BEGIN(0);
						nal_read_ref(in_bst, ptr_slice_header, SLICE_HEADER, ptr_nal_unit);
						nal_read_ue(in_bst, slice_id, uint32_t);
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
				NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag1", "slice_header", "");
				BST_FIELD_PROP_REF(ptr_slice_header);
				NAV_WRITE_TAG_END("Tag1");
				BST_FIELD_PROP_UE(slice_id, "identifies the slice associated with the slice data partition");
				DECLARE_FIELDPROP_END()
			}PACKED;

			struct SLICE_DATA_PARTITION_B_LAYER_RBSP : public SYNTAX_BITSTREAM_MAP
			{
				uint32_t				slice_id = 0;
				uint8_t					colour_plane_id = 0;
				uint8_t					redundant_pic_cnt = 0;

				SLICE_DATA_PARTITION_A_LAYER_RBSP*
										ptr_slice_data_partition_A_layer_rbsp;
				std::shared_ptr<NAL_UNIT>
										sp_pps;

				SLICE_DATA_PARTITION_B_LAYER_RBSP(SLICE_DATA_PARTITION_A_LAYER_RBSP* pSlicePartitionA)
					: ptr_slice_data_partition_A_layer_rbsp(pSlicePartitionA) {
				}

				int Map(AMBst in_bst)
				{
					SYNTAX_BITSTREAM_MAP::Map(in_bst);
					try
					{
						MAP_BST_BEGIN(0);
						nal_read_ue(in_bst, slice_id, uint32_t);

						if (slice_id != ptr_slice_data_partition_A_layer_rbsp->slice_id)
						{
							printf("[H264] A slice data partition B is not matched with the previous slice data partition A.\n");
							throw AMException(RET_CODE_ERROR_NOTIMPL);
						}

						if (ptr_slice_data_partition_A_layer_rbsp->ptr_nal_unit == NULL ||
							ptr_slice_data_partition_A_layer_rbsp->ptr_nal_unit->ptr_ctx_video_bst == NULL)
						{
							printf("[H264] Picture Parameter Set with pic_parameter_set_id: %d is not available.\n", 
								ptr_slice_data_partition_A_layer_rbsp->ptr_slice_header->pic_parameter_set_id);
							throw AMException(RET_CODE_ERROR_NOTIMPL);
						}

						uint8_t pic_parameter_set_id = ptr_slice_data_partition_A_layer_rbsp->ptr_slice_header->pic_parameter_set_id;
						sp_pps = ptr_slice_data_partition_A_layer_rbsp->ptr_nal_unit->ptr_ctx_video_bst->GetAVCPPS(pic_parameter_set_id);
						if (!sp_pps || sp_pps->ptr_pic_parameter_set_rbsp)
						{
							printf("[H264] Picture Parameter Set with pic_parameter_set_id: %d is not available.\n", pic_parameter_set_id);
							throw AMException(RET_CODE_ERROR_NOTIMPL);
						}

						auto sps_nu = ptr_slice_data_partition_A_layer_rbsp->ptr_nal_unit->ptr_ctx_video_bst->GetAVCSPS(sp_pps->ptr_pic_parameter_set_rbsp->seq_parameter_set_id);
						if (!sps_nu || sps_nu->ptr_seq_parameter_set_rbsp)
						{
							printf("[H264] Sequence Parameter Set is not available.\n");
							throw AMException(RET_CODE_ERROR_NOTIMPL);
						}

						auto sps = sps_nu->ptr_seq_parameter_set_rbsp;

						if (sps->seq_parameter_set_data.separate_colour_plane_flag)
						{
							nal_read_u(in_bst, colour_plane_id, 2, uint8_t);
						}

						if (!sp_pps && sp_pps->ptr_pic_parameter_set_rbsp->redundant_pic_cnt_present_flag)
						{
							nal_read_ue(in_bst, redundant_pic_cnt, uint8_t);
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
				BST_FIELD_PROP_UE(slice_id, "identifies the slice associated with the slice data partition");

				auto sp_sps_nu = ptr_slice_data_partition_A_layer_rbsp->ptr_nal_unit->ptr_ctx_video_bst->GetAVCSPS(sp_pps->ptr_pic_parameter_set_rbsp->seq_parameter_set_id);
				if (sp_sps_nu && sp_sps_nu->ptr_seq_parameter_set_rbsp)
				{
					auto sps = sp_sps_nu->ptr_seq_parameter_set_rbsp;
					if (sps != NULL && sps->seq_parameter_set_data.separate_colour_plane_flag)
					{
						BST_FIELD_PROP_2NUMBER1_F(colour_plane_id, 2, "specifies the %s colour plane associated with the current slice RBSP",
							colour_plane_id == 0 ? "Y" : (colour_plane_id == 1 ? "Cb" : (colour_plane_id == 2 ? "Cr" : "Unknown")));
					};
				}


				if (!sp_pps && sp_pps->ptr_pic_parameter_set_rbsp->redundant_pic_cnt_present_flag)
					BST_FIELD_PROP_UE(redundant_pic_cnt, "shall be equal to 0 for coded slices and coded slice data partitions belonging to the primary coded picture. The redundant_pic_cnt shall be greater than 0 for coded slices and coded slice data partitions in redundant coded pictures.");
				DECLARE_FIELDPROP_END()
			};

			struct SLICE_DATA_PARTITION_C_LAYER_RBSP : public SYNTAX_BITSTREAM_MAP
			{
				uint32_t				slice_id = 0;
				uint8_t					colour_plane_id = 0;
				uint8_t					redundant_pic_cnt = 0;

				SLICE_DATA_PARTITION_A_LAYER_RBSP*
										ptr_slice_data_partition_A_layer_rbsp;
				std::shared_ptr<NAL_UNIT>
										sp_pps;

				SLICE_DATA_PARTITION_C_LAYER_RBSP(SLICE_DATA_PARTITION_A_LAYER_RBSP* pSlicePartitionA)
					: ptr_slice_data_partition_A_layer_rbsp(pSlicePartitionA) {
				}

				int Map(AMBst in_bst)
				{
					SYNTAX_BITSTREAM_MAP::Map(in_bst);
					try
					{
						MAP_BST_BEGIN(0);
						nal_read_ue(in_bst, slice_id, uint32_t);

						if (slice_id != ptr_slice_data_partition_A_layer_rbsp->slice_id)
						{
							printf("[H264] A slice data partition C is not matched with the previous slice data partition A.\n");
							throw AMException(RET_CODE_ERROR_NOTIMPL);
						}

						uint8_t pic_parameter_set_id = ptr_slice_data_partition_A_layer_rbsp->ptr_slice_header->pic_parameter_set_id;
						auto sp_pps = ptr_slice_data_partition_A_layer_rbsp->ptr_nal_unit->ptr_ctx_video_bst->GetAVCPPS(pic_parameter_set_id);
						if (!sp_pps || sp_pps->ptr_pic_parameter_set_rbsp)
						{
							printf("[H264] Picture Parameter Set with pic_parameter_set_id: %d is not available.\n", pic_parameter_set_id);
							throw AMException(RET_CODE_ERROR_NOTIMPL);
						}

						auto sps_nu = ptr_slice_data_partition_A_layer_rbsp->ptr_nal_unit->ptr_ctx_video_bst->GetAVCSPS(sp_pps->ptr_pic_parameter_set_rbsp->seq_parameter_set_id);
						if (!sps_nu || sps_nu->ptr_seq_parameter_set_rbsp == NULL)
						{
							printf("[H264] Sequence Parameter Set is not available.\n");
							throw AMException(RET_CODE_ERROR_NOTIMPL);
						}

						auto sps = sps_nu->ptr_seq_parameter_set_rbsp;

						if (!sp_pps && sp_pps->ptr_pic_parameter_set_rbsp->redundant_pic_cnt_present_flag)
						{
							nal_read_ue(in_bst, redundant_pic_cnt, uint8_t);
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
				BST_FIELD_PROP_UE(slice_id, "identifies the slice associated with the slice data partition");

				auto sps_nu = ptr_slice_data_partition_A_layer_rbsp->ptr_nal_unit->ptr_ctx_video_bst->GetAVCSPS(sp_pps->ptr_pic_parameter_set_rbsp->seq_parameter_set_id);
				if (sps_nu && sps_nu->ptr_seq_parameter_set_rbsp)
				{
					auto sps = sps_nu->ptr_seq_parameter_set_rbsp;
					if (sps != NULL && sps->seq_parameter_set_data.separate_colour_plane_flag)
					{
						BST_FIELD_PROP_2NUMBER1_F(colour_plane_id, 2, "specifies the %s colour plane associated with the current slice RBSP",
							colour_plane_id == 0 ? "Y" : (colour_plane_id == 1 ? "Cb" : (colour_plane_id == 2 ? "Cr" : "Unknown")));
					}
				}
				if (!sp_pps && sp_pps->ptr_pic_parameter_set_rbsp->redundant_pic_cnt_present_flag)
					BST_FIELD_PROP_UE(redundant_pic_cnt, "shall be equal to 0 for coded slices and coded slice data partitions belonging to the primary coded picture. The redundant_pic_cnt shall be greater than 0 for coded slices and coded slice data partitions in redundant coded pictures.");
				DECLARE_FIELDPROP_END()
			};

			NAL_UNIT_HEADER		nal_unit_header;
			uint8_t				svc_extension_flag = 0;
			uint8_t				dword_align[3] = { 0 };
			union {
				void*							ptr_view_extension = nullptr;
				NAL_UNIT_HEADER_SVC_EXTENSION*	ptr_nal_unit_header_svc_extension;
				NAL_UNIT_HEADER_MVC_EXTENSION*	ptr_nal_unit_header_mvc_extension;
			}PACKED;
			union
			{
				void*							ptr_rbsp = nullptr;
				ACCESS_UNIT_DELIMITER_RBSP*		ptr_access_unit_delimiter_rbsp;
				SEQ_PARAMETER_SET_RBSP*			ptr_seq_parameter_set_rbsp;
				SEQ_PARAMETER_SET_EXTENSION_RBSP*
												ptr_seq_parameter_set_extension_rbsp;
				SEI_RBSP*						ptr_sei_rbsp;
				SUBSET_SEQ_PARAMETER_SET_RBSP*	ptr_subset_seq_parameter_set_rbsp;
				PIC_PARAMETER_SET_RBSP*			ptr_pic_parameter_set_rbsp;
				SLICE_LAYER_WITHOUT_PARTITIONING_RBSP*
												ptr_slice_layer_without_partitioning_rbsp;
				SLICE_DATA_PARTITION_A_LAYER_RBSP*
												ptr_slice_data_partition_a_layer_rbsp;
				SLICE_DATA_PARTITION_B_LAYER_RBSP*
												ptr_slice_data_partition_b_layer_rbsp;
				SLICE_DATA_PARTITION_C_LAYER_RBSP*
												ptr_slice_data_partition_c_layer_rbsp;
			}PACKED;
			std::vector<uint8_t>				unparsed_rbsp;
			VideoBitstreamCtx*					ptr_ctx_video_bst;

			NAL_UNIT() : ptr_rbsp(NULL), ptr_ctx_video_bst(NULL) {}
			virtual ~NAL_UNIT() {
				if (nal_unit_header.nal_unit_type == PREFIX_NUT || nal_unit_header.nal_unit_type == SL_EXT || nal_unit_header.nal_unit_type == SL_EXT_DVIEW)
				{
					if (svc_extension_flag) {
						UNMAP_STRUCT_POINTER5(ptr_nal_unit_header_svc_extension);
					}
					else {
						UNMAP_STRUCT_POINTER5(ptr_nal_unit_header_mvc_extension);
					}
				}
				switch (nal_unit_header.nal_unit_type)
				{
				case CS_NON_IDR_PIC:
					UNMAP_STRUCT_POINTER5(ptr_slice_layer_without_partitioning_rbsp);
					break;
				case CSD_PARTITION_A:
					UNMAP_STRUCT_POINTER5(ptr_slice_data_partition_a_layer_rbsp);
					break;
				case CSD_PARTITION_B:
					UNMAP_STRUCT_POINTER5(ptr_slice_data_partition_b_layer_rbsp);
					break;
				case CSD_PARTITION_C:
					UNMAP_STRUCT_POINTER5(ptr_slice_data_partition_c_layer_rbsp);
					break;
				case CS_IDR_PIC:
					UNMAP_STRUCT_POINTER5(ptr_slice_layer_without_partitioning_rbsp);
					break;
				case SEI_NUT:
					UNMAP_STRUCT_POINTER5(ptr_sei_rbsp);
					break;
				case SPS_NUT:
					UNMAP_STRUCT_POINTER5(ptr_seq_parameter_set_rbsp);
					break;
				case PPS_NUT:
					UNMAP_STRUCT_POINTER5(ptr_pic_parameter_set_rbsp);
					break;
				case AUD_NUT:
					UNMAP_STRUCT_POINTER5(ptr_access_unit_delimiter_rbsp);
					break;
				case EOS_NUT:
					break;
				case EOB_NUT:
					break;
				case FD_NUT:
					break;
				case SPS_EXT_NUT:
					UNMAP_STRUCT_POINTER5(ptr_seq_parameter_set_extension_rbsp);
					break;
				case PREFIX_NUT:
					break;
				case SUB_SPS_NUT:
					UNMAP_STRUCT_POINTER5(ptr_subset_seq_parameter_set_rbsp);
					break;
				case SL_WO_PARTITION:
					break;
				case SL_EXT:
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

				if (ptr_ctx_video_bst != NULL &&
					ptr_ctx_video_bst->nal_unit_type_filters.size() > 0)
				{
					if (std::find(ptr_ctx_video_bst->nal_unit_type_filters.begin(), ptr_ctx_video_bst->nal_unit_type_filters.end(), nal_unit_header.nal_unit_type) ==
						ptr_ctx_video_bst->nal_unit_type_filters.end())
						return RET_CODE_NOTHING_TODO;
				}

				if (nal_unit_header.nal_unit_type == PREFIX_NUT ||
					nal_unit_header.nal_unit_type == SL_EXT ||
					nal_unit_header.nal_unit_type == SL_EXT_DVIEW)
				{
					nal_read_u(bst, svc_extension_flag, 1, uint8_t);
					if (svc_extension_flag) {
						nal_read_ref(bst, ptr_nal_unit_header_svc_extension, NAL_UNIT_HEADER_SVC_EXTENSION);
					}
					else {
						nal_read_ref(bst, ptr_nal_unit_header_mvc_extension, NAL_UNIT_HEADER_MVC_EXTENSION);
					}
				}

				if (AMP_FAILED(iRet = AMBst_SetRBSPType(bst, BST_RBSP_NAL_UNIT)))
					return iRet;

				try
				{
					MAP_BST_BEGIN(1);
					switch (nal_unit_header.nal_unit_type)
					{
					case CS_NON_IDR_PIC:
						nal_read_ref(bst, ptr_slice_layer_without_partitioning_rbsp, SLICE_LAYER_WITHOUT_PARTITIONING_RBSP, this);
						break;
					case CSD_PARTITION_A:
						nal_read_ref(bst, ptr_slice_data_partition_a_layer_rbsp, SLICE_DATA_PARTITION_A_LAYER_RBSP, this);
						break;
					case CSD_PARTITION_B:
						if (ptr_ctx_video_bst->sp_prev_nal_unit != NULL &&
							ptr_ctx_video_bst->sp_prev_nal_unit->nal_unit_header.nal_unit_type == CSD_PARTITION_A) {
							nal_read_ref(bst, ptr_slice_data_partition_b_layer_rbsp, SLICE_DATA_PARTITION_B_LAYER_RBSP, ptr_ctx_video_bst->sp_prev_nal_unit->ptr_slice_data_partition_a_layer_rbsp);
						}
						break;
					case CSD_PARTITION_C:
						if (ptr_ctx_video_bst->sp_prev_nal_unit != NULL)
						{
							if (ptr_ctx_video_bst->sp_prev_nal_unit->nal_unit_header.nal_unit_type == CSD_PARTITION_A) {
								nal_read_ref(bst, ptr_slice_data_partition_c_layer_rbsp, SLICE_DATA_PARTITION_C_LAYER_RBSP, ptr_ctx_video_bst->sp_prev_nal_unit->ptr_slice_data_partition_a_layer_rbsp);
							}
							else if (ptr_ctx_video_bst->sp_prev_nal_unit->nal_unit_header.nal_unit_type == CSD_PARTITION_B &&
								ptr_ctx_video_bst->sp_prev_nal_unit->ptr_slice_data_partition_b_layer_rbsp != NULL) {
								nal_read_ref(bst, ptr_slice_data_partition_c_layer_rbsp, SLICE_DATA_PARTITION_C_LAYER_RBSP,
									ptr_ctx_video_bst->sp_prev_nal_unit->ptr_slice_data_partition_b_layer_rbsp->ptr_slice_data_partition_A_layer_rbsp);
							}
						}
						break;
					case CS_IDR_PIC:
						nal_read_ref(bst, ptr_slice_layer_without_partitioning_rbsp, SLICE_LAYER_WITHOUT_PARTITIONING_RBSP, this);
						break;
					case SEI_NUT:
						nal_read_ref(bst, ptr_sei_rbsp, SEI_RBSP, nal_unit_header.nal_unit_type, ptr_ctx_video_bst);
						break;
					case SPS_NUT:
						nal_read_ref(bst, ptr_seq_parameter_set_rbsp, SEQ_PARAMETER_SET_RBSP, ptr_ctx_video_bst);
						break;
					case PPS_NUT:
						nal_read_ref(bst, ptr_pic_parameter_set_rbsp, PIC_PARAMETER_SET_RBSP, ptr_ctx_video_bst);
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
					case SPS_EXT_NUT:
						nal_read_ref(bst, ptr_seq_parameter_set_extension_rbsp, SEQ_PARAMETER_SET_EXTENSION_RBSP);
						break;
					case PREFIX_NUT:
						break;
					case SUB_SPS_NUT:
						nal_read_ref(bst, ptr_subset_seq_parameter_set_rbsp, SUBSET_SEQ_PARAMETER_SET_RBSP);
						break;
					case SL_WO_PARTITION:
						break;
					case SL_EXT:
						break;
					}

					if (ptr_rbsp == NULL)
					{
						uint8_t u8Val = 0;
						for (;;)
						{
							nal_read_u(bst, u8Val, 8, uint8_t);
							unparsed_rbsp.push_back(u8Val);
						}
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
			NAV_WRITE_TAG_BEGIN2_1("NAL_UNIT", avc_nal_unit_type_descs[nal_unit_header.nal_unit_type]);
			BST_FIELD_PROP_OBJECT(nal_unit_header);
			if (nal_unit_header.nal_unit_type == PREFIX_NUT ||
				nal_unit_header.nal_unit_type == SL_EXT ||
				nal_unit_header.nal_unit_type == SL_EXT_DVIEW)
			{
				BST_FIELD_PROP_BOOL(svc_extension_flag, "nal_unit_header_svc_extension followed", "nal_unit_header_mvc_extension followed");
				if (svc_extension_flag) {
					BST_FIELD_PROP_REF2_1(ptr_nal_unit_header_svc_extension, "nal_unit_header_svc_extension", "");
				}
				else {
					BST_FIELD_PROP_REF2_1(ptr_nal_unit_header_mvc_extension, "nal_unit_header_mvc_extension", "");
				}
			}
			switch (nal_unit_header.nal_unit_type)
			{
			case CS_NON_IDR_PIC:
				BST_FIELD_PROP_REF2_1(ptr_slice_layer_without_partitioning_rbsp, "slice_layer_without_partitioning_rbsp", "");
				break;
			case CSD_PARTITION_A:
				BST_FIELD_PROP_REF2_1(ptr_slice_data_partition_a_layer_rbsp, "slice_data_partition_a_layer_rbsp", "");
				break;
			case CSD_PARTITION_B:
				BST_FIELD_PROP_REF2_1(ptr_slice_data_partition_b_layer_rbsp, "slice_data_partition_b_layer_rbsp", "");
				break;
			case CSD_PARTITION_C:
				BST_FIELD_PROP_REF2_1(ptr_slice_data_partition_c_layer_rbsp, "slice_data_partition_c_layer_rbsp", "");
				break;
			case CS_IDR_PIC:
				BST_FIELD_PROP_REF2_1(ptr_slice_layer_without_partitioning_rbsp, "slice_layer_without_partitioning_rbsp", "");
				break;
			case SEI_NUT:
				NAV_FIELD_PROP_REF(ptr_sei_rbsp);
				break;
			case SPS_NUT:
				BST_FIELD_PROP_REF2_1(ptr_seq_parameter_set_rbsp, "seq_parameter_set_rbsp", "");
				break;
			case PPS_NUT:
				BST_FIELD_PROP_REF2_1(ptr_pic_parameter_set_rbsp, "pic_parameter_set_rbsp", "");
				break;
			case AUD_NUT:
				BST_FIELD_PROP_REF2_1(ptr_access_unit_delimiter_rbsp, "access_unit_delimiter_rbsp", "");
				break;
			case EOS_NUT:
				break;
			case EOB_NUT:
				break;
			case FD_NUT:
				break;
			case SPS_EXT_NUT:
				BST_FIELD_PROP_REF2_1(ptr_seq_parameter_set_extension_rbsp, "seq_parameter_set_extension_rbsp", "");
				break;
			case PREFIX_NUT:
				break;
			case SUB_SPS_NUT:
				BST_FIELD_PROP_REF2_1(ptr_subset_seq_parameter_set_rbsp, "subset_seq_parameter_set_rbsp", "");
				break;
			case SL_WO_PARTITION:
				break;
			case SL_EXT:
				break;
			}
			if (ptr_rbsp == NULL)
			{
				BST_FIELD_PROP_FIXSIZE_BINSTR("rbsp_bytes", 8 * (unsigned long)unparsed_rbsp.size(), &unparsed_rbsp[0], (unsigned long)unparsed_rbsp.size(), "");
			}
			NAV_WRITE_TAG_END2("NAL_UNIT");
			DECLARE_FIELDPROP_END()
		};

		struct BYTE_STREAM_NAL_UNIT : public SYNTAX_BITSTREAM_MAP
		{
			std::vector<uint8_t>		leading_zero_8bits;
			uint8_t						has_zero_byte = 0;
			uint8_t						zero_byte = 0;
			uint32_t					start_code_prefix_one_3bytes = 0;
			std::shared_ptr<NAL_UNIT>					
										nal_unit;
			std::vector<uint8_t>		trailing_zero_8bits;
			VideoBitstreamCtx*			ctx_video_bst;
			uint32_t					NAL_Length_Delimiter_Size;
			uint32_t					NAL_Length = 0;

			BYTE_STREAM_NAL_UNIT(VideoBitstreamCtx* ctx = NULL, uint32_t cbNALLengthDelimiterSize = 0) 
				: ctx_video_bst(ctx), NAL_Length_Delimiter_Size(cbNALLengthDelimiterSize) {
			}

			/*!	@brief Map from a byte stream NAL unit
				@param in_bst the input bitstream which should only include only one byte stream NAL unit
				@remarks 
				byte stream NAL unit = start_code + NALU + ... start_code + NALU 
				NALU = NALU header + EBSP(encapsulate byte sequence payload) 
				EBSP = RBSP part 1 + 0x03 + RBSP part 2 + 0x03 + ....RBSP part n
				RBSP = SODB + RBSP stop bit + '0' bits
				When feeding the data to bitstream, please should not the data in more than one NAL unit
				*/
			int Map(AMBst in_bst)
			{
				int iRet = RET_CODE_SUCCESS;
				uint8_t leading_zero_byte = 0;

				SYNTAX_BITSTREAM_MAP::Map(in_bst);

				try
				{
					if (AMP_FAILED(iRet = AMBst_Realign(in_bst)))
					{
						printf("[H264] Error happened or data is insufficient in bitstream {retcode: %d}.\n", iRet);
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
						nal_read_b(in_bst, NAL_Length, NAL_Length_Delimiter_Size << 3, uint32_t);
					}

					nal_unit = ctx_video_bst->CreateAVCNU();

					if (AMP_FAILED(iRet = nal_unit->Map(in_bst)))
					{
						printf("[H264] Failed to map nal_unit {retcode: %d}.\n", iRet);
						// No more data in the current bit-stream, the current mapping is terminated, and deem it as "success"
						if (iRet == RET_CODE_NO_MORE_DATA) goto done;
					}

					map_status.number_of_fields++;
				}
				catch (AMException e)
				{
					return e.RetCode();
				}

				if (NAL_Length_Delimiter_Size == 0 && (ctx_video_bst == NULL || !ctx_video_bst->in_scanning) && AMBst_more_data(in_bst))
				{
					// Process trailing zero bytes
					try
					{
						while ((uint32_t)AMBst_PeekBits(in_bst, 24) != 0x000001 && (uint32_t)AMBst_PeekBits(in_bst, 32) != 0x00000001)
						{
							nal_read_f(in_bst, leading_zero_byte, 8, uint8_t);
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
				NAV_WRITE_TAG_BEGIN2_1("byte_stream_nal_unit", (map_status.status == 0 || map_status.number_of_fields >= (int)(leading_zero_8bits.size() + (has_zero_byte ? 1 : 0) + 4)) ? avc_nal_unit_type_descs[nal_unit->nal_unit_header.nal_unit_type] : "");
				if (leading_zero_8bits.size() > 0)
				{
					NAV_FIELD_PROP_FIXSIZE_BINSTR("leading_zero_8bits", 8 * (unsigned long)leading_zero_8bits.size(), &leading_zero_8bits[0], (unsigned long)leading_zero_8bits.size(), "equal to 0x00")
				}
				if (has_zero_byte)
				{
					NAV_FIELD_PROP_2NUMBER("zero_byte", 8, zero_byte, "equal to 0x00")
				}
				NAV_FIELD_PROP_2NUMBER("start_code_prefix_one_3bytes", 24, start_code_prefix_one_3bytes, "equal to 0x000001")
				NAV_FIELD_PROP_REF(nal_unit)
				if (trailing_zero_8bits.size() > 0)
				{
					NAV_FIELD_PROP_FIXSIZE_BINSTR("trailing_zero_8bits", 8 * (unsigned long)trailing_zero_8bits.size(), &trailing_zero_8bits[0], (unsigned long)trailing_zero_8bits.size(), "equal to 0x00")
				}
				NAV_WRITE_TAG_END2("byte_stream_nal_unit");
			}
			else
			{
				NAV_FIELD_PROP_2NUMBER1(NAL_Length, NAL_Length_Delimiter_Size, "Indicates the length in bytes of the following NAL unit. The length field can be configured to be of 1, 2, or 4 bytes.");
				NAV_FIELD_PROP_REF(nal_unit);
			}
			DECLARE_FIELDPROP_END()

		};

		struct CVideoBitstream : public SYNTAX_BITSTREAM_MAP
		{
			std::vector<std::shared_ptr<BYTE_STREAM_NAL_UNIT>>	
													bst_nal_units;
			VideoBitstreamCtx						ctx_video_bst;

			// used by scanning process
			std::vector<int8_t>						nal_unit_types;
			uint32_t								nal_length_delimiter_size = 0;

			CVideoBitstream() {}
			CVideoBitstream(std::initializer_list<uint8_t> unit_type_filters) {
				ctx_video_bst.nal_unit_type_filters = unit_type_filters;
			}

			virtual ~CVideoBitstream(){
			}

			bool ExistBDMVKeyFrame()
			{
				bool bSPSExist = false, bIDRExist = false;
				for (int i = 0; i < (int)nal_unit_types.size(); i++)
				{
					if (nal_unit_types[i] == SPS_NUT)
						bSPSExist = true;

					if (nal_unit_types[i] == CS_IDR_PIC)
						bIDRExist = true;
				}
				return bSPSExist && bIDRExist;
			}

			int Map(unsigned char* pBuf, unsigned long cbSize)
			{
				int iRet = RET_CODE_ERROR;
				unsigned char* pStartBuf = pBuf;
				unsigned long  cbBufSize = cbSize;
				bool bNalUnitZeroByte = false;
				AMBst bst = NULL;

				while (cbSize >= 3)
				{
					// Find the start_code_prefix_one_3bytes
					while (cbSize >= 3 && !(pBuf[0] == 0 && pBuf[1] == 0 && pBuf[2] == 1))
					{
						cbSize--;
						pBuf++;
					}

					if (cbSize >= 3)	// the start code is found
					{
						// zero_byte exists before start_code_prefix_one_3byte
						if (pBuf > pStartBuf && *(pBuf - 1) == 0)
						{
							cbSize++;
							pBuf--;
							bNalUnitZeroByte = true;
						}
						else
							bNalUnitZeroByte = false;
					}

					uint8_t nalUnitHeaderBytes = 1;
					uint8_t cbRequired = (bNalUnitZeroByte ? 4 : 3) + nalUnitHeaderBytes;
					// no basic NAL header exists
					if (cbSize < cbRequired)
					{
						//AMP_Warning(_T("[H264] No enough data, at least one byte NAL_header should exist {cbSize: %d, cbRequried: %d}.\n"), cbSize, cbRequired);
						break;
					}
					
					int8_t nal_unit_type = pBuf[bNalUnitZeroByte ? 4 : 3] & 0x1F;
					nal_unit_types.push_back(nal_unit_type);
					if (nal_unit_type == 14 || nal_unit_type == 20 || nal_unit_type == 21) 
					{
						cbRequired++;
						if (cbSize < cbRequired)
						{
							printf("[H264] No enough data for NAL_header for SVC/AVC_3D extension {left bytes: %lu, required bytes: %u}.\n", cbSize, cbRequired);
							break;
						}

						uint8_t svc_extension_flag = 0, avc_3d_extension_flag = 0;
						if (nal_unit_type != 21)
							svc_extension_flag = (pBuf[(bNalUnitZeroByte ? 5 : 4)] >> 7) & 0x01;
						else
							avc_3d_extension_flag = (pBuf[(bNalUnitZeroByte ? 5 : 4)] >> 7) & 0x01;

						if (svc_extension_flag) {
							//nal_unit_header_svc_extension() /* specified in Annex G */
							nalUnitHeaderBytes += 3;
							cbRequired += 2;
						}
						else if (avc_3d_extension_flag) {
							//nal_unit_header_3davc_extension() /* specified in Annex J */
							nalUnitHeaderBytes += 2;
							cbRequired++;
						}
						else {
							//nal_unit_header_mvc_extension() /* specified in Annex H */
							nalUnitHeaderBytes += 3;
							cbRequired += 2;
						}

						if (cbSize < cbRequired)
						{
							printf("[H264] No enough data for NAL_header for SVC/AVC_3D extension {left bytes: %lu, required bytes: %u}.\n", cbSize, cbRequired);
							break;
						}
					}

					// filter nal_units by nal_unit_type
					if (ctx_video_bst.nal_unit_type_filters.size() > 0)
					{
						if (std::find(ctx_video_bst.nal_unit_type_filters.begin(), ctx_video_bst.nal_unit_type_filters.end(), nal_unit_type) ==
							ctx_video_bst.nal_unit_type_filters.end())
						{
							uint8_t skip_bytes = (bNalUnitZeroByte ? 4 : 3) + nalUnitHeaderBytes;
							cbSize -= skip_bytes;
							pBuf += skip_bytes;
							continue;
						}
					}

					// Find the next start code
					uint8_t* pNextNALBuf = NextNALStart(pBuf + cbRequired, cbSize - cbRequired);

					if (bst != NULL)
						AMBst_Destroy(bst);

					if (pNextNALBuf == nullptr)
						bst = AMBst_CreateFromBuffer(pStartBuf, cbBufSize);
					else
						bst = AMBst_CreateFromBuffer(pStartBuf, (int)(pNextNALBuf - pStartBuf));

					AMP_NEWT1(ptr_bst_nal_unit, BYTE_STREAM_NAL_UNIT, &ctx_video_bst);
					std::shared_ptr<BYTE_STREAM_NAL_UNIT> sp_bst_nal_unit(ptr_bst_nal_unit);
					AMBst_Seek(bst, (cbBufSize - cbSize) * 8);	// in order to keep the bit position information correctly
					if (AMP_FAILED(iRet = sp_bst_nal_unit->Map(bst)))
						printf("[H264] Failed to map byte stream NAL unit {retcode: %d}.\n", iRet);

					bst_nal_units.push_back(sp_bst_nal_unit);

					if (iRet == RET_CODE_NO_MORE_DATA && pNextNALBuf == nullptr)
					{
						AMBst_Destroy(bst);
						iRet = RET_CODE_SUCCESS;
						break;
					}
					else if (pNextNALBuf == nullptr)
					{
						// Some data is unparsed, but quit the loop anyway
						break;
					}

					cbSize = cbBufSize - (int)(pNextNALBuf - pStartBuf);
					pBuf = pNextNALBuf;
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
				AMBst bst = nullptr;// AMBst_CreateFromBuffer(pStartBuf, cbBufSize);

				while (cbSize >= cbNALLengthDelimiterSize)
				{
					uint32_t nal_length = 0;
					for (uint32_t i = 0; i < cbNALLengthDelimiterSize; i++)
						nal_length = (nal_length << 8) | pBuf[i];

					if (cbSize < nal_length + cbNALLengthDelimiterSize)
					{
						printf("[H264] The actual NAL bytes(%" PRIu32 ") are less than the specified NAL length(%" PRIu32 ").\n", (uint32_t)(cbSize - cbNALLengthDelimiterSize), nal_length);
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
							cbSize -= cbNALLengthDelimiterSize + nal_length;
							pBuf += (ptrdiff_t)cbNALLengthDelimiterSize + nal_length;
							continue;
						}
					}

					AMP_NEWT1(ptr_bst_nal_unit, BYTE_STREAM_NAL_UNIT, &ctx_video_bst, cbNALLengthDelimiterSize);
					std::shared_ptr<BYTE_STREAM_NAL_UNIT> sp_bst_nal_unit(ptr_bst_nal_unit);
					if (bst != nullptr)
						AMBst_Destroy(bst);

					bst = AMBst_CreateFromBuffer(pStartBuf, cbBufSize - cbSize + cbNALLengthDelimiterSize + nal_length);
					AMBst_Seek(bst, (cbBufSize - cbSize) * 8);
					if (AMP_FAILED(iRet = ptr_bst_nal_unit->Map(bst)))
					{
						if (iRet != RET_CODE_NO_MORE_DATA)
							printf("[H264] Failed to map byte stream NAL unit {retcode: %d}.\n", iRet);
					}

					bst_nal_units.push_back(sp_bst_nal_unit);
					ptr_bst_nal_unit = NULL;

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
					AMP_NEWT1(ptr_bst_nal_unit, BYTE_STREAM_NAL_UNIT);
					std::shared_ptr<BYTE_STREAM_NAL_UNIT> sp_bst_nal_unit(ptr_bst_nal_unit);
					if (AMP_FAILED(iRet = sp_bst_nal_unit->Map(in_bst)))
						printf("[H264] Failed to map byte stream NAL unit {retcode: %d}.\n", iRet);

					bst_nal_units.push_back(sp_bst_nal_unit);

					if (iRet == RET_CODE_NO_MORE_DATA)
					{
						iRet = RET_CODE_SUCCESS;
						break;
					}
				} while (AMP_SUCCEEDED(iRet));

				MAP_BST_END();
				return iRet;
			}

			int MapES(unsigned char* pBuf, unsigned long cbSize)
			{
				int iRet = RET_CODE_SUCCESS;
				unsigned char* pStartBuf = NULL;
				unsigned char* pEndBuf = NULL;
				AMBst bstBytestreamNALunit = NULL_BST;

				if (cbSize > INT32_MAX)
					return RET_CODE_INVALID_PARAMETER;

				MAP_BST_BEGIN(0);

				unsigned char* pInBuf = pBuf;
				pStartBuf = pEndBuf = pBuf;
				long cbLeft = (long)cbSize;

				// Try to locate the start position of the first NAL unit
				while ((cbLeft >= 3 && !(pBuf[0] == 0 && pBuf[1] == 0 && pBuf[2] == 1)) || (cbLeft > 0 && cbLeft < 3))
				{
					cbLeft--;
					pBuf++;
				}

				// If finding the first start code prefix, skip it.
				if (cbLeft >= 3)
				{
					cbLeft -= 3;
					pBuf += 3;
				}

				while (cbLeft >= 0)
				{
					// Find the next start code prefix
					if (cbLeft == 0 ||	// End of Stream
						(cbLeft >= 3 && (pBuf[0] == 0 && pBuf[1] == 0 && pBuf[2] == 1)) ||
						(cbLeft >= 4 && (pBuf[0] == 0 && pBuf[1] == 0 && pBuf[2] == 0 && pBuf[3] == 1)))
					{
						pEndBuf = pBuf;
						BYTE_STREAM_NAL_UNIT* ptr_bst_nal_unit = NULL;

						AMP_NEWT(ptr_bst_nal_unit, BYTE_STREAM_NAL_UNIT, &ctx_video_bst);
						std::shared_ptr<BYTE_STREAM_NAL_UNIT> sp_bst_nal_unit(ptr_bst_nal_unit);

						bstBytestreamNALunit = AMBst_CreateFromBuffer(pStartBuf, (int)(pEndBuf - pStartBuf));

						if (AMP_FAILED(iRet = sp_bst_nal_unit->Map(bstBytestreamNALunit)))
							printf("[H264] Failed to map byte stream NAL unit {retcode: %d}.\n", iRet);

						AMBst_Destroy(bstBytestreamNALunit);

						sp_bst_nal_unit->bit_pos = (int)((pStartBuf - pInBuf) * 8);
						sp_bst_nal_unit->bit_end_pos = (int)((pEndBuf - pInBuf) * 8);

						bst_nal_units.push_back(sp_bst_nal_unit);

						if (sp_bst_nal_unit->nal_unit->nal_unit_header.nal_unit_type == PPS_NUT &&
							sp_bst_nal_unit->nal_unit->ptr_pic_parameter_set_rbsp != NULL)
						{
							int sps_pps_id = (sp_bst_nal_unit->nal_unit->ptr_pic_parameter_set_rbsp->seq_parameter_set_id << 8) |
								(sp_bst_nal_unit->nal_unit->ptr_pic_parameter_set_rbsp->pic_parameter_set_id);

							ctx_video_bst.sp_h264_ppses[sps_pps_id] = sp_bst_nal_unit->nal_unit;
						}

						ctx_video_bst.sp_prev_nal_unit = sp_bst_nal_unit->nal_unit;

						if (cbLeft == 0)
							break;

						pStartBuf = pBuf;

						// Skip the start code prefix
						if (cbLeft >= 3 && (pBuf[0] == 0 && pBuf[1] == 0 && pBuf[2] == 1))
						{
							cbLeft -= 3;
							pBuf += 3;
						}
						else if(cbLeft >= 4 && (pBuf[0] == 0 && pBuf[1] == 0 && pBuf[2] == 0 && pBuf[3] == 1))
						{
							cbLeft -= 4;
							pBuf += 4;
						}
					}
					else
					{
						cbLeft--;
						pBuf++;
					}
				}

				MAP_BST_END();
				return RET_CODE_SUCCESS;
			}

			int Unmap(AMBst out_bst)
			{
				return RET_CODE_ERROR_NOTIMPL;
			}

			DECLARE_FIELDPROP_BEGIN()
			NAV_WRITE_TAG_BEGIN_1("VideoBitstream", "H264 video bitstream", 1);
			long long start_bit_offset = bit_offset ? *bit_offset : 0;
			for (i = 0; i<(int)bst_nal_units.size(); i++)
			{
				if (bit_offset != NULL)
					*bit_offset = start_bit_offset + bst_nal_units[i]->bit_pos;
				NAV_FIELD_PROP_REF(bst_nal_units[i]);
			}
			NAV_WRITE_TAG_END("VideoBitstream");
			DECLARE_FIELDPROP_END()
		};

	}	//namespace H264Video

}	// namespace BST

#ifdef _WIN32
#pragma pack(pop)
#pragma warning(pop)
#endif
#undef PACKED

#endif
