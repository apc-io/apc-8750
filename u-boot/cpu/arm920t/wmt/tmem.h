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

#ifndef __MEM_H__
#define __MEM_H__

#if !defined(__TTYPE_H__)
#include "ttype.h"
#endif

#include "gmacif.h"
#include "pcinet.h"

/*---------------------  Export Definitions -------------------------*/
/* memory allocation map from RAW(allocated) memory -> ALIGNED memory */
typedef struct tagSAllocMap {
    /* this dwRawSize value must assign */
    DWORD   dwRawSize;                  /* allocated size  of the memory block */
    DWORD   dwRawVAddr;                 /* allocated virtual address of the memory block */
    QWORD   qwRawPAddr;                 /* allocated physical address of the memory block */
    DWORD   dwSize;                     /* aligned size (exactly) */
    DWORD   dwVAddr;                    /* aligned virtual address */
    DWORD   dwPAddr;                    /* aligned physical address */
    PVOID   pvHandle;                   /* handle to do system-call */
} SAllocMap, *PSAllocMap;

#define malloc GMEMvAllocate
/* #define memset MEMvSet */
/*---------------------  Export Types  ------------------------------*/

/*---------------------  Export Macros ------------------------------*/
#define freemem()                                 \
{                                                 \
    g_pvGBufferIndex = (PVOID)GMEMORY_BUFFER_START;\
    g_IO_point = PCI_base_addr;                     \
    g_Mem_point = PCI_base_addr+PCI_io_range ;      \
}
/*---------------------  Export Classes  ----------------------------*/
#define MEMvFree(a, b)
#define MEMvFreeShared(a)
#define free(a);
/*---------------------  Export Variables  --------------------------*/
extern PVOID g_pvGBufferIndex;
/*---------------------  Export Functions  --------------------------*/
#ifdef __cplusplus
extern "C" {                            /* Assume C declarations for C++ */
#endif /* __cplusplus */

VOID
MEMvAlign(
    PSAllocMap pamMem,
    UINT    uBoundary
    );

PVOID
GMEMvAllocate(
    UINT    uCount
    );

VOID
GMEMvAllocateShared(
    PSAllocMap pamMem
    );

VOID
MEMvCopy(
    PVOID   pvDst,
    PVOID   pvSrc,
    UINT    uCount
    );

VOID
MEMvSet(
    PVOID   pvDst,
    BYTE    byPattern,
    UINT    uCount
    );


#ifdef __cplusplus
}                                       /* End of extern "C" { */
#endif /* __cplusplus */

#endif /* __MEM_H__ */
