// SkTimeStamp - Change file dates easily, directly from explorer

// Copyright (C) 2012 - Stefan Kueng

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

// Initialize GUIDs (should be done only and at-least once per DLL/EXE)
#include <initguid.h>

#include "ShellExt.h"

// *********************** CShellExt *************************
CShellExt::CShellExt()
{
    m_cRef = 0L;
    g_cRefThisDll++;

    INITCOMMONCONTROLSEX used = {
        sizeof(INITCOMMONCONTROLSEX),
            ICC_DATE_CLASSES | ICC_WIN95_CLASSES | ICC_BAR_CLASSES | ICC_USEREX_CLASSES
    };
    InitCommonControlsEx(&used);
}

CShellExt::~CShellExt()
{
    g_cRefThisDll--;
}

STDMETHODIMP CShellExt::QueryInterface(REFIID riid, LPVOID FAR *ppv)
{
    *ppv = NULL;

    if (IsEqualIID(riid, IID_IShellExtInit) || IsEqualIID(riid, IID_IUnknown))
    {
        *ppv = (LPSHELLEXTINIT)this;
    }
    else if (IsEqualIID(riid, IID_IShellPropSheetExt))
    {
        *ppv = (LPSHELLPROPSHEETEXT)this;
    }
    if (*ppv)
    {
        AddRef();

        return NOERROR;
    }

    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CShellExt::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CShellExt::Release()
{
    if (--m_cRef)
        return m_cRef;

    delete this;

    return 0L;
}

// IShellExtInit
STDMETHODIMP CShellExt::Initialize(LPCITEMIDLIST /*pIDFolder*/,
                                   LPDATAOBJECT pDataObj,
                                   HKEY /* hRegKey */)
{
    files_.clear();
    // get selected files/folders
    if (pDataObj)
    {
        STGMEDIUM medium;
        FORMATETC fmte = {CF_HDROP,
            (DVTARGETDEVICE FAR *)NULL,
            DVASPECT_CONTENT,
            -1,
            TYMED_HGLOBAL};
        HRESULT hres = pDataObj->GetData(&fmte, &medium);

        if (SUCCEEDED(hres) && medium.hGlobal)
        {
            FORMATETC etc = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
            STGMEDIUM stg = { TYMED_HGLOBAL };
            if ( FAILED( pDataObj->GetData ( &etc, &stg )))
            {
                ReleaseStgMedium ( &medium );
                return E_INVALIDARG;
            }


            HDROP drop = (HDROP)GlobalLock(stg.hGlobal);
            if ( NULL == drop )
            {
                ReleaseStgMedium ( &stg );
                ReleaseStgMedium ( &medium );
                return E_INVALIDARG;
            }

            int count = DragQueryFile(drop, (UINT)-1, NULL, 0);
            for (int i = 0; i < count; i++)
            {
                // find the path length in chars
                UINT len = DragQueryFile(drop, i, NULL, 0);
                if (len == 0)
                    continue;
                TCHAR * szFileName = new TCHAR[len+1];
                if (0 == DragQueryFile(drop, i, szFileName, len+1))
                {
                    delete [] szFileName;
                    continue;
                }
                std::wstring str = std::wstring(szFileName);
                delete [] szFileName;
                if (str.empty() == false)
                {
                    files_.push_back(str);
                }
            }
            GlobalUnlock ( drop );
            ReleaseStgMedium ( &stg );
        }
    }

    return NOERROR;
}
