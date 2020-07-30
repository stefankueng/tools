// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#include <SDKDDKVer.h>

// we're a dll loaded by the shell: this define is needed to use the version 6 common controls
#define ISOLATION_AWARE_ENABLED 1

#include <windows.h>

#include <commctrl.h>
#include <Shlobj.h>
#include <Shlwapi.h>

#include <ole2.h>
#include <comcat.h>
#include <olectl.h>

#include "tstring.h"

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
    const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

MIDL_DEFINE_GUID(IID, IID_IShellFolderView, 0x37A378C0, 0xF82D, 0x11CE, 0xAE, 0x65, 0x08, 0x00, 0x2B, 0x2E, 0x12, 0x62);
