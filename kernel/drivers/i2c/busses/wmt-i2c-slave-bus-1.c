/*++
	drivers/i2c/busses/wmt-i2c-slave-bus-1.c

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

	History:
		2010/03/15 First Version
--*/

#include <linux/config.h>
#define WMT_I2C1_SLAVE_C

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
#include <asm/semaphore.h>
#include <linux/proc_fs.h>

#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <mach/hardware.h>
#include "./wmt-i2c-slave-bus.h"
/*#define DEBUG*/
#ifdef DEBUG
#define DPRINTK(fmt, args...)   printk(KERN_DEBUG "%s: " fmt, __func__ , ## args)
#else
#define DPRINTK(fmt, args...)
#endif


#define DEVICE_NAME "WMT_I2C_SLAVE-1"
#define PMC_ClOCK_ENABLE_LOWER          0xd8130250
#define CTRL_GPIO21 0xD8110055  
#define PU_EN_GPIO21 0xD8110495
#define PU_CTRL_GPIO21 0xD81104D5

#define DISABLE_I2C_SLAVE BIT15

struct i2c_slave_msg {
        __u16 addr;     /* slave address */
        __u16 flags;
#define I2C_M_RD        0x01
        __u16 len;      /* data length */
        __u8 *buf;      /* pointer to data */
};

struct slave_i2c_dev_s {
	/* module parameters */
	char *buf;

	/* char dev struct */
	struct cdev cdev;
	struct class *class_slave_i2c;
};


struct wmt_slave_i2c_s {
	struct i2c_regs_s *regs;
	int irq_no ;
	enum i2c_mode_e i2c_mode ;
	int isr_nack ;
	int isr_byte_end ;
	int isr_timeout ;
	int isr_int_pending ;
	struct compat_semaphore tx_sem;
	struct compat_semaphore rx_sem;
};

static struct i2c_regs_s regs_backup;
extern int wmt_getsyspara(char *varname, unsigned char *varval, int *varlen);
static unsigned int is_master = 1;/*master:1, slave:0*/

static DEFINE_SPINLOCK(slave_i2c_lock);

static int slave_i2c_dev_major = SLAVE_I2C_MAJOR;
static int slave_i2c_dev_minor = 1;
static int slave_i2c_dev_nr = 1;
static struct slave_i2c_dev_s slave_i2c_dev;

static struct wmt_slave_i2c_s slave_i2c_port;

static unsigned char slave_i2c_addr = 0x31;

#ifdef CONFIG_PROC_FS
static struct proc_dir_entry *slave_i2c_proc;

static int slave_i2c_reg_read(char *buf, char **start, off_t offset, int len)
{
	char *p = buf;
	p += sprintf(p, "reg : value\n");
	p += sprintf(p, "cr : 0x%.4x\n", slave_i2c_port.regs->cr_reg);
	p += sprintf(p, "tcr : 0x%.4x\n", slave_i2c_port.regs->tcr_reg);
	p += sprintf(p, "csr : 0x%.4x\n", slave_i2c_port.regs->csr_reg);
	p += sprintf(p, "isr : 0x%.4x\n", slave_i2c_port.regs->isr_reg);
	p += sprintf(p, "imr : 0x%.4x\n", slave_i2c_port.regs->imr_reg);
	p += sprintf(p, "cdr : 0x%.4x\n", slave_i2c_port.regs->cdr_reg);
	p += sprintf(p, "tr : 0x%.4x\n", slave_i2c_port.regs->tr_reg);
	p += sprintf(p, "scr : 0x%.4x\n", slave_i2c_port.regs->scr_reg);
	p += sprintf(p, "cssr : 0x%.4x\n", slave_i2c_port.regs->cssr_reg);
	p += sprintf(p, "simr : 0x%.4x\n", slave_i2c_port.regs->simr_reg);
	p += sprintf(p, "sisr : 0x%.4x\n", slave_i2c_port.regs->sisr_reg);
	p += sprintf(p, "csdr : 0x%.4x\n", slave_i2c_port.regs->csdr_reg);
	p += sprintf(p, "str : 0x%.4x\n", slave_i2c_port.regs->str_reg);
	return p - buf;
}

static int slave_i2c_addr_read(char *buf, char **start, off_t offset, int len)
{
	char *p = buf;
	p += sprintf(p, "i2c-slave address : 0x%.2x\n", slave_i2c_addr);
	return p - buf;
}

#endif

static irqreturn_t slave_i2c_isr(
	int irq,		/*!<; // IRQ number */
	void *dev,		/*!<; // Private paramater(i.e. pointer of spi port) */
	struct pt_regs *regs	/*!<; // interrupt register */
)
{
	unsigned short isr_status = slave_i2c_port.regs->sisr_reg;
	unsigned short ssr_status = slave_i2c_port.regs->cssr_reg;
	DPRINTK("slave_i2c isr sisr = %x\n", isr_status);
	DPRINTK("slave_i2c isr cssr = %x\n", ssr_status);
	if (isr_status & I2C_SISR_DAT_REQ) {
		slave_i2c_port.regs->sisr_reg |= I2C_SISR_DAT_REQ_WRITE_CLEAR;
		slave_i2c_port.isr_nack = ssr_status & I2C_SRCV_NACK_MASK;
		compat_up(&slave_i2c_port.tx_sem);
	} else if (isr_status & I2C_SISR_BYTE_END) {
		slave_i2c_port.regs->sisr_reg |= I2C_SISR_BYTE_END_WRITE_CLEAR;
		slave_i2c_port.isr_byte_end = 1;
		slave_i2c_port.isr_nack = ssr_status & I2C_SRCV_NACK_MASK;
		compat_up(&slave_i2c_port.rx_sem);
	} else if (isr_status & I2C_SISR_SCL_TIME_OUT) {
		slave_i2c_port.regs->sisr_reg |= I2C_SISR_SCL_TIME_OUT_WRITE_CLEAR;
		slave_i2c_port.isr_timeout = 1 ;
	}
	return IRQ_HANDLED;
}

static int wmt_slave_i2c_read_data(unsigned short addr , unsigned char *buf, int size, int flags)
{
	int ret = 0;
	int xfer_len = 0;
	int sleep_count = 1;
	DPRINTK("ssr = %x, isr = %x\n", slave_i2c_port.regs->cssr_reg, slave_i2c_port.regs->sisr_reg);
	while (xfer_len < size) {
		compat_down_interruptible(&slave_i2c_port.rx_sem);
		buf[xfer_len] = ((slave_i2c_port.regs->csdr_reg & I2C_SLAVE_READ_DATA_MASK) >> I2C_SLAVE_READ_DATA_SHIFT);
		DPRINTK("data = %x\n", buf[xfer_len]);
		xfer_len++;
	}
	ret = xfer_len;
	while (slave_i2c_port.regs->cssr_reg & I2C_SLAVE_ACTIVE) {
		if (compat_sema_count(&slave_i2c_port.rx_sem) > 0) {/*receive data was longer than request*/
			ret = -1;
			break;
		}
		msleep(sleep_count);
		if (sleep_count < 16)
			sleep_count *= 2;
	}
	DPRINTK("ssr = %x, isr = %x\n", slave_i2c_port.regs->cssr_reg, slave_i2c_port.regs->sisr_reg);

	return ret;
}

static int wmt_slave_i2c_write_data(unsigned short addr, unsigned char *buf, int size, int flags)
{
	int ret = 0;
	int xfer_len = 0;
	int sleep_count = 1;
	DPRINTK("tx ssr = %x, isr = %x\n", slave_i2c_port.regs->cssr_reg, slave_i2c_port.regs->sisr_reg);
	while (xfer_len < size) {
		compat_down_interruptible(&slave_i2c_port.tx_sem);
		slave_i2c_port.regs->csdr_reg = buf[xfer_len];
		DPRINTK("data = %x\n", buf[xfer_len]);
		++xfer_len;
	}
	ret = xfer_len;
	while (slave_i2c_port.regs->cssr_reg & I2C_SLAVE_ACTIVE) {
		msleep(sleep_count);
		if (sleep_count < 16)
			sleep_count *= 2;
	}
	DPRINTK("2.tx ssr = %x, isr = %x\n", slave_i2c_port.regs->cssr_reg, slave_i2c_port.regs->sisr_reg);
	return ret;
}

static int slave_i2c_do_xfer(unsigned short addr, unsigned char *buf, int size, int flags)
{
	int ret;
	if (flags & I2C_M_RD)
		ret = wmt_slave_i2c_read_data(addr , buf, size, flags);
	else
		ret = wmt_slave_i2c_write_data(addr , buf, size, flags);
	return ret;
}

int wmt_i2cslave_transfer1(struct i2c_slave_msg slave_msg)
{
	int flags = slave_msg.flags;
	unsigned char *buf = slave_msg.buf;
	unsigned short addr = slave_msg.addr;
	int size = slave_msg.len;
	int xfer_len;
	if (is_master == 1)
		return 0;
	spin_lock(&slave_i2c_lock);
	xfer_len = slave_i2c_do_xfer(addr, buf, size, flags);
	spin_unlock(&slave_i2c_lock);
	return xfer_len;
}
EXPORT_SYMBOL(wmt_i2cslave_transfer1);

void wmt_i2cslave_setaddr1(struct i2c_slave_msg msg)
{
	if (is_master == 1)
		return;
	spin_lock(&slave_i2c_lock);

	slave_i2c_addr = msg.addr;

	if (msg.addr & DISABLE_I2C_SLAVE)
		slave_i2c_port.regs->cr_reg &= ~I2C_CR_ENABLE;
	else {
		slave_i2c_port.regs->cr_reg = 0;
		slave_i2c_port.regs->scr_reg = 0;
		slave_i2c_port.regs->cr_reg = (I2C_SLAV_MODE_SEL|I2C_CR_ENABLE);
		slave_i2c_port.regs->sisr_reg = I2C_SISR_ALL_WRITE_CLEAR;

		if (slave_i2c_port.i2c_mode == I2C_STANDARD_MODE)
			slave_i2c_port.regs->scr_reg = slave_i2c_addr;
		else if (slave_i2c_port.i2c_mode == I2C_FAST_MODE)
			slave_i2c_port.regs->scr_reg = slave_i2c_addr;
		else
			slave_i2c_port.regs->scr_reg = (slave_i2c_addr|I2C_SLAVE_HS_MODE);

		slave_i2c_port.regs->simr_reg = I2C_SIMR_ALL_ENABLE;
	}
	spin_unlock(&slave_i2c_lock);
}
EXPORT_SYMBOL(wmt_i2cslave_setaddr1);

static ssize_t slave_i2c_read(
	struct file *filp,
	char __user *buf, 
	size_t count,
	loff_t *f_pos
)
{
	int ret = 0;
	struct i2c_slave_msg slave_msg;
	if (is_master == 1)
		return ret;
	slave_msg.buf = (char *)kmalloc(count * sizeof(unsigned char), GFP_KERNEL);
	slave_msg.flags = 0;
	slave_msg.flags |= I2C_M_RD;
	slave_msg.len = count;
	slave_msg.addr = slave_i2c_addr;
	ret = wmt_i2cslave_transfer1(slave_msg);
	if (copy_to_user(buf, slave_msg.buf, count)) {
		kfree(slave_msg.buf);
		return -EFAULT;
	}
	return ret;
}

static ssize_t slave_i2c_write(
	struct file *filp,
	const char __user *buf,
	size_t count,
	loff_t *f_pos
)
{
	int ret = 0;
	struct i2c_slave_msg slave_msg;
	if (is_master == 1)
		return ret;
	slave_msg.buf = (char *)kmalloc(count * sizeof(unsigned char), GFP_KERNEL);
	slave_msg.flags = 0;
	slave_msg.flags &= ~I2C_M_RD;
	slave_msg.len = count;
	slave_msg.addr = slave_i2c_addr;
	if (copy_from_user(slave_msg.buf, buf, count)) {
		kfree(slave_msg.buf);
		return -EFAULT;
	}
	ret = wmt_i2cslave_transfer1(slave_msg);
	return ret; /* return Write out data size*/
}

static int slave_i2c_open(
	struct inode *inode,
	struct file *filp
)
{
	struct slave_i2c_dev_s *dev;
	char name[40];
	int minor_no;

	dev = container_of(inode->i_cdev, struct slave_i2c_dev_s, cdev);
	filp->private_data = dev;
	minor_no = iminor(inode);	/* get */

	/* Create user name*/
	memset(name, 0x0, 8);
	sprintf(name, "slave-i2c%d", minor_no);
	return 0;
}

static int slave_i2c_release(
	struct inode *inode,
	struct file *filp
)
{
	struct slave_i2c_dev_s *dev;
	int minor_no;

	dev = container_of(inode->i_cdev, struct slave_i2c_dev_s, cdev);
	minor_no = iminor(inode);

	return 0;
}

static int slave_i2c_ioctl(
	struct inode *inode,
	struct file *filp,
	unsigned int cmd,
	unsigned long arg
)
{
	int ret = 0;
	struct i2c_slave_msg slave_msg[1];
	unsigned char *data_ptr;
	unsigned char __user *usr_ptr;
	switch (cmd) {
	case IOCTL_DO_TRANSFER:
		if (copy_from_user(slave_msg, (struct i2c_slave_msg *)arg,
				   sizeof(struct i2c_slave_msg)))
			return -EFAULT;

		data_ptr = (unsigned char *)kmalloc(slave_msg->len*sizeof(unsigned char), GFP_KERNEL);
		usr_ptr = (unsigned char __user *)slave_msg->buf;

		if (copy_from_user(data_ptr, (unsigned char *)slave_msg->buf,
				   slave_msg->len*sizeof(unsigned char))) {
			kfree(data_ptr);
			return -EFAULT;
		}
		slave_msg->buf = data_ptr;
		
		ret = wmt_i2cslave_transfer1((struct i2c_slave_msg)*slave_msg);

		if (slave_msg->flags & I2C_M_RD) {
			if (copy_to_user(
				usr_ptr,
				data_ptr,
				slave_msg->len))
				ret = -EFAULT;
		}
			
		kfree(data_ptr);
		break;
	case IOCTL_SET_ADDR:
		if (copy_from_user(slave_msg, (struct i2c_slave_msg *)arg,
				   sizeof(struct i2c_slave_msg)))
			return -EFAULT;
		wmt_i2cslave_setaddr1((struct i2c_slave_msg) *slave_msg);
		break;
	case IOCTL_QUERY_DATA:
		break;
	case IOCTL_SET_SPEED_MODE:
		break;
	default:
		break;
	}
	return ret;
}

/*!*************************************************************************
	driver file operations struct define
****************************************************************************/
static struct file_operations i2c_slave_fops = {
	.owner = THIS_MODULE,
	.open = slave_i2c_open,
	.read = slave_i2c_read,
	.write = slave_i2c_write,
	.ioctl = slave_i2c_ioctl,
	.release = slave_i2c_release,
};

static int slave_i2c_probe(
	struct device *dev
)
{
	int ret = 0;
	dev_t dev_no;
	struct cdev *cdev;
	char name[40];
	char buf[80];
	unsigned int port_num;
	int idx = 0;
	char varname1[] = "wmt.bus.i2c.slave_port";
	int ret_val = 0;
	int varlen = 80;
	memset(name, 0, 40);

	dev_no = MKDEV(slave_i2c_dev_major, slave_i2c_dev_minor);

	sprintf(name, "wmt_i2cslave%d",slave_i2c_dev_minor); 
	cdev = &slave_i2c_dev.cdev;
	cdev_init(cdev, &i2c_slave_fops);
	ret = cdev_add(cdev, dev_no, 8);
	slave_i2c_dev.class_slave_i2c = class_create(THIS_MODULE, "wmt_i2cslave1");
        device_create(slave_i2c_dev.class_slave_i2c, NULL ,
				MKDEV(slave_i2c_dev_major,slave_i2c_dev_minor),
                                NULL, name);
	if (ret) {
		printk(KERN_ALERT "*E* register char dev \n");
		return ret;
	}

#ifdef CONFIG_PROC_FS
	struct proc_dir_entry *res;

	slave_i2c_proc = proc_mkdir("driver/wmt_i2cslave1", NULL);
	/*
	slave_i2c_proc->owner = THIS_MODULE;
	*/
	res = create_proc_entry("registers", 0, slave_i2c_proc);
	if (res) {
    	    res->read_proc = slave_i2c_reg_read;
	}
	
	res = create_proc_entry("address", 0, slave_i2c_proc);
	if (res) {
    	    res->read_proc = slave_i2c_addr_read;
	}
#endif
	slave_i2c_port.regs = (struct i2c_regs_s *)I2C1_BASE_ADDR;
	slave_i2c_port.irq_no = IRQ_I2C1;
	slave_i2c_port.i2c_mode = I2C_STANDARD_MODE;
	slave_i2c_port.isr_nack = 0;
	slave_i2c_port.isr_byte_end = 0;
	slave_i2c_port.isr_timeout = 0;
	slave_i2c_port.isr_int_pending = 0;
	compat_sema_init(&slave_i2c_port.tx_sem, 0);
	compat_sema_init(&slave_i2c_port.rx_sem, 0);
	ret_val = wmt_getsyspara(varname1, buf, &varlen);
	is_master = 1;
	if (ret_val == 0) {
		ret_val = sscanf(buf, "%x", &port_num);
		while (ret_val) {
			if (port_num != 1)
				is_master = 1;
			else {
				is_master = 0;
				break;
			}
			idx += ret_val;
			ret_val = sscanf(buf + idx, ",%x", &port_num);
		}
	} else
		is_master = 1;
	/**/
	/* hardware initial*/
	/**/
	if (is_master == 0) {
		*(volatile unsigned int *)PMC_ClOCK_ENABLE_LOWER |= (BIT0);
		*(volatile unsigned int *)CTRL_GPIO21 &= ~(BIT2 | BIT3);
		*(volatile unsigned int *)PU_EN_GPIO21 |= (BIT2 | BIT3);
		*(volatile unsigned int *)PU_CTRL_GPIO21 |= (BIT2 | BIT3);

		/*set i2c slave register*/
		slave_i2c_port.regs->cr_reg = 0;
		slave_i2c_port.regs->scr_reg = 0;
		slave_i2c_port.regs->cr_reg = (I2C_SLAV_MODE_SEL|I2C_CR_ENABLE);
		slave_i2c_port.regs->sisr_reg = I2C_SISR_ALL_WRITE_CLEAR;

		if (slave_i2c_port.i2c_mode == I2C_STANDARD_MODE)
			slave_i2c_port.regs->scr_reg = slave_i2c_addr;
		else if (slave_i2c_port.i2c_mode == I2C_FAST_MODE)
			slave_i2c_port.regs->scr_reg = slave_i2c_addr;
		else
			slave_i2c_port.regs->scr_reg = (slave_i2c_addr|I2C_SLAVE_HS_MODE);

		slave_i2c_port.regs->simr_reg = I2C_SIMR_ALL_ENABLE;

		slave_i2c_port.regs->cr_reg &= ~I2C_CR_ENABLE;

		if (request_irq(slave_i2c_port.irq_no , &slave_i2c_isr, IRQF_DISABLED, "i2c-slave", 0) < 0) {
			DPRINTK(KERN_INFO "I2C-SLAVE: Failed to register I2C-SLAVE irq %i\n", slave_i2c_port.irq_no);
			return -ENODEV;
		}
	}

	return ret;
}

static int slave_i2c_remove(
	struct device *dev	/*!<; // please add parameters description her*/
)
{
	struct cdev *cdev;

	cdev = &slave_i2c_dev.cdev;
	cdev_del(cdev);

	return 0;
} /* End of spi_remove() */

static void slave_i2c_backup(void)
{
	regs_backup.cr_reg = slave_i2c_port.regs->cr_reg;
	regs_backup.tcr_reg = slave_i2c_port.regs->tcr_reg;
	regs_backup.scr_reg = slave_i2c_port.regs->scr_reg;
	regs_backup.simr_reg = slave_i2c_port.regs->simr_reg;
	regs_backup.str_reg = slave_i2c_port.regs->str_reg;
		
}

static void slave_i2c_restore(void)
{
	slave_i2c_port.regs->cr_reg = 0;
	slave_i2c_port.regs->cr_reg = regs_backup.cr_reg;
	slave_i2c_port.regs->tcr_reg = regs_backup.tcr_reg;
	slave_i2c_port.regs->scr_reg = regs_backup.scr_reg;
	slave_i2c_port.regs->simr_reg = regs_backup.simr_reg;
	slave_i2c_port.regs->str_reg = regs_backup.str_reg;
}

static int slave_i2c_suspend(
	struct device *dev,	/*!<; // please add parameters description her*/
	pm_message_t state	/*!<; // please add parameters description her*/
)
{
	if (is_master == 1)
		return 0;
	slave_i2c_backup();
	compat_sema_init(&slave_i2c_port.tx_sem, 0);
	compat_sema_init(&slave_i2c_port.rx_sem, 0);
	return 0;
}


static int slave_i2c_resume(
	struct device *dev
)
{
	if (is_master == 1)
		return 0;
	*(volatile unsigned int *)PMC_ClOCK_ENABLE_LOWER |= (BIT0);
	*(volatile unsigned int *)CTRL_GPIO21 &= ~(BIT2 | BIT3);
	*(volatile unsigned int *)PU_EN_GPIO21 |= (BIT2 | BIT3);
	*(volatile unsigned int *)PU_CTRL_GPIO21 |= (BIT2 | BIT3);
	slave_i2c_restore();
	return 0;
}

/*!*************************************************************************
	device driver struct define
****************************************************************************/
static struct device_driver slave_i2c_driver = {
	.name           = "wmt_i2c_slave_1", /* This name should equal to platform device name.*/
	.bus            = &platform_bus_type,
	.probe          = slave_i2c_probe,
	.remove         = slave_i2c_remove,
	.suspend        = slave_i2c_suspend,
	.resume         = slave_i2c_resume
};

static void slave_i2c_platform_release(
	struct device *device
)
{
}

/*!*************************************************************************
	platform device struct define
****************************************************************************/

static struct platform_device slave_i2c_device = {
	.name           = "wmt_i2c_slave_1",
	.id             = 1,
	.dev            = 	{	.release = slave_i2c_platform_release,
				},
	.num_resources  = 0,
	.resource       = NULL,
};

static int slave_i2c_init(void)
{
	int ret;
	dev_t dev_no;
	char dev_name[40];
	memset(dev_name, 0, 40);
	sprintf(dev_name, "wmt_i2c_slave%d", slave_i2c_device.id);

	if (slave_i2c_dev_major) {
		dev_no = MKDEV(slave_i2c_dev_major, slave_i2c_dev_minor);
		ret = register_chrdev_region(dev_no, slave_i2c_dev_nr, dev_name);
	} else {
		ret = alloc_chrdev_region(&dev_no, slave_i2c_dev_minor, slave_i2c_dev_nr, dev_name);
		slave_i2c_dev_major = MAJOR(dev_no);
	}

	if (ret < 0) {
		printk(KERN_ALERT "*E* can't get major %d\n", slave_i2c_dev_major);
		return ret;
	}

	platform_device_register(&slave_i2c_device);
	ret = driver_register(&slave_i2c_driver);

	return ret;
}

module_init(slave_i2c_init);

static void slave_i2c_exit(void)
{
	dev_t dev_no;

	driver_unregister(&slave_i2c_driver);
	platform_device_unregister(&slave_i2c_device);
	dev_no = MKDEV(slave_i2c_dev_major, slave_i2c_dev_minor);
	unregister_chrdev_region(dev_no, slave_i2c_dev_nr);

	return;
}

module_exit(slave_i2c_exit);

MODULE_AUTHOR("WMT SW Team");
MODULE_DESCRIPTION("WMT_slave_i2c device driver");
MODULE_LICENSE("GPL");
#undef WMT_I2C_SLAVE_C
