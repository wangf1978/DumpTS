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
#include "tinyxml2.h"
#include "AMSHA1.h"
#include "DataUtil.h"
#include "nal_parser.h"

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

		VideoBitstreamCtx::~VideoBitstreamCtx()
		{
		}
	}	// namespace H264Video
}	// namespace BST

int PushESBP(AMLinearRingBuffer rbNALUnitEBSP, uint8_t* pStart, uint8_t* pEnd)
{
	int cbLeftOfEBSP = 0;
	uint8_t* pEBSPWriteBuf = AM_LRB_GetWritePtr(rbNALUnitEBSP, &cbLeftOfEBSP);
	if (pEBSPWriteBuf == NULL || cbLeftOfEBSP < (int)(pEnd - pStart))
	{
		// Try to reform the ring buffer of EBSP
		AM_LRB_Reform(rbNALUnitEBSP);
		pEBSPWriteBuf = AM_LRB_GetWritePtr(rbNALUnitEBSP, &cbLeftOfEBSP);
		if (pEBSPWriteBuf == NULL || cbLeftOfEBSP < (int)(pEnd - pStart))
		{
			// Try to enlarge the ring buffer of EBSP
			int rb_size = AM_LRB_GetSize(rbNALUnitEBSP);
			int64_t new_rb_size = (int64_t)rb_size << 1;
			if (new_rb_size < rb_size + (pEnd - pStart) - (pEBSPWriteBuf != NULL ? cbLeftOfEBSP : 0))
				new_rb_size = rb_size + (pEnd - pStart) - (pEBSPWriteBuf != NULL ? cbLeftOfEBSP : 0);

			if (new_rb_size >= INT32_MAX || AM_LRB_Resize(rbNALUnitEBSP, (int)new_rb_size) < 0)
			{
				printf("Failed to resize the ring buffer of NAL Unit EBSP to %" PRId64 ".\n", (int64_t)rb_size * 2);
				return RET_CODE_ERROR;
			}

			pEBSPWriteBuf = AM_LRB_GetWritePtr(rbNALUnitEBSP, &cbLeftOfEBSP);
		}
	}

	// Write the parsed buffer to ring buffer
	memcpy(pEBSPWriteBuf, pStart, (size_t)(pEnd - pStart));

	AM_LRB_SkipWritePtr(rbNALUnitEBSP, (unsigned int)(pEnd - pStart));

	return RET_CODE_SUCCESS;
}

// FIXME
// At present, only support SPS and PPS, later we may support Sequence parameter set extension(13) and Subset sequence parameter set(15)
int LoadAVCParameterSet(INALAVCContext* pNALAVCCtx, AMLinearRingBuffer rbNALUnitEBSP, uint64_t cur_submit_pos)
{
	int iRet = RET_CODE_SUCCESS;
	int read_buf_len = 0;
	AMBst bst = NULL;

	uint8_t* pStart = AM_LRB_GetReadPtr(rbNALUnitEBSP, &read_buf_len);
	if (pStart == NULL || read_buf_len < 3)
	{
		//printf("the byte stream of NAL unit carry insufficient data {offset: %llu}.\n", cur_submit_pos);
		return RET_CODE_ERROR;
	}

	int8_t nal_unit_type = *pStart & 0x1F;
	AMP_Assert(IS_AVC_PARAMETERSET_NAL(nal_unit_type));

	auto nal_unit = pNALAVCCtx->CreateAVCNU();

	bst = AMBst_CreateFromBuffer(pStart, read_buf_len);
	if (AMP_FAILED(iRet = nal_unit->Map(bst)))
	{
		printf("Failed to unpack %s parameter set {offset: %" PRIu64 ", err: %d}\n", avc_nal_unit_type_descs[nal_unit_type], cur_submit_pos, iRet);
		goto done;
	}

	if (nal_unit->nal_unit_header.nal_unit_type == BST::H264Video::SPS_NUT)
	{
		static AMSHA1_RET prev_sps_sha1_ret[32] = { {0} };
		// Check whether the buffer is the same with previous one or not
		AMSHA1_RET sha1_ret = { 0 };
		AMSHA1 sha1_handle = AM_SHA1_Init(pStart, read_buf_len);
		AM_SHA1_Finalize(sha1_handle);
		AM_SHA1_GetHash(sha1_handle, sha1_ret);
		AM_SHA1_Uninit(sha1_handle);

		if (memcmp(prev_sps_sha1_ret[nal_unit->ptr_seq_parameter_set_rbsp->seq_parameter_set_data.seq_parameter_set_id], sha1_ret, sizeof(AMSHA1_RET)) == 0)
		{
			goto done;
		}
		else
		{
			memcpy(prev_sps_sha1_ret[nal_unit->ptr_seq_parameter_set_rbsp->seq_parameter_set_data.seq_parameter_set_id], sha1_ret, sizeof(AMSHA1_RET));
		}

		char* szXmlOutput = NULL;
		tinyxml2::XMLDocument xmlDoc;
		int xml_buffer_size = (int)nal_unit->ProduceDesc(NULL, 0);
		if (xml_buffer_size <= 0)
		{
			printf("Failed to export Xml from the H264 SPS NAL Unit.\n");
			goto done;
		}

		pNALAVCCtx->UpdateAVCSPS(nal_unit);

		szXmlOutput = new char[xml_buffer_size + 1];
		if ((xml_buffer_size = (int)nal_unit->ProduceDesc(szXmlOutput, xml_buffer_size + 1)) <= 0)
		{
			AMP_SAFEDELA(szXmlOutput);
			printf("Failed to generate the Xml from the H264 SPS NAL Unit.\n");
			goto done;
		}

		if (xmlDoc.Parse(szXmlOutput, xml_buffer_size) == tinyxml2::XML_SUCCESS)
		{
			int max_len_of_fixed_part = 0;
			// Get the max length
			{
				struct MaxLenTestUtil : tinyxml2::XMLVisitor
				{
					MaxLenTestUtil() : level(0), szLongSpace{ 0 }, max_length(0){
						memset(szLongSpace, ' ', 240);
					}
					/// Visit an element.
					virtual bool VisitEnter(const tinyxml2::XMLElement& element, const tinyxml2::XMLAttribute* firstAttribute) {
						char szTmp[2048] = { 0 };
						const char* szValue = element.Attribute("Value");
						const char* szAlias = element.Attribute("Alias");
						int ccWritten = (int)MBCSPRINTF_S(szTmp, sizeof(szTmp)/sizeof(szTmp[0]), "%.*s%s: %s", level * 4, szLongSpace,
							szAlias ? szAlias : element.Name(),
							szValue ? szValue : "");

						if (ccWritten > max_length)
							max_length = ccWritten;

						level++;
						return true;
					}
					/// Visit an element.
					virtual bool VisitExit(const tinyxml2::XMLElement& element) {
						level--;
						return true;
					}

					int level;
					char szLongSpace[241];
					int max_length;

				} max_length_tester;

				xmlDoc.Accept(&max_length_tester);
				max_len_of_fixed_part = max_length_tester.max_length;
			}

			struct TestUtil : tinyxml2::XMLVisitor
			{
				TestUtil(int max_length) : level(0), szLongSpace{ 0 }, max_fixed_part_length(max_length){
					memset(szLongSpace, ' ', 240);
				}
				/// Visit an element.
				virtual bool VisitEnter(const tinyxml2::XMLElement& element, const tinyxml2::XMLAttribute* firstAttribute) {
					char szTmp[2048] = { 0 };
					const char* szValue = element.Attribute("Value");
					const char* szAlias = element.Attribute("Alias");
					const char* szDesc = element.Attribute("Desc");
					int ccWritten = (int)MBCSPRINTF_S(szTmp, sizeof(szTmp) / sizeof(szTmp[0]), "%.*s%s: %s", level * 4, szLongSpace,
						szAlias ? szAlias : element.Name(),
						szValue ? szValue : "");

					printf("%s%.*s%s%s\n", szTmp,
						max_fixed_part_length - ccWritten, szLongSpace,
						szDesc && strcmp(szDesc, "") != 0 ? "// " : "",
						szDesc ? GetFixedWidthStrWithEllipsis(szDesc, 60).c_str() : "");
					level++;
					return true;
				}
				/// Visit an element.
				virtual bool VisitExit(const tinyxml2::XMLElement& element) {
					level--;
					return true;
				}

				int level;
				char szLongSpace[241];
				int max_fixed_part_length;

			} tester(max_len_of_fixed_part);

			xmlDoc.Accept(&tester);
		}
		else
			printf("The generated XML is invalid.\n");

		AMP_SAFEDELA(szXmlOutput);
	}
	else if (nal_unit->nal_unit_header.nal_unit_type == BST::H264Video::PPS_NUT)
	{

	}

done:
	if (bst != NULL)
		AMBst_Destroy(bst);

	nal_unit = nullptr;

	return iRet;
}

int ShowH264SPS(const char* szH264StreamFile)
{
	int8_t cur_nal_unit_type = -1;
	int8_t cur_nuh_layer_id = -1;
	int8_t cur_nuh_temporal_id_plus1 = -1;
	int8_t cur_nal_ref_idc = -1;
	long long count_nal_unit_scanned = 0LL;

	int64_t file_size = 0;

	bool bNalUnitZeroByte = false;
	uint64_t cur_scan_pos = 0, cur_submit_pos = 0;
	std::vector<NAL_UNIT_ENTRY>	nu_entries;

	const int read_unit_size = 2048;
	const int minimum_nal_parse_buffer_size = 5;	// 3 prefix start code + 2 NAL unit header

	FILE* rfp = NULL;
	int iRet = RET_CODE_SUCCESS;
	// Create a bigger ring buffer to avoid frequent linear ring buffer reform
	AMLinearRingBuffer rbRawBuf = AM_LRB_Create(read_unit_size * 128);

	// Create a ring buffer which is used to hold a whole VPS/SPS/PPS EBSP exclude NAL unit header
	AMLinearRingBuffer rbNALUnitEBSP = AM_LRB_Create(64 * 1024);

	int cbSize = 0;
	uint8_t* pStartBuf = NULL, *pBuf = NULL, *pCurParseStartBuf = NULL;

	INALAVCContext* pNALAVCCtx = NULL;
	if (AMP_FAILED(CreateAVCNALContext(&pNALAVCCtx)))
	{
		printf("Failed to create the NAL context.\n");
		return RET_CODE_ERROR;
	}

	errno_t errn = fopen_s(&rfp, szH264StreamFile, "rb");
	if (errn != 0 || rfp == NULL)
	{
		printf("Failed to open the file: %s {errno: %d}.\n", szH264StreamFile, errn);
		goto done;
	}

	// Get file size
	_fseeki64(rfp, 0, SEEK_END);
	file_size = _ftelli64(rfp);
	_fseeki64(rfp, 0, SEEK_SET);

	do
	{
		int read_size = read_unit_size;
		pBuf = AM_LRB_GetWritePtr(rbRawBuf, &read_size);
		if (pBuf == NULL || read_size < read_unit_size)
		{
			// Try to reform linear ring buffer
			AM_LRB_Reform(rbRawBuf);
			if ((pBuf = AM_LRB_GetWritePtr(rbRawBuf, &read_size)) == NULL || read_size < read_unit_size)
			{
				iRet = RET_CODE_ERROR;
				break;
			}
		}

		if ((read_size = (int)fread(pBuf, 1, read_unit_size, rfp)) <= 0)
		{
			iRet = RET_CODE_IO_READ_ERROR;
			break;
		}

		AM_LRB_SkipWritePtr(rbRawBuf, (unsigned int)read_size);

		if ((pStartBuf = AM_LRB_GetReadPtr(rbRawBuf, &cbSize)) == NULL || cbSize <= 0)
		{
			iRet = RET_CODE_ERROR;
			break;
		}

		pCurParseStartBuf = pBuf = pStartBuf;
		while (cbSize >= minimum_nal_parse_buffer_size)
		{
			// Find "start_code_prefix_one_3bytes"
			while (cbSize >= minimum_nal_parse_buffer_size && !(pBuf[0] == 0 && pBuf[1] == 0 && pBuf[2] == 1))
			{
				cbSize--;
				pBuf++;
			}

			// Don't miss the zero_byte before "start_code_prefix_one_3bytes"
			if (cbSize >= minimum_nal_parse_buffer_size)
			{
				if (pBuf > pCurParseStartBuf && *(pBuf - 1) == 0) {
					bNalUnitZeroByte = true;
					cbSize++;
					pBuf--;
				}
				else
					bNalUnitZeroByte = false;
			}

			// If the current NAL unit type is valid, need push the parsed data to ring buffer of EBSP
			if (cur_nal_unit_type != -1 && pBuf > pCurParseStartBuf)
			{
				if (FAILED(PushESBP(rbNALUnitEBSP, pCurParseStartBuf, pBuf)))
					goto done;
			}

			// Failed to find the "start_code_prefix_one_3bytes"
			if (cbSize < minimum_nal_parse_buffer_size)
				break;	// Quit the current loop and read more data and do the next round of scanning

			// check the nal_unit_type
			int8_t nal_unit_prefix_start_code_length = 3 + (bNalUnitZeroByte ? 1 : 0);
			int8_t forbidden_zero_bit = (pBuf[nal_unit_prefix_start_code_length] >> 7) & 0x01;

			int8_t nal_unit_type = -1, nuh_layer_id = -1, nuh_temporal_id_plus1 = -1, nal_ref_idc = -1;
			uint64_t file_offset = cur_scan_pos + (size_t)(pBuf + nal_unit_prefix_start_code_length - pStartBuf);

			pCurParseStartBuf = pBuf + nal_unit_prefix_start_code_length;

			if (IS_AVC_PARAMETERSET_NAL(cur_nal_unit_type))
			{
				// Parsing the ebsp parameter set, and store it
				LoadAVCParameterSet(pNALAVCCtx, rbNALUnitEBSP, cur_submit_pos);

				// Drop all data in ring buffer EBSP
				AM_LRB_Reset(rbNALUnitEBSP);
			}
			else if (IS_AVC_VCL_NAL(cur_nal_unit_type))
			{
				// Drop all data in ring buffer EBSP
				AM_LRB_Reset(rbNALUnitEBSP);
			}

			nal_ref_idc = (pBuf[nal_unit_prefix_start_code_length] >> 5) & 0x03;
			nal_unit_type = pBuf[nal_unit_prefix_start_code_length] & 0x1F;

			// Record the current nal_unit_header information and file offset
			nu_entries.emplace_back(file_offset, nal_unit_prefix_start_code_length, forbidden_zero_bit, nal_ref_idc, nal_unit_type);

			//printf("#%08lld [byte_pos: %08lld] NAL_UNIT, type: %S, nal_ref_idc: %d.\n",
			//	count_nal_unit_scanned, scan_info->nu_entries.back().file_offset, avc_nal_unit_type_names[nal_unit_type], nal_ref_idc);

			count_nal_unit_scanned++;

			// Skip "prefix start code", it may be 00 00 01 or 00 00 00 01
			pBuf += nal_unit_prefix_start_code_length; cbSize -= nal_unit_prefix_start_code_length;

			// If the current NAL_UNIT is not required to parse, just skip it, and find the next start code
			if (!IS_AVC_PARAMETERSET_NAL(nal_unit_type) && !IS_AVC_VCL_NAL(nal_unit_type))
			{
				// Skipping Zero byte, "start_code_prefix_one_3bytes" and nal_unit_header
				cur_nal_unit_type = -1;
				continue;
			}

			cur_nal_unit_type = nal_unit_type;
			cur_nal_ref_idc = nal_ref_idc;

			cur_submit_pos = cur_scan_pos + (size_t)(pBuf - pStartBuf);
		}

		// Skip the parsed raw data buffer
		AM_LRB_SkipReadPtr(rbRawBuf, (unsigned int)(pBuf - pStartBuf));

		// Update the scan position
		cur_scan_pos += pBuf - pStartBuf;

		// Since it is ok to only parsing slice header, so we can check here
		// Sometimes VCL NAL unit size is very big, for example 100MB, it is impossible to collect all NAL unit rbsp, and then parsing the header
		if (IS_AVC_VCL_NAL(cur_nal_unit_type))
		{
			// Drop all data in ring buffer EBSP
			AM_LRB_Reset(rbNALUnitEBSP);

			// Skip this NAL unit, and try to find the next NAL_UNIT
			cur_nal_unit_type = -1;
		}

	} while (!feof(rfp));

	if (feof(rfp))
	{
		if (IS_PARSABLE_AVC_NAL(cur_nal_unit_type))
		{
			if (FAILED(PushESBP(rbNALUnitEBSP, pCurParseStartBuf, pBuf + cbSize)))
				goto done;

			// If the current NAL unit is VPS/SPS/PPS, try to parse them
			if (IS_AVC_PARAMETERSET_NAL(cur_nal_unit_type))
			{
				// Parsing the ebsp parameter set, and store it
				LoadAVCParameterSet(pNALAVCCtx, rbNALUnitEBSP, cur_submit_pos);
			}
			else if (IS_AVC_VCL_NAL(cur_nal_unit_type))
			{
			}
		}

		cur_scan_pos = (uint64_t)file_size;
	}

done:
	if (rfp != NULL)
		fclose(rfp);

	if (rbRawBuf != NULL)
		AM_LRB_Destroy(rbRawBuf);

	if (rbNALUnitEBSP != NULL)
		AM_LRB_Destroy(rbNALUnitEBSP);

	if (pNALAVCCtx != NULL)
		pNALAVCCtx->Release();

	return iRet;
}

