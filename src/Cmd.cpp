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
#include "stdafx.h"
#include "SRBand.h"


void CDeskBand::StartCmd(const wstring& cwd, std::wstring params)
{
	STARTUPINFO startup;
	PROCESS_INFORMATION process;
	memset(&startup, 0, sizeof(startup));
	startup.cb = sizeof(startup);
	memset(&process, 0, sizeof(process));

	// find the cmd program
	TCHAR buf[MAX_PATH] = {0};
	if (DWORD(CRegStdWORD(_T("Software\\StefansTools\\StExBar\\UsePowershell"), FALSE)))
	{
		if (ExpandEnvironmentStrings(_T("%systemroot%"), buf, MAX_PATH))
		{
			_tcscat_s(buf, MAX_PATH, _T("\\system32\\windowspowershell\\v1.0\\powershell.exe"));
		}
		else
			_tcscpy_s(buf, MAX_PATH, _T("c:\\windows\\system32\\windowspowershell\\v1.0\\powershell.exe"));

		// the powershell ignores the '-noexit' parameter completely if it's not the first
		// parameter. Problem is, it also ignores it if we split the path to the exe and its parameters
		// in the CreateProcess() call. The only way it recognizes the parameter is if
		// we call CreateProcess with NULL as the first parameter and put everything in the buffer
		// of the second parameter. Really, really stupid.

		TCHAR * nonconstparams = new TCHAR[params.size()+MAX_PATH];
		_tcscpy_s(nonconstparams, params.size()+MAX_PATH, _T("\""));
		_tcscat_s(nonconstparams, params.size()+MAX_PATH, buf);
		_tcscat_s(nonconstparams, params.size()+MAX_PATH, _T("\" "));
		_tcscat_s(nonconstparams, params.size()+MAX_PATH, params.c_str());

		CreateProcess(NULL, 
			nonconstparams, 
			NULL, 
			NULL, 
			FALSE, 
			CREATE_NEW_CONSOLE|CREATE_DEFAULT_ERROR_MODE, 
			0, 
			cwd.empty() ? NULL : cwd.c_str(), 
			&startup, 
			&process);
		delete [] nonconstparams;
		CloseHandle(process.hThread);
		CloseHandle(process.hProcess);
		return;
	}
	else if (ExpandEnvironmentStrings(_T("%COMSPEC%"), buf, MAX_PATH)==NULL)
	{
		_tcscpy_s(buf, MAX_PATH, _T("cmd.exe"));
	}
	TCHAR * nonconstparams = new TCHAR[params.size()+1];
	_tcscpy_s(nonconstparams, params.size()+1, params.c_str());

	CreateProcess(buf, 
		nonconstparams, 
		NULL, 
		NULL, 
		FALSE, 
		CREATE_NEW_CONSOLE|CREATE_DEFAULT_ERROR_MODE, 
		0, 
		cwd.empty() ? NULL : cwd.c_str(), 
		&startup, 
		&process);
	delete [] nonconstparams;
	CloseHandle(process.hThread);
	CloseHandle(process.hProcess);
}

void CDeskBand::StartApplication(const wstring& cwd, std::wstring commandline)
{
	STARTUPINFO startup;
	PROCESS_INFORMATION process;
	memset(&startup, 0, sizeof(startup));
	startup.cb = sizeof(startup);
	memset(&process, 0, sizeof(process));

	DWORD len = ExpandEnvironmentStrings(commandline.c_str(), NULL, 0);
	TCHAR * nonconst = new TCHAR[len+1];
	if (ExpandEnvironmentStrings(commandline.c_str(), nonconst, len)==0)
		_tcscpy_s(nonconst, commandline.size()+1, commandline.c_str());

	CreateProcess(NULL, 
		nonconst, 
		NULL, 
		NULL, 
		FALSE, 
		CREATE_NEW_CONSOLE|CREATE_DEFAULT_ERROR_MODE, 
		0, 
		cwd.empty() ? NULL : cwd.c_str(), 
		&startup, 
		&process);
	delete [] nonconst;
	CloseHandle(process.hThread);
	CloseHandle(process.hProcess);
}