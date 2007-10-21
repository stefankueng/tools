#include "stdafx.h"
#include "SRBand.h"
#include "resource.h"
#include "version.h"
#include <algorithm>
#include "OptionsDlg.h"
#include "EditCmdDlg.h"
#include <string>

#include <boost/regex.hpp>
using namespace boost;
using namespace std;


COptionsDlg::COptionsDlg(HWND hParent) : m_regShowBtnText(_T("Software\\StefansTools\\StExBar\\ShowButtonText"), 1)
	, m_regUseUNCPaths(_T("Software\\StefansTools\\StExBar\\UseUNCPaths"), 1)
	, m_regUseSelector(_T("Software\\StefansTools\\StExBar\\UseSelector"), 1)
	, m_regHideEditBox(_T("Software\\StefansTools\\StExBar\\HideEditBox"), 0)
{
	m_hParent = hParent;
}

COptionsDlg::~COptionsDlg(void)
{
}

LRESULT COptionsDlg::DlgFunc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			InitDialog(hwndDlg, IDI_OPTIONS);

			m_hListControl = GetDlgItem(*this, IDC_CUSTCOMMANDS);

			m_link.ConvertStaticToHyperlink(hwndDlg, IDC_LINK, _T("http://tools.tortoisesvn.net"));

			SendMessage(GetDlgItem(hwndDlg, IDC_SHOWTEXT), BM_SETCHECK, DWORD(m_regShowBtnText) ? BST_CHECKED : BST_UNCHECKED, 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_USEUNCCHECK), BM_SETCHECK, DWORD(m_regUseUNCPaths) ? BST_CHECKED : BST_UNCHECKED, 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_HIDEEDITBOX), BM_SETCHECK, DWORD(m_regHideEditBox) ? BST_CHECKED : BST_UNCHECKED, 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_SELECTORCHECK), BM_SETCHECK, DWORD(m_regUseSelector) ? BST_CHECKED : BST_UNCHECKED, 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SELECTORCHECK), !DWORD(m_regHideEditBox));

			AddToolTip(IDC_SHOWTEXT, _T("shows the name of the button below the icon"));
			AddToolTip(IDC_USEUNCCHECK, _T("For mounted network drives, copies the UNC path for files/folders\r\ninstead of the path with the mounted drive letter"));
			AddToolTip(IDC_SELECTORCHECK, _T("Determines whether the edit box on the right behaves as a shortcut for the console\r\nor whether it selects items according to the entered mask"));
			AddToolTip(IDC_HIDEEDITBOX, _T("Hides the edit box on the right of the toolbar"));

			TCHAR buf[MAX_PATH] = {0};
			_stprintf_s(buf, MAX_PATH, _T("StExBar %ld.%ld.%ld.%ld"), VER_MAJOR, VER_MINOR, VER_MICRO, VER_REVISION);
			SetDlgItemText(hwndDlg, IDC_VERSIONSTRING, buf);

			m_commands.LoadFromFile();
			InitCustomCommandsList();
		}
		return (INT_PTR)TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			{
				m_regShowBtnText = SendMessage(GetDlgItem(hwndDlg, IDC_SHOWTEXT), BM_GETCHECK, 0, 0);
				m_regUseUNCPaths = SendMessage(GetDlgItem(hwndDlg, IDC_USEUNCCHECK), BM_GETCHECK, 0, 0);
				m_regUseSelector = SendMessage(GetDlgItem(hwndDlg, IDC_SELECTORCHECK), BM_GETCHECK, 0, 0);
				m_regHideEditBox = SendMessage(GetDlgItem(hwndDlg, IDC_HIDEEDITBOX), BM_GETCHECK, 0, 0);
				m_commands.SaveToFile();
			}
			// fall through
		case IDCANCEL:
			EndDialog(*this, LOWORD(wParam));
			return (INT_PTR)TRUE;
		case IDC_HIDEEDITBOX:
			EnableWindow(GetDlgItem(hwndDlg, IDC_SELECTORCHECK), !SendMessage(GetDlgItem(hwndDlg, IDC_HIDEEDITBOX), BM_GETCHECK, 0, 0));
			break;
		case IDC_EDITCMD:
			EditSelectedItem();
			break;
		case IDC_ADD:
			{
				Command cmd;
				cmd.separator = false;
				CEditCmdDlg dlg(*this);
				dlg.SetCommand(cmd);
				if (dlg.DoModal(hResource, IDD_EDITCMD, *this) == IDOK)
				{
					m_commands.InsertCommand(m_commands.GetCount(), dlg.GetCommand());
					InitCustomCommandsList();
				}
			}
			break;
		case IDC_REMOVE:
			RemoveSelectedItem();
			break;
		case IDC_MOVEUP:
			MoveSelectedUp();
			break;
		case IDC_MOVEDOWN:
			MoveSelectedDown();
			break;
		}
		break;
	case WM_NOTIFY:
		{
			LPNMHDR lpnmhdr = (LPNMHDR)lParam;
			if ((lpnmhdr->code == LVN_ITEMCHANGED)&&(lpnmhdr->hwndFrom == m_hListControl))
			{
				OnSelectListItem((LPNMLISTVIEW)lParam);
			}
			if ((lpnmhdr->code == LVN_KEYDOWN)&&(lpnmhdr->hwndFrom == m_hListControl))
			{
				LPNMLVKEYDOWN pnkd = (LPNMLVKEYDOWN)lParam;
				if (pnkd->wVKey == VK_DELETE)
				{
					RemoveSelectedItem();
				}
			}
			if ((lpnmhdr->code == NM_DBLCLK)&&(lpnmhdr->hwndFrom == m_hListControl))
			{
				EditSelectedItem();
			}
			return FALSE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}


void COptionsDlg::InitCustomCommandsList()
{
	if (m_hListControl == NULL)
		return;

	DWORD exStyle = LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER;
	ListView_DeleteAllItems(m_hListControl);

	int c = Header_GetItemCount(ListView_GetHeader(m_hListControl))-1;
	while (c>=0)
		ListView_DeleteColumn(m_hListControl, c--);

	ListView_SetExtendedListViewStyle(m_hListControl, exStyle);
	LVCOLUMN lvc = {0};
	lvc.mask = LVCF_TEXT;
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = -1;
	lvc.pszText = _T("Command");
	ListView_InsertColumn(m_hListControl, 0, &lvc);

	LVITEM item = {0};
	TCHAR buf[1024];
	for (int i = 0; i < m_commands.GetCount()-1; ++i)
	{
		Command cmd = m_commands.GetCommand(i+1);
		item.mask = LVIF_TEXT|LVIF_PARAM;
		item.iItem = i;
		item.lParam = i;
		if (cmd.separator)
			_tcscpy_s(buf, 1024, _T("----------"));
		else
			_tcscpy_s(buf, 1024, cmd.name.c_str());
		item.pszText = buf;
		ListView_InsertItem(m_hListControl, &item);
	}
	ListView_SetColumnWidth(m_hListControl, 0, LVSCW_AUTOSIZE_USEHEADER);

	// disable the edit and remove button since nothing is selected now
	EnableWindow(GetDlgItem(*this, IDC_EDITCMD), FALSE);
	EnableWindow(GetDlgItem(*this, IDC_REMOVE), FALSE);
	EnableWindow(GetDlgItem(*this, IDC_MOVEUP), FALSE);
	EnableWindow(GetDlgItem(*this, IDC_MOVEDOWN), FALSE);
	::InvalidateRect(m_hListControl, NULL, false);
}

void COptionsDlg::OnSelectListItem(LPNMLISTVIEW /*lpNMListView*/)
{
	UINT nCount = ListView_GetSelectedCount(m_hListControl);
	EnableWindow(GetDlgItem(*this, IDC_EDITCMD), nCount == 1);
	EnableWindow(GetDlgItem(*this, IDC_REMOVE), nCount == 1);
	EnableWindow(GetDlgItem(*this, IDC_MOVEUP), nCount == 1);
	EnableWindow(GetDlgItem(*this, IDC_MOVEDOWN), nCount == 1);
}

void COptionsDlg::MoveSelectedUp()
{
	LVITEM item = {0};
	for (int i=0; i<ListView_GetItemCount(m_hListControl); ++i)
	{
		item.mask = LVIF_PARAM|LVIF_STATE;
		item.stateMask = LVIS_SELECTED;
		item.iItem = i;
		ListView_GetItem(m_hListControl, &item);
		if (item.state & LVIS_SELECTED)
		{
			if (i > 1)
			{
				Command c1 = m_commands.GetCommand(i);
				m_commands.SetCommand(i, m_commands.GetCommand(i+1));
				m_commands.SetCommand(i+1, c1);
				InitCustomCommandsList();
				item.mask = LVIF_PARAM|LVIF_STATE;
				item.stateMask = LVIS_SELECTED;
				item.iItem = i-1;
				ListView_SetItem(m_hListControl, &item);
			}
			return;
		}
	}
}

void COptionsDlg::MoveSelectedDown()
{
	LVITEM item = {0};
	for (int i=0; i<ListView_GetItemCount(m_hListControl); ++i)
	{
		item.mask = LVIF_PARAM|LVIF_STATE;
		item.stateMask = LVIS_SELECTED;
		item.iItem = i;
		ListView_GetItem(m_hListControl, &item);
		if (item.state & LVIS_SELECTED)
		{
			if (i < m_commands.GetCount()-3)
			{
				Command c1 = m_commands.GetCommand(i+2);
				m_commands.SetCommand(i+2, m_commands.GetCommand(i+1));
				m_commands.SetCommand(i+1, c1);
				InitCustomCommandsList();
				item.mask = LVIF_PARAM|LVIF_STATE;
				item.stateMask = LVIS_SELECTED;
				item.iItem = i+1;
				ListView_SetItem(m_hListControl, &item);
			}
			return;
		}
	}
}

void COptionsDlg::RemoveSelectedItem()
{
	LVITEM item = {0};
	for (int i=0; i<ListView_GetItemCount(m_hListControl); ++i)
	{
		item.mask = LVIF_PARAM|LVIF_STATE;
		item.stateMask = LVIS_SELECTED;
		item.iItem = i;
		ListView_GetItem(m_hListControl, &item);
		if (item.state & LVIS_SELECTED)
		{
			m_commands.RemoveCommand(i+1);
			InitCustomCommandsList();
			return;
		}
	}
}

void COptionsDlg::EditSelectedItem()
{
	LVITEM item = {0};
	for (int i=0; i<ListView_GetItemCount(m_hListControl); ++i)
	{
		item.mask = LVIF_PARAM|LVIF_STATE;
		item.stateMask = LVIS_SELECTED;
		item.iItem = i;
		ListView_GetItem(m_hListControl, &item);
		if (item.state & LVIS_SELECTED)
		{
			Command cmd = m_commands.GetCommand(i+1);
			CEditCmdDlg dlg(*this);
			dlg.SetCommand(cmd);
			if (dlg.DoModal(hResource, IDD_EDITCMD, *this) == IDOK)
			{
				m_commands.SetCommand(i+1, dlg.GetCommand());
				InitCustomCommandsList();
			}

		}
	}
}