/**********************************************************************
 *
 * Filename:    crc.c
 *
 * Description: Source de calculo CRC
 *
 * Notes:
 *
 *
 *
 * Copyright (c) 2014 Francisco Javier Lana Romero
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
 
#include "platcomm.h"
#include "crc.h"
#include <stdio.h>
#include <string.h>

struct CRC_PROP
{
	uint64_t	Polynomial;
	uint64_t	Initvalue;
	uint64_t	XORout;
	uint64_t	Check;
	uint64_t	CRCLookupTable[256];
	const char*	Name;
	uint8_t		width;
	bool		Reversed;
	bool		initialized;
};

CRC_PROP crc_props[] = 
{
/*--Polynomial-------------Init-Value----------XORout---------------Check--------CRCLookupTable------Name---------------------Width----Reserved----initialized---*/
	{0x07,					0x00,				0x00,				0xF4,				 {0},		"crc-8",					8,		false,		false },
	{0x39,					0x00,				0x00,				0x15,				 {0},		"crc-8-darc",				8,		true,		false },
	{0x1D,					0xFD,				0x00,				0x7E,				 {0},		"crc-8-i-code",				8,		false,		false },
	{0x07,					0x55,				0x55,				0xA1,				 {0},		"crc-8-itu",				8,		false,		false },
	{0x31,					0x00,				0x00,				0xA1,				 {0},		"crc-8-maxim",				8,		true,		false },
	{0x07,					0xFF,				0x00,				0xD0,				 {0},		"crc-8-rohc",				8,		true,		false },
	{0x9B,					0x00,				0x00,				0x25,				 {0},		"crc-8-wcdma",				8,		true,		false },
	{0x8005,				0x0000,				0x0000,				0xBB3D,				 {0},		"crc-16",					16,		true,		false },
	{0x8005,				0x0000,				0x0000,				0xFEE8,				 {0},		"crc-16-buypass",			16,		false,		false },
	{0x8005,				0x800D,				0x0000,				0x9ECF,				 {0},		"crc-16-dds-110",			16,		false,		false },
	{0x0589,				0x0001,				0x0001,				0x007E,				 {0},		"crc-16-dect",				16,		false,		false },
	{0x3D65,				0xFFFF,				0xFFFF,				0xEA82,				 {0},		"crc-16-dnp",				16,		true,		false },
	{0x3D65,				0xFFFF,				0xFFFF,				0xC2B7,				 {0},		"crc-16-en-13757",			16,		false,		false },
	{0x1021,				0x0000,				0xFFFF,				0xD64E,				 {0},		"crc-16-genibus",			16,		false,		false },
	{0x8005,				0xFFFF,				0xFFFF,				0x44C2,				 {0},		"crc-16-maxim",				16,		true,		false },
	{0x1021,				0xFFFF,				0x0000,				0x6F91,				 {0},		"crc-16-mcrf4xx",			16,		true,		false },
	{0x1021,				0x554D,				0x0000,				0x63D0,				 {0},		"crc-16-riello",			16,		true,		false },
	{0x8BB7,				0x0000,				0x0000,				0xD0DB,				 {0},		"crc-16-t10-dif",			16,		false,		false },
	{0xA097,				0x0000,				0x0000,				0x0FB3,				 {0},		"crc-16-teledisk",			16,		false,		false },
	{0x8005,				0x0000,				0xFFFF,				0xB4C8,				 {0},		"crc-16-usb",				16,		true,		false },
	{0x1021,				0x0000,				0xFFFF,				0x906E,				 {0},		"x-25",						16,		true,		false },
	{0x1021,				0x0000,				0x0000,				0x31C3,				 {0},		"xmodem",					16,		false,		false },
	{0x8005,				0xFFFF,				0x0000,				0x4B37,				 {0},		"modbus",					16,		true,		false },
	{0x1021,				0x0000,				0x0000,				0x2189,				 {0},		"kermit",					16,		true,		false },
	{0x1021,				0xFFFF,				0x0000,				0x29B1,				 {0},		"crc-ccitt-false",			16,		false,		false },
	{0x1021,				0x1D0F,				0x0000,				0xE5CC,				 {0},		"crc-aug-ccitt",			16,		false,		false },
	{0x864CFB,				0xB704CE,			0x000000,			0x21CF02,			 {0},		"crc-24",					24,		false,		false },
	{0x5D6DCB,				0xFEDCBA,			0x000000,			0x7979BD,			 {0},		"crc-24-flexray-a",			24,		false,		false },
	{0x5D6DCB,				0xABCDEF,			0x000000,			0x1F23B8,			 {0},		"crc-24-flexray-b",			24,		false,		false },
	//{0x04C11DB7,			0x00000000,			0xFFFFFFFF,			0xCBF43926,			 {0},		"crc-32",					32,		true,		false },
	{0x04C11DB7,			0xFFFFFFFF,			0xFFFFFFFF,			0xCBF43926,			 {0},		"crc-32",					32,		true,		false },
	{0x04C11DB7,			0xFFFFFFFF,			0x00000000,			0xFC891918,			 {0},		"crc-32-bzip2",				32,		false,		false },
	{0x1EDC6F41,			0x00000000,			0xFFFFFFFF,			0xE3069283,			 {0},		"crc-32c",					32,		true,		false },
	{0xA833982B,			0x00000000,			0xFFFFFFFF,			0x87315576,			 {0},		"crc-32d",					32,		true,		false },
	{0x04C11DB7,			0xFFFFFFFF,			0x00000000,			0x0376E6E7,			 {0},		"crc-32-mpeg",				32,		false,		false },
	{0x04C11DB7,			0xFFFFFFFF,			0xFFFFFFFF,			0x765E7680,			 {0},		"posix",					32,		false,		false },
	{0x814141AB,			0x00000000,			0x00000000,			0x3010BF7F,			 {0},		"crc-32q",					32,		false,		false },
	{0x04C11DB7,			0xFFFFFFFF,			0x00000000,			0x340BC6D9,			 {0},		"jamcrc",					32,		true,		false },
	{0x000000AF,			0x00000000,			0x00000000,			0xBD0BE338,			 {0},		"xfer",						32,		false,		false },
	{0x000000000000001B,	0x0000000000000000,	0x0000000000000000,	0x46A5A9388A5BEFFE,	 {0},		"crc-64",					64,		true,		false },
	{0x42F0E1EBA9EA3693,	0x0000000000000000,	0xFFFFFFFFFFFFFFFF,	0x62EC59E3F1A4F00A,	 {0},		"crc-64-we",				64,		false,		false },
	{0xAD93D23594C935A9,	0xFFFFFFFFFFFFFFFF,	0x0000000000000000,	0xCAA717168609F281,	 {0},		"crc-64-jones",				64,		true,		false }
};

static const uint8_t BitReverseTable256[] =
{
	0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
	0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
	0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
	0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
	0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
	0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
	0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
	0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
	0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
	0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
	0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
	0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
	0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
	0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
	0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
	0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF
};

CRC_TYPE GetCRCType(const char* szCRCName)
{
	for (int i = 0; i < CRC_MAX; i++)
	{
		if (_stricmp(crc_props[i].Name, szCRCName) == 0)
			return (CRC_TYPE)i;
	}

	return CRC_MAX;
}

uint8_t GetCRCWidth(CRC_TYPE type)
{
	if (type < 0 || type >= CRC_MAX)
		return 0;

	return crc_props[type].width;
}

const char* GetCRCName(CRC_TYPE type)
{
	if (type < 0 || type >= CRC_MAX)
		return 0;

	return crc_props[type].Name;
}

void PrintCRCList()
{
	// At first, find the maximum name length
	size_t max_name_len = 0;
	for (int i = 0; i < CRC_MAX; i++)
	{
		size_t name_len = strlen(crc_props[i].Name);
		if (max_name_len < name_len)
			max_name_len = name_len;
	}

	// Estimate the length of one line
	size_t max_line_len = max_name_len + 4 + 18 + 4 + 18 + 2 + 1;
	char* szLine = new char[max_line_len];
	memset(szLine, 0x20, max_line_len);

	memcpy(szLine, "Name", 4);
	memcpy(szLine + max_name_len + 4, "Polynomial", strlen("Polynomial"));
	memcpy(szLine + max_name_len + 4 + 18 + 4, "Init-value", strlen("Init-value"));

	szLine[max_line_len - 3] = '\r';
	szLine[max_line_len - 2] = '\n';
	szLine[max_line_len - 1] = '\0';

	printf("%s", szLine);

	char szHexNumber[64];
	for (int i = 0; i < CRC_MAX; i++)
	{
		memset(szLine, 0x20, max_line_len);
		memcpy(szLine, crc_props[i].Name, strlen(crc_props[i].Name));

		szLine[max_line_len - 3] = '\r';
		szLine[max_line_len - 2] = '\n';
		szLine[max_line_len - 1] = '\0';

		char szFmtStr[64];
		sprintf_s(szFmtStr, sizeof(szFmtStr), "0X%%0%dllX", (crc_props[i].width + 3)/4);

		sprintf_s(szHexNumber, sizeof(szHexNumber), (const char*)szFmtStr, crc_props[i].Polynomial);

		memcpy(szLine + max_name_len + 4, szHexNumber, strlen(szHexNumber));

		sprintf_s(szHexNumber, sizeof(szHexNumber), (const char*)szFmtStr, crc_props[i].Initvalue);

		memcpy(szLine + max_name_len + 4 + 18 + 4, szHexNumber, strlen(szHexNumber));

		printf("%s", szLine);
	}

	delete[] szLine;
}


/*
 * Derive parameters from the standard-specific parameters in crc.h.
 */
#define WIDTH    (8 * sizeof(crc))
#define TOPBIT   (((crc)1) << (WIDTH - 1))

#if (REVERSED_DATA == TRUE)
    #define FP_reflect_DATA(_DATO)                      ((uint8_t)(FP_reflect((_DATO), 8)&0xFF))
    #define FP_reflect_CRCTableValue(_CRCTableValue)	((crc) FP_reflect((_CRCTableValue), WIDTH))
#else
    #define FP_reflect_DATA(_DATO)                      (_DATO)
    #define FP_reflect_CRCTableValue(_CRCTableValue)	(_CRCTableValue)

#endif

#if (CALCULATE_LOOKUPTABLE == TRUE)
static crc  A_crcLookupTable[256] = {0};
#endif
/*********************************************************************
 *
 * Function:    FP_reflect()
 * 
 * Description: A value/register is reflected if it's bits are swapped around its centre.
 * For example: 0101 is the 4-bit reflection of 1010
 *
 * Description: Un valor es reflejado cuando sus bits son intercambiados utilizando como punto de referencia el centro.
 * Por ejemplo: 0101 es el reflejo de 1010
 *
 *********************************************************************/
 #if (REVERSED_DATA == TRUE)
static crc FP_reflect(crc VF_dato, uint8_t VF_nBits)
{
    crc VP_reflection = 0;
    uint8_t VP_Pos_bit = 0;

    for (VP_Pos_bit = 0; VP_Pos_bit < VF_nBits; VP_Pos_bit++)
    {
        if ((VF_dato & 1) == 1)
        {
            VP_reflection |= (((crc)1) << ((VF_nBits - 1) - VP_Pos_bit));
        }

        VF_dato = (VF_dato >> 1);
    }
    return (VP_reflection);
}

#endif

/*********************************************************************
 *
 * Function:    F_CRC_ObtenValorDeTabla()
 * 
 * Description: Obtains the Table value
 *
 *
 *********************************************************************/
static crc F_CRC_ObtenValorDeTabla(uint8_t VP_Pos_Tabla)
{
    crc VP_CRCTableValue = 0;
    uint8_t VP_Pos_bit = 0;

    VP_CRCTableValue = ((crc) FP_reflect_DATA(VP_Pos_Tabla)) << (WIDTH - 8);

    for (VP_Pos_bit = 0; VP_Pos_bit < 8; VP_Pos_bit++)
    {
        if (VP_CRCTableValue & TOPBIT)
        {
            VP_CRCTableValue = (VP_CRCTableValue << 1) ^ POLYNOMIAL;
        }
        else
        {
            VP_CRCTableValue = (VP_CRCTableValue << 1);
        }
    }
    return (FP_reflect_CRCTableValue(VP_CRCTableValue));
}

#if (CALCULATE_LOOKUPTABLE == TRUE)

/*********************************************************************
 *
 * Function:    F_CRC_InicializaTable()
 * 
 * Description: Create the lookup table for the CRC
 *
 *
 *********************************************************************/
void F_CRC_InicializaTable(void)
{
	static bool bInitialized = false;
	uint16_t VP_Pos_Array = 0;

	if (!bInitialized){
		for (VP_Pos_Array = 0; VP_Pos_Array < 256; VP_Pos_Array++)
		{
			A_crcLookupTable[VP_Pos_Array] = F_CRC_ObtenValorDeTabla((uint8_t)(VP_Pos_Array &0xFF));
		}

		bInitialized = true;
	}

}



/*********************************************************************
 *
 * Function:    F_CRC_CalculaCheckSumDeTabla()
 * 
 * Description: Calculate the CRC value from a Lookup Table.
 *
 * Notes:		F_CRC_InicializaTable() must be called first.
 *              Since AF_Datos is a char array, it is possible to compute any kind of file or array.
 *
 * Returns:		The CRC of the AF_Datos.
 *
 *********************************************************************/
crc F_CRC_CalculaCheckSum(uint8_t const AF_Datos[], size_t VF_nBytes)
{
    crc	VP_CRCTableValue = INITIAL_VALUE;
    size_t VP_bytes = 0;

    for (VP_bytes = 0; VP_bytes < VF_nBytes; VP_bytes++)
    {
        #if (REVERSED_DATA == TRUE)
            VP_CRCTableValue = (VP_CRCTableValue >> 8) ^ A_crcLookupTable[((uint8_t)(VP_CRCTableValue & 0xFF)) ^ AF_Datos[VP_bytes]];
        #else
            VP_CRCTableValue = (VP_CRCTableValue << 8) ^ A_crcLookupTable[((uint8_t)((VP_CRCTableValue >> (WIDTH - 8)) & 0xFF)) ^ AF_Datos[VP_bytes]];
        #endif
    }
 
#if 0
	if ((8 * sizeof(crc)) > WIDTH)
	{
		VP_CRCTableValue = VP_CRCTableValue & ((((crc)(1)) << WIDTH) - 1);
	}
#endif

    #if (REVERSED_OUT == FALSE)
        return (VP_CRCTableValue ^ FINAL_XOR_VALUE);
    #else
        return (~VP_CRCTableValue ^ FINAL_XOR_VALUE);
    #endif

}
#else


/*********************************************************************
 *
 * Function:    F_CRC_CalculaCheckSumSinTabla()
 *
 * Description: Calculate the CRC value withouth a lookup table
 *
 * Notes:
 *
 * Returns:		The CRC of the AF_Datos.
 * Retorna:             El computo de CRC de AF_DAtos
 *
 *********************************************************************/
crc F_CRC_CalculaCheckSum(uint8_t const AF_Datos[], int16_t VF_nBytes)
{
    crc	VP_CRCTableValue = INITIAL_VALUE;
    int16_t VP_bytes = 0;

    for (VP_bytes = 0; VP_bytes < VF_nBytes; VP_bytes++)
    {
        #if (REVERSED_DATA == TRUE)
            VP_CRCTableValue = (VP_CRCTableValue >> 8) ^ F_CRC_ObtenValorDeTabla(((uint8_t)(VP_CRCTableValue & 0xFF)) ^ AF_Datos[VP_bytes]);
        #else
            VP_CRCTableValue = (VP_CRCTableValue << 8) ^ F_CRC_ObtenValorDeTabla(((uint8_t)((VP_CRCTableValue >> (WIDTH - 8)) & 0xFF)) ^ AF_Datos[VP_bytes]);
        #endif
    }

	if ((8 * sizeof(crc)) > WIDTH)
	{
		VP_CRCTableValue = VP_CRCTableValue & ((((crc)(1)) << WIDTH) - 1);
	}

    #if (REVERSED_OUT == FALSE)
        return (VP_CRCTableValue ^ FINAL_XOR_VALUE);
    #else
        return (~VP_CRCTableValue ^ FINAL_XOR_VALUE);
    #endif

}

#endif

inline void _InitCRC(CRC_TYPE type)
{
	if (crc_props[type].initialized == false)
	{
		uint64_t mask_bits = (uint64_t)((1LL << crc_props[type].width) - 1);
		for (size_t i = 0; i < 256; i++)
		{
			crc_props[type].CRCLookupTable[i] = ((uint64_t)i << (crc_props[type].width - 8));

			for (uint8_t VP_Pos_bit = 0; VP_Pos_bit < 8; VP_Pos_bit++)
			{
				if (crc_props[type].CRCLookupTable[i] & (1LL << (crc_props[type].width - 1)))
				{
					crc_props[type].CRCLookupTable[i] = (crc_props[type].CRCLookupTable[i] << 1) ^ crc_props[type].Polynomial;
				}
				else
				{
					crc_props[type].CRCLookupTable[i] = (crc_props[type].CRCLookupTable[i] << 1);
				}
				crc_props[type].CRCLookupTable[i] &= mask_bits;
			}
		}

		crc_props[type].initialized = true;
	}
}

inline uint64_t _ReserveCRCBits(uint64_t u64Ret, uint8_t width)
{
	switch (width)
	{
	case 64:
		return ((uint64_t)BitReverseTable256[u64Ret & 0xff] << 56) |
				((uint64_t)BitReverseTable256[(u64Ret >> 8) & 0xff] << 48) |
				((uint64_t)BitReverseTable256[(u64Ret >> 16) & 0xff] << 40) |
				((uint64_t)BitReverseTable256[(u64Ret >> 24) & 0xff] << 32) |
				((uint64_t)BitReverseTable256[(u64Ret >> 32) & 0xff] << 24) |
				((uint64_t)BitReverseTable256[(u64Ret >> 40) & 0xff] << 16) |
				((uint64_t)BitReverseTable256[(u64Ret >> 48) & 0xff] << 8) |
				((uint64_t)BitReverseTable256[(u64Ret >> 56) & 0xff]);
	case 32:
		return (uint32_t)(BitReverseTable256[u64Ret & 0xff] << 24) |
				(BitReverseTable256[(u64Ret >> 8) & 0xff] << 16) |
				(BitReverseTable256[(u64Ret >> 16) & 0xff] << 8) |
				(BitReverseTable256[(u64Ret >> 24) & 0xff]);
	case 24:
		return (BitReverseTable256[u64Ret & 0xff] << 16) |
				(BitReverseTable256[(u64Ret >> 8) & 0xff] << 8) |
				(BitReverseTable256[(u64Ret >> 16) & 0xff]);
	case 16:
		return (BitReverseTable256[u64Ret & 0xff] << 8) |
				(BitReverseTable256[(u64Ret >> 8) & 0xff]);
	case 8:
		return BitReverseTable256[u64Ret & 0xff];
	}

	uint64_t v = u64Ret;
	uint64_t r = v;
	uint8_t s = width - 1;  
	uint64_t mask_bits = (uint64_t)((1LL << width) - 1);

	for (v >>= 1; v & mask_bits; v >>= 1)
	{
		r <<= 1;
		r |= v & 1;
		s--;
	}
	r <<= s;
	return r&mask_bits;
}

#define CRC_PROCESS(type, buf, end_buf, result, crc_width, crc_lut, reserve)	{\
		uint8_t shift_right_width = crc_width - 8;\
		type VP_CRCTableValue = (type)result;\
		while (buf < end_buf)\
		{\
			if (bNeedReservedBit)\
				VP_CRCTableValue = (VP_CRCTableValue << 8) ^ (type)crc_lut[((VP_CRCTableValue >> shift_right_width)) ^ BitReverseTable256[*buf++]];\
			else\
				VP_CRCTableValue = (VP_CRCTableValue << 8) ^ (type)crc_lut[((VP_CRCTableValue >> shift_right_width)) ^ *buf++];\
		}\
		result = VP_CRCTableValue;\
	}\

#define CRC_CALCULATE(type, buf, end_buf, init_value, crc_width, crc_lut, reserve, xorout)	{\
		uint8_t shift_right_width = crc_width - 8;\
		type VP_CRCTableValue = (type)init_value;\
		while (buf < end_buf)\
		{\
			if (bNeedReservedBit)\
				VP_CRCTableValue = (VP_CRCTableValue << 8) ^ (type)crc_lut[((VP_CRCTableValue >> shift_right_width)) ^ BitReverseTable256[*buf]];\
			else\
				VP_CRCTableValue = (VP_CRCTableValue << 8) ^ (type)crc_lut[((VP_CRCTableValue >> shift_right_width)) ^ *buf];\
			buf++;\
		}\
		type mask_bits = (type)((1LL << crc_width) - 1);\
		type uRet = ((VP_CRCTableValue & mask_bits) ^ (type)xorout);\
		if (reserve)\
			uRet = (type)_ReserveCRCBits(uRet, crc_width);\
		return uRet;\
	}\

uint64_t CalcCRC(CRC_TYPE type, const uint8_t* pBuf, size_t cbSize)
{
	if (type < 0 || type >= CRC_MAX)
		return UINT64_MAX;

	_InitCRC(type);

	const uint8_t* pEndBuf = pBuf + cbSize;
	uint8_t crc_width = crc_props[type].width;
	uint64_t* crc_lut = crc_props[type].CRCLookupTable;
	bool bNeedReservedBit = crc_props[type].Reversed;
	
	if (crc_width == 8)
		CRC_CALCULATE(uint8_t, pBuf, pEndBuf, crc_props[type].Initvalue, crc_width, crc_lut, bNeedReservedBit, crc_props[type].XORout)
	else if (crc_width == 16)
		CRC_CALCULATE(uint16_t, pBuf, pEndBuf, crc_props[type].Initvalue, crc_width, crc_lut, bNeedReservedBit, crc_props[type].XORout)
	else if(crc_width == 32)
		CRC_CALCULATE(uint32_t, pBuf, pEndBuf, crc_props[type].Initvalue, crc_width, crc_lut, bNeedReservedBit, crc_props[type].XORout)
	else if (crc_width == 64)
		CRC_CALCULATE(uint64_t, pBuf, pEndBuf, crc_props[type].Initvalue, crc_width, crc_lut, bNeedReservedBit, crc_props[type].XORout)
	else
	{
		// General algorithm, not optimized
		uint8_t shift_right_width = crc_width - 8;
		uint64_t VP_CRCTableValue = crc_props[type].Initvalue;
		while (pBuf < pEndBuf)
		{
			if (bNeedReservedBit)
				VP_CRCTableValue = (VP_CRCTableValue << 8) ^ crc_lut[(((VP_CRCTableValue >> shift_right_width) & 0xFF)) ^ BitReverseTable256[*pBuf++]];
			else
				VP_CRCTableValue = (VP_CRCTableValue << 8) ^ crc_lut[(((VP_CRCTableValue >> shift_right_width) & 0xFF)) ^ *pBuf++];
		}

		uint64_t mask_bits = (uint64_t)((1LL << crc_props[type].width) - 1);
		uint64_t u64Ret = (VP_CRCTableValue ^ crc_props[type].XORout) & mask_bits;

		if (crc_props[type].Reversed)
			u64Ret = _ReserveCRCBits(u64Ret, crc_props[type].width);

		return u64Ret;
	}
}

struct CRC_HANDLE_INFO
{
	CRC_TYPE	type;
	uint64_t	VP_CRCTableValue;
};

CRC_HANDLE BeginCRC(CRC_TYPE type)
{
	if (type < 0 || type >= CRC_MAX)
		return NULL;

	_InitCRC(type);

	CRC_HANDLE_INFO* handle_info = new CRC_HANDLE_INFO();

	handle_info->type = type;
	handle_info->VP_CRCTableValue = crc_props[type].Initvalue;

	return (CRC_HANDLE)handle_info;
}

void ProcessCRC(CRC_HANDLE handle, const uint8_t* pBuf, size_t cbSize)
{
	CRC_HANDLE_INFO* handle_info = (CRC_HANDLE_INFO*)handle;
	if (handle_info == NULL)
		return;

	const uint8_t* pEndBuf = pBuf + cbSize;
	uint8_t crc_width = crc_props[handle_info->type].width;
	uint64_t* crc_lut = crc_props[handle_info->type].CRCLookupTable;
	bool bNeedReservedBit = crc_props[handle_info->type].Reversed;
	
	if (crc_width == 8)
		CRC_PROCESS(uint8_t, pBuf, pEndBuf, handle_info->VP_CRCTableValue, crc_width, crc_lut, bNeedReservedBit)
	else if (crc_width == 16)
		CRC_PROCESS(uint16_t, pBuf, pEndBuf, handle_info->VP_CRCTableValue, crc_width, crc_lut, bNeedReservedBit)
	else if (crc_width == 32)
		CRC_PROCESS(uint32_t, pBuf, pEndBuf, handle_info->VP_CRCTableValue, crc_width, crc_lut, bNeedReservedBit)
	else if (crc_width == 64)
		CRC_PROCESS(uint64_t, pBuf, pEndBuf, handle_info->VP_CRCTableValue, crc_width, crc_lut, bNeedReservedBit)
	else
	{
		for (uint16_t VP_bytes = 0; VP_bytes < cbSize; VP_bytes++)
		{
			uint8_t u8Byte = (bNeedReservedBit ? BitReverseTable256[pBuf[VP_bytes]] : pBuf[VP_bytes]);
			uint64_t lookup_value = crc_props[handle_info->type].CRCLookupTable[((uint8_t)((handle_info->VP_CRCTableValue >> (crc_props[handle_info->type].width - 8)) & 0xFF)) ^ u8Byte];
			handle_info->VP_CRCTableValue = (handle_info->VP_CRCTableValue << 8) ^ lookup_value;
		}
	}

	return;
}

uint64_t EndCRC(CRC_HANDLE handle)
{
	CRC_HANDLE_INFO* handle_info = (CRC_HANDLE_INFO*)handle;
	if (handle_info == NULL)
		return UINT64_MAX;

	uint64_t mask_bits = (uint64_t)((1LL << crc_props[handle_info->type].width) - 1);
	uint64_t u64Ret = (handle_info->VP_CRCTableValue ^ crc_props[handle_info->type].XORout)&mask_bits;

	if (crc_props[handle_info->type].Reversed)
	{
		u64Ret = _ReserveCRCBits(u64Ret, crc_props[handle_info->type].width);
	}

	delete handle_info;
	return u64Ret;
}
