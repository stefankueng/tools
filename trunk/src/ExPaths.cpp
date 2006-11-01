#include "stdafx.h"
#include "SRBand.h"
#include "SimpleIni.h"
#include "Pidl.h"

bool CDeskBand::FindPaths()
{
	m_currentDirectory.clear();
	m_selectedItems.clear();

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

							// find all selected items
							IEnumIDList * pEnum;
							if (SUCCEEDED(pFolderView->Items(SVGIO_SELECTION, IID_IEnumIDList, (LPVOID*)&pEnum)))
							{
								LPITEMIDLIST pidl;
								WCHAR buf[MAX_PATH] = {0};
								ULONG fetched = 0;
								do 
								{
									pidl = NULL;
									if (SUCCEEDED(pEnum->Next(1, &pidl, &fetched)))
									{
										if (fetched)
										{
											// the pidl we get here is relative!
											// create an absolute pidl with the pidl we got above
											LPITEMIDLIST abspidl = CPidl::Append(folderpidl, pidl);
											if (abspidl)
											{
												if (SHGetPathFromIDList(abspidl, buf))
													m_selectedItems.insert(wstring(buf));
												CoTaskMemFree(abspidl);
											}
										}
										CoTaskMemFree(pidl);
									}
								} while(fetched);
								pEnum->Release();
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

wstring CDeskBand::GetFileNames(wstring separator, bool quotespaces)
{
	wstring sRet;
	WCHAR buf[MAX_PATH+2];
	if (m_selectedItems.size())
	{
		for (set<wstring>::iterator it = m_selectedItems.begin(); it != m_selectedItems.end(); ++it)
		{
			size_t pos = it->find_last_of('\\');
			if (pos >= 0)
			{
				if (!sRet.empty())
					sRet += separator;
				if (quotespaces)
				{
					_tcscpy_s(buf, MAX_PATH, it->substr(pos+1).c_str());
					PathQuoteSpaces(buf);
					sRet += buf;
				}
				else
					sRet += it->substr(pos+1);
			}
		}
	}
	return sRet;
}

wstring CDeskBand::GetFilePaths(wstring separator, bool quotespaces)
{
	WCHAR buf[MAX_PATH+2];
	wstring sRet;
	if (m_selectedItems.size())
	{
		for (set<wstring>::iterator it = m_selectedItems.begin(); it != m_selectedItems.end(); ++it)
		{
			if (!sRet.empty())
				sRet += separator;
			if (quotespaces)
			{
				_tcscpy_s(buf, MAX_PATH, it->c_str());
				PathQuoteSpaces(buf);
				sRet += buf;
			}
			else
				sRet += *it;
		}
	}
	return sRet;
}