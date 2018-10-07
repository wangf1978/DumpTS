#pragma once

#if !defined(_WIN32)
#include <minwindef.h>
#endif
#include <basetyps.h>
#include <wtypes.h>
#include <Unknwn.h>

#if defined(_MSC_VER) && _MSC_VER>=1100
#define AMP_NOVTABLE __declspec(novtable)
#else
#define AMP_NOVTABLE
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

#if !defined(_WIN32)
#define ATTRIBUTE_WEAK __attribute__ ((weak))
#else
#define ATTRIBUTE_WEAK
#endif

#define AMP_DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
		EXTERN_C _declspec(selectany) const GUID FAR name ATTRIBUTE_WEAK \
        = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

#if defined(_WIN32)
// {00000000-0000-0000-C000-000000000046}
AMP_DEFINE_GUID(IID_IUnknown, 
	0x00000000, 0x0000, 0x0000, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46);
// {00000000-0000-0000-0000-000000000000}
AMP_DEFINE_GUID(GUID_NULL,
	0x00000000, 0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
#endif
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

	IUnknown* GetCOMOwner() const						/* Return the owner of this object */
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
