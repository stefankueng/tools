// StExBar - an explorer toolbar

// Copyright (C) 2011-2012 - Stefan Kueng

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
#include <stdio.h>
#include <wchar.h>
#include <algorithm>
#include <cctype>
#include <map>
#include <vector>
#include <regex>

class NumberReplacer
{
public:
    NumberReplacer()
        : leadzero(false)
        , padding(0)
        , start(1)
        , increment(1)
    {

    }

    bool            leadzero;
    int             padding;
    int             start;
    int             increment;
    std::wstring    expression;
};


class NumberReplaceHandler
{
public:
    NumberReplaceHandler(const std::wstring& sReplace)
        : m_sReplace(sReplace)
    {
        m_incVec.clear();
        // parse for ${count0L}, ${count0L(n)}, ${count0L(n,m)}, where
        // ${count}
        // is replaced later with numbers starting from 1, incremented by 1
        // ${count(n)}
        // is replaced with numbers starting from n, incremented by 1
        // ${count(n,m)}
        // is replaced with numbers starting from n, incremented by m
        // 0 and L are optional and specify the size of the right-aligned
        // number string. If 0 is specified, zeros are used for padding, otherwise spaces.
        //std::wregex expression = std::wregex(L"(?<!\\\\)\\$\\{count(?<leadzero>0)?(?<length>\\d+)?(\\((?<startval>[-0-9]+)\\)||\\((?<startval>[-0-9]+),(?<increment>[-0-9]+)\\))?\\}", std::regex::normal);
        std::wregex expression = std::wregex(L"\\$\\{count(0)?(\\d+)?(\\(([-0-9]+)\\)||\\(([-0-9]+),([-0-9]+)\\))?\\}", std::regex_constants::ECMAScript);
        std::match_results<std::wstring::const_iterator> whatc;
        std::wstring::const_iterator start, end;
        start = m_sReplace.begin();
        end = m_sReplace.end();
        while (std::regex_search(start, end, whatc, expression))
        {
            if (whatc[0].matched)
            {
                NumberReplacer nr;
                nr.leadzero = (((std::wstring)whatc[1]) == L"0");
                nr.padding = _wtoi(((std::wstring)whatc[2]).c_str());
                std::wstring s = (std::wstring)whatc[4];
                if (s.size())
                    nr.start = _wtoi(s.c_str());
                else
                {
                    s = (std::wstring)whatc[5];
                    if (s.size())
                        nr.start = _wtoi(s.c_str());
                }
                s = (std::wstring)whatc[6];
                if (s.size())
                    nr.increment = _wtoi(s.c_str());
                if (nr.increment == 0)
                    nr.increment = 1;
                nr.expression = (std::wstring)whatc[0];
                m_incVec.push_back(nr);
            }
            // update search position:
            if (start == whatc[0].second)
            {
                if (start == end)
                    break;
                ++start;
            }
            else
                start = whatc[0].second;
        }
    }

    std::wstring ReplaceCounters(const std::wstring& sText)
    {
        std::wstring sReplace = sText;
        if (!m_incVec.empty())
        {
            for (auto it = m_incVec.begin(); it != m_incVec.end(); ++it)
            {
                auto it_begin = std::search(sReplace.begin(), sReplace.end(), it->expression.begin(), it->expression.end());
                if (it_begin != sReplace.end())
                {
                    if ((it_begin == sReplace.begin())||((*(it_begin-1)) != '\\'))
                    {
                        auto it_end= it_begin + it->expression.size();
                        wchar_t format[10] = {0};
                        if (it->padding)
                        {
                            if (it->leadzero)
                                swprintf_s(format, _countof(format), L"%%0%dd", it->padding);
                            else
                                swprintf_s(format, _countof(format), L"%%%dd", it->padding);
                        }
                        else
                            wcscpy_s(format, L"%d");
                        wchar_t buf[50] = {0};
                        swprintf_s(buf, _countof(buf), format, it->start);
                        sReplace.replace(it_begin, it_end, buf);
                        it->start += it->increment;
                    }
                    else if ((*(it_begin-1)) == '\\')
                    {
                        sReplace.erase(it_begin-1);
                    };
                }
            }
        }

        return sReplace;
    }

private:
    std::wstring                m_sReplace;
    std::vector<NumberReplacer> m_incVec;
};
