#ifndef _WTYPES_H
#define _WTYPES_H

#include "rpc.h"

#if !defined( __IID_DEFINED__ )
#define __IID_DEFINED__
typedef GUID                IID;
typedef GUID                CLSID;

typedef const GUID&         REFGUID;
typedef const IID&          REFIID;
typedef const CLSID&        REFCLSID;
#endif //__IID_DEFINED__


#define IID_NULL    GUID_NULL
#define CLSID_NULL  GUID_NULL

#if defined( __cplusplus)
#ifndef _REFGUID_DEFINED
#define _REFGUID_DEFINED
#define REFGUID             const GUID &
#endif // !_REFGUID_DEFINED
#endif

#if !defined(_WIN32) && !defined(_MPPC_)
#ifndef _LONG_DEFINED
#define _LONG_DEFINED
typedef long LONG;
#endif // !_LONG_DEFINED
#endif

typedef
enum tagCLSCTX
{
	CLSCTX_INPROC_SERVER = 0x1,
	CLSCTX_INPROC_HANDLER = 0x2,
	CLSCTX_LOCAL_SERVER = 0x4,
	CLSCTX_INPROC_SERVER16 = 0x8,
	CLSCTX_REMOTE_SERVER = 0x10,
	CLSCTX_INPROC_HANDLER16 = 0x20,
	CLSCTX_INPROC_SERVERX86 = 0x40,
	CLSCTX_INPROC_HANDLERX86 = 0x80,
	CLSCTX_ESERVER_HANDLER = 0x100
}	CLSCTX;

// COM server types
#define CLSCTX_INPROC           (CLSCTX_INPROC_SERVER|CLSCTX_INPROC_HANDLER)
// With DCOM, CLSCTX_REMOTE_SERVER should be included
#if (_WIN32_WINNT >= 0x0400 ) || defined(_WIN32_DCOM) // DCOM
#define CLSCTX_ALL              (CLSCTX_INPROC_SERVER| \
                                 CLSCTX_INPROC_HANDLER| \
                                 CLSCTX_LOCAL_SERVER| \
                                 CLSCTX_REMOTE_SERVER)

#define CLSCTX_SERVER           (CLSCTX_INPROC_SERVER|CLSCTX_LOCAL_SERVER|CLSCTX_REMOTE_SERVER)
#else
#define CLSCTX_ALL              (CLSCTX_INPROC_SERVER| \
                                 CLSCTX_INPROC_HANDLER| \
                                 CLSCTX_LOCAL_SERVER )

#define CLSCTX_SERVER           (CLSCTX_INPROC_SERVER|CLSCTX_LOCAL_SERVER)
#endif

#ifndef _LCID_DEFINED
#define _LCID_DEFINED
typedef DWORD LCID;
#endif // !_LCID_DEFINED


#endif // _WTYPES_H


