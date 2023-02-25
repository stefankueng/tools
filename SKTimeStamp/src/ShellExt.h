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

#include "Registry.h"
#include <vector>

extern UINT      g_cRefThisDll; // Reference count of this DLL.
extern HINSTANCE g_hmodThisDll; // Instance handle for this DLL

#ifdef _WIN64
// {6A6B7688-3B34-41b2-8487-F1CE4C23FC60}
DEFINE_GUID(CLSID_SKTIMESTAMP,
            0x6a6b7688, 0x3b34, 0x41b2, 0x84, 0x87, 0xf1, 0xce, 0x4c, 0x23, 0xfc, 0x60);
#    define SKTIMESTAMP_GUID L"{6A6B7688-3B34-41b2-8487-F1CE4C23FC60}"
#else
// {01D8AD1E-46C9-4882-925C-CC29D9A99858}
DEFINE_GUID(CLSID_SKTIMESTAMP,
            0x1d8ad1e, 0x46c9, 0x4882, 0x92, 0x5c, 0xcc, 0x29, 0xd9, 0xa9, 0x98, 0x58);
#    define SKTIMESTAMP_GUID L"{01D8AD1E-46C9-4882-925C-CC29D9A99858}"
#endif
// The actual OLE Shell context menu handler
/**
 * \ingroup TortoiseShell
 * The main class of our COM object / Shell Extension.
 * It contains all Interfaces we implement for the shell to use.
 * \remark The implementations of the different interfaces are
 * split into several *.cpp files to keep them in a reasonable size.
 */
class CShellExt : public IShellExtInit
    , IShellPropSheetExt
{
protected:
    ULONG                     m_cRef;
    std::vector<std::wstring> m_files;

public:
    CShellExt();
    virtual ~CShellExt();

    /** \name IUnknown
     * IUnknown members
     */
    //@{
    HRESULT __stdcall QueryInterface(REFIID, LPVOID FAR *) override;
    ULONG __stdcall AddRef() override;
    ULONG __stdcall Release() override;
    //@}

    /** \name IShellExtInit
     * IShellExtInit methods
     */
    //@{
    HRESULT __stdcall Initialize(LPCITEMIDLIST pIDFolder, LPDATAOBJECT pDataObj, HKEY hKeyID) override;
    //@}

    /** \name IShellPropSheetExt
     * IShellPropSheetExt methods
     */
    //@{
    HRESULT __stdcall AddPages(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam) override;
    HRESULT __stdcall ReplacePage(UINT, LPFNADDPROPSHEETPAGE, LPARAM) override;
    //@}
};
