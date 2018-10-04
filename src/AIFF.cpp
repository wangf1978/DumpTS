#include "StdAfx.h"
#include "AIFF.h"

extern std::unordered_map<std::string, std::string> g_params;
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