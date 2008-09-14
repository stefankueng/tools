#include "StdAfx.h"
#include "shlwapi.h"
#include "DirFileEnum.h"

#pragma comment(lib, "shlwapi")

CSimpleFileFind::CSimpleFileFind(const wstring &sPath, LPCTSTR pPattern) :
m_dError(ERROR_SUCCESS),
m_bFirst(true),
m_sPathPrefix(sPath)
{
	if (PathIsDirectory(sPath.c_str()))
	{
		// Add a trailing \ to m_sPathPrefix if it is missing.
		// Do not add one to "C:" since "C:" and "C:\" are different.
		{
			int len = (int)m_sPathPrefix.size();
			if (len != 0) {
				TCHAR ch = sPath[len-1];
				if (ch != '\\' && (ch != ':' || len != 2)) {
					m_sPathPrefix += '\\';
				}
			}
		}
		m_hFindFile = ::FindFirstFile(wstring(m_sPathPrefix + pPattern).c_str(), &m_FindFileData);
		m_bFile = FALSE;
	}
	else
	{
		m_hFindFile = ::FindFirstFile(m_sPathPrefix.c_str(), &m_FindFileData); 
		m_bFile = TRUE;
	}
	if (m_hFindFile == INVALID_HANDLE_VALUE) {
		m_dError = ::GetLastError();
	}
}

CSimpleFileFind::~CSimpleFileFind()
{
	if (m_hFindFile != INVALID_HANDLE_VALUE) {
		::FindClose(m_hFindFile);
	}
}

bool CSimpleFileFind::FindNextFile()
{
	if (m_dError) {
		return false;
	}

	if (m_bFirst) {
		m_bFirst = false;
		return true;
	}

	if (!::FindNextFile(m_hFindFile, &m_FindFileData)) {
		m_dError = ::GetLastError();
		return false;
	}

	return true;
}

bool CSimpleFileFind::FindNextFileNoDots()
{
	bool result;
	do {
		result = FindNextFile();
	} while (result && IsDots());

	return result;
}

bool CSimpleFileFind::FindNextFileNoDirectories()
{
	bool result;
	do {
		result = FindNextFile();
	} while (result && IsDirectory());

	return result;
}


/*
* Implementation notes:
*
* This is a depth-first traversal.  Directories are visited before
* their contents.
*
* We keep a stack of directories.  The deepest directory is at the top
* of the stack, the originally-requested directory is at the bottom.
* If we come across a directory, we first return it to the user, then
* recurse into it.  The finder at the bottom of the stack always points
* to the file or directory last returned to the user (except immediately
* after creation, when the finder points to the first valid thing we need
* to return, but we haven't actually returned anything yet - hence the
* m_bIsNew variable).
*
* Errors reading a directory are assumed to be end-of-directory, and
* are otherwise ignored.
*
* The "." and ".." psedo-directories are ignored for obvious reasons.
*/


CDirFileEnum::CDirStackEntry::CDirStackEntry(CDirStackEntry * seNext,
											 const wstring& sDirName)
											 : CSimpleFileFind(sDirName),
											 m_seNext(seNext)
{
}

CDirFileEnum::CDirStackEntry::~CDirStackEntry()
{
}

inline void CDirFileEnum::PopStack()
{
	CDirStackEntry * seToDelete = m_seStack;
	m_seStack = seToDelete->m_seNext;
	delete seToDelete;
}

inline void CDirFileEnum::PushStack(const wstring& sDirName)
{
	m_seStack = new CDirStackEntry(m_seStack,sDirName);
}

CDirFileEnum::CDirFileEnum(const wstring& sDirName) :
m_seStack(NULL),
m_bIsNew(true)
{
	PushStack(sDirName);
}

CDirFileEnum::~CDirFileEnum()
{
	while (m_seStack != NULL) {
		PopStack();
	}
}

bool CDirFileEnum::NextFile(wstring &sResult, bool* pbIsDirectory, bool recurse)
{
	if (m_bIsNew) {
		// Special-case first time - haven't found anything yet,
		// so don't do recurse-into-directory check.
		m_bIsNew = false;
	} else if (m_seStack && m_seStack->IsDirectory() && recurse) {
		PushStack(m_seStack->GetFilePath());
	}

	while (m_seStack && !m_seStack->FindNextFileNoDots()) {
		// No more files in this directory, try parent.
		PopStack();
	}

	if (m_seStack) 
	{
		sResult = m_seStack->GetFilePath();
		if(pbIsDirectory != NULL)
		{
			*pbIsDirectory = m_seStack->IsDirectory();
		}
		return true;
	} else {
		return false;
	}
}
