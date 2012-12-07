// StExBar - an explorer toolbar

// Copyright (C) 2007-2012 - Stefan Kueng

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
#include "OptionsDlg.h"
#include "EditCmdDlg.h"
#include "InfoDlg.h"
#include <string>
#include <regex>

using namespace std;


COptionsDlg::COptionsDlg(HWND hParent) : m_regShowBtnText(_T("Software\\StefansTools\\StExBar\\ShowButtonText"), 1)
    , m_regUseUNCPaths(_T("Software\\StefansTools\\StExBar\\UseUNCPaths"), 1)
    , m_regUseSelector(_T("Software\\StefansTools\\StExBar\\UseSelector"), 1)
    , m_regHideEditBox(_T("Software\\StefansTools\\StExBar\\HideEditBox"), 0)
    , m_regContextMenu(_T("Software\\StefansTools\\StExBar\\ContextMenu"), 1)
    , m_regEditBoxUsage (_T("Software\\StefansTools\\StExBar\\EditBoxUsage"), IDC_USECONSOLE)
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
            SendMessage(GetDlgItem(hwndDlg, IDC_CONTEXTMENU), BM_SETCHECK, DWORD(m_regContextMenu) ? BST_CHECKED : BST_UNCHECKED, 0);
            CheckRadioButton(hwndDlg, IDC_USECONSOLE, IDC_USEAUTO, DWORD(m_regEditBoxUsage));
            EnableWindow(GetDlgItem(hwndDlg, IDC_USECONSOLE), !DWORD(m_regHideEditBox));
            EnableWindow(GetDlgItem(hwndDlg, IDC_USEPOWERSHELL), !DWORD(m_regHideEditBox));
            EnableWindow(GetDlgItem(hwndDlg, IDC_USEFILTER), !DWORD(m_regHideEditBox));
            EnableWindow(GetDlgItem(hwndDlg, IDC_USEAUTO), !DWORD(m_regHideEditBox));
            CRegStdString regGrepWinPath = CRegStdString(L"*\\Shell\\grepWin...\\command\\", L"", 0, HKEY_CLASSES_ROOT);
            wstring grepWinPath = regGrepWinPath;
            EnableWindow(GetDlgItem(hwndDlg, IDC_USEGREPWIN), !grepWinPath.empty() && !DWORD(m_regHideEditBox));

            AddToolTip(IDC_SHOWTEXT, _T("shows the name of the button below the icon"));
            AddToolTip(IDC_USEUNCCHECK, _T("For mounted network drives, copies the UNC path for files/folders\r\ninstead of the path with the mounted drive letter"));
            AddToolTip(IDC_SELECTORCHECK, _T("Determines whether the edit box on the right behaves as a shortcut for the console\r\nor whether it filters items according to the entered mask.\r\nDoes not work on Win7 for folders in a library and\r\ncan be very slow if not in detailed view!"));
            AddToolTip(IDC_HIDEEDITBOX, _T("Hides the edit box on the right of the toolbar"));
            AddToolTip(IDC_CONTEXTMENU, _T("Adds the commands also to the right-click context menu"));
            AddToolTip(IDC_USECONSOLE, _T("The edit box text is sent to the console (cmd.exe)"));
            AddToolTip(IDC_USEPOWERSHELL, _T("The edit box text is sent to the Powershell"));
            AddToolTip(IDC_USEGREPWIN, _T("The edit box text is sent to the grepWin tool if it is installed"));
            AddToolTip(IDC_USEFILTER, _T("The edit box text is used to filter the content in the explorer"));
            AddToolTip(IDC_USEAUTO, _T("The first char determines the function:\nf filtertext\ng grepWin search text\nc console command\np powershell command"));

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
                m_regShowBtnText = (DWORD)SendMessage(GetDlgItem(hwndDlg, IDC_SHOWTEXT), BM_GETCHECK, 0, 0);
                m_regUseUNCPaths = (DWORD)SendMessage(GetDlgItem(hwndDlg, IDC_USEUNCCHECK), BM_GETCHECK, 0, 0);
                m_regUseSelector = (DWORD)SendMessage(GetDlgItem(hwndDlg, IDC_SELECTORCHECK), BM_GETCHECK, 0, 0);
                m_regHideEditBox = (DWORD)SendMessage(GetDlgItem(hwndDlg, IDC_HIDEEDITBOX), BM_GETCHECK, 0, 0);
                m_regContextMenu = (DWORD)SendMessage(GetDlgItem(hwndDlg, IDC_CONTEXTMENU), BM_GETCHECK, 0, 0);
                if (IsDlgButtonChecked(hwndDlg, IDC_USECONSOLE))
                    m_regEditBoxUsage = IDC_USECONSOLE;
                else if (IsDlgButtonChecked(hwndDlg, IDC_USEPOWERSHELL))
                    m_regEditBoxUsage = IDC_USEPOWERSHELL;
                else if (IsDlgButtonChecked(hwndDlg, IDC_USEGREPWIN))
                    m_regEditBoxUsage = IDC_USEGREPWIN;
                else if (IsDlgButtonChecked(hwndDlg, IDC_USEFILTER))
                    m_regEditBoxUsage = IDC_USEFILTER;
                else //if (IsDlgButtonChecked(hwndDlg, IDC_USEAUTO))
                    m_regEditBoxUsage = IDC_USEAUTO;
                m_commands.SaveToFile();
            }
            // fall through
        case IDCANCEL:
            EndDialog(*this, LOWORD(wParam));
            return (INT_PTR)TRUE;
        case IDC_OPTIONSHELP:
            CInfoDlg::ShowDialog(IDR_OPTIONSHELP, hResource);
            break;
        case IDC_HIDEEDITBOX:
            {
                bool bHide = SendMessage(GetDlgItem(hwndDlg, IDC_HIDEEDITBOX), BM_GETCHECK, 0, 0) != BST_CHECKED;
                EnableWindow(GetDlgItem(hwndDlg, IDC_USECONSOLE), bHide);
                EnableWindow(GetDlgItem(hwndDlg, IDC_USEPOWERSHELL), bHide);
                EnableWindow(GetDlgItem(hwndDlg, IDC_USEFILTER), bHide);
                EnableWindow(GetDlgItem(hwndDlg, IDC_USEAUTO), bHide);
                CRegStdString regGrepWinPath = CRegStdString(L"*\\Shell\\grepWin...\\command\\", L"", 0, HKEY_CLASSES_ROOT);
                wstring grepWinPath = regGrepWinPath;
                EnableWindow(GetDlgItem(hwndDlg, IDC_USEGREPWIN), !grepWinPath.empty() && bHide);
            }
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
            if ((lpnmhdr->code == NM_CUSTOMDRAW)&&(lpnmhdr->hwndFrom == m_hListControl))
            {
                return OnCustomDrawListItem((LPNMLVCUSTOMDRAW)lParam);
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
    lvc.pszText = _T("Hotkey");
    ListView_InsertColumn(m_hListControl, 1, &lvc);

    LVITEM item = {0};
    TCHAR buf[1024];
    TCHAR buf2[1024];
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
        if ((!cmd.separator)&&(cmd.key.keycode))
        {
            wstring sKeyText;
            if (cmd.key.control)
                sKeyText += _T("Ctrl+");
            if (cmd.key.shift)
                sKeyText += _T("Shift+");
            if (cmd.key.alt)
                sKeyText += _T("Alt+");
            sKeyText += (wchar_t)cmd.key.keycode;
            _tcscpy_s(buf2, 1024, sKeyText.c_str());
            ListView_SetItemText(m_hListControl, i, 1, buf2);
        }
    }
    ListView_SetColumnWidth(m_hListControl, 0, LVSCW_AUTOSIZE_USEHEADER);
    ListView_SetColumnWidth(m_hListControl, 1, LVSCW_AUTOSIZE_USEHEADER);

    // disable the edit and remove button since nothing is selected now
    EnableWindow(GetDlgItem(*this, IDC_EDITCMD), FALSE);
    EnableWindow(GetDlgItem(*this, IDC_REMOVE), FALSE);
    EnableWindow(GetDlgItem(*this, IDC_MOVEUP), FALSE);
    EnableWindow(GetDlgItem(*this, IDC_MOVEDOWN), FALSE);
    ::InvalidateRect(m_hListControl, NULL, false);
}

void COptionsDlg::OnSelectListItem(LPNMLISTVIEW /*lpNMListView*/)
{
    LVITEM item = {0};
    UINT nCount = ListView_GetSelectedCount(m_hListControl);
    bool bIsSeparator = false;
    bool bIsInternal = false;
    bool bIsHidden = false;
    bool bIsOptions = false;
    for (int i=0; i<ListView_GetItemCount(m_hListControl); ++i)
    {
        item.mask = LVIF_PARAM|LVIF_STATE;
        item.stateMask = LVIS_SELECTED;
        item.iItem = i;
        ListView_GetItem(m_hListControl, &item);
        if (item.state & LVIS_SELECTED)
        {
            Command * pCmd = m_commands.GetCommandPtr(i+1);
            bIsSeparator = pCmd->separator;
            bIsHidden = pCmd->commandline.compare(INTERNALCOMMANDHIDDEN)==0;
            bIsInternal = (pCmd->commandline.compare(INTERNALCOMMAND) || bIsHidden);
            bIsOptions = pCmd->name.compare(_T("Options")) == 0;
        }
    }

    if (bIsHidden)
        ::SetWindowText(GetDlgItem(*this, IDC_REMOVE), _T("Activate"));
    else
        ::SetWindowText(GetDlgItem(*this, IDC_REMOVE), _T("Remove"));

    if (bIsOptions)
    {
        EnableWindow(GetDlgItem(*this, IDC_EDITCMD), FALSE);
        EnableWindow(GetDlgItem(*this, IDC_REMOVE), FALSE);
        EnableWindow(GetDlgItem(*this, IDC_MOVEUP), nCount == 1);
        EnableWindow(GetDlgItem(*this, IDC_MOVEDOWN), nCount == 1);
    }
    else
    {
        EnableWindow(GetDlgItem(*this, IDC_EDITCMD), nCount == 1 && !bIsSeparator);
        EnableWindow(GetDlgItem(*this, IDC_REMOVE), nCount == 1);
        EnableWindow(GetDlgItem(*this, IDC_MOVEUP), nCount == 1);
        EnableWindow(GetDlgItem(*this, IDC_MOVEDOWN), nCount == 1);
    }
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
                ListView_EnsureVisible(m_hListControl, i-1, FALSE);
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
                ListView_EnsureVisible(m_hListControl, i+1, FALSE);
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
            Command * pCmd = m_commands.GetCommandPtr(i+1);
            if ((pCmd)&&(pCmd->commandline.compare(INTERNALCOMMAND)==0))
            {
                pCmd->commandline = INTERNALCOMMANDHIDDEN;
            }
            else if ((pCmd)&&(pCmd->commandline.compare(INTERNALCOMMANDHIDDEN)==0))
            {
                pCmd->commandline = INTERNALCOMMAND;
            }
            else
                m_commands.RemoveCommand(i+1);
            InitCustomCommandsList();
            ListView_EnsureVisible(m_hListControl, i-1, FALSE);
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
            if (cmd.separator)
                continue;
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

LRESULT COptionsDlg::OnCustomDrawListItem(LPNMLVCUSTOMDRAW lpNMCustomDraw)
{
    // First thing - check the draw stage. If it's the control's prepaint
    // stage, then tell Windows we want messages for every item.
    LRESULT result =  CDRF_DODEFAULT;
    switch (lpNMCustomDraw->nmcd.dwDrawStage)
    {
    case CDDS_PREPAINT:
        result = CDRF_NOTIFYITEMDRAW;
        break;
    case CDDS_ITEMPREPAINT:
        {
            COLORREF crText = GetSysColor(COLOR_WINDOWTEXT);
            if (m_commands.GetCommand((int)lpNMCustomDraw->nmcd.dwItemSpec+1).commandline.compare(INTERNALCOMMANDHIDDEN) == 0)
                crText = GetSysColor(COLOR_GRAYTEXT);
            lpNMCustomDraw->clrText = crText;
            result = CDRF_NOTIFYSUBITEMDRAW;
        }
        break;
    }
    return result;
}
