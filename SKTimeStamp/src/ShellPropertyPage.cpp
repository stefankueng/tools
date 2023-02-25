// SkTimeStamp - Change file dates easily, directly from explorer

// Copyright (C) 2012-2013, 2023 - Stefan Kueng

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
#include "stdafx.h"

#include "ShellPropertyPage.h"
#include <sstream>
#include "ShellExt.h"

#include "resource.h"

// Nonmember function prototypes
BOOL CALLBACK PageProc(HWND, UINT, WPARAM, LPARAM);
UINT CALLBACK PropPageCallbackProc(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp);
// Misc utility functions.
void          readDtpCtrl(HWND hwnd, UINT idcDatePicker, UINT idcTimePicker, FILETIME* pFiletime);
void          setDtpCtrl(HWND hwnd, UINT idcDatePicker, UINT idcTimePicker, const FILETIME* pFiletime);

// CShellExt member functions (needed for IShellPropSheetExt)
STDMETHODIMP  CShellExt::AddPages(LPFNADDPROPSHEETPAGE lpfnAddPage,
                                  LPARAM               lParam)
{
    if (m_files.empty())
        return NOERROR;

    PROPSHEETPAGE psp;
    SecureZeroMemory(&psp, sizeof(PROPSHEETPAGE));
    CShellPropertyPage* sheetPage = new CShellPropertyPage(m_files);

    psp.dwSize                    = sizeof(psp);
    psp.dwFlags                   = PSP_USEREFPARENT | PSP_USETITLE | PSP_USEICONID | PSP_USECALLBACK;
    psp.hInstance                 = g_hmodThisDll;
    psp.pszTemplate               = MAKEINTRESOURCE(IDD_PROPPAGE);
    // psp.pszIcon = MAKEINTRESOURCE(IDI_APPSMALL);
    psp.pszTitle                  = L"TimeStamps";
    psp.pfnDlgProc                = reinterpret_cast<DLGPROC>(PageProc);
    psp.lParam                    = reinterpret_cast<LPARAM>(sheetPage);
    psp.pfnCallback               = PropPageCallbackProc;
    psp.pcRefParent               = &g_cRefThisDll;

    HPROPSHEETPAGE hPage          = CreatePropertySheetPage(&psp);

    if (hPage != nullptr)
    {
        if (!lpfnAddPage(hPage, lParam))
        {
            delete sheetPage;
            DestroyPropertySheetPage(hPage);
        }
    }

    return NOERROR;
}

STDMETHODIMP CShellExt::ReplacePage(UINT /*uPageID*/, LPFNADDPROPSHEETPAGE /*lpfnReplaceWith*/, LPARAM /*lParam*/)
{
    return E_FAIL;
}

/////////////////////////////////////////////////////////////////////////////
// Dialog procedures and other callback functions

BOOL CALLBACK PageProc(HWND hwnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    CShellPropertyPage* sheetPage;

    if (uMessage == WM_INITDIALOG)
    {
        sheetPage = reinterpret_cast<CShellPropertyPage*>(reinterpret_cast<LPPROPSHEETPAGEW>(lParam)->lParam);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(sheetPage));
        sheetPage->SetHwnd(hwnd);
    }
    else
    {
        sheetPage = reinterpret_cast<CShellPropertyPage*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (sheetPage != nullptr)
        return sheetPage->PageProc(hwnd, uMessage, wParam, lParam);

    return FALSE;
}

UINT CALLBACK PropPageCallbackProc(HWND /*hwnd*/, UINT uMsg, LPPROPSHEETPAGE ppsp)
{
    // Delete the page before closing.
    if (PSPCB_RELEASE == uMsg)
    {
        CShellPropertyPage* sheetPage = reinterpret_cast<CShellPropertyPage*>(ppsp->lParam);
        if (sheetPage != nullptr)
            delete sheetPage;
    }
    return 1;
}

// *********************** CShellPropertyPage *************************

CShellPropertyPage::CShellPropertyPage(const std::vector<std::wstring>& newFilenames)
    : m_hwnd(nullptr)
    , fileNames(newFilenames)
{
}

CShellPropertyPage::~CShellPropertyPage()
{
}

void CShellPropertyPage::SetHwnd(HWND newHwnd)
{
    m_hwnd = newHwnd;
}

BOOL CShellPropertyPage::PageProc(HWND /*hwnd*/, UINT uMessage, WPARAM wParam, LPARAM lParam)
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
            LPNMHDR point = reinterpret_cast<LPNMHDR>(lParam);
            int     code  = point->code;
            BOOL    bRet  = FALSE;
            switch (code)
            {
                case PSN_APPLY:
                {
                    FILETIME ftLastWriteTime, ftLastAccessTime, ftCreationTime;
                    // Retrieve the dates/times from the DTP controls.
                    readDtpCtrl(m_hwnd, IDC_DATECREATED, IDC_TIMECREATED, &ftCreationTime);
                    readDtpCtrl(m_hwnd, IDC_DATEMODIFIED, IDC_TIMEMODIFIED, &ftLastWriteTime);
                    readDtpCtrl(m_hwnd, IDC_DATEACCESSED, IDC_TIMEACCESSED, &ftLastAccessTime);

                    SetDates(ftCreationTime, ftLastWriteTime, ftLastAccessTime);

                    // Return PSNRET_NOERROR to allow the sheet to close if the user clicked OK.
                    SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, PSNRET_NOERROR);
                }
                break;
                case DTN_DATETIMECHANGE:
                {
                    // If the user changes any of the DTP controls, enable
                    // the Apply button.
                    SYSTEMTIME st  = {0};
                    LRESULT    res = SendDlgItemMessage(m_hwnd, IDC_DATECREATED, DTM_GETSYSTEMTIME, 0, reinterpret_cast<LPARAM>(&st));
                    SendDlgItemMessage(m_hwnd, IDC_TIMECREATED, WM_ENABLE, res == GDT_VALID, 0);
                    res = SendDlgItemMessage(m_hwnd, IDC_DATEACCESSED, DTM_GETSYSTEMTIME, 0, reinterpret_cast<LPARAM>(&st));
                    SendDlgItemMessage(m_hwnd, IDC_TIMEACCESSED, WM_ENABLE, res == GDT_VALID, 0);
                    res = SendDlgItemMessage(m_hwnd, IDC_DATEMODIFIED, DTM_GETSYSTEMTIME, 0, reinterpret_cast<LPARAM>(&st));
                    SendDlgItemMessage(m_hwnd, IDC_TIMEMODIFIED, WM_ENABLE, res == GDT_VALID, 0);

                    SendMessage(GetParent(m_hwnd), PSM_CHANGED, reinterpret_cast<WPARAM>(m_hwnd), 0);
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
                        FILETIME   ftLocal;
                        FILETIME   ftNull = {0};
                        SYSTEMTIME st;
                        GetSystemTime(&st);
                        SystemTimeToFileTime(&st, &ftLocal);
                        // 'touch' means to set the last modification time to the current time
                        SetDates(ftNull, ftLocal, ftNull);
                        InitWorkfileView(); // update the controls
                        return TRUE;
                    }
                    break;
            }
    }
    return FALSE;
}

void CShellPropertyPage::InitWorkfileView()
{
    WIN32_FILE_ATTRIBUTE_DATA fData          = {0};
    bool                      bValidCreate   = true;
    bool                      bValidWrite    = true;
    bool                      bValidAccessed = true;

    for (std::vector<std::wstring>::iterator it = fileNames.begin(); it != fileNames.end() && (bValidCreate || bValidWrite || bValidAccessed); ++it)
    {
        if (!GetFileAttributesEx(it->c_str(), GetFileExInfoStandard, &fData))
        {
            bValidCreate   = false;
            bValidWrite    = false;
            bValidAccessed = false;
        }
        else
            break;
    }
    if (bValidCreate)
    {
        setDtpCtrl(m_hwnd, IDC_DATECREATED, IDC_TIMECREATED, &fData.ftCreationTime);
    }
    else
    {
        setDtpCtrl(m_hwnd, IDC_DATECREATED, IDC_TIMECREATED, nullptr);
    }
    if (bValidWrite)
    {
        setDtpCtrl(m_hwnd, IDC_DATEMODIFIED, IDC_TIMEMODIFIED, &fData.ftLastWriteTime);
    }
    else
    {
        setDtpCtrl(m_hwnd, IDC_DATEMODIFIED, IDC_TIMEMODIFIED, nullptr);
    }
    if (bValidAccessed)
    {
        setDtpCtrl(m_hwnd, IDC_DATEACCESSED, IDC_TIMEACCESSED, &fData.ftLastAccessTime);
    }
    else
    {
        setDtpCtrl(m_hwnd, IDC_DATEACCESSED, IDC_TIMEACCESSED, nullptr);
    }
    if (fileNames.size() == 1)
    {
        // only one file/folder selected, show the full path
        SetDlgItemText(m_hwnd, IDC_FILEINFO, fileNames[0].c_str());
    }
    else if (fileNames.size() > 1)
    {
        // more than one file/folder selected, show only the number of
        // selected items as info text
        WCHAR buf[50]  = {0};
        WCHAR buf2[50] = {0};
        if (LoadString(g_hmodThisDll, IDS_FILEINFO, buf, _countof(buf)) == 0)
        {
            // load string failed, use hard coded string
            wcscpy_s(buf, _countof(buf), L"Selected %ld files/folders");
        }
        swprintf_s(buf2, _countof(buf2), buf, fileNames.size());
        SetDlgItemText(m_hwnd, IDC_FILEINFO, buf2);
    }
}

void CShellPropertyPage::SetDates(FILETIME ftCreationTime, FILETIME ftLastWriteTime, FILETIME ftLastAccessTime) const
{
    FILETIME                  ftLastWriteTime2, ftLastAccessTime2, ftCreationTime2;
    std::vector<std::wstring> failedFiles;
    for (const auto& fileName : fileNames)
    {
        HANDLE hFile = CreateFile(fileName.c_str(), FILE_WRITE_ATTRIBUTES | FILE_WRITE_EA | FILE_READ_ATTRIBUTES | FILE_READ_EA, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
        if (hFile != INVALID_HANDLE_VALUE)
        {
            if ((ftCreationTime.dwHighDateTime == 0 && ftCreationTime.dwLowDateTime == 0) ||
                (ftLastAccessTime.dwHighDateTime == 0 && ftLastAccessTime.dwLowDateTime == 0) ||
                (ftLastWriteTime.dwHighDateTime == 0 && ftLastWriteTime.dwLowDateTime == 0))
            {
                ftCreationTime2               = ftCreationTime;
                ftLastAccessTime2             = ftLastAccessTime;
                ftLastWriteTime2              = ftLastWriteTime;
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
                        failedFiles.push_back(fileName);
                }
                else
                {
                    // could not open the file
                    failedFiles.push_back(fileName);
                }
            }
            else
            {
                if (SetFileTime(hFile, &ftCreationTime, &ftLastAccessTime, &ftLastWriteTime) == FALSE)
                    failedFiles.push_back(fileName);
            }
            CloseHandle(hFile);
        }
        else
        {
            // could not open the file
            failedFiles.push_back(fileName);
        }
    }
    if (!failedFiles.empty())
    {
        // could not set the dates for one or more files
        // show an error message
        WCHAR buf[4096] = {0};
        if (LoadString(g_hmodThisDll, IDS_ERR_FILEDATES, buf, _countof(buf)) == 0)
        {
            // load string failed, use hard coded string
            wcscpy_s(buf, _countof(buf), L"Could not set the date/time for the following files:");
        }

        std::wstringstream strMsg;
        strMsg << buf;
        for (const auto& failedFile : failedFiles)
        {
            strMsg << L"\n\"" << failedFile.c_str() << L"\"";
        }

        MessageBox(m_hwnd, strMsg.str().c_str(), L"SKTimeStamp", MB_ICONERROR);
    }
}

void readDtpCtrl(HWND hwnd, UINT idcDatePicker, UINT idcTimePicker, FILETIME* pFiletime)
{
    SYSTEMTIME st = {0}, stDate = {0}, stTime = {0}, stAdjusted = {0};

    pFiletime->dwHighDateTime = 0;
    pFiletime->dwLowDateTime  = 0;

    if (SendDlgItemMessage(hwnd, idcDatePicker, DTM_GETSYSTEMTIME, 0, reinterpret_cast<LPARAM>(&stDate)) != GDT_VALID)
        return;
    if (SendDlgItemMessage(hwnd, idcTimePicker, DTM_GETSYSTEMTIME, 0, reinterpret_cast<LPARAM>(&stTime)) != GDT_VALID)
        return;

    st.wMonth  = stDate.wMonth;
    st.wDay    = stDate.wDay;
    st.wYear   = stDate.wYear;
    st.wHour   = stTime.wHour;
    st.wMinute = stTime.wMinute;
    st.wSecond = stTime.wSecond;

    TzSpecificLocalTimeToSystemTime(nullptr, &st, &stAdjusted);
    SystemTimeToFileTime(&stAdjusted, pFiletime);
}

void setDtpCtrl(HWND hwnd, UINT idcDatePicker, UINT idcTimePicker, const FILETIME* pFiletime)
{
    SYSTEMTIME st, stTemp;
    DWORD      flag = GDT_VALID;
    if (pFiletime)
    {
        FileTimeToSystemTime(pFiletime, &stTemp);
        SystemTimeToTzSpecificLocalTime(nullptr, &stTemp, &st);
    }
    else
    {
        flag = GDT_NONE;
    }

    SendDlgItemMessage(hwnd, idcDatePicker, DTM_SETSYSTEMTIME, flag, reinterpret_cast<LPARAM>(&st));
    SendDlgItemMessage(hwnd, idcTimePicker, DTM_SETSYSTEMTIME, 0, reinterpret_cast<LPARAM>(&st));
    SendDlgItemMessage(hwnd, idcTimePicker, WM_ENABLE, flag == GDT_VALID, 0);
}
