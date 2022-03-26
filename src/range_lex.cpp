/*

MIT License

Copyright (c) 2022 Ravin.Wang(wangf1978@hotmail.com)

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
#include "range_lex.h"

INCLUSIVE_NN_VALUE_RANGES::INCLUSIVE_NN_VALUE_RANGES(INT_VALUE_LITERAL_FORMAT literalFMT) : literal_fmt(literalFMT) {
}

RANGE_RESULT INCLUSIVE_NN_VALUE_RANGES::Parse(const char* szPrefix, const char* ps, const char* pe)
{
	if (ps == nullptr || pe == nullptr || ps >= pe)
		return RANGE_ERROR;

	// trim the space at the beginning and ending
	while (ps < pe && (*ps == ' ' || *ps == '\t'))ps++;
	while (pe - 1 > ps && (*(pe - 1) == ' ' || *(pe - 1) == '\t'))pe--;

	if (ps >= pe)
		return RANGE_ERROR;

	// Parse the '~'exclusion prefix part of syntax-element filter
	bool bExclude = false;
	size_t ccSeg = (size_t)(pe - ps);
	while (ccSeg > 0 && *ps == '~')
	{
		bExclude = !bExclude;
		ccSeg--;
		ps++;
	}

	size_t ccPrefix = szPrefix ? strlen(szPrefix) : 0;
	if (ps >= pe)
	{
		if (ccPrefix > 0)
			return RANGE_ERROR;

		m_ranges.clear();
		m_ranges.emplace_back(bExclude, RANGE_POINT_CAT_UNSPECIFIED, -1LL, RANGE_POINT_CAT_UNSPECIFIED, -1LL);
		return RANGE_OK;
	}

	// continue trimming the prefix space
	while (ps < pe && (*ps == ' ' || *ps == '\t'))ps++;

	if (ccPrefix > 0 && MBCSNICMP(ps, szPrefix, ccPrefix) != 0)
		return RANGE_PREFIX_MISMATCH;

	ps += ccPrefix;
	if (ps >= pe)
	{
		m_ranges.clear();
		m_ranges.emplace_back(bExclude, RANGE_POINT_CAT_UNSPECIFIED, -1LL, RANGE_POINT_CAT_UNSPECIFIED, -1LL);
		return RANGE_OK;
	}

	std::vector<VALUE_RANGE> ranges;
	RANGE_RESULT ret = ParseRanges(ps, pe, ranges);
	if (ret < 0)
		return ret;

	if (ret != RANGE_OK)
	{
		m_ranges.clear();
		m_ranges.emplace_back(bExclude, RANGE_POINT_CAT_UNSPECIFIED, -1LL, RANGE_POINT_CAT_UNSPECIFIED, -1LL);
		return RANGE_OK;
	}

	// Normalize the ranges
	m_ranges.emplace_back(bExclude, RANGE_POINT_CAT_UNSPECIFIED, -1LL, RANGE_POINT_CAT_UNSPECIFIED, -1LL);

	// Merge one by one
	do
	{
		bool bAllMergeFailed = true;
		auto iter = ranges.begin();
		while (iter != ranges.end())
		{
			if (m_ranges.back().Merge(*iter))
			{
				bAllMergeFailed = false;
				auto iterDel = iter;
				iter++;
				ranges.erase(iterDel);
			}
		}

		if (bAllMergeFailed)
		{
			iter = ranges.begin();
			if (iter != ranges.end())
			{
				m_ranges.push_back(*iter);
				ranges.erase(iter);
			}
		}

	} while (!ranges.empty());

	return RANGE_OK;
}

RANGE_RESULT INCLUSIVE_NN_VALUE_RANGES::ParseRanges(const char* ps, const char* pe, std::vector<VALUE_RANGE> ranges)
{
	if (ps == nullptr || pe == nullptr || ps >= pe)
		return RANGE_IGNORE;

	RANGE_RESULT ret = RANGE_OK;
	VALUE_RANGE range;
	const char* s = ps;
	const char* p = ps;

	while (p < pe)
	{
		if (*p == ',')
		{
			ret = ParseRange(s, p, range);
			if (ret == RANGE_OK)
				ranges.push_back(range);
			else if (ret < 0)
				return ret;

			s = p + 1;
		}
		p++;
	}

	if (p > s)
	{
		ret = ParseRange(s, p, range);
		if (ret == RANGE_OK)
			ranges.push_back(range);
		else if (ret < 0)
			return ret;
	}

	return RANGE_OK;
}

RANGE_RESULT INCLUSIVE_NN_VALUE_RANGES::ParseRange(const char* ps, const char* pe, VALUE_RANGE& range)
{
	if (ps == nullptr || pe == nullptr || ps >= pe)
		return RANGE_IGNORE;

	const char* s = ps;
	const char* p = ps;
	while (p < pe)
	{
		if (*p == '-')
		{
			int64_t i64Val = -1LL;
			if (p > s)
			{
				if (ConvertToInt(s, p, i64Val, literal_fmt) == false)
					return RANGE_ERROR;
				else
				{
					range.points[0].cat = RANGE_POINT_CAT_NUMBER;
					range.points[0].num = i64Val;
				}
			}
			else
				range.points[0].cat = RANGE_POINT_CAT_UNSPECIFIED;

			if (pe > p + 1)
			{
				if (ConvertToInt(p + 1, pe, i64Val, literal_fmt) == false)
					return RANGE_ERROR;
				else
				{
					range.points[1].cat = RANGE_POINT_CAT_NUMBER;
					range.points[1].num = i64Val;
				}
			}
			else
				range.points[1].cat = RANGE_POINT_CAT_UNSPECIFIED;

			if (range.points[0].cat == RANGE_POINT_CAT_NUMBER && range.points[1].cat == RANGE_POINT_CAT_NUMBER)
			{
				if (range.points[0].num > range.points[1].num)
					return RANGE_OUTOFRANGE;
				else if (range.points[0].num == range.points[1].num)
					return RANGE_IGNORE;
			}

			break;
		}

		p++;
	}

	// cant' find '-'
	if (p >= pe)
	{
		// only a single number
		int64_t i64Val = -1LL;
		if (ConvertToInt(ps, pe, i64Val, literal_fmt) == false)
			return RANGE_ERROR;
		else
		{
			range.points[0].cat = RANGE_POINT_CAT_NUMBER;
			range.points[0].num = i64Val;
			range.points[1].cat = RANGE_POINT_CAT_UNSELECTED;
		}
	}

	return RANGE_OK;
}
