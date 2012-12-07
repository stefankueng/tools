// StExBar - an explorer toolbar

// Copyright (C) 2007-2010, 2012 - Stefan Kueng

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
#include "SRBand.h"
#include "ItemIDList.h"
#include "VistaIcons.h"
#include "resource.h"

#define GetPIDLFolder(pida)  (LPCITEMIDLIST)(((LPBYTE)pida)+(pida)->aoffset[0])
#define GetPIDLItem(pida, i) (LPCITEMIDLIST)(((LPBYTE)pida)+(pida)->aoffset[i+1])

int         g_shellidlist   = RegisterClipboardFormat(CFSTR_SHELLIDLIST);
LPCTSTR     g_MenuIDString  = _T("StEx");

STDMETHODIMP CDeskBand::Initialize(LPCITEMIDLIST pIDFolder,
                                   LPDATAOBJECT pDataObj,
                                   HKEY /* hRegKey */)
{
    m_ContextItems.clear();
    m_ContextDirectory.clear();
    // get selected files/folders
    if (pDataObj)
    {
        STGMEDIUM medium;
        FORMATETC fmte = {(CLIPFORMAT)g_shellidlist,
            (DVTARGETDEVICE FAR *)NULL,
            DVASPECT_CONTENT,
            -1,
            TYMED_HGLOBAL};
        HRESULT hres = pDataObj->GetData(&fmte, &medium);

        if (SUCCEEDED(hres) && medium.hGlobal)
        {
            //Enumerate PIDLs which the user has selected
            CIDA* cida = (CIDA*)GlobalLock(medium.hGlobal);
            ItemIDList parent( GetPIDLFolder (cida));

            int count = cida->cidl;
            for (int i = 0; i < count; ++i)
            {
                ItemIDList child (GetPIDLItem (cida, i), &parent);
                tstring str = child.toString();
                if (str.empty() == false)
                {
                    m_ContextItems[str] = ENABLED_VIEWPATH|ENABLED_FOLDERSELECTED|ENABLED_FILESELECTED;
                }
            }
            GlobalUnlock(medium.hGlobal);

            ReleaseStgMedium ( &medium );
            if (medium.pUnkForRelease)
            {
                IUnknown* relInterface = (IUnknown*)medium.pUnkForRelease;
                relInterface->Release();
            }
        }
    }

    // get folder background
    if (pIDFolder)
    {
        ItemIDList list(pIDFolder);
        m_ContextDirectory = list.toString();
    }

    return NOERROR;
}

STDMETHODIMP CDeskBand::QueryContextMenu(HMENU hMenu,
                                         UINT indexMenu,
                                         UINT idCmdFirst,
                                         UINT /*idCmdLast*/,
                                         UINT uFlags)
{
    if ((uFlags & CMF_DEFAULTONLY)!=0)
        return S_OK;                    //we don't change the default action

    if (((uFlags & 0x000f)!=CMF_NORMAL)&&(!(uFlags & CMF_EXPLORE))&&(!(uFlags & CMF_VERBSONLY)))
        return S_OK;

    if ((m_ContextDirectory.empty()) && (m_ContextItems.empty()))
        return S_OK;


    if (m_ContextDirectory.empty())
    {
        // folder is empty, but maybe files are selected
        if (m_ContextItems.empty())
            return S_OK;    // nothing selected - we don't have a menu to show
        // check whether a selected entry is an UID - those are namespace extensions
        // which we can't handle
        for (std::map<tstring, ULONG>::const_iterator it = m_ContextItems.begin(); it != m_ContextItems.end(); ++it)
        {
            if (_tcsncmp(it->first.c_str(), _T("::{"), 3)==0)
                return S_OK;
        }
    }
    else
    {
        // ignore namespace extensions
        if (_tcsncmp(m_ContextDirectory.c_str(), _T("::{"), 3)==0)
            return S_OK;
    }

    if (DWORD(CRegStdDWORD(_T("Software\\StefansTools\\StExBar\\ContextMenu"), TRUE)) == FALSE)
        return S_OK;

    //check if we already added our menu entry for a folder.
    //we check that by iterating through all menu entries and check if
    //the dwItemData member points to our global ID string. That string is set
    //by our shell extension when the folder menu is inserted.
    TCHAR menubuf[MAX_PATH];
    int count = GetMenuItemCount(hMenu);
    for (int i=0; i<count; ++i)
    {
        MENUITEMINFO miif;
        SecureZeroMemory(&miif, sizeof(MENUITEMINFO));
        miif.cbSize = sizeof(MENUITEMINFO);
        miif.fMask = MIIM_DATA;
        miif.dwTypeData = menubuf;
        miif.cch = _countof(menubuf);
        GetMenuItemInfo(hMenu, i, TRUE, &miif);
        if (miif.dwItemData == (ULONG_PTR)g_MenuIDString)
            return S_OK;
    }

    OSVERSIONINFOEX inf;
    SecureZeroMemory(&inf, sizeof(OSVERSIONINFOEX));
    inf.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    GetVersionEx((OSVERSIONINFO *)&inf);
    WORD fullver = MAKEWORD(inf.dwMinorVersion, inf.dwMajorVersion);

    UINT idCmd = idCmdFirst;

    //create the sub menu
    HMENU subMenu = CreateMenu();
    int indexSubMenu = 0;


    m_commands.LoadFromFile();

    int index = 0;
    for (int j = 0; j < m_commands.GetCount(); ++j)
    {
        MENUITEMINFO menuiteminfo = {0};

        Command cmd = m_commands.GetCommand(j);
        m_hotkeys[cmd.key] = j;
        if ((cmd.commandline.compare(INTERNALCOMMANDHIDDEN)==0)&&(cmd.name.compare(_T("Options")) == 0))
        {
            cmd.commandline = INTERNALCOMMAND;  // make sure the options button is never hidden.
            m_commands.SetCommand(j, cmd);
        }
        if ((cmd.name.compare(_T("StexBar Internal Edit Box")) == 0)||
            (cmd.commandline.compare(INTERNALCOMMANDHIDDEN) == 0)||
            (cmd.name.compare(_T("New Folder")) == 0))
        {
            continue;
        }
        bool bEnabled = cmd.enabled_viewpath ||
                        (cmd.enabled_fileselected && !m_ContextItems.empty()) ||
                        (cmd.enabled_folderselected && (!m_ContextItems.empty() || !m_ContextDirectory.empty())) ||
                        (cmd.enabled_noselection && (m_ContextItems.empty())) ||
                        (cmd.enabled_selectedcount && (cmd.enabled_selectedcount == (int)!m_ContextItems.empty()));

        HICON hIcon = LoadCommandIcon(cmd);
        if (hIcon)
        {
            menuiteminfo.hbmpItem = (fullver >= 0x600) ? IconToBitmapPARGB32(hIcon) : HBMMENU_CALLBACK;
            DestroyIcon(hIcon);
        }
        else
            menuiteminfo.hbmpItem = NULL;
        if (!cmd.separator)
            m_tooltips[j] = cmd.name.c_str();

        myIDMap[idCmd - idCmdFirst] = j;
        myIDMap[idCmd] = j;

        menuiteminfo.cbSize = sizeof(menuiteminfo);
        menuiteminfo.fMask = cmd.separator ? MIIM_FTYPE : MIIM_FTYPE | MIIM_ID | MIIM_BITMAP | MIIM_STRING | MIIM_STATE;
        menuiteminfo.fType = cmd.separator ? MFT_SEPARATOR : MFT_STRING;
        menuiteminfo.fState = bEnabled ? MFS_ENABLED : MFS_DISABLED;
        TCHAR menutextbuf[100];
        _tcscpy_s(menutextbuf, 100, m_commands.GetCommandPtr(j)->name.c_str());
        menuiteminfo.dwTypeData = menutextbuf;
        menuiteminfo.wID = idCmd++;
        InsertMenuItem(subMenu, indexSubMenu++, TRUE, &menuiteminfo);
        index++;
    }

    //add sub menu to main context menu
    //don't use InsertMenu because this will lead to multiple menu entries in the explorer file menu.
    //see http://support.microsoft.com/default.aspx?scid=kb;en-us;214477 for details of that.
    MENUITEMINFO menuiteminfo = {0};
    SecureZeroMemory(&menuiteminfo, sizeof(menuiteminfo));
    menuiteminfo.cbSize = sizeof(menuiteminfo);
    menuiteminfo.fMask = MIIM_FTYPE | MIIM_ID | MIIM_SUBMENU | MIIM_DATA | MIIM_STRING;
    menuiteminfo.fType = MFT_STRING;
    menuiteminfo.dwTypeData = _T("StEx");

    menuiteminfo.hSubMenu = subMenu;
    menuiteminfo.wID = idCmd++;
    InsertMenuItem(hMenu, indexMenu++, TRUE, &menuiteminfo);

    //return number of menu items added
    return ResultFromScode(MAKE_SCODE(SEVERITY_SUCCESS, 0, (USHORT)(idCmd - idCmdFirst)));
}


// This is called when you invoke a command on the menu:
STDMETHODIMP CDeskBand::InvokeCommand(LPCMINVOKECOMMANDINFO lpcmi)
{
    HRESULT hr = E_INVALIDARG;
    if (lpcmi == NULL)
        return hr;

    if ((!m_ContextDirectory.empty()) || (!m_ContextItems.empty()))
    {
        UINT_PTR idCmd = LOWORD(lpcmi->lpVerb);

        // See if we have a handler interface for this id
        std::map<UINT_PTR, UINT_PTR>::const_iterator id_it = myIDMap.lower_bound(idCmd);
        if (id_it != myIDMap.end() && id_it->first == idCmd)
        {
            if (id_it->second >= (UINT_PTR)m_commands.GetCount())
                DebugBreak();
            Command cmd = m_commands.GetCommand((int)id_it->second);

            if (m_ContextDirectory.empty())
            {
                if (m_ContextItems.size() == 1)
                {
                    if (PathIsDirectory(m_ContextItems.begin()->first.c_str()))
                    {
                        m_ContextDirectory = m_ContextItems.begin()->first;
                    }
                }
                if (m_ContextDirectory.empty())
                    m_ContextDirectory = m_currentDirectory;
            }

            HandleCommand(lpcmi->hwnd, cmd, m_ContextDirectory, m_ContextItems);

            myIDMap.clear();
            hr = S_OK;
        }
    }

    return hr;
}

// This is for the status bar and things like that:
STDMETHODIMP CDeskBand::GetCommandString(UINT_PTR idCmd,
                                         UINT uFlags,
                                         UINT FAR * /*reserved*/,
                                         LPSTR pszName,
                                         UINT cchMax)
{
    HRESULT hr = E_INVALIDARG;
    //do we know the id?
    std::map<UINT_PTR, UINT_PTR>::const_iterator id_it = myIDMap.lower_bound(idCmd);
    if (id_it == myIDMap.end() || id_it->first != idCmd)
    {
        return hr;      //no, we don't
    }

    if (m_tooltips.find((int)id_it->first) == m_tooltips.end())
        return hr;


    const TCHAR * desc = m_tooltips[(int)id_it->first].c_str();
    switch(uFlags)
    {
    case GCS_HELPTEXTW:
        {
            std::wstring help = desc;
            lstrcpynW((LPWSTR)pszName, help.c_str(), cchMax);
            hr = S_OK;
            break;
        }
    }
    return hr;
}

STDMETHODIMP CDeskBand::HandleMenuMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT res;
    return HandleMenuMsg2(uMsg, wParam, lParam, &res);
}

STDMETHODIMP CDeskBand::HandleMenuMsg2(UINT uMsg, WPARAM /*wParam*/, LPARAM lParam, LRESULT *pResult)
{
    LRESULT res;
    if (pResult == NULL)
        pResult = &res;
    *pResult = FALSE;

    switch (uMsg)
    {
    case WM_MEASUREITEM:
        {
            MEASUREITEMSTRUCT* lpmis = (MEASUREITEMSTRUCT*)lParam;
            if (lpmis==NULL)
                break;
            lpmis->itemWidth += 2;
            if (lpmis->itemHeight < 16)
                lpmis->itemHeight = 16;
            *pResult = TRUE;
        }
        break;

    case WM_DRAWITEM:
        {
            DRAWITEMSTRUCT* lpdis = (DRAWITEMSTRUCT*)lParam;
            if ((lpdis==NULL)||(lpdis->CtlType != ODT_MENU))
                return S_OK;        //not for a menu

            int cmdID = (int)myIDMap[lpdis->itemID];
            if (m_commands.GetCount() <= cmdID)
                return S_OK;

            Command cmd = m_commands.GetCommand(cmdID);
            HICON hIcon = LoadCommandIcon(cmd);

            if (hIcon == NULL)
                return S_OK;

            DrawIconEx(lpdis->hDC,
                lpdis->rcItem.left - 16,
                lpdis->rcItem.top + (lpdis->rcItem.bottom - lpdis->rcItem.top - 16) / 2,
                hIcon, 16, 16,
                0, NULL, DI_NORMAL);
            DestroyIcon(hIcon);
            *pResult = TRUE;
        }
        break;

    default:
        return S_OK;
    }

    return S_OK;
}
