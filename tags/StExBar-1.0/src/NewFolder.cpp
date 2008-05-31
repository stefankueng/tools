#include "stdafx.h"
#include "SRBand.h"

bool CDeskBand::CreateNewFolder()
{
	if (m_pSite == NULL)
		return false;

	if (m_currentDirectory.empty())
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
				// get the context menu for the shell view background
				IContextMenu * pContextMenu;
				if (SUCCEEDED(pShellView->GetItemObject(SVGIO_BACKGROUND, IID_IContextMenu, (LPVOID*)&pContextMenu)))
				{
					HMENU hMenu = ::CreateMenu();
					if (hMenu)
					{
						if (SUCCEEDED(pContextMenu->QueryContextMenu(hMenu, 0, 0, 1000, CMF_CANRENAME|CMF_EXPLORE|CMF_NORMAL)))
						{
							CMINVOKECOMMANDINFO cici = {0};
							cici.cbSize = sizeof(CMINVOKECOMMANDINFO);
							cici.lpVerb = CMDSTR_NEWFOLDERA;
							if (SUCCEEDED(pContextMenu->InvokeCommand(&cici)))
							{
								IFolderView * pFolderView;
								if (SUCCEEDED(pShellView->QueryInterface(IID_IFolderView, (LPVOID*)&pFolderView)))
								{
									int nCount = 0;
									if (SUCCEEDED(pFolderView->ItemCount(SVGIO_ALLVIEW, &nCount)))
									{
										// since we just created the new folder, it is the last
										// item in the list
										pFolderView->SelectItem(nCount-1, SVSI_EDIT);
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