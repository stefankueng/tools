// StExBar - an explorer toolbar

// Copyright (C) 2007-2010, 2012, 2020 - Stefan Kueng

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
#include "OnOutOfScope.h"
#include <regex>

bool CDeskBand::Filter(LPTSTR filter)
{
    bool              bReturn = false;
    IServiceProvider* pServiceProvider;
    if (SUCCEEDED(m_pSite->QueryInterface(IID_IServiceProvider, (LPVOID*)&pServiceProvider)))
    {
        IShellBrowser* pShellBrowser;
        if (SUCCEEDED(pServiceProvider->QueryService(SID_SShellBrowser, IID_IShellBrowser, (LPVOID*)&pShellBrowser)))
        {
            IShellView* pShellView;
            if (SUCCEEDED(pShellBrowser->QueryActiveShellView(&pShellView)))
            {
                IFolderView* pFolderView;
                if (SUCCEEDED(pShellView->QueryInterface(IID_IFolderView, (LPVOID*)&pFolderView)))
                {
                    // hooray! we got the IFolderView interface!
                    // that means the explorer is active and well :)
                    IShellFolderView* pShellFolderView;
                    if (SUCCEEDED(pShellView->QueryInterface(IID_IShellFolderView, (LPVOID*)&pShellFolderView)))
                    {
                        // the first thing we do is to deselect all already selected entries
                        pFolderView->SelectItem(NULL, SVSI_DESELECTOTHERS);

                        // but we also need the IShellFolder interface because
                        // we need its GetDisplayNameOf() method
                        IPersistFolder2* pPersistFolder;
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
                                for (size_t i = 0; i < m_noShows.size(); ++i)
                                {
                                    CoTaskMemFree(m_noShows[i]);
                                }
                                m_noShows.clear();
                            }
                            IShellFolder* pShellFolder;
                            if (SUCCEEDED(pPersistFolder->QueryInterface(IID_IShellFolder, (LPVOID*)&pShellFolder)))
                            {
                                // our next task is to enumerate all the
                                // items in the folder view and select those
                                // which match the text in the edit control

                                bool bUseRegex = (filter[0] == '\\');

                                try
                                {
                                    const std::wregex regCheck(&filter[1], std::regex_constants::icase | std::regex_constants::ECMAScript);
                                }
                                catch (std::exception)
                                {
                                    bUseRegex = false;
                                }

                                if (!bUseRegex)
                                {
                                    // force the filter to lowercase
                                    wchar_t* pString = filter;
                                    while (*pString)
                                    {
                                        *pString = towlower(*pString);
                                        pString++;
                                    }
                                }

                                int nCount = 0;
                                if (SUCCEEDED(pFolderView->ItemCount(SVGIO_ALLVIEW, &nCount)))
                                {
                                    pShellFolderView->SetRedraw(FALSE);
                                    HWND    listView = GetListView32(pShellView);
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
                                    noShows.reserve(nCount);
                                    for (int i = 0; i < nCount; ++i)
                                    {
                                        LPITEMIDLIST pidl;
                                        if (SUCCEEDED(pFolderView->Item(i, &pidl)))
                                        {
                                            if (CheckDisplayName(pShellFolder, pidl, filter, bUseRegex))
                                            {
                                                // remove now shown items which are in the no-show list
                                                // this is necessary since we don't get a notification
                                                // if the shell refreshes its view
                                                for (std::vector<LPITEMIDLIST>::iterator it = m_noShows.begin(); it != m_noShows.end(); ++it)
                                                {
                                                    if (HRESULT_CODE(pShellFolder->CompareIDs(SHCIDS_CANONICALONLY, *it, pidl)) == 0)
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
                                    for (size_t i = 0; i < m_noShows.size(); ++i)
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
                                    m_noShows.insert(
                                        m_noShows.end(),
                                        std::make_move_iterator(noShows.begin()),
                                        std::make_move_iterator(noShows.end()));
                                    noShows.clear();
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

bool CDeskBand::CheckDisplayName(IShellFolder* shellFolder, LPITEMIDLIST pidl, LPCTSTR filter, bool bUseRegex)
{
    if (filter == nullptr || filter[0] == 0)
        return true;
    STRRET str;
    if (SUCCEEDED(shellFolder->GetDisplayNameOf(pidl,
                                                // SHGDN_FORPARSING needed to get the extensions even if they're not shown
                                                SHGDN_INFOLDER | SHGDN_FORPARSING,
                                                &str)))
    {
        OnOutOfScope(CoTaskMemFree(str.pOleStr));
        if (bUseRegex)
        {
            try
            {
                const std::wregex regCheck(&filter[1], std::regex_constants::icase | std::regex_constants::ECMAScript);
                std::wstring      s = str.pOleStr;

                return std::regex_search(s, regCheck);
            }
            catch (std::exception)
            {
            }
        }
        else
        {
            // check if the item name matches the text of the edit control
            return (StrStrIW(str.pOleStr, filter) != NULL);
        }
    }
    return false;
}
