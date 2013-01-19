// FileTool

// Copyright (C) 2013 - Stefan Kueng

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
#include "resource.h"
#include "CleanVerifyDlg.h"


CCleanVerifyDlg::CCleanVerifyDlg(HWND hParent, const std::wstring& path)
    : m_hParent(hParent)
    , m_path(path)
{
}

CCleanVerifyDlg::~CCleanVerifyDlg(void)
{
}

LRESULT CCleanVerifyDlg::DlgFunc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            InitDialog(hwndDlg, IDI_FILETOOL);
            SetDlgItemText(*this, IDC_PATH, m_path.c_str());
        }
        return TRUE;
    case WM_COMMAND:
        return DoCommand(LOWORD(wParam));
    default:
        return FALSE;
    }
}


LRESULT CCleanVerifyDlg::DoCommand(int id)
{
    switch (id)
    {
    case IDOK:
        {
            std::wstring verString = GetDlgItemText(IDC_VERIFYBOX).get();
            if (verString.compare(L"OK"))
                return 1;
        }
        // intentional fall through
    case IDCANCEL:
        EndDialog(*this, id);
        break;
    }
    return 1;
}

