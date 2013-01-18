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
#include "resource.h"
#include "FileToolDlg.h"
#include "SysImageList.h"

#include <Shellapi.h>
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "UxTheme.lib")

CFileToolDlg::CFileToolDlg(HWND hParent)
    : m_hParent(hParent)
    , m_pDropGroup(nullptr)
    , m_pDropList(nullptr)
    , m_bAscending(true)
{
}

CFileToolDlg::~CFileToolDlg(void)
{
    delete m_pDropGroup;
    delete m_pDropList;
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

