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
#ifndef __RANGE_LEX_H__
#define __RANGE_LEX_H__

#pragma once
#include "DataUtil.h"

enum RANGE_RESULT
{
	RANGE_PREFIX_MISMATCH = -3,
	RANGE_OUTOFRANGE = -2,
	RANGE_ERROR = -1,
	RANGE_OK = 0,
	RANGE_IGNORE,
};

/*
	Inclusive natural number ranges
	-------------------------------
	It is a utility parser for range:
		[~]*value-name[[[s][-][e]][,]]*
	For example,
		1-2			// means 1, 2
		-2			// means 0, 1, 2
		2-			// means 2, 3, 4, ....... INT64_MAX
		1,4-6,10	// means 1, 4, 5, 6, 10

	There may be multiple ranges to be returned
	1. a range have 2 points
	2. a range may be a normal range, or an exclusive range
	1. each range should NOT overlap
	2.
	  0               m       n
	  |---------------|.......|.........................
	  ________________/       \____________________
	EXCLUDE ALL != UNSELECTED

	retcode:
		0,  success
		-1, fatal error
		1   nothing to do, ignore
*/
struct INCLUSIVE_NN_VALUE_RANGES
{
	enum RANGE_POINT_CAT
	{
		RANGE_POINT_CAT_NUMBER = 0,
		RANGE_POINT_CAT_UNSELECTED = 1,
		RANGE_POINT_CAT_UNSPECIFIED = 2,
	};

	struct RANGE_POINT
	{
		RANGE_POINT_CAT		cat;
		int64_t				num;

		RANGE_POINT() : RANGE_POINT(RANGE_POINT_CAT_UNSELECTED, -1LL) {}
		RANGE_POINT(RANGE_POINT_CAT paramCAT, int64_t paramNum)
			: cat(paramCAT), num(paramNum) {}
	};

	struct VALUE_RANGE
	{
		/*
			// normal range
			0               m       n
			|---------------|.......|.........................
							\_______/
			// complementary range
			0               m       n
			|---------------|.......|.........................
			________________/       \____________________
		*/
		bool				complementary = false;	// false: normal range, like as [m, n]
													// true: out of the specified range, like as [0, m), (n, +infinite)
		RANGE_POINT			points[2];

		VALUE_RANGE() : VALUE_RANGE(RANGE_POINT_CAT_UNSELECTED, -1LL, RANGE_POINT_CAT_UNSELECTED, -1LL) {}
		VALUE_RANGE(RANGE_POINT_CAT paramCAT0, int64_t paramNum0, RANGE_POINT_CAT paramCAT1, int64_t paramNum1) :
			VALUE_RANGE(false, paramCAT0, paramNum0, paramCAT1, paramNum1) {}
		VALUE_RANGE(bool bComplementary, RANGE_POINT_CAT paramCAT0, int64_t paramNum0, RANGE_POINT_CAT paramCAT1, int64_t paramNum1)
			: complementary(bComplementary), points{ {paramCAT0, paramNum0}, {paramCAT1, paramNum1} }{}

		void Reset()
		{
			complementary = false;
			points[0].cat = RANGE_POINT_CAT_UNSELECTED;
			points[1].cat = RANGE_POINT_CAT_UNSELECTED;
		}

		bool IsNull() const {	// whole natural number set
			return (points[0].cat == RANGE_POINT_CAT_UNSELECTED) && (points[1].cat == RANGE_POINT_CAT_UNSELECTED);
		}

		bool IsNN() const {	// whole natural number set
			return (points[0].cat == RANGE_POINT_CAT_UNSPECIFIED) && (points[1].cat == RANGE_POINT_CAT_UNSPECIFIED);
		}

		bool IsExcludeAll() const {
			return complementary && (points[0].cat == RANGE_POINT_CAT_UNSPECIFIED) && (points[1].cat == RANGE_POINT_CAT_UNSPECIFIED);
		}

		bool GetSingleNum(int64_t* pnum = nullptr) const {
			if (complementary == false)
			{
				if ((points[0].cat == RANGE_POINT_CAT_NUMBER && points[1].cat == RANGE_POINT_CAT_UNSELECTED) ||
					(points[1].cat == RANGE_POINT_CAT_NUMBER && points[0].cat == RANGE_POINT_CAT_UNSELECTED))
				{
					int64_t num = points[0].cat == RANGE_POINT_CAT_NUMBER ? points[0].num : points[1].num;
					AMP_SAFEASSIGN(pnum, num);
					return true;
				}

				if (points[0].cat == RANGE_POINT_CAT_NUMBER && points[1].cat == RANGE_POINT_CAT_NUMBER)
				{
					if (points[0].num == points[1].num)
					{
						AMP_SAFEASSIGN(pnum, points[0].num);
						return true;
					}
				}
			}
			else if (complementary) {
				if (points[0].cat == RANGE_POINT_CAT_NUMBER && points[0].num == 1 && points[1].cat == RANGE_POINT_CAT_UNSPECIFIED)
				{
					AMP_SAFEASSIGN(pnum, 0);
					return true;
				}
			}

			return false;
		}

		bool IsNaR() const {	// check whether is a range or not
			if (points[0].cat == RANGE_POINT_CAT_NUMBER && points[1].cat == RANGE_POINT_CAT_NUMBER)
				if (points[0].num > points[1].num)
					return true;

			if (complementary) {
				if (!(points[0].cat == RANGE_POINT_CAT_NUMBER && points[1].cat == RANGE_POINT_CAT_UNSELECTED))
					return true;
			}
			else {
				if ((points[1].cat == RANGE_POINT_CAT_UNSELECTED && points[0].cat == RANGE_POINT_CAT_UNSPECIFIED) ||
					(points[0].cat == RANGE_POINT_CAT_UNSELECTED && points[1].cat == RANGE_POINT_CAT_UNSPECIFIED))
					return true;
			}
			return false;
		}

		bool Include(int64_t num) const
		{
			if (IsNaR() || IsExcludeAll() || IsNull())
				return false;

			if (IsNN())
				return true;

			if (complementary == false)
			{
				if (points[0].cat == RANGE_POINT_CAT_NUMBER) {
					if (points[1].cat == RANGE_POINT_CAT_UNSELECTED && points[0].num == num)
						return true;
					else if (points[1].cat == RANGE_POINT_CAT_NUMBER && num >= points[0].num && num <= points[1].num)
						return true;
					else if (points[1].cat == RANGE_POINT_CAT_UNSPECIFIED && num >= points[0].num)
						return true;
				}
				else if (points[1].cat == RANGE_POINT_CAT_NUMBER)
				{
					if (points[0].cat == RANGE_POINT_CAT_UNSELECTED && points[1].num == num)
						return true;
					else if (points[0].cat == RANGE_POINT_CAT_UNSPECIFIED && points[1].num >= num)
						return true;
				}
			}
			else
			{
				if (points[1].cat == RANGE_POINT_CAT_NUMBER && num > points[1].num)
					return true;

				if (points[0].cat == RANGE_POINT_CAT_NUMBER && num < points[0].num)
					return true;
			}

			return false;
		}

		bool IsOverlapped(const RANGE_POINT a[2], const RANGE_POINT b[2]) const
		{
			int64_t num_a[2], num_b[2];
			// a and b should be NOT a NaR
			if (a[0].cat == RANGE_POINT_CAT_UNSPECIFIED)
				num_a[0] = 0;
			else if (a[0].cat == RANGE_POINT_CAT_NUMBER)
				num_a[0] = a[0].num;
			else
				return false;

			if (a[1].cat == RANGE_POINT_CAT_UNSPECIFIED)
				num_a[1] = INT64_MAX;
			else if (a[1].cat == RANGE_POINT_CAT_NUMBER)
				num_a[1] = a[1].num;
			else
				return false;

			if (b[0].cat == RANGE_POINT_CAT_UNSPECIFIED)
				num_b[0] = 0;
			else if (b[0].cat == RANGE_POINT_CAT_NUMBER)
				num_b[0] = b[0].num;
			else
				return false;

			if (b[1].cat == RANGE_POINT_CAT_UNSPECIFIED)
				num_b[1] = INT64_MAX;
			else if (b[1].cat == RANGE_POINT_CAT_NUMBER)
				num_b[1] = b[1].num;
			else
				return false;

			int64_t min_start = AMP_MIN(num_a[0], num_b[0]);
			int64_t max_end = AMP_MAX(num_a[1], num_b[1]);

			if (max_end - min_start <= num_a[1] - num_a[0] + num_b[1] - num_b[0])
				return true;

			return false;
		}

		bool UnionRange(const RANGE_POINT a[2], const RANGE_POINT b[2], VALUE_RANGE& c)
		{
			int64_t num_a[2], num_b[2];
			// a and b should be NOT a NaR
			if (a[0].cat == RANGE_POINT_CAT_UNSPECIFIED)
				num_a[0] = 0;
			else if (a[0].cat == RANGE_POINT_CAT_NUMBER)
				num_a[0] = a[0].num;
			else
				return false;

			if (a[1].cat == RANGE_POINT_CAT_UNSPECIFIED)
				num_a[1] = INT64_MAX;
			else if (a[1].cat == RANGE_POINT_CAT_NUMBER)
				num_a[1] = a[1].num;
			else
				return false;

			if (b[0].cat == RANGE_POINT_CAT_UNSPECIFIED)
				num_b[0] = 0;
			else if (b[0].cat == RANGE_POINT_CAT_NUMBER)
				num_b[0] = b[0].num;
			else
				return false;

			if (b[1].cat == RANGE_POINT_CAT_UNSPECIFIED)
				num_b[1] = INT64_MAX;
			else if (b[1].cat == RANGE_POINT_CAT_NUMBER)
				num_b[1] = b[1].num;
			else
				return false;

			int64_t min_start = AMP_MIN(num_a[0], num_b[0]);
			int64_t max_end = AMP_MAX(num_a[1], num_b[1]);

			if (max_end - min_start <= num_a[1] - num_a[0] + num_b[1] - num_b[0])
			{
				c.complementary = false;
				c.points[0].cat = RANGE_POINT_CAT_NUMBER;
				c.points[0].num = min_start;
				c.points[1].cat = RANGE_POINT_CAT_NUMBER;
				c.points[1].num = max_end;
				return true;
			}

			return false;
		}

		bool UnionComplementaryRange(const RANGE_POINT a[2], const RANGE_POINT b[2], VALUE_RANGE& c)
		{
			int64_t num_a[2], num_b[2];
			// a and b should be NOT a NaR
			if (a[0].cat == RANGE_POINT_CAT_UNSPECIFIED)
				num_a[0] = 0;
			else if (a[0].cat == RANGE_POINT_CAT_NUMBER)
				num_a[0] = a[0].num;
			else
				return false;

			if (a[1].cat == RANGE_POINT_CAT_UNSPECIFIED)
				num_a[1] = INT64_MAX;
			else if (a[1].cat == RANGE_POINT_CAT_NUMBER)
				num_a[1] = a[1].num;
			else
				return false;

			if (b[0].cat == RANGE_POINT_CAT_UNSPECIFIED)
				num_b[0] = 0;
			else if (b[0].cat == RANGE_POINT_CAT_NUMBER)
				num_b[0] = b[0].num;
			else
				return false;

			if (b[1].cat == RANGE_POINT_CAT_UNSPECIFIED)
				num_b[1] = INT64_MAX;
			else if (b[1].cat == RANGE_POINT_CAT_NUMBER)
				num_b[1] = b[1].num;
			else
				return false;

			int64_t min_start = AMP_MIN(num_a[0], num_b[0]);
			int64_t max_end = AMP_MAX(num_a[1], num_b[1]);

			if (max_end - min_start <= num_a[1] - num_a[0] + num_b[1] - num_b[0])
			{
				c.complementary = true;
				c.points[0].cat = RANGE_POINT_CAT_NUMBER;
				c.points[0].num = AMP_MAX(num_a[0], num_b[0]);
				c.points[1].cat = RANGE_POINT_CAT_NUMBER;
				c.points[1].num = AMP_MIN(num_a[1], num_b[1]);;
				return true;
			}
			else if (min_start == 0 && max_end == INT64_MAX)
			{
				c.complementary = false;
				c.points[0].cat = RANGE_POINT_CAT_NUMBER;
				c.points[0].num = AMP_MIN(num_a[1], num_b[1]);
				c.points[1].cat = RANGE_POINT_CAT_NUMBER;
				c.points[1].num = AMP_MAX(num_a[0], num_b[0]);
			}

			return false;
		}

		bool UnionMixedRange(const RANGE_POINT a[2], const RANGE_POINT cb[2], VALUE_RANGE& c)
		{
			int64_t num_a[2], num_b[2];
			// a and b should be NOT a NaR
			if (a[0].cat == RANGE_POINT_CAT_UNSPECIFIED)
				num_a[0] = 0;
			else if (a[0].cat == RANGE_POINT_CAT_NUMBER)
				num_a[0] = a[0].num;
			else
				return false;

			if (a[1].cat == RANGE_POINT_CAT_UNSPECIFIED)
				num_a[1] = INT64_MAX;
			else if (a[1].cat == RANGE_POINT_CAT_NUMBER)
				num_a[1] = a[1].num;
			else
				return false;

			if (cb[0].cat == RANGE_POINT_CAT_UNSPECIFIED)
				num_b[0] = 0;
			else if (cb[0].cat == RANGE_POINT_CAT_NUMBER)
				num_b[0] = cb[0].num;
			else
				return false;

			if (cb[1].cat == RANGE_POINT_CAT_UNSPECIFIED)
				num_b[1] = INT64_MAX;
			else if (cb[1].cat == RANGE_POINT_CAT_NUMBER)
				num_b[1] = cb[1].num;
			else
				return false;

			int64_t min_start = AMP_MIN(num_a[0], num_b[0]);
			int64_t max_end = AMP_MAX(num_a[1], num_b[1]);

			/*
						  ________________________
						 /                        \
			-------------a0---b0----------b1------a1------------
			__________________/           \_____________________________
			NN scope
			*/
			if (min_start == num_a[0] && max_end == num_a[1])
			{
				// whole Natural Number Set
				c.complementary = false;
				c.points[0].cat = c.points[1].cat = RANGE_POINT_CAT_UNSPECIFIED;
				return true;
			}
			else if (min_start == num_b[0] && max_end == num_b[1])
			{
				return false;
			}

			/*
				 __________                          ________
				/          \                        /        \
			----a0---------a1---b0----------b1------a0------ a1------
			____________________/           \_____________________________
			*/
			if (num_a[1] <= num_b[0] || num_a[0] >= num_b[1])
			{
				c.complementary = true;
				c.points[0] = cb[0];
				c.points[1] = cb[1];
				return true;
			}
			/*
				 ____________________
				/                    \
			----a0--------------b0---a1-------b1------------ ------
			____________________/              \_____________________________
			*/
			else if (num_a[1] + 1 >= num_b[0] && num_a[1] + 1 <= num_b[1])
			{
				c.complementary = true;
				c.points[0].cat = RANGE_POINT_CAT_NUMBER;
				c.points[0].num = num_a[1] + 1;
				c.points[1] = cb[1];
			}
			/*
									 __________________
									/                  \
			------------------b0---a0-------b1---------a1--- ------
			____________________/           \_____________________________
			*/
			else if (num_a[0] >= num_b[0] + 1 && num_a[0] <= num_b[1] + 1)
			{
				c.complementary = true;
				c.points[0].cat = RANGE_POINT_CAT_NUMBER;
				c.points[0] = cb[0];
				c.points[1].num = num_a[0] - 1;
			}
			else
				printf("Should not hit here{%s()@%s: %d}\n", __FUNCTION__, __FILE__, __LINE__);

			return false;
		}

		bool Merge(const VALUE_RANGE& range)
		{
			if (IsNaR() || range.IsNaR())
				return false;

			if (IsNull())
			{
				if (range.IsNull())
					return true;
				else
					*this = range;
			}
			else if (range.IsNull())
			{
				return true;
			}

			if (IsNN() || range.IsNN())
			{
				complementary = false;
				points[0].cat = points[1].cat = RANGE_POINT_CAT_UNSELECTED;
				return true;
			}

			if (IsExcludeAll() || range.IsExcludeAll())
			{
				complementary = true;
				points[0].cat = points[1].cat = RANGE_POINT_CAT_UNSPECIFIED;
				return true;
			}

			int64_t num;
			if (range.GetSingleNum(&num))
				return Include(num);

			if (GetSingleNum(&num))
			{
				if (range.Include(num))
					*this = range;
				else
					return false;
			}

			// two ranges
			VALUE_RANGE ret_range;
			if (complementary == false && range.complementary == false)
			{
				if (UnionRange(points, range.points, ret_range) == false)
					return false;
			}
			else if (complementary && range.complementary)
			{
				if (UnionComplementaryRange(points, range.points, ret_range) == false)
					return false;
			}
			else
			{
				if (UnionMixedRange(complementary ? range.points : points, complementary ? points : range.points, ret_range) == false)
					return false;
			}

			*this = ret_range;

			return true;
		}
	};

	std::vector<VALUE_RANGE>	m_ranges;
	INT_VALUE_LITERAL_FORMAT	literal_fmt;

	INCLUSIVE_NN_VALUE_RANGES(INT_VALUE_LITERAL_FORMAT literalFMT = FMT_AUTO);

	RANGE_RESULT Parse(const char* szPrefix, const char* ps, const char* pe);

	RANGE_RESULT ParseRanges(const char* ps, const char* pe, std::vector<VALUE_RANGE> ranges);

	RANGE_RESULT ParseRange(const char* ps, const char* pe, VALUE_RANGE& range);
};

#endif
