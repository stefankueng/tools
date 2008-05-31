#pragma once
#include "SimpleIni.h"
#include <string>
#include <vector>

using namespace std;

#define INTERNALCOMMAND _T("INTERNALCOMMAND")
#define INTERNALCOMMANDHIDDEN _T("INTERNALCOMMANDHIDDEN")

class hotkey
{
public:
	hotkey() : keycode(0), control(false), shift(false), alt(false) {}

	WPARAM	keycode;
	bool	control;
	bool	shift;
	bool	alt;
	bool operator<(const hotkey & hk) const
	{
		if (keycode < hk.keycode) return true;
		if (keycode > hk.keycode) return false;

		return (((control ? 4 : 0)|(shift ? 2 : 0)|(alt ? 1 : 0))
			<
			((hk.control ? 4 : 0)|(hk.shift ? 2 : 0)|(hk.alt ? 1 : 0)));
	}
};


struct Command
{
	wstring			name;
	wstring			icon;
	WORD			nIconID;
	wstring			commandline;
	bool			separator;

	bool			enabled_viewpath;
	bool			enabled_noviewpath;

	bool			enabled_fileselected;
	bool			enabled_folderselected;
	bool			enabled_selected;
	bool			enabled_noselection;
	int				enabled_selectedcount;
	hotkey			key;
};


class CCommands
{
public:
	CCommands(void);
	~CCommands(void);

	bool				LoadFromFile();
	bool				SaveToFile();
	int					GetCount() {return (int)m_commands.size();}
	Command 			GetCommand(int index) {return m_commands[index];}
	Command *			GetCommandPtr(int index) {return &m_commands[index];}
	void				RemoveCommand(int index) {m_commands.erase(m_commands.begin()+index);}
	void				InsertCommand(int index, Command cmd) {m_commands.insert(m_commands.begin()+index, cmd);}
	void				SetCommand(int index, Command cmd) {m_commands[index] = cmd;}
private:
	vector<Command>		m_commands;
};
