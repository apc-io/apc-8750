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

#ifndef __DMA_H__
#define __DMA_H__


/***********************************************
* The VT3426 has 16 internal DMA channels.
*************************************************/
#define DMA_CHANNELS    2
#define MAX_DMA_CHANNELS DMA_CHANNELS
#define DMA_INT0 17
#define DMA_INT1 27
#define DMA_INT2 34
#define DMA_INT3 40
#define DMA_INT4 41
#define DMA_INT5 45
#define DMA_INT6 46
#define DMA_INT7 51
#define DMA_INT8 92
#define DMA_INT9 93
#define DMA_INT10 94
#define DMA_INT11 95
#define DMA_INT12 96
#define DMA_INT13 97
#define DMA_INT14 98
#define DMA_INT15 99


/***********************************************
* The DMA MEMORY REGISTER BASE ADDRESS.
*************************************************/
#define DMA_MEMREG_BASEADDR	0x5000000 /*80MB*/
#define DMA_DES0_BASEADDR 0x5800000 /*88MB*/
#define DMA_DES12_BASEADDR 0x5B00000 /*91MB*/

/**************************************
 This is the maximum DMA address that can be DMAd to.
***************************************/
#define MAX_DMA_ADDRESS		0x00FFFFFF

/*************************************
*  DMA_TCR_CHX_VAL
*  DMA Transfer Count
**************************************/
/* Reserved [31:24] */
#define DMA_TCR_MASK                        0x00FFFFFF      /* [23:00] -- Mask   */

/************************************
*
*  DMA GLOBAL CONTROL
*
*************************************/
#define DMA_SW_RST	BIT8
#define DMA_BIG_ENDIAN	BIT1
#define DMA_GLOBAL_EN	BIT0
/************************************
*
*  DMA_INTERRUPT ENABLE
*
*************************************/
#define CH00_INT_EN	BIT0
#define CH01_INT_EN	BIT1
#define CH02_INT_EN	BIT2
#define CH03_INT_EN	BIT3
#define CH04_INT_EN	BIT4
#define CH05_INT_EN	BIT5
#define CH06_INT_EN	BIT6
#define CH07_INT_EN	BIT7
#define CH08_INT_EN	BIT8
#define CH09_INT_EN	BIT9
#define CH10_INT_EN	BIT10
#define CH11_INT_EN	BIT11
#define CH12_INT_EN	BIT12
#define CH13_INT_EN	BIT13
#define CH14_INT_EN	BIT14
#define CH15_INT_EN	BIT15
#define ALL_INT_EN	0x0000FFFF

/************************************
*
*  DMA_INTERRUPT STATUS
*
*************************************/
#define CH00_INT_STS	BIT0
#define CH01_INT_STS	BIT1
#define CH02_INT_STS	BIT2
#define CH03_INT_STS	BIT3
#define CH04_INT_STS	BIT4
#define CH05_INT_STS	BIT5
#define CH06_INT_STS	BIT6
#define CH07_INT_STS	BIT7
#define CH08_INT_STS	BIT8
#define CH09_INT_STS	BIT9
#define CH10_INT_STS	BIT10
#define CH11_INT_STS	BIT11
#define CH12_INT_STS	BIT12
#define CH13_INT_STS	BIT13
#define CH14_INT_STS	BIT14
#define CH15_INT_STS	BIT15
#define ALL_INT_CLEAR	0x0000FFFF

/************************************
*
*  DMA SCHEDULE SCHEME
*
*************************************/
#define SCHEDULE_RR_DISABLE	BIT0
#define TIMER_1_SHIFT	16
#define TIMER_2_SHIFT	8
/************************************
*
*  DMA_CCR SETTIGN
*
*************************************/
/*WRAP MODE [31:30]*/
#define DMA_WRAP_1	0x00000000
#define DMA_WRAP_2	BIT30
#define DMA_WRAP_4	BIT31
#define DMA_WRAP_8	(BIT31|BIT30)
#define DMA_WRQP_MASK	0xC0000000
/*BUST[29:28]*/
#define DMA_BURST_1	0x00000000
#define DMA_BURST_4	BIT28
#define DMA_BURST_8	BIT29
#define DMA_BURST_MASK	0x30000000
/*TRANSFER SIZE[[27:26]*/
#define DMA_SIZE_8	0x00000000
#define DMA_SIZE_16	BIT26
#define DMA_SIZE_32	BIT27
#define DMA_SIZE_MASK	0x0C000000
/*/1/2 addr mode[25:24]*/
#define DMA_WRAP_MODE	0x00000000
#define DMA_INC_MODE	BIT24
#define DMA_SG_MODE	BIT25
#define DMA_UP_MEMREG_EN BIT15
#define DMA_ADDR_MODE_MASK	0x03000000
/*SW REQUEST ENABLE[23]*/
#define DMA_SW_REQ	BIT23
#define DMA_SW_REQ_MASK	0x00800000
/*DMA 1/2 TRANS DIRECTION[22]*/
#define DEVICE_TO_MEM	BIT22
#define DEVICE_TO_MEM_MASK	0x00400000
/*DMA REQ NUM[20:16]*/
#define DMA_REQ_ID_SHIFT	16
#define DMA_REQ_ID_MASK		0x000F8000

/*DMA complete BIT*/
#define DMA_P0_COMPLETE	BIT9	   /*0:complete 1:did't complete*/
#define DMA_P1_COMPLETE	BIT8

#define SYSTEM_DMA_RUN	BIT7
#define DMA_RUN_MASK	0x00000080
#define DMA_WAKE BIT6
#define DMA_WAKE_MASK	0x00000040
#define DMA_ACTIVE	BIT4
#define DMA_ACTIVE_MASK	0x00000010
#define DMA_USER_SET_MASK	0xFFFE0000

/*DMA ERROR ID[3:0]*/
#define DMA_EVT_ID_MASK	0x0000000F
#define DMA_EVT_NO_STATUS	0
#define DMA_EVT_REG	1
#define DMA_EVT_FF_UNDERRUN	2
#define DMA_EVT_FF_OVERRUN	3
#define DMA_EVT_DESP_READ	4
#define DMA_EVT_DESP_WRITE	5
#define DMA_EVT_MR_READ		6
#define DMA_EVT_MR_WRITE	7
#define DMA_EVT_DATA_READ	8
#define DMA_EVT_DATA_WRITE	9
#define DMA_EVT_SUCCESS		15


/*****************************************
	DMA data transfer size
******************************************/
#define SIZE_1B             	0x00000001
#define SIZE_2B             	0x00000002
#define SIZE_4B             	0x00000004
#define SIZE_8B             	0x00000008
#define SIZE_16B           	0x00000010
#define SIZE_32B            	0x00000020
#define SIZE_64B            	0x00000040
#define SIZE_128B           	0x00000080
#define SIZE_256B           	0x00000100
#define SIZE_512B           	0x00000200
#define SIZE_1KB            	0x00000400
#define SIZE_2KB            	0x00000800
#define SIZE_4KB            	0x00001000
#define SIZE_8KB            	0x00002000
#define SIZE_16KB           	0x00004000
#define SIZE_32KB           	0x00008000
#define SIZE_64KB           	0x00010000
#define SIZE_128KB          	0x00020000
#define SIZE_256KB          	0x00040000
#define SIZE_512KB          	0x00080000
#define SIZE_1MB            	0x00100000
#define SIZE_2MB            	0x00200000
#define SIZE_4MB            	0x00400000
#define SIZE_8MB            	0x00800000
#define SIZE_16MB           	0x01000000
#define SIZE_32MB           	0x02000000
#define SIZE_64MB           	0x04000000
#define SIZE_128MB          0x08000000
#define SIZE_256MB          0x10000000
#define SIZE_512MB          0x20000000
#define SIZE_1GB            	0x40000000
#define SIZE_2GB            	0x80000000
#define TRAN_SIZE	SIZE_2KB

/*****************************************
	DMA descript setting
******************************************/
#define DMA_DES_END	BIT31
#define DMA_FORMAT_DES1	BIT30
#define DMA_INTEN_DES	BIT16
#define DMA_DES0_SIZE 0x8	/*8 byte*/
#define DMA_DES1_SIZE 0x10	/*16 byte*/
/*************************************************
*
*	DMA Controller Register Set
*
**************************************************/

struct DMA_MEM_REG_GROUP {
	volatile unsigned long DMA_IF0RBR_CH;
	volatile unsigned long DMA_IF0DAR_CH;
	volatile unsigned long DMA_IF0BAR_CH;
	volatile unsigned long DMA_IF0CPR_CH;
	volatile unsigned long DMA_IF1RBR_CH;
	volatile unsigned long DMA_IF1DAR_CH;
	volatile unsigned long DMA_IF1BAR_CH;
	volatile unsigned long DMA_IF1CPR_CH;
};
struct DMA_MEM_REG {
	struct DMA_MEM_REG_GROUP MEM_REG_GROUP[DMA_CHANNELS];
};

struct DMA_REG {
	volatile unsigned long RESERVED[0x10];/*0x00-0x3F*/
	volatile unsigned long DMA_GCR; /*0x40-0x43*/
	volatile unsigned long DMA_MRPR;/*0x44-0x47*/
	volatile unsigned long DMA_IER; /*0x48-0x4B*/
	volatile unsigned long DMA_ISR; /*0x4C-0x4F*/
	volatile unsigned long DMA_TMR; /*0x50-0x53*/
	volatile unsigned long RESERVED_1[0xB];/*0x54-0x7F*/
	volatile unsigned long DMA_CCR_CH[0x10];/*0x80-0xBF*/
	volatile unsigned long RESERVED_2[0x10];/*0xC0-0xFF*/
};

struct DMA_DES_FMT0 {
	volatile unsigned long REQCNT;
	volatile unsigned long DATAADDR;
};
struct DMA_DES_FMT1 {
	volatile unsigned long REQCNT;
	volatile unsigned long DATAADDR;
	volatile unsigned long BRADDR;
	volatile unsigned long RESERVED;

};

struct desp_stack{
    unsigned int addr_base;
    unsigned int length;
    unsigned int cur_index;
    unsigned int head_index;
    unsigned int freedesp_num;
    unsigned int mem_startaddr;
    unsigned int mem_length;
};

struct DMA_DESCRIPT_ADDR {
	volatile unsigned long DES_0;
	volatile unsigned long DES_1_2;
	struct desp_stack despstack_0;
	struct desp_stack despstack_1_2;
};
/**************************************************
*All possible devices a DMA channel can be attached to.
***************************************************/
enum dma_device {
	SPI0_DMA_TX_REQ = 0 , /*spi0tx*/
	SPI0_DMA_RX_REQ = 1 , /*spi0rx*/
	SPI1_DMA_TX_REQ = 2 , /*spi1tx*/
	SPI1_DMA_RX_REQ = 3 , /*spi1tx*/
	SPI2_DMA_TX_REQ = 4 , /*spi2rx*/
	SPI2_DMA_RX_REQ = 5 , /*spi2rx*/
	UART_0_TX_DMA_REQ = 6,/* uart0*/
	UART_0_RX_DMA_REQ = 7,/* uart0*/
	UART_1_TX_DMA_REQ = 8,/* uart1*/
	UART_1_RX_DMA_REQ = 9,/* uart1*/
	UART_2_TX_DMA_REQ = 10,/* uart2*/
	UART_2_RX_DMA_REQ = 11,/* uart2*/
	UART_3_TX_DMA_REQ = 12,/* uart3*/
	UART_3_RX_DMA_REQ = 13,/* uart3*/
	I2S_TX_DMA_REQ = 14 ,/* i2s*/
	I2S_RX_DMA_REQ = 15 ,/* i2s*/
	UART_4_TX_DMA_REQ = 16,/* uart4*/
	UART_4_RX_DMA_REQ = 17,/* uart4*/
	AC97_PCM_TX_DMA_REQ = 18,/*AC97*/
	AC97_PCM_RX_DMA_REQ = 19,/*AC97*/
	AC97_MIC_DMA_REQ = 20, /*AC97 MIC*/
	UART_5_TX_DMA_REQ = 29,/* uart5*/
	UART_5_RX_DMA_REQ = 30,/* uart5*/
	MEMORY_DMA_REQ = 32,/* memory*/
	DEVICE_RESERVED = 33 /* reserved*/
};

/******************************************************************************
* DMA channel structure.
*******************************************************************************/
struct dma_device_cfg{
	enum dma_device	DeviceReqType;
	unsigned long DefaultCCR;
	unsigned long MIF0addr;
	unsigned long MIF1addr;
};

struct DMA_INFO{
	unsigned long  channel_no;       	/* channel no*/
	struct DMA_REG *regs;           		/* points to appropriate DMA registers*/
	struct DMA_MEM_REG *mem_regs;           /*points to DMA memory registers*/
	const char *device_id;      		/* device name*/
	//void (*callback)(void);		/* to call when buffers are done*/
	enum dma_device device_no;    		/* device no*/
	struct dma_device_cfg device_cfg;  	/* device cfg*/
	int in_use;                 			/* Does someone own the channel*/
	int descript_cnt;                 			/* count of queued buffers*/
	struct DMA_DESCRIPT_ADDR DES_ADDR;
};

struct dma_state{
	unsigned int request_chans;
	struct DMA_REG *regs;
};



extern unsigned int dma_irq_mapping[] ;

void init_dma(void);

void disable_dma(void);

void enable_dma(void);

int request_dma(unsigned int *channel, const char *device_id, enum dma_device device);

void clear_dma(unsigned int ch);

void free_dma(unsigned int ch);

int setup_dma(unsigned int ch, struct dma_device_cfg device_cfg);


int dma_busy(unsigned int ch);

int dma_complete(unsigned int ch);

int start_dma(unsigned long ch, unsigned long dma_ptr1, unsigned long dma_ptr2, unsigned long size);

void handle_dma_irq(unsigned long ch);

/****************************************************

*Function phototype in Dmaif.c

*****************************************************/

//void dma_initialization(IN int FunctionNumber);

//void dma_startio(void);

//void dma_diagio(struct diag *pdiag);

//void free_dma_channels(void);

void wake_dma_channel(unsigned int);

//unsigned int dma_dump_regs(void);

//unsigned int bus_size32_test(unsigned int);

//unsigned int bus_size16_test(unsigned int);

//unsigned int bus_size8_test(unsigned int);

//unsigned int mix_des_test(unsigned int);

//unsigned int wakeup_channel_test(unsigned int);

//unsigned int all_channels_test(void);

//unsigned int dma_fixed_priority_test(void);

//unsigned int dma_rotating_priority_test(void);

//unsigned int dma_channel_interrupt_test(int ch);

//unsigned int bus_halfword_test(unsigned int);

//unsigned int bus_32bit_byte_test(unsigned int);

//unsigned int bus_16bit_byte_test(unsigned int);

int prepare_dma_descript(unsigned int, unsigned int, unsigned int);

int prepare_dma_long_descript(unsigned int start_descript, unsigned int branch_descript,
	unsigned int start_addr, unsigned int size, unsigned int last_des);

void add_descript(unsigned int, unsigned int, unsigned int, unsigned int);

void init_descriptstack(unsigned int ch);
void reset_descriptstack(unsigned int ch);
void free_descriptstack(unsigned int ch);
int set_descript(struct desp_stack * pdesp_stack, unsigned int start_addr, unsigned int size);
int handle_transfer(unsigned long ch, unsigned long dma_ptr1, unsigned long dma_ptr2, unsigned long size);
#endif  /* __DMA_H__ */

