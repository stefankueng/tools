// StExBar - an explorer toolbar

// Copyright (C) 2007-2009, 2012-2013 - Stefan Kueng

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
#include "resource.h"
#include <regex>
#include "RenameDlg.h"
#include "Pidl.h"
#include "NumberReplacer.h"

void CDeskBand::Rename(HWND hwnd, const std::map<std::wstring, ULONG>& items)
{
    // fill the list of selected file/foldernames
    m_filelist.clear();
    if (items.size() > 1)
    {
        for (std::map<std::wstring, ULONG>::const_iterator it = items.begin(); it != items.end(); ++it)
        {
            size_t pos = it->first.find_last_of('\\');
            if (pos != std::wstring::npos)
            {
                m_filelist.insert(it->first.substr(pos+1));
            }
        }
    }
    else if (items.size() == 1)
    {
        for (std::map<std::wstring, ULONG>::const_iterator it = items.begin(); it != items.end(); ++it)
        {
            size_t pos = it->first.find_last_of('\\');
            if (pos != std::wstring::npos)
            {
                m_filelist.insert(it->first.substr(pos+1));
            }
        }
    }
    else
    {
        // no files or only one file were selected.
        // use all files and folders in the current folder instead
        IServiceProvider * pServiceProvider = NULL;
        if (SUCCEEDED(GetIServiceProvider(hwnd, &pServiceProvider)))
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

                        // but we also need the IShellFolder interface because
                        // we need its GetCurFolder() method
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
                                    m_currentDirectory = buf;
                                }
                                // if m_currentDirectory is empty here, that means
                                // the current directory is a virtual path

                                IShellFolder * pShellFolder;
                                if (SUCCEEDED(pPersistFolder->QueryInterface(IID_IShellFolder, (LPVOID*)&pShellFolder)))
                                {
                                    // find all selected items
                                    IEnumIDList * pEnum;
                                    if (SUCCEEDED(pFolderView->Items(SVGIO_ALLVIEW, IID_IEnumIDList, (LPVOID*)&pEnum)))
                                    {
                                        LPITEMIDLIST pidl;
                                        WCHAR buf[MAX_PATH] = {0};
                                        ULONG fetched = 0;
                                        ULONG attribs = 0;
                                        do
                                        {
                                            pidl = NULL;
                                            if (SUCCEEDED(pEnum->Next(1, &pidl, &fetched)))
                                            {
                                                if (fetched)
                                                {
                                                    // the pidl we get here is relative!
                                                    attribs = SFGAO_FILESYSTEM|SFGAO_FOLDER;
                                                    if (SUCCEEDED(pShellFolder->GetAttributesOf(1, (LPCITEMIDLIST*)&pidl, &attribs)))
                                                    {
                                                        if (attribs & SFGAO_FILESYSTEM)
                                                        {
                                                            // create an absolute pidl with the pidl we got above
                                                            LPITEMIDLIST abspidl = CPidl::Append(folderpidl, pidl);
                                                            if (abspidl)
                                                            {
                                                                if (SHGetPathFromIDList(abspidl, buf))
                                                                {
                                                                    std::wstring p = buf;
                                                                    size_t pos = p.find_last_of('\\');
                                                                    if (pos != std::wstring::npos)
                                                                    {
                                                                        m_filelist.insert(p.substr(pos+1));
                                                                    }
                                                                }
                                                                CoTaskMemFree(abspidl);
                                                            }
                                                        }
                                                    }
                                                }
                                                CoTaskMemFree(pidl);
                                            }
                                        } while(fetched);
                                        pEnum->Release();
                                    }
                                    pShellFolder->Release();
                                }
                                CoTaskMemFree(folderpidl);
                            }
                            pPersistFolder->Release();
                        }
                        pFolderView->Release();
                    }
                    pShellView->Release();
                }
                pShellBrowser->Release();
            }
            pServiceProvider->Release();
        }
    }

    // show the rename dialog
    m_bDialogShown = TRUE;
    CRenameDlg dlg(hwnd);
    dlg.SetFileList(m_filelist);
    if (dlg.DoModal(g_hInst, IDD_RENAMEDLG, hwnd, NULL) == IDOK)
    {
        try
        {
            const std::tr1::wregex regCheck(dlg.GetMatchString(), dlg.GetRegexFlags());
            NumberReplaceHandler handler(dlg.GetReplaceString());

            // start renaming the files
            IServiceProvider * pServiceProvider = NULL;
            if (SUCCEEDED(GetIServiceProvider(hwnd, &pServiceProvider)))
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

                            // but we also need the IShellFolder interface because
                            // we need its GetDisplayNameOf() method
                            IPersistFolder2 * pPersistFolder;
                            if (SUCCEEDED(pFolderView->GetFolder(IID_IPersistFolder2, (LPVOID*)&pPersistFolder)))
                            {
                                IShellFolder * pShellFolder;
                                if (SUCCEEDED(pPersistFolder->QueryInterface(IID_IShellFolder, (LPVOID*)&pShellFolder)))
                                {
                                    // our next task is to enumerate all the
                                    // items in the folder view and select those
                                    // which match the text in the edit control

                                    int nCount = 0;
                                    if (SUCCEEDED(pFolderView->ItemCount(SVGIO_ALLVIEW, &nCount)))
                                    {
                                        for (int i=0; i<nCount; ++i)
                                        {
                                            LPITEMIDLIST pidl;
                                            if (SUCCEEDED(pFolderView->Item(i, &pidl)))
                                            {
                                                STRRET str;
                                                if (SUCCEEDED(pShellFolder->GetDisplayNameOf(pidl,
                                                    // SHGDN_FORPARSING needed to get the extensions even if they're not shown
                                                    SHGDN_INFOLDER|SHGDN_FORPARSING,
                                                    &str)))
                                                {
                                                    TCHAR dispname[MAX_PATH];
                                                    StrRetToBuf(&str, pidl, dispname, _countof(dispname));

                                                    std::wstring replaced;
                                                    try
                                                    {
                                                        std::wstring sDispName = dispname;
                                                        // check if the item is in the list of selected items
                                                        if (m_filelist.find(sDispName) != m_filelist.end())
                                                        {
                                                            replaced = std::tr1::regex_replace(sDispName, regCheck, dlg.GetReplaceString());
                                                            replaced = handler.ReplaceCounters(replaced);
                                                            if (replaced.compare(sDispName))
                                                            {
                                                                ITEMIDLIST * pidlrenamed;
                                                                pShellFolder->SetNameOf(NULL, pidl, replaced.c_str(), SHGDN_FORPARSING|SHGDN_INFOLDER, &pidlrenamed);
                                                                // if the rename was successful, select the renamed item
                                                                if (pidlrenamed)
                                                                    pFolderView->SelectItem(i, SVSI_CHECK|SVSI_SELECT);
                                                            }
                                                        }
                                                    }
                                                    catch (std::exception)
                                                    {
                                                    }
                                                }
                                                CoTaskMemFree(pidl);
                                            }
                                        }
                                    }
                                    pShellFolder->Release();
                                }
                                pPersistFolder->Release();
                            }
                            pFolderView->Release();
                        }
                        pShellView->Release();
                    }
                    pShellBrowser->Release();
                }
                pServiceProvider->Release();
            }
        }
        catch (std::exception)
        {
        }
    }
    m_bDialogShown = FALSE;
}

HRESULT CDeskBand::GetIServiceProvider(HWND hwnd, IServiceProvider ** pServiceProvider)
{
    HRESULT hr = E_FAIL;
    if (m_pSite)
        hr = m_pSite->QueryInterface(IID_IServiceProvider, (LPVOID*)pServiceProvider);
    else
    {
        // we don't have a site, so we try finding the explorer window by enumerating
        // all explorer instances and compare that to the parent of hwnd.
        IShellWindows *psw;
        if (SUCCEEDED(CoCreateInstance(CLSID_ShellWindows, NULL, CLSCTX_ALL, IID_IShellWindows, (LPVOID*)&psw)))
        {
            VARIANT v;
            V_VT(&v) = VT_I4;
            IDispatch  *pdisp;
            BOOL fFound = FALSE;
            long count = -1;
            if (SUCCEEDED(psw->get_Count(&count)))
            {
                for (V_I4(&v) = 0; !fFound && (V_I4(&v) < count) && SUCCEEDED(psw->Item(v, &pdisp)); V_I4(&v)++)
                {
                    if (pdisp)
                    {
                        IWebBrowserApp *pwba;
                        if (SUCCEEDED(pdisp->QueryInterface(IID_IWebBrowserApp, (LPVOID*)&pwba)))
                        {
                            HWND hwndWBA;
                            if (SUCCEEDED(pwba->get_HWND((LONG_PTR*)&hwndWBA)))
                            {
                                if ((hwndWBA == hwnd)||(hwndWBA == ::GetParent(hwnd)))
                                {
                                    fFound = TRUE;
                                    hr = pwba->QueryInterface(IID_IServiceProvider, (void**)pServiceProvider);
                                }
                            }
                            pwba->Release();
                        }
                        pdisp->Release();
                    }
                }
            }
            psw->Release();
        }
    }
    return hr;
}
