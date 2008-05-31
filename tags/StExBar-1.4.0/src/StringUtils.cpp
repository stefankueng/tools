#include "StdAfx.h"
#include "StringUtils.h"

CStringUtils::CStringUtils(void)
{
}

CStringUtils::~CStringUtils(void)
{
}

int CStringUtils::strwildcmp(const char *wild, const char *string)
{
	const char *cp = NULL;
	const char *mp = NULL;
	while ((*string) && (*wild != '*')) 
	{
		if ((*wild != *string) && (*wild != '?')) 
		{
			return 0; 
		}
		wild++; 
		string++; 
	} // while ((*string) && (*wild != '*')) 
	while (*string) 
	{
		if (*wild == '*') 
		{
			if (!*++wild) 
			{
				return 1; 
			} 
			mp = wild; 
			cp = string+1;
		} // if (*wild == '*') 
		else if ((*wild == *string) || (*wild == '?')) 
		{
			wild++;
			string++;
		} 
		else 
		{
			wild = mp;
			string = cp++;
		}
	} // while (*string)

	while (*wild == '*') 
	{
		wild++;
	}
	return !*wild;
}

int CStringUtils::wcswildcmp(const wchar_t *wild, const wchar_t *string)
{
	const wchar_t *cp = NULL;
	const wchar_t *mp = NULL;
	while ((*string) && (*wild != '*')) 
	{
		if ((*wild != *string) && (*wild != '?')) 
		{
			return 0; 
		}
		wild++; 
		string++; 
	} // while ((*string) && (*wild != '*')) 
	while (*string) 
	{
		if (*wild == '*') 
		{
			if (!*++wild) 
			{
				return 1; 
			} 
			mp = wild; 
			cp = string+1;
		} // if (*wild == '*') 
		else if ((*wild == *string) || (*wild == '?')) 
		{
			wild++;
			string++;
		} 
		else 
		{
			wild = mp;
			string = cp++;
		}
	} // while (*string)

	while (*wild == '*') 
	{
		wild++;
	}
	return !*wild;
}
