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
#include "AMRingBuffer.h"
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

struct AM_Linear_Ring_Buffer
{
	unsigned char*		buffer;
	unsigned int		buffer_size;
	unsigned int		read_pos;
	unsigned int		write_pos;
	unsigned int		aligned_size;
	PTR_FUNC_LRBRWPOINTERRESET
						func_lrb_rwptr_reset;
	void*				context;
};

AMLinearRingBuffer AM_LRB_Create(int buffer_size, int aligned_size, PTR_FUNC_LRBRWPOINTERRESET func, void* context)
{
	if (buffer_size <= 0 || aligned_size <= 0)
		return NULL;

	AM_Linear_Ring_Buffer* ptr_ring_buffer = new AM_Linear_Ring_Buffer;
	if (ptr_ring_buffer == NULL)
		return NULL;

	// Adjust the buffer size
	buffer_size = (buffer_size + aligned_size - 1) / aligned_size * aligned_size;
	ptr_ring_buffer->buffer = new unsigned char[buffer_size];
	if (ptr_ring_buffer->buffer == NULL)
	{
		delete ptr_ring_buffer;
		return NULL;
	}

	ptr_ring_buffer->buffer_size = buffer_size;
	ptr_ring_buffer->read_pos = ptr_ring_buffer->write_pos = 0;

	ptr_ring_buffer->aligned_size = aligned_size;
	ptr_ring_buffer->func_lrb_rwptr_reset = func;
	ptr_ring_buffer->context = context;

	return (AMLinearRingBuffer)ptr_ring_buffer;
}

int AM_LRB_Resize(AMLinearRingBuffer ring_buffer, int buffer_size)
{
	if (ring_buffer == NULL)
		return RET_CODE_INVALID_PARAMETER;

	AM_Linear_Ring_Buffer* ptr_ring_buffer = (AM_Linear_Ring_Buffer*)ring_buffer;

	if ((int)ptr_ring_buffer->buffer_size == buffer_size)
		return RET_CODE_SUCCESS;

	// At present, only support expand buffer size, does not support shrink ring buffer
	if (buffer_size < 0 || buffer_size < (int)ptr_ring_buffer->buffer_size)
		return RET_CODE_INVALID_PARAMETER;

	buffer_size = (buffer_size + ptr_ring_buffer->aligned_size - 1) / ptr_ring_buffer->aligned_size*ptr_ring_buffer->aligned_size;
	unsigned char* pExpandedBuf = new unsigned char[buffer_size];
	if (pExpandedBuf == NULL)
		return RET_CODE_OUTOFMEMORY;
	
	memcpy(pExpandedBuf, ptr_ring_buffer->buffer, ptr_ring_buffer->buffer_size);
	delete [] ptr_ring_buffer->buffer;
	ptr_ring_buffer->buffer = pExpandedBuf;
	ptr_ring_buffer->buffer_size = (unsigned int)buffer_size;

	return RET_CODE_SUCCESS;
}

int AM_LRB_GetSize(AMLinearRingBuffer ring_buffer)
{
	if (ring_buffer == NULL)
		return RET_CODE_INVALID_PARAMETER;

	AM_Linear_Ring_Buffer* ptr_ring_buffer = (AM_Linear_Ring_Buffer*)ring_buffer;

	return (int)ptr_ring_buffer->buffer_size;
}

unsigned char* AM_LRB_GetReadPtr(AMLinearRingBuffer ring_buffer, int* ret_read_buffer_len)
{
	if (ring_buffer == NULL)
		return NULL;

	AM_Linear_Ring_Buffer* ptr_ring_buffer = (AM_Linear_Ring_Buffer*)ring_buffer;
	if (ptr_ring_buffer->read_pos == ptr_ring_buffer->write_pos)
		return NULL;

	AMP_SAFEASSIGN(ret_read_buffer_len, (ptr_ring_buffer->write_pos - ptr_ring_buffer->read_pos));
	return ptr_ring_buffer->buffer + ptr_ring_buffer->read_pos;
}

unsigned char* AM_LRB_GetWritePtr(AMLinearRingBuffer ring_buffer, int* ret_write_buffer_len)
{
	if (ring_buffer == NULL)
		return NULL;

	AM_Linear_Ring_Buffer* ptr_ring_buffer = (AM_Linear_Ring_Buffer*)ring_buffer;
	if (ptr_ring_buffer->write_pos == ptr_ring_buffer->buffer_size)
		return NULL;

	AMP_SAFEASSIGN(ret_write_buffer_len, (ptr_ring_buffer->buffer_size - ptr_ring_buffer->write_pos));
	return ptr_ring_buffer->buffer + ptr_ring_buffer->write_pos;
}

int AM_LRB_SkipReadPtr(AMLinearRingBuffer ring_buffer, unsigned int skip_count)
{
	if (ring_buffer == NULL)
		return RET_CODE_INVALID_PARAMETER;

	// Skip count should not exceed MAX_UINT
	if (skip_count > INT32_MAX)
		skip_count = INT32_MAX;

	int actual_skip_count = skip_count;
	AM_Linear_Ring_Buffer* ptr_ring_buffer = (AM_Linear_Ring_Buffer*)ring_buffer;
	if (ptr_ring_buffer->read_pos + skip_count >= ptr_ring_buffer->write_pos)
	{
		actual_skip_count = (ptr_ring_buffer->write_pos - ptr_ring_buffer->read_pos);
		ptr_ring_buffer->read_pos = ptr_ring_buffer->write_pos = 0;
	}
	else
		ptr_ring_buffer->read_pos += skip_count;

	return actual_skip_count;
}

int AM_LRB_SkipWritePtr(AMLinearRingBuffer ring_buffer, unsigned int skip_count)
{
	if (ring_buffer == NULL)
		return RET_CODE_INVALID_PARAMETER;

	// Skip count should not exceed MAX_UINT
	if (skip_count > INT32_MAX)
		skip_count = INT32_MAX;

	int actual_skip_count = (int)skip_count;
	AM_Linear_Ring_Buffer* ptr_ring_buffer = (AM_Linear_Ring_Buffer*)ring_buffer;
	if (ptr_ring_buffer->write_pos + skip_count >= ptr_ring_buffer->buffer_size)
		actual_skip_count = (int)(ptr_ring_buffer->buffer_size - ptr_ring_buffer->write_pos);

	if (actual_skip_count > 0)
		ptr_ring_buffer->write_pos += actual_skip_count;

	return actual_skip_count;
}

int AM_LRB_Write(AMLinearRingBuffer ring_buffer, uint8_t* chunk_buf, int chunk_size, int max_lrb_buf_size)
{
	int nRet = RET_CODE_SUCCESS;
	int nWriteBufLen = 0;
	uint8_t* pWriteBuf = AM_LRB_GetWritePtr(ring_buffer, &nWriteBufLen);
	if (nWriteBufLen <= 0 || pWriteBuf == nullptr || nWriteBufLen < chunk_size)
	{
		// Try to reform and enlarge the buffer.
		AM_LRB_Reform(ring_buffer);
		pWriteBuf = AM_LRB_GetWritePtr(ring_buffer, &nWriteBufLen);
		if (nWriteBufLen == 0 || pWriteBuf == nullptr || nWriteBufLen < chunk_size)
		{
			// Enlarge the ring buffer size
			AM_Linear_Ring_Buffer* ptr_ring_buffer = (AM_Linear_Ring_Buffer*)ring_buffer;
			int nCurSize = AM_LRB_GetSize(ring_buffer);
			int nExpandSize = (chunk_size + ptr_ring_buffer->aligned_size - 1) / ptr_ring_buffer->aligned_size * ptr_ring_buffer->aligned_size;
			if (nCurSize + nExpandSize > max_lrb_buf_size)
			{
				printf("The input buffer is too huge(size: %d, max: %d), can't be supported now.\n", nCurSize + nExpandSize, max_lrb_buf_size);
				return RET_CODE_OUT_OF_RANGE;
			}

			nCurSize += nExpandSize;

			printf("Try to resize the linear ring buffer size to %d bytes.\n", nCurSize);
			if (AMP_FAILED(nRet = AM_LRB_Resize(ring_buffer, nCurSize)))
				return nRet;

			pWriteBuf = AM_LRB_GetWritePtr(ring_buffer, &nWriteBufLen);
			if (nWriteBufLen == 0 || pWriteBuf == nullptr)
				return RET_CODE_OUTOFMEMORY;
		}
	}

	memcpy(pWriteBuf, chunk_buf, chunk_size);
	AM_LRB_SkipWritePtr(ring_buffer, chunk_size);

	return RET_CODE_SUCCESS;
}

unsigned char* AM_LRB_LockReadPtr(AMLinearRingBuffer ring_buffer, int* ret_read_buffer_len)
{
	UNREFERENCED_PARAMETER(ring_buffer);
	UNREFERENCED_PARAMETER(ret_read_buffer_len);
	return NULL;
}

int AM_LRB_UnlockReadPtr(AMLinearRingBuffer ring_buffer, int skip_forward)
{
	UNREFERENCED_PARAMETER(ring_buffer);
	UNREFERENCED_PARAMETER(skip_forward);
	return RET_CODE_ERROR_NOTIMPL;
}

unsigned char* AM_LRB_LockWritePtr(AMLinearRingBuffer ring_buffer, int* ret_write_buffer_len)
{
	UNREFERENCED_PARAMETER(ring_buffer);
	UNREFERENCED_PARAMETER(ret_write_buffer_len);
	return NULL;
}

int AM_LRB_UnlockWritePtr(AMLinearRingBuffer ring_buffer, int skip_forward)
{
	UNREFERENCED_PARAMETER(ring_buffer);
	UNREFERENCED_PARAMETER(skip_forward);
	return RET_CODE_ERROR_NOTIMPL;
}

int AM_LRB_Reform(AMLinearRingBuffer ring_buffer)
{
	AM_Linear_Ring_Buffer* ptr_ring_buffer = (AM_Linear_Ring_Buffer*)ring_buffer;
	if (ptr_ring_buffer->read_pos == 0)
		return RET_CODE_SUCCESS;

	int mem_move_count = (ptr_ring_buffer->write_pos - ptr_ring_buffer->read_pos);
	if (mem_move_count > 0)
		memmove(ptr_ring_buffer->buffer, ptr_ring_buffer->buffer + ptr_ring_buffer->read_pos, mem_move_count);

	unsigned int offset = ptr_ring_buffer->read_pos;

	ptr_ring_buffer->read_pos = 0;
	ptr_ring_buffer->write_pos = mem_move_count;

	if (ptr_ring_buffer->func_lrb_rwptr_reset != NULL)
		ptr_ring_buffer->func_lrb_rwptr_reset(offset, ptr_ring_buffer->context);

	return RET_CODE_SUCCESS;
}

int	AM_LRB_Reset(AMLinearRingBuffer ring_buffer)
{
	if (ring_buffer == NULL)
		return RET_CODE_INVALID_PARAMETER;

	AM_Linear_Ring_Buffer* ptr_ring_buffer = (AM_Linear_Ring_Buffer*)ring_buffer;

	ptr_ring_buffer->read_pos = ptr_ring_buffer->write_pos = 0;
	return RET_CODE_SUCCESS;
}

unsigned char* AM_LRB_DetachBuffer(AMLinearRingBuffer ring_buffer, bool bFreeLRB)
{
	if (ring_buffer == NULL)
		return NULL;

	AM_Linear_Ring_Buffer* ptr_ring_buffer = (AM_Linear_Ring_Buffer*)ring_buffer;
	unsigned char* pRetBuf = ptr_ring_buffer->buffer;
	if (bFreeLRB){
		ptr_ring_buffer->buffer = NULL;
		ptr_ring_buffer->buffer_size = 0;
		ptr_ring_buffer->read_pos = ptr_ring_buffer->write_pos = 0;
	}
	else
	{
		unsigned char* pNewBuf = new unsigned char[ptr_ring_buffer->buffer_size];
		memcpy(pNewBuf, pRetBuf, ptr_ring_buffer->buffer_size);
		ptr_ring_buffer->buffer = pNewBuf;
	}

	return pRetBuf;
}

AMLinearRingBuffer AM_LRB_Clone(AMLinearRingBuffer ring_buffer)
{
	if (ring_buffer == NULL)
		return NULL;

	AM_Linear_Ring_Buffer* ptr_ring_buffer = (AM_Linear_Ring_Buffer*)ring_buffer;

	AM_Linear_Ring_Buffer* ptr_new_ring_buffer = new AM_Linear_Ring_Buffer;
	if (ptr_new_ring_buffer == NULL)
		return NULL;

	ptr_new_ring_buffer->buffer = new unsigned char[ptr_ring_buffer->buffer_size];
	if (ptr_new_ring_buffer->buffer == NULL){
		delete ptr_new_ring_buffer;
		return NULL;
	}

	ptr_new_ring_buffer->buffer_size = ptr_ring_buffer->buffer_size;
	ptr_new_ring_buffer->read_pos = ptr_ring_buffer->read_pos;
	ptr_new_ring_buffer->write_pos = ptr_ring_buffer->write_pos;
	return (AMLinearRingBuffer)ptr_new_ring_buffer;
}

void AM_LRB_Destroy(AMLinearRingBuffer& ring_buffer)
{
	if (ring_buffer == NULL)
		return;

	AM_Linear_Ring_Buffer* ptr_ring_buffer = (AM_Linear_Ring_Buffer*)ring_buffer;

	ring_buffer = NULL;

	AMP_SAFEDELA(ptr_ring_buffer->buffer);
	delete ptr_ring_buffer;
	return;
}
