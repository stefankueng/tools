// AAClr - tool to adjust the aero colors according to the desktop wallpaper

// Copyright (C) 2011-2013 - Stefan Kueng

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
#include "Utils.h"

#include <objbase.h>
#include <windows.h>
#include <stdio.h>
#include <wbemidl.h>
#include <comdef.h>


CUtils::CUtils(void)
{
}

CUtils::~CUtils(void)
{
}

int CUtils::GetBrightness()
{
    int ret = -1;

    IWbemLocator *pLocator = NULL;
    IWbemServices *pNamespace = 0;
    IEnumWbemClassObject *pEnum = NULL;
    HRESULT hr = S_OK;

    BSTR path = SysAllocString(L"root\\wmi");
    BSTR ClassPath = SysAllocString(L"WmiMonitorBrightness");
    BSTR bstrQuery = SysAllocString(L"Select * from WmiMonitorBrightness");

    if (!path || !ClassPath)
    {
        goto cleanup;
    }


    // Initialize COM and connect up to CIMOM

    hr = CoInitialize(0);
    if (FAILED(hr))
    {
        goto cleanup;
    }

    //  NOTE:
    //  When using asynchronous WMI API's remotely in an environment where the "Local System" account
    //  has no network identity (such as non-Kerberos domains), the authentication level of
    //  RPC_C_AUTHN_LEVEL_NONE is needed. However, lowering the authentication level to
    //  RPC_C_AUTHN_LEVEL_NONE makes your application less secure. It is wise to
    // use semi-synchronous API's for accessing WMI data and events instead of the asynchronous ones.

    hr = CoInitializeSecurity ( NULL, -1, NULL, NULL,
         RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
         RPC_C_IMP_LEVEL_IMPERSONATE,
         NULL,
         EOAC_SECURE_REFS, //change to EOAC_NONE if you change dwAuthnLevel to RPC_C_AUTHN_LEVEL_NONE
         NULL );

    hr = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
                          IID_IWbemLocator, (LPVOID *) &pLocator);
    if (FAILED(hr))
    {
        goto cleanup;
    }
    hr = pLocator->ConnectServer(path, NULL, NULL, NULL, 0, NULL, NULL, &pNamespace);
    if(hr != WBEM_S_NO_ERROR)
    {
        goto cleanup;
    }


    hr = CoSetProxyBlanket(pNamespace,
         RPC_C_AUTHN_WINNT,
         RPC_C_AUTHZ_NONE,
         NULL,
         RPC_C_AUTHN_LEVEL_PKT,
         RPC_C_IMP_LEVEL_IMPERSONATE,
         NULL,
         EOAC_NONE
         );

    if(hr != WBEM_S_NO_ERROR)
    {
        goto cleanup;
    }


    hr =pNamespace->ExecQuery(_bstr_t(L"WQL"), //Query Language
        bstrQuery, //Query to Execute
        WBEM_FLAG_RETURN_IMMEDIATELY, //Make a semi-synchronous call
        NULL, //Context
        &pEnum //Enumeration Interface
        );

    if(hr != WBEM_S_NO_ERROR)
    {
        goto cleanup;
    }

    hr = WBEM_S_NO_ERROR;

    while (WBEM_S_NO_ERROR == hr)
    {

        ULONG ulReturned;
        IWbemClassObject *pObj;

        //Get the Next Object from the collection
        hr = pEnum->Next(WBEM_INFINITE, //Timeout
             1, //No of objects requested
             &pObj, //Returned Object
             &ulReturned //No of object returned
             );

        if(hr != WBEM_S_NO_ERROR)
        {
            goto cleanup;
        }

        VARIANT var1;
        hr = pObj->Get(_bstr_t(L"CurrentBrightness"),0,&var1,NULL,NULL);

        ret = V_UI1(&var1);

        VariantClear(&var1);
        if(hr != WBEM_S_NO_ERROR)
        {
            goto cleanup;
        }
    }

    // Free up resources
cleanup:

    SysFreeString(path);
    SysFreeString(ClassPath);
    SysFreeString(bstrQuery);

    if (pLocator)
        pLocator->Release();
    if (pNamespace)
        pNamespace->Release();

    CoUninitialize();

    return ret;
}

bool CUtils::SetBrightness(int val)
{
    bool bRet = true;

    IWbemLocator *pLocator = NULL;
    IWbemServices *pNamespace = 0;
    IWbemClassObject * pClass = NULL;
    IWbemClassObject * pInClass = NULL;
    IWbemClassObject * pInInst = NULL;
    IEnumWbemClassObject *pEnum = NULL;
    HRESULT hr = S_OK;

    BSTR path = SysAllocString(L"root\\wmi");
    BSTR ClassPath = SysAllocString(L"WmiMonitorBrightnessMethods");
    BSTR MethodName = SysAllocString(L"WmiSetBrightness");
    BSTR ArgName0 = SysAllocString(L"Timeout");
    BSTR ArgName1 = SysAllocString(L"Brightness");
    BSTR bstrQuery = SysAllocString(L"Select * from WmiMonitorBrightnessMethods");

    if (!path || ! ClassPath || !MethodName || ! ArgName0)
    {
        bRet = false;
        goto cleanup;
    }


    // Initialize COM and connect up to CIMOM

    hr = CoInitialize(0);
    if (FAILED(hr))
    {
        bRet = false;
        goto cleanup;
    }

    //  NOTE:
    //  When using asynchronous WMI API's remotely in an environment where the "Local System" account
    //  has no network identity (such as non-Kerberos domains), the authentication level of
    //  RPC_C_AUTHN_LEVEL_NONE is needed. However, lowering the authentication level to
    //  RPC_C_AUTHN_LEVEL_NONE makes your application less secure. It is wise to
    // use semi-synchronous API's for accessing WMI data and events instead of the asynchronous ones.

    hr = CoInitializeSecurity ( NULL, -1, NULL, NULL,
         RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
         RPC_C_IMP_LEVEL_IMPERSONATE,
         NULL,
         EOAC_SECURE_REFS, //change to EOAC_NONE if you change dwAuthnLevel to RPC_C_AUTHN_LEVEL_NONE
         NULL );

    hr = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
                          IID_IWbemLocator, (LPVOID *) &pLocator);
    if (FAILED(hr))
    {
        bRet = false;
        goto cleanup;
    }
    hr = pLocator->ConnectServer(path, NULL, NULL, NULL, 0, NULL, NULL, &pNamespace);
    if(hr != WBEM_S_NO_ERROR)
    {
        bRet = false;
        goto cleanup;
    }


    hr = CoSetProxyBlanket(pNamespace,
         RPC_C_AUTHN_WINNT,
         RPC_C_AUTHZ_NONE,
         NULL,
         RPC_C_AUTHN_LEVEL_PKT,
         RPC_C_IMP_LEVEL_IMPERSONATE,
         NULL,
         EOAC_NONE
         );

    if(hr != WBEM_S_NO_ERROR)
    {
        bRet = false;
        goto cleanup;
    }


    hr = pNamespace->ExecQuery(_bstr_t(L"WQL"), //Query Language
         bstrQuery, //Query to Execute
         WBEM_FLAG_RETURN_IMMEDIATELY, //Make a semi-synchronous call
         NULL, //Context
         &pEnum //Enumeration Interface
         );

    if(hr != WBEM_S_NO_ERROR)
    {
        bRet = false;
        goto cleanup;
    }

    hr = WBEM_S_NO_ERROR;

    while (WBEM_S_NO_ERROR == hr)
    {

        ULONG ulReturned;
        IWbemClassObject *pObj;

        //Get the Next Object from the collection
        hr = pEnum->Next(WBEM_INFINITE, //Timeout
             1, //No of objects requested
             &pObj, //Returned Object
             &ulReturned //No of object returned
             );

        if(hr != WBEM_S_NO_ERROR)
        {
            bRet = false;
            goto cleanup;
        }

        // Get the class object
        hr = pNamespace->GetObject(ClassPath, 0, NULL, &pClass, NULL);
        if(hr != WBEM_S_NO_ERROR)
        {
            bRet = false;
            goto cleanup;
        }

        // Get the input argument and set the property
        hr = pClass->GetMethod(MethodName, 0, &pInClass, NULL);
        if(hr != WBEM_S_NO_ERROR)
        {
            bRet = false;
            goto cleanup;
        }

        hr = pInClass->SpawnInstance(0, &pInInst);
        if(hr != WBEM_S_NO_ERROR)
        {
            bRet = false;
            goto cleanup;
        }

        VARIANT var1;
        VariantInit(&var1);

        V_VT(&var1) = VT_BSTR;
        V_BSTR(&var1) = SysAllocString(L"0");
        hr = pInInst->Put(ArgName0, 0, &var1, CIM_UINT32); //CIM_UINT64

        //var1.vt = VT_I4;
        //var1.ullVal = 0;
        //   hr = pInInst->Put(ArgName0, 0, &var1, 0);
        VariantClear(&var1);
        if(hr != WBEM_S_NO_ERROR)
        {
            bRet = false;
            goto cleanup;
        }

        VARIANT var;
        VariantInit(&var);

        V_VT(&var) = VT_BSTR;
        WCHAR buf[10]={0};
        _stprintf_s(buf, _countof(buf), L"%d", val);
        V_BSTR(&var) = SysAllocString(buf);
        hr = pInInst->Put(ArgName1, 0, &var, CIM_UINT8);

        //var.vt=VT_UI1;
        //var.uiVal = 100;
        //hr = pInInst->Put(ArgName1, 0, &var, 0);
        VariantClear(&var);
        if(hr != WBEM_S_NO_ERROR)
        {
            bRet = false;
            goto cleanup;
        }
        // Call the method

        VARIANT pathVariable;
        VariantInit(&pathVariable);

        hr = pObj->Get(_bstr_t(L"__PATH"),0,&pathVariable,NULL,NULL);
        if(hr != WBEM_S_NO_ERROR)
            goto cleanup;

        hr =pNamespace->ExecMethod(pathVariable.bstrVal, MethodName, 0, NULL, pInInst,NULL, NULL);
        VariantClear(&pathVariable);
        if(hr != WBEM_S_NO_ERROR)
        {
            bRet = false;
            goto cleanup;
        }

    }

    // Free up resources
cleanup:

    SysFreeString(path);
    SysFreeString(ClassPath);
    SysFreeString(MethodName);
    SysFreeString(ArgName0);
    SysFreeString(ArgName1);
    SysFreeString(bstrQuery);

    if (pClass)
        pClass->Release();
    if (pInInst)
        pInInst->Release();
    if (pInClass)
        pInClass->Release();
    if (pLocator)
        pLocator->Release();
    if (pNamespace)
        pNamespace->Release();

    CoUninitialize();

    return bRet;
}
