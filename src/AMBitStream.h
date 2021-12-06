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
#ifndef _CSTYLE_BITSTREAM_H_
#define _CSTYLE_BITSTREAM_H_

#include "AMRingBuffer.h"
#include <stdint.h>

// Finally we need implement 3 types of bitstream 
// Type 1: 
//		give a fixed buffer with a specified size
// Type 2:
//		give a ring buffer, and read the bits from the ring buffer
// Type 3:
//      maintain a self-buffer, and provide a callback, when the data is exhausted, try to call it, and fill this buffer from the specified position.

//---------------------------------------------------------------------------------------------------
//-- mode --------- External Buffer --------- Ring Buffer ------------- Inner-Buffer with callback---
//  READ				O						O									O
//  WRITE				O						X									X
//---------------------------------------------------------------------------------------------------
// SEQUENCE			READ/WRITE				   READ								READ/WRITE
// NAL_UNIT			   READ					   READ								   READ
//---------------------------------------------------------------------------------------------------

/*
For the sequence type, 
-----------------------------------------------------------------------------------------------------
                             |                                |
							 p                                next_p
							 |------> read -----|----left-----|              
*/

typedef void* AMBst;

/*!	@brief the callback function to fill buffer 
	@param buf the start buffer to be filled from
	@param cbSize the buffer size to be available for filling data 
	@param bEos the data are filled completely 
	@param pUserData the user data set together with callback */
typedef int (/*_cdecl */*AMBstFillBuffer)(unsigned char* buf, int cbSize, bool& bEos, void* pUserData);

enum AMBstAlign {
	BST_ALIGN_BYTE,
	BST_ALIGN_WORD,
	BST_ALIGN_DWORD,
	BST_ALIGN_QWORD,
	BST_ALIGN_NEXT,				// realign to the next cache bits
	BST_ALIGN_BEGIN,			// realign to its start position
	BST_ALIGN_END				// realign to the end position, it may trigger request new bitstream
};

enum AMBstAccessMode {
	BST_MODE_READ				= 1,
	BST_MODE_WRITE				= 2,
	BST_MODE_READ_WRITE			= (BST_MODE_READ | BST_MODE_WRITE)
};

enum AMBstRBSPType {
	BST_RBSP_SEQUENCE,			// general one-by-one read/write mode
	BST_RBSP_NAL_UNIT,			// skip/add emulation_prevention_three_byte (00 00 03)
	BST_RBSP_SEQUENCE_14BIT_PER_BE16BIT,
								// Every WORD, only the lower 14bits are available, it is used in DTS case
	BST_RBSP_SEQUENCE_14BIT_PER_LE16BIT,
								// Every WORD, only the lower 14bits are available, it is used in DTS case
	BST_RBSP_SEQUENCE_LE16BIT,	// The bitstream is aligned with 16bit, and 16-bit word is little-endian 
};

enum AMBstBitFillMode {
	BST_BIT_FILL_ZERO,			// Fill 1 for one bit
	BST_BIT_FILL_ONE,			// Fill 0 for one bit
	BST_BIT_FILL_KEEP,			// Keep the original bit value
	BST_BIT_FILL_NOTHING,		// Nothing to do, only skip the bit position
};

struct ESC_BYTE_STREAM_NAL_UNIT
{


};

#define BUF_FILL_ERROR			-1
#define BUF_FILL_EOD			-2
#define NULL_BST				NULL

/*******************************************************************************
// Heap management function
********************************************************************************/
AMP_FOUNDATION_PROC void*		AMBst_Alloc			(AMBst bst, int cbSize);
AMP_FOUNDATION_PROC void		AMBst_Free			(AMBst bst, void* buf);

/*******************************************************************************
// Bitstream constructor functions
********************************************************************************/
AMP_FOUNDATION_PROC AMBst		AMBst_CreateFromBuffer
													(uint8_t* pBuffer, int cbSize, int access_mode=BST_MODE_READ);
AMP_FOUNDATION_PROC AMBst		AMBst_CreateFromCallback
													(AMBstFillBuffer funptrFillBuffer, void* pUserData, int cbUnderlyingBufSize=65536, int access_mode=BST_MODE_READ);
#if 0
AMP_FOUNDATION_PROC AMBst		AMBst_CreateFromRingBuffer
													(AMRingChunk pRingChunk, int access_mode=BST_MODE_READ);
#endif

AMP_FOUNDATION_PROC AMBst		AMBst_Create();

AMP_FOUNDATION_PROC int			AMBst_AttachBuffer(AMBst bst, uint8_t* pBuffer, int cbSize, int access_mode = BST_MODE_READ);

AMP_FOUNDATION_PROC	void		AMBst_DetachBuffer(AMBst bst);

/*!	@brief Create the sub-set of a bitstream */
AMP_FOUNDATION_PROC AMBst		AMBst_Subset(AMBst bst, int nBits);

/*!	@brief Set the raw byte stream payload type since the current bit position. 
	@param rbsp_type raw byte stream payload type
	@remark if rbsp_type is sequence type, read method will read bits one by one without any skipping, 
	write method will write bits one by one without additional bits; if sequence type is nal_unit mode, 
	read method will skip '03' byte in emulation_prevention_three_byte, write method will write back 
	emulation_prevention_three_byte for bit string '00 00 00/01/02'. */
AMP_FOUNDATION_PROC int			AMBst_SetRBSPType(AMBst bst, AMBstRBSPType rbsp_type);

/*!	@brief Mark the end position of current bitstream. 
	@param end_pos the end position of current bit-stream, -1 is passed, the original end position will be restored. */
AMP_FOUNDATION_PROC int			AMBst_MarkEndPos(AMBst bst, int end_pos);

/*******************************************************************************
// Get the value from the current bitstream with big-endian byte order
********************************************************************************/
AMP_FOUNDATION_PROC uint64_t	AMBst_GetBits		(AMBst bst, int n);
/*!	@brief Peek bits value without changing bitstream status
	@remarks if bitstream is related with Ring-Chunk(type=BST_TYPE_RINGCHUNK), in the current chunk,
	the left bits is not enough for peeking, the exception will be raised. */
AMP_FOUNDATION_PROC uint64_t	AMBst_PeekBits		(AMBst bst, int n);
/*!	@brief Get one byte from bit-stream. */
AMP_FOUNDATION_PROC uint8_t		AMBst_GetByte		(AMBst bst);
/*!	@brief Get one 16-bit unsigned short integer/WORD from bit-stream. */
AMP_FOUNDATION_PROC uint16_t	AMBst_GetWord		(AMBst bst);
/*!	@brief Get one 32-bit unsigned integer/DWORD from bit-stream. */
AMP_FOUNDATION_PROC uint32_t	AMBst_GetDWord		(AMBst bst);
/*!	@brief Get one 64-bit unsigned integer/QWORD from bit-stream. */
AMP_FOUNDATION_PROC uint64_t	AMBst_GetQWord		(AMBst bst);
/*!	@brief Get one signed character from bit-stream. */
AMP_FOUNDATION_PROC int8_t		AMBst_GetChar		(AMBst bst);
/*!	@brief Get one 16-bit signed short integer from bit-stream. */
AMP_FOUNDATION_PROC int16_t		AMBst_GetShort		(AMBst bst);
/*!	@brief Get one 32-bit signed integer from bit-stream. */
AMP_FOUNDATION_PROC int32_t		AMBst_GetLong		(AMBst bst);
/*!	@brief Get one 64-bit signed integer from bit-stream. */
AMP_FOUNDATION_PROC int64_t		AMBst_GetLongLong	(AMBst bst);

// For H264/H265 NAL syntax
/*!	@brief Get a signed char with Two's Component */
AMP_FOUNDATION_PROC int8_t		AMBst_GetTCChar		(AMBst bst, int n);
/*!	@brief Get a signed short with Two's Component */
AMP_FOUNDATION_PROC int16_t		AMBst_GetTCShort	(AMBst bst, int n);
/*!	@brief Get a signed long integer with Two's Component */
AMP_FOUNDATION_PROC int32_t		AMBst_GetTCLong		(AMBst bst, int n);
/*!	@brief Get a signed long long integer with Two's Component */
AMP_FOUNDATION_PROC int64_t		AMBst_GetTCLongLong	(AMBst bst, int n);

/*!	@brief context-adaptive arithmetic entropy-coded syntax element. The parsing process for this descriptor is specified in clause 9.3 */
AMP_FOUNDATION_PROC int64_t		AMBst_Get_ae		(AMBst bst);
/*!	@brief signed integer 0-th order Exp-Golomb-coded syntax element with the left bit first. The parsing process for this descriptor is specified in clause 9.2 */
AMP_FOUNDATION_PROC int64_t		AMBst_Get_se		(AMBst bst);
/*!	@brief unsigned integer 0-th order Exp-Golomb-coded syntax element with the left bit first. The parsing process for this descriptor is specified in clause 9.2 */
AMP_FOUNDATION_PROC uint64_t	AMBst_Get_ue		(AMBst bst);
/*!	@brief Get null-terminated UTF-8 string. 
	@param bst the current bit-stream 
	@param bAlloc the returned string is allocated from heap or not. 
	@return null-terminated string encoded as universal coded character set (UCS) transmission format-8 (UTF-8) characters as specified in ISO/IEC 10646 */
AMP_FOUNDATION_PROC char*		AMBst_Get_String	(AMBst bst, bool bAlloc);

// For AOM AV-1
/*!	@brief Variable length unsigned n-bit number appearing directly in the bitstream. */
AMP_FOUNDATION_PROC uint32_t	AMBst_Get_uvlc		(AMBst bst);
/*! @brief Unsigned little-endian n-byte number appearing directly in the bitstream. */
AMP_FOUNDATION_PROC	uint64_t	AMBst_Get_le		(AMBst bst, int nBytes);
/*!	@brief Unsigned integer represented by a variable number of little-endian bytes. 
	@remarks In this encoding, the most significant bit of each byte is equal to 1 to signal that more bytes should be read, or equal to 0 to signal the end of the encoding.*/
AMP_FOUNDATION_PROC uint64_t	AMBst_Get_leb128(AMBst bst, uint8_t* pcbLeb128 = NULL);
/*! @brief Signed integer converted from an n bits unsigned integer in the bitstream. */
AMP_FOUNDATION_PROC	int64_t		AMBst_Get_su(AMBst bst, int nBits);
/*! @brief Unsigned encoded integer with maximum number of values n (i.e. output in range 0..n-1). */
AMP_FOUNDATION_PROC	uint64_t	AMBst_Get_ns(AMBst bst, unsigned long long v);

// For MPEG-4 AAC
AMP_FOUNDATION_PROC uint32_t	AMBst_LatmGetValue(AMBst bst);


/*!	@brief Copy the bytes to the destination buffer from the current bitstream. */
AMP_FOUNDATION_PROC int			AMBst_GetBytes		(AMBst bst, uint8_t* dest, int n);

/*******************************************************************************
// Put the value from the current bitstream with big-endian byte order
********************************************************************************/
AMP_FOUNDATION_PROC int			AMBst_PutBits		(AMBst bst, int n, uint64_t val);
AMP_FOUNDATION_PROC int			AMBst_ReservedBits	(AMBst bst, int n, BOOL ReservedBitSet=TRUE);
AMP_FOUNDATION_PROC int			AMBst_PutByte		(AMBst bst, uint8_t byte);
AMP_FOUNDATION_PROC int			AMBst_PutWord		(AMBst bst, uint16_t word);
AMP_FOUNDATION_PROC int			AMBst_PutDWord		(AMBst bst, uint32_t dword);
AMP_FOUNDATION_PROC int			AMBst_PutQWord		(AMBst bst, uint64_t qword);
AMP_FOUNDATION_PROC int			AMBst_PutChar		(AMBst bst, int8_t schar);
AMP_FOUNDATION_PROC int			AMBst_PutShort		(AMBst bst, int16_t sshort);
AMP_FOUNDATION_PROC int			AMBst_PutLong		(AMBst bst, int32_t slong);
AMP_FOUNDATION_PROC int			AMBst_PutLongLong	(AMBst bst, int64_t slonglong);

AMP_FOUNDATION_PROC	int			AMBst_PutBytes		(AMBst bst, uint8_t* src, int n);

// Only available for write operation
AMP_FOUNDATION_PROC int			AMBst_Flush			(AMBst bst);

/*!	@brief Return the position of bit-stream in unit of bits. 
	@brief For NAL_UNIT rbsp, this function can't be used to tell the original bit position in original raw string. */
AMP_FOUNDATION_PROC int			AMBst_Tell			(AMBst bst, int* left_bits_in_bst=NULL);
/*!	@brief Seek to the absolute position in the specified bit-stream. */
AMP_FOUNDATION_PROC int			AMBst_Seek			(AMBst bst, int bit_pos);

/*!	@brief Realign the read position.
	@param bitFillMode If there are still some bits in the cache bits block which are not modified, how to fill them
	@remarks If the bitstream is related with Ring Chunk(type=BST_TYPE_RINGCHUNK), realign behavior will
	change bitstream read position when the next align position can't reach, it will reach the bitstream 
	end position instead of keep the original position, it is not like as the scenario of external buffer 
	and inner buffer. */
AMP_FOUNDATION_PROC int			AMBst_Realign		(AMBst bst, AMBstAlign bstAlign = BST_ALIGN_BYTE, AMBstBitFillMode bitFillMode=BST_BIT_FILL_KEEP);
AMP_FOUNDATION_PROC int			AMBst_SkipBits		(AMBst bst, int skip_bits);
/*!	@brief Get the current pointer, and return the left bytes.
	@remark cbLeftBytes is only accurate for the bitstream with type#1. */
AMP_FOUNDATION_PROC uint8_t*	AMBst_LockCurPtr	(AMBst bst, int* nBitOffsetFromCurPtr, int* cbLeftBytes=NULL);
AMP_FOUNDATION_PROC void		AMBst_UnlockCurPtr	(AMBst bst, int cbUpdated);


/*******************************************************************************
H.264 Specification of syntax functions
byte_aligned() --> AMBst_IsAligned
- If the current position in the bitstream is on a byte boundary, i.e., the next bit in the bitstream is the first bit in a byte, the return value of byte_aligned( ) is equal to TRUE.
- Otherwise, the return value of byte_aligned( ) is equal to FALSE.

more_data_in_byte_stream( ) --> AMBst_more_data
- If more data follow in the byte stream, the return value of more_data_in_byte_stream( ) is equal to TRUE.
- Otherwise, the return value of more_data_in_byte_stream( ) is equal to FALSE.

more_rbsp_data() --> AMBst_NAL_more_rbsp_data
- If there is no more data in the RBSP, the return value of more_rbsp_data( ) is equal to FALSE.
- Otherwise, the RBSP data is searched for the last (least significant, right-most) bit equal to 1 that is present in the RBSP. Given the position of this bit, which is the first bit (rbsp_stop_one_bit) of the rbsp_trailing_bits( ) syntax structure, the following applies:
- If there is more data in an RBSP before the rbsp_trailing_bits( ) syntax structure, the return value of more_rbsp_data( ) is equal to TRUE.
- Otherwise, the return value of more_rbsp_data( ) is equal to FALSE.
The method for enabling determination of whether there is more data in the RBSP is specified by the application (or in Annex B for applications that use the byte stream format).
********************************************************************************/
AMP_FOUNDATION_PROC BOOL		AMBst_IsAligned		(AMBst bst, AMBstAlign bstAlign = BST_ALIGN_BYTE);
AMP_FOUNDATION_PROC BOOL		AMBst_more_data		(AMBst bst);
AMP_FOUNDATION_PROC BOOL		AMBst_NAL_more_rbsp_data
													(AMBst bst);

/*******************************************************************************
// Bitstream destructor functions
********************************************************************************/
AMP_FOUNDATION_PROC void		AMBst_Destroy		(AMBst& bst);

class CAMBstRef
{
public:
	CAMBstRef(AMBst bst) :m_bst(bst) {
	}
	~CAMBstRef() {
		AMBst_Destroy(m_bst);
	}
	operator AMBst() const throw() {
		return m_bst;
	}

private:
	AMBst	m_bst;
};

#endif
