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
#include "systemdef.h"

#define NAL_LEVEL_VSEQ				2
#define NAL_LEVEL_CVS				3
#define NAL_LEVEL_AU				4
#define NAL_LEVEL_NU				5
#define NAL_LEVEL_SEI_MSG			6
#define NAL_LEVEL_SEI_PAYLOAD		7

#define NAL_LEVEL_NAME(lvl_id)	(\
	(lvl_id) == NAL_LEVEL_VSEQ?"VSEQ":(\
	(lvl_id) == NAL_LEVEL_CVS?"CVS":(\
	(lvl_id) == NAL_LEVEL_AU?"AU":(\
	(lvl_id) == NAL_LEVEL_NU?"NU":(\
	(lvl_id) == NAL_LEVEL_SEI_MSG?"SEIMSG":(\
	(lvl_id) == NAL_LEVEL_SEI_PAYLOAD?"SEIPL":""))))))

#define MPV_LEVEL_VSEQ				2
#define MPV_LEVEL_GOP				3
#define MPV_LEVEL_AU				4
#define MPV_LEVEL_SE				5
#define MPV_LEVEL_MB				6

#define MPV_LEVEL_NAME(lvl_id)	(\
	(lvl_id) == MPV_LEVEL_VSEQ?"VSEQ":(\
	(lvl_id) == MPV_LEVEL_GOP?"GOP":(\
	(lvl_id) == MPV_LEVEL_AU?"AU":(\
	(lvl_id) == MPV_LEVEL_SE?"SE":(\
	(lvl_id) == MPV_LEVEL_MB?"MB":"")))))

#define AV1_LEVEL_VSEQ				2
#define AV1_LEVEL_CVS				3
#define AV1_LEVEL_TU				4
#define AV1_LEVEL_FU				5
#define AV1_LEVEL_OBU				6

#define AV1_LEVEL_NAME(lvl_id)	(\
	(lvl_id) == AV1_LEVEL_VSEQ?"VSEQ":(\
	(lvl_id) == AV1_LEVEL_CVS?"CVS":(\
	(lvl_id) == AV1_LEVEL_TU?"TU":(\
	(lvl_id) == AV1_LEVEL_FU?"FU":(\
	(lvl_id) == AV1_LEVEL_OBU?"OBU":"")))))

#define GENERAL_LEVEL_AU			4

enum MSE_ENUM_OPTION
{
	//
	// For NAL media scheme
	//
	NAL_ENUM_OPTION_VSEQ	= (1 << NAL_LEVEL_VSEQ),
	NAL_ENUM_OPTION_CVS		= (1 << NAL_LEVEL_CVS),
	NAL_ENUM_OPTION_AU		= (1 << NAL_LEVEL_AU),
	NAL_ENUM_OPTION_NU		= (1 << NAL_LEVEL_NU),
	NAL_ENUM_OPTION_SEI_MSG	= (1 << NAL_LEVEL_SEI_MSG),
	NAL_ENUM_OPTION_SEI_PAYLOAD
							= (1 << NAL_LEVEL_SEI_PAYLOAD),
	NAL_ENUM_OPTION_ALL		= (NAL_ENUM_OPTION_VSEQ | NAL_ENUM_OPTION_CVS | NAL_ENUM_OPTION_AU | NAL_ENUM_OPTION_NU | NAL_ENUM_OPTION_SEI_MSG | NAL_ENUM_OPTION_SEI_PAYLOAD),

	//
	// For MPV media scheme
	//
	MPV_ENUM_OPTION_VSEQ	= (1 << MPV_LEVEL_VSEQ),
	MPV_ENUM_OPTION_GOP		= (1 << MPV_LEVEL_GOP),
	MPV_ENUM_OPTION_AU		= (1 << MPV_LEVEL_AU),
	MPV_ENUM_OPTION_SE		= (1 << MPV_LEVEL_SE),
	MPV_ENUM_OPTION_MB		= (1 << MPV_LEVEL_MB),
	MPV_ENUM_OPTION_ALL		= (MPV_ENUM_OPTION_VSEQ | MPV_ENUM_OPTION_GOP | MPV_ENUM_OPTION_AU | MPV_ENUM_OPTION_SE | MPV_ENUM_OPTION_MB),

	//
	// For AV1 media scheme
	//
	AV1_ENUM_OPTION_VSEQ	= (1 << AV1_LEVEL_VSEQ),
	AV1_ENUM_OPTION_CVS		= (1 << AV1_LEVEL_CVS),
	AV1_ENUM_OPTION_TU		= (1 << AV1_LEVEL_TU),	// Temporal Unit
	AV1_ENUM_OPTION_FU		= (1 << AV1_LEVEL_FU),	// Frame Unit
	AV1_ENUM_OPTION_OBU		= (1 << AV1_LEVEL_OBU),	// Open Bitstream Unit
	AV1_ENUM_OPTION_ALL		= (AV1_ENUM_OPTION_VSEQ | AV1_ENUM_OPTION_CVS | AV1_ENUM_OPTION_TU | AV1_ENUM_OPTION_FU | AV1_ENUM_OPTION_OBU),

	//
	// For general audio scheme
	//
	GENERAL_ENUM_AU			= (1 << GENERAL_LEVEL_AU),

	// reserve for output style
	MSE_ENUM_CMD_MASK		= 0xF0000000,
	MSE_ENUM_LIST_VIEW		= 0x10000000,
	MSE_ENUM_SYNTAX_VIEW	= 0x20000000,
	MSE_ENUM_HEX_VIEW		= 0x30000000,
	MSE_ENUM_AU_RANGE		= 0x40000000,
};

/*!	@brief Media Syntax Element Parser */
class IMSEParser : public IUnknown
{
public:
	virtual MEDIA_SCHEME_TYPE
							GetSchemeType() = 0;
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
	virtual RET_CODE		EnumNewVSEQ(IUnknown* pCtx) = 0;
	virtual RET_CODE		EnumNewCVS(IUnknown* pCtx, int8_t represent_nal_unit_type) = 0;
	virtual RET_CODE		EnumNALAUBegin(IUnknown* pCtx, uint8_t* pEBSPAUBuf, size_t cbEBSPAUBuf, int picture_slice_type) = 0;
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
	virtual RET_CODE		EnumNewVSEQ(IUnknown* pCtx) = 0;
	virtual RET_CODE		EnumNewCVS(IUnknown* pCtx, int8_t reserved) = 0;
	virtual RET_CODE		EnumTemporalUnitStart(IUnknown* pCtx, uint8_t* ptr_TU_buf, uint32_t TU_size, int frame_type) = 0;
	virtual RET_CODE		EnumFrameUnitStart(IUnknown* pCtx, uint8_t* pFrameUnitBuf, uint32_t cbFrameUnitBuf, int frame_type) = 0;
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
	/*! @brief Notify a new GOP came
		@remarks Can't allocate GOP buffer, and no GOP end function correspondingly. */
	virtual RET_CODE		EnumVSEQStart(IUnknown* pCtx) = 0;
	virtual RET_CODE		EnumNewGOP(IUnknown* pCtx, bool closed_gop, bool broken_link) = 0;
	virtual RET_CODE		EnumAUStart(IUnknown* pCtx, uint8_t* pAUBuf, size_t cbAUBuf, int picCodingType) = 0;
	virtual RET_CODE		EnumObject(IUnknown* pCtx, uint8_t* pBufWithStartCode, size_t cbBufWithStartCode) = 0;
	virtual RET_CODE		EnumAUEnd(IUnknown* pCtx, uint8_t* pAUBuf, size_t cbAUBuf, int picCodingType) = 0;
	virtual RET_CODE		EnumVSEQEnd(IUnknown* pCtx) = 0;

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

/*!	@brief AAC ADTS payload syntax element enumerator */
class IADTSEnumerator : public IUnknown
{
public:
	// TODO...
};

#endif
