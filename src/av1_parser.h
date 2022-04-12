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
#ifndef __AV1_VIDEO_PARSER_H__
#define __AV1_VIDEO_PARSER_H__

#include "dump_data_type.h"
#include "AMRingBuffer.h"
#include "AMSHA1.h"

#include <assert.h>
#include <memory.h>
#include <time.h>
#include <sys/timeb.h>
#include "combase.h"
#include "AMArray.h"
#include "AMBitStream.h"
#include "DumpUtil.h"
#include "MSE.h"

enum AV1_BYTESTREAM_FORMAT
{
	AV1_BYTESTREAM_RAW = 0,				// (OBU)+, low-overhead bit-stream
	AV1_BYTESTREAM_LENGTH_DELIMITED,	// ([Temporal Unit [Frame Unit [OBU]+]+]+)
};

namespace BST {
	namespace AV1 {
		struct OPEN_BITSTREAM_UNIT;
		struct OBU_HEADER;
	}
}

/*
                               ___ Player Ref Count[1]/Decoder Ref Count[1]/PhysicalBuffer[1]
                              /
    Frame Buffers[10] [f0] [f1] [f2] [f3] [f4] [f5] [f6] [f7]
                       \__/_____/____/____/____/__|________
                         /     /    /    /    /   |        \
    VBI[8]            [1 ] [2 ] [3 ] [4 ] [5 ] [5 ] [-1] [0 ]
                       #0   #1   #2   #3   #4   #5   #6   #7
                       /
                      /__RefValid[0]/RefFrameId[0]/RefUpscaledWidth[0]/RefFrameWidth[0]
                         RefFrameHeight[0]/RefRenderWidth[0]/RefRenderHeight[0]/RefMiCols[0]
                         RefMiRows[0]/RefFrameType[0]/RefSubsamplingX[0]/RefSubsamplingY[0]
                         RefBitDepth[0]/RefOrderHint[0]
	ref_frame_idx: it is used to describe which VBI element frame is referred to by the current inter-frame
	refresh_frame_flags: it is used to describe the current frame is referred by which VBI frame
*/
struct HRD_FRAME_BUF	// The physical buffer in HRD concept
{
	uint32_t				Player_Ref_Count = 0;
	uint32_t				Decoder_Ref_Count = 0;
};

//
// Virtual Buffer Slot params which can be used for restoring the right context to do syntax processing 
//
struct VBI_SLOT_PARAMS
{
	// If 2 VBI Slot parameters has the same frame sequence id, it means that they pointed to the same frame buffer
	int32_t					FrameSeqID			= -1;
	//
	// The VBI slot additional information
	//
	uint8_t					RefValid:1;					// 0: invalid; 1: valid
	uint8_t					RefSubsamplingX : 1;
	uint8_t					RefSubsamplingY : 1;
	uint8_t					RefBitDepth : 5;

	int8_t					RefFrameType		= -1;

	int32_t					RefFrameId			= -1;	// Maximum 25 bit
	int32_t					RefUpscaledWidth	= -1;
	int32_t					RefFrameWidth		= -1;
	int32_t					RefFrameHeight		= -1;
	int32_t					RefRenderWidth		= -1;
	int32_t					RefRenderHeight		= -1;
	uint16_t				RefMiCols			=  0;
	uint16_t				RefMiRows			=  0;
	uint8_t					SavedOrderHints[REFS_PER_FRAME] = { 0 };
	uint8_t					RefOrderHint		=  0;
	uint8_t					RefFrameSignBias	=  0;		// bit0: LAST_FRAME,...bit6: 
	int32_t					SavedGmParams[REFS_PER_FRAME][6] = { {0} };
	int8_t					loop_filter_ref_deltas[TOTAL_REFS_PER_FRAME];
	int8_t					loop_filter_mode_deltas[2];
	uint8_t					FeatureEnabled[MAX_SEGMENTS][SEG_LVL_MAX] = { {0} };
	int16_t					FeatureData[MAX_SEGMENTS][SEG_LVL_MAX] = { {0} };

	VBI_SLOT_PARAMS() : RefValid(0), RefSubsamplingX(0), RefSubsamplingY(0), RefBitDepth(0) {}
};

//
// The active frame parameters defined in AV1 specification, which may not be complete.
// Most of them are from the uncompressed header or inherit from the previous reference frames
// Finally all information across frame will be updated into VBISlotParams after a frame is decoded
struct ACTIVE_FRAME_PARAMS
{
	int32_t					FrameSeqID = -1;

	uint8_t					SeenFrameHeader : 1;
	uint8_t					show_existing_frame : 1;
	uint8_t					frame_to_show_map_idx : 3;
	uint8_t					show_frame : 1;
	uint8_t					disable_cdf_update : 1;
	uint8_t					allow_screen_content_tools : 1;

	uint8_t					force_integer_mv : 1;
	uint8_t					allow_high_precision_mv : 1;
	uint8_t					use_ref_frame_mvs : 1;
	uint8_t					allow_intrabc : 1;
	uint8_t					is_motion_mode_switchable : 1;
	uint8_t					disable_frame_end_update_cdf : 1;
	uint8_t					allow_warped_motion : 1;
	uint8_t					reduced_tx_set : 1;
	
	int8_t					frame_type;
	uint8_t					OrderHint;

	uint8_t					reserved_for_future_use_0 : 1;
	uint8_t					subsampling_x : 1;
	uint8_t					subsampling_y : 1;
	uint8_t					BitDepth : 5;

	uint8_t					OrderHints[REFS_PER_FRAME] = { 0 };

	int32_t					PrevFrameID;
	int32_t					current_frame_id;

	uint8_t					refresh_frame_flags = 0;
	int8_t					ref_frame_idx[REFS_PER_FRAME] = { -1, -1, -1, -1, -1, -1, -1 };

	int32_t					UpscaledWidth = -1;
	int32_t					FrameWidth = -1;
	int32_t					FrameHeight = -1;
	int32_t					RenderWidth = -1;
	int32_t					RenderHeight = -1;
	uint16_t				MiCols = 0;
	uint16_t				MiRows = 0;

	int32_t					PrevGmParams[REFS_PER_FRAME][6] = { {0} };
	int32_t					gm_params[REFS_PER_FRAME][6] = { {0} };
	bool					loop_filter_delta_enabled = false;
	int8_t					loop_filter_ref_deltas[TOTAL_REFS_PER_FRAME];
	int8_t					loop_filter_mode_deltas[2];
	uint8_t					FeatureEnabled[MAX_SEGMENTS][SEG_LVL_MAX] = { {0} };
	int16_t					FeatureData[MAX_SEGMENTS][SEG_LVL_MAX] = { {0} };
	uint8_t					GmType[REFS_PER_FRAME] = { IDENTITY, IDENTITY, IDENTITY, IDENTITY, IDENTITY, IDENTITY, IDENTITY };

	uint8_t					TileCols = 0;		// The maximum value is 64(MAX_TILE_COLS)
	uint8_t					TileRows = 0;		// The maximum value is 64(MAX_TILE_ROWS)
	uint16_t				TileNum = 0;
	uint8_t					TileColsLog2;
	uint8_t					TileRowsLog2;
};

struct AV1ContextSnapshot
{
	int64_t					tu_idx;
	int32_t					tu_fu_idx;

	VBI_SLOT_PARAMS*		pVBISlotParams;

	ACTIVE_FRAME_PARAMS*	pActiveFrameParams;
};

struct OBU_PARSE_PARAMS
{
	ACTIVE_FRAME_PARAMS		ActiveFrameParams;

	VBI_SLOT_PARAMS			VBI[NUM_REF_FRAMES];
	HRD_FRAME_BUF			BufferPool[BUFFER_POOL_MAX_SIZE];


	// This process is invoked as the final step in decoding a frame
	RET_CODE UpdateRefreshFrame(uint16_t frame_seq_id)
	{
		if (ActiveFrameParams.refresh_frame_flags == 0)
			return RET_CODE_NOTHING_TODO;

		// Follow spec "7.20. Reference frame update process"
		for (uint8_t i = 0; i < NUM_REF_FRAMES; i++)
		{
			if ((ActiveFrameParams.refresh_frame_flags >> i) & 0x1) {
				VBI[i].FrameSeqID = frame_seq_id;
				VBI[i].RefValid = 1;
				VBI[i].RefSubsamplingX = ActiveFrameParams.subsampling_x;
				VBI[i].RefSubsamplingY = ActiveFrameParams.subsampling_y;
				VBI[i].RefBitDepth = ActiveFrameParams.BitDepth;

				VBI[i].RefFrameType = ActiveFrameParams.frame_type;

				memcpy(VBI[i].SavedOrderHints, ActiveFrameParams.OrderHints, sizeof(VBI[i].SavedOrderHints));

				VBI[i].RefFrameId = ActiveFrameParams.current_frame_id;

				VBI[i].RefUpscaledWidth = ActiveFrameParams.UpscaledWidth;
				VBI[i].RefFrameWidth = ActiveFrameParams.FrameWidth;
				VBI[i].RefFrameHeight = ActiveFrameParams.FrameHeight;
				VBI[i].RefRenderWidth = ActiveFrameParams.RenderWidth;
				VBI[i].RefRenderHeight = ActiveFrameParams.RenderHeight;

				VBI[i].RefMiCols = ActiveFrameParams.MiCols;
				VBI[i].RefMiRows = ActiveFrameParams.MiRows;

				VBI[i].RefOrderHint = ActiveFrameParams.OrderHint;
				memcpy(VBI[i].SavedGmParams, ActiveFrameParams.gm_params, sizeof(VBI[i].SavedGmParams));
				memcpy(VBI[i].loop_filter_ref_deltas, ActiveFrameParams.loop_filter_ref_deltas, sizeof(VBI[i].loop_filter_ref_deltas));
				memcpy(VBI[i].loop_filter_mode_deltas, ActiveFrameParams.loop_filter_mode_deltas, sizeof(VBI[i].loop_filter_mode_deltas));
				memcpy(VBI[i].FeatureEnabled, ActiveFrameParams.FeatureEnabled, sizeof(VBI[i].FeatureEnabled));
				memcpy(VBI[i].FeatureData, ActiveFrameParams.FeatureData, sizeof(VBI[i].FeatureData));
			}
		}

		if (g_verbose_level > 200)
		{
			printf("[AV1Parser] VBI with FrameSeqID: [%d,%d,%d,%d,%d,%d,%d,%d]\n",
				VBI[0].FrameSeqID, VBI[1].FrameSeqID, VBI[2].FrameSeqID, VBI[3].FrameSeqID,
				VBI[4].FrameSeqID, VBI[5].FrameSeqID, VBI[6].FrameSeqID, VBI[7].FrameSeqID);
		}

		return RET_CODE_SUCCESS;
	}

	void setup_past_independence(){
		for (uint8_t i = 0; i < MAX_SEGMENTS; i++) {
			for (uint8_t j = 0; j < SEG_LVL_MAX; j++) {
				ActiveFrameParams.FeatureData[i][j] = 0;
				ActiveFrameParams.FeatureEnabled[i][j] = 0;
			}
		}

		for (uint8_t ref = LAST_FRAME; ref <= ALTREF_FRAME; ref++) {
			ActiveFrameParams.GmType[ref - LAST_FRAME] = IDENTITY;
			for (uint8_t i = 0; i < 6; i++) {
				ActiveFrameParams.PrevGmParams[ref - LAST_FRAME][i] = (i % 3 == 2) ? 1 << WARPEDMODEL_PREC_BITS : 0;
			}
		}

		ActiveFrameParams.loop_filter_delta_enabled = true;
		ActiveFrameParams.loop_filter_ref_deltas[INTRA_FRAME] = 1;
		ActiveFrameParams.loop_filter_ref_deltas[LAST_FRAME] = 0;
		ActiveFrameParams.loop_filter_ref_deltas[LAST2_FRAME] = 0;
		ActiveFrameParams.loop_filter_ref_deltas[LAST3_FRAME] = 0;
		ActiveFrameParams.loop_filter_ref_deltas[BWDREF_FRAME] = 0;
		ActiveFrameParams.loop_filter_ref_deltas[GOLDEN_FRAME] = -1;
		ActiveFrameParams.loop_filter_ref_deltas[ALTREF_FRAME] = -1;
		ActiveFrameParams.loop_filter_ref_deltas[ALTREF2_FRAME] = -1;

		ActiveFrameParams.loop_filter_mode_deltas[0] = ActiveFrameParams.loop_filter_mode_deltas[1] = 0;
	}

	void load_previous(int8_t prevFrame)
	{
		memcpy(ActiveFrameParams.PrevGmParams, VBI[prevFrame].SavedGmParams, sizeof(ActiveFrameParams.PrevGmParams));
		memcpy(ActiveFrameParams.FeatureData, VBI[prevFrame].FeatureData, sizeof(ActiveFrameParams.FeatureData));
		memcpy(ActiveFrameParams.FeatureEnabled, VBI[prevFrame].FeatureEnabled, sizeof(ActiveFrameParams.FeatureEnabled));
	}
};

struct OBU_FILTER
{
	uint8_t					obu_type : 4;
	uint8_t					obu_extension_flag : 1;
	uint8_t					temporal_id : 3;
	uint8_t					spatial_id : 3;
	uint8_t					reserved_0 : 5;

	OBU_FILTER(uint8_t obuType)
		: obu_type(obuType), obu_extension_flag(0), temporal_id(0), spatial_id(0), reserved_0(0) {
	}

	OBU_FILTER(uint8_t obuType, uint8_t temporalID, uint8_t spatialID)
		: obu_type(obuType), obu_extension_flag(1), temporal_id(temporalID), spatial_id(spatialID), reserved_0(0) {
	}
};

using AV1_OBU = std::shared_ptr<BST::AV1::OPEN_BITSTREAM_UNIT>;

class IAV1Context : public IUnknown
{
public:
	virtual RET_CODE		SetOBUFilters(std::initializer_list<OBU_FILTER> obu_filters) = 0;
	virtual RET_CODE		GetOBUFilters(std::vector<OBU_FILTER>& obu_filters) = 0;
	virtual bool			IsOBUFiltered(OBU_FILTER obu_filter) = 0;
	virtual AV1_OBU			GetSeqHdrOBU() = 0;
	virtual RET_CODE		UpdateSeqHdrOBU(AV1_OBU seq_hdr_obu) = 0;
	virtual AV1_BYTESTREAM_FORMAT
							GetByteStreamFormat() = 0;
	virtual RET_CODE		LoadSnapshot(AV1ContextSnapshot* ptr_ctx_snapshot) = 0;
	virtual void			Reset() = 0;

public:
	IAV1Context() {}
	virtual ~IAV1Context() {}
};

class CAV1Parser : public CComUnknown, public IMSEParser
{
public:
	CAV1Parser(bool bAnnexB, bool bSingleOBUParse, bool bIVF, RET_CODE* pRetCode);
	virtual ~CAV1Parser();

	DECLARE_IUNKNOWN
	HRESULT NonDelegatingQueryInterface(REFIID uuid, void** ppvObj)
	{
		if (ppvObj == NULL)
			return E_POINTER;

		if (uuid == IID_IMSEParser)
			return GetCOMInterface((IMSEParser*)this, ppvObj);

		return CComUnknown::NonDelegatingQueryInterface(uuid, ppvObj);
	}

public:
	MEDIA_SCHEME_TYPE		GetSchemeType() { return MEDIA_SCHEME_AV1; }
	RET_CODE				SetEnumerator(IUnknown* pEnumerator, uint32_t options);
	RET_CODE				ProcessInput(uint8_t* pInput, size_t cbInput);
	RET_CODE				ProcessOutput(bool bDrain = false);
	RET_CODE				ParseFrameBuf(uint8_t* pAUBuf, size_t cbAUBuf);
	RET_CODE				GetContext(IUnknown** ppCtx);
	RET_CODE				Reset();

protected:
	RET_CODE				ProcessLengthDelimitedBitstreamOutput(bool bDrain);
	RET_CODE				ProcessLowOverheadBitstreamOutput(bool bDrain);
	RET_CODE				PushTemporalUnitBytes(uint8_t* pStart, uint8_t* pEnd);
	RET_CODE				SubmitAnnexBTU();
	RET_CODE				SubmitTU();
	RET_CODE				UpdateSeqHdrToContext(const uint8_t* pOBUBuf, uint32_t cbOBUBuf);
	RET_CODE				UpdateOBUParsePreconditionParams(const uint8_t* pOBUBodyBuf, size_t cbOBUBodyBuf, BST::AV1::OBU_HEADER& obu_hdr);
	RET_CODE				ParseUncompressedHeader(BITBUF& bitbuf, BST::AV1::OBU_HEADER& obu_hdr, AV1_OBU spSeqHdrOBU);

	//
	// The methods defined in the AV1 specification
	//
protected:
	RET_CODE				read_delta_q(BITBUF& bitbuf, int8_t& delta_q);
	RET_CODE				mark_ref_frames(AV1_OBU spSeqHdrOBU, uint8_t idLen, OBU_PARSE_PARAMS& obu_parse_params);
	RET_CODE				frame_size(AV1_OBU spSeqHdrOBU, BITBUF& bitbuf, OBU_PARSE_PARAMS& obu_parse_params, bool frame_size_override_flag);
	RET_CODE				frame_size_with_refs(AV1_OBU spSeqHdrOBU, BITBUF& bitbuf, OBU_PARSE_PARAMS& obu_parse_params, bool frame_size_override_flag, int8_t ref_frame_idx[REFS_PER_FRAME]);
	RET_CODE				superres_params(AV1_OBU spSeqHdrOBU, BITBUF& bitbuf, OBU_PARSE_PARAMS& obu_parse_params);
	RET_CODE				compute_image_size(AV1_OBU spSeqHdrOBU, BITBUF& bitbuf, OBU_PARSE_PARAMS& obu_parse_params);
	RET_CODE				render_size(AV1_OBU spSeqHdrOBU, BITBUF& bitbuf, OBU_PARSE_PARAMS& obu_parse_params);
	RET_CODE				set_frame_refs(AV1_OBU spSeqHdrOBU, OBU_PARSE_PARAMS& obu_parse_params, 
										   int8_t last_frame_idx, int8_t gold_frame_idx, int8_t ref_frame_idx[REFS_PER_FRAME]);
	RET_CODE				tile_info(AV1_OBU spSeqHdrOBU, BITBUF& bitbuf, OBU_PARSE_PARAMS& obu_parse_params);
	RET_CODE				global_motion_params(AV1_OBU spSeqHdrOBU, BITBUF& bitbuf, OBU_PARSE_PARAMS& obu_parse_params, bool FrameIsIntra, bool allow_high_precision_mv);
	RET_CODE				film_grain_params(AV1_OBU spSeqHdrOBU, BITBUF& bitbuf, OBU_PARSE_PARAMS& obu_parse_params, bool show_frame, bool showable_frame);
	RET_CODE				read_global_param(BITBUF& bitbuf, OBU_PARSE_PARAMS& obu_parse_params, uint8_t type, uint8_t ref, uint8_t idx, bool allow_high_precision_mv);

protected:
	IAV1Context*			m_pCtx = nullptr;
	AV1_BYTESTREAM_FORMAT	m_av1_bytestream_format;
	bool					m_bIVF = false;

	const int				read_unit_size = 2048;
	AMLinearRingBuffer		m_rbRawBuf = nullptr;
	// It is used to store a whole temporal unit buffer
	AMLinearRingBuffer		m_rbTemporalUnit;

	IAV1Enumerator*			m_av1_enum = nullptr;
	uint32_t				m_av1_enum_options = 0;

	int						m_av1_obu_type = -1;
	uint32_t				m_av1_obu_size = UINT32_MAX;
	uint32_t				m_av1_obu_parsed_size = 0;
	uint32_t				m_temporal_unit_size = UINT32_MAX;
	uint32_t				m_temporal_unit_parsed_size = 0;
	/*
	-------------------------------> stream_size <--------------------------------------
	|________________________________________________________________________________|
					   ^          ^        ^
	  cur_submit_pos--/      cur_scan_pos   \----- cur_byte_pos
	*/
	uint64_t				m_stream_size = 0;			// The file size
	uint64_t				m_cur_byte_pos = 0;			// The read pointer of stream handle
	uint64_t				m_cur_scan_pos = 0;			// The corresponding file position of the start buffer of raw data ring buffer
	uint64_t				m_cur_submit_pos = 0;		// The corresponding file position of the start buffer of OBU unit EBSP

	int64_t					m_num_temporal_units = 0LL;

	OBU_PARSE_PARAMS		m_obu_parse_params;			// The TU intermediate parse parameters, and it will be updated after one TU has been processed completely
	uint16_t				m_next_frame_seq_id = 0;	// The next unique frame sequence id
};

//
// 7.20. Reference frame update process
//
INLINE int FillVBISlotParamsWithActiveFrameParams(const ACTIVE_FRAME_PARAMS* frame_params, VBI_SLOT_PARAMS& vbi_slot_params)
{
	if (frame_params == nullptr)
		return RET_CODE_INVALID_PARAMETER;

	vbi_slot_params.RefValid = 1;
	vbi_slot_params.RefSubsamplingX = frame_params->subsampling_x;
	vbi_slot_params.RefSubsamplingY = frame_params->subsampling_y;
	vbi_slot_params.RefBitDepth = frame_params->BitDepth;
	vbi_slot_params.RefFrameType = frame_params->frame_type;
	vbi_slot_params.RefFrameId = frame_params->current_frame_id;
	vbi_slot_params.RefUpscaledWidth = frame_params->UpscaledWidth;
	vbi_slot_params.RefFrameWidth = frame_params->FrameWidth;
	vbi_slot_params.RefFrameHeight = frame_params->FrameHeight;
	vbi_slot_params.RefRenderWidth = frame_params->RenderWidth;
	vbi_slot_params.RefRenderHeight = frame_params->RenderHeight;
	vbi_slot_params.RefMiCols = frame_params->MiCols;
	vbi_slot_params.RefMiRows = frame_params->MiRows;
	memcpy(vbi_slot_params.SavedOrderHints, frame_params->OrderHints, sizeof(vbi_slot_params.SavedOrderHints));
	vbi_slot_params.RefOrderHint = frame_params->OrderHint;
	memcpy(vbi_slot_params.SavedGmParams, frame_params->gm_params, sizeof(vbi_slot_params.SavedGmParams));
	memcpy(vbi_slot_params.loop_filter_ref_deltas, frame_params->loop_filter_ref_deltas, sizeof(vbi_slot_params.loop_filter_ref_deltas));
	memcpy(vbi_slot_params.loop_filter_mode_deltas, frame_params->loop_filter_mode_deltas, sizeof(vbi_slot_params.loop_filter_mode_deltas));
	memcpy(vbi_slot_params.FeatureEnabled, frame_params->FeatureEnabled, sizeof(vbi_slot_params.FeatureEnabled));
	memcpy(vbi_slot_params.FeatureData, frame_params->FeatureData, sizeof(vbi_slot_params.FeatureData));

	return RET_CODE_SUCCESS;
}

#endif
