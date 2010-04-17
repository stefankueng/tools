#pragma once
#include "TextFile.h"

class ConvertTabSpaces
{
public:
    ConvertTabSpaces(void);
    ~ConvertTabSpaces(void);

public:
    static bool         Convert(CTextFile& file, bool useSpaces, int tabsize, bool checkonly);
    static bool         RemoveEndSpaces(CTextFile& file, bool checkonly);
};
