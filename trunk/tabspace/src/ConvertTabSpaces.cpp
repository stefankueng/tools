#include "StdAfx.h"
#include "ConvertTabSpaces.h"

ConvertTabSpaces::ConvertTabSpaces(void)
{
}

ConvertTabSpaces::~ConvertTabSpaces(void)
{
}

bool ConvertTabSpaces::Convert(CTextFile& file, bool useSpaces, int tabsize, bool checkonly)
{
	if (!checkonly)
	{
		// if we find a violation of the rule, we fix the file
		if (!useSpaces)
		{
			// tabify the file
			// first find out how many spaces we have to convert into tabs
			int count = 0;
			int spacecount = 0;
			vector<long> spacegrouppositions;
			long pos = 0;
			for (wstring::const_iterator it = file.GetFileString().begin(); it != file.GetFileString().end(); ++it, ++pos)
			{
				// we have to convert all spaces in groups of more than the tabsize
				// a space followed by a tab may lead to just removing the space
				if ((*it == ' ')||(*it == '\t'))
				{
					spacecount++;
					if ((spacecount == tabsize)||((*it == '\t')&&(spacecount > 1)))
					{
						spacegrouppositions.push_back(pos-spacecount+1);
						count += (spacecount-1);
						spacecount = 0;
					}
					if (*it == '\t')
						spacecount = 0;
				}
				else
					spacecount = 0;
			}
			// now we have the number of space groups we have to convert to tabs
			// create a new file buffer and copy everything over there, replacing those space
			// groups with tabs.
			if (count)
			{
				if (file.GetEncoding() == CTextFile::UNICODE_LE)
				{
					long newfilelen = file.GetFileLength();
					newfilelen -= (count*sizeof(WCHAR));
					WCHAR * pBuf = new WCHAR[newfilelen/sizeof(WCHAR)];
					WCHAR * pBufStart = pBuf;
					WCHAR * pOldBuf = (WCHAR*)file.GetFileContent();
					vector<long>::iterator it = spacegrouppositions.begin();
					for (long i=0; i<long(file.GetFileLength()/sizeof(WCHAR)); ++i)
					{
						if ((it != spacegrouppositions.end())&&(*it == i))
						{
							*pBuf++ = '\t';
							spacecount = 0;
							while ((spacecount<tabsize)&&(*pOldBuf == ' '))
							{
								i++;
								spacecount++;
								*pOldBuf++;
							}
							if ((spacecount<tabsize)&&(*pOldBuf == '\t'))
								pBuf--;
							--i;
							++it;
						}
						else
							*pBuf++ = *pOldBuf++;
					}
					file.ContentsModified(pBufStart, newfilelen);
					return true;
				}
				else if (file.GetEncoding() != CTextFile::BINARY)
				{
					long newfilelen = file.GetFileLength();
					newfilelen -= count;
					char * pBuf = new char[newfilelen];
					char * pBufStart = pBuf;
					char * pOldBuf = (char*)file.GetFileContent();
					vector<long>::iterator it = spacegrouppositions.begin();
					for (long i=0; i<(file.GetFileLength()); ++i)
					{
						if ((it != spacegrouppositions.end())&&(*it == i))
						{
							*pBuf++ = '\t';
							spacecount = 0;
							while ((spacecount<tabsize)&&(*pOldBuf == ' '))
							{
								i++;
								spacecount++;
								*pOldBuf++;
							}
							if ((spacecount<tabsize)&&(*pOldBuf == '\t'))
								pBuf--;
							--i;
							++it;
						}
						else
							*pBuf++ = *pOldBuf++;
					}
					file.ContentsModified(pBufStart, newfilelen);
					return true;
				}
			}
		}
		else
		{
			// untabify the file
			// first find the number of spaces we have to insert.
			long pos = 0;
			long inlinepos = 0;
			long spacestoinsert = 0;
			for (wstring::const_iterator it = file.GetFileString().begin(); it != file.GetFileString().end(); ++it, ++pos)
			{
				++inlinepos;
				if ((*it == '\r')||(*it == '\n'))
					inlinepos = 0;
				// we have to convert all tabs
				if (*it == '\t')
				{
					inlinepos += tabsize-1;
					long inlinepostemp = tabsize - ((inlinepos + tabsize)%tabsize);
					if (inlinepostemp == 0)
						inlinepostemp = tabsize;
					spacestoinsert += (inlinepostemp - 1);		// minus one because the tab itself gets replaced
					inlinepos += inlinepostemp;
				}
			}
			if (spacestoinsert)
			{
				inlinepos = 0;
				if (file.GetEncoding() == CTextFile::UNICODE_LE)
				{
					long newfilelen = file.GetFileLength() + (spacestoinsert*sizeof(WCHAR));
					WCHAR * pBuf = new WCHAR[newfilelen/sizeof(WCHAR)];
					WCHAR * pBufStart = pBuf;
					WCHAR * pOldBuf = (WCHAR*)file.GetFileContent();
					for (long i=0; i<long(file.GetFileLength()/sizeof(WCHAR)); ++i)
					{
						++inlinepos;
						if ((*pOldBuf == '\r')||(*pOldBuf == '\n'))
							inlinepos = 0;
						if (*pOldBuf == '\t')
						{
							long inlinepostemp = tabsize-(((inlinepos-1)+tabsize)%tabsize);
							if (inlinepostemp == 0)
								inlinepostemp = tabsize;
							inlinepos += (inlinepostemp-1);
							for (int j=0; j<inlinepostemp; ++j)
								*pBuf++ = ' ';
							pOldBuf++;
						}
						else
							*pBuf++ = *pOldBuf++;
					}
					file.ContentsModified(pBufStart, newfilelen);
					return true;
				}
				else if (file.GetEncoding() != CTextFile::BINARY)
				{
					long newfilelen = file.GetFileLength() + spacestoinsert;
					char * pBuf = new char[newfilelen];
					char * pBufStart = pBuf;
					char * pOldBuf = (char*)file.GetFileContent();
					for (long i=0; i<file.GetFileLength(); ++i)
					{
						++inlinepos;
						if ((*pOldBuf == '\r')||(*pOldBuf == '\n'))
							inlinepos = 0;
						if (*pOldBuf == '\t')
						{
							long inlinepostemp = tabsize-(((inlinepos-1)+tabsize)%tabsize);
							if (inlinepostemp == 0)
								inlinepostemp = tabsize;
							inlinepos += (inlinepostemp-1);
							for (int j=0; j<inlinepostemp; ++j)
							{
								*pBuf++ = ' ';
							}
							pOldBuf++;
						}
						else
							*pBuf++ = *pOldBuf++;
					}
					file.ContentsModified(pBufStart, newfilelen);
					return true;
				}
			}
		}
	}
	else
	{
		// don't touch the file, only print messages about what we find
		if (!useSpaces)
		{
			// if we're in tab mode, then more or equal spaces than the tabsize in a row are a violation
			// less spaces than the tabsize could be used to align text to a non-tab position and therefore
			// is not a violation.
			size_t pos = 0;
			for (wstring::const_iterator it = file.GetFileString().begin(); it != file.GetFileString().end(); ++it, ++pos)
			{
				if (*it == ' ')
				{
					++it;
					++pos;
					int count = 1;
					while ((it != file.GetFileString().end())&&(*it == ' '))
					{
						++count;
						++it;
						++pos;
					}
					if (count >= tabsize)
					{
						// we have more or equal spaces in a row than the tabsize is, that's a violation!
						TCHAR buf[MAX_PATH*2];
						_stprintf_s(buf, MAX_PATH*2, _T("found spaces instead of tabs in file'%s', line %d\n"), file.GetFileName().c_str(), file.LineFromPosition(pos));
						_fputts(buf, stderr);
					}
				}
			}
		}
		else
		{
			// in space mode, even one tab is a violation
			size_t pos = 0;
			for (wstring::const_iterator it = file.GetFileString().begin(); it != file.GetFileString().end(); ++it, ++pos)
			{
				if (*it == '\t')
				{
					// we have a tab, that's a violation!
					TCHAR buf[MAX_PATH*2];
					_stprintf_s(buf, MAX_PATH*2, _T("found tab instead of spaces in file'%s', line %d\n"), file.GetFileName().c_str(), file.LineFromPosition(pos));
					_fputts(buf, stderr);

					// now skip to the next non-space char
					while ((it != file.GetFileString().end())&&((*it == ' ')||(*it == '\t')))
					{
						++it;
						++pos;
					}
				}
			}
		}
	}
	return false;
}


bool ConvertTabSpaces::RemoveEndSpaces(CTextFile& file, bool checkonly)
{
	if (!checkonly)
	{
		// first count the whitespaces we have to remove
		int inlinepos = 0;
		int whitespaces = 0;
		int totalwhitespaces = 0;
		int pos = 0;
		vector<long> spacepositions;
		for (wstring::const_iterator it = file.GetFileString().begin(); it != file.GetFileString().end(); ++it)
		{
			++inlinepos;
			++pos;
			if ((*it == '\r')||(*it == '\n'))
			{
				if (whitespaces)
				{
					spacepositions.push_back(pos - whitespaces);
					totalwhitespaces += whitespaces;
				}
				whitespaces = 0;
				inlinepos = 0;
			}
			if ((*it == ' ')||(*it == '\t'))
				whitespaces++;
			else
				whitespaces = 0;
		}
		// now we have the amount of whitespaces we have to remove
		if (totalwhitespaces)
		{
			pos = 0;
			if (file.GetEncoding() == CTextFile::UNICODE_LE)
			{
				long newfilelen = file.GetFileLength();
				newfilelen -= (totalwhitespaces*sizeof(WCHAR));
				WCHAR * pBuf = new WCHAR[newfilelen/sizeof(WCHAR)];
				WCHAR * pBufStart = pBuf;
				WCHAR * pOldBuf = (WCHAR*)file.GetFileContent();
				vector<long>::iterator it = spacepositions.begin();
				for (long i=0; i<long(file.GetFileLength()/sizeof(WCHAR)); ++i)
				{
					++pos;
					if ((it != spacepositions.end())&&(pos == *it))
					{
						++it;
						TCHAR outbuf[MAX_PATH*2];
						_stprintf_s(outbuf, MAX_PATH*2, _T("fixed end-of-line whitespaces in file'%s', line %d\n"), file.GetFileName().c_str(), file.LineFromPosition(pos));
						_fputts(outbuf, stdout);
						// now skip the rest of the whitespaces
						while ((*pOldBuf == ' ')||(*pOldBuf == '\t'))
						{
							pOldBuf++;
							i++;
							pos++;
						}
					}
					*pBuf++ = *pOldBuf++;
				}
				file.ContentsModified(pBufStart, newfilelen);
				return true;
			}
			else
			{
				long newfilelen = file.GetFileLength();
				newfilelen -= totalwhitespaces;
				char * pBuf = new char[newfilelen];
				char * pBufStart = pBuf;
				char * pOldBuf = (char*)file.GetFileContent();
				vector<long>::iterator it = spacepositions.begin();
				for (long i=0; i<long(file.GetFileLength()); ++i)
				{
					++pos;
					if ((it != spacepositions.end())&&(pos == *it))
					{
						++it;
						TCHAR outbuf[MAX_PATH*2];
						_stprintf_s(outbuf, MAX_PATH*2, _T("fixed end-of-line whitespaces in file'%s', line %d\n"), file.GetFileName().c_str(), file.LineFromPosition(pos));
						_fputts(outbuf, stdout);
						// now skip the rest of the whitespaces
						while ((*pOldBuf == ' ')||(*pOldBuf == '\t'))
						{
							pOldBuf++;
							i++;
							pos++;
						}
					}
					*pBuf++ = *pOldBuf++;
				}
				file.ContentsModified(pBufStart, newfilelen);
				return true;
			}
		}
	}
	else
	{
		// only throw out messages when we find end-of-line whitespaces
		int inlinepos = 0;
		int whitespaces = 0;
		int pos = 0;
		for (wstring::const_iterator it = file.GetFileString().begin(); it != file.GetFileString().end(); ++it)
		{
			++inlinepos;
			++pos;
			if ((*it == '\r')||(*it == '\n'))
			{
				if (whitespaces)
				{
					TCHAR outbuf[MAX_PATH*2];
					_stprintf_s(outbuf, MAX_PATH*2, _T("end-of-line whitespaces found in file'%s', line %d\n"), file.GetFileName().c_str(), file.LineFromPosition(pos));
					_fputts(outbuf, stderr);
				}
				whitespaces = 0;
				inlinepos = 0;
			}
			if ((*it == ' ')||(*it == '\t'))
				whitespaces++;
			else
				whitespaces = 0;
		}
	}
	return false;
}