// tabspace - converts tabs to spaces and vice-versa in multiple files

// Copyright (C) 2011-2013, 2017 - Stefan Kueng

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
#include "ConvertTabSpaces.h"
#include <iostream>

ConvertTabSpaces::ConvertTabSpaces(void)
{
}

ConvertTabSpaces::~ConvertTabSpaces(void)
{
}

bool ConvertTabSpaces::Convert(CTextFile& file, bool useSpaces, int tabsize, bool checkonly, bool bCStyle)
{
    if (!checkonly)
    {
        // if we find a violation of the rule, we fix the file
        if (!useSpaces)
        {
            // tabify the file
            // first find out how many spaces we have to convert into tabs
            int count = 0;
            int spacecount = 0;
            std::vector<long> spacegrouppositions;
            long pos = 0;
            bool bHasSpacesToConvert = false;
            if (file.GetEncoding() == CTextFile::UNICODE_LE)
            {
                bool inChar = false;
                bool inString = false;
                bool escapeChar = false;
                for (std::wstring::const_iterator it = file.GetFileString().begin(); it != file.GetFileString().end(); ++it, ++pos)
                {
                    if (bCStyle)
                    {
                        if (escapeChar)
                        {
                            escapeChar = false;
                            continue;
                        }
                        if (*it == '\\')
                            escapeChar = true;
                        if (!inString && (*it == '\''))
                            inChar = !inChar;
                        if ((!inChar) && (*it == '\"'))
                            inString = !inString;
                        if ((*it == '\n') || (*it == '\r'))
                            inChar = false;
                        if (inChar || inString)
                        {
                            spacecount = 0;
                            continue;
                        }
                    }

                    // we have to convert all spaces in groups of more than the tabsize
                    // a space followed by a tab may lead to just removing the space
                    if ((*it == ' ') || (*it == '\t'))
                    {
                        spacecount++;
                        if ((spacecount == tabsize) || ((*it == '\t') && (spacecount > 1)))
                        {
                            spacegrouppositions.push_back(pos - spacecount + 1);
                            count += (spacecount - 1);
                            bHasSpacesToConvert = true;
                            spacecount = 0;
                        }
                        if (*it == '\t')
                            spacecount = 0;
                    }
                    else
                        spacecount = 0;
                }
            }
            else
            {
                bool inChar = false;
                bool inString = false;
                bool escapeChar = false;
                char * pBuf = (char*)file.GetFileContent();
                for (int i = 0; i < file.GetFileLength(); ++i, ++pos, ++pBuf)
                {
                    if (bCStyle)
                    {
                        if (escapeChar)
                        {
                            escapeChar = false;
                            continue;
                        }
                        if (*pBuf == '\\')
                            escapeChar = true;
                        if (!inString && (*pBuf == '\''))
                            inChar = !inChar;
                        if ((!inChar) && (*pBuf == '\"'))
                            inString = !inString;
                        if ((*pBuf == '\n') || (*pBuf == '\r'))
                            inChar = false;
                        if (inChar || inString)
                        {
                            spacecount = 0;
                            continue;
                        }
                    }

                    if ((*pBuf == ' ') || (*pBuf == '\t'))
                    {
                        spacecount++;
                        if ((spacecount == tabsize) || ((*pBuf == '\t') && (spacecount > 1)))
                        {
                            spacegrouppositions.push_back(pos - spacecount + 1);
                            count += (spacecount - 1);
                            bHasSpacesToConvert = true;
                            spacecount = 0;
                        }
                        if (*pBuf == '\t')
                            spacecount = 0;
                    }
                    else
                        spacecount = 0;
                }
            }
            // now we have the number of space groups we have to convert to tabs
            // create a new file buffer and copy everything over there, replacing those space
            // groups with tabs.
            if (bHasSpacesToConvert)
            {
                if (file.GetEncoding() == CTextFile::UNICODE_LE)
                {
                    long newfilelen = file.GetFileLength();
                    newfilelen -= (count * sizeof(WCHAR));
                    WCHAR * pBuf = new WCHAR[newfilelen / sizeof(WCHAR)];
                    WCHAR * pBufStart = pBuf;
                    WCHAR * pOldBuf = (WCHAR*)file.GetFileContent();
                    auto wlength = long(file.GetFileLength() / sizeof(WCHAR));
                    if (file.HasBOM())
                    {
                        *pBuf++ = *pOldBuf++;
                        --wlength;
                    }
                    std::vector<long>::iterator it = spacegrouppositions.begin();
                    for (long i = 0; i < wlength; ++i)
                    {
                        if ((it != spacegrouppositions.end()) && (*it == i))
                        {
                            *pBuf++ = '\t';
                            spacecount = 0;
                            while ((spacecount < tabsize) && (*pOldBuf == ' '))
                            {
                                i++;
                                spacecount++;
                                pOldBuf++;
                            }
                            if ((spacecount < tabsize) && (*pOldBuf == '\t'))
                                pBuf--;
                            --i;
                            ++it;
                        }
                        else
                            *pBuf++ = *pOldBuf++;
                    }
                    file.ContentsModified((BYTE*)pBufStart, newfilelen);
                    TCHAR outbuf[MAX_PATH * 2];
                    _stprintf_s(outbuf,
                                _countof(outbuf),
                                L"converted spaces to tabs in file '%s'\n",
                                file.GetFileName().c_str());
                    std::wcout << outbuf;
                    return true;
                }
                else if (file.GetEncoding() != CTextFile::BINARY)
                {
                    long newfilelen = file.GetFileLength();
                    newfilelen -= count;
                    char * pBuf = new char[newfilelen];
                    char * pBufStart = pBuf;
                    char * pOldBuf = (char*)file.GetFileContent();
                    std::vector<long>::iterator it = spacegrouppositions.begin();
                    for (long i = 0; i < (file.GetFileLength()); ++i)
                    {
                        if ((it != spacegrouppositions.end()) && (*it == i))
                        {
                            *pBuf++ = '\t';
                            spacecount = 0;
                            while ((spacecount < tabsize) && (*pOldBuf == ' '))
                            {
                                i++;
                                spacecount++;
                                pOldBuf++;
                            }
                            if ((spacecount < tabsize) && (*pOldBuf == '\t'))
                                pBuf--;
                            --i;
                            ++it;
                        }
                        else
                            *pBuf++ = *pOldBuf++;
                    }
                    file.ContentsModified((BYTE*)pBufStart, newfilelen);
                    TCHAR outbuf[MAX_PATH * 2];
                    _stprintf_s(outbuf,
                                _countof(outbuf),
                                L"converted spaces to tabs in file '%s'\n",
                                file.GetFileName().c_str());
                    std::wcout << outbuf;
                    return true;
                }
            }
        }
        else
        {
            // untabify the file
            // first find the number of spaces we have to insert.
            long pos = 0;
            long inlinepos = 0;
            long spacestoinsert = 0;
            bool bhasTabs = false;
            if (file.GetEncoding() == CTextFile::UNICODE_LE)
            {
                bool inChar = false;
                bool inString = false;
                bool escapeChar = false;
                for (std::wstring::const_iterator it = file.GetFileString().begin(); it != file.GetFileString().end(); ++it, ++pos)
                {
                    ++inlinepos;
                    if (bCStyle)
                    {
                        if (escapeChar)
                        {
                            escapeChar = false;
                            continue;
                        }
                        if (*it == '\\')
                            escapeChar = true;
                        if (!inString && (*it == '\''))
                            inChar = !inChar;
                        if ((!inChar) && (*it == '\"'))
                            inString = !inString;
                        if ((*it == '\n') || (*it == '\r'))
                            inChar = false;
                        if (inChar || inString)
                            continue;
                    }

                    if ((*it == '\r') || (*it == '\n'))
                        inlinepos = 0;
                    // we have to convert all tabs
                    if (*it == '\t')
                    {
                        inlinepos += tabsize - 1;
                        long inlinepostemp = tabsize - ((inlinepos + tabsize) % tabsize);
                        if (inlinepostemp == 0)
                            inlinepostemp = tabsize;
                        spacestoinsert += (inlinepostemp - 1);      // minus one because the tab itself gets replaced
                        inlinepos += inlinepostemp;
                        bhasTabs = true;
                    }
                }
            }
            else
            {
                bool inChar = false;
                bool inString = false;
                bool escapeChar = false;
                char * pBuf = (char*)file.GetFileContent();
                for (int i = 0; i < file.GetFileLength(); ++i, ++pos, ++pBuf)
                {
                    ++inlinepos;
                    if (bCStyle)
                    {
                        if (escapeChar)
                        {
                            escapeChar = false;
                            continue;
                        }
                        if (*pBuf == '\\')
                            escapeChar = true;
                        if (!inString && (*pBuf == '\''))
                            inChar = !inChar;
                        if ((!inChar) && (*pBuf == '\"'))
                            inString = !inString;
                        if ((*pBuf == '\n') || (*pBuf == '\r'))
                            inChar = false;
                        if (inChar || inString)
                            continue;
                    }

                    if ((*pBuf == '\r') || (*pBuf == '\n'))
                        inlinepos = 0;
                    // we have to convert all tabs
                    if (*pBuf == '\t')
                    {
                        inlinepos += tabsize - 1;
                        long inlinepostemp = tabsize - ((inlinepos + tabsize) % tabsize);
                        if (inlinepostemp == 0)
                            inlinepostemp = tabsize;
                        spacestoinsert += (inlinepostemp - 1);      // minus one because the tab itself gets replaced
                        inlinepos += inlinepostemp;
                        bhasTabs = true;
                    }
                }
            }
            if (bhasTabs)
            {
                inlinepos = 0;
                if (file.GetEncoding() == CTextFile::UNICODE_LE)
                {
                    long newfilelen = file.GetFileLength() + (spacestoinsert * sizeof(WCHAR));
                    WCHAR * pBuf = new WCHAR[newfilelen / sizeof(WCHAR)];
                    WCHAR * pBufStart = pBuf;
                    WCHAR * pOldBuf = (WCHAR*)file.GetFileContent();
                    auto wlength = long(file.GetFileLength() / sizeof(WCHAR));
                    if (file.HasBOM())
                    {
                        *pBuf++ = *pOldBuf++;
                        --wlength;
                    }
                    bool inChar = false;
                    bool inString = false;
                    bool escapeChar = false;
                    for (long i = 0; i < wlength; ++i)
                    {
                        ++inlinepos;
                        if (bCStyle)
                        {
                            if (escapeChar)
                            {
                                escapeChar = false;
                                *pBuf++ = *pOldBuf++;
                                continue;
                            }
                            if (*pOldBuf == '\\')
                                escapeChar = true;
                            if (!inString && (*pOldBuf == '\''))
                                inChar = !inChar;
                            if ((!inChar) && (*pOldBuf == '\"'))
                                inString = !inString;
                            if ((*pOldBuf == '\n') || (*pOldBuf == '\r'))
                                inChar = false;
                            if (inChar || inString)
                            {
                                *pBuf++ = *pOldBuf++;
                                continue;
                            }
                        }

                        if ((*pOldBuf == '\r') || (*pOldBuf == '\n'))
                            inlinepos = 0;
                        if (*pOldBuf == '\t')
                        {
                            long inlinepostemp = tabsize - (((inlinepos - 1) + tabsize) % tabsize);
                            if (inlinepostemp == 0)
                                inlinepostemp = tabsize;
                            inlinepos += (inlinepostemp - 1);
                            for (int j = 0; j < inlinepostemp; ++j)
                                *pBuf++ = ' ';
                            pOldBuf++;
                        }
                        else
                            *pBuf++ = *pOldBuf++;
                    }
                    file.ContentsModified((BYTE*)pBufStart, newfilelen);
                    TCHAR outbuf[MAX_PATH * 2];
                    _stprintf_s(outbuf,
                                _countof(outbuf),
                                L"converted tabs to spaces in file '%s'\n",
                                file.GetFileName().c_str());
                    std::wcout << outbuf;
                    return true;
                }
                else if (file.GetEncoding() != CTextFile::BINARY)
                {
                    long newfilelen = file.GetFileLength() + spacestoinsert;
                    char * pBuf = new char[newfilelen];
                    char * pBufStart = pBuf;
                    char * pOldBuf = (char*)file.GetFileContent();
                    bool inChar = false;
                    bool inString = false;
                    bool escapeChar = false;
                    for (long i = 0; i < file.GetFileLength(); ++i)
                    {
                        ++inlinepos;
                        if (bCStyle)
                        {
                            if (escapeChar)
                            {
                                escapeChar = false;
                                *pBuf++ = *pOldBuf++;
                                continue;
                            }
                            if (*pOldBuf == '\\')
                                escapeChar = true;
                            if (!inString && (*pOldBuf == '\''))
                                inChar = !inChar;
                            if ((!inChar) && (*pOldBuf == '\"'))
                                inString = !inString;
                            if ((*pOldBuf == '\n') || (*pOldBuf == '\r'))
                                inChar = false;
                            if (inChar || inString)
                            {
                                *pBuf++ = *pOldBuf++;
                                continue;
                            }
                        }

                        if ((*pOldBuf == '\r') || (*pOldBuf == '\n'))
                            inlinepos = 0;
                        if (*pOldBuf == '\t')
                        {
                            long inlinepostemp = tabsize - (((inlinepos - 1) + tabsize) % tabsize);
                            if (inlinepostemp == 0)
                                inlinepostemp = tabsize;
                            inlinepos += (inlinepostemp - 1);
                            for (int j = 0; j < inlinepostemp; ++j)
                            {
                                *pBuf++ = ' ';
                            }
                            pOldBuf++;
                        }
                        else
                            *pBuf++ = *pOldBuf++;
                    }
                    file.ContentsModified((BYTE*)pBufStart, newfilelen);
                    TCHAR outbuf[MAX_PATH * 2];
                    _stprintf_s(outbuf,
                                _countof(outbuf),
                                L"converted tabs to spaces in file '%s'\n",
                                file.GetFileName().c_str());
                    std::wcout << outbuf;
                    return true;
                }
            }
        }
    }
    else
    {
        // don't touch the file, only print messages about what we find
        if (!useSpaces)
        {
            // if we're in tab mode, then more or equal spaces than the tabsize in a row are a violation
            // less spaces than the tabsize could be used to align text to a non-tab position and therefore
            // is not a violation.
            size_t pos = 0;
            bool inChar = false;
            bool inString = false;
            bool escapeChar = false;
            for (std::wstring::const_iterator it = file.GetFileString().begin(); it != file.GetFileString().end(); ++it, ++pos)
            {
                if (bCStyle)
                {
                    if (escapeChar)
                    {
                        escapeChar = false;
                        continue;
                    }
                    if (*it == '\\')
                        escapeChar = true;
                    if (!inString && (*it == '\''))
                        inChar = !inChar;
                    if ((!inChar) && (*it == '\"'))
                        inString = !inString;
                    if ((*it == '\n') || (*it == '\r'))
                        inChar = false;
                    if (inChar || inString)
                        continue;
                }

                if (*it == ' ')
                {
                    ++it;
                    ++pos;
                    int count = 1;
                    while ((it != file.GetFileString().end()) && (*it == ' '))
                    {
                        ++count;
                        ++it;
                        ++pos;
                    }
                    if (count >= tabsize)
                    {
                        // we have more or equal spaces in a row than the tabsize is, that's a violation!
                        TCHAR buf[MAX_PATH * 2];
                        _stprintf_s(buf,
                                    _countof(buf),
                                    L"found spaces instead of tabs in file '%s', line %d\n",
                                    file.GetFileName().c_str(),
                                    file.LineFromPosition((long)pos));
                        _fputts(buf, stderr);
                    }
                }
            }
        }
        else
        {
            // in space mode, even one tab is a violation
            size_t pos = 0;
            bool inChar = false;
            bool inString = false;
            bool escapeChar = false;
            for (std::wstring::const_iterator it = file.GetFileString().begin(); it != file.GetFileString().end(); ++it, ++pos)
            {
                if (bCStyle)
                {
                    if (escapeChar)
                    {
                        escapeChar = false;
                        continue;
                    }
                    if (*it == '\\')
                        escapeChar = true;
                    if (!inString && (*it == '\''))
                        inChar = !inChar;
                    if ((!inChar) && (*it == '\"'))
                        inString = !inString;
                    if ((*it == '\n') || (*it == '\r'))
                        inChar = false;
                    if (inChar || inString)
                        continue;
                }

                if (*it == '\t')
                {
                    // we have a tab, that's a violation!
                    TCHAR buf[MAX_PATH * 2];
                    _stprintf_s(buf,
                                _countof(buf),
                                L"found tab instead of spaces in file '%s', line %d\n",
                                file.GetFileName().c_str(),
                                file.LineFromPosition((long)pos));
                    _fputts(buf, stderr);

                    // now skip to the next non-space char
                    while ((it != file.GetFileString().end()) && ((*it == ' ') || (*it == '\t')))
                    {
                        ++it;
                        ++pos;
                    }
                }
            }
        }
    }
    return false;
}

bool ConvertTabSpaces::RemoveEndSpaces(CTextFile& file, bool checkonly)
{
    if (!checkonly)
    {
        // first count the whitespaces we have to remove
        int inlinepos = 0;
        int whitespaces = 0;
        int totalwhitespaces = 0;
        int pos = 0;
        std::vector<long> spacepositions;
        if (file.GetEncoding() == CTextFile::UNICODE_LE)
        {
            for (std::wstring::const_iterator it = file.GetFileString().begin(); it != file.GetFileString().end(); ++it)
            {
                ++inlinepos;
                ++pos;
                if ((*it == '\r') || (*it == '\n'))
                {
                    if (whitespaces)
                    {
                        spacepositions.push_back(pos - whitespaces);
                        totalwhitespaces += whitespaces;
                    }
                    whitespaces = 0;
                    inlinepos = 0;
                }
                if ((*it == ' ') || (*it == '\t'))
                    whitespaces++;
                else
                    whitespaces = 0;
            }
            if (whitespaces)
            {
                // end space but no newline at the end
                spacepositions.push_back(pos - whitespaces + 1);
                totalwhitespaces += whitespaces;
            }
        }
        else
        {
            char * pBuf = (char*)file.GetFileContent();
            for (int i = 0; i < file.GetFileLength(); ++i, ++pBuf)
            {
                ++inlinepos;
                ++pos;
                if ((*pBuf == '\r') || (*pBuf == '\n'))
                {
                    if (whitespaces)
                    {
                        spacepositions.push_back(pos - whitespaces);
                        totalwhitespaces += whitespaces;
                    }
                    whitespaces = 0;
                    inlinepos = 0;
                }
                if ((*pBuf == ' ') || (*pBuf == '\t'))
                    whitespaces++;
                else
                    whitespaces = 0;
            }
            if (whitespaces)
            {
                // end space but no newline at the end
                spacepositions.push_back(pos - whitespaces + 1);
                totalwhitespaces += whitespaces;
            }
        }
        // now we have the amount of whitespaces we have to remove
        if (totalwhitespaces)
        {
            pos = 0;
            if (file.GetEncoding() == CTextFile::UNICODE_LE)
            {
                long newfilelen = file.GetFileLength();
                newfilelen -= (totalwhitespaces * sizeof(WCHAR));
                WCHAR * pBuf = new WCHAR[newfilelen / sizeof(WCHAR)];
                WCHAR * pBufStart = pBuf;
                WCHAR * pOldBuf = (WCHAR*)file.GetFileContent();
                auto wlength = long(file.GetFileLength() / sizeof(WCHAR));
                if (file.HasBOM())
                {
                    *pBuf++ = *pOldBuf++;
                    --wlength;
                }
                std::vector<long>::iterator it = spacepositions.begin();
                for (long i = 0; i < wlength; ++i)
                {
                    ++pos;
                    if ((it != spacepositions.end()) && (pos == *it))
                    {
                        ++it;
                        TCHAR outbuf[MAX_PATH * 2];
                        _stprintf_s(outbuf,
                                    _countof(outbuf),
                                    L"fixed end-of-line whitespaces in file '%s', line %d\n",
                                    file.GetFileName().c_str(),
                                    file.LineFromPosition(pos));
                        std::wcout << outbuf;
                        // now skip the rest of the whitespaces
                        while ((*pOldBuf == ' ') || (*pOldBuf == '\t'))
                        {
                            pOldBuf++;
                            i++;
                            pos++;
                        }
                    }
                    if (i < wlength)
                        *pBuf++ = *pOldBuf++;
                }
                file.ContentsModified((BYTE*)pBufStart, newfilelen);
                return true;
            }
            else
            {
                long newfilelen = file.GetFileLength();
                newfilelen -= totalwhitespaces;
                char * pBuf = new char[newfilelen];
                char * pBufStart = pBuf;
                char * pOldBuf = (char*)file.GetFileContent();
                std::vector<long>::iterator it = spacepositions.begin();
                for (long i = 0; i < long(file.GetFileLength()); ++i)
                {
                    ++pos;
                    if ((it != spacepositions.end()) && (pos == *it))
                    {
                        ++it;
                        TCHAR outbuf[MAX_PATH * 2];
                        _stprintf_s(outbuf,
                                    _countof(outbuf),
                                    L"fixed end-of-line whitespaces in file '%s', line %d\n",
                                    file.GetFileName().c_str(),
                                    file.LineFromPosition(pos));
                        std::wcout << outbuf;
                        // now skip the rest of the whitespaces
                        while ((*pOldBuf == ' ') || (*pOldBuf == '\t'))
                        {
                            pOldBuf++;
                            i++;
                            pos++;
                        }
                    }
                    if (i < long(file.GetFileLength()))
                        *pBuf++ = *pOldBuf++;
                }
                file.ContentsModified((BYTE*)pBufStart, newfilelen);
                return true;
            }
        }
    }
    else
    {
        // only throw out messages when we find end-of-line whitespaces
        int inlinepos = 0;
        int whitespaces = 0;
        int pos = 0;
        for (std::wstring::const_iterator it = file.GetFileString().begin(); it != file.GetFileString().end(); ++it)
        {
            ++inlinepos;
            ++pos;
            if ((*it == '\r') || (*it == '\n'))
            {
                if (whitespaces)
                {
                    TCHAR outbuf[MAX_PATH * 2];
                    _stprintf_s(outbuf,
                                _countof(outbuf),
                                L"end-of-line whitespaces found in file '%s', line %d\n",
                                file.GetFileName().c_str(),
                                file.LineFromPosition(pos));
                    _fputts(outbuf, stderr);
                }
                whitespaces = 0;
                inlinepos = 0;
            }
            if ((*it == ' ') || (*it == '\t'))
                whitespaces++;
            else
                whitespaces = 0;
        }
    }
    return false;
}
