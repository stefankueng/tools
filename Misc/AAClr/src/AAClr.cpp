// AAClr - tool to adjust the monitor brightness

// Copyright (C) 2011-2012 - Stefan Kueng

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
#include "CmdLineParser.h"
#include "MainWindow.h"
#include "Utils.h"
#pragma warning(push)
#pragma warning(disable: 4458) // declaration of 'xx' hides class member
#include <GdiPlus.h>
#pragma warning(pop)


// Global Variables:
HINSTANCE hInst;                                // current instance

// Forward declarations of functions included in this code module:
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                       HINSTANCE hPrevInstance,
                       LPTSTR    lpCmdLine,
                       int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    MSG msg;

    CMainWindow trayWindow(hInstance);

    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    if (trayWindow.RegisterAndCreateWindow())
    {
        HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_AACLR));
        // Main message loop:
        while (GetMessage(&msg, NULL, 0, 0))
        {
            if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        Gdiplus::GdiplusShutdown(gdiplusToken);
        return (int) msg.wParam;
    }
    Gdiplus::GdiplusShutdown(gdiplusToken);
    return 1;
}
