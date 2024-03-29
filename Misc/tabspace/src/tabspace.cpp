﻿// tabspace - converts tabs to spaces and vice-versa in multiple files

// Copyright (C) 2011-2013, 2017, 2021-2022 - Stefan Kueng

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
#include "DirFileEnum.h"
#include "CmdLineParser.h"
#include "TextFile.h"
#include "ConvertTabSpaces.h"
#include "StringUtils.h"
#include <string>
#include <set>
#include <algorithm>
#include <cctype>
#include <io.h>
#include <fcntl.h>
#include <Shlwapi.h>

#pragma comment(lib, "Shlwapi.lib")

std::set<std::wstring> g_allowedPatterns;
std::set<std::wstring> g_excludedPatterns;

bool                   FileExtensionInPattern(const std::wstring& filepath)
{
    std::wstring lowercasepath = filepath;
    std::transform(lowercasepath.begin(), lowercasepath.end(), lowercasepath.begin(), ::towlower);

    std::wstring           lowercasename = lowercasepath;
    std::string::size_type lastPos       = lowercasepath.find_last_of('\\');
    if (lastPos != std::wstring::npos)
        lowercasename = lowercasepath.substr(lastPos + 1);

    for (auto it = g_excludedPatterns.cbegin(); it != g_excludedPatterns.cend(); ++it)
    {
        if (it->find('\\') == std::wstring::npos)
        {
            if (wcswildcmp(it->c_str(), lowercasename.c_str()))
                return false;
        }
        else if (wcswildcmp(it->c_str(), lowercasepath.c_str()))
            return false;
    }
    for (auto it = g_allowedPatterns.cbegin(); it != g_allowedPatterns.cend(); ++it)
    {
        if (it->find('\\') == std::wstring::npos)
        {
            if (wcswildcmp(it->c_str(), lowercasename.c_str()))
                return true;
        }
        else if (wcswildcmp(it->c_str(), lowercasepath.c_str()))
            return true;
    }

    return false;
}

int _tmain(int argc, _TCHAR* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);
    // options and their default values
    bool           bCheckOnly            = false;
    bool           bUseSpaces            = true;
    int            tabsize               = 4;
    bool           bRemoveEOLWhitespaces = true;
    bool           bExtPatterns          = true;
    bool           bCStyle               = false;
    std::wstring   filepattern           = L"c;cc;cpp;cs;cxx;h;hpp;hxx";

    LPWSTR         lpCmdLine             = GetCommandLine();
    CCmdLineParser parser(lpCmdLine);

    if (parser.HasVal(L"unicode"))
        _setmode(_fileno(stdout), _O_U16TEXT);

    if (parser.HasKey(L"?") || parser.HasKey(L"help"))
    {
        // print help text and get out
        _fputts(L"usage:\n", stdout);
        _fputts(L"tabspace [/path:\"path\\to\\check\"] [/checkonly] [/usetabs] [/tabsize:4] [/leaveeol] [/ext:\"extension;list\"]\n", stdout);
        _fputts(L"/path      : the path to scan (file or dir), defaults to the current directory\n", stdout);
        _fputts(L"/checkonly : if specified, the files are not modified but only an info is shown\n", stdout);
        _fputts(L"/usetabs   : convert spaces to tabs instead of tabs to spaces\n", stdout);
        _fputts(L"/tabsize   : specifies the tab size, defaults to 4\n", stdout);
        _fputts(L"/leaveeol  : if specified, whitespaces at the end of lines are not removed\n", stdout);
        _fputts(L"/cstyle    : if specified, whitespaces inside C/C++ strings are ignored\n", stdout);
        _fputts(L"/ext       : a list of file extensions to scan, other extensions are ignored.\n", stdout);
        _fputts(L"             defaults to \"c;cc;cpp;cs;cxx;h;hpp;hxx\"\n", stdout);
        _fputts(L"             if this is set, /include must not be set!\n", stdout);
        _fputts(L"/include   : a list of patterns to include, separated by ';'\n", stdout);
        _fputts(L"             if this is set, /ext must not be set!\n", stdout);
        _fputts(L"             for example \"c:\\sub1\\*.*;*\\sub2\\*.cpp\"\n", stdout);
        _fputts(L"/exclude   : a list of patterns to ignore, separated by ';'\n", stdout);
        _fputts(L"             for example \"c:\\sub1\\*.*;*\\sub2\\*.cpp\"\n", stdout);
        _fputts(L"/unicode   : switches the console output to unicode\n", stdout);
        return 0;
    }

    TCHAR cwd[MAX_PATH] = {0};
    GetCurrentDirectory(_countof(cwd), cwd);

    if (parser.HasVal(L"path"))
    {
        _tcscpy_s(cwd, _countof(cwd), parser.GetVal(L"path"));
        auto attributes = GetFileAttributes(cwd);
        if (attributes == INVALID_FILE_ATTRIBUTES)
        {
            _fputts(L"error: the path \"", stderr);
            _fputts(cwd, stderr);
            _fputts(L"\" does not exist!", stderr);
            return -1;
        }
    }
    if (parser.HasVal(L"ext"))
    {
        if (parser.HasVal(L"include"))
        {
            _fputts(L"error: both /ext and /include are set", stderr);
            return -1;
        }
        filepattern = parser.GetVal(L"ext");
    }
    if (parser.HasVal(L"include"))
    {
        if (parser.HasVal(L"ext"))
        {
            _fputts(L"error: both /ext and /include are set", stderr);
            return -1;
        }
        bExtPatterns = false;
        filepattern  = parser.GetVal(L"include");
    }
    if (parser.HasKey(L"checkonly"))
        bCheckOnly = true;
    if (parser.HasKey(L"usetabs"))
        bUseSpaces = false;
    if (parser.HasVal(L"tabsize"))
        tabsize = parser.GetLongVal(L"tabsize");
    if (parser.HasKey(L"leaveeol"))
        bRemoveEOLWhitespaces = false;
    if (parser.HasKey(L"cstyle"))
        bCStyle = true;

    // split the file extension string into the extensions
    {
        g_allowedPatterns.clear();
        // skip delimiters at beginning.
        std::string::size_type lastPos = filepattern.find_first_not_of(L";", 0);

        // find first "non-delimiter".
        std::string::size_type pos     = filepattern.find_first_of(L";", lastPos);

        while (std::string::npos != pos || std::string::npos != lastPos)
        {
            // found a token, add it to the set.
            std::wstring ext = filepattern.substr(lastPos, pos - lastPos);
            std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);
            if (bExtPatterns)
                ext = L"*." + ext;
            g_allowedPatterns.insert(ext);

            // skip delimiters. Note the "not_of"
            lastPos = filepattern.find_first_not_of(L";", pos);

            // find next "non-delimiter"
            pos     = filepattern.find_first_of(L";", lastPos);
        }
    }
    // split the file extension string into the extensions
    {
        if (parser.HasVal(L"exclude"))
        {
            filepattern = parser.GetVal(L"exclude");
            g_excludedPatterns.clear();
            // skip delimiters at beginning.
            std::string::size_type lastPos = filepattern.find_first_not_of(L";", 0);

            // find first "non-delimiter".
            std::string::size_type pos     = filepattern.find_first_of(L";", lastPos);

            while (std::string::npos != pos || std::string::npos != lastPos)
            {
                // found a token, add it to the set.
                std::wstring ext = filepattern.substr(lastPos, pos - lastPos);
                std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);
                g_excludedPatterns.insert(ext);

                // skip delimiters. Note the "not_of"
                lastPos = filepattern.find_first_not_of(L";", pos);

                // find next "non-delimiter"
                pos     = filepattern.find_first_of(L";", lastPos);
            }
        }
    }

    if (PathIsDirectory(cwd))
    {
        CDirFileEnum filelister((cwd));

        std::wstring filepath;
        bool         bIsDir = false;
        while (filelister.NextFile(filepath, &bIsDir, true))
        {
            if (!bIsDir)
            {
                if (!FileExtensionInPattern(filepath))
                    continue;

                CTextFile              file;
                CTextFile::UnicodeType ut;
                volatile LONG          cancelled = FALSE;
                if (file.Load(filepath.c_str(), ut, false, &cancelled))
                {
                    if (ConvertTabSpaces::Convert(file, bUseSpaces, tabsize, bCheckOnly, bCStyle))
                    {
                        // the file was modified, we have to reload it for the next conversion
                        file.Save(filepath.c_str());
                        file.Load(filepath.c_str(), ut, false, &cancelled);
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
                    _fputts(L"error: could not load file '", stderr);
                    _fputts(filepath.c_str(), stderr);
                    _fputts(L"'\n", stderr);
                }
            }
        }
    }
    else
    {
        std::wstring           filepath = cwd;
        CTextFile              file;
        CTextFile::UnicodeType ut;
        volatile LONG          cancelled = FALSE;
        if (file.Load(filepath.c_str(), ut, false, &cancelled))
        {
            if (ConvertTabSpaces::Convert(file, bUseSpaces, tabsize, bCheckOnly, bCStyle))
            {
                // the file was modified, we have to reload it for the next conversion
                file.Save(filepath.c_str());
                file.Load(filepath.c_str(), ut, false, &cancelled);
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
            _fputts(L"error: could not load file '", stderr);
            _fputts(filepath.c_str(), stderr);
            _fputts(L"'\n", stderr);
        }
    }

    return 0;
}
