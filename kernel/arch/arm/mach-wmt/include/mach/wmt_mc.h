/*++
linux/include/asm-arm/arch-wmt/wmt_mc.h

Copyright (c) 2008  WonderMedia Technologies, Inc.

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
#error "You must include hardware.h, not vt8500_mc.h"
#endif

#ifndef __VT8500_MC_H
#define __VT8500_MC_H

/*#include "vt8500_mmap.h" */

/*
 *   Refer Memory Controller.pdf ver. 0.25
 *
 */

/*
 *   MC Registers can only be accessed as data with system privilege.
 *   Accessing the registers without system privilege or without the data
 *   attribute will generate an ERROR response on the AHB.
 */

/* #define MEMORY_CTRL_CFG_BASE_ADDR                       0xF8010800  // 1K , 8/16/32 RW */
/*
 * Address
 */
#define MC_REGION_0_CFG_ADDR					(0x0000+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_REGION_1_CFG_ADDR					(0x0020+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_REGION_2_CFG_ADDR					(0x0040+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_REGION_3_CFG_ADDR					(0x0060+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_AHB_0_MSTR_PFTCH_ALLOWED_ADDR		(0x0080+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_AHB_0_PRESENT_OFFSET_A_B_ADDR		(0x0088+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_AHB_0_MSTR_PFTCH_LOW_OFST_ADDR		(0x0090+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_AHB_0_MSTR_PFTCH_HI_OFST_ADDR		(0x0098+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_AHB_1_MSTR_PFTCH_ALLOWED_ADDR		(0x00A0+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_AHB_1_PRESENT_OFFSET_A_B_ADDR		(0x00A8+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_AHB_1_MSTR_PFTCH_LOW_OFST_ADDR		(0x00B0+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_AHB_1_MSTR_PFTCH_HI_OFST_ADDR		(0x00B8+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_AHB_2_MSTR_PFTCH_ALLOWED_ADDR		(0x00C0+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_AHB_2_PRESENT_OFFSET_A_B_ADDR		(0x00C8+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_AHB_2_MSTR_PFTCH_LOW_OFST_ADDR		(0x00D0+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_AHB_2_MSTR_PFTCH_HI_OFST_ADDR		(0x00D8+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_AHB_3_MSTR_PFTCH_ALLOWED_ADDR		(0x00E0+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_AHB_3_PRESENT_OFFSET_A_B_ADDR		(0x00E8+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_AHB_3_MSTR_PFTCH_LOW_OFST_ADDR		(0x00F0+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_AHB_3_MSTR_PFTCH_HI_OFST_ADDR		(0x00F8+MEMORY_CTRL_CFG_BASE_ADDR)

#define MC_REFRESH_TIMING_ADDR					(0x0100+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_PERFORM_PWR_ADDR						(0x0108+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_DCP_ADDR_ADDR						(0x0110+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_DCP_CTR_ADDR							(0x0118+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_GLB_TIMING_1_ADDR					(0x0130+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_GLB_TIMING_2_ADDR					(0x0138+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_GLB_TIMING_3_ADDR					(0x0140+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_DRV_STRENGTH_CTRL1_ADDR				(0x0160+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_DRV_STRENGTH_CTRL2_ADDR				(0x0168+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_DRV_STRENGTH_CTRL3_ADDR				(0x0170+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_DRV_STRENGTH_STATUS_ADDR				(0x0178+MEMORY_CTRL_CFG_BASE_ADDR)


#define     MC_REGION_0_CFG_OFST               0x00000000
#define     MC_REGION_1_CFG_OFST               0x00000020
#define     MC_REGION_2_CFG_OFST               0x00000040
#define     MC_REGION_3_CFG_OFSTT              0x00000060
#define     MC_AHB_0_MSTR_PFTCH_ALLOWED_OFST   0x00000080
#define     MC_AHB_0_PRESENT_OFFSET_A_B_OFST   0x00000088
#define     MC_AHB_0_MSTR_PFTCH_LOW_OFST_OFST  0x00000090
#define     MC_AHB_0_MSTR_PFTCH_HI_OFST_OFST   0x00000098
#define     MC_AHB_1_MSTR_PFTCH_ALLOWED_OFST   0x000000A0
#define     MC_AHB_1_PRESENT_OFFSET_A_B_OFST   0x000000A8
#define     MC_AHB_1_MSTR_PFTCH_LOW_OFST_OFST  0x000000B0
#define     MC_AHB_1_MSTR_PFTCH_HI_OFST_OFST   0x000000B8
#define     MC_AHB_2_MSTR_PFTCH_ALLOWED_OFST   0x000000C0
#define     MC_AHB_2_PRESENT_OFFSET_A_B_OFST   0x000000C8
#define     MC_AHB_2_MSTR_PFTCH_LOW_OFST_OFST  0x000000D0
#define     MC_AHB_2_MSTR_PFTCH_HI_OFST_OFST   0x000000D8
#define     MC_AHB_3_MSTR_PFTCH_ALLOWED_OFST   0x000000E0
#define     MC_AHB_3_PRESENT_OFFSET_A_B_OFST   0x000000E8
#define     MC_AHB_3_MSTR_PFTCH_LOW_OFST_OFST  0x000000F0
#define     MC_AHB_3_MSTR_PFTCH_HI_OFST_OFST   0x000000F8
#define     MC_REFRESH_TIMING_OFST             0x00000100
#define     MC_PERFORM_PWR_OFST                0x00000108
#define     MC_DCP_ADDR_OFST                   0x00000110
#define     MC_DCP_CTR_OFST                    0x00000118
#define     MC_GLB_TIMING_1_OFST               0x00000130
#define     MC_GLB_TIMING_2_OFST               0x00000138
#define     MC_GLB_TIMING_3_OFST               0x00000140
#define     MC_DRV_STRENGTH_CTRL1_OFST         0x00000160
#define     MC_DRV_STRENGTH_CTRL2_OFST         0x00000168
#define     MC_DRV_STRENGTH_CTRL3_OFST         0x00000170
#define     MC_DRV_STRENGTH_STATUS_REG         0x00000178


/*
 * Registers
 */
#define MC_REGION_0_CFG_REG         REG32_PTR(0x0000+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_REGION_0_PARMS_REG       REG32_PTR(0x0004+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_REGION_1_CFG_REG         REG32_PTR(0x0010+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_REGION_1_PARMS_REG       REG32_PTR(0x0014+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_REGION_2_CFG_REG         REG32_PTR(0x0020+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_REGION_2_PARMS_REG       REG32_PTR(0x0024+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_REGION_3_CFG_REG         REG32_PTR(0x0030+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_REGION_3_PARMS_REG       REG32_PTR(0x0034+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_REGION_4_CFG_REG         REG32_PTR(0x0040+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_REGION_4_PARMS_REG       REG32_PTR(0x0044+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_REGION_5_CFG_REG         REG32_PTR(0x0050+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_REGION_5_PARMS_REG       REG32_PTR(0x0054+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_REGION_6_CFG_REG         REG32_PTR(0x0060+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_REGION_6_PARMS_REG       REG32_PTR(0x0064+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_REGION_7_CFG_REG         REG32_PTR(0x0070+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_REGION_7_PARMS_REG       REG32_PTR(0x0074+MEMORY_CTRL_CFG_BASE_ADDR)

#define MC_CLOCK_CONFIG_REG         REG32_PTR(0x0080+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_BUF_CONTROL_REG          REG32_PTR(0x0084+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_BUF_STATUS_REG           REG32_PTR(0x0088+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_STATUS_REG               REG32_PTR(0x008C+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_DCP_ADDRESS_REG          REG32_PTR(0x0090+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_DCP_WRDATA_REG           REG32_PTR(0x0094+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_DCP_CNTRLS_REG           REG32_PTR(0x0098+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_DCP_RDDATA_REG           REG32_PTR(0x009C+MEMORY_CTRL_CFG_BASE_ADDR)


/*
 * VAL Registers
 */
#define MC_REGION_0_CFG_VAL         REG32_VAL(0x0000+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_REGION_0_PARMS_VAL       REG32_VAL(0x0004+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_REGION_1_CFG_VAL         REG32_VAL(0x0010+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_REGION_1_PARMS_VAL       REG32_VAL(0x0014+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_REGION_2_CFG_VAL         REG32_VAL(0x0020+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_REGION_2_PARMS_VAL       REG32_VAL(0x0024+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_REGION_3_CFG_VAL         REG32_VAL(0x0030+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_REGION_3_PARMS_VAL       REG32_VAL(0x0034+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_REGION_4_CFG_VAL         REG32_VAL(0x0040+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_REGION_4_PARMS_VAL       REG32_VAL(0x0044+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_REGION_5_CFG_VAL         REG32_VAL(0x0050+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_REGION_5_PARMS_VAL       REG32_VAL(0x0054+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_REGION_6_CFG_VAL         REG32_VAL(0x0060+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_REGION_6_PARMS_VAL       REG32_VAL(0x0064+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_REGION_7_CFG_VAL         REG32_VAL(0x0070+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_REGION_7_PARMS_VAL       REG32_VAL(0x0074+MEMORY_CTRL_CFG_BASE_ADDR)

#define MC_CLOCK_CONFIG_VAL         REG32_VAL(0x0080+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_BUF_CONTROL_VAL          REG32_VAL(0x0084+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_BUF_STATUS_VAL           REG32_VAL(0x0088+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_STATUS_VAL               REG32_VAL(0x008C+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_DCP_ADDRESS_VAL          REG32_VAL(0x0090+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_DCP_WRDATA_VAL           REG32_VAL(0x0094+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_DCP_CNTRLS_VAL           REG32_VAL(0x0098+MEMORY_CTRL_CFG_BASE_ADDR)
#define MC_DCP_RDDATA_VAL           REG32_VAL(0x009C+MEMORY_CTRL_CFG_BASE_ADDR)


#define MC_DCP_OPER_STAT_BIT        0x02000000

/*
 *  MC_REGION_X_CFG
 *  start_addr, size , wp , mem_type , bus_width ,
 *  clock_active , clock_sel ,
 */
#define START_ADDR_BASE_UNIT        0x01000000   /* [31:24] -- 16 MB */

#define MC_REGION_0_DEFAULT_ADDR    0x00000000
#define MC_REGION_1_DEFAULT_ADDR    0x10000000
#define MC_REGION_2_DEFAULT_ADDR    0x20000000
#define MC_REGION_3_DEFAULT_ADDR    0x30000000
#define MC_REGION_4_DEFAULT_ADDR    0x40000000
#define MC_REGION_5_DEFAULT_ADDR    0x50000000
#define MC_REGION_6_DEFAULT_ADDR    0x60000000
#define MC_REGION_7_DEFAULT_ADDR    0x70000000

#define MC_SIZE_BASE_UNIT           0x00008000   /* [23:20} -- 32K - 512M   */
#define MC_SIZE_0FF                 0x00000000
#define MC_SIZE_32K                 0x00100000
#define MC_SIZE_64K                 0x00200000
#define MC_SIZE_128K                0x00300000
#define MC_SIZE_256K                0x00400000
#define MC_SIZE_512K                0x00500000
#define MC_SIZE_1M                  0x00600000
#define MC_SIZE_2M                  0x00700000
#define MC_SIZE_4M                  0x00800000
#define MC_SIZE_8M                  0x00900000
#define MC_SIZE_16M                 0x00A00000
#define MC_SIZE_32M                 0x00B00000
#define MC_SIZE_64M                 0x00C00000
#define MC_SIZE_128M                0x00D00000
#define MC_SIZE_256M                0x00E00000
#define MC_SIZE_512M                0x00F00000

#define SET_WRITE_PROTECT           0x00080000   /* [19]             */
#define CLR_WRITE_PROTECT           0x00000000

#define FAST_ASYNC_MEMORY           0x00000000   /* [18:16]          */
#define SLOW_ASYNC_MEMORY           0x00010000
#define PURE_ASYNC_MEMORY           0x00020000
/* Reserved */
#define SYNC_RD_ASYNC_WR_MEMORY     0x00040000   /* for flash memory as K3/K18 */
#define SYNC_MEMORY                 0x00050000   /* for sync static ram */
#define SDR_SDRAM                   0x00060000
#define DDR_SDRAM                   0x00070000
#define MC_MEM_TYPE_MASK            0x00070000

#define MC_BUS_WIDTH_8              0x00000000   /* [15:14] */
#define MC_BUS_WIDTH_16             0x00004000
#define MC_BUS_WIDTH_32             0x00008000
#define MC_BUS_WIDTH_64             0x0000C000
#define MC_BUS_WIDTH_MASK           0x0000C000
/* MC bus width in bit, input REGIONx_CFG */
#define MC_BUS_WIDTH(x)             (8 << (((x) & MC_BUS_WIDTH_MASK) >> 14))

/* Reserved */                                   /* [13] */

#define SET_MEM_CLOCK_ACTIVE        0x00001000   /* [12] */
#define CLR_MEM_CLOCK               0x00000000

#define MEM_CLOCK_SEL_0             0x00000000   /* [11:10] */
#define MEM_CLOCK_SEL_1             0x00000400
#define MEM_CLOCK_SEL_2             0x00000800
#define MEM_CLOCK_SEL_3             0x00000C00


/* Reserved */                                  /* [9:8] */


#define EXTEND_HOLD_ON_RISING       0x00000000  /* [07:07] */
#define EXTEND_HOLD_ON_FALLING      0x00000080

#define WR_DATA_DELAYED             0x00000040  /* [6] */
#define WR_DATA_NO_DELAYED          0x00000000  /* [6] */

#define CHIP_SEL_LATCHED            0x00000020  /* [5] */
#define CHIP_SEL_NO_LATCHED         0x00000000  /* [5] */

#define ADR_ADV_DELAYED             0x00000010  /* [4] */
#define ADR_ADV_NO_DELAYED          0x00000000  /* [4] */

#define READY_LOW_ENABLE            0x00000000  /* [3] */
#define READY_HIGH_ENABLE           0x00000008

#define BUS_TURN_AROUND_0           0x00000000  /* [2:0] */
#define BUS_TURN_AROUND_1           0x00000001
#define BUS_TURN_AROUND_2           0x00000002
#define BUS_TURN_AROUND_3           0x00000003
#define BUS_TURN_AROUND_4           0x00000004
#define BUS_TURN_AROUND_5           0x00000005
#define BUS_TURN_AROUND_6           0x00000006
#define BUS_TURN_AROUND_7           0x00000007


/*  SDRAM */
/* Reserved [9] */
/* Reserved [8] */

#define BANK_1                      0x00000000  /* [7:6] */
#define BANK_2                      0x00000040
#define BANK_4                      0x00000080
#define BANK_8                      0x000000C0

#define ROW_ADRS_BITS_10            0x00000000   /* [5:3] */
#define ROW_ADRS_BITS_11            0x00000008
#define ROW_ADRS_BITS_12            0x00000010
#define ROW_ADRS_BITS_13            0x00000018
#define ROW_ADRS_BITS_14            0x00000020
#define ROW_ADRS_BITS_15            0x00000028
#define ROW_ADRS_BITS_16            0x00000030
#define ROW_ADRS_BITS_17            0x00000038

#define COL_ADRS_BITS_7             0x00000000  /* [2:0] */
#define COL_ADRS_BITS_8             0x00000001
#define COL_ADRS_BITS_9             0x00000002
#define COL_ADRS_BITS_10            0x00000003
#define COL_ADRS_BITS_11            0x00000004
#define COL_ADRS_BITS_12            0x00000005
#define COL_ADRS_BITS_13            0x00000006
#define COL_ADRS_BITS_14            0x00000007


/*
 *  Parameter Register 0x00?4 static memory
 */
#define PAGE_WR_DELAY_0             0x00000000  /* [31:28] */
#define PAGE_WR_DELAY_1             0x10000000
#define PAGE_WR_DELAY_2             0x20000000
#define PAGE_WR_DELAY_3             0x30000000
#define PAGE_WR_DELAY_4             0x40000000
#define PAGE_WR_DELAY_5             0x50000000
#define PAGE_WR_DELAY_6             0x60000000
#define PAGE_WR_DELAY_7             0x70000000
#define PAGE_WR_DELAY_8             0x80000000
#define PAGE_WR_DELAY_9             0x90000000
#define PAGE_WR_DELAY_10            0xA0000000
#define PAGE_WR_DELAY_11            0xB0000000
#define PAGE_WR_DELAY_12            0xC0000000
#define PAGE_WR_DELAY_13            0xD0000000
#define PAGE_WR_DELAY_14            0xE0000000
#define PAGE_WR_DELAY_15            0xF0000000

#
#define WRITE_PAGE_SIZE_1           0x00000000  /* [27:25] */
#define WRITE_PAGE_SIZE_2           0x02000000
#define WRITE_PAGE_SIZE_4           0x04000000
#define WRITE_PAGE_SIZE_8           0x06000000
#define WRITE_PAGE_SIZE_16          0x08000000
#define WRITE_PAGE_SIZE_32          0x0A000000

#define INGORE_WRITE_PAGE           0x00000000  /* WRITE_PAGE_SIZE_1 */
/* Revered PAGE 64  */
/* Revered PAGE 128 */

#define WRITE_EN_DELAY_0            0x00000000  /* [24:23] */
#define WRITE_EN_DELAY_1            0x00800000
#define WRITE_EN_DELAY_2            0x01000000
#define WRITE_EN_DELAY_3            0x01800000

#define WRITE_DONE_DELAY_0          0x00000000  /* [22:16] */
#define WRITE_DONE_DELAY_1          0x00010000
#define WRITE_DONE_DELAY_2          0x00020000
#define WRITE_DONE_DELAY_3          0x00030000
#define WRITE_DONE_DELAY_4          0x00040000
#define WRITE_DONE_DELAY_5          0x00050000
#define WRITE_DONE_DELAY_6          0x00060000
#define WRITE_DONE_DELAY_7          0x00070000
#define WRITE_DONE_DELAY_8          0x00080000
#define WRITE_DONE_DELAY_9          0x00090000
#define WRITE_DONE_DELAY_10         0x000A0000
#define WRITE_DONE_DELAY_11         0x000B0000
#define WRITE_DONE_DELAY_12         0x000C0000
#define WRITE_DONE_DELAY_13         0x000D0000
#define WRITE_DONE_DELAY_14         0x000E0000
#define WRITE_DONE_DELAY_15         0x000F0000
#define WRITE_DONE_DELAY_31         0x001F0000
#define WRITE_DONE_DELAY_63         0x003F0000
#define WRITE_DONE_DELAY_127        0x007F0000

#define PAGE_RD_DELAY_0             0x00000000  /* [15:12] */
#define PAGE_RD_DELAY_1             0x00001000
#define PAGE_RD_DELAY_2             0x00002000
#define PAGE_RD_DELAY_3             0x00003000
#define PAGE_RD_DELAY_4             0x00004000
#define PAGE_RD_DELAY_5             0x00005000
#define PAGE_RD_DELAY_6             0x00006000
#define PAGE_RD_DELAY_7             0x00007000
#define PAGE_RD_DELAY_8             0x00008000
#define PAGE_RD_DELAY_9             0x00009000
#define PAGE_RD_DELAY_10            0x0000A000
#define PAGE_RD_DELAY_11            0x0000B000
#define PAGE_RD_DELAY_12            0x0000C000
#define PAGE_RD_DELAY_13            0x0000D000
#define PAGE_RD_DELAY_14            0x0000E000
#define PAGE_RD_DELAY_15            0x0000F000


#define READ_PAGE_SIZE_1            0x00000000 /* [11:9] */
#define READ_PAGE_SIZE_2            0x00000200
#define READ_PAGE_SIZE_4            0x00000400
#define READ_PAGE_SIZE_8            0x00000600
#define READ_PAGE_SIZE_16           0x00000800
#define READ_PAGE_SIZE_32           0x00000A00
/* Revered PAGE 64  */
/* Revered PAGE 128 */
#define INGORE_READ_PAGE            0x00000000  /* READ_PAGE_SIZE_1 */

#define OUT_EN_DELAY_0              0x00000000  /* [8:7] */
#define OUT_EN_DELAY_1              0x00000080
#define OUT_EN_DELAY_2              0x00000100
#define OUT_EN_DELAY_3              0x00000180

#define READ_VALID_DELAY_0          0x00000000  /* [6:0] */
#define READ_VALID_DELAY_1          0x00000001
#define READ_VALID_DELAY_2          0x00000002
#define READ_VALID_DELAY_3          0x00000003
#define READ_VALID_DELAY_4          0x00000004
#define READ_VALID_DELAY_5          0x00000005
#define READ_VALID_DELAY_6          0x00000006
#define READ_VALID_DELAY_7          0x00000007
#define READ_VALID_DELAY_8          0x00000008
#define READ_VALID_DELAY_9          0x00000009
#define READ_VALID_DELAY_10         0x0000000A
#define READ_VALID_DELAY_11         0x0000000B
#define READ_VALID_DELAY_12         0x0000000C
#define READ_VALID_DELAY_13         0x0000000D
#define READ_VALID_DELAY_14         0x0000000E
#define READ_VALID_DELAY_15         0x0000000F
#define READ_VALID_DELAY_31         0x0000001F
#define READ_VALID_DELAY_63         0x0000003F
#define READ_VALID_DELAY_127        0x0000007F


/*
 *  Parameter Register 0x00?4 dynamic memory
 *
 */

#define PRECHARGE_PERIOD_0          0x00000000  /* [31:28] tRP */
#define PRECHARGE_PEROID_1          0x10000000
#define PRECHARGE_PEROID_2          0x20000000
#define PRECHARGE_PEROID_3          0x30000000
#define PRECHARGE_PEROID_4          0x40000000
#define PRECHARGE_PEROID_5          0x50000000
#define PRECHARGE_PEROID_6          0x60000000
#define PRECHARGE_PEROID_7          0x70000000
#define PRECHARGE_PEROID_8          0x80000000
#define PRECHARGE_PEROID_9          0x90000000
#define PRECHARGE_PEROID_10         0xA0000000
#define PRECHARGE_PEROID_11         0xB0000000
#define PRECHARGE_PEROID_12         0xC0000000
#define PRECHARGE_PEROID_13         0xD0000000
#define PRECHARGE_PEROID_14         0xE0000000
#define PRECHARGE_PEROID_15         0xF0000000


#define ACTIVE_PRECHARGE_0          0x00000000  /* [27:24] tRAS */
#define ACTIVE_PRECHARGE_1          0x01000000
#define ACTIVE_PRECHARGE_2          0x02000000
#define ACTIVE_PRECHARGE_3          0x03000000
#define ACTIVE_PRECHARGE_4          0x04000000
#define ACTIVE_PRECHARGE_5          0x05000000
#define ACTIVE_PRECHARGE_6          0x06000000
#define ACTIVE_PRECHARGE_7          0x07000000
#define ACTIVE_PRECHARGE_8          0x08000000
#define ACTIVE_PRECHARGE_9          0x09000000
#define ACTIVE_PRECHARGE_10         0x0A000000
#define ACTIVE_PRECHARGE_11         0x0B000000
#define ACTIVE_PRECHARGE_12         0x0C000000
#define ACTIVE_PRECHARGE_13         0x0D000000
#define ACTIVE_PRECHARGE_14         0x0E000000
#define ACTIVE_PRECHARGE_15         0x0F000000


#define READ_CYCLE_0                0x00000000  /* [23:20] tRC */
#define READ_CYCLE_1                0x00100000
#define READ_CYCLE_2                0x00200000
#define READ_CYCLE_3                0x00300000
#define READ_CYCLE_4                0x00400000
#define READ_CYCLE_5                0x00500000
#define READ_CYCLE_6                0x00600000
#define READ_CYCLE_7                0x00700000
#define READ_CYCLE_8                0x00800000
#define READ_CYCLE_9                0x00900000
#define READ_CYCLE_10               0x00A00000
#define READ_CYCLE_11               0x00B00000
#define READ_CYCLE_12               0x00C00000
#define READ_CYCLE_13               0x00D00000
#define READ_CYCLE_14               0x00E00000
#define READ_CYCLE_15               0x00F00000


#define AUTO_REFRESH_ACTIVE_0       0x00000000  /* [19:16] tRFC */
#define AUTO_REFRESH_ACTIVE_1       0x00010000
#define AUTO_REFRESH_ACTIVE_2       0x00020000
#define AUTO_REFRESH_ACTIVE_3       0x00030000
#define AUTO_REFRESH_ACTIVE_4       0x00040000
#define AUTO_REFRESH_ACTIVE_5       0x00050000
#define AUTO_REFRESH_ACTIVE_6       0x00060000
#define AUTO_REFRESH_ACTIVE_7       0x00070000
#define AUTO_REFRESH_ACTIVE_8       0x00080000
#define AUTO_REFRESH_ACTIVE_9       0x00090000
#define AUTO_REFRESH_ACTIVE_10      0x000A0000
#define AUTO_REFRESH_ACTIVE_11      0x000B0000
#define AUTO_REFRESH_ACTIVE_12      0x000C0000
#define AUTO_REFRESH_ACTIVE_13      0x000D0000
#define AUTO_REFRESH_ACTIVE_14      0x000E0000
#define AUTO_REFRESH_ACTIVE_15      0x000F0000


#define WRITE_RECOVERY_0            0x00000000  /* [15:12] tWR */
#define WRITE_RECOVERY_1            0x00001000
#define WRITE_RECOVERY_2            0x00002000
#define WRITE_RECOVERY_3            0x00003000
#define WRITE_RECOVERY_4            0x00004000
#define WRITE_RECOVERY_5            0x00005000
#define WRITE_RECOVERY_6            0x00006000
#define WRITE_RECOVERY_7            0x00007000
#define WRITE_RECOVERY_8            0x00008000
#define WRITE_RECOVERY_9            0x00009000
#define WRITE_RECOVERY_10           0x0000A000
#define WRITE_RECOVERY_11           0x0000B000
#define WRITE_RECOVERY_12           0x0000C000
#define WRITE_RECOVERY_13           0x0000D000
#define WRITE_RECOVERY_14           0x0000E000
#define WRITE_RECOVERY_15           0x0000F000


#define CAS_LATENCY_1               0x00000000  /* [11:10] */
#define CAS_LATENCY_2               0x00000400
#define CAS_LATENCY_3               0x00000800
#define CAS_LATENCY_2P5             0x00000C00


#define RAS_LATENCY_1               0x00000000  /* [9:8] */
#define RAS_LATENCY_2               0x00000100
#define RAS_LATENCY_3               0x00000200
#define RAS_LATENCY_4               0x00000300

#define BURST_LENGTH_1              0x00000000  /* [7:5] */
#define BURST_LENGTH_2              0x00000020
#define BURST_LENGTH_4              0x00000040
#define BURST_LENGTH_8              0x00000060
#define BURST_LENGTH_16             0x00000080
#define BURST_LENGTH_32             0x000000A0
/* Revered BURST 64  */
/* Revered BURST 128 */


/*
 *  SDRAM Clock CFG Register 0x0080 dynamic memory
 *
 */

/* Reserved */                                  /* [31:31] */
#define FLASH_PROG_ON               0x40000000  /* [30:30] */
#define FLASH_PROG_OFF              0x00000000

#define SYNC_FLASH_HVOLT_ON         0x20000000  /* [29:29] */
#define SYNC_FLASH_HVOLT_OFF        0x00000000

#define SYNC_FLASH_POWERDOWN_ON     0x10000000  /* [28:28] */
#define SYNC_FLASH_POWERDOWN_OFF    0x00000000

#define MEM_CLOCK3_FAST             0x08000000  /* [27:27] */
#define MEM_CLOCK3_SLOW             0x00000000
#define MEM_CLOCK3_ON               0x00000000  /* [26:25] */
#define MEM_CLOCK3_DYNAMIC          0x02000000
#define MEM_CLOCK3_OFF              0x04000000

#define MEM_CLOCK2_FAST             0x01000000  /* [24:24] */
#define MEM_CLOCK2_SLOW             0x00000000
#define MEM_CLOCK2_ON               0x00000000  /* [23:22] */
#define MEM_CLOCK2_DYNAMIC          0x00400000
#define MEM_CLOCK2_OFF              0x00800000

#define MEM_CLOCK1_FAST             0x00200000  /* [21:21] */
#define MEM_CLOCK1_SLOW             0x00000000
#define MEM_CLOCK1_ON               0x00000000  /* [20:19] */
#define MEM_CLOCK1_DYNAMIC          0x00080000
#define MEM_CLOCK1_OFF              0x00100000

#define MEM_CLOCK0_FAST             0x00040000  /* [18:18] */
#define MEM_CLOCK0_SLOW             0x00000000
#define MEM_CLOCK0_ON               0x00000000  /* [17:16] */
#define MEM_CLOCK0_DYNAMIC          0x00010000
#define MEM_CLOCK0_OFF              0x00020000

#define DEFAULT_REFLASH             0x000002E8  /* [15:00]  64000000 / 8192 / 10.5 = 744 (0x2E8) */
												/*          64000000 / 8192 / 10.4 = 751 (0x2EF) */
												/*          64000000 / 8192 / 10   = 781 (0x30D) */

/*
 *   MC Buffer Controller 0x0084
 *
 */
/* Reserved [31:8] */
#define MC_BUF7_FLUSH              0x00000080  /* [07:07] */
#define MC_BUF6_FLUSH              0x00000040  /* [06:06] */
#define MC_BUF5_FLUSH              0x00000020  /* [05:05] */
#define MC_BUF4_FLUSH              0x00000010  /* [04:04] */
#define MC_BUF3_FLUSH              0x00000008  /* [03:03] */
#define MC_BUF2_FLUSH              0x00000004  /* [02:02] */
#define MC_BUF1_FLUSH              0x00000002  /* [01:01] */
#define MC_BUF0_FLUSH              0x00000001  /* [00:00] */
#define MC_ALL_BUF_FLUSH           0x000000FF

/*
 *   MC Buffer Status 0x0088
 *
 */
#define MC_BUF7_STATUS_MASK         0xF0000000   /* [31:28] */
#define MC_BUF6_STATUS_MASK         0x0F000000   /* [27:24] */
#define MC_BUF5_STATUS_MASK         0x00F00000   /* [23:20] */
#define MC_BUF4_STATUS_MASK         0x000F0000   /* [19:16] */
#define MC_BUF3_STATUS_MASK         0x0000F000   /* [15:12] */
#define MC_BUF2_STATUS_MASK         0x00000F00   /* [11:08] */
#define MC_BUF1_STATUS_MASK         0x000000F0   /* [07:04] */
#define MC_BUF0_STATUS_MASK         0x0000000F   /* [03:00] */

#define MC_BUF_STATU_EMPTY                0x0
#define MC_BUF_STATU_READING              0x1
#define MC_BUF_STATU_WRITE                0x2
#define MC_BUF_STATU_READ_VALID           0x3
#define MC_BUF_STATU_UNASSIGNED_04        0x4
#define MC_BUF_STATU_UNASSIGNED_05        0x5
#define MC_BUF_STATU_MODIFIED_PURGE       0x6
#define MC_BUF_STATU_MODIFIED             0x7
#define MC_BUF_STATU_NC_READING           0x8
#define MC_BUF_STATU_NC_VALID             0x9
#define MC_BUF_STATU_NB_WRITING           0xA
#define MC_BUF_STATU_NBWR_DONE            0xB
#define MC_BUF_STATU_DIRTY                0xC
#define MC_BUF_STATU_DIRTY_PURGE          0xD
#define MC_BUF_STATU_UNASSIGNED_0E        0xE
#define MC_BUF_STATU_UNASSIGNED_0F        0xF

/*
 *   MC Status 0x08C
 *
 */
/* Reserved [31:18] */
#define MC_STATUS_CLOCK_ERROR             0x00020000  /* [17:17] */
#define MC_STATUS_FLASH_STATUS_MASK       0x00010000  /* [16:16] */
#define MC_STATUS_DEQ_ACTIVE_MASK         0x00008000  /* [15:15] */
#define MC_STATUS_DEQ_REGION_ID_MASK      0x00007000  /* [14:12] */
/* Reserved */                                        /* [11:11] */
#define MC_STATUS_DEQ_MTYPE_MASK          0x00000700  /* [10:08] */
#define MC_STATUS_WRITE_PEND_MASK         0x00000080  /* [07:07] */
#define MC_STATUS_DEQ_BUF_ID_MASK         0x00000070  /* [06:04] */
#define MC_STATUS_MASTER_ID_MASK          0x0000000F  /* [03:00] */

/*
 *   DCP Address Register 0x090
 *
 */
/* Reserved [31:31] */
#define MC_DCP_DEVICE_SELECT_MASK         0x70000000  /* [30:28] */
#define MC_DCP_DEVICE_SELECT_NO_0         0x00000000
#define MC_DCP_DEVICE_SELECT_NO_1         0x10000000
#define MC_DCP_DEVICE_SELECT_NO_2         0x20000000
#define MC_DCP_DEVICE_SELECT_NO_3         0x30000000
#define MC_DCP_DEVICE_SELECT_NO_4         0x40000000
#define MC_DCP_DEVICE_SELECT_NO_5         0x50000000
#define MC_DCP_DEVICE_SELECT_NO_6         0x60000000
#define MC_DCP_DEVICE_SELECT_NO_7         0x70000000
/* Reserved [27:27] */
#define MC_DCP_DEVICE_ADDRESS_MASK        0x07FFFFFF  /* [26:00] */


/*
 *   DCP Write Data Register 0x094
 *
 */


/*
 *   DCP Control Sequence Register 0x098
 *
 */
#define MC_DCP_SEQ1_RAS_ASS              0x80000000  /* [31:31] */
#define MC_DCP_SEQ1_RAS_DES              0x00000000
#define MC_DCP_SEQ1_CAS_ASS              0x40000000  /* [30:30] */
#define MC_DCP_SEQ1_CAS_DES              0x00000000
#define MC_DCP_SEQ1_WE_ASS               0x20000000  /* [29:29] */
#define MC_DCP_SEQ1_WE_DES               0x00000000
#define MC_DCP_SEQ1_OE_ASS               0x10000000  /* [28:28] */
#define MC_DCP_SEQ1_OE_DES               0x00000000
#define MC_DCP_SEQ1_CKE_ASS              0x08000000  /* [27:27] */
#define MC_DCP_SEQ1_CKE_DES              0x00000000
#define MC_DCP_SEQ1_CNT_0                0x00000000  /* [26:24] */
#define MC_DCP_SEQ1_CNT_1                0x01000000
#define MC_DCP_SEQ1_CNT_2                0x02000000
#define MC_DCP_SEQ1_CNT_3                0x03000000
#define MC_DCP_SEQ1_CNT_4                0x04000000
#define MC_DCP_SEQ1_CNT_5                0x05000000
#define MC_DCP_SEQ1_CNT_6                0x06000000
#define MC_DCP_SEQ1_CNT_7                0x07000000


#define MC_DCP_SEQ2_RAS_ASS              0x00800000  /* [23:23] */
#define MC_DCP_SEQ2_RAS_DES              0x00000000
#define MC_DCP_SEQ2_CAS_ASS              0x00400000  /* [22:22] */
#define MC_DCP_SEQ2_CAS_DES              0x00000000
#define MC_DCP_SEQ2_WE_ASS               0x00200000  /* [21:21] */
#define MC_DCP_SEQ2_WE_DES               0x00000000
#define MC_DCP_SEQ2_OE_ASS               0x00100000  /* [20:20] */
#define MC_DCP_SEQ2_OE_DES               0x00000000
#define MC_DCP_SEQ2_CKE_ASS              0x00080000  /* [19:19] */
#define MC_DCP_SEQ2_CKE_DES              0x00000000
#define MC_DCP_SEQ2_CNT_0                0x00000000  /* [18:16] */
#define MC_DCP_SEQ2_CNT_1                0x00010000
#define MC_DCP_SEQ2_CNT_2                0x00020000
#define MC_DCP_SEQ2_CNT_3                0x00030000
#define MC_DCP_SEQ2_CNT_4                0x00040000
#define MC_DCP_SEQ2_CNT_5                0x00050000
#define MC_DCP_SEQ2_CNT_6                0x00060000
#define MC_DCP_SEQ2_CNT_7                0x00070000

#define MC_DCP_SEQ3_RAS_ASS              0x00008000  /* [15:15] */
#define MC_DCP_SEQ3_RAS_DES              0x00000000
#define MC_DCP_SEQ3_CAS_ASS              0x00004000  /* [14:14] */
#define MC_DCP_SEQ3_CAS_DES              0x00000000
#define MC_DCP_SEQ3_WE_ASS               0x00002000  /* [13:13] */
#define MC_DCP_SEQ3_WE_DES               0x00000000
#define MC_DCP_SEQ3_OE_ASS               0x00001000  /* [12:12] */
#define MC_DCP_SEQ3_OE_DES               0x00000000
#define MC_DCP_SEQ3_CKE_ASS              0x00000800  /* [11:11] */
#define MC_DCP_SEQ3_CKE_DES              0x00000000
#define MC_DCP_SEQ3_CNT_0                0x00000000  /* [10:08] */
#define MC_DCP_SEQ3_CNT_1                0x00000100
#define MC_DCP_SEQ3_CNT_2                0x00000200
#define MC_DCP_SEQ3_CNT_3                0x00000300
#define MC_DCP_SEQ3_CNT_4                0x00000400
#define MC_DCP_SEQ3_CNT_5                0x00000500
#define MC_DCP_SEQ3_CNT_6                0x00000600
#define MC_DCP_SEQ3_CNT_7                0x00000700

#define MC_DCP_SEQ4_RAS_ASS              0x00000080  /* [07:07] */
#define MC_DCP_SEQ4_RAS_DES              0x00000000
#define MC_DCP_SEQ4_CAS_ASS              0x00000040  /* [06:06] */
#define MC_DCP_SEQ4_CAS_DES              0x00000000
#define MC_DCP_SEQ4_WE_ASS               0x00000020  /* [05:05] */
#define MC_DCP_SEQ4_WE_DES               0x00000000
#define MC_DCP_SEQ4_OE_ASS               0x00000010  /* [04:04] */
#define MC_DCP_SEQ4_OE_DES               0x00000000
#define MC_DCP_SEQ4_CKE_ASS              0x00000008  /* [03:03] */
#define MC_DCP_SEQ4_CKE_DES              0x00000000
#define MC_DCP_SEQ4_CNT_0                0x00000000  /* [02:00] */
#define MC_DCP_SEQ4_CNT_1                0x00000001
#define MC_DCP_SEQ4_CNT_2                0x00000002
#define MC_DCP_SEQ4_CNT_3                0x00000003
#define MC_DCP_SEQ4_CNT_4                0x00000004
#define MC_DCP_SEQ4_CNT_5                0x00000005
#define MC_DCP_SEQ4_CNT_6                0x00000006
#define MC_DCP_SEQ4_CNT_7                0x00000007


/*
 *   DCP Read Data Register  0x09C
 *
 */
#endif /* __VT8500_MC_H */
