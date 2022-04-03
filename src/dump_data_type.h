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
#ifndef __DUMP_DATA_TYPE_H__
#define __DUMP_DATA_TYPE_H__

#pragma once

#include <functional>
#include <stdint.h>
#include "AMSHA1.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4200)
#pragma warning(disable:4201)
#pragma warning(disable:4101)
#pragma warning(disable:4100)
#pragma warning(disable:4189)
#pragma warning(disable:4127)
#pragma pack(push,1)
#define PACKED
#else
#define PACKED __attribute__ ((__packed__))
#endif

using TM_27MHZ = int64_t;
using TM_90KHZ = int64_t;
using TM_45KHZ = int64_t;
using RET_CODE = int32_t;
using TM_HNS = int64_t;
using TS_PID = uint16_t;

union RET_READ
{
	RET_CODE	err_ret_code;	// If read failed, err_ret_code return the error code which is less than 0
	uint32_t	read_count;		// If read successfully, ret_count return the read count

	RET_READ(RET_CODE code) : err_ret_code(code) {}
	RET_READ(uint32_t nRead) : read_count(nRead) {}
};

enum FRAGMENTATION_INDICATOR
{
	FRAG_INDICATOR_COMPLETE = 0,
	FRAG_INDICATOR_FIRST,
	FRAG_INDICATOR_MIDDLE,
	FRAG_INDICATOR_LAST
};

struct PROCESS_DATA_INFO
{
	uint32_t		valid;	// Indicate there is valid data in the below information
							// 0: no valid data information
	union
	{
		struct
		{
			// For MMT/TLV
			FRAGMENTATION_INDICATOR
							indicator;
			uint16_t		CID;
			uint16_t		packet_id;
			uint32_t		packet_sequence_number;
			uint32_t		MPU_sequence_number;
		};
	};
	// For future expansion
};

struct ACCESS_UNIT_INFO
{
	uint64_t		AU_byte_pos;
	union
	{
		struct
		{
			uint32_t		picture_type : 3;
			uint32_t		hevc_picture_slice_type : 2;// slice_type values that may be present in the coded picture, 2: I; 1: P; 0: B
			uint32_t		reserved_for_future_use_0_hevc : 1;
			uint32_t		sequence_change_point : 1;
			uint32_t		reserved_for_future_use_1_hevc : 25;
		}PACKED;
		struct
		{
			uint32_t		IDR : 1;					//	1: IDR picture; 0: non-IDR picture
			uint32_t		avc_picture_slice_type : 4;	//	0:	P(P slice)
														//	1:	B(B slice)
														//	2:	I(I slice)
														//	3:	SP(SP slice)
														//	4:	SI(SI slice)
														//	5:	P(P slice)
														//	6:	B(B slice)
														//	7:	I(I slice)
														//	8:	SP(SP slice)
														//	9:	SI(SI slice)
			uint32_t		reserved_for_future_use_0_avc : 1;
			uint32_t		sequence_change_point_avc : 1;
			uint32_t		reserved_for_future_use_1_avc : 25;
		}PACKED;
	};
}PACKED;

using CB_AU_STARTPOINT = std::function<void(TM_90KHZ, TM_90KHZ, const PROCESS_DATA_INFO*, const ACCESS_UNIT_INFO*, void*)>;

struct SHA1HashVaue
{
	bool		fValid = false;
	AMSHA1_RET	hash = { 0 };

	SHA1HashVaue(){}

	SHA1HashVaue(const uint8_t* pBuf, size_t cbBuf) { UpdateHash(pBuf, cbBuf); }

	void UpdateHash(const uint8_t* pBuf, size_t cbBuf) {
		if (pBuf == nullptr || cbBuf == 0){
			fValid = false;
			return;
		}

		AMSHA1 hSha1 = AM_SHA1_Init((uint8_t*)pBuf, (unsigned long)cbBuf);
		if (hSha1 != nullptr){
			if (AMP_SUCCEEDED(AM_SHA1_Finalize(hSha1)) && AMP_SUCCEEDED(AM_SHA1_GetHash(hSha1, hash)))
				fValid = true;
			else
				fValid = false;
			AM_SHA1_Uninit(hSha1);
		}
		else
			fValid = false;
	}
};

inline bool operator==(SHA1HashVaue const &hash_val1, SHA1HashVaue const &hash_val2){
	return (hash_val1.fValid == hash_val2.fValid && hash_val1.fValid && memcmp(hash_val1.hash, hash_val2.hash, sizeof(hash_val1.hash)) == 0) ? true : false;
}

inline bool operator==(SHA1HashVaue const &hash_val1, AMSHA1_RET hash_val2){
	return (hash_val1.fValid && memcmp(hash_val1.hash, hash_val2, sizeof(hash_val1.hash)) == 0) ? true : false;
}

inline bool operator==(AMSHA1_RET hash_val1, SHA1HashVaue const &hash_val2){
	return (hash_val2.fValid && memcmp(hash_val2.hash, hash_val1, sizeof(hash_val2.hash)) == 0) ? true : false;
}

inline bool operator!=(SHA1HashVaue const &hash_val1, SHA1HashVaue const &hash_val2) {
	return (hash_val1.fValid == hash_val2.fValid && hash_val1.fValid && memcmp(hash_val1.hash, hash_val2.hash, sizeof(hash_val1.hash)) == 0) ? false : true;
}

inline bool operator!=(SHA1HashVaue const &hash_val1, AMSHA1_RET hash_val2) {
	return (hash_val1.fValid && memcmp(hash_val1.hash, hash_val2, sizeof(hash_val1.hash)) == 0) ? false : true;
}

inline bool operator!=(AMSHA1_RET hash_val1, SHA1HashVaue const &hash_val2) {
	return (hash_val2.fValid && memcmp(hash_val2.hash, hash_val1, sizeof(hash_val2.hash)) == 0) ? false : true;
}

#ifdef _MSC_VER
#pragma pack(pop)
#pragma warning(pop)
#endif
#undef PACKED

#endif
