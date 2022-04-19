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
#ifndef _MPEG_SYSTEM_DEF_H_
#define _MPEG_SYSTEM_DEF_H_

#define DUMP_RAW_OUTPUT									(1<<0)
#define DUMP_BD_M2TS									(1<<1)
#define DUMP_PES_OUTPUT									(1<<2)
#define DUMP_PTS_VIEW									(1<<3)
#define DUMP_PCM										(1<<4)		// Raw LPCM data
#define DUMP_WAV										(1<<5)		// MSFT wave file
#define DUMP_MEDIA_INFO_VIEW							(1<<6)

// For TS
#define DUMP_DTV_SIT									(1<<7)
#define DUMP_PMT										(1<<8)
#define DUMP_PAT										(1<<9)
#define DUMP_CAT										(1<<10)
#define DUMP_EIT										(1<<11)

// For all
#define DUMP_STREAM_INFO_VIEW							(1<<16)

// For NAL
#define DUMP_NAL_VPS									(1<<17)
#define DUMP_NAL_SPS									(1<<18)
#define DUMP_NAL_PPS									(1<<19)

// For Program Stream
#define DUMP_VOB										(1<<7)
#define DUMP_MPG										(1<<8)

// For MMT/TLV
#define DUMP_MPT										DUMP_PMT
#define DUMP_PLT										DUMP_PAT
#define DUMP_MH_EIT										DUMP_EIT

enum TS_FORMAT
{
	TSF_GENERIC				= 0x0,
	TSF_ATSC				= 0x800,
	TSF_DVB					= 0x1000,
	TSF_ISDB				= 0x2000,
	TSF_BD					= 0x3000,
	TSF_HDMV				= 0x3001,
	TSF_SESF				= 0x3002,
};

enum MPEG_SYSTEM_TYPE
{
	MPEG_SYSTEM_UNKNOWN		= -1,
	MPEG_SYSTEM_PS			= 0,		// MPEG-2 program stream
	MPEG_SYSTEM_TS			= 1,		// MPEG-2 (188-byte packet) transport stream
	MPEG_SYSTEM_TTS			= 2,		// 192-byte packet transport stream
	MPEG_SYSTEM_TS204		= 3,		// 204-byte packet transport stream
	MPEG_SYSTEM_TS208		= 4,		// 204-byte packet transport stream
	MPEG_SYSTEM_DVD_VIDEO	= 5,		// DVD-ROM VOB
	MPEG_SYSTEM_DVD_VR		= 6,		// DVD-VR VOB
	MPEG_SYSTEM_BDMV		= 7,		// BDMV transport stream
	MPEG_SYSTEM_BDAV		= 8,		// BDAV transport stream
	MPEG_SYSTEM_MP4			= 9,		// ISO MP4
	MPEG_SYSTEM_MATROSKA	= 10,		// Matroska based file-format
	MPEG_SYSTEM_MAX
};

// define the container type
enum MEDIA_SCHEME_TYPE
{
	MEDIA_SCHEME_UNKNOWN			= -1,
	MEDIA_SCHEME_TRANSPORT_STREAM	= 0,
	MEDIA_SCHEME_PROGRAM_STREAM,
	MEDIA_SCHEME_ISOBMFF,
	MEDIA_SCHEME_MATROSKA,
	MEDIA_SCHEME_MMT,
	MEDIA_SCHEME_AIFF,
	MEDIA_SCHEME_IVF,

	MEDIA_SCHEME_NAL				= 0x8000,
	MEDIA_SCHEME_ADTS,
	MEDIA_SCHEME_LOAS_LATM,
	MEDIA_SCHEME_AV1,
	MEDIA_SCHEME_MPV,
};

#define MPEG_SYSTEM_NAMEA(sys_type)	\
	((sys_type)==MPEG_SYSTEM_PS?"Program Stream":\
	((sys_type)==MPEG_SYSTEM_TS?"Transport Stream":\
	((sys_type)==MPEG_SYSTEM_TTS?"TTS":\
	((sys_type)==MPEG_SYSTEM_TS204?"Transport Stream(204 byte/packet)":\
	((sys_type)==MPEG_SYSTEM_TS208?"Transport Stream(208 byte/packet)":\
	((sys_type)==MPEG_SYSTEM_DVD_VIDEO?"DVD-Video":\
	((sys_type)==MPEG_SYSTEM_DVD_VR?"DVD-VR":\
	((sys_type)==MPEG_SYSTEM_BDMV?"BDMV":\
	((sys_type)==MPEG_SYSTEM_BDAV?"BDAV":\
	((sys_type)==MPEG_SYSTEM_MP4?"MP4":\
	((sys_type)==MPEG_SYSTEM_MATROSKA?"MKV":"Unknown")))))))))))

#define TS_FORMAT_NAMEA(ts_format)	\
	((ts_format)==TSF_ATSC?"ATSC":\
	((ts_format)==TSF_DVB?"DVG":\
	((ts_format)==TSF_ISDB?"ISDB":\
	((ts_format)==TSF_BD?"Blu-ray":\
	((ts_format)==TSF_HDMV?"HDMV":\
	((ts_format)==TSF_SESF?"BDAV-SESF":"Unknown"))))))

#endif
