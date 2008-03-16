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
#include "resource.h"
#include <boost\regex.hpp>
#include "RenameDlg.h"

void CDeskBand::Rename()
{
	// fill the list of selected file/foldernames
	m_filelist.clear();
	if (m_selectedItems.size())
	{
		for (map<wstring, ULONG>::iterator it = m_selectedItems.begin(); it != m_selectedItems.end(); ++it)
		{
			size_t pos = it->first.find_last_of('\\');
			if (pos >= 0)
			{
				m_filelist.insert(it->first.substr(pos+1));
			}
		}
	}

	// show the rename dialog
	m_bDialogShown = TRUE;
	CRenameDlg dlg(m_hWnd);
	dlg.SetFileList(m_filelist);
	if (dlg.DoModal(g_hInst, IDD_RENAMEDLG, m_hWnd) == IDOK)
	{
		boost::wregex e1;
		try
		{
			e1 = boost::wregex(dlg.GetMatchString(), boost::regex::icase);

			// start renaming the files
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
													StrRetToBuf(&str, pidl, dispname, MAX_PATH);

													wstring replaced;
													try
													{
														wstring sDispName = dispname;
														// check if the item is in the list of selected items
														if (m_filelist.find(sDispName) != m_filelist.end())
														{
															replaced = boost::regex_replace(sDispName, e1, dlg.GetReplaceString());
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
													catch (runtime_error x)
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
		catch (boost::bad_expression e)
		{
		}
	}
	m_bDialogShown = FALSE;
}
