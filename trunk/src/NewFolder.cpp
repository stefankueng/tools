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
							CMINVOKECOMMANDINFOEX cici = {0};
							cici.cbSize = sizeof(CMINVOKECOMMANDINFOEX);
							cici.lpVerb = CMDSTR_NEWFOLDERA;
							cici.lpVerbW = CMDSTR_NEWFOLDER;
							cici.nShow = SW_SHOWNORMAL;
							cici.hwnd = m_hwndParent;
							cici.fMask = CMIC_MASK_UNICODE | CMIC_MASK_FLAG_NO_UI;
							if (SUCCEEDED(pContextMenu->InvokeCommand((CMINVOKECOMMANDINFO*)&cici)))
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