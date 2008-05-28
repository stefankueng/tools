#pragma once
#include <vector>
#include "ShellExt.h"

#define ListView_GetItemTextEx(hwndLV, i, iSubItem_, __buf) \
{ \
  int nLen = 1024;\
  int nRes;\
  LV_ITEM _ms_lvi;\
  _ms_lvi.iSubItem = iSubItem_;\
  do\
  {\
	nLen += 2;\
	_ms_lvi.cchTextMax = nLen;\
    if (__buf)\
		delete[] __buf;\
	__buf = new TCHAR[nLen];\
	_ms_lvi.pszText = __buf;\
    nRes  = (int)::SendMessage((hwndLV), LVM_GETITEMTEXT, (WPARAM)(i), (LPARAM)(LV_ITEM *)&_ms_lvi);\
  } while (nRes == nLen-1);\
}
#define GetDlgItemTextEx(hwndDlg, _id, __buf) \
{\
	int nLen = 1024;\
	int nRes;\
	do\
	{\
		nLen *= 2;\
		if (__buf)\
			delete [] __buf;\
		__buf = new TCHAR[nLen];\
		nRes = GetDlgItemText(hwndDlg, _id, __buf, nLen);\
	} while (nRes == nLen-1);\
}

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
	virtual void InitWorkfileView();
	
	HWND m_hwnd;
	std::vector<std::wstring> filenames;
};


