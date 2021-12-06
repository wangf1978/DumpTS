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
#ifndef __SYNCER_H__
#define __SYNCER_H__

#include "dump_data_type.h"
#include <chrono>

#ifdef _WIN32
#include <Windows.h>
#endif

enum CLOCK_VALUE_PRECISION
{
	CLOCK_VALUE_45KHZ		= (1<<0),	// BD navigation time-base
	CLOCK_VALUE_90KHZ		= (1<<1),	// PTS/DTS
	CLOCK_VALUE_27MHZ		= (1<<2),	// SCR/PCR/ATC
	CLOCK_VALUE_1HZ			= (1<<3),	// 1 second
	CLOCK_VALUE_1KHZ		= (1<<4),	// 1 millisecond
	CLOCK_VALUE_10MHZ		= (1<<5),	// 1 hns(100 nanoseconds)

	CLOCK_VALUE_30BIT		= (1<<8),	// 30-bits, for example, ATC arrive time defined in BD and TTS
	CLOCK_VALUE_32BIT		= (1<<9),	// 32-bits, for example, BDAV navigation database IN_time/OUT_time
	CLOCK_VALUE_33BIT		= (1<<10),	// 33-bits, for example, PTS/DTS in MPEG2-TS stream
	CLOCK_VALUE_48BIT_PCR	= (1<<11),	// 48-bits, 33-bits(base) + 6-bits(reserved) + 9-bits(extension), for example, PCR in MPEG-2 TS
	CLOCK_VALUE_64BIT		= (1<<12),

	CLOCK_MPEG2_TS_ATC = CLOCK_VALUE_27MHZ | CLOCK_VALUE_30BIT,
};

class STDClockSyncer
{
public:
	STDClockSyncer(int clock_precision = CLOCK_MPEG2_TS_ATC);
	~STDClockSyncer();

	int				Start(uint64_t init_clock_value);
	int				Pause(bool bOn);
	int				Stop();
	/*!	@brief Sync with the last sync point, the current thread may be blocked by this function. 
		@retval RET_CODE_TIME_OUT time-out happened, the caller may need check the state, and continue wait
		@retval RET_CODE_CLOCK_DISCONTINUITY Hit the discontinuity in the external clock from input stream
		@retval RET_CODE_SUCCESS sync with the previous sync point successfully, and can continue the next process
	*/
	int				Sync(uint64_t clock_value);

	int				SetClockDiscontinuityThreshold(uint64_t threshold_ms);

protected:
	uint64_t		GetDiffHns(uint64_t clock_val1, uint64_t clock_val2);
	uint64_t		AdvanceHns(uint64_t clock_val, int64_t hns);
	void			UpdateSyncPair(uint64_t clock_value);

protected:
	int				m_clock_precision;
	int				m_clock_state;
	uint64_t		m_init_clock_value;
	std::chrono::high_resolution_clock::time_point
					m_ref_clock_start;

	uint64_t		m_last_clock_value;
	std::chrono::high_resolution_clock::time_point
					m_last_ref_clock;
;
	uint64_t		m_clock_discontinuity_threshold_ms;
	
#ifdef _WIN32
	CRITICAL_SECTION
					m_csState;
	CONDITION_VARIABLE
					m_condVarState;
#else

#endif

};

#endif
