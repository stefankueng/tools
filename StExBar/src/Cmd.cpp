// StExBar - an explorer toolbar

// Copyright (C) 2007-2012, 2014, 2017, 2020 - Stefan Kueng

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
#include "resource.h"

void CDeskBand::StartCmd(const std::wstring& cwd, std::wstring params, bool elevated)
{
    STARTUPINFO         startup;
    PROCESS_INFORMATION process;
    SecureZeroMemory(&startup, sizeof(startup));
    startup.cb = sizeof(startup);
    SecureZeroMemory(&process, sizeof(process));

    // find the cmd program
    wchar_t buf[MAX_PATH] = {0};
    if (ExpandEnvironmentStrings(L"%COMSPEC%", buf, _countof(buf)) == NULL)
    {
        wcscpy_s(buf, _countof(buf), L"cmd.exe");
    }
    auto nonconstparams = std::make_unique<wchar_t[]>(params.size() + 1);
    wcscpy_s(nonconstparams.get(), params.size() + 1, params.c_str());

    if (elevated)
    {
        size_t psize   = params.size() + 20 + cwd.size();
        nonconstparams = std::make_unique<wchar_t[]>(psize);
        wcscpy_s(nonconstparams.get(), psize, L"/k cd /d \"");
        wcscat_s(nonconstparams.get(), psize, cwd.c_str());
        wcscat_s(nonconstparams.get(), psize, L"\" ");
        wcscat_s(nonconstparams.get(), psize, params.c_str());

        SHELLEXECUTEINFO shExecInfo;

        shExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);

        shExecInfo.fMask        = NULL;
        shExecInfo.hwnd         = m_hWnd;
        shExecInfo.lpVerb       = L"runas";
        shExecInfo.lpFile       = buf;
        shExecInfo.lpParameters = nonconstparams.get();
        shExecInfo.lpDirectory  = cwd.empty() ? NULL : cwd.c_str();
        shExecInfo.nShow        = SW_NORMAL;
        shExecInfo.hInstApp     = NULL;

        ShellExecuteEx(&shExecInfo);
    }
    else
    {
        CreateProcess(buf,
                      nonconstparams.get(),
                      NULL,
                      NULL,
                      FALSE,
                      CREATE_NEW_CONSOLE | CREATE_DEFAULT_ERROR_MODE,
                      0,
                      cwd.empty() ? NULL : cwd.c_str(),
                      &startup,
                      &process);
        CloseHandle(process.hThread);
        CloseHandle(process.hProcess);
    }
}

void CDeskBand::StartPS(const std::wstring& cwd, std::wstring params, bool elevated)
{
    STARTUPINFO         startup;
    PROCESS_INFORMATION process;
    SecureZeroMemory(&startup, sizeof(startup));
    startup.cb = sizeof(startup);
    SecureZeroMemory(&process, sizeof(process));

    // find the cmd program
    wchar_t buf[MAX_PATH] = {0};

    if (ExpandEnvironmentStrings(L"%systemroot%", buf, _countof(buf)))
        wcscat_s(buf, _countof(buf), L"\\system32\\windowspowershell\\v1.0\\powershell.exe");
    else
        wcscpy_s(buf, _countof(buf), L"c:\\windows\\system32\\windowspowershell\\v1.0\\powershell.exe");

    // the powershell ignores the '-noexit' parameter completely if it's not the first
    // parameter. Problem is, it also ignores it if we split the path to the exe and its parameters
    // in the CreateProcess() call. The only way it recognizes the parameter is if
    // we call CreateProcess with NULL as the first parameter and put everything in the buffer
    // of the second parameter. Really, really stupid.

    if (params.empty())
    {
        params = L"-NoExit -Command \"Set-Location -LiteralPath '" + cwd + L"'\"";
    }

    auto nonconstparams = std::make_unique<wchar_t[]>(params.size() + MAX_PATH);

    if (elevated)
    {
        SHELLEXECUTEINFO shExecInfo;

        shExecInfo.cbSize       = sizeof(SHELLEXECUTEINFO);
        shExecInfo.fMask        = NULL;
        shExecInfo.hwnd         = m_hWnd;
        shExecInfo.lpVerb       = L"runas";
        shExecInfo.lpFile       = buf;
        shExecInfo.lpParameters = params.c_str();
        shExecInfo.lpDirectory  = cwd.empty() ? NULL : cwd.c_str();
        shExecInfo.nShow        = SW_NORMAL;
        shExecInfo.hInstApp     = NULL;

        ShellExecuteEx(&shExecInfo);
    }
    else
    {
        wcscpy_s(nonconstparams.get(), params.size() + MAX_PATH, L"\"");
        wcscat_s(nonconstparams.get(), params.size() + MAX_PATH, buf);
        wcscat_s(nonconstparams.get(), params.size() + MAX_PATH, L"\" ");
        wcscat_s(nonconstparams.get(), params.size() + MAX_PATH, params.c_str());
        CreateProcess(NULL,
                      nonconstparams.get(),
                      NULL,
                      NULL,
                      FALSE,
                      CREATE_NEW_CONSOLE | CREATE_DEFAULT_ERROR_MODE,
                      0,
                      cwd.empty() ? NULL : cwd.c_str(),
                      &startup,
                      &process);
        CloseHandle(process.hThread);
        CloseHandle(process.hProcess);
    }
}

void CDeskBand::StartApplication(const std::wstring& cwd, std::wstring commandline, bool elevated)
{
    STARTUPINFO         startup;
    PROCESS_INFORMATION process;
    SecureZeroMemory(&startup, sizeof(startup));
    startup.cb = sizeof(startup);
    SecureZeroMemory(&process, sizeof(process));

    DWORD len      = ExpandEnvironmentStrings(commandline.c_str(), NULL, 0);
    auto  nonconst = std::make_unique<wchar_t[]>(len + 1);
    if (ExpandEnvironmentStrings(commandline.c_str(), nonconst.get(), len) == 0)
        wcscpy_s(nonconst.get(), commandline.size() + 1, commandline.c_str());

    if (elevated)
    {
        auto     nclen  = wcslen(nonconst.get());
        wchar_t* params = nullptr;
        // try to separate the command from its params
        if (nonconst[0] == '"')
        {
            auto p = wcschr(nonconst.get() + 1, '"');
            if (p)
            {
                params = p;
                params++;
                if (nonconst.get() + nclen > params)
                {
                    *params = 0;
                    params++;
                }
                else
                    params = nullptr;
            }
        }
        else
        {
            auto p = wcschr(nonconst.get(), ' ');
            if (p)
            {
                params = p;
                if (nonconst.get() + nclen > params)
                {
                    *params = 0;
                    params++;
                }
                else
                    params = nullptr;
            }
        }
        SHELLEXECUTEINFO shExecInfo;

        shExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);

        shExecInfo.fMask        = NULL;
        shExecInfo.hwnd         = m_hWnd;
        shExecInfo.lpVerb       = L"runas";
        shExecInfo.lpFile       = nonconst.get();
        shExecInfo.lpParameters = params;
        shExecInfo.lpDirectory  = cwd.empty() ? NULL : cwd.c_str();
        shExecInfo.nShow        = SW_NORMAL;
        shExecInfo.hInstApp     = NULL;

        ShellExecuteEx(&shExecInfo);
    }
    else
    {
        CreateProcess(NULL,
                      nonconst.get(),
                      NULL,
                      NULL,
                      FALSE,
                      CREATE_NEW_CONSOLE | CREATE_DEFAULT_ERROR_MODE,
                      0,
                      cwd.empty() ? NULL : cwd.c_str(),
                      &startup,
                      &process);
        CloseHandle(process.hThread);
        CloseHandle(process.hProcess);
    }
}
