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
#include "platcomm.h"
#include "av1.h"
#include "av1_parser.h"

#define ENABLE_LOAD_CONTEXT_SNAPSHOT

INLINE RET_CODE decode_subexp(BITBUF& bitbuf, int32_t numSyms, int32_t& val)
{
	int iRet = RET_CODE_SUCCESS;
	int32_t i = 0;
	int32_t mk = 0;
	int32_t k = 3;
	while (1) {
		uint8_t b2 = i ? k + i - 1 : k;
		int32_t a = 1 << b2;
		if (numSyms <= mk + 3 * a) {
			int32_t subexp_final_bits;
			if (AMP_FAILED(iRet = bitbuf.AV1ns((uint64_t)numSyms - mk, subexp_final_bits)))break;
			val = subexp_final_bits + mk;
			break;
		}
		else {
			bool subexp_more_bits;
			if (AMP_FAILED(iRet = bitbuf.GetFlag(subexp_more_bits)))break;
			if (subexp_more_bits) {
				i++;
				mk += a;
			}
			else {
				uint32_t subexp_bits;
				if (AMP_FAILED(iRet = bitbuf.GetValue(b2, subexp_bits)))break;
				val = subexp_bits + mk;
				break;
			}
		}
	}

	return iRet;
}

INLINE RET_CODE decode_unsigned_subexp_with_ref(BITBUF& bitbuf, int32_t mx, int32_t r, int32_t& val)
{
	int32_t v;
	int iRet = decode_subexp(bitbuf, mx, v);
	if (AMP_FAILED(iRet))return iRet;

	if ((r << 1) <= mx) {
		val = BST::AV1::inverse_recenter(r, v);
	}
	else {
		val = mx - 1 - BST::AV1::inverse_recenter(mx - 1 - r, v);
	}

	return iRet;
}

INLINE RET_CODE decode_signed_subexp_with_ref(BITBUF& bitbuf, int32_t low, int32_t high, int32_t r, int32_t& val)
{
	int32_t x;
	if (high < low)
		return RET_CODE_INVALID_PARAMETER;

	int iRet = decode_unsigned_subexp_with_ref(bitbuf, high - low, r - low, x);
	if (AMP_FAILED(iRet))return iRet;
	val = x + low;
	return iRet;
}

CAV1Parser::CAV1Parser(bool bAnnexB, bool bSingleOBUParse, bool bIVF, RET_CODE* pRetCode)
	: m_av1_bytestream_format(bAnnexB ? AV1_BYTESTREAM_LENGTH_DELIMITED : AV1_BYTESTREAM_RAW)
	, m_bIVF(bIVF)
	, m_rbTemporalUnit(nullptr)
{
	RET_CODE ret_code = RET_CODE_SUCCESS;
	if (AMP_FAILED(CreateAV1Context(&m_pCtx, bAnnexB, bSingleOBUParse)))
	{
		printf("Failed to create the AV1 Video context.\n");
		ret_code = RET_CODE_ERROR;
	}

	m_rbRawBuf = AM_LRB_Create(read_unit_size * 128);

	// Create 2MB linear ring-buffer, it is used to store a whole temporal unit buffer
	m_rbTemporalUnit = AM_LRB_Create(1024 * 1024 * 2);

	AMP_SAFEASSIGN(pRetCode, ret_code);
}

CAV1Parser::~CAV1Parser()
{
	AMP_SAFERELEASE(m_av1_enum);
	AMP_SAFERELEASE(m_pCtx);
	AM_LRB_Destroy(m_rbTemporalUnit);
	AM_LRB_Destroy(m_rbRawBuf);
}

RET_CODE CAV1Parser::SetEnumerator(IUnknown* pEnumerator, uint32_t options)
{
	IAV1Enumerator* pAV1Eumerator = nullptr;
	if (pEnumerator != nullptr)
	{
		if (FAILED(pEnumerator->QueryInterface(IID_IAV1Enumerator, (void**)&pAV1Eumerator)))
		{
			return RET_CODE_ERROR;
		}
	}

	if (m_av1_enum)
		m_av1_enum->Release();

	m_av1_enum = pAV1Eumerator;

	m_av1_enum_options = options;

	return RET_CODE_SUCCESS;
}

RET_CODE CAV1Parser::ProcessInput(uint8_t* pInput, size_t cbInput)
{
	int read_size = 0;
	RET_CODE iRet = RET_CODE_SUCCESS;

	if (pInput == NULL || cbInput == 0)
		return RET_CODE_INVALID_PARAMETER;

	uint8_t* pBuf = AM_LRB_GetWritePtr(m_rbRawBuf, &read_size);
	if (pBuf == NULL || read_size < 0 || (size_t)read_size < cbInput)
	{
		// Try to reform linear ring buffer
		AM_LRB_Reform(m_rbRawBuf);
		if ((pBuf = AM_LRB_GetWritePtr(m_rbRawBuf, &read_size)) == NULL || read_size < 0 || (size_t)read_size < cbInput)
		{
			printf("[NALParser] Failed to get the write buffer(%p), or the write buffer size(%d) is not enough.\n", pBuf, read_size);
			return RET_CODE_BUFFER_TOO_SMALL;
		}
	}

	memcpy(pBuf, pInput, cbInput);

	AM_LRB_SkipWritePtr(m_rbRawBuf, (unsigned int)cbInput);

	return RET_CODE_SUCCESS;
}

RET_CODE CAV1Parser::ProcessOutput(bool bDrain)
{
	if (m_av1_bytestream_format == AV1_BYTESTREAM_LENGTH_DELIMITED)
		return ProcessLengthDelimitedBitstreamOutput(bDrain);
	else if (m_av1_bytestream_format == AV1_BYTESTREAM_RAW)
		return ProcessLowOverheadBitstreamOutput(bDrain);

	return RET_CODE_ERROR_NOTIMPL;
}

RET_CODE CAV1Parser::ParseFrameBuf(uint8_t* pAUBuf, size_t cbAUBuf)
{
	RET_CODE iRet = RET_CODE_SUCCESS;

	uint8_t* p = pAUBuf;
	int64_t cbLeft = (int64_t)cbAUBuf;
	while (cbLeft > 0)
	{
		int64_t cbSubmit = AMP_MIN(cbLeft, 2048);
		if (AMP_SUCCEEDED(iRet = ProcessInput(p, (size_t)cbSubmit)))
		{
			if (ProcessOutput(cbLeft <= 2048 ? true : false) == RET_CODE_ABORT)
			{
				iRet = RET_CODE_ABORT;
				break;
			}
			cbLeft -= cbSubmit;
			p += cbSubmit;
		}
		else
			break;
	}

	return iRet;
}

RET_CODE CAV1Parser::GetContext(IUnknown** ppCtx)
{
	if (ppCtx == nullptr)
		return RET_CODE_INVALID_PARAMETER;

	m_pCtx->AddRef();
	*ppCtx = (IUnknown*)m_pCtx;
	return RET_CODE_SUCCESS;
}

RET_CODE CAV1Parser::Reset()
{
	m_av1_obu_type = -1;
	m_av1_obu_size = UINT32_MAX;
	m_av1_obu_parsed_size = 0;
	m_temporal_unit_size = UINT32_MAX;
	m_temporal_unit_parsed_size = 0;

	m_stream_size = 0;
	m_cur_byte_pos = m_cur_scan_pos = m_cur_submit_pos = 0;

	AM_LRB_Reset(m_rbTemporalUnit);
	AM_LRB_Reset(m_rbRawBuf);

	m_pCtx->Reset();

	return RET_CODE_SUCCESS;
}

RET_CODE CAV1Parser::ProcessLengthDelimitedBitstreamOutput(bool bDrain)
{
	RET_CODE iRet = RET_CODE_SUCCESS;

	int cbSize = 0;
	uint8_t* pStartBuf = NULL, *pBuf = NULL, *pCurParseStartBuf = NULL;

	uint8_t temporal_unit_size_len = 0;
	uint8_t bNeedMoreData = 0;
	uint8_t last_spatial_id = 0;

	if ((pStartBuf = AM_LRB_GetReadPtr(m_rbRawBuf, &cbSize)) == NULL || cbSize <= 0)
	{
		return RET_CODE_NEEDMOREINPUT;
	}

	pBuf = pStartBuf;

	int iRet2 = RET_CODE_SUCCESS;	// indicate the process result of the coming loop
	while (cbSize > 0)
	{
		if (m_temporal_unit_size == UINT32_MAX)
		{
			if (m_bIVF)
			{
				if (cbSize < 12)
				{
					iRet2 = RET_CODE_NEEDMOREINPUT;
					break;
				}

				m_temporal_unit_size = pBuf[0] | ((uint32_t)pBuf[1] << 8) | ((uint32_t)pBuf[2] << 16) | ((uint32_t)pBuf[3] << 24);
				temporal_unit_size_len = 12;
			}
			else
			{
				m_temporal_unit_size = BST::AV1::leb128(pBuf, cbSize, &temporal_unit_size_len, &bNeedMoreData);
				if (m_temporal_unit_size == UINT32_MAX)
				{
					if (bNeedMoreData)
					{
						iRet2 = RET_CODE_NEEDMOREINPUT;
						break;
					}
					else
					{
						printf("[AV1Parser] The AV1 byte-stream is NOT compatible.\n");
						iRet = RET_CODE_BUFFER_NOT_COMPATIBLE;
						cbSize--; pBuf++;
						continue;
					}
				}
			}

			PushTemporalUnitBytes(pBuf, pBuf + temporal_unit_size_len);
			AM_LRB_SkipReadPtr(m_rbRawBuf, temporal_unit_size_len);

			cbSize -= temporal_unit_size_len;
			pBuf += temporal_unit_size_len;

			// Update the submit position
			m_cur_submit_pos = m_cur_scan_pos;

			// Update the scan position
			m_cur_scan_pos += temporal_unit_size_len;
		}

		int nLeftTUBytes = m_temporal_unit_size - m_temporal_unit_parsed_size;
		int nCopyTUBytes = AMP_MIN(cbSize, nLeftTUBytes);

		if (nCopyTUBytes > 0)
		{
			PushTemporalUnitBytes(pBuf, pBuf + nCopyTUBytes);
			AM_LRB_SkipReadPtr(m_rbRawBuf, nCopyTUBytes);

			m_temporal_unit_parsed_size += nCopyTUBytes;
		}

		if (nLeftTUBytes <= cbSize)
		{
			// Already found a complete temporal_unit, begin to parse it
			if (SubmitAnnexBTU() == RET_CODE_ABORT)
			{
				iRet = RET_CODE_ABORT;
				break;
			}

			AM_LRB_Reset(m_rbTemporalUnit);
			m_temporal_unit_size = UINT32_MAX;
			m_temporal_unit_parsed_size = 0;
		}

		cbSize -= nCopyTUBytes;
		pBuf += nCopyTUBytes;

		// Update the scan position
		m_cur_scan_pos += nCopyTUBytes;
	}

	return iRet;
}

RET_CODE CAV1Parser::ProcessLowOverheadBitstreamOutput(bool bDrain)
{
	RET_CODE iRet = RET_CODE_SUCCESS;

	int cbSize = 0;
	uint8_t* pStartBuf = NULL, *pBuf = NULL, *pCurParseStartBuf = NULL;

	uint8_t temporal_unit_size_len = 0;
	uint8_t temporal_unit_size_leb128_bytes = 0;
	uint8_t bNeedMoreData = 0;
	uint8_t last_spatial_id = 0;
	int iRet2 = RET_CODE_SUCCESS;	// indicate the process result of the coming loop

	if ((pStartBuf = AM_LRB_GetReadPtr(m_rbRawBuf, &cbSize)) == NULL || cbSize <= 0)
	{
		if (bDrain)
			goto Drain;

		return RET_CODE_NEEDMOREINPUT;
	}

	pBuf = pStartBuf;

	while (cbSize > 0)
	{
		if (m_temporal_unit_size == UINT32_MAX && m_bIVF)
		{
			if (cbSize < 12)
			{
				iRet2 = RET_CODE_NEEDMOREINPUT;
				break;
			}

			m_temporal_unit_size = pBuf[0] | ((uint32_t)pBuf[1] << 8) | ((uint32_t)pBuf[2] << 16) | ((uint32_t)pBuf[3] << 24);
			temporal_unit_size_len = 12;

			PushTemporalUnitBytes(pBuf, pBuf + temporal_unit_size_len);
			AM_LRB_SkipReadPtr(m_rbRawBuf, temporal_unit_size_len);

			cbSize -= temporal_unit_size_len;
			pBuf += temporal_unit_size_len;

			// Update the submit position
			m_cur_submit_pos = m_cur_scan_pos;

			// Update the scan position
			m_cur_scan_pos += temporal_unit_size_len;
		}

		if (m_av1_obu_type == -1)
		{
			if (cbSize < 1)
			{
				iRet2 = RET_CODE_NEEDMOREINPUT;
				break;
			}

			uint8_t* pOBUBuf = pBuf, cbOBUSize = 0;
			uint8_t obu_header_byte = *pOBUBuf;

			uint8_t obu_forbidden_bit = (obu_header_byte >> 7) & 0x01;
			uint8_t obu_type = (obu_header_byte >> 3) & 0xF;
			uint8_t obu_extension_flag = (obu_header_byte >> 2) & 0x1;
			uint8_t obu_has_size_flag = (obu_header_byte >> 1) & 0x1;
			uint8_t obu_reserved_1bit = obu_header_byte & 0x1;

			pOBUBuf++;
			cbOBUSize++;

			uint8_t temporal_id = 0;
			uint8_t spatial_id = 0;
			uint8_t extension_header_reserved_3bits = 0;

			uint8_t obu_size_leb128_bytes = 0;
			uint8_t obu_need_more_data = 0;
			uint32_t obu_size = UINT32_MAX;

			if (obu_extension_flag)
			{
				if (cbSize < 2)
				{
					iRet2 = RET_CODE_NEEDMOREINPUT;
					break;
				}

				temporal_id = ((*pOBUBuf) >> 5) & 0x7;
				spatial_id = ((*pOBUBuf) >> 3) & 0x3;
				extension_header_reserved_3bits = (*pOBUBuf) & 0x7;

				pOBUBuf++;
				cbOBUSize++;
			}

			if (obu_has_size_flag)
			{
				obu_size = BST::AV1::leb128(pOBUBuf, cbSize - cbOBUSize, &obu_size_leb128_bytes, &obu_need_more_data);
				if (obu_size == UINT32_MAX)
				{
					if (!obu_need_more_data)
					{
						printf("[AV1Parser] Failed to extract obu_size, it seems the current AV1 stream is corrupt or invalid.\n");
						iRet = RET_CODE_BUFFER_NOT_COMPATIBLE;
						goto done;
					}
					else
					{
						iRet2 = RET_CODE_NEEDMOREINPUT;
						break;
					}
				}
			}
			else
			{
				// If there is no obu_size field, quit the current loop
				printf("[AV1] For Low-Overhead AV1 stream, obu_size field is not available, so quit the scanning process.\n");
				iRet = RET_CODE_ERROR_NOTIMPL;
				goto done;
			}

			// Finished parsing the OBU header and size completely
			bool bGotTU = false;
			if (!m_bIVF && m_temporal_unit_parsed_size > 0)
			{
				if (obu_type == OBU_TEMPORAL_DELIMITER)
				{
					bGotTU = true;
				}
				else if (obu_size == 0)
				{
					bGotTU = true;
				}
				else if (obu_extension_flag && last_spatial_id != spatial_id)
				{
					bGotTU = true;
				}

				if (bGotTU)
				{
					// Already found a complete temporal_unit, begin to parse it
					if (SubmitTU() == RET_CODE_ABORT)
					{
						iRet = RET_CODE_ABORT;
						goto done;
					}

					m_temporal_unit_parsed_size = 0;

					// Update the submit position
					m_cur_submit_pos = m_cur_scan_pos;
				}
			}

			m_av1_obu_type = obu_type;
			m_av1_obu_size = obu_size + 1 + (obu_extension_flag ? 1 : 0) + obu_size_leb128_bytes;
			m_av1_obu_parsed_size = 0;

			if (obu_extension_flag && last_spatial_id != spatial_id)
				last_spatial_id = spatial_id;
		}

		int nLeftOBUBytes = m_av1_obu_size - m_av1_obu_parsed_size;
		int nCopyOBUBytes = AMP_MIN(cbSize, nLeftOBUBytes);

		if (nCopyOBUBytes > 0)
		{
			PushTemporalUnitBytes(pBuf, pBuf + nCopyOBUBytes);
			AM_LRB_SkipReadPtr(m_rbRawBuf, nCopyOBUBytes);
		}

		if (nLeftOBUBytes <= cbSize)
		{
			// Already got a complete OBU
			m_av1_obu_type = -1;
			m_av1_obu_size = UINT32_MAX;
			m_av1_obu_parsed_size = 0;
		}
		else
			m_av1_obu_parsed_size += nCopyOBUBytes;

		m_temporal_unit_parsed_size += nCopyOBUBytes;

		cbSize -= nCopyOBUBytes;
		pBuf += nCopyOBUBytes;

		// Update the scan position
		m_cur_scan_pos += nCopyOBUBytes;

		if (m_bIVF && m_temporal_unit_parsed_size >= m_temporal_unit_size)
		{
			// Already found a complete temporal_unit, begin to parse it
			if (SubmitTU() == RET_CODE_ABORT) 
			{
				iRet = RET_CODE_ABORT;
				goto done;
			}

			m_temporal_unit_parsed_size = 0;
			m_temporal_unit_size = UINT32_MAX;

			// Update the submit position
			m_cur_submit_pos = m_cur_scan_pos;
		}
	}

Drain:
	if (bDrain)
	{
		// Flush the data the final temporal unit, if it exists
		if (m_temporal_unit_parsed_size > 0)
		{
			if (SubmitTU() == RET_CODE_ABORT)
			{
				iRet = RET_CODE_ABORT;
				goto done;
			}

			m_temporal_unit_parsed_size = 0;
		}

		m_cur_scan_pos = m_stream_size;
	}

done:
	return iRet;
}

RET_CODE CAV1Parser::PushTemporalUnitBytes(uint8_t* pStart, uint8_t* pEnd)
{
	int cbLeftOfTU = 0;
	uint8_t* pEBSPWriteBuf = AM_LRB_GetWritePtr(m_rbTemporalUnit, &cbLeftOfTU);
	if (pEBSPWriteBuf == NULL || cbLeftOfTU < (int)(pEnd - pStart))
	{
		// Try to reform the ring buffer of EBSP
		AM_LRB_Reform(m_rbTemporalUnit);
		pEBSPWriteBuf = AM_LRB_GetWritePtr(m_rbTemporalUnit, &cbLeftOfTU);
		if (pEBSPWriteBuf == NULL || cbLeftOfTU < (int)(pEnd - pStart))
		{
			// Try to enlarge the ring buffer of EBSP
			int rb_size = AM_LRB_GetSize(m_rbTemporalUnit);
			int64_t new_rb_size = (int64_t)rb_size << 1;
			if (new_rb_size < rb_size + (pEnd - pStart) - (pEBSPWriteBuf != NULL ? cbLeftOfTU : 0))
				new_rb_size = rb_size + (pEnd - pStart) - (pEBSPWriteBuf != NULL ? cbLeftOfTU : 0);

			if (new_rb_size >= INT32_MAX || AM_LRB_Resize(m_rbTemporalUnit, (int)new_rb_size) < 0)
			{
				printf("[AV1Parser] Failed to resize the ring buffer of AV1 Temporal Unit to %" PRId64 ".\n", (int64_t)rb_size * 2);
				return RET_CODE_ERROR;
			}

			pEBSPWriteBuf = AM_LRB_GetWritePtr(m_rbTemporalUnit, &cbLeftOfTU);
		}
	}

	// Write the parsed buffer to ring buffer
	memcpy(pEBSPWriteBuf, pStart, (size_t)(pEnd - pStart));

	AM_LRB_SkipWritePtr(m_rbTemporalUnit, (unsigned int)(pEnd - pStart));

	return RET_CODE_SUCCESS;
}

RET_CODE CAV1Parser::SubmitAnnexBTU()
{
	int iRet = RET_CODE_SUCCESS;
	int cbTUBuf = 0;
	uint8_t* pTUBuf = AM_LRB_GetReadPtr(m_rbTemporalUnit, &cbTUBuf);

	unsigned long cbParsed = 0;
	uint32_t temporal_unit_size = 0;
	uint8_t cbLeb128 = 0;

	if (cbTUBuf > INT32_MAX)
	{
		printf("[AV1Parser] At present, don't support the temporal unit which size is greater than 2GB.\n");
		return RET_CODE_ERROR_NOTIMPL;
	}

	std::vector<
		std::tuple<uint32_t			/* offset to TU start */, 
				   uint8_t*			/*FU buffer start*/, 
				   uint32_t			/*FU size*/,
				   int,				/*Frame Type*/
				   std::vector<
						std::tuple<uint32_t			/* OBU offset to TU start */,
								   uint8_t*			/* OBU buffer start*/,
								   uint32_t,		/* OBU buffer length */
								   uint8_t,			/* OBU type */
								   uint32_t,		/* OBU size in OBU header */
								   OBU_PARSE_PARAMS	/* OBU parser parameters which will be updated to AV1 context separately */>
				   >
		>
	> tu_fu_obus;

	uint8_t* pBuf = pTUBuf;
	uint32_t cbSize = (uint32_t)cbTUBuf;
	bool bAnnexB = m_pCtx->GetByteStreamFormat() == AV1_BYTESTREAM_LENGTH_DELIMITED ? true : false;
	int tu_frame_type = INT32_MIN;
	bool bNewGOP = false;
	bool bNewVSeq = false;

	if (m_bIVF)
	{
		if (cbSize < 12)
			return RET_CODE_BUFFER_NOT_COMPATIBLE;

		// Skip the 12-bytes of frame header
		pBuf += 12;
		cbSize -= 12;
	}

	temporal_unit_size = BST::AV1::leb128(pBuf + cbParsed, cbSize - cbParsed, &cbLeb128);
	if (temporal_unit_size == UINT32_MAX)
	{
		iRet = RET_CODE_BUFFER_NOT_COMPATIBLE;
		goto done;
	}

	cbParsed += cbLeb128;

	while (cbParsed < cbSize)
	{
		uint32_t cbFrameParsed = 0;
		uint32_t frame_unit_size = UINT32_MAX;
		uint8_t* pFrameUnit = nullptr;

		// enumerate each OBU in the current buffer
		frame_unit_size = BST::AV1::leb128(pBuf + cbParsed, cbSize - cbParsed, &cbLeb128);
		if (frame_unit_size == UINT32_MAX)
		{
			iRet = RET_CODE_BUFFER_NOT_COMPATIBLE;
			break;
		}

		cbParsed += cbLeb128;
		pFrameUnit = pBuf + cbParsed;

		tu_fu_obus.emplace_back((uint32_t)(pFrameUnit - pTUBuf), pFrameUnit, frame_unit_size, -1, 
			std::vector<std::tuple<uint32_t, uint8_t*, uint32_t, uint8_t, uint32_t, OBU_PARSE_PARAMS>>());

		while (cbFrameParsed < frame_unit_size)
		{
			uint32_t obu_length = BST::AV1::leb128(pBuf + cbParsed, cbSize - cbParsed, &cbLeb128);
			if (obu_length == UINT32_MAX)
			{
				iRet = RET_CODE_BUFFER_NOT_COMPATIBLE;
				break;
			}

			cbFrameParsed += cbLeb128;
			cbParsed += cbLeb128;

			do
			{
				if (obu_length == 0 || (int64_t)cbParsed >= (int64_t)cbTUBuf)
					break;

				BST::AV1::OBU_HEADER obu_hdr;

				uint8_t* p = pBuf + cbParsed;
				size_t cbOBULeft = obu_length;
				uint32_t obu_size = UINT32_MAX;

				obu_hdr.obu_type = ((*p) >> 3) & 0xF;
				obu_hdr.obu_extension_flag = ((*p) >> 2) & 0x1;
				obu_hdr.obu_has_size_field = ((*p) >> 1) & 0x1;
				obu_hdr.obu_reserved_1bit = (*p) & 0x1;

				// obu_reserved_1bit must be set to 0. The value is ignored by a decoder.
				if (obu_hdr.obu_reserved_1bit != 0)
				{
					if (m_av1_enum && AMP_FAILED(m_av1_enum->EnumError(m_pCtx, m_cur_submit_pos, 3)))
					{
						iRet = RET_CODE_ABORT;
						goto done;
					}
				}
				
				p++; cbOBULeft--;
				
				if (obu_hdr.obu_extension_flag){
					if (cbOBULeft == 0)
					{
						if (m_av1_enum && AMP_FAILED(m_av1_enum->EnumError(m_pCtx, m_cur_submit_pos, 1)))
						{
							iRet = RET_CODE_ABORT;
							goto done;
						}
						break;
					}

					obu_hdr.obu_extension_header.temporal_id = ((*p) >> 5) & 0x7;
					obu_hdr.obu_extension_header.spatial_id = ((*p) >> 3) & 0x3;
					obu_hdr.obu_extension_header.extension_header_reserved_3bits = (*p) & 0x7;

					if (obu_hdr.obu_extension_header.extension_header_reserved_3bits != 0)
					{
						if (m_av1_enum && AMP_FAILED(m_av1_enum->EnumError(m_pCtx, m_cur_submit_pos, 3)))
						{
							iRet = RET_CODE_ABORT;
							goto done;
						}
					}

					cbOBULeft--;
					p++;
				}

				if (obu_hdr.obu_has_size_field)
				{
					if (cbOBULeft == 0)
					{
						if (m_av1_enum && AMP_FAILED(m_av1_enum->EnumError(m_pCtx, m_cur_submit_pos, 1)))
						{
							iRet = RET_CODE_ABORT;
							goto done;
						}
						break;
					}

					obu_size = BST::AV1::leb128(p, (uint32_t)cbOBULeft, &cbLeb128);
					if (obu_size != UINT32_MAX)
					{
						uint32_t ob_preceding_size = 1 + obu_hdr.obu_extension_flag + cbLeb128;
						if (obu_size + ob_preceding_size != obu_length)
						{
							printf("[AV1Parser] obu_size and obu_length are NOT set consistently.\n");
							if (m_av1_enum && AMP_FAILED(m_av1_enum->EnumError(m_pCtx, m_cur_submit_pos, 4)))
							{
								iRet = RET_CODE_ABORT;
								goto done;
							}
						}
					}

					if (cbOBULeft < cbLeb128)
						break;

					cbOBULeft -= cbLeb128;
					p += cbLeb128;
				}
				else
					obu_size = obu_length >= (1UL + obu_hdr.obu_extension_flag) ? (obu_length - 1 - obu_hdr.obu_extension_flag) : 0;

				uint8_t obu_type = obu_hdr.obu_type;

				std::get<4>(tu_fu_obus.back()).emplace_back((uint32_t)(pBuf + cbParsed - pTUBuf), pBuf + cbParsed, obu_length, obu_type, obu_size, m_obu_parse_params);

				if (obu_type == OBU_TEMPORAL_DELIMITER)
				{
					m_obu_parse_params.ActiveFrameParams.SeenFrameHeader = false;
				}
				else if (obu_type == OBU_SEQUENCE_HEADER)
				{
					// 7.5. Ordering of OBUs
					//	A new coded video sequence is defined to start at each temporal unit which satisfies both of the following conditions :
					//		• A sequence header OBU appears before the first frame header.
					bool bHitFirstSeqHdrOBU = m_pCtx->GetSeqHdrOBU() == nullptr ? true : false;

					// Update the Sequence Header OBU to the AV1 context
					if (AMP_FAILED(iRet == UpdateSeqHdrToContext(pBuf + cbParsed, obu_length)))
					{
						iRet = RET_CODE_ABORT;
						goto done;
					}

					if (iRet == RET_CODE_SUCCESS)
					{
						if (bHitFirstSeqHdrOBU)
							bNewGOP = true;
						bNewVSeq = true;
					}
				}
				else if (obu_type == OBU_FRAME_HEADER || obu_type == OBU_FRAME || obu_type == OBU_REDUNDANT_FRAME_HEADER)
				{
					if ((iRet = UpdateOBUParsePreconditionParams(p, cbOBULeft, obu_hdr)) == RET_CODE_ABORT)
					{
						iRet = RET_CODE_ABORT;
						goto done;
					}

					// 7.5. Ordering of OBUs
					//	A new coded video sequence is defined to start at each temporal unit which satisfies both of the following conditions :
					//		• A sequence header OBU appears before the first frame header.
					//		• The first frame header has frame_type equal to KEY_FRAME, show_frame equal to 1, show_existing_frame
					//		  equal to 0, and temporal_id equal to 0.
					if (!obu_hdr.obu_extension_flag || (obu_hdr.obu_extension_flag && obu_hdr.obu_extension_header.temporal_id == 0))
					{
						if (!m_obu_parse_params.ActiveFrameParams.show_existing_frame &&
							 m_obu_parse_params.ActiveFrameParams.show_frame &&
							 m_obu_parse_params.ActiveFrameParams.frame_type == KEY_FRAME)
							bNewGOP = true;
					}
				}

			} while (0);

			cbFrameParsed += obu_length;
			cbParsed += obu_length;
		}

		std::get<3>(tu_fu_obus.back()) = m_obu_parse_params.ActiveFrameParams.frame_type;
		if (tu_frame_type == INT32_MIN)
			tu_frame_type = m_obu_parse_params.ActiveFrameParams.frame_type;
		else if (tu_frame_type != m_obu_parse_params.ActiveFrameParams.frame_type)
			tu_frame_type = -1;
	}

	// prepare the notification for the current TU
	if (m_av1_enum != nullptr && AMP_SUCCEEDED(iRet))
	{
		if (bNewVSeq && (m_av1_enum_options&AV1_ENUM_OPTION_VSEQ) && AMP_FAILED(m_av1_enum->EnumNewVSEQ(m_pCtx)))
		{
			iRet = RET_CODE_ABORT;
			goto done;
		}

		if (bNewGOP && (m_av1_enum_options&AV1_ENUM_OPTION_CVS) && AMP_FAILED(m_av1_enum->EnumNewCVS(m_pCtx, 0)))
		{
			iRet = RET_CODE_ABORT;
			goto done;
		}

		if ((m_av1_enum_options&AV1_ENUM_OPTION_TU) && AMP_FAILED(m_av1_enum->EnumTemporalUnitStart(m_pCtx, pTUBuf + (m_bIVF ? 12 : 0), (uint32_t)cbTUBuf, tu_frame_type)))
		{
			iRet = RET_CODE_ABORT;
			goto done;
		}

		for (auto& fu : tu_fu_obus)
		{
			int32_t tu_fu_idx = 0;
			auto pFrameUnit = std::get<1>(fu);
			auto frame_unit_size = std::get<2>(fu);
			if ((m_av1_enum_options&AV1_ENUM_OPTION_FU) && AMP_FAILED(m_av1_enum->EnumFrameUnitStart(m_pCtx, pFrameUnit, frame_unit_size, std::get<3>(fu))))
			{
				iRet = RET_CODE_ABORT;
				goto done;
			}

			for (auto& obu : std::get<4>(fu))
			{
				auto pOBUBuf = std::get<1>(obu);
				auto obu_length = std::get<2>(obu);
				auto obu_type = std::get<3>(obu);
				auto obu_size = std::get<4>(obu);
				if ((m_av1_enum_options&AV1_ENUM_OPTION_OBU) && obu_length != UINT32_MAX && obu_length > 0)
				{
#ifdef ENABLE_LOAD_CONTEXT_SNAPSHOT
					auto& obu_params = std::get<5>(obu);
					AV1ContextSnapshot ctx_snapshot;
					ctx_snapshot.tu_idx = m_num_temporal_units;
					ctx_snapshot.tu_fu_idx = tu_fu_idx;
					ctx_snapshot.pVBISlotParams = obu_params.VBI;
					ctx_snapshot.pActiveFrameParams = &obu_params.ActiveFrameParams;

					if (g_verbose_level > 200)
					{
						printf("[AV1Parser] SeenFrameHeader: %d.\n", obu_params.ActiveFrameParams.SeenFrameHeader);
						printf("[AV1Parser] VBI [%" PRId32 ",%" PRId32 ",%" PRId32 ",%" PRId32 ",%" PRId32 ",%" PRId32 ",%" PRId32 ",%" PRId32 "]\n",
							obu_params.VBI[0].FrameSeqID, obu_params.VBI[0].FrameSeqID, obu_params.VBI[0].FrameSeqID, obu_params.VBI[0].FrameSeqID,
							obu_params.VBI[0].FrameSeqID, obu_params.VBI[0].FrameSeqID, obu_params.VBI[0].FrameSeqID, obu_params.VBI[0].FrameSeqID);
					}

					if (AMP_FAILED(m_pCtx->LoadSnapshot(&ctx_snapshot)))
					{
						printf("[AV1Parser] Failed to load context snapshot for OBU parsing.\n");
					}
#endif
					if (AMP_FAILED(m_av1_enum->EnumOBU(m_pCtx, pOBUBuf, obu_length, obu_type, obu_size)))
					{
						iRet = RET_CODE_ABORT;
						goto done;
					}
				}
			}

			if ((m_av1_enum_options&AV1_ENUM_OPTION_FU) && AMP_FAILED(m_av1_enum->EnumFrameUnitEnd(m_pCtx, pFrameUnit, frame_unit_size)))
			{
				iRet = RET_CODE_ABORT;
				goto done;
			}

			tu_fu_idx++;
		}

		if ((m_av1_enum_options&AV1_ENUM_OPTION_TU) && AMP_FAILED(m_av1_enum->EnumTemporalUnitEnd(m_pCtx, pTUBuf + (m_bIVF ? 12 : 0), (uint32_t)cbTUBuf)))
		{
			iRet = RET_CODE_ABORT;
			goto done;
		}
	}

	m_num_temporal_units++;

done:
	AM_LRB_Reset(m_rbTemporalUnit);
	return iRet;
}

RET_CODE CAV1Parser::SubmitTU()
{
	uint32_t obu_size = UINT32_MAX;
	uint8_t leb128_byte_count = 0;
	uint8_t leb128_need_more_data = false;

	int iRet = RET_CODE_SUCCESS;
	int cbTUBuf = 0;
	uint8_t* pTUBuf = AM_LRB_GetReadPtr(m_rbTemporalUnit, &cbTUBuf);

	if (cbTUBuf > INT32_MAX)
	{
		printf("[AV1Parser] At present, don't support the temporal unit which size is greater than 2GB.\n");
		return RET_CODE_ERROR_NOTIMPL;
	}

	unsigned long cbParsed = 0;
	uint8_t cbLeb128 = 0;
	uint32_t frame_unit_size = UINT32_MAX;
	uint32_t obu_length = UINT32_MAX;
	uint8_t* pFrameUnit = pTUBuf;
	uint8_t* pBuf = pTUBuf;
	uint32_t cbSize = (uint32_t)cbTUBuf;
	int tu_frame_type = INT32_MIN;
	bool bCompleteFUFound = false;
	bool bNewGOP = false;
	bool bNewVSeq = false;

	std::vector<
		std::tuple<uint32_t			/* offset to TU start */, 
				   uint8_t*			/*FU buffer start*/, 
				   uint32_t			/*FU size*/,
				   int,				/*Frame Type*/
				   std::vector<
						std::tuple<uint32_t			/* OBU offset to TU start */,
								   uint8_t*			/* OBU buffer start*/,
								   uint32_t,		/* OBU buffer length */
								   uint8_t,			/* OBU type */
								   uint32_t,		/* OBU size in OBU header */
								   OBU_PARSE_PARAMS	/* OBU parser parameters which will be updated to AV1 context separately */>
				   >
		>
	> tu_fu_obus;

	uint32_t cbFrameParsed = 0;
	std::vector<std::tuple<uint32_t, uint8_t*, uint32_t, uint8_t, uint32_t, OBU_PARSE_PARAMS>> fu_obus;

	if (m_bIVF)
	{
		if (cbSize < 12)
			return RET_CODE_BUFFER_NOT_COMPATIBLE;

		// Skip the 12-bytes of frame header
		pBuf += 12;
		cbSize -= 12;
	}

	while (cbParsed < cbSize)
	{
		BST::AV1::OBU_HEADER obu_hdr;

		// enumerate each OBU in the current buffer
		uint8_t* pOBUBuf = pBuf + cbParsed;
		uint8_t* p = pOBUBuf;
		uint32_t cbLeft = cbSize > cbParsed ? (cbSize - cbParsed) : 0;
		if (cbLeft == 0)
		{
			if (m_av1_enum && AMP_FAILED(m_av1_enum->EnumError(m_pCtx, m_cur_submit_pos, 1)))
			{
				iRet = RET_CODE_ABORT;
				goto done;
			}
		}

		obu_hdr.obu_forbidden_bit = ((*p) >> 7) & 0x1;
		if (obu_hdr.obu_forbidden_bit != 0)
		{
			if (m_av1_enum && AMP_FAILED(m_av1_enum->EnumError(m_pCtx, m_cur_submit_pos, 2)))
			{
				iRet = RET_CODE_ABORT;
				goto done;
			}
		}

		obu_hdr.obu_type = ((*p) >> 3) & 0xF;
		obu_hdr.obu_extension_flag = ((*p) >> 2) & 0x1;
		obu_hdr.obu_has_size_field = ((*p) >> 1) & 0x1;
		obu_hdr.obu_reserved_1bit = (*p) & 0x1;

		// obu_reserved_1bit must be set to 0. The value is ignored by a decoder.
		if (obu_hdr.obu_reserved_1bit != 0)
		{
			if (m_av1_enum && AMP_FAILED(m_av1_enum->EnumError(m_pCtx, m_cur_submit_pos, 3)))
			{
				iRet = RET_CODE_ABORT;
				goto done;
			}
		}

		p++;
		cbLeft--;

		if (obu_hdr.obu_extension_flag)
		{
			if (cbLeft == 0)
			{
				if (m_av1_enum && AMP_FAILED(m_av1_enum->EnumError(m_pCtx, m_cur_submit_pos, 1)))
				{
					iRet = RET_CODE_ABORT;
					goto done;
				}
			}

			obu_hdr.obu_extension_header.temporal_id = ((*p) >> 5) & 0x7;
			obu_hdr.obu_extension_header.spatial_id = ((*p) >> 3) & 0x3;
			obu_hdr.obu_extension_header.extension_header_reserved_3bits = (*p) & 0x7;

			if (obu_hdr.obu_extension_header.extension_header_reserved_3bits != 0)
			{
				if (m_av1_enum && AMP_FAILED(m_av1_enum->EnumError(m_pCtx, m_cur_submit_pos, 3)))
				{
					iRet = RET_CODE_ABORT;
					goto done;
				}
			}

			p++;
			cbLeft--;
		}

		leb128_byte_count = 0;
		if (obu_hdr.obu_has_size_field)
		{
			if (cbLeft == 0)
			{
				if (m_av1_enum && AMP_FAILED(m_av1_enum->EnumError(m_pCtx, m_cur_submit_pos, 1)))
				{
					iRet = RET_CODE_ABORT;
					goto done;
				}
			}

			leb128_need_more_data = false;
			obu_size = BST::AV1::leb128(p, (int)cbLeft, &leb128_byte_count, &leb128_need_more_data);
			if (leb128_need_more_data || obu_size == UINT32_MAX)
			{
				if (m_av1_enum && AMP_FAILED(m_av1_enum->EnumError(m_pCtx, m_cur_submit_pos, 4)))
				{
					iRet = RET_CODE_ABORT;
					goto done;
				}
			}

			cbLeft -= leb128_byte_count;
			p += leb128_byte_count;
		}
		else
		{
			obu_size = UINT32_MAX;
		}

		if (obu_size == UINT32_MAX)
		{
			printf("[AVParser] Don't know how to process the left OBU stream\n");
			break;
		}

		obu_length = obu_size + 1 + obu_hdr.obu_extension_flag + leb128_byte_count;
		cbParsed += obu_length;

		uint8_t obu_type = obu_hdr.obu_type;

		if (obu_type == OBU_SEQUENCE_HEADER || obu_type == OBU_FRAME_HEADER || obu_type == OBU_FRAME || obu_type == OBU_REDUNDANT_FRAME_HEADER)
		{
			if (bCompleteFUFound)
			{
				// calculate the frame unit size
				frame_unit_size = (uint32_t)(pOBUBuf - pFrameUnit);

				tu_fu_obus.emplace_back((uint32_t)(pFrameUnit - pTUBuf), pFrameUnit, frame_unit_size, m_obu_parse_params.ActiveFrameParams.frame_type, fu_obus);
				
				fu_obus.clear();

				// Update the start pointer of next frame unit
				pFrameUnit = pFrameUnit + frame_unit_size;
				bCompleteFUFound = false;
			}
		}

		fu_obus.emplace_back((uint32_t)(pOBUBuf - pTUBuf), pOBUBuf, obu_length, obu_type, obu_size, m_obu_parse_params);

		if (obu_type == OBU_TEMPORAL_DELIMITER)
		{
			m_obu_parse_params.ActiveFrameParams.SeenFrameHeader = false;
		}
		else if (obu_type == OBU_SEQUENCE_HEADER)
		{
			// 7.5. Ordering of OBUs
			//	A new coded video sequence is defined to start at each temporal unit which satisfies both of the following conditions :
			//		• A sequence header OBU appears before the first frame header.
			bool bHitFirstSeqHdrOBU = m_pCtx->GetSeqHdrOBU() == nullptr ? true : false;
			
			// Update the Sequence Header OBU to the AV1 context
			if (AMP_FAILED(iRet = UpdateSeqHdrToContext(pOBUBuf, obu_length)))
			{
				iRet = RET_CODE_ABORT;
				goto done;
			}

			if (iRet == RET_CODE_SUCCESS)
			{
				if (bHitFirstSeqHdrOBU)
					bNewGOP = true;
				bNewVSeq = true;
			}
		}
		else if (obu_type == OBU_FRAME_HEADER || obu_type == OBU_FRAME || obu_type == OBU_REDUNDANT_FRAME_HEADER)
		{
			if ((iRet = UpdateOBUParsePreconditionParams(p, cbLeft, obu_hdr)) == RET_CODE_ABORT)
			{
				iRet = RET_CODE_ABORT;
				goto done;
			}

			// 7.5. Ordering of OBUs
			//	A new coded video sequence is defined to start at each temporal unit which satisfies both of the following conditions :
			//		• A sequence header OBU appears before the first frame header.
			//		• The first frame header has frame_type equal to KEY_FRAME, show_frame equal to 1, show_existing_frame
			//		  equal to 0, and temporal_id equal to 0.
			if (!obu_hdr.obu_extension_flag || (obu_hdr.obu_extension_flag && obu_hdr.obu_extension_header.temporal_id == 0))
			{
				if (!m_obu_parse_params.ActiveFrameParams.show_existing_frame && 
					 m_obu_parse_params.ActiveFrameParams.show_frame &&
					 m_obu_parse_params.ActiveFrameParams.frame_type == KEY_FRAME)
					bNewGOP = true;
			}

			if (m_obu_parse_params.ActiveFrameParams.SeenFrameHeader == false)
			{
				// a frame is already finished
				bCompleteFUFound = true;

				// Try to update the frame type of frame-unit
				if (tu_frame_type == INT32_MIN)
					tu_frame_type = m_obu_parse_params.ActiveFrameParams.frame_type;
				else if (tu_frame_type != m_obu_parse_params.ActiveFrameParams.frame_type)
					tu_frame_type = -1;
			}
		}
	}

	// Form the left OBUs as a frame unit whenever the last tile is hit or not
	if (fu_obus.size() > 0)
	{
		// calculate the frame unit size
		frame_unit_size = (uint32_t)(pTUBuf + cbTUBuf - pFrameUnit);

		if (tu_frame_type == INT32_MIN)
			tu_frame_type = m_obu_parse_params.ActiveFrameParams.frame_type;
		else if (tu_frame_type != m_obu_parse_params.ActiveFrameParams.frame_type)
			tu_frame_type = -1;

		tu_fu_obus.emplace_back((uint32_t)(pFrameUnit - pTUBuf), pFrameUnit, frame_unit_size, m_obu_parse_params.ActiveFrameParams.frame_type, fu_obus);
	}

	// prepare the notification for the current TU
	if (m_av1_enum != nullptr && AMP_SUCCEEDED(iRet))
	{
		int32_t tu_fu_idx = 0;
		if (bNewVSeq && (m_av1_enum_options&AV1_ENUM_OPTION_VSEQ) && AMP_FAILED(m_av1_enum->EnumNewVSEQ(m_pCtx)))
		{
			iRet = RET_CODE_ABORT;
			goto done;
		}

		if (bNewGOP && (m_av1_enum_options&AV1_ENUM_OPTION_CVS) && AMP_FAILED(m_av1_enum->EnumNewCVS(m_pCtx, 0)))
		{
			iRet = RET_CODE_ABORT;
			goto done;
		}

		if ((m_av1_enum_options&AV1_ENUM_OPTION_TU) && AMP_FAILED(m_av1_enum->EnumTemporalUnitStart(m_pCtx, pTUBuf + (m_bIVF ? 12 : 0), (uint32_t)cbTUBuf, tu_frame_type)))
		{
			iRet = RET_CODE_ABORT;
			goto done;
		}

		for (auto& fu : tu_fu_obus)
		{
			auto pFrameUnit = std::get<1>(fu);
			auto frame_unit_size = std::get<2>(fu);

			if ((m_av1_enum_options&AV1_ENUM_OPTION_FU) && AMP_FAILED(m_av1_enum->EnumFrameUnitStart(m_pCtx, pFrameUnit, frame_unit_size, std::get<3>(fu))))
			{
				iRet = RET_CODE_ABORT;
				goto done;
			}

			for (auto& obu : std::get<4>(fu))
			{
				if ((m_av1_enum_options&AV1_ENUM_OPTION_OBU) && obu_length != UINT32_MAX && obu_length > 0)
				{
#ifdef ENABLE_LOAD_CONTEXT_SNAPSHOT
					auto& obu_params = std::get<5>(obu);
					AV1ContextSnapshot ctx_snapshot;
					ctx_snapshot.tu_idx = m_num_temporal_units;
					ctx_snapshot.tu_fu_idx = tu_fu_idx;
					ctx_snapshot.pVBISlotParams = obu_params.VBI;
					ctx_snapshot.pActiveFrameParams = &obu_params.ActiveFrameParams;

					if (g_verbose_level > 200)
					{
						printf("[AV1Parser] SeenFrameHeader: %d.\n", obu_params.ActiveFrameParams.SeenFrameHeader);
						printf("[AV1Parser] VBI [%" PRId32 ",%" PRId32 ",%" PRId32 ",%" PRId32 ",%" PRId32 ",%" PRId32 ",%" PRId32 ",%" PRId32 "]\n",
							obu_params.VBI[0].FrameSeqID, obu_params.VBI[0].FrameSeqID, obu_params.VBI[0].FrameSeqID, obu_params.VBI[0].FrameSeqID,
							obu_params.VBI[0].FrameSeqID, obu_params.VBI[0].FrameSeqID, obu_params.VBI[0].FrameSeqID, obu_params.VBI[0].FrameSeqID);
					}

					if (AMP_FAILED(m_pCtx->LoadSnapshot(&ctx_snapshot)))
					{
						printf("[AV1Parser] Failed to load context snapshot for OBU parsing.\n");
					}
#endif

					if (AMP_FAILED(m_av1_enum->EnumOBU(m_pCtx, std::get<1>(obu), std::get<2>(obu), std::get<3>(obu), std::get<4>(obu))))
					{
						iRet = RET_CODE_ABORT;
						goto done;
					}
				}
			}

			if ((m_av1_enum_options&AV1_ENUM_OPTION_FU) && AMP_FAILED(m_av1_enum->EnumFrameUnitEnd(m_pCtx, pFrameUnit, frame_unit_size)))
			{
				iRet = RET_CODE_ABORT;
				goto done;
			}

			tu_fu_idx++;
		}

		if ((m_av1_enum_options&AV1_ENUM_OPTION_TU) && AMP_FAILED(m_av1_enum->EnumTemporalUnitEnd(m_pCtx, pTUBuf + (m_bIVF ? 12 : 0), (uint32_t)cbTUBuf)))
		{
			iRet = RET_CODE_ABORT;
			goto done;
		}
	}

	m_num_temporal_units++;

done:
	AM_LRB_Reset(m_rbTemporalUnit);
	return iRet;
}

RET_CODE CAV1Parser::UpdateSeqHdrToContext(const uint8_t* pOBUBuf, uint32_t cbOBUBuf)
{
	int iRet = RET_CODE_NOTHING_TODO;
	AMBst in_bst = nullptr;
	// Try to update the sequence header
	SHA1HashVaue currHashValue(pOBUBuf, cbOBUBuf);
	auto spSeqHdr = m_pCtx->GetSeqHdrOBU();
	if (spSeqHdr == nullptr || (currHashValue.fValid && spSeqHdr->obu_hash_value != currHashValue))
	{
		// New VideoSequence has been found?
		BST::AV1::OPEN_BITSTREAM_UNIT* ptr_obu_seq_hdr = new BST::AV1::OPEN_BITSTREAM_UNIT(nullptr);
		if (ptr_obu_seq_hdr == nullptr) {
			iRet = RET_CODE_OUTOFMEMORY;;
			goto done;
		}

		AV1_OBU sp_obu_seq_hdr = AV1_OBU(ptr_obu_seq_hdr);
		in_bst = AMBst_CreateFromBuffer((uint8_t*)pOBUBuf, (int)cbOBUBuf);
		if (in_bst == nullptr) {
			iRet = RET_CODE_OUTOFMEMORY;
			goto done;
		}

		if (AMP_SUCCEEDED(iRet = sp_obu_seq_hdr->Map(in_bst)))
		{
			if (!sp_obu_seq_hdr->obu_header.obu_extension_flag ||
				(sp_obu_seq_hdr->obu_header.obu_extension_flag && sp_obu_seq_hdr->obu_header.obu_extension_header.temporal_id == 0))
			{
				sp_obu_seq_hdr->obu_hash_value.UpdateHash(pOBUBuf, cbOBUBuf);
				m_pCtx->UpdateSeqHdrOBU(sp_obu_seq_hdr);
				iRet = RET_CODE_SUCCESS;
			}
			else
				iRet = RET_CODE_NOTHING_TODO;
		}
	}

done:
	AMBst_Destroy(in_bst);
	return iRet;
}

RET_CODE CAV1Parser::UpdateOBUParsePreconditionParams(const uint8_t* pOBUBodyBuf, size_t cbOBUBodyBuf, BST::AV1::OBU_HEADER& obu_hdr)
{
	/*
		The input parameter 'parse_params' is the parameter set of the previous frame OBU
		After finishing this function successfully, the current OBU context parameter will be updated into it
	*/
	int iRet = RET_CODE_SUCCESS;

	auto spSeqHdr = m_pCtx->GetSeqHdrOBU();
	if (spSeqHdr == nullptr || spSeqHdr->ptr_sequence_header_obu == nullptr)
		return RET_CODE_HEADER_LOST;

	BITBUF bitbuf(pOBUBodyBuf, cbOBUBodyBuf);

	if (m_obu_parse_params.ActiveFrameParams.SeenFrameHeader)
	{
		// Re-use the current active frame params
	}
	else
	{
		// parsing uncompressed_header()
		m_obu_parse_params.ActiveFrameParams.SeenFrameHeader = true;
		if (AMP_FAILED(iRet = ParseUncompressedHeader(bitbuf, obu_hdr, spSeqHdr)))
		{
			printf("[AV1Parse] Failed to parse uncompressed_header() {error code: %d}.\n", iRet);
			return iRet;
		}

		if (m_obu_parse_params.ActiveFrameParams.show_existing_frame)
		{
			// decode_frame_wrapup()
			m_obu_parse_params.ActiveFrameParams.SeenFrameHeader = false;

			m_obu_parse_params.UpdateRefreshFrame(m_obu_parse_params.VBI[m_obu_parse_params.ActiveFrameParams.frame_to_show_map_idx].FrameSeqID);

			if (g_verbose_level > 0)
				printf("Finished decoding a frame by re-using the previous frame.\n");
		}
		else
		{
			m_obu_parse_params.ActiveFrameParams.TileNum = 0;
			m_obu_parse_params.ActiveFrameParams.SeenFrameHeader = true;
		}
	}

	if (obu_hdr.obu_type == OBU_FRAME)
	{
		if (AMP_FAILED(iRet = bitbuf.ByteAlign()))goto done;

		if (bitbuf.cbBuf > 0)
		{
			bool tile_start_and_end_present_flag = false;
			// process a part of tile_group_obu
			uint32_t NumTiles = m_obu_parse_params.ActiveFrameParams.TileCols*m_obu_parse_params.ActiveFrameParams.TileRows;
			uint32_t tg_start, tg_end;
			if (NumTiles > 1 && AMP_FAILED(iRet = bitbuf.GetFlag(tile_start_and_end_present_flag)))goto done;

			if (NumTiles == 1 || !tile_start_and_end_present_flag) {
				tg_start = 0;
				tg_end = NumTiles - 1;
			}
			else
			{
				uint8_t tileBits = m_obu_parse_params.ActiveFrameParams.TileColsLog2 + m_obu_parse_params.ActiveFrameParams.TileRowsLog2;
				if (AMP_FAILED(iRet = bitbuf.GetValue(tileBits, tg_start)))goto done;
				if (AMP_FAILED(iRet = bitbuf.GetValue(tileBits, tg_end)))goto done;
			}

			if (AMP_FAILED(iRet = bitbuf.ByteAlign()))goto done;

			// Don't parse any more, and calculate whether it is the last tile or not
			if (tg_end == NumTiles - 1) {
				/*
				if (!disable_frame_end_update_cdf) {
					frame_end_update_cdf()
				}
				decode_frame_wrapup()
				*/
				m_obu_parse_params.ActiveFrameParams.SeenFrameHeader = false;

				m_obu_parse_params.UpdateRefreshFrame(m_next_frame_seq_id);

				m_next_frame_seq_id = ((uint32_t)m_next_frame_seq_id + 1) % ((uint32_t)UINT16_MAX + 1);

				if (g_verbose_level > 0)
					printf("[AV1Parser] Finished decoding a frame after decoding the last tile.\n");
			}
		}
		
	}

done:
	return iRet;
}

RET_CODE CAV1Parser::ParseUncompressedHeader(BITBUF& bitbuf, BST::AV1::OBU_HEADER& obu_hdr, AV1_OBU spSeqHdrOBU)
{
	bool FrameIsIntra = false;
	bool show_existing_frame = false;
	bool show_frame = true;
	bool showable_frame = false;
	bool error_resilient_mode = true;
	int8_t ref_frame_idx[REFS_PER_FRAME] = {-1, -1, -1, -1, -1, -1, -1};

	auto pSeqHdrOBU = spSeqHdrOBU->ptr_sequence_header_obu;

	int iRet = RET_CODE_SUCCESS;
	uint8_t idLen = pSeqHdrOBU->additional_frame_id_length_minus_1 +
		pSeqHdrOBU->delta_frame_id_length_minus_2 + 3;

	uint32_t allFrames = (1 << NUM_REF_FRAMES) - 1;
	bool disable_cdf_update;
	bool allow_screen_content_tools;
	bool force_integer_mv;
	bool frame_size_override_flag;
	uint8_t primary_ref_frame;
	bool allow_intrabc = false;
	bool disable_frame_end_update_cdf;
	uint8_t base_q_idx;
	int8_t DeltaQYDc;
	uint8_t NumPlanes = pSeqHdrOBU->color_config->mono_chrome ? 1 : 3;
	bool diff_uv_delta = false;
	int8_t DeltaQUDc = 0, DeltaQUAc = 0, DeltaQVDc = 0, DeltaQVAc = 0;
	bool using_qmatrix;
	bool segmentation_enabled = false;
	bool segmentation_update_map = true;;
	bool segmentation_temporal_update = false;
	bool segmentation_update_data = true;
	bool FeatureEnabled[MAX_SEGMENTS][SEG_LVL_MAX] = { {0} };
	int16_t FeatureData[MAX_SEGMENTS][SEG_LVL_MAX] = { {0} };
	int64_t feature_value = 0;
	bool delta_q_present = false;
	bool delta_lf_present = false;
	bool CodedLossless = true;
	bool LosslessArray[MAX_SEGMENTS];
	bool AllLossless;
	uint8_t loop_filter_level[4] = { 0 };
	bool UsesLr = false, usesChromaLr = false;
	bool reference_select = false;
	bool skipModeAllowed, skip_mode_present;
	bool allow_warped_motion;
	bool reduced_tx_set;
	bool allow_high_precision_mv = false;

	if (pSeqHdrOBU->reduced_still_picture_header)
	{
		m_obu_parse_params.ActiveFrameParams.show_existing_frame = false;
		m_obu_parse_params.ActiveFrameParams.show_frame = true;
		m_obu_parse_params.ActiveFrameParams.frame_type = KEY_FRAME;
		FrameIsIntra = true;
	}
	else
	{
		if (AMP_FAILED(iRet = bitbuf.GetFlag(show_existing_frame)))goto done;

		m_obu_parse_params.ActiveFrameParams.show_existing_frame = show_existing_frame;

		if (show_existing_frame == 1)
		{
			uint8_t frame_to_show_map_idx;
			if (AMP_FAILED(iRet = bitbuf.GetValue(3, frame_to_show_map_idx)))goto done;

			m_obu_parse_params.ActiveFrameParams.frame_to_show_map_idx = frame_to_show_map_idx;

			if (pSeqHdrOBU->decoder_model_info_present_flag &&
				!pSeqHdrOBU->ptr_timing_info->equal_picture_interval)
			{
				uint8_t frame_presentation_time_length =
					(uint8_t)(pSeqHdrOBU->ptr_decoder_model_info->frame_presentation_time_length_minus_1 + 1);
				bitbuf.SkipFast(frame_presentation_time_length);
			}

			m_obu_parse_params.ActiveFrameParams.refresh_frame_flags = 0;
			// check uncompressed_header partly
			if (pSeqHdrOBU->frame_id_numbers_present_flag) {
				bitbuf.SkipFast(idLen);	// skip display_frame_id
			}

			m_obu_parse_params.ActiveFrameParams.frame_type = m_obu_parse_params.VBI[frame_to_show_map_idx].RefFrameType;
			if (m_obu_parse_params.ActiveFrameParams.frame_type == KEY_FRAME)
				m_obu_parse_params.ActiveFrameParams.refresh_frame_flags = allFrames;

			return RET_CODE_SUCCESS;
		}
		
		if (AMP_FAILED(iRet = bitbuf.GetValue(2, m_obu_parse_params.ActiveFrameParams.frame_type)))goto done;

		FrameIsIntra = (m_obu_parse_params.ActiveFrameParams.frame_type == INTRA_ONLY_FRAME ||
						m_obu_parse_params.ActiveFrameParams.frame_type == KEY_FRAME);

		if (AMP_FAILED(iRet = bitbuf.GetFlag(show_frame)))goto done;

		m_obu_parse_params.ActiveFrameParams.show_frame = show_frame;

		if (show_frame && pSeqHdrOBU->decoder_model_info_present_flag && !pSeqHdrOBU->ptr_timing_info->equal_picture_interval)
		{
			// temporal_point_info()
			uint8_t frame_presentation_time_length =
				(uint8_t)(pSeqHdrOBU->ptr_decoder_model_info->frame_presentation_time_length_minus_1 + 1);
			if (AMP_FAILED(iRet = bitbuf.Skip(frame_presentation_time_length)))goto done;
		}

		showable_frame = m_obu_parse_params.ActiveFrameParams.frame_type != KEY_FRAME;
		if (!show_frame && AMP_FAILED(iRet = bitbuf.GetFlag(showable_frame)))goto done;

		if (m_obu_parse_params.ActiveFrameParams.frame_type == SWITCH_FRAME || (m_obu_parse_params.ActiveFrameParams.frame_type == KEY_FRAME && show_frame))
			error_resilient_mode = true;
		else if (AMP_FAILED(iRet = bitbuf.GetFlag(error_resilient_mode)))
			goto done;
	}

	// Key frame for random access point
	// Reset the VBI slot parameters
	if (m_obu_parse_params.ActiveFrameParams.frame_type == KEY_FRAME && show_frame)
	{
		for (uint8_t i = 0; i < NUM_REF_FRAMES; i++) {
			m_obu_parse_params.VBI[i].FrameSeqID = -1;
			m_obu_parse_params.VBI[i].RefValid = 0;
			m_obu_parse_params.VBI[i].RefOrderHint = 0;
		}

		for (uint8_t i = 0; i < REFS_PER_FRAME; i++) {
			m_obu_parse_params.ActiveFrameParams.OrderHints[i] = 0;
		}
	}

	if (AMP_FAILED(iRet = bitbuf.GetFlag(disable_cdf_update)))goto done;

	if (pSeqHdrOBU->seq_force_screen_content_tools == SELECT_SCREEN_CONTENT_TOOLS) {
		if (AMP_FAILED(iRet = bitbuf.GetFlag(allow_screen_content_tools)))goto done;
	}
	else
		allow_screen_content_tools = pSeqHdrOBU->seq_force_screen_content_tools;

	if (allow_screen_content_tools)
	{
		if (pSeqHdrOBU->seq_force_integer_mv == SELECT_INTEGER_MV) {
			if (AMP_FAILED(iRet = bitbuf.GetFlag(force_integer_mv)))goto done;
		}
		else
			force_integer_mv = pSeqHdrOBU->seq_force_integer_mv;
	}
	else
		force_integer_mv = false;

	if (FrameIsIntra)
		force_integer_mv = true;

	if (pSeqHdrOBU->frame_id_numbers_present_flag) {
		m_obu_parse_params.ActiveFrameParams.PrevFrameID = m_obu_parse_params.ActiveFrameParams.current_frame_id;
		if (AMP_FAILED(iRet = bitbuf.GetValue(idLen, m_obu_parse_params.ActiveFrameParams.current_frame_id)))goto done;
		mark_ref_frames(spSeqHdrOBU, idLen, m_obu_parse_params);
	}
	else
		m_obu_parse_params.ActiveFrameParams.current_frame_id = 0;

	if (m_obu_parse_params.ActiveFrameParams.frame_type == SWITCH_FRAME)
		frame_size_override_flag = 1;
	else if (pSeqHdrOBU->reduced_still_picture_header)
		frame_size_override_flag = 0;
	else if (AMP_FAILED(iRet = bitbuf.GetFlag(frame_size_override_flag)))
		goto done;

	if (AMP_FAILED(iRet = bitbuf.GetValue(pSeqHdrOBU->OrderHintBits, m_obu_parse_params.ActiveFrameParams.OrderHint)))goto done;

	if (FrameIsIntra || error_resilient_mode)
		primary_ref_frame = PRIMARY_REF_NONE;
	else if (AMP_FAILED(iRet = bitbuf.GetValue(3, primary_ref_frame)))goto done;

	if (pSeqHdrOBU->decoder_model_info_present_flag)
	{
		bool buffer_removal_time_present_flag;
		if (AMP_FAILED(iRet = bitbuf.GetFlag(buffer_removal_time_present_flag)))goto done;

		if (buffer_removal_time_present_flag)
		{
			for (uint8_t opNum = 0; opNum <= pSeqHdrOBU->operating_points_cnt_minus_1; opNum++)
			{
				if (pSeqHdrOBU->operating_points[opNum].decoder_model_present_for_this_op)
				{
					uint8_t temporal_id = obu_hdr.obu_extension_flag ? obu_hdr.obu_extension_header.temporal_id : 0;
					uint8_t spatial_id = obu_hdr.obu_extension_flag ? obu_hdr.obu_extension_header.spatial_id : 0;
					uint32_t opPtIdc = pSeqHdrOBU->operating_points[opNum].operating_point_idc;
					uint32_t inTemporalLayer = (opPtIdc >> temporal_id) & 1;
					uint32_t inSpatialLayer = (opPtIdc >> (spatial_id + 8)) & 1;
					if (opPtIdc == 0 || (inTemporalLayer && inSpatialLayer))
						bitbuf.SkipFast((size_t)(pSeqHdrOBU->ptr_decoder_model_info->buffer_removal_time_length_minus_1 + 1));
				}
			}
		}
	}

	if (m_obu_parse_params.ActiveFrameParams.frame_type == SWITCH_FRAME || (m_obu_parse_params.ActiveFrameParams.frame_type == KEY_FRAME && show_frame))
		m_obu_parse_params.ActiveFrameParams.refresh_frame_flags = allFrames;
	else if (AMP_FAILED(iRet = bitbuf.GetByte(m_obu_parse_params.ActiveFrameParams.refresh_frame_flags)))
		goto done;

	if (!FrameIsIntra || m_obu_parse_params.ActiveFrameParams.refresh_frame_flags != allFrames)
	{
		if (error_resilient_mode && pSeqHdrOBU->enable_order_hint)
		{
			for (uint8_t i = 0; i < NUM_REF_FRAMES; i++)
			{
				int8_t ref_order_hint;
				if (AMP_FAILED(iRet = bitbuf.GetValue(pSeqHdrOBU->OrderHintBits, ref_order_hint)))goto done;
				if (ref_order_hint != m_obu_parse_params.VBI[i].RefOrderHint)
					m_obu_parse_params.VBI[i].RefValid = 0;
			}
		}
	}

	if (FrameIsIntra)
	{
		// Skip frame_size()
		if (AMP_FAILED(iRet = frame_size(spSeqHdrOBU, bitbuf, m_obu_parse_params, frame_size_override_flag)))goto done;

		// Skip render_size()
		if (AMP_FAILED(iRet = render_size(spSeqHdrOBU, bitbuf, m_obu_parse_params)))goto done;

		if (allow_screen_content_tools && m_obu_parse_params.ActiveFrameParams.UpscaledWidth == m_obu_parse_params.ActiveFrameParams.FrameWidth) {
			if (AMP_FAILED(iRet = bitbuf.GetFlag(allow_intrabc)))goto done;
		}
	}
	else
	{
		bool frame_refs_short_signaling = false;
		if (pSeqHdrOBU->enable_order_hint && AMP_FAILED(iRet = bitbuf.GetFlag(frame_refs_short_signaling)))goto done;

		int8_t last_frame_idx = -1, gold_frame_idx = -1;
		if (frame_refs_short_signaling) {
			if (AMP_FAILED(iRet = bitbuf.GetValue(3, last_frame_idx)))goto done;
			if (AMP_FAILED(iRet = bitbuf.GetValue(3, gold_frame_idx)))goto done;

			if (AMP_FAILED(iRet = set_frame_refs(spSeqHdrOBU, m_obu_parse_params, last_frame_idx, gold_frame_idx, ref_frame_idx)))goto done;
		}

		for (uint8_t i = 0; i < REFS_PER_FRAME; i++)
		{
			if (!frame_refs_short_signaling) {
				if (AMP_FAILED(iRet = bitbuf.GetValue(3, ref_frame_idx[i])))goto done;
			}

			if (pSeqHdrOBU->frame_id_numbers_present_flag) {
				bitbuf.SkipFast((size_t)pSeqHdrOBU->delta_frame_id_length_minus_2 + 2);	// skip delta_frame_id_minus_1
			}
		}

		if (frame_size_override_flag && !error_resilient_mode)
		{
			if (AMP_FAILED(iRet = frame_size_with_refs(spSeqHdrOBU, bitbuf, m_obu_parse_params, frame_size_override_flag, ref_frame_idx)))goto done;
		}
		else
		{
			// Skip frame_size()
			if (AMP_FAILED(iRet = frame_size(spSeqHdrOBU, bitbuf, m_obu_parse_params, frame_size_override_flag)))goto done;

			// Skip render_size()
			if (AMP_FAILED(iRet = render_size(spSeqHdrOBU, bitbuf, m_obu_parse_params)))goto done;
		}

		if (!force_integer_mv && AMP_FAILED(iRet = bitbuf.GetFlag(allow_high_precision_mv)))goto done;

		// Skip read_interpolation_filter()
		bool is_filter_switchable = false;
		if (AMP_FAILED(iRet = bitbuf.GetFlag(is_filter_switchable)))goto done;

		uint8_t interpolation_filter;
		if (is_filter_switchable)
			interpolation_filter = SWITCHABLE;
		else if (AMP_FAILED(iRet = bitbuf.GetValue(2, interpolation_filter)))goto done;

		bool is_motion_mode_switchable = false;
		if (AMP_FAILED(iRet = bitbuf.GetFlag(is_motion_mode_switchable)))goto done;

		bool use_ref_frame_mvs;
		if (error_resilient_mode || !pSeqHdrOBU->enable_ref_frame_mvs)
			use_ref_frame_mvs = false;
		else if (AMP_FAILED(iRet = bitbuf.GetFlag(use_ref_frame_mvs)))goto done;

		for (uint8_t i = 0; i < REFS_PER_FRAME; i++) {
			uint8_t refFrame = LAST_FRAME + i;
			auto hint = m_obu_parse_params.VBI[ref_frame_idx[i]].RefOrderHint;
			m_obu_parse_params.ActiveFrameParams.OrderHints[i] = hint;

			if ( !spSeqHdrOBU->ptr_sequence_header_obu->enable_order_hint ) {
				m_obu_parse_params.VBI[ref_frame_idx[i]].RefFrameSignBias &= ~(1 << i);
			} else {
				bool bBackward = spSeqHdrOBU->ptr_sequence_header_obu->get_relative_dist(hint, m_obu_parse_params.ActiveFrameParams.OrderHint) > 0;
				if (bBackward)
					m_obu_parse_params.VBI[ref_frame_idx[i]].RefFrameSignBias |= 1 << i;
				else
					m_obu_parse_params.VBI[ref_frame_idx[i]].RefFrameSignBias &= ~(1 << i);
			}
		}
	}	// end if (FrameIsIntra)

	if (pSeqHdrOBU->reduced_still_picture_header || disable_cdf_update)
		disable_frame_end_update_cdf = true;
	else if (AMP_FAILED(iRet = bitbuf.GetFlag(disable_frame_end_update_cdf)))goto done;

	if (primary_ref_frame == PRIMARY_REF_NONE) {
		// init_non_coeff_cdfs()
		m_obu_parse_params.setup_past_independence();
	}
	else
	{
		// load_cdfs(ref_frame_idx[primary_ref_frame])
		// load_previous()
		m_obu_parse_params.load_previous(ref_frame_idx[primary_ref_frame]);
	}

	// Skip tile_info
	if (AMP_FAILED(iRet = tile_info(spSeqHdrOBU, bitbuf, m_obu_parse_params)))goto done;

	// Skip quantization_params()
	if (AMP_FAILED(iRet = bitbuf.GetByte(base_q_idx)))goto done;

	if (AMP_FAILED(iRet = read_delta_q(bitbuf, DeltaQYDc)))goto done;

	if (NumPlanes > 1) {
		if (pSeqHdrOBU->color_config->separate_uv_delta_q && AMP_FAILED(iRet = bitbuf.GetFlag(diff_uv_delta)))goto done;
		if (AMP_FAILED(iRet = read_delta_q(bitbuf, DeltaQUDc)))goto done;
		if (AMP_FAILED(iRet = read_delta_q(bitbuf, DeltaQUAc)))goto done;
		if (diff_uv_delta)
		{
			if (AMP_FAILED(iRet = read_delta_q(bitbuf, DeltaQVDc)))goto done;
			if (AMP_FAILED(iRet = read_delta_q(bitbuf, DeltaQVAc)))goto done;
		}
		else
		{
			DeltaQVDc = DeltaQUDc;
			DeltaQVAc = DeltaQUAc;
		}
	}
	
	if (AMP_FAILED(iRet = bitbuf.GetFlag(using_qmatrix)))goto done;
	if (using_qmatrix)
	{
		bitbuf.SkipFast(4);	// qm_y
		bitbuf.SkipFast(4);	// qm_u
		if (pSeqHdrOBU->color_config->separate_uv_delta_q)
			bitbuf.SkipFast(4);	// qm_v
	}

	// Skip segmentation_params()
	if (AMP_FAILED(iRet = bitbuf.GetFlag(segmentation_enabled)))goto done;
	if (segmentation_enabled)
	{
		if (primary_ref_frame != PRIMARY_REF_NONE)
		{
			if (AMP_FAILED(iRet = bitbuf.GetFlag(segmentation_update_map)))goto done;
			if (segmentation_update_data) {
				if (AMP_FAILED(iRet = bitbuf.GetFlag(segmentation_temporal_update)))
					goto done;
			}
			if (AMP_FAILED(iRet = bitbuf.GetFlag(segmentation_update_data)))goto done;
		}

		if (segmentation_update_data == 1) {
			for (uint8_t i = 0; i < MAX_SEGMENTS; i++) {
				for (uint8_t j = 0; j < SEG_LVL_MAX; j++) {
					feature_value = 0;
					bool feature_enabled;
					if (AMP_FAILED(iRet = bitbuf.GetFlag(feature_enabled))) goto done;
					FeatureEnabled[i][j] = feature_enabled;
					int16_t clippedValue = 0;
					if (feature_enabled)
					{
						auto bitsToRead = Segmentation_Feature_Bits[j];
						auto limit = Segmentation_Feature_Max[j];
						if (Segmentation_Feature_Signed[j] == 1)
						{
							int16_t feature_value;
							if (AMP_FAILED(iRet = bitbuf.AV1su(1 + bitsToRead, feature_value))) goto done;
							clippedValue = AV1_Clip3(-limit, limit, feature_value);
						}
						else
						{
							uint16_t feature_value;
							if (AMP_FAILED(iRet = bitbuf.GetValue(bitsToRead, feature_value))) goto done;
							clippedValue = AV1_Clip3(0, limit, feature_value);
						}
					}
					FeatureData[i][j] = clippedValue;
				}
			}
		}
	}

	// delta_q_params()
	if (base_q_idx > 0 && AMP_FAILED(iRet = bitbuf.GetFlag(delta_q_present)))goto done;
	if (delta_q_present)bitbuf.SkipFast(2);

	// delta_lf_params()
	
	if (delta_q_present) {
		if (!allow_intrabc) {
			if (AMP_FAILED(iRet = bitbuf.GetFlag(delta_lf_present)))
				goto done;
		}

		if (delta_lf_present) {
			bitbuf.SkipFast(2);	// delta_lf_res
			bitbuf.SkipFast(1);	// delta_lf_multi
		}
	}

	for (uint8_t segmentId = 0; segmentId < MAX_SEGMENTS; segmentId++) {
		int qindex = base_q_idx;
		if (segmentation_enabled && FeatureEnabled[segmentId][SEG_LVL_ALT_Q])
		{
			const int data = FeatureData[segmentId][SEG_LVL_ALT_Q];
			const int seg_qindex = base_q_idx + data;
			qindex = AV1_Clip3(0, 255, seg_qindex);
		}
		LosslessArray[segmentId] = qindex == 0 && DeltaQYDc == 0 &&
			DeltaQUAc == 0 && DeltaQUDc == 0 &&
			DeltaQVAc == 0 && DeltaQVDc == 0;

		if (!LosslessArray[segmentId])
			CodedLossless = 0;
	}

	AllLossless = CodedLossless && (m_obu_parse_params.ActiveFrameParams.FrameWidth == m_obu_parse_params.ActiveFrameParams.UpscaledWidth);

	// skip loop_filter_params()
	if (!(CodedLossless || allow_intrabc)) {
		if (AMP_FAILED(iRet = bitbuf.GetValue(6, loop_filter_level[0])))goto done;
		if (AMP_FAILED(iRet = bitbuf.GetValue(6, loop_filter_level[1])))goto done;
		if (NumPlanes > 1) {
			if (loop_filter_level[0] || loop_filter_level[1]) {
				if (AMP_FAILED(iRet = bitbuf.GetValue(6, loop_filter_level[2])))goto done;
				if (AMP_FAILED(iRet = bitbuf.GetValue(6, loop_filter_level[3])))goto done;
			}
		}

		bitbuf.SkipFast(3);	// loop_filter_sharpness
		bool loop_filter_delta_enabled;
		if (AMP_FAILED(iRet = bitbuf.GetFlag(loop_filter_delta_enabled)))goto done;
		int8_t loop_filter_ref_deltas[TOTAL_REFS_PER_FRAME];
		int8_t loop_filter_mode_deltas[2];
		if (loop_filter_delta_enabled) {
			bool loop_filter_delta_update;
			if (AMP_FAILED(iRet = bitbuf.GetFlag(loop_filter_delta_update)))goto done;
			if (loop_filter_delta_update == 1) {
				for (uint8_t i = 0; i < TOTAL_REFS_PER_FRAME; i++) {
					bool update_ref_delta;
					if (AMP_FAILED(iRet = bitbuf.GetFlag(update_ref_delta))) goto done;

					if (update_ref_delta) {
						if (AMP_FAILED(iRet = bitbuf.AV1su(7, loop_filter_ref_deltas[i]))) goto done;
					}
				}
				for (uint8_t i = 0; i < 2; i++) {
					bool update_mode_delta;
					if (AMP_FAILED(iRet = bitbuf.GetFlag(update_mode_delta))) goto done;
					if (update_mode_delta) {
						if (AMP_FAILED(iRet = bitbuf.AV1su(7, loop_filter_mode_deltas[i]))) goto done;
					}
				}
			}
		}
	}

	// skip cdef_params()
	if (!(CodedLossless || allow_intrabc || !pSeqHdrOBU->enable_cdef)) {
		uint8_t cdef_y_pri_strength[8] = { 0 };
		uint8_t cdef_y_sec_strength[8] = { 0 };
		uint8_t cdef_uv_pri_strength[8] = { 0 };
		uint8_t cdef_uv_sec_strength[8] = { 0 };
		// Skip cdef_damping_minus_3
		bitbuf.SkipFast(2);
		uint8_t cdef_bits;
		if (AMP_FAILED(iRet = bitbuf.GetValue(2, cdef_bits)))goto done;
		for (uint32_t i = 0; i < (uint32_t)(1UL << cdef_bits); i++) {
			//bitbuf.SkipFast(4);	// cdef_y_pri_strength[i]
			//bitbuf.SkipFast(2);	// cdef_y_sec_strength[i]
			if (AMP_FAILED(iRet = bitbuf.GetValue(4, cdef_y_pri_strength[i])))goto done;
			if (AMP_FAILED(iRet = bitbuf.GetValue(2, cdef_y_sec_strength[i])))goto done;
			if (NumPlanes > 1) {
				//bitbuf.SkipFast(4);	// cdef_uv_pri_strength[i]
				//bitbuf.SkipFast(2);	// cdef_uv_sec_strength[i]
				if (AMP_FAILED(iRet = bitbuf.GetValue(4, cdef_uv_pri_strength[i])))goto done;
				if (AMP_FAILED(iRet = bitbuf.GetValue(2, cdef_uv_sec_strength[i])))goto done;
			}
		}
	}

	// skip lr_params()
	if (!(AllLossless || allow_intrabc || !pSeqHdrOBU->enable_restoration)) {
		for (uint8_t i = 0; i < NumPlanes; i++) {
			uint8_t lr_type;
			if (AMP_FAILED(iRet = bitbuf.GetValue(2, lr_type))) goto done;
			if (Remap_Lr_Type[lr_type] != RESTORE_NONE) {
				UsesLr = true;
				if (i > 0)
					usesChromaLr = true;
			}
		}
		if (UsesLr) {
			uint8_t lr_unit_shift, lr_unit_extra_shift, lr_uv_shift;
			if (AMP_FAILED(iRet = bitbuf.GetValue(1, lr_unit_shift)))goto done;
			if (pSeqHdrOBU->use_128x128_superblock)
				lr_unit_shift++;
			else
			{
				if (lr_unit_shift) {
					if (AMP_FAILED(iRet = bitbuf.GetValue(1, lr_unit_extra_shift)))goto done;
					lr_unit_shift += lr_unit_extra_shift;
				}
			}

			if (pSeqHdrOBU->color_config->subsampling_x && pSeqHdrOBU->color_config->subsampling_y && usesChromaLr) {
				if (AMP_FAILED(iRet = bitbuf.GetValue(1, lr_uv_shift)))goto done;
			}
		}
	}

	// skip read_tx_mode()
	if (!CodedLossless)
		bitbuf.SkipFast(1);	// tx_mode_select

	// skip frame_reference_mode()
	if (!FrameIsIntra && AMP_FAILED(iRet = bitbuf.GetFlag(reference_select)))goto done;

	// skip skip_mode_params()
	if (FrameIsIntra || !reference_select || !pSeqHdrOBU->enable_order_hint) {
		skipModeAllowed = false;
	}
	else
	{
		int8_t forwardIdx = -1, backwardIdx = -1, forwardHint = -1, backwardHint = -1;
		for (int8_t i = 0; i < REFS_PER_FRAME; i++) {
			auto refHint = m_obu_parse_params.VBI[ref_frame_idx[i]].RefOrderHint;
			auto diff = spSeqHdrOBU->ptr_sequence_header_obu->get_relative_dist(refHint, m_obu_parse_params.ActiveFrameParams.OrderHint);
			if (diff < 0)
			{
				if (forwardIdx < 0 || spSeqHdrOBU->ptr_sequence_header_obu->get_relative_dist(refHint, forwardHint) > 0) {
					forwardIdx = i;
					forwardHint = refHint;
				}
			}
			else if (diff > 0)
			{
				if (backwardIdx < 0 || spSeqHdrOBU->ptr_sequence_header_obu->get_relative_dist(refHint, backwardHint) < 0) {
					backwardIdx = i;
					backwardHint = refHint;
				}
			}
		}

		if (forwardIdx < 0) {
			skipModeAllowed = false;
		}
		else if (backwardIdx >= 0) {
			skipModeAllowed = true;
		}
		else {
			int8_t secondForwardIdx = -1, secondForwardHint = -1;
			for (int8_t i = 0; i < REFS_PER_FRAME; i++) {
				auto refHint = m_obu_parse_params.VBI[ref_frame_idx[i]].RefOrderHint;
				if (spSeqHdrOBU->ptr_sequence_header_obu->get_relative_dist(refHint, forwardHint) < 0) {
					if (secondForwardIdx < 0 ||
						spSeqHdrOBU->ptr_sequence_header_obu->get_relative_dist(refHint, secondForwardHint) > 0) {
						secondForwardIdx = i;
						secondForwardHint = refHint;
					}
				}
			}

			if (secondForwardIdx < 0) {
				skipModeAllowed = false;
			}
			else {
				skipModeAllowed = true;
			}
		}
	}

	if (skipModeAllowed)
	{
		if (AMP_FAILED(iRet = bitbuf.GetFlag(skip_mode_present)))goto done;
	}
	else
		skip_mode_present = false;

	if (FrameIsIntra || error_resilient_mode || !pSeqHdrOBU->enable_warped_motion)
		allow_warped_motion = false;
	else if (AMP_FAILED(iRet = bitbuf.GetFlag(allow_warped_motion)))goto done;

	if (AMP_FAILED(iRet = bitbuf.GetFlag(reduced_tx_set)))goto done;

	// skip global_motion_params()
	if (AMP_FAILED(iRet = global_motion_params(spSeqHdrOBU, bitbuf, m_obu_parse_params, FrameIsIntra, allow_high_precision_mv)))goto done;

	// skip film_grain_params
	if (AMP_FAILED(iRet = film_grain_params(spSeqHdrOBU, bitbuf, m_obu_parse_params, show_frame, showable_frame)))goto done;

done:
	return iRet;
}

RET_CODE CAV1Parser::read_delta_q(BITBUF& bitbuf, int8_t& ret_delta_q)
{
	bool delta_coded;
	int iRet = RET_CODE_SUCCESS;
	if (AMP_FAILED(iRet = bitbuf.GetFlag(delta_coded)))
		return iRet;

	int8_t delta_q = 0;
	if (delta_coded && AMP_FAILED(iRet = bitbuf.AV1su(7, delta_q)))
		return iRet;

	ret_delta_q = delta_q;
	return iRet;
}

RET_CODE CAV1Parser::mark_ref_frames(AV1_OBU spSeqHdrOBU, uint8_t idLen, OBU_PARSE_PARAMS& obu_parse_params)
{
	uint8_t diffLen = spSeqHdrOBU->ptr_sequence_header_obu->delta_frame_id_length_minus_2 + 2;
	for (uint8_t i = 0; i < NUM_REF_FRAMES; i++) {
		if (obu_parse_params.ActiveFrameParams.current_frame_id > (1 << diffLen)) {
			if (obu_parse_params.VBI[i].RefFrameId > obu_parse_params.ActiveFrameParams.current_frame_id ||
				obu_parse_params.VBI[i].RefFrameId < (obu_parse_params.ActiveFrameParams.current_frame_id - (1 << diffLen)))
				obu_parse_params.VBI[i].RefValid = 0;
		}
		else {
			if (obu_parse_params.VBI[i].RefFrameId > obu_parse_params.ActiveFrameParams.current_frame_id &&
				obu_parse_params.VBI[i].RefFrameId < ((1 << idLen) + obu_parse_params.ActiveFrameParams.current_frame_id - (1 << diffLen)))
				obu_parse_params.VBI[i].RefValid = 0;
		}
	}
	return RET_CODE_SUCCESS;
}

RET_CODE CAV1Parser::frame_size(AV1_OBU spSeqHdrOBU, BITBUF& bitbuf, OBU_PARSE_PARAMS& obu_parse_params, bool frame_size_override_flag)
{
	int iRet = RET_CODE_SUCCESS;
	if (frame_size_override_flag)
	{
		uint32_t frame_width_minus_1 = 0, frame_height_minus_1 = 0;
		if (AMP_FAILED(iRet = bitbuf.GetValue(spSeqHdrOBU->ptr_sequence_header_obu->frame_width_bits_minus_1 + 1, frame_width_minus_1)))return iRet;
		if (AMP_FAILED(iRet = bitbuf.GetValue(spSeqHdrOBU->ptr_sequence_header_obu->frame_width_bits_minus_1 + 1, frame_width_minus_1)))return iRet;
		obu_parse_params.ActiveFrameParams.FrameWidth = frame_width_minus_1 + 1;
		obu_parse_params.ActiveFrameParams.FrameHeight = frame_height_minus_1 + 1;
	}
	else
	{
		obu_parse_params.ActiveFrameParams.FrameWidth = spSeqHdrOBU->ptr_sequence_header_obu->max_frame_width_minus_1 + 1;
		obu_parse_params.ActiveFrameParams.FrameHeight = spSeqHdrOBU->ptr_sequence_header_obu->max_frame_height_minus_1 + 1;
	}

	if (AMP_FAILED(iRet = superres_params(spSeqHdrOBU, bitbuf, obu_parse_params)))return iRet;
	if (AMP_FAILED(iRet = compute_image_size(spSeqHdrOBU, bitbuf, obu_parse_params)))return iRet;

	return iRet;
}

RET_CODE CAV1Parser::superres_params(AV1_OBU spSeqHdrOBU, BITBUF& bitbuf, OBU_PARSE_PARAMS& obu_parse_params)
{
	int iRet = RET_CODE_SUCCESS;
	bool use_superres = false;
	uint8_t SuperresDenom = SUPERRES_NUM;
	if (spSeqHdrOBU->ptr_sequence_header_obu->enable_superres) {
		if (AMP_FAILED(iRet = bitbuf.GetFlag(use_superres)))return iRet;
		if (use_superres)
		{
			uint8_t coded_denom;
			if (AMP_FAILED(iRet = bitbuf.GetValue(SUPERRES_DENOM_BITS, coded_denom)))return iRet;
			SuperresDenom = (uint8_t)((uint16_t)coded_denom + SUPERRES_DENOM_MIN);
		}
	}
	obu_parse_params.ActiveFrameParams.UpscaledWidth = obu_parse_params.ActiveFrameParams.FrameWidth;
	obu_parse_params.ActiveFrameParams.FrameWidth = (obu_parse_params.ActiveFrameParams.UpscaledWidth * SUPERRES_NUM + (SuperresDenom / 2)) / SuperresDenom;

	return iRet;
}

RET_CODE CAV1Parser::compute_image_size(AV1_OBU spSeqHdrOBU, BITBUF& bitbuf, OBU_PARSE_PARAMS& obu_parse_params)
{
	obu_parse_params.ActiveFrameParams.MiCols = 2 * ((obu_parse_params.ActiveFrameParams.FrameWidth + 7) >> 3);
	obu_parse_params.ActiveFrameParams.MiRows = 2 * ((obu_parse_params.ActiveFrameParams.FrameHeight + 7) >> 3);
	return RET_CODE_SUCCESS;
}

RET_CODE CAV1Parser::render_size(AV1_OBU spSeqHdrOBU, BITBUF& bitbuf, OBU_PARSE_PARAMS& obu_parse_params)
{
	int iRet = RET_CODE_SUCCESS;
	bool render_and_frame_size_different;
	if (AMP_FAILED(iRet = bitbuf.GetFlag(render_and_frame_size_different)))return iRet;
	if (render_and_frame_size_different) {
		uint16_t render_width_minus_1, render_height_minus_1;
		if (AMP_FAILED(iRet = bitbuf.GetValue(16, render_width_minus_1)))return iRet;
		if (AMP_FAILED(iRet = bitbuf.GetValue(16, render_height_minus_1)))return iRet;
		obu_parse_params.ActiveFrameParams.RenderWidth = render_width_minus_1 + 1;
		obu_parse_params.ActiveFrameParams.RenderHeight = render_height_minus_1 + 1;
	}
	else
	{
		obu_parse_params.ActiveFrameParams.RenderWidth = obu_parse_params.ActiveFrameParams.UpscaledWidth;
		obu_parse_params.ActiveFrameParams.RenderHeight = obu_parse_params.ActiveFrameParams.FrameHeight;
	}

	return iRet;
}

RET_CODE CAV1Parser::frame_size_with_refs(AV1_OBU spSeqHdrOBU, BITBUF& bitbuf, OBU_PARSE_PARAMS& obu_parse_params, bool frame_size_override_flag, int8_t ref_frame_idx[REFS_PER_FRAME])
{
	int iRet = RET_CODE_SUCCESS;
	bool found_ref = false;
	for (uint8_t i = 0; i < REFS_PER_FRAME; i++) {
		if (AMP_FAILED(iRet = bitbuf.GetFlag(found_ref)))return iRet;
		if (found_ref == 1) {
			obu_parse_params.ActiveFrameParams.UpscaledWidth	= obu_parse_params.VBI[ref_frame_idx[i]].RefUpscaledWidth;
			obu_parse_params.ActiveFrameParams.FrameWidth		= obu_parse_params.ActiveFrameParams.UpscaledWidth;
			obu_parse_params.ActiveFrameParams.FrameHeight	= obu_parse_params.VBI[ref_frame_idx[i]].RefFrameHeight;
			obu_parse_params.ActiveFrameParams.RenderWidth	= obu_parse_params.VBI[ref_frame_idx[i]].RefRenderWidth;
			obu_parse_params.ActiveFrameParams.RenderHeight	= obu_parse_params.VBI[ref_frame_idx[i]].RefRenderHeight;
			break;
		}
	}

	if (found_ref == false) {
		if (AMP_FAILED(iRet = frame_size(spSeqHdrOBU, bitbuf, obu_parse_params, frame_size_override_flag)))return iRet;
		if (AMP_FAILED(iRet = render_size(spSeqHdrOBU, bitbuf, obu_parse_params)))return iRet;
	}
	else {
		if (AMP_FAILED(iRet = superres_params(spSeqHdrOBU, bitbuf, obu_parse_params)))return iRet;
		if (AMP_FAILED(iRet = compute_image_size(spSeqHdrOBU, bitbuf, obu_parse_params)))return iRet;
	}

	return iRet;
}

RET_CODE CAV1Parser::set_frame_refs(
	AV1_OBU spSeqHdrOBU, OBU_PARSE_PARAMS& obu_parse_params,
	int8_t last_frame_idx, int8_t gold_frame_idx, int8_t ref_frame_idx[REFS_PER_FRAME])
{
	int iRet = RET_CODE_SUCCESS;
	bool usedFrame[REFS_PER_FRAME] = { 0 };
	int16_t shiftedOrderHints[NUM_REF_FRAMES] = { 0 };

	for (uint8_t i = 0; i < REFS_PER_FRAME; i++)
		ref_frame_idx[i] = -1;
	ref_frame_idx[LAST_FRAME - LAST_FRAME] = last_frame_idx;
	ref_frame_idx[GOLDEN_FRAME - LAST_FRAME] = gold_frame_idx;

	usedFrame[last_frame_idx] = true;
	usedFrame[gold_frame_idx] = true;

	uint8_t curFrameHint = (uint8_t)(1 << (spSeqHdrOBU->ptr_sequence_header_obu->OrderHintBits - 1));
	for (uint8_t i = 0; i < NUM_REF_FRAMES; i++)
		shiftedOrderHints[i] = curFrameHint + spSeqHdrOBU->ptr_sequence_header_obu->get_relative_dist(obu_parse_params.VBI[i].RefOrderHint, 
																									  obu_parse_params.ActiveFrameParams.OrderHint);

	int16_t lastOrderHint = shiftedOrderHints[last_frame_idx];
	int16_t goldOrderHint = shiftedOrderHints[gold_frame_idx];

	int16_t ref = BST::AV1::find_latest_backward(shiftedOrderHints, usedFrame, curFrameHint);

	if (ref >= 0) {
		ref_frame_idx[ALTREF_FRAME - LAST_FRAME] = (int8_t)ref;
		usedFrame[ref] = true;
	}

	ref = BST::AV1::find_earliest_backward(shiftedOrderHints, usedFrame, curFrameHint);
	if (ref >= 0) {
		ref_frame_idx[BWDREF_FRAME - LAST_FRAME] = (int8_t)ref;
		usedFrame[ref] = true;
	}

	ref = BST::AV1::find_earliest_backward(shiftedOrderHints, usedFrame, curFrameHint);
	if (ref >= 0) {
		ref_frame_idx[ALTREF2_FRAME - LAST_FRAME] = (int8_t)ref;
		usedFrame[ref] = true;
	}

	for (uint8_t i = 0; i < REFS_PER_FRAME - 2; i++) {
		auto refFrame = Ref_Frame_List[i];
		if (ref_frame_idx[refFrame - LAST_FRAME] < 0) {
			auto ref = BST::AV1::find_latest_forward(shiftedOrderHints, usedFrame, curFrameHint);
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

RET_CODE CAV1Parser::tile_info(AV1_OBU spSeqHdrOBU, BITBUF& bitbuf, OBU_PARSE_PARAMS& obu_parse_params)
{
	int iRet = RET_CODE_SUCCESS;
	auto pSeqHdrOBU = spSeqHdrOBU->ptr_sequence_header_obu;
	uint32_t sbCols = pSeqHdrOBU->use_128x128_superblock 
		? ((obu_parse_params.ActiveFrameParams.MiCols + 31) >> 5)
		: ((obu_parse_params.ActiveFrameParams.MiCols + 15) >> 4);
	uint32_t sbRows = pSeqHdrOBU->use_128x128_superblock 
		? ((obu_parse_params.ActiveFrameParams.MiRows + 31) >> 5) 
		: ((obu_parse_params.ActiveFrameParams.MiRows + 15) >> 4);
	uint32_t sbShift = pSeqHdrOBU->use_128x128_superblock ? 5 : 4;
	uint32_t sbSize = sbShift + 2;
	uint32_t maxTileWidthSb = MAX_TILE_WIDTH >> sbSize;
	uint32_t maxTileAreaSb = MAX_TILE_AREA >> (2 * sbSize);
	uint8_t	 minLog2TileCols = BST::AV1::tile_log2(maxTileWidthSb, sbCols);
	uint8_t	 maxLog2TileCols = BST::AV1::tile_log2(1, Min(sbCols, MAX_TILE_COLS));
	uint8_t	 maxLog2TileRows = BST::AV1::tile_log2(1, Min(sbRows, MAX_TILE_ROWS));
	uint8_t	 maxLog2TitleAreaSb = BST::AV1::tile_log2(maxTileAreaSb, sbRows * sbCols);
	uint8_t	 minLog2Tiles = AMP_MAX(minLog2TileCols, maxLog2TitleAreaSb);
	std::vector<uint32_t> MiColStarts, MiRowStarts;
	uint32_t TileSizeBytes = 0;
	uint32_t context_update_tile_id = 0;

	bool uniform_tile_spacing_flag = false;
	if (AMP_FAILED(iRet = bitbuf.GetFlag(uniform_tile_spacing_flag)))goto done;

	if (uniform_tile_spacing_flag) {
		obu_parse_params.ActiveFrameParams.TileColsLog2 = minLog2TileCols;
		while (obu_parse_params.ActiveFrameParams.TileColsLog2 < maxLog2TileCols) {
			bool increment_tile_cols_log2;
			if (AMP_FAILED(iRet = bitbuf.GetFlag(increment_tile_cols_log2)))goto done;

			if (increment_tile_cols_log2 == 1)
				obu_parse_params.ActiveFrameParams.TileColsLog2++;
			else
				break;
		}
		uint32_t tileWidthSb = (sbCols + (1 << obu_parse_params.ActiveFrameParams.TileColsLog2) - 1) >> obu_parse_params.ActiveFrameParams.TileColsLog2;
		uint32_t i = 0;
		for (uint32_t startSb = 0; startSb < sbCols; startSb += tileWidthSb) {
			MiColStarts.push_back(startSb << sbShift);
			i += 1;
		}
		MiColStarts.push_back(obu_parse_params.ActiveFrameParams.MiCols);
		obu_parse_params.ActiveFrameParams.TileCols = i;

		uint8_t minLog2TileRows = (uint8_t)AMP_MAX((int)(minLog2Tiles - obu_parse_params.ActiveFrameParams.TileColsLog2), 0);
		obu_parse_params.ActiveFrameParams.TileRowsLog2 = minLog2TileRows;
		while (obu_parse_params.ActiveFrameParams.TileRowsLog2 < maxLog2TileRows) {
			bool increment_tile_rows_log2;
			if (AMP_FAILED(iRet = bitbuf.GetFlag(increment_tile_rows_log2))) goto done;
			if (increment_tile_rows_log2 == 1)
				obu_parse_params.ActiveFrameParams.TileRowsLog2++;
			else
				break;
		}
		uint32_t tileHeightSb = (sbRows + (1 << obu_parse_params.ActiveFrameParams.TileRowsLog2) - 1) >> obu_parse_params.ActiveFrameParams.TileRowsLog2;
		i = 0;
		for (uint32_t startSb = 0; startSb < sbRows; startSb += tileHeightSb) {
			MiRowStarts.push_back(startSb << sbShift);
			i += 1;
		}
		MiRowStarts.push_back(obu_parse_params.ActiveFrameParams.MiRows);
		obu_parse_params.ActiveFrameParams.TileRows = i;
	}
	else {
		uint32_t widestTileSb = 0;
		uint32_t startSb = 0;
		uint32_t i = 0;
		for (; startSb < sbCols; i++) {
			MiColStarts.push_back(startSb << sbShift);
			uint32_t maxWidth = AMP_MIN(sbCols - startSb, maxTileWidthSb);
			uint32_t width_in_sbs_minus_1;
			if (AMP_FAILED(iRet = bitbuf.AV1ns(maxWidth, width_in_sbs_minus_1)))goto done;
			uint32_t sizeSb = width_in_sbs_minus_1 + 1;
			widestTileSb = AMP_MAX(sizeSb, widestTileSb);
			startSb += sizeSb;
		}

		MiColStarts.push_back(obu_parse_params.ActiveFrameParams.MiCols);
		obu_parse_params.ActiveFrameParams.TileCols = i;
		obu_parse_params.ActiveFrameParams.TileColsLog2 = BST::AV1::tile_log2(1, obu_parse_params.ActiveFrameParams.TileCols);
		if (minLog2Tiles > 0)
			maxTileAreaSb = (sbRows * sbCols) >> (minLog2Tiles + 1);
		else
			maxTileAreaSb = sbRows * sbCols;

		uint32_t maxTileHeightSb = AMP_MAX(maxTileAreaSb / widestTileSb, 1);
		startSb = 0;
		for (i = 0; startSb < sbRows; i++) {
			MiRowStarts.push_back(startSb << sbShift);
			uint32_t maxHeight = Min(sbRows - startSb, maxTileHeightSb);
			uint32_t height_in_sbs_minus_1;
			if (AMP_FAILED(iRet = bitbuf.AV1ns(maxHeight, height_in_sbs_minus_1)))goto done;
			uint32_t sizeSb = height_in_sbs_minus_1 + 1;
			startSb += sizeSb;
		}
		MiRowStarts.push_back(obu_parse_params.ActiveFrameParams.MiRows);
		obu_parse_params.ActiveFrameParams.TileRows = i;
		obu_parse_params.ActiveFrameParams.TileRowsLog2 = BST::AV1::tile_log2(1, obu_parse_params.ActiveFrameParams.TileRows);
	}

	if (obu_parse_params.ActiveFrameParams.TileColsLog2 > 0 || obu_parse_params.ActiveFrameParams.TileRowsLog2 > 0) {
		if (AMP_FAILED(iRet = bitbuf.GetValue(obu_parse_params.ActiveFrameParams.TileRowsLog2 + 
											  obu_parse_params.ActiveFrameParams.TileColsLog2, context_update_tile_id)))goto done;

		uint8_t tile_size_bytes_minus_1;
		if (AMP_FAILED(iRet = bitbuf.GetValue(2, tile_size_bytes_minus_1)))goto done;
		TileSizeBytes = tile_size_bytes_minus_1 + 1;
	}

done:
	return iRet;
}

RET_CODE CAV1Parser::global_motion_params(AV1_OBU spSeqHdrOBU, BITBUF& bitbuf, OBU_PARSE_PARAMS& obu_parse_params, bool FrameIsIntra, bool allow_high_precision_mv)
{
	int iRet = RET_CODE_SUCCESS;

	for (int8_t ref = LAST_FRAME; ref <= ALTREF_FRAME; ref++) {
		obu_parse_params.ActiveFrameParams.GmType[ref-LAST_FRAME] = IDENTITY;
		for (uint8_t i = 0; i < 6; i++) {
			obu_parse_params.ActiveFrameParams.gm_params[ref-LAST_FRAME][i] = ((i % 3 == 2) ?
				1 << WARPEDMODEL_PREC_BITS : 0);
		}
	}

	if (FrameIsIntra)
		return RET_CODE_SUCCESS;

	for (uint8_t ref = LAST_FRAME; ref <= ALTREF_FRAME; ref++) {
		bool is_global;
		uint8_t type;
		if (AMP_FAILED(iRet = bitbuf.GetFlag(is_global)))goto done;
		if (is_global)
		{
			bool is_rot_zoom;
			if (AMP_FAILED(iRet = bitbuf.GetFlag(is_rot_zoom)))goto done;
			if (is_rot_zoom)
				type = ROTZOOM;
			else
			{
				bool is_translation;
				if (AMP_FAILED(iRet = bitbuf.GetFlag(is_translation)))goto done;
				type = is_translation ? TRANSLATION : AFFINE;
			}
		}
		else
			type = IDENTITY;

		obu_parse_params.ActiveFrameParams.GmType[ref - LAST_FRAME] = type;

		if (type >= ROTZOOM) {
			if (AMP_FAILED(iRet = read_global_param(bitbuf, obu_parse_params, type, ref, 2, allow_high_precision_mv)))goto done;
			if (AMP_FAILED(iRet = read_global_param(bitbuf, obu_parse_params, type, ref, 3, allow_high_precision_mv)))goto done;
			if (type == AFFINE) {
				if (AMP_FAILED(iRet = read_global_param(bitbuf, obu_parse_params, type, ref, 4, allow_high_precision_mv)))goto done;
				if (AMP_FAILED(iRet = read_global_param(bitbuf, obu_parse_params, type, ref, 5, allow_high_precision_mv)))goto done;
			}
			else {
				obu_parse_params.ActiveFrameParams.gm_params[ref -  LAST_FRAME][4] = -obu_parse_params.ActiveFrameParams.gm_params[ref - LAST_FRAME][3];
				obu_parse_params.ActiveFrameParams.gm_params[ref - LAST_FRAME][5] =  obu_parse_params.ActiveFrameParams.gm_params[ref - LAST_FRAME][2];
			}
		}
		if (type >= TRANSLATION) {
			if (AMP_FAILED(iRet = read_global_param(bitbuf, obu_parse_params, type, ref, 0, allow_high_precision_mv)))goto done;
			if (AMP_FAILED(iRet = read_global_param(bitbuf, obu_parse_params, type, ref, 1, allow_high_precision_mv)))goto done;
		}
	}

done:
	return iRet;
}

RET_CODE CAV1Parser::film_grain_params(AV1_OBU spSeqHdrOBU, BITBUF& bitbuf, OBU_PARSE_PARAMS& obu_parse_params, bool show_frame, bool showable_frame)
{
	int iRet = RET_CODE_SUCCESS;
	bool apply_gain = false;
	uint16_t grain_seed;
	bool update_grain;
	uint8_t num_y_points, num_cb_points, num_cr_points;;
	uint8_t point_y_value[16] = { 0 };
	uint8_t point_y_scaling[16] = { 0 };
	uint8_t point_cb_value[16] = { 0 };
	uint8_t point_cb_scaling[16] = { 0 };
	uint8_t point_cr_value[16] = { 0 };
	uint8_t point_cr_scaling[16] = { 0 };
	uint8_t ar_coeffs_y_plus_128[24] = { 0 };
	uint8_t ar_coeffs_cb_plus_128[25] = { 0 };
	uint8_t ar_coeffs_cr_plus_128[25] = { 0 };
	bool chroma_scaling_from_luma;
	uint8_t grain_scaling_minus_8;
	uint8_t ar_coeff_lag = 0, numPosLuma = 0, numPosChroma = 0;
	uint8_t ar_coeff_shift_minus_6, grain_scale_shift;
	bool overlap_flag, clip_to_restricted_range;
	uint8_t cb_mult, cr_mult;
	uint8_t cb_luma_mult, cr_luma_mult;
	uint16_t cb_offset, cr_offset;
	if (!spSeqHdrOBU->ptr_sequence_header_obu->film_gain_params_present || (!show_frame && !showable_frame))
	{
		// reset_grain_params();
		return RET_CODE_SUCCESS;
	}
	if (AMP_FAILED(iRet = bitbuf.GetFlag(apply_gain))) goto done;

	if (!apply_gain) {
		// reset_grain_params();
		return RET_CODE_SUCCESS;
	}

	if (AMP_FAILED(iRet = bitbuf.GetValue(16, grain_seed)))goto done;
	if (obu_parse_params.ActiveFrameParams.frame_type == INTER_FRAME)
	{
		if (AMP_FAILED(iRet = bitbuf.GetFlag(update_grain)))goto done;
	}
	else
		update_grain = true;
	
	if (!update_grain)
	{
		uint8_t film_grain_params_ref_idx;
		if (AMP_FAILED(iRet = bitbuf.GetValue(3, film_grain_params_ref_idx)))goto done;
		/*
			tempGrainSeed = grain_seed
			load_grain_params( film_grain_params_ref_idx )
			grain_seed = tempGrainSeed
		*/
		return RET_CODE_SUCCESS;
	}

	if (AMP_FAILED(iRet = bitbuf.GetValue(4, num_y_points)))goto done;
	for (uint8_t i = 0; i < num_y_points; i++)
	{
		if (AMP_FAILED(iRet = bitbuf.GetByte(point_y_value[i])))goto done;
		if (AMP_FAILED(iRet = bitbuf.GetByte(point_y_scaling[i])))goto done;
	}

	if (spSeqHdrOBU->ptr_sequence_header_obu->color_config->mono_chrome)
		chroma_scaling_from_luma = false;
	else if (AMP_FAILED(iRet = bitbuf.GetFlag(chroma_scaling_from_luma)))goto done;

	if (spSeqHdrOBU->ptr_sequence_header_obu->color_config->mono_chrome || chroma_scaling_from_luma ||
		(spSeqHdrOBU->ptr_sequence_header_obu->color_config->subsampling_x == 1 && 
			spSeqHdrOBU->ptr_sequence_header_obu->color_config->subsampling_y == 1 &&
			num_y_points == 0)) {
		num_cb_points = 0;
		num_cr_points = 0;
	}
	else
	{
		if (AMP_FAILED(iRet = bitbuf.GetValue(4, num_cb_points)))goto done;
		for (uint8_t i = 0; i < num_cb_points; i++)
		{
			if (AMP_FAILED(iRet = bitbuf.GetByte(point_cb_value[i])))goto done;
			if (AMP_FAILED(iRet = bitbuf.GetByte(point_cb_scaling[i])))goto done;
		}
		if (AMP_FAILED(iRet = bitbuf.GetValue(4, num_cr_points)))goto done;
		for (uint8_t i = 0; i < num_cb_points; i++)
		{
			if (AMP_FAILED(iRet = bitbuf.GetByte(point_cr_value[i])))goto done;
			if (AMP_FAILED(iRet = bitbuf.GetByte(point_cr_scaling[i])))goto done;
		}
	}

	if (AMP_FAILED(iRet = bitbuf.GetValue(2, grain_scaling_minus_8)))goto done;
	if (AMP_FAILED(iRet = bitbuf.GetValue(2, ar_coeff_lag)))goto done;
	numPosLuma = 2 * ar_coeff_lag * (ar_coeff_lag + 1);
	if (num_y_points) {
		numPosChroma = numPosLuma + 1;
		for (uint8_t i = 0; i < numPosLuma; i++) {
			if (AMP_FAILED(iRet = bitbuf.GetByte(ar_coeffs_y_plus_128[i])))goto done;
		}
	}
	else
		numPosChroma = numPosLuma;

	if (chroma_scaling_from_luma || num_cb_points) {
		for (uint8_t i = 0; i < numPosChroma; i++) {
			if (AMP_FAILED(iRet = bitbuf.GetByte(ar_coeffs_cb_plus_128[i])))goto done;
		}
	}
	if (chroma_scaling_from_luma || num_cr_points) {
		for (uint8_t i = 0; i < numPosChroma; i++) {
			if (AMP_FAILED(iRet = bitbuf.GetByte(ar_coeffs_cr_plus_128[i])))goto done;
		}
	}

	if (AMP_FAILED(iRet = bitbuf.GetValue(2, ar_coeff_shift_minus_6)))goto done;
	if (AMP_FAILED(iRet = bitbuf.GetValue(2, grain_scale_shift)))goto done;

	if (num_cb_points) {
		if (AMP_FAILED(iRet = bitbuf.GetByte(cb_mult)))goto done;
		if (AMP_FAILED(iRet = bitbuf.GetByte(cb_luma_mult)))goto done;
		if (AMP_FAILED(iRet = bitbuf.GetValue(9, cb_offset)))goto done;
	}

	if (num_cr_points) {
		if (AMP_FAILED(iRet = bitbuf.GetByte(cr_mult)))goto done;
		if (AMP_FAILED(iRet = bitbuf.GetByte(cr_luma_mult)))goto done;
		if (AMP_FAILED(iRet = bitbuf.GetValue(9, cr_offset)))goto done;
	}

	if (AMP_FAILED(iRet = bitbuf.GetFlag(overlap_flag)))goto done;
	if (AMP_FAILED(iRet = bitbuf.GetFlag(clip_to_restricted_range)))goto done;

done:
	return iRet;
}

RET_CODE CAV1Parser::read_global_param(BITBUF& bitbuf, OBU_PARSE_PARAMS& obu_parse_params, uint8_t type, uint8_t ref, uint8_t idx, bool allow_high_precision_mv)
{
	uint8_t absBits = GM_ABS_ALPHA_BITS;
	uint8_t precBits = GM_ALPHA_PREC_BITS;

	if (idx < 2) {
		if (type == TRANSLATION) {
			absBits = GM_ABS_TRANS_ONLY_BITS - !allow_high_precision_mv;
			precBits = GM_TRANS_ONLY_PREC_BITS - !allow_high_precision_mv;
		}
		else {
			absBits = GM_ABS_TRANS_BITS;
			precBits = GM_TRANS_PREC_BITS;
		}
	}

	int8_t precDiff = WARPEDMODEL_PREC_BITS - precBits;
	int32_t round = (idx % 3) == 2 ? (1 << WARPEDMODEL_PREC_BITS) : 0;
	int32_t sub = (idx % 3) == 2 ? (1 << precBits) : 0;
	int32_t mx = (1 << absBits);
	int32_t r = (obu_parse_params.ActiveFrameParams.PrevGmParams[ref - LAST_FRAME][idx] >> precDiff) - sub;
	int32_t val;
	int iRet = decode_signed_subexp_with_ref(bitbuf, -mx, mx + 1, r, val);
	if (AMP_FAILED(iRet))return iRet;

	obu_parse_params.ActiveFrameParams.gm_params[ref - LAST_FRAME][idx] = (val << precDiff) + round;

	return RET_CODE_SUCCESS;
}

