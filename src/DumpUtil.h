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
#ifndef _DUMPUTIL_15A547E8_2D7E_494A_96D8_068E511BA6B0_H_
#define _DUMPUTIL_15A547E8_2D7E_494A_96D8_068E511BA6B0_H_

#include <stdio.h>
#include <tchar.h>
#include <assert.h>
#include <stdint.h>
#include <inttypes.h>
#include "AMBitStream.h"
#include "combase.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4768)
#pragma warning(disable:4200)
#pragma warning(disable:4996)
#pragma warning(disable:4101)
#pragma warning(disable:4201)
#pragma pack(push,1)
#define PACKED
#else
#define PACKED __attribute__ ((__packed__))
#endif

#if (defined(__linux__))&&(!defined(__int64))
#define __int64 long long
#endif

#define DECLARE_ENDIAN_BEGIN()		void Endian(bool bBig2SysByteOrder=true){UNREFERENCED_PARAMETER(bBig2SysByteOrder);
#define DECLARE_ENDIAN_END()		return;}

#define USHORT_VALUE(big_endian, p)	(big_endian?\
	(unsigned short)((*(p)<<8) | *((p)+1)):\
	(unsigned short)((*((p)+1)<<8) | *(p)))

#define ULONG_VALUE(big_endian, p)	(big_endian?\
	(unsigned long)((*(p)<<24) | (*((p)+1)<<16) | (*((p)+2)<<8) | *((p)+3)):\
	(unsigned long)((*((p)+3)<<24) | (*((p)+2)<<16) | (*((p)+1)<<8) | *(p)))

#define LONG_VALUE(big_endian, p)	(big_endian?\
	(long)((*(p)<<24) | (*((p)+1)<<16) | (*((p)+2)<<8) | *((p)+3)):\
	(long)((*((p)+3)<<24) | (*((p)+2)<<16) | (*((p)+1)<<8) | *(p)))

//
// Define macros that can "endian" the specified variables.
//
#define FIELD_ENDIAN(field)\
	BST::DIRECT_ENDIAN_MAP::rbswap(field);\

#define FIELD_ENDIAN2(field1,field2)\
	BST::DIRECT_ENDIAN_MAP::rbswap(field1);\
	BST::DIRECT_ENDIAN_MAP::rbswap(field2);\
		
#define FIELD_ENDIAN3(field1,field2,field3)\
	BST::DIRECT_ENDIAN_MAP::rbswap(field1);\
	BST::DIRECT_ENDIAN_MAP::rbswap(field2);\
	BST::DIRECT_ENDIAN_MAP::rbswap(field3);\

#define FIELD_ENDIAN4(field1,field2,field3,field4)\
	BST::DIRECT_ENDIAN_MAP::rbswap(field1);\
	BST::DIRECT_ENDIAN_MAP::rbswap(field2);\
	BST::DIRECT_ENDIAN_MAP::rbswap(field3);\
	BST::DIRECT_ENDIAN_MAP::rbswap(field4);\

#define FIELD_ENDIAN5(field1,field2,field3,field4,field5)\
	BST::DIRECT_ENDIAN_MAP::rbswap(field1);\
	BST::DIRECT_ENDIAN_MAP::rbswap(field2);\
	BST::DIRECT_ENDIAN_MAP::rbswap(field3);\
	BST::DIRECT_ENDIAN_MAP::rbswap(field4);\
	BST::DIRECT_ENDIAN_MAP::rbswap(field5);\

#define PRINT_AC(c)	(isprint(c)?(c):'.')
#define PRINT_WC(c)	(iswprint(c)?(c):L'.')
#ifdef _UNICODE
#define PRINT_TC(c)	PRINT_WC(c)
#else
#define PRINT_TC(c)	PRINT_AC(c)
#endif

// Check whether the buffer that will be converted to STRUCT is enough big.
#define DECLARE_IS_BUFFER_READY(STRUCT)\
	bool IsBufferReady(unsigned char* pBuf, unsigned long cbSize){return cbSize >= sizeof(STRUCT);}

// Check whether the buffer that will be converted to VAR-STRUCT is enough big.
// HDR_SIZE		specifies the size including the part from header to the length field 
//				that specifies the size of the part starting immediately following it.
// TYPE			specifies the type of the length field.
// OFFSET		specifies the offset address from the beginning of pBuf parameter.
// SHIFT		specifies how many bits length info contained in the length field shift 
//				to the right to get the correct value.
// MASK			specifies the mask which masks the length field to get the correct length info.
#define DECLARE_IS_BUFFER_READY2(HDR_SIZE,TYPE,OFFSET,SHIFT,MASK)\
	bool IsBufferReady(unsigned char* pBuf, unsigned long cbSize){\
		return cbSize >= HDR_SIZE + ((DIRECT_ENDIAN_MAP::bswap(*((TYPE*)(pBuf + OFFSET)))&MASK) >> SHIFT);}

//
// The macro is fit for the struct with var-length member variable whose type is a struct 
// inherited from DIRECT_ENDIAN_MAP.
//
#define DECLARE_IS_BUFFER_READY3(HDR_SIZE,STRUCT_TYPE,VAR_BODY_TYPE,TYPE,OFFSET,SHIFT,MASK)\
	bool IsBufferReady(unsigned char* pBuf, unsigned long cbSize){\
		int body_length_field = (int)((DIRECT_ENDIAN_MAP::bswap(*((TYPE*)(pBuf + OFFSET)))&MASK) >> SHIFT);\
		int var_body_size = HDR_SIZE + body_length_field - sizeof(*this);\
		return cbSize >= HDR_SIZE + body_length_field && var_body_size % sizeof(VAR_BODY_STRUCT) != 0;}

//
// In Method Is_Buffer_Ready, the length field have some relative parameter, but in
// the below methods, only BODY_LENGTH_FIELD parameter is provided. the situation is
// caused by that Is_Buffer_Ready method must be called before Endian method, so the
// member's state is big-endian. However IsValid method is called after Endian, the
// member variable have the correct little-endian value.
//
#define DECLARE_EMPTY_ISVALID()\
	bool IsValid(){return true;}

//
// define Macro that Gets the size of var-length part in struct by byte number.
//
#define DECLARE_GETVARBODYSIZE(HDR_SIZE, BODY_LENGTH_FIELD, VAR_BODY_STRUCT)\
	int GetVarBodySize()\
	{\
		int var_body_size = HDR_SIZE + BODY_LENGTH_FIELD - sizeof(*this);\
		if (var_body_size % sizeof(VAR_BODY_STRUCT) != 0)\
			return -1;\
		return var_body_size;\
	}

#define DECLARE_GETVARBODYSIZE2(VAR_BODY_UNIT_NS, VAR_BODY_FIELD)\
	int GetVarBodySize()\
	{\
		int i=0, nVarBodyLen = 0;\
		for(;i<VAR_BODY_UNIT_NS;i++)\
			nVarBodyLen += VAR_BODY_FIELD[i].GetVarBodySize();\
		return nVarBodyLen;\
	}

//
// Var-body part may be a array of struct inherited from DIRECT_ENDIAN_MAP,
// so they must be "endianed" when its parent struct calls Endian method.
//
#define VAR_BODY_ENDIAN(HDR_SIZE, BODY_LENGTH_FIELD, VAR_BODY_STRUCT, VAR_BODY_FIELD)\
	{\
		int var_body_size = HDR_SIZE + BODY_LENGTH_FIELD - sizeof(*this);\
		var_body_size = var_body_size / sizeof(VAR_BODY_STRUCT);\
		int var_body_item_idx;\
		for(var_body_item_idx=0;var_body_item_idx<var_body_size;var_body_item_idx++)\
			VAR_BODY_FIELD[var_body_item_idx].Endian();\
	}

#define VAR_BODY_ENDIAN2(VAR_BODY_LENGTH_FIELD, VAR_BODY_STRUCT, VAR_BODY_FIELD)\
	{\
		int var_body_size = VAR_BODY_LENGTH_FIELD;\
		int var_body_item_idx;\
		for(var_body_item_idx=0;var_body_item_idx<var_body_size;var_body_item_idx++)\
			VAR_BODY_FIELD[var_body_item_idx].Endian();\
	}

#define VAR_BODY_MAP(VAR_BODY_NS, VAR_BODY_STRUCT, VAR_BODY_FIELD)\
	{\
		VAR_BODY_FIELD = NULL;\
		if(VAR_BODY_NS > 0){\
			VAR_BODY_FIELD = (VAR_BODY_STRUCT*)(pBuf + ulMappedSize);\
			int var_body_item_idx;\
			for(var_body_item_idx=0;var_body_item_idx<VAR_BODY_NS;var_body_item_idx++)\
			{\
				VAR_BODY_FIELD[var_body_item_idx].Endian();\
				ulMappedSize += sizeof(VAR_BODY_STRUCT) + VAR_BODY_FIELD[var_body_item_idx].GetVarBodySize();\
			}\
		}\
	}

#define VAR_BODY_UNMAP(VAR_BODY_NS, VAR_BODY_FIELD)\
	{\
		int var_body_item_idx;\
		for(var_body_item_idx=0;var_body_item_idx<VAR_BODY_NS;var_body_item_idx++)\
			VAR_BODY_FIELD[var_body_item_idx].Endian();\
		VAR_BODY_FIELD = NULL;\
	}

#define MAP_MEM_TO_HDR(pointer, type)\
	if(cbSize < sizeof(type))\
		return RET_CODE_HEADER_LOST;\
	pointer = (type*)pBuf;\
	ulMappedSize += sizeof(type);\
	if(!pointer->IsValid()){\
		pointer = NULL;\
		return RET_CODE_BUFFER_NOT_COMPATIBLE;}\
	pointer->Endian();

// try to decrease using it, it will wasted too memory.
#define MAP_MEM_TO_HDR2(HDR_start_addr, HDR_size)\
	if(cbSize < HDR_size)\
		return RET_CODE_HEADER_LOST;\
	memcpy(HDR_start_addr, pBuf, HDR_size);\
	ulMappedSize += HDR_size;\
	this->Endian();

#define MAP_MEM_TO_STRUCT_POINTER(flag, pointer, type)\
	if(flag)\
	{\
		if(cbSize - ulMappedSize < sizeof(type))\
			return RET_CODE_BUFFER_TOO_SMALL;\
		pointer = (type *)(pBuf + ulMappedSize);\
		ulMappedSize += sizeof(type) + pointer->GetVarBodySize();\
		if(!pointer->IsValid()){\
			pointer = NULL;\
			return RET_CODE_BUFFER_NOT_COMPATIBLE;}\
		pointer->Endian();\
	}

#define MAP_MEM_TO_STRUCT_POINTER_EX(flag, pointer, type, size)\
	if(flag)\
	{\
		if(cbSize - ulMappedSize < size)\
			return RET_CODE_BUFFER_TOO_SMALL;\
		pointer = (type *)(pBuf + ulMappedSize);\
		unsigned int required_size = size + pointer->GetVarBodySize();\
		if(cbSize - ulMappedSize < required_size)\
			return RET_CODE_BUFFER_TOO_SMALL;\
		ulMappedSize += size + pointer->GetVarBodySize();\
		if(!pointer->IsValid()){\
			pointer = NULL;\
			return RET_CODE_BUFFER_NOT_COMPATIBLE;}\
		pointer->Endian();\
	}

#define	MAP_MEM_TO_STRUCT_POINTER2(flag, pointer, type, ...)\
	if(flag)\
	{\
		unsigned long ulDesiredSizeOut;\
		pointer = new type(__VA_ARGS__);\
		/*AMP_RegisterMem(GetCurrentModule(), pointer, sizeof(type));*/\
		int iMSMRet = pointer->Map(pBuf + ulMappedSize,cbSize - ulMappedSize,&ulDesiredSizeOut);\
		if(iMSMRet != RET_CODE_SUCCESS)\
			return iMSMRet;\
		ulMappedSize += ulDesiredSizeOut;\
	}

#define MAP_MEM_TO_STRUCT_POINTER3(flag1, flag2, pointer, type)\
	if(flag1)\
	{\
		if(cbSize - ulMappedSize < sizeof(type))\
			return RET_CODE_BUFFER_TOO_SMALL;\
		pointer = (type *)(pBuf + ulMappedSize);\
		ulMappedSize += sizeof(type) + (flag2?pointer->GetVarBodySize():0);\
		if(!pointer->IsValid()){\
			pointer = NULL;\
			return RET_CODE_BUFFER_NOT_COMPATIBLE;}\
		pointer->Endian();\
	}

#define MAP_MEM_TO_STRUCT_POINTER4(flag, inbuf, inbufsize, pointer, type)\
	if(flag)\
	{\
		if(inbufsize < sizeof(type))\
			return RET_CODE_BUFFER_TOO_SMALL;\
		pointer = (type *)(inbuf);\
		if(!pointer->IsValid()){\
			pointer = NULL;\
			return RET_CODE_BUFFER_NOT_COMPATIBLE;}\
		pointer->Endian();\
	}

#define	MAP_MEM_TO_STRUCT_POINTER5(flag, pointer, type, ...)\
	if(flag)\
	{\
		pointer = new type(__VA_ARGS__);\
		/*AMP_RegisterMem(GetCurrentModule(), pointer, sizeof(type));*/\
		int iMSMRet = pointer->Map(bst);\
		if(iMSMRet != RET_CODE_SUCCESS)\
			return iMSMRet;\
	}

#define	MAP_MEM_TO_STRUCT_POINTER5_1(bst, flag, pointer, type, ...)\
	if(flag)\
	{\
		pointer = new type(__VA_ARGS__);\
		/*AMP_RegisterMem(GetCurrentModule(), pointer, sizeof(type));*/\
		int iMSMRet = pointer->Map(bst);\
		if(iMSMRet != RET_CODE_SUCCESS)\
			return iMSMRet;\
	}

#define	MAP_MEM_TO_STRUCT_POINTER6(flag, buf, size, pointer, type, ...)\
	if(flag)\
	{\
		pointer = new type(__VA_ARGS__);\
		/*AMP_RegisterMem(GetCurrentModule(), pointer, sizeof(type));*/\
		int iMSMRet = pointer->Map(buf, size);\
		if(iMSMRet != RET_CODE_SUCCESS)\
			return iMSMRet;\
	}

#define	MAP_MEM_TO_STRUCT_POINTER7(flag, buf, size, lend, left, pointer, type, ...)\
	if(flag)\
	{\
		pointer = new type(__VA_ARGS__);\
		/*AMP_RegisterMem(GetCurrentModule(), pointer, sizeof(type));*/\
		int iMSMRet = pointer->Process(buf, size, &lend, &left);\
		if(iMSMRet != RET_CODE_SUCCESS)\
			return iMSMRet;\
	}

#define MAP_MEM_TO_STUFFING(length, pointer)\
	if(cbSize - ulMappedSize < length)\
		return RET_CODE_BUFFER_TOO_SMALL;\
	pointer = (unsigned char*)(pBuf + ulMappedSize);\
	ulMappedSize += length;

#define UNMAP_STRUCT_POINTER(pointer)\
	if(pointer){\
		pointer->Endian(false);\
		pointer = NULL;}

#define UNMAP_STRUCT_POINTER0_1(pointer, type)\
	if(pointer){\
		if (pcbSize != NULL && pBuf == NULL)\
			*pcbSize = sizeof(type) + pointer->GetVarBodySize();\
		else if(pcbSize == NULL){\
			pointer->Endian(false);\
			pointer = NULL;}\
	}\

#define UNMAP_STRUCT_POINTER2(pointer)\
	if(pointer){\
		pointer->Unmap();\
		/*AMP_UnregisterMem(GetCurrentModule(), pointer);*/\
		delete pointer;\
		pointer = NULL;}

#define UNMAP_STRUCT_POINTER2_1(pointer)\
	if(pointer){\
		pointer->Unmap(pBuf, pcbSize);\
		if (pcbSize == NULL){\
			/*AMP_UnregisterMem(GetCurrentModule(), pointer);*/\
			delete pointer;\
			pointer = NULL;}\
	}

#define UNMAP_STRUCT_POINTER3(pointer, buf, len)\
	if(pointer){\
		pointer->Unmap(buf, &len);\
		/*AMP_UnregisterMem(GetCurrentModule(), pointer);*/\
		delete pointer;\
		pointer = NULL;}

#define UNMAP_STRUCT_POINTER5(pointer)\
	if(pointer){\
		pointer->Unmap(NULL);\
		/*AMP_UnregisterMem(GetCurrentModule(), pointer);*/\
		delete pointer;\
		pointer = NULL;}

#define UNMAP_STUFFING(pointer)	if(pointer){pointer = NULL;}

#define CHECK_VALID_ITEM(pointer) if(pointer){bRet = bRet && pointer->IsValid();}

#define TEMP_SIZE		(4096 + 128)
#define TEMP2_SIZE		2048
#define TEMP3_SIZE		32
#define TEMP4_SIZE		255
#define TAGNAME_SIZE	255

#define DECLARE_FIELDPROP_BEGIN()\
	size_t ProduceDesc(_Out_writes_(cbLen) char* szOutXml, size_t cbLen, bool bPrint=false, long long* bit_offset = NULL)\
	{\
		size_t cbRequired = 0;\
		int i=0, field_prop_idx = 0;\
		char szTemp[TEMP_SIZE], szTemp2[TEMP2_SIZE], szTemp3[TEMP3_SIZE], szTemp4[TEMP4_SIZE], szTagName[TAGNAME_SIZE];\
		char* pTemp = NULL;\
		DBG_UNREFERENCED_LOCAL_VARIABLE(szTemp);\
		DBG_UNREFERENCED_LOCAL_VARIABLE(i);\
		DBG_UNREFERENCED_LOCAL_VARIABLE(field_prop_idx);\
		memset(szTemp2, 0, sizeof(szTemp2));\
		memset(szTemp3, 0, sizeof(szTemp3));\
		memset(szTemp4, 0, sizeof(szTemp4));\
		memset(szTagName, 0, sizeof(szTagName));\
		if(szOutXml != 0 && cbLen > 0)\
		{\
			memset(szOutXml, 0, cbLen);\
		}\

#define DECLARE_FIELDPROP_BEGIN1(classname)\
	size_t classname::ProduceDesc(_Out_writes_(cbLen) char* szOutXml, size_t cbLen, bool bPrint, long long* bit_offset)\
	{\
		size_t cbRequired = 0;\
		int i=0, field_prop_idx = 0;\
		char szTemp[TEMP_SIZE], szTemp2[TEMP2_SIZE], szTemp3[TEMP3_SIZE], szTemp4[TEMP4_SIZE], szTagName[TAGNAME_SIZE];\
		char* pTemp = NULL;\
		DBG_UNREFERENCED_LOCAL_VARIABLE(szTemp);\
		DBG_UNREFERENCED_LOCAL_VARIABLE(i);\
		DBG_UNREFERENCED_LOCAL_VARIABLE(field_prop_idx);\
		memset(szTemp2, 0, sizeof(szTemp2));\
		memset(szTemp3, 0, sizeof(szTemp3));\
		memset(szTemp4, 0, sizeof(szTemp4));\
		memset(szTagName, 0, sizeof(szTagName));\
		if(szOutXml != 0 && cbLen > 0)\
		{\
			memset(szOutXml, 0, cbLen);\
		}\

#define DECLARE_FIELDPROP()\
	size_t ProduceDesc(_Out_writes_(cbLen) char* szOutXml, size_t cbLen, bool bPrint=false, long long* bit_offset = NULL)

#define BASECLASS_IMPLEMENT(baseclassname)\
		cbRequired += baseclassname::ProduceDesc(szOutXml, cbLen, bPrint, bit_offset)

#define NAV_WRITE_TAG_BEGIN(Group_Name)\
		if(szOutXml != 0 && cbLen > 0)\
			cbRequired += MBCSPRINTF_S(szOutXml + cbRequired, cbLen - cbRequired, "<%s Offset=\"%lld\">", Group_Name, bit_offset?*bit_offset:-1LL);\
		else\
			cbRequired += MBCSPRINTF_S(szTagName, TAGNAME_SIZE, "<%s Offset=\"%lld\">", Group_Name, bit_offset?*bit_offset:-1LL);\
		if(bPrint)\
			printf("<%s Offset=\"%lld\">", Group_Name, bit_offset?*bit_offset:-1LL);\
		AMP_NOP1(((void)0))

#define NAV_WRITE_TAG_BEGIN_1(Group_Name, TAG_Desc, auto_expand)\
		if(szOutXml != 0 && cbLen > 0)\
			cbRequired += MBCSPRINTF_S(szOutXml + cbRequired, cbLen - cbRequired, "<%s Offset=\"%lld\" Desc=\"%s\"%s>", Group_Name, bit_offset?*bit_offset:-1LL, TAG_Desc, auto_expand?" auto_expand=\"1\"":"");\
		else\
			cbRequired += MBCSPRINTF_S(szTemp2, TEMP2_SIZE, "<%s Offset=\"%lld\" Desc=\"%s\"%s>", Group_Name, bit_offset?*bit_offset:-1LL, TAG_Desc, auto_expand?" auto_expand=\"1\"":"");\
		if(bPrint)\
			printf("<%s Offset=\"%lld\" Desc=\"%s\"%s>", Group_Name, bit_offset?*bit_offset:-1LL, TAG_Desc, auto_expand?" auto_expand=\"1\"":"");\
		AMP_NOP1(((void)0))

#define NAV_WRITE_TAG_BEGIN_WITH_ALIAS(Group_Name, Group_Alias, TAG_Desc)	\
		if(szOutXml != 0 && cbLen > 0)\
			cbRequired += MBCSPRINTF_S(szOutXml + cbRequired, cbLen - cbRequired, "<%s Alias=\"%s\" Offset=\"%lld\" Desc=\"%s\">", Group_Name, Group_Alias, bit_offset?*bit_offset:-1LL, TAG_Desc);\
		else\
			cbRequired += MBCSPRINTF_S(szTemp, TEMP_SIZE, "<%s Alias=\"%s\" Offset=\"%lld\" Desc=\"%s\">", Group_Name, Group_Alias, bit_offset?*bit_offset:-1LL, TAG_Desc);\
		if(bPrint)\
			printf("<%s Alias=\"%s\" Offset=\"%lld\" Desc=\"%s\">", Group_Name, Group_Alias, bit_offset?*bit_offset:-1LL, TAG_Desc);\
		AMP_NOP1(((void)0))

#define NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F(Group_Name, Group_Alias, TAG_Desc, ...)	\
		MBCSPRINTF_S(szTagName, TAGNAME_SIZE, Group_Alias, ##__VA_ARGS__);\
		if(szOutXml != 0 && cbLen > 0)\
			cbRequired += MBCSPRINTF_S(szOutXml + cbRequired, cbLen - cbRequired, "<%s Alias=\"%s\" Desc=\"%s\">", Group_Name, szTagName, TAG_Desc);\
		else\
			cbRequired += MBCSPRINTF_S(szTemp, TEMP_SIZE, "<%s Alias=\"%s\" Desc=\"%s\">", Group_Name, szTagName, TAG_Desc);\
		if(bPrint)\
			printf("<%s Alias=\"%s\" Desc=\"%s\">", Group_Name, szTagName, TAG_Desc);\
		AMP_NOP1(((void)0))

#define NAV_WRITE_AUTOEXPAND_TAG_BEGIN_WITH_ALIAS_F(Group_Name, Group_Alias, TAG_Desc, ...)	\
		MBCSPRINTF_S(szTagName, TAGNAME_SIZE, Group_Alias, ##__VA_ARGS__);\
		if(szOutXml != 0 && cbLen > 0)\
			cbRequired += MBCSPRINTF_S(szOutXml + cbRequired, cbLen - cbRequired, "<%s Alias=\"%s\" Desc=\"%s\" auto_expand=\"1\">", Group_Name, szTagName, TAG_Desc);\
		else\
			cbRequired += MBCSPRINTF_S(szTemp, TEMP_SIZE, "<%s Alias=\"%s\" Desc=\"%s\" auto_expand=\"1\">", Group_Name, szTagName, TAG_Desc);\
		if(bPrint)\
			printf("<%s Alias=\"%s\" Desc=\"%s\" auto_expand=\"1\">", Group_Name, szTagName, TAG_Desc);\
		AMP_NOP1(((void)0))

#define NAV_WRITE_TAG_BEGIN_WITH_ALIAS_DESC_F(Group_Name, Group_Alias, TAG_Desc, ...)	\
		MBCSPRINTF_S(szTemp2, TEMP2_SIZE, TAG_Desc, ##__VA_ARGS__);\
		if(szOutXml != 0 && cbLen > 0)\
			cbRequired += MBCSPRINTF_S(szOutXml + cbRequired, cbLen - cbRequired, "<%s Alias=\"%s\" Desc=\"%s\">", Group_Name, Group_Alias, szTemp2);\
		else\
			cbRequired += MBCSPRINTF_S(szTemp, TEMP_SIZE, "<%s Alias=\"%s\" Desc=\"%s\">", Group_Name, Group_Alias, szTemp2);\
		if(bPrint)\
			printf("<%s Alias=\"%s\" Desc=\"%s\">", Group_Name, Group_Alias, szTemp2);\
		AMP_NOP1(((void)0))

#define NAV_WRITE_TAG_WITH_ALIAS(Group_Name, Group_Alias, TAG_Desc)	\
		if(szOutXml != 0 && cbLen > 0)\
			cbRequired += MBCSPRINTF_S(szOutXml + cbRequired, cbLen - cbRequired, "<%s Alias=\"%s\" Offset=\"%lld\" Desc=\"%s\" />", Group_Name, Group_Alias, bit_offset?*bit_offset:-1LL, TAG_Desc);\
		else\
			cbRequired += MBCSPRINTF_S(szTemp, TEMP_SIZE, "<%s Alias=\"%s\" Offset=\"%lld\" Desc=\"%s\" />", Group_Name, Group_Alias, bit_offset?*bit_offset:-1LL, TAG_Desc);\
		if(bPrint)\
			printf("<%s Alias=\"%s\" Offset=\"%lld\" Desc=\"%s\" />", Group_Name, Group_Alias, bit_offset?*bit_offset:-1LL, TAG_Desc);\
		AMP_NOP1(((void)0))

#define NAV_WRITE_TAG_WITH_ALIAS_F(Group_Name, Group_Alias, TAG_Desc, ...)	\
		MBCSPRINTF_S(szTagName, TAGNAME_SIZE, Group_Alias, ##__VA_ARGS__);\
		if(szOutXml != 0 && cbLen > 0)\
			cbRequired += MBCSPRINTF_S(szOutXml + cbRequired, cbLen - cbRequired, "<%s Alias=\"%s\" Offset=\"%lld\" Desc=\"%s\" />", Group_Name, szTagName, bit_offset?*bit_offset:-1LL, TAG_Desc);\
		else\
			cbRequired += MBCSPRINTF_S(szTemp, TEMP_SIZE, "<%s Alias=\"%s\" Offset=\"%lld\" Desc=\"%s\" />", Group_Name, szTagName, bit_offset?*bit_offset:-1LL, TAG_Desc);\
		if(bPrint)\
			printf("<%s Alias=\"%s\" Offset=\"%lld\" Desc=\"%s\" />", Group_Name, szTagName, bit_offset?*bit_offset:-1LL, szTemp2);\
		AMP_NOP1(((void)0))

#define NAV_WRITE_AUTOEXPAND_TAG_WITH_ALIAS_F(Group_Name, Group_Alias, TAG_Desc, ...)	\
		MBCSPRINTF_S(szTagName, TAGNAME_SIZE, Group_Alias, ##__VA_ARGS__);\
		if(szOutXml != 0 && cbLen > 0)\
			cbRequired += MBCSPRINTF_S(szOutXml + cbRequired, cbLen - cbRequired, "<%s Alias=\"%s\" Offset=\"%lld\" Desc=\"%s\" auto_expand=\"1\" />", Group_Name, szTagName, bit_offset?*bit_offset:-1LL, TAG_Desc);\
		else\
			cbRequired += MBCSPRINTF_S(szTemp, TEMP_SIZE, "<%s Alias=\"%s\" Offset=\"%lld\" Desc=\"%s\" auto_expand=\"1\" />", Group_Name, szTagName, bit_offset?*bit_offset:-1LL, TAG_Desc);\
		if(bPrint)\
			printf("<%s Alias=\"%s\" Offset=\"%lld\" Desc=\"%s\" auto_expand=\"1\" />", Group_Name, szTagName, bit_offset?*bit_offset:-1LL, szTemp2);\
		AMP_NOP1(((void)0))

#define NAV_WRITE_TAG_WITH_ALIAS_DESC_F(Group_Name, Group_Alias, TAG_Desc, ...)	\
		MBCSPRINTF_S(szTemp2, TEMP2_SIZE, TAG_Desc, ##__VA_ARGS__);\
		if(szOutXml != 0 && cbLen > 0)\
			cbRequired += MBCSPRINTF_S(szOutXml + cbRequired, cbLen - cbRequired, "<%s Alias=\"%s\" Offset=\"%lld\" Desc=\"%s\" />", Group_Name, Group_Alias, bit_offset?*bit_offset:-1LL, szTemp2);\
		else\
			cbRequired += MBCSPRINTF_S(szTemp, TEMP_SIZE, "<%s Alias=\"%s\" Offset=\"%lld\" Desc=\"%s\" />", Group_Name, Group_Alias, bit_offset?*bit_offset:-1LL, szTemp2);\
		if(bPrint)\
			printf("<%s Alias=\"%s\" Offset=\"%lld\" Desc=\"%s\" />", Group_Name, Group_Alias, bit_offset?*bit_offset:-1LL, szTemp2);\
		AMP_NOP1(((void)0))

#define NAV_WRITE_TAG_WITH_VALUEFMTSTR(Group_Name, FormatStr, TAG_Desc, ...)	\
		if(szOutXml != 0 && cbLen > 0)\
			cbRequired += MBCSPRINTF_S(szOutXml + cbRequired, cbLen - cbRequired, "<%s Desc=\"%s\" Value=\"" FormatStr "\" />", Group_Name, TAG_Desc, ##__VA_ARGS__);\
		else\
			cbRequired += MBCSPRINTF_S(szTemp2, TEMP2_SIZE, "<%s Desc=\"%s\" Value=\"" FormatStr "\" />", Group_Name, TAG_Desc, ##__VA_ARGS__);\
		if(bPrint)\
			printf("<%s Desc=\"%s\" Value=\"" FormatStr "\" />", Group_Name, TAG_Desc, ##__VA_ARGS__);\
		AMP_NOP1(((void)0))

#define NAV_WRITE_TAGALIAS_WITH_VALUEFMTSTR(TAG_Name, TAG_Alias, FormatStr, TAG_Desc, ...)	\
		if(szOutXml != 0 && cbLen > 0)\
			cbRequired += MBCSPRINTF_S(szOutXml + cbRequired, cbLen - cbRequired, "<%s Alias=\"%s\" Desc=\"%s\" Value=\"" FormatStr "\" />", TAG_Name, TAG_Alias, TAG_Desc, ##__VA_ARGS__);\
		else\
			cbRequired += MBCSPRINTF_S(szTemp2, TEMP2_SIZE, "<%s Alias=\"%s\" Desc=\"%s\" Value=\"" FormatStr "\" />", TAG_Name, TAG_Alias, TAG_Desc, ##__VA_ARGS__);\
		if(bPrint)\
			printf("<%s Alias=\"%s\" Desc=\"%s\" Value=\"" FormatStr "\" />", TAG_Name, TAG_Alias, TAG_Desc, ##__VA_ARGS__);\
		AMP_NOP1(((void)0))

#define NAV_WRITE_TAG_WITH_ALIAS_VALUEFMTSTR_AND_NUMBER_VALUE(Group_Name, Group_Alias, FormatStr, nValue, TAG_Desc, ...)	\
		MBCSPRINTF_S(szTemp4, TEMP4_SIZE, Group_Alias, ##__VA_ARGS__);\
		if(szOutXml != 0 && cbLen > 0)\
			cbRequired += MBCSPRINTF_S(szOutXml + cbRequired, cbLen - cbRequired, "<%s Alias=\"%s\" Value=\"" FormatStr "\" Desc=\"%s\" />", Group_Name, szTemp4, nValue, nValue, TAG_Desc);\
		else\
			cbRequired += MBCSPRINTF_S(szTemp2, TEMP2_SIZE, "<%s Alias=\"%s\" Value=\"" FormatStr "\" Desc=\"%s\" />", Group_Name, szTemp4, nValue, nValue, TAG_Desc);\
		if(bPrint)\
			printf("<%s Alias=\"%s\" Value=\"" FormatStr "\" Desc=\"%s\" />", Group_Name, szTemp4, nValue, nValue, TAG_Desc);\
		AMP_NOP1(((void)0))

#define NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE(Group_Name, Group_Alias, nValue, TAG_Desc, ...)	\
		MBCSPRINTF_S(szTemp4, TEMP4_SIZE, Group_Alias, ##__VA_ARGS__);\
		if(szOutXml != 0 && cbLen > 0)\
			cbRequired += MBCSPRINTF_S(szOutXml + cbRequired, cbLen - cbRequired, "<%s Alias=\"%s\" Value=\"%d(0X%X)\" Desc=\"%s\" />", Group_Name, szTemp4, nValue, nValue, TAG_Desc);\
		else\
			cbRequired += MBCSPRINTF_S(szTemp2, TEMP2_SIZE, "<%s Alias=\"%s\" Value=\"%d(0X%X)\" Desc=\"%s\" />", Group_Name, szTemp4, nValue, nValue, TAG_Desc);\
		if(bPrint)\
			printf("<%s Alias=\"%s\" Value=\"%d(0X%X)\" Desc=\"%s\" />", Group_Name, szTemp4, nValue, nValue, TAG_Desc);\
		AMP_NOP1(((void)0))

#define NAV_WRITE_TAG_WITH_ALIAS_AND_HEXWORD_VALUE(Group_Name, Group_Alias, nValue, TAG_Desc, ...)	\
		MBCSPRINTF_S(szTemp4, TEMP4_SIZE, Group_Alias, ##__VA_ARGS__);\
		if(szOutXml != 0 && cbLen > 0)\
			cbRequired += MBCSPRINTF_S(szOutXml + cbRequired, cbLen - cbRequired, "<%s Alias=\"%s\" Value=\"0X%04X\" Desc=\"%s\" />", Group_Name, szTemp4, nValue, TAG_Desc);\
		else\
			cbRequired += MBCSPRINTF_S(szTemp2, TEMP2_SIZE, "<%s Alias=\"%s\" Value=\"0X%04X\" Desc=\"%s\" />", Group_Name, szTemp4, nValue, TAG_Desc);\
		if(bPrint)\
			printf("<%s Alias=\"%s\" Value=\"0X%04X\" Desc=\"%s\" />", Group_Name, szTemp4, nValue, TAG_Desc);\
		AMP_NOP1(((void)0))

#define NAV_WRITE_TAG_WITH_NUMBER_VALUE(Group_Name, nValue, TAG_Desc)	\
		if(szOutXml != 0 && cbLen > 0)\
			cbRequired += MBCSPRINTF_S(szOutXml + cbRequired, cbLen - cbRequired, "<%s Offset=\"%lld\" Value=\"%d(0X%X)\" Desc=\"%s\" />", Group_Name, bit_offset?*bit_offset:-1LL, nValue, nValue, TAG_Desc);\
		else\
			cbRequired += MBCSPRINTF_S(szTemp, TEMP_SIZE, "<%s Offset=\"%lld\" Value=\"%d(0X%X)\" Desc=\"%s\" />", Group_Name, bit_offset?*bit_offset:-1LL, nValue, nValue, TAG_Desc);\
		if(bPrint)\
			printf("<%s Offset=\"%lld\" Value=\"%d(0X%X)\" Desc=\"%s\" />", Group_Name, bit_offset?*bit_offset:-1LL, nValue, nValue, TAG_Desc);\
		AMP_NOP1(((void)0))

#define NAV_WRITE_TAG_WITH_NUMBER_VALUE_DESC_F(Group_Name, nValue, TAG_Desc, ...)	\
		MBCSPRINTF_S(szTemp2, TEMP2_SIZE, TAG_Desc, ##__VA_ARGS__);\
		if(szOutXml != 0 && cbLen > 0)\
			cbRequired += MBCSPRINTF_S(szOutXml + cbRequired, cbLen - cbRequired, "<%s Offset=\"%lld\" Value=\"%d(0X%X)\" Desc=\"%s\" />", Group_Name, bit_offset?*bit_offset:-1LL, nValue, nValue, szTemp2);\
		else\
			cbRequired += MBCSPRINTF_S(szTemp, TEMP_SIZE, "<%s Offset=\"%lld\" Value=\"%d(0X%X)\" Desc=\"%s\" />", Group_Name, bit_offset?*bit_offset:-1LL, nValue, nValue, szTemp2);\
		if(bPrint)\
			printf("<%s Offset=\"%lld\" Value=\"%d(0X%X)\" Desc=\"%s\" />", Group_Name, bit_offset?*bit_offset:-1LL, nValue, nValue, szTemp2);\
		AMP_NOP1(((void)0))

#define NAV_WRITE_TAG_WITH_1NUMBER_VALUE(Group_Name, nValue, TAG_Desc)	\
		if(szOutXml != 0 && cbLen > 0)\
			cbRequired += MBCSPRINTF_S(szOutXml + cbRequired, cbLen - cbRequired, "<%s Offset=\"%lld\" Value=\"%d\" Desc=\"%s\" />", Group_Name, bit_offset?*bit_offset:-1LL, nValue, TAG_Desc);\
		else\
			cbRequired += MBCSPRINTF_S(szTemp2, TEMP2_SIZE, "<%s Offset=\"%lld\" Value=\"%d\" Desc=\"%s\" />", Group_Name, bit_offset?*bit_offset:-1LL, nValue, TAG_Desc);\
		if(bPrint)\
			printf("<%s Offset=\"%lld\" Value=\"%d\" Desc=\"%s\" />", Group_Name, bit_offset?*bit_offset:-1LL, nValue, TAG_Desc);\
		AMP_NOP1(((void)0))

#define NAV_WRITE_TAG_WITH_1NUMBER_VALUE_BEGIN(Group_Name, Group_Alias, nValue, TAG_Desc)	\
		if (Group_Alias[0] == '\0'){\
			if(szOutXml != 0 && cbLen > 0)\
				cbRequired += MBCSPRINTF_S(szOutXml + cbRequired, cbLen - cbRequired, "<%s Value=\"%d\" Desc=\"%s\" >", Group_Name, nValue, TAG_Desc);\
			else\
				cbRequired += MBCSPRINTF_S(szTemp2, TEMP2_SIZE, "<%s Value=\"%d\" Desc=\"%s\" >", Group_Name, nValue, TAG_Desc);\
			if(bPrint)\
				printf("<%s Value=\"%d\" Desc=\"%s\" >", Group_Name, nValue, TAG_Desc);\
		} else {\
			if(szOutXml != 0 && cbLen > 0)\
				cbRequired += MBCSPRINTF_S(szOutXml + cbRequired, cbLen - cbRequired, "<%s Alias=\"%s\" Value=\"%d\" Desc=\"%s\" >", Group_Name, Group_Alias, nValue, TAG_Desc);\
			else\
				cbRequired += MBCSPRINTF_S(szTemp2, TEMP2_SIZE, "<%s Alias=\"%s\" Value=\"%d\" Desc=\"%s\" >", Group_Name, Group_Alias, nValue, TAG_Desc);\
			if(bPrint)\
				printf("<%s Alias=\"%s\" Value=\"%d\" Desc=\"%s\" >", Group_Name, Group_Alias, nValue, TAG_Desc);\
		}\
		AMP_NOP1(((void)0))

#define NAV_WRITE_TAG_WITH_1NUMBER_VALUE_BEGIN_ALIAS_F(Group_Name, Group_Alias, nValue, TAG_Desc, ...)	\
		MBCSPRINTF_S(szTagName, TAGNAME_SIZE, Group_Alias, ##__VA_ARGS__);\
		if(szOutXml != 0 && cbLen > 0)\
			cbRequired += MBCSPRINTF_S(szOutXml + cbRequired, cbLen - cbRequired, "<%s Alias=\"%s\" Value=\"%d\" Desc=\"%s\" >", Group_Name, szTagName, nValue, TAG_Desc);\
		else\
			cbRequired += MBCSPRINTF_S(szTemp2, TEMP2_SIZE, "<%s Alias=\"%s\" Value=\"%d\" Desc=\"%s\" >", Group_Name, szTagName, nValue, TAG_Desc);\
		if(bPrint)\
			printf("<%s Alias=\"%s\" Value=\"%d\" Desc=\"%s\" >", Group_Name, szTemp2, nValue, TAG_Desc);\
		AMP_NOP1(((void)0))

#define NAV_WRITE_TAG_WITH_LL_NUMBER_VALUE(Group_Name, nValue, TAG_Desc)	\
		if(szOutXml != 0 && cbLen > 0)\
			cbRequired += MBCSPRINTF_S(szOutXml + cbRequired, cbLen - cbRequired, "<%s Offset=\"%lld\" Value=\"%" PRId64 "(0X%" PRIX64 ")\" Desc=\"%s\" />", Group_Name, bit_offset?*bit_offset:-1LL, nValue, nValue, TAG_Desc);\
		else\
			cbRequired += MBCSPRINTF_S(szTemp2, TEMP2_SIZE, "<%s Offset=\"%lld\" Value=\"%" PRId64 "(0X%" PRIX64 ")\" Desc=\"%s\" />", Group_Name, bit_offset?*bit_offset:-1LL, nValue, nValue, TAG_Desc);\
		if(bPrint)\
			printf("<%s Offset=\"%lld\" Value=\"%" PRId64 "(0X%" PRIX64 ")\" Desc=\"%s\" />", Group_Name, bit_offset?*bit_offset:-1LL, nValue, nValue, TAG_Desc);\
		AMP_NOP1(((void)0))

#define NAV_WRITE_TAG_WITH_NUMBER_VALUE1(Group_Name, TAG_DESC)	NAV_WRITE_TAG_WITH_NUMBER_VALUE(#Group_Name, Group_Name, TAG_DESC)
#define NAV_WRITE_TAG_WITH_1NUMBER_VALUE1(Group_Name, TAG_DESC)	NAV_WRITE_TAG_WITH_1NUMBER_VALUE(#Group_Name, Group_Name, TAG_DESC)

#define NAV_WRITE_TAG_BEGIN2(Group_Name)\
		NAV_WRITE_TAG_BEGIN(Group_Name)

#define NAV_WRITE_TAG_BEGIN2_1(Group_Name, TAG_Desc)\
		NAV_WRITE_TAG_BEGIN_1(Group_Name, TAG_Desc, 0)

#define NAV_WRITE_TAG_BEGIN3(Group_Name, idx)\
		if(szOutXml != 0 && cbLen > 0)\
			cbRequired += MBCSPRINTF_S(szOutXml + cbRequired, cbLen - cbRequired, "<%s Alias=\"%s[%lu]\" Offset=\"%lld\">", Group_Name, Group_Name, (unsigned long)(idx), bit_offset?*bit_offset:-1LL);\
		else\
			cbRequired += MBCSPRINTF_S(szTemp4, TEMP4_SIZE, "<%s Alias=\"%s[%lu]\" Offset=\"%lld\">", Group_Name, Group_Name, (unsigned long)(idx), bit_offset?*bit_offset:-1LL);\
		if(bPrint)\
			printf("<%s Alias=\"%s[%lu]\" Offset=\"%lld\">", Group_Name, Group_Name, (unsigned long)(idx), bit_offset?*bit_offset:-1LL);\
		AMP_NOP1(((void)0))

#define NAV_WRITE_TAG_BEGIN3_1(Group_Name, idx, TAG_Desc)\
		if(szOutXml != 0 && cbLen > 0)\
			cbRequired += MBCSPRINTF_S(szOutXml + cbRequired, cbLen - cbRequired, "<%s Alias=\"%s[%lu]\" Offset=\"%lld\" Desc=\"%s\">", Group_Name, Group_Name, (unsigned long)(idx), bit_offset?*bit_offset:-1LL, TAG_Desc);\
		else\
			cbRequired += MBCSPRINTF_S(szTemp, TEMP_SIZE, "<%s Alias=\"%s[%lu]\" Offset=\"%lld\" Desc=\"%s\">", Group_Name, Group_Name, (unsigned long)(idx), bit_offset?*bit_offset:-1LL, TAG_Desc);\
		if(bPrint)\
			printf("<%s Alias=\"%s[%lu]\" Offset=\"%lld\" Desc=\"%s\">", Group_Name, Group_Name, (unsigned long)(idx), bit_offset?*bit_offset:-1LL, TAG_Desc);\
		AMP_NOP1(((void)0))

#define NAV_WRITE_TAG_BEGIN4(Group_Name, formatstr, idx)\
		if(szOutXml != 0 && cbLen > 0)\
			cbRequired += MBCSPRINTF_S(szOutXml + cbRequired, cbLen - cbRequired, "<%s_" formatstr " Offset=\"%lld\">", Group_Name, (unsigned long)(idx), bit_offset?*bit_offset:-1LL);\
		else\
			cbRequired += MBCSPRINTF_S(szTemp4, TEMP4_SIZE, "<%s_" formatstr " Offset=\"%lld\">", Group_Name, (unsigned long)(idx), bit_offset?*bit_offset:-1LL);\
		if(bPrint)\
			printf("<%s>\n", szTemp4);\

#define NAV_WRITE_TAG_ARRAY_BEGIN_(Group_Name, prefix_idx, idx, TAG_Desc, ...)\
		MBCSPRINTF_S(szTemp2, sizeof(szTemp2)/sizeof(szTemp2[0]), TAG_Desc, ##__VA_ARGS__);\
		if(szOutXml != 0 && cbLen > 0)\
			cbRequired += MBCSPRINTF_S(szOutXml + cbRequired, cbLen - cbRequired, "<%s Alias=\"%s[%s%lu]\" Offset=\"%lld\" Desc=\"%s\">", Group_Name, Group_Name, prefix_idx, (unsigned long)(idx), bit_offset?*bit_offset:-1LL, szTemp2);\
		else\
			cbRequired += MBCSPRINTF_S(szTemp, TEMP_SIZE, "<%s Alias=\"%s[%s%lu]\" Offset=\"%lld\" Desc=\"%s\">", Group_Name, Group_Name, prefix_idx, (unsigned long)(idx), bit_offset?*bit_offset:-1LL, szTemp2);\
		if(bPrint)\
			printf("<%s Alias=\"%s[%s%lu]\" Offset=\"%lld\" Desc=\"%s\">", Group_Name, Group_Name, prefix_idx, (unsigned long)(idx), bit_offset?*bit_offset:-1LL, szTemp2);\
		AMP_NOP1(((void)0))

#define NAV_WRITE_TAG_ARRAY_BEGIN__(Group_Name, prfix_array, prefix_idx, idx, TAG_Desc, ...)\
		MBCSPRINTF_S(szTemp2, TEMP2_SIZE, TAG_Desc, ##__VA_ARGS__);\
		if(szOutXml != 0 && cbLen > 0)\
			cbRequired += MBCSPRINTF_S(szOutXml + cbRequired, cbLen - cbRequired, "<%s Alias=\"%s[%s%lu]\" Offset=\"%lld\" Desc=\"%s\">", Group_Name, prfix_array, prefix_idx, (unsigned long)(idx), bit_offset?*bit_offset:-1LL, szTemp2);\
		else\
			cbRequired += MBCSPRINTF_S(szTemp, TEMP_SIZE, "<%s Alias=\"%s[%s%lu]\" Offset=\"%lld\" Desc=\"%s\">", Group_Name, prfix_array, prefix_idx, (unsigned long)(idx), bit_offset?*bit_offset:-1LL, szTemp2);\
		if(bPrint)\
			printf("<%s Alias=\"%s[%s%lu]\" Offset=\"%lld\" Desc=\"%s\">", Group_Name, prfix_array, prefix_idx, (unsigned long)(idx), bit_offset?*bit_offset:-1LL, szTemp2);\
		AMP_NOP1(((void)0))

#define NAV_WRITE_TAG_ARRAY_BEGIN(Group_Name, idx, TAG_Desc, ...)	NAV_WRITE_TAG_ARRAY_BEGIN_(Group_Name, "", idx, TAG_Desc)

#define NAV_WRITE_TAG_ARRAY_BEGIN0(Group_Name, idx, TAG_Desc)	\
		if(szOutXml != 0 && cbLen > 0)\
			cbRequired += MBCSPRINTF_S(szOutXml + cbRequired, cbLen - cbRequired, "<%s Alias=\"%s[%lu]\" Offset=\"%lld\" Desc=\"%s\">", Group_Name, Group_Name, (unsigned long)(idx), bit_offset?*bit_offset:-1LL, TAG_Desc);\
		else\
			cbRequired += MBCSPRINTF_S(szTemp, TEMP_SIZE, "<%s Alias=\"%s[%lu]\" Offset=\"%lld\" Desc=\"%s\">", Group_Name, Group_Name, (unsigned long)(idx), bit_offset?*bit_offset:-1LL, TAG_Desc);\
		if(bPrint)\
			printf("<%s Alias=\"%s[%lu]\" Offset=\"%lld\" Desc=\"%s\">", Group_Name, Group_Name, (unsigned long)(idx), bit_offset?*bit_offset:-1LL, TAG_Desc);\
		AMP_NOP1(((void)0))

#define NAV_FIELD_PROP(Field_Name, Field_Bits, Field_Value, Field_Desc, Field_Bit_Offset, Type)\
	if(szOutXml != 0 && cbLen > 0)\
		cbRequired += MBCSPRINTF_S(szOutXml + cbRequired, cbLen - cbRequired, "<%s Value=\"%s\" Bits=\"%lu\" Desc=\"%s\" Offset=\"%lld\" Type=\"%s\"/>", Field_Name, Field_Value, (unsigned long)(Field_Bits), Field_Desc, Field_Bit_Offset, Type);\
	else\
		cbRequired += MBCSPRINTF_S(szTemp, TEMP_SIZE, "<%s Value=\"%s\" Bits=\"%lu\" Desc=\"%s\" Offset=\"%lld\" Type=\"%s\"/>", Field_Name, Field_Value, (unsigned long)(Field_Bits), Field_Desc, Field_Bit_Offset, Type);\
	if(bPrint)\
		printf("%s\n", szTemp);\

#define NAV_FIELD_PROP_F(Field_Name, Field_Bits, Field_Value_FmtStr, Field_Desc, Field_Bit_Offset, Type, ...)\
	if(szOutXml != 0 && cbLen > 0)\
		cbRequired += MBCSPRINTF_S(szOutXml + cbRequired, cbLen - cbRequired, "<%s Bits=\"%lu\" Desc=\"%s\" Offset=\"%lld\" Type=\"%s\" Value=\"" Field_Value_FmtStr "\"/>", Field_Name, (unsigned long)(Field_Bits), Field_Desc, Field_Bit_Offset, Type, ##__VA_ARGS__);\
	else\
		cbRequired += MBCSPRINTF_S(szTemp, TEMP_SIZE, "<%s Bits=\"%lu\" Desc=\"%s\" Offset=\"%lld\" Type=\"%s\" Value=\"" Field_Value_FmtStr "\"/>", Field_Name, (unsigned long)(Field_Bits), Field_Desc, Field_Bit_Offset, Type, ##__VA_ARGS__);\
	if(bPrint)\
		printf("%s\n", szTemp);\

#define NAV_FIELD_PROP_BEGIN(Field_Name, Field_Bits, Field_Value, Field_Desc, Field_Bit_Offset, Type)\
	if(szOutXml != 0 && cbLen > 0)\
		cbRequired += MBCSPRINTF_S(szOutXml + cbRequired, cbLen - cbRequired, "<%s Value=\"%s\" Bits=\"%lu\" Desc=\"%s\" Offset=\"%lld\" Type=\"%s\">", Field_Name, Field_Value, (unsigned long)(Field_Bits), Field_Desc, Field_Bit_Offset, Type);\
	else\
		cbRequired += MBCSPRINTF_S(szTemp, TEMP_SIZE, "<%s Value=\"%s\" Bits=\"%lu\" Desc=\"%s\" Offset=\"%lld\" Type=\"%s\">", Field_Name, Field_Value, (unsigned long)(Field_Bits), Field_Desc, Field_Bit_Offset, Type);\
	if(bPrint)\
		printf("%s\n", szTemp);\
	if (bit_offset)*bit_offset += Field_Bits;\

#define NAV_FIELD_PROP_WITH_ALIAS(Field_Name, Field_Alias, Field_Bits, Field_Value, Field_Desc, Field_Bit_Offset, Type)\
	if(szOutXml != 0 && cbLen > 0)\
		cbRequired += MBCSPRINTF_S(szOutXml + cbRequired, cbLen - cbRequired, "<%s Alias=\"%s\" Value=\"%s\" Bits=\"%lu\" Desc=\"%s\" Offset=\"%lld\" Type=\"%s\"/>", Field_Name, Field_Alias, Field_Value, (unsigned long)(Field_Bits), Field_Desc, Field_Bit_Offset, Type);\
	else\
		cbRequired += MBCSPRINTF_S(szTemp, TEMP_SIZE, "<%s Alias=\"%s\" Value=\"%s\" Bits=\"%lu\" Desc=\"%s\" Offset=\"%lld\" Type=\"%s\"/>", Field_Name, Field_Alias, Field_Value, (unsigned long)(Field_Bits), Field_Desc, Field_Bit_Offset, Type);\
	if(bPrint)\
		printf("%s\n", szTemp);\
	if (bit_offset)*bit_offset += Field_Bits;\

#define NAV_FIELD_PROP_WITH_ALIAS_VALUEFMT(Field_Name, Field_Alias, Field_Bits, Field_Value_FmtStr, Field_Desc, Field_Bit_Offset, Type, ...)\
	if(szOutXml != 0 && cbLen > 0)\
		cbRequired += MBCSPRINTF_S(szOutXml + cbRequired, cbLen - cbRequired, \
			"<%s Alias=\"%s\" Bits=\"%lu\" Desc=\"%s\" Offset=\"%lld\" Type=\"%s\" Value=\"" Field_Value_FmtStr "\"/>", \
			Field_Name, Field_Alias, (unsigned long)(Field_Bits), Field_Desc, Field_Bit_Offset, Type, ##__VA_ARGS__);\
	else\
		cbRequired += MBCSPRINTF_S(szTemp, TEMP_SIZE, \
			"<%s Alias=\"%s\" Bits=\"%lu\" Desc=\"%s\" Offset=\"%lld\" Type=\"%s\" Value=\"" Field_Value_FmtStr "\"/>", \
			Field_Name, Field_Alias, (unsigned long)(Field_Bits), Field_Desc, Field_Bit_Offset, Type, ##__VA_ARGS__);\
	if(bPrint)\
		printf("%s\n", szTemp);\
	if (bit_offset)*bit_offset += Field_Bits;\

#define NAV_FIELD_PROP_WITH_ALIAS_BEGIN(Field_Name, Field_Alias, Field_Bits, Field_Value, Field_Desc, Field_Bit_Offset, Type)\
	if(szOutXml != 0 && cbLen > 0)\
		cbRequired += MBCSPRINTF_S(szOutXml + cbRequired, cbLen - cbRequired, "<%s Alias=\"%s\" Value=\"%s\" Bits=\"%lu\" Desc=\"%s\" Offset=\"%lld\" Type=\"%s\">", Field_Name, Field_Alias, Field_Value, (unsigned long)(Field_Bits), Field_Desc, Field_Bit_Offset, Type);\
	else\
		cbRequired += MBCSPRINTF_S(szTemp, TEMP_SIZE, "<%s Alias=\"%s\" Value=\"%s\" Bits=\"%lu\" Desc=\"%s\" Offset=\"%lld\" Type=\"%s\">", Field_Name, Field_Alias, Field_Value, (unsigned long)(Field_Bits), Field_Desc, Field_Bit_Offset, Type);\
	if(bPrint)\
		printf("%s\n", szTemp);\
	if (bit_offset)*bit_offset += Field_Bits;\

#define NAV_FIELD_PROP_END(Field_Name)	NAV_WRITE_TAG_END(Field_Name)

#define NAV_FIELD_PROP_NUMBER(Field_Name, Field_Bits, Field_Value, Field_Desc)\
	MBCSPRINTF_S(szTemp3, TEMP3_SIZE, "%lu", (unsigned long)(Field_Value));\
	NAV_FIELD_PROP(Field_Name, Field_Bits, szTemp3, Field_Desc, bit_offset?*bit_offset:-1LL, "I");\
	if (bit_offset)*bit_offset += Field_Bits;\

#define NAV_FIELD_PROP_NUMBER_ARRAY_F(Field_Name, Field_Bits, FormatStr, Field_Desc, ...)\
	NAV_FIELD_PROP_F(Field_Name, Field_Bits, FormatStr, Field_Desc, bit_offset?*bit_offset:-1LL, "IA", ##__VA_ARGS__);\
	if (bit_offset)*bit_offset += Field_Bits;\

#define NAV_FIELD_PROP_NUMBER_BEGIN(Field_Name, Field_Bits, Field_Value, Field_Desc)\
	MBCSPRINTF_S(szTemp3, TEMP3_SIZE, "%lu", (unsigned long)(Field_Value));\
	NAV_FIELD_PROP_BEGIN(Field_Name, Field_Bits, szTemp3, Field_Desc, bit_offset?*bit_offset:-1LL, "I");\

#define NAV_FIELD_PROP_NUMBER_WITH_ALIAS_BEGIN(Field_Name, Field_Alias, Field_Bits, Field_Value, Field_Desc)\
	MBCSPRINTF_S(szTemp3, TEMP3_SIZE, "%lu", (unsigned long)(Field_Value));\
	NAV_FIELD_PROP_WITH_ALIAS_BEGIN(Field_Name, Field_Alias, Field_Bits, szTemp3, Field_Desc, bit_offset?*bit_offset:-1LL, "I");\

#define NAV_FIELD_PROP_NUMBER_END(Field_Name)	NAV_FIELD_PROP_END(Field_Name)

#define NAV_FIELD_PROP_NUMBER1(Field_Name, Field_Bits, Field_Desc)		NAV_FIELD_PROP_NUMBER(#Field_Name, Field_Bits, Field_Name, Field_Desc)

#define BST_FIELD_PROP_NUMBER_ARRAY_F(Field_Name, Field_Bits, FormatStr, Field_Desc, ...)\
	if (map_status.status == 0 || (map_status.error == 0 &&  map_status.number_of_fields > 0 && field_prop_idx < map_status.number_of_fields) ){\
		NAV_FIELD_PROP_NUMBER_ARRAY_F(Field_Name, Field_Bits, FormatStr, Field_Desc, ##__VA_ARGS__)\
		field_prop_idx++;}\

#define BST_FIELD_PROP_NUMBER(Field_Name, Field_Bits, Field_Value, Field_Desc)\
	if (map_status.status == 0 || (map_status.error == 0 &&  map_status.number_of_fields > 0 && field_prop_idx < map_status.number_of_fields) ){\
		NAV_FIELD_PROP_NUMBER(Field_Name, Field_Bits, Field_Value, Field_Desc)\
		field_prop_idx++;}\

#define BST_FIELD_PROP_SIGNNUMBER(Field_Name, Field_Bits, Field_Value, Field_Desc)\
	if (map_status.status == 0 || (map_status.error == 0 &&  map_status.number_of_fields > 0 && field_prop_idx < map_status.number_of_fields) ){\
		NAV_FIELD_PROP_SIGNNUMBER(Field_Name, Field_Bits, Field_Value, Field_Desc)\
		field_prop_idx++;}\

#define BST_FIELD_PROP_NUMBER_BEGIN(Field_Name, Field_Bits, Field_Value, Field_Desc)\
	if (map_status.status == 0 || (map_status.error == 0 &&  map_status.number_of_fields > 0 && field_prop_idx < map_status.number_of_fields) ){\
		NAV_FIELD_PROP_NUMBER_BEGIN(Field_Name, Field_Bits, Field_Value, Field_Desc)\
		field_prop_idx++;\

#define BST_FIELD_PROP_NUMBER_WITH_ALIAS_BEGIN(Field_Name, Field_Alias, Field_Bits, Field_Value, Field_Desc)\
	if (map_status.status == 0 || (map_status.error == 0 &&  map_status.number_of_fields > 0 && field_prop_idx < map_status.number_of_fields) ){\
		NAV_FIELD_PROP_NUMBER_WITH_ALIAS_BEGIN(Field_Name, Field_Alias, Field_Bits, Field_Value, Field_Desc)\
		field_prop_idx++;\

#define BST_FIELD_PROP_NUMBER_END(Field_Name)	NAV_FIELD_PROP_NUMBER_END(Field_Name);}

#define BST_FIELD_PROP_NUMBER1(Field_Name, Field_Bits, Field_Desc)		BST_FIELD_PROP_NUMBER(#Field_Name, Field_Bits, Field_Name, Field_Desc)
#define BST_FIELD_PROP_BOOL(Field_Name, TRUE_Desc, FALSE_Desc)			BST_FIELD_PROP_NUMBER1(Field_Name, 1, Field_Name?TRUE_Desc:FALSE_Desc)
#define BST_FIELD_PROP_BOOL1(Obj, Field_Name, TRUE_Desc, FALSE_Desc)	BST_FIELD_PROP_NUMBER(#Field_Name, 1, (uint32_t)Obj.Field_Name, Obj.Field_Name?TRUE_Desc:FALSE_Desc)

#define BST_FIELD_PROP_BOOL_BEGIN(Field_Name, TRUE_Desc, FALSE_Desc)\
	if (map_status.status == 0 || (map_status.error == 0 &&  map_status.number_of_fields > 0 && field_prop_idx < map_status.number_of_fields) ){\
		MBCSPRINTF_S(szTemp3, TEMP3_SIZE, "%d", (int)Field_Name);\
		NAV_FIELD_PROP_BEGIN(#Field_Name, 1, szTemp3, Field_Name?TRUE_Desc:FALSE_Desc, bit_offset?*bit_offset:-1LL, "I");\
		if (bit_offset)*bit_offset += 1;\
		field_prop_idx++;\

#define BST_FIELD_PROP_BOOL_BEGIN_(Field_Name, Field_Value, TRUE_Desc, FALSE_Desc)\
	if (map_status.status == 0 || (map_status.error == 0 &&  map_status.number_of_fields > 0 && field_prop_idx < map_status.number_of_fields) ){\
		MBCSPRINTF_S(szTemp3, TEMP3_SIZE, "%d", (int)Field_Value);\
		NAV_FIELD_PROP_BEGIN(Field_Name, 1, szTemp3, Field_Value?TRUE_Desc:FALSE_Desc, bit_offset?*bit_offset:-1LL, "I");\
		if (bit_offset)*bit_offset += 1;\
		field_prop_idx++;\

#define BST_ARRAY_FIELD_PROP_BOOL_BEGIN_(Field_Name, Prefix_Idx, Idx, Field_Value, TRUE_Desc, FALSE_Desc)\
	if (map_status.status == 0 || (map_status.error == 0 &&  map_status.number_of_fields > 0 && field_prop_idx < map_status.number_of_fields) ){\
		MBCSPRINTF_S(szTagName, TAGNAME_SIZE, "%s[%s%d]", #Field_Name, Prefix_Idx, (int)Idx);\
		MBCSPRINTF_S(szTemp3, TEMP3_SIZE, "%d", (int)Field_Value);\
		NAV_FIELD_PROP_WITH_ALIAS_BEGIN(#Field_Name, szTagName, 1, szTemp3, Field_Value?TRUE_Desc:FALSE_Desc, bit_offset?*bit_offset:-1LL, "I");\
		field_prop_idx++;\

#define BST_ARRAY_FIELD_PROP_BOOL_BEGIN(Field_Name, Prefix_Idx, Idx, TRUE_Desc, FALSE_Desc)\
	BST_ARRAY_FIELD_PROP_BOOL_BEGIN_(Field_Name, Prefix_Idx, Idx, Field_Name[Idx], TRUE_Desc, FALSE_Desc)

#define BST_FIELD_PROP_BOOL_END(Field_Name)	NAV_WRITE_TAG_END(Field_Name);}
#define BST_ARRAY_FIELD_PROP_BOOL_END(Field_Name) NAV_WRITE_TAG_END(#Field_Name);}

#define NAV_FIELD_PROP_SIGNNUMBER(Field_Name, Field_Bits, Field_Value, Field_Desc)\
	MBCSPRINTF_S(szTemp3, TEMP3_SIZE, "%ld", ((long)(Field_Value)));\
	NAV_FIELD_PROP(Field_Name, Field_Bits, szTemp3, Field_Desc, bit_offset?*bit_offset:-1LL, "I");\
	if (bit_offset)*bit_offset += Field_Bits;\

#define NAV_FIELD_PROP_SIGNNUMBER_(Field_Name, Field_Bits, Field_Value_Fmt, Field_Value, Field_Desc)\
	MBCSPRINTF_S(szTemp3, TEMP3_SIZE, Field_Value_Fmt, Field_Value);\
	NAV_FIELD_PROP(Field_Name, Field_Bits, szTemp3, Field_Desc, bit_offset?*bit_offset:-1LL, "I");\
	if (bit_offset)*bit_offset += Field_Bits;\

#define NAV_FIELD_PROP_SIGNNUMBER1(Field_Name, Field_Bits, Field_Desc)	NAV_FIELD_PROP_SIGNNUMBER(#Field_Name, Field_Bits, Field_Name, Field_Desc)

#define NAV_FIELD_PROP_NUMBER_IDX(Field_Name, Idx, Field_Bits, Field_Value, Field_Desc)\
	MBCSPRINTF_S(szTemp3, TEMP3_SIZE, "%lu#%d", (unsigned long)(Field_Value), (int)Idx);\
	NAV_FIELD_PROP(Field_Name, Field_Bits, szTemp3, Field_Desc, bit_offset?*bit_offset:-1LL, "I");\
	if (bit_offset)*bit_offset += Field_Bits;\

#define NAV_FIELD_PROP_NUMBER_IDX1(Field_Name, Field_Bits, Field_Desc)	NAV_FIELD_PROP_NUMBER_IDX(#Field_Name, Field_Bits, Field_Name, Field_Desc)

#define NAV_ARRAY_FIELD_PROP_NUMBER_(Field_Name, Prefix_Idx, Idx, Field_Bits, Field_Value, Field_Desc)\
	MBCSPRINTF_S(szTagName, TAGNAME_SIZE, "%s[%s%d]", Field_Name, Prefix_Idx, (int)Idx);\
	MBCSPRINTF_S(szTemp3, TEMP3_SIZE, "%lu", (unsigned long)(Field_Value));\
	NAV_FIELD_PROP_WITH_ALIAS(Field_Name, szTagName, Field_Bits, szTemp3, Field_Desc, bit_offset?*bit_offset:-1LL, "I");\

#define NAV_ARRAY_FIELD_PROP_NUMBER__(Field_Name, Prefix_array, Prefix_Idx, Idx, Field_Bits, Field_Value, Field_Desc)\
	MBCSPRINTF_S(szTagName, TAGNAME_SIZE, "%s[%s%d]", Prefix_array, Prefix_Idx, (int)Idx);\
	MBCSPRINTF_S(szTemp3, TEMP3_SIZE, "%lu", (unsigned long)(Field_Value));\
	NAV_FIELD_PROP_WITH_ALIAS(Field_Name, szTagName, Field_Bits, szTemp3, Field_Desc, bit_offset?*bit_offset:-1LL, "I");\

#define NAV_ARRAY_FIELD_PROP_NUMBER___(Field_Name, Prefix_array, Prefix_Idx, Idx, Field_Bits, Value_FmtStr, Field_Value, Field_Desc)\
	MBCSPRINTF_S(szTagName, TAGNAME_SIZE, "%s[%s%d]", Prefix_array, Prefix_Idx, (int)Idx);\
	MBCSPRINTF_S(szTemp3, TEMP3_SIZE, Value_FmtStr, Field_Value);\
	NAV_FIELD_PROP_WITH_ALIAS(Field_Name, szTagName, Field_Bits, szTemp3, Field_Desc, bit_offset?*bit_offset:-1LL, "I");\

#define NAV_ARRAY_FIELD_PROP_NUMBER_WITH_ALIAS_VALUEFMT(Field_Name, Prefix_array, Prefix_Idx, Idx, Field_Bits, Value_FmtStr, Field_Desc,...)\
	MBCSPRINTF_S(szTagName, TAGNAME_SIZE, "%s[%s%d]", Prefix_array, Prefix_Idx, (int)Idx);\
	MBCSPRINTF_S(szTemp3, TEMP3_SIZE, Value_FmtStr, Field_Value);\
	NAV_FIELD_PROP_WITH_ALIAS(Field_Name, szTagName, Field_Bits, szTemp3, Field_Desc, bit_offset?*bit_offset:-1LL, "I");\

#define NAV_ARRAY_FIELD_PROP_NUMBER(Field_Name, Idx, Field_Bits, Field_Value, Field_Desc)\
	NAV_ARRAY_FIELD_PROP_NUMBER_(Field_Name, "", Idx, Field_Bits, Field_Value, Field_Desc)

#define NAV_ARRAY_FIELD_PROP_SIGNNUMBER(Field_Name, Idx, Field_Bits, Field_Value, Field_Desc)\
	NAV_ARRAY_FIELD_PROP_SIGNNUMBER_(Field_Name, "", Idx, false, Field_Bits, Field_Value, Field_Desc)

#define NAV_ARRAY_FIELD_PROP_2NUMBER(Field_Name, Idx, Field_Bits, Field_Value, Field_Desc)\
	MBCSPRINTF_S(szTagName, TAGNAME_SIZE, "%s[%d]", Field_Name, (int)Idx);\
	MBCSPRINTF_S(szTemp3, TEMP3_SIZE, "%" PRIu32 "(0X%" PRIX32 ")", (uint32_t)(Field_Value), (uint32_t)(Field_Value));\
	NAV_FIELD_PROP_WITH_ALIAS(Field_Name, szTagName, Field_Bits, szTemp3, Field_Desc, bit_offset?*bit_offset:-1LL, "I");\

#define NAV_ARRAY_FIELD_PROP_2NUMBER_DESC_F(Field_Name, Idx, Field_Bits, Field_Value, Field_Desc, ...)\
	MBCSPRINTF_S(szTemp2, sizeof(szTemp2)/sizeof(szTemp2[0]), Field_Desc, ##__VA_ARGS__);\
	MBCSPRINTF_S(szTagName, TAGNAME_SIZE, "%s[%d]", Field_Name, (int)Idx);\
	MBCSPRINTF_S(szTemp3, TEMP3_SIZE, "%" PRIu32 "(0X%" PRIX32 ")", (uint32_t)(Field_Value), (uint32_t)(Field_Value));\
	NAV_FIELD_PROP_WITH_ALIAS(Field_Name, szTagName, Field_Bits, szTemp3, szTemp2, bit_offset?*bit_offset:-1LL, "I");\

#define NAV_ARRAY_FIELD_PROP_2NUMBER_(Field_Name, Prefix_Idx, Idx, Simple, Field_Bits, Field_Value, Field_Desc)\
	MBCSPRINTF_S(szTagName, TAGNAME_SIZE, "%s[%s%d]", Simple?"":Field_Name, Prefix_Idx, (int)Idx);\
	MBCSPRINTF_S(szTemp3, TEMP3_SIZE, "%" PRIu32 "(0X%" PRIX32 ")", (uint32_t)(Field_Value), (uint32_t)(Field_Value));\
	NAV_FIELD_PROP_WITH_ALIAS(Field_Name, szTagName, Field_Bits, szTemp3, Field_Desc, bit_offset?*bit_offset:-1LL, "I");\

#define NAV_ARRAY_FIELD_PROP_SIGNNUMBER_(Field_Name, Prefix_Idx, Idx, Simple, Field_Bits, Field_Value, Field_Desc)\
	MBCSPRINTF_S(szTagName, TAGNAME_SIZE, "%s[%s%d]", Simple?"":Field_Name, Prefix_Idx, (int)Idx);\
	MBCSPRINTF_S(szTemp3, TEMP3_SIZE, "%" PRId64 "", (int64_t)Field_Value);\
	NAV_FIELD_PROP_WITH_ALIAS(Field_Name, szTagName, Field_Bits, szTemp3, Field_Desc, bit_offset?*bit_offset:-1LL, "I");\

#define NAV_ARRAY_FIELD_PROP_2NUMBER64_(Field_Name, Prefix_Idx, Idx, Simple, Field_Bits, Field_Value, Field_Desc)\
	MBCSPRINTF_S(szTagName, TAGNAME_SIZE, "%s[%s%d]", Simple?"":Field_Name, Prefix_Idx, (int)Idx);\
	MBCSPRINTF_S(szTemp3, TEMP3_SIZE, "%llu(0X%llX)", Field_Value, Field_Value);\
	NAV_FIELD_PROP_WITH_ALIAS(Field_Name, szTagName, Field_Bits, szTemp3, Field_Desc, bit_offset?*bit_offset:-1LL, "I");\

#define NAV_ARRAY_ARRAY_FIELD_PROP_NUMBER(Field_Name, Sub_Field_Name, Idx, Field_Bits, Field_Value, Field_Desc)\
	MBCSPRINTF_S(szTagName, TAGNAME_SIZE, "%s[%s[%d]]", Field_Name, Sub_Field_Name, (int)Idx);\
	MBCSPRINTF_S(szTemp3, TEMP3_SIZE, "%lu", (unsigned long)(Field_Value));\
	NAV_FIELD_PROP_WITH_ALIAS(Field_Name, szTagName, Field_Bits, szTemp3, Field_Desc, bit_offset?*bit_offset:-1LL, "I");\

#define NAV_2ARRAY_FIELD_PROP_NUMBER_(Field_Name, Prefix_Idx1, Idx1, Prefix_Idx2, Idx2, Field_Bits, Field_Value, Field_Desc)\
	MBCSPRINTF_S(szTagName, TAGNAME_SIZE, "%s[%s%d][%s%d]", Field_Name, Prefix_Idx1, (int)Idx1, Prefix_Idx2, (int)Idx2);\
	MBCSPRINTF_S(szTemp3, TEMP3_SIZE, "%lu", (unsigned long)(Field_Value));\
	NAV_FIELD_PROP_WITH_ALIAS(Field_Name, szTagName, Field_Bits, szTemp3, Field_Desc, bit_offset?*bit_offset:-1LL, "I");\

#define NAV_2ARRAY_FIELD_PROP_2NUMBER_(Field_Name, Prefix_Idx1, Idx1, Prefix_Idx2, Idx2, Field_Bits, Field_Value, Field_Desc)\
	MBCSPRINTF_S(szTagName, TAGNAME_SIZE, "%s[%s%d][%s%d]", Field_Name, Prefix_Idx1, (int)Idx1, Prefix_Idx2, (int)Idx2);\
	MBCSPRINTF_S(szTemp3, TEMP3_SIZE, "%lu(0X%lX)", (unsigned long)(Field_Value), (unsigned long)(Field_Value));\
	NAV_FIELD_PROP_WITH_ALIAS(Field_Name, szTagName, Field_Bits, szTemp3, Field_Desc, bit_offset?*bit_offset:-1LL, "I");\

#define NAV_2ARRAY_FIELD_PROP_NUMBER(Field_Name, Idx1, Idx2, Field_Bits, Field_Value, Field_Desc)\
	NAV_2ARRAY_FIELD_PROP_NUMBER_(Field_Name, "", Idx1, "", Idx2, Field_Bits, Field_Value, Field_Desc)

#define NAV_3ARRAY_FIELD_PROP_NUMBER(Field_Name, Idx1, Idx2, Idx3, Field_Bits, Field_Value, Field_Desc)\
	MBCSPRINTF_S(szTagName, TAGNAME_SIZE, "%s[%d][%d][%d]", Field_Name, (int)Idx1, (int)Idx2, (int)Idx3);\
	MBCSPRINTF_S(szTemp3, TEMP3_SIZE, "%lu", (unsigned long)(Field_Value));\
	NAV_FIELD_PROP_WITH_ALIAS(Field_Name, szTagName, Field_Bits, szTemp3, Field_Desc, bit_offset?*bit_offset:-1LL, "I");\

#define NAV_3ARRAY_FIELD_PROP_SIGN_NUMBER(Field_Name, Idx1, Idx2, Idx3, Field_Bits, Field_Value, Field_Desc)\
	MBCSPRINTF_S(szTagName, TAGNAME_SIZE, "%s[%d][%d][%d]", Field_Name, (int)Idx1, (int)Idx2, (int)Idx3);\
	MBCSPRINTF_S(szTemp3, TEMP3_SIZE, "%ld", (long)(Field_Value));\
	NAV_FIELD_PROP_WITH_ALIAS(Field_Name, szTagName, Field_Bits, szTemp3, Field_Desc, bit_offset?*bit_offset:-1LL, "I");\

#define BST_ARRAY_FIELD_PROP_NUMBER_(Field_Name, Prefix_Idx, Idx, Field_Bits, Field_Value, Field_Desc)\
	if (map_status.status == 0 || (map_status.error == 0 &&  map_status.number_of_fields > 0 && field_prop_idx < map_status.number_of_fields) ){\
		NAV_ARRAY_FIELD_PROP_NUMBER_(Field_Name, Prefix_Idx, Idx, Field_Bits, Field_Value, Field_Desc)\
		field_prop_idx++;}\

#define BST_ARRAY_FIELD_PROP_NUMBER__(Field_Name, Prefix_array, Prefix_Idx, Idx, Field_Bits, Field_Value, Field_Desc)\
	if (map_status.status == 0 || (map_status.error == 0 &&  map_status.number_of_fields > 0 && field_prop_idx < map_status.number_of_fields) ){\
		NAV_ARRAY_FIELD_PROP_NUMBER__(Field_Name, Prefix_array, Prefix_Idx, Idx, Field_Bits, Field_Value, Field_Desc)\
		field_prop_idx++;}\

#define BST_ARRAY_FIELD_PROP_NUMBERS_WITH_ALIAS_VALUEFMT(Field_Name, Prefix_Array, Prefix_Idx, Idx, Field_Bits, Field_Value_FmtStr, Field_Desc, ...)\
	if (map_status.status == 0 || (map_status.error == 0 &&  map_status.number_of_fields > 0 && field_prop_idx < map_status.number_of_fields) ){\
		MBCSPRINTF_S(szTagName, TAGNAME_SIZE, "%s[%s%d]", Prefix_Array, Prefix_Idx, Idx);\
		NAV_FIELD_PROP_WITH_ALIAS_VALUEFMT(Field_Name, szTagName, Field_Bits, Field_Value_FmtStr, Field_Desc, ##__VA_ARGS__)\
		field_prop_idx++;}\

#define BST_FIELD_PROP_NUMBER_BITARRAY(Field_Name, Field_BitArray, Field_Desc, start_idx)\
	if (Field_BitArray.UpperBound() >= start_idx){\
	if (map_status.status == 0 || (map_status.error == 0 &&  map_status.number_of_fields > 0 && field_prop_idx < map_status.number_of_fields) ){\
		int ccWritten = 0, bitarray_idx = start_idx;\
		int ccWrittenOnce = MBCSPRINTF_S(szTemp2, TEMP2_SIZE, "%d", Field_BitArray[bitarray_idx++]);\
		while(ccWrittenOnce > 0 && bitarray_idx <= Field_BitArray.UpperBound()){\
			ccWritten += ccWrittenOnce;\
			if (TEMP2_SIZE <= ccWritten + 1)break;\
			ccWrittenOnce = MBCSPRINTF_S(szTemp2 + ccWritten, (size_t)TEMP2_SIZE - ccWritten, ",%d", Field_BitArray[bitarray_idx++]);\
		}\
		NAV_FIELD_PROP(Field_Name, Field_BitArray.UpperBound() + 1, szTemp2, Field_Desc, bit_offset?*bit_offset:-1LL, "IA");\
		field_prop_idx++;}}\

#define BST_FIELD_PROP_NUMBER_BITARRAY1(Field_Name, Field_Desc)\
		BST_FIELD_PROP_NUMBER_BITARRAY(#Field_Name, Field_Name, Field_Desc, 0)

#define BST_ARRAY_FIELD_PROP_NUMBER(Field_Name, Idx, Field_Bits, Field_Value, Field_Desc)\
		BST_ARRAY_FIELD_PROP_NUMBER_(Field_Name, "", Idx, Field_Bits, Field_Value, Field_Desc)

#define BST_ARRAY_FIELD_PROP_NUMBER1(Field_Name, Idx, Field_Bits, Field_Desc)\
		BST_ARRAY_FIELD_PROP_NUMBER(#Field_Name, Idx, Field_Bits, Field_Name[Idx], Field_Desc)

#define BST_ARRAY_FIELD_PROP_NUMBER_F(Field_Name, Idx, Field_Bits, Field_Value, Field_Desc, ...)\
	if (map_status.status == 0 || (map_status.error == 0 &&  map_status.number_of_fields > 0 && field_prop_idx < map_status.number_of_fields) ){\
		MBCSPRINTF_S(szTemp2, TEMP2_SIZE, Field_Desc, ##__VA_ARGS__); \
		NAV_ARRAY_FIELD_PROP_NUMBER(Field_Name, Idx, Field_Bits, Field_Value, szTemp2)\
		field_prop_idx++;}\

#define BST_ARRAY_FIELD_PROP_SIGNNUMBER_(Field_Name, Prefix_Idx, Idx, Field_Bits, Field_Value, Field_Desc)\
	if (map_status.status == 0 || (map_status.error == 0 &&  map_status.number_of_fields > 0 && field_prop_idx < map_status.number_of_fields) ){\
		NAV_ARRAY_FIELD_PROP_SIGNNUMBER_(Field_Name, Prefix_Idx, Idx, false, Field_Bits, Field_Value, Field_Desc)\
		field_prop_idx++;}\

#define BST_ARRAY_FIELD_PROP_SIGNNUMBER(Field_Name, Idx, Field_Bits, Field_Value, Field_Desc)\
		BST_ARRAY_FIELD_PROP_SIGNNUMBER_(Field_Name, "", Idx, Field_Bits, Field_Value, Field_Desc)

#define BST_ARRAY_FIELD_PROP_SIGNNUMBER1(Field_Name, Idx, Field_Bits, Field_Desc)\
		BST_ARRAY_FIELD_PROP_SIGNNUMBER(#Field_Name, Idx, Field_Bits, Field_Name[Idx], Field_Desc)

#define BST_ARRAY_FIELD_PROP_SIGNNUMBER_F(Field_Name, Idx, Field_Bits, Field_Value, Field_Desc, ...)\
	if (map_status.status == 0 || (map_status.error == 0 &&  map_status.number_of_fields > 0 && field_prop_idx < map_status.number_of_fields) ){\
		MBCSPRINTF_S(szTemp2, TEMP4_SIZE, Field_Desc, ##__VA_ARGS__); \
		NAV_ARRAY_FIELD_PROP_SIGNNUMBER(Field_Name, Idx, Field_Bits, Field_Value, szTemp2)\
		field_prop_idx++;}\

#define BST_ARRAY_FIELD_PROP_NUMBER1_F(Field_Name, Idx, Field_Bits, Field_Desc, ...)\
	if (map_status.status == 0 || (map_status.error == 0 &&  map_status.number_of_fields > 0 && field_prop_idx < map_status.number_of_fields) ){\
		MBCSPRINTF_S(szTemp2, TEMP4_SIZE, Field_Desc, ##__VA_ARGS__); \
		NAV_ARRAY_FIELD_PROP_NUMBER(#Field_Name, Idx, Field_Bits, Field_Name[Idx], szTemp2)\
		field_prop_idx++;}\

#define BST_ARRAY_FIELD_PROP_OBJECT(Field_Name, Idx, Field_Desc)\
	if (map_status.status == 0 || (map_status.error == 0 &&  map_status.number_of_fields > 0 && field_prop_idx < map_status.number_of_fields) ){\
		NAV_WRITE_TAG_BEGIN_WITH_ALIAS(#Field_Name, #Field_Name "()", Field_Desc);\
		NAV_FIELD_PROP_OBJECT(Field_Name[Idx]);\
		NAV_WRITE_TAG_END(#Field_Name);\
		field_prop_idx++;}\

#define BST_ARRAY_FIELD_PROP_REF(Field_Name, Idx, Field_Desc)\
	if (map_status.status == 0 || (map_status.error == 0 &&  map_status.number_of_fields > 0 && field_prop_idx < map_status.number_of_fields) ){\
		NAV_WRITE_TAG_ARRAY_BEGIN(#Field_Name, Idx, Field_Desc);\
		NAV_FIELD_PROP_REF(Field_Name[Idx]);\
		NAV_WRITE_TAG_END(#Field_Name);\
		field_prop_idx++;}\

#define BST_ARRAY_ARRAY_FIELD_PROP_NUMBER(Field_Name, Sub_Field_Name, Idx, Field_Bits, Field_Value, Field_Desc)\
	if (map_status.status == 0 || (map_status.error == 0 &&  map_status.number_of_fields > 0 && field_prop_idx < map_status.number_of_fields) ){\
		NAV_ARRAY_ARRAY_FIELD_PROP_NUMBER(Field_Name, Sub_Field_Name, Idx, Field_Bits, Field_Value, Field_Desc)\
		field_prop_idx++;}\

#define BST_ARRAY_FIELD_PROP_SE(Field_Name, Idx, Field_Desc)\
	BST_ARRAY_FIELD_PROP_SIGNNUMBER(#Field_Name, Idx, \
									(long long)quick_log2((Field_Name[Idx]>=0?Field_Name[Idx]:((-Field_Name[Idx]) + 1)) + 1)*2 + 1, \
									Field_Name[Idx], Field_Desc)

#define BST_ARRAY_FIELD_PROP_UE(Field_Name, Idx, Field_Desc)\
	BST_ARRAY_FIELD_PROP_NUMBER(#Field_Name, Idx, (long long)quick_log2(Field_Name[Idx] + 1)*2 + 1, Field_Name[Idx], Field_Desc)

#define BST_ARRAY_FIELD_PROP_SE1(Object, Field_Name, Idx, Field_Desc)\
	BST_ARRAY_FIELD_PROP_SIGNNUMBER(#Field_Name, Idx, (long long)quick_log2((Object[Idx].Field_Name>=0?Object[Idx].Field_Name:((-Object[Idx].Field_Name) + 1)) + 1)*2 + 1, Object[Idx].Field_Name, Field_Desc)

#define BST_ARRAY_FIELD_PROP_UE1(Object, Field_Name, Idx, Field_Desc)\
	BST_ARRAY_FIELD_PROP_NUMBER(#Field_Name, Idx, (long long)quick_log2(Object[Idx].Field_Name + 1)*2 + 1, Object[Idx].Field_Name, Field_Desc)

#define BST_ARRAY_FIELD_PROP_UE2(Field_Name, Idx, Field_Value, Field_Desc)\
	BST_ARRAY_FIELD_PROP_NUMBER(#Field_Name, Idx, (long long)quick_log2(Field_Value + 1)*2 + 1, Field_Value, Field_Desc)

#define BST_ARRAY_ARRAY_FIELD_PROP_SE(Field_Name, Sub_Field_Name, Idx, Field_Value, Field_Desc)\
	BST_ARRAY_ARRAY_FIELD_PROP_NUMBER(#Field_Name, #Sub_Field_Name, Idx, \
	(long long)quick_log2((Field_Value>=0?Field_Value:((-Field_Value) + 1)) + 1)*2 + 1, Field_Value, Field_Desc)

#define BST_ARRAY_ARRAY_FIELD_PROP_UE(Field_Name, Sub_Field_Name, Idx, Field_Value, Field_Desc)\
	BST_ARRAY_ARRAY_FIELD_PROP_NUMBER(#Field_Name, #Sub_Field_Name, Idx, \
	(long long)quick_log2(Field_Value + 1)*2 + 1, Field_Value, Field_Desc)

#define BST_2ARRAY_FIELD_PROP_NUMBER_(Field_Name, Prefix_Idx1, Idx1, Prefix_Idx2, Idx2, Field_Bits, Field_Value, Field_Desc)\
	if (map_status.status == 0 || (map_status.error == 0 &&  map_status.number_of_fields > 0 && field_prop_idx < map_status.number_of_fields) ){\
		NAV_2ARRAY_FIELD_PROP_NUMBER_(Field_Name, Prefix_Idx1, Idx1, Prefix_Idx2, Idx2, Field_Bits, Field_Value, Field_Desc)\
		field_prop_idx++;}\

#define BST_2ARRAY_FIELD_PROP_2NUMBER_(Field_Name, Prefix_Idx1, Idx1, Prefix_Idx2, Idx2, Field_Bits, Field_Value, Field_Desc)\
	if (map_status.status == 0 || (map_status.error == 0 &&  map_status.number_of_fields > 0 && field_prop_idx < map_status.number_of_fields) ){\
		NAV_2ARRAY_FIELD_PROP_2NUMBER_(Field_Name, Prefix_Idx1, Idx1, Prefix_Idx2, Idx2, Field_Bits, Field_Value, Field_Desc)\
		field_prop_idx++;}\

#define BST_FIELD_PROP_2NUMBER_ALIAS_F_(Field_Name, Field_Alias, Field_Bits, Field_Value, Field_Desc, ...)\
	if (map_status.status == 0 || (map_status.error == 0 &&  map_status.number_of_fields > 0 && field_prop_idx < map_status.number_of_fields) ){\
		MBCSPRINTF_S(szTagName, TAGNAME_SIZE, Field_Alias, ##__VA_ARGS__);\
		MBCSPRINTF_S(szTemp3, TEMP3_SIZE, "%lu(0X%lX)", (unsigned long)(Field_Value), (unsigned long)(Field_Value));\
		NAV_FIELD_PROP_WITH_ALIAS(Field_Name, szTagName, Field_Bits, szTemp3, Field_Desc, bit_offset?*bit_offset:-1LL, "I");\
		field_prop_idx++;}\

#define BST_2ARRAY_FIELD_PROP_NUMBER(Field_Name, Idx1, Idx2, Field_Bits, Field_Value, Field_Desc)\
	if (map_status.status == 0 || (map_status.error == 0 &&  map_status.number_of_fields > 0 && field_prop_idx < map_status.number_of_fields) ){\
		NAV_2ARRAY_FIELD_PROP_NUMBER(Field_Name, Idx1, Idx2, Field_Bits, Field_Value, Field_Desc)\
		field_prop_idx++;}\

#define BST_2ARRAY_FIELD_PROP_UE(Field_Name, Idx1, Idx2, Field_Desc)\
	if (map_status.status == 0 || (map_status.error == 0 &&  map_status.number_of_fields > 0 && field_prop_idx < map_status.number_of_fields) ){\
		NAV_2ARRAY_FIELD_PROP_NUMBER(#Field_Name, Idx1, Idx2, (long long)quick_log2(Field_Name[Idx1][Idx2] + 1)*2 + 1, Field_Name[Idx1][Idx2], Field_Desc)\
		field_prop_idx++;}\

#define BST_3ARRAY_FIELD_PROP_NUMBER(Field_Name, Idx1, Idx2, Idx3, Field_Bits, Field_Value, Field_Desc)\
	if (map_status.status == 0 || (map_status.error == 0 &&  map_status.number_of_fields > 0 && field_prop_idx < map_status.number_of_fields) ){\
		NAV_3ARRAY_FIELD_PROP_NUMBER(Field_Name, Idx1, Idx2, Idx3, Field_Bits, Field_Value, Field_Desc)\
		field_prop_idx++;}\

#define BST_3ARRAY_FIELD_UE(Field_Name, Idx1, Idx2, Idx3, Field_Desc)\
	if (map_status.status == 0 || (map_status.error == 0 &&  map_status.number_of_fields > 0 && field_prop_idx < map_status.number_of_fields) ){\
		NAV_3ARRAY_FIELD_PROP_NUMBER(#Field_Name, Idx1, Idx2, Idx3, (long long)quick_log2(Field_Name[Idx1][Idx2][Idx3] + 1)*2 + 1, Field_Name[Idx1][Idx2][Idx3], Field_Desc)\
		field_prop_idx++;}\

#define BST_3ARRAY_FIELD_UE1(Object, Field_Name, Idx1, Idx2, Idx3, Field_Desc)\
	if (map_status.status == 0 || (map_status.error == 0 &&  map_status.number_of_fields > 0 && field_prop_idx < map_status.number_of_fields) ){\
		NAV_3ARRAY_FIELD_PROP_NUMBER(#Field_Name, Idx1, Idx2, Idx3, (long long)quick_log2(Object[Idx1][Idx2][Idx3].Field_Name + 1)*2 + 1, Object[Idx1][Idx2][Idx3].Field_Name, Field_Desc)\
		field_prop_idx++;}\

#define BST_3ARRAY_FIELD_BOOL1(Object, Field_Name, Idx1, Idx2, Idx3, TRUE_Desc, FALSE_Desc)\
	if (map_status.status == 0 || (map_status.error == 0 &&  map_status.number_of_fields > 0 && field_prop_idx < map_status.number_of_fields) ){\
		NAV_3ARRAY_FIELD_PROP_NUMBER(#Field_Name, Idx1, Idx2, Idx3, 1, Object[Idx1][Idx2][Idx3].Field_Name, Object[Idx1][Idx2][Idx3].Field_Name?TRUE_Desc:FALSE_Desc)\
		field_prop_idx++;}\

#define BST_3ARRAY_FIELD_PROP_SIGN_NUMBER(Field_Name, Idx1, Idx2, Idx3, Field_Bits, Field_Value, Field_Desc)\
	if (map_status.status == 0 || (map_status.error == 0 &&  map_status.number_of_fields > 0 && field_prop_idx < map_status.number_of_fields) ){\
		NAV_3ARRAY_FIELD_PROP_SIGN_NUMBER(Field_Name, Idx1, Idx2, Idx3, Field_Bits, Field_Value, Field_Desc)\
		field_prop_idx++;}\

#define NAV_FIELD_PROP_2NUMBER_DESC_F(Field_Name, Field_Bits, Field_Value, Field_Desc, ...)\
	MBCSPRINTF_S(szTemp2, TEMP2_SIZE, Field_Desc, ##__VA_ARGS__);\
	MBCSPRINTF_S(szTemp3, TEMP3_SIZE, "%lu(0X%lX)", (unsigned long)(Field_Value), (unsigned long)(Field_Value));\
	NAV_FIELD_PROP(Field_Name, Field_Bits, szTemp3, szTemp2, bit_offset?*bit_offset:-1LL, "I");\
	if (bit_offset)*bit_offset += Field_Bits;\

#define NAV_FIELD_PROP_NUMBER64_DESC_F(Field_Name, Field_Bits, Field_Value, Field_Desc, ...)\
	MBCSPRINTF_S(szTemp2, TEMP2_SIZE, Field_Desc, ##__VA_ARGS__);\
	MBCSPRINTF_S(szTemp3, TEMP3_SIZE, "%lld", (long long)(Field_Value));\
	NAV_FIELD_PROP(Field_Name, Field_Bits, szTemp3, szTemp2, bit_offset?*bit_offset:-1LL, "I");\
	if (bit_offset)*bit_offset += Field_Bits;\

#define NAV_FIELD_PROP_FOURCC(Field_Name, Field_Value, Field_Desc, ...)\
	MBCSPRINTF_S(szTemp3, TEMP3_SIZE, "%c%c%c%c",\
		isprint((Field_Value >> 24) & 0xFF) ? ((Field_Value >> 24) & 0xFF) : '.',\
		isprint((Field_Value >> 16) & 0xFF) ? ((Field_Value >> 16) & 0xFF) : '.',\
		isprint((Field_Value >> 8) & 0xFF) ? ((Field_Value >> 8) & 0xFF) : '.',\
		isprint((Field_Value >> 0) & 0xFF) ? ((Field_Value >> 0) & 0xFF) : '.');\
	NAV_FIELD_PROP(Field_Name, 32, szTemp3, Field_Desc, bit_offset?*bit_offset:-1LL, "I");\
	if (bit_offset)*bit_offset += 32;\

#define NAV_FIELD_PROP_FOURCC_EX(Field_Name, Field_Value, Field_Desc, ...)\
	MBCSPRINTF_S(szTemp3, TEMP3_SIZE, "%c%c%c%c(0X%08X)",\
		isprint((Field_Value >> 24) & 0xFF) ? ((Field_Value >> 24) & 0xFF) : '.',\
		isprint((Field_Value >> 16) & 0xFF) ? ((Field_Value >> 16) & 0xFF) : '.',\
		isprint((Field_Value >> 8) & 0xFF) ? ((Field_Value >> 8) & 0xFF) : '.',\
		isprint((Field_Value >> 0) & 0xFF) ? ((Field_Value >> 0) & 0xFF) : '.', Field_Value);\
	NAV_FIELD_PROP(Field_Name, 32, szTemp3, Field_Desc, bit_offset?*bit_offset:-1LL, "I");\
	if (bit_offset)*bit_offset += 32;\

#define NAV_ARRAY_FIELD_PROP_FOURCC(Field_Name, Idx, Simple, Field_Value, Field_Desc)\
	MBCSPRINTF_S(szTagName, TAGNAME_SIZE, "%s[%d]", Simple?"":Field_Name, (int)Idx);\
	MBCSPRINTF_S(szTemp3, TEMP3_SIZE, "%c%c%c%c",\
		isprint((Field_Value >> 24) & 0xFF) ? ((Field_Value >> 24) & 0xFF) : '.',\
		isprint((Field_Value >> 16) & 0xFF) ? ((Field_Value >> 16) & 0xFF) : '.',\
		isprint((Field_Value >> 8) & 0xFF) ? ((Field_Value >> 8) & 0xFF) : '.',\
		isprint((Field_Value >> 0) & 0xFF) ? ((Field_Value >> 0) & 0xFF) : '.');\
	NAV_FIELD_PROP_WITH_ALIAS(Field_Name, szTagName, 32, szTemp3, Field_Desc, bit_offset?*bit_offset:-1LL, "I");\

#define NAV_FIELD_PROP_FOURCC1(Field_Name, Field_Desc)	NAV_FIELD_PROP_FOURCC(#Field_Name, Field_Name, Field_Desc)
#define NAV_FIELD_PROP_FOURCC_EX1(Field_Name, Field_Desc)	NAV_FIELD_PROP_FOURCC_EX(#Field_Name, Field_Name, Field_Desc)
#define NAV_ARRAY_FIELD_PROP_FOURCC1(Field_Name, i, Simple, Field_Desc)	NAV_ARRAY_FIELD_PROP_FOURCC(#Field_Name, i, Simple, Field_Name[i], Field_Desc)

#define NAV_FIELD_PROP_NUMBER_ALIAS_F(Field_Name, Field_Alias, Field_Bits, Field_Format, Field_Value, Field_Desc, ...)\
	MBCSPRINTF_S(szTagName, TAGNAME_SIZE, Field_Alias, ##__VA_ARGS__); \
	MBCSPRINTF_S(szTemp3, TEMP3_SIZE, Field_Format, Field_Value);\
	NAV_FIELD_PROP_WITH_ALIAS(Field_Name, szTagName, Field_Bits, szTemp3, Field_Desc, bit_offset?*bit_offset:-1LL, "I");\

#define NAV_FIELD_PROP_2NUMBER_ALIAS_F(Field_Name, Field_Alias, Field_Bits, Field_Format, Field_Value, Field_Desc, ...)\
	MBCSPRINTF_S(szTagName, TAGNAME_SIZE, Field_Alias, ##__VA_ARGS__); \
	MBCSPRINTF_S(szTemp3, TEMP3_SIZE, Field_Format, Field_Value, Field_Value);\
	NAV_FIELD_PROP_WITH_ALIAS(Field_Name, szTagName, Field_Bits, szTemp3, Field_Desc, bit_offset?*bit_offset:-1LL, "I");\

#define NAV_FIELD_PROP_2NUMBER(Field_Name, Field_Bits, Field_Value, Field_Desc)\
	MBCSPRINTF_S(szTemp3, TEMP3_SIZE, "%" PRIu32 "(0X%" PRIX32 ")", (uint32_t)(Field_Value), (uint32_t)(Field_Value));\
	NAV_FIELD_PROP(Field_Name, Field_Bits, szTemp3, Field_Desc, bit_offset?*bit_offset:-1LL, "I");\
	if (bit_offset)*bit_offset += Field_Bits;\

#define NAV_FIELD_PROP_2LLNUMBER(Field_Name, Field_Bits, Field_Value, Field_Desc)\
	MBCSPRINTF_S(szTemp3, TEMP3_SIZE, "%" PRId64 "(0X%" PRIX64 ")", (int64_t)(Field_Value), (int64_t)(Field_Value));\
	NAV_FIELD_PROP(Field_Name, Field_Bits, szTemp3, Field_Desc, bit_offset?*bit_offset:-1LL, "I");\
	if (bit_offset)*bit_offset += Field_Bits;\

#define NAV_FIELD_PROP_2NUMBER_WITH_ALIAS(Field_Alias, Field_Bits, Field_Value, Field_Desc)\
	MBCSPRINTF_S(szTemp3, TEMP3_SIZE, "%" PRIu32 "(0X%" PRIX32 ")", (uint32_t)(Field_Value), (uint32_t)(Field_Value));\
	NAV_FIELD_PROP_WITH_ALIAS("Field", Field_Alias, Field_Bits, szTemp3, Field_Desc, bit_offset?*bit_offset:-1LL, "I");\

#define NAV_FIELD_PROP_2NUMBER1(Field_Name, Field_Bits, Field_Desc) \
	NAV_FIELD_PROP_2NUMBER(#Field_Name, Field_Bits, Field_Name, Field_Desc)

#define NAV_FIELD_PROP_2NUMBER1_DESC_F(Field_Name, Field_Bits, Field_Desc, ...)	\
	NAV_FIELD_PROP_2NUMBER_DESC_F(#Field_Name, Field_Bits, Field_Name, Field_Desc, ##__VA_ARGS__)

#define BST_FIELD_PROP_2NUMBER(Field_Name, Field_Bits, Field_Value, Field_Desc)\
	if (map_status.status == 0 || (map_status.error == 0 &&  map_status.number_of_fields > 0 && field_prop_idx < map_status.number_of_fields) ){\
		NAV_FIELD_PROP_2NUMBER(Field_Name, Field_Bits, Field_Value, Field_Desc)\
		field_prop_idx++;}\

#define BST_FIELD_PROP_2NUMBER_WITH_ALIAS(Field_Alias, Field_Bits, Field_Value, Field_Desc)\
	if (map_status.status == 0 || (map_status.error == 0 &&  map_status.number_of_fields > 0 && field_prop_idx < map_status.number_of_fields) ){\
		NAV_FIELD_PROP_2NUMBER_WITH_ALIAS(Field_Alias, Field_Bits, Field_Value, Field_Desc)\
		field_prop_idx++;}\

#define BST_FIELD_PROP_2NUMBER1(Field_Name, Field_Bits, Field_Desc)		BST_FIELD_PROP_2NUMBER(#Field_Name, Field_Bits, Field_Name, Field_Desc)
#define BST_FIELD_PROP_2NUMBER1_F(Field_Name, Field_Bits, Field_Desc, ...)	\
	{MBCSPRINTF_S(szTemp2, TEMP2_SIZE, Field_Desc, ##__VA_ARGS__); \
	BST_FIELD_PROP_2NUMBER1(Field_Name, Field_Bits, szTemp2);}\

#define BST_FIELD_PROP_2LLNUMBER(Field_Name, Field_Bits, Field_Value, Field_Desc)\
	if (map_status.status == 0 || (map_status.error == 0 &&  map_status.number_of_fields > 0 && field_prop_idx < map_status.number_of_fields) ){\
		NAV_FIELD_PROP_2LLNUMBER(Field_Name, Field_Bits, Field_Value, Field_Desc)\
		field_prop_idx++;}\

#define BST_FIELD_PROP_UE(Field_Name, Field_Desc)						BST_FIELD_PROP_2NUMBER(#Field_Name, (long long)quick_log2(Field_Name + 1)*2 + 1, Field_Name, Field_Desc)
#define BST_FIELD_PROP_UE_F(Field_Name, Field_Desc, ...)				BST_FIELD_PROP_2NUMBER1_F(Field_Name, (long long)quick_log2(Field_Name + 1)*2 + 1, Field_Desc, ##__VA_ARGS__)
#define BST_FIELD_PROP_UE1(Field_Name, AliasName, Field_Desc)			BST_FIELD_PROP_2NUMBER_ALIAS_F_(#Field_Name, "%s", (long long)quick_log2(Field_Name + 1)*2 + 1, Field_Name, Field_Desc, AliasName)
#define BST_FIELD_PROP_SE(Field_Name, Field_Desc)						BST_FIELD_PROP_SIGNNUMBER(#Field_Name, (long long)quick_log2((Field_Name>=0?Field_Name:((-Field_Name) + 1)) + 1)*2 + 1, Field_Name, Field_Desc)
#define BST_FIELD_PROP_UVLC(Field_Name, Field_Desc)						BST_FIELD_PROP_2NUMBER(#Field_Name, (long long)quick_log2(Field_Name + 1)*2 + 1, Field_Name, Field_Desc)

#define NAV_FIELD_PROP_NUMBER_PRINTF(Field_Name, Field_Bits, Field_Desc, Field_Value_Format, ...)	\
	MBCSPRINTF_S(szTemp4, TEMP4_SIZE, Field_Value_Format, ##__VA_ARGS__);\
	NAV_FIELD_PROP(Field_Name, Field_Bits, szTemp4, Field_Desc, bit_offset?*bit_offset:-1LL, "I");\
	if (bit_offset)*bit_offset += Field_Bits;\

#define NAV_FIELD_PROP_NUMBER64(Field_Name, Field_Bits, Field_Value, Field_Desc)\
	MBCSPRINTF_S(szTemp3, TEMP3_SIZE, "%lld", (long long)(Field_Value));\
	NAV_FIELD_PROP(Field_Name, Field_Bits, szTemp3, Field_Desc, bit_offset?*bit_offset:-1LL, "I");\
	if (bit_offset)*bit_offset += Field_Bits;\

#define NAV_FIELD_PROP_NUMBER64_1(Field_Name, Field_Bits, Field_Desc)	NAV_FIELD_PROP_NUMBER64(#Field_Name, Field_Bits, Field_Name, Field_Desc)

#define NAV_FIELD_PROP_FIXSIZE_STR(Field_Name, Field_Bits, Field_Value, Value_Size, Field_Desc)\
	memset(szTemp2, 0, sizeof(szTemp2));\
	MEMCPY(szTemp2, sizeof(szTemp2), Field_Value, Value_Size);\
	NAV_FIELD_PROP(Field_Name, Field_Bits, szTemp2, Field_Desc, bit_offset?*bit_offset:-1LL, "S");\
	if (bit_offset)*bit_offset += Field_Bits;\

#define NAV_FIELD_PROP_FIXSIZE_STR_WITH_ALIAS(Field_Name, Field_Alias, Field_Bits, Field_Value, Value_Size, Field_Desc)\
	memset(szTemp2, 0, sizeof(szTemp2));\
	MEMCPY(szTemp2, sizeof(szTemp2), Field_Value, Value_Size);\
	NAV_FIELD_PROP_WITH_ALIAS(Field_Name, Field_Alias, Field_Bits, szTemp2, Field_Desc, bit_offset?*bit_offset:-1LL, "S");\

#define NAV_FIELD_PROP_FIXSIZE_BINSTR(Field_Name, Field_Bits, Field_Value, Value_Size, Field_Desc)\
	if (Field_Bits > 0 && Value_Size > 0)\
	{\
		unsigned long jjj=0;\
		auto ptr_field_value = Field_Value;\
		memset(szTemp2, 0, sizeof(szTemp2));\
		for(jjj=0;jjj<((unsigned long long)(Value_Size)>255UL?255UL:(unsigned long long)(Value_Size));jjj++)\
		{\
			MBCSPRINTF_S(szTemp3, TEMP3_SIZE, "%02x ", ptr_field_value[jjj]);\
			MBCSCAT(szTemp2, TEMP2_SIZE, szTemp3);\
		}\
		NAV_FIELD_PROP(Field_Name, Field_Bits, szTemp2, Field_Desc, bit_offset?*bit_offset:-1LL, "B");\
		if (bit_offset)*bit_offset += (long long)Field_Bits;\
	}\

#define NAV_FIELD_PROP_FIXSIZE_BINSTR1(Field_Name, Field_Bits, Field_Desc)	NAV_FIELD_PROP_FIXSIZE_BINSTR(#Field_Name, Field_Bits, Field_Name, (Field_Bits>>3), Field_Desc)

#define NAV_FIELD_PROP_1D_ARRAY_MATRIX(Field_Name, Field_Value, M, N, Field_Desc)\
	if (M > 0 && N > 0){\
		unsigned long iii=0, jjj=0;\
		auto ptr_field_value = Field_Value;\
		memset(szTemp2, 0, sizeof(szTemp2));\
		for(iii=0;iii<M;iii++){\
			for(jjj=0;jjj<N;jjj++){\
				MBCSPRINTF_S(szTemp3, TEMP3_SIZE, "%3d ", ptr_field_value[iii*N + jjj]);\
				MBCSCAT(szTemp2, TEMP2_SIZE, szTemp3);}\
			MBCSCAT(szTemp2, TEMP2_SIZE, "\n");}\
		NAV_FIELD_PROP(Field_Name, ((long long)M*N*8), szTemp2, Field_Desc, bit_offset?*bit_offset:-1LL, "M");\
		if (bit_offset)*bit_offset += (long long)M*N*8;\
	}\

#define NAV_FIELD_PROP_1D_ARRAY_MATRIX1(Field_Name, M, N, Field_Desc)	NAV_FIELD_PROP_1D_ARRAY_MATRIX(#Field_Name, Field_Name, M, N, Field_Desc)

#define BST_FIELD_PROP_FIXSIZE_BINSTR(Field_Name, Field_Bits, Field_Value, Value_Size, Field_Desc)\
	if (map_status.status == 0 || (map_status.error == 0 && map_status.number_of_fields > 0 && field_prop_idx < map_status.number_of_fields) ){\
		NAV_FIELD_PROP_FIXSIZE_BINSTR(Field_Name, Field_Bits, Field_Value, Value_Size, Field_Desc)\
		field_prop_idx++;}\

#define BST_FIELD_PROP_1D_ARRAY_MATRIX(Field_Name, Field_Value, M, N, Field_Desc)	\
	if (map_status.status == 0 || (map_status.error == 0 && map_status.number_of_fields > 0 && field_prop_idx < map_status.number_of_fields) ){\
		NAV_FIELD_PROP_1D_ARRAY_MATRIX(Field_Name, Field_Value, M, N, Field_Desc)\
		field_prop_idx++;}\

#define BST_FIELD_PROP_UUID(Field_Name, Field_Value, Field_Desc) \
	if (map_status.status == 0 || (map_status.error == 0 && map_status.number_of_fields > 0 && field_prop_idx < map_status.number_of_fields) ){\
		auto uuid = Field_Value;\
		MBCSPRINTF_S(szTemp2, TEMP2_SIZE, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",\
			uuid[0x00], uuid[0x01], uuid[0x02], uuid[0x03],uuid[0x04], uuid[0x05], uuid[0x06], uuid[0x07],\
			uuid[0x08], uuid[0x09], uuid[0x0a], uuid[0x0b],uuid[0x0c], uuid[0x0d], uuid[0x0e], uuid[0x0f]);\
		NAV_FIELD_PROP(Field_Name, 128, szTemp2, Field_Desc, bit_offset?*bit_offset:-1LL, "S");\
		if (bit_offset)*bit_offset += 128LL;\
		field_prop_idx++;}\

#define BST_FIELD_PROP_UUID1(Field_Name, Field_Desc)	BST_FIELD_PROP_UUID(#Field_Name, Field_Name, Field_Desc)

#define BST_FIELD_PROP_FIXSIZE_BINSTR1(Field_Name, Field_Bits, Field_Desc)	BST_FIELD_PROP_FIXSIZE_BINSTR(#Field_Name, Field_Bits, Field_Name, (Field_Bits>>3), Field_Desc)

#define BST_FIELD_PROP_1D_ARRAY_MATROX1(Field_Name, M, N, Field_Desc) BST_FIELD_PROP_1D_ARRAY_MATRIX(#Field_Name, Field_Name, M, N, Field_Desc)

#define NAV_FIELD_PROP_FIXSIZE_BINCHARSTR(Field_Name, Field_Bits, Field_Value, Value_Size, Field_Desc)\
	if (Field_Bits >0 && Value_Size > 0)\
	{\
		unsigned long jjj=0;\
		memset(szTemp2, 0, sizeof(szTemp2));\
		for(jjj=0;jjj<((unsigned long long)(Value_Size)>255?255:(unsigned long long)(Value_Size));jjj++)\
		{\
			MBCSPRINTF_S(szTemp3, TEMP3_SIZE, "%02x(%c) ", Field_Value[jjj], isprint(Field_Value[jjj])?Field_Value[jjj]:0x20);\
			MBCSCAT(szTemp2, TEMP2_SIZE, szTemp3);\
		}\
		NAV_FIELD_PROP(Field_Name, Field_Bits, szTemp2, Field_Desc, bit_offset?*bit_offset:-1LL, "B");\
		if (bit_offset)*bit_offset += Field_Bits;\
	}\

#define NAV_FIELD_PROP_FIXSIZE_BINCHARSTR1(Field_Name, Field_Bits, Field_Desc)	NAV_FIELD_PROP_FIXSIZE_BINCHARSTR(#Field_Name, Field_Bits, Field_Name, (Field_Bits>>3), "")

#define NAV_FIELD_PROP_OBJECT(Obj_Name)\
	if(szOutXml != NULL && cbLen > 0){\
		pTemp = szOutXml + cbRequired;\
		cbRequired += Obj_Name.ProduceDesc(pTemp, cbLen - cbRequired, bPrint, bit_offset);\
	}else{\
		cbRequired += Obj_Name.ProduceDesc(NULL, 0, bPrint, bit_offset);\
	}\

#define BST_FIELD_PROP_OBJECT(Obj_Name)\
	if (map_status.status == 0 || (map_status.error == 0 &&  map_status.number_of_fields > 0 && field_prop_idx < map_status.number_of_fields) ){\
		NAV_FIELD_PROP_OBJECT(Obj_Name)\
		field_prop_idx++;}\

#define NAV_FIELD_PROP_REF(Obj_Name)\
	if (Obj_Name){\
		if(szOutXml != NULL && cbLen > 0 && Obj_Name){\
			pTemp = szOutXml + cbRequired;\
			cbRequired += Obj_Name->ProduceDesc(pTemp, cbLen - cbRequired, bPrint, bit_offset);\
		}else if(Obj_Name){\
			cbRequired += Obj_Name->ProduceDesc(NULL, 0, bPrint, bit_offset);\
		}\
	}\

#define BST_FIELD_PROP_REF(Obj_Name)\
	if (map_status.status == 0 || (map_status.error == 0 &&  map_status.number_of_fields > 0 && field_prop_idx < map_status.number_of_fields) ){\
		NAV_FIELD_PROP_REF(Obj_Name)\
		field_prop_idx++;}\

#define NAV_FIELD_PROP_REF4(Obj_Name, TAG_NAME, TAG_DESC)\
	NAV_FIELD_PROP_REF4_1(Obj_Name, TAG_NAME, TAG_DESC, 0)

#define NAV_FIELD_PROP_REF4_1(Obj_Name, TAG_NAME, TAG_DESC, auto_expand)\
	if (Obj_Name){\
	NAV_WRITE_TAG_BEGIN_1(TAG_NAME, TAG_DESC, auto_expand);\
	if(szOutXml != NULL && cbLen > 0 && Obj_Name){\
		pTemp = szOutXml + cbRequired;\
		cbRequired += Obj_Name->ProduceDesc(pTemp, cbLen - cbRequired, bPrint, bit_offset);\
	}else if(Obj_Name){\
		cbRequired += Obj_Name->ProduceDesc(NULL, 0, bPrint, bit_offset);\
	}\
	NAV_WRITE_TAG_END(TAG_NAME);\
	}\
	AMP_NOP1(((void)0))

#define BST_FIELD_PROP_REF4_1(Obj_Name, TAG_NAME, TAG_DESC, auto_expand)\
	if (map_status.status == 0 || (map_status.error == 0 &&  map_status.number_of_fields > 0 && field_prop_idx < map_status.number_of_fields) ){\
	if (Obj_Name){\
	NAV_WRITE_TAG_BEGIN_1(TAG_NAME, TAG_DESC, auto_expand);\
	if(szOutXml != NULL && cbLen > 0 && Obj_Name){\
		pTemp = szOutXml + cbRequired;\
		cbRequired += Obj_Name->ProduceDesc(pTemp, cbLen - cbRequired, bPrint, bit_offset);\
	}else if(Obj_Name){\
		cbRequired += Obj_Name->ProduceDesc(NULL, 0, bPrint, bit_offset);\
	}\
	NAV_WRITE_TAG_END(TAG_NAME);\
	}\
	field_prop_idx++;\
	}\
	AMP_NOP1(((void)0))

#define BST_FIELD_PROP_REF4(Obj_Name, TAG_NAME, TAG_DESC)	BST_FIELD_PROP_REF4_1(Obj_Name, TAG_NAME, TAG_DESC, 0)

#define NAV_FIELD_PROP_REF1(Obj_Name)\
	NAV_WRITE_TAG_BEGIN2(#Obj_Name);\
	NAV_FIELD_PROP_REF(Obj_Name)\
	NAV_WRITE_TAG_END2(#Obj_Name)

#define NAV_FIELD_PROP_REF1_1(Obj_Name, Tag_Desc, auto_expand)\
	NAV_WRITE_TAG_BEGIN_1 (#Obj_Name, Tag_Desc, auto_expand);\
	NAV_FIELD_PROP_REF(Obj_Name)\
	NAV_WRITE_TAG_END(#Obj_Name)

#define BST_FIELD_PROP_REF1(Obj_Name)\
	NAV_WRITE_TAG_BEGIN2(#Obj_Name);\
	BST_FIELD_PROP_REF(Obj_Name)\
	NAV_WRITE_TAG_END2(#Obj_Name)	

#define BST_FIELD_PROP_REF1_1(Obj_Name, TAG_DESC)\
	NAV_WRITE_TAG_BEGIN_WITH_ALIAS(#Obj_Name, #Obj_Name "()", TAG_DESC);\
	BST_FIELD_PROP_REF(Obj_Name)\
	NAV_WRITE_TAG_END(#Obj_Name)

#define NAV_FIELD_PROP_REF2(Obj_Name, TAG_NAME)\
	NAV_WRITE_TAG_BEGIN2(TAG_NAME);\
	NAV_FIELD_PROP_REF(Obj_Name)\
	NAV_WRITE_TAG_END2(TAG_NAME)

#define NAV_FIELD_PROP_REF2_1(Obj_Name, TAG_NAME, TAG_DESC)\
	NAV_WRITE_TAG_BEGIN2_1(TAG_NAME, TAG_DESC);\
	NAV_FIELD_PROP_REF(Obj_Name)\
	NAV_WRITE_TAG_END2(TAG_NAME)

#define NAV_FIELD_PROP_REF2_2(Obj_Name, TAG_NAME, TAG_DESC)\
	NAV_WRITE_TAG_BEGIN_WITH_ALIAS(TAG_NAME, TAG_NAME "()", TAG_DESC);\
	NAV_FIELD_PROP_REF(Obj_Name)\
	NAV_WRITE_TAG_END2(TAG_NAME)

#define NAV_FIELD_PROP_REF2_3(Obj_Name, TAG_NAME, TAG_DESC, auto_expand)\
	NAV_WRITE_TAG_BEGIN_1(TAG_NAME, TAG_DESC, auto_expand);\
	NAV_FIELD_PROP_REF(Obj_Name)\
	NAV_WRITE_TAG_END(TAG_NAME)

#define BST_FIELD_PROP_REF2(Obj_Name, TAG_NAME)\
	NAV_WRITE_TAG_BEGIN2(TAG_NAME);\
	BST_FIELD_PROP_REF(Obj_Name)\
	NAV_WRITE_TAG_END2(TAG_NAME)

#define BST_FIELD_PROP_REF2_1(Obj_Name, TAG_NAME, TAG_DESC)\
	NAV_WRITE_TAG_BEGIN_WITH_ALIAS(TAG_NAME, TAG_NAME "()", TAG_DESC);\
	BST_FIELD_PROP_REF(Obj_Name)\
	NAV_WRITE_TAG_END(TAG_NAME)

#define NAV_FIELD_PROP_REF3(Obj_Name, TAG_NAME, idx)\
	NAV_WRITE_TAG_BEGIN3(TAG_NAME, idx);\
	NAV_FIELD_PROP_REF(Obj_Name)\
	NAV_WRITE_TAG_END3(TAG_NAME, idx)

#define BST_FIELD_PROP_REF3(Obj_Name, TAG_NAME, idx)\
	NAV_WRITE_TAG_BEGIN3(TAG_NAME, idx);\
	BST_FIELD_PROP_REF(Obj_Name)\
	NAV_WRITE_TAG_END3(TAG_NAME, idx)

#define NAV_WRITE_TAG_END(Group_Name)\
	if(szOutXml != 0 && cbLen > 0)\
		cbRequired += MBCSPRINTF_S(szOutXml+cbRequired, cbLen-cbRequired, "</%s>\n", Group_Name);\
	else\
		cbRequired += MBCSPRINTF_S(szTagName, TAGNAME_SIZE, "</%s>\n", Group_Name);\
	if(bPrint)\
		printf("</%s>\n", Group_Name);\
	AMP_NOP1(((void)0))

#define NAV_WRITE_TAG_END2(Group_Name) NAV_WRITE_TAG_END(Group_Name)

#define NAV_WRITE_TAG_END3(Group_Name, idx)	NAV_WRITE_TAG_END(Group_Name)

#define NAV_WRITE_TAG_END4(Group_Name, formatstr, idx)\
	if(szOutXml != 0 && cbLen > 0)\
		cbRequired += MBCSPRINTF_S(szOutXml + cbRequired, cbLen - cbRequired, "</%s_" formatstr ">\n", Group_Name, (unsigned long)(idx));\
	else\
		cbRequired += MBCSPRINTF_S(szTemp4, TEMP4_SIZE, "</%s_" formatstr ">\n", Group_Name, (unsigned long)(idx));\
	if(bPrint)\
		printf("</%s>\n", Group_Name);\

#define NAV_FIELD_PROP_OBJECT_WITH_TAG1(Obj_Name, Group_Name)\
	NAV_WRITE_TAG_BEGIN(Group_Name);\
	NAV_FIELD_PROP_OBJECT(Obj_Name)\
	NAV_WRITE_TAG_END(Group_Name)\

#define NAV_FIELD_PROP_OBJECT_WITH_TAG2(Obj_Name, Group_Name)\
	NAV_WRITE_TAG_BEGIN2(Group_Name);\
	NAV_FIELD_PROP_OBJECT(Obj_Name)\
	NAV_WRITE_TAG_END2(Group_Name)\

#define NAV_FIELD_PROP_OBJECT_WITH_TAG2_1(Obj_Name, TAG_NAME, TAG_DESC)\
	NAV_WRITE_TAG_BEGIN2_1(TAG_NAME, TAG_DESC);\
	NAV_FIELD_PROP_OBJECT(Obj_Name)\
	NAV_WRITE_TAG_END2(TAG_NAME)

#define NAV_FIELD_PROP_OBJECT_WITH_TAG2_2(Obj_Name, TAG_NAME, TAG_DESC, auto_expand)\
	NAV_WRITE_TAG_BEGIN_1(TAG_NAME, TAG_DESC, auto_expand);\
	NAV_FIELD_PROP_OBJECT(Obj_Name)\
	NAV_WRITE_TAG_END(TAG_NAME)

#define NAV_FIELD_PROP_OBJECT_WITH_TAG3(Obj_Name, Group_Name, idx)\
	NAV_WRITE_TAG_BEGIN3(Group_Name, idx);\
	NAV_FIELD_PROP_OBJECT(Obj_Name)\
	NAV_WRITE_TAG_END3(Group_Name, idx)\

#define NAV_FIELD_PROP_OBJECT_WITH_TAG3_1(Obj_Name, Group_Name, idx, TAG_DESC)\
	NAV_WRITE_TAG_BEGIN3_1(Group_Name, idx, TAG_DESC);\
	NAV_FIELD_PROP_OBJECT(Obj_Name)\
	NAV_WRITE_TAG_END3(Group_Name, idx)\

#define NAV_FIELD_PROP_REF_WITH_TAG1(Obj_Name, Group_Name)\
	if (Obj_Name){\
	NAV_WRITE_TAG_BEGIN(Group_Name);\
	NAV_FIELD_PROP_REF(Obj_Name)\
	NAV_WRITE_TAG_END(Group_Name);\
	}\

#define NAV_FIELD_PROP_REF_WITH_TAG2(Obj_Name, Group_Name)\
	if (Obj_Name){\
	NAV_WRITE_TAG_BEGIN2(Group_Name);\
	NAV_FIELD_PROP_REF(Obj_Name)\
	NAV_WRITE_TAG_END2(Group_Name);\
	}\

#define NAV_FIELD_PROP_REF_WITH_TAG2_1(Obj_Name, Group_Name, TAG_DESC)\
	if (Obj_Name){\
	NAV_WRITE_TAG_BEGIN2_1(Group_Name, TAG_DESC);\
	NAV_FIELD_PROP_REF(Obj_Name)\
	NAV_WRITE_TAG_END2(Group_Name);\
	}\

#define NAV_FIELD_PROP_REF_WITH_TAG2_2(Obj_Name, Group_Name, TAG_DESC, auto_expand)\
	if (Obj_Name){\
	NAV_WRITE_TAG_BEGIN_1(Group_Name, TAG_DESC, auto_expand);\
	NAV_FIELD_PROP_REF(Obj_Name)\
	NAV_WRITE_TAG_END(Group_Name);\
	}\

#define NAV_FIELD_PROP_REF_WITH_TAG3(Obj_Name, Group_Name, idx)\
	if (Obj_Name){\
	NAV_WRITE_TAG_BEGIN3(Group_Name, idx);\
	NAV_FIELD_PROP_REF(Obj_Name)\
	NAV_WRITE_TAG_END3(Group_Name, idx);\
	}\

#define NAV_FIELD_PROP_REF_WITH_TAG3_1(Obj_Name, Group_Name, idx, TAG_DESC)\
	if (Obj_Name){\
		NAV_WRITE_TAG_BEGIN3_1(Group_Name, idx, TAG_DESC);\
		NAV_FIELD_PROP_REF(Obj_Name)\
		NAV_WRITE_TAG_END3(Group_Name, idx);\
	}\

#define DECLARE_FIELDPROP_END()\
		return cbRequired;\
	}\

#define UNMAP_DUPLICATE_STRUCT_POINTER(pointer, type)	\
	if(pointer){\
		int cbLeft = 0, nBitsOffset = 0;\
		assert((AMBst_Tell(bs)%8) == 0);\
		AMBst_Flush(bs);\
		uint8_t* pDst = AMBst_LockCurPtr(bs, &nBitsOffset, &cbLeft);\
		assert(nBitsOffset%8 == 0);\
		pDst += nBitsOffset/8;\
		int pointer_content_size = sizeof(type) + pointer->GetVarBodySize();\
		if (pointer_content_size > cbLeft)\
			return RET_CODE_BUFFER_TOO_SMALL;\
		memcpy(pDst, (void*)pointer, pointer_content_size);\
		type* pTmp = (type*)pDst;\
		pTmp->Endian(false);\
		AMBst_UnlockCurPtr(bs, pointer_content_size);}\

#define UNMAP_WRITE_TO_BITSTREAM()	\
	AMBst bs = AMBst_CreateFromBuffer(pBuf, *pcbSize, BST_MODE_WRITE);\
	if (IS_INVALID_HANDLE(bs))\
		return RET_CODE_ERROR;\
	if (WriteToBs(bs) != RET_CODE_SUCCESS){\
		AMBst_Destroy(bs);\
		return RET_CODE_ERROR;\
	}\
	AMBst_Flush(bs);\
	*pcbSize = AMBst_Tell(bs)>>3;\
	AMBst_Destroy(bs);\

#define UNMAP_GENERAL_UTILITY()	\
	if (pBuf == NULL)\
		*pcbSize = cbRequired;\
	else\
	{\
		if (*pcbSize < cbRequired)\
			return RET_CODE_BUFFER_TOO_SMALL;\
		UNMAP_WRITE_TO_BITSTREAM()\
	}\

#define UNMAP_GENERAL_UTILITY_2()	\
	unsigned long cbRequired = 0;\
	Unmap(NULL, &cbRequired);\
	if (*pcbSize < cbRequired)\
		return RET_CODE_BUFFER_TOO_SMALL;\
	UNMAP_WRITE_TO_BITSTREAM()\

#define MAP_BST_INIT()						map_status.status = -1
#define MAP_BST_BEGIN(num)					map_status.error = 0; map_status.broken = 1; map_status.number_of_fields = num
#define MAP_BST_END()						map_status.status = 0
#define MAP_BST_BITS(bst, field, bits)		field = AMBst_GetBits(bst, bits); map_status.number_of_fields++
#define MAP_BST_T_BITS(bst, T, field, bits)	field = (T)AMBst_GetBits(bst, bits); map_status.number_of_fields++
#define MAP_BST_BYTES(bst, field, bytes)	AMBst_GetBytes(bst, field, bytes); map_status.number_of_fields++
#define MAP_BST_BYTE(bst, field)			field = AMBst_GetByte(bst);
#define MAP_BST_WORD(bst, field)			field = AMBst_GetWord(bst);

#define	bst_read_b(bst, field, n, type)		{field = (type)AMBst_GetBits(bst, n); map_status.number_of_fields++;}AMP_NOP1(0)
#define bsrb(bs, field, n)					{field = (decltype(field))(AMBst_GetBits(bs, n)); map_status.number_of_fields++;}AMP_NOP1(0)
#define bsrb1(bs, field, n)					{field = (std::remove_reference<decltype(field)>::type)(AMBst_GetBits(bs, n)); map_status.number_of_fields++;}AMP_NOP1(0)
#define bsrb2(bs, field, n)					{field = (std::remove_reference<decltype(field)>::type)(AMBst_GetBits(bs, n));}AMP_NOP1(0)
#define bsrbarray(bs, bitarray, idx)		{AMBst_GetBits(bs, 1)?bitarray.BitSet(idx):bitarray.BitClear(idx); map_status.number_of_fields++;}AMP_NOP1(0)
#define bsrbarray1(bs, bitarray, idx)		{AMBst_GetBits(bs, 1)?AM_BitSet(bitarray, idx):AM_BitClear(bitarray, idx); map_status.number_of_fields++;}AMP_NOP1(0)
#define bsrbytes(bs, field, nbytes)			{AMBst_GetBytes(bs, field, nbytes); map_status.number_of_fields++;}AMP_NOP1(0)
#define bsrbreadref1						nal_read_ref1
#define bsrbreadref							nal_read_ref

#define nal_read_ae(bst, field, type)		{field = (type)AMBst_Get_ae(bst); map_status.number_of_fields++;}AMP_NOP1(0)
#define nal_read_b(bst, field, n, type)		bst_read_b(bst, field, n, type)
#define nal_read_f(bst, field, n, type)		{field = (type)AMBst_GetBits(bst, n); map_status.number_of_fields++;}AMP_NOP1(0)
#define nal_read_i8(bst, field, n)			{field = AMBst_GetTCChar(bst, n); map_status.number_of_fields++;}AMP_NOP1(0)
#define nal_read_i16(bst, field, n)			{field = AMBst_GetTCShort(bst, n); map_status.number_of_fields++;}AMP_NOP1(0)
#define nal_read_i32(bst, field, n)			{field = AMBst_GetTCLong(bst, n); map_status.number_of_fields++;}AMP_NOP1(0)
#define nal_read_i64(bst, field, n)			{field = AMBst_GetTCLongLong(bst, n); map_status.number_of_fields++;}AMP_NOP1(0)
#define nal_read_se(bst, field, type)		{field = (type)AMBst_Get_se(bst); map_status.number_of_fields++;}AMP_NOP1(0)
#define nal_read_se1(bst, field)			{field = (std::remove_reference<decltype(field)>::type)AMBst_Get_se(bst); map_status.number_of_fields++;}AMP_NOP1(0)
#define nal_read_st(bst, field)				{field = AMBst_Get_String(bst, false); map_status.number_of_fields++;}AMP_NOP1(0)
#define nal_read_u(bst, field, n, type)		{field = (type)AMBst_GetBits(bst, n); map_status.number_of_fields++;}AMP_NOP1(0)
#define nal_read_ue(bst, field, type)		{field = (type)AMBst_Get_ue(bst); map_status.number_of_fields++;}AMP_NOP1(0)
#define nal_read_ue1(bst, field)			{field = (std::remove_reference<decltype(field)>::type)AMBst_Get_ue(bst); map_status.number_of_fields++;}AMP_NOP1(0)
#define nal_read_obj(bst, obj)				{obj.Map(bst); map_status.number_of_fields++;}AMP_NOP1(0)
#define nal_read_ref1(bst, flag, ptr, type, ...)	\
											{MAP_MEM_TO_STRUCT_POINTER5_1(bst, flag, ptr, type, ##__VA_ARGS__); map_status.number_of_fields++;}AMP_NOP1(0)
#define nal_read_ref(bst, ptr, type, ...)	nal_read_ref1(bst, 1, ptr, type, ##__VA_ARGS__)
#define nal_read_bitarray(bst, bitarray, idx)	\
											{AMBst_GetBits(bst, 1)?bitarray.BitSet(idx):bitarray.BitClear(idx); map_status.number_of_fields++;}AMP_NOP1(0)
#define nal_read_bytes(bst, field, nbytes)	{AMBst_GetBytes(bst, field, nbytes); map_status.number_of_fields++;}AMP_NOP1(0)

#define av1_read_uvlc(bst, field)			{field = (std::remove_reference<decltype(field)>::type)AMBst_Get_uvlc(bst); map_status.number_of_fields++;}AMP_NOP1(0)
#define av1_read_leb128(bst, field)			{field = (std::remove_reference<decltype(field)>::type)AMBst_Get_leb128(bst); map_status.number_of_fields++;}AMP_NOP1(0)
#define av1_read_leb128_1(bst, field, len)	{field = (std::remove_reference<decltype(field)>::type)AMBst_Get_leb128(bst, &len); map_status.number_of_fields++;}AMP_NOP1(0)
#define av1_read_ns(bst, field, max_v)		{field = (std::remove_reference<decltype(field)>::type)AMBst_Get_ns(bst, max_v); map_status.number_of_fields++;}AMP_NOP1(0)
#define av1_read_ref1(bst, flag, ptr, type, ...)	\
											{MAP_MEM_TO_STRUCT_POINTER5_1(bst, flag, ptr, type, ##__VA_ARGS__); map_status.number_of_fields++;}AMP_NOP1(0)
#define av1_read_ref(bst, ptr, type, ...)	av1_read_ref1(bst, 1, ptr, type, ##__VA_ARGS__)
#define av1_read_obj(bst, obj)				{obj.Map(bst); map_status.number_of_fields++;}AMP_NOP1(0)

#define LatmValue_num_of_bits(LatmValue)	(LatmValue > 0xFFFF?26:(LatmValue>0xFF?18:(LatmValue>0?10:2)))

namespace BST {

//
// Don't use the virtual function, it may cause to load error info from a buffer.
// It will be convenient for convert the memory block to struct pointer.
// it is recommended that the single struct entry can derived from the base class.
// The struct/class directing from DIRECT_ENDIAN_MAP must be satisfied with the below,
//		1.the converted buffer shall not be smaller than the size that the struct block requires,
//		  including the varbody size.
//		2.the struct/class's all members must be consequently mapped into the buffer,
//		  alternative member with different size shall not exist.
//		3.When explicitly converting a buffer address, Endian method shall be called
//		  immediately, it may fail; when the struct block is not used yet, Endian method
//		  shall be called again so that the memory can be resume to original state.
//		4.When the buffer is converted explicitly to the struct pointer, and Endian method
//		  is called, the converted buffer shall not be accessed by other thread until
//		  Endian method is called again.
//		5.The large buffer may not be converted to the struct pointer.
//		6.All other methods excepts IsBufferReady, shall be called after the Endian method is called first.
//
struct DIRECT_ENDIAN_MAP
{
	static bool IsBufferReady(unsigned char *pBuf,unsigned long cbSize){UNREFERENCED_PARAMETER(pBuf); UNREFERENCED_PARAMETER(cbSize); return false;}
												// Check whether the buffer is enough to be converted to the struct.
	//
	// The struct is mapped into TS packet buffer, and calling Endian method will change
	// the TS packet buffer temporarily, when getting your wanted information, you must 
	// revert to the original state of TS packet buffer, so Endian method can rollback 
	// automatically if it fails or what IsValid method returns is false.
	//
	// If another struct or class inherit from it, the struct and class's Endian method must accord 
	// with ACID, all members are endianed, or all failed.
	//
	void Endian(bool bBig2SysByteOrder=true){UNREFERENCED_PARAMETER(bBig2SysByteOrder); return;}			// Endian the specified ushort, ulong and uint64 member variable.

	//
	// IsValid shall be invoked before Endian method is called, so the member variables are big-endian.
	//
	bool IsValid(){return true;}						// Judge whether the memory converted to struct is valid.

	//
	// GetVarBodySize shall be invoked before Endian method is called, so the member variables are big-endian.
	//
	int  GetVarBodySize(){return 0;}					// Get the unsized array size in the struct instance in unit of bytes.

	static int64_t bswap(int64_t t) {
		return ENDIANUINT64(t);
	}

	static uint64_t bswap(uint64_t t) {
		return ENDIANUINT64(t);
	}

	static int32_t bswap(int32_t t) {
		return ENDIANULONG(t);
	}

	static uint32_t bswap(uint32_t t) {
		return ENDIANULONG(t);
	}

	static int16_t bswap(int16_t t) {
		return ENDIANUSHORT(t);
	}

	static uint16_t bswap(uint16_t t) {
		return ENDIANUSHORT(t);
	}

	template<class T> static void rbswap(T &t){t = bswap(t);}

//#if defined(__GNUC__) || defined(__GNUG__)
//	char reserved[];
//#endif
};

//
//	The class derived from ADVANCE_ENDIAN_MAP won't re-alloc memory, it will enhance the utility of memory.
//	However, after calling Map, some memory block will be reserved because of endian, so when you finish
//	accessing the memory, you can invoke Unmap to resume the original state of memory.
//
struct ADVANCE_ENDIAN_MAP
{
	/*!	@brief Map the original buffer to the navigation metadata C++ object. */
	virtual int  Map(unsigned char *pBuf,unsigned long cbSize,unsigned long *desired_size = NULL,unsigned long *stuffing_size = NULL) = 0;
	/*!	@brief Try to restore the original buffer, and be able to export post-processed buffer based on the navigation metadata C++ object. */
	virtual int  Unmap(/* Out */ unsigned char* pBuf=NULL, /* In/Out */unsigned long* pcbSize=NULL) = 0;
	/*!	@brief Transfer the upper layer bitstream, and continue to write the fields into the bitstream */
	virtual int	 WriteToBs(AMBst bs){UNREFERENCED_PARAMETER(bs); return RET_CODE_ERROR_NOTIMPL;}
	/*!	@brief Check whether the current mapped buffer is valid or according to the spec */
	virtual bool IsValid(){return true;}
	/*!	@brief Do conversion between big-endian and little-endian. */
	virtual void Endian(bool bBig2SysByteOrder=true){UNREFERENCED_PARAMETER(bBig2SysByteOrder); return;}
};

struct BITSTREAM_MAP
{
	/*!	@brief Using the specified input bitstream object to read the metadata. */
	virtual int		Map(AMBst in_bst)=0;
	/*!	@brief Using the specified output bitstream object to write back the metadata. */
	virtual int		Unmap(AMBst out_bst=NULL)=0;
	/*!	@brief Check whether the current mapped buffer is valid or according to the spec */
	virtual bool	IsValid(){return true;}
};

struct INavFieldProp
{
	virtual size_t ProduceDesc(_Out_writes_(cbLen) char* szOutXml, size_t cbLen, bool bPrint=false, long long* bit_offset = NULL)=0;
};

/*!	@brief syntax bit-stream mapping status.
	@remarks At the beginning, error is 1, all other fields are 0. 
	When MAP_BST_BEGIN is called, error will become 0, but broken flag will be 1. 
	When MAP_BST_END is called, all fields will become 0, it means the current syntax mapping is complete. */
union SYNTAX_MAP_STATUS
{
	struct {
		int		error:1;				// 0: OK 1: error
		int		broken:1;				// 0: data is not broken(but error may exist), 1: data is broken
		int		reserved:14;			// reserved for future use
		int		number_of_fields:16;	// if no error, this field should be 0; 
										// if mapped fields can be used partly, it means how many fields are available
	}PACKED;
	int		status;
}PACKED;

struct SYNTAX_BITSTREAM_MAP: public BITSTREAM_MAP
{
	int	bit_pos;				// start bit position
	int bit_end_pos;			// end bit position
	SYNTAX_MAP_STATUS map_status;

	SYNTAX_BITSTREAM_MAP(): bit_pos(-1), bit_end_pos(-1){MAP_BST_INIT();}
	virtual ~SYNTAX_BITSTREAM_MAP() {}

	virtual int Map(AMBst in_bst){
		bit_pos = AMBst_Tell(in_bst);
		return RET_CODE_SUCCESS;
	}

	virtual int Unmap(AMBst out_bst=NULL){
		UNREFERENCED_PARAMETER(out_bst);
		return RET_CODE_SUCCESS;
	}

	int EndMap(AMBst in_bst){
		bit_end_pos = AMBst_Tell(in_bst);
		return RET_CODE_SUCCESS;
	}
}PACKED;

enum AM_MEM_USAGE
{
	MEM_USAGE_UNKNOWN,								/* Unknown memory usage. */
	MEM_USAGE_SOUND_BDMV,							/* Alloc/dealloc the memory hold by sound.bdmv. */
	MEM_USAGE_FONT_0,								/* The first font buffer with 2MB. */
	MEM_USAGE_FONT_1,								/* The second font buffer with 2MB. */
	MEM_USAGE_READ_BUF1,							/* Which is used for B SSIF content pre-read */
	MEM_USAGE_READ_BUF2,							/* Which is used for D SSIF content pre-read */
};

/*!	@brief Playback memory management.
	@remark In some case, in order to decrease the memory usage, it is necessary to implement this interface. */
class IAMMemMgr : public IUnknown
{
public:
	/*!	@brief Allocate the big block memory for specified usage. */
	virtual uint8_t*	AllocMem(AM_MEM_USAGE mem_usage, int cbSize = 0) = 0;
	/*!	@brief Free the memory allocated by AllocMem. */
	virtual void		FreeMem(AM_MEM_USAGE mem_usage, uint8_t* binData = NULL) = 0;
	/*!	@brief Copy the memory from one place to another place, HW acceleration may be used. */
	virtual void		MemCopy(_Out_writes_(cbSize) uint8_t* pDst, uint8_t* pSrc, int cbSize) = 0;
};

class MEM_MAP_MGR: public INavFieldProp
{
public:
	virtual int		Map(unsigned long *desired_size = NULL,unsigned long *stuffing_size = NULL) = 0;
	virtual int		Unmap(/* Out */ unsigned char* pBuf=NULL, /* In/Out */unsigned long* pcbSize=NULL) = 0;
	virtual size_t	ProduceDesc(_Out_writes_(cbLen) char* szOutXml, size_t cbLen, bool bPrint=false, long long* bit_offset = NULL)=0;
	virtual bool	IsValid(){return true;}
	virtual void	Endian(bool bBig2SysByteOrder=true){UNREFERENCED_PARAMETER(bBig2SysByteOrder); return;}

	virtual unsigned char* AllocMem(unsigned long cbSize, AM_MEM_USAGE mem_usage=MEM_USAGE_UNKNOWN)
	{
		assert(m_binData == NULL && m_cbDataSize == 0);
		m_mem_usage = mem_usage;
		if (m_pExMemMgr == NULL)
		{
			m_binData = new unsigned char[cbSize];
			//AMP_RegisterMem(GetCurrentModule(), m_binData, cbSize);
		}
		else
			m_binData = m_pExMemMgr->AllocMem(mem_usage, cbSize);
		m_cbDataSize = m_binData == NULL?0:cbSize;
		m_is_external_buf = false;
		return m_binData;
	}

	virtual void FreeMem(uint8_t* &pBinData, AM_MEM_USAGE mem_usage=MEM_USAGE_UNKNOWN)
	{
		if (m_is_external_buf == true)
			return;

		if (m_pExMemMgr == NULL)
		{
			AMP_SAFEDELA(pBinData);
		}
		else
		{
			m_pExMemMgr->FreeMem(mem_usage, pBinData);
			pBinData = NULL;
		}
	}

	virtual void FreeMem()
	{
		FreeMem(m_binData, m_mem_usage);
		m_cbDataSize = 0;
	}

	virtual int AttachBuf(uint8_t* pBinData, int cbSize){
		assert(m_binData == NULL && m_cbDataSize == 0);
		if (pBinData == NULL || cbSize <= 0)
			return RET_CODE_INVALID_PARAMETER;

		m_binData = pBinData;
		m_cbDataSize = cbSize;
		m_is_external_buf = true;
		return RET_CODE_SUCCESS;
	}

	virtual void DetachBuf(uint8_t* pBinData){
		if (pBinData >= m_binData && pBinData < m_binData + m_cbDataSize)
		{
			m_binData = NULL;
			m_cbDataSize = 0;
			m_is_external_buf = false;
		}
	}

	virtual void CommitExternalBuf(){
		if (m_is_external_buf == false)
			return;

		m_is_external_buf = false;
	}

	virtual uint8_t* GetMemInfo(int &cbSize)
	{
		cbSize = m_cbDataSize;
		return m_binData;
	}

	virtual int GetNavFileType(){
		return m_nav_file_type;
	}

	MEM_MAP_MGR(int nav_file_type, IAMMemMgr* pExMemMgr)
		: m_pExMemMgr(pExMemMgr), m_nav_file_type(nav_file_type), m_is_external_buf(false){
	}
	virtual ~MEM_MAP_MGR(){FreeMem();}

protected:
	unsigned char*	m_binData = nullptr;
	unsigned int	m_cbDataSize = 0;
	IAMMemMgr*		m_pExMemMgr = nullptr;
	AM_MEM_USAGE	m_mem_usage = MEM_USAGE_UNKNOWN;
	int				m_nav_file_type = -1;
	bool			m_is_external_buf = false;
};

}	// namespace BST

#ifdef _WIN32
#pragma pack(pop)
#pragma warning(pop)
#endif
#undef PACKED

#endif
