// tabspace - converts tabs to spaces and vice-versa in multiple files

// Copyright (C) 2011 - Stefan Kueng

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
#include <string>

/**
 * Enumerates over a directory tree, non-recursively.
 * Advantages over CFileFind:
 * 1) Return values are not broken.  An error return from
 *    CFileFind::FindNext() indicates that the *next*
 *    call to CFileFind::FindNext() will fail.
 *    A failure from CSimpleFileFind means *that* call
 *    failed, which is what I'd expect.
 * 2) Error handling.  If you use CFileFind, you are
 *    required to call ::GetLastError() yourself, there
 *    is no abstraction.
 * 3) Support for ignoring the "." and ".." directories
 *    automatically.
 * 4) No dynamic memory allocation.
 */
class CSimpleFileFind {
private:
    /**
     * Windows FindFirstFile() handle.
     */
    HANDLE m_hFindFile;

    /**
     * Windows error code - if all is well, ERROR_SUCCESS.
     * At end of directory, ERROR_NO_MORE_FILES.
     */
    DWORD m_dError;

    /**
     * Flag indicating that FindNextFile() has not yet been
     * called.
     */
    BOOL m_bFirst;

    /**
     * Flag indicating whether CSimpleFileFind was started for a file
     */
    BOOL m_bFile;
protected:
    /**
     * The prefix for files in this directory.
     * Ends with a "\", unless it's a drive letter only
     * ("C:" is different from "C:\", and "C:filename" is
     * legal anyway.)
     */
    std::wstring m_sPathPrefix;

    /**
     * The file data returned by FindFirstFile()/FindNextFile().
     */
    WIN32_FIND_DATA m_FindFileData;

public:

    /**
     * Constructor.
     *
     * \param sPath    The path to search in.
     * \param sPattern The filename pattern - default all files.
     */
    CSimpleFileFind(const std::wstring &sPath, LPCTSTR pPattern = _T("*.*"));
    ~CSimpleFileFind();

    /**
     * Advance to the next file.
     * Note that the state of this object is undefined until
     * this method is called the first time.
     *
     * \return TRUE if a file was found, FALSE on error or
     * end-of-directory (use IsError() and IsPastEnd() to
     * disambiguate).
     */
    bool FindNextFile();

    /**
     * Advance to the next file, ignoring the "." and ".."
     * pseudo-directories (if seen).
     *
     * Behaves like FindNextFile(), apart from ignoring "."
     * and "..".
     *
     * \return TRUE if a file was found, FALSE on error or
     * end-of-directory.
     */
    bool FindNextFileNoDots();

    /**
     * Advance to the next file, ignoring all directories.
     *
     * Behaves like FindNextFile(), apart from ignoring
     * directories.
     *
     * \return TRUE if a file was found, FALSE on error or
     * end-of-directory.
     */
    bool FindNextFileNoDirectories();

    /**
     * Get the Windows error code.
     * Only useful when IsError() returns true.
     *
     * \return Windows error code.
     */
    inline DWORD GetError() const
    {
        return m_dError;
    }

    /**
     * Check if the current file data is valid.
     * (I.e. there has not been an error and we are not past
     * the end of the directory).
     *
     * \return TRUE iff the current file data is valid.
     */
    inline bool IsValid() const
    {
        return (m_dError == ERROR_SUCCESS);
    }

    /**
     * Check if we have passed the end of the directory.
     *
     * \return TRUE iff we have passed the end of the directory.
     */
    inline BOOL IsPastEnd() const
    {
        return (m_dError == ERROR_NO_MORE_FILES);
    }

    /**
     * Check if there has been an unexpected error - i.e.
     * any error other than passing the end of the directory.
     *
     * \return TRUE iff there has been an unexpected error.
     */
    inline bool IsError() const
    {
        return (m_dError != ERROR_SUCCESS)
               && (m_dError != ERROR_NO_MORE_FILES);
    }

    /**
     * Check if the current file is a directory.
     *
     * \return TRUE iff the current file is a directory.
     */
    inline bool IsDirectory() const
    {
        return !!(m_FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
    }

    /**
     * Get the current file name (excluding the path).
     *
     * \return the current file name.
     */
    inline std::wstring GetFileName() const
    {
        return m_FindFileData.cFileName;
    }

    /*
     * Get the current file name, including the path.
     *
     * \return the current file path.
     */
    inline std::wstring GetFilePath() const
    {
        if (m_bFile)
            return m_sPathPrefix;
        return m_sPathPrefix + m_FindFileData.cFileName;
    }

    /**
     * Check if the current file is the "." or ".."
     * pseudo-directory.
     *
     * \return TRUE iff the current file is the "." or ".."
     * pseudo-directory.
     */
    inline bool IsDots() const
    {
        return IsDirectory()
               && m_FindFileData.cFileName[0] == _T('.')
               && ((m_FindFileData.cFileName[1] == 0)
                   || (m_FindFileData.cFileName[1] == _T('.')
                       && m_FindFileData.cFileName[2] == 0));
    }
};

/**
 * Enumerates over a directory tree, recursively.
 */
class CDirFileEnum
{
private:

    class CDirStackEntry : public CSimpleFileFind {
    public:
        CDirStackEntry(CDirStackEntry * seNext, const std::wstring& sDirName);
        ~CDirStackEntry();

        CDirStackEntry * m_seNext;
    };

    CDirStackEntry * m_seStack;
    bool m_bIsNew;

    inline void PopStack();
    inline void PushStack(const std::wstring& sDirName);

public:
    /**
     * Iterate through the specified directory and all subdirectories.
     * It does not matter whether or not the specified directory ends
     * with a slash.  Both relative and absolute paths are allowed,
     * the results of this iterator will be consistent with the style
     * passed to this constructor.
     *
     * @param dirName The directory to search in.
     */
    CDirFileEnum(const std::wstring& dirName);

    /**
     * Destructor.  Frees all resources.
     */
    ~CDirFileEnum();

    /**
     * Get the next file from this iterator.
     *
     * \param  result On successful return, holds the full path to the found
     *                file. (If this function returns FALSE, the value of
     *                result is unspecified).
     * \param  pbIsDirectory Pointer to a bool variable which will hold
     *                TRUE if the \c result path is a directory, FALSE
     *                if it's a file. Pass NULL if you don't need that information.
     * \param  recurse true if recursing into subdirectories is requested.
     * \return TRUE iff a file was found, false at end of the iteration.
     */
    bool NextFile(std::wstring &result, bool* pbIsDirectory, bool recurse = true);
};
