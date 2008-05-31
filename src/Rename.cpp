#include "stdafx.h"
#include "SRBand.h"
#include "resource.h"
#include <boost\regex.hpp>


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
	if (DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_RENAMEDLG), m_hWnd, RenameDlgFunc, (LPARAM)this)==IDOK)
	{
		boost::wregex e1;
		try
		{
			e1 = boost::wregex(m_sMatch, boost::regex::icase);

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
															replaced = boost::regex_replace(sDispName, e1, m_sReplace);
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
}

INT_PTR CDeskBand::RenameDlgFunc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		{
			CDeskBand * pThis = (CDeskBand*)lParam;
			HWND hwndOwner; 
			RECT rc, rcDlg, rcOwner;

			SetLastError(0);
			if ((SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)pThis)==0)&&(GetLastError()))
				EndDialog(hDlg, IDCANCEL);	// if we could not set the user data, get out ogf here immediately!

			hwndOwner = ::GetParent(hDlg);
			if (hwndOwner == NULL)
				hwndOwner = ::GetDesktopWindow();

#pragma warning(push)
#pragma warning(disable: 4127)	// conditional expression is constant
			BEGIN_SIZINGRULES(pThis->m_grip, hDlg)
				ADDRULE(pThis->m_grip, IDC_MATCHSTRING, RX)
				ADDRULE(pThis->m_grip, IDC_REPLACESTRING, RX)
				ADDRULE(pThis->m_grip, IDC_FILELIST, RXY)
				ADDRULE(pThis->m_grip, IDOK, MXY)
				ADDRULE(pThis->m_grip, IDCANCEL, MXY)
				END_SIZINGRULES
#pragma warning(pop)

				GetWindowRect(hwndOwner, &rcOwner); 
			GetWindowRect(hDlg, &rcDlg); 
			CopyRect(&rc, &rcOwner); 

			OffsetRect(&rcDlg, -rcDlg.left, -rcDlg.top); 
			OffsetRect(&rc, -rc.left, -rc.top); 
			OffsetRect(&rc, -rcDlg.right, -rcDlg.bottom); 

			SetWindowPos(hDlg, HWND_TOP, rcOwner.left + (rc.right / 2), rcOwner.top + (rc.bottom / 2), 0, 0,	SWP_NOSIZE); 
			HICON hIcon = (HICON)::LoadImage(g_hInst, MAKEINTRESOURCE(IDI_RENAME), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE|LR_SHARED);
			::SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
			::SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

			// set up the list control
			DWORD exStyle = LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER;
			HWND hListCtrl = GetDlgItem(hDlg, IDC_FILELIST);
			if (hListCtrl)
			{
				ListView_DeleteAllItems(hListCtrl);
				ListView_SetExtendedListViewStyle(hListCtrl, exStyle);
				LVCOLUMN lvc = {0};
				lvc.mask = LVCF_TEXT;
				lvc.fmt = LVCFMT_LEFT;
				lvc.cx = -1;
				lvc.pszText = _T("filename");
				ListView_InsertColumn(hListCtrl, 0, &lvc);
				lvc.pszText = _T("renamed");
				ListView_InsertColumn(hListCtrl, 1, &lvc);
			}
			pThis->FillRenamedList(hDlg);

			::SetFocus(::GetDlgItem(hDlg, IDC_MATCHSTRING));
		}
		return (INT_PTR)TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			{
				CDeskBand * pThis = (CDeskBand*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
				TCHAR buf[MAX_PATH];
				HWND hMatchstring = GetDlgItem(hDlg, IDC_MATCHSTRING);
				if (hMatchstring == NULL)
					return (INT_PTR)TRUE;
				GetWindowText(hMatchstring, buf, MAX_PATH);
				pThis->m_sMatch = buf;
				HWND hReplaceString = GetDlgItem(hDlg, IDC_REPLACESTRING);
				if (hReplaceString == NULL)
					return (INT_PTR)TRUE;
				GetWindowText(hReplaceString, buf, MAX_PATH);
				pThis->m_sReplace = buf;
			}
			// fall through
		case IDCANCEL:
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		case IDC_MATCHSTRING:
		case IDC_REPLACESTRING:
			if (HIWORD(wParam)==EN_CHANGE)
			{
				CDeskBand * pThis = (CDeskBand*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
				pThis->FillRenamedList(hDlg);
			}
			return (INT_PTR)TRUE;
		}
		break;
	case WM_NOTIFY:
		{
			LPNMHDR pnmhdr = (LPNMHDR)lParam;
			switch (pnmhdr->code)
			{
			case NM_CUSTOMDRAW:
				{
					if (pnmhdr->hwndFrom == GetDlgItem(hDlg, IDC_FILELIST))
					{
						LPNMLVCUSTOMDRAW lpcd = (LPNMLVCUSTOMDRAW)lParam;
						// use the default processing if not otherwise specified
						SetWindowLong(hDlg, DWL_MSGRESULT, CDRF_DODEFAULT); 
						if (lpcd->nmcd.dwDrawStage == CDDS_PREPAINT)
						{
							SetWindowLong(hDlg, DWL_MSGRESULT, CDRF_NOTIFYITEMDRAW); 
						}
						else if (lpcd->nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
						{
							// This is the prepaint stage for an item. Here's where we set the
							// item's text color. Our return value will tell Windows to draw the
							// item itself, but it will use the new color we set here.
							COLORREF crText = GetSysColor(COLOR_WINDOWTEXT);

							if (lpcd->nmcd.lItemlParam == 0)
								crText = GetSysColor(COLOR_GRAYTEXT);

							// Store the color back in the NMLVCUSTOMDRAW struct.
							lpcd->clrText = crText;

							// Tell Windows to paint the control itself.
							SetWindowLong(hDlg, DWL_MSGRESULT, CDRF_DODEFAULT); 
						}
					}
					return TRUE;
				}
			}
		}
		break;
	case WM_SIZE:
		{
			CDeskBand * pThis = (CDeskBand*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
			DORESIZE(pThis->m_grip);
		}
		break;
	case WM_SIZING:
		{
			MINMAX(400,200);
		}
		break;
	}
	return (INT_PTR)FALSE;
}

void CDeskBand::FillRenamedList(HWND hDlg)
{
	HWND hListCtrl = GetDlgItem(hDlg, IDC_FILELIST);
	TCHAR buf[MAX_PATH];
	HWND hMatchstring = GetDlgItem(hDlg, IDC_MATCHSTRING);
	if (hMatchstring == NULL)
		return;
	GetWindowText(hMatchstring, buf, MAX_PATH);
	m_sMatch = buf;
	HWND hReplaceString = GetDlgItem(hDlg, IDC_REPLACESTRING);
	if (hReplaceString == NULL)
		return;
	GetWindowText(hReplaceString, buf, MAX_PATH);
	m_sReplace = buf;

	ListView_DeleteAllItems(hListCtrl);
	// we also need a map which assigns each file its renamed equivalent
	map<wstring, wstring> renamedmap;
	// create the regex
	boost::wregex e1;
	try
	{
		e1 = boost::wregex(m_sMatch, boost::regex::icase);

		wstring replaced;
		for (set<wstring>::iterator it = m_filelist.begin(); it != m_filelist.end(); ++it)
		{
			replaced = boost::regex_replace(*it, e1, m_sReplace);
			renamedmap[*it] = replaced;
		}
		// now fill in the list of files which got renamed
		int iItem = 0;
		TCHAR textbuf[MAX_PATH] = {0};
		for (map<wstring, wstring>::iterator it = renamedmap.begin(); it != renamedmap.end(); ++it)
		{
			LVITEM lvi = {0};
			lvi.mask = LVIF_TEXT|LVIF_PARAM;
			_tcscpy_s(textbuf, MAX_PATH, it->first.c_str());
			lvi.pszText = textbuf;
			lvi.iItem = iItem;
			// we use the lParam to store the result of the comparison of the original and renamed string
			// later in the NM_CUSTOMDRAW handler, we use that information to draw items which stay the
			// same in the rename grayed out.
			lvi.lParam = it->first.compare(it->second);
			ListView_InsertItem(hListCtrl, &lvi);
			_tcscpy_s(textbuf, MAX_PATH, it->second.c_str());
			ListView_SetItemText(hListCtrl, iItem, 1, textbuf);
			iItem++;
		}
		ListView_SetColumnWidth(hListCtrl, 0, LVSCW_AUTOSIZE_USEHEADER);
		ListView_SetColumnWidth(hListCtrl, 1, LVSCW_AUTOSIZE_USEHEADER);
	}
	catch (boost::bad_expression e)
	{
		int iItem = 0;
		TCHAR textbuf[MAX_PATH] = {0};
		for (set<wstring>::iterator it = m_filelist.begin(); it != m_filelist.end(); ++it)
		{
			LVITEM lvi = {0};
			lvi.mask = LVIF_TEXT|LVIF_PARAM;
			_tcscpy_s(textbuf, MAX_PATH, it->c_str());
			lvi.pszText = textbuf;
			lvi.iItem = iItem;
			// we use the lParam to store the result of the comparison of the original and renamed string
			// later in the NM_CUSTOMDRAW handler, we use that information to draw items which stay the
			// same in the rename grayed out.
			lvi.lParam = 0;
			ListView_InsertItem(hListCtrl, &lvi);
			ListView_SetItemText(hListCtrl, iItem, 1, textbuf);
			iItem++;
		}
		ListView_SetColumnWidth(hListCtrl, 0, LVSCW_AUTOSIZE_USEHEADER);
		ListView_SetColumnWidth(hListCtrl, 1, LVSCW_AUTOSIZE_USEHEADER);
	}
}