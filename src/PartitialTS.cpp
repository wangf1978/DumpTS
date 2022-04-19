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
#include "DataUtil.h"
#include "system_13818_1.h"

using namespace std;

extern const char *dump_msg[];
extern map<std::string, std::string, CaseInsensitiveComparator> g_params;
extern MEDIA_SCHEME_TYPE					g_source_media_scheme_type;
extern MEDIA_SCHEME_TYPE					CheckAndUpdateFileFormat(std::string& filepath, const char* param_name);

// Dump a partial TS
int DumpPartialTS()
{
	errno_t errn = 0;
	int nDumpRet = -1;
	FILE *fp = NULL, *fw = NULL;
	uint8_t offset = 0;
	uint8_t ts_pack_size = TS_PACKET_SIZE - 4;
	uint8_t buf[TS_PACKET_SIZE] = { 0 };
	std::set<uint16_t> pid_filters;
	bool bTTSOutput = false;
	int64_t num_of_ts_packs_skip = 0;
	int64_t num_of_ts_packs_written = 0;

	auto iter_srcfmt = g_params.find("srcfmt");
	auto iter_dstfmt = g_params.find("outputfmt");
	auto iter_inputfile = g_params.find("input");
	auto iter_outputfile = g_params.find("output");
	auto iter_PID = g_params.find("pid");
	auto iterStart = g_params.find("start");
	auto iterEnd = g_params.find("end");

	int64_t nStart = -1LL, nEnd = INT64_MAX, iVal = -1LL;
	if (iterStart != g_params.end())
	{
		iVal = ConvertToLongLong(iterStart->second);
		if (iVal >= 0 && iVal <= UINT32_MAX)
			nStart = (uint32_t)iVal;
	}

	if (iterEnd != g_params.end())
	{
		iVal = ConvertToLongLong(iterEnd->second);
		if (iVal >= 0 && iVal <= UINT32_MAX)
			nEnd = (uint32_t)iVal;
	}

	if (g_params.find("outputfmt") == g_params.end())
	{
		// Assume the output format is the same with input format
		g_params["outputfmt"] = g_params["srcfmt"];
		iter_dstfmt = g_params.find("outputfmt");
	}

	if (iter_PID != g_params.end())
	{
		std::vector<std::string> strPIDs;
		splitstr(iter_PID->second.c_str(), ",;.:", strPIDs);
		for (auto& strPID : strPIDs)
		{
			int64_t pidVal = -1LL;
			if (ConvertToInt(strPID, pidVal) && pidVal >= 0LL && pidVal <= 0x1FFFLL)
				pid_filters.insert((uint16_t)pidVal);
			else
				printf("Found an invalid PID: %s\n", strPID.c_str());
		}

		if (pid_filters.size() == 0)
		{
			nDumpRet = -1;
			printf("Please specify a valid PID or multiple valid PIDs delimited by ',.:;'.\n");
			goto done;
		}
	}

	if (iter_srcfmt == g_params.end() ||
		(_stricmp(iter_srcfmt->second.c_str(), "ts") != 0 &&
		 _stricmp(iter_srcfmt->second.c_str(), "tts") != 0 &&
		 _stricmp(iter_srcfmt->second.c_str(), "m2ts") != 0))
	{
		nDumpRet = -1;
		printf("Only support extracting stream/PSI from transport stream.\n");
		goto done;
	}

	if (iter_dstfmt == g_params.end() ||
		(_stricmp(iter_dstfmt->second.c_str(), "ts") != 0 &&
		 _stricmp(iter_dstfmt->second.c_str(), "tts") != 0 &&
		 _stricmp(iter_dstfmt->second.c_str(), "m2ts") != 0))
	{
		nDumpRet = -1;
		printf("Only support extracting stream/PSI to transport stream.\n");
		goto done;
	}

	if (iter_inputfile == g_params.end())
	{
		printf("Please specify an input transport stream file.\n");
		nDumpRet = -1;
		goto done;
	}

	if (iter_outputfile == g_params.end())
	{
		printf("Please specify an output transport stream file.\n");
		nDumpRet = -1;
		goto done;
	}

	// support:
	// m2ts/tts -> tts
	// m2ts/tts -> ts
	// not support:
	// ts -> tts
	if (_stricmp(iter_srcfmt->second.c_str(), "ts") == 0 && 
			(_stricmp(iter_dstfmt->second.c_str(), "tts") == 0 ||
			 _stricmp(iter_dstfmt->second.c_str(), "m2ts") == 0))
	{
		nDumpRet = -1;
		printf("Does NOT support extracting ts to tts or m2ts.\n");
		goto done;
	}

	if (_stricmp(iter_srcfmt->second.c_str(), "tts") == 0 || _stricmp(iter_srcfmt->second.c_str(), "m2ts") == 0)
	{
		ts_pack_size = TS_PACKET_SIZE;
		offset = 4;
	}

	if (_stricmp(iter_dstfmt->second.c_str(), "tts") == 0 || _stricmp(iter_dstfmt->second.c_str(), "m2ts") == 0)
	{
		bTTSOutput = true;
	}

	errn = fopen_s(&fp, iter_inputfile->second.c_str(), "rb");
	if (errn != 0 || fp == NULL)
	{
		printf("Failed to open the file: %s {errno: %d}.\n", iter_inputfile->second.c_str(), errn);
		goto done;
	}

	errn = fopen_s(&fw, iter_outputfile->second.c_str(), "wb+");
	if (errn != 0 || fw == NULL)
	{
		printf("Failed to open the file: %s {errno: %d}.\n", iter_outputfile->second.c_str(), errn);
		goto done;
	}

	if (nStart > 0)
	{
		_fseeki64(fp, nStart*ts_pack_size, SEEK_SET);
		num_of_ts_packs_skip = nStart;
	}

	while (true)
	{
		if (num_of_ts_packs_skip >= nEnd)
			break;

		size_t nRead = fread(buf, 1, ts_pack_size, fp);
		if (nRead < ts_pack_size)
			break;

		uint16_t PID = ((buf[offset + 1] & 0x3F) << 8) | buf[offset + 2];
		if (pid_filters.find(PID) == pid_filters.end() && pid_filters.size() != 0)
			continue;

		size_t nWritten = 0;
		if (bTTSOutput || offset == 0)
			nWritten = fwrite(buf, 1, ts_pack_size, fw);
		else
			nWritten = fwrite(buf + 4, 1, ts_pack_size - 4, fw);

		if (nWritten + ((bTTSOutput || offset == 0) ? 0 : 4) < (size_t)ts_pack_size)
		{
			printf("Failed to write the data into destination file.\n");
			break;
		}

		num_of_ts_packs_written++;
		num_of_ts_packs_skip++;
	}

	printf("Write %" PRId64 " transport stream packs successfully.\n", num_of_ts_packs_written);
	nDumpRet = 0;

done:
	if (fp != nullptr)
		fclose(fp);

	if (fw != nullptr)
		fclose(fw);

	return nDumpRet;
}
