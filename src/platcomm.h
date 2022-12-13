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
#if !defined(PLAT_COMM_H__E6271142_93A8_4CD1_B0C9_9747CCBA1E3E__INCLUDED_)
#define PLAT_COMM_H__E6271142_93A8_4CD1_B0C9_9747CCBA1E3E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <stdio.h>
#include <memory.h>
#include <string>
#include <map>
#include <set>
#include <unordered_map>
#include <tuple>
#include <algorithm>
#include <vector>
#include <assert.h>
#include <tchar.h>
#include <stdint.h>
#include <inttypes.h>
#include <limits>
#include <climits>
#include <new>
#include <memory>
#include "LibPlatform/platdef.h"

#ifdef _WIN32
#define AMP_NOP1(p)					p
#else
#define AMP_NOP1(p)					(void)0
#endif

#if defined(_WIN32) && defined(_USRDLL)
#ifdef	AMP_FOUNDATION_DLL
#define AMP_FOUNDATION_PROC __declspec(dllexport)
#else
#define AMP_FOUNDATION_PROC __declspec(dllimport)
#endif
#define AMP_CLASS			class __declspec(dllimport)
#else
#define AMP_FOUNDATION_PROC
#define AMP_CLASS
#endif

#ifdef _MSC_VER
#define INLINE __forceinline /* use __forceinline (VC++ specific) */
#else
#define INLINE inline        /* use standard inline */
#endif

#ifdef _WIN32
#define AMP_CUR_THREAD_ID()			(::GetCurrentThreadId())
#define STRICMP							_tcsicmp
#define STRNICMP						_tcsnicmp
#define STRCMP							_tcscmp
#define SSCANF							_stscanf_s
#define STRUPR(s,l)						_tcsupr_s(s, l)
#define STRCPY(s,l,ss)					_tcscpy_s(s,l,ss)
#define STRCAT(s,l,ss)					_tcscat_s(s,l,ss)
#define STRNCPY(s,l,ss,n)				_tcsncpy_s(s,l,ss,n)
#define _STPRINTF_S(s,l,f,...)			_stprintf_s(s,l,f,__VA_ARGS__)
#define	_SNTPRINTF_S(s,l,cc,f,...)		_sntprintf_s(s, l, cc, f, __VA_ARGS__)
#define _VSNTPRINTF_S(s,l,c,f,a)		_vsntprintf_s(s,l,c,f,a)

#define MBCSICMP						_stricmp
#define MBCSNICMP						_strnicmp
#define MBCSCPY(s,l,ss)					strcpy_s(s,l,ss)
#define MBCSCAT(s,l,ss)					strcat_s(s,l,ss)
#define MBCSNCPY(s,l,ss,n)				strncpy_s(s,l,ss,n)
#define	MBCSPRINTF_S(s,l,f,...)			sprintf_s(s,l,f, __VA_ARGS__)
#define	MBCSNPRINTF_S(s,l,cc,f,...)		_snprintf_s(s,l,cc, f, __VA_ARGS__)
#define MBCSVSNPRINTF_S(s,l,c,f,a)		_vsnprintf_s(s,l,c,f,a)

#define WCSICMP							_wcsicmp
#define WCSNICMP						_wcsnicmp
#define WCSCPY(s,l,ss)					wcscpy_s(s,l,ss)
#define WCSCAT(s,l,ss)					wcscat_s(s,l,ss)
#define WCSNCPY(s,l,ss,n)				wcsncpy_s(s,l,ss,n)
#define	WCSPRINTF_S(s,l,f,...)			swprintf_s(s,l,f, __VA_ARGS__)
#define	WCSNPRINTF_S(s,l,cc,f,...)		_snwprintf_s(s,l,cc, f, __VA_ARGS__)
#define WCSVSNPRINTF_S(s,l,c,f,a)		_vsnwprintf_s(s,l,c,f,a)

#ifdef _UNICODE
#define FOPEN(fp, n, a)					fp=_tfsopen(n, _T(a), _SH_DENYNO)
#else
#define FOPEN(fp, n, a)					fp=_fsopen(n, a, _SH_DENYNO)
#endif
#define FOPENA(fp, n, a)				fp=_fsopen(n, a, _SH_DENYNO)
#define FOPENW(fp, n, a)				fp=_wfsopen(n, a, _SH_DENYNO)

#define UNLINK(p)						_tunlink(p)
#define _ACCESS							_access
#define _OPEN							_open
#define _CLOSE							_close
#define _READ							_read
#define _WRITE							_write
#define _LSEEKI64						_lseeki64
#define _LOCALTIME(a, b)				localtime_s(a, b)
#define _TZSET							_tzset
#define	mkdir(path, mode)				_tmkdir(path)
#define _MKDIR(path, mode)				_mkdir(path)
#if __cplusplus > 199711L || defined(_MSVC_LANG) && _MSVC_LANG >= 201402L
#define MEMCPY(d,ds,s,ss)				memcpy_s(d,ds,s,ss)
#else
#define MEMCPY(d,ds,s,ss)				memcpy(d,s,ss)
#endif
#define FSEEK64							_fseeki64
#define FTELL64							_ftelli64
#elif defined(__linux__) || defined(__APPLE__)
#define AMP_CUR_THREAD_ID()  			((unsigned long)pthread_self())
#define STRCMP							strcmp
#define STRNICMP						strncasecmp
#define STRUPR(s,l)						_strupr(s)	
#define STRICMP							strcasecmp
#define SSCANF							sscanf
#define STRCPY(s,l,ss)					strcpy(s,ss)
#define STRCAT(s,l,ss)					strcat(s,ss)
#define STRNCPY(s,l,ss,n)				strncpy(s,ss,n)
#define _STPRINTF_S(s,l,f,...)			_stprintf(s,f,##__VA_ARGS__)
#define	_SNTPRINTF_S(s,l,cc,f,...)		_sntprintf(s, cc+1, f, ##__VA_ARGS__)
#define _VSNTPRINTF_S(s,l,c,f,a)		_vsntprintf(s,c+1,f,a)

#define MBCSICMP						strcasecmp
#define MBCSNICMP						strncasecmp
#define MBCSCPY(s,l,ss)					strcpy(s,ss)
#define MBCSCAT(s,l,ss)					strcat(s,ss)
#define MBCSNCPY(s,l,ss,n)				strncpy(s,ss,n)
#define	MBCSPRINTF_S(s,l,f,...)			sprintf(s,f, ##__VA_ARGS__)
#define	MBCSNPRINTF_S(s,l,cc,f,...)		snprintf(s,cc+1,f, ##__VA_ARGS__)
#define MBCSVSNPRINTF_S(s,l,c,f,a)		vsnprintf(s,c+1,f,a)

#define WCSICMP							wcscasecmp
#define WCSNICMP						wcsncasecmp
#define WCSCPY(s,l,ss)					wcscpy(s,ss)
#define WCSCAT(s,l,ss)					wcscat(s,ss)
#define WCSNCPY(s,l,ss,n)				wcsncpy(s,ss,n)
#define	WCSPRINTF_S(s,l,f,...)			swprintf(s,l, f, ##__VA_ARGS__)
#define	WCSNPRINTF_S(s,l,cc,f,...)		swprintf(s,cc+1,f, ##__VA_ARGS__)
#define WCSVSNPRINTF_S(s,l,c,f,a)		vswprintf(s,c+1,f,a)

#define _ACCESS							access
#define _MKDIR(path, mode)				mkdir(path, mode)
#define FOPEN(fp, n, a)					fp = fopen(n, a);
#define FOPENA(fp, n, a)				fp = fopen(n, a);
#define UNLINK(p)						unlink(p)
#define _OPEN							open
#define _CLOSE							close
#define _READ							read
#define _WRITE							write
#define _LSEEKI64						lseek64
#define _LOCALTIME(a, b)				a = localtime(b)
#define _TZSET							_tzset
#define MEMCPY(d,ds,s,ss)				memcpy(d,s,ss)
#define FSEEK64							fseeko
#define FTELL64							ftello
#endif // _WIN32

#ifdef _DEBUG
#define AMP_Assert( expr ) if( !(expr) ) { \
	_tprintf(_T("[BST] ** Assertion failed: \"%s\" at %s:%d %s()\n"), _T(#expr), _T(__FILE__), __LINE__, _T(__FUNCTION__) );\
	int* x = 0; *x = 0; \
	}

#define AMP_Verify( expr ) if( !(expr) ) { \
	_tprintf( _T("[BST] ** Verification failed: \"%s\" at %s:%d %s()\n"), _T(#expr), _T(__FILE__), __LINE__, _T(__FUNCTION__) );\
	int* x = 0; *x = 0; \
	}
#else
#define AMP_Assert( expr ) if( !(expr) ) { \
	_tprintf(_T("[BST] ** Assertion failed: \"%s\" at %s:%d %s()\n"), _T(#expr), _T(__FILE__), __LINE__, _T(__FUNCTION__) );\
	}

#define AMP_Verify( expr ) if( !(expr) ) { \
	_tprintf( _T("[BST] ** Verification failed: \"%s\" at %s:%d %s()\n"), _T(#expr), _T(__FILE__), __LINE__, _T(__FUNCTION__) );\
	}
#endif

#if defined(_MSC_VER) && _MSC_VER < 1920
#define CODE_NOP1(p)						p
#else
#define CODE_NOP1(p)						(void)0
#endif

#define AMP_ABS(A)							((A) < 0 ? (-(A)) : (A))
#define AMP_ABS_MINUS(A, B)					((A)>=(B)?((A)-(B)):((B)-(A)))
#define AMP_MIN(A, B)						((A) <= (B)?(A):(B))
#define AMP_MAX(A, B)						((A) >= (B)?(A):(B))

#define AMP_SAFERELEASE(p)					if(p){p->Release();p = NULL;}CODE_NOP1(p)
#define AMP_SAFEASSIGN(p, v)				if(p){*(p) = (v);}CODE_NOP1(p)
#define AMP_SAFEASSIGN1(p, v)				if(p){*(p) = (v); if(v)v->ProcAddRef();}CODE_NOP1(p)

#define AMP_SAFEDEL(p)						if(p){delete p;p = NULL;}CODE_NOP1(p)
#define AMP_SAFEDEL2(p)						if(p){/*AMP_UnregisterMem(GetCurrentModule(), (void*)p); */delete p;p = NULL;}AMP_NOP1(p)
#define AMP_SAFEDEL3(p)						/*AMP_UnregisterMem(GetCurrentModule(), (void*)p); */delete p; p = NULL
#define AMP_SAFEDEL4(p)						/*AMP_UnregisterMem(GetCurrentModule(), (void*)p); */delete p

#define AMP_SAFEDELA(p)						if(p){delete [] p;p = NULL;}CODE_NOP1(p)
#define AMP_SAFEDELA2(p)					if(p){/*AMP_UnregisterMem(GetCurrentModule(), (void*)p);*/ delete [] p;p = NULL;}AMP_NOP1(p)
#define AMP_SAFEDELA3(p)					/*AMP_UnregisterMem(GetCurrentModule(), (void*)p); */delete [] p; p = NULL
#define AMP_SAFEDELA4(p)					/*AMP_UnregisterMem(GetCurrentModule(), (void*)p); */delete [] p

#define AMP_FAILED(retcode)					(((int)(retcode)) < 0)
#define AMP_SUCCEEDED(retcode)				(((int)(retcode)) >= 0)

#define AMP_CHKRETBREAK(retcode)			if(AMP_FAILED(retcode))break;
#define AMP_CHKBOOL1(retcode, lb)			if(!(retcode))goto lb
#define AMP_CHKBOOL(retcode)				AMP_CHKBOOL1(retcode, done)
#define AMP_CHKRET1(retcode, lb)			if(AMP_FAILED((retcode)))goto lb
#define AMP_CHKRET(retcode)					AMP_CHKRET1(retcode, done)

#ifdef _DEBUG
#define _AMP_NEW(mtype, size)				(size)<=0?NULL:(new mtype[size])
#define _AMP_NEWT(mtype, ...)				new mtype(__VA_ARGS__)
#else
#define _AMP_NEW(mtype, size)				(size)<=0?NULL:(new (std::nothrow) mtype[size])
#define _AMP_NEWT(mtype, ...)				new (std::nothrow) mtype(__VA_ARGS__)
#endif

#define AMP_NEW0(p, mtype, size)			{(p) = _AMP_NEW(mtype, size); if(p){/*AMP_RegisterMem(GetCurrentModule(), (void*)(p), sizeof(mtype)*(size)); */memset((p), 0, sizeof(mtype)*(size));}}AMP_NOP1(p)
#define AMP_NEW(p, mtype, size)				{(p) = _AMP_NEW(mtype, size); if(p){/*AMP_RegisterMem(GetCurrentModule(), (void*)(p), sizeof(mtype)*(size));*/}}AMP_NOP1(p)
#define AMP_NEW1(p, mtype, size)			mtype* p = _AMP_NEW(mtype, size); if(p){/*AMP_RegisterMem(GetCurrentModule(), (void*)(p), sizeof(mtype)*(size));*/}AMP_NOP1(p)

#define AMP_NEWT(p, mtype, ...)				{(p) = _AMP_NEWT(mtype, __VA_ARGS__); if(p){/*AMP_RegisterMem(GetCurrentModule(), (void*)(p), sizeof(mtype));*/}}AMP_NOP1(p)
#define AMP_NEWT1(p, mtype, ...)			mtype* p = _AMP_NEWT(mtype, __VA_ARGS__); if(p){/*AMP_RegisterMem(GetCurrentModule(), (void*)(p), sizeof(mtype));*/}AMP_NOP1(p)

#define _MACRO_W(x)							L ## x

#define DECL_TUPLE_A(x)						{x, #x}
#define DECL_TUPLE_W(x)						{x, _MACRO_W(#x)}

#ifdef _UNICODE
#define DECL_TUPLE(x)						DECL_TUPLE_W(##x)
#else
#define DECL_TUPLE(x)						DECL_TUPLE_A(##x)
#endif

#define DECL_ENUM_ITEMW(x, l)				((x) == l? _MACRO_W(#l):
#define DECL_ENUM_LAST_ITEMW(x, l, v)		((x) == l? _MACRO_W(#l):v

#define DECL_ENUM_ITEMA(x, l)				((x) == l? #l:
#define DECL_ENUM_LAST_ITEMA(x, l, v)		((x) == l? #l:v

#ifndef UNREFERENCED_PARAMETER
#ifdef _WIN32
#define UNREFERENCED_PARAMETER(P)          (P)
#else
#define UNREFERENCED_PARAMETER(P)		   (void)0
#endif
#endif

#ifndef DBG_UNREFERENCED_PARAMETER
#ifdef _WIN32
#define DBG_UNREFERENCED_PARAMETER(P)      (P)
#else
#define DBG_UNREFERENCED_PARAMETER(P)      (void)0
#endif
#endif

#ifndef DBG_UNREFERENCED_LOCAL_VARIABLE
#ifdef _WIN32
#define DBG_UNREFERENCED_LOCAL_VARIABLE(V) (V)
#else
#define DBG_UNREFERENCED_LOCAL_VARIABLE(V) (void)0
#endif
#endif

#define RET_CODE_SUCCESS					0
#define RET_CODE_ERROR					   -1
#define RET_CODE_ERROR_NOTIMPL			   -2
#define RET_CODE_ERROR_FILE_NOT_EXIST	   -3
#define RET_CODE_ERROR_PERMISSION		   -6
#define RET_CODE_INVALID_PARAMETER		   -7
#define RET_CODE_OUTOFMEMORY			   -8
#define RET_CODE_ABORT					   -9
#define RET_CODE_NOT_FOUND				   -10

#define RET_CODE_NOT_INITIALIZED		   -500
#define RET_CODE_MISMATCH				   -504
#define RET_CODE_TIME_OUT				   -512
#define RET_CODE_OUT_OF_RANGE			   -515
#define RET_CODE_CONTAINER_NOT_EXIST	   -518
#define RET_CODE_IGNORE_REQUEST			   -521
#define RET_CODE_IO_READ_ERROR			   -534
#define RET_CODE_NOT_FINALIZED			   -540
#define RET_CODE_ERROR_STATE_TRANSITION	   -541
#define RET_CODE_NO_MORE_DATA			   -546
#define RET_CODE_NOT_ALIGNED			   -547

#define RET_CODE_NEEDMOREINPUT			   -1000
#define RET_CODE_NEEDBYTEALIGN			   -1001

#define RET_CODE_HEADER_LOST			   -2000			// Header information can't be retrieved.
#define RET_CODE_BUFFER_TOO_SMALL		   -2001			// Can't retrieve all information field of struct from the memory block
#define RET_CODE_BUFFER_NOT_COMPATIBLE	   -2002			// The loaded buffer is not compatible with spec.
#define RET_CODE_BUFFER_NOT_FOUND		   -2003
#define RET_CODE_ERROR_CRC				   -2004
#define RET_CODE_BUFFER_OVERFLOW		   -2005
#define RET_CODE_BUFFER_UNDERFLOW		   -2006
#define RET_CODE_DEFERRED				   -2007

#define RET_CODE_BOX_TOO_SMALL			   -2100			// ISO 14496-12 box size is too small, and can't unpack the information according to spec
#define RET_CODE_BOX_INCOMPATIBLE		   -2101			// the current stream is incompatible with ISO 14496-12

/* For ODD return value(-3xxx) */

/* For Multimedia return value (-4xxx) */
#define RET_CODE_CLOCK_DISCONTINUITY	   -4000

#define RET_CODE_FALSE						1

#define RET_CODE_CONTINUE					256
#define RET_CODE_COMPLETED					257
#define RET_CODE_UOP_COMPLETED				RET_CODE_UOP_COMPLETED
#define RET_CODE_ALREADY_EXIST				500
#define RET_CODE_NOTHING_TODO				501
#define RET_CODE_REQUIRE_MORE_MEM			502
#define RET_CODE_DELAY_APPLY				503
#define RET_CODE_PSR_OVERWRITE				504
#define RET_CODE_CONTINUE_NAVICMD			505

#define INVALID_TM_90KHZ_VALUE				-1LL

///////////////////////////////////////////////////////////////
// Utility PTS compare
///////////////////////////////////////////////////////////////
#define PTS_90K_EQ(pts1, pts2)				(((pts1)>>9) == ((pts2)>>9))
#define PTS_90K_GT(pts1, pts2)				(((pts1)>>9) >  ((pts2)>>9))
#define PTS_90K_LT(pts1, pts2)				(((pts1)>>9) <  ((pts2)>>9))
#define PTS_90K_GE(pts1, pts2)				(((pts1)>>9) >= ((pts2)>>9))
#define PTS_90K_LE(pts1, pts2)				(((pts1)>>9) <= ((pts2)>>9))

#define PTS_45K_EQ(pts1, pts2)				(((pts1)>>8) == ((pts2)>>8))
#define PTS_45K_GT(pts1, pts2)				(((pts1)>>8) >  ((pts2)>>8))
#define PTS_45K_LT(pts1, pts2)				(((pts1)>>8) <  ((pts2)>>8))
#define PTS_45K_GE(pts1, pts2)				(((pts1)>>8) >= ((pts2)>>8))
#define PTS_45K_LE(pts1, pts2)				(((pts1)>>8) <= ((pts2)>>8))

#define PTS_100NS_EQ(pts1, pts2)			(((pts1*9/1000)>>9) == ((pts2*9/1000)>>9))
#define PTS_100NS_GT(pts1, pts2)			(((pts1*9/1000)>>9) >  ((pts2*9/1000)>>9))
#define PTS_100NS_LT(pts1, pts2)			(((pts1*9/1000)>>9) <  ((pts2*9/1000)>>9))
#define PTS_100NS_GE(pts1, pts2)			(((pts1*9/1000)>>9) >= ((pts2*9/1000)>>9))
#define PTS_100NS_LE(pts1, pts2)			(((pts1*9/1000)>>9) <= ((pts2*9/1000)>>9))

#ifdef _BIG_ENDIAN_
#define ENDIANUSHORT(src)           (uint16_t)src
#define ENDIANULONG(src)			(uint32_t)src
#define ENDIANUINT64(src)			(uint64_t)src

#define USHORT_FIELD_ENDIAN(field)
#define ULONG_FIELD_ENDIAN(field)
#define UINT64_FIELD_ENDIAN(field)
#define UTYPE_FIELD_ENDIAN(field)
#else
#define ENDIANUSHORT(src)			((uint16_t)((((src)>>8)&0xff) |\
												(((src)<<8)&0xff00)))

#define ENDIANULONG(src)			((uint32_t)((((src)>>24)&0xFF) |\
												(((src)>> 8)&0xFF00) |\
												(((src)<< 8)&0xFF0000) |\
												(((src)<<24)&0xFF000000)))

#define ENDIANUINT64(src)			((uint64_t)((((src)>>56)&0xFF) |\
												(((src)>>40)&0xFF00) |\
												(((src)>>24)&0xFF0000) |\
												(((src)>> 8)&0xFF000000) |\
												(((src)<< 8)&0xFF00000000LL) |\
												(((src)<<24)&0xFF0000000000LL) |\
												(((src)<<40)&0xFF000000000000LL) |\
												(((src)<<56)&0xFF00000000000000LL)))

#define USHORT_FIELD_ENDIAN(field)	field = ENDIANUSHORT(((uint16_t)field));
#define ULONG_FIELD_ENDIAN(field)	field = ENDIANULONG(((uint32_t)field));
#define UINT64_FIELD_ENDIAN(field)	field = ENDIANUINT64(((uint64_t)field));
#define UTYPE_FIELD_ENDIAN(field)	field.Endian();
#endif //_BIG_ENDIAN_

#define IS_INVALID_HANDLE(handle)	((handle) == NULL)

#define IVF_HDR_SIZE				32
#define IVF_PIC_HDR_SIZE			12

#ifndef _WIN32
#define HMODULE												void*
#define LoadLibrary(szPath)									dlopen(szPath,RTLD_NOW)
#define LoadLibraryEx(szPath, hFile, dwFlags)				dlopen(szPath,RTLD_NOW)
#define LoadPackagedLibrary(szPath, Reserved)				dlopen(szPath,RTLD_NOW)
#define GetProcAddress(hModule,lpProcName)					dlsym(hModule,lpProcName)
#define FreeLibrary(hModule)								dlclose(hModule)
#endif

enum FLAG_VALUE
{
	FLAG_UNSET = 0,
	FLAG_SET = 1,
	FLAG_UNKNOWN = 2,
};

void print_mem(uint8_t* pBuf, int cbSize, int indent);
long long GetFileSizeByFP(FILE* rfp);

// Get how many records to be displayed
int GetTopRecordCount();

inline uint8_t quick_log2(uint32_t v)
{
	for (int i = 31; i >= 0; i--)
		if ((v&(1 << i)))
			return i;
	return 0;
}

inline uint8_t quick64_log2(uint64_t v)
{
	for (int i = 63; i >= 0; i--)
		if ((v&((uint64_t)1 << i)))
			return i;
	return 0;
}

template<typename T,
	typename std::enable_if<std::numeric_limits<T>::is_integer>::type* = nullptr>
	inline uint8_t ceiling_quick_log2(T v)
{
	int8_t i = sizeof(T) << 3;
	for (; i >= 0; i--)
		if ((v&(1 << i)))
			break;

	if (i > 0)
	{
		int8_t j = i - 1;
		for (; j >= 0; j--)
			if ((v&(1 << j)))
			{
				i++;
				break;
			}
	}

	return i;
}

template <typename T>
inline int8_t floor_quick_log2(T x)
{
	int8_t s = 0;
	while (x != 0)
	{
		x = x >> 1;
		s++;
	}
	return s - 1;
}

inline uint8_t quick_ceil_log2(uint32_t v)
{
	uint8_t qlog2_value = quick_log2(v);
	return (1UL << qlog2_value) != v ? (qlog2_value + 1) : (qlog2_value);
}

struct CaseInsensitiveComparator
{
	bool operator()(const std::string& a, const std::string& b) const noexcept
	{
		return MBCSICMP(a.c_str(), b.c_str()) < 0;
	}
};

#ifdef _MSC_VER

constexpr size_t constexpr_strlen_internal(const char* s, size_t len) {
	return *s == '\0' ? len : constexpr_strlen_internal(s + 1, len + 1);
}

static_assert(constexpr_strlen_internal("123456789", 0) == 9,
	"Someone appears to have broken constexpr_strlen...");
#endif

constexpr size_t constexpr_strlen(const char* s) {
#if defined(__clang__)
	return __builtin_strlen(s);
#elif defined(_MSC_VER)
	return s == nullptr ? 0 : constexpr_strlen_internal(s, 0);
#else
	return strlen(s);
#endif
}

#endif
