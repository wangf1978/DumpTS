// StdAfx.h : include file for standard system include files,
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
#include <set>
#include <unordered_map>
#include <tuple>
#include <algorithm>
#include <vector>
#include <assert.h>
#include <tchar.h>
#include <stdint.h>
#include <inttypes.h>
#include <climits>
#include "LibPlatform/platdef.h"

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

#define RET_CODE_IGNORE_REQUEST			   -521

#define RET_CODE_NEEDMOREINPUT			   -1000
#define RET_CODE_NEEDBYTEALIGN			   -1001

#define RET_CODE_HEADER_LOST			   -2000			// Header information can't be retrieved.
#define RET_CODE_BUFFER_TOO_SMALL		   -2001			// Can't retrieve all information field of struct from the memory block
#define RET_CODE_BUFFER_NOT_COMPATIBLE	   -2002			// The loaded buffer is not compatible with spec.
#define RET_CODE_BUFFER_NOT_FOUND		   -2003
#define RET_CODE_ERROR_CRC				   -2004

#define RET_CODE_BOX_TOO_SMALL			   -2100			// ISO 14496-12 box size is too small, and can't unpack the information according to spec
#define RET_CODE_BOX_INCOMPATIBLE		   -2101			// the current stream is incompatible with ISO 14496-12

#define RET_CODE_CONTINUE					256
#define RET_CODE_UOP_COMPLETED				257
#define RET_CODE_ALREADY_EXIST				500
#define RET_CODE_NOTHING_TODO				501
#define RET_CODE_REQUIRE_MORE_MEM			502
#define RET_CODE_DELAY_APPLY				503
#define RET_CODE_PSR_OVERWRITE				504
#define RET_CODE_CONTINUE_NAVICMD			505

enum FLAG_VALUE
{
	FLAG_UNSET = 0,
	FLAG_SET = 1,
	FLAG_UNKNOWN = 2,
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__E6271142_93A8_4CD1_B0C9_9747CCBA1E3E__INCLUDED_)
