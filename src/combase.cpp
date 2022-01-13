#include "platcomm.h"
#include "combase.h"

CComUnknown::CComUnknown() : m_cRef(0), m_pUnknown(nullptr)
{
	m_pUnknown = reinterpret_cast<IUnknown*>(static_cast<INonDelegatingCOMUnknown*>(this));
}

HRESULT CComUnknown::NonDelegatingQueryInterface(REFIID uuid, void** ppvObj)
{
	if (uuid == __uuidof(IUnknown))
	{
		IUnknown* Unknown = reinterpret_cast<IUnknown*>(static_cast<INonDelegatingCOMUnknown*>(this));
		AMP_SAFEASSIGN(ppvObj, Unknown);
		Unknown->AddRef();
		return S_OK;
	}
	else
	{
		*ppvObj = NULL;
		return E_NOINTERFACE;
	}
}

ULONG CComUnknown::NonDelegatingAddRef()
{
#ifdef _DEBUG
	LONG lRef = InterlockedIncrement(&m_cRef);
	assert(lRef > 0);
#else
	InterlockedIncrement(&m_cRef);
#endif
	return m_cRef;
}

ULONG CComUnknown::NonDelegatingRelease()
{
	LONG lRef = InterlockedDecrement(&m_cRef);
	assert(lRef >= 0);

	if (lRef == 0)
	{
		m_cRef++;
		delete this;
		return int(0);
	}

	return lRef;
}