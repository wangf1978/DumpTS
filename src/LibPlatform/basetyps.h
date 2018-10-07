//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992-1997.
//
//  File:       basetyps.h
//
//----------------------------------------------------------------------------
#if !defined( _BASETYPS_H_ )
#define _BASETYPS_H_

#include <string.h>

// Common macros gleamed from COMPOBJ.H

#ifdef __cplusplus
    #define EXTERN_C    extern "C"
#else
    #define EXTERN_C    extern
#endif

#ifdef _WIN32

// Win32 doesn't support __export

#define STDMETHODCALLTYPE       __stdcall
#define STDMETHODVCALLTYPE      __cdecl

#define STDAPICALLTYPE          __stdcall
#define STDAPIVCALLTYPE         __cdecl

#else

#define STDMETHODCALLTYPE       __export __stdcall
#define STDMETHODVCALLTYPE      __export __cdecl

#define STDAPICALLTYPE          __export __stdcall
#define STDAPIVCALLTYPE         __export __cdecl

#endif

#define STDAPI                  EXTERN_C HRESULT STDAPICALLTYPE
#define STDAPI_(type)           EXTERN_C type STDAPICALLTYPE

#define STDMETHODIMP            HRESULT STDMETHODCALLTYPE
#define STDMETHODIMP_(type)     type STDMETHODCALLTYPE

// The 'V' versions allow Variable Argument lists.

#define STDAPIV                 EXTERN_C HRESULT STDAPIVCALLTYPE
#define STDAPIV_(type)          EXTERN_C type STDAPIVCALLTYPE

#define STDMETHODIMPV           HRESULT STDMETHODVCALLTYPE
#define STDMETHODIMPV_(type)    type STDMETHODVCALLTYPE

#define IFACEMETHODIMP          STDMETHODIMP
#define IFACEMETHODIMP_(type)   STDMETHODIMP_(type)


#if defined(__cplusplus) && !defined(CINTERFACE)
//#define interface               struct FAR
#define interface struct
#define STDMETHOD(method)       virtual HRESULT STDMETHODCALLTYPE method
#define STDMETHOD_(type,method) virtual type STDMETHODCALLTYPE method
#define PURE                    = 0
#define THIS_
#define THIS                    void
#ifndef DECLARE_INTERFACE
#define DECLARE_INTERFACE(iface)    interface iface
#endif
#define DECLARE_INTERFACE_(iface, baseiface)    interface iface : public baseiface



#else

#define interface               struct

#define STDMETHOD(method)       HRESULT (STDMETHODCALLTYPE * method)
#define STDMETHOD_(type,method) type (STDMETHODCALLTYPE * method)




#define PURE
#define THIS_                   INTERFACE FAR* This,
#define THIS                    INTERFACE FAR* This
#ifdef CONST_VTABLE
#define DECLARE_INTERFACE(iface)    typedef interface iface { \
                                    const struct iface##Vtbl FAR* lpVtbl; \
                                } iface; \
                                typedef const struct iface##Vtbl iface##Vtbl; \
                                const struct iface##Vtbl
#else
#define DECLARE_INTERFACE(iface)    typedef interface iface { \
                                    struct iface##Vtbl FAR* lpVtbl; \
                                } iface; \
                                typedef struct iface##Vtbl iface##Vtbl; \
                                struct iface##Vtbl
#endif
#define DECLARE_INTERFACE_(iface, baseiface)    DECLARE_INTERFACE(iface)

#endif

#ifndef INITGUID
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    EXTERN_C const GUID FAR name
#else

#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        EXTERN_C const GUID name \
                = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
#endif // INITGUID

#define IsEqualGUID(rguid1, rguid2)		(!memcmp(&rguid1, &rguid2, sizeof(GUID)))

#define IsEqualIID(riid1, riid2)		IsEqualGUID(riid1, riid2)
#define IsEqualCLSID(rclsid1, rclsid2)	IsEqualGUID(rclsid1, rclsid2)

#define DEFINE_OLEGUID(name, l, w1, w2) \
    DEFINE_GUID(name, l, w1, w2, 0xC0,0,0,0,0,0,0,0x46)

#ifndef _ERROR_STATUS_T_DEFINED
typedef unsigned long error_status_t;
#define _ERROR_STATUS_T_DEFINED
#endif

#ifndef GUID_DEFINED
#define GUID_DEFINED
typedef struct _GUID
{
    unsigned long Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char Data4[8];

	bool operator==(const _GUID &g) const 
		{
		return (Data1 == g.Data1 && Data2 == g.Data2 && Data3 == g.Data3 &&
			Data4[0] == g.Data4[0] && Data4[1] == g.Data4[1] &&
			Data4[2] == g.Data4[2] && Data4[3] == g.Data4[3] &&
			Data4[4] == g.Data4[4] && Data4[5] == g.Data4[5] &&
			Data4[6] == g.Data4[6] && Data4[7] == g.Data4[7]);
		}
	bool operator!=(const _GUID &g) const { return !(*this==g); }

} __attribute__ ((__packed__)) GUID;
#endif /* GUID_DEFINED */

#ifndef ATTRIBUTE_WEAK
#if !defined(_WIN32)
#define ATTRIBUTE_WEAK __attribute__ ((weak))
#else
#define ATTRIBUTE_WEAK
#endif
#endif

static const GUID GUID_NULL = { 0,0,0,{ 0 } };

#define nybble_from_hex(c)      ((c>='0'&&c<='9')?(c-'0'):((c>='a'&&c<='f')?(c-'a' + 10):((c>='A'&&c<='F')?(c-'A' + 10):0)))
#define byte_from_hex(c1, c2)   ((nybble_from_hex(c1)<<4)|nybble_from_hex(c2))

struct Initializable : public GUID
{
	explicit
		Initializable(char const (&spec)[37])
		: GUID()
	{
		for (int i = 0; i < 8; ++i)
		{
			Data1 = (Data1 << 4) | nybble_from_hex(spec[i]);
		}
		assert(spec[8] == '-');
		for (int i = 9; i < 13; ++i)
		{
			Data2 = (Data2 << 4) | nybble_from_hex(spec[i]);
		}
		assert(spec[13] == '-');
		for (int i = 14; i < 18; ++i)
		{
			Data3 = (Data3 << 4) | nybble_from_hex(spec[i]);
		}
		assert(spec[18] == '-');
		for (int i = 19; i < 23; i += 2)
		{
			Data4[(i - 19) / 2] = (nybble_from_hex(spec[i]) << 4) | nybble_from_hex(spec[i + 1]);
		}
		assert(spec[23] == '-');
		for (int i = 24; i < 36; i += 2)
		{
			Data4[2 + (i - 24) / 2] = (nybble_from_hex(spec[i]) << 4) | nybble_from_hex(spec[i + 1]);
		}
	}
};

template<class T>
inline
auto __my_uuidof()
-> GUID const &
{
	return GUID_NULL;
}

#define CPPX_GNUC_UUID_FOR( name, spec )            \
template<>                                          \
inline                                              \
auto __my_uuidof<name>()                            \
    -> GUID const&                                  \
{                                                   \
    using ::operator"" _uuid;                       \
    static GUID the_uuid = spec ## _uuid;           \
                                                    \
    return the_uuid;                                \
}                                                   \
                                                    \
template<>                                          \
inline                                              \
auto __my_uuidof<name*>()                           \
    -> GUID const&                                  \
{ return __my_uuidof<name>(); }                     \
                                                    \
static_assert( true, "" )

inline auto operator"" _uuid(char const* const s, size_t const size)
-> GUID
{
	return Initializable(reinterpret_cast<char const (&)[37]>(*s));
}

#define CPPX_UUID_FOR    CPPX_GNUC_UUID_FOR

#define __uuid(T)       __my_uuidof<T>()
#define __uuidof		__uuid

#define DEFINE_COMIF_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        EXTERN_C _declspec(selectany) const GUID IID_##name ATTRIBUTE_WEAK \
        = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } };\
		DEFINE_UUIDOF(name)

#endif

