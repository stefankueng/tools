// ResizableGrip.cpp: implementation of the CResizableGrip class.
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2000-2001 by Paolo Messina
// (http://www.geocities.com/ppescher - ppescher@yahoo.com)
//
// The contents of this file are subject to the Artistic License (the "License").
// You may not use this file except in compliance with the License. 
// You may obtain a copy of the License at:
// http://www.opensource.org/licenses/artistic-license.html
//
// If you find this code useful, credits would be nice!
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ResizableGrip.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif





// Constructor
CResizableControl::CResizableControl()
{
	m_hCtrl = NULL;
	m_bAllowMoveX = m_bAllowMoveY = m_bAllowResizeX = m_bAllowResizeY = FALSE;
}

// Methods
void CResizableControl::AllowMoveOnResize(BOOL bAllow)
{
	m_bAllowMoveX = m_bAllowMoveY = bAllow;
}

void CResizableControl::AllowMoveXOnResize()
{
	m_bAllowMoveX = TRUE;
}

void CResizableControl::AllowMoveYOnResize()
{
	m_bAllowMoveY = TRUE;
}

void CResizableControl::AllowResizeOnResize(BOOL bAllow)
{
	m_bAllowResizeX= m_bAllowResizeY = bAllow;
}

void CResizableControl::AllowResizeXOnResize()
{
	m_bAllowResizeX = TRUE;
}

void CResizableControl::AllowResizeYOnResize()
{
	m_bAllowResizeY = TRUE;
}

void CResizableControl::Init(HWND hCtrl)
{
	// save the initial bounding rects of this control, and its parent
	m_hCtrl = hCtrl;
	if (m_hCtrl)
	{
		::GetWindowRect(m_hCtrl, &m_rect);
		HWND hParent = ::GetParent(m_hCtrl);
		if (hParent)
			::GetWindowRect(hParent,&m_parentrect);
	}
}

void CResizableControl::MoveAndResize()
{
	if (!m_hCtrl) return;

	if ( (m_bAllowMoveX || m_bAllowMoveY) && !(m_bAllowResizeX || m_bAllowResizeY) ) // move the ctrl
	{
		HWND hParent = ::GetParent(m_hCtrl);
		if (hParent)
		{
			::InvalidateRect(m_hCtrl, NULL, TRUE);

			::GetWindowRect(m_hCtrl,&m_rect);
			POINT pt1 = { m_rect.left, m_rect.top };
			POINT pt2 = { m_rect.right, m_rect.bottom };
			::ScreenToClient(hParent, &pt1);
			::ScreenToClient(hParent, &pt2);
			m_rect.left = pt1.x;m_rect.top = pt1.y;
			m_rect.right = pt2.x;m_rect.bottom = pt2.y;

			RECT parentrect;
			::GetWindowRect(hParent,&parentrect);
	
			int x = m_rect.left + (m_bAllowMoveX ? 
				( (parentrect.right - parentrect.left) - (m_parentrect.right - m_parentrect.left)) : 0);
			int y = m_rect.top + (m_bAllowMoveY ? 
				( (parentrect.bottom - parentrect.top) - (m_parentrect.bottom - m_parentrect.top)) : 0);

			m_parentrect = parentrect;

			::SetWindowPos(m_hCtrl, NULL, x,y, 0,0, SWP_NOSIZE);
		}
	}
	else if ( !(m_bAllowMoveX || m_bAllowMoveY) && (m_bAllowResizeX || m_bAllowResizeY) ) // resize the ctrl
	{
		HWND hParent = ::GetParent(m_hCtrl);;
		if (hParent)
		{
			RECT parentrect;
			::GetWindowRect(hParent,&parentrect);
	
			int cx = (m_rect.right-m_rect.left) + (m_bAllowResizeX ? 
				( (parentrect.right - parentrect.left) - (m_parentrect.right - m_parentrect.left)) : 0);
			int cy = (m_rect.bottom-m_rect.top) + (m_bAllowResizeY ? 
				( (parentrect.bottom - parentrect.top) - (m_parentrect.bottom - m_parentrect.top)) : 0);

			::SetWindowPos(m_hCtrl, NULL, 0,0, cx,cy, SWP_NOMOVE);
		}
	}
	else if ( (m_bAllowMoveX || m_bAllowMoveY) && (m_bAllowResizeX || m_bAllowResizeY) ) // move and resize the ctrl
	{
		HWND hParent = ::GetParent(m_hCtrl);;
		if (hParent)		
		{
			if (::IsIconic(hParent) || !::IsWindowVisible(hParent) || !::IsWindowEnabled(hParent))
				return;

			::InvalidateRect(m_hCtrl, NULL, TRUE);

			::GetWindowRect(m_hCtrl,&m_rect);
			POINT pt1 = { m_rect.left, m_rect.top };
			POINT pt2 = { m_rect.right, m_rect.bottom };
			::ScreenToClient(hParent, &pt1);
			::ScreenToClient(hParent, &pt2);
			m_rect.left = pt1.x;m_rect.top = pt1.y;
			m_rect.right = pt2.x;m_rect.bottom = pt2.y;

			RECT parentrect;
			::GetWindowRect(hParent,&parentrect);
	
			int x = m_rect.left + (m_bAllowMoveX ? 
				( (parentrect.right - parentrect.left) - (m_parentrect.right - m_parentrect.left) ) : 0);
			int y = m_rect.top + (m_bAllowMoveY ? 
				( (parentrect.bottom - parentrect.top) - (m_parentrect.bottom - m_parentrect.top) ) : 0);
			int cx = (m_rect.right-m_rect.left) + (m_bAllowResizeX ? 
				( (parentrect.right - parentrect.left) - (m_parentrect.right - m_parentrect.left) ) : 0);
			int cy = (m_rect.bottom-m_rect.top) + (m_bAllowResizeY ? 
				( (parentrect.bottom - parentrect.top) - (m_parentrect.bottom - m_parentrect.top) ) : 0);

			m_parentrect = parentrect;

			::SetWindowPos(m_hCtrl, NULL, x,y, cx,cy, 0);
		}
	}
}










//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CResizableGrip::CResizableGrip()
{
	m_hParent = NULL;

	m_sizeGrip.cx = GetSystemMetrics(SM_CXVSCROLL);
	m_sizeGrip.cy = GetSystemMetrics(SM_CYHSCROLL);

	m_binitialrect = FALSE;
}

CResizableGrip::~CResizableGrip()
{
	// remember to destroy the control dynamic helpers
	for (std::vector<CResizableControl*>::iterator it = m_wndControls.begin(); it != m_wndControls.end(); ++it)
		delete *it;
}

void CResizableGrip::UpdateGripPos()
{
	// size-grip goes bottom right in the client area
	// (any right-to-left adjustment should go here)

	RECT rect;
	::GetClientRect(m_hParent,&rect);

	rect.left = rect.right - m_sizeGrip.cx;
	rect.top = rect.bottom - m_sizeGrip.cy;

	// must stay below other children
	::SetWindowPos(m_wndGrip,HWND_BOTTOM, rect.left, rect.top, 0, 0,
		SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOREPOSITION);

	// maximized windows cannot be resized
	
	

	if ( ::IsZoomed(m_hParent) )
	{
		::EnableWindow(m_wndGrip, FALSE);
		ShowSizeGrip(FALSE);
	}
	else
	{
		::EnableWindow(m_wndGrip, TRUE);
		ShowSizeGrip(TRUE);
	}
}

void CResizableGrip::ShowSizeGrip(BOOL bShow)
{
	::ShowWindow(m_wndGrip, bShow ? SW_SHOW : SW_HIDE);
}


HWND CResizableGrip::GetSafeHwnd()
{
	return ::IsWindow(m_wndGrip) ? m_wndGrip : NULL;
}

HWND CResizableGrip::GetParentHwnd()
{
	return ::IsWindow(m_hParent) ? m_hParent : NULL;
}

#define RSZ_GRIP_OBJ	_T("ResizableGrip")

BOOL CResizableGrip::InitGrip(HWND hParent)
{
	m_hParent = hParent;

	RECT rect = { 0 , 0, m_sizeGrip.cx, m_sizeGrip.cy };

	m_wndGrip = ::CreateWindowEx(0, _T("SCROLLBAR"), 
								(LPCTSTR)NULL, 
								WS_CHILD | WS_CLIPSIBLINGS | SBS_SIZEGRIP,
								rect.left, rect.top, 
								rect.right-rect.left,
								rect.bottom-rect.top,
								hParent,
								(HMENU)0,
								NULL,
								NULL);

	if (m_wndGrip)
	{
		// set a triangular window region
		HRGN rgnGrip, rgn;
		rgn = ::CreateRectRgn(0,0,1,1);
		rgnGrip = ::CreateRectRgnIndirect(&rect);

		for (int y=0; y<m_sizeGrip.cy; y++)
		{
			::SetRectRgn(rgn, 0, y, m_sizeGrip.cx-y, y+1);
			::CombineRgn(rgnGrip, rgnGrip, rgn, RGN_DIFF); 
		}
		::SetWindowRgn(m_wndGrip, rgnGrip, FALSE);

		// subclass control
		::SetProp(m_wndGrip, RSZ_GRIP_OBJ,
			(HANDLE)::GetWindowLongPtr(m_wndGrip, GWLP_WNDPROC));
		::SetWindowLongPtr(m_wndGrip, GWLP_WNDPROC, (LONG)GripWindowProc);

		// force dialog styles (RESIZABLE BORDER, NO FLICKERING)
		::SetWindowLongPtr(hParent, GWL_STYLE, 
			::GetWindowLongPtr(hParent, GWL_STYLE) | WS_THICKFRAME | WS_CLIPCHILDREN);
		

		// update pos
		UpdateGripPos();
		ShowSizeGrip();
	}

	return m_wndGrip!=NULL;
}

LRESULT CResizableGrip::GripWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	WNDPROC oldWndProc = (WNDPROC)::GetProp(hwnd, RSZ_GRIP_OBJ);

	switch (msg)
	{
	case WM_NCHITTEST:

		// choose proper cursor shape
		if (IsRTL(hwnd))
			return HTBOTTOMLEFT;
		else
			return HTBOTTOMRIGHT;

	case WM_DESTROY:
		
		// unsubclass
		::RemoveProp(hwnd, RSZ_GRIP_OBJ);
		::SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG)oldWndProc);

		break;
	}

	return ::CallWindowProc(oldWndProc, hwnd, msg, wParam, lParam);
}



CResizableControl *CResizableGrip::AddDynamicControls()
{
	CResizableControl *pControl = new CResizableControl();
	if (pControl) m_wndControls.push_back( pControl );
	return pControl;
}


void CResizableGrip::RemoveDynamicControls(CResizableControl *p)
{
	for (std::vector<CResizableControl*>::iterator it = m_wndControls.begin(); it != m_wndControls.end(); ++it)
	{
		if (*it && *it==p)
		{
			m_wndControls.erase(it);
			delete *it;
			return;
		}
	}
}

void CResizableGrip::MoveAndResize()
{
	for (std::vector<CResizableControl*>::iterator it = m_wndControls.begin(); it != m_wndControls.end(); ++it)
	{
		(*it)->MoveAndResize();
	}
}

