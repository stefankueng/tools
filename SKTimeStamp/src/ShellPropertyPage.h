#pragma once
#include <vector>
#include "ShellExt.h"


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
    /**
     * Sets the dates on the selected files and folders.
     * If a filetime is zero, the original date of the file/folder is used, i.e., the filedate is not changed.
     */
    void SetDates(FILETIME ftCreationTime, FILETIME ftLastWriteTime, FILETIME ftLastAccessTime);

    HWND m_hwnd;
    std::vector<std::wstring> filenames;
};
