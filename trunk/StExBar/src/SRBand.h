// StExBar - an explorer toolbar

// Copyright (C) 2007-2009 - Stefan Kueng

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
#pragma once

#include <shlobj.h>

#include "Globals.h"
#include <set>
#include <map>
#include "Registry.h"
#include "Commands.h"

using namespace std;

#define DB_CLASS_NAME (TEXT("StExBarClass"))

#define MIN_SIZE_X   100
#define MIN_SIZE_Y   20

#define IDM_COMMAND  0
#define ID_SELECTTIMER 101

#define EDITBOXSIZEX	60
#define SPACEBETWEENEDITANDBUTTON 0

// the number of 'internal' commands
#define NUMINTERNALCOMMANDS 9

// the timer ID
#define TID_IDLE 100
#define TID_FILTER 101

// the enabled states
#define ENABLED_VIEWPATH			0x00000001
#define ENABLED_NOVIEWPATH			0x00000002
#define ENABLED_FOLDERSELECTED		0x00000004
#define ENABLED_FILESELECTED		0x00000008
#define ENABLED_NOSELECTION			0x00000010
#define ENABLED_SELECTED (ENABLED_FOLDERSELECTED|ENABLED_FILESELECTED)
#define ENABLED_ALWAYS	(ENABLED_VIEWPATH|ENABLED_NOVIEWPATH|ENABLED_FOLDERSELECTED|ENABLED_FILESELECTED|ENABLED_NOSELECTION)



/**
 * Desk Band.
 * Implements a desk band used by the shell.
 * This requires the implementation of several interfaces.
 */
class CDeskBand : public IDeskBand2 
	, public IInputObject
	, public IObjectWithSite
	, public IPersistStream
	, public IContextMenu3
	, public IShellExtInit
{
protected:
	DWORD m_ObjRefCount;

public:
	CDeskBand();
	~CDeskBand();

	// IUnknown methods
	STDMETHODIMP QueryInterface(REFIID, LPVOID*);
	STDMETHODIMP_(DWORD) AddRef();
	STDMETHODIMP_(DWORD) Release();

	// IOleWindow methods
	STDMETHOD (GetWindow) (HWND*);
	STDMETHOD (ContextSensitiveHelp) (BOOL);

	// IDockingWindow methods
	STDMETHOD (ShowDW) (BOOL fShow);
	STDMETHOD (CloseDW) (DWORD dwReserved);
	STDMETHOD (ResizeBorderDW) (LPCRECT prcBorder, IUnknown* punkToolbarSite, BOOL fReserved);

	// IDeskBand methods
	STDMETHOD (GetBandInfo) (DWORD, DWORD, DESKBANDINFO*);

	// IDeskBand2 methods
	STDMETHOD (CanRenderComposited) (BOOL * pfCanRenderComposited);
	STDMETHOD (GetCompositionState) (BOOL * pfCompositionState);
	STDMETHOD (SetCompositionState) (BOOL pfCompositionState);

	// IInputObject methods
	STDMETHOD (UIActivateIO) (BOOL, LPMSG);
	STDMETHOD (HasFocusIO) (void);
	STDMETHOD (TranslateAcceleratorIO) (LPMSG);

	// IObjectWithSite methods
	STDMETHOD (SetSite) (IUnknown*);
	STDMETHOD (GetSite) (REFIID, LPVOID*);

	// IPersistStream methods
	STDMETHOD (GetClassID) (LPCLSID);
	STDMETHOD (IsDirty) (void);
	STDMETHOD (Load) (LPSTREAM);
	STDMETHOD (Save) (LPSTREAM, BOOL);
	STDMETHOD (GetSizeMax) (ULARGE_INTEGER*);

	// IContextMenu2 members
	STDMETHOD	(QueryContextMenu) (HMENU hMenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
	STDMETHOD	(InvokeCommand) (LPCMINVOKECOMMANDINFO lpcmi);
	STDMETHOD	(GetCommandString) (UINT_PTR idCmd, UINT uFlags, UINT FAR *reserved, LPSTR pszName, UINT cchMax);
	STDMETHOD	(HandleMenuMsg) (UINT uMsg, WPARAM wParam, LPARAM lParam);

	// IContextMenu3 members
	STDMETHOD	(HandleMenuMsg2) (UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *pResult);

	// IShellExtInit methods
	STDMETHOD	(Initialize) (LPCITEMIDLIST pIDFolder, LPDATAOBJECT pDataObj, HKEY hKeyID);

	HWND GetParent(){return m_hwndParent;}
private:
	BOOL			m_bFocus;			///< true if the deskband or one of its children has the focus
	HWND			m_hwndParent;		///< handle of the parent window, usually the shell
	HWND			m_hWnd;				///< window handle of the deskband itself
	HWND			m_hWndToolbar;		///< window handle of the toolbar which hold the button
	HWND			m_hWndEdit;			///< window handle of the edit control
	DWORD			m_dwViewMode;		///< viewing mode of the deskband
	DWORD			m_dwBandID;			///< the ID of the deskband
	IInputObjectSite *m_pSite;			///< the IInputObjectSite of the parent
	HIMAGELIST		m_hToolbarImgList;	///< image list of the toolbar
	SIZE			m_tbSize;			///< the max size of the toolbar
	BOOL			m_bCompositionState;///< the composition state
	BOOL			m_bDialogShown;		///< true if a dialog is currently shown to deactivate the keyboard accelerators

	static std::map<DWORD, CDeskBand*> m_desklist;	///< map of thread-ID's and CDeskBand objects which use the keyboard hook
	HHOOK			m_hook;				///< handle of the keyboard hook procedure

	wstring			m_currentDirectory;	///< the current directory of the explorer view
	map<wstring, ULONG>	m_selectedItems;///< list of items which are selected in the explorer view
	bool			m_bFilesSelected;	///< at least one file is selected
	bool			m_bFolderSelected;	///< at least one folder is selected

	CCommands		m_commands;
	map<hotkey, int> m_hotkeys;			///< the hotkeys for our commands
	map<int, DWORD> m_enablestates;		///< the custom commands and their enabled states
	bool			m_bCmdEditEnabled;	///< the cmd edit box is special, because it's not part of the toolbar

	set<wstring>	m_filelist;			///< the list of selected file/folder names
	map<int, wstring> m_tooltips;		///< maps command/button ids against the tooltips to show for them
	CRegStdWORD		m_regShowBtnText;	///< config setting whether to show the text for the toolbar buttons or not
	CRegStdWORD		m_regUseUNCPaths;	///< config setting whether to copy the UNC paths of mapped paths or not
	CRegStdWORD		m_regUseSelector;	///< config setting whether to use the selector or the cmd.exe replacement
	CRegStdWORD		m_regHideEditBox;	///< config setting whether to show the edit box or not

	std::vector<LPITEMIDLIST>	m_noShows;	///< list of pidls which didn't match a filter
	LPITEMIDLIST	m_currentFolder;	///< pidl of the current folder
	HWND			m_hwndListView;		///< handle of the list view control

	wstring			m_ContextDirectory;	///< the folder background path for the context menu
	map<wstring, ULONG>	m_ContextItems;	///< list of items which are selected for the context menu
	std::map<UINT_PTR, UINT_PTR>	myIDMap;	///< maps menu ids to command ids

private:
	/// window procedure of the sub classed desk band control
	static LRESULT CALLBACK DeskBandProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
	/// window procedure of the sub classed edit control
	static LRESULT CALLBACK	EditProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

	static LRESULT CALLBACK KeyboardHookProc(int code, WPARAM wParam, LPARAM lParam);

	/// change the focus
	void					FocusChange(BOOL);
	/// called when the deskband looses focus
	LRESULT					OnKillFocus(void);
	/// called when the deskband gains focus
	LRESULT					OnSetFocus(void);
	/// window procedure of the deskband
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
	/// handles WM_COMMAND messages
	LRESULT					OnCommand(WPARAM wParam, LPARAM lParam);
	/// handles WM_SIZE messages
	LRESULT					OnSize(LPARAM);
	/// handles WM_MOVE messages
	LRESULT					OnMove(LPARAM);
	/// registers and creates the deskband and the child controls
	BOOL					RegisterAndCreateWindow(void);
	/// set up the toolbar
	BOOL					BuildToolbarButtons();
	/// handle a command
	void					HandleCommand(HWND hWnd, const Command& cmd, const wstring& cwd, const map<wstring, ULONG>& items);
	/// loads the icon for the command. The caller is responsible for destroying the icon after using it.
	HICON					LoadCommandIcon(const Command& cmd);

	// functions put into separate cpp files

	/// find all the paths of the current explorer view and the selected items
	bool					FindPaths();
	/// get a list of filenames in one string, separated by \c separator
	wstring					GetFileNames(const map<wstring, ULONG>& items, wstring separator, bool quotespaces, bool includefiles, bool includefolders);
	/// get a list of file paths in one string, separated by \c separator
	wstring					GetFilePaths(const map<wstring, ULONG>& items, wstring separator, bool quotespaces, bool includefiles, bool includefolders, bool useunc);
	/// get a list of folder paths in one string, separated by \c separator
	/// put a string on the clipboard
	bool					WriteStringToClipboard(const wstring& sClipdata, HWND hOwningWnd);
	/// starts the console program to run a script
	void					StartCmd(const wstring& cwd, wstring params);
	/// start a new process with the specified command line
	void					StartApplication(const wstring& cwd, std::wstring commandline);
	/// creates a new folder and starts the editing of it
	bool					CreateNewFolder();
	/// Opens a rename dialog where the user can rename the selected files
	void					Rename(HWND hwnd, const map<wstring, ULONG>& items);
	/// helper function to find the IServiceProvider
	HRESULT					GetIServiceProvider(HWND hwnd, IServiceProvider ** pServiceProvider);
	/// Fills the list with the renamed files in the rename dialog
	void					FillRenamedList(HWND hDlg);
	/// convert a path to an UNC path (if it points to a network share)
	wstring					ConvertToUNC(wstring sPath);
	/// filters out any file/folder in the current view which don't match the filter string
	bool					Filter(LPTSTR filter);
	/// writes the string \a paths to a tempfile, either in unicode or ascii. The path to the tempfile is returned
	wstring					WriteFileListToTempFile(bool bUnicode, const wstring& paths);
	/// returns true if the pidl matches the filter string
	bool					CheckDisplayName(IShellFolder * shellFolder, LPITEMIDLIST pidl, LPCTSTR filter, bool bUseRegex);
	/// returns the list view control
	HWND					GetListView32(IShellView * shellView);
	static BOOL CALLBACK	EnumChildProc(HWND hwnd, LPARAM lParam);

};

