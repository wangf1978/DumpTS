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
#include "AMBitStream.h"
#include "AMRingBuffer.h"
#include <exception>
#include "AMException.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4201)
#endif

#ifndef MAXUINT32
#define MAXUINT32   ((UINT32)~((UINT32)0))
#endif
//#define MAXINT32    ((INT32)(MAXUINT32 >> 1))
//#define MININT32    ((INT32)~MAXINT32)

#ifndef MAXUINT64
#define MAXUINT64   ((UINT64)~((UINT64)0))
#endif
//#define MAXINT64    ((INT64)(MAXUINT64 >> 1))
//#define MININT64    ((INT64)~MAXINT64)

#define CUR_RINGBYTES_24BITS()		((bst_data->cursor.nal_ring_bytes.units[(bst_data->cursor.nal_ring_bytes.read_pos)]<<16) | \
									 (bst_data->cursor.nal_ring_bytes.units[(bst_data->cursor.nal_ring_bytes.read_pos + 1) % NAL_EMULATION_PREVENTION_BYTELEN] << 8) | \
									 (bst_data->cursor.nal_ring_bytes.units[(bst_data->cursor.nal_ring_bytes.read_pos + 2) % NAL_EMULATION_PREVENTION_BYTELEN]))

#define CUR_LINEARBYTES_24BITS()	(((*bst_data->cursor.p) << 16) | ((*(bst_data->cursor.p + 1)) << 8) | (*(bst_data->cursor.p + 2)))

#define CURBITS_TO_MODBITS(cursor)	((cursor.curbits << cursor.bits_left) | ((cursor.curbits >> ((sizeof(CURBITS_TYPE)<<3) - cursor.bits_left))&CURBITS_MASK(cursor.bits_left)))
#define MODBITS_TO_CURBITS(cursor)	((cursor.modbits << cursor.bits_left) | ((cursor.curbits&(CURBITS_MASK(cursor.bits_left)))))

enum AM_BST_TYPE{
	BST_TYPE_EXTERNAL_BUF,
	BST_TYPE_INNER_BUF,
	BST_TYPE_RING_CHUNK
};

//#define _USE_64BIT_CACHE

#ifdef _USE_64BIT_CACHE
#define CURBITS_TYPE				uint64_t
#define CURBITS_VALUE(p)			ENDIANUINT64(*(CURBITS_TYPE*)p)
#define CURBITS_VALUE1(n)			ENDIANUINT64(n)
#define CURBITS_MASK(bits)			bits == 64 ? MAXUINT64 : ~(MAXUINT64 << bits)
#else
#define CURBITS_TYPE				uint32_t
#define CURBITS_VALUE(p)			ENDIANULONG(*(CURBITS_TYPE*)p)
#define CURBITS_VALUE1(n)			ENDIANULONG(n)
#define CURBITS_MASK(bits)			(bits == 32 ? MAXUINT32 : ~(MAXUINT32 << bits))
#endif

#define CURBITS_BYTECOUNT(cursor)	((cursor.p_end >= cursor.p + sizeof(CURBITS_TYPE))?sizeof(CURBITS_TYPE):(cursor.p_end - cursor.p))

#define NAL_EMULATION_PREVENTION_BYTES		0x000003
#define NAL_EMULATION_PREVENTION_BYTELEN	3

struct AM_BST_DATA{

	// bit-stream cursor information, which is used to get/peek value from bit-stream
	struct AM_BST_CURSOR
	{
		uint8_t*			p_start;					// bitstream start pointer
		uint8_t*			p;							// bitstream current pointer
		uint8_t*			p_end;						// bitstream end pointer, [start_pointer, end_pointer - exclude_bits) contains valid data
		uint8_t				exclude_bits;				// how many last bits are excluded in the last byte *(end_pointer-1)
		uint8_t				reserved[3];
		int					buf_size;					// when type is INNER_BUF, it is the underlying buffer size
		int					start_offset;
		int					bits_left;					// How many bits left in "curbits" and "modbits"
		CURBITS_TYPE		curbits;					// It is used for read/get operation
		CURBITS_TYPE		modbits;					// Only available for write mode, save the changed bits, and will be updated to buffer after skip the current cache unit

		CAMRingUnits<uint8_t, NAL_EMULATION_PREVENTION_BYTELEN>
							nal_ring_bytes;				// Store nal_unit ring bytes

		CAMRingUnits<uint8_t, 16>
							chunk_unread_bytes;			// the unread bytes in the previous chunk(s)

		AM_BST_CURSOR() : p_start(NULL), p(NULL), p_end(NULL), exclude_bits(0), buf_size(0), start_offset(0), bits_left(0), curbits(0), modbits(0) {
			memset(reserved, 0, sizeof(reserved));
		}
	};

	AM_BST_TYPE			type;							// bitstream type
	int					access_mode;					// the access mode of current bit-stream
	union
	{
		struct
		{
			AMBstFillBuffer		func_ptr_fillbuffer;	// function pointer to fill the buffer
			void*				user_data;				// user data passed to func_ptr_fillbuffer
		};
#if 0
		AMRingChunk			rchunk;						// ring chunk of current bit-stream
#else
		void*				rchunk;
#endif
	};

	AMBstRBSPType		rbsp_type;						// At default, BST_RBSP_SEQUENCE type is used

	AM_BST_CURSOR		cursor;							// the current cursor information.
	AM_BST_CURSOR		save_point;						// the save-point cursor information which is mainly used for peek restore

	AM_BST_DATA(AM_BST_TYPE bst_type, int bst_access_mode)
		: type(bst_type)
		, access_mode(bst_access_mode)
		, func_ptr_fillbuffer(nullptr)
		, user_data(nullptr)
		, rbsp_type(BST_RBSP_SEQUENCE){
	}

	void CleanSavePoint() {
		save_point.p = NULL;
	}
};

INLINE int GetAllLeftBits(AM_BST_DATA* bst_data)
{
	int bits_left = bst_data->cursor.p_end <= bst_data->cursor.p ? 0 : (((int)(bst_data->cursor.p_end - bst_data->cursor.p)) << 3);
	if (bst_data->rbsp_type == BST_RBSP_NAL_UNIT)
	{
		int bits_left_in_cache = bst_data->cursor.bits_left + bst_data->cursor.nal_ring_bytes.length();
		return bits_left + bits_left_in_cache;
	}
	else if(bst_data->rbsp_type == BST_RBSP_SEQUENCE)
	{
		return bits_left <= bst_data->cursor.exclude_bits 
			? 0
			: (bits_left < (int)(sizeof(CURBITS_TYPE)<<3) 
				? (bst_data->cursor.bits_left - (int)bst_data->cursor.exclude_bits) 
				: (bits_left - (int)(sizeof(CURBITS_TYPE)<<3) + bst_data->cursor.bits_left - bst_data->cursor.exclude_bits));
	}

	return -1;
}

void* AMBst_Alloc(AMBst bst, int cbSize)
{
	UNREFERENCED_PARAMETER(bst);
	unsigned char* p = new unsigned char[cbSize];
	return p;
}

void AMBst_Free(AMBst bst, void* buf)
{
	UNREFERENCED_PARAMETER(bst);
	if (buf != NULL)
		delete[] (unsigned char*)buf;
}

AMBst AMBst_CreateFromBuffer(uint8_t* pBuffer, int cbSize, int access_mode)
{
	if (pBuffer == NULL || cbSize <= 0)
		return NULL;

	AM_BST_DATA* bst_data = new AM_BST_DATA(BST_TYPE_EXTERNAL_BUF, access_mode);
	if (bst_data == NULL)
		return NULL;

	bst_data->rchunk = NULL;
	bst_data->func_ptr_fillbuffer = NULL;
	bst_data->user_data = NULL;

	bst_data->cursor.start_offset = ((intptr_t)pBuffer & 3);
	bst_data->cursor.p = bst_data->cursor.p_start = (uint8_t*)pBuffer - bst_data->cursor.start_offset;
	bst_data->cursor.p_end = pBuffer + cbSize;
	bst_data->cursor.bits_left = (int)((CURBITS_BYTECOUNT(bst_data->cursor) - bst_data->cursor.start_offset) << 3);

	if (bst_data->cursor.p + sizeof(CURBITS_TYPE) <= bst_data->cursor.p_end)
	{
		bst_data->cursor.curbits = CURBITS_VALUE(bst_data->cursor.p);
	}
	else
	{
		bst_data->cursor.curbits = 0;
		for (uint8_t* pbyte = bst_data->cursor.p; pbyte < bst_data->cursor.p_end; pbyte++) {
			bst_data->cursor.curbits <<= 8;
			bst_data->cursor.curbits |= *(uint8_t*)pbyte;
		}
	}

	bst_data->cursor.modbits = bst_data->cursor.curbits;

	return (AMBst)bst_data;
}

AMBst AMBst_Create()
{
	AM_BST_DATA* bst_data = new AM_BST_DATA(BST_TYPE_EXTERNAL_BUF, 0);
	if (bst_data == NULL)
		return NULL;

	bst_data->rchunk = NULL;
	bst_data->func_ptr_fillbuffer = NULL;
	bst_data->user_data = NULL;

	bst_data->cursor.start_offset = 0;
	bst_data->cursor.p = bst_data->cursor.p_start = NULL;
	bst_data->cursor.p_end = NULL;
	bst_data->cursor.bits_left = 0;

	bst_data->cursor.curbits = 0;
	bst_data->cursor.modbits = 0;

	return (AMBst)bst_data;
}

int AMBst_AttachBuffer(AMBst bst, uint8_t* pBuffer, int cbSize, int access_mode)
{
	if (pBuffer == NULL || cbSize <= 0)
		return RET_CODE_INVALID_PARAMETER;

	AM_BST_DATA* bst_data = (AM_BST_DATA*)bst;
	if (bst_data == NULL)
		return RET_CODE_INVALID_PARAMETER;

	bst_data->type = BST_TYPE_EXTERNAL_BUF;

	bst_data->rchunk = NULL;
	bst_data->func_ptr_fillbuffer = NULL;
	bst_data->user_data = NULL;

	bst_data->cursor.start_offset = ((intptr_t)pBuffer & 3);
	bst_data->cursor.p = bst_data->cursor.p_start = (uint8_t*)pBuffer - bst_data->cursor.start_offset;
	bst_data->cursor.p_end = pBuffer + cbSize;
	bst_data->cursor.bits_left = (int)((CURBITS_BYTECOUNT(bst_data->cursor) - bst_data->cursor.start_offset) << 3);
	bst_data->access_mode = access_mode;

	bst_data->cursor.curbits = CURBITS_VALUE(bst_data->cursor.p);
	bst_data->cursor.modbits = bst_data->cursor.curbits;

	return RET_CODE_SUCCESS;
}

void AMBst_DetachBuffer(AMBst bst)
{
	AM_BST_DATA* bst_data = (AM_BST_DATA*)bst;
	if (bst_data == NULL)
		return;

	// Can only detach the bitstream with the external buffer
	if (bst_data->type != BST_TYPE_EXTERNAL_BUF)
		return;

	bst_data->cursor.start_offset = 0;
	bst_data->cursor.p = bst_data->cursor.p_start = NULL;
	bst_data->cursor.p_end = NULL;
	bst_data->cursor.bits_left = 0;

	return;
}

AMBst AMBst_CreateFromCallback(AMBstFillBuffer funptrFillBuffer, void* pUserData, int cbUnderlyingBufSize, int access_mode)
{
	if (cbUnderlyingBufSize < (int)sizeof(int64_t) || funptrFillBuffer == NULL)
		return NULL;

	if ((access_mode&BST_MODE_READ_WRITE) == BST_MODE_READ_WRITE)
	{
		printf("[Bitstream] BST_MODE_WRITE_READ access mode is not supported for bitstream with calling external function to fill buffer!\n");
		return NULL;
	}

	AM_BST_DATA* bst_data = new AM_BST_DATA(BST_TYPE_INNER_BUF, access_mode);
	if (bst_data == NULL)
		return NULL;

	bst_data->rchunk = NULL;
	bst_data->func_ptr_fillbuffer = funptrFillBuffer;
	bst_data->user_data = pUserData;

	bst_data->cursor.p_start = new uint8_t[(cbUnderlyingBufSize + 3)&0xFFFFFFFC];
	if (bst_data->cursor.p_start == NULL){
		delete bst_data;
		return NULL;
	}

	bst_data->cursor.start_offset = 0;
	if (access_mode == BST_MODE_WRITE)
	{
		bst_data->cursor.p_end = bst_data->cursor.p_start + cbUnderlyingBufSize;
		bst_data->cursor.bits_left = AMP_MIN((int)sizeof(CURBITS_TYPE), cbUnderlyingBufSize) << 3;
	}
	else
	{
		bst_data->cursor.p_end = bst_data->cursor.p_start;
		bst_data->cursor.bits_left = 0;
	}
	bst_data->cursor.p = bst_data->cursor.p_start;
	bst_data->cursor.buf_size = cbUnderlyingBufSize;
	bst_data->cursor.curbits = bst_data->cursor.modbits = 0;
	
	return (AMBst)bst_data;
}

#if 0
AMBst AMBst_CreateFromRingBuffer(AMRingChunk pRingChunk, int access_mode)
{
	if (IS_INVALID_HANDLE((HANDLE*)pRingChunk))
		return NULL;

	if (access_mode&BST_MODE_WRITE)
	{
		AMP_Warning(_T("[Bitstream] BST_MODE_WRITE access mode is not supported for bitstream binding with ring buffer!\n"));
		return NULL;
	}

	AM_BST_DATA* bst_data = new AM_BST_DATA(BST_TYPE_RING_CHUNK, access_mode);
	if (bst_data == NULL)
		return NULL;

	bst_data->rchunk = pRingChunk;

	bst_data->cursor.start_offset = 0;
	bst_data->cursor.p = bst_data->cursor.p_start = bst_data->cursor.p_end = NULL;
	bst_data->cursor.bits_left = 0;
	if((bst_data->cursor.buf_size = AM_RingChunk_GetChunkSize(pRingChunk)) < sizeof(CURBITS_TYPE))
	{
		delete bst_data;
		return NULL;
	}

	bst_data->cursor.curbits = bst_data->cursor.modbits = 0;

	return (AMBst)bst_data;
}
#endif

AMBst AMBst_Subset(AMBst bst, int nBits)
{
	AM_BST_DATA* bst_data = (AM_BST_DATA*)bst;
	if (bst_data == NULL)
		return NULL;

	// At present, only support bitstream type: BST_TYPE_EXTERNAL_BUF and rbsp type: BST_RBSP_SEQUENCE
	if (bst_data->type != BST_TYPE_EXTERNAL_BUF && bst_data->rbsp_type != BST_RBSP_SEQUENCE)
		return NULL;

	AM_BST_DATA* sub_bst_data = new AM_BST_DATA(bst_data->type, bst_data->access_mode);
	if (bst_data->type == BST_TYPE_EXTERNAL_BUF)
	{
		if (nBits > GetAllLeftBits(bst_data))
			throw AMException(RET_CODE_INVALID_PARAMETER, _T("nBits parameter exceed the all left bits in current bitstream"));

		sub_bst_data->rchunk = NULL;
		sub_bst_data->func_ptr_fillbuffer = NULL;
		sub_bst_data->user_data = NULL;

		sub_bst_data->cursor = bst_data->cursor;
		sub_bst_data->rbsp_type = bst_data->rbsp_type;
		
		if (sub_bst_data->rbsp_type == BST_RBSP_SEQUENCE)
		{
			// Check whether to lie at the last CURBITS
			if (sub_bst_data->cursor.bits_left >= nBits)
			{
				/*
				|--------|--------|--------|--------|
				p          r          
				           ^++++++ bits_left +++++++^
						   ^+++++nBits+++++++++^
				*/
				bool bLastCurBits = sub_bst_data->cursor.p + sizeof(CURBITS_TYPE) < sub_bst_data->cursor.p_end ? false : true;
				if (!bLastCurBits)
				{
					sub_bst_data->cursor.p_end = sub_bst_data->cursor.p + sizeof(CURBITS_TYPE);
					sub_bst_data->cursor.exclude_bits = (uint8_t)(sub_bst_data->cursor.bits_left - nBits);
				}
				else
				{
					sub_bst_data->cursor.exclude_bits = (uint8_t)(sub_bst_data->cursor.exclude_bits + sub_bst_data->cursor.bits_left - nBits);
				}
			}
			else
			{
				/*
				|--------|--------|--------|--------|.......|--------|--------|--------|--------|
				p          r
				           ^++++++ bits_left +++++++^
				           ^+++++nBits++++++++++++++++++++++++++++++^
				*/
				int nLeftBits = nBits - sub_bst_data->cursor.bits_left;
				sub_bst_data->cursor.p_end = sub_bst_data->cursor.p + sizeof(CURBITS_TYPE) + ((nLeftBits + 7) >> 8);
				sub_bst_data->cursor.exclude_bits = (uint8_t)(((nLeftBits + 7)&0xFFFFFFF8) - nLeftBits);
			}
		}
		else if (sub_bst_data->rbsp_type == BST_RBSP_NAL_UNIT)
		{
		}

		return (AMBst)sub_bst_data;
	}

	return NULL;
}

void _Advance_InCacheBits(AM_BST_DATA* bst_data, int n, AMBstBitFillMode bit_fill_mode = BST_BIT_FILL_KEEP)
{
	bst_data->cursor.bits_left -= n;

	if (!(bst_data->access_mode&BST_MODE_WRITE) /*|| (bst_data->cursor.modbits == bst_data->cursor.curbits && bit_fill_mode == BST_BIT_FILL_KEEP)*/)
		return;

	// curbits -> modbits
	CURBITS_TYPE nValMask = CURBITS_MASK(n);

	//AMP_Assert(bst_data->type == BST_TYPE_EXTERNAL_BUF);
	switch (bit_fill_mode) {
	case BST_BIT_FILL_KEEP:
		if (n < (int)(sizeof(CURBITS_TYPE) << 3))
			bst_data->cursor.modbits <<= n;
		else
			bst_data->cursor.modbits = 0;
		bst_data->cursor.modbits |= (bst_data->cursor.curbits >> bst_data->cursor.bits_left)&nValMask;
		break;
	case BST_BIT_FILL_ONE:
		if (n < (int)(sizeof(CURBITS_TYPE) << 3))
			bst_data->cursor.modbits <<= n;
		else
			bst_data->cursor.modbits = 0;
		bst_data->cursor.modbits |= nValMask;
		break;
	case BST_BIT_FILL_ZERO:
		if (n < (int)(sizeof(CURBITS_TYPE) << 3))
			bst_data->cursor.modbits <<= n;
		else
			bst_data->cursor.modbits = 0;
		break;
	}
}

/*!	@brief Update the current/modified bits and bits_left 
	@remark it should not update the other fields except curbits, modbits and bits_left */
inline void _UpdateCurBits(AM_BST_DATA* bst_data, bool bEos=false, bool bFlush=false)
{
	int skip_remaining_bits = 0;
	
	/*
		Reach the end of inner buffer
		If the current bitstream is writable only, call the callback function to flush the current buffer, and then restart from the beginning
	*/
	if (bst_data->type == BST_TYPE_INNER_BUF && bst_data->access_mode == BST_MODE_WRITE && (
		bst_data->cursor.p  > bst_data->cursor.p_start || (
		bst_data->cursor.p == bst_data->cursor.p_start && bst_data->cursor.bits_left < (int)(CURBITS_BYTECOUNT(bst_data->cursor)<<3))) && (bst_data->cursor.p == bst_data->cursor.p_end || bFlush))
	{
		/*
		/-----p_start            /----- p
		|_______________________/_________________________________________|
		                        |     |<- left bits ->|
		|<--------cbSubmit-------->|    \_____________________nProcessed1
		|<----- cbCommit----->|    \__pProcssed
		                      |<------->|
		                            \___________cbUnprocessed	
		*/

		int nProcessedCompletelyBits = 0;
		int AvailableBytes = (int)((bst_data->cursor.p_end >= bst_data->cursor.p + sizeof(CURBITS_TYPE)) ? sizeof(CURBITS_TYPE) : (bst_data->cursor.p_end - bst_data->cursor.p));
		if ((AvailableBytes << 3) > bst_data->cursor.bits_left)
			nProcessedCompletelyBits = (AvailableBytes << 3) - bst_data->cursor.bits_left;

		uint8_t* pProcessed = bst_data->cursor.p + nProcessedCompletelyBits / 8;
		uint8_t* pProcessed1 = bst_data->cursor.p + (nProcessedCompletelyBits + 7) / 8;

		int cbSubmitted = (int)(pProcessed - bst_data->cursor.p_start);

		/* For the bits which are not enough for 1 byte, does not feed back to the external callback. */
		int cbCommit = bst_data->func_ptr_fillbuffer(bst_data->cursor.p_start, (int)(pProcessed - bst_data->cursor.p_start), bEos, bst_data->user_data);
		if (cbCommit <= 0 || cbCommit > cbSubmitted)
			return;

		int cbUnprocessed = (int)(pProcessed1 - (bst_data->cursor.p_start + cbCommit));

		// Move the unprocessed bytes and bits to the beginning of the inner buffer
		if (cbUnprocessed > 0)
			memcpy(bst_data->cursor.p_start, bst_data->cursor.p_start + cbCommit, cbUnprocessed);

		// Find an aligned pointer
		bst_data->cursor.p = bst_data->cursor.p_start + cbUnprocessed / sizeof(CURBITS_TYPE) * sizeof(CURBITS_TYPE);
		
		AvailableBytes = (bst_data->cursor.p_end >= bst_data->cursor.p + sizeof(CURBITS_TYPE)) ? (int)sizeof(CURBITS_TYPE) : (int)(bst_data->cursor.p_end - bst_data->cursor.p);
		int cbLeftUnprocessedBits = (cbUnprocessed % sizeof(CURBITS_TYPE)) << 3;
		int cbActualUnprocessedBits = cbLeftUnprocessedBits - ((nProcessedCompletelyBits % 8 == 0) ? 0 : (8 - (nProcessedCompletelyBits % 8)));

		if (cbActualUnprocessedBits < 0)
		{
			bst_data->cursor.p -= sizeof(CURBITS_TYPE);
			skip_remaining_bits = sizeof(CURBITS_TYPE) + cbActualUnprocessedBits;
		}
		else
			skip_remaining_bits = cbActualUnprocessedBits;
	}

	if (bst_data->rbsp_type == BST_RBSP_SEQUENCE)
	{
		if (bst_data->cursor.p + sizeof(CURBITS_TYPE) <= bst_data->cursor.p_end)
		{
			bst_data->cursor.curbits = CURBITS_VALUE(bst_data->cursor.p);
			bst_data->cursor.bits_left = sizeof(CURBITS_TYPE) * 8;
		}
		else
		{
			bst_data->cursor.curbits = 0;
			for (uint8_t* pbyte = bst_data->cursor.p; pbyte < bst_data->cursor.p_end; pbyte++) {
				bst_data->cursor.curbits <<= 8;
				bst_data->cursor.curbits |= *(uint8_t*)pbyte;
				bst_data->cursor.bits_left += 8;
			}
		}
	}
	else if (bst_data->rbsp_type == BST_RBSP_NAL_UNIT)
	{
		// Under rbsp_type: RBSP_NAL_UNIT, start code should NOT be hit, if it hit
		if((bst_data->type == BST_TYPE_INNER_BUF || bst_data->type == BST_TYPE_RING_CHUNK))
		{
			if (bst_data->cursor.nal_ring_bytes.full())
			{
				// Update the current bits, and ring bytes layout
				if (NAL_EMULATION_PREVENTION_BYTES == CUR_RINGBYTES_24BITS())
				{
					bst_data->cursor.curbits = (NAL_EMULATION_PREVENTION_BYTES) >> 8;
					bst_data->cursor.bits_left = 16;

					// The available bits has been read, reset the current ring bytes;
					bst_data->cursor.nal_ring_bytes.reset();
				}
				else
				{
					bst_data->cursor.curbits = bst_data->cursor.nal_ring_bytes.read();
					bst_data->cursor.bits_left = 8;
				}
			}
			else if (bEos)
			{
				// There are remaining bytes in current NAL ring bytes, and less than 3 bytes, since EOS is reached, use the left bytes.
				AMP_Assert(bst_data->cursor.p_end == bst_data->cursor.p);
				if (!bst_data->cursor.nal_ring_bytes.empty())
				{
					bst_data->cursor.curbits = bst_data->cursor.nal_ring_bytes.read();
					bst_data->cursor.bits_left = 8;
				}
			}
		}
		else if (bst_data->type == BST_TYPE_EXTERNAL_BUF)
		{
			// update the active bytes (1 ~ 2) in current bits
			if (bst_data->cursor.p_end - bst_data->cursor.p >= NAL_EMULATION_PREVENTION_BYTELEN &&
				NAL_EMULATION_PREVENTION_BYTES == CUR_LINEARBYTES_24BITS())
			{
				bst_data->cursor.curbits = (((*bst_data->cursor.p) << 8) | (*(bst_data->cursor.p + 1))) & 0xFFFF;
				bst_data->cursor.bits_left = 16;
			}
			else if (bst_data->cursor.p_end > bst_data->cursor.p)
			{
				bst_data->cursor.curbits = *bst_data->cursor.p;
				bst_data->cursor.bits_left = 8;
			}
		}
	}

	bst_data->cursor.modbits = bst_data->cursor.curbits;

	if (skip_remaining_bits > 0)
	{
		_Advance_InCacheBits(bst_data, skip_remaining_bits);
	}
}

inline void _UpdateCurBitsForUnreadChunk(AM_BST_DATA* bst_data, bool bEos = false)
{
#ifdef _DEBUG
	assert(bst_data->type == BST_TYPE_RING_CHUNK);
#endif
	if (bst_data->rbsp_type == BST_RBSP_SEQUENCE)
	{
		if (bst_data->cursor.chunk_unread_bytes.length() >= (int)sizeof(CURBITS_TYPE))
		{
#ifdef _USE_64BIT_CACHE
			bst_data->cursor.curbits = bst_data->cursor.chunk_unread_bytes.peeku64();
#else
			bst_data->cursor.curbits = bst_data->cursor.chunk_unread_bytes.peeku32();
#endif
			bst_data->cursor.bits_left = sizeof(CURBITS_TYPE) * 8;
		}
		else
		{
			bst_data->cursor.curbits = 0;
			for (int i = 0; i < bst_data->cursor.chunk_unread_bytes.length();i++){
				bst_data->cursor.curbits <<= 8;
				bst_data->cursor.curbits |= bst_data->cursor.chunk_unread_bytes.units[((size_t)bst_data->cursor.chunk_unread_bytes.read_pos + i) % _countof(bst_data->cursor.chunk_unread_bytes.units)];
				bst_data->cursor.bits_left += 8;
			}
		}
	}
	else if (bst_data->rbsp_type == BST_RBSP_NAL_UNIT)
	{
		if (bst_data->cursor.nal_ring_bytes.full())
		{
			// Update the current bits, and ring bytes layout
			if (NAL_EMULATION_PREVENTION_BYTES == CUR_RINGBYTES_24BITS())
			{
				bst_data->cursor.curbits = (NAL_EMULATION_PREVENTION_BYTES) >> 8;
				bst_data->cursor.bits_left = 16;

				// The available bits has been read, reset the current ring bytes;
				bst_data->cursor.nal_ring_bytes.reset();
			}
			else
			{
				bst_data->cursor.curbits = bst_data->cursor.nal_ring_bytes.read();
				bst_data->cursor.bits_left = 8;
			}
		}
		else if (bEos)
		{
			// There are remaining bytes in current NAL ring bytes, and less than 3 bytes, since EOS is reached, use the left bytes.
			AMP_Assert(bst_data->cursor.chunk_unread_bytes.empty());
			if (!bst_data->cursor.nal_ring_bytes.empty())
			{
				bst_data->cursor.curbits = bst_data->cursor.nal_ring_bytes.read();
				bst_data->cursor.bits_left = 8;
			}
		}
	}

	bst_data->cursor.modbits = bst_data->cursor.curbits;
}

void _WriteBackCurBits(AM_BST_DATA* bst_data, BOOL bKeepPos=FALSE)
{
	if (bst_data->cursor.p + sizeof(CURBITS_TYPE) <= bst_data->cursor.p_end)
	{
		*(CURBITS_TYPE*)bst_data->cursor.p = CURBITS_VALUE1(bst_data->cursor.modbits);
		if (!bKeepPos)
		{
			bst_data->cursor.p += sizeof(CURBITS_TYPE);
			_UpdateCurBits(bst_data);
		}
	}
	else
	{
		/*
		   Two extreme cases are required to be handled
		   0x123456 to be put
		   Case#1
		   |-byte#0-|-byte#1-|-byte#2-|
		     \
		      \---p/p_start/start_offset=0
		   byte#0 = 0x12; byte#1 = 0x34; byte#2 = 0x56
		   |-byte#0-|-byte#1-|-byte#2-|-byte#3-|
		      \_p/p_start \
		                   \_start_offset = 1
		   byte#0 = N/A;  byte#1 = 0x12; byte#2 = 0x34; byte#4 = 0x56
		*/

		int cur_start_offset = bst_data->cursor.p == bst_data->cursor.p_start?bst_data->cursor.start_offset:0;
		uint8_t* pbyte = bst_data->cursor.p + cur_start_offset;
		size_t full_byte_len = (bst_data->cursor.p_end - bst_data->cursor.p) - cur_start_offset;
		for(;pbyte<bst_data->cursor.p_end;pbyte++)
		{
			*pbyte = (bst_data->cursor.modbits >> (full_byte_len - 1)*8)&0xFF;
			full_byte_len--;
		}

		if (!bKeepPos)
		{
			bst_data->cursor.p = bst_data->cursor.p_end;
			_UpdateCurBits(bst_data);
		}
	}
}

#if 0
inline int _RefineRingChunkBst(AM_BST_DATA* bst_data, bool bPeek = false)
{
	UNREFERENCED_PARAMETER(bPeek);
	if (bst_data->cursor.p_start != NULL)
	{
		// Allow to write the data to current chunk in another thread
		AM_RingChunk_UnlockRead(bst_data->rchunk, (AMChunk)bst_data->cursor.p_start);
		bst_data->cursor.p_start = bst_data->cursor.p = bst_data->cursor.p_end = NULL;
	}

	// Try to get another chunk to read bits from it.
	bst_data->cursor.p_start = AM_RingChunk_LockRead(bst_data->rchunk);
	if (bst_data->cursor.p_start == NULL){
		// No data is available
		AMP_Warning(_T("[Bitstream] no buffer is available in ring buffer.\n"));
		return RET_CODE_ERROR;
	}

	// Update the bitstream read pointer, current bits and so on.
	bst_data->cursor.p = bst_data->cursor.p_start;
	bst_data->cursor.p_end = bst_data->cursor.p + AM_RingChunk_GetChunkSize(bst_data->rchunk);
	return RET_CODE_SUCCESS;
}
#endif

// When there is no more bit in current cursor, try to get more bits from the underlying bit stream to fill the cursor.
inline void _FillCurrentBits(AM_BST_DATA* bst_data, bool bPeek = false)
{
	AMP_Assert(bst_data->cursor.bits_left == 0);

	// Fill the curbits
	if (bst_data->rbsp_type == BST_RBSP_SEQUENCE)
	{
		if (bst_data->type == BST_TYPE_RING_CHUNK && !bst_data->cursor.chunk_unread_bytes.empty())
		{
			// Move the available chunk_unread_bytes to the beginning of buffer.
			bst_data->cursor.chunk_unread_bytes.skip(sizeof(CURBITS_TYPE));
			_UpdateCurBitsForUnreadChunk(bst_data);
		}
		
		// if chunk_unread_bytes is empty, it may be also external buffer or inner buffer type except the read-out ring-chunk buffer type.
		if (bst_data->cursor.chunk_unread_bytes.empty() && bst_data->cursor.p < bst_data->cursor.p_end)
		{
			assert(bst_data->cursor.bits_left == 0);
			if (bst_data->cursor.p_end <= bst_data->cursor.p + sizeof(CURBITS_TYPE))
				bst_data->cursor.p = bst_data->cursor.p_end;
			else
				bst_data->cursor.p += sizeof(CURBITS_TYPE);

			_UpdateCurBits(bst_data);
		}
	}
	else if (bst_data->rbsp_type == BST_RBSP_NAL_UNIT)
	{
		_UpdateCurBits(bst_data);

		bst_data->cursor.p += bst_data->cursor.bits_left == 16 ? NAL_EMULATION_PREVENTION_BYTELEN : bst_data->cursor.bits_left/8;
	}

	BOOL bExhaust = (bst_data->cursor.p == bst_data->cursor.p_end && bst_data->cursor.bits_left == 0)?TRUE:FALSE;
	if (FALSE == bExhaust)
		return;

	if (bst_data->type == BST_TYPE_INNER_BUF)
	{
		bool bEos = false;
		int nRet = bst_data->func_ptr_fillbuffer(bst_data->cursor.p_start, bst_data->cursor.buf_size, bEos, bst_data->user_data);
		if (nRet <= 0)
			return;

		if (bPeek)
		{
			// Activate a save point for future restore
			bst_data->save_point = bst_data->cursor;
		}

		if (bst_data->rbsp_type == BST_RBSP_SEQUENCE)
		{
			bst_data->cursor.p = bst_data->cursor.p_start;
			bst_data->cursor.p_end = bst_data->cursor.p_start + nRet;
		}
		else if (bst_data->rbsp_type == BST_RBSP_NAL_UNIT)
		{
			// Fill the ring bytes
			while (bst_data->cursor.p_end > bst_data->cursor.p && bst_data->cursor.nal_ring_bytes.len < NAL_EMULATION_PREVENTION_BYTELEN)
			{
				bst_data->cursor.nal_ring_bytes.write(*(bst_data->cursor.p++));
			}
		}

		_UpdateCurBits(bst_data, bEos);

		if (bst_data->rbsp_type == BST_RBSP_NAL_UNIT)
			bst_data->cursor.p += bst_data->cursor.bits_left == 16 ? NAL_EMULATION_PREVENTION_BYTELEN : bst_data->cursor.bits_left / 8;
	}
	else if(bst_data->type == BST_TYPE_RING_CHUNK)
	{
		if (bPeek == true && bst_data->save_point.p != NULL)
		{
			int skip_bytes = (int)(bst_data->save_point.p_end - bst_data->save_point.p);
			int left_unread_bytes = sizeof(bst_data->cursor.chunk_unread_bytes.units) - bst_data->cursor.chunk_unread_bytes.length();
			AMP_Assert(skip_bytes <= left_unread_bytes);
			for (int i = 0; i < skip_bytes; i++)
				bst_data->cursor.chunk_unread_bytes.write(*(bst_data->save_point.p + i));
		}

#if 0
		_RefineRingChunkBst(bst_data);
#endif

		if (bPeek == true)
		{
			if (bst_data->save_point.p == NULL)
				bst_data->save_point = bst_data->cursor;
			else
			{
				bst_data->save_point.p = bst_data->cursor.p;
				bst_data->save_point.p_end = bst_data->cursor.p_end;
				bst_data->save_point.p_start = bst_data->cursor.p_start;
				bst_data->save_point.buf_size = bst_data->cursor.buf_size;
				bst_data->save_point.start_offset = bst_data->cursor.start_offset;
			}
		}

		_UpdateCurBits(bst_data);

		if (bst_data->rbsp_type == BST_RBSP_NAL_UNIT)
			bst_data->cursor.p += bst_data->cursor.bits_left == 16 ? NAL_EMULATION_PREVENTION_BYTELEN : bst_data->cursor.bits_left / 8;
	}
}

inline void _NAL_UNIT_FillRingBytes(AM_BST_DATA* bst_data)
{
	while(!bst_data->cursor.nal_ring_bytes.full())
	{
		bst_data->cursor.nal_ring_bytes.write(*bst_data->cursor.p);
	}

	// check whether there exist emulation_prevention_three_byte, remove it.
	return;
}

inline uint64_t _GetBits(AM_BST_DATA* bst_data, int n, bool bPeek = false, bool bThrowExceptionHitStartCode = false)
{
	UNREFERENCED_PARAMETER(bThrowExceptionHitStartCode);
	if(!(bst_data->access_mode&BST_MODE_READ))
		throw AMException(RET_CODE_ERROR_PERMISSION, _T("current bitstream handle can't read bits"));

	if (bst_data->rbsp_type != BST_RBSP_SEQUENCE && bst_data->rbsp_type != BST_RBSP_NAL_UNIT)
		throw AMException(RET_CODE_ERROR_NOTIMPL, _T("the specified bitstream rbsp type is not supported.\n"));

	if (bPeek)
	{
		// Make sure there is no active data in save point of bitstream
		AMP_Assert(bst_data->save_point.p == NULL);
	}

	uint64_t nRet = 0;
	// For NAL RBSP stream, without scanning the buffer, didn't know its left bits
	if (bst_data->type == BST_TYPE_EXTERNAL_BUF)
	{
		if (bst_data->rbsp_type == BST_RBSP_SEQUENCE)
		{
			int nAllLeftBits = GetAllLeftBits(bst_data);
			if (n > nAllLeftBits)
				throw AMException(RET_CODE_NO_MORE_DATA, _T("invalid parameter, no enough data"));
		}

		if (bst_data->cursor.bits_left == 0)
			_UpdateCurBits(bst_data);

		// Activate a save_point for the current bit-stream cursor
		if (bPeek)
			bst_data->save_point = bst_data->cursor;
	}
	else if(bst_data->type == BST_TYPE_INNER_BUF)
	{
		// Store the save point for future restore
		if (bst_data->save_point.p == NULL && bPeek)
			bst_data->save_point = bst_data->cursor;
		/*
		Under NAL_unit rbsp type:
		For inner buffer filled by callback function, we should assume the full inner buffer should
		contain a 64bit value, in another word, buffer size should be ensured 8 bytes /(2/3) = 12 bytes.
		for example, 00 00 03 00 00 03 00 00 03 00 00 03, it contain 8 bytes, but bitstream length is 12 bytes.
		*/
		if (bst_data->cursor.p == bst_data->cursor.p_end && bst_data->cursor.bits_left == 0)
		{
			_FillCurrentBits(bst_data, bPeek);
		}
	}
	else if (bst_data->type == BST_TYPE_RING_CHUNK)
	{
		// Store the save point for future restore
		if (bst_data->save_point.p == NULL && bPeek)
			bst_data->save_point = bst_data->cursor;

		/*
		For the ring chunk, there may be available bytes in chunk_unread_bytes, need consider this case.
		*/
		if (bst_data->cursor.p == bst_data->cursor.p_end && bst_data->cursor.bits_left == 0)
		{
			_FillCurrentBits(bst_data, bPeek);
		}
	}

	while(n > 0 && bst_data->cursor.bits_left > 0)
	{
		int nProcessed = AMP_MIN(n, bst_data->cursor.bits_left);
		CURBITS_TYPE nMask = CURBITS_MASK(bst_data->cursor.bits_left);
		nRet <<= nProcessed;
		nRet  |= (bst_data->cursor.curbits&nMask)>>(bst_data->cursor.bits_left - nProcessed);
		_Advance_InCacheBits(bst_data, nProcessed);

		if (bst_data->cursor.bits_left == 0)
		{
			if ((bPeek == false) &&
				(bst_data->access_mode&BST_MODE_WRITE) &&
				(bst_data->cursor.curbits != bst_data->cursor.modbits))
			{
				_WriteBackCurBits(bst_data, TRUE);
			}
			
			_FillCurrentBits(bst_data, bPeek);
		}

		n -= nProcessed;
	}

	// Restore the save point
	if (bst_data->save_point.p != NULL && bPeek == true)
	{
		if (bst_data->type == BST_TYPE_RING_CHUNK)
		{
			bst_data->cursor.p = bst_data->save_point.p;
			bst_data->cursor.curbits = bst_data->save_point.curbits;
			bst_data->cursor.modbits = bst_data->save_point.modbits;
			bst_data->cursor.bits_left = bst_data->save_point.bits_left;
			bst_data->cursor.nal_ring_bytes = bst_data->save_point.nal_ring_bytes;
		}
		else
			bst_data->cursor = bst_data->save_point;
	}

	if (n != 0)
		throw AMException(RET_CODE_NO_MORE_DATA, _T("invalid parameter, no enough data"));

	return nRet;
}

int _SkipBits(AM_BST_DATA* bst_data, int skip_bits, AMBstBitFillMode bit_fill_mode=BST_BIT_FILL_KEEP)
{
	int ret_skip_bits = skip_bits;

	// FIXME
	// 20180321, for inner buffer with callback filling, if calling AMBst_SkipBits at first, also need trying to fill data
	if (bst_data->type == BST_TYPE_INNER_BUF)
	{
		if (bst_data->cursor.p == bst_data->cursor.p_end && bst_data->cursor.bits_left == 0)
		{
			_FillCurrentBits(bst_data);
		}
	}

	while(skip_bits > 0 && bst_data->cursor.p < bst_data->cursor.p_end)
	{
		int nProcessed = AMP_MIN(skip_bits, bst_data->cursor.bits_left);
		_Advance_InCacheBits(bst_data, nProcessed, bit_fill_mode);

		if (bst_data->cursor.bits_left == 0)
		{
			if((bst_data->access_mode&BST_MODE_WRITE) && bst_data->cursor.curbits != bst_data->cursor.modbits)
				_WriteBackCurBits(bst_data, TRUE);

			_FillCurrentBits(bst_data);
		}

		skip_bits -= nProcessed;
	}

	ret_skip_bits -=skip_bits;

	return ret_skip_bits;
}

int _RefineInnerBuffer(AM_BST_DATA* bst_data)
{
	// Move the bytes to the head of inner buffer
	int left_buf_size = 0;
	if (bst_data->cursor.p_end > bst_data->cursor.p)
	{
		left_buf_size = (int)(bst_data->cursor.p_end - bst_data->cursor.p);
		memmove(bst_data->cursor.p_start, bst_data->cursor.p, left_buf_size);
	}

	bst_data->cursor.p = bst_data->cursor.p_start;
	bst_data->cursor.p_end = bst_data->cursor.p + left_buf_size;

	bool bEos = false;
	int nRet = bst_data->func_ptr_fillbuffer(bst_data->cursor.p_end, bst_data->cursor.buf_size - left_buf_size, bEos, bst_data->user_data);
	if (nRet <= 0)
		return nRet;

	bst_data->cursor.p_end += nRet;	

	if (bst_data->rbsp_type == BST_RBSP_SEQUENCE)
	{
		int32_t backup_bits_left = bst_data->cursor.bits_left;
		bst_data->cursor.bits_left = 0;
		_UpdateCurBits(bst_data);
		int new_left_buf_size = (int)(bst_data->cursor.p_end - bst_data->cursor.p);
		bst_data->cursor.bits_left = (int)(backup_bits_left + ((new_left_buf_size > (int)sizeof(CURBITS_TYPE) ? (int)sizeof(CURBITS_TYPE) : new_left_buf_size) - left_buf_size) * 8);
	}
	else if (bst_data->rbsp_type == BST_RBSP_NAL_UNIT)
	{
		// Fill the ring bytes
		while (bst_data->cursor.p_end > bst_data->cursor.p && !bst_data->cursor.nal_ring_bytes.full())
		{
			bst_data->cursor.nal_ring_bytes.write(*(bst_data->cursor.p++));
		}

		_UpdateCurBits(bst_data, bEos);
	}

	return RET_CODE_SUCCESS;
}

int _Flush(AM_BST_DATA* bst_data)
{
	int iRet = RET_CODE_SUCCESS;

	int left_shift_bits = bst_data->cursor.bits_left&(sizeof(CURBITS_TYPE)*8-1);

	if (left_shift_bits != 0 && bst_data->cursor.modbits != bst_data->cursor.curbits)
	{
		// modbits to curbits
		CURBITS_TYPE curbits_backup = bst_data->cursor.modbits;
		CURBITS_TYPE nValMask = CURBITS_MASK(left_shift_bits);

		bst_data->cursor.modbits <<= left_shift_bits;
		bst_data->cursor.modbits |= (bst_data->cursor.curbits&nValMask);
		_WriteBackCurBits(bst_data, TRUE);

		bst_data->cursor.modbits = bst_data->cursor.curbits = curbits_backup;
	}
	else
		iRet = RET_CODE_NOTHING_TODO;

	int AvailableBytes = (int)((bst_data->cursor.p_end >= bst_data->cursor.p + sizeof(CURBITS_TYPE)) ? sizeof(CURBITS_TYPE) : (bst_data->cursor.p_end - bst_data->cursor.p));
	if (bst_data->access_mode == BST_MODE_WRITE && bst_data->type == BST_TYPE_INNER_BUF && (
		bst_data->cursor.p > bst_data->cursor.p_start || (bst_data->cursor.p == bst_data->cursor.p_start && bst_data->cursor.bits_left < (AvailableBytes <<3))))
	{
		_UpdateCurBits(bst_data, false, true);
		iRet = RET_CODE_SUCCESS;
	}

	return iRet;
}

int	AMBst_SetRBSPType(AMBst bst, AMBstRBSPType rbsp_type)
{
	if (bst == NULL)
		return RET_CODE_INVALID_PARAMETER;

	AM_BST_DATA* bst_data = (AM_BST_DATA*)bst;
	if (rbsp_type == bst_data->rbsp_type)
		return RET_CODE_SUCCESS;

	if (rbsp_type == BST_RBSP_NAL_UNIT)
	{
		// At present, does not support access mode with WRITE flag
		// If client want to write EBSP side-by-side, it can use inner buffer with callback mode
		// In the callback, it can convert the rbsp to ebsp
		if ((bst_data->access_mode&BST_MODE_WRITE) == BST_MODE_WRITE)
			return RET_CODE_ERROR_NOTIMPL;

		// Check whether the current position is byte aligned or not
		if (bst_data->cursor.bits_left % 8 != 0)
			return RET_CODE_NOT_ALIGNED;

		// if stream is filled by callback, the inner buffer should be greater than 64bit*3/2
		if (bst_data->type == BST_TYPE_INNER_BUF && bst_data->cursor.buf_size <= (int)sizeof(uint64_t) * 3 / 2)
		{
			printf("[Bitstream] For the bitstream filled by callback, the inner buffer should be greater than 12bit.\n");
			return RET_CODE_BUFFER_TOO_SMALL;
		}

		// For the writable sequence stream, write-back the changes.
		if ((bst_data->access_mode&BST_MODE_WRITE) && bst_data->cursor.curbits != bst_data->cursor.modbits)
			_WriteBackCurBits(bst_data, TRUE);

		// update the real position.
		int bytes_left = (int)(bst_data->cursor.p_end - bst_data->cursor.p);
		int bytes_offset = (AMP_MIN(bytes_left, (int)sizeof(CURBITS_TYPE))) - bst_data->cursor.bits_left / 8;
		bst_data->cursor.p += bytes_offset;

		// reset the previous data
		bst_data->cursor.bits_left = 0;

		// Switch to NAL_UNIT context of bitstream parsing
		bst_data->rbsp_type = rbsp_type;

		// For the inner_buffer filled with callback and Ring-Chunk, we need fill the ring bytes
		if (bst_data->type == BST_TYPE_INNER_BUF || bst_data->type == BST_TYPE_RING_CHUNK)
		{
			// Initialize the ring bytes
			bst_data->cursor.nal_ring_bytes.reset();

			// Fill the ring buffer
			while (bst_data->cursor.p_end > bst_data->cursor.p && !bst_data->cursor.nal_ring_bytes.full())
			{
				bst_data->cursor.nal_ring_bytes.write(*(bst_data->cursor.p++));
			}
		}

		_UpdateCurBits(bst_data);

		if(bst_data->type == BST_TYPE_EXTERNAL_BUF)
		{
			bst_data->cursor.p += bst_data->cursor.bits_left==16?NAL_EMULATION_PREVENTION_BYTELEN:1;
		}
	}
	else if (rbsp_type == BST_RBSP_SEQUENCE)
	{
		uint8_t* p = bst_data->cursor.p - (bst_data->cursor.bits_left + 7) / 8;
		if (bst_data->cursor.p < bst_data->cursor.p_end || bst_data->cursor.bits_left > 0)
		{
			// At first find the right p which including the current bits
			int start_offset = ((intptr_t)p & 3);
			bst_data->cursor.p = p - start_offset;

			int bytes_left = (int)(bst_data->cursor.p_end - p);
			bst_data->cursor.bits_left = ((AMP_MIN(bytes_left, (int)sizeof(CURBITS_TYPE))) - start_offset) * 8 + (8 - bst_data->cursor.bits_left) % 8;

			if (bst_data->cursor.p + sizeof(CURBITS_TYPE) <= bst_data->cursor.p_end)
			{
				bst_data->cursor.curbits = CURBITS_VALUE(bst_data->cursor.p);
			}
			else
			{
				bst_data->cursor.curbits = 0;
				for (uint8_t* pbyte = bst_data->cursor.p; pbyte < bst_data->cursor.p_end; pbyte++) {
					bst_data->cursor.curbits <<= 8;
					bst_data->cursor.curbits |= *(uint8_t*)pbyte;
				}
			}

			bst_data->cursor.modbits = bst_data->cursor.curbits;
		}

		bst_data->rbsp_type = rbsp_type;
	}

	return RET_CODE_SUCCESS;
}

int AMBst_MarkEndPos(AMBst bst, int end_pos)
{
	UNREFERENCED_PARAMETER(bst);
	UNREFERENCED_PARAMETER(end_pos);
	return RET_CODE_ERROR_NOTIMPL;
}

uint64_t AMBst_GetBits(AMBst bst, int n)
{
	if (bst == NULL || n > (int)sizeof(uint64_t)*8)
		throw AMException(RET_CODE_INVALID_PARAMETER, _T("invalid parameter"));

	AM_BST_DATA* bst_data = (AM_BST_DATA*)bst;
	return _GetBits(bst_data, n);
}

uint64_t AMBst_PeekBits(AMBst bst, int n)
{
	if (bst == NULL || n > (int)sizeof(uint64_t)*8)
		throw AMException(RET_CODE_INVALID_PARAMETER, _T("invalid parameter"));

	AM_BST_DATA* bst_data = (AM_BST_DATA*)bst;

	if(!(bst_data->access_mode&BST_MODE_READ))
		throw AMException(RET_CODE_ERROR_PERMISSION, _T("current bitstream handle can't read bits"));

	if (n == 0)
		return 0;

#if 1
	// Clean up the previous save point
	bst_data->CleanSavePoint();

	return _GetBits(bst_data, n, true);
#else
	uint64_t peek_val = 0LL;
	if (bst_data->type == BST_TYPE_EXTERNAL_BUF)
	{
		// Backup the current bit-stream context information
		uint8_t* backup_p = bst_data->cursor.p;
		int32_t backup_bits_left = bst_data->cursor.bits_left;
		CURBITS_TYPE backup_curbits = bst_data->cursor.curbits;
		AM_BST_DATA::AM_BST_NAL_RING_BYTES backup_ring_bytes = bst_data->cursor.nal_ring_bytes;
		
		// c++0x/11 support this feature, so it need at least gcc4.3/vs2010
		std::exception_ptr e_ptr;

		try
		{
			peek_val = _GetBits(bst_data, n);
		}
		catch (...)
		{
			e_ptr = std::current_exception();
		}

		// Restore the current bit-stream context information
		bst_data->cursor.p = backup_p;
		bst_data->cursor.curbits = backup_curbits;
		bst_data->cursor.bits_left = backup_bits_left;
		bst_data->cursor.nal_ring_bytes = backup_ring_bytes;

		// if exception has ever been throw in _GetBits, re-throw it here
		if (e_ptr)
			std::rethrow_exception(e_ptr);

		return peek_val;
	}
	else if (bst_data->type == BST_TYPE_INNER_BUF)
	{
		if ((bst_data->rbsp_type == BST_RBSP_SEQUENCE && n > GetAllLeftBits(bst_data)) ||
			(bst_data->rbsp_type == BST_RBSP_NAL_UNIT && n > bst_data->cursor.bits_left))
		{
			// TODO...
			_RefineInnerBuffer(bst_data);
		}
	}

	int nAllLeftBits = GetAllLeftBits(bst_data);
	if(bst_data->type == BST_TYPE_INNER_BUF && n > nAllLeftBits)
	{
		_RefineInnerBuffer(bst_data);

		nAllLeftBits = GetAllLeftBits(bst_data);
	}
	else if(bst_data->type == BST_TYPE_RING_CHUNK)
	{
		if (bst_data->cursor.p == bst_data->cursor.p_end && bst_data->cursor.bits_left == 0)
		{
			_RefineRingChunkBst(bst_data);
			nAllLeftBits = GetAllLeftBits(bst_data);
		}
	}

	if (n <= nAllLeftBits)
	{
		// Save the current position
		uint8_t* backup_p = bst_data->cursor.p;
		int32_t backup_bits_left = bst_data->cursor.bits_left;
		CURBITS_TYPE backup_curbits = bst_data->cursor.curbits;
		peek_val = _GetBits(bst_data, n);
		bst_data->cursor.p = backup_p;
		bst_data->cursor.curbits = backup_curbits;
		bst_data->cursor.bits_left = backup_bits_left;
	}
	else
	{
		throw AMException(RET_CODE_NO_MORE_DATA, _T("no enough buffer"));
	}

	return peek_val;
#endif
}

uint8_t AMBst_GetByte(AMBst bst)
{
	if (bst == NULL)
		throw AMException(RET_CODE_INVALID_PARAMETER, _T("invalid parameter"));

	AM_BST_DATA* bst_data = (AM_BST_DATA*)bst;
	return (uint8_t)_GetBits(bst_data, 8);
}

uint16_t AMBst_GetWord(AMBst bst)
{
	if (bst == NULL)
		throw AMException(RET_CODE_INVALID_PARAMETER, _T("invalid parameter"));

	AM_BST_DATA* bst_data = (AM_BST_DATA*)bst;
	return (uint16_t)_GetBits(bst_data, 16);
}

uint32_t AMBst_GetDWord(AMBst bst)
{
	if (bst == NULL)
		throw AMException(RET_CODE_INVALID_PARAMETER, _T("invalid parameter"));

	AM_BST_DATA* bst_data = (AM_BST_DATA*)bst;
	return (uint32_t)_GetBits(bst_data, 32);
}

uint64_t AMBst_GetQWord(AMBst bst)
{
	if (bst == NULL)
		throw AMException(RET_CODE_INVALID_PARAMETER, _T("invalid parameter"));

	AM_BST_DATA* bst_data = (AM_BST_DATA*)bst;
	return _GetBits(bst_data, 64);
}

int8_t AMBst_GetChar(AMBst bst)
{
	if (bst == NULL)
		throw AMException(RET_CODE_INVALID_PARAMETER, _T("invalid parameter"));

	AM_BST_DATA* bst_data = (AM_BST_DATA*)bst;
	return (int8_t)_GetBits(bst_data, 8);
}

int16_t AMBst_GetShort(AMBst bst)
{
	if (bst == NULL)
		throw AMException(RET_CODE_INVALID_PARAMETER, _T("invalid parameter"));

	AM_BST_DATA* bst_data = (AM_BST_DATA*)bst;
	return (int16_t)_GetBits(bst_data, 16);
}

int32_t AMBst_GetLong(AMBst bst)
{
	if (bst == NULL)
		throw AMException(RET_CODE_INVALID_PARAMETER, _T("invalid parameter"));

	AM_BST_DATA* bst_data = (AM_BST_DATA*)bst;
	return (int32_t)_GetBits(bst_data, 16);
}

int64_t AMBst_GetLongLong(AMBst bst)
{
	if (bst == NULL)
		throw AMException(RET_CODE_INVALID_PARAMETER, _T("invalid parameter"));

	AM_BST_DATA* bst_data = (AM_BST_DATA*)bst;
	return (int64_t)_GetBits(bst_data, 64);
}

int AMBst_PutBits(AMBst bst, int n, uint64_t val)
{
	AM_BST_DATA* bst_data = (AM_BST_DATA*)bst;
	if (bst_data == NULL)
		return RET_CODE_INVALID_PARAMETER;

	if(!(bst_data->access_mode&BST_MODE_WRITE))
		throw AMException(RET_CODE_ERROR_PERMISSION, _T("current bitstream handle can't write bits"));

	// At present, only support external buffer mode.
	if (bst_data->type != BST_TYPE_EXTERNAL_BUF && bst_data->type != BST_TYPE_INNER_BUF)
		return RET_CODE_ERROR_NOTIMPL;

    if (n > (int)sizeof(uint64_t)*8)
		return RET_CODE_INVALID_PARAMETER;

	int nTotalProcessed = 0;
	while(n > 0 && bst_data->cursor.p < bst_data->cursor.p_end)
	{
		int nProcessed = AMP_MIN(n, bst_data->cursor.bits_left);
		CURBITS_TYPE nValMask = CURBITS_MASK(nProcessed);
		CURBITS_TYPE nCurBits = (nProcessed==sizeof(CURBITS_TYPE)*8)?0:(bst_data->cursor.modbits << nProcessed);
		bst_data->cursor.modbits = (CURBITS_TYPE)(nCurBits | ((val>>(n-nProcessed))&nValMask));
		bst_data->cursor.bits_left -= nProcessed;

		if (bst_data->cursor.bits_left == 0)
			_WriteBackCurBits(bst_data);

		n -= nProcessed;
		nTotalProcessed += nProcessed;
	}

	return nTotalProcessed;
}

int AMBst_ReservedBits(AMBst bst, int n, BOOL ReservedBitSet)
{
	return AMBst_PutBits(bst, n, ReservedBitSet?MAXUINT64:0);
}
   
int AMBst_PutByte(AMBst bst, uint8_t byte)
{   
	return AMBst_PutBits(bst, 8, byte);
}   
    
int AMBst_PutWord(AMBst bst, uint16_t word)
{   
	return AMBst_PutBits(bst, 16, word);
}   
    
int AMBst_PutDWord(AMBst bst, uint32_t dword)
{   
	return AMBst_PutBits(bst, 32, dword);
}   
    
int AMBst_PutQWord(AMBst bst, uint64_t qword)
{   
	return AMBst_PutBits(bst, 64, qword);
}   
    
int AMBst_PutChar(AMBst bst, int8_t schar)
{   
	return AMBst_PutBits(bst, 8, schar);
}   
    
int AMBst_PutShort(AMBst bst, int16_t sshort)
{   
	return AMBst_PutBits(bst, 16, sshort);
}   
    
int AMBst_PutLong(AMBst bst, int32_t slong)
{   
	return AMBst_PutBits(bst, 32, slong);
}   
    
int AMBst_PutLongLong(AMBst bst, int64_t slonglong)
{
	return AMBst_PutBits(bst, 64, slonglong);
}

int AMBst_Flush(AMBst bst)
{
	if (IS_INVALID_HANDLE(bst))
		return RET_CODE_INVALID_PARAMETER;

	AM_BST_DATA* bst_data = (AM_BST_DATA*)bst;

	if(!(bst_data->access_mode&BST_MODE_WRITE))
		return RET_CODE_NOTHING_TODO;

	return _Flush(bst_data);
}

int AMBst_Tell(AMBst bst, int* left_bits_in_bst)
{
	if (bst == NULL)
		throw AMException(RET_CODE_INVALID_PARAMETER, _T("invalid parameter"));

	AM_BST_DATA* bst_data = (AM_BST_DATA*)bst;

	if (bst_data->cursor.p_start == NULL || bst_data->cursor.p == NULL)
	{
		AMP_SAFEASSIGN(left_bits_in_bst, 0);
		return 0;
	}

	int nAllLeftBits = GetAllLeftBits(bst_data);
	if (left_bits_in_bst != NULL)
		*left_bits_in_bst = nAllLeftBits;

	return (int)(8*(bst_data->cursor.p_end - bst_data->cursor.p_start - bst_data->cursor.start_offset) - nAllLeftBits);
}

int AMBst_Seek(AMBst bst, int bit_pos)
{
	if (IS_INVALID_HANDLE(bst))
		return RET_CODE_INVALID_PARAMETER;

	AM_BST_DATA* bst_data = (AM_BST_DATA*)bst;

	if (bst_data->type != BST_TYPE_EXTERNAL_BUF)
		return RET_CODE_ERROR_NOTIMPL;

	if (bit_pos > (bst_data->cursor.p_end - bst_data->cursor.p_start - bst_data->cursor.start_offset)*8)
		return RET_CODE_INVALID_PARAMETER;

	if (bit_pos == -1)
		bit_pos = (int)(bst_data->cursor.p_end - bst_data->cursor.p_start - bst_data->cursor.start_offset)*8;

	uint8_t* ptr_dest  = bst_data->cursor.p_start + (bit_pos + (int64_t)bst_data->cursor.start_offset*8)/(sizeof(CURBITS_TYPE)*8)*sizeof(CURBITS_TYPE);
 	int bytes_left = (int)(bst_data->cursor.p_end - bst_data->cursor.p);
	int bits_left = AMP_MIN(bytes_left, (int)sizeof(CURBITS_TYPE))*8 - (bit_pos + (int64_t)bst_data->cursor.start_offset*8)%(sizeof(CURBITS_TYPE)*8);

	if (ptr_dest != bst_data->cursor.p){
		if((bst_data->access_mode&BST_MODE_WRITE) && bst_data->cursor.modbits != bst_data->cursor.curbits)
			_Flush(bst_data);
	}

	bst_data->cursor.p = ptr_dest;
	_UpdateCurBits(bst_data);
	bst_data->cursor.bits_left = bits_left;

	return bit_pos;
}

int AMBst_Realign(AMBst bst, AMBstAlign bstAlign, AMBstBitFillMode bitFillMode)
{
	if (bst == NULL)
		return RET_CODE_INVALID_PARAMETER;

	AM_BST_DATA* bst_data = (AM_BST_DATA*)bst;

	if (bstAlign == BST_ALIGN_BEGIN){
		// If the current mode is also under write mode, and cache bits has been switched
		if((bst_data->access_mode&BST_MODE_WRITE) && bst_data->cursor.p != bst_data->cursor.p_start)
			_Flush(bst_data);

		bst_data->cursor.p = bst_data->cursor.p_start;
		bst_data->cursor.bits_left = 0;			
		
		_UpdateCurBits(bst_data);

		return RET_CODE_SUCCESS;
	}
	else if(bstAlign == BST_ALIGN_END)
	{
		if((bst_data->access_mode&BST_MODE_WRITE))
			_Flush(bst_data);

		bst_data->cursor.p = bst_data->cursor.p_end;
		bst_data->cursor.bits_left = 0;
		return RET_CODE_SUCCESS;
	}

	int skip_bits = 0, align_bits = 0;
	size_t nPos = ( 8 * (bst_data->cursor.p - bst_data->cursor.p_start) + (sizeof(bst_data->cursor.curbits)*8) - bst_data->cursor.bits_left - (int64_t)bst_data->cursor.start_offset*8);
	switch(bstAlign)
	{
	case BST_ALIGN_BYTE: align_bits =  8; break;
	case BST_ALIGN_WORD: align_bits = 16; break;
	case BST_ALIGN_DWORD:align_bits = 32; break;
	case BST_ALIGN_QWORD:align_bits = 64; break;
	case BST_ALIGN_NEXT: align_bits = sizeof(CURBITS_TYPE); break;
	}
	if (align_bits == 0)
		return RET_CODE_INVALID_PARAMETER;

	int pos_mod = (int)(nPos%align_bits);
	if((skip_bits = pos_mod==0?0:(align_bits - pos_mod)) == 0)
		return RET_CODE_SUCCESS;

	int nAllLeftBits = GetAllLeftBits(bst_data);
	if(bst_data->type == BST_TYPE_INNER_BUF && nAllLeftBits < skip_bits)
	{
		_RefineInnerBuffer(bst_data);

		nAllLeftBits = GetAllLeftBits(bst_data);
	}

	// check whether to be able skip "skip_bits"
	if (nAllLeftBits >= skip_bits || bst_data->type == BST_TYPE_RING_CHUNK)
		_SkipBits(bst_data, skip_bits, bitFillMode);
	else
		return RET_CODE_IGNORE_REQUEST;

	return RET_CODE_SUCCESS;
}

BOOL AMBst_IsAligned(AMBst bst, AMBstAlign bstAlign)
{
	if (bst == NULL)
		return FALSE;

	AM_BST_DATA* bst_data = (AM_BST_DATA*)bst;

	if (bstAlign == BST_ALIGN_BEGIN){
		if (bst_data->type == BST_TYPE_EXTERNAL_BUF)
			return bst_data->cursor.p == bst_data->cursor.p_start && bst_data->cursor.bits_left == (int)(sizeof(CURBITS_TYPE) - bst_data->cursor.start_offset)*8?TRUE:FALSE;
		return FALSE;
	}
	else if(bstAlign == BST_ALIGN_END)
	{
		if (bst_data->type == BST_TYPE_EXTERNAL_BUF)
			return bst_data->cursor.p == bst_data->cursor.p_end && bst_data->cursor.bits_left == 0?TRUE:FALSE;
		return FALSE;
	}

	int align_bits = 0;
	size_t nPos = ( 8 * (bst_data->cursor.p - bst_data->cursor.p_start) + (sizeof(bst_data->cursor.curbits)*8) - bst_data->cursor.bits_left - (int64_t)bst_data->cursor.start_offset*8);
	switch(bstAlign)
	{
	case BST_ALIGN_BYTE: align_bits =  8; break;
	case BST_ALIGN_WORD: align_bits = 16; break;
	case BST_ALIGN_DWORD:align_bits = 32; break;
	case BST_ALIGN_QWORD:align_bits = 64; break;
	case BST_ALIGN_NEXT: align_bits = sizeof(CURBITS_TYPE); break;
	}
	if (align_bits == 0)
		return FALSE;

	int pos_mod = (int)(nPos%align_bits);
	return (pos_mod==0?0:(align_bits - pos_mod)) == 0?TRUE:FALSE;
}

int AMBst_SkipBits(AMBst bst, int skip_bits)
{
	if (bst == NULL)
		throw AMException(RET_CODE_INVALID_PARAMETER, _T("invalid parameter"));

	AM_BST_DATA* bst_data = (AM_BST_DATA*)bst;
	return _SkipBits(bst_data, skip_bits);
}

uint8_t* AMBst_LockCurPtr(AMBst bst, int* nBitOffsetFromCurPtr, int* cbLeftBytes)
{
	if (bst == NULL)
		return NULL;

	AM_BST_DATA* bst_data = (AM_BST_DATA*)bst;

 	int bytes_left = (int)(bst_data->cursor.p_end - bst_data->cursor.p);
	int bits_offset = (AMP_MIN(bytes_left, (int)sizeof(CURBITS_TYPE)))*8 - bst_data->cursor.bits_left;

	AMP_SAFEASSIGN(cbLeftBytes, bytes_left);
	AMP_SAFEASSIGN(nBitOffsetFromCurPtr, bits_offset);

	return bst_data->cursor.p;
}

void AMBst_UnlockCurPtr(AMBst bst, int cbUpdated)
{
	if (bst == NULL)
		return;

	AM_BST_DATA* bst_data = (AM_BST_DATA*)bst;
	_SkipBits(bst_data, cbUpdated*8, BST_BIT_FILL_NOTHING);
	int nBitsLeft = bst_data->cursor.bits_left;
	_UpdateCurBits(bst_data);
	bst_data->cursor.bits_left = nBitsLeft;

	// Change the modbits
	bst_data->cursor.modbits >>= nBitsLeft;
	return;
}

int AMBst_GetBytes(AMBst bst, uint8_t* dest, int n)
{
	if (bst == NULL || dest == NULL || n <= 0)
		return RET_CODE_INVALID_PARAMETER;

	AM_BST_DATA* bst_data = (AM_BST_DATA*)bst;
	if(!(bst_data->access_mode&BST_MODE_READ))
		throw AMException(RET_CODE_ERROR_PERMISSION, _T("current bitstream handle can't read bits"));

	// If the current position not byte align, don't copy the bytes
	if (bst_data->cursor.bits_left%8 != 0)
		return RET_CODE_ERROR;

	int nCopyRet = 0, nLeft = n, nCurCopyCnt = 0;

	uint8_t* pSrcByte = bst_data->cursor.p;
	if (bst_data->rbsp_type == BST_RBSP_NAL_UNIT)
	{
		int i, nProcessed = AMP_MIN(nLeft, bst_data->cursor.bits_left / 8);
		// At first check the left bits, and copy the bytes to the buffer
		for (i = 0; i < nProcessed; i++, nLeft--, bst_data->cursor.bits_left -= 8)
			dest[i] = (bst_data->cursor.curbits >> (i * 8)) & 0xFF;

		// For inner buffer filled by callback or ring chunk, copy the bytes from ring bytes
		if (bst_data->type == BST_TYPE_INNER_BUF || bst_data->type == BST_TYPE_RING_CHUNK)
		{
			while(nLeft > 0)
			{
				bool bEos = false;
				if (bst_data->cursor.p_end <= bst_data->cursor.p && bst_data->cursor.bits_left == 0)
				{
					int nRet = 0;
					if (bst_data->type == BST_TYPE_INNER_BUF && (nRet = bst_data->func_ptr_fillbuffer(bst_data->cursor.p_start, bst_data->cursor.buf_size, bEos, bst_data->user_data)) > 0)
					{
						bst_data->cursor.p = bst_data->cursor.p_start;
						bst_data->cursor.p_end = bst_data->cursor.p_start + nRet;
					}
#if 0
					else if (bst_data->type == BST_TYPE_RING_CHUNK)
					{
						if (_RefineRingChunkBst(bst_data) != RET_CODE_SUCCESS)
							break;
					}
#endif
					else
						break;
				}

				// Fill the ring buffer
				while (bst_data->cursor.p_end > bst_data->cursor.p && !bst_data->cursor.nal_ring_bytes.full())
				{
					bst_data->cursor.nal_ring_bytes.write(*(bst_data->cursor.p++));
				}

				// Update the current bits
				_UpdateCurBits(bst_data, bEos);

				bst_data->cursor.p += bst_data->cursor.bits_left == 16 ? NAL_EMULATION_PREVENTION_BYTELEN : bst_data->cursor.bits_left / 8;

				// fill the destination buffer with the current bits
				for (i = 0; i < AMP_MIN(nLeft, bst_data->cursor.bits_left / 8); i++, nLeft--, bst_data->cursor.bits_left -= 8)
					dest[nProcessed++] = (bst_data->cursor.curbits >> (i * 8)) & 0xFF;
			}
		}
		else if (bst_data->type == BST_TYPE_EXTERNAL_BUF)
		{
			bool bNeedUpdateCurBits = false;
			// Fill the destination buffer with the nal_unit raw bit stream payload
			while (bst_data->cursor.p < bst_data->cursor.p_end && nLeft > 0)
			{
				if (bst_data->cursor.p_end >= bst_data->cursor.p + NAL_EMULATION_PREVENTION_BYTELEN && NAL_EMULATION_PREVENTION_BYTES == CUR_LINEARBYTES_24BITS())
				{
					dest[nProcessed++] = *(bst_data->cursor.p);
					nLeft--;

					if (nLeft > 0)
					{
						dest[nProcessed++] = *(bst_data->cursor.p + 1);
						nLeft--;
						bNeedUpdateCurBits = true;
					}
					else
					{
						bst_data->cursor.curbits = *(bst_data->cursor.p + 1);
						bst_data->cursor.bits_left = 8;
						bNeedUpdateCurBits = false;
					}

					bst_data->cursor.p += NAL_EMULATION_PREVENTION_BYTELEN;
				}
				else
				{
					dest[nProcessed++] = *(bst_data->cursor.p++);
					nLeft--;

					bNeedUpdateCurBits = true;
				}
			}

			if (bNeedUpdateCurBits && bst_data->cursor.p < bst_data->cursor.p_end)
			{
				bst_data->cursor.bits_left = 0;
				_FillCurrentBits(bst_data);
			}

			return n - nLeft;
		}
	}
	else if (bst_data->rbsp_type == BST_RBSP_SEQUENCE)
	{
		// For sequence bytes, update the real position where copy bytes
		pSrcByte += (sizeof(bst_data->cursor.curbits) - bst_data->cursor.bits_left / 8);

		while (nLeft > 0)
		{
			bool bEos = false;
			if (bst_data->cursor.p_end <= pSrcByte)
			{
				int nRet = 0;
				if (bst_data->type == BST_TYPE_INNER_BUF && (nRet = bst_data->func_ptr_fillbuffer(bst_data->cursor.p_start, bst_data->cursor.buf_size, bEos, bst_data->user_data)) > 0)
				{
					bst_data->cursor.p = bst_data->cursor.p_start;
					bst_data->cursor.p_end = bst_data->cursor.p_start + nRet;
					_UpdateCurBits(bst_data);
					pSrcByte = bst_data->cursor.p + (sizeof(bst_data->cursor.curbits) - bst_data->cursor.bits_left / 8);
				}
#if 0
				else if (bst_data->type == BST_TYPE_RING_CHUNK)
				{
					if (_RefineRingChunkBst(bst_data) != RET_CODE_SUCCESS)
						break;
					pSrcByte = bst_data->cursor.p;
				}
#endif
				else
					break;
			}

			int nAvailCurCopyCnt = (int)(bst_data->cursor.p_end - pSrcByte);
			nCurCopyCnt = AMP_MIN(nAvailCurCopyCnt, nLeft);
			memcpy(dest, pSrcByte, nCurCopyCnt);

			pSrcByte += nCurCopyCnt;
			nLeft -= nCurCopyCnt;
			nCopyRet += nCurCopyCnt;
			dest += nCurCopyCnt;
		}

		int start_offset = ((intptr_t)pSrcByte) & 3;
		bst_data->cursor.p = pSrcByte - start_offset;
		bst_data->cursor.bits_left = (sizeof(CURBITS_TYPE) - start_offset) * 8;

		bst_data->cursor.curbits = CURBITS_VALUE(bst_data->cursor.p);
		bst_data->cursor.modbits = bst_data->cursor.curbits;
	}

	return nCopyRet;
}

int AMBst_PutBytes(AMBst bst, uint8_t* src, int n)
{
	if (bst == NULL || src == NULL || n <= 0)
		return RET_CODE_INVALID_PARAMETER;

	AM_BST_DATA* bst_data = (AM_BST_DATA*)bst;
	if ((bst_data->access_mode&BST_MODE_READ_WRITE) == BST_MODE_READ_WRITE)
		throw AMException(RET_CODE_ERROR_PERMISSION, _T("current bitstream handle can't write/read bits at the same time"));

	// If the current position not byte align, don't copy the bytes
	if (bst_data->cursor.bits_left%8 != 0)
		return RET_CODE_ERROR;

	int nTotalCpyCnt = 0;
	do
	{
		int curbits_byte_cnt = (int)CURBITS_BYTECOUNT(bst_data->cursor);
#if 0
		if (bst_data->cursor.bits_left < (curbits_byte_cnt << 3) && bst_data->cursor.modbits != bst_data->cursor.curbits)
		{
			// modbits to curbits
			CURBITS_TYPE curbits_backup = bst_data->cursor.modbits;
			CURBITS_TYPE nValMask = CURBITS_MASK(left_shift_bits);

			bst_data->cursor.modbits <<= left_shift_bits;
			bst_data->cursor.modbits |= (bst_data->cursor.curbits&nValMask);

			if (bst_data->cursor.bits_left < (curbits_byte_cnt << 3))
				_WriteBackCurBits(bst_data, TRUE);
		}
#endif

		uint8_t* pDstByte = bst_data->cursor.p + curbits_byte_cnt - bst_data->cursor.bits_left / 8;
		int nDstCnt = GetAllLeftBits(bst_data) / 8;
		int nCpyCnt = AMP_MIN(nDstCnt, n);

		nTotalCpyCnt += nCpyCnt;

		if (nCpyCnt <= 0)
			return nCpyCnt;

		memcpy(pDstByte, src, nCpyCnt);
		src += nCpyCnt;
		n -= nCpyCnt;

		// FIXME
		// 2018/3/21, update the curbits in current buffer position, and when skipping the left bits, modbits will be updated to the underlying buffer with left curbits
		if (bst_data->cursor.p + sizeof(CURBITS_TYPE) <= bst_data->cursor.p_end)
			bst_data->cursor.curbits = CURBITS_VALUE(bst_data->cursor.p);
		else
		{
			bst_data->cursor.curbits = 0;
			for (uint8_t* pbyte = bst_data->cursor.p; pbyte < bst_data->cursor.p_end; pbyte++) {
				bst_data->cursor.curbits <<= 8;
				bst_data->cursor.curbits |= *(uint8_t*)pbyte;
			}
		}

		_SkipBits(bst_data, nCpyCnt * 8);

#if 0

		if (nCpyCnt >= bst_data->cursor.bits_left/8 && bst_data->cursor.bits_left < (curbits_byte_cnt<<3))
		{
			nCpyCnt -= bst_data->cursor.bits_left / 8;
			// FIXME
			// 2018/3/21, update the curbits in current buffer position, and when skipping the left bits, modbits will be updated to the underlying buffer with left curbits
			if (bst_data->cursor.p + sizeof(CURBITS_TYPE) <= bst_data->cursor.p_end)
				bst_data->cursor.curbits = CURBITS_VALUE(bst_data->cursor.p);
			else
			{
				bst_data->cursor.curbits = 0;
				for (uint8_t* pbyte = bst_data->cursor.p; pbyte < bst_data->cursor.p_end; pbyte++) {
					bst_data->cursor.curbits <<= 8;
					bst_data->cursor.curbits |= *(uint8_t*)pbyte;
				}
			}
			_SkipBits(bst_data, bst_data->cursor.bits_left);
		}

		if (nCpyCnt / sizeof(CURBITS_TYPE) > 0 || bst_data->cursor.bits_left == (curbits_byte_cnt<<3))
		{
			bst_data->cursor.p += nCpyCnt / sizeof(CURBITS_TYPE) * sizeof(CURBITS_TYPE);
			if (bst_data->cursor.p >= )
			_UpdateCurBits(bst_data);
			nCpyCnt %= sizeof(CURBITS_TYPE);
		}

		if (nCpyCnt > 0)
		{
			_SkipBits(bst_data, nCpyCnt * 8);
			// FIXME
			// 2018/03/18, I am not sure why _UpdateCurBits need to be called, as my understanding
			// _SkipBits is called, and th curbits, modbits and other cursor value should be already updated
			// What's more it will cause bits_left value in cursor become mess, so comment the below line, but need more test.
			//_UpdateCurBits(bst_data);
		}
#endif
	} while (bst_data->cursor.p < bst_data->cursor.p_end && n > 0);

	return nTotalCpyCnt;
}

void AMBst_Destroy(AMBst& bst)
{
	if (bst == NULL)
		return;

	AM_BST_DATA* bst_data = (AM_BST_DATA*)bst;
	bst = NULL;

	if (bst_data->access_mode&BST_MODE_WRITE)
		_Flush(bst_data);

	if (bst_data->type == BST_TYPE_INNER_BUF)
	{
		AMP_SAFEDELA(bst_data->cursor.p_start);
	}

	delete bst_data;
}

template <typename T> T _GetTC(AMBst bst, int n){
	if (bst == NULL || n > (int)(sizeof(T)*8) || n <= 0)
		throw AMException(RET_CODE_INVALID_PARAMETER, _T("invalid parameter"));

	AM_BST_DATA* bst_data = (AM_BST_DATA*)bst;
	uint64_t val = _GetBits(bst_data, n);
	return (val>>(n-1))?
		(T)((((uint64_t)-1)<<n) | val):
		(T)val;
}

int8_t AMBst_GetTCChar(AMBst bst, int n)
{
	return _GetTC<int8_t>(bst, n);
}

int16_t AMBst_GetTCShort(AMBst bst, int n)
{
	return _GetTC<int16_t>(bst, n);
}

int32_t AMBst_GetTCLong(AMBst bst, int n)
{
	return _GetTC<int32_t>(bst, n);
}

int64_t AMBst_GetTCLongLong(AMBst bst, int n)
{
	return _GetTC<int64_t>(bst, n);
}

int64_t AMBst_Get_ae(AMBst bst)
{
	UNREFERENCED_PARAMETER(bst);
	return 0;
}

int64_t AMBst_Get_se(AMBst bst)
{
	int leadingZeroBits = -1;
	for(bool b = false; !b; leadingZeroBits++ )
		b = AMBst_GetBits(bst, 1)?true:false;

	if (leadingZeroBits >= 64)
		throw AMException(RET_CODE_BUFFER_NOT_COMPATIBLE, _T("Bitstream is not compatible"));

	uint64_t codeNum = (1LL<<leadingZeroBits) - 1LL + AMBst_GetBits(bst, leadingZeroBits);

	return (int64_t)(codeNum%2?((codeNum>>1) + 1):(~(codeNum>>1) + 1));
}

uint64_t AMBst_Get_ue(AMBst bst)
{
	int leadingZeroBits = -1;
	for(bool b = false; !b; leadingZeroBits++ )
		b = AMBst_GetBits(bst, 1)?true:false;

	if (leadingZeroBits >= 64)
		throw AMException(RET_CODE_BUFFER_NOT_COMPATIBLE, _T("Bitstream is not compatible"));

	return (1LL<<leadingZeroBits) - 1LL + AMBst_GetBits(bst, leadingZeroBits);
}

char* AMBst_Get_String(AMBst bst, bool bAlloc)
{
	UNREFERENCED_PARAMETER(bst);
	UNREFERENCED_PARAMETER(bAlloc);
	return NULL;
}

uint32_t AMBst_Get_uvlc(AMBst bst)
{
	if (bst == NULL)
		throw AMException(RET_CODE_INVALID_PARAMETER, _T("invalid parameter"));

	AM_BST_DATA* bst_data = (AM_BST_DATA*)bst;

	uint8_t leadingZeros = 0;
	while (1)
	{
		if (_GetBits(bst_data, 1))
			break;
		leadingZeros++;
	}

	if (leadingZeros >= 32)
		return UINT32_MAX;

	if (leadingZeros == 0)
		return 0;

	return (uint32_t)_GetBits(bst_data, leadingZeros) + (1 << leadingZeros) - 1;
}

uint64_t AMBst_Get_le(AMBst bst, int nBytes)
{
	if (bst == NULL || nBytes > 8)
		throw AMException(RET_CODE_INVALID_PARAMETER, _T("invalid parameter"));

	AM_BST_DATA* bst_data = (AM_BST_DATA*)bst;

	uint64_t t = 0;
	for (uint8_t i = 0; i < nBytes; i++)
		t += _GetBits(bst_data, 8) << (i * 8);
	return t;
}

uint64_t AMBst_Get_leb128(AMBst bst, uint8_t* pcbLeb128)
{
	if (bst == NULL)
		throw AMException(RET_CODE_INVALID_PARAMETER, _T("invalid parameter"));

	AM_BST_DATA* bst_data = (AM_BST_DATA*)bst;

	uint8_t i = 0;
	uint64_t value = 0;
	uint8_t Leb128Bytes = 0;
	for (; i < 8; i++)
	{
		uint8_t leb128_byte = (uint8_t)_GetBits(bst_data, 8);
		value |= (((uint64_t)leb128_byte & 0x7f) << (i * 7));
		Leb128Bytes += 1;
		if (!(leb128_byte & 0x80))
			break;
	}

	AMP_SAFEASSIGN(pcbLeb128, Leb128Bytes);
	
	if (value >= UINT32_MAX || i >= 8)
		return UINT64_MAX;

	return value;
}

int64_t	AMBst_Get_su(AMBst bst, int nBits)
{
	if (bst == NULL || nBits > 64)
		throw AMException(RET_CODE_INVALID_PARAMETER, _T("invalid parameter"));

	AM_BST_DATA* bst_data = (AM_BST_DATA*)bst;

	uint64_t value = _GetBits(bst_data, nBits);
	uint64_t signMask = 1ULL << (nBits - 1);
	if (value & signMask)
		return (int64_t)(value - (signMask << 1));

	return value;
}

uint64_t AMBst_Get_ns(AMBst bst, unsigned long long v_max)
{
	if (bst == NULL || v_max == 0)
		throw AMException(RET_CODE_INVALID_PARAMETER, _T("invalid parameter"));

	AM_BST_DATA* bst_data = (AM_BST_DATA*)bst;

	int8_t w = 0;
	unsigned long long vv = v_max;
	while (vv != 0)
	{
		vv = vv >> 1;
		w++;
	}
	
	unsigned long long m = (1ULL << w) - v_max;
	uint64_t v = _GetBits(bst_data, w - 1);

	if (v < m)
		return v;

	uint64_t extract_bit = _GetBits(bst_data, 1);
	return (v << 1) - m + extract_bit;
}

uint32_t AMBst_LatmGetValue(AMBst bst)
{
	uint8_t bytesForValue = (uint8_t)AMBst_GetBits(bst, 2);
	uint32_t value = 0;	/* helper variable 32bit */
	for (uint8_t i = 0; i <= bytesForValue; i++)
		value = (value << 8) | AMBst_GetByte(bst);
	return value;
}

BOOL AMBst_more_data(AMBst bst)
{
	if (bst == NULL)
		throw AMException(RET_CODE_INVALID_PARAMETER, _T("invalid parameter"));

	AM_BST_DATA* bst_data = (AM_BST_DATA*)bst;

	if (!(bst_data->access_mode&BST_MODE_READ))
		throw AMException(RET_CODE_ERROR_PERMISSION, _T("current bitstream handle can't read bits"));

	int bits_left = GetAllLeftBits(bst_data);

	if (bst_data->type == BST_TYPE_EXTERNAL_BUF)
	{
		return bits_left > 0 ? TRUE : FALSE;
	}
	else if (bst_data->type == BST_TYPE_INNER_BUF)
	{
		if (bst_data->rbsp_type == BST_RBSP_NAL_UNIT)
			return bits_left > 0 ? TRUE : (bst_data->cursor.nal_ring_bytes.length() > 0 ? TRUE : FALSE);
		return bits_left > 0 ? TRUE : FALSE;
	}

	// To be implement later
	throw AMException(RET_CODE_ERROR_NOTIMPL, _T("not implemented for inner buffer and ring chunk"));
}

BOOL AMBst_NAL_more_rbsp_data(AMBst bst)
{
	if (bst == NULL)
		throw AMException(RET_CODE_INVALID_PARAMETER, _T("invalid parameter"));

	AM_BST_DATA* bst_data = (AM_BST_DATA*)bst;

	if (!(bst_data->access_mode&BST_MODE_READ))
		throw AMException(RET_CODE_ERROR_PERMISSION, _T("current bitstream handle can't read bits"));

	if (bst_data->rbsp_type != BST_RBSP_NAL_UNIT)
		throw AMException(RET_CODE_MISMATCH, _T("more_rbsp_data must be used in NAL_UNIT rbsp type of bitstream"));

	/* no more data */
	if (!AMBst_more_data(bst))
		return FALSE;

	/* no rbsp_stop_bit yet */
	if (AMBst_PeekBits(bst, 1) == 0)
		return TRUE;

	// Create a restore point
	AM_BST_DATA::AM_BST_CURSOR restore_point = bst_data->cursor;

	BOOL bRet = FALSE;
	_GetBits(bst_data, 1);
	while (AMBst_more_data(bst))
	{
		/* A later bit was 1, it wasn't the rsbp_stop_bit */
		if (_GetBits(bst_data, 1) == 1)
		{
			bRet = TRUE;
			break;
		}
	}

	// Restore from the previous restore point
	bst_data->cursor = restore_point;

	return bRet;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
