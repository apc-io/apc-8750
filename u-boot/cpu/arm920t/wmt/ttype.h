/*++ 
Copyright (c) 2010 WonderMedia Technologies, Inc.

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software 
Foundation, either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details. You
should have received a copy of the GNU General Public License along with this
program. If not, see http://www.gnu.org/licenses/>.

WonderMedia Technologies, Inc.
10F, 529, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C.
--*/

#include <common.h>
#ifndef __TTYPE_H__
#define __TTYPE_H__

/******* Common definitions and typedefs ***********************************/
#ifndef VOID
#define VOID            void
#endif

#ifndef CONST
#define CONST           const
#endif

#ifndef STATIC
#define STATIC          static
#endif

#if !defined(TRUE)
#define TRUE            1
#endif
#if !defined(FALSE)
#define FALSE           0
#endif

#if !defined(SUCCESS)
#define SUCCESS         0
#endif
#if !defined(FAILED)
#define FAILED          -1
#endif

typedef int             BOOL;

/****** Simple typedefs  ***************************************************/
typedef char            CHAR;
typedef signed short    SHORT;
typedef signed int      INT;
typedef signed long     LONG;

typedef unsigned char   UCHAR;
typedef unsigned short  USHORT;
typedef unsigned int    UINT;
typedef unsigned long   ULONG;

#ifndef BYTE
#define BYTE            unsigned char
#endif

#ifndef WORD
#define WORD            unsigned short
#endif

#ifndef DWORD
#define DWORD           unsigned long
#endif

/* QWORD is for those situation that we want */
/* an 8-byte-aligned 8 byte long structure */
/* which is NOT really a floating point number. */
typedef union tagUQuadWord {
	struct {
		DWORD   dwLowDword;
		DWORD   dwHighDword;
	} u;
	double    DoNotUseThisField;
} UQuadWord;
typedef UQuadWord       QWORD;          /* 64-bit */

#define U8              unsigned char
#define U16             unsigned short
#define USHORT          unsigned short
#define UINT            unsigned int

/* flat memory model */
#if !defined(FAR)
#define FAR
#endif
#if !defined(NEAR)
#define NEAR
#endif
#if !defined(DEF)
#define DEF
#endif
#if !defined(CALLBACK)
#define CALLBACK
#endif

/****** Common pointer types ***********************************************/
typedef signed char *PI8;
typedef signed short *PI16;
typedef signed long *PI32;

typedef unsigned char *PU8;
typedef unsigned short *PU16;
typedef unsigned long *PU32;

/* boolean pointer */
typedef int *PBOOL;

typedef int *PINT;
typedef const int *PCINT;

typedef unsigned int *PUINT;
typedef const unsigned int *PCUINT;

typedef long *PLONG;

typedef BYTE * PBYTE;
typedef const BYTE * PCBYTE;

typedef WORD * PWORD;
typedef const WORD * PCWORD;

typedef DWORD *PDWORD;
typedef const DWORD *PCDWORD;

typedef QWORD * PQWORD;
typedef const QWORD * PCQWORD;

/* typedef void *PVOID; */
typedef void DEF * PVOID;
typedef void NEAR * NPVOID;
typedef void FAR * LPVOID;

/*
 * ANSI (Single-byte Character) types
 */
typedef char DEF * PCH;
typedef char NEAR * NPCH;
typedef char FAR * LPCH;
typedef const char DEF * PCCH;
typedef const char NEAR * NPCCH;
typedef const char FAR * LPCCH;

typedef char DEF * PSTR;
typedef char NEAR * NPSTR;
typedef char FAR * LPSTR;
typedef const char DEF * PCSTR;
typedef const char NEAR * NPCSTR;
typedef const char FAR * LPCSTR;

/*
typedef char *PCH;
typedef const char *PCCH;

typedef char *PSTR;
typedef const char *PCSTR;
*/

/****** Misc definitions, types ********************************************/

/* parameter prefix */
#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

typedef void *HANDLE;
#define MAX_NET_DEVICE            4

#endif /* __TTYPE_H__ */
