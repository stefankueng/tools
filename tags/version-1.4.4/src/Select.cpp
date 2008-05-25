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
#include "StringUtils.h"
#include <regex>

bool CDeskBand::Select(LPTSTR filter)
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

					// the first thing we do is to deselect all already selected entries
					pFolderView->SelectItem(NULL, SVSI_DESELECTOTHERS);

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
								bool bFirstMatch = true;
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

											if (bUseRegex)
											{

												try
												{
													const tr1::wregex regCheck(&filter[1], tr1::regex_constants::icase | tr1::regex_constants::ECMAScript);
													wstring s = dispname;
													if (tr1::regex_search(s, regCheck))
													{
														// yes, we have a match!
														// now select that match
														DWORD dwFlags = SVSI_CHECK|SVSI_SELECT|SVSI_ENSUREVISIBLE;
														if (bFirstMatch)
														{
															// if this is the first item we have to select,
															// deselect all previously selected items
															// and make this one the first in the selection
															dwFlags |= SVSI_SELECTIONMARK|SVSI_DESELECTOTHERS;
															bFirstMatch = false;
														}
														pFolderView->SelectItem(i, dwFlags);
													}
												}
												catch (exception) {}
											}
											else
											{
												// we now have the display name of the item
												// i.e. the way the item is shown
												// since the windows filesystem is case-insensitive
												// we have to force the display name to lowercase
												// so the filter matches case-insensitive too
												TCHAR * pString = dispname;
												while (*pString)
												{
													*pString = _totlower(*pString);
													pString++;
												}
												// check if the item name matches the wildcard of the edit control
												if (CStringUtils::wcswildcmp(filter, dispname))
												{
													// yes, we have a match!
													// now select that match
													DWORD dwFlags = SVSI_CHECK|SVSI_SELECT|SVSI_ENSUREVISIBLE|SVSI_POSITIONITEM;
													if (bFirstMatch)
													{
														// if this is the first item we have to select,
														// deselect all previously selected items
														// and make this one the first in the selection
														dwFlags |= SVSI_SELECTIONMARK|SVSI_DESELECTOTHERS;
														bFirstMatch = false;
													}
													pFolderView->SelectItem(i, dwFlags);
												}
											}
											bReturn = true;
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
	return bReturn;
}