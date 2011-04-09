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
#pragma once
#include "basewindow.h"
#include "Resource.h"
#include "shellapi.h"
#include "shlwapi.h"
#include <commctrl.h>



class CMainWindow : public CWindow
{
public:
    CMainWindow(HINSTANCE hInst, const WNDCLASSEX* wcx = NULL)
        : CWindow(hInst, wcx)
        , hwndNextViewer(NULL)
        , foregroundWND(NULL)
    {
        SetWindowTitle((LPCTSTR)ResString(hResource, IDS_APP_TITLE));
    };

    ~CMainWindow(void)
    {
    };

    bool                RegisterAndCreateWindow();

protected:
    /// the message handler for this window
    LRESULT CALLBACK    WinMsgHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT             HandleCustomMessages(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    /// Handles all the WM_COMMAND window messages (e.g. menu commands)
    LRESULT             DoCommand(int id);

    void                ShowTrayIcon();
    DWORD               GetDllVersion(LPCTSTR lpszDllName);

protected:
    NOTIFYICONDATA      niData;
    HWND                hwndNextViewer;
    HWND                foregroundWND;

    typedef BOOL(__stdcall *PFNCHANGEWINDOWMESSAGEFILTER)(UINT message, DWORD dwFlag);
    static PFNCHANGEWINDOWMESSAGEFILTER m_pChangeWindowMessageFilter;
};
