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
#pragma once

#include <stdint.h>
#include <exception>

//#define _USE_64BIT_CACHE

#ifdef _USE_64BIT_CACHE
#define CURBITS_TYPE				uint64_t
#define CURBITS_VALUE(p)			ENDIANUINT64(*(CURBITS_TYPE*)p)
#define CURBITS_VALUE1(n)			ENDIANUINT64(n)
#define CURBITS_MASK(bits)			bits == 64 ? UINT64_MAX : ~(UINT64_MAX << bits)
#else
#define CURBITS_TYPE				uint32_t
#define CURBITS_VALUE(p)			ENDIANULONG(*(CURBITS_TYPE*)p)
#define CURBITS_VALUE1(n)			ENDIANULONG(n)
#define CURBITS_MASK(bits)			bits == 32 ? UINT32_MAX : ~(UINT32_MAX << bits)
#endif

enum BITSTREAM_ALIGNMENT
{
	BYTE_ALIGNMENT,
	WORD_ALIGNMENT,
	DWORD_ALIGNMENT,
	QWORD_ALIGNMENT,
	NEXT_ALIGNMENT,										// realign to the next cache bits
	BEGIN_ALIGNMENT,									// realign to its start position
	END_ALIGNMENT										// realign to the end position, it may trigger request new bitstream
};

class CBitstream
{
	// bit-stream cursor information, which is used to get/peek value from bit-stream
	struct AM_BST_CURSOR
	{
		uint8_t*			p_start;					// bitstream start pointer
		uint8_t*			p;							// bitstream current pointer
		uint8_t*			p_end;						// bitstream end pointer, [start_pointer, end_pointer - exclude_bits) contains valid data
		uint8_t				exclude_bits;				// how many last bits are excluded in the last byte *(end_pointer-1)
		uint8_t				reserved[3];
		int					buf_size;
		int					start_offset;				// give the offset between the aligned address and aligned address which is the first byte of bitstream
		int					bits_left;					// Indicate how many bits are left in "curbits"
		CURBITS_TYPE		curbits;					// It is used for read/get operation
	};

public:
	CBitstream(uint8_t* pBuf, size_t cbitSize);
	virtual ~CBitstream();

	/*!	@brief Get bits value from the current bitstream. */
	virtual uint64_t	GetBits(int n);

	/*!	@brief Get sign-bits value from the current bitstream. */
	virtual int64_t		GetSignBits(int n);

	/*!	@brief Peek bits value without changing bitstream status
		@remarks if bitstream is related with Ring-Chunk(type=BST_TYPE_RINGCHUNK), in the current chunk,
		the left bits is not enough for peeking, the exception will be raised. */
	virtual uint64_t	PeekBits(int n);
	virtual int64_t		SkipBits(int64_t skip_bits);
	/*!	@brief Return the position of bit-stream in unit of bits. */
	virtual uint64_t	Tell(uint64_t* left_bits_in_bst = NULL);
	/*!	@brief Seek to the absolute position in the specified bit-stream. */
	virtual int			Seek(uint64_t bit_pos);
	/*!	@brief Read data from the bit-stream */
	virtual int			Read(uint8_t* buffer, int cbSize);
	/*! @brief Peek data from the bit-stream, and the read cursor will not move forward */
	virtual int			Peek(uint8_t* buffer, int cbSize);

	/*!	@brief Get one byte from bit-stream. */
	uint8_t				GetByte();
	/*!	@brief Get one 16-bit unsigned short integer/WORD from bit-stream. */
	uint16_t			GetWord();
	/*!	@brief Get one 32-bit unsigned integer/DWORD from bit-stream. */
	uint32_t			GetDWord();
	/*!	@brief Get one 64-bit unsigned integer/QWORD from bit-stream. */
	uint64_t			GetQWord();
	/*!	@brief Get one signed character from bit-stream. */
	int8_t				GetChar();
	/*!	@brief Get one 16-bit signed short integer from bit-stream. */
	int16_t				GetShort();
	/*!	@brief Get one 32-bit signed integer from bit-stream. */
	int32_t				GetLong();
	/*!	@brief Get one 64-bit signed integer from bit-stream. */
	int64_t				GetLongLong();

	int					Realign(BITSTREAM_ALIGNMENT bstAlign = BYTE_ALIGNMENT);
	bool				IsAlign(BITSTREAM_ALIGNMENT bstAlign = BYTE_ALIGNMENT);

	template <typename T>
	void				GetBits(int n, T& val);

protected:
	virtual int			GetAllLeftBits();
	/*! Get or peek bits from the underlying buffer. 
		@param bFullBufferMode if its value is true, it means that all data is already in the buffer. */
	virtual uint64_t	_GetBits(int n, bool bPeek = false, bool bThrowExceptionHitStartCode = false);
	virtual void		_UpdateCurBits(bool bEos = false);
	virtual void		_Advance_InCacheBits(int n);
	virtual void		_FillCurrentBits(bool bPeek = false);
	virtual bool		_EOF() { return false; }
	
	void				CleanSavePoint() {save_point.p = NULL;}

protected:
	CBitstream();

protected:
	AM_BST_CURSOR		cursor;
	AM_BST_CURSOR		save_point;

};

class CFileBitstream : public CBitstream
{
public:
	CFileBitstream(const char* szFileName, int cache_size = 0, int* ptr_ret = NULL);
	virtual ~CFileBitstream();

	virtual int64_t		SkipBits(int64_t skip_bits);
	/*!	@brief Return the position of bit-stream in unit of bits. */
	virtual uint64_t	Tell(uint64_t* left_bits_in_bst = NULL);
	/*!	@brief Seek to the absolute position in the specified bit-stream. */
	virtual int			Seek(uint64_t bit_pos);
	virtual int			Peek(uint8_t* buffer, int cbSize);

protected:
	virtual void		_FillCurrentBits(bool bPeek = false);
	virtual bool		_EOF();

protected:
	FILE*				m_fp;
	int64_t				m_filesize;		// the file size of current file
	int64_t				m_filemappos;	// the file position which is mapped to the start address of cache buffer
};

template <typename T> void CBitstream::GetBits(int n, T& val)
{
	val = (T)GetBits(n);
}

inline uint8_t CBitstream::GetByte()
{
	return (uint8_t)GetBits(8);
}

inline uint16_t CBitstream::GetWord()
{
	return (uint16_t)GetBits(16);
}

inline uint32_t CBitstream::GetDWord()
{
	return (uint32_t)GetBits(32);
}

inline uint64_t CBitstream::GetQWord()
{
	return (uint64_t)GetBits(64);
}

inline int8_t CBitstream::GetChar()
{
	return (int8_t)GetBits(8);
}

inline int16_t CBitstream::GetShort()
{
	return (int16_t)GetBits(16);
}

inline int32_t CBitstream::GetLong()
{
	return (int32_t)GetBits(32);
}

inline int64_t CBitstream::GetLongLong()
{
	return (int64_t)GetBits(64);
}

template<class First, class Tuple, std::size_t N, std::size_t K = N>
struct ArrayFiller {
	static void fill_bytes_from_tuple(const Tuple& t, std::vector<uint8_t>& bytes) {
		ArrayFiller<First, Tuple, N, K - 1>::fill_bytes_from_tuple(t, bytes);
		auto val = std::get<K - 1>(t);
		uint8_t* pBytes = (uint8_t*)&val;
		for (size_t i = 0; i < sizeof(val); i++)
			bytes.push_back(pBytes[i]);
	}
};

template<class First, class Tuple, std::size_t N>
struct ArrayFiller<First, Tuple, N, 1> {
	static void fill_bytes_from_tuple(const Tuple& t, std::vector<uint8_t>& bytes) {
		auto val = std::get<0>(t);
		uint8_t* pBytes = (uint8_t*)&val;
		for (size_t i = 0; i < sizeof(val); i++)
			bytes.push_back(pBytes[i]);
	}
};

template<typename First, typename... Rem>
void fill_bytes_from_tuple(const std::tuple<First, Rem...>& t, std::vector<uint8_t>& bytes) {
	ArrayFiller<First, decltype(t), 1 + sizeof...(Rem)>::fill_bytes_from_tuple(t, bytes);
}

//
// A utility bit stream reader aiming the best performance and the minimum memory foot-print
// And the allocated buffer is in stack instead of heap
//
struct BITBUF
{
	const uint8_t*		pBuf;
	size_t				cbBuf;
	size_t				bitoffset = 0;

	BITBUF(const uint8_t* p, size_t cb) : pBuf(p), cbBuf(cb) {}

	int GetFlag(bool& bFlag) {
		if (bitoffset + 1 > (cbBuf << 3))
			return RET_CODE_BUFFER_TOO_SMALL;

		pBuf += (bitoffset >> 3);
		cbBuf -= (bitoffset >> 3);
		bitoffset = bitoffset & 0x7;

		bFlag = (*pBuf >> (8 - (bitoffset % 8) - 1)) & 0x1;
		bitoffset = (bitoffset % 8) + 1;
		return RET_CODE_SUCCESS;
	}

	int GetByte(uint8_t& u8Val)
	{
		if (bitoffset + 8 > (cbBuf << 3))
			return RET_CODE_BUFFER_TOO_SMALL;

		pBuf += (bitoffset >> 3);
		cbBuf -= (bitoffset >> 3);
		bitoffset = bitoffset & 0x7;

		uint8_t left_bits = (uint8_t)(8 - (bitoffset % 8));
		u8Val = (*pBuf) &((1 << left_bits) - 1);
		pBuf++;
		cbBuf--;

		if (left_bits < 8)
		{
			left_bits = 8 - left_bits;
			u8Val = (u8Val << left_bits) | (((*pBuf) >> (8 - left_bits))&((1 << left_bits) - 1));
			bitoffset = left_bits;
		}
		else
			bitoffset = 0;
		return RET_CODE_SUCCESS;
	}

	template<class T>
	int GetValue(uint8_t n, T& uVal) {
		if (n > 64)
			return RET_CODE_INVALID_PARAMETER;

		if (bitoffset + n > (cbBuf << 3))
			return RET_CODE_BUFFER_TOO_SMALL;

		uint64_t u64Val;
		uint8_t head_part_left = 0, body_cout_of_bytes = 0, tail_part_left = 0;

		pBuf += (bitoffset >> 3);
		cbBuf -= (bitoffset >> 3);
		bitoffset = bitoffset & 0x7;

		head_part_left = 8 - (uint8_t)(bitoffset % 8);
		pBuf += bitoffset / 8;

		if (head_part_left <= n)
		{
			body_cout_of_bytes = (n - head_part_left) / 8;
			tail_part_left = (n - head_part_left) % 8;

			u64Val = (*pBuf)&((1 << head_part_left) - 1); pBuf++; cbBuf--;

			for (uint8_t i = 0; i < body_cout_of_bytes; i++) {
				u64Val = (u64Val << 8) | (*pBuf);
				pBuf++; cbBuf--;
			}

			if (tail_part_left > 0) {
				u64Val = (u64Val << tail_part_left) | (((*pBuf) >> (8 - tail_part_left))&((1 << tail_part_left) - 1));
				bitoffset = tail_part_left;
			}
			else
				bitoffset = 0;
		}
		else
		{
			u64Val = (((*pBuf)&((1 << head_part_left) - 1)) >> (head_part_left - n))&((1 << n) - 1);
			bitoffset += n;
		}

		uVal = (T)(u64Val);

		return RET_CODE_SUCCESS;
	}

	inline int Skip(size_t nSkipBits)
	{
		if (bitoffset + nSkipBits > (cbBuf << 3))
			return RET_CODE_BUFFER_TOO_SMALL;

		bitoffset += nSkipBits;
		return RET_CODE_SUCCESS;
	}

	template<class T>
	int AV1ns(uint64_t v_max, T& uVal)
	{
		if (v_max == 0)
			return RET_CODE_INVALID_PARAMETER;

		int8_t w = 0;
		uint64_t vv = v_max;
		int iRet = RET_CODE_SUCCESS;
		while (vv != 0){
			vv = vv >> 1;
			w++;
		}

		uint64_t v = 0;
		uint64_t m = (1ULL << w) - v_max;
		if (w > 1) {
			if (AMP_FAILED(iRet = GetValue(w - 1, v)))
				return iRet;
		}

		if (v < m)
		{
			uVal = (T)v;
			return RET_CODE_SUCCESS;
		}

		uint64_t extract_bit;
		if (AMP_FAILED(iRet = GetValue(1, extract_bit)))
			return iRet;

		uVal = (T)((v << 1) - m + extract_bit);
		return iRet;
	}

	template<class T>
	int AV1su(uint8_t nBits, T& uVal)
	{
		if (nBits > 64)
			return RET_CODE_INVALID_PARAMETER;

		uint64_t value;
		int iRet = RET_CODE_SUCCESS;
		if (AMP_FAILED(iRet = GetValue(nBits, value)))
			return iRet;
		uint64_t signMask = 1ULL << (nBits - 1);
		if (value & signMask)
			uVal = (T)(value - (signMask << 1));
		else
			uVal = (T)value;
		return iRet;
	}

	inline void SkipFast(size_t nSkipBits){
		bitoffset += nSkipBits;
	}

	inline int ByteAlign(){
		if (bitoffset % 8)
		{
			bitoffset += 8 - (bitoffset % 8);

			if (bitoffset > (cbBuf << 3))
				return RET_CODE_BUFFER_TOO_SMALL;

			pBuf += (bitoffset >> 3);
			cbBuf -= (bitoffset >> 3);
			bitoffset = 0;
		}
		return RET_CODE_SUCCESS;
	}
};

