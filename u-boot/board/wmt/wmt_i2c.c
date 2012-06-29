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
/*
 * (C) Copyright 2001, 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 * This has been changed substantially by Gerald Van Baren, Custom IDEAS,
 * vanbaren@cideas.com.  It was heavily influenced by LiMon, written by
 * Neil Russell.
 */

#include <common.h>
#include <i2c.h>
#include "include/i2c.h"
#include "include/wmt_clk.h"
#include <malloc.h>

#if defined(CONFIG_HARD_I2C)

/* #define	DEBUG_I2C	*/


/*-----------------------------------------------------------------------
 * Definitions
 */

#define RETRIES		0


#define I2C_ACK		0		/* PD_SDA level to ack a byte */
#define I2C_NOACK	1		/* PD_SDA level to noack a byte */


#ifdef DEBUG_I2C
#define PRINTD(fmt, args...)	do {	\
	DECLARE_GLOBAL_DATA_PTR;	\
	if (gd->have_console)		\
		printf(fmt , ##args);	\
	} while (0)
#else
#define PRINTD(fmt, args...)
#endif

static struct i2c_s i2c ;
static int retry_time = 3;

/*-----------------------------------------------------------------------
 * Local functions
 */

static int i2c_do_xfer(struct i2c_msg_s msgs[], int num);

#if 0
static int write_byte(uchar byte);
#endif

static int i2c_read_msg(unsigned short slave_addr, unsigned char *buf,
			unsigned short length, int restart, int last);
static int i2c_write_msg(unsigned short slave_addr, unsigned char *buf,
			unsigned short length, int restart, int last);
#if 0
static uchar read_byte(int);
#endif

enum i2c_mode_s i2c_xfer_mode = I2C_STANDARD_MODE;

extern int auto_pll_divisor(enum dev_id dev, enum clk_cmd cmd, int unit, int freq);
extern int wmt_delayus(int us);

#define GPIO_CTRL_I2C (*(volatile char *)0xD8110055)
#define GPIO_PAD_EN_I2C (*(volatile char *)0xD8110495)
#define GPIO_PAD_PU_I2C (*(volatile char *)0xD81104D5)

/* [Rx00] GPIO Enable Control Register for I2C */
#define GPIO_I2C0_SDA	0x00000002
#define GPIO_I2C0_SCL	0x00000001
/* [Rx600] GPIO Pull up/down  Control Register for I2C */
#define GPIO_I2C0_SCL_PULL_EN 0x00000001
#define GPIO_I2C0_SDA_PULL_EN 0x00000002
#define GPIO_I2C0_SCL_PULL_UP 0x00000001
#define GPIO_I2C0_SDA_PULL_UP 0x00000002

static int i2c_wait_bus_not_busy(void)
{
	unsigned int timeout = 30000 ;

	while (1) {
		if ((i2c.regs->IICSR & I2C_STATUS_MASK) == I2C_READY)
			break ;
		--timeout ;
		if (timeout == 0)
			break ;
	}
	if (timeout == 0) {
		PRINTD("i2c_err : wait ready timeout error\n\r") ;
		return -1 ;
	}
	return  0 ;
}


#if 0
/*-----------------------------------------------------------------------
 * Send 8 bits and look for an acknowledgement.
 */
static int write_byte(uchar data)
{
	unsigned int length = 1;
	int ret ;
	unsigned char *buf[1];
	struct i2c_msg_s wr[1];

	*buf = 0x00;

	wr[0].addr  = data ;
	wr[0].flags = I2C_M_WR ;
	wr[0].len   = length ;
	wr[0].buf   = (unsigned char *)buf ;
	ret = i2c_transfer(wr, 1);

	if (ret != 1) {
		PRINTD("%d, %s, write fail with address=0x%X\n", __LINE__, __func__, address);
		free(buf);
		return -1;
	}

	PRINTD("i2c_write: chip %02X addr %02X alen %d buffer %p len %d\n",
		chip, addr, alen, buffer, len);
	free(buf);

	return 0 ;
}


/*-----------------------------------------------------------------------
 * if ack == I2C_ACK, ACK the byte so can continue reading, else
 * send I2C_NOACK to end the read.
 */
static uchar read_byte(int ack)
{
	return 1;
}
#endif

/*=====================================================================*/
/*                         Public Functions                            */
/*=====================================================================*/

/*-----------------------------------------------------------------------
 * Initialization
 */
void i2c_init(int speed, int slaveaddr)
{
	unsigned short tmp ;

	auto_pll_divisor(DEV_I2C0, CLK_ENABLE, 0, 0);
	auto_pll_divisor(DEV_I2C0, SET_DIV, 2, 20);
	/**/
	/* software initial*/
	i2c.regs = (struct I2C_REG *)BA_I2C0;
	i2c.irq_no      = 19;
	/*set i2c master transfer mode*/
	if (i2c_xfer_mode == I2C_STANDARD_MODE)
		i2c.i2c_mode    = I2C_STANDARD_MODE ;
	else if (i2c_xfer_mode == I2C_FAST_MODE)
		i2c.i2c_mode    = I2C_FAST_MODE ;
	else if (i2c_xfer_mode == I2C_HS_MODE)
		i2c.i2c_mode    = I2C_HS_MODE ;
	i2c.isr_nack    = 0 ;
	i2c.isr_byte_end = 0 ;
	i2c.isr_timeout = 0 ;
	/* Set I2C/GPIO pinmux to IIC funciton*/
	/* Set bit[0-3] to zero*/
	GPIO_CTRL_I2C &= ~(GPIO_I2C0_SCL | GPIO_I2C0_SDA);
	GPIO_PAD_EN_I2C = (GPIO_I2C0_SCL_PULL_EN |
			GPIO_I2C0_SDA_PULL_EN);
	GPIO_PAD_PU_I2C = (GPIO_I2C0_SCL_PULL_UP |
			GPIO_I2C0_SDA_PULL_UP);

	/* Ensure I2C clock is enabled*/
	/*set i2c master register */
	i2c.regs->IICCR = 0;
	i2c.regs->IICDIV = 12;		/* 12MHz input clk directly*/
	i2c.regs->IICISR = I2C_ISR_ALL_WRITE_CLEAR;
	i2c.regs->IICIMR = I2C_IMR_ALL_ENABLE;
	i2c.regs->IICCR = I2C_CR_ENABLE;
	tmp = i2c.regs->IICSR;		/* Read clear "received ACK bit"*/
	i2c.regs->IICISR = I2C_ISR_ALL_WRITE_CLEAR;
	if (i2c.i2c_mode == I2C_STANDARD_MODE)
		i2c.regs->IICTR = I2C_TR_STD_VALUE ;   /* 0x8064*/
	else if (i2c.i2c_mode == I2C_FAST_MODE)
		i2c.regs->IICTR = I2C_TR_FAST_VALUE ; /* 0x8019*/

	return ;
}

/*===========================================================================*/
/*  i2c_transfer*/
/**/
/*  return:*/
/*===========================================================================*/
int i2c_transfer(struct i2c_msg_s msgs[], int num)
{
	int ret ;
	int retries ;
	int i;
	int delay = 1000;

	retries = retry_time ;
	for (i = retries ;  i > 0; i--) {
		ret = i2c_do_xfer(msgs, num);
		if (ret > 0)
			return ret ;

		PRINTD("%s:i2c_test: retrying transmission\n\r", __func__);
		delay = 1000;
		while (delay == 0)
			delay--;
	}
	wmt_delayus(100);/*100us*/

	PRINTD("i2c_err : retried %i times\n\r", retries);
	return -1 ;

}

static int i2c_do_xfer(struct i2c_msg_s msgs[], int num)
{
	struct i2c_msg_s *pmsg = NULL;
	int i;
	int ret = 0 ;

	ret = i2c_wait_bus_not_busy();
	if (ret < 0)
		return ret ;

	for (i = 0; ret >= 0 && i < num; i++) {
		int last = ((i + 1) == num);
		int restart = (i != 0) ;
		pmsg = &msgs[i];

		if (pmsg->flags & I2C_M_RD) /* READ*/
			ret = i2c_read_msg(pmsg->addr, pmsg->buf, pmsg->len, restart, last);
		else /* Write*/
			ret = i2c_write_msg(pmsg->addr, pmsg->buf, pmsg->len, restart, last);
	}

	if (ret < 0)
		return ret;
	else
		return i;
}
/*-----------------------------------------------------------------------
 * Probe to see if a chip is present.  Also good for checking for the
 * completion of EEPROM writes since the chip stops responding until
 * the write completes (typically 10mSec).
 */
int i2c_probe(uchar addr)
{
	int rc;

	/* perform 1 byte read transaction */
	/*
	rc = write_byte(addr);
	*/
	rc = 1;

	return rc ? 1 : 0;
}

/*===========================================================================*/
/*  i2c_irq_handler*/
/**/
/*  return: NULL*/
/*===========================================================================*/
static void i2c_irq_handler(void)
{
	unsigned short isr_status ;

	isr_status = i2c.regs->IICISR ;

	if (isr_status & I2C_ISR_NACK_ADDR) {
		unsigned short tmp ;
		i2c.regs->IICISR = I2C_ISR_NACK_ADDR_WRITE_CLEAR ;
		tmp = i2c.regs->IICSR ;  /* read clear*/
		i2c.isr_nack = 1 ;
		return ;
	}

	if (isr_status & I2C_ISR_BYTE_END) {
		i2c.regs->IICISR = I2C_ISR_BYTE_END_WRITE_CLEAR ;
		i2c.isr_byte_end = 1 ;
		return ;
	}

	if (isr_status & I2C_ISR_SCL_TIME_OUT) {
		i2c.regs->IICISR = I2C_ISR_SCL_TIME_OUT_WRITE_CLEAR ;
		i2c.isr_timeout = 1 ;
		return ;
	} else {
		PRINTD("i2c_err : unknown I2C ISR Handle 0x%4.4X" , isr_status) ;
		return ;
	}
}

/*
 * i2c_write_msg
 * return: 0 success
 *         -1 fail
 */
static int i2c_write_msg(unsigned short slave_addr,
			unsigned char *buf,
			unsigned short length,
			int restart,
			int last)
{
	unsigned short tcr_value ;
	unsigned int xfer_length ;
	unsigned int timeout ;
	if (length == 0)
		return -1 ;
	xfer_length = 0 ; /* for array index and also for checking counting*/

	i2c.isr_nack    = 0 ;
	i2c.isr_byte_end = 0 ;
	i2c.isr_timeout = 0 ;

	i2c.regs->IICDR = (unsigned short)(buf[xfer_length] & I2C_CDR_DATA_WRITE_MASK)  ;
	if (i2c.i2c_mode == I2C_STANDARD_MODE)
		tcr_value = (unsigned short)(I2C_TCR_STANDARD_MODE|I2C_TCR_MASTER_WRITE |\
					  (slave_addr & I2C_TCR_SLAVE_ADDR_MASK)) ;
	else if (i2c.i2c_mode == I2C_FAST_MODE)
		tcr_value = (unsigned short)(I2C_TCR_FAST_MODE|I2C_TCR_MASTER_WRITE |\
					  (slave_addr & I2C_TCR_SLAVE_ADDR_MASK)) ;
	else {
		tcr_value = (unsigned short)(I2C_TCR_HS_MODE|I2C_TCR_MASTER_WRITE |\
					  (slave_addr & I2C_TCR_SLAVE_ADDR_MASK)) ;
		i2c.regs->IICDIV = HS_MASTER_CODE;
	}

	/* SET TRANSFER MODE*/
	i2c.regs->IICTCR = tcr_value ;

	/*repeat start case*/
	if (restart == 1)
		i2c.regs->IICCR |= I2C_CR_CPU_RDY ;

	while (1) {
		timeout = 500000 ;
		while (1) {
			i2c_irq_handler();
			if ((i2c.isr_nack == 1) || (i2c.isr_byte_end == 1) || (i2c.isr_timeout == 1))
				break ;
			--timeout ;
			if (timeout == 0)
				break ;
		}
		/* fail case*/
		if (timeout == 0) {
			PRINTD("[%s]i2c_err : wrire software timeout error (tx)\n\r", __func__) ;
			return -1 ;
		}
		if (i2c.isr_nack == 1) {
			PRINTD("i2c_err : write NACK error (tx) \n\r") ;
			return -1 ;
		}
		if (i2c.isr_timeout == 1) {
			PRINTD("%s:i2c_err : write SCL timeout error (tx)\n\r", __func__) ;
			return -1 ;
		}

		/* pass case*/
		if (i2c.isr_byte_end == 1)
			++xfer_length ;
		i2c.isr_nack    = 0 ;
		i2c.isr_byte_end = 0 ;
		i2c.isr_timeout = 0 ;

		if ((i2c.regs->IICSR & I2C_CSR_RCV_ACK_MASK) == I2C_CSR_RCV_NOT_ACK) {
			PRINTD("i2c_err : write RCV NACK error\n\r") ;
			return -1 ;
		}


		if (length > xfer_length) {
			i2c.regs->IICDR = (unsigned short) (buf[xfer_length] & I2C_CDR_DATA_WRITE_MASK) ;
			i2c.regs->IICCR = (I2C_CR_CPU_RDY | I2C_CR_ENABLE) ;
		} else if (length == xfer_length) { /* end tx xfer*/
			if (last == 1) {  /* stop case*/
				//i2c.regs->IICCR = (I2C_CR_TX_END|I2C_CR_CPU_RDY|I2C_CR_ENABLE) ;
				wmt_delayus(2);/*2 us*/
				i2c.regs->IICCR |= (I2C_CR_TX_END) ;
				break ;
			} else {  /* restart case*/
				/* handle the restart for first write then the next is read*/
				i2c.regs->IICCR = (I2C_CR_ENABLE) ;
				break ;
			}
		} else {
			PRINTD("i2c_err : write unknown error\n\r") ;
			return -1 ;
		}
	}
	i2c.regs->IICCR &= ~(I2C_CR_TX_END|I2C_CR_CPU_RDY) ;
	return 0 ;
}

/*
 * i2c_read_msg
 * return: 0 success
 *         -1 fail
 */
static int i2c_read_msg(unsigned short slave_addr,
			unsigned char *buf,
			unsigned short length,
			int restart,
			int last)
{
	unsigned short tcr_value ;
	unsigned int xfer_length ;
	unsigned int timeout ;

	if (length == 0)
		return -1 ;
	xfer_length = 0 ;

	i2c.isr_nack    = 0 ;
	i2c.isr_byte_end = 0 ;
	i2c.isr_timeout = 0 ;

	if (i2c.i2c_mode == I2C_STANDARD_MODE)
		tcr_value = (unsigned short)(I2C_TCR_STANDARD_MODE|I2C_TCR_MASTER_READ |\
					  (slave_addr & I2C_TCR_SLAVE_ADDR_MASK)) ;
	else if (i2c.i2c_mode == I2C_FAST_MODE)
		tcr_value = (unsigned short)(I2C_TCR_FAST_MODE|I2C_TCR_MASTER_READ |\
					  (slave_addr & I2C_TCR_SLAVE_ADDR_MASK)) ;
	else
		tcr_value = (unsigned short)(I2C_TCR_HS_MODE|I2C_TCR_MASTER_READ |\
					  (slave_addr & I2C_TCR_SLAVE_ADDR_MASK)) ;

	i2c.regs->IICTCR = tcr_value;

	/*repeat start case*/
	if (restart == 1)
		i2c.regs->IICCR |= I2C_CR_CPU_RDY ;

	if (length == 1)
		i2c.regs->IICCR |= I2C_CR_TX_NEXT_NO_ACK; /*only 8-bit to read*/

	while (1) {
		timeout = 500000 ;
		while (1) {
			i2c_irq_handler();
			if ((i2c.isr_nack == 1) || (i2c.isr_byte_end == 1) || (i2c.isr_timeout == 1))
				break ;
			--timeout ;
			if (timeout == 0)
				break ;
		}
		/* fail case*/
		if (i2c.isr_nack == 1) {
			PRINTD("i2c_err : write NACK error (rx) \n\r") ;
			return -1 ;
		}
		if (i2c.isr_timeout == 1) {
			PRINTD("%s, i2c_err : write SCL timeout error (rx)\n\r", __func__) ;
			return -1 ;
		}
		if (timeout == 0) {
			PRINTD("[%s]i2c_err: write software timeout error (rx) \n\r", __func__) ;
			return -1 ;
		}
		/* pass case*/
		if (i2c.isr_byte_end == 1) {
			buf[xfer_length] = (i2c.regs->IICDR >> 8) ;
			++xfer_length ;
		}
		i2c.isr_nack    = 0 ;
		i2c.isr_byte_end = 0 ;
		i2c.isr_timeout = 0 ;

		if (length > xfer_length) {
			if ((length - 1) ==  xfer_length) /* next read is the last one*/
				i2c.regs->IICCR |= (I2C_CR_TX_NEXT_NO_ACK | I2C_CR_CPU_RDY);
			else
				i2c.regs->IICCR |= I2C_CR_CPU_RDY ;
		} else if (length == xfer_length) { /* end rx xfer*/
			if (last == 1)  /* stop case*/
				break ;
			else  /* restart case*/
				/* ??? how to handle the restart after read ?*/
				break ;
		} else {
			PRINTD("i2c_err : read known error\n\r") ;
			return -1 ;
		}
	}
	PRINTD("i2c_test: read sequence completed\n\r");
	i2c.regs->IICCR &= ~(I2C_CR_TX_NEXT_NO_ACK | I2C_CR_CPU_RDY);
	return 0 ;
}

/*-----------------------------------------------------------------------
 * Read bytes
 */
int i2c_read(uchar chip, uint addr, int alen, uchar *buffer, int len)
{
	int ret;
	unsigned char *reg_idx;
	struct i2c_msg_s wr[1] ;
	struct i2c_msg_s rd[1] ;

	reg_idx = calloc(alen, sizeof(unsigned char *));

	wr[0].addr  = chip ;
	wr[0].flags = I2C_M_WR ;
	wr[0].len   = alen ;
	wr[0].buf   = reg_idx ;

	rd[0].addr  = chip ;
	rd[0].flags = I2C_M_RD ;
	rd[0].len   = len ;
	rd[0].buf   = buffer  ;

	if (alen > 0)
		ret = i2c_transfer(wr, 1);
	ret = i2c_transfer(rd, 1);

	PRINTD("i2c_read: chip %02X addr %02X alen %d buffer %p len %d\n",
		chip, addr, alen, buffer, len);

	if (ret != 1) {
		PRINTD("[i2c_register_read] read fail \n");
		free(reg_idx);
		return -1;
	}

	free(reg_idx);
	return 0 ;

}

/*-----------------------------------------------------------------------
 * Write bytes
 */
int i2c_write(uchar chip, uint addr, int alen, uchar *buffer, int len)
{
	unsigned int length = len + alen;
	int ret ;
	unsigned char *buf;
	struct i2c_msg_s wr[1];
	unsigned int i = 0;
	buf = calloc(length, sizeof(unsigned char *));

	for (i = 0; i < length; ++i) {
		if (i < alen)
			*(buf + alen - i - 1) = (unsigned char) ((addr >> (i * 8)) & 0xFF);
		else
			*(buf + i) = *(buffer + i - alen);
	}
	wr[0].addr  = chip ;
	wr[0].flags = I2C_M_WR ;
	wr[0].len   = length ;
	wr[0].buf   = buf ;
	ret = i2c_transfer(wr, 1);

	if (ret != 1) {
		PRINTD("%d, %s, write fail with address=0x%X\n", __LINE__, __func__, address);
		free(buf);
		return -1;
	}

	PRINTD("i2c_write: chip %02X addr %02X alen %d buffer %p len %d\n",
		chip, addr, alen, buffer, len);
	free(buf);

	return 0 ;
}

/*-----------------------------------------------------------------------
 * Read a register
 */
uchar i2c_reg_read(uchar i2c_addr, uchar reg)
{
	uchar buf;

	i2c_read(i2c_addr, reg, 1, &buf, 1);

	return buf;
}

/*-----------------------------------------------------------------------
 * Write a register
 */
void i2c_reg_write(uchar i2c_addr, uchar reg, uchar val)
{
	i2c_write(i2c_addr, reg, 1, &val, 1);
}


#endif	/* CONFIG_SOFT_I2C */
