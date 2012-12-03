/*++
linux/drivers/mmc/host/mmc_atsmb.h

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



#ifndef	__MMC_ATSMB_H
#define	__MMC_ATSMB_H

#define	MMC0_DRIVER_NAME "sdmmc"
#define	MMC1_DRIVER_NAME "sdmmc1"
#define	MMC2_DRIVER_NAME "sdmmc2"



#define	MMC_ERR_NONE	0	/* Only for Android kernel 2.6.29, in MVL5, this variable is defined in mmc.h */
#define	MMC_ERR_FAILED	4	/* Only for Android kernel 2.6.29, in MVL5, this variable is defined in mmc.h */
/***************Driver version****************/
#define	MMC_DRV_3357_A0_A3			0
#define	MMC_DRV_3357_A4_A7			1
#define	MMC_DRV_3358				2
#define	MMC_DRV_3359				3
#define	MMC_DRV_3400_A0				4
#define	MMC_DRV_3400_A1				5
#define	MMC_DRV_3426_A0				6
#define	MMC_DRV_3426_A1				7
#define	MMC_DRV_3426_A2				8
#define	MMC_DRV_3437_A0				9
#define	MMC_DRV_3437_A1				10
#define MMC_DRV_3429                11
#define	MMC_DRV_3451_A0				12
#define MMC_DRV_3465				13
#define MMC_DRV_3445                14
/**************	INT	and	Base address*****************/
/*special	settings for each	one*/

#define	MMC_ATSMB0_BASE				SD0_SDIO_MMC_BASE_ADDR
#define	MMC_ATMSB0_END				(SD0_SDIO_MMC_BASE_ADDR + 0x3FF)

#define	MMC_ATSMB1_BASE				SD1_SDIO_MMC_BASE_ADDR
#define	MMC_ATMSB1_END				(SD1_SDIO_MMC_BASE_ADDR + 0x3FF)

#define	MMC_ATSMB2_BASE				SD2_SDIO_MMC_BASE_ADDR
#define	MMC_ATMSB2_END				(SD2_SDIO_MMC_BASE_ADDR + 0x3FF)


/**************	INT	and	Base address*****************/

/* SD0 pin */
#define GPIO_SD0_CD				BIT4
#define	GPIO_SD0_Data			(BIT4 | BIT5 | BIT6 | BIT7)
#define	GPIO_SD0_WriteProtect	BIT3
#define	GPIO_SD0_Command		BIT2
#define	GPIO_SD0_Clock			BIT1
#define	GPIO_SD0_POWER			BIT5

/* SD1 pin */
#define	GPIO_SD1_Data			(BIT7 | BIT6 | BIT5 | BIT4 | BIT3 | BIT2 | BIT1 | BIT0)
#define	GPIO_SD1_Command		BIT1
#define	GPIO_SD1_Clock			BIT0
#define GPIO_SD1_CD				BIT4
#define	GPIO_SD1_WriteProtect	BIT3
#define	GPIO_SD1_POWER			BIT2
#define GPIO_SD1_RSTN			BIT1

/* SD2 pin */
#define	GPIO_SD2_Data			(BIT2 | BIT5 | BIT4 | BIT1)
#define	GPIO_SD2_Command		BIT6
#define	GPIO_SD2_Clock			BIT7
#define GPIO_SD2_CD				BIT0
#define	GPIO_SD2_WriteProtect	BIT3
#define	GPIO_SD2_POWER			BIT1


#define	SD0_PIN		(GPIO_SD0_Clock | GPIO_SD0_Command | GPIO_SD0_WriteProtect | GPIO_SD0_Data)

/* PIN share switch */
#define GPIO_SD0_PinShare		BIT8
#define GPIO_SD1_PinShare		BIT1
#define GPIO_SD2_PinShare		BIT6


#define SD0_CARD_PWR	BIT1
#define SD1_CARD_PWR	BIT2
#define SD2_CARD_PWR	BIT3


/* IO Deive Strength and Slew Rate*/
#define SD0_IO_Slew_Rate BIT1
#define SD0_IO_Drive_Strength BIT0



/*common settings*/

#define	NR_SG	 128

/*=============================================*/
/* WMT ATSMB Register	Offset*/
/*=============================================*/
#define	_CTL		0x00	/* 1 byte*/
#define	_CMD_IDX	0x01	/* 1 byte*/
#define	_RSP_TYPE	0x02	/* 1 byte*/

#define	_CMD_ARG	0x04	/* 4 bytes*/
#define	_BUS_MODE	0x08	/* 1 byte*/

#define	_BLK_LEN	0x0c	/* 2 bytes*/
#define	_BLK_CNT	0x0e	/* 2 bytes*/
#define	_RSP_0		0x10	/* 1 bytes*/
#define	_RSP_1		0x11	/* 1 bytes*/
#define	_RSP_2		0x12	/* 1 bytes*/
#define	_RSP_3		0x13	/* 1 bytes*/
#define	_RSP_4		0x14	/* 1 bytes*/
#define	_RSP_5		0x15	/* 1 bytes*/
#define	_RSP_6		0x16	/* 1 bytes*/
#define	_RSP_7		0x17	/* 1 bytes*/
#define	_RSP_8		0x18	/* 1 bytes*/
#define	_RSP_9		0x19	/* 1 bytes*/
#define	_RSP_10		0x1a	/* 1 bytes*/
#define	_RSP_11		0x1b	/* 1 bytes*/
#define	_RSP_12		0x1c	/* 1 bytes*/
#define	_RSP_13		0x1d	/* 1 bytes*/
#define	_RSP_14		0x1e	/* 1 bytes*/
#define	_RSP_15		0x1f	/* 1 bytes*/

#define	_CURBLK_CNT	0x20	/* 4 bytes*/
#define	_INT_MASK_0	0x24	/* 1 byte*/
#define	_INT_MASK_1	0x25	/* 1 byte*/

#define	_SD_STS_0	0x28	/* 1 byte*/
#define	_SD_STS_1	0x29	/* 1 byte*/
#define	_SD_STS_2	0x2a	/* 1 byte*/
#define	_SD_STS_3	0x2b	/* 1 byte*/
#define	_RSP_TOUT	0x2c	/* 1 byte*/

#define	_CLK_SEL	0x30	/* 1 byte*/	/*Not	used*/

#define	_EXT_CTL	0x34	/* 1 byte*/

#define	_SHDW_BLKLEN 0x38	/* 2 bytes*/

#define	_TIMER_VAL	0x3c	/* 2 bytes*/


#define	_PDMA_GCR		0x100
#define	_PDMA_IER		0x104
#define	_PDMA_ISR		0x108
#define	_PDMA_DESPR		0x10C
#define	_PDMA_RBR		0x110
#define	_PDMA_DAR		0x114
#define	_PDMA_BAR		0x118
#define	_PDMA_CPR		0x11C
#define	_PDMA_CCR		0X120


/*=========================================*/
/* SD0 PDMA	related	Registers	           */
/*=========================================*/
#define	ATSMB0_PDMA_GCR		(REG32_PTR(_PDMA_GCR + MMC_ATSMB0_BASE))
#define	ATSMB0_PDMA_IER		(REG32_PTR(_PDMA_IER + MMC_ATSMB0_BASE))
#define	ATSMB0_PDMA_ISR		(REG32_PTR(_PDMA_ISR + MMC_ATSMB0_BASE))
#define	ATSMB0_PDMA_DESPR	(REG32_PTR(_PDMA_DESPR + MMC_ATSMB0_BASE))
#define	ATSMB0_PDMA_RBR		(REG32_PTR(_PDMA_RBR + MMC_ATSMB0_BASE))
#define	ATSMB0_PDMA_DAR		(REG32_PTR(_PDMA_DAR + MMC_ATSMB0_BASE))
#define	ATSMB0_PDMA_BAR		(REG32_PTR(_PDMA_BAR + MMC_ATSMB0_BASE))
#define	ATSMB0_PDMA_CPR		(REG32_PTR(_PDMA_CPR + MMC_ATSMB0_BASE))
#define	ATSMB0_PDMA_CCR		(REG32_PTR(_PDMA_CCR + MMC_ATSMB0_BASE))


/*=========================================*/
/* SD0 Register	pointer.                   */
/*=========================================*/
#define	ATSMB0_CTL			(REG8_PTR(_CTL + MMC_ATSMB0_BASE))
#define	ATSMB0_CMD_IDX		(REG8_PTR(_CMD_IDX + MMC_ATSMB0_BASE))
#define	ATSMB0_RSP_TYPE		(REG8_PTR(_RSP_TYPE	+ MMC_ATSMB0_BASE))
#define	ATSMB0_CMD_ARG		(REG32_PTR(_CMD_ARG	+ MMC_ATSMB0_BASE))
#define	ATSMB0_BUS_MODE		(REG8_PTR(_BUS_MODE	+ MMC_ATSMB0_BASE))
#define	ATSMB0_BLK_LEN		(REG16_PTR(_BLK_LEN	+ MMC_ATSMB0_BASE))
#define	ATSMB0_BLK_CNT		(REG16_PTR(_BLK_CNT	+ MMC_ATSMB0_BASE))
#define	ATSMB0_RSP_0		(REG8_PTR(_RSP_0 + MMC_ATSMB0_BASE))
#define	ATSMB0_RSP_1		(REG8_PTR(_RSP_1 + MMC_ATSMB0_BASE))
#define	ATSMB0_RSP_2		(REG8_PTR(_RSP_2 + MMC_ATSMB0_BASE))
#define	ATSMB0_RSP_3		(REG8_PTR(_RSP_3 + MMC_ATSMB0_BASE))
#define	ATSMB0_RSP_4		(REG8_PTR(_RSP_4 + MMC_ATSMB0_BASE))
#define	ATSMB0_RSP_5		(REG8_PTR(_RSP_5 + MMC_ATSMB0_BASE))
#define	ATSMB0_RSP_6		(REG8_PTR(_RSP_6 + MMC_ATSMB0_BASE))
#define	ATSMB0_RSP_7		(REG8_PTR(_RSP_7 + MMC_ATSMB0_BASE))
#define	ATSMB0_RSP_8		(REG8_PTR(_RSP_8 + MMC_ATSMB0_BASE))
#define	ATSMB0_RSP_9		(REG8_PTR(_RSP_9 + MMC_ATSMB0_BASE))
#define	ATSMB0_RSP_10		(REG8_PTR(_RSP_10 +	MMC_ATSMB0_BASE))
#define	ATSMB0_RSP_11		(REG8_PTR(_RSP_11 + MMC_ATSMB0_BASE))
#define	ATSMB0_RSP_12		(REG8_PTR(_RSP_12 + MMC_ATSMB0_BASE))
#define	ATSMB0_RSP_13		(REG8_PTR(_RSP_13 +	MMC_ATSMB0_BASE))
#define	ATSMB0_RSP_14		(REG8_PTR(_RSP_14 +	MMC_ATSMB0_BASE))
#define	ATSMB0_RSP_15		(REG8_PTR(_RSP_15 +	MMC_ATSMB0_BASE))

#define	ATSMB0_CURBLK_CNT	(REG32_PTR(_CURBLK_CNT + MMC_ATSMB0_BASE))
#define	ATSMB0_INT_MASK_0	(REG8_PTR(_INT_MASK_0 +	MMC_ATSMB0_BASE))
#define	ATSMB0_INT_MASK_1	(REG8_PTR(_INT_MASK_1 +	MMC_ATSMB0_BASE))
#define	ATSMB0_SD_STS_0		(REG8_PTR(_SD_STS_0	+ MMC_ATSMB0_BASE))
#define	ATSMB0_SD_STS_1		(REG8_PTR(_SD_STS_1	+ MMC_ATSMB0_BASE))
#define	ATSMB0_SD_STS_2		(REG8_PTR(_SD_STS_2	+ MMC_ATSMB0_BASE))
#define	ATSMB0_SD_STS_3		(REG8_PTR(_SD_STS_3	+ MMC_ATSMB0_BASE))

#define	ATSMB0_RSP_TOUT		(REG8_PTR(_RSP_TOUT	+ MMC_ATSMB0_BASE))
#define	ATSMB0_CLK_SEL		(REG8_PTR(_CLK_SEL + MMC_ATSMB0_BASE))
#define	ATSMB0_EXT_CTL		(REG8_PTR(_EXT_CTL + MMC_ATSMB0_BASE))

#define	ATSMB0_SHDW_BLKLEN	(REG16_PTR(_SHDW_BLKLEN	+ MMC_ATSMB0_BASE))
#define	ATSMB0_TIMER_VAL	(REG16_PTR(_TIMER_VAL + MMC_ATSMB0_BASE))

//#define	ATSMB0_INT			(REG8_PTR(0xd8140056))




/*=========================================*/
/* SD1 PDMA	related	Registers	           */
/*=========================================*/
#define	ATSMB1_PDMA_GCR		(REG32_PTR(_PDMA_GCR + MMC_ATSMB1_BASE))
#define	ATSMB1_PDMA_IER		(REG32_PTR(_PDMA_IER + MMC_ATSMB1_BASE))
#define	ATSMB1_PDMA_ISR		(REG32_PTR(_PDMA_ISR + MMC_ATSMB1_BASE))
#define	ATSMB1_PDMA_DESPR	(REG32_PTR(_PDMA_DESPR + MMC_ATSMB1_BASE))
#define	ATSMB1_PDMA_RBR		(REG32_PTR(_PDMA_RBR + MMC_ATSMB1_BASE))
#define	ATSMB1_PDMA_DAR		(REG32_PTR(_PDMA_DAR + MMC_ATSMB1_BASE))
#define	ATSMB1_PDMA_BAR		(REG32_PTR(_PDMA_BAR + MMC_ATSMB1_BASE))
#define	ATSMB1_PDMA_CPR		(REG32_PTR(_PDMA_CPR + MMC_ATSMB1_BASE))
#define	ATSMB1_PDMA_CCR		(REG32_PTR(_PDMA_CCR + MMC_ATSMB1_BASE))


/*=========================================*/
/* SD1 Register	pointer.                   */
/*=========================================*/
#define	ATSMB1_CTL			(REG8_PTR(_CTL + MMC_ATSMB1_BASE))
#define	ATSMB1_CMD_IDX		(REG8_PTR(_CMD_IDX + MMC_ATSMB1_BASE))
#define	ATSMB1_RSP_TYPE		(REG8_PTR(_RSP_TYPE	+ MMC_ATSMB1_BASE))
#define	ATSMB1_CMD_ARG		(REG32_PTR(_CMD_ARG	+ MMC_ATSMB1_BASE))
#define	ATSMB1_BUS_MODE		(REG8_PTR(_BUS_MODE	+ MMC_ATSMB1_BASE))
#define	ATSMB1_BLK_LEN		(REG16_PTR(_BLK_LEN	+ MMC_ATSMB1_BASE))
#define	ATSMB1_BLK_CNT		(REG16_PTR(_BLK_CNT	+ MMC_ATSMB1_BASE))
#define	ATSMB1_RSP_0		(REG8_PTR(_RSP_0 + MMC_ATSMB1_BASE))
#define	ATSMB1_RSP_1		(REG8_PTR(_RSP_1 + MMC_ATSMB1_BASE))
#define	ATSMB1_RSP_2		(REG8_PTR(_RSP_2 + MMC_ATSMB1_BASE))
#define	ATSMB1_RSP_3		(REG8_PTR(_RSP_3 + MMC_ATSMB1_BASE))
#define	ATSMB1_RSP_4		(REG8_PTR(_RSP_4 + MMC_ATSMB1_BASE))
#define	ATSMB1_RSP_5		(REG8_PTR(_RSP_5 + MMC_ATSMB1_BASE))
#define	ATSMB1_RSP_6		(REG8_PTR(_RSP_6 + MMC_ATSMB1_BASE))
#define	ATSMB1_RSP_7		(REG8_PTR(_RSP_7 + MMC_ATSMB1_BASE))
#define	ATSMB1_RSP_8		(REG8_PTR(_RSP_8 + MMC_ATSMB1_BASE))
#define	ATSMB1_RSP_9		(REG8_PTR(_RSP_9 + MMC_ATSMB1_BASE))
#define	ATSMB1_RSP_10		(REG8_PTR(_RSP_10 + MMC_ATSMB1_BASE))
#define	ATSMB1_RSP_11		(REG8_PTR(_RSP_11 + MMC_ATSMB1_BASE))
#define	ATSMB1_RSP_12		(REG8_PTR(_RSP_12 + MMC_ATSMB1_BASE))
#define	ATSMB1_RSP_13		(REG8_PTR(_RSP_13 + MMC_ATSMB1_BASE))
#define	ATSMB1_RSP_14		(REG8_PTR(_RSP_14 + MMC_ATSMB1_BASE))
#define	ATSMB1_RSP_15		(REG8_PTR(_RSP_15 + MMC_ATSMB1_BASE))

#define	ATSMB1_CURBLK_CNT	(REG32_PTR(_CURBLK_CNT + MMC_ATSMB1_BASE))
#define	ATSMB1_INT_MASK_0	(REG8_PTR(_INT_MASK_0 + MMC_ATSMB1_BASE))
#define	ATSMB1_INT_MASK_1	(REG8_PTR(_INT_MASK_1 + MMC_ATSMB1_BASE))
#define	ATSMB1_SD_STS_0		(REG8_PTR(_SD_STS_0	+ MMC_ATSMB1_BASE))
#define	ATSMB1_SD_STS_1		(REG8_PTR(_SD_STS_1	+ MMC_ATSMB1_BASE))
#define	ATSMB1_SD_STS_2		(REG8_PTR(_SD_STS_2	+ MMC_ATSMB1_BASE))
#define	ATSMB1_SD_STS_3		(REG8_PTR(_SD_STS_3	+ MMC_ATSMB1_BASE))

#define	ATSMB1_RSP_TOUT		(REG8_PTR(_RSP_TOUT	+ MMC_ATSMB1_BASE))
#define	ATSMB1_CLK_SEL		(REG8_PTR(_CLK_SEL + MMC_ATSMB1_BASE))
#define	ATSMB1_EXT_CTL		(REG8_PTR(_EXT_CTL + MMC_ATSMB1_BASE))

#define	ATSMB1_SHDW_BLKLEN	(REG16_PTR(_SHDW_BLKLEN	+ MMC_ATSMB1_BASE))
#define	ATSMB1_TIMER_VAL	(REG16_PTR(_TIMER_VAL + MMC_ATSMB1_BASE))



/*=========================================*/
/* SD2 PDMA	related	Registers	           */
/*=========================================*/
#define	ATSMB2_PDMA_GCR		(REG32_PTR(_PDMA_GCR + MMC_ATSMB2_BASE))
#define	ATSMB2_PDMA_IER		(REG32_PTR(_PDMA_IER + MMC_ATSMB2_BASE))
#define	ATSMB2_PDMA_ISR		(REG32_PTR(_PDMA_ISR + MMC_ATSMB2_BASE))
#define	ATSMB2_PDMA_DESPR	(REG32_PTR(_PDMA_DESPR + MMC_ATSMB2_BASE))
#define	ATSMB2_PDMA_RBR		(REG32_PTR(_PDMA_RBR + MMC_ATSMB2_BASE))
#define	ATSMB2_PDMA_DAR		(REG32_PTR(_PDMA_DAR + MMC_ATSMB2_BASE))
#define	ATSMB2_PDMA_BAR		(REG32_PTR(_PDMA_BAR + MMC_ATSMB2_BASE))
#define	ATSMB2_PDMA_CPR		(REG32_PTR(_PDMA_CPR + MMC_ATSMB2_BASE))
#define	ATSMB2_PDMA_CCR		(REG32_PTR(_PDMA_CCR + MMC_ATSMB2_BASE))


/*=========================================*/
/* SD2 Register	pointer.                   */
/*=========================================*/
#define	ATSMB2_CTL			(REG8_PTR(_CTL + MMC_ATSMB2_BASE))
#define	ATSMB2_CMD_IDX		(REG8_PTR(_CMD_IDX + MMC_ATSMB2_BASE))
#define	ATSMB2_RSP_TYPE		(REG8_PTR(_RSP_TYPE	+ MMC_ATSMB2_BASE))
#define	ATSMB2_CMD_ARG		(REG32_PTR(_CMD_ARG	+ MMC_ATSMB2_BASE))
#define	ATSMB2_BUS_MODE		(REG8_PTR(_BUS_MODE	+ MMC_ATSMB2_BASE))
#define	ATSMB2_BLK_LEN		(REG16_PTR(_BLK_LEN	+ MMC_ATSMB2_BASE))
#define	ATSMB2_BLK_CNT		(REG16_PTR(_BLK_CNT	+ MMC_ATSMB2_BASE))
#define	ATSMB2_RSP_0		(REG8_PTR(_RSP_0 + MMC_ATSMB2_BASE))
#define	ATSMB2_RSP_1		(REG8_PTR(_RSP_1 + MMC_ATSMB2_BASE))
#define	ATSMB2_RSP_2		(REG8_PTR(_RSP_2 + MMC_ATSMB2_BASE))
#define	ATSMB2_RSP_3		(REG8_PTR(_RSP_3 + MMC_ATSMB2_BASE))
#define	ATSMB2_RSP_4		(REG8_PTR(_RSP_4 + MMC_ATSMB2_BASE))
#define	ATSMB2_RSP_5		(REG8_PTR(_RSP_5 + MMC_ATSMB2_BASE))
#define	ATSMB2_RSP_6		(REG8_PTR(_RSP_6 + MMC_ATSMB2_BASE))
#define	ATSMB2_RSP_7		(REG8_PTR(_RSP_7 + MMC_ATSMB2_BASE))
#define	ATSMB2_RSP_8		(REG8_PTR(_RSP_8 + MMC_ATSMB2_BASE))
#define	ATSMB2_RSP_9		(REG8_PTR(_RSP_9 + MMC_ATSMB2_BASE))
#define	ATSMB2_RSP_10		(REG8_PTR(_RSP_10 + MMC_ATSMB2_BASE))
#define	ATSMB2_RSP_11		(REG8_PTR(_RSP_11 + MMC_ATSMB2_BASE))
#define	ATSMB2_RSP_12		(REG8_PTR(_RSP_12 + MMC_ATSMB2_BASE))
#define	ATSMB2_RSP_13		(REG8_PTR(_RSP_13 + MMC_ATSMB2_BASE))
#define	ATSMB2_RSP_14		(REG8_PTR(_RSP_14 + MMC_ATSMB2_BASE))
#define	ATSMB2_RSP_15		(REG8_PTR(_RSP_15 + MMC_ATSMB2_BASE))

#define	ATSMB2_CURBLK_CNT	(REG32_PTR(_CURBLK_CNT + MMC_ATSMB2_BASE))
#define	ATSMB2_INT_MASK_0	(REG8_PTR(_INT_MASK_0 + MMC_ATSMB2_BASE))
#define	ATSMB2_INT_MASK_1	(REG8_PTR(_INT_MASK_1 + MMC_ATSMB2_BASE))
#define	ATSMB2_SD_STS_0		(REG8_PTR(_SD_STS_0	+ MMC_ATSMB2_BASE))
#define	ATSMB2_SD_STS_1		(REG8_PTR(_SD_STS_1	+ MMC_ATSMB2_BASE))
#define	ATSMB2_SD_STS_2		(REG8_PTR(_SD_STS_2	+ MMC_ATSMB2_BASE))
#define	ATSMB2_SD_STS_3		(REG8_PTR(_SD_STS_3	+ MMC_ATSMB2_BASE))

#define	ATSMB2_RSP_TOUT		(REG8_PTR(_RSP_TOUT	+ MMC_ATSMB2_BASE))
#define	ATSMB2_CLK_SEL		(REG8_PTR(_CLK_SEL + MMC_ATSMB2_BASE))
#define	ATSMB2_EXT_CTL		(REG8_PTR(_EXT_CTL + MMC_ATSMB2_BASE))

#define	ATSMB2_SHDW_BLKLEN	(REG16_PTR(_SHDW_BLKLEN	+ MMC_ATSMB2_BASE))
#define	ATSMB2_TIMER_VAL	(REG16_PTR(_TIMER_VAL + MMC_ATSMB2_BASE))


/*=========================================*/
/* Register	usage	const*/
/*=========================================*/
/*================================================================================*/
/**/
/* SD	Host Register	Bit	Fields*/
/**/
/*================================================================================*/
/**/
/* Control Register*/
#define	ATSMB_START		0x1
#define	ATSMB_SPISTP	BIT1
#define	ATSMB_RXTX		BIT2
#define	ATSMB_FFRST		BIT3
#define	ATSMB_CT		(BIT4|BIT5|BIT6|BIT7)
/**/
/* Command Index Register*/

/**/
/*Response Type	Register*/
#define	ATSMB_RT	(BIT0|BIT1|BIT2|BIT3)
#define	ATSMB_RY	BIT4

/**/
/* Command Argument	Register 0,1,2,3*/

/**/
/* Bus Mode	Register*/
#define	ATSMB_SPI			BIT0
#define	ATSMB_BUS_WIDTH_4	BIT1
#define	ATSMB_RW			BIT2
#define	ATSMB_SPICRC		BIT3
#define	ATSMB_CST			BIT4
#define	ATSMB_SPICS			BIT5
#define	ATSMB_SDPWR			BIT6
#define	ATSMB_SFTRST		BIT7

/**/
/* Block Length	Register 0,1*/
#define	ATSMB_BS_L			(BIT0|BIT1|BIT2|BIT3|BIT4|BIT5|BIT6|BIT7)
#define	ATSMB_BS_H			(BIT8|BIT9|BIT10)
#define	ATSMB_CFD			BIT4
#define	ATSMB_INTEN			BIT7

/**/
/* Block Count Register	0,1*/

/**/
/* Response	Register*/

/**/
/* Data	Register*/

/**/
/* Interrupt Mask	Register 0*/
#define	ATSMB_THIE					BIT0
#define	ATSMB_TEIE					BIT1
#define	ATSMB_TAIE					BIT2
#define	ATSMB_RHIE					BIT3
#define	ATSMB_MULTI_XFER_DONE_EN	BIT4
#define	ATSMB_BLOCK_XFER_DONE_EN	BIT5
#define	ATSMB_CDIE					BIT6
#define	ATSMB_DEVICE_INSERT_EN		BIT7

/**/
/* Interrupt Mask	Register 1*/
#define	ATSMB_SDIO_EN				BIT0
#define	ATSMB_RSP_DONE_EN			BIT1
#define	ATSMB_RSP_TIMEOUT_EN		BIT2
#define	ATSMB_AUTO_STOP_EN			BIT3
#define	ATSMB_DATA_TIMEOUT_EN		BIT4
#define	ATSMB_RSP_CRC_ERR_EN		BIT5
#define	ATSMB_READ_CRC_ERR_EN		BIT6
#define	ATSMB_WRITE_CRC_ERR_EN		BIT7

/**/
/* SD	Status Register	0*/
#define	ATSMB_TH					BIT0
#define	ATSMB_WRITE_PROTECT			BIT1	/* 1.	write	protected	is disabled.*/
#define	ATSMB_CARD_NOT_IN_SLOT		BIT2
#define	ATSMB_CARD_NOT_IN_SLOT_GPI	BIT3
#define	ATSMB_MULTI_XFER_DONE		BIT4
#define	ATSMB_BLOCK_XFER_DONE		BIT5
#define	ATSMB_SD_CD					BIT6 /*	what is	its	purpose?*/
#define	ATSMB_DEVICE_INSERT			BIT7

/**/
/* SD	Status Register	1*/
#define	ATSMB_SDIO_INT			BIT0
#define	ATSMB_RSP_DONE			BIT1
#define	ATSMB_RSP_TIMEOUT		BIT2
#define	ATSMB_AUTO_STOP			BIT3
#define	ATSMB_DATA_TIMEOUT		BIT4
#define	ATSMB_RSP_CRC_ERR		BIT5
#define	ATSMB_READ_CRC_ERR		BIT6
#define	ATSMB_WRITE_CRC_ERR		BIT7

/**/
/* SD	Status Register	2*/
#define	ATSMB_RSP_BUSY			BIT5
#define	ATSMB_CLK_FREEZ_STS		BIT6
#define	ATSMB_CLK_FREEZ_EN		BIT7


/* SD	Response types*/
#define	ATMSB_TYPE_R0	0		/* NONE	response*/
#define	ATMSB_TYPE_R1	1		/* Basic response	format*/
#define	ATMSB_TYPE_R2	2		/* R2	response.	Used by	ALL_SEND_CID(CMD2),*/
/* SEND_CID(CMD10) and SEND_CSD(CMD9)*/
#define	ATMSB_TYPE_R3	3		/* R3	response.	Used by	SEND_APP_OP_COND(ACMD41)*/
#define	ATMSB_TYPE_R6	6		/* R6	response.	Used by	SEND_RELATIVE_ADDR(CMD3)*/
#define	ATMSB_TYPE_R7	7		/* R6	response.	Used by	SEND_RELATIVE_ADDR(CMD3)*/
#define	ATMSB_TYPE_R1b 	9
#define	ATMSB_TYPE_R5	5
#define	ATMSB_TYPE_R4	4

/*
 *	SD PDMA
 */
struct SD_PDMA_REG {
	unsigned long	DMA_GCR;		/*	Rx00	*/
	unsigned long	DMA_IER;		/*	Rx04	*/
	unsigned long	DMA_ISR;		/*	Rx08	*/
	unsigned long	*DMA_DESPR;	/*	Rx0C	*/
	unsigned long	DMA_RBR;		/*	Rx10	*/
	unsigned long	DMA_DAR;		/*	Rx14	*/
	unsigned long	DMA_BAR;		/*	Rx18	*/
	unsigned long	DMA_CPR;		/*	Rx1C	*/
	unsigned long	DMA_CCR;		/*	RX20	*/
	unsigned long	resv[5];			/*	RX2C-3C	*/
};
/*
 *	SD PDMA	-	DMA_GCR	:	DMA	Global Control Register
 */
#define	SD_PDMA_GCR_DMA_EN	 	0x00000001	/* [0] --	DMA controller enable*/
#define	SD_PDMA_GCR_SOFTRESET 	0x00000100	/* [8] --	Software rest*/

/*
 *	SD PDMA	-	DMA_IER	:	DMA	Interrupt	Enable Register
 */
#define	SD_PDMA_IER_INT_EN 		0x00000001 /* [0] --DMA interrupt enable */
/*
 *	SD PDMA	-	DMA_ISR	:	DMA	Interrupt	Status Register
 */
#define	SD_PDMA_IER_INT_STS 	0x00000001 /* [0] --	DMA interrupt status */
/*
 *	SD PDMA	-	DMA_DESPR	:	DMA	Descriptor base	address	Pointer	Register
 */

/*
 *	SD PDMA	-	DMA_RBR	:	DMA	Residual Bytes Register
 */
#define	SD_PDMA_RBR_End		 	0x80000000	/* [0] --	DMA	interrupt	status */
#define	SD_PDMA_RBR_Format	 	0x40000000	/* [0] --	DMA	interrupt	status */
/*
 *	SD PDMA	-	DMA_DAR	:	DMA	Data Address Register
 */

/*
 *	SD PDMA	-	DMA_BAR	:	DMA	Rbanch Address Register
 */

/*
 *	SD PDMA	-	DMA_CPR	:	DMA	Command	Pointer	Register
 */

/*
 *	SD PDMA	-	DMA_CCR	:	DMAContext Control Register	for	Channel	0
 */
#define	SD_PDMA_READ					0x00
#define	SD_PDMA_WRITE					0x01
#define	SD_PDMA_CCR_RUN					0x00000080
#define	SD_PDMA_CCR_IF_to_peripheral	0x00000000
#define	SD_PDMA_CCR_peripheral_to_IF	0x00400000
#define	SD_PDMA_CCR_EvtCode				0x0000000f
#define	SD_PDMA_CCR_Evt_no_status		0x00000000
#define	SD_PDMA_CCR_Evt_ff_underrun		0x00000001
#define	SD_PDMA_CCR_Evt_ff_overrun		0x00000002
#define	SD_PDMA_CCR_Evt_desp_read		0x00000003
#define	SD_PDMA_CCR_Evt_data_rw			0x00000004
#define	SD_PDMA_CCR_Evt_early_end		0x00000005
#define	SD_PDMA_CCR_Evt_success			0x0000000f

/*
 *	PDMA Descriptor	short
 */
struct SD_PDMA_DESC_S{
	unsigned long volatile ReqCount : 16;		/* bit 0 -15 -Request count             */
	unsigned long volatile i : 1;				/* bit 16    -interrupt                 */
	unsigned long volatile reserve : 13;		/* bit 17-29 -reserved                  */
	unsigned long volatile format : 1;		/* bit 30    -The descriptor format     */
	unsigned long volatile end : 1;			/* bit 31    -End flag of descriptor list*/
	unsigned long volatile *DataBufferAddr;	/* bit 31    -Data Buffer address       */
} ;

/*
 *	PDMA Descriptor	long
 */
struct SD_PDMA_DESC_L{
	unsigned long	volatile ReqCount:16;		/* bit 0-15-Request count	*/
	unsigned long	volatile i:1;				/* bit 16	-interrupt*/
	unsigned long	volatile reserve:13;		/* bit 17-29-reserved*/
	unsigned long	volatile format:1;		/* bit 30-The descriptor format */
	unsigned long	volatile end:1;			/* bit 31	-End	flag of descriptor list*/
	unsigned long	volatile *DataBufferAddr;	/* bit 31-0	 -Data Buffer	address*/
	unsigned long	volatile *BranchAddr;		/* bit 31-2-Descriptor Branch address*/
	unsigned long	volatile reserve0;		/* bit 31-0-reserved*/
};


/**/
/* DMA usage const for Rx08[Config]*/
/**/
#define	DMA_CFG_WRITE			0x0
#define	DMA_CFG_READ			0x10000
/*
#define	DMA_CFG_8B				0x00
#define	DMA_CFG_16B				0x01
#define	DMA_CFG_32B				0x02

#define	DMA_CFG_SINGLE			0x00
#define	DMA_CFG_INC4			0x10
#define	DMA_CFG_INC8			0x20

#define	DMA_CFG_DATA			0x100
#define	DMA_CFG_PRIVILEDGE		0x200
#define	DMA_CFG_BUFFER			0x400
#define	DMA_CFG_CACHE			0x800

#define	DMA_CFG_INC				0x1000
#define	DMA_CFG_READ			0x10000
#define	DMA_CFG_WRITE			0x0

#define	DMA_CFG_NOINT			0x0
#define	DMA_CFG_ENTCINT			0x100000
#define	DMA_CFG_ENXFERINT		0x200000
#define	DMA_CFG_ENERRINT		0x400000
#define	DMA_CFG_ENFIFOINT		0x800000

//
// DMA usage const for RxC[Control]
//
#define	DMA_CTL_ENABLE			0x1
#define	DMA_CTL_SOFTREQ			0x10

//
// DMA usage const for Rx10[Status]
//
#define	DMA_STS_TC				0x1
#define	DMA_STS_AHB_ERROR		0x4
#define	DMA_STS_FIFO_EMPTY		0x8
#define	DMA_STS_BULK_COMPLETE	0x2
*/
/*=========================================*/
/* structure definition.*/
/*=========================================*/
struct atsmb_host {
	struct mmc_host	*mmc;
	spinlock_t lock;
	struct resource *res;
	void *base;
	int	 regular_irq;
	int	 dma_irq;
	/* 2009/01/13	janshiue-s */
	unsigned long	*DescVirAddr;
	dma_addr_t DescPhyAddr;
	unsigned int DescSize;
	unsigned long	*BufVirAddr;
	dma_addr_t BufPhyAddr;
	unsigned long	DmaIntMask;
	/* 2009/01/13	janshiue-s */
	struct mmc_request	*mrq;
	struct mmc_command	*cmd;
	struct mmc_data *data;
	u32 pwr;
	struct mmc_platform_data *plat;
	unsigned int sg_len;
	/* pio stuff */
	struct scatterlist *sg_ptr;
	unsigned int sg_off;
	unsigned int size;
	/* to	support	sg,	we need	little loops when	requesting data*/
	u32 opcode;
	/*this code	may different from the one in command. Eg. When we are accessing data*/
	unsigned char	soft_timeout;
	void *done_data;	/* completion	data */
	void (*done)(void *data);/*	completion function	*/
	int current_clock;

};

#define	MAX_DESC_NUM 256

enum WMT_SD_FUN {
	SD_MMC,
	SDIO_WIFI,
	SD_MAX_FUN
};

int	atsmb_init_short_desc(unsigned long *DescAddr, unsigned	int	ReqCount,
	unsigned long *BufferAddr, int End);
int	atsmb_init_long_desc(unsigned long *DescAddr, unsigned int ReqCount,
	unsigned long *BufferAddr,	unsigned long	 *BranchAddr, int End);

int	atsmb1_init_short_desc(unsigned long *DescAddr, unsigned	int	ReqCount,
	unsigned long *BufferAddr, int End);
int	atsmb1_init_long_desc(unsigned long *DescAddr, unsigned int ReqCount,
	unsigned long *BufferAddr,	unsigned long	 *BranchAddr, int End);


int	atsmb2_init_short_desc(unsigned long *DescAddr, unsigned	int	ReqCount,
	unsigned long *BufferAddr, int End);
int	atsmb2_init_long_desc(unsigned long *DescAddr, unsigned int ReqCount,
	unsigned long *BufferAddr,	unsigned long	 *BranchAddr, int End);

#endif	/* __MMC_ATSMB_H */
