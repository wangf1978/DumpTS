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

extern std::map<std::string, std::string, CaseInsensitiveComparator> g_params;

extern MEDIA_SCHEME_TYPE			g_source_media_scheme_type;
extern MEDIA_SCHEME_TYPE			g_output_media_scheme_type;

int TranscodeFromESToES()
{
	int nDumpRet = 0;
	auto iter_dstfmt = g_params.find("outputfmt");
	auto iter_bitrate = g_params.find("bitrate");
	if (iter_bitrate == g_params.end())
		return RET_CODE_ERROR_NOTIMPL;

	int64_t target_bitrate = -1;
	if (ConvertToInt(iter_bitrate->second, target_bitrate) == false)
		return RET_CODE_ERROR_NOTIMPL;

	return nDumpRet;
}

