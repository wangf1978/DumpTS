#include "platcomm.h"
#include "av1.h"

const char* obu_type_names[16] = {
	"Reserved",
	"Sequence header OBU", 
	"Temporal delimiter OBU",
	"Frame header OBU",
	"Tile group OBU",
	"Metadata OBU",
	"Frame OBU",
	"Redundant frame header OBU",
	"Tile list OBU",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Padding OBU"
};

const char* obu_type_short_names[16] = {
	"Reserved",
	"Sequence header",
	"Temporal delimiter",
	"Frame header",
	"Tile group",
	"Metadata",
	"Frame",
	"Redundant frame header",
	"Tile list",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Padding"
};

const char* color_primaries_descs[23] = {
	"Reserved",
	"BT.709",
	"Unspecified",
	"Reserved",
	"BT.470 System M(historical)",
	"BT.470 System B, G(historical)",
	"BT.601",
	"SMPTE 240",
	"Generic film(color filters using illuminant C)",
	"BT.2020, BT.2100",
	"SMPTE 428 (CIE 1921 XYZ)",
	"SMPTE RP 431-2",
	"SMPTE EG 432-1",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"EBU Tech. 3213-E",
};

const char* transfer_characteristics_descs[19] = {
	"For future use",
	"BT.709",
	"Unspecified",
	"For future use",
	"BT.470 System M(historical)",
	"BT.470 System B, G(historical)",
	"BT.601",
	"SMPTE 240 M",
	"Linear",
	"Logarithmic(100 : 1 range)",
	"Logarithmic(100 * Sqrt(10) : 1 range)",
	"IEC 61966-2-4",
	"BT.1361",
	"sRGB or sYCC",
	"BT.2020 10-bit systems",
	"BT.2020 12-bit systems",
	"SMPTE ST 2084, ITU BT.2100 PQ",
	"SMPTE ST 428",
	"BT.2100 HLG, ARIB STD-B67",
};

const char* matrix_coefficients_descs[15] = {
	"Identity matrix",
	"BT.709",
	"Unspecified",
	"For future use",
	"US FCC 73.628",
	"BT.470 System B, G (historical)",
	"BT.601",
	"SMPTE 240 M",
	"YCgCo",
	"BT.2020 non-constant luminance, BT.2100 YCbCr",
	"BT.2020 constant luminance",
	"SMPTE ST 2085 YDzDx",
	"Chromaticity-derived non-constant luminance",
	"Chromaticity-derived constant luminance",
	"BT.2100 ICtCp",
};

const char* chroma_sample_position_descs[4] = {
	"Unknown(in this case the source video transfer function must be signaled outside the AV1 bitstream)",
	"Horizontally co - located with(0, 0) luma sample, vertical position in the middle between two luma samples", 
	"co-located with(0, 0) luma sample", 
	""
};

const uint8_t Segmentation_Feature_Bits[SEG_LVL_MAX] = { 8, 6, 6, 6, 6, 3, 0, 0 };
const uint8_t Segmentation_Feature_Signed[SEG_LVL_MAX] = { 1, 1, 1, 1, 1, 0, 0, 0 };
const uint8_t Segmentation_Feature_Max[SEG_LVL_MAX] = { 255, MAX_LOOP_FILTER, MAX_LOOP_FILTER,MAX_LOOP_FILTER, MAX_LOOP_FILTER, 7, 0, 0 };
const uint8_t Remap_Lr_Type[4] = { RESTORE_NONE, RESTORE_SWITCHABLE, RESTORE_WIENER, RESTORE_SGRPROJ };
const uint8_t Ref_Frame_List[REFS_PER_FRAME - 2] = {LAST2_FRAME, LAST3_FRAME, BWDREF_FRAME, ALTREF2_FRAME, ALTREF_FRAME};

const WarpedMotionParams default_warp_params = {
	IDENTITY,
	{ 0, 0, (1 << WARPEDMODEL_PREC_BITS), 0, 0, (1 << WARPEDMODEL_PREC_BITS), 0,
	0 },
	0, 0, 0, 0,
	0,
};

const char* get_av1_profile_name(int profile)
{
	switch (profile)
	{
		case BST::AV1::AV1_PROFILE_MAIN: return "Main";
		case BST::AV1::AV1_PROFILE_HIGH: return "High";
		case BST::AV1::AV1_PROFILE_PROFESSIONAL: return "Professional";
		default: break;
	}
	return "Unknown";
}

const char* get_av1_level_name(int level)
{
	switch (level)
	{
	case BST::AV1::AV1_LEVEL_2:		return "2.0";
	case BST::AV1::AV1_LEVEL_2_1:	return "2.1";
	case BST::AV1::AV1_LEVEL_2_2:	return "2.2";
	case BST::AV1::AV1_LEVEL_2_3:	return "2.3";
	case BST::AV1::AV1_LEVEL_3:		return "3.0";
	case BST::AV1::AV1_LEVEL_3_1:	return "3.1";
	case BST::AV1::AV1_LEVEL_3_2:	return "3.2";
	case BST::AV1::AV1_LEVEL_3_3:	return "3.3";
	case BST::AV1::AV1_LEVEL_4:		return "4.0";
	case BST::AV1::AV1_LEVEL_4_1:	return "4.1";
	case BST::AV1::AV1_LEVEL_4_2:	return "4.2";
	case BST::AV1::AV1_LEVEL_4_3:	return "4.3";
	case BST::AV1::AV1_LEVEL_5:		return "5.0";
	case BST::AV1::AV1_LEVEL_5_1:	return "5.1";
	case BST::AV1::AV1_LEVEL_5_2:	return "5.2";
	case BST::AV1::AV1_LEVEL_5_3:	return "5.3";
	case BST::AV1::AV1_LEVEL_6:		return "6.0";
	case BST::AV1::AV1_LEVEL_6_1:	return "6.1";
	case BST::AV1::AV1_LEVEL_6_2:	return "6.2";
	case BST::AV1::AV1_LEVEL_6_3:	return "6.3";
	case BST::AV1::AV1_LEVEL_7:		return "7.0";
	case BST::AV1::AV1_LEVEL_7_1:	return "7.0";
	case BST::AV1::AV1_LEVEL_7_2:	return "7.0";
	case BST::AV1::AV1_LEVEL_7_3:	return "7.0";
	default:
		break;
	}
	return "Unknown";
}

RET_CODE CreateAV1Context(IAV1Context** ppAV1Ctx, bool bAnnexB, bool bSingleOBUParse)
{
	if (ppAV1Ctx == NULL)
		return RET_CODE_INVALID_PARAMETER;

	auto pCtx = new BST::AV1::VideoBitstreamCtx(bAnnexB, bSingleOBUParse);
	pCtx->AddRef();
	*ppAV1Ctx = (IAV1Context*)pCtx;
	return RET_CODE_SUCCESS;
}

namespace BST
{
	namespace AV1
	{
		OPEN_BITSTREAM_UNIT::FRAME_HEADER_OBU::UNCOMPRESSED_HEADER* OPEN_BITSTREAM_UNIT::FRAME_HEADER_OBU::ptr_last_uncompressed_header = nullptr;

		int OPEN_BITSTREAM_UNIT::FRAME_OBU::Map(AMBst in_bst)
		{
			SYNTAX_BITSTREAM_MAP::Map(in_bst);
			try
			{
				MAP_BST_BEGIN(0);
				int startBitPos = AMBst_Tell(in_bst);
				av1_read_ref(in_bst, ptr_frame_header_obu, FRAME_HEADER_OBU, ptr_OBU);

				AMBst_Realign(in_bst);

				int endBitPos = AMBst_Tell(in_bst);
				int headerBytes = (endBitPos - startBitPos) / 8;

				int sz_tile_group_obu = ptr_OBU->obu_size - headerBytes;

				av1_read_ref(in_bst, ptr_tile_group_obu, OPEN_BITSTREAM_UNIT::TILE_GROUP_OBU, ptr_OBU, sz_tile_group_obu);

				MAP_BST_END();
			}
			catch (AMException e)
			{
				return e.RetCode();
			}

			return RET_CODE_SUCCESS;
		}

		VideoBitstreamCtx::VideoBitstreamCtx(bool bAnnexB, bool bSingleOBUParse) : AnnexB(bAnnexB), SingleOBUParse(bSingleOBUParse) {
			buffer_pool = new BufferPool(this);

			cfbi = -1;

			current_frame_id = output_frame_index = -1;

			memset(VBI, -1, sizeof(VBI));
		}

		VideoBitstreamCtx::~VideoBitstreamCtx() {
			delete buffer_pool;
		}

		HRESULT VideoBitstreamCtx::NonDelegatingQueryInterface(REFIID uuid, void** ppvObj)
		{
			if (ppvObj == NULL)
				return E_POINTER;

			if (uuid == IID_IAV1Context)
				return GetCOMInterface((IAV1Context*)this, ppvObj);

			return CComUnknown::NonDelegatingQueryInterface(uuid, ppvObj);
		}

		RET_CODE VideoBitstreamCtx::SetOBUFilters(std::initializer_list<OBU_FILTER> obu_filters)
		{
			m_obu_filters = obu_filters;
			return RET_CODE_SUCCESS;
		}

		RET_CODE VideoBitstreamCtx::GetOBUFilters(std::vector<OBU_FILTER>& obu_filters)
		{
			obu_filters = m_obu_filters;
			return RET_CODE_SUCCESS;
		}

		bool VideoBitstreamCtx::IsOBUFiltered(OBU_FILTER obu_filter)
		{
			if (m_obu_filters.size() == 0)
				return true;

			for (auto filter : m_obu_filters)
			{
				if (obu_filter.obu_type == filter.obu_type)
				{
					if (obu_filter.obu_extension_flag == filter.obu_extension_flag)
					{
						if (obu_filter.obu_extension_flag == 0)
							return true;
						else if (obu_filter.temporal_id == filter.temporal_id &&
							obu_filter.spatial_id == filter.spatial_id)
							return true;
					}
				}
			}

			return true;
		}

		AV1_OBU VideoBitstreamCtx::GetSeqHdrOBU() {
			return sp_sequence_header;
		}

		RET_CODE VideoBitstreamCtx::UpdateSeqHdrOBU(AV1_OBU seq_hdr_obu)
		{
			if (!seq_hdr_obu)
				return RET_CODE_INVALID_PARAMETER;

			sp_sequence_header = seq_hdr_obu;
			return RET_CODE_SUCCESS;
		}

		AV1_BYTESTREAM_FORMAT VideoBitstreamCtx::GetByteStreamFormat() {
			return AnnexB ? AV1_BYTESTREAM_LENGTH_DELIMITED : AV1_BYTESTREAM_RAW;
		}

		void VideoBitstreamCtx::setup_past_independence()
		{
			auto& cur_frame = buffer_pool->frame_bufs[cfbi];
			for (uint8_t i = 0; i < MAX_SEGMENTS; i++) {
				for (uint8_t j = 0; j < SEG_LVL_MAX; j++) {
					cur_frame.FeatureData[i][j] = 0;
					cur_frame.FeatureEnabled[i][j] = 0;
				}
			}

			for (uint8_t ref = LAST_FRAME; ref <= ALTREF_FRAME; ref++) {
				GmType[ref - LAST_FRAME] = IDENTITY;
				for (uint8_t i = 0; i < 6; i++) {
					PrevGmParams[ref - LAST_FRAME][i] = (i % 3 == 2) ? 1 << WARPEDMODEL_PREC_BITS : 0;
				}
			}

			cur_frame.loop_filter_delta_enabled = true;
			cur_frame.loop_filter_ref_deltas[INTRA_FRAME] = 1;
			cur_frame.loop_filter_ref_deltas[LAST_FRAME] = 0;
			cur_frame.loop_filter_ref_deltas[LAST2_FRAME] = 0;
			cur_frame.loop_filter_ref_deltas[LAST3_FRAME] = 0;
			cur_frame.loop_filter_ref_deltas[BWDREF_FRAME] = 0;
			cur_frame.loop_filter_ref_deltas[GOLDEN_FRAME] = -1;
			cur_frame.loop_filter_ref_deltas[ALTREF_FRAME] = -1;
			cur_frame.loop_filter_ref_deltas[ALTREF2_FRAME] = -1;

			cur_frame.loop_filter_mode_deltas[0] = cur_frame.loop_filter_mode_deltas[1] = 0;
		}

		void VideoBitstreamCtx::load_previous(uint8_t primary_ref_frame)
		{
			if (ref_frame_idx[primary_ref_frame] < 0 || 
				ref_frame_idx[primary_ref_frame] >= NUM_REF_FRAMES)
			{
				printf("Do nothing in call 'load_previous()'\n");
				return;
			}

			int buf_idx = VBI[ref_frame_idx[primary_ref_frame]];
			if (buf_idx < 0 || buf_idx >= FRAME_BUFFERS)
			{
				printf("No available VBI slot for the primary_ref_frame: %s.\n", AV1_REF_FRAME_NAMEA(primary_ref_frame));
				return;
			}

			// The variable prevFrame is set equal to ref_frame_idx[ primary_ref_frame ]
			auto prevFrame = &buffer_pool->frame_bufs[buf_idx];
			auto currFrame = &buffer_pool->frame_bufs[cfbi];

			// PrevGmParams is set equal to SavedGmParams[ prevFrame ]
			memcpy(PrevGmParams, prevFrame->gm_params, sizeof(PrevGmParams));

			// The function load_loop_filter_params( prevFrame ) specified in section 7.21 is invoked
			currFrame->loop_filter_delta_enabled = prevFrame->loop_filter_delta_enabled;
			memcpy(currFrame->loop_filter_ref_deltas, prevFrame->loop_filter_ref_deltas, sizeof(currFrame->loop_filter_ref_deltas));
			memcpy(currFrame->loop_filter_mode_deltas, prevFrame->loop_filter_mode_deltas, sizeof(currFrame->loop_filter_mode_deltas));

			// The function load_segmentation_params( prevFrame ) specified in section 7.21 is invoked
			memcpy(currFrame->FeatureEnabled, prevFrame->FeatureEnabled, sizeof(currFrame->FeatureEnabled));
			memcpy(currFrame->FeatureData, prevFrame->FeatureData, sizeof(currFrame->FeatureData));
		}

		void VideoBitstreamCtx::reset_grain_params()
		{
			assert(cfbi >= 0 && cfbi < FRAME_BUFFERS);
			auto& curr_frame = buffer_pool->frame_bufs[cfbi];
			memset(&curr_frame.film_grain_params, 0, sizeof(curr_frame.film_grain_params));
		}

		int VideoBitstreamCtx::load_grain_params(uint8_t ref_idx)
		{
			assert(cfbi >= 0 && cfbi < FRAME_BUFFERS);
			assert(ref_idx >= 0 && ref_idx < NUM_REF_FRAMES);

			if (ref_idx != cfbi)
			{
				auto& curr_frame = buffer_pool->frame_bufs[cfbi];
				curr_frame.film_grain_params = buffer_pool->frame_bufs[ref_idx].film_grain_params;
			}

			return RET_CODE_SUCCESS;
		}

		int VideoBitstreamCtx::set_frame_refs(int8_t last_frame_idx, int8_t gold_frame_idx, uint8_t OrderHint)
		{
			int iRet = RET_CODE_SUCCESS;
			bool usedFrame[REFS_PER_FRAME] = { 0 };
			int16_t shiftedOrderHints[NUM_REF_FRAMES] = { 0 };

			for (uint8_t i = 0; i < REFS_PER_FRAME; i++){
				ref_frame_idx[i] = -1;
			}

			ref_frame_idx[LAST_FRAME - LAST_FRAME] = last_frame_idx;
			ref_frame_idx[GOLDEN_FRAME - LAST_FRAME] = gold_frame_idx;

			usedFrame[last_frame_idx] = true;
			usedFrame[gold_frame_idx] = true;

			uint8_t curFrameHint = (uint8_t)(1 << (sp_sequence_header->ptr_sequence_header_obu->OrderHintBits - 1));
			for (uint8_t i = 0; i < NUM_REF_FRAMES; i++) {
				int RefOrderHint = 0;
				if (VBI[i] >= 0)
					RefOrderHint = buffer_pool->frame_bufs[VBI[i]].current_order_hint;
				shiftedOrderHints[i] = curFrameHint +
					sp_sequence_header->ptr_sequence_header_obu->get_relative_dist(RefOrderHint, OrderHint);
			}

			int16_t lastOrderHint = shiftedOrderHints[last_frame_idx];
			int16_t goldOrderHint = shiftedOrderHints[gold_frame_idx];

			int16_t ref = find_latest_backward(shiftedOrderHints, usedFrame, curFrameHint);

			if (ref >= 0) {
				ref_frame_idx[ALTREF_FRAME - LAST_FRAME] = (int8_t)ref;
				usedFrame[ref] = true;
			}

			ref = find_earliest_backward(shiftedOrderHints, usedFrame, curFrameHint);
			if (ref >= 0) {
				ref_frame_idx[BWDREF_FRAME - LAST_FRAME] = (int8_t)ref;
				usedFrame[ref] = true;
			}

			ref = find_earliest_backward(shiftedOrderHints, usedFrame, curFrameHint);
			if (ref >= 0) {
				ref_frame_idx[ALTREF2_FRAME - LAST_FRAME] = (int8_t)ref;
				usedFrame[ref] = true;
			}

			for (uint8_t i = 0; i < REFS_PER_FRAME - 2; i++) {
				auto refFrame = Ref_Frame_List[i];
				if (ref_frame_idx[refFrame - LAST_FRAME] < 0) {
					auto ref = find_latest_forward(shiftedOrderHints, usedFrame, curFrameHint);
					if (ref >= 0) {
						ref_frame_idx[refFrame - LAST_FRAME] = (int8_t)ref;
						usedFrame[ref] = true;
					}
				}
			}

			ref = -1;
			int16_t earliestOrderHint = INT8_MAX;
			for (int8_t i = 0; i < NUM_REF_FRAMES; i++) {
				auto hint = shiftedOrderHints[i];
				if (ref < 0 || hint < earliestOrderHint) {
					ref = i;
					earliestOrderHint = hint;
				}
			}
			for (int8_t i = 0; i < REFS_PER_FRAME; i++) {
				if (ref_frame_idx[i] < 0) {
					ref_frame_idx[i] = (int8_t)ref;
				}
			}

			return iRet;
		}

		void VideoBitstreamCtx::Reset()
		{
			SeenFrameHeader = false;

			cfbi = -1;

			current_frame_id = output_frame_index = -1;

			memset(VBI, -1, sizeof(VBI));

			if (buffer_pool)
				buffer_pool->Reset();
		}

		RET_CODE VideoBitstreamCtx::LoadSnapshot(AV1ContextSnapshot* ptr_ctx_snapshot)
		{
			int iRet = RET_CODE_SUCCESS;

			if (ptr_ctx_snapshot == nullptr || 
				ptr_ctx_snapshot->pVBISlotParams == nullptr ||
				ptr_ctx_snapshot->pActiveFrameParams == nullptr)
				return RET_CODE_INVALID_PARAMETER;

			tu_idx = ptr_ctx_snapshot->tu_idx;
			tu_fu_idx = ptr_ctx_snapshot->tu_fu_idx;

			VBI_SLOT_PARAMS* pVBISlotParams = (VBI_SLOT_PARAMS*)ptr_ctx_snapshot->pVBISlotParams;

			// At first, reset the current buffer pool
			if (buffer_pool == nullptr)
				buffer_pool = new BufferPool(this);
			else
				buffer_pool->Reset();

			// Reset the VBI and next_VBI
			for (uint8_t i = 0; i < NUM_REF_FRAMES; i++) {
				VBI[i] = -1;
			}

			if (g_verbose_level > 200)
			{
				printf("VBI FrameSeqId: [%d,%d,%d,%d,%d,%d,%d,%d]\n",
					pVBISlotParams[0].FrameSeqID, pVBISlotParams[1].FrameSeqID,
					pVBISlotParams[2].FrameSeqID, pVBISlotParams[3].FrameSeqID,
					pVBISlotParams[4].FrameSeqID, pVBISlotParams[5].FrameSeqID,
					pVBISlotParams[6].FrameSeqID, pVBISlotParams[7].FrameSeqID);
			}

			std::map<uint16_t, int8_t> FrameSeqBufIdx;
			int8_t RestoredVBI[NUM_REF_FRAMES] = { -1, -1, -1, -1, -1, -1, -1, -1 };
			for (uint8_t i = 0; i < NUM_REF_FRAMES; i++)
			{
				if (pVBISlotParams[i].FrameSeqID < 0)
					continue;

				auto iter = FrameSeqBufIdx.find(pVBISlotParams[i].FrameSeqID);
				if (iter == FrameSeqBufIdx.end())
				{
					int8_t buf_idx = buffer_pool->get_free_fb();
					FrameSeqBufIdx[pVBISlotParams[i].FrameSeqID] = buf_idx;
					RestoredVBI[i] = buf_idx;

					auto& cur_frame = buffer_pool->frame_bufs[buf_idx];
					cur_frame.valid_for_referencing = pVBISlotParams[i].RefValid;

					if (pVBISlotParams[i].RefFrameType < KEY_FRAME || pVBISlotParams[i].RefFrameType > SWITCHABLE)
					{
						printf("[AV1] The loaded RefFrameType[%d](%d) is invalid!\n", i, pVBISlotParams[i].RefFrameType);
						iRet = RET_CODE_ERROR;
						goto done;
					}

					cur_frame.LoadVBISlotParams(pVBISlotParams[i]);
				}
				else
					RestoredVBI[i] = iter->second;
			}

			memcpy(VBI, RestoredVBI, sizeof(VBI));

			if (g_verbose_level > 200)
			{
				printf("refresh_frame_flags: 0x%X\n", ptr_ctx_snapshot->pActiveFrameParams->refresh_frame_flags);
				printf("VBI: [%d, %d, %d, %d, %d, %d, %d, %d]\n", VBI[0], VBI[1], VBI[2], VBI[3], VBI[4], VBI[5], VBI[6], VBI[7]);
				printf("RefOrderHint: [%d, %d, %d, %d, %d, %d, %d, %d]\n", pVBISlotParams[0].RefOrderHint, pVBISlotParams[1].RefOrderHint
					, pVBISlotParams[2].RefOrderHint, pVBISlotParams[3].RefOrderHint, pVBISlotParams[4].RefOrderHint
					, pVBISlotParams[5].RefOrderHint, pVBISlotParams[6].RefOrderHint, pVBISlotParams[7].RefOrderHint);
			}

			// Second, apply the active frame parameters
			if (ptr_ctx_snapshot->pActiveFrameParams->FrameSeqID == -1)
			{
				cfbi = -1;
				//printf("cfbi: %d.\n", cfbi);
			}
			else
			{
				auto iter_cfbi = FrameSeqBufIdx.find(ptr_ctx_snapshot->pActiveFrameParams->FrameSeqID);
				if (iter_cfbi != FrameSeqBufIdx.end()){
					cfbi = iter_cfbi->second;
					//printf("cfbi: %d.\n", cfbi);
				}
				else
				{
					// Allocate a new frame buffer from buffer pool
					int8_t buf_idx = buffer_pool->get_free_fb();
					assert(buf_idx >= 0);

					cfbi = buf_idx;
					auto cur_frame = buffer_pool->frame_bufs[buf_idx];
					VBI_SLOT_PARAMS VBI_slot_params;
					FillVBISlotParamsWithActiveFrameParams(ptr_ctx_snapshot->pActiveFrameParams, VBI_slot_params);
					//printf("Load cfbi[buf_idx: %d] slot params.\n", buf_idx);
					cur_frame.LoadVBISlotParams(VBI_slot_params);
				}
			}

			SeenFrameHeader = ptr_ctx_snapshot->pActiveFrameParams->SeenFrameHeader;
			show_existing_frame = ptr_ctx_snapshot->pActiveFrameParams->show_existing_frame;
			show_frame = ptr_ctx_snapshot->pActiveFrameParams->show_frame;
			refresh_frame_flags = ptr_ctx_snapshot->pActiveFrameParams->refresh_frame_flags;
			TileNum = ptr_ctx_snapshot->pActiveFrameParams->TileNum;
			memcpy(PrevGmParams, ptr_ctx_snapshot->pActiveFrameParams->PrevGmParams, sizeof(PrevGmParams));

			iRet = RET_CODE_SUCCESS;
		done:
			return iRet;
		}

		int VideoBitstreamCtx::reference_frame_update()
		{
			int ref_index = 0;
			auto pool = buffer_pool;
			auto frame_bufs = pool->frame_bufs;
			bool bVBISlotChanged[NUM_REF_FRAMES] = { false, false, false, false, false, false, false, false };

			for (uint8_t i = 0; i < NUM_REF_FRAMES; i++)
			{
				if (refresh_frame_flags & (1 << i) && VBI[i] != cfbi)
				{
					if (VBI[i] != -1)
						buffer_pool->decrease_ref_count(VBI[i]);

					VBI[i] = cfbi;
					buffer_pool->frame_bufs[VBI[i]].ref_count++;

					bVBISlotChanged[i] = true;
				}
			}

			// Invalidate these references until the next frame starts.
			for (ref_index = 0; ref_index < REFS_PER_FRAME; ref_index++) {
				ref_frame_idx[ref_index] = -1;
			}

			if (cfbi >= 0)
				buffer_pool->decrease_ref_count(cfbi);

			if (g_verbose_level > 99)
			{
				// drc: decoder reference count
				printf("VBI: [%s%d(drc: %d), %s%d(drc: %d), %s%d(drc: %d), %s%d(drc: %d), %s%d(drc: %d), %s%d(drc: %d), %s%d(drc: %d), %s%d(drc: %d)]\n",
					bVBISlotChanged[0] ? "*" : " ", VBI[0], VBI[0] >= 0 && VBI[0] < NUM_REF_FRAMES ? frame_bufs[VBI[0]].ref_count : 0,
					bVBISlotChanged[1] ? "*" : " ", VBI[1], VBI[1] >= 0 && VBI[1] < NUM_REF_FRAMES ? frame_bufs[VBI[1]].ref_count : 0,
					bVBISlotChanged[2] ? "*" : " ", VBI[2], VBI[2] >= 0 && VBI[2] < NUM_REF_FRAMES ? frame_bufs[VBI[2]].ref_count : 0,
					bVBISlotChanged[3] ? "*" : " ", VBI[3], VBI[3] >= 0 && VBI[3] < NUM_REF_FRAMES ? frame_bufs[VBI[3]].ref_count : 0,
					bVBISlotChanged[4] ? "*" : " ", VBI[4], VBI[4] >= 0 && VBI[4] < NUM_REF_FRAMES ? frame_bufs[VBI[4]].ref_count : 0,
					bVBISlotChanged[5] ? "*" : " ", VBI[5], VBI[5] >= 0 && VBI[5] < NUM_REF_FRAMES ? frame_bufs[VBI[5]].ref_count : 0,
					bVBISlotChanged[6] ? "*" : " ", VBI[6], VBI[6] >= 0 && VBI[6] < NUM_REF_FRAMES ? frame_bufs[VBI[6]].ref_count : 0,
					bVBISlotChanged[7] ? "*" : " ", VBI[7], VBI[7] >= 0 && VBI[7] < NUM_REF_FRAMES ? frame_bufs[VBI[7]].ref_count : 0);
			}

			return RET_CODE_SUCCESS;
		}

		void VideoBitstreamCtx::av1_setup_frame_buf_refs(uint32_t order_hint)
		{
			auto cur_frame = &buffer_pool->frame_bufs[cfbi];

			// Update the order hint of the current decoding frame
			cur_frame->current_order_hint = order_hint;

			// Update the order hint of the frames which current frame depends on to do prediction
			for (int ref_frame = LAST_FRAME; ref_frame <= ALTREF_FRAME; ++ref_frame) {
				const int buf_idx = VBI[ref_frame_idx[ref_frame - LAST_FRAME]];
				if (buf_idx >= 0)
					cur_frame->ref_order_hints[ref_frame - LAST_FRAME] =
						buffer_pool->frame_bufs[buf_idx].current_order_hint;
			}
		}

		int RefCntBuffer::LoadVBISlotParams(const VBI_SLOT_PARAMS& VBI_slot_params)
		{
			frame_type = (AV1_FRAME_TYPE)VBI_slot_params.RefFrameType;
			current_frame_id = VBI_slot_params.RefFrameId;
			upscaled_width = VBI_slot_params.RefUpscaledWidth;
			width = VBI_slot_params.RefFrameWidth;
			height = VBI_slot_params.RefFrameHeight;
			render_width = VBI_slot_params.RefRenderWidth;
			render_height = VBI_slot_params.RefRenderHeight;
			mi_cols = VBI_slot_params.RefMiCols;
			mi_rows = VBI_slot_params.RefMiRows;
			current_order_hint = VBI_slot_params.RefOrderHint;
			memcpy(ref_order_hints, VBI_slot_params.SavedOrderHints, sizeof(ref_order_hints));
			memcpy(gm_params, VBI_slot_params.SavedGmParams, sizeof(gm_params));
			memcpy(loop_filter_ref_deltas, VBI_slot_params.loop_filter_ref_deltas, sizeof(loop_filter_ref_deltas));
			memcpy(loop_filter_mode_deltas, VBI_slot_params.loop_filter_mode_deltas, sizeof(loop_filter_mode_deltas));
			memcpy(FeatureEnabled, VBI_slot_params.FeatureEnabled, sizeof(FeatureEnabled));
			memcpy(FeatureData, VBI_slot_params.FeatureData, sizeof(FeatureData));

			return RET_CODE_SUCCESS;
		}

		BufferPool::BufferPool(VideoBitstreamCtx* pCtx) : ctx_video_bst(pCtx) {
		}

		void BufferPool::ref_cnt_fb(int8_t *idx, int new_idx)
		{
			const int ref_index = *idx;

			if (ref_index >= 0 && frame_bufs[ref_index].ref_count > 0)
			{
				frame_bufs[ref_index].ref_count--;
			}

			*idx = new_idx;

			frame_bufs[new_idx].ref_count++;
		}

		void BufferPool::decrease_ref_count(int8_t idx)
		{
			if (idx >= 0) {
				--frame_bufs[idx].ref_count;
				if (frame_bufs[idx].ref_count == 0) {
					if (g_verbose_level > 0)
					{
						printf("[AV1] Release the frame buffer #%d.\n", idx);
						for (uint8_t i = 0; i < NUM_REF_FRAMES; i++)
							if (ctx_video_bst->VBI[i] == idx)
								ctx_video_bst->VBI[i] = -1;
					}
				}
			}
		}

		/*
			The allocated frame buffer should NOT lies in the VBI
		*/
		int8_t BufferPool::get_free_fb()
		{
			int i;

			for (i = 0; i < FRAME_BUFFERS; ++i)
				if (frame_bufs[i].ref_count == 0)
					break;

			if (i != FRAME_BUFFERS) {
				frame_bufs[i].ref_count = 1;
			}
			else {
				// Reset i to be INVALID_IDX to indicate no free buffer found.
				printf("[AV1] No available free frame buffer!!\n");
				i = -1;
			}

			return i;
		}

		void BufferPool::Reset() {
			for (size_t i = 0; i < _countof(frame_bufs); i++) {
				frame_bufs[i].Reset();
			}
		}
	}
}