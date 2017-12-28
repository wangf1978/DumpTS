#include "stdafx.h"
#include "Bitstream.h"
#include <exception>
#include <assert.h>

CBitstream::CBitstream(uint8_t* pBuf, size_t cbitSize)
{
	size_t cbSize = (cbitSize + 7) >> 3;

	cursor.start_offset = ((intptr_t)pBuf & 3);
	cursor.p = cursor.p_start = (uint8_t*)pBuf - cursor.start_offset;
	cursor.p_end = pBuf + cbSize;
	cursor.bits_left = (sizeof(CURBITS_TYPE) - cursor.start_offset) * 8;

	cursor.curbits = CURBITS_VALUE(cursor.p);

	if ((size_t)cursor.bits_left >= cbitSize)
	{
		/*
		|--------|--------|--------|--------|
		p          r
		           ^++++++ bits_left +++++++^
		           ^+++++nBits+++++++++^
		*/
		cursor.exclude_bits = (uint8_t)(cursor.bits_left - cbitSize);
	}
	else
	{
		/*
		|--------|--------|--------|--------|.......|--------|--------|--------|--------|
		p          r
		           ^++++++ bits_left +++++++^
		           ^+++++nBits++++++++++++++++++++++++++++++^
		*/
		int nLeftBits = cbitSize - cursor.bits_left;
		cursor.exclude_bits = (uint8_t)(((nLeftBits + 7) & 0xFFFFFFF8) - nLeftBits);
	}

	memset(&save_point, 0, sizeof(save_point));
}

int CBitstream::GetAllLeftBits()
{
	int bits_left = cursor.p_end <= cursor.p ? 0 : (((int)(cursor.p_end - cursor.p)) << 3);
	return bits_left <= cursor.exclude_bits ? 0
		: (bits_left < (sizeof(CURBITS_TYPE) << 3) ? (bits_left - cursor.exclude_bits)
		: (bits_left - (sizeof(CURBITS_TYPE) << 3) + bits_left - cursor.exclude_bits));
}

void CBitstream::_UpdateCurBits(bool bEos)
{
	if (cursor.p + sizeof(CURBITS_TYPE) <= cursor.p_end)
	{
		cursor.curbits = CURBITS_VALUE(cursor.p);
		cursor.bits_left = sizeof(CURBITS_TYPE) * 8;
	}
	else
	{
		cursor.curbits = 0;
		for (uint8_t* pbyte = cursor.p; pbyte < cursor.p_end; pbyte++) {
			cursor.curbits <<= 8;
			cursor.curbits |= *(uint8_t*)pbyte;
			cursor.bits_left += 8;
		}
	}
}

void CBitstream::_Advance_InCacheBits(int n)
{
	cursor.bits_left -= n;
}

void CBitstream::_FillCurrentBits(bool bPeek)
{
	// Fill the curbits
	// It may be also external buffer or inner buffer type except the read-out ring-chunk buffer type.
	if (cursor.p < cursor.p_end)
	{
		assert(cursor.bits_left == 0);
		if (cursor.p_end <= cursor.p + sizeof(CURBITS_TYPE))
			cursor.p = cursor.p_end;
		else
			cursor.p += sizeof(CURBITS_TYPE);

		_UpdateCurBits();
	}
}

uint64_t CBitstream::_GetBits(int n, bool bPeek, bool bThrowExceptionHitStartCode)
{
	UNREFERENCED_PARAMETER(bThrowExceptionHitStartCode);

	uint64_t nRet = 0;

	if (bPeek)
	{
		// Make sure there is no active data in save point of bitstream
		assert(save_point.p == NULL);
	}

	int nAllLeftBits = GetAllLeftBits();
	if (n > nAllLeftBits)
		throw std::exception("invalid parameter, no enough data");

	if (cursor.bits_left == 0)
		_UpdateCurBits();

	// Activate a save_point for the current bit-stream cursor
	if (bPeek)
		save_point = cursor;

	while (n > 0 && cursor.bits_left > 0)
	{
		int nProcessed = std::min(n, cursor.bits_left);
		CURBITS_TYPE nMask = CURBITS_MASK(cursor.bits_left);
		nRet <<= nProcessed;
		nRet |= (cursor.curbits&nMask) >> (cursor.bits_left - nProcessed);
		_Advance_InCacheBits(nProcessed);

		if (cursor.bits_left == 0)
		{
			_FillCurrentBits(bPeek);
		}

		n -= nProcessed;
	}

	// Restore the save point
	if (save_point.p != NULL && bPeek == true)
	{
		cursor = save_point;
	}

	if (n != 0)
		throw std::exception("invalid parameter, no enough data");

	return nRet;
}

int CBitstream::SkipBits(int skip_bits)
{
	int ret_skip_bits = skip_bits;
	while (skip_bits > 0 && cursor.p < cursor.p_end)
	{
		int nProcessed = std::min(skip_bits, cursor.bits_left);
		_Advance_InCacheBits(nProcessed);

		if (cursor.bits_left == 0)
		{
			_FillCurrentBits();
		}

		skip_bits -= nProcessed;
	}

	ret_skip_bits -= skip_bits;

	return ret_skip_bits;
}

uint64_t CBitstream::GetBits(int n)
{
	if (n > sizeof(uint64_t) * 8)
		throw std::exception("invalid parameter");

	return _GetBits(n);
}

uint64_t CBitstream::PeekBits(int n)
{
	if (n > sizeof(uint64_t) * 8)
		throw std::exception("invalid parameter");

	if (n == 0)
		return 0;

	// Clean up the previous save point
	CleanSavePoint();

	return _GetBits(n, true);
}

uint8_t CBitstream::GetByte()
{
	return (uint8_t)GetBits(8);
}

uint16_t CBitstream::GetWord()
{
	return (uint16_t)GetBits(16);
}

uint32_t CBitstream::GetDWord()
{
	return (uint32_t)GetBits(32);
}

uint64_t CBitstream::GetQWord()
{
	return (uint64_t)GetBits(64);
}

int8_t CBitstream::GetChar()
{
	return (int8_t)GetBits(8);
}

int16_t CBitstream::GetShort()
{
	return (int16_t)GetBits(16);
}

int32_t CBitstream::GetLong()
{
	return (int32_t)GetBits(32);
}

int64_t CBitstream::GetLongLong()
{
	return (int64_t)GetBits(64);
}

CBitstream::~CBitstream()
{
}
