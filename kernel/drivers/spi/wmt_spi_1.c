/*++
	drivers/spi/wmt_spi_1.c

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
#include <linux/init.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/ioport.h>
#include <linux/dma-mapping.h>
#include <linux/sched.h>

#include <mach/hardware.h>
#include <mach/wmt_gpio.h>

//#define  DEBUG  1   /* debug open */
#include <linux/platform_device.h>
#include <linux/spi/spi.h>
#include <mach/wmt-spi.h>

#ifdef  DEBUG
#define spi_dbg(fmt, args...) printk(KERN_ERR "[%s]_%d: " fmt, __func__ , __LINE__, ## args)
#define spi_trace()           printk(KERN_ERR "trace in %s %d\n", __func__, __LINE__);
#else
#define spi_dbg(fmt, args...)
#define spi_trace()
#endif

static unsigned int pllb_input_freq = 83333;
static bool g_use_ss0 = true;
static bool g_use_ss1 = false;
static bool g_use_ss2 = false;
static bool g_use_ss3 = false;

/*
 * spi_set_reg32 - write a u32 value to spi register
 * @spi: spi controller's driver data
 * @reg_offset: register's offset address
 * @val: value register will be set
 */
static inline void 
spi_set_reg32(struct wmt_spi *spi, u32 reg_offset, u32 val)
{
	iowrite32(val, spi->regs_base + reg_offset);  
}

/*
 * spi_get_reg32 - read a u32 value from spi register
 * @spi: spi controller's driver data
 * @reg_offset: register's offset address
 */
static inline unsigned int 
spi_get_reg32(struct wmt_spi *spi, int reg_offset)
{
	return ioread32(spi->regs_base + reg_offset);
}

/*
 * spi_setbit: write bit1 to related register's bit
 * @spi: spi controller's driver data
 * @offset: register's offset address
 * @mask: bit setting mask
 */
static void
spi_setbit(struct wmt_spi *spi, u32 reg_offset, u32 mask)
{
	u32 tmp;
	tmp  = spi_get_reg32(spi, reg_offset);
	tmp |= mask;
	spi_set_reg32(spi, reg_offset, tmp);
}

/*
 * spi_clrbit: write bit0 to related register's bit
 * @spi: spi controller's driver data
 * @offset: register's offset address
 * @mask: bit setting mask
 */
static void
spi_clrbit(struct wmt_spi *spi, u32 reg_offset, u32 mask)
{
	u32 tmp;
	tmp  = spi_get_reg32(spi, reg_offset);
	tmp &= ~mask;
	spi_set_reg32(spi, reg_offset, tmp);
}

/*
 * spi_write_fifo: write a u8 value to spi tx fifo
 * @spi: spi controller's driver data
 * @fifo_reg: spi tx fifo register offset
 * @val: value writen to spi tx fifo
 */
static inline void 
spi_write_fifo(struct wmt_spi *spi, u32 fifo_reg, const u8 val)
{
	iowrite8(val, spi->regs_base + fifo_reg);
}

/*
 * spi_read_fifo: read a u8 value from spi rx fifo
 * @spi: spi controller's driver data
 * @fifo_reg: spi rx fifo register offset
 */
static inline u8 
spi_read_fifo(struct wmt_spi *spi, u32 fifo_reg)
{
	return ioread8(spi->regs_base + fifo_reg);
}

/*
 * wmt_spi_clock_enable: set GPIO to SPI mode, ten enable spi 
 *  clock from Power management module and set default spi clock 
 *  dividor to 0x04 as default (93Mhz)
 */
static inline void wmt_spi_clock_enable(void)
{
	int timeout = POLLING_SPI_REG_TIMEOUT;

	/*Enable SPI function and disable GPIO function for related SPI pin*/
	GPIO_CTRL_GP14_SD_BYTE_VAL &= ~(GPIO_SD0_CLK |
			GPIO_SD0_CMD |
			GPIO_SD0_DATA1);

#if 0
	GPIO_CTRL_GP11_SPI_BYTE_VAL |= GPIO_SPI0_SSB;/*set ssb to gpio pin*/
	GPIO_OC_GP11_SPI_BYTE_VAL |= BIT4; /*set ssb to gpio output*/
	GPIO_OD_GP11_SPI_BYTE_VAL |= BIT4; /*ssb pull high*/

	GPIO_CTRL_GP0_BYTE_VAL |= BIT4;
	GPIO_OC_GP0_BYTE_VAL |= BIT4;
	GPIO_OD_GP0_BYTE_VAL |= BIT4;
#endif
	

	GPIO_PULL_EN_GP14_SD_BYTE_VAL |= (GPIO_SD0_CLK |
				GPIO_SD0_CMD |
				GPIO_SD0_DATA1);
	GPIO_PULL_CTRL_GP14_SD_BYTE_VAL |= (GPIO_SD0_CMD | GPIO_SD0_DATA1);
	GPIO_PULL_CTRL_GP14_SD_BYTE_VAL &= ~(GPIO_SD0_CLK);
	if (g_use_ss0) {
		GPIO_CTRL_GP14_SD_BYTE_VAL &= ~GPIO_SD0_DATA0;
		GPIO_PULL_EN_GP14_SD_BYTE_VAL |= GPIO_SD0_DATA0;
		GPIO_PULL_CTRL_GP14_SD_BYTE_VAL |= GPIO_SD0_DATA0;
	} else
		 GPIO_CTRL_GP14_SD_BYTE_VAL |= GPIO_SD0_DATA0;
	if (g_use_ss1) {
		GPIO_CTRL_GP14_SD_BYTE_VAL &= ~GPIO_SD0_WP;
		GPIO_PULL_EN_GP14_SD_BYTE_VAL |= GPIO_SD0_WP;
		GPIO_PULL_CTRL_GP14_SD_BYTE_VAL |= GPIO_SD0_WP;
	} else
		GPIO_CTRL_GP14_SD_BYTE_VAL |= GPIO_SD0_WP;
	if (g_use_ss2) {
		GPIO_CTRL_GP14_SD_BYTE_VAL &= ~GPIO_SD0_DATA2;
		GPIO_PULL_EN_GP14_SD_BYTE_VAL |= GPIO_SD0_DATA2;
		GPIO_PULL_CTRL_GP14_SD_BYTE_VAL |= GPIO_SD0_DATA2;
	} else
		GPIO_CTRL_GP14_SD_BYTE_VAL |= GPIO_SD0_DATA2;
	if (g_use_ss3) {
		GPIO_CTRL_GP14_SD_BYTE_VAL &= ~GPIO_SD0_DATA3;
		GPIO_PULL_EN_GP14_SD_BYTE_VAL |= GPIO_SD0_DATA3;
		GPIO_PULL_CTRL_GP14_SD_BYTE_VAL |= GPIO_SD0_DATA3;
	} else
		GPIO_CTRL_GP14_SD_BYTE_VAL |= GPIO_SD0_DATA3;

	/* clock open */
	auto_pll_divisor(DEV_SPI1, CLK_ENABLE, 0, 0);
	pllb_input_freq = auto_pll_divisor(DEV_SPI1, SET_DIV, 1, 100000);
	pllb_input_freq /= 1000;

	/* check if config successful */
	while (timeout--) {
		if (!(REG8_VAL(PMC_REG_BASE) & 0x08000000))
			return ;
		udelay(1);
	}
	printk(KERN_ERR "Enable SPI1 clock from PMC module failed\n!");
}

/*
 * wmt_spi_clock_disable: reset GPIO and disable spi clock from 
 *  Power management module 
 */
static inline void wmt_spi_clock_disable(void)
{
#if 0
	GPIO_CTRL_GP11_SPI_BYTE_VAL |= (GPIO_SPI0_CLK |
			GPIO_SPI0_MOSI|
			GPIO_SPI0_SS|
			GPIO_SPI0_MISO|
			GPIO_SPI0_SSB);
#endif
}

/*
 * spi_is_busy: check spi controller(master) is busy or not 
 * @spi: spi controller's driver data
 */
static int spi_is_busy(struct wmt_spi *spi)
{
	unsigned int timeout = POLLING_SPI_REG_TIMEOUT;
	while (timeout--) {
		if ((spi_get_reg32(spi, SPI_SR) & SPI_SR_BUSY_MASK) == 0)
			return 0;
	}
	return -EAGAIN;  
}

/*
 * spi_enable: enable spi module
 * @spi: spi controller's driver data
 */
static void spi_enable(struct wmt_spi *spi)
{
	spi_setbit(spi, SPI_CR, SPI_CR_ME_MASK);
}

/*
 * spi_disable: disable spi module
 * @spi: spi controller's driver data
 */
static void spi_disable(struct wmt_spi *spi)
{
	if (spi_is_busy(spi)) {
		dev_warn(&spi->master->dev, "Disable spi controller failed\n");
	}
	spi_clrbit(spi, SPI_CR, SPI_CR_ME_MASK);
}

/*
 * spi_tx_is_finish: polling if data in tx fifo has been sent out 
 * @spi: spi controller's driver data
 */
static int spi_tx_is_finish(struct wmt_spi *spi)
{
	unsigned int timeout = POLLING_SPI_REG_TIMEOUT;
	while (timeout--) {
		if (spi_get_reg32(spi, SPI_SR) | SPI_SR_TFEI_MASK)
			return 0;
	}
	return -EAGAIN;
}

/*
 * spi_rx_is_finish: polling if data in rx fifo has been read out 
 * @spi: spi controller's driver data
 */
static int spi_rx_is_finish(struct wmt_spi *spi)
{
	unsigned int timeout = POLLING_SPI_REG_TIMEOUT;
	while (timeout--) {
		if ((spi_get_reg32(spi, SPI_SR) | SPI_SR_RFEI_MASK))
			return 0;
	}
	return -EAGAIN;
}

/*
 * wmt_spi_cs_active: enable chip select signal. SSN is driven 
 *    low to active if spi controller use SSN_CTRL_PROGRAM mode
 * @spi: spi controller's driver data
 */
static inline void wmt_spi_cs_active(struct spi_device *spi_dev, struct wmt_spi *spi)
{
	struct wmt_spi_hw *hw_info = spi->spi_hw_info;

	/* enable SSN */
	if (hw_info->ssn_ctrl == SSN_CTRL_PROGRAM)
		spi_clrbit(spi, SPI_DFCR, SPI_DFCR_DSV_MASK | SPI_DFCR_DSE_MASK);
}

/*
 * wmt_spi_cs_inactive: disable chip select signal. A SSN is driven
 *   high to inactive if spi controller use SSN_CTRL_PROGRAM mode
 * @spi: spi controller's driver data
 */
static inline void wmt_spi_cs_inactive(struct spi_device *spi_dev, struct wmt_spi *spi)
{
	struct wmt_spi_hw *hw_info = spi->spi_hw_info;

	if (hw_info->ssn_ctrl == SSN_CTRL_PROGRAM)
		spi_setbit(spi, SPI_DFCR, SPI_DFCR_DSV_MASK | SPI_DFCR_DSE_MASK);
}

/*
 * get_spi_input_freq: get spi original input frequence from power 
 *  management module 
 */
static unsigned int get_spi_input_freq(void)
{
	return pllb_input_freq;
}

/*
 * spi_set_clock_div: set clock divider in spi control register,
 *    if the divisor is too low or too high, set spi working 
 *    frequency 5Mhz as default 
 * @spi: spi controller's driver data
 * @speed_hz: spi working clock frequency, unit is hz
 */
static void spi_set_clock_div(struct wmt_spi *spi, int speed_hz)
{
	unsigned int divisor;
	unsigned int hw_freq = get_spi_input_freq()*1000; /* KHz to Hz */

	spi_clrbit(spi, SPI_CR, SPI_CR_TCD_MASK);
	if (!speed_hz)
		goto err;

	divisor = hw_freq/(2*speed_hz);
	if (divisor < 0 || divisor > 0x7ff) /* spi cr bit(21,31), max:0x7ff */
		goto err;

	spi_setbit(spi, SPI_CR, divisor << SPI_CR_TCD_SHIFT);
	return ;

err:
	divisor = hw_freq/(2*SPI_DFLT_FREQ);
	spi_setbit(spi, SPI_CR, divisor << SPI_CR_TCD_SHIFT);
	dev_err(&spi->master->dev, "SPI frequency %dhz not support, " \
	    "set %dhz as default\n", speed_hz, SPI_DFLT_FREQ);
	return ;
}

/*
 * spi_set_bit_order: set spi redeive/transmit significant bit order
 * @spi: spi controller's driver data
 * @mode: spi device working mode
 */
static void spi_set_bit_order(struct wmt_spi *spi, u8 mode)
{
	if (mode & SPI_LSB_FIRST)
		spi_setbit(spi, SPI_DFCR, SPI_DFCR_RSBO_MASK | SPI_DFCR_TSBO_MASK);
}

/*
 * spi_set_clock_mode: set spi clock polarity and phase (spi clock mode)
 * @spi: spi controller's driver data
 * @clk_mode: spi clock mode
 */
static void spi_set_clock_mode(struct wmt_spi *spi, u8 clk_mode)
{
	spi_clrbit(spi, SPI_CR, SPI_CR_CPHS_MASK);
	if (clk_mode > SPI_CLK_MODE3)
		goto err;
	spi_setbit(spi, SPI_CR, clk_mode << SPI_CR_CPHS_SHIFT);
	return ;
err:
	spi_setbit(spi, SPI_CR, SPI_CLK_MODE3 << SPI_CR_CPHS_SHIFT);
	dev_err(&spi->master->dev, "clock mode err, set clock mode 3 as default\n");
	return ;
}

/*
 * spi_as_master: spi master/slave select
 * @spi: spi controller's driver data
 * @is_master: if spi configured as master, is_master = 1, else is_master = 0
 */
static void spi_as_master(struct wmt_spi *spi, u8 is_master)
{
	if (is_master)
		spi_clrbit(spi, SPI_CR, SPI_CR_MSMS_MASK);
	else
		spi_setbit(spi, SPI_CR, SPI_CR_MSMS_MASK);
}

/*
 * spi_reset_tx_fifo: reset spi transmit fifo
 * @spi: spi controller's driver data
 */
static int spi_reset_tx_fifo(struct wmt_spi *spi)
{
	unsigned int timeout = POLLING_SPI_REG_TIMEOUT;
	spi_setbit(spi, SPI_CR, SPI_CR_TFR_MASK);
	while (timeout--) {
		if ((spi_get_reg32(spi, SPI_CR) & SPI_CR_TFR_MASK) == 0)
        		return 0;
	}
	return -1;
}

/*
 * spi_reset_rx_fifo: reset spi receive fifo
 * @spi: spi controller's driver data
 */
static int spi_reset_rx_fifo(struct wmt_spi *spi)
{
	unsigned int timeout = POLLING_SPI_REG_TIMEOUT;
	spi_setbit(spi, SPI_CR, SPI_CR_RFR_MASK);
	while (timeout--) {
		if ((spi_get_reg32(spi, SPI_CR) & SPI_CR_RFR_MASK) == 0)
        		return 0;
	}
	return -EAGAIN;
}

/*
 * spi_reset_fifo: reset both spi transmit fifo and receive fifo
 * @spi: spi controller's driver data
 */
static int spi_reset_fifo(struct wmt_spi *spi)
{
	if (spi_reset_tx_fifo(spi))
		return -EAGAIN;
	if (spi_reset_rx_fifo(spi))
		return -EAGAIN;
	return 0;
}

/*
 * spi_reset: reset spi status register and reset spi tx and rx fifo
 * @spi: spi controller's driver data
 */
static int spi_reset(struct wmt_spi *spi)
{
	spi_set_reg32(spi, SPI_SR, ~0UL);
	return spi_reset_fifo(spi);
}

/*
 * spi_reset_transfer_speed: reset spi work frequency according 
 *  to spi_transfer
 * @spi: spi controller's driver data
 * @t: spi transfer
 */
static void 
spi_reset_transfer_speed(struct wmt_spi *spi, struct spi_transfer *t)
{
	if (t && t->speed_hz)
		spi_set_clock_div(spi, t->speed_hz);
}

/*
 * wmt_spi_regs_config: config spi registers according to the 
 *   information get from spi_device
 @spi_dev: Master side proxy for an SPI slave device
 */
static int wmt_spi_regs_config(struct spi_device *spi_dev)
{
	int ret = 0;
	u8 clk_mode;
	struct wmt_spi *wmt_spi;
	struct wmt_spi_slave *slave_info;
	struct wmt_spi_hw *hw_info;

	wmt_spi = spi_master_get_devdata(spi_dev->master);
	slave_info = spi_dev->controller_data;
	hw_info    = wmt_spi->spi_hw_info;

	/* clear spi control register               */
	spi_set_reg32(wmt_spi, SPI_CR, 0x00UL);
	/* clear spi status register                */
	spi_set_reg32(wmt_spi, SPI_SR, ~0UL);
	/* spi cre register */
	spi_set_reg32(wmt_spi, SPI_CRE, 0x20);

	/* reset tx and rx fifo                     */
	ret = spi_reset_fifo(wmt_spi);
	if (ret)
		goto err;
	/* setting spi controller register          */
	/* 1. set spi clock divider                 */
	spi_set_clock_div(wmt_spi, spi_dev->max_speed_hz);
	/* 2 set clock mode                         */
	clk_mode = (spi_dev->mode & (BIT0 | BIT1));
	spi_set_clock_mode(wmt_spi, clk_mode);
	/* 3. spi as master                         */
	spi_as_master(wmt_spi, 1);
	/* 4. slave selection                       */
	/*
	spi_setbit(wmt_spi, SPI_CR, spi_dev->chip_select << SPI_CR_SS_SHIFT);
	*/
	/*for wm3465 A0*/
	spi_setbit(wmt_spi, SPI_CR, 0 << SPI_CR_SS_SHIFT);

	/* setting spi data format control register */
	/* 1. port mode setting                     */
	if (PORT_MODE_PTP == hw_info->port_mode)
		spi_setbit(wmt_spi, SPI_DFCR, SPI_DFCR_SPM_MASK);
	/* 2. spi tx/rx significant bit order       */
	spi_set_bit_order(wmt_spi, spi_dev->mode);
	/* 3. ssn control setting                   */
	if (SSN_CTRL_PROGRAM == hw_info->ssn_ctrl) {
		if ((clk_mode & SPI_CR_CPHS_MASK) == 0) {
			dev_warn(&spi_dev->dev, "SSN_ctrl conflict with clock mode\n");
			/* do not abort now, the conflict is not a serious problem, 
			   driver can handle this well, so we work on */
			goto err;
		}
		spi_setbit(wmt_spi, SPI_DFCR, SPI_DFCR_SC_MASK);
	}
	return 0;

err:
	dev_err(&wmt_spi->pdev->dev, "SPI config register error\n");
	return ret;
}

/*
 * wmt_spi_fifo_tx: transmit data by spi TX_FIFO.
 * @spi: spi controller's driver data
 * @tx_buf: data transmited by spi tx fifo
 * @len: tx buffers length. Note: len is not checked in this function, 
 *     and wmt spi fifo size is 32bytes, so caller must be sure 
 *     len <= 32bytes
 */
static void
wmt_spi_fifo_tx(struct wmt_spi *spi, const u8 *tx_buf, int len)
{
	int i;
	/* load data to tx fifo */
	if (tx_buf) {
		for (i = 0; i < len; i++)
			spi_write_fifo(spi, SPI_TXFIFO, tx_buf[i]);
	} /* load idle data to tx fifo */else {
		for (i = 0; i < len; i++)
			spi_write_fifo(spi, SPI_TXFIFO, 0x00);
	}
}

/*
 * wmt_spi_write_read: spi transfer routine
 * @spi_dev: Master side proxy for an SPI slave device
 * @t: spi transfer
 *     wmt spi transmit fifo is SPI_FIFO_SIZE(32bytes), 
 *     so if transmit data more than 32bytes, we need transmit 
 *     those data more than one time 
 **/
static int 
wmt_spi_write_read(struct spi_device *spi, struct spi_transfer *t)
{
	int i = 0;
	const u8 *tx = t->tx_buf;
	u8 *rx = t->rx_buf;
	unsigned int cnt = t->len;
	struct wmt_spi *wmt_spi = spi_master_get_devdata(spi->master);

	/* reset spi first */
	if (spi_reset(wmt_spi))
		goto out;
	/* if data length more than SPI_FIFO_SIZE(32bytes) */
	while (cnt >= SPI_FIFO_SIZE) {
		/* load data form tx_buf to tx_fifo */
		wmt_spi_fifo_tx(wmt_spi, tx, SPI_FIFO_SIZE);
		/* send data */
		spi_enable(wmt_spi);
		/* waitting for send finish */
		if (spi_tx_is_finish(wmt_spi))
			goto out;
		/* data send has finished */
		spi_disable(wmt_spi);
		/* read data and stroe in rx_buf */
		if (rx) {
			for (i = 0; i < SPI_FIFO_SIZE; i++)
				*rx++ = spi_read_fifo(wmt_spi, SPI_RXFIFO);
		}
		/* check if rx finish */
		if (spi_rx_is_finish(wmt_spi))
			goto out;
		if (tx)
			tx += SPI_FIFO_SIZE;
		cnt -= SPI_FIFO_SIZE;
		if (t->speed_hz < 5*1000*1000)
			udelay(2000);
		/* reset for the next transfer */
		if (spi_reset(wmt_spi))
			goto out;
	}
	/* remain data transfer */
	if (cnt && cnt < SPI_FIFO_SIZE) {
		wmt_spi_fifo_tx(wmt_spi, tx, cnt);
		spi_enable(wmt_spi);
		if (spi_tx_is_finish(wmt_spi))
		    goto out;
		spi_disable(wmt_spi);
		if (rx) {
			for (; cnt > 0; cnt--)
				*rx++ = spi_read_fifo(wmt_spi, SPI_RXFIFO);
		} else {
			for (; cnt > 0; cnt--)
				spi_read_fifo(wmt_spi, SPI_RXFIFO);
		}
	}
out:
	return (t->len - cnt);
}

/*
 * spi_dsr_w: spi dma transmit callback function
 * @arg: point to wmt_spi
 **/
static void spi_dsr_w(void *arg)
{
	struct wmt_spi *spi = (struct wmt_spi *)arg;
	struct wmt_spi_dma *spi_dma = spi->spi_dma_info;
	spi_dma->tx_ack = 1;
	wake_up_interruptible(&spi_dma->tx_event);
}

/*
 * spi_dsr_r: spi dma receive callback function
 * @arg: point to wmt_spi
 **/
static void spi_dsr_r(void *arg)
{
	struct wmt_spi *spi = (struct wmt_spi *)arg;
	struct wmt_spi_dma *spi_dma = spi->spi_dma_info;
	spi_dma->rx_ack = 1;
	wake_up_interruptible(&spi_dma->rx_event);
}

/*
 * wmt_spi_dma_write_read: spi transfer (use DMA mode)
 * @spi_dev: Master side proxy for an SPI slave device
 * @t: spi transfer1
 **/
static int 
wmt_spi_dma_write_read(struct spi_device *spi_dev, struct spi_transfer *t)
{
	struct wmt_spi *wmt_spi = spi_master_get_devdata(spi_dev->master);
	struct wmt_spi_dma *spi_dma = wmt_spi->spi_dma_info;
	wait_queue_head_t *event = &spi_dma->tx_event;
	volatile int *ack = &spi_dma->tx_ack;
	unsigned int transfered_cnt = 0;
	u32 ctrl;

	/* spi dma transfer need cs inactive first*/
	wmt_spi_cs_inactive(spi_dev, wmt_spi);
	if (t->speed_hz > 15000000)
		spi_set_clock_div(wmt_spi, 15000000);
	ctrl = spi_get_reg32(wmt_spi, SPI_CR);
	ctrl |= SPI_CR_DRC_MASK | SPI_CR_RFTS_MASK | SPI_CR_TFTS_MASK;
	spi_set_reg32(wmt_spi, SPI_CR, ctrl);  
	/* reset spi fifo */
	if (spi_reset(wmt_spi))
		goto out;
	/* tx dma buffer prepare */
	if (t->tx_buf)
		memcpy(spi_dma->io_waddr, t->tx_buf, t->len);
	/* tx dma request */
	if (wmt_request_dma(&spi_dma->tx_ch, "wmt_spi_tx", 
			    spi_dma->tx_config.DeviceReqType, 
			    spi_dsr_w, wmt_spi)) {
		dev_err(&spi_dev->dev, "SPI request TX DMA failed\n");
		goto out;                        
	}
	/* rx dma request and start */
	if (t->rx_buf) {
		if (wmt_request_dma(&spi_dma->rx_ch, "wmt_spi_rx",
				  spi_dma->rx_config.DeviceReqType,
				  spi_dsr_r, wmt_spi)) {
			dev_err(&spi_dev->dev, "SPI request RX DMA failed\n");
			goto free_tx_dma;
		}
		wmt_setup_dma(spi_dma->rx_ch, spi_dma->rx_config);
		wmt_start_dma(spi_dma->rx_ch, spi_dma->phys_raddr, 0x00, t->len);
		event = &spi_dma->rx_event;
		ack = &spi_dma->rx_ack;
	}
	/* transmit dma setup and start */
	wmt_setup_dma(spi_dma->tx_ch, spi_dma->tx_config);
	wmt_start_dma(spi_dma->tx_ch, spi_dma->phys_waddr, 0x00, t->len + 7);
	/* enable spi and active chipselect signal */
	msleep(2);
	spi_enable(wmt_spi);
	/* waitting for transmit finish */
	msleep(2);
	wmt_spi_cs_active(spi_dev, wmt_spi);
	/* waitting transfer finish */
	if (!wait_event_interruptible_timeout(*event, *ack, 100)) {
		dev_err(&spi_dev->dev, "SPI DMA transfer failed\n");
		goto out;
	}
	transfered_cnt = t->len;
	/* if RX buf is not empty, copy received data from dma to t->rx_buf */
	if (t->rx_buf) {
		memcpy(t->rx_buf, spi_dma->io_raddr, t->len);
		wmt_free_dma(spi_dma->rx_ch);
		memset(spi_dma->io_raddr, 0x00, t->len);
	}
free_tx_dma:
	wmt_free_dma(spi_dma->tx_ch);
	memset(spi_dma->io_waddr, 0x00, t->len);    
out:
	spi_disable(wmt_spi);
	spi_dma->rx_ack = spi_dma->tx_ack = 0;
	return transfered_cnt;
}

/*
 * wmt_spi_work: wmt spi controller work queue routine function.
 *   Check every message which queued in workqueue (wmt_spi->queue),
 *   if all things correct, message dequeued and begin transfer routine
 * @work: work which queued in spi controller's work queue
 */
static void wmt_spi_work(struct work_struct *work)
{   
	struct wmt_spi *wmt_spi;
	struct wmt_spi_slave *spi_ctrl;

	wmt_spi = container_of(work, struct wmt_spi, work);
	spin_lock_irq(&wmt_spi->spinlock);

	while (!list_empty(&wmt_spi->queue)) {
		int status    = 0;
		u8 cs_active = 0;
		struct spi_message *m  = NULL;
		struct spi_transfer *t = NULL;
		struct spi_device *spi;

		m = container_of(wmt_spi->queue.next, struct spi_message, queue);
		spi_ctrl = m->spi->controller_data;

		list_del_init(&m->queue);
		spin_unlock_irq(&wmt_spi->spinlock);

		spi = m->spi;
		status = wmt_spi_regs_config(m->spi);
		if (status < 0)
			goto msg_done;
		/* check every transfer which queued in message, 
		 * if all things right, begin data transfer      */
		list_for_each_entry(t, &m->transfers, transfer_list) {            
			spi_reset_transfer_speed(wmt_spi, t);

			if (!cs_active) {
				wmt_spi_cs_active(spi, wmt_spi);
				cs_active = 1;
			}

			/* data transfer begins here */
			if (t->len) {
				if (wmt_spi->spi_hw_info->dma_support && m->is_dma_mapped
				    && spi_ctrl->dma_en && t->len > SPI_FIFO_SIZE) {
					m->actual_length += wmt_spi_dma_write_read(m->spi, t);
				} else {
					m->actual_length += wmt_spi_write_read(m->spi, t);
				}
			}
			/* some device need this feature support */
			if (t->delay_usecs)
				udelay(t->delay_usecs);

			/* if cs need change in next transfer, just inactive it */
			if (t->cs_change) {
			    wmt_spi_cs_inactive(spi, wmt_spi);
			    cs_active = 0;
			}       
		}

msg_done:
		if (status < 0)
			dev_err(&wmt_spi->master->dev, "SPI transfer error!\n");
		if (cs_active)
			wmt_spi_cs_inactive(spi, wmt_spi);
		m->status = status;
		if (m->complete && m->context)
			m->complete(m->context);
		spin_lock_irq(&wmt_spi->spinlock);
	} 
	spin_unlock(&wmt_spi->spinlock);
}

/*
 * bits_per_word_is_support: check if spi controller can support 
 *    this bits_per_word type or not
 * @spi: spi controller's driver data
 * @bit_per_word: bits_per_word type will be check 
 */
static int 
bits_per_word_is_support(struct wmt_spi *spi, u8 bit_per_word)
{
	int ret = 0;
	struct wmt_spi_hw *hw_info = spi->spi_hw_info;

	if ((bit_per_word != 8) && (bit_per_word != 16 )) {
		ret = -EINVAL;
		goto err;
	}

	if ((bit_per_word == 8) && 
	    !(hw_info->bits_per_word_en & BITS8_PER_WORD_EN)) {
		ret = -EINVAL;
		goto err;
	}

	if ((bit_per_word == 16) && 
	    !(hw_info->bits_per_word_en & BITS16_PER_WORD_EN)) {
		ret = -EINVAL;
		goto err;
	}    
err:
	return ret;
}

/*
 * speed_hz_is_support: check spi controller can support this 
 *    frequency or not
 * @spi: spi controller's driver data
 * @speed_hz: frequency spi controller will working with 
 */
static int speed_hz_is_support(struct wmt_spi *spi, u32 speed_hz)
{
	int ret = 0;
	struct wmt_spi_hw *hw_info = spi->spi_hw_info;

	if (speed_hz < hw_info->min_freq_hz) {
		ret = -EINVAL;
		goto err;
	}

	if (speed_hz > hw_info->max_freq_hz) {
		ret = -EINVAL;
	}

err:
	return ret;
}

/*
 * wmt_spi_transfer: check transfers queued in spi message, 
 *    then queued spi message in workqueue
 *@spi_dev: Master side proxy for an SPI slave device
 * @m: spi message which will be added to the queue
 */
/* bidirectional bulk transfers
 *
 * + The transfer() method may not sleep; its main role is
 *   just to add the message to the queue.
 * + For now there's no remove-from-queue operation, or
 *   any other request management
 * + To a given spi_device, message queueing is pure fifo
 *
 * + The master's main job is to process its message queue,
 *   selecting a chip then transferring data
 * + If there are multiple spi_device children, the i/o queue
 *   arbitration algorithm is unspecified (round robin, fifo,
 *   priority, reservations, preemption, etc)
 *
 * + Chipselect stays active during the entire message
 *   (unless modified by spi_transfer.cs_change != 0).
 * + The message transfers use clock and SPI mode parameters
 *   previously established by setup() for this device
 */
static int 
wmt_spi_transfer(struct spi_device *spi_dev, struct spi_message *m)
{
	struct wmt_spi *wmt_spi = spi_master_get_devdata(spi_dev->master);
	struct spi_transfer *t        = NULL;
	unsigned long flags           = 0;

	m->actual_length = 0;
	m->status = 0;
	/* reject invalid messages and transfers */
	if (list_empty(&m->transfers) || !m->complete) {
		//if (list_empty(&m->transfers)) { // for none sleep
		dev_err(&spi_dev->dev, "msg rejected: invalid message of transfer\n");
		goto msg_reject;
	}
	/* transfers members bits_per_word and speed_hz checking and setting */
	list_for_each_entry(t, &m->transfers, transfer_list) {
		if ((!t->tx_buf) && (!t->rx_buf) && t->len) {
			dev_err(&spi_dev->dev, "msg rejected: invalid message data\n");
			goto msg_reject;
		}

		/* checking transfer length */
		if (t->len > wmt_spi->spi_hw_info->max_transfer_length) {
			dev_err(&spi_dev->dev, "msg rejected: transfer lenth can not be"\
			    "bigger than %d, please split it into smaller chunks"\
			    "and submit them separately\n", 
			    wmt_spi->spi_hw_info->max_transfer_length);
			goto msg_reject;
		}

		/* checking and resetting transfer bits_per_word */
		if (!t->bits_per_word 
		    || bits_per_word_is_support(wmt_spi, t->bits_per_word))
			t->bits_per_word = spi_dev->bits_per_word;

		/* checking and resetting transfer speed_hz */
		if (!t->speed_hz || speed_hz_is_support(wmt_spi, t->speed_hz))     
			t->speed_hz = spi_dev->max_speed_hz;
	}
	spin_lock_irqsave(&wmt_spi->spinlock, flags);
	/* add this message into spi's queue */
	list_add_tail(&m->queue, &wmt_spi->queue);
	/* doing message transfer routine in spi work */
	queue_work(wmt_spi->workqueue, &wmt_spi->work);
	spin_unlock_irqrestore(&wmt_spi->spinlock, flags);
	return 0;

msg_reject:
	m->status = -EINVAL;
	if (m->complete)
		m->complete(m->context);
	return -EINVAL;
}

/*
 * wmt_spi_setup: updates the device mode, bits_per_word and clocking 
 *  records used by a device's SPI controller; protocol code may call 
 *  this.  This must fail if an unrecognized or unsupported mode is 
 *  requested.It's always safe to call this unless transfers are pending 
 *  on the device whose settings are being modified.
 * @spi_dev: Master side proxy for an SPI slave device
 */
#define MODEBITS (SPI_CPOL | SPI_CPHA | SPI_LOOP | SPI_LSB_FIRST)
static int wmt_spi_setup(struct spi_device *spi_dev)
{
	int ret = 0;
	struct wmt_spi *wmt_spi;
	struct wmt_spi_slave *slave_info;
	struct wmt_spi_hw *hw_info;

	wmt_spi = spi_master_get_devdata(spi_dev->master);
	slave_info = spi_dev->controller_data;
	hw_info    = wmt_spi->spi_hw_info;

	/* mode checking */
	if (spi_dev->mode & ~MODEBITS) {
		dev_err(&spi_dev->dev, "SPI unsupported this mode 0x%08x\n", 
			spi_dev->mode & ~MODEBITS);
		ret = -EINVAL;
		goto err;
	}

	/* bits_per_word checking */
	if (!spi_dev->bits_per_word)
		spi_dev->bits_per_word = slave_info->bits_per_word;

	if (spi_dev->bits_per_word) {
		if (bits_per_word_is_support(wmt_spi, spi_dev->bits_per_word)) {
			if (bits_per_word_is_support(wmt_spi, 
				slave_info->bits_per_word)) {
				dev_err(&spi_dev->dev, "SPI unsupport %d and %dbits_per_word\n", 
					spi_dev->bits_per_word, slave_info->bits_per_word);
				ret = -EINVAL;
				goto err;
			}
			spi_dev->bits_per_word = slave_info->bits_per_word;
		}
	}

	/* max_speed_hz checking */
	if ((spi_dev->max_speed_hz == 0) || 
	    (spi_dev->max_speed_hz > hw_info->max_freq_hz))
		spi_dev->max_speed_hz = hw_info->max_freq_hz;

	if (spi_dev->max_speed_hz < hw_info->min_freq_hz) {
		dev_err(&spi_dev->dev, "SPI unspport speed lower than %dhz\n", 
			hw_info->min_freq_hz);
		ret = -EINVAL;
		goto err;
	}
	return 0;

err:
	dev_err(&spi_dev->dev, "SPI setup spi device failed!\n");
	return ret;
}

/*
 * wmt_spi_cleanup: called on module_exit to free memory 
 *   provided by spi_master
 * @wmt_spi: spi controller driver data
 */
static void wmt_spi_cleanup(struct wmt_spi *wmt_spi)
{
	if (wmt_spi->spi_hw_info)
		kfree(wmt_spi->spi_hw_info);
	wmt_spi->spi_hw_info = NULL;

	if (wmt_spi->spi_dma_info)
		kfree(wmt_spi->spi_dma_info);
	wmt_spi->spi_dma_info = NULL; 
}

/*
 * spi_get_hw_info: get spi controller's configuration infomation 
 *   form platform device 
 * @pdev: spi platform device structure
 */
static inline void *spi_get_hw_info(struct platform_device *pdev)
{
	return pdev->dev.platform_data;
}

/*
 * wmt_spi_dma_init: request spi dma memory and initialize spi dma info
    if dma supported
 * @spi: spi controller's driver data
 */
static int wmt_spi_dma_init(struct wmt_spi *spi)
{
	int ret  = 0;
	unsigned int dma_size;

	if (!spi->spi_hw_info)
		goto out;
	if (!spi->spi_hw_info->dma_support)
		goto out;
	spi->spi_dma_info = kmalloc(sizeof(struct wmt_spi_dma), GFP_KERNEL);
	if (!spi->spi_dma_info) {
		ret = -ENOMEM;
		dev_err(&spi->pdev->dev, "SPI allocating dma info memory failed\n");
		goto out;
	}
	dma_size = spi->spi_hw_info->max_transfer_length;
	/* dma read config */
	spi->spi_dma_info->rx_ch = ~0UL;
	spi->spi_dma_info->rx_config.ChunkSize     = SPI_DMA_CHUNK_SIZE;
	spi->spi_dma_info->rx_config.DefaultCCR    = SPI_RX_DMA_CFG;
	spi->spi_dma_info->rx_config.DeviceReqType = SPI0_DMA_RX_REQ;
	spi->spi_dma_info->rx_config.MIF1addr      = SPI0_BASE_ADDR + SPI_RXFIFO;
	spi->spi_dma_info->io_raddr = dma_alloc_coherent(&spi->pdev->dev, 
					dma_size,
					&spi->spi_dma_info->phys_raddr, 
					GFP_KERNEL | GFP_DMA);
	if (!spi->spi_dma_info->io_raddr) {
		ret = -ENOMEM;
		dev_err(&spi->pdev->dev, "SPI allocate rdma failed\n");
		goto out;
	}
	memset(spi->spi_dma_info->io_raddr, 0x00, dma_size);
	spi->spi_dma_info->rx_config.MIF0addr = (ulong)spi->spi_dma_info->io_raddr;
	init_waitqueue_head(&spi->spi_dma_info->rx_event);
	spi->spi_dma_info->rx_ack = 0;
	/* dma write config */
	spi->spi_dma_info->tx_ch = ~0UL;
	spi->spi_dma_info->tx_config.ChunkSize     = SPI_DMA_CHUNK_SIZE;
	spi->spi_dma_info->tx_config.DefaultCCR    = SPI_TX_DMA_CFG;
	spi->spi_dma_info->tx_config.DeviceReqType = SPI0_DMA_TX_REQ;
	spi->spi_dma_info->tx_config.MIF1addr      = SPI0_BASE_ADDR + SPI_TXFIFO;
	spi->spi_dma_info->io_waddr = dma_alloc_coherent(&spi->pdev->dev, 
					dma_size + 7,
					&spi->spi_dma_info->phys_waddr,
					GFP_KERNEL | GFP_DMA);
	if (!spi->spi_dma_info->io_waddr) {
		ret = -ENOMEM;
		dev_err(&spi->pdev->dev, "SPI allocate wdma failed\n");
		goto free_spi_rx_dma;
	}
	memset(spi->spi_dma_info->io_waddr, 0x00, dma_size + 7);
	spi->spi_dma_info->tx_config.MIF0addr = (ulong)spi->spi_dma_info->io_waddr;
	init_waitqueue_head(&spi->spi_dma_info->tx_event);
	spi->spi_dma_info->tx_ack = 0;
	return 0;

free_spi_rx_dma:
	dma_free_coherent(&spi->pdev->dev,dma_size,spi->spi_dma_info->io_raddr,
			   spi->spi_dma_info->phys_raddr);
out:
    return ret;
}

/*
 * wmt_spi_dma_release: release spi dma memory if dma supported
 * @spi: spi controller's driver data
 */
static void wmt_spi_dma_release(struct wmt_spi *spi)
{
	if (!spi->spi_hw_info)
	    goto out;
	if (!spi->spi_hw_info->dma_support)
	    goto out;

	dma_free_coherent(&spi->pdev->dev, 
		    spi->spi_hw_info->max_transfer_length, 
		    spi->spi_dma_info->io_raddr,
		    spi->spi_dma_info->phys_raddr);
	dma_free_coherent(&spi->pdev->dev,
		    spi->spi_hw_info->max_transfer_length + 7, 
		    spi->spi_dma_info->io_waddr,
		    spi->spi_dma_info->phys_waddr);
out:
	return ;
}

static int __devinit wmt_spi_probe(struct platform_device *pdev)
{
	int ret;
	struct device *dev = &pdev->dev;
	struct spi_master *master;
	struct wmt_spi *wmt_spi;
	struct wmt_spi_hw *wmt_dev_info;
	struct resource *res;

	/* SPI master allocation */
	master = spi_alloc_master(dev, sizeof(struct wmt_spi));
	if (!master) {
		ret = -ENOMEM;
		dev_err(dev, "SPI master allocation failed!\n");
		goto the_end;
	}

	/* spi controller structure initialization */
	wmt_spi = (struct wmt_spi *)spi_master_get_devdata(master);
	memset(wmt_spi, 0x00, sizeof(struct wmt_spi));
	wmt_spi->master = spi_master_get(master);
	wmt_spi->pdev = pdev;
	wmt_dev_info = spi_get_hw_info(pdev);
	wmt_spi->spi_hw_info = kmalloc(sizeof(struct wmt_spi_hw), GFP_KERNEL);
	if (!wmt_spi->spi_hw_info) {
		dev_err(dev, "SPI allocating hardware info memory failed\n");
		ret = -ENOMEM;
		goto release_master;
	}
	memset(wmt_spi->spi_hw_info, 0x00, sizeof(struct wmt_spi_hw));
	memcpy(wmt_spi->spi_hw_info, wmt_dev_info,
	       sizeof(struct wmt_spi_hw));

	/* spi master initialization */
	master->bus_num         = pdev->id;
	master->num_chipselect  = wmt_dev_info->num_chipselect;
	master->setup           = wmt_spi_setup;
	master->transfer        = wmt_spi_transfer;

	/* the spi->mode bits understood by this driver: */
	master->mode_bits = SPI_CPOL | SPI_CPHA | SPI_CS_HIGH;

	platform_set_drvdata(pdev, wmt_spi);

	/* device resource request */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		ret = -ENXIO;
		dev_err(dev, "SPI getting platform resource failed\n");
		goto release_hw_info;
	}
#if 0
	if (!request_mem_region(res->start, res->end-res->start+1, pdev->name)) {
		ret = -ENXIO;
		dev_err(dev, "SPI request mem region failed\n");
		goto release_hw_info;
	}
	wmt_spi->regs_base = ioremap(res->start, res->end - res->start + 1);
#endif
	wmt_spi->regs_base = (void __iomem *)SPI1_BASE_ADDR;
	if (!wmt_spi->regs_base) {
		ret = -ENXIO;
		dev_err(dev, "SPI ioremap failed!\n");
		goto release_region;
	}

	if (wmt_dev_info->dma_support) {
		if (wmt_spi_dma_init(wmt_spi)) {
			ret = -ENXIO;
			wmt_dev_info->dma_support = 0;
			dev_err(dev, "SPI dma init failed\n");
			goto release_ioremap;
		}          
	}

	/* work queue create */
	spin_lock_init(&wmt_spi->spinlock);
	INIT_LIST_HEAD(&wmt_spi->queue);
	INIT_WORK(&wmt_spi->work, wmt_spi_work);
	init_waitqueue_head(&wmt_spi->waitq);
	wmt_spi->workqueue = create_singlethread_workqueue("wmt_spi");
	if (NULL == wmt_spi->workqueue) {
		ret = -ENXIO;
		dev_err(dev, "SPI create workqueue failed!\n");
		goto release_dma;
	}
	/* power and clock open */
	wmt_spi_clock_enable();

	/* registe spi master at last */
	ret = spi_register_master(master);
	if (ret) {
		dev_err(dev, "SPI register master failed\n");
		goto release_workqueue;
	}

	printk(KERN_INFO "WMT EVB SPI Controlor Driver OK!\n");
	return 0;

release_workqueue:
	wmt_spi_clock_disable();
	destroy_workqueue(wmt_spi->workqueue);
release_dma:
	wmt_spi_dma_release(wmt_spi);
release_ioremap:
	iounmap(wmt_spi->regs_base);
release_region:
	release_mem_region(res->start, res->end - res->start + 1);
release_hw_info:
	kfree(wmt_spi->spi_hw_info);
	platform_set_drvdata(pdev, NULL);
release_master:
	spi_master_put(master);
the_end:
	dev_err(dev, "WMT EVB SPI Controlor Probe Failed!\n");
	return ret;
}

static int wmt_spi_stop_queue(struct wmt_spi *spi)
{
	unsigned long flags;
	unsigned limit = 500;
	int status = 0;

	spin_lock_irqsave(&spi->spinlock, flags);

	while (!list_empty(&spi->queue) && limit--) {
		spin_unlock_irqrestore(&spi->spinlock, flags);
		msleep(10);
		spin_lock_irqsave(&spi->spinlock, flags);
	}

	if (!list_empty(&spi->queue))
		status = -EBUSY;

	spin_unlock_irqrestore(&spi->spinlock, flags);

	return status;
}

static int __devexit wmt_spi_remove(struct platform_device *pdev)
{
	struct wmt_spi *spi;
	struct wmt_spi_hw *spi_hw_info;
	struct resource *res;

	spi = (struct wmt_spi *)platform_get_drvdata(pdev);
	spi_hw_info = spi_get_hw_info(pdev);
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);

	/* stop spi message queue             */
	wmt_spi_stop_queue(spi);
	/* SSN inactive                       */
	/*
	wmt_spi_cs_inactive(spi);
	*/
	spi_disable(spi);
	/* clock and power off                */
	wmt_spi_clock_disable();
	/* work queue flush and destory       */
	flush_workqueue(spi->workqueue);
	destroy_workqueue(spi->workqueue);
	/* dma free if dma support and enable */
	wmt_spi_dma_release(spi);
	/* irq free                           */
	if (spi->irq)
		free_irq(spi->irq, spi);
	/* release requested resource         */
	iounmap(spi->regs_base);
	release_mem_region(res->start, res->end - res->start + 1);
	/* driver data entry reset            */
	platform_set_drvdata(pdev, NULL);
    /* free memory provide by spi master  */
    wmt_spi_cleanup(spi);
	/* spi master unregister and free     */
	spi_unregister_master(spi->master);
	spi_master_put(spi->master);

	return 0;
}

#ifdef CONFIG_PM
static int
wmt_spi_suspend(struct platform_device *pdev, pm_message_t state)
{
	int ret = 0;
	struct wmt_spi *spi = platform_get_drvdata(pdev);
	state = state;

	spi_dbg("Enter\n");
	ret = wmt_spi_stop_queue(spi);
	if (ret) {
	    dev_warn(&pdev->dev, "suspend wmt spi failed\n");
	    return ret;
	}
	/*
	wmt_spi_cs_inactive(spi);
	*/
	spi_disable(spi);

	spi_dbg("Exit\n");
	return 0;
}

static int wmt_spi_resume(struct platform_device *pdev)
{
	struct wmt_spi *spi = platform_get_drvdata(pdev);

	spi_dbg("Enter\n");
	/* hardware reset  */
	wmt_spi_clock_enable();
	/* start msg queue */
	queue_work(spi->workqueue, &spi->work);

	spi_dbg("Exit\n");
	return 0;
}
#else
#define wmt_spi_suspend    NULL
#define wmt_spi_resume     NULL
#endif /* CONFIG_PM */

static struct platform_driver wmt_spi_driver = {
	.driver	= {
		.name	= "wmt_spi_1",
		.owner	= THIS_MODULE,
	},
	.suspend	= wmt_spi_suspend,
	.resume		= wmt_spi_resume,
	.remove		= __devexit_p(wmt_spi_remove),
};

static int __init wmt_spi_init(void)
{
	return platform_driver_probe(&wmt_spi_driver, wmt_spi_probe);
}
module_init(wmt_spi_init);

static void __exit wmt_spi_exit(void)
{
	platform_driver_unregister(&wmt_spi_driver);
}
module_exit(wmt_spi_exit);

MODULE_AUTHOR("WonderMedia Technologies, Inc.");
MODULE_DESCRIPTION("WMT SPI Controller Driver");
MODULE_LICENSE("GPL");
