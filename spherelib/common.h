#pragma once

#define IMULDIV(a,b,c) (((a)*(b))/(c))	// windows MulDiv will round ! 

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