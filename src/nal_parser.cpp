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
#include "nal_parser.h"
#include "h264_video.h"
#include "h265_video.h"
#include "h266_video.h"

CNALParser::CNALParser(NAL_CODING coding, NAL_BYTESTREAM_FORMAT fmt, uint8_t NULenDelimiterSize, RET_CODE* pRetCode)
	: m_nal_coding(coding)
	, m_nal_bytestream_format(fmt)
	, m_nal_length_delimiter_size(NULenDelimiterSize)
{
	RET_CODE ret_code = RET_CODE_SUCCESS;
	m_rbRawBuf = AM_LRB_Create(read_unit_size * 128);
	m_rbNALUnitEBSP = AM_LRB_Create(64 * 1024);

	memset(state_buf, 0, sizeof(state_buf));

	// Try to create a NAL parser context
	if (coding == NAL_CODING_AVC)
	{
		INALAVCContext* pNALAVCCtx = nullptr;
		if (AMP_FAILED(CreateAVCNALContext(&pNALAVCCtx)))
		{
			printf("Failed to create the AVC NAL context.\n");
			ret_code = RET_CODE_ERROR;
		}
		else
		{
			m_pNALAVCCtx = pNALAVCCtx;
			pNALAVCCtx->QueryInterface(IID_INALContext, (void**)&m_pCtx);
		}

		avc_presentation_time_code = 0;
		avc_num_units_in_tick = 0;
		avc_time_scale = 0;
		avc_nuit_field_based_flag = 1;
	}
	else if (coding == NAL_CODING_HEVC)
	{
		INALHEVCContext* pNALHEVCCtx = nullptr;
		if (AMP_FAILED(CreateHEVCNALContext(&pNALHEVCCtx)))
		{
			printf("Failed to create the HEVC NAL context.\n");
			ret_code = RET_CODE_ERROR;
		}
		else
		{
			m_pNALHEVCCtx = pNALHEVCCtx;
			pNALHEVCCtx->QueryInterface(IID_INALContext, (void**)&m_pCtx);
		}

		hevc_presentation_time_code = 0;	// in the unit of 100-nano seconds
		hevc_num_units_in_tick = 0;
		hevc_time_scale = 0;
		hevc_units_field_based_flag = 0;
	}
	else if (coding == NAL_CODING_VVC)
	{
		INALVVCContext* pNALVVCCtx = nullptr;
		if (AMP_FAILED(CreateVVCNALContext(&pNALVVCCtx)))
		{
			printf("Failed to create the VVC NAL context.\n");
			ret_code = RET_CODE_ERROR;
		}
		else
		{
			m_pNALVVCCtx = pNALVVCCtx;
			pNALVVCCtx->QueryInterface(IID_INALContext, (void**)&m_pCtx);
		}

		vvc_presentation_time_code = 0;	// in the unit of 100-nano seconds
		vvc_num_units_in_tick = 0;
		vvc_time_scale = 0;
		vui_progressive_source_flag = 0;
		vui_interlaced_source_flag = 0;
	}

	AMP_SAFEASSIGN(pRetCode, ret_code);
}

CNALParser::~CNALParser()
{
	AMP_SAFERELEASE(m_nal_enum);

	if (m_nal_coding == NAL_CODING_AVC)
	{
		AMP_SAFERELEASE(m_pNALAVCCtx);
	}
	else if (m_nal_coding == NAL_CODING_HEVC)
	{
		AMP_SAFERELEASE(m_pNALHEVCCtx);
	}
	else if (m_nal_coding == NAL_CODING_VVC)
	{
		AMP_SAFERELEASE(m_pNALVVCCtx);
	}

	AMP_SAFERELEASE(m_pCtx);

	AM_LRB_Destroy(m_rbNALUnitEBSP);
	AM_LRB_Destroy(m_rbRawBuf);
}

RET_CODE CNALParser::SetEnumerator(IUnknown* pEnumerator, uint32_t options)
{
	INALEnumerator* pNALEumerator = nullptr;
	if (pEnumerator != nullptr)
	{
		if (FAILED(pEnumerator->QueryInterface(IID_INALEnumerator, (void**)&pNALEumerator)))
		{
			return RET_CODE_ERROR;
		}
	}

	if (m_nal_enum)
		m_nal_enum->Release();

	m_nal_enum = pNALEumerator;
	
	m_nal_enum_options = options;

	return RET_CODE_SUCCESS;
}

RET_CODE CNALParser::GetContext(IUnknown** ppCtx)
{
	if (ppCtx == nullptr)
		return RET_CODE_INVALID_PARAMETER;

	m_pCtx->AddRef();
	*ppCtx = (IUnknown*)m_pCtx;
	return RET_CODE_SUCCESS;
}

RET_CODE CNALParser::Reset()
{
	m_cur_nal_unit_type = -1;
	AM_LRB_Reset(m_rbNALUnitEBSP);
	AM_LRB_Reset(m_rbRawBuf);

	m_cur_scan_pos = 0;
	m_cur_submit_pos = 0;
	m_cur_nal_unit_prefix_start_code_length = 0;
	m_count_nal_unit_scanned = 0;

	m_nu_entries.clear();
	m_last_first_slice_nu_entry.file_offset = UINT64_MAX;

	memset(state_buf, 0, sizeof(state_buf));

	nal_sequences.clear();

	m_pCtx->Reset();

	return RET_CODE_SUCCESS;
}

RET_CODE CNALParser::ProcessInput(uint8_t* pInput, size_t cbInput)
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

RET_CODE CNALParser::ProcessAnnexBOutput(bool bDrain)
{
	int cbSize = 0;
	int8_t cur_nuh_layer_id = -1;
	int8_t cur_nuh_temporal_id_plus1 = -1;
	int8_t cur_nal_ref_idc = -1;

	bool bNalUnitZeroByte = false;
	int iNUCommitRet = RET_CODE_SUCCESS;

	RET_CODE iRet = RET_CODE_SUCCESS;
	uint8_t* pStartBuf = NULL, *pBuf = NULL, *pCurParseStartBuf = NULL, *pSubmitEnd = NULL;

	if ((pStartBuf = AM_LRB_GetReadPtr(m_rbRawBuf, &cbSize)) == NULL || cbSize <= 0)
	{
		return RET_CODE_NEEDMOREINPUT;
	}

	pCurParseStartBuf = pBuf = pSubmitEnd = pStartBuf;
	while (cbSize >= minimum_nal_parse_buffer_size)
	{
		// Find "start_code_prefix_one_3bytes"
		while (cbSize >= minimum_nal_parse_buffer_size && !(pBuf[0] == 0 && pBuf[1] == 0 && pBuf[2] == 1))
		{
			cbSize--;
			pBuf++;
		}

		uint8_t nal_unit_prefix_start_code_length = 3;
		// Don't miss the zero_byte before "start_code_prefix_one_3bytes"
		if (cbSize >= minimum_nal_parse_buffer_size)
		{
			if (pBuf > pCurParseStartBuf && *(pBuf - 1) == 0) {
				bNalUnitZeroByte = true;
				cbSize++;
				pBuf--;
				nal_unit_prefix_start_code_length++;
			}
			else
				bNalUnitZeroByte = false;
		}

		// If the current NAL unit type is valid, need push the parsed data to ring buffer of EBSP
		if (m_cur_nal_unit_type != -1 && pBuf > pCurParseStartBuf)
		{
			//printf("pushing ");
			//for (uint8_t* z = pCurParseStartBuf; z < AMP_MIN(pBuf, pCurParseStartBuf + 4); z++)
			//	printf("%02X ", *z);
			//printf(": %d bytes\n", (int)(pBuf - pCurParseStartBuf));

			m_cur_submit_nal_unit_len += (uint32_t)(pBuf - pCurParseStartBuf);

			if (FAILED(iRet = PushESBP(pCurParseStartBuf, pBuf)))
				goto done;

			//if (pCurParseStartBuf[0] == 0xCB && pCurParseStartBuf[1] == 0x0B && pCurParseStartBuf[2] == 0x3C && pCurParseStartBuf[3] == 0xBD)
			//if (pCurParseStartBuf[0] == 0xCE && pCurParseStartBuf[1] == 0xCF && pCurParseStartBuf[2] == 0x1E && pCurParseStartBuf[3] == 0xAB)
/*			if (pCurParseStartBuf[0] == 0x37 && pCurParseStartBuf[1] == 0x95 && pCurParseStartBuf[2] == 0x12 && pCurParseStartBuf[3] == 0x83)
				printf("Hitting here again.\n")*/;
		}

		// pBuf is the next submit position
		pSubmitEnd = pBuf;

		// Failed to find the "start_code_prefix_one_3bytes"
		if (cbSize < nal_unit_prefix_start_code_length + 2)
			break;	// Quit the current loop and read more data and do the next round of scanning

		// check the nal_unit_type
		//uint8_t nal_unit_prefix_start_code_length = 3 + (bNalUnitZeroByte ? 1 : 0);
		int8_t forbidden_zero_bit = (pBuf[nal_unit_prefix_start_code_length] >> 7) & 0x01;

		int8_t nal_unit_type = -1, nuh_layer_id = -1, nuh_temporal_id_plus1 = -1, nal_ref_idc = -1;
		uint64_t file_offset = m_cur_scan_pos + (int64_t)(pBuf - pStartBuf) + nal_unit_prefix_start_code_length;

		pCurParseStartBuf = pBuf/* + nal_unit_prefix_start_code_length*/;

		// Complete a NAL Unit, and try to commit it
		// During committing it, do the advance analysis, it may parsing the AU, SPS/PPS, and SEI message/payload
		if (m_cur_nal_unit_type != -1 && (iNUCommitRet = CommitNALUnit(m_cur_nal_unit_prefix_start_code_length)) == RET_CODE_ABORT)
		{
			m_cur_submit_nal_unit_len = 0;
			iRet = iNUCommitRet;
			goto done;
		}

		m_cur_submit_nal_unit_len = 0;
		if (m_nal_coding == NAL_CODING_AVC)
		{
			nal_ref_idc = (pBuf[nal_unit_prefix_start_code_length] >> 5) & 0x3;
			nal_unit_type = pBuf[nal_unit_prefix_start_code_length] & 0x1F;
		}
		else if (m_nal_coding == NAL_CODING_HEVC)
		{
			nal_unit_type = (pBuf[nal_unit_prefix_start_code_length] >> 1) & 0x3F;
			nuh_layer_id = (int8_t)(((pBuf[nal_unit_prefix_start_code_length] & 0x1) << 5) |
				((pBuf[nal_unit_prefix_start_code_length + 1] >> 3) & 0x1F));
			nuh_temporal_id_plus1 = pBuf[nal_unit_prefix_start_code_length + 1] & 0x7;
		}
		else if (m_nal_coding == NAL_CODING_VVC)
		{
			nuh_layer_id = pBuf[nal_unit_prefix_start_code_length] & 0x3F;
			nal_unit_type = (pBuf[nal_unit_prefix_start_code_length + 1] >> 3) & 0x1F;
			nuh_temporal_id_plus1 = pBuf[nal_unit_prefix_start_code_length + 1] & 0x7;
		}

		// Skip "prefix start code", it may be 00 00 01 or 00 00 00 01
		pBuf += nal_unit_prefix_start_code_length; cbSize -= nal_unit_prefix_start_code_length;

		// If the current NAL_UNIT is not required to parse, just skip it, and find the next start code
		if (m_pCtx && !m_pCtx->IsNUFiltered(nal_unit_type))
		{
			// Skipping Zero byte, "start_code_prefix_one_3bytes" and nal_unit_header
			m_cur_nal_unit_type = -1;
			continue;
		}

		m_cur_nal_unit_type = nal_unit_type;

		if (m_nal_coding == NAL_CODING_AVC)
		{
			cur_nal_ref_idc = nal_ref_idc;
		}
		else if (m_nal_coding == NAL_CODING_HEVC || m_nal_coding == NAL_CODING_VVC)
		{
			cur_nuh_layer_id = nuh_layer_id;
			cur_nuh_temporal_id_plus1 = nuh_temporal_id_plus1;
		}

		m_cur_submit_pos = m_cur_scan_pos + (size_t)(pCurParseStartBuf - pStartBuf);
		m_cur_nal_unit_prefix_start_code_length = nal_unit_prefix_start_code_length;
	}

	// Skip the parsed raw data buffer
	AM_LRB_SkipReadPtr(m_rbRawBuf, (unsigned int)(pSubmitEnd - pStartBuf));

	// Update the scan position
	m_cur_scan_pos += pSubmitEnd - pStartBuf;

	if (bDrain && (pStartBuf = AM_LRB_GetReadPtr(m_rbRawBuf, &cbSize)) != NULL && cbSize > 0)
	{
		uint8_t* pEndBuf = pStartBuf + cbSize;
		pCurParseStartBuf = pBuf = pStartBuf;
		if (cbSize >= minimum_nal_parse_buffer_size)
		{
			// Find "start_code_prefix_one_3bytes"
			while (cbSize >= minimum_nal_parse_buffer_size && !(pBuf[0] == 0 && pBuf[1] == 0 && pBuf[2] == 1))
			{
				cbSize--;
				pBuf++;
			}

			// Don't miss the zero_byte before "start_code_prefix_one_3bytes"
			if (cbSize >= minimum_nal_parse_buffer_size)
			{
				if (pBuf > pCurParseStartBuf && *(pBuf - 1) == 0) {
					bNalUnitZeroByte = true;
					cbSize++;
					pBuf--;
				}
				else
					bNalUnitZeroByte = false;
			}
		}

		if (m_cur_nal_unit_type != -1)
		{
			if (pStartBuf < pBuf && FAILED(iRet = PushESBP(pStartBuf, pBuf)))
				goto done;

			if ((iNUCommitRet = CommitNALUnit(m_cur_nal_unit_prefix_start_code_length)) == RET_CODE_ABORT)
			{
				iRet = iNUCommitRet;
				goto done;
			}

			m_cur_scan_pos += pBuf - pStartBuf;
		}

		if (cbSize >= minimum_nal_parse_buffer_size)
		{
			m_cur_submit_pos = m_cur_scan_pos + (size_t)(pBuf - pStartBuf);
			m_cur_nal_unit_prefix_start_code_length = 3 + (bNalUnitZeroByte ? 1 : 0);
			if (FAILED(iRet = PushESBP(pBuf, pEndBuf)))
				goto done;

			if ((iNUCommitRet = CommitNALUnit(m_cur_nal_unit_prefix_start_code_length)) == RET_CODE_ABORT)
			{
				iRet = iNUCommitRet;
				goto done;
			}

			m_cur_scan_pos += pEndBuf - pBuf;
		}
	}

	if (bDrain)
	{
		if ((m_nal_enum_options&(NAL_ENUM_OPTION_AU|NAL_ENUM_OPTION_CVS|NAL_ENUM_OPTION_VSEQ)) || ((m_nal_enum_options&MSE_ENUM_CMD_MASK) == MSE_ENUM_SYNTAX_VIEW))
			CommitSliceInfo(true);

		AM_LRB_Reset(m_rbRawBuf);
		AM_LRB_Reset(m_rbNALUnitEBSP);
	}

done:
	return iRet;
}

RET_CODE CNALParser::ProcessOutput(bool bDrain)
{
	if (m_nal_bytestream_format == NAL_BYTESTREAM_ANNEX_B)
		return ProcessAnnexBOutput(bDrain);

	return RET_CODE_ERROR_NOTIMPL;
}

int CNALParser::PushESBP(uint8_t* pStart, uint8_t* pEnd)
{
	int cbLeftOfEBSP = 0;
	uint8_t* pEBSPWriteBuf = AM_LRB_GetWritePtr(m_rbNALUnitEBSP, &cbLeftOfEBSP);
	if (pEBSPWriteBuf == NULL || cbLeftOfEBSP < (int)(pEnd - pStart))
	{
		// Try to reform the ring buffer of EBSP
		AM_LRB_Reform(m_rbNALUnitEBSP);
		pEBSPWriteBuf = AM_LRB_GetWritePtr(m_rbNALUnitEBSP, &cbLeftOfEBSP);
		if (pEBSPWriteBuf == NULL || cbLeftOfEBSP < (int)(pEnd - pStart))
		{
			// Try to enlarge the ring buffer of EBSP
			int rb_size = AM_LRB_GetSize(m_rbNALUnitEBSP);
			int64_t new_rb_size = (int64_t)rb_size << 1;
			if (new_rb_size < (int64_t)rb_size + (int64_t)(pEnd - pStart) - (pEBSPWriteBuf != NULL ? cbLeftOfEBSP : 0))
				new_rb_size = (int64_t)rb_size + (int64_t)(pEnd - pStart) - (pEBSPWriteBuf != NULL ? cbLeftOfEBSP : 0);

			if (new_rb_size >= INT32_MAX || AM_LRB_Resize(m_rbNALUnitEBSP, (int)new_rb_size) < 0)
			{
				printf("Failed to resize the ring buffer of NAL Unit EBSP to %" PRId64 ".\n", (int64_t)rb_size * 2);
				return RET_CODE_ERROR;
			}

			pEBSPWriteBuf = AM_LRB_GetWritePtr(m_rbNALUnitEBSP, &cbLeftOfEBSP);
		}
	}

	// Write the parsed buffer to ring buffer
	memcpy(pEBSPWriteBuf, pStart, (size_t)(pEnd - pStart));

	AM_LRB_SkipWritePtr(m_rbNALUnitEBSP, (unsigned int)(pEnd - pStart));

	return RET_CODE_SUCCESS;
}

int CNALParser::LoadVVCParameterSet(uint8_t* pNUBuf, int cbNUBuf, uint64_t cur_submit_pos)
{
	int iRet = RET_CODE_SUCCESS;
	int read_buf_len = 0;
	AMBst bst = NULL;

	//uint8_t* pStart = AM_LRB_GetReadPtr(m_rbNALUnitEBSP, &read_buf_len);

	uint8_t* pStart = pNUBuf;
	read_buf_len = cbNUBuf;

	if (pStart == NULL || read_buf_len < 4)
	{
		//printf("the byte stream of NAL unit carry insufficient data {offset: %llu}.\n", cur_submit_pos);
		return RET_CODE_ERROR;
	}

	int8_t nal_unit_type = (*(pStart + 1) >> 3) & 0x1F;
	AMP_Assert(IS_VVC_PARAMETERSET_NAL(nal_unit_type) || nal_unit_type == VVC_PH_NUT);

	auto nal_unit = m_pNALVVCCtx->CreateVVCNU();

	bst = AMBst_CreateFromBuffer(pStart, read_buf_len);
	if (AMP_FAILED(iRet = nal_unit->Map(bst)))
	{
		printf("Failed to unpack %s parameter set {offset: %" PRIu64 ", err: %d}\n", vvc_nal_unit_type_descs[nal_unit_type], cur_submit_pos, iRet);
		goto done;
	}

	if (nal_unit_type == BST::H266Video::OPI_NUT)
	{
		m_pNALVVCCtx->UpdateVVCOPI(nal_unit);
	}
	else if (nal_unit_type == BST::H266Video::DCI_NUT)
	{
		m_pNALVVCCtx->UpdateVVCDCI(nal_unit);
	}
	else if (nal_unit_type == BST::H266Video::VPS_NUT)
	{
		m_pNALVVCCtx->UpdateVVCVPS(nal_unit);
	}
	else if (nal_unit_type == BST::H266Video::SPS_NUT)
	{
		m_pNALVVCCtx->UpdateVVCSPS(nal_unit);
	}
	else if (nal_unit_type == BST::H266Video::PPS_NUT)
	{
		m_pNALVVCCtx->UpdateVVCPPS(nal_unit);
	}
	else if (nal_unit_type == BST::H266Video::PH_NUT)
	{
		m_pNALVVCCtx->UpdateVVCPH(nal_unit);
	}

done:
	if (bst != NULL)
		AMBst_Destroy(bst);

	nal_unit = nullptr;

	return iRet;
}

int CNALParser::LoadHEVCParameterSet(uint8_t* pNUBuf, int cbNUBuf, uint64_t cur_submit_pos)
{
	int iRet = RET_CODE_SUCCESS;
	int read_buf_len = 0;
	AMBst bst = NULL;

	//uint8_t* pStart = AM_LRB_GetReadPtr(m_rbNALUnitEBSP, &read_buf_len);

	uint8_t* pStart = pNUBuf;
	read_buf_len = cbNUBuf;

	if (pStart == NULL || read_buf_len < 4)
	{
		//printf("the byte stream of NAL unit carry insufficient data {offset: %llu}.\n", cur_submit_pos);
		return RET_CODE_ERROR;
	}

	int8_t nal_unit_type = (*pStart >> 1) & 0x3F;
	AMP_Assert(IS_HEVC_PARAMETERSET_NAL(nal_unit_type));

	auto nal_unit = m_pNALHEVCCtx->CreateHEVCNU();

	bst = AMBst_CreateFromBuffer(pStart, read_buf_len);
	if (AMP_FAILED(iRet = nal_unit->Map(bst)))
	{
		printf("Failed to unpack %s parameter set {offset: %" PRIu64 ", err: %d}\n", hevc_nal_unit_type_descs[nal_unit_type], cur_submit_pos, iRet);
		goto done;
	}

	if (nal_unit_type == BST::H265Video::VPS_NUT)
	{
		m_pNALHEVCCtx->UpdateHEVCVPS(nal_unit);
	}
	else if (nal_unit_type == BST::H265Video::SPS_NUT)
	{
		m_pNALHEVCCtx->UpdateHEVCSPS(nal_unit);
	}
	else if (nal_unit_type == BST::H265Video::PPS_NUT)
	{
		m_pNALHEVCCtx->UpdateHEVCPPS(nal_unit);
	}

done:
	if (bst != NULL)
		AMBst_Destroy(bst);

	nal_unit = nullptr;

	return iRet;
}

int CNALParser::LoadAVCParameterSet(uint8_t* pNUBuf, int cbNUBuf, uint64_t cur_submit_pos)
{
	int iRet = RET_CODE_SUCCESS;
	int read_buf_len = 0;
	AMBst bst = NULL;

	//uint8_t* pStart = AM_LRB_GetReadPtr(m_rbNALUnitEBSP, &read_buf_len);

	uint8_t* pStart = pNUBuf;
	read_buf_len = cbNUBuf;

	if (pStart == NULL || read_buf_len < 3)
	{
		//printf("the byte stream of NAL unit carry insufficient data {offset: %llu}.\n", cur_submit_pos);
		return RET_CODE_ERROR;
	}

	int8_t nal_unit_type = *pStart & 0x1F;
	AMP_Assert(IS_AVC_PARAMETERSET_NAL(nal_unit_type));

	auto nal_unit = m_pNALAVCCtx->CreateAVCNU();

	bst = AMBst_CreateFromBuffer(pStart, read_buf_len);
	if (AMP_FAILED(iRet = nal_unit->Map(bst)))
	{
		printf("Failed to unpack %s parameter set {offset: %" PRIu64 ", err: %d}\n", avc_nal_unit_type_descs[nal_unit_type], cur_submit_pos, iRet);
		goto done;
	}

	if (nal_unit_type == AVC_SPS_NUT)
	{
		m_pNALAVCCtx->UpdateAVCSPS(nal_unit);
	}
	else if (nal_unit_type == AVC_PPS_NUT)
	{
		m_pNALAVCCtx->UpdateAVCPPS(nal_unit);
	}

done:
	if (bst != NULL)
		AMBst_Destroy(bst);

	nal_unit = nullptr;

	return iRet;
}

int CNALParser::PickupLastSliceHeaderInfo(uint8_t* pNUBuf, int cbNUBuf)
{
	if (m_nal_coding == NAL_CODING_VVC)
		return PickupVVCSliceHeaderInfo(m_nu_entries.back(), pNUBuf, cbNUBuf);
	else if (m_nal_coding == NAL_CODING_HEVC)
		return PickupHEVCSliceHeaderInfo(m_nu_entries.back(), pNUBuf, cbNUBuf);
	else if (m_nal_coding == NAL_CODING_AVC)
	{
		int iRet = PickupAVCSliceHeaderInfo(m_nu_entries.back(), pNUBuf, cbNUBuf);

		// Check whether the current slice is the first VCL NAL unit or not in the new access unit
		if (AMP_SUCCEEDED(iRet))
		{
			auto& cur_AVC_VCL_nu = m_nu_entries.back();
			cur_AVC_VCL_nu.first_slice_segment_in_pic_flag = cur_AVC_VCL_nu.Is_The_First_AVC_VCL_NU(m_last_first_slice_nu_entry) ? 1 : 0;

			if (cur_AVC_VCL_nu.first_slice_segment_in_pic_flag)
			{
				m_last_first_slice_nu_entry = cur_AVC_VCL_nu;
			}
		}

		return iRet;
	}

	return RET_CODE_ERROR_NOTIMPL;
}

int CNALParser::PickupVVCSliceHeaderInfo(NAL_UNIT_ENTRY& nu_entry, uint8_t* pNUBuf, int cbNUBuf)
{
	int iRet = RET_CODE_SUCCESS;
	int read_buf_len = 0;
	//uint8_t* pEBSPBuf = AM_LRB_GetReadPtr(m_rbNALUnitEBSP, &read_buf_len);

	uint8_t* pEBSPBuf = pNUBuf;
	read_buf_len = cbNUBuf;

	if (pEBSPBuf == NULL || read_buf_len < 3)
		return RET_CODE_NEEDMOREINPUT;

	int8_t forbidden_zero_bit = (pEBSPBuf[0] >> 7) & 0x01;
	int8_t nuh_reserved_zero_bit = (pEBSPBuf[0] >> 6) & 0x01;
	int8_t nuh_layer_id = pEBSPBuf[0] & 0x3F;
	int8_t nuh_temporal_id_plus1 = pEBSPBuf[1] & 0x07;
	int8_t nal_unit_type = (pEBSPBuf[1] >> 3) & 0x1F;

	AMBst bs = AMBst_CreateFromBuffer(pEBSPBuf + 2, read_buf_len - 2);
	AMBst_SetRBSPType(bs, BST_RBSP_NAL_UNIT);

	if (m_hit_PH_NU_in_one_AU) {
		nu_entry.first_slice_segment_in_pic_flag = true;
		m_hit_PH_NU_in_one_AU = false;
	}

	H266_NU sps, pps, ph;
	int pic_parameter_set_id = -1;

	try
	{
		uint16_t ph_inter_slice_allowed_flag = 0;
		uint8_t sh_picture_header_in_slice_header_flag = (uint8_t)AMBst_GetBits(bs, 1);
		if (sh_picture_header_in_slice_header_flag)
		{
			nu_entry.first_slice_segment_in_pic_flag = true;

			BST::H266Video::NAL_UNIT::PICTURE_HEADER_STRUCTURE PicHdrStruct((BST::H266Video::VideoBitstreamCtx*)m_pCtx);
			iRet = PicHdrStruct.Map(bs);

			pic_parameter_set_id = PicHdrStruct.ph_pic_parameter_set_id;
			ph_inter_slice_allowed_flag = PicHdrStruct.ph_inter_slice_allowed_flag;
		}
		else
		{
			ph = m_pNALVVCCtx->GetVVCPH();
			if (ph != nullptr && ph->ptr_picture_header_rbsp)
			{
				pic_parameter_set_id = ph->ptr_picture_header_rbsp->picture_header_structure.ph_pic_parameter_set_id;
				ph_inter_slice_allowed_flag = ph->ptr_picture_header_rbsp->picture_header_structure.ph_inter_slice_allowed_flag;
			}

			iRet = RET_CODE_NOTHING_TODO;
		}

		if (pic_parameter_set_id == -1 ||
			(pps = m_pNALVVCCtx->GetVVCPPS(pic_parameter_set_id)) == nullptr ||
			pps->ptr_pic_parameter_set_rbsp == nullptr ||
			(sps = m_pNALVVCCtx->GetVVCSPS(pps->ptr_pic_parameter_set_rbsp->pps_seq_parameter_set_id)) == nullptr ||
			sps->ptr_seq_parameter_set_rbsp == nullptr)
		{
			nu_entry.slice_type = -2;
			goto done;
		}

		if (!ph_inter_slice_allowed_flag)
		{
			nu_entry.slice_type = 2;
		}
		else
		{
			int CurrSubpicIdx = 0;
			if (sps->ptr_seq_parameter_set_rbsp->sps_subpic_info_present_flag) {
				uint32_t sh_subpic_id = (uint32_t)AMBst_GetBits(bs, sps->ptr_seq_parameter_set_rbsp->sps_subpic_id_len_minus1 + 1);

				for (; CurrSubpicIdx <= sps->ptr_seq_parameter_set_rbsp->sps_num_subpics_minus1; CurrSubpicIdx++)
					if (pps->ptr_pic_parameter_set_rbsp->SubpicIdVal[CurrSubpicIdx] == sh_subpic_id)
						break;
			}

			uint32_t sh_slice_address = 0;
			uint32_t NumTilesInPic = pps->ptr_pic_parameter_set_rbsp->NumTileRows*pps->ptr_pic_parameter_set_rbsp->NumTileColumns;
			if ((pps->ptr_pic_parameter_set_rbsp->pps_rect_slice_flag && pps->ptr_pic_parameter_set_rbsp->NumSlicesInSubpic.size() > 0 && 
				 pps->ptr_pic_parameter_set_rbsp->NumSlicesInSubpic[CurrSubpicIdx] > 1) ||
				(!pps->ptr_pic_parameter_set_rbsp->pps_rect_slice_flag && NumTilesInPic > 1))
			{
				if (!pps->ptr_pic_parameter_set_rbsp->pps_rect_slice_flag) {
					sh_slice_address = (uint32_t)AMBst_GetBits(bs, quick_ceil_log2(NumTilesInPic));
				}
				else
				{
					sh_slice_address = (uint32_t)AMBst_GetBits(bs, quick_ceil_log2(pps->ptr_pic_parameter_set_rbsp->NumSlicesInSubpic[CurrSubpicIdx]));
				}
			}

			if (sps->ptr_seq_parameter_set_rbsp->NumExtraShBits > 0)
				AMBst_SkipBits(bs, sps->ptr_seq_parameter_set_rbsp->NumExtraShBits);

			if (!pps->ptr_pic_parameter_set_rbsp->pps_rect_slice_flag && NumTilesInPic > 1 + sh_slice_address) {
				uint32_t sh_num_tiles_in_slice_minus1 = (uint32_t)AMBst_Get_ue(bs);
			}

			nu_entry.slice_type = (int8_t)AMBst_Get_ue(bs);
		}

		nu_entry.slice_pic_parameter_set_id = (int16_t)pic_parameter_set_id;
	}
	catch (...)
	{
		iRet = RET_CODE_ERROR;
	}

done:
	AMBst_Destroy(bs);
	return iRet;
}

int CNALParser::PickupHEVCSliceHeaderInfo(NAL_UNIT_ENTRY& nu_entry, uint8_t* pNUBuf, int cbNUBuf)
{
	int iRet = RET_CODE_SUCCESS;
	int read_buf_len = 0;
	//uint8_t* pEBSPBuf = AM_LRB_GetReadPtr(m_rbNALUnitEBSP, &read_buf_len);

	uint8_t* pEBSPBuf = pNUBuf;
	read_buf_len = cbNUBuf;

	if (pEBSPBuf == NULL || read_buf_len < 3)
		return RET_CODE_NEEDMOREINPUT;

	int8_t forbidden_zero_bit = (pEBSPBuf[0] >> 7) & 0x01;
	int8_t nal_unit_type = (pEBSPBuf[0] >> 1) & 0x3F;
	int8_t nuh_layer_id = ((pEBSPBuf[0] & 0x01) << 5) | ((pEBSPBuf[1] >> 3) & 0x1F);
	int8_t nuh_temporal_id_plus1 = pEBSPBuf[1] & 0x07;

	AMBst bs = AMBst_CreateFromBuffer(pEBSPBuf + 2, read_buf_len - 2);
	AMBst_SetRBSPType(bs, BST_RBSP_NAL_UNIT);

	int dependent_slice_segment_flag = 0;

	try
	{
		nu_entry.first_slice_segment_in_pic_flag = (int8_t)AMBst_GetBits(bs, 1);
		if (nal_unit_type >= BST::H265Video::BLA_W_LP && nal_unit_type <= BST::H265Video::RSV_IRAP_VCL23)
			nu_entry.no_output_of_prior_pics_flag = (int8_t)AMBst_GetBits(bs, 1);

		nu_entry.slice_pic_parameter_set_id = (uint8_t)AMBst_Get_ue(bs);

		H265_NU pps, sps;
		if (nu_entry.slice_pic_parameter_set_id < 0 ||
			nu_entry.slice_pic_parameter_set_id > UINT8_MAX || !(pps = m_pNALHEVCCtx->GetHEVCPPS((uint8_t)nu_entry.slice_pic_parameter_set_id)) ||
			pps->ptr_pic_parameter_set_rbsp == nullptr)
		{
			nu_entry.slice_type = -2;
			goto done;
		}

		if (!nu_entry.first_slice_segment_in_pic_flag)
		{
			if (pps->ptr_pic_parameter_set_rbsp->dependent_slice_segments_enabled_flag)
				dependent_slice_segment_flag = (int8_t)AMBst_GetBits(bs, 1);

			sps = m_pNALHEVCCtx->GetHEVCSPS(pps->ptr_pic_parameter_set_rbsp->pps_pic_parameter_set_id);
			if (!sps || sps->ptr_seq_parameter_set_rbsp == nullptr)
			{
				nu_entry.slice_type = -2;
				goto done;
			}

			uint32_t slice_segment_address = (uint32_t)AMBst_GetBits(bs, quick_ceil_log2(sps->ptr_seq_parameter_set_rbsp->PicSizeInCtbsY));
		}

		if (!dependent_slice_segment_flag)
		{
			AMBst_SkipBits(bs, pps->ptr_pic_parameter_set_rbsp->num_extra_slice_header_bits);
			nu_entry.slice_type = (int8_t)AMBst_Get_ue(bs);
		}
		else
			nu_entry.slice_type = -1;
	}
	catch (...)
	{
		iRet = RET_CODE_ERROR;
	}

done:
	AMBst_Destroy(bs);
	return iRet;
}

int CNALParser::PickupAVCSliceHeaderInfo(NAL_UNIT_ENTRY& nu_entry, uint8_t* pNUBuf, int cbNUBuf)
{
	int iRet = RET_CODE_SUCCESS;
	int read_buf_len = 0;
	//uint8_t* pEBSPBuf = AM_LRB_GetReadPtr(m_rbNALUnitEBSP, &read_buf_len);

	uint8_t* pEBSPBuf = pNUBuf;
	read_buf_len = cbNUBuf;

	if (pEBSPBuf == NULL || read_buf_len < 2)
		return RET_CODE_NEEDMOREINPUT;

	uint8_t nalUnitHeaderBytes = 1;

	int8_t forbidden_zero_bit = (pEBSPBuf[0] >> 7) & 0x01;
	int8_t nal_ref_idc = (pEBSPBuf[0] >> 5) & 0x3;
	int8_t nal_unit_type = pEBSPBuf[0] & 0x1F;

	int8_t svc_extension_flag = 0;
	int8_t avc_3d_extension_flag = 0;

	if (nal_unit_type == 14 || nal_unit_type == 20 || nal_unit_type == 21)
	{
		if (nal_unit_type != 21)
			svc_extension_flag = (pEBSPBuf[1] >> 7) & 0x01;
		else
			avc_3d_extension_flag = (pEBSPBuf[1] >> 7) & 0x01;

		if (svc_extension_flag)
		{
			nalUnitHeaderBytes += 3;
			if (read_buf_len < 5)
				return RET_CODE_NEEDMOREINPUT;
		}
		else if (avc_3d_extension_flag)
		{
			nalUnitHeaderBytes += 2;
			if (read_buf_len < 4)
				return RET_CODE_NEEDMOREINPUT;
		}
		else
		{
			nalUnitHeaderBytes += 3;
			if (read_buf_len < 5)
				return RET_CODE_NEEDMOREINPUT;
		}
	}

	if (nal_unit_type == BST::H264Video::CSD_PARTITION_B || nal_unit_type == BST::H264Video::CSD_PARTITION_C)
	{
		nu_entry.first_slice_segment_in_pic_flag = 0;
		nu_entry.no_output_of_prior_pics_flag = 0;
		nu_entry.slice_pic_parameter_set_id = -1;
		nu_entry.slice_type = -1;
		return RET_CODE_SUCCESS;
	}

	AMBst bs = AMBst_CreateFromBuffer(pEBSPBuf + nalUnitHeaderBytes, read_buf_len - nalUnitHeaderBytes);
	AMBst_SetRBSPType(bs, BST_RBSP_NAL_UNIT);

	try
	{
		uint32_t first_mb_in_slice = (uint32_t)AMBst_Get_ue(bs);
		nu_entry.slice_type = (int8_t)AMBst_Get_ue(bs);
		nu_entry.slice_pic_parameter_set_id = (int16_t)AMBst_Get_ue(bs);

		//printf("slice_pic_parameter_set_id: %d.\n", nu_entry.slice_pic_parameter_set_id);

		H264_NU pps, sps;
		if (nu_entry.slice_pic_parameter_set_id < 0 || 
			nu_entry.slice_pic_parameter_set_id > UINT8_MAX || !(pps = m_pNALAVCCtx->GetAVCPPS((uint8_t)nu_entry.slice_pic_parameter_set_id)))
		{
			nu_entry.slice_type = -2;
			goto done;
		}

		sps = m_pNALAVCCtx->GetAVCSPS(pps->ptr_pic_parameter_set_rbsp->seq_parameter_set_id);
		if (!sps)
		{
			nu_entry.slice_type = -2;
			goto done;
		}

		auto& seq_parameter_set_data = sps->ptr_seq_parameter_set_rbsp->seq_parameter_set_data;
		if (seq_parameter_set_data.separate_colour_plane_flag == 1)
			AMBst_SkipBits(bs, 2);	// Skip colour_plane_id

		nu_entry.frame_num = (uint16_t)AMBst_GetBits(bs, seq_parameter_set_data.log2_max_frame_num_minus4 + 4);
		nu_entry.frame_mbs_only_flag = seq_parameter_set_data.frame_mbs_only_flag;

		if (!seq_parameter_set_data.frame_mbs_only_flag)
		{
			nu_entry.field_pic_flag = (uint8_t)AMBst_GetBits(bs, 1);
			if (nu_entry.field_pic_flag)
				nu_entry.bottom_field_flag = (uint8_t)AMBst_GetBits(bs, 1);
		}
		else
		{
			nu_entry.field_pic_flag = 0;
			nu_entry.bottom_field_flag = 0;
		}

		if (nal_unit_type == 5)
			nu_entry.idr_pic_id = (uint16_t)AMBst_Get_ue(bs);

		nu_entry.pic_order_cnt_type = seq_parameter_set_data.pic_order_cnt_type;

		if (seq_parameter_set_data.pic_order_cnt_type == 0)
		{
			nu_entry.pic_order_cnt_lsb = (uint16_t)AMBst_GetBits(bs, seq_parameter_set_data.log2_max_pic_order_cnt_lsb_minus4 + 4);
			if (pps->ptr_pic_parameter_set_rbsp->bottom_field_pic_order_in_frame_present_flag && !nu_entry.field_pic_flag)
				nu_entry.delta_pic_order_cnt_bottom = (int32_t)AMBst_Get_se(bs);
			else
				nu_entry.delta_pic_order_cnt_bottom = 0;
		}
		else
			nu_entry.delta_pic_order_cnt_bottom = 0;

		if (seq_parameter_set_data.pic_order_cnt_type == 1 && !seq_parameter_set_data.delta_pic_order_always_zero_flag)
		{
			nu_entry.delta_pic_order_cnt_0 = (int32_t)AMBst_Get_se(bs);
			if (pps->ptr_pic_parameter_set_rbsp->bottom_field_pic_order_in_frame_present_flag && !nu_entry.field_pic_flag)
				nu_entry.delta_pic_order_cnt_1 = (int32_t)AMBst_Get_se(bs);
		}
		else
		{
			nu_entry.delta_pic_order_cnt_0 = 0;
			nu_entry.delta_pic_order_cnt_1 = 0;
		}
	}
	catch (...)
	{
		iRet = RET_CODE_ERROR;
	}

done:
	AMBst_Destroy(bs);
	return iRet;
}

int CNALParser::CommitNALUnit(uint8_t number_of_leading_bytes)
{
	int iRet = RET_CODE_SUCCESS;
	int read_buf_len = 0;
	int8_t nal_unit_type = -1;
	uint8_t* pEBSPBuf = AM_LRB_GetReadPtr(m_rbNALUnitEBSP, &read_buf_len);

	if (pEBSPBuf == NULL || read_buf_len <= 0)
		return RET_CODE_NEEDMOREINPUT;

	// Don't need analyze the Access-Unit
	if (!((m_nal_enum_options&(NAL_ENUM_OPTION_AU | NAL_ENUM_OPTION_CVS | NAL_ENUM_OPTION_VSEQ)) || ((m_nal_enum_options&MSE_ENUM_CMD_MASK) == MSE_ENUM_SYNTAX_VIEW)))
	{
		uint8_t* pNUBuf = pEBSPBuf;
		size_t cbNUBuf = (size_t)read_buf_len;
		if (m_nal_bytestream_format == NAL_BYTESTREAM_ISO)
		{
			pNUBuf += m_nal_length_delimiter_size;
			cbNUBuf -= m_nal_length_delimiter_size;
		}
		else if (m_nal_bytestream_format == NAL_BYTESTREAM_ANNEX_B)
		{
			pNUBuf += number_of_leading_bytes;
			cbNUBuf -= number_of_leading_bytes;
		}

		if (ParseNALUnit(pNUBuf, (int)cbNUBuf) == RET_CODE_ABORT)
			return RET_CODE_ABORT;

		AM_LRB_Reset(m_rbNALUnitEBSP);
		return RET_CODE_SUCCESS;
	}

	// Collect the NAL_UNIT information, and store it to m_nu_entries
	// Find the last nu_entries, and get the new NAL unit
	uint32_t offset = 0;
	if (m_nu_entries.size() > 0)
		offset = m_nu_entries.back().NU_offset + m_nu_entries.back().NU_length;

	if (read_buf_len < 0 || (uint32_t)read_buf_len < offset)
	{
		printf("The NAL-Unit ring buffers is inconsistent with NU entries.\n");
		return RET_CODE_ERROR;
	}

	if ((uint32_t)read_buf_len == offset)
		return RET_CODE_NEEDMOREINPUT;

	uint8_t count_of_leading_bytes = 0;
	uint32_t NAL_Unit_Len = (uint32_t)(read_buf_len - offset);

	//printf("NAL_Unit_len: %lu\n", NAL_Unit_Len);

	//if (NAL_Unit_Len != m_cur_submit_nal_unit_len)
	//	printf("unexpected.\n");

	NAL_UNIT_ENTRY nu_entry;
	nu_entry.NU_offset = offset;
	nu_entry.NU_length = NAL_Unit_Len;

	uint8_t* p = pEBSPBuf + offset;
	
	if (m_nal_bytestream_format == NAL_BYTESTREAM_ISO)
	{
		count_of_leading_bytes = m_nal_length_delimiter_size;
		p += m_nal_length_delimiter_size;
	}
	else if (m_nal_bytestream_format == NAL_BYTESTREAM_ANNEX_B)
	{
		count_of_leading_bytes = number_of_leading_bytes;
		p += number_of_leading_bytes;
	}
	else if(m_nal_bytestream_format != NAL_BYTESTREAM_RAW)
		return RET_CODE_ERROR_NOTIMPL;

	m_count_nal_unit_scanned++;

	// Ok, now nal_unit is extracted successfully
	if (m_nal_coding == NAL_CODING_AVC)
	{
		int8_t forbidden_zero_bit = (p[0] >> 7) & 0x01;
		int8_t nal_ref_idc = (p[0] >> 5) & 0x03;
		
		nal_unit_type = p[0] & 0x1F;

		// Record the current nal_unit_header information and file offset
		m_nu_entries.emplace_back(m_cur_submit_pos, offset, NAL_Unit_Len, count_of_leading_bytes, forbidden_zero_bit, nal_ref_idc, nal_unit_type);

		if (IS_AVC_PARAMETERSET_NAL(nal_unit_type))
		{
			// Parsing the ebsp parameter set, and store it
			iRet = LoadAVCParameterSet(p, NAL_Unit_Len - count_of_leading_bytes, m_cur_submit_pos);
		}
		else if (IS_AVC_VCL_NAL(nal_unit_type))
		{
			// Parsing a part of slice header
			if (AMP_SUCCEEDED(PickupLastSliceHeaderInfo(p, NAL_Unit_Len - count_of_leading_bytes)))
			{
				// Commit the slice information, during committing, picture information may be generated
				iRet = CommitSliceInfo(false);
			}
		}
	}
	else if (m_nal_coding == NAL_CODING_HEVC)
	{
		int8_t forbidden_zero_bit = (p[0] >> 7) & 0x01;
		int8_t nuh_layer_id = ((p[0] & 0x01) << 5) | ((p[1] >> 3) & 0x1F);
		int8_t nuh_temporal_id_plus1 = p[0 + 1] & 0x07;

		nal_unit_type = (p[0] >> 1) & 0x3F;

		// Record the current nal_unit_header information and file offset
		m_nu_entries.emplace_back(m_cur_submit_pos, offset, NAL_Unit_Len, count_of_leading_bytes, forbidden_zero_bit, nal_unit_type, nuh_layer_id, nuh_temporal_id_plus1);

		if (IS_HEVC_PARAMETERSET_NAL(nal_unit_type))
		{
			// Parsing the ebsp parameter set, and store it
			iRet = LoadHEVCParameterSet(p, NAL_Unit_Len - count_of_leading_bytes, m_cur_submit_pos);
		}
		else if (IS_HEVC_VCL_NAL(nal_unit_type))
		{
			// Parsing a part of slice header
			if (AMP_SUCCEEDED(PickupLastSliceHeaderInfo(p, NAL_Unit_Len - count_of_leading_bytes)))
			{
				// Commit the slice information, during committing, picture information may be generated
				iRet = CommitSliceInfo(false);
			}
		}
	}
	else if (m_nal_coding == NAL_CODING_VVC)
	{
		int8_t forbidden_zero_bit = (p[0] >> 7) & 0x01;
		int8_t nuh_reserved_zero_bit = (p[0] >> 6) & 0x01;
		int8_t nuh_layer_id = p[0] & 0x3F;
		int8_t nuh_temporal_id_plus1 = p[1] & 0x07;

		nal_unit_type = (p[1] >> 3) & 0x1F;

		// Record the current nal_unit_header information and file offset
		m_nu_entries.emplace_back(m_cur_submit_pos, offset, NAL_Unit_Len, count_of_leading_bytes, forbidden_zero_bit, nuh_reserved_zero_bit, nal_unit_type, nuh_layer_id, nuh_temporal_id_plus1);

		// When hitting the below NAL unit(s),it means a new AU starts, and reset the flag
		// m_hit_PH_NU_in_one_AU until a new PH_NU is hit
		if (nal_unit_type == VVC_AUD_NUT ||
			nal_unit_type == VVC_OPI_NUT ||
			nal_unit_type == VVC_DCI_NUT ||
			nal_unit_type == VVC_VPS_NUT ||
			nal_unit_type == VVC_SPS_NUT ||
			nal_unit_type == VVC_PPS_NUT ||
			nal_unit_type == VVC_PREFIX_APS_NUT)
			m_hit_PH_NU_in_one_AU = false;

		if (IS_VVC_PARAMETERSET_NAL(nal_unit_type))
		{
			// Parsing the ebsp parameter set, and store it
			iRet = LoadVVCParameterSet(p, NAL_Unit_Len - count_of_leading_bytes, m_cur_submit_pos);
		}
		else if (nal_unit_type == VVC_PH_NUT)
		{
			m_hit_PH_NU_in_one_AU = true;
			iRet = LoadVVCParameterSet(p, NAL_Unit_Len - count_of_leading_bytes, m_cur_submit_pos);
		}
		else if (IS_VVC_VCL_NAL(nal_unit_type) && nal_unit_type != VVC_RSV_VCL_4 && nal_unit_type != VVC_RSV_VCL_5 && nal_unit_type != VVC_RSV_VCL_6 && nal_unit_type != VVC_RSV_IRAP_11)
		{
			// Parsing a part of slice header
			if (AMP_SUCCEEDED(PickupLastSliceHeaderInfo(p, NAL_Unit_Len - count_of_leading_bytes)))
			{
				// Commit the slice information, during committing, picture information may be generated
				iRet = CommitSliceInfo(false);
			}

			m_hit_PH_NU_in_one_AU = false;
		}
	}
	else
	{
		goto done;
		iRet = RET_CODE_ERROR_NOTIMPL;
	}

done:
	return iRet;
}

int CNALParser::CommitSliceInfo(bool bDrain)
{
	int iRet = RET_CODE_SUCCESS;
	auto& back = m_nu_entries.back();

	if (!back.first_slice_segment_in_pic_flag && !bDrain)
		return RET_CODE_SUCCESS;

	// For the first round, find the last NAL unit before firstBlPicNalUnit
	auto iter_last_VCL_NAL_unit = m_nu_entries.cend();
	auto iter_begin = m_nu_entries.cbegin();
	auto iter_firstBlPicNalUnit = m_nu_entries.cend();
	auto iter = iter_begin;

	do
	{
		for (; iter != m_nu_entries.cend(); iter++)
		{
			if (iter->first_slice_segment_in_pic_flag && iter_last_VCL_NAL_unit != m_nu_entries.cend())
			{
				iter_firstBlPicNalUnit = iter;
				break;
			}

			if ((m_nal_coding == NAL_CODING_HEVC && IS_HEVC_VCL_NAL(iter->nal_unit_type_hevc)) ||
				(m_nal_coding == NAL_CODING_AVC  && IS_AVC_VCL_NAL(iter->nal_unit_type_avc)) ||
				(m_nal_coding == NAL_CODING_VVC  && IS_VVC_VCL_NAL(iter->nal_unit_type_vvc)))
				iter_last_VCL_NAL_unit = iter;
		}

		if ((iter_last_VCL_NAL_unit == m_nu_entries.cend() ||
			iter_firstBlPicNalUnit == m_nu_entries.cend()) && !bDrain)
			break;

		for (iter = iter_last_VCL_NAL_unit; iter != iter_firstBlPicNalUnit; iter++)
		{
			// find the access unit boundary according H.264 spec: 7.4.1.2.3 Order of NAL units and coded pictures and association to access units
			if (m_nal_coding == NAL_CODING_AVC && (
				iter->nal_unit_type_avc == BST::H264Video::AUD_NUT ||
				iter->nal_unit_type_avc == BST::H264Video::SPS_NUT ||
				iter->nal_unit_type_avc == BST::H264Video::PPS_NUT ||
				iter->nal_unit_type_avc == BST::H264Video::SEI_NUT || (
				iter->nal_unit_type_avc >= BST::H264Video::PREFIX_NUT && iter->nal_unit_type_avc <= 18)))
				break;
			// find the access unit boundary according to 7.4.2.4.4 Order of NAL units and coded pictures and their association to access units
			else if (m_nal_coding == NAL_CODING_HEVC && iter->nuh_layer_id_hevc == 0 && (
				iter->nal_unit_type_hevc == BST::H265Video::AUD_NUT ||
				iter->nal_unit_type_hevc == BST::H265Video::VPS_NUT ||
				iter->nal_unit_type_hevc == BST::H265Video::SPS_NUT ||
				iter->nal_unit_type_hevc == BST::H265Video::PPS_NUT ||
				iter->nal_unit_type_hevc == BST::H265Video::PREFIX_SEI_NUT || (
				iter->nal_unit_type_hevc >= BST::H265Video::RSV_NVCL41 && iter->nal_unit_type_hevc <= BST::H265Video::RSV_NVCL41 + 3) || (
				iter->nal_unit_type_hevc >= BST::H265Video::UNSPEC48   && iter->nal_unit_type_hevc <= BST::H265Video::UNSPEC48 + 7)))
				break;
			// 7.4.2.4.4 Order of NAL units and coded pictures and their association to PUs
			else if (m_nal_coding == NAL_CODING_VVC && (
				iter->nal_unit_type_vvc == BST::H266Video::AUD_NUT ||
				iter->nal_unit_type_vvc == BST::H266Video::OPI_NUT ||
				iter->nal_unit_type_vvc == BST::H266Video::DCI_NUT ||
				iter->nal_unit_type_vvc == BST::H266Video::VPS_NUT ||
				iter->nal_unit_type_vvc == BST::H266Video::SPS_NUT ||
				iter->nal_unit_type_vvc == BST::H266Video::PPS_NUT ||
				iter->nal_unit_type_vvc == BST::H266Video::PREFIX_APS_NUT ||
				iter->nal_unit_type_vvc == BST::H266Video::PH_NUT ||
				iter->nal_unit_type_vvc == BST::H266Video::PREFIX_SEI_NUT ||
				iter->nal_unit_type_vvc == BST::H266Video::RSV_NVCL_26 ||
				iter->nal_unit_type_vvc == BST::H266Video::UNSPEC_28 || iter->nal_unit_type_vvc == BST::H266Video::UNSPEC_29))
				break;
		}

		if (iter_last_VCL_NAL_unit != m_nu_entries.cend())
		{
			if (m_nal_coding == NAL_CODING_AVC)
				iRet = CommitAVCPicture(iter_begin, iter);
			else if (m_nal_coding == NAL_CODING_HEVC)
				iRet = CommitHEVCPicture(iter_begin, iter);
			else if (m_nal_coding == NAL_CODING_VVC)
				iRet = CommitVVCPicture(iter_begin, iter);

			if (iRet == RET_CODE_ABORT)
				goto done;
		}

		iter_firstBlPicNalUnit = iter_last_VCL_NAL_unit = m_nu_entries.cend();

		iter_begin = iter;

	} while (iter_begin != m_nu_entries.cend());

	// Skip the read buffer which is already parsed
	if (iter_begin == m_nu_entries.cend())
	{
		AM_LRB_Reset(m_rbNALUnitEBSP);
		m_nu_entries.clear();
	}
	else
	{
		uint32_t num_of_advanced_bytes = iter_begin->NU_offset;

		AM_LRB_SkipReadPtr(m_rbNALUnitEBSP, num_of_advanced_bytes);

		// Erase the related NAL Unit Entries
		m_nu_entries.erase(m_nu_entries.begin(), iter_begin);

		// Need update the NU offset/length
		for (auto iter = m_nu_entries.begin(); iter != m_nu_entries.end(); iter++)
			iter->NU_offset -= num_of_advanced_bytes;
	}

done:
	return iRet;
}

int CNALParser::CommitVVCPicture(
	std::vector<NAL_UNIT_ENTRY>::const_iterator pic_start,
	std::vector<NAL_UNIT_ENTRY>::const_iterator pic_end)
{
	int iRet = RET_CODE_SUCCESS;
	uint8_t picture_type = PIC_TYPE_UNKNOWN;

	if (pic_end <= pic_start)
		return RET_CODE_NEEDMOREINPUT;

	int read_buf_len = 0;
	uint8_t* pEBSPBuf = AM_LRB_GetReadPtr(m_rbNALUnitEBSP, &read_buf_len);

	uint8_t* pAUBufStart = pEBSPBuf + pic_start->NU_offset;
	uint8_t* pAUBufEnd = pEBSPBuf + (pic_end == m_nu_entries.cend() ? read_buf_len : pic_end->NU_offset);

	// Get the first VCL NAL Unit in base layer
	int16_t slice_pic_parameter_set_id = -1;
	uint8_t nal_unit_type = 0XFF, picture_slice_type = 3;
	auto firstBlPicNalUnit = pic_end;
	bool bCVSChange = false, bHasVPS = false, bHasSPS = false;
	int8_t represent_nal_unit_type = -1, first_I_nal_unit_type = -1;

	for (auto iter = pic_start; iter != pic_end; iter++)
	{
		if (iter->nal_unit_type_vvc == BST::H266Video::VPS_NUT)
			bHasVPS = true;
		else if (iter->nal_unit_type_vvc == BST::H266Video::SPS_NUT)
			bHasSPS = true;

		// At present, only parsing base-layer NAL unit in advance
		if (!IS_VVC_VCL_NAL(iter->nal_unit_type_vvc) || iter->nuh_layer_id_vvc != 0)
			continue;

		if (represent_nal_unit_type == -1 && (IS_VVC_IDR(iter->nal_unit_type_vvc) || IS_VVC_CRA(iter->nal_unit_type_vvc)))
		{
			represent_nal_unit_type = (int8_t)iter->nal_unit_type_vvc;
			bCVSChange = true;
		}

		if (slice_pic_parameter_set_id == -1)
		{
			firstBlPicNalUnit = iter;
			slice_pic_parameter_set_id = iter->slice_pic_parameter_set_id;
		}
		else if (slice_pic_parameter_set_id != iter->slice_pic_parameter_set_id)
		{
			// Break the spec, record it to error table
			printf("[NALParser] slice_pic_parameter_set_id of all VCL NAL Units shall be the same in a coded picture.\n");
			if (m_nal_enum)
				m_nal_enum->EnumNALError(m_pCtx, iter->file_offset - iter->leading_bytes, 3);
			break;
		}

		if (nal_unit_type == 0xFF)
			nal_unit_type = iter->nal_unit_type_vvc;
		else if (nal_unit_type != iter->nal_unit_type_vvc)
		{
			// Break the spec, record it to error table
			printf("[NALParser] nal_unit_type of all VCL NAL Units shall be the same in a coded picture.\n");
			if (m_nal_enum)
				m_nal_enum->EnumNALError(m_pCtx, iter->file_offset - iter->leading_bytes, 4);
		}

		if (iter->slice_type == BST::H266Video::B_SLICE)
			picture_slice_type = 0;
		else if (iter->slice_type == BST::H266Video::P_SLICE && picture_slice_type >= 2)
			picture_slice_type = 1;
		else if (iter->slice_type == BST::H266Video::I_SLICE && picture_slice_type == 3)
		{
			first_I_nal_unit_type = iter->nal_unit_type_vvc;
			picture_slice_type = 2;
		}
	}

	if (picture_slice_type == 2 && bCVSChange == false && bHasVPS && bHasSPS)
	{
		represent_nal_unit_type = first_I_nal_unit_type;
		bCVSChange = true;
	}

	// Judge the current picture type
	if (IS_VVC_IDR(nal_unit_type))
		picture_type = PIC_TYPE_IDR;
	else if (IS_VVC_CRA(nal_unit_type))
		picture_type = PIC_TYPE_CRA;
	else if (IS_VVC_IRAP(nal_unit_type))
		picture_type = PIC_TYPE_IRAP;
	else if (IS_VVC_LEADING(nal_unit_type))
		picture_type = PIC_TYPE_LEADING;
	else if (IS_VVC_TRAILING(nal_unit_type))
		picture_type = PIC_TYPE_TRAILING;

	// Update EP_MAP
	auto byte_stream_nal_unit_start_pos = pic_start->file_offset - pic_start->leading_bytes;

	uint16_t pic_width_in_luma_samples = 0;
	uint16_t pic_height_in_luma_samples = 0;
	uint8_t aspect_ratio_info_present_flag = 0, colour_description_present_flag = 0, timing_info_present_flag = 0, units_field_based_flag = 0;
	uint8_t aspect_ratio_idc = 0;
	uint16_t sar_width = 0, sar_height = 0;
	uint8_t colour_primaries = 0, transfer_characteristics = 0, matrix_coeffs = 0;
	uint32_t num_units_in_tick = 0, time_scale = 0;

	// Check PPS reference is changed or not
	bool bSequenceChange = false;

	if (slice_pic_parameter_set_id < 0 || slice_pic_parameter_set_id >= 64)
	{
		// Break the spec, record it to error table
		printf("[NALParser] No available slice_pic_parameter_set_id for the current coded picture.\n");
		if (m_nal_enum)
			m_nal_enum->EnumNALError(m_pCtx, pic_start->file_offset - pic_start->leading_bytes, 4);
		goto done;
	}

	if (firstBlPicNalUnit == pic_end)
		printf("[NALParser] There is no base-layer VCL NAL Unit in the current code picture unit.\n");
	else
	{
		// Check the sequence is changed or not
		H266_NU sp_pps, sp_sps, sp_vps;
		if (slice_pic_parameter_set_id < 0 ||
			slice_pic_parameter_set_id > UINT8_MAX ||
			!(sp_pps = m_pNALVVCCtx->GetVVCPPS((uint8_t)slice_pic_parameter_set_id)) ||
			sp_pps->ptr_pic_parameter_set_rbsp == nullptr)
		{
			printf("[NALParser] The current picture unit is NOT associated with an available SPS.\n");
			goto done;
		}

		sp_sps = m_pNALVVCCtx->GetVVCSPS(sp_pps->ptr_pic_parameter_set_rbsp->pps_seq_parameter_set_id);
		if (!sp_sps || sp_sps->ptr_seq_parameter_set_rbsp == nullptr)
		{
			printf("[NALParser] The current picture unit is NOT associated with an available SPS.\n");
			goto done;
		}

		m_pNALVVCCtx->ActivateSPS((int8_t)sp_pps->ptr_pic_parameter_set_rbsp->pps_seq_parameter_set_id);

		sp_vps = m_pNALVVCCtx->GetVVCVPS(sp_sps->ptr_seq_parameter_set_rbsp->sps_video_parameter_set_id);

		pic_width_in_luma_samples = sp_sps->ptr_seq_parameter_set_rbsp->sps_pic_width_max_in_luma_samples;
		pic_height_in_luma_samples = sp_sps->ptr_seq_parameter_set_rbsp->sps_pic_height_max_in_luma_samples;
		if (sp_vps &&
			sp_vps->ptr_video_parameter_set_rbsp != nullptr &&
			sp_vps->ptr_video_parameter_set_rbsp->vps_timing_hrd_params_present_flag)
		{
			vvc_num_units_in_tick = sp_vps->ptr_video_parameter_set_rbsp->general_timing_hrd_parameters->num_units_in_tick;
			vvc_time_scale = sp_vps->ptr_video_parameter_set_rbsp->general_timing_hrd_parameters->time_scale;
		}

		timing_info_present_flag = sp_sps->ptr_seq_parameter_set_rbsp->sps_timing_hrd_params_present_flag;
		if (timing_info_present_flag)
		{
			num_units_in_tick = sp_sps->ptr_seq_parameter_set_rbsp->general_timing_hrd_parameters->num_units_in_tick;
			time_scale = sp_sps->ptr_seq_parameter_set_rbsp->general_timing_hrd_parameters->time_scale;

			vvc_num_units_in_tick = num_units_in_tick;
			vvc_time_scale = time_scale;
		}

		if (sp_sps->ptr_seq_parameter_set_rbsp->sps_vui_parameters_present_flag)
		{
			vui_progressive_source_flag = sp_sps->ptr_seq_parameter_set_rbsp->vui_payload->vui_parameters.vui_progressive_source_flag;
			vui_interlaced_source_flag  = sp_sps->ptr_seq_parameter_set_rbsp->vui_payload->vui_parameters.vui_interlaced_source_flag;
			aspect_ratio_info_present_flag = sp_sps->ptr_seq_parameter_set_rbsp->vui_payload->vui_parameters.vui_aspect_ratio_info_present_flag;
			if (aspect_ratio_info_present_flag)
			{
				aspect_ratio_idc = sp_sps->ptr_seq_parameter_set_rbsp->vui_payload->vui_parameters.vui_aspect_ratio_idc;
				if (aspect_ratio_idc == 0xFF)
				{
					sar_width = sp_sps->ptr_seq_parameter_set_rbsp->vui_payload->vui_parameters.vui_sar_width;
					sar_height = sp_sps->ptr_seq_parameter_set_rbsp->vui_payload->vui_parameters.vui_sar_height;
				}
			}

			colour_description_present_flag = sp_sps->ptr_seq_parameter_set_rbsp->vui_payload->vui_parameters.vui_colour_description_present_flag;
			if (colour_description_present_flag)
			{
				colour_primaries = sp_sps->ptr_seq_parameter_set_rbsp->vui_payload->vui_parameters.vui_colour_primaries;
				transfer_characteristics = sp_sps->ptr_seq_parameter_set_rbsp->vui_payload->vui_parameters.vui_transfer_characteristics;
				matrix_coeffs = sp_sps->ptr_seq_parameter_set_rbsp->vui_payload->vui_parameters.vui_matrix_coeffs;
			}
		}
		else
		{
			vui_progressive_source_flag = 0;
			vui_interlaced_source_flag = 0;
		}

		// check whether the same with the current sequence
		if (nal_sequences.size() <= 0)
			bSequenceChange = true;
		else
		{
			auto& back = nal_sequences.back();
			if (back.pic_width_in_luma_samples != pic_width_in_luma_samples ||
				back.pic_height_in_luma_samples != pic_height_in_luma_samples)
				bSequenceChange = true;
			else
			{
				if (aspect_ratio_info_present_flag == 1 && back.aspect_ratio_info_present_flag == 1)
				{
					if (aspect_ratio_idc != back.aspect_ratio_idc)
						bSequenceChange = true;
					else if (aspect_ratio_idc == 0xFF && sar_height != 0 && back.sar_height != 0)
						bSequenceChange = sar_height * back.sar_width != sar_width * back.sar_height ? true : false;
				}

				if (bSequenceChange == false && colour_description_present_flag == 1 && back.colour_description_present_flag == 1)
				{
					if (colour_primaries != back.colour_primaries ||
						transfer_characteristics != back.transfer_characteristics ||
						matrix_coeffs != back.matrix_coeffs)
						bSequenceChange = true;
				}

				if (bSequenceChange == false && timing_info_present_flag == 1 && back.timing_info_present_flag == 1)
				{
					if (num_units_in_tick*back.time_scale != time_scale * back.num_units_in_tick)
						bSequenceChange = true;
				}
			}
		}
	}

	if (bSequenceChange)
	{
		nal_sequences.emplace_back();
		auto& vvc_seq = nal_sequences.back();
		memset(&vvc_seq, 0, sizeof(vvc_seq));
		vvc_seq.start_byte_pos = byte_stream_nal_unit_start_pos;

		vvc_seq.pic_width_in_luma_samples = pic_width_in_luma_samples;
		vvc_seq.pic_height_in_luma_samples = pic_height_in_luma_samples;

		vvc_seq.aspect_ratio_info_present_flag = aspect_ratio_info_present_flag;
		if (aspect_ratio_info_present_flag)
		{
			vvc_seq.aspect_ratio_idc = aspect_ratio_idc;
			if (aspect_ratio_idc == 0xFF)
			{
				vvc_seq.sar_width = sar_width;
				vvc_seq.sar_height = sar_height;
			}
		}

		vvc_seq.colour_description_present_flag = colour_description_present_flag;
		if (colour_description_present_flag)
		{
			vvc_seq.colour_primaries = colour_primaries;
			vvc_seq.transfer_characteristics = transfer_characteristics;
			vvc_seq.matrix_coeffs = matrix_coeffs;
		}

		vvc_seq.timing_info_present_flag = timing_info_present_flag;
		if (timing_info_present_flag)
		{
			vvc_seq.num_units_in_tick = num_units_in_tick;
			vvc_seq.time_scale = time_scale;
			vvc_seq.units_field_based_flag = units_field_based_flag;
		}

		// if there is no frame-rate information, don't set the presentation_start_time for the current VVC sequence
		if (vvc_time_scale == 0 || vvc_presentation_time_code < 0)
			vvc_seq.presentation_start_time = 0xFFFFFFFF;
		else
			vvc_seq.presentation_start_time = (uint32_t)(vvc_presentation_time_code / 10000LL);
	}

	// Add all related pps_pic_parameter_set_id
	nal_sequences.back().pic_parameter_set_id_sel[slice_pic_parameter_set_id] = true;

	if (vvc_time_scale != 0 && vvc_presentation_time_code >= 0)
	{
		bool bOnlyProgressive = vui_progressive_source_flag && !vui_interlaced_source_flag;
		bool bOnlyInterlace = !vui_progressive_source_flag && vui_interlaced_source_flag;

		if (bOnlyProgressive)
			vvc_presentation_time_code += (int64_t)vvc_num_units_in_tick * 10000000LL / vvc_time_scale;
		else if (bOnlyInterlace)
			vvc_presentation_time_code += (int64_t)vvc_num_units_in_tick * 10000000LL * 2 / vvc_time_scale;
		else
			vvc_presentation_time_code = -1LL;
	}
	else
		vvc_presentation_time_code = -1LL;

	if (m_nal_enum)
	{
		uint8_t* pAUBuf = pAUBufStart;
		size_t cbAUBuf = (size_t)(pAUBufEnd - pAUBufStart);

		if (bSequenceChange && (m_nal_enum_options&NAL_ENUM_OPTION_VSEQ) && AMP_FAILED(m_nal_enum->EnumNewVSEQ(m_pCtx)))
		{
			iRet = RET_CODE_ABORT;
			goto done;
		}

		if (bCVSChange && (m_nal_enum_options&NAL_ENUM_OPTION_CVS) && AMP_FAILED(m_nal_enum->EnumNewCVS(m_pCtx, represent_nal_unit_type)))
		{
			iRet = RET_CODE_ABORT;
			goto done;
		}

		//if (pic_start->nal_unit_type != AVC_AUD_NUT)
		//	printf("file position: %" PRIu64 "\n", pic_start->file_offset);
		if ((m_nal_enum_options&NAL_ENUM_OPTION_AU) && AMP_FAILED(m_nal_enum->EnumNALAUBegin(m_pCtx, pAUBuf, cbAUBuf, picture_slice_type)))
		{
			iRet = RET_CODE_ABORT;
			goto done;
		}

		for (auto iter = pic_start; iter != pic_end; iter++)
		{
			uint8_t* pNALUnitBuf = pEBSPBuf + iter->NU_offset;
			pNALUnitBuf += iter->leading_bytes;

			if (ParseNALUnit(pNALUnitBuf, iter->NU_length - iter->leading_bytes) == RET_CODE_ABORT)
			{
				iRet = RET_CODE_ABORT;
				goto done;
			}
		}

		if ((m_nal_enum_options&NAL_ENUM_OPTION_AU) && AMP_FAILED(m_nal_enum->EnumNALAUEnd(m_pCtx, pAUBuf, cbAUBuf)))
		{
			iRet = RET_CODE_ABORT;
			goto done;
		}
	}

done:
	return iRet;
}

int CNALParser::CommitHEVCPicture(
	std::vector<NAL_UNIT_ENTRY>::const_iterator pic_start,
	std::vector<NAL_UNIT_ENTRY>::const_iterator pic_end)
{
	int iRet = RET_CODE_SUCCESS;
	uint8_t picture_type = PIC_TYPE_UNKNOWN;

	if (pic_end <= pic_start)
		return RET_CODE_NEEDMOREINPUT;

	int read_buf_len = 0;
	uint8_t* pEBSPBuf = AM_LRB_GetReadPtr(m_rbNALUnitEBSP, &read_buf_len);

	uint8_t* pAUBufStart = pEBSPBuf + pic_start->NU_offset;
	uint8_t* pAUBufEnd = pEBSPBuf + (pic_end == m_nu_entries.cend() ? read_buf_len : pic_end->NU_offset);
	
	// Get the first VCL NAL Unit in base layer
	int16_t slice_pic_parameter_set_id = -1;
	uint8_t nal_unit_type = 0XFF, picture_slice_type = 3;
	auto firstBlPicNalUnit = pic_end;
	bool bCVSChange = false, bHasVPS = false, bHasSPS = false;
	int8_t represent_nal_unit_type = -1, first_I_nal_unit_type = -1;

	for (auto iter = pic_start; iter != pic_end; iter++)
	{
		if (iter->nal_unit_type_hevc == BST::H265Video::VPS_NUT)
			bHasVPS = true;
		else if (iter->nal_unit_type_hevc == BST::H265Video::SPS_NUT)
			bHasSPS = true;

		// At present, only parsing base-layer NAL unit in advance
		if (!IS_HEVC_VCL_NAL(iter->nal_unit_type_hevc) || iter->nuh_layer_id_hevc != 0)
			continue;

		if (represent_nal_unit_type == -1 && (IS_BLA(iter->nal_unit_type_hevc) || IS_IDR(iter->nal_unit_type_hevc) || IS_CRA(iter->nal_unit_type_hevc)))
		{
			represent_nal_unit_type = (int8_t)iter->nal_unit_type_hevc;
			bCVSChange = true;
		}

		if (slice_pic_parameter_set_id == -1)
		{
			firstBlPicNalUnit = iter;
			slice_pic_parameter_set_id = iter->slice_pic_parameter_set_id;
		}
		else if (slice_pic_parameter_set_id != iter->slice_pic_parameter_set_id)
		{
			// Break the spec, record it to error table
			printf("[NALParser] slice_pic_parameter_set_id of all VCL NAL Units shall be the same in a coded picture.\n");
			if (m_nal_enum)
				m_nal_enum->EnumNALError(m_pCtx, iter->file_offset - iter->leading_bytes, 3);
			break;
		}

		if (nal_unit_type == 0xFF)
			nal_unit_type = iter->nal_unit_type_hevc;
		else if (nal_unit_type != iter->nal_unit_type_hevc)
		{
			// Break the spec, record it to error table
			printf("[NALParser] nal_unit_type of all VCL NAL Units shall be the same in a coded picture.\n");
			if (m_nal_enum)
				m_nal_enum->EnumNALError(m_pCtx, iter->file_offset - iter->leading_bytes, 4);
		}

		if (iter->slice_type == BST::H265Video::B_SLICE)
			picture_slice_type = 0;
		else if (iter->slice_type == BST::H265Video::P_SLICE && picture_slice_type >= 2)
			picture_slice_type = 1;
		else if (iter->slice_type == BST::H265Video::I_SLICE && picture_slice_type == 3)
		{
			first_I_nal_unit_type = iter->nal_unit_type_hevc;
			picture_slice_type = 2;
		}
	}

	if (picture_slice_type == 2 && bCVSChange == false && bHasVPS && bHasSPS)
	{
		represent_nal_unit_type = first_I_nal_unit_type;
		bCVSChange = true;
	}

	// Judge the current picture type
	if (IS_IDR(nal_unit_type))
		picture_type = PIC_TYPE_IDR;
	else if (IS_CRA(nal_unit_type))
		picture_type = PIC_TYPE_CRA;
	else if (IS_IRAP(nal_unit_type))
		picture_type = PIC_TYPE_IRAP;
	else if (IS_LEADING(nal_unit_type))
		picture_type = PIC_TYPE_LEADING;
	else if (IS_TRAILING(nal_unit_type))
		picture_type = PIC_TYPE_TRAILING;

	// Update EP_MAP
	auto byte_stream_nal_unit_start_pos = pic_start->file_offset - pic_start->leading_bytes;

	uint16_t pic_width_in_luma_samples = 0;
	uint16_t pic_height_in_luma_samples = 0;
	uint8_t aspect_ratio_info_present_flag = 0, colour_description_present_flag = 0, vui_timing_info_present_flag = 0, units_field_based_flag = 0;
	uint8_t aspect_ratio_idc = 0;
	uint16_t sar_width = 0, sar_height = 0;
	uint8_t colour_primaries = 0, transfer_characteristics = 0, matrix_coeffs = 0;
	uint32_t vui_num_units_in_tick = 0, vui_time_scale = 0;

	// Check PPS reference is changed or not
	bool bSequenceChange = false;

	if (slice_pic_parameter_set_id < 0 || slice_pic_parameter_set_id >= 64)
	{
		// Break the spec, record it to error table
		printf("[NALParser] No available slice_pic_parameter_set_id for the current coded picture.\n");
		if (m_nal_enum)
			m_nal_enum->EnumNALError(m_pCtx, pic_start->file_offset - pic_start->leading_bytes, 4);
		goto done;
	}

	if (firstBlPicNalUnit == pic_end)
		printf("[NALParser] There is no base-layer VCL NAL Unit in the current code picture unit.\n");
	else
	{
		// Check the sequence is changed or not
		H265_NU sp_pps, sp_sps, sp_vps;
		if (slice_pic_parameter_set_id < 0 ||
			slice_pic_parameter_set_id > UINT8_MAX || 
			!(sp_pps = m_pNALHEVCCtx->GetHEVCPPS((uint8_t)slice_pic_parameter_set_id)) || 
			sp_pps->ptr_pic_parameter_set_rbsp == nullptr)
		{
			printf("[NALParser] The current picture unit is NOT associated with an available SPS.\n");
			goto done;
		}

		sp_sps = m_pNALHEVCCtx->GetHEVCSPS(sp_pps->ptr_pic_parameter_set_rbsp->pps_seq_parameter_set_id);
		if (!sp_sps || sp_sps->ptr_seq_parameter_set_rbsp == nullptr)
		{
			printf("[NALParser] The current picture unit is NOT associated with an available SPS.\n");
			goto done;
		}

		m_pNALHEVCCtx->ActivateSPS((int8_t)sp_pps->ptr_pic_parameter_set_rbsp->pps_seq_parameter_set_id);
		
		sp_vps = m_pNALHEVCCtx->GetHEVCVPS(sp_sps->ptr_seq_parameter_set_rbsp->sps_video_parameter_set_id);

		pic_width_in_luma_samples = sp_sps->ptr_seq_parameter_set_rbsp->pic_width_in_luma_samples;
		pic_height_in_luma_samples = sp_sps->ptr_seq_parameter_set_rbsp->pic_height_in_luma_samples;

		if (sp_vps &&
			sp_vps->ptr_video_parameter_set_rbsp != nullptr &&
			sp_vps->ptr_video_parameter_set_rbsp->vps_timing_info_present_flag)
		{
			hevc_num_units_in_tick = sp_vps->ptr_video_parameter_set_rbsp->vps_num_units_in_tick;
			hevc_time_scale = sp_vps->ptr_video_parameter_set_rbsp->vps_time_scale;
		}

		if (sp_sps->ptr_seq_parameter_set_rbsp->vui_parameters)
		{
			aspect_ratio_info_present_flag = sp_sps->ptr_seq_parameter_set_rbsp->vui_parameters->aspect_ratio_info_present_flag;
			if (aspect_ratio_info_present_flag)
			{
				aspect_ratio_idc = sp_sps->ptr_seq_parameter_set_rbsp->vui_parameters->aspect_ratio_idc;
				if (aspect_ratio_idc == 0xFF)
				{
					sar_width = sp_sps->ptr_seq_parameter_set_rbsp->vui_parameters->sar_width;
					sar_height = sp_sps->ptr_seq_parameter_set_rbsp->vui_parameters->sar_height;
				}
			}

			colour_description_present_flag = sp_sps->ptr_seq_parameter_set_rbsp->vui_parameters->colour_description_present_flag;
			if (colour_description_present_flag)
			{
				colour_primaries = sp_sps->ptr_seq_parameter_set_rbsp->vui_parameters->colour_primaries;
				transfer_characteristics = sp_sps->ptr_seq_parameter_set_rbsp->vui_parameters->transfer_characteristics;
				matrix_coeffs = sp_sps->ptr_seq_parameter_set_rbsp->vui_parameters->matrix_coeffs;
			}

			vui_timing_info_present_flag = sp_sps->ptr_seq_parameter_set_rbsp->vui_parameters->vui_timing_info_present_flag;
			if (vui_timing_info_present_flag)
			{
				vui_num_units_in_tick = sp_sps->ptr_seq_parameter_set_rbsp->vui_parameters->vui_num_units_in_tick;
				vui_time_scale = sp_sps->ptr_seq_parameter_set_rbsp->vui_parameters->vui_time_scale;
				units_field_based_flag = sp_sps->ptr_seq_parameter_set_rbsp->vui_parameters->field_seq_flag == 1 ? 1 : 0;

				hevc_num_units_in_tick = vui_num_units_in_tick;
				hevc_time_scale = vui_time_scale;
				hevc_units_field_based_flag = units_field_based_flag;
			}
		}

		// check whether the same with the current sequence
		if (nal_sequences.size() <= 0)
			bSequenceChange = true;
		else
		{
			auto& back = nal_sequences.back();
			if (back.pic_width_in_luma_samples != pic_width_in_luma_samples ||
				back.pic_height_in_luma_samples != pic_height_in_luma_samples)
				bSequenceChange = true;
			else
			{
				if (aspect_ratio_info_present_flag == 1 && back.aspect_ratio_info_present_flag == 1)
				{
					if (aspect_ratio_idc != back.aspect_ratio_idc)
						bSequenceChange = true;
					else if (aspect_ratio_idc == 0xFF && sar_height != 0 && back.sar_height != 0)
						bSequenceChange = sar_height * back.sar_width != sar_width * back.sar_height ? true : false;
				}

				if (bSequenceChange == false && colour_description_present_flag == 1 && back.colour_description_present_flag == 1)
				{
					if (colour_primaries != back.colour_primaries ||
						transfer_characteristics != back.transfer_characteristics ||
						matrix_coeffs != back.matrix_coeffs)
						bSequenceChange = true;
				}

				if (bSequenceChange == false && vui_timing_info_present_flag == 1 && back.timing_info_present_flag == 1)
				{
					if (vui_num_units_in_tick*back.time_scale != vui_time_scale * back.num_units_in_tick)
						bSequenceChange = true;
				}
			}
		}
	}

	if (bSequenceChange)
	{
		nal_sequences.emplace_back();
		auto& hevc_seq = nal_sequences.back();
		memset(&hevc_seq, 0, sizeof(hevc_seq));
		hevc_seq.start_byte_pos = byte_stream_nal_unit_start_pos;

		hevc_seq.pic_width_in_luma_samples = pic_width_in_luma_samples;
		hevc_seq.pic_height_in_luma_samples = pic_height_in_luma_samples;

		hevc_seq.aspect_ratio_info_present_flag = aspect_ratio_info_present_flag;
		if (aspect_ratio_info_present_flag)
		{
			hevc_seq.aspect_ratio_idc = aspect_ratio_idc;
			if (aspect_ratio_idc == 0xFF)
			{
				hevc_seq.sar_width = sar_width;
				hevc_seq.sar_height = sar_height;
			}
		}

		hevc_seq.colour_description_present_flag = colour_description_present_flag;
		if (colour_description_present_flag)
		{
			hevc_seq.colour_primaries = colour_primaries;
			hevc_seq.transfer_characteristics = transfer_characteristics;
			hevc_seq.matrix_coeffs = matrix_coeffs;
		}

		hevc_seq.timing_info_present_flag = vui_timing_info_present_flag;
		if (vui_timing_info_present_flag)
		{
			hevc_seq.num_units_in_tick = vui_num_units_in_tick;
			hevc_seq.time_scale = vui_time_scale;
			hevc_seq.units_field_based_flag = units_field_based_flag;
		}

		// if there is no frame-rate information, don't set the presentation_start_time for the current HEVC sequence
		if (hevc_time_scale == 0 || hevc_presentation_time_code < 0)
			hevc_seq.presentation_start_time = 0xFFFFFFFF;
		else
			hevc_seq.presentation_start_time = (uint32_t)(hevc_presentation_time_code / 10000LL);
	}

	// Add all related pps_pic_parameter_set_id
	nal_sequences.back().pic_parameter_set_id_sel[slice_pic_parameter_set_id] = true;

	if (hevc_time_scale != 0 && hevc_presentation_time_code >= 0)
		hevc_presentation_time_code += (int64_t)hevc_num_units_in_tick * 10000000LL * ((int64_t)hevc_units_field_based_flag + 1) / hevc_time_scale;
	else
		hevc_presentation_time_code = -1LL;

	if (m_nal_enum)
	{
		uint8_t* pAUBuf = pAUBufStart;
		size_t cbAUBuf = (size_t)(pAUBufEnd - pAUBufStart);

		if (bSequenceChange && (m_nal_enum_options&NAL_ENUM_OPTION_VSEQ) && AMP_FAILED(m_nal_enum->EnumNewVSEQ(m_pCtx)))
		{
			iRet = RET_CODE_ABORT;
			goto done;
		}

		if (bCVSChange && (m_nal_enum_options&NAL_ENUM_OPTION_CVS) && AMP_FAILED(m_nal_enum->EnumNewCVS(m_pCtx, represent_nal_unit_type)))
		{
			iRet = RET_CODE_ABORT;
			goto done;
		}

		//if (pic_start->nal_unit_type != AVC_AUD_NUT)
		//	printf("file position: %" PRIu64 "\n", pic_start->file_offset);
		if ((m_nal_enum_options&NAL_ENUM_OPTION_AU) && AMP_FAILED(m_nal_enum->EnumNALAUBegin(m_pCtx, pAUBuf, cbAUBuf, picture_slice_type)))
		{
			iRet = RET_CODE_ABORT;
			goto done;
		}

		for (auto iter = pic_start; iter != pic_end; iter++)
		{
			uint8_t* pNALUnitBuf = pEBSPBuf + iter->NU_offset;
			pNALUnitBuf += iter->leading_bytes;

			if (ParseNALUnit(pNALUnitBuf, iter->NU_length - iter->leading_bytes) == RET_CODE_ABORT)
			{
				iRet = RET_CODE_ABORT;
				goto done;
			}
		}

		if ((m_nal_enum_options&NAL_ENUM_OPTION_AU) && AMP_FAILED(m_nal_enum->EnumNALAUEnd(m_pCtx, pAUBuf, cbAUBuf)))
		{
			iRet = RET_CODE_ABORT;
			goto done;
		}
	}

done:
	return iRet;
}

int CNALParser::CommitAVCPicture(
	std::vector<NAL_UNIT_ENTRY>::const_iterator pic_start,
	std::vector<NAL_UNIT_ENTRY>::const_iterator pic_end)
{
	int iRet = RET_CODE_SUCCESS;
	if (pic_end <= pic_start)
		return RET_CODE_NEEDMOREINPUT;

	int read_buf_len = 0;
	uint8_t* pEBSPBuf = AM_LRB_GetReadPtr(m_rbNALUnitEBSP, &read_buf_len);

	uint8_t* pAUBufStart = pEBSPBuf + pic_start->NU_offset;
	uint8_t* pAUBufEnd = pEBSPBuf + (pic_end == m_nu_entries.cend() ? read_buf_len : pic_end->NU_offset);

	// Get the first VCL NAL Unit in base layer
	int16_t slice_pic_parameter_set_id = -1;
	uint8_t nal_unit_type = 0XFF, picture_slice_type = 0xFF;
	auto firstBlPicNalUnit = pic_end;
	bool bCVSChange = false, bHasSPS = false;
	int8_t represent_nal_unit_type = -1, first_I_nal_unit_type = -1;
	for (auto iter = pic_start; iter != pic_end; iter++)
	{
		if (iter->nal_unit_type_hevc == BST::H264Video::SPS_NUT)
			bHasSPS = true;

		// At present, only parsing base-layer NAL unit in advance
		if (!IS_AVC_VCL_NAL(iter->nal_unit_type_hevc))
			continue;

		if (represent_nal_unit_type == -1 && iter->nal_unit_type_hevc == BST::H264Video::CS_IDR_PIC)
		{
			represent_nal_unit_type = iter->nal_unit_type_hevc;
			bCVSChange = true;
		}

		if (slice_pic_parameter_set_id == -1)
		{
			firstBlPicNalUnit = iter;
			slice_pic_parameter_set_id = iter->slice_pic_parameter_set_id;
		}
		else if (slice_pic_parameter_set_id != iter->slice_pic_parameter_set_id)
		{
			// Break the spec, record it to error table
			printf("[AVCScanner] slice_pic_parameter_set_id of all VCL NAL Units shall be the same in a coded picture.\n");
			if (m_nal_enum)
				m_nal_enum->EnumNALError(m_pCtx, iter->file_offset - iter->leading_bytes, 3);
			break;
		}

		if (nal_unit_type == 0xFF)
			nal_unit_type = iter->nal_unit_type_hevc;
		else if (nal_unit_type != iter->nal_unit_type_hevc)
		{
			// Break the spec, record it to error table
			printf("[AVCScanner] nal_unit_type of all VCL NAL Units shall be the same in a coded picture.\n");
			if (m_nal_enum)
				m_nal_enum->EnumNALError(m_pCtx, iter->file_offset - iter->leading_bytes, 4);
		}

		switch (iter->slice_type)
		{
		case 0:	// For P
		case 5:
			if (picture_slice_type == 0xFF)
				picture_slice_type = AVC_PIC_SLICE_P;
			else if (picture_slice_type == AVC_PIC_SLICE_I || picture_slice_type == AVC_PIC_SLICE_SI ||
				picture_slice_type == AVC_PIC_SLICE_SP || picture_slice_type == AVC_PIC_SLICE_SI_I)
				picture_slice_type = AVC_PIC_SLICE_SI_I_SP_P;
			else if (picture_slice_type == AVC_PIC_SLICE_B)
				picture_slice_type = AVC_PIC_SLICE_SI_I_SP_P_B;
			break;
		case 1:	// For B
		case 6:
			if (picture_slice_type == 0xFF)
				picture_slice_type = AVC_PIC_SLICE_B;
			else if (picture_slice_type != AVC_PIC_SLICE_B)
				picture_slice_type = AVC_PIC_SLICE_SI_I_SP_P_B;
			break;
		case 2:	// For I
		case 7:
			if (picture_slice_type == 0xFF)
			{
				first_I_nal_unit_type = iter->nal_unit_type_hevc;
				picture_slice_type = AVC_PIC_SLICE_I;
			}
			else if (picture_slice_type == AVC_PIC_SLICE_SI)
				picture_slice_type = AVC_PIC_SLICE_SI_I;
			else if (picture_slice_type == AVC_PIC_SLICE_SP || picture_slice_type == AVC_PIC_SLICE_P)
				picture_slice_type = AVC_PIC_SLICE_SI_I_SP_P;
			else if (picture_slice_type == AVC_PIC_SLICE_B)
				picture_slice_type = AVC_PIC_SLICE_SI_I_SP_P_B;
			break;
		case 3:	// for SP
		case 8:
			if (picture_slice_type == 0xFF)
				picture_slice_type = AVC_PIC_SLICE_SP;
			else if (picture_slice_type == AVC_PIC_SLICE_SI || picture_slice_type == AVC_PIC_SLICE_I || picture_slice_type == AVC_PIC_SLICE_P)
				picture_slice_type = AVC_PIC_SLICE_SI_I_SP_P;
			else if (picture_slice_type == AVC_PIC_SLICE_B)
				picture_slice_type = AVC_PIC_SLICE_SI_I_SP_P_B;
			break;
		case 4:	// For SI
		case 9:
			if (picture_slice_type == 0xFF)
				picture_slice_type = AVC_PIC_SLICE_SI;
			else if (picture_slice_type == AVC_PIC_SLICE_I)
				picture_slice_type = AVC_PIC_SLICE_SI_I;
			else if (picture_slice_type == AVC_PIC_SLICE_P || picture_slice_type == AVC_PIC_SLICE_SP)
				picture_slice_type = AVC_PIC_SLICE_SI_I_SP_P;
			else if (picture_slice_type == AVC_PIC_SLICE_B)
				picture_slice_type = AVC_PIC_SLICE_SI_I_SP_P_B;
			break;
		}
	}

	if (picture_slice_type == AVC_PIC_SLICE_I && bCVSChange == false && bHasSPS)
	{
		represent_nal_unit_type = first_I_nal_unit_type;
		bCVSChange = true;
	}

	auto byte_stream_nal_unit_start_pos = pic_start->file_offset - pic_start->leading_bytes;

	uint16_t pic_width_in_luma_samples = 0;
	uint16_t pic_height_in_luma_samples = 0;
	uint8_t aspect_ratio_info_present_flag = 0, colour_description_present_flag = 0, timing_info_present_flag = 0, nuit_field_based_flag = 1;
	uint8_t aspect_ratio_idc = 0;
	uint16_t sar_width = 0, sar_height = 0;
	uint8_t colour_primaries = 0, transfer_characteristics = 0, matrix_coeffs = 0;
	uint32_t num_units_in_tick = 0, time_scale = 0;
	uint8_t fixed_frame_rate_flag = 0;

	// Check PPS reference is changed or not
	bool bSequenceChange = false;
	if (firstBlPicNalUnit == pic_end)
		printf("[AVCScanner] There is no base-layer VCL NAL Unit in the current code picture unit.\n");
	else 
	{
		H264_NU pps;
		// Check the sequence is changed or not
		if (slice_pic_parameter_set_id < 0 ||
			slice_pic_parameter_set_id > UINT8_MAX ||
			!(pps = m_pNALAVCCtx->GetAVCPPS((uint8_t)slice_pic_parameter_set_id)) || 
			pps->ptr_pic_parameter_set_rbsp == nullptr)
		{
			printf("[AVCScanner] The current picture unit is NOT associated with an available PPS.\n");
			goto done;
		}

		auto sps = m_pNALAVCCtx->GetAVCSPS(pps->ptr_pic_parameter_set_rbsp->seq_parameter_set_id);

		m_pNALAVCCtx->ActivateSPS(pps->ptr_pic_parameter_set_rbsp->seq_parameter_set_id);

		auto& sps_data = sps->ptr_seq_parameter_set_rbsp->seq_parameter_set_data;
		pic_width_in_luma_samples = (sps_data.pic_width_in_mbs_minus1 + 1) << 4;
		pic_height_in_luma_samples = (sps_data.pic_height_in_map_units_minus1 + 1)*(2 - sps_data.frame_mbs_only_flag) << 4;

		if (sps_data.vui_parameters_present_flag && sps_data.vui_parameters)
		{
			aspect_ratio_info_present_flag = sps_data.vui_parameters->aspect_ratio_info_present_flag;
			if (aspect_ratio_info_present_flag)
			{
				aspect_ratio_idc = sps_data.vui_parameters->aspect_ratio_idc;
				if (aspect_ratio_idc == 0xFF)
				{
					sar_width = sps_data.vui_parameters->sar_width;
					sar_height = sps_data.vui_parameters->sar_height;
				}
			}

			colour_description_present_flag = sps_data.vui_parameters->video_signal_type_present_flag && sps_data.vui_parameters->colour_description_present_flag;
			if (colour_description_present_flag)
			{
				colour_primaries = sps_data.vui_parameters->colour_primaries;
				transfer_characteristics = sps_data.vui_parameters->transfer_characteristics;
				matrix_coeffs = sps_data.vui_parameters->matrix_coeffs;
			}

			timing_info_present_flag = sps_data.vui_parameters->timing_info_present_flag;
			if (timing_info_present_flag)
			{
				num_units_in_tick = sps_data.vui_parameters->num_units_in_tick;
				time_scale = sps_data.vui_parameters->time_scale;
				fixed_frame_rate_flag = sps_data.vui_parameters->fixed_frame_rate_flag;
				avc_num_units_in_tick = num_units_in_tick;
				avc_time_scale = time_scale;
				avc_nuit_field_based_flag = nuit_field_based_flag;
			}
		}

		// check whether the same with the current sequence
		if (nal_sequences.size() <= 0)
			bSequenceChange = true;
		else
		{
			auto& back = nal_sequences.back();
			if (back.pic_width_in_luma_samples != pic_width_in_luma_samples ||
				back.pic_height_in_luma_samples != pic_height_in_luma_samples)
				bSequenceChange = true;
			else
			{
				if (aspect_ratio_info_present_flag == 1 && back.aspect_ratio_info_present_flag == 1)
				{
					if (aspect_ratio_idc != back.aspect_ratio_idc)
						bSequenceChange = true;
					else if (aspect_ratio_idc == 0xFF && sar_height != 0 && back.sar_height != 0)
						bSequenceChange = sar_height * back.sar_width != sar_width * back.sar_height ? true : false;
				}

				if (bSequenceChange == false && colour_description_present_flag == 1 && back.colour_description_present_flag == 1)
				{
					if (colour_primaries != back.colour_primaries ||
						transfer_characteristics != back.transfer_characteristics ||
						matrix_coeffs != back.matrix_coeffs)
						bSequenceChange = true;
				}

				if (bSequenceChange == false && timing_info_present_flag == 1 && back.timing_info_present_flag == 1)
				{
					if (num_units_in_tick*back.time_scale != time_scale * back.num_units_in_tick)
						bSequenceChange = true;
				}
			}
		}
	}

	if (bSequenceChange)
	{
		nal_sequences.emplace_back();
		auto& avc_seq = nal_sequences.back();
		memset(&avc_seq, 0, sizeof(avc_seq));
		avc_seq.start_byte_pos = byte_stream_nal_unit_start_pos;

		avc_seq.pic_width_in_luma_samples = pic_width_in_luma_samples;
		avc_seq.pic_height_in_luma_samples = pic_height_in_luma_samples;

		avc_seq.aspect_ratio_info_present_flag = aspect_ratio_info_present_flag;
		if (aspect_ratio_info_present_flag)
		{
			avc_seq.aspect_ratio_idc = aspect_ratio_idc;
			if (aspect_ratio_idc == 0xFF)
			{
				avc_seq.sar_width = sar_width;
				avc_seq.sar_height = sar_height;
			}
		}

		avc_seq.colour_description_present_flag = colour_description_present_flag;
		if (colour_description_present_flag)
		{
			avc_seq.colour_primaries = colour_primaries;
			avc_seq.transfer_characteristics = transfer_characteristics;
			avc_seq.matrix_coeffs = matrix_coeffs;
		}

		avc_seq.timing_info_present_flag = timing_info_present_flag;
		avc_seq.units_field_based_flag = 1;		// For H264, default value is 1
		if (timing_info_present_flag)
		{
			avc_seq.num_units_in_tick = num_units_in_tick;
			avc_seq.time_scale = time_scale;
			avc_seq.fixed_frame_rate_flag = fixed_frame_rate_flag;
			avc_seq.units_field_based_flag = nuit_field_based_flag;
		}

		// if there is no frame-rate information, don't set the presentation_start_time for the current AVC sequence
		if (avc_time_scale == 0 || avc_presentation_time_code < 0)
			avc_seq.presentation_start_time = 0xFFFFFFFF;
		else
			avc_seq.presentation_start_time = (uint32_t)(avc_presentation_time_code / 10000);
	}

	// Add all related pps_pic_parameter_set_id
	if (slice_pic_parameter_set_id >= 0 && slice_pic_parameter_set_id <= 255)
		nal_sequences.back().pic_parameter_set_id_sel[slice_pic_parameter_set_id] = true;

	if (avc_time_scale != 0 && avc_presentation_time_code >= 0)
		avc_presentation_time_code += (int64_t)avc_num_units_in_tick * 10000000LL * ((int64_t)avc_nuit_field_based_flag + 1) / avc_time_scale;
	else
		avc_presentation_time_code = -1LL;

	if (m_nal_enum)
	{
		uint8_t* pAUBuf = pAUBufStart;
		size_t cbAUBuf = (size_t)(pAUBufEnd - pAUBufStart);

		if (bSequenceChange && (m_nal_enum_options&NAL_ENUM_OPTION_VSEQ) && AMP_FAILED(m_nal_enum->EnumNewVSEQ(m_pCtx)))
		{
			iRet = RET_CODE_ABORT;
			goto done;
		}

		if (bCVSChange && (m_nal_enum_options&NAL_ENUM_OPTION_CVS) && AMP_FAILED(m_nal_enum->EnumNewCVS(m_pCtx, represent_nal_unit_type)))
		{
			iRet = RET_CODE_ABORT;
			goto done;
		}

		//if (pic_start->nal_unit_type != AVC_AUD_NUT)
		//	printf("file position: %" PRIu64 "\n", pic_start->file_offset);
		if ((m_nal_enum_options&NAL_ENUM_OPTION_AU) && AMP_FAILED(m_nal_enum->EnumNALAUBegin(m_pCtx, pAUBuf, cbAUBuf, picture_slice_type)))
		{
			iRet = RET_CODE_ABORT;
			goto done;
		}

		for (auto iter = pic_start; iter != pic_end; iter++)
		{
			uint8_t* pNALUnitBuf = pEBSPBuf + iter->NU_offset;
			pNALUnitBuf += iter->leading_bytes;

			if (ParseNALUnit(pNALUnitBuf, iter->NU_length - iter->leading_bytes) == RET_CODE_ABORT)
			{
				iRet = RET_CODE_ABORT;
				goto done;
			}
		}

		if ((m_nal_enum_options&NAL_ENUM_OPTION_AU) && AMP_FAILED(m_nal_enum->EnumNALAUEnd(m_pCtx, pAUBuf, cbAUBuf)))
		{
			iRet = RET_CODE_ABORT;
			goto done;
		}
	}

done:

	return iRet;
}

int CNALParser::ParseNALUnit(uint8_t* pNUBuf, int cbNUBuf)
{
	int nal_unit_type = -1;
	if (m_nal_coding == NAL_CODING_AVC)
		nal_unit_type = pNUBuf[0] & 0x1F;
	else if (m_nal_coding == NAL_CODING_HEVC)
		nal_unit_type = (pNUBuf[0] >> 1) & 0x3F;
	else if (m_nal_coding == NAL_CODING_VVC)
		nal_unit_type = (pNUBuf[1] >> 3) & 0x1F;

	if (m_nal_enum)
	{
		if (m_nal_enum_options&NAL_ENUM_OPTION_NU)
		{
			if (AMP_FAILED(m_nal_enum->EnumNALUnitBegin(m_pCtx, pNUBuf, cbNUBuf)))
				return RET_CODE_ABORT;
		}

		// If the current NAL unit is a SEI, try to drill its sei_message and sei_payload
		if (m_nal_enum_options&(NAL_ENUM_OPTION_SEI_MSG | NAL_ENUM_OPTION_SEI_PAYLOAD))
		{
			if ((m_nal_coding == NAL_CODING_AVC  && (nal_unit_type == BST::H264Video::SEI_NUT)) ||
				(m_nal_coding == NAL_CODING_HEVC && (nal_unit_type == BST::H265Video::PREFIX_SEI_NUT || nal_unit_type == BST::H265Video::SUFFIX_SEI_NUT)) ||
				(m_nal_coding == NAL_CODING_VVC  && (nal_unit_type == BST::H266Video::PREFIX_SEI_NUT || nal_unit_type == BST::H266Video::SUFFIX_SEI_NUT)))
			{
				if (ParseSEINU(pNUBuf, (int)cbNUBuf) == RET_CODE_ABORT)
				{
					printf("Abort parsing SEI NAL Unit.\n");
					return RET_CODE_ABORT;
				}
			}
		}

		if (m_nal_enum_options&NAL_ENUM_OPTION_NU)
		{
			if (AMP_FAILED(m_nal_enum->EnumNALUnitEnd(m_pCtx, pNUBuf, cbNUBuf)))
				return RET_CODE_ABORT;
		}
	}

	return RET_CODE_SUCCESS;
}

/*
	This function can be used by AVC, HEVC and VVC commonly
*/
int CNALParser::ParseSEINU(uint8_t* pNUBuf, int cbNUBuf)
{
	int iRet = RET_CODE_SUCCESS;
	uint8_t nalUnitHeaderBytes = 1;

	int8_t nal_unit_type = -1;
	if (m_nal_coding == NAL_CODING_AVC)
	{
		int8_t forbidden_zero_bit = (pNUBuf[0] >> 7) & 0x01;
		int8_t nal_ref_idc = (pNUBuf[0] >> 5) & 0x3;
		nal_unit_type = pNUBuf[0] & 0x1F;

		int8_t svc_extension_flag = 0;
		int8_t avc_3d_extension_flag = 0;

		if (nal_unit_type == 14 || nal_unit_type == 20 || nal_unit_type == 21)
		{
			if (nal_unit_type != 21)
				svc_extension_flag = (pNUBuf[1] >> 7) & 0x01;
			else
				avc_3d_extension_flag = (pNUBuf[1] >> 7) & 0x01;

			if (svc_extension_flag)
			{
				nalUnitHeaderBytes += 3;
				if (cbNUBuf < 5)
					return RET_CODE_NEEDMOREINPUT;
			}
			else if (avc_3d_extension_flag)
			{
				nalUnitHeaderBytes += 2;
				if (cbNUBuf < 4)
					return RET_CODE_NEEDMOREINPUT;
			}
			else
			{
				nalUnitHeaderBytes += 3;
				if (cbNUBuf < 5)
					return RET_CODE_NEEDMOREINPUT;
			}
		}
	}
	else if (m_nal_coding == NAL_CODING_HEVC)
	{
		nalUnitHeaderBytes = 2;
		nal_unit_type = (pNUBuf[0] >> 1) & 0x3F;
	}
	else if (m_nal_coding == NAL_CODING_VVC)
	{
		nalUnitHeaderBytes = 2;
		nal_unit_type = (pNUBuf[1] >> 3) & 0x1F;
	}
	else
		return RET_CODE_ERROR_NOTIMPL;

	AMBst in_bst = AMBst_CreateFromBuffer(pNUBuf + nalUnitHeaderBytes, cbNUBuf - nalUnitHeaderBytes);
	AMBst_SetRBSPType(in_bst, BST_RBSP_NAL_UNIT);

	bool b_stop_one_bit = false;

	std::vector<uint8_t> sei_message_buf;
	sei_message_buf.reserve((size_t)cbNUBuf - nalUnitHeaderBytes);

	m_pCtx->SetActiveNUType(nal_unit_type);

	try
	{
		do
		{
			b_stop_one_bit = false;

			uint8_t ff_byte;
			uint32_t payloadType = 0, payloadSize = 0;
			uint32_t payload_offset = nalUnitHeaderBytes;
			int left_bits_in_bst = 0;

			sei_message_buf.clear();

			// Avoid to trigger exception to cause bad performance
			if (AMP_SUCCEEDED(AMBst_Tell(in_bst, &left_bits_in_bst)) && left_bits_in_bst < 16)
				break;

			while (AMBst_PeekBits(in_bst, 8) == 0xFF) {
				ff_byte = (uint8_t)AMBst_GetBits(in_bst, 8);
				sei_message_buf.push_back(ff_byte);
				payloadType += 255;
			}

			uint8_t last_payload_type_byte = (uint8_t)AMBst_GetBits(in_bst, 8);
			payloadType += last_payload_type_byte;
			sei_message_buf.push_back(last_payload_type_byte);

			while (AMBst_PeekBits(in_bst, 8) == 0xFF) {
				ff_byte = (uint8_t)AMBst_GetBits(in_bst, 8);
				sei_message_buf.push_back(ff_byte);
				payloadSize += 255;
			}

			uint8_t last_payload_size_byte = (uint8_t)AMBst_GetBits(in_bst, 8);
			payloadSize += last_payload_size_byte;
			sei_message_buf.push_back(last_payload_size_byte);

			size_t sei_payload_offset = sei_message_buf.size();

			sei_message_buf.resize(sei_payload_offset + payloadSize);

			AMBst_GetBytes(in_bst, &sei_message_buf[sei_payload_offset], payloadSize);

			if (m_nal_enum != nullptr)
			{
				if ((m_nal_enum_options&NAL_ENUM_OPTION_SEI_MSG) && AMP_FAILED(m_nal_enum->EnumNALSEIMessageBegin(m_pCtx, sei_message_buf.data(), sei_message_buf.size())))
				{
					iRet = RET_CODE_ABORT;
					goto done;
				}

				if ((m_nal_enum_options&NAL_ENUM_OPTION_SEI_PAYLOAD) && AMP_FAILED(m_nal_enum->EnumNALSEIPayloadBegin(m_pCtx, payloadType, sei_message_buf.data() + sei_payload_offset, payloadSize)))
				{
					iRet = RET_CODE_ABORT;
					goto done;
				}

				if ((m_nal_enum_options&NAL_ENUM_OPTION_SEI_PAYLOAD) && AMP_FAILED(m_nal_enum->EnumNALSEIPayloadEnd(m_pCtx, payloadType, sei_message_buf.data() + sei_payload_offset, payloadSize)))
				{
					iRet = RET_CODE_ABORT;
					goto done;
				}

				if ((m_nal_enum_options&NAL_ENUM_OPTION_SEI_MSG) && AMP_FAILED(m_nal_enum->EnumNALSEIMessageEnd(m_pCtx, sei_message_buf.data(), sei_message_buf.size())))
				{
					iRet = RET_CODE_ABORT;
					goto done;
				}
			}

			//b_stop_one_bit = AMBst_PeekBits(in_bst, 1) ? true : false;

		} while (AMP_SUCCEEDED(iRet) && !b_stop_one_bit);

	}
	catch (...)
	{
		goto done;
	}

done:
	if (in_bst)
		AMBst_Destroy(in_bst);

	m_pCtx->SetActiveNUType(-1);

	return iRet;
}

