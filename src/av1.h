#pragma once

#include <cstdint>
#include "Bitstream.h"

#define Abs(x)				((x)>=0?(x):-(x))
#define Clip3(x,y,z)		((z)<(x)?(x):((z) > (y)?(y):(z)))
#define Clip1(x, bitdepth)	Clip3(0, (1<<bitdepth)-1, x)
#define Min(x, y)			((x) <= (y)?(x):(y))
#define Max(x, y)			((x) >= (y)?(x):(y))
#define Round2(x, n)		((x + (1<<(n-1)))/(1<<n))
#define Round2Signed(x, n)	((x) >= 0?Round2(x, n):Round2(-x, n))

#define OBU_SEQUENCE_HEADER			1
#define OBU_TEMPORAL_DELIMITER		2
#define OBU_FRAME_HEADER			3
#define OBU_TILE_GROUP				4
#define OBU_METADATA				5
#define OBU_FRAME					6
#define OBU_REDUNDANT_FRAME_HEADER	7
#define OBU_PADDING					15

namespace AV1
{
	template <typename T>
	inline int8_t FloorLog2(T x)
	{
		int8_t s = 0;
		while (x != 0)
		{
			x = x >> 1;
			s++;
		}
		return s - 1;
	}

	template <typename T>
	inline int8_t CeilLog2(T x)
	{
		if (x < 2)
			return 0;

		int8_t i = 1;
		T p = 2;
		while (p < x) {
			i++;
			p = p << 1;
		}

		return i
	}

	uint64_t f(CBitstream& bs, uint8_t n)
	{
		return bs.GetBits(n);
	}

	uint64_t uvlc(CBitstream& bs)
	{
		uint8_t leadingZeros = 0;
		while (1)
		{
			if (bs.GetBits(1))
				break;
			leadingZeros++;
		}

		if (leadingZeros >= 32)
			return UINT32_MAX;
		
		return (uint32_t)bs.GetBits(leadingZeros) + (1 << leadingZeros) - 1;
	}

	uint64_t le(CBitstream& bs, uint8_t nBytes)
	{
		uint64_t t = 0;
		for (uint8_t i = 0; i < nBytes; i++)
			t += bs.GetBits(8) << (i + 8);
		return t;
	}

	uint64_t leb128(CBitstream& bs)
	{
		uint64_t value = 0;
		uint64_t Leb128Bytes = 0;
		for (uint8_t i = 0; i < 8; i++)
		{
			uint8_t leb128_byte = bs.GetByte();
			value |= (((uint64_t)leb128_byte & 0x7f) << (i * 7));
			Leb128Bytes += 1;
			if (!(leb128_byte & 0x80))
				break;
		}

		return value;
	}

	int64_t su(CBitstream& bs, uint8_t n)
	{
		uint64_t value = f(bs, n);
		uint64_t signMask = 1 << (n - 1);
		if (value & signMask)
		{
			if (n == 64)
				return -(~value) - 1LL;
				
			return (int64_t)(value - (signMask << 1));
		}

		return value;
	}

	struct OPEN_BITSTREAM_UNIT
	{
		struct OBU_HEADER
		{
			uint8_t obu_forbidden_bit : 1;
			uint8_t obu_type : 4;
			uint8_t obu_extension_flag : 1;
			uint8_t obu_has_size_field : 1;
			uint8_t obu_reserved_1bit : 1;

			uint8_t temporal_id : 3;
			uint8_t spatial_id : 2;
			uint8_t extension_header_reserved_3bits : 3;
		};

		OBU_HEADER	obu_header;
		uint64_t	obu_size;

	};

	int DumpAV1()
	{
		CFileBitstream bs("d:\\140626_av1_720p_850kbps.av1", 4096);

		//uint64_t len = leb128(bs);

		OPEN_BITSTREAM_UNIT obu;

		obu.obu_header.obu_forbidden_bit = (uint8_t)f(bs, 1);
		obu.obu_header.obu_type = (uint8_t)f(bs, 4);
		obu.obu_header.obu_extension_flag = (uint8_t)f(bs, 1);
		obu.obu_header.obu_has_size_field = (uint8_t)f(bs, 1);
		obu.obu_header.obu_reserved_1bit = (uint8_t)f(bs, 1);

		if (obu.obu_header.obu_extension_flag)
		{
			obu.obu_header.temporal_id = (uint8_t)f(bs, 3);
			obu.obu_header.spatial_id = (uint8_t)f(bs, 2);
			obu.obu_header.extension_header_reserved_3bits = (uint8_t)f(bs, 3);
		}

		if (obu.obu_header.obu_has_size_field)
			obu.obu_size = leb128(bs);
		else
			obu.obu_size = 0;

		return 0;
	}


} // AV1