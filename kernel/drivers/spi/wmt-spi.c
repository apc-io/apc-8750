/*++
	drivers/spi/wmt-spi.c

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
		2010/03/18 First Version
--*/

#include <linux/config.h>
#define WMT_SPI_C


#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/errno.h>
#include <asm/uaccess.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>

#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/sched.h>

#include <mach/hardware.h>
#include <mach/wmt_spi.h>
#include "wmt-spiio.h"
/*#define DEBUG*/
#undef DEBUG
#ifdef DEBUG

#define DPRINTK(fmt, args...)   printk(KERN_DEBUG "%s: " fmt, __func__ , ## args)
#define	ENTER()	printk(KERN_NOTICE "Enter %s, file:%s line:%d\n", \
				__func__, __FILE__, __LINE__)

#define	LEAVE()	printk(KERN_NOTICE "Exit %s, file:%s line:%d\n", \
				__func__, __FILE__, __LINE__)

#else
#define DPRINTK(fmt, args...)
#define	ENTER()
#define	LEAVE()
#endif

#define SPI_MAX_XFER_SIZE	512	/* DMA Allocate Size*/

#define SPI_DMA_CHUNK_SIZE	  4096	/* DMA Allocate Size: 4KB*/

#define DEVICE_NAME "WMT_SPI" /* appear in /proc/devices & /proc/wmt_spi */

#define SPI0_CLK_DIVISOR_VAL  0x06

#define SPI0_CLK_LOW_DIVISOR_VAL  0

struct spi_dev_s {
	/* module parameters */
	char *buf;

	/* char dev struct */
	struct cdev cdev;
};

static int spi_dev_major = 153;
static int spi_dev_minor;
static int spi_dev_nr = 1;
static struct spi_dev_s spi_dev;

static struct spi_port_s SPI_PORT[SPI_PORT_NUM];
static unsigned int get_spi_input_freq(int port);
static unsigned int pllb_input_freq;

/*!*************************************************************************
* spi_user_reset()
*
* Private Function by Jason Y. Lin, 2007/01/02
*/
/*!
* \brief   Reset SPI user record
*
* \retval  N/A
*/
static void spi_user_reset(
	struct spi_user_rec_s *spi_user
)
{

	ENTER();

	/* check user valid*/
	if (!spi_user) {
		printk(KERN_ERR "[SPI]: null user enter %s.\n", __func__);
		return;
	}

	memset(spi_user, 0x0, sizeof(struct spi_user_rec_s));
	/* Initial Private Data Structure
	  Clock mode = Mode 0 (Polarity,Phase) = (0,0)
	  Operate = SPI_POLLING_MODE
	  Arbiter = Master
	  TX Driver = 0 (Disable)
	  TX Driver Count = 0 (No TX output)
	*/
	spi_user->freq = 1000;	/* Setting default frequence is 1Mhz*/
	spi_user->op_mode = SPI_DMA_MODE;		/* work with dma mode*/
	spi_user->ssn_port_mode = SPI_SSN_PORT_PTP;	/* SSN Port Mode = Point to point*/
	spi_user->tx_nodata_value = 1;	/* Setting TX Drive Output Value = 0xFF*/

	LEAVE();

	return;
}

/*!*************************************************************************
* spi_port_reset()
*
* Private Function by Jason Y. Lin, 2007/01/02
*/
/*!
* \brief   Initial SPI hardware configuration and reset spi resource
*
* \retval  N/A
*/
static void spi_port_reset(
	struct spi_port_s *spi_port	/*!<; Pointer of spi port */
)
{
	int port = spi_port->port; /* keep spi port number*/
	/*
	int port_offset;
	*/
	unsigned int spi_base_addr ;

	ENTER();
	if (port >= SPI_PORT_NUM)
		return;

	/* check user valid*/
	if (!spi_port) {
		printk(KERN_ERR "[SPI]: null port enter %s.\n", __func__);
		return;
	}

	memset(spi_port, 0x0, sizeof(struct spi_port_s));

	spi_port->port = port; /* restore spi port number*/
	if (port == 0)
		spi_base_addr = SPI_REG_BASE;
	/* Setting Register address*/
	spi_port->regs.cr = (volatile unsigned int *)(spi_base_addr + SPICR);
	spi_port->regs.sr = (volatile unsigned int *)(spi_base_addr + SPISR);
	spi_port->regs.dfcr = (volatile unsigned int *)(spi_base_addr + SPIDFCR);
	spi_port->regs.cre = (volatile unsigned int *)(spi_base_addr + SPICRE);
	spi_port->regs.rfifo = (volatile unsigned char *)(spi_base_addr + SPIRXFIFO);
	spi_port->regs.wfifo = (volatile unsigned char *)(spi_base_addr + SPITXFIFO);

	init_MUTEX(&spi_port->port_sem);	/* Reset Semaphore*/

	/* Initial IRQ*/
	spi_port->irq_num = IRQ_SPI;	/* Setting SPI IRQ Number*/
	/*********************** DMA RX *************************/
	/* Initial read in dma*/
	init_waitqueue_head(&spi_port->rdma.event_queue);

	/* Setting read in dma dev id*/
	spi_port->rdma.config.DeviceReqType = SPI0_DMA_RX_REQ;

	spi_port->rdma.config.DefaultCCR = SPI_RX_DMA_CFG;  /* transfer size: 8-bit; burst length: INC1*/

	spi_port->rdma.dmach = 0xffffffff;
	spi_port->rdma.config.MIF1addr = (unsigned long)(spi_port->regs.rfifo);
	spi_port->rdma.config.MIF0addr = 0x0;		/* Destination should be memory*/
	spi_port->rdma.config.ChunkSize = SPI_DMA_CHUNK_SIZE;
	spi_port->rdma.xfer_cnt = 0x0;
	spi_port->rdma.event_ack = 0x0;
	spin_lock_init(&spi_port->rdma.spinlock);
	/***************************** DMA TX ************************/
	/* Initial write out dma*/
	init_waitqueue_head(&spi_port->wdma.event_queue);

	spi_port->wdma.config.DeviceReqType = SPI0_DMA_TX_REQ;

	spi_port->wdma.config.DefaultCCR = SPI_TX_DMA_CFG;  /*transfer size: 8-bit; burst length: INC1*/

	spi_port->wdma.dmach = 0xffffffff;
	spi_port->wdma.config.MIF0addr = 0x0;
	spi_port->wdma.config.MIF1addr = (unsigned long)(spi_port->regs.wfifo);
	spi_port->wdma.config.ChunkSize = SPI_DMA_CHUNK_SIZE;

	spi_port->wdma.xfer_cnt = 0x0;
	spi_port->wdma.event_ack = 0x0;
	spin_lock_init(&spi_port->wdma.spinlock);

	/* Reset Hardware*/
	*(spi_port->regs.cr) = SPI_CR_RESET_MASK | SPI_CR_RFR_MASK|SPI_CR_TFR_MASK;
	*(spi_port->regs.sr) = 0x0;
	*(spi_port->regs.dfcr) = SPI_DFCR_RESET_MASK;

	LEAVE();

	return;
}

/*!*************************************************************************
* spi_set_freq()
*
* Private Function by Jason Y. Lin, 2007/01/02
*/
/*!
* \brief   Set working frequency for the user
*
* \retval  NONE
*/
void spi_set_freq(
	struct spi_user_rec_s *spi_user,	/*!<; // Pointer of registered user record */
	int freq				/*!<; // Setting frequency */
)
{

	unsigned int divisor = 0x0;
	unsigned int spi_input_freq;
	struct spi_port_s *spi_port = spi_user->spi_port;
	ENTER();

	REG8_VAL(SPI0_CLOCK_DIVISOR)  = SPI0_CLK_DIVISOR_VAL;
	/* check user valid*/
	if (!spi_user) {
		printk(KERN_ERR "[SPI]: null user enter %s.\n", __func__);
		return;
	}

	spi_input_freq = auto_pll_divisor(DEV_SPI0, SET_DIV, 1, 100000);
	spi_input_freq /= 1000;

	if (spi_user->freq != 0)
		divisor = spi_input_freq / (freq * 2);
	else
		divisor = 0;
	if (divisor > 0x7FF) {
		REG8_VAL(SPI0_CLOCK_DIVISOR)  = SPI0_CLK_LOW_DIVISOR_VAL;
		spi_input_freq /= 8;
		spi_input_freq = auto_pll_divisor(DEV_SPI0, SET_DIV, 1, spi_input_freq);
		spi_input_freq /= 1000;
		if (spi_user->freq != 0)
			divisor = spi_input_freq / (spi_user->freq * 2);
		else
			divisor = 0;
		if (divisor > 0x7FF){
			printk("Not support  %d\n", spi_user->freq);
			freq = spi_input_freq/0x7FF;
		}
	}
	pllb_input_freq = spi_input_freq;
	spi_user->freq = freq; /* Set work frequency for the user*/
	printk("real spi freq = %d\n", freq);

	LEAVE();

	return;
}

/*!*************************************************************************
* spi_set_clk_mode()
*
* Private Function by Jason Y. Lin, 2007/01/02
*/
/*!
* \brief   Set bus clock mode for the user
*
* \retval  NONE
*/
void spi_set_clk_mode(
	struct spi_user_rec_s *spi_user,	/*!<; // Pointer of registered user record */
	enum SPI_TyClkMode_e clk_mode	/*!<; // Setting clock mode */
)
{

	ENTER();

	/* check user valid*/
	if (!spi_user) {
		printk(KERN_ERR "[SPI]: null user enter %s.\n", __func__);
		return;
	}

	if ((clk_mode != SPI_MODE_0) && (clk_mode != SPI_MODE_1)  \
		&& (clk_mode != SPI_MODE_2) && (clk_mode != SPI_MODE_3)) {
		printk(KERN_ERR "[spi_set_clk_mode] clock mode error, force clk_mode to mode 0\n");
		spi_user->clk_mode = SPI_MODE_0;
	}
	spi_user->clk_mode = clk_mode; /* set bus clock mode for the use*/

	LEAVE();

	return;
}

/*!*************************************************************************
* spi_set_chipselect()
*
* Private Function by Howay, 2007/09/13
*/
/*!
* \brief   Set bus clock mode for the user
*
* \retval  NONE
*/
void spi_set_chipselect(
	struct spi_user_rec_s *spi_user,/*!<; // Pointer of registered user record */
	enum SPI_TySSn_e spi_ssn	/*!<; // Select SPISS0 or SPISS1 or SPISS2 or SPISS3 */
)
{

	ENTER();

	/* check user valid*/
	if (!spi_user) {
		printk(KERN_ERR "[SPI]: null user enter %s.\n", __func__);
		return;
	}
	if ((spi_ssn != SPI_SS0) && (spi_ssn != SPI_SS1)  \
		&& (spi_ssn != SPI_SS2) && (spi_ssn != SPI_SS3)) {
		printk(KERN_WARNING "[spi_set_chipselect] spi_ssn error, force spi_ssn to SPI_SS0\n");
		spi_user->slave_select = SPI_SS0;
	} else
		spi_user->slave_select  = spi_ssn; /* set bus clock mode for the use*/

	LEAVE();

	return;
}

void spi_set_tx_fifo_threshold(
	struct spi_user_rec_s *spi_user,/*!<; // Pointer of registered user record */
	enum SPI_TyFIFOTHRESHOLD_e  spi_threshold	/*!<; // Set threshold to 8 bytes or 16 bytes*/
)
{
	/* check user valid*/
	if (!spi_user) {
		printk(KERN_ERR "[SPI]: null user enter %s.\n", __func__);
		return;
	}

	if ((spi_threshold != SPI_THRESHOLD_8BYTES) && (spi_threshold != SPI_THRESHOLD_16BYTES)) {
		printk(KERN_WARNING "[spi_set_tx_fifo_threshold] spi tx fifo threshold error, force tx fifo threshold to 8 bytes\n");
		spi_user->tx_fifo_threshold = SPI_THRESHOLD_8BYTES;
	} else
		spi_user->tx_fifo_threshold = spi_threshold;

}

void spi_set_rx_fifo_threshold(
	struct spi_user_rec_s *spi_user,	/*!<; // Pointer of registered user record */
	enum SPI_TyFIFOTHRESHOLD_e  spi_threshold	/*!<; // Set threshold to 8 bytes or 16 bytes*/
)
{
	/* check user valid*/
	if (!spi_user) {
		printk(KERN_ERR "[SPI]: null user enter %s.\n", __func__);
		return;
	}

	if ((spi_threshold != SPI_THRESHOLD_8BYTES) && (spi_threshold != SPI_THRESHOLD_16BYTES)) {
		printk(KERN_WARNING "[spi_set_rx_fifo_threshold] spi rx fifo threshold error, force rx fifo threshold to 8 bytes\n");
		spi_user->rx_fifo_threshold = SPI_THRESHOLD_8BYTES;
	} else
		spi_user->rx_fifo_threshold = spi_threshold;

}
/*!*************************************************************************
* spi_set_op_mode()
*
* Private Function by Jason Y. Lin, 2007/01/02
*/
/*!
* \brief   Set operation mode for the user
*
* \retval  NONE
*/
void spi_set_op_mode(
	struct spi_user_rec_s *spi_user,	/*!<; // Pointer of registered user record */
	enum SPI_TyOpMode_e op_mode	/*!<; // Setting operation mode */
)
{

	ENTER();

	/* check user valid*/
	if (!spi_user) {
		printk(KERN_ERR "[SPI]: null user enter %s.\n", __func__);
		return;
	}
	/* set operation mode for the use*/
	if (op_mode == SPI_INTERRUPT_MODE)
		printk(KERN_INFO "[spi_set_op_mode] SPI INTERRUPT MODE\n");
	else if (op_mode == SPI_DMA_MODE)
		printk(KERN_INFO "[spi_set_op_mode] SPI DMA MODE\n");
	else if (op_mode != SPI_POLLING_MODE)
		printk(KERN_INFO "[spi_set_op_mode] SPI operation mode error, force to polling mode\n");

	spi_user->op_mode = op_mode;

	LEAVE();

	return;
}

/*!*************************************************************************
* spi_set_arbiter()
*
* Private Function by Jason Y. Lin, 2007/01/02
*/
/*!
* \brief   Set bus arbiter for the user
*
* \retval  NONE
*/
void spi_set_arbiter(
	struct spi_user_rec_s *spi_user,	/*!<; // Pointer of registered user record */
	enum SPI_TyArbiter_e arbiter	/*!<; // Setting bus arbiter */
)
{

	ENTER();

	/* check user valid*/
	if (!spi_user) {
		printk(KERN_ERR "[SPI]: null user enter %s.\n", __func__);
		return;
	}
	if ((spi_user->arbiter != SPI_ARBITER_MASTER) && (spi_user->arbiter != SPI_ARBITER_SLAVE)) {
		printk(KERN_WARNING "[spi_set_arbiter] arbiter error, force arbiter to master\n");
		spi_user->arbiter = SPI_ARBITER_MASTER;
	}

	spi_user->arbiter = arbiter; /* set bus clock mode for the use*/

	LEAVE();

	return;
}

/*!*************************************************************************
* spi_set_port_mode()
*
* Private Function by Jason Y. Lin, 2007/01/02
*/
/*!
* \brief   Set SSn port mode for the user
*
* \retval  NONE
*/
void spi_set_port_mode(
	struct spi_user_rec_s *spi_user,			/*!<; // Pointer of registered user record */
	enum SPI_TySSnPortMode_e ssn_port_mode	/*!<; // Setting SSn Port mode */
)
{

	ENTER();

	/* check user valid*/
	if (!spi_user) {
		printk(KERN_ERR "[SPI]: null user enter %s.\n", __func__);
		return;
	}

	if ((ssn_port_mode != SPI_SSN_PORT_MM) && (ssn_port_mode != SPI_SSN_PORT_PTP)) {
		printk(KERN_WARNING "[spi_set_port_mode] port mode error, force port mode to point to point mode\n");
		spi_user->ssn_port_mode = SPI_SSN_PORT_PTP;
	}
	spi_user->ssn_port_mode = ssn_port_mode; /* Set SSn port mode*/

	LEAVE();

	return;
}

/*!*************************************************************************
* get_spi_input_freq()
*
* Private Function by Howay, 2007/09/13
*/
/*!
* \brief   get WMT SPI input freq from 25MHz PLL
*
* \retval  input freq value
*/
static unsigned int get_spi_input_freq(int port)
{
	return pllb_input_freq;
}

/*!*************************************************************************
* spi_set_reg()
*
* Private Function by Jason Y. Lin, 2007/01/02
*/
/*!
* \brief   Map SPI Information to register setting and then set register
*
* \retval  0 if success
*/
int spi_set_reg(
	struct spi_user_rec_s *spi_user			/*!<; // Pointer of registered user record */
)
{
	/* SSn Default is pull high*/
	unsigned int dataformat = SPI_DFCR_RESET_MASK;
	unsigned int control = 0x0;
	unsigned int divisor = 0x0;
	struct spi_port_s *spi_port = spi_user->spi_port;
	unsigned int spi_input_freq;

	ENTER();

	/* Check user and spi port valid*/
	if (!spi_user || !spi_port) {
		printk(KERN_ERR "[SPI]: invaild user enter %s.\n", __func__);
		return -1;
	}

	/* Reset Hardware*/
	*(spi_port->regs.cr) = SPI_CR_RESET_MASK|SPI_CR_RFR_MASK|SPI_CR_TFR_MASK;
	*(spi_port->regs.dfcr) = dataformat;

	*(spi_port->regs.sr) |= (SPI_SR_RFTPI_MASK | SPI_SR_TFTPI_MASK | SPI_SR_TFUI_MASK  \
			| SPI_SR_TFEI_MASK | SPI_SR_RFOI_MASK | SPI_SR_RFFI_MASK  |  \
			SPI_SR_RFEI_MASK | SPI_SR_MFEI_MASK);

	spi_input_freq = get_spi_input_freq(spi_port->port);

	if (spi_user->freq != 0)
		divisor = spi_input_freq / (spi_user->freq * 2);
	else
		divisor = 0;  /*set to Maximal SPI frequence*/

	if (divisor > 0x7FF)
		divisor = 0x7FF;
	DPRINTK("spi divisor = 0x%X\n", divisor);

	control |= ((divisor << SPI_CR_TCD_SHIFT) & SPI_CR_TCD_MASK);

	/* Setting output ssn signal*/
	control |= ((spi_user->slave_select << SPI_CR_SS_SHIFT) & SPI_CR_SS_MASK);

	if (spi_user->op_mode == SPI_DMA_MODE) { /* DMA Setting*/
		/* Enable Threshold Requst, Under/Over-run, Empty, full IRQ request.*/
		/* Threshold is set to 8 bytes => DMA burst size must less than 8 bytes*/
		if (spi_user->tx_cnt) {
			control |= SPI_CR_DRC_MASK | SPI_CR_TFUI_MASK | SPI_CR_TFEI_MASK;
			if (spi_user->tx_fifo_threshold == SPI_THRESHOLD_8BYTES)
				control |= SPI_CR_TFTS_MASK;
		}
		if (spi_user->rx_cnt) {
			control |= SPI_CR_DRC_MASK | SPI_CR_RFOI_MASK | SPI_CR_RFEI_MASK | SPI_CR_RFFI_MASK;
			if (spi_user->rx_fifo_threshold == SPI_THRESHOLD_8BYTES)
				control |= SPI_CR_RFTS_MASK;
		}
	} else if (spi_user->op_mode == SPI_INTERRUPT_MODE) {	/* Interrupt Setting*/
		/* Pass Threshold Requst to SPI interrupt, and threshold is set to 8 bytes.*/
		/*
		if (spi_user->tx_cnt)
			control |= SPI_CR_TIDS_MASK | SPI_CR_TFTS_MASK | SPI_CR_TFUI_MASK | SPI_CR_TFEI_MASK;
		if (spi_user->rx_cnt)
			control |= SPI_CR_TIDS_MASK | SPI_CR_RFTS_MASK |
				SPI_CR_RFOI_MASK | SPI_CR_RFEI_MASK | SPI_CR_RFFI_MASK;
		*/
		control |= SPI_CR_TFEI_MASK;
		/* Enable IRQ*/
		control |= SPI_CR_IE_MASK;
	}

	/* Setting master and slave control*/
	if (spi_user->arbiter == SPI_ARBITER_SLAVE) { /* Slave Mode*/
		control |= SPI_CR_MSMS_MASK;
		/* If work in slave mode, ssn can't be controled and port can't be Multi-Master*/
		if (spi_user->ssn_port_mode == 0)
			printk(KERN_ERR "[SPI]: spi port can't be Multi-Master mode when using as a slaver.\n");
		/* Check this, Should it set to Point to Point Mode
		 and should ssn be set to auto control by hardware*/
		spi_user->ssn_port_mode = 1;
		dataformat |= SPI_DFCR_SPM_MASK;
	}

	/* Set Clock Mode*/
	if ((spi_user->clk_mode == SPI_MODE_2) || (spi_user->clk_mode == SPI_MODE_3))
		control |= SPI_CR_CPS_MASK;
	if ((spi_user->clk_mode == SPI_MODE_1) || (spi_user->clk_mode == SPI_MODE_3))
		control |= SPI_CR_CPHS_MASK;
	/*
	 *  Use for output setting value when FIFO goes empty
	 *  If enable TX driver Feature, Setting related register.
	 *  else don't care
	 */
	if (spi_user->tx_drive_enable) {
		dataformat |= SPI_DFCR_TDE_MASK;
		/* Setting Driver Value 0xFF*/
		dataformat |= ((spi_user->tx_nodata_value << SPI_DFCR_TNDV_SHIFT) & SPI_DFCR_TNDV_MASK);
		/* Setting Driver Connt*/
		dataformat |= ((spi_user->tx_drive_count << SPI_DFCR_TDCNT_SHIFT) & SPI_DFCR_TDCNT_MASK);
	}

	if (spi_user->ssn_port_mode == 0) {	/* Multi-Master Mode*/
		/* Enable Passed Mode Fault Error Interrupt request if interrupt is enabled*/
		control |= SPI_CR_MFEI_MASK|SPI_CR_MFEF_MASK;
	} else {	 /* otherwise, Point-to-Point mode*/
		dataformat |= SPI_DFCR_SPM_MASK;
	}

	/* Set mapping configuration to register*/
	*(spi_port->regs.dfcr) = dataformat;
	*(spi_port->regs.cr) = control;

	DPRINTK("SPI%d Register Setting:\n", spi_port->port);
	DPRINTK("  Control Register:0x%08x\n", *(spi_port->regs.cr));
	DPRINTK("  Status Register:0x%08x\n", *(spi_port->regs.sr));
	DPRINTK("  Data Format Register:0x%08x\n", *(spi_port->regs.dfcr));

	LEAVE();

	return 0;
}

/*!*************************************************************************
* spi_enable()
*
* Private Function by Jason Y. Lin, 2007/01/02
*/
/*!
* \brief   Enable spi module and start transfer
*
* \retval  0 if success
*/
int spi_enable(
	struct spi_user_rec_s *spi_user			/*!<; // Pointer of registered user record */
)
{
	struct spi_port_s *spi_port = spi_user->spi_port;

	ENTER();

	/* Check user and spi port valid*/
	if (!spi_user || !spi_port) {
		printk(KERN_ERR "[SPI]: invaild user enter %s.\n", __func__);
		return -1;
	}

	down(&spi_port->port_sem); /* take port semaphore for pending other user use*/

	DPRINTK("(%s) Acquired Semaphore@ %lu\n", spi_user->name, jiffies);

	DPRINTK("SPI Enable, CR:%08x SR:%08x DFR:%08x\n",
		*(spi_port->regs.cr), *(spi_port->regs.sr), *(spi_port->regs.dfcr));

	/* Module enable*/
	*(spi_port->regs.cr) |= SPI_CR_ME_MASK;

	LEAVE();

	return 0;
}

/*!*************************************************************************
* spi_disable()
*
* Private Function by Jason Y. Lin, 2007/01/02
*/
/*!
* \brief   Disable spi module and stop transfer
*
* \retval  0 if success
*/
int spi_disable(
	struct spi_user_rec_s *spi_user			/*!<; // Pointer of registered user record */
)
{
	struct spi_port_s *spi_port = spi_user->spi_port;

	ENTER();

	/* Check user and spi port valid*/
	if (!spi_user || !spi_port) {
		printk(KERN_ERR "[SPI]: invaild user enter %s.\n", __func__);
		return -1;
	}

	/* Module disable*/
	*(spi_port->regs.cr) &= ~(SPI_CR_ME_MASK);

	up(&spi_port->port_sem); /* release port semaphore for other user use*/
	DPRINTK("(%s) Released Semaphore@ %lu\n", spi_user->name, jiffies);

	DPRINTK("SPI Disable, CR:%08x SR:%08x DFR:%08x\n",
		*(spi_port->regs.cr), *(spi_port->regs.sr), *(spi_port->regs.dfcr));

	LEAVE();

	return 0;
}

/*!*************************************************************************
* spi_isr()
*
* Private Function by Jason Y. Lin, 2007/01/02
*/
/*!
* \brief   SPI interrupt service routine
*
* \retval  0 if success
*/
static irqreturn_t spi_isr(
	int irq,				/*!<; // IRQ number */
	void *dev				/*!<; // Private paramater(i.e. pointer of spi port) */
)
{
	unsigned int timecnt = 0x30000;
	unsigned int rx_count = 0;
	unsigned int i = 0;
	struct spi_port_s *spi_port = (struct spi_port_s *)dev;

	unsigned int spi_status = *(spi_port->regs.sr);

	ENTER();
	disable_irq_nosync(spi_port->irq_num);
	DPRINTK("spi%d control register = 0x%08x\n", spi_port->port, *(spi_port->regs.cr));
	if (spi_status  & SPI_CR_RFFI_MASK)
		printk(KERN_WARNING "RX FIFO Full interrupt\n");
	if (spi_status  & SPI_CR_RFOI_MASK)
		printk(KERN_WARNING "RX FIFO over-run interrupt\n");
	if (spi_status & SPI_SR_TFES_MASK) {
		while (spi_status & SPI_SR_BUSY_MASK) {
			if (timecnt <= 0)
				break;
			spi_status = *(spi_port->regs.sr);
			--timecnt;
		}
		rx_count = spi_status >> SPI_SR_RFCNT_SHIFT;
		for (i = 0; i < rx_count; ++i)
			*((spi_port->user->rbuf) + i) = *(spi_port->regs.rfifo);
	}
	if (spi_port->user->callback)
		spi_port->user->callback(spi_port->user->rbuf);
	else
		printk("no callback func\n");
	*(spi_port->regs.sr) |= (BIT14 | BIT13 | BIT12 | BIT11 | BIT10 | BIT9  | BIT8 | BIT4);
	enable_irq(spi_port->irq_num);
	LEAVE();
	return IRQ_HANDLED;
};

/*!*************************************************************************
* spi_dsr_r()
*
* Private Function by Jason Y. Lin, 2007/01/02
*/
/*!
* \brief   Map SPI Information to register setting
*
* \retval  0 if success
*/
static void spi_dsr_r(
	void *data	/*!<; // Pointer of spi port */
)
{
	struct spi_port_s *spi_port = (struct spi_port_s *)data;
	struct spi_user_rec_s *spi_user = spi_port->user;
	unsigned int remain_size;
	unsigned long flags;
	int i;

	DPRINTK("enter spi_dsr_r\n");
	/* Check user and spi port valid*/
	if (!spi_port || !spi_user)
		return;

	spin_lock_irqsave(&spi_port->rdma.spinlock, flags); /* stall interrupt trigger*/

	ENTER();

	/* copy data from physical memory to virtual memory for dma moving*/
	memcpy(spi_user->rbuf + spi_port->cur_rx_cnt, spi_user->ior, spi_port->rdma.xfer_cnt);

	spi_port->cur_rx_cnt += spi_port->rdma.xfer_cnt; /* Increase read in data counter*/
	remain_size = spi_user->rx_cnt - spi_port->cur_rx_cnt;
	DPRINTK("[spi_dsr_r] remain_size =%u\n", remain_size);
	/* Hardware Limitation:*/
	/* 		Tf FIFO size is less then threshold size,*/
	/* 		it will not trigger dma request and Data will always stay in FIFO*/
	/* Patch:*/
	/*		Moving by software*/
	/* Threshold size: 8 bytes*/
	if (remain_size > 8) {	/* Bigger than threshold, it can be move by dma*/
		/* Maximum transfer bytes is SPI_DMA_CHUNK_SIZE each time*/
		if (remain_size > SPI_DMA_CHUNK_SIZE)
			spi_port->rdma.xfer_cnt = SPI_DMA_CHUNK_SIZE;
		else
			/* minus (% 8) is because lasr remain
			bytes(less than 8) have be sent by polling mode*/
			spi_port->rdma.xfer_cnt = remain_size - (remain_size % 8);/* Must be times of 8*/
		/* Enable next time dma*/
		START_DMA(spi_port->rdma.dmach, spi_user->phys_addr_r, spi_port->rdma.xfer_cnt);
	} else { /* moving by polling method*/
		/* software patch*/
		if (remain_size) {
			for (i = 0; i < remain_size; i++) {
				/* Read in ramind data of RX FIFO*/
				*(spi_user->rbuf + spi_port->cur_rx_cnt) = *(spi_port->regs.rfifo);
				spi_port->cur_rx_cnt++; /* Increase read in counter*/
			}
		}
		/* Create event for notifing thet transfer is complete*/
		/*--> modify by howay 20071012 (wdma --> rdma)*/
		/*spi_port->wdma.event_ack = 1;*/
		/*wake_up_interruptible(&(spi_port->wdma.event_queue));*/
		spi_port->rdma.event_ack = 1;
		wake_up_interruptible(&(spi_port->rdma.event_queue));
		/*<-- end added*/
	}

	LEAVE();

	spin_unlock_irqrestore(&spi_port->rdma.spinlock, flags); /* restore interrupt trigger*/

	return;
};

/*!*************************************************************************
* spi_dsr_w()
*
* Private Function by Jason Y. Lin, 2007/01/02
*/
/*!
* \brief   Map SPI Information to register setting
*
* \retval  0 if success
*/
static void spi_dsr_w(
	void *data	/*!<; // Pointer of spi port */
)
{
	struct spi_port_s *spi_port = (struct spi_port_s *)data;
	struct spi_user_rec_s *spi_user = spi_port->user;
	unsigned int remain_size;
	unsigned long flags;
	DPRINTK("spi_dsr_w\n");

	/* Check user and spi port valid*/
	if (!spi_port || !spi_user)
		return;


	spin_lock_irqsave(&spi_port->wdma.spinlock, flags); /* stall interrupt trigger*/

	ENTER();

	spi_port->cur_tx_cnt += spi_port->wdma.xfer_cnt; /* Increase read in data counter*/
	remain_size = spi_user->tx_cnt - spi_port->cur_tx_cnt;
	DPRINTK("remain_size = %d\n", remain_size);
	if (remain_size) {
		/* Maximum transfer bytes is SPI_DMA_CHUNK_SIZE each time*/
		if (remain_size > SPI_DMA_CHUNK_SIZE)
			spi_port->wdma.xfer_cnt = SPI_DMA_CHUNK_SIZE;
		else
			spi_port->wdma.xfer_cnt = remain_size;
		if (spi_user->wbuf)
			/* else case: Write-for-read. So, it does't not need to moving source data*/
			/* copy source data from virtual memory to physical memory for dma moving*/
			memcpy(spi_user->iow, spi_user->wbuf + spi_port->cur_tx_cnt, spi_port->wdma.xfer_cnt);
		/* Enable next time dma*/
		START_DMA(spi_port->wdma.dmach, spi_user->phys_addr_w, spi_port->wdma.xfer_cnt);
	} else {
		/* Create event for notifing thet transfer is complete*/
		if (spi_user->wbuf) {	/* This is really write out data, not just Write-for-read*/
		    DPRINTK("[spi_dsr_w] ack\n");
		    spi_port->wdma.event_ack = 1;
		    wake_up_interruptible(&(spi_port->wdma.event_queue));
		}
	}

	LEAVE();

	spin_unlock_irqrestore(&spi_port->wdma.spinlock, flags); /* restore interrupt trigger*/

	return;
};

/*!*************************************************************************
* spi_dma_write_and_read_data()
*
* Private Function by Jason Y. Lin, 2007/01/02
*            Modifed by howayhuo, 2007/12/10
*/
/*!
* \brief   Read in data from SPI port using dma
*
* \retval  >= 0 Read in data size, retval < 0 error
*/
/*
* SPI specification:
* The slave just sends and receives data if the master generates the necessary clock signal.
* The master however generates the clock signal only while sending data.
* That means that the master has to send data to the slave to read data from the slave.
*/
static int spi_dma_write_and_read_data(
	struct spi_user_rec_s *spi_user,/*!<; // Pointer of registered user record */
	unsigned char *wbuf,
	unsigned char *rbuf, 	/*!<; // Buffer for reading in data */
	unsigned int size	/*!<; // Size of wanted read in data */
)
{
	struct spi_port_s *spi_port = spi_user->spi_port;
	wait_queue_head_t *event_queue;
	volatile int *event_ack;
	int timeout_cnt;
	unsigned int status;
	DPRINTK("%s  %d\n", __func__, __LINE__);
	ENTER();
	spi_port->cur_rx_cnt = 0;	/* reset current receive count*/
	spi_port->cur_tx_cnt = 0;	/* reset current transfer count*/
	spi_user->rx_cnt = size;	/* Set read in size*/
	spi_user->rbuf = rbuf;		/* Set read in address*/
	spi_user->wbuf = NULL;		/* Set write out address. NULL means no write data*/
	if (spi_user->arbiter == SPI_ARBITER_MASTER)	/* Set write out size, Master needs write-for-read*/
		spi_user->tx_cnt = size;	/* write out for read*/
	else
		spi_user->tx_cnt = 0;

	spi_set_reg(spi_user);

	if ((spi_user->op_mode == SPI_DMA_MODE) && (spi_user->dma_init)) {
		/* Request DMA channel. If there is no using dma channel, work with polling mode*/
		if (REQUEST_DMA(&spi_port->rdma.dmach, "SPI_RX",
			spi_port->rdma.config.DeviceReqType, spi_dsr_r, spi_port) < 0) {
			printk(KERN_ERR "[SPI]: Error request dma, force polling mode.\n");
			spi_user->op_mode = SPI_POLLING_MODE; /* force polling mode*/
			goto polling_write_data;
		}
		DPRINTK("[spi_dma_write_and_read_data] SPI RX FIFO channel = %d\n",
			spi_port->rdma.dmach);
		/* Master setting bcz. write-for-read*/
		if (spi_user->arbiter == SPI_ARBITER_MASTER) {
			if (REQUEST_DMA(&spi_port->wdma.dmach, "SPI_TX",
				spi_port->wdma.config.DeviceReqType, spi_dsr_w, spi_port) < 0) {
				printk(KERN_ERR "[SPI]: Error request dma, force polling mode.\n");
				spi_user->op_mode = SPI_POLLING_MODE; /* force polling mode*/
				FREE_DMA(spi_port->rdma.dmach); /* free acquired dma*/
				goto polling_write_data;
			}
			DPRINTK("[spi_dma_write_and_read_data] SPI TX FIFO channel = %d\n",
				spi_port->wdma.dmach);
			if (wbuf != NULL)
				memcpy(spi_user->iow, wbuf, size);

			/*
			printk("[spi_read_data] Send Data =");
			for (i = 0;i < size+8; i++)
				printk("%02x ", *((spi_user->iow)+i));
			printk("\n");
			*/
		}
		/*
		 * if data size are bigger than allocated memory size,
		 * transfer allocated memory size and finish the others in dma service routine
		 */
		if (size > SPI_DMA_CHUNK_SIZE) {
			spi_port->wdma.xfer_cnt = SPI_DMA_CHUNK_SIZE;
			spi_port->rdma.xfer_cnt = SPI_DMA_CHUNK_SIZE;
		} else {
			spi_port->wdma.xfer_cnt = size;
			if (spi_port->wdma.xfer_cnt  > SPI_DMA_CHUNK_SIZE)
				spi_port->wdma.xfer_cnt = SPI_DMA_CHUNK_SIZE;
			spi_port->rdma.xfer_cnt = size;
		}

		/* Settup read in dma and enable read in dma*/
		SETUP_DMA(spi_port->rdma.dmach, spi_port->rdma.config);
		START_DMA(spi_port->rdma.dmach, spi_user->phys_addr_r, spi_port->rdma.xfer_cnt);
		/* assign wait queue to dma queue*/
		event_queue = &spi_port->rdma.event_queue;
		event_ack = &spi_port->rdma.event_ack;

		/* Settup write out dma and enable write out dma*/
		SETUP_DMA(spi_port->wdma.dmach, spi_port->wdma.config);
		spi_enable(spi_user);
		START_DMA(spi_port->wdma.dmach, spi_user->phys_addr_w, spi_port->wdma.xfer_cnt);
	} else {
polling_write_data:
		/* assign wait queue to irq queue*/
		event_queue = &spi_port->rx_event_queue;
		event_ack = &spi_port->rx_event_ack;
	}

	/* Wait for transfer complete*/
	wait_event_interruptible(*event_queue, *event_ack);
	*event_ack = 0; /* clear even ack*/

	/*wait for TX FIFO send complete*/
	timeout_cnt = 0x400000;
	while (timeout_cnt) {
		status = *(spi_port->regs.sr);
		if (status & SPI_SR_TFES_MASK)
			break;
		timeout_cnt = timeout_cnt - 1;
	}

	/* Disable SPI module*/
	spi_disable(spi_user);

	/*
	printk("[spi_read_data] Read Data =");
	for (i = 0; i < size; i++)
		printk("%0x02 ", *(rbuf+i));
	printk("\n");
	*/

	/* for working with dma mode to release DMA channel*/
	if (spi_user->dma_init) {
		if (spi_user->op_mode == SPI_DMA_MODE) {/* free dma bcz this time is transfer by dma*/
			FREE_DMA(spi_port->wdma.dmach);
			FREE_DMA(spi_port->rdma.dmach);
		}
		spi_user->op_mode = SPI_DMA_MODE;	/* force to dma mode for next time*/
	}

	LEAVE();

	if (!timeout_cnt) {
		printk(KERN_ERR "[spi_dma_read_data] write for read time out\n");
		return -1;
	}
	return spi_port->cur_rx_cnt;	/* Read in data size*/
};

/*!*************************************************************************
* spi_polling_write_and_read_32bytes()
*/
/* Private Function by by howayhuo, 2007/12/11
*/
/*!
* \brief   Write out data to spi port and  read in data from spi port
*            Because SPI TX FIFO and RX are 32 Bytes,
*		only read and write 32 Bytes every time.
*		   Notify: Work only for Polling Mode
*
* \retval < 0, Read in data fail, retval >= 0, Read in data successful
*/
static int spi_polling_write_and_read_32bytes(
	struct spi_user_rec_s *spi_user,/*!<; // Pointer of registered user record */
	unsigned char *wbuf,
	unsigned char *rbuf, 	/*!<; // Address for put in read data */
	unsigned int size	/*!<; // Size of read/write data */
)
{
	struct spi_port_s *spi_port = spi_user->spi_port;
	unsigned int status, i;
	int timeout_cnt;
	int retval = size;

	if (!spi_user || !spi_port) {
		printk(KERN_ERR "[SPI]: spi user null\n");
		return -1;
	}

	if (size > 0x20) {
		printk(KERN_ERR "[spi_polling_write_and_read_32bytes] Error: Transfer size must less than 32 bytes.\n");
		return -1;
	}

	if (wbuf == NULL) {
		for (i = 0; i < size; i++)
			*(spi_port->regs.wfifo) = 0xff;	/* write in garbage data*/
	} else {
		for (i = 0; i < size; i++)
			*(spi_port->regs.wfifo) = *(wbuf + i);	/* write in source data*/
	}
	for (i = 0; i < size; i++) {
		timeout_cnt = 0x20000;
		while (timeout_cnt) {
			status = *(spi_port->regs.sr);
			if (status & SPI_SR_RFCNT_MASK)
				break;
			timeout_cnt = timeout_cnt - 1;
		}

		if (timeout_cnt)
			*(rbuf + i) = *(spi_port->regs.rfifo);
		else {/* there is no read in data*/
			printk(KERN_ERR "[spi_polling_write_and_read_32bytes] time out\n");
			*(rbuf + i) = *(spi_port->regs.rfifo);
			retval = -1;
		}
	}
	return retval;
}


static int spi_interrupt_write_and_read_32bytes(
	struct spi_user_rec_s *spi_user,/*!<; // Pointer of registered user record */
	unsigned char *wbuf,
	unsigned char *rbuf, 	/*!<; // Address for put in read data */
	unsigned int size	/*!<; // Size of read/write data */
)
{
	struct spi_port_s *spi_port = spi_user->spi_port;
	unsigned int i;
	int retval = size;

	if (!spi_user || !spi_port) {
		printk(KERN_ERR "[SPI]: spi user null\n");
		return -1;
	}

	if (size > 0x20) {
		printk(KERN_ERR "[spi_polling_write_and_read_32bytes] Error: Transfer size must less than 32 bytes.\n");
		return -1;
	}

	if (wbuf == NULL) {
		for (i = 0; i < size; i++)
			*(spi_port->regs.wfifo) = 0xff;	/* write in garbage data*/
	} else {
		for (i = 0; i < size; i++)
			*(spi_port->regs.wfifo) = *(wbuf + i);	/* write in source data*/
	}
	return retval;
}

/*!*************************************************************************
* spi_polling_write_and_read_data()
*
* Private Function by by howayhuo, 2007/12/10
*/
/*!
* \brief   Read in data from SPI port using polling mode
*
* \retval  >= 0 Read in data size, retval < 0 error
*/
/*
* SPI specification:
* The slave just sends and receives data if the master generates the necessary clock signal.
* The master however generates the clock signal only while sending data.
* That means that the master has to send data to the slave to read data from the slave.
*/
static int spi_polling_write_and_read_data(
	struct spi_user_rec_s *spi_user,/*!<; // Pointer of registered user record */
	unsigned char *wbuf,
	unsigned char *rbuf,/*!<; // Buffer for reading in data */
	unsigned int size/*!<; // Size of wanted read in data */
)
{
	unsigned char *p_tx, *p_rx;
	unsigned int transfer_cnt = size;
	int ret;
	DPRINTK("%s  %d\n", __func__, __LINE__);
	p_tx = wbuf;
	p_rx = rbuf;
	DPRINTK("%s  %d\n", __func__, __LINE__);
	spi_set_reg(spi_user);
	spi_enable(spi_user);
	while (transfer_cnt) {
		/*
		* \brief   Write out data to spi port, and  read in data from spi port
		*            Because WMT SPI TX FIFO and RX FIFO are 32 Bytes,
		*		only write and read 32 Bytes every time.
		*		Notify: Work only for Polling Mode
		*/
		if (transfer_cnt > 32) {
			DPRINTK("spi_polling_write_and_read_data (user, buf_tx=0x%x," \
						"buf_rx=0x%x, size=0x%x)\n",
						(u32) wbuf, (u32) rbuf, 32);

			ret = spi_polling_write_and_read_32bytes(spi_user, p_tx, p_rx, 32);
			if (ret < 0) {
				spi_disable(spi_user);
				return size - transfer_cnt;
			}

			if (wbuf != NULL)
				p_tx += 32;
			p_rx += 32;
			transfer_cnt -= 32;
		} else {
			DPRINTK("spi_polling_write_and_read_data (user, buf_tx=0x%x, buf_rx=0x%x,"\
						" size=0x%x)\n",
						(u32) wbuf, (u32) rbuf, transfer_cnt);

			ret = spi_polling_write_and_read_32bytes(spi_user, p_tx, p_rx, transfer_cnt);
			if (ret < 0) {
				spi_disable(spi_user);
				return size - transfer_cnt;
			}
			transfer_cnt = 0;
		}
	}
	spi_disable(spi_user);
	return size;
}
static int spi_interrupt_write_and_read_data(
	struct spi_user_rec_s *spi_user,/*!<; // Pointer of registered user record */
	unsigned char *wbuf,
	unsigned char *rbuf,/*!<; // Buffer for reading in data */
	unsigned int size/*!<; // Size of wanted read in data */
)
{
	unsigned char *p_tx, *p_rx;
	unsigned int transfer_cnt = size;
	int ret;
	if (transfer_cnt > 32)
		return -1;
	DPRINTK("%s  %d\n", __func__, __LINE__);
	p_tx = wbuf;
	p_rx = rbuf;
	spi_disable(spi_user);
	spi_set_reg(spi_user);
	spi_enable(spi_user);
	/*
	* \brief   Write out data to spi port, and  read in data from spi port
	*            Because WMT SPI TX FIFO and RX FIFO are 32 Bytes,
	*		only write and read 32 Bytes every time.
	*/
	DPRINTK("spi_interrupt_write_and_read_data (user, buf_tx=0x%x, buf_rx=0x%x,"\
				" size=0x%x)\n",
				(u32) wbuf, (u32) rbuf, transfer_cnt);

	ret = spi_interrupt_write_and_read_32bytes(spi_user, p_tx, p_rx, transfer_cnt);
	if (ret < 0) {
		spi_disable(spi_user);
		return size - transfer_cnt;
	}
	transfer_cnt = 0;
	return size;
}

/*!*************************************************************************
* spi_write_and_read_data()
*
* Private Function by Jason Y. Lin, 2007/01/02
*            Modifed by howayhuo, 2007/12/10
*/
/*!
* \brief   Read in data from SPI port using dma
*
* \retval  >= 0 Read in data size, retval < 0 error
*/
/*
* SPI specification:
* The slave just sends and receives data if the master generates the necessary clock signal.
* The master however generates the clock signal only while sending data.
* That means that the master has to send data to the slave to read data from the slave.
*/
int spi_write_and_read_data(
	struct spi_user_rec_s *spi_user,/*!<; // Pointer of registered user record */
	unsigned char *wbuf,
	unsigned char *rbuf,/*!<; // Buffer for reading in data */
	unsigned int size/*!<; // Size of wanted read in data */
)
{
	struct spi_port_s *spi_port = spi_user->spi_port;
	spi_user->wbuf = wbuf;
	spi_user->rbuf = rbuf;
	DPRINTK("%s  %d\n", __func__, __LINE__);
	if (!spi_user || !spi_port) {
		printk(KERN_ERR "[SPI]: spi user null\n");
		return -1;
	}
	*(spi_port->regs.cr) |= (BIT17 | BIT16); /*reset RX FIFO and TX FIFO*/
	if (spi_user->op_mode == SPI_DMA_MODE)
		return spi_dma_write_and_read_data(spi_user, wbuf, rbuf, size);
	else if (spi_user->op_mode == SPI_POLLING_MODE)
		return spi_polling_write_and_read_data(spi_user, wbuf, rbuf, size);
	else if (spi_user->op_mode == SPI_INTERRUPT_MODE)
		return spi_interrupt_write_and_read_data(spi_user, wbuf, rbuf, size);
	else {
		printk(KERN_WARNING "[spi_write_and_read_data]: don't surpport the operation mode: %d\n",
			spi_user->op_mode);
		return -1;
	}
}
/*!*************************************************************************
* spi_dma_write_data()
*
* Private Function by Jason Y. Lin, 2007/01/02
*            Modified by howayhuo, 2007/12/11
*/
/*!
* \brief   Write out data to SPI port using dma
*
* \retval  > 0 Write out data size
*/
static int spi_dma_write_data(
	struct spi_user_rec_s *spi_user,/*!<; // Pointer of registered user record */
	unsigned char *buf,/*!<; // Address for write out data */
	unsigned int size/*!<; // Size of the write out data */
)
{
	struct spi_port_s *spi_port = spi_user->spi_port;
	wait_queue_head_t *event_queue;
	volatile int *event_ack;
	int timeout_cnt;
	unsigned int status;

	ENTER();

	/*
	printk("[spi_write_data] Send Data =");
	for (i = 0; i < size;i ++)
		printk("%02x ", *(buf+i));
	printk("\n");
	*/

	spi_port->cur_rx_cnt = 0;	/* reset current receive count*/
	spi_port->cur_tx_cnt = 0;	/* reset current transfer count*/
	spi_user->wbuf = buf;		/* Set write out address*/
	spi_user->rbuf = NULL;		/* Set read in address. NULL means no read in data*/
	spi_user->tx_cnt = size;	/* Set write out size*/
	spi_user->rx_cnt = 0;		/* Set read in size*/

	/* Mapping all configuration to registers setting*/
	spi_set_reg(spi_user);

	/* Work in DMA mode, and have continue memory for dma using*/
	if ((spi_user->op_mode == SPI_DMA_MODE) && (spi_user->dma_init)) {
		/* Request DMA channel. If there is no using dma channel, work with polling mode*/
		if (REQUEST_DMA(&spi_port->wdma.dmach, "SPI_TX",
			spi_port->wdma.config.DeviceReqType, spi_dsr_w, spi_port) < 0) {

			printk(KERN_ERR "[SPI]: Error request dma, force polling mode.\n");
			spi_user->op_mode = SPI_POLLING_MODE;
			goto polling_write_data;
		}
		DPRINTK("SPI_TX channel = %u\n", spi_port->wdma.dmach);

		/* if data size are bigger than allocated memory size,*/
		/* transfer allocated memory size and finish the others in dma service routine*/
		if (size > SPI_DMA_CHUNK_SIZE)
			spi_port->wdma.xfer_cnt = SPI_DMA_CHUNK_SIZE;
		else
			spi_port->wdma.xfer_cnt = size;

		/* if write out data buffer is not the same with dma buffer*/
		/* copy data from virtual memory to physical memory for dma moving*/
		if (spi_user->iow != spi_user->wbuf)
			memcpy(spi_user->iow, spi_user->wbuf, spi_port->wdma.xfer_cnt);

		/* Settup DMA and enable dma*/
		SETUP_DMA(spi_port->wdma.dmach, spi_port->wdma.config);
		spi_enable(spi_user);
		START_DMA(spi_port->wdma.dmach, spi_user->phys_addr_w, spi_port->wdma.xfer_cnt);
		/* assign wait queue to dma queue*/
		event_queue = &spi_port->wdma.event_queue;
		event_ack = &spi_port->wdma.event_ack;
	} else {
polling_write_data:
		/* assign wait queue to irq queue*/
		event_queue = &spi_port->tx_event_queue;
		event_ack = &spi_port->tx_event_ack;
	}

	/* Wait for transfer complete*/
	wait_event_interruptible(*event_queue, *event_ack);
	*event_ack = 0; /* clear even ack*/

	timeout_cnt = 0x400000;
	while (timeout_cnt) {
		status = *(spi_port->regs.sr);
		if (status & SPI_SR_TFES_MASK)
			break;
		timeout_cnt = timeout_cnt - 1;
	}

	/* Disable SPI module*/
	spi_disable(spi_user);

	/* for working with dma mode to release DMA channel*/
	if (spi_user->dma_init) {
		if (spi_user->op_mode == SPI_DMA_MODE)	/* free dma bcz this time is transfer by dma*/
			FREE_DMA(spi_port->wdma.dmach);
		spi_user->op_mode = SPI_DMA_MODE;		/* force to dma mode for next time*/
	}

	LEAVE();

	if (!timeout_cnt) {
		printk(KERN_ERR "spi_write_data time out\n");
		return -1;
	}
	return spi_port->cur_tx_cnt; /* return Write out data size*/
};

/*!*************************************************************************
* spi_polling_write_32bytes()
*/
/* Private Function by howayhuo 2007/12/11
*/
/*!
* \brief   Write out data to spi port
*            Because WMT SPI TX FIFO is 32 Bytes,
*		only write 32 Bytes every time.
*		Notify: Work only for Polling Mode
*
* \retval < 0, write out data fail, retval >= 0, write out data successful
*/
static int spi_polling_write_32bytes(
	struct spi_user_rec_s *spi_user,/*!<; // Pointer of registered user record */
	unsigned char *wbuf,/*!<; // Address for write out data */
	unsigned int size/*!<; // Size of read/write data */
)
{
	struct spi_port_s *spi_port = spi_user->spi_port;
	unsigned int status, i;
	int timeout_cnt;

	if (size > 0x20) {
		printk(KERN_ERR "[spi_polling_write_32bytes] Error: Transfer size must less then 32 bytes.\n");
		return -1;
	}

	for (i = 0; i < size; i++)
		*(spi_port->regs.wfifo) = *(wbuf + i);

	timeout_cnt = 0x200000;
	while (timeout_cnt) {
		status = *(spi_port->regs.sr);
		if (status & SPI_SR_TFES_MASK)
			break;
		timeout_cnt = timeout_cnt - 1;
	}

	if (!timeout_cnt) {
		printk(KERN_ERR "[spi_polling_write_32bytes] time out\n");
		return -1;
	}

	return size;
}


/*!*************************************************************************
* spi_polling_write_data()
*/
/* Private Function by howayhuo 2007/12/11
*/
/*!
* \brief   Write out data to SPI port using poling mode
*
* \retval  > 0 Write out data size
*/
static int spi_polling_write_data(
	struct spi_user_rec_s *spi_user,/*!<; // Pointer of registered user record */
	unsigned char *buf,/*!<; // Address for write out data */
	unsigned int size/*!<; // Size of the write out data */
)
{
	int transfer_cnt = size;
	unsigned char *p_tx;
	int ret;

	/* Mapping all configuration to registers setting*/
	spi_set_reg(spi_user);

	p_tx = buf;
	spi_enable(spi_user);
	while (transfer_cnt) {
		/*
		 * \brief   Write out data to spi port
		 *          Because WMT SPI TX FIFO is 32 Bytes,
		 *          only write 32 Bytes every time.
		 *          Notify: Work only for Polling Mode
		 */
		if (transfer_cnt > 32) {
			DPRINTK("spi_polling_write_data (user, buf_tx=0x%x, size=0x%x)\n",
							(u32) buf, 32);
			ret = spi_polling_write_32bytes(spi_user, p_tx, 32);
			if (ret < 0) {
				spi_disable(spi_user);
				return size - transfer_cnt;
			}
			p_tx += 32;
			transfer_cnt -= 32;
		} else {
			DPRINTK("spi_polling_write_data (user, buf_tx=0x%x, size=0x%x)\n",
							(u32) buf, transfer_cnt);
			ret = spi_polling_write_32bytes(spi_user, p_tx, transfer_cnt);
			if (ret < 0) {
				spi_disable(spi_user);
				return size-transfer_cnt;
			}
			transfer_cnt = 0;
		}
	}
	spi_disable(spi_user);
	return size;
}


/*!*************************************************************************
* spi_write_data()
*
* Private Function by Jason Y. Lin, 2007/01/02
*            Modifed by howayhuo, 2007/12/11
*/
/*!
* \brief   Write out data to SPI port
*
* \retval  > 0 Write out data size
*/
int spi_write_data(
	struct spi_user_rec_s *spi_user,	/*!<; // Pointer of registered user record */
	unsigned char *wbuf, 	/*!<; // Address for write out data */
	unsigned int size		/*!<; // Size of the write out data */
)
{
	struct spi_port_s *spi_port = spi_user->spi_port;

	if (!spi_user || !spi_port) {
		printk(KERN_ERR "[spi_write_data]: spi user null\n");
		return -1;
	}
	*(spi_port->regs.cr) |= BIT16; /*reset TX FIFO*/
	if (spi_user->op_mode == SPI_DMA_MODE)
		return spi_dma_write_data(spi_user, wbuf, size);
	else if (spi_user->op_mode == SPI_POLLING_MODE)
		return spi_polling_write_data(spi_user, wbuf, size);
	else {
		printk(KERN_WARNING "[spi_write_data]: don't surpport the operation mode: %d\n",
				spi_user->op_mode);
		return -1;
	}
}

/*!*************************************************************************
* spi_write_tx_fifo()
*
* Private Function by HowayHuo, 2007/12/15
*
*/
/*!
* \brief   Write out data to SPI FIFO
*
* \retval  >= 0 write to SPI TX FIFO data size; <0 fail
*/

int spi_write_tx_fifo(struct spi_user_rec_s *spi_user, unsigned char *wbuf, unsigned int size)
{
	struct spi_port_s *spi_port = spi_user->spi_port;
	int i;

	if (!spi_user || !spi_port) {
		printk(KERN_ERR "[spi]: spi user null\n");
		return -1;
	}

	if (size > 32)
		printk(KERN_WARNING "[spi_write_tx_fifo] SPI TX FIFO depth is 32 bytes, force size to 32bytes\n");


	spi_reset_tx_fifo(spi_user);

	for (i = 0; i < size; i++)
		*(spi_port->regs.wfifo) = *(wbuf + i);	/* write to SPI TX FIFO*/

	printk(KERN_INFO "[spi_write_tx_fifo] SPI%d Status Register = 0x%08x\n",
			spi_port->port, *(spi_port->regs.sr));

	return size;

}

/*!*************************************************************************
* spi_read_rx_fifo()
*
* Private Function by HowayHuo, 2007/12/14
*
*/
/*!
* \brief   read data from SPI RX FIFO
*
* \retval  >= 0 read in data size; <0 error
*/
int spi_read_rx_fifo(struct spi_user_rec_s *spi_user, unsigned char *rbuf)
{
	struct spi_port_s *spi_port = spi_user->spi_port;
	unsigned int status, i;

	if (!spi_user || !spi_port) {
		printk(KERN_ERR "[spi]: spi user null\n");
		return -1;
	}

	for (i = 0 ; i < 32; i++) {  /*SPI RX FIFO is 32 bytes*/
		status = *(spi_port->regs.sr);
		if (status & SPI_SR_RFCNT_MASK)
			*(rbuf + i) = *(spi_port->regs.rfifo);
		else
			break;
	}
	return i;
}

void set_callback_func(struct spi_user_rec_s *spi_user, void (*callback)(unsigned char *data))
{
	spi_user->callback = callback;
}

/*!*************************************************************************
* spi_reset_tx_fifo()
*
* Private Function by HowayHuo, 2007/12/15
*
*/
/*!
* \brief   reset SPI TX FIFO
*
* \retval  = 0 success; <0 fail
*/
int spi_reset_tx_fifo(struct spi_user_rec_s *spi_user)
{
	struct spi_port_s *spi_port = spi_user->spi_port;
	unsigned int timeout_cnt, status;

	if (!spi_user || !spi_port) {
		printk(KERN_ERR "[spi]: spi user null\n");
		return -1;
	}

	*(spi_port->regs.cr) |= BIT16; /*Clear TX FIFO*/

	timeout_cnt = 0x400000;
	while (timeout_cnt) {
		status = *(spi_port->regs.cr);
		if (status & BIT16)
			timeout_cnt = timeout_cnt - 1;
		else
			break;
	}

	if (!timeout_cnt) {
		printk(KERN_ERR "[spi_reset_tx_fifo] fail\n");
		return -2;
	}
    return 0;
}

/*!*************************************************************************
* spi_reset_rx_fifo()
*
* Private Function by HowayHuo, 2007/12/15
*
*/
/*!
* \brief   reset SPI RX FIFO
*
* \retval  = 0 success; <0 fail
*/
int spi_reset_rx_fifo(struct spi_user_rec_s *spi_user)
{
	struct spi_port_s *spi_port = spi_user->spi_port;
	unsigned int timeout_cnt, status;

	if (!spi_user || !spi_port) {
		printk(KERN_ERR "[spi]: spi user null\n");
		return -1;
	}

	*(spi_port->regs.cr) |= BIT17; /*Clear TX FIFO*/

	timeout_cnt = 0x400000;
	while (timeout_cnt) {
		status = *(spi_port->regs.cr);
		if (status & BIT17)
			timeout_cnt = timeout_cnt - 1;
		else
			break;
	}

	if (!timeout_cnt) {
		printk(KERN_ERR "[spi_reset_rx_fifo] fail\n");
		return -2;
	}
    return 0;
}


/*!*************************************************************************
* register_user()
*
* Private Function by Jason Y. Lin, 2007/01/02
*
* Modify for VT8500 by HowayHuo, 2008/05/28
*
* Modify for WM8510 by DeanHsiao, 2009/03/04
*/
/*!
* \brief   Register user, reset user data, and require relative resource
*
*/
struct spi_user_rec_s *register_user(
	char *name,	/*!<; // Register Device Name. At least 8 bytes */
	int portIndex		/*!<; // portIndex: 0 -- SPI0, 1 -- SPI1, 2 -- SPI2 */
)
{
	struct spi_port_s *spi_port;
	struct spi_user_rec_s *spi_user;

	ENTER();

	/* Check device exist*/
	if (portIndex >= SPI_PORT_NUM) {
		printk(KERN_ERR "Error: Not exist SPI%d!\n", portIndex);
		return NULL;
	}

	spi_port = &SPI_PORT[portIndex];

	/* if the SPI has been used*/
	if (spi_port->user != NULL) {
		printk(KERN_ERR "SPI%d is  busy!\n", portIndex);
		return NULL;
	}

	/* allocate memory for save this user's information*/
	spi_user = (struct spi_user_rec_s *)kmalloc(sizeof(struct spi_user_rec_s), GFP_KERNEL);
	if (spi_user == NULL) {
		printk(KERN_ERR "No memory for spi user record!!!\n");
		return NULL;
	}

	/*Enable SPI function and disable GPIO function for related SPI pin*/
	GPIO_CTRL_GP11_SPI_BYTE_VAL &= ~(GPIO_SPI0_CLK |
			GPIO_SPI0_MOSI|
			GPIO_SPI0_SS|
			GPIO_SPI0_MISO);

	/* Did not turn on SSNb
	*(volatile unsigned int *)0xD8110200 &= ~0x100;
	*/
	GPIO_PULL_EN_GP11_SPI_BYTE_VAL |= ( GPIO_SPI0_CLK_PULL_EN |
									GPIO_SPI0_SS_PULL_EN |
									GPIO_SPI0_MISO_PULL_EN |
									GPIO_SPI0_MOSI_PULL_EN );
									
									

	GPIO_PULL_CTRL_GP11_SPI_BYTE_VAL |= (GPIO_SPI0_SS_PULL_UP | 
								GPIO_SPI0_MISO_PULL_UP |
								GPIO_SPI0_MOSI_PULL_UP );


	GPIO_PULL_CTRL_GP11_SPI_BYTE_VAL &=~ (GPIO_SPI0_CLK_PULL_UP);

	PMCEL_VAL |= 0x00001000;

	spi_user_reset(spi_user); /* reset use configuration with default sttinge*/
	memcpy(spi_user->name, name, 8); /* save register device name*/
	spi_user->spi_port = spi_port; /* link spi port to the user*/
	spi_port->user = spi_user; /* link this user to spi port*/

	/* Allocate Read In DMA Memory*/
	spi_user->ior = (unsigned char *)dma_alloc_coherent(NULL,
						SPI_DMA_CHUNK_SIZE,
						&spi_user->phys_addr_r,
						GFP_KERNEL | GFP_DMA);
	/* If allocate failure, force work with polling mode*/
	if (spi_user->ior == NULL) {
		printk(KERN_WARNING "DMA memory allocate fail, force polling mode.\n");
		spi_user->op_mode = SPI_POLLING_MODE;
		goto register_user_leave;
	} else
		DPRINTK("Allocate Read In DMA Memory successful\n");

	memset(spi_user->ior, 0x0, SPI_DMA_CHUNK_SIZE); /* reset allocated memory*/

	/* Allocate Write Out DMA Memory*/
	spi_user->iow = (unsigned char *)dma_alloc_coherent(NULL,
						SPI_DMA_CHUNK_SIZE,
						&spi_user->phys_addr_w,
						GFP_KERNEL | GFP_DMA);
	/* If allocate failure, force work with polling mode, and free read-in dma memory*/
	if (spi_user->iow == NULL) {
		printk(KERN_WARNING "DMA memory allocate fail, force polling mode.\n");
		/*dma_free_coherent(NULL,SPI_DMA_CHUNK_SIZE,spi_user->ior,spi_user->phys_addr_r);*/
		spi_user->op_mode = SPI_POLLING_MODE;
		spi_user->ior = NULL;
	} else {
		spi_user->dma_init = 1; /* set dma resource is available*/
		memset(spi_user->iow, 0x0, SPI_DMA_CHUNK_SIZE); /* reset allocated memory*/
		DPRINTK("Allocate Write Out DMA Memory successful\n");
	}

	DPRINTK("%s: %s register %p (port:%d) success.\n", __func__, name, spi_user, spi_port->port);
register_user_leave:

	LEAVE();

	return spi_user;
};

/*!*************************************************************************
* unregister_user()
*
* Private Function by Jason Y. Lin, 2007/01/02
*/
/*!
* \brief   Unregister user, reset user data, and release relative resource
*
* \retval  0 if success
*/
int unregister_user(
	struct spi_user_rec_s *spi_user,	/*!<; // Pointer of registered user record */
	int portIndex				/* portIndex: 0 -- SPI0, 1 -- SPI1, 2 -- SPI2 */
)
{
	struct spi_port_s *spi_port = &SPI_PORT[portIndex];

	ENTER();

	/* Check device exist*/
	if (portIndex >= SPI_PORT_NUM) {
		printk(KERN_ERR "Error: Not exist spi%d !\n", portIndex);
		return -1;
	}

	spi_port = &SPI_PORT[portIndex];

	/* if the spi has not been used or register with different user*/
	if ((spi_port->user != spi_user) || (spi_user == NULL)) {
		printk(KERN_ERR "Error unregister spi%d user!\n", portIndex);
		return -1;
	}

	/* if there are successfully allocatee memory when registerd, release it*/
	if (spi_user->dma_init) {
		dma_free_coherent(NULL, SPI_DMA_CHUNK_SIZE, spi_user->ior, spi_user->phys_addr_r);
		dma_free_coherent(NULL, SPI_DMA_CHUNK_SIZE, spi_user->iow, spi_user->phys_addr_w);
	}

	DPRINTK("%s: unregister %p (port:%d) success.\n", __func__, spi_user, spi_port->port);

	/* free user information*/
	kfree(spi_user);
	spi_user = NULL;

	/* clear this user in spi port*/
	spi_port->user = NULL;

	LEAVE();

	return 0;
};

/*!*************************************************************************
* spi_open()
*
* Private Function by Jason Y. Lin, 2007/01/02
*/
/*!
* \brief   Register spi port as a user
*
* \retval  0 if success
*/
static int spi_open(
	struct inode *inode,	/*!<; //[IN] a pointer point to struct inode */
	struct file *filp	/*!<; //[IN] a pointer point to struct file  */
)
{
	struct spi_dev_s *dev;
	char name[8];
	int minor_no;

	ENTER();

	dev = container_of(inode->i_cdev, struct spi_dev_s, cdev);
	filp->private_data = dev;
	minor_no = iminor(inode);	/* get */

	/* Check device exist*/
	if (minor_no > SPI_PORT_NUM)
		return -ENODEV;

	/* Check the user valid or invalid*/
	if (SPI_PORT[minor_no].user)
		return -EBUSY;

	/* Create user name*/
	memset(name, 0x0, 8);
	sprintf(name, "SPI%d", minor_no);

	/* register user*/
	if (register_user(name, minor_no) == NULL) {
		printk(KERN_ERR "Register %s fail.\n", name);
		return -ENODEV;
	}

	LEAVE();

	return 0;
} /* End of spi_open() */

/*!*************************************************************************
* spi_release()
*
* Private Function by Jason Y. Lin, 2007/01/02
*/
/*!
* \brief   Release register user
*
* \retval  0 if success
*/
static int spi_release(
	struct inode *inode,	/*!<; //[IN] a pointer point to struct inode */
	struct file *filp	/*!<; //[IN] a pointer point to struct file  */
)
{
	struct spi_dev_s *dev;
	int minor_no;

	ENTER();

	dev = container_of(inode->i_cdev, struct spi_dev_s, cdev);
	minor_no = iminor(inode);	/* get */

	/* Check device exist*/
	if (minor_no > SPI_PORT_NUM)
		return -ENODEV;

	/* Check the user valid or invalid*/
	if (SPI_PORT[minor_no].user == NULL)
		return -ENODEV;

	/* Unregister user*/
	unregister_user(SPI_PORT[minor_no].user, minor_no);

	LEAVE();

	return 0;
} /* End of spi_release() */

/*!*************************************************************************
* spi_read()
*
* Private Function by Jason Y. Lin, 2007/01/02
*/
/*!
* \brief   Read in data via SPI
*
* \retval  > 0 Read in data size
*/
static ssize_t spi_read(
	struct file *filp, /*!<; //[IN] a pointer point to struct file  */
	char __user *buf,  /*!<; // please add parameters description her*/
	size_t count,      /*!<; // please add parameters description her*/
	loff_t *f_pos	   /*!<; // please add parameters description her*/
)
{
	struct spi_user_rec_s *spi_user;
	unsigned char *rbuf;
	ssize_t len = 0, idx = 0;
	int minor_no;

	ENTER();

	minor_no = iminor(filp->f_dentry->d_inode);

	/* Check device exist*/
	if (minor_no > SPI_PORT_NUM)
		return -ENODEV;

	spi_user = SPI_PORT[minor_no].user;
	/* Check the user valid or invalid*/
	if (spi_user == NULL)
		return -ENODEV;

	len = count;
	/* if there are dma using memory, use it*/
	if (spi_user->dma_init)
		rbuf = spi_user->ior;
	else	/* else, kmalloc memory*/
		rbuf = (unsigned char *)kmalloc(0x200, GFP_KERNEL);	/* allocate buffer*/

	if (rbuf == NULL) /* there is no memory for saving read-in data*/
		return 0;

	do {
		if (spi_user->dma_init && count > SPI_DMA_CHUNK_SIZE)
			len = SPI_DMA_CHUNK_SIZE;
		else if (!spi_user->dma_init && count > 0x200)
			len = 0x200;
		else
			len = count;

		spi_write_and_read_data(spi_user, NULL, rbuf, (unsigned int) len); /* read in data*/
		copy_to_user(buf+idx, rbuf, len);	/* copy read in data to user buffer*/

		idx += len;
		count -= len;
	} while (count);

	if (rbuf != spi_user->ior) /* if there is allocated memory for read-in data, free it*/
		kfree(rbuf);

	LEAVE();

	return idx;
} /* spi_read */

/*!*************************************************************************
* spi_write()
*
* Private Function by Jason Y. Lin, 2007/01/02
*/
/*!
* \brief   Write out data via SPI
*
* \retval > 0 Write out data size
*/
static ssize_t spi_write(
	struct file *filp, /*!<; //[IN] a pointer point to struct file  */
	const char __user *buf,  /*!<; // please add parameters description her*/
	size_t count,      /*!<; // please add parameters description her*/
	loff_t *f_pos	   /*!<; // please add parameters description her*/
)
{
	struct spi_user_rec_s *spi_user;
	unsigned char *wbuf;
	ssize_t len = 0, idx = 0;
	int minor_no;

	ENTER();

	minor_no = iminor(filp->f_dentry->d_inode);

	/* Check device exist*/
	if (minor_no > SPI_PORT_NUM)
		return -ENODEV;

	spi_user = SPI_PORT[minor_no].user;
	/* Check the user valid or invalid*/
	if (spi_user == NULL)
		return -ENODEV;

	len = count;
	/* if there are dma using memory, use it*/
	if (spi_user->dma_init)
		wbuf = spi_user->iow;
	else /* else, kmalloc memory*/
		wbuf = (unsigned char *)kmalloc(0x200, GFP_KERNEL);	/* allocate buffer*/

	if (wbuf == NULL) /* there is no memory for saving write-out data*/
		return 0;

	do {
		if (spi_user->dma_init && count > SPI_DMA_CHUNK_SIZE)
			len = SPI_DMA_CHUNK_SIZE;
		else if (!spi_user->dma_init && count > 0x200)
			len = 0x200;
		else
			len = count;

		copy_from_user(wbuf, buf+idx, len);
		spi_write_data(spi_user, wbuf, (unsigned int) len);	/* Write out data*/

		idx += len;
		count -= len;
	} while (count);

	if (wbuf != spi_user->iow) /* if there is allocated memory for saveing write-out data, free it*/
		kfree(wbuf);

	LEAVE();

	return idx;
} /* End of spi_write() */

/*!*************************************************************************
* spi_ioctl()
*
* Private Function by Jason Y. Lin, 2007/01/02
*/
/*!
* \brief   Provide specific function for user space AP
*
* \retval  0 if success
*/
static int spi_ioctl(
	struct inode *inode,	/*!<; //[IN] a pointer point to struct inode */
	struct file *filp,	/*!<; //[IN] a pointer point to struct file  */
	unsigned int cmd,	/*!<; // please add parameters description her*/
	unsigned long arg	/*!<; // please add parameters description her*/
)
{
	struct spi_user_rec_s *spi_user;
	struct spi_xfer_req_s spi_xfer_req;
	unsigned char *rbuf, *wbuf;
	int err = 0;
	int retval = 0;
	int minor_no, value;
	ssize_t len = 0, idx = 0;

	ENTER();

	/* check type and number, if fail return ENOTTY */
	if (_IOC_TYPE(cmd) != SPI_IOC_MAGIC)
		return -ENOTTY;
	if (_IOC_NR(cmd) > SPI_IOC_MAXNR)
		return -ENOTTY;

	/* check argument area */
	if (_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_WRITE, (void __user *) arg, _IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err = !access_ok(VERIFY_READ, (void __user *) arg, _IOC_SIZE(cmd));

	if (err)
		return -EFAULT;

	minor_no = iminor(inode);	/* get */

	/* Check device exist*/
	if (minor_no > SPI_PORT_NUM)
		return -ENODEV;

	/* Check the user valid or invalid*/
	spi_user = SPI_PORT[minor_no].user;
	if (spi_user == NULL)
		return -ENODEV;

	switch (cmd) {
	case SPIIOGET_FREQ:	/* Get bus frequency of the user*/
		value = spi_user->freq;
		put_user(value, (int *)arg);
		break;

	case SPIIOSET_FREQ: /* Set bus frequency for the user*/
		get_user(value, (int *)arg);
		spi_set_freq(spi_user, value);
		break;

	case SPIIOGET_CLKMODE: /* Get bus clock mode of the user*/
		put_user(spi_user->clk_mode, (int *)arg);
		break;

	case SPIIOSET_CLKMODE: /* Set bus clock mode for the user*/
		get_user(value, (int *)arg);
		if (value < SPI_MODE_MAX)	/* Check boundary*/
			spi_set_clk_mode(spi_user, value);
		else
			retval = -ENOTTY;
		break;

	case SPIIOGET_ARBITER:	/* Get bus arbiter*/
		put_user(spi_user->arbiter, (int *)arg);
		break;

	case SPIIOSET_ARBITER:	/* Set bus arbiter*/
		get_user(value, (int *)arg);
		if (value < SPI_ARBITER_MAX)	/* Check boundary*/
			spi_set_arbiter(spi_user, value);
		else
			retval = -ENOTTY;
		break;

	case SPIIOGET_OPMODE:	/* Get Operation Mode*/
		put_user(spi_user->op_mode, (int *)arg);
		break;

	case SPIIOSET_OPMODE:	/* Set Operation Mode*/
		get_user(value, (int *)arg);
		if (value < SPI_OP_MAX)	/* Check boundary*/
			spi_set_op_mode(spi_user, value);
		else
			retval = -ENOTTY;
		break;

	case SPIIOGET_PORTMODE:	/* Get SSn Port Mode*/
		put_user(spi_user->ssn_port_mode, (int *)arg);
		break;

	case SPIIOSET_PORTMODE:	/* Set SSn Port Mode*/
		get_user(value, (int *)arg);
		if (value < SPI_SSN_PORT_MAX) /* Check boundary*/
			spi_set_port_mode(spi_user, value);
		else
			retval = -ENOTTY;
		break;

	case SPIIO_READ:	/* Read in data*/
		/* Get transfer requirment from user space*/
		retval = copy_from_user((struct spi_xfer_req_s *)&spi_xfer_req,
								(const void *)arg,
								sizeof(struct spi_xfer_req_s));

		/* if there are dma using memory, use it*/
		if (spi_user->dma_init)
			rbuf = spi_user->ior;
		else	/* else, kmalloc memory*/
			rbuf = (unsigned char *)kmalloc(spi_xfer_req.rsize, GFP_KERNEL);

		if (rbuf == NULL)	/* there is no memory for read in data*/
			return -EBUSY;

		do {
			if (spi_user->dma_init && spi_xfer_req.rsize > SPI_DMA_CHUNK_SIZE)
				len = SPI_DMA_CHUNK_SIZE;
			else if (!spi_user->dma_init && spi_xfer_req.rsize > 0x200)
				len = 0x200;
			else
				len = spi_xfer_req.rsize;

			spi_write_and_read_data(spi_user, NULL, rbuf, len);
			copy_to_user(spi_xfer_req.rbuf + idx, rbuf, len);

			idx += len;
			spi_xfer_req.rsize -= len;
		} while (spi_xfer_req.rsize);

		if (rbuf != spi_user->ior)
			kfree(rbuf);
		break;

	case SPIIO_WRITE:	/* Write out data*/
		/* Get transfer requirment from user space*/
		retval = copy_from_user((struct spi_xfer_req_s *)&spi_xfer_req,
								(const void *)arg,
								sizeof(struct spi_xfer_req_s));
		/* if there are dma using memory, use it*/
		if (spi_user->dma_init)
			wbuf = spi_user->iow;
		else	/* else, kmalloc memory*/
			wbuf = (unsigned char *)kmalloc(spi_xfer_req.wsize, GFP_KERNEL); /* allocate buffer*/

		if (wbuf == NULL)
			return -EBUSY;	/* there is no memory for saving write-out data*/

		do {
			if (spi_user->dma_init && spi_xfer_req.wsize  > SPI_DMA_CHUNK_SIZE)
				len = SPI_DMA_CHUNK_SIZE;
			else if (!spi_user->dma_init && spi_xfer_req.wsize  > 0x200)
				len = 0x200;
			else
				len = spi_xfer_req.wsize ;

			copy_from_user(wbuf, spi_xfer_req.wbuf + idx, len);
			spi_write_data(spi_user, wbuf, (unsigned int) len);/* Write out data*/

			idx += len;
			spi_xfer_req.wsize -= len;
		} while (spi_xfer_req.wsize);

		if (wbuf != spi_user->iow)
			kfree(wbuf);

		break;
	default:
		return -ENOTTY;
	}

	LEAVE();

	return retval;
} /* End of spi_ioctl() */

/*!*************************************************************************
	driver file operations struct define
****************************************************************************/
struct file_operations spi_fops = {
	.owner = THIS_MODULE,
	.open = spi_open,
	.read = spi_read,
	.write = spi_write,
	.ioctl = spi_ioctl,
	.release = spi_release,
};

/*!*************************************************************************
* spi_probe()
*
* Private Function by Jason Y. Lin, 2007/01/02
*/
/*!
* \brief   Initial port information and resource
*
* \retval  0 if success
*/
static int spi_probe(
	struct platform_device *pdev	/*!<; // please add parameters description her*/
)
{
	int port;
	int ret = 0;
	dev_t dev_no;
	struct cdev *cdev;

	ENTER();

	printk("Dean:%s\n", __func__);

	dev_no = MKDEV(spi_dev_major, spi_dev_minor);

	/* register char device */
	/* cdev = cdev_alloc();*/
	cdev = &spi_dev.cdev;
	cdev_init(cdev, &spi_fops);
	ret = cdev_add(cdev, dev_no, 8);

	if (ret) {
		printk(KERN_ALERT "*E* register char dev \n");
		return ret;
	}

	/* Initial Hardware*/
	/* Initial Input Clock (PMC PLL)*/
	REG32_VAL(WMT_PMC_BASE + 0x250) |= (SPI0CLKEN | SPI1CLKEN | SPI2CLKEN);
	/* Initial SPI Divisor*/
	REG8_VAL(SPI0_CLOCK_DIVISOR)  = SPI0_CLK_DIVISOR_VAL;

	/* Initial and request each port resource*/
	for (port = 0; port < SPI_PORT_NUM; port++) {
		SPI_PORT[port].port = port;			/* Setting i-th port*/
		spi_port_reset(&SPI_PORT[port]);	/* Reset spi hardware and property*/
		/* Request IRQ*/
		printk("spi irq num = 0x%x\n", SPI_PORT[port].irq_num);
		printk("B:%x\n", *(volatile unsigned char *)(0xD8140058));
		if (request_irq(SPI_PORT[port].irq_num, spi_isr, IRQF_DISABLED, "SPI-IRQ", &SPI_PORT[port]) < 0) {
			printk(KERN_ERR "request irq failed \n");
			return -1;
		}
		printk("A:%x\n", *(volatile unsigned char *)(0xD8140058));
	}

	printk(KERN_ALERT "spi_probe: /dev/%s major number %d, minor number %d, device number %d\n",
			DEVICE_NAME, spi_dev_major,
			spi_dev_minor,
			SPI_PORT_NUM);

	LEAVE();

	return ret;
} /* End of spi_probe() */

/*!*************************************************************************
* spi_remove()
*
* Private Function by Jason Y. Lin, 2007/01/02
*/
/*!
* \brief   Remove resource and reset port information
*
* \retval  0 if success
*/
static int spi_remove(
	struct platform_device *pdev	/*!<; // please add parameters description her*/
)
{
	struct cdev *cdev;
	int port;

	ENTER();

	cdev = &spi_dev.cdev;
	cdev_del(cdev);

	/* Release each port resource*/
	for (port = 0; port < SPI_PORT_NUM; port++) {
		/* Unregister each user*/
		/* If user is not unregister, unregister it*/
		if (SPI_PORT[port].user)
			unregister_user(SPI_PORT[port].user, port);

		SPI_PORT[port].port = port;			/* Setting i-th port*/
		free_irq(SPI_PORT[port].irq_num, &SPI_PORT[port]);		/* Free IRQ*/
		spi_port_reset(&SPI_PORT[port]);	/* Reset spi hardware and property*/
	}

	LEAVE();
	return 0;
} /* End of spi_remove() */

/*!*************************************************************************
* spi_suspend()
*
* Private Function by Jason Y. Lin, 2007/01/02
*/
/*!
* \brief
*
* \retval  0 if success
*/
static int spi_suspend(
	struct platform_device *pdev, pm_message_t state
)
{

	ENTER();
#if 0
	switch (state) {
	case SUSPEND_NOTIFY:
		/*
		 * Suspend transition is about to happen.
		 */
		break;
	case SUSPEND_SAVE_STATE:
		/*
		 * We only need to save hardware registers
		 * on power-off suspend.
		 */
		break;
	case SUSPEND_DISABLE:
		/*
		 * Stop I/O transactions.
		 */
		break;
	case SUSPEND_POWER_DOWN:
		/*
		 * Place the device in the low power state
		 */
		break;
	}
#endif

	LEAVE();

	return 0;
} /* End of spi_suspend() */

/*!*************************************************************************
* spi_resume()
*
* Private Function by Jason Y. Lin, 2007/01/02
*/
/*!
* \brief
*
* \retval  0 if success
*/
static int spi_resume(
	struct platform_device *pdev	/*!<; // please add parameters description her*/
)
{

	ENTER();
	return 0;
} /* End of spi_resume() */

/*!*************************************************************************
	device driver struct define
****************************************************************************/
static struct platform_driver spi_driver = {
	.driver.name           = "wmt_spi", /* This name should equal to platform device name.*/
	.probe          = spi_probe,
	.remove         = spi_remove,
	.suspend        = spi_suspend,
	.resume         = spi_resume
};

/*!*************************************************************************
* spi_platform_release()
*
* Private Function by Jason Y. Lin, 2007/01/02
*/
/*!
* \brief
*
* \retval  0 if success
*/
static void spi_platform_release(
	struct device *device	/*!<; // please add parameters description her*/
)
{
	ENTER();
	LEAVE();
} /* End of spi_platform_release() */

/*!*************************************************************************
	platform device struct define
****************************************************************************/

static struct platform_device spi_device = {
	.name           = "wmt_spi",
	.id             = 0,
	.dev            = 	{	.release = spi_platform_release,
#if 0
							.dma_mask = &spi_dma_mask,
							.coherent_dma_mask = ~0,
#endif
						},
	.num_resources  = 0,		/* ARRAY_SIZE(spi_resources), */
	.resource       = NULL,		/* spi_resources, */
};

/*!*************************************************************************
* spi_init()
*
* Private Function by Jason Y. Lin, 2007/01/02
*/
/*!
* \brief   Hang up driver
*
* \retval  0 if success
*/
static int spi_init(void)
{
	int ret;
	dev_t dev_no;

	ENTER();

	if (spi_dev_major) {
		dev_no = MKDEV(spi_dev_major, spi_dev_minor);
		ret = register_chrdev_region(dev_no, spi_dev_nr, "wmt_spi");
	} else {
		ret = alloc_chrdev_region(&dev_no, spi_dev_minor, spi_dev_nr, "wmt_spi");
		spi_dev_major = MAJOR(dev_no);
	}

	if (ret < 0) {
		printk(KERN_ALERT "*E* can't get major %d\n", spi_dev_major);
		return ret;
	}


	ret = platform_driver_register(&spi_driver);
	if (!ret) {
		ret = platform_device_register(&spi_device);
		if (ret)
			platform_driver_unregister(&spi_driver);
	}

	LEAVE();

	return ret;
} /* End of spi_init() */

module_init(spi_init);

/*!*************************************************************************
* spi_exit()
*
* Private Function by Jason Y. Lin, 2007/01/02
*/
/*!
* \brief   Unregister driver
*
* \retval  0 if success
*/
static void spi_exit(void)
{
	dev_t dev_no;

	ENTER();

	platform_driver_unregister(&spi_driver);
	platform_device_unregister(&spi_device);
	dev_no = MKDEV(spi_dev_major, spi_dev_minor);
	unregister_chrdev_region(dev_no, spi_dev_nr);

	LEAVE();

	return;
} /* End of spi_exit() */

module_exit(spi_exit);

MODULE_AUTHOR("WMT SW Team");
MODULE_DESCRIPTION("WMT_spi device driver");
MODULE_LICENSE("GPL");

EXPORT_SYMBOL(register_user);
EXPORT_SYMBOL(unregister_user);
EXPORT_SYMBOL(spi_set_freq);
EXPORT_SYMBOL(spi_set_clk_mode);
EXPORT_SYMBOL(spi_set_op_mode);
EXPORT_SYMBOL(spi_set_arbiter);
EXPORT_SYMBOL(spi_set_port_mode);
EXPORT_SYMBOL(spi_write_and_read_data);
EXPORT_SYMBOL(spi_write_data);
EXPORT_SYMBOL(spi_write_tx_fifo);
EXPORT_SYMBOL(spi_read_rx_fifo);
EXPORT_SYMBOL(spi_reset_tx_fifo);
EXPORT_SYMBOL(spi_reset_rx_fifo);
EXPORT_SYMBOL(spi_set_chipselect);
EXPORT_SYMBOL(spi_set_reg);
EXPORT_SYMBOL(spi_enable);
EXPORT_SYMBOL(spi_disable);


/*--------------------End of Function Body -----------------------------------*/

#undef WMT_SPI_C
