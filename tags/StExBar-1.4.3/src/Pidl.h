#pragma once

class CPidl
{
public:
	/// returns the next ID in the list
	static LPITEMIDLIST			GetNextItemID(LPCITEMIDLIST pidl);
	/// returns the size of the pidl
	static UINT					GetSize(LPCITEMIDLIST pidl);
	/// creates a copy of a pidl. The returned pidl must be freed with CoTaskMemFree
	static LPITEMIDLIST			MakeCopy(LPCITEMIDLIST pidl);
	/// removes the child part from a pidl
	static BOOL					GetParentID(LPITEMIDLIST pidl);
	/// appends two pidls. The returned pidl must be freed with CoTaskMemFree
	static LPITEMIDLIST			Append(LPITEMIDLIST pidlBase, LPCITEMIDLIST pidlAdd);

};