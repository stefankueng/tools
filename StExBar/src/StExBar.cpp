// StExBar - an explorer toolbar

// Copyright (C) 2007-2012 - Stefan Kueng

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

#include "stdafx.h"
#include "ClassFactory.h"
#include <strsafe.h>
#include <shlwapi.h>

#pragma comment(lib, "shlwapi")

// include the manifest required to use the version 6 common controls
#ifndef WIN64
#   pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='X86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#   pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

// This part is only done once.
// If you need to use the GUID in another file, just include Guid.h.
#define INITGUID
#include <initguid.h>
#include <shlguid.h>
#include "Guid.h"

extern "C" BOOL WINAPI DllMain(HINSTANCE, DWORD, LPVOID);
BOOL RegisterServer(CLSID, LPTSTR);
BOOL RegisterComCat(CLSID, CATID);
BOOL UnRegisterServer(CLSID, LPTSTR);
BOOL UnRegisterComCat(CLSID, CATID);

HINSTANCE   g_hInst;
UINT        g_DllRefCount;
HRESULT     hr;


void my_invalid_parameter(const wchar_t * expression,
                        const wchar_t * function,
                        const wchar_t * file,
                        unsigned int line,
                        uintptr_t pReserved)
{
    UNREFERENCED_PARAMETER(expression);
    UNREFERENCED_PARAMETER(function);
    UNREFERENCED_PARAMETER(file);
    UNREFERENCED_PARAMETER(line);
    UNREFERENCED_PARAMETER(pReserved);
}

extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance,
                               DWORD dwReason,
                               LPVOID /*lpReserved*/)
{
    switch(dwReason)
    {
    case DLL_PROCESS_ATTACH:
        {
            // only load the dll in explorer
            WCHAR path[MAX_PATH] = {0};
            if (GetModuleFileName(NULL, path, MAX_PATH)==FALSE)
                return FALSE;
            wchar_t * slash = wcsrchr(path, '\\');
            if (slash == NULL)
                return FALSE;
            if (_wcsicmp(L"\\explorer.exe", slash) != 0)
                return FALSE;
            g_hInst = hInstance;
            _set_invalid_parameter_handler(my_invalid_parameter);
        }
        break;
    case DLL_PROCESS_DETACH:
        UnregisterClass(DB_CLASS_NAME, g_hInst);
        break;
    }

    return TRUE;
}

STDAPI DllCanUnloadNow(void)
{
    return (g_DllRefCount ? S_FALSE : S_OK);
}

STDAPI DllGetClassObject(REFCLSID rclsid,
                         REFIID riid,
                         LPVOID *ppReturn)
{
    *ppReturn = NULL;

    // If this classid is not supported, return the proper error code.
    if (!IsEqualCLSID(rclsid, CLSID_StExBand))
        return CLASS_E_CLASSNOTAVAILABLE;

    // Create a CClassFactory object and check it for validity.
    CClassFactory *pClassFactory = new CClassFactory(rclsid);
    if (NULL == pClassFactory)
        return E_OUTOFMEMORY;

    // Get the QueryInterface return for our return value
    HRESULT hResult = pClassFactory->QueryInterface(riid, ppReturn);

    // Call Release to decrement the ref count - creating the object set the ref
    // count to 1 and QueryInterface incremented it. Since it's only being used
    // externally, the ref count should only be 1.
    pClassFactory->Release();

    // Return the result from QueryInterface.
    return hResult;
}

HRESULT CreateRegistryString(LPCTSTR keypath, LPCTSTR name, LPCTSTR value)
{
    HRESULT result = SELFREG_E_CLASS;
    HKEY hKey;
    LONG lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
        keypath,
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_WRITE,
        NULL,
        &hKey,
        NULL);

    if (NOERROR == lResult)
    {
        lResult = RegSetValueEx(hKey,
            name,
            0,
            REG_SZ,
            (LPBYTE)value,
            (DWORD)_tcslen(value) * sizeof(TCHAR));
        if (NOERROR == lResult)
            result = S_OK;
        else
            result = SELFREG_E_CLASS;
        RegCloseKey(hKey);
    }
    else
        result = SELFREG_E_CLASS;
    return result;
}

STDAPI DllRegisterServer(void)
{
    HRESULT result = SELFREG_E_CLASS;

    // check if we're on XP or later
    OSVERSIONINFOEX osex = {0};
    osex.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    GetVersionEx((OSVERSIONINFO *)&osex);
    bool bIsWindowsXPorLater = ((osex.dwMajorVersion > 5) || ((osex.dwMajorVersion == 5) && (osex.dwMinorVersion >= 1)));

    if (!bIsWindowsXPorLater)
        return result;

    // Register the desk band object.
    if (!RegisterServer(CLSID_StExBand, _T("S&tExBar")))
        return result;

    // Register the component categories for the desk band object.
    if (!RegisterComCat(CLSID_StExBand, CATID_DeskBand))
        return result;

    // Register the object in explorer
    LPWSTR pwsz;
    TCHAR  szCLSID[MAX_PATH];
    // Get the CLSID in string form.
    if (SUCCEEDED(result = StringFromIID(CLSID_StExBand, &pwsz)))
    {
        if (pwsz)
        {
            if (SUCCEEDED(result = StringCchCopy(szCLSID, MAX_PATH, pwsz)))
            {
                CreateRegistryString(_T("Software\\Microsoft\\Internet Explorer\\Toolbar"), szCLSID, _T("StExBar"));

                CreateRegistryString(_T("Software\\Classes\\Drive\\shellex\\ContextMenuHandlers\\StExBar"), NULL, szCLSID);
                CreateRegistryString(_T("Software\\Classes\\Directory\\shellex\\ContextMenuHandlers\\StExBar"), NULL, szCLSID);
                CreateRegistryString(_T("Software\\Classes\\Directory\\Background\\shellex\\ContextMenuHandlers\\StExBar"), NULL, szCLSID);
                CreateRegistryString(_T("Software\\Classes\\Folder\\shellex\\ContextMenuHandlers\\StExBar"), NULL, szCLSID);
                CreateRegistryString(_T("Software\\Classes\\*\\shellex\\ContextMenuHandlers\\StExBar"), NULL, szCLSID);
                CreateRegistryString(_T("Software\\Classes\\LibraryLocation\\shellex\\ContextMenuHandlers\\StExBar"), NULL, szCLSID);
                CreateRegistryString(_T("Software\\Classes\\LibraryFolder\\background\\shellex\\ContextMenuHandlers\\StExBar"), NULL, szCLSID);

                CreateRegistryString(_T("Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved"), szCLSID, _T("StExBar"));
            }
            // Free the string.
            LPMALLOC pMalloc;
            CoGetMalloc(1, &pMalloc);
            pMalloc->Free(pwsz);
            pMalloc->Release();
        }
        else
            result = SELFREG_E_CLASS;
    }

    return result;
}

STDAPI DllUnregisterServer(void)
{
    HRESULT res = S_OK;
    // UnRegister the component categories for the desk band object.
    if (!UnRegisterComCat(CLSID_StExBand, CATID_DeskBand))
        res = SELFREG_E_CLASS;

    // UnRegister the desk band object.
    if (!UnRegisterServer(CLSID_StExBand, _T("S&tExBar")))
        res = SELFREG_E_CLASS;

    LPWSTR   pwsz;
    TCHAR    szCLSID[MAX_PATH];
    // Get the CLSID in string form.
    if (SUCCEEDED(res = StringFromIID(CLSID_StExBand, &pwsz)))
    {
        if (pwsz)
        {
            if (SUCCEEDED(res = StringCchCopy(szCLSID, MAX_PATH, pwsz)))
            {
                SHDeleteValue(HKEY_LOCAL_MACHINE, _T("Software\\Microsoft\\Internet Explorer\\Toolbar"), szCLSID);
                SHDeleteKey(HKEY_LOCAL_MACHINE, _T("Software\\Classes\\Drive\\shellex\\ContextMenuHandlers\\StExBar"));
                SHDeleteKey(HKEY_LOCAL_MACHINE, _T("Software\\Classes\\Directory\\shellex\\ContextMenuHandlers\\StExBar"));
                SHDeleteKey(HKEY_LOCAL_MACHINE, _T("Software\\Classes\\Directory\\Background\\shellex\\ContextMenuHandlers\\StExBar"));
                SHDeleteKey(HKEY_LOCAL_MACHINE, _T("Software\\Classes\\Folder\\shellex\\ContextMenuHandlers\\StExBar"));
                SHDeleteKey(HKEY_LOCAL_MACHINE, _T("Software\\Classes\\*\\shellex\\ContextMenuHandlers\\StExBar"));
                SHDeleteKey(HKEY_LOCAL_MACHINE, _T("Software\\Classes\\LibraryLocation\\shellex\\ContextMenuHandlers\\StExBar"));
                SHDeleteKey(HKEY_LOCAL_MACHINE, _T("Software\\Classes\\LibraryFolder\\background\\shellex\\ContextMenuHandlers\\StExBar"));
                res = S_OK;
            }
            // Free the string.
            LPMALLOC pMalloc;
            CoGetMalloc(1, &pMalloc);
            pMalloc->Free(pwsz);
            pMalloc->Release();
        }
        else
            res = SELFREG_E_CLASS;
    }

    return res;
}

typedef struct{
    HKEY  hRootKey;
    LPTSTR szSubKey;        // TCHAR szSubKey[MAX_PATH];
    LPTSTR lpszValueName;
    LPTSTR szData;          // TCHAR szData[MAX_PATH];
}DOREGSTRUCT, *LPDOREGSTRUCT;

BOOL RegisterServer(CLSID clsid, LPTSTR lpszTitle)
{
    int      i;
    HKEY     hKey;
    LRESULT  lResult;
    DWORD    dwDisp;
    TCHAR    szSubKey[MAX_PATH];
    TCHAR    szCLSID[MAX_PATH];
    TCHAR    szModule[MAX_PATH];
    LPWSTR   pwsz;
    DWORD    retval;

    // Get the CLSID in string form.
    StringFromIID(clsid, &pwsz);

    if (pwsz)
    {
        hr = StringCchCopy(szCLSID, MAX_PATH, pwsz);

        // Free the string.
        LPMALLOC pMalloc;
        CoGetMalloc(1, &pMalloc);
        pMalloc->Free(pwsz);
        pMalloc->Release();
        if (FAILED(hr))
            return FALSE;
    }

    // Get this app's path and file name.
    retval = GetModuleFileName(g_hInst, szModule, MAX_PATH);
    if (retval == NULL)
        return FALSE;

    DOREGSTRUCT ClsidEntries[ ] = {HKEY_CLASSES_ROOT,
        _T("CLSID\\%38s"),
        NULL,
        lpszTitle,
        HKEY_CLASSES_ROOT,
        _T("CLSID\\%38s\\InprocServer32"),
        NULL,
        szModule,
        HKEY_CLASSES_ROOT,
        _T("CLSID\\%38s\\InprocServer32"),
        _T("ThreadingModel"),
        _T("Apartment"),
        NULL,
        NULL,
        NULL,
        NULL};

    // register the CLSID entries
    for(i = 0; ClsidEntries[i].hRootKey; i++)
    {
        // create the sub key string - for this case, insert the file extension
        if (FAILED(StringCchPrintf(szSubKey,
            MAX_PATH,
            ClsidEntries[i].szSubKey,
            szCLSID)))
            return FALSE;



        lResult = RegCreateKeyEx(ClsidEntries[i].hRootKey,
            szSubKey,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_WRITE,
            NULL,
            &hKey,
            &dwDisp);

        if (NOERROR == lResult)
        {
            TCHAR szData[MAX_PATH];

            // If necessary, create the value string.
            if (SUCCEEDED(StringCchPrintf(szData,
                MAX_PATH,
                ClsidEntries[i].szData,
                szModule)))
            {
                size_t length = 0;
                if (SUCCEEDED(StringCchLength(szData, MAX_PATH, &length)))
                {
                    lResult = RegSetValueEx(hKey,
                        ClsidEntries[i].lpszValueName,
                        0,
                        REG_SZ,
                        (LPBYTE)szData,
                        (DWORD)(length + 1) * sizeof(TCHAR));
                    if (NOERROR != lResult)
                    {
                        RegCloseKey(hKey);
                        return FALSE;
                    }
                }
                else
                {
                    RegCloseKey(hKey);
                    return FALSE;
                }
                RegCloseKey(hKey);
            }
            else
            {
                RegCloseKey(hKey);
                return FALSE;
            }
        }
        else
            return FALSE;
    }

    return TRUE;
}

BOOL UnRegisterServer(CLSID clsid, LPTSTR lpszTitle)
{
    int      i;
    TCHAR    szSubKey[MAX_PATH];
    TCHAR    szCLSID[MAX_PATH];
    TCHAR    szModule[MAX_PATH];
    LPWSTR   pwsz;
    DWORD    retval;

    // Get the CLSID in string form.
    if (FAILED(StringFromIID(clsid, &pwsz)))
        return FALSE;

    if (pwsz)
    {
        hr = StringCchCopy(szCLSID, MAX_PATH, pwsz);
        // Free the string.
        LPMALLOC pMalloc;
        CoGetMalloc(1, &pMalloc);
        pMalloc->Free(pwsz);
        pMalloc->Release();
        if (FAILED(hr))
            return FALSE;
    }

    // Get this app's path and file name.
    retval = GetModuleFileName(g_hInst, szModule, MAX_PATH);
    if (retval == NULL)
        return FALSE;

    DOREGSTRUCT ClsidEntries[ ] = {HKEY_CLASSES_ROOT,
        _T("CLSID\\%38s"),
        NULL,
        lpszTitle,
        HKEY_CLASSES_ROOT,
        _T("CLSID\\%38s\\InprocServer32"),
        NULL,
        szModule,
        HKEY_CLASSES_ROOT,
        _T("CLSID\\%38s\\InprocServer32"),
        _T("ThreadingModel"),
        _T("Apartment"),
        NULL,
        NULL,
        NULL,
        NULL};

    // deregister the CLSID entries
    for(i = 0; ClsidEntries[i].hRootKey; i++)
    {
        //create the sub key string - for this case, insert the file extension
        if (FAILED(StringCchPrintf(szSubKey,
            MAX_PATH,
            ClsidEntries[i].szSubKey,
            szCLSID)))
            return FALSE;

        SHDeleteKey(HKEY_CLASSES_ROOT, szSubKey);
    }

    return TRUE;
}

/**************************************************************************

RegisterComCat

**************************************************************************/

BOOL RegisterComCat(CLSID clsid, CATID CatID)
{
    ICatRegister   *pcr;
    HRESULT        hr = S_OK ;

    CoInitialize(NULL);

    hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_ICatRegister,
        (LPVOID*)&pcr);

    if (SUCCEEDED(hr))
    {
        hr = pcr->RegisterClassImplCategories(clsid, 1, &CatID);
        pcr->Release();
    }

    CoUninitialize();

    return SUCCEEDED(hr);
}

BOOL UnRegisterComCat(CLSID clsid, CATID CatID)
{
    ICatRegister   *pcr;
    HRESULT        hr = S_OK ;

    CoInitialize(NULL);

    hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_ICatRegister,
        (LPVOID*)&pcr);

    if (SUCCEEDED(hr))
    {
        hr = pcr->UnRegisterClassImplCategories(clsid, 1, &CatID);

        pcr->Release();
    }

    CoUninitialize();

    return S_OK;
}
