// StExBar - an explorer toolbar

// Copyright (C) 2007-2009, 2012 - Stefan Kueng

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
#include "DlgResizer.h"
#include "AutoComplete.h"
#include <string>
#include <regex>

struct __lesscasecmp
{
    bool operator() (const std::wstring& a, const std::wstring& b) const
    {
        return (_wcsicmp(a.c_str(), b.c_str()) < 0);
    }
};

/**
* bookmarks dialog.
*/
class CRenameDlg : public CDialog
{
public:
    CRenameDlg(HWND hParent);
    ~CRenameDlg(void);

    const std::wstring&     GetMatchString() {return m_sMatch;}
    std::regex_constants::syntax_option_type GetRegexFlags() const {return m_fl;}
    const std::wstring&     GetReplaceString() const {return m_sReplace;}
    void                    SetFileList(const std::set<std::wstring>& list);

protected:
    LRESULT CALLBACK        DlgFunc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    bool                    PreTranslateMessage(MSG* pMsg);
    void                    FillRenamedList();
private:
    HWND                    m_hParent;
    CDlgResizer             m_resizer;
    std::wstring            m_sMatch;           ///< the match string of the rename
    std::wstring            m_sReplace;         ///< the replace string of the rename
    std::set<std::wstring, __lesscasecmp> m_filelist;     ///< the list of selected file/folder names
    CAutoComplete           m_AutoCompleteRen1;
    CAutoComplete           m_AutoCompleteRen2;

    std::regex_constants::syntax_option_type m_fl;
};
