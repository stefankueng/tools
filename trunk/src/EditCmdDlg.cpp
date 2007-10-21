#include "stdafx.h"
#include "SRBand.h"
#include "resource.h"
#include "version.h"
#include <algorithm>
#include "EditCmdDlg.h"
#include <string>

#include <boost/regex.hpp>
using namespace boost;
using namespace std;


CEditCmdDlg::CEditCmdDlg(HWND hParent)
{
	m_hParent = hParent;
}

CEditCmdDlg::~CEditCmdDlg(void)
{
}

LRESULT CEditCmdDlg::DlgFunc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM /*lParam*/)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			InitDialog(hwndDlg, IDI_OPTIONS);

			SetupControls();
		}
		return (INT_PTR)TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			{
				SetupCommand();
			}
			// fall through
		case IDCANCEL:
			EndDialog(*this, LOWORD(wParam));
			return (INT_PTR)TRUE;
		case IDC_SEPARATOR:
			m_command.separator = SendMessage(GetDlgItem(*this, IDC_SEPARATOR), BM_GETCHECK, 0, 0) == BST_CHECKED;
			SetSeparator(m_command.separator);
			break;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

void CEditCmdDlg::SetSeparator(bool bSeparator)
{
	SendMessage(GetDlgItem(*this, IDC_SEPARATOR), BM_SETCHECK, m_command.separator ? BST_CHECKED : BST_UNCHECKED, 0);
	EnableWindow(GetDlgItem(*this, IDC_NAME), !bSeparator);
	EnableWindow(GetDlgItem(*this, IDC_ICONPATH), !bSeparator);
	EnableWindow(GetDlgItem(*this, IDC_COMMANDLINE), !bSeparator);
	EnableWindow(GetDlgItem(*this, IDC_HOTKEY), !bSeparator);
	EnableWindow(GetDlgItem(*this, IDC_VIEWPATH), !bSeparator);
	EnableWindow(GetDlgItem(*this, IDC_NOVIEWPATH), !bSeparator);
	EnableWindow(GetDlgItem(*this, IDC_FILESELECTED), !bSeparator);
	EnableWindow(GetDlgItem(*this, IDC_FOLDERSELECTED), !bSeparator);
	EnableWindow(GetDlgItem(*this, IDC_SELECTED), !bSeparator);
	EnableWindow(GetDlgItem(*this, IDC_NOSELECTION), !bSeparator);
	EnableWindow(GetDlgItem(*this, IDC_SELECTEDCOUNT), !bSeparator);
}

void CEditCmdDlg::SetupControls()
{
	SetSeparator(m_command.separator);
	if (m_command.separator)
		return;		// nothing more to do here

	SetDlgItemText(*this, IDC_NAME, m_command.name.c_str());
	SetDlgItemText(*this, IDC_ICONPATH, m_command.icon.c_str());
	SetDlgItemText(*this, IDC_COMMANDLINE, m_command.commandline.c_str());

	WPARAM hk = m_command.key.keycode;
	hk |= m_command.key.alt ? HOTKEYF_ALT<<8 : 0;
	hk |= m_command.key.shift ? HOTKEYF_SHIFT<<8 : 0;
	hk |= m_command.key.control ? HOTKEYF_CONTROL<<8 : 0;
	SendMessage(GetDlgItem(*this, IDC_HOTKEY), HKM_SETHOTKEY, hk, 0);

	SendMessage(GetDlgItem(*this, IDC_VIEWPATH), BM_SETCHECK, m_command.enabled_viewpath ? BST_CHECKED : BST_UNCHECKED, 0);
	SendMessage(GetDlgItem(*this, IDC_NOVIEWPATH), BM_SETCHECK, m_command.enabled_noviewpath ? BST_CHECKED : BST_UNCHECKED, 0);

	SendMessage(GetDlgItem(*this, IDC_FILESELECTED), BM_SETCHECK, m_command.enabled_fileselected ? BST_CHECKED : BST_UNCHECKED, 0);
	SendMessage(GetDlgItem(*this, IDC_FOLDERSELECTED), BM_SETCHECK, m_command.enabled_folderselected ? BST_CHECKED : BST_UNCHECKED, 0);
	SendMessage(GetDlgItem(*this, IDC_SELECTED), BM_SETCHECK, m_command.enabled_selected ? BST_CHECKED : BST_UNCHECKED, 0);
	SendMessage(GetDlgItem(*this, IDC_NOSELECTION), BM_SETCHECK, m_command.enabled_noselection ? BST_CHECKED : BST_UNCHECKED, 0);

	TCHAR buf[40] = {0};
	_stprintf_s(buf, 40, _T("%ld"), m_command.enabled_selectedcount);
	SetDlgItemText(*this, IDC_SELECTEDCOUNT, buf);

	// disable controls which must not be changed for internal commands
	if (m_command.commandline.compare(INTERNALCOMMAND) == 0)
	{
		EnableWindow(GetDlgItem(*this, IDC_SEPARATOR), FALSE);
		EnableWindow(GetDlgItem(*this, IDC_NAME), FALSE);
		EnableWindow(GetDlgItem(*this, IDC_ICONPATH), FALSE);
		EnableWindow(GetDlgItem(*this, IDC_COMMANDLINE), FALSE);
		EnableWindow(GetDlgItem(*this, IDC_VIEWPATH), FALSE);
		EnableWindow(GetDlgItem(*this, IDC_NOVIEWPATH), FALSE);
		EnableWindow(GetDlgItem(*this, IDC_FILESELECTED), FALSE);
		EnableWindow(GetDlgItem(*this, IDC_FOLDERSELECTED), FALSE);
		EnableWindow(GetDlgItem(*this, IDC_SELECTED), FALSE);
		EnableWindow(GetDlgItem(*this, IDC_NOSELECTION), FALSE);
		EnableWindow(GetDlgItem(*this, IDC_SELECTEDCOUNT), FALSE);
	}
	else
	{
		EnableWindow(GetDlgItem(*this, IDC_SEPARATOR), TRUE);
	}
}

void CEditCmdDlg::SetupCommand()
{
	m_command.separator = SendMessage(GetDlgItem(*this, IDC_SEPARATOR), BM_GETCHECK, 0, 0) == BST_CHECKED;
	if (m_command.separator)
		return;		// nothing more to do here
	TCHAR buf[EDITCMDDLG_MAXBUF] = {0};

	GetDlgItemText(*this, IDC_NAME, buf, EDITCMDDLG_MAXBUF);
	m_command.name = buf;
	GetDlgItemText(*this, IDC_ICONPATH, buf, EDITCMDDLG_MAXBUF);
	m_command.icon = buf;
	GetDlgItemText(*this, IDC_COMMANDLINE, buf, EDITCMDDLG_MAXBUF);
	m_command.commandline = buf;

	WPARAM hk = SendMessage(GetDlgItem(*this, IDC_HOTKEY), HKM_GETHOTKEY, 0, 0);
	m_command.key.keycode = LOBYTE(hk);
	m_command.key.alt = (HIBYTE(hk) & HOTKEYF_ALT) != 0;
	m_command.key.shift = (HIBYTE(hk) & HOTKEYF_SHIFT) != 0;
	m_command.key.control = (HIBYTE(hk) & HOTKEYF_CONTROL) != 0;

	m_command.enabled_viewpath = SendMessage(GetDlgItem(*this, IDC_VIEWPATH), BM_GETCHECK, 0, 0) == BST_CHECKED;
	m_command.enabled_noviewpath = SendMessage(GetDlgItem(*this, IDC_NOVIEWPATH), BM_GETCHECK, 0, 0) == BST_CHECKED;

	m_command.enabled_fileselected = SendMessage(GetDlgItem(*this, IDC_FILESELECTED), BM_GETCHECK, 0, 0) == BST_CHECKED;
	m_command.enabled_folderselected = SendMessage(GetDlgItem(*this, IDC_FOLDERSELECTED), BM_GETCHECK, 0, 0) == BST_CHECKED;
	m_command.enabled_selected = SendMessage(GetDlgItem(*this, IDC_SELECTED), BM_GETCHECK, 0, 0) == BST_CHECKED;

	GetDlgItemText(*this, IDC_SELECTEDCOUNT, buf, EDITCMDDLG_MAXBUF);
	m_command.enabled_selectedcount = _ttol(buf);
}
