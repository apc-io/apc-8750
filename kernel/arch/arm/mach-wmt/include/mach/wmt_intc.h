/*++
linux/include/asm-arm/arch-wmt/wmt_intc.h

Copyright (c) 2009  WonderMedia Technologies, Inc.

This program is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software Foundation,
either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with
this program.  If not, see <http://www.gnu.org/licenses/>.

WonderMedia Technologies, Inc.
10F, 529, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C.
--*/

/* Be sure that virtual mapping is defined right */
#ifndef __ASM_ARCH_HARDWARE_H
#error "You must include hardware.h, not wmt_intc.h"
#endif

#ifndef __WMT_INTC_H
#define __WMT_INTC_H

/******************************************************************************
 *
 * Define the register access macros.
 *
 * Note: Current policy in standalone program is using register as a pointer.
 *
 ******************************************************************************/
#include "wmt_mmap.h"

/******************************************************************************
 *
 * WM8425 Interrupt Controller Base Address.
 *
 ******************************************************************************/
#ifdef __IC_BASE
#error  "__IC_BASE has already been defined in another file."
#endif
#ifdef INTERRUPT_CTRL_BASE_ADDR        /* From wmt_mmap.h  */
#define __IC_BASE       INTERRUPT0_CTRL_BASE_ADDR
#else
#define __IC_BASE       0xFE140000      /* 64K */
#endif

#ifdef INTERRUPT1_CTRL_BASE_ADDR        /* From wmt_mmap.h */
#define __IC1_BASE       INTERRUPT1_CTRL_BASE_ADDR
#else
#define __IC1_BASE       0xFE150000      /* 64K */
#endif
/******************************************************************************
 *
 * WM8425 Interrupt Controller (IC) control registers.
 *
 * Registers Abbreviations:
 *
 * ICHS         IC Highest-priority Status register. (32RO)
 *
 * ICPC         IC Priority Control register. (32RW)
 *
 * ICDC         IC Destination Control register. (32R,8/16/32W)
 *
 * ICIS         IC Interrupt Status register. (32RO)
 *
 ******************************************************************************/
/******************************************************************************
 *
 * Address constant for each register.
 *
 ******************************************************************************/
#define	ICHS0_ADDR      (__IC_BASE + 0x00)      /* zac_irq    */
#define	ICHS1_ADDR      (__IC_BASE + 0x04)      /* zac_fiq    */
#define	ICHS2_ADDR      (__IC_BASE + 0x08)      /* dss_irq[0] */
#define	ICHS3_ADDR      (__IC_BASE + 0x0C)      /* dss_irq[1] */
#define	ICHS4_ADDR      (__IC_BASE + 0x10)      /* dss_irq[2] */
#define	ICHS5_ADDR      (__IC_BASE + 0x14)      /* dss_irq[3] */
#define	ICHS6_ADDR      (__IC_BASE + 0x18)      /* dss_irq[4] */
#define	ICHS7_ADDR      (__IC_BASE + 0x1C)      /* dss_irq[5] */

#define	ICPC0_ADDR      (__IC_BASE + 0x20)      /* zac_irq    */
#define	ICPC1_ADDR      (__IC_BASE + 0x24)      /* zac_fiq    */
#define	ICPC2_ADDR      (__IC_BASE + 0x28)      /* dss_irq[0] */
#define	ICPC3_ADDR      (__IC_BASE + 0x2C)      /* dss_irq[1] */
#define	ICPC4_ADDR      (__IC_BASE + 0x30)      /* dss_irq[2] */
#define	ICPC5_ADDR      (__IC_BASE + 0x34)      /* dss_irq[3] */
#define	ICPC6_ADDR      (__IC_BASE + 0x38)      /* dss_irq[4] */
#define	ICPC7_ADDR      (__IC_BASE + 0x3C)      /* dss_irq[5] */

#define	ICDC0_ADDR      (__IC_BASE + 0x40)      /* dest_ctl_addr[0]  */
#define	ICDC1_ADDR      (__IC_BASE + 0x41)      /* dest_ctl_addr[1]  */
#define	ICDC2_ADDR      (__IC_BASE + 0x42)      /* dest_ctl_addr[2]  */
#define	ICDC3_ADDR      (__IC_BASE + 0x43)      /* dest_ctl_addr[3]  */
#define	ICDC4_ADDR      (__IC_BASE + 0x44)      /* dest_ctl_addr[4]  */
#define	ICDC5_ADDR      (__IC_BASE + 0x45)      /* dest_ctl_addr[5]  */
#define	ICDC6_ADDR      (__IC_BASE + 0x46)      /* dest_ctl_addr[6]  */
#define	ICDC7_ADDR      (__IC_BASE + 0x47)      /* dest_ctl_addr[7]  */
#define	ICDC8_ADDR      (__IC_BASE + 0x48)      /* dest_ctl_addr[8]  */
#define	ICDC9_ADDR      (__IC_BASE + 0x49)      /* dest_ctl_addr[9]  */
#define	ICDC10_ADDR     (__IC_BASE + 0x4A)      /* dest_ctl_addr[10] */
#define	ICDC11_ADDR     (__IC_BASE + 0x4B)      /* dest_ctl_addr[11] */
#define	ICDC12_ADDR     (__IC_BASE + 0x4C)      /* dest_ctl_addr[12] */
#define	ICDC13_ADDR     (__IC_BASE + 0x4D)      /* dest_ctl_addr[13] */
#define	ICDC14_ADDR     (__IC_BASE + 0x4E)      /* dest_ctl_addr[14] */
#define	ICDC15_ADDR     (__IC_BASE + 0x4F)      /* dest_ctl_addr[15] */
#define	ICDC16_ADDR     (__IC_BASE + 0x50)      /* dest_ctl_addr[16] */
#define	ICDC17_ADDR     (__IC_BASE + 0x51)      /* dest_ctl_addr[17] */
#define	ICDC18_ADDR     (__IC_BASE + 0x52)      /* dest_ctl_addr[18] */
#define	ICDC19_ADDR     (__IC_BASE + 0x53)      /* dest_ctl_addr[19] */
#define	ICDC20_ADDR     (__IC_BASE + 0x54)      /* dest_ctl_addr[20] */
#define	ICDC21_ADDR     (__IC_BASE + 0x55)      /* dest_ctl_addr[21] */
#define	ICDC22_ADDR     (__IC_BASE + 0x56)      /* dest_ctl_addr[22] */
#define	ICDC23_ADDR     (__IC_BASE + 0x57)      /* dest_ctl_addr[23] */
#define	ICDC24_ADDR     (__IC_BASE + 0x58)      /* dest_ctl_addr[24] */
#define	ICDC25_ADDR     (__IC_BASE + 0x59)      /* dest_ctl_addr[25] */
#define	ICDC26_ADDR     (__IC_BASE + 0x5A)      /* dest_ctl_addr[26] */
#define	ICDC27_ADDR     (__IC_BASE + 0x5B)      /* dest_ctl_addr[27] */
#define	ICDC28_ADDR     (__IC_BASE + 0x5C)      /* dest_ctl_addr[28] */
#define	ICDC29_ADDR     (__IC_BASE + 0x5D)      /* dest_ctl_addr[29] */
#define	ICDC30_ADDR     (__IC_BASE + 0x5E)      /* dest_ctl_addr[30] */
#define	ICDC31_ADDR     (__IC_BASE + 0x5F)      /* dest_ctl_addr[31] */
#define	ICDC32_ADDR     (__IC_BASE + 0x60)      /* dest_ctl_addr[32] */
#define	ICDC33_ADDR     (__IC_BASE + 0x61)      /* dest_ctl_addr[33] */
#define	ICDC34_ADDR     (__IC_BASE + 0x62)      /* dest_ctl_addr[34] */
#define	ICDC35_ADDR     (__IC_BASE + 0x63)      /* dest_ctl_addr[35] */
#define	ICDC36_ADDR     (__IC_BASE + 0x64)      /* dest_ctl_addr[36] */
#define	ICDC37_ADDR     (__IC_BASE + 0x65)      /* dest_ctl_addr[37] */
#define	ICDC38_ADDR     (__IC_BASE + 0x66)      /* dest_ctl_addr[38] */
#define	ICDC39_ADDR     (__IC_BASE + 0x67)      /* dest_ctl_addr[39] */
#define	ICDC40_ADDR     (__IC_BASE + 0x68)      /* dest_ctl_addr[40] */
#define	ICDC41_ADDR     (__IC_BASE + 0x69)      /* dest_ctl_addr[41] */
#define	ICDC42_ADDR     (__IC_BASE + 0x6A)      /* dest_ctl_addr[42] */
#define	ICDC43_ADDR     (__IC_BASE + 0x6B)      /* dest_ctl_addr[43] */
#define	ICDC44_ADDR     (__IC_BASE + 0x6C)      /* dest_ctl_addr[44] */
#define	ICDC45_ADDR     (__IC_BASE + 0x6D)      /* dest_ctl_addr[45] */
#define	ICDC46_ADDR     (__IC_BASE + 0x6E)      /* dest_ctl_addr[46] */
#define	ICDC47_ADDR     (__IC_BASE + 0x6F)      /* dest_ctl_addr[47] */
#define	ICDC48_ADDR     (__IC_BASE + 0x70)      /* dest_ctl_addr[48] */
#define	ICDC49_ADDR     (__IC_BASE + 0x71)      /* dest_ctl_addr[49] */
#define	ICDC50_ADDR     (__IC_BASE + 0x72)      /* dest_ctl_addr[50] */
#define	ICDC51_ADDR     (__IC_BASE + 0x73)      /* dest_ctl_addr[51] */
#define	ICDC52_ADDR     (__IC_BASE + 0x74)      /* dest_ctl_addr[52] */
#define	ICDC53_ADDR     (__IC_BASE + 0x75)      /* dest_ctl_addr[53] */
#define	ICDC54_ADDR     (__IC_BASE + 0x76)      /* dest_ctl_addr[54] */
#define	ICDC55_ADDR     (__IC_BASE + 0x77)      /* dest_ctl_addr[55] */
#define	ICDC56_ADDR     (__IC_BASE + 0x78)      /* dest_ctl_addr[56] */
#define	ICDC57_ADDR     (__IC_BASE + 0x79)      /* dest_ctl_addr[57] */
#define	ICDC58_ADDR     (__IC_BASE + 0x7A)      /* dest_ctl_addr[58] */
#define	ICDC59_ADDR     (__IC_BASE + 0x7B)      /* dest_ctl_addr[59] */
#define	ICDC60_ADDR     (__IC_BASE + 0x7C)      /* dest_ctl_addr[60] */
#define	ICDC61_ADDR     (__IC_BASE + 0x7D)      /* dest_ctl_addr[61] */
#define	ICDC62_ADDR     (__IC_BASE + 0x7E)      /* dest_ctl_addr[62] */
#define	ICDC63_ADDR     (__IC_BASE + 0x7F)      /* dest_ctl_addr[63] */

#define	ICIS0A_ADDR     (__IC_BASE + 0x80)      /* zac_irq[0-31]     */
#define	ICIS0B_ADDR     (__IC_BASE + 0x84)      /* zac_irq[32-63]    */
#define	ICIS1A_ADDR     (__IC_BASE + 0x88)      /* zac_fiq[0-31]     */
#define	ICIS1B_ADDR     (__IC_BASE + 0x8C)      /* zac_fiq[32-63]    */
#define	ICIS2A_ADDR     (__IC_BASE + 0x90)      /* dss_irq[0][0-31]  */
#define	ICIS2B_ADDR     (__IC_BASE + 0x94)      /* dss_irq[0][32-63] */
#define	ICIS3A_ADDR     (__IC_BASE + 0x98)      /* dss_irq[1][0-31]  */
#define	ICIS3B_ADDR     (__IC_BASE + 0x9C)      /* dss_irq[1][32-63] */
#define	ICIS4A_ADDR     (__IC_BASE + 0xA0)      /* dss_irq[2][0-31]  */
#define	ICIS4B_ADDR     (__IC_BASE + 0xA4)      /* dss_irq[2][32-63] */
#define	ICIS5A_ADDR     (__IC_BASE + 0xA8)      /* dss_irq[3][0-31]  */
#define	ICIS5B_ADDR     (__IC_BASE + 0xAC)      /* dss_irq[3][32-63] */
#define	ICIS6A_ADDR     (__IC_BASE + 0xB0)      /* dss_irq[4][0-31]  */
#define	ICIS6B_ADDR     (__IC_BASE + 0xB4)      /* dss_irq[4][32-63] */
#define	ICIS7A_ADDR     (__IC_BASE + 0xB8)      /* dss_irq[5][0-31]  */
#define	ICIS7B_ADDR     (__IC_BASE + 0xBC)      /* dss_irq[5][32-63] */

/******************************************************************************
 *
 * Register pointer.
 *
 ******************************************************************************/

#define	ICHS0_REG       (REG32_PTR(ICHS0_ADDR))         /* zac_irq    */
#define	ICHS1_REG       (REG32_PTR(ICHS1_ADDR))         /* zac_fiq    */
#define	ICHS2_REG       (REG32_PTR(ICHS2_ADDR))         /* dss_irq[0] */
#define	ICHS3_REG       (REG32_PTR(ICHS3_ADDR))         /* dss_irq[1] */
#define	ICHS4_REG       (REG32_PTR(ICHS4_ADDR))         /* dss_irq[2] */
#define	ICHS5_REG       (REG32_PTR(ICHS5_ADDR))         /* dss_irq[3] */
#define	ICHS6_REG       (REG32_PTR(ICHS6_ADDR))         /* dss_irq[4] */
#define	ICHS7_REG       (REG32_PTR(ICHS7_ADDR))         /* dss_irq[5] */

#define	ICPC0_REG       (REG32_PTR(ICPC0_ADDR))         /* zac_irq    */
#define	ICPC1_REG       (REG32_PTR(ICPC1_ADDR))         /* zac_fiq    */
#define	ICPC2_REG       (REG32_PTR(ICPC2_ADDR))         /* dss_irq[0] */
#define	ICPC3_REG       (REG32_PTR(ICPC3_ADDR))         /* dss_irq[1] */
#define	ICPC4_REG       (REG32_PTR(ICPC4_ADDR))         /* dss_irq[2] */
#define	ICPC5_REG       (REG32_PTR(ICPC5_ADDR))         /* dss_irq[3] */
#define	ICPC6_REG       (REG32_PTR(ICPC6_ADDR))         /* dss_irq[4] */
#define	ICPC7_REG       (REG32_PTR(ICPC7_ADDR))         /* dss_irq[5] */
#if 0
#define	ICDC0_REG       (REG8_PTR(ICDC0_ADDR))          /* dest_ctl_reg[0]  */
#define	ICDC1_REG       (REG8_PTR(ICDC1_ADDR))          /* dest_ctl_reg[1]  */
#define	ICDC2_REG       (REG8_PTR(ICDC2_ADDR))          /* dest_ctl_reg[2]  */
#define	ICDC3_REG       (REG8_PTR(ICDC3_ADDR))          /* dest_ctl_reg[3]  */
#define	ICDC4_REG       (REG8_PTR(ICDC4_ADDR))          /* dest_ctl_reg[4]  */
#define	ICDC5_REG       (REG8_PTR(ICDC5_ADDR))          /* dest_ctl_reg[5]  */
#define	ICDC6_REG       (REG8_PTR(ICDC6_ADDR))          /* dest_ctl_reg[6]  */
#define	ICDC7_REG       (REG8_PTR(ICDC7_ADDR))          /* dest_ctl_reg[7]  */
#define	ICDC8_REG       (REG8_PTR(ICDC8_ADDR))          /* dest_ctl_reg[8]  */
#define	ICDC9_REG       (REG8_PTR(ICDC9_ADDR))          /* dest_ctl_reg[9]  */
#define	ICDC10_REG      (REG8_PTR(ICDC10_ADDR))         /* dest_ctl_reg[10] */
#define	ICDC11_REG      (REG8_PTR(ICDC11_ADDR))         /* dest_ctl_reg[11] */
#define	ICDC12_REG      (REG8_PTR(ICDC12_ADDR))         /* dest_ctl_reg[12] */
#define	ICDC13_REG      (REG8_PTR(ICDC13_ADDR))         /* dest_ctl_reg[13] */
#define	ICDC14_REG      (REG8_PTR(ICDC14_ADDR))         /* dest_ctl_reg[14] */
#define	ICDC15_REG      (REG8_PTR(ICDC15_ADDR))         /* dest_ctl_reg[15] */
#define	ICDC16_REG      (REG8_PTR(ICDC16_ADDR))         /* dest_ctl_reg[16] */
#define	ICDC17_REG      (REG8_PTR(ICDC17_ADDR))         /* dest_ctl_reg[17] */
#define	ICDC18_REG      (REG8_PTR(ICDC18_ADDR))         /* dest_ctl_reg[18] */
#define	ICDC19_REG      (REG8_PTR(ICDC19_ADDR))         /* dest_ctl_reg[19] */
#define	ICDC20_REG      (REG8_PTR(ICDC20_ADDR))         /* dest_ctl_reg[20] */
#define	ICDC21_REG      (REG8_PTR(ICDC21_ADDR))         /* dest_ctl_reg[21] */
#define	ICDC22_REG      (REG8_PTR(ICDC22_ADDR))         /* dest_ctl_reg[22] */
#define	ICDC23_REG      (REG8_PTR(ICDC23_ADDR))         /* dest_ctl_reg[23] */
#define	ICDC24_REG      (REG8_PTR(ICDC24_ADDR))         /* dest_ctl_reg[24] */
#define	ICDC25_REG      (REG8_PTR(ICDC25_ADDR))         /* dest_ctl_reg[25] */
#define	ICDC26_REG      (REG8_PTR(ICDC26_ADDR))         /* dest_ctl_reg[26] */
#define	ICDC27_REG      (REG8_PTR(ICDC27_ADDR))         /* dest_ctl_reg[27] */
#define	ICDC28_REG      (REG8_PTR(ICDC28_ADDR))         /* dest_ctl_reg[28] */
#define	ICDC29_REG      (REG8_PTR(ICDC29_ADDR))         /* dest_ctl_reg[29] */
#define	ICDC30_REG      (REG8_PTR(ICDC30_ADDR))         /* dest_ctl_reg[30] */
#define	ICDC31_REG      (REG8_PTR(ICDC31_ADDR))         /* dest_ctl_reg[31] */
#define	ICDC32_REG      (REG8_PTR(ICDC32_ADDR))         /* dest_ctl_reg[32] */
#define	ICDC33_REG      (REG8_PTR(ICDC33_ADDR))         /* dest_ctl_reg[33] */
#define	ICDC34_REG      (REG8_PTR(ICDC34_ADDR))         /* dest_ctl_reg[34] */
#define	ICDC35_REG      (REG8_PTR(ICDC35_ADDR))         /* dest_ctl_reg[35] */
#define	ICDC36_REG      (REG8_PTR(ICDC36_ADDR))         /* dest_ctl_reg[36] */
#define	ICDC37_REG      (REG8_PTR(ICDC37_ADDR))         /* dest_ctl_reg[37] */
#define	ICDC38_REG      (REG8_PTR(ICDC38_ADDR))         /* dest_ctl_reg[38] */
#define	ICDC39_REG      (REG8_PTR(ICDC39_ADDR))         /* dest_ctl_reg[39] */
#define	ICDC40_REG      (REG8_PTR(ICDC40_ADDR))         /* dest_ctl_reg[40] */
#define	ICDC41_REG      (REG8_PTR(ICDC41_ADDR))         /* dest_ctl_reg[41] */
#define	ICDC42_REG      (REG8_PTR(ICDC42_ADDR))         /* dest_ctl_reg[42] */
#define	ICDC43_REG      (REG8_PTR(ICDC42_ADDR))         /* dest_ctl_reg[43] */
#define	ICDC44_REG      (REG8_PTR(ICDC44_ADDR))         /* dest_ctl_reg[44] */
#define	ICDC45_REG      (REG8_PTR(ICDC45_ADDR))         /* dest_ctl_reg[45] */
#define	ICDC46_REG      (REG8_PTR(ICDC46_ADDR))         /* dest_ctl_reg[46] */
#define	ICDC47_REG      (REG8_PTR(ICDC47_ADDR))         /* dest_ctl_reg[47] */
#define	ICDC48_REG      (REG8_PTR(ICDC48_ADDR))         /* dest_ctl_reg[48] */
#define	ICDC49_REG      (REG8_PTR(ICDC49_ADDR))         /* dest_ctl_reg[49] */
#define	ICDC50_REG      (REG8_PTR(ICDC50_ADDR))         /* dest_ctl_reg[50] */
#define	ICDC51_REG      (REG8_PTR(ICDC51_ADDR))         /* dest_ctl_reg[51] */
#define	ICDC52_REG      (REG8_PTR(ICDC52_ADDR))         /* dest_ctl_reg[52] */
#define	ICDC53_REG      (REG8_PTR(ICDC53_ADDR))         /* dest_ctl_reg[53] */
#define	ICDC54_REG      (REG8_PTR(ICDC54_ADDR))         /* dest_ctl_reg[54] */
#define	ICDC55_REG      (REG8_PTR(ICDC55_ADDR))         /* dest_ctl_reg[55] */
#define	ICDC56_REG      (REG8_PTR(ICDC56_ADDR))         /* dest_ctl_reg[56] */
#define	ICDC57_REG      (REG8_PTR(ICDC57_ADDR))         /* dest_ctl_reg[57] */
#define	ICDC58_REG      (REG8_PTR(ICDC58_ADDR))         /* dest_ctl_reg[58] */
#define	ICDC59_REG      (REG8_PTR(ICDC59_ADDR))         /* dest_ctl_reg[59] */
#define	ICDC60_REG      (REG8_PTR(ICDC60_ADDR))         /* dest_ctl_reg[60] */
#define	ICDC61_REG      (REG8_PTR(ICDC61_ADDR))         /* dest_ctl_reg[61] */
#define	ICDC62_REG      (REG8_PTR(ICDC62_ADDR))         /* dest_ctl_reg[62] */
#define	ICDC63_REG      (REG8_PTR(ICDC63_ADDR))         /* dest_ctl_reg[63] */
#endif
#define	ICIS0A_REG      (REG32_PTR(ICIS0A_ADDR))        /* zac_irq[0-31]     */
#define	ICIS0B_REG      (REG32_PTR(ICIS0B_ADDR))        /* zac_irq[32-63]    */
#define	ICIS1A_REG      (REG32_PTR(ICIS1A_ADDR))        /* zac_fiq[0-31]     */
#define	ICIS1B_REG      (REG32_PTR(ICIS1B_ADDR))        /* zac_fiq[32-63]    */
#define	ICIS2A_REG      (REG32_PTR(ICIS2A_ADDR))        /* dss_irq[0][0-31]  */
#define	ICIS2B_REG      (REG32_PTR(ICIS2B_ADDR))        /* dss_irq[0][32-63] */
#define	ICIS3A_REG      (REG32_PTR(ICIS3A_ADDR))        /* dss_irq[1][0-31]  */
#define	ICIS3B_REG      (REG32_PTR(ICIS3B_ADDR))        /* dss_irq[1][32-63] */
#define	ICIS4A_REG      (REG32_PTR(ICIS4A_ADDR))        /* dss_irq[2][0-31]  */
#define	ICIS4B_REG      (REG32_PTR(ICIS4B_ADDR))        /* dss_irq[2][32-63] */
#define	ICIS5A_REG      (REG32_PTR(ICIS5A_ADDR))        /* dss_irq[3][0-31]  */
#define	ICIS5B_REG      (REG32_PTR(ICIS5B_ADDR))        /* dss_irq[3][32-63] */
#define	ICIS6A_REG      (REG32_PTR(ICIS6A_ADDR))        /* dss_irq[4][0-31]  */
#define	ICIS6B_REG      (REG32_PTR(ICIS6B_ADDR))        /* dss_irq[4][32-63] */
#define	ICIS7A_REG      (REG32_PTR(ICIS7A_ADDR))        /* dss_irq[5][0-31]  */
#define	ICIS7B_REG      (REG32_PTR(ICIS7B_ADDR))        /* dss_irq[5][32-63] */

/* 16'h00C0-16'hFFFF Reserved (Read-only, all zeros) */

/******************************************************************************
 *
 * Register value.
 *
 ******************************************************************************/
#define	ICHS0_VAL       (REG32_VAL(ICHS0_ADDR))         /* zac_irq    */
#define	ICHS1_VAL       (REG32_VAL(ICHS1_ADDR))         /* zac_fiq    */
#define	ICHS2_VAL	(REG32_VAL(ICHS2_ADDR))            /* dss_irq[0] */
#define	ICHS3_VAL       (REG32_VAL(ICHS3_ADDR))         /* dss_irq[1] */
#define	ICHS4_VAL       (REG32_VAL(ICHS4_ADDR))         /* dss_irq[2] */
#define	ICHS5_VAL       (REG32_VAL(ICHS5_ADDR))         /* dss_irq[3] */
#define	ICHS6_VAL       (REG32_VAL(ICHS6_ADDR))         /* dss_irq[4] */
#define	ICHS7_VAL       (REG32_VAL(ICHS7_ADDR))         /* dss_irq[5] */

#define	ICPC0_VAL       (REG32_VAL(ICPC0_ADDR))         /* zac_irq    */
#define	ICPC1_VAL       (REG32_VAL(ICPC1_ADDR))         /* zac_fiq    */
#define	ICPC2_VAL       (REG32_VAL(ICPC2_ADDR))         /* dss_irq[0] */
#define	ICPC3_VAL       (REG32_VAL(ICPC3_ADDR))         /* dss_irq[1] */
#define	ICPC4_VAL       (REG32_VAL(ICPC4_ADDR))         /* dss_irq[2] */
#define	ICPC5_VAL       (REG32_VAL(ICPC5_ADDR))         /* dss_irq[3] */
#define	ICPC6_VAL       (REG32_VAL(ICPC6_ADDR))         /* dss_irq[4] */
#define	ICPC7_VAL       (REG32_VAL(ICPC7_ADDR))         /* dss_irq[5] */
#if 0
#define	ICDC0_VAL       (REG8_VAL(ICDC0_ADDR))          /* dest_ctl_reg[0]  */
#define	ICDC1_VAL       (REG8_VAL(ICDC1_ADDR))          /* dest_ctl_reg[1]  */
#define	ICDC2_VAL       (REG8_VAL(ICDC2_ADDR))          /* dest_ctl_reg[2]  */
#define	ICDC3_VAL       (REG8_VAL(ICDC3_ADDR))          /* dest_ctl_reg[3]  */
#define	ICDC4_VAL       (REG8_VAL(ICDC4_ADDR))          /* dest_ctl_reg[4]  */
#define	ICDC5_VAL       (REG8_VAL(ICDC5_ADDR))          /* dest_ctl_reg[5]  */
#define	ICDC6_VAL       (REG8_VAL(ICDC6_ADDR))          /* dest_ctl_reg[6]  */
#define	ICDC7_VAL       (REG8_VAL(ICDC7_ADDR))          /* dest_ctl_reg[7]  */
#define	ICDC8_VAL       (REG8_VAL(ICDC8_ADDR))          /* dest_ctl_reg[8]  */
#define	ICDC9_VAL       (REG8_VAL(ICDC9_ADDR))          /* dest_ctl_reg[9]  */
#define	ICDC10_VAL      (REG8_VAL(ICDC10_ADDR))         /* dest_ctl_reg[10] */
#define	ICDC11_VAL      (REG8_VAL(ICDC11_ADDR))         /* dest_ctl_reg[11] */
#define	ICDC12_VAL      (REG8_VAL(ICDC12_ADDR))         /* dest_ctl_reg[12] */
#define	ICDC13_VAL      (REG8_VAL(ICDC13_ADDR))         /* dest_ctl_reg[13] */
#define	ICDC14_VAL      (REG8_VAL(ICDC14_ADDR))         /* dest_ctl_reg[14] */
#define	ICDC15_VAL      (REG8_VAL(ICDC15_ADDR))         /* dest_ctl_reg[15] */
#define	ICDC16_VAL      (REG8_VAL(ICDC16_ADDR))         /* dest_ctl_reg[16] */
#define	ICDC17_VAL      (REG8_VAL(ICDC17_ADDR))         /* dest_ctl_reg[17] */
#define	ICDC18_VAL      (REG8_VAL(ICDC18_ADDR))         /* dest_ctl_reg[18] */
#define	ICDC19_VAL      (REG8_VAL(ICDC19_ADDR))         /* dest_ctl_reg[19] */
#define	ICDC20_VAL      (REG8_VAL(ICDC20_ADDR))         /* dest_ctl_reg[20] */
#define	ICDC21_VAL      (REG8_VAL(ICDC21_ADDR))         /* dest_ctl_reg[21] */
#define	ICDC22_VAL      (REG8_VAL(ICDC22_ADDR))         /* dest_ctl_reg[22] */
#define	ICDC23_VAL      (REG8_VAL(ICDC23_ADDR))         /* dest_ctl_reg[23] */
#define	ICDC24_VAL      (REG8_VAL(ICDC24_ADDR))         /* dest_ctl_reg[24] */
#define	ICDC25_VAL      (REG8_VAL(ICDC25_ADDR))         /* dest_ctl_reg[25] */
#define	ICDC26_VAL      (REG8_VAL(ICDC26_ADDR))         /* dest_ctl_reg[26] */
#define	ICDC27_VAL      (REG8_VAL(ICDC27_ADDR))         /* dest_ctl_reg[27] */
#define	ICDC28_VAL      (REG8_VAL(ICDC28_ADDR))         /* dest_ctl_reg[28] */
#define	ICDC29_VAL      (REG8_VAL(ICDC29_ADDR))         /* dest_ctl_reg[29] */
#define	ICDC30_VAL      (REG8_VAL(ICDC30_ADDR))         /* dest_ctl_reg[30] */
#define	ICDC31_VAL      (REG8_VAL(ICDC31_ADDR))         /* dest_ctl_reg[31] */
#define	ICDC32_VAL      (REG8_VAL(ICDC32_ADDR))         /* dest_ctl_reg[32] */
#define	ICDC33_VAL      (REG8_VAL(ICDC33_ADDR))         /* dest_ctl_reg[33] */
#define	ICDC34_VAL      (REG8_VAL(ICDC34_ADDR))         /* dest_ctl_reg[34] */
#define	ICDC35_VAL      (REG8_VAL(ICDC35_ADDR))         /* dest_ctl_reg[35] */
#define	ICDC36_VAL      (REG8_VAL(ICDC36_ADDR))         /* dest_ctl_reg[36] */
#define	ICDC37_VAL      (REG8_VAL(ICDC37_ADDR))         /* dest_ctl_reg[37] */
#define	ICDC38_VAL      (REG8_VAL(ICDC38_ADDR))         /* dest_ctl_reg[38] */
#define	ICDC39_VAL      (REG8_VAL(ICDC39_ADDR))         /* dest_ctl_reg[39] */
#define	ICDC40_VAL      (REG8_VAL(ICDC40_ADDR))         /* dest_ctl_reg[40] */
#define	ICDC41_VAL      (REG8_VAL(ICDC41_ADDR))         /* dest_ctl_reg[41] */
#define	ICDC42_VAL      (REG8_VAL(ICDC42_ADDR))         /* dest_ctl_reg[42] */
#define	ICDC43_VAL      (REG8_VAL(ICDC42_ADDR))         /* dest_ctl_reg[43] */
#define	ICDC44_VAL      (REG8_VAL(ICDC44_ADDR))         /* dest_ctl_reg[44] */
#define	ICDC45_VAL      (REG8_VAL(ICDC45_ADDR))         /* dest_ctl_reg[45] */
#define	ICDC46_VAL      (REG8_VAL(ICDC46_ADDR))         /* dest_ctl_reg[46] */
#define	ICDC47_VAL      (REG8_VAL(ICDC47_ADDR))         /* dest_ctl_reg[47] */
#define	ICDC48_VAL      (REG8_VAL(ICDC48_ADDR))         /* dest_ctl_reg[48] */
#define	ICDC49_VAL      (REG8_VAL(ICDC49_ADDR))         /* dest_ctl_reg[49] */
#define	ICDC50_VAL      (REG8_VAL(ICDC50_ADDR))         /* dest_ctl_reg[50] */
#define	ICDC51_VAL      (REG8_VAL(ICDC51_ADDR))         /* dest_ctl_reg[51] */
#define	ICDC52_VAL      (REG8_VAL(ICDC52_ADDR))         /* dest_ctl_reg[52] */
#define	ICDC53_VAL      (REG8_VAL(ICDC53_ADDR))         /* dest_ctl_reg[53] */
#define	ICDC54_VAL      (REG8_VAL(ICDC54_ADDR))         /* dest_ctl_reg[54] */
#define	ICDC55_VAL      (REG8_VAL(ICDC55_ADDR))         /* dest_ctl_reg[55] */
#define	ICDC56_VAL      (REG8_VAL(ICDC56_ADDR))         /* dest_ctl_reg[56] */
#define	ICDC57_VAL      (REG8_VAL(ICDC57_ADDR))         /* dest_ctl_reg[57] */
#define	ICDC58_VAL      (REG8_VAL(ICDC58_ADDR))         /* dest_ctl_reg[58] */
#define	ICDC59_VAL      (REG8_VAL(ICDC59_ADDR))         /* dest_ctl_reg[59] */
#define	ICDC60_VAL      (REG8_VAL(ICDC60_ADDR))         /* dest_ctl_reg[60] */
#define	ICDC61_VAL      (REG8_VAL(ICDC61_ADDR))         /* dest_ctl_reg[61] */
#define	ICDC62_VAL      (REG8_VAL(ICDC62_ADDR))         /* dest_ctl_reg[62] */
#define	ICDC63_VAL      (REG8_VAL(ICDC63_ADDR))         /* dest_ctl_reg[63] */
#endif
#define	ICIS0A_VAL      (REG32_VAL(ICIS0A_ADDR))        /* zac_irq[0-31]     */
#define	ICIS0B_VAL      (REG32_VAL(ICIS0B_ADDR))        /* zac_irq[32-63]    */
#define	ICIS1A_VAL      (REG32_VAL(ICIS1A_ADDR))        /* zac_fiq[0-31]     */
#define	ICIS1B_VAL      (REG32_VAL(ICIS1B_ADDR))        /* zac_fiq[32-63]    */
#define	ICIS2A_VAL      (REG32_VAL(ICIS2A_ADDR))        /* dss_irq[0][0-31]  */
#define	ICIS2B_VAL      (REG32_VAL(ICIS2B_ADDR))        /* dss_irq[0][32-63] */
#define	ICIS3A_VAL      (REG32_VAL(ICIS3A_ADDR))        /* dss_irq[1][0-31]  */
#define	ICIS3B_VAL      (REG32_VAL(ICIS3B_ADDR))        /* dss_irq[1][32-63] */
#define	ICIS4A_VAL      (REG32_VAL(ICIS4A_ADDR))        /* dss_irq[2][0-31]  */
#define	ICIS4B_VAL      (REG32_VAL(ICIS4B_ADDR))        /* dss_irq[2][32-63] */
#define	ICIS5A_VAL      (REG32_VAL(ICIS5A_ADDR))        /* dss_irq[3][0-31]  */
#define	ICIS5B_VAL      (REG32_VAL(ICIS5B_ADDR))        /* dss_irq[3][32-63] */
#define	ICIS6A_VAL      (REG32_VAL(ICIS6A_ADDR))        /* dss_irq[4][0-31]  */
#define	ICIS6B_VAL      (REG32_VAL(ICIS6B_ADDR))        /* dss_irq[4][32-63] */
#define	ICIS7A_VAL      (REG32_VAL(ICIS7A_ADDR))        /* dss_irq[5][0-31]  */
#define	ICIS7B_VAL      (REG32_VAL(ICIS7B_ADDR))        /* dss_irq[5][32-63] */
/* 16'h00C0-16'hFFFF Reserved (Read-only, all zeros) */

/******************************************************************************
 *
 * IC Destination Control Register (ICDC_REG) 32-bit access.
 *
 * Note: There are 16 groups of 32-bit accesses to the Destination Control
 *       Registers.
 *
 ******************************************************************************/
#define	ICDC0_REG32     (REG32_PTR(__IC_BASE + 0x40))   /* dest_ctl_reg[0-3]   */
#define	ICDC1_REG32     (REG32_PTR(__IC_BASE + 0x44))   /* dest_ctl_reg[4-7]   */
#define	ICDC2_REG32     (REG32_PTR(__IC_BASE + 0x48))   /* dest_ctl_reg[8-11]  */
#define	ICDC3_REG32     (REG32_PTR(__IC_BASE + 0x4C))   /* dest_ctl_reg[12-15] */
#define	ICDC4_REG32     (REG32_PTR(__IC_BASE + 0x50))   /* dest_ctl_reg[16-19] */
#define	ICDC5_REG32     (REG32_PTR(__IC_BASE + 0x54))   /* dest_ctl_reg[20-23] */
#define	ICDC6_REG32     (REG32_PTR(__IC_BASE + 0x58))   /* dest_ctl_reg[24-27] */
#define	ICDC7_REG32     (REG32_PTR(__IC_BASE + 0x5C))   /* dest_ctl_reg[28-31] */
#define	ICDC8_REG32     (REG32_PTR(__IC_BASE + 0x60))   /* dest_ctl_reg[32-35] */
#define	ICDC9_REG32     (REG32_PTR(__IC_BASE + 0x64))   /* dest_ctl_reg[36-39] */
#define	ICDC10_REG32    (REG32_PTR(__IC_BASE + 0x68))   /* dest_ctl_reg[40-43] */
#define	ICDC11_REG32    (REG32_PTR(__IC_BASE + 0x6C))   /* dest_ctl_reg[44-47] */
#define	ICDC12_REG32    (REG32_PTR(__IC_BASE + 0x70))   /* dest_ctl_reg[48-51] */
#define	ICDC13_REG32    (REG32_PTR(__IC_BASE + 0x74))   /* dest_ctl_reg[52-55] */
#define	ICDC14_REG32    (REG32_PTR(__IC_BASE + 0x78))   /* dest_ctl_reg[56-59] */
#define	ICDC15_REG32    (REG32_PTR(__IC_BASE + 0x7C))   /* dest_ctl_reg[60-63] */

/******************************************************************************
 *
 * IC Destination Control Register (ICDC_REG) 16-bit access.
 *
 * Note: There are 32 groups of 16-bit accesses to the Destination Control
 *       Registers.
 *
 ******************************************************************************/
#define	ICDC0_REG16     (REG16_PTR(__IC_BASE + 0x40))   /* dest_ctl_reg[0-1]   */
#define	ICDC1_REG16     (REG16_PTR(__IC_BASE + 0x42))   /* dest_ctl_reg[2-3]   */
#define	ICDC2_REG16     (REG16_PTR(__IC_BASE + 0x44))   /* dest_ctl_reg[4-5]   */
#define	ICDC3_REG16     (REG16_PTR(__IC_BASE + 0x46))   /* dest_ctl_reg[6-7]   */
#define	ICDC4_REG16     (REG16_PTR(__IC_BASE + 0x48))   /* dest_ctl_reg[8-9]   */
#define	ICDC5_REG16     (REG16_PTR(__IC_BASE + 0x4A))   /* dest_ctl_reg[10-11] */
#define	ICDC6_REG16     (REG16_PTR(__IC_BASE + 0x4C))   /* dest_ctl_reg[12-13] */
#define	ICDC7_REG16     (REG16_PTR(__IC_BASE + 0x4E))   /* dest_ctl_reg[14-15] */
#define	ICDC8_REG16     (REG16_PTR(__IC_BASE + 0x50))   /* dest_ctl_reg[16-17] */
#define	ICDC9_REG16     (REG16_PTR(__IC_BASE + 0x52))   /* dest_ctl_reg[18-19] */
#define	ICDC10_REG16    (REG16_PTR(__IC_BASE + 0x54))   /* dest_ctl_reg[20-21] */
#define	ICDC11_REG16    (REG16_PTR(__IC_BASE + 0x56))   /* dest_ctl_reg[22-23] */
#define	ICDC12_REG16    (REG16_PTR(__IC_BASE + 0x58))   /* dest_ctl_reg[24-25] */
#define	ICDC13_REG16    (REG16_PTR(__IC_BASE + 0x5A))   /* dest_ctl_reg[26-27] */
#define	ICDC14_REG16    (REG16_PTR(__IC_BASE + 0x5C))   /* dest_ctl_reg[28-29] */
#define	ICDC15_REG16    (REG16_PTR(__IC_BASE + 0x5E))   /* dest_ctl_reg[30-31] */
#define	ICDC16_REG16    (REG16_PTR(__IC_BASE + 0x60))   /* dest_ctl_reg[32-33] */
#define	ICDC17_REG16    (REG16_PTR(__IC_BASE + 0x62))   /* dest_ctl_reg[34-35] */
#define	ICDC18_REG16    (REG16_PTR(__IC_BASE + 0x64))   /* dest_ctl_reg[36-37] */
#define	ICDC19_REG16    (REG16_PTR(__IC_BASE + 0x66))   /* dest_ctl_reg[38-39] */
#define	ICDC20_REG16    (REG16_PTR(__IC_BASE + 0x68))   /* dest_ctl_reg[40-41] */
#define	ICDC21_REG16    (REG16_PTR(__IC_BASE + 0x6A))   /* dest_ctl_reg[42-43] */
#define	ICDC22_REG16    (REG16_PTR(__IC_BASE + 0x6C))   /* dest_ctl_reg[44-45] */
#define	ICDC23_REG16    (REG16_PTR(__IC_BASE + 0x6E))   /* dest_ctl_reg[46-47] */
#define	ICDC24_REG16    (REG16_PTR(__IC_BASE + 0x70))   /* dest_ctl_reg[48-49] */
#define	ICDC25_REG16    (REG16_PTR(__IC_BASE + 0x72))   /* dest_ctl_reg[50-51] */
#define	ICDC26_REG16    (REG16_PTR(__IC_BASE + 0x74))   /* dest_ctl_reg[52-53] */
#define	ICDC27_REG16    (REG16_PTR(__IC_BASE + 0x76))   /* dest_ctl_reg[54-55] */
#define	ICDC28_REG16    (REG16_PTR(__IC_BASE + 0x78))   /* dest_ctl_reg[56-57] */
#define	ICDC29_REG16    (REG16_PTR(__IC_BASE + 0x7A))   /* dest_ctl_reg[58-59] */
#define	ICDC30_REG16    (REG16_PTR(__IC_BASE + 0x7C))   /* dest_ctl_reg[60-61] */
#define	ICDC31_REG16    (REG16_PTR(__IC_BASE + 0x7E))   /* dest_ctl_reg[62-63] */

/******************************************************************************
 *
 * ICHS         Interrupt Controller Highest-priority Status register macro.
 *
 ******************************************************************************/
#define ICHS_NUM        0x08            /* status registers number = 8    */
#define ICHS_NUMMASK    0x07            /* mask to avoid overflow         */
#define ICHS_OFFSET(x)  (4 * ((x) & ICHS_NUMMASK))      /* address offset */
#define ICHS_REG(x)     (REG32_PTR(ICHS0_ADDR + ICHS_OFFSET(x)))        /* 0 to 7 */
#define ICHS_VAL(x)     (REG32_VAL(ICHS0_ADDR + ICHS_OFFSET(x)))        /* 0 to 7 */

/******************************************************************************
 *
 * ICPC         Interrupt Controller Priority Control register macro.
 *
 ******************************************************************************/
#define	ICPC_PRIMASK    (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5)
#define	ICPC_ROTATING   BIT6                     /* Rotating Priority Enable bit  */
#define ICPC_NUM        0x08                     /* pri_ctl registers number = 8  */
#define ICPC_NUMMASK    0x07                            /* mask to avoid overflow */
#define ICPC_OFFSET(x)  (4 * ((x) & ICPC_NUMMASK))              /* address offset */
#define ICPC_REG(x)     (REG32_PTR(ICPC0_ADDR + ICPC_OFFSET(x)))        /* 0 to 7 */
#define ICPC_VAL(x)     (REG32_VAL(ICPC0_ADDR + ICPC_OFFSET(x)))        /* 0 to 7 */

/******************************************************************************
 *
 * ICDC         Interrupt Controller Destination Control register macro.
 *
 ******************************************************************************/
#define	ICDC_IRQ			0x00                    /* Route to zac_irq     */
#define	ICDC_FIQ			0x01                    /* Route to zac_fiq     */
#define	ICDC_DSS0			0x02                    /* Route to dss_irq[0]  */
#define	ICDC_DSS1			0x03                    /* Route to dss_irq[1]  */
#define	ICDC_DSS2			0x04                    /* Route to dss_irq[2]  */
#define	ICDC_DSS3			0x05                    /* Route to dss_irq[3]  */
#define	ICDC_DSS4			0x06                    /* Route to dss_irq[4]  */
#define	ICDC_DSS5			0x07                    /* Route to dss_irq[5]  */
#define	ICDC_ROUTEMASK  (BIT0 | BIT1 | BIT2)  /* Bits field to setup interrupt routing */
#define	ICDC_ENABLE     BIT3                  /* Interrupt enable bit.*/

#define ICDC_NUM_WMT           0x80           /* dest_ctl registers number = 64 */
#define ICDC_NUMMASK_WMT    0x7F           /* mask to avoid overflow */
#define ICDC_OFFSET_WMT(x)  ((x) & ICDC_NUMMASK_WMT)                     /* address offset WMT */
#define ICDC0_REG(x)     (REG8_PTR(ICDC0_ADDR + ICDC_OFFSET_WMT(x)))        /* 0 to 63 */
#define ICDC0_VAL(x)     (REG8_VAL(ICDC0_ADDR + ICDC_OFFSET_WMT(x)))        /* 0 to 63 */
#define ICDC1_REG(x)     (REG8_PTR(__IC1_BASE + 0x40 + ICDC_OFFSET_WMT(x))) /* 0 to 63 */
#define ICDC1_VAL(x)     (REG8_VAL(__IC1_BASE + 0x40 + ICDC_OFFSET_WMT(x))) /* 0 to 63 */

/******************************************************************************
 *
 * ICIS         Interrupt Controller Interrupt Status Register macro.
 *
 ******************************************************************************/
#define ICIS_NUM        0x08                   /* status registers pairs = 8 */
#define ICIS_NUMMASK    0x07                       /* mask to avoid overflow */
#define ICIS_OFFSET(x)  (8 * ((x) & ICIS_NUMMASK))         /* address offset */
#define ICIS_A_REG(x)   (REG32_PTR(ICIS0A_ADDR + ICDC_OFFSET(x)))  /* 0 to 7 */
#define ICIS_B_REG(x)   (REG32_PTR(ICIS0B_ADDR + ICDC_OFFSET(x)))  /* 0 to 7 */
#define ICIS_A_VAL(x)   (REG32_VAL(ICIS0A_ADDR + ICDC_OFFSET(x)))  /* 0 to 7 */
#define ICIS_B_VAL(x)   (REG32_VAL(ICIS0B_ADDR + ICDC_OFFSET(x)))  /* 0 to 7 */

#endif /* __WMT_INTC_H */
