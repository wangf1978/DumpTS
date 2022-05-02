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
#include "mpeg2_video.h"
#include "mpeg2_video_parser.h"

CMPEG2VideoParser::CMPEG2VideoParser(RET_CODE* pRetCode)
	: m_pCtx(nullptr)
{
	RET_CODE ret_code = RET_CODE_SUCCESS;
	if (AMP_FAILED(CreateMPVContext(&m_pCtx)))
	{
		printf("Failed to create the MPEG2 Video context.\n");
		ret_code = RET_CODE_ERROR;
	}

	m_rbRawBuf = AM_LRB_Create(read_unit_size * 128);
	m_rbMPVUnit = AM_LRB_Create(64 * 1024);

	AMP_SAFEASSIGN(pRetCode, ret_code);
}

CMPEG2VideoParser::~CMPEG2VideoParser()
{
	AMP_SAFERELEASE(m_mpv_enum);
	AMP_SAFERELEASE(m_pCtx);
	AM_LRB_Destroy(m_rbMPVUnit);
	AM_LRB_Destroy(m_rbRawBuf);
}

RET_CODE CMPEG2VideoParser::SetEnumerator(IUnknown* pEnumerator, uint32_t options)
{
	IMPVEnumerator* pMPVEumerator = nullptr;
	if (pEnumerator != nullptr)
	{
		if (FAILED(pEnumerator->QueryInterface(IID_IMPVEnumerator, (void**)&pMPVEumerator)))
		{
			return RET_CODE_ERROR;
		}
	}

	if (m_mpv_enum)
		m_mpv_enum->Release();

	m_mpv_enum = pMPVEumerator;

	m_mpv_enum_options = options;

	return RET_CODE_SUCCESS;
}

RET_CODE CMPEG2VideoParser::ProcessInput(uint8_t* pInput, size_t cbInput)
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
			printf("[MPVParser] Failed to get the write buffer(%p), or the write buffer size(%d) is not enough.\n", pBuf, read_size);
			return RET_CODE_BUFFER_TOO_SMALL;
		}
	}

	memcpy(pBuf, pInput, cbInput);

	AM_LRB_SkipWritePtr(m_rbRawBuf, (unsigned int)cbInput);

	return RET_CODE_SUCCESS;
}

RET_CODE CMPEG2VideoParser::ProcessOutput(bool bDrain)
{
	RET_CODE iRet = RET_CODE_SUCCESS;
	int iMPVUnitCommitRet = RET_CODE_SUCCESS;
	uint8_t* pStartBuf = NULL, *pBuf = NULL, *pCurParseStartBuf = NULL, *pSubmitEnd = NULL;

	int cbSize = 0;
	if ((pStartBuf = AM_LRB_GetReadPtr(m_rbRawBuf, &cbSize)) == NULL || cbSize <= 0)
	{
		return RET_CODE_NEEDMOREINPUT;
	}

	pCurParseStartBuf = pBuf = pSubmitEnd = pStartBuf;
	while (cbSize >= minimum_mpv_parse_buffer_size)
	{
		// Find the start_code_prefix_one_3bytes
		while (cbSize >= minimum_mpv_parse_buffer_size && !(pBuf[0] == 0 && pBuf[1] == 0 && pBuf[2] == 1))
		{
			cbSize--;
			pBuf++;
		}

		// If the current MPV start code is valid, need push the parsed data to ring buffer of EBSP
		if (m_cur_mpv_start_code != -1 && pBuf > pCurParseStartBuf)
		{
			//printf("pushing ");
			//for (uint8_t* z = pCurParseStartBuf; z < AMP_MIN(pBuf, pCurParseStartBuf + 4); z++)
			//	printf("%02X ", *z);
			//printf(": %d bytes\n", (int)(pBuf - pCurParseStartBuf));

			if (AMP_FAILED(iRet = PushMPVUnitBuf(pCurParseStartBuf, pBuf)))
				goto done;

			//if (pCurParseStartBuf[0] == 0xCB && pCurParseStartBuf[1] == 0x0B && pCurParseStartBuf[2] == 0x3C && pCurParseStartBuf[3] == 0xBD)
			//if (pCurParseStartBuf[0] == 0xCE && pCurParseStartBuf[1] == 0xCF && pCurParseStartBuf[2] == 0x1E && pCurParseStartBuf[3] == 0xAB)
/*			if (pCurParseStartBuf[0] == 0x37 && pCurParseStartBuf[1] == 0x95 && pCurParseStartBuf[2] == 0x12 && pCurParseStartBuf[3] == 0x83)
				printf("Hitting here again.\n")*/;
		}

		// pBuf is the next submit position
		pSubmitEnd = pBuf;

		// Failed to find the "start_code_prefix_one_3bytes"
		if (cbSize < minimum_mpv_parse_buffer_size)
			break;	// Quit the current loop and read more data and do the next round of scanning

		int16_t mpv_start_code = -1;
		uint64_t file_offset = m_cur_scan_pos + (size_t)(pBuf - pStartBuf);

		pCurParseStartBuf = pBuf/* + minimum_mpv_parse_buffer_size*/;

		// Complete a MPV Unit, and try to commit it
		// During committing it, do the advance analysis, it may parse the MPEG2 video unit
		if (m_cur_mpv_start_code != -1 && (iMPVUnitCommitRet = CommitMPVUnit()) == RET_CODE_ABORT)
		{
			iRet = iMPVUnitCommitRet;
			goto done;
		}

		mpv_start_code = pBuf[3];

		if (m_cur_mpv_start_code != -1 && (m_mpv_enum_options&(MPV_ENUM_OPTION_AU|MPV_ENUM_OPTION_GOP)))
		{
			if (mpv_start_code == SEQUENCE_HEADER_CODE || mpv_start_code == GROUP_START_CODE || mpv_start_code == PICTURE_START_CODE)
			{
				if (m_pCtx->GetCurrentLevel() == 2)
				{
					// in the previous ring buffer, there is already a AU
					if ((iMPVUnitCommitRet = CommitAU()) == RET_CODE_ABORT)
					{
						iRet = iMPVUnitCommitRet;
						goto done;
					}
				}
			}
		}

		// Skip "prefix start code" and "start code", it should be 00 00 01 xx 
		pBuf += minimum_mpv_parse_buffer_size; cbSize -= minimum_mpv_parse_buffer_size;

		/*
			picture_start_code			00
			slice_start_code			01 through AF
			reserved					B0
			reserved					B1
			user_data_start_code		B2
			sequence_header_code		B3
			sequence_error_code			B4
			extension_start_code		B5
			reserved					B6
			sequence_end_code			B7s
			group_start_code			B8
			system start codes(Note)	B9 through FF
		*/
		// Only care about the start codes involved into MPEG2 video
		if (mpv_start_code < 0 || mpv_start_code > 0xB8 || (!(m_mpv_enum_options&MPV_ENUM_OPTION_AU) && !m_pCtx->IsStartCodeFiltered((uint8_t)mpv_start_code)))
		{
			// although the start code is filtered, it should update start code chain list
			if (mpv_start_code >= 0 && mpv_start_code <= 0xB8)
				m_pCtx->UpdateStartCode((uint8_t)mpv_start_code);

			// don't care about the MPEG unit which is not in the scope of this MPEG2-video scheme, or is 
			m_cur_mpv_start_code = -1;
			continue;
		}

		m_cur_mpv_start_code = mpv_start_code;

		m_cur_submit_pos = m_cur_scan_pos + (size_t)(pCurParseStartBuf - pStartBuf);
	}

	// Skip the parsed raw data buffer
	AM_LRB_SkipReadPtr(m_rbRawBuf, (unsigned int)(pSubmitEnd - pStartBuf));

	// Update the scan position
	m_cur_scan_pos += pSubmitEnd - pStartBuf;

	if (bDrain)
	{
		if (m_cur_mpv_start_code != -1 && (pStartBuf = AM_LRB_GetReadPtr(m_rbRawBuf, &cbSize)) != NULL && cbSize > 0)
		{
			assert(cbSize <= minimum_mpv_parse_buffer_size);
			uint8_t* pEndBuf = pStartBuf + cbSize;
			pCurParseStartBuf = pBuf = pStartBuf;
			if (pStartBuf < pEndBuf && AMP_FAILED(iRet = PushMPVUnitBuf(pStartBuf, pEndBuf)))
				goto done;

			if ((iMPVUnitCommitRet = CommitMPVUnit()) == RET_CODE_ABORT)
			{
				iRet = iMPVUnitCommitRet;
				goto done;
			}

			m_cur_scan_pos += pEndBuf - pStartBuf;

			if (m_mpv_enum_options&(MPV_ENUM_OPTION_AU | MPV_ENUM_OPTION_GOP))
			{
				if (m_pCtx->GetCurrentLevel() == 2)
				{
					// in the previous ring buffer, there is already a AU
					if ((iMPVUnitCommitRet = CommitAU()) == RET_CODE_ABORT)
					{
						iRet = iMPVUnitCommitRet;
						goto drain_done;
					}
				}
			}
		}

	drain_done:
		AM_LRB_Reset(m_rbRawBuf);
		AM_LRB_Reset(m_rbMPVUnit);
		m_last_commit_buf_len = 0;
		m_picture_coding_type = -1;
	}

done:
	return iRet;
}

RET_CODE CMPEG2VideoParser::ParseAUBuf(uint8_t* pAUBuf, size_t cbAUBuf)
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

RET_CODE CMPEG2VideoParser::GetContext(IUnknown** ppCtx)
{
	if (ppCtx == nullptr)
		return RET_CODE_INVALID_PARAMETER;

	m_pCtx->AddRef();
	*ppCtx = (IUnknown*)m_pCtx;
	return RET_CODE_SUCCESS;
}

RET_CODE CMPEG2VideoParser::Reset()
{
	AM_LRB_Reset(m_rbMPVUnit);
	AM_LRB_Reset(m_rbRawBuf);

	m_cur_scan_pos = 0;
	m_cur_submit_pos = 0;
	m_cur_mpv_start_code = -1;
	m_last_commit_buf_len = 0;
	m_picture_coding_type = -1;
	m_gop_start = 0;
	m_wait_for_new_vseq = true;

	m_pCtx->Reset();

	return RET_CODE_SUCCESS;
}

int CMPEG2VideoParser::PushMPVUnitBuf(uint8_t* pStart, uint8_t* pEnd)
{
	int cbLeftOfEBSP = 0;
	uint8_t* pEBSPWriteBuf = AM_LRB_GetWritePtr(m_rbMPVUnit, &cbLeftOfEBSP);
	if (pEBSPWriteBuf == NULL || cbLeftOfEBSP < (int)(pEnd - pStart))
	{
		// Try to reform the ring buffer of EBSP
		AM_LRB_Reform(m_rbMPVUnit);
		pEBSPWriteBuf = AM_LRB_GetWritePtr(m_rbMPVUnit, &cbLeftOfEBSP);
		if (pEBSPWriteBuf == NULL || cbLeftOfEBSP < (int)(pEnd - pStart))
		{
			// Try to enlarge the ring buffer of EBSP
			int rb_size = AM_LRB_GetSize(m_rbMPVUnit);
			int64_t new_rb_size = (int64_t)rb_size << 1;
			if (new_rb_size < (int64_t)rb_size + (int64_t)(pEnd - pStart) - (pEBSPWriteBuf != NULL ? cbLeftOfEBSP : 0))
				new_rb_size = (int64_t)rb_size + (int64_t)(pEnd - pStart) - (pEBSPWriteBuf != NULL ? cbLeftOfEBSP : 0);

			if (new_rb_size >= INT32_MAX || AM_LRB_Resize(m_rbMPVUnit, (int)new_rb_size) < 0)
			{
				printf("Failed to resize the ring buffer of MPV Unit EBSP to %" PRId64 ".\n", (int64_t)rb_size * 2);
				return RET_CODE_ERROR;
			}

			pEBSPWriteBuf = AM_LRB_GetWritePtr(m_rbMPVUnit, &cbLeftOfEBSP);
		}
	}

	// Write the parsed buffer to ring buffer
	memcpy(pEBSPWriteBuf, pStart, (size_t)(pEnd - pStart));

	AM_LRB_SkipWritePtr(m_rbMPVUnit, (unsigned int)(pEnd - pStart));

	return RET_CODE_SUCCESS;
}

int CMPEG2VideoParser::CommitMPVUnit(bool bDrain)
{
	int iRet = RET_CODE_SUCCESS;
	AMBst bst = nullptr;
	int read_buf_len = 0;
	int16_t mpv_start_code = -1;
	uint8_t* pBuf = AM_LRB_GetReadPtr(m_rbMPVUnit, &read_buf_len);

	if (pBuf == NULL || read_buf_len < minimum_mpv_parse_buffer_size)
		return RET_CODE_NEEDMOREINPUT;

	if (m_last_commit_buf_len >= read_buf_len)
	{
		//printf("m_last_commit_buf_len: %d, read_buf_len: %d\n", m_last_commit_buf_len, read_buf_len);
		// It is already committed last time, don't commit this time
		return RET_CODE_NOTHING_TODO;
	}

	pBuf = pBuf + m_last_commit_buf_len;
	int32_t  last_mpv_unit_off = m_last_commit_buf_len;
	uint32_t last_mpv_unit_len = (uint32_t)(read_buf_len - m_last_commit_buf_len);

	//printf("MPV start code: %02X %02X %02X %02X, m_last_commit_buf_len: %d, read_buf_len: %d\n",
	//	pBuf[0], pBuf[1], pBuf[2], pBuf[3], m_last_commit_buf_len, read_buf_len);

	// A new MPV unit is commit, and update the new ring buffer size, and process the current MPV unit
	m_last_commit_buf_len = read_buf_len;

	assert(pBuf[0] == 0 && pBuf[1] == 0 && pBuf[2] == 1 && (pBuf[3] >= 0 && pBuf[3] <= 0xB8));

	mpv_start_code = pBuf[3];
	try
	{
		if (mpv_start_code == SEQUENCE_HEADER_CODE)
		{
			if ((bst = AMBst_CreateFromBuffer(pBuf, read_buf_len)) == nullptr)
			{
				iRet = RET_CODE_OUTOFMEMORY;
				goto done;
			}
			BST::MPEG2Video::CSequenceHeader* pSeqHdr =
				new(std::nothrow) BST::MPEG2Video::CSequenceHeader();
			if (pSeqHdr == nullptr)
			{
				iRet = RET_CODE_OUTOFMEMORY;
				goto done;
			}
			if (AMP_FAILED(iRet = pSeqHdr->Map(bst)))
			{
				delete pSeqHdr;
				printf("[MPVParser] Failed to load the sequence header {error code: %d}\n", iRet);
				goto done;
			}

			std::shared_ptr<BST::MPEG2Video::CSequenceHeader> spSeqHdr(pSeqHdr);
			m_pCtx->UpdateSeqHdr(spSeqHdr);

			if (m_wait_for_new_vseq && (m_mpv_enum_options&MPV_ENUM_OPTION_VSEQ))
			{
				m_wait_for_new_vseq = false;
				if (m_mpv_enum != nullptr && AMP_FAILED(iRet = m_mpv_enum->EnumVSEQStart(m_pCtx)))
				{
					iRet = RET_CODE_ABORT;
					goto done;
				}
			}
		}
		else if (mpv_start_code == EXTENSION_START_CODE)
		{
			if (m_pCtx->GetCurrentLevel() == 0 && read_buf_len > 4 && ((pBuf[4]>>4)&0xF) == SEQUENCE_EXTENSION_ID)
			{
				// It should be a sequence extension
				if ((bst = AMBst_CreateFromBuffer(pBuf, read_buf_len)) == nullptr)
				{
					iRet = RET_CODE_OUTOFMEMORY;
					goto done;
				}

				BST::MPEG2Video::CSequenceExtension* pSeqExt =
					new (std::nothrow) BST::MPEG2Video::CSequenceExtension();
				if (pSeqExt == nullptr)
				{
					iRet = RET_CODE_OUTOFMEMORY;
					goto done;
				}
				if (AMP_FAILED(iRet = pSeqExt->Map(bst)))
				{
					delete pSeqExt;
					printf("[MPVParser] Failed to load the sequence extension {error code: %d}\n", iRet);
					goto done;
				}

				std::shared_ptr<BST::MPEG2Video::CSequenceExtension> spSeqExt(pSeqExt);
				m_pCtx->UpdateSeqExt(spSeqExt);
			}
			else if (m_pCtx->GetCurrentLevel() == 0 && read_buf_len > 4 && ((pBuf[4] >> 4) & 0xF) == SEQUENCE_SCALABLE_EXTENSION_ID)
			{
				if ((bst = AMBst_CreateFromBuffer(pBuf, read_buf_len)) == nullptr)
				{
					iRet = RET_CODE_OUTOFMEMORY;
					goto done;
				}

				BST::MPEG2Video::CSequenceScalableExtension* pSeqScalableExt =
					new (std::nothrow) BST::MPEG2Video::CSequenceScalableExtension();
				if (pSeqScalableExt == nullptr)
				{
					iRet = RET_CODE_OUTOFMEMORY;
					goto done;
				}
				if (AMP_FAILED(iRet = pSeqScalableExt->Map(bst)))
				{
					delete pSeqScalableExt;
					printf("[MPVParser] Failed to load the sequence scalable extension {error code: %d}\n", iRet);
					goto done;
				}

				std::shared_ptr<BST::MPEG2Video::CSequenceScalableExtension> spSeqScalableExt(pSeqScalableExt);
				m_pCtx->UpdateSeqScalableExt(spSeqScalableExt);
			}
		}
		else if (mpv_start_code == PICTURE_START_CODE)
		{
			if (last_mpv_unit_len >= 6)
				m_picture_coding_type = (pBuf[5] >> 3) & 0x7;
		}
		else if (mpv_start_code == GROUP_START_CODE)
		{
			if (last_mpv_unit_len >= 8)
			{
				m_gop_start = 1;
				m_closed_gop  = (pBuf[7] >> 6) & 0x1;
				m_broken_link = (pBuf[7] >> 5) & 0x1;
			}
		}

		if (mpv_start_code >= 0 && mpv_start_code <= 0xB8)
			m_pCtx->UpdateStartCode((uint8_t)mpv_start_code);

		if(m_mpv_enum_options&MPV_ENUM_OPTION_SE)
		{
			if (m_pCtx->IsStartCodeFiltered((uint8_t)mpv_start_code))
			{
				if (!(m_mpv_enum_options&(MPV_ENUM_OPTION_AU | MPV_ENUM_OPTION_GOP)))
				{
					if (m_mpv_enum && m_mpv_enum->EnumObject(m_pCtx, pBuf, (size_t)read_buf_len) == RET_CODE_ABORT)
					{
						iRet = RET_CODE_ABORT;
						goto done;
					}
				}
				else
				{
					m_se_ranges_in_au.push_back({ last_mpv_unit_off , last_mpv_unit_len });
				}
			}
		}

		if (!(m_mpv_enum_options&(MPV_ENUM_OPTION_AU|MPV_ENUM_OPTION_GOP)))
		{
			AM_LRB_Reset(m_rbMPVUnit);
			m_last_commit_buf_len = 0;
			m_se_ranges_in_au.clear();
		}

		if (mpv_start_code == SEQUENCE_END_CODE)
		{
			if (m_wait_for_new_vseq == false && (m_mpv_enum_options&MPV_ENUM_OPTION_VSEQ))
			{
				if (m_mpv_enum != nullptr && AMP_FAILED(iRet = m_mpv_enum->EnumVSEQEnd(m_pCtx)))
				{
					iRet = RET_CODE_ABORT;
					goto done;
				}
				m_wait_for_new_vseq = true;
			}
		}
	}
	catch (AMException e)
	{
		iRet = e.RetCode();
	}

done:
	if (bst != nullptr)
		AMBst_Destroy(bst);

	return iRet;
}

RET_CODE CMPEG2VideoParser::CommitAU()
{
	int iRet = RET_CODE_SUCCESS;
	AMBst bst = nullptr;
	int read_buf_len = 0;
	int16_t mpv_start_code = -1;
	uint8_t* pBuf = AM_LRB_GetReadPtr(m_rbMPVUnit, &read_buf_len);

	if (pBuf == nullptr || read_buf_len <= 0)
		return RET_CODE_NOTHING_TODO;

	if (m_mpv_enum != nullptr)
	{
		if (m_gop_start && (m_mpv_enum_options&MPV_ENUM_OPTION_GOP) &&
			AMP_FAILED(iRet = m_mpv_enum->EnumNewGOP(m_pCtx, m_closed_gop, m_broken_link)))
		{
			iRet = RET_CODE_ABORT;
			goto done;
		}

		if((m_mpv_enum_options&MPV_ENUM_OPTION_AU) && AMP_FAILED(iRet = m_mpv_enum->EnumAUStart(m_pCtx, pBuf, (size_t)read_buf_len, m_picture_coding_type)))
		{
			iRet = RET_CODE_ABORT;
			goto done;
		}

		if (m_mpv_enum_options&MPV_ENUM_OPTION_SE)
		{
			for (auto& range : m_se_ranges_in_au)
			{
				if (AMP_FAILED(m_mpv_enum->EnumObject(m_pCtx, pBuf + std::get<0>(range), (size_t)std::get<1>(range))))
				{
					iRet = RET_CODE_ABORT;
					goto done;
				}
			}
		}

		if((m_mpv_enum_options&MPV_ENUM_OPTION_AU) && AMP_FAILED(m_mpv_enum->EnumAUEnd(m_pCtx, pBuf, (size_t)read_buf_len, m_picture_coding_type)))
		{
			iRet = RET_CODE_ABORT;
			goto done;
		}
	}

done:
	AM_LRB_Reset(m_rbMPVUnit);
	m_last_commit_buf_len = 0;
	m_picture_coding_type = -1;
	m_se_ranges_in_au.clear();
	m_gop_start = 0;

	return iRet;
}

