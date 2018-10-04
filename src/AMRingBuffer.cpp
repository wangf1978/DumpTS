#include "StdAfx.h"
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
};

AMLinearRingBuffer AM_LRB_Create(int buffer_size)
{
	if (buffer_size <= 0)
		return NULL;

	AM_Linear_Ring_Buffer* ptr_ring_buffer = new AM_Linear_Ring_Buffer;
	if (ptr_ring_buffer == NULL)
		return NULL;

	ptr_ring_buffer->buffer = new unsigned char[buffer_size];
	if (ptr_ring_buffer->buffer == NULL)
	{
		delete ptr_ring_buffer;
		return NULL;
	}

	ptr_ring_buffer->buffer_size = buffer_size;
	ptr_ring_buffer->read_pos = ptr_ring_buffer->write_pos = 0;

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
	if (skip_count > INT_MAX)
		skip_count = INT_MAX;

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
	if (skip_count > INT_MAX)
		skip_count = INT_MAX;

	int actual_skip_count = (int)skip_count;
	AM_Linear_Ring_Buffer* ptr_ring_buffer = (AM_Linear_Ring_Buffer*)ring_buffer;
	if (ptr_ring_buffer->write_pos + skip_count >= ptr_ring_buffer->buffer_size)
		actual_skip_count = (int)(ptr_ring_buffer->buffer_size - ptr_ring_buffer->write_pos);

	if (actual_skip_count > 0)
		ptr_ring_buffer->write_pos += actual_skip_count;

	return actual_skip_count;
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

	ptr_ring_buffer->read_pos = 0;
	ptr_ring_buffer->write_pos = mem_move_count;

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

unsigned char* AM_LRB_DetachBuffer(AMLinearRingBuffer ring_buffer, BOOL bFreeLRB)
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
