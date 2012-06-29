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


#ifndef __PCINET_H__
#define __PCINET_H__

#if !defined(__TTYPE_H__)
#include "ttype.h"
#endif
#if !defined(__TPCI_H__)
#include "tpci.h"
#endif


/*---------------------  Export Definitions -------------------------*/
/* #define CB_MAX_NET_DEVICE       6       // max. # of the devices */

/*
 * PCI total devices with the same Vendor id and Device id
 */
typedef struct tagSPciDevice {
    WORD    wDeviceNum;
    WORD    awBusDevFunID[MAX_NET_DEVICE];
} SPciDevice, DEF * PSPciDevice;


/*
 * Registers in the PCI configuration space
 */
#define PCI_REG_MODE0       0x60
#define PCI_REG_MODE1       0x61
#define PCI_REG_MODE2       0x62
#define PCI_REG_MODE3       0x63
#define PCI_REG_DELAY_TIMER 0x64


/*
 * Registers for power management (offset)
 */
#define PCI_REG_PM_BASE     0x50

#define PM_CAP_ID           0x00
#define PM_NEXT_ITEM_PTR    0x01
#define PM_PMC0             0x02
#define PM_PMC1		        0x03
#define PM_PMCSR0	        0x04
#define PM_PMCSR1	        0x05
#define PM_CSR_BSE	        0x06
#define PM_DATA		        0x07


/*
 * Bits in the COMMAND register
 */
#define COMMAND_BUSM        0x04

/*
 * Bits in the MODE0 register
 */
#define MODE0_QPKTDS        0x80
#define MODE0_GWTHSEL       0x04
#define MODE0_FFWHSEL       0x02
#define MODE0_DLWHSEL       0x01

/*
 * Bits in the MODE2 register
 */
#define MODE2_PCEROPT       0x80
#define MODE2_TXQ16         0x40
#define MODE2_TCPLSOPT      0x01

/*
 * Bits in the MODE3 register
 */
#define MODE3_MIION         0x04

/*
 * Bits in PMCSR0 register
 */
#define PMCSR0_PW_STAT1     0x02
#define PMCSR0_PW_STAT0     0x01

/*
 * Bits in PMCSR1 register
 */
#define PMCSR1_PME_STATUS   0x80
#define PMCSR1_PME_EN       0x01


/*---------------------  Export Classes  ----------------------------*/

/*---------------------  Export Variables  --------------------------*/
extern int g_IO_point;
extern int g_Mem_point;
/*---------------------  Export Functions  --------------------------*/
#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif  /* __cplusplus */

BOOL NPCIbIsRegBitsOn(HANDLE hHandle, WORD wBusDevFunId, BYTE byRegOffset, BYTE byBits);
BOOL NPCIbIsRegBitsOff(HANDLE hHandle, WORD wBusDevFunId, BYTE byRegOffset, BYTE byBits);

void NPCIvRegBitsOn(HANDLE hHandle, WORD wBusDevFunId, BYTE byRegOffset, BYTE byBits);
void NPCIvRegBitsOff(HANDLE hHandle, WORD wBusDevFunId, BYTE byRegOffset, BYTE byBits);

void NPCIvReadManyConfigSpace(HANDLE hHandle, WORD wBusDevFunId, BYTE byRegOffset,
		PBYTE pbyBuffer, WORD wCount);

void NPCIvReadRevisionId(HANDLE hHandle, WORD wBusDevFunId, PBYTE pbyRevId);

BOOL NPCIbReadDeviceInfo(HANDLE hHandle, WORD wVendorID, WORD wDeviceID, PSPciDevice pPciDevs);

void NPCIvInitialize(HANDLE hHandle, WORD wBusDevFunId, BYTE byRevId);

BOOL NPCIbPCIAlloc(HANDLE hHandle, WORD wVendorID, WORD wDeviceID, PSPciDevice pPciDevs);

void pciconfigurationdump(WORD wBusDevFunId);

#ifdef __cplusplus
}                       /* End of extern "C" { */
#endif /* __cplusplus */

#endif /* __PCINET_H__ */
