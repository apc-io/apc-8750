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

#ifndef __TPCI_H__
#define __TPCI_H__

#if !defined(__TTYPE_H__)
#include "ttype.h"
#endif

/*---------------------  Export Definitions -------------------------*/
#define MAX_PCI_BUS             256     /* max. # of PCI bus that we support */
#define MAX_PCI_DEVICE          32      /* max. # of PCI devices */

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

#define PCI_REG_MAX_SIZE        0x100      /* maximun total PCI registers */

/* added by kevin */
#define PCI_base_addr           0xc0000000
#define PCI_io_range            0x10000
#define PCI_memory_range        0x400000
#define PCI_io_page             0x100      /* 256 byte */
#define PCI_memory_page         0x1000     /* 4k */
#define PCI_invalid             0xffffffff
#define PCI_IO_ENABLE           0x1

/*---------------------  Export Types  ------------------------------*/

/*---------------------  Export Macros ------------------------------*/

/* macro MAKE Bus Dev Fun ID into WORD */
#define MAKE_BDF_TO_WORD(byBusId, byDevId, byFunId) \
	MAKEWORD(((((BYTE)(byDevId)) & 0x1F) << 3) + (((BYTE)(byFunId)) & 0x07), (byBusId))

#define GET_BUSID(wBusDevFunId) \
    HIBYTE(wBusDevFunId)

#define GET_DEVID(wBusDevFunId) \
    (LOBYTE(wBusDevFunId) >> 3)

#define GET_FUNID(wBusDevFunId) \
    (LOBYTE(wBusDevFunId) & 0x07)


/*---------------------  Export Classes  ----------------------------*/

/*---------------------  Export Variables  --------------------------*/

/*---------------------  Export Functions  --------------------------*/

#endif /* __TPCI_H__ */
