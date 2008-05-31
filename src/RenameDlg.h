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
