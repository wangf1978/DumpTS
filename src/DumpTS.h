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

