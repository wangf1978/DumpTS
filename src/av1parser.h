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

enum AV1_ENUM_OPTION
{
	AV1_ENUM_OPTION_TU		= (1<<4),	// Temporal Unit
	AV1_ENUM_OPTION_FU		= (1<<5),	// Frame Unit
	AV1_ENUM_OPTION_OBU		= (1<<6),	// Open Bitstream Unit
	AV1_ENUM_OPTION_ALL		= (AV1_ENUM_OPTION_TU | AV1_ENUM_OPTION_FU | AV1_ENUM_OPTION_OBU),
};

enum AV1_BYTESTREAM_FORMAT
{
	AV1_BYTESTREAM_RAW = 0,				// (OBU)+, low-overhead bit-stream
	AV1_BYTESTREAM_LENGTH_DELIMITED,	// ([Temporal Unit [Frame Unit [OBU]+]+]+)
};

namespace BST {
	namespace AV1 {
		struct OPEN_BITSTREAM_UNIT;
	}
}

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
	virtual void			Reset() = 0;

public:
	IAV1Context() {}
	virtual ~IAV1Context() {}
};

class IAV1Enumerator
{
public:
	virtual RET_CODE		EnumTemporalUnitStart(IAV1Context* pCtx, uint8_t* ptr_TU_buf, uint32_t TU_size) = 0;
	virtual RET_CODE		EnumFrameUnitStart(IAV1Context* pCtx, uint8_t* pFrameUnitBuf, uint32_t cbFrameUnitBuf) = 0;
	virtual RET_CODE		EnumOBU(IAV1Context* pCtx, uint8_t* pOBUBuf, size_t cbOBUBuf) = 0;
	virtual RET_CODE		EnumFrameUnitEnd(IAV1Context* pCtx, uint8_t* pFrameUnitBuf, uint32_t cbFrameUnitBuf) = 0;
	virtual RET_CODE		EnumTemporalUnitEnd(IAV1Context* pCtx, uint8_t* ptr_TU_buf, uint32_t TU_size) = 0;
	/*
		Error Code:
			1 Buffer is too small
			2 forbidon_bit is not 0
			3 reserved bit is not 0
			4 incompatible bit-stream
	*/
	virtual RET_CODE		EnumError(IAV1Context* pCtx, uint64_t stream_offset, int error_code) = 0;
};

class CAV1Parser
{
public:
	CAV1Parser(bool bAnnexB, bool bSingleOBUParse, RET_CODE* pRetCode);
	virtual ~CAV1Parser();

public:
	RET_CODE				SetEnumerator(IAV1Enumerator* pEnumerator, uint32_t options);
	RET_CODE				ProcessInput(uint8_t* pInput, size_t cbInput);
	RET_CODE				ProcessOutput(bool bDrain = false);
	RET_CODE				ParseFrameBuf(uint8_t* pAUBuf, size_t cbAUBuf);
	RET_CODE				GetAV1Context(IAV1Context** ppCtx);
	RET_CODE				Reset();

protected:
	RET_CODE				ProcessLengthDelimitedBitstreamOutput(bool bDrain);
	RET_CODE				ProcessLowOverheadBitstreamOutput(bool bDrain);
	RET_CODE				PushTemporalUnitBytes(uint8_t* pStart, uint8_t* pEnd);
	RET_CODE				SubmitAnnexBTU();
	RET_CODE				SubmitTU();

protected:
	IAV1Context*			m_pCtx = nullptr;
	AV1_BYTESTREAM_FORMAT	m_av1_bytestream_format;

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
};

#endif
