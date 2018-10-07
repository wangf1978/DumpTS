/*
In the code under Windows development, there are lots of codes which are related with TCHAR, 
in order to let these parts of codes to be compiled successfully, introduce this header file
and redirect TCHAR to ANSI or UTF8 always.
*/
#ifndef _USR_TCHAR_H
#define _USR_TCHAR_H 1

#include <wchar.h>

typedef char TCHAR;
typedef char _TCHAR;
typedef unsigned char _TUCHAR;

#define _tprintf	printf

#define _totupper   toupper
#define _totlower   tolower
#define _tcsdup    _strdup
#define _tcslen    strlen
#define _tcscpy    strcpy
#define _tcsncpy   strncpy
#define _tcscat    strcat
#define _tcsncat   strncat
#define _tcsncicmp strnicmp
#define _tcsnicmp  strnicmp
#define _tcsicmp   stricmp
#define _tcscmp    strcmp
#define _tcsncmp   strncmp
#define _tcschr    strchr
#define _tcsrchr   strrchr
#define _stprintf  sprintf
#define _stscanf   sscanf
#define _tcsupr    _strupr
#define _tcslwr    _strlwr
#define _tclen     strlen
#define _ttoi      atoi
#define _istdigit  isdigit
#define _vstprintf vsprintf
#define _tcspbrk   strpbrk
#define _tcsstr    strstr
#define _tunlink   unlink
#define _tgetenv   getenv
#define _tputenv   putenv
#define _taccess   access
#define _topen     open
#ifdef __linux__
#define _tfopen	   fopen64
#else
#define _tfopen	   fopen
#endif

#define _tctime	   ctime

#define _tcscat_s(buffer, buffer_size, stringbuffer)	_tcscat(buffer, stringbuffer)
#define _tcscpy_s(buffer, buffer_size, stringbuffer)	strcpy(buffer, stringbuffer)

#define _T(x) x
#define _ttol(x) atol(x)

#define _tmain      main

#endif //#ifndef _USR_TCHAR_H
