// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once
// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER                      // Allow use of features specific to Windows 95 and Windows NT 4 or later.
#   define WINVER 0x0600            // Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif

#ifndef _WIN32_WINNT                // Allow use of features specific to Windows Vista or later.
#   define _WIN32_WINNT 0x0600      // Change this to the appropriate value to target Windows Vista or later.
#endif

#ifndef _WIN32_WINDOWS              // Allow use of features specific to Vista or later.
#   define _WIN32_WINDOWS 0x0600    // Change this to the appropriate value to target Windows Vista or later.
#endif

#ifndef _WIN32_IE                   // Allow use of features specific to IE 7.0 or later.
#   define _WIN32_IE 0x700          // Change this to the appropriate value to target IE 7.0 or later.
#endif

// we're a dll loaded by the shell: this define is needed to use the version 6 common controls
#define ISOLATION_AWARE_ENABLED 1

#include <windows.h>

#include <commctrl.h>
#include <Shlobj.h>
#include <Shlwapi.h>
#include <tchar.h>

#include <ole2.h>
#include <comcat.h>
#include <olectl.h>

#include "tstring.h"
