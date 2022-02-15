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
		case BST::H264Video::BASELINE_PROFILE: return "Baseline Profile";
		case BST::H264Video::CONSTRAINED_BASELINE_PROFILE: return "Constrained Baseline Profile";
		case BST::H264Video::MAIN_PROFILE: return "Main Profile";
		case BST::H264Video::EXTENDED_PROFILE: return "Extended Profile";
		case BST::H264Video::HIGH_PROFILE: return "High Profile";
		case BST::H264Video::PROGRESSIVE_HIGH_PROFILE: return "Progressive High Profile";
		case BST::H264Video::CONSTRAINED_HIGH_PROFILE: return "Constrained High Profile";
		case BST::H264Video::HIGH_10_PROFILE: return "High 10 Profile";
		case BST::H264Video::HIGH_10_INTRA_PROFILE: return "High 10 Intra Profile";
		case BST::H264Video::HIGH_422_PROFILE: return "High 422 Profile";
		case BST::H264Video::HIGH_422_INTRA_PROFILE: return "High 422 Intra Profile";
		case BST::H264Video::HIGH_444_PREDICTIVE_PROFILE: return "High 444 Predictive Profile";
		case BST::H264Video::HIGH_444_INTRA_PROFILE: return "High 444 Intra Profile";
		case BST::H264Video::CAVLC_444_INTRA_PROFIILE: return "CAVLC 444 Intra Profile";
		case BST::H264Video::MULTIVIEW_HIGH_PROFILE: return "MultiView High Profile";
		case BST::H264Video::STEREO_HIGH_PROFILE: return "Stereo High Profile";
		case BST::H264Video::SCALABLE_BASELINE_PROFILE: return "Scalable Baseline Profile";
		case BST::H264Video::SCALABLE_CONSTRAINED_BASELINE_PROFILE: return "Scalable Constrained Baseline Profile";
		case BST::H264Video::SCALABLE_HIGH_PROFILE: return "Scalable High Profile";
		case BST::H264Video::SCALABLE_CONSTRAINED_HIGH_PROFILE: return "Scalable Constrained High Profile";
		case BST::H264Video::SCALABLE_HIGH_INTRA_PROFILE: return "Scalable High Intra Profile";
		case BST::H264Video::MULTIVIEW_DEPTH_HIGH_PROFILE: return "MultiView Depth High Profile";
	}

	return "Unknown Profile";
}

const char* get_h264_level_name(int level)
{
	switch (level)
	{
		case BST::H264Video::LEVEL_1: return "1";
		case BST::H264Video::LEVEL_1b: return "1b";
		case BST::H264Video::LEVEL_1_1: return "1.1";
		case BST::H264Video::LEVEL_1_2: return "1.2";
		case BST::H264Video::LEVEL_1_3: return "1.3";
		case BST::H264Video::LEVEL_2: return "2";
		case BST::H264Video::LEVEL_2_1: return "2.1";
		case BST::H264Video::LEVEL_2_2: return "2.2";
		case BST::H264Video::LEVEL_3: return "3";
		case BST::H264Video::LEVEL_3_1: return "3.1";
		case BST::H264Video::LEVEL_3_2: return "3.2";
		case BST::H264Video::LEVEL_4: return "4";
		case BST::H264Video::LEVEL_4_1: return "4.1";
		case BST::H264Video::LEVEL_4_2: return "4.2";
		case BST::H264Video::LEVEL_5: return "5";
		case BST::H264Video::LEVEL_5_1: return "5.1";
		case BST::H264Video::LEVEL_5_2:return "5.2";
	}
	return "Unknown";
}

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
						run_length_minus1.reserve(num_slice_groups_minus1 + 1);
						for (int iGroup = 0; iGroup <= num_slice_groups_minus1; iGroup++)
						{
							nal_read_ue(in_bst, run_length_minus1[iGroup], uint16_t);
						}
					}
					else if (slice_group_map_type == 2)
					{
						top_left.reserve(num_slice_groups_minus1 + 1);
						bottom_right.reserve(num_slice_groups_minus1 + 1);
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
						slice_group_id.reserve(pic_size_in_map_units_minus1 + 1);
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

