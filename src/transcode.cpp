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

#ifdef _WIN32
#define VTC_MODULE		"VideoTranscoder.dll"
#define VE_MODULE		"TranscodeEngine.dll"
#else
#define VTC_MODULE		"libvtc.so"
#define VE_MODULE		"libve.so"
#endif

extern std::map<std::string, std::string, CaseInsensitiveComparator> g_params;

extern MEDIA_SCHEME_TYPE			g_source_media_scheme_type;
extern MEDIA_SCHEME_TYPE			g_output_media_scheme_type;

extern int TranscodeFromESToES();

int LoadVTCExport(VTC_EXPORT& vtc_exports)
{
	vtc_exports.hModule = (void*)LoadLibrary(VTC_MODULE);
	if (vtc_exports.hModule == NULL) {
#ifdef _WIN32
		printf("Failed to load the share library '%s'{WIN32 error: %lu}\n", VTC_MODULE, GetLastError());
#else
		printf("Failed to load the share library '%s' error: %s\n", VTC_MODULE, dlerror());
#endif
		return RET_CODE_ERROR;
	}

	vtc_exports.fn_params_init		= (PFUNC_VTC_PARAMS_INIT)GetProcAddress((HMODULE)vtc_exports.hModule, "vtc_params_init");
	vtc_exports.fn_params_parse		= (PFUNC_VTC_PARAMS_PARSE)GetProcAddress((HMODULE)vtc_exports.hModule, "vtc_params_parse");
	vtc_exports.fn_params_clone		= (PFUNC_VTC_PARAMS_CLONE)GetProcAddress((HMODULE)vtc_exports.hModule, "vtc_params_clone");
	vtc_exports.fn_param_autoselect_profile_tier_level
									= (PFUNC_VTC_PARAMS_AUTOSELECT_PROFILE_TIER_LEVEL)GetProcAddress((HMODULE)vtc_exports.hModule, "vtc_params_autoselect_profile_tier_level");
	vtc_exports.fn_params_cleanup	= (PFUNC_VTC_PARAMS_CLEANUP)GetProcAddress((HMODULE)vtc_exports.hModule, "vtc_params_cleanup");
	vtc_exports.fn_picture_es_init	= (PFUNC_VTC_PICTURE_ES_INIT)GetProcAddress((HMODULE)vtc_exports.hModule, "vtc_picture_es_init");
	vtc_exports.fn_picture_es_cleanup 
									= (PFUNC_VTC_PICTURE_ES_CLEANUP)GetProcAddress((HMODULE)vtc_exports.hModule, "vtc_picture_es_cleanup");
	vtc_exports.fn_open				= (PFUNC_VTC_OPEN)GetProcAddress((HMODULE)vtc_exports.hModule, "video_transcoder_open");
	vtc_exports.fn_input			= (PFUNC_VTC_INPUT)GetProcAddress((HMODULE)vtc_exports.hModule, "video_transcoder_input");
	vtc_exports.fn_output			= (PFUNC_VTC_OUTPUT)GetProcAddress((HMODULE)vtc_exports.hModule, "video_transcoder_output");
	vtc_exports.fn_get_state		= (PFUNC_VTC_GET_STATE)GetProcAddress((HMODULE)vtc_exports.hModule, "video_transcoder_get_state");
	vtc_exports.fn_get_halt_error	= (PFUNC_VTC_GET_HALT_ERROR)GetProcAddress((HMODULE)vtc_exports.hModule, "video_transcoder_get_halt_error");
	vtc_exports.fn_close			= (PFUNC_VTC_CLOSE)GetProcAddress((HMODULE)vtc_exports.hModule, "video_transcoder_close");

	return RET_CODE_SUCCESS;
}

int UnloadVTCExport(VTC_EXPORT& vtc_exports)
{
	if (vtc_exports.hModule != NULL)
	{
		int ret = (int)FreeLibrary((HMODULE)vtc_exports.hModule);
#ifdef _WIN32
		if (ret == FALSE)
			printf("Failed to free the library '%s'{WIN32 error: %lu}\n", VTC_MODULE, GetLastError());
#else
		if (ret != 0)
			printf("Failed to free the library '%s'{error: %s}\n", VTC_MODULE, dlerror());
#endif
	}
	memset(&vtc_exports, 0, sizeof(vtc_exports));
	return RET_CODE_SUCCESS;
}

int Transcode()
{
	int nDumpRet = 0;
	auto iter_bitrate = g_params.find("bitrate");
	if (iter_bitrate == g_params.end())
		return RET_CODE_ERROR_NOTIMPL;

	int64_t target_bitrate = -1;
	if (ConvertToInt(iter_bitrate->second, target_bitrate) == false)
		return RET_CODE_ERROR_NOTIMPL;

	// From ES stream to ES stream
	bool bSrcIsES = false, bDstIsES = false;
	if (g_source_media_scheme_type == MEDIA_SCHEME_NAL
		|| g_source_media_scheme_type == MEDIA_SCHEME_AV1
		|| g_source_media_scheme_type == MEDIA_SCHEME_MPV)
		bSrcIsES = true;

	if (g_output_media_scheme_type == MEDIA_SCHEME_NAL
		|| g_output_media_scheme_type == MEDIA_SCHEME_AV1
		|| g_output_media_scheme_type == MEDIA_SCHEME_MPV)
		bDstIsES = true;

	if (bSrcIsES && bDstIsES)
	{
		nDumpRet = TranscodeFromESToES();
	}
	else
	{
		printf("Oops! Not implement yet for this case.\n");
		nDumpRet = RET_CODE_ERROR_NOTIMPL;
	}

	return nDumpRet;
}

