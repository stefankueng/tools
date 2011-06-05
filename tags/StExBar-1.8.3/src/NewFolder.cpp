// StExBar - an explorer toolbar

// Copyright (C) 2007-2008, 2010 - Stefan Kueng

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

bool CDeskBand::CreateNewFolder()
{
    if (m_pSite == NULL)
        return false;

    if (m_currentDirectory.empty())
        return false;

    bool bFolderCreated = false;
    // first step: get a list of all folder items, then create the new folder
    IServiceProvider * pServiceProvider;
    if (SUCCEEDED(m_pSite->QueryInterface(IID_IServiceProvider, (LPVOID*)&pServiceProvider)))
    {
        IShellBrowser * pShellBrowser;
        if (SUCCEEDED(pServiceProvider->QueryService(SID_SShellBrowser, IID_IShellBrowser, (LPVOID*)&pShellBrowser)))
        {
            IShellView * pShellView;
            if (SUCCEEDED(pShellBrowser->QueryActiveShellView(&pShellView)))
            {
                // get the context menu for the shell view background
                IContextMenu * pContextMenu;
                if (SUCCEEDED(pShellView->GetItemObject(SVGIO_BACKGROUND, IID_IContextMenu, (LPVOID*)&pContextMenu)))
                {
                    HMENU hMenu = ::CreateMenu();
                    if (hMenu)
                    {
                        if (SUCCEEDED(pContextMenu->QueryContextMenu(hMenu, 0, 0, 1000, CMF_NORMAL)))
                        {
                            IFolderView * pFolderView;
                            if (SUCCEEDED(pShellView->QueryInterface(IID_IFolderView, (LPVOID*)&pFolderView)))
                            {
                                int nCount = 0;
                                if (SUCCEEDED(pFolderView->ItemCount(SVGIO_ALLVIEW, &nCount)))
                                {
                                    // de-select all entries
                                    for (int i=0; i<nCount; ++i)
                                    {
                                        pFolderView->SelectItem(i, SVSI_DESELECT);
                                        LPITEMIDLIST pidl;
                                        pFolderView->Item(i, &pidl);
                                        m_newfolderPidls.push_back(pidl);
                                    }
                                    m_newfolderTimeoutCounter = 20;
                                    IPersistFolder2 * pPersistFolder;
                                    if (SUCCEEDED(pFolderView->GetFolder(IID_IPersistFolder2, (LPVOID*)&pPersistFolder)))
                                    {
                                        LPITEMIDLIST folderpidl;
                                        if (SUCCEEDED(pPersistFolder->GetCurFolder(&folderpidl)))
                                        {
                                            // we have the current folder
                                            TCHAR buf[MAX_PATH] = {0};
                                            // find the path of the folder
                                            if (SHGetPathFromIDList(folderpidl, buf))
                                            {
                                                // if buf is empty here, that means
                                                // the current directory is a virtual path
                                                if (buf[0])
                                                {
                                                    CMINVOKECOMMANDINFOEX cici = {0};
                                                    cici.cbSize = sizeof(CMINVOKECOMMANDINFOEX);
                                                    cici.lpVerb = CMDSTR_NEWFOLDERA;
                                                    cici.lpVerbW = CMDSTR_NEWFOLDER;
                                                    cici.nShow = SW_SHOWNORMAL;
                                                    cici.hwnd = m_hwndParent;
                                                    cici.fMask = CMIC_MASK_UNICODE | CMIC_MASK_FLAG_NO_UI;
                                                    // create the new folder
                                                    if (SUCCEEDED(pContextMenu->InvokeCommand((CMINVOKECOMMANDINFO*)&cici)))
                                                    {
                                                        // refresh the view
                                                        bFolderCreated = true;
                                                        pShellView->Refresh();
                                                        SHChangeNotify(0, SHCNF_FLUSH, 0, 0);
                                                        // now try to get the new refreshed items. This usually works
                                                        // for local drives where the refresh is fast, but times out
                                                        // on network drives.
                                                        // Note: for network drives, even waiting 10 seconds in the following loop
                                                        // didn't help - seems the window message loop has first to run
                                                        // before such a view really refreshes.
                                                        int nCount2 = 0;
                                                        int timeoutCounter = 100;
                                                        while (((nCount2 == 0)||(nCount2 == nCount)) && (timeoutCounter-- > 0))
                                                        {
                                                            pFolderView->Release();
                                                            pShellView->QueryInterface(IID_IFolderView, (LPVOID*)&pFolderView);
                                                            Sleep(10);
                                                            pFolderView->ItemCount(SVGIO_ALLVIEW, &nCount2);
                                                        }
                                                        // but we also need the IShellFolder interface because
                                                        // we need its CompareIDs() method
                                                        bool bEditing = false;
                                                        IShellFolder * pShellFolder;
                                                        if (SUCCEEDED(pPersistFolder->QueryInterface(IID_IShellFolder, (LPVOID*)&pShellFolder)))
                                                        {
                                                            // now compare the items of the refreshed view with the items
                                                            // of the view before the new folder was created.
                                                            // The difference should be the new folder...
                                                            nCount2 = 0;
                                                            if (SUCCEEDED(pFolderView->ItemCount(SVGIO_ALLVIEW, &nCount2)))
                                                            {
                                                                for (int i=0; i<nCount2; ++i)
                                                                {
                                                                    LPITEMIDLIST pidl;
                                                                    pFolderView->Item(i, &pidl);
                                                                    bool bFound = false;
                                                                    for (std::vector<LPITEMIDLIST>::iterator it = m_newfolderPidls.begin(); it != m_newfolderPidls.end(); ++it)
                                                                    {
                                                                        HRESULT hr = pShellFolder->CompareIDs(0, pidl, *it);
                                                                        if (HRESULT_CODE(hr) == 0)
                                                                        {
                                                                            // this item was there before, so it's not the new folder
                                                                            CoTaskMemFree(*it);
                                                                            m_newfolderPidls.erase(it);
                                                                            bFound = true;
                                                                            break;
                                                                        }
                                                                    }
                                                                    if (!bFound)
                                                                    {
                                                                        // yes, this item wasn't there before, so we assume
                                                                        // it's the new folder and set it into edit mode
                                                                        pShellView->SelectItem(pidl, SVSI_EDIT);
                                                                        bEditing = true;
                                                                    }
                                                                    CoTaskMemFree(pidl);
                                                                }
                                                            }
                                                            if (!bEditing)
                                                            {
                                                                // do nothing for now, since we can't find the new folder
                                                                // which was created. Just let the timer task try again later
                                                            }
                                                            else
                                                            {
                                                                for (std::vector<LPITEMIDLIST>::iterator it = m_newfolderPidls.begin(); it != m_newfolderPidls.end(); ++it)
                                                                {
                                                                    CoTaskMemFree(*it);
                                                                }
                                                            }
                                                            pShellFolder->Release();
                                                        }
                                                    }
                                                }
                                            }
                                            CoTaskMemFree(folderpidl);
                                        }
                                        pPersistFolder->Release();
                                    }
                                    pFolderView->Release();
                                }
                            }
                        }
                        DestroyMenu(hMenu);
                    }
                    pContextMenu->Release();
                }
                pShellView->Release();
            }
            pShellBrowser->Release();
        }
        pServiceProvider->Release();
    }


    return true;
}