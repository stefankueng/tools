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
#pragma once
#include <string>

using namespace std;

/**
 * Loads a string from the application resources.
 */
class ResString
{
	enum { MAX_RESSTRING = 1024 };
public:
	ResString (HINSTANCE hInst, int resId);
	operator TCHAR const * () const { return _buf; }
private:
	TCHAR _buf [MAX_RESSTRING + 1];
};

/**
 * A base window class.
 * Provides separate window message handlers for every window object based on
 * this class.
 */
class CWindow
{
public:
	virtual bool RegisterWindow(UINT style, HICON hIcon, HCURSOR hCursor, HBRUSH hbrBackground, 
		LPCTSTR lpszMenuName, LPCTSTR lpszClassName, HICON hIconSm);
	virtual bool RegisterWindow(CONST WNDCLASSEX* wcx);

	/// static message handler to put in WNDCLASSEX structure
	static LRESULT CALLBACK stWinMsgHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	/**
	 * Sets the window title. 
	 */
	void SetWindowTitle(const std::wstring& sTitle) 
	{
		sWindowTitle = sTitle;
	};

	/**
	 * Sets the transparency of the window.
	 * \remark note that this also sets the WS_EX_LAYERED style!
	 */
	void SetTransparency(BYTE alpha, COLORREF color = 0xFF000000);

	virtual bool Create();
	virtual bool Create(DWORD dwStyles, HWND hParent = NULL, RECT* rect = NULL);
	virtual bool CreateEx(DWORD dwExStyles, DWORD dwStyles, HWND hParent = NULL, RECT* rect = NULL);

	//void MsgLoop();
	bool IsWindowClosed() { return bWindowClosed; };

	operator HWND() {return m_hwnd;}
protected:
	HINSTANCE hResource;
	HWND m_hwnd;
	bool bWindowClosed;
	std::wstring sClassName;
	std::wstring sWindowTitle;

	//constructor 
	CWindow(HINSTANCE hInst, CONST WNDCLASSEX* wcx = NULL) : m_hwnd(NULL)
		, hResource(NULL)
		, bWindowClosed(FALSE)
	{
		hResource = hInst; 
		if (wcx != NULL)
			RegisterWindow(wcx);
	};

	// the real message handler
	virtual LRESULT CALLBACK WinMsgHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;

	// returns a pointer the window (stored as the WindowLong)
	inline static CWindow * GetObjectFromWindow(HWND hWnd)
	{
		return (CWindow *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	}
};

