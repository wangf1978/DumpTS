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
#include "av1parser.h"

CAV1Parser::CAV1Parser(bool bAnnexB, bool bSingleOBUParse, RET_CODE* pRetCode)
	: m_av1_bytestream_format(bAnnexB?AV1_BYTESTREAM_LENGTH_DELIMITED: AV1_BYTESTREAM_RAW)
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

	uint8_t temporal_unit_size_leb128_bytes = 0;
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
			m_temporal_unit_size = BST::AV1::leb128(pBuf, cbSize, &temporal_unit_size_leb128_bytes, &bNeedMoreData);
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

			PushTemporalUnitBytes(pBuf, pBuf + temporal_unit_size_leb128_bytes);
			AM_LRB_SkipReadPtr(m_rbRawBuf, temporal_unit_size_leb128_bytes);

			cbSize -= temporal_unit_size_leb128_bytes;
			pBuf += temporal_unit_size_leb128_bytes;

			// Update the submit position
			m_cur_submit_pos = m_cur_scan_pos;

			// Update the scan position
			m_cur_scan_pos += temporal_unit_size_leb128_bytes;
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
			SubmitAnnexBTU();

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
		if (m_av1_obu_type == -1)
		{
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
				printf("[AV1] For the non-Annex-B AV1 stream, obu_size field is not available, so quit the scanning process.\n");
				iRet = RET_CODE_ERROR_NOTIMPL;
				goto done;
			}

			// Finished parsing the OBU header and size completely
			bool bGotTU = false;
			if (m_temporal_unit_parsed_size > 0)
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
					SubmitTU();
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
	}

Drain:
	if (bDrain)
	{
		// Flush the data the final temporal unit, if it exists
		if (m_temporal_unit_parsed_size > 0)
		{
			SubmitTU();
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

	uint8_t* pBuf = pTUBuf;
	uint32_t cbSize = (uint32_t)cbTUBuf;
	bool bAnnexB = m_pCtx->GetByteStreamFormat() == AV1_BYTESTREAM_LENGTH_DELIMITED ? true : false;

	if (m_av1_enum && (m_av1_enum_options&AV1_ENUM_OPTION_TU) && AMP_FAILED(m_av1_enum->EnumTemporalUnitStart(m_pCtx, pTUBuf, (uint32_t)cbTUBuf)))
	{
		iRet = RET_CODE_ABORT;
		goto done;
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

		if (m_av1_enum && (m_av1_enum_options&AV1_ENUM_OPTION_FU) && AMP_FAILED(m_av1_enum->EnumFrameUnitStart(m_pCtx, pFrameUnit, frame_unit_size)))
		{
			iRet = RET_CODE_ABORT;
			goto done;
		}

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

			if (obu_length > 0 && (int64_t)cbParsed < (int64_t)cbTUBuf)
			{
				uint8_t* p = pBuf + cbParsed;
				size_t cbOBULeft = obu_length;
				uint8_t obu_type = ((*p) >> 3) & 0xF;
				uint8_t obu_extension_flag = ((*p) >> 2) & 0x1;
				uint8_t obu_has_size_field = ((*p) >> 1) & 0x1;
				uint8_t obu_reserved_1bit = (*p) & 0x1;
				uint32_t obu_size = UINT32_MAX;

				if (obu_extension_flag)
				{
					cbOBULeft--;
					p++;
				}

				if (cbOBULeft > 0)
				{
					if (obu_has_size_field)
					{
						obu_size = BST::AV1::leb128(p, (uint32_t)cbOBULeft, &cbLeb128);
						if (obu_size != UINT32_MAX)
						{
							uint32_t ob_preceding_size = 1 + obu_extension_flag + cbLeb128;
							if (obu_size + ob_preceding_size != obu_length)
								printf("[AV1Parser] obu_size and obu_length are NOT set consistently.\n");
						}
					}
					else
						obu_size = obu_length >= (1UL + obu_extension_flag) ? (obu_length - 1 - obu_extension_flag) : 0;

					if (m_av1_enum != nullptr && (m_av1_enum_options&AV1_ENUM_OPTION_OBU) && obu_size != UINT32_MAX && obu_size > 0)
						m_av1_enum->EnumOBU(m_pCtx, pBuf + cbParsed, obu_length, obu_type, obu_size);
				}
			}

			cbFrameParsed += obu_length;
			cbParsed += obu_length;
		}

		if (AMP_FAILED(iRet))
			break;
		else
		{
			if (m_av1_enum && (m_av1_enum_options&AV1_ENUM_OPTION_FU) && AMP_FAILED(m_av1_enum->EnumFrameUnitEnd(m_pCtx, pFrameUnit, frame_unit_size)))
			{
				iRet = RET_CODE_ABORT;
				goto done;
			}
		}
	}

	if (m_av1_enum && (m_av1_enum_options&AV1_ENUM_OPTION_TU) && AMP_FAILED(m_av1_enum->EnumTemporalUnitEnd(m_pCtx, pTUBuf, (uint32_t)cbTUBuf)))
	{
		iRet = RET_CODE_ABORT;
		goto done;
	}

	m_num_temporal_units++;

done:
	AM_LRB_Reset(m_rbTemporalUnit);
	return iRet;
}

RET_CODE CAV1Parser::SubmitTU()
{
	uint8_t obu_forbidden_bit, obu_type, obu_extension_flag, obu_has_size_field, obu_reserved_1bit;
	uint32_t obu_size = UINT32_MAX;
	uint8_t leb128_byte_count = 0;
	uint8_t leb128_need_more_data = false;

	int iRet = RET_CODE_SUCCESS;
	int cbTUBuf = 0;
	uint8_t* pTUBuf = AM_LRB_GetReadPtr(m_rbTemporalUnit, &cbTUBuf);

	unsigned long cbParsed = 0;
	uint32_t temporal_unit_size = 0;
	uint8_t cbLeb128 = 0;
	uint32_t frame_unit_size = UINT32_MAX;
	uint8_t* pFrameUnit = nullptr;
	bool bFrameHeaderHit = false;
	bool bQuitLoop = false;

	if (cbTUBuf > INT32_MAX)
	{
		printf("[AV1Parser] At present, don't support the temporal unit which size is greater than 2GB.\n");
		return RET_CODE_ERROR_NOTIMPL;
	}

	uint8_t* pBuf = pTUBuf;
	uint32_t cbSize = (uint32_t)cbTUBuf;

	std::vector<std::tuple<uint8_t* /*obu buf*/, uint32_t/*the size of buf*/, uint8_t/*obu_type*/, uint32_t /*obu size*/>> parsed_obus;
	auto iterFrame = parsed_obus.begin();
	auto iter = parsed_obus.begin();

	if (m_av1_enum && (m_av1_enum_options&AV1_ENUM_OPTION_TU) && AMP_FAILED(m_av1_enum->EnumTemporalUnitStart(m_pCtx, pTUBuf, (uint32_t)cbTUBuf)))
	{
		iRet = RET_CODE_ABORT;
		goto done;
	}

	while (cbParsed < cbSize)
	{
		uint32_t cbFrameParsed = 0;
		frame_unit_size = UINT32_MAX;
		pFrameUnit = nullptr;

		// enumerate each OBU in the current buffer
		uint8_t* p = pBuf + cbParsed;
		uint32_t cbLeft = cbSize > cbParsed ? (cbSize - cbParsed) : 0;
		if (cbLeft == 0)
		{
			if (m_av1_enum && AMP_FAILED(m_av1_enum->EnumError(m_pCtx, m_cur_submit_pos, 1)))
			{
				iRet = RET_CODE_ABORT;
				goto done;
			}
		}

		obu_forbidden_bit = ((*p) >> 7) & 0x1;
		if (obu_forbidden_bit != 0)
		{
			if (m_av1_enum && AMP_FAILED(m_av1_enum->EnumError(m_pCtx, m_cur_submit_pos, 2)))
			{
				iRet = RET_CODE_ABORT;
				goto done;
			}
		}

		obu_type = ((*p) >> 3) & 0xF;
		obu_extension_flag = ((*p) >> 2) & 0x1;
		obu_has_size_field = ((*p) >> 1) & 0x1;
		obu_reserved_1bit = (*p) & 0x1;

		// obu_reserved_1bit must be set to 0. The value is ignored by a decoder.
		if (obu_reserved_1bit != 0)
		{
			if (m_av1_enum && AMP_FAILED(m_av1_enum->EnumError(m_pCtx, m_cur_submit_pos, 3)))
			{
				iRet = RET_CODE_ABORT;
				goto done;
			}
		}

		if (obu_extension_flag)
		{
			p++;
			cbLeft--;
			if (cbLeft == 0)
			{
				if (m_av1_enum && AMP_FAILED(m_av1_enum->EnumError(m_pCtx, m_cur_submit_pos, 1)))
				{
					iRet = RET_CODE_ABORT;
					goto done;
				}
			}

			uint8_t temporal_id = ((*p) >> 5) & 0x7;
			uint8_t spatial_id = ((*p) >> 3) & 0x3;
			uint8_t extension_header_reserved_3bits = (*p) & 0x7;

			if (extension_header_reserved_3bits != 0)
			{
				if (m_av1_enum && AMP_FAILED(m_av1_enum->EnumError(m_pCtx, m_cur_submit_pos, 3)))
				{
					iRet = RET_CODE_ABORT;
					goto done;
				}
			}
		}

		leb128_byte_count = 0;
		if (obu_has_size_field)
		{
			p++;
			cbLeft--;
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

		pFrameUnit = pBuf + cbParsed;
		frame_unit_size = obu_size + 1 + obu_extension_flag + leb128_byte_count;
		cbParsed += frame_unit_size;

		parsed_obus.emplace_back(std::make_tuple(pFrameUnit, frame_unit_size, obu_type, obu_size));
	}

	// Now enumerate all frames and OBUs
	bFrameHeaderHit = false;
	iterFrame = parsed_obus.end();
	iter = parsed_obus.begin();

	do {
		if (iterFrame == parsed_obus.end())
		{
			if (iter == parsed_obus.end())
			{
				bQuitLoop = true;
				break;
			}

			if (std::get<2>(*iter) == OBU_FRAME_HEADER)
				bFrameHeaderHit = true;
			iterFrame = iter;
			continue;
		}

		if (iter == parsed_obus.end() || std::get<2>(*iter) == OBU_FRAME_HEADER)
		{
			if (!bFrameHeaderHit && iter != parsed_obus.end())
			{
				bFrameHeaderHit = true;
				iter++;
				continue;
			}

			// commit the frame unit
			if (iterFrame != parsed_obus.end())
			{
				frame_unit_size = UINT32_MAX;
				pFrameUnit = std::get<0>(*iterFrame);
				if (iter != parsed_obus.end())
					frame_unit_size = (uint32_t)(std::get<0>(*iter) - pFrameUnit);
				else
					frame_unit_size = (uint32_t)(pTUBuf + cbTUBuf - pFrameUnit);

				if (m_av1_enum && (m_av1_enum_options&AV1_ENUM_OPTION_FU) && AMP_FAILED(m_av1_enum->EnumFrameUnitStart(m_pCtx, pFrameUnit, frame_unit_size)))
				{
					iRet = RET_CODE_ABORT;
					goto done;
				}

				for (auto obu_iter = iterFrame; obu_iter != iter; obu_iter++)
				{
					if (m_av1_enum && (m_av1_enum_options&AV1_ENUM_OPTION_OBU) && AMP_FAILED(m_av1_enum->EnumOBU(m_pCtx, std::get<0>(*obu_iter), std::get<1>(*obu_iter), std::get<2>(*obu_iter), std::get<3>(*obu_iter))))
					{
						iRet = RET_CODE_ABORT;
						goto done;
					}
				}

				if (m_av1_enum && (m_av1_enum_options&AV1_ENUM_OPTION_FU) && AMP_FAILED(m_av1_enum->EnumFrameUnitEnd(m_pCtx, pFrameUnit, frame_unit_size)))
				{
					iRet = RET_CODE_ABORT;
					goto done;
				}
			}

			if (iter != parsed_obus.end())
			{
				bFrameHeaderHit = false;
				iterFrame = iter;
			}
			else
				break;
		}
	
		iter++;
	} while (!bQuitLoop);

	if (m_av1_enum && (m_av1_enum_options&AV1_ENUM_OPTION_TU) && AMP_FAILED(m_av1_enum->EnumTemporalUnitEnd(m_pCtx, pTUBuf, (uint32_t)cbTUBuf)))
	{
		iRet = RET_CODE_ABORT;
		goto done;
	}

	m_num_temporal_units++;

done:
	AM_LRB_Reset(m_rbTemporalUnit);

	return iRet;
}

