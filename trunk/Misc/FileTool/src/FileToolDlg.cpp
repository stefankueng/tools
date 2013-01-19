// FileTool

// Copyright (C) 2013 - Stefan Kueng

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
#include "FileTool.h"
#include "FileToolDlg.h"
#include "CleanVerifyDlg.h"
#include "SysImageList.h"
#include "BrowseFolder.h"
#include "SmartHandle.h"
#include "ProgressDlg.h"
#include "DirFileEnum.h"

#include <time.h>
#include <Shellapi.h>
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "UxTheme.lib")
#pragma comment(lib, "Shlwapi.lib")

int getrand(int min, int max)
{
    return (rand()%(max-min)+min);
}

CFileToolDlg::CFileToolDlg(HWND hParent)
    : m_hParent(hParent)
    , m_pDropGroup(nullptr)
    , m_pDropList(nullptr)
    , m_pDropTarget(nullptr)
    , m_bAscending(true)
{
}

CFileToolDlg::~CFileToolDlg(void)
{
    delete m_pDropGroup;
    delete m_pDropList;
    delete m_pDropTarget;
}

LRESULT CFileToolDlg::DlgFunc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            InitDialog(hwndDlg, IDI_FILETOOL);

            HWND hLockGroup = GetDlgItem(hwndDlg, IDC_LOCKGROUP);
            m_pDropGroup = new CFileDropTarget(hLockGroup, *this);
            HWND hLockList = GetDlgItem(hwndDlg, IDC_LOCKLIST);
            m_pDropList = new CFileDropTarget(hLockList, *this);

            // the path edit control should work as a drop target for files and folders
            HWND hSearchPath = GetDlgItem(hwndDlg, IDC_PATH);
            m_pDropTarget = new CFileDropTarget(hSearchPath);
            RegisterDragDrop(hSearchPath, m_pDropTarget);
            // create the supported formats:
            FORMATETC ftetc={0};
            ftetc.cfFormat = CF_TEXT;
            ftetc.dwAspect = DVASPECT_CONTENT;
            ftetc.lindex = -1;
            ftetc.tymed = TYMED_HGLOBAL;
            m_pDropTarget->AddSuportedFormat(ftetc);
            ftetc.cfFormat=CF_HDROP;
            m_pDropTarget->AddSuportedFormat(ftetc);
            SHAutoComplete(GetDlgItem(*this, IDC_PATH), SHACF_FILESYSTEM|SHACF_AUTOSUGGEST_FORCE_ON);

            HWND hListControl = GetDlgItem(*this, IDC_LOCKLIST);
            DWORD exStyle = LVS_EX_DOUBLEBUFFER|LVS_EX_INFOTIP|LVS_EX_FULLROWSELECT;
            SetWindowTheme(hListControl, L"Explorer", NULL);
            ListView_DeleteAllItems(hListControl);

            int c = Header_GetItemCount(ListView_GetHeader(hListControl))-1;
            while (c>=0)
                ListView_DeleteColumn(hListControl, c--);

            ListView_SetExtendedListViewStyle(hListControl, exStyle);
            ListView_SetImageList(hListControl, (WPARAM)(HIMAGELIST)CSysImageList::GetInstance(), LVSIL_SMALL);
            LVCOLUMN lvc = {0};
            lvc.mask = LVCF_TEXT|LVCF_FMT;
            lvc.fmt = LVCFMT_LEFT;
            lvc.cx = -1;
            lvc.pszText = _T("Name");
            ListView_InsertColumn(hListControl, 0, &lvc);
            ListView_SetColumnWidth(hListControl, 0, LVSCW_AUTOSIZE_USEHEADER);

            SetDlgItemText(*this, IDC_FILECOUNT, L"1");
            SetDlgItemText(*this, IDC_FILESIZE, L"0");
            SetDlgItemText(*this, IDC_FILLFROM, L"0");
            SetDlgItemText(*this, IDC_FILLTO, L"255");
        }
        return TRUE;
    case WM_COMMAND:
        return DoCommand(LOWORD(wParam));
    case WM_NOTIFY:
        {
            if (wParam == IDC_LOCKLIST)
            {
                DoListNotify((LPNMITEMACTIVATE)lParam);
            }
        }
        return FALSE;
    case WM_DROPFILES:
        {
            HDROP hDrop = (HDROP)wParam;

            UINT cFiles = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
            for (UINT i = 0; i < cFiles; ++i)
            {
                int reqSize = DragQueryFile(hDrop, i, NULL, 0);
                if (reqSize)
                {
                    std::unique_ptr<TCHAR[]> szFileName(new TCHAR[reqSize+1]);
                    if (DragQueryFile(hDrop, i, szFileName.get(), reqSize+1))
                    {
                        LockFile(szFileName.get());
                    }
                }
            }
            FillLockList();
        }
        return TRUE;
    default:
        return FALSE;
    }
}


LRESULT CFileToolDlg::DoCommand(int id)
{
    switch (id)
    {
    case IDOK:
        break;
    case IDCANCEL:
        for (auto it = m_lockedFiles.cbegin(); it != m_lockedFiles.cend(); ++it)
        {
            CloseHandle(it->second);
        }
        EndDialog(*this, IDOK);
        break;
    case IDC_PATHBROWSE:
        {
            CBrowseFolder browse;

            auto path = GetDlgItemText(IDC_PATH);
            std::unique_ptr<WCHAR[]> pathbuf(new WCHAR[MAX_PATH_NEW]);
            wcscpy_s(pathbuf.get(), MAX_PATH_NEW, path.get());
            browse.SetInfo(_T("Select path to create files in"));
            if (browse.Show(*this, pathbuf.get(), MAX_PATH_NEW, pathbuf.get()) == CBrowseFolder::OK)
            {
                SetDlgItemText(*this, IDC_PATH, pathbuf.get());
            }
        }
        break;
    case IDC_CREATE:
        CreateFiles();
        break;
    case IDC_CLEAN:
        Clean();
        break;
    }
    return 1;
}


void CFileToolDlg::DoListNotify( LPNMITEMACTIVATE lpNMItemActivate )
{
    if (lpNMItemActivate->hdr.code == LVN_GETINFOTIP)
    {
        NMLVGETINFOTIP *pInfoTip = reinterpret_cast<NMLVGETINFOTIP*>(lpNMItemActivate);
        pInfoTip->pszText[0] = 0;

        // Which item number?
        LVITEM item = {0};
        item.mask = LVIF_PARAM|LVIF_STATE;
        item.stateMask = LVIS_SELECTED;
        item.iItem = pInfoTip->iItem;
        ListView_GetItem(lpNMItemActivate->hdr.hwndFrom, &item);
        if (item.lParam)
        {
            std::wstring path = (LPCWSTR)item.lParam;
            lstrcpyn(pInfoTip->pszText,path.c_str(), pInfoTip->cchTextMax);
        }
    }
    if (lpNMItemActivate->hdr.code == LVN_KEYDOWN)
    {
         NMLVKEYDOWN *pnkd = reinterpret_cast<NMLVKEYDOWN*>(lpNMItemActivate);
         if (pnkd)
         {
             switch (pnkd->wVKey)
             {
             case VK_DELETE:
                 {
                     HWND hListControl = GetDlgItem(*this, IDC_LOCKLIST);

                     LVITEM item = {0};
                     int i = 0;
                     while (i  <ListView_GetItemCount(hListControl))
                     {
                         item.mask = LVIF_PARAM|LVIF_STATE;
                         item.stateMask = LVIS_SELECTED;
                         item.iItem = i;
                         item.lParam = 0;
                         ListView_GetItem(hListControl, &item);
                         if ((item.state & LVIS_SELECTED) && (item.lParam))
                         {
                             std::wstring path = (LPCWSTR)item.lParam;
                             HANDLE hFile = m_lockedFiles[path];
                             CloseHandle(hFile);
                             m_lockedFiles.erase(path);
                         }
                         ++i;
                     }
                     FillLockList();
                 }
                 break;
             }
         }
    }
}

void CFileToolDlg::LockFile( LPCWSTR path )
{
    DWORD shareMode = 0;
    if (IsDlgButtonChecked(*this, IDC_SHAREDELETE) == BST_CHECKED)
        shareMode |= FILE_SHARE_DELETE;
    if (IsDlgButtonChecked(*this, IDC_SHAREREAD) == BST_CHECKED)
        shareMode |= FILE_SHARE_READ;
    if (IsDlgButtonChecked(*this, IDC_SHAREWRITE) == BST_CHECKED)
        shareMode |= FILE_SHARE_WRITE;
    HANDLE hFile = CreateFile(path, FILE_GENERIC_READ, shareMode, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        m_lockedFiles[path] = hFile;
    }
}

void CFileToolDlg::FillLockList()
{
    SetCursor(LoadCursor(NULL, IDC_APPSTARTING));
    RefreshCursor();

    HWND hListControl = GetDlgItem(*this, IDC_LOCKLIST);
    SendMessage(hListControl, WM_SETREDRAW, FALSE, 0);

    ListView_DeleteAllItems(hListControl);

    int index = 0;
    std::unique_ptr<TCHAR[]> pBuf = std::unique_ptr<TCHAR[]>(new TCHAR[32767]);
    for (auto it = m_lockedFiles.cbegin(); it != m_lockedFiles.cend(); ++it)
    {
        LVITEM lv = {0};
        lv.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
        std::wstring name = it->first.substr(it->first.find_last_of('\\')+1);
        _tcscpy_s(pBuf.get(), it->first.size()+1, name.c_str());
        lv.pszText = pBuf.get();
        lv.iImage = CSysImageList::GetInstance().GetFileIconIndex(it->first.c_str());
        lv.iItem = index++;
        lv.lParam = (LPARAM)it->first.c_str();
        ListView_InsertItem(hListControl, &lv);
    }
    ListView_SetColumnWidth(hListControl, 0, LVSCW_AUTOSIZE_USEHEADER);
    SendMessage(hListControl, WM_SETREDRAW, TRUE, 0);
    SetCursor(LoadCursor(NULL, IDC_ARROW));
    RefreshCursor();

    RedrawWindow(hListControl, NULL, NULL, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
}

void CFileToolDlg::CreateFiles()
{
    // get all the data
    std::wstring sPath          = GetDlgItemText(IDC_PATH).get();
    std::wstring sCount         = GetDlgItemText(IDC_FILECOUNT).get();
    std::wstring sName          = GetDlgItemText(IDC_FILENAME).get();
    std::wstring sSize          = GetDlgItemText(IDC_FILESIZE).get();
    std::wstring sFillFrom      = GetDlgItemText(IDC_FILLFROM).get();
    std::wstring sFillTo        = GetDlgItemText(IDC_FILLTO).get();
    bool bFolders               = (IsDlgButtonChecked(*this, IDC_CREATEFOLDERS) == BST_CHECKED);
    bool bRecursive             = (IsDlgButtonChecked(*this, IDC_RECURSE) == BST_CHECKED);

    __int64 nCount              = _wtoi64(sCount.c_str());
    __int64 nSize               = _wtoi64(sSize.c_str());
    int     nFillFrom           = _wtoi(sFillFrom.c_str());
    int     nFillTo             = _wtoi(sFillTo.c_str());

    // validate the parameters
    if (sPath.empty() || !PathIsDirectory(sPath.c_str()))
    {
        ShowEditBalloon(IDC_PATH, L"please specify a valid directory path to create the files/folders in", L"invalid path");
        return;
    }
    if (nCount < 0)
    {
        ShowEditBalloon(IDC_FILECOUNT, L"The number of files/folders to create must be at least 1", L"invalid file count");
        return;
    }
    if (sName.empty())
    {
        ShowEditBalloon(IDC_FILENAME, L"please specify a filename", L"invalid file name");
        return;
    }
    if (nSize < 0)
    {
        ShowEditBalloon(IDC_FILESIZE, L"The file size must be positive", L"invalid file size");
        return;
    }
    if (nFillFrom < 0)
    {
        ShowEditBalloon(IDC_FILLFROM, L"The fill value must be between 0 and 255", L"invalid fill value");
        return;
    }
    if (nFillTo < 0)
    {
        ShowEditBalloon(IDC_FILLTO, L"The fill value must be between 0 and 255", L"invalid fill value");
        return;
    }
    if (bFolders && (nSize > 0))
    {
        ShowEditBalloon(IDC_FILESIZE, L"The file size must be zero or empty if folders are to be created", L"invalid file size");
        return;
    }
    size_t pos = sName.find('?');

    if (pos == std::wstring::npos)
    {
        sName += L"?";
        pos = sName.find('?');
    }

    std::wstring filenameleft = sName.substr(0, pos);
    std::wstring filenameright;
    bool    leadzero   = false;
    int     padding    = 0;
    __int64 start      = 1;
    __int64 increment  = 1;
    ++pos;
    // format is: ?{0L(start,increment)}
    if ((sName.size() > pos) && (sName[pos] == '{'))
    {
        size_t closepos = sName.find('}');
        if ((closepos == std::wstring::npos) || (closepos <= pos))
        {
            ShowEditBalloon(IDC_FILENAME, L"missing closing bracket '}' in filename", L"invalid file name");
            return;
        }
        ++closepos;
        if (sName.size() > closepos)
            filenameright = sName.substr(closepos);
        ++pos;
        if (sName.size() > pos)
        {
            leadzero = (sName[pos] == '0');
            if (leadzero)
                ++pos;
            if (sName.size() > pos)
            {
                padding = _wtoi(sName.substr(pos).c_str());
                ++pos;
                if ((sName.size() > pos) && (sName[pos] == '('))
                {
                    closepos = sName.find(')');
                    if ((closepos == std::wstring::npos) || (closepos <= pos))
                    {
                        ShowEditBalloon(IDC_FILENAME, L"missing closing bracket ')' in filename", L"invalid file name");
                        return;
                    }
                    ++pos;
                    start = _wtoi(sName.substr(pos).c_str());
                    pos = sName.find(',', pos);
                    if (pos != std::wstring::npos)
                    {
                        ++pos;
                        if (sName.size() > pos)
                            increment = _wtoi(sName.substr(pos).c_str());
                    }
                }
            }
        }
    }

    std::vector<std::wstring> folderlist;
    folderlist.push_back(sPath);
    if (bRecursive)
    {
        CDirFileEnum enumerator(sPath);
        std::wstring dirpath;
        bool bIsDir = false;
        while (enumerator.NextFile(dirpath, &bIsDir))
        {
            if (bIsDir)
                folderlist.push_back(dirpath);
        }
    }

    srand((unsigned int)time(NULL));
    CProgressDlg progDlg;
    if (bFolders)
        progDlg.SetTitle(L"Creating folders");
    else
        progDlg.SetTitle(L"Creating files");
    progDlg.SetProgress64(0, nCount*folderlist.size());
    progDlg.SetTime();
    progDlg.ShowModeless(*this);
    __int64 currentCount = 0;
    __int64 recStart = start;
    for (auto dirIt = folderlist.cbegin(); dirIt != folderlist.cend(); ++dirIt)
    {
        start = recStart;
        for (__int64 i = 0; (i < nCount) && !progDlg.HasUserCancelled(); ++i)
        {
            wchar_t format[10] = {0};
            if (padding)
            {
                if (leadzero)
                    swprintf_s(format, _countof(format), L"%%0%dd", padding);
                else
                    swprintf_s(format, _countof(format), L"%%%dd", padding);
            }
            else
                wcscpy_s(format, L"%d");
            wchar_t buf[MAX_PATH] = {0};
            swprintf_s(buf, _countof(buf), format, start);
            start += increment;

            std::wstring filename = filenameleft + buf + filenameright;
            CTraceToOutputDebugString::Instance()(L"filename %s\n", filename.c_str());
            if (bFolders)
                progDlg.SetLine(1, L"Creating folder:");
            else
                progDlg.SetLine(1, L"Creating file:");
            const int writebufsize = 64*1024;
            std::wstring fullpath = *dirIt + L"\\" + filename;
            progDlg.SetLine(2, fullpath.c_str(), true);
            if (bFolders)
            {
                if (!CreateDirectory(fullpath.c_str(), NULL))
                {
                    CFormatMessageWrapper error(GetLastError());
                    std::unique_ptr<wchar_t[]> message(new wchar_t[writebufsize]);
                    swprintf_s(message.get(), writebufsize, L"Could not create directory\n%s\nError:\n%s", fullpath.c_str(), (LPCWSTR)error);
                    MessageBox(*this, message.get(), L"Create dir error", MB_ICONERROR);
                    return;
                }
            }
            else
            {
                CAutoFile hFile = CreateFile(fullpath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN, NULL);
                if (hFile.IsValid())
                {
                    __int64 bytesToWrite = nSize;
                    std::unique_ptr<BYTE[]> writebuf(new BYTE[writebufsize]);
                    while (bytesToWrite > 0)
                    {
                        // fill the buffer with the random data in the specified range
                        BYTE * pByte = writebuf.get();
                        for (int r = 0; r < writebufsize; ++r)
                        {
                            *pByte = (BYTE)getrand(nFillFrom, nFillTo);
                            ++pByte;
                        }
                        // now write the buffer to the file
                        DWORD written = 0;
                        BOOL writeRet = WriteFile(hFile, writebuf.get(), (DWORD)min(writebufsize, bytesToWrite), &written, NULL);
                        bytesToWrite -= written;
                        if (!writeRet)
                        {
                            CFormatMessageWrapper error(GetLastError());
                            std::unique_ptr<wchar_t[]> message(new wchar_t[writebufsize]);
                            swprintf_s(message.get(), writebufsize, L"Could not write to file\n%s\nError:\n%s", fullpath.c_str(), (LPCWSTR)error);
                            MessageBox(*this, message.get(), L"File write error", MB_ICONERROR);
                            return;
                        }
                    }
                }
                else
                {
                    CFormatMessageWrapper error(GetLastError());
                    std::unique_ptr<wchar_t[]> message(new wchar_t[writebufsize]);
                    swprintf_s(message.get(), writebufsize, L"Could not create file\n%s\nError:\n%s", fullpath.c_str(), (LPCWSTR)error);
                    MessageBox(*this, message.get(), L"File create error", MB_ICONERROR);
                    return;
                }
            }
            progDlg.SetProgress64(currentCount+1, nCount*folderlist.size());
            ++currentCount;
        }
    }
}

void CFileToolDlg::Clean()
{
    std::wstring sPath = GetDlgItemText(IDC_PATH).get();
    if (sPath.empty())
    {
        ShowEditBalloon(IDC_PATH, L"please specify a valid directory path to clean", L"invalid path");
        return;
    }
    CCleanVerifyDlg verDlg(*this, sPath);
    if (verDlg.DoModal(g_hInst, IDD_CLEANVERIFY, *this)!=IDOK)
        return;
    std::vector<std::wstring> folderlist;
    CDirFileEnum enumerator(sPath);
    std::wstring fpath;
    bool bDir = false;
    while (enumerator.NextFile(fpath, &bDir))
    {
        if (bDir)
            folderlist.push_back(fpath);
        else
        {
            if (DeleteFile(fpath.c_str()))
            {
                CFormatMessageWrapper error(GetLastError());
                std::unique_ptr<wchar_t[]> message(new wchar_t[65535]);
                swprintf_s(message.get(), 65535, L"Could not delete file\n%s\nError:\n%s", fpath.c_str(), (LPCWSTR)error);
                MessageBox(*this, message.get(), L"File delete error", MB_ICONERROR);
                return;
            }
        }
    }
    // delete the directories in reverse order
    for (auto it = folderlist.crbegin(); it != folderlist.crend(); ++it)
    {
        if (!RemoveDirectory(it->c_str()))
        {
            CFormatMessageWrapper error(GetLastError());
            std::unique_ptr<wchar_t[]> message(new wchar_t[65535]);
            swprintf_s(message.get(), 65535, L"Could not delete directory\n%s\nError:\n%s", it->c_str(), (LPCWSTR)error);
            MessageBox(*this, message.get(), L"Directory delete error", MB_ICONERROR);
            return;
        }
    }
}

