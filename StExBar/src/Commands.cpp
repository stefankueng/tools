// StExBar - an explorer toolbar

// Copyright (C) 2007-2017, 2020-2021 - Stefan Kueng

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
#include "Commands.h"
#include "resource.h"
#include <VersionHelpers.h>

CCommands::CCommands(void)
{
}

CCommands::~CCommands(void)
{
}

bool CCommands::LoadFromFile()
{
    m_commands.clear();
    // fill in the default commands first
    Command      c;
    hotkey       key;
    const hotkey nokey;
    c.name                   = L"StexBar Internal Edit Box";
    c.commandline            = INTERNALCOMMAND;
    c.separator              = false;
    c.nIconID                = 0;
    c.enabled_viewpath       = true;
    c.enabled_noviewpath     = false;
    c.enabled_fileselected   = true;
    c.enabled_folderselected = true;
    c.enabled_selected       = true;
    c.enabled_noselection    = true;
    c.enabled_selectedcount  = 0;
    key.control              = true;
    key.shift                = false;
    key.alt                  = false;
    key.keycode              = WPARAM('K');
    c.key                    = key;
    m_commands.push_back(c);

    c.name                   = L"Options";
    c.commandline            = INTERNALCOMMAND;
    c.separator              = false;
    c.nIconID                = IDI_OPTIONS;
    c.enabled_viewpath       = true;
    c.enabled_noviewpath     = true;
    c.enabled_fileselected   = true;
    c.enabled_folderselected = true;
    c.enabled_selected       = true;
    c.enabled_noselection    = true;
    c.enabled_selectedcount  = 0;
    c.key                    = nokey;
    m_commands.push_back(c);

    c.name                   = L"";
    c.commandline            = INTERNALCOMMAND;
    c.separator              = true;
    c.nIconID                = 0;
    c.enabled_viewpath       = true;
    c.enabled_noviewpath     = true;
    c.enabled_fileselected   = true;
    c.enabled_folderselected = true;
    c.enabled_selected       = true;
    c.enabled_noselection    = true;
    c.enabled_selectedcount  = 0;
    c.key                    = nokey;
    m_commands.push_back(c);

    c.name                   = L"Show system files";
    c.commandline            = INTERNALCOMMAND;
    c.separator              = false;
    c.nIconID                = IDI_SHOWHIDE;
    c.enabled_viewpath       = true;
    c.enabled_noviewpath     = false;
    c.enabled_fileselected   = true;
    c.enabled_folderselected = true;
    c.enabled_selected       = true;
    c.enabled_noselection    = true;
    c.enabled_selectedcount  = 0;
    key.control              = true;
    key.shift                = true;
    key.alt                  = false;
    key.keycode              = WPARAM('H');
    c.key                    = key;
    m_commands.push_back(c);

    c.name                   = L"Show extensions";
    c.commandline            = INTERNALCOMMAND;
    c.separator              = false;
    c.nIconID                = IDI_SHOWEXTS;
    c.enabled_viewpath       = true;
    c.enabled_noviewpath     = false;
    c.enabled_fileselected   = true;
    c.enabled_folderselected = true;
    c.enabled_selected       = true;
    c.enabled_noselection    = true;
    c.enabled_selectedcount  = 0;
    c.key                    = nokey;
    m_commands.push_back(c);

    c.name                   = L"Up";
    c.commandline            = IsWindowsVistaOrGreater() ? INTERNALCOMMANDHIDDEN : INTERNALCOMMAND;
    c.separator              = false;
    c.nIconID                = IDI_GOUP;
    c.enabled_viewpath       = true;
    c.enabled_noviewpath     = false;
    c.enabled_fileselected   = true;
    c.enabled_folderselected = true;
    c.enabled_selected       = true;
    c.enabled_noselection    = true;
    c.enabled_selectedcount  = 0;
    c.key                    = nokey;
    m_commands.push_back(c);

    c.name                   = L"Console";
    c.commandline            = INTERNALCOMMAND;
    c.separator              = false;
    c.nIconID                = 0;
    c.enabled_viewpath       = true;
    c.enabled_noviewpath     = false;
    c.enabled_fileselected   = true;
    c.enabled_folderselected = true;
    c.enabled_selected       = true;
    c.enabled_noselection    = true;
    c.enabled_selectedcount  = 0;
    key.control              = true;
    key.shift                = false;
    key.alt                  = false;
    key.keycode              = WPARAM('M');
    c.key                    = key;
    m_commands.push_back(c);

    c.name                   = L"PowerShell";
    c.commandline            = INTERNALCOMMAND;
    c.separator              = false;
    c.nIconID                = 0;
    c.enabled_viewpath       = true;
    c.enabled_noviewpath     = false;
    c.enabled_fileselected   = true;
    c.enabled_folderselected = true;
    c.enabled_selected       = true;
    c.enabled_noselection    = true;
    c.enabled_selectedcount  = 0;
    key.control              = true;
    key.shift                = true;
    key.alt                  = false;
    key.keycode              = WPARAM('M');
    c.key                    = key;
    m_commands.push_back(c);

    c.name                   = L"Copy Names";
    c.commandline            = INTERNALCOMMAND;
    c.separator              = false;
    c.nIconID                = IDI_COPYNAME;
    c.enabled_viewpath       = true;
    c.enabled_noviewpath     = false;
    c.enabled_fileselected   = true;
    c.enabled_folderselected = true;
    c.enabled_selected       = true;
    c.enabled_noselection    = true;
    c.enabled_selectedcount  = 0;
    c.key                    = nokey;
    m_commands.push_back(c);

    c.name                   = L"Copy Paths";
    c.commandline            = INTERNALCOMMAND;
    c.separator              = false;
    c.nIconID                = IDI_COPYPATH;
    c.enabled_viewpath       = true;
    c.enabled_noviewpath     = false;
    c.enabled_fileselected   = true;
    c.enabled_folderselected = true;
    c.enabled_selected       = true;
    c.enabled_noselection    = true;
    c.enabled_selectedcount  = 0;
    key.control              = true;
    key.shift                = true;
    key.alt                  = false;
    key.keycode              = WPARAM('C');
    c.key                    = key;
    m_commands.push_back(c);

    // Win7 already does create a new folder with Ctrl-Shift-N
    c.name                   = L"New Folder";
    c.commandline            = IsWindows7OrGreater() ? INTERNALCOMMANDHIDDEN : INTERNALCOMMAND;
    c.separator              = false;
    c.nIconID                = IDI_NEWFOLDER;
    c.enabled_viewpath       = true;
    c.enabled_noviewpath     = false;
    c.enabled_fileselected   = true;
    c.enabled_folderselected = true;
    c.enabled_selected       = true;
    c.enabled_noselection    = true;
    c.enabled_selectedcount  = 0;
    c.key                    = nokey;
    if (!IsWindows7OrGreater())
    {
        key.control = true;
        key.shift   = true;
        key.alt     = false;
        key.keycode = WPARAM('N');
        c.key       = key;
    }
    m_commands.push_back(c);

    c.name                   = L"";
    c.commandline            = INTERNALCOMMAND;
    c.separator              = true;
    c.nIconID                = 0;
    c.enabled_viewpath       = true;
    c.enabled_noviewpath     = true;
    c.enabled_fileselected   = true;
    c.enabled_folderselected = true;
    c.enabled_selected       = true;
    c.enabled_noselection    = true;
    c.enabled_selectedcount  = 0;
    c.key                    = nokey;
    m_commands.push_back(c);

    c.name                   = L"Rename";
    c.commandline            = INTERNALCOMMAND;
    c.separator              = false;
    c.nIconID                = IDI_RENAME;
    c.enabled_viewpath       = true;
    c.enabled_noviewpath     = false;
    c.enabled_fileselected   = true;
    c.enabled_folderselected = true;
    c.enabled_selected       = true;
    c.enabled_noselection    = false;
    c.enabled_selectedcount  = 0;
    key.control              = true;
    key.shift                = true;
    key.alt                  = false;
    key.keycode              = WPARAM('R');
    c.key                    = key;
    m_commands.push_back(c);

    c.name                   = L"Move to subfolder";
    c.commandline            = INTERNALCOMMAND;
    c.separator              = false;
    c.nIconID                = IDI_MOVETOFOLDER;
    c.enabled_viewpath       = false;
    c.enabled_noviewpath     = false;
    c.enabled_fileselected   = true;
    c.enabled_folderselected = true;
    c.enabled_selected       = true;
    c.enabled_noselection    = false;
    c.enabled_selectedcount  = 0;
    c.key                    = nokey;
    m_commands.push_back(c);

    wchar_t szPath[MAX_PATH] = {0};
    if (SUCCEEDED(SHGetFolderPath(NULL,
                                  CSIDL_APPDATA | CSIDL_FLAG_CREATE,
                                  NULL,
                                  SHGFP_TYPE_CURRENT,
                                  szPath)))
    {
        PathAppend(szPath, L"StExBar");
        CreateDirectory(szPath, NULL);
        PathAppend(szPath, L"Commands.ini");
    }

    CSimpleIni inifile;
    inifile.LoadFile(szPath);
    CSimpleIni::TNamesDepend sections;
    inifile.GetAllSections(sections);
    for (CSimpleIni::TNamesDepend::iterator it = sections.begin(); it != sections.end(); ++it)
    {
        hotkey key2;
        key2.control = (std::wstring(inifile.GetValue(*it, L"control", L"")).compare(L"true") == 0) ? true : false;
        key2.shift   = (std::wstring(inifile.GetValue(*it, L"shift", L"")).compare(L"true") == 0) ? true : false;
        key2.alt     = (std::wstring(inifile.GetValue(*it, L"alt", L"")).compare(L"true") == 0) ? true : false;
        key2.keycode = _wtol(std::wstring(inifile.GetValue(*it, L"keycode", L"")).c_str());

        Command cmd;
        cmd.name        = inifile.GetValue(*it, L"name", L"");
        cmd.icon        = inifile.GetValue(*it, L"icon", L"");
        cmd.nIconID     = 0;
        cmd.commandline = inifile.GetValue(*it, L"commandline", L"");
        cmd.startin     = inifile.GetValue(*it, L"startin", L"");
        cmd.separator   = (std::wstring(inifile.GetValue(*it, L"separator", L"")).compare(L"true") == 0) ? true : false;

        cmd.enabled_viewpath       = (std::wstring(inifile.GetValue(*it, L"viewpath", L"")).compare(L"true") == 0) ? true : false;
        cmd.enabled_noviewpath     = (std::wstring(inifile.GetValue(*it, L"noviewpath", L"")).compare(L"true") == 0) ? true : false;
        cmd.enabled_fileselected   = (std::wstring(inifile.GetValue(*it, L"fileselected", L"")).compare(L"true") == 0) ? true : false;
        cmd.enabled_folderselected = (std::wstring(inifile.GetValue(*it, L"folderselected", L"")).compare(L"true") == 0) ? true : false;
        cmd.enabled_selected       = (std::wstring(inifile.GetValue(*it, L"selected", L"")).compare(L"true") == 0) ? true : false;
        cmd.enabled_noselection    = (std::wstring(inifile.GetValue(*it, L"noselection", L"")).compare(L"true") == 0) ? true : false;
        cmd.enabled_selectedcount  = _wtol(std::wstring(inifile.GetValue(*it, L"selectedcount", L"")).c_str());
        cmd.key                    = key2;

        // check if that command already exists
        if (cmd.name.compare(L"StexBar Internal Edit Box") != 0)
        {
            if (!cmd.separator)
            {
                for (std::vector<Command>::iterator cit = m_commands.begin(); cit != m_commands.end(); ++cit)
                {
                    if (cit->name.compare(cmd.name) == 0)
                    {
                        // found the command
                        // for normal commands, we simply ignore duplicate entries.
                        // but for internal commands, we overwrite the default settings.
                        if ((cit->commandline.compare(INTERNALCOMMAND) == 0) || (cit->commandline.compare(INTERNALCOMMANDHIDDEN) == 0))
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
    }
    // remove leftover separators
    while (m_commands[1].separator)
        m_commands.erase(m_commands.begin() + 1);
    return false;
}

bool CCommands::SaveToFile()
{
    wchar_t szPath[MAX_PATH] = {0};
    if (SUCCEEDED(SHGetFolderPath(NULL,
                                  CSIDL_APPDATA | CSIDL_FLAG_CREATE,
                                  NULL,
                                  SHGFP_TYPE_CURRENT,
                                  szPath)))
    {
        PathAppend(szPath, L"StExBar");
        CreateDirectory(szPath, NULL);
        PathAppend(szPath, L"Commands.ini");
    }

    CSimpleIni inifile;
    int        counter = 0;
    for (std::vector<Command>::iterator it = m_commands.begin(); it != m_commands.end(); ++it)
    {
        Command* pCmd = &(*it);

        wchar_t countstr[40]   = {0};
        wchar_t keycodestr[40] = {0};
        wchar_t counterstr[40] = {0};
        swprintf_s(countstr, _countof(countstr), L"%d", pCmd->enabled_selectedcount);
        swprintf_s(keycodestr, _countof(keycodestr), L"%Iu", pCmd->key.keycode);
        swprintf_s(counterstr, _countof(counterstr), L"%04d_%s", counter, pCmd->name.c_str());

        inifile.SetValue(counterstr, L"name", pCmd->name.c_str());
        inifile.SetValue(counterstr, L"icon", pCmd->icon.c_str());
        inifile.SetValue(counterstr, L"commandline", pCmd->commandline.c_str());
        inifile.SetValue(counterstr, L"startin", pCmd->startin.c_str());
        inifile.SetValue(counterstr, L"separator", pCmd->separator ? L"true" : L"");

        inifile.SetValue(counterstr, L"viewpath", pCmd->enabled_viewpath ? L"true" : L"");
        inifile.SetValue(counterstr, L"noviewpath", pCmd->enabled_noviewpath ? L"true" : L"");
        inifile.SetValue(counterstr, L"fileselected", pCmd->enabled_fileselected ? L"true" : L"");
        inifile.SetValue(counterstr, L"folderselected", pCmd->enabled_folderselected ? L"true" : L"");
        inifile.SetValue(counterstr, L"selected", pCmd->enabled_selected ? L"true" : L"");
        inifile.SetValue(counterstr, L"noselection", pCmd->enabled_noselection ? L"true" : L"");
        inifile.SetValue(counterstr, L"selectedcount", countstr);
        inifile.SetValue(counterstr, L"control", pCmd->key.control ? L"true" : L"");
        inifile.SetValue(counterstr, L"shift", pCmd->key.shift ? L"true" : L"");
        inifile.SetValue(counterstr, L"alt", pCmd->key.alt ? L"true" : L"");
        inifile.SetValue(counterstr, L"keycode", keycodestr);
        counter++;
    }

    FILE* file = NULL;
    _wfopen_s(&file, szPath, L"w");
    inifile.SaveFile(file);
    fclose(file);

    return true;
}