// SKBright - tool to adjust the monitor brightness

// Copyright (C) 2010 - Stefan Kueng

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
#include <memory>
#include "shlwapi.h"

#ifndef ASSERT
#define ASSERT(x)
#endif

/**
 * \ingroup Utils
 * Base class for the registry classes.
 *
 * \par requirements 
 * - win98 or later, win2k or later, win95 with IE4 or later, winNT4 with IE4 or later
 * - import library Shlwapi.lib
 */

template<class S>
class CRegBaseCommon
{
protected:

    /**
     * String type specific operations.
     */

    virtual LPCTSTR GetPlainString (const S& s) const = 0;
    virtual DWORD GetLength (const S& s) const = 0;

public:	//methods

    /** Default constructor.
     */
    CRegBaseCommon();
	/**
	 * Constructor.
	 * \param key the path to the key, including the key. example: "Software\\Company\\SubKey\\MyValue"
	 * \param force set to TRUE if no cache should be used, i.e. always read and write directly from/to registry
	 * \param base a predefined base key like HKEY_LOCAL_MACHINE. see the SDK documentation for more information.
	 */
	CRegBaseCommon(const S& key, bool force, HKEY base = HKEY_CURRENT_USER);

	/**
	 * Removes the whole registry key including all values. So if you set the registry
	 * entry to be HKCU\Software\Company\Product\key\value there will only be
	 * HKCU\Software\Company\Product key in the registry.
	 * \return ERROR_SUCCESS or an nonzero error code. Use FormatMessage() to get an error description.
	 */
	DWORD removeKey();
	/**
	 * Removes the value of the registry object. If you set the registry entry to
	 * be HKCU\Software\Company\Product\key\value there will only be
	 * HKCU\Software\Company\Product\key\ in the registry.
	 * \return ERROR_SUCCESS or an nonzero error code. Use FormatMessage() to get an error description.
	 */
	LONG removeValue();

	/**
	 * Returns the string of the last error occurred.
	 */
	virtual S getErrorString()
	{
		LPVOID lpMsgBuf;

		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			LastError,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR) &lpMsgBuf,
			0, NULL );

		return (LPCTSTR)lpMsgBuf;
	};

    /// get failure info for last operation

    LONG GetLastError() const
    {
        return LastError;
    }

    /// used in subclass templates to specify the correct string type

    typedef S StringT;  

protected:	
    
    //members
	HKEY m_base;		///< handle to the registry base
	S m_key;		    ///< the name of the value
	S m_path;		    ///< the path to the key
	LONG LastError;		///< the value of the last error occurred

	bool m_read;		///< indicates if the value has already been read from the registry
	bool m_force;		///< indicates if no cache should be used, i.e. always read and write directly from registry
    bool m_exists;      ///< true, if the registry actually exists
};

// implement CRegBaseCommon<> members

template<class S>
CRegBaseCommon<S>::CRegBaseCommon()
    : m_base (HKEY_CURRENT_USER)
    , m_key()
    , m_path()
    , LastError (ERROR_SUCCESS)
    , m_read (false)
    , m_force (false)
    , m_exists (false)
{
}

template<class S>
CRegBaseCommon<S>::CRegBaseCommon (const S& key, bool force, HKEY base)
    : m_base (base)
	, m_key (key)
    , m_path()
    , LastError (ERROR_SUCCESS)
    , m_read (false)
    , m_force (force)
    , m_exists (false)
{
}

template<class S>
DWORD CRegBaseCommon<S>::removeKey() 
{ 
    m_exists = false;
    m_read = true;

    HKEY hKey = NULL;
    RegOpenKeyEx (m_base, GetPlainString (m_path), 0, KEY_WRITE, &hKey); 
    return SHDeleteKey(m_base, GetPlainString (m_path)); 
}

template<class S>
LONG CRegBaseCommon<S>::removeValue() 
{ 
    m_exists = false;
    m_read = true;

    HKEY hKey = NULL;
    RegOpenKeyEx(m_base, GetPlainString (m_path), 0, KEY_WRITE, &hKey); 
    return RegDeleteValue(hKey, GetPlainString (m_key)); 
}

/**
 * \ingroup Utils
 * Base class for MFC type registry classes.
 */

#ifdef __CSTRINGT_H__
class CRegBase : public CRegBaseCommon<CString>
{
protected:

    /**
     * String type specific operations.
     */

    virtual LPCTSTR GetPlainString (const CString& s) const {return (LPCTSTR)s;}
    virtual DWORD GetLength (const CString& s) const {return s.GetLength();}

public:	//methods

    /** Default constructor.
     */
    CRegBase();
	/**
	 * Constructor.
	 * \param key the path to the key, including the key. example: "Software\\Company\\SubKey\\MyValue"
	 * \param force set to TRUE if no cache should be used, i.e. always read and write directly from/to registry
	 * \param base a predefined base key like HKEY_LOCAL_MACHINE. see the SDK documentation for more information.
	 */
	CRegBase(const CString& key, bool force, HKEY base = HKEY_CURRENT_USER);

	/**
	 * Returns the string of the last error occurred.
	 */
	CString getErrorString()
	{
        CString error = CRegBaseCommon<CString>::getErrorString();
#if defined IDS_REG_ERROR
		CString sTemp;
		sTemp.Format(IDS_REG_ERROR, (LPCTSTR)m_key, (LPCTSTR)error);
		return sTemp;
#else
		return error;
#endif
	};
};
#endif

typedef std::wstring wide_string;
#ifndef stdstring
#	ifdef UNICODE
#		define stdstring wide_string
#	else
#		define stdstring std::string
#	endif
#endif

/**
 * \ingroup Utils
 * Base class for STL string type registry classes.
 */

class CRegStdBase : public CRegBaseCommon<stdstring>
{
protected:

    /**
     * String type specific operations.
     */

    virtual LPCTSTR GetPlainString (const stdstring& s) const {return s.c_str();}
    virtual DWORD GetLength (const stdstring& s) const {return static_cast<DWORD>(s.size());}

public:	//methods

    /** Default constructor.
     */
    CRegStdBase();
	/**
	 * Constructor.
	 * \param key the path to the key, including the key. example: "Software\\Company\\SubKey\\MyValue"
	 * \param force set to TRUE if no cache should be used, i.e. always read and write directly from/to registry
	 * \param base a predefined base key like HKEY_LOCAL_MACHINE. see the SDK documentation for more information.
	 */
	CRegStdBase(const stdstring& key, bool force, HKEY base = HKEY_CURRENT_USER);
};

/**
 * \ingroup Utils
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
 * an error occurred during read from the registry, a default
 * value of 100 is used when accessing the variable.
 * now the variable can be used like any other DWORD variable:
 * \code
 * regvalue = 200;						//stores the value 200 in the registry
 * int temp = regvalue + 300;			//temp has value 500 now
 * regvalue += 300;						//now the registry has the value 500 too
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
template<class T, class Base>
class CRegTypedBase : public Base
{
private:

	T m_value;					///< the cached value of the registry
	T m_defaultvalue;			///< the default value to use

    /**
     * sub-classes must provide type-specific code to extract data from
     * and write data to an open registry key.
     */

    virtual void InternalRead (HKEY hKey, T& value) = 0;
    virtual void InternalWrite (HKEY hKey, const T& value) = 0;

public:

    /**
     * We use this instead of a default constructor because not all 
     * data types may provide an adequate default constructor.
     */
	CRegTypedBase(const T& def);

	/**
	 * Constructor.
	 * \param key the path to the key, including the key. example: "Software\\Company\\SubKey\\MyValue"
	 * \param def the default value used when the key does not exist or a read error occurred
	 * \param force set to TRUE if no cache should be used, i.e. always read and write directly from/to registry
	 * \param base a predefined base key like HKEY_LOCAL_MACHINE. see the SDK documentation for more information.
	 */
    CRegTypedBase(const typename Base::StringT& key, const T& def, bool force = FALSE, HKEY base = HKEY_CURRENT_USER);

	/**
	 * reads the assigned value from the registry. Use this method only if you think the registry
	 * value could have been altered without using the CRegDWORD object.
	 * \return the read value
	 */
	void   	read();						///< reads the value from the registry
	void	write();					///< writes the value to the registry

    bool    exists();                   ///< test whether registry entry exits
    const T& defaultValue() const;      ///< return the default passed to the constructor

    /**
     * Data access.
     */

    operator const T&();
	CRegTypedBase<T,Base>& operator=(const T& rhs);
};

// implement CRegTypedBase<> members

template<class T, class Base>
CRegTypedBase<T, Base>::CRegTypedBase (const T& def)
    : m_value (def)
    , m_defaultvalue (def)
{
}

template<class T, class Base>
CRegTypedBase<T, Base>::CRegTypedBase (const typename Base::StringT& key, const T& def, bool force, HKEY base)
    : Base (key, force, base)
    , m_value (def)
    , m_defaultvalue (def)
{
}

template<class T, class Base>
void CRegTypedBase<T, Base>::read()
{
	m_value = m_defaultvalue;
    m_exists = false;

    HKEY hKey = NULL;
	if ((LastError = RegOpenKeyEx (m_base, GetPlainString (m_path), 0, KEY_EXECUTE, &hKey))==ERROR_SUCCESS)
	{
		m_read = true;

        T value = m_defaultvalue;
        InternalRead (hKey, value);

		if (LastError ==ERROR_SUCCESS)
		{
            m_exists = true;
			m_value = value;
		}

        LastError = RegCloseKey(hKey);
	}
}

template<class T, class Base>
void CRegTypedBase<T, Base>::write()
{
    HKEY hKey = NULL;

    DWORD disp = 0;
	if ((LastError = RegCreateKeyEx(m_base, GetPlainString (m_path), 0, _T(""), REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &disp))!=ERROR_SUCCESS)
	{
		return;
	}

    InternalWrite (hKey, m_value);
	if (LastError ==ERROR_SUCCESS)
	{
		m_read = true;
        m_exists = true;
	}
	LastError = RegCloseKey(hKey);
}

template<class T, class Base>
bool CRegTypedBase<T, Base>::exists()
{
    if (!m_read && (LastError == ERROR_SUCCESS))
        read();

    return m_exists;
}

template<class T, class Base>
const T& CRegTypedBase<T, Base>::defaultValue() const
{
    return m_defaultvalue;
}

template<class T, class Base>
CRegTypedBase<T, Base>::operator const T&()
{
	if ((m_read)&&(!m_force))
	{
		LastError = ERROR_SUCCESS;
	}
	else
	{
		read();
	}

	return m_value;
}

template<class T, class Base>
CRegTypedBase<T, Base>& CRegTypedBase<T, Base>::operator =(const T& d)
{
	if ((d==m_value)&&(!m_force))
	{
		//no write to the registry required, its the same value
		LastError = ERROR_SUCCESS;
		return *this;
	}
	m_value = d;
	write();
	return *this;
}

/**
 * \ingroup Utils
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
 * an error occurred during read from the registry, a default
 * value of 100 is used when accessing the variable.
 * now the variable can be used like any other DWORD variable:
 * \code
 * regvalue = 200;						//stores the value 200 in the registry
 * int temp = regvalue + 300;			//temp has value 500 now
 * regvalue += 300;						//now the registry has the value 500 too
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
template<class Base>
class CRegDWORDCommon : public CRegTypedBase<DWORD,Base>
{
private:

    /**
     * provide type-specific code to extract data from and write data to an open registry key.
     */

    virtual void InternalRead (HKEY hKey, DWORD& value);
    virtual void InternalWrite (HKEY hKey, const DWORD& value);

public:

	CRegDWORDCommon(void);
	/**
	 * Constructor.
	 * \param key the path to the key, including the key. example: "Software\\Company\\SubKey\\MyValue"
	 * \param def the default value used when the key does not exist or a read error occurred
	 * \param force set to TRUE if no cache should be used, i.e. always read and write directly from/to registry
	 * \param base a predefined base key like HKEY_LOCAL_MACHINE. see the SDK documentation for more information.
	 */
    CRegDWORDCommon(const typename Base::StringT& key, DWORD def = 0, bool force = false, HKEY base = HKEY_CURRENT_USER);

    CRegDWORDCommon& operator=(DWORD rhs) {CRegTypedBase<DWORD, Base>::operator =(rhs); return *this;}
	CRegDWORDCommon& operator+=(DWORD d) { return *this = *this + d;}
	CRegDWORDCommon& operator-=(DWORD d) { return *this = *this - d;}
	CRegDWORDCommon& operator*=(DWORD d) { return *this = *this * d;}
	CRegDWORDCommon& operator/=(DWORD d) { return *this = *this / d;}
	CRegDWORDCommon& operator%=(DWORD d) { return *this = *this % d;}
	CRegDWORDCommon& operator<<=(DWORD d) { return *this = *this << d;}
	CRegDWORDCommon& operator>>=(DWORD d) { return *this = *this >> d;}
	CRegDWORDCommon& operator&=(DWORD d) { return *this = *this & d;}
	CRegDWORDCommon& operator|=(DWORD d) { return *this = *this | d;}
	CRegDWORDCommon& operator^=(DWORD d) { return *this = *this ^ d;}
};

// implement CRegDWORDCommon<> methods

template<class Base>
CRegDWORDCommon<Base>::CRegDWORDCommon(void)
    : CRegTypedBase<DWORD, Base>(0)
{
}

template<class Base>
CRegDWORDCommon<Base>::CRegDWORDCommon(const typename Base::StringT& key, DWORD def, bool force, HKEY base)
    : CRegTypedBase<DWORD, Base> (key, def, force, base)
{
}

template<class Base>
void CRegDWORDCommon<Base>::InternalRead (HKEY hKey, DWORD& value)
{
    DWORD size = sizeof(value);
	DWORD type = 0;
	if ((LastError = RegQueryValueEx(hKey, GetPlainString (m_key), NULL, &type, (BYTE*) &value, &size))==ERROR_SUCCESS)
    {
		ASSERT(type==REG_DWORD);
    }
}

template<class Base>
void CRegDWORDCommon<Base>::InternalWrite (HKEY hKey, const DWORD& value)
{
	LastError = RegSetValueEx (hKey, GetPlainString (m_key), 0, REG_DWORD,(const BYTE*) &value, sizeof(value));
}

/**
 * \ingroup Utils
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
 * an error occurred during read from the registry, a default
 * value of "default" is used when accessing the variable.
 * now the variable can be used like any other CString variable:
 * \code
 * regvalue = "some string";			//stores the value "some string" in the registry
 * CString temp = regvalue + "!!";		//temp has value "some string!!" now
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
template<class Base>
class CRegStringCommon : public CRegTypedBase<typename Base::StringT, Base>
{
private:

    /**
     * provide type-specific code to extract data from and write data to an open registry key.
     */

    virtual void InternalRead (HKEY hKey, typename Base::StringT& value);
    virtual void InternalWrite (HKEY hKey, const typename Base::StringT& value);

public:
	CRegStringCommon();
	/**
	 * Constructor.
	 * \param key the path to the key, including the key. example: "Software\\Company\\SubKey\\MyValue"
	 * \param def the default value used when the key does not exist or a read error occurred
	 * \param force set to TRUE if no cache should be used, i.e. always read and write directly from/to registry
	 * \param base a predefined base key like HKEY_LOCAL_MACHINE. see the SDK documentation for more information.
	 */
    CRegStringCommon(const typename Base::StringT& key, const typename Base::StringT& def = _T(""), bool force = false, HKEY base = HKEY_CURRENT_USER);
	
    CRegStringCommon& operator=(const typename Base::StringT& rhs) {CRegTypedBase<StringT, Base>::operator =(rhs); return *this;}
	CRegStringCommon& operator+=(const typename Base::StringT& s) { return *this = (typename Base::StringT)*this + s; }
};

// implement CRegDWORD<> methods

template<class Base>
CRegStringCommon<Base>::CRegStringCommon(void)
    : CRegTypedBase<typename Base::StringT, Base>(typename Base::StringT())
{
}

template<class Base>
CRegStringCommon<Base>::CRegStringCommon(const typename Base::StringT& key, const typename Base::StringT& def, bool force, HKEY base)
    : CRegTypedBase<typename Base::StringT, Base> (key, def, force, base)
{
}

template<class Base>
void CRegStringCommon<Base>::InternalRead (HKEY hKey, typename Base::StringT& value)
{
	DWORD size = 0;
	DWORD type = 0;
	LastError = RegQueryValueEx(hKey, GetPlainString (m_key), NULL, &type, NULL, &size);

    std::auto_ptr<TCHAR> pStr (new TCHAR[size]);
	if ((LastError = RegQueryValueEx(hKey, GetPlainString (m_key), NULL, &type, (BYTE*) pStr.get(), &size))==ERROR_SUCCESS)
    {
        ASSERT(type==REG_SZ || type==REG_EXPAND_SZ);
        value = StringT (pStr.get());
    }
}

template<class Base>
void CRegStringCommon<Base>::InternalWrite (HKEY hKey, const typename Base::StringT& value)
{
	LastError = RegSetValueEx(hKey, GetPlainString (m_key), 0, REG_SZ, (BYTE *)GetPlainString (value), (GetLength(value)+1)*sizeof (TCHAR));
}

/**
 * \ingroup Utils
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
 * an error occurred during read from the registry, a default
 * value of 100,100,200,200 is used when accessing the variable.
 * now the variable can be used like any other CRect variable:
 * \code
 * regvalue = CRect(40,20,300,500);				//stores the value in the registry
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

#ifdef __ATLTYPES_H__   // defines CRect 
class CRegRect : public CRegTypedBase<CRect, CRegBase>
{
private:

    /**
     * provide type-specific code to extract data from and write data to an open registry key.
     */

    virtual void InternalRead (HKEY hKey, CRect& value);
    virtual void InternalWrite (HKEY hKey, const CRect& value);

public:
	CRegRect();
	/**
	 * Constructor.
	 * \param key the path to the key, including the key. example: "Software\\Company\\SubKey\\MyValue"
	 * \param def the default value used when the key does not exist or a read error occurred
	 * \param force set to TRUE if no cache should be used, i.e. always read and write directly from/to registry
	 * \param base a predefined base key like HKEY_LOCAL_MACHINE. see the SDK documentation for more information.
	 */
	CRegRect(const CString& key, const CRect& def = CRect(), bool force = false, HKEY base = HKEY_CURRENT_USER);
	~CRegRect(void);
	
    CRegRect& operator=(const CRect& rhs) {CRegTypedBase<CRect, CRegBase>::operator =(rhs); return *this;}
	operator LPCRECT() { return (LPCRECT)(CRect)*this; }
	operator LPRECT() { return (LPRECT)(CRect)*this; }
	CRegRect& operator+=(POINT r) { return *this = (CRect)*this + r;}
	CRegRect& operator+=(SIZE r) { return *this = (CRect)*this + r;}
	CRegRect& operator+=(LPCRECT  r) { return *this = (CRect)*this + r;}
	CRegRect& operator-=(POINT r) { return *this = (CRect)*this - r;}
	CRegRect& operator-=(SIZE r) { return *this = (CRect)*this - r;}
	CRegRect& operator-=(LPCRECT  r) { return *this = (CRect)*this - r;}
	
	CRegRect& operator&=(CRect r) { return *this = r & *this;}
	CRegRect& operator|=(CRect r) { return *this = r | *this;}
};
#endif

/**
 * \ingroup Utils
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
 * an error occurred during read from the registry, a default
 * value of 100,100 is used when accessing the variable.
 * now the variable can be used like any other CPoint variable:
 * \code
 * regvalue = CPoint(40,20);					//stores the value in the registry
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

#ifdef __ATLTYPES_H__   // defines CPoint 
class CRegPoint : public CRegTypedBase<CPoint, CRegBase>
{
private:

    /**
     * provide type-specific code to extract data from and write data to an open registry key.
     */

    virtual void InternalRead (HKEY hKey, CPoint& value);
    virtual void InternalWrite (HKEY hKey, const CPoint& value);

public:
	CRegPoint();
	/**
	 * Constructor.
	 * \param key the path to the key, including the key. example: "Software\\Company\\SubKey\\MyValue"
	 * \param def the default value used when the key does not exist or a read error occurred
	 * \param force set to TRUE if no cache should be used, i.e. always read and write directly from/to registry
	 * \param base a predefined base key like HKEY_LOCAL_MACHINE. see the SDK documentation for more information.
	 */
	CRegPoint(const CString& key, const CPoint& def = CPoint(), bool force = false, HKEY base = HKEY_CURRENT_USER);
	~CRegPoint(void);
	
    CRegPoint& operator=(const CPoint& rhs) {CRegTypedBase<CPoint, CRegBase>::operator =(rhs); return *this;}
	CRegPoint& operator+=(CPoint p) { return *this = p + *this; }
	CRegPoint& operator-=(CPoint p) { return *this = p - *this; }
};
#endif

/**
 * \ingroup Utils
 * Manages a registry key (not a value). Provides methods to create and remove the
 * key and to query the list of values and sub keys.
 */

#ifdef __AFXCOLL_H__   // defines CStringList 
class CRegistryKey
{
public:	//methods
	/**
	 * Constructor.
	 * \param key the path to the key, including the key. example: "Software\\Company\\SubKey"
	 * \param base a predefined base key like HKEY_LOCAL_MACHINE. see the SDK documentation for more information.
	 */
	CRegistryKey(const CString& key, HKEY base = HKEY_CURRENT_USER);
	~CRegistryKey();

	/**
	 * Creates the registry key if it does not already exist.
	 * \return ERROR_SUCCESS or an nonzero error code. Use FormatMessage() to get an error description.
	 */
	DWORD createKey();
	/**
	 * Removes the whole registry key including all values. So if you set the registry
	 * entry to be HKCU\Software\Company\Product\key there will only be
	 * HKCU\Software\Company\Product key in the registry.
	 * \return ERROR_SUCCESS or an nonzero error code. Use FormatMessage() to get an error description.
	 */
	DWORD removeKey();

	bool getValues(CStringList& values);		///< returns the list of values
	bool getSubKeys(CStringList& subkeys);		///< returns the list of sub keys

public:	//members
	HKEY m_base;		///< handle to the registry base
	HKEY m_hKey;		///< handle to the open registry key
	CString m_path;		///< the path to the key
};
#endif

/**
 * Instantiate templates for common (data type, string type) combinations.
 */

#ifdef __CSTRINGT_H__
CRegDWORDCommon<CRegBase>;
typedef CRegDWORDCommon<CRegBase> CRegDWORD;
CRegStringCommon<CRegBase>;
typedef CRegStringCommon<CRegBase> CRegString;
#endif

CRegDWORDCommon<CRegStdBase>;
typedef CRegDWORDCommon<CRegStdBase> CRegStdWORD;
CRegStringCommon<CRegStdBase>;
typedef CRegStringCommon<CRegStdBase> CRegStdString;


