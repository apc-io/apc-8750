/*++
	drivers/spi/wmt-spi.h

	Some descriptions of such software. Copyright (c) 2008  WonderMedia Technologies, Inc.

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

	History:
		The code was inherit from vt8430
		2009/03/04 First Version
--*/
#include <linux/config.h>
#ifndef WMT_SPI_H
/* To assert that only one occurrence is included */
#define WMT_SPI_H

#include <linux/config.h>
#include <linux/types.h>
#include <linux/module.h>
/* Include your headers here*/

#include <asm/dma.h>
#include <asm/sizes.h>

#define PLL_25MHZ		25000  /*it is forced value: 25 MHz = 25000 KHz*/
#define PLL_27MHZ		27000  /*it is forced value: 27 MHz = 27000 KHz*/
#define START_DMA(a, b, c)		wmt_start_dma(a, b, 0, c)
#define SETUP_DMA(a, b)			wmt_setup_dma(a, b)
#define	REQUEST_DMA(a, b, c, d, e)	wmt_request_dma(a, b, c, d, e)
#define FREE_DMA(a)				wmt_free_dma(a)

/************ PMC Register Address ************/
#define WMT_PMC_BASE             0xD8130000

#define SPI0_CLOCK_DIVISOR         (WMT_PMC_BASE + 0x33C)
#define SPI1_CLOCK_DIVISOR         (WMT_PMC_BASE + 0x340)
#define SPI2_CLOCK_DIVISOR         (WMT_PMC_BASE + 0x344)

#define PLLB_MULTIPLIER            (WMT_PMC_BASE + 0x204)

/************ Bit Definiton for WMT_PMC Clock Enables Lower Register ************/
#define SPI0CLKEN                   BIT12
#define SPI1CLKEN                   BIT13
#define SPI2CLKEN                   0

/************ IRQ/DMA MACRO Define ************/
#define	SPI_PORT1_IRQ			IRQ_SPI1	/* SPI PORT 1, IRQ Number*/
#define	SPI_PORT1_RX_DMA_REQ	APB_SPI_1_RX_REQ	/* SPI PORT 1, RX DMA ID*/
#define	SPI_PORT1_TX_DMA_REQ	APB_SPI_1_TX_REQ	/* SPI PORT 1, TX DMA ID*/

#define SPI_PORT_NUM				1	/* The number of SPI port*/


/************ Register Address ************/
#define SPI_REG_BASE			0xD8240000	/* SPI PORT 0 Base Address*/
#define SPI1_REG_BASE			0xD8250000	/* SPI PORT 1 Base Address*/
#define SPI2_REG_BASE			0xD82A0000	/* SPI PORT 2 Base Address*/
#define SPI1_PORT_OFFSET	       0x10000
#define SPI2_PORT_OFFSET            0x60000
#define SPICR			0x00	/* SPI Control Register Offset Base on PORT Base Address*/
#define SPISR			0x04	/* SPI Status Register Offset Base on PORT Base Address*/
#define SPIDFCR			0x08	/* SPI Data Format Control Register Offset Base on PORT Base Address*/
#define SPICRE			0x0C
#define SPITXFIFO		0x10	/* SPI TX FIFO Offset Base on PORT Base Address*/
#define SPIRXFIFO		0x30	/* SPI RX FIFO Offset Base on PORT Base Address*/

/************ Control Register ************/
/* Transmit Clock Driver*/
#define SPI_CR_TCD_SHIFT		21
#define SPI_CR_TCD_MASK			(BIT31|BIT30|BIT29|BIT28|BIT27|BIT26|BIT25|BIT24|BIT23|BIT22|BIT21)
/* Slave Selection*/
#define SPI_CR_SS_SHIFT			19
#define SPI_CR_SS_MASK			(BIT20|BIT19)
/* Transmit FIFO Byte Write Method*/
#define SPI_CR_WM_SHIFT			18
#define SPI_CR_WM_MASK			(BIT18)
/* Receive FIFO Reset*/
#define SPI_CR_RFR_SHIFT		17
#define SPI_CR_RFR_MASK			(BIT17)
/* Transmit FIFO Reset*/
#define SPI_CR_TFR_SHIFT		16
#define SPI_CR_TFR_MASK			(BIT16)
/* DMA Request Control*/
#define SPI_CR_DRC_SHIFT		15
#define SPI_CR_DRC_MASK			(BIT15)
/* Receive FIFO Threshold Selection*/
#define SPI_CR_RFTS_SHIFT		14
#define SPI_CR_RFTS_MASK		(BIT14)
/* Transmit FIFO Threshold Selection*/
#define SPI_CR_TFTS_SHIFT		13
#define SPI_CR_TFTS_MASK		(BIT13)
/* Transmit FIFO Under-run Interrupt*/
#define SPI_CR_TFUI_SHIFT		12
#define SPI_CR_TFUI_MASK		(BIT12)
/* Transmit FIFO Empty Interrupt*/
#define SPI_CR_TFEI_SHIFT		11
#define SPI_CR_TFEI_MASK		(BIT11)
/* Receive FIFO Over-run Interrupt*/
#define SPI_CR_RFOI_SHIFT		10
#define SPI_CR_RFOI_MASK		(BIT10)
/* Receive FIFO Full Interrupt*/
#define SPI_CR_RFFI_SHIFT		9
#define SPI_CR_RFFI_MASK		(BIT9)
/* Receive FIFO Empty Interrupt*/
#define SPI_CR_RFEI_SHIFT		8
#define SPI_CR_RFEI_MASK		(BIT8)
/* Threshold IRQ/DMA Selection*/
#define SPI_CR_TIDS_SHIFT		7
#define SPI_CR_TIDS_MASK		(BIT7)
/* Interrupt Enable*/
#define SPI_CR_IE_SHIFT			6
#define SPI_CR_IE_MASK			(BIT6)
/* Module Enable*/
#define SPI_CR_ME_SHIFT			5
#define SPI_CR_ME_MASK			(BIT5)
/* Module Fault Error Interrupt*/
#define SPI_CR_MFEI_SHIFT		4
#define SPI_CR_MFEI_MASK		(BIT4)
/* Master/Slave Mode Select*/
#define SPI_CR_MSMS_SHIFT		3
#define SPI_CR_MSMS_MASK		(BIT3)
/* Clock Polarity Select*/
#define SPI_CR_CPS_SHIFT		2
#define SPI_CR_CPS_MASK			(BIT2)
/* Clock Phase Select*/
#define SPI_CR_CPHS_SHIFT		1
#define SPI_CR_CPHS_MASK		(BIT1)
/* Module Fault Error Feature*/
#define SPI_CR_MFEF_SHIFT		0
#define SPI_CR_MFEF_MASK		(BIT0)
/* SPI Control Register Reset Value*/
#define SPI_CR_RESET_MASK		SPI_CR_MSMS_MASK

/************ Status Register *************/
/* RX FIFO Count*/
#define SPI_SR_RFCNT_SHIFT		24
#define SPI_SR_RFCNT_MASK		(BIT31|BIT30|BIT29|BIT28|BIT27|BIT26|BIT25|BIT24)
/* TX FIFO Count*/
#define SPI_SR_TFCNT_SHIFT		16
#define SPI_SR_TFCNT_MASK		(BIT23|BIT22|BIT21|BIT20|BIT19|BIT18|BIT17|BIT16)
/* TX FIFO Empty Status*/
#define SPI_SR_TFES_SHIFT		15
#define SPI_SR_TFES_MASK		(BIT15)
/* Receive FIFO Threshold Passed Interrupt*/
#define SPI_SR_RFTPI_SHIFT		14
#define SPI_SR_RFTPI_MASK		(BIT14)
/* Transmit FIFO Threshold Passed Interrupt*/
#define SPI_SR_TFTPI_SHIFT		13
#define SPI_SR_TFTPI_MASK		(BIT13)
/* Transmit FIFO Under-run Interrupt*/
#define SPI_SR_TFUI_SHIFT		12
#define SPI_SR_TFUI_MASK		(BIT12)
/* Transmit FIFO Empty Interrupt*/
#define SPI_SR_TFEI_SHIFT		11
#define SPI_SR_TFEI_MASK		(BIT11)
/* Receive FIFO Over-run Interrupt*/
#define SPI_SR_RFOI_SHIFT		10
#define SPI_SR_RFOI_MASK		(BIT10)
/* Receive FIFO Full Interrupt*/
#define SPI_SR_RFFI_SHIFT		9
#define SPI_SR_RFFI_MASK		(BIT9)
/* Receive FIFO Empty Interrupt*/
#define SPI_SR_RFEI_SHIFT		8
#define SPI_SR_RFEI_MASK		(BIT8)
/* SPI Busy*/
#define SPI_SR_BUSY_SHIFT		7
#define SPI_SR_BUSY_MASK		(BIT7)
/* Mode Fault Error Interrupt*/
#define SPI_SR_MFEI_SHIFT		4
#define SPI_SR_MFEI_MASK		(BIT4)

/****** Data Format Control Register ******/
/* Mode Fault Delay Count*/
#define SPI_DFCR_MFDCNT_SHIFT	16
#define SPI_DFCR_MFDCNT_MASK	(BIT23|BIT22|BIT21|BIT20|BIT19|BIT18|BIT17|BIT16)
/* TX Drive Count*/
#define SPI_DFCR_TDCNT_SHIFT	8
#define SPI_DFCR_TDCNT_MASK		(BIT15|BIT14|BIT13|BIT12|BIT11|BIT10|BIT9|BIT8)
/* TX Drive Enable*/
#define SPI_DFCR_TDE_SHIFT		7
#define SPI_DFCR_TDE_MASK		(BIT7)
/* TX No Data Value*/
#define SPI_DFCR_TNDV_SHIFT		6
#define SPI_DFCR_TNDV_MASK		(BIT6)
/* Direct SSN Enable*/
#define SPI_DFCR_DSE_SHIFT		5
#define SPI_DFCR_DSE_MASK		(BIT5)
/* Direct SSN Value*/
#define SPI_DFCR_DSV_SHIFT		4
#define SPI_DFCR_DSV_MASK		(BIT4)
/* SSN Control*/
#define SPI_DFCR_SC_SHIFT		3
#define SPI_DFCR_SC_MASK		(BIT3)
/* SSN Port Mode*/
#define SPI_DFCR_SPM_SHIFT		2
#define SPI_DFCR_SPM_MASK		(BIT2)
/* Receive Significant Bit Order*/
#define SPI_DFCR_RSBO_SHIFT		1
#define SPI_DFCR_RSBO_MASK		(BIT1)
/* Transmit Significant Bit Order*/
#define SPI_DFCR_TSBO_SHIFT		0
#define SPI_DFCR_TSBO_MASK		(BIT0)
/* SPI Data Format Control Register Reset Value*/
#define SPI_DFCR_RESET_MASK		(SPI_DFCR_DSV_MASK|SPI_DFCR_DSE_MASK)

/****** SW Enumeration Define ******/
/* Clock Mode Define*/
#define	SPI_CLK_MODE_LIST						\
	T(SPI_MODE_0),	/*  0, CLK Idles Low  + SS  activation */	\
	T(SPI_MODE_1),	/*  1, CLK Idles Low  + CLK activation */	\
	T(SPI_MODE_2),	/*  2, CLK Idles High + SS  activation */	\
	T(SPI_MODE_3),	/*  3, CLK Idles High + CLK activation */	\
	T(SPI_MODE_MAX)	/*  4 */

/* Bus Master Define*/
#define	SPI_ARBITER_LIST						\
	T(SPI_ARBITER_MASTER),	/*  0 */	\
	T(SPI_ARBITER_SLAVE),	/*	1 */	\
	T(SPI_ARBITER_MAX)	/*	2 */

/* Bus Operate Mode Define*/
#define	SPI_OP_MODE_LIST						\
	T(SPI_POLLING_MODE),	/*  0 */	\
	T(SPI_INTERRUPT_MODE),	/*  1 */	\
	T(SPI_DMA_MODE),	/*  2 */	\
	T(SPI_OP_MAX)	/*  3 */

/* Bus Port Mode Define*/
#define	SPI_SSN_PORT_MODE_LIST						\
	T(SPI_SSN_PORT_MM),	/*  0, Multi-Master */	\
	T(SPI_SSN_PORT_PTP),	/*  1, Point To Point */	\
	T(SPI_SSN_PORT_MAX)	/*  2 */

/* SPI Chip Select Define*/
#define	SPI_SSN_LIST		\
	T(SPI_SS0),	 /*  0 */	\
	T(SPI_SS1),	 /*  1 */	\
	T(SPI_SS2),	/*  2 */	\
	T(SPI_SS3),	/*  3  */	\
	T(SPI_SSN_MAX)	/*  4 */

/*FIFO Threshold Define*/
#define	SPI_FIFO_THRESHOLD_LIST						\
	T(SPI_THRESHOLD_16BYTES	),	/*  0 */	\
	T(SPI_THRESHOLD_8BYTES),	/*	1 */	\
	T(SPI_THRESHOLD_MAX)	/*	2 */


/* [Rx1C] GPIO Control Register for SPI */
#define GPIO_SPI0_SSB	BIT4
#define GPIO_SPI0_CLK	BIT3
#define GPIO_SPI0_SS	BIT2
#define GPIO_SPI0_MISO	BIT1
#define GPIO_SPI0_MOSI	BIT0
/* [Rx48B] GPIO PULL EN for SPI */
#define GPIO_SPI0_CLK_PULL_EN	 BIT0
#define GPIO_SPI0_SS_PULL_EN	 BIT3
#define GPIO_SPI0_MISO_PULL_EN	 BIT1
#define GPIO_SPI0_MOSI_PULL_EN	 BIT2
/* [Rx48B] GPIO PULL UP/DOWN for SPI */
#define GPIO_SPI0_CLK_PULL_UP	 BIT0
#define GPIO_SPI0_SS_PULL_UP	 BIT3
#define GPIO_SPI0_MISO_PULL_UP BIT1
#define GPIO_SPI0_MOSI_PULL_UP	 BIT2

/*-------------------- EXPORTED PRIVATE TYPES---------------------------------*/
/* typedef  void  spi_xxx_t;  *//*Example*/

#define T(x) x
enum SPI_TyClkMode_e{
	SPI_CLK_MODE_LIST
};

enum SPI_TyArbiter_e{
	SPI_ARBITER_LIST
};

enum SPI_TyOpMode_e{
	SPI_OP_MODE_LIST
};

enum SPI_TySSnPortMode_e{
	SPI_SSN_PORT_MODE_LIST
};

enum SPI_TySSn_e{
	SPI_SSN_LIST
};

enum SPI_TyFIFOTHRESHOLD_e{
	SPI_FIFO_THRESHOLD_LIST
};

struct spi_reg_s{
	unsigned int volatile *cr;	/* Control Register*/
	unsigned int volatile *sr;	/* Status Register*/
	unsigned int volatile *dfcr;	/* Data Format Control Register*/
	unsigned int volatile *cre; /*Extended Control Register*/
	unsigned char volatile *rfifo;	/* Read FIFO, i.e. Receive FIFO*/
	unsigned char volatile *wfifo;	/* Write FIFO, i.e. Transfer FIFO*/
};

struct spi_dma_s{
	struct dma_device_cfg_s config;
	dmach_t dmach;
	unsigned int xfer_cnt;				/* The number of bytes transfers by DMA*/
	wait_queue_head_t event_queue;		/* Event queue for controling dma complete*/
	volatile int event_ack;				/* Event ack for notifing dma complete*/
	spinlock_t spinlock;				/* Spin Lock*/
	/* void (*dsr)(void *spi_port);		// DMA service routine*/
};

struct spi_port_s{
	int port;							/* SPI port number*/

	struct semaphore port_sem;			/* Prevent from multi user confiliciton*/

	wait_queue_head_t tx_event_queue;	/* Event queue for controling write out complete*/
	wait_queue_head_t rx_event_queue;	/* Event queue for controling read in complete*/
	volatile int tx_event_ack;			/* Event ack for notifing dma complete*/
	volatile int rx_event_ack;			/* Event ack for notifing dma complete*/

	unsigned int cur_tx_cnt;			/* Current write out bytes*/
	unsigned int cur_rx_cnt;			/* Current read in bytes*/

	struct spi_user_rec_s *user;			/* SPI user*/

	struct spi_reg_s regs;				/* SPI register set*/

	/* interrup and dma control*/
	struct spi_dma_s rdma;				/* Read in DMA information*/
	struct spi_dma_s wdma;				/* Write out DMA information*/

	int	irq_num;				/* IRQ number*/

};

struct spi_user_rec_s{
	unsigned char name[9];
	struct spi_port_s *spi_port;

	dma_addr_t phys_addr_r;				/* I/O data physical address for dma read in*/
	dma_addr_t phys_addr_w;				/* I/O data physical address for dma write out*/
	unsigned char *ior;					/* I/O data buffer for dma read in*/
	unsigned char *iow;					/* I/O data buffer for dma write out*/
	unsigned char dma_init;				/* Flag for recording DMA hardware/resource initial completed*/
	enum SPI_TyFIFOTHRESHOLD_e tx_fifo_threshold;        /* Set TX FIFO threshold*/
	enum SPI_TyFIFOTHRESHOLD_e rx_fifo_threshold;       /*  Set RX FIFO threshold*/

	unsigned char *rbuf;				/* I/O data buffer for polling read in*/
	unsigned char *wbuf;				/* I/O data buffer for polling write out*/

	unsigned int freq;					/* Bus frequence, Unit Khz*/
	enum SPI_TyClkMode_e clk_mode;				/* Bus clock mode*/
	enum SPI_TyOpMode_e op_mode;				/* Operation mode, polling/irq/dma*/
	/* Use for output setting value when FIFO goes empty*/
	unsigned char tx_drive_count;		/* The number of bytes to send out in TX drive function*/
	unsigned char tx_drive_enable;		/* Enable/Disable TX drive*/
	unsigned char tx_nodata_value;		/* Output value, o => 0x0, 1 =>0xff*/
	/* user for master and slave control*/
	enum SPI_TyArbiter_e arbiter;
	enum SPI_TySSnPortMode_e ssn_port_mode;	/* SSn port mode. 0 => multi-master, 1 => P2P*/
	enum SPI_TySSn_e slave_select;			/* Output SSn pin {a,b,c,d}*/

	unsigned int rx_cnt;				/* The number of byte receives*/
	unsigned int tx_cnt;				/* The number of byte transfers*/

	void (*callback)(unsigned char *);
};

/*---------------------------------------------------------------------------------------*/
/* GPIO Public functions declaration*/
/*---------------------------------------------------------------------------------------*/


/*--------------------- EXPORTED PRIVATE MACROS -------------------------------*/


/*--------------------- EXPORTED PRIVATE FUNCTIONS  ---------------------------*/
/* extern void  spi_xxxx(vdp_Void); *//*Example*/
/*following comment by howay*/
/*register user id and unregister user id*/
/************************************************************************
function:     register_user()
purpose:     register a user. A SPI has a chipselect
		So at most register a user for a SPI
Parameter: name: user name string, max length: 8 char
		portIndex-- 0: SPI0, 1:SPI1, 2:SPI2
return:       a pointer that  point to a memory which store the user's information
		NULL -- fail
		>=0  -- success
************************************************************************/
extern struct spi_user_rec_s *register_user(char *name, int portIndex);


/************************************************************************
function:    unregister_user()
purpose:    unregister a user.
parameter:  spi_user - a pointer that  point to a memory which store the user's information
		   portIndex-- 0: SPI0, 1:SPI1, 2:SPI2
return:      0 - successful;  -1: fail
************************************************************************/
extern int unregister_user(struct spi_user_rec_s *spi_user, int portIndex);


/*use follow function to set SPI Control Register*/
/************************************************************************
function: spi_set_freq()
purpose: set spi freq,
/------------------------------------------------------------------------
supported SPI freq (fspi) is calculated as follows:
    fspi_in=25MHz * PLL_B_multiplier/PMC_spi_divisor
    fspi  = fspi_in/(2 * Transmit Clock Divider)

   Transmit Clock Divider -> SPI control register Bit31~Bit21
   If  Transmit Clock divider = 0, then fspi = fspi_in.
/------------------------------------------------------------------------
parameter: spi_user - a pointer that  point to a memory which store the user's information
	freq -- frquence which you want
return:   None
************************************************************************/
extern void spi_set_freq(struct spi_user_rec_s *spi_user, int freq);


/***************************************************************************
function: spi_set_clk_mode()
purpose: set (polarity, phase)
/-------------------------------------------------------------------------
SPI has 4 modes:
     Mode              (polarity, phase)
    mode 0  ------     (0,0) every data rising edge sampled
    mode 1  ------     (0,1)
    mode 2  ------     (1,0)
    mode 3  ------     (1,1) every data rising edge sampled
/--------------------------------------------------------------
parameter:
  spi_user - a pointer that  point to a memory which store the user's information
  clk_mode -- only can be SPI_MODE_0,SPI_MODE_1,SPI_MODE_2,SPI_MODE_3
****************************************************************************/
extern void spi_set_clk_mode(struct spi_user_rec_s *spi_user, enum SPI_TyClkMode_e clk_mode);


/****************************************************************************
function: spi_set_op_mode()
purpose: set operation mode to polling mode, DMA mode or interrupt mode
parameter:
   spi_user - a pointer that  point to a memory which store the user's information
   op_mode - only can be SPI_POLLING_MODE, SPI_INTERRUPT_MODE or SPI_DMA_MODE
****************************************************************************/
extern void spi_set_op_mode(struct spi_user_rec_s *spi_user, enum SPI_TyOpMode_e op_mode);


/****************************************************************************
function: spi_set_arbiter()
purpose: set SPI as master or slaver
parameter:
   spi_user - a pointer that  point to a memory which store the user's information
   arbiter - only can be SPI_ARBITER_MASTER, SPI_ARBITER_SLAVE
****************************************************************************/
extern void spi_set_arbiter(struct spi_user_rec_s *spi_user, enum SPI_TyArbiter_e arbiter);


/****************************************************************************
function: spi_set_port_mode()
purpose: set SPI work in Multi-Master or Point To Point environment
parameter:
   spi_user - a pointer that  point to a memory which store the user's information
   ssn_port_mode - only can be SPI_SSN_PORT_MM, SPI_SSN_PORT_PTP
****************************************************************************/
extern void spi_set_port_mode(struct spi_user_rec_s *spi_user, enum SPI_TySSnPortMode_e ssn_port_mode);


/****************************************************************************
function: spi_set_chipselect()
purpose: every SPI has 4 chip select (SPISS0, SPISS1, SPISS2, SPISS3)
		please select a chipselect which you want to use
parameter:
	spi_user - a pointer that  point to a memory which store the user's information
	spi_ssn --only can be SPI_SS0,SPI_SS1, SPI_SS2 or SPI_SS3
****************************************************************************/
extern void spi_set_chipselect(struct spi_user_rec_s *spi_user, enum SPI_TySSn_e  spi_ssn);


/****************************************************************************
function:  spi_set_tx_fifo_threshold()
purpose:  this function only use in dma mode or interrupt mode, don't use to polling mode.
		set a threshold, SPI controller judge available space >= threshold in SPI TX FIFO,
		it trigger DMA request or Interrupt to send the data in SPI TX FIFO
		threshold only can be 8 bytes or 16 bytes
parameter:
	spi_user - a pointer that  point to a memory which store the user's information
	spi_threshold - only can be SPI_THRESHOLD_16BYTES or SPI_THRESHOLD_8BYTES
****************************************************************************/
extern void spi_set_tx_fifo_threshold(struct spi_user_rec_s *spi_user,
					enum SPI_TyFIFOTHRESHOLD_e  spi_threshold);


/****************************************************************************
function:  spi_set_rx_fifo_threshold()
purpose:  this function only use in dma mode or interrupt mode, don't use to polling mode.
	set a threshold, SPI controller judge the data number >= thresholdin SPI RX FIFO,
	it trigger DMA request or Interrupt to receive data
	threshold only can be 8 bytes or 16 bytes
parameter:
	spi_user - a pointer that  point to a memory which store the user's information
	spi_threshold - only can be SPI_THRESHOLD_16BYTES or SPI_THRESHOLD_8BYTES
****************************************************************************/
extern void spi_set_rx_fifo_threshold(struct spi_user_rec_s *spi_user,
					enum SPI_TyFIFOTHRESHOLD_e  spi_threshold);


/****************************************************************************
function: spi_set_reg()
purpose: After set frequence, set clk mode, set chipslect ... All change about SPI Control Register,
	Please Remeber call this function to really write them to SPI Control Register
parameter:
	spi_user - a pointer that  point to a memory which store the user's information
return:
	always 0
****************************************************************************/
extern int spi_set_reg(struct spi_user_rec_s *spi_user);


/*Enable and Disable SPI Module*/
extern int spi_enable(struct spi_user_rec_s *spi_user);
extern int spi_disable(struct spi_user_rec_s *spi_user);


/*SPI read, SPI write*/
/****************************************************************************
function: spi_write_data()
purpose: please put your data to wbuf, and appoint the bytes you want to write,
	the function will send your data out using SPI interface.
parameter:
	spi_user - a pointer that  point to a memory which store the user's information
	wbuf -- store the data whitch you want to send
	size  -- how many bytes you want to send
return:
	>=0: write out data size
	< 0 : error
****************************************************************************/
extern int spi_write_data(struct spi_user_rec_s *spi_user, unsigned char *wbuf, unsigned int size);


/****************************************************************************
function: spi_write_and_read_data()
purpose:
	write data for read data, if no special requirement, you can default write garbage data, if your
	device need write special data to read data, then put you data to wbuf.
	The received data can be found in  read buf.
----------------------------------------------------------------------------
SPI specification:
The slave just sends and receives data if the master generates the necessary clock signal.
The master however generates the clock signal only while sending data.
That means that the master has to send data to the slave to read data from the slave.
----------------------------------------------------------------------------
parameter:
spi_user - a pointer that  point to a memory which store the user's information
wbuf --  Buffer for write out data, if wbuf == NULL, then send default garbage data
rbut  --  Buffer for read in data
szie  --  Size of wanted read in data

return:
	>= 0: Read in data size
	< 0  : error
****************************************************************************/
extern int spi_write_and_read_data(struct spi_user_rec_s *spi_user,
				unsigned char *wbuf,
				unsigned char *rbuf,
				unsigned int size);

/****************************************************************************
function: spi_write_tx_fifo()
purpose: write data to  SPI TX FIFO. Because SPI TX FIFO is 32 bytes, so at most can write 32 bytes data
parameter:
	spi_user - a pointer that  point to a memory which store the user's information
	wbut  --  Buffer for write data to SPI TX FIFO
	size  --  bytes number you want to write
return:
	>=0 : write to SPI TX FIFO data size
	<0   : error
****************************************************************************/
int spi_write_tx_fifo(struct spi_user_rec_s *spi_user, unsigned char *wbuf, unsigned int size);


/****************************************************************************
function: spi_read_rx_fifo()
purpose: read SPI RX FIFO. Because SPI RX FIFO is 32 bytes, so at most can read 32 bytes data
parameter:
	spi_user - a pointer that  point to a memory which store the user's information
	rbut  --  Buffer for read in data
return:
	>=0 : Read in data size
	<0   : error
****************************************************************************/
int spi_read_rx_fifo(struct spi_user_rec_s *spi_user, unsigned char *rbuf);

/*Reset TX FIFO, RX FIFO*/
int spi_reset_tx_fifo(struct spi_user_rec_s *spi_user);
int spi_reset_rx_fifo(struct spi_user_rec_s *spi_user);

void set_callback_func(struct spi_user_rec_s *spi_user, void (*callback)(unsigned char *data));

#endif /* ifndef WMT_SPI_H */

/*=== END wmt-spi.h ==========================================================*/
