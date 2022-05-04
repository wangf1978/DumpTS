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
#include "h264_video.h"
#include "AMException.h"
#include "AMRingBuffer.h"

#include "NAL.h"
#include "DataUtil.h"
#include "nal_com.h"

const char* avc_nal_unit_type_names[32] = {
	"non-VCL::Unspecified",
	"VCL::non-IDR",
	"VCL::A",
	"VCL::B",
	"VCL::C",
	"VCL::IDR",
	"non-VCL::SEI",
	"non-VCL::SPS",
	"non-VCL::PPS",
	"non-VCL::AUD",
	"non-VCL::EOS",
	"non-VCL::EOB",
	"non-VCL::FD",
	"non-VCL::SPS_EXT",
	"non-VCL::PREFIX_NU",
	"non-VCL::SUBSET_SPS",
	"non-VCL::DPS",
	"non-VCL::reserved",
	"non-VCL::reserved",
	"non-VCL::SL_AUX",
	"non-VCL::SL_EXT",
	"non-VCL::SL_EXT_DVC",
	"non-VCL::reserved",
	"non-VCL::reserved",
	"non-VCL::Unspecified",
	"non-VCL::Unspecified",
	"non-VCL::Unspecified",
	"non-VCL::Unspecified",
	"non-VCL::Unspecified",
	"non-VCL::Unspecified",
	"non-VCL::Unspecified",
	"non-VCL::Unspecified"
};

const char* avc_nal_unit_type_short_names[32] = {
	"Unspecified",
	"non-IDR",
	"A",
	"B",
	"C",
	"IDR",
	"SEI",
	"SPS",
	"PPS",
	"AUD",
	"EOS",
	"EOB",
	"FD",
	"SPS_EXT",
	"PREFIX_NU",
	"SUBSET_SPS",
	"DPS",
	"reserved",
	"reserved",
	"SL_AUX",
	"SL_EXT",
	"SL_EXT_DVC",
	"reserved",
	"reserved",
	"Unspecified",
	"Unspecified",
	"Unspecified",
	"Unspecified",
	"Unspecified",
	"Unspecified",
	"Unspecified",
	"Unspecified"
};

const char* avc_nal_unit_type_descs[32] = {
	"Unspecified",
	"Coded slice of a non-IDR picture",
	"Coded slice data partition A",
	"Coded slice data partition B",
	"Coded slice data partition C",
	"Coded slice of an IDR picture",
	"Supplemental enhancement information(SEI)",
	"Sequence parameter set",
	"Picture parameter set",
	"Access unit delimiter",
	"End of sequence",
	"End of stream",
	"Filler data",
	"Sequence parameter set extension",
	"Prefix NAL unit",
	"Subset sequence parameter set",
	"reserved",
	"reserved",
	"reserved",
	"Coded slice of an auxiliary coded picture without partitioning",
	"Coded slice extension",
	"Coded slice extension for depth view components /*specified in Annex I */",
	"reserved",
	"reserved",
	"Unspecified",
	"Unspecified",
	"Unspecified",
	"Unspecified",
	"Unspecified",
	"Unspecified",
	"Unspecified",
	"Unspecified"
};

const char* chroma_format_idc_names[4] = {
	"Monochrome", "4:2:0", "4:2:2", "4:4:4"
};

const char* h264_slice_type_names[10] = {
	/*0*/"P(P slice)",
	/*1*/"B(B slice)",
	/*2*/"I(I slice)",
	/*3*/"SP(SP slice)",
	/*4*/"SI(SI slice)",
	/*5*/"P(P slice)",
	/*6*/"B(B slice)",
	/*7*/"I(I slice)",
	/*8*/"SP(SP slice)",
	/*9*/"SI(SI slice)",
};

const char* h264_primary_pic_type_names[8] = {
	"I(I slice)",
	"P(P slice), I(I slice)",
	"P(P slice), B(B slice), I(I slice)",
	"SI(SI slice)",
	"SP(SI slice), SI(SI slice)",
	"I(I slice), SI(SI slice)"
	"P(P slice), I(I slice), SP(SP slice), SI(SI slice)",
	"P(P slice), B(B slice), I(I slice), SP(SP slice), SI(SI slice)",
};

const char* get_h264_profile_name(int profile)
{
	switch (profile)
	{
		case BST::H264Video::AVC_PROFILE_BASELINE : return "Baseline Profile";
		case BST::H264Video::AVC_PROFILE_CONSTRAINED_BASELINE : return "Constrained Baseline Profile";
		case BST::H264Video::AVC_PROFILE_MAIN : return "Main Profile";
		case BST::H264Video::AVC_PROFILE_EXTENDED : return "Extended Profile";
		case BST::H264Video::AVC_PROFILE_HIGH : return "High Profile";
		case BST::H264Video::AVC_PROFILE_PROGRESSIVE_HIGH : return "Progressive High Profile";
		case BST::H264Video::AVC_PROFILE_CONSTRAINED_HIGH : return "Constrained High Profile";
		case BST::H264Video::AVC_PROFILE_HIGH_10 : return "High 10 Profile";
		case BST::H264Video::AVC_PROFILE_HIGH_10_INTRA : return "High 10 Intra Profile";
		case BST::H264Video::AVC_PROFILE_PROGRESSIVE_HIGH_10: return "Progressive High 10 Profile";
		case BST::H264Video::AVC_PROFILE_HIGH_422 : return "High 422 Profile";
		case BST::H264Video::AVC_PROFILE_HIGH_422_INTRA : return "High 422 Intra Profile";
		case BST::H264Video::AVC_PROFILE_HIGH_444_PREDICTIVE : return "High 444 Predictive Profile";
		case BST::H264Video::AVC_PROFILE_HIGH_444_INTRA : return "High 444 Intra Profile";
		case BST::H264Video::AVC_PROFILE_CAVLC_444_INTRA_PROFIILE: return "CAVLC 444 Intra Profile";
		case BST::H264Video::AVC_PROFILE_MULTIVIEW_HIGH : return "MultiView High Profile";
		case BST::H264Video::AVC_PROFILE_STEREO_HIGH : return "Stereo High Profile";
		case BST::H264Video::AVC_PROFILE_SCALABLE_BASELINE : return "Scalable Baseline Profile";
		case BST::H264Video::AVC_PROFILE_SCALABLE_CONSTRAINED_BASELINE : return "Scalable Constrained Baseline Profile";
		case BST::H264Video::AVC_PROFILE_SCALABLE_HIGH : return "Scalable High Profile";
		case BST::H264Video::AVC_PROFILE_SCALABLE_CONSTRAINED_HIGH : return "Scalable Constrained High Profile";
		case BST::H264Video::AVC_PROFILE_SCALABLE_HIGH_INTRA : return "Scalable High Intra Profile";
		case BST::H264Video::AVC_PROFILE_MULTIVIEW_DEPTH_HIGH : return "MultiView Depth High Profile";
	}

	return "Unknown Profile";
}

const char* get_h264_level_name(int level)
{
	switch (level)
	{
		case BST::H264Video::AVC_LEVEL_1: return "1";
		case BST::H264Video::AVC_LEVEL_1b: return "1b";
		case BST::H264Video::AVC_LEVEL_1_1: return "1.1";
		case BST::H264Video::AVC_LEVEL_1_2: return "1.2";
		case BST::H264Video::AVC_LEVEL_1_3: return "1.3";
		case BST::H264Video::AVC_LEVEL_2: return "2";
		case BST::H264Video::AVC_LEVEL_2_1: return "2.1";
		case BST::H264Video::AVC_LEVEL_2_2: return "2.2";
		case BST::H264Video::AVC_LEVEL_3: return "3";
		case BST::H264Video::AVC_LEVEL_3_1: return "3.1";
		case BST::H264Video::AVC_LEVEL_3_2: return "3.2";
		case BST::H264Video::AVC_LEVEL_4: return "4";
		case BST::H264Video::AVC_LEVEL_4_1: return "4.1";
		case BST::H264Video::AVC_LEVEL_4_2: return "4.2";
		case BST::H264Video::AVC_LEVEL_5: return "5";
		case BST::H264Video::AVC_LEVEL_5_1: return "5.1";
		case BST::H264Video::AVC_LEVEL_5_2:return "5.2";
		case BST::H264Video::AVC_LEVEL_6: return "6";
		case BST::H264Video::AVC_LEVEL_6_1: return "6.1";
		case BST::H264Video::AVC_LEVEL_6_2: return "6.2";
	}
	return "Unknown";
}

const std::map<int, AVC_LEVEL_LIMIT> avc_level_limits =
{
	/*1	*/	{BST::H264Video::AVC_LEVEL_1,	{ 1485		,	99		, 396		, 64		,175		, 64	, 2, UINT8_MAX }},
	/*1b*/	{BST::H264Video::AVC_LEVEL_1b,	{ 1485		,	99		, 396		, 128		,350		, 64	, 2, UINT8_MAX }},
	/*1.1*/	{BST::H264Video::AVC_LEVEL_1_1, { 3000		,	396		, 900		, 192		,500		, 128	, 2, UINT8_MAX }},
	/*1.2*/	{BST::H264Video::AVC_LEVEL_1_2, { 6000		,	396		, 2376		, 384		,1000		, 128	, 2, UINT8_MAX }},
	/*1.3*/	{BST::H264Video::AVC_LEVEL_1_3, { 11880		,	396		, 2376		, 768		,2000		, 128	, 2, UINT8_MAX }},
	/*2	*/	{BST::H264Video::AVC_LEVEL_2,	{ 11880		,	396		, 2376		, 2000		,2000		, 128	, 2, UINT8_MAX }},
	/*2.1*/	{BST::H264Video::AVC_LEVEL_2_1, { 19800		,	792		, 4752		, 4000		,4000		, 256	, 2, UINT8_MAX }},
	/*2.2*/	{BST::H264Video::AVC_LEVEL_2_2, { 20250		,	1620	, 8100		, 4000		,4000		, 256	, 2, UINT8_MAX }},
	/*3	*/	{BST::H264Video::AVC_LEVEL_3,	{ 40500		,	1620	, 8100		, 10000		,10000		, 256	, 2, 32		   }},
	/*3.1*/	{BST::H264Video::AVC_LEVEL_3_1, { 108000	,	3600	, 18000		, 14000		,14000		, 512	, 4, 16		   }},
	/*3.2*/	{BST::H264Video::AVC_LEVEL_3_2, { 216000	,	5120	, 20480		, 20000		,20000		, 512	, 4, 16		   }},
	/*4	*/	{BST::H264Video::AVC_LEVEL_4,	{ 245760	,	8192	, 32768		, 20000		,25000		, 512	, 4, 16		   }},
	/*4.1*/	{BST::H264Video::AVC_LEVEL_4_1, { 245760	,	8192	, 32768		, 50000		,62500		, 512	, 2, 16		   }},
	/*4.2*/	{BST::H264Video::AVC_LEVEL_4_2, { 522240	,	8704	, 34816		, 50000		,62500		, 512	, 2, 16		   }},
	/*5	*/	{BST::H264Video::AVC_LEVEL_5,	{ 589824	,	22080	, 110400	, 135000	,135000		, 512	, 2, 16		   }},
	/*5.1*/	{BST::H264Video::AVC_LEVEL_5_1, { 983040	,	36864	, 184320	, 240000	,240000		, 512	, 2, 16		   }},
	/*5.2*/	{BST::H264Video::AVC_LEVEL_5_2, { 2073600	,	36864	, 184320	, 240000	,240000		, 512	, 2, 16		   }},
	/*6	*/	{BST::H264Video::AVC_LEVEL_6,	{ 4177920	,	139264	, 696320	, 240000	,240000		, 8192	, 2, 16		   }},
	/*6.1*/	{BST::H264Video::AVC_LEVEL_6_1, { 8355840	,	139264	, 696320	, 480000	,480000		, 8192	, 2, 16		   }},
	/*6.2*/	{BST::H264Video::AVC_LEVEL_6_2, { 16711680	,	139264	, 696320	, 800000	,800000		, 8192	, 2, 16		   }},
};

RET_CODE CreateAVCNALContext(INALAVCContext** ppNALCtx)
{
	if (ppNALCtx == NULL)
		return RET_CODE_INVALID_PARAMETER;

	auto pCtx = new BST::H264Video::VideoBitstreamCtx();
	pCtx->AddRef();
	*ppNALCtx = (INALAVCContext*)pCtx;
	return RET_CODE_SUCCESS;
}

namespace BST
{
	namespace H264Video
	{
		AVC_PROFILE NAL_UNIT::SEQ_PARAMETER_SET_DATA::GetH264Profile()
		{
			if (profile_idc == 66) {
				if (constraint_set1_flag)
					return AVC_PROFILE_CONSTRAINED_BASELINE;

				return AVC_PROFILE_BASELINE;
			}
			else if (profile_idc == 77)
			{
				return AVC_PROFILE_MAIN;
			}
			else if (profile_idc == 88)
			{
				return AVC_PROFILE_EXTENDED;
			}
			else if (profile_idc == 100)
			{
				if (constraint_set4_flag && constraint_set5_flag)
					return AVC_PROFILE_CONSTRAINED_HIGH;
				if (constraint_set4_flag)
					return AVC_PROFILE_PROGRESSIVE_HIGH;

				return AVC_PROFILE_HIGH;
			}
			else if (profile_idc == 110)
			{
				if (constraint_set3_flag)
					return AVC_PROFILE_HIGH_10_INTRA;
				else if (constraint_set4_flag)
					return AVC_PROFILE_PROGRESSIVE_HIGH_10;

				return AVC_PROFILE_HIGH_10;
			}
			else if (profile_idc == 122)
			{
				if (constraint_set3_flag)
					return AVC_PROFILE_HIGH_422_INTRA;
				return AVC_PROFILE_HIGH_422;
			}
			else if (profile_idc == 244)
			{
				if (constraint_set3_flag)
					return AVC_PROFILE_HIGH_444_INTRA;
				return AVC_PROFILE_HIGH_444_PREDICTIVE;
			}
			else if (profile_idc == 44)
			{
				return AVC_PROFILE_CAVLC_444_INTRA_PROFIILE;
			}
			else if (profile_idc == 118)
			{
				return AVC_PROFILE_MULTIVIEW_HIGH;
			}
			else if (profile_idc == 128)
			{
				return AVC_PROFILE_STEREO_HIGH;
			}
			else if (profile_idc == 83)
			{
				if (constraint_set5_flag)
					return AVC_PROFILE_SCALABLE_CONSTRAINED_BASELINE;

				return AVC_PROFILE_SCALABLE_BASELINE;
			}
			else if (profile_idc == 86)
			{
				if (constraint_set5_flag)
					return AVC_PROFILE_SCALABLE_CONSTRAINED_HIGH;
				else if (constraint_set3_flag)
					return AVC_PROFILE_SCALABLE_HIGH_INTRA;
				return AVC_PROFILE_SCALABLE_HIGH;
			}
			else if (profile_idc == 138)
				return AVC_PROFILE_MULTIVIEW_DEPTH_HIGH;

			return AVC_PROFILE_UNKNOWN;
		}

		AVC_LEVEL NAL_UNIT::SEQ_PARAMETER_SET_DATA::GetH264Level()
		{
			AVC_PROFILE profile = GetH264Profile();
			if (profile == AVC_PROFILE_BASELINE || profile == AVC_PROFILE_CONSTRAINED_BASELINE ||
				profile == AVC_PROFILE_MAIN || profile == AVC_PROFILE_EXTENDED)
			{
				if (level_idc == 11 && constraint_set3_flag)
					return AVC_LEVEL_1b;
				else
					return (AVC_LEVEL)(level_idc * 10);
			}
			else if (profile == AVC_PROFILE_HIGH || profile == AVC_PROFILE_PROGRESSIVE_HIGH || profile == AVC_PROFILE_CONSTRAINED_HIGH ||
				profile == AVC_PROFILE_HIGH_10 || profile == AVC_PROFILE_HIGH_10_INTRA || profile == AVC_PROFILE_PROGRESSIVE_HIGH_10 ||
				profile == AVC_PROFILE_HIGH_422 || profile == AVC_PROFILE_HIGH_422_INTRA ||
				profile == AVC_PROFILE_HIGH_444_PREDICTIVE || profile == AVC_PROFILE_HIGH_444_INTRA ||
				profile == AVC_PROFILE_CAVLC_444_INTRA_PROFIILE ||
				profile == AVC_PROFILE_MULTIVIEW_HIGH || profile == AVC_PROFILE_STEREO_HIGH ||
				profile == AVC_PROFILE_SCALABLE_BASELINE || profile == AVC_PROFILE_SCALABLE_CONSTRAINED_BASELINE ||
				profile == AVC_PROFILE_SCALABLE_HIGH || profile == AVC_PROFILE_SCALABLE_CONSTRAINED_HIGH || profile == AVC_PROFILE_SCALABLE_HIGH_INTRA ||
				profile == AVC_PROFILE_MULTIVIEW_DEPTH_HIGH)
			{
				if (level_idc == 9)
					return AVC_LEVEL_1b;
				else
					return (AVC_LEVEL)(level_idc * 10);
			}

			return AVC_LEVEL_UNKNOWN;
		}

		void NAL_UNIT::SEQ_PARAMETER_SET_DATA::UpdateConcludedValues()
		{
			SubWidthC = (chroma_format_idc == 1 || chroma_format_idc == 2) ? 2 : (chroma_format_idc == 3 && separate_colour_plane_flag == 0 ? 1 : 0);
			SubHeightC = (chroma_format_idc == 2 || (chroma_format_idc == 3 && separate_colour_plane_flag == 0)) ? 1 : (chroma_format_idc == 1 ? 2 : 0);
			BitDepthY = 8 + bit_depth_luma_minus8;
			BitDepthC = 8 + bit_depth_chroma_minus8;
			QpBdOffsetY = 6 * bit_depth_luma_minus8;
			QpBdOffsetC = 6 * bit_depth_chroma_minus8;
			MbWidthC = SubWidthC ? 16 / SubWidthC : 0;
			MbHeightC = SubHeightC ? 16 / SubHeightC : 0;
			RawMbBits = 256 * BitDepthY + 2 * MbWidthC*MbHeightC*BitDepthC;
			MaxFrameNum = 2 ^ (log2_max_frame_num_minus4 + 4);
			MaxPicOrderCntLsb = 2 ^ (log2_max_pic_order_cnt_lsb_minus4 + 4);
			PicWidthInMbs = pic_width_in_mbs_minus1 + 1;
			PicWidthInSamplesL = PicWidthInMbs * 16;
			PicWidthInSamplesC = PicWidthInMbs * MbWidthC;
			PicHeightInMapUnits = pic_height_in_map_units_minus1 + 1;
			PicSizeInMapUnits = PicWidthInMbs * PicHeightInMapUnits;
			FrameHeightInMbs = (2 - frame_mbs_only_flag) * PicHeightInMapUnits;
			ChromaArrayType = separate_colour_plane_flag == 0 ? chroma_format_idc : 0;
			CropUnitX = ChromaArrayType == 0 ? 1 : SubWidthC;
			CropUnitY = ChromaArrayType == 0 ? (2 - frame_mbs_only_flag) : SubHeightC * (2 - frame_mbs_only_flag);

			frame_buffer_width = PicWidthInSamplesL;
			frame_buffer_height = FrameHeightInMbs * 16;
			display_width = frame_buffer_width;
			display_height = frame_buffer_height;

			if (frame_cropping_flag)
			{
				uint32_t crop_unit_x = 0, crop_unit_y = 0;
				if (0 == chroma_format_idc)	// monochrome
				{
					crop_unit_x = 1;
					crop_unit_y = 2 - frame_mbs_only_flag;
				}
				else if (1 == chroma_format_idc)	// 4:2:0
				{
					crop_unit_x = 2;
					crop_unit_y = 2 * (2 - frame_mbs_only_flag);
				}
				else if (2 == chroma_format_idc)	// 4:2:2
				{
					crop_unit_x = 2;
					crop_unit_y = 2 - frame_mbs_only_flag;
				}
				else if (3 == chroma_format_idc)
				{
					crop_unit_x = 1;
					crop_unit_y = 2 - frame_mbs_only_flag;
				}

				display_width -= crop_unit_x * (frame_crop_left_offset + frame_crop_right_offset);
				display_height -= crop_unit_y * (frame_crop_top_offset + frame_crop_bottom_offset);

				return;
			}
		}

		int NAL_UNIT::PIC_PARAMETER_SET_RBSP::Map(AMBst in_bst)
		{
			int iRet = RET_CODE_SUCCESS;
			SYNTAX_BITSTREAM_MAP::Map(in_bst);

			try
			{
				MAP_BST_BEGIN(0);
				nal_read_ue(in_bst, pic_parameter_set_id, uint8_t);
				nal_read_ue(in_bst, seq_parameter_set_id, uint8_t);
				nal_read_u(in_bst, entropy_coding_mode_flag, 1, uint8_t);
				nal_read_u(in_bst, bottom_field_pic_order_in_frame_present_flag, 1, uint8_t);
				nal_read_ue(in_bst, num_slice_groups_minus1, uint8_t);

				if (num_slice_groups_minus1 > 0)
				{
					nal_read_ue(in_bst, slice_group_map_type, uint8_t);
					if (slice_group_map_type == 0)
					{
						run_length_minus1.reserve((size_t)num_slice_groups_minus1 + 1);
						for (int iGroup = 0; iGroup <= num_slice_groups_minus1; iGroup++)
						{
							nal_read_ue(in_bst, run_length_minus1[iGroup], uint16_t);
						}
					}
					else if (slice_group_map_type == 2)
					{
						top_left.reserve((size_t)num_slice_groups_minus1 + 1);
						bottom_right.reserve((size_t)num_slice_groups_minus1 + 1);
						for (int iGroup = 0; iGroup <= num_slice_groups_minus1; iGroup++)
						{
							nal_read_ue(in_bst, top_left[iGroup], uint16_t);
							nal_read_ue(in_bst, bottom_right[iGroup], uint16_t);
						}
					}
					else if (slice_group_map_type == 3 || slice_group_map_type == 4 || slice_group_map_type == 5)
					{
						nal_read_u(in_bst, slice_group_change_direction_flag, 1, uint8_t);
						nal_read_ue(in_bst, slice_group_change_rate_minus1, uint16_t);
					}
					else if (slice_group_map_type == 6)
					{
						nal_read_ue(in_bst, pic_size_in_map_units_minus1, uint16_t);
						slice_group_id.reserve((size_t)pic_size_in_map_units_minus1 + 1);
						uint8_t v = quick_ceil_log2(num_slice_groups_minus1 + 1);
						for (int i = 0; i <= pic_size_in_map_units_minus1; i++)
						{
							nal_read_u(in_bst, slice_group_id[i], v, uint8_t);
						}
					}
				}

				nal_read_ue(in_bst, num_ref_idx_l0_default_active_minus1, uint16_t);
				nal_read_ue(in_bst, num_ref_idx_l1_default_active_minus1, uint16_t);

				nal_read_u(in_bst, weighted_pred_flag, 1, uint16_t);
				nal_read_u(in_bst, weighted_bipred_idc, 2, uint16_t);

				nal_read_se(in_bst, pic_init_qp_minus26, int8_t);
				nal_read_se(in_bst, pic_init_qs_minus26, int8_t);
				nal_read_se(in_bst, chroma_qp_index_offset, int8_t);

				nal_read_u(in_bst, deblocking_filter_control_present_flag, 1, uint16_t);
				nal_read_u(in_bst, constrained_intra_pred_flag, 1, uint16_t);
				nal_read_u(in_bst, redundant_pic_cnt_present_flag, 1, uint16_t);

				if ((m_bMoreData = AMBst_NAL_more_rbsp_data(in_bst)))
				{
					nal_read_u(in_bst, transform_8x8_mode_flag, 1, uint8_t);
					nal_read_u(in_bst, pic_scaling_matrix_present_flag, 1, uint8_t);
					if (pic_scaling_matrix_present_flag)
					{
						uint16_t scaling_list_size = 6;
						if (transform_8x8_mode_flag != 0)
						{
							if (ptr_ctx_video_bst == NULL)
								throw AMException(RET_CODE_CONTAINER_NOT_EXIST, _T("Don't find h264 SPS in SPSes of container"));

							auto sp_sps = ptr_ctx_video_bst->GetAVCSPS(seq_parameter_set_id);
							if (!sp_sps || sp_sps->ptr_seq_parameter_set_rbsp == NULL)
								throw AMException(RET_CODE_CONTAINER_NOT_EXIST, _T("Don't find h264 SPS in SPSes of container"));

							chroma_format_idc = sp_sps->ptr_seq_parameter_set_rbsp->seq_parameter_set_data.chroma_format_idc;
							scaling_list_size += ((chroma_format_idc != 3) ? 2 : 6) * transform_8x8_mode_flag;
						}
						scaling_list.resize(scaling_list_size);
						for (int i = 0; i < scaling_list_size; i++)
						{
							nal_read_bitarray(in_bst, pic_scaling_list_present_flag, i);
							if (pic_scaling_list_present_flag[i])
							{
								nal_read_ref(in_bst, scaling_list[i], SCALING_LIST, i < 6 ? 16 : 64);
							}
							else
								scaling_list[i] = NULL;
						}
					}

					nal_read_se(in_bst, second_chroma_qp_index_offset, int8_t);
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
			prev_seq_parameter_set_id = -1; 
			sp_h264_spses.clear(); 
			sp_h264_ppses.clear(); 
			sp_prev_nal_unit = nullptr;
			m_active_sps_id = -1;
			m_active_nu_type = -1;
		}
		
		H264_NU VideoBitstreamCtx::GetAVCSPS(uint8_t sps_id)
		{
			auto iter = sp_h264_spses.find(sps_id);
			if (iter != sp_h264_spses.end())
				return iter->second;

			return std::shared_ptr<NAL_UNIT>();
		}

		H264_NU VideoBitstreamCtx::GetAVCPPS(uint8_t pps_id)
		{
			auto iter = sp_h264_ppses.find(pps_id);
			if (iter != sp_h264_ppses.end())
				return iter->second;

			return std::shared_ptr<NAL_UNIT>();
		}

		RET_CODE VideoBitstreamCtx::UpdateAVCSPS(H264_NU sps_nu)
		{
			if (!sps_nu || sps_nu->nal_unit_header.nal_unit_type != SPS_NUT || sps_nu->ptr_seq_parameter_set_rbsp == nullptr)
				return RET_CODE_INVALID_PARAMETER;

			sp_h264_spses[sps_nu->ptr_seq_parameter_set_rbsp->seq_parameter_set_data.seq_parameter_set_id] = sps_nu;
			return RET_CODE_SUCCESS;
		}

		RET_CODE VideoBitstreamCtx::UpdateAVCPPS(H264_NU pps_nu)
		{
			if (!pps_nu || pps_nu->nal_unit_header.nal_unit_type != PPS_NUT || pps_nu->ptr_pic_parameter_set_rbsp == nullptr)
				return RET_CODE_INVALID_PARAMETER;

			sp_h264_ppses[pps_nu->ptr_pic_parameter_set_rbsp->pic_parameter_set_id] = pps_nu;
			return RET_CODE_SUCCESS;
		}

		H264_NU VideoBitstreamCtx::CreateAVCNU()
		{
			auto ptr_AVC_NU = new NAL_UNIT;
			ptr_AVC_NU->UpdateCtx(this);
			return std::shared_ptr<NAL_UNIT>(ptr_AVC_NU);
		}

		int8_t VideoBitstreamCtx::GetActiveSPSID()
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

		VideoBitstreamCtx::~VideoBitstreamCtx()
		{
		}
	}	// namespace H264Video
}	// namespace BST

