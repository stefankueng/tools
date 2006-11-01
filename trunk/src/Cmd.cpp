#include "stdafx.h"
#include "SRBand.h"


void CDeskBand::StartCmd(std::wstring params)
{
	STARTUPINFO startup;
	PROCESS_INFORMATION process;
	memset(&startup, 0, sizeof(startup));
	startup.cb = sizeof(startup);
	memset(&process, 0, sizeof(process));

	// find the cmd program
	TCHAR buf[MAX_PATH] = {0};
	if (ExpandEnvironmentStrings(_T("%COMSPEC%"), buf, MAX_PATH)==NULL)
	{
		_tcscpy_s(buf, MAX_PATH, _T("cmd.exe"));
	}
	TCHAR * nonconstparams = new TCHAR[params.size()+1];
	_tcscpy_s(nonconstparams, params.size()+1, params.c_str());

	CreateProcess(buf, nonconstparams, NULL, NULL, FALSE, CREATE_NEW_CONSOLE|CREATE_DEFAULT_ERROR_MODE, 0, m_currentDirectory.c_str(), &startup, &process);
	delete [] nonconstparams;
	CloseHandle(process.hThread);
	CloseHandle(process.hProcess);
}

void CDeskBand::StartApplication(std::wstring commandline)
{
	STARTUPINFO startup;
	PROCESS_INFORMATION process;
	memset(&startup, 0, sizeof(startup));
	startup.cb = sizeof(startup);
	memset(&process, 0, sizeof(process));

	TCHAR * nonconst = new TCHAR[commandline.size()+1];
	_tcscpy_s(nonconst, commandline.size()+1, commandline.c_str());

	CreateProcess(NULL, nonconst, NULL, NULL, FALSE, CREATE_NEW_CONSOLE|CREATE_DEFAULT_ERROR_MODE, 0, m_currentDirectory.c_str(), &startup, &process);
	delete [] nonconst;
	CloseHandle(process.hThread);
	CloseHandle(process.hProcess);
}