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

#ifndef __SDC_H__
#define __SDC_H__

#define SD_FALSE  	0
#define SD_TRUE   	1

//SD command definition

#define		GO_IDLE_STATE           	0
#define		SEND_OP_COND            	1 		//for MMC
#define		ALL_SEND_CID            	2
#define		SEND_RELATIVE_ADDR      	3
#define		SET_RELATIVE_ADDR			3 		//for MMC1
#define		SET_DSR						4
#define		SELECT_DESELECT_CARD    	7
#define		SEND_IF_COND				8		//for SD2.0 2006/01/20 janshiue
#define		SEND_EXT_CSD				8		//for MMC
#define		SEND_CSD                	9
#define		SEND_CID                	10
//#define READ_DAT_UNTIL_STOP     11
#define		STOP_TRANSMISSION       	12
#define		SEND_STATUS             	13
//#define SET_BUS_WIDTH_REGISTER  14
#define		GO_INACTIVE_STATE       	15
#define		SET_BLOCKLEN            	16
#define		READ_SINGLE_BLOCK      		17
#define		READ_MULTIPLE_BLOCK     	18
//#define WRITE_DAT_UNTIL_STOP    20
#define		WRITE_SINGLE_BLOCK      	24
#define		WRITE_MULTIPLE_BLOCK    	25
//#define PROGRAM_CID             26
#define		PROGRAM_CSD            		27
#define		SET_WRITE_PROT         		28
#define		CLR_WRITE_PROT         		29
#define		SEND_WRITE_PROT        		30
#define		ERASE_START             	32
#define		ERASE_END					33
//#define UNTAG_SECTOR            34
//#define TAG_ERASE_GROUP_START   35
//#define TAG_ERASE_GROUP_END     36
//#define UNTAG_ERASE_GROUP       37
#define		ERASE_GO                	38
//for SPI mode
#define		READ_OCR                	58 //for SPI
#define		CRC_ON_OFF              	59
#define		APP_CMD                 	55
//APP command
#define		SET_BUS_WIDTH           	6
#define		SD_STATUS               	13
#define		SD_NUM_WR_BLOCKS        	22
#define		SD_WR_BLK_ERASE_COUNT   	23
#define		SD_APP_OP_COND          	41
#define		SD_SET_CLR_CARD_DETECT  	42
#define		SD_SEND_SCR             	51

#define		SWITCH_MMC					6
// SD Response types
#define		R0	0         	// NONE response
#define		R1 	1         	// Basic response format
#define		R2	2         	// R2 response. Used by ALL_SEND_CID(CMD2),
                    		// SEND_CID(CMD10) and SEND_CSD(CMD9)
#define    	R3	3         	// R3 response. Used by SEND_APP_OP_COND(ACMD41)
#define    	R6	6         	// R6 response. Used by SEND_RELATIVE_ADDR(CMD3)
#define    	R1b	9

//bit definition for SD controller register

// 0x0 Control register
#define		FIFO_RESET		0x08
#define		CMD_START		0x01
#define		CMD_READ		0x00
#define		CMD_WRITE		0x04
#define		CMD_SWRITE		0x10
#define		CMD_SREAD		0x20
#define		CMD_MWRITE		0x30
#define		CMD_MREAD		0x40

//0x08 BusMode register
#define		SOFT_RESET		0x80
#define		SD_POWER	  	0x40
#define		SPI_CS		  	0x20
#define		SD_OFF			0x10
#define		FOURBIT_MODE	0x02
#define   	ONEBIT_MODE   	0xFD
#define 	SPI_MODE		0x01
#define   	SD_MODE       	0x00
#define   	EIGHTBIT_MODE	0x04

//0x0C BlkLen
#define		INT_ENABLE		0x80
#define		DATA3_CD	  	0x40
#define		GPI_CD		  	0x20
#define   	CD_POL_HIGH   	0x10
#define		CRCERR_ABORT	0x08 //abort multiple-blk-transfer when CRC-Err


//0x24 IntMask0
#define 		DI_INT_EN             	0x80
#define 		CD_INT_EN				0x40
#define 		BLK_TRAN_DONE_INT_EN  	0x20
#define 		MBLK_TRAN_DONE_INT_EN 	0x10

//0x25 IntMask0
#define 		WCRC_ERR_INT_EN            	0x80
#define 		RCRC_ERR_INT_EN            	0x40
#define 		RESCRC_ERR_INT_EN          	0x20
#define 		DATA_TOUT_INT_EN           	0x10
#define 		MBLK_AUTO_STOP_INT_EN		0x08
#define 		CMD_RES_TOUT_INT_EN        	0x04
#define 		CMD_RES_TRAN_DONE_INT_EN	0x02

//0x28 Status0 register
#define		DEVICE_INS		0x80
#define		CARD_DETECT		0x40
#define		BLK_DONE		0x20
#define		MBLK_DONE		0x10
#define		CD_GPI			0x08
#define		CD_DATA3		0x04
#define		Write_Protect	0x02


//0x29 Status1 register
#define		WCRC_ERR		0x80
#define		RCRC_ERR		0x40
#define		RSP_CRC_ERR		0x20
#define		DATA_TIMEOUT	0x10
#define		AUTOSTOP_DONE	0x08
#define		RSP_TIMEOUT		0x04
#define		CMDRSP_DONE		0x02
#define		SDIO_INT		0x01

//0x2A Status2 register
#define		DIS_FORCECLK	0x80
#define		DATARSP_BUSY	0x20
#define   	CMD_RES_BUSY  	0x10

//0x30 Clock register
#define		Clk_375			0x00
#define		Clk_10			0x01
#define		Clk_12			0x02
#define		Clk_15			0x03
#define		Clk_20			0x04
#define		Clk_24			0x05
#define		Clk_30			0x06
#define		Clk_40			0x07
#define		Clk_48			0x08

#define SD_Clk_Auto				0
#define SD_Clk_15MHz			1
#define SD_Clk_20MHz			2
#define SD_Clk_25MHz			3
#define SD_Clk_33MHz			4
#define SD_Clk_40MHz			5
#define SD_Clk_44MHz			6
#define SD_Clk_50MHz			7
#define SD_Clk_400KHz			8


//0x34 Extension Control register
#define		ArgShift			0x02
#define		AutoStop			0x01

//return sdstatus
#define 	CMD_OK				0x00
#define		CMDRSP_TOUT			0x01
#define		RSP_CRCERR			0x02
#define 	APPCMD_FAIL			0x03
#define		DATA_TOUT			0x04
#define		WRITE_CRCERR		0x05
#define		READ_CRCERR			0x06
#define		TYPE_UNKNOWN		0x07
#define		NOCARD_INSERT		0x08
#define		POWER_FAIL			0x09
#define		READ_CID_ERR		0x0a
#define		READ_RCA_ERR		0x0b
#define		SET_RCA_ERR			0x0c
#define		READ_CSD_ERR		0x0d
#define		SELECT_CARD_ERR		0x0e
#define		READ_SCR_ERR		0x0f
#define		SET_BUSWIDTH_ERR	0x10
#define		SET_BLKLEN_ERR		0x11
#define		STOP_FAIL			0x12
#define		MBLK_FAIL			0x13
#define   	TURE_T       		0x14
#define   	FALSE_F      		0x15

#define	SWITCH_ERR      		0x14
#define	SD1_0					0x00
#define	SD1_1					0x01
#define	SD2_0					0x02

#define	MMC1_12					0x00
#define	MMC1_4					0x01
#define	MMC2_02					0x02
#define	MMC3_123				0x03
#define	MMC4_01					0x04

#define	HostCapacitySupport		0x40
#define	StandardCapacity		0x00
#define	HighCapacity			0x01

#define MMC_BLOCK_SIZE			512
#define CFG_MMC_BASE		0xF0000000

//SD DMA channel control registers -- CTR

/* Reserved [07:05] */
#define SD_DMA_CTR_SW_REQ_EN                     	0x10            /* [04:04] */
#define SD_DMA_CTR_SW_REQ_DIS                    	0x00

/* Reserved [03:01] */
#define SD_DMA_CTR_CH_EN                          	0x01            /* [00:00] */
#define SD_DMA_CTR_CH_DIS                         	0x00            /* [00:00] */

//SD DMA channel status registers -- CSR
#define SD_DMA_CSR_DMA_REQ                        	0x80            /* [07:07] */
/* Reserved [06:04] */
#define SD_DMA_CSR_FIFO_EMPTY_INT          			0x08            /* [03:03] */
#define SD_DMA_CSR_FIFO_EMPTY_INT_WRITE_CLEAR     	0x08            /* [03:03] */
#define SD_DMA_CSR_AHB_BUS_ERR                    	0x04            /* [02:02] */
#define SD_DMA_CSR_AHB_BUS_ERR_WRITE_CLEAR        	0x04            /* [02:02] */
/* For USB use [01:01] */
#define SD_DMA_CSR_TC                             	0x01            /* [00:00] */
#define SD_DMA_CSR_TC_WRITE_CLEAR            		0x01

#define SD_DMA_CSR_ALL_SET_CLEAR               		0x0F

//SD DMA global control registers -- GCR

/* Reserved [31:25] */
#define SD_DMA_GCR_MANUAL_FLUSH_EN            		0x01000000      /* [24:24] */
#define SD_DMA_GCR_MANUAL_FLUAH_DIS            		0x00000000      /* [24:24] */
/* Reserved [23:17] */
#define SD_DMA_GCR_FLUSH_FIFO                  		0x00010000      /* [16:16] */
#define SD_DMA_GCR__FLUSH_SELF_CLEAR           		0x00000000      /* [16:16] */
/* Reserved [15:9] */
#define SD_DMA_GCR_GINT_EN                     		0x00000100      /* [08:08] */
#define SD_DMA_GCR_GINT_DIS                    		0x00000000      /* [08:08] */
/* Reserved [07:01] */
#define SD_DMA_GCR_GDMA_EN                     		0x00000001      /* [00:00] */
#define SD_DMA_GCR_GDMA_DIS                    		0x00000000      /* [00:00] */

/*
 *	SD PDMA
 */
#define SD_PDMA_BASE_ADDR 0xD800A100
#define SD_DESC_BASE_ADDR 0x00d00000
struct _SD_PDMA_REG_ {
	volatile unsigned long DMA_GCR;		/*	Rx00	*/
	volatile unsigned long DMA_IER;		/*	Rx04	*/
	volatile unsigned long DMA_ISR;		/*	Rx08	*/
	volatile unsigned long *DMA_DESPR;	/*	Rx0C	*/
	volatile unsigned long DMA_RBR;		/*	Rx10	*/
	volatile unsigned long DMA_DAR;		/*	Rx14	*/
	volatile unsigned long DMA_BAR;		/*	Rx18	*/
	volatile unsigned long DMA_CPR;		/*	Rx1C	*/
	volatile unsigned long DMA_CCR;		/*	RX20	*/
	volatile unsigned long resv[5];    	/*	RX2C-3C	*/
} SD_PDMA_REG, *PSD_PDMA_REG;

/*
 *	SD PDMA - DMA_GCR : DMA Global Control Register
 */
#define SD_PDMA_GCR_DMA_EN			0x00000001      /* [0] -- DMA controller enable */
#define SD_PDMA_GCR_SOFTRESET		0x00000100      /* [8] -- Software rest */

/*
 *	SD PDMA - DMA_IER : DMA Interrupt Enable Register
 */
#define SD_PDMA_IER_INT_EN			0x00000001      /* [0] -- DMA interrupt enable */
/*
 *	SD PDMA - DMA_ISR : DMA Interrupt Status Register
 */
#define SD_PDMA_IER_INT_STS			0x00000001      /* [0] -- DMA interrupt status */
/*
 *	SD PDMA - DMA_DESPR : DMA Descriptor base address Pointer Register
 */

/*
 *	SD PDMA - DMA_RBR : DMA Residual Bytes Register
 */
#define SD_PDMA_RBR_End				0x80000000      /* [0] -- DMA interrupt status */
#define SD_PDMA_RBR_Format			0x40000000      /* [0] -- DMA interrupt status */
/*
 *	SD PDMA - DMA_DAR : DMA Data Address Register
 */

/*
 *	SD PDMA - DMA_BAR : DMA Rbanch Address Register
 */

/*
 *	SD PDMA - DMA_CPR : DMA Command Pointer Register
 */

/*
 *	SD PDMA - DMA_CCR : DMAContext Control Register for Channel 0
 */
#define SD_PDMA_READ					0x00
#define SD_PDMA_WRITE					0x01
#define SD_PDMA_CCR_RUN					0x00000080
#define SD_PDMA_CCR_IF_to_peripheral	0x00000000
#define SD_PDMA_CCR_peripheral_to_IF	0x00400000
#define SD_PDMA_CCR_EvtCode				0x0000000f
#define SD_PDMA_CCR_Evt_no_status		0x00000000
#define SD_PDMA_CCR_Evt_ff_underrun		0x00000001
#define SD_PDMA_CCR_Evt_ff_overrun		0x00000002
#define SD_PDMA_CCR_Evt_desp_read		0x00000003
#define SD_PDMA_CCR_Evt_data_rw			0x00000004
#define SD_PDMA_CCR_Evt_early_end		0x00000005
#define SD_PDMA_CCR_Evt_success			0x0000000f

/*
 *	PDMA Descriptor short
 */
struct _SD_PDMA_DESC_S{
	unsigned long volatile ReqCount:16;		/* bit 0 -15 -Request count             */
	unsigned long volatile i:1;				/* bit 16    -interrupt                 */
	unsigned long volatile reserve:13;		/* bit 17-29 -reserved                  */
	unsigned long volatile format:1;		/* bit 30    -The descriptor format     */
	unsigned long volatile end:1;			/* bit 31    -End flag of descriptor list*/
	unsigned long volatile *DataBufferAddr;	/* bit 31    -Data Buffer address       */
};//__attribute__((packed)); /*mvl tool chain can't uas __attribute__((packed))*/

/*
 *	PDMA Descriptor long
 */
struct _SD_PDMA_DESC_L{
	unsigned long volatile ReqCount:16;		/* bit 0 -15 -Request count             */
	unsigned long volatile i:1;				/* bit 16    -interrupt                 */
	unsigned long volatile reserve:13;		/* bit 17-29 -reserved                  */
	unsigned long volatile format:1;		/* bit 30    -The descriptor format     */
	unsigned long volatile end:1;			/* bit 31    -End flag of descriptor list*/
	unsigned long volatile *DataBufferAddr;	/* bit 31-0  -Data Buffer address       */
	unsigned long volatile *BranchAddr;		/* bit 31-2  -Descriptor Branch address	*/
	unsigned long volatile reserve0;		/* bit 31-0  -reserved					*/
};//__attribute__((packed)); 

//SD card related information
typedef struct sd_info_s {
	unsigned char	RCAHi;
	unsigned char	RCALo;
	unsigned long	SDCard_Size;
	unsigned char	MMC_Card;
	unsigned char	SD_Card;
  	unsigned char 	SD_IO_Card;
	unsigned char	InitOK;
	unsigned char	CardStateChange;
	unsigned char	CardInsert;
	unsigned char	CardRemove;
	unsigned char 	SDVersion;
	unsigned char 	CardCapacityStatus;
	unsigned char	MMCMaxClockRate;
	unsigned char 	HighSpeedSupport;
  	unsigned char	MMCVersion;
	unsigned char	ocr[4];
	unsigned char	cid[16];
	unsigned char	csd[16];
	unsigned char	ext_csd[512];
	unsigned char   scr[8];
	unsigned char	CtrlID;
	unsigned long	RCA;
} sd_info_t;

/*SDMMC control registers.*/
typedef struct{
    volatile unsigned char      ctlr;           /*Control Register*/
    volatile unsigned char      cmdi;           /*Command Index Register*/
    volatile unsigned char      respt;          /*Response Type Register*/
    volatile unsigned char      Reserved1;      /*Reserved1*/
    volatile unsigned char      cmdarg0;        /*Command Argument Register 0x03*/
    volatile unsigned char      cmdarg1;        /*Command Argument Register 0x04*/
    volatile unsigned char      cmdarg2;        /*Command Argument Register 0x05*/
    volatile unsigned char      cmdarg3;        /*Command Argument Register 0x06*/
    volatile unsigned char      busm;           /*Bus Mode Register*/
    volatile unsigned char      Reserved2[3];   /*Reserved2*/
    volatile unsigned short     blklen;         /*Block Length Register*/
    volatile unsigned short     blkcnt;         /*Block Count Register*/
    volatile unsigned char      resr[16];       /*Response Register*/
    volatile unsigned short     cbc;            /*Current Block Count*/
    volatile unsigned short     Reserved3;      /*Reserved3*/
    volatile unsigned char      imr0;           /*Interrupt Mask Register 0*/
    volatile unsigned char      imr1;           /*Interrupt Mask Register 1*/
    volatile unsigned char      Reserved4[2];   /*Reserved3*/
    volatile unsigned char      str0;           /*SD Status Register 0*/
    volatile unsigned char      str1;           /*SD Status Register 1*/
    volatile unsigned char      str2;           /*SD Status Register 2*/
    volatile unsigned char      str3;           /*SD Status Register 3 (SPI Data Error)*/
    volatile unsigned char      rtor;           /*Response Time Out Register*/
    volatile unsigned char      Reserved5[3];   /*Reserved4*/
    volatile unsigned long      Reserved6;      /*Reserved5*/
    volatile unsigned char      extctl;         /*Extend Control Register*/
    volatile unsigned char      Reserved7[3];   /*Reserved6*/
    volatile unsigned short     sblklen;       /*Shadowed Block Length Register*/
    volatile unsigned short     Reserved8;     /*Reserved7*/
    volatile unsigned short     timeval;       /*Timer Value Register*/
}
WMT_SDMMC_REG, *PWMT_SDMMC_REG;



/*GPIO*/
typedef struct _GPIO_REG_
{
  volatile unsigned long CTRL_KPAD_MPGTS_ROM;   // [Rx00] GPIO Enable Control Register for Kpad, Mpeg-TS, CEN_SEEROM
  volatile unsigned long CTRL_DVI;        // [Rx04] GPIO Enable Control Register for DVI Signal
  volatile unsigned long CTRL_CF;         // [Rx08] GPIO Enable Control Register for CF
  volatile unsigned long CTRL_NF;         // [Rx0C] GPIO Enable Control Register for NF
  volatile unsigned long CTRL_PCIAD;        // [Rx10] GPIO Enable Control for PCI AD BUS
  volatile unsigned long CTRL_PCI;        // [Rx14] GPIO Eanble Control for Non PCI AD BUS
  volatile unsigned long CTRL_LCD_SPDIF;      // [Rx18] GPIO Enable Control for LCD, SPDIF
  volatile unsigned long RESV_1C;         // [Rx1C]

  volatile unsigned long OC_KPAD_MPGTS_ROM;   // [Rx20] GPIO Output Control Register for Kpad, Mpeg-TS, CEN_SEEROM
  volatile unsigned long OC_DVI;          // [Rx24] GPIO Output Control Register for DVI Signal
  volatile unsigned long OC_CF;         // [Rx28] GPIO Output Control Register for CF
  volatile unsigned long OC_NF;         // [Rx2C] GPIO Output Control Register for NF
  volatile unsigned long OC_PCIAD;        // [Rx30] GPIO Output Control Register for PCI AD BUS
  volatile unsigned long OC_PCI;          // [Rx34] GPIO Output Control Register for Non PCI AD BUS
  volatile unsigned long OC_LCD_SPDIF;      // [Rx38] GPIO Output Control Register for LCD, SPDIF
  volatile unsigned long RESV_3C;         // [Rx3C]

  volatile unsigned long OD_KPAD_MPGTS_ROM;   // [Rx40] GPIO Output Data Register for Kpad, Mpeg-TS, CEN_SEEROM
  volatile unsigned long OD_DVI;          // [Rx44] GPIO Output Data Register for DVI Signal
  volatile unsigned long OD_CF;         // [Rx44] GPIO Output Data Register for CF
  volatile unsigned long OD_NF;         // [Rx4C] GPIO Output Data Register for NF
  volatile unsigned long OD_PCIAD;        // [Rx50] GPIO Output Data Register for PCIAD
  volatile unsigned long OD_PCI;          // [Rx54] GPIO Output Data Register for Non PCI AD BUS
  volatile unsigned long OD_LCD_SPDIF;      // [Rx58] GPIO Output Data Register for LCD, SPDIF
  volatile unsigned long RESV_5C;         // [Rx5C]

  volatile unsigned long ID_KPAD_MPGTS_ROM;   // [Rx60] GPIO Input Data Register for Kpad, Mpeg-TS, CEN_SEEROM
  volatile unsigned long ID_DVI;          // [Rx64] GPIO Input Data Register for DVI Signal
  volatile unsigned long ID_CF;         // [Rx68] GPIO Input Data Register for CF
  volatile unsigned long ID_NF;         // [Rx6C] GPIO Input Data Register for NF
  volatile unsigned long ID_PCIAD;        // [Rx70] GPIO Input Data Register for PCIAD
  volatile unsigned long ID_PCI;          // [Rx74] GPIO Input Data Register for Non PCI AD BUS
  volatile unsigned long ID_LCD_SPDIF;      // [Rx78] GPIO Input Data Register for LCD, SPDIF
  volatile unsigned long RESV_7C;         // [Rx7C]

  volatile unsigned long RESV_80[0x20];     // [Rx80-RxFF]

  volatile unsigned long STRAP_STS1;        // [Rx100] Strapping Option Status Register1
  volatile unsigned long STRAP_STS2;        // [Rx104] Strapping Option Status Register2
  volatile unsigned long AHB_CTRL;        // [Rx108] AHB Control Register
  volatile unsigned long RESV_10C[0x3D];      // [Rx10C-Rx1FF]

  volatile unsigned long PIN_MULTI;       // [Rx200] Pin Multiplexing
  volatile unsigned long RESV_204[0x3];     // [Rx204-Rx20F]
  volatile unsigned long PCI_DELAY_TEST;      // [Rx210]
  volatile unsigned long PCI_RESET;       // [Rx214]
  volatile unsigned long RESV_218[2];       // [Rx218 - Rx21F]
  volatile unsigned long PCI_1X_DELAY;      // [Rx220]
  volatile unsigned long PCI_2X_DELAY;      // [Rx224]
  volatile unsigned long PCI_2X_RELAY_INVERT; // [Rx228]
  volatile unsigned long RESV_22C[0x35];      // [Rx22C - Rx2FF]

  volatile unsigned long INT_REQ_TYPE;      // [Rx300] GPIO Interrupt Request Type Register
  volatile unsigned long INT_REQ_STS;       // [Rx304] GPIO Interrupt Request Status Register
  volatile unsigned long RESV_308[0x3E];      // [Rx308-Rx3FF]

  volatile unsigned long IDE_IO_DRV;        // [Rx400] IDE I/O Drive Strength Register
  volatile unsigned long MS_IO_DRV;       // [Rx404] MS I/O Drive Strength and Slew Rate Register
  volatile unsigned long PCI_IO_DRV;        // [Rx408] PCI Bus I/O Drive Strength and Slew Rate Register
  volatile unsigned long SD_IO_DRV;       // [Rx40C] SD I/O Drive Strength and Slwe Rate Register
  volatile unsigned long VID_IO_DRV;        // [Rx410] VID Clock I/O Drive Strength and Slew Rate Register
  volatile unsigned long SPI_IO_DRV;        // [Rx414] SPI I/O Drive Strength and Slew Rate Register
  volatile unsigned long CF_IO_DRV;       // [Rx418] CF I/O Drive Strength and Slew Rate Register
  volatile unsigned long LPC_IO_DRV;        // [Rx41C] LPC I/O Drive Strength and Slew Rate Register
  volatile unsigned long NF_IO_DRV;       // [Rx420] NAND I/O Drive Strength and Slew Rate Register
  volatile unsigned long MISC_IO_DRV;       // [Rx424] MISC I/O Drive Strength and Slew Rate Register
}GPIO_REG, * PGPIO_REG;

#endif
