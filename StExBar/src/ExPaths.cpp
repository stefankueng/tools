// StExBar - an explorer toolbar

// Copyright (C) 2007-2010, 2012-2013, 2015, 2020 - Stefan Kueng

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
#include "SimpleIni.h"
#include "Pidl.h"

#pragma comment(lib, "Mpr")

// returns true if the m_currentDirectory changed
bool CDeskBand::FindPaths()
{
    auto oldCurrentDir = m_currentDirectory;
    m_currentDirectory.clear();
    m_selectedItems.clear();
    m_bFilesSelected = false;
    m_bFolderSelected = false;

    if (m_pSite == NULL)
        return false;
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
                                // if there was a new folder created but not found to set into editing mode,
                                // we try here to do that
                                if (!m_newfolderPidls.empty())
                                {
                                    int nCount2 = 0;
                                    IShellFolder * pShellFolder2 = nullptr;
                                    if (SUCCEEDED(pPersistFolder->QueryInterface(IID_IShellFolder, (LPVOID*)&pShellFolder2)))
                                    {
                                        if (SUCCEEDED(pFolderView->ItemCount(SVGIO_ALLVIEW, &nCount2)))
                                        {
                                            for (int i=0; i<nCount2; ++i)
                                            {
                                                LPITEMIDLIST pidl;
                                                pFolderView->Item(i, &pidl);
                                                bool bFound = false;
                                                for (std::vector<LPITEMIDLIST>::iterator it = m_newfolderPidls.begin(); it != m_newfolderPidls.end(); ++it)
                                                {
                                                    HRESULT hr = pShellFolder2->CompareIDs(0, pidl, *it);
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
                                                    pShellView->SelectItem(pidl, SVSI_EDIT);
                                                }
                                                CoTaskMemFree(pidl);
                                            }
                                        }
                                        if ((nCount2)||(m_newfolderTimeoutCounter-- <= 0))
                                        {
                                            m_newfolderTimeoutCounter = 0;
                                            for (std::vector<LPITEMIDLIST>::iterator it = m_newfolderPidls.begin(); it != m_newfolderPidls.end(); ++it)
                                            {
                                                CoTaskMemFree(*it);
                                            }
                                            m_newfolderPidls.clear();
                                        }
                                        pShellFolder2->Release();
                                    }

                                }
                                // find all selected items
                                IEnumIDList * pEnum;
                                if (SUCCEEDED(pFolderView->Items(SVGIO_SELECTION, IID_IEnumIDList, (LPVOID*)&pEnum)))
                                {
                                    LPITEMIDLIST pidl;
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
                                                                m_selectedItems[std::wstring(buf)] = attribs;
                                                                if (m_currentDirectory.empty())
                                                                {
                                                                    // remove the last part of the path of the selected item
                                                                    WCHAR * pSlash = _tcsrchr(buf, '\\');
                                                                    if (pSlash)
                                                                        *pSlash = 0;
                                                                    m_currentDirectory = std::wstring(buf);
                                                                }
                                                            }
                                                            CoTaskMemFree(abspidl);
                                                        }
                                                        if (attribs & SFGAO_FOLDER)
                                                            m_bFolderSelected = true;
                                                        else
                                                            m_bFilesSelected = true;
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
    return (oldCurrentDir != m_currentDirectory);
}

std::wstring CDeskBand::GetFileNames(const std::map<std::wstring, ULONG>& items, std::wstring separator, bool quotespaces, bool includefiles, bool includefolders)
{
    std::wstring sRet;
    WCHAR buf[MAX_PATH+2];
    if (!items.empty())
    {
        for (std::map<std::wstring, ULONG>::const_iterator it = items.begin(); it != items.end(); ++it)
        {
            if (((it->second & SFGAO_FOLDER)&&(includefolders))||(((it->second & SFGAO_FOLDER)==0)&&(includefiles)))
            {
                size_t pos = it->first.find_last_of('\\');
                if (pos != std::string::npos)
                {
                    if (!sRet.empty())
                        sRet += separator;
                    if (quotespaces)
                    {
                        _tcscpy_s(buf, _countof(buf), it->first.substr(pos+1).c_str());
                        PathQuoteSpaces(buf);
                        sRet += buf;
                    }
                    else
                        sRet += it->first.substr(pos+1);
                }
            }
        }
    }
    return sRet;
}

std::wstring CDeskBand::GetFilePaths(const std::map<std::wstring, ULONG>& items, std::wstring separator, bool quotespaces, bool includefiles, bool includefolders, bool useunc)
{
    WCHAR buf[MAX_PATH+2];
    std::wstring sRet;
    if (!items.empty())
    {
        for (std::map<std::wstring, ULONG>::const_iterator it = items.begin(); it != items.end(); ++it)
        {
            if (((it->second & SFGAO_FOLDER)&&(includefolders))||(((it->second & SFGAO_FOLDER)==0)&&(includefiles)))
            {
                if (!sRet.empty())
                    sRet += separator;
                std::wstring sPath = it->first;
                if (useunc)
                {
                    sPath = ConvertToUNC(sPath);
                }
                if (quotespaces)
                {
                    _tcscpy_s(buf, _countof(buf), sPath.c_str());
                    PathQuoteSpaces(buf);
                    sRet += buf;
                }
                else
                    sRet += sPath;
            }
        }
    }
    return sRet;
}

std::wstring CDeskBand::ConvertToUNC(std::wstring sPath)
{
    WCHAR temp;
    DWORD bufsize = 0;
    std::wstring sRet = sPath;
    //Call WNetGetUniversalName using UNIVERSAL_NAME_INFO_LEVEL option
    if (WNetGetUniversalName(sPath.c_str(),
        UNIVERSAL_NAME_INFO_LEVEL,
        (LPVOID) &temp,
        &bufsize) == ERROR_MORE_DATA)
    {
        // now we have the size required to hold the UNC path
        WCHAR * buf = new WCHAR[bufsize+1];
        UNIVERSAL_NAME_INFO * puni = (UNIVERSAL_NAME_INFO *)buf;
        if (WNetGetUniversalName(sPath.c_str(),
                                 UNIVERSAL_NAME_INFO_LEVEL,
                                 (LPVOID) puni,
                                 &bufsize) == NO_ERROR)
        {
            sRet = std::wstring(puni->lpUniversalName);
        }
        delete [] buf;
    }

    return sRet;
}