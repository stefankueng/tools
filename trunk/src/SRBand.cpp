#include "stdafx.h"
#include "SRBand.h"
#include "Guid.h"
#include <strsafe.h>
#include <algorithm>
#include <vector>
#include "resource.h"
#include "SimpleIni.h"
#include "uxtheme.h"

#pragma comment(lib, "uxtheme.lib")


map<DWORD, CDeskBand*> CDeskBand::m_desklist;	///< set of CDeskBand objects which use the keyboard hook

CDeskBand::CDeskBand() : m_bFocus(false)
	, m_hwndParent(NULL)
	, m_hWnd(NULL)
	, m_hWndEdit(NULL)
	, m_dwViewMode(0)
	, m_dwBandID(0)
	, m_oldEditWndProc(NULL)
	, m_pSite(NULL)
	, m_regShowBtnText(_T("Software\\StefansTools\\StExBar\\ShowButtonText"), 1)
	, m_bCmdEditEnabled(true)
{
	m_ObjRefCount = 1;
	g_DllRefCount++;

	m_tbSize.cx = 0;
	m_tbSize.cy = 0;

	INITCOMMONCONTROLSEX used = {
		sizeof(INITCOMMONCONTROLSEX),
		ICC_STANDARD_CLASSES | ICC_BAR_CLASSES | ICC_COOL_CLASSES
	};
	InitCommonControlsEx(&used);
}

CDeskBand::~CDeskBand()
{
	// This should have been freed in a call to SetSite(NULL), but 
	// it is defined here to be safe.
	if (m_pSite)
	{
		UnhookWindowsHookEx(m_hook);
		m_pSite->Release();
		m_pSite = NULL;
		map<DWORD, CDeskBand*>::iterator it = m_desklist.find(GetCurrentThreadId());
		if (it != m_desklist.end())
			m_desklist.erase(it);
	}

	g_DllRefCount--;
}

//////////////////////////////////////////////////////////////////////////
// IUnknown methods
//////////////////////////////////////////////////////////////////////////
STDMETHODIMP CDeskBand::QueryInterface(REFIID riid, LPVOID *ppReturn)
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

	if (*ppReturn)
	{
		(*(LPUNKNOWN*)ppReturn)->AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}                                             

STDMETHODIMP_(DWORD) CDeskBand::AddRef()
{
	return ++m_ObjRefCount;
}


STDMETHODIMP_(DWORD) CDeskBand::Release()
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
STDMETHODIMP CDeskBand::GetWindow(HWND *phWnd)
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
			::SetTimer(m_hWnd, TID_IDLE, 100, NULL);
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
		UnhookWindowsHookEx(m_hook);
		m_pSite->Release();
		m_pSite = NULL;
		map<DWORD, CDeskBand*>::iterator it = m_desklist.find(GetCurrentThreadId());
		if (it != m_desklist.end())
			m_desklist.erase(it);
	}

	m_tbSize.cx = 0;
	m_tbSize.cy = 0;

	// If punkSite is not NULL, a new site is being set.
	if (punkSite)
	{
		// Get the parent window.
		IOleWindow  *pOleWindow;

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

		m_hook = SetWindowsHookEx(WH_KEYBOARD, KeyboardHookProc, NULL, GetCurrentThreadId());
		m_desklist[GetCurrentThreadId()] = this;

		// Get and keep the IInputObjectSite pointer.
		if (SUCCEEDED(punkSite->QueryInterface(IID_IInputObjectSite, 
			(LPVOID*)&m_pSite)))
		{
			return S_OK;
		}

		return E_FAIL;
	}

	return S_OK;
}

STDMETHODIMP CDeskBand::GetSite(REFIID riid, LPVOID *ppvReturn)
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
		m_dwBandID = dwBandID;
		m_dwViewMode = dwViewMode;

		if (pdbi->dwMask & DBIM_MINSIZE)
		{
			if (DBIF_VIEWMODE_FLOATING & dwViewMode)
			{
				pdbi->ptMinSize.x = m_tbSize.cx + (m_bCmdEditEnabled ? EDITBOXSIZEX : 0);
				pdbi->ptMinSize.y = m_tbSize.cy;
			}
			else
			{
				pdbi->ptMinSize.x = m_tbSize.cx + (m_bCmdEditEnabled ? EDITBOXSIZEX : 0);
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
			pdbi->ptActual.x = m_tbSize.cx + (m_bCmdEditEnabled ? EDITBOXSIZEX : 0);
			pdbi->ptActual.y = m_tbSize.cy;
		}

		if (pdbi->dwMask & DBIM_TITLE)
		{
			StringCchCopy(pdbi->wszTitle, 256, L"StEx");
		}

		if (pdbi->dwMask & DBIM_MODEFLAGS)
		{
			pdbi->dwModeFlags = DBIMF_NORMAL;

			pdbi->dwModeFlags |= DBIMF_VARIABLEHEIGHT;
		}

		if (pdbi->dwMask & DBIM_BKCOLOR)
		{
			// Use the default background color by removing this flag.
			pdbi->dwMask &= ~DBIM_BKCOLOR;
		}

		return S_OK;
	}

	return E_INVALIDARG;
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

STDMETHODIMP CDeskBand::GetSizeMax(ULARGE_INTEGER * /*pul*/)
{
	return E_NOTIMPL;
}

//////////////////////////////////////////////////////////////////////////
// helper functions for our own DeskBand
//////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK CDeskBand::WndProc(HWND hWnd, 
									UINT uMessage, 
									WPARAM wParam, 
									LPARAM lParam)
{
	CDeskBand *pThis = (CDeskBand*)GetWindowLongPtr(hWnd, GWL_USERDATA);

	switch (uMessage)
	{
	case WM_NCCREATE:
		{
			LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
			pThis = (CDeskBand*)(lpcs->lpCreateParams);
			SetWindowLong(hWnd, GWL_USERDATA, (LONG)pThis);

			//set the window handle
			pThis->m_hWnd = hWnd;
		}
		break;

	case WM_TIMER:
		if (wParam == TID_IDLE)
		{
			pThis->FindPaths();
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
			if (pThis->m_selectedItems.size() == 0)
				state |= ENABLED_NOSELECTION;
			for (map<int, DWORD>::iterator it = pThis->m_enablestates.begin(); it != pThis->m_enablestates.end(); ++it)
			{
				if (((it->second & 0xFFFF)&state)&&
					((HIWORD(it->second) == 0)||((pThis->m_selectedItems.size() == 0)&&(it->second & ENABLED_NOSELECTION))||(HIWORD(it->second) == pThis->m_selectedItems.size())))
					::SendMessage(pThis->m_hWndToolbar, TB_ENABLEBUTTON, it->first, (LPARAM)TRUE);
				else
					::SendMessage(pThis->m_hWndToolbar, TB_ENABLEBUTTON, it->first, (LPARAM)FALSE);
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

	case WM_ERASEBKGND:
		{
			HDC hDC = (HDC)wParam;
			RECT rc;
			::GetClientRect(pThis->m_hWnd, &rc);
			// only draw the themed background if themes are enabled
			if (IsThemeActive())
			{
				HTHEME hTheme = OpenThemeData(pThis->m_hWnd, L"Rebar");
				if (hTheme)
				{
					// now draw the themed background of a rebar control, because
					// that's what we're actually in and should look like.
					DrawThemeBackground(hTheme, hDC, 0, 0, &rc, NULL);
					return TRUE;	// we've drawn the background
				}
			}
			// just do nothing so the system knows that we haven't erased the background
		}

	}

	return DefWindowProc(hWnd, uMessage, wParam, lParam);
}

LRESULT CALLBACK CDeskBand::EditProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	CDeskBand *pThis = (CDeskBand*)GetWindowLongPtr(hWnd, GWL_USERDATA);
	if (uMessage == WM_SETFOCUS)
	{
		pThis->OnSetFocus();
	}
	return CallWindowProc(pThis->m_oldEditWndProc, hWnd, uMessage, wParam, lParam);
}

LRESULT CDeskBand::OnCommand(WPARAM wParam, LPARAM /*lParam*/)
{
	switch (HIWORD(wParam))
	{
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
			if (m_selectedItems.size() == 0)
				state |= ENABLED_NOSELECTION;
			map<int, DWORD>::iterator it = m_enablestates.find(LOWORD(wParam));
			bool bEnabled = false;
			if (it != m_enablestates.end())
			{
				if (((it->second & 0xFFFF)&state)&&
					((HIWORD(it->second) == 0)||((m_selectedItems.size() == 0)&&(it->second & ENABLED_NOSELECTION))||(HIWORD(it->second) == m_selectedItems.size())))
					bEnabled = true;
			}
			if (!bEnabled)
				return 0;

			FindPaths();
			switch(LOWORD(wParam))
			{
			case 0:		// edit control enter pressed
				{
					// get the command entered in the edit box
					int count = MAX_PATH;
					TCHAR * buf = new TCHAR[count+1];
					while (::GetWindowText(m_hWndEdit, buf, count)>=count)
					{
						delete [] buf;
						count += MAX_PATH;
						buf = new TCHAR[count+1];
					}
					// when we start the console with the command the user
					// has entered in the edit box, we want the console
					// to execute the command immediately, and *not* quit after
					// executing the command so the user can see the output.
					// If however the user enters a '@' char in front of the command
					// then the console shall quit after executing the command.
					wstring params;
					if (buf[0] == '@')
						params = _T("/c ");
					else				
						params = _T("/k ");
					params += buf;
					StartCmd(params);
					delete [] buf;
				}
				break;
			case 1:		// options
				if (DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_OPTIONS), m_hWnd, OptionsDlgFunc, (LPARAM)this)==IDOK)
				{
					BuildToolbarButtons();
					OnMove(0);
				}
				break;
			case 2:		// cmd
				StartCmd(_T(""));
				break;
			case 3:		// copy name
				{
					wstring str = GetFileNames(_T("\r\n"), false, true, true);
					WriteStringToClipboard(str, m_hWnd);
				}
				break;
			case 4:		// copy path
				{
					wstring str = GetFilePaths(_T("\r\n"), false, true, true);
					WriteStringToClipboard(str, m_hWnd);
				}
				break;
			case 5:		// New Folder
				{
					CreateNewFolder();
				}
				break;
			default:	// custom commands
				{
					map<WORD, wstring>::iterator cl = m_commands.find(LOWORD(wParam));
					if (cl != m_commands.end())
					{
						// now it->second is the command line string for the command
						wstring commandline = cl->second;

						// replace "%selpaths" with the paths of the selected items
						wstring tag(_T("%selpaths"));
						wstring::iterator it_begin = search(commandline.begin(), commandline.end(), tag.begin(), tag.end());
						if (it_begin != commandline.end())
						{
							// prepare the selected paths
							wstring selpaths = GetFilePaths(_T(" "), true, true, true);
							wstring::iterator it_end= it_begin + tag.size();
							commandline.replace(it_begin, it_end, selpaths);
						}
						// replace "%selnames" with the names of the selected items
						tag = _T("%selnames");
						it_begin = search(commandline.begin(), commandline.end(), tag.begin(), tag.end());
						if (it_begin != commandline.end())
						{
							// prepare the selected names
							wstring selnames = GetFileNames(_T(" "), true, true, true);
							wstring::iterator it_end= it_begin + tag.size();
							commandline.replace(it_begin, it_end, selnames);
						}
						// replace "%curdir" with the current directory
						tag = _T("%curdir");
						it_begin = search(commandline.begin(), commandline.end(), tag.begin(), tag.end());
						if (it_begin != commandline.end())
						{
							wstring::iterator it_end= it_begin + tag.size();
							commandline.replace(it_begin, it_end, m_currentDirectory);
						}
						StartApplication(commandline);
					}
				}
				break;
			}
			FocusChange(false);
			break;
		}
	}
	return 0;
}

LRESULT CDeskBand::OnSize(LPARAM /*lParam*/)
{
	RECT rc;
	::GetClientRect(m_hWnd, &rc);

	HDWP hdwp = BeginDeferWindowPos(2);
	DeferWindowPos(hdwp, m_hWndToolbar, NULL, 0, 0, m_tbSize.cx, m_tbSize.cy, SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOZORDER);
	DeferWindowPos(hdwp, m_hWndEdit, NULL, m_tbSize.cx+SPACEBETWEENEDITANDBUTTON, 0, rc.right-rc.left-m_tbSize.cx-SPACEBETWEENEDITANDBUTTON, m_tbSize.cy, SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOZORDER);
	EndDeferWindowPos(hdwp);
	return 0;
}

LRESULT CDeskBand::OnMove(LPARAM /*lParam*/)
{
	RECT rc;
	::GetClientRect(m_hWnd, &rc);

	HDWP hdwp = BeginDeferWindowPos(2);
	DeferWindowPos(hdwp, m_hWndToolbar, NULL, 0, 0, m_tbSize.cx, m_tbSize.cy, SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOZORDER);
	DeferWindowPos(hdwp, m_hWndEdit, NULL, m_tbSize.cx+SPACEBETWEENEDITANDBUTTON, 0, rc.right-rc.left-m_tbSize.cx-SPACEBETWEENEDITANDBUTTON, m_tbSize.cy, SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOZORDER);
	EndDeferWindowPos(hdwp);
	return 0;
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
		WNDCLASS wc;
		if (!GetClassInfo(g_hInst, DB_CLASS_NAME, &wc))
		{
			ZeroMemory(&wc, sizeof(wc));
			wc.style          = CS_HREDRAW | CS_VREDRAW | CS_GLOBALCLASS;
			wc.lpfnWndProc    = (WNDPROC)WndProc;
			wc.cbClsExtra     = 0;
			wc.cbWndExtra     = 0;
			wc.hInstance      = g_hInst;
			wc.hIcon          = NULL;
			wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
			wc.hbrBackground  = (HBRUSH)(COLOR_BTNFACE+1);
			wc.lpszMenuName   = NULL;
			wc.lpszClassName  = DB_CLASS_NAME;

			if (!RegisterClass(&wc))
			{
				return FALSE;
			}
		}

		RECT  rc;

		GetClientRect(m_hwndParent, &rc);

		//Create the window. The WndProc will set m_hWnd.
		CreateWindowEx(0,
			DB_CLASS_NAME,
			NULL,
			WS_CHILD | WS_CLIPSIBLINGS,
			rc.left,
			rc.top,
			rc.right - rc.left,
			rc.bottom - rc.top,
			m_hwndParent,
			NULL,
			g_hInst,
			(LPVOID)this);

		GetClientRect(m_hWnd, &rc);

		// create an edit control
		m_hWndEdit = CreateWindowEx(0,
			L"EDIT",
			NULL,
			WS_VISIBLE | WS_TABSTOP | WS_CHILD | WS_CLIPSIBLINGS | WS_BORDER | ES_LEFT | ES_AUTOHSCROLL,
			rc.left,
			rc.top,
			rc.right - rc.left - EDITBOXSIZEX - SPACEBETWEENEDITANDBUTTON,
			rc.bottom - rc.top,
			m_hWnd,
			NULL,
			g_hInst,
			NULL);

		if (m_hWndEdit == NULL)
			return FALSE;

		// subclass the edit control to intercept the WM_SETFOCUS messages
		m_oldEditWndProc = (WNDPROC)SetWindowLongPtr(m_hWndEdit, GWL_WNDPROC, (LONG)EditProc);
		SetWindowLongPtr(m_hWndEdit, GWL_USERDATA, (LONG)this);

		// set the font for the edit control
		SendMessage(m_hWndEdit, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);

		// create a toolbar which will hold our button
		m_hWndToolbar = CreateWindowEx(TBSTYLE_EX_MIXEDBUTTONS,
			TOOLBARCLASSNAME, 
			NULL, 
			WS_CHILD|TBSTYLE_LIST|TBSTYLE_FLAT|TBSTYLE_TRANSPARENT|CCS_NORESIZE|CCS_NODIVIDER|CCS_NOPARENTALIGN, 
			rc.right - EDITBOXSIZEX,
			rc.top,
			EDITBOXSIZEX,
			rc.bottom - rc.top,
			m_hWnd, 
			NULL, 
			g_hInst, 
			NULL);

		// Send the TB_BUTTONSTRUCTSIZE message, which is required for 
		// backward compatibility. 
		SendMessage(m_hWndToolbar, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0); 
		
		ShowWindow(m_hWndToolbar, SW_SHOW); 
	}
	return (NULL != m_hWnd);
}

LRESULT CALLBACK CDeskBand::KeyboardHookProc(int code, WPARAM wParam, LPARAM lParam)
{
	DWORD threadID = GetCurrentThreadId();
	map<DWORD, CDeskBand*>::iterator it = m_desklist.find(threadID);
	if (it != m_desklist.end())
	{
		if ((code >= 0)&&((lParam & 0xc0000000) == 0))//key went from 'up' to 'down' state
		{
			map<WPARAM, hotkeymodifiers>::iterator hk = it->second->m_hotkeys.find(wParam);
			if (hk != it->second->m_hotkeys.end())
			{
				bool alt = !!(GetKeyState(VK_MENU)&0x8000);
				bool shift = !!(GetKeyState(VK_SHIFT)&0x8000);
				bool control = !!(GetKeyState(VK_CONTROL)&0x8000);
				if ((hk->second.alt == alt)&&
					(hk->second.shift == shift)&&
					(hk->second.control == control))
				{
					// special handling of command 0: just set the focus!
					if ((hk->second.command==0)&&(it->second->m_pSite))
					{
						if (it->second->m_bCmdEditEnabled)
							it->second->OnSetFocus();
						return 1;//we processed it
					}
					it->second->OnCommand(MAKEWORD(hk->second.command, BN_CLICKED), 0);
					return 1; // we processed it
				}
			}
		}
		return CallNextHookEx(it->second->m_hook, code, wParam, lParam);
	}
	return 0;
}

BOOL CDeskBand::BuildToolbarButtons()
{
	m_hotkeys.clear();
	m_commands.clear();
	m_enablestates.clear();
	m_bCmdEditEnabled = true;
	// now fill in our own hotkeys and enabled states
	hotkeymodifiers modifiers;
	modifiers.command = 0;		// edit box : ctrl-K
	modifiers.alt = false;
	modifiers.shift = false;
	modifiers.control = true;
	m_hotkeys[WPARAM('K')] = modifiers;
	m_enablestates[1] = ENABLED_ALWAYS;
	modifiers.command = 2;		// console : ctrl-M
	modifiers.alt = false;
	modifiers.shift = false;
	modifiers.control = true;
	m_hotkeys[WPARAM('M')] = modifiers;
	m_enablestates[2] = ENABLED_VIEWPATH;
	m_enablestates[3] = ENABLED_SELECTED;
	modifiers.command = 4;		// copy paths : ctrl-shift-C
	modifiers.alt = false;
	modifiers.shift = true;
	modifiers.control = true;
	m_hotkeys[WPARAM('C')] = modifiers;
	m_enablestates[4] = ENABLED_SELECTED;
	modifiers.command = 5;		// new folder : ctrl-shift-N
	modifiers.alt = false;
	modifiers.shift = true;
	modifiers.control = true;
	m_hotkeys[WPARAM('N')] = modifiers;
	m_enablestates[5] = ENABLED_VIEWPATH;

	if (m_hWndToolbar == NULL)
		return FALSE;

	// first remove all existing buttons to start from scratch
	LRESULT buttoncount = ::SendMessage(m_hWndToolbar, TB_BUTTONCOUNT, 0, 0);
	for (int i=0; i<buttoncount; ++i)
	{
		::SendMessage(m_hWndToolbar, TB_DELETEBUTTON, 0, 0);
	}
	// destroy the image list
	ImageList_Destroy(m_hToolbarImgList);

	// find custom commands
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
	}

	CSimpleIni inifile;
	inifile.LoadFile(szPath);
	CSimpleIni::TNamesDepend sections;
	inifile.GetAllSections(sections);
	TBBUTTON * tb = new TBBUTTON[sections.size()+NUMINTERNALCOMMANDS];

	// create an image list containing the icons for the toolbar
	m_hToolbarImgList = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, sections.size()+NUMINTERNALCOMMANDS, 1);
	if (m_hToolbarImgList == NULL)
	{
		delete [] tb;
		return false;
	}
	BYTE fsStyle = BTNS_BUTTON;
	if (DWORD(m_regShowBtnText))
		fsStyle |= BTNS_SHOWTEXT;

	int customindex = 0;

	// now add the default command buttons
	HICON hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_OPTIONS));
	tb[customindex].iBitmap = ImageList_AddIcon(m_hToolbarImgList, hIcon);
	tb[customindex].idCommand = 1;
	tb[customindex].fsState = TBSTATE_ENABLED;
	tb[customindex].fsStyle = fsStyle;
	tb[customindex].iString = (INT_PTR)_T("Options");
	DestroyIcon(hIcon);

	customindex++;

	tb[customindex].iBitmap = 0;
	tb[customindex].idCommand = 0;
	tb[customindex].fsState = 0;
	tb[customindex].fsStyle = BTNS_SEP;
	tb[customindex].dwData = 0;
	tb[customindex].iString = 0;

	customindex++;

	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_CMD));
	tb[customindex].iBitmap = ImageList_AddIcon(m_hToolbarImgList, hIcon);
	tb[customindex].idCommand = 2;
	tb[customindex].fsState = TBSTATE_ENABLED;
	tb[customindex].fsStyle = fsStyle;
	tb[customindex].iString = (INT_PTR)_T("Console");
	DestroyIcon(hIcon);

	customindex++;

	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_COPYNAME));
	tb[customindex].iBitmap = ImageList_AddIcon(m_hToolbarImgList, hIcon);
	tb[customindex].idCommand = 3;
	tb[customindex].fsState = TBSTATE_ENABLED;
	tb[customindex].fsStyle = fsStyle;
	tb[customindex].iString = (INT_PTR)_T("Copy Names");
	DestroyIcon(hIcon);

	customindex++;

	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_COPYPATH));
	tb[customindex].iBitmap = ImageList_AddIcon(m_hToolbarImgList, hIcon);
	tb[customindex].idCommand = 4;
	tb[customindex].fsState = TBSTATE_ENABLED;
	tb[customindex].fsStyle = fsStyle;
	tb[customindex].iString = (INT_PTR)_T("Copy Paths");
	DestroyIcon(hIcon);

	customindex++;

	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_NEWFOLDER));
	tb[customindex].iBitmap = ImageList_AddIcon(m_hToolbarImgList, hIcon);
	tb[customindex].idCommand = 5;
	tb[customindex].fsState = TBSTATE_ENABLED;
	tb[customindex].fsStyle = fsStyle;
	tb[customindex].iString = (INT_PTR)_T("New Folder");
	DestroyIcon(hIcon);

	vector<int> hidelist;
	for (CSimpleIni::TNamesDepend::iterator it = sections.begin(); it != sections.end(); ++it)
	{
		wstring value = inifile.GetValue(*it, _T("internalcommand"), _T(""));
		if (!value.empty())
		{
			// some internal commands are overwritten
			WCHAR * stop;
			long command = wcstol(value.c_str(), &stop, 0);
			if (command <= NUMINTERNALCOMMANDS)
			{
				// we have to find the existing hotkey for that command
				// and remove it from the map first, but only if the user
				// has specified a new hotkey or set it to 0
				if (_tcslen(inifile.GetValue(*it, _T("hotkey"), _T(""))))
				{
					for (map<WPARAM, hotkeymodifiers>::iterator hk = m_hotkeys.begin(); hk != m_hotkeys.end(); ++hk)
					{
						if (hk->second.command == command)
						{
							m_hotkeys.erase(hk);
							break;
						}
					}
				}

				hotkeymodifiers modifiers;
				modifiers.command = command;
				value = inifile.GetValue(*it, _T("hotkey_alt"), _T(""));
				modifiers.alt = ((value.compare(_T("1"))==0)||(value.compare(_T("yes"))==0));
				value = inifile.GetValue(*it, _T("hotkey_shift"), _T(""));
				modifiers.shift = ((value.compare(_T("1"))==0)||(value.compare(_T("yes"))==0));
				value = inifile.GetValue(*it, _T("hotkey_control"), _T(""));
				modifiers.control = ((value.compare(_T("1"))==0)||(value.compare(_T("yes"))==0));
				value = inifile.GetValue(*it, _T("hotkey"), _T(""));
				long val = 0;
				if (!value.empty())
				{
					val = wcstol(value.c_str(), &stop, 0);
				}
				m_hotkeys[WPARAM(val)] = modifiers;
				value = inifile.GetValue(*it, _T("hide"), _T(""));
				if ((value.compare(_T("1"))==0)||(value.compare(_T("yes"))==0))
				{
					hidelist.push_back(command);
				}
			}
			continue;
		}

		customindex++;
		// check if this entry is a separator
		value = inifile.GetValue(*it, _T("separator"), _T(""));
		if (((value.compare(_T("1"))==0)||(value.compare(_T("yes"))==0)))
		{
			tb[customindex].iBitmap = 0;
			tb[customindex].idCommand = 0;
			tb[customindex].fsState = 0;
			tb[customindex].fsStyle = BTNS_SEP;
			tb[customindex].dwData = 0;
			tb[customindex].iString = 0;
			continue;
		}
		wstring cl = inifile.GetValue(*it, _T("commandline"), _T(""));
		if (cl.empty())
		{
			// if no command line is specified, just get out of here
			customindex--;
			continue;
		}
		value = inifile.GetValue(*it, _T("icon"), _T(""));
		hIcon = LoadIcon(g_hInst, value.c_str());
		if (hIcon)
			tb[customindex].iBitmap = ImageList_AddIcon(m_hToolbarImgList, hIcon);
		else
		{
			// icon loading failed. Let's try to load it differently:
			// the user might have specified a module path and an icon index
			// like this: c:\windows\explorer.exe,3 (the icon with ID 3 in explorer.exe)
			hIcon = NULL;
			if (value.find(',')>=0)
			{
				size_t pos = value.find_last_of(',');
				wstring resourcefile, iconid;
				if (pos >= 0)
				{
					resourcefile = value.substr(0, pos);
					iconid = value.substr(pos+1);
					hIcon = ExtractIcon(g_hInst, resourcefile.c_str(), _ttoi(iconid.c_str()));
				}
			}
			if (hIcon == NULL)
			{
				// loading the icon with an index didn't work either
				// next we try to use the icon of the application defined in the commandline
				wstring appname;
				if (cl.find(' ')>=0)
					appname = cl.substr(0, cl.find(' '));
				else
					appname = cl;
				hIcon = ExtractIcon(g_hInst, appname.c_str(), 0);
			}
			if (hIcon == NULL)
			{
				// if the icon handle is still invalid (no icon found yet),
				// we use a default icon
				hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_DEFAULT));
			}
			tb[customindex].iBitmap = ImageList_AddIcon(m_hToolbarImgList, hIcon);
		}
		tb[customindex].idCommand = customindex+1;
		tb[customindex].fsState = TBSTATE_ENABLED;
		tb[customindex].fsStyle = fsStyle;
		tb[customindex].iString = (INT_PTR)inifile.GetValue(*it, _T("name"), _T(""));
		DestroyIcon(hIcon);
		// now add the hotkey if it's present
		hotkeymodifiers modifiers;
		modifiers.command = customindex+1;
		value = inifile.GetValue(*it, _T("hotkey_alt"), _T(""));
		modifiers.alt = ((value.compare(_T("1"))==0)||(value.compare(_T("yes"))==0));
		value = inifile.GetValue(*it, _T("hotkey_shift"), _T(""));
		modifiers.shift = ((value.compare(_T("1"))==0)||(value.compare(_T("yes"))==0));
		value = inifile.GetValue(*it, _T("hotkey_control"), _T(""));
		modifiers.control = ((value.compare(_T("1"))==0)||(value.compare(_T("yes"))==0));
		value = inifile.GetValue(*it, _T("hotkey"), _T(""));
		if ((!value.empty())&&(!cl.empty()))
		{
			// the hotkey value could be a simple char or something more complicated like VK_F9
			WCHAR * stop;
			long val = wcstol(value.c_str(), &stop, 0);
			if (val)
				m_hotkeys[WPARAM(val)] = modifiers;
		}
		if (!cl.empty())
			m_commands[WORD(customindex+1)] = cl;
		// now parse the enabled states
		DWORD state = 0;
		value = inifile.GetValue(*it, _T("enabled_selected"), _T(""));
		if ((value.compare(_T("1"))==0)||(value.compare(_T("yes"))==0))
			state |= ENABLED_SELECTED;
		value = inifile.GetValue(*it, _T("enabled_fileselected"), _T(""));
		if ((value.compare(_T("1"))==0)||(value.compare(_T("yes"))==0))
			state |= ENABLED_FILESELECTED;
		value = inifile.GetValue(*it, _T("enabled_folderselected"), _T(""));
		if ((value.compare(_T("1"))==0)||(value.compare(_T("yes"))==0))
			state |= ENABLED_FOLDERSELECTED;
		value = inifile.GetValue(*it, _T("enabled_noselection"), _T(""));
		if ((value.compare(_T("1"))==0)||(value.compare(_T("yes"))==0))
			state |= ENABLED_NOSELECTION;
		value = inifile.GetValue(*it, _T("enabled_viewpath"), _T(""));
		if ((value.compare(_T("1"))==0)||(value.compare(_T("yes"))==0))
			state |= ENABLED_VIEWPATH;
		value = inifile.GetValue(*it, _T("enabled_noviewpath"), _T(""));
		if ((value.compare(_T("1"))==0)||(value.compare(_T("yes"))==0))
			state |= ENABLED_NOVIEWPATH;
		if (state == 0)
			state = ENABLED_ALWAYS;
		value = inifile.GetValue(*it, _T("enabled_numberselected"), _T(""));
		if (!value.empty())
		{
			WCHAR * stop;
			long val = wcstol(value.c_str(), &stop, 0);
			if (val)
				state = MAKELONG(state, val);
		}
		m_enablestates[customindex+1] = state;
	}

	SendMessage(m_hWndToolbar, TB_SETIMAGELIST, 0, (LPARAM)m_hToolbarImgList);
	SendMessage(m_hWndToolbar, TB_ADDBUTTONS, sections.size()+NUMINTERNALCOMMANDS, (LPARAM)tb);
	SendMessage(m_hWndToolbar, TB_SETEXTENDEDSTYLE, 0,(LPARAM)TBSTYLE_EX_MIXEDBUTTONS);
	SendMessage(m_hWndToolbar, TB_AUTOSIZE, 0, 0);
	SendMessage(m_hWndToolbar, TB_GETMAXSIZE, 0,(LPARAM)&m_tbSize);
	delete [] tb;
	// now hide the internal commands which the user configured to be hidden:
	for (vector<int>::iterator it = hidelist.begin(); it != hidelist.end(); ++it)
	{
		if (*it == 0)
		{
			m_bCmdEditEnabled = false;
		}
		else
			::SendMessage(m_hWndToolbar, TB_HIDEBUTTON, *it, (LPARAM)TRUE);
	}
	::ShowWindow(m_hWndEdit, m_bCmdEditEnabled ? SW_SHOW : SW_HIDE);

	// now inform our parent that the size of the deskband has changed
	if (m_pSite)
	{
		IOleCommandTarget * pOleCommandTarget;
		if (SUCCEEDED(m_pSite->QueryInterface(IID_IOleCommandTarget, (LPVOID*)&pOleCommandTarget)))
		{
			pOleCommandTarget->Exec(&CGID_DeskBand, DBID_BANDINFOCHANGED, OLECMDEXECOPT_DONTPROMPTUSER, NULL, NULL);
		}
	}
	return TRUE;
}
