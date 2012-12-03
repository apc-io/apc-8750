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

#ifndef __UASSERT_H__
#define __UASSERT_H__

#if !defined(__TTYPE_H__)
#include "ttype.h"
#endif

/*---------------------  Export Definitions -------------------------*/

/* if not define _DEBUG, then DBG_ASSERT will be null */
#ifndef _DEBUG
/* translate ASSERT(f) to empty space */
#define DBG_ASSERT(f)
#define DBG_ASSERT_VALID(f)
#else  /* _DEBUG */
/* translate ASSERT(f) to c function call */
#define DBG_ASSERT(f)
#define DBG_ASSERT_VALID(p)
#endif /* _DEBUG */

/* if (_DEBUG) then (_DEBUG_LOUD) */
#ifndef _DEBUG
#define DBG_PRN(arg)
#else  /* _DEBUG */
#define _DEBUG_LOUD
#define DBG_PRN(arg)                    {DbgPrint("***** "); DbgPrint arg ; }
#endif /* _DEBUG */

#ifndef _DEBUG_LOUD
#define DBG_PRN_LOUD(arg)
#else  /* _DEBUG_LOUD */
#define DBG_PRN_LOUD(arg)               {DbgPrint("##### "); DbgPrint arg ; }
#endif /* _DEBUG_LOUD */

#ifndef _DEBUG_PORT80
#define DBG_IMM_PORT80(handle, value)
#define DBG_PORT80(value)
#else  /* _DEBUG_PORT80 */
#define DBG_IMM_PORT80(handle, value)   NdisImmediateWritePortUchar(handle, 0x80, value);
#define DBG_PORT80(value)               NdisRawWritePortUchar(0x80, value);
#endif /* _DEBUG_PORT80 */

#ifndef _DEBUG_PORT80_ALWAYS
#define DBG_PORT80_ALWAYS(value)
#else  /* _DEBUG_PORT80_ALWAYS */
#define DBG_PORT80_ALWAYS(value)        NdisRawWritePortUchar(0x80, value);
#endif /* _DEBUG_PORT80_ALWAYS */

#ifndef _DEBUG_BREAK
#define DBG_BRK()
#else  /* _DEBUG_BREAK */
#define DBG_BRK()                       { ASM{  int 3   } }
#endif /* _DEBUG_BREAK */

/*---------------------  Export Classes  ----------------------------*/

/*---------------------  Export Variables  --------------------------*/

/*---------------------  Export Functions  --------------------------*/

#endif /* __UASSERT_H__ */
