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
#ifndef __MEDIA_SYNTAX_ELEMENT_H__
#define __MEDIA_SYNTAX_ELEMENT_H__

#include "dump_data_type.h"
#include "combase.h"
#include <cstdint>

/*!	@brief Media Syntax Element Parser */
class IMSEParser : public IUnknown
{
public:
	virtual RET_CODE		SetEnumerator(IUnknown* pEnumerator, uint32_t options) = 0;
	virtual RET_CODE		ProcessInput(uint8_t* pBuf, size_t cbBuf) = 0;
	virtual RET_CODE		ProcessOutput(bool bDrain = false) = 0;
	virtual RET_CODE		GetContext(IUnknown** ppCtx) = 0;
	virtual RET_CODE		Reset() = 0;

public:
	IMSEParser(){}
	virtual ~IMSEParser() {}
};

/*!	@brief General Access-Unit enumerator */
class IAUEnumerator : public IUnknown
{
public:
	virtual RET_CODE		EnumAUBegin(IUnknown* pCtx, uint8_t* pAUBuf, size_t cbAUBuf) = 0;
	virtual RET_CODE		EnumAUEnd(IUnknown* pCtx, uint8_t* pAUBuf, size_t cbAUBuf) = 0;
};

/*!	@brief NAL syntax element enumerator */
class INALEnumerator : public IUnknown
{
public:
	virtual RET_CODE		EnumNALAUBegin(IUnknown* pCtx, uint8_t* pEBSPAUBuf, size_t cbEBSPAUBuf) = 0;
	virtual RET_CODE		EnumNALUnitBegin(IUnknown* pCtx, uint8_t* pEBSPNUBuf, size_t cbEBSPNUBuf) = 0;
	virtual RET_CODE		EnumNALSEIMessageBegin(IUnknown* pCtx, uint8_t* pRBSPSEIMsgRBSPBuf, size_t cbRBSPSEIMsgBuf) = 0;
	virtual RET_CODE		EnumNALSEIPayloadBegin(IUnknown* pCtx, uint32_t payload_type, uint8_t* pRBSPSEIPayloadBuf, size_t cbRBSPPayloadBuf) = 0;
	virtual RET_CODE		EnumNALSEIPayloadEnd(IUnknown* pCtx, uint32_t payload_type, uint8_t* pRBSPSEIPayloadBuf, size_t cbRBSPPayloadBuf) = 0;
	virtual RET_CODE		EnumNALSEIMessageEnd(IUnknown* pCtx, uint8_t* pRBSPSEIMsgRBSPBuf, size_t cbRBSPSEIMsgBuf) = 0;
	virtual RET_CODE		EnumNALUnitEnd(IUnknown* pCtx, uint8_t* pEBSPNUBuf, size_t cbEBSPNUBuf) = 0;
	virtual RET_CODE		EnumNALAUEnd(IUnknown* pCtx, uint8_t* pEBSPAUBuf, size_t cbEBSPAUBuf) = 0;
	/*
		Error Code:
			1 Buffer is too small
			2 forbidon_zero_bit is not 0
			3 Incompatible slice header in a coded picture
			4 nal_unit_header in the same picture unit is not the same
			5 Incompatible parameter set rbsp
	*/
	virtual RET_CODE		EnumNALError(IUnknown* pCtx, uint64_t stream_offset, int error_code) = 0;
};

/*! @brief AV1 syntax element enumerator */
class IAV1Enumerator : public IUnknown
{
public:
	virtual RET_CODE		EnumTemporalUnitStart(IUnknown* pCtx, uint8_t* ptr_TU_buf, uint32_t TU_size) = 0;
	virtual RET_CODE		EnumFrameUnitStart(IUnknown* pCtx, uint8_t* pFrameUnitBuf, uint32_t cbFrameUnitBuf) = 0;
	virtual RET_CODE		EnumOBU(IUnknown* pCtx, uint8_t* pOBUBuf, size_t cbOBUBuf, uint8_t obu_type, uint32_t obu_size) = 0;
	virtual RET_CODE		EnumFrameUnitEnd(IUnknown* pCtx, uint8_t* pFrameUnitBuf, uint32_t cbFrameUnitBuf) = 0;
	virtual RET_CODE		EnumTemporalUnitEnd(IUnknown* pCtx, uint8_t* ptr_TU_buf, uint32_t TU_size) = 0;
	/*
		Error Code:
			1 Buffer is too small
			2 forbidon_bit is not 0
			3 reserved bit is not 0
			4 incompatible bit-stream
	*/
	virtual RET_CODE		EnumError(IUnknown* pCtx, uint64_t stream_offset, int error_code) = 0;
};

/*! @brief MPEG Video syntax element enumerator */
class IMPVEnumerator : public IUnknown
{
public:
	virtual RET_CODE		EnumAUStart(IUnknown* pCtx, uint8_t* pAUBuf, size_t cbAUBuf) = 0;
	virtual RET_CODE		EnumSliceStart(IUnknown* pCtx, uint8_t* pSliceBuf, size_t cbSliceBuf) = 0;
	virtual RET_CODE		EnumSliceEnd(IUnknown* pCtx, uint8_t* pSliceBuf, size_t cbSliceBuf) = 0;
	virtual RET_CODE		EnumAUEnd(IUnknown* pCtx, uint8_t* pAUBuf, size_t cbAUBuf) = 0;
	virtual RET_CODE		EnumObject(IUnknown* pCtx, uint8_t* pBufWithStartCode, size_t cbBufWithStartCode) = 0;

	/*
		Error Code:
			1 Buffer is too small
			2 forbidon_zero_bit is not 0
			3 Incompatible slice header in a coded picture
			4 nal_unit_header in the same picture unit is not the same
			5 Incompatible parameter set rbsp
	*/
	virtual RET_CODE		EnumError(IUnknown* pCtx, uint64_t stream_offset, int error_code) = 0;
};

/*! @brief MPEG-4 AAC LOAS/LATM syntax element enumerator*/
class ILOASEnumerator : public IUnknown
{
public:
	virtual RET_CODE		EnumLATMAUBegin(IUnknown* pCtx, uint8_t* pLATMAUBuf, size_t cbLATMAUBuf) = 0;
	virtual RET_CODE		EnumSubFrameBegin(IUnknown* pCtx, uint8_t* pSubFramePayload, size_t cbSubFramePayload) = 0;
	virtual RET_CODE		EnumSubFrameEnd(IUnknown* pCtx, uint8_t* pSubFramePayload, size_t cbSubFramePayload) = 0;
	virtual RET_CODE		EnumLATMAUEnd(IUnknown* pCtx, uint8_t* pLATMAUBuf, size_t cbLATMAUBuf) = 0;
	virtual RET_CODE		EnumError(IUnknown* pCtx, RET_CODE error_code) = 0;
};

#endif
