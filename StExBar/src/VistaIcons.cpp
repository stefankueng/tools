// StExBar - an explorer toolbar

// Copyright (C) 2009, 2012, 2020 - Stefan Kueng

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
#include "VistaIcons.h"

HRESULT Create32BitHBITMAP(HDC hdc, const SIZE *psize, __deref_opt_out void **ppvBits, __out HBITMAP *phBmp);
HRESULT ConvertBufferToPARGB32(HPAINTBUFFER hPaintBuffer, HDC hdc, HICON hicon, SIZE &sizIcon);
bool    HasAlpha(__in ARGB *pargb, SIZE &sizImage, int cxRow);
HRESULT ConvertToPARGB32(HDC hdc, __inout ARGB *pargb, HBITMAP hbmp, SIZE &sizImage, int cxRow);

typedef HRESULT(WINAPI *FN_GetBufferedPaintBits)(HPAINTBUFFER hBufferedPaint, RGBQUAD **ppbBuffer, int *pcxRow);
typedef HPAINTBUFFER(WINAPI *FN_BeginBufferedPaint)(HDC hdcTarget, const RECT *prcTarget, BP_BUFFERFORMAT dwFormat, BP_PAINTPARAMS *pPaintParams, HDC *phdc);
typedef HRESULT(WINAPI *FN_EndBufferedPaint)(HPAINTBUFFER hBufferedPaint, BOOL fUpdateTarget);

FN_GetBufferedPaintBits pfnGetBufferedPaintBits = NULL;
FN_BeginBufferedPaint   pfnBeginBufferedPaint   = NULL;
FN_EndBufferedPaint     pfnEndBufferedPaint     = NULL;

HBITMAP IconToBitmapPARGB32(HICON hIcon)
{
    if (!hIcon)
        return NULL;

    pfnGetBufferedPaintBits = NULL;
    pfnBeginBufferedPaint   = NULL;
    pfnEndBufferedPaint     = NULL;

    HMODULE hUxTheme = LoadLibrary(L"UXTHEME.DLL");

    if (hUxTheme)
    {
        pfnGetBufferedPaintBits = (FN_GetBufferedPaintBits)::GetProcAddress(hUxTheme, "GetBufferedPaintBits");
        pfnBeginBufferedPaint   = (FN_BeginBufferedPaint)::GetProcAddress(hUxTheme, "BeginBufferedPaint");
        pfnEndBufferedPaint     = (FN_EndBufferedPaint)::GetProcAddress(hUxTheme, "EndBufferedPaint");
    }

    if (pfnBeginBufferedPaint == NULL || pfnEndBufferedPaint == NULL || pfnGetBufferedPaintBits == NULL)
    {
        FreeLibrary(hUxTheme);
        return NULL;
    }

    SIZE sizIcon;
    sizIcon.cx = GetSystemMetrics(SM_CXSMICON);
    sizIcon.cy = GetSystemMetrics(SM_CYSMICON);

    RECT rcIcon;
    SetRect(&rcIcon, 0, 0, sizIcon.cx, sizIcon.cy);
    HBITMAP hBmp = NULL;

    HDC hdcDest = CreateCompatibleDC(NULL);
    if (hdcDest)
    {
        if (SUCCEEDED(Create32BitHBITMAP(hdcDest, &sizIcon, NULL, &hBmp)))
        {
            HBITMAP hbmpOld = (HBITMAP)SelectObject(hdcDest, hBmp);
            if (hbmpOld)
            {
                BLENDFUNCTION  bfAlpha     = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
                BP_PAINTPARAMS paintParams = {0};
                paintParams.cbSize         = sizeof(paintParams);
                paintParams.dwFlags        = BPPF_ERASE;
                paintParams.pBlendFunction = &bfAlpha;

                HDC          hdcBuffer;
                HPAINTBUFFER hPaintBuffer = pfnBeginBufferedPaint(hdcDest, &rcIcon, BPBF_DIB, &paintParams, &hdcBuffer);
                if (hPaintBuffer)
                {
                    if (DrawIconEx(hdcBuffer, 0, 0, hIcon, sizIcon.cx, sizIcon.cy, 0, NULL, DI_NORMAL))
                    {
                        // If icon did not have an alpha channel we need to convert buffer to PARGB
                        ConvertBufferToPARGB32(hPaintBuffer, hdcDest, hIcon, sizIcon);
                    }

                    // This will write the buffer contents to the destination bitmap
                    pfnEndBufferedPaint(hPaintBuffer, TRUE);
                }

                SelectObject(hdcDest, hbmpOld);
            }
        }

        DeleteDC(hdcDest);
    }

    FreeLibrary(hUxTheme);

    return hBmp;
}

HRESULT Create32BitHBITMAP(HDC hdc, const SIZE *psize, __deref_opt_out void **ppvBits, __out HBITMAP *phBmp)
{
    *phBmp = NULL;

    BITMAPINFO bmi;
    SecureZeroMemory(&bmi, sizeof(bmi));
    bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biCompression = BI_RGB;

    bmi.bmiHeader.biWidth    = psize->cx;
    bmi.bmiHeader.biHeight   = psize->cy;
    bmi.bmiHeader.biBitCount = 32;

    HDC hdcUsed = hdc ? hdc : GetDC(NULL);
    if (hdcUsed)
    {
        *phBmp = CreateDIBSection(hdcUsed, &bmi, DIB_RGB_COLORS, ppvBits, NULL, 0);
        if (hdc != hdcUsed)
        {
            ReleaseDC(NULL, hdcUsed);
        }
    }
    return (NULL == *phBmp) ? E_OUTOFMEMORY : S_OK;
}

HRESULT ConvertBufferToPARGB32(HPAINTBUFFER hPaintBuffer, HDC hdc, HICON hicon, SIZE &sizIcon)
{
    RGBQUAD *prgbQuad;
    int      cxRow;
    HRESULT  hr = pfnGetBufferedPaintBits(hPaintBuffer, &prgbQuad, &cxRow);
    if (SUCCEEDED(hr))
    {
        ARGB *pargb = reinterpret_cast<ARGB *>(prgbQuad);
        if (!HasAlpha(pargb, sizIcon, cxRow))
        {
            ICONINFO info;
            if (GetIconInfo(hicon, &info))
            {
                if (info.hbmMask)
                {
                    hr = ConvertToPARGB32(hdc, pargb, info.hbmMask, sizIcon, cxRow);
                }

                DeleteObject(info.hbmColor);
                DeleteObject(info.hbmMask);
            }
        }
    }

    return hr;
}

bool HasAlpha(__in ARGB *pargb, SIZE &sizImage, int cxRow)
{
    ULONG cxDelta = cxRow - sizImage.cx;
    for (ULONG y = sizImage.cy; y; --y)
    {
        for (ULONG x = sizImage.cx; x; --x)
        {
            if (*pargb++ & 0xFF000000)
            {
                return true;
            }
        }

        pargb += cxDelta;
    }

    return false;
}

HRESULT ConvertToPARGB32(HDC hdc, __inout ARGB *pargb, HBITMAP hbmp, SIZE &sizImage, int cxRow)
{
    BITMAPINFO bmi;
    SecureZeroMemory(&bmi, sizeof(bmi));
    bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biCompression = BI_RGB;

    bmi.bmiHeader.biWidth    = sizImage.cx;
    bmi.bmiHeader.biHeight   = sizImage.cy;
    bmi.bmiHeader.biBitCount = 32;

    HRESULT hr     = E_OUTOFMEMORY;
    HANDLE  hHeap  = GetProcessHeap();
    void *  pvBits = HeapAlloc(hHeap, 0, bmi.bmiHeader.biWidth * 4 * bmi.bmiHeader.biHeight);
    if (pvBits)
    {
        hr = E_UNEXPECTED;
        if (GetDIBits(hdc, hbmp, 0, bmi.bmiHeader.biHeight, pvBits, &bmi, DIB_RGB_COLORS) == bmi.bmiHeader.biHeight)
        {
            ULONG cxDelta   = cxRow - bmi.bmiHeader.biWidth;
            ARGB *pargbMask = static_cast<ARGB *>(pvBits);

            for (ULONG y = bmi.bmiHeader.biHeight; y; --y)
            {
                for (ULONG x = bmi.bmiHeader.biWidth; x; --x)
                {
                    if (*pargbMask++)
                    {
                        // transparent pixel
                        *pargb++ = 0;
                    }
                    else
                    {
                        // opaque pixel
                        *pargb++ |= 0xFF000000;
                    }
                }

                pargb += cxDelta;
            }

            hr = S_OK;
        }

        HeapFree(hHeap, 0, pvBits);
    }

    return hr;
}

HBITMAP IconToBitmap(HICON hIcon)
{
    RECT rect;

    rect.right  = ::GetSystemMetrics(SM_CXMENUCHECK);
    rect.bottom = ::GetSystemMetrics(SM_CYMENUCHECK);

    rect.left = rect.top = 0;

    HWND desktop = ::GetDesktopWindow();
    if (desktop == NULL)
    {
        return NULL;
    }

    HDC screen_dev = ::GetDC(desktop);
    if (screen_dev == NULL)
    {
        return NULL;
    }

    // Create a compatible DC
    HDC dst_hdc = ::CreateCompatibleDC(screen_dev);
    if (dst_hdc == NULL)
    {
        ::ReleaseDC(desktop, screen_dev);
        return NULL;
    }

    // Create a new bitmap of icon size
    HBITMAP bmp = ::CreateCompatibleBitmap(screen_dev, rect.right, rect.bottom);
    if (bmp == NULL)
    {
        ::DeleteDC(dst_hdc);
        ::ReleaseDC(desktop, screen_dev);
        return NULL;
    }

    // Select it into the compatible DC
    HBITMAP old_dst_bmp = (HBITMAP)::SelectObject(dst_hdc, bmp);
    if (old_dst_bmp == NULL)
    {
        return NULL;
    }

    // Fill the background of the compatible DC with the white color
    // that is taken by menu routines as transparent
    ::SetBkColor(dst_hdc, RGB(255, 255, 255));
    ::ExtTextOut(dst_hdc, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);

    // Draw the icon into the compatible DC
    ::DrawIconEx(dst_hdc, 0, 0, hIcon, rect.right, rect.bottom, 0, NULL, DI_NORMAL);

    // Restore settings
    ::SelectObject(dst_hdc, old_dst_bmp);
    ::DeleteDC(dst_hdc);
    ::ReleaseDC(desktop, screen_dev);

    return bmp;
}
