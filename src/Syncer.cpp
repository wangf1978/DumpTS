#include "StdAfx.h"
#include "Syncer.h"

#define CLOCK_SYNCER_STATE_INIT		0
#define CLOCK_SYNCER_STATE_START	1
#define CLOCK_SYNCER_STATE_PAUSE	2
#define CLOCK_SYNCER_STATE_STOP		3

#define CLOCK_SYNC_MIN_THRESHOLD	100000			// 10 ms
#define CLOCK_SYNC_MAX_THRESHOLD	20000000		// 2 seconds
#define CLOCK_DISCONTINUITY_THRESHOLD_DEFAULT	\
									50000000		// 5 seconds

STDClockSyncer::STDClockSyncer(int clock_precision)
	: m_clock_precision(clock_precision)
	, m_clock_state(CLOCK_SYNCER_STATE_INIT)
	, m_init_clock_value(0)
	, m_ref_clock_start(std::chrono::high_resolution_clock::now())
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

	uint64_t delta_clock_value = GetDiffHns(clock_value, m_last_clock_value);

	if ((uint64_t)delta_ref_clock >= delta_clock_value*100)
	{
		// It means that the clock value from data is slower than reference clock, and in another word
		// The data transfer rate is a little slower, need speed up, don't block it
		m_last_clock_value = clock_value;
		m_last_ref_clock += std::chrono::nanoseconds(delta_clock_value * 100);
		return RET_CODE_SUCCESS;
	}

	uint64_t delay_clock_ns = delta_clock_value * 100 - delta_ref_clock;
	if (delay_clock_ns <= CLOCK_SYNC_MIN_THRESHOLD)
	{
		// The delay is less than threshold value, ignore this minor delay at this time
		m_last_ref_clock = ref_now;
		m_last_clock_value = AdvanceHns(m_last_clock_value, delay_clock_ns / 100);
		return RET_CODE_SUCCESS;
	}

	if (delay_clock_ns >= m_clock_discontinuity_threshold_ms)
		return RET_CODE_CLOCK_DISCONTINUITY;

	if (delay_clock_ns > CLOCK_SYNC_MAX_THRESHOLD)
		delay_clock_ns = CLOCK_SYNC_MAX_THRESHOLD;

#ifdef _WIN32
	EnterCriticalSection(&m_csState);
	BOOL bRet = SleepConditionVariableCS(&m_condVarState, &m_csState, (DWORD)(delay_clock_ns/10000));

	if (m_clock_state != CLOCK_SYNCER_STATE_PAUSE &&
		m_clock_state != CLOCK_SYNCER_STATE_START)
	{
		LeaveCriticalSection(&m_csState);
		return RET_CODE_ERROR;
	}

	ref_now = std::chrono::high_resolution_clock::now();
	delta_ref_clock = std::chrono::duration_cast<std::chrono::milliseconds>(ref_now - m_last_ref_clock).count();
	if (delta_ref_clock < 0)
	{
		LeaveCriticalSection(&m_csState);
		printf("Fatal error happened, the reference clock is out of order!");
		return RET_CODE_ERROR;
	}

	// Still can't catch up with the clock from stream, need continue wait in the next round
	if (delta_clock_value > (uint64_t)delta_ref_clock + CLOCK_SYNC_MIN_THRESHOLD)
	{
		LeaveCriticalSection(&m_csState);
		return RET_CODE_TIME_OUT;
	}

	// sync perfectly, update the pair of data clock and reference time clock
	if (delta_clock_value * 100 >= delta_clock_value)
	{
		m_last_ref_clock = ref_now;
		uint64_t delay_clock_ns = delta_clock_value * 100 - delta_ref_clock;
		m_last_clock_value = AdvanceHns(m_last_clock_value, delay_clock_ns / 100);
	}
	else
	{
		uint64_t delay_clock_ns = delta_ref_clock - delta_clock_value * 100;
		m_last_clock_value = clock_value;
		m_last_ref_clock -= std::chrono::nanoseconds(delay_clock_ns);
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
				return (0x200000000ULL + clock_val2 - clock_val1) * 1000 / 9;
			else if (m_clock_precision&CLOCK_VALUE_32BIT)
				return (0x100000000ULL + clock_val2 - clock_val1) * 1000 / 9;
			else if (m_clock_precision&CLOCK_VALUE_30BIT)
				return (0x40000000ULL + clock_val2 - clock_val1) * 1000 / 9;
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
				return (0x200000000ULL + clock_val2 - clock_val1) * 10000 / 45;
			else if (m_clock_precision&CLOCK_VALUE_32BIT)
				return (0x100000000ULL + clock_val2 - clock_val1) * 10000 / 45;
			else if (m_clock_precision&CLOCK_VALUE_30BIT)
				return (0x40000000ULL + clock_val2 - clock_val1) * 10000 / 45;
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
				return (0x200000000LL + clock_val2 - clock_val1) * 10000000;
			else if (m_clock_precision&CLOCK_VALUE_32BIT)
				return (0x100000000LL + clock_val2 - clock_val1) * 10000000;
			else if (m_clock_precision&CLOCK_VALUE_30BIT)
				return (0x40000000LL + clock_val2 - clock_val1) * 10000000;
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
				return (0x200000000LL + clock_val2 - clock_val1) * 10000;
			else if (m_clock_precision&CLOCK_VALUE_32BIT)
				return (0x100000000LL + clock_val2 - clock_val1) * 10000;
			else if (m_clock_precision&CLOCK_VALUE_30BIT)
				return (0x40000000LL + clock_val2 - clock_val1) * 10000;
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
				return (0x200000000ULL + clock_val2 - clock_val1);
			else if (m_clock_precision&CLOCK_VALUE_32BIT)
				return (0x100000000ULL + clock_val2 - clock_val1);
			else if (m_clock_precision&CLOCK_VALUE_30BIT)
				return (0x40000000ULL + clock_val2 - clock_val1);
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
					base_diff = 0x200000000 + clock_val2_base - clock_val1;

				uint64_t diff = base_diff * 300;

				if (clock_val1_ext >= clock_val2_ext)
					diff += clock_val1_ext - clock_val2_ext;
				else
					diff += 300 + clock_val2_ext - clock_val1_ext;

				return diff * 10 / 27;
			}
		}
		else
		{
			if (clock_val1 == clock_val2)
				return 0;
			else if (clock_val1 > clock_val2)
				return (clock_val1 - clock_val2) / 27000;
			else
			{
				// process the wrap-around case
				if (m_clock_precision&CLOCK_VALUE_33BIT)
					return (0x200000000ULL + clock_val2 - clock_val1) * 10 / 27;
				else if (m_clock_precision&CLOCK_VALUE_32BIT)
					return (0x100000000ULL + clock_val2 - clock_val1) * 10 / 27;
				else if (m_clock_precision&CLOCK_VALUE_30BIT)
					return (0x40000000ULL + clock_val2 - clock_val1) * 10 / 27;
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
			return ((uint64_t)(clock_val + hns * 9 / 1000)) & 0x4FFFFFFFULL;
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
			return ((uint64_t)(clock_val + hns * 45 / 10000)) & 0x4FFFFFFFULL;
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
			return ((uint64_t)(clock_val + hns / 10000000)) & 0x4FFFFFFFULL;
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
			return ((uint64_t)(clock_val + hns / 10000)) & 0x4FFFFFFFULL;
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
			return ((uint64_t)(clock_val + hns)) & 0x4FFFFFFFULL;
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
				return ((uint64_t)(clock_val + hns * 27 / 10)) & 0x4FFFFFFFULL;
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

