/*

MIT License

Copyright (c) 2021 Ravin.Wang(wangf1978@hotmail.com)

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
#include "ESRepacker.h"
#include "Matroska.h"

CESRepacker::CESRepacker(ES_BYTE_STREAM_FORMAT srcESFmt, ES_BYTE_STREAM_FORMAT dstESFmt)
	: m_srcESFmt(srcESFmt), m_dstESFmt(dstESFmt), m_fpSrc(nullptr), m_fpDst(nullptr)
	, m_callback_au_startpoint(nullptr), m_context_au_startpoint(nullptr)
{
	memset(&m_config, 0, sizeof(m_config));
	memset(m_szSrcFilepath, 0, sizeof(m_szSrcFilepath));
}


CESRepacker::~CESRepacker()
{
}

int CESRepacker::Config(ES_REPACK_CONFIG es_repack_config)
{
	if (m_srcESFmt == ES_BYTE_STREAM_ISO_NALAU_SAMPLE)
	{
		if (es_repack_config.codec_id != CODEC_ID_V_MPEG4_AVC &&
			es_repack_config.codec_id != CODEC_ID_V_MPEGH_HEVC)
		{
			printf("The source byte-stream is ISO NAL Access Unit, at present only support MPEG4-AVC and HEVC codec id.\n");
			return RET_CODE_INVALID_PARAMETER;
		}

		if (es_repack_config.codec_id == CODEC_ID_V_MPEG4_AVC)
		{
			if (es_repack_config.pAVCConfigRecord == nullptr)
			{
				printf("AVCDecoderConfigurationRecord information is required to repack AVC NAL access unit sample.\n");
				return RET_CODE_INVALID_PARAMETER;
			}
		}
		else if (es_repack_config.codec_id == CODEC_ID_V_MPEGH_HEVC)
		{
			if (es_repack_config.pHEVCConfigRecord == nullptr)
			{
				printf("HEVCDecoderConfigurationRecord information is required to repack HEVC NAL access unit sample.\n");
				return RET_CODE_INVALID_PARAMETER;
			}
		}
	}

	memcpy(&m_config, &es_repack_config, sizeof(ES_REPACK_CONFIG));
	return RET_CODE_SUCCESS;
}

int CESRepacker::Open(const char* szSrcFile)
{
	errno_t errn = 0;
	if (szSrcFile != nullptr)
	{
		errn = fopen_s(&m_fpSrc, szSrcFile, "rb");
		if (errn != 0 || m_fpSrc == NULL)
		{
			printf("Failed to open the file: %s {errno: %d}.\n", szSrcFile, errn);
			return RET_CODE_ERROR_FILE_NOT_EXIST;
		}
		strcpy_s(m_szSrcFilepath, MAX_PATH, szSrcFile);
	}
	else
	{
		m_fpSrc = nullptr;
		memset(m_szSrcFilepath, 0, sizeof(m_szSrcFilepath));
	}

	if (m_config.es_output_file_path[0] != '\0')
	{
		errn = fopen_s(&m_fpDst, m_config.es_output_file_path, "wb+");
		if (errn != 0 || m_fpDst == NULL)
		{
			printf("Failed to open the file: %s {errno: %d} to write the ES data.\n", m_config.es_output_file_path, errn);
		}
	}

	return RET_CODE_SUCCESS;
}

int CESRepacker::PreProcessMatroskaBlock()
{
	using namespace BST::Matroska;

	int iRet = RET_CODE_SUCCESS;

	uint8_t u8Bytes[4];
	uint64_t u64Val = UINT64_MAX;
	uint8_t num_frames_minus1 = 0;
	uint64_t total_frame_size = 0;

	// Get the SimpleBlock or Block Element ID and its size
	uint32_t ID = (uint32_t)EBMLElement::UnpackUnsignedIntVal(m_fpSrc, 4, false);
	if (ID == UINT32_MAX)
	{
		printf("ESRepacker] Invalid Element ID, current seek point is not a Matroska SimpleBlock or Block.\n");
		return RET_CODE_ERROR;
	}

	if (ID == 0xA0)
		m_cur_seek_point_info.seek_point_type = ES_SEEK_MATROSKA_BLOCK_GROUP;
	else if (ID == 0xA3)
		m_cur_seek_point_info.seek_point_type = ES_SEEK_MATROSKA_SIMPLE_BLOCK;
	else
	{
		printf("[ESRepacker] The Matroska Element ID(%Xh) is not supported.\n", ID);
		return RET_CODE_ERROR_NOTIMPL;
	}

	uint64_t Size = EBMLElement::UnpackUnsignedIntVal(m_fpSrc);
	if (Size == UINT64_MAX)
	{
		printf("[ESRepacker] Invalid Element Size for Element \"%s\"\n", ID==0xA0?"BlockGroup":"SimpleBlock");
		return RET_CODE_ERROR;
	}

	if (ID == 0xA0)
	{
		// Try to find the Block element
		do
		{
			ID = (uint32_t)EBMLElement::UnpackUnsignedIntVal(m_fpSrc, 4, false);
			if (ID == UINT32_MAX)
				break;

			if ((Size = EBMLElement::UnpackUnsignedIntVal(m_fpSrc)) == UINT64_MAX)
				break;

			if (ID != 0xA1)
			{
				if (_fseeki64(m_fpSrc, (long long)Size, SEEK_CUR) != 0)
				{
					ID = UINT32_MAX;
					Size = UINT64_MAX;
					break;
				}
			}
		} while (ID != 0xA1);

		if (ID != 0xA1 || Size == UINT64_MAX)
		{
			printf("[ESRepacker] Can't find Block element in BlockGroup element.\n");
			return RET_CODE_ERROR;
		}
	}

	uint64_t cbLeftSize = Size;
	u64Val = EBMLElement::UnpackUnsignedIntVal(m_fpSrc);
	if (u64Val == UINT64_MAX)
	{
		iRet = RET_CODE_BOX_INCOMPATIBLE;
		goto done;
	}

	if (u64Val >= 0x80)
	{
		printf("[ESRepacker] At present, only support 127 tracks at maximum.\n");
		iRet = RET_CODE_ERROR_NOTIMPL;
		goto done;
	}

	cbLeftSize--;
	m_cur_seek_point_info.track_number = (uint8_t)u64Val;
	
	if (fread(u8Bytes, 1, 3, m_fpSrc) != 3)
	{
		printf("[ESRepacker] Failed to read 3 bytes of Block or SimpleBlock header.\n");
		iRet = RET_CODE_ERROR;
		goto done;
	}

	m_cur_seek_point_info.time_code = (u8Bytes[0] << 8) | u8Bytes[1];
	m_cur_seek_point_info.key_frame = m_cur_seek_point_info.seek_point_type == ES_SEEK_MATROSKA_SIMPLE_BLOCK ?
		((u8Bytes[2]&0x80)?FLAG_SET:FLAG_UNSET) : FLAG_UNKNOWN;
	m_cur_seek_point_info.invisible = (u8Bytes[2] & 0x8) ? FLAG_SET : FLAG_UNSET;
	m_cur_seek_point_info.lacing = (u8Bytes[2] & 0x6) >> 1;
	m_cur_seek_point_info.discardable = m_cur_seek_point_info.seek_point_type == ES_SEEK_MATROSKA_SIMPLE_BLOCK ?
		((u8Bytes[2] & 0x1) ? FLAG_SET : FLAG_UNSET) : FLAG_UNKNOWN;

	cbLeftSize -= 3;

	m_cur_seek_point_info.frame_sizes.clear();
	if (m_cur_seek_point_info.lacing != 0)
	{
		if (fread(&num_frames_minus1, 1, 1, m_fpSrc) == 0)
		{
			printf("[ESRepacker] Failed to read the field \"Number of frames in the lace-1\".\n");
			iRet = RET_CODE_ERROR;
			goto done;
		}

		uint8_t u8Byte;
		for (uint8_t i = 0; i < num_frames_minus1; i++)
		{
			if (m_cur_seek_point_info.lacing == 1)
			{
				// Xiph lacing
				uint32_t lacing_code_size = 0;
				do
				{
					if (fread(&u8Byte, 1, 1, m_fpSrc) == 0)
					{
						printf("[Matroska] Failed to read lace-coded size for Xiph lacing.\n");
						return -1;
					}
					lacing_code_size += u8Byte;
				} while (u8Byte == 0xFF);

				m_cur_seek_point_info.frame_sizes.push_back(lacing_code_size);
			}
			else if (m_cur_seek_point_info.lacing == 3)
			{
				// EBML lacing
				if (fread(&u8Byte, 1, 1, m_fpSrc) == 0)
				{
					printf("[Matroska] Failed to read lace-coded size for EBML lacing.\n");
					return -1;
				}

				uint8_t idx = 0;
				for (; idx < 7; idx++)
					if ((u8Byte&(1 << (7 - idx))) == 1)
						break;

				if (idx >= 7)
				{
					printf("[Matroska] Failed to read lace-coded size for EBML lacing.\n");
					return -1;
				}

				int32_t lacing_coded_size = u8Byte & ~(1 << (7 - idx));
				if (idx > 0)
				{
					uint8_t lacing_size_byte[8];
					if (fread(lacing_size_byte, 1, idx, m_fpSrc) != idx)
					{
						printf("[Matroska] Failed to read lace-coded size for EBML lacing.\n");
						return -1;
					}

					for (uint8_t j = 0; j < idx; j++)
						lacing_coded_size = (lacing_coded_size << 8) | lacing_size_byte[j];
				}

				if (m_cur_seek_point_info.frame_sizes.size() == 0)
					m_cur_seek_point_info.frame_sizes.push_back((uint32_t)lacing_coded_size);
				else
				{
					lacing_coded_size -= (1LL << (((idx + 1) << 3) - (idx + 1) - 1)) - 1;
					m_cur_seek_point_info.frame_sizes.push_back(m_cur_seek_point_info.frame_sizes.back() + lacing_coded_size);
				}
			}
			else if (m_cur_seek_point_info.lacing == 2)
			{
				m_cur_seek_point_info.frame_sizes.push_back((uint32_t)(cbLeftSize / ((uint64_t)num_frames_minus1 + 1)));
			}

			total_frame_size += m_cur_seek_point_info.frame_sizes.back();
		}
	}

	if (cbLeftSize < total_frame_size)
	{
		printf("[Matroska] The data in lacing header is invalid.\n");
		return -1;
	}

	m_cur_seek_point_info.frame_sizes.push_back((uint32_t)(cbLeftSize - total_frame_size));

done:
	return iRet;
}

int CESRepacker::PostProcessMatroSkaBlock()
{
	return RET_CODE_ERROR_NOTIMPL;
}

int CESRepacker::Seek(int64_t src_sample_file_offset, ES_SEEK_POINT_TYPE seek_point_type)
{
	int iRet = RET_CODE_SUCCESS;
	if (seek_point_type == ES_SEEK_SKIP)
	{
		if (_fseeki64(m_fpSrc, src_sample_file_offset, SEEK_CUR) != 0)
			return RET_CODE_ERROR;

		return RET_CODE_SUCCESS;
	}

	if (_fseeki64(m_fpSrc, src_sample_file_offset, SEEK_SET) != 0)
		return RET_CODE_ERROR;

	if (seek_point_type == ES_SEEK_MATROSKA_SIMPLE_BLOCK || seek_point_type == ES_SEEK_MATROSKA_BLOCK_GROUP)
	{
		iRet = PreProcessMatroskaBlock();
	}

	return iRet;
}

int CESRepacker::UpdateSeekPointInfo()
{
	return RET_CODE_ERROR_NOTIMPL;
}

int CESRepacker::GetSeekPointInfo(SEEK_POINT_INFO& seek_point_info)
{
	seek_point_info = m_cur_seek_point_info;
	return RET_CODE_SUCCESS;
}

int CESRepacker::SetNextMPUPtsDts(int number_of_au, const TM_90KHZ* PTSes, const TM_90KHZ* DTSes)
{
	if (number_of_au < 0)
		return RET_CODE_INVALID_PARAMETER;

	m_curr_ptses = m_next_ptses;
	m_curr_dtses = m_next_dtses;

	m_next_ptses.clear();
	m_next_dtses.clear();

	if (number_of_au == 0)
		return RET_CODE_SUCCESS;

	m_next_ptses.reserve(number_of_au);
	m_next_dtses.reserve(number_of_au);

	m_next_ptses.insert(m_next_ptses.begin(), PTSes, PTSes + number_of_au);
	m_next_dtses.insert(m_next_dtses.begin(), DTSes, DTSes + number_of_au);

	return RET_CODE_SUCCESS;
}

int	CESRepacker::Repack(uint32_t sample_size, FLAG_VALUE keyframe)
{
	// fall back to direct byte-2-byte copy
	uint8_t buf[2048];
	uint32_t cbLeftSize = sample_size;
	do
	{
		uint32_t cbRead = (uint32_t)AMP_MIN(cbLeftSize, 2048U);
		if ((cbRead = (uint32_t)fread(buf, 1, cbRead, m_fpSrc)) == 0)
			break;

		if (NULL != m_fpDst)
			fwrite(buf, 1, cbRead, m_fpDst);

		if (cbLeftSize < cbRead)
		{
			printf("Unexpected! The number of read bytes is greater than the requested.\n");
			break;
		}

		cbLeftSize -= cbRead;
	} while (cbLeftSize > 0);

	return RET_CODE_SUCCESS;
}

int CESRepacker::SetAUStartPointCallback(CB_AU_STARTPOINT cb_au_startpoint, void* pCtx)
{
	m_callback_au_startpoint = cb_au_startpoint;
	m_context_au_startpoint = pCtx;
	return RET_CODE_SUCCESS;
}

int CESRepacker::Process(uint8_t* pBuf, int cbSize, const PROCESS_DATA_INFO* data_info)
{
	if (m_dstESFmt == ES_BYTE_STREAM_RAW)
	{
		if (m_fpDst != nullptr)
		{
			if (cbSize <= 0)
				return RET_CODE_INVALID_PARAMETER;

			if (fwrite(pBuf, 1, (size_t)cbSize, m_fpDst) != (size_t)cbSize)
			{
				printf("[ESRepacker] Failed to write %d bytes into the output file.\n", cbSize);
				return RET_CODE_ERROR;
			}

			return RET_CODE_SUCCESS;
		}
	}

	return RET_CODE_ERROR_NOTIMPL;
}

int CESRepacker::Flush()
{
	return RET_CODE_ERROR_NOTIMPL;
}

int CESRepacker::Drain()
{
	return RET_CODE_ERROR_NOTIMPL;
}

int CESRepacker::Close()
{
	if (m_fpSrc != nullptr)
	{
		fclose(m_fpSrc);
		m_fpSrc = nullptr;
	}

	if (m_fpDst != nullptr)
	{
		fclose(m_fpDst);
		m_fpDst = nullptr;
	}

	return RET_CODE_SUCCESS;
}

CNALRepacker::CNALRepacker(ES_BYTE_STREAM_FORMAT srcESFmt, ES_BYTE_STREAM_FORMAT dstESFmt)
	: CESRepacker(srcESFmt, dstESFmt)
	, m_NALAURepacker(nullptr)
	, m_lrb_NAL(nullptr)
	, m_current_au_idx(0)
{
}

CNALRepacker::~CNALRepacker() {
	if (m_NALAURepacker != nullptr)
	{
		delete m_NALAURepacker;
		m_NALAURepacker = nullptr;
	}

	if (m_lrb_NAL != nullptr)
	{
		AM_LRB_Destroy(m_lrb_NAL);
		m_lrb_NAL = nullptr;
	}
}

int CNALRepacker::Open(const char* szSrcFile)
{
	int iRet = CESRepacker::Open(szSrcFile);
	if (iRet < 0)
		return iRet;

	// For AVC/HEVC codec, if the source and target byte stream format are different, routine into the related stream repacker
	if (m_srcESFmt != m_dstESFmt)
	{
		if (m_config.codec_id == CODEC_ID_V_MPEG4_AVC)
		{
			m_NALAURepacker = new BST::ISOBMFF::AVCSampleRepacker(m_fpSrc, m_fpDst, m_config.pMMTESDataOutputAgent, m_config.pAVCConfigRecord);
		}
		else if (m_config.codec_id == CODEC_ID_V_MPEGH_HEVC)
		{
			m_NALAURepacker = new BST::ISOBMFF::HEVCSampleRepacker(m_fpSrc, m_fpDst, m_config.pMMTESDataOutputAgent, m_config.pHEVCConfigRecord);
		}
	}

	if (m_NALAURepacker != nullptr)
	{
		m_NALAURepacker->SetAUStartPointCallback(m_callback_au_startpoint, m_context_au_startpoint);
	}

	return iRet;
}

int	CNALRepacker::Repack(uint32_t sample_size, FLAG_VALUE keyframe)
{
	if (m_NALAURepacker != nullptr)
	{
		if (m_srcESFmt == ES_BYTE_STREAM_ISO_NALAU_SAMPLE && (m_dstESFmt == ES_BYTE_STREAM_AVC_ANNEXB || m_dstESFmt == ES_BYTE_STREAM_HEVC_ANNEXB))
			return m_NALAURepacker->RepackSamplePayloadToAnnexBByteStream(sample_size, keyframe);
	}

	return CESRepacker::Repack(sample_size, keyframe);
}

int CNALRepacker::SetNextMPUPtsDts(int number_of_au, const TM_90KHZ* PTSes, const TM_90KHZ* DTSes)
{
	int iRet = CESRepacker::SetNextMPUPtsDts(number_of_au, PTSes, DTSes);
	if (iRet < 0)
		return iRet;

	if (m_NALAURepacker != nullptr && number_of_au > 0)
	{
		iRet = m_NALAURepacker->SetNextAUPTSDTS(PTSes[0], DTSes[0]);
	}

	m_current_au_idx = 0;

	return iRet;
}

int CNALRepacker::Process(uint8_t* pBuf, int cbSize, const PROCESS_DATA_INFO* data_info)
{
	int iRet = RET_CODE_SUCCESS;

	if (m_lrb_NAL == nullptr)
		m_lrb_NAL = AM_LRB_Create(4 * 1024 * 1024);

	if (m_srcESFmt == ES_BYTE_STREAM_NALUNIT_WITH_LEN)
	{
		uint8_t nDelimiterLengthSize = 4;
		if (m_config.codec_id == CODEC_ID_V_MPEG4_AVC && m_config.pAVCConfigRecord != nullptr)
			nDelimiterLengthSize = m_config.pAVCConfigRecord->lengthSizeMinusOne + 1;
		else if (m_config.codec_id == CODEC_ID_V_MPEGH_HEVC && m_config.pHEVCConfigRecord != nullptr)
			nDelimiterLengthSize = m_config.pHEVCConfigRecord->lengthSizeMinusOne + 1;
		else if (m_config.NALUnit_Length_Size > 0 && (size_t)m_config.NALUnit_Length_Size <= sizeof(uint64_t))
			nDelimiterLengthSize = m_config.NALUnit_Length_Size;

		int nNalBufLen = 0;
		uint8_t* pNalBuf = AM_LRB_GetReadPtr(m_lrb_NAL, &nNalBufLen);
		if (data_info->indicator == FRAG_INDICATOR_COMPLETE || data_info->indicator == FRAG_INDICATOR_FIRST)
		{
			// Flush the previous buffer
			if (pNalBuf != nullptr && nNalBufLen > 0)
			{
				if (m_NALAURepacker != nullptr)
				{
					if (nNalBufLen > nDelimiterLengthSize)
					{
						uint64_t nNalUnitLen = 0;
						for (uint8_t i = 0; i < nDelimiterLengthSize; i++)
							nNalUnitLen = (nNalUnitLen << 8) | pNalBuf[i];

						// it is a normal unit buffer or not.
						if (nNalUnitLen + nDelimiterLengthSize <= (uint64_t)nNalBufLen)
						{
							bool bAUCommitted = false;
							m_NALAURepacker->RepackNALUnitToAnnexBByteStream(pNalBuf + nDelimiterLengthSize, (int)nNalUnitLen, data_info, &bAUCommitted);
							if (bAUCommitted && m_next_ptses.size() > 0)
							{
								m_current_au_idx++;
								if (m_current_au_idx <= (int)m_next_ptses.size())
									m_NALAURepacker->SetNextAUPTSDTS(m_next_ptses[m_current_au_idx], m_next_dtses[m_current_au_idx]);
								else if (m_current_au_idx == (int)m_next_ptses.size())	// no PTS/DTS available any more
									m_NALAURepacker->SetNextAUPTSDTS(-1LL, -1LL);
								else
									printf("Unexpected case!!!! The committed AU seems to exceed the specified number of AUs.\n");
							}
						}
						else
						{
							if (g_verbose_level > 0)
								printf("[ESRepacker] Hit an unexpected case, NAL unit length(%" PRIu64 ") indicated by delimiter-length is less than the actual(%d).\n",
									nNalUnitLen, nNalBufLen - nDelimiterLengthSize);
						}
					}
					else
					{
						if (g_verbose_level > 0)
							printf("[ESRepacker] Hit an unexpected case, buffer length(%d) is greater than delimiter length(%d).\n", nNalBufLen, nDelimiterLengthSize);
					}
				}
				else
				{
					if (m_fpDst != nullptr)
					{
						if (fwrite(pNalBuf, 1, (size_t)nNalBufLen, m_fpDst) != (size_t)nNalBufLen)
							printf("[ESRepacker] Failed to write %d bytes into the output file.\n", nNalBufLen);
					}
				}

				AM_LRB_Reset(m_lrb_NAL);
			}
		}

		if ((data_info->indicator == FRAG_INDICATOR_MIDDLE || data_info->indicator == FRAG_INDICATOR_LAST) && (pNalBuf == nullptr || nNalBufLen <= 0))
		{
			// the NAL unit data is sent in the middle, ignore it.
			return RET_CODE_BUFFER_NOT_COMPATIBLE;
		}

		int nNalWriteBufLen = 0;
		uint8_t* pNalWriteBuf = AM_LRB_GetWritePtr(m_lrb_NAL, &nNalWriteBufLen);
		if (nNalWriteBufLen <= 0 || pNalWriteBuf == nullptr || nNalWriteBufLen < cbSize)
		{
			// Try to reform and enlarge the buffer.
			AM_LRB_Reform(m_lrb_NAL);
			pNalWriteBuf = AM_LRB_GetWritePtr(m_lrb_NAL, &nNalWriteBufLen);
			if (nNalWriteBufLen == 0 || pNalWriteBuf == nullptr || nNalWriteBufLen < cbSize)
			{
				// Enlarge the ring buffer size
				int nCurSize = AM_LRB_GetSize(m_lrb_NAL);
				if (nCurSize == INT32_MAX || nCurSize + cbSize > INT32_MAX)
				{
					printf("The input NAL unit buffer is too huge, can't be supported now.\n");
					return RET_CODE_OUTOFMEMORY;
				}

				if (nCurSize + cbSize >= INT32_MAX / 2)
					nCurSize = INT32_MAX;
				else
					nCurSize += cbSize;

				printf("Try to resize the linear ring buffer size to %d bytes.\n", nCurSize);
				if ((iRet = AM_LRB_Resize(m_lrb_NAL, nCurSize)) < 0)
					return iRet;

				pNalWriteBuf = AM_LRB_GetWritePtr(m_lrb_NAL, &nNalWriteBufLen);
				if (nNalWriteBufLen == 0 || pNalWriteBuf == nullptr)
					return RET_CODE_OUTOFMEMORY;
			}
		}

		memcpy(pNalWriteBuf, pBuf, cbSize);
		AM_LRB_SkipWritePtr(m_lrb_NAL, cbSize);

		if (data_info->indicator == FRAG_INDICATOR_LAST || data_info->indicator == FRAG_INDICATOR_COMPLETE)
		{
			pNalBuf = AM_LRB_GetReadPtr(m_lrb_NAL, &nNalBufLen);
			if (pNalBuf != nullptr && nNalBufLen > 0)
			{
				if (m_NALAURepacker != nullptr)
				{
					if (nNalBufLen > nDelimiterLengthSize)
					{
						uint64_t nNalUnitLen = 0;
						for (uint8_t i = 0; i < nDelimiterLengthSize; i++)
							nNalUnitLen = (nNalUnitLen << 8) | pNalBuf[i];

						// it is a normal unit buffer or not.
						if (nNalUnitLen + nDelimiterLengthSize <= (uint64_t)nNalBufLen)
						{
							bool bAUCommitted = false;
							m_NALAURepacker->RepackNALUnitToAnnexBByteStream(pNalBuf + nDelimiterLengthSize, (int)nNalUnitLen, data_info, &bAUCommitted);
							if (bAUCommitted && m_next_ptses.size() > 0)
							{
								m_current_au_idx++;
								if (m_current_au_idx < (int)m_next_ptses.size())
									m_NALAURepacker->SetNextAUPTSDTS(m_next_ptses[m_current_au_idx], m_next_dtses[m_current_au_idx]);
								else if (m_current_au_idx == (int)m_next_ptses.size())	// no PTS/DTS available any more
									m_NALAURepacker->SetNextAUPTSDTS(-1LL, -1LL);
								else
									printf("Unexpected case!!!! The committed AU seems to exceed the specified number of AUs.\n");
							}
						}
						else
						{
							if (g_verbose_level > 0)
								printf("[ESRepacker] Hit an unexpected case, NAL unit length(%" PRIu64 ") indicated by delimiter-length is less than the actual(%d).\n",
									nNalUnitLen, nNalBufLen - nDelimiterLengthSize);
						}
					}
					else
					{
						if (g_verbose_level > 0)
							printf("[ESRepacker] Hit an unexpected case, buflen(%d) is greater than delimiter length(%d).\n", nNalBufLen, nDelimiterLengthSize);
					}
				}
				else
				{
					if (m_fpDst != nullptr)
					{
						if (fwrite(pNalBuf, 1, (size_t)nNalBufLen, m_fpDst) != (size_t)nNalBufLen)
							printf("[ESRepacker] Failed to write %d bytes into the output file.\n", nNalBufLen);
					}
				}

				AM_LRB_Reset(m_lrb_NAL);
			}
		}

		return RET_CODE_SUCCESS;
	}

	return CESRepacker::Process(pBuf, cbSize, data_info);
}

int CNALRepacker::Flush()
{
	if (m_srcESFmt == ES_BYTE_STREAM_NALUNIT_WITH_LEN)
	{
		uint8_t nDelimiterLengthSize = 4;
		if (m_config.codec_id == CODEC_ID_V_MPEG4_AVC && m_config.pAVCConfigRecord != nullptr)
			nDelimiterLengthSize = m_config.pAVCConfigRecord->lengthSizeMinusOne + 1;
		else if (m_config.codec_id == CODEC_ID_V_MPEGH_HEVC && m_config.pHEVCConfigRecord != nullptr)
			nDelimiterLengthSize = m_config.pHEVCConfigRecord->lengthSizeMinusOne + 1;
		else if (m_config.NALUnit_Length_Size > 0 && (size_t)m_config.NALUnit_Length_Size <= sizeof(uint64_t))
			nDelimiterLengthSize = m_config.NALUnit_Length_Size;

		int nNalBufLen = 0;
		uint8_t* pNalBuf = AM_LRB_GetReadPtr(m_lrb_NAL, &nNalBufLen);

		// Flush the previous buffer
		if (pNalBuf != nullptr && nNalBufLen > 0)
		{
			if (m_NALAURepacker != nullptr)
			{
				if (nNalBufLen > nDelimiterLengthSize)
				{
					uint64_t nNalUnitLen = 0;
					for (uint8_t i = 0; i < nDelimiterLengthSize; i++)
						nNalUnitLen = (nNalUnitLen << 8) | pNalBuf[i];

					// it is a normal unit buffer or not.
					if (nNalUnitLen + nDelimiterLengthSize <= (uint64_t)nNalBufLen)
						m_NALAURepacker->RepackNALUnitToAnnexBByteStream(pNalBuf + nDelimiterLengthSize, (int)nNalUnitLen);
				}
			}
			else
			{
				if (m_fpDst != nullptr)
				{
					if (fwrite(pNalBuf, 1, (size_t)nNalBufLen, m_fpDst) != (size_t)nNalBufLen)
						printf("[ESRepacker] Failed to flush %d bytes into the output file.\n", nNalBufLen);
				}
			}

			AM_LRB_Reset(m_lrb_NAL);
		}
	}

	return m_NALAURepacker ? m_NALAURepacker->Flush() : RET_CODE_SUCCESS;
}

int CNALRepacker::Drain()
{
	if (m_srcESFmt == ES_BYTE_STREAM_NALUNIT_WITH_LEN)
	{
		uint8_t nDelimiterLengthSize = 4;
		if (m_config.codec_id == CODEC_ID_V_MPEG4_AVC && m_config.pAVCConfigRecord != nullptr)
			nDelimiterLengthSize = m_config.pAVCConfigRecord->lengthSizeMinusOne + 1;
		else if (m_config.codec_id == CODEC_ID_V_MPEGH_HEVC && m_config.pHEVCConfigRecord != nullptr)
			nDelimiterLengthSize = m_config.pHEVCConfigRecord->lengthSizeMinusOne + 1;
		else if (m_config.NALUnit_Length_Size > 0 && (size_t)m_config.NALUnit_Length_Size <= sizeof(uint64_t))
			nDelimiterLengthSize = m_config.NALUnit_Length_Size;

		int nNalBufLen = 0;
		uint8_t* pNalBuf = AM_LRB_GetReadPtr(m_lrb_NAL, &nNalBufLen);

		// Flush the previous buffer
		if (pNalBuf != nullptr && nNalBufLen > 0)
		{
			if (m_NALAURepacker != nullptr)
			{
				if (nNalBufLen > nDelimiterLengthSize)
				{
					uint64_t nNalUnitLen = 0;
					for (uint8_t i = 0; i < nDelimiterLengthSize; i++)
						nNalUnitLen = (nNalUnitLen << 8) | pNalBuf[i];

					// it is a normal unit buffer or not.
					if (nNalUnitLen + nDelimiterLengthSize <= (uint64_t)nNalBufLen)
						m_NALAURepacker->RepackNALUnitToAnnexBByteStream(pNalBuf + nDelimiterLengthSize, (int)nNalUnitLen);
				}
			}
			else
			{
				if (m_fpDst != nullptr)
				{
					if (fwrite(pNalBuf, 1, (size_t)nNalBufLen, m_fpDst) != (size_t)nNalBufLen)
						printf("[ESRepacker] Failed to flush %d bytes into the output file.\n", nNalBufLen);
				}
			}

			AM_LRB_Reset(m_lrb_NAL);
		}
	}

	return m_NALAURepacker ? m_NALAURepacker->Drain() : RET_CODE_SUCCESS;
}

int CNALRepacker::Close()
{
	if (m_NALAURepacker != nullptr)
	{
		delete m_NALAURepacker;
		m_NALAURepacker = nullptr;
	}

	return CESRepacker::Close();
}

CMPEG4AACLOASRepacker::CMPEG4AACLOASRepacker(ES_BYTE_STREAM_FORMAT srcESFmt, ES_BYTE_STREAM_FORMAT dstESFmt)
	: CESRepacker(srcESFmt, dstESFmt)
	, m_lrb_input(nullptr)
	, m_current_au_idx(0)
{
}

CMPEG4AACLOASRepacker::~CMPEG4AACLOASRepacker()
{
	if (m_lrb_input != nullptr)
	{
		AM_LRB_Destroy(m_lrb_input);
		m_lrb_input = nullptr;
	}
}

int CMPEG4AACLOASRepacker::SetNextMPUPtsDts(int number_of_au, const TM_90KHZ* PTSes, const TM_90KHZ* DTSes)
{
	int iRet = CESRepacker::SetNextMPUPtsDts(number_of_au, PTSes, DTSes);
	if (iRet < 0)
		return iRet;

	m_current_au_idx = 0;

	return iRet;
}

int CMPEG4AACLOASRepacker::Process(uint8_t* pBuf, int cbSize, const PROCESS_DATA_INFO* data_info)
{
	int iRet = RET_CODE_SUCCESS;

	if (m_lrb_input == nullptr)
		m_lrb_input = AM_LRB_Create(1024 * 1024);

	if (m_srcESFmt == ES_BYTE_LATM_AudioMuxElement &&
		m_dstESFmt == ES_BYTE_LOAS_AudioSyncStream)
	{
		// As for transmission of audio signal of LATM/LOAS stream format, AudioMuxElement () that 
		// synchronization byte and length information are removed from AudioSyncStream() is
		// transmitted as MFU. In the receiver, it is output to audio decoder as AudioSyncStream() that 
		// synchronization byte and length information are added to AudioMuxElement() which is 
		// involved in the received MFU.
		int nLATMBufLen = 0;
		uint8_t* pLATMBuf = AM_LRB_GetReadPtr(m_lrb_input, &nLATMBufLen);
		if (data_info->indicator == FRAG_INDICATOR_COMPLETE || data_info->indicator == FRAG_INDICATOR_FIRST)
		{
			// Flush the previous buffer
			if (pLATMBuf != nullptr && nLATMBufLen > 0)
			{
				if ((iRet = WriteLATMToLOASDstFile(pLATMBuf, nLATMBufLen)) < 0)
				{
					AM_LRB_Reset(m_lrb_input);
					return iRet;
				}

				AM_LRB_Reset(m_lrb_input);

				if (m_callback_au_startpoint != nullptr)
				{
					ACCESS_UNIT_INFO au_info;
					memset(&au_info, 0, sizeof(au_info));

					TM_90KHZ dts = -1LL, pts = -1LL;
					if (m_current_au_idx > 0 && (size_t)m_current_au_idx < m_next_ptses.size())
						pts = m_next_ptses[m_current_au_idx];
					if (m_current_au_idx > 0 && (size_t)m_current_au_idx < m_next_dtses.size())
						dts = m_next_dtses[m_current_au_idx];
					m_callback_au_startpoint(pts, dts, data_info, &au_info, m_context_au_startpoint);
				}
				m_current_au_idx++;
			}
		}

		if ((data_info->indicator == FRAG_INDICATOR_MIDDLE || data_info->indicator == FRAG_INDICATOR_LAST) && (pLATMBuf == nullptr || nLATMBufLen <= 0))
		{
			// the NAL unit data is sent in the middle, ignore it.
			return RET_CODE_BUFFER_NOT_COMPATIBLE;
		}

		if (data_info->indicator == FRAG_INDICATOR_COMPLETE)
		{
			if (m_callback_au_startpoint != nullptr)
			{
				ACCESS_UNIT_INFO au_info;
				memset(&au_info, 0, sizeof(au_info));

				TM_90KHZ dts = -1LL, pts = -1LL;
				if (m_current_au_idx > 0 && (size_t)m_current_au_idx < m_next_ptses.size())
					pts = m_next_ptses[m_current_au_idx];
				if (m_current_au_idx > 0 && (size_t)m_current_au_idx < m_next_dtses.size())
					dts = m_next_dtses[m_current_au_idx];
				m_callback_au_startpoint(pts, dts, data_info, &au_info, m_context_au_startpoint);
			}
			m_current_au_idx++;

			return WriteLATMToLOASDstFile(pBuf, cbSize);
		}

		pLATMBuf = AM_LRB_GetReadPtr(m_lrb_input, &nLATMBufLen);
		if (nLATMBufLen <= 0 || pLATMBuf == nullptr || nLATMBufLen < cbSize)
		{
			// Try to reform and enlarge the buffer.
			AM_LRB_Reform(m_lrb_input);
			pLATMBuf = AM_LRB_GetWritePtr(m_lrb_input, &nLATMBufLen);
			if (nLATMBufLen == 0 || pLATMBuf == nullptr || nLATMBufLen < cbSize)
			{
				// Enlarge the ring buffer size
				int nCurSize = AM_LRB_GetSize(m_lrb_input);
				if (nCurSize == INT32_MAX || nCurSize + cbSize > INT32_MAX)
				{
					printf("The input audio buffer is too huge, can't be supported now.\n");
					return RET_CODE_OUTOFMEMORY;
				}

				if (nCurSize + cbSize >= INT32_MAX / 2)
					nCurSize = INT32_MAX;
				else
					nCurSize += cbSize;

				printf("Try to resize the linear ring buffer size to %d bytes.\n", nCurSize);
				if ((iRet = AM_LRB_Resize(m_lrb_input, nCurSize)) < 0)
					return iRet;

				pLATMBuf = AM_LRB_GetWritePtr(m_lrb_input, &nLATMBufLen);
				if (nLATMBufLen == 0 || pLATMBuf == nullptr)
					return RET_CODE_OUTOFMEMORY;
			}
		}

		memcpy(pLATMBuf, pBuf, cbSize);
		AM_LRB_SkipWritePtr(m_lrb_input, cbSize);

		if (data_info->indicator == FRAG_INDICATOR_LAST)
		{
			pLATMBuf = AM_LRB_GetReadPtr(m_lrb_input, &nLATMBufLen);
			if (pLATMBuf != nullptr && nLATMBufLen > 0)
			{
				if ((iRet = WriteLATMToLOASDstFile(pLATMBuf, nLATMBufLen)) < 0)
				{
					AM_LRB_Reset(m_lrb_input);
					return iRet;
				}

				AM_LRB_Reset(m_lrb_input);
			}
		}

		return RET_CODE_SUCCESS;
	}

	return CESRepacker::Process(pBuf, cbSize, data_info);
}

int CMPEG4AACLOASRepacker::WriteLATMToLOASDstFile(uint8_t* pBuf, int cbSize)
{
	uint8_t sync_header[4] = { 0 };

	if (pBuf == nullptr || cbSize <= 0)
		return RET_CODE_IGNORE_REQUEST;

	sync_header[0] = 0x56;
	sync_header[1] = ((cbSize >> 8) & 0x1F) | 0xE0;
	sync_header[2] = cbSize & 0xFF;

	if (m_fpDst != NULL)
	{
		if (fwrite(sync_header, 1, 3, m_fpDst) != 3)
		{
			printf("Failed to write data into the destination file.\n");
			return RET_CODE_ERROR;
		}

		if (fwrite(pBuf, 1, (size_t)cbSize, m_fpDst) != (size_t)cbSize)
		{
			printf("Failed to write data into the destination file.\n");
			return RET_CODE_ERROR;
		}

		return RET_CODE_SUCCESS;
	}

	return RET_CODE_IGNORE_REQUEST;
}

