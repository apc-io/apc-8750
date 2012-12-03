/*++
	drivers/i2c/busses/wmt_i2c_bus.c

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
/* Include your headers here*/
#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/i2c.h>
#include <linux/i2c-id.h>
#include <linux/init.h>
#include <linux/time.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/interrupt.h>

#include <mach/hardware.h>
#include <asm/irq.h>
#include <mach/irqs.h>
#include <mach/wmt-i2c-bus.h>

#ifdef __KERNEL__

#ifdef DEBUG
	#define DPRINTK		printk
#else
	#define DPRINTK(x...)
#endif

#else
	#define DPRINTK    	printf

#endif


#define MAX_BUS_READY_CNT		50	  /* jiffy*/
#define MAX_TX_TIMEOUT			500   /* ms*/
#define MAX_RX_TIMEOUT			500	  /* ms*/
#define CTRL_GPIO21 GPIO_CTRL_GP21_I2C_BYTE_ADDR  
#define PU_EN_GPIO21 GPIO_PULL_EN_GP21_I2C_BYTE_ADDR
#define PU_CTRL_GPIO21 GPIO_PULL_CTRL_GP21_I2C_BYTE_ADDR

#define USE_UBOOT_PARA

struct wmt_i2c_s {
	struct i2c_regs_s *regs;
	int irq_no ;
	enum i2c_mode_e i2c_mode ;
	int volatile isr_nack ;
	int volatile isr_byte_end ;
	int volatile isr_timeout ;
	int volatile isr_int_pending ;
};

static int i2c_wmt_wait_bus_not_busy(void);
extern int wmt_getsyspara(char *varname, unsigned char *varval, int *varlen);
static unsigned int speed_mode = 0;
static unsigned int is_master = 1;/*master:1, slave:0*/
unsigned int wmt_i2c0_is_master = 1;
unsigned int wmt_i2c0_speed_mode = 0;
EXPORT_SYMBOL(wmt_i2c0_is_master);

/**/
/*  variable*/
/*-------------------------------------------------*/
static volatile struct wmt_i2c_s i2c ;

DECLARE_WAIT_QUEUE_HEAD(i2c1_wait);
spinlock_t i2c1_wmt_irqlock = SPIN_LOCK_UNLOCKED;
static struct list_head wmt_i2c_fifohead;
static spinlock_t i2c_fifolock = SPIN_LOCK_UNLOCKED;
static int i2c_wmt_read_buf(
	unsigned int slave_addr,
	char *buf,
	unsigned int length,
	int restart,
	int last
);
static int i2c_wmt_write_buf(
	unsigned int slave_addr,
	char *buf,
	unsigned int length,
	int restart,
	int last
);

static void i2c_wmt_set_mode(enum i2c_mode_e mode  /*!<; //[IN] mode */)
{
	if (is_master == 0)
		return;
	i2c.i2c_mode = mode ;

	if (i2c.i2c_mode == I2C_STANDARD_MODE)  {
		DPRINTK("I2C: set standard mode \n");
		i2c.regs->tr_reg = I2C_TR_STD_VALUE ;   /* 0x8041*/
	} else if (i2c.i2c_mode == I2C_FAST_MODE) {
		DPRINTK("I2C: set fast mode \n");
		i2c.regs->tr_reg = I2C_TR_FAST_VALUE ; /* 0x8011*/
	}
}


static int i2c_send_request(
	struct i2c_msg *msg,
	int msg_num,
	int non_block,
	void (*callback)(void *data),
	void *data
)
{
	struct wmt_i2cbusfifo *i2c_fifo_head;
	struct i2c_msg *pmsg = NULL;
	int ret = 0;
	int restart = 0;
	int last = 0;
	unsigned long flags;
	int slave_addr = msg[0].addr;

	if (slave_addr == WMT_I2C_API_I2C_ADDR)
		return ret ;

	i2c.isr_nack    	= 0 ;
	i2c.isr_byte_end	= 0 ;
	i2c.isr_timeout 	= 0 ;
	i2c.isr_int_pending = 0;

	i2c_fifo_head = kzalloc(sizeof(struct wmt_i2cbusfifo), GFP_ATOMIC);
	INIT_LIST_HEAD(&i2c_fifo_head->busfifohead);

	pmsg = &msg[0];
	i2c_fifo_head->msg = pmsg;
	i2c_fifo_head->msg_num = msg_num;
	
	spin_lock_irqsave(&i2c_fifolock, flags);
	if (list_empty(&wmt_i2c_fifohead)) {
		i2c_wmt_wait_bus_not_busy();
		pmsg = &msg[0];
		i2c_fifo_head->xfer_length = 1;
		i2c_fifo_head->xfer_msgnum = 0;
		i2c_fifo_head->restart = 0;
		i2c_fifo_head->non_block = non_block;

		if (non_block == 1) {
			i2c_fifo_head->callback = callback;
			i2c_fifo_head->data = data;
		} else {
			i2c_fifo_head->callback = 0;
			i2c_fifo_head->data = 0;
		}
			
		list_add_tail(&i2c_fifo_head->busfifohead, &wmt_i2c_fifohead);
		if (pmsg->flags & I2C_M_RD) {
			i2c_fifo_head->xfer_length = 1;
			ret = i2c_wmt_read_buf(pmsg->addr, pmsg->buf, pmsg->len, restart, last);
		} else {
			i2c_fifo_head->xfer_length = 1;
			if (pmsg->flags & I2C_M_NOSTART)
				i2c_fifo_head->restart = 1;
			else
				i2c_fifo_head->restart = 0;
			ret = i2c_wmt_write_buf(pmsg->addr, pmsg->buf, pmsg->len, restart, last);
		}
			
	} else {
		i2c_fifo_head->xfer_length = 0;
		i2c_fifo_head->xfer_msgnum = 0;
		i2c_fifo_head->restart = 0;
		i2c_fifo_head->non_block = non_block;
		if (non_block == 1) {
			i2c_fifo_head->callback = callback;
			i2c_fifo_head->data = data;
		} else {
			i2c_fifo_head->callback = 0;
			i2c_fifo_head->data = 0;
		}
		list_add_tail(&i2c_fifo_head->busfifohead, &wmt_i2c_fifohead);
	}
	spin_unlock_irqrestore(&i2c_fifolock, flags);
	if (non_block == 0) {
		wait_event(i2c1_wait, i2c.isr_int_pending);
		ret = msg_num;
		if (i2c.isr_nack == 1) {
			DPRINTK("i2c_err : write NACK error (rx) \n\r") ;
			ret = -EIO ;
		}
		if (i2c.isr_timeout == 1) {
			DPRINTK("i2c_err : write SCL timeout error (rx)\n\r") ;
			ret = -ETIMEDOUT ;
		}

	}

	return ret;
		

}
static int i2c_wmt_read_buf(
	unsigned int slave_addr,
	char *buf,
	unsigned int length,
	int restart,
	int last
)
{
	unsigned short tcr_value;
	int ret = 0;

	DPRINTK("[%s]:length = %d , slave_addr = %x\n", __func__, length , slave_addr);

	if (length <=0)
		return -1;
	i2c.isr_nack    	= 0 ;
	i2c.isr_byte_end	= 0 ;
	i2c.isr_timeout 	= 0 ;
	/*i2c.isr_int_pending = 0;*/

	i2c.regs->cr_reg &= ~(I2C_CR_TX_END); /*clear Tx end*/
	i2c.regs->cr_reg &= ~(I2C_CR_TX_NEXT_NO_ACK); /*clear NEXT_NO_ACK*/
	if (length <=0)
		return -1;
	i2c.isr_nack    	= 0 ;
	i2c.isr_byte_end	= 0 ;
	i2c.isr_timeout 	= 0 ;
	/*i2c.isr_int_pending = 0;*/

	tcr_value = 0 ;

	if (i2c.i2c_mode == I2C_STANDARD_MODE) 
		tcr_value = (unsigned short)(I2C_TCR_STANDARD_MODE|I2C_TCR_MASTER_READ |\
				(slave_addr & I2C_TCR_SLAVE_ADDR_MASK)) ;
	else if (i2c.i2c_mode == I2C_FAST_MODE)
		tcr_value = (unsigned short)(I2C_TCR_FAST_MODE|I2C_TCR_MASTER_READ |\
				(slave_addr & I2C_TCR_SLAVE_ADDR_MASK)) ;

	if (length == 1)
		i2c.regs->cr_reg |= I2C_CR_TX_NEXT_NO_ACK; /*only 8-bit to read*/

	i2c.regs->tcr_reg = tcr_value ;
	return ret;
}

static int i2c_wmt_write_buf(
	unsigned int slave_addr,
	char *buf,
	unsigned int length,
	int restart,
	int last
)
{
	unsigned short tcr_value ;
	unsigned int xfer_length ;
	int ret = 0 ;

	DPRINTK("[%s]length = %d , slave_addr = %x\n", __func__, length , slave_addr);
	if (slave_addr == WMT_I2C_API_I2C_ADDR)
		return ret ;

	if (is_master == 0)
		return -ENXIO;

	/* special case allow length:0, for i2c_smbus_xfer*/
	/**/
	if (length < 0)
		return -1 ;
	xfer_length = 0 ; /* for array index and also for checking counting*/

	i2c.isr_nack            = 0 ;
	i2c.isr_byte_end        = 0 ;
	i2c.isr_timeout         = 0 ;
	/*i2c.isr_int_pending = 0;*/

	i2c.regs->cr_reg &= ~(I2C_CR_TX_END); /*clear Tx end*/
	i2c.regs->cr_reg &= ~(I2C_CR_TX_NEXT_NO_ACK); /*clear NEXT_NO_ACK*/

	if (length == 0)
		i2c.regs->cdr_reg = 0 ;
	else
		i2c.regs->cdr_reg = (unsigned short)(buf[xfer_length] & I2C_CDR_DATA_WRITE_MASK)  ;

	tcr_value = 0 ;
	if (i2c.i2c_mode == I2C_STANDARD_MODE)
		tcr_value = (unsigned short)(I2C_TCR_STANDARD_MODE|I2C_TCR_MASTER_WRITE |\
				(slave_addr & I2C_TCR_SLAVE_ADDR_MASK)) ;
	else if (i2c.i2c_mode == I2C_FAST_MODE)
		tcr_value = (unsigned short)(I2C_TCR_FAST_MODE|I2C_TCR_MASTER_WRITE |\
				(slave_addr & I2C_TCR_SLAVE_ADDR_MASK)) ;

	i2c.regs->tcr_reg = tcr_value ;

	ret = 0 ;
	return ret;

}
static int i2c_wmt_read_msg(
	unsigned int slave_addr, 	/*!<; //[IN] Salve address */
	char *buf, 				/*!<; //[OUT] Pointer to data */
	unsigned int length,		/*!<; //Data length */
	int restart, 				/*!<; //Need to restart after a complete read */
	int last 					/*!<; //Last read */
)
{
	unsigned short tcr_value ;
	unsigned int xfer_length ;
	int is_timeout ;
	int ret  = 0 ;
	int wait_event_result  = 0 ;

	if (is_master == 0)
		return -ENXIO;
	if (length <= 0)
		return -1 ;
	xfer_length = 0 ;

	if (restart == 0)
		ret = i2c_wmt_wait_bus_not_busy()  ;
	if (ret < 0)
		return ret ;

	i2c.isr_nack    	= 0 ;
	i2c.isr_byte_end	= 0 ;
	i2c.isr_timeout 	= 0 ;
	i2c.isr_int_pending = 0;

	i2c.regs->cr_reg &= ~(I2C_CR_TX_END); /*clear Tx end*/
	i2c.regs->cr_reg &= ~(I2C_CR_TX_NEXT_NO_ACK); /*clear NEXT_NO_ACK*/
	if (restart == 0)
		i2c.regs->cr_reg |= (I2C_CR_CPU_RDY); /*release SCL*/

	tcr_value = 0 ;
	if (i2c.i2c_mode == I2C_STANDARD_MODE)  {
		tcr_value = (unsigned short)(I2C_TCR_STANDARD_MODE|I2C_TCR_MASTER_READ |\
				(slave_addr & I2C_TCR_SLAVE_ADDR_MASK)) ;
	} else if (i2c.i2c_mode == I2C_FAST_MODE) {
		tcr_value = (unsigned short)(I2C_TCR_FAST_MODE|I2C_TCR_MASTER_READ |\
				(slave_addr & I2C_TCR_SLAVE_ADDR_MASK)) ;
	}
	if (length == 1)
		i2c.regs->cr_reg |= I2C_CR_TX_NEXT_NO_ACK; /*only 8-bit to read*/

	i2c.regs->tcr_reg = tcr_value ;

	/*repeat start case*/
	if (restart == 1)
		i2c.regs->cr_reg |= (I2C_CR_CPU_RDY); /*release SCL*/

	ret  = 0 ;
	for (; ;) {
		is_timeout = 0 ;
		wait_event_result = wait_event_interruptible_timeout(i2c1_wait, i2c.isr_int_pending ,
			(MAX_RX_TIMEOUT * HZ / 1000)) ;
		if (likely(wait_event_result > 0)) {
			DPRINTK("I2C: wait interrupted (rx) \n");
			ret  = 0 ;
		} else if (likely(i2c.isr_int_pending == 0)) {
			DPRINTK("I2C: wait timeout (rx) \n");
			is_timeout = 1 ;
			ret = -ETIMEDOUT ;
		}

		/**/
		/* fail case*/
		/**/
		if (i2c.isr_nack == 1) {
			DPRINTK("i2c_err : write NACK error (rx) \n\r") ;
			ret = -EIO ;
			break ;
		}
		if (i2c.isr_timeout == 1) {
			DPRINTK("i2c_err : write SCL timeout error (rx)\n\r") ;
			msleep(10);
			ret = -ETIMEDOUT ;
			break ;
		}
		if (is_timeout == 1) {
			DPRINTK("i2c_err: write software timeout error (rx) \n\r") ;
			ret = -ETIMEDOUT ;
			break ;
		}


		/**/
		/* pass case*/
		/**/
		if (i2c.isr_byte_end == 1) {
			buf[xfer_length] = (i2c.regs->cdr_reg >> 8) ;
			++xfer_length ;
			DPRINTK("i2c_test: received BYTE_END\n\r");
		}
		i2c.isr_int_pending = 0;
		i2c.isr_nack    	= 0 ;
		i2c.isr_byte_end	= 0 ;
		i2c.isr_timeout 	= 0 ;

		if (length > xfer_length) {
			if ((length - 1) ==  xfer_length) { /* next read is the last one*/
				i2c.regs->cr_reg |= (I2C_CR_TX_NEXT_NO_ACK | I2C_CR_CPU_RDY);
				DPRINTK("i2c_test: set CPU_RDY & TX_ACK. next data is last.\r\n");
			} else {
				i2c.regs->cr_reg |= I2C_CR_CPU_RDY ;
				DPRINTK("i2c_test: more data to read. only set CPU_RDY. \r\n");
			}
		} else if (length == xfer_length) { /* end rx xfer*/
			if (last == 1) {  /* stop case*/
				DPRINTK("i2c_test: read completed \r\n");
				break ;
			} else {  /* restart case*/
				/* ??? how to handle the restart after read ?*/
				DPRINTK("i2c_test: RX ReStart Case \r\n") ;
				break ;
			}
		} else {
			DPRINTK("i2c_err : read known error\n\r") ;
			ret = -EIO ;
			break ;
		}
	}

	DPRINTK("i2c_test: read sequence completed\n\r");
	return ret ;
}

static int i2c_wmt_write_msg(
	unsigned int slave_addr, 	/*!<; //[IN] Salve address */
	char *buf, 				/*!<; //[OUT] Pointer to data */
	unsigned int length,		/*!<; //Data length */
	int restart, 				/*!<; //Need to restart after a complete write */
	int last 					/*!<; //Last read */
)
{
	unsigned short tcr_value ;
	unsigned int xfer_length ;
	int is_timeout ;
	int ret = 0 ;
	int wait_event_result ;

	DPRINTK("length = %d , slave_addr = %x\n", length , slave_addr);
	if (slave_addr == WMT_I2C_API_I2C_ADDR)
		return ret ;

	if (is_master == 0)
		return -ENXIO;

	/* special case allow length:0, for i2c_smbus_xfer*/
	/**/
	if (length < 0)
		return -1 ;
	xfer_length = 0 ; /* for array index and also for checking counting*/
	if (restart == 0)
		ret = i2c_wmt_wait_bus_not_busy()  ;
	if (ret < 0)
		return ret ;

	i2c.isr_nack    	= 0 ;
	i2c.isr_byte_end	= 0 ;
	i2c.isr_timeout 	= 0 ;
	i2c.isr_int_pending = 0;

	/**/
	/* special case allow length:0, for i2c_smbus_xfer*/
	/**/
	if (length == 0)
		i2c.regs->cdr_reg = 0 ;
	else
		i2c.regs->cdr_reg = (unsigned short)(buf[xfer_length] & I2C_CDR_DATA_WRITE_MASK)  ;

	if (restart == 0) {
		i2c.regs->cr_reg &= ~(I2C_CR_TX_END); /*clear Tx end*/
		i2c.regs->cr_reg |= (I2C_CR_CPU_RDY); /*release SCL*/
	}

	/**/
	/* I2C: Set transfer mode [standard/fast]*/
	/**/
	tcr_value = 0 ;
	if (i2c.i2c_mode == I2C_STANDARD_MODE)
		tcr_value = (unsigned short)(I2C_TCR_STANDARD_MODE|I2C_TCR_MASTER_WRITE |\
				(slave_addr & I2C_TCR_SLAVE_ADDR_MASK)) ;
	else if (i2c.i2c_mode == I2C_FAST_MODE)
		tcr_value = (unsigned short)(I2C_TCR_FAST_MODE|I2C_TCR_MASTER_WRITE |\
				(slave_addr & I2C_TCR_SLAVE_ADDR_MASK)) ;

	i2c.regs->tcr_reg = tcr_value ;

	if (restart == 1)
		i2c.regs->cr_reg |= I2C_CR_CPU_RDY ;

	ret = 0 ;
	for (; ;) {

		is_timeout = 0 ;
		/**/
		/* I2C: Wait for interrupt. if ( i2c.isr_int_pending == 1 ) ==> an interrupt exsits.*/
		/**/
		wait_event_result = wait_event_interruptible_timeout(i2c1_wait, i2c.isr_int_pending , (MAX_TX_TIMEOUT * HZ / 1000)) ;

		if (likely(wait_event_result > 0)) {
			DPRINTK("I2C: wait interrupted (tx)\n");
			ret  = 0 ;
		} else if (likely(i2c.isr_int_pending == 0)) {
			DPRINTK("I2C: wait timeout (tx) \n");
			is_timeout = 1 ;
			ret = -ETIMEDOUT ;
		}

		/**/
		/* fail case*/
		/**/
		if (i2c.isr_nack == 1) {
			DPRINTK("i2c_err : write NACK error (tx) \n\r") ;
			ret = -EIO ;
			break ;
		}
		if (i2c.isr_timeout == 1) {
			DPRINTK("i2c_err : write SCL timeout error (tx)\n\r") ;
			msleep(10);
			ret = -ETIMEDOUT ;
			break ;
		}
		if (is_timeout == 1) {
			DPRINTK("i2c_err : write software timeout error (tx)\n\r") ;
			ret = -ETIMEDOUT ;
			break ;
		}

		/**/
		/* pass case*/
		/**/
		if (i2c.isr_byte_end == 1) {
			DPRINTK("i2c: isr end byte (tx)\n\r") ;
			++xfer_length ;
		}
		i2c.isr_int_pending = 0 ;
		i2c.isr_nack    	= 0 ;
		i2c.isr_byte_end	= 0 ;
		i2c.isr_timeout 	= 0 ;


		if ((i2c.regs->csr_reg & I2C_CSR_RCV_ACK_MASK) == I2C_CSR_RCV_NOT_ACK) {
			DPRINTK("i2c_err : write RCV NACK error\n\r") ;
			ret = -EIO ;
			break ;
		}

		/**/
		/* special case allow length:0, for i2c_smbus_xfer*/
		/**/
		if (length == 0) {
			i2c.regs->cr_reg = (I2C_CR_TX_END|I2C_CR_CPU_RDY|I2C_CR_ENABLE) ;
			break ;
		}
		if (length > xfer_length) {
			i2c.regs->cdr_reg = (unsigned short) (buf[xfer_length] & I2C_CDR_DATA_WRITE_MASK) ;
			i2c.regs->cr_reg = (I2C_CR_CPU_RDY | I2C_CR_ENABLE) ;
			DPRINTK("i2c_test: write register data \n\r") ;
		} else if (length == xfer_length) { /* end tx xfer*/
			if (last == 1) {  /* stop case*/
				i2c.regs->cr_reg = (I2C_CR_TX_END|I2C_CR_CPU_RDY|I2C_CR_ENABLE) ;
				DPRINTK("i2c_test: finish write \n\r") ;
				break ;
			} else {  /* restart case*/
				/* handle the restart for first write then the next is read*/
				i2c.regs->cr_reg = (I2C_CR_ENABLE) ;
				DPRINTK("i2c_test: tx restart Case \n\r") ;
				break ;
			}
		} else {
			DPRINTK("i2c_err : write unknown error\n\r") ;
			ret = -EIO ;
			break ;
		}
	} ;

	DPRINTK("i2c_test: write sequence completed\n\r");

	return ret ;
}

static int i2c_wmt_wait_bus_not_busy(void)
{
	int ret ;
	int cnt ;

	ret = 0 ;
	cnt = 0 ;
	while (1) {
		if ((REG16_VAL(I2C_CSR_ADDR) & I2C_STATUS_MASK) == I2C_READY) {
			ret = 0;
			break ;
		}
		cnt++ ;

		if (cnt > MAX_BUS_READY_CNT) {
			ret = (-EBUSY) ;
			printk("i2c_err 0: wait but not ready time-out\n\r") ;
			cnt = 0;
		}
	}
	return ret ;
}

static void i2c_wmt_reset(void)
{
	unsigned short tmp ;
	if (is_master == 0)
		return;

	/**/
	/* software initial*/
	/**/
	i2c.regs        = (struct i2c_regs_s *)I2C0_BASE_ADDR ;
	i2c.irq_no      = IRQ_I2C0 ;
	if (speed_mode == 0)
		i2c.i2c_mode    = I2C_STANDARD_MODE ;
	else
		i2c.i2c_mode    = I2C_FAST_MODE ;
	i2c.isr_nack    = 0 ;
	i2c.isr_byte_end = 0 ;
	i2c.isr_timeout = 0 ;
	i2c.isr_int_pending = 0;

	/**/
	/* hardware initial*/
	/**/

	i2c.regs->cr_reg  = 0 ;
	i2c.regs->div_reg = APB_96M_I2C_DIV ;
	i2c.regs->isr_reg = I2C_ISR_ALL_WRITE_CLEAR ;   /* 0x0007*/
	i2c.regs->imr_reg = I2C_IMR_ALL_ENABLE ;        /* 0x0007*/

	i2c.regs->cr_reg  = I2C_CR_ENABLE ;
	tmp = i2c.regs->csr_reg ;                     /* read clear*/
	i2c.regs->isr_reg = I2C_ISR_ALL_WRITE_CLEAR ; /* 0x0007*/

	if (i2c.i2c_mode == I2C_STANDARD_MODE)
		i2c.regs->tr_reg = I2C_TR_STD_VALUE ;   /* 0x8041*/
	else if (i2c.i2c_mode == I2C_FAST_MODE)
		i2c.regs->tr_reg = I2C_TR_FAST_VALUE ; /* 0x8011*/

	DPRINTK("Resetting I2C Controller Unit\n");

	return ;
}
static int wmt_i2c_transfer_msg(struct wmt_i2cbusfifo *fifo_head)
{
	int xfer_length = fifo_head->xfer_length;
	int xfer_msgnum = fifo_head->xfer_msgnum;
	struct i2c_msg *pmsg = &fifo_head->msg[xfer_msgnum];
	int restart = fifo_head->restart;
	unsigned short tcr_value;
	unsigned short slave_addr = pmsg->addr;
	int length = pmsg->len;
	int ret = 0;

	if (pmsg->flags &  I2C_M_RD) {
		if (restart == 0)
			i2c_wmt_wait_bus_not_busy();
		i2c.isr_nack    	= 0 ;
		i2c.isr_byte_end	= 0 ;
		i2c.isr_timeout 	= 0 ;
		/*i2c.isr_int_pending = 0;*/

		i2c.regs->cr_reg &= ~(I2C_CR_TX_END); /*clear Tx end*/
		i2c.regs->cr_reg &= ~(I2C_CR_TX_NEXT_NO_ACK); /*clear NEXT_NO_ACK*/
		if (restart == 0)
			i2c.regs->cr_reg |= (I2C_CR_CPU_RDY); /*release SCL*/

		tcr_value = 0 ;
		if (i2c.i2c_mode == I2C_STANDARD_MODE)  {
			tcr_value = (unsigned short)(I2C_TCR_STANDARD_MODE|I2C_TCR_MASTER_READ |\
					(slave_addr & I2C_TCR_SLAVE_ADDR_MASK)) ;
		} else if (i2c.i2c_mode == I2C_FAST_MODE) {
			tcr_value = (unsigned short)(I2C_TCR_FAST_MODE|I2C_TCR_MASTER_READ |\
					(slave_addr & I2C_TCR_SLAVE_ADDR_MASK)) ;

		}
		if (length == 1)
			i2c.regs->cr_reg |= I2C_CR_TX_NEXT_NO_ACK; /*only 8-bit to read*/

		i2c.regs->tcr_reg = tcr_value ;

		/*repeat start case*/
		if (restart == 1)
			i2c.regs->cr_reg |= (I2C_CR_CPU_RDY); /*release SCL*/
	} else {
		if (restart == 0)
			i2c_wmt_wait_bus_not_busy();
		i2c.isr_nack    	= 0 ;
		i2c.isr_byte_end	= 0 ;
		i2c.isr_timeout 	= 0 ;
		/*i2c.isr_int_pending = 0;*/

		/**/
		/* special case allow length:0, for i2c_smbus_xfer*/
		/**/
		if (length == 0)
			i2c.regs->cdr_reg = 0 ;
		else
			i2c.regs->cdr_reg = (unsigned short)(pmsg->buf[xfer_length] & I2C_CDR_DATA_WRITE_MASK)  ;

		if (restart == 0) {
			i2c.regs->cr_reg &= ~(I2C_CR_TX_END); /*clear Tx end*/
			i2c.regs->cr_reg |= (I2C_CR_CPU_RDY); /*release SCL*/
		}

		/**/
		/* I2C: Set transfer mode [standard/fast]*/
		/**/
		tcr_value = 0 ;
		if (i2c.i2c_mode == I2C_STANDARD_MODE)
			tcr_value = (unsigned short)(I2C_TCR_STANDARD_MODE|I2C_TCR_MASTER_WRITE |\
					(slave_addr & I2C_TCR_SLAVE_ADDR_MASK)) ;
		else if (i2c.i2c_mode == I2C_FAST_MODE)
			tcr_value = (unsigned short)(I2C_TCR_FAST_MODE|I2C_TCR_MASTER_WRITE |\
					(slave_addr & I2C_TCR_SLAVE_ADDR_MASK)) ;

		i2c.regs->tcr_reg = tcr_value ;

		if (restart == 1)
			i2c.regs->cr_reg |= I2C_CR_CPU_RDY ;
	}
	return ret;
}

static irqreturn_t i2c_wmt_handler(
	int this_irq, 			/*!<; //[IN] IRQ number */
	void *dev_id 		/*!<; //[IN] Pointer to device ID */
)
{
	int wakeup ;
	unsigned short isr_status ;
	unsigned short tmp ;
	unsigned long flags;
	struct wmt_i2cbusfifo *fifo_head;
	int xfer_length = 0;
	int xfer_msgnum = 0;
	struct i2c_msg *pmsg;

	spin_lock_irqsave(&i2c1_wmt_irqlock, flags);
	isr_status = i2c.regs->isr_reg ;
	wakeup = 0 ;
	fifo_head = list_first_entry(&wmt_i2c_fifohead, struct wmt_i2cbusfifo, busfifohead);

	if (isr_status & I2C_ISR_NACK_ADDR) {
		DPRINTK("[%s]:i2c NACK\n", __func__);
		/*spin_lock(&i2c_fifolock);*/
		list_del(&fifo_head->busfifohead);/*del request*/
		/*spin_unlock(&i2c_fifolock);*/
		xfer_length = 0;
		i2c.regs->isr_reg = I2C_ISR_NACK_ADDR_WRITE_CLEAR ;
		tmp = i2c.regs->csr_reg ;  /* read clear*/
		i2c.isr_nack = 1 ;
		wakeup = 1 ;
	}

	if (isr_status & I2C_ISR_BYTE_END) {
		i2c.regs->isr_reg = I2C_ISR_BYTE_END_WRITE_CLEAR ;
		i2c.isr_byte_end = 1 ;
		xfer_length = fifo_head->xfer_length;
		xfer_msgnum = fifo_head->xfer_msgnum;
		pmsg = &fifo_head->msg[xfer_msgnum];
		
		/*read case*/
		if (pmsg->flags &  I2C_M_RD) {
			pmsg->buf[xfer_length - 1] = (i2c.regs->cdr_reg >> 8) ;
			/*the last data in current msg?*/
			if (xfer_length == pmsg->len - 1) {
				/*last msg of the current request?*/
				/*spin_lock(&i2c_fifolock);*/
				if (pmsg->flags & I2C_M_NOSTART) {
					++fifo_head->xfer_length;
					fifo_head->restart = 1;
					/*
					++fifo_head->xfer_msgnum;
					*/
					i2c.regs->cr_reg |= I2C_CR_CPU_RDY;
				} else {
					++fifo_head->xfer_length;
					fifo_head->restart = 0;
					/*
					++fifo_head->xfer_msgnum;
					*/
					i2c.regs->cr_reg |= (I2C_CR_CPU_RDY | I2C_CR_TX_NEXT_NO_ACK);
				}
				/*spin_unlock(&i2c_fifolock);*/
			} else if (xfer_length == pmsg->len) {/*next msg*/
				if (xfer_msgnum < fifo_head->msg_num - 1) {
					/*spin_lock(&i2c_fifolock);*/
					fifo_head->xfer_length = 0;
					++fifo_head->xfer_msgnum;

					wmt_i2c_transfer_msg(fifo_head);
					++fifo_head->xfer_length;
					/*spin_unlock(&i2c_fifolock);*/
				} else { /*data of this msg has been transfered*/
					/*spin_lock(&i2c_fifolock);*/
					list_del(&fifo_head->busfifohead);/*del request*/
					/*next request exist?*/
					if (list_empty(&wmt_i2c_fifohead)) {/*no more reqeust*/
						/*kfree(fifo_head);*/
						if (fifo_head->non_block == 0) {
							wakeup = 1;
						} else {
							fifo_head->callback(fifo_head->data);
						}
						kfree(fifo_head);
					} else { /*more request*/
						if (fifo_head->non_block == 0) {
							wakeup = 1;
						} else {
							fifo_head->callback(fifo_head->data);
						}
						kfree(fifo_head);
						fifo_head = list_first_entry(&wmt_i2c_fifohead,
								struct wmt_i2cbusfifo, busfifohead);
						/*
						if (fifo_head->non_block == 0)
							wakeup = 1;
						*/

						fifo_head->xfer_length = 0;
						wmt_i2c_transfer_msg(fifo_head);
						++fifo_head->xfer_length;

						/*
						if (fifo_head->non_block == 0) {
							printk("2 : non callback\n");
							wakeup = 1;
						} else {
							printk("2 :callback\n");
							fifo_head->callback(fifo_head->data);
						}
						*/
					}
					/*spin_unlock(&i2c_fifolock);*/
				}
			} else {/*next data*/
				/*spin_lock(&i2c_fifolock);*/
				++fifo_head->xfer_length;
				/*spin_unlock(&i2c_fifolock);*/
				i2c.regs->cr_reg |= I2C_CR_CPU_RDY;
			}
				
		} else { /*write case*/
			/*the last data in current msg?*/
			if (xfer_length == pmsg->len) {
				/*last msg of the current request?*/
				if (xfer_msgnum < fifo_head->msg_num - 1) {
					/*spin_lock(&i2c_fifolock);*/
					if (pmsg->flags & I2C_M_NOSTART) {
						++fifo_head->xfer_length;
						fifo_head->restart = 1;
					} else {
						++fifo_head->xfer_length;
						fifo_head->restart = 0;
						i2c.regs->cr_reg &= ~(I2C_CR_TX_END);
						udelay(2);
						i2c.regs->cr_reg |= (I2C_CR_TX_END);
					}
					/*access next msg*/
					fifo_head->xfer_length = 0;
					++fifo_head->xfer_msgnum;

					wmt_i2c_transfer_msg(fifo_head);
					++fifo_head->xfer_length;
					/*spin_unlock(&i2c_fifolock);*/
				} else {/*this request finish*/
					/*spin_lock(&i2c_fifolock);*/
					/*next request exist?*/
					list_del(&fifo_head->busfifohead);/*del request*/
					if (list_empty(&wmt_i2c_fifohead)) {
						/*kfree(fifo_head);*/
						/*
						if (fifo_head->non_block == 0)
							wakeup = 1;
						*/
						i2c.regs->cr_reg &= ~(I2C_CR_TX_END);
						udelay(2);
						i2c.regs->cr_reg |= (I2C_CR_TX_END);
						if (fifo_head->non_block == 0) {
							wakeup = 1;
						} else {
							fifo_head->callback(fifo_head->data);
						}
						kfree(fifo_head);
							
					} else {
						i2c.regs->cr_reg &= ~(I2C_CR_TX_END);
						udelay(2);
						i2c.regs->cr_reg |= (I2C_CR_TX_END);
						if (fifo_head->non_block == 0) {
							wakeup = 1;
						} else {
							fifo_head->callback(fifo_head->data);
						}
						kfree(fifo_head);
						fifo_head = list_first_entry(&wmt_i2c_fifohead,
								struct wmt_i2cbusfifo, busfifohead);
						/*
						if (fifo_head->non_block == 0)
							wakeup = 1;
						*/

						/*next msg*/
						fifo_head->xfer_length = 0;
						++fifo_head->xfer_msgnum;
						wmt_i2c_transfer_msg(fifo_head);
						++fifo_head->xfer_length;
						/*
						if (fifo_head->non_block == 0) {
							printk("4:non callback\n");
							wakeup = 1;
						} else {
							printk("4:callback\n");
							fifo_head->callback(fifo_head->data);
						}
						*/
					}
					/*spin_unlock(&i2c_fifolock);*/
				}
			} else {/*next data*/
				i2c.regs->cdr_reg = (unsigned short) (pmsg->buf[fifo_head->xfer_length] & I2C_CDR_DATA_WRITE_MASK);
				/*spin_lock(&i2c_fifolock);*/
				++fifo_head->xfer_length;
				/*spin_unlock(&i2c_fifolock);*/
				i2c.regs->cr_reg |= (I2C_CR_CPU_RDY | I2C_CR_ENABLE);
			}
		}
	}

	if (isr_status & I2C_ISR_SCL_TIME_OUT) {
		DPRINTK("[%s]SCL timeout\n", __func__);
#if 0
		i2c.regs->cr_reg |= BIT7;/*reset status*/
		/*spin_lock(&i2c_fifolock);*/
		list_del(&fifo_head->busfifohead);/*del request*/
		/*spin_unlock(&i2c_fifolock);*/
		xfer_length = 0;
		i2c.regs->isr_reg = I2C_ISR_SCL_TIME_OUT_WRITE_CLEAR | I2C_ISR_BYTE_END_WRITE_CLEAR;
		i2c.isr_timeout = 1 ;
		wakeup = 1;
#endif
		i2c.regs->isr_reg = I2C_ISR_SCL_TIME_OUT_WRITE_CLEAR ;
	}


	if (wakeup) {
		/*spin_lock_irqsave(&i2c_wmt_irqlock, flags);*/
		i2c.isr_int_pending = 1;
		/*spin_unlock_irqrestore(&i2c_wmt_irqlock, flags);*/
		wake_up(&i2c1_wait);
	} else
		DPRINTK("i2c_err : unknown I2C ISR Handle 0x%4.4X" , isr_status) ;
	spin_unlock_irqrestore(&i2c1_wmt_irqlock, flags);
	return IRQ_HANDLED;
}

static int i2c_wmt_resource_init(void)
{
	if (is_master == 0)
		return 0;
	if (request_irq(i2c.irq_no , &i2c_wmt_handler, IRQF_DISABLED, "i2c", 0) < 0) {
		DPRINTK(KERN_INFO "I2C: Failed to register I2C irq %i\n", i2c.irq_no);
		return -ENODEV;
	}
	return 0;
}

static void i2c_wmt_resource_release(void)
{
	if (is_master == 0)
		return;
	free_irq(i2c.irq_no, 0);
}

static struct i2c_algo_wmt_data i2c_wmt_data = {
	write_msg:          i2c_wmt_write_msg,
	read_msg:           i2c_wmt_read_msg,
	send_request:       i2c_send_request,
	wait_bus_not_busy:  i2c_wmt_wait_bus_not_busy,
	reset:              i2c_wmt_reset,
	set_mode:		i2c_wmt_set_mode,
	udelay:             I2C_ALGO_UDELAY,
	timeout:            I2C_ALGO_TIMEOUT,
};

static struct i2c_adapter i2c_wmt_ops = {
	.owner      		= THIS_MODULE,
	.id         		= I2C_ALGO_WMT,
	.algo_data  		= &i2c_wmt_data,
	.name       		= "wmt_i2c_adapter",
	.retries           	= I2C_ADAPTER_RETRIES,
	.nr                     = 0,
};

extern int wmt_i2c_add_bus(struct i2c_adapter *);
extern int wmt_i2c_del_bus(struct i2c_adapter *);

static int __init i2c_adap_wmt_init(void)
{
	unsigned short tmp ;
	char varname[] = "wmt.i2c.param";
#ifdef CONFIG_I2C_SLAVE_WMT
	char varname1[] = "wmt.bus.i2c.slave_port";
#endif
	unsigned char buf[80];
	int ret;
	unsigned int port_num;
	int idx = 0;
	int varlen = 80;

#ifdef CONFIG_I2C_SLAVE_WMT
#ifdef USE_UBOOT_PARA
	ret = wmt_getsyspara(varname1, buf, &varlen);
#else
	ret = 1;
#endif
	is_master = 1;
	if (ret == 0) {
		ret = sscanf(buf, "%x", &port_num);
		while (ret) {
			if (port_num != 0)
				is_master = 1;
			else {
				is_master = 0;
				break;
			}
			idx += ret;
			ret = sscanf(buf + idx, ",%x", &port_num);
		}
	} else
		is_master = 1;
#endif
	wmt_i2c0_is_master = is_master;
	if (is_master == 1) {
#ifdef USE_UBOOT_PARA
		ret = wmt_getsyspara(varname, buf, &varlen);
#else
		ret = 1;
#endif

		if (ret == 0) {
			ret = sscanf(buf, "%x:%x", &port_num, &speed_mode);
			while (ret) {
				if (ret < 2)
					speed_mode = 0;
				else {
					if (port_num != 0)
						speed_mode = 0;
					else
						break;
				}
				idx += (ret + 1);
				ret = sscanf(buf + idx, ",%x:%x", &port_num, &speed_mode);
			}
		}
		if (speed_mode > 1)
			speed_mode = 0;
		wmt_i2c0_speed_mode = speed_mode;

		/**/
		/* software initial*/
		/**/
		i2c.regs        = (struct i2c_regs_s *)I2C0_BASE_ADDR ;
		i2c.irq_no      = IRQ_I2C0 ;

		printk("PORT 0 speed_mode = %d\n", speed_mode);
		if (speed_mode == 0)
			i2c.i2c_mode    = I2C_STANDARD_MODE ;
		else if (speed_mode == 1)
			i2c.i2c_mode    = I2C_FAST_MODE ;

		i2c.isr_nack    = 0 ;
		i2c.isr_byte_end = 0 ;
		i2c.isr_timeout = 0 ;
		i2c.isr_int_pending = 0;
		/**/
		/* hardware initial*/
		/**/
		auto_pll_divisor(DEV_I2C0, CLK_ENABLE, 0, 0);
		auto_pll_divisor(DEV_I2C0, SET_DIV, 2, 20);/*20M Hz*/
		*(volatile unsigned int *)CTRL_GPIO21 &= ~(BIT0 | BIT1);
		*(volatile unsigned int *)PU_EN_GPIO21 |= (BIT0 | BIT1);
		*(volatile unsigned int *)PU_CTRL_GPIO21 |= (BIT0 | BIT1);
		i2c.regs->cr_reg  = 0 ;
		i2c.regs->div_reg = APB_96M_I2C_DIV ;
		i2c.regs->isr_reg = I2C_ISR_ALL_WRITE_CLEAR ;   /* 0x0007*/
		i2c.regs->imr_reg = I2C_IMR_ALL_ENABLE ;        /* 0x0007*/

		i2c.regs->cr_reg  = I2C_CR_ENABLE ;
		tmp = i2c.regs->csr_reg ;                     /* read clear*/
		i2c.regs->isr_reg = I2C_ISR_ALL_WRITE_CLEAR ; /* 0x0007*/

		if (i2c.i2c_mode == I2C_STANDARD_MODE)
			i2c.regs->tr_reg = I2C_TR_STD_VALUE ;   /* 0x8064*/
		else if (i2c.i2c_mode == I2C_FAST_MODE)
			i2c.regs->tr_reg = I2C_TR_FAST_VALUE ; /* 0x8019*/
	}


	if (i2c_wmt_resource_init() == 0) {
		if (wmt_i2c_add_bus(&i2c_wmt_ops) < 0) {
			i2c_wmt_resource_release();
			printk(KERN_INFO "i2c: Failed to add bus\n");
			return -ENODEV;
		}
	} else
		return -ENODEV;

	INIT_LIST_HEAD(&wmt_i2c_fifohead);

	printk(KERN_INFO "i2c: successfully added bus\n");

#ifdef I2C_REG_TEST
	printk("i2c.regs->cr_reg= 0x%08x\n\r", i2c.regs->cr_reg);
	printk("i2c.regs->tcr_reg= 0x%08x\n\r", i2c.regs->tcr_reg);
	printk("i2c.regs->csr_reg= 0x%08x\n\r", i2c.regs->csr_reg);
	printk("i2c.regs->isr_reg= 0x%08x\n\r", i2c.regs->isr_reg);
	printk("i2c.regs->imr_reg= 0x%08x\n\r", i2c.regs->imr_reg);
	printk("i2c.regs->cdr_reg= 0x%08x\n\r", i2c.regs->cdr_reg);
	printk("i2c.regs->tr_reg= 0x%08x\n\r", i2c.regs->tr_reg);
	printk("i2c.regs->div_reg= 0x%08x\n\r", i2c.regs->div_reg);
#endif

	return 0;
}

static void i2c_adap_wmt_exit(void)
{
	wmt_i2c_del_bus(&i2c_wmt_ops);
	i2c_wmt_resource_release();

	printk(KERN_INFO "i2c: successfully removed bus\n");
}


MODULE_AUTHOR("WonderMedia Technologies, Inc.");
MODULE_DESCRIPTION("WMT I2C Adapter Driver");
MODULE_LICENSE("GPL");

module_init(i2c_adap_wmt_init);
module_exit(i2c_adap_wmt_exit);

