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
#include "AIFF.h"

extern std::map<std::string, std::string, CaseInsensitiveComparator> g_params;
extern int g_verbose_level;

int DumpAIFF()
{
	int nRet = RET_CODE_SUCCESS;
	if (g_params.find("input") == g_params.end())
		return -1;

	std::string szOutputFile;
	std::string szOutputFmt;
	std::string& szInputFile = g_params["input"];

	if (g_params.find("outputfmt") != g_params.end())
		szOutputFmt = g_params["outputfmt"];

	if (g_params.find("output") != g_params.end())
		szOutputFile = g_params["output"];

	CFileBitstream bs(szInputFile.c_str(), 4096, &nRet);

	if (nRet < 0)
	{
		printf("Failed to open the file: %s.\n", szInputFile.c_str());
		return nRet;
	}

	AIFF::FormAIFCChunk form_chunk;
	nRet = form_chunk.Unpack(bs);

	form_chunk.Print();

	return nRet;
}