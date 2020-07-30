// StExBar - an explorer toolbar

// Copyright (C) 2007-2008, 2020 - Stefan Kueng

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
    STDMETHODIMP_(DWORD)
    AddRef();
    STDMETHODIMP_(DWORD)
    Release();

    // IClassFactory methods
    STDMETHODIMP CreateInstance(LPUNKNOWN, REFIID, LPVOID*);
    STDMETHODIMP LockServer(BOOL);

private:
    CLSID m_clsidObject;
};
