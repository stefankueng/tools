// StExBar - an explorer toolbar

// Copyright (C) 2007-2010 - Stefan Kueng

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
#include "Commands.h"
#include "resource.h"

using namespace std;

CCommands::CCommands(void)
{
}

CCommands::~CCommands(void)
{
}

bool CCommands::LoadFromFile()
{
	OSVERSIONINFOEX inf;
	SecureZeroMemory(&inf, sizeof(OSVERSIONINFOEX));
	inf.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	GetVersionEx((OSVERSIONINFO *)&inf);
	WORD fullWinver = MAKEWORD(inf.dwMinorVersion, inf.dwMajorVersion);

	m_commands.clear();
	// fill in the default commands first
	Command c;
	hotkey key;
	const hotkey nokey;
	c.name = _T("StexBar Internal Edit Box");
	c.commandline = INTERNALCOMMAND;
	c.separator = false;
	c.nIconID = 0;
	c.enabled_viewpath = true;
	c.enabled_noviewpath = false;
	c.enabled_fileselected = true;
	c.enabled_folderselected = true;
	c.enabled_selected = true;
	c.enabled_noselection = true;
	c.enabled_selectedcount = 0;
	key.control = true;
	key.shift = false;
	key.alt = false;
	key.keycode = WPARAM('K');
	c.key = key;
	m_commands.push_back(c);

	c.name = _T("Options");
	c.commandline = INTERNALCOMMAND;
	c.separator = false;
	c.nIconID = IDI_OPTIONS;
	c.enabled_viewpath = true;
	c.enabled_noviewpath = true;
	c.enabled_fileselected = true;
	c.enabled_folderselected = true;
	c.enabled_selected = true;
	c.enabled_noselection = true;
	c.enabled_selectedcount = 0;
	c.key = nokey;
	m_commands.push_back(c);

	c.name = _T("");
	c.commandline = INTERNALCOMMAND;
	c.separator = true;
	c.nIconID = 0;
	c.enabled_viewpath = true;
	c.enabled_noviewpath = true;
	c.enabled_fileselected = true;
	c.enabled_folderselected = true;
	c.enabled_selected = true;
	c.enabled_noselection = true;
	c.enabled_selectedcount = 0;
	c.key = nokey;
	m_commands.push_back(c);

	c.name = _T("Show system files");
	c.commandline = INTERNALCOMMAND;
	c.separator = false;
	c.nIconID = IDI_SHOWHIDE;
	c.enabled_viewpath = true;
	c.enabled_noviewpath = false;
	c.enabled_fileselected = true;
	c.enabled_folderselected = true;
	c.enabled_selected = true;
	c.enabled_noselection = true;
	c.enabled_selectedcount = 0;
	key.control = true;
	key.shift = true;
	key.alt = false;
	key.keycode = WPARAM('H');
	c.key = key;
	m_commands.push_back(c);

	c.name = _T("Show extensions");
	c.commandline = INTERNALCOMMAND;
	c.separator = false;
	c.nIconID = IDI_SHOWEXTS;
	c.enabled_viewpath = true;
	c.enabled_noviewpath = false;
	c.enabled_fileselected = true;
	c.enabled_folderselected = true;
	c.enabled_selected = true;
	c.enabled_noselection = true;
	c.enabled_selectedcount = 0;
	c.key = nokey;
	m_commands.push_back(c);

	c.name = _T("Up");
	c.commandline = fullWinver <= 0x501 ? INTERNALCOMMAND : INTERNALCOMMANDHIDDEN;
	c.separator = false;
	c.nIconID = IDI_GOUP;
	c.enabled_viewpath = true;
	c.enabled_noviewpath = false;
	c.enabled_fileselected = true;
	c.enabled_folderselected = true;
	c.enabled_selected = true;
	c.enabled_noselection = true;
	c.enabled_selectedcount = 0;
	c.key = nokey;
	m_commands.push_back(c);

	c.name = _T("Console");
	c.commandline = INTERNALCOMMAND;
	c.separator = false;
	c.nIconID = IDI_CMD;
	c.enabled_viewpath = true;
	c.enabled_noviewpath = false;
	c.enabled_fileselected = true;
	c.enabled_folderselected = true;
	c.enabled_selected = true;
	c.enabled_noselection = true;
	c.enabled_selectedcount = 0;
	key.control = true;
	key.shift = false;
	key.alt = false;
	key.keycode = WPARAM('M');
	c.key = key;
	m_commands.push_back(c);

	c.name = _T("Copy Names");
	c.commandline = INTERNALCOMMAND;
	c.separator = false;
	c.nIconID = IDI_COPYNAME;
	c.enabled_viewpath = true;
	c.enabled_noviewpath = false;
	c.enabled_fileselected = true;
	c.enabled_folderselected = true;
	c.enabled_selected = true;
	c.enabled_noselection = true;
	c.enabled_selectedcount = 0;
	c.key = nokey;
	m_commands.push_back(c);

	c.name = _T("Copy Paths");
	c.commandline = INTERNALCOMMAND;
	c.separator = false;
	c.nIconID = IDI_COPYPATH;
	c.enabled_viewpath = true;
	c.enabled_noviewpath = false;
	c.enabled_fileselected = true;
	c.enabled_folderselected = true;
	c.enabled_selected = true;
	c.enabled_noselection = true;
	c.enabled_selectedcount = 0;
	key.control = true;
	key.shift = true;
	key.alt = false;
	key.keycode = WPARAM('C');
	c.key = key;
	m_commands.push_back(c);

	c.name = _T("New Folder");
	c.commandline = INTERNALCOMMAND;
	c.separator = false;
	c.nIconID = IDI_NEWFOLDER;
	c.enabled_viewpath = true;
	c.enabled_noviewpath = false;
	c.enabled_fileselected = true;
	c.enabled_folderselected = true;
	c.enabled_selected = true;
	c.enabled_noselection = true;
	c.enabled_selectedcount = 0;
	if (fullWinver < 0x601)
	{
		// Win7 already does create a new folder with Ctrl-Shift-N
		key.control = true;
		key.shift = true;
		key.alt = false;
		key.keycode = WPARAM('N');
		c.key = key;
	}
	else
		c.key = nokey;
	m_commands.push_back(c);

	c.name = _T("");
	c.commandline = INTERNALCOMMAND;
	c.separator = true;
	c.nIconID = 0;
	c.enabled_viewpath = true;
	c.enabled_noviewpath = true;
	c.enabled_fileselected = true;
	c.enabled_folderselected = true;
	c.enabled_selected = true;
	c.enabled_noselection = true;
	c.enabled_selectedcount = 0;
	c.key = nokey;
	m_commands.push_back(c);

	c.name = _T("Rename");
	c.commandline = INTERNALCOMMAND;
	c.separator = false;
	c.nIconID = IDI_RENAME;
	c.enabled_viewpath = true;
	c.enabled_noviewpath = false;
	c.enabled_fileselected = true;
	c.enabled_folderselected = true;
	c.enabled_selected = true;
	c.enabled_noselection = false;
	c.enabled_selectedcount = 0;
	key.control = true;
	key.shift = true;
	key.alt = false;
	key.keycode = WPARAM('R');
	c.key = key;
	m_commands.push_back(c);

	TCHAR szPath[MAX_PATH] = {0};
	if (SUCCEEDED(SHGetFolderPath(NULL, 
		CSIDL_APPDATA|CSIDL_FLAG_CREATE, 
		NULL, 
		SHGFP_TYPE_CURRENT, 
		szPath))) 
	{
		PathAppend(szPath, TEXT("StExBar"));
		CreateDirectory(szPath, NULL);
		PathAppend(szPath, TEXT("Commands.ini"));
	}

	CSimpleIni inifile;
	inifile.LoadFile(szPath);
	CSimpleIni::TNamesDepend sections;
	inifile.GetAllSections(sections);
	for (CSimpleIni::TNamesDepend::iterator it = sections.begin(); it != sections.end(); ++it)
	{
		hotkey key;
		key.control = (wstring(inifile.GetValue(*it, _T("control"), _T(""))).compare(_T("true")) == 0) ? true : false;
		key.shift = (wstring(inifile.GetValue(*it, _T("shift"), _T(""))).compare(_T("true")) == 0) ? true : false;
		key.alt = (wstring(inifile.GetValue(*it, _T("alt"), _T(""))).compare(_T("true")) == 0) ? true : false;
		key.keycode = _tstol(wstring(inifile.GetValue(*it, _T("keycode"), _T(""))).c_str());

		Command cmd;
		cmd.name = inifile.GetValue(*it, _T("name"), _T(""));
		cmd.icon = inifile.GetValue(*it, _T("icon"), _T(""));
		cmd.nIconID = 0;
		cmd.commandline = inifile.GetValue(*it, _T("commandline"), _T(""));
		cmd.separator = (wstring(inifile.GetValue(*it, _T("separator"), _T(""))).compare(_T("true")) == 0) ? true : false;

		cmd.enabled_viewpath = (wstring(inifile.GetValue(*it, _T("viewpath"), _T(""))).compare(_T("true")) == 0) ? true : false;
		cmd.enabled_noviewpath = (wstring(inifile.GetValue(*it, _T("noviewpath"), _T(""))).compare(_T("true")) == 0) ? true : false;
		cmd.enabled_fileselected = (wstring(inifile.GetValue(*it, _T("fileselected"), _T(""))).compare(_T("true")) == 0) ? true : false;
		cmd.enabled_folderselected = (wstring(inifile.GetValue(*it, _T("folderselected"), _T(""))).compare(_T("true")) == 0) ? true : false;
		cmd.enabled_selected = (wstring(inifile.GetValue(*it, _T("selected"), _T(""))).compare(_T("true")) == 0) ? true : false;
		cmd.enabled_noselection = (wstring(inifile.GetValue(*it, _T("noselection"), _T(""))).compare(_T("true")) == 0) ? true : false;
		cmd.enabled_selectedcount = _tstol(wstring(inifile.GetValue(*it, _T("selectedcount"), _T(""))).c_str());
		cmd.key = key;

		// check if that command already exists
		if (!cmd.separator)
		{
			for (vector<Command>::iterator cit = m_commands.begin(); cit != m_commands.end(); ++cit)
			{
				if (cit->name.compare(cmd.name) == 0)
				{
					// found the command
					// for normal commands, we simply ignore duplicate entries.
					// but for internal commands, we overwrite the default settings.
					if ((cit->commandline.compare(INTERNALCOMMAND)==0)||(cit->commandline.compare(INTERNALCOMMANDHIDDEN)==0))
					{
						// the icon for internal commands isn't saved to the ini file, so
						// we copy it from the existing one
						cmd.nIconID = cit->nIconID;
						m_commands.erase(cit);
					}
					break;
				}
			}
		}
		m_commands.push_back(cmd);
	}
	// remove leftover separators
	while (m_commands[0].separator)
		m_commands.erase(m_commands.begin());
	return false;
}

bool CCommands::SaveToFile()
{
	TCHAR szPath[MAX_PATH] = {0};
	if (SUCCEEDED(SHGetFolderPath(NULL, 
		CSIDL_APPDATA|CSIDL_FLAG_CREATE, 
		NULL, 
		SHGFP_TYPE_CURRENT, 
		szPath))) 
	{
		PathAppend(szPath, TEXT("StExBar"));
		CreateDirectory(szPath, NULL);
		PathAppend(szPath, TEXT("Commands.ini"));
	}

	CSimpleIni inifile;
	int counter = 0;
	for (vector<Command>::iterator it = m_commands.begin(); it != m_commands.end(); ++it)
	{
		Command * pCmd = &(*it);

		TCHAR countstr[40] = {0};
		TCHAR keycodestr[40] = {0};
		TCHAR counterstr[40] = {0};
		_stprintf_s(countstr, 40, _T("%ld"), pCmd->enabled_selectedcount);
		_stprintf_s(keycodestr, 40, _T("%ld"), pCmd->key.keycode);
		_stprintf_s(counterstr, 40, _T("%04ld_%s"), counter, pCmd->name.c_str());


		inifile.SetValue(counterstr, _T("name"), pCmd->name.c_str());
		inifile.SetValue(counterstr, _T("icon"), pCmd->icon.c_str());
		inifile.SetValue(counterstr, _T("commandline"), pCmd->commandline.c_str());
		inifile.SetValue(counterstr, _T("separator"), pCmd->separator ? _T("true") : _T(""));

		inifile.SetValue(counterstr, _T("viewpath"), pCmd->enabled_viewpath ? _T("true") : _T(""));
		inifile.SetValue(counterstr, _T("noviewpath"), pCmd->enabled_noviewpath ? _T("true") : _T(""));
		inifile.SetValue(counterstr, _T("fileselected"), pCmd->enabled_fileselected ? _T("true") : _T(""));
		inifile.SetValue(counterstr, _T("folderselected"), pCmd->enabled_folderselected ? _T("true") : _T(""));
		inifile.SetValue(counterstr, _T("selected"), pCmd->enabled_selected ? _T("true") : _T(""));
		inifile.SetValue(counterstr, _T("noselection"), pCmd->enabled_noselection ? _T("true") : _T(""));
		inifile.SetValue(counterstr, _T("selectedcount"), countstr);
		inifile.SetValue(counterstr, _T("control"), pCmd->key.control ? _T("true") : _T(""));
		inifile.SetValue(counterstr, _T("shift"), pCmd->key.shift ? _T("true") : _T(""));
		inifile.SetValue(counterstr, _T("alt"), pCmd->key.alt ? _T("true") : _T(""));
		inifile.SetValue(counterstr, _T("keycode"), keycodestr);
		counter++;
	}

	FILE * file = NULL;
	_tfopen_s(&file, szPath, _T("w"));
	inifile.SaveFile(file);
	fclose(file);

	return true;
}