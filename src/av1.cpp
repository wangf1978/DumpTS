#include "platcomm.h"
#include "av1.h"

const char* obu_type_names[] = {
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

const WarpedMotionParams default_warp_params = {
	IDENTITY,
	{ 0, 0, (1 << WARPEDMODEL_PREC_BITS), 0, 0, (1 << WARPEDMODEL_PREC_BITS), 0,
	0 },
	0, 0, 0, 0,
	0,
};

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

		void TEMPORAL_UNIT::SwapFrameBuffer()
		{
			int ref_index = 0, mask;
			auto pool = ctx_video_bst.buffer_pool;
			auto frame_bufs = ctx_video_bst.buffer_pool->frame_bufs;

			// In ext-tile decoding, the camera frame header is only decoded once. So,
			// we don't release the references here.
			if (!ctx_video_bst.camera_frame_header_ready) 
			{
				for (mask = ctx_video_bst.refresh_frame_flags; mask; mask >>= 1) 
				{
					const int old_idx = ctx_video_bst.ref_frame_map[ref_index];
					// Current thread releases the holding of reference frame.
					decrease_ref_count(old_idx, frame_bufs, pool);

					// Release the reference frame holding in the reference map for the
					// decoding of the next frame.
					if (mask & 1) decrease_ref_count(old_idx, frame_bufs, pool);
					ctx_video_bst.ref_frame_map[ref_index] = ctx_video_bst.next_ref_frame_map[ref_index];
					++ref_index;
				}

				// Current thread releases the holding of reference frame.
				const int check_on_show_existing_frame = !ctx_video_bst.show_existing_frame || ctx_video_bst.reset_decoder_state;
				for (; ref_index < NUM_REF_FRAMES && check_on_show_existing_frame;++ref_index) {
					const int old_idx = ctx_video_bst.ref_frame_map[ref_index];
					decrease_ref_count(old_idx, frame_bufs, pool);
					ctx_video_bst.ref_frame_map[ref_index] = ctx_video_bst.next_ref_frame_map[ref_index];
				}
			}

			if (ctx_video_bst.show_existing_frame || ctx_video_bst.show_frame)
			{
				if (ctx_video_bst.output_frame_index >= 0)
					decrease_ref_count(ctx_video_bst.output_frame_index, frame_bufs, pool);
				
				ctx_video_bst.output_frame_index = ctx_video_bst.new_fb_idx;
			}
			else
			{
				decrease_ref_count(ctx_video_bst.new_fb_idx, frame_bufs, pool);
			}

			if (!ctx_video_bst.camera_frame_header_ready) {
				ctx_video_bst.hold_ref_buf = false;

				// Invalidate these references until the next frame starts.
				for (ref_index = 0; ref_index < REFS_PER_FRAME; ref_index++) {
					ctx_video_bst.frame_refs[ref_index].idx = -1;
				}
			}
		}
	}
}