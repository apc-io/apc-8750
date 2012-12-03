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

#ifndef __UPCI_H__
#define __UPCI_H__

#if !defined(__UPCIX_H__)
#include "upcix.h"
#endif

/*---------------------  Export Definitions -------------------------*/

/*---------------------  Export Types  ------------------------------*/

/*---------------------  Export Macros ------------------------------*/

#define PCIvReadConfigB(pvHandle, wBusDevFunId, byRegOffset, pbyData)   \
{                                                                       \
    PCIXvReadB(wBusDevFunId, byRegOffset, pbyData);                     \
}

#define PCIvReadConfigW(pvHandle, wBusDevFunId, byRegOffset, pwData)    \
{                                                                       \
    PCIXvReadW(wBusDevFunId, byRegOffset, pwData);                      \
}

#define PCIvReadConfigD(pvHandle, wBusDevFunId, byRegOffset, pdwData)   \
{                                                                       \
    PCIXvReadD(wBusDevFunId, byRegOffset, pdwData);                     \
}

#define PCIvWriteConfigB(pvHandle, wBusDevFunId, byRegOffset, byData)   \
{                                                                       \
    PCIXvWriteB(wBusDevFunId, byRegOffset, byData);                     \
}

#define PCIvWriteConfigW(pvHandle, wBusDevFunId, byRegOffset, wData)    \
{                                                                       \
    PCIXvWriteW(wBusDevFunId, byRegOffset, wData);                      \
}

#define PCIvWriteConfigD(pvHandle, wBusDevFunId, byRegOffset, dwData)   \
{                                                                       \
    PCIXvWriteD(wBusDevFunId, byRegOffset, dwData);                     \
}

#define PCIvReadConfigManyBytes(pvHandle, wBusDevFunId, byRegOffset, pbyBuffer, byCount)    \
{                                                                                           \
    PCIXvReadManyBytes(wBusDevFunId, byRegOffset, pbyBuffer, byCount);                      \
}

#define PCIbFindConfigDeviceInfo(dwDevVenID, pbyBusNum, pbySlotNum, pbyFuncNum, pbyRevId, \
				pdwIoBase, pdwIoSpaceRange, pbyIrqNo) \
    (PCIXbFindDeviceInfo(dwDevVenID, pbyBusNum, pbySlotNum, pbyFuncNum, pbyRevId, pdwIoBase, \
				pdwIoSpaceRange, pbyIrqNo))

/*---------------------  Export Classes  ----------------------------*/

/*---------------------  Export Variables  --------------------------*/

/*---------------------  Export Functions  --------------------------*/

#endif /* __UPCI_H__ */
