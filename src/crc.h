/**********************************************************************
 *
 * Filename:    crc.h
 * 
 * Description: Header de calculo CRC
 *
 * Notes:       
 *
 * 
 *
 * Copyright (c) 2014 Francisco Javier Lana Romero
 *
 * Code that I have seen and studied to be able to solve this problem:
 * Codigo que he estudiado para poder solucionar este problema:
 * http://www.barrgroup.com/code/crc.zip
 * http://www.networkdls.com/Downloads/CRC32/1003/CCRC32.zip
 * http://www.opensource.apple.com/source/xnu/xnu-1456.1.26/bsd/libkern/crc32.c
 * http://opensource.apple.com/source/gdb/gdb-213/src/gdb/rdi-share/crc.c
 * http://www.cs.fsu.edu/~baker/devices/lxr/http/source/linux/lib/crc-ccitt.c#L63
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
 *
 *
 */


#ifndef _crc_h
#define _crc_h


//El valor CHECK es el resultado de realizar el calculo CRC para el char[] = "123456789";
//The CHECK value is the result of making the checksum of the char[] = "123456789";
/*
 * Valores sacados de la pagina http://crcmod.sourceforge.net/crcmod.predefined.html Es posible que alguno este mal.
 * This values have been taken from http://crcmod.sourceforge.net/crcmod.predefined.html some of them could be wrong
 Name                   Polynomial              Reversed?   Init-value	        XOR-out	            Check
crc-8                   0x07                    False       0x00				0x00	            0xF4
crc-8-darc              0x39	                True        0x00				0x00	            0x15
crc-8-i-code            0x1D	                False       0xFD				0x00	            0x7E
crc-8-itu               0x07	                False       0x55				0x55	            0xA1
crc-8-maxim             0x31	                True        0x00				0x00	            0xA1
crc-8-rohc              0x07	                True        0xFF				0x00	            0xD0
crc-8-wcdma             0x9B	                True        0x00				0x00	            0x25
crc-16                  0x8005	                True        0x0000				0x0000	            0xBB3D
crc-16-buypass          0x8005	                False       0x0000				0x0000	            0xFEE8
crc-16-dds-110          0x8005	                False       0x800D				0x0000	            0x9ECF
crc-16-dect             0x0589	                False       0x0001				0x0001	            0x007E
crc-16-dnp              0x3D65	                True        0xFFFF				0xFFFF	            0xEA82
crc-16-en-13757         0x3D65	                False       0xFFFF				0xFFFF	            0xC2B7
crc-16-genibus          0x1021	                False       0x0000				0xFFFF	            0xD64E
crc-16-maxim            0x8005	                True        0xFFFF				0xFFFF	            0x44C2
crc-16-mcrf4xx          0x1021	                True        0xFFFF				0x0000	            0x6F91
crc-16-riello           0x1021	                True        0x554D				0x0000	            0x63D0
crc-16-t10-dif          0x8BB7	                False       0x0000				0x0000	            0xD0DB
crc-16-teledisk         0xA097	                False       0x0000				0x0000	            0x0FB3
crc-16-usb              0x8005	                True        0x0000				0xFFFF	            0xB4C8
x-25                    0x1021	                True        0x0000				0xFFFF	            0x906E
xmodem                  0x1021	                False       0x0000				0x0000	            0x31C3
modbus                  0x8005	                True        0xFFFF				0x0000	            0x4B37
kermit [1]              0x1021	                True        0x0000				0x0000	            0x2189
crc-ccitt-false [1]		0x1021	                False       0xFFFF				0x0000	            0x29B1
crc-aug-ccitt [1]		0x1021                  False       0x1D0F				0x0000              0xE5CC
crc-24                  0x864CFB                False       0xB704CE	        0x000000	        0x21CF02
crc-24-flexray-a		0x5D6DCB                False       0xFEDCBA	        0x000000	        0x7979BD
crc-24-flexray-b		0x5D6DCB                False       0xABCDEF	        0x000000	        0x1F23B8
crc-32                  0x04C11DB7              True        0x00000000?	        0xFFFFFFFF	        0xCBF43926
crc-32-bzip2            0x04C11DB7              False       0xFFFFFFFF	        0x00000000	        0xFC891918
crc-32c                 0x1EDC6F41              True        0x00000000	        0xFFFFFFFF	        0xE3069283
crc-32d                 0xA833982B              True        0x00000000	        0xFFFFFFFF	        0x87315576
crc-32-mpeg             0x04C11DB7              False       0xFFFFFFFF	        0x00000000	        0x0376E6E7
posix                   0x04C11DB7              False       0xFFFFFFFF	        0xFFFFFFFF	        0x765E7680
crc-32q                 0x814141AB              False       0x00000000	        0x00000000	        0x3010BF7F
jamcrc                  0x04C11DB7              True        0xFFFFFFFF          0x00000000	        0x340BC6D9
xfer                    0x000000AF              False       0x00000000          0x00000000	        0xBD0BE338
crc-64                  0x000000000000001B		True        0x0000000000000000	0x0000000000000000	0x46A5A9388A5BEFFE
crc-64-we               0x42F0E1EBA9EA3693		False       0x0000000000000000	0xFFFFFFFFFFFFFFFF	0x62EC59E3F1A4F00A
crc-64-jones            0xAD93D23594C935A9		True        0xFFFFFFFFFFFFFFFF	0x0000000000000000	0xCAA717168609F281
*/

#include <stdint.h>

#define TRUE    1
#define FALSE   0

enum CRC_TYPE
{
	CRC8,
	CRC8_DARC,
	CRC8_I_CODE,
	CRC8_ITU,
	CRC8_MAXIM,
	CRC8_ROHC,
	CRC8_WCDMA,
	CRC16,
	CRC16_BUYPASS,
	CRC16_DDS_110,
	CRC16_DECT,
	CRC16_DNP,
	CRC16_EN_13757,
	CRC16_GENIBUS,
	CRC16_MAXIM,
	CRC16_MCRF4XX,
	CRC16_RIELLO,
	CRC16_T10_DIF,
	CRC16_TELEDISK,
	CRC16_USB,
	X_25,
	XMODEM,
	MODBUS,
	KERMIT,
	CRC_CCITT_FALSE,
	CRC_AUG_CCITT,
	CRC24,
	CRC24_FLEXRAY_A,
	CRC24_FLEXRAY_B,
	CRC32,
	CRC32_BZIP2,
	CRC32C,
	CRC32D,
	CRC32_MPEG,
	POSIX,
	CRC32Q,
	JAMCRC,
	XFER,
	CRC64,
	CRC64_WE,
	CRC64_JONES,
	CRC_MAX
};

typedef void*	CRC_HANDLE;

//Indicate here the CRC algorithm that you want to use
#define _CRC_32_MPEG

//Indicate here if you want to do the calculation using a LookupTable
#define CALCULATE_LOOKUPTABLE   TRUE


#if defined(_CRC_8)

typedef uint8_t crc;

#define POLYNOMIAL				0x07
#define INITIAL_VALUE           0
#define FINAL_XOR_VALUE			0
#define REVERSED_DATA           FALSE
#define REVERSED_OUT            FALSE

#elif defined(_CRC_CCITT)

typedef uint16_t crc;

#define POLYNOMIAL				0x1021
#define INITIAL_VALUE           0xFFFF
#define FINAL_XOR_VALUE			0
#define REVERSED_DATA           FALSE
#define REVERSED_OUT            FALSE

#elif defined(MODBUS)

typedef uint16_t crc;

#define POLYNOMIAL				0x8005
#define INITIAL_VALUE           0xFFFF
#define FINAL_XOR_VALUE			0
#define REVERSED_DATA           TRUE
#define REVERSED_OUT            FALSE



#elif defined(_CRC_16)

typedef uint16_t crc;

#define POLYNOMIAL				0x8005
#define INITIAL_VALUE           0x0000
#define FINAL_XOR_VALUE			0x0000
#define REVERSED_DATA           TRUE
#define REVERSED_OUT            FALSE

#elif defined(_CRC_24)
//Since is impossible to define a 24bit variable and i need to use and structure instead, what i have done is just put the WIDTH directly. In this case the valid part from the result,
//will be the less significat 24bits (&0xFFF) the rest must be discarted.

//Como es imposible definir variables de 24bits y necesito crear una estructura, lo que he hecho es definir la anchura directamente para poder conseguir un resultado valido.
//Para este caso la parte con la que debemos quedarnos son los 24 bits de menos peso (&0xFFF), la parte alta debemos de descartarla
typedef uint32_t crc;

#define POLYNOMIAL				0x864CFB
#define INITIAL_VALUE           0xB704CE
#define FINAL_XOR_VALUE			0x000000
#define REVERSED_DATA           FALSE
#define REVERSED_OUT            FALSE
#define WIDTH                   (24)

#elif defined(_CRC_32)

typedef uint32_t crc;

#define POLYNOMIAL				0x04C11DB7
#define INITIAL_VALUE           0xFFFFFFFF
#define FINAL_XOR_VALUE			0xFFFFFFFF
#define REVERSED_DATA           TRUE
#define REVERSED_OUT            FALSE


#elif defined(_CRC_32_BZIP2)

typedef uint32_t crc;

#define POLYNOMIAL				0x04C11DB7
#define INITIAL_VALUE           0xFFFFFFFF
#define FINAL_XOR_VALUE			0
#define REVERSED_DATA           FALSE
#define REVERSED_OUT            TRUE


#elif defined(_CRC_64_JONES)

typedef uint64_t crc;

#define POLYNOMIAL				0xAD93D23594C935A9
#define INITIAL_VALUE           0xFFFFFFFFFFFFFFFF
#define FINAL_XOR_VALUE			0
#define REVERSED_DATA           TRUE
#define REVERSED_OUT            FALSE

#elif defined(_CRC_32_MPEG)

typedef uint32_t crc;

#define POLYNOMIAL				0x04C11DB7
#define INITIAL_VALUE           0xFFFFFFFF
#define FINAL_XOR_VALUE			0x00000000
#define REVERSED_DATA           FALSE
#define REVERSED_OUT            FALSE

#else

#error "Debes definir un algoritmo CRC"
#error "You should define at least one algorithm"

#endif

#if (CALCULATE_LOOKUPTABLE == FALSE)
    #define F_CRC_InicializaTable()
#else
    void F_CRC_InicializaTable(void);
#endif
crc			F_CRC_CalculaCheckSum(uint8_t const AF_Datos[], size_t VF_nBytes);

uint64_t	CalcCRC(CRC_TYPE type, uint8_t* pBuf, size_t cbSize);

CRC_HANDLE	BeginCRC(CRC_TYPE type);
void		ProcessCRC(CRC_HANDLE handle, const uint8_t* pBuf, size_t cbSize);
uint64_t	EndCRC(CRC_HANDLE handle);


#endif 