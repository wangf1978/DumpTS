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
#include "AMSHA1.h"
#include <assert.h>

struct AMSHA1_DATA{
	unsigned H[5];						// Message digest buffers

	unsigned Length_Low;				// Message length in bits
	unsigned Length_High;				// Message length in bits

	unsigned char Message_Block[64];	// 512-bit message blocks
	int Message_Block_Index;			// Index into message block array

	bool Computed;						// Is the digest computed?
	bool Corrupted;						// Is the message digest corrupted?

	/*	
	 *	CircularShift
	 *
	 *	Description:
	 *		This member function will perform a circular shifting operation.
	 *
	 *	Parameters:
	 *		bits: [in]
	 *			The number of bits to shift (1-31)
	 *		word: [in]
	 *			The value to shift (assumes a 32-bit integer)
	 *
	 *	Returns:
	 *		The shifted value.
	 *
	 *	Comments:
	 */
	unsigned CircularShift(int bits, unsigned word)
	{
		return ((word << bits) & 0xFFFFFFFF) | ((word & 0xFFFFFFFF) >> (32-bits));
	}

	/*	
	 *	ProcessMessageBlock
	 *
	 *	Description:
	 *		This function will process the next 512 bits of the message
	 *		stored in the Message_Block array.
	 *
	 *	Parameters:
	 *		None.
	 *
	 *	Returns:
	 *		Nothing.
	 *
	 *	Comments:
	 *		Many of the variable names in this function, especially the single
	 *	 	character names, were used because those were the names used
	 *	  	in the publication.
	 */
	void ProcessMessageBlock()
	{
		const unsigned K[] = 	{ 				// Constants defined for SHA-1
									0x5A827999,
									0x6ED9EBA1,
									0x8F1BBCDC,
									0xCA62C1D6
								};
		int 		t;							// Loop counter
		unsigned 	temp;						// Temporary word value
		unsigned	W[80];						// Word sequence
		unsigned	A, B, C, D, E;				// Word buffers

		/*
		 *	Initialize the first 16 words in the array W
		 */
		for(t = 0; t < 16; t++){
			W[t] = ((unsigned) Message_Block[t * 4]) << 24;
			W[t] |= ((unsigned) Message_Block[t * 4 + 1]) << 16;
			W[t] |= ((unsigned) Message_Block[t * 4 + 2]) << 8;
			W[t] |= ((unsigned) Message_Block[t * 4 + 3]);
		}

		for(t = 16; t < 80; t++){
		   W[t] = CircularShift(1,W[t-3] ^ W[t-8] ^ W[t-14] ^ W[t-16]);
		}

		A = H[0];
		B = H[1];
		C = H[2];
		D = H[3];
		E = H[4];

		for(t = 0; t < 20; t++){
			temp = CircularShift(5,A) + ((B & C) | ((~B) & D)) + E + W[t] + K[0];
			temp &= 0xFFFFFFFF;
			E = D;
			D = C;
			C = CircularShift(30,B);
			B = A;
			A = temp;
		}

		for(t = 20; t < 40; t++){
			temp = CircularShift(5,A) + (B ^ C ^ D) + E + W[t] + K[1];
			temp &= 0xFFFFFFFF;
			E = D;
			D = C;
			C = CircularShift(30,B);
			B = A;
			A = temp;
		}

		for(t = 40; t < 60; t++){
			temp = CircularShift(5,A) +
		 		   ((B & C) | (B & D) | (C & D)) + E + W[t] + K[2];
			temp &= 0xFFFFFFFF;
			E = D;
			D = C;
			C = CircularShift(30,B);
			B = A;
			A = temp;
		}

		for(t = 60; t < 80; t++){
			temp = CircularShift(5,A) + (B ^ C ^ D) + E + W[t] + K[3];
			temp &= 0xFFFFFFFF;
			E = D;
			D = C;
			C = CircularShift(30,B);
			B = A;
			A = temp;
		}

		H[0] = (H[0] + A) & 0xFFFFFFFF;
		H[1] = (H[1] + B) & 0xFFFFFFFF;
		H[2] = (H[2] + C) & 0xFFFFFFFF;
		H[3] = (H[3] + D) & 0xFFFFFFFF;
		H[4] = (H[4] + E) & 0xFFFFFFFF;
	}

	/*	
	 *	PadMessage
	 *
	 *	Description:
	 *		According to the standard, the message must be padded to an even
	 *		512 bits.  The first padding bit must be a '1'.  The last 64 bits
	 *		represent the length of the original message.  All bits in between
	 *		should be 0.  This function will pad the message according to those
	 *		rules by filling the message_block array accordingly.  It will also
	 *		call ProcessMessageBlock() appropriately.  When it returns, it
	 *		can be assumed that the message digest has been computed.
	 *
	 *	Parameters:
	 *		None.
	 *
	 *	Returns:
	 *		Nothing.
	 *
	 *	Comments:
	 *
	 */
	void PadMessage()
	{
		/*
		 *	Check to see if the current message block is too small to hold
		 *	the initial padding bits and length.  If so, we will pad the
		 *	block, process it, and then continue padding into a second block.
		 */
		assert(Message_Block_Index >= 0 && Message_Block_Index < 64);
		if (Message_Block_Index > 55)
		{
			Message_Block[Message_Block_Index++] = 0x80;
			while(Message_Block_Index < 64)
			{
				Message_Block[Message_Block_Index++] = 0;
			}

			ProcessMessageBlock();
			Message_Block_Index = 0;

			while(Message_Block_Index < 56)
			{
				Message_Block[Message_Block_Index++] = 0;
			}
		}
		else
		{
			Message_Block[Message_Block_Index++] = 0x80;
			while(Message_Block_Index < 56)
			{
				Message_Block[Message_Block_Index++] = 0;
			}

		}

		/* Store the message length as the last 8 octets */
		Message_Block[56] = (Length_High >> 24) & 0xFF;
		Message_Block[57] = (Length_High >> 16) & 0xFF;
		Message_Block[58] = (Length_High >> 8) & 0xFF;
		Message_Block[59] = (Length_High) & 0xFF;
		Message_Block[60] = (Length_Low >> 24) & 0xFF;
		Message_Block[61] = (Length_Low >> 16) & 0xFF;
		Message_Block[62] = (Length_Low >> 8) & 0xFF;
		Message_Block[63] = (Length_Low) & 0xFF;

		ProcessMessageBlock();
		Message_Block_Index = 0;
	}
};

AMSHA1 AM_SHA1_Init(unsigned char* pBuf, unsigned long cbBuf)
{
	AMSHA1 handle = (AMSHA1)new AMSHA1_DATA;
	AM_SHA1_Reset(handle);
	if (pBuf != NULL && cbBuf > 0){
		if (AMP_FAILED(AM_SHA1_Input(handle, pBuf, cbBuf))){
			delete (AMSHA1_DATA*)handle;
			return NULL;
		}
	}

	return handle;
}

int AM_SHA1_Input(AMSHA1 handle, unsigned char* pBuf, unsigned long cbBuf)
{
	AMSHA1_DATA* sha1_data = (AMSHA1_DATA*)handle;
	
	if (sha1_data == NULL)
		return RET_CODE_INVALID_PARAMETER;

	if (pBuf == NULL || cbBuf <= 0)
		return RET_CODE_INVALID_PARAMETER;

	if (sha1_data->Computed || sha1_data->Corrupted)
	{
		sha1_data->Corrupted = true;
		return RET_CODE_ERROR;
	}

	while(cbBuf-- && !sha1_data->Corrupted)
	{
		assert(sha1_data->Message_Block_Index >= 0 && sha1_data->Message_Block_Index < 64);
		sha1_data->Message_Block[sha1_data->Message_Block_Index] = (*pBuf & 0xFF);

		sha1_data->Length_Low += 8;
		sha1_data->Length_Low &= 0xFFFFFFFF;				// Force it to 32 bits
		if (sha1_data->Length_Low == 0)
		{
			sha1_data->Length_High++;
			sha1_data->Length_High &= 0xFFFFFFFF;			// Force it to 32 bits
			if (sha1_data->Length_High == 0)
				sha1_data->Corrupted = true;				// Message is too long
		}

		if (++sha1_data->Message_Block_Index == 64)
		{
			sha1_data->ProcessMessageBlock();
			sha1_data->Message_Block_Index = 0;
		}

		pBuf++;
	}

	return RET_CODE_SUCCESS;
}

int AM_SHA1_Finalize(AMSHA1 handle)
{
	AMSHA1_DATA* sha1_data = (AMSHA1_DATA*)handle;
	
	if (sha1_data == NULL)
		return RET_CODE_INVALID_PARAMETER;

	if (sha1_data->Corrupted)
		return RET_CODE_ERROR;

	if (sha1_data->Computed)
		return RET_CODE_ALREADY_EXIST;

	sha1_data->PadMessage();
	sha1_data->Computed = true;

	return RET_CODE_SUCCESS;
}

int AM_SHA1_GetHash	(AMSHA1 handle, AMSHA1_RET& Result)
{
	AMSHA1_DATA* sha1_data = (AMSHA1_DATA*)handle;
	
	if (sha1_data == NULL)
		return RET_CODE_INVALID_PARAMETER;

	if(!sha1_data->Computed)
		return RET_CODE_NOT_FINALIZED;

	for(int i = 0; i < 5; i++)
	{
		Result[0 + 4*i] = (unsigned char)(sha1_data->H[i]>>24);
		Result[1 + 4*i] = (unsigned char)(sha1_data->H[i]>>16);
		Result[2 + 4*i] = (unsigned char)(sha1_data->H[i]>> 8);
		Result[3 + 4*i] = (unsigned char)(sha1_data->H[i]    );
	}

	return RET_CODE_SUCCESS;
}

void AM_SHA1_Reset(AMSHA1 handle)
{
	AMSHA1_DATA* sha1_data = (AMSHA1_DATA*)handle;
	
	if (sha1_data == NULL)
		return;

	sha1_data->Length_Low			= 0;
	sha1_data->Length_High			= 0;
	sha1_data->Message_Block_Index	= 0;

	sha1_data->H[0]		= 0x67452301;
	sha1_data->H[1]		= 0xEFCDAB89;
	sha1_data->H[2]		= 0x98BADCFE;
	sha1_data->H[3]		= 0x10325476;
	sha1_data->H[4]		= 0xC3D2E1F0;

	sha1_data->Computed	= false;
	sha1_data->Corrupted= false;
}

void AM_SHA1_Uninit(AMSHA1& handle)
{
	AMSHA1_DATA* sha1_data = (AMSHA1_DATA*)handle;
	
	if (sha1_data == NULL)
		return;

	delete sha1_data;
	handle = NULL;
}
