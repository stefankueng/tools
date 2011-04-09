#pragma once
#include <string>

class CAeroColors
{
public:
    CAeroColors(void);
    ~CAeroColors(void);

    std::wstring AdjustColorsFromWallpaper();

private:
    std::wstring oldWallpaperPath;
    FILETIME oldWallPaperDate;
};

