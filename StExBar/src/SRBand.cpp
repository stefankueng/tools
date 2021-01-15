// StExBar - an explorer toolbar

// Copyright (C) 2007-2015, 2017-2021 - Stefan Kueng

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
#include "SRBand.h"
#include "Guid.h"
#include <strsafe.h>
#include <algorithm>
#include <vector>
#include "resource.h"
#include "SimpleIni.h"
#include <uxtheme.h>
#include <vsstyle.h>
#include "ChevronMenu.h"
#include "OptionsDlg.h"
#include "UnicodeUtils.h"
#include "PathUtils.h"
#include "StringUtils.h"
#include "NameDlg.h"
#include "DPIAware.h"
#include "GDIHelpers.h"
#include <commctrl.h>

#pragma comment(lib, "uxtheme.lib")

std::map<DWORD, CDeskBand*> CDeskBand::m_desklist; ///< set of CDeskBand objects which use the keyboard hook

CDeskBand::CDeskBand()
    : m_bFocus(false)
    , m_hwndParent(NULL)
    , m_hWnd(NULL)
    , m_hWndToolbar(NULL)
    , m_hWndEdit(NULL)
    , m_dwViewMode(0)
    , m_dwBandID(0)
    , m_pSite(NULL)
    , m_hToolbarImgList(NULL)
    , m_bCompositionState(false)
    , m_bDialogShown(FALSE)
    , m_hook(NULL)
    , m_bFilesSelected(false)
    , m_bFolderSelected(false)
    , m_bCmdEditEnabled(false)
    , m_regShowBtnText(L"Software\\StefansTools\\StExBar\\ShowButtonText", 1)
    , m_regUseUNCPaths(L"Software\\StefansTools\\StExBar\\UseUNCPaths", 1)
    , m_regEditBoxUsage(L"Software\\StefansTools\\StExBar\\EditBoxUsage", IDC_USECONSOLE)
    , m_regHideEditBox(L"Software\\StefansTools\\StExBar\\HideEditBox", 0)
    , m_currentFolder(NULL)
    , m_hwndListView(NULL)
    , m_newfolderTimeoutCounter(0)
    , m_pShouldAppsUseDarkMode(nullptr)
    , m_pAllowDarkModeForWindow(nullptr)
    , m_pIsDarkModeAllowedForWindow(nullptr)
    , m_pIsDarkModeAllowedForApp(nullptr)
    , m_pShouldSystemUseDarkMode(nullptr)
    , m_hUxthemeLib(0)
    , m_bDark(false)
    , m_bCanHaveDarkMode(false)
{
    m_ObjRefCount = 1;
    g_DllRefCount++;

    m_tbSize.cx = 0;
    m_tbSize.cy = 0;

    INITCOMMONCONTROLSEX used = {
        sizeof(INITCOMMONCONTROLSEX),
        ICC_STANDARD_CLASSES | ICC_BAR_CLASSES | ICC_COOL_CLASSES};
    InitCommonControlsEx(&used);

    m_bCanHaveDarkMode = false;
    PWSTR sysPath      = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_System, 0, nullptr, &sysPath)))
    {
        std::wstring dllPath = sysPath;
        CoTaskMemFree(sysPath);
        dllPath += L"\\uxtheme.dll";
        auto                      version = CPathUtils::GetVersionFromFile(L"uxtheme.dll");
        std::vector<std::wstring> tokens;
        stringtok(tokens, version, false, L".");
        if (tokens.size() == 4)
        {
            auto major = std::stol(tokens[0]);
            //auto minor = std::stol(tokens[1]);
            auto micro = std::stol(tokens[2]);
            //auto build = std::stol(tokens[3]);

            // the windows 10 update 1809 has the version
            // number as 10.0.17763.1
            if (major == 10 && micro > 17762)
                m_bCanHaveDarkMode = true;
        }
    }

    m_hUxthemeLib = LoadLibrary(L"uxtheme.dll");
    if (m_hUxthemeLib && m_bCanHaveDarkMode)
    {
        // Note: these functions are undocumented! Which meas I shouldn't even use them.
        // But the explorer dark mode in Win10 1809 makes the StExBar look ugly and out of place
        // if I don't do anything to make the toolbar dark as well.
        // So, since MS decided to keep this new feature to themselves, I have to use
        // undocumented functions to adjust.
        // Let's just hope they change their minds and document these functions one day...

        // first try with the names, just in case MS decides to properly export these functions
        m_pAllowDarkModeForWindow     = (AllowDarkModeForWindowFPN)GetProcAddress(m_hUxthemeLib, "AllowDarkModeForWindow");
        m_pShouldAppsUseDarkMode      = (ShouldAppsUseDarkModeFPN)GetProcAddress(m_hUxthemeLib, "ShouldAppsUseDarkMode");
        m_pIsDarkModeAllowedForWindow = (IsDarkModeAllowedForWindowFPN)GetProcAddress(m_hUxthemeLib, "IsDarkModeAllowedForWindow");
        m_pIsDarkModeAllowedForApp    = (IsDarkModeAllowedForAppFPN)GetProcAddress(m_hUxthemeLib, "IsDarkModeAllowedForApp");
        m_pShouldSystemUseDarkMode    = (ShouldSystemUseDarkModeFPN)GetProcAddress(m_hUxthemeLib, "ShouldSystemUseDarkMode");
        if (m_pAllowDarkModeForWindow == nullptr)
            m_pAllowDarkModeForWindow = (AllowDarkModeForWindowFPN)GetProcAddress(m_hUxthemeLib, MAKEINTRESOURCEA(133));
        if (m_pShouldAppsUseDarkMode == nullptr)
            m_pShouldAppsUseDarkMode = (ShouldAppsUseDarkModeFPN)GetProcAddress(m_hUxthemeLib, MAKEINTRESOURCEA(132));
        if (m_pIsDarkModeAllowedForWindow == nullptr)
            m_pIsDarkModeAllowedForWindow = (IsDarkModeAllowedForWindowFPN)GetProcAddress(m_hUxthemeLib, MAKEINTRESOURCEA(137));
        if (m_pIsDarkModeAllowedForApp == nullptr)
            m_pIsDarkModeAllowedForApp = (IsDarkModeAllowedForAppFPN)GetProcAddress(m_hUxthemeLib, MAKEINTRESOURCEA(139));
        if (m_pShouldSystemUseDarkMode == nullptr)
            m_pShouldSystemUseDarkMode = (ShouldSystemUseDarkModeFPN)GetProcAddress(m_hUxthemeLib, MAKEINTRESOURCEA(138));
    }
}

CDeskBand::~CDeskBand()
{
    // This should have been freed in a call to SetSite(NULL), but
    // it is defined here to be safe.
    if (m_pSite)
    {
        m_pSite->Release();
        m_pSite = NULL;
    }
    if (m_hook)
    {
        UnhookWindowsHookEx(m_hook);
        std::map<DWORD, CDeskBand*>::iterator it = m_desklist.find(GetCurrentThreadId());
        if (it != m_desklist.end())
            m_desklist.erase(it);
    }
    for (size_t i = 0; i < m_noShows.size(); ++i)
    {
        CoTaskMemFree(m_noShows[i]);
    }
    m_noShows.clear();

    if (m_currentFolder)
        CoTaskMemFree(m_currentFolder);

    // un-subclass
    RemoveWindowSubclass(::GetParent(m_hwndParent), DeskBandProc, (UINT_PTR)this);
    RemoveWindowSubclass(m_hWndEdit, EditProc, (UINT_PTR)this);

    if (m_hWnd)
    {
        DestroyWindow(m_hWnd);
        m_hWnd = NULL;
    }

    FreeLibrary(m_hUxthemeLib);

    g_DllRefCount--;
}

//////////////////////////////////////////////////////////////////////////
// IUnknown methods
//////////////////////////////////////////////////////////////////////////
STDMETHODIMP CDeskBand::QueryInterface(REFIID riid, LPVOID* ppReturn)
{
    *ppReturn = NULL;

    // IUnknown
    if (IsEqualIID(riid, IID_IUnknown))
    {
        *ppReturn = this;
    }

    // IOleWindow
    else if (IsEqualIID(riid, IID_IOleWindow))
    {
        *ppReturn = (IOleWindow*)this;
    }

    // IDockingWindow
    else if (IsEqualIID(riid, IID_IDockingWindow))
    {
        *ppReturn = (IDockingWindow*)this;
    }

    // IInputObject
    else if (IsEqualIID(riid, IID_IInputObject))
    {
        *ppReturn = (IInputObject*)this;
    }

    // IObjectWithSite
    else if (IsEqualIID(riid, IID_IObjectWithSite))
    {
        *ppReturn = (IObjectWithSite*)this;
    }

    // IDeskBand
    else if (IsEqualIID(riid, IID_IDeskBand))
    {
        *ppReturn = (IDeskBand*)this;
    }

    // IDeskBand2
    else if (IsEqualIID(riid, IID_IDeskBand2))
    {
        *ppReturn = (IDeskBand2*)this;
    }

    // IPersist
    else if (IsEqualIID(riid, IID_IPersist))
    {
        *ppReturn = (IPersist*)this;
    }

    // IPersistStream
    else if (IsEqualIID(riid, IID_IPersistStream))
    {
        *ppReturn = (IPersistStream*)this;
    }

    // IContextMenu
    else if (IsEqualIID(riid, IID_IContextMenu))
    {
        *ppReturn = (IContextMenu*)this;
    }

    // IContextMenu2
    else if (IsEqualIID(riid, IID_IContextMenu2))
    {
        *ppReturn = (IContextMenu2*)this;
    }

    // IContextMenu3
    else if (IsEqualIID(riid, IID_IContextMenu3))
    {
        *ppReturn = (IContextMenu3*)this;
    }

    // IShellExtInit
    else if (IsEqualIID(riid, IID_IShellExtInit))
    {
        *ppReturn = (IShellExtInit*)this;
    }
    if (*ppReturn)
    {
        (*(LPUNKNOWN*)ppReturn)->AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDMETHODIMP_(DWORD)
CDeskBand::AddRef()
{
    return ++m_ObjRefCount;
}

STDMETHODIMP_(DWORD)
CDeskBand::Release()
{
    if (--m_ObjRefCount == 0)
    {
        delete this;
        return 0;
    }

    return m_ObjRefCount;
}

//////////////////////////////////////////////////////////////////////////
// IOleWindow methods
//////////////////////////////////////////////////////////////////////////
STDMETHODIMP CDeskBand::GetWindow(HWND* phWnd)
{
    *phWnd = m_hWnd;

    return S_OK;
}

STDMETHODIMP CDeskBand::ContextSensitiveHelp(BOOL /*fEnterMode*/)
{
    return E_NOTIMPL;
}

//////////////////////////////////////////////////////////////////////////
// IDockingWindow methods
//////////////////////////////////////////////////////////////////////////
STDMETHODIMP CDeskBand::ShowDW(BOOL fShow)
{
    if (m_hWnd)
    {
        if (fShow)
        {
            // show our window
            ShowWindow(m_hWnd, SW_SHOW);
            ::SetTimer(m_hWnd, TID_IDLE, 3000, NULL);
        }
        else
        {
            // hide our window
            ShowWindow(m_hWnd, SW_HIDE);
            ::KillTimer(m_hWnd, TID_IDLE);
        }
    }

    return S_OK;
}

STDMETHODIMP CDeskBand::CloseDW(DWORD /*dwReserved*/)
{
    ShowDW(FALSE);

    if (IsWindow(m_hWnd))
    {
        ImageList_Destroy(m_hToolbarImgList);
        DestroyWindow(m_hWnd);
    }
    m_hWnd = NULL;

    for (size_t i = 0; i < m_noShows.size(); ++i)
    {
        CoTaskMemFree(m_noShows[i]);
    }
    m_noShows.clear();

    return S_OK;
}

STDMETHODIMP CDeskBand::ResizeBorderDW(LPCRECT /*prcBorder*/,
                                       IUnknown* /*punkSite*/,
                                       BOOL /*fReserved*/)
{
    // This method is never called for Band Objects.
    return E_NOTIMPL;
}

//////////////////////////////////////////////////////////////////////////
// IInputObject methods
//////////////////////////////////////////////////////////////////////////
STDMETHODIMP CDeskBand::UIActivateIO(BOOL fActivate, LPMSG /*pMsg*/)
{
    if (fActivate)
        SetFocus(m_hWnd);

    return S_OK;
}

STDMETHODIMP CDeskBand::HasFocusIO(void)
{
    // If this window or one of its descendants has the focus, return S_OK.
    // Return S_FALSE if neither has the focus.
    if (m_bFocus)
        return S_OK;

    return S_FALSE;
}

STDMETHODIMP CDeskBand::TranslateAcceleratorIO(LPMSG pMsg)
{
    // we have to translate the accelerator keys ourselves, otherwise
    // the edit control won't get the keys the explorer uses itself
    // (e.g. backspace)
    int nVirtKey = (int)(pMsg->wParam);
    if (VK_RETURN == nVirtKey)
    {
        // remove system beep on enter key by setting key code to 0
        pMsg->wParam = 0;
        ::PostMessage(m_hWnd, WM_COMMAND, BN_CLICKED, 0);
        return S_OK;
    }
    else if (WM_KEYDOWN == pMsg->message && nVirtKey == VK_TAB)
    {
        // loose the focus
        FocusChange(FALSE);
        return S_FALSE;
    }

    TranslateMessage(pMsg);
    DispatchMessage(pMsg);
    return S_OK;
}

//////////////////////////////////////////////////////////////////////////
// IObjectWithSite methods
//////////////////////////////////////////////////////////////////////////
STDMETHODIMP CDeskBand::SetSite(IUnknown* punkSite)
{
    // If a site is being held, release it.
    if (m_pSite)
    {
        m_pSite->Release();
        m_pSite = NULL;
    }
    if ((m_hook) && (punkSite))
    {
        UnhookWindowsHookEx(m_hook);
        std::map<DWORD, CDeskBand*>::iterator it = m_desklist.find(GetCurrentThreadId());
        if (it != m_desklist.end())
            m_desklist.erase(it);
    }
    for (size_t i = 0; i < m_noShows.size(); ++i)
    {
        CoTaskMemFree(m_noShows[i]);
    }
    m_noShows.clear();

    m_tbSize.cx = 0;
    m_tbSize.cy = 0;

    // If punkSite is not NULL, a new site is being set.
    if (punkSite)
    {
        // Get the parent window.
        IOleWindow* pOleWindow;

        m_hwndParent = NULL;

        if (SUCCEEDED(punkSite->QueryInterface(IID_IOleWindow,
                                               (LPVOID*)&pOleWindow)))
        {
            pOleWindow->GetWindow(&m_hwndParent);
            pOleWindow->Release();
        }

        if (!m_hwndParent)
            return E_FAIL;

        if (!RegisterAndCreateWindow())
            return E_FAIL;

        if (!BuildToolbarButtons())
            return E_FAIL;

        SetTheme();

        // Get and keep the IInputObjectSite pointer.
        if (FAILED(punkSite->QueryInterface(IID_IInputObjectSite,
                                            (LPVOID*)&m_pSite)))
        {
            return E_FAIL;
        }

        m_hook                           = SetWindowsHookEx(WH_KEYBOARD, KeyboardHookProc, NULL, GetCurrentThreadId());
        m_desklist[GetCurrentThreadId()] = this;

        return S_OK;
    }

    return S_OK;
}

STDMETHODIMP CDeskBand::GetSite(REFIID riid, LPVOID* ppvReturn)
{
    *ppvReturn = NULL;

    if (m_pSite)
        return m_pSite->QueryInterface(riid, ppvReturn);

    return E_FAIL;
}

//////////////////////////////////////////////////////////////////////////
// IDeskBand methods
//////////////////////////////////////////////////////////////////////////
STDMETHODIMP CDeskBand::GetBandInfo(DWORD dwBandID, DWORD dwViewMode, DESKBANDINFO* pdbi)
{
    if (pdbi)
    {
        m_dwBandID   = dwBandID;
        m_dwViewMode = dwViewMode;

        if (pdbi->dwMask & DBIM_MINSIZE)
        {
            if (DBIF_VIEWMODE_FLOATING & dwViewMode) // Duplicate if/else branches
            {
                pdbi->ptMinSize.x = 0;
                pdbi->ptMinSize.y = m_tbSize.cy;
            }
            else
            {
                pdbi->ptMinSize.x = 0;
                pdbi->ptMinSize.y = m_tbSize.cy;
            }
        }

        if (pdbi->dwMask & DBIM_MAXSIZE)
        {
            pdbi->ptMaxSize.x = -1;
            pdbi->ptMaxSize.y = m_tbSize.cy;
        }

        if (pdbi->dwMask & DBIM_INTEGRAL)
        {
            pdbi->ptIntegral.x = 1;
            pdbi->ptIntegral.y = 1;
        }

        if (pdbi->dwMask & DBIM_ACTUAL)
        {
            pdbi->ptActual.x = m_tbSize.cx; // + (m_bCmdEditEnabled ? EDITBOXSIZEX : 0);
            pdbi->ptActual.y = m_tbSize.cy;
        }

        if (pdbi->dwMask & DBIM_TITLE)
        {
            StringCchCopy(pdbi->wszTitle, 256, L"StEx");
        }

        if (pdbi->dwMask & DBIM_MODEFLAGS)
        {
            // We want chevrons
            pdbi->dwModeFlags = DBIMF_NORMAL | DBIMF_USECHEVRON;
            if (m_bDark)
                pdbi->dwModeFlags |= DBIMF_BKCOLOR;
        }

        if (pdbi->dwMask & DBIM_BKCOLOR)
        {
            if (m_bDark)
            {
                // unfortunately, this doesn't really do anything.
                // at least in my tests.
                // but since the docs say this is the way to do it...
                pdbi->crBkgnd = GetSysColor(COLOR_WINDOWTEXT);
            }
            else
            {
                // Use the default background color by removing this flag.
                pdbi->dwMask &= ~DBIM_BKCOLOR;
            }
        }

        return S_OK;
    }

    return E_INVALIDARG;
}

//////////////////////////////////////////////////////////////////////////
// IDeskBand methods
//////////////////////////////////////////////////////////////////////////
STDMETHODIMP CDeskBand::CanRenderComposited(BOOL* pfCanRenderComposited)
{
    if (pfCanRenderComposited)
        *pfCanRenderComposited = TRUE;
    return S_OK;
}

STDMETHODIMP CDeskBand::GetCompositionState(BOOL* pfCompositionState)
{
    if (pfCompositionState)
        *pfCompositionState = m_bCompositionState;
    return S_OK;
}

STDMETHODIMP CDeskBand::SetCompositionState(BOOL CompositionState)
{
    m_bCompositionState = CompositionState;
    return S_OK;
}

//////////////////////////////////////////////////////////////////////////
// IPersistStream methods
//////////////////////////////////////////////////////////////////////////
STDMETHODIMP CDeskBand::GetClassID(LPCLSID pClassID)
{
    // this is the only method of the IPersistStream interface we need
    *pClassID = CLSID_StExBand;

    return S_OK;
}

STDMETHODIMP CDeskBand::IsDirty(void)
{
    return S_FALSE;
}

STDMETHODIMP CDeskBand::Load(LPSTREAM /*pStream*/)
{
    return S_OK;
}

STDMETHODIMP CDeskBand::Save(LPSTREAM /*pStream*/, BOOL /*fClearDirty*/)
{
    return S_OK;
}

STDMETHODIMP CDeskBand::GetSizeMax(ULARGE_INTEGER* /*pul*/)
{
    return E_NOTIMPL;
}

//////////////////////////////////////////////////////////////////////////
// helper functions for our own DeskBand
//////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK CDeskBand::WndProc(HWND   hWnd,
                                    UINT   uMessage,
                                    WPARAM wParam,
                                    LPARAM lParam)
{
    CDeskBand* pThis = (CDeskBand*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    switch (uMessage)
    {
        case WM_NCCREATE:
        {
            LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
            pThis               = (CDeskBand*)(lpcs->lpCreateParams);
            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);

            //set the window handle
            pThis->m_hWnd = hWnd;
        }
        break;

        case WM_TIMER:
            if (wParam == TID_IDLE)
            {
                if (pThis->FindPaths())
                {
                    // current path changed, clear the filter
                    if (DWORD(pThis->GetEditBoxUsage()) == IDC_USEFILTER)
                    {
                        auto buf = pThis->GetEditBoxText();
                        if (buf.get() && buf.get()[0])
                        {
                            SetWindowText(pThis->m_hWndEdit, L"");
                            SetTimer(pThis->m_hWnd, TID_FILTER, 200, NULL);
                        }
                    }
                }
                // now enable/disable the commands depending
                // on view and the selected items

                WORD state = 0;
                if (pThis->m_currentDirectory.empty())
                    state |= ENABLED_NOVIEWPATH;
                else
                    state |= ENABLED_VIEWPATH;
                if (pThis->m_bFilesSelected)
                    state |= ENABLED_FILESELECTED;
                if (pThis->m_bFolderSelected)
                    state |= ENABLED_FOLDERSELECTED;
                if (pThis->m_selectedItems.empty())
                    state |= ENABLED_NOSELECTION;
                for (std::map<int, DWORD>::iterator it = pThis->m_enablestates.begin(); it != pThis->m_enablestates.end(); ++it)
                {
                    if (((it->second & 0xFFFF) & state) &&
                        ((HIWORD(it->second) == 0) || ((pThis->m_selectedItems.empty()) && (it->second & ENABLED_NOSELECTION)) || (HIWORD(it->second) == !(pThis->m_selectedItems.empty()))))
                        ::SendMessage(pThis->m_hWndToolbar, TB_ENABLEBUTTON, it->first, (LPARAM)TRUE);
                    else
                        ::SendMessage(pThis->m_hWndToolbar, TB_ENABLEBUTTON, it->first, (LPARAM)FALSE);
                }
                // the "show/hide system files" button is special: it has a pressed state if the
                // hidden files are shown
                SHELLSTATE shellstate = {0};
                SHGetSetSettings(&shellstate, SSF_SHOWSYSFILES | SSF_SHOWSUPERHIDDEN | SSF_SHOWALLOBJECTS | SSF_SHOWEXTENSIONS, FALSE);
                for (int i = 0; i < pThis->m_commands.GetCount(); ++i)
                {
                    if (pThis->m_commands.GetCommandPtr(i)->name.compare(L"Show system files") == 0)
                    {
                        ::SendMessage(pThis->m_hWndToolbar, TB_CHECKBUTTON, i, (LPARAM)shellstate.fShowAllObjects);
                    }
                    if (pThis->m_commands.GetCommandPtr(i)->name.compare(L"Show extensions") == 0)
                    {
                        ::SendMessage(pThis->m_hWndToolbar, TB_CHECKBUTTON, i, (LPARAM)shellstate.fShowExtensions);
                    }
                }
                ::SetTimer(pThis->m_hWnd, TID_IDLE, 500, NULL);
            }
            if (wParam == TID_FILTER)
            {
                KillTimer(pThis->m_hWnd, TID_FILTER);
                if (DWORD(pThis->GetEditBoxUsage()) == IDC_USEFILTER)
                {
                    auto buf = pThis->GetEditBoxText();
                    // select the files which match the filter string
                    pThis->Filter(buf.get());
                }
            }
            break;
        case WM_COMMAND:
            return pThis->OnCommand(wParam, lParam);

        case WM_SETFOCUS:
            return pThis->OnSetFocus();

        case WM_KILLFOCUS:
            return pThis->OnKillFocus();

        case WM_SIZE:
            return pThis->OnSize(lParam);

        case WM_MOVE:
            return pThis->OnMove(lParam);

        case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;
            if (pnmh->code == TTN_GETDISPINFO)
            {
                // tooltips requested...
                LPNMTTDISPINFO                        lpnmtdi = (LPNMTTDISPINFO)lParam;
                std::map<int, std::wstring>::iterator it      = pThis->m_tooltips.find((int)pnmh->idFrom);
                if (it != pThis->m_tooltips.end())
                {
                    wcsncpy_s(lpnmtdi->lpszText, 80, it->second.c_str(), 79);
                }
            }
        }
        break;

        case WM_CTLCOLOREDIT:
            if (pThis->m_bDark)
            {
                // while the edit control gets the style "SearchBoxEditComposited", that's not
                // enough: the text color still stays black. So we have to change that here.
                SetTextColor((HDC)wParam, GetSysColor(COLOR_WINDOW));
                return TRUE;
            }
            break;
        case WM_ERASEBKGND:
        {
            HDC  hDC = (HDC)wParam;
            RECT rc;
            ::GetClientRect(pThis->m_hWnd, &rc);
            if (pThis->m_bDark)
            {
                // in dark mode, just paint the whole background in black
                GDIHelpers::FillSolidRect(hDC, &rc, GetSysColor(COLOR_WINDOWTEXT));
                return TRUE;
            }
            // only draw the themed background if themes are enabled
            if (IsThemeActive())
            {
                HTHEME hTheme = OpenThemeData(pThis->m_hWnd, L"Rebar");
                if (hTheme)
                {
                    // now draw the themed background of a rebar control, because
                    // that's what we're actually in and should look like.
                    if (IsThemeBackgroundPartiallyTransparent(hTheme, RP_BAND, 0))
                    {
                        DrawThemeParentBackground(pThis->m_hWnd, hDC, &rc);
                    }
                    DrawThemeBackground(hTheme, hDC, RP_BAND, 0, &rc, NULL);
                    CloseThemeData(hTheme);
                    return TRUE; // we've drawn the background
                }
            }
            // just do nothing so the system knows that we haven't erased the background
        }
        break;
        case WM_SETTINGCHANGE:
        case WM_SYSCOLORCHANGE:
            pThis->SetTheme();
            break;
    }

    return DefWindowProc(hWnd, uMessage, wParam, lParam);
}

LRESULT CALLBACK CDeskBand::EditProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam, UINT_PTR /*uIdSubclass*/, DWORD_PTR dwRefData)
{
    CDeskBand* pThis = (CDeskBand*)dwRefData;
    if (pThis == NULL)
        return 0;
    if (uMessage == WM_SETFOCUS)
    {
        pThis->OnSetFocus();
        ::SendMessage(pThis->m_hWndEdit, EM_SETSEL, 0, (LPARAM)-1);
        switch (pThis->GetEditBoxUsage())
        {
            case IDC_USEFILTER:
                ::SendMessage(pThis->m_hWndEdit, EM_SETCUEBANNER, TRUE, (LPARAM)L"Filter items");
                break;
            case IDC_USECONSOLE:
                ::SendMessage(pThis->m_hWndEdit, EM_SETCUEBANNER, TRUE, (LPARAM)L"Console commands");
                break;
            case IDC_USEPOWERSHELL:
                ::SendMessage(pThis->m_hWndEdit, EM_SETCUEBANNER, TRUE, (LPARAM)L"Powershell commands");
                break;
            case IDC_USEGREPWIN:
                ::SendMessage(pThis->m_hWndEdit, EM_SETCUEBANNER, TRUE, (LPARAM)L"search with grepWin");
                break;
            case IDC_USEAUTO:
                ::SendMessage(pThis->m_hWndEdit, EM_SETCUEBANNER, TRUE, (LPARAM)L"Auto command - use f, c, p or g");
                break;
        }
    }
    if ((uMessage == WM_LBUTTONDBLCLK) ||
        ((uMessage == WM_KEYDOWN) && (wParam == VK_ESCAPE)))
    {
        ::SetWindowText(pThis->m_hWndEdit, L"");
        // clear the filter
        pThis->Filter(L"");
    }
    return DefSubclassProc(hWnd, uMessage, wParam, lParam);
}

LRESULT CALLBACK CDeskBand::DeskBandProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam, UINT_PTR /*uIdSubclass*/, DWORD_PTR dwRefData)
{
    CDeskBand* pThis = (CDeskBand*)dwRefData;
    if (pThis == NULL)
        return 0;
    LPNMREBARCHEVRON pnmh = (LPNMREBARCHEVRON)lParam;
    if ((uMessage == WM_NOTIFY) && (pnmh->hdr.code == RBN_CHEVRONPUSHED) && (pnmh->wID == pThis->m_dwBandID))
    {
        CChevronMenu menu(g_hInst);
        if (menu.Show(pnmh, pThis->m_hWndToolbar))
        {
            ::SendMessage(pThis->m_hWndToolbar, menu.m_uMsg, menu.m_wParam, menu.m_lParam);
        }
        return 0;
    }
    return DefSubclassProc(hWnd, uMessage, wParam, lParam);
}

LRESULT CDeskBand::OnCommand(WPARAM wParam, LPARAM /*lParam*/)
{
    switch (HIWORD(wParam))
    {
        case EN_CHANGE:
        {
            SetTimer(m_hWnd, TID_FILTER, 500, NULL);
        }
        break;
        case BN_CLICKED:
        {
            // button was pressed
            // but was it really pressed or did the keyboard hook send the message?
            // we have to check if the command is really enabled:

            WORD state = 0;
            if (m_currentDirectory.empty())
                state |= ENABLED_NOVIEWPATH;
            else
                state |= ENABLED_VIEWPATH;
            if (m_bFilesSelected)
                state |= ENABLED_FILESELECTED;
            if (m_bFolderSelected)
                state |= ENABLED_FOLDERSELECTED;
            if (m_selectedItems.empty())
                state |= ENABLED_NOSELECTION;
            std::map<int, DWORD>::iterator it       = m_enablestates.find(LOWORD(wParam));
            bool                           bEnabled = true;
            if (it != m_enablestates.end())
            {
                if (((it->second & 0xFFFF) & state) &&
                    ((HIWORD(it->second) == 0) || ((m_selectedItems.empty()) && (it->second & ENABLED_NOSELECTION)) || (HIWORD(it->second) == m_selectedItems.size())))
                    bEnabled = true;
                else
                    bEnabled = false;
            }
            if (!bEnabled)
                return 0;

            FindPaths();

            // find the command
            if (LOWORD(wParam) >= m_commands.GetCount())
                DebugBreak();
            Command cmd = m_commands.GetCommand(LOWORD(wParam));

            if (m_currentDirectory.empty())
            {
                WCHAR buf[MAX_PATH] = {0};
                GetCurrentDirectory(_countof(buf), buf);
                m_currentDirectory = buf;
            }

            HandleCommand(m_hWnd, cmd, m_currentDirectory, m_selectedItems);

            FocusChange(false);
            break;
        }
    }
    return 0;
}

void CDeskBand::HandleCommand(HWND hWnd, const Command& cmd, const std::wstring& cwd, const std::map<std::wstring, ULONG>& items)
{
    if (cmd.commandline.compare(INTERNALCOMMAND) == 0)
    {
        // an internal command
        if (cmd.name.compare(L"StexBar Internal Edit Box") == 0)
        {
            // get the command entered in the edit box
            auto buf = GetEditBoxText();
            switch (GetEditBoxUsage())
            {
                case IDC_USEFILTER:
                    // select the files which match the filter string
                    Filter(buf.get());
                    break;
                case IDC_USEPOWERSHELL:
                    // when we start the console with the command the user
                    // has entered in the edit box, we want the console
                    // to execute the command immediately, and *not* quit after
                    // executing the command so the user can see the output.
                    // If however the user enters a '@' char in front of the command
                    // then the console shall quit after executing the command.
                    {
                        auto ps = L"-NoExit -Command \"Set-Location -LiteralPath '" + cwd + L"'\"";

                        std::wstring params;
                        if (buf[0] == '@')
                            params = L"-Command \"Set-Location -LiteralPath '" + cwd + L"'\" ; ";
                        else
                            params = L"-NoExit -Command \"Set-Location -LiteralPath '" + cwd + L"'\" ; ";
                        params += buf.get();
                        StartPS(cwd, params, (GetKeyState(VK_LWIN) & 0x8000) != 0);
                    }
                    break;
                case IDC_USECONSOLE:
                {
                    std::wstring params;
                    if (buf[0] == '@')
                        params = L"/c ";
                    else
                        params = L"/k ";
                    params += buf.get();
                    StartCmd(cwd, params, (GetKeyState(VK_LWIN) & 0x8000) != 0);
                }
                break;
                case IDC_USEGREPWIN:
                {
                    CRegStdString regGrepWinPath = CRegStdString(L"*\\Shell\\grepWin\\command\\", L"", 0, HKEY_CLASSES_ROOT);
                    std::wstring  grepWinPath    = regGrepWinPath;
                    grepWinPath                  = grepWinPath.substr(0, grepWinPath.find_last_of('/'));
                    std::wstring params          = grepWinPath;
                    auto         slashCwd        = cwd;
                    SearchReplace(slashCwd, L"\\", L"/");
                    std::wstring searchFor = buf.get();
                    SearchReplace(searchFor, L"\"", L"\\\"");
                    params += L" /searchpath:\"";
                    params += slashCwd;
                    params += L"\" /searchfor:\"";
                    params += searchFor;
                    params += L"\"";
                    StartApplication(cwd, params, (GetKeyState(VK_LWIN) & 0x8000) != 0);
                }
                break;
            }
        }
        else if (cmd.name.compare(L"Options") == 0)
        {
            COptionsDlg dlg(hWnd);
            m_bDialogShown = TRUE;
            if (dlg.DoModal(g_hInst, IDD_OPTIONS, hWnd) == IDOK)
            {
                m_bDialogShown = FALSE;
                m_regEditBoxUsage.read();
                m_regUseUNCPaths.read();
                m_regShowBtnText.read();
                BuildToolbarButtons();
                SetTheme();
                OnMove(0);
            }
            m_bDialogShown = FALSE;
        }
        else if (cmd.name.compare(L"Show system files") == 0)
        {
            HCURSOR hCur = GetCursor();
            SetCursor(LoadCursor(NULL, IDC_WAIT));
            SHELLSTATE state = {0};
            SHGetSetSettings(&state, SSF_SHOWSYSFILES | SSF_SHOWSUPERHIDDEN | SSF_SHOWALLOBJECTS, FALSE);
            state.fShowSysFiles    = !state.fShowAllObjects;
            state.fShowAllObjects  = !state.fShowAllObjects;
            state.fShowSuperHidden = !state.fShowAllObjects;
            SHGetSetSettings(&state, SSF_SHOWSYSFILES | SSF_SHOWSUPERHIDDEN | SSF_SHOWALLOBJECTS, TRUE);
            // now refresh the view
            IServiceProvider* pServiceProvider;
            if (m_pSite)
            {
                if (SUCCEEDED(m_pSite->QueryInterface(IID_IServiceProvider, (LPVOID*)&pServiceProvider)))
                {
                    IShellBrowser* pShellBrowser;
                    if (SUCCEEDED(pServiceProvider->QueryService(SID_SShellBrowser, IID_IShellBrowser, (LPVOID*)&pShellBrowser)))
                    {
                        IShellView* pShellView;
                        if (SUCCEEDED(pShellBrowser->QueryActiveShellView(&pShellView)))
                        {
                            pShellView->Refresh();
                            pShellView->Release();
                        }
                        pShellBrowser->Release();
                    }
                    pServiceProvider->Release();
                }
            }
            SetCursor(hCur);
        }
        else if (cmd.name.compare(L"Show extensions") == 0)
        {
            HCURSOR hCur = GetCursor();
            SetCursor(LoadCursor(NULL, IDC_WAIT));
            SHELLSTATE state = {0};
            SHGetSetSettings(&state, SSF_SHOWEXTENSIONS, FALSE);
            state.fShowExtensions = !state.fShowExtensions;
            SHGetSetSettings(&state, SSF_SHOWEXTENSIONS, TRUE);
            // now refresh the view
            IServiceProvider* pServiceProvider;
            if (m_pSite)
            {
                if (SUCCEEDED(m_pSite->QueryInterface(IID_IServiceProvider, (LPVOID*)&pServiceProvider)))
                {
                    IShellBrowser* pShellBrowser;
                    if (SUCCEEDED(pServiceProvider->QueryService(SID_SShellBrowser, IID_IShellBrowser, (LPVOID*)&pShellBrowser)))
                    {
                        IShellView* pShellView;
                        if (SUCCEEDED(pShellBrowser->QueryActiveShellView(&pShellView)))
                        {
                            pShellView->Refresh();
                            pShellView->Release();
                        }
                        pShellBrowser->Release();
                    }
                    pServiceProvider->Release();
                }
            }
            SetCursor(hCur);
        }
        else if (cmd.name.compare(L"Up") == 0)
        {
            IServiceProvider* pServiceProvider;
            if (m_pSite)
            {
                if (SUCCEEDED(m_pSite->QueryInterface(IID_IServiceProvider, (LPVOID*)&pServiceProvider)))
                {
                    IShellBrowser* pShellBrowser;
                    if (SUCCEEDED(pServiceProvider->QueryService(SID_SShellBrowser, IID_IShellBrowser, (LPVOID*)&pShellBrowser)))
                    {
                        if (pShellBrowser)
                            pShellBrowser->BrowseObject(NULL, (GetKeyState(VK_CONTROL) < 0 ? SBSP_NEWBROWSER : SBSP_SAMEBROWSER) | SBSP_DEFMODE | SBSP_PARENT);
                        pShellBrowser->Release();
                    }
                    pServiceProvider->Release();
                }
            }
        }
        else if (cmd.name.compare(L"Console") == 0)
        {
            StartCmd(cwd, L"", (GetKeyState(VK_LWIN) & 0x8000) != 0);
        }
        else if (cmd.name.compare(L"PowerShell") == 0)
        {
            StartPS(cwd, L"", (GetKeyState(VK_LWIN) & 0x8000) != 0);
        }
        else if (cmd.name.compare(L"Copy Names") == 0)
        {
            std::wstring str = GetFileNames(items, L"\r\n", true, true, true);
            if (str.empty())
            {
                // Seems no items are selected
                // Use the view path instead
                size_t pos = cwd.find_last_of('\\');
                WCHAR  buf[MAX_PATH];
                if (pos != std::wstring::npos)
                {
                    wcscpy_s(buf, _countof(buf), cwd.substr(pos + 1).c_str());
                    PathQuoteSpaces(buf);
                    str = buf;
                }
            }
            WriteStringToClipboard(str, hWnd);
        }
        else if (cmd.name.compare(L"Copy Paths") == 0)
        {
            std::wstring str = GetFilePaths(items, L"\r\n", true, true, true, DWORD(m_regUseUNCPaths) ? true : false);
            if (str.empty())
            {
                // Seems no items are selected
                // Use the view path instead
                WCHAR buf[MAX_PATH];
                if (DWORD(m_regUseUNCPaths))
                {
                    str = ConvertToUNC(cwd);
                    wcscpy_s(buf, _countof(buf), str.c_str());
                }
                else
                    wcscpy_s(buf, _countof(buf), cwd.c_str());

                PathQuoteSpaces(buf);
                str = buf;
            }
            WriteStringToClipboard(str, hWnd);
        }
        else if (cmd.name.compare(L"New Folder") == 0)
        {
            CreateNewFolder();
        }
        else if (cmd.name.compare(L"Rename") == 0)
        {
            Rename(hWnd, items);
        }
        else if (cmd.name.compare(L"Move to subfolder") == 0)
        {
            MoveToSubfolder(hWnd, cwd, items);
        }
        else
            DebugBreak();
    }
    else
    {
        auto         editboxtext = GetEditBoxText();
        std::wstring consoletext = editboxtext.get();

        // replace "%selpaths" with the paths of the selected items
        std::wstring           tag(L"%selpaths");
        std::wstring           commandline = cmd.commandline;
        std::wstring::iterator it_begin    = search(commandline.begin(), commandline.end(), tag.begin(), tag.end());
        if (it_begin != commandline.end())
        {
            // prepare the selected paths
            std::wstring selpaths = GetFilePaths(items, L" ", true, true, true, false);
            if (selpaths.empty())
            {
                wchar_t buf[MAX_PATH] = {0};
                wcscpy_s(buf, _countof(buf), cwd.c_str());
                PathQuoteSpaces(buf);
                selpaths = buf;
            }
            std::wstring::iterator it_end = it_begin + tag.size();
            commandline.replace(it_begin, it_end, selpaths);
        }
        // replace "%sel*paths" with the paths of the selected items
        tag      = L"%sel*paths";
        it_begin = search(commandline.begin(), commandline.end(), tag.begin(), tag.end());
        if (it_begin != commandline.end())
        {
            // prepare the selected paths
            std::wstring selpaths = GetFilePaths(items, L"*", false, true, true, false);
            if (selpaths.empty())
            {
                wchar_t buf[MAX_PATH] = {0};
                wcscpy_s(buf, _countof(buf), cwd.c_str());
                PathQuoteSpaces(buf);
                selpaths = buf;
            }
            std::wstring::iterator it_end = it_begin + tag.size();
            commandline.replace(it_begin, it_end, selpaths);
        }
        // replace "%selnames" with the names of the selected items
        tag      = L"%selnames";
        it_begin = search(commandline.begin(), commandline.end(), tag.begin(), tag.end());
        if (it_begin != commandline.end())
        {
            // prepare the selected names
            std::wstring           selnames = GetFileNames(items, L" ", true, true, true);
            std::wstring::iterator it_end   = it_begin + tag.size();
            commandline.replace(it_begin, it_end, selnames);
        }
        // replace "%curdir" with the current directory
        tag      = L"%curdir";
        it_begin = search(commandline.begin(), commandline.end(), tag.begin(), tag.end());
        if (it_begin != commandline.end())
        {
            wchar_t buf[MAX_PATH] = {0};
            wcscpy_s(buf, _countof(buf), cwd.c_str());
            PathQuoteSpaces(buf);
            std::wstring cwdquoted = buf;

            std::wstring::iterator it_end = it_begin + tag.size();
            commandline.replace(it_begin, it_end, cwdquoted);
        }
        // replace "%cmdtext" with the text in the console edit box
        tag      = L"%cmdtext";
        it_begin = search(commandline.begin(), commandline.end(), tag.begin(), tag.end());
        if (it_begin != commandline.end())
        {
            std::wstring::iterator it_end = it_begin + tag.size();
            commandline.replace(it_begin, it_end, consoletext);
        }
        // replace "%selafile" with path to file containing all the selected files separated by newlines
        tag      = L"%selafile";
        it_begin = search(commandline.begin(), commandline.end(), tag.begin(), tag.end());
        if (it_begin != commandline.end())
        {
            std::wstring selpaths = GetFilePaths(items, L"\r\n", false, true, true, false);
            if (selpaths.empty())
            {
                wchar_t buf[MAX_PATH] = {0};
                wcscpy_s(buf, _countof(buf), cwd.c_str());
                PathQuoteSpaces(buf);
                selpaths = buf;
            }
            std::wstring           tempFilePath = WriteFileListToTempFile(false, selpaths);
            std::wstring::iterator it_end       = it_begin + tag.size();
            commandline.replace(it_begin, it_end, tempFilePath);
        }
        // replace "%selufile" with path to file containing all the selected files separated by newlines in utf-16 format
        tag      = L"%selufile";
        it_begin = search(commandline.begin(), commandline.end(), tag.begin(), tag.end());
        if (it_begin != commandline.end())
        {
            std::wstring selpaths = GetFilePaths(items, L"\r\n", false, true, true, false);
            if (selpaths.empty())
            {
                wchar_t buf[MAX_PATH] = {0};
                wcscpy_s(buf, _countof(buf), cwd.c_str());
                PathQuoteSpaces(buf);
                selpaths = buf;
            }
            std::wstring           tempFilePath = WriteFileListToTempFile(true, selpaths);
            std::wstring::iterator it_end       = it_begin + tag.size();
            commandline.replace(it_begin, it_end, tempFilePath);
        }
        StartApplication(!cmd.startin.empty() ? cmd.startin : cwd, commandline, (GetKeyState(VK_LWIN) & 0x8000) != 0);
    }
}

std::wstring CDeskBand::WriteFileListToTempFile(bool bUnicode, const std::wstring& paths)
{
    //write all selected files and paths to a temporary file
    //for TortoiseProc.exe to read out again.
    DWORD pathlength = GetTempPath(0, NULL);
    auto  path       = std::make_unique<wchar_t[]>(pathlength + 1);
    auto  tempFile   = std::make_unique<wchar_t[]>(pathlength + 100);
    GetTempPath(pathlength + 1, path.get());
    GetTempFileName(path.get(), L"stx", 0, tempFile.get());
    std::wstring retFilePath = std::wstring(tempFile.get());

    HANDLE file = ::CreateFile(tempFile.get(),
                               GENERIC_WRITE,
                               FILE_SHARE_READ,
                               0,
                               CREATE_ALWAYS,
                               FILE_ATTRIBUTE_TEMPORARY,
                               0);

    if (file == INVALID_HANDLE_VALUE)
        return {};

    DWORD written = 0;
    if (bUnicode)
    {
        ::WriteFile(file, paths.c_str(), (DWORD)paths.size() * sizeof(wchar_t), &written, 0);
    }
    else
    {
        std::string p = WideToMultibyte(paths);
        ::WriteFile(file, p.c_str(), (DWORD)p.size() * sizeof(char), &written, 0);
    }

    ::CloseHandle(file);
    return retFilePath;
}

LRESULT CDeskBand::OnSize(LPARAM /*lParam*/)
{
    RECT rc;
    ::GetClientRect(m_hWnd, &rc);

    if (m_bCmdEditEnabled)
    {
        HDWP hdwp = BeginDeferWindowPos(2);
        DeferWindowPos(hdwp, m_hWndToolbar, NULL, 0, 0, m_tbSize.cx, m_tbSize.cy, SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);
        DeferWindowPos(hdwp, m_hWndEdit, NULL, m_tbSize.cx + SPACEBETWEENEDITANDBUTTON, 0, rc.right - rc.left - m_tbSize.cx - SPACEBETWEENEDITANDBUTTON, m_tbSize.cy, SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);
        EndDeferWindowPos(hdwp);
        ShowWindow(m_hWndEdit, SW_SHOW);
    }
    else
    {
        SetWindowPos(m_hWndToolbar, NULL, 0, 0, m_tbSize.cx, m_tbSize.cy, SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);
        ShowWindow(m_hWndEdit, SW_HIDE);
    }
    return 0;
}

LRESULT CDeskBand::OnMove(LPARAM /*lParam*/)
{
    RECT rc;
    ::GetClientRect(m_hWnd, &rc);

    if (m_bCmdEditEnabled)
    {
        HDWP hdwp = BeginDeferWindowPos(2);
        DeferWindowPos(hdwp, m_hWndToolbar, NULL, 0, 0, m_tbSize.cx, m_tbSize.cy, SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);
        DeferWindowPos(hdwp, m_hWndEdit, NULL, m_tbSize.cx, 0, rc.right - rc.left - m_tbSize.cx - SPACEBETWEENEDITANDBUTTON, m_tbSize.cy, SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);
        EndDeferWindowPos(hdwp);
        ShowWindow(m_hWndEdit, SW_SHOW);
    }
    else
    {
        SetWindowPos(m_hWndToolbar, NULL, 0, 0, m_tbSize.cx, m_tbSize.cy, SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);
        ShowWindow(m_hWndEdit, SW_HIDE);
    }
    return 0;
}

void CDeskBand::SetTheme()
{
    auto parent = ::GetParent(::GetParent(::GetParent(m_hwndParent)));
    m_bDark     = (m_bCanHaveDarkMode && m_pIsDarkModeAllowedForWindow) ? m_pIsDarkModeAllowedForWindow(parent) : false;
    if (m_pIsDarkModeAllowedForApp)
        m_bDark = m_bDark && m_pIsDarkModeAllowedForApp();
    //if (m_pShouldSystemUseDarkMode)
    //    m_bDark = m_bDark && m_pShouldSystemUseDarkMode();
    if (m_pShouldAppsUseDarkMode)
    {
        auto usedark = (m_pShouldAppsUseDarkMode() & 0x01) != 0;
        m_bDark      = m_bDark && usedark;
    }
    HIGHCONTRAST info = {0};
    info.cbSize       = sizeof(HIGHCONTRAST);
    if (SystemParametersInfoW(SPI_GETHIGHCONTRAST, 0, &info, 0))
    {
        // no dark mode in high-contrast mode
        if (info.dwFlags & HCF_HIGHCONTRASTON)
        {
            m_bDark = false;
        }
    }
    if (m_bCanHaveDarkMode)
    {
        // first set the AllowDarkModeForWindow() to true/false depending on mode
        // without this, the button texts stays black in dark mode
        EnumChildWindows(m_hWnd, DarkChildProc, (LPARAM)this);
        if (m_bDark)
        {
            // set the themes for the controls:
            // the edit box needs SearchBoxEditComposited to draw
            // correctly on the dark background,
            // and the toolbar and rebar need to have their default
            // style removed so they don't draw in "explorer" style
            SetWindowTheme(m_hWndEdit, L"SearchBoxEditComposited", nullptr);
            SetWindowTheme(m_hWndToolbar, nullptr, nullptr);
            SetWindowTheme(m_hwndParent, nullptr, nullptr);
        }
        else
        {
            SetWindowTheme(m_hWndEdit, L"Explorer", nullptr);
            SetWindowTheme(m_hWndToolbar, L"Explorer", nullptr);
            SetWindowTheme(m_hwndParent, L"Explorer", nullptr);
        }
    }
}

void CDeskBand::FocusChange(BOOL bFocus)
{
    m_bFocus = bFocus;

    // inform the input object site that the focus has changed
    if (m_pSite)
    {
        m_pSite->OnFocusChangeIS((IDockingWindow*)this, bFocus);
    }
}

LRESULT CDeskBand::OnSetFocus(void)
{
    FocusChange(TRUE);
    ::SetFocus(m_hWndEdit);
    return 0;
}

LRESULT CDeskBand::OnKillFocus(void)
{
    FocusChange(FALSE);

    return 0;
}
BOOL CALLBACK CDeskBand::DarkChildProc(HWND hwnd, LPARAM lParam)
{
    CDeskBand* pThis = (CDeskBand*)lParam;
    if (pThis == nullptr)
        return FALSE;

    if (pThis->m_pAllowDarkModeForWindow)
    {
        pThis->m_pAllowDarkModeForWindow(hwnd, pThis->m_bDark);
    }
    return TRUE;
}
BOOL CDeskBand::RegisterAndCreateWindow(void)
{
    // If the window doesn't exist yet, create it now.
    if (!m_hWnd)
    {
        // Can't create a child window without a parent.
        if (!m_hwndParent)
        {
            return FALSE;
        }

        // If the window class has not been registered, then do so.
        WNDCLASSEX wc;
        if (!GetClassInfoEx(g_hInst, DB_CLASS_NAME, &wc))
        {
            SecureZeroMemory(&wc, sizeof(wc));
            wc.cbSize        = sizeof(WNDCLASSEX);
            wc.style         = CS_HREDRAW | CS_VREDRAW;
            wc.lpfnWndProc   = (WNDPROC)WndProc;
            wc.cbClsExtra    = 0;
            wc.cbWndExtra    = 0;
            wc.hInstance     = g_hInst;
            wc.hIcon         = NULL;
            wc.hIconSm       = NULL;
            wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
            wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
            wc.lpszMenuName  = NULL;
            wc.lpszClassName = DB_CLASS_NAME;

            if (!RegisterClassEx(&wc))
            {
                return FALSE;
            }
        }

        RECT rc;

        GetClientRect(m_hwndParent, &rc);

        // subclass the parent deskbar control to intercept the RBN_CHEVRONPUSHED messages
        SetWindowSubclass(::GetParent(m_hwndParent), DeskBandProc, (UINT_PTR)this, (DWORD_PTR)this);

        //Create the window. The WndProc will set m_hWnd.
        CreateWindowEx(WS_EX_CONTROLPARENT,
                       DB_CLASS_NAME,
                       NULL,
                       WS_CHILD | WS_CLIPSIBLINGS,
                       rc.left,
                       rc.top,
                       rc.right - rc.left,
                       rc.bottom - rc.top,
                       m_hwndParent,
                       NULL,
                       NULL,
                       (LPVOID)this);

        GetClientRect(m_hWnd, &rc);

        // create an edit control
        m_hWndEdit = CreateWindowEx(0,
                                    L"EDIT",
                                    NULL,
                                    WS_VISIBLE | WS_TABSTOP | WS_CHILD | WS_CLIPSIBLINGS | WS_BORDER | ES_LEFT | ES_AUTOHSCROLL,
                                    rc.left,
                                    rc.top,
                                    rc.right - rc.left - CDPIAware::Instance().Scale(m_hWnd, EDITBOXSIZEX) - SPACEBETWEENEDITANDBUTTON,
                                    rc.bottom - rc.top,
                                    m_hWnd,
                                    NULL,
                                    NULL,
                                    NULL);

        if (m_hWndEdit == NULL)
            return FALSE;

        // set the font for the edit control
        HGDIOBJ hFont = GetStockObject(DEFAULT_GUI_FONT);
        SendMessage(m_hWndEdit, WM_SETFONT, (WPARAM)hFont, 0);
        // subclass the edit control to intercept the WM_SETFOCUS messages
        SetWindowSubclass(m_hWndEdit, EditProc, (UINT_PTR)this, (DWORD_PTR)this);

        // create a toolbar which will hold our button
        m_hWndToolbar = CreateWindowEx(TBSTYLE_EX_MIXEDBUTTONS | TBSTYLE_EX_HIDECLIPPEDBUTTONS,
                                       TOOLBARCLASSNAME,
                                       NULL,
                                       WS_CHILD | TBSTYLE_TOOLTIPS | TBSTYLE_WRAPABLE | TBSTYLE_LIST | TBSTYLE_FLAT | TBSTYLE_TRANSPARENT | CCS_NORESIZE | CCS_NODIVIDER | CCS_NOPARENTALIGN,
                                       rc.right - CDPIAware::Instance().Scale(m_hWnd, EDITBOXSIZEX),
                                       rc.top,
                                       CDPIAware::Instance().Scale(m_hWnd, EDITBOXSIZEX),
                                       rc.bottom - rc.top,
                                       m_hWnd,
                                       NULL,
                                       g_hInst,
                                       NULL);

        // Send the TB_BUTTONSTRUCTSIZE message, which is required for
        // backward compatibility.
        SendMessage(m_hWndToolbar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);

        ShowWindow(m_hWndToolbar, SW_SHOW);
    }
    return (NULL != m_hWnd);
}

LRESULT CALLBACK CDeskBand::KeyboardHookProc(int code, WPARAM wParam, LPARAM lParam)
{
    DWORD                                 threadID = GetCurrentThreadId();
    std::map<DWORD, CDeskBand*>::iterator it       = m_desklist.find(threadID);
    if (it != m_desklist.end())
    {
        if ((!it->second->m_bDialogShown) && (code >= 0) && ((lParam & 0xc0000000) == 0)) //key went from 'up' to 'down' state
        {
            hotkey realhk;
            realhk.keycode                     = wParam;
            realhk.alt                         = !!(GetKeyState(VK_MENU) & 0x8000);
            realhk.control                     = !!(GetKeyState(VK_CONTROL) & 0x8000);
            realhk.shift                       = !!(GetKeyState(VK_SHIFT) & 0x8000);
            std::map<hotkey, int>::iterator hk = it->second->m_hotkeys.find(realhk);
            if (hk != it->second->m_hotkeys.end())
            {
                // special handling of command 0: just set the focus!
                if ((hk->second == 0) && (it->second->m_pSite))
                {
                    if (it->second->m_bCmdEditEnabled)
                        it->second->OnSetFocus();
                    return 1; //we processed it
                }
                it->second->OnCommand(MAKEWORD(hk->second, BN_CLICKED), 0);
                return 1; // we processed it
            }
        }
        return CallNextHookEx(it->second->m_hook, code, wParam, lParam);
    }
    return 0;
}

BOOL CDeskBand::BuildToolbarButtons()
{
    m_hotkeys.clear();
    m_enablestates.clear();
    m_tooltips.clear();
    m_regHideEditBox.read();
    m_bCmdEditEnabled = !DWORD(m_regHideEditBox);

    if (m_hWndToolbar == NULL)
        return FALSE;

    // first remove all existing buttons to start from scratch
    LRESULT buttoncount = ::SendMessage(m_hWndToolbar, TB_BUTTONCOUNT, 0, 0);
    for (int i = 0; i < buttoncount; ++i)
    {
        ::SendMessage(m_hWndToolbar, TB_DELETEBUTTON, 0, 0);
    }
    if (m_hToolbarImgList)
    {
        // destroy the image list
        ImageList_Destroy(m_hToolbarImgList);
    }

    m_commands.LoadFromFile();

    auto tb = std::make_unique<TBBUTTON[]>(m_commands.GetCount());
    // create an image list containing the icons for the toolbar
    m_hToolbarImgList = ImageList_Create(int(16 * CDPIAware::Instance().ScaleFactor(m_hWnd)), int(16 * CDPIAware::Instance().ScaleFactor(m_hWnd)), ILC_COLOR32 | ILC_MASK, m_commands.GetCount(), 1);
    if (m_hToolbarImgList == NULL)
    {
        return false;
    }
    BYTE fsStyle = BTNS_BUTTON;
    if (DWORD(m_regShowBtnText))
        fsStyle |= BTNS_SHOWTEXT;

    int index = 0;
    for (int j = 0; j < m_commands.GetCount(); ++j)
    {
        Command cmd        = m_commands.GetCommand(j);
        m_hotkeys[cmd.key] = j;
        if ((cmd.commandline.compare(INTERNALCOMMANDHIDDEN) == 0) && (cmd.name.compare(L"Options") == 0))
        {
            cmd.commandline = INTERNALCOMMAND; // make sure the options button is never hidden.
            m_commands.SetCommand(j, cmd);
        }
        if ((cmd.name.compare(L"StexBar Internal Edit Box") == 0) ||
            (cmd.commandline.compare(INTERNALCOMMANDHIDDEN) == 0))
        {
            continue;
        }
        DWORD es = 0;
        es |= cmd.enabled_viewpath ? ENABLED_VIEWPATH : 0;
        es |= cmd.enabled_noviewpath ? ENABLED_NOVIEWPATH : 0;
        es |= cmd.enabled_fileselected ? ENABLED_FILESELECTED : 0;
        es |= cmd.enabled_folderselected ? ENABLED_FOLDERSELECTED : 0;
        es |= cmd.enabled_selected ? ENABLED_SELECTED : 0;
        es |= cmd.enabled_noselection ? ENABLED_NOSELECTION : 0;
        es |= cmd.enabled_fileselected ? ENABLED_FILESELECTED : 0;
        es |= (cmd.enabled_selectedcount << 16);
        m_enablestates[j] = es;

        HICON hIcon = LoadCommandIcon(cmd);
        if (hIcon)
        {
            tb[index].iBitmap = ImageList_AddIcon(m_hToolbarImgList, hIcon);
            DestroyIcon(hIcon);
        }
        else
            tb[index].iBitmap = NULL;
        tb[index].idCommand = j;
        tb[index].fsState   = cmd.separator ? 0 : TBSTATE_ENABLED;
        tb[index].fsStyle   = cmd.separator ? BTNS_SEP : fsStyle;
        tb[index].iString   = cmd.separator ? NULL : (INT_PTR)m_commands.GetCommandPtr(j)->name.c_str();
        tb[index].dwData    = NULL;
        if (!cmd.separator)
        {
            std::wstring sTip = cmd.name;
            if (cmd.key.keycode)
            {
                sTip += L" (";
                if (cmd.key.control)
                    sTip += L"Ctrl+";
                if (cmd.key.shift)
                    sTip += L"Shift+";
                if (cmd.key.alt)
                    sTip += L"Alt+";
                LONG  scanCode = MapVirtualKey((UINT)cmd.key.keycode, MAPVK_VK_TO_VSC);
                WCHAR buf[50];
                GetKeyNameText(((LONG)(scanCode) << 16), buf, _countof(buf));
                sTip += buf;
                sTip += L")";
            }
            m_tooltips[tb[index].idCommand] = sTip;
        }
        index++;
    }

    SendMessage(m_hWndToolbar, TB_SETIMAGELIST, 0, (LPARAM)m_hToolbarImgList);
    SendMessage(m_hWndToolbar, TB_ADDBUTTONS, index, (LPARAM)tb.get());
    SendMessage(m_hWndToolbar, TB_SETEXTENDEDSTYLE, 0, (LPARAM)TBSTYLE_EX_MIXEDBUTTONS | TBSTYLE_EX_HIDECLIPPEDBUTTONS);
    SendMessage(m_hWndToolbar, TB_AUTOSIZE, 0, 0);
    SendMessage(m_hWndToolbar, TB_GETMAXSIZE, 0, (LPARAM)&m_tbSize);

    // now inform our parent that the size of the deskband has changed
    if (m_pSite)
    {
        IOleCommandTarget* pOleCommandTarget;
        if (SUCCEEDED(m_pSite->QueryInterface(IID_IOleCommandTarget, (LPVOID*)&pOleCommandTarget)))
        {
            pOleCommandTarget->Exec(&CGID_DeskBand, DBID_BANDINFOCHANGED, OLECMDEXECOPT_DONTPROMPTUSER, NULL, NULL);
            pOleCommandTarget->Release();
        }
    }
    return TRUE;
}

HICON CDeskBand::LoadCommandIcon(const Command& cmd)
{
    HICON hIcon = NULL;
    if (cmd.nIconID)
        hIcon = (HICON)LoadImage(g_hInst, MAKEINTRESOURCE(cmd.nIconID), IMAGE_ICON, int(16 * CDPIAware::Instance().ScaleFactor(m_hWnd)), int(16 * CDPIAware::Instance().ScaleFactor(m_hWnd)), LR_DEFAULTCOLOR);
    else if (!cmd.separator)
    {
        hIcon = (HICON)LoadImage(g_hInst, cmd.icon.c_str(), IMAGE_ICON, int(16 * CDPIAware::Instance().ScaleFactor(m_hWnd)), int(16 * CDPIAware::Instance().ScaleFactor(m_hWnd)), LR_DEFAULTCOLOR);
        if (hIcon == NULL)
            hIcon = (HICON)LoadImage(g_hInst, cmd.icon.c_str(), IMAGE_ICON, int(16 * CDPIAware::Instance().ScaleFactor(m_hWnd)), int(16 * CDPIAware::Instance().ScaleFactor(m_hWnd)), LR_LOADFROMFILE);
        if (hIcon == NULL)
        {
            // icon loading failed. Let's try to load it differently:
            // the user might have specified a module path and an icon index
            // like this: c:\windows\explorer.exe,3 (the icon with ID 3 in explorer.exe)
            hIcon = NULL;
            if (cmd.icon.find(',') != std::wstring::npos)
            {
                size_t       pos = cmd.icon.find_last_of(',');
                std::wstring resourcefile, iconid;
                if (pos != std::wstring::npos)
                {
                    resourcefile = cmd.icon.substr(0, pos);
                    iconid       = cmd.icon.substr(pos + 1);
                    hIcon        = ExtractIcon(g_hInst, resourcefile.c_str(), _wtoi(iconid.c_str()));
                }
            }
            if (hIcon == NULL)
            {
                // loading the icon with an index didn't work either
                // next we try to use the icon of the application defined in the commandline
                std::wstring appname;
                if (cmd.commandline.find(' ') != std::wstring::npos)
                    appname = cmd.commandline.substr(0, cmd.commandline.find(' '));
                else
                    appname = cmd.commandline;
                if (appname.compare(L"INTERNALCOMMAND") == 0)
                {
                    if (cmd.name.compare(L"PowerShell") == 0)
                        appname = L"powershell.exe";
                    else if (cmd.name.compare(L"Console") == 0)
                        appname = L"cmd.exe";
                }
                hIcon = ExtractIcon(g_hInst, appname.c_str(), 0);
            }
            if (hIcon == NULL)
            {
                hIcon = ExtractIcon(g_hInst, cmd.icon.c_str(), 0);
                if (hIcon == NULL)
                {
                    // if the icon handle is still invalid (no icon found yet),
                    // we use a default icon
                    hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_DEFAULT));
                }
            }
        }
    }
    return hIcon;
}
HWND CDeskBand::GetListView32(IShellView* shellView)
{
    HWND parent = NULL;
    if (SUCCEEDED(shellView->GetWindow(&parent)))
    {
        EnumChildWindows(parent, EnumChildProc, (LPARAM)this);
        return m_hwndListView;
    }
    return NULL;
}

BOOL CDeskBand::EnumChildProc(HWND hwnd, LPARAM lParam)
{
    wchar_t    cName[100];
    CDeskBand* pTHIS = (CDeskBand*)lParam;
    if (GetClassName(hwnd, cName, 100))
    {
        if (wcscmp(cName, L"SysListView32") == 0)
        {
            pTHIS->m_hwndListView = hwnd;
            return FALSE;
        }
        if (wcscmp(cName, L"DirectUIHWND") == 0)
        {
            pTHIS->m_hwndListView = hwnd;
            return FALSE;
        }
    }
    pTHIS->m_hwndListView = NULL;
    return TRUE;
}

DWORD CDeskBand::GetEditBoxUsage()
{
    if (DWORD(m_regEditBoxUsage) == IDC_USEAUTO)
    {
        auto buf = GetEditBoxText(false);
        if (wcslen(buf.get()) > 1)
        {
            switch (buf[0])
            {
                case 'G':
                case 'g':
                    return IDC_USEGREPWIN;
                case 'C':
                case 'c':
                    return IDC_USECONSOLE;
                case 'P':
                case 'p':
                    return IDC_USEPOWERSHELL;
                case 'F':
                case 'f':
                    return IDC_USEFILTER;
                default:
                    return IDC_USEFILTER;
            }
        }
    }
    return DWORD(m_regEditBoxUsage);
}

std::unique_ptr<wchar_t[]> CDeskBand::GetEditBoxText(bool sanitized /* = true */)
{
    // get the command entered in the edit box
    int  count = MAX_PATH;
    auto buf   = std::make_unique<wchar_t[]>(count + 1);
    while (::GetWindowText(m_hWndEdit, buf.get(), count) >= count)
    {
        count += MAX_PATH;
        buf = std::make_unique<wchar_t[]>(count + 1);
    }

    if ((sanitized) && (DWORD(m_regEditBoxUsage) == IDC_USEAUTO))
    {
        // remove the command-switch char and whitespaces after it
        if (buf[0] != '\0')
        {
            switch (buf[0])
            {
                case 'G':
                case 'g':
                case 'C':
                case 'c':
                case 'P':
                case 'p':
                case 'F':
                case 'f':
                {
                    int i = 1;
                    while (buf[i] && (buf[i] == ' '))
                        ++i;
                    auto sanitizedbuf = std::make_unique<wchar_t[]>(count + 1);
                    wcscpy_s(sanitizedbuf.get(), count + 1, &buf[i]);
                    buf = std::move(sanitizedbuf);
                }
                break;
            }
        }
    }
    return buf;
}

void CDeskBand::MoveToSubfolder(HWND hWnd, std::wstring cwd, const std::map<std::wstring, ULONG>& items)
{
    if (cwd.empty() || !PathFileExists(cwd.c_str()))
    {
        // use the parent dir of the first item
        if (items.empty())
            return;
        cwd = CPathUtils::GetParentDirectory(items.begin()->first);
    }

    // try to find a default name for the new folder from the selected files
    std::wstring foldername;
    for (const auto& file : items)
    {
        std::wstring name = CPathUtils::GetFileNameWithoutExtension(file.first);
        if (foldername.empty())
            foldername = name;
        else
        {
            size_t neqpos          = 0;
            size_t shortnamelength = min(foldername.size(), name.size());
            for (size_t i = 0; i < shortnamelength; ++i)
            {
                if (foldername[i] == name[i])
                    neqpos = i;
                else if (::towlower(foldername[i]) == ::towlower(name[i]))
                    neqpos = i;
                else
                    break;
            }
            if (neqpos > 3)
                foldername = name.substr(0, neqpos + 1);
            else
            {
                foldername.clear();
                break;
            }
        }
    }
    foldername.erase(std::find_if(foldername.rbegin(), foldername.rend(), [](int ch) {
                         return ch != '\n' || ch != '\r' || ch != ' ' || ch != '\t' || ch != '-' || ch != '_' || ch != '.' || ch != ';';
                     }).base(),
                     foldername.end());

    if (foldername.empty())
        foldername = L"folder";

    CNameDlg dlg(hWnd);
    dlg.Name = foldername;
    if (dlg.DoModal(g_hInst, IDD_NAMEDLG, hWnd) == IDOK)
    {
        foldername = dlg.Name;
        CStringUtils::trim(foldername);
        std::wstring folderpath = cwd + L"\\" + foldername;
        int          retrycount = 1;
        while (PathFileExists(folderpath.c_str()))
        {
            folderpath = CStringUtils::Format(L"%s\\%s (%d)", cwd.c_str(), foldername.c_str(), retrycount);
            ++retrycount;
        }
        CreateDirectory(folderpath.c_str(), NULL);
        for (const auto& file : items)
        {
            std::wstring dest = folderpath + L"\\" + CPathUtils::GetFileName(file.first);
            MoveFile(file.first.c_str(), dest.c_str());
        }
    }
}
