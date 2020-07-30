// StExBar - an explorer toolbar

// Copyright (C) 2007-2009, 2011-2014, 2020 - Stefan Kueng

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
#include "EditCmdDlg.h"
#include "InfoRtfDialog.h"
#include "BrowseFolder.h"
#include <string>

#include <regex>

CEditCmdDlg::CEditCmdDlg(HWND hParent)
    : m_hParent(hParent)
{
}

CEditCmdDlg::~CEditCmdDlg(void)
{
}

LRESULT CEditCmdDlg::DlgFunc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM /*lParam*/)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            InitDialog(hwndDlg, IDI_OPTIONS);

            SetupControls();

            AddToolTip(IDC_SEPARATOR, L"When enabled, the entry is made a separator");
            AddToolTip(IDC_INTERNALHIDE, L"Hides an internal command, the command won't show up in the toolbar");
            AddToolTip(IDC_NAME, L"The name for the command.\r\nEach command must have a unique name.");
            AddToolTip(IDC_ICONPATH, L"Path to an icon file to use for the toolbar button.\r\nLeave this empty to use the default icon");
            AddToolTip(IDC_COMMANDLINE, L"Command line to execute. Environment variables are expanded properly.\r\n\
Special placeholders are available:\r\n\
%selpaths\t: replaced with the paths of the selected items, separated by a space\r\n\
%sel*paths\t: replaced with the paths of the selected items, separated by '*' char\r\n\
%selafile\t\t: replaced with the path to a file containing the paths in ANSI of the selected items separated by a newline char\r\n\
%selufile\t\t: replaced with the path to a file containing the paths in UNICODE of the selected items separated by a newline char\r\n\
%selnames\t: replaced with the names of the selected items\r\n\
%curdir\t\t: replaced with the path of the currently shown directory\r\n\
%cmdtext\t: replaced with the content of the edit box\r\n");

            AddToolTip(IDC_STARTIN, L"Path to a directory in which the tool is started in");
            AddToolTip(IDC_VIEWPATH, L"Enables the command when the explorer shows a file system path");
            AddToolTip(IDC_NOVIEWPATH, L"Enables the command when the explorer shows a non file system path, e.g., the printers view");
            AddToolTip(IDC_FILESELECTED, L"Enables the command when one or more files in the explorer are selected");
            AddToolTip(IDC_FOLDERSELECTED, L"Enables the command when one or more directories in the explorer are selected");
            AddToolTip(IDC_SELECTED, L"Enables the command when one or more items (files, folders or other items, e.g. printers) in the explorer are selected");
            AddToolTip(IDC_NOSELECTION, L"Enables the command when no item in the explorer is selected");
            AddToolTip(IDC_SELECTEDCOUNT, L"Enables the command when nothing or the amount of items set in this box are selected");
        }
            return (INT_PTR)TRUE;
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                {
                    if (!SetupCommand())
                        return TRUE;
                }
                    // fall through
                case IDCANCEL:
                    EndDialog(*this, LOWORD(wParam));
                    return (INT_PTR)TRUE;
                case IDC_BROWSEICON:
                {
                    OPENFILENAME ofn              = {0}; // common dialog box structure
                    wchar_t      szFile[MAX_PATH] = {0}; // buffer for file name. Explorer can't handle paths longer than MAX_PATH.
                    ofn.lStructSize               = sizeof(OPENFILENAME);
                    ofn.hwndOwner                 = *this;
                    ofn.lpstrFile                 = szFile;
                    ofn.nMaxFile                  = _countof(szFile);
                    ofn.lpstrFilter               = L"Icons (*.ico, *.dll)\0*.ico;*.dll\0All files (*.*)\0*.*\0\0";
                    ofn.nFilterIndex              = 1;
                    ofn.lpstrFileTitle            = NULL;
                    ofn.nMaxFileTitle             = 0;
                    ofn.lpstrInitialDir           = NULL;
                    ofn.lpstrTitle                = L"Select icon";
                    ofn.Flags                     = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_EXPLORER;

                    // Display the Open dialog box.
                    bool bRet = !!GetOpenFileName(&ofn);
                    if (bRet)
                    {
                        SetDlgItemText(*this, IDC_ICONPATH, ofn.lpstrFile);
                    }
                }
                break;
                case IDC_BROWSECOMMAND:
                {
                    OPENFILENAME ofn              = {0}; // common dialog box structure
                    wchar_t      szFile[MAX_PATH] = {0}; // buffer for file name. Explorer can't handle paths longer than MAX_PATH.
                    ofn.lStructSize               = sizeof(OPENFILENAME);
                    ofn.hwndOwner                 = *this;
                    ofn.lpstrFile                 = szFile;
                    ofn.nMaxFile                  = _countof(szFile);
                    ofn.lpstrFilter               = L"Executables\0*.exe;*.dll;*.cmd;*.vbs;*.js;*.bat\0All files (*.*)\0*.*\0\0";
                    ofn.nFilterIndex              = 1;
                    ofn.lpstrFileTitle            = NULL;
                    ofn.nMaxFileTitle             = 0;
                    ofn.lpstrInitialDir           = NULL;
                    ofn.lpstrTitle                = L"Select Application";
                    ofn.Flags                     = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_EXPLORER;

                    // Display the Open dialog box.
                    bool bRet = !!GetOpenFileName(&ofn);
                    if (bRet)
                    {
                        SetDlgItemText(*this, IDC_COMMANDLINE, ofn.lpstrFile);
                    }
                }
                break;
                case IDC_BROWSESTARTIN:
                {
                    CBrowseFolder folderBrowser;
                    WCHAR         path[MAX_PATH] = {0};
                    if (folderBrowser.Show(*this, path, _countof(path)) == CBrowseFolder::OK)
                    {
                        SetDlgItemText(*this, IDC_STARTIN, path);
                    }
                }
                break;
                case IDC_SEPARATOR:
                    m_command.separator = SendMessage(GetDlgItem(*this, IDC_SEPARATOR), BM_GETCHECK, 0, 0) == BST_CHECKED;
                    SetSeparator(m_command.separator);
                    break;
                case IDC_COMMANDSHELP:
                {
                    CInfoRtfDialog dlg;
                    dlg.DoModal(hResource, *this, "StExBar Commands", IDR_COMMANDSHELP, L"rtf", IDI_OPTIONS, 300, 400);
                }
                break;
            }
            break;
    }
    return (INT_PTR)FALSE;
}

void CEditCmdDlg::SetSeparator(bool bSeparator)
{
    SendMessage(GetDlgItem(*this, IDC_SEPARATOR), BM_SETCHECK, m_command.separator ? BST_CHECKED : BST_UNCHECKED, 0);
    EnableWindow(GetDlgItem(*this, IDC_NAME), !bSeparator);
    EnableWindow(GetDlgItem(*this, IDC_ICONPATH), !bSeparator);
    EnableWindow(GetDlgItem(*this, IDC_COMMANDLINE), !bSeparator);
    EnableWindow(GetDlgItem(*this, IDC_STARTIN), !bSeparator);
    EnableWindow(GetDlgItem(*this, IDC_HOTKEY), !bSeparator);
    EnableWindow(GetDlgItem(*this, IDC_VIEWPATH), !bSeparator);
    EnableWindow(GetDlgItem(*this, IDC_NOVIEWPATH), !bSeparator);
    EnableWindow(GetDlgItem(*this, IDC_FILESELECTED), !bSeparator);
    EnableWindow(GetDlgItem(*this, IDC_FOLDERSELECTED), !bSeparator);
    EnableWindow(GetDlgItem(*this, IDC_SELECTED), !bSeparator);
    EnableWindow(GetDlgItem(*this, IDC_NOSELECTION), !bSeparator);
    EnableWindow(GetDlgItem(*this, IDC_SELECTEDCOUNT), !bSeparator);
}

void CEditCmdDlg::SetupControls()
{
    SetSeparator(m_command.separator);
    if (m_command.separator)
        return; // nothing more to do here

    SetDlgItemText(*this, IDC_NAME, m_command.name.c_str());
    SetDlgItemText(*this, IDC_ICONPATH, m_command.icon.c_str());
    SetDlgItemText(*this, IDC_COMMANDLINE, m_command.commandline.c_str());
    SetDlgItemText(*this, IDC_STARTIN, m_command.startin.c_str());

    WPARAM hk = m_command.key.keycode;
    hk |= m_command.key.alt ? HOTKEYF_ALT << 8 : 0;
    hk |= m_command.key.shift ? HOTKEYF_SHIFT << 8 : 0;
    hk |= m_command.key.control ? HOTKEYF_CONTROL << 8 : 0;
    SendMessage(GetDlgItem(*this, IDC_HOTKEY), HKM_SETHOTKEY, hk, 0);

    SendMessage(GetDlgItem(*this, IDC_VIEWPATH), BM_SETCHECK, m_command.enabled_viewpath ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessage(GetDlgItem(*this, IDC_NOVIEWPATH), BM_SETCHECK, m_command.enabled_noviewpath ? BST_CHECKED : BST_UNCHECKED, 0);

    SendMessage(GetDlgItem(*this, IDC_FILESELECTED), BM_SETCHECK, m_command.enabled_fileselected ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessage(GetDlgItem(*this, IDC_FOLDERSELECTED), BM_SETCHECK, m_command.enabled_folderselected ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessage(GetDlgItem(*this, IDC_SELECTED), BM_SETCHECK, m_command.enabled_selected ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessage(GetDlgItem(*this, IDC_NOSELECTION), BM_SETCHECK, m_command.enabled_noselection ? BST_CHECKED : BST_UNCHECKED, 0);

    wchar_t buf[40] = {0};
    swprintf_s(buf, _countof(buf), L"%d", m_command.enabled_selectedcount);
    SetDlgItemText(*this, IDC_SELECTEDCOUNT, buf);

    // disable controls which must not be changed for internal commands
    if ((m_command.commandline.compare(INTERNALCOMMAND) == 0) || (m_command.commandline.compare(INTERNALCOMMANDHIDDEN) == 0))
    {
        ShowWindow(GetDlgItem(*this, IDC_INTERNALHIDE), m_command.name.compare(L"Options") == 0 ? SW_SHOW : SW_HIDE);
        SendMessage(GetDlgItem(*this, IDC_INTERNALHIDE), BM_SETCHECK, (m_command.commandline.compare(INTERNALCOMMANDHIDDEN) == 0) ? BST_CHECKED : BST_UNCHECKED, 0);

        EnableWindow(GetDlgItem(*this, IDC_SEPARATOR), FALSE);
        EnableWindow(GetDlgItem(*this, IDC_NAME), FALSE);
        EnableWindow(GetDlgItem(*this, IDC_ICONPATH), FALSE);
        EnableWindow(GetDlgItem(*this, IDC_COMMANDLINE), FALSE);
        EnableWindow(GetDlgItem(*this, IDC_STARTIN), FALSE);
        EnableWindow(GetDlgItem(*this, IDC_VIEWPATH), FALSE);
        EnableWindow(GetDlgItem(*this, IDC_NOVIEWPATH), FALSE);
        EnableWindow(GetDlgItem(*this, IDC_FILESELECTED), FALSE);
        EnableWindow(GetDlgItem(*this, IDC_FOLDERSELECTED), FALSE);
        EnableWindow(GetDlgItem(*this, IDC_SELECTED), FALSE);
        EnableWindow(GetDlgItem(*this, IDC_NOSELECTION), FALSE);
        EnableWindow(GetDlgItem(*this, IDC_SELECTEDCOUNT), FALSE);
    }
    else
    {
        ShowWindow(GetDlgItem(*this, IDC_INTERNALHIDE), SW_HIDE);
        EnableWindow(GetDlgItem(*this, IDC_SEPARATOR), TRUE);
    }
}

bool CEditCmdDlg::SetupCommand()
{
    m_command.separator = SendMessage(GetDlgItem(*this, IDC_SEPARATOR), BM_GETCHECK, 0, 0) == BST_CHECKED;
    if (m_command.separator)
        return true; // nothing more to do here

    auto buf       = GetDlgItemText(IDC_NAME);
    m_command.name = buf.get();
    if (m_command.name.empty())
    {
        ShowEditBalloon(IDC_NAME, L"missing text", L"please enter a name for the command");
        return false;
    }
    buf                   = GetDlgItemText(IDC_ICONPATH);
    m_command.icon        = buf.get();
    buf                   = GetDlgItemText(IDC_COMMANDLINE);
    m_command.commandline = buf.get();
    buf                   = GetDlgItemText(IDC_STARTIN);
    m_command.startin     = buf.get();

    WPARAM hk             = SendMessage(GetDlgItem(*this, IDC_HOTKEY), HKM_GETHOTKEY, 0, 0);
    m_command.key.keycode = LOBYTE(hk);
    m_command.key.alt     = (HIBYTE(hk) & HOTKEYF_ALT) != 0;
    m_command.key.shift   = (HIBYTE(hk) & HOTKEYF_SHIFT) != 0;
    m_command.key.control = (HIBYTE(hk) & HOTKEYF_CONTROL) != 0;

    m_command.enabled_viewpath   = SendMessage(GetDlgItem(*this, IDC_VIEWPATH), BM_GETCHECK, 0, 0) == BST_CHECKED;
    m_command.enabled_noviewpath = SendMessage(GetDlgItem(*this, IDC_NOVIEWPATH), BM_GETCHECK, 0, 0) == BST_CHECKED;

    m_command.enabled_fileselected   = SendMessage(GetDlgItem(*this, IDC_FILESELECTED), BM_GETCHECK, 0, 0) == BST_CHECKED;
    m_command.enabled_folderselected = SendMessage(GetDlgItem(*this, IDC_FOLDERSELECTED), BM_GETCHECK, 0, 0) == BST_CHECKED;
    m_command.enabled_selected       = SendMessage(GetDlgItem(*this, IDC_SELECTED), BM_GETCHECK, 0, 0) == BST_CHECKED;
    m_command.enabled_noselection    = SendMessage(GetDlgItem(*this, IDC_NOSELECTION), BM_GETCHECK, 0, 0) == BST_CHECKED;

    buf                             = GetDlgItemText(IDC_SELECTEDCOUNT);
    m_command.enabled_selectedcount = _ttol(buf.get());

    if ((m_command.commandline.compare(INTERNALCOMMAND) == 0) || (m_command.commandline.compare(INTERNALCOMMANDHIDDEN) == 0))
    {
        if (SendMessage(GetDlgItem(*this, IDC_INTERNALHIDE), BM_GETCHECK, 0, 0) == BST_CHECKED)
        {
            m_command.commandline = INTERNALCOMMANDHIDDEN;
        }
        else
        {
            m_command.commandline = INTERNALCOMMAND;
        }
    }
    return true;
}
