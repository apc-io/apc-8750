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


#ifndef __TMACRO_H__
#define __TMACRO_H__

#if !defined(__TTYPE_H__)
#include "ttype.h"
#endif

/****** Common helper macros ***********************************************/

#if !defined(LONIBBLE)
#define LONIBBLE(b)         ((BYTE)(b) & 0x0F)
#endif
#if !defined(HINIBBLE)
#define HINIBBLE(b)         ((BYTE)(((WORD)(b) >> 4) & 0x0F))
#endif

#if !defined(LOBYTE)
#define LOBYTE(w)           ((BYTE)(w))
#endif
#if !defined(HIBYTE)
#define HIBYTE(w)           ((BYTE)(((WORD)(w) >> 8) & 0xFF))
#endif

#if !defined(LOWORD)
#define LOWORD(d)           ((WORD)(d))
#endif
#if !defined(HIWORD)
#define HIWORD(d)           ((WORD)((((DWORD)(d)) >> 16) & 0xFFFF))
#endif

#define LODWORD(q)          ((q).u.dwLowDword)
#define HIDWORD(q)          ((q).u.dwHighDword)

#define MAKEBYTE(ln, hn)    ((BYTE)(((BYTE)(ln) & 0x0F) | ((BYTE)(hn) << 4)))
#if !defined(MAKEWORD)
#define MAKEWORD(lb, hb)    ((WORD)(((BYTE)(lb)) | (((WORD)((BYTE)(hb))) << 8)))
#endif
#if !defined(MAKEDWORD)
#define MAKEDWORD(lw, hw)   ((DWORD)(((WORD)(lw)) | (((DWORD)((WORD)(hw))) << 16)))
#endif
#define MAKEQWORD(ld, hd, pq) {pq->u.dwLowDword = ld; pq->u.dwHighDword = hd; }

#if !defined(MAKELONG)
#define MAKELONG(low, high) ((LONG)(((WORD)(low)) | (((DWORD)((WORD)(high))) << 16)))
#endif

#if !defined(REVDWORD)
#define REVDWORD(d) (MAKEDWORD(MAKEWORD(HIBYTE(HIWORD(d)), LOBYTE(HIWORD(d))), \
	MAKEWORD(HIBYTE(LOWORD(d)), LOBYTE(LOWORD(d)))))
#endif

#ifndef NOMINMAX
#ifndef max
#define max(a, b)            (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a, b)            (((a) < (b)) ? (a) : (b))
#endif
#endif /* NOMINMAX */

/****** Misc macros ********************************************************/

/* get the field offset in the type(struct, class, ...) */
#define OFFSET(type, field) ((int)(&((type NEAR*)1)->field)-1)

/* string equality shorthand */
#define STR_EQ(x, y)        (strcmp(x, y) == 0)
#define STR_NE(x, y)        (strcmp(x, y) != 0)


/* calculate element # of array */
#define ELEMENT_NUM(array)  (sizeof(array) / sizeof(array[0]))
#define ARRAY_SIZE(a)       (sizeof(a) / sizeof(a[0]))

/* null statement */
#define NULL_FUNC()

/* Since not all compilers support structure assignment, the ASSIGN()
 * macro is used. This controls how it's actually implemented.
 */
#ifdef	NOSTRUCTASSIGN	/* Version for old compilers that don't support it */
#define	ASSIGN(a, b)	memcpy((char *)&(a), (char *)&(b), sizeof(b);
#else			/* Version for compilers that do */
#define	ASSIGN(a, b)	((a) = (b))
#endif

/* Wrap Around */
#define ADD_ONE_WITH_WRAP_AROUND(uVar, uModulo) {			\
	if ((uVar) >= ((uModulo) - 1))                   		\
		(uVar) = 0;                                      	\
	else                                             		\
		(uVar)++;                                        	\
}

#define ADD_N_WITH_WRAP_AROUND(uVar, uNum, uModulo) {			\
	if (((uVar) + (uNum)) > ((uModulo) - 1))             		\
		(uVar) = (uVar) + (uNum) - (uModulo);                	\
	else                                                 		\
		(uVar) += (uNum);                                    	\
}

#define SUB_ONE_WITH_WRAP_AROUND(uVar, uModulo) {			\
	if ((uVar) <= 0)                                 		\
		(uVar) = ((uModulo) - 1);                        	\
	else                                             		\
		(uVar)--;                                        	\
}

#if defined(__WATCOMC__)
#define RANDOM(_x_)    (UINT)(((UINT)rand()*(_x_)) / (UINT)(RAND_MAX+1))
#else
#define RANDOM(_x_)    random(_x_)
#endif

#endif /* __TMACRO_H__ */
