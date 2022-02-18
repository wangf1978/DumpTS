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
#include <stdio.h>
#include <tchar.h>
#include <memory.h>
#include <assert.h>
#include "AMArray.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4200)
#pragma warning(disable:4201)
#pragma pack(push,1)
#define PACKED
#else
#define PACKED __attribute__ ((__packed__))
#endif

typedef struct _AM_Bit_Array_Internal
{
	// bit_array_type
	// 0: simple bit array, the ptr_bit_bytes are the continuous memory after bit_array_size
	// 1: advance bit array, the ptr_bit_bytes contains (4 + 4(32-bit)/8(64-bit)) bytes
	// bit_array_unit_size_minus_1
	// bit_array_unit_size = (bit_array_unit_size_minus_1 + 1)
	union {
		struct {
#ifdef _BIG_ENDIAN_
			int bit_array_type : 1;
			int bit_array_unit_size_minus_1 : 6;
			int	bit_array_size : 25;
#else
			int bit_array_size : 25;
			int bit_array_unit_size_minus_1 : 6;
			int bit_array_type : 1;
#endif
		};
		int bit_array_hdr;
	};
	unsigned char ptr_bit_bytes[];
}AM_Bit_Array_Internal;

#define REAL_ARRAY_BIT_BUFFER(x) (x->bit_array_type==0?x->ptr_bit_bytes:(*(unsigned char**)(&x->ptr_bit_bytes[4])))

/*
The first 4 bytes represent the bit-array size, and the next bytes represent the bit array content
--------------------------------------------------------------
|bit-array size| b0 | b1 | b2 | b3 | b4 |......
--------------------------------------------------------------
*/
AMBitArray AM_CreateBitArray(int array_size, BIT_OPERATION bAllBitsOp, uint8_t nUnitBits)
{
	if (array_size*nUnitBits <= 0)
	{
		printf("[Bitarray] array_size(%d) * nUnitBits(%d) = %d is not greater than 0.\n", array_size, nUnitBits, array_size*nUnitBits);
		return NULL;
	}

	// To save the buffer, the array size should be less than 2^26
	if (array_size >= (1 << 26))
	{
		printf("[Bitarray] The array size: %d should be less than 2^26.\n", array_size);
		return NULL;
	}

	// decide how many bytes should be allocated
	int num_of_bytes = (array_size*nUnitBits + 7) / 8;
	uint8_t* array_container_bytes = new uint8_t[num_of_bytes + sizeof(int)];
	if (array_container_bytes == NULL)
	{
		printf("[Bitarray] Failed to allocate memory for bit-array with size: %d.\n", array_size);
		return NULL;
	}

	AM_Bit_Array_Internal* pBitArray = (AM_Bit_Array_Internal*)array_container_bytes;

	// set all bits to zero at default.
	if (bAllBitsOp&(BIT_CLEAR|BIT_SET))
		memset(pBitArray->ptr_bit_bytes, bAllBitsOp==BIT_SET?0XFF:0, num_of_bytes);
	
	pBitArray->bit_array_type = 0;
	pBitArray->bit_array_unit_size_minus_1 = nUnitBits - 1;
	pBitArray->bit_array_size = array_size;
	return (AMBitArray)pBitArray;
}

AMBitArray AM_CreateBitArrayFromBuffer(int array_size, uint8_t array_item_bit_size, unsigned char* pArrayBuf, int cbArrayBuf)
{
	if (array_size*array_item_bit_size <= 0)
	{
		printf("[Bitarray] array_size(%d) * nUnitBits(%d) = %d is not greater than 0.\n", array_size, array_item_bit_size, array_size*array_item_bit_size);
		return NULL;
	}

	// To save the buffer, the total number of bits should be NOT greater than the external buffer bit size
	if (array_size*array_item_bit_size > cbArrayBuf*8)
		return NULL;

	uint8_t* array_container_bytes = new uint8_t[4 + sizeof(void*) + sizeof(int)];
	if (array_container_bytes == NULL)
	{
		printf("[Bitarray] Failed to allocate memory for bit-array with size: %d.\n", array_size);
		return NULL;
	}

	AM_Bit_Array_Internal* pBitArray = (AM_Bit_Array_Internal*)array_container_bytes;
	pBitArray->bit_array_type = -1;
	pBitArray->bit_array_unit_size_minus_1 = array_item_bit_size - 1;
	pBitArray->bit_array_size = array_size;
	(*(int*)(&pBitArray->ptr_bit_bytes[0])) = cbArrayBuf;
	(*(unsigned char**)(&pBitArray->ptr_bit_bytes[4])) = pArrayBuf;
	return (AMBitArray)pBitArray;
}

AMBitArray AM_CloneBitArray(AMBitArray bit_array)
{
	AM_Bit_Array_Internal* pSrcBitArray = (AM_Bit_Array_Internal*)bit_array;
	if (pSrcBitArray == NULL)
		return NULL;

	int cbPayload = 0;
	uint8_t* array_container_bytes = NULL;
	if (pSrcBitArray->bit_array_type)
	{
		cbPayload = sizeof(int) + sizeof(int) + sizeof(void*);
		array_container_bytes = new uint8_t[cbPayload];
	}
	else
	{
		int num_of_bytes = (pSrcBitArray->bit_array_size*(pSrcBitArray->bit_array_unit_size_minus_1 + 1) + 7) / 8;
		cbPayload = num_of_bytes + sizeof(int);
		array_container_bytes = new uint8_t[num_of_bytes + sizeof(int)];
	}

	if (array_container_bytes == NULL)
	{
		printf("[Bitarray] Failed to allocate memory for bit-array with size: %d.\n", cbPayload);
		return NULL;
	}

	AM_Bit_Array_Internal* pBitArray = (AM_Bit_Array_Internal*)array_container_bytes;
	pBitArray->bit_array_type = pSrcBitArray->bit_array_type;
	pBitArray->bit_array_unit_size_minus_1 = pSrcBitArray->bit_array_unit_size_minus_1;
	pBitArray->bit_array_size = pSrcBitArray->bit_array_size;
	memcpy(pBitArray->ptr_bit_bytes, pSrcBitArray->ptr_bit_bytes, cbPayload - sizeof(int));
	return (AMBitArray)pBitArray;
}

void AM_DestoryBitArray(AMBitArray& bit_array)
{
	if (bit_array == NULL)
		return;

	uint8_t* pInPlaceBuf = (uint8_t*)bit_array;
	delete[] pInPlaceBuf;
	bit_array = NULL;
}

int AM_BitSetValue(AMBitArray bit_array, int array_idx, uint64_t val)
{
	if (bit_array == NULL)
		return RET_CODE_NOT_INITIALIZED;

	AM_Bit_Array_Internal* pBitArray = (AM_Bit_Array_Internal*)bit_array;
	if (pBitArray->bit_array_size <= array_idx)
		return RET_CODE_OUT_OF_RANGE;

	// Calculate the position of bit item
	int item_bits = (pBitArray->bit_array_unit_size_minus_1 + 1);
	int bit_pos = array_idx*item_bits;
	int byte_idx = bit_pos / 8;
	int bits_left = item_bits;
	int bits_suffix = (array_idx + 1)*item_bits >= ((byte_idx + 1) << 3)
		? 0 : (((byte_idx + 1) << 3) - (array_idx + 1)*item_bits);
	int bits_left_in_byte = ((byte_idx + 1) << 3) - bit_pos - bits_suffix;

	uint8_t* pBitByte = REAL_ARRAY_BIT_BUFFER(pBitArray);
	while (bits_left > 0)
	{
		uint8_t u8Val = (val >> (bits_left - bits_left_in_byte))&((1ULL << bits_left_in_byte) - 1);
		u8Val <<= bits_suffix;
		pBitByte[byte_idx] &= ~(((1 << bits_left_in_byte) - 1) << bits_suffix);
		pBitByte[byte_idx] |= u8Val;

		bits_left -= bits_left_in_byte;
		byte_idx++;
		bit_pos = byte_idx << 3;
		bits_suffix = (array_idx + 1)*item_bits >= ((byte_idx + 1) << 3) ? 0 : (int)(((byte_idx + 1) << 3) - (array_idx + 1)*item_bits);
		bits_left_in_byte = ((byte_idx + 1) << 3) - bit_pos - bits_suffix;
	}

	return RET_CODE_SUCCESS;
}

int AM_BitGetValue(AMBitArray bit_array, int array_idx, uint64_t* ptr_val)
{
	if (bit_array == NULL)
		return RET_CODE_NOT_INITIALIZED;

	AM_Bit_Array_Internal* pBitArray = (AM_Bit_Array_Internal*)bit_array;
	if (pBitArray->bit_array_size <= array_idx)
		return RET_CODE_OUT_OF_RANGE;

	// Calculate the position of bit item
	int item_bits = (pBitArray->bit_array_unit_size_minus_1 + 1);
	int bit_pos = array_idx*item_bits;
	int byte_idx = bit_pos / 8;
	int bits_left = item_bits;
	int bits_suffix = (array_idx + 1)*item_bits >= ((byte_idx + 1) << 3)
		? 0 : (((byte_idx + 1) << 3) - (array_idx + 1)*item_bits);
	int bits_left_in_byte = ((byte_idx + 1) << 3) - bit_pos - bits_suffix;

	uint64_t u64Val = 0;
	uint8_t* pBitByte = REAL_ARRAY_BIT_BUFFER(pBitArray);
	while (bits_left > 0)
	{
		u64Val <<= bits_left_in_byte;
		u64Val |= (pBitByte[byte_idx] >> bits_suffix)&((1 << bits_left_in_byte) - 1);

		bits_left -= bits_left_in_byte;
		byte_idx++;
		bit_pos = byte_idx << 3;
		bits_suffix = (array_idx + 1)*item_bits >= ((byte_idx + 1) << 3) ? 0 : (int)(((byte_idx + 1) << 3) - (array_idx + 1)*item_bits);
		bits_left_in_byte = ((byte_idx + 1) << 3) - bit_pos - bits_suffix;
	}

	AMP_SAFEASSIGN(ptr_val, u64Val);

	return RET_CODE_SUCCESS;
}

int AM_BitClear(AMBitArray bit_array, int array_idx)
{
	if (bit_array == NULL)
		return RET_CODE_NOT_INITIALIZED;

	AM_Bit_Array_Internal* pBitArray = (AM_Bit_Array_Internal*)bit_array;
	if (pBitArray->bit_array_size <= array_idx)
		return RET_CODE_OUT_OF_RANGE;

	// Calculate the position of bit item
	int item_bits = (pBitArray->bit_array_unit_size_minus_1 + 1);
	int bit_pos = array_idx*item_bits;
	int byte_idx = bit_pos / 8;
	int bits_left = item_bits;
	int bits_suffix = (array_idx + 1)*item_bits >= ((byte_idx + 1) << 3)
		? 0 : (((byte_idx + 1) << 3) - (array_idx + 1)*item_bits);
	int bits_left_in_byte = ((byte_idx + 1) << 3) - bit_pos - bits_suffix;

	uint8_t* pBitByte = REAL_ARRAY_BIT_BUFFER(pBitArray);
	while (bits_left > 0)
	{
		pBitByte[byte_idx] &= ~(((1 << bits_left_in_byte) - 1) << bits_suffix);

		bits_left -= bits_left_in_byte;
		byte_idx++;
		bit_pos = byte_idx << 3;
		bits_suffix = (array_idx + 1)*item_bits >= ((byte_idx + 1) << 3) ? 0 : (int)(((byte_idx + 1) << 3) - (array_idx + 1)*item_bits);
		bits_left_in_byte = ((byte_idx + 1) << 3) - bit_pos - bits_suffix;
	}

	return RET_CODE_SUCCESS;
}

int AM_BitSet(AMBitArray bit_array, int array_idx)
{
	if (bit_array == NULL)
		return RET_CODE_NOT_INITIALIZED;

	AM_Bit_Array_Internal* pBitArray = (AM_Bit_Array_Internal*)bit_array;
	if (pBitArray->bit_array_size <= array_idx)
		return RET_CODE_OUT_OF_RANGE;

	// Calculate the position of bit item
	int item_bits = (pBitArray->bit_array_unit_size_minus_1 + 1);
	int bit_pos = array_idx*item_bits;
	int byte_idx = bit_pos / 8;
	int bits_left = item_bits;
	int bits_suffix = (array_idx + 1)*item_bits >= ((byte_idx + 1) << 3)
		? 0 : (((byte_idx + 1) << 3) - (array_idx + 1)*item_bits);
	int bits_left_in_byte = ((byte_idx + 1) << 3) - bit_pos - bits_suffix;

	uint8_t* pBitByte = REAL_ARRAY_BIT_BUFFER(pBitArray);
	while (bits_left > 0)
	{
		pBitByte[byte_idx] |= (((1 << bits_left_in_byte) - 1) << bits_suffix);

		bits_left -= bits_left_in_byte;
		byte_idx++;
		bit_pos = byte_idx << 3;
		bits_suffix = (array_idx + 1)*item_bits >= ((byte_idx + 1) << 3) ? 0 : (int)(((byte_idx + 1) << 3) - (array_idx + 1)*item_bits);
		bits_left_in_byte = ((byte_idx + 1) << 3) - bit_pos - bits_suffix;
	}

	return RET_CODE_SUCCESS;
}

BOOL AM_IsBitSet(AMBitArray bit_array, int array_idx)
{
	if (bit_array == NULL)
		return FALSE;

	AM_Bit_Array_Internal* pBitArray = (AM_Bit_Array_Internal*)bit_array;
	if (pBitArray->bit_array_size <= array_idx)
		return FALSE;

	uint64_t u64Val = 0;
	if (AMP_FAILED(AM_BitGetValue(bit_array, array_idx, &u64Val)))
		return FALSE;

	return u64Val ? TRUE : FALSE;
}

int AM_ResizeBitArray(AMBitArray& bit_array, int new_array_size, BIT_OPERATION bAllExpandedBitOp)
{
	if (new_array_size <= 0)
	{
		AM_DestoryBitArray(bit_array);
		return RET_CODE_SUCCESS;
	}

	AM_Bit_Array_Internal* old_bit_array_ptr = (AM_Bit_Array_Internal*)bit_array;
	if (old_bit_array_ptr == NULL)
	{
		bit_array = AM_CreateBitArray(new_array_size, bAllExpandedBitOp);
		return bit_array==NULL?RET_CODE_OUTOFMEMORY:RET_CODE_SUCCESS;
	}
	else if (old_bit_array_ptr->bit_array_type)
	{
		// At present, does not support expand the bit array whose bit buffer is mapped from the external buffer
		int map_buffer_size = *(int*)(&old_bit_array_ptr->ptr_bit_bytes[0]);
		if (new_array_size > map_buffer_size*8)
			return RET_CODE_ERROR_NOTIMPL;

		old_bit_array_ptr->bit_array_size = new_array_size;
		return RET_CODE_SUCCESS;
	}

	// decide the new size is greater than the original or not.
	int orig_num_of_bytes = (old_bit_array_ptr->bit_array_size*(old_bit_array_ptr->bit_array_unit_size_minus_1 + 1) + 7)/8;
	int new_num_of_bytes  = (new_array_size*(old_bit_array_ptr->bit_array_unit_size_minus_1 + 1) + 7)/8;

	AM_Bit_Array_Internal* new_bit_array_ptr = NULL;
	int start_bit_pos = (old_bit_array_ptr->bit_array_size*(old_bit_array_ptr->bit_array_unit_size_minus_1 + 1) - 1) % 8 + 1, end_bit_pos = 8;
	unsigned char *pOverlapByte = old_bit_array_ptr->ptr_bit_bytes + (old_bit_array_ptr->bit_array_size*(old_bit_array_ptr->bit_array_unit_size_minus_1 + 1) - 1) / 8;
	if (new_num_of_bytes <= orig_num_of_bytes)
	{
		if (new_array_size <= old_bit_array_ptr->bit_array_size)
		{
			((AM_Bit_Array_Internal*)bit_array)->bit_array_size = new_array_size;
			return RET_CODE_SUCCESS;
		}
		else
		{
			end_bit_pos = (new_array_size*(old_bit_array_ptr->bit_array_unit_size_minus_1 + 1) - 1) % 8 + 1;
			old_bit_array_ptr->bit_array_size = new_array_size;
		}
	}
	else
	{
		new_bit_array_ptr = (AM_Bit_Array_Internal*)AM_CreateBitArray(new_array_size, bit_array==NULL?bAllExpandedBitOp:BIT_NOP, (uint8_t)(old_bit_array_ptr->bit_array_unit_size_minus_1 + 1));
		if (new_bit_array_ptr == NULL)
			return RET_CODE_OUTOFMEMORY;
		
		// Copy the original bits into new bit-array
		memcpy(new_bit_array_ptr->ptr_bit_bytes, old_bit_array_ptr->ptr_bit_bytes, orig_num_of_bytes);
		memset(new_bit_array_ptr->ptr_bit_bytes + orig_num_of_bytes, bAllExpandedBitOp==BIT_SET?0xFF:0, (size_t)new_num_of_bytes - orig_num_of_bytes);
		end_bit_pos = (new_array_size*(old_bit_array_ptr->bit_array_unit_size_minus_1 + 1) - 1) % 8 + 1;
	}

	if (bAllExpandedBitOp&(BIT_CLEAR|BIT_SET))
	{
		// initialize the bit value for the overlap parts
		for(int i=start_bit_pos;i<end_bit_pos;i++)
		{
			if (bAllExpandedBitOp == BIT_CLEAR)
				*pOverlapByte &= ~(1<<(7-i));
			else
				*pOverlapByte |= (1<<(7-i))&0xFF;
		}
	}

	uint8_t* array_container_bytes = (uint8_t*)bit_array;
	AMP_SAFEDELA(array_container_bytes);
	bit_array = (AMBitArray)new_bit_array_ptr;

	return RET_CODE_SUCCESS;
}

void AM_ResetBitArray(AMBitArray bit_array, BIT_OPERATION bAllBitOp)
{
	if(!(bAllBitOp&(BIT_CLEAR|BIT_SET)))
		return;

	AM_Bit_Array_Internal* pBitArray = (AM_Bit_Array_Internal*)bit_array;
	memset(REAL_ARRAY_BIT_BUFFER(pBitArray), bAllBitOp == BIT_SET ? 0xFF : 0, (pBitArray->bit_array_size*(pBitArray->bit_array_unit_size_minus_1 + 1) + 7) / 8);
}

const uint8_t* AM_GetBitArrayBuffer(AMBitArray bit_array)
{
	AM_Bit_Array_Internal* pBitArray = (AM_Bit_Array_Internal*)bit_array;
	if (pBitArray->bit_array_type == 0)
		return pBitArray->ptr_bit_bytes;

	return (*(uint8_t**)(&pBitArray->ptr_bit_bytes[4]));
}

const int AM_GetBitArraySize(AMBitArray bit_array)
{
	if (bit_array == NULL)
		return 0;

	AM_Bit_Array_Internal* pBitArray = (AM_Bit_Array_Internal*)bit_array;
	return pBitArray->bit_array_size;
}

#ifdef _WIN32
#pragma pack(pop)
#pragma warning(pop)
#endif
#undef PACKED
