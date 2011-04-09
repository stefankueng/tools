#include "StdAfx.h"
#include "AeroColors.h"

#include <dwmapi.h>
#pragma  comment(lib, "dwmapi.lib")

CAeroColors::CAeroColors(void)
{
    oldWallPaperDate.dwLowDateTime  = 0;
    oldWallPaperDate.dwHighDateTime = 0;
}


CAeroColors::~CAeroColors(void)
{
}

void CAeroColors::AdjustColorsFromWallpaper()
{
    WCHAR wallPaperPath[MAX_PATH] = {0};
    SystemParametersInfo(SPI_GETDESKWALLPAPER, MAX_PATH, wallPaperPath, 0);
    if (wallPaperPath[0] == 0)
    {
        // no wallpaper - no nothing
        return;
    }
    if (oldWallpaperPath.compare(wallPaperPath) == 0)
    {
        HANDLE hFile = CreateFile(wallPaperPath, GENERIC_READ, FILE_SHARE_DELETE|FILE_SHARE_WRITE|FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
        if (hFile != INVALID_HANDLE_VALUE)
        {
            FILETIME create, access, write;
            GetFileTime(hFile, &create, &access, &write);
            CloseHandle(hFile);
            if (CompareFileTime(&oldWallPaperDate, &write) == 0)
                return;
            oldWallPaperDate = write;
        }
        else
            return;
    }
    oldWallpaperPath = wallPaperPath;
    BOOL bDwmEnabled = FALSE;
    if (SUCCEEDED(DwmIsCompositionEnabled(&bDwmEnabled)) && bDwmEnabled)
    {
        Gdiplus::Bitmap * bmp = new Gdiplus::Bitmap(wallPaperPath);
        if (bmp == nullptr)
            return;

        Gdiplus::Bitmap * bitmap = new Gdiplus::Bitmap(1, 1);
        Gdiplus::Graphics * graphics = Gdiplus::Graphics::FromImage(bitmap);
        graphics->SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
        graphics->DrawImage(bmp, Gdiplus::RectF(0, 0, 1, 1));
        Gdiplus::Color clr;
        bitmap->GetPixel(0, 0, &clr);

        typedef struct tagCOLORIZATIONPARAMS
        {
            COLORREF    clr1;
            COLORREF    clr2;
            UINT        nIntensity;
            UINT        nReserved2;
            UINT        nReserved3;
            UINT        nReserved4;
            BOOL        fOpaque;
        } COLORIZATIONPARAMS;

        typedef void (WINAPI *FN_DwmGetColorizationParameters) (COLORIZATIONPARAMS * parameters);
        typedef void (WINAPI *FN_DwmSetColorizationParameters) (COLORIZATIONPARAMS * parameters, BOOL unknown);

        FN_DwmGetColorizationParameters pDwmGetColorizationParameters = (FN_DwmGetColorizationParameters)GetProcAddress(GetModuleHandle(L"dwmapi.dll"), LPCSTR(127));
        FN_DwmSetColorizationParameters pDwmSetColorizationParameters = (FN_DwmSetColorizationParameters)GetProcAddress(GetModuleHandle(L"dwmapi.dll"), LPCSTR(131));
        if (pDwmGetColorizationParameters && pDwmSetColorizationParameters)
        {
            COLORIZATIONPARAMS params = {0};
            pDwmGetColorizationParameters(&params);
            params.clr1 = clr.ToCOLORREF();
            params.clr2 = clr.ToCOLORREF();
            pDwmSetColorizationParameters(&params, 0);
        }

        delete graphics;
        delete bitmap;
        delete bmp;
    }
}
