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


#if !defined(__UPC_H__)
#include "upc.h"
#endif
#if !defined(__MAC_H__)
#include "mac.h"
#endif
#if !defined(__VPCI_H__)
#include "vpci.h"
#endif

#if !defined(__PCINET_H__)
#include "pcinet.h"
#endif

/*---------------------  Static Definitions -------------------------*/

/*---------------------  Static Classes  ----------------------------*/

/*---------------------  Static Variables  --------------------------*/

/*---------------------  Static Functions  --------------------------*/

/*---------------------  Export Variables  --------------------------*/

/*---------------------  Export Functions  --------------------------*/
VOID
VPCIvInitialize(
    DWORD   dwIoBase,
    BYTE    byRevId
    )
{
    /* PATCH.... turn this on to avoid retry forever */
    VPCIvRegBitsOn(dwIoBase, PCI_REG_MODE2, MODE2_PCEROPT);
    /* PATCH.... */
    /* for some legacy BIOS and OS don't open BusM */
    /* bit in PCI configuration space. So, turn it on. */
    VPCIvRegBitsOn(dwIoBase, PCI_REG_COMMAND, COMMAND_BUSM);
    /* PATCH.... turn this on to detect MII coding error */
    VPCIvRegBitsOn(dwIoBase, PCI_REG_MODE3, MODE3_MIION);
}

VOID
VPCIvReadB(
    DWORD   dwIoBase,
    BYTE    byRegOffset,
    PBYTE   pbyData
    )
{
    DWORD   dwPCICfgAddr;


    dwPCICfgAddr = dwIoBase + PCI_CFG_SPACE_OFFSET;
    VNSvInPortB(dwPCICfgAddr + byRegOffset, pbyData);
}

VOID
VPCIvReadW(
    DWORD   dwIoBase,
    BYTE    byRegOffset,
    PWORD   pwData
    )
{
    DWORD   dwPCICfgAddr;


    dwPCICfgAddr = dwIoBase + PCI_CFG_SPACE_OFFSET;
    VNSvInPortW(dwPCICfgAddr + byRegOffset, pwData);
}

VOID
VPCIvReadD(
    DWORD   dwIoBase,
    BYTE    byRegOffset,
    PDWORD   pdwData
    )
{
    DWORD   dwPCICfgAddr;


    dwPCICfgAddr = dwIoBase + PCI_CFG_SPACE_OFFSET;
    VNSvInPortD(dwPCICfgAddr + byRegOffset, pdwData);
}

VOID
VPCIvRegBitsOn(
    DWORD   dwIoBase,
    BYTE    byRegOffset,
    BYTE    byBits
    )
{
    BYTE    byOrgData;


    VPCIvReadB(dwIoBase, byRegOffset, &byOrgData);
    VPCIvWriteB(dwIoBase, byRegOffset, (BYTE)(byOrgData | byBits));
}

VOID
VPCIvRegBitsOff(
    DWORD   dwIoBase,
    BYTE    byRegOffset,
    BYTE    byBits
    )
{
    BYTE    byOrgData;


    VPCIvReadB(dwIoBase, byRegOffset, &byOrgData);
    VPCIvWriteB(dwIoBase, byRegOffset, (BYTE)(byOrgData & ~byBits));
}

VOID
VPCIvWriteB(
    DWORD   dwIoBase,
    BYTE    byRegOffset,
    BYTE    byData
    )
{
    DWORD   dwPCICfgAddr;


    dwPCICfgAddr = dwIoBase + PCI_CFG_SPACE_OFFSET;
    VNSvOutPortB(dwPCICfgAddr + byRegOffset, byData);
}

VOID
VPCIvWriteW(
    DWORD   dwIoBase,
    BYTE    byRegOffset,
    WORD    wData
    )
{
    DWORD   dwPCICfgAddr;


    dwPCICfgAddr = dwIoBase + PCI_CFG_SPACE_OFFSET;
    VNSvOutPortW(dwPCICfgAddr + byRegOffset, wData);
}

VOID
VPCIvWriteD(
    DWORD   dwIoBase,
    BYTE    byRegOffset,
    DWORD    dwData
    )
{
    DWORD   dwPCICfgAddr;


    dwPCICfgAddr = dwIoBase + PCI_CFG_SPACE_OFFSET;
    VNSvOutPortD(dwPCICfgAddr + byRegOffset, dwData);
}
