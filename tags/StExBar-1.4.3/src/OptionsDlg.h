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
	CHyperLink				m_link;				///< the hyperlink used in the options dialog

	CCommands				m_commands;
	HWND					m_hListControl;
};
