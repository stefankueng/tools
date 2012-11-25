// StExBar - an explorer toolbar

// Copyright (C) 2007-2009, 2011 - Stefan Kueng

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
#include "SimpleIni.h"
#include <string>
#include <vector>


#define INTERNALCOMMAND _T("INTERNALCOMMAND")
#define INTERNALCOMMANDHIDDEN _T("INTERNALCOMMANDHIDDEN")

class hotkey
{
public:
    hotkey() : keycode(0), control(false), shift(false), alt(false) {}

    WPARAM  keycode;
    bool    control;
    bool    shift;
    bool    alt;
    bool operator<(const hotkey & hk) const
    {
        if (keycode < hk.keycode) return true;
        if (keycode > hk.keycode) return false;

        return (((control ? 4 : 0)|(shift ? 2 : 0)|(alt ? 1 : 0))
            <
            ((hk.control ? 4 : 0)|(hk.shift ? 2 : 0)|(hk.alt ? 1 : 0)));
    }
};


class Command
{
public:
    Command() : nIconID(0), separator(false), enabled_viewpath(true), enabled_noviewpath(true)
        , enabled_fileselected(true)
        , enabled_folderselected(true)
        , enabled_selected(true)
        , enabled_noselection(true)
        , enabled_selectedcount(0)
    {
    }

    std::wstring    name;
    std::wstring    icon;
    WORD            nIconID;
    std::wstring    commandline;
    std::wstring    startin;
    bool            separator;

    bool            enabled_viewpath;
    bool            enabled_noviewpath;

    bool            enabled_fileselected;
    bool            enabled_folderselected;
    bool            enabled_selected;
    bool            enabled_noselection;
    int             enabled_selectedcount;
    hotkey          key;
};


class CCommands
{
public:
    CCommands(void);
    ~CCommands(void);

    bool                LoadFromFile();
    bool                SaveToFile();
    int                 GetCount() {return (int)m_commands.size();}
    Command             GetCommand(int index) {return m_commands[index];}
    Command *           GetCommandPtr(int index) {return &m_commands[index];}
    void                RemoveCommand(int index) {m_commands.erase(m_commands.begin()+index);}
    void                InsertCommand(int index, Command cmd) {m_commands.insert(m_commands.begin()+index, cmd);}
    void                SetCommand(int index, Command cmd) {m_commands[index] = cmd;}
private:
    std::vector<Command>        m_commands;
};
