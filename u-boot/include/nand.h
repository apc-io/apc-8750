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

#ifndef __NAND_H__
#define __NAND_H__

#define __NFC_BASE 0xd8009000
#define __SCC_BASE 0xd8120000

typedef struct WMT_NFC_CFG_t{
	volatile unsigned int NFCR0;           /* 0x00 */
	volatile unsigned int	NFCR1;           /* 0x04 */
	volatile unsigned int	NFCR2;           /* 0x08 */
	volatile unsigned int	NFCR3;           /* 0x0c */
	volatile unsigned int	NFCR4;           /* 0x10 */
	volatile unsigned int NFCR5;           /* 0x14 */
	volatile unsigned int	NFCR6;           /* 0x18 */
	volatile unsigned int	NFCR7;           /* 0x1c */
	volatile unsigned int	NFCR8;           /* 0x20 */
	volatile unsigned int	NFCR9;           /* 0x24 */
	volatile unsigned int NFCRa;           /* 0x28 */
	volatile unsigned int	NFCRb;           /* 0x2c */
	volatile unsigned int	NFCRc;           /* 0x30 */
	volatile unsigned int	NFCRd;           /* 0x34 */
	volatile unsigned int NFCRe;           /* 0x38 */
	volatile unsigned int NFCRf;           /* 0x3c */
	volatile unsigned int NFCR10;          /* 0x40 */
	volatile unsigned int NFCR11;          /* 0x44 */
	volatile unsigned int NFCR12;          /* 0x48 */
	volatile unsigned int NFCR13;          /* 0x4c */
	volatile unsigned int NFCR14;          /* 0x50 */
	volatile unsigned int NFCR15;          /* 0x54 */
	volatile unsigned int NFCR16;          /* 0x58 */
	volatile unsigned int NFCR17;          /* 0x5c */
	volatile unsigned int NFCR18;          /* 0x60 */
	volatile unsigned int NFCR19;          /* 0x64 */
	volatile unsigned int NFCR1a;          /* 0x68 */
	volatile unsigned int NFCR1b;          /* 0x6c */
	volatile unsigned int NFCR1c;          /* 0x70 */
	volatile unsigned int NFCR1d;          /* 0x74 */
	volatile unsigned int NFCR1e;          /* 0x78 */
	volatile unsigned int NFCR1f;          /* 0x7c */
	volatile unsigned int NFCR20;          /* 0x80 */
	volatile unsigned int NFCR21;          /* 0x84 */
	volatile unsigned int NFCR22;          /* 0x88 */
	volatile unsigned int NFCR23;          /* 0x8c */
	volatile unsigned int NFCR24;          /* 0x90 */
	volatile unsigned int NFCR25;          /* 0x94 */
	volatile unsigned int NFCR26;          /* 0x98 */
	volatile unsigned int NFCR27;          /* 0x9c */
	volatile unsigned int NFCR28;          /* 0xa0 */
	volatile unsigned int NFCR29;          /* 0xa4 */
	volatile unsigned int NFCR2a;          /* 0xa8 */
	volatile unsigned int NFCR2b;          /* 0xac */
	volatile unsigned int NFCR2c;          /* 0xb0 */
	volatile unsigned int NFCR2d;          /* 0xb4 */
	volatile unsigned int NFCR2e;          /* 0xb8 */
	volatile unsigned int NFCR2f;          /* 0xbc */
	volatile unsigned int NFCR30;					 /* 0xc0 */
	volatile unsigned int NFCR31;					 /* 0xc4 */
	volatile unsigned int NFCR32;					 /* 0xc8 */
	volatile unsigned int resv1[13]; /* 0xcc ~ 0xfc */
	volatile unsigned int DMA_GCR;  /* 0x100 */
	volatile unsigned int DMA_IER;  /* 0x104 */
	volatile unsigned int DMA_ISR;  /* 0x108 */
	volatile unsigned int DMA_DESPR;/* 0x10C */
	volatile unsigned int DMA_RBR;  /* 0x110 */
	volatile unsigned int DMA_DAR;  /* 0x114 */
	volatile unsigned int DMA_BAR;  /* 0x118 */
	volatile unsigned int DMA_CPR;  /* 0x11C */
	volatile unsigned int DMA_CCR;  /* 0x120 */
	volatile unsigned int resv2[39];	/* 0x124 ~ 0x1BC */
	volatile unsigned char FIFO[64]; /* 0x1C0 ~ 0x1FC */
} WMT_NFC_CFG, *WMT_PNFC_CFG;

typedef struct WMTNF_INFO_s {
	unsigned int 	id;
	unsigned int 	oobblock;
	unsigned int	oobsize;
	unsigned int	page_shift;
	unsigned int	totblk;
	unsigned int	erasesize;
	unsigned int  col; /* addr_cycle; */
	unsigned int  row; /* page_cycle; */
	unsigned int 	page[2];
	unsigned int  bbpos;
	//struct WMT_nand_flash_dev *nand_dev_info;
	int (*nfc_read_page)(unsigned int start, unsigned int maddr, unsigned int len);
	int (*update_table_inflash)(unsigned int last);
	int (*update_table_inram)(unsigned int addr);
	int (*isbadblock)(unsigned int addr);
} WMTNF_INFO_t;

typedef struct nand_oob_s {
	unsigned int   dwReserved1;   /* Reserved - used by FAL */
	unsigned char  bOEMReserved;  /* For use by OEM */
	unsigned char  bBadBlock;     /* Indicates if block is BAD */
	unsigned int   dwReserved2;   /* Reserved - used by FAL */
} nand_oob_t;

typedef struct WMTNAND_ID_s {
	unsigned int 	manuf_code;
	unsigned int 	dev_code;
	unsigned int  width;
	unsigned int 	oobblock;
	unsigned int  oobsize;
	unsigned int	totblk;
	unsigned int	erasesize;
	unsigned int  totcycles;
	unsigned int  bbpos1;
	unsigned int  bbpos2;
} WMTNAND_ID_t;



/*
 *	NAND PDMA
 */
#define NAND_PDMA_BASE_ADDR 0xD8009100
#define NAND_DESC_BASE_ADDR 0x00D00000
struct _NAND_PDMA_REG_ {
	volatile unsigned long DMA_GCR;		/*	Rx00	*/
	volatile unsigned long DMA_IER;		/*	Rx04	*/
	volatile unsigned long DMA_ISR;		/*	Rx08	*/
	volatile unsigned long DMA_DESPR;	/*	Rx0C	*/
	volatile unsigned long DMA_RBR;		/*	Rx10	*/
	volatile unsigned long DMA_DAR;		/*	Rx14	*/
	volatile unsigned long DMA_BAR;		/*	Rx18	*/
	volatile unsigned long DMA_CPR;		/*	Rx1C	*/
	volatile unsigned long DMA_CCR;		/*	RX20	*/
	volatile unsigned long resv[5];  	/*	RX2C-3C	*/
} NAND_PDMA_REG, *PNAND_PDMA_REG;

/*
 *	NAND PDMA - DMA_GCR : DMA Global Control Register
 */
#define NAND_PDMA_GCR_DMA_EN			0x00000001      /* [0] -- DMA controller enable */
#define NAND_PDMA_GCR_SOFTRESET		0x00000100      /* [8] -- Software rest */

/*
 *	NAND PDMA - DMA_IER : DMA Interrupt Enable Register
 */
#define NAND_PDMA_IER_INT_EN			0x00000001      /* [0] -- DMA interrupt enable */
/*
 *	NAND PDMA - DMA_ISR : DMA Interrupt Status Register
 */
#define NAND_PDMA_IER_INT_STS			0x00000001      /* [0] -- DMA interrupt status */
/*
 *	NAND PDMA - DMA_DESPR : DMA Descriptor base address Pointer Register
 */

/*
 *	NAND PDMA - DMA_RBR : DMA Residual Bytes Register
 */
#define NAND_PDMA_RBR_End				0x80000000      /* [31] -- DMA descriptor end flag */
#define NAND_PDMA_RBR_Format			0x40000000      /* [30] -- DMA descriptor format */
/*
 *	NAND PDMA - DMA_DAR : DMA Data Address Register
 */

/*
 *	NAND PDMA - DMA_BAR : DMA Rbanch Address Register
 */

/*
 *	NAND PDMA - DMA_CPR : DMA Command Pointer Register
 */

/*
 *	NAND PDMA - DMA_CCR : DMAContext Control Register for Channel 0
 */
#define NAND_PDMA_READ					0x00
#define NAND_PDMA_WRITE					0x01
#define NAND_PDMA_CCR_RUN					0x00000080
#define NAND_PDMA_CCR_IF_to_peripheral	0x00000000
#define NAND_PDMA_CCR_peripheral_to_IF	0x00400000
#define NAND_PDMA_CCR_EvtCode				0x0000000f
#define NAND_PDMA_CCR_Evt_no_status		0x00000000
#define NAND_PDMA_CCR_Evt_ff_underrun		0x00000001
#define NAND_PDMA_CCR_Evt_ff_overrun		0x00000002
#define NAND_PDMA_CCR_Evt_desp_read		0x00000003
#define NAND_PDMA_CCR_Evt_data_rw			0x00000004
#define NAND_PDMA_CCR_Evt_early_end		0x00000005
#define NAND_PDMA_CCR_Evt_success			0x0000000f

/*
 *	PDMA Descriptor short
 */
struct _NAND_PDMA_DESC_S{
	unsigned long volatile ReqCount:16;		/* bit 0 -15 -Request count             */
	unsigned long volatile i:1;				/* bit 16    -interrupt                 */
	unsigned long volatile reserve:13;		/* bit 17-29 -reserved                  */
	unsigned long volatile format:1;		/* bit 30    -The descriptor format     */
	unsigned long volatile end:1;			/* bit 31    -End flag of descriptor list*/
	unsigned long volatile DataBufferAddr:32;/* bit 31    -Data Buffer address       */
} NAND_PDMA_DESC_S, *PNAND_PDMA_DESC_S;

/*
 *	PDMA Descriptor long
 */
struct _NAND_PDMA_DESC_L{
	unsigned long volatile ReqCount:16;		/* bit 0 -15 -Request count             */
	unsigned long volatile i:1;				/* bit 16    -interrupt                 */
	unsigned long volatile reserve:13;		/* bit 17-29 -reserved                  */
	unsigned long volatile format:1;		/* bit 30    -The descriptor format     */
	unsigned long volatile end:1;			/* bit 31    -End flag of descriptor list*/
	unsigned long volatile DataBufferAddr:32;/* bit 31-0  -Data Buffer address       */
	unsigned long volatile BranchAddr:32;	/* bit 31-2  -Descriptor Branch address	*/
	unsigned long volatile reserve0:32;		/* bit 31-0  -reserved */
} NAND_PDMA_DESC_L, *PNAND_PDMA_DESC_L;


#define DMA_SINGNAL 0
#define DMA_INC4 0x10
#define DMA_INC8 0x20

#define ECC1bit 0
#define ECC4bit 1
#define ECC8bit 2
#define ECC12bit 3
#define ECC16bit 4
#define ECC24bitPer1K 5
#define ECC40bitPer1K 6
#define ECC44bitPer1K 7
#define ECC44bit 8
#define ECC1bit_bit_count 32
#define ECC4bit_bit_count 52
#define ECC8bit_bit_count 104
#define ECC12bit_bit_count 156
#define ECC16bit_bit_count 208
#define ECC24bitPer1K_bit_count 336
#define ECC40bitPer1K_bit_count 560
#define ECC44bitPer1K_bit_count 616
#define ECC44bit_bit_count 576
#define ECC1bit_byte_count 4
#define ECC4bit_byte_count 8
#define ECC8bit_byte_count 16
#define ECC12bit_byte_count 20
#define ECC16bit_byte_count 26
#define ECC24bitPer1K_byte_count 42
#define ECC40bitPer1K_byte_count 70
#define ECC44bitPer1K_byte_count 77
#define ECC44bit_byte_count 72
#define ECC_MOD_SEL 7
#define MAX_ERR_BIT 44
#define eccBCH_interrupt_enable 0x101

#define NAND_RESERVED 0xFE
#define NAND_LARGE_RESERVED_OFS 8
#define NAND_SMALL_RESERVED_OFS 7
#define NAND_DWRESERVED1 0x54424C30
#define NAND_LARGE_DWRESERVED1_OFS 4
#define NAND_SMALL_DWRESERVED1_OFS 0
#define NAND_DWRESERVED2 0x30
#define NAND_LARGE_DWRESERVED2_OFS 0
#define NAND_SMALL_DWRESERVED2_OFS 14

#define ADDR_COLUMN 1
#define ADDR_PAGE 2
#define ADDR_COLUMN_PAGE 3

/* err define */
#define ERR_NFC_CMD  1
#define ERR_NAND_IDLE  2
#define ERR_DMA_CFG 3
#define ERR_NAND_READY  4
#define ERR_NFC_READY  5
#define ERR_NO_BBT	   6
#define ERR_NAND_UNKNOW 7
#define ERR_NOT_SUPPORT 8
#define ERR_ECC_UNCORRECT 9
#define ERR_ECC_BANK	10
#define ERR_UNKNOW 11
#define ERR_NAND_BOUND 12
#define ERR_BBT_BAD 13

#define ERR_IMAGE_FORMAT 13
/* nand id */
#define NAND_HYNIX 0xad
#define NAND_HYNIX_new 0x2c
#define NAND_SAMSUNG 0xec
#define NAND_TOSHIBA 0x98
#define NAND_UNKONW 0x00

#define ECC_1bit_ERR 0x02

/*
 * Standard NAND flash commands
 */
#define NAND_READ0		0
#define NAND_READ1		1
#define NAND_READ2		0x50
#define NAND_PAGEPROG	0x10
#define NAND_ERASE_SET	0x60
#define NAND_STATUS		0x70
#define NAND_MULTI_STATUS	0x71
#define NAND_SEQIN		0x80
#define NAND_READID		0x90
#define NAND_ERASE_CONFIRM		0xd0
#define NAND_RESET		0xff

#define	NAND_READ_CONFIRM 0x30
#define	NAND_DUMMY_READ 0x03
#define NAND_DUMMY_PAGEPROG 0x11
#define NAND_CMD_COPYBACK 0x8A

/* Standard NAND Status bits */
#define NAND_STATUS_FAIL	0x01
#define PLANE0_FAIL				0x02
#define PLANE1_FAIL				0x04
#define PLANE2_FAIL				0x08
#define PLANE3_FAIL				0x10
#define NAND_STATUS_PER 	0x20
#define NAND_STATUS_READY	0x40
#define NAND_STATUS_NP		0x80

/*
 *    Register Status Bit
 */
/* cfg_12 */
#define WP_DISABLE (1<<4) /* disable write protection */
#define OLDATA_EN (1<<2)  /* enable old data structure */
#define WIDTH_8	0
#define WIDTH_16	(1<<3)
#define PAGE_512 0
#define PAGE_2K 1
#define PAGE_4K 2
#define PAGE_8K 3
#define DIRECT_MAP (1<<5)
/* cfg_1 */
#define DPAHSE_DISABLE 0x80 /* disable data phase */
#define NAND2NFC  0x40  /* direction : nand to controller */
#define NFC2NAND  0x00  /* direction : controller to controller */
#define SING_RW	0x20 /*	enable signal read/ write command */
#define MUL_CMDS 0x10  /*  support cmd+addr+cmd */
#define NFC_TRIGGER 0x01 /* start cmd&addr sequence */
/* cfg_9 */
#define OOB_RW 0x02
/* cfg_a */
#define NFC_CMD_RDY 0x04
#define NFC_BUSY	0x02  /* command and data is being transfer in flash I/O */
#define FLASH_RDY 0x01  /* flash is ready */
/* cfg_b */
#define B2R	0x08 /* status form busy to ready */
/* cfg_15 */
#define USE_SW_ECC 0x04
#define USE_HW_ECC 0

/* cfg_1d */
#define NFC_IDLE	0x01

/* cfg_23 */
#define READ_RESUME 0x100
/* cfg_25 status */
#define ERR_CORRECT 0x100
#define BCH_ERR     0x1
/* cfg_26 status */
#define BANK_DR     0x800
#define BANK_NUM    0x700
#define BCH_ERR_CNT 0x1F

#define FLASH_UNKNOW 0xFFFF

#define ADDR_COLUMN 1
#define ADDR_PAGE 2
#define ADDR_COLUMN_PAGE 3

/*
 * Standard NAND flash commands
 */
#define NAND_CMD_READ0		0
#define NAND_CMD_READ1		1
#define NAND_CMD_READ2		0x50
#define NAND_CMD_PAGEPROG	0x10
#define NAND_CMD_ERASE1		0x60
#define NAND_CMD_STATUS		0x70
#define NAND_CMD_MULTI_STATUS	0x71
#define NAND_CMD_SEQIN		0x80
#define NAND_CMD_READID		0x90
#define NAND_CMD_ERASE2		0xd0
#define NAND_CMD_RESET		0xff
#define NAND_CMD_RANDOM_DATA_IN 0x85

#define	NAND_DUMMY_READ 0x03
#define NAND_DUMMY_PAGEPROG 0x11
#define NAND_CMD_COPYBACK 0x8A
#define NAND_LAG_READ		0x30

/* Status bits */
#define NAND_STATUS_ERR	0x01

#define DMA_SINGNAL 0
#define DMA_INC4 0x10
#define DMA_INC8 0x20

/* #define TEST_FUN */

#ifdef TEST_FUN
#define __OST_BASE  0xD8130100    /* OS Timers base address */

typedef struct nand_ost_regs_s {
	unsigned int volatile OSMR[4];  /* Match Register 0-3        */
	unsigned int volatile OSCR;     /* Count Register            */
	unsigned int volatile OSTS;     /* Status Register           */
	unsigned int volatile OSTW;     /* Watchdog Enable Register  */
	unsigned int volatile OSTI;     /* Interrupt Enable Register */
	unsigned int volatile OSTC;     /* Control Register          */
	unsigned int volatile OSTA;     /* Access Status Register    */
} nand_ost_regs_t;

/* for ecc test */
/* we setup max 5 error to test */
typedef struct ecc_test_info_s {
	unsigned int err_cnt;
	unsigned int pos[5];
} ecc_test_info_t;

int post_write_test(unsigned int size, unsigned int cnt);
int post_read_test(unsigned int size, unsigned int cnt);
void nand_read_counter(unsigned int *data);
unsigned int hex2dec(unsigned int hex);
int post_rw_one(unsigned int type);
int post_rw_comp(unsigned int size, unsigned int cnt, unsigned int type, unsigned int type1);
int post_table_test(unsigned int type);
unsigned int creat_bch_pattern(unsigned char *fifo, ecc_test_info_t info,
		unsigned int oob, unsigned char *buf);
int post_bch_program(unsigned int *naddr, unsigned int first, unsigned int maddr);
int post_test_bch(unsigned int count, unsigned int oob);
int post_comp_bch(unsigned int src, unsigned int dest);
int post_read_by_bank(unsigned int start, unsigned int maddr, unsigned int len, unsigned char *fifo);
#endif

/*
 *	PDMA API
 */
int nand_init_pdma(void);
int nand_free_pdma(void);
int nand_alloc_desc_pool(unsigned long *DescAddr);
int nand_init_short_desc(unsigned long *DescAddr, unsigned int ReqCount, unsigned long *BufferAddr, int End);
int nand_init_long_desc(unsigned long *DescAddr, unsigned int ReqCount, unsigned long *BufferAddr,
unsigned long *BranchAddr, int End);
int nand_config_pdma(unsigned long *DescAddr, unsigned int dir);
int nand_pdma_handler(void);
#endif
