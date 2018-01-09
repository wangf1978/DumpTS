#include "stdafx.h"
#include "Bitstream.h"
#include <exception>
#include <assert.h>

CBitstream::CBitstream()
	: CBitstream(NULL, 0)
{}

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
		: (bits_left < (sizeof(CURBITS_TYPE) << 3) ? (cursor.bits_left - cursor.exclude_bits)
			: (bits_left - (sizeof(CURBITS_TYPE) << 3) + cursor.bits_left - cursor.exclude_bits));
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

uint64_t CBitstream::_GetBits(int n, bool bPeek, bool bFullBufferMode, bool bThrowExceptionHitStartCode)
{
	UNREFERENCED_PARAMETER(bThrowExceptionHitStartCode);

	uint64_t nRet = 0;

	if (bPeek)
	{
		// Make sure there is no active data in save point of bitstream
		assert(save_point.p == NULL);
	}

	if (bFullBufferMode)
	{
		int nAllLeftBits = GetAllLeftBits();
		if (n > nAllLeftBits)
			throw std::exception("invalid parameter, no enough data");

		if (cursor.bits_left == 0)
			_UpdateCurBits();

		// Activate a save_point for the current bit-stream cursor
		if (bPeek)
			save_point = cursor;
	}

	if (!bFullBufferMode)
	{
		if (cursor.p == cursor.p_end && cursor.bits_left == 0)
		{
			_FillCurrentBits(bPeek);
		}
	}

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

uint64_t CBitstream::Tell(uint64_t* left_bits_in_bst)
{
	if (cursor.p_start == NULL || cursor.p == NULL)
	{
		if (left_bits_in_bst)
			*left_bits_in_bst = 0;
		return 0;
	}

	int nAllLeftBits = GetAllLeftBits();
	if (left_bits_in_bst != NULL)
		*left_bits_in_bst = nAllLeftBits;

	return (uint64_t)(8 * (cursor.p_end - cursor.p_start - cursor.start_offset) - nAllLeftBits);
}

int CBitstream::Seek(uint64_t bit_pos)
{
	if (bit_pos > (cursor.p_end - cursor.p_start - cursor.start_offset) * 8)
		return -1;

	if (bit_pos == (uint64_t)-1LL)
		bit_pos = (uint64_t)(cursor.p_end - cursor.p_start - cursor.start_offset) * 8;

	uint8_t* ptr_dest = cursor.p_start + (bit_pos + cursor.start_offset * 8) / (sizeof(CURBITS_TYPE) * 8) * sizeof(CURBITS_TYPE);
	size_t bytes_left = (size_t)(cursor.p_end - cursor.p);
	size_t bits_left = std::min(bytes_left, sizeof(CURBITS_TYPE)) * 8 - (bit_pos + cursor.start_offset * 8) % (sizeof(CURBITS_TYPE) * 8);

	cursor.p = ptr_dest;
	_UpdateCurBits();
	cursor.bits_left = bits_left;

	return 0;
}


CBitstream::~CBitstream()
{
}

CFileBitstream::CFileBitstream(const char* szFileName, int cache_size, int* ptr_ret)
{
	int iRet = -1;
	if (cache_size < sizeof(int64_t))
		goto done;

	errno_t err_no = fopen_s(&m_fp, szFileName, "rb");
	if (err_no != 0 || m_fp == NULL)
		goto done;

	if (_fseeki64(m_fp, -1, SEEK_END) != 0)
		goto done;

	m_filesize = _ftelli64(m_fp);
	if (_fseeki64(m_fp, 0, SEEK_SET) != 0)
		goto done;

	cursor.p_start = new uint8_t[(cache_size + 3) / 4 * 4 + 4];
	if (cursor.p_start == NULL) {
		goto done;
	}

	cursor.start_offset = 0;
	cursor.p = cursor.p_end = cursor.p_start;
	cursor.buf_size = cache_size;
	cursor.exclude_bits = 0;
	cursor.curbits = 0;
	cursor.bits_left = 0;
	
	iRet = 0;

done:
	if (ptr_ret)
		*ptr_ret = iRet;

	if (iRet < 0)
	{
		if (m_fp != NULL)
		{
			fclose(m_fp);
			m_fp = NULL;
		}
	}
}

CFileBitstream::~CFileBitstream()
{
	if (m_fp != NULL)
	{
		fclose(m_fp);
		m_fp = NULL;
	}
}

uint64_t CFileBitstream::Tell(uint64_t* left_bits_in_bst)
{
	/*
	                                               ______ File position
	                                              /
	|______________________                      /_________________
	                       |<-- Cache Buffer -->|
                 p_start__/      \               \
	                              \               \____ p_end
	                               \_ p
	*/

	long long file_pos = _ftelli64(m_fp);

	if (file_pos < 0 || file_pos < (cursor.p_end - cursor.p))
		throw std::exception("invalid file position");

	uint64_t byte_position = file_pos - (cursor.p_end - cursor.p);
	uint64_t bitpos_in_curword = std::min((size_t)(cursor.p_end - cursor.p), sizeof(CURBITS_TYPE)) - cursor.bits_left;

	if (left_bits_in_bst)
	{
		int nAllLeftBits = GetAllLeftBits();
		*left_bits_in_bst = nAllLeftBits + ((m_filesize - file_pos) << 3);
	}

	return (byte_position << 3) + bitpos_in_curword;
}

int CFileBitstream::Seek(uint64_t bitpos)
{
	/*
	                                               ______ File position
	                                              /
	|______________________                      /_________________
	                       |<-- Cache Buffer -->|
                 p_start__/      \               \
	                              \               \____ p_end
	                               \_ p
	*/
	// At first check whether bitpos lies at the Cache Buffer or not
	long long file_pos = _ftelli64(m_fp);

	uint64_t file_pos_p_start = file_pos - (cursor.p_end - cursor.p_start);	// start_offset is always equal to 0
	uint64_t file_pos_p_end = file_pos;

	int iRet = -1;
	if (bitpos >= (file_pos_p_start << 3) && bitpos < (file_pos_p_end << 3))
	{
		iRet = CBitstream::Seek(bitpos - (file_pos_p_start << 3));
	}
	else if (bitpos < (uint64_t)(m_filesize<<3))
	{
		// Calculate the correct file position
		size_t align_bitcount = sizeof(CURBITS_TYPE)<<3;
		uint64_t byte_file_pos = bitpos / align_bitcount;
		if (_fseeki64(m_fp, byte_file_pos, SEEK_SET) != 0)
			return -1;

		cursor.p = cursor.p_end;
		cursor.bits_left = 0;

		_FillCurrentBits();

		// Locate to bit position
		cursor.bits_left = std::min((size_t)(cursor.p_end - cursor.p), sizeof(CURBITS_TYPE)) - (bitpos % align_bitcount);
	}

	return iRet;
}

void CFileBitstream::_FillCurrentBits(bool bPeek)
{
	assert(cursor.bits_left == 0);

	bool bExhaust = (cursor.p == cursor.p_end && cursor.bits_left == 0) ? true : false;
	if (false == bExhaust)
		return;

	bool bEos = false;

	size_t cbRead = fread(cursor.p_start, 1, cursor.buf_size, m_fp);
	if (cbRead <= 0)
		return;

	bEos = feof(m_fp)?true:false;

	if (bPeek)
	{
		// Activate a save point for future restore
		save_point = cursor;
	}

	cursor.p = cursor.p_start;
	cursor.p_end = cursor.p_start + cbRead;

	_UpdateCurBits(bEos);

	return;
}
