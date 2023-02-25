// SkTimeStamp - Change file dates easily, directly from explorer

// Copyright (C) 2012, 2023 - Stefan Kueng

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

#include <vector>
#include "ShellExt.h"

/**
 * Displays and updates all controls on the property page. The property
 * page itself is shown by explorer.
 */
class CShellPropertyPage
{
public:
    CShellPropertyPage(const std::vector<std::wstring> &filenames);
    virtual ~CShellPropertyPage();

    /**
     * Sets the window handle.
     * \param hwnd the handle.
     */
    virtual void SetHwnd(HWND hwnd);
    /**
     * Callback function which receives the window messages of the
     * property page. See the Win32 API for PropertySheets for details.
     */
    virtual BOOL PageProc(HWND hwnd, UINT uMessage, WPARAM wParam, LPARAM lParam);

protected:
    /**
     * Initializes the property page.
     */
    virtual void              InitWorkfileView();
    /**
     * Sets the dates on the selected files and folders.
     * If a filetime is zero, the original date of the file/folder is used, i.e., the filedate is not changed.
     */
    void                      SetDates(FILETIME ftCreationTime, FILETIME ftLastWriteTime, FILETIME ftLastAccessTime);

    HWND                      m_hwnd;
    std::vector<std::wstring> fileNames;
};
