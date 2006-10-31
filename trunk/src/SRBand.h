#pragma once

#include <shlobj.h>

#include "Globals.h"
#include "ResizableGrip.h"
#include <set>
#include <map>
#include "Registry.h"

#define DB_CLASS_NAME (TEXT("SRBarClass"))

#define MIN_SIZE_X   100
#define MIN_SIZE_Y   20

#define IDM_COMMAND  0
#define ID_SELECTTIMER 101

#define BUTTONSIZEX	60
#define SPACEBETWEENEDITANDBUTTON 0

#define NUMINTERNALCOMMANDS 4

/*
command
	name
	icon
	commandline
	shortcutkey
*/

/**
 * Desk Band.
 * Implements a desk band used by the shell.
 * This requires the implementation of several interfaces.
 */
class CDeskBand : public IDeskBand 
	, public IInputObject
	, public IObjectWithSite
	, public IPersistStream
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
	WNDPROC			m_oldEditWndProc;	///< pointer to the original window proc of the edit control
	HIMAGELIST		m_hToolbarImgList;	///< image list of the toolbar
	SIZE			m_tbSize;			///< the max size of the toolbar

	CResizableGrip	m_grip;				///< the grip used in the rename dialog

	static std::map<DWORD, CDeskBand*> m_desklist;	///< map of thread-ID's and CDeskBand objects which use the keyboard hook
	HHOOK			m_hook;				///< handle of the keyboard hook procedure

	stdstring		m_currentDirectory;	///< the current directory of the explorer view
	std::set<stdstring> m_selectedItems;///< list of items which are selected in the explorer view
private:
	/// window procedure of the sub classed edit control
	static LRESULT CALLBACK	EditProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
	/// rename dialog callback function
	static INT_PTR CALLBACK	RenameDlgFunc(HWND, UINT, WPARAM, LPARAM);

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

	// functions put into separate cpp files

	/// find all the paths of the current explorer view and the selected items
	bool					FindPaths();
	/// get a list of filenames in one string, separated by \c separator
	std::wstring			GetFileNames(std::wstring separator);
	/// get a list of filepaths in one string, separated by \c separator
	std::wstring			GetFilePaths(std::wstring separator);
	/// put a string on the clipboard
	bool					WriteStringToClipboard(const stdstring& sClipdata, HWND hOwningWnd);
	/// starts the console program to run a script
	void					StartCmd(std::wstring params);

};

