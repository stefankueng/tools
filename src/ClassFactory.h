#pragma once

#include <windows.h>
#include "Globals.h"
#include "SRBand.h"

/**
 * Class factory pattern.
 * Returns the requested COM-Objects
 */
class CClassFactory : public IClassFactory
{
protected:
	DWORD m_ObjRefCount;

public:
	CClassFactory(CLSID);
	~CClassFactory();

	// IUnknown methods
	STDMETHODIMP QueryInterface(REFIID, LPVOID*);
	STDMETHODIMP_(DWORD) AddRef();
	STDMETHODIMP_(DWORD) Release();

	// IClassFactory methods
	STDMETHODIMP CreateInstance(LPUNKNOWN, REFIID, LPVOID*);
	STDMETHODIMP LockServer(BOOL);

private:
	CLSID m_clsidObject;
};


