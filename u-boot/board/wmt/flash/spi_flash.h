/*
 * Copyright (c) 2006
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * Copyright (c) 2008 WonderMedia Technologies, Inc.
 *
 * This program is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software Foundation,
 * either version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * WonderMedia Technologies, Inc.
 * 10F, 529, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C.
 */

#ifndef __SPI_FLASH_H_
#define __SPI_FLASH_H_

#include <asm/arch/common_def.h>	/* WMT common definitions and macros */
/* GPIO reg */
#define GPIO_BASE_ADDR 0xD8110000
#define GPIO_ENABLE_SF_REG  REG32_PTR(GPIO_BASE_ADDR+0x0)
#define GPIO_STRAPPING_REG  REG32_PTR(GPIO_BASE_ADDR+0x0100)

/* PMC module reg */
#define PMC_BASE_ADDR 0XD8130000
#define PMC_SF_CLK_REG  REG32_PTR(PMC_BASE_ADDR + 0x314)
#define PMC_PLLB_MULTIPLIER_REG  REG32_PTR(PMC_BASE_ADDR + 0x204)
#define PMC_PLLA_MULTIPLIER_REG  REG32_PTR(PMC_BASE_ADDR + 0x200)
#define PMC_PLLC_MULTIPLIER_REG  REG32_PTR(PMC_BASE_ADDR + 0x208)
#define PMC_PLLD_MULTIPLIER_REG  REG32_PTR(PMC_BASE_ADDR + 0x20C)

#define SF_BASE_ADDR 0xD8002000

typedef struct sfreg_s {
	unsigned long volatile     CHIP_SEL_0_CFG ;    /* 0xD8002000 */
	unsigned long volatile     Res1 ;              /* 0x04 */
	unsigned long volatile     CHIP_SEL_1_CFG ;    /* 0xD8002008 */
	unsigned long volatile     Res2[13] ;          /* 0x0C */
	unsigned long volatile     SPI_INTF_CFG ;      /* 0xD8002040 */
	unsigned long volatile     Res3[3] ;           /* 0x44 */
	unsigned long volatile     SPI_RD_WR_CTR ;     /* 0xD8002050 */
	unsigned long volatile     Res4[3] ;           /* 0x54 */
	unsigned long volatile     SPI_WR_EN_CTR ;     /* 0xD8002060 */
	unsigned long volatile     Res5[3] ;           /* 0x64 */
	unsigned long volatile     SPI_ER_CTR ;        /* 0xD8002070 */
	unsigned long volatile     SPI_ER_START_ADDR ; /* 0xD8002074 */
	unsigned long volatile     Res6[2] ;           /* 0x78 */
	unsigned long volatile     SPI_ERROR_STATUS ;  /* 0xD8002080 */
	unsigned long volatile     Res7[31] ;          /* 0x84 */
	unsigned long volatile     SPI_MEM_0_SR_ACC ;  /* 0xD8002100 */
	unsigned long volatile     Res8[3] ;           /* 0x104 */
	unsigned long volatile     SPI_MEM_1_SR_ACC ;  /* 0xD8002110 */
	unsigned long volatile     Res9[27] ;          /* 0x114 */
	unsigned long volatile     SPI_PDWN_CTR_0 ;    /* 0xD8002180 */
	unsigned long volatile     Res10[3] ;          /* 0x184 */
	unsigned long volatile     SPI_PDWN_CTR_1 ;    /* 0xD8002190 */
	unsigned long volatile     Res11[27] ;         /* 0x194 */
	unsigned long volatile     SPI_PROG_CMD_CTR ;  /* 0xD8002200 */
	unsigned long volatile     Res12[3] ;          /* 0x204 */
	unsigned long volatile     SPI_USER_CMD_VAL ;  /* 0xD8002210 */
	unsigned long volatile     Res13[59] ;         /* 0x214 */
	unsigned char volatile     SPI_PROG_CMD_WBF[64] ;  /* 0xD8002300 */
	unsigned long volatile     Res14[16] ;         /* 0x340 */
	unsigned char volatile     SPI_PROG_CMD_RBF[64] ;  /* 0xD8002380 */
} sfreg_t ;

/* Chip select 0-1 configuration register, 0x0 & 0x8 */
#define SF_START_ADDR          0xFF800000 /* [36:16] */
#define SF_MEM_SIZE_8M         0x00000800 /* [11:8] */

/* SPI interface configuration register, 0x40 */
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
/* SPI flash read/write control register, 0x50 */
#define SF_ID_RD               0x10 /* [4:4] */
#define SF_STATUS_RD           0x0  /* [4:4] */
#define SF_RD_SPD_FAST         0x1  /* [0:0] */
#define SF_RD_SPD_NOR          0x0  /* [0:0] */

/* SPI flash write enable control register, 0x60 */
#define SF_CS1_WR_EN           0x2 /* [1:1] */
#define SF_CS1_WR_DIS          0x0 /* [1:1] */
#define SF_CS0_WR_EN           0x1 /* [0:0] */
#define SF_CS0_WR_DIS          0x0 /* [0:0] */

/* SPI flash erase control register, 0x70 */
#define SF_SEC_ER_EN           0x8000 /* [15:15] */
#define SF_SEC_ER_DIS          0x0 /* [15:15] */
#define SF_CHIP_ER_EN          0x1 /* [0:0] */
#define SF_CHIP_ER_DIS         0x0 /* [0:0] */

/* SPI flash erase start address register, 0x74 */
#define SF_ER_START_ADDR       0x0 /* [31:16] */
#define CHIP_ER_CS1            0x2 /* [1:1] */
#define CHIP_ER_CS0            0x1 /* [0:0] */

/* SPI flash error status register, 0x80 */
#define SF_WR_PROT_ERR         0x20 /* [5:5] */
#define SF_MEM_REGION_ERR      0x10 /* [4:4] */
#define SF_PWR_DWN_ACC_ERR     0x8  /* [3:3] */
#define SF_PCMD_OP_ERR         0x4  /* [2:2] */
#define SF_PCMD_ACC_ERR        0x2  /* [1:1] */
#define SF_MASLOCK_ERR         0x1  /* [0:0] */
/* SPI power down control register, 0x180 & 0x190 */
#define PWR_DWN_EN             0x1 /* [0:0] */
#define PWR_DWN_DIS            0x0 /* [0:0] */
/* SPI programmable command mode control register, 0x200 */
#define SF_TX_DATA_SIZE        0x0 /* MACRO [30:24] */
#define SF_RX_DATA_SIZE        0x0 /* MACRO [22:16] */
#define SF_CMD_CS1             0x2 /* MACRO [1:1] */
#define SF_CMD_CS0             0x0 /* MACRO [1:1] */
#define SF_CMD_EN              0x1 /* [0:0] */
#define SF_CMD_DIS             0x0 /* [0:0] */
/* SPI user command value register, 0x210 */
#define SF_USR_WR_CMD          0x0 /* MACRO [23:16] */
#define SF_USR_RD_CMD          0x0 /* MACRO [7:0] */

unsigned long spi_flash_init(void);
int spi_flash_erase(flash_info_t *info, int s_first, int s_last);
int spi_write_buff(flash_info_t *info, unsigned char *src, unsigned long addr, unsigned long cnt);
void spi_flash_print_info(flash_info_t *info);
#ifdef CFG_FLASH_PROTECTION
int spi_flash_real_protect(flash_info_t *info, long sector, int prot);
#endif

#endif	/* __SPI_FLASH_H_ */
