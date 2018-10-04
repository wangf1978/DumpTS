#pragma once

#ifdef _WIN32
#include <Windows.h>
#include <Unknwn.h>
#endif

#if defined(_MSC_VER) && _MSC_VER>=1100
#define AMP_NOVTABLE __declspec(novtable)
#else
#define AMP_NOVTABLE
#endif

#ifndef _WIN32
#ifndef BASETYPES
#define BASETYPES
typedef unsigned long ULONG;
typedef ULONG *PULONG;
typedef unsigned short USHORT;
typedef USHORT *PUSHORT;
typedef unsigned char UCHAR;
typedef UCHAR *PUCHAR;
#endif  /* !BASETYPES */

#ifndef _HRESULT_DEFINED
#define _HRESULT_DEFINED
typedef long HRESULT;
#endif // !_HRESULT_DEFINED

#ifndef _REFIID_DEFINED
#define _REFIID_DEFINED
#ifdef __cplusplus
#define REFIID const IID &
#else
#define REFIID const IID * __MIDL_CONST
#endif
#endif

class IUnknown
{
public:
	virtual HRESULT QueryInterface(/* [in] */ REFIID riid, /* [iid_is][out] */void **ppvObject) = 0;
	virtual ULONG	AddRef(void) = 0;
	virtual ULONG	Release(void) = 0;
};

#ifndef GUID_DEFINED
#define GUID_DEFINED
#if defined(__midl)
typedef struct {
	unsigned long	Data1;
	unsigned short	Data2;
	unsigned short	Data3;
	byte			Data4[8];
} GUID;
#else
typedef struct _GUID {
	unsigned long	Data1;
	unsigned short	Data2;
	unsigned short	Data3;
	unsigned char	Data4[8];
} GUID;
#endif
#endif

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef GUID IID;
typedef IID *LPIID;
#define IID_NULL            GUID_NULL
#define IsEqualIID(riid1, riid2) IsEqualGUID(riid1, riid2)
typedef GUID CLSID;
typedef CLSID *LPCLSID;
#define CLSID_NULL          GUID_NULL
#define IsEqualCLSID(rclsid1, rclsid2) IsEqualGUID(rclsid1, rclsid2)
typedef GUID FMTID;
typedef FMTID *LPFMTID;
#define FMTID_NULL          GUID_NULL
#define IsEqualFMTID(rfmtid1, rfmtid2) IsEqualGUID(rfmtid1, rfmtid2)

#ifdef __midl_proxy
#define __MIDL_CONST
#else
#define __MIDL_CONST const
#endif

#ifndef _REFGUID_DEFINED
#define _REFGUID_DEFINED
#ifdef __cplusplus
#define REFGUID const GUID &
#else
#define REFGUID const GUID * __MIDL_CONST
#endif
#endif

#ifndef _REFIID_DEFINED
#define _REFIID_DEFINED
#ifdef __cplusplus
#define REFIID const IID &
#else
#define REFIID const IID * __MIDL_CONST
#endif
#endif

#ifndef _REFCLSID_DEFINED
#define _REFCLSID_DEFINED
#ifdef __cplusplus
#define REFCLSID const IID &
#else
#define REFCLSID const IID * __MIDL_CONST
#endif
#endif

#ifndef _REFFMTID_DEFINED
#define _REFFMTID_DEFINED
#ifdef __cplusplus
#define REFFMTID const IID &
#else
#define REFFMTID const IID * __MIDL_CONST
#endif
#endif

#endif // !__IID_DEFINED__

#endif

#define DECLARE_IUNKNOWN													\
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj){				\
        return GetCOMOwner()->QueryInterface(riid, ppvObj);					\
    }																		\
    virtual ULONG STDMETHODCALLTYPE AddRef() {												\
        return GetCOMOwner()->AddRef();										\
    }																		\
    virtual ULONG STDMETHODCALLTYPE Release() {												\
        return GetCOMOwner()->Release();									\
    }

#ifdef __linux__
#define ATTRIBUTE_WEAK __attribute__ ((weak))
#else
#define ATTRIBUTE_WEAK
#endif

#define AMP_DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
		EXTERN_C _declspec(selectany) const GUID FAR name ATTRIBUTE_WEAK \
        = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

// {00000000-0000-0000-C000-000000000046}
AMP_DEFINE_GUID(IID_IUnknown, 
	0x00000000, 0x0000, 0x0000, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46);
// {00000000-0000-0000-0000-000000000000}
AMP_DEFINE_GUID(GUID_NULL,
	0x00000000, 0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
// {2451C6D7-C87C-4644-9EBF-A68BE8529183}
AMP_DEFINE_GUID(IID_IBox ,
	0x2451c6d7, 0xc87c, 0x4644, 0x9e, 0xbf, 0xa6, 0x8b, 0xe8, 0x52, 0x91, 0x83);
// {E5788C97-95CF-440F-8896-7F63F44C0411}
AMP_DEFINE_GUID(IID_IEBMLElement ,
	0xe5788c97, 0x95cf, 0x440f, 0x88, 0x96, 0x7f, 0x63, 0xf4, 0x4c, 0x04, 0x11);


#ifndef INONDELEGATINGUNKNOWN_DEFINED
#define INONDELEGATINGUNKNOWN_DEFINED
/*!	@brief Define a non-delegation IUnknown interface for aggregation. */
class INonDelegatingCOMUnknown
{
public:
	virtual HRESULT NonDelegatingQueryInterface(REFIID riid, void **ppvObj) = 0;
	virtual ULONG	NonDelegatingAddRef() = 0;
	virtual ULONG	NonDelegatingRelease() = 0;
};
#endif

/*!	@brief The utility function for IProcUnknown I/F implementation. */
class AMP_NOVTABLE CComUnknown : public INonDelegatingCOMUnknown
{
public:
	CComUnknown();
	virtual ~CComUnknown() {};

	IUnknown* GetCOMOwner() const							/* Return the owner of this object */
	{
		return (IUnknown*)m_pUnknown;
	}

	virtual HRESULT NonDelegatingQueryInterface(REFIID uuid, void** ppvObj);
	virtual ULONG	NonDelegatingAddRef();
	virtual ULONG	NonDelegatingRelease();

protected:
	volatile long	m_cRef;								/* Number of reference counts */

private:
	const IUnknown* m_pUnknown;							/* Owner of this object */
};

inline HRESULT GetCOMInterface(IUnknown* pUnk, void **ppv)
{
	*ppv = pUnk;
	pUnk->AddRef();
	return S_OK;
}