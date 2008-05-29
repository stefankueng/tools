#include "stdafx.h"

#include "ShellPropertyPage.h"
#include "ShellExt.h"


// Nonmember function prototypes
BOOL CALLBACK PageProc (HWND, UINT, WPARAM, LPARAM);
UINT CALLBACK PropPageCallbackProc ( HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp );
// Misc utility functins.
void ReadDTPCtrl(HWND hwnd, UINT idcDatePicker, UINT idcTimePicker, FILETIME* pFiletime);
void SetDTPCtrl(HWND hwnd, UINT idcDatePicker, UINT idcTimePicker, const FILETIME* pFiletime);

// CShellExt member functions (needed for IShellPropSheetExt)
STDMETHODIMP CShellExt::AddPages (LPFNADDPROPSHEETPAGE lpfnAddPage,
                                  LPARAM lParam)
{
	if (files_.size() == 0)
		return NOERROR;

    PROPSHEETPAGE psp;
	ZeroMemory(&psp, sizeof(PROPSHEETPAGE));
	HPROPSHEETPAGE hPage;
	CShellPropertyPage *sheetpage = new CShellPropertyPage(files_);

    psp.dwSize = sizeof (psp);
    psp.dwFlags = PSP_USEREFPARENT | PSP_USETITLE | PSP_USEICONID | PSP_USECALLBACK;	
	psp.hInstance = g_hmodThisDll;
	psp.pszTemplate = MAKEINTRESOURCE(IDD_PROPPAGE);
    //psp.pszIcon = MAKEINTRESOURCE(IDI_APPSMALL);
    psp.pszTitle = _T("TimeStamps");
    psp.pfnDlgProc = (DLGPROC) PageProc;
    psp.lParam = (LPARAM) sheetpage;
    psp.pfnCallback = PropPageCallbackProc;
    psp.pcRefParent = &g_cRefThisDll;

    hPage = CreatePropertySheetPage (&psp);

	if (hPage != NULL)
	{
        if (!lpfnAddPage (hPage, lParam))
        {
            delete sheetpage;
            DestroyPropertySheetPage (hPage);
        }
	}

    return NOERROR;
}



STDMETHODIMP CShellExt::ReplacePage (UINT /*uPageID*/, LPFNADDPROPSHEETPAGE /*lpfnReplaceWith*/, LPARAM /*lParam*/)
{
    return E_FAIL;
}

/////////////////////////////////////////////////////////////////////////////
// Dialog procedures and other callback functions

BOOL CALLBACK PageProc (HWND hwnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    CShellPropertyPage * sheetpage;

    if (uMessage == WM_INITDIALOG)
    {
        sheetpage = (CShellPropertyPage*) ((LPPROPSHEETPAGE) lParam)->lParam;
        SetWindowLongPtr (hwnd, GWLP_USERDATA, (LONG_PTR) sheetpage);
        sheetpage->SetHwnd(hwnd);
    }
    else
    {
        sheetpage = (CShellPropertyPage*) GetWindowLongPtr (hwnd, GWLP_USERDATA);
    }

    if (sheetpage != 0L)
        return sheetpage->PageProc(hwnd, uMessage, wParam, lParam);
    else
        return FALSE;
}

UINT CALLBACK PropPageCallbackProc ( HWND /*hwnd*/, UINT uMsg, LPPROPSHEETPAGE ppsp )
{
    // Delete the page before closing.
    if (PSPCB_RELEASE == uMsg)
    {
        CShellPropertyPage* sheetpage = (CShellPropertyPage*) ppsp->lParam;
        if (sheetpage != NULL)
            delete sheetpage;
    }
    return 1;
}

// *********************** CShellPropertyPage *************************

CShellPropertyPage::CShellPropertyPage(const std::vector<stdstring> &newFilenames)
	:filenames(newFilenames)
{
}

CShellPropertyPage::~CShellPropertyPage(void)
{
}

void CShellPropertyPage::SetHwnd(HWND newHwnd)
{
    m_hwnd = newHwnd;
}

BOOL CShellPropertyPage::PageProc (HWND /*hwnd*/, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	switch (uMessage)
	{
	case WM_INITDIALOG:
		{
			InitWorkfileView();
			return TRUE;
		}
	case WM_NOTIFY:
		{
			LPNMHDR point = (LPNMHDR)lParam;
			int code = point->code;
			//
			// Respond to notifications.
			//    
			if (code == PSN_APPLY)
			{
			}
			SetWindowLongPtr (m_hwnd, DWLP_MSGRESULT, FALSE);
			return TRUE;        

			}
		case WM_DESTROY:
			return TRUE;

		case WM_COMMAND:
			switch (HIWORD(wParam))
			{
				case BN_CLICKED:
					//if (LOWORD(wParam) == IDC_SHOWLOG)
					//{
					//	STARTUPINFO startup;
					//	PROCESS_INFORMATION process;
					//	memset(&startup, 0, sizeof(startup));
					//	startup.cb = sizeof(startup);
					//	memset(&process, 0, sizeof(process));
					//	CRegStdString tortoiseProcPath(_T("Software\\TortoiseSVN\\ProcPath"), _T("TortoiseProc.exe"), false, HKEY_LOCAL_MACHINE);
					//	stdstring svnCmd = _T(" /command:");
					//	svnCmd += _T("log /path:\"");
					//	svnCmd += filenames.front().c_str();
					//	svnCmd += _T("\"");
					//	if (CreateProcess(tortoiseProcPath, const_cast<TCHAR*>(svnCmd.c_str()), NULL, NULL, FALSE, 0, 0, 0, &startup, &process))
					//	{
					//		CloseHandle(process.hThread);
					//		CloseHandle(process.hProcess);
					//	}
					//}
					//if (LOWORD(wParam) == IDC_EDITPROPERTIES)
					//{
					//	DWORD pathlength = GetTempPath(0, NULL);
					//	TCHAR * path = new TCHAR[pathlength+1];
					//	TCHAR * tempFile = new TCHAR[pathlength + 100];
					//	GetTempPath (pathlength+1, path);
					//	GetTempFileName (path, _T("svn"), 0, tempFile);
					//	stdstring retFilePath = stdstring(tempFile);

					//	HANDLE file = ::CreateFile (tempFile,
					//		GENERIC_WRITE, 
					//		FILE_SHARE_READ, 
					//		0, 
					//		CREATE_ALWAYS, 
					//		FILE_ATTRIBUTE_TEMPORARY,
					//		0);

					//	delete path;
					//	delete tempFile;
					//	if (file != INVALID_HANDLE_VALUE)
					//	{
					//		DWORD written = 0;
					//		for (std::vector<stdstring>::iterator I = filenames.begin(); I != filenames.end(); ++I)
					//		{
					//			::WriteFile (file, I->c_str(), I->size()*sizeof(TCHAR), &written, 0);
					//			::WriteFile (file, _T("\n"), 2, &written, 0);
					//		}
					//		::CloseHandle(file);

					//		STARTUPINFO startup;
					//		PROCESS_INFORMATION process;
					//		memset(&startup, 0, sizeof(startup));
					//		startup.cb = sizeof(startup);
					//		memset(&process, 0, sizeof(process));
					//		CRegStdString tortoiseProcPath(_T("Software\\TortoiseSVN\\ProcPath"), _T("TortoiseProc.exe"), false, HKEY_LOCAL_MACHINE);
					//		stdstring svnCmd = _T(" /command:");
					//		svnCmd += _T("properties /pathfile:\"");
					//		svnCmd += retFilePath.c_str();
					//		svnCmd += _T("\"");
					//		svnCmd += _T(" /deletepathfile");
					//		if (CreateProcess(tortoiseProcPath, const_cast<TCHAR*>(svnCmd.c_str()), NULL, NULL, FALSE, 0, 0, 0, &startup, &process))
					//		{
					//			CloseHandle(process.hThread);
					//			CloseHandle(process.hProcess);
					//		}
					//	}
					//}
					break;
			}
	}
	return FALSE;
}

void CShellPropertyPage::InitWorkfileView()
{
	WIN32_FILE_ATTRIBUTE_DATA fdata = {0};
	if (GetFileAttributesEx(filenames[0].c_str(), GetFileExInfoStandard, &fdata))
	{
		SetDTPCtrl(m_hwnd, IDC_DATECREATED, IDC_TIMECREATED, &fdata.ftCreationTime);
		SetDTPCtrl(m_hwnd, IDC_DATEMODIFIED, IDC_TIMEMODIFIED, &fdata.ftLastWriteTime);
		SetDTPCtrl(m_hwnd, IDC_DATEACCESSED, IDC_TIMEACCESSED, &fdata.ftLastAccessTime);
	}
}

void ReadDTPCtrl(HWND hwnd, UINT idcDatePicker, UINT idcTimePicker, FILETIME* pFiletime)
{
	SYSTEMTIME st = {0}, stDate = {0}, stTime = {0};
	FILETIME   ftLocal;

	SendDlgItemMessage(hwnd, idcDatePicker, DTM_GETSYSTEMTIME, 0, (LPARAM)&stDate);
	SendDlgItemMessage(hwnd, idcTimePicker, DTM_GETSYSTEMTIME, 0, (LPARAM)&stTime);

	st.wMonth  = stDate.wMonth;
	st.wDay    = stDate.wDay;
	st.wYear   = stDate.wYear;
	st.wHour   = stTime.wHour;
	st.wMinute = stTime.wMinute;
	st.wSecond = stTime.wSecond;

	SystemTimeToFileTime(&st, &ftLocal);
	LocalFileTimeToFileTime(&ftLocal, pFiletime);
}


void SetDTPCtrl(HWND hwnd, UINT idcDatePicker, UINT idcTimePicker, const FILETIME* pFiletime)
{
	SYSTEMTIME st;
	FILETIME   ftLocal;

	FileTimeToLocalFileTime(pFiletime, &ftLocal);
	FileTimeToSystemTime(&ftLocal, &st);

	SendDlgItemMessage(hwnd, idcDatePicker, DTM_SETSYSTEMTIME, 0, (LPARAM)&st);
	SendDlgItemMessage(hwnd, idcTimePicker, DTM_SETSYSTEMTIME, 0, (LPARAM)&st);
}


