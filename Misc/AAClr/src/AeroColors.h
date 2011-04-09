#pragma once
#include <string>

class CAeroColors
{
public:
    CAeroColors(void);
    ~CAeroColors(void);

    void AdjustColorsFromWallpaper();

private:
    std::wstring oldWallpaperPath;
    FILETIME oldWallPaperDate;
};

