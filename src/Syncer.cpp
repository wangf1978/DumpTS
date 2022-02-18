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
#include "platcomm.h"
#include "Syncer.h"

#define CLOCK_SYNCER_STATE_INIT		0
#define CLOCK_SYNCER_STATE_START	1
#define CLOCK_SYNCER_STATE_PAUSE	2
#define CLOCK_SYNCER_STATE_STOP		3

#define NANOSECONDS_PER_MILLISECOND	1000000ULL
#define CLOCK_SYNC_MIN_THRESHOLD	10			// 10 ms
#define CLOCK_SYNC_MAX_THRESHOLD	2000		// 2 seconds
#define CLOCK_DISCONTINUITY_THRESHOLD_DEFAULT	\
									5000		// 5 seconds

STDClockSyncer::STDClockSyncer(int clock_precision)
	: m_clock_precision(clock_precision)
	, m_clock_state(CLOCK_SYNCER_STATE_INIT)
	, m_init_clock_value(0)
	, m_ref_clock_start(std::chrono::high_resolution_clock::now())
	, m_last_clock_value(0)
	, m_last_ref_clock(std::chrono::high_resolution_clock::now())
	, m_clock_discontinuity_threshold_ms(CLOCK_DISCONTINUITY_THRESHOLD_DEFAULT)
{
#ifdef _WIN32
	InitializeCriticalSection(&m_csState);
	InitializeConditionVariable(&m_condVarState);
#else

#endif
}

STDClockSyncer::~STDClockSyncer()
{
#ifdef _WIN32
	DeleteCriticalSection(&m_csState);
#endif
}

int STDClockSyncer::Start(uint64_t init_clock_value)
{
#ifdef _WIN32
	EnterCriticalSection(&m_csState);
#endif
	m_last_clock_value = m_init_clock_value = init_clock_value;
	m_last_ref_clock = m_ref_clock_start = std::chrono::high_resolution_clock::now();
	m_clock_state = CLOCK_SYNCER_STATE_START;
#ifdef _WIN32
	LeaveCriticalSection(&m_csState);
#endif
	return RET_CODE_SUCCESS;
}

int STDClockSyncer::Pause(bool bOn)
{
#ifdef _WIN32
	EnterCriticalSection(&m_csState);
#endif

	if (m_clock_state != CLOCK_SYNCER_STATE_PAUSE &&
		m_clock_state != CLOCK_SYNCER_STATE_START)
	{
#ifdef _WIN32
		LeaveCriticalSection(&m_csState);
#endif
		return RET_CODE_ERROR_STATE_TRANSITION;
	}

	if (bOn)
	{
		// pause the clock
		auto ref_now = std::chrono::high_resolution_clock::now();
		if (m_clock_state == CLOCK_SYNCER_STATE_START)
			m_last_clock_value += std::chrono::duration_cast<std::chrono::nanoseconds>(ref_now - m_last_ref_clock).count() * 27 / 1000;
		m_last_ref_clock = ref_now;
		m_clock_state = CLOCK_SYNCER_STATE_PAUSE;
	}
	else
	{
		// resume the clock
		m_last_ref_clock = std::chrono::high_resolution_clock::now();
		m_clock_state = CLOCK_SYNCER_STATE_START;
	}

#ifdef _WIN32
	WakeAllConditionVariable(&m_condVarState);

	LeaveCriticalSection(&m_csState);
#endif
	
	return RET_CODE_SUCCESS;
}

int STDClockSyncer::Stop()
{
#ifdef _WIN32
	EnterCriticalSection(&m_csState);
#endif
	m_clock_state = CLOCK_SYNCER_STATE_STOP;
#ifdef _WIN32
	WakeAllConditionVariable(&m_condVarState);
	LeaveCriticalSection(&m_csState);
#endif
	return RET_CODE_SUCCESS;
}

extern int g_debug_sync;
int STDClockSyncer::Sync(uint64_t clock_value)
{
	// Can't process sync operation under stop or init state
	if (m_clock_state != CLOCK_SYNCER_STATE_PAUSE &&
		m_clock_state != CLOCK_SYNCER_STATE_START)
		return RET_CODE_ERROR;

	auto ref_now = std::chrono::high_resolution_clock::now();

	int64_t delta_ref_clock = std::chrono::duration_cast<std::chrono::nanoseconds>(ref_now - m_last_ref_clock).count();
	if (delta_ref_clock < 0)
	{
		printf("Fatal error happened, the reference clock is out of order!");
		return RET_CODE_ERROR;
	}

	uint64_t delta_clock_value_hns = GetDiffHns(clock_value, m_last_clock_value);
	if (delta_clock_value_hns == UINT64_MAX)
	{
		printf("Hit an unexpected error when diff the data clock.\n");
		return RET_CODE_ERROR;
	}

	uint64_t delta_clock_value_ns = delta_clock_value_hns * 100;

	//printf("delta_ref_clock: %lld(%lld ms), delta_clock_value_ns: %llu (%llu ms)\n",
	//	delta_ref_clock, delta_ref_clock / 1000000, delta_clock_value_ns, delta_clock_value_ns / 1000000);

	if ((uint64_t)delta_ref_clock >= delta_clock_value_ns)
	{
		// It means that the clock value from data is slower than reference clock, and in another word
		// The data transfer rate is a little slower, need speed up, don't block it
		m_last_clock_value = clock_value;
		if (g_debug_sync)
			printf("m_last_clock_value is changed to %" PRIu64 " {%s(), %d}.\n", m_last_clock_value, __FUNCTION__, __LINE__);
		m_last_ref_clock += std::chrono::nanoseconds(delta_clock_value_ns);
		return RET_CODE_SUCCESS;
	}

	uint64_t delay_clock_ns = delta_clock_value_ns - delta_ref_clock;
	if (delay_clock_ns <= CLOCK_SYNC_MIN_THRESHOLD * NANOSECONDS_PER_MILLISECOND)
	{
		// The delay is less than threshold value, ignore this minor delay at this time
		if (g_debug_sync)
			printf("m_last_clock_value: %" PRIu64 " {delay_clock_ns: %" PRIu64 ", %s(), %d}.\n", m_last_clock_value, delay_clock_ns, __FUNCTION__, __LINE__);
		m_last_clock_value = AdvanceHns(m_last_clock_value, std::chrono::duration_cast<std::chrono::nanoseconds>(ref_now - m_last_ref_clock).count()/100);

		m_last_ref_clock = ref_now;

		if (g_debug_sync)
			printf("m_last_clock_value is changed to %" PRIu64 " {delay_clock_ns: %" PRIu64 ", %s(), %d}.\n", m_last_clock_value, delay_clock_ns, __FUNCTION__, __LINE__);

		return RET_CODE_SUCCESS;
	}

	if (delay_clock_ns >= m_clock_discontinuity_threshold_ms * NANOSECONDS_PER_MILLISECOND)
		return RET_CODE_CLOCK_DISCONTINUITY;

	if (delay_clock_ns > CLOCK_SYNC_MAX_THRESHOLD * NANOSECONDS_PER_MILLISECOND)
		delay_clock_ns = CLOCK_SYNC_MAX_THRESHOLD * NANOSECONDS_PER_MILLISECOND;

#ifdef _WIN32
	EnterCriticalSection(&m_csState);
	
	std::chrono::high_resolution_clock::time_point tp0 = std::chrono::high_resolution_clock::now();
	BOOL bRet = SleepConditionVariableCS(&m_condVarState, &m_csState, (DWORD)(delay_clock_ns/NANOSECONDS_PER_MILLISECOND));
	std::chrono::high_resolution_clock::time_point tp1 = std::chrono::high_resolution_clock::now();
#if 0
	printf("Try to wait for %lu (ms), actual wait: %lu (ms).\n", 
		(DWORD)(delay_clock_ns / NANOSECONDS_PER_MILLISECOND), 
		(DWORD)(std::chrono::duration_cast<std::chrono::milliseconds>(tp1 - tp0).count()));
#else
	printf(".");
#endif

	if (m_clock_state != CLOCK_SYNCER_STATE_PAUSE &&
		m_clock_state != CLOCK_SYNCER_STATE_START)
	{
		LeaveCriticalSection(&m_csState);
		return RET_CODE_ERROR;
	}

	ref_now = std::chrono::high_resolution_clock::now();
	delta_ref_clock = std::chrono::duration_cast<std::chrono::nanoseconds>(ref_now - m_last_ref_clock).count();
	if (delta_ref_clock < 0)
	{
		LeaveCriticalSection(&m_csState);
		printf("Fatal error happened, the reference clock is out of order!");
		return RET_CODE_ERROR;
	}

	// Still can't catch up with the clock from stream, need continue wait in the next round
	if (delta_clock_value_ns > (uint64_t)delta_ref_clock + CLOCK_SYNC_MIN_THRESHOLD * NANOSECONDS_PER_MILLISECOND)
	{
		LeaveCriticalSection(&m_csState);
		return RET_CODE_TIME_OUT;
	}

	// sync perfectly, update the pair of data clock and reference time clock
	if (delta_clock_value_ns >= (uint64_t)delta_ref_clock)
	{
		m_last_ref_clock = ref_now;
		m_last_clock_value = AdvanceHns(m_last_clock_value, delta_clock_value_hns);
		if (g_debug_sync)
			printf("m_last_clock_value is changed to %llu {%s(), %d}.\n", m_last_clock_value, __FUNCTION__, __LINE__);

	}
	else
	{
		m_last_clock_value = clock_value;
		if (g_debug_sync)
			printf("m_last_clock_value is changed to %llu {%s(), %d}.\n", m_last_clock_value, __FUNCTION__, __LINE__);

		m_last_ref_clock += std::chrono::nanoseconds(delta_clock_value_ns);
	}

	LeaveCriticalSection(&m_csState);

	return RET_CODE_SUCCESS;
#else
	return RET_CODE_ERROR_NOTIMPL;
#endif
}

int STDClockSyncer::SetClockDiscontinuityThreshold(uint64_t threshold_ms)
{
	m_clock_discontinuity_threshold_ms = threshold_ms;
	return RET_CODE_SUCCESS;
}

uint64_t STDClockSyncer::GetDiffHns(uint64_t clock_val1, uint64_t clock_val2)
{
	if (m_clock_precision&CLOCK_VALUE_90KHZ)
	{
		if (PTS_90K_EQ(clock_val1, clock_val2) == 0)
			return 0;
		else if (PTS_90K_GT(clock_val1, clock_val2))
			return (clock_val1 - clock_val2) / 90;
		else
		{
			// process the wrap-around case
			if (m_clock_precision&CLOCK_VALUE_33BIT)
				return (0x200000000ULL + clock_val1 - clock_val2) * 1000 / 9;
			else if (m_clock_precision&CLOCK_VALUE_32BIT)
				return (0x100000000ULL + clock_val1 - clock_val2) * 1000 / 9;
			else if (m_clock_precision&CLOCK_VALUE_30BIT)
				return (0x40000000ULL + clock_val1 - clock_val2) * 1000 / 9;
			else
				assert(0);
		}
	}
	else if (m_clock_precision&CLOCK_VALUE_45KHZ)
	{
		if (PTS_45K_EQ(clock_val1, clock_val2) == 0)
			return 0;
		else if (PTS_45K_GT(clock_val1, clock_val2))
			return (clock_val1 - clock_val2) / 45;
		else
		{
			// process the wrap-around case
			if (m_clock_precision&CLOCK_VALUE_33BIT)
				return (0x200000000ULL + clock_val1 - clock_val2) * 10000 / 45;
			else if (m_clock_precision&CLOCK_VALUE_32BIT)
				return (0x100000000ULL + clock_val1 - clock_val2) * 10000 / 45;
			else if (m_clock_precision&CLOCK_VALUE_30BIT)
				return (0x40000000ULL + clock_val1 - clock_val2) * 10000 / 45;
			else
				assert(0);
		}
	}
	else if (m_clock_precision&CLOCK_VALUE_1HZ)
	{
		if (clock_val1 == clock_val2)
			return 0;
		else if (clock_val1 > clock_val2)
			return (clock_val1 - clock_val2) * 1000;
		else
		{
			// process the wrap-around case
			if (m_clock_precision&CLOCK_VALUE_33BIT)
				return (0x200000000LL + clock_val1 - clock_val2) * 10000000;
			else if (m_clock_precision&CLOCK_VALUE_32BIT)
				return (0x100000000LL + clock_val1 - clock_val2) * 10000000;
			else if (m_clock_precision&CLOCK_VALUE_30BIT)
				return (0x40000000LL + clock_val1 - clock_val2) * 10000000;
			else
				assert(0);
		}
	}
	else if (m_clock_precision&CLOCK_VALUE_1KHZ)
	{
		if (clock_val1 == clock_val2)
			return 0;
		else if (clock_val1 > clock_val2)
			return (clock_val1 - clock_val2);
		else
		{
			// process the wrap-around case
			if (m_clock_precision&CLOCK_VALUE_33BIT)
				return (0x200000000LL + clock_val1 - clock_val2) * 10000;
			else if (m_clock_precision&CLOCK_VALUE_32BIT)
				return (0x100000000LL + clock_val1 - clock_val2) * 10000;
			else if (m_clock_precision&CLOCK_VALUE_30BIT)
				return (0x40000000LL + clock_val1 - clock_val2) * 10000;
			else
				assert(0);
		}
	}
	else if (m_clock_precision&CLOCK_VALUE_10MHZ)
	{
		if (clock_val1 == clock_val2)
			return 0;
		else if (clock_val1 > clock_val2)
			return (clock_val1 - clock_val2) / 10000;
		else
		{
			// process the wrap-around case
			if (m_clock_precision&CLOCK_VALUE_33BIT)
				return (0x200000000ULL + clock_val1 - clock_val2);
			else if (m_clock_precision&CLOCK_VALUE_32BIT)
				return (0x100000000ULL + clock_val1 - clock_val2);
			else if (m_clock_precision&CLOCK_VALUE_30BIT)
				return (0x40000000ULL + clock_val1 - clock_val2);
			else
				assert(0);
		}
	}
	else if (m_clock_precision&CLOCK_VALUE_27MHZ)
	{
		if (m_clock_precision&CLOCK_VALUE_48BIT_PCR)
		{
			uint64_t clock_val1_base = ((clock_val1 >> 15) & 0x1FFFFFFFFULL);
			uint16_t clock_val1_ext = (uint16_t)(clock_val1 & 0x1FF);
			uint64_t clock_val1_27MHZ = clock_val1_base * 300 + clock_val1_ext;
			uint64_t clock_val2_base = ((clock_val2 >> 15) & 0x1FFFFFFFFULL);
			uint16_t clock_val2_ext = (uint16_t)(clock_val2 & 0x1FF);
			uint64_t clock_val2_27MHZ = clock_val2_base * 300 + clock_val2_ext;

			if (clock_val1_27MHZ == clock_val2_27MHZ)
				return 0;
			else if (clock_val1_27MHZ > clock_val2_27MHZ)
				return (clock_val1_27MHZ - clock_val2_27MHZ) * 10 / 27;
			else
			{
				uint64_t base_diff = 0;
				if (clock_val1_base == clock_val2_base)
					base_diff = 0;
				else if (clock_val1_base > clock_val2_base)
					base_diff = clock_val1_base - clock_val2_base;
				else
					base_diff = 0x200000000ULL + clock_val1_base - clock_val2_base;

				uint64_t diff = base_diff * 300;

				if (clock_val1_ext >= clock_val2_ext)
					diff += clock_val1_ext - clock_val2_ext;
				else
					diff += 300ULL + clock_val1_ext - clock_val2_ext;

				return diff * 10 / 27;
			}
		}
		else
		{
			if (clock_val1 == clock_val2)
				return 0;
			else if (clock_val1 > clock_val2)
				return (clock_val1 - clock_val2) *10 / 27;
			else
			{
				// process the wrap-around case
				if (m_clock_precision&CLOCK_VALUE_33BIT)
					return (0x200000000ULL + clock_val1 - clock_val2) * 10 / 27;
				else if (m_clock_precision&CLOCK_VALUE_32BIT)
					return (0x100000000ULL + clock_val1 - clock_val2) * 10 / 27;
				else if (m_clock_precision&CLOCK_VALUE_30BIT)
					return (0x40000000ULL + clock_val1 - clock_val2) * 10 / 27;
				else
					assert(0);
			}
		}
	}

	return UINT64_MAX;
}

uint64_t STDClockSyncer::AdvanceHns(uint64_t clock_val, int64_t hns)
{
	if (m_clock_precision&CLOCK_VALUE_90KHZ)
	{
		if (m_clock_precision&CLOCK_VALUE_33BIT)
			return ((uint64_t)(clock_val + hns * 9 / 1000)) & 0x1FFFFFFFFULL;
		else if (m_clock_precision&CLOCK_VALUE_32BIT)
			return ((uint64_t)(clock_val + hns * 9 / 1000)) & 0xFFFFFFFFULL;
		else if (m_clock_precision&CLOCK_VALUE_30BIT)
			return ((uint64_t)(clock_val + hns * 9 / 1000)) & 0x3FFFFFFFULL;
		else if (m_clock_precision&CLOCK_VALUE_64BIT)
			return ((uint64_t)(clock_val + hns * 9 / 1000));
		else
			assert(0);
	}
	else if (m_clock_precision&CLOCK_VALUE_45KHZ)
	{
		if (m_clock_precision&CLOCK_VALUE_33BIT)
			return ((uint64_t)(clock_val + hns * 45 / 10000)) & 0x1FFFFFFFFULL;
		else if (m_clock_precision&CLOCK_VALUE_32BIT)
			return ((uint64_t)(clock_val + hns * 45 / 10000)) & 0xFFFFFFFFULL;
		else if (m_clock_precision&CLOCK_VALUE_30BIT)
			return ((uint64_t)(clock_val + hns * 45 / 10000)) & 0x3FFFFFFFULL;
		else if (m_clock_precision&CLOCK_VALUE_64BIT)
			return ((uint64_t)(clock_val + hns * 45 / 10000));
		else
			assert(0);
	}
	else if (m_clock_precision&CLOCK_VALUE_1HZ)
	{
		if (m_clock_precision&CLOCK_VALUE_33BIT)
			return ((uint64_t)(clock_val + hns / 10000000)) & 0x1FFFFFFFFULL;
		else if (m_clock_precision&CLOCK_VALUE_32BIT)
			return ((uint64_t)(clock_val + hns / 10000000)) & 0xFFFFFFFFULL;
		else if (m_clock_precision&CLOCK_VALUE_30BIT)
			return ((uint64_t)(clock_val + hns / 10000000)) & 0x3FFFFFFFULL;
		else if (m_clock_precision&CLOCK_VALUE_64BIT)
			return ((uint64_t)(clock_val + hns / 10000000));
		else
			assert(0);
	}
	else if (m_clock_precision&CLOCK_VALUE_1KHZ)
	{
		if (m_clock_precision&CLOCK_VALUE_33BIT)
			return ((uint64_t)(clock_val + hns / 10000)) & 0x1FFFFFFFFULL;
		else if (m_clock_precision&CLOCK_VALUE_32BIT)
			return ((uint64_t)(clock_val + hns / 10000)) & 0xFFFFFFFFULL;
		else if (m_clock_precision&CLOCK_VALUE_30BIT)
			return ((uint64_t)(clock_val + hns / 10000)) & 0x3FFFFFFFULL;
		else if (m_clock_precision&CLOCK_VALUE_64BIT)
			return ((uint64_t)(clock_val + hns / 10000));
		else
			assert(0);
	}
	else if (m_clock_precision&CLOCK_VALUE_10MHZ)
	{
		if (m_clock_precision&CLOCK_VALUE_33BIT)
			return ((uint64_t)(clock_val + hns)) & 0x1FFFFFFFFULL;
		else if (m_clock_precision&CLOCK_VALUE_32BIT)
			return ((uint64_t)(clock_val + hns)) & 0xFFFFFFFFULL;
		else if (m_clock_precision&CLOCK_VALUE_30BIT)
			return ((uint64_t)(clock_val + hns)) & 0x3FFFFFFFULL;
		else if (m_clock_precision&CLOCK_VALUE_64BIT)
			return ((uint64_t)(clock_val + hns));
		else
			assert(0);
	}
	else if (m_clock_precision&CLOCK_VALUE_27MHZ)
	{
		if (m_clock_precision&CLOCK_VALUE_48BIT_PCR)
		{
			uint64_t clock_val_base = ((clock_val >> 15) & 0x1FFFFFFFFULL);
			uint16_t clock_val_ext = (uint16_t)(clock_val & 0x1FF);
			uint64_t clock_val_27MHZ = clock_val_base * 300 + clock_val_ext;

			int64_t hns_to_27MHZ = hns * 27 / 10;

			if (hns_to_27MHZ >= 0 || clock_val_27MHZ > (uint64_t)(-hns_to_27MHZ))
			{
				clock_val_27MHZ += hns_to_27MHZ;
			}
			else
			{
				clock_val_27MHZ = 0x1FFFFFFFFULL * 300 + 300 + hns_to_27MHZ + clock_val_27MHZ;
			}

			return ((clock_val_27MHZ / 300) << 15) | (((clock_val >> 9) & 0x3F) << 9) | (clock_val_27MHZ % 300);
		}
		else
		{
			if (m_clock_precision&CLOCK_VALUE_33BIT)
				return ((uint64_t)(clock_val + hns * 27 / 10)) & 0x1FFFFFFFFULL;
			else if (m_clock_precision&CLOCK_VALUE_32BIT)
				return ((uint64_t)(clock_val + hns * 27 / 10)) & 0xFFFFFFFFULL;
			else if (m_clock_precision&CLOCK_VALUE_30BIT)
				return ((uint64_t)(clock_val + hns * 27 / 10)) & 0x3FFFFFFFULL;
			else if (m_clock_precision&CLOCK_VALUE_64BIT)
				return ((uint64_t)(clock_val + hns * 27 / 10));
			else
				assert(0);
		}
	}

	return UINT64_MAX;
}

void STDClockSyncer::UpdateSyncPair(uint64_t clock_value)
{
	
}

