// StExBar - an explorer toolbar

// Copyright (C) 2008 - Stefan Kueng

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
#include "StdAfx.h"
#include "unicodeutils.h"


string WideToMultibyte(const wstring& wide)
{
	char * narrow = new char[wide.length()*3+2];
	BOOL defaultCharUsed;
	int ret = (int)WideCharToMultiByte(CP_ACP, 0, wide.c_str(), (int)wide.size(), narrow, (int)wide.length()*3 - 1, ".", &defaultCharUsed);
	narrow[ret] = 0;
	string str = narrow;
	delete[] narrow;
	return str;
}

string WideToUTF8(const wstring& wide)
{
	char * narrow = new char[wide.length()*3+2];
	int ret = (int)WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), (int)wide.size(), narrow, (int)wide.length()*3 - 1, NULL, NULL);
	narrow[ret] = 0;
	string str = narrow;
	delete[] narrow;
	return str;
}

wstring MultibyteToWide(const string& multibyte)
{
	size_t length = multibyte.length();
	if (length == 0)
		return wstring();

	wchar_t * wide = new wchar_t[multibyte.length()*2+2];
	if (wide == NULL)
		return wstring();
	int ret = (int)MultiByteToWideChar(CP_ACP, 0, multibyte.c_str(), (int)multibyte.size(), wide, (int)length*2 - 1);
	wide[ret] = 0;
	wstring str = wide;
	delete[] wide;
	return str;
}

wstring UTF8ToWide(const string& multibyte)
{
	size_t length = multibyte.length();
	if (length == 0)
		return wstring();

	wchar_t * wide = new wchar_t[length*2+2];
	if (wide == NULL)
		return wstring();
	int ret = (int)MultiByteToWideChar(CP_UTF8, 0, multibyte.c_str(), (int)multibyte.size(), wide, (int)length*2 - 1);
	wide[ret] = 0;
	wstring str = wide;
	delete[] wide;
	return str;
}
