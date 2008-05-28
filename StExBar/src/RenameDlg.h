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
#include "basedialog.h"
#include "DlgResizer.h"
#include <string>

using namespace std;

/**
* bookmarks dialog.
*/
class CRenameDlg : public CDialog
{
public:
	CRenameDlg(HWND hParent);
	~CRenameDlg(void);

	wstring					GetMatchString() {return m_sMatch;}
	wstring					GetReplaceString() {return m_sReplace;}
	void					SetFileList(const set<wstring>& list) {m_filelist = list;}

protected:
	LRESULT CALLBACK		DlgFunc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void					FillRenamedList();
private:
	HWND					m_hParent;
	CDlgResizer				m_resizer;
	wstring					m_sMatch;			///< the match string of the rename
	wstring					m_sReplace;			///< the replace string of the rename
	set<wstring>			m_filelist;			///< the list of selected file/folder names
};
