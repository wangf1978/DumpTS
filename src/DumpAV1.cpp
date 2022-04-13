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
#include <stdint.h>
#include "DataUtil.h"
#include "mediaobjprint.h"
#include <unordered_map>
#include "av1.h"

extern std::map<std::string, std::string, CaseInsensitiveComparator> g_params;

using OBU_UNIT_LAYOUT = std::tuple<
	uint8_t,					/* OBU length byte length */
	uint32_t>;					/* OBU length */

using FRAME_UNIT_LAYOUT = std::tuple<
	uint8_t,					/* frame_unit_size byte length */
	uint32_t,					/* frame_unit_size */
	std::vector<OBU_UNIT_LAYOUT>>;

using TEMPORAL_UNIT_LAYOUT = std::tuple<
	uint8_t,					/* temporal_unit_size byte length*/
	uint32_t,					/* temporal_unit_size */
	std::vector<FRAME_UNIT_LAYOUT>>;

// try to find a sequence header obu
RET_CODE CheckAV1LowOverheadBitstream(uint8_t* pBuf, size_t cbBuf, FLAG_VALUE& check_ret)
{
	AMBst bst = nullptr;
	uint8_t* p = pBuf;
	size_t cbLeft = cbBuf;

	if (pBuf == nullptr || cbBuf == 0)
		return RET_CODE_NEEDMOREINPUT;

	if (cbBuf > INT32_MAX)
		return RET_CODE_ERROR_NOTIMPL;

	uint8_t obu_forbidden_bit, obu_type, obu_extension_flag, obu_has_size_field, obu_reserved_1bit;
	uint32_t obu_size = UINT32_MAX;
	uint8_t leb128_byte_count = 0;
	uint8_t leb128_need_more_data = false;
	RET_CODE iRet = RET_CODE_NEEDMOREINPUT;

	do
	{
		obu_forbidden_bit = ((*p) >> 7) & 0x1;
		if (obu_forbidden_bit != 0)
		{
			p++;
			continue;
		}

		obu_type = ((*p) >> 3) & 0xF;
		obu_extension_flag = ((*p) >> 2) & 0x1;
		obu_has_size_field = ((*p) >> 1) & 0x1;
		obu_reserved_1bit = (*p) & 0x1;

		// obu_reserved_1bit must be set to 0. The value is ignored by a decoder.
		if (obu_reserved_1bit != 0)
		{
			p++;
			cbLeft--;
			continue;
		}

		if (obu_extension_flag)
		{
			p++;
			cbLeft--;

			if (cbLeft == 0)
				return RET_CODE_NEEDMOREINPUT;

			uint8_t temporal_id = ((*p) >> 5) & 0x7;
			uint8_t spatial_id = ((*p) >> 3) & 0x3;
			uint8_t extension_header_reserved_3bits = (*p) & 0x7;

			if (extension_header_reserved_3bits != 0)
			{
				p++;
				cbLeft--;
				continue;
			}
		}

		if (obu_has_size_field)
		{
			leb128_need_more_data = false;
			obu_size = BST::AV1::leb128(p, (int)cbLeft, &leb128_byte_count, &leb128_need_more_data);
			if (leb128_need_more_data)
				return RET_CODE_NEEDMOREINPUT;

			if (obu_size == UINT32_MAX)
			{
				p++;
				cbLeft--;
				continue;
			}
		}
		else
		{
			obu_size = UINT32_MAX;
		}

		if (obu_type == OBU_TEMPORAL_DELIMITER)
		{
			if (obu_size == UINT32_MAX)
			{
				if (cbLeft == 0)
					return RET_CODE_NEEDMOREINPUT;

				p++;
				cbLeft--;
			}
			else
			{
				if (cbLeft < obu_size)
					return RET_CODE_NEEDMOREINPUT;

				p += obu_size;
				cbLeft -= obu_size;
			}

			continue;
		}
		else if (obu_type == OBU_SEQUENCE_HEADER)
		{
			if (obu_size != UINT32_MAX && cbLeft < obu_size)
				return RET_CODE_NEEDMOREINPUT;
			else if (obu_size == UINT32_MAX && cbLeft == 0)
				return RET_CODE_NEEDMOREINPUT;

			bst = AMBst_CreateFromBuffer(p, (int)(obu_size == UINT32_MAX ? cbLeft : obu_size));

			BST::AV1::OPEN_BITSTREAM_UNIT::SEQUENCE_HEADER_OBU SeqHdr(nullptr);
			try
			{
				if (AMP_SUCCEEDED(SeqHdr.Map(bst)))
				{
					check_ret = FLAG_SET;
					iRet = RET_CODE_SUCCESS;
					break;
				}
			}
			catch (AMException& ame)
			{
				if (ame.RetCode() == RET_CODE_NEEDMOREINPUT || ame.RetCode() == RET_CODE_NO_MORE_DATA)
				{
					iRet = RET_CODE_NEEDMOREINPUT;
					break;
				}
			}
			catch (...)
			{
				iRet = RET_CODE_ERROR;
				break;
			}
			AMBst_Destroy(bst);
			bst = nullptr;
		}
		else
		{
			if (obu_size == UINT32_MAX)
			{
				// Don't know how to process the left OBU, but can't also determine it is not an AV1 low-overhead bitstream
				check_ret = FLAG_UNKNOWN;
				iRet = RET_CODE_SUCCESS;
				break;
			}

			if (obu_size != UINT32_MAX && cbLeft < obu_size)
			{
				iRet = RET_CODE_NEEDMOREINPUT;
				break;
			}

			p += obu_size;
			cbLeft -= obu_size;
			continue;
		}
	} while (cbLeft > 0);

	if (bst != nullptr)
		AMBst_Destroy(bst);

	return iRet;
}

/*!	@brief Process a temporal unit from the specified position.
	@param szAV1FileName the AV1 file path name
	@param start_pos the specified file position where to start the process
	@param temporal_unit_layout the analyzing result
	@return the process result
	@retval -1 it is not a valid temporal unit
	@retval 0 it is a valid temporal unit
	@remark the 'temporal_unit_size' at the length delimited bitstream is expected to be the first parsed unit.
*/
int AV1_ProcessAnnexBTemporalUnit(const char* szAV1FileName, int64_t start_pos, TEMPORAL_UNIT_LAYOUT& temporal_unit_layout)
{
	// for the first round try it as Annex B: length delimited bitstream format

	int iRet = RET_CODE_ERROR;
	const int read_unit_size = 2048;
	int64_t cur_scan_pos = start_pos;
	int scan_phase = 0;	// 0: whole file; 1: temporal_unit; 2: frame_unit; 3: obu_unit

	const char* szScanPhase[4] = {
		_T("file"), _T("temporal unit"), _T("frame unit"), _T("obu unit")
	};

	struct
	{
		int64_t		file_offset;
		int64_t		unit_size;
		int64_t		left_size;
		int32_t		parsed_unit_count;
	}scan_phase_info[4];

	memset(&scan_phase_info, 0, sizeof(scan_phase_info));

	// Create a bigger ring buffer to avoid frequent linear ring buffer reform
	AMLinearRingBuffer rbRawBuf = AM_LRB_Create(read_unit_size * 128);

	int cbSize = 0;
	uint8_t* pStartBuf = NULL, *pBuf = NULL, *pCurParseStartBuf = NULL;

	// At first, try to open the stream file
	FILE* rfp = NULL;
	FOPEN(rfp, szAV1FileName, "rb");

	if (rfp == NULL)
	{
		iRet = RET_CODE_ERROR_FILE_NOT_EXIST;
		goto done;
	}

	// Get the file size
	if (_fseeki64(rfp, 0, SEEK_END) != 0)
		goto done;

	scan_phase_info[0].file_offset = start_pos;

	if ((scan_phase_info[0].left_size = scan_phase_info[0].unit_size = _ftelli64(rfp) - scan_phase_info[0].file_offset) <= 0)
	{
		iRet = RET_CODE_NO_MORE_DATA;
		goto done;
	}

	if (_fseeki64(rfp, start_pos, SEEK_SET) != 0)
		goto done;

	scan_phase = 1;

	do
	{
		int read_size = read_unit_size;
		pBuf = AM_LRB_GetWritePtr(rbRawBuf, &read_size);
		if (pBuf == NULL || read_size < read_unit_size)
		{
			// Try to reform linear ring buffer
			AM_LRB_Reform(rbRawBuf);
			if ((pBuf = AM_LRB_GetWritePtr(rbRawBuf, &read_size)) == NULL || read_size < read_unit_size)
			{
				iRet = RET_CODE_ERROR;
				break;
			}
		}

		if ((read_size = (int)fread(pBuf, 1, read_unit_size, rfp)) <= 0)
		{
			iRet = RET_CODE_IO_READ_ERROR;
			break;
		}

		AM_LRB_SkipWritePtr(rbRawBuf, (unsigned int)read_size);

		if ((pStartBuf = AM_LRB_GetReadPtr(rbRawBuf, &cbSize)) == NULL || cbSize <= 0)
		{
			iRet = RET_CODE_ERROR;
			break;
		}

		pCurParseStartBuf = pBuf = pStartBuf;

		BOOL bNeedMoreData = FALSE;

		while (!bNeedMoreData && cbSize > 0)
		{
			if (scan_phase < 1 || scan_phase > 3)
			{
				printf("[AV1] Unexpected scan phase.\n");
				goto done;
			}

			uint8_t Leb128Bytes = 0;
			size_t i = 0, cbAvailLeb128ExtractBytes = 0;

			if (scan_phase_info[scan_phase].unit_size == 0)	// No available unit size
			{
				int64_t unit_size = 0;
				// extract temporal_unit_size
				cbAvailLeb128ExtractBytes = AMP_MIN(8, cbSize);
				for (; i < cbAvailLeb128ExtractBytes; i++)
				{
					unit_size |= ((int64_t)(pBuf[i] & 0x7f) << (i * 7));
					Leb128Bytes++;
					if (!(pBuf[i] & 0x80))
						break;
				}

				if (unit_size >= UINT32_MAX)
				{
					printf("[AV1][%s] the value(%" PRId64 ") returned from the leb128 parsing process should be less than or equal to (1<<32)-1 {file offset: %" PRId64 "}\n",
						szScanPhase[scan_phase], unit_size, cur_scan_pos);
					goto done;
				}

				if (i >= cbAvailLeb128ExtractBytes)
				{
					// Failed to extract leb128 from the enough data
					if (cbSize >= 8)
					{
						printf("[AV1][%s] Failed to extract leb128 size {file offset: %" PRId64 "}.\n", szScanPhase[scan_phase], cur_scan_pos);
						goto done;
					}
					else
					{
						bNeedMoreData = TRUE;
						continue;	// No data is available, need read more data
					}
				}

				// Successfully extract unit size
				scan_phase_info[scan_phase].file_offset = cur_scan_pos;
				scan_phase_info[scan_phase].unit_size = unit_size;
				scan_phase_info[scan_phase].left_size = unit_size;

				// Skip over the buffer
				cbSize -= Leb128Bytes;
				pBuf += Leb128Bytes;
				cur_scan_pos += Leb128Bytes;

				scan_phase_info[scan_phase - 1].left_size -= Leb128Bytes;
				if (scan_phase_info[scan_phase - 1].left_size < 0)
				{
					if (g_verbose_level > 0)
						printf("[AV1][%s] The sub unit costs too many bytes(%" PRId64 ") which exceed the upper unit size(%" PRId64 ").\n",
							szScanPhase[scan_phase], scan_phase_info[scan_phase - 1].unit_size - scan_phase_info[scan_phase - 1].left_size, scan_phase_info[scan_phase - 1].unit_size);
					goto done;
				}

				// Skip the parsed raw data buffer
				AM_LRB_SkipReadPtr(rbRawBuf, Leb128Bytes);

				switch (scan_phase)
				{
				case 1:
					temporal_unit_layout = std::make_tuple(Leb128Bytes, (uint32_t)unit_size, std::vector<FRAME_UNIT_LAYOUT>());
					scan_phase++;
					if (g_verbose_level > 0)
						printf("[AV1] hit one temporal_unit (Leb128Bytes: %d, unit_size: %" PRId64 ").\n", Leb128Bytes, unit_size);
					break;
				case 2:
				{
					auto& frame_units = std::get<2>(temporal_unit_layout);
					frame_units.emplace_back(Leb128Bytes, (uint32_t)unit_size, std::vector<OBU_UNIT_LAYOUT>());
					scan_phase++;
					if (g_verbose_level > 0)
						printf("[AV1] \thit one frame_unit (Leb128Bytes: %d, unit_size: %" PRId64 ").\n", Leb128Bytes, unit_size);
				}
				break;
				case 3:
				{
					auto& last_frame_unit = std::get<2>(temporal_unit_layout).back();
					auto& obu_units = std::get<2>(last_frame_unit);
					obu_units.emplace_back(Leb128Bytes, (uint32_t)unit_size);
					if (g_verbose_level > 0)
						printf("[AV1] \t\thit one obu_unit (Leb128Bytes: %d, unit_size: %" PRId64 ").\n", Leb128Bytes, unit_size);
				}
				break;
				default:
					assert(0);
				}
			}
			else
			{
				if (scan_phase == 3 &&
					scan_phase_info[scan_phase].left_size == scan_phase_info[scan_phase].unit_size)
				{
					int cbAvailSize = cbSize;
					uint8_t* pOBUBuf = pBuf;

					// At the beginning of OBU payload, collect enough data to be parsed
					uint8_t obu_header_byte = *pOBUBuf;

					uint8_t obu_forbidden_bit = (obu_header_byte >> 7) & 0x01;
					uint8_t obu_type = (obu_header_byte >> 3) & 0xF;
					uint8_t obu_extension_flag = (obu_header_byte >> 2) & 0x1;
					uint8_t obu_has_size_flag = (obu_header_byte >> 1) & 0x1;
					uint8_t obu_reserved_1bit = obu_header_byte & 0x1;

					if (obu_forbidden_bit != 0 || obu_reserved_1bit != 0)
					{
						if (g_verbose_level > 0)
							printf("[AV1] Hit the invalid obu_header where obu_forbidden_bit: %d, obu_reserved_1bit: %d.\n", obu_forbidden_bit, obu_reserved_1bit);
						goto done;
					}

					cbAvailSize--;
					pOBUBuf++;

					if (g_verbose_level > 0)
						printf("[AV1] \t\thit obu_type: %s.\n", obu_type_names[obu_type]);

					if (obu_extension_flag)
					{
						if (cbAvailSize < 1)
						{
							bNeedMoreData = TRUE;
							continue;	// No data is available, need read more data
						}

						uint8_t temporal_id = (*pOBUBuf >> 5) & 0x7;
						uint8_t spatial_id = (*pOBUBuf >> 3) & 0x03;
						uint8_t extension_header_reserved_3bits = (*pOBUBuf & 0x07);

						if (extension_header_reserved_3bits == 0)
						{
							printf("[AV1] Hit the invalid obu_extension_header where extension_header_reserved_3bits: %d.\n", extension_header_reserved_3bits);
							goto done;
						}

						cbAvailSize--;
						pOBUBuf++;
					}

					if (obu_has_size_flag)
					{
						int64_t obu_size = 0;
						cbAvailLeb128ExtractBytes = AMP_MIN(8, cbAvailSize);
						for (; i < cbAvailLeb128ExtractBytes; i++)
						{
							obu_size |= ((int64_t)(pOBUBuf[i] & 0x7f) << (i * 7));
							Leb128Bytes++;
							if (!(pOBUBuf[i] & 0x80))
								break;
						}

						if (obu_size >= UINT32_MAX)
						{
							printf("[AV1][open_bitstream_unit] the value(%" PRId64 ") returned from the leb128 parsing process should be less than or equal to (1<<32)-1 {file offset: %" PRId64 "}\n",
								obu_size, cur_scan_pos + (pOBUBuf - pBuf));
							goto done;
						}

						if (i >= cbAvailLeb128ExtractBytes)
						{
							// Failed to extract leb128 from the enough data
							if (cbAvailSize >= 8)
							{
								printf("[AV1][open_bitstream_unit] Failed to extract leb128 size {file offset: %" PRId64 "}.\n", cur_scan_pos);
								goto done;
							}
							else
							{
								bNeedMoreData = TRUE;
								continue;	// No data is available, need read more data
							}
						}

						cbAvailSize -= Leb128Bytes;
						pOBUBuf += Leb128Bytes;

						// Find the obu_size
						if (scan_phase_info[scan_phase].unit_size != obu_size + pOBUBuf - pBuf)
						{
							printf("[AV1][open_bitstream_unit] the obu_size in open_bitstream_unit is not consistent with the obu_length specified in the Length delimited bitstream.\n");
							goto done;
						}
					}
				}

				if (scan_phase_info[scan_phase].left_size < 0 || cbSize < 0)
				{
					assert(0);
					goto done;
				}

				uint32_t nSkip = (uint32_t)AMP_MIN(scan_phase_info[scan_phase].left_size, cbSize);
				scan_phase_info[scan_phase].left_size -= nSkip;

				cbSize -= nSkip;
				pBuf += nSkip;
				cur_scan_pos += nSkip;

				// current unit parsing is finished
				if (scan_phase_info[scan_phase].left_size == 0)
				{
					scan_phase_info[scan_phase].parsed_unit_count++;
					scan_phase_info[scan_phase - 1].left_size -= scan_phase_info[scan_phase].unit_size;

					if (scan_phase_info[scan_phase - 1].left_size < 0)
					{
						if (g_verbose_level > 0)
							printf("[AV1][%s] The sub unit costs too many bytes(%" PRId64 ") which exceed the upper unit size(%" PRId64 ").\n",
								szScanPhase[scan_phase], scan_phase_info[scan_phase - 1].unit_size - scan_phase_info[scan_phase - 1].left_size, scan_phase_info[scan_phase - 1].unit_size);
						goto done;
					}

					scan_phase_info[scan_phase].unit_size = 0;	// Finish processing the current unit, and prepare processing the next unit

					if (scan_phase_info[scan_phase - 1].left_size == 0)
					{
						// Parsing the next unit
						scan_phase--;
						if (scan_phase == 1)
						{
							iRet = 0;
							goto done;
						}
					}
				}

				// Skip the parsed raw data buffer
				AM_LRB_SkipReadPtr(rbRawBuf, nSkip);
			}
		}	// while (!bNeedMoreData)
	} while (!feof(rfp));

	iRet = 0;

done:
	AM_LRB_Destroy(rbRawBuf);

	if (rfp != nullptr)
		fclose(rfp);

	return iRet;
}

int AV1_ProcessIVFLowOverheadBitstream(const char* szAV1FileName, bool& bAnnexB)
{
	int cbSize = 0;
	int iRet = RET_CODE_ERROR;

	uint8_t ivf_hdr[IVF_HDR_SIZE] = { 0 };
	uint8_t pic_hdr[IVF_PIC_HDR_SIZE] = { 0 };

	uint8_t Leb128Bytes = 0;
	uint32_t cb_read_pos = 0;
	uint32_t sz_ivf_frame = 0, cb_ivf_frame_left = 0;
	uint32_t sz_tu = 0, cb_tu_left = 0;
	uint32_t sz_fu = 0, cb_fu_left = 0;
	uint32_t sz_obu = 0, cb_obu_left = 0;

	size_t i = 0;
	int64_t unit_size = 0;
	uint8_t buf[16] = { 0 };
	uint32_t read_size = 0;
	uint16_t ivf_hdr_len;
	uint32_t codec_fourcc;

	// At first, try to open the stream file
	FILE* rfp = NULL;
	FOPEN(rfp, szAV1FileName, "rb");

	if (rfp == NULL)
	{
		iRet = RET_CODE_ERROR_FILE_NOT_EXIST;
		goto done;
	}

	if (fread(ivf_hdr, 1, IVF_HDR_SIZE, rfp) != IVF_HDR_SIZE ||
		ivf_hdr[0] != 'D' || ivf_hdr[1] != 'K' || ivf_hdr[2] != 'I' || ivf_hdr[3] != 'F' 
		|| ((ivf_hdr[5] << 8) | ivf_hdr[4]) != 0	// Version should be 0
		|| ((ivf_hdr[7] << 8) | ivf_hdr[6]) < 0x20	// header length should be NOT less than 32
		)
	{
		iRet = RET_CODE_BOX_INCOMPATIBLE;
		goto done;
	}

	ivf_hdr_len = (ivf_hdr[7] << 8) | ivf_hdr[6];
	codec_fourcc = ((uint32_t)ivf_hdr[8] << 24) | ((uint32_t)ivf_hdr[9] << 16) | ((uint32_t)ivf_hdr[10] << 8) | ivf_hdr[11];

	if (codec_fourcc != 'AV01')
	{
		iRet = RET_CODE_ERROR_NOTIMPL;
		goto done;
	}

	if (_fseeki64(rfp, ivf_hdr_len, SEEK_SET) != 0)
	{
		iRet = RET_CODE_IO_READ_ERROR;
		goto done;
	}

	if (fread(pic_hdr, 1, IVF_PIC_HDR_SIZE, rfp) != IVF_PIC_HDR_SIZE)
	{
		iRet = RET_CODE_BOX_INCOMPATIBLE;
		goto done;
	}

	sz_ivf_frame = ((uint32_t)pic_hdr[3] << 24) | ((uint32_t)pic_hdr[2] << 16) | ((uint32_t)pic_hdr[1] << 8) | pic_hdr[0];
	cb_ivf_frame_left = sz_ivf_frame;
	cb_read_pos = ivf_hdr_len + IVF_PIC_HDR_SIZE;

	if (sz_ivf_frame == 0)
	{
		iRet = RET_CODE_BOX_INCOMPATIBLE;
		goto done;
	}

	// Try to check whether it is an Annex-B AV1 stream or not
	read_size = AMP_MIN(sz_ivf_frame, 8);
	if ((read_size = (uint32_t)fread(buf, 1, read_size, rfp)) <= 0)
	{
		iRet = RET_CODE_IO_READ_ERROR;
		goto done;
	}

	for (i = 0; i < read_size; i++){
		unit_size |= ((int64_t)(buf[i] & 0x7f) << (i * 7));
		Leb128Bytes++;
		if (!(buf[i] & 0x80)) break;
	}

	if (i>= read_size || unit_size >= UINT32_MAX || (int64_t)sz_ivf_frame != unit_size + Leb128Bytes)
		goto check_low_overhead;

	cb_read_pos += Leb128Bytes;
	sz_tu = cb_tu_left = (uint32_t)unit_size;

	while (cb_tu_left > 0)
	{
		if (_fseeki64(rfp, cb_read_pos, SEEK_SET) != 0)
			goto check_low_overhead;

		read_size = AMP_MIN(cb_tu_left, 8);
		if ((read_size = (uint32_t)fread(buf, 1, read_size, rfp)) <= 0)
			goto check_low_overhead;

		for (i = 0; i < read_size; i++){
			unit_size |= ((int64_t)(buf[i] & 0x7f) << (i * 7));
			Leb128Bytes++;
			if (!(buf[i] & 0x80)) break;
		}

		if (i >= read_size || unit_size >= UINT32_MAX || (int64_t)cb_tu_left < unit_size + Leb128Bytes)
			goto check_low_overhead;

		cb_read_pos += Leb128Bytes;
		cb_tu_left -= (uint32_t)(unit_size + Leb128Bytes);
		sz_fu = cb_fu_left = (uint32_t)unit_size;

		while (cb_fu_left > 0)
		{
			if (_fseeki64(rfp, cb_read_pos, SEEK_SET) != 0)
				goto check_low_overhead;

			read_size = AMP_MIN(cb_fu_left, 8);
			if ((read_size = (uint32_t)fread(buf, 1, read_size, rfp)) <= 0)
				goto check_low_overhead;

			for (i = 0; i < read_size; i++){
				unit_size |= ((int64_t)(buf[i] & 0x7f) << (i * 7));
				Leb128Bytes++;
				if (!(buf[i] & 0x80)) break;
			}

			if (i >= read_size || unit_size >= UINT32_MAX || (int64_t)cb_fu_left < unit_size + Leb128Bytes)
				goto check_low_overhead;

			cb_read_pos += Leb128Bytes;
			cb_fu_left -= Leb128Bytes;

			sz_obu = cb_obu_left = (uint32_t)unit_size;

			if (sz_obu <= 0)
				goto check_low_overhead;

			// Try to do the rough check for the OBU
			if (_fseeki64(rfp, cb_read_pos, SEEK_SET) != 0)
				goto check_low_overhead;

			if ((read_size = (uint32_t)fread(buf, 1, 1, rfp)) == 1)
			{
				uint8_t obu_forbidden_bit = (buf[0] >> 7) & 0x01;
				uint8_t obu_type = (buf[0] >> 3) & 0xF;
				uint8_t obu_extension_flag = (buf[0] >> 2) & 0x1;
				uint8_t obu_has_size_flag = (buf[0] >> 1) & 0x1;
				uint8_t obu_reserved_1bit = buf[0] & 0x1;

				if (obu_forbidden_bit != 0 || obu_reserved_1bit != 0)
				{
					if (g_verbose_level > 0)
						printf("[AV1] Hit the invalid obu_header where obu_forbidden_bit: %d, obu_reserved_1bit: %d.\n", obu_forbidden_bit, obu_reserved_1bit);
					goto check_low_overhead;
				}

				if (obu_extension_flag)
				{
					if ((read_size = (uint32_t)fread(buf, 1, 1, rfp)) == 1)
					{
						uint8_t temporal_id = (buf[0] >> 5) & 0x7;
						uint8_t spatial_id = (buf[0] >> 3) & 0x03;
						uint8_t extension_header_reserved_3bits = (buf[0] & 0x07);

						if (extension_header_reserved_3bits == 0)
						{
							if (g_verbose_level > 0)
								printf("[AV1] Hit the invalid obu_extension_header where extension_header_reserved_3bits: %d.\n", extension_header_reserved_3bits);
							goto check_low_overhead;
						}
					}
					else
						goto check_low_overhead;
				}
			}
			else
				goto check_low_overhead;

			cb_fu_left -= sz_obu;
			cb_read_pos += sz_obu;
		}

		cb_tu_left -= sz_fu;
	}

	bAnnexB = true;
	iRet = RET_CODE_SUCCESS;
	goto done;

check_low_overhead:

	cb_read_pos = ivf_hdr_len + IVF_PIC_HDR_SIZE;

	while (cb_ivf_frame_left > 0)
	{
		if (_fseeki64(rfp, cb_read_pos, SEEK_SET) != 0)
		{
			iRet = RET_CODE_IO_READ_ERROR;
			break;
		}

		if ((read_size = (uint32_t)fread(buf, 1, 1, rfp)) <= 0)
		{
			iRet = RET_CODE_BUFFER_NOT_COMPATIBLE;
			break;
		}

		uint8_t obu_forbidden_bit = (buf[0] >> 7) & 0x01;
		uint8_t obu_type = (buf[0] >> 3) & 0xF;
		uint8_t obu_extension_flag = (buf[0] >> 2) & 0x1;
		uint8_t obu_has_size_flag = (buf[0] >> 1) & 0x1;
		uint8_t obu_reserved_1bit = buf[0] & 0x1;

		if (obu_forbidden_bit != 0 || obu_reserved_1bit != 0)
		{
			if (g_verbose_level > 0)
				printf("[AV1] Hit the invalid obu_header where obu_forbidden_bit: %d, obu_reserved_1bit: %d.\n", obu_forbidden_bit, obu_reserved_1bit);
			iRet = RET_CODE_BUFFER_NOT_COMPATIBLE;
			break;
		}

		cb_ivf_frame_left--;
		cb_read_pos++;

		if (obu_extension_flag)
		{
			if (cb_ivf_frame_left <= 0)
			{
				iRet = RET_CODE_BUFFER_NOT_COMPATIBLE;
				break;
			}

			if ((read_size = (uint32_t)fread(buf, 1, 1, rfp)) <= 0)
			{
				iRet = RET_CODE_BUFFER_NOT_COMPATIBLE;
				break;
			}

			uint8_t temporal_id = (buf[0] >> 5) & 0x7;
			uint8_t spatial_id = (buf[0] >> 3) & 0x03;
			uint8_t extension_header_reserved_3bits = (buf[0] & 0x07);

			cb_ivf_frame_left--;
			cb_read_pos++;

			if (extension_header_reserved_3bits == 0)
			{
				printf("[AV1] Hit the invalid obu_extension_header where extension_header_reserved_3bits: %d.\n", extension_header_reserved_3bits);
				iRet = RET_CODE_BUFFER_NOT_COMPATIBLE;
				break;
			}
		}

		if (obu_has_size_flag)
		{
			int64_t obu_size = 0;
			read_size = AMP_MIN(8, cb_ivf_frame_left);
			if ((read_size = (uint32_t)fread(buf, 1, read_size, rfp)) <= 0)
			{
				iRet = RET_CODE_BUFFER_NOT_COMPATIBLE;
				goto done;
			}

			for (; i < read_size; i++) {
				obu_size |= ((int64_t)(buf[i] & 0x7f) << (i * 7));
				Leb128Bytes++;
				if (!(buf[i] & 0x80))break;
			}

			if (i >= read_size || obu_size >= UINT32_MAX || cb_ivf_frame_left < Leb128Bytes + obu_size)
			{
				printf("[AV1] the value(%" PRId64 ") returned from the leb128 parsing process should be less than or equal to (1<<32)-1 {file offset: %" PRIu32 "}\n",
					obu_size, cb_read_pos);
				break;
			}

			cb_ivf_frame_left -= (uint32_t)(Leb128Bytes + obu_size);
			cb_read_pos += (uint32_t)(Leb128Bytes + obu_size);
		}
	}

	bAnnexB = false;
	iRet = RET_CODE_SUCCESS;

done:
	if (rfp != nullptr)
		fclose(rfp);

	return iRet;
}

int AV1_ProcessLowOverheadBitstream(const char* szAV1FileName, int64_t start_pos, uint32_t& full_obu_size)
{
	// find 2 consequent open_bitstream_units
	int iRet = RET_CODE_ERROR;
	const int read_unit_size = 2048;
	int64_t cur_scan_pos = start_pos;
	int64_t file_len = 0;

	// Create a bigger ring buffer to avoid frequent linear ring buffer reform
	AMLinearRingBuffer rbRawBuf = AM_LRB_Create(read_unit_size * 128);

	int cbSize = 0;
	uint8_t* pStartBuf = NULL, *pBuf = NULL, *pCurParseStartBuf = NULL;

	// At first, try to open the stream file
	FILE* rfp = NULL;
	FOPEN(rfp, szAV1FileName, "rb");

	if (rfp == NULL)
	{
		iRet = RET_CODE_ERROR_FILE_NOT_EXIST;
		goto done;
	}

	// Get the file size
	if (_fseeki64(rfp, 0, SEEK_END) != 0)
		goto done;

	if ((file_len = _ftelli64(rfp)) <= start_pos)
	{
		iRet = RET_CODE_NO_MORE_DATA;
		goto done;
	}

	if (_fseeki64(rfp, start_pos, SEEK_SET) != 0)
		goto done;

	do
	{
		int read_size = read_unit_size;
		pBuf = AM_LRB_GetWritePtr(rbRawBuf, &read_size);
		if (pBuf == NULL || read_size < read_unit_size)
		{
			// Try to reform linear ring buffer
			AM_LRB_Reform(rbRawBuf);
			if ((pBuf = AM_LRB_GetWritePtr(rbRawBuf, &read_size)) == NULL || read_size < read_unit_size)
			{
				iRet = RET_CODE_ERROR;
				break;
			}
		}

		if ((read_size = (int)fread(pBuf, 1, read_unit_size, rfp)) <= 0)
		{
			iRet = RET_CODE_IO_READ_ERROR;
			break;
		}

		AM_LRB_SkipWritePtr(rbRawBuf, (unsigned int)read_size);

		if ((pStartBuf = AM_LRB_GetReadPtr(rbRawBuf, &cbSize)) == NULL || cbSize <= 0)
		{
			iRet = RET_CODE_ERROR;
			break;
		}

		uint8_t Leb128Bytes = 0;
		size_t i = 0, cbAvailLeb128ExtractBytes = 0;

		int cbAvailSize = cbSize;
		uint8_t* pOBUBuf = pStartBuf;

		// At the beginning of OBU payload, collect enough data to be parsed
		uint8_t obu_header_byte = *pOBUBuf;

		uint8_t obu_forbidden_bit = (obu_header_byte >> 7) & 0x01;
		uint8_t obu_type = (obu_header_byte >> 3) & 0xF;
		uint8_t obu_extension_flag = (obu_header_byte >> 2) & 0x1;
		uint8_t obu_has_size_flag = (obu_header_byte >> 1) & 0x1;
		uint8_t obu_reserved_1bit = obu_header_byte & 0x1;

		if (obu_forbidden_bit != 0 || obu_reserved_1bit != 0 || obu_has_size_flag == 0)
		{
			if (g_verbose_level > 0)
				printf("[AV1] Hit the invalid obu_header where obu_forbidden_bit: %d, obu_has_size_flag: %d, obu_reserved_1bit: %d.\n",
					obu_forbidden_bit, obu_has_size_flag, obu_reserved_1bit);
			goto done;
		}

		cbAvailSize--;
		pOBUBuf++;

		if (g_verbose_level > 0)
			printf("[AV1] \t\thit obu_type: %s.\n", obu_type_names[obu_type]);

		if (obu_extension_flag)
		{
			if (cbAvailSize < 1)
			{
				continue;	// No data is available, need read more data
			}

			uint8_t temporal_id = (*pOBUBuf >> 5) & 0x7;
			uint8_t spatial_id = (*pOBUBuf >> 3) & 0x03;
			uint8_t extension_header_reserved_3bits = (*pOBUBuf & 0x07);

			if (extension_header_reserved_3bits == 0)
			{
				if (g_verbose_level > 0)
					printf("[AV1] Hit the invalid obu_extension_header where extension_header_reserved_3bits: %d.\n", extension_header_reserved_3bits);
				goto done;
			}

			cbAvailSize--;
			pOBUBuf++;
		}

		if (obu_has_size_flag)
		{
			int64_t obu_size = 0;
			cbAvailLeb128ExtractBytes = AMP_MIN(8, cbAvailSize);
			for (; i < cbAvailLeb128ExtractBytes; i++)
			{
				obu_size |= ((int64_t)(pOBUBuf[i] & 0x7f) << (i * 7));
				Leb128Bytes++;
				if (!(pOBUBuf[i] & 0x80))
					break;
			}

			if (obu_size >= UINT32_MAX)
			{
				printf("[AV1][open_bitstream_unit] the value(%" PRId64 ") returned from the leb128 parsing process should be less than or equal to (1<<32)-1 {file offset: %" PRId64 "}\n",
					obu_size, cur_scan_pos + (ptrdiff_t)(pOBUBuf - pBuf));
				goto done;
			}

			if (i >= cbAvailLeb128ExtractBytes)
			{
				// Failed to extract leb128 from the enough data
				if (cbAvailSize >= 8)
				{
					printf("[AV1][open_bitstream_unit] Failed to extract leb128 size {file offset: %" PRId64 "}.\n", cur_scan_pos);
					goto done;
				}
				else
				{
					continue;	// No data is available, need read more data
				}
			}

			cbAvailSize -= Leb128Bytes;
			pOBUBuf += Leb128Bytes;

			full_obu_size = (uint32_t)obu_size;
		}

		full_obu_size += (uint32_t)(pOBUBuf - pStartBuf);

		break;

	} while (!feof(rfp));

	iRet = 0;

done:
	AM_LRB_Destroy(rbRawBuf);

	if (rfp != nullptr)
		fclose(rfp);

	return iRet;
}

int AV1_PreparseStream(const char* szAV1FileName, bool& bIsAnnexB)
{
	TEMPORAL_UNIT_LAYOUT temporal_unit_layout;
	int temporal_unit_num = 0;
	int64_t start_pos = 0;
	int iRet = 0;
	bool bIVF = false;

	const int MAX_TRIES_OF_SEARCHING_TU = 2;

	bIsAnnexB = false;

	auto iterContainer = g_params.find("container");
	if (iterContainer != g_params.end())
		bIVF = _stricmp(iterContainer->second.c_str(), "ivf") == 0 ? true : false;

	if (bIVF)
	{
		if (AMP_FAILED(iRet = AV1_ProcessIVFLowOverheadBitstream(szAV1FileName, bIsAnnexB)))
		{
			_tprintf(_T("[AV1] Unknown IVF AV1 stream file.\n"));
			return RET_CODE_ERROR;
		}
	}
	else
	{
		do
		{
			if (AMP_FAILED(iRet = AV1_ProcessAnnexBTemporalUnit(szAV1FileName, start_pos, temporal_unit_layout)))
			{
				if (iRet == RET_CODE_NO_MORE_DATA)
				{
					iRet = RET_CODE_SUCCESS;
					break;
				}

				break;
			}
			else
			{
				start_pos += (int64_t)std::get<0>(temporal_unit_layout) + std::get<1>(temporal_unit_layout);
				temporal_unit_num++;
			}
		} while (temporal_unit_num < MAX_TRIES_OF_SEARCHING_TU);

		if (temporal_unit_num > 0)
		{
			if (g_verbose_level > 0)
				printf("[AV1][temporal_unit#%08d] The current file: %s is an Annex-B length delimited bitstream.\n", temporal_unit_num, szAV1FileName);
			bIsAnnexB = true;
		}
		else
		{
			if (g_verbose_level > 0)
				printf("[AV1][temporal_unit#%08d] The current file: %s is NOT an Annex-B length delimited bitstream.\n", temporal_unit_num, szAV1FileName);
		}

		if (!bIsAnnexB)
		{
			// Try to check whether it is a low overhead bitstream format or not
			uint32_t full_obu_size = 0;
			if (AMP_FAILED(iRet = AV1_ProcessLowOverheadBitstream(szAV1FileName, 0, full_obu_size)))
			{
				_tprintf(_T("[AV1] Unknown AV1 stream file, no valid OBU.\n"));
				return RET_CODE_ERROR;
			}

			// Check whether there are 2 consequent open_bitstream_unit or not
			if (AMP_FAILED(iRet = AV1_ProcessLowOverheadBitstream(szAV1FileName, full_obu_size, full_obu_size)) && iRet != RET_CODE_NO_MORE_DATA)
			{
				_tprintf(_T("[AV1] Unknown AV1 stream file, no consequent OBUs.\n"));
				return RET_CODE_ERROR;
			}

			_tprintf(_T("[AV1] The current file: %s is a low overhead bitstream format.\n"), szAV1FileName);
		}
	}

	return RET_CODE_SUCCESS;
}

class CAV1BaseEnumerator : public CComUnknown, public IAV1Enumerator
{
public:
	CAV1BaseEnumerator(IAV1Context* pAV1Context)
		: m_pAV1Context(pAV1Context) {
		if (m_pAV1Context)
			m_pAV1Context->AddRef();
	}
	virtual ~CAV1BaseEnumerator() {
		AMP_SAFERELEASE(m_pAV1Context);
	}

	DECLARE_IUNKNOWN
	HRESULT NonDelegatingQueryInterface(REFIID uuid, void** ppvObj)
	{
		if (ppvObj == NULL)
			return E_POINTER;

		if (uuid == IID_IAV1Enumerator)
			return GetCOMInterface((IAV1Enumerator*)this, ppvObj);

		return CComUnknown::NonDelegatingQueryInterface(uuid, ppvObj);
	}

public:
	RET_CODE EnumNewVSEQ(IUnknown* pCtx) { return RET_CODE_SUCCESS; }
	RET_CODE EnumNewCVS(IUnknown* pCtx, int8_t reserved) { return RET_CODE_SUCCESS; }
	RET_CODE EnumTemporalUnitStart(IUnknown* pCtx, uint8_t* ptr_TU_buf, uint32_t TU_size, int frame_type) {return RET_CODE_SUCCESS;}
	RET_CODE EnumFrameUnitStart(IUnknown* pCtx, uint8_t* pFrameUnitBuf, uint32_t cbFrameUnitBuf, int frame_type) {return RET_CODE_SUCCESS;}
	RET_CODE EnumOBU(IUnknown* pCtx, uint8_t* pOBUBuf, size_t cbOBUBuf, uint8_t obu_type, uint32_t obu_size) {return RET_CODE_SUCCESS;}
	RET_CODE EnumFrameUnitEnd(IUnknown* pCtx, uint8_t* pFrameUnitBuf, uint32_t cbFrameUnitBuf) {return RET_CODE_SUCCESS;}
	RET_CODE EnumTemporalUnitEnd(IUnknown* pCtx, uint8_t* ptr_TU_buf, uint32_t TU_size) {return RET_CODE_SUCCESS;}
	RET_CODE EnumError(IUnknown* pCtx, uint64_t stream_offset, int error_code) {return RET_CODE_SUCCESS;}

protected:
	IAV1Context* m_pAV1Context = nullptr;
};

class CAV1ShowOBUEnumerator : public CAV1BaseEnumerator
{
public:
	CAV1ShowOBUEnumerator(IAV1Context* pCtx) : CAV1BaseEnumerator(pCtx) {}

	RET_CODE EnumNewVSEQ(IUnknown* pCtx) { return RET_CODE_SUCCESS; }
	RET_CODE EnumNewCVS(IUnknown* pCtx, int8_t reserved) { return RET_CODE_SUCCESS; }
	RET_CODE EnumTemporalUnitStart(IUnknown* pCtx, uint8_t* ptr_TU_buf, uint32_t TU_size, int frame_type) {
		m_FU_count_in_TU = 0;
		printf("Temporal Unit#[%" PRId64 "]:\n", m_TU_count);
		return RET_CODE_SUCCESS;
	}
	RET_CODE EnumFrameUnitStart(IUnknown* pCtx, uint8_t* pFrameUnitBuf, uint32_t cbFrameUnitBuf, int frame_type) {
		m_OBU_count_in_FU = 0;
		printf("%.*sFrame Unit#%" PRId64 "/global#%" PRId64 "\n", m_indent[1], m_szIndent, m_FU_count_in_TU, m_FU_count);
		return RET_CODE_SUCCESS;
	}
	RET_CODE EnumOBU(IUnknown* pCtx, uint8_t* pOBUBuf, size_t cbOBUBuf, uint8_t obu_type, uint32_t obu_size) {
		printf("%.*sOBU#%" PRId64 ": %-32s (len/size--%5zu/%-5" PRIu32 ")/global#%" PRId64 "\n",
			m_indent[2], m_szIndent, m_OBU_count_in_FU, obu_type < 0xF ? obu_type_names[obu_type] : "", cbOBUBuf, obu_size, m_OBU_count);
		m_OBU_count_in_FU++;
		m_OBU_count++;
		return RET_CODE_SUCCESS;
	}
	RET_CODE EnumFrameUnitEnd(IUnknown* pCtx, uint8_t* pFrameUnitBuf, uint32_t cbFrameUnitBuf) {
		m_FU_count_in_TU++;
		m_FU_count++;
		return RET_CODE_SUCCESS;
	}
	RET_CODE EnumTemporalUnitEnd(IUnknown* pCtx, uint8_t* ptr_TU_buf, uint32_t TU_size) {
		m_TU_count++;
		return RET_CODE_SUCCESS;
	}

public:
	int		m_indent[3] = { 0, 4, 8 };

protected:
	int64_t m_TU_count = 0;
	int64_t m_FU_count_in_TU = 0;
	int64_t m_FU_count = 0;
	int64_t m_OBU_count_in_FU = 0;
	int64_t m_OBU_count = 0;
	const char* m_szIndent = "                    ";
};

class CAV1ShowSeqHdrOBUEnumerator : public CAV1BaseEnumerator
{
public:
	CAV1ShowSeqHdrOBUEnumerator(IAV1Context* pCtx) : CAV1BaseEnumerator(pCtx) {}
	RET_CODE EnumOBU(IUnknown* pCtx, uint8_t* pOBUBuf, size_t cbOBUBuf, uint8_t obu_type, uint32_t obu_size) {
		if (obu_type == OBU_SEQUENCE_HEADER)
		{
			AMSHA1_RET retSHA1 = { 0 };
			AMSHA1 hSHA1 = AM_SHA1_Init(pOBUBuf, (unsigned long)cbOBUBuf);
			if (hSHA1 != nullptr)
			{
				AM_SHA1_Finalize(hSHA1);
				AM_SHA1_GetHash(hSHA1, retSHA1);
				AM_SHA1_Uninit(hSHA1);

				if (m_pAV1Context->GetSeqHdrOBU() == nullptr || memcmp(retSHA1, m_sha1SeqHdrOBU, sizeof(AMSHA1)) != 0)
				{
					BST::AV1::OPEN_BITSTREAM_UNIT* ptr_obu_seq_hdr = new BST::AV1::OPEN_BITSTREAM_UNIT(nullptr);
					if (ptr_obu_seq_hdr == nullptr)
						return RET_CODE_NOTHING_TODO;

					AV1_OBU sp_obu_seq_hdr = AV1_OBU(ptr_obu_seq_hdr);

					AMBst in_bst = AMBst_CreateFromBuffer(pOBUBuf, (int)cbOBUBuf);
					if (in_bst == nullptr)
						return RET_CODE_NOTHING_TODO;

					if (AMP_SUCCEEDED(sp_obu_seq_hdr->Map(in_bst)))
					{
						m_pAV1Context->UpdateSeqHdrOBU(sp_obu_seq_hdr);

						PrintMediaObject(sp_obu_seq_hdr.get());

						memcpy(m_sha1SeqHdrOBU, retSHA1, sizeof(AMSHA1));
					}

					AMBst_Destroy(in_bst);
				}
			}
		}

		return RET_CODE_SUCCESS;
	}

protected:
	AMSHA1_RET m_sha1SeqHdrOBU = { 0 };
};

enum AV1_INFO_CMD
{
	AV1_INFO_CMD_LIST_OBU = 0,
	AV1_INFO_CMD_SHOW_OBUSEQHDR = 1,
};

int	AV1InfoCmd(AV1_INFO_CMD cmd)
{
	IAV1Context* pAV1Context = nullptr;
	uint8_t pBuf[2048] = { 0 };

	const int read_unit_size = 2048;

	FILE* rfp = NULL;
	int iRet = RET_CODE_SUCCESS;
	int64_t file_size = 0;

	auto iter_srcfmt = g_params.find("srcfmt");
	if (iter_srcfmt == g_params.end())
		return RET_CODE_ERROR_NOTIMPL;

	if (iter_srcfmt->second.compare("av1") != 0)
		return RET_CODE_ERROR_NOTIMPL;

	int options = 0;
	std::string& strShowOBU = g_params["showOBU"];
	std::vector<std::string> strShowAUOptions;
	splitstr(strShowOBU.c_str(), ",;.:", strShowAUOptions);
	if (strShowAUOptions.size() == 0)
		options = AV1_ENUM_OPTION_ALL;
	else
	{
		for (auto& sopt : strShowAUOptions)
		{
			if (MBCSICMP(sopt.c_str(), "tu") == 0)
				options |= AV1_ENUM_OPTION_TU;

			if (MBCSICMP(sopt.c_str(), "fu") == 0)
				options |= AV1_ENUM_OPTION_FU;

			if (MBCSICMP(sopt.c_str(), "obu") == 0)
				options |= AV1_ENUM_OPTION_OBU;
		}
	}

	int top = GetTopRecordCount();
	bool bAnnexBStream = false, bIVF = false;

	auto iterContainer = g_params.find("container");
	if (iterContainer != g_params.end())
		bIVF = _stricmp(iterContainer->second.c_str(), "ivf") == 0 ? true : false;

	if (AMP_FAILED(AV1_PreparseStream(g_params["input"].c_str(), bAnnexBStream)))
	{
		printf("Failed to detect it is a low-overhead bit-stream or length-delimited AV1 bit-stream.\n");
		return RET_CODE_ERROR_NOTIMPL;
	}

	printf("%s AV1 bitstream...\n", bAnnexBStream ? "Length-Delimited" : "Low-Overhead");

	CAV1Parser AV1Parser(bAnnexBStream, false, bIVF, nullptr);
	IUnknown* pMSECtx = nullptr;
	if (AMP_FAILED(AV1Parser.GetContext(&pMSECtx)) ||
		FAILED(pMSECtx->QueryInterface(IID_IAV1Context, (void**)&pAV1Context)))
	{
		AMP_SAFERELEASE(pMSECtx);
		printf("Failed to get the AV1 context.\n");
		return RET_CODE_ERROR_NOTIMPL;
	}
	AMP_SAFERELEASE(pMSECtx);

	CAV1BaseEnumerator* pAV1Enumerator = nullptr;

	if (cmd == AV1_INFO_CMD_LIST_OBU)
	{
		CAV1ShowOBUEnumerator* pAV1ShowOBUEnumerator = new CAV1ShowOBUEnumerator(pAV1Context);

		if (!(options&AV1_ENUM_OPTION_TU))
		{
			pAV1ShowOBUEnumerator->m_indent[1] = 0;
			pAV1ShowOBUEnumerator->m_indent[2] = 4;
		}

		if (!(options&AV1_ENUM_OPTION_FU))
			pAV1ShowOBUEnumerator->m_indent[2] -= 4;

		pAV1Enumerator = (CAV1BaseEnumerator*)pAV1ShowOBUEnumerator;
	}
	else if (cmd == AV1_INFO_CMD_SHOW_OBUSEQHDR)
	{
		CAV1ShowSeqHdrOBUEnumerator* pAV1SeqHdrOBUEnumerator = new CAV1ShowSeqHdrOBUEnumerator(pAV1Context);
		pAV1Enumerator = (CAV1BaseEnumerator*)pAV1SeqHdrOBUEnumerator;
	}

	errno_t errn = fopen_s(&rfp, g_params["input"].c_str(), "rb");
	if (errn != 0 || rfp == NULL)
	{
		printf("Failed to open the file: %s {errno: %d}.\n", g_params["input"].c_str(), errn);
		goto done;
	}

	// Get file size
	_fseeki64(rfp, 0, SEEK_END);
	file_size = _ftelli64(rfp);
	_fseeki64(rfp, 0, SEEK_SET);

	AV1Parser.SetEnumerator((IAV1Enumerator*)(pAV1Enumerator), options);

	do
	{
		int read_size = read_unit_size;
		if ((read_size = (int)fread(pBuf, 1, read_unit_size, rfp)) <= 0)
		{
			iRet = RET_CODE_IO_READ_ERROR;
			break;
		}

		iRet = AV1Parser.ProcessInput(pBuf, read_size);
		if (AMP_FAILED(iRet))
			break;

		iRet = AV1Parser.ProcessOutput();
		if (iRet == RET_CODE_ABORT)
			break;

	} while (!feof(rfp));

	if (feof(rfp))
		iRet = AV1Parser.ProcessOutput(true);

done:
	if (rfp != nullptr)
		fclose(rfp);

	if (pAV1Context)
	{
		pAV1Context->Release();
		pAV1Context = nullptr;
	}

	return iRet;
}

int	ShowOBUs()
{
	return AV1InfoCmd(AV1_INFO_CMD_LIST_OBU);
}

int	ShowOBUSeqHdr()
{
	return AV1InfoCmd(AV1_INFO_CMD_SHOW_OBUSEQHDR);
}

int ShowAV1Info()
{
	return -1;
}

