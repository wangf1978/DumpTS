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

#define DECLARE_IUNKNOWN															\
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj){	\
        return GetCOMOwner()->QueryInterface(riid, ppvObj);							\
    }																				\
    virtual ULONG STDMETHODCALLTYPE AddRef() {										\
        return GetCOMOwner()->AddRef();												\
    }																				\
    virtual ULONG STDMETHODCALLTYPE Release() {										\
        return GetCOMOwner()->Release();											\
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

/*
	For NAL bitstream parser
*/
// {8BAC62DB-6941-403D-BBC6-D38BE9F63A82}
AMP_DEFINE_GUID(IID_INALContext,
	0x8bac62db, 0x6941, 0x403d, 0xbb, 0xc6, 0xd3, 0x8b, 0xe9, 0xf6, 0x3a, 0x82);
// {F41EECDD-8FB2-43E4-AD47-95FD6CC3B532}
AMP_DEFINE_GUID(IID_INALAVCContext,
	0xf41eecdd, 0x8fb2, 0x43e4, 0xad, 0x47, 0x95, 0xfd, 0x6c, 0xc3, 0xb5, 0x32);
// {25CFDB16-8AF5-4C38-ABE4-7EA836B2E009}
AMP_DEFINE_GUID(IID_INALHEVCContext,
	0x25cfdb16, 0x8af5, 0x4c38, 0xab, 0xe4, 0x7e, 0xa8, 0x36, 0xb2, 0xe0, 0x09);
// {62DDA014-4209-4487-A296-70926C422B3C}
AMP_DEFINE_GUID(IID_INALVVCContext,
	0x62dda014, 0x4209, 0x4487, 0xa2, 0x96, 0x70, 0x92, 0x6c, 0x42, 0x2b, 0x3c);
// {0EEB4D5B-CBAE-442C-9E1D-CFBE88244397}
AMP_DEFINE_GUID(IID_IMP4AACContext,
	0x0eeb4d5b, 0xcbae, 0x442c, 0x9e, 0x1d, 0xcf, 0xbe, 0x88, 0x24, 0x43, 0x97);
// {2293ABC1-911C-40C4-9EFA-1B3F5D24B3A9}
AMP_DEFINE_GUID(IID_IMP2AACContext,
	0x2293abc1, 0x911c, 0x40c4, 0x9e, 0xfa, 0x1b, 0x3f, 0x5d, 0x24, 0xb3, 0xa9);
// {94E873E9-77F2-40DD-BC65-588C2CB3BE72}
AMP_DEFINE_GUID(IID_IMPVContext,
	0x94e873e9, 0x77f2, 0x40dd, 0xbc, 0x65, 0x58, 0x8c, 0x2c, 0xb3, 0xbe, 0x72);
// {92DA4999-CF6F-4E76-B556-B5A8BFDD3EBF}
AMP_DEFINE_GUID(IID_IAV1Context,
	0x92da4999, 0xcf6f, 0x4e76, 0xb5, 0x56, 0xb5, 0xa8, 0xbf, 0xdd, 0x3e, 0xbf);
// {40C079B5-49C7-40D2-A767-D4AD878835EA}
AMP_DEFINE_GUID(IID_INALEnumerator,
	0x40c079b5, 0x49c7, 0x40d2, 0xa7, 0x67, 0xd4, 0xad, 0x87, 0x88, 0x35, 0xea);
// {F129FDBB-1B71-4D3B-B679-A71AE6233F77}
AMP_DEFINE_GUID(IID_IAV1Enumerator,
	0xf129fdbb, 0x1b71, 0x4d3b, 0xb6, 0x79, 0xa7, 0x1a, 0xe6, 0x23, 0x3f, 0x77);
// {2E228374-69E0-4F99-99E7-CFD3C0E7D621}
AMP_DEFINE_GUID(IID_IMPVEnumerator,
	0x2e228374, 0x69e0, 0x4f99, 0x99, 0xe7, 0xcf, 0xd3, 0xc0, 0xe7, 0xd6, 0x21);
// {3826630E-0B23-4D22-8112-00FE222CF7F3}
AMP_DEFINE_GUID(IID_IAUEnumerator,
	0x3826630e, 0x0b23, 0x4d22, 0x81, 0x12, 0x00, 0xfe, 0x22, 0x2c, 0xf7, 0xf3);
// {3BE0E7A2-1AC6-4F8E-A106-FB70A135C3FD}
AMP_DEFINE_GUID(IID_IMSEParser,
	0x3be0e7a2, 0x1ac6, 0x4f8e, 0xa1, 0x06, 0xfb, 0x70, 0xa1, 0x35, 0xc3, 0xfd);
// {4D104B3F-EFDD-4C0D-A352-EC842DCEB575}
AMP_DEFINE_GUID(IID_ILOASEnumerator,
	0x4d104b3f, 0xefdd, 0x4c0d, 0xa3, 0x52, 0xec, 0x84, 0x2d, 0xce, 0xb5, 0x75);



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
