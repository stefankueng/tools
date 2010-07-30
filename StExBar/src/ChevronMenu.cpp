// StExBar - an explorer toolbar

// Copyright (C) 2007-2008 - Stefan Kueng

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
#include "uxtheme.h"
#include "ChevronMenu.h"

#pragma comment(lib, "UxTheme")

bool CChevronMenu::Show(LPNMREBARCHEVRON lpRebarChevron, HWND hToolbar)
{
    m_hwndToolbar = hToolbar;
    // create our popup window
    RegisterWindow(CS_HREDRAW|CS_VREDRAW, NULL, NULL, NULL, NULL, _T("ChevronMenuToolBar"), NULL);
    CreateEx(0, WS_POPUP|WS_CHILD|WS_CLIPCHILDREN, lpRebarChevron->hdr.hwndFrom, NULL);
    ::SetParent(*this, NULL);
    // Retrieve the current bounding rectangle for the selected band
    RECT rebarrect;
    ::SendMessage(lpRebarChevron->hdr.hwndFrom, RB_GETRECT, lpRebarChevron->uBand, (LPARAM)&rebarrect);
    ::MapWindowPoints(lpRebarChevron->hdr.hwndFrom, NULL, (LPPOINT)&rebarrect, 2);
    rebarrect.right -= (lpRebarChevron->rc.right-lpRebarChevron->rc.left);
    // Retrieve the total number of buttons
    int nButtons = (int)::SendMessage(hToolbar, TB_BUTTONCOUNT, 0, 0);
    // check for every button if it's completely visible
    int iFirstHidden = -1;
    for (int i=0; i<nButtons; ++i)
    {
        RECT buttonrect;
        ::SendMessage(hToolbar, TB_GETITEMRECT, i, (LPARAM)&buttonrect);
        ::MapWindowPoints(hToolbar, NULL, (LPPOINT)&buttonrect, 2);

        RECT intersect;
        IntersectRect(&intersect, &buttonrect, &rebarrect);
        if (EqualRect(&intersect, &buttonrect)==FALSE)
        {
            // this is the first button which is not completely visible
            // but make sure that it's not a separator
            TBBUTTON tb = {0};
            ::SendMessage(hToolbar, TB_GETBUTTON, i, (LPARAM)&tb);
            if ((tb.fsStyle & BTNS_SEP) == 0)
            {
                iFirstHidden = i;
                break;
            }
        }
    }
    if (iFirstHidden >= 0)
    {
        // create a popup window to hold our toolbar
        // create a vertical toolbar with all the hidden buttons on it
        hHiddenToolbar = CreateWindowEx(TBSTYLE_EX_MIXEDBUTTONS|TBSTYLE_EX_HIDECLIPPEDBUTTONS|TBSTYLE_EX_DRAWDDARROWS,
            TOOLBARCLASSNAME,
            NULL,
            WS_CHILD|TBSTYLE_TOOLTIPS|TBSTYLE_WRAPABLE|TBSTYLE_LIST|TBSTYLE_TRANSPARENT|TBSTYLE_FLAT|CCS_NODIVIDER|CCS_LEFT|CCS_NORESIZE,
            0,
            0,
            0,
            0,
            (*this),
            NULL,
            hResource,
            NULL);

        // Send the TB_BUTTONSTRUCTSIZE message, which is required for
        // backward compatibility.
        SendMessage(hHiddenToolbar, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0);
        // Get ImageList from the toolbar
        HIMAGELIST  hHot = (HIMAGELIST)::SendMessage(hToolbar, TB_GETIMAGELIST, 0, 0);

        // Create a duplicate of the image list
        HIMAGELIST  hImageList = ImageList_Duplicate(hHot);

        // Set the image list for our new toolbar
        ::SendMessage(hHiddenToolbar, TB_SETIMAGELIST, 0, (LPARAM)hImageList);

        // add all the invisible buttons to the new toolbar
        for (int i=iFirstHidden; i<nButtons; ++i)
        {
            TBBUTTON tb = {0};
            ::SendMessage(hToolbar, TB_GETBUTTON, i, (LPARAM)&tb);
            tb.fsStyle |= (BTNS_SHOWTEXT);
            ::SendMessage(hHiddenToolbar, TB_ADDBUTTONS, 1, (LPARAM)&tb);
        }
        ::SendMessage(hHiddenToolbar, TB_AUTOSIZE, 0, 0);
        SIZE tbsize;
        ::SendMessage(hHiddenToolbar, TB_GETMAXSIZE, 0, (LPARAM)&tbsize);
        RECT tbRect;
        ::GetWindowRect(m_hwndToolbar, &tbRect);
        // position the toolbar left-aligned with the chevron
        // but first we need to convert the chevron coordinates into screen coordinates
        POINT chevronxy;
        chevronxy.x = lpRebarChevron->rc.left;
        chevronxy.y = lpRebarChevron->rc.bottom;
        ::ClientToScreen(lpRebarChevron->hdr.hwndFrom, &chevronxy);
        // make sure that the toolbar does not leave the screen on the right
        RECT testrect = {chevronxy.x, chevronxy.y, chevronxy.x+tbsize.cx, chevronxy.y+tbsize.cy};
        HMONITOR hMonitor = MonitorFromRect(&testrect, MONITOR_DEFAULTTONEAREST);
        MONITORINFO mi = {0};
        mi.cbSize = sizeof(mi);
        GetMonitorInfo(hMonitor, &mi);
        testrect = mi.rcWork;
        if (chevronxy.x + tbsize.cx > (testrect.right))
            chevronxy.x = testrect.right - tbsize.cx;
        if (chevronxy.y + tbsize.cy > (testrect.bottom))
            chevronxy.y = testrect.bottom - tbsize.cy;
        ::ShowWindow(*this, SW_SHOW);
        ::SetWindowPos(*this, HWND_TOP,
            chevronxy.x, chevronxy.y,
            tbsize.cx+10, tbsize.cy+10,
            0);
        ::ShowWindow(hHiddenToolbar, SW_SHOW);
        ::SetWindowPos(hHiddenToolbar, HWND_TOP,
            0, 0,
            tbsize.cx+10, tbsize.cy+10,
            0);

        m_uMsg = 0;
        m_wParam = 0;
        m_lParam = 0;

        // stay modal
        MSG msg;
        m_bRun = true;
        while (m_bRun)
        {
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                if (msg.message == WM_QUIT)
                {
                    m_bRun = false;
                }
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        ImageList_Destroy(hImageList);
    }
    DestroyWindow(*this);

    return (m_uMsg != 0);
}

LRESULT CChevronMenu::WinMsgHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_ERASEBKGND:
        {
            HDC hDC = (HDC)wParam;
            RECT rc;
            ::GetClientRect(hwnd, &rc);
            // only draw the themed background if themes are enabled
            if (IsThemeActive())
            {
                HTHEME hTheme = OpenThemeData(hwnd, L"Rebar");
                if (hTheme)
                {
                    // now draw the themed background of a rebar control, because
                    // that's what we're actually in and should look like.
                    DrawThemeBackground(hTheme, hDC, 0, 0, &rc, NULL);
                    return TRUE;    // we've drawn the background
                }
            }
            // just do nothing so the system knows that we haven't erased the background
        }
        break;
    case WM_KILLFOCUS:
        ::SendMessage(*this, WM_CLOSE, 0, 0);
        return 0L;
    case WM_CLOSE:
        DestroyWindow(*this);
        PostMessage(*this, WM_QUIT, 0, 0);
        m_bRun = false;
        return 0L;
    case WM_COMMAND:
        {
            m_uMsg = uMsg;
            m_wParam = wParam;
            m_lParam = lParam;
            DestroyWindow(*this);
            ::PostQuitMessage(0);
            return 0L;
        }
        break;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}