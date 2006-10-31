#include "stdafx.h"
#include "SRBand.h"

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
					// we need its GetDisplayNameOf() method
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
							CoTaskMemFree(folderpidl);
						}

						// find all selected items
						IEnumIDList * pEnum;
						if (SUCCEEDED(pFolderView->Items(SVGIO_SELECTION, IID_IEnumIDList, (LPVOID*)&pEnum)))
						{
							LPITEMIDLIST pidl;
							TCHAR buf[MAX_PATH] = {0};
							ULONG fetched = 0;
							do 
							{
								pidl = NULL;
								if (SUCCEEDED(pEnum->Next(1, &pidl, &fetched)))
								{
									if (fetched)
									{
										if (SHGetPathFromIDList(pidl, buf))
											m_selectedItems.insert(stdstring(buf));
									}
									CoTaskMemFree(pidl);
								}
							} while(fetched);
							pEnum->Release();
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