//
//      String manipulation functions
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#ifndef __BORLANDC__
#include <strings.h>
#endif
#endif
#include <ctype.h>

#include "strlib.h"

#define MAXBUFS 4 // May need increasing later

static char localbufarray[MAXBUFS][MAX_LINE_LEN];

char *NOTHING="\0";

// Declare inline function
__inline char iisxspace(unsigned char input);


/*
 *      isXspace - is the character under scrutiny whitespace or not?
 *                 Two versions: export and inline
 */

char isxspace(unsigned char input)
{
if(!input)
	return 0;
return input<=32?1:0;
}

__inline char iisxspace(unsigned char input)
{
if(!input)
	return 0;
return input<=32?1:0;
}


/*
 *      first - A LISP string processing routine.
 *              first returns only the first word of a string.
 *              e.g. first("1 2 3") returns "1"
 *
 *              first returns a copy of the string, it does not modify the
 *              original string.  Use hardfirst() if you want this to happen.
 */

char *strfirst(char *input)
{
char *tbuf;
char *p,*q,ok;

tbuf = strLocalBuf();

// Simple cases first

if(input==NULL)
	return NOTHING;
if(input==NOTHING)
	return NOTHING;
if(input[0]=='\0' || input[0]=='\n')
	return NOTHING;
if(strlen(input) == 0)
	return NOTHING;

// Then the rest

strcpy(tbuf,input);
p=strchr(tbuf,' ');
if(p)
	*p=0;
p=strchr(tbuf,'\t');
if(p)
	*p=0;

p=tbuf;

// Now we need to know if there's anything useful there

if(*p == 0)
	return NOTHING;  // No there isn't

// Examine each character

ok=0;
for(q=p;*q;q++)
	if(!iisxspace(*q))
		ok=1;

if(!ok)
	return NOTHING;  // Only whitespace

if(*p == 0)
	return NOTHING;  // Nothing there

if(strlen(p) == 0)
	return NOTHING;

return p;
}


/*
 *      rest - A LISP string processing routine.
 *             rest returns all but the first word of a string.
 *             e.g. rest("1 2 3") returns "2 3"
 */

char *strrest(char *input)
{
short ctr,len,ptr,ptr2,ptr3;

if(input==NULL)
	return NOTHING;

len=strlen(input);
ptr=-1;
for(ctr=0;ctr<len;ctr++)
	if(!iisxspace(input[ctr]))
		{
		ptr=ctr;
		break;
		}
if(ptr==-1)
	return NOTHING;

ptr2=-1;
for(ctr=ptr;ctr<len;ctr++)
	if(iisxspace(input[ctr]))
		{
		ptr2=ctr;
		break;
		}

if(ptr2==-1)
	return NOTHING;

ptr3=-1;
for(ctr=ptr2;ctr<len;ctr++)
	if(!iisxspace(input[ctr]))
		{
		ptr3=ctr;
		break;
		}
if(ptr3==-1)
	return NOTHING;

return &input[ptr3];
}


/*
 *      count - Return the number of words in the sentence
 */

int strcount(char *a)
{
int rc;
char *p,*sp;

// First look for an empty string (0 words)

if(!a[0])
	return 0;

for(p=a;iisxspace(*p);p++);
if(!*p)
	return 0;

// Now do the rest of the job

sp=a;
rc = 0;
do	{
	p = strrest(sp);
	sp=p;
	rc++;
	} while(p != NOTHING);
return rc;
}

/*
int strcount(char *a)
{
int rc;
char tvb[MAX_LINE_LEN];
char tvb2[MAX_LINE_LEN];
char *p;

// First look for an empty string (0 words)

if(!a[0])
	return 0;

for(p=a;iisxspace(*p);p++);
if(!*p)
	return 0;

// Now do the rest of the job

strcpy(tvb,a);
rc = 0;
do	{
	p = strrest(tvb);
	strcpy(tvb2,p);
	strcpy(tvb,tvb2);
	rc++;
	} while(p != NOTHING);
return rc;
}
*/

/*
 *  word - Return the word at the specified position
 *             e.g. word("a b c",2) = 'b'
 */

char *strgetword(char *a, int pos)
{
char tvb[MAX_LINE_LEN];
char *p;
int rc=1;

// Hack for 1

if(pos == 1)
	return(strfirst(a));

// This for the rest..

strcpy(tvb,a);
for(p=&tvb[0];*p;p++)
	if(iisxspace(*p))
		{
		*p=0;
		if(!iisxspace(*(p+1))) // Must be just 1 space to count
			rc++;
		if(rc == pos)
			return strfirst(++p);
		}
return NOTHING;
}

/*
 *      Strip - Shorten a string to remove any trailing spaces
 */

void strstrip(char *tbuf)
{
char *tptr;

// Sanity checks first

if(tbuf == NOTHING || tbuf == NULL)
	return;                     // There is nothing there!  abort

// Check for too short string

if(strlen(tbuf) <= 1)
	return;                     // There is nothing there!  abort

// Check for just a lot of spaces

if(strcount(tbuf) == 0)      // how many words?
	return;                     // There is nothing there!  abort

// Find first non-whitespace character
for(tptr = tbuf;iisxspace(*tptr);tptr++);

// Make sure there was something

if(!*tptr)
	return;

// Tptr is now the beginning of the 'pure' string
strcpy(tbuf,tptr);

// Now we need to find the whitespace character at the end of the string,
// if there is one

// Wind to end

for(tptr = tbuf;*tptr;tptr++);

// Find first non-whitespace character

--tptr;

// Ok, we're at the end of the string.  If there's no trailing space, we're
// fine and don't need do do any more work

if(!iisxspace(*tptr))
	return;

// Find the first character that's not a space

for(;iisxspace(*tptr);tptr--);

// Hit it

*(++tptr)=0;
}



/*
 * We can't return the address of a local buffer because it will change, so
 * we use this to provide scratch space instead.  The buffer it returns is
 * one of many so we don't need to worry about recursive calls overwriting
 * each other's scratch space
 */

char *strLocalBuf()
{
static int bufctr=0;
char *b;

bufctr++;
if(bufctr>=MAXBUFS)
	bufctr=0;
b=&localbufarray[bufctr][0];

*b=0; // Blank it for the function
return b;
}


/*
 *   strdeck(string,char) - Find all occurences of a character and "deck 'em"
 *                          i.e. turn them into spaces
 */

void strdeck(char *line,char c)
{
char *p;
do
	{
	p=strchr(line,c);
	if(p)
		*p=' ';
	} while(p);
}

