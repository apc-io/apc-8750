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

#ifndef _CHIPTOP_H_
#define _CHIPTOP_H_

#define IN
#define OUT
#define IO

#define true    1
#define false   0
#define NULL    0

typedef unsigned char  uchar;
typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned long  ulong;
typedef unsigned long	 u32;
typedef unsigned short u16;
typedef unsigned char  u8;
typedef unsigned int   err_no_t ;

/*
 * Internal APB Slaves Memory Address Map
 */
#define BA_MC3           0xD8000000	/* DDR/DDR2 Memory Controller Configuration */
#define BA_MSC           0xD800B000	/* MemoryStick Controller Base Address */
#define	BA_MSCDMA        0xD800B100	/* MemoryStick Controller DMA Base Address */
#define BA_DMA           0xD8001000 /* system dma Base Address */
#define BA_RTC           0xD8100000	/* RTC Base Address */
#define BA_GPIO          0xD8110000	/* GPIO Base Address */
#define BA_SCC           0xD8120000	/* System Configuration Control Base Address */
#define BA_PMC           0xD8130000	/* Power Management Control Base Address */
#define BA_IC            0xD8140000	/* Interrupt Controller Base Address */
#define BA_UART0         0xD8200000	/* UART0 Base Address */
#define BA_UART1         0xD8210000	/* UART1 Base Address */
#define BA_SPI           0xD8240000	/* SPI Base Address */
#define BA_I2C           0xD8290000	/* I2C Base Address */
#define BA_PATAHC        0xD8008000 /* PATA HC Base Address */
#define BA_PATAREG       0xD8008270 /* PATA Data /Conmmand Base Address */
#define BA_PATASG        0xD8008500 /* PATA SG Base Address */
#define BA_PATAHC_PCI    0xD8008100 /* PATA PCI Base Address */
#define BA_PATACSREG     0xD8008376 /* PATA Control/Status Base Address */
/* USB Host Controller (UHCI & EHCI) */
/* #define BA_USB           0xD8007000 // USB CPAI Register Base Address */
#define BA_EHCI_PCI      0xD8007800 /* USB 2.0 EHCI USB Host Configuration Base Address */
#define BA_EHCI_REG      0xD8007900 /* USB 2.0 EHCI USB Host Register Base Address */
#define BA_UHCI_PCI      0xD8007A00 /* USB 1.1 UHCI USB Host Configuration Base Address */
#define BA_UHCI_REG      0xD8007B00 /* USB 1.1 UHCI USB Host Register Base Address */
#define BA_CFC           0xD800C000 /* CFC Base Address */
#define BA_DMACFC        0xD800C100 /* CFC Dma Base Address */

#define BA_PCISATAREG    0xC0000AF0
#define BA_PCISATAREG2   0xC0000A70
#define BA_PCIPATAREG    0xC00001F0
#define BA_PCISATACSREG  0xC0000AFA
#define BA_PCISATACSREG2 0xC0000A7A
#define BA_PCIPATACSREG  0xC00001FA
#define BA_PCISATASG     0xC000CC00
#define BA_PCIPATASG     0xC000CC10

/* Public functions */

#endif	/* _CHIPTOP_H_ */
