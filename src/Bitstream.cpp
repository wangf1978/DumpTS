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
#include "Bitstream.h"
#include <exception>
#include <assert.h>
#include <iostream>

CBitstream::CBitstream()
	: CBitstream(NULL, 0)
{}

CBitstream::CBitstream(uint8_t* pBuf, size_t cbitSize)
{
	size_t cbSize = (cbitSize + 7) >> 3;

	cursor.start_offset = ((intptr_t)pBuf & 3);
	cursor.p = cursor.p_start = pBuf?((uint8_t*)pBuf - cursor.start_offset):nullptr;
	cursor.p_end = pBuf?(pBuf + cbSize):nullptr;
	cursor.bits_left = (sizeof(CURBITS_TYPE) - cursor.start_offset) * 8;

	cursor.curbits = pBuf?CURBITS_VALUE(cursor.p):0;

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
		size_t nLeftBits = cbitSize - cursor.bits_left;
		cursor.exclude_bits = (uint8_t)(((nLeftBits + 7) & 0xFFFFFFF8) - nLeftBits);
	}

	memset(&save_point, 0, sizeof(save_point));
}

int CBitstream::GetAllLeftBits()
{
	int bits_left = cursor.p_end <= cursor.p ? 0 : (((int)(cursor.p_end - cursor.p)) << 3);
	return bits_left <= cursor.exclude_bits ? 0
		: (bits_left < (int)(sizeof(CURBITS_TYPE) << 3) ? (cursor.bits_left - cursor.exclude_bits)
			: (bits_left - (int)(sizeof(CURBITS_TYPE) << 3) + cursor.bits_left - cursor.exclude_bits));
}

void CBitstream::_UpdateCurBits(bool bEos)
{
	DBG_UNREFERENCED_PARAMETER(bEos);
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
	DBG_UNREFERENCED_PARAMETER(bPeek);
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

	if (cursor.bits_left == 0)
	{
		if (_EOF())
			throw std::out_of_range("reach EOF");

		_UpdateCurBits();
	}

	// Activate a save_point for the current bit-stream cursor
	if (bPeek)
		save_point = cursor;

	while (n > 0 && cursor.bits_left > 0)
	{
		int nProcessed = AMP_MIN(n, cursor.bits_left);
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
		throw std::out_of_range("invalid parameter, no enough data");

	return nRet;
}

int64_t CBitstream::SkipBits(int64_t skip_bits)
{
	int64_t ret_skip_bits = skip_bits;
	while (skip_bits > 0 && cursor.p < cursor.p_end)
	{
		int nProcessed = (int)AMP_MIN(skip_bits, (int64_t)cursor.bits_left);
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
	if (n > (int)sizeof(uint64_t) * 8)
		throw std::invalid_argument("invalid parameter");

	return _GetBits(n);
}

int64_t CBitstream::GetSignBits(int n)
{
	uint64_t u64Val = GetBits(n);

	if (n == 0 || (u64Val&(1ULL << (n - 1))) == 0)
		return u64Val;

	if (n == 64)
		return -((int64_t)((~u64Val) + 1ULL));
	
	uint64_t nMask = (uint64_t)((1ULL << n) - 1);
	return -((int64_t)(((~u64Val)&nMask) + 1ULL));
}

uint64_t CBitstream::PeekBits(int n)
{
	if (n > (int)sizeof(uint64_t) * 8)
		throw std::invalid_argument("invalid parameter");

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

	return ((uint64_t)(cursor.p_end - cursor.p_start) - cursor.start_offset) * 8 - nAllLeftBits;
}

int CBitstream::Seek(uint64_t bit_pos)
{
	if (bit_pos > ((uint64_t)(cursor.p_end - cursor.p_start) - cursor.start_offset) * 8)
		return -1;

	if (bit_pos == (uint64_t)-1LL)
		bit_pos = ((uint64_t)(cursor.p_end - cursor.p_start) - cursor.start_offset) * 8;

	uint8_t* ptr_dest = cursor.p_start + (bit_pos + (int64_t)cursor.start_offset * 8) / (sizeof(CURBITS_TYPE) * 8) * sizeof(CURBITS_TYPE);
	size_t bytes_left = (size_t)(cursor.p_end - cursor.p);
	size_t bits_left = AMP_MIN(bytes_left, sizeof(CURBITS_TYPE)) * 8 - (bit_pos + (int64_t)cursor.start_offset * 8) % (sizeof(CURBITS_TYPE) * 8);

	cursor.p = ptr_dest;
	_UpdateCurBits();
	cursor.bits_left = (int)bits_left;

	return 0;
}

int CBitstream::Read(uint8_t* buffer, int cbSize)
{
	if (cbSize < 0)
		return RET_CODE_INVALID_PARAMETER;

	if (cursor.bits_left % 8 != 0)
		return RET_CODE_NEEDBYTEALIGN;

	// Wrong pointer
	if (cursor.p_end < cursor.p || cursor.p_end > cursor.p + INT32_MAX)
		return RET_CODE_ERROR;

	int orig_size = cbSize;
	
	do
	{
		uint8_t* p_start = cursor.p + AMP_MIN((int)(cursor.p_end - cursor.p), (int)sizeof(CURBITS_TYPE)) - cursor.bits_left / 8;

		int left_bytes = cursor.p_end >= cursor.p + sizeof(CURBITS_TYPE) ?
			((int)(cursor.p_end - cursor.p - sizeof(CURBITS_TYPE)) + cursor.bits_left / 8) : cursor.bits_left / 8;

		int skip_bytes = AMP_MIN(left_bytes, cbSize);
		if (cursor.p_end > p_start)
		{
			memcpy(buffer, p_start, skip_bytes);
			buffer += skip_bytes;
			cbSize -= skip_bytes;
		}

		if (SkipBits((int64_t)skip_bytes << 3) <= 0)
			break;

	} while (cbSize > 0);

	return orig_size - cbSize;
}

int CBitstream::Peek(uint8_t* buffer, int cbSize)
{
	if (cbSize < 0)
		return RET_CODE_INVALID_PARAMETER;

	if (cursor.bits_left % 8 != 0)
		return RET_CODE_NEEDBYTEALIGN;

	// Wrong pointer
	if (cursor.p_end < cursor.p || cursor.p_end > cursor.p + INT32_MAX)
		return RET_CODE_ERROR;

	uint8_t* p_start = cursor.p + AMP_MIN((cursor.p_end - cursor.p), (int)sizeof(CURBITS_TYPE)) - cursor.bits_left / 8;
	int nAllLeftBits = GetAllLeftBits();

	if (nAllLeftBits <= 0)
		return 0;

	int peek_bytes = AMP_MIN((nAllLeftBits/8), cbSize);

	if (peek_bytes > 0)
		memcpy(buffer, p_start, peek_bytes);

	return peek_bytes;
}

int CBitstream::Realign(BITSTREAM_ALIGNMENT bstAlign)
{
	if (bstAlign == BYTE_ALIGNMENT)
	{
		// If the current mode is also under write mode, and cache bits has been switched
		cursor.p = cursor.p_start;
		cursor.bits_left = 0;

		_UpdateCurBits();

		return RET_CODE_SUCCESS;
	}
	else if (bstAlign == END_ALIGNMENT)
	{
		cursor.p = cursor.p_end;
		cursor.bits_left = 0;
		return RET_CODE_SUCCESS;
	}

	int skip_bits = 0, align_bits = 0;
	size_t nPos = (8 * (cursor.p - cursor.p_start) + (sizeof(cursor.curbits) * 8) - cursor.bits_left - (int64_t)cursor.start_offset * 8);
	switch (bstAlign)
	{
	case BYTE_ALIGNMENT: align_bits = 8; break;
	case WORD_ALIGNMENT: align_bits = 16; break;
	case DWORD_ALIGNMENT:align_bits = 32; break;
	case QWORD_ALIGNMENT:align_bits = 64; break;
	case NEXT_ALIGNMENT: align_bits = sizeof(CURBITS_TYPE); break;
	}
	if (align_bits == 0)
		return RET_CODE_INVALID_PARAMETER;

	int pos_mod = (int)(nPos%align_bits);
	if ((skip_bits = (pos_mod == 0 ? 0 : (align_bits - pos_mod))) == 0)
		return RET_CODE_SUCCESS;

	int nAllLeftBits = GetAllLeftBits();
	// check whether to be able skip "skip_bits"
	if (nAllLeftBits >= skip_bits)
		SkipBits(skip_bits);
	else
		return RET_CODE_IGNORE_REQUEST;

	return RET_CODE_SUCCESS;
}

bool CBitstream::IsAlign(BITSTREAM_ALIGNMENT bstAlign)
{
	if (bstAlign == BEGIN_ALIGNMENT)
		return cursor.p == cursor.p_start && cursor.bits_left == (int)(sizeof(CURBITS_TYPE) - cursor.start_offset) * 8 ? true : false;
	else if (bstAlign == END_ALIGNMENT)
		return cursor.p == cursor.p_end && cursor.bits_left == 0 ? true : false;

	int align_bits = 0;
	size_t nPos = (8 * (cursor.p - cursor.p_start) + (sizeof(cursor.curbits) * 8) - cursor.bits_left - (int64_t)cursor.start_offset * 8);
	switch (bstAlign)
	{
	case BYTE_ALIGNMENT: align_bits = 8; break;
	case WORD_ALIGNMENT: align_bits = 16; break;
	case DWORD_ALIGNMENT:align_bits = 32; break;
	case QWORD_ALIGNMENT:align_bits = 64; break;
	case NEXT_ALIGNMENT: align_bits = sizeof(CURBITS_TYPE); break;
	}
	if (align_bits == 0)
		return false;

	int pos_mod = (int)(nPos%align_bits);
	return (pos_mod == 0 ? 0 : (align_bits - pos_mod)) == 0 ? true : false;
}

CBitstream::~CBitstream()
{
}

CFileBitstream::CFileBitstream(const char* szFileName, int cache_size, int* ptr_ret)
	: m_fp(nullptr)
	, m_filesize(0)
	, m_filemappos(0)
{
	int iRet = -1;
	errno_t err_no;
	if (cache_size < (int)sizeof(int64_t))
		goto done;

	err_no = fopen_s(&m_fp, szFileName, "rb");
	if (err_no != 0 || m_fp == NULL)
		goto done;

	if (_fseeki64(m_fp, 0, SEEK_END) != 0)
		goto done;

	m_filemappos = 0;
	m_filesize = _ftelli64(m_fp);
	if (_fseeki64(m_fp, 0, SEEK_SET) != 0)
		goto done;

	cursor.p_start = new uint8_t[(cache_size + sizeof(CURBITS_TYPE) - 1) / sizeof(CURBITS_TYPE) * sizeof(CURBITS_TYPE) + sizeof(CURBITS_TYPE)];
	if (cursor.p_start == NULL) {
		goto done;
	}

	cursor.start_offset = 0;
	cursor.p = cursor.p_end = cursor.p_start;
	cursor.buf_size = cache_size;
	cursor.exclude_bits = 0;
	cursor.curbits = 0;
	cursor.bits_left = 0;

	_FillCurrentBits();
	
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

	if (cursor.p_start)
		delete[] cursor.p_start;
}

int64_t CFileBitstream::SkipBits(int64_t skip_bits)
{
	// Check whether the current skip_bits does not exceed the cache buffer
	int nAllLeftBits = GetAllLeftBits();
	ptrdiff_t cache_buf_size = cursor.p_end - cursor.p_start;
	int64_t bitpos_in_cache_buffer = (((int64_t)cache_buf_size<<3) - nAllLeftBits);
	int64_t skippos_in_cache_buffer = bitpos_in_cache_buffer + skip_bits;

	if (skippos_in_cache_buffer < 0 || skippos_in_cache_buffer >= ((int64_t)(cursor.p_end - cursor.p_start) << 3))
	{
		int64_t ret_skip_bits = skip_bits;
		// Go through the Seek operation
		int64_t skip_after_pos = (m_filemappos<<3) + skippos_in_cache_buffer;
		if (skip_after_pos < 0)
		{
			ret_skip_bits += skip_after_pos;
			skip_after_pos = 0;
		}
		else if (skip_after_pos >= (m_filesize << 3))
		{
			ret_skip_bits -= skip_after_pos - (m_filesize << 3);
			skip_after_pos = (m_filesize << 3);
		}

		if (Seek((uint64_t)skip_after_pos) != 0)
			throw std::ios_base::failure("failed to seek the specified bit-position");

		return ret_skip_bits;
	}
	
	return CBitstream::SkipBits(skip_bits);
}

uint64_t CFileBitstream::Tell(uint64_t* left_bits_in_bst)
{
	/*
	 File map position____                         ______ File position
	                      \                       /
	|______________________\                     /_________________
	                       |<-- Cache Buffer -->|
                 p_start__/      \               \
	                              \               \____ p_end
	                               \_ p
	*/

	if (m_filemappos < 0)
		throw std::out_of_range("invalid file position");

	int nAllLeftBits = GetAllLeftBits();
	ptrdiff_t cache_buf_size = cursor.p_end - cursor.p_start;
	uint64_t bitpos_in_cache_buffer = (((uint64_t)cache_buf_size<<3) - nAllLeftBits);

	AMP_SAFEASSIGN(left_bits_in_bst, nAllLeftBits + ((m_filesize - (m_filemappos + cache_buf_size)) << 3));

	return (((uint64_t)m_filemappos) << 3) + bitpos_in_cache_buffer;
}

bool CFileBitstream::_EOF()
{
	if (m_filemappos >= 0)
	{
		int nAllLeftBits = GetAllLeftBits();
		ptrdiff_t cache_buf_size = cursor.p_end - cursor.p_start;

		return (nAllLeftBits + ((m_filesize - (m_filemappos + cache_buf_size)) << 3)) == 0 ? true : false;
	}

	return false;
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
	uint64_t file_pos_p_start = m_filemappos;
	uint64_t file_pos_p_end = file_pos_p_start + (cursor.p_end - cursor.p_start);

	int iRet = -1;
	if (bitpos >= (file_pos_p_start << 3) && bitpos < (file_pos_p_end << 3))
	{
		iRet = CBitstream::Seek(bitpos - (file_pos_p_start << 3));
	}
	else if (bitpos < (uint64_t)(m_filesize<<3))
	{
		// Calculate the correct file position
		size_t align_bitcount = sizeof(CURBITS_TYPE)<<3;
		uint64_t byte_file_pos = (bitpos / align_bitcount * align_bitcount) >> 3;
		if (_fseeki64(m_fp, byte_file_pos, SEEK_SET) != 0)
			return -1;

		cursor.p = cursor.p_end;
		cursor.bits_left = 0;

		m_filemappos = byte_file_pos;

		_FillCurrentBits();

		// Locate to bit position
		cursor.bits_left = (int)(AMP_MIN((size_t)(cursor.p_end - cursor.p), sizeof(CURBITS_TYPE)) << 3) - (int)(bitpos % align_bitcount);

		iRet = 0;
	}
	else if (bitpos == (uint64_t)(m_filesize << 3))
	{
		if (_fseeki64(m_fp, 0, SEEK_END) != 0)
			return -1;

		cursor.p = cursor.p_end;
		cursor.bits_left = 0;

		m_filemappos = m_filesize;

		iRet = 0;
	}

	return iRet;
}

int CFileBitstream::Peek(uint8_t* buffer, int cbSize)
{
	if (cbSize < 0)
		return RET_CODE_INVALID_PARAMETER;

	if (cursor.bits_left % 8 != 0)
		return RET_CODE_NEEDBYTEALIGN;

	// Wrong pointer
	if (cursor.p_end < cursor.p || cursor.p_end > cursor.p + INT32_MAX)
		return RET_CODE_ERROR;

	ptrdiff_t cache_buf_size = cursor.p_end - cursor.p_start;
	uint8_t* p_start = cursor.p + AMP_MIN(cache_buf_size, (int)sizeof(CURBITS_TYPE)) - cursor.bits_left / 8;
	int nAllLeftBits = GetAllLeftBits();

	if (nAllLeftBits <= 0)
		return 0;

	if ((nAllLeftBits >> 3) >= cbSize)
	{
		memcpy(buffer, p_start, cbSize);
		return cbSize;
	}

	if (m_filemappos < 0)
		throw std::out_of_range("invalid file position");

	uint64_t bitpos_in_cache_buffer = (((uint64_t)cache_buf_size << 3) - nAllLeftBits);

	long long fpos = _ftelli64(m_fp);

	if (_fseeki64(m_fp, m_filemappos + (bitpos_in_cache_buffer >> 3), SEEK_SET) != 0)
		return -1;

	size_t cbRead = fread(buffer, 1, cbSize, m_fp);

	if (cbRead > INT_MAX)
		throw std::out_of_range("unexpected read return value");

	// restore the file position
	if (_fseeki64(m_fp, fpos, SEEK_SET) != 0)
		throw std::range_error("can't restore the file position");

	return (int)cbRead;
}

void CFileBitstream::_FillCurrentBits(bool bPeek)
{
	assert(cursor.bits_left == 0);

	bool bExhaust = (cursor.p + sizeof(CURBITS_TYPE) >= cursor.p_end && cursor.bits_left == 0) ? true : false;
	if (false == bExhaust)
	{
		CBitstream::_FillCurrentBits(bPeek);
		return;
	}

	size_t cbRead = 0;
	bool bEos = feof(m_fp) ? true : false;

	int will_fill = cursor.buf_size;
	uint8_t* will_read_from_buf = cursor.p_start;

	if (!bEos)
	{
		m_filemappos = _ftelli64(m_fp);

		// for peek case, don't overwrite the previous buffer, try to extend the cursor.p_end
		if (bPeek)
		{
			// For unexpected case, return directly
			if (save_point.p_end < save_point.p || (uint64_t)(save_point.p_end - save_point.p) >= (uint64_t)INT32_MAX)
				return;

			int before_peek_remaining_buf_size = (int)(save_point.p_end - save_point.p);
			if (save_point.bits_left > 0)
			{
				// shift cursor.p to cursor.p_start
				if (save_point.p_end > save_point.p)
					memmove(save_point.p_start, save_point.p, before_peek_remaining_buf_size);

				m_filemappos -= before_peek_remaining_buf_size;

				save_point.p = save_point.p_start;
				
				// Buffer can't be filled with full size, fill the left buffer
				assert(will_fill - before_peek_remaining_buf_size);
				will_fill -= before_peek_remaining_buf_size;
				will_read_from_buf += before_peek_remaining_buf_size;
			}
		}

		if (will_fill > 0)
		{
			cbRead = fread(will_read_from_buf, 1, will_fill, m_fp);
			if (cbRead <= 0)
				return;
		}

		bEos = feof(m_fp) ? true : false;
	}
	else
	{
		// No data to be read
		return;
	}

	cursor.p = will_read_from_buf;
	cursor.p_end = cursor.p + cbRead;

	if (bPeek)
		save_point.p_end = cursor.p_end;

	_UpdateCurBits(bEos);

	return;
}

