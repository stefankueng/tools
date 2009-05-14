// StExBar - an explorer toolbar

// Copyright (C) 2007-2009 - Stefan Kueng

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
#include "hyperlink.h"
#include "Commands.h"
#include <string>

using namespace std;

/**
* bookmarks dialog.
*/
class COptionsDlg : public CDialog
{
public:
	COptionsDlg(HWND hParent);
	~COptionsDlg(void);

protected:
	LRESULT CALLBACK		DlgFunc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void					InitCustomCommandsList();
	void					OnSelectListItem(LPNMLISTVIEW lpNMListView);
	LRESULT					OnCustomDrawListItem(LPNMLVCUSTOMDRAW lpNMCustomDraw);
	void					EditSelectedItem();
	void					RemoveSelectedItem();
	void					MoveSelectedUp();
	void					MoveSelectedDown();
private:
	HWND					m_hParent;
	CRegStdWORD				m_regShowBtnText;	///< config setting whether to show the text for the toolbar buttons or not
	CRegStdWORD				m_regUseUNCPaths;	///< config setting whether to copy the UNC paths of mapped paths or not
	CRegStdWORD				m_regUseSelector;	///< config setting whether to use the selector or the cmd.exe replacement
	CRegStdWORD				m_regHideEditBox;	///< config setting whether to show the edit box or not
	CRegStdWORD				m_regGlassForCmd;	///< config setting whether to enable glass effect for cmd.exe on Vista
	CHyperLink				m_link;				///< the hyperlink used in the options dialog

	CCommands				m_commands;
	HWND					m_hListControl;
};
