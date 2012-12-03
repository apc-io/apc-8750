/*++ 
Copyright (c) 2010 WonderMedia Technologies, Inc.

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software 
Foundation, either version 2 of the License, or (at your option) any later 
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details. You
should have received a copy of the GNU General Public License along with this
program. If not, see http://www.gnu.org/licenses/>.

WonderMedia Technologies, Inc.
10F, 529, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C.
--*/

#ifndef __TTCHAR_H__
#define __TTCHAR_H__

#include <stddef.h>

#if !defined(__TTYPE_H__)
#include "ttype.h"
#endif

/*---------------------  Export Definitions -------------------------*/

/*
 * UNICODE (Wide Character) types
 */
typedef wchar_t WCHAR;                  /* wc,   16-bit UNICODE character */

typedef WCHAR * PWCHAR;

typedef WCHAR * PWCH;
typedef WCHAR * LPWCH;
typedef CONST WCHAR * PCWCH;
typedef CONST WCHAR * LPCWCH;

typedef WCHAR * PWSTR;
typedef WCHAR * NWPSTR;
typedef WCHAR * LPWSTR;
typedef CONST WCHAR * PCWSTR;
typedef CONST WCHAR * LPCWSTR;

/*
 * ANSI (Multi-byte Character) types
 */
#ifdef _MBCS

typedef CHAR * PCHAR;

typedef CHAR * PCH;
typedef CHAR * LPCH;
typedef CONST CHAR * PCCH;
typedef CONST CHAR * LPCCH;

typedef CHAR * PSTR;
typedef CHAR * NPSTR;
typedef CHAR * LPSTR;
typedef CONST CHAR * PCSTR;
typedef CONST CHAR * LPCSTR;

#endif /* _MBCS */

/*
 * Neutral ANSI(SBCS,MBCS)/UNICODE types and macros
 */
#ifdef _UNICODE

#ifndef _TCHAR_DEFINED
typedef WCHAR           TCHAR;
typedef WCHAR * PTCHAR;
typedef WCHAR           TBYTE;
typedef WCHAR * PTBYTE;
#define _TCHAR_DEFINED
#endif /* !_TCHAR_DEFINED */

typedef LPWSTR          PTCH;
typedef LPWSTR          LPTCH;
typedef LPWSTR          PTSTR;
typedef LPWSTR          LPTSTR;
typedef LPCWSTR         PCTSTR;
typedef LPCWSTR         LPCTSTR;

#define __TEXT(quote)   L##quote

/* String functions */

#define _tcscat     wcscat
#define _tcschr     wcschr
#define _tcscpy     wcscpy
#define _tcscspn    wcscspn
#define _tcslen     wcslen
#define _tcsncat    wcsncat
#define _tcsncpy    wcsncpy
#define _tcspbrk    wcspbrk
#define _tcsrchr    wcsrchr
#define _tcsspn     wcsspn
#define _tcsstr     wcsstr
#define _tcstok     wcstok

#define _tcsdup     _wcsdup
#define _tcsnset    _wcsnset
#define _tcsrev     _wcsrev
#define _tcsset     _wcsset

#define _tcscmp     wcscmp
#define _tcsicmp    _wcsicmp
#define _tcsnccmp   wcsncmp
#define _tcsncmp    wcsncmp
#define _tcsncicmp  _wcsnicmp
#define _tcsnicmp   _wcsnicmp

#define _tcscoll    wcscoll
#define _tcsicoll   _wcsicoll
#define _tcsnccoll  _wcsncoll
#define _tcsncoll   _wcsncoll
#define _tcsncicoll _wcsnicoll
#define _tcsnicoll  _wcsnicoll

/* ctype functions */

#define _istalnum   iswalnum
#define _istalpha   iswalpha
#define _istascii   iswascii
#define _istcntrl   iswcntrl
#define _istdigit   iswdigit
#define _istgraph   iswgraph
#define _istlower   iswlower
#define _istprint   iswprint
#define _istpunct   iswpunct
#define _istspace   iswspace
#define _istupper   iswupper
#define _istxdigit  iswxdigit

#define _totupper   towupper
#define _totlower   towlower

#else   /* !_UNICODE */

/* ++++++++++++++++++++ SBCS and MBCS ++++++++++++++++++++ */

#ifdef _MBCS
/* ++++++++++++++++++++ MBCS ++++++++++++++++++++ */
/* there are no definitons here, now. */
/* TBD.... */

#else   /* !_MBCS */

/* ++++++++++++++++++++ SBCS ++++++++++++++++++++ */

#ifndef _TCHAR_DEFINED
typedef char            TCHAR;
typedef char *PTCHAR;
typedef unsigned char   TBYTE;
typedef unsigned char *PTBYTE;
#define _TCHAR_DEFINED
#endif /* !_TCHAR_DEFINED */

typedef PCH             PTCH;
typedef LPCH            LPTCH;
typedef PSTR            PTSTR;
typedef LPSTR           LPTSTR;
typedef PCSTR           PCTSTR;
typedef LPCSTR          LPCTSTR;

#define __TEXT(quote)   quote

/* String functions */

#define _tcscat     strcat
#define _tcscpy     strcpy
#define _tcslen     strlen
#define _tcsxfrm    strxfrm
#define _tcsdup     _strdup

/* String functions */

#define _tcschr     strchr
#define _tcscspn    strcspn
#define _tcsncat    strncat
#define _tcsncpy    strncpy
#define _tcspbrk    strpbrk
#define _tcsrchr    strrchr
#define _tcsspn     strspn
#define _tcsstr     strstr
#define _tcstok     strtok

#define _tcsnset    _strnset
#define _tcsrev     _strrev
#define _tcsset     _strset

#define _tcscmp     strcmp
#define _tcsicmp    _stricmp
#define _tcsnccmp   strncmp
#define _tcsncmp    strncmp
#define _tcsncicmp  _strnicmp
#define _tcsnicmp   _strnicmp

#define _tcscoll    strcoll
#define _tcsicoll   _stricoll
#define _tcsnccoll  _strncoll
#define _tcsncoll   _strncoll
#define _tcsncicoll _strnicoll
#define _tcsnicoll  _strnicoll

/* ctype functions */

#define _istascii   isascii
#define _istcntrl   iscntrl
#define _istxdigit  isxdigit

/* ctype functions */

#define _istalnum   isalnum
#define _istalpha   isalpha
#define _istdigit   isdigit
#define _istgraph   isgraph
#define _istlower   islower
#define _istprint   isprint
#define _istpunct   ispunct
#define _istspace   isspace
#define _istupper   isupper

#define _totupper   toupper
#define _totlower   tolower

#endif /* !_MBCS */

#endif /* !_UNICODE */

/* Generic text macros to be used with string literals and character constants.
   Will also allow symbolic constants that resolve to same. */
#define _T(x)       __TEXT(x)

/*---------------------  Export Classes  ----------------------------*/

/*---------------------  Export Variables  --------------------------*/

/*---------------------  Export Functions  --------------------------*/

#endif /* __TTCHAR_H__ */
