#pragma once



class CStringUtils
{
public:
	CStringUtils(void);
	~CStringUtils(void);


#ifdef UNICODE
#define _tcswildcmp	wcswildcmp
#else
#define _tcswildcmp strwildcmp
#endif

	/**
	 * Performs a wildcard compare of two strings.
	 * \param wild the wildcard string
	 * \param string the string to compare the wildcard to
	 * \return TRUE if the wildcard matches the string, 0 otherwise
	 * \par example
	 * \code 
	 * if (strwildcmp("bl?hblah.*", "bliblah.jpeg"))
	 *  printf("success\n");
	 * else
	 *  printf("not found\n");
	 * if (strwildcmp("bl?hblah.*", "blabblah.jpeg"))
	 *  printf("success\n");
	 * else
	 *  printf("not found\n");
	 * \endcode
	 * The output of the above code would be:
	 * \code
	 * success
	 * not found
	 * \endcode
	 */
	static int strwildcmp(const char * wild, const char * string);
	static int wcswildcmp(const wchar_t * wild, const wchar_t * string);
};
