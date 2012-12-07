// StExBar - an explorer toolbar

// Copyright (C) 2008, 2012 - Stefan Kueng

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
#include "InfoDlg.h"

#include <mshtmhst.h>

#pragma comment(lib, "Urlmon.lib")

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CInfoDlg::CInfoDlg()
{
}

CInfoDlg::~CInfoDlg()
{
}

//Function which takes input of An HTML Resource Id
BOOL CInfoDlg::ShowDialog(UINT idAboutHTMLID, HINSTANCE hInstance)
{
    //Load the IE Specific MSTML Interface DKK
    HINSTANCE hinstMSHTML = LoadLibrary(_T("MSHTML.DLL"));
    BOOL bSuccess = FALSE;
    if(hinstMSHTML)
    {
        SHOWHTMLDIALOGFN *pfnShowHTMLDialog;
        //Locate The Function ShowHTMLDialog in the Loaded MSHTML.DLL
        pfnShowHTMLDialog = (SHOWHTMLDIALOGFN*)GetProcAddress(hinstMSHTML, "ShowHTMLDialog");
        if(pfnShowHTMLDialog)
        {
            LPTSTR lpszModule = new TCHAR[MAX_PATH];
            //Get The Application Path
            if (GetModuleFileName(hInstance, lpszModule, _countof(lpszModule)))
            {
                //Add the IE Res protocol
                TCHAR strResourceURL[MAX_PATH*4];
                _stprintf_s(strResourceURL, _countof(strResourceURL), _T("res://%s/%d"), lpszModule, idAboutHTMLID);
                size_t iLength = _tcslen(strResourceURL);
                //Attempt to Create the URL Moniker to the specified in the URL String
                IMoniker *pmk;
                if(SUCCEEDED(CreateURLMoniker(NULL,strResourceURL,&pmk)))
                {
                    //Invoke the ShowHTMLDialog function by pointer
                    //passing the HWND of your Application , the Moniker,
                    //the remaining parameters can be set to NULL
                    pfnShowHTMLDialog(NULL,pmk,NULL,L"resizable:yes",NULL);
                    bSuccess = TRUE;
                }
            }
            delete [] lpszModule;
        }
        FreeLibrary(hinstMSHTML);
    }
    return bSuccess;
}
