// StExBar - an explorer toolbar

// Copyright (C) 2007-2010 - Stefan Kueng

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
#include "StringUtils.h"
#include <regex>

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
    const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

MIDL_DEFINE_GUID(IID, IID_IShellFolderView,0x37A378C0, 0xF82D, 0x11CE,0xAE,0x65,0x08,0x00,0x2B,0x2E,0x12,0x62);

bool CDeskBand::Filter(LPTSTR filter)
{
    bool bReturn = false;
    IServiceProvider * pServiceProvider;
    if (SUCCEEDED(m_pSite->QueryInterface(IID_IServiceProvider, (LPVOID*)&pServiceProvider)))
    {
        IShellBrowser * pShellBrowser;
        if (SUCCEEDED(pServiceProvider->QueryService(SID_SShellBrowser, IID_IShellBrowser, (LPVOID*)&pShellBrowser)))
        {
            IShellView * pShellView;
            if (SUCCEEDED(pShellBrowser->QueryActiveShellView(&pShellView)))
            {
                IFolderView * pFolderView;
                if (SUCCEEDED(pShellView->QueryInterface(IID_IFolderView, (LPVOID*)&pFolderView)))
                {
                    // hooray! we got the IFolderView interface!
                    // that means the explorer is active and well :)
                    IShellFolderView * pShellFolderView;
                    if (SUCCEEDED(pShellView->QueryInterface(IID_IShellFolderView, (LPVOID*)&pShellFolderView)))
                    {
                        // the first thing we do is to deselect all already selected entries
                        pFolderView->SelectItem(NULL, SVSI_DESELECTOTHERS);

                        // but we also need the IShellFolder interface because
                        // we need its GetDisplayNameOf() method
                        IPersistFolder2 * pPersistFolder;
                        if (SUCCEEDED(pFolderView->GetFolder(IID_IPersistFolder2, (LPVOID*)&pPersistFolder)))
                        {
                            LPITEMIDLIST curFolder;
                            pPersistFolder->GetCurFolder(&curFolder);
                            if (ILIsEqual(m_currentFolder, curFolder))
                            {
                                CoTaskMemFree(curFolder);
                            }
                            else
                            {
                                CoTaskMemFree(m_currentFolder);
                                m_currentFolder = curFolder;
                                for (size_t i=0; i<m_noShows.size(); ++i)
                                {
                                    CoTaskMemFree(m_noShows[i]);
                                }
                                m_noShows.clear();
                            }
                            IShellFolder * pShellFolder;
                            if (SUCCEEDED(pPersistFolder->QueryInterface(IID_IShellFolder, (LPVOID*)&pShellFolder)))
                            {
                                // our next task is to enumerate all the
                                // items in the folder view and select those
                                // which match the text in the edit control

                                bool bUseRegex = (filter[0] == '\\');

                                try
                                {
                                    const tr1::wregex regCheck(&filter[1], tr1::regex_constants::icase | tr1::regex_constants::ECMAScript);
                                }
                                catch (exception)
                                {
                                    bUseRegex = false;
                                }

                                if (!bUseRegex)
                                {
                                    // force the filter to lowercase
                                    TCHAR * pString = filter;
                                    while (*pString)
                                    {
                                        *pString = _totlower(*pString);
                                        pString++;
                                    }
                                }

                                int nCount = 0;
                                if (SUCCEEDED(pFolderView->ItemCount(SVGIO_ALLVIEW, &nCount)))
                                {
                                    pShellFolderView->SetRedraw(FALSE);
                                    HWND listView = GetListView32(pShellView);
                                    LRESULT viewType = 0;
                                    if (listView)
                                    {
                                        // inserting items in the list view if the list view is set to
                                        // e.g., LV_VIEW_LIST is painfully slow. So save the current view
                                        // and set it to LV_VIEW_DETAILS (which is much faster for inserting)
                                        // and restore the view after we're done.
                                        viewType = SendMessage(listView, LVM_GETVIEW, 0, 0);
                                        SendMessage(listView, LVM_SETVIEW, LV_VIEW_DETAILS, 0);
                                    }
                                    std::vector<LPITEMIDLIST> noShows;
                                    for (int i=0; i<nCount; ++i)
                                    {
                                        LPITEMIDLIST pidl;
                                        if (SUCCEEDED(pFolderView->Item(i, &pidl)))
                                        {
                                            if (CheckDisplayName(pShellFolder, pidl, filter, bUseRegex))
                                            {
                                                // remove now shown items which are in the no-show list
                                                // this is necessary since we don't get a notification
                                                // if the shell refreshes its view
                                                for (std::vector<LPITEMIDLIST>::iterator it = m_noShows.begin(); it != m_noShows.end(); ++it )
                                                {
                                                    if (HRESULT_CODE(pShellFolder->CompareIDs(SHCIDS_CANONICALONLY, *it, pidl))==0)
                                                    {
                                                        m_noShows.erase(it);
                                                        break;
                                                    }
                                                }
                                                CoTaskMemFree(pidl);
                                            }
                                            else
                                            {
                                                UINT puItem = 0;
                                                if (pShellFolderView->RemoveObject(pidl, &puItem) == S_OK)
                                                {
                                                    i--;
                                                    nCount--;
                                                    noShows.push_back(pidl);
                                                }
                                            }
                                        }
                                    }
                                    // now add all those items again which were removed by a previous filter string
                                    // but don't match this new one
                                    //pShellFolderView->SetObjectCount(5000, SFVSOC_INVALIDATE_ALL|SFVSOC_NOSCROLL);
                                    for (size_t i=0; i<m_noShows.size(); ++i)
                                    {
                                        LPITEMIDLIST pidlNoShow = m_noShows[i];
                                        if (CheckDisplayName(pShellFolder, pidlNoShow, filter, bUseRegex))
                                        {
                                            m_noShows.erase(m_noShows.begin() + i);
                                            i--;
                                            UINT puItem = (UINT)i;
                                            pShellFolderView->AddObject(pidlNoShow, &puItem);
                                            CoTaskMemFree(pidlNoShow);
                                        }
                                    }
                                    for (size_t i=0; i<noShows.size(); ++i)
                                    {
                                        m_noShows.push_back(noShows[i]);
                                    }
                                    if (listView)
                                    {
                                        SendMessage(listView, LVM_SETVIEW, viewType, 0);
                                    }

                                    pShellFolderView->SetRedraw(TRUE);
                                }
                                pShellFolder->Release();
                            }
                            pPersistFolder->Release();
                        }
                        pShellFolderView->Release();
                    }
                    pFolderView->Release();
                }
                pShellView->Release();
            }
            pShellBrowser->Release();
        }
        pServiceProvider->Release();
    }
    return bReturn;
}

bool CDeskBand::CheckDisplayName(IShellFolder * shellFolder, LPITEMIDLIST pidl, LPCTSTR filter, bool bUseRegex)
{
    STRRET str;
    if (SUCCEEDED(shellFolder->GetDisplayNameOf(pidl,
        // SHGDN_FORPARSING needed to get the extensions even if they're not shown
        SHGDN_INFOLDER|SHGDN_FORPARSING,
        &str)))
    {
        TCHAR dispname[MAX_PATH];
        StrRetToBuf(&str, pidl, dispname, MAX_PATH);

        if (bUseRegex)
        {

            try
            {
                const tr1::wregex regCheck(&filter[1], tr1::regex_constants::icase | tr1::regex_constants::ECMAScript);
                wstring s = dispname;

                return tr1::regex_search(s, regCheck);
            }
            catch (exception)
            {
            }
        }
        else
        {
            // we now have the display name of the item
            // i.e. the way the item is shown
            // since the windows file system is case-insensitive
            // we have to force the display name to lowercase
            // so the filter matches case-insensitive too
            TCHAR * pString = dispname;
            while (*pString)
            {
                *pString = _totlower(*pString);
                pString++;
            }
            // check if the item name matches the text of the edit control
            return (_tcsstr(dispname, filter) != NULL);
        }
    }
    return false;
}
