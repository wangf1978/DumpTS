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
#ifndef _AM_ARRAY_LIST_H_
#define _AM_ARRAY_LIST_H_

#include "AMException.h"
#include <limits>
#include <vector>
#include <memory>

#define AM_INIT_ARRAY_SIZE					32
#define AM_BITS_BYTES(bits)					((bits + 7)>>3)
#define AM_BITUNITS_BYTES(n, item_size)		(((n)*(item_size) + 7)>>3)

#ifdef __cplusplus
extern "C"{
#endif

typedef void* AMBitArray;

enum BIT_OPERATION
{
	BIT_SET			= 1,
	BIT_CLEAR		= 2,
	BIT_NOP			= 4,
	BIT_UPDATE_MASK = (BIT_SET|BIT_CLEAR),
};

/*!	@brief Create a bit array. */
AMP_FOUNDATION_PROC AMBitArray	AM_CreateBitArray(int array_size, BIT_OPERATION bAllBitsOp=BIT_CLEAR, uint8_t nUnitBits=1);
/*!	@brief Create a bit array, and use the external array buffer, don't allocate bit buffer by self. */
AMP_FOUNDATION_PROC AMBitArray	AM_CreateBitArrayFromBuffer(int array_size, uint8_t array_item_bit_size, unsigned char* pArrayBuf, int cbArrayBuf);
/*!	@brief Clone a new bit-array from the existed bit-array. */
AMP_FOUNDATION_PROC AMBitArray	AM_CloneBitArray(AMBitArray bit_array);
/*!	@brief Destroy a bit array. */
AMP_FOUNDATION_PROC void		AM_DestoryBitArray(AMBitArray& bit_array);
/*!	@brief Set a value at the specified position of a bit-array. */
AMP_FOUNDATION_PROC int			AM_BitSetValue(AMBitArray bit_array, int array_idx, uint64_t val);
/*!	@brief Get the value at the specified position of a bit-array. */
AMP_FOUNDATION_PROC int			AM_BitGetValue(AMBitArray bit_array, int array_idx, uint64_t* ptr_val);
/*!	@brief Set the bit value to 0 at the position "array_idx". */
AMP_FOUNDATION_PROC int			AM_BitClear(AMBitArray bit_array, int array_idx);
/*!	@brief Set the bit value to 1 at the position "array_idx". */
AMP_FOUNDATION_PROC int			AM_BitSet(AMBitArray bit_array, int array_idx);
/*!	@brief Get the bit value at the position "array_idx". */
AMP_FOUNDATION_PROC BOOL		AM_IsBitSet(AMBitArray bit_array, int array_idx);
/*!	@brief Resize the current bit array. 
	@param bit_array NULL: create a new array, otherwise, resize this bit array. 
	@param new_array_size the new bit-array size. 
	@param bAllExpandedBitOp how to set the default value of the new expanded bits.
	@remarks It is not available for the array mapped to the external buffer, which is created with AM_CreateBitArray(int array_size, unsigned char* pArrayBuf, int cbArrayBuf) */
AMP_FOUNDATION_PROC int			AM_ResizeBitArray(AMBitArray& bit_array, int new_array_size, BIT_OPERATION bAllExpandedBitOp=BIT_CLEAR);
/*!	@brief Reset all bits values in the current array. */
AMP_FOUNDATION_PROC void		AM_ResetBitArray(AMBitArray bit_array, BIT_OPERATION bAllBitOp=BIT_CLEAR);
/*!	@brief Return the underlying bit-array buffer. */
AMP_FOUNDATION_PROC const uint8_t*
								AM_GetBitArrayBuffer(AMBitArray bit_array);
/*!	@brief Return the array size. */
AMP_FOUNDATION_PROC const int	AM_GetBitArraySize(AMBitArray bit_array);

#ifdef __cplusplus
}
#endif

template<
	typename T = BOOL,
	int init_array_size = 32,
	int init_array_bit_size = 1,
	typename std::enable_if<std::numeric_limits<T>::is_integer>::type* = nullptr>
class CAMBitsArray
{
public:
	CAMBitsArray() : CAMBitsArray(init_array_size, init_array_bit_size){}

	CAMBitsArray(int array_size, uint8_t array_bit_size=1) : m_upper_bound(-1)
	{
		m_bit_array = AM_CreateBitArray(array_size, BIT_CLEAR, array_bit_size);
	}

	CAMBitsArray(int array_size, uint8_t array_bit_size, unsigned char* pArrayBuf, int cbArrayBuf) : m_upper_bound(-1)
	{
		m_bit_array = AM_CreateBitArrayFromBuffer(array_size, array_bit_size, pArrayBuf, cbArrayBuf);
	}

	CAMBitsArray(_In_ const CAMBitsArray<T>& ArraySrc)
	{
		m_bit_array = AM_CloneBitArray(ArraySrc.m_bit_array);
		m_upper_bound = ArraySrc.m_upper_bound;
	}

	~CAMBitsArray() { AM_DestoryBitArray(m_bit_array); }

	BOOL BitSet(_In_ int idx) { return UpdateBits(idx, TRUE, TRUE); }
	BOOL BitClear(_In_ int idx) { return UpdateBits(idx, FALSE, TRUE); }
	BOOL BitSetValue(_In_ int idx, uint64_t val) { return UpdateBits(idx, val); }
	int Size() { return AM_GetBitArraySize(m_bit_array); }
	int UpperBound() { return m_upper_bound; }
	void Reset() { m_upper_bound = -1; }

	CAMBitsArray<T, init_array_size, init_array_bit_size>& operator=(_In_ const CAMBitsArray<T, init_array_size, init_array_bit_size>& ArraySrc)
	{
		if (this->m_bit_array != NULL)
			AM_DestoryBitArray(this->m_bit_array);
		this->m_bit_array = AM_CloneBitArray(ArraySrc.m_bit_array);
		this->m_upper_bound = ArraySrc.m_upper_bound;
		return (*this);
	}

	const uint8_t* GetBuffer(){return AM_GetBitArrayBuffer(m_bit_array);}

	const T operator[](_In_ int idx) const
	{
		int iRet = RET_CODE_SUCCESS;
		uint64_t u64Val = 0;
		if (AMP_FAILED(iRet = AM_BitGetValue(m_bit_array, idx, &u64Val)))
			throw AMException(iRet, _T("Failed to get the bitarray value."));
		return static_cast<T>(u64Val);
	}

protected:
	AMBitArray m_bit_array;
	int	m_upper_bound;
	BOOL UpdateBits(_In_ int idx, _In_ uint64_t u64SetValue, _In_ BOOL bBitSetClear=FALSE)
	{
		if (idx >= AM_GetBitArraySize(m_bit_array))
		{
			if (AMP_FAILED(AM_ResizeBitArray(m_bit_array, idx + 32)))
				return FALSE;
		}

		if (m_upper_bound < idx)
			m_upper_bound = idx;

		int nRet = bBitSetClear ? (u64SetValue ? AM_BitSet(m_bit_array, idx) : AM_BitClear(m_bit_array, idx)) : AM_BitSetValue(m_bit_array, idx, u64SetValue);
		return AMP_SUCCEEDED(nRet) ? TRUE : FALSE;
	}
};

typedef CAMBitsArray<> CAMBitArray;

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4201)
#pragma pack(push,1)
#define PACKED
#else
#define PACKED __attribute__ ((__packed__))
#endif

#ifdef _WIN32
#pragma pack(pop)
#pragma warning(pop)
#endif
#undef PACKED

#endif
