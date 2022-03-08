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
#include <stdint.h>
#include <algorithm>
#include "AudChMap.h"

enum DUMP_STATE
{
	DUMP_STATE_INVALID = 0,
	DUMP_STATE_INITIALIZE,
	DUMP_STATE_RUNNING,
	DUMP_STATE_PAUSE,
	DUMP_STATE_STOP,
	DUMP_STATE_MAX
};

enum STREAM_CAT
{
	STREAM_CAT_VIDEO = 0,
	STREAM_CAT_AUDIO,
	STREAM_CAT_SUBTITLE,
	STREAM_CAT_UNKNOWN = 0xFF
};

struct DUMP_STATUS
{
	int32_t			completed;				// Dump process is finished or not, 1: finished, 0: not finished
	DUMP_STATE		state;					// Dump State
	uint64_t		num_of_packs;			// Total number of TS/PS packs
	uint64_t		cur_pack_idx;			// The current processing TS pack index

	uint64_t		num_of_processed_payloads;
											// number of payloads processed, including the failure dump or success dump
	uint64_t		num_of_dumped_payloads;	// number of payloads dumped successfully
};

union PES_FILTER_INFO
{
	struct
	{
		unsigned short	PID;
		int				stream_type;
		int				stream_id;
		int				stream_id_extension;
	} TS;

	struct
	{
		STREAM_CAT		Stream_CAT;
		int				StreamIndex;	// If Stream_CAT is video, and StreamIndex is 0, it means video stream#0
		int				stream_type;
	} VOB;
};

struct AUDIO_INFO
{
	uint32_t		sample_frequency;
	CH_MAPPING		channel_mapping;	// The channel assignment according to bit position defined in CHANNEL_MAP_LOC
	uint32_t		bits_per_sample;
	uint32_t		bitrate;			// bits/second

	AUDIO_INFO()
		: sample_frequency(0)
		, bits_per_sample(0)
		, bitrate(0) {
	}
};

struct VIDEO_INFO
{
	int32_t			profile;			// See H264_PRFOILE, or HEVC_PROFILE
	int32_t			tier;
	int32_t			level;
	uint8_t			transfer_characteristics;
										// HDR10, SDR
	uint8_t			colour_primaries;	// REC.601, BT.709 and BT.2020
	uint8_t			chroma_format_idc;	// 0: unspecified, 1: YUV 4:2:0, 2: 4:2:2 or 3: 4:4:4
	uint8_t			reserved;
	uint32_t		video_height;
	uint32_t		video_width;
	uint32_t		framerate_numerator;
	uint32_t		framerate_denominator;
	uint16_t		aspect_ratio_numerator;
	uint16_t		aspect_ratio_denominator;
	uint32_t		bitrate;			// bits/second
	uint32_t		max_bitrate;		// bits/second

	VIDEO_INFO()
		: profile(-1), tier(-1), level(-1)
		, transfer_characteristics(0), colour_primaries(0), chroma_format_idc(0)
		, reserved(0), video_height(0), video_width(0), framerate_numerator(0), framerate_denominator(0), aspect_ratio_numerator(0), aspect_ratio_denominator(0)
		, bitrate(0), max_bitrate(0){
	}
};

struct STREAM_INFO
{
	int32_t		stream_coding_type = -1;
	int32_t		stream_id = -1;
	int32_t		stream_id_extension = -1;

	union
	{
		AUDIO_INFO	audio_info;
		VIDEO_INFO	video_info;
		uint8_t		bytes[std::max(sizeof(AUDIO_INFO), sizeof(VIDEO_INFO))];
	};

	STREAM_INFO() {
		memset(bytes, 0, sizeof(bytes));
	}

	STREAM_INFO(int stm_type) : stream_coding_type(stm_type) {
		memset(bytes, 0, sizeof(bytes));
	}
};

