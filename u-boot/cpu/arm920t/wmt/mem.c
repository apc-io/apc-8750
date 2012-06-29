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


/*#ifndef _GLOBAL_H_
#include "global.h"
#endif
#ifndef _EXTVARS_H_
#include "extvars.h"
#endif*/

#if !defined(__TMACRO_H__)
#include "tmacro.h"
#endif
#if !defined(__MEM_H__)
#include "mem.h"
#endif
#include <malloc.h>
#include "macif.h"

/*---------------------  Static Definitions -------------------------*/

/*---------------------  Static Classes  ----------------------------*/

/*---------------------  Static Variables  --------------------------*/
extern PVOID g_pvBufferIndex;

/*---------------------  Static Functions  --------------------------*/

/*---------------------  Export Variables  --------------------------*/

/*---------------------  Export Functions  --------------------------*/

VOID
MEMvAlign(
	PSAllocMap pamMem,
	UINT    uBoundary
	)
{
	ULONG ulAlignmentOffset = 0;

	if ((pamMem->dwRawVAddr % uBoundary) != 0)
		ulAlignmentOffset = uBoundary - (pamMem->dwRawVAddr % uBoundary);
	pamMem->dwVAddr = pamMem->dwRawVAddr + ulAlignmentOffset;
	pamMem->dwPAddr = LODWORD(pamMem->qwRawPAddr) + ulAlignmentOffset;
}

/*PVOID
MEMvAllocate(
	UINT    uCount
	)
{
	PVOID   pvMemAddr;

	pvMemAddr = g_pvBufferIndex;
	g_pvBufferIndex += uCount;

	return pvMemAddr;
}*/

VOID
MEMvAllocateShared(
	PSAllocMap pamMem
	)
{
	if (!g_bInit)
		pamMem->dwRawVAddr = (DWORD)mALLOc((size_t)pamMem->dwRawSize * (size_t)sizeof(BYTE));

	/* If allocation failed, virtual/phisical address == NULL */
	if ((PVOID)pamMem->dwRawVAddr == NULL) {
		LODWORD(pamMem->qwRawPAddr) = 0;
		HIDWORD(pamMem->qwRawPAddr) = 0;
		return;
	} else
		memset((PVOID)pamMem->dwRawVAddr, 0, (size_t)pamMem->dwRawSize * (size_t)sizeof(BYTE));

	LODWORD(pamMem->qwRawPAddr) = pamMem->dwRawVAddr;
	HIDWORD(pamMem->qwRawPAddr) = 0;
}

VOID
MEMvCopy(
	PVOID   pvDst,
	PVOID   pvSrc,
	UINT    uCount
	)
{
	memcpy(pvDst, pvSrc, uCount);
}

VOID
MEMvSet(
	PVOID   pvDst,
	BYTE    byPattern,
	UINT    uCount
	)
{
	memset(pvDst, byPattern, uCount);
}
