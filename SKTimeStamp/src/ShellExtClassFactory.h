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
#pragma once

/**
 * This class factory object creates the main handlers -
 * its constructor says which OLE class it has to make.
 */
class CShellExtClassFactory : public IClassFactory
{
protected:
    ULONG m_cRef;

public:
    CShellExtClassFactory();
    virtual ~CShellExtClassFactory();

    //@{
    /// IUnknown members
    HRESULT __stdcall QueryInterface(REFIID, LPVOID FAR *) override;
    ULONG __stdcall AddRef() override;
    ULONG __stdcall Release() override;
    //@}

    //@{
    /// IClassFactory members
    HRESULT __stdcall CreateInstance(LPUNKNOWN, REFIID, LPVOID FAR *) override;
    HRESULT __stdcall LockServer(BOOL) override;
    //@}
};
