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

#ifndef __WMT_NOR_H__
#define __WMT_NOR_H__

#ifdef NOR_CTRL_CFG_BASE_ADDR
#define NOR_BASE_ADDR NOR_CTRL_CFG_BASE_ADDR
#else
#define NOR_BASE_ADDR      0xd8009400
#endif



#ifdef CONFIG_MTD_WMT_NOR

#if (CONFIG_MTD_NOR_CHIP_NUM == 2)
	#define MTDNOR_CHIP_NUM  2
#elif (CONFIG_MTD_NOR_CHIP_NUM == 3)
	#define MTDNOR_CHIP_NUM  3
#elif (CONFIG_MTD_NOR_CHIP_NUM == 4)
	#define MTDNOR_CHIP_NUM  4
#else
	#define MTDNOR_CHIP_NUM  1
#endif

#if(CONFIG_MTD_NOR_CHIP_SIZE == 16)
	#define MTDNOR_CHIP_SIZE  (16 * 1024 * 1024)
#elif (CONFIG_MTD_NOR_CHIP_SIZE == 32)
	#define MTDNOR_CHIP_SIZE  (32 * 1024 * 1024)
#elif (CONFIG_MTD_NOR_CHIP_SIZE == 64)
	#define MTDNOR_CHIP_SIZE  (64 * 1024 * 1024)
#else
	#define MTDNOR_CHIP_SIZE  (8 * 1024 * 1024)
#endif

#if (CONFIG_MTD_NOR_ERASE_SIZE == 128)
	#define MTDNOR_ERASE_SIZE  (128 * 1024)
#elif (CONFIG_MTD_NOR_ERASE_SIZE == 256)
	#define MTDNOR_ERASE_SIZE  (256 * 1024)
#elif (CONFIG_MTD_NOR_ERASE_SIZE == 512)
	#define MTDNOR_ERASE_SIZE  (512 * 1024)
#else
	#define MTDNOR_ERASE_SIZE  (64 * 1024)
#endif

#endif



struct nor_reg_t {
		unsigned long volatile     NOR0_CR;    // 0xD8009400
		unsigned long volatile     NOR0_BUSY_TIMEOUT;              // 0x04
		unsigned long volatile     NOR0_READ_TIMING;    // 0xD8009408
		unsigned long volatile     NOR0_WRITE_TIMING;          // 0x0C

		unsigned long volatile     NOR1_CR;    // 0xD8009410
		unsigned long volatile     NOR1_BUSY_TIMEOUT;              // 0x14
		unsigned long volatile     NOR1_READ_TIMING;    // 0xD8009418
		unsigned long volatile     NOR1_WRITE_TIMING;          // 0x1C

		unsigned long volatile     NOR2_CR;    // 0xD8009420
		unsigned long volatile     NOR2_BUSY_TIMEOUT;              // 0x24
		unsigned long volatile     NOR2_READ_TIMING;    // 0xD8009428
		unsigned long volatile     NOR2_WRITE_TIMING;          // 0x2C

		unsigned long volatile     NOR3_CR;    // 0xD8009430
		unsigned long volatile     NOR3_BUSY_TIMEOUT;              // 0x34
		unsigned long volatile     NOR3_READ_TIMING;    // 0xD8009438
		unsigned long volatile     NOR3_WRITE_TIMING;          // 0x3C

		unsigned long volatile     NOR_RESET_TIMING;    // 0xD8009440
		unsigned long volatile     NOR_INTERRUPT_MASK;              // 0x44
		unsigned long volatile     NOR_INTERRUPT_STATUS;    // 0xD8009448
		unsigned long volatile     NOR_TOTAL_CONTROL;          // 0x4C
		unsigned long volatile     NOR_STATUS;          // 0x50
		unsigned long volatile     NOR_RESERVED1;          // 0x54
		unsigned long volatile     NOR_RESERVED2;          // 0x58
		unsigned long volatile     NOR_RESERVED3;          // 0x5C

		unsigned long volatile     NOR_BASE_0;          // 0x60
		unsigned long volatile     NOR_BASE_1;          // 0x64
		unsigned long volatile     NOR_BASE_2;          // 0x68
		unsigned long volatile     NOR_BASE_3;          // 0x6C
};

#define FLASH_UNKNOWN 0xFFFFFFFF

/* MKid_DEVid : MANUFACTURE ID + DEVICE CODE CYCLE 1 */
/* NUMONYX */
#define NX_MK_M29W640G   0x0020227E /*   8 MB */
#define NX_MK_M29W128G   0x0020227E /*  16 MB */
/* EON */                               
#define E0N_MK_EN29LV640T 0x007F22C9 /*  8 MB */
#define E0N_MK_EN29LV640B 0x007F22CB /*  8 MB */
#define E0N_MK_EN29LV640H 0x007F227E /*  8 MB */
#define E0N_MK_EN29LV640U 0x007F22D7 /*  8 MB */
/* MXIC */
#define MX_MK_EN29LV640T 0x00C222C9 /*  8 MB */
#define MX_MK_EN29LV640B 0x00C222CB /*  8 MB */
#define MX_MK_EN29GL256E 0x00C2227E /*  32 MB */

/* MKid_DEVid1 : DEVICE CODE CYCLE 1 + CYCLE 2 */
/* NUMONYX */
#define	NX_M29W640GT	0x22102201 /*   8 MB */
#define	NX_M29W640GB	0x22102200 /*   8 MB */
#define	NX_M29W640GH	0x220C2201 /*   8 MB */
#define	NX_M29W640GL	0x220C2200 /*   8 MB */
#define	NX_M29W640GL	0x220C2200 /*   8 MB */
#define	NX_M29W128GH	0x22212201 /*  16 MB */
#define	NX_M29W128GL	0x22212200 /*  16 MB */

/* MXIC */

/* SPANSION */

/* SST */

/* WinBond */


#define NOR_IDALL(x, y)	((x<<16)|y)

#endif /* __WMT_NOR_H__ */
