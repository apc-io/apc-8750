/*++
drivers/mtd/devices/wmt_sf.h

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

#ifndef __WMT_SF_H__
#define __WMT_SF_H__

#define DRIVER_NAME "MTDSF"
#define MTDSF_TOTAL_SIZE (16 * 1024 * 1024)
#define MTDSF_ERASE_SIZE (64 * 1024)
/*#define MTDSF_PHY_ADDR (0xFF000000)*/
#define MTDSF_BOOT_PHY_ADDR (0xFF000000)
#define MTDSF_NOT_BOOT_PHY_ADDR (0xEF000000)
#define SPI_FLASH_TYPE  0
#define NAND_FLASH_TYPE 2
#define NOR_FLASH_TYPE  4
#define BOOT_TYPE_8BIT  0
#define BOOT_TYPE_16BIT 1

#define SF_CLOCK_EN	0x0800000

struct sfreg_t {
    unsigned long volatile     CHIP_SEL_0_CFG ;    /* 0xD8002000*/
    unsigned long volatile     Res1 ;              /* 0x04*/
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
    unsigned long volatile     Res10[3] ;           /* 0x184 */
    unsigned long volatile     SPI_PDWN_CTR_1 ;    /* 0xD8002190 */
    unsigned long volatile     Res11[27] ;         /* 0x194 */
    unsigned long volatile     SPI_PROG_CMD_CTR ;  /* 0xD8002200 */
    unsigned long volatile     Res12[3] ;          /* 0x204 */
    unsigned long volatile     SPI_USER_CMD_VAL ;  /* 0xD8002210 */
    unsigned long volatile     Res13[59] ;         /* 0x214 */
    unsigned char volatile     SPI_PROG_CMD_WBF[64] ;  /* 0xD8002300 */
    unsigned long volatile     Res14[16] ;         /* 0x340 */
    unsigned char volatile     SPI_PROG_CMD_RBF[64];  /* 0xD8002380 */
};

struct wmt_flash_info_t {
	unsigned int id;
	unsigned int phy;
	unsigned int val;
	unsigned int total;
};

/* SPI flash erase control register, 0x70 */
#define SF_SEC_ER_EN           0x8000 /* [15:15] */
#define SF_SEC_ER_DIS          0x0 /* [15:15] */
#define SF_CHIP_ER_EN          0x1 /* [0:0] */
#define SF_CHIP_ER_DIS          0x0 /* [0:0] */

#define FLASH_UNKNOW 0xFFFFFFFF

#define SF_BIT_WR_PROT_ERR         0x20 /* [5:5] */
#define SF_BIT_MEM_REGION_ERR      0x10 /* [4:4] */
#define SF_BIT_PWR_DWN_ACC_ERR     0x8 /* [3:3] */
#define SF_BIT_PCMD_OP_ERR         0x4 /* [2:2] */
#define SF_BIT_PCMD_ACC_ERR        0x2 /* [1:1] */
#define SF_BIT_MASLOCK_ERR         0x1 /* [0:0] */
#define BIT_SEQUENCE_ERROR	0x00300030
#define BIT_TIMEOUT		0x80000000

#define ERR_OK        0x0
#define ERR_TIMOUT        0x11
#define ERR_PROG_ERROR	0x22

#define EON_MANUFACT	0x1C
#define NUMONYX_MANUFACT	0x20
#define MXIC_MANUFACT	0xC2
#define SPANSION_MANUFACT	0x01
#define SST_MANUFACT	0xBF
#define WB_MANUFACT 0xEF
#define ATMEL_MANUF    0x1F
#define GD_MANUF    0xC8

/* EON */
#define EON_25P16_ID	0x2015 /* 2 MB */
#define EON_25P64_ID	0x2017 /* 8 MB */
#define EON_25Q64_ID	0x3017 /* 8 MB */
#define EON_25F40_ID	0x3113 /* 512 KB */
#define EON_25F16_ID	0x3115 /* 2 MB */

/* NUMONYX */
#define	NX_25P16_ID	0x2015 /* 2 MB */
#define	NX_25P64_ID	0x2017 /* 8 MB */

/* MXIC */
#define	MX_L512_ID	0x2010 /* 64 KB , 4KB*/
#define	MX_L1605D_ID	0x2015 /* 2 MB */
#define	MX_L3205D_ID	0x2016 /* 4 MB */
#define	MX_L6405D_ID	0x2017 /* 8 MB */
#define	MX_L1635D_ID	0x2415 /* 2 MB */
#define	MX_L3235D_ID	0x5E16 /* 4 MB */
#define	MX_L12805D_ID	0x2018 /* 16 MB */

/* SPANSION */
#define SPAN_FL016A_ID	0x0214 /* 2 MB */
#define SPAN_FL064A_ID	0x0216 /* 8 MB */

/* SST */
#define SST_VF016B_ID	0x2541 /* 2 MB */

/* WinBond */
#define WB_X16A_ID	0x3015	/* 2 MB */
#define WB_X32_ID	0x3016	/* 4 MB */
#define WB_X64_ID	0x3017	/* 8 MB */
#define WB_X64_25Q64_ID	0x4017	/* 8 MB */
#define WB_X128_ID	0x4018	/* 16 MB */

/* ATMEL */
#define AT_25DF041A_ID 0x4401 /* 512KB */

/* GD -Giga Device- */
#define GD_25Q40_ID			0x4013 /* 512KB */
#define GD_25Q128_ID		0x4018 /* 16MB */

/* ST M25P64 CMD */
#define SF_CMD_WREN      0x06
#define SF_CMD_WRDI      0x04
#define SF_CMD_RDID      0x9F
#define SF_CMD_RDSR      0x05
#define SF_CMD_WRSR      0x01
#define SF_CMD_READ      0x03
#define SF_CMD_FAST_READ 0x0B
#define SF_CMD_PP        0x02
#define SF_CMD_SE        0xD8
#define SF_CMD_BE        0xC7
#define SF_CMD_RES       0xAB

/* SPI Interface Configuration Register(0x40) */
#define SF_MANUAL_MODE 0x40

/* SPI Programmable Command Mode Control Register(0x200) */
#define SF_RUN_CMD   0x01

struct wm_sf_dev_t {
	unsigned int id;
	unsigned int size; /* KBytes */
};

#define SF_IDALL(x, y)	((x<<16)|y)

#endif /* __WMT_SF_H__ */
