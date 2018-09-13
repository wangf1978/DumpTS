// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

// Don't want to depend on windows build environment, and it is expected to be compiled successfully under Linux

#if !defined(AFX_STDAFX_H__E6271142_93A8_4CD1_B0C9_9747CCBA1E3E__INCLUDED_)
#define AFX_STDAFX_H__E6271142_93A8_4CD1_B0C9_9747CCBA1E3E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <stdio.h>
#include <memory.h>
#include <string>
#include <map>
#include <unordered_map>
#include <tuple>
#include <io.h>
#include <locale>
#include <algorithm>
#include <vector>
#include <assert.h>
#include <tchar.h>

#ifdef _DEBUG
#define AMP_Assert( expr ) if( !(expr) ) { \
	_tprintf(_T("[AMPSDK] ** Assertion failed: \"%s\" at %s:%d %s()\n"), _T(#expr), _T(__FILE__), __LINE__, _T(__FUNCTION__) );\
	int* x = 0; *x = 0; \
	}

#define AMP_Verify( expr ) if( !(expr) ) { \
	_tprintf( _T("[AMPSDK] ** Verification failed: \"%s\" at %s:%d %s()\n"), _T(#expr), _T(__FILE__), __LINE__, _T(__FUNCTION__) );\
	int* x = 0; *x = 0; \
	}
#else
#define AMP_Assert( expr ) if( !(expr) ) { \
	_tprintf(_T("[AMPSDK] ** Assertion failed: \"%s\" at %s:%d %s()\n"), _T(#expr), _T(__FILE__), __LINE__, _T(__FUNCTION__) );\
	}

#define AMP_Verify( expr ) if( !(expr) ) { \
	_tprintf( _T("[AMPSDK] ** Verification failed: \"%s\" at %s:%d %s()\n"), _T(#expr), _T(__FILE__), __LINE__, _T(__FUNCTION__) );\
	}
#endif

#ifdef _WIN32
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
#define AMP_SAFEDELA(p)						if(p){delete [] p;p = NULL;}CODE_NOP1(p)

#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(P)          (P)
#endif

#ifndef DBG_UNREFERENCED_PARAMETER
#define DBG_UNREFERENCED_PARAMETER(P)      (P)
#endif

#ifndef DBG_UNREFERENCED_LOCAL_VARIABLE
#define DBG_UNREFERENCED_LOCAL_VARIABLE(V) (V)
#endif

// Portable error definition
#ifdef __linux__
#define E_NOINTERFACE						0x80004002L
#define E_FAIL								0x80004005L

#define S_OK								0L
#define S_FALSE								1L
#endif

#define RET_CODE_SUCCESS					0
#define RET_CODE_ERROR					   -1
#define RET_CODE_ERROR_NOTIMPL			   -2
#define RET_CODE_ERROR_FILE_NOT_EXIST	   -3
#define RET_CODE_ERROR_PERMISSION		   -6
#define RET_CODE_INVALID_PARAMETER		   -7
#define RET_CODE_OUTOFMEMORY			   -8

#define RET_CODE_NEEDMOREINPUT			   -1000
#define RET_CODE_NEEDBYTEALIGN			   -1001

#define RET_CODE_HEADER_LOST			   -2000			// Header information can't be retrieved.
#define RET_CODE_BUFFER_TOO_SMALL		   -2001			// Can't retrieve all information field of struct from the memory block
#define RET_CODE_BUFFER_NOT_COMPATIBLE	   -2002			// The loaded buffer is not compatible with spec.
#define RET_CODE_BUFFER_NOT_FOUND		   -2003
#define RET_CODE_ERROR_CRC				   -2004

#define RET_CODE_BOX_TOO_SMALL			   -2100			// ISO 14496-12 box size is too small, and can't unpack the information according to spec
#define RET_CODE_BOX_INCOMPATIBLE		   -2101			// the current stream is incompatible with ISO 14496-12

enum FLAG_VALUE
{
	FLAG_UNSET = 0,
	FLAG_SET = 1,
	FLAG_UNKNOWN = 2,
};

enum INT_VALUE_LITERAL_FORMAT
{
	FMT_AUTO = 0,
	FMT_HEX,
	FMT_DEC,
	FMT_OCT,
	FMT_BIN
};

inline bool ConvertToInt(char* ps, char* pe, int64_t& ret_val, INT_VALUE_LITERAL_FORMAT literal_fmt = FMT_AUTO)
{
	ret_val = 0;
	bool bNegative = false;

	if (pe <= ps)
		return false;
		
	if (literal_fmt == FMT_AUTO)
	{
		if (pe > ps + 2 && _strnicmp(ps, "0x", 2) == 0)	// hex value
		{
			ps += 2;
			literal_fmt = FMT_HEX;
		}
		else if (*(pe - 1) == 'h' || *(pe - 1) == 'H')
		{
			pe--;
			literal_fmt = FMT_HEX;
		}
		else if (pe > ps + 2 && _strnicmp(ps, "0b", 2) == 0)
		{
			ps += 2;
			literal_fmt = FMT_BIN;
		}
		else if (*(pe - 1) == 'b' || *(pe - 1) == 'B')
		{
			// check whether all value from ps to pe-1, all are 0 and 1
			auto pc = ps;
			for (; pc < pe - 1; pc++)
				if (*pc != '0' && *pc != '1')
					break;

			if (pc >= pe - 1)
			{
				pe--;
				literal_fmt = FMT_BIN;
			}
		}
		else if (*ps == '0' || *ps == 'o' || *ps == 'O')
		{
			ps++;
			literal_fmt = FMT_OCT;
		}
		else if (*(pe - 1) == 'o' || *(pe - 1) == 'O')
		{
			pe--;
			literal_fmt = FMT_OCT;
		}

		if (literal_fmt == FMT_AUTO)
		{
			// still not decided
			auto pc = ps;
			for (; pc < pe; pc++)
			{
				if (*pc >= 'a' && *pc <= 'f' || *pc >= 'A' && *pc <= 'F')
					literal_fmt = FMT_HEX;
				else if (*pc >= '0' && *pc <= '9')
				{
					if (literal_fmt == FMT_AUTO)
						literal_fmt = FMT_DEC;
				}
				else
					return false;	// It is not an valid value
			}
		}
	}
	else if (literal_fmt == FMT_HEX)
	{
		if (pe > ps + 2 && _strnicmp(ps, "0x", 2) == 0)	// hex value
			ps += 2;
		else if (*(pe - 1) == 'h' || *(pe - 1) == 'H')
			pe--;
	}
	else if (literal_fmt == FMT_DEC)
	{
		if (*ps == '-')
		{
			bNegative = true;
			ps++;
		}
	}
	else if (literal_fmt == FMT_OCT)
	{
		if (*ps == '0' || *ps == 'o' || *ps == 'O')
			ps++;
		else if (*(pe - 1) == 'o' || *(pe - 1) == 'O')
			pe--;
	}
	else if (literal_fmt == FMT_BIN)
	{
		if (pe > ps + 2 && _strnicmp(ps, "0b", 2) == 0)	// binary value
			ps += 2;
		else if (*(pe - 1) == 'b' || *(pe - 1) == 'B')
			pe--;
	}

	if (literal_fmt != FMT_HEX && literal_fmt != FMT_DEC && literal_fmt != FMT_OCT && literal_fmt != FMT_BIN)
		return false;

	while (ps < pe)
	{
		if (literal_fmt == FMT_HEX)	// hex value
		{
			if (*ps >= '0' && *ps <= '9')
				ret_val = (ret_val << 4) | (*ps - '0');
			else if (*ps >= 'a' && *ps <= 'f')
				ret_val = (ret_val << 4) | (*ps - 'a' + 10);
			else if (*ps >= 'A' && *ps <= 'F')
				ret_val = (ret_val << 4) | (*ps - 'A' + 10);
			else
				return false;
		}
		else if (literal_fmt == FMT_OCT)	// octal value
		{
			if (*ps >= '0' && *ps <= '7')
				ret_val = (ret_val << 3) | (*ps - '0');
			else
				return false;
		}
		else if (literal_fmt == FMT_DEC)		// decimal value
		{
			if (*ps >= '0' && *ps <= '9')
				ret_val = (ret_val * 10) + (*ps - '0');
			else
				return false;
		}
		else if (literal_fmt == FMT_BIN)
		{
			if (*ps >= '0' && *ps <= '1')
				ret_val = (ret_val << 1) | (*ps - '0');
			else
				return false;
		}
		else
			return false;

		ps++;
	}

	if (bNegative)
		ret_val = -1LL * ret_val;

	return true;
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__E6271142_93A8_4CD1_B0C9_9747CCBA1E3E__INCLUDED_)
