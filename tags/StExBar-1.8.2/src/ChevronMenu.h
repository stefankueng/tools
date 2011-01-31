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
#pragma once
#include "basewindow.h"

/**
 * Implements a 'menu' using a vertical toolbar to show the missing/hidden
 * buttons of a toolbar. Use this class in response to an RBN_CHEVRONPUSHED
 * message.
 */
class CChevronMenu : public CWindow
{
public:
    CChevronMenu(HINSTANCE hInst, const WNDCLASSEX* wcx = NULL) : CWindow(hInst, wcx)
    {
        m_hwndToolbar = NULL;
        hHiddenToolbar = NULL;
    }

    /**
     * Show the hidden toolbar buttons at the chevron position like a context menu.
     *
     * Instead of a simple context menu, a vertical toolbar is used as the IE and
     * windows explorer do in that situation.
     *
     * \param lpRebarChevron the pointer to the LPNMREBARCHEVRON struct which you get
     *                       from the RBN_CHEVRONPUSHED message
     * \param hToolbar the handle of the original toolbar for which to show the 'menu'
     * \return true if a button was clicked. In that case, use the public member variables
     *              m_uMsg, m_wParam and m_lParam to use in a SendMessage() call to tell
     *              the original toolbar to execute that command.
     */
    bool Show(LPNMREBARCHEVRON lpRebarChevron, HWND hToolbar);

    UINT m_uMsg;
    WPARAM m_wParam;
    LPARAM m_lParam;
private:
    bool m_bRun;
    HWND m_hwndToolbar;
    HWND hHiddenToolbar;
    LRESULT CALLBACK WinMsgHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
