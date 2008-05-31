#include "stdafx.h"
#include "Pidl.h"


LPITEMIDLIST CPidl::GetNextItemID(LPCITEMIDLIST pidl)
{
	// Check for valid pidl.
	if(pidl == NULL)
		return NULL;

	// Get the size of the specified item identifier. 
	int cb = pidl->mkid.cb; 

	// If the size is zero, it is the end of the list. 
	if (cb == 0) 
		return NULL; 

	// Add cb to pidl (casting to increment by bytes). 
	pidl = (LPITEMIDLIST) (((LPBYTE) pidl) + cb); 

	// Return NULL if it is null-terminating, or a pidl otherwise. 
	return (pidl->mkid.cb == 0) ? NULL : (LPITEMIDLIST) pidl; 
}

UINT CPidl::GetSize(LPCITEMIDLIST pidl)
{
	UINT cbTotal = 0;
	if (pidl)
	{
		cbTotal += sizeof(pidl->mkid.cb);    // Terminating null character
		while (pidl)
		{
			cbTotal += pidl->mkid.cb;
			pidl = GetNextItemID(pidl);
		}
	}
	return cbTotal;
}

LPITEMIDLIST CPidl::MakeCopy(LPCITEMIDLIST pidl)
{
	UINT cb = 0;

	// Calculate size of list.
	cb = GetSize(pidl);

	LPITEMIDLIST pidlRet = (LPITEMIDLIST)CoTaskMemAlloc(cb);
	if (pidlRet)
		CopyMemory(pidlRet, pidl, cb);
	return pidlRet;
}

BOOL CPidl::GetParentID(LPITEMIDLIST pidl)
{
	BOOL fRemoved = FALSE;

	// Make sure it's a valid PIDL.
	if (pidl == NULL)
		return(FALSE);

	if (pidl->mkid.cb)
	{
		LPITEMIDLIST pidlNext = pidl;
		while (pidlNext)
		{
			pidl = pidlNext;
			pidlNext = GetNextItemID(pidl);
		}
		// Remove the last one, insert terminating null character.
		pidl->mkid.cb = 0; 
		fRemoved = TRUE;
	}

	return fRemoved;
}

LPITEMIDLIST CPidl::Append(LPITEMIDLIST pidlBase, LPCITEMIDLIST pidlAdd)
{
	if(pidlBase == NULL)
		return NULL;
	if(pidlAdd == NULL)
		return pidlBase;

	LPITEMIDLIST pidlNew;

	UINT cb1 = GetSize(pidlBase) - sizeof(pidlBase->mkid.cb);
	UINT cb2 = GetSize(pidlAdd);

	pidlNew = (LPITEMIDLIST)CoTaskMemAlloc(cb1 + cb2);
	if (pidlNew)
	{
		CopyMemory(pidlNew, pidlBase, cb1);
		CopyMemory(((LPSTR)pidlNew) + cb1, pidlAdd, cb2);
	}
	return pidlNew;
}

