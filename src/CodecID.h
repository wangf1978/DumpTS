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
#pragma once

enum CODEC_ID
{
	CODEC_ID_UNKNOWN = 0,
	CODEC_ID_V_MPEG1 = 1,
	CODEC_ID_V_MPEG2,
	CODEC_ID_V_MPEG4,
	CODEC_ID_V_MPEG4_AVC = 0x10,
	CODEC_ID_V_MPEGH_HEVC,
	CODEC_ID_V_VVC,
	CODEC_ID_V_VC1 = 0x100,
	CODEC_ID_V_WMV,
	CODEC_ID_V_VP8 = 0x200,
	CODEC_ID_V_VP9,
	CODEC_ID_V_AV1,

	CODEC_ID_A_LPCM_BIG_ENDIAN = 0x80000000,
	CODEC_ID_A_LPCM_LITTLE_ENDIAN,
	CODEC_ID_A_LPCM_BLURAY,
	CODEC_ID_A_LPCM_FLOAT,

	CODEC_ID_A_AC3 = 0x80001000,
	CODEC_ID_A_DOLBY_DIGITAL_PLUS,
	CODEC_ID_A_DOLBY_TRUEHD,

	CODEC_ID_A_DTS = 0x80002000,
	CODEC_ID_A_DTSHD,
	CODEC_ID_A_DTSHD_XLL,

	CODEC_ID_A_MPEG_L3 = 0x80003000,
	CODEC_ID_A_MPEG_L2,
	CODEC_ID_A_MPEG_L1,
	CODEC_ID_A_MPEG2_AAC,
	CODEC_ID_A_MPEG4_AAC,

	CODEC_ID_A_VORBIS = 0x8000F000,
	CODEC_ID_A_FLAC,

	// Reserved for MMT/TLV container
	CODEC_ID_MMT_ASSET_STPP = 0xA0000000,	// Timed text (closed-caption and superimposition)
	CODEC_ID_MMT_ASSET_AAPP,				// Application
	CODEC_ID_MMT_ASSET_ASGD,				// Synchronous type general-purpose data
	CODEC_ID_MMT_ASSET_AAGD,				// Asynchronous type general-purpose data

};