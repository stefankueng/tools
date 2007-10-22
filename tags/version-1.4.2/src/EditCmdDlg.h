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
