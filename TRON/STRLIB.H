/*
 *      IT-HE Generic Support Library, (C) 2000 IT-HE Software
 */

#ifndef __ITHE_SUPPORT_LIB__
#define __ITHE_SUPPORT_LIB__

#ifdef __BORLANDC__
#define __inline
#endif

#include <stdio.h>            // Just in case
#include <time.h>

// Enable full memory debugging with Fortify (very slow!)
//#define FORTIFY

// Defines that may need changing

#define MAX_LINE_LEN 256	// Maximum line length for string processing


#ifdef __cplusplus
extern "C" {
#endif

#ifndef NO_FORTIFY
//#include "fortify.h"          // Fortify is a memory-leak detector
#endif

// String manipulation library

extern char *strfirst(char *input);        // Get COPY of head of string
extern char *strrest(char *input);         // Get pointer to tail of string
extern char *strgetword(char *line, int p);// Get the word at this position
extern void strstrip(char *tbuf);          // Strip a string
extern int strcount(char *a);              // Count number of words

extern char *strLocalBuf(void);        		// Get a local buffer
extern void strdeck(char *a, char b);		// Kill all instances of one char
extern char *NOTHING;                      // blank, non-NULL string

// DOS functions (For Linux and BeOS, Windows already has them)
extern long filelength(int fhandle);

//#include <io.h>
#include <string.h>

#define SAFE_STRCPY(a,b) {strncpy((a),(b),sizeof((a)));a[sizeof((a))-1]=0;}


// End of library functions

#ifdef __cplusplus
}
#endif

#endif // __ITHE_SUPPORT_LIB__
