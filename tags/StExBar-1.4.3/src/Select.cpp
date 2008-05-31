#include "stdafx.h"
#include "SRBand.h"
#include "StringUtils.h"
#include <boost\regex.hpp>

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

							boost::wregex e;
							try
							{
								e = boost::wregex(&filter[1], boost::regex::icase);
							}
							catch (boost::bad_expression e)
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
													boost::wcmatch what;
													if (boost::regex_match(dispname, what, e))
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
												catch (std::runtime_error e)
												{

												}
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