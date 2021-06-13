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