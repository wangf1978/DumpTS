/* platform-independent definition of IUnknown and IClassFactory */

#ifndef __VA_UNKNOWN_H__
#define __VA_UNKNOWN_H__

/* Forward Declarations */
#ifndef __IUnknown_FWD_DEFINED__
#define __IUnknown_FWD_DEFINED__
typedef interface IUnknown IUnknown;
#endif 	/* __IUnknown_FWD_DEFINED__ */


#ifndef __IClassFactory_FWD_DEFINED__
#define __IClassFactory_FWD_DEFINED__
typedef interface IClassFactory IClassFactory;
#endif 	/* __IClassFactory_FWD_DEFINED__ */

#ifndef __IUnknown_INTERFACE_DEFINED__
#define __IUnknown_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IUnknown
 * at Thu Sep 11 10:57:03 1997
 * using MIDL 3.03.0110
 ****************************************/
/* [unique][uuid][object][local] */


typedef /* [unique] */ IUnknown __RPC_FAR *LPUNKNOWN;

//////////////////////////////////////////////////////////////////
// IID_IUnknown and all other system IIDs are provided in UUID.LIB
// Link that library in with your proxies, clients and servers
//////////////////////////////////////////////////////////////////
#ifdef __WIN32
EXTERN_C const IID IID_IUnknown;
#else
DEFINE_GUID(IID_IUnknown, 0x00000000, 0x0000, 0x0000, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46);
#endif

    MIDL_INTERFACE("00000000-0000-0000-C000-000000000046")
	IUnknown
    {
    public:
		static const GUID s_guid;

        BEGIN_INTERFACE
        virtual HRESULT STDMETHODCALLTYPE QueryInterface(
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject) = 0;

        virtual ULONG STDMETHODCALLTYPE AddRef( void) = 0;

        virtual ULONG STDMETHODCALLTYPE Release( void) = 0;

        END_INTERFACE
    };

#endif 	/* __IUnknown_INTERFACE_DEFINED__ */


#ifndef __IClassFactory_INTERFACE_DEFINED__
#define __IClassFactory_INTERFACE_DEFINED__

typedef /* [unique] */ IClassFactory __RPC_FAR *LPCLASSFACTORY;

#ifdef _WIN32
EXTERN_C const IID IID_IClassFactory;
#else
DEFINE_GUID(IID_IClassFactory, 0x00000001, 0x0000, 0x0000, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46);
#endif

    MIDL_INTERFACE("00000001-0000-0000-C000-000000000046")
    IClassFactory : public IUnknown
    {
    public:
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE CreateInstance( 
            /* [unique][in] */ IUnknown __RPC_FAR *pUnkOuter,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject) = 0;
        
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE LockServer( 
            /* [in] */ BOOL fLock) = 0;
        
    };

#endif 	/* __IClassFactory_INTERFACE_DEFINED__ */

#endif  /* __VA_UNKNOWN_H__ */ 

