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
#include "FileTool.h"
#include "FileToolDlg.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE g_hInst = 0;                                // current instance

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    SetDllDirectory(L"");
    ::OleInitialize(NULL);
    ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    // we need some of the common controls
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_LINK_CLASS|ICC_LISTVIEW_CLASSES|ICC_PAGESCROLLER_CLASS
        |ICC_PROGRESS_CLASS|ICC_STANDARD_CLASSES|ICC_TAB_CLASSES|ICC_TREEVIEW_CLASSES
        |ICC_UPDOWN_CLASS|ICC_USEREX_CLASSES|ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icex);

    g_hInst = hInstance;
    CFileToolDlg mainDlg(NULL);

    return (int) mainDlg.DoModal(g_hInst, IDD_FILETOOL, NULL);
}


