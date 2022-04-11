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
#include "PayloadBuf.h"
#include "DumpTS.h"
#include "Bitstream.h"
#include <assert.h>
#include <functional>
#include "DataUtil.h"
#include "mpeg2_video_parser.h"
#include "mpeg2_video.h"
#include "system_13818_1.h"

extern std::map<std::string, std::string, CaseInsensitiveComparator> g_params;
extern int g_verbose_level;
extern DUMP_STATUS g_dump_status;
extern TS_FORMAT_INFO	g_ts_fmtinfo;

extern int FlushPESBuffer(
	unsigned char* pes_buffer,
	int pes_buffer_len,
	int &raw_data_len,
	PES_FILTER_INFO& filter_info,
	FILE* fw,
	int dumpopt);

extern const char* vui_transfer_characteristics_names[256];
extern const char* vui_colour_primaries_names[256];
extern const char* chroma_format_idc_names[4];

#define MPEG_PROGRAM_END_CODE					0xB9
#define MPEG_PROGRAM_PACK_START_CODE			0xBA
#define MPEG_PROGRAM_SYSTEM_HEADER_START_CODE	0xBB
#define MPEG_PROGRAM_MAP_STREAM_ID				0xBC

int DumpOneStreamFromPS()
{
	int iRet = -1, dump_ret = -1;
	FILE *fp = NULL, *fw = NULL;
	int stream_id = -1;
	int sub_stream_id = -1;
	int stream_id_extension = -1;
	int dumpopt = 0;
	errno_t errn;
	int64_t start_pspck_pos = 0;
	int64_t end_pspck_pos = std::numeric_limits<decltype(end_pspck_pos)>::max();
	uint16_t ts_pack_size = g_ts_fmtinfo.packet_size;
	unsigned long ps_pack_idx = 0;
	bool bCopyOriginalStream = false;
	int cur_pes_len = 0;
	int raw_data_len = 0;
	int stream_type = -1;
	STREAM_CAT stream_cat = STREAM_CAT_UNKNOWN;
	int stream_number = 0;

	int pes_buf_pos = 0;
	int pes_buf_size = 10 * 1024 * 1024;
	unsigned char* pes_buffer = new (std::nothrow) unsigned char[pes_buf_size];
	unsigned char buf[2052];
	int cbSize = 0;
	uint8_t* pStartBuf = NULL, *pBuf = NULL, *pCurParseStartBuf = NULL;

	if (g_params.find("stream_id") == g_params.end())
	{
		printf("No stream_id is specified, nothing to do.\n");
		goto done;
	}

	stream_id = (int)ConvertToLongLong(g_params["stream_id"]);

	if (g_params.find("sub_stream_id_extension") != g_params.end())
		stream_id_extension = (int)ConvertToLongLong(g_params["stream_id_extension"]);

	if (g_params.find("sub_stream_id") != g_params.end())
		sub_stream_id = (int)ConvertToLongLong(g_params["sub_stream_id"]);

	if (g_params.find("srcfmt") != g_params.end() && g_params["srcfmt"].compare("vob") == 0)
		dumpopt |= DUMP_VOB;

	// Make sure the dump option
	if (g_params.find("outputfmt") != g_params.end())
	{
		std::string& strOutputFmt = g_params["outputfmt"];
		if (strOutputFmt.compare("es") == 0)
			dumpopt |= DUMP_RAW_OUTPUT;
		else if (strOutputFmt.compare("pes") == 0)
			dumpopt |= DUMP_PES_OUTPUT;
		else if (strOutputFmt.compare("pcm") == 0)
			dumpopt |= DUMP_PCM;
		else if (strOutputFmt.compare("wav") == 0)
			dumpopt |= DUMP_WAV;
	}

	if (g_params.find("showpts") != g_params.end())
		dumpopt |= DUMP_PTS_VIEW;

	if (g_params.find("showinfo") != g_params.end())
		dumpopt |= DUMP_MEDIA_INFO_VIEW;

	if (dumpopt&DUMP_VOB)
	{
		if (g_params.find("start") != g_params.end())
		{
			start_pspck_pos = ConvertToLongLong(g_params["start"]);

			// Check the start ts pack position
			if (start_pspck_pos < 0 || (unsigned long long)start_pspck_pos >= g_dump_status.num_of_packs)
			{
				iRet = RET_CODE_INVALID_PARAMETER;
				printf("The current start pack pos(%" PRId64 ") exceed the valid range [0, %" PRIu64 ").\n", start_pspck_pos, g_dump_status.num_of_packs);
				goto done;
			}

			if (FSEEK64(fp, start_pspck_pos*ts_pack_size, SEEK_SET) != 0)
			{
				iRet = RET_CODE_ERROR;
				printf("Failed to seek the specified position {err: %d}.\n", ferror(fp));
				goto done;
			}

			g_dump_status.cur_pack_idx = start_pspck_pos;
			ps_pack_idx = (unsigned long)start_pspck_pos;
		}
		else
			start_pspck_pos = 0;

		if (g_params.find("end") != g_params.end())
		{
			end_pspck_pos = ConvertToLongLong(g_params["end"]);

			// Check the end ts pack position
			if (end_pspck_pos <= 0 || (unsigned long long)end_pspck_pos > g_dump_status.num_of_packs)
			{
				iRet = RET_CODE_INVALID_PARAMETER;
				printf("The current end pack pos(%" PRId64 ") exceed the valid range (0, %" PRIu64 "].\n", end_pspck_pos, g_dump_status.num_of_packs);
				goto done;
			}
		}
		else
			end_pspck_pos = std::numeric_limits<decltype(end_pspck_pos)>::max();
	}

	if (g_params.find("outputfmt") != g_params.end())
	{
		bCopyOriginalStream = _stricmp(g_params["outputfmt"].c_str(), "copy") == 0 ? true : false;
		if (g_verbose_level > 0)
		{
			if (start_pspck_pos > 0 || (end_pspck_pos > 0 && end_pspck_pos != std::numeric_limits<decltype(end_pspck_pos)>::max()))
				printf("copy a part of the original stream file.\n");
			else
				printf("copy the original stream file.\n");
		}
	}

	if ((stream_id & 0xF0) == 0xE0)
	{
		g_params["video"] = "";
		if (dumpopt&DUMP_VOB)
			stream_type = 2;	// MPEG-2 video
	}
	else if ((stream_id & 0xF8) == 0xC0)
	{
		stream_type = MPEG2_AUDIO_STREAM;
	}

	if (dumpopt&DUMP_VOB)
	{
		if (stream_id == 0xBD)
		{
			// TODO...
		}
	}

	errn = fopen_s(&fp, g_params["input"].c_str(), "rb");
	if (errn != 0 || fp == NULL)
	{
		printf("Failed to open the file: %s {errno: %d}.\n", g_params["input"].c_str(), errn);
		goto done;
	}

	if (g_params.find("output") != g_params.end())
	{
		errn = fopen_s(&fw, g_params["output"].c_str(), "wb+");
		if (errn != 0 || fw == NULL)
		{
			printf("Failed to open the file: %s {errno: %d}.\n", g_params["output"].c_str(), errn);
			goto done;
		}
	}

	g_dump_status.state = DUMP_STATE_RUNNING;

	while (true)
	{
		// Check whether to reach the stopping position or not
		if (g_dump_status.cur_pack_idx >= (unsigned long long)end_pspck_pos)
		{
			if (end_pspck_pos != std::numeric_limits<decltype(end_pspck_pos)>::max())
				printf("Reach the end position: %" PRId64 ", stop dumping the stream.\n", end_pspck_pos);
			break;
		}

		uint16_t nRead = (uint16_t)fread(buf + cbSize, 1, 2048, fp);
		if (nRead <= 0)
			break;

		cbSize += nRead;

		pCurParseStartBuf = pBuf = buf;
		while (cbSize >= 4)
		{
			// only check the stream ID for program sequence, ignore others, for example, sequence, GOP, picture, slice and so on
			while (cbSize >= 4 && !(pBuf[0] == 0 && pBuf[1] == 0 && pBuf[2] == 1 && pBuf[3] >= MPEG_PROGRAM_END_CODE))
			{
				cbSize--;
				pBuf++;
			}

			if (pes_buf_pos > 0 && pBuf > pCurParseStartBuf)
			{
				ptrdiff_t pick_up_size = pBuf - pCurParseStartBuf;
				if (pes_buf_pos + pick_up_size > pes_buf_size)
				{
					size_t new_pes_buf_size = pes_buf_pos + pick_up_size;
					unsigned char* new_pes_buffer = new(std::nothrow) unsigned char[new_pes_buf_size];
					if (new_pes_buffer == NULL)
					{
						iRet = RET_CODE_OUTOFMEMORY;
						goto done;
					}

					if (pes_buf_pos > 0)
						memcpy(new_pes_buffer, pes_buffer, pes_buf_pos);

					delete[] pes_buffer;
					pes_buffer = new_pes_buffer;
				}

				memcpy(pes_buffer + pes_buf_pos, pCurParseStartBuf, pick_up_size);
				pes_buf_pos += (int)pick_up_size;
			}

			// Failed to find the start code
			if (cbSize < 4)
				break;

			if (pes_buf_pos > 0)
			{
				if (pes_buffer[3] == MPEG_PROGRAM_PACK_START_CODE)
				{
					// Skip it
					pes_buf_pos = 0;
				}
				else if (pes_buffer[3] == MPEG_PROGRAM_SYSTEM_HEADER_START_CODE)
				{
					// Skip it
					pes_buf_pos = 0;
				}
				else if (pes_buffer[3] >= MPEG_PROGRAM_MAP_STREAM_ID)
				{
					PES_FILTER_INFO pes_filter_info;
					pes_filter_info.VOB.Stream_CAT = stream_cat;
					pes_filter_info.VOB.StreamIndex = stream_number;

					dump_ret = FlushPESBuffer(pes_buffer, pes_buf_pos, raw_data_len, pes_filter_info, fw, dumpopt);
					pes_buf_pos = 0;
				}
				else
				{
					pes_buf_pos = 0;
				}
			}

			if (pBuf[3] == stream_id)
			{
				pes_buf_pos = 4;
				memcpy(pes_buffer, pBuf, 4);
			}

			pBuf += 4;
			cbSize -= 4;

			pCurParseStartBuf = pBuf;
		}

		assert(cbSize < 4);
		if (cbSize > 0)
		{
			// Move the unparsed data to the beginning of buffer, and next read will start from cbSize to begin reading
			memcpy(buf, pBuf, cbSize);
		}
	}

	if (cbSize > 0)
	{
		if (pes_buf_pos + cbSize < pes_buf_size)
		{
			size_t new_pes_buf_size = (size_t)pes_buf_pos + cbSize;
			unsigned char* new_pes_buffer = new(std::nothrow) unsigned char[new_pes_buf_size];
			if (new_pes_buffer == NULL)
			{
				iRet = RET_CODE_OUTOFMEMORY;
				goto done;
			}

			if (pes_buf_pos > 0)
				memcpy(new_pes_buffer, pes_buffer, pes_buf_pos);

			delete[] pes_buffer;
			pes_buffer = new_pes_buffer;
		}

		memcpy(pes_buffer, pBuf, cbSize);
		pes_buf_pos += cbSize;
	}

	if (pes_buf_pos > 4 && pes_buffer[3] >= MPEG_PROGRAM_MAP_STREAM_ID)
	{
		PES_FILTER_INFO pes_filter_info;
		pes_filter_info.VOB.Stream_CAT = stream_cat;
		pes_filter_info.VOB.StreamIndex = stream_number;

		dump_ret = FlushPESBuffer(pes_buffer, pes_buf_pos, raw_data_len, pes_filter_info, fw, dumpopt);
	}

done:
	if (fp)
		fclose(fp);
	
	if (fw)
		fclose(fw);

	if (pes_buffer)
		delete[] pes_buffer;

	return iRet;
}


int GetStreamInfoSeqHdrAndExt(
	BST::MPEG2Video::CSequenceHeader* pSeqHdr, 
	BST::MPEG2Video::CSequenceExtension* pSeqExt, 
	BST::MPEG2Video::CSequenceDisplayExtension* pSeqDispExt,
	STREAM_INFO& stm_info)
{
	if (pSeqHdr == nullptr || pSeqExt == nullptr)
		return RET_CODE_NEEDMOREINPUT;

	stm_info.video_info.profile = pSeqExt->GetProfile();
	stm_info.video_info.level = pSeqExt->GetLevel();
	stm_info.video_info.video_width = (pSeqExt->horizontal_size_extension << 12) | pSeqHdr->horizontal_size_value;
	stm_info.video_info.video_height = (pSeqExt->vertical_size_extension << 12) | pSeqHdr->vertical_size_value;

	if (pSeqDispExt)
	{
		stm_info.video_info.video_width = pSeqDispExt->display_horizontal_size;
		stm_info.video_info.video_height = pSeqDispExt->display_vertical_size;
		if (pSeqDispExt->colour_description)
		{
			stm_info.video_info.colour_primaries = pSeqDispExt->colour_primaries;
			stm_info.video_info.transfer_characteristics = pSeqDispExt->transfer_characteristics;
		}
	}

	stm_info.video_info.chroma_format_idc = pSeqExt->chroma_format;
	stm_info.video_info.bitrate = (uint32_t)(((uint32_t)pSeqExt->bit_rate_extension << 18) | pSeqHdr->bit_rate_value) * 400;
	switch (pSeqHdr->frame_rate_code)
	{
	case 1: stm_info.video_info.framerate_numerator = 24000; stm_info.video_info.framerate_denominator = 1001; break;
	case 2: stm_info.video_info.framerate_numerator = 24; stm_info.video_info.framerate_denominator = 1; break;
	case 3: stm_info.video_info.framerate_numerator = 25; stm_info.video_info.framerate_denominator = 1; break;
	case 4: stm_info.video_info.framerate_numerator = 30000; stm_info.video_info.framerate_denominator = 1001; break;
	case 5: stm_info.video_info.framerate_numerator = 30; stm_info.video_info.framerate_denominator = 1; break;
	case 6: stm_info.video_info.framerate_numerator = 50; stm_info.video_info.framerate_denominator = 1; break;
	case 7: stm_info.video_info.framerate_numerator = 60000; stm_info.video_info.framerate_denominator = 1001; break;
	case 8: stm_info.video_info.framerate_numerator = 60; stm_info.video_info.framerate_denominator = 1; break;
	}

	stm_info.video_info.framerate_numerator *= (pSeqExt->frame_rate_extension_n + 1);
	stm_info.video_info.framerate_denominator *= (pSeqExt->frame_rate_extension_d + 1);

	uint64_t c = gcd(stm_info.video_info.framerate_numerator, stm_info.video_info.framerate_denominator);

	stm_info.video_info.framerate_numerator = (uint32_t)(stm_info.video_info.framerate_numerator / c);
	stm_info.video_info.framerate_denominator = (uint32_t)(stm_info.video_info.framerate_denominator / c);

	if (pSeqHdr->aspect_ratio_information == 1)
	{
		c = gcd(stm_info.video_info.video_width, stm_info.video_info.video_height);
		stm_info.video_info.aspect_ratio_numerator = (uint32_t)(stm_info.video_info.video_width / c);
		stm_info.video_info.aspect_ratio_denominator = (uint32_t)(stm_info.video_info.video_height / c);
	}
	else if (pSeqHdr->aspect_ratio_information == 2)
	{
		stm_info.video_info.aspect_ratio_numerator = 4;
		stm_info.video_info.aspect_ratio_denominator = 3;
	}
	else if (pSeqHdr->aspect_ratio_information == 3)
	{
		stm_info.video_info.aspect_ratio_numerator = 16;
		stm_info.video_info.aspect_ratio_denominator = 9;
	}
	else if (pSeqHdr->aspect_ratio_information == 4)
	{
		stm_info.video_info.aspect_ratio_numerator = 221;
		stm_info.video_info.aspect_ratio_denominator = 100;
	}

	return RET_CODE_SUCCESS;
}

#define SHOW_MPV_AU				0x01
#define SHOW_MPV_SUMMARY		0x02
#define SHOW_MPV_UNIT_DETAIL	0x04
/*
	filters		{extension_id | start_code, ....}
	for example, sequence header: (0x00<<8) | 0xB3 and so on

	show option:
		0x01	show the AU
		0x02	print the rough information
		0x04	print the object information
*/
int ShowMPVUnit(std::initializer_list<uint16_t> filters, int show_options)
{
	IMPVContext* pMPVContext = nullptr;
	uint8_t pBuf[2048] = { 0 };

	const int read_unit_size = 2048;

	FILE* rfp = NULL;
	int iRet = RET_CODE_SUCCESS;
	int64_t file_size = 0;

	auto iter_srcfmt = g_params.find("srcfmt");
	if (iter_srcfmt == g_params.end())
		return RET_CODE_ERROR_NOTIMPL;

	int top = GetTopRecordCount();

	CMPEG2VideoParser MPVParser;
	IUnknown* pMSECtx = nullptr;
	if (AMP_FAILED(MPVParser.GetContext(&pMSECtx)) ||
		FAILED(pMSECtx->QueryInterface(IID_IMPVContext, (void**)&pMPVContext)))
	{
		AMP_SAFERELEASE(pMSECtx);
		printf("Failed to get the MPV context.\n");
		return RET_CODE_ERROR_NOTIMPL;
	}
	AMP_SAFERELEASE(pMSECtx);

	pMPVContext->SetStartCodeFilters(filters);

	class CMPVEnumerator : public CComUnknown, public IMPVEnumerator
	{
	public:
		CMPVEnumerator(IMPVContext* pCtx, int options)
			: m_pMPVContext(pCtx), m_show_options(options) {
			if (m_pMPVContext)
				m_pMPVContext->AddRef();
		}

		virtual ~CMPVEnumerator() {
			AMP_SAFERELEASE(m_pMPVContext);
		}

		DECLARE_IUNKNOWN
		HRESULT NonDelegatingQueryInterface(REFIID uuid, void** ppvObj)
		{
			if (ppvObj == NULL)
				return E_POINTER;

			if (uuid == IID_IMPVEnumerator)
				return GetCOMInterface((IMPVEnumerator*)this, ppvObj);

			return CComUnknown::NonDelegatingQueryInterface(uuid, ppvObj);
		}

	public:
		RET_CODE EnumVSEQStart(IUnknown* pCtx) { return RET_CODE_SUCCESS; }
		RET_CODE EnumNewGOP(IUnknown* pCtx, bool closed_gop, bool broken_link) { return RET_CODE_SUCCESS; }
		RET_CODE EnumAUStart(IUnknown* pCtx, uint8_t* pAUBuf, size_t cbAUBuf, int picCodingType) { return RET_CODE_SUCCESS; }
		RET_CODE EnumAUEnd(IUnknown* pCtx, uint8_t* pAUBuf, size_t cbAUBuf, int picCodingType) { return RET_CODE_SUCCESS; }
		RET_CODE EnumObject(IUnknown* pCtx, uint8_t* pBufWithStartCode, size_t cbBufWithStartCode)
		{
			if (cbBufWithStartCode < 4 || cbBufWithStartCode > INT32_MAX)
				return RET_CODE_NOTHING_TODO;

			AMBst bst = nullptr;
			RET_CODE ret_code = RET_CODE_SUCCESS;
			uint8_t mpv_start_code = pBufWithStartCode[3];
			if (mpv_start_code == SEQUENCE_HEADER_CODE)
			{
				m_bHitSeqHdr = true;
			}
			else if (mpv_start_code == EXTENSION_START_CODE)
			{
				uint8_t extension_start_code_identifier = ((pBufWithStartCode[4] >> 4) & 0xFF);
				if (m_pMPVContext->GetCurrentLevel() == 0)
				{
					if (extension_start_code_identifier == SEQUENCE_EXTENSION_ID)
					{
					}

					if (extension_start_code_identifier == SEQUENCE_DISPLAY_EXTENSION_ID)
					{
						// Try to find sequence_display_extension()
						if ((bst = AMBst_CreateFromBuffer(pBufWithStartCode, (int)cbBufWithStartCode)) == nullptr)
						{
							printf("Failed to create a bitstream object.\n");
							return RET_CODE_ABORT;
						}

						try
						{
							AMBst_SkipBits(bst, 32);
							int left_bits = 0;

							BST::MPEG2Video::CSequenceDisplayExtension* pSeqDispExt =
								new (std::nothrow) BST::MPEG2Video::CSequenceDisplayExtension;
							if (pSeqDispExt == nullptr)
							{
								printf("Failed to create Sequence_Display_Extension.\n");
								ret_code = RET_CODE_ABORT;
								goto done;
							}
							if (AMP_FAILED(ret_code = pSeqDispExt->Map(bst)))
							{
								delete pSeqDispExt;
								printf("Failed to parse the sequence_display_extension() {error code: %d}.\n", ret_code);
								goto done;
							}
							m_sp_sequence_display_extension = std::shared_ptr<BST::MPEG2Video::CSequenceDisplayExtension>(pSeqDispExt);
						}
						catch (AMException& e)
						{
							ret_code = e.RetCode();
						}
					}
				}
			}
			else if (mpv_start_code == PICTURE_START_CODE)
			{
				if (m_show_options&SHOW_MPV_SUMMARY)
				{
					if (m_bHitSeqHdr)
					{
						STREAM_INFO stm_info;
						if (AMP_SUCCEEDED(GetStreamInfoSeqHdrAndExt(m_pMPVContext->GetSeqHdr().get(),
							m_pMPVContext->GetSeqExt().get(),
							m_sp_sequence_display_extension.get(), stm_info)) && memcmp(&stm_info, &m_prev_stm_info, sizeof(stm_info)) != 0)
						{
							if (stm_info.video_info.profile != BST::MPEG2Video::MPV_PROFILE_UNKNOWN)
								printf("\tMPEG2 Video Profile: %s\n", mpeg2_profile_names[stm_info.video_info.profile]);

							if (stm_info.video_info.level != BST::MPEG2Video::MPV_LEVEL_UNKNOWN)
								printf("\tMPEG2 Video Level: %s\n", mpeg2_level_names[stm_info.video_info.level]);

							if (stm_info.video_info.video_width != 0 && stm_info.video_info.video_height != 0)
								printf("\tVideo Resolution: %" PRIu32 "x%" PRIu32 "\n", stm_info.video_info.video_width, stm_info.video_info.video_height);

							if (stm_info.video_info.aspect_ratio_numerator != 0 && stm_info.video_info.aspect_ratio_denominator != 0)
								printf("\tVideo Aspect Ratio: %d:%d\n", stm_info.video_info.aspect_ratio_numerator, stm_info.video_info.aspect_ratio_denominator);

							if (stm_info.video_info.chroma_format_idc >= 0 && stm_info.video_info.chroma_format_idc <= 3)
								printf("\tVideo Chroma format: %s\n", chroma_format_idc_names[stm_info.video_info.chroma_format_idc]);

							if (stm_info.video_info.colour_primaries > 0)
								printf("\tColor Primaries: %s\n", vui_colour_primaries_names[stm_info.video_info.colour_primaries]);

							if (stm_info.video_info.transfer_characteristics > 0)
								printf("\tEOTF: %s\n", vui_transfer_characteristics_names[stm_info.video_info.transfer_characteristics]);

							if (stm_info.video_info.framerate_denominator != 0 && stm_info.video_info.framerate_numerator != 0)
								printf("\tFrame rate: %" PRIu32 "/%" PRIu32 " fps\n", stm_info.video_info.framerate_numerator, stm_info.video_info.framerate_denominator);

							if (stm_info.video_info.bitrate != 0 && stm_info.video_info.bitrate != UINT32_MAX)
							{
								uint32_t k = 1000;
								uint32_t m = k * k;
								if (stm_info.video_info.bitrate >= m)
									printf("\tBitrate: %" PRIu32 ".%03" PRIu32 " Mbps.\n", stm_info.video_info.bitrate / m, (uint32_t)(((uint64_t)stm_info.video_info.bitrate * 1000) / m % 1000));
								else if (stm_info.video_info.bitrate >= k)
									printf("\tBitrate: %" PRIu32 ".%03" PRIu32 " Kbps.\n", stm_info.video_info.bitrate / k, stm_info.video_info.bitrate * 1000 / k % 1000);
								else
									printf("\tBitrate: %" PRIu32 " bps.\n", stm_info.video_info.bitrate);
							}

							m_prev_stm_info = stm_info;
						}

						m_bHitSeqHdr = false;
					}
				}
			}

		done:
			if (bst != nullptr)
				AMBst_Destroy(bst);
			return ret_code;
		}
		RET_CODE EnumVSEQEnd(IUnknown* pCtx) { return RET_CODE_SUCCESS; }
		RET_CODE EnumError(IUnknown* pCtx, uint64_t stream_offset, int error_code) { return RET_CODE_SUCCESS; }

		IMPVContext*			m_pMPVContext;
		int						m_show_options;
		std::shared_ptr<BST::MPEG2Video::CSequenceDisplayExtension>
								m_sp_sequence_display_extension;
		STREAM_INFO				m_prev_stm_info;
		bool					m_bHitSeqHdr = false;

	} MPVEnumerator(pMPVContext, show_options);

	MPVEnumerator.AddRef();
	MPVParser.SetEnumerator(&MPVEnumerator, MPV_ENUM_OPTION_SE | ((show_options&0x01)?MPV_ENUM_OPTION_AU:0));

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

	do
	{
		int read_size = read_unit_size;
		if ((read_size = (int)fread(pBuf, 1, read_unit_size, rfp)) <= 0)
		{
			iRet = RET_CODE_IO_READ_ERROR;
			break;
		}

		iRet = MPVParser.ProcessInput(pBuf, read_size);
		if (AMP_FAILED(iRet))
			break;

		iRet = MPVParser.ProcessOutput();
		if (iRet == RET_CODE_ABORT)
			break;

	} while (!feof(rfp));

	if (feof(rfp))
		iRet = MPVParser.ProcessOutput(true);

done:
	if (rfp != nullptr)
		fclose(rfp);

	if (pMPVContext)
	{
		pMPVContext->Release();
		pMPVContext = nullptr;
	}

	return iRet;
}

int	ShowMPVInfo()
{
	return ShowMPVUnit({}, SHOW_MPV_SUMMARY);
}

