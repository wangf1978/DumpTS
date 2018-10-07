/*

MIT License

Copyright (c) 2017 Ravin.Wang(wangf1978@hotmail.com)

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

typedef void* AMLinearRingBuffer;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Linear Ring Buffer Interface
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
created as a ring buffer. Each write is toward a linear i.e. physically continuous region. In a
case where the write pointer plus a requested write size exceeds the physical end of the buffer, the write
will be directed to the beginning of the buffer
*/
AMLinearRingBuffer	AM_LRB_Create(int buffer_size);

/* The below functions are not thread-safe, please DONT use it under multi-thread scenario. */
unsigned char*		AM_LRB_GetReadPtr(AMLinearRingBuffer ring_buffer, int* ret_read_buffer_len = NULL);
unsigned char*		AM_LRB_GetWritePtr(AMLinearRingBuffer ring_buffer, int* ret_write_buffer_len = NULL);
int					AM_LRB_SkipReadPtr(AMLinearRingBuffer ring_buffer, unsigned int skip_count);
int					AM_LRB_SkipWritePtr(AMLinearRingBuffer ring_buffer, unsigned int skip_count);

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
