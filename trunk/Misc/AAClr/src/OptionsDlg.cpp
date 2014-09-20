// AAClr - tool to adjust the aero colors according to the desktop wallpaper

// Copyright (C) 2011-2012, 2014 - Stefan Kueng

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
#include "OptionsDlg.h"
#include "Registry.h"
#include <string>
#include <Commdlg.h>

COptionsDlg::COptionsDlg(HWND hParent)
    : m_hParent(hParent)
    , randomcolors(false)
    , brightness(false)
{
}

COptionsDlg::~COptionsDlg(void)
{
}

LRESULT COptionsDlg::DlgFunc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            InitDialog(hwndDlg, IDI_AACLR);

            AddToolTip(IDC_AUTOSTART, _T("Starts AAClr automatically when Windows starts up."));

            // initialize the controls
            CRegStdString regRun = CRegStdString(_T("Software\\Microsoft\\Windows\\CurrentVersion\\Run\\AAClr"));
            bool bStartWithWindows = !std::wstring(regRun).empty();
            SendDlgItemMessage(*this, IDC_AUTOSTART, BM_SETCHECK, bStartWithWindows ? BST_CHECKED : BST_UNCHECKED, NULL);

            randomcolors = !!CRegStdDWORD(_T("Software\\AAClr\\randomcolors"), 1);
            SendDlgItemMessage(*this, IDC_RANDOMCOLOR, BM_SETCHECK, randomcolors ? BST_CHECKED : BST_UNCHECKED, NULL);

            brightness = !!CRegStdDWORD(_T("Software\\AAClr\\brightness"), 1);
            SendDlgItemMessage(*this, IDC_BRIGHTNESS, BM_SETCHECK, brightness ? BST_CHECKED : BST_UNCHECKED, NULL);

            ExtendFrameIntoClientArea(0, 0, 0, 0);
            m_aerocontrols.SubclassControl(GetDlgItem(*this, IDC_AUTOSTART));
            m_aerocontrols.SubclassControl(GetDlgItem(*this, IDC_RANDOMCOLOR));
            m_aerocontrols.SubclassControl(GetDlgItem(*this, IDC_BRIGHTNESS));
            m_aerocontrols.SubclassControl(GetDlgItem(*this, IDOK));
            m_aerocontrols.SubclassControl(GetDlgItem(*this, IDCANCEL));
        }
        return TRUE;
    case WM_COMMAND:
        return DoCommand(LOWORD(wParam));
    default:
        return FALSE;
    }
}

LRESULT COptionsDlg::DoCommand(int id)
{
    switch (id)
    {
    case IDOK:
        {
            CRegStdString regStartWithWindows = CRegStdString(_T("Software\\Microsoft\\Windows\\CurrentVersion\\Run\\AAClr"));
            bool bStartWithWindows = !!SendDlgItemMessage(*this, IDC_AUTOSTART, BM_GETCHECK, 0, NULL);
            if (bStartWithWindows)
            {
                TCHAR buf[MAX_PATH*4];
                GetModuleFileName(NULL, buf, _countof(buf));
                std::wstring cmd = std::wstring(buf);
                regStartWithWindows = cmd;
            }
            else
                regStartWithWindows.removeValue();

            CRegStdDWORD regrandomcolors(_T("Software\\AAClr\\randomcolors"), 1);
            randomcolors = !!SendDlgItemMessage(*this, IDC_RANDOMCOLOR, BM_GETCHECK, 0, NULL);
            regrandomcolors = randomcolors;
            CRegStdDWORD regbrightness(_T("Software\\AAClr\\brightness"), 1);
            brightness = !!SendDlgItemMessage(*this, IDC_BRIGHTNESS, BM_GETCHECK, 0, NULL);
            regbrightness = brightness;
        }
        // fall through
    case IDCANCEL:
        EndDialog(*this, id);
        break;
    }
    return 1;
}
