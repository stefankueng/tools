// StExBar - an explorer toolbar

// Copyright (C) 2007-2008 - Stefan Kueng

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

bool CDeskBand::FindPaths()
{
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
								m_currentDirectory = buf;
							// if m_currentDirectory is empty here, that means
							// the current directory is a virtual path

							IShellFolder * pShellFolder;
							if (SUCCEEDED(pPersistFolder->QueryInterface(IID_IShellFolder, (LPVOID*)&pShellFolder)))
							{
								// find all selected items
								IEnumIDList * pEnum;
								if (SUCCEEDED(pFolderView->Items(SVGIO_SELECTION, IID_IEnumIDList, (LPVOID*)&pEnum)))
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
																m_selectedItems[wstring(buf)] = attribs;
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
	return ((!m_currentDirectory.empty()) || (m_selectedItems.size()!=0));
}

wstring CDeskBand::GetFileNames(wstring separator, bool quotespaces, bool includefiles, bool includefolders)
{
	wstring sRet;
	WCHAR buf[MAX_PATH+2];
	if (m_selectedItems.size())
	{
		for (map<wstring, ULONG>::iterator it = m_selectedItems.begin(); it != m_selectedItems.end(); ++it)
		{
			if (((it->second & SFGAO_FOLDER)&&(includefolders))||(((it->second & SFGAO_FOLDER)==0)&&(includefiles)))
			{
				size_t pos = it->first.find_last_of('\\');
				if (pos >= 0)
				{
					if (!sRet.empty())
						sRet += separator;
					if (quotespaces)
					{
						_tcscpy_s(buf, MAX_PATH, it->first.substr(pos+1).c_str());
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

wstring CDeskBand::GetFilePaths(wstring separator, bool quotespaces, bool includefiles, bool includefolders, bool useunc)
{
	WCHAR buf[MAX_PATH+2];
	wstring sRet;
	if (m_selectedItems.size())
	{
		for (map<wstring, ULONG>::iterator it = m_selectedItems.begin(); it != m_selectedItems.end(); ++it)
		{
			if (((it->second & SFGAO_FOLDER)&&(includefolders))||(((it->second & SFGAO_FOLDER)==0)&&(includefiles)))
			{
				if (!sRet.empty())
					sRet += separator;
				wstring sPath = it->first;
				if (useunc)
				{
					sPath = ConvertToUNC(sPath);
				}
				if (quotespaces)
				{
					_tcscpy_s(buf, MAX_PATH, sPath.c_str());
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

wstring CDeskBand::ConvertToUNC(wstring sPath)
{
	WCHAR temp;
	UNIVERSAL_NAME_INFO * puni = NULL;
	DWORD bufsize = 0;
	wstring sRet = sPath;
	//Call WNetGetUniversalName using UNIVERSAL_NAME_INFO_LEVEL option
	if (WNetGetUniversalName(sPath.c_str(),
		UNIVERSAL_NAME_INFO_LEVEL,
		(LPVOID) &temp,
		&bufsize) == ERROR_MORE_DATA)
	{
		// now we have the size required to hold the UNC path
		WCHAR * buf = new WCHAR[bufsize+1];
		puni = (UNIVERSAL_NAME_INFO *)buf;
		if (WNetGetUniversalName(sPath.c_str(),
			UNIVERSAL_NAME_INFO_LEVEL,
			(LPVOID) puni,
			&bufsize) == NO_ERROR)
		{
			sRet = wstring(puni->lpUniversalName);
		}
		delete [] buf;
	}

	return sRet;;
} 