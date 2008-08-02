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
#include "version.h"
#include <algorithm>
#include "RenameDlg.h"
#include <string>

#include <regex>
using namespace std;


CRenameDlg::CRenameDlg(HWND hParent)
{
	m_hParent = hParent;
}

CRenameDlg::~CRenameDlg(void)
{
}

LRESULT CRenameDlg::DlgFunc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			InitDialog(hwndDlg, IDI_RENAME);

			m_resizer.Init(hwndDlg);
			m_resizer.AddControl(hwndDlg, IDC_MATCHLABEL, RESIZER_TOPLEFT);
			m_resizer.AddControl(hwndDlg, IDC_REPLACELABEL, RESIZER_TOPLEFT);
			m_resizer.AddControl(hwndDlg, IDC_MATCHSTRING, RESIZER_TOPLEFTRIGHT);
			m_resizer.AddControl(hwndDlg, IDC_REPLACESTRING, RESIZER_TOPLEFTRIGHT);
			m_resizer.AddControl(hwndDlg, IDC_FILELIST, RESIZER_TOPLEFTBOTTOMRIGHT);
			m_resizer.AddControl(hwndDlg, IDOK, RESIZER_BOTTOMRIGHT);
			m_resizer.AddControl(hwndDlg, IDCANCEL, RESIZER_BOTTOMRIGHT);

			// set up the list control
			DWORD exStyle = LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER;
			HWND hListCtrl = GetDlgItem(hwndDlg, IDC_FILELIST);
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
			FillRenamedList();

			::SetFocus(::GetDlgItem(hwndDlg, IDC_MATCHSTRING));
		}
		return (INT_PTR)TRUE;
	case WM_SIZE:
		m_resizer.DoResize(LOWORD(lParam), HIWORD(lParam));
		return (INT_PTR)TRUE;
	case WM_GETMINMAXINFO:
		{
			MINMAXINFO * mmi = (MINMAXINFO*)lParam;
			mmi->ptMinTrackSize.x = m_resizer.GetDlgRect()->right;
			mmi->ptMinTrackSize.y = m_resizer.GetDlgRect()->bottom;
			return (INT_PTR)0;
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			{
				TCHAR buf[MAX_PATH];
				HWND hMatchstring = GetDlgItem(*this, IDC_MATCHSTRING);
				if (hMatchstring == NULL)
					return (INT_PTR)TRUE;
				GetWindowText(hMatchstring, buf, MAX_PATH);
				m_sMatch = buf;
				HWND hReplaceString = GetDlgItem(*this, IDC_REPLACESTRING);
				if (hReplaceString == NULL)
					return (INT_PTR)TRUE;
				GetWindowText(hReplaceString, buf, MAX_PATH);
				m_sReplace = buf;
			}
			// fall through
		case IDCANCEL:
			EndDialog(*this, LOWORD(wParam));
			return (INT_PTR)TRUE;
		case IDC_MATCHSTRING:
		case IDC_REPLACESTRING:
			if (HIWORD(wParam)==EN_CHANGE)
			{
				FillRenamedList();
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
					if (pnmhdr->hwndFrom == GetDlgItem(*this, IDC_FILELIST))
					{
						LPNMLVCUSTOMDRAW lpcd = (LPNMLVCUSTOMDRAW)lParam;
						// use the default processing if not otherwise specified
						SetWindowLongPtr(*this, DWLP_MSGRESULT, CDRF_DODEFAULT); 
						if (lpcd->nmcd.dwDrawStage == CDDS_PREPAINT)
						{
							SetWindowLongPtr(*this, DWLP_MSGRESULT, CDRF_NOTIFYITEMDRAW); 
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
							SetWindowLongPtr(*this, DWLP_MSGRESULT, CDRF_DODEFAULT); 
						}
					}
					return TRUE;
				}
			}
		}
		break;
	}
	return (INT_PTR)FALSE;
}

void CRenameDlg::FillRenamedList()
{
	HWND hListCtrl = GetDlgItem(*this, IDC_FILELIST);
	TCHAR buf[MAX_PATH];
	HWND hMatchstring = GetDlgItem(*this, IDC_MATCHSTRING);
	if (hMatchstring == NULL)
		return;
	GetWindowText(hMatchstring, buf, MAX_PATH);
	m_sMatch = buf;
	if (m_sMatch.size() == 0)
		return;
	HWND hReplaceString = GetDlgItem(*this, IDC_REPLACESTRING);
	if (hReplaceString == NULL)
		return;
	GetWindowText(hReplaceString, buf, MAX_PATH);
	m_sReplace = buf;

	ListView_DeleteAllItems(hListCtrl);
	// we also need a map which assigns each file its renamed equivalent
	map<wstring, wstring> renamedmap;

	try
	{
		const tr1::wregex regCheck(m_sMatch, tr1::regex_constants::icase | tr1::regex_constants::ECMAScript);

		wstring replaced;
		for (set<wstring>::iterator it = m_filelist.begin(); it != m_filelist.end(); ++it)
		{
			replaced = tr1::regex_replace(*it, regCheck, m_sReplace);
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
	catch (const exception&)
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