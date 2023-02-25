// SkTimeStamp - Change file dates easily, directly from explorer

// Copyright (C) 2012, 2023 - Stefan Kueng

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
#include "ShellExt.h"
#include "ShellExtClassFactory.h"

CShellExtClassFactory::CShellExtClassFactory()
{
    m_cRef = 0L;

    g_cRefThisDll++;
}

CShellExtClassFactory::~CShellExtClassFactory()
{
    g_cRefThisDll--;
}

HRESULT __stdcall CShellExtClassFactory::QueryInterface(REFIID      riid,
                                                        LPVOID FAR *ppv)
{
    *ppv = nullptr;

    // Any interface on this object is the object pointer

    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IClassFactory))
    {
        *ppv = static_cast<LPCLASSFACTORY>(this);

        AddRef();

        return NOERROR;
    }

    return E_NOINTERFACE;
}

ULONG __stdcall CShellExtClassFactory::AddRef()
{
    return ++m_cRef;
}

ULONG __stdcall CShellExtClassFactory::Release()
{
    if (--m_cRef)
        return m_cRef;

    delete this;

    return 0L;
}

HRESULT __stdcall CShellExtClassFactory::CreateInstance(LPUNKNOWN pUnkOuter,
                                                        REFIID    riid,
                                                        LPVOID   *ppvObj)
{
    *ppvObj = nullptr;

    // Shell extensions typically don't support aggregation (inheritance)

    if (pUnkOuter)
        return CLASS_E_NOAGGREGATION;

    // Create the main shell extension object.  The shell will then call
    // QueryInterface with IID_IShellExtInit--this is how shell extensions are
    // initialized.

    CShellExt *pShellExt = new CShellExt(); // Create the CShellExt object

    if (nullptr == pShellExt)
        return E_OUTOFMEMORY;

    return pShellExt->QueryInterface(riid, ppvObj);
}

HRESULT __stdcall CShellExtClassFactory::LockServer(BOOL /*fLock*/)
{
    return static_cast<HRESULT>(0x80004001L);
}
