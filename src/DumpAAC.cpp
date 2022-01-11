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
#include <stdint.h>
#include "ISO14496_3.h"
#include "DataUtil.h"
#include "mediaobjprint.h"
#include <unordered_map>

using namespace std;

extern unordered_map<std::string, std::string> g_params;

int	ShowStreamMuxConfig()
{
	BST::AACAudio::IMP4AACContext* pCtxMP4AAC = nullptr;
	uint8_t pBuf[2048] = { 0 };

	const int read_unit_size = 2048;

	FILE* rfp = NULL;
	int iRet = RET_CODE_SUCCESS;
	int64_t file_size = 0;

	auto iter_srcfmt = g_params.find("srcfmt");
	if (iter_srcfmt == g_params.end() || iter_srcfmt->second.compare("loas") != 0)
		return RET_CODE_ERROR_NOTIMPL;

	int options = 0;

	int top = -1;
	auto iterTop = g_params.find("top");
	if (iterTop != g_params.end())
	{
		int64_t top_records = -1;
		ConvertToInt(iterTop->second, top_records);
		if (top_records < 0 || top_records > INT32_MAX)
			top = -1;
	}

	BST::AACAudio::CLOASParser LOASParser;
	if (AMP_FAILED(LOASParser.GetMP4AContext(&pCtxMP4AAC)))
	{
		printf("Failed to get the MPEG4 AAC context.\n");
		return RET_CODE_ERROR;
	}

	class CLOASEnumerator : public BST::AACAudio::ILOASEnumerator
	{
	public:
		CLOASEnumerator(BST::AACAudio::IMP4AACContext* pCtxMP4AAC) : m_pCtxMP4AAC(pCtxMP4AAC) {
			memset(audio_specific_config_sha1, 0, sizeof(audio_specific_config_sha1));
		}

		virtual ~CLOASEnumerator() {}

		RET_CODE EnumLATMAUBegin(BST::AACAudio::IMP4AACContext* pCtx, uint8_t* pLATMAUBuf, size_t cbLATMAUBuf)
		{
			//printf("Access-Unit#%" PRIu64 "\n", m_AUCount);

			auto mux_stream_config = pCtx->GetMuxStreamConfig();

			// check whether the SHA1, and judge whether AudioSpecificConfig is changed
			for (int prog = 0; prog < 16; prog++)
			{
				for (int lay = 0; lay < 8; lay++)
				{
					if (mux_stream_config->AudioSpecificConfig[prog][lay] == nullptr)
						continue;

					if (memcmp(mux_stream_config->AudioSpecificConfig[prog][lay]->sha1_value, audio_specific_config_sha1[prog][lay], sizeof(AMSHA1_RET)) != 0)
					{
						printf("Audio Stream#%d:\n", mux_stream_config->streamID[prog][lay]);
						PrintMediaObject(mux_stream_config->AudioSpecificConfig[prog][lay]);
						memcpy(audio_specific_config_sha1[prog][lay], mux_stream_config->AudioSpecificConfig[prog][lay]->sha1_value, sizeof(AMSHA1_RET));
					}
				}
			}

			return RET_CODE_SUCCESS;
		}

		RET_CODE EnumSubFrameBegin(BST::AACAudio::IMP4AACContext* pCtx, uint8_t* pSubFramePayload, size_t cbSubFramePayload)
		{
			return RET_CODE_SUCCESS;
		}

		RET_CODE EnumSubFrameEnd(BST::AACAudio::IMP4AACContext* pCtx, uint8_t* pSubFramePayload, size_t cbSubFramePayload)
		{
			return RET_CODE_SUCCESS;
		}

		RET_CODE EnumLATMAUEnd(BST::AACAudio::IMP4AACContext* pCtx, uint8_t* pLATMAUBuf, size_t cbLATMAUBuf)
		{
			m_AUCount++;
			return RET_CODE_SUCCESS;
		}

		RET_CODE EnumError(BST::AACAudio::IMP4AACContext* pCtx, RET_CODE error_code)
		{
			printf("Hitting an error {code: %d}!\n", error_code);
			return RET_CODE_SUCCESS;
		}

		BST::AACAudio::IMP4AACContext* m_pCtxMP4AAC = nullptr;
		uint64_t m_AUCount = 0;
		AMSHA1_RET audio_specific_config_sha1[16][8];
	}LOASEnumerator(pCtxMP4AAC);

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

	LOASParser.SetEnumerator((BST::AACAudio::ILOASEnumerator*)(&LOASEnumerator), options);

	do
	{
		int read_size = read_unit_size;
		if ((read_size = (int)fread(pBuf, 1, read_unit_size, rfp)) <= 0)
		{
			iRet = RET_CODE_IO_READ_ERROR;
			break;
		}

		iRet = LOASParser.ProcessInput(pBuf, read_size);
		if (AMP_FAILED(iRet))
			break;

		iRet = LOASParser.ProcessOutput();
		if (iRet == RET_CODE_ABORT)
			break;

	} while (!feof(rfp));

	if (feof(rfp))
		iRet = LOASParser.ProcessOutput(true);

done:
	if (rfp != nullptr)
		fclose(rfp);

	AMP_SAFERELEASE(pCtxMP4AAC);
	return iRet;
}


