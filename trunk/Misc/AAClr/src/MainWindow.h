// AAClr - tool to adjust the aero colors according to the desktop wallpaper

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

#pragma once
#include "BaseWindow.h"
#include "resource.h"
#include "AeroColors.h"
#include "Registry.h"
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
        , randomcolors(false)
    {
        SetWindowTitle((LPCTSTR)ResString(hResource, IDS_APP_TITLE));
        randomcolors = !!CRegStdDWORD(_T("Software\\AAClr\\randomcolors"), 1);
        brightness = !!CRegStdDWORD(_T("Software\\AAClr\\brightness"), 1);
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

    static unsigned int __stdcall WatcherThread(LPVOID lpvParam);

protected:
    NOTIFYICONDATA      niData;
    HWND                hwndNextViewer;
    HWND                foregroundWND;
    bool                randomcolors;
    bool                brightness;
    CAeroColors         aeroColors;
    static std::wstring wpPath;
    static bool         threadRunning;

    typedef BOOL(__stdcall *PFNCHANGEWINDOWMESSAGEFILTEREX)(HWND hWnd, UINT message, DWORD dwFlag, PCHANGEFILTERSTRUCT pChangeFilterStruct);
    static PFNCHANGEWINDOWMESSAGEFILTEREX m_pChangeWindowMessageFilter;
};
