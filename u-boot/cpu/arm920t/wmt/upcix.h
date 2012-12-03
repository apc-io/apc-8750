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

#ifndef __UPCIX_H__
#define __UPCIX_H__

#if !defined(__TTYPE_H__)
#include "ttype.h"
#endif

/*---------------------  Export Definitions -------------------------*/

/*---------------------  Export Types  ------------------------------*/

/*---------------------  Export Macros ------------------------------*/

/*---------------------  Export Classes  ----------------------------*/

/*---------------------  Export Variables  --------------------------*/

/*---------------------  Export Functions  --------------------------*/

#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif  /* __cplusplus */

VOID PCIXvReadB(WORD wBusDevFunId, BYTE byRegOffset, PBYTE pbyData);
VOID PCIXvReadW(WORD wBusDevFunId, BYTE byRegOffset, PWORD pwData);
VOID PCIXvReadD(WORD wBusDevFunId, BYTE byRegOffset, PDWORD pdwData);
VOID PCIXvWriteB(WORD wBusDevFunId, BYTE byRegOffset, BYTE byData);
VOID PCIXvWriteW(WORD wBusDevFunId, BYTE byRegOffset, WORD wData);
VOID PCIXvWriteD(WORD wBusDevFunId, BYTE byRegOffset, DWORD dwData);

VOID PCIXvReadManyBytes(WORD wBusDevFunId, BYTE byRegOffset, PBYTE pbyBuffer, WORD wCount);

BOOL PCIXbFindDeviceInfo(DWORD dwDevVenID, PBYTE pbyBusNum,
		PBYTE pbySlotNum, PBYTE pbyFuncNum, PBYTE pbyRevId, PDWORD pdwIoBase,
		PDWORD pdwIoSpaceRange, PBYTE pbyIrqNo);

#ifdef __cplusplus
}                       /* End of extern "C" { */
#endif /* __cplusplus */

#endif /* __UPCIX_H__ */
