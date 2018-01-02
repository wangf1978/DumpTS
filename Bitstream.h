#pragma once

#include <stdint.h>

#ifdef _BIG_ENDIAN_
#define ENDIANUSHORT(src)           (uint16_t)src
#define ENDIANULONG(src)			(uint32_t)src
#define ENDIANUINT64(src)			(uint64_t)src

#define USHORT_FIELD_ENDIAN(field)
#define ULONG_FIELD_ENDIAN(field)
#define UINT64_FIELD_ENDIAN(field)
#else
#define ENDIANUSHORT(src)			((uint16_t)((((src)>>8)&0xff) |\
												(((src)<<8)&0xff00)))

#define ENDIANULONG(src)			((uint32_t)((((src)>>24)&0xFF) |\
												(((src)>> 8)&0xFF00) |\
												(((src)<< 8)&0xFF0000) |\
												(((src)<<24)&0xFF000000)))

#define ENDIANUINT64(src)			((uint64_t)((((src)>>56)&0xFF) |\
												(((src)>>40)&0xFF00) |\
												(((src)>>24)&0xFF0000) |\
												(((src)>> 8)&0xFF000000) |\
												(((src)<< 8)&0xFF00000000LL) |\
												(((src)<<24)&0xFF0000000000LL) |\
												(((src)<<40)&0xFF000000000000LL) |\
												(((src)<<56)&0xFF00000000000000LL)))

#define USHORT_FIELD_ENDIAN(field)	field = ENDIANUSHORT(((uint16_t)field));
#define ULONG_FIELD_ENDIAN(field)	field = ENDIANULONG(((uint32_t)field));
#define UINT64_FIELD_ENDIAN(field)	field = ENDIANUINT64(((uint64_t)field));
#endif //_BIG_ENDIAN_

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

class CBitstream
{
public:
	// bit-stream cursor information, which is used to get/peek value from bit-stream
	struct AM_BST_CURSOR
	{
		uint8_t*			p_start;					// bitstream start pointer
		uint8_t*			p;							// bitstream current pointer
		uint8_t*			p_end;						// bitstream end pointer, [start_pointer, end_pointer - exclude_bits) contains valid data
		uint8_t				exclude_bits;				// how many last bits are excluded in the last byte *(end_pointer-1)
		uint8_t				reserved[3];
		int					start_offset;
		int					bits_left;					// How many bits left in "curbits"
		CURBITS_TYPE		curbits;					// It is used for read/get operation
	};

public:
	CBitstream(uint8_t* pBuf, size_t cbitSize);
	~CBitstream();

	/*!	@brief Get bits value from the current bitstream. */
	uint64_t			GetBits(int n);

	template <typename T> 
	void				GetBits(int n, T& val);
	/*!	@brief Peek bits value without changing bitstream status
		@remarks if bitstream is related with Ring-Chunk(type=BST_TYPE_RINGCHUNK), in the current chunk,
		the left bits is not enough for peeking, the exception will be raised. */
	uint64_t			PeekBits(int n);
	int					SkipBits(int skip_bits);
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

	uint32_t			Tell(int* left_bits_in_bst = NULL);

protected:
	int					GetAllLeftBits();
	uint64_t			_GetBits(int n, bool bPeek = false, bool bThrowExceptionHitStartCode = false);
	void				_UpdateCurBits(bool bEos = false);
	void				_Advance_InCacheBits(int n);
	void				_FillCurrentBits(bool bPeek = false);
	void				CleanSavePoint() {save_point.p = NULL;}

private:
	AM_BST_CURSOR		cursor;
	AM_BST_CURSOR		save_point;
};

template <typename T> void CBitstream::GetBits(int n, T& val)
{
	val = (T)GetBits(n);
}

template<class First, class Tuple, std::size_t N, std::size_t K = N>
struct ArrayFiller {
	static void fill_bytes_from_tuple(const Tuple& t, std::vector<uint8_t>& bytes) {
		ArrayFiller<First, Tuple, N, K - 1>::fill_bytes_from_tuple(t, bytes);
		auto val = std::get<K - 1>(t);
		uint8_t* pBytes = (uint8_t*)&val;
		for (int i = 0; i < sizeof(val); i++)
			bytes.push_back(pBytes[i]);
	}
};

template<class First, class Tuple, std::size_t N>
struct ArrayFiller<First, Tuple, N, 1> {
	static void fill_bytes_from_tuple(const Tuple& t, std::vector<uint8_t>& bytes) {
		auto val = std::get<0>(t);
		uint8_t* pBytes = (uint8_t*)&val;
		for (int i = 0; i < sizeof(val); i++)
			bytes.push_back(pBytes[i]);
	}
};

template<typename First, typename... Rem>
void fill_bytes_from_tuple(const std::tuple<First, Rem...>& t, std::vector<uint8_t>& bytes) {
	ArrayFiller<First, decltype(t), 1 + sizeof...(Rem)>::fill_bytes_from_tuple(t, bytes);
}

