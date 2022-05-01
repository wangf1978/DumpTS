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
#include <stdio.h>
#include <map>
#include "video_transcoder.h"
#include "systemdef.h"
#include "DataUtil.h"
#include "MSE.h"
#include "nal_com.h"
#include "mpeg2_video_parser.h"
#include "MSEnav.h"

extern std::map<std::string, std::string, CaseInsensitiveComparator> g_params;

extern MEDIA_SCHEME_TYPE			g_source_media_scheme_type;
extern MEDIA_SCHEME_TYPE			g_output_media_scheme_type;

extern int LoadVTCExport(VTC_EXPORT& vtc_exports);
extern int UnloadVTCExport(VTC_EXPORT& vtc_exports);

class CMPVAUEnumerator : public CMPVNavEnumerator
{
public:
	CMPVAUEnumerator(IMPVContext* pCtx, uint32_t options, MSENav* pMSENav)
		: CMPVNavEnumerator(pCtx, options, pMSENav) {}

public:
	RET_CODE OnProcessAU(IUnknown* pCtx, uint8_t* pAUBuf, size_t cbAUBuf, int picCodingType)
	{
		return RET_CODE_ERROR_NOTIMPL;
	}

protected:
	vtc_param_t		m_vtc_params;
};


int BindAUTranscodeEnumerator(IMSEParser* pMSEParser, IUnknown* pCtx, uint32_t enum_options, MSENav& mse_nav, FILE* fp, VTC_EXPORT& vtc_export)
{
	if (pMSEParser == nullptr || pCtx == nullptr)
		return RET_CODE_ERROR_NOTIMPL;

	int iRet = RET_CODE_ERROR_NOTIMPL;
	MEDIA_SCHEME_TYPE scheme_type = pMSEParser->GetSchemeType();

/*	if (scheme_type == MEDIA_SCHEME_NAL)
	{
		INALContext* pNALCtx = nullptr;
		if (SUCCEEDED(pCtx->QueryInterface(IID_INALContext, (void**)&pNALCtx)))
		{
			IUnknown* pMSEEnumerator = nullptr;
			uint32_t options = mse_nav.GetEnumOptions();
			CNALSEEnumerator* pNALSEEnumerator = new CNALSEEnumerator(pNALCtx, enum_options | options, &mse_nav);
			if (SUCCEEDED(iRet = pNALSEEnumerator->QueryInterface(__uuidof(IUnknown), (void**)&pMSEEnumerator)))
			{
				iRet = pMSEParser->SetEnumerator(pMSEEnumerator, enum_options | options);
				AMP_SAFERELEASE(pMSEEnumerator);
			}

			AMP_SAFERELEASE(pNALCtx);
		}
	}
	else if (scheme_type == MEDIA_SCHEME_AV1)
	{
		IAV1Context* pAV1Ctx = nullptr;
		if (SUCCEEDED(pCtx->QueryInterface(IID_IAV1Context, (void**)&pAV1Ctx)))
		{
			IUnknown* pMSEEnumerator = nullptr;
			uint32_t options = mse_nav.GetEnumOptions();
			CAV1SEEnumerator* pAV1SEEnumerator = new CAV1SEEnumerator(pAV1Ctx, enum_options | options, &mse_nav);
			if (SUCCEEDED(iRet = pAV1SEEnumerator->QueryInterface(__uuidof(IUnknown), (void**)&pMSEEnumerator)))
			{
				iRet = pMSEParser->SetEnumerator(pMSEEnumerator, enum_options | options);
				AMP_SAFERELEASE(pMSEEnumerator);
			}

			AMP_SAFERELEASE(pAV1Ctx);
		}
	}
	else */if (scheme_type == MEDIA_SCHEME_MPV)
	{
		IMPVContext* pMPVCtx = nullptr;
		if (SUCCEEDED(pCtx->QueryInterface(IID_IMPVContext, (void**)&pMPVCtx)))
		{
			IUnknown* pMSEEnumerator = nullptr;
			uint32_t options = mse_nav.GetEnumOptions();



			CMPVAUEnumerator* pMPVAUEnumerator = new CMPVAUEnumerator(pMPVCtx, enum_options | options, &mse_nav);
			if (SUCCEEDED(iRet = pMPVAUEnumerator->QueryInterface(__uuidof(IUnknown), (void**)&pMSEEnumerator)))
			{
				iRet = pMSEParser->SetEnumerator(pMSEEnumerator, enum_options | options);
				AMP_SAFERELEASE(pMSEEnumerator);
			}

			AMP_SAFERELEASE(pMPVCtx);
		}
	}
	else if (scheme_type == MEDIA_SCHEME_LOAS_LATM)
	{

	}
	else if (scheme_type == MEDIA_SCHEME_ISOBMFF)
	{

	}

	return iRet;
}

int32_t get_video_stream_fourcc(const char* srcfmt)
{
	if (_stricmp(srcfmt, "h264") == 0)
		return 'avc3';
	else if (_stricmp(srcfmt, "h266") == 0)
		return 'hev1';
	else if (_stricmp(srcfmt, "mpv") == 0)
		return 'mp2v';
	else if (_stricmp(srcfmt, "av1") == 0)
		return 'av01';
	
	return 0;
}

int InitVideoTranscodeParams(VTC_EXPORT& vtc_export, vtc_param_t& vtc_params)
{
	int iRet = RET_CODE_SUCCESS;
	int64_t width = 0, height = 0;
	auto iter_dstfmt	= g_params.find("outputfmt");
	auto iter_srcfmt	= g_params.find("srcfmt");
	auto iter_bitrate	= g_params.find("bitrate");
	auto iter_width		= g_params.find("width");
	auto iter_height	= g_params.find("height");
	auto iter_fps		= g_params.find("fps");
	auto iter_sar		= g_params.find("SAR");
	auto iter_profile	= g_params.find("profile");
	auto iter_tier		= g_params.find("tier");
	auto iter_level		= g_params.find("level");

	if (iter_srcfmt == g_params.end())
	{
		printf("Can't create video encoder since no source format is specified.\n");
		return RET_CODE_ERROR;
	}

	if (iter_dstfmt == g_params.end())
	{
		printf("Can't create video encoder since no target format is specified.\n");
		return RET_CODE_ERROR;
	}

	vtc_export.fn_params_init(&vtc_params);

	vtc_params.src_stream_fourcc = get_video_stream_fourcc(iter_srcfmt->second.c_str());
	vtc_params.dst_stream_fourcc = get_video_stream_fourcc(iter_dstfmt->second.c_str());

	if (vtc_params.src_stream_fourcc == 0) {
		printf("Can't get the FOURCC for the input source format, and abort the transcode.\n");
		iRet = RET_CODE_INVALID_PARAMETER;
		goto done;
	}
	else if (vtc_params.dst_stream_fourcc == 0) 
	{
		vtc_params.dst_stream_fourcc = vtc_params.src_stream_fourcc;
	}

	// Check the width and height
	if (iter_width != g_params.end())
	{
		if (ConvertToInt(iter_width->second, width) == false || width < 0 || width > INT32_MAX)
		{
			iRet = RET_CODE_ERROR_NOTIMPL;
			goto done;
		}

		vtc_params.width = (int32_t)width;
	}

	if (iter_height != g_params.end())
	{
		if (ConvertToInt(iter_height->second, height) == false || height < 0 || height > INT32_MAX)
		{
			iRet = RET_CODE_ERROR_NOTIMPL;
			goto done;
		}

		vtc_params.height = (int32_t)height;
	}

	// check and parse fps
	if (iter_fps != g_params.end())
	{
		int64_t fps_num = -1, fps_den = -1;
		if (ConvertToRationalNumber(iter_fps->second, fps_num, fps_den) == false ||
			fps_num < 0 || fps_num > UINT32_MAX ||
			fps_den < 0 || fps_den > UINT32_MAX)
		{
			printf("The 'fps' parameter \"--fps=%s\" can't be parsed.\n", iter_fps->second.c_str());
			iRet = RET_CODE_ERROR;
			goto done;
		}
		else
		{
			vtc_params.fps_num = (uint32_t)fps_num;
			vtc_params.fps_den = (uint32_t)fps_den;
		}
	}

	// check and parse SAR
	if (iter_sar != g_params.end())
	{
		int64_t sar_num = -1, sar_den = -1;
		if (ConvertToRationalNumber(iter_sar->second, sar_num, sar_den) == false ||
			sar_num < 0 || sar_num > UINT32_MAX ||
			sar_den < 0 || sar_den > UINT32_MAX)
		{
			printf("The 'SAR' parameter \"--SAR=%s\" can't be parsed.\n", iter_sar->second.c_str());
			iRet = RET_CODE_ERROR;
			goto done;
		}
		else
		{
			vtc_params.sar_num = (uint32_t)sar_num;
			vtc_params.sar_den = (uint32_t)sar_den;
		}
	}

	// Check and parse the bitrate
	if (iter_bitrate != g_params.end())
	{
		int64_t target_bitrate = -1;
		if (ConvertToInt(iter_bitrate->second, target_bitrate) == false || target_bitrate < 0 || target_bitrate > UINT32_MAX)
		{
			iRet = RET_CODE_ERROR_NOTIMPL;
			goto done;
		}

		vtc_params.bitrate = (uint32_t)target_bitrate;
	}

	if (iter_profile != g_params.end())
		vtc_export.fn_params_parse(&vtc_params, "profile", iter_profile->second.c_str());

	if (iter_tier != g_params.end())
		vtc_export.fn_params_parse(&vtc_params, "tier", iter_tier->second.c_str());

	if (iter_level != g_params.end())
		vtc_export.fn_params_parse(&vtc_params, "level", iter_level->second.c_str());

	iRet = RET_CODE_SUCCESS;

done:
	vtc_export.fn_params_cleanup(&vtc_params);
	return iRet;
}

int TranscodeFromESToES()
{
	FILE* rfp = NULL;
	int iRet = RET_CODE_SUCCESS;
	int64_t file_size = 0;
	uint8_t pBuf[2048] = { 0 };
	IMSEParser* pMediaParser = nullptr;
	IUnknown* pCtx = nullptr;
	VTC_EXPORT vtc_export;

	auto iter_dstfmt = g_params.find("outputfmt");
	auto iter_bitrate = g_params.find("bitrate");
	if (iter_bitrate == g_params.end())
		return RET_CODE_ERROR_NOTIMPL;

	int64_t target_bitrate = -1;
	if (ConvertToInt(iter_bitrate->second, target_bitrate) == false)
		return RET_CODE_ERROR_NOTIMPL;

	const int read_unit_size = 2048;

	auto iter_srcInput = g_params.find("input");
	if (iter_srcInput == g_params.end())
	{
		printf("Please specify an input ES file to transcode.\n");
		return RET_CODE_ERROR_NOTIMPL;
	}

	if (AMP_FAILED(iRet = CreateMSEParser(&pMediaParser)))
	{
		printf("Failed to create the media syntax element parser {error: %d}\n", iRet);
		return RET_CODE_ERROR_NOTIMPL;
	}

	int enum_cmd_option = MSE_ENUM_AU_RANGE;
	MSENav mse_nav(pMediaParser->GetSchemeType());
	if (AMP_FAILED(iRet = mse_nav.Load(enum_cmd_option)))
	{
		printf("Failed to load the Media Syntax Element URI.\n");
		return iRet;
	}

	errno_t errn = fopen_s(&rfp, iter_srcInput->second.c_str(), "rb");
	if (errn != 0 || rfp == NULL)
	{
		printf("Failed to open the file: %s {errno: %d}.\n", iter_srcInput->second.c_str(), errn);
		goto done;
	}

	if (AMP_FAILED(iRet = pMediaParser->GetContext(&pCtx)))
	{
		printf("Failed to get the media element syntax element context {error: %d}\n", iRet);
		goto done;
	}

	if (AMP_FAILED(LoadVTCExport(vtc_export)))
	{
		printf("Failed to initialize the video transcoder.\n");
		goto done;
	}

	if (AMP_FAILED(iRet = BindAUTranscodeEnumerator(pMediaParser, pCtx, enum_cmd_option, mse_nav, rfp, vtc_export)))
	{
		printf("Failed to bind the media syntax element enumerator {error: %d}\n", iRet);
		goto done;
	}

	file_size = GetFileSizeByFP(rfp);

	PrintMSEHeader(pMediaParser, pCtx, enum_cmd_option, mse_nav, rfp);

	LocateMSEParseStartPosition(pCtx, rfp);

	do
	{
		int read_size = read_unit_size;
		if ((read_size = (int)fread(pBuf, 1, read_unit_size, rfp)) <= 0)
		{
			iRet = RET_CODE_IO_READ_ERROR;
			break;
		}

		iRet = pMediaParser->ProcessInput(pBuf, read_size);
		if (AMP_FAILED(iRet))
			break;

		iRet = pMediaParser->ProcessOutput();
		if (iRet == RET_CODE_ABORT)
			break;

	} while (!feof(rfp));

	if (feof(rfp))
		iRet = pMediaParser->ProcessOutput(true);

done:
	if (rfp != nullptr)
		fclose(rfp);

	if (pMediaParser)
	{
		pMediaParser->Release();
		pMediaParser = nullptr;
	}

	if (pCtx)
	{
		pCtx->Release();
		pCtx = nullptr;
	}

	UnloadVTCExport(vtc_export);

	return iRet;
}

