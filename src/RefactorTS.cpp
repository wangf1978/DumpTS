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
#include <stdio.h>
#include "PayloadBuf.h"
#include "AMRingBuffer.h"
#include <algorithm>

extern map<std::string, std::string, CaseInsensitiveComparator> g_params;

// Refactor TS, for example, change PID, PTS, or changing from ts to m2ts and so on
int RefactorTS()
{
	// At present, only support the below operation
	// Src PID#1 -> Dest PID#1
	// Src PID#2 -> Dest PID#2
	// ...
	// Add 30 bits of ATC clock time at the beginning of 188 bytes

	int iRet = -1;
	unsigned long ulATCTime = 0x1951D2E;
	bool bAppendATCTime = false;
	unordered_map<int, int> pid_maps;
	unsigned char buf[TS_PACKET_SIZE];
	FILE *fp = NULL, *fw = NULL;
	unordered_map<unsigned short, CPayloadBuf*> pPayloadBufs;

	unsigned long pes_buffer_len = 0;
	unsigned char buf_head_offset = 0;	// dumpopt&DUMP_BD_M2TS ? 4 : 0;
	unsigned char ts_pack_size = TS_PACKET_SIZE - 4;	// dumpopt&DUMP_BD_M2TS ? TS_PACKET_SIZE : TS_PACKET_SIZE - 4;
	unsigned char dest_ts_pack_size = TS_PACKET_SIZE - 4;

	unsigned long ts_pack_idx = 0;

	errno_t errn = fopen_s(&fp, g_params["input"].c_str(), "rb");
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

	if (g_params.find("srcfmt") == g_params.end())
	{
		// Check its extension name
		char szExt[_MAX_EXT];
		memset(szExt, 0, sizeof(szExt));
		_splitpath_s(g_params["input"].c_str(), NULL, 0, NULL, 0, NULL, 0, szExt, _MAX_EXT);

		if (_stricmp(szExt, ".ts") == 0)
			g_params["srcfmt"] = "ts";
		else if (_stricmp(szExt, ".m2ts") == 0)
			g_params["srcfmt"] = "m2ts";
		else
		{
			// Implement it later, need scan the current TS, and decide which kind of TS stream it is.
			goto done;
		}
	}

	if (g_params.find("outputfmt") == g_params.end())
	{
		// Assume the output format is the same with input format
		g_params["outputfmt"] = g_params["srcfmt"];
	}

	// Check whether to add 30bits of ATC time at the beginning of each 188 TS packet
	if (g_params["outputfmt"].compare("m2ts") == 0 && g_params["srcfmt"].compare("ts") == 0)
		bAppendATCTime = true;

	if (g_params.find("pid") != g_params.end() && g_params.find("destpid") != g_params.end())
	{
		// Set up the PID maps.
		// Assume every PIDs are separated with ',' or ';'
		const char* p = g_params["pid"].c_str(), *s = p;
		std::vector<int> src_pids;

		for (;;)
		{
			if (*p == ',' || *p == ';' || *p == '\0')
			{
				std::string strPID(s, p - s);
				long long llPID = ConvertToLongLong(strPID);
				if (llPID >= 0 && llPID <= 0x1FFFLL)
					src_pids.push_back((int)llPID);
				else
					src_pids.push_back(-1);

				if (*p == '\0')
					break;

				s = p + 1;
			}

			p++;
		}

		s = p = g_params["destpid"].c_str();
		std::vector<int> dest_pids;

		for (;;)
		{
			if (*p == ',' || *p == ';' || *p == '\0')
			{
				std::string strPID(s, p - s);
				long long llPID = ConvertToLongLong(strPID);
				if (llPID >= 0 && llPID <= 0x1FFFLL)
					dest_pids.push_back((int)llPID);
				else
					dest_pids.push_back(-1);

				if (*p == '\0')
					break;

				s = p + 1;
			}

			p++;
		}

		for (size_t i = 0; i < std::max(src_pids.size(), dest_pids.size()); i++)
		{
			if (src_pids[i] == -1)
				continue;

			if (dest_pids[i] == -1)
				continue;

			pid_maps[src_pids[i]] = dest_pids[i];
		}
	}

	if (g_params.find("srcfmt") != g_params.end() && g_params["srcfmt"].compare("m2ts") == 0)
		ts_pack_size = TS_PACKET_SIZE;

	if (g_params.find("outputfmt") != g_params.end() && g_params["outputfmt"].compare("m2ts") == 0)
	{
		buf_head_offset = 4;
		dest_ts_pack_size = TS_PACKET_SIZE;
	}

	while (true)
	{
		size_t nRead = fread(buf, 1, ts_pack_size, fp);
		if (nRead < ts_pack_size)
			break;

		// Try to append ATC time
		if (dest_ts_pack_size == 192 && ts_pack_size == 188)
		{
			memmove(buf + 4, buf, 188);

			if (bAppendATCTime)
			{
				buf[0] = (ulATCTime >> 24) & 0xFF;
				buf[1] = (ulATCTime >> 16) & 0xFF;
				buf[2] = (ulATCTime >> 8) & 0xFF;
				buf[3] = ulATCTime & 0xFF;

				ulATCTime += 0x1F2;			// Fixed value at present, later we may support the accurate ATC time
				ulATCTime &= 0x3FFFFFFF;
			}
			else
				*((unsigned long*)(&buf[0])) = 0;
		}
		else if (dest_ts_pack_size == 188 && ts_pack_size == 192)
		{
			memmove(buf, buf + 4, 188);
		}
		else
		{
			printf("Don't know how to process {%s(), %s: %d}.\n", __FUNCTION__, __FILE__, __LINE__);
			break;
		}

		// Try to change PID
		unsigned short PID = (buf[buf_head_offset + 1] & 0x1f) << 8 | buf[buf_head_offset + 2];
		if (pid_maps.find(PID) != pid_maps.end())
		{
			buf[buf_head_offset + 1] &= 0xE0;
			buf[buf_head_offset + 1] |= ((pid_maps[PID] >> 8) & 0x1F);
			buf[buf_head_offset + 2] = pid_maps[PID] & 0xFF;
		}

		if (fw != NULL)
		{
			if (fwrite(buf, 1, dest_ts_pack_size, fw) < dest_ts_pack_size)
			{
				printf("Failed to write %d bytes into output file.\n", dest_ts_pack_size);
				break;
			}
		}

		int index = buf_head_offset + 4;
		unsigned char payload_unit_start = buf[buf_head_offset + 1] & 0x40;
		unsigned char adaptation_field_control = (buf[buf_head_offset + 3] >> 4) & 0x03;
		//unsigned char discontinuity_counter = buf[buf_head_offset + 3] & 0x0f;

		if (pPayloadBufs.find(PID) != pPayloadBufs.end())
		{
			if (payload_unit_start)
			{
				pPayloadBufs[PID]->Process(pid_maps);
				pPayloadBufs[PID]->Reset();
			}
		}
		else
			pPayloadBufs[PID] = new CPayloadBuf(fw, dest_ts_pack_size);

		if (adaptation_field_control & 0x02)
			index += buf[buf_head_offset + 4] + 1;

		if (payload_unit_start || (!payload_unit_start && pes_buffer_len > 0))
		{
			pPayloadBufs[PID]->PushTSBuf(ts_pack_idx, buf, index, dest_ts_pack_size);
		}

		ts_pack_idx++;
	}

	if (feof(fp) && dest_ts_pack_size == 192 && ts_pack_size == 188 && (ts_pack_idx % 32) != 0)
	{
		// Fill the NULL packet to be aligned with 6K
		int padding_pack_count = 32 - (ts_pack_idx % 32);
		for (int i = 0; i < padding_pack_count; i++)
		{
			memset(buf + 4, 0xFF, 188);
			buf[4] = 0x47;
			buf[5] = 0x1F;
			buf[6] = 0xFF;
			buf[7] = 0x10;

			if (bAppendATCTime)
			{
				buf[0] = (ulATCTime >> 24) & 0xFF;
				buf[1] = (ulATCTime >> 16) & 0xFF;
				buf[2] = (ulATCTime >> 8) & 0xFF;
				buf[3] = ulATCTime & 0xFF;

				ulATCTime += 0x1F2;			// Fixed value at present, later we may support the accurate ATC time
				ulATCTime &= 0x3FFFFFFF;
			}
			else
				*((unsigned long*)(&buf[0])) = 0;

			if (fw != NULL)
			{
				if (fwrite(buf, 1, dest_ts_pack_size, fw) < dest_ts_pack_size)
				{
					printf("Failed to write %d bytes into output file.\n", dest_ts_pack_size);
					break;
				}
			}
		}
	}

	for (std::unordered_map<unsigned short, CPayloadBuf*>::iterator iter = pPayloadBufs.begin(); iter != pPayloadBufs.end(); iter++)
	{
		if (iter->second != NULL)
			iter->second->Process(pid_maps);
	}

	iRet = 0;

done:
	if (fp)
		fclose(fp);
	if (fw)
		fclose(fw);

	for (std::unordered_map<unsigned short, CPayloadBuf*>::iterator iter = pPayloadBufs.begin(); iter != pPayloadBufs.end(); iter++)
	{
		delete iter->second;
		iter->second = NULL;
	}

	return iRet;
}