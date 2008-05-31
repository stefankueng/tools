#include "stdafx.h"

#include "ShellPropertyPage.h"
#include "ShellExt.h"
#include <sstream>


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
			BOOL bRet = FALSE;
			switch (code)
			{
			case PSN_APPLY:
				{
					FILETIME ftLastWriteTime, ftLastAccessTime, ftCreationTime;
					// Retrieve the dates/times from the DTP controls.
					ReadDTPCtrl(m_hwnd, IDC_DATECREATED, IDC_TIMECREATED, &ftCreationTime);
					ReadDTPCtrl(m_hwnd, IDC_DATEMODIFIED, IDC_TIMEMODIFIED, &ftLastWriteTime);
					ReadDTPCtrl(m_hwnd, IDC_DATEACCESSED, IDC_TIMEACCESSED, &ftLastAccessTime);

					SetDates(ftCreationTime, ftLastWriteTime, ftLastAccessTime);

					// Return PSNRET_NOERROR to allow the sheet to close if the user clicked OK.
					SetWindowLong(m_hwnd, DWL_MSGRESULT, PSNRET_NOERROR);
				}
				break;
			case DTN_DATETIMECHANGE:
				{
					// If the user changes any of the DTP controls, enable
					// the Apply button.
					SYSTEMTIME st = {0};
					LRESULT res = SendDlgItemMessage(m_hwnd, IDC_DATECREATED, DTM_GETSYSTEMTIME, 0, (LPARAM)&st);
					SendDlgItemMessage(m_hwnd, IDC_TIMECREATED, WM_ENABLE, res == GDT_VALID, 0);
					res = SendDlgItemMessage(m_hwnd, IDC_DATEACCESSED, DTM_GETSYSTEMTIME, 0, (LPARAM)&st);
					SendDlgItemMessage(m_hwnd, IDC_TIMEACCESSED, WM_ENABLE, res == GDT_VALID, 0);
					res = SendDlgItemMessage(m_hwnd, IDC_DATEMODIFIED, DTM_GETSYSTEMTIME, 0, (LPARAM)&st);
					SendDlgItemMessage(m_hwnd, IDC_TIMEMODIFIED, WM_ENABLE, res == GDT_VALID, 0);

					SendMessage(GetParent(m_hwnd), PSM_CHANGED, (WPARAM)m_hwnd, 0);
				}
				break;
			}
			SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, FALSE);
			return bRet;
		}
		case WM_DESTROY:
			return TRUE;

		case WM_COMMAND:
			switch (HIWORD(wParam))
			{
				case BN_CLICKED:
					if (LOWORD(wParam) == IDC_TOUCH)
					{
						FILETIME ftLocal, ft;
						FILETIME ftNULL = {0};
						SYSTEMTIME st;
						GetSystemTime(&st);
						SystemTimeToFileTime(&st, &ftLocal);
						LocalFileTimeToFileTime(&ftLocal, &ft);
						// 'touch' means to set the last modification time to the current time
						SetDates(ftNULL, ft, ftNULL);
						InitWorkfileView();	// update the controls
						return TRUE;
					}
					break;
			}
	}
	return FALSE;
}

void CShellPropertyPage::InitWorkfileView()
{
	WIN32_FILE_ATTRIBUTE_DATA fdata = {0};
	WIN32_FILE_ATTRIBUTE_DATA fdata2 = {0};
	bool bValidCreate = true;
	bool bValidWrite = true;
	bool bValidAccessed = true;
	int count = 0;

	for (std::vector<std::wstring>::iterator it = filenames.begin(); it != filenames.end() && (bValidCreate || bValidWrite || bValidAccessed); ++it)
	{
		fdata2 = fdata;
		if (!GetFileAttributesEx(it->c_str(), GetFileExInfoStandard, &fdata))
		{
			bValidCreate = false;
			bValidWrite = false;
			bValidAccessed = false;
		}
		if (count > 0)
		{
			if ((fdata.ftCreationTime.dwHighDateTime != fdata2.ftCreationTime.dwHighDateTime) ||
				(fdata2.ftCreationTime.dwLowDateTime != fdata2.ftCreationTime.dwLowDateTime))
				bValidCreate = false;
			if ((fdata.ftLastWriteTime.dwHighDateTime != fdata2.ftLastWriteTime.dwHighDateTime) ||
				(fdata2.ftLastWriteTime.dwLowDateTime != fdata2.ftLastWriteTime.dwLowDateTime))
				bValidWrite = false;
			if ((fdata.ftLastAccessTime.dwHighDateTime != fdata2.ftLastAccessTime.dwHighDateTime) ||
				(fdata2.ftLastAccessTime.dwLowDateTime != fdata2.ftLastAccessTime.dwLowDateTime))
				bValidAccessed = false;
		}
		count++;
	}
	if (bValidCreate)
	{
		SetDTPCtrl(m_hwnd, IDC_DATECREATED, IDC_TIMECREATED, &fdata.ftCreationTime);
	}
	else
	{
		SetDTPCtrl(m_hwnd, IDC_DATECREATED, IDC_TIMECREATED, 0);
	}
	if (bValidWrite)
	{
		SetDTPCtrl(m_hwnd, IDC_DATEMODIFIED, IDC_TIMEMODIFIED, &fdata.ftLastWriteTime);
	}
	else
	{
		SetDTPCtrl(m_hwnd, IDC_DATEMODIFIED, IDC_TIMEMODIFIED, 0);
	}
	if (bValidAccessed)
	{
		SetDTPCtrl(m_hwnd, IDC_DATEACCESSED, IDC_TIMEACCESSED, &fdata.ftLastAccessTime);
	}
	else
	{
		SetDTPCtrl(m_hwnd, IDC_DATEACCESSED, IDC_TIMEACCESSED, 0);
	}
	if (filenames.size() == 1)
	{
		// only one file/folder selected, show the full path
		SetDlgItemText(m_hwnd, IDC_FILEINFO, filenames[0].c_str());
	}
	else if (filenames.size() > 1)
	{
		// more than one file/folder selected, show only the number of
		// selected items as info text
		TCHAR buf[50] = {0};
		TCHAR buf2[50] = {0};
		if (LoadString(g_hmodThisDll, IDS_FILEINFO, buf, sizeof(buf)/sizeof(TCHAR)) == 0)
		{
			// load string failed, use hard coded string
			_tcscpy_s(buf, 50, _T("Selected %ld files/folders"));
		}
		_stprintf_s(buf2, 50, buf, filenames.size());
		SetDlgItemText(m_hwnd, IDC_FILEINFO, buf2);
	}
}

void CShellPropertyPage::SetDates(FILETIME ftCreationTime, FILETIME ftLastWriteTime, FILETIME ftLastAccessTime)
{
	FILETIME ftLastWriteTime2, ftLastAccessTime2, ftCreationTime2;
	std::vector<std::wstring> failedFiles;
	for (std::vector<std::wstring>::iterator it = filenames.begin(); it != filenames.end(); ++it)
	{
		HANDLE hFile = CreateFile(it->c_str(), GENERIC_WRITE|GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			if ((ftCreationTime.dwHighDateTime == 0 && ftCreationTime.dwLowDateTime == 0) ||
				(ftLastAccessTime.dwHighDateTime == 0 && ftLastAccessTime.dwLowDateTime == 0) ||
				(ftLastWriteTime.dwHighDateTime == 0 && ftLastWriteTime.dwLowDateTime == 0))
			{
				ftCreationTime2 = ftCreationTime;
				ftLastAccessTime2 = ftLastAccessTime;
				ftLastWriteTime2 = ftLastWriteTime;
				BY_HANDLE_FILE_INFORMATION fi = {0};
				if (GetFileInformationByHandle(hFile, &fi))
				{
					if (ftCreationTime.dwHighDateTime == 0 && ftCreationTime.dwLowDateTime == 0)
						ftCreationTime2 = fi.ftCreationTime;
					if (ftLastAccessTime.dwHighDateTime == 0 && ftLastAccessTime.dwLowDateTime == 0)
						ftLastAccessTime2 = fi.ftLastAccessTime;
					if (ftLastWriteTime.dwHighDateTime == 0 && ftLastWriteTime.dwLowDateTime == 0)
						ftLastWriteTime2 = fi.ftLastWriteTime;
					if (SetFileTime(hFile, &ftCreationTime2, &ftLastAccessTime2, &ftLastWriteTime2) == FALSE)
						failedFiles.push_back(*it);
				}
				else
				{
					// could not open the file
					failedFiles.push_back(*it);
				}
			}
			else
			{
				if (SetFileTime(hFile, &ftCreationTime, &ftLastAccessTime, &ftLastWriteTime) == FALSE)
					failedFiles.push_back(*it);
			}
			CloseHandle(hFile);
		}
		else
		{
			// could not open the file
			failedFiles.push_back(*it);
		}
	}
	if (failedFiles.size() > 0)
	{
		// could not set the dates for one or more files
		// show an error message
		TCHAR buf[4096] = {0};
		if (LoadString(g_hmodThisDll, IDS_ERR_FILEDATES, buf, sizeof(buf)/sizeof(TCHAR)) == 0)
		{
			// load string failed, use hard coded string
			_tcscpy_s(buf, 4096, _T("Could not set the date/time for the following files:"));
		}

		std::wstringstream strMsg;
		strMsg << buf;
		for (std::vector<std::wstring>::iterator it = failedFiles.begin(); it != failedFiles.end(); ++it)
		{
			strMsg << _T("\n\"") << it->c_str() << _T("\"");
		}

		MessageBox(m_hwnd, strMsg.str().c_str(), _T("SKTimeStamp"), MB_ICONERROR);
	}
}

void ReadDTPCtrl(HWND hwnd, UINT idcDatePicker, UINT idcTimePicker, FILETIME* pFiletime)
{
	SYSTEMTIME st = {0}, stDate = {0}, stTime = {0};
	FILETIME   ftLocal;

	if (pFiletime)
	{
		pFiletime->dwHighDateTime = 0;
		pFiletime->dwLowDateTime = 0;
	}
	if (SendDlgItemMessage(hwnd, idcDatePicker, DTM_GETSYSTEMTIME, 0, (LPARAM)&stDate) != GDT_VALID)
		return;
	if (SendDlgItemMessage(hwnd, idcTimePicker, DTM_GETSYSTEMTIME, 0, (LPARAM)&stTime) != GDT_VALID)
		return;

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
	DWORD flag = GDT_VALID;
	if (pFiletime)
	{
		FileTimeToLocalFileTime(pFiletime, &ftLocal);
		FileTimeToSystemTime(&ftLocal, &st);
	}
	else
	{
		flag = GDT_NONE;
	}

	SendDlgItemMessage(hwnd, idcDatePicker, DTM_SETSYSTEMTIME, flag, (LPARAM)&st);
	SendDlgItemMessage(hwnd, idcTimePicker, DTM_SETSYSTEMTIME, 0, (LPARAM)&st);
	SendDlgItemMessage(hwnd, idcTimePicker, WM_ENABLE, flag == GDT_VALID, 0);
}


