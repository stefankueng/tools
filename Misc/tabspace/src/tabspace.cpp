// tabspace.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "DirFileEnum.h"
#include "CmdLineParser.h"
#include "TextFile.h"
#include "ConvertTabSpaces.h"
#include <string>
#include <set>
#include <algorithm>
#include <cctype>

using namespace std;

set<wstring> g_allowedPatterns;

bool FileExtensionInPattern(const wstring& filepath)
{
    const TCHAR * pFound = _tcsrchr(filepath.c_str(), '.');
    wstring ext;
    if (pFound)
        ext = pFound+1;

    std::transform(ext.begin(), ext.end(), ext.begin(), std::tolower);

    return (g_allowedPatterns.find(ext) != g_allowedPatterns.end());
}

int _tmain(int argc, _TCHAR* argv[])
{
    // options and their default values
    bool    bCheckOnly              =   false;
    bool    bUseSpaces              =   true;
    int     tabsize                 =   4;
    bool    bRemoveEOLWhitespaces   =   true;
    wstring filepattern             =   _T("c;cpp;cxx;cc;h;hpp;hxx;cs");


    LPWSTR lpCmdLine = GetCommandLine();
    CCmdLineParser parser(lpCmdLine);

    if (parser.HasKey(_T("?")) || parser.HasKey(_T("help")))
    {
        // print help text and get out
        _fputts(_T("usage:\n"), stdout);
        _fputts(_T("tabspace [/path:\"path\\to\\check\"] [/checkonly] [/usetabs] [/tabsize:4] [/leaveeol] [/ext:\"ext;ension;list\"\n"), stdout);
        _fputts(_T("/path      : the path to scan, defaults to the current directory\n"), stdout);
        _fputts(_T("/checkonly : if specified, the files are not modified but only an info is shown\n"), stdout);
        _fputts(_T("/usetabs   : convert spaces to tabs instead of tabs to spaces\n"), stdout);
        _fputts(_T("/tabsize   : specifies the tab size, defaults to 4\n"), stdout);
        _fputts(_T("/leaveol   : if specified, whitespaces at the end of lines are not removed\n"), stdout);
        _fputts(_T("/ext       : a list of file extensions to scan, other extensions are ignored.\n"), stdout);
        _fputts(_T("             defaults to \"c;cpp;cxx;cc;h;hpp;hxx;cs\"\n"), stdout);
        return 0;
    }


    TCHAR cwd[MAX_PATH] = {0};
    GetCurrentDirectory(MAX_PATH, cwd);

    if (parser.HasVal(_T("path")))
    {
        _tcscpy_s(cwd, MAX_PATH, parser.GetVal(_T("path")));
    }
    if (parser.HasVal(_T("ext")))
    {
        filepattern = parser.GetVal(_T("ext"));
    }
    if (parser.HasKey(_T("checkonly")))
        bCheckOnly = true;
    if (parser.HasKey(_T("usetabs")))
        bUseSpaces = false;
    if (parser.HasVal(_T("tabsize")))
        tabsize = parser.GetLongVal(_T("tabsize"));
    if (parser.HasKey(_T("leaveeol")))
        bRemoveEOLWhitespaces = false;

    // split the file extension string into the extensions
    {
        g_allowedPatterns.clear();
        // skip delimiters at beginning.
        string::size_type lastPos = filepattern.find_first_not_of(_T(";"), 0);

        // find first "non-delimiter".
        string::size_type pos = filepattern.find_first_of(_T(";"), lastPos);

        while (string::npos != pos || string::npos != lastPos)
        {
            // found a token, add it to the set.
            wstring ext = filepattern.substr(lastPos, pos - lastPos);
            std::transform(ext.begin(), ext.end(), ext.begin(), std::tolower);
            g_allowedPatterns.insert(ext);

            // skip delimiters. Note the "not_of"
            lastPos = filepattern.find_first_not_of(_T(";"), pos);

            // find next "non-delimiter"
            pos = filepattern.find_first_of(_T(";"), lastPos);
        }
    }

    CDirFileEnum filelister((cwd));

    wstring filepath;
    bool bIsDir = false;
    while (filelister.NextFile(filepath, &bIsDir, true))
    {
        if (!bIsDir)
        {
            if (!FileExtensionInPattern(filepath))
                continue;

            CTextFile file;
            if (file.Load(filepath.c_str()))
            {
                if (ConvertTabSpaces::Convert(file, bUseSpaces, tabsize, bCheckOnly))
                {
                    // the file was modified, we have to reload it for the next conversion
                    file.Save(filepath.c_str());
                    file.Load(filepath.c_str());
                }
                if (bRemoveEOLWhitespaces)
                {
                    if (ConvertTabSpaces::RemoveEndSpaces(file, bCheckOnly))
                    {
                        file.Save(filepath.c_str());
                    }
                }
            }
            else
            {
                _fputts(_T("error: could not load file '"), stderr);
                _fputts(filepath.c_str(), stderr);
                _fputts(_T("'\n"), stderr);
            }
        }
    }


    return 0;
}

