/*++
	drivers/spi/wmt_spi.h

	Copyright (c) 2008 WonderMedia Technologies, Inc.

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
#ifndef __VT34XX_SPI_H__
#define __VT34XX_SPI_H__

#include <asm/dma.h>
#include <mach/wmt_mmap.h>

/* wmt spi controller configure parameters */
#define SPI_DMA_DISABLE    0x00
#define SPI_DMA_ENABLE     0x01

#define BITS8_PER_WORD_EN  0x01
#define BITS16_PER_WORD_EN 0x10

#define PORT_MODE_PTP      0x01
#define PORT_MODE_MMS      0x00

#define SSN_CTRL_PROGRAM   0x01
#define SSN_CTRL_HARDWARE  0x00

#define BIT_ORDER_LSB      0x00
#define BIT_ORDER_MSB      0x01

#define MAX_SPI_SLAVE      0xFF
#define SPI_FIFO_SIZE      0x20
#define SPI_MIN_FREQ_HZ    (SPI_MAX_FREQ_HZ/0x7FF/2)  //22889hz actually
#define SPI_MAX_FREQ_HZ    (100*1000*1000)            //93.707566Mhz actually
#define SPI_DFLT_FREQ      (5*1000*1000)

/**
 *struct wmt_spi_hw - wmt spi controller config information
 * @dma_support:         spi master can support dma or not
 * @num_chipselect:      how many slaves can support
 * @bits_per_word_en:    bits_per_word type support
 * @port_mode:           port mode, point-to-point or multi-master
 * @ssn_ctrl:            ssn control by program or hardware auto
 * @fifo_size:           spi tx and rx fifo size
 * @max_transfer_length  max data length in a spi transfer action
 * @min_freq_hz:         min spi frequence can be supported
 * @max_freq_hz:         max spi frenquece can be supported
 **/
 struct wmt_spi_hw {
    u8  dma_support;
    u8  num_chipselect;
    u8  bits_per_word_en;
    u8  port_mode;
    u8  ssn_ctrl;
    u8  fifo_size;
    u16 max_transfer_length;
    u32 min_freq_hz;
    u32 max_freq_hz;
};

/**
 * struct wmt_spi_slave - wmt spi slave config infomatin
 * @dma_en:        transfer use dma or not 
 * @bits_per_word: bit_per_word this chip only can work with
 */
struct wmt_spi_slave {
    u8  dma_en;
    u8  bits_per_word;
};

/**
 *struct wmt_spi  - wmt spi controller driver data
 */
struct wmt_spi {
    struct platform_device *pdev;  /* Driver model hookup     */
    struct spi_master *master;     /* SPI framework hookup    */
    void __iomem *regs_base;       /* SPI regs base of WMT*/
    int irq;                       /* SPI IRQ number          */

    spinlock_t spinlock;           /* Prevent multi user confiliciton */
    struct workqueue_struct *workqueue;
    struct work_struct work;
    struct list_head queue;
    wait_queue_head_t waitq;
    
    struct wmt_spi_hw *spi_hw_info;
    struct wmt_spi_dma *spi_dma_info;
};


struct wmt_spi_dma {
    unsigned int  rx_ch;
    unsigned int  tx_ch;
    dma_addr_t    phys_raddr;
    dma_addr_t    phys_waddr;
    u8 *io_raddr;
    u8 *io_waddr;
	wait_queue_head_t rx_event;
	volatile int rx_ack;
	wait_queue_head_t tx_event;
	volatile int tx_ack;
	struct dma_device_cfg_s tx_config;
	struct dma_device_cfg_s rx_config;
};

#define SPI_CLK_MODE0   0x00
#define SPI_CLK_MODE1   0x01
#define SPI_CLK_MODE2   0x02
#define SPI_CLK_MODE3   0x03

#define SPI_OP_POLLING  0x00
#define SPI_OP_DMA      0x01
#define SPI_OP_IRQ      0x02

#define POLLING_SPI_REG_TIMEOUT   0x20000

#define GPIO_SPI0_CLK_PULL_EN	BIT0
#define GPIO_SPI0_SS_PULL_EN	BIT3
#define GPIO_SPI0_MISO_PULL_EN	BIT1
#define GPIO_SPI0_MOSI_PULL_EN	BIT2
#define GPIO_SPI0_CLK_PULL_UP	BIT0
#define GPIO_SPI0_SS_PULL_UP	BIT3
#define GPIO_SPI0_MISO_PULL_UP  BIT1
#define GPIO_SPI0_MOSI_PULL_UP	BIT2

#define GPIO_SPI0_CLK	BIT3
#define GPIO_SPI0_SS	BIT2
#define GPIO_SPI0_MISO	BIT1
#define GPIO_SPI0_MOSI	BIT0
#define GPIO_SPI0_SSB   BIT4

/* SPI register setting related */
#define PMC_REG_BASE            PM_CTRL_BASE_ADDR

#define SPI_CLK_DIV_VAL         0x04  
#define PLL_25MHZ               25000
#define PLL_B_MULTI_RANGE_REG   0x0204
#define SPI_CLK_ENABLE_REG      0x0250
#define SPI_CLK_DIV_REG         0x033C

#define SPI_CR     0X00  /* SPI Control Register Offset             */
#define SPI_SR     0X04  /* SPI Status Register Offset              */
#define SPI_DFCR   0X08  /* SPI Data Format Control Register Offset */
#define SPI_CRE    0X0C
#define SPI_TXFIFO 0X10  /* SPI TX FIFO Offset                      */
#define SPI_RXFIFO 0X30  /* SPI RX FIFO Offset                      */

/************ Control Register ************/
/* Transmit Clock Driver*/
#define SPI_CR_TCD_SHIFT    21
#define SPI_CR_TCD_MASK     (BIT31|BIT30|BIT29|BIT28|BIT27|BIT26|BIT25|BIT24|BIT23|BIT22|BIT21)
/* Slave Selection*/
#define SPI_CR_SS_SHIFT     19
#define SPI_CR_SS_MASK      (BIT20|BIT19)
/* Transmit FIFO Byte Write Method*/
#define SPI_CR_WM_SHIFT     18
#define SPI_CR_WM_MASK      (BIT18)
/* Receive FIFO Reset*/
#define SPI_CR_RFR_SHIFT    17
#define SPI_CR_RFR_MASK     (BIT17)
/* Transmit FIFO Reset*/
#define SPI_CR_TFR_SHIFT    16
#define SPI_CR_TFR_MASK     (BIT16)
/* DMA Request Control*/
#define SPI_CR_DRC_SHIFT    15
#define SPI_CR_DRC_MASK     (BIT15)
/* Receive FIFO Threshold Selection*/
#define SPI_CR_RFTS_SHIFT   14
#define SPI_CR_RFTS_MASK    (BIT14)
/* Transmit FIFO Threshold Selection*/
#define SPI_CR_TFTS_SHIFT   13
#define SPI_CR_TFTS_MASK    (BIT13)
/* Transmit FIFO Under-run Interrupt*/
#define SPI_CR_TFUI_SHIFT   12
#define SPI_CR_TFUI_MASK    (BIT12)
/* Transmit FIFO Empty Interrupt*/
#define SPI_CR_TFEI_SHIFT   11
#define SPI_CR_TFEI_MASK    (BIT11)
/* Receive FIFO Over-run Interrupt*/
#define SPI_CR_RFOI_SHIFT   10
#define SPI_CR_RFOI_MASK    (BIT10)
/* Receive FIFO Full Interrupt*/
#define SPI_CR_RFFI_SHIFT   9
#define SPI_CR_RFFI_MASK    (BIT9)
/* Receive FIFO Empty Interrupt*/
#define SPI_CR_RFEI_SHIFT   8
#define SPI_CR_RFEI_MASK    (BIT8)
/* Threshold IRQ/DMA Selection*/
#define SPI_CR_TIDS_SHIFT   7
#define SPI_CR_TIDS_MASK    (BIT7)
/* Interrupt Enable*/
#define SPI_CR_IE_SHIFT     6
#define SPI_CR_IE_MASK      (BIT6)
/* Module Enable*/
#define SPI_CR_ME_SHIFT     5
#define SPI_CR_ME_MASK      (BIT5)
/* Module Fault Error Interrupt*/
#define SPI_CR_MFEI_SHIFT   4
#define SPI_CR_MFEI_MASK    (BIT4)
/* Master/Slave Mode Select*/
#define SPI_CR_MSMS_SHIFT   3
#define SPI_CR_MSMS_MASK    (BIT3)
/* Clock Polarity Select*/
#define SPI_CR_CPS_SHIFT    2
#define SPI_CR_CPS_MASK     (BIT2)
/* Clock Phase Select*/
#define SPI_CR_CPHS_SHIFT   1
#define SPI_CR_CPHS_MASK    (BIT1)
/* Module Fault Error Feature*/
#define SPI_CR_MFEF_SHIFT   0
#define SPI_CR_MFEF_MASK    (BIT0)
/* SPI Control Register Reset Value*/
#define SPI_CR_RESET_MASK   SPI_CR_MSMS_MASK

/************ Status Register *************/
/* RX FIFO Count*/
#define SPI_SR_RFCNT_SHIFT    24
#define SPI_SR_RFCNT_MASK   (BIT31|BIT30|BIT29|BIT28|BIT27|BIT26|BIT25|BIT24)
/* TX FIFO Count*/
#define SPI_SR_TFCNT_SHIFT    16
#define SPI_SR_TFCNT_MASK   (BIT23|BIT22|BIT21|BIT20|BIT19|BIT18|BIT17|BIT16)
/* TX FIFO Empty Status*/
#define SPI_SR_TFES_SHIFT   15
#define SPI_SR_TFES_MASK    (BIT15)
/* Receive FIFO Threshold Passed Interrupt*/
#define SPI_SR_RFTPI_SHIFT    14
#define SPI_SR_RFTPI_MASK   (BIT14)
/* Transmit FIFO Threshold Passed Interrupt*/
#define SPI_SR_TFTPI_SHIFT    13
#define SPI_SR_TFTPI_MASK   (BIT13)
/* Transmit FIFO Under-run Interrupt*/
#define SPI_SR_TFUI_SHIFT   12
#define SPI_SR_TFUI_MASK    (BIT12)
/* Transmit FIFO Empty Interrupt*/
#define SPI_SR_TFEI_SHIFT   11
#define SPI_SR_TFEI_MASK    (BIT11)
/* Receive FIFO Over-run Interrupt*/
#define SPI_SR_RFOI_SHIFT   10
#define SPI_SR_RFOI_MASK    (BIT10)
/* Receive FIFO Full Interrupt*/
#define SPI_SR_RFFI_SHIFT   9
#define SPI_SR_RFFI_MASK    (BIT9)
/* Receive FIFO Empty Interrupt*/
#define SPI_SR_RFEI_SHIFT   8
#define SPI_SR_RFEI_MASK    (BIT8)
/* SPI Busy*/
#define SPI_SR_BUSY_SHIFT   7
#define SPI_SR_BUSY_MASK    (BIT7)
/* Mode Fault Error Interrupt*/
#define SPI_SR_MFEI_SHIFT   4
#define SPI_SR_MFEI_MASK    (BIT4)

/****** Data Format Control Register ******/
/*Preset Counter*/
#define SPI_SSN_PRE_COUNTER_SHIFT 28
#define SPI_SSN_PRE_COUNTER_MASK (BIT31|BIT30|BIT29|BIT28)
/*HOLD EN*/
#define SPI_SSN_HOLD_EN BIT26
/*Microwire EN*/
#define SPI_MICROWIRE_EN BIT25
/*RX theshold Pass Interrupt Enable*/
#define SPI_RX_THESHOLD_INT_EN BIT24
/* Mode Fault Delay Count*/
#define SPI_DFCR_MFDCNT_SHIFT 16
#define SPI_DFCR_MFDCNT_MASK  (BIT23|BIT22|BIT21|BIT20|BIT19|BIT18|BIT17|BIT16)
/* TX Drive Count*/
#define SPI_DFCR_TDCNT_SHIFT  8
#define SPI_DFCR_TDCNT_MASK   (BIT15|BIT14|BIT13|BIT12|BIT11|BIT10|BIT9|BIT8)
/* TX Drive Enable*/
#define SPI_DFCR_TDE_SHIFT    7
#define SPI_DFCR_TDE_MASK   (BIT7)
/* TX No Data Value*/
#define SPI_DFCR_TNDV_SHIFT   6
#define SPI_DFCR_TNDV_MASK    (BIT6)
/* Direct SSN Enable*/
#define SPI_DFCR_DSE_SHIFT    5
#define SPI_DFCR_DSE_MASK   (BIT5)
/* Direct SSN Value*/
#define SPI_DFCR_DSV_SHIFT    4
#define SPI_DFCR_DSV_MASK   (BIT4)
/* SSN Control*/
#define SPI_DFCR_SC_SHIFT   3
#define SPI_DFCR_SC_MASK    (BIT3)
/* SSN Port Mode*/
#define SPI_DFCR_SPM_SHIFT    2
#define SPI_DFCR_SPM_MASK   (BIT2)
/* Receive Significant Bit Order*/
#define SPI_DFCR_RSBO_SHIFT   1
#define SPI_DFCR_RSBO_MASK    (BIT1)
/* Transmit Significant Bit Order*/
#define SPI_DFCR_TSBO_SHIFT   0
#define SPI_DFCR_TSBO_MASK    (BIT0)
/* SPI Data Format Control Register Reset Value*/
#define SPI_DFCR_RESET_MASK   (SPI_DFCR_DSV_MASK|SPI_DFCR_DSE_MASK)


/* spi dma related */
#define SPI_DMA_CHUNK_SIZE          1
#define SPI_MAX_TRANSFER_LENGTH     (4*1024)
#endif /* __VT34XX_SPI_H */
