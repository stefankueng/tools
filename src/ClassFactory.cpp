// StExBar - an explorer toolbar

// Copyright (C) 2007-2008 - Stefan Kueng

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
#include "stdafx.h"
#include "ClassFactory.h"
#include "Guid.h"

CClassFactory::CClassFactory(CLSID clsid)
{
	m_clsidObject = clsid;
	m_ObjRefCount = 1;
	g_DllRefCount++;
}

CClassFactory::~CClassFactory()
{
	g_DllRefCount--;
}

STDMETHODIMP CClassFactory::QueryInterface(REFIID riid, LPVOID *ppReturn)
{
	*ppReturn = NULL;

	if (IsEqualIID(riid, IID_IUnknown))
	{
		*ppReturn = this;
	}

	else if (IsEqualIID(riid, IID_IClassFactory))
	{
		*ppReturn = (IClassFactory*)this;
	}   

	if (*ppReturn)
	{
		(*(LPUNKNOWN*)ppReturn)->AddRef();
		return S_OK;
	}

	// no interface we know of or implement
	return E_NOINTERFACE;
}                                             

STDMETHODIMP_(DWORD) CClassFactory::AddRef()
{
	return ++m_ObjRefCount;
}

STDMETHODIMP_(DWORD) CClassFactory::Release()
{
	if (--m_ObjRefCount == 0)
	{
		delete this;
		return 0;
	}

	return m_ObjRefCount;
}

STDMETHODIMP CClassFactory::CreateInstance(LPUNKNOWN pUnknown, 
										   REFIID riid, 
										   LPVOID *ppObject)
{
	HRESULT  hResult = E_FAIL;
	LPVOID   pTemp = NULL;

	*ppObject = NULL;

	if (pUnknown != NULL)
		return CLASS_E_NOAGGREGATION;

	// create the requested object
	if (IsEqualCLSID(m_clsidObject, CLSID_StExBand))
	{
		CDeskBand *pDeskBand = new CDeskBand();
		if (NULL == pDeskBand)
			return E_OUTOFMEMORY;

		pTemp = pDeskBand;
	}

	if (pTemp)
	{
		// get the QueryInterface return for our return value
		hResult = ((LPUNKNOWN)pTemp)->QueryInterface(riid, ppObject);

		// call Release to decrement the ref count
		((LPUNKNOWN)pTemp)->Release();
	}

	return hResult;
}

STDMETHODIMP CClassFactory::LockServer(BOOL)
{
	// not implemented
	return E_NOTIMPL;
}
