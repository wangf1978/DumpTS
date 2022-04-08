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

		OPEN_BITSTREAM_UNIT::SEQUENCE_HEADER_OBU* OPEN_BITSTREAM_UNIT::get_sequence_header_obu()
		{
			BST::AV1::OPEN_BITSTREAM_UNIT* ptr_active_obu =
#if 0
				(BST::AV1::OPEN_BITSTREAM_UNIT*)AMTLS_GetEnvPointer(_T("BST_CONTAINER_AV1_ACTIVE_SEQHDR"), nullptr);
#else
				nullptr;
#endif

			if (ptr_active_obu == nullptr)
				ptr_active_obu = ctx_video_bst->sp_sequence_header.get();

			if (ptr_active_obu != nullptr && ptr_active_obu->obu_header.obu_type == OBU_SEQUENCE_HEADER)
				return ptr_active_obu->ptr_sequence_header_obu;

			return nullptr;
		}

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
			camera_frame_header_ready = false;

			buffer_pool = new BufferPool();

			cfbi = -1;

			current_frame_id = output_frame_index = -1;

			memset(VBI, -1, sizeof(VBI));
			memset(next_VBI, -1, sizeof(next_VBI));

			need_resync = true;

			tu_fu_idx = -1;

			memset(&film_grain_params, 0, sizeof(film_grain_params));
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
			for (uint8_t i = 0; i < MAX_SEGMENTS; i++) {
				for (uint8_t j = 0; j < SEG_LVL_MAX; j++) {
					FeatureData[i][j] = 0;
					FeatureEnabled[i][j] = 0;
				}
			}

			for (uint8_t ref = LAST_FRAME; ref <= ALTREF_FRAME; ref++) {
				GmType[ref - LAST_FRAME] = IDENTITY;
				for (uint8_t i = 0; i < 6; i++) {
					PrevGmParams[ref - LAST_FRAME][i] = (i % 3 == 2) ? 1 << WARPEDMODEL_PREC_BITS : 0;
				}
			}

			loop_filter_delta_enabled = true;
			loop_filter_ref_deltas[INTRA_FRAME] = 1;
			loop_filter_ref_deltas[LAST_FRAME] = 0;
			loop_filter_ref_deltas[LAST2_FRAME] = 0;
			loop_filter_ref_deltas[LAST3_FRAME] = 0;
			loop_filter_ref_deltas[BWDREF_FRAME] = 0;
			loop_filter_ref_deltas[GOLDEN_FRAME] = -1;
			loop_filter_ref_deltas[ALTREF_FRAME] = -1;
			loop_filter_ref_deltas[ALTREF2_FRAME] = -1;

			loop_filter_mode_deltas[0] = loop_filter_mode_deltas[1] = 0;
		}

		void VideoBitstreamCtx::load_previous(uint8_t primary_ref_frame)
		{
			if (frame_refs[primary_ref_frame].map_idx < 0 || 
				frame_refs[primary_ref_frame].map_idx >= NUM_REF_FRAMES)
			{
				printf("Do nothing in call 'load_previous()'\n");
				return;
			}

			int buf_idx = VBI[frame_refs[primary_ref_frame].map_idx];
			if (buf_idx < 0 || buf_idx >= FRAME_BUFFERS)
			{
				printf("No available VBI slot for the primary_ref_frame: %s.\n", AV1_REF_FRAME_NAMEA(primary_ref_frame));
				return;
			}

			// The variable prevFrame is set equal to ref_frame_idx[ primary_ref_frame ]
			auto prevFrame = &buffer_pool->frame_bufs[buf_idx];

			// PrevGmParams is set equal to SavedGmParams[ prevFrame ]
			memcpy(PrevGmParams, prevFrame->gm_params, sizeof(PrevGmParams));

			// The function load_loop_filter_params( prevFrame ) specified in section 7.21 is invoked
			memcpy(loop_filter_ref_deltas, prevFrame->loop_filter_ref_deltas, sizeof(loop_filter_ref_deltas));
			memcpy(loop_filter_mode_deltas, prevFrame->loop_filter_mode_deltas, sizeof(loop_filter_mode_deltas));

			// The function load_segmentation_params( prevFrame ) specified in section 7.21 is invoked
			memcpy(FeatureEnabled, prevFrame->FeatureEnabled, sizeof(FeatureEnabled));
			memcpy(FeatureData, prevFrame->FeatureData, sizeof(FeatureData));
		}

		void VideoBitstreamCtx::reset_grain_params()
		{
			memset(&film_grain_params, 0, sizeof(film_grain_params));
		}

		int VideoBitstreamCtx::load_grain_params(uint8_t ref_idx)
		{
			if (buffer_pool == nullptr || ref_idx < 0)
				return RET_CODE_ERROR;

			film_grain_params = buffer_pool->frame_bufs[ref_idx].film_grain_params;

			return RET_CODE_SUCCESS;
		}

		int VideoBitstreamCtx::set_frame_refs(int8_t last_frame_idx, int8_t gold_frame_idx, uint8_t OrderHint)
		{
			int iRet = RET_CODE_SUCCESS;
			bool usedFrame[REFS_PER_FRAME] = { 0 };
			int16_t shiftedOrderHints[NUM_REF_FRAMES] = { 0 };

			for (uint8_t i = 0; i < REFS_PER_FRAME; i++){
				frame_refs[i].idx = frame_refs[i].map_idx = -1;
			}

			frame_refs[LAST_FRAME - LAST_FRAME].map_idx = last_frame_idx;
			frame_refs[LAST_FRAME - LAST_FRAME].idx = VBI[last_frame_idx];
			frame_refs[GOLDEN_FRAME - LAST_FRAME].map_idx = gold_frame_idx;
			frame_refs[GOLDEN_FRAME - LAST_FRAME].idx = VBI[gold_frame_idx];

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
				frame_refs[ALTREF_FRAME - LAST_FRAME].map_idx = (int8_t)ref;
				frame_refs[ALTREF_FRAME - LAST_FRAME].idx = VBI[ref];
				usedFrame[ref] = true;
			}

			ref = find_earliest_backward(shiftedOrderHints, usedFrame, curFrameHint);
			if (ref >= 0) {
				frame_refs[BWDREF_FRAME - LAST_FRAME].map_idx = (int8_t)ref;
				frame_refs[BWDREF_FRAME - LAST_FRAME].idx = VBI[ref];
				usedFrame[ref] = true;
			}

			ref = find_earliest_backward(shiftedOrderHints, usedFrame, curFrameHint);
			if (ref >= 0) {
				frame_refs[ALTREF2_FRAME - LAST_FRAME].map_idx = (int8_t)ref;
				frame_refs[ALTREF2_FRAME - LAST_FRAME].idx = VBI[ref];
				usedFrame[ref] = true;
			}

			for (uint8_t i = 0; i < REFS_PER_FRAME - 2; i++) {
				auto refFrame = Ref_Frame_List[i];
				if (frame_refs[refFrame - LAST_FRAME].map_idx < 0) {
					auto ref = find_latest_forward(shiftedOrderHints, usedFrame, curFrameHint);
					if (ref >= 0) {
						frame_refs[refFrame - LAST_FRAME].map_idx = (int8_t)ref;
						frame_refs[refFrame - LAST_FRAME].idx = VBI[ref];
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
				if (frame_refs[i].map_idx < 0) {
					frame_refs[i].map_idx = (int8_t)ref;
					frame_refs[i].idx = VBI[ref];
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
			memset(next_VBI, -1, sizeof(next_VBI));

			if (buffer_pool)
				buffer_pool->Reset();

			need_resync = true;
		}

		RET_CODE VideoBitstreamCtx::LoadVBISnapshot(void* VBIsnapshot, int cbSize)
		{
			int iRet = RET_CODE_SUCCESS;
			VBISlotParams* pVBISlotParams = (VBISlotParams*)VBIsnapshot;
			if (cbSize / sizeof(VBISlotParams) < NUM_REF_FRAMES)
			{
				printf("[AV1] The VBI snapshot size is too small.\n");
				return RET_CODE_INVALID_PARAMETER;
			}

			// At first, reset the current buffer pool
			if (buffer_pool == nullptr)
				buffer_pool = new BufferPool();
			else
				buffer_pool->Reset();

			cfbi = -1;

			// Reset the VBI and next_VBI
			for (uint8_t i = 0; i < NUM_REF_FRAMES; i++) {
				VBI[i] = next_VBI[i] = -1;
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

					auto cur_frame = buffer_pool->frame_bufs[buf_idx];
					cur_frame.valid_for_referencing = pVBISlotParams[i].RefValid;

					if (pVBISlotParams[i].RefFrameType < KEY_FRAME || pVBISlotParams[i].RefFrameType > SWITCHABLE)
					{
						printf("[AV1] The loaded RefFrameType[%d](%d) is invalid!\n", i, pVBISlotParams[i].RefFrameType);
						iRet = RET_CODE_ERROR;
						goto done;
					}

					cur_frame.frame_type		= (AV1_FRAME_TYPE)pVBISlotParams[i].RefFrameType;
					cur_frame.current_frame_id	= pVBISlotParams[i].RefFrameId;
					cur_frame.upscaled_width	= pVBISlotParams[i].RefUpscaledWidth;
					cur_frame.width				= pVBISlotParams[i].RefFrameWidth;
					cur_frame.height			= pVBISlotParams[i].RefFrameHeight;
					cur_frame.render_width		= pVBISlotParams[i].RefRenderWidth;
					cur_frame.render_height		= pVBISlotParams[i].RefRenderHeight;
					cur_frame.mi_cols			= pVBISlotParams[i].RefMiCols;
					cur_frame.mi_rows			= pVBISlotParams[i].RefMiRows;
					cur_frame.current_order_hint= pVBISlotParams[i].RefOrderHint;
					memcpy(cur_frame.gm_params, pVBISlotParams[i].SavedGmParams, sizeof(cur_frame.gm_params));
					memcpy(cur_frame.loop_filter_ref_deltas, pVBISlotParams[i].loop_filter_ref_deltas, sizeof(cur_frame.loop_filter_ref_deltas));
					memcpy(cur_frame.loop_filter_mode_deltas, pVBISlotParams[i].loop_filter_mode_deltas, sizeof(cur_frame.loop_filter_mode_deltas));
					memcpy(cur_frame.FeatureEnabled, pVBISlotParams[i].FeatureEnabled, sizeof(cur_frame.FeatureEnabled));
					memcpy(cur_frame.FeatureData, pVBISlotParams[i].FeatureData, sizeof(cur_frame.FeatureData));
				}
				else
					RestoredVBI[i] = iter->second;
			}

		done:
			return iRet;
		}

		int VideoBitstreamCtx::reference_frame_update()
		{
			int ref_index = 0, mask;
			auto pool = buffer_pool;
			auto frame_bufs = pool->frame_bufs;

			// In ext-tile decoding, the camera frame header is only decoded once. So,
			// we don't release the references here.
			if (!camera_frame_header_ready)
			{
				for (mask = refresh_frame_flags; mask; mask >>= 1)
				{
					const int old_idx = VBI[ref_index];
					// Current thread releases the holding of reference frame.
					buffer_pool->decrease_ref_count(old_idx);

					// Release the reference frame holding in the reference map for the
					// decoding of the next frame.
					if (mask & 1) buffer_pool->decrease_ref_count(old_idx);
					VBI[ref_index] = next_VBI[ref_index];
					++ref_index;
				}

				// Current thread releases the holding of reference frame.
				const int check_on_show_existing_frame = !show_existing_frame || reset_decoder_state;
				for (; ref_index < NUM_REF_FRAMES && check_on_show_existing_frame; ++ref_index) {
					const int old_idx = VBI[ref_index];
					buffer_pool->decrease_ref_count(old_idx);
					VBI[ref_index] = next_VBI[ref_index];
				}
			}

			if (show_existing_frame || show_frame)
			{
				if (output_frame_index >= 0)
					buffer_pool->decrease_ref_count(output_frame_index);

				output_frame_index = cfbi;
			}
			else
			{
				buffer_pool->decrease_ref_count(cfbi);
			}

			if (!camera_frame_header_ready) {
				hold_ref_buf = false;

				// Invalidate these references until the next frame starts.
				for (ref_index = 0; ref_index < REFS_PER_FRAME; ref_index++) {
					frame_refs[ref_index].idx = -1;
				}
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
				const int buf_idx = frame_refs[ref_frame - LAST_FRAME].idx;
				if (buf_idx >= 0)
					cur_frame->ref_order_hints[ref_frame - LAST_FRAME] =
					buffer_pool->frame_bufs[buf_idx].current_order_hint;
			}
		}
	}
}