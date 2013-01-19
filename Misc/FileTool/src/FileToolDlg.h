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

#pragma once

#include "BaseDialog.h"
#include "AeroControls.h"
#include "FileDropTarget.h"
#include "hyperlink.h"

/**
 * Main dialog.
 */
class CFileToolDlg : public CDialog
{
public:
    CFileToolDlg(HWND hParent);
    ~CFileToolDlg(void);

protected:
    LRESULT CALLBACK        DlgFunc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT                 DoCommand(int id);
    void                    DoListNotify(LPNMITEMACTIVATE lpNMItemActivate);

    void                    LockFile(LPCWSTR path);
    void                    FillLockList();
    void                    CreateFiles();
    void                    Clean();

private:
    HWND                    m_hParent;
    AeroControlBase         m_aerocontrols;
    CHyperLink              m_aboutLink;
    CFileDropTarget *       m_pDropGroup;
    CFileDropTarget *       m_pDropList;
    CFileDropTarget *       m_pDropTarget;
    std::map<std::wstring,HANDLE>   m_lockedFiles;
    bool                    m_bAscending;

};
