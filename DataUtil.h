#pragma once
#include <stdio.h>
#include <string>
#include <math.h>
#include <limits>
#include <stdint.h>

enum INT_VALUE_LITERAL_FORMAT
{
	FMT_AUTO = 0,
	FMT_HEX,
	FMT_DEC,
	FMT_OCT,
	FMT_BIN
};

inline bool ConvertToInt(char* ps, char* pe, int64_t& ret_val, INT_VALUE_LITERAL_FORMAT literal_fmt = FMT_AUTO)
{
	ret_val = 0;
	bool bNegative = false;

	if (pe <= ps)
		return false;

	if (literal_fmt == FMT_AUTO)
	{
		if (pe > ps + 2 && _strnicmp(ps, "0x", 2) == 0)	// hex value
		{
			ps += 2;
			literal_fmt = FMT_HEX;
		}
		else if (*(pe - 1) == 'h' || *(pe - 1) == 'H')
		{
			pe--;
			literal_fmt = FMT_HEX;
		}
		else if (pe > ps + 2 && _strnicmp(ps, "0b", 2) == 0)
		{
			ps += 2;
			literal_fmt = FMT_BIN;
		}
		else if (*(pe - 1) == 'b' || *(pe - 1) == 'B')
		{
			// check whether all value from ps to pe-1, all are 0 and 1
			auto pc = ps;
			for (; pc < pe - 1; pc++)
				if (*pc != '0' && *pc != '1')
					break;

			if (pc >= pe - 1)
			{
				pe--;
				literal_fmt = FMT_BIN;
			}
		}
		else if (*ps == '0' || *ps == 'o' || *ps == 'O')
		{
			ps++;
			literal_fmt = FMT_OCT;
		}
		else if (*(pe - 1) == 'o' || *(pe - 1) == 'O')
		{
			pe--;
			literal_fmt = FMT_OCT;
		}

		if (literal_fmt == FMT_AUTO)
		{
			// still not decided
			auto pc = ps;
			for (; pc < pe; pc++)
			{
				if (*pc >= 'a' && *pc <= 'f' || *pc >= 'A' && *pc <= 'F')
					literal_fmt = FMT_HEX;
				else if (*pc >= '0' && *pc <= '9')
				{
					if (literal_fmt == FMT_AUTO)
						literal_fmt = FMT_DEC;
				}
				else
					return false;	// It is not an valid value
			}
		}
	}
	else if (literal_fmt == FMT_HEX)
	{
		if (pe > ps + 2 && _strnicmp(ps, "0x", 2) == 0)	// hex value
			ps += 2;
		else if (*(pe - 1) == 'h' || *(pe - 1) == 'H')
			pe--;
	}
	else if (literal_fmt == FMT_DEC)
	{
		if (*ps == '-')
		{
			bNegative = true;
			ps++;
		}
	}
	else if (literal_fmt == FMT_OCT)
	{
		if (*ps == '0' || *ps == 'o' || *ps == 'O')
			ps++;
		else if (*(pe - 1) == 'o' || *(pe - 1) == 'O')
			pe--;
	}
	else if (literal_fmt == FMT_BIN)
	{
		if (pe > ps + 2 && _strnicmp(ps, "0b", 2) == 0)	// binary value
			ps += 2;
		else if (*(pe - 1) == 'b' || *(pe - 1) == 'B')
			pe--;
	}

	if (literal_fmt != FMT_HEX && literal_fmt != FMT_DEC && literal_fmt != FMT_OCT && literal_fmt != FMT_BIN)
		return false;

	while (ps < pe)
	{
		if (literal_fmt == FMT_HEX)	// hex value
		{
			if (*ps >= '0' && *ps <= '9')
				ret_val = (ret_val << 4) | (*ps - '0');
			else if (*ps >= 'a' && *ps <= 'f')
				ret_val = (ret_val << 4) | (*ps - 'a' + 10);
			else if (*ps >= 'A' && *ps <= 'F')
				ret_val = (ret_val << 4) | (*ps - 'A' + 10);
			else
				return false;
		}
		else if (literal_fmt == FMT_OCT)	// octal value
		{
			if (*ps >= '0' && *ps <= '7')
				ret_val = (ret_val << 3) | (*ps - '0');
			else
				return false;
		}
		else if (literal_fmt == FMT_DEC)		// decimal value
		{
			if (*ps >= '0' && *ps <= '9')
				ret_val = (ret_val * 10) + (*ps - '0');
			else
				return false;
		}
		else if (literal_fmt == FMT_BIN)
		{
			if (*ps >= '0' && *ps <= '1')
				ret_val = (ret_val << 1) | (*ps - '0');
			else
				return false;
		}
		else
			return false;

		ps++;
	}

	if (bNegative)
		ret_val = -1LL * ret_val;

	return true;
}

inline int isLeapYear(int year) {
	return (year % 400 == 0) || ((year % 100 != 0) && (year % 4 == 0));
}

inline std::string DateTimeStr(uint64_t elapse_seconds_since_baseyear, int32_t base_year = 1904, uint64_t fraction_second=0)
{
	int year = base_year;
	constexpr uint64_t leap_year_ticks = 366ULL * 24 * 3600;
	constexpr uint64_t normal_year_ticks = 365ULL * 24 * 3600;
	uint64_t elapse_seconds = elapse_seconds_since_baseyear;
	while (elapse_seconds > 0)
	{
		uint64_t ticks = isLeapYear(year) ? leap_year_ticks : normal_year_ticks;
		if (elapse_seconds >= ticks)
		{
			elapse_seconds -= ticks;
			year++;
		}
		else
			break;
	}

	// Got year, now try to get month
	int month = 1;
	constexpr int month_ticks[12] = {
		31 * 24 * 3600, 28 * 24 * 3600, 31 * 24 * 3600, 30 * 24 * 3600, 31 * 24 * 3600, 30 * 24 * 3600,
		31 * 24 * 3600,	31 * 24 * 3600, 30 * 24 * 3600, 31 * 24 * 3600, 30 * 24 * 3600, 31 * 24 * 3600
	};

	int is_leap_year = isLeapYear(year);

	while (elapse_seconds > 0)
	{
		uint64_t ticks = (month == 2 && leap_year_ticks) ? 24 * 3600 : 0 + month_ticks[month - 1];
		if (elapse_seconds >= ticks)
		{
			elapse_seconds -= ticks;
			month++;
		}
		else
			break;
	}

	int day = 1 + (int)(elapse_seconds / (24 * 3600));
	elapse_seconds -= (day - 1) * 24 * 3600;

	int hour = (int)(elapse_seconds / 3600);
	int minute = (int)(elapse_seconds / 60 % 60);
	int second = (int)(elapse_seconds % 60);

	std::string strDateTime;
	// 1904-04-01 00h:00m:00s
	if (fraction_second == 0)
	{
		strDateTime.reserve(32);
		sprintf_s(&strDateTime[0], 32, "%04d-%02d-%02d %02dh:%02dm:%02ds", year, month, day, hour, minute, second);
	}
	else
	{
		strDateTime.reserve(64);
		sprintf_s(&strDateTime[0], 64, "%04d-%02d-%02d %02dh:%02dm:%02d.%llus", year, month, day, hour, minute, second, fraction_second);
	}
	return strDateTime;
}

inline uint64_t ConvertBigEndianUINT64(uint8_t bytes[8])
{
	uint64_t u64Val = 0;
	for (int i = 0; i < 8; i++)
		u64Val = (u64Val << 8) | bytes[i];
	return u64Val;
}

inline double ConvertExtentToDouble(uint8_t bytes[10])
{
	bool sign = (bytes[0] >> 7) & 0x1;
	uint16_t exp = (uint16_t)(((bytes[0] << 8) | bytes[1]) & 0x7FFF);
	bool integer_part = (bytes[2] >> 7) & 0x1;
	uint64_t m = ConvertBigEndianUINT64(&bytes[2]);
	uint64_t fraction = m&INT64_MAX;

	if (exp == 0)
	{
		if (integer_part == 0)
		{
			if (fraction == 0)
				return 0.;
			else
				return m*pow(2, -16382);
		}
		else
			return m*pow(2, -16382);
	}
	else if (exp == 0x7FFF)
	{
		if (((m >> 62) & 0x3) == 0)
		{
			if ((m & 0x3FFFFFFFFFFFFFFFULL) == 0)
				return std::numeric_limits<double>::infinity();
			else
				return std::numeric_limits<double>::quiet_NaN();
		}
		else if (((m >> 62) & 0x3) == 1)
		{
			return std::numeric_limits<double>::quiet_NaN();
		}
		else if (((m >> 62) & 0x3) == 2)
		{
			if ((m & 0x3FFFFFFFFFFFFFFFULL) == 0)
				return std::numeric_limits<double>::infinity();
			else
				return std::numeric_limits<double>::signaling_NaN();
		}

		return std::numeric_limits<double>::quiet_NaN();
	}

	double normalizeCorrection = (m & 0x3FFFFFFFFFFFFFFFULL) == 0 ? .0 : 1.;

	return (sign ? -1.0 : 1.0) * (normalizeCorrection + (double)fraction / ((1ULL << 63))) * pow(2, exp - 16383);
}
