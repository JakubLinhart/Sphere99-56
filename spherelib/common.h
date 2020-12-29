#pragma once

#define MIN min
#define MAX max
#define IMULDIV(a,b,c) (((a)*(b))/(c))	// windows MulDiv will round ! 

#ifndef _UNICODE		// _WIN32
#define TCHAR			char
#define LPCTSTR			LPCSTR
#define LPCWSTR			LPCSTR
#endif  // _UNICODE _WIN32
#ifndef _TEXT
#define _TEXT(x)		(TCHAR *)x
#endif	// _TEXT

// use to indicate that a function uses printf-style arguments, allowing GCC
// to validate the format string and arguments:
// a = 1-based index of format string
// b = 1-based index of arguments
// (note: add 1 to index for non-static class methods because 'this' argument
// is inserted in position 1)
#ifdef __GNUC__
#define __printfargs(a,b) __attribute__ ((format(printf, a, b)))
#else
#define __printfargs(a,b)
#endif

#ifndef COUNTOF
#define COUNTOF(a)	(sizeof(a)/sizeof((a)[0]))
#endif

#define UID_INDEX DWORD
#define HASH_INDEX DWORD
#define HASH_COMPARE(a, b) (a>b)

#ifndef _1BITMASK
#define _1BITMASK(b)    (((size_t)1) << (b))
#endif

#define ISWHITESPACE(ch)		 (isspace(ch)||(ch)==0xa0)	// isspace
#define GETNONWHITESPACE( pStr ) while ( ISWHITESPACE( (pStr)[0] )) { (pStr)++; }
