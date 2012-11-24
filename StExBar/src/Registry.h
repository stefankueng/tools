// StExBar - an explorer toolbar

// Copyright (C) 2007-2008 - Stefan Kueng

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
#include <list>
#include "shlwapi.h"
#ifdef _MFC_VER

/**
 * \ingroup CommonClasses
 * Base class for the registry classes.
 *
 * \par requirements
 * - win98 or later, win2k or later, win95 with IE4 or later, winNT4 with IE4 or later
 * - import library Shlwapi.lib
 */
class CRegBase
{
public: //methods
    /**
     * Removes the whole registry key including all values. So if you set the registry
     * entry to be HKCU\Software\Company\Product\key\value there will only be
     * HKCU\Software\Company\Product key in the registry.
     * \return ERROR_SUCCESS or an nonzero errorcode. Use FormatMessage() to get an error description.
     */
    DWORD removeKey() { RegOpenKeyEx(m_base, m_path, 0, KEY_WRITE, &m_hKey); return SHDeleteKey(m_base, (LPCTSTR)m_path); }
    /**
     * Removes the value of the registry object. If you set the registry entry to
     * be HKCU\Software\Company\Product\key\value there will only be
     * HKCU\Software\Company\Product\key\ in the registry.
     * \return ERROR_SUCCESS or an nonzero errorcode. Use FormatMessage() to get an error description.
     */
    LONG removeValue() { RegOpenKeyEx(m_base, m_path, 0, KEY_WRITE, &m_hKey); return RegDeleteValue(m_hKey, (LPCTSTR)m_key); }

    /**
     * Returns the string of the last error occurred.
     */
    CString getErrorString()
    {
        LPVOID lpMsgBuf;

        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            LastError,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR) &lpMsgBuf,
            0, NULL );
#if defined IDS_REG_ERROR
        CString sTemp;
        sTemp.Format(IDS_REG_ERROR, m_path, (LPCTSTR)lpMsgBuf);
        return sTemp;
#else
        return CString((LPCTSTR)lpMsgBuf);
#endif
    };

public: //members
    HKEY m_base;        ///< handle to the registry base
    HKEY m_hKey;        ///< handle to the open registry key
    CString m_key;      ///< the name of the value
    CString m_path;     ///< the path to the key
    LONG LastError;     ///< the value of the last error occurred
};

/**
 * \ingroup CommonClasses
 * DWORD value in registry. with this class you can use DWORD values in registry
 * like normal DWORD variables in your program.
 * Usage:
 * in your header file, declare your registry DWORD variable:
 * \code
 * CRegDWORD regvalue;
 * \endcode
 * next initialize the variable e.g. in the constructor of your class:
 * \code
 * regvalue = CRegDWORD("Software\\Company\\SubKey\\MyValue", 100);
 * \endcode
 * this will set the registry value "MyValue" under HKEY_CURRENT_USER with path
 * "Software\Company\SubKey" to the variable. If the key does not yet exist or
 * an error occured during read from the registry, a default
 * value of 100 is used when accessing the variable.
 * now the variable can be used like any other DWORD variable:
 * \code
 * regvalue = 200;                      //stores the value 200 in the registry
 * int temp = regvalue + 300;           //temp has value 500 now
 * regvalue += 300;                     //now the registry has the value 500 too
 * \endcode
 * to avoid too much access to the registry the value is cached inside the object.
 * once the value is read, no more read accesses to the registry will be made.
 * this means the variable will contain a wrong value if the corresponding registry
 * entry is changed by anything else than this variable! If you think that could happen
 * then use
 * \code
 * regvalue.read();
 * \endcode
 * to force a refresh of the variable with the registry.
 * a write to the registry is only made if the new value assigned with the variable
 * is different than the last assigned value.
 * to force a write use the method write();
 * another option to force reads and writes to the registry is to specify TRUE as the
 * third parameter in the constructor.
 */
class CRegDWORD : public CRegBase
{
public:
    CRegDWORD(void);
    /**
     * Constructor.
     * \param key the path to the key, including the key. example: "Software\\Company\\SubKey\\MyValue"
     * \param def the default value used when the key does not exist or a read error occured
     * \param force set to TRUE if no cache should be used, i.e. always read and write directly from/to registry
     * \param base a predefined base key like HKEY_LOCAL_MACHINE. see the SDK documentation for more information.
     */
    CRegDWORD(const CString& key, DWORD def = 0, BOOL force = FALSE, HKEY base = HKEY_CURRENT_USER);
    ~CRegDWORD(void);
    /**
     * reads the assigned value from the registry. Use this method only if you think the registry
     * value could have been altered without using the CRegDWORD object.
     * \return the read value
     */
    DWORD   read();                     ///< reads the value from the registry
    void    write();                    ///< writes the value to the registry

    operator DWORD();
    CRegDWORD& operator=(DWORD d);
    CRegDWORD& operator+=(DWORD d) { return *this = *this + d;}
    CRegDWORD& operator-=(DWORD d) { return *this = *this - d;}
    CRegDWORD& operator*=(DWORD d) { return *this = *this * d;}
    CRegDWORD& operator/=(DWORD d) { return *this = *this / d;}
    CRegDWORD& operator%=(DWORD d) { return *this = *this % d;}
    CRegDWORD& operator<<=(DWORD d) { return *this = *this << d;}
    CRegDWORD& operator>>=(DWORD d) { return *this = *this >> d;}
    CRegDWORD& operator&=(DWORD d) { return *this = *this & d;}
    CRegDWORD& operator|=(DWORD d) { return *this = *this | d;}
    CRegDWORD& operator^=(DWORD d) { return *this = *this ^ d;}

protected:

    DWORD   m_value;                    ///< the cached value of the registry
    DWORD   m_defaultvalue;             ///< the default value to use
    BOOL    m_read;                     ///< indicates if the value has already been read from the registry
    BOOL    m_force;                    ///< indicates if no cache should be used, i.e. always read and write directly from registry
};

/**
 * \ingroup CommonClasses
 * CString value in registry. with this class you can use CString values in registry
 * almost like normal CString variables in your program.
 * Usage:
 * in your header file, declare your registry CString variable:
 * \code
 * CRegString regvalue;
 * \endcode
 * next initialize the variable e.g. in the constructor of your class:
 * \code
 * regvalue = CRegString("Software\\Company\\SubKey\\MyValue", "default");
 * \endcode
 * this will set the registry value "MyValue" under HKEY_CURRENT_USER with path
 * "Software\Company\SubKey" to the variable. If the key does not yet exist or
 * an error occured during read from the registry, a default
 * value of "default" is used when accessing the variable.
 * now the variable can be used like any other CString variable:
 * \code
 * regvalue = "some string";            //stores the value "some string" in the registry
 * CString temp = regvalue + "!!";      //temp has value "some string!!" now
 * \endcode
 * to use the normal methods of the CString class, just typecast the CRegString to a CString
 * and do whatever you want with the string:
 * \code
 * ((CString)regvalue).GetLength();
 * ((CString)regvalue).Trim();
 * \endcode
 * please be aware that in the second line the change in the string won't be written
 * to the registry! To force a write use the write() method. A write() is only needed
 * if you change the String with Methods not overloaded by CRegString.
 * to avoid too much access to the registry the value is cached inside the object.
 * once the value is read, no more read accesses to the registry will be made.
 * this means the variable will contain a wrong value if the corresponding registry
 * entry is changed by anything else than this variable! If you think that could happen
 * then use
 * \code
 * regvalue.read();
 * \endcode
 * to force a refresh of the variable with the registry.
 * a write to the registry is only made if the new value assigned with the variable
 * is different than the last assigned value.
 * to force a write use the method write();
 * another option to force reads and writes to the registry is to specify TRUE as the
 * third parameter in the constructor.
 */
class CRegString : public CRegBase
{
public:
    CRegString();
    /**
     * Constructor.
     * \param key the path to the key, including the key. example: "Software\\Company\\SubKey\\MyValue"
     * \param def the default value used when the key does not exist or a read error occured
     * \param force set to TRUE if no cache should be used, i.e. always read and write directly from/to registry
     * \param base a predefined base key like HKEY_LOCAL_MACHINE. see the SDK documentation for more information.
     */
    CRegString(const CString& key, const CString& def = _T(""), BOOL force = FALSE, HKEY base = HKEY_CURRENT_USER);
    ~CRegString(void);

    CString read();                     ///< reads the value from the registry
    void    write();                    ///< writes the value to the registry

    operator CString();
    CRegString& operator=(const CString& s);
    CRegString& operator+=(const CString& s) { return *this = (CString)*this + s; }

protected:

    CString m_value;                    ///< the cached value of the registry
    CString m_defaultvalue;             ///< the default value to use
    BOOL    m_read;                     ///< indicates if the value has already been read from the registry
    BOOL    m_force;                    ///< indicates if no cache should be used, i.e. always read and write directly from registry
};

/**
 * \ingroup CommonClasses
 * CRect value in registry. with this class you can use CRect values in registry
 * almost like normal CRect variables in your program.
 * Usage:
 * in your header file, declare your registry CString variable:
 * \code
 * CRegRect regvalue;
 * \endcode
 * next initialize the variable e.g. in the constructor of your class:
 * \code
 * regvalue = CRegRect("Software\\Company\\SubKey\\MyValue", CRect(100,100,200,200));
 * \endcode
 * this will set the registry value "MyValue" under HKEY_CURRENT_USER with path
 * "Software\Company\SubKey" to the variable. If the key does not yet exist or
 * an error occured during read from the registry, a default
 * value of 100,100,200,200 is used when accessing the variable.
 * now the variable can be used like any other CRect variable:
 * \code
 * regvalue = CRect(40,20,300,500);             //stores the value in the registry
 * CRect temp = regvalue + CPoint(1,1);
 * temp |= CSize(5,5);
 * \endcode
 * to use the normal methods of the CRect class, just typecast the CRegRect to a CRect
 * and do whatever you want with the rect:
 * \code
 * ((CRect)regvalue).MoveToX(100);
 * ((CRect)regvalue).DeflateRect(10,10);
 * \endcode
 * please be aware that in the second line the change in the CRect won't be written
 * to the registry! To force a write use the write() method. A write() is only needed
 * if you change the CRect with Methods not overloaded by CRegRect.
 * to avoid too much access to the registry the value is cached inside the object.
 * once the value is read, no more read accesses to the registry will be made.
 * this means the variable will contain a wrong value if the corresponding registry
 * entry is changed by anything else than this variable! If you think that could happen
 * then use
 * \code
 * regvalue.read();
 * \endcode
 * to force a refresh of the variable with the registry.
 * a write to the registry is only made if the new value assigned with the variable
 * is different than the last assigned value.
 * to force a write use the method write();
 * another option to force reads and writes to the registry is to specify TRUE as the
 * third parameter in the constructor.
 */
class CRegRect : public CRegBase
{
public:
    CRegRect();
    /**
     * Constructor.
     * \param key the path to the key, including the key. example: "Software\\Company\\SubKey\\MyValue"
     * \param def the default value used when the key does not exist or a read error occured
     * \param force set to TRUE if no cache should be used, i.e. always read and write directly from/to registry
     * \param base a predefined base key like HKEY_LOCAL_MACHINE. see the SDK documentation for more information.
     */
    CRegRect(const CString& key, CRect def = CRect(), BOOL force = FALSE, HKEY base = HKEY_CURRENT_USER);
    ~CRegRect(void);

    CRect read();                       ///< reads the value from the registry
    void    write();                    ///< writes the value to the registry

    operator CRect();
    operator LPCRECT() { return (LPCRECT)(CRect)*this; }
    operator LPRECT() { return (LPRECT)(CRect)*this; }
    CRegRect& operator=(CRect r);
    CRegRect& operator+=(POINT r) { return *this = (CRect)*this + r;}
    CRegRect& operator+=(SIZE r) { return *this = (CRect)*this + r;}
    CRegRect& operator+=(LPCRECT  r) { return *this = (CRect)*this + r;}
    CRegRect& operator-=(POINT r) { return *this = (CRect)*this - r;}
    CRegRect& operator-=(SIZE r) { return *this = (CRect)*this - r;}
    CRegRect& operator-=(LPCRECT  r) { return *this = (CRect)*this - r;}

    CRegRect& operator&=(CRect r) { return *this = r & *this;}
    CRegRect& operator|=(CRect r) { return *this = r | *this;}

protected:

    CRect   m_value;                    ///< the cached value of the registry
    CRect   m_defaultvalue;             ///< the default value to use
    BOOL    m_read;                     ///< indicates if the value has already been read from the registry
    BOOL    m_force;                    ///< indicates if no cache should be used, i.e. always read and write directly from registry
};

/**
 * \ingroup CommonClasses
 * CPoint value in registry. with this class you can use CPoint values in registry
 * almost like normal CPoint variables in your program.
 * Usage:
 * in your header file, declare your registry CPoint variable:
 * \code
 * CRegPoint regvalue;
 * \endcode
 * next initialize the variable e.g. in the constructor of your class:
 * \code
 * regvalue = CRegPoint("Software\\Company\\SubKey\\MyValue", CPoint(100,100));
 * \endcode
 * this will set the registry value "MyValue" under HKEY_CURRENT_USER with path
 * "Software\Company\SubKey" to the variable. If the key does not yet exist or
 * an error occured during read from the registry, a default
 * value of 100,100 is used when accessing the variable.
 * now the variable can be used like any other CPoint variable:
 * \code
 * regvalue = CPoint(40,20);                    //stores the value in the registry
 * CPoint temp = regvalue + CPoint(1,1);
 * temp += CSize(5,5);
 * \endcode
 * to use the normal methods of the CPoint class, just typecast the CRegPoint to a CPoint
 * and do whatever you want with the point:
 * \code
 * ((CRect)regvalue).Offset(100,10);
 * \endcode
 * please be aware that in the above example the change in the CPoint won't be written
 * to the registry! To force a write use the write() method. A write() is only needed
 * if you change the CPoint with Methods not overloaded by CRegPoint.
 * to avoid too much access to the registry the value is cached inside the object.
 * once the value is read, no more read accesses to the registry will be made.
 * this means the variable will contain a wrong value if the corresponding registry
 * entry is changed by anything else than this variable! If you think that could happen
 * then use
 * \code
 * regvalue.read();
 * \endcode
 * to force a refresh of the variable with the registry.
 * a write to the registry is only made if the new value assigned with the variable
 * is different than the last assigned value.
 * to force a write use the method write();
 * another option to force reads and writes to the registry is to specify TRUE as the
 * third parameter in the constructor.
 */
class CRegPoint : public CRegBase
{
public:
    CRegPoint();
    /**
     * Constructor.
     * \param key the path to the key, including the key. example: "Software\\Company\\SubKey\\MyValue"
     * \param def the default value used when the key does not exist or a read error occured
     * \param force set to TRUE if no cache should be used, i.e. always read and write directly from/to registry
     * \param base a predefined base key like HKEY_LOCAL_MACHINE. see the SDK documentation for more information.
     */
    CRegPoint(const CString& key, CPoint def = CPoint(), BOOL force = FALSE, HKEY base = HKEY_CURRENT_USER);
    ~CRegPoint(void);

    CPoint read();
    void    write();                    ///< writes the value to the registry

    operator CPoint();
    CRegPoint& operator=(CPoint p);

    CRegPoint& operator+=(CPoint p) { return *this = p + *this; }
    CRegPoint& operator-=(CPoint p) { return *this = p - *this; }

protected:

    CPoint  m_value;                    ///< the cached value of the registry
    CPoint  m_defaultvalue;             ///< the default value to use
    BOOL    m_read;                     ///< indicates if the value has already been read from the registry
    BOOL    m_force;                    ///< indicates if no cache should be used, i.e. always read and write directly from registry
};

/**
 * \ingroup CommonClasses
 * Manages a registry key (not a value). Provides methods to create and remove the
 * key and to query the list of values and subkeys.
 */
class CRegistryKey
{
public: //methods
    /**
     * Constructor.
     * \param key the path to the key, including the key. example: "Software\\Company\\SubKey"
     * \param base a predefined base key like HKEY_LOCAL_MACHINE. see the SDK documentation for more information.
     */
    CRegistryKey(const CString& key, HKEY base = HKEY_CURRENT_USER);
    ~CRegistryKey();

    /**
     * Creates the registry key if it does not already exist.
     * \return ERROR_SUCCESS or an nonzero errorcode. Use FormatMessage() to get an error description.
     */
    DWORD createKey();
    /**
     * Removes the whole registry key including all values. So if you set the registry
     * entry to be HKCU\Software\Company\Product\key there will only be
     * HKCU\Software\Company\Product key in the registry.
     * \return ERROR_SUCCESS or an nonzero errorcode. Use FormatMessage() to get an error description.
     */
    DWORD removeKey();

    bool getValues(CStringList& values);        ///< returns the list of values
    bool getSubKeys(CStringList& subkeys);      ///< returns the list of subkeys

public: //members
    HKEY m_base;        ///< handle to the registry base
    HKEY m_hKey;        ///< handle to the open registry key
    CString m_path;     ///< the path to the key
};
#endif

typedef std::wstring wide_string;
#ifndef stdstring
#   ifdef UNICODE
#       define stdstring wide_string
#   else
#       define stdstring std::string
#   endif
#endif

class CRegStdBase
{
public: //methods
    /**
     * Removes the whole registry key including all values. So if you set the registry
     * entry to be HKCU\Software\Company\Product\key\value there will only be
     * HKCU\Software\Company\Product key in the registry.
     * \return ERROR_SUCCESS or an nonzero errorcode. Use FormatMessage() to get an error description.
     */
    DWORD removeKey() { RegOpenKeyEx(m_base, m_path.c_str(), 0, KEY_WRITE, &m_hKey); return SHDeleteKey(m_base, m_path.c_str()); }
    /**
     * Removes the value of the registry object. If you set the registry entry to
     * be HKCU\Software\Company\Product\key\value there will only be
     * HKCU\Software\Company\Product\key\ in the registry.
     * \return ERROR_SUCCESS or an nonzero errorcode. Use FormatMessage() to get an error description.
     */
    LONG removeValue() { RegOpenKeyEx(m_base, m_path.c_str(), 0, KEY_WRITE, &m_hKey); return RegDeleteValue(m_hKey, m_key.c_str()); }

    stdstring getErrorString()
    {
        LPVOID lpMsgBuf;

        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            LastError,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR) &lpMsgBuf,
            0, NULL );
        return stdstring((LPCTSTR)lpMsgBuf);
    }
public: //members
    HKEY m_base;        ///< handle to the registry base
    HKEY m_hKey;        ///< handle to the open registry key
    stdstring m_key;        ///< the name of the value
    stdstring m_path;       ///< the path to the key
    LONG LastError;     ///< the last value of the last occurred error
};

/**
 * \ingroup CommonClasses
 * std::string value in registry. with this class you can use std::string values in registry
 * almost like normal std::string variables in your program.
 * Usage:
 * in your header file, declare your registry std::string variable:
 * \code
 * CRegStdString regvalue;
 * \endcode
 * next initialize the variable e.g. in the constructor of your class:
 * \code
 * regvalue = CRegStdString("Software\\Company\\SubKey\\MyValue", "default");
 * \endcode
 * this will set the registry value "MyValue" under HKEY_CURRENT_USER with path
 * "Software\Company\SubKey" to the variable. If the key does not yet exist or
 * an error occured during read from the registry, a default
 * value of "default" is used when accessing the variable.
 * to avoid too much access to the registry the value is cached inside the object.
 * once the value is read, no more read accesses to the registry will be made.
 * this means the variable will contain a wrong value if the corresponding registry
 * entry is changed by anything else than this variable! If you think that could happen
 * then use
 * \code
 * regvalue.read();
 * \endcode
 * to force a refresh of the variable with the registry.
 * a write to the registry is only made if the new value assigned with the variable
 * is different than the last assigned value.
 * to force a write use the method write();
 * another option to force reads and writes to the registry is to specify TRUE as the
 * third parameter in the constructor.
 */
class CRegStdString : public CRegStdBase
{
public:
    CRegStdString();
    /**
     * Constructor.
     * \param key the path to the key, including the key. example: "Software\\Company\\SubKey\\MyValue"
     * \param def the default value used when the key does not exist or a read error occured
     * \param force set to TRUE if no cache should be used, i.e. always read and write directly from/to registry
     * \param base a predefined base key like HKEY_LOCAL_MACHINE. see the SDK documentation for more information.
     */
    CRegStdString(const stdstring& key, const stdstring& def = _T(""), BOOL force = FALSE, HKEY base = HKEY_CURRENT_USER);
    ~CRegStdString(void);

    stdstring read();                       ///< reads the value from the registry
    void    write();                    ///< writes the value to the registry

    operator stdstring();
    CRegStdString& operator=(stdstring s);
    CRegStdString& operator+=(stdstring s) { return *this = (stdstring)*this + s; }
    operator LPCTSTR();

protected:

    stdstring   m_value;                ///< the cached value of the registry
    stdstring   m_defaultvalue;         ///< the default value to use
    BOOL    m_read;                     ///< indicates if the value has already been read from the registry
    BOOL    m_force;                    ///< indicates if no cache should be used, i.e. always read and write directly from registry
};

/**
 * \ingroup CommonClasses
 * DWORD value in registry. with this class you can use DWORD values in registry
 * like normal DWORD variables in your program.
 * Usage:
 * in your header file, declare your registry DWORD variable:
 * \code
 * CRegStdDWORD regvalue;
 * \endcode
 * next initialize the variable e.g. in the constructor of your class:
 * \code
 * regvalue = CRegStdDWORD("Software\\Company\\SubKey\\MyValue", 100);
 * \endcode
 * this will set the registry value "MyValue" under HKEY_CURRENT_USER with path
 * "Software\Company\SubKey" to the variable. If the key does not yet exist or
 * an error occured during read from the registry, a default
 * value of 100 is used when accessing the variable.
 * now the variable can be used like any other DWORD variable:
 * \code
 * regvalue = 200;                      //stores the value 200 in the registry
 * int temp = regvalue + 300;           //temp has value 500 now
 * regvalue += 300;                     //now the registry has the value 500 too
 * \endcode
 * to avoid too much access to the registry the value is cached inside the object.
 * once the value is read, no more read accesses to the registry will be made.
 * this means the variable will contain a wrong value if the corresponding registry
 * entry is changed by anything else than this variable! If you think that could happen
 * then use
 * \code
 * regvalue.read();
 * \endcode
 * to force a refresh of the variable with the registry.
 * a write to the registry is only made if the new value assigned with the variable
 * is different than the last assigned value.
 * to force a write use the method write();
 * another option to force reads and writes to the registry is to specify TRUE as the
 * third parameter in the constructor.
 */
class CRegStdDWORD : public CRegStdBase
{
public:
    CRegStdDWORD();
    /**
     * Constructor.
     * \param key the path to the key, including the key. example: "Software\\Company\\SubKey\\MyValue"
     * \param def the default value used when the key does not exist or a read error occured
     * \param force set to TRUE if no cache should be used, i.e. always read and write directly from/to registry
     * \param base a predefined base key like HKEY_LOCAL_MACHINE. see the SDK documentation for more information.
     */
    CRegStdDWORD(const stdstring& key, DWORD def = 0, BOOL force = FALSE, HKEY base = HKEY_CURRENT_USER);
    ~CRegStdDWORD(void);

    DWORD read();                       ///< reads the value from the registry
    void    write();                    ///< writes the value to the registry

    operator DWORD();
    CRegStdDWORD& operator=(DWORD d);
    CRegStdDWORD& operator+=(DWORD d) { return *this = *this + d;}
    CRegStdDWORD& operator-=(DWORD d) { return *this = *this - d;}
    CRegStdDWORD& operator*=(DWORD d) { return *this = *this * d;}
    CRegStdDWORD& operator/=(DWORD d) { return *this = *this / d;}
    CRegStdDWORD& operator%=(DWORD d) { return *this = *this % d;}
    CRegStdDWORD& operator<<=(DWORD d) { return *this = *this << d;}
    CRegStdDWORD& operator>>=(DWORD d) { return *this = *this >> d;}
    CRegStdDWORD& operator&=(DWORD d) { return *this = *this & d;}
    CRegStdDWORD& operator|=(DWORD d) { return *this = *this | d;}
    CRegStdDWORD& operator^=(DWORD d) { return *this = *this ^ d;}

protected:

    DWORD   m_value;                ///< the cached value of the registry
    DWORD   m_defaultvalue;         ///< the default value to use
    BOOL    m_read;                 ///< indicates if the value has already been read from the registry
    BOOL    m_force;                ///< indicates if no cache should be used, i.e. always read and write directly from registry
};

typedef std::list<stdstring> stdregistrykeylist;

/**
 * \ingroup CommonClasses
 * Manages a registry key (not a value). Provides methods to create and remove the
 * key and to query the list of values and subkeys.
 */
class CStdRegistryKey
{
public: //methods
    /**
     * Constructor.
     * \param key the path to the key, including the key. example: "Software\\Company\\SubKey"
     * \param base a predefined base key like HKEY_LOCAL_MACHINE. see the SDK documentation for more information.
     */
    CStdRegistryKey(const stdstring& key, HKEY base = HKEY_CURRENT_USER);
    ~CStdRegistryKey();

    /**
     * Creates the registry key if it does not already exist.
     * \return ERROR_SUCCESS or an nonzero errorcode. Use FormatMessage() to get an error description.
     */
    DWORD createKey();
    /**
     * Removes the whole registry key including all values. So if you set the registry
     * entry to be HKCU\Software\Company\Product\key there will only be
     * HKCU\Software\Company\Product key in the registry.
     * \return ERROR_SUCCESS or an nonzero errorcode. Use FormatMessage() to get an error description.
     */
    DWORD removeKey();

    bool getValues(stdregistrykeylist& values);         ///< returns the list of values
    bool getSubKeys(stdregistrykeylist& subkeys);       ///< returns the list of subkeys

public: //members
    HKEY m_base;            ///< handle to the registry base
    HKEY m_hKey;            ///< handle to the open registry key
    stdstring m_path;       ///< the path to the key
};


