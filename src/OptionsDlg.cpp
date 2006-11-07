#include "stdafx.h"
#include "SRBand.h"
#include "resource.h"
#include <algorithm>

INT_PTR CALLBACK CDeskBand::OptionsDlgFunc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			CDeskBand * pThis = (CDeskBand*)lParam;
			HWND hwndOwner; 
			RECT rc, rcDlg, rcOwner;

			SetLastError(0);
			if ((SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)pThis)==0)&&(GetLastError()))
				EndDialog(hwndDlg, IDCANCEL);	// if we could not set the user data, get out of here immediately!

			hwndOwner = ::GetParent(hwndDlg);
			if (hwndOwner == NULL)
				hwndOwner = ::GetDesktopWindow();

			GetWindowRect(hwndOwner, &rcOwner); 
			GetWindowRect(hwndDlg, &rcDlg); 
			CopyRect(&rc, &rcOwner); 

			OffsetRect(&rcDlg, -rcDlg.left, -rcDlg.top); 
			OffsetRect(&rc, -rc.left, -rc.top); 
			OffsetRect(&rc, -rcDlg.right, -rcDlg.bottom); 

			SetWindowPos(hwndDlg, HWND_TOP, rcOwner.left + (rc.right / 2), rcOwner.top + (rc.bottom / 2), 0, 0,	SWP_NOSIZE); 
			//HICON hIcon = (HICON)::LoadImage(g_hInst, MAKEINTRESOURCE(IDI_RENDLG), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE|LR_SHARED);
			//::SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
			//::SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

			pThis->m_link.ConvertStaticToHyperlink(hwndDlg, IDC_LINK, _T("http://tools.tortoisesvn.net"));

			SendMessage(GetDlgItem(hwndDlg, IDC_SHOWTEXT), BM_SETCHECK, DWORD(pThis->m_regShowBtnText) ? BST_CHECKED : BST_UNCHECKED, 0);
		}
		return (INT_PTR)TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			{
				CDeskBand * pThis = (CDeskBand*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
				pThis->m_regShowBtnText = SendMessage(GetDlgItem(hwndDlg, IDC_SHOWTEXT), BM_GETCHECK, 0, 0);
			}
			// fall through
		case IDCANCEL:
			EndDialog(hwndDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		case IDC_EDITCONFIG:
			{
				// find custom commands ini file
				TCHAR szPath[MAX_PATH] = {0};
				if (SUCCEEDED(SHGetFolderPath(NULL, 
					CSIDL_APPDATA|CSIDL_FLAG_CREATE, 
					NULL, 
					SHGFP_TYPE_CURRENT, 
					szPath))) 
				{
					PathAppend(szPath, TEXT("StExBar"));
					CreateDirectory(szPath, NULL);
					PathAppend(szPath, TEXT("CustomCommands.ini"));
					// if the file does not exist (yet), we create one and
					// fill it with helpful comments
					if (!PathFileExists(szPath))
					{
						FILE * stream;
						// Open for write 
						errno_t err = _tfopen_s(&stream, szPath, _T("w"));
						if (err == 0)
						{
							fputs("### configuration file\n"
								"### -------------------\n"
								"### This file configures the predefined command buttons\n"
								"### and additional custom commands.\n"
								"###\n"
								"### Every configuration starts with a section header. The names of\n"
								"### the section headers are sorted when this file is parsed, which\n"
								"### means that the section with the lowest order is used first.\n"
								"### You can use this sorting to define where the custom buttons\n"
								"### will show up in the toolbar.\n"
								"\n"
								"### the format of a custom command is as follows:\n"
								"# [00NotePad]\n"
								"### the name of the custom command is shown as the button text.\n"
								"# name = Editor\n"
								"### a tooltip will be shown on each button in the toolbar.\n"
								"# tooltip = Opens the selected file in the editor\n"
								"### set icon to a path pointing to an *.ico file which has the size 16x16 and 24x24\n"
								"# icon = c:\\myicons\\customcommand.ico\n"
								"### the command line is the path to the custom program, including the parameters passed to it.\n"
								"# commandline = %WINDIR%\\system32\\notepad.exe %selpaths\n"
								"### the following settings define the conditions on when the custom command button\n"
								"### should be enabled. Use this to disable the custom command for situations where it\n"
								"### does not make sense, e.g. disable an editor if a folder is selected.\n"
								"# enabled_fileselected = yes\n"
								"# enabled_folderselected = no\n"
								"### enabled_selected means the command is enabled if either a file or a folder is selected.\n"
								"# enabled_selected = no\n"
								"### enabled_noselection means the command is enabled if nothing is selected.\n"
								"# enabled_noselection = yes\n"
								"### Set enabled_viewpath to enable the command when the explorer view points to a valid path.\n"
								"### Invalid paths include for example the 'printers' view\n"
								"# enabled_viewpath = yes\n"
								"### Set enabled_noviewpath if the command should be enabled when the explorer view points to\n"
								"### an invalid path, for example when showing the printers list.\n"
								"# enabled_noviewpath = no\n"
								"### The enabled_numberselected can be used to restrict enabling of the command button to a\n"
								"### specific number of selected items. For example diff tools usually need two selected\n"
								"### files to work, text editors usually need only one file selected to open it.\n"
								"### Note that if enabled_noselection is set, then the command is enabled if nothing or the\n"
								"### number specified with enabled_numberselected is selected.\n"
								"# enabled_numberselected = 1\n"
								"### The next options define a hotkey which when pressed will start the custom tool as if the\n"
								"### user clicked on the toolbar button.\n"
								"### The hotkey is the keyboard scan code. For normal characters, that's just the ASCII code.\n"
								"### For special keys please refer to the MSDN documentation:\n"
								"#### http://msdn.microsoft.com/library/default.asp?url=/library/en-us/winui/WinUI/WindowsUserInterface/UserInput/VirtualKeyCodes.asp\n"
								"# hotkey = 0x45\n"
								"# hotkey_shift = yes\n"
								"# hotkey_control = yes\n"
								"# hotkey_alt = no\n"
								"\n"
								"\n"
								"### To change the internal commands:\n"
								"# [somegroupname]\n"
								"### the internal commands are numbered like this:\n"
								"### 0 = console edit box\n"
								"### 1 = options\n"
								"### 2 = start console\n"
								"### 3 = copy file names\n"
								"### 4 = copy file paths\n"
								"### To overwrite a hotkey for an internal command, just set internalcommand to the command number.\n"
								"# internalcommand = 2\n"
								"### Then simply assign a new hotkey for that command\n"
								"# hotkey = 0x41\n"
								"# hotkey_shift = yes\n"
								"# hotkey_control = yes\n"
								"### to remove the hotkey assigned to the internal command, set\n"
								"# hotkey = 0\n"
								"### You can also hide the button for the internal command:\n"
								"### Note: if an internal command has a hotkey assigned to it, the hotkey\n"
								"### will keep working even if the button is hidden.\n"
								"# hide = yes\n"
								"\n"
								"\n"
								"#### to insert a separator, do the following\n"
								"# [somegroupname2]\n"
								"# separator = 1\n"
								"\n"
								"\n"
								"\n"
								"\n"
								"### some special placeholders are available for the command line:\n"
								"### %selpaths : will be replaced with the paths of the selected items\n"
								"###             note: if too many items are selected, the command line\n"
								"###             could get too long and get truncated!\n"
								"### %selnames : will be replaced with the names of the selected items\n"
								"### %curdir   : will be replaced with the path of the currently shown\n"
								"###             directory.\n"
								"###             note: not all views in explorer have a defined directory\n"
								"###             for example the search window does not have a 'current\n"
								"###             directory', and there the %curdir is not defined\n"
								"\n"
								"\n"
								, stream);
						}

						// Close stream if it is not NULL 
						if (stream)
							fclose(stream);
					}
					PathQuoteSpaces(szPath);
					// now start the default text editor with our config file
					wstring viewer;
					CRegStdString txt = CRegStdString(_T(".txt\\"), _T(""), FALSE, HKEY_CLASSES_ROOT);
					viewer = (wstring)txt;
					if (!viewer.empty())
					{
						viewer = viewer + _T("\\Shell\\Open\\Command\\");
						CRegStdString txtexe = CRegStdString(viewer, _T(""), FALSE, HKEY_CLASSES_ROOT);
						viewer = (wstring)txtexe;
						DWORD len = ExpandEnvironmentStrings(viewer.c_str(), NULL, 0);
						TCHAR * buf = new TCHAR[len+1];
						ExpandEnvironmentStrings(viewer.c_str(), buf, len);
						viewer = buf;
						delete buf;
					}
					else
					{
						// no viewer for txt files???
						// use hard coded path to notepad.exe
						TCHAR buf[MAX_PATH] = {0};
						SHGetFolderPath(hwndDlg, CSIDL_SYSTEM, NULL, SHGFP_TYPE_CURRENT, buf);
						PathAppend(buf, _T("Notepad.exe"));
						viewer = buf;
					}
					// replace "%1" with the path to our config file
					wstring tag(_T("%1"));
					// get the range of tag in viewer
					wstring::iterator it_begin = search(viewer.begin(), viewer.end(), tag.begin(), tag.end());

					if (it_begin != viewer.end())
					{
						wstring::iterator it_end= it_begin + tag.size();
						viewer.replace(it_begin, it_end, wstring(szPath));
					}
					else
					{
						viewer += _T(" ");
						viewer += szPath;
					}
					// finally start the application
					STARTUPINFO startup;
					PROCESS_INFORMATION process;
					memset(&startup, 0, sizeof(startup));
					startup.cb = sizeof(startup);
					memset(&process, 0, sizeof(process));

					if (CreateProcess(NULL, const_cast<TCHAR*>(viewer.c_str()), NULL, NULL, FALSE, 0, 0, 0, &startup, &process)==0)
					{
						LPVOID lpMsgBuf;
						FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
							FORMAT_MESSAGE_FROM_SYSTEM | 
							FORMAT_MESSAGE_IGNORE_INSERTS,
							NULL,
							GetLastError(),
							MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
							(LPTSTR) &lpMsgBuf,
							0,
							NULL 
							);
						MessageBox( NULL, (LPCTSTR)lpMsgBuf, _T("Error"), MB_OK | MB_ICONINFORMATION );
						LocalFree( lpMsgBuf );
					}
					else
					{
						CloseHandle(process.hThread);
						CloseHandle(process.hProcess);
					}
				}
			}
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
