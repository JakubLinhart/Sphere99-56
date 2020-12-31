/**
* @file CString.
* @brief Custom String implementation.
*/

#pragma once
#ifndef _INC_CSTRING_H
#define _INC_CSTRING_H

#include "common.h"

#define CSTRING_MAX_LEN 8*1024

class CString
{
public:
	CString(LPCTSTR pStr);
	operator LPCTSTR() const
	{
		throw "Not implemented";
	}
};

/**
* @brief Custom String implementation.
*/
class CGString
{
private:
	TCHAR* m_pchData; ///< Data pointer.
	int		m_iLength; ///< Length of string.
	int		m_iMaxLength; ///< Size of memory allocated pointed by m_pchData.

public:
	static const char* m_sClassName;

private:
	/**
	* @brief Initializes internal data.
	*
	* Allocs STRING_DEFAULT_SIZE by default. If DEBUG_STRINGS setted, updates statistical information (total memory allocated).
	*/
	void Init();

public:
	/**
	* @brief CGString destructor.
	*
	* If DEBUG_STRINGS setted, updates statistical information (total CGString instantiated).
	*/
	~CGString();
	/**
	* @brief Default constructor.
	*
	* Initializes string. If DEBUG_STRINGS setted, update statistical information (total CGString instantiated).
	* @see Init()
	*/
	CGString();
	/**
	* @brief Copy constructor.
	*
	* @see Copy()
	* @param pStr string to copy.
	*/
	CGString(LPCTSTR pStr);
	/**
	* @brief Copy constructor.
	*
	* @see Copy()
	* @param pStr string to copy.
	*/
	CGString(const CGString& s);

	/**
	* @brief Check if there is data allocated and if the string is zero ended.
	* @return true if is valid, false otherwise.
	*/
	bool IsValid() const;
	/**
	* @brief Change the length of the CGString.
	*
	* If the new length is lesser than the current lenght, only set a zero at the end of the string.
	* If the new length is bigger than the current length, alloc memory for the string and copy.
	* If DEBUG_STRINGS setted, update statistical information (reallocs count, total memory allocated).
	* @param iLen new length of the string.
	* @return the new length of the CGString.
	*/
	int SetLength(int iLen);
	/**
	* @brief Get the length of the CGString.
	* @return the length of the CGString.
	*/
	int GetLength() const;
	/**
	* @brief Check the length of the CGString.
	* @return true if length is 0, false otherwise.
	*/
	bool IsEmpty() const;
	/**
	* @brief Sets length to zero.
	*
	* If bTotal is true, then free the memory allocated. If DEBUG_STRINGS setted, update statistical information (total memory allocated).
	* @param bTotal true for free the allocated memory.
	*/
	void Empty(bool bTotal = false);
	/**
	* @brief Copy a string into the CGString.
	* @see SetLength()
	* @see strcpylen()
	* @param pStr string to copy.
	*/
	void Copy(LPCTSTR pStr);

	/**
	* @brief Gets the reference to character a specified position (0 based).
	* @param nIndex position of the character.
	* @return reference to character in position nIndex.
	*/
	TCHAR& ReferenceAt(int nIndex);
	/**
	* @brief Gets the caracter in a specified position (0 based).
	* @param nIndex position of the character.
	* @return character in position nIndex.
	*/
	TCHAR GetAt(int nIndex) const;
	/**
	* @brief Puts a character in a specified position (0 based).
	*
	* If character is 0, updates the length of the string (truncated).
	* @param nIndex position to put the character.
	* @param ch character to put.
	*/
	void SetAt(int nIndex, TCHAR ch);
	/**
	* @brief Gets the internal pointer.
	* @return Pointer to internal data.
	*/
	LPCTSTR GetPtr() const;

	/**
	* @brief Join a formated string (printf like) with values and copy into this.
	* @param pStr formated string.
	* @param args list of values.
	*/
	void FormatV(LPCTSTR pStr, va_list args);
	/**
	* @brief Join a formated string (printf like) with values and copy into this.
	* @see FormatV()
	* @param pStr formated string.
	* @param ... list of values.
	*/
	void _cdecl Format(LPCTSTR pStr, ...) __printfargs(2, 3);
	/**
	* @brief Print a long value into the string.
	* @see Format()
	* @param iVal value to print.
	*/
	void FormatVal(long iVal);
	/**
	* @brief Print a long long value into the string.
	* @see Format()
	* @param iVal value to print.
	*/
	void FormatLLVal(long long iVal);
	/**
	* @brief Print a unsigned long long value into the string.
	* @see Format()
	* @param iVal value to print.
	*/
	void FormatULLVal(unsigned long long iVal);
	/**
	* @brief Print a unsigned long value into the string.
	* @see Format()
	* @param iVal value to print.
	*/
	void FormatUVal(unsigned long iVal);
	/**
	* @brief Print a DWORD value into the string (hex format).
	* @see Format()
	* @param iVal value to print.
	*/
	void FormatHex(DWORD dwVal);
	/**
	* @brief Print a unsigned long long value into the string (hex format).
	* @see Format()
	* @param iVal value to print.
	*/
	void FormatLLHex(unsigned long long dwVal);

	/**
	* @brief Compares the CGString to string pStr (strcmp wrapper).
	*
	* This function starts comparing the first character of CGString and the string.
	* If they are equal to each other, it continues with the following
	* pairs until the characters differ or until a terminating null-character
	* is reached. This function performs a binary comparison of the characters.
	* @param pStr string to compare.
	* @return <0 if te first character that not match has lower value in CGString than in pStr. 0 if hte contents of both are equal. >0 if the first character that does not match has greater value in CGString than pStr.
	*/
	int Compare(LPCTSTR pStr) const;
	/**
	* @brief Compares the CGString to string pStr (case insensitive) (_strcmpi wrapper).
	*
	* This function starts comparing the first character of CGString and the string.
	* If they are equal to each other, it continues with the following
	* pairs until the characters differ or until a terminating null-character
	* is reached. This function performs a case insensitive comparison of the characters.
	* @param pStr string to compare.
	* @return <0 if te first character that not match has lower value in CGString than in pStr. 0 if hte contents of both are equal. >0 if the first character that does not match has greater value in CGString than pStr.
	*/
	int CompareNoCase(LPCTSTR pStr) const;
	/**
	* @brief Look for the first occurence of c in CGString.
	* @param c character to look for.
	* @return position of the character in CGString if any, -1 otherwise.
	*/
	int indexOf(TCHAR c);
	/**
	* @brief Look for the first occurence of c in CGString from a position.
	* @param c character to look for.
	* @param offset position from start the search.
	* @return position of the character in CGString if any, -1 otherwise.
	*/
	int indexOf(TCHAR c, int offset);
	/**
	* @brief Look for the first occurence of a substring in CGString from a position.
	* @param str substring to look for.
	* @param offset position from start the search.
	* @return position of the substring in CGString if any, -1 otherwise.
	*/
	int indexOf(CGString str, int offset);
	/**
	* @brief Look for the first occurence of a substring in CGString.
	* @param str substring to look for.
	* @return position of the substring in CGString if any, -1 otherwise.
	*/
	int indexOf(CGString str);
	/**
	* @brief Look for the last occurence of c in CGString.
	* @param c character to look for.
	* @return position of the character in CGString if any, -1 otherwise.
	*/
	int lastIndexOf(TCHAR c);
	/**
	* @brief Look for the last occurence of c in CGString from a position to the end.
	* @param c character to look for.
	* @param from position where stop the search.
	* @return position of the character in CGString if any, -1 otherwise.
	*/
	int lastIndexOf(TCHAR c, int from);
	/**
	* @brief Look for the last occurence of a substring in CGString from a position to the end.
	* @param str substring to look for.
	* @param from position where stop the search.
	* @return position of the substring in CGString if any, -1 otherwise.
	*/
	int lastIndexOf(CGString str, int from);
	/**
	* @brief Look for the last occurence of a substring in CGString.
	* @param str substring to look for.
	* @return position of the substring in CGString if any, -1 otherwise.
	*/
	int lastIndexOf(CGString str);
	/**
	* @brief Adds a char at the end of the CGString.
	* @param ch character to add.
	*/
	void Add(TCHAR ch);
	/**
	* @brief Adds a string at the end of the CGString.
	* @parampszStrh string to add.
	*/
	void Add(LPCTSTR pszStr);
	/**
	* @brief Reverses the CGString.
	*/
	void Reverse();
	/**
	* @brief Changes the capitalization of CGString to upper.
	*/
	void MakeUpper() { _strupr(m_pchData); }
	/**
	* @brief Changes the capitalization of CGString to lower.
	*/
	void MakeLower() { _strlwr(m_pchData); }

	/**
	* @brief Gets the caracter in a specified position (0 based).
	* @see GetAt()
	* @param nIndex position of the character.
	* @return character in position nIndex.
	*/
	TCHAR operator[](int nIndex) const
	{
		return GetAt(nIndex);
	}
	/**
	* @brief Gets the reference to character a specified position (0 based).
	* @see ReferenceAt()
	* @param nIndex position of the character.
	* @return reference to character in position nIndex.
	*/
	TCHAR& operator[](int nIndex)
	{
		return ReferenceAt(nIndex);
	}
	/**
	* @brief cast as const LPCSTR.
	* @return internal data pointer.
	*/
	operator LPCTSTR() const
	{
		return(GetPtr());
	}
	operator CString&() const
	{
		return CString(GetPtr());
	}
	/**
	* @brief Concatenate CGString with a string.
	* @param psz string to concatenate with.
	* @return The result of concatenate the CGString with psz.
	*/
	const CGString& operator+=(LPCTSTR psz)	// like strcat
	{
		Add(psz);
		return(*this);
	}
	/**
	* @brief Concatenate CGString with a character.
	* @param ch character to concatenate with.
	* @return The result of concatenate the CGString with ch.
	*/
	const CGString& operator+=(TCHAR ch)
	{
		Add(ch);
		return(*this);
	}
	/**
	* @brief Copy supplied string into the CGString.
	* @param pStr string to copy.
	* @return the CGString.
	*/
	const CGString& operator=(LPCTSTR pStr)
	{
		Copy(pStr);
		return(*this);
	}
	/**
	* @brief Copy supplied CGString into the CGString.
	* @param s CGString to copy.
	* @return the CGString.
	*/
	const CGString& operator=(const CGString& s)
	{
		Copy(s.GetPtr());
		return(*this);
	}
};

/**
* match result defines
*/
enum MATCH_TYPE
{
	MATCH_INVALID = 0,
	MATCH_VALID,		///< valid match
	MATCH_END,			///< premature end of pattern string
	MATCH_ABORT,		///< premature end of text string
	MATCH_RANGE,		///< match failure on [..] construct
	MATCH_LITERAL,		///< match failure on literal match
	MATCH_PATTERN		///< bad pattern
};

extern int strcpylen(TCHAR* pDst, LPCTSTR pSrc);
extern int strcpylen(TCHAR* pDst, LPCTSTR pSrc, int imaxlen);

// extern TCHAR * Str_GetTemporary(int amount = 1);
extern TCHAR* Str_GetTemp();

#endif _INC_CSTRING_H