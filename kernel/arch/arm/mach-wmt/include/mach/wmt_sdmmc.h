/*++
linux/include/asm-arm/arch-wmt/wmt_sdmmc.h

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
#error "You must include hardware.h, not wmt_sdmmc.h"
#endif

#ifndef	__WMT_SDMMC_H
#define	__WMT_SDMMC_H

#ifdef	__SDMMC_BASE
#error "__SDMMC_BASE has already been defined in another file."
#endif
#ifdef	SD_SDIO_MMC_BASE_ADDR		/* from vt8500_mmap.h */
#define	__SDMMC_BASE	SD0_SDIO_MMC_BASE_ADDR
#else
#ifdef ARL_EXTERNAL_SDHOST
#define	__SDMMC_BASE	0xC0000000	/* 64K */
#else
#define	__SDMMC_BASE	0xC0000000	/* 64K */
#endif
#endif


#define		MEM8(addr)		(*(volatile char *)(addr))
#define		MEM32(addr)		(MEM16(addr+2)<<16 | MEM16(addr))
#define		MEM16(addr)		(MEM8(addr+1)<<8 | MEM8(addr))

/*
 *  SD/SDIO/MMC Host Control Register Offset
 */
#define		_CTLR		0x00
#define		_CMDI		0X01
#define		_REST		0X02
#define		_CMDA3		0X03
#define		_CMDA2		0X04
#define		_CMDA1		0X05
#define		_CMDA0		0X06
#define		_BUSM		0X07
#define		_BLKL1		0X08
#define		_BLKL0		0X09
#define		_BLKC1		0X0A
#define		_BLKC0		0X0B
#define		_RESR		0X0C
#define		_DATR		0X0D
#define		_IMR0		0X0E
#define		_IMR1		0X0F
#define		_STR0		0X10
#define		_STR1		0X11
#define		_STR2		0X12
#define		_STR3		0X13
#define		_RTOR		0X14
#define		_DTOR2		0X15
#define		_DTOR1		0X16
#define		_DTOR0		0X17
#define		_CKDR		0X18
#define		_DMAC		0X19

/*
 * Address constant for each register.
 */
#define	CTLR_ADDR	(__SDMMC_BASE+_CTLR)
#define CMDI_ADDR	(__SDMMC_BASE+_CMDI)
#define REST_ADDR	(__SDMMC_BASE+_REST)
#define CMDA3_ADDR	(__SDMMC_BASE+_CMDA3)
#define CMDA2_ADDR	(__SDMMC_BASE+_CMDA2)
#define CMDA1_ADDR	(__SDMMC_BASE+_CMDA1)
#define CMDA0_ADDR	(__SDMMC_BASE+_CMDA0)
#define BUSM_ADDR	(__SDMMC_BASE+_BUSM)
#define BLKL1_ADDR	(__SDMMC_BASE+_BLKL1)
#define BLKL0_ADDR	(__SDMMC_BASE+_BLKL0)
#define BLKC1_ADDR	(__SDMMC_BASE+_BLKC1)
#define BLKC0_ADDR	(__SDMMC_BASE+_BLKC0)
#define RESR_ADDR	(__SDMMC_BASE+_RESR)
#define DATR_ADDR	(__SDMMC_BASE+_DATR)
#define IMR0_ADDR	(__SDMMC_BASE+_IMR0)
#define IMR1_ADDR	(__SDMMC_BASE+_IMR1)
#define STR0_ADDR	(__SDMMC_BASE+_STR0)
#define STR1_ADDR	(__SDMMC_BASE+_STR1)
#define STR2_ADDR	(__SDMMC_BASE+_STR2)
#define STR3_ADDR	(__SDMMC_BASE+_STR3)
#define RTOR_ADDR	(__SDMMC_BASE+_RTOR)
#define DTOR2_ADDR	(__SDMMC_BASE+_DTOR2)
#define DTOR1_ADDR	(__SDMMC_BASE+_DTOR1)
#define DTOR0_ADDR	(__SDMMC_BASE+_DTOR0)
#define CKDR_ADDR	(__SDMMC_BASE+_CKDR)
#define DMAC_ADDR	(__SDMMC_BASE+_DMAC)

/*
 * Register pointer.
 */
#define CTLR_REG	(REG8_PTR(CTLR_ADDR))
#define CMDI_REG	(REG8_PTR(CMDI_ADDR))
#define REST_REG	(REG8_PTR(REST_ADDR))
#define CMDA3_REG	(REG8_PTR(CMDA3_ADDR))
#define CMDA2_REG	(REG8_PTR(CMDA2_ADDR))
#define CMDA1_REG	(REG8_PTR(CMDA1_ADDR))
#define CMDA0_REG	(REG8_PTR(CMDA0_ADDR))
#define BUSM_REG	(REG8_PTR(BUSM_ADDR))
#define BLKL1_REG	(REG8_PTR(BLKL1_ADDR))
#define BLKL0_REG	(REG8_PTR(BLKL0_ADDR))
#define BLKC1_REG	(REG8_PTR(BLKC1_ADDR))
#define BLKC0_REG	(REG8_PTR(BLKC0_ADDR))
#define RESR_REG	(REG8_PTR(RESR_ADDR))
#define DATR_REG	(REG8_PTR(DATR_ADDR))
#define IMR0_REG	(REG8_PTR(IMR0_ADDR))
#define IMR1_REG	(REG8_PTR(IMR1_ADDR))
#define STR0_REG	(REG8_PTR(STR0_ADDR))
#define STR1_REG	(REG8_PTR(STR1_ADDR))
#define STR2_REG	(REG8_PTR(STR2_ADDR))
#define STR3_REG	(REG8_PTR(STR3_ADDR))
#define RTOR_REG	(REG8_PTR(RTOR_ADDR))
#define DTOR2_REG	(REG8_PTR(DTOR2_ADDR))
#define DTOR1_REG	(REG8_PTR(DTOR1_ADDR))
#define DTOR0_REG	(REG8_PTR(DTOR0_ADDR))
#define CKDR_REG	(REG8_PTR(CKDR_ADDR))
#define DMAC_REG	(REG8_PTR(DMAC_ADDR))

/*
 * SDH Command Index value
 */
#define CMD(x) (x)
#define ACMD(x) (x)


/*
 * SDH DATA STRUCTURES
 */
#define SECTOR_SIZE	512
#define TEST_FILE_SIZE	32768

/*
 * SDH Parameter Base Address Value
 *
 * Clark - I had to slow the AHB to 12 MHz to be able to communicate with the
 * xilinx part on the external AHB. This caused the ddr to not work properly.
 * I then changed the board to use the SDRAM and moved these storage locations
 * into SDRAM
 */
#define BLOCK_ORG_BASE		0x30000000
#define BLOCK_TMP_BASE		0x30002000

/*
#define BLOCK_TMP_BASE		0x03000000
#define FAT_TMP_BASE		0x03100000
#define FAT_ORG_BASE		0x03200000
#define ROOT_TMP_BASE		0x03300000
#define ROOT_ORG_BASE		0x03400000
#define TEST_FILE_BASE		0x03500000
#define TEST_FILE_TMP_BASE 	0x03600000
*/


/*
 * SD Host Register Bit Fields
 */

/* Control Register */
#define STR	        BIT0
#define SPISTP      BIT1
#define RXTX        BIT2
#define FFRST       BIT3
#define CT          (BIT4 | BIT5 | BIT6 | BIT7)

/* Command Index Register */

/* Response Type Register */
#define RT          (BIT0 | BIT1 | BIT2 | BIT3)
#define RY          BIT4

/* Command Argument Register 0,1,2,3 */

/* Bus Mode Register */
#define SPI         BIT0
#define WB          BIT1
#define RW          BIT2
#define SPICRC      BIT3
#define CST         BIT4
#define SPICS       BIT5
#define SDPWR       BIT6
#define SFTRST      BIT7


/* Block Length Register 0,1 */
#define BS_L        (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7)
#define BS_H        (BIT8 | BIT9 | BIT10)
#define CFD         BIT4
#define INTEN       BIT7


/* Block Count Register 0,1 */


/* Response Register */


/* Data Register */


/* Interrupt Mask Register 0 */
#define THIE        BIT0
#define TEIE        BIT1
#define TAIE        BIT2
#define RHIE        BIT3
#define RFIE        BIT4
#define RPIE        BIT5
#define CDIE        BIT6
#define SIIE        BIT7


/* Interrupt Mask Register 1 */
#define IOIE        BIT0
#define CRIE        BIT1
#define RAIE        BIT2
#define DDIE        BIT3
#define DTIE        BIT4
#define SCIE        BIT5
#define RCIE        BIT6
#define WCIE        BIT7


/* SD Status Register 0 */
#define TH          BIT0
#define TE          BIT1
#define TA          BIT2
#define RH          BIT3
#define RF          BIT4
#define PP          BIT5
#define SD_CD       BIT6
#define SI          BIT7


/* SD Status Register 1 */
#define SD_IO       BIT0
#define CR          BIT1
#define RA          BIT2
#define DD          BIT3
#define DT          BIT4
#define SC          BIT5
#define RC          BIT6
#define WC          BIT7


/* SD Status Register 2 */
#define CRCW        (BIT0 | BIT1 | BIT2)
#define CB          BIT4
#define DB          BIT5
#define CF          BIT6


/* SD Status Register 3 */
#define SPIE        BIT0
#define CCE         BIT1
#define CEF         BIT2
#define OOR         BIT3
#define SPIRE       BIT4
#define REIE        BIT5


/* Response Time Out Register */
#define RESTO       (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7)


/* Data Time Out Register 0,1,2 */
#define TMAX        (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7)


/* Clock Divisor Register */
#define DIV         (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7)


/* DMA Control Register */
#define DMA         BIT0
#define DEM         BIT1
#define LCNT        (BIT2 | BIT3)
#define DCNT        (BIT4 | BIT5 | BIT6 | BIT7)

/*
 * DMAC Registers
 */

#define SAR_OFFSET          0x10	/* size 32 */
#define DAR_OFFSET          0x10
#define TCR_OFFSET          0x10
#define CCR_OFFSET          0x10
#define CTR_OFFSET          0x01	/* size 8 */
#define CSR_OFFSET          0x01

#define DMA_SAR_0_VAL(ch)	REG32_VAL(DMA_SAR_CH0_0_ADDR+SAR_OFFSET*ch)
#define DMA_DAR_0_VAL(ch)	REG32_VAL(DMA_DAR_CH0_0_ADDR+DAR_OFFSET*ch)
#define DMA_TCR_0_VAL(ch)	REG32_VAL(DMA_TCR_CH0_0_ADDR+TCR_OFFSET*ch)
#define DMA_CCR_0_VAL(ch)	REG32_VAL(DMA_CCR_CH0_ADDR+CCR_OFFSET*ch)
#define DMA_CTR_VAL(ch)		REG8_VAL(DMA_CTR_CH0_ADDR+CTR_OFFSET*ch)
#define DMA_CSR_VAL(ch)		REG8_VAL(DMA_CSR_CH0_ADDR+CSR_OFFSET*ch)
#define DMA_SAR_1_VAL(ch)	REG32_VAL(DMA_SAR_CH0_1_ADDR+SAR_OFFSET*ch)
#define DMA_DAR_1_VAL(ch)	REG32_VAL(DMA_DAR_CH0_1_ADDR+DAR_OFFSET*ch)
#define DMA_TCR_1_VAL(ch)	REG32_VAL(DMA_TCR_CH0_1_ADDR+TCR_OFFSET*ch)

#define SD_BASE_ADDR        0xD800A000
#define SD_FALSE            0
#define SD_TRUE             1

#define	SD1_0               0x00
#define	SD1_1               0x01
#define	SD2_0               0x02



/*
 * SD command definition
 */

#define	GO_IDLE_STATE           0
#define	SEND_OP_COND            1   /*for MMC */
#define	ALL_SEND_CID            2
#define	SEND_RELATIVE_ADDR      3
#define	SET_RELATIVE_ADDR       3   /*for MMC1 */
#define	SET_DSR                 4
#define	SELECT_DESELECT_CARD    7
#define	SEND_IF_COND            8	/*for SD2.0  */
#define	SEND_EXT_CSD            8	/*for MMC  */
#define	SEND_CSD                9
#define	SEND_CID                10
/*#define READ_DAT_UNTIL_STOP     11  */
#define	STOP_TRANSMISSION       12
#define	SEND_STATUS             13
/*#define SET_BUS_WIDTH_REGISTER  14 */
#define	GO_INACTIVE_STATE       15
#define	SET_BLOCKLEN            16
#define	READ_SINGLE_BLOCK       17
#define	READ_MULTIPLE_BLOCK     18
/*#define WRITE_DAT_UNTIL_STOP    20  */
#define	WRITE_SINGLE_BLOCK      24
#define	WRITE_MULTIPLE_BLOCK    25
/*#define PROGRAM_CID             26 */
#define	PROGRAM_CSD             27
#define	SET_WRITE_PROT          28
#define	CLR_WRITE_PROT          29
#define	SEND_WRITE_PROT         30
#define	ERASE_START             32
#define	ERASE_END               33
/*#define UNTAG_SECTOR            34  */
/*#define TAG_ERASE_GROUP_START   35  */
/*#define TAG_ERASE_GROUP_END     36  */
/*#define UNTAG_ERASE_GROUP       37  */
#define	ERASE_GO                38
/*for SPI mode  */
#define	READ_OCR                58   /*for SPI */
#define	CRC_ON_OFF              59
#define	APP_CMD                 55
/*APP command */
#define	SET_BUS_WIDTH           6
#define	SD_STATUS               13
#define	SD_NUM_WR_BLOCKS        22
#define	SD_WR_BLK_ERASE_COUNT   23
#define	SD_APP_OP_COND          41
#define	SD_SET_CLR_CARD_DETECT  42
#define	SD_SEND_SCR             51


/* SD Response types  */
#define	R0          0       /* NONE response */
#define	R1          1       /* Basic response format  */
#define	R2          2       /* R2 response. Used by ALL_SEND_CID(CMD2),  */
							/* SEND_CID(CMD10) and SEND_CSD(CMD9) */
#define	R3          3       /* R3 response. Used by SEND_APP_OP_COND(ACMD41)  */
#define	R6          6       /* R6 response. Used by SEND_RELATIVE_ADDR(CMD3)  */
#define	R1b         9
/*response format in SPI mode */
#define	SPI_R1      7       /* Format R1. used in SPI mode  */
#define	SPI_R2      8       /* Format R2. used in SPI mode, SEND_STATUS, SD_STATUS  */
/*#define    	SPI_R3	9      // Format R3. used in SPI mode, READ_OCR      */


/*
 *  32bit status in Response
 */
#define		OUT_OF_RANGE_ERROR      0x80000000      /* Bit 31 */
#define		ADDRESS_ERROR           0x40000000      /* Bit 30 */
#define		BLOCK_LEN_ERROR         0x20000000      /* Bit 29 */
#define		ERASE_SEQ_ERROR         0x10000000      /* Bit 28 */
#define		ERASE_PARAM_ERROR       0x08000000      /* Bit 27 */
#define		WP_VIOLATION            0x04000000      /* Bit 26 */
#define		CARD_IS_LOCKED          0x02000000      /* Bit 25 */
#define		LOCK_UNLOCK_FAILED      0x01000000      /* Bit 24 */
#define		CMD_CRC_ERROR           0x00800000      /* Bit 23 */
#define		ILLEAGL_COMMAND         0x00400000      /* Bit 22 */
#define		CARD_ECC_FAILED         0x00200000      /* Bit 21 */
#define		CC_ERROR                0x00100000      /* Bit 20 */
#define		EERROR                  0x00080000      /* Bit 19 */
#define		UNDERRUN                0x00040000      /* Bit 18 */
#define		OVERRUN                 0x00020000      /* Bit 17 */
#define		CIDCSD_OVERWRITE        0x00010000      /* Bit 16 */
#define		WP_ERASE_SKIP           0x00008000      /* Bit 15 */
#define		CARD_ECC_DISABLED       0x00004000      /* Bit 14 */
#define		ERASE_RESET             0x00002000      /* Bit 13 */
#define		READY_FOR_DATA          0x00000100      /* Bit 8  */
#define		APPL_CMD                0x00000020      /* Bit 5  */
#define		AKE_SEQ_ERROR           0x00000008      /* Bit 3  */

/* current status bit12 ~ bit9 */
#define		IDLE                    0x00000000
#define		READY                   0x00000200
#define		IDENT                   0x00000400
#define		STBY                    0x00000600
#define		TRAN                    0x00000800
#define		DATA                    0x00000A00
#define		RCV                     0x00000C00
#define		PRG                     0x00000E00
#define		DIS                     0x00001000

/*bit definition for SD controller register */

/* 0x0 Control register */
#define		FIFO_RESET				0x08
#define		CMD_START				0x01
#define		CMD_READ				0x00
#define		CMD_WRITE				0x04
#define		CMD_SWRITE				0x10
#define		CMD_SREAD				0x20
#define		CMD_MWRITE				0x30
#define		CMD_MREAD				0x40

/*0x08 BusMode register */
#define		SOFT_RESET				0x80
#define		SD_POWER	  			0x40
#define		SPI_CS		  			0x20
#define		SD_OFF					0x10
#define		FOURBIT_MODE			0x02
#define 	SPI_MODE				0x01
#define		SD_MODE					0x00


/*0x0C BlkLen */
#define		INT_ENABLE				0x80
#define		DATA3_CD	  			0x40
#define		GPI_CD		  			0x20
#define		CD_POL_HIGH				0x10
#define		CRCERR_ABORT			0x08 /*abort multiple-blk-transfer when CRC-Err  */


/*0x24 IntMask0 */
#define DI_INT_EN             		0x80
#define CD_INT_EN             		0x40
#define BLK_TRAN_DONE_INT_EN  		0x20
#define MBLK_TRAN_DONE_INT_EN 		0x10

/*0x25 IntMask0 */
#define WCRC_ERR_INT_EN             0x80
#define RCRC_ERR_INT_EN             0x40
#define RESCRC_ERR_INT_EN           0x20
#define DATA_TOUT_INT_EN            0x10
#define MBLK_AUTO_STOP_INT_EN       0x08
#define CMD_RES_TOUT_INT_EN         0x04
#define CMD_RES_TRAN_DONE_INT_EN    0x02

/*0x28 Status0 register */
#define		DEVICE_INS				0x80
#define		CARD_DETECT				0x40
#define		BLK_DONE				0x20
#define		MBLK_DONE				0x10
#define		CD_GPI					0x08
#define		CD_DATA3				0x04
#define		Write_Protect			0x02


/*0x29 Status1 register */
#define		WCRC_ERR				0x80
#define		RCRC_ERR				0x40
#define		RSP_CRC_ERR				0x20
#define		DATA_TIMEOUT			0x10
#define		AUTOSTOP_DONE			0x08
#define		RSP_TIMEOUT				0x04
#define		CMDRSP_DONE				0x02
#define		SDIO_INT				0x01

/*0x2A Status2 register */
#define		DIS_FORCECLK			0x80
#define		DATARSP_BUSY			0x20
#define		CMD_RES_BUSY  			0x10

/*0x30 Clock register */
#define		Clk_375					0x00
#define		Clk_10					0x01
#define		Clk_12					0x02
#define		Clk_15					0x03
#define		Clk_20					0x04
#define		Clk_24					0x05
#define		Clk_30					0x06
#define		Clk_40					0x07

/*0x34 Extension Control register */
#define		ArgShift				0x02
#define		AutoStop				0x01

/*return sdstatus */
#define 	CMD_OK					0x00
#define		CMDRSP_TOUT				0x01
#define		RSP_CRCERR				0x02
#define 	APPCMD_FAIL				0x03
#define		DATA_TOUT				0x04
#define		WRITE_CRCERR			0x05
#define		READ_CRCERR				0x06
#define		TYPE_UNKNOWN			0x07
#define		NOCARD_INSERT			0x08
#define		POWER_FAIL				0x09
#define		READ_CID_ERR			0x0a
#define		READ_RCA_ERR			0x0b
#define		SET_RCA_ERR				0x0c
#define		READ_CSD_ERR			0x0d
#define		SELECT_CARD_ERR			0x0e
#define		READ_SCR_ERR			0x0f
#define		SET_BUSWIDTH_ERR		0x10
#define		SET_BLKLEN_ERR			0x11
#define		STOP_FAIL				0x12
#define		MBLK_FAIL				0x13
#define		TURE_T					0x14
#define		FALSE_F					0x15


/*
 * SD TPE DMA
 */

/*
 *   Refer AHB DMA Controller for TPE Peripherals
 */
#define SD_DMA_BASE_ADDR			0xD800A100


/*SD DMA channel configuration registers -- CCR */
#define SD_DMA_CCR_TR_SIZE_8                   0x00000000      /* [1:0] -- desired_transfer_size 8-bit */
#define SD_DMA_CCR_TR_SIZE_16                  0x00000001      /* [1:0] -- desired_transfer_size 16-bit */
#define SD_DMA_CCR_TR_SIZE_32                  0x00000002      /* [1:0] -- desired_transfer_size 32-bit */
#define SD_DMA_CCR_TRANSFER_SIZE_MASK          0x00000003
/* Reserved [3:2] */
#define SD_DMA_CCR_BURST_SINGLE                0x00000000      /* [5:4] -- burst_length single */
#define SD_DMA_CCR_BURST_INC4                  0x00000010      /* [5:4] -- burst_length INC4   */
#define SD_DMA_CCR_BURST_INC8                  0x00000020      /* [5:4] -- burst_length INC8   */
/* Reserved [7:6] */
#define SD_DMA_CCR_PROT_OP_FETCH               0x00000000      /* [11:08] */
#define SD_DMA_CCR_PROT_DATA_ACCESS            0x00000100
#define SD_DMA_CCR_PROT_USER_ACCESS            0x00000000
#define SD_DMA_CCR_PROT_PRIV_ACCESS            0x00000200
#define SD_DMA_CCR_PROT_NOT_BUF                0x00000000
#define SD_DMA_CCR_PROT_BUF                    0x00000400
#define SD_DMA_CCR_PROT_NOT_CACHE              0x00000000
#define SD_DMA_CCR_PROT_CACHE                  0x00000800
#define SD_DMA_CCR_SRC_NON_INC                 0x00000000      /* [12:12] -- Source address mode */
#define SD_DMA_CCR_SRC_INC                     0x00001000      /* [12:12] -- Source address mode */
#define SD_DMA_CCR_DES_INC                     0x00001000

/* Reserved [15:13] */
#define SD_DMA_CCR_SYS_TO_MEM                  0x00000000
#define SD_DMA_CCR_MEM_TO_SYS                  0x00010000      /* [16:16] -- SD DMA transfer direction */
/* Reserved [19:17] */
#define SD_DMA_CCR_TC_INT_EN                   0x00100000      /* [20:20] -- Terminal Count Inttrupt Enable */
#define SD_DMA_CCR_TR_COMPLETE_INT_EN          0x00200000      /* [21:21] -- Transfer Complete Inttrupt Enable */
#define SD_DMA_CCR_AHB_ERR_INT_EN              0x00400000      /* [22:22] -- AHB Bus Error Inttrupt Enable */
#define SD_DMA_CCR_FIFO_EMPTY_INT_EN           0x00800000      /* [23:23] -- FIFO Empty Inttrupt Enable */

#define SD_DMA_CCR_ALL_INT_EN                  0x00F00000
#define SD_DMA_CCR_ALL_INT_DIS                 0x00000000
/* Reserved [31:24] */

/*SD DMA channel control registers -- CTR */

/* Reserved [07:05] */
#define SD_DMA_CTR_SW_REQ_EN                      0x10            /* [04:04] */
#define SD_DMA_CTR_SW_REQ_DIS                     0x00

/* Reserved [03:01] */
#define SD_DMA_CTR_CH_EN                          0x01            /* [00:00] */
#define SD_DMA_CTR_CH_DIS                         0x00            /* [00:00] */

/*SD DMA channel status registers -- CSR */
#define SD_DMA_CSR_DMA_REQ                        0x80            /* [07:07] */
/* Reserved [06:04] */
#define SD_DMA_CSR_FIFO_EMPTY_INT                 0x08            /* [03:03] */
#define SD_DMA_CSR_FIFO_EMPTY_INT_WRITE_CLEAR     0x08            /* [03:03] */
#define SD_DMA_CSR_AHB_BUS_ERR                    0x04            /* [02:02] */
#define SD_DMA_CSR_AHB_BUS_ERR_WRITE_CLEAR        0x04            /* [02:02] */
/* For USB use [01:01] */
#define SD_DMA_CSR_TC                             0x01            /* [00:00] */
#define SD_DMA_CSR_TC_WRITE_CLEAR                 0x01

#define SD_DMA_CSR_ALL_SET_CLEAR                  0x0F

/*SD DMA global control registers -- GCR */

/* Reserved [31:25] */
#define SD_DMA_GCR_MANUAL_FLUSH_EN             0x01000000      /* [24:24] */
#define SD_DMA_GCR_MANUAL_FLUAH_DIS            0x00000000      /* [24:24] */
/* Reserved [23:17] */
#define SD_DMA_GCR_FLUSH_FIFO                  0x00010000      /* [16:16] */
#define SD_DMA_GCR__FLUSH_SELF_CLEAR           0x00000000      /* [16:16] */
/* Reserved [15:9] */
#define SD_DMA_GCR_GINT_EN                     0x00000100      /* [08:08] */
#define SD_DMA_GCR_GINT_DIS                    0x00000000      /* [08:08] */
/* Reserved [07:01] */
#define SD_DMA_GCR_GDMA_EN                     0x00000001      /* [00:00] */
#define SD_DMA_GCR_GDMA_DIS                    0x00000000      /* [00:00] */

/* SD DMA global status registers -- GSR */

/* Reserved [31:17] */
#define SD_DMA_GSR_FIFO_EMPTY                  0x00010000      /* [16:16] */
/* Reserved [15:9] */
#define SD_DMA_GSR_CH0_EN                      0x00000100      /* [08:08] */
/* Reserved [7:1] */
#define SD_DMA_GSR_CH0_AGGR_STATUS             0x00000001      /* [00:00] */

/* SD DMA global purpose registers -- GPR */
/* For USB use */

#endif	/* __WMT_SDMMC_H */




