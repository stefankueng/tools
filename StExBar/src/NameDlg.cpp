// StExBar - an explorer toolbar

// Copyright (C) 2014, 2018 - Stefan Kueng

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
#include "NameDlg.h"
#include <string>




CNameDlg::CNameDlg(HWND hParent)
    : m_hParent(hParent)
{}

CNameDlg::~CNameDlg(void)
{}

LRESULT CNameDlg::DlgFunc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM /*lParam*/)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            InitDialog(hwndDlg, IDI_RENAME);

            if (!Name.empty())
                SetDlgItemText(*this, IDC_NAME, Name.c_str());
        }
            return (INT_PTR)TRUE;
        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case IDOK:
                case IDCANCEL:
                    Name = GetDlgItemText(IDC_NAME).get();
                    EndDialog(*this, LOWORD(wParam));
                    return (INT_PTR)TRUE;
            }
        }
    }
    return (INT_PTR)FALSE;
}