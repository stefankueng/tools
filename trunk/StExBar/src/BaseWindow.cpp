// StExBar - an explorer toolbar

// Copyright (C) 2007-2008, 2012 - Stefan Kueng

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
#include "BaseWindow.h"
#include <shlwapi.h>

#pragma comment(lib, "shlwapi.lib")

ResString::ResString (HINSTANCE hInst, int resId)
{
    if (!::LoadString (hInst, resId, _buf, MAX_RESSTRING + 1))
    {
        SecureZeroMemory(_buf, sizeof(_buf));
    }
}


bool CWindow::RegisterWindow(UINT style, HICON hIcon, HCURSOR hCursor, HBRUSH hbrBackground,
                                    LPCTSTR lpszMenuName, LPCTSTR lpszClassName, HICON hIconSm)
{
    WNDCLASSEX wcx;

    // Fill in the window class structure with default parameters

    wcx.cbSize = sizeof(WNDCLASSEX);                // size of structure
    wcx.style = style;                              // redraw if size changes
    wcx.lpfnWndProc = CWindow::stWinMsgHandler;     // points to window procedure
    wcx.cbClsExtra = 0;                             // no extra class memory
    wcx.cbWndExtra = 0;                             // no extra window memory
    wcx.hInstance = hResource;                      // handle to instance
    wcx.hIcon = hIcon;                              // predefined app. icon
    wcx.hCursor = hCursor;                          // predefined arrow
    wcx.hbrBackground = hbrBackground;              // white background brush
    wcx.lpszMenuName = lpszMenuName;                // name of menu resource
    wcx.lpszClassName = lpszClassName;              // name of window class
    wcx.hIconSm = hIconSm;                          // small class icon

    // Register the window class.
    return RegisterWindow(&wcx);
}

bool CWindow::RegisterWindow(CONST WNDCLASSEX* wcx)
{
    // Register the window class.
    sClassName = std::wstring(wcx->lpszClassName);

    if (RegisterClassEx(wcx) == 0)
    {
        if (GetLastError() == ERROR_CLASS_ALREADY_EXISTS)
            return TRUE;
        return FALSE;
    }
    else
        return TRUE;
}

LRESULT CALLBACK CWindow::stWinMsgHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CWindow* pWnd;

    if (uMsg == WM_NCCREATE)
    {
        // get the pointer to the window from lpCreateParams which was set in CreateWindow
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (long)((LPCREATESTRUCT(lParam))->lpCreateParams));
    }

    // get the pointer to the window
    pWnd = GetObjectFromWindow(hwnd);

    // if we have the pointer, go to the message handler of the window
    // else, use DefWindowProc
    if (pWnd)
    {
        switch (uMsg)
        {
        case WM_ACTIVATE:
            if ((wParam == WA_ACTIVE)&&(!pWnd->bWindowRestored)&&(!pWnd->sRegistryPath.empty()))
            {
                WINDOWPLACEMENT wpl = {0};
                DWORD size = sizeof(wpl);
                if (SHGetValue(HKEY_CURRENT_USER, pWnd->sRegistryPath.c_str(), pWnd->sRegistryValue.c_str(), REG_NONE, &wpl, &size) == ERROR_SUCCESS)
                    SetWindowPlacement(hwnd, &wpl);
                else
                    ShowWindow(hwnd, SW_SHOW);
                pWnd->bWindowRestored = true;
            }
            break;
        case WM_CLOSE:
            if (!pWnd->sRegistryPath.empty())
            {
                WINDOWPLACEMENT wpl = {0};
                wpl.length = sizeof(WINDOWPLACEMENT);
                GetWindowPlacement(hwnd, &wpl);
                SHSetValue(HKEY_CURRENT_USER, pWnd->sRegistryPath.c_str(), pWnd->sRegistryValue.c_str(), REG_NONE, &wpl, sizeof(wpl));
            }
            break;
        }
        return pWnd->WinMsgHandler(hwnd, uMsg, wParam, lParam);
    }
    else
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

bool CWindow::Create()
{
    // Create the window
    RECT rect;

    rect.top = 0;
    rect.left = 0;
    rect.right = 600;
    rect.bottom = 400;

    return Create(WS_OVERLAPPEDWINDOW | WS_VISIBLE, NULL, &rect);
}

bool CWindow::Create(DWORD dwStyles, HWND hParent /* = NULL */, RECT* rect /* = NULL */)
{
    return CreateEx(0, dwStyles, hParent, rect);
}

bool CWindow::CreateEx(DWORD dwExStyles, DWORD dwStyles, HWND hParent /* = NULL */, RECT* rect /* = NULL */)
{
    // send the this pointer as the window creation parameter
    if (rect == NULL)
        m_hwnd = CreateWindowEx(dwExStyles, sClassName.c_str(), sWindowTitle.c_str(), dwStyles, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hParent, NULL, hResource, (void *)this);
    else
    {
        m_hwnd = CreateWindowEx(dwExStyles, sClassName.c_str(), sWindowTitle.c_str(), dwStyles, rect->left, rect->top,
            rect->right - rect->left, rect->bottom - rect->top, hParent, NULL, hResource,
            (void *)this);
    }
    return (m_hwnd != NULL);
}

void CWindow::SetTransparency(BYTE alpha, COLORREF color /* = 0xFF000000 */)
{
    if (alpha == 255)
    {
        LONG_PTR exstyle = GetWindowLongPtr(*this, GWL_EXSTYLE);
        exstyle &= ~WS_EX_LAYERED;
        SetWindowLongPtr(*this, GWL_EXSTYLE, exstyle);
    }
    else
    {
        LONG_PTR exstyle = GetWindowLongPtr(*this, GWL_EXSTYLE);
        exstyle |= WS_EX_LAYERED;
        SetWindowLongPtr(*this, GWL_EXSTYLE, exstyle);
    }
    COLORREF col = color;
    DWORD flags = LWA_ALPHA;
    if (col & 0xFF000000)
    {
        col = RGB(255, 255, 255);
        flags = LWA_ALPHA;
    }
    else
    {
        flags = LWA_COLORKEY;
    }
    SetLayeredWindowAttributes(*this, col, alpha, flags);
}
