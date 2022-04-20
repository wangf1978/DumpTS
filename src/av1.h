/*

MIT License

Copyright (c) 2022 Ravin.Wang(wangf1978@hotmail.com)

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
#ifndef __AV1_H__
#define __AV1_H__

#pragma once

#include "AMBitStream.h"
#include "DumpUtil.h"
#include "AMException.h"
#include "AMArray.h"
#include <vector>
#include <unordered_map>
#include "av1_def.h"
#include "ISO14496_12.h"
#include "av1_parser.h"

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

#define Abs(x)						((x)>=0?(x):-(x))
#define AV1_Clip3(x,y,z)			((z)<(x)?(x):((z) > (y)?(y):(z)))
#define AV1_Clip1(x, bitdepth)		AV1_Clip3(0, (1<<bitdepth)-1, x)
#define Min(x, y)					((x) <= (y)?(x):(y))
#define Max(x, y)					((x) >= (y)?(x):(y))
#define Round2(x, n)				((x + (1<<(n-1)))/(1<<n))
#define Round2Signed(x, n)			((x) >= 0?Round2(x, n):Round2(-x, n))

#define OBU_SEQUENCE_HEADER			1
#define OBU_TEMPORAL_DELIMITER		2
#define OBU_FRAME_HEADER			3
#define OBU_TILE_GROUP				4
#define OBU_METADATA				5
#define OBU_FRAME					6
#define OBU_REDUNDANT_FRAME_HEADER	7
#define OBU_TILE_LIST				8
#define OBU_PADDING					15

#define CP_BT_709					1
#define CP_UNSPECIFIED				2
#define CP_BT_470_M					4
#define CP_BT_470_B_G				5
#define CP_BT_601					6
#define CP_SMPTE_240				7
#define CP_GENERIC_FILM				8
#define CP_BT_2020					9
#define CP_XYZ						10
#define CP_SMPTE_431				11
#define CP_SMPTE_432				12
#define CP_EBU_3213					22

#define TC_RESERVED_0				0
#define TC_BT_709					1
#define TC_UNSPECIFIED				2
#define TC_RESERVED_3				3
#define TC_BT_470_M					4
#define TC_BT_470_B_G				5
#define TC_BT_601					6
#define TC_SMPTE_240				7
#define TC_LINEAR					8
#define TC_LOG_100					9
#define TC_LOG_100_SQRT10			10
#define TC_IEC_61966				11
#define TC_BT_1361					12
#define TC_SRGB						13
#define TC_BT_2020_10_BIT			14
#define TC_BT_2020_12_BIT			15
#define TC_SMPTE_2084				16
#define TC_SMPTE_428				17
#define TC_HLG						18

#define MC_IDENTITY					0
#define MC_BT_709					1
#define MC_UNSPECIFIED				2
#define MC_RESERVED_3				3
#define MC_FCC						4
#define MC_BT_470_B_G				5
#define MC_BT_601					6
#define MC_SMPTE_240				7
#define MC_SMPTE_YCGCO				8
#define MC_BT_2020_NCL				9
#define MC_BT_2020_CL				10
#define MC_SMPTE_2085				11
#define MC_CHROMAT_NCL				12
#define MC_CHROMAT_CL				13
#define MC_ICTCP					14

#define CSP_UNKNOWN					0	// Unknown(in this case the source video transfer function must be signaled outside the AV1 bitstream)
#define CSP_VERTICAL				1	// Horizontally co - located with(0, 0) luma sample, vertical position in the middle between two luma samples
#define CSP_COLOCATED				2	// co-located with(0, 0) luma sample
#define CSP_RESERVED				3

#define IS_OBU_FRAME(obu_type)		(obu_type == OBU_FRAME || obu_type == OBU_FRAME_HEADER || obu_type == OBU_REDUNDANT_FRAME_HEADER)

auto constexpr GetFrameRestorationTypeName(uint8_t type) {
	return (type == 0 ? "NONE" : 
		   (type == 1 ? "WIENER" : 
		   (type == 2 ? "SGRPROJ" : 
		   (type == 3 ? "SWITCHABLE" : "Unknown"))));
}

#define TX_MODE_DESC(x)	(\
	(x) == ONLY_4X4?"ONLY_4X4, the inverse transform will use only 4x4 transforms":(\
	(x) == TX_MODE_LARGEST?"TX_MODE_SELECT, the choice of transform size is specified explicitly for each block":(\
	(x) == TX_MODE_SELECT?"TX_MODE_LARGEST, the inverse transform will use the largest transform size that fits inside the block":"Unknown")))

#define GLOBAL_MOTION_TYPE_NAME(GmType)	(\
	(GmType) == IDENTITY?"IDENTITY":(\
	(GmType) == TRANSLATION?"TRANSLATION":(\
	(GmType) == ROTZOOM?"ROTZOOM":(\
	(GmType) == AFFINE?"AFFINE":"Unknown"))))

extern const char* obu_type_names[16];
extern const char* obu_type_short_names[16];
extern const char* color_primaries_descs[23];
extern const char* transfer_characteristics_descs[19];
extern const char* matrix_coefficients_descs[15];
extern const char* chroma_sample_position_descs[4];
extern const uint8_t Segmentation_Feature_Bits[SEG_LVL_MAX];
extern const uint8_t Segmentation_Feature_Signed[SEG_LVL_MAX];
extern const uint8_t Segmentation_Feature_Max[SEG_LVL_MAX];
extern const uint8_t Remap_Lr_Type[4];
extern const uint8_t Ref_Frame_List[REFS_PER_FRAME - 2];
extern const WarpedMotionParams default_warp_params;
extern const char* get_av1_profile_name(int profile);
extern const char* get_av1_level_name(int level);

extern RET_CODE CreateAV1Context(IAV1Context** ppAV1Ctx, bool bAnnexB, bool bSingleOBUParse);

#define METADATA_TYPE_NAME(t)	(\
	(t) == METADATA_TYPE_HDR_CLL?"METADATA_TYPE_HDR_CLL":(\
	(t) == METADATA_TYPE_HDR_MDCV?"METADATA_TYPE_HDR_MDCV":(\
	(t) == METADATA_TYPE_SCALABILITY?"METADATA_TYPE_SCALABILITY":(\
	(t) == METADATA_TYPE_ITUT_T35?"METADATA_TYPE_ITUT_T35":(\
	(t) == METADATA_TYPE_TIMECODE?"METADATA_TYPE_TIMECODE":(\
	(t) == 0?"Reserved for AOM use":(\
	((t) >= 6 && (t) <= 31)?"Unregistered user private":"Unknown")))))))

namespace BST
{
	namespace AV1
	{
		enum AV1_PROFILE
		{
			AV1_PROFILE_UNKNOWN = -1,
			AV1_PROFILE_MAIN,
			AV1_PROFILE_HIGH,
			AV1_PROFILE_PROFESSIONAL,
		};

		enum AV1_LEVEL
		{
			AV1_LEVEL_UNKNOWN	= -1,
			AV1_LEVEL_2			= 0,
			AV1_LEVEL_2_1		= 1,
			AV1_LEVEL_2_2		= 2,
			AV1_LEVEL_2_3		= 3,
			AV1_LEVEL_3			= 4,
			AV1_LEVEL_3_1		= 5,
			AV1_LEVEL_3_2		= 6,
			AV1_LEVEL_3_3		= 7,
			AV1_LEVEL_4			= 8,
			AV1_LEVEL_4_1		= 9,
			AV1_LEVEL_4_2		= 10,
			AV1_LEVEL_4_3		= 11,
			AV1_LEVEL_5			= 12,
			AV1_LEVEL_5_1		= 13,
			AV1_LEVEL_5_2		= 14,
			AV1_LEVEL_5_3		= 15,
			AV1_LEVEL_6			= 16,
			AV1_LEVEL_6_1		= 17,
			AV1_LEVEL_6_2		= 18,
			AV1_LEVEL_6_3		= 19,
			AV1_LEVEL_7			= 20,
			AV1_LEVEL_7_1		= 21,
			AV1_LEVEL_7_2		= 22,
			AV1_LEVEL_7_3		= 23,
		};

		struct FILM_GRAIN_PARAMS_DATA
		{
			uint8_t				apply_grain : 1;
			uint8_t				update_grain : 1;
			uint8_t				film_grain_params_ref_idx : 3;
			uint8_t				byte_align_0 : 3;

			uint16_t			grain_seed;

			uint8_t				num_y_points : 4;
			uint8_t				chroma_scaling_from_luma : 1;
			uint8_t				byte_align_1 : 3;

			uint8_t				point_y_value[16];
			uint8_t				point_y_scaling[16];

			uint8_t				num_cb_points : 4;
			uint8_t				num_cr_points : 4;

			uint8_t				point_cb_value[16];
			uint8_t				point_cb_scaling[16];

			uint8_t				point_cr_value[16];
			uint8_t				point_cr_scaling[16];

			uint8_t				grain_scaling_minus_8 : 2;
			uint8_t				ar_coeff_lag : 2;
			uint8_t				byte_align_2 : 4;

			uint8_t				numPosLuma;
			uint8_t				numPosChroma;

			uint8_t				ar_coeffs_y_plus_128[24];
			uint8_t				ar_coeffs_cb_plus_128[25];
			uint8_t				ar_coeffs_cr_plus_128[25];

			uint8_t				ar_coeff_shift_minus_6 : 2;
			uint8_t				grain_scale_shift : 2;
			uint8_t				overlap_flag : 1;
			uint8_t				clip_to_restricted_range : 1;
			uint8_t				byte_align_3 : 2;

			uint8_t				cb_mult;
			uint8_t				cb_luma_mult;
			uint16_t			cb_offset;

			uint8_t				cr_mult;
			uint8_t				cr_luma_mult;
			uint16_t			cr_offset;
		}PACKED;

		struct RefCntBuffer 
		{
			int32_t				ref_count = 0;

			//
			// The fields related with 7.20. Reference frame update process
			//

			bool				valid_for_referencing = false;
														// RefValid[i] is set equal to 1
			int32_t				current_frame_id = 0;	// RefFrameId[i] is set equal to current_frame_id
			// Width and height give the size of the buffer (before any upscaling, unlike
			// the sizes that can be derived from the buf structure)
			int32_t				upscaled_width = -1;	// RefUpscaledWidth[i] is set equal to UpscaledWidth
			int32_t				width = -1;				// RefFrameWidth[i] is set equal to FrameWidth
			int32_t				height = -1;			// RefFrameHeight[i] is set equal to FrameHeight
			int32_t				render_width = -1;		// RefRenderWidth[i] is set equal to RenderWidth
			int32_t				render_height = -1;		// RefRenderHeight[i] is set equal to RenderHeight
			uint16_t			mi_cols = 0;			// RefMiCols[i] is set equal to MiCols
			uint16_t			mi_rows = 0;			// RefMiRows[i] is set equal to MiRows
			int8_t				frame_type = -1;		// RefFrameType[i] is set equal to frame_type.

														// RefSubsamplingX[i] is set equal to subsampling_x.
														// RefSubsamplingY[i] is set equal to subsampling_y
														// RefBitDepth[i] is set equal to BitDepth

			uint8_t				ref_order_hints[REFS_PER_FRAME] = { 0 };
														// SavedOrderHints[i][j + LAST_FRAME] is set equal to OrderHints[j + LAST_FRAME] for j = 0..REFS_PER_FRAME-1.

			uint8_t				current_order_hint = 0;	// RefOrderHint[i] is set equal to OrderHint

			// Since no real decoding happen, the below fields are ignored at present

														// FrameStore[i][0][y][x] is set equal to LrFrame[0][y][x] for x = 0..UpscaledWidth-1, for y = 0..FrameHeight-1.
														// FrameStore[i][plane][y][x] is set equal to LrFrame[plane][y][x] for plane = 1..2, 
														// for x = 0..((UpscaledWidth + subsampling_x) >> subsampling_x) - 1,
														// for y = 0..((FrameHeight + subsampling_y) >> subsampling_y) - 1

														// SavedRefFrames[i][row][col] is set equal to MfRefFrames[row][col] for row = 0..MiRows-1, for col = 0..MiCols-1.

														// SavedMvs[i][row][col][comp] is set equal to MfMvs[row][col][comp] 
														// for comp = 0..1, for row = 0..MiRows-1, for col = 0..MiCols-1.
			
			int32_t				gm_params[REFS_PER_FRAME][6] = { {0} };
														// SavedGmParams[i][ref][j] is set equal to gm_params[ref][j] for ref = LAST_FRAME..ALTREF_FRAME, for j = 0..5
			
			bool				loop_filter_delta_enabled = false;
			int8_t				loop_filter_ref_deltas[TOTAL_REFS_PER_FRAME] = { 0 };
			int8_t				loop_filter_mode_deltas[2] = { 0 };
			uint8_t				FeatureEnabled[MAX_SEGMENTS][SEG_LVL_MAX] = { {0} };
			int16_t				FeatureData[MAX_SEGMENTS][SEG_LVL_MAX] = { {0} };
			
			WarpedMotionParams	global_motion[NUM_REF_FRAMES];
			uint8_t				showable_frame = 0;		// frame can be used as show existing frame in future
			FILM_GRAIN_PARAMS_DATA
								film_grain_params;

			void Reset() {
				ref_count = 0;
				valid_for_referencing = 0;
			}

			int LoadVBISlotParams(const VBI_SLOT_PARAMS& VBI_slot_params);
		};

		struct VideoBitstreamCtx;

		struct BufferPool 
		{
			RefCntBuffer		frame_bufs[FRAME_BUFFERS];
			VideoBitstreamCtx*	ctx_video_bst;

			BufferPool(VideoBitstreamCtx* pCtx);
			void ref_cnt_fb(int8_t *idx, int new_idx);
			void decrease_ref_count(int8_t idx);
			int8_t get_free_fb();
			void Reset();
		};

		struct OPEN_BITSTREAM_UNIT;

		struct VideoBitstreamCtx : public CComUnknown, public IAV1Context
		{
			/* The current bitstream is annex-b: length delimited bitstream format or not */
			bool				AnnexB;
			/* 
				For Single OBU parsing, no reference frame information at all, so all refer-frame related process will be ignored. 
				It is only supported for the case:
				The AV1 stream is parsed, and save the key information to the database, and when skip to this frame, it can load
				the key information from the database.
			*/
			
			bool				SingleOBUParse;
			bool				SeenFrameHeader;
			int64_t				num_payload_bits = 0LL;
			int64_t				num_trailing_bits = 0LL;
			uint16_t			TileNum = 0;

			//
			// The global context member related with decode process
			//
			BufferPool*			buffer_pool = nullptr;			// BufferPool, a storage area for a set of frame buffers
																// When a frame buffer is used for storing a decoded frame, 
																// it is indicated by a VBI slot that points to this frame buffer
			int8_t				VBI[NUM_REF_FRAMES];			// virtual buffer index,  maps fb_idx to reference slot
																// an array of indices of the frame areas in the BufferPool. VBI elements which do not point to
																// any slot in the VBI are set to -1. VBI array size is equal to 8, with the indices taking values from 0 to 7
			int8_t				cfbi;							// current frame buffer index
																// the variable that contains the index to the area in the BufferPool that contains the current frame

			uint8_t				GmType[REFS_PER_FRAME] = { IDENTITY, IDENTITY, IDENTITY, IDENTITY, IDENTITY, IDENTITY, IDENTITY };
			int32_t				PrevGmParams[REFS_PER_FRAME][6] = { {0} };

			//
			// The current decoding frame context members
			//
			int8_t				ref_frame_idx[REFS_PER_FRAME];	// current decoding frame's reference frames

			int32_t				current_frame_id;
			int32_t				output_frame_index;
			uint8_t				refresh_frame_flags;

			bool				show_existing_frame;
			bool				show_frame;
			int					tile_rows;
			int					tile_cols;
			int					log2_tile_cols;					// only valid for uniform tiles
			int					log2_tile_rows;					// only valid for uniform tiles
			
			int32_t				tu_fu_idx = -1;					// Indicate the frame-unit index of the current temporal-unit
			int64_t				tu_idx = -1;					// indicate the index of the current parsed in the current temporal unit

			std::shared_ptr<OPEN_BITSTREAM_UNIT>
								sp_sequence_header;

			//
			// The below part is only available for SingleOBUParse
			// The below members should be filled when doing the parsing
			std::vector<uint8_t>
								RefFrameSignBiases;				// Only available when SingleOBUParse is set to TRUE
			std::vector<uint8_t>
								frame_types;					// Only available when SingleOBUParse is set to TRUE 
			// End SingleOBUParse member area
			//

			std::vector<OBU_FILTER>
								m_obu_filters;

			VideoBitstreamCtx(bool bAnnexB, bool bSingleOBUParse);
			~VideoBitstreamCtx();

			DECLARE_IUNKNOWN

			HRESULT		NonDelegatingQueryInterface(REFIID uuid, void** ppvObj);

			RET_CODE	SetOBUFilters(std::initializer_list<OBU_FILTER> obu_filters);

			RET_CODE	GetOBUFilters(std::vector<OBU_FILTER>& obu_filters);

			bool		IsOBUFiltered(OBU_FILTER obu_filter);

			AV1_OBU		GetSeqHdrOBU();

			RET_CODE	UpdateSeqHdrOBU(AV1_OBU seq_hdr_obu);

			AV1_BYTESTREAM_FORMAT 
						GetByteStreamFormat();

			RET_CODE	LoadSnapshot(AV1ContextSnapshot* ptr_ctx_snapshot);
			void		Reset();

			void		setup_past_independence();
			void		load_previous(uint8_t primary_ref_frame);
			void		reset_grain_params();
			int			load_grain_params(uint8_t ref_idx);
			int			set_frame_refs(int8_t last_frame_idx, int8_t gold_frame_idx, uint8_t OrderHint);
			int			reference_frame_update();
			void		av1_setup_frame_buf_refs(uint32_t order_hint);
		};

		struct OBU_HEADER : public SYNTAX_BITSTREAM_MAP
		{
			union OBU_EXTENSION_HEADER
			{
				uint8_t		byteVal;
				struct
				{
					uint8_t		temporal_id : 3;
					uint8_t		spatial_id : 2;
					uint8_t		extension_header_reserved_3bits : 3;
				}PACKED;
			}PACKED;

			uint8_t		obu_forbidden_bit : 1;
			uint8_t		obu_type : 4;
			uint8_t		obu_extension_flag : 1;
			uint8_t		obu_has_size_field : 1;
			uint8_t		obu_reserved_1bit : 1;

			OBU_EXTENSION_HEADER
						obu_extension_header = {0};

			OBU_HEADER()
				: obu_forbidden_bit(0), obu_type(0), obu_extension_flag(0), obu_has_size_field(0), obu_reserved_1bit(0) {
			}

			int Map(AMBst in_bst)
			{
				SYNTAX_BITSTREAM_MAP::Map(in_bst);
				try
				{
					MAP_BST_BEGIN(0);
					bsrb1(in_bst, obu_forbidden_bit, 1);
					bsrb1(in_bst, obu_type, 4);
					bsrb1(in_bst, obu_extension_flag, 1);
					bsrb1(in_bst, obu_has_size_field, 1);
					bsrb1(in_bst, obu_reserved_1bit, 1);
					if (obu_extension_flag)
					{
						bsrb1(in_bst, obu_extension_header.temporal_id, 3);
						bsrb1(in_bst, obu_extension_header.spatial_id, 2);
						bsrb1(in_bst, obu_extension_header.extension_header_reserved_3bits, 3);
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
			BST_FIELD_PROP_NUMBER1(obu_forbidden_bit, 1, "must be set to 0");
			BST_FIELD_PROP_2NUMBER1(obu_type, 4, obu_type_names[obu_type]);
			BST_FIELD_PROP_NUMBER1(obu_extension_flag, 1, "indicates if the optional obu_extension_header is present");
			BST_FIELD_PROP_BOOL(obu_has_size_field, "indicates that the obu_size syntax element will be present", 
				"indicates that the obu_size syntax element will not be present");
			BST_FIELD_PROP_NUMBER1(obu_reserved_1bit, 1, "must be set to 0");
			if (obu_extension_flag)
			{
				NAV_WRITE_TAG_BEGIN2("obu_extension_header");
					BST_FIELD_PROP_2NUMBER("temporal_id", 3, obu_extension_header.temporal_id, 
						"specifies the temporal level of the data contained in the OBU");
					BST_FIELD_PROP_2NUMBER("spatial_id", 3, obu_extension_header.spatial_id, 
						"specifies the spatial level of the data contained in the OBU");
					BST_FIELD_PROP_2NUMBER("extension_header_reserved_3bits", 3, obu_extension_header.extension_header_reserved_3bits, 
						"must be set to 0. The value is ignored by a decoder");
				NAV_WRITE_TAG_END2("obu_extension_header");
			}
			DECLARE_FIELDPROP_END()
		};

		struct OPEN_BITSTREAM_UNIT : public SYNTAX_BITSTREAM_MAP
		{
			struct TRAILING_BITS : public SYNTAX_BITSTREAM_MAP
			{
				int			m_nbBits;
				CAMBitArray	trailing_bits;

				TRAILING_BITS(int nbBits) : m_nbBits(nbBits) {
				}

				int Map(AMBst in_bst)
				{
					SYNTAX_BITSTREAM_MAP::Map(in_bst);
					try
					{
						MAP_BST_BEGIN(0);
						int bit_idx = 0;
						while(bit_idx < m_nbBits)
						{
							bsrbarray(in_bst, trailing_bits, bit_idx);
							bit_idx++;
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
				if (m_nbBits > 0)
				{
					BST_FIELD_PROP_NUMBER("trailing_one_bit", 1, trailing_bits[0], "shall be equal to 1");
					for (int trailing_zero_bit_idx = 0; trailing_zero_bit_idx < m_nbBits - 1; trailing_zero_bit_idx++)
					{
						BST_ARRAY_FIELD_PROP_NUMBER("trailing_zero_bit", trailing_zero_bit_idx, 1, trailing_bits[i + 1], "");
					}
				}
				DECLARE_FIELDPROP_END()
			};

			struct SEQUENCE_HEADER_OBU : public SYNTAX_BITSTREAM_MAP
			{
				struct TIMING_INFO : public SYNTAX_BITSTREAM_MAP
				{
					uint32_t		num_units_in_display_tick;
					uint32_t		time_scale;
					uint8_t			equal_picture_interval:1;
					uint8_t			align_byte_0 : 7;
					uint32_t		num_ticks_per_picture_minus_1;

					TIMING_INFO()
						: num_units_in_display_tick(0), time_scale(0), equal_picture_interval(0), align_byte_0(0), num_ticks_per_picture_minus_1(0) {
					}

					int Map(AMBst in_bst)
					{
						SYNTAX_BITSTREAM_MAP::Map(in_bst);
						try
						{
							MAP_BST_BEGIN(0);
							bsrb1(in_bst, num_units_in_display_tick, 32);
							bsrb1(in_bst, time_scale, 32);
							bsrb1(in_bst, equal_picture_interval, 1);
							if (equal_picture_interval)
								av1_read_uvlc(in_bst, num_ticks_per_picture_minus_1);
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
					BST_FIELD_PROP_2NUMBER1(num_units_in_display_tick, 32, 
						"the number of time units of a clock operating at the frequency time_scale Hz "
						"that corresponds to one increment of a clock tick counter");
					BST_FIELD_PROP_2NUMBER1(time_scale, 32, "the number of time units that pass in one second");
					BST_FIELD_PROP_BOOL_BEGIN(equal_picture_interval, 
						"indicates that pictures should be displayed according to their output order with the number of ticks "
						"between two consecutive pictures (without dropping frames) specified by num_ticks_per_picture_minus_1 + 1", 
						"indicates that the interval between two consecutive pictures is not specified");
					if (equal_picture_interval)
					{
						BST_FIELD_PROP_UVLC(num_ticks_per_picture_minus_1, 
							"plus 1 specifies the number of clock ticks corresponding to output time between two consecutive pictures in the output order");
					}
					BST_FIELD_PROP_BOOL_END("equal_picture_interval");
					DECLARE_FIELDPROP_END()
				}PACKED;

				struct DECODER_MODEL_INFO : public SYNTAX_BITSTREAM_MAP
				{
					uint64_t		bitrate_scale : 4;
					uint64_t		buffer_size_scale : 4;
					uint64_t		buffer_delay_length_minus_1 : 5;
					uint64_t		num_units_in_decoding_tick : 32;
					uint64_t		buffer_removal_time_length_minus_1 : 5;
					uint64_t		frame_presentation_time_length_minus_1 : 5;
					uint64_t		qword_align_0 : 9;

					DECODER_MODEL_INFO()
						: bitrate_scale(0), buffer_size_scale(0), buffer_delay_length_minus_1(0)
						, num_units_in_decoding_tick(0), buffer_removal_time_length_minus_1(0), frame_presentation_time_length_minus_1(0), qword_align_0(0) {
					}

					int Map(AMBst in_bst)
					{
						SYNTAX_BITSTREAM_MAP::Map(in_bst);
						try
						{
							MAP_BST_BEGIN(0);
							bsrb1(in_bst, bitrate_scale, 4);
							bsrb1(in_bst, buffer_size_scale, 4);
							bsrb1(in_bst, buffer_delay_length_minus_1, 5);
							bsrb1(in_bst, num_units_in_decoding_tick, 32);
							bsrb1(in_bst, buffer_removal_time_length_minus_1, 5);
							bsrb1(in_bst, frame_presentation_time_length_minus_1, 5);
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
					BST_FIELD_PROP_2NUMBER1(bitrate_scale, 4, 
						"together with bitrate_minus_1 specifies the maximum smoothing buffer input bitrate");
					BST_FIELD_PROP_2NUMBER1(buffer_size_scale, 4, 
						"together with buffer_size_minus_1 specifies the smoothing buffer size");
					BST_FIELD_PROP_2NUMBER1(buffer_delay_length_minus_1, 5, 
						"plus 1 specifies the length of the decoder_buffer_delay and the encoder_buffer_delay syntax elements, in bits");
					BST_FIELD_PROP_2NUMBER1(num_units_in_decoding_tick, 32, 
						"the number of time units of a decoding clock operating at the frequency time_scale Hz that corresponds to one increment of a clock tick counter");
					BST_FIELD_PROP_2NUMBER1(buffer_removal_time_length_minus_1, 5, 
						"plus 1 specifies the length of the buffer_removal_time syntax element, in bits");
					BST_FIELD_PROP_2NUMBER1(frame_presentation_time_length_minus_1, 5, 
						"plus 1 specifies the length of the frame_presentation_time syntax element, in bits");
					DECLARE_FIELDPROP_END()
				}PACKED;

				struct COLOR_CONFIG : public SYNTAX_BITSTREAM_MAP
				{
					uint8_t		high_bitdepth : 1;
					uint8_t		twelve_bit : 1;
					uint8_t		mono_chrome : 1;
					uint8_t		color_description_present_flag : 1;
					uint8_t		BitDepth : 4;

					uint8_t		color_primaries;
					uint8_t		transfer_characteristics;
					uint8_t		matrix_coefficients;

					uint8_t		color_range : 1;
					uint8_t		subsampling_x : 1;
					uint8_t		subsampling_y : 1;
					uint8_t		chroma_sample_position : 2;
					uint8_t		separate_uv_delta_q : 1;
					uint8_t		byte_align_1 : 2;

					SEQUENCE_HEADER_OBU*
								ptr_sequence_header_obu;

					COLOR_CONFIG(SEQUENCE_HEADER_OBU* pSeqHdrOBU) : ptr_sequence_header_obu(pSeqHdrOBU) {

					}

					int Map(AMBst in_bst)
					{
						SYNTAX_BITSTREAM_MAP::Map(in_bst);
						try
						{
							MAP_BST_BEGIN(0);
							bsrb1(in_bst, high_bitdepth, 1);
							if (ptr_sequence_header_obu->seq_profile == 2 && high_bitdepth) {
								bsrb1(in_bst, twelve_bit, 1);
								BitDepth = twelve_bit ? 12 : 10;
							}
							else if (ptr_sequence_header_obu->seq_profile <= 2) {
								BitDepth = high_bitdepth ? 10 : 8;
							}

							if (ptr_sequence_header_obu->seq_profile == 1)
								mono_chrome = 0;
							else
							{
								bsrb1(in_bst, mono_chrome, 1);
							}
							uint8_t NumPlanes = mono_chrome ? 1 : 3;
							bsrb1(in_bst, color_description_present_flag, 1);
							if (color_description_present_flag)
							{
								bsrb1(in_bst, color_primaries, 8);
								bsrb1(in_bst, transfer_characteristics, 8);
								bsrb1(in_bst, matrix_coefficients, 8);
							}
							else
							{
								color_primaries = CP_UNSPECIFIED;
								transfer_characteristics = TC_UNSPECIFIED;
								matrix_coefficients = MC_UNSPECIFIED;
							}

							if (mono_chrome)
							{
								bsrb1(in_bst, color_range, 1);
								subsampling_x = 1;
								subsampling_y = 1;
								chroma_sample_position = CSP_UNKNOWN;
								separate_uv_delta_q = 0;
							}
							else if(color_primaries == CP_BT_709 &&
								transfer_characteristics == TC_SRGB &&
								matrix_coefficients == MC_IDENTITY)
							{
								color_range = 1;
								subsampling_x = 0;
								subsampling_y = 0;
								bsrb1(in_bst, separate_uv_delta_q, 1);
							}
							else
							{
								bsrb1(in_bst, color_range, 1);
								if (ptr_sequence_header_obu->seq_profile == 0) {
									subsampling_x = 1;
									subsampling_y = 1;
								}
								else if (ptr_sequence_header_obu->seq_profile == 1) {
									subsampling_x = 0;
									subsampling_y = 0;
								}
								else {
									if (BitDepth == 12) {
										bsrb1(in_bst, subsampling_x, 1);
										if (subsampling_x) {
											bsrb1(in_bst, subsampling_y, 1);
										}
										else
											subsampling_y = 0;
									}
									else
									{
										subsampling_x = 1;
										subsampling_y = 0;
									}
								}

								if (subsampling_x && subsampling_y)
								{
									bsrb1(in_bst, chroma_sample_position, 2);
								}

								bsrb1(in_bst, separate_uv_delta_q, 1);
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
					BST_FIELD_PROP_NUMBER1(high_bitdepth, 1, "together with seq_profile, determine the bit depth");
					if (high_bitdepth && ptr_sequence_header_obu->seq_profile == 2)
					{
						BST_FIELD_PROP_NUMBER1(twelve_bit, 1, "together with seq_profile, determine the bit depth");
						NAV_WRITE_TAG_WITH_NUMBER_VALUE("BitDepth", high_bitdepth ? 12 : 10, "BitDepth = twelve_bit ? 12 : 10");
					}
					else if(ptr_sequence_header_obu->seq_profile <= 2)
					{
						NAV_WRITE_TAG_WITH_NUMBER_VALUE("BitDepth", high_bitdepth ? 10 : 8, "BitDepth = high_bitdepth ? 10 : 8");
					}

					if (ptr_sequence_header_obu->seq_profile == 1) {
						NAV_WRITE_TAG_WITH_NUMBER_VALUE("mono_chrome", mono_chrome, "Should be 0");
					}
					else {
						BST_FIELD_PROP_BOOL(mono_chrome, "indicates that the video does not contain U and V color planes", 
							"indicates that the video contains Y, U, and V color planes");
					}

					NAV_WRITE_TAG_WITH_NUMBER_VALUE("NumPlanes", mono_chrome ? 1 : 3, "NumPlanes = mono_chrome ? 1 : 3");
					BST_FIELD_PROP_BOOL_BEGIN(color_description_present_flag, 
						"specifies that color_primaries, transfer_characteristics, and matrix_coefficients are present", 
						"specifies that color_primaries, transfer_characteristics and matrix_coefficients are not present");

					if (color_description_present_flag)
					{
						BST_FIELD_PROP_2NUMBER1(color_primaries, 8, color_primaries_descs[color_primaries]);
						BST_FIELD_PROP_2NUMBER1(transfer_characteristics, 8, transfer_characteristics_descs[transfer_characteristics]);
						BST_FIELD_PROP_2NUMBER1(matrix_coefficients, 8, matrix_coefficients_descs[matrix_coefficients]);
					}
					else
					{
						NAV_WRITE_TAG_WITH_NUMBER_VALUE("color_primaries", color_primaries, color_primaries_descs[color_primaries]);
						NAV_WRITE_TAG_WITH_NUMBER_VALUE("transfer_characteristics", transfer_characteristics, transfer_characteristics_descs[transfer_characteristics]);
						NAV_WRITE_TAG_WITH_NUMBER_VALUE("matrix_coefficients", matrix_coefficients, matrix_coefficients_descs[matrix_coefficients]);
					}
					BST_FIELD_PROP_BOOL_END("color_description_present_flag");

					auto YUVColorSpace = [&]()->auto {
						return subsampling_x == 0 && subsampling_y == 0 && mono_chrome == 0 ? "YUV 4:4:4" : (
							subsampling_x == 1 && subsampling_y == 0 && mono_chrome == 0 ? "YUV 4:2:2" : (
								subsampling_x == 1 && subsampling_y == 1 && mono_chrome == 0 ? "YUV 4:2:0" : (
									subsampling_x == 1 && subsampling_y == 1 && mono_chrome == 1 ? "Monochrome 4:2:0" : "Unknown")));
					};

					if (mono_chrome)
					{
						BST_FIELD_PROP_BOOL(color_range, "shall be referred to as the full swing representation for all intents relating to this specification", 
														 "shall be referred to as the studio swing representation");
						NAV_WRITE_TAG_WITH_NUMBER_VALUE("subsampling_x", subsampling_x, "");
						NAV_WRITE_TAG_WITH_NUMBER_VALUE("subsampling_y", subsampling_y, "");
						NAV_WRITE_TAG_WITH_ALIAS("ColorSpace", "YUV Color-space", YUVColorSpace());
						NAV_WRITE_TAG_WITH_NUMBER_VALUE("chroma_sample_position", chroma_sample_position, chroma_sample_position_descs[chroma_sample_position]);
						NAV_WRITE_TAG_WITH_NUMBER_VALUE("separate_uv_delta_q", separate_uv_delta_q,
							separate_uv_delta_q ? "indicates that the U and V planes may have separate delta quantizer values"
												: "indicates that the U and V planes will share the same delta quantizer value");

					}
					else if(color_primaries == CP_BT_709 &&
						transfer_characteristics == TC_SRGB &&
						matrix_coefficients == MC_IDENTITY)
					{
						NAV_WRITE_TAG_WITH_NUMBER_VALUE1(color_range, color_range 
							? "shall be referred to as the full swing representation for all intents relating to this specification"
							: "shall be referred to as the studio swing representation");
						NAV_WRITE_TAG_WITH_NUMBER_VALUE("subsampling_x", subsampling_x, "");
						NAV_WRITE_TAG_WITH_NUMBER_VALUE("subsampling_y", subsampling_y, "");
						NAV_WRITE_TAG_WITH_ALIAS("ColorSpace", "YUV Color-space", YUVColorSpace());
						BST_FIELD_PROP_BOOL(separate_uv_delta_q, "indicates that the U and V planes may have separate delta quantizer values", 
							"indicates that the U and V planes will share the same delta quantizer value");
					}
					else
					{
						BST_FIELD_PROP_BOOL(color_range, "shall be referred to as the full swing representation for all intents relating to this specification", 
														 "shall be referred to as the studio swing representation");
						if (ptr_sequence_header_obu->seq_profile != 0 && ptr_sequence_header_obu->seq_profile != 1 && BitDepth == 12)
						{
							BST_FIELD_PROP_NUMBER1(subsampling_x, 1, "");
							if (subsampling_x) {
								BST_FIELD_PROP_NUMBER1(subsampling_y, 1, "");
							}
							else
							{
								NAV_WRITE_TAG_WITH_NUMBER_VALUE("subsampling_y", subsampling_y, "");
							}
						}
						else
						{
							NAV_WRITE_TAG_WITH_NUMBER_VALUE("subsampling_x", subsampling_x, "");
							NAV_WRITE_TAG_WITH_NUMBER_VALUE("subsampling_y", subsampling_y, "");
						}

						NAV_WRITE_TAG_WITH_ALIAS("ColorSpace", "YUV Color-space", YUVColorSpace());

						if (subsampling_x && subsampling_y) {
							BST_FIELD_PROP_2NUMBER1(chroma_sample_position, 2, chroma_sample_position_descs[chroma_sample_position]);
						}

					}

					DECLARE_FIELDPROP_END()
				}PACKED;

				struct OPERATING_PARAMETERS_INFO : public SYNTAX_BITSTREAM_MAP
				{
					uint32_t		bitrate_minus_1;
					uint32_t		buffer_size_minus_1;
					uint8_t			cbr_flag : 1;
					uint8_t			low_delay_mode_flag : 1;
					uint8_t			byte_align_0 : 6;
					uint32_t		decoder_buffer_delay;
					uint32_t		encoder_buffer_delay;

					SEQUENCE_HEADER_OBU*
									ptr_sequence_header_obu;

					OPERATING_PARAMETERS_INFO(SEQUENCE_HEADER_OBU* pSeqHdrOBU) 
						: bitrate_minus_1(0), buffer_size_minus_1(0), cbr_flag(0), low_delay_mode_flag(0)
						, byte_align_0(0), decoder_buffer_delay(0), encoder_buffer_delay(0), ptr_sequence_header_obu(pSeqHdrOBU) {
					}

					int Map(AMBst in_bst)
					{
						SYNTAX_BITSTREAM_MAP::Map(in_bst);
						try
						{
							MAP_BST_BEGIN(0);
							av1_read_uvlc(in_bst, bitrate_minus_1);
							av1_read_uvlc(in_bst, buffer_size_minus_1);
							bsrb1(in_bst, cbr_flag, 1);
							uint8_t n = (uint8_t)ptr_sequence_header_obu->ptr_decoder_model_info->buffer_delay_length_minus_1 + 1;
							bsrb(in_bst, decoder_buffer_delay, n);
							bsrb(in_bst, encoder_buffer_delay, n);
							bsrb1(in_bst, low_delay_mode_flag, 1);
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
					BST_FIELD_PROP_UVLC(bitrate_minus_1, 
						"together with bitrate_scale specifies the maximum smoothing buffer input bitrate for the operating point op");
					NAV_WRITE_TAG_WITH_NUMBER_VALUE("BitRate", (bitrate_minus_1 + 1) << (6 + ptr_sequence_header_obu->ptr_decoder_model_info->bitrate_scale), 
						"in bits/s, the maximum smoothing buffer input bitrate for the current operating point");
					BST_FIELD_PROP_UVLC(buffer_size_minus_1,
						"together with buffer_size_scale specifies the smoothing buffer size for operating point op");
					NAV_WRITE_TAG_WITH_NUMBER_VALUE("BufferSize", (buffer_size_minus_1 + 1) << (4 + ptr_sequence_header_obu->ptr_decoder_model_info->buffer_size_scale),
						"in bits/s, the smoothing buffer size for the current operating point op");
					BST_FIELD_PROP_BOOL(cbr_flag, "indicates that the decoder model associated with operating point op operates in constant bitrate (CBR) mode", 
						"indicates that the decoder model associated with operating point op operates in variable bitrate (VBR) mode");
					uint8_t n = (uint8_t)ptr_sequence_header_obu->ptr_decoder_model_info->buffer_delay_length_minus_1 + 1;
					BST_FIELD_PROP_2NUMBER1(decoder_buffer_delay, n, 
						"specifies the time interval between the arrival of the first bit in the smoothing buffer and the subsequent removal of the data that "
						"belongs to the first coded frame for operating point op, measured in units of 1/90000");
					BST_FIELD_PROP_2NUMBER1(encoder_buffer_delay, n, 
						"specifies, in combination with decoder_buffer_delay[op] syntax element, the first bit arrival time of frames to be decoded to the smoothing buffer. "
						"encoder_buffer_delay is measured in units of 1/90000 seconds");
					BST_FIELD_PROP_BOOL(low_delay_mode_flag, "indicates that the smoothing buffer operates in low-delay mode for operating point op", 
						"indicates that the smoothing buffer operates in strict mode, where buffer underflow is not allowed");
					DECLARE_FIELDPROP_END()
				}PACKED;

				struct OPERATING_POINT: public SYNTAX_BITSTREAM_MAP
				{
					union
					{
						uint32_t	u32Val;
						struct
						{
							uint32_t		operating_point_idc : 12;
							uint32_t		seq_level_idx : 5;
							uint32_t		seq_tier : 1;
							uint32_t		decoder_model_present_for_this_op : 1;
							uint32_t		initial_display_delay_present_for_this_op : 1;
							uint32_t		initial_display_delay_minus_1 : 4;
							uint32_t		reserved : 8;
						}PACKED;
					}PACKED;

					OPERATING_PARAMETERS_INFO*
								ptr_operating_parameters_info = nullptr;
					SEQUENCE_HEADER_OBU*
								ptr_sequence_header_obu = nullptr;

					OPERATING_POINT(SEQUENCE_HEADER_OBU* pSeqHdrOBU)
						: u32Val(0)
						, ptr_sequence_header_obu(pSeqHdrOBU) {}

					~OPERATING_POINT()
					{
						UNMAP_STRUCT_POINTER5(ptr_operating_parameters_info);
					}

					int Map(AMBst in_bst)
					{
						SYNTAX_BITSTREAM_MAP::Map(in_bst);
						try
						{
							MAP_BST_BEGIN(0);
							bsrb1(in_bst, operating_point_idc, 12);
							bsrb1(in_bst, seq_level_idx, 5);
							if (seq_level_idx > 7) {
								bsrb1(in_bst, seq_tier, 1);
							}
							else
								seq_tier = 0;

							if (ptr_sequence_header_obu->decoder_model_info_present_flag) {
								bsrb1(in_bst, decoder_model_present_for_this_op, 1);
								MAP_MEM_TO_STRUCT_POINTER5_1(in_bst, decoder_model_present_for_this_op, ptr_operating_parameters_info, 
									OPERATING_PARAMETERS_INFO, ptr_sequence_header_obu);
							}
							else
								decoder_model_present_for_this_op = 0;

							if (ptr_sequence_header_obu->initial_display_delay_present_flag) {
								bsrb1(in_bst, initial_display_delay_present_for_this_op, 1);
								if (initial_display_delay_present_for_this_op) {
									bsrb1(in_bst, initial_display_delay_minus_1, 4);
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
					BST_FIELD_PROP_2NUMBER1(operating_point_idc, 12, "contains a bitmask that indicates which spatial and temporal layers "
						"should be decoded for the current operating point");
					BST_FIELD_PROP_2NUMBER1(seq_level_idx, 5, "specifies the level that the coded video sequence conforms to when operating point i is selected");
					if (seq_level_idx > 7) {
						BST_FIELD_PROP_2NUMBER1(seq_tier, 1, "specifies the tier that the coded video sequence conforms to when operating point i is selected");
					}
					else
					{
						NAV_WRITE_TAG_WITH_1NUMBER_VALUE1(seq_tier, "specifies the tier that the coded video sequence conforms to when operating point i is selected");
					}

					if (ptr_sequence_header_obu->decoder_model_info_present_flag)
					{
						BST_FIELD_PROP_BOOL(decoder_model_present_for_this_op, "", "");
						if (decoder_model_present_for_this_op) {
							NAV_WRITE_TAG_BEGIN_WITH_ALIAS("operating_parameters_info", "operating_parameters_info()", "");
							NAV_FIELD_PROP_REF(ptr_operating_parameters_info);
							NAV_WRITE_TAG_END2("operating_parameters_info");
						}
					}
					else
					{
						NAV_WRITE_TAG_WITH_1NUMBER_VALUE1(decoder_model_present_for_this_op, "");
					}

					if (ptr_sequence_header_obu->initial_display_delay_present_flag) {
						BST_FIELD_PROP_BOOL(initial_display_delay_present_for_this_op, "indicates that there is a decoder model associated with the current operating point", 
							"indicates that there is not a decoder model associated with the current operating point");
						if (initial_display_delay_present_for_this_op) {
							BST_FIELD_PROP_2NUMBER1(initial_display_delay_minus_1, 4, 
								"plus 1 specifies, for the current operating point, the number of decoded frames" 
								"that should be present in the buffer pool before the first presentable frame is displayed." 
								"This will ensure that all presentable frames in the sequence can be decoded at or before the time that they are scheduled for display");
						}
					}

					DECLARE_FIELDPROP_END()
				}PACKED;

				uint8_t			seq_profile : 3;
				uint8_t			still_picture : 1;
				uint8_t			reduced_still_picture_header : 1;
				uint8_t			timing_info_present_flag : 1;
				uint8_t			decoder_model_info_present_flag : 1;
				uint8_t			initial_display_delay_present_flag : 1;

				TIMING_INFO*	ptr_timing_info = nullptr;
				DECODER_MODEL_INFO*
								ptr_decoder_model_info = nullptr;

				uint8_t			operating_points_cnt_minus_1 : 5;
				uint8_t			byte_align_0 : 3;

				std::vector<OPERATING_POINT>
								operating_points;

				uint8_t			frame_width_bits_minus_1 : 4;
				uint8_t			frame_height_bits_minus_1 : 4;
				uint16_t		max_frame_width_minus_1;
				uint16_t		max_frame_height_minus_1;

				uint8_t			frame_id_numbers_present_flag : 1;
				uint8_t			delta_frame_id_length_minus_2 : 4;
				uint8_t			additional_frame_id_length_minus_1 : 3;

				uint8_t			use_128x128_superblock : 1;
				uint8_t			enable_filter_intra : 1;
				uint8_t			enable_intra_edge_filter : 1;
				uint8_t			enable_interintra_compound : 1;
				uint8_t			enable_masked_compound : 1;
				uint8_t			enable_warped_motion : 1;
				uint8_t			enable_dual_filter : 1;
				uint8_t			enable_order_hint : 1;

				uint8_t			enable_jnt_comp : 1;
				uint8_t			enable_ref_frame_mvs : 1;
				uint8_t			seq_choose_screen_content_tools : 1;
				uint8_t			seq_force_screen_content_tools : 2;
				uint8_t			seq_choose_integer_mv : 1;
				uint8_t			seq_force_integer_mv : 2;

				uint8_t			order_hint_bits_minus_1 : 3;
				uint8_t			enable_superres : 1;
				uint8_t			enable_cdef : 1;
				uint8_t			enable_restoration : 1;
				uint8_t			film_gain_params_present : 1;
				uint8_t			byte_align_2 : 1;

				uint8_t			OrderHintBits;

				COLOR_CONFIG*	color_config = nullptr;

				OPEN_BITSTREAM_UNIT*
								ptr_OBU = nullptr;

				SEQUENCE_HEADER_OBU(OPEN_BITSTREAM_UNIT* pOBU) : ptr_OBU(pOBU) {

				}

				~SEQUENCE_HEADER_OBU() {
					UNMAP_STRUCT_POINTER5(ptr_timing_info);
					UNMAP_STRUCT_POINTER5(ptr_decoder_model_info);
					UNMAP_STRUCT_POINTER5(color_config);
				}

				int get_relative_dist(int a, int b)
				{
					if (!enable_order_hint)
						return 0;

					assert(OrderHintBits >= 1);
					assert(a >= 0 && a < (1 << OrderHintBits));
					assert(b >= 0 && b < (1 << OrderHintBits));

					int diff = a - b;
					int m = 1 << (OrderHintBits - 1);
					diff = (diff & (m - 1)) - (diff & m);
					return diff;
				}

				int Map(AMBst in_bst)
				{
					SYNTAX_BITSTREAM_MAP::Map(in_bst);
					try
					{
						MAP_BST_BEGIN(0);
						bsrb1(in_bst, seq_profile, 3);
						bsrb1(in_bst, still_picture, 1);
						bsrb1(in_bst, reduced_still_picture_header, 1);

						if (reduced_still_picture_header)
						{
							timing_info_present_flag = 0;
							decoder_model_info_present_flag = 0;
							initial_display_delay_present_flag = 0;
							operating_points_cnt_minus_1 = 0;

							operating_points.emplace_back(this);
							operating_points[0].operating_point_idc = 0;
							bsrb1(in_bst, operating_points[0].seq_level_idx, 5);
							operating_points[0].seq_tier = 0;
							operating_points[0].decoder_model_present_for_this_op = 0;
							operating_points[0].initial_display_delay_present_for_this_op = 0;
						}
						else
						{
							bsrb1(in_bst, timing_info_present_flag, 1);
							if (timing_info_present_flag)
							{
								av1_read_ref(in_bst, ptr_timing_info, TIMING_INFO);
								bsrb1(in_bst, decoder_model_info_present_flag, 1);
								if (decoder_model_info_present_flag)
								{
									av1_read_ref(in_bst, ptr_decoder_model_info, DECODER_MODEL_INFO);
								}
							}
							else
								decoder_model_info_present_flag = 0;

							bsrb1(in_bst, initial_display_delay_present_flag, 1);
							bsrb1(in_bst, operating_points_cnt_minus_1, 5);
							for (uint8_t idxOP = 0; idxOP < operating_points_cnt_minus_1 + 1; idxOP++)
							{
								operating_points.emplace_back(this);
								av1_read_obj(in_bst, operating_points[idxOP]);
							}
						}

						bsrb1(in_bst, frame_width_bits_minus_1, 4);
						bsrb1(in_bst, frame_height_bits_minus_1, 4);
						bsrb1(in_bst, max_frame_width_minus_1, frame_width_bits_minus_1 + 1);
						bsrb1(in_bst, max_frame_height_minus_1, frame_height_bits_minus_1 + 1);

						if (reduced_still_picture_header)
							frame_id_numbers_present_flag = 0;
						else {
							bsrb1(in_bst, frame_id_numbers_present_flag, 1);
						}

						if (frame_id_numbers_present_flag)
						{
							bsrb1(in_bst, delta_frame_id_length_minus_2, 4);
							bsrb1(in_bst, additional_frame_id_length_minus_1, 3);
						}

						bsrb1(in_bst, use_128x128_superblock, 1);
						bsrb1(in_bst, enable_filter_intra, 1);
						bsrb1(in_bst, enable_intra_edge_filter, 1);

						if (reduced_still_picture_header)
						{
							enable_interintra_compound = 0;
							enable_masked_compound = 0;
							enable_warped_motion = 0;
							enable_dual_filter = 0;
							enable_order_hint = 0;
							enable_jnt_comp = 0;
							enable_ref_frame_mvs = 0;
							seq_force_screen_content_tools = SELECT_SCREEN_CONTENT_TOOLS;
							seq_force_integer_mv = SELECT_INTEGER_MV;
							OrderHintBits = 0;
						}
						else
						{
							bsrb1(in_bst, enable_interintra_compound, 1);
							bsrb1(in_bst, enable_masked_compound, 1);
							bsrb1(in_bst, enable_warped_motion, 1);
							bsrb1(in_bst, enable_dual_filter, 1);
							bsrb1(in_bst, enable_order_hint, 1);

							if (enable_order_hint)
							{
								bsrb1(in_bst, enable_jnt_comp, 1);
								bsrb1(in_bst, enable_ref_frame_mvs, 1);
							}
							else
							{
								enable_jnt_comp = 0;
								enable_ref_frame_mvs = 0;
							}

							bsrb1(in_bst, seq_choose_screen_content_tools, 1);
							if (seq_choose_screen_content_tools) {
								seq_force_screen_content_tools = SELECT_SCREEN_CONTENT_TOOLS;
							}
							else 
							{
								bsrb1(in_bst, seq_force_screen_content_tools, 1);
							}

							if (seq_force_screen_content_tools > 0)
							{
								bsrb1(in_bst, seq_choose_integer_mv, 1);
								if (seq_choose_integer_mv)
								{
									seq_force_integer_mv = SELECT_INTEGER_MV;
								}
								else
								{
									bsrb1(in_bst, seq_force_integer_mv, 1);
								}
							}
							else
							{
								seq_force_integer_mv = SELECT_INTEGER_MV;
							}

							if (enable_order_hint)
							{
								bsrb1(in_bst, order_hint_bits_minus_1, 3);
								OrderHintBits = order_hint_bits_minus_1 + 1;
							}
							else
							{
								OrderHintBits = 0;
							}
						}

						bsrb1(in_bst, enable_superres, 1);
						bsrb1(in_bst, enable_cdef, 1);
						bsrb1(in_bst, enable_restoration, 1);

						av1_read_ref(in_bst, color_config, COLOR_CONFIG, this);

						bsrb1(in_bst, film_gain_params_present, 1);

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
					BST_FIELD_PROP_2NUMBER1_F(seq_profile, 3, "%s Profile", get_av1_profile_name(seq_profile));
					BST_FIELD_PROP_BOOL(still_picture, "the coded video sequence contains only one coded frame", "the coded video sequence contains one or more coded frames");
					BST_FIELD_PROP_NUMBER1(reduced_still_picture_header, 1, "specifies that the syntax elements not needed by a still picture are omitted");
					if (reduced_still_picture_header)
					{
						NAV_WRITE_TAG_WITH_NUMBER_VALUE1(timing_info_present_flag, "");
						NAV_WRITE_TAG_WITH_NUMBER_VALUE1(decoder_model_info_present_flag, "");
						NAV_WRITE_TAG_WITH_NUMBER_VALUE1(initial_display_delay_present_flag, "");
						NAV_WRITE_TAG_WITH_NUMBER_VALUE1(operating_points_cnt_minus_1, "");
						NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("operating_point_idc", "operating_point_idc[0]", operating_points[0].operating_point_idc, "");
						BST_FIELD_PROP_2NUMBER_ALIAS_F_("seq_level_idx", "seq_level_idx[0]", 
							5, operating_points[0].seq_level_idx, get_av1_level_name(operating_points[0].seq_level_idx));
						NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("seq_tier", "seq_tier[0]", operating_points[0].seq_tier, "");
						NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("decoder_model_present_for_this_op", 
							"decoder_model_present_for_this_op[0]", operating_points[0].decoder_model_present_for_this_op, "");
						NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("initial_display_delay_present_for_this_op", 
							"initial_display_delay_present_for_this_op[0]", operating_points[0].initial_display_delay_present_for_this_op, "");
					}
					else
					{
						BST_FIELD_PROP_BOOL_BEGIN(timing_info_present_flag, "timing info is present in the coded video sequence", 
							"timing info is NOT present in the coded video sequence");
						if (timing_info_present_flag)
						{
							BST_FIELD_PROP_REF2_1(ptr_timing_info, "timing_info", "");
							BST_FIELD_PROP_BOOL_BEGIN(decoder_model_info_present_flag, 
								"decoder model information is present in the coded video sequence", 
								"decoder model information is NOT present in the coded video sequence");
							if (ptr_decoder_model_info != nullptr) {
								BST_FIELD_PROP_REF2_1(ptr_decoder_model_info, "decoder_model_info", "");
							}
							BST_FIELD_PROP_BOOL_END("decoder_model_info_present_flag");
						}
						BST_FIELD_PROP_BOOL_END("timing_info_present_flag");

						BST_FIELD_PROP_NUMBER1(initial_display_delay_present_flag, 1, 
							"specifies whether initial display delay information is present in the coded video sequence");
						BST_FIELD_PROP_2NUMBER1(operating_points_cnt_minus_1, 5, 
							"plus 1 indicates the number of operating points present in the coded video sequence");

						NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "for(i=0;i&lt;=operating_points_cnt_minus_1;i++)", "");
						for (i = 0; i <= operating_points_cnt_minus_1; i++) {
							NAV_WRITE_TAG_ARRAY_BEGIN0("operating_point", i, "");
							BST_FIELD_PROP_2NUMBER("operating_point_idc", 12, operating_points[i].operating_point_idc, 
								"contains a bitmask that indicates which spatial and temporal layers should be decoded for the current operating point");
							BST_FIELD_PROP_2NUMBER("seq_level_idx", 5, operating_points[i].seq_level_idx, get_av1_level_name(operating_points[i].seq_level_idx));
							if (operating_points[i].seq_level_idx > 7)
							{
								BST_FIELD_PROP_2NUMBER("seq_tier", 1, operating_points[i].seq_tier, 
									"specifies the tier that the coded video sequence conforms to when the current operating point is selected");
							}

							if (decoder_model_info_present_flag)
							{
								BST_FIELD_PROP_NUMBER_BEGIN("decoder_model_present_for_this_op", 1, operating_points[i].decoder_model_present_for_this_op, 
									operating_points[i].decoder_model_present_for_this_op
									?"there is a decoder model associated with operating point"
									:"No decoder model associated with operating point");
								BST_FIELD_PROP_REF2_1(operating_points[i].ptr_operating_parameters_info, "operating_parameters_info", "");
								BST_FIELD_PROP_NUMBER_END("decoder_model_present_for_this_op");
							}
							else
							{
								NAV_FIELD_PROP_NUMBER("decoder_model_present_for_this_op", 1, operating_points[i].decoder_model_present_for_this_op, "");
							}

							if (initial_display_delay_present_flag)
							{
								BST_FIELD_PROP_NUMBER_BEGIN("initial_display_delay_present_for_this_op", 1, operating_points[i].initial_display_delay_present_for_this_op,
									operating_points[i].initial_display_delay_present_for_this_op 
									? "indicates that initial_display_delay_minus_1 is specified for the current operating point" 
									: "indicates that initial_display_delay_minus_1 is NOT specified for the current operating point");
								BST_FIELD_PROP_2NUMBER("initial_display_delay_minus_1", 4, operating_points[i].initial_display_delay_minus_1, 
									"plus 1 specifies, for the current operating point, the number of decoded frames that "
									"should be present in the buffer pool before the first presentable frame is displayed");
								BST_FIELD_PROP_NUMBER_END("initial_display_delay_present_for_this_op");
							}

							NAV_WRITE_TAG_END("operating_point");
						}
						NAV_WRITE_TAG_END("Tag0");

						BST_FIELD_PROP_2NUMBER1(frame_width_bits_minus_1, 4, "the number of bits minus 1 used for transmitting the frame width syntax elements");
						BST_FIELD_PROP_2NUMBER1(frame_height_bits_minus_1, 4, "the number of bits minus 1 used for transmitting the frame height syntax elements");
						BST_FIELD_PROP_2NUMBER1(max_frame_width_minus_1, (long long)frame_width_bits_minus_1 + 1, 
							"the maximum frame width minus 1 for the frames represented by this sequence header");
						BST_FIELD_PROP_2NUMBER1(max_frame_height_minus_1, (long long)frame_height_bits_minus_1 + 1, 
							"the maximum frame height minus 1 for the frames represented by this sequence header");

						if (reduced_still_picture_header)
						{
							NAV_FIELD_PROP_NUMBER1(frame_id_numbers_present_flag, 1, "");
						}
						else
						{
							BST_FIELD_PROP_BOOL_BEGIN(frame_id_numbers_present_flag, 
								"frame id numbers are present in the coded video sequence", "frame id numbers are NOT present in the coded video sequence");
								BST_FIELD_PROP_2NUMBER1(delta_frame_id_length_minus_2, 4, "plus 2 used to encode delta_frame_id syntax elements");
								BST_FIELD_PROP_2NUMBER1(additional_frame_id_length_minus_1, 3, 
									"is used to calculate the number of bits used to encode the frame_id syntax element");
							BST_FIELD_PROP_BOOL_END("frame_id_numbers_present_flag");
						}

						BST_FIELD_PROP_BOOL(use_128x128_superblock, "superblocks contain 128x128 luma samples", "superblocks contain 64x64 luma samples");
						BST_FIELD_PROP_BOOL(enable_filter_intra, "the use_filter_intra syntax element may be present", 
							"the use_filter_intra syntax element will not be present");
						BST_FIELD_PROP_BOOL(enable_intra_edge_filter, "the intra edge filtering process should be enabled", 
							"the intra edge filtering process should NOT be enabled");

						if (reduced_still_picture_header)
						{
							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(enable_interintra_compound, "Should be 0");
							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(enable_masked_compound, "Should be 0");
							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(enable_warped_motion, "Should be 0");
							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(enable_dual_filter, "Should be 0");
							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(enable_order_hint, "Should be 0");
							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(enable_jnt_comp, "Should be 0");
							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(enable_ref_frame_mvs, "Should be 0");
							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(seq_force_screen_content_tools, "Should be SELECT_SCREEN_CONTENT_TOOLS");
							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(seq_force_integer_mv, "Should be SELECT_INTEGER_MV");
							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(OrderHintBits, "Should be 0");
						}
						else
						{
							BST_FIELD_PROP_BOOL(enable_interintra_compound, "the mode info for inter blocks may contain the syntax element interintra", 
								"the syntax element interintra will not be present");
							BST_FIELD_PROP_BOOL(enable_masked_compound, "the mode info for inter blocks may contain the syntax element compound_type", 
								"the syntax element compound_type will not be present");
							BST_FIELD_PROP_BOOL(enable_warped_motion, "the allow_warped_motion syntax element may be present", 
								"the allow_warped_motion syntax element will not be present");
							BST_FIELD_PROP_BOOL(enable_dual_filter, "the inter prediction filter type may be specified independently in the horizontal and vertical directions", 
								"only one filter type may be specified, which is then used in both directions");
							BST_FIELD_PROP_BOOL(enable_order_hint, "tools based on the values of order hints may be used", 
								"tools based on order hints are disabled");
							if (enable_order_hint)
							{
								BST_FIELD_PROP_BOOL(enable_jnt_comp, "the distance weights process may be used for inter prediction", 
									"the distance weights process should NOT be used for inter prediction");
								BST_FIELD_PROP_BOOL(enable_ref_frame_mvs, "the use_ref_frame_mvs syntax element may be present", 
									"the use_ref_frame_mvs syntax element will not be present");
							}
							else
							{
								NAV_WRITE_TAG_WITH_NUMBER_VALUE1(enable_jnt_comp, "");
								NAV_WRITE_TAG_WITH_NUMBER_VALUE1(enable_ref_frame_mvs, "");
							}
							BST_FIELD_PROP_BOOL(seq_choose_screen_content_tools, 
								"seq_force_screen_content_tools should be set equal to SELECT_SCREEN_CONTENT_TOOLS", 
								"the seq_force_screen_content_tools syntax element will be present");
							if (seq_choose_screen_content_tools) {
								NAV_WRITE_TAG_WITH_NUMBER_VALUE1(seq_force_screen_content_tools, seq_force_screen_content_tools == SELECT_SCREEN_CONTENT_TOOLS
									?"the allow_screen_content_tools syntax element will be present in the frame header"
									:"seq_force_screen_content_tools contains the value for allow_screen_content_tools");
							}
							else
							{
								BST_FIELD_PROP_BOOL(seq_force_screen_content_tools, "", "");
							}

							if (seq_force_screen_content_tools > 0)
							{
								BST_FIELD_PROP_BOOL(seq_choose_integer_mv, 
									"seq_force_integer_mv should be set equal to SELECT_INTEGER_MV", 
									"the seq_force_integer_mv syntax element will be present");
								if (seq_choose_integer_mv)
								{
									NAV_WRITE_TAG_WITH_NUMBER_VALUE1(seq_force_integer_mv, "Should be SELECT_INTEGER_MV");
								}
								else
								{
									BST_FIELD_PROP_BOOL(seq_force_integer_mv, "", "");
								}
							}
							else
							{
								NAV_WRITE_TAG_WITH_NUMBER_VALUE1(seq_force_integer_mv, "Should be SELECT_INTEGER_MV");
							}

							if (enable_order_hint)
							{
								BST_FIELD_PROP_2NUMBER1(order_hint_bits_minus_1, 3, "plus 1 specifies the number of bits used for the order_hint syntax element");
							}
							NAV_WRITE_TAG_WITH_1NUMBER_VALUE1(OrderHintBits, "the number of bits used for the order_hint syntax element");
						}

						BST_FIELD_PROP_BOOL(enable_superres, 
							"the use_superres syntax element will be present in the uncompressed header", 
							"the use_superres syntax element will not be present");
						BST_FIELD_PROP_BOOL(enable_cdef, "cdef filtering may be enabled", "cdef filtering is disabled");
						BST_FIELD_PROP_BOOL(enable_restoration, "loop restoration filtering may be enabled", "loop restoration filtering is disabled");

						BST_FIELD_PROP_REF2_1(color_config, "color_config", "");

						BST_FIELD_PROP_BOOL(film_gain_params_present, 
							"film grain parameters are present in the coded video sequence", 
							"film grain parameters are NOT present in the coded video sequence");
					}
				DECLARE_FIELDPROP_END()

			};

			struct TEMPORAL_DELIMITER_OBU : public SYNTAX_BITSTREAM_MAP
			{
				OPEN_BITSTREAM_UNIT* ptr_OBU = nullptr;

				TEMPORAL_DELIMITER_OBU(OPEN_BITSTREAM_UNIT* pOBU): ptr_OBU(pOBU) {
				}

				int Map(AMBst in_bst)
				{
					SYNTAX_BITSTREAM_MAP::Map(in_bst);
					try
					{
						MAP_BST_BEGIN(0);
						if (ptr_OBU != nullptr && ptr_OBU->ctx_video_bst)
						{
							ptr_OBU->ctx_video_bst->SeenFrameHeader = false;
							ptr_OBU->ctx_video_bst->tu_fu_idx = 0;
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

			struct PADDING_OBU : public SYNTAX_BITSTREAM_MAP
			{
				std::vector<uint8_t>	obu_padding_bytes;

				PADDING_OBU(size_t obu_padding_length) {
					if (obu_padding_length > 0)
						obu_padding_bytes.resize(obu_padding_length);
				}

				int Map(AMBst in_bst)
				{
					SYNTAX_BITSTREAM_MAP::Map(in_bst);
					try
					{
						MAP_BST_BEGIN(0);
						for (size_t i = 0; i < obu_padding_bytes.size(); i++)
						{
							bsrb1(in_bst, obu_padding_bytes[i], 8);
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
				for (i = 0; i < (int)obu_padding_bytes.size(); i++)
				{
					BST_ARRAY_FIELD_PROP_NUMBER1(obu_padding_bytes, i, 8, "");
				}
				DECLARE_FIELDPROP_END()
			};

			struct METADATA_OBU : public SYNTAX_BITSTREAM_MAP
			{
				struct METADATA_ITUT_T35 : public SYNTAX_BITSTREAM_MAP
				{
					uint8_t			itu_t_t35_country_code = 0;
					uint8_t			itu_t_t35_country_code_extension_byte = 0;
					std::vector<uint8_t>
									itu_t_t35_payload_bytes;

					VideoBitstreamCtx*
									ctx_video_bst = nullptr;

					METADATA_ITUT_T35(VideoBitstreamCtx* pVideoBstCtx): ctx_video_bst(pVideoBstCtx) {

					}

					int Map(AMBst in_bst)
					{
						SYNTAX_BITSTREAM_MAP::Map(in_bst);
						try
						{
							MAP_BST_BEGIN(0);
							bsrb1(in_bst, itu_t_t35_country_code, 8);
							ctx_video_bst->num_payload_bits -= 8;
							if (itu_t_t35_country_code == 0xFF)
							{
								bsrb1(in_bst, itu_t_t35_country_code_extension_byte, 8);
								ctx_video_bst->num_payload_bits -= 8;
							}

							if (ctx_video_bst->num_payload_bits >= 8)
							{
								itu_t_t35_payload_bytes.resize((size_t)(ctx_video_bst->num_payload_bits/8));
								for (int i = 0; i < (int)(ctx_video_bst->num_payload_bits / 8); i++)
								{
									bsrb1(in_bst, itu_t_t35_payload_bytes[i], 8);
									ctx_video_bst->num_payload_bits -= 8;
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
						BST_FIELD_PROP_2NUMBER1(itu_t_t35_country_code, 8, "");
						if (itu_t_t35_country_code == 0xFF)
						{
							BST_FIELD_PROP_2NUMBER1(itu_t_t35_country_code_extension_byte, 8, "");
						}
						for (i = 0; i < (int)itu_t_t35_payload_bytes.size(); i++)
						{
							BST_ARRAY_FIELD_PROP_NUMBER1(itu_t_t35_payload_bytes, i, 8, "");
						}
					DECLARE_FIELDPROP_END()
				};

				struct METADATA_HDR_CLL : public SYNTAX_BITSTREAM_MAP
				{
					uint16_t		max_cll = 0;
					uint16_t		max_fall = 0;

					int Map(AMBst in_bst)
					{
						SYNTAX_BITSTREAM_MAP::Map(in_bst);
						try
						{
							MAP_BST_BEGIN(0);
							bsrb1(in_bst, max_cll, 16);
							bsrb1(in_bst, max_fall, 16);
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
					BST_FIELD_PROP_2NUMBER1(max_cll, 16, "specifies the maximum content light level as specified in CEA-861.3");
					BST_FIELD_PROP_2NUMBER1(max_fall, 16, "specifies the maximum frame-average light level as specified in CEA-861.3");
					DECLARE_FIELDPROP_END()
				}PACKED;

				struct METADATA_HDR_MDCV : public SYNTAX_BITSTREAM_MAP
				{
					uint16_t		primary_chromaticity_x[3] = { 0 };
					uint16_t		primary_chromaticity_y[3] = { 0 };

					uint16_t		white_point_chromaticity_x = 0;
					uint16_t		white_point_chromaticity_y = 0;

					uint32_t		luminance_max = 0;
					uint32_t		luminance_min = 0;

					int Map(AMBst in_bst)
					{
						SYNTAX_BITSTREAM_MAP::Map(in_bst);
						try
						{
							MAP_BST_BEGIN(0);
							for (int i = 0; i < 3; i++)
							{
								bsrb1(in_bst, primary_chromaticity_x[i], 16);
								bsrb1(in_bst, primary_chromaticity_y[i], 16);
							}
							bsrb1(in_bst, white_point_chromaticity_x, 16);
							bsrb1(in_bst, white_point_chromaticity_y, 16);

							bsrb1(in_bst, luminance_max, 16);
							bsrb1(in_bst, luminance_min, 16);
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
						for (i = 0; i < 3; i++)
						{
							NAV_WRITE_TAG_ARRAY_BEGIN("primary_chromaticity", i, "where i = 0,1,2 specifies Red, Green, Blue respectively");
								BST_FIELD_PROP_2NUMBER("x", 16, primary_chromaticity_x[i], "specifies a 0.16 fixed-point X chromaticity coordinate as defined by CIE 1931");
								BST_FIELD_PROP_2NUMBER("y", 16, primary_chromaticity_y[i], "specifies a 0.16 fixed-point Y chromaticity coordinate as defined by CIE 1931");
							NAV_WRITE_TAG_END("primary_chomaticity");
						}
						BST_FIELD_PROP_2NUMBER1(white_point_chromaticity_x, 16, "specifies a 0.16 fixed-point white X chromaticity coordinate as defined by CIE 1931");
						BST_FIELD_PROP_2NUMBER1(white_point_chromaticity_y, 16, "specifies a 0.16 fixed-point white Y chromaticity coordinate as defined by CIE 1931");
						BST_FIELD_PROP_2NUMBER1(luminance_max, 16, "a 24.8 fixed-point maximum luminance, represented in candelas per square meter");
						BST_FIELD_PROP_2NUMBER1(luminance_min, 16, "18.14 fixed-point minimum luminance, represented in candelas per square meter");
					DECLARE_FIELDPROP_END()
				}PACKED;

				struct METADATA_SCALABILITY : public SYNTAX_BITSTREAM_MAP
				{
					struct SCALABILITY_STRUCTURE : public SYNTAX_BITSTREAM_MAP
					{
						struct TEMPORAL_GROUP
						{
							uint8_t		temporal_group_temporal_id : 3;
							uint8_t		temporal_group_temporal_switching_up_point_flag : 1;
							uint8_t		temporal_group_spatial_switching_up_point_flag : 1;
							uint8_t		temporal_group_ref_cnt : 3;
							uint8_t		temporal_group_ref_pic_diff[7];
						}PACKED;

						uint8_t		    spatial_layers_cnt_minus_1 : 2;				// f(2)
						uint8_t			spatial_layer_dimensions_present_flag:1;	// f(1)
						uint8_t			spatial_layer_description_present_flag:1;	// f(1)
						uint8_t			temporal_group_description_present_flag:1;	// f(1)
						uint8_t			scalability_structure_reserved_3bits:3;		// f(3)

						uint16_t		spatial_layer_max_width[4] = { 0 };
						uint16_t		spatial_layer_max_height[4] = { 0 };

						uint8_t			spatial_layer_ref_id[4] = { 0 };

						uint8_t			temporal_group_size = 0;
						std::vector<TEMPORAL_GROUP>
										temporal_groups;

						SCALABILITY_STRUCTURE()
							: spatial_layers_cnt_minus_1(0), spatial_layer_dimensions_present_flag(0)
							, spatial_layer_description_present_flag(0), temporal_group_description_present_flag(0), scalability_structure_reserved_3bits(0) {
						}

						int Map(AMBst in_bst)
						{
							SYNTAX_BITSTREAM_MAP::Map(in_bst);
							try
							{
								MAP_BST_BEGIN(0);
								bsrb1(in_bst, spatial_layers_cnt_minus_1, 2);
								bsrb1(in_bst, spatial_layer_dimensions_present_flag, 1);
								bsrb1(in_bst, spatial_layer_description_present_flag, 1);
								bsrb1(in_bst, temporal_group_description_present_flag, 1);
								bsrb1(in_bst, scalability_structure_reserved_3bits, 3);

								if (spatial_layer_dimensions_present_flag)
								{
									for (int i = 0; i <= spatial_layers_cnt_minus_1; i++)
									{
										bsrb1(in_bst, spatial_layer_max_width[i], 16);
										bsrb1(in_bst, spatial_layer_max_height[i], 16);
									}
								}

								if (spatial_layer_description_present_flag)
								{
									for (int i = 0; i <= spatial_layers_cnt_minus_1; i++)
									{
										bsrb1(in_bst, spatial_layer_ref_id[i], 8);
									}
								}

								if (temporal_group_description_present_flag)
								{
									bsrb1(in_bst, temporal_group_size, 8);
									if (temporal_group_size > 0)
										temporal_groups.resize(temporal_group_size);

									for (int i = 0; i < temporal_group_size; i++)
									{
										bsrb1(in_bst, temporal_groups[i].temporal_group_temporal_id, 3);
										bsrb1(in_bst, temporal_groups[i].temporal_group_temporal_switching_up_point_flag, 1);
										bsrb1(in_bst, temporal_groups[i].temporal_group_spatial_switching_up_point_flag, 1);
										bsrb1(in_bst, temporal_groups[i].temporal_group_ref_cnt, 3);

										for (int j = 0; j < temporal_groups[i].temporal_group_ref_cnt; j++)
										{
											bsrb1(in_bst, temporal_groups[i].temporal_group_ref_pic_diff[j], 8);
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
						BST_FIELD_PROP_2NUMBER1(spatial_layers_cnt_minus_1, 2, "indicates the number of spatial layers present in the video sequence minus one");
						BST_FIELD_PROP_BOOL(spatial_layer_dimensions_present_flag, 
							"the spatial_layer_max_width and spatial_layer_max_height parameters are present for each of the (spatial_layers_cnt_minus_1 + 1) layers", 
							"the spatial_layer_max_width and spatial_layer_max_height parameters are NOT present for each of the (spatial_layers_cnt_minus_1 + 1) layers");
						BST_FIELD_PROP_BOOL(spatial_layer_description_present_flag, 
							"the spatial_layer_ref_id is present for each of the (spatial_layers_cnt_minus_1 + 1) layers", 
							"the spatial_layer_ref_id is NOT present for each of the (spatial_layers_cnt_minus_1 + 1) layers");
						BST_FIELD_PROP_BOOL(temporal_group_description_present_flag, 
							"the temporal dependency information is present", " the temporal dependency information is NOT present");
						BST_FIELD_PROP_2NUMBER1(scalability_structure_reserved_3bits, 3, 
							"must be set to zero and be ignored by decoders");

						if (spatial_layer_dimensions_present_flag)
						{
							NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "for(i=0;i&lt;=spatial_layers_cnt_minus_1;i++)", "spatial layers' resolution");
							for (i = 0; i <= spatial_layers_cnt_minus_1; i++)
							{
								BST_ARRAY_FIELD_PROP_NUMBER1(spatial_layer_max_width, i, 16, 
									"specifies the maximum frame width for the frames with spatial_id equal to i");
								BST_ARRAY_FIELD_PROP_NUMBER1(spatial_layer_max_height, i, 16, 
									"specifies the maximum frame height for the frames with spatial_id equal to i");
							}
							NAV_WRITE_TAG_END("Tag0");
						}

						if (spatial_layer_description_present_flag) {
							NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag1", "for(i=0;i&lt;=spatial_layers_cnt_minus_1;i++)", "spatial layers' reference ids");
							for (i = 0; i <= spatial_layers_cnt_minus_1; i++)
							{
								BST_ARRAY_FIELD_PROP_NUMBER1(spatial_layer_ref_id, i, 8, 
									"specifies the spatial_id value of the frame within the current temporal unit that the frame of layer i uses for reference. "
									"If no frame within the current temporal unit is used for reference the value must be equal to 255");
							}
							NAV_WRITE_TAG_END("Tag1");
						}

						if (temporal_group_description_present_flag) {
							BST_FIELD_PROP_2NUMBER1(temporal_group_size, 8, "");
							NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag2", "for(i=0;i&lt;temporal_group_size;i++)", "");
							for (int i = 0; i < temporal_group_size; i++)
							{
								NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag21", "for(j=0;j&lt;temporal_group_ref_cnt[i];j++)", "");

								BST_ARRAY_FIELD_PROP_NUMBER_("temporal_group_temporal_id", "pic", i, 3, temporal_groups[i].temporal_group_temporal_id, 
									"specifies the temporal_id value for the i-th picture in the temporal group");
								BST_ARRAY_FIELD_PROP_NUMBER_("temporal_group_temporal_switching_up_point_flag", "pic", 
									i, 3, temporal_groups[i].temporal_group_temporal_switching_up_point_flag, 
									"set to 1 if subsequent (in decoding order) pictures with a temporal_id higher than "
									"temporal_group_temporal_id[i] do not depend on any picture preceding the "
									"current picture(in coding order) with temporal_id higher than temporal_group_temporal_id[i]");
								BST_ARRAY_FIELD_PROP_NUMBER_("temporal_group_spatial_switching_up_point_flag", "pic", 
									i, 3, temporal_groups[i].temporal_group_spatial_switching_up_point_flag, 
									"is set to 1 if spatial layers of the current picture in the temporal group "
									"(i.e., pictures with a spatial_id higher than zero) do not depend on any picture "
									"preceding the current picture in the temporal group");
								BST_ARRAY_FIELD_PROP_NUMBER_("temporal_group_ref_cnt", "pic", i, 3, temporal_groups[i].temporal_group_ref_cnt, 
									"indicates the number of reference pictures used by the i-th picture in the temporal group");

								for (int j = 0; j < temporal_groups[i].temporal_group_ref_cnt; j++)
								{
									BST_2ARRAY_FIELD_PROP_2NUMBER_("temporal_group_ref_pic_diff", "pic#", i, "refpic#", j, 8, 
										temporal_groups[i].temporal_group_ref_pic_diff[j], 
										"indicates, for the i-th picture in the temporal group, the temporal distance "
										"between the i-th picture and the j-th reference picture used by the i-th picture. "
										"The temporal distance is measured in frames, counting only frames of identical spatial_id values");
								}
								NAV_WRITE_TAG_END("Tag21");
							}
							NAV_WRITE_TAG_END("Tag2");
						}

						DECLARE_FIELDPROP_END()

					};

					uint8_t					scalability_mode_idc = 0;
					SCALABILITY_STRUCTURE	scalability_structure;

					int Map(AMBst in_bst)
					{
						SYNTAX_BITSTREAM_MAP::Map(in_bst);
						try
						{
							MAP_BST_BEGIN(0);
							bsrb1(in_bst, scalability_mode_idc, 8);
							if (scalability_mode_idc == SCALABILITY_SS)
							{
								av1_read_obj(in_bst, scalability_structure);
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
					BST_FIELD_PROP_2NUMBER1(scalability_mode_idc, 8, "indicates the picture prediction structure of the coded video sequence");
					if (scalability_mode_idc == SCALABILITY_SS)
					{
						NAV_WRITE_TAG_BEGIN("scalability_structure");
						BST_FIELD_PROP_OBJECT(scalability_structure);
						NAV_WRITE_TAG_END("scalability_structure");
					}
					DECLARE_FIELDPROP_END()
				};

				struct METADATA_TIMECODE : public SYNTAX_BITSTREAM_MAP
				{
					uint64_t		counting_type : 5;
					uint64_t		full_timestamp_flag : 1;
					uint64_t		discontinuity_flag : 1;
					uint64_t		cnt_dropped_flag : 1;
					uint64_t		n_frames : 9;

					uint64_t		seconds_flag : 1;
					uint64_t		seconds_value : 6;
					uint64_t		minutes_flag : 1;
					uint64_t		minutes_value : 6;
					uint64_t		hours_flag : 1;
					uint64_t		hours_value : 5;

					uint64_t		time_offset_length : 5;

					uint64_t		qword_align : 25;

					uint32_t		time_offset_value;

					METADATA_TIMECODE()
						: counting_type(0), full_timestamp_flag(0), discontinuity_flag(0), cnt_dropped_flag(0), n_frames(0)
						, seconds_flag(0), seconds_value(0), minutes_flag(0), minutes_value(0), hours_flag(0), hours_value(0)
						, time_offset_length(0), qword_align(0), time_offset_value(0) {
					}

					int Map(AMBst in_bst)
					{
						SYNTAX_BITSTREAM_MAP::Map(in_bst);
						try
						{
							MAP_BST_BEGIN(0);

							bsrb1(in_bst, counting_type, 5);
							bsrb1(in_bst, full_timestamp_flag, 1);
							bsrb1(in_bst, discontinuity_flag, 1);
							bsrb1(in_bst, cnt_dropped_flag, 1);
							bsrb1(in_bst, n_frames, 9);
							if (full_timestamp_flag)
							{
								bsrb1(in_bst, seconds_value, 6);
								bsrb1(in_bst, minutes_value, 6);
								bsrb1(in_bst, hours_value, 5);
							}
							else
							{
								bsrb1(in_bst, seconds_flag, 1);
								if (seconds_flag)
								{
									bsrb1(in_bst, seconds_value, 6);
									bsrb1(in_bst, minutes_flag, 1);
									if (minutes_flag)
									{
										bsrb1(in_bst, minutes_value, 6);
										bsrb1(in_bst, hours_flag, 1);
										if (hours_flag)
										{
											bsrb1(in_bst, hours_value, 5);
										}
									}
								}
							}

							bsrb1(in_bst, time_offset_length, 5);
							if (time_offset_length > 0)
							{
								bsrb1(in_bst, time_offset_value, time_offset_length);
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
					BST_FIELD_PROP_2NUMBER1(counting_type, 5, "");
					BST_FIELD_PROP_BOOL(full_timestamp_flag, 
						"indicates that the seconds_value, minutes_value, hours_value syntax elements will be present", 
						"indicates that there are flags to control the presence of seconds_value, minutes_value, hours_value syntax elements");
					BST_FIELD_PROP_BOOL(discontinuity_flag, 
						"indicates that the difference between the current value of clockTimestamp and the value of clockTimestamp computed from "
						"the previous set of clock timestamp syntax elements in output order should not be interpreted as the time difference "
						"between the times of origin or capture of the associated frames or fields", 
						"indicates that the difference between the current value of clockTimestamp and the value of clockTimestamp computed from "
						"the previous set of timestamp syntax elements in output order can be interpreted as the time difference between the times "
						"of origin or capture of the associated frames or fields");
					BST_FIELD_PROP_NUMBER1(cnt_dropped_flag, 1, "specifies the skipping of one or more values of n_frames using the counting method specified by counting_type");
					BST_FIELD_PROP_2NUMBER1(n_frames, 9, "is used to compute clockTimestamp");
					if (full_timestamp_flag)
					{
						BST_FIELD_PROP_2NUMBER1(seconds_value, 6, "");
						BST_FIELD_PROP_2NUMBER1(minutes_value, 6, "");
						BST_FIELD_PROP_2NUMBER1(hours_value, 5, "");
					}
					else
					{
						BST_FIELD_PROP_BOOL(seconds_flag, "that seconds_value and minutes_flag are present", "that seconds_value and minutes_flag are NOT present");
						if (seconds_flag)
						{
							BST_FIELD_PROP_2NUMBER1(seconds_value, 6, "");
							BST_FIELD_PROP_BOOL(minutes_flag, "minutes_value and hours_flag are present", "minutes_value and hours_flag are NOT present");
							if (minutes_flag)
							{
								BST_FIELD_PROP_2NUMBER1(minutes_value, 6, "");
								BST_FIELD_PROP_BOOL(hours_flag, "hours_value is present", "hours_value is NOT present");
								if (hours_flag)
								{
									BST_FIELD_PROP_2NUMBER1(hours_value, 5, "");
								}
							}
						}
					}

					BST_FIELD_PROP_2NUMBER1(time_offset_length, 5, "specifies the length in bits of the time_offset_value syntax element");
					if (time_offset_length > 0) {
						BST_FIELD_PROP_2NUMBER1(time_offset_value, time_offset_length, "");
					}

					DECLARE_FIELDPROP_END()
				}PACKED;

				struct METADATA_UNKNOWN : public SYNTAX_BITSTREAM_MAP
				{
					std::vector<uint8_t>	bytes;
					CAMBitArray				bits;
					VideoBitstreamCtx*		ctx_video_bst = nullptr;

					METADATA_UNKNOWN(VideoBitstreamCtx* pCtxVideoBst) :
						ctx_video_bst(pCtxVideoBst) {
					}

					int Map(AMBst in_bst)
					{
						SYNTAX_BITSTREAM_MAP::Map(in_bst);
						try
						{
							MAP_BST_BEGIN(0);
							if (ctx_video_bst->num_payload_bits / 8 > 0)
							{
								bytes.resize((size_t)(ctx_video_bst->num_payload_bits / 8));
								for (int i = 0; i < (ctx_video_bst->num_payload_bits / 8); i++) {
									bsrb1(in_bst, bytes[i], 8);
									ctx_video_bst->num_payload_bits -= 8;
								}
							}

							for (int i = 0; ctx_video_bst->num_payload_bits > 0 && i < ctx_video_bst->num_payload_bits % 8; i++)
							{
								bsrbarray(in_bst, bits, i);
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

				uint64_t		metadata_type = 0;
				union
				{
					METADATA_ITUT_T35*	ptr_metadata_itut_t35;
					METADATA_HDR_CLL*	ptr_metadata_hdr_cll;
					METADATA_HDR_MDCV*	ptr_metadata_hdr_mdcv;
					METADATA_SCALABILITY*
										ptr_metadata_scalability;
					METADATA_TIMECODE*	ptr_metadata_timecode;
					METADATA_UNKNOWN*	ptr_metadata_unknown;
					void*				ptr_metadata;
				};

				OPEN_BITSTREAM_UNIT*	ptr_OBU = nullptr;
				uint8_t					metadata_type_leb128_bytes = 0;

				METADATA_OBU(OPEN_BITSTREAM_UNIT* pOBU)
					: ptr_OBU(pOBU){
					ptr_metadata = nullptr;
				}

				~METADATA_OBU() {
					if (metadata_type == METADATA_TYPE_ITUT_T35)
					{
						AMP_SAFEDEL2(ptr_metadata_itut_t35);
					}
					else if (metadata_type == METADATA_TYPE_HDR_CLL)
					{
						AMP_SAFEDEL2(ptr_metadata_hdr_cll);
					}
					else if (metadata_type == METADATA_TYPE_HDR_MDCV)
					{
						AMP_SAFEDEL2(ptr_metadata_hdr_mdcv);
					}
					else if (metadata_type == METADATA_TYPE_SCALABILITY)
					{
						AMP_SAFEDEL2(ptr_metadata_scalability);
					}
					else if (metadata_type == METADATA_TYPE_TIMECODE)
					{
						AMP_SAFEDEL2(ptr_metadata_timecode);
					}
					else
					{
						AMP_SAFEDEL2(ptr_metadata_unknown);
					}
				}

				int Map(AMBst in_bst)
				{
					SYNTAX_BITSTREAM_MAP::Map(in_bst);
					try
					{
						MAP_BST_BEGIN(0);
						av1_read_leb128_1(in_bst, metadata_type, metadata_type_leb128_bytes);
						ptr_OBU->ctx_video_bst->num_payload_bits -= (int64_t)AMBst_Tell(in_bst) - bit_pos;
						if (metadata_type == METADATA_TYPE_ITUT_T35)
						{
							av1_read_ref(in_bst, ptr_metadata_itut_t35, METADATA_ITUT_T35, ptr_OBU->ctx_video_bst);
						}
						else if (metadata_type == METADATA_TYPE_HDR_CLL)
						{
							av1_read_ref(in_bst, ptr_metadata_hdr_cll, METADATA_HDR_CLL);
						}
						else if (metadata_type == METADATA_TYPE_HDR_MDCV)
						{
							av1_read_ref(in_bst, ptr_metadata_hdr_mdcv, METADATA_HDR_MDCV);
						}
						else if (metadata_type == METADATA_TYPE_SCALABILITY)
						{
							av1_read_ref(in_bst, ptr_metadata_scalability, METADATA_SCALABILITY);
						}
						else if (metadata_type == METADATA_TYPE_TIMECODE)
						{
							av1_read_ref(in_bst, ptr_metadata_timecode, METADATA_TIMECODE);
						}
						else
						{
							av1_read_ref(in_bst, ptr_metadata_unknown, METADATA_UNKNOWN, ptr_OBU->ctx_video_bst);
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
				BST_FIELD_PROP_2NUMBER1(metadata_type, (long long)metadata_type_leb128_bytes<<3, METADATA_TYPE_NAME(metadata_type));
				if (metadata_type == METADATA_TYPE_ITUT_T35)
				{
					BST_FIELD_PROP_REF2_1(ptr_metadata_itut_t35, "metadata_itut_t35", "");
				}
				else if (metadata_type == METADATA_TYPE_HDR_CLL)
				{
					BST_FIELD_PROP_REF2_1(ptr_metadata_hdr_cll, "metadata_hdr_cll", "");
				}
				else if (metadata_type == METADATA_TYPE_HDR_MDCV)
				{
					BST_FIELD_PROP_REF2_1(ptr_metadata_hdr_mdcv, "metadata_hdr_mdcv", "");
				}
				else if (metadata_type == METADATA_TYPE_SCALABILITY)
				{
					BST_FIELD_PROP_REF2_1(ptr_metadata_scalability, "metadata_scalability", "");
				}
				else if (metadata_type == METADATA_TYPE_TIMECODE)
				{
					BST_FIELD_PROP_REF2_1(ptr_metadata_timecode, "metadata_timecode", "");
				}
				else
				{
					BST_FIELD_PROP_REF2_1(ptr_metadata_unknown, "metadata_unknown", "");
				}
				DECLARE_FIELDPROP_END()

			}PACKED;

			struct RESERVED_OBU : public SYNTAX_BITSTREAM_MAP
			{
				std::vector<uint8_t>	bytes;
				CAMBitArray				bits;

				OPEN_BITSTREAM_UNIT*	ptr_OBU;

				RESERVED_OBU(OPEN_BITSTREAM_UNIT* pOBU) :ptr_OBU(pOBU) {
				}

				int Map(AMBst in_bst)
				{
					SYNTAX_BITSTREAM_MAP::Map(in_bst);
					try
					{
						MAP_BST_BEGIN(0);
						if (ptr_OBU->ctx_video_bst->num_payload_bits / 8 > 0)
						{
							bytes.resize((size_t)(ptr_OBU->ctx_video_bst->num_payload_bits / 8));
							for (int i = 0; i < (ptr_OBU->ctx_video_bst->num_payload_bits / 8); i++) {
								bsrb1(in_bst, bytes[i], 8);
								ptr_OBU->ctx_video_bst->num_payload_bits -= 8;
							}
						}

						for (int i = 0; ptr_OBU->ctx_video_bst->num_payload_bits > 0 && i < ptr_OBU->ctx_video_bst->num_payload_bits % 8; i++)
						{
							bsrbarray(in_bst, bits, i);
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

			struct TILE_LIST_OBU : public SYNTAX_BITSTREAM_MAP
			{
				struct TILE_LIST_ENTRY: public SYNTAX_BITSTREAM_MAP
				{
					uint8_t			anchor_frame_idx = 0;
					uint8_t			anchor_tile_row = 0;
					uint8_t			anchor_tile_col = 0;

					uint16_t		tile_data_size_minus_1 = 0;
					std::vector<uint8_t>
									coded_tile_data;

					int Map(AMBst in_bst)
					{
						SYNTAX_BITSTREAM_MAP::Map(in_bst);
						try
						{
							MAP_BST_BEGIN(0);
							bsrb1(in_bst, anchor_frame_idx, 8);
							bsrb1(in_bst, anchor_tile_row, 8);
							bsrb1(in_bst, anchor_tile_col, 8);

							bsrb1(in_bst, tile_data_size_minus_1, 16);
							coded_tile_data.resize((size_t)tile_data_size_minus_1 + 1);

							for (size_t i = 0; i < coded_tile_data.size(); i++) {
								bsrb1(in_bst, coded_tile_data[i], 8);
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
					BST_FIELD_PROP_2NUMBER1(anchor_frame_idx, 8, "the index into an array AnchorFrames of the frames that the tile uses for prediction");
					BST_FIELD_PROP_2NUMBER1(anchor_tile_row, 8, "the row coordinate of the tile in the frame that it belongs, in tile units");
					BST_FIELD_PROP_2NUMBER1(anchor_tile_col, 8, "the column coordinate of the tile in the frame that it belongs, in tile units");
					BST_FIELD_PROP_2NUMBER1(tile_data_size_minus_1, 16, "plus one is the size of the coded tile data, coded_tile_data, in bytes");
					NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "coded_tile_data", "plus one is the size of the coded tile data, coded_tile_data, in bytes");
					for (i = 0; i < tile_data_size_minus_1 + 1; i++)
					{
						BST_ARRAY_FIELD_PROP_NUMBER__("Tag", "", "", i, 8, coded_tile_data[i], "");
					}
					NAV_WRITE_TAG_END("Tag0");
					DECLARE_FIELDPROP_END()
				};

				uint8_t			output_frame_width_in_tiles_minus_1 = 0;
				uint8_t			output_frame_height_in_tiles_minus_1 = 0;

				uint16_t		tile_count_minus_1 = 0;

				std::vector<TILE_LIST_ENTRY>
								tile_list_entries;

				int Map(AMBst in_bst)
				{
					SYNTAX_BITSTREAM_MAP::Map(in_bst);
					try
					{
						MAP_BST_BEGIN(0);
						bsrb1(in_bst, output_frame_width_in_tiles_minus_1, 8);
						bsrb1(in_bst, output_frame_height_in_tiles_minus_1, 8);
						bsrb1(in_bst, tile_count_minus_1, 16);
						tile_list_entries.resize((size_t)tile_count_minus_1 + 1);
						for (size_t i = 0; i < tile_list_entries.size(); i++)
						{
							av1_read_obj(in_bst, tile_list_entries[i]);
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
				BST_FIELD_PROP_2NUMBER1(output_frame_width_in_tiles_minus_1, 8, "plus one is the width of the output frame, in tile units");
				BST_FIELD_PROP_2NUMBER1(output_frame_height_in_tiles_minus_1, 8, "plus one is the height of the output frame, in tile units");
				BST_FIELD_PROP_2NUMBER1(tile_count_minus_1, 16, "plus one is the number of tile_list_entry in the list");
				NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "tile_list_entries", "");
				for (int i = 0; i < tile_count_minus_1 + 1; i++)
				{
					NAV_WRITE_TAG_ARRAY_BEGIN0("Tag00", i, "");
					BST_FIELD_PROP_OBJECT(tile_list_entries[i]);
					NAV_WRITE_TAG_END("Tag00");
				}
				NAV_WRITE_TAG_END("Tag0");
				DECLARE_FIELDPROP_END()
			};

			struct FRAME_HEADER_OBU : public SYNTAX_BITSTREAM_MAP
			{
				struct UNCOMPRESSED_HEADER : public SYNTAX_BITSTREAM_MAP
				{
					struct TEMPORAL_POINT_INFO
					{
						uint32_t	frame_presentation_time;
					}PACKED;

					struct FRAME_SIZE : public SYNTAX_BITSTREAM_MAP
					{
						struct SUPERRES_PARAMS : public SYNTAX_BITSTREAM_MAP
						{
							uint8_t		use_superres : 1;
							uint8_t		coded_denom : SUPERRES_DENOM_BITS;
							uint8_t		enable_superres : 1;
							uint8_t		byte_align : 3;

							uint8_t		SuperresDenom = 0;
							uint16_t	UpscaledWidth = 0;
							uint16_t	FrameWidth = 0;

							FRAME_SIZE*	ptr_frame_size = nullptr;

							SUPERRES_PARAMS(FRAME_SIZE* pFrameSize) 
								: use_superres(0), coded_denom(0), enable_superres(0), byte_align(0), ptr_frame_size(pFrameSize) {
								UpscaledWidth = ptr_frame_size->FrameWidth;
								enable_superres = ptr_frame_size->ptr_uncompressed_header->ptr_sequence_header_obu->enable_superres;
							}

							SUPERRES_PARAMS(uint16_t frame_width, uint8_t bEnableSuperres)
								: use_superres(0), coded_denom(0), enable_superres(bEnableSuperres), byte_align(0), UpscaledWidth(frame_width){
							}

							int Map(AMBst in_bst)
							{
								SYNTAX_BITSTREAM_MAP::Map(in_bst);
								try
								{
									MAP_BST_BEGIN(0);
									if (enable_superres)
									{
										bsrb1(in_bst, use_superres, 1);
									}
									else
										use_superres = 0;

									if (use_superres)
									{
										bsrb1(in_bst, coded_denom, SUPERRES_DENOM_BITS);
										SuperresDenom = coded_denom + SUPERRES_DENOM_MIN;
									}
									else
									{
										SuperresDenom = SUPERRES_NUM;
									}

									FrameWidth = (UpscaledWidth*SUPERRES_NUM + (SuperresDenom / 2)) / SuperresDenom;

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
							if (ptr_frame_size->ptr_uncompressed_header->ptr_sequence_header_obu->enable_superres)
							{
								BST_FIELD_PROP_NUMBER1(use_superres, 1, "");
							}
							else
							{
								NAV_WRITE_TAG_WITH_1NUMBER_VALUE1(use_superres, "");
							}

							if (use_superres)
							{
								BST_FIELD_PROP_2NUMBER1(coded_denom, SUPERRES_DENOM_BITS, "");
								NAV_WRITE_TAG_WITH_NUMBER_VALUE1(SuperresDenom, "SuperresDenom = coded_denom + SUPERRES_DENOM_MIN");
							}
							else
							{
								NAV_WRITE_TAG_WITH_NUMBER_VALUE1(SuperresDenom, "SuperresDenom = SUPERRES_NUM");
							}

							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(UpscaledWidth, "UpscaledWidth = FrameWidth");
							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(FrameWidth, "(UpscaledWidth*SUPERRES_NUM+(SuperresDenom/2))/SuperresDenom");

							DECLARE_FIELDPROP_END()

						}PACKED;

						struct COMPUTE_IMAGE_SIZE
						{
							uint16_t	MiCols;
							uint16_t	MiRows;
						}PACKED;

						uint16_t			frame_width_minus_1 = 0;
						uint16_t			frame_height_minus_1 = 0;

						uint16_t			FrameWidth = 0;
						uint16_t			FrameHeight = 0;

						SUPERRES_PARAMS*	ptr_superres_params = nullptr;
						COMPUTE_IMAGE_SIZE	compute_image_size = { 0, 0 };

						UNCOMPRESSED_HEADER* ptr_uncompressed_header;

						FRAME_SIZE(UNCOMPRESSED_HEADER* pUncompressedHdr)
							: ptr_uncompressed_header(pUncompressedHdr){}

						~FRAME_SIZE() {
							UNMAP_STRUCT_POINTER5(ptr_superres_params);
						}

						int Map(AMBst in_bst)
						{
							SYNTAX_BITSTREAM_MAP::Map(in_bst);
							try
							{
								MAP_BST_BEGIN(0);
								if (ptr_uncompressed_header->frame_size_override_flag)
								{
									uint8_t n = ptr_uncompressed_header->ptr_sequence_header_obu->frame_width_bits_minus_1 + 1;
									bsrb1(in_bst, frame_width_minus_1, n);
									n = ptr_uncompressed_header->ptr_sequence_header_obu->frame_height_bits_minus_1 + 1;
									bsrb1(in_bst, frame_height_minus_1, n);
									FrameWidth = frame_width_minus_1 + 1;
									FrameHeight = frame_height_minus_1 + 1;
								}
								else
								{
									FrameWidth = ptr_uncompressed_header->ptr_sequence_header_obu->max_frame_width_minus_1 + 1;
									FrameHeight = ptr_uncompressed_header->ptr_sequence_header_obu->max_frame_height_minus_1 + 1;
								}

								av1_read_ref(in_bst, ptr_superres_params, SUPERRES_PARAMS, this);

								compute_image_size.MiCols = 2 * ((FrameWidth + 7) >> 3);
								compute_image_size.MiRows = 2 * ((FrameHeight + 7) >> 3);

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
						if (ptr_uncompressed_header->frame_size_override_flag)
						{
							uint8_t n = ptr_uncompressed_header->ptr_sequence_header_obu->frame_width_bits_minus_1 + 1;
							BST_FIELD_PROP_2NUMBER1(frame_width_minus_1, n, "");
							n = ptr_uncompressed_header->ptr_sequence_header_obu->frame_height_bits_minus_1 + 1;
							BST_FIELD_PROP_2NUMBER1(frame_height_minus_1, n, "");
							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(FrameWidth, "FrameWidth = frame_width_minus_1 + 1");
							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(FrameHeight, "FrameHeight = frame_height_minus_1 + 1");
						}
						else
						{
							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(FrameWidth, "FrameWidth = max_frame_width_minus_1 + 1");
							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(FrameHeight, "FrameHeight = max_frame_height_minus_1 + 1");
						}

						BST_FIELD_PROP_REF2_1(ptr_superres_params, "superres_params", "");
						NAV_WRITE_TAG_BEGIN_WITH_ALIAS("compute_image_size", "compute_image_size()", "");
							NAV_WRITE_TAG_WITH_NUMBER_VALUE("MiCols", compute_image_size.MiCols, "");
							NAV_WRITE_TAG_WITH_NUMBER_VALUE("MiRows", compute_image_size.MiRows, "");
						NAV_WRITE_TAG_END("compute_image_size");

						DECLARE_FIELDPROP_END()

					}PACKED;

					struct RENDER_SIZE : public SYNTAX_BITSTREAM_MAP
					{
						uint8_t		render_and_frame_size_different : 1;
						uint8_t		byte_align_0 : 7;

						uint16_t	render_width_minus_1 = 0;
						uint16_t	render_height_minus_1 = 0;

						uint16_t	RenderWidth = 0;
						uint16_t	RenderHeight = 0;

						FRAME_SIZE*	ptr_frame_size = nullptr;

						RENDER_SIZE(FRAME_SIZE* pFrameSize) : render_and_frame_size_different(0), byte_align_0(0), ptr_frame_size(pFrameSize) {
						}

						int Map(AMBst in_bst)
						{
							SYNTAX_BITSTREAM_MAP::Map(in_bst);
							try
							{
								MAP_BST_BEGIN(0);
								bsrb1(in_bst, render_and_frame_size_different, 1);
								if (render_and_frame_size_different)
								{
									bsrb1(in_bst, render_width_minus_1, 16);
									bsrb1(in_bst, render_height_minus_1, 16);
									RenderWidth = render_width_minus_1 + 1;
									RenderHeight = render_height_minus_1 + 1;
								}
								else
								{
									RenderWidth = ptr_frame_size->ptr_superres_params->UpscaledWidth;
									RenderHeight = ptr_frame_size->FrameHeight;
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
							BST_FIELD_PROP_BOOL_BEGIN(render_and_frame_size_different
								, "the render width and height are explicitly coded"
								, "the render width and height are inferred from the frame width and height");
							if (render_and_frame_size_different)
							{
								BST_FIELD_PROP_2NUMBER1(render_width_minus_1, 16, "plus one is the render width of the frame in luma samples");
								BST_FIELD_PROP_2NUMBER1(render_height_minus_1, 16, "plus one is the render height of the frame in luma samples");
								NAV_WRITE_TAG_WITH_NUMBER_VALUE1(RenderWidth, "RenderWidth=render_width_minus_1+1");
								NAV_WRITE_TAG_WITH_NUMBER_VALUE1(RenderHeight, "RenderHeight=render_height_minus_1+1");
							}
							else
							{
								NAV_WRITE_TAG_WITH_NUMBER_VALUE1(RenderWidth, "RenderWidth = UpscaledWidth");
								NAV_WRITE_TAG_WITH_NUMBER_VALUE1(RenderHeight, "RenderHeight = FrameHeight");
							}
							BST_FIELD_PROP_BOOL_END("render_and_frame_size_different");
						DECLARE_FIELDPROP_END()

					}PACKED;

					struct FRAME_SIZE_WITH_REFS : public SYNTAX_BITSTREAM_MAP
					{
						CAMBitArray		found_ref;

						uint32_t		UpscaleWidth = 0;
						uint32_t		FrameWidth = 0;
						uint32_t		FrameHeight = 0;
						uint32_t		RenderWidth = 0;
						uint32_t		RenderHeight = 0;

						FRAME_SIZE*		ptr_frame_size = nullptr;
						RENDER_SIZE*	ptr_render_size = nullptr;
						FRAME_SIZE::SUPERRES_PARAMS*
										ptr_superres_params = nullptr;
						FRAME_SIZE::COMPUTE_IMAGE_SIZE
										compute_image_size = {0, 0};

						UNCOMPRESSED_HEADER*
										ptr_uncompressed_header = nullptr;

						FRAME_SIZE_WITH_REFS(UNCOMPRESSED_HEADER* pUncompressedHdr)
							: ptr_uncompressed_header(pUncompressedHdr) {}

						~FRAME_SIZE_WITH_REFS() {
							UNMAP_STRUCT_POINTER5(ptr_frame_size);
							UNMAP_STRUCT_POINTER5(ptr_render_size);
							UNMAP_STRUCT_POINTER5(ptr_superres_params);
						}

						int Map(AMBst in_bst)
						{
							SYNTAX_BITSTREAM_MAP::Map(in_bst);
							try
							{
								MAP_BST_BEGIN(0);
								int i = 0;
								auto ctx_video_bst = ptr_uncompressed_header->ptr_frame_header_OBU->ptr_OBU->ctx_video_bst;
								auto frame_bufs = ctx_video_bst->buffer_pool->frame_bufs;
								for (; i < REFS_PER_FRAME; i++)
								{
									bsrbarray(in_bst, found_ref, i);
									if (found_ref[i])
									{
										auto frame_buf = frame_bufs[ctx_video_bst->VBI[ctx_video_bst->ref_frame_idx[i]]];
										UpscaleWidth = frame_buf.upscaled_width;
										FrameWidth = frame_buf.width;
										FrameHeight = frame_buf.height;
										RenderWidth = frame_buf.render_width;
										RenderHeight = frame_buf.render_height;
										break;
									}
								}

								if (i >= REFS_PER_FRAME)
								{
									av1_read_ref(in_bst, ptr_frame_size, FRAME_SIZE, ptr_uncompressed_header);
									av1_read_ref(in_bst, ptr_render_size, RENDER_SIZE, ptr_frame_size);
								}
								else
								{
									av1_read_ref(in_bst, ptr_superres_params, FRAME_SIZE::SUPERRES_PARAMS, FrameWidth, 
										ptr_uncompressed_header->ptr_sequence_header_obu->enable_superres);
									compute_image_size.MiCols = 2 * ((FrameWidth + 7) >> 3);
									compute_image_size.MiRows = 2 * ((FrameHeight + 7) >> 3);
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
						NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "for(i=0;i&lt;REFS_PER_FRAME;i++)", "");
						for (i = 0; i < REFS_PER_FRAME; i++)
						{
							BST_ARRAY_FIELD_PROP_BOOL_BEGIN_("found_ref", "ref", i, found_ref[i], 
								"indicates that the frame dimensions can be inferred from reference frame i "
								"where i is the loop counter in the syntax parsing process for frame_size_with_refs", 
								"indicates that the frame dimensions are not inferred from reference frame i");

							if (found_ref[i])
							{

							}

							BST_ARRAY_FIELD_PROP_BOOL_END("found_ref");

							if (found_ref[i])
								break;
						}
						NAV_WRITE_TAG_END("Tag0");

						if (i >= REFS_PER_FRAME)
						{
							BST_FIELD_PROP_REF2_1(ptr_frame_size, "frame_size", "");
							BST_FIELD_PROP_REF2_1(ptr_render_size, "render_size", "");
						}
						else
						{
							BST_FIELD_PROP_REF2_1(ptr_superres_params, "superres_params", "Superres params semantics");
							NAV_WRITE_TAG_BEGIN_WITH_ALIAS("compute_image_size", "compute_image_size()", "Compute image size semantics");
							NAV_WRITE_TAG_WITH_NUMBER_VALUE("MiCols", compute_image_size.MiCols, "the number of 4x4 block columns in the frame");
							NAV_WRITE_TAG_WITH_NUMBER_VALUE("MiRows", compute_image_size.MiRows, "the number of 4x4 block rows in the frame");
							NAV_WRITE_TAG_END("compute_image_size");
						}

						DECLARE_FIELDPROP_END()
					};

					struct READ_INTERPOLATION_FILTER : public SYNTAX_BITSTREAM_MAP
					{
						uint8_t		is_filter_switchable : 1;
						uint8_t		interpolation_filter : 3;
						uint8_t		byte_align : 4;

						READ_INTERPOLATION_FILTER()
							: is_filter_switchable(0), interpolation_filter(0), byte_align(0) {
						}

						int Map(AMBst in_bst)
						{
							SYNTAX_BITSTREAM_MAP::Map(in_bst);
							try
							{
								MAP_BST_BEGIN(0);
								bsrb1(in_bst, is_filter_switchable, 1);
								if (is_filter_switchable == 1)
									interpolation_filter = SWITCHABLE;
								else
								{
									bsrb1(in_bst, interpolation_filter, 2);
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
						BST_FIELD_PROP_BOOL(is_filter_switchable, "", "");
						if (is_filter_switchable)
						{
							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(interpolation_filter, "");
						}
						else
						{
							BST_FIELD_PROP_2NUMBER1(interpolation_filter, 2, "");
						}
						DECLARE_FIELDPROP_END()
					}PACKED;

					struct TILE_INFO : public SYNTAX_BITSTREAM_MAP
					{
						uint16_t		sbCols;
						uint16_t		sbRows;
						uint16_t		sbShift;
						uint16_t		sbSize;
						uint16_t		maxTileWidthSb;
						uint16_t		maxTileAreaSb;
						uint8_t			minLog2TileCols;
						uint8_t			maxLog2TileCols;
						uint8_t			minLog2TileRows;
						uint8_t			maxLog2TileRows;
						uint8_t			minLog2Tiles;

						uint8_t			uniform_tile_spacing_flag : 1;
						uint8_t			byte_align_0 : 7;

						CAMBitArray		increment_tile_cols_log2;
						CAMBitArray		increment_tile_rows_log2;

						std::vector<uint32_t>
										width_in_sbs_minus_1;
						std::vector<uint32_t>
										height_in_sbs_minus_1;

						std::vector<uint16_t>
										MiColStarts;
						std::vector<uint16_t>
										MiRowStarts;

						uint16_t		TileCols;
						uint16_t		TileRows;
						uint16_t		widestTileSb;

						uint32_t		context_update_tile_id;
						uint8_t			tile_size_bytes_minus_1 : 2;
						uint8_t			TileSizeBytes : 3;
						uint8_t			byte_align_1 : 3;

						UNCOMPRESSED_HEADER*
										ptr_uncompressed_header = nullptr;

						TILE_INFO(UNCOMPRESSED_HEADER* pUncompressedHdr) : ptr_uncompressed_header(pUncompressedHdr) {
						}

						int Map(AMBst in_bst)
						{
							SYNTAX_BITSTREAM_MAP::Map(in_bst);
							try
							{
								MAP_BST_BEGIN(0);

								uint16_t MiCols = 0, MiRows = 0;
								if (ptr_uncompressed_header->ptr_frame_size_with_refs != nullptr)
								{
									if (ptr_uncompressed_header->ptr_frame_size_with_refs->found_ref[0] == 1)
									{
										MiCols = ptr_uncompressed_header->ptr_frame_size_with_refs->ptr_frame_size->compute_image_size.MiCols;
										MiRows = ptr_uncompressed_header->ptr_frame_size_with_refs->ptr_frame_size->compute_image_size.MiRows;
									}
									else
									{
										MiCols = ptr_uncompressed_header->ptr_frame_size_with_refs->compute_image_size.MiCols;
										MiRows = ptr_uncompressed_header->ptr_frame_size_with_refs->compute_image_size.MiRows;
									}
								}
								else if (ptr_uncompressed_header->ptr_frame_size != nullptr)
								{
									MiCols = ptr_uncompressed_header->ptr_frame_size->compute_image_size.MiCols;
									MiRows = ptr_uncompressed_header->ptr_frame_size->compute_image_size.MiRows;
								}
								else
									return RET_CODE_BUFFER_NOT_COMPATIBLE;

								sbCols = ptr_uncompressed_header->ptr_sequence_header_obu->use_128x128_superblock ? ((MiCols + 31) >> 5) : ((MiCols + 15) >> 4);
								sbRows = ptr_uncompressed_header->ptr_sequence_header_obu->use_128x128_superblock ? ((MiRows + 31) >> 5) : ((MiRows + 15) >> 4);
								sbShift = ptr_uncompressed_header->ptr_sequence_header_obu->use_128x128_superblock ? 5 : 4;
								sbSize = sbShift + 2;

								maxTileWidthSb = MAX_TILE_WIDTH >> sbSize;
								maxTileAreaSb = MAX_TILE_AREA >> (2 * sbSize);

								minLog2TileCols = tile_log2(maxTileWidthSb, sbCols);
								maxLog2TileCols = tile_log2(1, AMP_MIN(sbCols, MAX_TILE_COLS));
								maxLog2TileRows = tile_log2(1, AMP_MIN(sbRows, MAX_TILE_ROWS));

								minLog2Tiles = AMP_MAX(minLog2TileCols, tile_log2(maxTileAreaSb, sbRows*sbCols));

								bsrb1(in_bst, uniform_tile_spacing_flag, 1);
								
								uint8_t TileColsLog2 = 0, TileRowsLog2 = 0;
								if (uniform_tile_spacing_flag)
								{
									TileColsLog2 = minLog2TileCols;
									while (TileColsLog2 < maxLog2TileCols)
									{
										bsrbarray(in_bst, increment_tile_cols_log2, TileColsLog2 - minLog2TileCols);
										if (increment_tile_cols_log2[TileColsLog2 - minLog2TileCols])
											TileColsLog2++;
										else
											break;
									}

									uint16_t tileWidthSb = (sbCols + (1 << TileColsLog2) - 1) >> TileColsLog2;
									uint16_t i = 0;
									MiColStarts.resize((size_t)sbCols + 1);
									for (uint16_t startSb = 0; startSb < sbCols; startSb += tileWidthSb) {
										MiColStarts[i] = startSb << sbShift;
										i++;
									}
									MiColStarts[i] = MiCols;
									TileCols = i;

									minLog2TileRows = 0;
									if (minLog2Tiles > TileColsLog2)
										minLog2TileRows = minLog2Tiles - TileColsLog2;

									TileRowsLog2 = minLog2TileRows;
									while (TileRowsLog2 < maxLog2TileRows)
									{
										bsrbarray(in_bst, increment_tile_rows_log2, TileRowsLog2 - minLog2TileRows);
										if (increment_tile_rows_log2[TileRowsLog2 - minLog2TileRows])
											TileRowsLog2++;
										else
											break;
									}

									uint16_t tileHeightSb = (sbRows + (1 << TileRowsLog2) - 1) >> TileRowsLog2;
									i = 0;
									MiRowStarts.resize((size_t)sbRows + 1);
									for (uint16_t startSb = 0; startSb < sbRows; startSb += tileHeightSb) {
										MiRowStarts[i] = startSb << sbShift;
										i++;
									}

									MiRowStarts[i] = MiRows;
									TileRows = i;
								}
								else
								{
									widestTileSb = 0;
									uint16_t startSb = 0, i = 0;
									for (; startSb < sbCols; i++)
									{
										MiColStarts.push_back(startSb << sbShift);
										uint16_t maxWidth = AMP_MIN(sbCols - startSb, maxTileWidthSb);
										uint32_t nWidthInSbsMinus1;
										av1_read_ns(in_bst, nWidthInSbsMinus1, maxWidth);
										width_in_sbs_minus_1.push_back(nWidthInSbsMinus1);
										uint16_t sizeSb = nWidthInSbsMinus1 + 1;
										widestTileSb = AMP_MAX(sizeSb, widestTileSb);
										startSb += sizeSb;
									}

									MiColStarts.push_back(MiCols);
									TileCols = i;
									TileColsLog2 = tile_log2(1, TileCols);

									if (minLog2Tiles > 0)
										maxTileAreaSb = (sbRows * sbCols) >> (minLog2Tiles + 1);
									else
										maxTileAreaSb = sbRows * sbCols;

									uint16_t maxTileHeightSb = AMP_MAX(maxTileAreaSb / widestTileSb, 1);

									startSb = 0;
									if (sbRows)

									for (uint16_t i = 0; startSb < sbRows; i++) {
										MiRowStarts.push_back(startSb << sbShift);
										uint16_t maxHeight = AMP_MIN(sbRows - startSb, maxTileHeightSb);
										uint32_t nHeightInSbsMinus1;
										av1_read_ns(in_bst, nHeightInSbsMinus1, maxHeight);
										height_in_sbs_minus_1.push_back(nHeightInSbsMinus1);
										uint16_t sizeSb = nHeightInSbsMinus1 + 1;
										startSb += sizeSb;
									}

									MiRowStarts.push_back(MiRows);
									TileRows = i;
									TileRowsLog2 = tile_log2(1, TileRows);
								}

								if (TileColsLog2 > 0 || TileRowsLog2 > 0)
								{
									bsrb1(in_bst, context_update_tile_id, TileRowsLog2 + TileColsLog2);
									bsrb1(in_bst, tile_size_bytes_minus_1, 2);
									TileSizeBytes = tile_size_bytes_minus_1 + 1;
								}
								else
								{
									context_update_tile_id = 0;
									TileSizeBytes = 0;
								}

								ptr_uncompressed_header->ptr_frame_header_OBU->ptr_OBU->ctx_video_bst->tile_cols = TileCols;
								ptr_uncompressed_header->ptr_frame_header_OBU->ptr_OBU->ctx_video_bst->tile_rows = TileRows;
								ptr_uncompressed_header->ptr_frame_header_OBU->ptr_OBU->ctx_video_bst->log2_tile_cols = TileColsLog2;
								ptr_uncompressed_header->ptr_frame_header_OBU->ptr_OBU->ctx_video_bst->log2_tile_rows = TileRowsLog2;

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
						uint16_t MiCols = 0, MiRows = 0;
						if (ptr_uncompressed_header->ptr_frame_size_with_refs != nullptr)
						{
							if (ptr_uncompressed_header->ptr_frame_size_with_refs->found_ref[0] == 1)
							{
								MiCols = ptr_uncompressed_header->ptr_frame_size_with_refs->ptr_frame_size->compute_image_size.MiCols;
								MiRows = ptr_uncompressed_header->ptr_frame_size_with_refs->ptr_frame_size->compute_image_size.MiRows;
							}
							else
							{
								MiCols = ptr_uncompressed_header->ptr_frame_size_with_refs->compute_image_size.MiCols;
								MiRows = ptr_uncompressed_header->ptr_frame_size_with_refs->compute_image_size.MiRows;
							}
						}
						else if (ptr_uncompressed_header->ptr_frame_size != nullptr)
						{
							MiCols = ptr_uncompressed_header->ptr_frame_size->compute_image_size.MiCols;
							MiRows = ptr_uncompressed_header->ptr_frame_size->compute_image_size.MiRows;
						}
						else
							goto done;

						{
							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(sbCols, 
								ptr_uncompressed_header->ptr_sequence_header_obu->use_128x128_superblock ? "((MiCols + 31) &gt;&gt; 5)" : "((MiCols + 15) &gt;&gt; 4)");
							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(sbRows, 
								ptr_uncompressed_header->ptr_sequence_header_obu->use_128x128_superblock ? "((MiRows + 31) &gt;&gt; 5)" : "((MiRows + 15) &gt;&gt; 4)");
							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(sbShift, ptr_uncompressed_header->ptr_sequence_header_obu->use_128x128_superblock ? "5" : "4");
							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(sbSize, "sbShift + 2");

							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(maxTileWidthSb, "maxTileWidthSb = MAX_TILE_WIDTH &gt;&gt; sbSize");
							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(maxTileAreaSb, "maxTileAreaSb = MAX_TILE_AREA &gt;&gt; (2 * sbSize)");

							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(minLog2TileCols, "minLog2TileCols = tile_log2(maxTileWidthSb, sbCols)");
							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(maxLog2TileCols, "maxLog2TileCols = tile_log2(1, AMP_MIN(sbCols, MAX_TILE_COLS))");
							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(maxLog2TileRows, "maxLog2TileRows = tile_log2(1, AMP_MIN(sbRows, MAX_TILE_ROWS))");

							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(minLog2Tiles, "minLog2Tiles = Max(minLog2TileCols, tile_log2(maxTileAreaSb, sbRows*sbCols))");

							uint8_t TileColsLog2 = 0, TileRowsLog2 = 0;
							BST_FIELD_PROP_BOOL_BEGIN(uniform_tile_spacing_flag, "the tiles are uniformly spaced across the frame", "the tile sizes are coded");

							if (uniform_tile_spacing_flag)
							{
								NAV_WRITE_TAG_WITH_ALIAS_F("TileColsLog2", "TileColsLog2 = minLog2TileCols", "");
								for (i = 0; i <= increment_tile_cols_log2.UpperBound(); i++)
								{
									BST_ARRAY_FIELD_PROP_NUMBER1(increment_tile_cols_log2, i, 1, "");
								}
								NAV_WRITE_TAG_WITH_NUMBER_VALUE1(TileColsLog2, "specifies the base 2 logarithm of the desired number of tiles across the frame");

								uint16_t tileWidthSb = (sbCols + (1 << TileColsLog2) - 1) >> TileColsLog2;
								NAV_WRITE_TAG_WITH_NUMBER_VALUE1(tileWidthSb, "tileWidthSb=(sbCols+(1&lt;&lt;TileColsLog2)-1)&gt;&gt;TileColsLog2");

								NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("Tag0", "for(startSb=0;startSb&lt;sbCols;startSb+=tileWidthSb)", "");
								i = 0;
								for (size_t startSb = 0; startSb < sbCols; startSb += tileWidthSb) {
									NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag01", "MiColStarts[%d]", 
										(int)(startSb << sbShift), "MiColStarts[i]=startSb&lt;&lt;sbShift", i);
									i++;
								}
								NAV_WRITE_TAG_END("Tag0");
								NAV_WRITE_TAG_WITH_NUMBER_VALUE1(TileCols, "specifies the number of tiles across the frame, not greater than MAX_TILE_COLS");

								NAV_WRITE_TAG_WITH_NUMBER_VALUE1(minLog2TileRows, "minLog2TileRows = Max(minLog2Tiles-TileColsLog2, 0)");

								NAV_WRITE_TAG_WITH_ALIAS_F("TileRowsLog2", "TileRowsLog2 = minLog2TileRows", "");
								for (i = 0; i <= increment_tile_rows_log2.UpperBound(); i++)
								{
									BST_ARRAY_FIELD_PROP_NUMBER1(increment_tile_rows_log2, i, 1, "");
								}
								NAV_WRITE_TAG_WITH_NUMBER_VALUE1(TileRowsLog2, "specifies the base 2 logarithm of the desired number of tiles down the frame");

								uint16_t tileHeightSb = (sbRows + (1 << TileRowsLog2) - 1) >> TileRowsLog2;
								NAV_WRITE_TAG_WITH_NUMBER_VALUE1(tileHeightSb, "tileHeightSb=(sbRows+(1&lt;&lt;TileRowsLog2)-1)&gt;&gt;TileRowsLog2");

								NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("Tag1", "for(startSb=0;startSb&lt;sbRows;startSb+=tileHeightSb)", "");
								i = 0;
								for (size_t startSb = 0; startSb < sbRows; startSb += tileHeightSb) {
									NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag11", "MiRowStarts[%d]", 
										(int)(startSb << sbShift), "MiRowStarts[i]=startSb&lt;&lt;sbShift", i);
									i++;
								}
								NAV_WRITE_TAG_END("Tag1");
								NAV_WRITE_TAG_WITH_NUMBER_VALUE1(TileRows, "specifies the number of tiles down the frame, not greater than MAX_TILE_ROWS");
							}
							else
							{
								NAV_WRITE_TAG_WITH_ALIAS_F("widestTileSb", "widestTileSb = 0", "");
								uint16_t startSb = 0, i = 0;
								NAV_WRITE_TAG_WITH_ALIAS_F("startSb", "startSb = 0", "");
								NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("Tag0", "for(i=0; startSb&lt;sbCols;i++)", "");
								for (; startSb < sbCols; i++)
								{
									NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("Tag00", "[%d]", "", i);
									NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag001", "MiColStarts[%d]", MiColStarts[i], "MiColStarts[i]=startSb&lt;&lt;sbShift", i);
									uint16_t maxWidth = AMP_MIN(sbCols - startSb, maxTileWidthSb);
									NAV_WRITE_TAG_WITH_NUMBER_VALUE1(maxWidth, "maxWidth=Min(sbCols-startSb, maxTileWidthSb)");
									BST_FIELD_PROP_2NUMBER("width_in_sbs_minus_1", ns_len(maxWidth, width_in_sbs_minus_1[i]), width_in_sbs_minus_1[i], "");
									uint16_t sizeSb = width_in_sbs_minus_1[i] + 1;
									startSb += sizeSb;
									NAV_WRITE_TAG_WITH_NUMBER_VALUE1(startSb, "");
									NAV_WRITE_TAG_END("Tag00");
								}
								NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag01", "MiColStarts[%d]", MiColStarts[i], "", i);
								NAV_WRITE_TAG_END("Tag0");

								NAV_WRITE_TAG_WITH_NUMBER_VALUE1(widestTileSb, "");
								NAV_WRITE_TAG_WITH_NUMBER_VALUE1(TileCols, "specifies the number of tiles across the frame, not greater than MAX_TILE_COLS");
								NAV_WRITE_TAG_WITH_NUMBER_VALUE1(TileColsLog2, "TileColsLog2 = tile_log2(1, TileCols)");

								uint16_t maxTileHeightSb = AMP_MAX(maxTileAreaSb / widestTileSb, 1);
								NAV_WRITE_TAG_WITH_NUMBER_VALUE1(maxTileHeightSb, "");

								startSb = 0;
								NAV_WRITE_TAG_WITH_ALIAS_F("startSb", "startSb = 0", "");
								NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("Tag1", "for(i=0; startSb&lt;sbRows;i++)", "");
								for (; startSb < sbRows; i++)
								{
									NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("Tag10", "[%d]", "", i);
									NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag001", "MiRowStarts[%d]", MiRowStarts[i], "MiRowStarts[i]=startSb&lt;&lt;sbShift", i);
									uint16_t maxHeight = AMP_MIN(sbRows - startSb, maxTileHeightSb);
									NAV_WRITE_TAG_WITH_NUMBER_VALUE1(maxHeight, "maxHeight=Min(sbRows-startSb, maxTileHeightSb)");
									BST_FIELD_PROP_2NUMBER("height_in_sbs_minus_1", ns_len(maxHeight, height_in_sbs_minus_1[i]), height_in_sbs_minus_1[i], "");
									uint16_t sizeSb = height_in_sbs_minus_1[i] + 1;
									startSb += sizeSb;
									NAV_WRITE_TAG_WITH_NUMBER_VALUE1(startSb, "");
									NAV_WRITE_TAG_END("Tag10");
								}
								NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag11", "MiRowStarts[%d]", MiRowStarts[i], "", i);
								NAV_WRITE_TAG_END("Tag1");

								NAV_WRITE_TAG_WITH_NUMBER_VALUE1(TileRows, "specifies the number of tiles down the frame, not greater than MAX_TILE_ROWS");
								NAV_WRITE_TAG_WITH_NUMBER_VALUE1(TileRowsLog2, 
									"TileRowsLog2 = tile_log2(1, TileRows), specifies the base 2 logarithm of the desired number of tiles down the frame");
							}

							BST_FIELD_PROP_BOOL_END("uniform_tile_spacing_flag");

							if (TileColsLog2 > 0 || TileRowsLog2 > 0)
							{
								BST_FIELD_PROP_2NUMBER1(context_update_tile_id, (long long)TileRowsLog2 + TileColsLog2, "specifies which tile to use for the CDF update");
								BST_FIELD_PROP_2NUMBER1(tile_size_bytes_minus_1, 2, "plus 1 specifies the number of bytes needed to code each tile size");
							}
							else
							{
								NAV_WRITE_TAG_WITH_NUMBER_VALUE1(context_update_tile_id, "Should be 0");
								NAV_WRITE_TAG_WITH_NUMBER_VALUE1(TileSizeBytes, "Should be 0");
							}
						}

						done:
						DECLARE_FIELDPROP_END()

					};

					struct QUANTIZATION_PARAMS : public SYNTAX_BITSTREAM_MAP
					{
						struct READ_DELTA_Q : public SYNTAX_BITSTREAM_MAP
						{
							uint8_t		delta_coded : 1;
							uint8_t		byte_align : 7;

							int8_t		delta_q = 0;

							READ_DELTA_Q() : delta_coded(0), byte_align(0) {
							}

							int Map(AMBst in_bst)
							{
								SYNTAX_BITSTREAM_MAP::Map(in_bst);
								try
								{
									MAP_BST_BEGIN(0);
									bsrb1(in_bst, delta_coded, 1);
									if (delta_coded)
									{
										delta_q = (int8_t)AMBst_Get_su(in_bst, 7);
										map_status.number_of_fields++;
									}
									else
										delta_q = 0;
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
							BST_FIELD_PROP_BOOL_BEGIN(delta_coded, "the delta_q syntax element is present", "the delta_q syntax element is NOT present");
							if (delta_q) {
								BST_FIELD_PROP_2NUMBER1(delta_q, 7, "an offset (relative to base_q_idx) for a particular quantization parameter");
							}
							else {
								NAV_WRITE_TAG_WITH_NUMBER_VALUE1(delta_q, "Should be 0");
							}
							BST_FIELD_PROP_BOOL_END("delta_coded");
							DECLARE_FIELDPROP_END()
						}PACKED;

						uint8_t			base_q_idx;

						uint8_t			diff_uv_delta : 1;
						uint8_t			using_qmatrix : 1;
						uint8_t			qm_y : 4;
						uint8_t			byte_align_0 : 2;

						uint8_t			qm_u : 4;
						uint8_t			qm_v : 4;

						int8_t			DeltaQYDc;
						int8_t			DeltaQUDc;
						int8_t			DeltaQUAc;
						int8_t			DeltaQVDc;
						int8_t			DeltaQVAc;

						std::vector<READ_DELTA_Q>
										read_delta_q;
						UNCOMPRESSED_HEADER*
										ptr_uncompressed_header = nullptr;

						QUANTIZATION_PARAMS(UNCOMPRESSED_HEADER* pUncompressedHdr)
							: ptr_uncompressed_header(pUncompressedHdr) {
						}

						int Map(AMBst in_bst)
						{
							SYNTAX_BITSTREAM_MAP::Map(in_bst);
							try
							{
								MAP_BST_BEGIN(0);
								bsrb1(in_bst, base_q_idx, 8);

								read_delta_q.emplace_back();
								av1_read_obj(in_bst, read_delta_q[0]);
								DeltaQYDc = read_delta_q[0].delta_q;

								diff_uv_delta = 0;
								if (!ptr_uncompressed_header->ptr_sequence_header_obu->color_config->mono_chrome)
								{
									if (ptr_uncompressed_header->ptr_sequence_header_obu->color_config->separate_uv_delta_q)
									{
										bsrb1(in_bst, diff_uv_delta, 1);
									}

									read_delta_q.emplace_back();
									av1_read_obj(in_bst, read_delta_q[1]);
									DeltaQUDc = read_delta_q[1].delta_q;

									read_delta_q.emplace_back();
									av1_read_obj(in_bst, read_delta_q[2]);
									DeltaQUAc = read_delta_q[2].delta_q;

									if (diff_uv_delta)
									{
										read_delta_q.emplace_back();
										av1_read_obj(in_bst, read_delta_q[3]);
										DeltaQVDc = read_delta_q[3].delta_q;

										read_delta_q.emplace_back();
										av1_read_obj(in_bst, read_delta_q[4]);
										DeltaQVAc = read_delta_q[4].delta_q;
									}
									else
									{
										DeltaQVDc = DeltaQUDc;
										DeltaQVAc = DeltaQUAc;
									}
								}
								else
								{
									DeltaQUDc = 0;
									DeltaQUAc = 0;
									DeltaQVDc = 0;
									DeltaQVAc = 0;
								}

								bsrb1(in_bst, using_qmatrix, 1);
								if (using_qmatrix)
								{
									bsrb1(in_bst, qm_y, 4);
									bsrb1(in_bst, qm_u, 4);

									if (ptr_uncompressed_header->ptr_sequence_header_obu->color_config->separate_uv_delta_q)
									{
										bsrb1(in_bst, qm_v, 4);
									}
									else
										qm_v = qm_u;
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
						BST_FIELD_PROP_2NUMBER1(base_q_idx, 8, "the base frame qindex. This is used for Y AC coefficients and as the base value for the other quantizers");

						NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("Tag0", "read_delta_q[0]", "DeltaQYDc = read_delta_q()");
						BST_FIELD_PROP_OBJECT(read_delta_q[0]);
						NAV_WRITE_TAG_END("Tag0");

						NAV_WRITE_TAG_WITH_NUMBER_VALUE1(DeltaQYDc, "the Y DC quantizer relative to base_q_idx");

						if (!ptr_uncompressed_header->ptr_sequence_header_obu->color_config->mono_chrome)
						{
							if (ptr_uncompressed_header->ptr_sequence_header_obu->color_config->separate_uv_delta_q)
							{
								BST_FIELD_PROP_BOOL(diff_uv_delta, "the U and V delta quantizer values are coded separately", "the U and V delta quantizer values share a common value");
							}
							else
							{
								NAV_WRITE_TAG_WITH_NUMBER_VALUE1(diff_uv_delta, "Should be 0, the U and V delta quantizer values share a common value");
							}

							NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("Tag1", "read_delta_q[1]", "DeltaQUDc = read_delta_q()");
							BST_FIELD_PROP_OBJECT(read_delta_q[1]);
							NAV_WRITE_TAG_END("Tag1");

							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(DeltaQUDc, "the U DC quantizer relative to base_q_idx");

							NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("Tag2", "read_delta_q[2]", "DeltaQUAc = read_delta_q()");
							BST_FIELD_PROP_OBJECT(read_delta_q[2]);
							NAV_WRITE_TAG_END("Tag2");

							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(DeltaQUAc, "the U AC quantizer relative to base_q_idx");

							if (diff_uv_delta)
							{
								NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("Tag3", "read_delta_q[3]", "DeltaQVDc = read_delta_q()");
								BST_FIELD_PROP_OBJECT(read_delta_q[3]);
								NAV_WRITE_TAG_END("Tag3");

								NAV_WRITE_TAG_WITH_NUMBER_VALUE1(DeltaQVDc, "the V DC quantizer relative to base_q_idx");

								NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("Tag4", "read_delta_q[4]", "DeltaQVAc = read_delta_q()");
								BST_FIELD_PROP_OBJECT(read_delta_q[4]);
								NAV_WRITE_TAG_END("Tag4");

								NAV_WRITE_TAG_WITH_NUMBER_VALUE1(DeltaQVAc, "the V AC quantizer relative to base_q_idx");
							}
							else
							{
								NAV_WRITE_TAG_WITH_NUMBER_VALUE1(DeltaQVDc, "DeltaQVDc = DeltaQUDc, the V DC quantizer relative to base_q_idx");
								NAV_WRITE_TAG_WITH_NUMBER_VALUE1(DeltaQVAc, "DeltaQVAc = DeltaQVAc, the V AC quantizer relative to base_q_idx");
							}
						}
						else
						{
							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(DeltaQUDc, "Should be 0, the U DC quantizer relative to base_q_idx");
							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(DeltaQUAc, "Should be 0, the U AC quantizer relative to base_q_idx");
							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(DeltaQVDc, "Should be 0, the V DC quantizer relative to base_q_idx");
							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(DeltaQVAc, "Should be 0, the V AC quantizer relative to base_q_idx");
						}

						if (using_qmatrix)
						{
							BST_FIELD_PROP_BOOL_BEGIN(using_qmatrix, "the quantizer matrix will be used to compute quantizers", "the quantizer matrix will NOT be used to compute quantizers");

							BST_FIELD_PROP_2NUMBER1(qm_y, 4, "the level in the quantizer matrix that should be used for luma plane decoding");
							BST_FIELD_PROP_2NUMBER1(qm_u, 4, "the level in the quantizer matrix that should be used for chroma U plane decoding");

							if (ptr_uncompressed_header->ptr_sequence_header_obu->color_config->separate_uv_delta_q)
							{
								BST_FIELD_PROP_2NUMBER1(qm_v, 4, "the level in the quantizer matrix that should be used for chroma V plane decoding");
							}
							else
							{
								NAV_WRITE_TAG_WITH_NUMBER_VALUE1(qm_v, "qm_v = qm_u");
							}

							BST_FIELD_PROP_BOOL_END("using_qmatrix");
						}
						else
						{
							BST_FIELD_PROP_BOOL(using_qmatrix, "the quantizer matrix will be used to compute quantizers", "the quantizer matrix will NOT be used to compute quantizers");
						}
						DECLARE_FIELDPROP_END()

					};

					struct SEGMENTATION_PARAMS : public SYNTAX_BITSTREAM_MAP
					{
						uint8_t		segmentation_enabled : 1;
						uint8_t		segmentation_update_map : 1;
						uint8_t		segmentation_temporal_update : 1;
						uint8_t		segmentation_update_data : 1;
						uint8_t		byte_aligned : 4;

						CAMBitArray	feature_enabled;
						int16_t		feature_value[MAX_SEGMENTS][SEG_LVL_MAX] = { {0} };
						uint8_t		FeatureEnabled[MAX_SEGMENTS][SEG_LVL_MAX] = { {0} };
						int16_t		FeatureData[MAX_SEGMENTS][SEG_LVL_MAX] = { {0} };

						uint8_t		SegIdPreSkip = 0;
						uint8_t		LastActiveSegId = 0;

						UNCOMPRESSED_HEADER*
									ptr_uncompressed_header = nullptr;

						SEGMENTATION_PARAMS(UNCOMPRESSED_HEADER* pUncompressedHdr) 
							: segmentation_enabled(0), segmentation_update_map(0), segmentation_temporal_update(0), segmentation_update_data(0)
							, ptr_uncompressed_header(pUncompressedHdr) {
						}

						int Map(AMBst in_bst)
						{
							SYNTAX_BITSTREAM_MAP::Map(in_bst);
							try
							{
								MAP_BST_BEGIN(0);
								bsrb1(in_bst, segmentation_enabled, 1);
								if (segmentation_enabled)
								{
									if (ptr_uncompressed_header->primary_ref_frame == PRIMARY_REF_NONE)
									{
										segmentation_update_map = 1;
										segmentation_temporal_update = 0;
										segmentation_update_data = 1;
									}
									else
									{
										bsrb1(in_bst, segmentation_update_map, 1);
										if (segmentation_update_map)
										{
											bsrb1(in_bst, segmentation_temporal_update, 1);
										}
										bsrb1(in_bst, segmentation_update_data, 1);
									}

									if (segmentation_update_data == 1) {
										for (uint8_t i = 0; i < MAX_SEGMENTS; i++) {
											for (uint8_t j = 0; j < SEG_LVL_MAX; j++) {
												feature_value[i][j] = 0;
												bsrbarray(in_bst, feature_enabled, i*j);
												FeatureEnabled[i][j] = feature_enabled[i*j] ? 1 : 0;

												int16_t clippedValue = 0;
												if (feature_enabled[i*j])
												{
													uint8_t bitsToRead = Segmentation_Feature_Bits[j];
													uint8_t limit = Segmentation_Feature_Max[j];

													if (Segmentation_Feature_Signed[j] == 1)
													{
														feature_value[i][j] = (int16_t)AMBst_Get_su(in_bst, 1 + bitsToRead);
														map_status.number_of_fields++;
														clippedValue = AV1_Clip3(-limit, limit, feature_value[i][j]);
													}
													else
													{
														bsrb1(in_bst, feature_value[i][j], bitsToRead);
														clippedValue = AV1_Clip3(0, limit, feature_value[i][j]);
													}
												}

												FeatureData[i][j] = clippedValue;
											}
										}
									}
								}
								else
								{
									for (uint8_t i = 0; i < MAX_SEGMENTS; i++) {
										for (uint8_t j = 0; j < SEG_LVL_MAX; j++) {
											FeatureEnabled[i][j] = 0;
											FeatureData[i][j] = 0;
										}
									}
								}

								SegIdPreSkip = 0;
								LastActiveSegId = 0;

								for (uint8_t i = 0; i < MAX_SEGMENTS; i++) {
									for (uint8_t j = 0; j < SEG_LVL_MAX; j++) {
										if (FeatureEnabled[i][j]) {
											LastActiveSegId = i;
											if (j >= SEG_LVL_REF_FRAME) {
												SegIdPreSkip = 1;
											}
										}
									}
								}

								// Update it to the current frame in BufferPool
								auto ctx_video_bst = ptr_uncompressed_header->ptr_frame_header_OBU->ptr_OBU->ctx_video_bst;
								auto curFrame = &ctx_video_bst->buffer_pool->frame_bufs[ctx_video_bst->cfbi];

								// The function save_segmentation_params( i ) is invoked
								// save_segmentation_params( i ) is a function call that indicates that the values of FeatureEnabled[j][k] and 
								// FeatureData[j][k] for j = 0 .. MAX_SEGMENTS-1, for k = 0 .. SEG_LVL_MAX-1 should be saved into an area of memory indexed by i
								memcpy(curFrame->FeatureEnabled, FeatureEnabled, sizeof(curFrame->FeatureEnabled));
								memcpy(curFrame->FeatureData, FeatureData, sizeof(curFrame->FeatureData));

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
						bool bAllDisabled = true;
						BST_FIELD_PROP_BOOL_BEGIN(segmentation_enabled, "this frame makes use of the segmentation tool", "the frame does not use segmentation");
						if (segmentation_enabled)
						{
							if (ptr_uncompressed_header->primary_ref_frame == PRIMARY_REF_NONE)
							{
								NAV_WRITE_TAG_WITH_NUMBER_VALUE1(segmentation_update_map, "Should be 1, the segmentation map are updated during the decoding of this frame");
								NAV_WRITE_TAG_WITH_NUMBER_VALUE1(segmentation_temporal_update, "Should be 0");
								NAV_WRITE_TAG_WITH_NUMBER_VALUE1(segmentation_update_data, "Should be 1");
							}
							else
							{
								BST_FIELD_PROP_BOOL_BEGIN(segmentation_update_map, "the segmentation map are updated during the decoding of this frame", 
									"the segmentation map from the previous frame is used");
								if (segmentation_update_map)
								{
									BST_FIELD_PROP_BOOL(segmentation_temporal_update,
										"the updates to the segmentation map are coded relative to the existing segmentation map",
										"the new segmentation map is coded without reference to the existing segmentation map");
								}
								BST_FIELD_PROP_BOOL_END("segmentation_update_map");
								BST_FIELD_PROP_BOOL(segmentation_update_data, "new parameters are about to be specified for each segment", 
									"the segmentation parameters should keep their existing values");
							}

							if (segmentation_update_data == 1) {
								NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("Tag0", "for(i=0;i&lt;MAX_SEGMENTS(%d);i++)", "", MAX_SEGMENTS);
								for (uint8_t i = 0; i < MAX_SEGMENTS; i++) {
									NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("Tag00", "for(j=0;j&lt;SEG_LVL_MAX(%d);j++)", "", SEG_LVL_MAX);
									for (uint8_t j = 0; j < SEG_LVL_MAX; j++) {
										NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("Tag000", "[%d][%d]", "", i, j);
										BST_FIELD_PROP_NUMBER("feature_enabled", 1, feature_enabled[i*j], 
											feature_enabled[i*j]?"the feature value is coded":"the corresponding feature is unused and has value equal to 0");
										int16_t clippedValue = 0;
										if (feature_enabled[i*j])
										{
											uint8_t bitsToRead = Segmentation_Feature_Bits[j];
											uint8_t limit = Segmentation_Feature_Max[j];

											if (Segmentation_Feature_Signed[j] == 1)
											{
												BST_FIELD_PROP_2NUMBER("feature_value", 1LL + bitsToRead, feature_value[i][j], "the feature data for a segment feature");
												clippedValue = AV1_Clip3(-limit, limit, feature_value[i][j]);
												NAV_WRITE_TAG_WITH_NUMBER_VALUE1(clippedValue, "Clip3(-limit, limit, feature_value)");
											}
											else
											{
												BST_FIELD_PROP_2NUMBER("feature_value", bitsToRead, feature_value[i][j], "the feature data for a segment feature");
												clippedValue = AV1_Clip3(0, limit, feature_value[i][j]);
												NAV_WRITE_TAG_WITH_NUMBER_VALUE1(clippedValue, "Clip3(0, limit, feature_value)");
											}
										}

										NAV_WRITE_TAG_WITH_NUMBER_VALUE("FeatureData", FeatureData[i][j], "");

										NAV_WRITE_TAG_END("Tag000");
									}
									NAV_WRITE_TAG_END("Tag00");
								}
								NAV_WRITE_TAG_END("Tag0");
							}
						}
						else
						{
							NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("Tag0", "for(i=0;i&lt;MAX_SEGMENTS(%d);i++)", "", MAX_SEGMENTS);
							for (uint8_t i = 0; i < MAX_SEGMENTS; i++) {
								NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("Tag00", "[segmentId%d][%s,%s,%s,%s,%s,%s,%s,%s]", "", i,
									SEG_LVL_NAMEA(0), SEG_LVL_NAMEA(1), SEG_LVL_NAMEA(2), SEG_LVL_NAMEA(3),
									SEG_LVL_NAMEA(4), SEG_LVL_NAMEA(5), SEG_LVL_NAMEA(6), SEG_LVL_NAMEA(7));
									NAV_WRITE_TAG_WITH_VALUEFMTSTR("FeatureEnabled", "%d,%d,%d,%d,%d,%d,%d,%d", "",
										FeatureEnabled[i][0], FeatureEnabled[i][1], FeatureEnabled[i][2], FeatureEnabled[i][3], 
										FeatureEnabled[i][4], FeatureEnabled[i][5], FeatureEnabled[i][6], FeatureEnabled[i][7]);
									NAV_WRITE_TAG_WITH_VALUEFMTSTR("FeatureData", "%d,%d,%d,%d,%d,%d,%d,%d", "",
										FeatureData[i][0], FeatureData[i][1], FeatureData[i][2], FeatureData[i][3],
										FeatureData[i][4], FeatureData[i][5], FeatureData[i][6], FeatureData[i][7]);
								NAV_WRITE_TAG_END("Tag00");

								//NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("Tag00", "for(j=0;j&lt;SEG_LVL_MAX(%d);j++)", "", SEG_LVL_MAX);
								//for (uint8_t j = 0; j < SEG_LVL_MAX; j++) {
								//	NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("Tag000", "[%d][%d]", "", i, j);
								//	NAV_WRITE_TAG_WITH_NUMBER_VALUE("FeatureEnabled", FeatureEnabled[i][j], "");
								//	NAV_WRITE_TAG_WITH_NUMBER_VALUE("FeatureData", FeatureData[i][j], "");
								//	NAV_WRITE_TAG_END("Tag000");
								//}
								//NAV_WRITE_TAG_END("Tag00");
							}
							NAV_WRITE_TAG_END("Tag0");
						}
						BST_FIELD_PROP_BOOL_END("segmentation_enabled");

						NAV_WRITE_TAG_WITH_ALIAS_F("Tag1", "SegIdPrevSkip = 0", "");
						NAV_WRITE_TAG_WITH_ALIAS_F("Tag2", "LastActiveSegId = 0", "");

						for (uint8_t i = 0; i < MAX_SEGMENTS; i++) {
							for (uint8_t j = 0; j < SEG_LVL_MAX; j++) {
								if (FeatureEnabled[i][j]) {
									bAllDisabled = false;
									i = MAX_SEGMENTS;
									break;
								}
							}
						}

						if (!bAllDisabled) {
							NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("Tag1", "for(i=0;i&lt;MAX_SEGMENTS(%d);i++)", "", MAX_SEGMENTS);
							for (uint8_t i = 0; i < MAX_SEGMENTS; i++) {
								NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("Tag10", "for(j=0;j&lt;SEG_LVL_MAX(%d);j++)", "", SEG_LVL_MAX);
								for (uint8_t j = 0; j < SEG_LVL_MAX; j++) {
									if (FeatureEnabled[i][j]) {
										NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("Tag100", "[%d][%d]", "", i, j);
										NAV_WRITE_TAG_WITH_ALIAS_F("Tag101", "SegIdPrevSkip = %d", "", i);
										if (j >= SEG_LVL_REF_FRAME) {
											NAV_WRITE_TAG_WITH_ALIAS_F("Tag102", "LastActiveSegId = 1", "");
										}
										NAV_WRITE_TAG_END("Tag100");
									}
								}
								NAV_WRITE_TAG_END("Tag10");
							}

								NAV_WRITE_TAG_WITH_NUMBER_VALUE1(SegIdPreSkip, SegIdPreSkip 
									? "the segment id will be read before the skip syntax element" 
									: "the skip syntax element will be read first");
								NAV_WRITE_TAG_WITH_1NUMBER_VALUE1(LastActiveSegId, "the highest numbered segment id that has some enabled feature. "
									"This is used when decoding the segment id to only decode choices corresponding to used segments");
							NAV_WRITE_TAG_END("Tag1");
						}
						DECLARE_FIELDPROP_END()

					};

					// 5.9.17. Quantizer index delta parameters syntax
					struct DELTA_Q_PARAMS : public SYNTAX_BITSTREAM_MAP
					{
						uint8_t		delta_q_res : 1;
						uint8_t		delta_q_present : 2;
						uint8_t		byte_align : 5;

						UNCOMPRESSED_HEADER*
									ptr_uncompressed_header = nullptr;

						DELTA_Q_PARAMS(UNCOMPRESSED_HEADER* pUncompressedHdr)
							: delta_q_res(0), delta_q_present(0), byte_align(0)
							, ptr_uncompressed_header(pUncompressedHdr){}

						int Map(AMBst in_bst)
						{
							SYNTAX_BITSTREAM_MAP::Map(in_bst);
							try
							{
								MAP_BST_BEGIN(0);
								delta_q_res = 0;
								delta_q_present = 0;

								if (ptr_uncompressed_header->ptr_quantization_params->base_q_idx > 0)
								{
									bsrb1(in_bst, delta_q_present, 1);
								}

								if (delta_q_present) {
									bsrb1(in_bst, delta_q_res, 2);
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
						if (ptr_uncompressed_header->ptr_quantization_params->base_q_idx > 0)
						{
							BST_FIELD_PROP_BOOL(delta_q_present, "quantizer index delta values are present", "quantizer index delta values are NOT present");
						}
						else
						{
							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(delta_q_present, "Should be 0, quantizer index delta values are NOT present");
						}

						if (delta_q_present)
						{
							BST_FIELD_PROP_2NUMBER1(delta_q_res, 2, "the left shift which should be applied to decoded quantizer index delta values");
						}
						else
						{
							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(delta_q_res, "Should be 0, the left shift which should be applied to decoded quantizer index delta values");
						}

						DECLARE_FIELDPROP_END()

					}PACKED;

					// 5.9.18. Loop filter delta parameters syntax
					struct DELTA_LF_PARAMS : public SYNTAX_BITSTREAM_MAP
					{
						uint8_t		delta_lf_present : 1;
						uint8_t		delta_lf_res : 2;
						uint8_t		delta_lf_multi : 1;

						UNCOMPRESSED_HEADER*
									ptr_uncompressed_header = nullptr;

						DELTA_LF_PARAMS(UNCOMPRESSED_HEADER* pUncompressedHdr) 
							: delta_lf_present(0), delta_lf_res(0), delta_lf_multi(0)
							, ptr_uncompressed_header(pUncompressedHdr) {}

						int Map(AMBst in_bst)
						{
							SYNTAX_BITSTREAM_MAP::Map(in_bst);
							try
							{
								MAP_BST_BEGIN(0);
								delta_lf_present = 0;
								delta_lf_res = 0;
								delta_lf_multi = 0;

								if (ptr_uncompressed_header->ptr_delta_q_params->delta_q_present > 0)
								{
									if (!ptr_uncompressed_header->allow_intrabc)
									{
										bsrb1(in_bst, delta_lf_present, 1);
									}

									if (delta_lf_present)
									{
										bsrb1(in_bst, delta_lf_res, 2);
										bsrb1(in_bst, delta_lf_multi, 1);
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
						if (ptr_uncompressed_header->ptr_delta_q_params->delta_q_present > 0)
						{
							if (!ptr_uncompressed_header->allow_intrabc)
							{
								BST_FIELD_PROP_BOOL(delta_lf_present, "loop filter delta values are present", "loop filter delta values are NOT present");
							}
							else
							{
								NAV_WRITE_TAG_WITH_NUMBER_VALUE1(delta_lf_present, "Should be 0, loop filter delta values are NOT present");
							}

							if (delta_lf_present)
							{
								BST_FIELD_PROP_2NUMBER1(delta_lf_res, 2, "the left shift which should be applied to decoded loop filter delta values");
								BST_FIELD_PROP_BOOL(delta_lf_multi, 
									"separate loop filter deltas are sent for horizontal luma edges, vertical luma edges, the U edges, and the V edges", 
									"the same loop filter delta is used for all edges");
							}
							else
							{
								NAV_WRITE_TAG_WITH_NUMBER_VALUE1(delta_lf_res, "Should be 0, the left shift which should be applied to decoded loop filter delta values");
								NAV_WRITE_TAG_WITH_NUMBER_VALUE1(delta_lf_multi, delta_lf_multi?
									"separate loop filter deltas are sent for horizontal luma edges, vertical luma edges, the U edges, and the V edges": 
									"the same loop filter delta is used for all edges");
							}
						}
						DECLARE_FIELDPROP_END()
					}PACKED;

					struct LOOP_FILTER_PARAMS : public SYNTAX_BITSTREAM_MAP
					{
						uint8_t		loop_filter_level[4] = { 0, };
						
						uint8_t		loop_filter_sharpness : 3;
						uint8_t		loop_filter_delta_enabled : 1;
						uint8_t		loop_filter_delta_update : 1;
						uint8_t		byte_align_0 : 3;

						CAMBitArray	update_ref_delta;
						int8_t		loop_filter_ref_deltas[TOTAL_REFS_PER_FRAME];

						CAMBitArray	update_mode_delta;
						int8_t		loop_filter_mode_deltas[2];

						UNCOMPRESSED_HEADER*
									ptr_uncompressed_header = nullptr;

						LOOP_FILTER_PARAMS(UNCOMPRESSED_HEADER* pUncompressedHDR)
							: ptr_uncompressed_header(pUncompressedHDR){}

						int Map(AMBst in_bst)
						{
							SYNTAX_BITSTREAM_MAP::Map(in_bst);
							try
							{
								MAP_BST_BEGIN(0);
								if (ptr_uncompressed_header->CodedLossless || ptr_uncompressed_header->allow_intrabc)
								{
									loop_filter_level[0] = loop_filter_level[1] = 0;

									loop_filter_ref_deltas[INTRA_FRAME] = 1;
									loop_filter_ref_deltas[LAST_FRAME] = 0;
									loop_filter_ref_deltas[LAST2_FRAME] = 0;
									loop_filter_ref_deltas[LAST3_FRAME] = 0;
									loop_filter_ref_deltas[BWDREF_FRAME] = 0;
									loop_filter_ref_deltas[GOLDEN_FRAME] = -1;
									loop_filter_ref_deltas[ALTREF_FRAME] = -1;
									loop_filter_ref_deltas[ALTREF2_FRAME] = -1;

									for (int i = 0; i < 2; i++) {
										loop_filter_mode_deltas[i] = 0;
									}
								}
								else
								{
									bsrb1(in_bst, loop_filter_level[0], 6);
									bsrb1(in_bst, loop_filter_level[1], 6);

									if (!ptr_uncompressed_header->ptr_sequence_header_obu->color_config->mono_chrome)
									{
										if (loop_filter_level[0] || loop_filter_level[1])
										{
											bsrb1(in_bst, loop_filter_level[2], 6);
											bsrb1(in_bst, loop_filter_level[3], 6);
										}
									}

									bsrb1(in_bst, loop_filter_sharpness, 3);
									bsrb1(in_bst, loop_filter_delta_enabled, 1);

									if (loop_filter_delta_enabled)
									{
										bsrb1(in_bst, loop_filter_delta_update, 1);
										if (loop_filter_delta_update)
										{
											for (int i = 0; i < TOTAL_REFS_PER_FRAME; i++)
											{
												bsrbarray(in_bst, update_ref_delta, i);
												if (update_ref_delta[i]) {
													loop_filter_ref_deltas[i] = (int8_t)AMBst_Get_su(in_bst, 7);
													map_status.number_of_fields++;
												}
											}

											for (int i = 0; i < 2; i++)
											{
												bsrbarray(in_bst, update_mode_delta, i);
												if (update_mode_delta[i]) {
													loop_filter_mode_deltas[i] = (int8_t)AMBst_Get_su(in_bst, 7);
													map_status.number_of_fields++;
												}
											}
										}
									}
								}

								// Update it to the current frame in BufferPool
								auto ctx_video_bst = ptr_uncompressed_header->ptr_frame_header_OBU->ptr_OBU->ctx_video_bst;
								auto curFrame = &ctx_video_bst->buffer_pool->frame_bufs[ctx_video_bst->cfbi];

								// The function save_loop_filter_params( i ) is invoked (see below).
								// save_loop_filter_params( i ) is a function call that indicates that the values of loop_filter_ref_deltas[j] for j = 0 ..
								// TOTAL_REFS_PER_FRAME-1, and the values of loop_filter_mode_deltas[j] for j = 0 .. 1 should be saved into an area of
								// memory indexed by i
								memcpy(curFrame->loop_filter_ref_deltas, loop_filter_ref_deltas, sizeof(curFrame->loop_filter_ref_deltas));
								memcpy(curFrame->loop_filter_mode_deltas, loop_filter_mode_deltas, sizeof(curFrame->loop_filter_mode_deltas));

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
						if (ptr_uncompressed_header->CodedLossless || ptr_uncompressed_header->allow_intrabc)
						{
							NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("loop_filter_level", "loop_filter_level[0]", loop_filter_level[0], "Should be 0");
							NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("loop_filter_level", "loop_filter_level[1]", loop_filter_level[1], "Should be 0");

							NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("loop_filter_ref_deltas", "loop_filter_ref_deltas[INTRA_FRAME]", 
								loop_filter_ref_deltas[INTRA_FRAME], "Should be 1");
							NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("loop_filter_ref_deltas", "loop_filter_ref_deltas[LAST_FRAME]", 
								loop_filter_ref_deltas[LAST_FRAME], "Should be 0");
							NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("loop_filter_ref_deltas", "loop_filter_ref_deltas[LAST2_FRAME]", 
								loop_filter_ref_deltas[LAST2_FRAME], "Should be 0");
							NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("loop_filter_ref_deltas", "loop_filter_ref_deltas[LAST3_FRAME]", 
								loop_filter_ref_deltas[LAST3_FRAME], "Should be 0");
							NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("loop_filter_ref_deltas", "loop_filter_ref_deltas[GOLDEN_FRAME]", 
								loop_filter_ref_deltas[GOLDEN_FRAME], "Should be -1");
							NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("loop_filter_ref_deltas", "loop_filter_ref_deltas[BWDREF_FRAME]", 
								loop_filter_ref_deltas[BWDREF_FRAME], "Should be 0");
							NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("loop_filter_ref_deltas", "loop_filter_ref_deltas[ALTREF2_FRAME]", 
								loop_filter_ref_deltas[ALTREF2_FRAME], "Should be -1");
							NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("loop_filter_ref_deltas", "loop_filter_ref_deltas[ALTREF_FRAME]", 
								loop_filter_ref_deltas[ALTREF_FRAME], "Should be -1");

							NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("loop_filter_mode_deltas", "loop_filter_mode_deltas[0]", loop_filter_mode_deltas[0], "Should be 0");
							NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("loop_filter_mode_deltas", "loop_filter_mode_deltas[1]", loop_filter_mode_deltas[1], "Should be 0");
						}
						else
						{
							if (!ptr_uncompressed_header->ptr_sequence_header_obu->color_config->mono_chrome && (loop_filter_level[0] || loop_filter_level[1]))
							{
								BST_FIELD_PROP_NUMBER_ARRAY_F("loop_filter_level", 24, "%d, %d, %d, %d", "loop filter strength value",
									loop_filter_level[0], loop_filter_level[1], loop_filter_level[2], loop_filter_level[3]);
							}
							else
							{
								BST_FIELD_PROP_NUMBER_ARRAY_F("loop_filter_level", 12, "%d, %d", "loop filter strength value",
									loop_filter_level[0], loop_filter_level[1]);
							}

							BST_FIELD_PROP_2NUMBER1(loop_filter_sharpness, 3, 
								"indicates the sharpness level. The loop_filter_level and loop_filter_sharpness together determine"
								"when a block edge is filtered, and by how much the filtering can change the sample values");
							BST_FIELD_PROP_BOOL_BEGIN(loop_filter_delta_enabled, 
								"the filter level depends on the mode and reference frame used to predict a block", 
								"the filter level does not depend on the mode and reference frame");

							if (loop_filter_delta_enabled)
							{
								BST_FIELD_PROP_BOOL_BEGIN(loop_filter_delta_update, 
									"additional syntax elements are present that specify which mode and reference frame deltas are to be updated", 
									"additional syntax related with mode and reference frame deltas update are not presented");
								if (loop_filter_delta_update)
								{
									NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "for(i=0;i&lt;TOTAL_REFS_PER_FRAME;i++)", "");
									for (int i = 0; i < TOTAL_REFS_PER_FRAME; i++)
									{
										BST_ARRAY_FIELD_PROP_BOOL_BEGIN(update_ref_delta, "RefFrame#", i, 
											"the syntax element loop_filter_ref_delta is present", "the syntax element loop_filter_ref_delta is NOT present");
										if (update_ref_delta[i]) {
											BST_ARRAY_FIELD_PROP_NUMBER("loop_filter_ref_deltas", i, 7, loop_filter_ref_deltas[i], 
												"contains the adjustment needed for the filter level based on the chosen reference frame"
												"If this syntax element is not present, it maintains its previous value");
										}
										BST_ARRAY_FIELD_PROP_BOOL_END(update_ref_delta);
									}
									NAV_WRITE_TAG_END("Tag0");

									NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag1", "for(i=0;i&lt;2;i++)", "");
									for (int i = 0; i < 2; i++)
									{
										BST_ARRAY_FIELD_PROP_BOOL_BEGIN(update_mode_delta, "", i,
											"the syntax element loop_filter_mode_deltas is present", "the syntax element loop_filter_mode_deltas is NOT present");
										if (update_mode_delta[i]) {
											BST_ARRAY_FIELD_PROP_NUMBER("loop_filter_mode_deltas", i, 7, loop_filter_mode_deltas[i],
												"contains the adjustment needed for the filter level based on the chosen mode"
												"If this syntax element is not present, it maintains its previous value");
										}
										BST_ARRAY_FIELD_PROP_BOOL_END(update_mode_delta);
									}
									NAV_WRITE_TAG_END("Tag1");
								}
								BST_FIELD_PROP_BOOL_END("loop_filter_delta_update");
							}

							BST_FIELD_PROP_BOOL_END("loop_filter_delta_enabled");
						}
						DECLARE_FIELDPROP_END()

					};

					// 5.9.19. CDEF params syntax
					struct CDEF_PARAMS : public SYNTAX_BITSTREAM_MAP 
					{
						union CDEFFilter
						{
							uint8_t				bytes[2];
							struct
							{
								uint8_t			cdef_y_pri_strength : 4;
								uint8_t			cdef_y_sec_strength : 2;
								uint8_t			byte_align_0 : 2;

								uint8_t			cdef_uv_pri_strength : 4;
								uint8_t			cdef_uv_sec_strength : 2;
								uint8_t			byte_align_1 : 2;
							}PACKED;
						}PACKED;

						uint8_t			cdef_damping_minus_3 : 2;
						uint8_t			cdef_bits : 2;
						uint8_t			CdefDamping : 4;

						CDEFFilter		filters[8] = { {{0}} };

						UNCOMPRESSED_HEADER*	
										ptr_uncompressed_header = nullptr;

						CDEF_PARAMS(UNCOMPRESSED_HEADER* pUncompressedHdr)
							: cdef_damping_minus_3(0), cdef_bits(0), CdefDamping(0)
							, ptr_uncompressed_header(pUncompressedHdr){}

						INLINE uint8_t GetCdefYSecStrength(uint8_t filter_id)
						{
							if (filters[filter_id].cdef_y_sec_strength == 3)
								return 4;
							return filters[filter_id].cdef_y_sec_strength;
						}

						INLINE uint8_t GetCdefUVSecStrength(uint8_t filter_id)
						{
							if (filters[filter_id].cdef_uv_sec_strength == 3)
								return 4;
							return filters[filter_id].cdef_uv_sec_strength;
						}

						int Map(AMBst in_bst)
						{
							SYNTAX_BITSTREAM_MAP::Map(in_bst);
							try
							{
								MAP_BST_BEGIN(0);
								if (ptr_uncompressed_header->CodedLossless || ptr_uncompressed_header->allow_intrabc || !ptr_uncompressed_header->ptr_sequence_header_obu->enable_cdef)
								{
									cdef_bits = 0;
									cdef_damping_minus_3 = 0;
									CdefDamping = 3;
									filters[0].cdef_y_pri_strength = 0;
									filters[0].cdef_y_sec_strength = 0;
									filters[0].cdef_uv_pri_strength = 0;
									filters[0].cdef_uv_sec_strength = 0;
								}
								else
								{
									bsrb1(in_bst, cdef_damping_minus_3, 2);
									CdefDamping = cdef_damping_minus_3 + 3;
									bsrb1(in_bst, cdef_bits, 2);

									for (int i = 0; i < (1 << cdef_bits); i++)
									{
										bsrb1(in_bst, filters[i].cdef_y_pri_strength, 4);
										bsrb1(in_bst, filters[i].cdef_y_sec_strength, 2);
										
										if (!ptr_uncompressed_header->ptr_sequence_header_obu->color_config->mono_chrome)
										{
											bsrb1(in_bst, filters[i].cdef_uv_pri_strength, 4);
											bsrb1(in_bst, filters[i].cdef_uv_sec_strength, 2);
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
						if (ptr_uncompressed_header->CodedLossless || ptr_uncompressed_header->allow_intrabc || !ptr_uncompressed_header->ptr_sequence_header_obu->enable_cdef)
						{
							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(cdef_bits, "the number of bits needed to specify which CDEF filter to apply");
							NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("cdef_y_pri_strength", "cdef_y_pri_strength[0]", filters[0].cdef_y_pri_strength, "");
							NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("cdef_y_sec_strength", "cdef_y_sec_strength[0]", filters[0].cdef_y_sec_strength, "");
							NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("cdef_uv_pri_strength", "cdef_uv_pri_strength[0]", filters[0].cdef_uv_pri_strength, "");
							NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("cdef_uv_sec_strength", "cdef_uv_sec_strength[0]", filters[0].cdef_uv_sec_strength, "");
							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(CdefDamping, "");
						}
						else
						{
							BST_FIELD_PROP_2NUMBER1(cdef_damping_minus_3, 2, "controls the amount of damping in the deringing filter");
							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(CdefDamping, "cdef_damping_minus_3 + 3");
							BST_FIELD_PROP_2NUMBER1(cdef_bits, 2, "the number of bits needed to specify which CDEF filter to apply");

							NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "for (i=0;i&lt;(1&lt;&lt;cdef_bits);i++)", "");
							for (int i = 0; i < (1 << cdef_bits); i++)
							{
								NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("cdef_filter", "[cdef_filter#%d]", "", i);
								BST_FIELD_PROP_2NUMBER("cdef_y_pri_strength", 4, filters[i].cdef_y_pri_strength, "specify the strength of the primary filter");
								BST_FIELD_PROP_2NUMBER("cdef_y_sec_strength", 2, filters[i].cdef_y_sec_strength, "specify the strength of the secondary filter");
								
								if (!ptr_uncompressed_header->ptr_sequence_header_obu->color_config->mono_chrome)
								{
									BST_FIELD_PROP_2NUMBER("cdef_uv_pri_strength", 4, filters[i].cdef_uv_pri_strength, "specify the strength of the primary filter");
									BST_FIELD_PROP_2NUMBER("cdef_uv_sec_strength", 2, filters[i].cdef_uv_sec_strength, "specify the strength of the secondary filter");
								}
								NAV_WRITE_TAG_END("cdef_filter");
							}
							NAV_WRITE_TAG_END("Tag0");
						}
						DECLARE_FIELDPROP_END()
					}PACKED;

					// 5.9.20. Loop restoration params syntax
					struct LR_PARAMS : public SYNTAX_BITSTREAM_MAP
					{
						uint8_t		lr_type[3] = { 0 };
						uint8_t		lr_unit_shift : 1;
						uint8_t		lr_unit_extra_shift : 1;
						uint8_t		lr_uv_shift : 1;
						uint8_t		usesLr : 1;
						uint8_t		usesChromaLr : 1;
						uint8_t		byte_align : 3;

						uint8_t		FrameRestorationType[3] = { 0 };
						uint8_t		LoopRestorationSize[3] = { 0 };

						UNCOMPRESSED_HEADER*
									ptr_uncompressed_header = nullptr;

						LR_PARAMS(UNCOMPRESSED_HEADER* pUncompressedHdr) 
							: lr_unit_shift(0), lr_unit_extra_shift(0), lr_uv_shift(0), usesLr(0), usesChromaLr(0), byte_align(0)
							, ptr_uncompressed_header(pUncompressedHdr) {}

						uint8_t GetLRUnitShift()
						{
							if (usesLr)
							{
								if (ptr_uncompressed_header->ptr_sequence_header_obu->use_128x128_superblock)
									return lr_unit_shift + 1;
								else if (lr_unit_shift)
									return lr_unit_shift + lr_unit_extra_shift;
								else
									return lr_unit_shift;
							}

							return 0xFF;
						}

						int Map(AMBst in_bst)
						{
							SYNTAX_BITSTREAM_MAP::Map(in_bst);
							try
							{
								MAP_BST_BEGIN(0);
								
								if (ptr_uncompressed_header->AllLossless || ptr_uncompressed_header->allow_intrabc || !ptr_uncompressed_header->ptr_sequence_header_obu->enable_restoration)
								{
									FrameRestorationType[0] = RESTORE_NONE;
									FrameRestorationType[1] = RESTORE_NONE;
									FrameRestorationType[2] = RESTORE_NONE;
									usesLr = 1;
								}
								else
								{
									uint8_t Remap_Lr_Type[4] = {
										RESTORE_NONE, RESTORE_SWITCHABLE, RESTORE_WIENER, RESTORE_SGRPROJ
									};

									usesLr = 0;
									usesChromaLr = 0;
									for (int i = 0; i < (ptr_uncompressed_header->ptr_sequence_header_obu->color_config->mono_chrome ? 1 : 3); i++)
									{
										bsrb1(in_bst, lr_type[i], 2);
										FrameRestorationType[i] = Remap_Lr_Type[lr_type[i]];
										if (FrameRestorationType[i] != RESTORE_NONE) {
											usesLr = 1;
											if (i > 0)
												usesChromaLr = 1;
										}
									}

									if (usesLr)
									{
										if (ptr_uncompressed_header->ptr_sequence_header_obu->use_128x128_superblock)
										{
											bsrb1(in_bst, lr_unit_shift, 1);
										}
										else
										{
											bsrb1(in_bst, lr_unit_shift, 1);
											if (lr_unit_shift) {
												bsrb1(in_bst, lr_unit_extra_shift, 1);
											}
										}

										LoopRestorationSize[0] = RESTORATION_TILESIZE_MAX >> (2 - GetLRUnitShift());
										if (ptr_uncompressed_header->ptr_sequence_header_obu->color_config->subsampling_x &&
											ptr_uncompressed_header->ptr_sequence_header_obu->color_config->subsampling_y &&
											usesChromaLr)
										{
											bsrb1(in_bst, lr_uv_shift, 1);
										}
										else
											lr_uv_shift = 0;

										LoopRestorationSize[1] = LoopRestorationSize[0] >> lr_uv_shift;
										LoopRestorationSize[2] = LoopRestorationSize[0] >> lr_uv_shift;
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
						if (ptr_uncompressed_header->AllLossless || ptr_uncompressed_header->allow_intrabc || !ptr_uncompressed_header->ptr_sequence_header_obu->enable_restoration)
						{
							NAV_WRITE_TAG_WITH_VALUEFMTSTR("FrameRestorationType", "%s,%s,%s", "All should be RESTORE_NONE(0)",
								GetFrameRestorationTypeName(FrameRestorationType[0]), GetFrameRestorationTypeName(FrameRestorationType[1]), GetFrameRestorationTypeName(FrameRestorationType[2]));
							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(usesLr, "");
						}
						else
						{
							if (ptr_uncompressed_header->ptr_sequence_header_obu->color_config->mono_chrome)
							{
								BST_FIELD_PROP_NUMBER_ARRAY_F("lr_type", 2, "%d", "is used to compute FrameRestorationType", lr_type[0]);
								NAV_WRITE_TAG_WITH_VALUEFMTSTR("FrameRestorationType", "%s", "", GetFrameRestorationTypeName(FrameRestorationType[0]));
							}
							else
							{
								BST_FIELD_PROP_NUMBER_ARRAY_F("lr_type", 6, "%d,%d,%d", "is used to compute FrameRestorationType", lr_type[0], lr_type[1], lr_type[2]);
								NAV_WRITE_TAG_WITH_VALUEFMTSTR("FrameRestorationType", "%s,%s,%s", "", 
									GetFrameRestorationTypeName(FrameRestorationType[0]), GetFrameRestorationTypeName(FrameRestorationType[1]), GetFrameRestorationTypeName(FrameRestorationType[2]));
							}

							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(usesLr, "indicates if any plane uses loop restoration");
							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(usesChromaLr, "indicates if any plane uses chroma loop restoration");

							if (usesLr)
							{
								if (ptr_uncompressed_header->ptr_sequence_header_obu->use_128x128_superblock)
								{
									BST_FIELD_PROP_BOOL(lr_unit_shift, "the luma restoration size should be halved", "the luma restoration size should NOT be halved");
								}
								else
								{
									BST_FIELD_PROP_BOOL(lr_unit_shift, "the luma restoration size should be halved", "the luma restoration size should NOT be halved");
									if (lr_unit_shift) {
										BST_FIELD_PROP_BOOL(lr_unit_extra_shift, "the luma restoration size should be halved again", "the luma restoration size should NOT be halved again");
									}
								}

								if (ptr_uncompressed_header->ptr_sequence_header_obu->color_config->subsampling_x &&
									ptr_uncompressed_header->ptr_sequence_header_obu->color_config->subsampling_y &&
									usesChromaLr)
								{
									BST_FIELD_PROP_BOOL(lr_uv_shift, "the chroma size should be half the luma size", "the chroma size should NOT be half the luma size");
								}
								else
								{
									NAV_WRITE_TAG_WITH_1NUMBER_VALUE1(lr_uv_shift, "Should be 0, the chroma size should NOT be half the luma size");
								}
							}
						}
						DECLARE_FIELDPROP_END()

					}PACKED;

					// 5.9.21. TX mode syntax
					struct READ_TX_MODE : public SYNTAX_BITSTREAM_MAP
					{
						uint8_t		tx_mode_select : 1;
						uint8_t		TxMode : 2;
						uint8_t		byte_align : 5;

						UNCOMPRESSED_HEADER*
									ptr_uncompressed_header = nullptr;

						READ_TX_MODE(UNCOMPRESSED_HEADER* pUncompressedHdr) 
							: tx_mode_select(0), TxMode(0), byte_align(0)
							, ptr_uncompressed_header(pUncompressedHdr) {}

						int Map(AMBst in_bst)
						{
							SYNTAX_BITSTREAM_MAP::Map(in_bst);
							try
							{
								MAP_BST_BEGIN(0);
								if (ptr_uncompressed_header->CodedLossless)
								{
									TxMode = ONLY_4X4;
								}
								else
								{
									bsrb1(in_bst, tx_mode_select, 1);
									if (tx_mode_select)
									{
										TxMode = TX_MODE_SELECT;
									}
									else
									{
										TxMode = TX_MODE_LARGEST;
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
						if (ptr_uncompressed_header->CodedLossless)
						{
							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(TxMode, "ONLY_4X4, the inverse transform will use only 4x4 transforms");
						}
						else
						{
							BST_FIELD_PROP_BOOL(tx_mode_select, "TX_MODE_SELECT", "TX_MODE_LARGEST");
							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(TxMode, TX_MODE_DESC(TxMode));
						}
						DECLARE_FIELDPROP_END()
					}PACKED;

					// 5.9.23. Frame reference mode syntax
					struct FRAME_REFERENCE_MODE : public SYNTAX_BITSTREAM_MAP
					{
						uint8_t		reference_select : 1;
						uint8_t		byte_align : 7;

						UNCOMPRESSED_HEADER*
									ptr_uncompressed_header = nullptr;

						FRAME_REFERENCE_MODE(UNCOMPRESSED_HEADER* pUncompressedHdr) 
							: reference_select(0), byte_align(0), ptr_uncompressed_header(pUncompressedHdr) {}

						int Map(AMBst in_bst)
						{
							SYNTAX_BITSTREAM_MAP::Map(in_bst);
							try
							{
								MAP_BST_BEGIN(0);
								if (ptr_uncompressed_header->frame_type == INTRA_ONLY_FRAME ||
									ptr_uncompressed_header->frame_type == KEY_FRAME)
								{
									reference_select = 0;
								}
								else
								{
									bsrb1(in_bst, reference_select, 1);
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
						if (ptr_uncompressed_header->frame_type == INTRA_ONLY_FRAME ||
							ptr_uncompressed_header->frame_type == KEY_FRAME)
						{
							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(reference_select, "Should be 0, specifies that all inter blocks will use single prediction");
						}
						else
						{
							BST_FIELD_PROP_BOOL(reference_select, 
								"specifies that the mode info for inter blocks contains the syntax element comp_mode that indicates whether to use single or compound reference prediction", 
								"specifies that all inter blocks will use single prediction");
						}
						DECLARE_FIELDPROP_END()

					}PACKED;

					// 5.9.22. Skip mode params syntax
					struct SKIP_MODE_PARAMS : public SYNTAX_BITSTREAM_MAP
					{
						uint8_t		skipModeAllowed:1;
						uint8_t		skip_mode_present : 1;
						uint8_t		byte_align : 6;

						int8_t		forwardIdx = 0;
						uint8_t		forwardHint = 0;
						int8_t		backwardIdx = 0;
						uint8_t		backwardHint = 0;
						int8_t		secondForwardIdx = 0;
						uint8_t		secondForwardHint = 0;

						uint8_t		SkipModeFrame[2] = { 0 };

						UNCOMPRESSED_HEADER*
									ptr_uncompressed_header = nullptr;

						SKIP_MODE_PARAMS(UNCOMPRESSED_HEADER* pUncompressedHdr) 
							: skipModeAllowed(0), skip_mode_present(0), byte_align(0), ptr_uncompressed_header(pUncompressedHdr) {}

						int Map(AMBst in_bst)
						{
							SYNTAX_BITSTREAM_MAP::Map(in_bst);
							try
							{
								MAP_BST_BEGIN(0);
								auto FrameIsIntra = (ptr_uncompressed_header->frame_type == INTRA_ONLY_FRAME ||
													 ptr_uncompressed_header->frame_type == KEY_FRAME);

								if (FrameIsIntra 
									|| !ptr_uncompressed_header->ptr_frame_reference_mode->reference_select 
									|| !ptr_uncompressed_header->ptr_sequence_header_obu->enable_order_hint)
								{
									skipModeAllowed = 0;
								}
								else
								{
									forwardIdx = -1;
									backwardIdx = -1;
									auto ctx_video_bst = ptr_uncompressed_header->ptr_frame_header_OBU->ptr_OBU->ctx_video_bst;
									auto frame_bufs = ctx_video_bst->buffer_pool->frame_bufs;
									auto cur_frame = &ctx_video_bst->buffer_pool->frame_bufs[ctx_video_bst->cfbi];
									auto OrderHint = cur_frame->current_order_hint;
									for (int i = 0; i < REFS_PER_FRAME; i++)
									{
										const int buf_idx = ctx_video_bst->VBI[ctx_video_bst->ref_frame_idx[i]];
										if (buf_idx == -1)
											continue;

										uint8_t refHint = frame_bufs[buf_idx].current_order_hint;
										if (ptr_uncompressed_header->ptr_sequence_header_obu->get_relative_dist(refHint, OrderHint) < 0)
										{
											if (forwardIdx < 0 ||
												ptr_uncompressed_header->ptr_sequence_header_obu->get_relative_dist(refHint, forwardHint) > 0)
											{
												forwardIdx = i;
												forwardHint = refHint;
											}
										}
										else if (ptr_uncompressed_header->ptr_sequence_header_obu->get_relative_dist(refHint, OrderHint) > 0)
										{
											if (backwardIdx < 0 ||
												ptr_uncompressed_header->ptr_sequence_header_obu->get_relative_dist(refHint, backwardHint) < 0)
											{
												backwardIdx = i;
												backwardHint = refHint;
											}
										}
									}

									if (forwardIdx < 0) 
									{
										skipModeAllowed = 0;
									}
									else if (backwardIdx >= 0) {
										// == Bi-directional prediction ==
										skipModeAllowed = 1;
										SkipModeFrame[0] = LAST_FRAME + Min(forwardIdx, backwardIdx);
										SkipModeFrame[1] = LAST_FRAME + Max(forwardIdx, backwardIdx);
									}
									else
									{
										// == Forward prediction only ==
										// Identify the second nearest forward reference.
										secondForwardIdx = -1;
										for (int i = 0; i < REFS_PER_FRAME; i++)
										{
											const int buf_idx = ctx_video_bst->VBI[ctx_video_bst->ref_frame_idx[i]];
											if (buf_idx == -1)
												continue;

											uint8_t refHint = frame_bufs[buf_idx].current_order_hint;
											if (ptr_uncompressed_header->ptr_sequence_header_obu->get_relative_dist(refHint, forwardHint) < 0) {
												if (secondForwardIdx < 0 ||
													ptr_uncompressed_header->ptr_sequence_header_obu->get_relative_dist(refHint, secondForwardHint) > 0)
												{
													secondForwardIdx = i;
													secondForwardHint = refHint;
												}
											}
										}
										if (secondForwardIdx < 0) 
										{
											skipModeAllowed = 0;
										}
										else
										{
											skipModeAllowed = 1;
											SkipModeFrame[0] = LAST_FRAME + Min(forwardIdx, secondForwardIdx);
											SkipModeFrame[1] = LAST_FRAME + Max(forwardIdx, secondForwardIdx);
										}
									}
								}

								if (skipModeAllowed) {
									bsrb1(in_bst, skip_mode_present, 1);
								}
								else
									skip_mode_present = 0;
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
						auto FrameIsIntra = (ptr_uncompressed_header->frame_type == INTRA_ONLY_FRAME ||
											 ptr_uncompressed_header->frame_type == KEY_FRAME);

						if (FrameIsIntra 
							|| !ptr_uncompressed_header->ptr_frame_reference_mode->reference_select 
							|| !ptr_uncompressed_header->ptr_sequence_header_obu->enable_order_hint)
						{
							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(skipModeAllowed, "");
						}
						else
						{
							if (forwardIdx > -1)
							{
								NAV_WRITE_TAG_WITH_NUMBER_VALUE1(forwardIdx, "");
								NAV_WRITE_TAG_WITH_NUMBER_VALUE1(forwardHint, "");
							}

							if (backwardIdx > -1)
							{
								NAV_WRITE_TAG_WITH_NUMBER_VALUE1(backwardIdx, "");
								NAV_WRITE_TAG_WITH_NUMBER_VALUE1(backwardHint, "");
							}

							if (forwardIdx < 0)
							{
								NAV_WRITE_TAG_WITH_NUMBER_VALUE1(skipModeAllowed, "Should be 0");
							}
							else if (backwardIdx >= 0) 
							{
								NAV_WRITE_TAG_WITH_NUMBER_VALUE1(skipModeAllowed, "Should be 1");
								NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("SkipModeFrame", "SkipModeFrame[0]", SkipModeFrame[0], 
									"specifies the frames to use for compound prediction when skip_mode is equal to 1");
								NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("SkipModeFrame", "SkipModeFrame[1]", SkipModeFrame[1], 
									"specifies the frames to use for compound prediction when skip_mode is equal to 1");
							}
							else
							{
								if (secondForwardIdx > -1)
								{
									NAV_WRITE_TAG_WITH_NUMBER_VALUE1(secondForwardIdx, "");
									NAV_WRITE_TAG_WITH_NUMBER_VALUE1(secondForwardHint, "");
								}

								if (secondForwardIdx < 0)
								{
									NAV_WRITE_TAG_WITH_NUMBER_VALUE1(skipModeAllowed, "Should be 0");
								}
								else
								{
									NAV_WRITE_TAG_WITH_NUMBER_VALUE1(skipModeAllowed, "Should be 1");
									NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("SkipModeFrame", "SkipModeFrame[0]", SkipModeFrame[0], 
										"specifies the frames to use for compound prediction when skip_mode is equal to 1");
									NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("SkipModeFrame", "SkipModeFrame[1]", SkipModeFrame[1], 
										"specifies the frames to use for compound prediction when skip_mode is equal to 1");
								}
							}
						}

						if (skipModeAllowed)
						{
							BST_FIELD_PROP_BOOL(skip_mode_present, "specifies that the syntax element skip_mode will be present", 
								"specifies that skip_mode will not be used for this frame");
						}
						else
						{
							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(skip_mode_present, "Should be 0, skip_mode will not be used for this frame");
						}

						DECLARE_FIELDPROP_END()

					}PACKED;

					// 5.9.24. Global motion params syntax
					struct GLOBAL_MOTION_PARAMS : public SYNTAX_BITSTREAM_MAP
					{
						struct REF_GLOBAL_MOTION_PARAMS: public SYNTAX_BITSTREAM_MAP
						{
							// 5.9.28. Decode subexp syntax
							struct DECODE_SUBEXP : public SYNTAX_BITSTREAM_MAP
							{
								uint16_t	numSyms;
								union
								{
									uint16_t	subexp_final_bits = 0;
									uint16_t	subexp_bits;
								};
								uint16_t	decode_subexp;
								CAMBitArray	subexp_more_bits;
								uint16_t	final_i;
								uint16_t	final_mk;
								uint16_t	final_k;

								DECODE_SUBEXP(uint16_t n) : numSyms(n), decode_subexp(0), final_i(0), final_mk(0), final_k(3) {}

								bool IsFinalBits() {
									uint8_t b2 = final_i ? (final_k + final_i - 1) : final_k;
									uint16_t a = (uint16_t)(1 << b2);
									return numSyms <= final_mk + 3 * a;
								}

								int Map(AMBst in_bst)
								{
									SYNTAX_BITSTREAM_MAP::Map(in_bst);
									try
									{
										MAP_BST_BEGIN(0);
										uint16_t i = 0, mk = 0, k = 3;
										while (1)
										{
											uint8_t b2 = i ? (k + i - 1) : k;
											uint16_t a = (uint16_t)(1 << b2);
											if (numSyms <= mk + 3 * a)
											{
												assert(numSyms - mk > 0);
												subexp_final_bits = (uint16_t)AMBst_Get_ns(in_bst, numSyms - mk);
												map_status.number_of_fields++;
												decode_subexp = subexp_final_bits + mk;
												break;
											}
											else
											{
												bsrbarray(in_bst, subexp_more_bits, subexp_more_bits.UpperBound() + 1);
												if (subexp_more_bits[subexp_more_bits.UpperBound()])
												{
													i++;
													mk += a;
												}
												else
												{
													bsrb1(in_bst, subexp_bits, b2);
													decode_subexp = subexp_bits + mk;
													break;
												}
											}
										}

										final_i = i; final_mk = mk; final_k = k;
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

								DECLARE_FIELDPROP_BEGIN() {
									uint16_t i = 0, mk = 0, k = 3;
									NAV_WRITE_TAG_WITH_NUMBER_VALUE("i", 0, "initialize i to 0");
									NAV_WRITE_TAG_WITH_NUMBER_VALUE("mk", 0, "initialize mk to 0");
									NAV_WRITE_TAG_WITH_NUMBER_VALUE("k", 3, "initialize k to 3");
									NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "while(1)", "");
									int loop_count = 0;
									while (1)
									{
										bool bQuitLoop = false;
										NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("Tag01", "Loop(#%d)", "", loop_count);
										uint8_t b2 = i ? (k + i - 1) : k;
										NAV_WRITE_TAG_WITH_NUMBER_VALUE1(b2, "");
										uint16_t a = (uint16_t)(1 << b2);
										NAV_WRITE_TAG_WITH_NUMBER_VALUE1(a, "");
										if (numSyms <= mk + 3 * a)
										{
											assert(numSyms - mk > 0);
											NAV_FIELD_PROP_2NUMBER1_DESC_F(subexp_final_bits, ns_len(numSyms - mk, subexp_final_bits), "ns(numSyms - mk = %d)", numSyms - mk);
											NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag011", "return subexp_final_bits + mk", decode_subexp, "");
											bQuitLoop = true;
										}
										else
										{
											BST_ARRAY_FIELD_PROP_BOOL_BEGIN(subexp_more_bits, "", loop_count, "", "");
											if (subexp_more_bits[subexp_more_bits.UpperBound()])
											{
												i++;
												mk += a;
												NAV_WRITE_TAG_WITH_NUMBER_VALUE1(i, "i++");
												NAV_WRITE_TAG_WITH_NUMBER_VALUE1(mk, "mk += a");
											}
											else
											{
												NAV_FIELD_PROP_2NUMBER1(subexp_bits, b2, "");
												NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag011", "return subexp_bits + mk", decode_subexp, "");
												bQuitLoop = true;
											}
											BST_ARRAY_FIELD_PROP_BOOL_END(subexp_more_bits);
										}
										NAV_WRITE_TAG_END("Tag01");

										if (bQuitLoop)
											break;

										loop_count++;
									}
									NAV_WRITE_TAG_END("Tag0");
								}
								DECLARE_FIELDPROP_END()

							};

							// 5.9.25.Global param syntax
							struct READ_GLOBAL_PARAM : public SYNTAX_BITSTREAM_MAP
							{
								uint8_t		absBits = 0;
								uint8_t		precBits = 0;
								int8_t		precDiff = 0;

								DECODE_SUBEXP*
											decode_subexp;

								uint8_t		m_type;
								uint8_t		m_ref;
								uint8_t		m_idx;
								int32_t		PrevGmParams = 0;

								REF_GLOBAL_MOTION_PARAMS*
											ptr_ref_global_motion_params;

								READ_GLOBAL_PARAM(REF_GLOBAL_MOTION_PARAMS* pRefGlobalMotionParams, uint8_t type, uint8_t ref, uint8_t idx)
									: decode_subexp(nullptr), m_type(type), m_ref(ref), m_idx(idx), ptr_ref_global_motion_params(pRefGlobalMotionParams) {
								}

								virtual ~READ_GLOBAL_PARAM(){
									AMP_SAFEDEL2(decode_subexp);
								}

								int32_t decode_signed_subexp_with_ref(AMBst in_bst, int32_t low, int32_t high, int32_t r)
								{
									int32_t x = decode_unsigned_subexp_with_ref(in_bst, high - low, r - low);
									return x + low;
								}

								int32_t decode_unsigned_subexp_with_ref(AMBst in_bst, int32_t mx, int32_t r)
								{
									assert(decode_subexp == nullptr);
									decode_subexp = new DECODE_SUBEXP(mx);
									decode_subexp->Map(in_bst);
									uint16_t v = decode_subexp->decode_subexp;
									if ((r << 1) <= mx)
									{
										return inverse_recenter(r, v);
									}
									else
									{
										return mx - 1 - inverse_recenter(mx - 1 - r, v);
									}
								}

								int Map(AMBst in_bst)
								{
									auto ptr_uncompressed_header = ptr_ref_global_motion_params->ptr_global_motion_params->ptr_uncompressed_header;
									auto ctx_video_bst = ptr_uncompressed_header->ptr_frame_header_OBU->ptr_OBU->ctx_video_bst;
									SYNTAX_BITSTREAM_MAP::Map(in_bst);
									try
									{
										MAP_BST_BEGIN(0);
										absBits = GM_ABS_ALPHA_BITS;
										precBits = GM_ALPHA_PREC_BITS;

										if (m_idx < 2)
										{
											if (m_type == TRANSLATION)
											{
												absBits = 
													GM_ABS_TRANS_ONLY_BITS - !ptr_uncompressed_header->allow_high_precision_mv;
												precBits = 
													GM_TRANS_ONLY_PREC_BITS - !ptr_uncompressed_header->allow_high_precision_mv;
											}
											else
											{
												absBits = GM_ABS_TRANS_BITS;
												precBits = GM_TRANS_PREC_BITS;
											}
										}

										precDiff = WARPEDMODEL_PREC_BITS - precBits;
										PrevGmParams = ctx_video_bst->PrevGmParams[m_ref - LAST_FRAME][m_idx];

										int32_t round = (m_idx % 3) == 2 ? (1 << WARPEDMODEL_PREC_BITS) : 0;
										int32_t sub = (m_idx % 3) == 2 ? (1 << precBits) : 0;
										int32_t mx = (1 << absBits);
										int32_t r = (PrevGmParams >> precDiff) - sub;
										ptr_ref_global_motion_params->gm_params[m_idx] = (decode_signed_subexp_with_ref(in_bst, -mx, mx + 1, r) << precDiff) + round;

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
									NAV_WRITE_TAG_WITH_NUMBER_VALUE1(absBits, 
										"is used to compute the range of values that can be used for gm_params[ref][idx]. "
										"The values allowed are in the range -(1 &lt;&lt; absBits) to (1 &lt;&lt; absBits)");
									NAV_WRITE_TAG_WITH_NUMBER_VALUE1(precBits, 
										"the number of fractional bits used for representing gm_params[ref][idx]. "
										"All global motion parameters are stored in the model with WARPEDMODEL_PREC_BITS fractional bits, "
										"but the parameters are encoded with less precision");

									NAV_WRITE_TAG_WITH_NUMBER_VALUE1(precDiff, "");
									int32_t round = (m_idx % 3) == 2 ? (1 << WARPEDMODEL_PREC_BITS) : 0;
									NAV_WRITE_TAG_WITH_NUMBER_VALUE1(round, "");
									int32_t sub = (m_idx % 3) == 2 ? (1 << precBits) : 0;
									NAV_WRITE_TAG_WITH_NUMBER_VALUE1(sub, "");
									int32_t mx = (1 << absBits);
									NAV_WRITE_TAG_WITH_NUMBER_VALUE1(mx, "");
									int32_t r = (PrevGmParams >> precDiff) - sub;
									NAV_WRITE_TAG_WITH_NUMBER_VALUE1(r, "");

									NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("decode_signed_subexp_with_ref", "decode_signed_subexp_with_ref(-%d, %d, %d)", "", mx, mx + 1, r);
									NAV_FIELD_PROP_REF(decode_subexp);
									NAV_WRITE_TAG_END("decode_signed_subexp_with_ref");

									NAV_WRITE_TAG_WITH_ALIAS_VALUEFMTSTR_AND_NUMBER_VALUE("gm_params", "gm_params[ref#%d][idx#%d]", "%d(0X%X)", 
										ptr_ref_global_motion_params->gm_params[m_idx],
										"gm_params[ref][idx] = (decode_signed_subexp_with_ref(-mx, mx + 1, r)&lt;&lt;precDiff)+round", m_ref, m_idx);
								DECLARE_FIELDPROP_END()
							}PACKED;

							uint8_t		is_global : 1;
							uint8_t		is_rot_zoom : 1;
							uint8_t		is_translation : 1;
							uint8_t		GmType : 2;
							uint8_t		ref : 3;

							READ_GLOBAL_PARAM*
										ptr_read_global_params[6];
							int32_t		gm_params[6];

							GLOBAL_MOTION_PARAMS*
										ptr_global_motion_params = nullptr;

							REF_GLOBAL_MOTION_PARAMS(GLOBAL_MOTION_PARAMS* pGlobalMotionParams, uint8_t refID)
								:ptr_global_motion_params(pGlobalMotionParams) {
								memset(ptr_read_global_params, 0, sizeof(ptr_read_global_params));
								assert(refID >= 0 && refID <= 7);
								ref = refID;
							}

							virtual ~REF_GLOBAL_MOTION_PARAMS() {
								for (size_t i = 0; i < _countof(ptr_read_global_params); i++){
									AMP_SAFEDEL(ptr_read_global_params[i]);
								}
							}

							int Map(AMBst in_bst)
							{
								SYNTAX_BITSTREAM_MAP::Map(in_bst);
								try
								{
									MAP_BST_BEGIN(0);
									bsrb1(in_bst, is_global, 1);
									if (is_global)
									{
										bsrb1(in_bst, is_rot_zoom, 1);
										if (is_rot_zoom)
											GmType = ROTZOOM;
										else
										{
											bsrb1(in_bst, is_translation, 1);
											GmType = is_translation ? TRANSLATION : AFFINE;
										}
									}
									else
										GmType = IDENTITY;

									if (GmType >= ROTZOOM)
									{
										ptr_read_global_params[2] = new READ_GLOBAL_PARAM(this, GmType, ref, 2);
										ptr_read_global_params[2]->Map(in_bst);

										ptr_read_global_params[3] = new READ_GLOBAL_PARAM(this, GmType, ref, 3);
										ptr_read_global_params[3]->Map(in_bst);

										if (GmType == AFFINE)
										{
											ptr_read_global_params[4] = new READ_GLOBAL_PARAM(this, GmType, ref, 4);
											ptr_read_global_params[4]->Map(in_bst);

											ptr_read_global_params[5] = new READ_GLOBAL_PARAM(this, GmType, ref, 5);
											ptr_read_global_params[5]->Map(in_bst);
										}
										else
										{
											gm_params[4] = -gm_params[3];
											gm_params[5] = gm_params[2];
										}
									}

									if (GmType >= TRANSLATION)
									{
										ptr_read_global_params[0] = new READ_GLOBAL_PARAM(this, GmType, ref, 0);
										ptr_read_global_params[0]->Map(in_bst);

										ptr_read_global_params[1] = new READ_GLOBAL_PARAM(this, GmType, ref, 1);
										ptr_read_global_params[1]->Map(in_bst);
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
							BST_FIELD_PROP_BOOL_BEGIN(is_global, 
								"global motion parameters are present for a particular reference frame", 
								"global motion parameters are NOT present for a particular reference frame");
							if (is_global)
							{
								BST_FIELD_PROP_BOOL_BEGIN(is_rot_zoom, 
									"a particular reference frame uses rotation and zoom global motion", 
									"a particular reference frame does NOT use rotation and zoom global motion");
								if (is_rot_zoom)
								{
									NAV_WRITE_TAG_WITH_NUMBER_VALUE1(GmType, GLOBAL_MOTION_TYPE_NAME(GmType));
								}
								else
								{
									BST_FIELD_PROP_BOOL_BEGIN(is_translation, 
										"a particular reference frame uses translation global motion", 
										"a particular reference frame does NOT use translation global motion");
									NAV_WRITE_TAG_WITH_NUMBER_VALUE1(GmType, GLOBAL_MOTION_TYPE_NAME(GmType));
									BST_FIELD_PROP_BOOL_END("is_translation");
								}
								BST_FIELD_PROP_BOOL_END("is_rot_zoom");
							}
							else
							{
								NAV_WRITE_TAG_WITH_NUMBER_VALUE1(GmType, GLOBAL_MOTION_TYPE_NAME(GmType));
							}
							BST_FIELD_PROP_BOOL_END("is_global");

							if (GmType >= ROTZOOM)
							{
								NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("read_global_param", "read_global_param(type: %s, ref#%d, 2)", "", GLOBAL_MOTION_TYPE_NAME(GmType), ref);
								NAV_FIELD_PROP_REF(ptr_read_global_params[2]);
								NAV_WRITE_TAG_END("read_global_param");

								NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("read_global_param", "read_global_param(type: %s, ref#%d, 3)", "", GLOBAL_MOTION_TYPE_NAME(GmType), ref);
								NAV_FIELD_PROP_REF(ptr_read_global_params[3]);
								NAV_WRITE_TAG_END("read_global_param");

								if (GmType == AFFINE)
								{
									NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("read_global_param", "read_global_param(type: %s, ref#%d, 4)", "", GLOBAL_MOTION_TYPE_NAME(GmType), ref);
									NAV_FIELD_PROP_REF(ptr_read_global_params[4]);
									NAV_WRITE_TAG_END("read_global_param");

									NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("read_global_param", "read_global_param(type: %s, ref#%d, 5)", "", GLOBAL_MOTION_TYPE_NAME(GmType), ref);
									NAV_FIELD_PROP_REF(ptr_read_global_params[5]);
									NAV_WRITE_TAG_END("read_global_param");
								}
								else
								{
									NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("gm_param", "gm_params[4] = -gm_params[3]", gm_params[4], "");
									NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("gm_param", "gm_params[5] = -gm_params[2]", gm_params[5], "");
								}
							}

							if (GmType >= TRANSLATION)
							{
								NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("read_global_param", "read_global_param(type: %s, ref#%d, 0)", "", GLOBAL_MOTION_TYPE_NAME(GmType), ref);
								NAV_FIELD_PROP_REF(ptr_read_global_params[0]);
								NAV_WRITE_TAG_END("read_global_param");

								NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("read_global_param", "read_global_param(type: %s, ref#%d, 1)", "", GLOBAL_MOTION_TYPE_NAME(GmType), ref);
								NAV_FIELD_PROP_REF(ptr_read_global_params[1]);
								NAV_WRITE_TAG_END("read_global_param");
							}
							DECLARE_FIELDPROP_END()

						}PACKED;

						REF_GLOBAL_MOTION_PARAMS* 
										ptr_ref_gm_params[REFS_PER_FRAME];

						UNCOMPRESSED_HEADER*
										ptr_uncompressed_header;

						GLOBAL_MOTION_PARAMS(UNCOMPRESSED_HEADER* pUncompressedHdr): ptr_uncompressed_header(pUncompressedHdr)
						{
							memset(ptr_ref_gm_params, 0, sizeof(ptr_ref_gm_params));
						}

						~GLOBAL_MOTION_PARAMS() {
							for (size_t i = 0; i < REFS_PER_FRAME; i++) {
								if (ptr_ref_gm_params[i] != nullptr)
									delete ptr_ref_gm_params[i];
							}
						}

						int Map(AMBst in_bst)
						{
							SYNTAX_BITSTREAM_MAP::Map(in_bst);
							try
							{
								MAP_BST_BEGIN(0);
								for (int ref = LAST_FRAME; ref <= ALTREF_FRAME; ref++)
								{
									ptr_ref_gm_params[ref - LAST_FRAME] = new REF_GLOBAL_MOTION_PARAMS(this, ref);
									ptr_ref_gm_params[ref - LAST_FRAME]->GmType = IDENTITY;

									for (int i = 0; i < 6; i++)
									{
										ptr_ref_gm_params[ref - LAST_FRAME]->gm_params[i] = ((i % 3) == 2) ? (1 << WARPEDMODEL_PREC_BITS) : 0;
									}
								}

								auto FrameIsIntra = (ptr_uncompressed_header->frame_type == INTRA_ONLY_FRAME ||
													 ptr_uncompressed_header->frame_type == KEY_FRAME);
								if (!FrameIsIntra)
								{
									for (int ref = LAST_FRAME; ref <= ALTREF_FRAME; ref++)
									{
										int iMSMRet = ptr_ref_gm_params[ref - LAST_FRAME]->Map(in_bst);
										if (iMSMRet < 0)
											return iMSMRet;
									}
								}

								// Update it to the current frame in BufferPool
								auto ctx_video_bst = ptr_uncompressed_header->ptr_frame_header_OBU->ptr_OBU->ctx_video_bst;
								auto curFrame = &ctx_video_bst->buffer_pool->frame_bufs[ctx_video_bst->cfbi];

								// 7.21. Reference frame loading process
								// SavedGmParams[i][ref][j] is set equal to gm_params[ref][j] for ref = LAST_FRAME..ALTREF_FRAME, for j = 0..5
								for (uint8_t ref = LAST_FRAME; ref <= ALTREF_FRAME; ref++) {
									memcpy(curFrame->gm_params[ref - LAST_FRAME], ptr_ref_gm_params[ref - LAST_FRAME]->gm_params, 
										sizeof(ptr_ref_gm_params[ref - LAST_FRAME]->gm_params));
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
						auto FrameIsIntra = (ptr_uncompressed_header->frame_type == INTRA_ONLY_FRAME ||
											 ptr_uncompressed_header->frame_type == KEY_FRAME);
						NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "for(ref=LAST_FRAME;ref&lt;=ALTREF_FRAME;ref++)", "");
						for (int ref = LAST_FRAME; ref <= ALTREF_FRAME; ref++)
						{
							if (ptr_ref_gm_params[ref - LAST_FRAME] == nullptr)
								continue;

							NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("Tag00", "[ref#%d/%s]", "", ref, AV1_REF_FRAME_NAMEA(ref));
							if (!FrameIsIntra)
							{
								NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("Tag000", "ref_frame_gm_param(ref#%d)", "", ref);
								NAV_FIELD_PROP_REF(ptr_ref_gm_params[ref - LAST_FRAME]);
								NAV_WRITE_TAG_END("Tag000");
							}

							NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag001", "GmType[ref#%d]", 
								ptr_ref_gm_params[ref - LAST_FRAME]->GmType, GLOBAL_MOTION_TYPE_NAME(ptr_ref_gm_params[ref - LAST_FRAME]->GmType), ref);
							//NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("gm_param", "gm_params", ptr_ref_gm_params[ref - LAST_FRAME]->gm_params[idx], "", ref, idx);
							NAV_WRITE_TAG_WITH_VALUEFMTSTR("gm_params", "%d,%d,%d,%d,%d,%d", "", 
								ptr_ref_gm_params[ref - LAST_FRAME]->gm_params[0], ptr_ref_gm_params[ref - LAST_FRAME]->gm_params[1], 
								ptr_ref_gm_params[ref - LAST_FRAME]->gm_params[2],
								ptr_ref_gm_params[ref - LAST_FRAME]->gm_params[3], ptr_ref_gm_params[ref - LAST_FRAME]->gm_params[4], 
								ptr_ref_gm_params[ref - LAST_FRAME]->gm_params[5]);
							NAV_WRITE_TAG_END("Tag00");
						}
						NAV_WRITE_TAG_END("Tag0");
						DECLARE_FIELDPROP_END()
					}PACKED;

					// 5.9.30. Film grain params syntax
					struct FILM_GRAIN_PARAMS : public SYNTAX_BITSTREAM_MAP
					{
						FILM_GRAIN_PARAMS_DATA
									film_grain_params_data;

						uint16_t	tempGrainSeed;

						UNCOMPRESSED_HEADER*
									ptr_uncompressed_header = nullptr;

						FILM_GRAIN_PARAMS(UNCOMPRESSED_HEADER* pUncompressedHeader) 
							: tempGrainSeed(0), ptr_uncompressed_header(pUncompressedHeader){
							memset(&film_grain_params_data, 0, sizeof(film_grain_params_data));
						}

						int Map(AMBst in_bst)
						{
							auto ctx_video_bst = ptr_uncompressed_header->ptr_frame_header_OBU->ptr_OBU->ctx_video_bst;
							SYNTAX_BITSTREAM_MAP::Map(in_bst);
							try
							{
								MAP_BST_BEGIN(0);

								if (!ptr_uncompressed_header->ptr_sequence_header_obu->film_gain_params_present || (
									!ptr_uncompressed_header->show_frame && !ptr_uncompressed_header->showable_frame))
								{
									ctx_video_bst->reset_grain_params();
									ctx_video_bst->buffer_pool->frame_bufs[ctx_video_bst->cfbi].film_grain_params = film_grain_params_data;
									MAP_BST_END();
									return RET_CODE_SUCCESS;
								}

								bsrb1(in_bst, film_grain_params_data.apply_grain, 1);
								if (!film_grain_params_data.apply_grain)
								{
									ctx_video_bst->reset_grain_params();
									ctx_video_bst->buffer_pool->frame_bufs[ctx_video_bst->cfbi].film_grain_params = film_grain_params_data;
									MAP_BST_END();
									return RET_CODE_SUCCESS;
								}

								bsrb1(in_bst, film_grain_params_data.grain_seed, 16);
								if (ptr_uncompressed_header->frame_type == INTER_FRAME)
								{
									bsrb1(in_bst, film_grain_params_data.update_grain, 1);
								}
								else
									film_grain_params_data.update_grain = 1;

								if (!film_grain_params_data.update_grain)
								{
									bsrb1(in_bst, film_grain_params_data.film_grain_params_ref_idx, 3);
									auto& cur_frame = ctx_video_bst->buffer_pool->frame_bufs[ctx_video_bst->cfbi];
									tempGrainSeed = cur_frame.film_grain_params.grain_seed;
									ctx_video_bst->load_grain_params(film_grain_params_data.film_grain_params_ref_idx);
									cur_frame.film_grain_params.grain_seed = tempGrainSeed;
									ctx_video_bst->buffer_pool->frame_bufs[ctx_video_bst->cfbi].film_grain_params = film_grain_params_data;
									MAP_BST_END();
									return RET_CODE_SUCCESS;
								}

								bsrb1(in_bst, film_grain_params_data.num_y_points, 4);
								for (int i = 0; i < film_grain_params_data.num_y_points; i++)
								{
									bsrb1(in_bst, film_grain_params_data.point_y_value[i], 8);
									bsrb1(in_bst, film_grain_params_data.point_y_scaling[i], 8);
								}

								if (ptr_uncompressed_header->ptr_sequence_header_obu->color_config->mono_chrome)
								{
									film_grain_params_data.chroma_scaling_from_luma = 0;
								}
								else
								{
									bsrb1(in_bst, film_grain_params_data.chroma_scaling_from_luma, 1);
								}

								if (ptr_uncompressed_header->ptr_sequence_header_obu->color_config->mono_chrome ||
									film_grain_params_data.chroma_scaling_from_luma || (ptr_uncompressed_header->ptr_sequence_header_obu->color_config->subsampling_x == 1 &&
										ptr_uncompressed_header->ptr_sequence_header_obu->color_config->subsampling_y == 1 && film_grain_params_data.num_y_points == 0))
								{
									film_grain_params_data.num_cb_points = 0;
									film_grain_params_data.num_cr_points = 0;
								}
								else
								{
									bsrb1(in_bst, film_grain_params_data.num_cb_points, 4);
									for (int i = 0; i < film_grain_params_data.num_cb_points; i++)
									{
										bsrb1(in_bst, film_grain_params_data.point_cb_value[i], 8);
										bsrb1(in_bst, film_grain_params_data.point_cb_scaling[i], 8);
									}
									bsrb1(in_bst, film_grain_params_data.num_cr_points, 4);
									for (int i = 0; i < film_grain_params_data.num_cr_points; i++)
									{
										bsrb1(in_bst, film_grain_params_data.point_cr_value[i], 8);
										bsrb1(in_bst, film_grain_params_data.point_cr_scaling[i], 8);
									}
								}

								bsrb1(in_bst, film_grain_params_data.grain_scaling_minus_8, 2);
								bsrb1(in_bst, film_grain_params_data.ar_coeff_lag, 2);

								film_grain_params_data.numPosLuma = 2 * film_grain_params_data.ar_coeff_lag * (film_grain_params_data.ar_coeff_lag + 1);
								if (film_grain_params_data.num_y_points)
								{
									film_grain_params_data.numPosChroma = film_grain_params_data.numPosLuma + 1;
									for (int i = 0; i < film_grain_params_data.numPosLuma; i++)
									{
										bsrb1(in_bst, film_grain_params_data.ar_coeffs_y_plus_128[i], 8);
									}
								}
								else
								{
									film_grain_params_data.numPosChroma = film_grain_params_data.numPosLuma;
								}

								if (film_grain_params_data.chroma_scaling_from_luma || film_grain_params_data.num_cb_points) {
									for (int i = 0; i < film_grain_params_data.numPosChroma; i++) {
										bsrb1(in_bst, film_grain_params_data.ar_coeffs_cb_plus_128[i], 8);
									}
								}
								if (film_grain_params_data.chroma_scaling_from_luma || film_grain_params_data.num_cr_points) {
									for (int i = 0; i < film_grain_params_data.numPosChroma; i++) {
										bsrb1(in_bst, film_grain_params_data.ar_coeffs_cr_plus_128[i], 8);
									}
								}
								bsrb1(in_bst, film_grain_params_data.ar_coeff_shift_minus_6, 2);
								bsrb1(in_bst, film_grain_params_data.grain_scale_shift, 2);

								if (film_grain_params_data.num_cb_points)
								{
									bsrb1(in_bst, film_grain_params_data.cb_mult, 8);
									bsrb1(in_bst, film_grain_params_data.cb_luma_mult, 8);
									bsrb1(in_bst, film_grain_params_data.cb_offset, 9);
								}
								if (film_grain_params_data.num_cr_points)
								{
									bsrb1(in_bst, film_grain_params_data.cr_mult, 8);
									bsrb1(in_bst, film_grain_params_data.cr_luma_mult, 8);
									bsrb1(in_bst, film_grain_params_data.cr_offset, 9);
								}

								bsrb1(in_bst, film_grain_params_data.overlap_flag, 1);
								bsrb1(in_bst, film_grain_params_data.clip_to_restricted_range, 1);

								// Update it to the related frame
								ctx_video_bst->buffer_pool->frame_bufs[ctx_video_bst->cfbi].film_grain_params = film_grain_params_data;

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
						if (!ptr_uncompressed_header->ptr_sequence_header_obu->film_gain_params_present || (
							!ptr_uncompressed_header->show_frame && !ptr_uncompressed_header->showable_frame))
						{
							NAV_WRITE_TAG_WITH_ALIAS("reset_grain_params", "reset_grain_params()", 
								"a function call that indicates that all the syntax elements read in film_grain_params should be set equal to 0");
						}
						else
						{
							BST_FIELD_PROP_BOOL_BEGIN_("apply_grain", film_grain_params_data.apply_grain, 
								"specifies that film grain should be added to this frame", "specifies that film grain should NOT be added to this frame");
							if (!film_grain_params_data.apply_grain)
							{
								NAV_WRITE_TAG_WITH_ALIAS("reset_grain_params", "reset_grain_params()", 
									"a function call that indicates that all the syntax elements read in film_grain_params should be set equal to 0");
							}
							else
							{
								BST_FIELD_PROP_2NUMBER("grain_seed", 16, film_grain_params_data.grain_seed, 
									"specifies the starting value for the pseudo-random numbers used during film grain synthesis");

								if (ptr_uncompressed_header->frame_type == INTER_FRAME)
								{
									BST_FIELD_PROP_BOOL(film_grain_params_data.update_grain, "means that a new set of parameters should be sent"
																						   , "means that the previous set of parameters should be used");
								}
								else
								{
									NAV_WRITE_TAG_WITH_1NUMBER_VALUE("update_grain", film_grain_params_data.update_grain, 
										"Should be 1, means that a new set of parameters should be sent");
								}

								if (!film_grain_params_data.update_grain)
								{
									BST_FIELD_PROP_2NUMBER("film_grain_params_ref_idx", 3, film_grain_params_data.film_grain_params_ref_idx, 
										"indicates which reference frame contains the film grain parameters to be used for this frame");
									NAV_WRITE_TAG_WITH_NUMBER_VALUE("tempGrainSeed", tempGrainSeed, "tempGrainSeed = grain_seed");
									NAV_WRITE_TAG_WITH_ALIAS_F("load_grain_params", "load_grain_params(film_grain_params_ref_idx#%d)", 
										"a function call that indicates that all the syntax elements read in film_grain_params "
										"should be set equal to the values stored in an area of memory indexed by idx", 
										film_grain_params_data.film_grain_params_ref_idx);
								}
								else
								{
									NAV_WRITE_TAG_BEGIN2("update_grain");
									BST_FIELD_PROP_2NUMBER("num_y_points", 4, film_grain_params_data.num_y_points, 
										"specifies the number of points for the piece-wise linear scaling function of the luma component");
									NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "for(i=0;i&lt;num_y_points;i++)", "");
									for (int i = 0; i < film_grain_params_data.num_y_points; i++)
									{
										NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("Tag00", "[point_y#%d]", "", i);
										BST_FIELD_PROP_2NUMBER("point_y_value", 8, film_grain_params_data.point_y_value[i], 
											"represents the x (luma value) coordinate for the i-th point of the piecewise linear scaling function for luma component");
										BST_FIELD_PROP_2NUMBER("point_y_scaling", 8, film_grain_params_data.point_y_scaling[i], 
											"represents the scaling (output) value for the i-th point of the piecewise linear scaling function for luma component");
										NAV_WRITE_TAG_END("Tag00");
									}
									NAV_WRITE_TAG_END("Tag0");

									if (ptr_uncompressed_header->ptr_sequence_header_obu->color_config->mono_chrome)
									{
										NAV_WRITE_TAG_WITH_NUMBER_VALUE("chroma_scaling_from_luma", film_grain_params_data.chroma_scaling_from_luma, 
											"specifies that the chroma scaling is inferred from the luma scaling");
									}
									else
									{
										BST_FIELD_PROP_BOOL1(film_grain_params_data, chroma_scaling_from_luma, "the chroma scaling is inferred from the luma scaling", 
											"the chroma scaling is NOT inferred from the luma scaling");
									}

									if (ptr_uncompressed_header->ptr_sequence_header_obu->color_config->mono_chrome ||
										film_grain_params_data.chroma_scaling_from_luma || (ptr_uncompressed_header->ptr_sequence_header_obu->color_config->subsampling_x == 1 &&
											ptr_uncompressed_header->ptr_sequence_header_obu->color_config->subsampling_y == 1 && film_grain_params_data.num_y_points == 0))
									{
										NAV_WRITE_TAG_WITH_NUMBER_VALUE("num_cb_points", film_grain_params_data.num_cb_points, "Should be 0");
										NAV_WRITE_TAG_WITH_NUMBER_VALUE("num_cr_points", film_grain_params_data.num_cr_points, "Should be 0");
									}
									else
									{
										BST_FIELD_PROP_2NUMBER("num_cb_points", 4, film_grain_params_data.num_cb_points, 
											"specifies the number of points for the piece-wise linear scaling function of the cb component");
										NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag1", "for(i=0;i&lt;num_cb_points;i++)", "");
										for (int i = 0; i < film_grain_params_data.num_cb_points; i++)
										{
											NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("Tag10", "[point_cb#%d]", "", i);
											BST_FIELD_PROP_2NUMBER("point_cb_value", 8, film_grain_params_data.point_cb_value[i], 
												"represents the x coordinate for the i-th point of the piece-wise linear scaling function for cb component");
											BST_FIELD_PROP_2NUMBER("point_cb_scaling", 8, film_grain_params_data.point_cb_scaling[i], 
												"represents the scaling (output) value for the i-th point of the piecewise linear scaling function for cb component");
											NAV_WRITE_TAG_END("Tag10");
										}
										NAV_WRITE_TAG_END("Tag1");

										BST_FIELD_PROP_2NUMBER("num_cr_points", 4, film_grain_params_data.num_cr_points, 
											"specifies represents the number of points for the piece-wise linear scaling function of the cr component");
										NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag2", "for(i=0;i&lt;num_cr_points;i++)", "");
										for (int i = 0; i < film_grain_params_data.num_cr_points; i++)
										{
											NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("Tag20", "[point_cr#%d]", "", i);
											BST_FIELD_PROP_2NUMBER("point_cr_value", 8, film_grain_params_data.point_cr_value[i], 
												"represents the x coordinate for the i-th point of the piece-wise linear scaling function for cr component");
											BST_FIELD_PROP_2NUMBER("point_cr_scaling", 8, film_grain_params_data.point_cr_scaling[i], 
												"represents the scaling (output) value for the i-th point of the piecewise linear scaling function for cr component");
											NAV_WRITE_TAG_END("Tag20");
										}
										NAV_WRITE_TAG_END("Tag2");
									}

									BST_FIELD_PROP_2NUMBER("grain_scaling_minus_8", 2, film_grain_params_data.grain_scaling_minus_8, 
										"represents the shift-8 applied to the values of the chroma component");
									BST_FIELD_PROP_2NUMBER("ar_coeff_lag", 2, film_grain_params_data.ar_coeff_lag, 
										"specifies the number of auto-regressive coefficients for luma and chroma");

									NAV_WRITE_TAG_WITH_NUMBER_VALUE("numPosLuma", film_grain_params_data.numPosLuma, "numPosLuma = 2*ar_coeff_lag*(ar_coeff_lag+1)");
									if (film_grain_params_data.num_y_points)
									{
										NAV_WRITE_TAG_WITH_NUMBER_VALUE("numPosChroma", film_grain_params_data.numPosChroma, "numPosChroma = numPosLuma + 1");
										NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag3", "for(i=0;i&lt;numPosLuma;i++)", "");
										for (int i = 0; i < film_grain_params_data.numPosLuma; i++)
										{
											BST_ARRAY_FIELD_PROP_NUMBER("ar_coeffs_y_plus_128", i, 8, film_grain_params_data.ar_coeffs_y_plus_128[i], 
												"auto-regressive coefficients used for the Y plane");
										}
										NAV_WRITE_TAG_END("Tag3");
									}
									else
									{
										NAV_WRITE_TAG_WITH_NUMBER_VALUE("numPosChroma", film_grain_params_data.numPosChroma, "numPosChroma = numPosLuma");
									}

									if (film_grain_params_data.chroma_scaling_from_luma || film_grain_params_data.num_cb_points) {
										NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag4", "for(i=0;i&lt;numPosChroma;i++)", "");
										for (int i = 0; i < film_grain_params_data.numPosChroma; i++)
										{
											BST_ARRAY_FIELD_PROP_NUMBER("ar_coeffs_cb_plus_128", i, 8, film_grain_params_data.ar_coeffs_cb_plus_128[i], 
												"auto-regressive coefficients used for the U plane");
										}
										NAV_WRITE_TAG_END("Tag4");
									}
									if (film_grain_params_data.chroma_scaling_from_luma || film_grain_params_data.num_cr_points) {
										NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag5", "for(i=0;i&lt;numPosChroma;i++)", "");
										for (int i = 0; i < film_grain_params_data.numPosChroma; i++)
										{
											BST_ARRAY_FIELD_PROP_NUMBER("ar_coeffs_cr_plus_128", i, 8, film_grain_params_data.ar_coeffs_cr_plus_128[i], 
												"auto-regressive coefficients used for the V plane");
										}
										NAV_WRITE_TAG_END("Tag5");
									}

									BST_FIELD_PROP_2NUMBER("ar_coeff_shift_minus_6", 2, film_grain_params_data.ar_coeff_shift_minus_6, 
										"specifies the range of the auto-regressive coefficients");
									BST_FIELD_PROP_2NUMBER("grain_scale_shift", 2, film_grain_params_data.grain_scale_shift, 
										"specifies how much the Gaussian random numbers should be scaled down during the grain synthesis process");

									if (film_grain_params_data.num_cb_points)
									{
										BST_FIELD_PROP_2NUMBER("cb_mult", 8, film_grain_params_data.cb_mult, 
											"represents a multiplier for the cb component used in derivation of the input index to the cb component scaling function");
										BST_FIELD_PROP_2NUMBER("cb_luma_mult", 8, film_grain_params_data.cb_luma_mult, 
											"represents a multiplier for the average luma component used in derivation of the input index to the cb component scaling function");
										BST_FIELD_PROP_2NUMBER("cb_offset", 9, film_grain_params_data.cb_offset, 
											"represents an offset used in derivation of the input index to the cb component scaling function");
									}

									if (film_grain_params_data.num_cr_points)
									{
										BST_FIELD_PROP_2NUMBER("cr_mult", 8, film_grain_params_data.cr_mult, 
											"represents a multiplier for the cr component used in derivation of the input index to the cr component scaling function");
										BST_FIELD_PROP_2NUMBER("cr_luma_mult", 8, film_grain_params_data.cr_luma_mult, 
											"represents a multiplier for the average luma component used in derivation of the input index to the cr component scaling function");
										BST_FIELD_PROP_2NUMBER("cr_offset", 9, film_grain_params_data.cr_offset, 
											"represents an offset used in derivation of the input index to the cr component scaling function");
									}

									BST_FIELD_PROP_BOOL1(film_grain_params_data, overlap_flag, "indicates that the overlap between film grain blocks shall be applied", 
										"indicates that the overlap between film grain blocks shall not be applied");
									BST_FIELD_PROP_BOOL1(film_grain_params_data, clip_to_restricted_range,
										"indicates that clipping to the restricted (studio) range shall be applied to the sample values "
										"after adding the film grain(see the semantics for color_range for an explanation of studio swing)", 
										"indicates that clipping to the full range shall be applied to the sample values after adding the film grain");

									NAV_WRITE_TAG_END("update_grain");
								}
							}
							BST_FIELD_PROP_BOOL_END("apply_grain");
						}
						DECLARE_FIELDPROP_END()

					}PACKED;

					uint8_t			show_existing_frame : 1;
					uint8_t			frame_to_show_map_idx : 3;
					uint8_t			byte_align_0 : 4;

					TEMPORAL_POINT_INFO
									temporal_point_info;

					int32_t			display_frame_id;

					uint8_t			frame_type : 2;
					uint8_t			show_frame : 1;
					uint8_t			showable_frame : 1;
					uint8_t			error_resilient_mode : 1;
					uint8_t			disable_cdf_update : 1;
					uint8_t			allow_screen_content_tools : 2;

					uint8_t			force_integer_mv : 2;
					uint8_t			frame_size_override_flag : 1;
					uint8_t			primary_ref_frame : 3;
					uint8_t			buffer_removal_time_present_flag : 1;
					uint8_t			allow_intrabc : 1;

					uint8_t			order_hint;

					int32_t			current_frame_id;

					std::vector<uint32_t>
									buffer_removal_time;

					uint8_t			refresh_frame_flags;
					uint8_t			ref_order_hint[NUM_REF_FRAMES];

					// The below 2 fields should NOT affect any parse or process logic, it is only used to show syntax view
					int8_t			RefValid_of_mark_ref_frames[NUM_REF_FRAMES] = { -1, -1, -1, -1, -1, -1, -1, -1};

					FRAME_SIZE*		ptr_frame_size = nullptr;
					RENDER_SIZE*	ptr_render_size = nullptr;

					uint8_t			frame_refs_short_signaling : 1;
					uint8_t			last_frame_idx : 3;
					uint8_t			gold_frame_idx : 3;
					uint8_t			byte_align_1 : 1;

					uint8_t			ref_frame_idx[REFS_PER_FRAME];
					uint8_t			RefFrameSignBias = 0;
					uint32_t		delta_frame_id_minus_1[REFS_PER_FRAME];
					uint32_t		expectedFrameId[REFS_PER_FRAME];

					FRAME_SIZE_WITH_REFS*
									ptr_frame_size_with_refs = nullptr;

					uint8_t			allow_high_precision_mv : 1;
					uint8_t			is_motion_mode_switchable : 1;
					uint8_t			use_ref_frame_mvs : 1;
					uint8_t			disable_frame_end_update_cdf : 1;
					uint8_t			allow_warped_motion : 1;
					uint8_t			reduced_tx_set : 1;
					uint8_t			CodedLossless : 1;
					uint8_t			AllLossless : 1;

					BOOL			LosslessArray[MAX_SEGMENTS];
					uint8_t			SegQMLevel[3][MAX_SEGMENTS];

					TILE_INFO*		ptr_tile_info = nullptr;
					QUANTIZATION_PARAMS*
									ptr_quantization_params = nullptr;
					SEGMENTATION_PARAMS*
									ptr_segmentation_params = nullptr;

					READ_INTERPOLATION_FILTER*
									ptr_read_interpolation_filter = nullptr;
					DELTA_Q_PARAMS*	ptr_delta_q_params = nullptr;
					DELTA_LF_PARAMS*
									ptr_delta_lf_params = nullptr;

					LOOP_FILTER_PARAMS*
									ptr_loop_filter_params = nullptr;
					CDEF_PARAMS*	ptr_cdef_params = nullptr;
					LR_PARAMS*		ptr_lr_params = nullptr;
					READ_TX_MODE*	ptr_read_tx_mode = nullptr;
					FRAME_REFERENCE_MODE*
									ptr_frame_reference_mode = nullptr;
					SKIP_MODE_PARAMS*
									ptr_skip_mode_params = nullptr;
					GLOBAL_MOTION_PARAMS*
									ptr_global_motion_params = nullptr;
					FILM_GRAIN_PARAMS*
									ptr_film_grain_params = nullptr;

					FRAME_HEADER_OBU*
									ptr_frame_header_OBU = nullptr;

					AV1_OBU			sp_sequence_header;
					SEQUENCE_HEADER_OBU*
									ptr_sequence_header_obu = nullptr;
					RefCntBuffer*	ptr_frame = nullptr;

					UNCOMPRESSED_HEADER(FRAME_HEADER_OBU* pFrameHdrOBU): ptr_frame_header_OBU(pFrameHdrOBU){}

					~UNCOMPRESSED_HEADER() {
						UNMAP_STRUCT_POINTER5(ptr_frame_size);
						UNMAP_STRUCT_POINTER5(ptr_render_size);
						UNMAP_STRUCT_POINTER5(ptr_frame_size_with_refs);
						UNMAP_STRUCT_POINTER5(ptr_read_interpolation_filter);
						UNMAP_STRUCT_POINTER5(ptr_tile_info);
						UNMAP_STRUCT_POINTER5(ptr_quantization_params);
						UNMAP_STRUCT_POINTER5(ptr_segmentation_params);
						UNMAP_STRUCT_POINTER5(ptr_delta_q_params);
						UNMAP_STRUCT_POINTER5(ptr_delta_lf_params);
						UNMAP_STRUCT_POINTER5(ptr_loop_filter_params);
						UNMAP_STRUCT_POINTER5(ptr_cdef_params);
						UNMAP_STRUCT_POINTER5(ptr_lr_params);
						UNMAP_STRUCT_POINTER5(ptr_read_tx_mode);
						UNMAP_STRUCT_POINTER5(ptr_frame_reference_mode);
						UNMAP_STRUCT_POINTER5(ptr_skip_mode_params);
						UNMAP_STRUCT_POINTER5(ptr_global_motion_params);
						UNMAP_STRUCT_POINTER5(ptr_film_grain_params);
					}

					int Map(AMBst in_bst)
					{
						BOOL FrameIsIntra = TRUE;
						int32_t PrevFrameID = 0;

						SYNTAX_BITSTREAM_MAP::Map(in_bst);
						try
						{
							if (ptr_frame_header_OBU == nullptr ||
								ptr_frame_header_OBU->ptr_OBU == nullptr ||
								ptr_frame_header_OBU->ptr_OBU->ctx_video_bst == nullptr ||
								ptr_frame_header_OBU->ptr_OBU->ctx_video_bst->sp_sequence_header == nullptr)
								return RET_CODE_NOTHING_TODO;

							if((ptr_sequence_header_obu = ptr_frame_header_OBU->ptr_OBU->ctx_video_bst->sp_sequence_header->ptr_sequence_header_obu) == nullptr)
								return RET_CODE_NOTHING_TODO;

							auto ctx_video_bst = ptr_frame_header_OBU->ptr_OBU->ctx_video_bst;
							auto frame_bufs = ctx_video_bst->buffer_pool->frame_bufs;

							MAP_BST_BEGIN(0);

							uint8_t idLen = 0;
							if (ptr_sequence_header_obu->frame_id_numbers_present_flag)
								idLen = ptr_sequence_header_obu->additional_frame_id_length_minus_1 + ptr_sequence_header_obu->delta_frame_id_length_minus_2 + 3;

							uint16_t allFrames = (uint16_t)((1 << NUM_REF_FRAMES) - 1);
							if (ptr_sequence_header_obu->reduced_still_picture_header)
							{
								show_existing_frame = 0;
								ctx_video_bst->show_existing_frame = false;
								frame_type = KEY_FRAME;
								FrameIsIntra = TRUE;
								show_frame = 1;
								ctx_video_bst->show_frame = true;
								showable_frame = 0;
							}
							else
							{
								bsrb1(in_bst, show_existing_frame, 1);
								ctx_video_bst->show_existing_frame = show_existing_frame;
								if (show_existing_frame == 1)
								{
									bsrb1(in_bst, frame_to_show_map_idx, 3);
									const int frame_to_show = ctx_video_bst->VBI[frame_to_show_map_idx];
									if (ptr_sequence_header_obu->decoder_model_info_present_flag && !ptr_sequence_header_obu->ptr_timing_info->equal_picture_interval) {
										int frame_presentation_time_length = (int)(ptr_sequence_header_obu->ptr_decoder_model_info->frame_presentation_time_length_minus_1 + 1);
										bsrb1(in_bst, temporal_point_info.frame_presentation_time, frame_presentation_time_length);
									}

									ctx_video_bst->cfbi = frame_to_show;
									ptr_frame = &ctx_video_bst->buffer_pool->frame_bufs[ctx_video_bst->cfbi];

									refresh_frame_flags = 0;
									if (ptr_sequence_header_obu->frame_id_numbers_present_flag) {
										bsrb1(in_bst, display_frame_id, idLen);
										int8_t buf_idx = ctx_video_bst->VBI[frame_to_show_map_idx];

										if (buf_idx >= 0 && (display_frame_id != ctx_video_bst->buffer_pool->frame_bufs[buf_idx].current_frame_id ||
															!ctx_video_bst->buffer_pool->frame_bufs[buf_idx].valid_for_referencing))
										{
											printf("[AV1] Reference buffer frame ID mismatch.\n");
										}
									}

									if (frame_to_show < 0 || frame_bufs[frame_to_show].ref_count < 1)
									{
										printf("[AV1] No valid reference frame for frame_to_show_map_idx: %d/frame_to_show: %d.\n", frame_to_show_map_idx, frame_to_show);
										return ctx_video_bst->SingleOBUParse?RET_CODE_SUCCESS:RET_CODE_BUFFER_NOT_COMPATIBLE;
									}

									//ctx_video_bst->buffer_pool->ref_cnt_fb(&ctx_video_bst->cfbi, frame_to_show);

									// double reference for frame_to_show
									frame_bufs[frame_to_show].ref_count++;

									frame_bufs[frame_to_show].showable_frame = 0;

									if (ctx_video_bst->SingleOBUParse)
										frame_type = ctx_video_bst->frame_types[ctx_video_bst->tu_fu_idx];
									else
										frame_type = frame_bufs[frame_to_show].frame_type;

									if (frame_type == KEY_FRAME)
									{
										refresh_frame_flags = (uint8_t)allFrames;

										for (int i = 0; i < REFS_PER_FRAME; ++i) {
											ctx_video_bst->ref_frame_idx[i] = -1;
										}

										if (ctx_video_bst->SingleOBUParse == false)
										{
											if (ptr_sequence_header_obu->frame_id_numbers_present_flag)
											{
												int8_t buf_idx = ctx_video_bst->VBI[frame_to_show_map_idx];
												if (buf_idx >= 0)
												{
													auto reset_display_frame_id = ctx_video_bst->buffer_pool->frame_bufs[buf_idx].current_frame_id;
													for (int i = 0; i < NUM_REF_FRAMES; i++)
													{
														if ((refresh_frame_flags >> i) & 1) {
															buf_idx = ctx_video_bst->VBI[i];
															if (buf_idx >= 0)
															{
																ctx_video_bst->buffer_pool->frame_bufs[buf_idx].current_frame_id = reset_display_frame_id;
																ctx_video_bst->buffer_pool->frame_bufs[buf_idx].valid_for_referencing = true;
															}
														}
													}
												}
											}
										}
									}

									ctx_video_bst->refresh_frame_flags = refresh_frame_flags;

									if (ptr_sequence_header_obu->film_gain_params_present)
									{
										ctx_video_bst->load_grain_params(frame_to_show_map_idx);
									}
									
									MAP_BST_END();
									goto done;
								}

								// Hit a new frame, allocate the frame buffer for it
								if ((ctx_video_bst->cfbi = ctx_video_bst->buffer_pool->get_free_fb()) == -1)
								{
									printf("[AV1] Failed to get the free frame buffer from BufferPool.\n");
									return RET_CODE_ERROR;
								}

								ptr_frame = &ctx_video_bst->buffer_pool->frame_bufs[ctx_video_bst->cfbi];

								bsrb1(in_bst, frame_type, 2);
								FrameIsIntra = (frame_type == INTRA_ONLY_FRAME || frame_type == KEY_FRAME) ? TRUE : FALSE;

								bsrb1(in_bst, show_frame, 1);
								ctx_video_bst->show_frame = show_frame;
								if (show_frame && ptr_sequence_header_obu->decoder_model_info_present_flag && !ptr_sequence_header_obu->ptr_timing_info->equal_picture_interval)
								{
									bsrb1(in_bst, temporal_point_info.frame_presentation_time, 
										(int)(ptr_sequence_header_obu->ptr_decoder_model_info->frame_presentation_time_length_minus_1 + 1));
								}

								if (show_frame) {
									showable_frame = frame_type != KEY_FRAME ? TRUE : FALSE;
								}
								else
								{
									bsrb1(in_bst, showable_frame, 1);
								}

								if (frame_type == SWITCH_FRAME || (frame_type == KEY_FRAME && show_frame))
								{
									error_resilient_mode = 1;
								}
								else
								{
									bsrb1(in_bst, error_resilient_mode, 1);
								}
							}

							if (frame_type == KEY_FRAME && show_frame)
							{
								for (int i = 0; i < NUM_REF_FRAMES; i++)
								{
									auto buf_idx = ctx_video_bst->VBI[i];
									if (buf_idx >= 0)
										ctx_video_bst->buffer_pool->frame_bufs[buf_idx].valid_for_referencing = false;
									if (ctx_video_bst->VBI[i] >= 0)
										frame_bufs[ctx_video_bst->VBI[i]].current_order_hint = 0;	// RefOrderHint[i] = 0
								}

								for (int i = 0; i < REFS_PER_FRAME; i++)
								{
									frame_bufs[ctx_video_bst->cfbi].ref_order_hints[i] = 0;	// OrderHints[LAST_FRAME + i] = 0
								}
							}

							bsrb1(in_bst, disable_cdf_update, 1);
							if (ptr_sequence_header_obu->seq_force_screen_content_tools == SELECT_SCREEN_CONTENT_TOOLS)
							{
								bsrb1(in_bst, allow_screen_content_tools, 1);
							}
							else
								allow_screen_content_tools = ptr_sequence_header_obu->seq_force_screen_content_tools;

							if (allow_screen_content_tools)
							{
								if (ptr_sequence_header_obu->seq_force_integer_mv == SELECT_INTEGER_MV) {
									bsrb1(in_bst, force_integer_mv, 1);
								}
								else
									force_integer_mv = ptr_sequence_header_obu->seq_force_integer_mv;
							}
							else
							{
								force_integer_mv = 0;
							}

							if (FrameIsIntra)
							{
								force_integer_mv = 1;
							}

							if (ptr_sequence_header_obu->frame_id_numbers_present_flag)
							{
								PrevFrameID = current_frame_id;
								bsrb1(in_bst, current_frame_id, idLen);
								ctx_video_bst->current_frame_id = current_frame_id;
								mark_ref_frames(ptr_sequence_header_obu, idLen);
							}
							else
								current_frame_id = 0;

							if (frame_type == SWITCH_FRAME)
								frame_size_override_flag = 1;
							else if(ptr_sequence_header_obu->reduced_still_picture_header)
								frame_size_override_flag = 0;
							else
							{
								bsrb1(in_bst, frame_size_override_flag, 1);
							}

							bsrb1(in_bst, order_hint, ptr_sequence_header_obu->OrderHintBits);
							
							if (FrameIsIntra || error_resilient_mode)
								primary_ref_frame = PRIMARY_REF_NONE;
							else
							{
								bsrb1(in_bst, primary_ref_frame, 3);
							}

							if (ptr_sequence_header_obu->decoder_model_info_present_flag)
							{
								bsrb1(in_bst, buffer_removal_time_present_flag, 1);
								if (buffer_removal_time_present_flag)
								{
									buffer_removal_time.resize((size_t)ptr_sequence_header_obu->operating_points_cnt_minus_1 + 1);
									for (int opNum = 0; opNum <= ptr_sequence_header_obu->operating_points_cnt_minus_1; opNum++)
									{
										if (ptr_sequence_header_obu->operating_points[opNum].decoder_model_present_for_this_op)
										{
											uint32_t opPtIdc = ptr_sequence_header_obu->operating_points[opNum].operating_point_idc;
											uint8_t temporal_id = ptr_frame_header_OBU->ptr_OBU->obu_header.obu_extension_header.temporal_id;
											uint8_t spatial_id = ptr_frame_header_OBU->ptr_OBU->obu_header.obu_extension_header.spatial_id;
											uint32_t inTemporalLayer = (opPtIdc >> temporal_id) & 1;
											uint32_t inSpatialLayer = (opPtIdc >> (spatial_id + 8)) & 1;
											if (opPtIdc == 0 || (inTemporalLayer && inSpatialLayer)) {
												uint8_t n = (uint8_t)(ptr_sequence_header_obu->ptr_decoder_model_info->buffer_removal_time_length_minus_1 + 1);
												bsrb1(in_bst, buffer_removal_time[opNum], n);
											}
										}
									}
								}
							}

							allow_high_precision_mv = 0;
							use_ref_frame_mvs = 0;
							allow_intrabc = 0;

							if (frame_type == KEY_FRAME)
							{
								if (show_frame)
									refresh_frame_flags = (uint8_t)allFrames;
								else
								{
									bsrb1(in_bst, refresh_frame_flags, 8);
								}

								for (int i = 0; i < REFS_PER_FRAME; ++i) {
									ctx_video_bst->ref_frame_idx[i] = -1;
								}
							}
							else
							{
								if (FrameIsIntra)
								{
									bsrb1(in_bst, refresh_frame_flags, 8);
									if (refresh_frame_flags == 0xFF)
										printf("[AV1] Intra only frames cannot have refresh flags 0xFF.\n");
								}
								else
								{
									if (frame_type == SWITCH_FRAME)
										refresh_frame_flags = (uint8_t)allFrames;
									else
									{
										bsrb1(in_bst, refresh_frame_flags, 8);
									}
								}
							}

							ctx_video_bst->refresh_frame_flags = refresh_frame_flags;

							if (!FrameIsIntra || refresh_frame_flags != allFrames)
							{
								if (error_resilient_mode && ptr_sequence_header_obu->enable_order_hint)
								{
									for (uint8_t ref_idx = 0; ref_idx < NUM_REF_FRAMES; ref_idx++)
									{
										bsrb1(in_bst, ref_order_hint[ref_idx], ptr_sequence_header_obu->OrderHintBits);
										int buf_idx = ctx_video_bst->VBI[ref_idx];
										assert(buf_idx < NUM_REF_FRAMES);
										if (buf_idx == -1 || ref_order_hint[ref_idx] != frame_bufs[buf_idx].current_order_hint)
										{
											if (buf_idx >= 0)
											{
												// ref_order_hint is already messed, try to release this frame_buffer in buffer pool
												// If there is no other references, it will be revoked into frame buffer pool
												ctx_video_bst->buffer_pool->decrease_ref_count(buf_idx);
											}

											// If no corresponding buffer exists, allocate a new buffer
											// The purpose of allocating a new buffer is to update ref_order_hint into it
											buf_idx = ctx_video_bst->buffer_pool->get_free_fb();
											if (buf_idx == -1)
												printf("[AV1] Unable to find free frame buffer.\n");

											ctx_video_bst->VBI[ref_idx] = buf_idx;
											frame_bufs[buf_idx].current_order_hint = ref_order_hint[ref_idx];
											frame_bufs[buf_idx].valid_for_referencing = false;
										}
									}
								}
							}

							if (frame_type == KEY_FRAME)
							{
								av1_read_ref(in_bst, ptr_frame_size, FRAME_SIZE, this);
								av1_read_ref(in_bst, ptr_render_size, RENDER_SIZE, ptr_frame_size);
								if (allow_screen_content_tools && ptr_frame_size->ptr_superres_params->UpscaledWidth == ptr_frame_size->FrameWidth) {
									bsrb1(in_bst, allow_intrabc, 1);
								}
							}
							else
							{
								if (frame_type == INTRA_ONLY_FRAME)
								{
									av1_read_ref(in_bst, ptr_frame_size, FRAME_SIZE, this);
									av1_read_ref(in_bst, ptr_render_size, RENDER_SIZE, ptr_frame_size);
									if (allow_screen_content_tools && ptr_frame_size->ptr_superres_params->UpscaledWidth == ptr_frame_size->FrameWidth) {
										bsrb1(in_bst, allow_intrabc, 1);
									}
								}
								else
								{
									if (!ptr_sequence_header_obu->enable_order_hint) {
										frame_refs_short_signaling = 0;
									}
									else
									{
										bsrb1(in_bst, frame_refs_short_signaling, 1);
										if (frame_refs_short_signaling)
										{
											bsrb1(in_bst, last_frame_idx, 3);
											bsrb1(in_bst, gold_frame_idx, 3);
											ctx_video_bst->set_frame_refs((int8_t)last_frame_idx, (int8_t)gold_frame_idx, order_hint);
										}
									}

									for (int i = 0; i < REFS_PER_FRAME; i++)
									{
										if (!frame_refs_short_signaling)
										{
											bsrb1(in_bst, ref_frame_idx[i], 3);

											// This is a index which point to a buffer in buffer pool
											// You can image VBI is VBI
											const int idx = ctx_video_bst->VBI[ref_frame_idx[i]];

											if (idx == -1)
												printf("[AV1] Inter frame requests nonexistent reference at VBI#%d.\n", ref_frame_idx[i]);
											
											ctx_video_bst->ref_frame_idx[i] = ref_frame_idx[i];
										}

										if (ptr_sequence_header_obu->frame_id_numbers_present_flag) {
											uint8_t n = ptr_sequence_header_obu->delta_frame_id_length_minus_2 + 2;
											bsrb1(in_bst, delta_frame_id_minus_1[i], n);
											uint32_t DeltaFrameId = delta_frame_id_minus_1[i] + 1;
											expectedFrameId[i] = ((current_frame_id + (1 << idLen) - DeltaFrameId) % (1 << idLen));

											// Compare values derived from delta_frame_id_minus_1 and refresh_frame_flags. Also, check valid for referencing
											if (ctx_video_bst->ref_frame_idx[i] >= 0)
											{
												auto buf_idx = ctx_video_bst->VBI[ctx_video_bst->ref_frame_idx[i]];
												if (buf_idx >= 0 && ((int64_t)expectedFrameId[i] != (int64_t)frame_bufs[buf_idx].current_frame_id || !frame_bufs[buf_idx].valid_for_referencing))
													printf("[AV1] Reference buffer frame ID mismatch.\n");
											}
										}
									}

									if (frame_size_override_flag && !error_resilient_mode)
									{
										av1_read_ref(in_bst, ptr_frame_size_with_refs, FRAME_SIZE_WITH_REFS, this);
									}
									else
									{
										av1_read_ref(in_bst, ptr_frame_size, FRAME_SIZE, this);
										av1_read_ref(in_bst, ptr_render_size, RENDER_SIZE, ptr_frame_size);
									}

									if (force_integer_mv)
									{
										allow_high_precision_mv = 0;
									}
									else
									{
										bsrb1(in_bst, allow_high_precision_mv, 1);
									}

									av1_read_ref(in_bst, ptr_read_interpolation_filter, READ_INTERPOLATION_FILTER);
									bsrb1(in_bst, is_motion_mode_switchable, 1);

									if (error_resilient_mode || !ptr_sequence_header_obu->enable_ref_frame_mvs)
										use_ref_frame_mvs = 0;
									else
									{
										bsrb1(in_bst, use_ref_frame_mvs, 1);
									}
								}

								if (primary_ref_frame != PRIMARY_REF_NONE && (
									ctx_video_bst->ref_frame_idx[primary_ref_frame] < 0 ||
									ctx_video_bst->VBI[ctx_video_bst->ref_frame_idx[primary_ref_frame]] < 0))
								{
									printf("[AV1] Reference frame containing this frame's initial frame context is unavailable.\n");
								}
							}

							// 7.8. Set frame refs process
							ctx_video_bst->av1_setup_frame_buf_refs(order_hint);

							if (!FrameIsIntra)
							{
								for (int ref_frame = LAST_FRAME; ref_frame <= ALTREF_FRAME; ++ref_frame)
								{
									if (ctx_video_bst->SingleOBUParse)
									{
										if (ctx_video_bst->tu_fu_idx >= 0 && (size_t)ctx_video_bst->tu_fu_idx < ctx_video_bst->RefFrameSignBiases.size())
										{
											RefFrameSignBias = ctx_video_bst->RefFrameSignBiases[ctx_video_bst->tu_fu_idx];
										}
									}
									else
									{
										int8_t cur_ref_frame_idx = ctx_video_bst->ref_frame_idx[ref_frame - LAST_FRAME];
										const int buf_idx = cur_ref_frame_idx >= 0 ? ctx_video_bst->VBI[cur_ref_frame_idx] : -1;

										if (!ptr_sequence_header_obu->enable_order_hint || buf_idx == -1)
										{
											RefFrameSignBias &= ~(1<< (ref_frame - LAST_FRAME));
										}
										else
										{
											auto ref_frame_order_hint = ctx_video_bst->buffer_pool->frame_bufs[buf_idx].current_order_hint;

											ctx_video_bst->buffer_pool->frame_bufs[ctx_video_bst->cfbi].ref_order_hints[ref_frame - LAST_FRAME] = ref_frame_order_hint;
											bool bBackward = ptr_sequence_header_obu->get_relative_dist(ref_frame_order_hint, order_hint) > 0;
											if (bBackward)
												RefFrameSignBias |= 1 << (ref_frame - LAST_FRAME);
											else
												RefFrameSignBias &= ~(1 << (ref_frame - LAST_FRAME));
										}
									}
								}

								if (g_verbose_level > 200)
								{
									printf("ref_frame_idx: [%d,%d,%d,%d,%d,%d,%d]\n",
										ctx_video_bst->ref_frame_idx[0], ctx_video_bst->ref_frame_idx[1], ctx_video_bst->ref_frame_idx[2],
										ctx_video_bst->ref_frame_idx[3], ctx_video_bst->ref_frame_idx[4], ctx_video_bst->ref_frame_idx[5], ctx_video_bst->ref_frame_idx[6]);

									printf("OrderHints: [%d,%d,%d,%d,%d,%d,%d]\n",
										ctx_video_bst->buffer_pool->frame_bufs[ctx_video_bst->cfbi].ref_order_hints[0],
										ctx_video_bst->buffer_pool->frame_bufs[ctx_video_bst->cfbi].ref_order_hints[1],
										ctx_video_bst->buffer_pool->frame_bufs[ctx_video_bst->cfbi].ref_order_hints[2],
										ctx_video_bst->buffer_pool->frame_bufs[ctx_video_bst->cfbi].ref_order_hints[3],
										ctx_video_bst->buffer_pool->frame_bufs[ctx_video_bst->cfbi].ref_order_hints[4],
										ctx_video_bst->buffer_pool->frame_bufs[ctx_video_bst->cfbi].ref_order_hints[5],
										ctx_video_bst->buffer_pool->frame_bufs[ctx_video_bst->cfbi].ref_order_hints[6]);
								}
							}

							frame_bufs[ctx_video_bst->cfbi].frame_type = (AV1_FRAME_TYPE)frame_type;

							if (ptr_sequence_header_obu->frame_id_numbers_present_flag)
							{
								/* If bitmask is set, update reference frame id values and mark frames as valid for reference */
								for (int i = 0; i < NUM_REF_FRAMES; i++) {
									if ((refresh_frame_flags >> i) & 1) {
										auto buf_idx = ctx_video_bst->VBI[i];
										if (buf_idx >= 0)
										{
											frame_bufs[buf_idx].current_frame_id = current_frame_id;
											frame_bufs[buf_idx].valid_for_referencing = true;
										}
									}
								}
							}

							if (ptr_sequence_header_obu->reduced_still_picture_header || disable_cdf_update)
								disable_frame_end_update_cdf = 0;
							else
							{
								bsrb1(in_bst, disable_frame_end_update_cdf, 1);
							}

							auto cur_frame = ctx_video_bst->buffer_pool->frame_bufs[ctx_video_bst->cfbi];

							if (ptr_frame_size_with_refs != nullptr)
							{
								if (ptr_frame_size_with_refs->found_ref.UpperBound() == 0)
								{
									cur_frame.upscaled_width = ptr_frame_size_with_refs->ptr_frame_size->ptr_superres_params->UpscaledWidth;
									cur_frame.width = ptr_frame_size_with_refs->ptr_frame_size->FrameWidth;
									cur_frame.height = ptr_frame_size_with_refs->ptr_frame_size->FrameHeight;
									cur_frame.render_width = ptr_frame_size_with_refs->ptr_render_size->RenderWidth;
									cur_frame.render_height = ptr_frame_size_with_refs->ptr_render_size->RenderHeight;
								}
								else
								{
									cur_frame.upscaled_width = ptr_frame_size_with_refs->ptr_superres_params->UpscaledWidth;
									cur_frame.width = ptr_frame_size_with_refs->FrameWidth;
									cur_frame.height = ptr_frame_size_with_refs->FrameHeight;
									cur_frame.render_width = ptr_frame_size_with_refs->RenderWidth;
									cur_frame.render_height = ptr_frame_size_with_refs->RenderHeight;
								}
							}
							else
							{
								cur_frame.upscaled_width = ptr_frame_size->ptr_superres_params->UpscaledWidth;
								cur_frame.width = ptr_frame_size->FrameWidth;
								cur_frame.height = ptr_frame_size->FrameHeight;
								cur_frame.render_width = ptr_render_size->RenderWidth;
								cur_frame.render_height = ptr_render_size->RenderHeight;
							}

							if (primary_ref_frame == PRIMARY_REF_NONE)
							{
								// init_non_coeff_cdfs( )
								ctx_video_bst->setup_past_independence();
							}
							else
							{
								// load_cdfs( ref_frame_idx[primary_ref_frame] )
								ctx_video_bst->load_previous(primary_ref_frame);
							}

							if (use_ref_frame_mvs == 1)
							{
								// motion_field_estimation( )
							}

							av1_read_ref(in_bst, ptr_tile_info, TILE_INFO, this);
							av1_read_ref(in_bst, ptr_quantization_params, QUANTIZATION_PARAMS, this);
							av1_read_ref(in_bst, ptr_segmentation_params, SEGMENTATION_PARAMS, this);
							av1_read_ref(in_bst, ptr_delta_q_params, DELTA_Q_PARAMS, this);
							av1_read_ref(in_bst, ptr_delta_lf_params, DELTA_LF_PARAMS, this);

							if (primary_ref_frame == PRIMARY_REF_NONE)
							{
								// init_coeff_cdfs()
							}
							else
							{
								// load_previous_segment_ids()
							}

							CodedLossless = 1;
							for (uint8_t segmentId = 0; segmentId < MAX_SEGMENTS; segmentId++)
							{
								uint8_t qindex = get_qindex(1, segmentId);
								LosslessArray[segmentId] = qindex == 0 && 
									ptr_quantization_params->DeltaQYDc == 0 &&
									ptr_quantization_params->DeltaQUAc == 0 && ptr_quantization_params->DeltaQUDc == 0 &&
									ptr_quantization_params->DeltaQVAc == 0 && ptr_quantization_params->DeltaQVDc == 0;

								if (!LosslessArray[segmentId])
									CodedLossless = 0;
								if (ptr_quantization_params->using_qmatrix)
								{
									if (LosslessArray[segmentId])
									{
										SegQMLevel[0][segmentId] = 15;
										SegQMLevel[1][segmentId] = 15;
										SegQMLevel[2][segmentId] = 15;
									}
									else
									{
										SegQMLevel[0][segmentId] = ptr_quantization_params->qm_y;
										SegQMLevel[1][segmentId] = ptr_quantization_params->qm_u;
										SegQMLevel[2][segmentId] = ptr_quantization_params->qm_v;
									}
								}
							}

							AllLossless = (CodedLossless && (GetFrameWidth() == GetUpscaleWidth())) ? 1 : 0;

							av1_read_ref(in_bst, ptr_loop_filter_params, LOOP_FILTER_PARAMS, this);
							av1_read_ref(in_bst, ptr_cdef_params, CDEF_PARAMS, this);
							av1_read_ref(in_bst, ptr_lr_params, LR_PARAMS, this);
							av1_read_ref(in_bst, ptr_read_tx_mode, READ_TX_MODE, this);
							av1_read_ref(in_bst, ptr_frame_reference_mode, FRAME_REFERENCE_MODE, this);
							av1_read_ref(in_bst, ptr_skip_mode_params, SKIP_MODE_PARAMS, this);

							if (FrameIsIntra || error_resilient_mode || !ptr_sequence_header_obu->enable_warped_motion)
							{
								allow_warped_motion = 0;
							}
							else
							{
								bsrb1(in_bst, allow_warped_motion, 1);
							}

							bsrb1(in_bst, reduced_tx_set, 1);

							av1_read_ref(in_bst, ptr_global_motion_params, GLOBAL_MOTION_PARAMS, this);
							av1_read_ref(in_bst, ptr_film_grain_params, FILM_GRAIN_PARAMS, this);

							MAP_BST_END();
						}
						catch (AMException e)
						{
							return e.RetCode();
						}

					done:
						return RET_CODE_SUCCESS;
					}

					uint16_t GetFrameWidth() {
						if (ptr_frame_size_with_refs != nullptr)
						{
							if (ptr_frame_size_with_refs->found_ref[0] == 1)
								return ptr_frame_size_with_refs->ptr_frame_size->FrameWidth;
							else
								return ptr_frame_size_with_refs->ptr_superres_params->FrameWidth;
						}
						else if (ptr_frame_size != nullptr)
							return ptr_frame_size->FrameWidth;

						return 0;
					}

					uint16_t GetFrameHeight() {
						if (ptr_frame_size_with_refs != nullptr)
						{
							if (ptr_frame_size_with_refs->found_ref[0] == 1)
								return ptr_frame_size_with_refs->ptr_frame_size->FrameHeight;
							else
								return ptr_frame_size_with_refs->FrameHeight;
						}
						else if (ptr_frame_size != nullptr)
							return ptr_frame_size->FrameHeight;

						return 0;
					}

					uint16_t GetUpscaleWidth() {
						if (ptr_frame_size_with_refs != nullptr)
						{
							if (ptr_frame_size_with_refs->found_ref[0] == 1)
								return ptr_frame_size_with_refs->ptr_frame_size->ptr_superres_params->UpscaledWidth;
							else
								return ptr_frame_size_with_refs->ptr_superres_params->UpscaledWidth;
						}
						else if (ptr_frame_size != nullptr)
							return ptr_frame_size->ptr_superres_params->UpscaledWidth;

						return 0;
					}

					int get_qindex(bool ignoreDeltaQ, uint8_t segmentId)
					{
						if (ptr_segmentation_params->segmentation_enabled && ptr_segmentation_params->FeatureEnabled[segmentId][SEG_LVL_ALT_Q])
						{
							const int data = ptr_segmentation_params->FeatureData[segmentId][SEG_LVL_ALT_Q];
							const int seg_qindex = ptr_quantization_params->base_q_idx + data;
							if (!ignoreDeltaQ && ptr_delta_q_params->delta_q_present) {
								// Not implement yet
								assert(0);
							}
							return AV1_Clip3(0, 255, seg_qindex);
						}
						else if (!ignoreDeltaQ && ptr_delta_q_params->delta_q_present)
						{
							// Not implement yet
							assert(0);
						}
						return ptr_quantization_params->base_q_idx;
					}

					void mark_ref_frames(SEQUENCE_HEADER_OBU* ptr_seqhdr_obu, uint8_t idLen)
					{
						auto ctx_video_bst = ptr_seqhdr_obu->ptr_OBU->ctx_video_bst;
						uint8_t diffLen = ptr_seqhdr_obu->delta_frame_id_length_minus_2 + 2;
						for (uint8_t i = 0; i < NUM_REF_FRAMES; i++)
						{
							if (ctx_video_bst->VBI[i] < 0)
								continue;

							// The previous decoded frame should be available, in another word
							// For the frames in VBI slot should be available, they are available, and accessible.
							auto RefFrameId = ctx_video_bst->buffer_pool->frame_bufs[ctx_video_bst->VBI[i]].current_frame_id;

							if (current_frame_id > ((int32_t)i << diffLen)) {
								if (RefFrameId > current_frame_id || RefFrameId + (1 << diffLen) < (current_frame_id))
								{
									RefValid_of_mark_ref_frames[i] = false;
									auto buf_idx = ctx_video_bst->VBI[i];
									if (buf_idx >= 0)
										ctx_video_bst->buffer_pool->frame_bufs[buf_idx].valid_for_referencing = false;
								}
							}
							else
							{
								if (RefFrameId > current_frame_id && RefFrameId + (1 << diffLen) < (1 << idLen) + current_frame_id)
								{
									RefValid_of_mark_ref_frames[i] = false;
									auto buf_idx = ctx_video_bst->VBI[i];
									if (buf_idx >= 0)
										ctx_video_bst->buffer_pool->frame_bufs[buf_idx].valid_for_referencing = false;
								}
							}
						}
					}

					int Unmap(AMBst out_bst)
					{
						UNREFERENCED_PARAMETER(out_bst);
						return RET_CODE_ERROR_NOTIMPL;
					}

					DECLARE_FIELDPROP_BEGIN()
					uint8_t idLen = 0;
					BOOL FrameIsIntra = FALSE;
					auto ctx_video_bst = ptr_frame_header_OBU->ptr_OBU->ctx_video_bst;
					uint32_t allFrames = (1 << NUM_REF_FRAMES) - 1;

					if (ptr_sequence_header_obu == nullptr)
					{
						goto done;
					}

					if (ptr_sequence_header_obu->frame_id_numbers_present_flag)
					{
						idLen = ptr_sequence_header_obu->additional_frame_id_length_minus_1 + ptr_sequence_header_obu->delta_frame_id_length_minus_2 + 3;
						NAV_WRITE_TAG_WITH_NUMBER_VALUE("idLen", idLen, "idLen = (additional_frame_id_length_minus_1 + delta_frame_id_length_minus_2 + 3)");
					}

					NAV_WRITE_TAG_WITH_NUMBER_VALUE("allFrames", allFrames, "allFrames = (1&lt;&lt;NUM_REF_FRAMES)-1");
					if (ptr_sequence_header_obu->reduced_still_picture_header)
					{
						NAV_WRITE_TAG_WITH_NUMBER_VALUE("show_existing_frame", show_existing_frame, "Should be 0");
						NAV_WRITE_TAG_WITH_NUMBER_VALUE("frame_type", frame_type, "Should be KEY_FRAME");
						NAV_WRITE_TAG_WITH_NUMBER_VALUE("FrameIsIntra", 1, "Should be 1");
						FrameIsIntra = 1;
						NAV_WRITE_TAG_WITH_NUMBER_VALUE("show_frame", show_frame, "Should be 1");
						NAV_WRITE_TAG_WITH_NUMBER_VALUE("showable_frame", showable_frame, "Should be 0");
					}
					else
					{
						BST_FIELD_PROP_BOOL_BEGIN(show_existing_frame, "indicates the frame indexed by frame_to_show_map_idx is to be output", 
							"indicates that further processing is required");
						if (show_existing_frame == 1)
						{
							BST_FIELD_PROP_2NUMBER1(frame_to_show_map_idx, 3, "specifies the frame to be output");
							if (ptr_sequence_header_obu->decoder_model_info_present_flag && !ptr_sequence_header_obu->ptr_timing_info->equal_picture_interval) {
								NAV_WRITE_TAG_BEGIN_WITH_ALIAS("temporal_point_info", "temporal_point_info()", "");
								BST_FIELD_PROP_2NUMBER("frame_presentation_time",
									(int)(ptr_sequence_header_obu->ptr_decoder_model_info->frame_presentation_time_length_minus_1 + 1),
									temporal_point_info.frame_presentation_time,
									"the presentation time of the frame in clock ticks DispCT counted from the removal time of "
									"the last random access point for the operating point that is being decoded");
								NAV_WRITE_TAG_END("temporal_point_info");
							}
							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(refresh_frame_flags, "Should be 0");
							if (ptr_sequence_header_obu->frame_id_numbers_present_flag) {
								BST_FIELD_PROP_2NUMBER1(display_frame_id, idLen, "provides the frame id number for the frame to output");
							}

							NAV_WRITE_TAG_WITH_NUMBER_VALUE_DESC_F("frame_type", frame_type, 
								"%s, frame_type = RefFrameType[frame_to_show_map_idx]", AV1_FRAME_TYPE_NAMEA(frame_type));

							if (frame_type == KEY_FRAME) {
								NAV_WRITE_TAG_WITH_NUMBER_VALUE1(refresh_frame_flags, "refresh_frame_flags = allFrames");
							}
							/*
							if (film_grain_params_present) {
								load_grain_params(frame_to_show_map_idx)
							}
							return
							*/
						}
						BST_FIELD_PROP_BOOL_END("show_existing_frame");

						if (show_existing_frame == 1)
						{
							goto done;
						}

						BST_FIELD_PROP_2NUMBER1(frame_type, 2, AV1_FRAME_TYPE_NAMEA(frame_type));
						FrameIsIntra = (frame_type == INTRA_ONLY_FRAME || frame_type == KEY_FRAME);
						NAV_WRITE_TAG_WITH_1NUMBER_VALUE("FrameIsIntra", FrameIsIntra ? 1 : 0, "FrameIsIntra=(frame_type==INTRA_ONLY_FRAME || frame_type==KEY_FRAME)");
						BST_FIELD_PROP_BOOL(show_frame, "specifies that this frame should be immediately output once decoded", 
							"specifies that this frame should not be immediately output");
						if (show_frame && ptr_sequence_header_obu->decoder_model_info_present_flag && !ptr_sequence_header_obu->ptr_timing_info->equal_picture_interval) {
							NAV_WRITE_TAG_BEGIN_WITH_ALIAS("temporal_point_info", "temporal_point_info()", "");
							BST_FIELD_PROP_2NUMBER("frame_presentation_time",
								(int)(ptr_sequence_header_obu->ptr_decoder_model_info->frame_presentation_time_length_minus_1 + 1),
								temporal_point_info.frame_presentation_time,
								"the presentation time of the frame in clock ticks DispCT counted from the removal time of "
								"the last random access point for the operating point that is being decoded");
							NAV_WRITE_TAG_END("temporal_point_info");
						}

						if (show_frame) {
							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(showable_frame, "showable_frame = frame_type != KEY_FRAME");
						}
						else
						{
							BST_FIELD_PROP_BOOL(showable_frame, "specifies that the frame may be output using the show_existing_frame mechanism", 
								"specifies that this frame will not be output using the show_existing_frame mechanism");
						}

						if (frame_type == SWITCH_FRAME || (frame_type == KEY_FRAME && show_frame))
						{
							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(error_resilient_mode, "Should be 1");
						}
						else
						{
							BST_FIELD_PROP_BOOL(error_resilient_mode, "indicates that error resilient mode is enabled", "indicates that error resilient mode is disabled");
						}
					}

					if (frame_type == SWITCH_FRAME && show_frame)
					{
						NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "for(i=0; i&lt;NUM_REF_FRAMES; i++)", "");
						for (int i = 0; i < NUM_REF_FRAMES; i++)
						{
							NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("RefValid", "RefValid[%d]", 0, "", i);
							NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("RefOrderHint", "RefOrderHint[%d]", 0, "", i);
						}
						NAV_WRITE_TAG_END("Tag0");

						NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag1", "for(i=0; i&lt;REFS_PER_FRAME; i++ )", "");
						for (int i = 0; i < REFS_PER_FRAME; i++)
						{
							NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("OrderHints", "OrderHints[%d]", 0, "", i);
						}
						NAV_WRITE_TAG_END("Tag1");
					}

					BST_FIELD_PROP_NUMBER1(disable_cdf_update, 1, "specifies whether the CDF update in the symbol decoding process should be disabled");

					if (ptr_sequence_header_obu->seq_force_screen_content_tools != SELECT_SCREEN_CONTENT_TOOLS ||
						map_status.status == 0 || (map_status.error == 0 && map_status.number_of_fields > 0 && field_prop_idx < map_status.number_of_fields))
					{
						if (ptr_sequence_header_obu->seq_force_screen_content_tools != SELECT_SCREEN_CONTENT_TOOLS)
						{
							NAV_WRITE_TAG_WITH_1NUMBER_VALUE_BEGIN("allow_screen_content_tools", "", allow_screen_content_tools, "");
						}
						else
						{

							MBCSPRINTF_S(szTemp3, TEMP3_SIZE, "%d", (int)allow_screen_content_tools);
							NAV_FIELD_PROP_BEGIN("allow_screen_content_tools", 1, szTemp3,
								allow_screen_content_tools ? "indicates that intra blocks may use palette encoding" : "indicates that palette encoding is never used",
								bit_offset ? *bit_offset : -1LL, "I");
							if (bit_offset)*bit_offset += 1;
							field_prop_idx++;
						}

						if (allow_screen_content_tools)
						{
							if (ptr_sequence_header_obu->seq_force_integer_mv == SELECT_INTEGER_MV) {
								BST_FIELD_PROP_BOOL(force_integer_mv, "specifies that motion vectors will always be integers", 
									"specifies that motion vectors can contain fractional bits");
							}
							else
							{
								NAV_WRITE_TAG_WITH_NUMBER_VALUE1(force_integer_mv, "force_integer_mv = seq_force_integer_mv");
							}
						}
						else
						{
							NAV_WRITE_TAG_WITH_NUMBER_VALUE1(force_integer_mv, "");
						}

						NAV_WRITE_TAG_END("allow_screen_content_tools");
					}

					if (FrameIsIntra)
					{
						NAV_WRITE_TAG_WITH_NUMBER_VALUE1(force_integer_mv, "Should be 1 since the current frame is intra");
					}

					if (ptr_sequence_header_obu->frame_id_numbers_present_flag)
					{
						BST_FIELD_PROP_2NUMBER1(current_frame_id, idLen, "specifies the frame id number for the current frame");

						NAV_WRITE_TAG_BEGIN_WITH_ALIAS("mark_ref_frames", "mark_ref_frames()", "");
							uint8_t diffLen = ptr_sequence_header_obu->delta_frame_id_length_minus_2 + 2;
							NAV_WRITE_TAG_WITH_NUMBER_VALUE("diffLen", diffLen, "diffLen=delta_frame_id_length_minus_2+2");
							NAV_WRITE_TAG_BEGIN_WITH_ALIAS("mark_ref_frames_tag0", "for(i=0;i&lt;NUM_REF_FRAMES;i++ )", "");
							for (i = 0; i < NUM_REF_FRAMES; i++)
							{
								if (RefValid_of_mark_ref_frames[i] == 0) {
									char szCondition[256] = { 0 };
									MBCSPRINTF_S(szCondition, _countof(szCondition), "current_frame_id&gt;(1&lt;&lt;diffLen: %s", 
										current_frame_id > ((int32_t)1 << diffLen)?"true":"false");
									NAV_WRITE_TAG_WITH_ALIAS_F("mark_ref_frames_tag1", "RefValid[%d] = 0", szCondition, i);
								}
							}
							NAV_WRITE_TAG_END("mark_ref_frames_tag0");

						NAV_WRITE_TAG_END("mark_ref_frames");
					}
					else
					{
						NAV_WRITE_TAG_WITH_NUMBER_VALUE1(current_frame_id, "Should be 0");
					}

					if (frame_type == SWITCH_FRAME)
					{
						NAV_WRITE_TAG_WITH_NUMBER_VALUE1(frame_size_override_flag, "Should be 1");
					}
					else if (ptr_sequence_header_obu->reduced_still_picture_header)
					{
						NAV_WRITE_TAG_WITH_NUMBER_VALUE1(frame_size_override_flag, "Should be 0");
					}
					else
					{
						BST_FIELD_PROP_BOOL(frame_size_override_flag, 
							"specifies that the frame size will either be specified as the size of one of the reference frames, "
							"or computed from the frame_width_minus_1 and frame_height_minus_1 syntax elements", 
							"specifies that the frame size is equal to the size in the sequence header");
					}

					BST_FIELD_PROP_2NUMBER1(order_hint, ptr_sequence_header_obu->OrderHintBits, "is used to compute OrderHint");
					NAV_WRITE_TAG_WITH_NUMBER_VALUE("OrderHint", order_hint, "specifies OrderHintBits least significant bits of the expected output order for this frame");

					if (FrameIsIntra || error_resilient_mode)
					{
						NAV_WRITE_TAG_WITH_NUMBER_VALUE1(primary_ref_frame, "primary_ref_frame = PRIMARY_REF_NONE");
					}
					else
					{
						BST_FIELD_PROP_2NUMBER1(primary_ref_frame, 3, 
							"specifies which reference frame contains the CDF values and other state that should be loaded at the start of the frame");
					}

					if (ptr_sequence_header_obu->decoder_model_info_present_flag)
					{
						BST_FIELD_PROP_BOOL_BEGIN(buffer_removal_time_present_flag, "specifies that buffer_removal_time is present", 
							"specifies that buffer_removal_time is not present");
						if (buffer_removal_time_present_flag)
						{
							NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag2", "for(opNum=0;opNum&lt;=operating_points_cnt_minus_1;opNum++ )", "buffer_removal_time information");
							for (int opNum = 0; opNum <= ptr_sequence_header_obu->operating_points_cnt_minus_1; opNum++)
							{
								if (ptr_sequence_header_obu->operating_points[opNum].decoder_model_present_for_this_op)
								{
									uint32_t opPtIdc = ptr_sequence_header_obu->operating_points[opNum].operating_point_idc;
									uint8_t temporal_id = ptr_frame_header_OBU->ptr_OBU->obu_header.obu_extension_header.temporal_id;
									uint8_t spatial_id = ptr_frame_header_OBU->ptr_OBU->obu_header.obu_extension_header.spatial_id;
									uint32_t inTemporalLayer = (opPtIdc >> temporal_id) & 1;
									uint32_t inSpatialLayer = (opPtIdc >> (spatial_id + 8)) & 1;
									NAV_WRITE_TAG_WITH_NUMBER_VALUE("opPtIdc", opPtIdc, "opPtIdc = operating_point_idc[opNum]");
									NAV_WRITE_TAG_WITH_NUMBER_VALUE("inTemporalLayer", inTemporalLayer, "inTemporalLayer = (opPtIdc&gt;&gt;temporal_id) &amp; 1");
									NAV_WRITE_TAG_WITH_NUMBER_VALUE("inSpatialLayer", inSpatialLayer, "inSpatialLayer = (opPtIdc &gt;&gt; (spatial_id + 8)) &amp; 1");
									if (opPtIdc == 0 || (inTemporalLayer && inSpatialLayer)) {
										uint8_t n = (uint8_t)(ptr_sequence_header_obu->ptr_decoder_model_info->buffer_removal_time_length_minus_1 + 1);
										NAV_WRITE_TAG_WITH_NUMBER_VALUE("n", n, "n = buffer_removal_time_length_minus_1 + 1");
										BST_ARRAY_FIELD_PROP_NUMBER("buffer_removal_time", opNum, n, buffer_removal_time[opNum], 
											"specifies the frame removal time in units of DecCT clock ticks counted from the removal time of "
											"the last random access point for operating point opNum. "
											"buffer_removal_time is signaled as a fixed length unsigned integer with a length in bits given "
											"by buffer_removal_time_length_minus_1+1")
									}
								}
							}
							NAV_WRITE_TAG_END("Tag2");
						}
						BST_FIELD_PROP_BOOL_END("buffer_removal_time_present_flag");
					}

					NAV_WRITE_TAG_WITH_ALIAS("allow_high_precision_mv", "allow_high_precision_mv = 0", 
						"initialize its value to 0, specifies that motion vectors are specified to quarter pel precision");
					NAV_WRITE_TAG_WITH_ALIAS("use_ref_frame_mvs", "use_ref_frame_mvs = 0", 
						"initialize its value to 0, specifies that this information will not be used");
					NAV_WRITE_TAG_WITH_ALIAS("allow_intrabc", "allow_intrabc = 0", 
						"initialize its value to 0, indicates that intra block copy is not allowed in this frame");

					if (frame_type == SWITCH_FRAME || (frame_type == KEY_FRAME && show_frame)){
						NAV_WRITE_TAG_WITH_NUMBER_VALUE1(refresh_frame_flags, 
							"contains a bitmask that specifies which reference frame slots will be updated with the current frame after it is decoded.");
					}
					else
					{
						BST_FIELD_PROP_2NUMBER1(refresh_frame_flags, 8, 
							"contains a bitmask that specifies which reference frame slots will be updated with the current frame after it is decoded.");
					}

					if (!FrameIsIntra || refresh_frame_flags != allFrames)
					{
						if (error_resilient_mode && ptr_sequence_header_obu->enable_order_hint)
						{
							for (uint8_t i = 0; i < NUM_REF_FRAMES; i++)
							{
								BST_ARRAY_FIELD_PROP_NUMBER1(ref_order_hint, i, ptr_sequence_header_obu->OrderHintBits, 
									"specifies the expected output order hint for each reference frame");
								//if (ref_order_hint[i] != RefOrderHint[i]) {
								//	RefValid[i] = 0;
								//}
							}
						}
					}

					if (frame_type == KEY_FRAME)
					{
						BST_FIELD_PROP_REF2_1(ptr_frame_size, "frame_size", "");
						BST_FIELD_PROP_REF2_1(ptr_render_size, "render_size", 
							"The render size is provided as a hint to the application about the desired display size. It has no effect on the decoding process");
						if (allow_screen_content_tools && ptr_frame_size->ptr_superres_params->UpscaledWidth == ptr_frame_size->FrameWidth) {
							BST_FIELD_PROP_BOOL(allow_intrabc, 
								"indicates that intra block copy may be used in this frame", 
								"indicates that intra block copy is not allowed in this frame");
						}
					}
					else
					{
						if (frame_type == INTRA_ONLY_FRAME)
						{
							BST_FIELD_PROP_REF2_1(ptr_frame_size, "frame_size", "");
							BST_FIELD_PROP_REF2_1(ptr_render_size, "render_size", 
								"The render size is provided as a hint to the application about the desired display size. It has no effect on the decoding process");
							if (allow_screen_content_tools && ptr_frame_size->ptr_superres_params->UpscaledWidth == ptr_frame_size->FrameWidth) {
								BST_FIELD_PROP_BOOL(allow_intrabc,
									"indicates that intra block copy may be used in this frame",
									"indicates that intra block copy is not allowed in this frame");
							}
						}
						else
						{
							if (!ptr_sequence_header_obu->enable_order_hint) 
							{
								NAV_WRITE_TAG_WITH_NUMBER_VALUE1(frame_refs_short_signaling, "Should be 0, indicates that all reference frames are explicitly signaled");
							}
							else
							{
								if (frame_refs_short_signaling)
								{
									BST_FIELD_PROP_BOOL_BEGIN(frame_refs_short_signaling, 
										"indicates that only two reference frames are explicitly signaled", 
										"indicates that all reference frames are explicitly signaled");
										BST_FIELD_PROP_2NUMBER1(last_frame_idx, 3, "specifies the reference frame to use for LAST_FRAME");
										BST_FIELD_PROP_2NUMBER1(gold_frame_idx, 3, "specifies the reference frame to use for GOLDEN_FRAME");
									NAV_WRITE_TAGALIAS_WITH_VALUEFMTSTR("set_frame_refs", "set_frame_refs()", "%s", 
										"A function call that indicates the conceptual point where the ref_frame_idx values are computed", "");
									BST_FIELD_PROP_BOOL_END("frame_refs_short_signaling");
								}
								else
								{
									BST_FIELD_PROP_BOOL(frame_refs_short_signaling,
										"indicates that only two reference frames are explicitly signaled",
										"indicates that all reference frames are explicitly signaled");
								}
							}

							if (!frame_refs_short_signaling)
							{
								BST_FIELD_PROP_NUMBER_ARRAY_F("ref_frame_idx", 21, "%d,%d,%d,%d,%d,%d,%d",
									"specifies which reference frames are used by inter frames."
									"It is a requirement of bitstream conformance"
									"that RefValid[ref_frame_idx[i]] is equal to 1, and that the selected reference frames match the current frame in bit depth,"
									"profile, chroma subsampling, and color space", 
									ref_frame_idx[0], ref_frame_idx[1], ref_frame_idx[2], ref_frame_idx[3], ref_frame_idx[4], ref_frame_idx[5], ref_frame_idx[6]);
							}

							if (ptr_sequence_header_obu->frame_id_numbers_present_flag) {
								NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag3", "for(i=0;i&lt;REFS_PER_FRAME;i++)", "ref_frame_idx and expectedFrameId");
								for (int i = 0; i < REFS_PER_FRAME; i++)
								{
									uint8_t n = ptr_sequence_header_obu->delta_frame_id_length_minus_2 + 2;
									BST_ARRAY_FIELD_PROP_NUMBER1(delta_frame_id_minus_1, i, n, "plus 1 specifies the distance to the frame id for the reference frame");
									NAV_WRITE_TAG_WITH_ALIAS_VALUEFMTSTR_AND_NUMBER_VALUE("expectedFrameId", "expectedFrameId[%d]", "%" PRIu32 "(0X%" PRIX32 ")", 
										expectedFrameId[i],
										"specifies the frame id for each frame used for reference. It is a requirement of bitstream"
										"conformance that whenever expectedFrameId[i] is calculated, the value matches RefFrameId[ref_frame_idx[i]]"
										"(this contains the value of current_frame_id at the time that the frame indexed by ref_frame_idx was stored)", i);
								}
								NAV_WRITE_TAG_END("Tag3");
							}

							if (frame_size_override_flag && !error_resilient_mode)
							{
								BST_FIELD_PROP_REF2_1(ptr_frame_size_with_refs, "frame_size_with_refs", 
									"For inter frames, the frame size is either set equal to the size of a reference frame, or can be sent explicitly");
							}
							else
							{
								BST_FIELD_PROP_REF2_1(ptr_frame_size, "frame_size", "");
								BST_FIELD_PROP_REF2_1(ptr_render_size, "render_size", 
									"The render size is provided as a hint to the application about the desired display size. It has no effect on the decoding process");
							}

							if (force_integer_mv)
							{
								NAV_WRITE_TAG_WITH_NUMBER_VALUE1(allow_high_precision_mv, allow_high_precision_mv
									? "specifies that motion vectors are specified to eighth pel precision"
									: "specifies that motion vectors are specified to quarter pel precision");
							}
							else
							{
								BST_FIELD_PROP_BOOL(allow_high_precision_mv
									, "specifies that motion vectors are specified to eighth pel precision"
									, "specifies that motion vectors are specified to quarter pel precision");
							}

							BST_FIELD_PROP_REF2_1(ptr_read_interpolation_filter, "read_interpolation_filter", "");
							BST_FIELD_PROP_BOOL(is_motion_mode_switchable, "", "specifies that only the SIMPLE motion mode will be used");
							if (error_resilient_mode || !ptr_sequence_header_obu->enable_ref_frame_mvs)
							{
								NAV_WRITE_TAG_WITH_NUMBER_VALUE1(use_ref_frame_mvs, "Should be 0");
							}
							else
							{
								BST_FIELD_PROP_BOOL(use_ref_frame_mvs, 
									"specifies that motion vector information from a previous frame can be used when decoding the current frame", 
									"specifies that motion vector information from a previous frame will not be used");
							}
						}
					}

					if (!FrameIsIntra)
					{
						if (!ctx_video_bst->SingleOBUParse && ptr_frame != nullptr) {
							NAV_WRITE_TAG_WITH_VALUEFMTSTR("OrderHints", "%d,%d,%d,%d,%d,%d,%d", "The order hints of ref frame of current frame",
								ptr_frame->ref_order_hints[0], ptr_frame->ref_order_hints[1], ptr_frame->ref_order_hints[2],
								ptr_frame->ref_order_hints[3], ptr_frame->ref_order_hints[4], ptr_frame->ref_order_hints[5], ptr_frame->ref_order_hints[6]);
						}
						NAV_WRITE_TAG_WITH_VALUEFMTSTR("RefFrameSignBias", "%d,%d,%d,%d,%d,%d,%d", "0, forward reference, 1: backward reference",
							(RefFrameSignBias&(1 << 0)) ? 1 : 0, (RefFrameSignBias&(1 << 1)) ? 1 : 0, (RefFrameSignBias&(1 << 2)) ? 1 : 0,
							(RefFrameSignBias&(1 << 3)) ? 1 : 0, (RefFrameSignBias&(1 << 4)) ? 1 : 0, (RefFrameSignBias&(1 << 5)) ? 1 : 0, (RefFrameSignBias&(1 << 6)) ? 1 : 0);
					}

					if (ptr_sequence_header_obu->reduced_still_picture_header || disable_cdf_update)
					{
						NAV_WRITE_TAG_WITH_NUMBER_VALUE1(disable_frame_end_update_cdf, "Should be 1, indicates that the end of frame CDF update is disabled");
					}
					else
					{
						BST_FIELD_PROP_BOOL(disable_frame_end_update_cdf, 
							"indicates that the end of frame CDF update is disabled", 
							"indicates that the end of frame CDF update is enabled");
					}

					if (primary_ref_frame == PRIMARY_REF_NONE)
					{
						NAV_WRITE_TAG_WITH_ALIAS("init_non_coeff_cdfs", "init_non_coeff_cdfs()", 
							"a function call that indicates that the CDF tables which are not used in the coeff() syntax structure should be initialized");
						NAV_WRITE_TAG_WITH_ALIAS("setup_past_independence", "setup_past_independence()",
							"a function call that indicates that this frame can be decoded without dependence on previous coded frames");
					}
					else
					{
						NAV_WRITE_TAG_WITH_ALIAS_F("load_cdfs", "load_cdfs(frame context number: %d)",
							"a function call that indicates that the CDF tables are loaded from frame context number ctx", ref_frame_idx[primary_ref_frame]);
						NAV_WRITE_TAG_WITH_ALIAS("load_previous", "load_previous()",
							"a function call that indicates that information from a previous frame may be loaded for use in decoding the current frame");
					}

					if (use_ref_frame_mvs == 1)
					{
						NAV_WRITE_TAG_WITH_ALIAS("motion_field_estimation", "motion_field_estimation()",
							"a function call which indicates that the motion field estimation process should be invoked");
					}

					BST_FIELD_PROP_REF2_1(ptr_tile_info, "tile_info", "");
					BST_FIELD_PROP_REF2_1(ptr_quantization_params, "quantization_params", "");
					BST_FIELD_PROP_REF2_1(ptr_segmentation_params, "segmentation_params", "");
					BST_FIELD_PROP_REF2_1(ptr_delta_q_params, "delta_q_params", "");
					BST_FIELD_PROP_REF2_1(ptr_delta_lf_params, "delta_if_params", "");

					if (primary_ref_frame == PRIMARY_REF_NONE)
					{
						NAV_WRITE_TAG_WITH_ALIAS("init_coeff_cdfs", "init_coeff_cdfs()",
							"a function call that indicates that the CDF tables used in the coeff() syntax structure should be initialized");
					}
					else
					{
						NAV_WRITE_TAG_WITH_ALIAS("load_previous_segment_ids", "load_previous_segment_ids()",
							"a function call that indicates that a segment map from a previous frame may be loaded for use in decoding the current frame");
					}

					if (!ptr_quantization_params->using_qmatrix)
					{
						NAV_WRITE_TAG_WITH_VALUEFMTSTR("LosslessArray", "%d,%d,%d,%d,%d,%d,%d,%d", "", 
							LosslessArray[0], LosslessArray[1], LosslessArray[2], LosslessArray[3],
							LosslessArray[4], LosslessArray[5], LosslessArray[6], LosslessArray[7]);
					}
					else
					{
						NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag5", "for(segmentId=0;segmentId&lt;MAX_SEGMENTS;segmentId++)", "");
						for (uint8_t segmentId = 0; segmentId < MAX_SEGMENTS; segmentId++)
						{
							NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("LosslessArray", "LosslessArray[seg#%d]", LosslessArray[segmentId], "", segmentId);
							if (ptr_quantization_params->using_qmatrix)
							{
								NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("SeqQMLevel", "SeqQMLevel[0][seg#%d]", SegQMLevel[0][segmentId], "", segmentId);
								NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("SeqQMLevel", "SeqQMLevel[1][seg#%d]", SegQMLevel[1][segmentId], "", segmentId);
								NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("SeqQMLevel", "SeqQMLevel[2][seg#%d]", SegQMLevel[2][segmentId], "", segmentId);
							}
						}
						NAV_WRITE_TAG_END("Tag5");
					}

					// This indicates that the frame is fully lossless at the coded resolution of FrameWidth by FrameHeight. 
					// In this case, the loop filter and CDEF filter are disabled
					NAV_WRITE_TAG_WITH_NUMBER_VALUE1(CodedLossless, CodedLossless
						?"all segments use lossless encoding"
						:"");

					BST_FIELD_PROP_REF2_1(ptr_loop_filter_params, "loop_filter_params", "");
					BST_FIELD_PROP_REF2_1(ptr_cdef_params, "cdef_params", "");
					BST_FIELD_PROP_REF2_1(ptr_lr_params, "lr_params", "");
					BST_FIELD_PROP_REF2_1(ptr_read_tx_mode, "read_tx_mode", "");
					BST_FIELD_PROP_REF2_1(ptr_frame_reference_mode, "frame_reference_mode", "");
					BST_FIELD_PROP_REF2_1(ptr_skip_mode_params, "skip_mode_params", "");

					if (FrameIsIntra || error_resilient_mode || !ptr_sequence_header_obu->enable_warped_motion)
					{
						NAV_WRITE_TAG_WITH_NUMBER_VALUE1(allow_warped_motion, "Should be 0, indicates that the syntax element motion_mode will not be present");
					}
					else
					{
						BST_FIELD_PROP_BOOL(allow_warped_motion, 
							"indicates that the syntax element motion_mode may be present", "indicates that the syntax element motion_mode will not be present");
					}

					BST_FIELD_PROP_BOOL(reduced_tx_set, 
						"specifies that the frame is restricted to a reduced subset of the full set of transform types", 
						"specifies that the frame is NOT restricted to a reduced subset of the full set of transform types");

					BST_FIELD_PROP_REF2_1(ptr_global_motion_params, "global_motion_params", "");
					BST_FIELD_PROP_REF2_1(ptr_film_grain_params, "film_grain_params", "");

				done:
					DECLARE_FIELDPROP_END()

				};

				static UNCOMPRESSED_HEADER*
								ptr_last_uncompressed_header;

				bool			frame_header_copied;
				UNCOMPRESSED_HEADER*
								ptr_uncompressed_header = nullptr;
				OPEN_BITSTREAM_UNIT*
								ptr_OBU = nullptr;

				FRAME_HEADER_OBU(OPEN_BITSTREAM_UNIT* pOBU): frame_header_copied(false), ptr_OBU(pOBU){}

				~FRAME_HEADER_OBU() {
					if (frame_header_copied)
						ptr_uncompressed_header = nullptr;
					else
					{
						UNMAP_STRUCT_POINTER5(ptr_uncompressed_header);
					}
				}

				int Map(AMBst in_bst)
				{
					SYNTAX_BITSTREAM_MAP::Map(in_bst);
					try
					{
						MAP_BST_BEGIN(0);
						if (ptr_OBU->ctx_video_bst->SeenFrameHeader)
						{
							// frame_header_copy()
							ptr_uncompressed_header = ptr_last_uncompressed_header;
							frame_header_copied = true;
						}
						else
						{
							ptr_OBU->ctx_video_bst->tu_fu_idx++;

							if (g_verbose_level > 99)
								printf("[AV1] Start processing temporal-unit#%" PRId64 "/frame-unit#%" PRId32 ".\n"
									, ptr_OBU->ctx_video_bst->tu_idx, ptr_OBU->ctx_video_bst->tu_fu_idx);

							ptr_OBU->ctx_video_bst->SeenFrameHeader = true;

							if (ptr_OBU->ctx_video_bst->cfbi >= 0)
							{
								auto cur_frame = ptr_OBU->ctx_video_bst->buffer_pool->frame_bufs[ptr_OBU->ctx_video_bst->cfbi];
								for (int i = LAST_FRAME; i <= ALTREF_FRAME; ++i) {
									cur_frame.global_motion[i] = default_warp_params;
								}
							}

							av1_read_ref(in_bst, ptr_uncompressed_header, UNCOMPRESSED_HEADER, this);
							if (ptr_uncompressed_header->show_existing_frame)
							{
								// decode_frame_wrapup()
								ptr_OBU->ctx_video_bst->SeenFrameHeader = false;
								ptr_OBU->ctx_video_bst->reference_frame_update();
							}
							else
							{
								ptr_OBU->ctx_video_bst->TileNum = 0;
								ptr_OBU->ctx_video_bst->SeenFrameHeader = true;
							}

							ptr_last_uncompressed_header = ptr_uncompressed_header;
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
				if (frame_header_copied)
				{
					NAV_WRITE_TAG_WITH_ALIAS("frame_header_copy", "frame_header_copy()", 
						"a function call that indicates that a copy of the previous frame_header_obu should be inserted at this point");
				}
				else
				{
					if (ptr_uncompressed_header != nullptr)
					{
						BST_FIELD_PROP_REF2_1(ptr_uncompressed_header, "uncompressed_header", "");
						if (ptr_uncompressed_header->show_existing_frame)
						{
							NAV_WRITE_TAG_WITH_ALIAS("decode_frame_wrapup", "decode_frame_wrapup()", "");
							NAV_WRITE_TAG_WITH_NUMBER_VALUE("SeenFrameHeader", 0, "SeenFrameHeader = 0");
						}
						else
						{
							NAV_WRITE_TAG_WITH_NUMBER_VALUE("TileNum", ptr_OBU->ctx_video_bst->TileNum, "a variable giving the index (zero-based) of the current tile");
							NAV_WRITE_TAG_WITH_NUMBER_VALUE("SeenFrameHeader", 1, "SeenFrameHeader = 1");
						}
					}
				}
				DECLARE_FIELDPROP_END()

			}PACKED;

			struct TILE_GROUP_OBU;
			struct FRAME_OBU : public SYNTAX_BITSTREAM_MAP
			{
				FRAME_HEADER_OBU*	ptr_frame_header_obu = nullptr;
				TILE_GROUP_OBU*		ptr_tile_group_obu = nullptr;

				OPEN_BITSTREAM_UNIT*
									ptr_OBU;

				FRAME_OBU(OPEN_BITSTREAM_UNIT* pOBU): ptr_OBU(pOBU){}

				~FRAME_OBU() {
					UNMAP_STRUCT_POINTER5(ptr_tile_group_obu);
					UNMAP_STRUCT_POINTER5(ptr_frame_header_obu);
				}

				int Map(AMBst in_bst);

				int Unmap(AMBst out_bst)
				{
					UNREFERENCED_PARAMETER(out_bst);
					return RET_CODE_ERROR_NOTIMPL;
				}

				DECLARE_FIELDPROP_BEGIN()
					BST_FIELD_PROP_REF2_1(ptr_frame_header_obu, "frame_header_obu", "");
					BST_FIELD_PROP_REF2_1(ptr_tile_group_obu, "tile_group_obu", "");
				DECLARE_FIELDPROP_END()

			}PACKED;

			struct TILE_GROUP_OBU : public SYNTAX_BITSTREAM_MAP
			{
				struct DECODE_TILE : public SYNTAX_BITSTREAM_MAP
				{



				}PACKED;

				uint8_t			tile_start_and_end_present_flag : 1;
				uint8_t			byte_align_0 : 7;

				uint32_t		tg_start = 0;
				uint32_t		tg_end = 0;


				bool			is_last_tg;
				int				tg_obu_sz;

				OPEN_BITSTREAM_UNIT*
								ptr_OBU;

				TILE_GROUP_OBU(OPEN_BITSTREAM_UNIT* pOBU, int sz)
					: tile_start_and_end_present_flag(0), byte_align_0(0), is_last_tg(false), tg_obu_sz(sz), ptr_OBU(pOBU){}

				int Map(AMBst in_bst)
				{
					SYNTAX_BITSTREAM_MAP::Map(in_bst);
					try
					{
						MAP_BST_BEGIN(0);

						auto ctx_video_bst = ptr_OBU->ctx_video_bst;

						const int NumTiles = ctx_video_bst->tile_cols*ctx_video_bst->tile_rows;
						int startBitPos = AMBst_Tell(in_bst);
						tile_start_and_end_present_flag = 0;

						if (NumTiles > 1) {
							bsrb1(in_bst, tile_start_and_end_present_flag, 1);
						}

						if (NumTiles == 1 || !tile_start_and_end_present_flag) {
							tg_start = 0;
							tg_end = NumTiles - 1;
						}
						else
						{
							int tileBits = ctx_video_bst->log2_tile_cols + ctx_video_bst->log2_tile_rows;
							bsrb1(in_bst, tg_start, tileBits);
							bsrb1(in_bst, tg_end, tileBits);
						}

						AMBst_Realign(in_bst);

						// TODO...

						is_last_tg = ((int64_t)tg_end == ((int64_t)NumTiles - 1)) ? true : false;
						if (is_last_tg)
						{
							// decode a frame completely
							ctx_video_bst->SeenFrameHeader = false;
							ptr_OBU->ctx_video_bst->reference_frame_update();
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
				auto ctx_video_bst = ptr_OBU->ctx_video_bst;
				const int NumTiles = ctx_video_bst->tile_cols*ctx_video_bst->tile_rows;
				NAV_WRITE_TAG_WITH_NUMBER_VALUE("NumTiles", NumTiles, "NumTiles = TileCols * TileRows");
				if (NumTiles > 1)
				{
					BST_FIELD_PROP_BOOL(tile_start_and_end_present_flag, "", "");
				}
				else
				{
					NAV_WRITE_TAG_WITH_NUMBER_VALUE1(tile_start_and_end_present_flag, "Should be 0");
				}

				if (NumTiles == 1 || !tile_start_and_end_present_flag)
				{
					NAV_WRITE_TAG_WITH_NUMBER_VALUE1(tg_start, "");
					NAV_WRITE_TAG_WITH_NUMBER_VALUE1(tg_end, "");
				}
				else
				{
					int tileBits = ctx_video_bst->log2_tile_cols + ctx_video_bst->log2_tile_rows;
					NAV_WRITE_TAG_WITH_NUMBER_VALUE1(tileBits, "");
					BST_FIELD_PROP_NUMBER1(tg_start, tileBits, "");
					BST_FIELD_PROP_NUMBER1(tg_end, tileBits, "");
				}

				if (is_last_tg)
				{
					NAV_WRITE_TAG_WITH_NUMBER_VALUE("SeenFrameHeader", 0, "SeenFrameHeader = 0");
				}

				DECLARE_FIELDPROP_END()
			}PACKED;

			OBU_HEADER			obu_header;
			uint32_t			obu_size = 0;
			uint8_t				obu_size_leb128_bytes = 0;
			SHA1HashVaue		obu_hash_value;

			union
			{
				void*					ptr_void = nullptr;
				SEQUENCE_HEADER_OBU*	ptr_sequence_header_obu;
				TEMPORAL_DELIMITER_OBU*	ptr_temporal_delimiter_obu;
				FRAME_HEADER_OBU*		ptr_frame_header_obu;
				TILE_GROUP_OBU*			ptr_tile_group_obu;
				METADATA_OBU*			ptr_metadata_obu;
				FRAME_OBU*				ptr_frame_obu;
				TILE_LIST_OBU*			ptr_tile_list_obu;
				PADDING_OBU*			ptr_padding_obu;
				RESERVED_OBU*			ptr_reserved_obu;
			};

			TRAILING_BITS*		ptr_trailing_bits = nullptr;

			VideoBitstreamCtx*	ctx_video_bst;

			uint32_t			m_full_obu_size;
			uint16_t			m_OperatingPointIdc = 0;

			OPEN_BITSTREAM_UNIT(VideoBitstreamCtx* ctxVideoBst, uint32_t sz=0) 
				: ctx_video_bst(ctxVideoBst)
				, m_full_obu_size(sz){
			}

			~OPEN_BITSTREAM_UNIT()
			{
				switch (obu_header.obu_type)
				{
				case OBU_SEQUENCE_HEADER:
					UNMAP_STRUCT_POINTER5(ptr_sequence_header_obu);
					break;
				case OBU_TEMPORAL_DELIMITER:
					UNMAP_STRUCT_POINTER5(ptr_temporal_delimiter_obu);
					break;
				case OBU_FRAME_HEADER:
					UNMAP_STRUCT_POINTER5(ptr_frame_header_obu);
					break;
				case OBU_REDUNDANT_FRAME_HEADER:
					UNMAP_STRUCT_POINTER5(ptr_frame_header_obu);
					break;
				case OBU_TILE_GROUP:
					UNMAP_STRUCT_POINTER5(ptr_tile_group_obu);
					break;
				case OBU_METADATA:
					UNMAP_STRUCT_POINTER5(ptr_metadata_obu);
					break;
				case OBU_FRAME:
					UNMAP_STRUCT_POINTER5(ptr_frame_obu);
					break;
				case OBU_TILE_LIST:
					UNMAP_STRUCT_POINTER5(ptr_tile_list_obu);
					break;
				case OBU_PADDING:
					UNMAP_STRUCT_POINTER5(ptr_padding_obu);
					break;
				default:
					UNMAP_STRUCT_POINTER5(ptr_reserved_obu);
				}

				UNMAP_STRUCT_POINTER5(ptr_trailing_bits);
			}

			int Map(AMBst in_bst)
			{
				SYNTAX_BITSTREAM_MAP::Map(in_bst);
				try
				{
					MAP_BST_BEGIN(0);
					av1_read_obj(in_bst, obu_header);

					if (obu_header.obu_has_size_field)
					{
						av1_read_leb128_1(in_bst, obu_size, obu_size_leb128_bytes);
						if (obu_size >= UINT32_MAX)
							return RET_CODE_BUFFER_NOT_COMPATIBLE;

						m_full_obu_size = obu_size + obu_size_leb128_bytes + 1 + (obu_header.obu_extension_flag ? 1 : 0);
					}
					else if (m_full_obu_size > 0)
					{
						obu_size = m_full_obu_size - 1 - obu_header.obu_extension_flag;
					}
					else
						obu_size = -1;

					int startPosition = AMBst_Tell(in_bst);
					if (obu_header.obu_type != OBU_SEQUENCE_HEADER &&
						obu_header.obu_type != OBU_TEMPORAL_DELIMITER &&
						m_OperatingPointIdc != 0 &&
						obu_header.obu_extension_flag == 1)
					{
						uint16_t inTemporalLayer = (m_OperatingPointIdc >> obu_header.obu_extension_header.temporal_id) & 1;
						uint16_t inSpatialLayer = (m_OperatingPointIdc >> (obu_header.obu_extension_header.spatial_id + 8)) & 1;
						if (!inTemporalLayer || !inSpatialLayer) {
							if (obu_size > 0)
								AMBst_SkipBits(in_bst, (int)obu_size * 8);

							MAP_BST_END();
							return RET_CODE_SUCCESS;
						}
					}

					switch (obu_header.obu_type)
					{
					case OBU_SEQUENCE_HEADER:
						av1_read_ref(in_bst, ptr_sequence_header_obu, SEQUENCE_HEADER_OBU, this);
						break;
					case OBU_TEMPORAL_DELIMITER:
						av1_read_ref(in_bst, ptr_temporal_delimiter_obu, TEMPORAL_DELIMITER_OBU, this);
						break;
					case OBU_FRAME_HEADER:
						// If obu_type is equal to OBU_FRAME_HEADER, it is a requirement of bitstream conformance that SeenFrameHeader is equal to 0.
						assert(!ctx_video_bst->SeenFrameHeader);
						av1_read_ref(in_bst, ptr_frame_header_obu, FRAME_HEADER_OBU, this);
						break;
					case OBU_REDUNDANT_FRAME_HEADER:
						// If obu_type is equal to OBU_REDUNDANT_FRAME_HEADER, it is a requirement of bitstream conformance that SeenFrameHeader is equal to 1.
						assert(ctx_video_bst->SeenFrameHeader);
						av1_read_ref(in_bst, ptr_frame_header_obu, FRAME_HEADER_OBU, this);
						break;
					case OBU_TILE_GROUP:
						av1_read_ref(in_bst, ptr_tile_group_obu, TILE_GROUP_OBU, this, obu_size);
						break;
					case OBU_METADATA:
						av1_read_ref(in_bst, ptr_metadata_obu, METADATA_OBU, this);
						break;
					case OBU_FRAME:
						av1_read_ref(in_bst, ptr_frame_obu, FRAME_OBU, this);
						break;
					case OBU_TILE_LIST:
						av1_read_ref(in_bst, ptr_tile_list_obu, TILE_LIST_OBU);
						break;
					case OBU_PADDING:
						if (obu_size >= 0 && obu_size <= UINT32_MAX)
						{
							av1_read_ref(in_bst, ptr_padding_obu, PADDING_OBU, (size_t)obu_size);
						}
						break;
					default:
						{
						av1_read_ref(in_bst, ptr_reserved_obu, RESERVED_OBU, this);
						}
					}

					int currentPosition = AMBst_Tell(in_bst);
					int payloadBits = currentPosition - startPosition;
					assert(payloadBits >= 0);
					if (obu_size > 0 && obu_header.obu_type != OBU_TILE_GROUP && obu_header.obu_type != OBU_FRAME)
					{
						av1_read_ref(in_bst, ptr_trailing_bits, TRAILING_BITS, (int)(obu_size * 8 - payloadBits));
					}
					else if (obu_size * 8 > (uint32_t)payloadBits)
					{
						AMBst_SkipBits(in_bst, (int)(obu_size * 8 - payloadBits));
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
			long long orig_bit_pos = 0;
			if (bit_offset)
				orig_bit_pos = *bit_offset;
			NAV_WRITE_TAG_BEGIN_WITH_ALIAS("obu_header", "obu_header()", "");
			BST_FIELD_PROP_OBJECT(obu_header);
			NAV_WRITE_TAG_END("obu_header");

			if (obu_header.obu_has_size_field)
			{
				BST_FIELD_PROP_2NUMBER1(obu_size, (long long)obu_size_leb128_bytes<<3, 
					"contains the size in bytes of the OBU not including the bytes within obu_header or the obu_size syntax element");
			}
			else
			{
				NAV_WRITE_TAG_WITH_NUMBER_VALUE("obu_size", (int)obu_size, 
					"contains the size in bytes of the OBU not including the bytes within obu_header or the obu_size syntax element");
			}

			if (obu_header.obu_type != OBU_SEQUENCE_HEADER &&
				obu_header.obu_type != OBU_TEMPORAL_DELIMITER &&
				m_OperatingPointIdc != 0 &&
				obu_header.obu_extension_flag == 1)
			{
				uint16_t inTemporalLayer = (m_OperatingPointIdc >> obu_header.obu_extension_header.temporal_id) & 1;
				uint16_t inSpatialLayer = (m_OperatingPointIdc >> (obu_header.obu_extension_header.spatial_id + 8)) & 1;

				NAV_WRITE_TAG_WITH_NUMBER_VALUE("inTemporalLayer", inTemporalLayer, "");
				NAV_WRITE_TAG_WITH_NUMBER_VALUE("inSpatialLayer", inSpatialLayer, "");

				if (!inTemporalLayer || !inSpatialLayer) {
					NAV_WRITE_TAG_WITH_ALIAS("drop_obu", "drop_obu()", "");
					NAV_WRITE_TAG_WITH_ALIAS("return", "return", "");
					goto done;
				}
			}

			switch (obu_header.obu_type)
			{
			case OBU_SEQUENCE_HEADER:
				BST_FIELD_PROP_REF2_1(ptr_sequence_header_obu, "sequence_header_obu", "");
				break;
			case OBU_TEMPORAL_DELIMITER:
				BST_FIELD_PROP_REF2_1(ptr_temporal_delimiter_obu, "temporal_delimiter_obu", "");
				break;
			case OBU_FRAME_HEADER:
				BST_FIELD_PROP_REF2_1(ptr_frame_header_obu, "frame_header_obu", "");
				break;
			case OBU_REDUNDANT_FRAME_HEADER:
				BST_FIELD_PROP_REF2_1(ptr_frame_header_obu, "frame_header_obu", "");
				break;
			case OBU_TILE_GROUP:
				BST_FIELD_PROP_REF2_1(ptr_tile_group_obu, "tile_group", "");
				break;
			case OBU_METADATA:
				BST_FIELD_PROP_REF2_1(ptr_metadata_obu, "metadata_obu", "");
				break;
			case OBU_FRAME:
				BST_FIELD_PROP_REF2_1(ptr_frame_obu, "frame_obu", "");
				break;
			case OBU_TILE_LIST:
				BST_FIELD_PROP_REF2_1(ptr_tile_list_obu, "tile_list_obu", "");
				break;
			case OBU_PADDING:
				BST_FIELD_PROP_REF2_1(ptr_padding_obu, "padding_obu", "");
				break;
			default:
				BST_FIELD_PROP_REF2_1(ptr_reserved_obu, "reserved_obu", "");
			}

			if (ptr_trailing_bits != nullptr)
			{
				if (bit_offset)
					*bit_offset = orig_bit_pos + ptr_trailing_bits->bit_pos;
				BST_FIELD_PROP_REF2_1(ptr_trailing_bits, "trailing_bits", "");
			}

		done:
			DECLARE_FIELDPROP_END()

		};

		struct TEMPORAL_UNIT : public SYNTAX_BITSTREAM_MAP
		{
			std::vector<std::shared_ptr<OPEN_BITSTREAM_UNIT>> open_bitstream_units;

			VideoBitstreamCtx& ctx_video_bst;

			TEMPORAL_UNIT(VideoBitstreamCtx& ctxVideoBst) : ctx_video_bst(ctxVideoBst) {
			}

			int Map(unsigned char* pBuf, unsigned long cbSize)
			{
				int iRet = RET_CODE_SUCCESS;
				unsigned long cbParsed = 0;
				uint32_t temporal_unit_size = 0;
				uint8_t cbLeb128 = 0;

				if (cbSize >= INT64_MAX)
					return RET_CODE_BUFFER_OVERFLOW;

				AMBst bst = AMBst_CreateFromBuffer(pBuf, (int)cbSize);

				printf("[AV1][%08" PRId64 "] Begin decoding one AV1 temporal unit.\n", ctx_video_bst.tu_idx + 1);

				if (ctx_video_bst.AnnexB)
				{
					temporal_unit_size = leb128(pBuf + cbParsed, cbSize - cbParsed, &cbLeb128);
					if (temporal_unit_size == UINT32_MAX)
						return RET_CODE_BUFFER_NOT_COMPATIBLE;

					cbParsed += cbLeb128;
				}

				while (cbParsed < cbSize)
				{
					uint32_t cbFrameParsed = 0;
					uint32_t frame_unit_size = UINT32_MAX;

					// enumerate each OBU in the current buffer
					if (ctx_video_bst.AnnexB)
					{
						frame_unit_size = leb128(pBuf + cbParsed, cbSize - cbParsed, &cbLeb128);
						if (frame_unit_size == UINT32_MAX)
						{
							iRet = RET_CODE_BUFFER_NOT_COMPATIBLE;
							break;
						}

						cbParsed += cbLeb128;

						while (cbFrameParsed < frame_unit_size)
						{
							uint32_t obu_length = leb128(pBuf + cbParsed, cbSize - cbParsed, &cbLeb128);
							if (obu_length == UINT32_MAX)
							{
								iRet = RET_CODE_BUFFER_NOT_COMPATIBLE;
								break;
							}

							cbFrameParsed += cbLeb128;
							cbParsed += cbLeb128;

							AMP_NEWT1(ptr_obu, OPEN_BITSTREAM_UNIT, &ctx_video_bst, obu_length);
							std::shared_ptr<OPEN_BITSTREAM_UNIT> sp_obu(ptr_obu);

							// in order to keep the bit position information correctly
							AMBst_Seek(bst, cbParsed << 3);

							if (AMP_FAILED(iRet = sp_obu->Map(bst)))
								printf("[AV1] Failed to map Annex-B byte stream AV1 OBU {retcode: %d}.\n", iRet);

							open_bitstream_units.push_back(sp_obu);

							cbFrameParsed += obu_length;
							cbParsed += obu_length;
						}

						if (AMP_FAILED(iRet))
							break;
					}
					else
					{
						do
						{
							AMP_NEWT1(ptr_obu, OPEN_BITSTREAM_UNIT, &ctx_video_bst);
							std::shared_ptr<OPEN_BITSTREAM_UNIT> sp_obu(ptr_obu);

							int nStartPos = AMBst_Tell(bst);
							if (AMP_FAILED(iRet = sp_obu->Map(bst)))
							{
								printf("[AV1] Failed to map byte stream AV1 OBU {retcode: %d}.\n", iRet);
								iRet = RET_CODE_BUFFER_NOT_COMPATIBLE;
								break;
							}

							open_bitstream_units.push_back(sp_obu);

							uint32_t full_obu_size = 0;
							if (sp_obu->obu_header.obu_has_size_field)
								full_obu_size = 1 + (sp_obu->obu_header.obu_extension_flag ? 1 : 0) + sp_obu->obu_size + sp_obu->obu_size_leb128_bytes;
							else
								full_obu_size = AMBst_Tell(bst) - nStartPos;

							cbParsed += full_obu_size;
							cbFrameParsed += full_obu_size;

							if (AMP_SUCCEEDED(iRet) && IS_OBU_FRAME(sp_obu->obu_header.obu_type) && !ctx_video_bst.SeenFrameHeader)
							{
								cbFrameParsed = 0;
							}
						} while (cbFrameParsed > 0 && cbParsed < cbSize);
					}
				}

				AMBst_Destroy(bst);

				ctx_video_bst.tu_idx++;

				return iRet;
			}

			// The bitstream should be a whole temporal_unit, and don't include the data across more than one temoral_unit
			int Map(AMBst in_bst)
			{
				return -1;
			}

			int Unmap(AMBst out_bst)
			{
				return RET_CODE_ERROR_NOTIMPL;
			}

			DECLARE_FIELDPROP_BEGIN()
			long long start_bit_offset = bit_offset ? *bit_offset : 0LL;
			for (size_t idx = 0; idx<open_bitstream_units.size(); idx++)
			{
				if (bit_offset != NULL)
					*bit_offset = start_bit_offset + open_bitstream_units[idx]->bit_pos;
				NAV_FIELD_PROP_REF2_3(open_bitstream_units[idx], "open_bitstream_unit", obu_type_names[open_bitstream_units[idx]->obu_header.obu_type], 1);
			}
			DECLARE_FIELDPROP_END()

		};
	}	// AV1
}	// BST

#ifdef _WIN32
#pragma pack(pop)
#pragma warning(pop)
#endif
#undef PACKED

#endif
