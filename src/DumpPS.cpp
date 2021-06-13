#include "StdAfx.h"
#include "PayloadBuf.h"
#include "DumpTS.h"
#include "Bitstream.h"
#include <assert.h>
#include <functional>

extern std::unordered_map<std::string, std::string> g_params;
extern int g_verbose_level;
extern DUMP_STATUS g_dump_status;
extern TS_FORMAT_INFO	g_ts_fmtinfo;

extern int FlushPESBuffer(FILE* fw,
	unsigned short PID,
	int stream_type,
	unsigned char* pes_buffer,
	int pes_buffer_len,
	int dumpopt,
	int &raw_data_len,
	int stream_id = -1,
	int stream_id_extension = -1,
	int sub_stream_id = -1);

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
				else if (pes_buffer[3] == stream_id)
				{
					dump_ret = FlushPESBuffer(fw, 0x1FFF, stream_type, pes_buffer, pes_buf_pos, dumpopt, raw_data_len, stream_id, -1, sub_stream_id);
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
			size_t new_pes_buf_size = pes_buf_pos + cbSize;
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

	if (pes_buf_pos > 4 && pes_buffer[3] == stream_id)
	{
		dump_ret = FlushPESBuffer(fw, 0x1FFF, stream_type, pes_buffer, pes_buf_pos, dumpopt, raw_data_len, stream_id, -1, sub_stream_id);
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

