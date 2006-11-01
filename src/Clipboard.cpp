#include "stdafx.h"
#include "SRBand.h"

bool CDeskBand::WriteStringToClipboard(const std::wstring& sClipdata, HWND hOwningWnd)
{
	// copy the string in ASCII and UNICODE to the clipboard

	// convert the unicode string to ASCII
	std::string sClipdataA;
	int len = (int)sClipdata.size();
	if (len==0)
		return false;

	int size = len*4;
	char * narrow = new char[size];
	int ret = WideCharToMultiByte(CP_ACP, 0, sClipdata.c_str(), len, narrow, size-1, NULL, NULL);
	narrow[ret] = 0;
	sClipdataA = std::string(narrow);
	delete narrow;

	if (OpenClipboard(hOwningWnd))
	{
		EmptyClipboard();
		HGLOBAL hClipboardData;
		hClipboardData = GlobalAlloc(GMEM_DDESHARE, sClipdataA.size()+1);
		if (hClipboardData)
		{
			char * pchData;
			pchData = (char*)GlobalLock(hClipboardData);
			if (pchData)
			{
				strcpy_s(pchData, sClipdataA.size()+1, sClipdataA.c_str());
				if (GlobalUnlock(hClipboardData))
				{
					if (SetClipboardData(CF_TEXT,hClipboardData)==NULL)
					{
						CloseClipboard();
						return false;
					}
				}
				else
				{
					CloseClipboard();
					return false;
				}
			}
			else
			{
				CloseClipboard();
				return false;
			}
		}
		else
		{
			CloseClipboard();
			return false;
		}

		hClipboardData = GlobalAlloc(GMEM_DDESHARE, sClipdataA.size()*sizeof(TCHAR)+2);
		if (hClipboardData)
		{
			TCHAR * pchData;
			pchData = (TCHAR*)GlobalLock(hClipboardData);
			if (pchData)
			{
				_tcscpy_s(pchData, sClipdata.size()+1, sClipdata.c_str());
				if (GlobalUnlock(hClipboardData))
				{
					if (SetClipboardData(CF_UNICODETEXT,hClipboardData)==NULL)
					{
						CloseClipboard();
						return false;
					}
				}
				else
				{
					CloseClipboard();
					return false;
				}
			}
			else
			{
				CloseClipboard();
				return false;
			}
		}
		else
		{
			CloseClipboard();
			return false;
		}
		CloseClipboard();
		return true;
	}
	return false;
}
