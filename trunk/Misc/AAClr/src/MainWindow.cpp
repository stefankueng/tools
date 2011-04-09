// AAClr - tool to adjust the aero colors according to the desktop wallpaper

// Copyright (C) 2011 - Stefan Kueng

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
#include "StdAfx.h"
#include "MainWindow.h"
#include "NotifySlider.h"
#include "AboutDlg.h"
#include "OptionsDlg.h"

#include <WindowsX.h>

#define TRAY_WM_MESSAGE     WM_APP+1

#define TIMER_CLIPWAIT 100


static UINT WM_TASKBARCREATED = RegisterWindowMessage(_T("TaskbarCreated"));
CMainWindow::PFNCHANGEWINDOWMESSAGEFILTER CMainWindow::m_pChangeWindowMessageFilter = NULL;

#define PACKVERSION(major,minor) MAKELONG(minor,major)

DWORD CMainWindow::GetDllVersion(LPCTSTR lpszDllName)
{
    HINSTANCE hinstDll;
    DWORD dwVersion = 0;

    hinstDll = LoadLibrary(lpszDllName);

    if (hinstDll)
    {
        DLLGETVERSIONPROC pDllGetVersion;
        pDllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hinstDll,
            "DllGetVersion");

        if (pDllGetVersion)
        {
            DLLVERSIONINFO dvi;
            HRESULT hr;

            ZeroMemory(&dvi, sizeof(dvi));
            dvi.cbSize = sizeof(dvi);

            hr = (*pDllGetVersion)(&dvi);

            if (SUCCEEDED(hr))
            {
                dwVersion = PACKVERSION(dvi.dwMajorVersion, dvi.dwMinorVersion);
            }
        }

        FreeLibrary(hinstDll);
    }
    return dwVersion;
}

bool CMainWindow::RegisterAndCreateWindow()
{
    WNDCLASSEX wcx;

    // Fill in the window class structure with default parameters
    wcx.cbSize = sizeof(WNDCLASSEX);
    wcx.style = CS_HREDRAW | CS_VREDRAW;
    wcx.lpfnWndProc = CWindow::stWinMsgHandler;
    wcx.cbClsExtra = 0;
    wcx.cbWndExtra = 0;
    wcx.hInstance = hResource;
    wcx.hCursor = NULL;
    wcx.lpszClassName = ResString(hResource, IDS_APP_TITLE);
    wcx.hIcon = LoadIcon(hResource, MAKEINTRESOURCE(IDI_AACLR));
    wcx.hbrBackground = NULL;
    wcx.lpszMenuName = NULL;
    wcx.hIconSm = LoadIcon(wcx.hInstance, MAKEINTRESOURCE(IDI_AACLR));
    if (RegisterWindow(&wcx))
    {
        if (CreateEx(NULL, WS_POPUP, NULL))
        {
            // On Vista, the message TasbarCreated may be blocked by the message filter.
            // We try to change the filter here to get this message through. If even that
            // fails, then we can't do much about it and the task bar icon won't show up again.
            HMODULE hLib = LoadLibrary(_T("user32.dll"));
            if (hLib)
            {
                m_pChangeWindowMessageFilter = (CMainWindow::PFNCHANGEWINDOWMESSAGEFILTER)GetProcAddress(hLib, "ChangeWindowMessageFilter");
                if (m_pChangeWindowMessageFilter)
                {
                    (*m_pChangeWindowMessageFilter)(WM_TASKBARCREATED, MSGFLT_ADD);
                }
            }

            ShowTrayIcon();
            return true;
        }
    }
    return false;
}

void CMainWindow::ShowTrayIcon()
{
    // since our main window is hidden most of the time
    // we have to add an auxiliary window to the system tray
    ZeroMemory(&niData,sizeof(NOTIFYICONDATA));

    ULONGLONG ullVersion = GetDllVersion(_T("Shell32.dll"));
    if (ullVersion >= MAKEDLLVERULL(6,0,0,0))
        niData.cbSize = sizeof(NOTIFYICONDATA);
    else if(ullVersion >= MAKEDLLVERULL(5,0,0,0))
        niData.cbSize = NOTIFYICONDATA_V2_SIZE;
    else niData.cbSize = NOTIFYICONDATA_V1_SIZE;

    niData.uID = IDI_AACLR;
    niData.uFlags = NIF_ICON|NIF_MESSAGE|NIF_TIP|NIF_INFO;

    niData.hIcon = (HICON)LoadImage(hResource, MAKEINTRESOURCE(IDI_AACLR),
        IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
    niData.hWnd = *this;
    niData.uCallbackMessage = TRAY_WM_MESSAGE;
    niData.uVersion = 6;

    Shell_NotifyIcon(NIM_DELETE,&niData);
    Shell_NotifyIcon(NIM_ADD,&niData);
    Shell_NotifyIcon(NIM_SETVERSION,&niData);
    DestroyIcon(niData.hIcon);
}

LRESULT CMainWindow::HandleCustomMessages(HWND /*hwnd*/, UINT uMsg, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    if (uMsg == WM_TASKBARCREATED)
    {
        ShowTrayIcon();
    }
    return 0L;
}

LRESULT CALLBACK CMainWindow::WinMsgHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        {
            m_hwnd = hwnd;
        }
        break;
    case WM_COMMAND:
        return DoCommand(LOWORD(wParam));
        break;
    case TRAY_WM_MESSAGE:
        {
            switch(lParam)
            {
            case WM_RBUTTONUP:
            case WM_CONTEXTMENU:
                {
                    POINT pt;
                    GetCursorPos(&pt);
                    HMENU hMenu = LoadMenu(hResource, MAKEINTRESOURCE(IDC_AACLR));
                    HMENU hPopMenu = GetSubMenu(hMenu, 0);
                    SetForegroundWindow(*this);
                    TrackPopupMenu(hPopMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, *this, NULL);
                    DestroyMenu(hMenu);
                }
                break;
            case WM_LBUTTONDOWN:
                {
                    POINT pt;
                    GetCursorPos(&pt);
                    CNotifySlider dlg(NULL);
                    dlg.xPos = pt.x;
                    dlg.yPos = pt.y;
                    dlg.DoModal(hResource, IDD_NOTIFYSLIDER, NULL);
                }
                break;
            }
        }
        break;
    case WM_DESTROY:
        Shell_NotifyIcon(NIM_DELETE,&niData);
        PostQuitMessage(0);
        break;
    case WM_TIMER:
        if (wParam == TIMER_CLIPWAIT)
        {
            KillTimer(*this, TIMER_CLIPWAIT);
        }
        break;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
};

LRESULT CMainWindow::DoCommand(int id)
{
    switch (id)
    {
    case IDM_EXIT:
        Shell_NotifyIcon(NIM_DELETE,&niData);
        ::PostQuitMessage(0);
        return 0;
        break;
    case IDM_ABOUT:
        {
            CAboutDlg dlg(NULL);
            dlg.DoModal(hResource, IDD_ABOUTBOX, NULL);
        }
        break;
    case IDM_OPTIONS:
        {
            COptionsDlg dlg(NULL);
            dlg.DoModal(hResource, IDD_OPTIONS, NULL);
        }
        break;
    default:
        break;
    };
    return 1;
}


