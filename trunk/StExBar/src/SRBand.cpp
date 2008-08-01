// StExBar - an explorer toolbar

// Copyright (C) 2007-2008 - Stefan Kueng

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
#include "uxtheme.h"
#include "ChevronMenu.h"
#include "OptionsDlg.h"

#pragma comment(lib, "uxtheme.lib")


map<DWORD, CDeskBand*> CDeskBand::m_desklist;	///< set of CDeskBand objects which use the keyboard hook

CDeskBand::CDeskBand() : m_bFocus(false)
	, m_hwndParent(NULL)
	, m_hWnd(NULL)
	, m_hWndEdit(NULL)
	, m_hWndToolbar(NULL)
	, m_dwViewMode(0)
	, m_dwBandID(0)
	, m_oldEditWndProc(NULL)
	, m_pSite(NULL)
	, m_regShowBtnText(_T("Software\\StefansTools\\StExBar\\ShowButtonText"), 1)
	, m_regUseUNCPaths(_T("Software\\StefansTools\\StExBar\\UseUNCPaths"), 1)
	, m_regUseSelector(_T("Software\\StefansTools\\StExBar\\UseSelector"), 1)
	, m_regHideEditBox(_T("Software\\StefansTools\\StExBar\\HideEditBox"), 0)
	, m_hToolbarImgList(NULL)
	, m_bDialogShown(FALSE)
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
		m_pSite->Release();
		m_pSite = NULL;
	}
	UnhookWindowsHookEx(m_hook);
	map<DWORD, CDeskBand*>::iterator it = m_desklist.find(GetCurrentThreadId());
	if (it != m_desklist.end())
		m_desklist.erase(it);
	// un-subclass
	SetWindowLongPtr(::GetParent(m_hwndParent), GWLP_WNDPROC, (LONG_PTR)m_oldDeskBandProc);

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

	// IPersistStream
	else if (IsEqualIID(riid, IID_IColumnProvider))
	{
		*ppReturn = (IColumnProvider*)this;
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
			pdbi->ptActual.x = m_tbSize.cx;// + (m_bCmdEditEnabled ? EDITBOXSIZEX : 0);
			pdbi->ptActual.y = m_tbSize.cy;
		}

		if (pdbi->dwMask & DBIM_TITLE)
		{
			StringCchCopy(pdbi->wszTitle, 256, L"StEx");
		}

		if (pdbi->dwMask & DBIM_MODEFLAGS)
		{
			// We want chevrons
			pdbi->dwModeFlags = DBIMF_NORMAL|DBIMF_USECHEVRON;
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
// IDeskBand methods
//////////////////////////////////////////////////////////////////////////
STDMETHODIMP CDeskBand::CanRenderComposited(BOOL * pfCanRenderComposited)
{
	if (pfCanRenderComposited)
		*pfCanRenderComposited = TRUE;
	return S_OK;
}

STDMETHODIMP CDeskBand::GetCompositionState(BOOL * pfCompositionState)
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
	CDeskBand *pThis = (CDeskBand*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	switch (uMessage)
	{
	case WM_NCCREATE:
		{
			LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
			pThis = (CDeskBand*)(lpcs->lpCreateParams);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);

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
			// the "show/hide system files" button is special: it has a pressed state if the
			// hidden files are shown
			SHELLSTATE shellstate = {0};
			SHGetSetSettings(&shellstate, SSF_SHOWSYSFILES|SSF_SHOWSUPERHIDDEN|SSF_SHOWALLOBJECTS, FALSE);
			for (int i=0; i<pThis->m_commands.GetCount(); ++i)
			{
				if (pThis->m_commands.GetCommandPtr(i)->name.compare(_T("Show system files")) == 0)
				{
					::SendMessage(pThis->m_hWndToolbar, TB_CHECKBUTTON, i, (LPARAM)shellstate.fShowAllObjects);
					break;
				}
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
				LPNMTTDISPINFO lpnmtdi = (LPNMTTDISPINFO) lParam;
				map<int, wstring>::iterator it = pThis->m_tooltips.find(pnmh->idFrom);
				if (it != pThis->m_tooltips.end())
				{
					_tcsncpy_s(lpnmtdi->lpszText, 80, it->second.c_str(), 79);
				}
			}

		}
		break;

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
	CDeskBand *pThis = (CDeskBand*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if (uMessage == WM_SETFOCUS)
	{
		pThis->OnSetFocus();
		::SendMessage(pThis->m_hWndEdit, EM_SETSEL, 0, (LPARAM)-1);
	}
	return CallWindowProc(pThis->m_oldEditWndProc, hWnd, uMessage, wParam, lParam);
}

LRESULT CALLBACK CDeskBand::DeskBandProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	CDeskBand *pThis = (CDeskBand*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	LPNMREBARCHEVRON pnmh = (LPNMREBARCHEVRON)lParam;
	if ((uMessage == WM_NOTIFY)&&(pnmh->hdr.code == RBN_CHEVRONPUSHED)&&(pnmh->wID == pThis->m_dwBandID))
	{
		CChevronMenu menu(g_hInst);
		if (menu.Show(pnmh, pThis->m_hWndToolbar))
		{
			::SendMessage(pThis->m_hWndToolbar, menu.m_uMsg, menu.m_wParam, menu.m_lParam);
		}
		return 0;
	}
	return CallWindowProc(pThis->m_oldDeskBandProc, hWnd, uMessage, wParam, lParam);
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
			bool bEnabled = true;
			if (it != m_enablestates.end())
			{
				if (((it->second & 0xFFFF)&state)&&
					((HIWORD(it->second) == 0)||((m_selectedItems.size() == 0)&&(it->second & ENABLED_NOSELECTION))||(HIWORD(it->second) == m_selectedItems.size())))
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
			if (cmd.commandline.compare(INTERNALCOMMAND) == 0)
			{
				// an internal command
				if (cmd.name.compare(_T("StexBar Internal Edit Box")) == 0)
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
					if (DWORD(m_regUseSelector))
					{
						// select the files which match the filter string
						Select(buf);
					}
					else
					{
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
					}
					delete [] buf;
				}
				else if (cmd.name.compare(_T("Options")) == 0)
				{
					COptionsDlg dlg(m_hWnd);
					m_bDialogShown = TRUE;
					if (dlg.DoModal(g_hInst, IDD_OPTIONS, m_hWnd) == IDOK)
					{
						m_bDialogShown = FALSE;
						m_regUseSelector.read();
						m_regUseUNCPaths.read();
						m_regShowBtnText.read();
						BuildToolbarButtons();
						OnMove(0);
					}
					m_bDialogShown = FALSE;
				}
				else if (cmd.name.compare(_T("Show system files")) == 0)
				{
					HCURSOR hCur = GetCursor();
					SetCursor(LoadCursor(NULL, IDC_WAIT));
					SHELLSTATE state = {0};
					SHGetSetSettings(&state, SSF_SHOWSYSFILES|SSF_SHOWSUPERHIDDEN|SSF_SHOWALLOBJECTS, FALSE);
					state.fShowSysFiles = !state.fShowAllObjects;
					state.fShowAllObjects = !state.fShowAllObjects;
					state.fShowSuperHidden = !state.fShowAllObjects;
					SHGetSetSettings(&state, SSF_SHOWSYSFILES|SSF_SHOWSUPERHIDDEN|SSF_SHOWALLOBJECTS, TRUE);
					// now refresh the view
					IServiceProvider * pServiceProvider;
					if (SUCCEEDED(m_pSite->QueryInterface(IID_IServiceProvider, (LPVOID*)&pServiceProvider)))
					{
						IShellBrowser * pShellBrowser;
						if (SUCCEEDED(pServiceProvider->QueryService(SID_SShellBrowser, IID_IShellBrowser, (LPVOID*)&pShellBrowser)))
						{
							IShellView * pShellView;
							if (SUCCEEDED(pShellBrowser->QueryActiveShellView(&pShellView)))
							{
								pShellView->Refresh();
								pShellView->Release();
							}
							pShellBrowser->Release();
						}
						pServiceProvider->Release();
					}
					SetCursor(hCur);
				}
				else if (cmd.name.compare(_T("Console")) == 0)
				{
					StartCmd(_T(""));
				}
				else if (cmd.name.compare(_T("Copy Names")) == 0)
				{
					wstring str = GetFileNames(_T("\r\n"), false, true, true);
					if (str.empty())
					{
						// Seems no items are selected
						// Use the view path instead
						size_t pos = m_currentDirectory.find_last_of('\\');
						WCHAR buf[MAX_PATH];
						if (pos >= 0)
						{
							_tcscpy_s(buf, MAX_PATH, m_currentDirectory.substr(pos+1).c_str());
							PathQuoteSpaces(buf);
							str = buf;
						}
					}
					WriteStringToClipboard(str, m_hWnd);
				}
				else if (cmd.name.compare(_T("Copy Paths")) == 0)
				{
					wstring str = GetFilePaths(_T("\r\n"), false, true, true, DWORD(m_regUseUNCPaths) ? true : false);
					if (str.empty())
					{
						// Seems no items are selected
						// Use the view path instead
						WCHAR buf[MAX_PATH];
						_tcscpy_s(buf, MAX_PATH, m_currentDirectory.c_str());
						PathQuoteSpaces(buf);
						str = buf;
					}
					WriteStringToClipboard(str, m_hWnd);
				}
				else if (cmd.name.compare(_T("New Folder")) == 0)
				{
					CreateNewFolder();
				}
				else if (cmd.name.compare(_T("Rename")) == 0)
				{
					Rename();
				}
				else
					DebugBreak();
			}
			else
			{
				int count = MAX_PATH;
				TCHAR * buf = new TCHAR[count+1];
				while (::GetWindowText(m_hWndEdit, buf, count)>=count)
				{
					delete [] buf;
					count += MAX_PATH;
					buf = new TCHAR[count+1];
				}
				wstring consoletext = buf;

				// replace "%selpaths" with the paths of the selected items
				wstring tag(_T("%selpaths"));
				wstring commandline = cmd.commandline;
				wstring::iterator it_begin = search(commandline.begin(), commandline.end(), tag.begin(), tag.end());
				if (it_begin != commandline.end())
				{
					// prepare the selected paths
					wstring selpaths = GetFilePaths(_T(" "), true, true, true, false);
					if (selpaths.empty())
						selpaths = m_currentDirectory;
					wstring::iterator it_end= it_begin + tag.size();
					commandline.replace(it_begin, it_end, selpaths);
				}
				// replace "%sel*paths" with the paths of the selected items
				tag = _T("%sel*paths");
				it_begin = search(commandline.begin(), commandline.end(), tag.begin(), tag.end());
				if (it_begin != commandline.end())
				{
					// prepare the selected names
					wstring selpaths = GetFilePaths(_T("*"), false, true, true, false);
					if (selpaths.empty())
						selpaths = m_currentDirectory;
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
				// replace "%cmdtext" with the text in the console edit box
				tag = _T("%cmdtext");
				it_begin = search(commandline.begin(), commandline.end(), tag.begin(), tag.end());
				if (it_begin != commandline.end())
				{
					wstring::iterator it_end= it_begin + tag.size();
					commandline.replace(it_begin, it_end, consoletext);
				}
				StartApplication(commandline);
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

	if (m_bCmdEditEnabled)
	{
		HDWP hdwp = BeginDeferWindowPos(2);
		DeferWindowPos(hdwp, m_hWndToolbar, NULL, 0, 0, m_tbSize.cx, m_tbSize.cy, SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOZORDER);
		DeferWindowPos(hdwp, m_hWndEdit, NULL, m_tbSize.cx+SPACEBETWEENEDITANDBUTTON, 0, rc.right-rc.left-m_tbSize.cx-SPACEBETWEENEDITANDBUTTON, m_tbSize.cy, SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOZORDER);
		EndDeferWindowPos(hdwp);
		ShowWindow(m_hWndEdit, SW_SHOW);
	}
	else
	{
		SetWindowPos(m_hWndToolbar, NULL, 0, 0, m_tbSize.cx, m_tbSize.cy, SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOZORDER);
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
		DeferWindowPos(hdwp, m_hWndToolbar, NULL, 0, 0, m_tbSize.cx, m_tbSize.cy, SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOZORDER);
		DeferWindowPos(hdwp, m_hWndEdit, NULL, m_tbSize.cx, 0, rc.right-rc.left-m_tbSize.cx-SPACEBETWEENEDITANDBUTTON, m_tbSize.cy, SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOZORDER);
		EndDeferWindowPos(hdwp);
		ShowWindow(m_hWndEdit, SW_SHOW);
	}
	else
	{
		SetWindowPos(m_hWndToolbar, NULL, 0, 0, m_tbSize.cx, m_tbSize.cy, SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOZORDER);
		ShowWindow(m_hWndEdit, SW_HIDE);
	}
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
		WNDCLASSEX wc;
		if (!GetClassInfoEx(g_hInst, DB_CLASS_NAME, &wc))
		{
			ZeroMemory(&wc, sizeof(wc));
			wc.cbSize		  = sizeof(WNDCLASSEX);
			wc.style          = CS_HREDRAW | CS_VREDRAW;
			wc.lpfnWndProc    = (WNDPROC)WndProc;
			wc.cbClsExtra     = 0;
			wc.cbWndExtra     = 0;
			wc.hInstance      = g_hInst;
			wc.hIcon          = NULL;
			wc.hIconSm		  = NULL;
			wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
			wc.hbrBackground  = (HBRUSH)(COLOR_BTNFACE+1);
			wc.lpszMenuName   = NULL;
			wc.lpszClassName  = DB_CLASS_NAME;

			if (!RegisterClassEx(&wc))
			{
				return FALSE;
			}
		}

		RECT  rc;

		GetClientRect(m_hwndParent, &rc);

		// subclass the parent deskbar control to intercept the RBN_CHEVRONPUSHED messages
		m_oldDeskBandProc = (WNDPROC)SetWindowLongPtr(::GetParent(m_hwndParent), GWLP_WNDPROC, (LONG_PTR)DeskBandProc);
		SetWindowLongPtr(::GetParent(m_hwndParent), GWLP_USERDATA, (LONG_PTR)this);

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
			rc.right - rc.left - EDITBOXSIZEX - SPACEBETWEENEDITANDBUTTON,
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
		m_oldEditWndProc = (WNDPROC)SetWindowLongPtr(m_hWndEdit, GWLP_WNDPROC, (LONG_PTR)EditProc);
		SetWindowLongPtr(m_hWndEdit, GWLP_USERDATA, (LONG_PTR)this);

		// create a toolbar which will hold our button
		m_hWndToolbar = CreateWindowEx(TBSTYLE_EX_MIXEDBUTTONS|TBSTYLE_EX_HIDECLIPPEDBUTTONS,
			TOOLBARCLASSNAME, 
			NULL, 
			WS_CHILD|TBSTYLE_TOOLTIPS|TBSTYLE_WRAPABLE|TBSTYLE_LIST|TBSTYLE_FLAT|TBSTYLE_TRANSPARENT|CCS_NORESIZE|CCS_NODIVIDER|CCS_NOPARENTALIGN, 
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
		if ((!it->second->m_bDialogShown)&&(code >= 0)&&((lParam & 0xc0000000) == 0))//key went from 'up' to 'down' state
		{
			hotkey realhk;
			realhk.keycode = wParam;
			realhk.alt = !!(GetKeyState(VK_MENU)&0x8000);
			realhk.control = !!(GetKeyState(VK_CONTROL)&0x8000);
			realhk.shift = !!(GetKeyState(VK_SHIFT)&0x8000);
			map<hotkey, int>::iterator hk = it->second->m_hotkeys.find(realhk);
			if (hk != it->second->m_hotkeys.end())
			{
					// special handling of command 0: just set the focus!
					if ((hk->second==0)&&(it->second->m_pSite))
					{
						if (it->second->m_bCmdEditEnabled)
							it->second->OnSetFocus();
						return 1;//we processed it
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
	for (int i=0; i<buttoncount; ++i)
	{
		::SendMessage(m_hWndToolbar, TB_DELETEBUTTON, 0, 0);
	}
	if (m_hToolbarImgList)
	{
		// destroy the image list
		ImageList_Destroy(m_hToolbarImgList);
	}

	m_commands.LoadFromFile();

	TBBUTTON * tb = new TBBUTTON[m_commands.GetCount()];
	// create an image list containing the icons for the toolbar
	m_hToolbarImgList = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, m_commands.GetCount(), 1);
	if (m_hToolbarImgList == NULL)
	{
		delete [] tb;
		return false;
	}
	BYTE fsStyle = BTNS_BUTTON;
	if (DWORD(m_regShowBtnText))
		fsStyle |= BTNS_SHOWTEXT;

	int index = 0;
	for (int j = 0; j < m_commands.GetCount(); ++j)
	{
		Command cmd = m_commands.GetCommand(j);
		m_hotkeys[cmd.key] = j;
		if ((cmd.name.compare(_T("StexBar Internal Edit Box")) == 0)||
			((cmd.commandline.compare(INTERNALCOMMANDHIDDEN)==0))&&(cmd.name.compare(_T("Options"))))
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

		HICON hIcon = NULL;
		if (cmd.nIconID)
			hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(cmd.nIconID));
		else if (!cmd.separator)
		{
			hIcon = LoadIcon(g_hInst, cmd.icon.c_str());
			if (hIcon == NULL)
			{
				// icon loading failed. Let's try to load it differently:
				// the user might have specified a module path and an icon index
				// like this: c:\windows\explorer.exe,3 (the icon with ID 3 in explorer.exe)
				hIcon = NULL;
				if (cmd.icon.find(',')>=0)
				{
					size_t pos = cmd.icon.find_last_of(',');
					wstring resourcefile, iconid;
					if (pos >= 0)
					{
						resourcefile = cmd.icon.substr(0, pos);
						iconid = cmd.icon.substr(pos+1);
						hIcon = ExtractIcon(g_hInst, resourcefile.c_str(), _ttoi(iconid.c_str()));
					}
				}
				if (hIcon == NULL)
				{
					// loading the icon with an index didn't work either
					// next we try to use the icon of the application defined in the commandline
					wstring appname;
					if (cmd.commandline.find(' ')>=0)
						appname = cmd.commandline.substr(0, cmd.commandline.find(' '));
					else
						appname = cmd.commandline;
					hIcon = ExtractIcon(g_hInst, appname.c_str(), 0);
				}
				if (hIcon == NULL)
				{
					// if the icon handle is still invalid (no icon found yet),
					// we use a default icon
					hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_DEFAULT));
				}
			}
		}
		if (hIcon)
		{
			tb[index].iBitmap = ImageList_AddIcon(m_hToolbarImgList, hIcon);
			DestroyIcon(hIcon);
		}
		else
			tb[index].iBitmap = NULL;
		tb[index].idCommand = j;
		tb[index].fsState = cmd.separator ? 0 : TBSTATE_ENABLED;
		tb[index].fsStyle = cmd.separator ? BTNS_SEP : fsStyle;
		tb[index].iString = cmd.separator ? NULL : (INT_PTR)m_commands.GetCommandPtr(j)->name.c_str();
		tb[index].dwData = NULL;
		if (!cmd.separator)
			m_tooltips[tb[index].idCommand] = cmd.name.c_str();
		index++;
	}

	SendMessage(m_hWndToolbar, TB_SETIMAGELIST, 0, (LPARAM)m_hToolbarImgList);
	SendMessage(m_hWndToolbar, TB_ADDBUTTONS, index, (LPARAM)tb);
	SendMessage(m_hWndToolbar, TB_SETEXTENDEDSTYLE, 0,(LPARAM)TBSTYLE_EX_MIXEDBUTTONS|TBSTYLE_EX_HIDECLIPPEDBUTTONS);
	SendMessage(m_hWndToolbar, TB_AUTOSIZE, 0, 0);
	SendMessage(m_hWndToolbar, TB_GETMAXSIZE, 0,(LPARAM)&m_tbSize);
	delete [] tb;

	// now inform our parent that the size of the deskband has changed
	if (m_pSite)
	{
		IOleCommandTarget * pOleCommandTarget;
		if (SUCCEEDED(m_pSite->QueryInterface(IID_IOleCommandTarget, (LPVOID*)&pOleCommandTarget)))
		{
			pOleCommandTarget->Exec(&CGID_DeskBand, DBID_BANDINFOCHANGED, OLECMDEXECOPT_DONTPROMPTUSER, NULL, NULL);
			pOleCommandTarget->Release();
		}
	}
	return TRUE;
}
