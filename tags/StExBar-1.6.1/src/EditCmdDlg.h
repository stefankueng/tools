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
#include "Commands.h"
#include <string>

using namespace std;

#define EDITCMDDLG_MAXBUF 4096

/**
* bookmarks dialog.
*/
class CEditCmdDlg : public CDialog
{
public:
	CEditCmdDlg(HWND hParent);
	~CEditCmdDlg(void);

	void					SetCommand(Command cmd) {m_command = cmd;}
	Command					GetCommand() {return m_command;}
protected:
	LRESULT CALLBACK		DlgFunc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void					SetupControls();
	void					SetupCommand();
	void					SetSeparator(bool bSeparator);
private:
	HWND					m_hParent;
	Command					m_command;
};
