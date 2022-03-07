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
#ifndef _LINEAR_RING_BUFFER_H_
#define _LINEAR_RING_BUFFER_H_

#include <assert.h>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Single ring buffer utility, which is not a safe-thread implementation
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T, const int ring_unit_size>
struct CAMRingUnits
{
	T					units[ring_unit_size] = { 0 };
	// fill 3 bytes in the ring buffer
	int					read_pos;					// read from which bytes
	int					write_pos;					// write from which bytes;
	int					len;						// current ring bytes length;

	CAMRingUnits() : read_pos(0), write_pos(0), len(0) {
	}

	CAMRingUnits(CAMRingUnits& ring_units) {
		for (int i = 0; i < ring_unit_size; i++)
			units[i] = ring_units.units[i];

		read_pos = ring_units.read_pos;
		write_pos = ring_units.write_pos;
		len = ring_units.len;
	}

	INLINE void reset() {
		read_pos = write_pos = len = 0;
	}

	INLINE void write(uint8_t b) {
		units[write_pos] = b;
		write_pos = (int)(((size_t)write_pos + 1) % _countof(units));
		len++;
	}

	INLINE T read() {
		T ret = units[read_pos];
		read_pos = (int)(((size_t)read_pos + 1) % _countof(units));
		len--;
		return ret;
	}

	INLINE int skip(int n) {
		int skip_unit = AMP_MIN(n, len);
		if (skip_unit != 0)
		{
			read_pos = (int)(((size_t)read_pos + skip_unit) % _countof(units));
			len -= skip_unit;
		}
		return skip_unit;
	}

	INLINE bool full() {
		return len == _countof(units);
	}

	INLINE bool empty() {
		return len == 0;
	}

	INLINE int size() {
		return ring_unit_size;
	}

	INLINE int length() {
		return len;
	}

	INLINE uint32_t peeku32()
	{
#ifdef _DEBUG
		assert(len >= sizeof(uint32_t));
#endif
		return (units[(size_t)read_pos] << 24 |
			units[((size_t)read_pos + 1) % _countof(units)] << 16 |
			units[((size_t)read_pos + 2) % _countof(units)] << 8 |
			units[((size_t)read_pos + 3) % _countof(units)]);
	}

	INLINE uint64_t peeku64()
	{
#ifdef _DEBUG
		assert(len >= sizeof(uint64_t));
#endif
		return ((uint64_t)units[read_pos] << 56 |
			(uint64_t)units[(read_pos + 1) % _countof(units)] << 48 |
			(uint64_t)units[(read_pos + 2) % _countof(units)] << 40 |
			(uint64_t)units[(read_pos + 3) % _countof(units)] << 32 |
			(uint64_t)units[(read_pos + 4) % _countof(units)] << 24 |
			(uint64_t)units[(read_pos + 5) % _countof(units)] << 16 |
			(uint64_t)units[(read_pos + 6) % _countof(units)] << 8 |
			(uint64_t)units[(read_pos + 7) % _countof(units)]);
	}
};

typedef void* AMLinearRingBuffer;
typedef void(*PTR_FUNC_LRBRWPOINTERRESET)(unsigned int, void*);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Linear Ring Buffer Interface
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
created as a ring buffer. Each write is toward a linear i.e. physically continuous region. In a
case where the write pointer plus a requested write size exceeds the physical end of the buffer, the write
will be directed to the beginning of the buffer
*/
AMLinearRingBuffer	AM_LRB_Create(int buffer_size, int aligned_size=1, PTR_FUNC_LRBRWPOINTERRESET func=NULL, void* context=NULL);

/* The below functions are not thread-safe, please DONT use it under multi-thread scenario. */
unsigned char*		AM_LRB_GetReadPtr(AMLinearRingBuffer ring_buffer, int* ret_read_buffer_len = NULL);
unsigned char*		AM_LRB_GetWritePtr(AMLinearRingBuffer ring_buffer, int* ret_write_buffer_len = NULL);
int					AM_LRB_SkipReadPtr(AMLinearRingBuffer ring_buffer, unsigned int skip_count);
int					AM_LRB_SkipWritePtr(AMLinearRingBuffer ring_buffer, unsigned int skip_count);
/*!	@brief Write the data into the ring buffer.
	@retval RET_CODE_SUCCESS Successfully write the data into the ring buffer
	@retval RET_CODE_OUT_OF_RANGE The buffer size is not enough to hold new written data, and 
								  the enlarged buffer size also hit the upper limitation specified by max_lrb_buf_size
	@retval RET_CODE_OUTOFMEMORY No enough memory
	@remarks When the ring buffer does not have enough space to hold the written data, enlarge it automatically
			 But if the enlarged size is greater than max_lrb_buf_size, this function also return failure. */
int					AM_LRB_Write(AMLinearRingBuffer ring_buffer, uint8_t* chunk_buf, int chunk_size, int max_lrb_buf_size);

/* The below functions are thread-safe, but usage is a little complex. */
unsigned char*		AM_LRB_LockReadPtr(AMLinearRingBuffer ring_buffer, int* ret_read_buffer_len = NULL);
int					AM_LRB_UnlockReadPtr(AMLinearRingBuffer ring_buffer, int skip_forward=0);
unsigned char*		AM_LRB_LockWritePtr(AMLinearRingBuffer ring_buffer, int* ret_write_buffer_len = NULL);
int					AM_LRB_UnlockWritePtr(AMLinearRingBuffer ring_buffer, int skip_forward);

/* General liner ring buffer operation. */
int					AM_LRB_Reform(AMLinearRingBuffer ring_buffer);
int					AM_LRB_Reset(AMLinearRingBuffer ring_buffer);
int					AM_LRB_Resize(AMLinearRingBuffer ring_buffer, int buffer_size);
int					AM_LRB_GetSize(AMLinearRingBuffer ring_buffer);

/* the upper function are thread-safe, but usage is a little complex */

unsigned char*		AM_LRB_DetachBuffer(AMLinearRingBuffer ring_buffer, bool bFreeLRB=false);
AMLinearRingBuffer	AM_LRB_Clone(AMLinearRingBuffer ring_buffer);
void				AM_LRB_Destroy(AMLinearRingBuffer& ring_buffer);

#endif
