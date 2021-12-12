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
#ifndef _H266_VIDEO_H_
#define _H266_VIDEO_H_

#include "sei.h"
#include <vector>
#include <unordered_map>

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4100)
#pragma warning(disable:4127)
#pragma warning(disable:4200)
#pragma warning(disable:4201)
#pragma pack(push,1)
#define PACKED
#else
#define PACKED __attribute__ ((__packed__))
#endif

namespace BST {

	namespace H266Video {

		enum NAL_UNIT_TYPE
		{
			TRAIL_NUT = 0,
			STSA_NUT = 1,
			RADL_NUT = 2,
			RASL_NUT = 3,
			RSV_VCL_4 = 4,
			RSV_VCL_5 = 5,
			RSV_VCL_6 = 6,
			IDR_W_RADL = 7,
			IDR_N_LP = 8,
			CRA_NUT = 9,
			GDR_NUT = 10,
			RSV_IRAP_11 = 11,
			OPI_NUT = 12,
			DCI_NUT = 13,
			VPS_NUT = 14,
			SPS_NUT = 15,
			PPS_NUT = 16,
			PREFIX_APS_NUT = 17,
			SUFFIX_APS_NUT = 18,
			PH_NUT = 19,
			AUD_NUT = 20,
			EOS_NUT = 21,
			EOB_NUT = 22,
			PREFIX_SEI_NUT = 23,
			SUFFIX_SEI_NUT = 24,
			FD_NUT = 25,
			RSV_NVCL_26 = 26,
			RSV_NVCL_27 = 27,
			UNSPEC_28 = 28,
			UNSPEC_29 = 29,
			UNSPEC_30 = 30,
			UNSPEC_31 = 31,
		};
	}
}

#ifdef _WIN32
#pragma pack(pop)
#pragma warning(pop)
#endif
#undef PACKED

#endif
