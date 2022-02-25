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
	virtual void			Reset() = 0;

public:
	IAV1Context() {}
	virtual ~IAV1Context() {}
};

class IAV1Enumerator
{
public:
	virtual RET_CODE		EnumTemporalUnitStart(IAV1Context* pCtx, uint32_t temporal_unit_size) = 0;
	virtual RET_CODE		EnumFrameUnitStart(IAV1Context* pCtx, uint8_t* pFrameUnitBuf, uint32_t cbFrameUnitBuf) = 0;
	virtual RET_CODE		EnumOBU(IAV1Context* pCtx, uint8_t* pOBUBuf, size_t cbOBUBuf) = 0;
	virtual RET_CODE		EnumFrameUnitEnd(IAV1Context* pCtx, uint8_t* pFrameUnitBuf, uint32_t cbFrameUnitBuf) = 0;
	virtual RET_CODE		EnumTemporalUnitEnd(IAV1Context* pCtx) = 0;
	virtual RET_CODE		EnumError(IAV1Context* pCtx, uint64_t stream_offset, int error_code) = 0;
};

class CAV1Parser
{
public:
	CAV1Parser(bool bAnnexB, bool bSingleOBUParse, RET_CODE* pRetCode);
	virtual ~CAV1Parser();

public:
	RET_CODE				SetEnumerator(IAV1Enumerator* pEnumerator, uint32_t options);
	RET_CODE				ProcessInput(uint8_t* pBuf, size_t cbBuf);
	RET_CODE				ProcessOutput(bool bDrain = false);
	RET_CODE				ParseFrameBuf(uint8_t* pAUBuf, size_t cbAUBuf);
	RET_CODE				GetAV1Context(IAV1Context** ppCtx);
	RET_CODE				Reset();

protected:
	IAV1Context*			m_pCtx = nullptr;

	const int				read_unit_size = 2048;
	AMLinearRingBuffer		m_rbRawBuf = nullptr;

	IAV1Enumerator*			m_av1_enum = nullptr;
	uint32_t				m_av1_enum_options = 0;
};


#endif
