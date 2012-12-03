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


#ifndef __VPCI_H__
#define __VPCI_H__

#if !defined(__TTYPE_H__)
#include "ttype.h"
#endif

/*---------------------  Export Definitions -------------------------*/
#define PCI_CFG_SPACE_OFFSET    0x0100  /* PCI Configuration Space appended after CSR */

/*
 * Registers in the PCI configuration space
 */
#define PCI_REG_VENDOR_ID       0x00
#define PCI_REG_DEVICE_ID       0x02
#define PCI_REG_COMMAND         0x04
#define PCI_REG_STATUS          0x06
#define PCI_REG_REV_ID          0x08
#define PCI_REG_CLASS_CODE      0x09
#define PCI_REG_CACHELINE_SIZE  0x0C
#define PCI_REG_LAT_TIMER       0x0D
#define PCI_REG_HDR_TYPE        0x0E
#define PCI_REG_BIST            0x0F

#define PCI_REG_BAR0            0x10
#define PCI_REG_BAR1            0x14
#define PCI_REG_BAR2            0x18
#define PCI_REG_CARDBUS_CIS_PTR 0x28

#define PCI_REG_SUB_VEN_ID      0x2C
#define PCI_REG_SUB_SYS_ID      0x2E
#define PCI_REG_EXP_ROM_BAR     0x30
#define PCI_REG_CAP             0x34

#define PCI_REG_INT_LINE        0x3C
#define PCI_REG_INT_PIN         0x3D
#define PCI_REG_MIN_GNT         0x3E
#define PCI_REG_MAX_LAT         0x3F

/*
 * Registers in the PCI configuration space
 */
/* #define PCI_REG_MODE0           0x50    // ? */
#define PCI_REG_MODE0           0x60
#define PCI_REG_MODE1           0x61
#define PCI_REG_MODE2           0x62
#define PCI_REG_MODE3           0x63
#define PCI_REG_DELAY_TIMER     0x64
#define PCI_REG_FIFOTST         0x51
/* #define PCI_REG_MODE2           0x52    // ? */
/* #define PCI_REG_MODE3           0x53    // ? */
/* #define PCI_REG_DELAY_TIMER     0x54    // ? */
#define PCI_REG_FIFOCMD         0x56
#define PCI_REG_FIFOSTA         0x57
#define PCI_REG_BNRY            0x58
#define PCI_REG_CURR            0x5A
#define PCI_REG_FIFO_DATA       0x5C

#define PCI_REG_MAX_SIZE        0x100   /* maximun total PCI registers */

/*
 * Registers for power management (offset)
 */
/* #define PCI_REG_PM_BASE         0x40    // ? */
#define PCI_REG_PM_BASE         0x50
#define PM_CAP_ID               0x00
#define PM_NEXT_ITEM_PTR        0x01
#define PM_PMC0                 0x02
#define PM_PMC1                 0x03
#define PM_PMCSR0               0x04
#define PM_PMCSR1               0x05
#define PM_CSR_BSE              0x06
#define PM_DATA                 0x07

/*
 * Bits in the COMMAND register
 */
#define COMMAND_BUSM            0x04

/*
 * Bits in the MODE0 register
 */
#define MODE0_QPKTDS            0x80

/*
 * Bits in the MODE2 register
 */
#define MODE2_PCEROPT           0x80    /* VT3065 only */
#define MODE2_DISABT            0x40
#define MODE2_MODE10T           0x02

/*
 * Bits in the MODE3 register
 */
#define MODE3_XONOPT            0x80
#define MODE3_TPACEN            0x40
#define MODE3_BACKOPT           0x20
#define MODE3_DLTSEL            0x10
#define MODE3_MIIDMY            0x08
#define MODE3_MIION             0x04

/*
 * Bits in PMCSR0 register
 */
#define PMCSR0_PW_STAT1         0x02
#define PMCSR0_PW_STAT0         0x01

/*
 * Bits in PMCSR1 register
 */
#define PMCSR1_PME_STATUS       0x80
#define PMCSR1_PME_EN           0x01

/*---------------------  Export Types  ------------------------------*/

/*---------------------  Export Macros ------------------------------*/

/*---------------------  Export Classes  ----------------------------*/

/*---------------------  Export Variables  --------------------------*/

/*---------------------  Export Functions  --------------------------*/

#ifdef __cplusplus
extern "C" {                            /* Assume C declarations for C++ */
#endif /* __cplusplus */

VOID
VPCIvInitialize(
    DWORD   dwIoBase,
    BYTE    byRevId
    );

VOID
VPCIvReadB(
    DWORD   dwIoBase,
    BYTE    byRegOffset,
    PBYTE   pbyData
    );

VOID
VPCIvReadW(
    DWORD   dwIoBase,
    BYTE    byRegOffset,
    PWORD   pwData
    );

VOID
VPCIvReadD(
    DWORD   dwIoBase,
    BYTE    byRegOffset,
    PDWORD   pdwData
    );

VOID
VPCIvRegBitsOn(
    DWORD   dwIoBase,
    BYTE    byRegOffset,
    BYTE    byBits
    );

VOID
VPCIvRegBitsOff(
    DWORD   dwIoBase,
    BYTE    byRegOffset,
    BYTE    byBits
    );

VOID
VPCIvWriteB(
    DWORD   dwIoBase,
    BYTE    byRegOffset,
    BYTE    byData
    );

VOID
VPCIvWriteW(
    DWORD   dwIoBase,
    BYTE    byRegOffset,
    WORD    wData
    );

VOID
VPCIvWriteD(
    DWORD   dwIoBase,
    BYTE    byRegOffset,
    DWORD    dwData
    );

#ifdef __cplusplus
}                                       /* End of extern "C" { */
#endif /* __cplusplus */

#endif /* __VPCI_H__ */
