// SkTimeStamp - Change file dates easily, directly from explorer

// Copyright (C) 2012, 2015, 2023 - Stefan Kueng

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
#include "ShellExt.h"
#include "ShellExtClassFactory.h"
#include <olectl.h>

HINSTANCE              g_hmodThisDll = nullptr; ///< handle to this DLL itself.
UINT                   g_cRefThisDll = 0;       ///< reference count of this DLL.

extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance,
                               DWORD     dwReason,
                               LPVOID /*lpReserved*/)
{
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
            // Extension DLL one-time initialization
            g_hmodThisDll = hInstance;
            break;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

STDAPI DllCanUnloadNow(void)
{
    return (g_cRefThisDll == 0 ? S_OK : S_FALSE);
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppvOut)
{
    *ppvOut = nullptr;

    if (IsEqualIID(rclsid, CLSID_SKTIMESTAMP))
    {
        CShellExtClassFactory *pcf = new CShellExtClassFactory();
        return pcf->QueryInterface(riid, ppvOut);
    }

    return CLASS_E_CLASSNOTAVAILABLE;
}

STDAPI DllInstall(__in BOOL /*bInstall*/, __in LPCWSTR /*pszCmdLine*/)
{
    return S_OK;
}

STDAPI DllRegisterServer(void)
{
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

    WCHAR szModule[MAX_PATH * 4] = {0};
    // Get this dll's path and file name.
    DWORD retval                 = GetModuleFileName(g_hmodThisDll, szModule, _countof(szModule));
    if (retval == NULL)
    {
        CoUninitialize();
        return SELFREG_E_CLASS;
    }

    if (RegSetValue(HKEY_CLASSES_ROOT, L"*\\shellex\\PropertySheetHandlers\\SKTimeStamp", REG_SZ, SKTIMESTAMP_GUID, static_cast<DWORD>(wcslen(SKTIMESTAMP_GUID)) * sizeof(WCHAR)) != ERROR_SUCCESS)
    {
        CoUninitialize();
        return SELFREG_E_CLASS;
    }
    if (RegSetValue(HKEY_CLASSES_ROOT, L"Directory\\shellex\\PropertySheetHandlers\\SKTimeStamp", REG_SZ, SKTIMESTAMP_GUID, static_cast<DWORD>(wcslen(SKTIMESTAMP_GUID)) * sizeof(WCHAR)) != ERROR_SUCCESS)
    {
        CoUninitialize();
        return SELFREG_E_CLASS;
    }
    if (RegSetValue(HKEY_CURRENT_USER, L"SOFTWARE\\Classes\\CLSID\\" SKTIMESTAMP_GUID, REG_SZ, L"SKTimeStamp", static_cast<DWORD>(wcslen(L"SKTimeStamp")) * sizeof(WCHAR)) != ERROR_SUCCESS)
    {
        CoUninitialize();
        return SELFREG_E_CLASS;
    }
    if (RegSetValue(HKEY_CURRENT_USER, L"SOFTWARE\\Classes\\CLSID\\" SKTIMESTAMP_GUID L"\\InProcServer32", REG_SZ, szModule, static_cast<DWORD>(wcslen(szModule)) * sizeof(WCHAR)) != ERROR_SUCCESS)
    {
        CoUninitialize();
        return SELFREG_E_CLASS;
    }
    if (SHSetValue(HKEY_CURRENT_USER, L"SOFTWARE\\Classes\\CLSID\\" SKTIMESTAMP_GUID L"\\InProcServer32", L"ThreadingModel", REG_SZ, const_cast<LPWSTR>(L"Apartment"), 20) != ERROR_SUCCESS)
    {
        CoUninitialize();
        return SELFREG_E_CLASS;
    }
    if (SHSetValue(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved", SKTIMESTAMP_GUID, REG_SZ, const_cast<LPWSTR>(L"SKTimeStamp"), 24) != ERROR_SUCCESS)
    {
        CoUninitialize();
        return SELFREG_E_CLASS;
    }
    CoUninitialize();
    return S_OK;
}

STDAPI DllUnregisterServer(void)
{
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (SHDeleteKey(HKEY_CLASSES_ROOT, L"*\\shellex\\PropertySheetHandlers\\SKTimeStamp") != ERROR_SUCCESS)
    {
        CoUninitialize();
        return SELFREG_E_CLASS;
    }
    if (SHDeleteKey(HKEY_CLASSES_ROOT, L"Directory\\shellex\\PropertySheetHandlers\\SKTimeStamp") != ERROR_SUCCESS)
    {
        CoUninitialize();
        return SELFREG_E_CLASS;
    }
    if (SHDeleteKey(HKEY_CURRENT_USER, L"SOFTWARE\\Classes\\CLSID\\" SKTIMESTAMP_GUID) != ERROR_SUCCESS)
    {
        CoUninitialize();
        return SELFREG_E_CLASS;
    }
    if (SHDeleteValue(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved", SKTIMESTAMP_GUID) != ERROR_SUCCESS)
    {
        CoUninitialize();
        return SELFREG_E_CLASS;
    }
    CoUninitialize();
    return S_OK;
}
