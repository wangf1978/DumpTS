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

extern map<std::string, std::string, CaseInsensitiveComparator> g_params;

int	ShowStreamMuxConfig(bool bOnlyShowAudioSpecificConfig)
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

	int top = GetTopRecordCount();

	BST::AACAudio::CLOASParser LOASParser;
	IUnknown* pMSECtx = nullptr;
	if (AMP_FAILED(LOASParser.GetContext(&pMSECtx)) ||
		FAILED(pMSECtx->QueryInterface(IID_IMP4AACContext, (void**)&pCtxMP4AAC)))
	{
		AMP_SAFERELEASE(pMSECtx);
		printf("Failed to get the MPEG4 AAC context.\n");
		return RET_CODE_ERROR_NOTIMPL;
	}
	AMP_SAFERELEASE(pMSECtx);

	class CLOASEnumerator : public CComUnknown, public ILOASEnumerator
	{
	public:
		CLOASEnumerator(BST::AACAudio::IMP4AACContext* pCtxMP4AAC, bool bOnlyShowAudioSpecificConfig) 
			: m_pCtxMP4AAC(pCtxMP4AAC)
			, m_bOnlyShowAudioSpecificConfig(bOnlyShowAudioSpecificConfig){
			if (m_pCtxMP4AAC)
				m_pCtxMP4AAC->AddRef();
			memset(audio_specific_config_sha1, 0, sizeof(audio_specific_config_sha1));
		}

		virtual ~CLOASEnumerator() {
			AMP_SAFERELEASE(m_pCtxMP4AAC);
		}

		DECLARE_IUNKNOWN
		HRESULT NonDelegatingQueryInterface(REFIID uuid, void** ppvObj)
		{
			if (ppvObj == NULL)
				return E_POINTER;

			if (uuid == IID_ILOASEnumerator)
				return GetCOMInterface((ILOASEnumerator*)this, ppvObj);

			return CComUnknown::NonDelegatingQueryInterface(uuid, ppvObj);
		}

	public:
		RET_CODE EnumLATMAUBegin(IUnknown* pCtx, uint8_t* pLATMAUBuf, size_t cbLATMAUBuf)
		{
			//printf("Access-Unit#%" PRIu64 "\n", m_AUCount);
			if (pCtx == nullptr)
				return RET_CODE_INVALID_PARAMETER;

			BST::AACAudio::IMP4AACContext* pMP4ACtx = nullptr;
			if (FAILED(pCtx->QueryInterface(IID_IMP4AACContext, (void**)&pMP4ACtx)))
				return RET_CODE_INVALID_PARAMETER;

			auto mux_stream_config = pMP4ACtx->GetMuxStreamConfig();

			// check whether the SHA1, and judge whether AudioSpecificConfig is changed
			for (int prog = 0; prog < 16; prog++)
			{
				for (int lay = 0; lay < 8; lay++)
				{
					if (mux_stream_config->AudioSpecificConfig[prog][lay] == nullptr)
						continue;

					if (memcmp(mux_stream_config->AudioSpecificConfig[prog][lay]->sha1_value, audio_specific_config_sha1[prog][lay], sizeof(AMSHA1_RET)) != 0)
					{
						if (m_bOnlyShowAudioSpecificConfig)
						{
							printf("Audio Stream#%d:\n", mux_stream_config->streamID[prog][lay]);
							PrintMediaObject(mux_stream_config->AudioSpecificConfig[prog][lay].get());

							// Also show the audio frame duration
							auto audio_specific_config = mux_stream_config->AudioSpecificConfig[prog][lay];
							auto audio_object_type = audio_specific_config->GetAudioObjectType();

							int frameLength = 0;	// Unknown
							if (audio_object_type == BST::AACAudio::AAC_main ||
								audio_object_type == BST::AACAudio::AAC_LC ||
								audio_object_type == BST::AACAudio::AAC_SSR ||
								audio_object_type == BST::AACAudio::AAC_LTP ||
								audio_object_type == BST::AACAudio::AAC_Scalable ||
								audio_object_type == BST::AACAudio::TwinVQ ||
								audio_object_type == BST::AACAudio::ER_AAC_LC ||
								audio_object_type == BST::AACAudio::ER_AAC_LTP ||
								audio_object_type == BST::AACAudio::ER_AAC_scalable ||
								audio_object_type == BST::AACAudio::ER_TwinVQ ||
								audio_object_type == BST::AACAudio::ER_BSAC ||
								audio_object_type == BST::AACAudio::ER_AAC_LD)
							{
								int nSamplingRates[] = { 96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000, 7350, -1, -1, -1 };

								if (audio_object_type == BST::AACAudio::AAC_SSR)
									frameLength = 256;
								else if (audio_object_type == BST::AACAudio::ER_AAC_LD)
									frameLength = audio_specific_config->GASpecificConfig->frameLengthFlag ? 480 : 512;
								else
									frameLength = audio_specific_config->GASpecificConfig->frameLengthFlag ? 960 : 1024;

								if (audio_specific_config->samplingFrequencyIndex == 0xF)
								{
									printf("\tFrame Duration: %" PRIu64 ".%03" PRIu32 "ms\n", 
										(uint64_t)(frameLength * 1000ULL / audio_specific_config->samplingFrequency),
										(uint32_t)(frameLength * 1000000ULL / audio_specific_config->samplingFrequency%1000));
								}
								else if (nSamplingRates[audio_specific_config->samplingFrequencyIndex] > 0)
								{
									printf("\tFrame Duration: %" PRIu64 ".%03" PRIu32 "ms\n",
										(uint64_t)(frameLength * 1000ULL / nSamplingRates[audio_specific_config->samplingFrequencyIndex]),
										(uint32_t)(frameLength * 1000000ULL / nSamplingRates[audio_specific_config->samplingFrequencyIndex] % 1000));
								}
							}
						}
						else
						{
							printf("Updated Stream Mux Config:\n");
							PrintMediaObject(mux_stream_config.get());
						}
						memcpy(audio_specific_config_sha1[prog][lay], mux_stream_config->AudioSpecificConfig[prog][lay]->sha1_value, sizeof(AMSHA1_RET));
					}
				}
			}

			AMP_SAFERELEASE(pMP4ACtx);

			return RET_CODE_SUCCESS;
		}

		RET_CODE EnumSubFrameBegin(IUnknown* pCtx, uint8_t* pSubFramePayload, size_t cbSubFramePayload){return RET_CODE_SUCCESS;}
		RET_CODE EnumSubFrameEnd(IUnknown* pCtx, uint8_t* pSubFramePayload, size_t cbSubFramePayload){return RET_CODE_SUCCESS;}

		RET_CODE EnumLATMAUEnd(IUnknown* pCtx, uint8_t* pLATMAUBuf, size_t cbLATMAUBuf)
		{
			m_AUCount++;
			return RET_CODE_SUCCESS;
		}

		RET_CODE EnumError(IUnknown* pCtx, RET_CODE error_code)
		{
			printf("Hitting an error {code: %d}!\n", error_code);
			return RET_CODE_SUCCESS;
		}

		BST::AACAudio::IMP4AACContext* m_pCtxMP4AAC = nullptr;
		uint64_t m_AUCount = 0;
		AMSHA1_RET audio_specific_config_sha1[16][8];
		bool m_bOnlyShowAudioSpecificConfig;
	}LOASEnumerator(pCtxMP4AAC, bOnlyShowAudioSpecificConfig);

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

	LOASEnumerator.AddRef();
	LOASParser.SetEnumerator((IUnknown*)(&LOASEnumerator), options);

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


