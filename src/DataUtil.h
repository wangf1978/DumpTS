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

inline bool ConvertToInt(const char* ps, const char* pe, int64_t& ret_val, INT_VALUE_LITERAL_FORMAT literal_fmt = FMT_AUTO)
{
	ret_val = 0;
	bool bNegative = false;

	// trim the space at the beginning and ending
	while (ps < pe && (*ps == ' ' || *ps == '\t'))ps++;
	while (pe - 1 > ps && (*(pe - 1) == ' ' || *(pe - 1) == '\t'))pe--;

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
			// Check sign
			if (*ps == '-')
			{
				bNegative = true;
				ps++;
			}

			// still not decided
			auto pc = ps;
			for (; pc < pe; pc++)
			{
				if ((*pc >= 'a' && *pc <= 'f') || (*pc >= 'A' && *pc <= 'F'))
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
				ret_val = (ret_val << 4) | ((int64_t)*ps - '0');
			else if (*ps >= 'a' && *ps <= 'f')
				ret_val = (ret_val << 4) | ((int64_t)*ps - 'a' + 10);
			else if (*ps >= 'A' && *ps <= 'F')
				ret_val = (ret_val << 4) | ((int64_t)*ps - 'A' + 10);
			else
				return false;
		}
		else if (literal_fmt == FMT_OCT)	// octal value
		{
			if (*ps >= '0' && *ps <= '7')
				ret_val = (ret_val << 3) | ((int64_t)*ps - '0');
			else
				return false;
		}
		else if (literal_fmt == FMT_DEC)		// decimal value
		{
			if (*ps >= '0' && *ps <= '9')
				ret_val = (ret_val * 10) + ((int64_t)*ps - '0');
			else
				return false;
		}
		else if (literal_fmt == FMT_BIN)
		{
			if (*ps >= '0' && *ps <= '1')
				ret_val = (ret_val << 1) | ((int64_t)*ps - '0');
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

inline bool ConvertToInt(char* ps, char* pe, int64_t& ret_val, INT_VALUE_LITERAL_FORMAT literal_fmt = FMT_AUTO)
{
	return ConvertToInt((const char*)ps, (const char*)pe, ret_val, literal_fmt);
}

inline bool ConvertToInt(const std::string& str, int64_t& ret_val, INT_VALUE_LITERAL_FORMAT literal_fmt = FMT_AUTO)
{
	return ConvertToInt(str.c_str(), str.c_str() + str.length(), ret_val, literal_fmt);
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
		uint64_t ticks = (month == 2 && is_leap_year) ? 24 * 3600 : 0 + month_ticks[month - 1];
		if (elapse_seconds >= ticks)
		{
			elapse_seconds -= ticks;
			month++;
		}
		else
			break;
	}

	int day = 1 + (int)(elapse_seconds / (24 * 3600));
	elapse_seconds -= ((uint64_t)day - 1) * 24 * 3600;

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
		char szFraction[16] = { 0 };
		int ccWritten = sprintf_s(szFraction, 16, "%.6f", (double)((uint32_t)fraction_second) / 0x100000000LL);

		strDateTime.reserve(64);
		sprintf_s(&strDateTime[0], 64, "%04d-%02d-%02d %02dh:%02dm:%02d%ss", year, month, day, hour, minute, second, strchr(szFraction, '.'));
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

inline std::string GetReadableNum(uint64_t n)
{
	std::string strRet;
	char szTmp[256] = { 0 };
	int ccWritten = MBCSPRINTF_S(szTmp, sizeof(szTmp), "%" PRIu64 "", n);
	if (ccWritten > 0)
	{
		int nGroup = (ccWritten + 2) / 3;
		int nReminder = ccWritten % 3;
		
		strRet.reserve((size_t)ccWritten + ((size_t)nGroup - 1) * 3 + 1);
		const char* p = szTmp;
		for (int i = 0; i < nGroup; i++)
		{
			if (nGroup > 1 && i > 0)
				strRet.append(1, ',');

			for (int c = 0; c < (i > 0 || nReminder == 0 ? 3 : nReminder); c++)
				strRet.append(1, *p++);
		}
	}

	return strRet;
}

inline std::string GetFixedWidthStrWithEllipsis(const char* szStr, size_t max_width=80)
{
	size_t sz = strlen(szStr);
	if (sz + 3 <= max_width)
		return szStr;

	std::string strRet;
	strRet.reserve(max_width + 1);
	strRet.append(szStr, max_width - 3);
	strRet.append("...");
	return strRet;
}

inline bool splitstr(const char* s, const char* e, const char* delimiters, std::vector<std::string>& substrs)
{
	if (s == nullptr || delimiters == nullptr || (s >= e && e))
		return false;

	size_t dlen = strlen(delimiters);
	if (dlen == 0)
	{
		substrs.emplace_back(s, e);
		return true;
	}

	size_t i = 0;
	const char* p = s;
	const char* ps = s;
	while ((e && p != e) || (!e && *p != '\0'))
	{
		for (i = 0; i < dlen; i++)
		{
			if (*p == delimiters[i])
			{
				substrs.emplace_back(ps, p - ps);
				ps = p + 1;
				break;
			}
		}

		p++;
	}

	if (p > ps)
		substrs.emplace_back(ps, p - ps);

	return true;
}

inline bool splitstr(const char* szStr, const char* delimiters, std::vector<std::string>& substrs)
{
	return splitstr(szStr, NULL, delimiters, substrs);
}

inline std::string GetHumanReadNumber(uint64_t n, bool base10241K = false, uint8_t fraction_keep = 2, uint8_t integer_keep = 0, bool fixed_width=false)
{
	uint32_t base = 1000;
	if (base10241K)
		base = 1024;

	uint64_t fraction_scaler = 1;
	for (uint8_t i = 0; i < fraction_keep; i++)
		fraction_scaler *= 10ULL;

	uint64_t decimal = 0, fraction = 0;
	const char* unit_names[2][4] = { { "G", "M", "K", "" }, { "Gi", "Mi", "Ki", "" } };
	const char* unit_sel = "";
	if (n >= (uint64_t)base * base*base)
	{
		decimal = n / ((uint64_t)base*base*base);
		fraction = n * fraction_scaler / ((uint64_t)base*base*base) % fraction_scaler;
		unit_sel = unit_names[base10241K ? 1 : 0][0];
	}
	else if (n >= (uint64_t)base * base)
	{
		decimal = n / ((uint64_t)base*base);
		fraction = n * fraction_scaler / ((uint64_t)base*base) % fraction_scaler;
		unit_sel = unit_names[base10241K ? 1 : 0][1];
	}
	else if (n >= base)
	{
		decimal = n / base;
		fraction = n * fraction_scaler / base % fraction_scaler;
		unit_sel = unit_names[base10241K ? 1 : 0][2];
	}
	else
	{
		decimal = n;
		fraction = 0;
	}

	int ccWritten = 0, ccWrittenOnce = 0;
	char szOutput[256] = { 0 };
	if (integer_keep == 0)
		ccWrittenOnce = MBCSPRINTF_S(szOutput + ccWritten, sizeof(szOutput) / sizeof(szOutput[0]) - ccWritten, "%" PRIu64 "", decimal);
	else
	{
		char szFormatStr[64] = { 0 };
		MBCSPRINTF_S(szFormatStr, 64, "%%%d" PRIu64 "", integer_keep);
		ccWrittenOnce = MBCSPRINTF_S(szOutput + ccWritten, sizeof(szOutput) / sizeof(szOutput[0]) - ccWritten, szFormatStr, decimal);
	}

	if (ccWrittenOnce > 0)
		ccWritten += ccWrittenOnce;

	if (fixed_width || fraction > 0)
	{
		char szFormatStr[64] = { 0 };
		MBCSPRINTF_S(szFormatStr, 64, ".%%0%d" PRIu64 "", fraction_keep);
		ccWrittenOnce = MBCSPRINTF_S(szOutput + ccWritten, sizeof(szOutput) / sizeof(szOutput[0]) - ccWritten, szFormatStr, fraction);
		if (ccWrittenOnce > 0)
			ccWritten += ccWrittenOnce;
	}

	if (unit_sel[0] != '\0')
		ccWrittenOnce = MBCSPRINTF_S(szOutput + ccWritten, sizeof(szOutput) / sizeof(szOutput[0]) - ccWritten, "%s", unit_sel);

	return szOutput;
}

inline uint64_t gcd(uint64_t a, uint64_t b)
{
	if (b == 0)
		return 0;

	uint64_t c = a % b;
	if (c == 0)
		return b;
	else
		return gcd(b, c);
}

inline bool StrBeginWith(const char* str1, const char* str2, bool bIgnoreCase, bool* pbIsEqual, const char** str1_left)
{
	size_t szLen = strlen(str2), i = 0;
	for (; i < szLen && str1[i] != 0 && (bIgnoreCase ? (toupper(str2[i]) == toupper(str1[i])) : (str2[i] == str1[i])); i++);

	if (i == szLen)
	{
		AMP_SAFEASSIGN(pbIsEqual, str1[i] == _T('\0'));
		AMP_SAFEASSIGN(str1_left, &str1[i]);
		return TRUE;
	}

	return FALSE;
}

inline size_t StrEndWith(const char* str1, const char* str2, bool bIgnoreCase)
{
	size_t szLen = strlen(str1);
	size_t szLen2 = strlen(str2);

	if (szLen < szLen2)
		return std::string::npos;

	size_t i = szLen - strlen(str2), j = 0, end_pos = i;
	for (; i >= 0 && i < szLen && (bIgnoreCase ? (toupper(str2[j]) == toupper(str1[i])) : (str2[j] == str1[i])); i++, j++);

	return i == szLen ? end_pos : std::string::npos;
}

inline bool ConvertToInclusiveNNRange(const char* ps, const char* pe, int64_t& ret_val_start, int64_t& ret_val_end, int64_t max_unspecified = INT64_MAX, INT_VALUE_LITERAL_FORMAT literal_fmt = FMT_AUTO)
{
	// trim the space at the beginning and ending
	while (ps < pe && (*ps == ' ' || *ps == '\t'))ps++;
	while (pe - 1 > ps && (*(pe - 1) == ' ' || *(pe - 1) == '\t'))pe--;

	if (ps >= pe)
	{
		ret_val_start = 0;
		ret_val_end = max_unspecified;
		return true;
	}

	const char* p = ps;
	while (p < pe)
	{
		if (*p == '-')
			break;

		p++;
	}

	if (p >= pe)
	{
		if (ConvertToInt(ps, pe, ret_val_start, literal_fmt) == false)
			return false;

		ret_val_end = ret_val_start;
		return true;
	}

	if (p == ps)
		ret_val_start = 0;
	else if (p > ps)
	{
		if (ConvertToInt(ps, p, ret_val_start, literal_fmt) == false)
			return false;
	}
	else
		return false;

	if (p + 1 == pe)
		ret_val_end = max_unspecified;
	else if (p + 1 < pe)
	{
		if (ConvertToInt(p + 1, pe, ret_val_end, literal_fmt) == false)
			return false;
	}
	else
		return false;

	return true;
}

inline bool ConvertToRationalNumber(const char* s, const char* e, int64_t& num, int64_t& den)
{
	std::vector<std::string> substrs;
	if (splitstr(s, e, "/:", substrs) == false)
		return false;

	int64_t pick_num[2] = { 1, 1 };
	if (ConvertToInt(substrs[0], pick_num[0]) == false) {
		return false;
	}

	if (substrs.size() > 1) {
		if (ConvertToInt(substrs[1], pick_num[1]) == false) {
			return false;
		}
	}

	num = pick_num[0];
	den = pick_num[1];

	return true;
}

inline bool ConvertToRationalNumber(const char* s, int64_t& num, int64_t& den)
{
	return ConvertToRationalNumber(s, nullptr, num, den);
}

inline bool ConvertToRationalNumber(char* s, int64_t& num, int64_t& den)
{
	return ConvertToRationalNumber((const char*)s, nullptr, num, den);
}

inline bool ConvertToRationalNumber(const std::string& str, int64_t& num, int64_t& den)
{
	return ConvertToRationalNumber(str.c_str(), str.c_str() + str.length(), num, den);
}

