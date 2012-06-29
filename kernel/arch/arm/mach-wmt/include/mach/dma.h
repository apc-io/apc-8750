/*++
linux/include/asm-arm/arch-wmt/dma.h

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

#ifndef __ASM_ARCH_DMA_H
#define __ASM_ARCH_DMA_H

#include <asm/sizes.h>
#include "hardware.h"

#define dmach_t unsigned int

/*
 * This is the maximum DMA address that can be DMAd to.
 */

#define MAX_DMA_ADDRESS		        0xFFFFFFFF
#define DMA_MEM_REG_OFFSET		0x100

/*
 * Maximum physical DMA buffer size
 */

#define MAX_DMA_SIZE			SZ_16K
#define CUT_DMA_SIZE			SZ_4K

/***********************************************
* The WM3437 has 16 internal DMA channels.
*************************************************/

#define DMA_CHANNELS    16
#define MAX_DMA_CHANNELS DMA_CHANNELS
#define DMA_INT0 IRQ_DMA_CH_0
#define DMA_INT1 IRQ_DMA_CH_1
#define DMA_INT2 IRQ_DMA_CH_2
#define DMA_INT3 IRQ_DMA_CH_3
#define DMA_INT4 IRQ_DMA_CH_4
#define DMA_INT5 IRQ_DMA_CH_5
#define DMA_INT6 IRQ_DMA_CH_6
#define DMA_INT7 IRQ_DMA_CH_7
#define DMA_INT8 IRQ_DMA_CH_8
#define DMA_INT9 IRQ_DMA_CH_9
#define DMA_INT10 IRQ_DMA_CH_10
#define DMA_INT11 IRQ_DMA_CH_11
#define DMA_INT12 IRQ_DMA_CH_12
#define DMA_INT13 IRQ_DMA_CH_13
#define DMA_INT14 IRQ_DMA_CH_14
#define DMA_INT15 IRQ_DMA_CH_15

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
#define SYSTEM_DMA_REQ_EN BIT11

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

/*DMA UPDATE MEMORY REG. BIT*/
#define DMA_UP_MEMREG_EN BIT5

/*****************************************
	DMA descript setting
******************************************/
#define DMA_DES_END	BIT31
#define DMA_FORMAT_DES1	BIT30
#define DMA_INTEN_DES	BIT29
#define DMA_DONE_DES    BIT16
#define DMA_DES0_SIZE 0x8	/*8 byte*/
#define DMA_DES1_SIZE 0x10	/*16 byte*/
#define DMA_DES_REQCNT_MASK 0xFFFF


/**/
/*  I2S CFG setting*/
/**/

#define I2S_RX_SETTING    (DMA_WRAP_MODE | DEVICE_TO_MEM)
#define I2S_TX_SETTING    (DMA_WRAP_MODE)

#define I2S_8BITS_SETTING          (DMA_WRAP_1 | DMA_BURST_8 | DMA_SIZE_8)
#define I2S_16BITS_SETTING         (DMA_WRAP_1 | DMA_BURST_8 | DMA_SIZE_16)
#define I2S_32BITS_SETTING         (DMA_WRAP_1 | DMA_BURST_8 | DMA_SIZE_32)

#define I2S_RX_DMA_8BITS_CFG   (I2S_8BITS_SETTING  | I2S_RX_SETTING)
#define I2S_RX_DMA_16BITS_CFG  (I2S_16BITS_SETTING | I2S_RX_SETTING)
#define I2S_RX_DMA_32BITS_CFG  (I2S_32BITS_SETTING | I2S_RX_SETTING)

#define I2S_TX_DMA_8BITS_CFG   (I2S_8BITS_SETTING  | I2S_TX_SETTING)
#define I2S_TX_DMA_16BITS_CFG  (I2S_16BITS_SETTING | I2S_TX_SETTING)
#define I2S_TX_DMA_32BITS_CFG  (I2S_32BITS_SETTING | I2S_TX_SETTING)

#define I2S_RX_DMA_CFG         (I2S_RX_DMA_32BITS_CFG)
#define I2S_TX_DMA_CFG         (I2S_TX_DMA_32BITS_CFG)

#define I2S_TX_FIFO 0xD80EDB60
#define I2S_RX_FIFO 0xD80EDB00
#define SPDIF_RX_FIFO 0xD80EDB20

/**/
/*  PCM CFG setting*/
/**/

#define PCM_RX_SETTING    (DMA_WRAP_MODE | DEVICE_TO_MEM)
#define PCM_TX_SETTING    (DMA_WRAP_MODE)

#define PCM_8BITS_SETTING          (DMA_WRAP_1 | DMA_BURST_8 | DMA_SIZE_8)
#define PCM_16BITS_SETTING         (DMA_WRAP_1 | DMA_BURST_8 | DMA_SIZE_16)
#define PCM_32BITS_SETTING         (DMA_WRAP_1 | DMA_BURST_8 | DMA_SIZE_32)

#define PCM_RX_DMA_8BITS_CFG   (PCM_8BITS_SETTING  | PCM_RX_SETTING)
#define PCM_RX_DMA_16BITS_CFG  (PCM_16BITS_SETTING | PCM_RX_SETTING)
#define PCM_RX_DMA_32BITS_CFG  (PCM_32BITS_SETTING | PCM_RX_SETTING)

#define PCM_TX_DMA_8BITS_CFG   (PCM_8BITS_SETTING  | PCM_TX_SETTING)
#define PCM_TX_DMA_16BITS_CFG  (PCM_16BITS_SETTING | PCM_TX_SETTING)
#define PCM_TX_DMA_32BITS_CFG  (PCM_32BITS_SETTING | PCM_TX_SETTING)

#define PCM_RX_DMA_CFG         (PCM_RX_DMA_16BITS_CFG)
#define PCM_TX_DMA_CFG         (PCM_TX_DMA_16BITS_CFG)

#define PCM_TX_FIFO (PCM_BASE_ADDR + 0x10)
#define PCM_RX_FIFO (PCM_BASE_ADDR + 0x30)

/**/
/*  AC97 CFG setting*/
/**/
#define AC97_RX_SETTING    (DMA_WRAP_MODE | DEVICE_TO_MEM)
#define AC97_TX_SETTING    (DMA_WRAP_MODE)

#define AC97_8BITS_SETTING          (DMA_WRAP_1 | DMA_BURST_8 | DMA_SIZE_8)
#define AC97_16BITS_SETTING         (DMA_WRAP_1 | DMA_BURST_8 | DMA_SIZE_16)
#define AC97_32BITS_SETTING         (DMA_WRAP_1 | DMA_BURST_8 | DMA_SIZE_32)

#define AC97_MIC_DMA_8BITS_CFG      (AC97_8BITS_SETTING  | AC97_RX_SETTING)
#define AC97_MIC_DMA_16BITS_CFG     (AC97_16BITS_SETTING | AC97_RX_SETTING)

#define AC97_RX_DMA_8BITS_CFG   (AC97_8BITS_SETTING  | AC97_RX_SETTING)
#define AC97_RX_DMA_16BITS_CFG  (AC97_16BITS_SETTING | AC97_RX_SETTING)
#define AC97_RX_DMA_32BITS_CFG  (AC97_32BITS_SETTING | AC97_RX_SETTING)

#define AC97_TX_DMA_8BITS_CFG   (AC97_8BITS_SETTING  | AC97_TX_SETTING)
#define AC97_TX_DMA_16BITS_CFG  (AC97_16BITS_SETTING | AC97_TX_SETTING)
#define AC97_TX_DMA_32BITS_CFG  (AC97_32BITS_SETTING | AC97_TX_SETTING)

#define AC97_MIC_DMA_CFG            (AC97_MIC_DMA_16BITS_CFG)
#define AC97_RX_DMA_CFG         (AC97_RX_DMA_32BITS_CFG)
#define AC97_TX_DMA_CFG         (AC97_TX_DMA_32BITS_CFG)

#define AC97_TX_FIFO 0xD8290080
#define AC97_RX_FIFO 0xD82900C0 
#define AC97_MIC_FIFO 0xD8290100 

/**/
/*  SPI CFG setting*/
/**/

#define SPI_RX_SETTING    (DMA_WRAP_MODE | DEVICE_TO_MEM)
#define SPI_TX_SETTING    (DMA_WRAP_MODE)

#define SPI_8BITS_SETTING          (DMA_WRAP_1 | DMA_BURST_1 | DMA_SIZE_8)
#define SPI_16BITS_SETTING         (DMA_WRAP_1 | DMA_BURST_8 | DMA_SIZE_16)
#define SPI_32BITS_SETTING         (DMA_WRAP_1 | DMA_BURST_8 | DMA_SIZE_32)

#define SPI_RX_DMA_8BITS_CFG   (SPI_8BITS_SETTING  | SPI_RX_SETTING)
#define SPI_RX_DMA_16BITS_CFG  (SPI_16BITS_SETTING | SPI_RX_SETTING)
#define SPI_RX_DMA_32BITS_CFG  (SPI_32BITS_SETTING | SPI_RX_SETTING)

#define SPI_TX_DMA_8BITS_CFG   (SPI_8BITS_SETTING  | SPI_TX_SETTING)
#define SPI_TX_DMA_16BITS_CFG  (SPI_16BITS_SETTING | SPI_TX_SETTING)
#define SPI_TX_DMA_32BITS_CFG  (SPI_32BITS_SETTING | SPI_TX_SETTING)

#define SPI_RX_DMA_CFG         (SPI_RX_DMA_8BITS_CFG)
#define SPI_TX_DMA_CFG         (SPI_TX_DMA_8BITS_CFG)

#define SPI0_TX_FIFO 0xD8240010
#define SPI0_RX_FIFO 0xD8240030 
#define SPI1_TX_FIFO 0xD8250010
#define SPI1_RX_FIFO 0xD8250030 

/**/
/*  UART CFG setting*/
/**/

#define UART_RX_SETTING    (DMA_WRAP_MODE | DEVICE_TO_MEM)
#define UART_TX_SETTING    (DMA_WRAP_MODE)

#define UART_8BITS_SETTING          (DMA_WRAP_1 | DMA_BURST_1 | DMA_SIZE_8)
#define UART_16BITS_SETTING         (DMA_WRAP_1 | DMA_BURST_1 | DMA_SIZE_16)
#define UART_32BITS_SETTING         (DMA_WRAP_1 | DMA_BURST_1 | DMA_SIZE_32)

#define UART_RX_DMA_8BITS_CFG   (UART_8BITS_SETTING  | UART_RX_SETTING)
#define UART_RX_DMA_16BITS_CFG  (UART_16BITS_SETTING | UART_RX_SETTING)
#define UART_RX_DMA_32BITS_CFG  (UART_32BITS_SETTING | UART_RX_SETTING)

#define UART_TX_DMA_8BITS_CFG   (UART_8BITS_SETTING  | UART_TX_SETTING)
#define UART_TX_DMA_16BITS_CFG  (UART_16BITS_SETTING | UART_TX_SETTING)
#define UART_TX_DMA_32BITS_CFG  (UART_32BITS_SETTING | UART_TX_SETTING)

#define UART_RX_DMA_CFG         (UART_RX_DMA_8BITS_CFG)
#define UART_TX_DMA_CFG         (UART_TX_DMA_8BITS_CFG)

#define UART0_TX_FIFO (0x1000+UART0_BASE_ADDR-WMT_MMAP_OFFSET)
#define UART0_RX_FIFO (0x1020+UART0_BASE_ADDR-WMT_MMAP_OFFSET)

#define UART1_TX_FIFO (0x1000+UART1_BASE_ADDR-WMT_MMAP_OFFSET)
#define UART1_RX_FIFO (0x1020+UART1_BASE_ADDR-WMT_MMAP_OFFSET)

#define UART2_TX_FIFO (0x1000+UART2_BASE_ADDR-WMT_MMAP_OFFSET)
#define UART2_RX_FIFO (0x1020+UART2_BASE_ADDR-WMT_MMAP_OFFSET)

#define UART3_TX_FIFO (0x1000+UART3_BASE_ADDR-WMT_MMAP_OFFSET)
#define UART3_RX_FIFO (0x1020+UART3_BASE_ADDR-WMT_MMAP_OFFSET)

#define UART4_TX_FIFO (0x1000+UART4_BASE_ADDR-WMT_MMAP_OFFSET)
#define UART4_RX_FIFO (0x1020+UART4_BASE_ADDR-WMT_MMAP_OFFSET)

#define UART5_TX_FIFO (0x1000+UART5_BASE_ADDR-WMT_MMAP_OFFSET)
#define UART5_RX_FIFO (0x1020+UART5_BASE_ADDR-WMT_MMAP_OFFSET)

/*
 * All possible devices a DMA channel can be attached to.
 */

enum dma_device_e {
	SPI0_DMA_TX_REQ = 0 , /*spi0tx*/
	SPI0_DMA_RX_REQ = 1 , /*spi0rx*/
	SPI1_DMA_TX_REQ = 2 , /*spi1tx*/
	SPI1_DMA_RX_REQ = 3 , /*spi1tx*/
	UART_5_TX_DMA_REQ = 4,/* uart5*/
	UART_5_RX_DMA_REQ = 5,/* uart5*/
	UART_0_TX_DMA_REQ = 6,/* uart0*/
	UART_0_RX_DMA_REQ = 7,/* uart0*/
	UART_1_TX_DMA_REQ = 8,/* uart1*/
	UART_1_RX_DMA_REQ = 9,/* uart1*/
	UART_2_TX_DMA_REQ = 10,/* uart2*/
	UART_2_RX_DMA_REQ = 11,/* uart2*/
	UART_3_TX_DMA_REQ = 12,/* uart3*/
	UART_3_RX_DMA_REQ = 13,/* uart3*/
	PCM_TX_DMA_REQ = 14 ,/* pcm*/
	PCM_RX_DMA_REQ = 15 ,/* pcm*/
	UART_4_TX_DMA_REQ = 16,/* uart4*/
	UART_4_RX_DMA_REQ = 17,/* uart4*/
	AC97_TX_DMA_REQ = 18,/*AC97*/
	AC97_RX_DMA_REQ = 19,/*AC97*/
	AC97_MIC_DMA_REQ = 20, /*AC97 MIC*/
	AHB1_AUD_DMA_REQ_0 = 21,
	AHB1_AUD_DMA_REQ_1 = 22,
	AHB1_AUD_DMA_REQ_2 = 23,
	AHB1_AUD_DMA_REQ_3 = 24,
	AHB1_AUD_DMA_REQ_4 = 25,
	AHB1_AUD_DMA_REQ_5 = 26,
	AHB1_AUD_DMA_REQ_6 = 27,
	AHB1_AUD_DMA_REQ_7 = 28,
	SMARTCARD_TX_DMA_REQ = 29,/* smart card*/
	SMARTCARD_RX_DMA_REQ = 30,/* smart card*/
	MEMORY_DMA_REQ = 32,/* memory*/
	DEVICE_RESERVED = 33 /* reserved*/
};

/*
 * DMA device configuration structure
 * when memory to memory
 *	MIF0addr : source address
 *	MIF1addr : destination address
 * when device to memory or memory to device
 *	MIF0addr : memory address
 *	MIF1addr : device FIFO address
 */

struct dma_device_cfg_s {
	enum dma_device_e DeviceReqType;
	unsigned long DefaultCCR;
	unsigned long MIF0addr;
	unsigned long MIF1addr;
	unsigned int  ChunkSize;
};

/*
* DMA descriptor registers
*/

struct dma_des_fmt0 {
	volatile unsigned long ReqCnt;
	volatile unsigned long DataAddr;
};
struct dma_des_fmt1 {
	volatile unsigned long ReqCnt;
	volatile unsigned long DataAddr;
	volatile unsigned long BrAddr;
	volatile unsigned long reserved;

};
struct dma_descript_addr {
	volatile unsigned long *des_0;
	volatile unsigned long *des_1;
};

/*
* DMA MEMORY REGISTER
*/
struct dma_mem_reg_group_s {
	volatile unsigned long DMA_IF0RBR_CH;
	volatile unsigned long DMA_IF0DAR_CH;
	volatile unsigned long DMA_IF0BAR_CH;
	volatile unsigned long DMA_IF0CPR_CH;
	volatile unsigned long DMA_IF1RBR_CH;
	volatile unsigned long DMA_IF1DAR_CH;
	volatile unsigned long DMA_IF1BAR_CH;
	volatile unsigned long DMA_IF1CPR_CH;
};
struct dma_mem_reg_s {
	struct dma_mem_reg_group_s mem_reg_group[MAX_DMA_CHANNELS];
};

/*
 * DMA control register set structure
 */
struct dma_regs_s {
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

extern struct dma_device_cfg_s dma_device_cfg_table[];         /* DMA device config table */

/*
 * DMA function prototypes
 */

extern
int wmt_request_dma(dmach_t *channel, const char *device_id, enum dma_device_e device,
		void (*callback)(void *data), void *callback_data);
extern
void wmt_free_dma(dmach_t ch);

extern
int wmt_start_dma(dmach_t ch, dma_addr_t dma_ptr, dma_addr_t dma_ptr2, unsigned int size);

extern
void wmt_reset_dma(dmach_t ch);

extern
void wmt_clear_dma(dmach_t ch);

extern
int wmt_setup_dma(dmach_t ch, struct dma_device_cfg_s device_cfg);

extern
void wmt_stop_dma(dmach_t ch);

extern
void wmt_resume_dma(dmach_t ch);

extern
struct dma_mem_reg_group_s wmt_get_dma_pos_info(dmach_t ch);

extern
unsigned int wmt_get_dma_pos(dmach_t ch);

extern
int wmt_dma_busy(dmach_t ch);

extern
void wmt_dump_dma_regs(dmach_t ch);

#endif  /* _ASM_ARCH_DMA_H */
