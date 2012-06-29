/*++
linux/include/asm-arm/arch-wmt/wmt_sf.h

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
#error "You must include hardware.h, not wmt_sf.h"
#endif

#ifndef __WMT_SF1_H__
#define __WMT_SF1_H__


/******************************************************************************
 * memory use macro
 ******************************************************************************/
#define MEM8(addr)		(*(volatile char *)(addr))
#define MEM32(addr)		(MEM16(addr+2)<<16 | MEM16(addr))
#define MEM16(addr)		(MEM8(addr+1)<<8 | MEM8(addr))

#define SF_BASE_ADDR SF_MEM_CTRL_CFG_BASE_ADDR


/*Chip select 0-1 configuration register, 0x0 & 0x8 */
#define SF_START_ADDR          0xFF800000 /* [36:16] */
#define SF_MEM_SIZE_8M         0x00000800 /* [11:8]  */

/*SPI interface configuration register, 0x40 */
#define SF_PDWN_DELY           0x0  /* [31:28] */
#define SF_RES_DELY            0x0  /* [27:24] */
#define SF_CS_DELY             0x0  /* [18:16] */
#define SF_PROG_CMD_MOD_EN     0x40 /* [6:6] */
#define SF_PROG_CMD_MOD_DIS    0x0  /* [6:6] */
#define SF_USR_WR_CMD_MOD_EN   0x20 /* [5:5] */
#define SF_USR_WR_CMD_MOD_DIS  0x0  /* [5:5] */
#define SF_USR_RD_CMD_MOD_EN   0x10 /* [4:4] */
#define SF_USR_RD_CMD_MOD_DIS  0x0  /* [4:4] */
#define SF_ADDR_WIDTH_24       0x0  /* [0:0] */
#define SF_ADDR_WIDTH_32       0x1  /* [0:0] */
/*SPI flash read/write control register, 0x50 */
#define SF_ID_RD               0x10 /* [4:4] */
#define SF_STATUS_RD           0x0  /* [4:4] */
#define SF_RD_SPD_FAST         0x1  /* [0:0] */
#define SF_RD_SPD_NOR          0x0  /* [0:0] */

/*SPI flash write enable control register, 0x60 */
#define SF_CS1_WR_EN           0x2  /* [1:1] */
#define SF_CS1_WR_DIS          0x0  /* [1:1] */
#define SF_CS0_WR_EN           0x1  /* [0:0] */
#define SF_CS0_WR_DIS          0x0  /* [0:0] */

/*SPI flash erase control register, 0x70 */
#define SF_SEC_ER_EN           0x8000 /* [15:15] */
#define SF_SEC_ER_DIS          0x0  /* [15:15] */
#define SF_CHIP_ER_EN          0x1  /* [0:0] */
#define SF_CHIP_ER_DIS         0x0  /* [0:0] */

/*SPI flash erase start address register, 0x74 */
#define SF_ER_START_ADDR       0x0  /* [31:16] */
#define CHIP_ER_CS1            0x2  /* [1:1] */
#define CHIP_ER_CS0            0x1  /* [0:0] */

/*SPI flash error status register, 0x80 */
#define SF_WR_PROT_ERR         0x20 /* [5:5] */
#define SF_MEM_REGION_ERR      0x10 /* [4:4] */
#define SF_PWR_DWN_ACC_ERR     0x8  /* [3:3] */
#define SF_PCMD_OP_ERR         0x4  /* [2:2] */
#define SF_PCMD_ACC_ERR        0x2  /* [1:1] */
#define SF_MASLOCK_ERR         0x1  /* [0:0] */
/*SPI power down control register, 0x180 & 0x190 */
#define PWR_DWN_EN             0x1  /* [0:0] */
#define PWR_DWN_DIS            0x0  /* [0:0] */
/*SPI programmable command mode control register, 0x200 */
#define SF_TX_DATA_SIZE        0x0  /* MACRO [30:24] */
#define SF_RX_DATA_SIZE        0x0  /* MACRO [22:16] */
#define SF_CMD_CS1             0x2  /* MACRO [1:1] */
#define SF_CMD_CS0             0x0  /* MACRO [1:1] */
#define SF_CMD_EN              0x1  /* [0:0] */
#define SF_CMD_DIS             0x0  /* [0:0] */
/*SPI user command value register, 0x210 */
#define SF_USR_WR_CMD          0x0  /* MACRO [23:16] */
#define SF_USR_RD_CMD          0x0  /* MACRO [7:0] */


#define CHIP_SEL_0_CFG_ADDR		(SF_BASE_ADDR + 0x00)
#define CHIP_SEL_1_CFG_ADDR		(SF_BASE_ADDR + 0x08)
#define SPI_INTF_CFG_ADDR		(SF_BASE_ADDR + 0x40)
#define SPI_RD_WR_CTR_ADDR		(SF_BASE_ADDR + 0x50)
#define SPI_WR_EN_CTR_ADDR		(SF_BASE_ADDR + 0x60)
#define SPI_ER_CTR_ADDR			(SF_BASE_ADDR + 0x70)
#define SPI_ER_START_ADDR_ADDR	(SF_BASE_ADDR + 0x74)
#define SPI_ERROR_STATUS_ADDR	(SF_BASE_ADDR + 0x80)
#define SPI_MEM_0_SR_ACC_ADDR	(SF_BASE_ADDR + 0x100)
#define SPI_MEM_1_SR_ACC_ADDR	(SF_BASE_ADDR + 0x110)
#define SPI_PDWN_CTR_0_ADDR		(SF_BASE_ADDR + 0x180)
#define SPI_PDWN_CTR_1_ADDR		(SF_BASE_ADDR + 0x190)
#define SPI_PROG_CMD_CTR_ADDR	(SF_BASE_ADDR + 0x200)
#define SPI_USER_CMD_VAL_ADDR	(SF_BASE_ADDR + 0x210)
#define SPI_PROG_CMD_WBF_ADDR	(SF_BASE_ADDR + 0x300)
#define SPI_PROG_CMD_RBF_ADDR	(SF_BASE_ADDR + 0x380)

#define	CHIP_SEL_0_CFG_M		(REG32_VAL(CHIP_SEL_0_CFG_ADDR))
#define	CHIP_SEL_1_CFG_M		(REG32_VAL(CHIP_SEL_1_CFG_ADDR))
#define	SPI_INTF_CFG_M			(REG32_VAL(SPI_INTF_CFG_ADDR))
#define	SPI_RD_WR_CTR_M			(REG32_VAL(SPI_RD_WR_CTR_ADDR))
#define	SPI_WR_EN_CTR_M			(REG32_VAL(SPI_WR_EN_CTR_ADDR))
#define	SPI_ER_CTR_M			(REG32_VAL(SPI_ER_CTR_ADDR))
#define	SPI_ER_START_ADDR_M		(REG32_VAL(SPI_ER_START_ADDR_ADDR))
#define	SPI_ERROR_STATUS_M		(REG32_VAL(SPI_ERROR_STATUS_ADDR))
#define	SPI_MEM_0_SR_ACC_M		(REG32_VAL(SPI_MEM_0_SR_ACC_ADDR))
#define	SPI_MEM_1_SR_ACC_M		(REG32_VAL(SPI_MEM_1_SR_ACC_ADDR))
#define	SPI_PDWN_CTR_0_M		(REG32_VAL(SPI_PDWN_CTR_0_ADDR))
#define	SPI_PDWN_CTR_1_M		(REG32_VAL(SPI_PDWN_CTR_1_ADDR))
#define	SPI_PROG_CMD_CTR_M		(REG32_VAL(SPI_PROG_CMD_CTR_ADDR))
#define	SPI_USER_CMD_VAL_M		(REG32_VAL(SPI_USER_CMD_VAL_ADDR))
#define	SPI_PROG_CMD_WBF_M		(REG32_VAL(SPI_PROG_CMD_WBF_ADDR))
#define	SPI_PROG_CMD_RBF_M		(REG32_VAL(SPI_PROG_CMD_RBF_ADDR))

#endif	/* __WMT_SF1_H__ */




