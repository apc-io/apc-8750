/* drivers/i2c/chips/kxte9.c - KXTE9 accelerometer driver
 *
 * Copyright (C) 2010 Kionix, Inc.
 * Written by Kuching Tan <kuchingtan@kionix.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <linux/err.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/uaccess.h>
#include <linux/workqueue.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
//#include <linux/kxte9.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <mach/hardware.h>
#include "kxte9.h"
#include <linux/earlysuspend.h>
#include <mach/wmt-i2c-bus.h>

#define NAME			"kxte9"
#define G_MAX			2000
/* OUTPUT REGISTERS */
#define CT_RESP			0x0C
#define WHO_AM_I		0x0F
#define TILT_POS_CUR	0x10
#define TILT_POS_PRE	0x11
#define XOUT			0x12
#define INT_STATUS_REG	0x16
#define INT_SRC_REG2	0x17
#define INT_REL			0x1A
/* CONTROL REGISTERS */
#define CTRL_REG1		0x1B
#define CTRL_REG2		0x1C
#define CTRL_REG3		0x1D
#define INT_CTRL1		0x1E
#define INT_CTRL2		0x1F
#define TILT_TIMER		0x28
#define WUF_TIMER		0x29
#define B2S_TIMER		0x2A
#define WUF_THRESH		0x5A
#define B2S_THRESH		0x5B
/* CTRL_REG1 BITS */
#define PC1_OFF			0x00
#define PC1_ON			0x80
/* INT_SRC_REG2 BITS */
#define TPS				0x01
#define WUFS			0x02
#define B2SS			0x04
/* Direction Mask */
/* Used for TILT_POS_CUR, TILT_POS_PRE	*/
/*			INT_SRC_REG1, CTRL_REG2		*/
#define	DIR_LE			0x20
#define DIR_RI			0x10
#define DIR_DO			0x08
#define DIR_UP			0x04
#define DIR_FD			0x02
#define DIR_FU			0x01
/* ODR MASKS */
#define ODRM			0x18 // CTRL_REG1
#define	OWUFM			0x03 // CTRL_REG3
#define OB2SM			0x0C // CTRL_REG3
/* INPUT_ABS CONSTANTS */
#define FUZZ			32
#define FLAT			32
/* RESUME STATE INDICES */
#define RES_CTRL_REG1		0
#define RES_CTRL_REG3		1
#define RES_INT_CTRL1		2
#define RES_TILT_TIMER		3
#define RES_WUF_TIMER		4
#define RES_B2S_TIMER		5
#define RES_WUF_THRESH		6
#define RES_B2S_THRESH		7
#define RES_CURRENT_ODR		8
#define RESUME_ENTRIES		9
/* OFFSET and SENSITIVITY */
//#define	OFFSET		32 //6-bit
#define	OFFSET		128 //8-bit
#define SENS		16

#define IOCTL_BUFFER_SIZE	64

//#define WM3445_A0
//#define INT_MODE

//////////////////////////////////////////////////////////////////////////
//#define DEBUG_WMT_GSENSOR
#ifdef DEBUG_WMT_GSENSOR
#define kxte9_dbg(fmt, args...) printk(KERN_ALERT "[%s]: " fmt, __FUNCTION__ , ## args)
//#define kxte9_dbg(fmt, args...) if (kpadall_isrundbg()) printk(KERN_ALERT "[%s]: " fmt, __FUNCTION__, ## args)
#else
#define kxte9_dbg(fmt, args...)
#endif

#undef errlog
#undef klog
#define errlog(fmt, args...) printk(KERN_ERR "[%s]: " fmt, __FUNCTION__, ## args)
#define klog(fmt, args...) printk(KERN_DEBUG "[%s]: " fmt, __FUNCTION__, ## args)
//////////////////////////////////////////////////////////////////////////

/*
 * The following table lists the maximum appropriate poll interval for each
 * available output data rate.
 */
struct {
	unsigned int interval;
	u8 mask;
} kxte9_odr_table[] = {
	{1000, ODR1E},
	{334, ODR3E}, 
	{100, ODR10E}, 
	{25, ODR40E},
	{8, ODR125E},
};

struct kxte9_data {
	//struct i2c_client *client;
	struct kxte9_platform_data *pdata;
	struct mutex lock;
	struct delayed_work input_work;
	struct input_dev *input_dev;
#ifdef INT_MODE
	struct work_struct irq_work;
#endif
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend earlysuspend;
#endif
	int hw_initialized;
	atomic_t enabled;
	u8 resume[RESUME_ENTRIES];
	int i2c_xfer_complete;
	int suspend;
};

struct gsensor_config
{
	int op;
	int samp;
	int xyz_axis[3][3]; // (axis,direction)
	struct proc_dir_entry* sensor_proc;
	int sensorlevel;
	unsigned int avg_count;
	unsigned int kxte9_8bit;
	int name;
	int bmp;
	unsigned int ctraddr;
	unsigned int ocaddr;
	unsigned int idaddr;
	unsigned int peaddr;
	unsigned int pcaddr;
	unsigned int itbmp;
	unsigned int itaddr;
	unsigned int isbmp;
	unsigned int isaddr;
	int irq;
};

static struct gsensor_config gconf = {
	.op = 0,
	.samp = 40,
	.xyz_axis = {
		{ABS_X, -1},
		{ABS_Y, 1},
		{ABS_Z, -1},
		},
	.sensor_proc = NULL,
	.avg_count = 4,
	.kxte9_8bit = 1,
	.name = 3,
#ifdef WM3445_A0
	.bmp = 0x100, /* GPIO 8 */
	.ctraddr = GPIO_BASE_ADDR + 0x40,
	.ocaddr = GPIO_BASE_ADDR + 0x80,
	.idaddr = GPIO_BASE_ADDR + 0x00,
	.peaddr = GPIO_BASE_ADDR + 0x480,
	.pcaddr = GPIO_BASE_ADDR + 0x4c0,
	.itbmp = 0x30000, /* Rising Edge */
	.itaddr = GPIO_BASE_ADDR + 0x300,
	.isbmp = 0x100,
	.isaddr = GPIO_BASE_ADDR + 0x304,
	.irq = IRQ_GPIO8,
#else
	.bmp = 0x8, /* GPIO 3 */
	.ctraddr = GPIO_BASE_ADDR + 0x40,
	.ocaddr = GPIO_BASE_ADDR + 0x80,
	.idaddr = GPIO_BASE_ADDR + 0x00,
	.peaddr = GPIO_BASE_ADDR + 0x480,
	.pcaddr = GPIO_BASE_ADDR + 0x4c0,
	.itbmp = 0x83000000, /* Rising Edge */
	.itaddr = GPIO_BASE_ADDR + 0x300,
	.isbmp = 0x8,
	.isaddr = GPIO_BASE_ADDR + 0x320,
	.irq = 5,
#endif
};

static struct kxte9_platform_data kxte9_pdata = {
	.min_interval = 1,
	.poll_interval = 100,
	.ctrl_reg1_init = ODR10E & ~B2SE & ~WUFE & ~TPE,
	.engine_odr_init = OB2S1 | OWUF1,
	.int_ctrl_init = KXTE9_IEA,
	.tilt_timer_init = 0x00,
	.wuf_timer_init = 0x00,
	.wuf_thresh_init = 0x20,
	.b2s_timer_init = 0x00,
	.b2s_thresh_init = 0x60,
};

#ifdef WM3445_A0
#define SET_GPIO_GSENSOR_INT() {\
	REG32_VAL(gconf.ctraddr) &= ~gconf.bmp; \
	REG32_VAL(gconf.ocaddr) &= ~gconf.bmp; \
	REG32_VAL(gconf.peaddr) |= gconf.bmp; \
	REG32_VAL(gconf.pcaddr) &= ~gconf.bmp; \
	REG32_VAL(gconf.itaddr) |= gconf.itbmp; \
	REG32_VAL(GPIO_BASE_ADDR + 0x308) |= gconf.bmp; \
	REG32_VAL(gconf.isaddr) |= gconf.isbmp; \
}
#define ENABLE_SENSOR_INT(enable) { \
	if (enable) \
	{\
		REG32_VAL(GPIO_BASE_ADDR + 0x308) &= ~gconf.bmp; \
	} else {\
		REG32_VAL(GPIO_BASE_ADDR + 0x308) |= gconf.bmp; \
	}\
}

#else
#define SET_GPIO_GSENSOR_INT() {\
	REG32_VAL(gconf.ctraddr) |= gconf.bmp; \
	REG32_VAL(gconf.ocaddr) &= ~gconf.bmp; \
	REG32_VAL(gconf.pcaddr) &= ~gconf.bmp; \
	REG32_VAL(gconf.itaddr) |= gconf.itbmp; \
	REG32_VAL(gconf.isaddr) |= gconf.isbmp; \
}
#endif

#define X_CONVERT(x) x*gconf.xyz_axis[ABS_X][1]
#define Y_CONVERT(y) y*gconf.xyz_axis[ABS_Y][1]
#define Z_CONVERT(z) z*gconf.xyz_axis[ABS_Z][1]

#ifdef CONFIG_HAS_EARLYSUSPEND
static void kxte9_early_suspend(struct early_suspend *h);
static void kxte9_late_resume(struct early_suspend *h);
#endif

extern int wmt_getsyspara(char *varname, unsigned char *varval, int *varlen);
extern int wmt_i2c_xfer_continue_if_4(struct i2c_msg *msg, unsigned int num, int bus_id);
extern unsigned int wmt_read_oscr(void); 

static struct kxte9_data *te9 = NULL;
static atomic_t kxte9_dev_open_count;
static struct kobject *android_gsensor_kobj = NULL;
static void kxte9_read_callback(void *data);
struct i2c_msg *kxte9_msg;
unsigned char *i2c_read_buf;
unsigned char *i2c_write_buf;
static struct timer_list kxte9_timer;
static unsigned char *x_count;
static unsigned char *y_count;
static unsigned char *z_count;
static unsigned int x_total = 0, y_total = 0, z_total = 0;
static unsigned int xyz_index = 0;

static int wait_i2c_xfer_complete(void)
{
	unsigned int now_time = 0;
	unsigned int delay_time = 0;

	now_time = wmt_read_oscr();
	while (!te9->i2c_xfer_complete)  {
		delay_time = wmt_read_oscr() - now_time;
		if (delay_time > 60000) {//20ms
			printk(KERN_WARNING "[kxte9] transfer timeout!\n");
			return 0;
		}
	}
	return 1;
}

static void kxte9_read_callback(void *data)
{
	int xyz[3];

	if (te9->suspend) {
		te9->i2c_xfer_complete = 1;
		return;
	}
	if (xyz_index >= gconf.avg_count)
		xyz_index = 0;
	x_total -= x_count[xyz_index];
	y_total -= y_count[xyz_index];
	z_total -= z_count[xyz_index];
	if (gconf.kxte9_8bit) {
		x_count[xyz_index] = i2c_read_buf[0];
		y_count[xyz_index] = i2c_read_buf[1];
		z_count[xyz_index] = i2c_read_buf[2];
	} else {
		x_count[xyz_index] = i2c_read_buf[0] & ~0x03;
		y_count[xyz_index] = i2c_read_buf[1] & ~0x03;
		z_count[xyz_index] = i2c_read_buf[2] & ~0x03;	
	}
	x_total += x_count[xyz_index];
	y_total += y_count[xyz_index];
	z_total += z_count[xyz_index];
	xyz[ABS_X] = (x_total/gconf.avg_count - OFFSET) << 4;
	xyz[ABS_Y] = (y_total/gconf.avg_count - OFFSET) << 4;
	xyz[ABS_Z] = (z_total/gconf.avg_count - OFFSET) << 4;
	//printk(KERN_DEBUG "   [%d] x:%d y:%d z:%d\n", xyz_index, x_count[xyz_index], y_count[xyz_index], z_count[xyz_index]);
	//printk(KERN_DEBUG " total x:%d y:%d z:%d\n", x_total, y_total, z_total);
	//printk(KERN_DEBUG "   avg x:%d y:%d z:%d\n", x_total/gconf.avg_count, y_total/gconf.avg_count, z_total/gconf.avg_count);
	//printk(KERN_DEBUG "report x:%d y:%d z:%d\n", xyz[ABS_X], xyz[ABS_Y], xyz[ABS_Z]);
	xyz_index++;
	input_report_abs(te9->input_dev, ABS_X, X_CONVERT(xyz[gconf.xyz_axis[ABS_X][0]]));
	input_report_abs(te9->input_dev, ABS_Y, Y_CONVERT(xyz[gconf.xyz_axis[ABS_Y][0]]));
	input_report_abs(te9->input_dev, ABS_Z, Z_CONVERT(xyz[gconf.xyz_axis[ABS_Z][0]]));
	input_sync(te9->input_dev);
	te9->i2c_xfer_complete = 1;
	mod_timer(&kxte9_timer,  jiffies +  msecs_to_jiffies(te9->pdata->poll_interval));
}

static void kxte9_read_data(u8 addr, int len)
{
	i2c_write_buf[0] = addr;
	kxte9_msg[0].addr = KXTE9_I2C_ADDR;
	kxte9_msg[0].flags = 0 ;
	kxte9_msg[0].len = 1;
	kxte9_msg[0].buf = i2c_write_buf;
	kxte9_msg[1].addr = KXTE9_I2C_ADDR;
	kxte9_msg[1].flags = I2C_M_RD;
	kxte9_msg[1].len = len;
	kxte9_msg[1].buf = i2c_read_buf;
	wmt_i2c_transfer(kxte9_msg, 2, 0, kxte9_read_callback, 0);
}

static int kxte9_i2c_read(u8 addr, u8 *data, int len)
{
	int err;

	struct i2c_msg msgs[] = {
		{
		 .addr = KXTE9_I2C_ADDR,
		 .flags = 0 & ~(I2C_M_RD), //te9->client->flags & I2C_M_TEN,
		 .len = 1,
		 .buf = &addr,
		 },
		{
		 .addr = KXTE9_I2C_ADDR, //te9->client->addr,
		 .flags = (I2C_M_RD), //(te9->client->flags & I2C_M_TEN) | I2C_M_RD,
		 .len = len,
		 .buf = data,
		 },
	};
	err = wmt_i2c_xfer_continue_if_4(msgs, 2, 0);

	if(err != 2)
		errlog("read transfer error\n");
	else
		err = 0;

	return err;
}

static int kxte9_i2c_write(u8 addr, u8 *data, int len)
{
	int err;
	int i;
	u8 buf[len + 1];

	struct i2c_msg msgs[] = {
		{
		 .addr = KXTE9_I2C_ADDR, //te9->client->addr,
		 .flags = 0 & ~(I2C_M_RD), //te9->client->flags & I2C_M_TEN,
		 .len = len + 1,
		 .buf = buf,
		 },
	};

	buf[0] = addr;
	for (i = 0; i < len; i++)
		buf[i + 1] = data[i];

	err = wmt_i2c_xfer_continue_if_4(msgs, 1, 0);
	if(err != 1)
		errlog("write transfer error\n");
	else
		err = 0;
	return err;
}

int kxte9_get_bits(u8 reg_addr, u8* bits_value, u8 bits_mask)
{
	int err;
	u8 reg_data;

	err = kxte9_i2c_read(reg_addr, &reg_data, 1);
	kxte9_dbg("kxte9_i2c_read(%x, 1)=%x, err=%d\n", reg_addr, reg_data, err);
	if(err < 0)
		return err;

	*bits_value = reg_data & bits_mask;

	return 1;
}

int kxte9_get_byte(u8 reg_addr, u8* reg_value)
{
	int err;
	u8 reg_data;

	err = kxte9_i2c_read(reg_addr, &reg_data, 1);
	kxte9_dbg("kxte9_i2c_read(%x, 1)=%x, err=%d\n", reg_addr, reg_data, err);
	if(err < 0)
		return err;

	*reg_value = reg_data;

	return 1;
}

int kxte9_set_bits(int res_index, u8 reg_addr, u8 bits_value, u8 bits_mask)
{
	int err=0, err1=0, retval=0;
	u8 reg_data = 0x00, reg_bits = 0x00, bits_set = 0x00;

	// Turn off PC1
	reg_data = te9->resume[RES_CTRL_REG1] & ~PC1_ON;

	err = kxte9_i2c_write(CTRL_REG1, &reg_data, 1);
	kxte9_dbg("kxte9_i2c_write(%x, %x, 1), err=%d\n", CTRL_REG1, reg_data, err);
	if(err < 0)
		goto exit0;

	// Read from device register
	err = kxte9_i2c_read(reg_addr, &reg_data, 1);
	kxte9_dbg("kxte9_i2c_read(%x, 1)=%x, err=%d\n", reg_addr, reg_data, err);
	if(err < 0)
		goto exit0;

	// Apply mask to device register;
	reg_bits = reg_data & bits_mask;

	// Update resume state data
	bits_set = bits_mask & bits_value;
	te9->resume[res_index] &= ~bits_mask;
	te9->resume[res_index] |= bits_set;

	// Return 0 if value in device register and value to be written is the same
	if(reg_bits == bits_set)
		retval = 0;
	// Else, return 1
	else
		retval = 1;

	// Write to device register
	err = kxte9_i2c_write(reg_addr, &te9->resume[res_index], 1);
	kxte9_dbg("kxte9_i2c_write(%x, %x, 1), err=%d\n", reg_addr, te9->resume[res_index], err);
	if(err < 0)
		goto exit0;

exit0:
	// Turn on PC1
	reg_data = te9->resume[RES_CTRL_REG1] | PC1_ON;

	err1 = kxte9_i2c_write(CTRL_REG1, &reg_data, 1);
	kxte9_dbg("kxte9_i2c_write(%x, %x, 1), err=%d\n", CTRL_REG1, reg_data, err);
	if(err1 < 0)
		return err1;

	if(err < 0)
		return err;

	return retval;
}

int kxte9_set_byte(int res_index, u8 reg_addr, u8 reg_value)
{
	int err, err1, retval=0;
	u8 reg_data;

	// Turn off PC1
	reg_data = te9->resume[RES_CTRL_REG1] & ~PC1_ON;

	err = kxte9_i2c_write(CTRL_REG1, &reg_data, 1);
	kxte9_dbg("kxte9_i2c_write(%x, %x, 1), err=%d\n", CTRL_REG1, reg_data, err);
	if(err < 0)
		goto exit0;

	// Read from device register
	err = kxte9_i2c_read(reg_addr, &reg_data, 1);
	kxte9_dbg("kxte9_i2c_read(%x, 1)=%x, err=%d\n", reg_addr, reg_data, err);
	if(err < 0)
		goto exit0;

	// Update resume state data
	te9->resume[res_index] = reg_value;

	// Return 0 if value in device register and value to be written is the same
	if(reg_data == reg_value)
		retval = 0;
	// Else, return 1
	else
		retval = 1;

	// Write to device register
	err = kxte9_i2c_write(reg_addr, &te9->resume[res_index], 1);
	kxte9_dbg("kxte9_i2c_write(%x, %x, 1), err=%d\n", reg_addr, te9->resume[res_index], err);
	if(err < 0)
		goto exit0;

exit0:
	// Turn on PC1
	reg_data = te9->resume[RES_CTRL_REG1] | PC1_ON;
	err1 = kxte9_i2c_write(CTRL_REG1, &reg_data, 1);
	kxte9_dbg("kxte9_i2c_write(%x, %x, 1), err=%d\n", CTRL_REG1, reg_data, err);
	if(err1 < 0)
		return err1;

	if(err < 0)
		return err;

	return retval;
}

int kxte9_set_pc1_off(void)
{
	u8 reg_data;

	reg_data = te9->resume[RES_CTRL_REG1] & ~PC1_ON;

	kxte9_dbg("kxte9_i2c_write(%x, %x, 1)\n", CTRL_REG1, reg_data);
	return kxte9_i2c_write(CTRL_REG1, &reg_data, 1);
}

int kxte9_set_pc1_on(void)
{
	u8 reg_data;

	reg_data = te9->resume[RES_CTRL_REG1] | PC1_ON;

	kxte9_dbg("kxte9_i2c_write(%x, %x, 1)\n", CTRL_REG1, reg_data);
	return kxte9_i2c_write(CTRL_REG1, &reg_data, 1);
}

static int kxte9_verify(void)
{
	int err;
	u8 buf;

	err = kxte9_i2c_read(WHO_AM_I, &buf, 1);
	kxte9_dbg("kxte9_i2c_read(%x, 1)=%x, err=%d\n", WHO_AM_I, buf, err);
	if(err < 0)
		errlog( "read err int source\n");
	if(buf != 0)
		err = -1;
	return err;
}

static int kxte9_hw_init(void)
{
	int err;
	u8 buf = PC1_OFF;

	err = kxte9_i2c_write(CTRL_REG1, &buf, 1);
	kxte9_dbg("kxte9_i2c_write(%x, %x, 1), err=%d\n", CTRL_REG1, buf, err);
	if(err < 0)
		return err;
	err = kxte9_i2c_write(CTRL_REG3, &te9->resume[RES_CTRL_REG3], 1);
	kxte9_dbg("kxte9_i2c_write(%x, %x, 1), err=%d\n", CTRL_REG3, te9->resume[RES_CTRL_REG3], err);
	if(err < 0)
		return err;
	err = kxte9_i2c_write(INT_CTRL1, &te9->resume[RES_INT_CTRL1], 1);
	kxte9_dbg("kxte9_i2c_write(%x, %x, 1), err=%d\n", INT_CTRL1, te9->resume[RES_INT_CTRL1], err);
	if(err < 0)
		return err;
	err = kxte9_i2c_write(TILT_TIMER, &te9->resume[RES_TILT_TIMER], 1);
	kxte9_dbg("kxte9_i2c_write(%x, %x, 1), err=%d\n", TILT_TIMER, te9->resume[RES_TILT_TIMER], err);
	if(err < 0)
		return err;
	err = kxte9_i2c_write(WUF_TIMER, &te9->resume[RES_WUF_TIMER], 1);
	kxte9_dbg("kxte9_i2c_write(%x, %x, 1), err=%d\n", WUF_TIMER, te9->resume[RES_WUF_TIMER], err);
	if(err < 0)
		return err;
	err = kxte9_i2c_write(B2S_TIMER, &te9->resume[RES_B2S_TIMER], 1);
	kxte9_dbg("kxte9_i2c_write(%x, %x, 1), err=%d\n", B2S_TIMER, te9->resume[RES_B2S_TIMER], err);
	if(err < 0)
		return err;
	err = kxte9_i2c_write(WUF_THRESH, &te9->resume[RES_WUF_THRESH], 1);
	kxte9_dbg("kxte9_i2c_write(%x, %x, 1), err=%d\n", WUF_THRESH, te9->resume[RES_WUF_THRESH], err);
	if(err < 0)
		return err;
	err = kxte9_i2c_write(B2S_THRESH, &te9->resume[RES_B2S_THRESH], 1);
	kxte9_dbg("kxte9_i2c_write(%x, %x, 1), err=%d\n", B2S_THRESH, te9->resume[RES_B2S_THRESH], err);
	if(err < 0)
		return err;
	buf = te9->resume[RES_CTRL_REG1] | PC1_ON;
	err = kxte9_i2c_write(CTRL_REG1, &buf, 1);
	kxte9_dbg("kxte9_i2c_write(%x, %x, 1), err=%d\n", CTRL_REG1, buf, err);
	if(err < 0)
		return err;

	te9->resume[RES_CTRL_REG1] = buf;
	te9->hw_initialized = 1;

	return 0;
}

static void kxte9_device_power_off(void)
{
	int err;
	u8 buf = PC1_OFF;

	err = kxte9_i2c_write(CTRL_REG1, &buf, 1);
	kxte9_dbg("kxte9_i2c_write(%x, %x, 1), err=%d\n", CTRL_REG1, buf, err);
	if(err < 0)
		errlog("soft power off failed\n");
#ifdef INT_MODE
	ENABLE_SENSOR_INT(0);
//	disable_irq(gconf.irq);
#endif
	te9->hw_initialized = 0;
}

static int kxte9_device_power_on(void)
{
	int err;

	if(!te9->hw_initialized) {
		//mdelay(110);
		err = kxte9_hw_init();
		if(err < 0) {
			kxte9_device_power_off();
			return err;
		}
	}
#ifdef INT_MODE
	ENABLE_SENSOR_INT(1);
//	enable_irq(gconf.irq);
#endif
	return 0;
}

static u8 kxte9_resolve_dir(u8 dir)
{
	switch (dir) {
	case 0x20:	/* -X */
		if (gconf.xyz_axis[ABS_X][1] < 0)
			dir = 0x10;
		if (gconf.xyz_axis[ABS_Y][0] == 0)
			dir >>= 2;
		if (gconf.xyz_axis[ABS_Z][0] == 0)
			dir >>= 4;
		break;
	case 0x10:	/* +X */
		if (gconf.xyz_axis[ABS_X][1] < 0)
			dir = 0x20;
		if (gconf.xyz_axis[ABS_Y][0] == 0)
			dir >>= 2;
		if (gconf.xyz_axis[ABS_Z][0] == 0)
			dir >>= 4;
		break;
	case 0x08:	/* -Y */
		if (gconf.xyz_axis[ABS_Y][1] < 0)
			dir = 0x04;
		if (gconf.xyz_axis[ABS_X][0] == 1)
			dir <<= 2;
		if (gconf.xyz_axis[ABS_Z][0] == 1)
			dir >>= 2;
		break;
	case 0x04:	/* +Y */
		if (gconf.xyz_axis[ABS_Y][1] < 0)
			dir = 0x08;
		if (gconf.xyz_axis[ABS_X][0] == 1)
			dir <<= 2;
		if (gconf.xyz_axis[ABS_Z][0] == 1)
			dir >>= 2;
		break;
	case 0x02:	/* -Z */
		if (gconf.xyz_axis[ABS_Z][1] < 0)
			dir = 0x01;
		if (gconf.xyz_axis[ABS_X][0] == 2)
			dir <<= 4;
		if (gconf.xyz_axis[ABS_Y][0] == 2)
			dir <<= 2;
		break;
	case 0x01:	/* +Z */
		if (gconf.xyz_axis[ABS_Z][1] < 0)
			dir = 0x02;
		if (gconf.xyz_axis[ABS_X][0] == 2)
			dir <<= 4;
		if (gconf.xyz_axis[ABS_Y][0] == 2)
			dir <<= 2;
		break;
	default:
		return -EINVAL;
	}

	return dir;
}

#ifdef INT_MODE
static void kxte9_irq_work_func(struct work_struct *work)
{
/*
 *	int_status output:
 *	[INT_SRC_REG1][INT_SRC_REG2][TILT_POS_PRE][TILT_POS_CUR]
 *	INT_SRC_REG2, TILT_POS_PRE, and TILT_POS_CUR directions are translated
 *	based on platform data variables.
 */

	int err;
	int i;
	int int_status = 0;
	u8 status;
	u8 b2s_comp;
	u8 wuf_comp;
	u8 buf[2];

	err = kxte9_i2c_read(INT_STATUS_REG, &status, 1);
	kxte9_dbg("kxte9_i2c_read(%x, 1)=%x, err=%d\n", INT_STATUS_REG, status, err);
	if(err < 0)
		errlog("read err int source\n");
	int_status = status << 24;
	if((status & TPS) > 0) {
		err = kxte9_i2c_read(TILT_POS_CUR, buf, 2);
	kxte9_dbg("kxte9_i2c_read(%x, 2)=%x,%x, err=%d\n", TILT_POS_CUR, buf[0], buf[1], err);
		if(err < 0)
			errlog("read err tilt dir\n");
		int_status |= kxte9_resolve_dir(buf[0]);
		int_status |= (kxte9_resolve_dir(buf[1])) << 8;
		kxte9_dbg("IRQ TILT [%x]\n", kxte9_resolve_dir(buf[0]));
	}
	if((status & WUFS) > 0) {
		kxte9_dbg("for WUFS\n");
		err = kxte9_i2c_read(INT_SRC_REG2, buf, 1);
		kxte9_dbg("kxte9_i2c_read(%x, 1)=%x, err=%d\n", INT_SRC_REG2, buf[0], err);
		if(err < 0)
			kxte9_dbg("reading err wuf dir\n");
		int_status |= (kxte9_resolve_dir(buf[0])) << 16;
		b2s_comp = (te9->resume[RES_CTRL_REG3] & 0x0C) >> 2;
		wuf_comp = te9->resume[RES_CTRL_REG3] & 0x03;
		if(!te9->resume[RES_CURRENT_ODR] &&
				!(te9->resume[RES_CTRL_REG1] & ODR125E) &&
						!(b2s_comp & wuf_comp)) {
			/* set the new poll interval based on wuf odr */
			for (i = 0; i < ARRAY_SIZE(kxte9_odr_table); i++) {
				if(kxte9_odr_table[i].mask ==	wuf_comp << 3) {
					te9->pdata->poll_interval = kxte9_odr_table[i].interval;
					break;
				}
			}
			if(te9->input_dev) {
				cancel_delayed_work_sync(&te9->input_work);
				schedule_delayed_work(&te9->input_work,
						msecs_to_jiffies(te9->pdata->poll_interval));
			}
		}
	}
	if((status & B2SS) > 0) {
		kxte9_dbg("fro B2SS\n");
		b2s_comp = (te9->resume[RES_CTRL_REG3] & 0x0C) >> 2;
		wuf_comp = te9->resume[RES_CTRL_REG3] & 0x03;
		if(!te9->resume[RES_CURRENT_ODR] &&
				!(te9->resume[RES_CTRL_REG1] & ODR125E) &&
						!(b2s_comp & wuf_comp)) {
			/* set the new poll interval based on b2s odr */
			for (i = 1; i < ARRAY_SIZE(kxte9_odr_table); i++) {
				if(kxte9_odr_table[i].mask == b2s_comp << 3) {
					te9->pdata->poll_interval = kxte9_odr_table[i].interval;
					break;
				}
			}
			if(te9->input_dev) {
				cancel_delayed_work_sync(&te9->input_work);
				schedule_delayed_work(&te9->input_work,
						msecs_to_jiffies(te9->pdata->poll_interval));
			}
		}
	}
	input_report_abs(te9->input_dev, ABS_MISC, int_status);
	input_sync(te9->input_dev);
	err = kxte9_i2c_read(INT_REL, buf, 1);
	kxte9_dbg("kxte9_i2c_read(%x, 1)=%x, err=%d\n", INT_REL, buf[0], err);
	if(err < 0)
		errlog("error clearing interrupt\n");
	//enable_irq(gconf.irq);
}

static irqreturn_t kxte9_isr(int irq, void *dev)
{
	unsigned int status = REG32_VAL(gconf.isaddr);

	// clr int status ...??
	if ((status & gconf.isbmp) != 0)
	{
		kxte9_dbg("\n");
		REG32_VAL(gconf.isaddr) |= gconf.isbmp;
		udelay(5);
		// disable int and satrt irq work
		disable_irq_nosync(irq);
		schedule_work(&te9->irq_work);
		return IRQ_HANDLED;
	}
	return IRQ_NONE;
}
#endif

static int kxte9_update_odr(int poll_interval)
{
	int err = -1;
	int i;
	u8 config;

	/*  Convert the poll interval into an output data rate configuration
	 *  that is as low as possible.  The ordering of these checks must be
	 *  maintained due to the cascading cut off values - poll intervals are
	 *  checked from shortest to longest.  At each check, if the next lower
	 *  ODR cannot support the current poll interval, we stop searching */
	for (i = 0; i < ARRAY_SIZE(kxte9_odr_table); i++) {
		config = kxte9_odr_table[i].mask;
		if(poll_interval >= kxte9_odr_table[i].interval)
			break;
	}

	config |= (te9->resume[RES_CTRL_REG1] & ~(ODR40E | ODR125E));
	te9->resume[RES_CTRL_REG1] = config;
	if(atomic_read(&te9->enabled)) {
		err = kxte9_set_byte(RES_CTRL_REG1, CTRL_REG1, config);
		if(err < 0)
			return err;
	}
	klog("Set new ODR to 0x%02X\n", config);
	return 0;
}

static void kxte9_input_work_func(unsigned long unused)
{
	//mutex_lock(&te9->lock);
	if (te9->suspend)
		return;
	if (te9->i2c_xfer_complete) {
		te9->i2c_xfer_complete = 0;
		kxte9_read_data(XOUT, 3);
	}
	//mutex_unlock(&te9->lock);
}

static int kxte9_enable(void)
{
	int err;
	int int_status = 0;
	u8 buf;

	if(!atomic_cmpxchg(&te9->enabled, 0, 1)) {
		err = kxte9_device_power_on();
		err = kxte9_i2c_read(INT_REL, &buf, 1);
		kxte9_dbg("kxte9_i2c_read(%x, 1)=%x, err=%d\n", INT_REL, buf, err);
		if(err < 0) {
			errlog("error clearing interrupt: %d\n", err);
			atomic_set(&te9->enabled, 0);
			return err;
		}
		if((te9->resume[RES_CTRL_REG1] & TPS) > 0) {
			err = kxte9_i2c_read(TILT_POS_CUR, &buf, 1);
			kxte9_dbg("kxte9_i2c_read(%x, 1)=%x, err=%d\n", TILT_POS_CUR, buf, err);
			if(err < 0)
				errlog("kxte9 error reading current tilt\n");
			int_status |= kxte9_resolve_dir(buf);
			input_report_abs(te9->input_dev, ABS_MISC, int_status);
			input_sync(te9->input_dev);
		}
		kxte9_msg = kzalloc(2*sizeof(struct i2c_msg), GFP_ATOMIC);
		i2c_read_buf = kzalloc(3*sizeof(unsigned char), GFP_ATOMIC);
		i2c_write_buf = kzalloc(sizeof(char), GFP_ATOMIC);
		setup_timer(&kxte9_timer, kxte9_input_work_func, 0);
		mod_timer(&kxte9_timer,  jiffies +  msecs_to_jiffies(te9->pdata->poll_interval));
		kxte9_dbg("Enabled\n");
	}
	return 0;
}

static int kxte9_disable(void)
{
	if(atomic_cmpxchg(&te9->enabled, 1, 0)) {
		del_timer_sync(&kxte9_timer);
		wait_i2c_xfer_complete();
		kfree(kxte9_msg);
		kfree(i2c_read_buf);
		kfree(i2c_write_buf);
#ifdef WM3445_A0
		ENABLE_SENSOR_INT(1);
#endif
		kxte9_device_power_off();
		kxte9_dbg(" Disabled\n");
		//#endif
	}
	return 0;
}

int kxte9_input_open(struct input_dev *input)
{
	kxte9_dbg("\n");
	return kxte9_enable();
}

void kxte9_input_close(struct input_dev *dev)
{
	kxte9_dbg("\n");
	kxte9_disable();
}

static int kxte9_input_init(void)
{
	int err;

	te9->input_dev = input_allocate_device();
	if(!te9->input_dev) {
		err = -ENOMEM;
		errlog("input device allocate failed\n");
		goto err0;
	}
	te9->input_dev->open = kxte9_input_open;
	te9->input_dev->close = kxte9_input_close;

	input_set_drvdata(te9->input_dev, te9);

	set_bit(EV_ABS, te9->input_dev->evbit);
	set_bit(ABS_MISC, te9->input_dev->absbit);

	input_set_abs_params(te9->input_dev, ABS_X, -G_MAX, G_MAX, FUZZ, FLAT);
	input_set_abs_params(te9->input_dev, ABS_Y, -G_MAX, G_MAX, FUZZ, FLAT);
	input_set_abs_params(te9->input_dev, ABS_Z, -G_MAX, G_MAX, FUZZ, FLAT);

	te9->input_dev->name = INPUT_NAME_ACC;

	err = input_register_device(te9->input_dev);
	if(err) {
		errlog("unable to register input polled device %s: %d\n",
			te9->input_dev->name, err);
		goto err1;
	}

	return 0;
err1:
	input_free_device(te9->input_dev);
err0:
	return err;
}

static void kxte9_input_cleanup(void)
{
	input_unregister_device(te9->input_dev);
}

/* sysfs */
static ssize_t kxte9_delay_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", te9->pdata->poll_interval);
}

static ssize_t kxte9_delay_store(struct device *dev,
					struct device_attribute *attr,
						const char *buf, size_t count)
{
	int val = simple_strtoul(buf, NULL, 10);
	u8 ctrl;

	te9->pdata->poll_interval = max(val, te9->pdata->min_interval);
	kxte9_update_odr(te9->pdata->poll_interval);
	ctrl = te9->resume[RES_CTRL_REG1] & 0x18;
	te9->resume[RES_CURRENT_ODR] = ctrl;
	/* All ODRs are changed when this method is used. */
	ctrl = (ctrl >> 1) | (ctrl >> 3);
	kxte9_i2c_write(CTRL_REG3, &ctrl, 1);
	kxte9_dbg("kxte9_i2c_write(%x, %x, 1)\n", CTRL_REG3, ctrl);
	te9->resume[RES_CTRL_REG3] = ctrl;
	return count;
}

static ssize_t kxte9_enable_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", atomic_read(&te9->enabled));
}

static ssize_t kxte9_enable_store(struct device *dev,
					struct device_attribute *attr,
						const char *buf, size_t count)
{
	int val = simple_strtoul(buf, NULL, 10);
	if(val)
		kxte9_enable();
	else
		kxte9_disable();
	return count;
}

static ssize_t kxte9_tilt_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	u8 tilt;

	if(te9->resume[RES_CTRL_REG1] & TPE) {
		kxte9_i2c_read(TILT_POS_CUR, &tilt, 1);
		kxte9_dbg("kxte9_i2c_read(%x, 1)=%x\n", TILT_POS_CUR, tilt);
		return sprintf(buf, "%d\n", kxte9_resolve_dir(tilt));
	} else {
		return sprintf(buf, "%d\n", 0);
	}
}

static ssize_t kxte9_tilt_store(struct device *dev,
					struct device_attribute *attr,
						const char *buf, size_t count)
{
	int val = simple_strtoul(buf, NULL, 10);
	u8 ctrl;
	if(val)
		te9->resume[RES_CTRL_REG1] |= TPE;
	else
		te9->resume[RES_CTRL_REG1] &= (~TPE);
	ctrl = te9->resume[RES_CTRL_REG1];
	kxte9_i2c_write(CTRL_REG1, &ctrl, 1);
	kxte9_dbg("kxte9_i2c_write(%x, %x, 1)\n", CTRL_REG1, ctrl);
	return count;
}

static ssize_t kxte9_wake_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	u8 val = te9->resume[RES_CTRL_REG1] & WUFE;
	if(val)
		return sprintf(buf, "%d\n", 1);
	else
		return sprintf(buf, "%d\n", 0);
}

static ssize_t kxte9_wake_store(struct device *dev,
					struct device_attribute *attr,
						const char *buf, size_t count)
{
	int val = simple_strtoul(buf, NULL, 10);
	u8 ctrl;
	if(val)
		te9->resume[RES_CTRL_REG1] |= (WUFE | B2SE);
	else
		te9->resume[RES_CTRL_REG1] &= (~WUFE & ~B2SE);
	ctrl = te9->resume[RES_CTRL_REG1];
	kxte9_i2c_write(CTRL_REG1, &ctrl, 1);
	kxte9_dbg("kxte9_i2c_write(%x, %x, 1)\n", CTRL_REG1, ctrl);
	return count;
}

static ssize_t kxte9_selftest_store(struct device *dev,
					struct device_attribute *attr,
						const char *buf, size_t count)
{
	int val = simple_strtoul(buf, NULL, 10);
	u8 ctrl = 0x00;
	if(val)
		ctrl = 0xCA;
	kxte9_i2c_write(0x3A, &ctrl, 1);
	kxte9_dbg("kxte9_i2c_write(0x3A, %x, 1)\n", ctrl);
	return count;
}

static DEVICE_ATTR(delay, S_IRUGO|S_IWUSR, kxte9_delay_show, kxte9_delay_store);
static DEVICE_ATTR(enable, S_IRUGO|S_IWUSR, kxte9_enable_show,
						kxte9_enable_store);
static DEVICE_ATTR(tilt, S_IRUGO|S_IWUSR, kxte9_tilt_show, kxte9_tilt_store);
static DEVICE_ATTR(wake, S_IRUGO|S_IWUSR, kxte9_wake_show, kxte9_wake_store);
static DEVICE_ATTR(selftest, S_IWUSR, NULL, kxte9_selftest_store);

static struct attribute *kxte9_attributes[] = {
	&dev_attr_delay.attr,
	&dev_attr_enable.attr,
	&dev_attr_tilt.attr,
	&dev_attr_wake.attr,
	&dev_attr_selftest.attr,
	NULL
};

static struct attribute_group kxte9_attribute_group = {
	.attrs = kxte9_attributes
};
/* /sysfs */

static int kxte9_get_count(char *buf, int bufsize)
{
	const char ACC_REG_SIZE = 3;
	int err;
	/* Data bytes from hardware xL, xH, yL, yH, zL, zH */
	u8 acc_data[ACC_REG_SIZE];
	/* x,y,z hardware values */
	int xyz[3];

	if((!buf)||(bufsize<=(sizeof(xyz)*3)))
		return -1;

	err = kxte9_i2c_read(XOUT, acc_data, ACC_REG_SIZE);
	kxte9_dbg("kxte9_i2c_read(%x, %x)=%x,%x,%x, err=%d\n", XOUT, ACC_REG_SIZE,
		acc_data[0], acc_data[1], acc_data[2], err);
	if(err < 0)
		return err;

	sprintf(buf, "%d %d %d", acc_data[0], acc_data[1], acc_data[2]);

	return err;
}

static int kxte9_get_mg(char *buf, int bufsize)
{
	const char ACC_REG_SIZE = 3;
	int err;
	/* Data bytes from hardware xL, xH, yL, yH, zL, zH */
	u8 acc_data[ACC_REG_SIZE];
	/* x,y,z hardware values */
	int xyz[3], mg[3];

	if((!buf)||(bufsize<=(sizeof(mg))))
		return -1;

	err = kxte9_i2c_read(XOUT, acc_data, ACC_REG_SIZE);
	kxte9_dbg("kxte9_i2c_read(%x, %x)=%x,%x,%x, err=%x\n", XOUT, ACC_REG_SIZE,
		acc_data[0], acc_data[1], acc_data[2], err);
	if(err < 0)
		return err;

	xyz[0] = ((int)(acc_data[0]) - OFFSET) << 4;
	xyz[1] = ((int)(acc_data[1]) - OFFSET) << 4;
	xyz[2] = ((int)(acc_data[2]) - OFFSET) << 4;

	mg[0] = X_CONVERT(xyz[gconf.xyz_axis[ABS_X][0]]);
	mg[1] = Y_CONVERT(xyz[gconf.xyz_axis[ABS_Y][0]]);
	mg[2] = Z_CONVERT(xyz[gconf.xyz_axis[ABS_Z][0]]);

	sprintf(buf, "%d %d %d",mg[0], mg[1], mg[2]);

	kxte9_dbg(" [%5d] [%5d] [%5d]\n",  mg[0], mg[1], mg[2]);

	return err;
}

static void kxte9_dump_reg(void)
{
	u8  regval = 0;

	klog("**********************kxte9 reg val***************\n");
	// ct_resp
	kxte9_i2c_read(CT_RESP, &regval, 1);
	klog("ct_resp:0x%x\n", regval);
	// who_am_i
	kxte9_i2c_read(WHO_AM_I, &regval, 1);
	klog("who_am_i:0x%x\n", regval);
	// tilt_pos_cur
	kxte9_i2c_read(TILT_POS_CUR, &regval, 1);
	klog("tilt_pos_cur:0x%x\n", regval);
	// int_src_reg1
	kxte9_i2c_read(INT_STATUS_REG, &regval, 1);
	klog("int_src_reg1:0x%x\n", regval);
	// int_src_reg2
	kxte9_i2c_read(INT_SRC_REG2, &regval, 1);
	klog("int_src_reg2:0x%x\n", regval);
	// status_reg
	kxte9_i2c_read(0x18, &regval, 1);
	klog("status_reg:0x%x\n", regval);
	// int_rel
	kxte9_i2c_read(INT_REL, &regval, 1);
	klog("int_rel:0x%x\n", regval);
	// ctrl_reg1
	kxte9_i2c_read(CTRL_REG1, &regval, 1);
	klog("ctrl_reg1:0x%x\n", regval);
	// ctrl_reg2
	kxte9_i2c_read(CTRL_REG2, &regval, 1);
	klog("ctrl_reg2:0x%x\n", regval);
	// ctrl_reg3
	kxte9_i2c_read(CTRL_REG3, &regval, 1);
	klog("ctrl_reg3:0x%x\n", regval);
	// int_ctrl_reg1
	kxte9_i2c_read(INT_CTRL1, &regval, 1);
	klog("int_ctrl_reg1:0x%x\n", regval);
	// int_ctrl_reg2
	kxte9_i2c_read(0x1F, &regval, 1);
	klog("int_ctrl_reg2:0x%x\n", regval);
	// tilt_timer
	kxte9_i2c_read(TILT_TIMER, &regval, 1);
	klog("tilt_timer:0x%x\n", regval);
	// wuf_timer
	kxte9_i2c_read(WUF_TIMER, &regval, 1);
	klog("wuf_timer:0x%x\n", regval);
	// b2s_timer
	kxte9_i2c_read(B2S_TIMER, &regval, 1);
	klog("b2s_timer:0x%x\n", regval);
	// wuf_thresh
	kxte9_i2c_read(WUF_THRESH, &regval, 1);
	klog("wuf_thresh:0x%x\n", regval);
	// b2s_thresh
	kxte9_i2c_read(B2S_THRESH, &regval, 1);
	klog("b2s_thresh:0x%x\n", regval);
	// tilt_angle
	kxte9_i2c_read(0x5c, &regval, 1);
	klog("tilt_angle:0x%x\n", regval);
	// hyst_set
	kxte9_i2c_read(0x5f, &regval, 1);
	klog("hyst_set:0x%x\n", regval);
	klog("**********************************************************");
}

static int kxte9_open(struct inode *inode, struct file *file)
{
	int ret = -1;

	if(kxte9_enable() < 0)
		return ret;

	atomic_inc(&kxte9_dev_open_count);
	kxte9_dbg("opened %d times\n",\
		atomic_read(&kxte9_dev_open_count));
	kxte9_dump_reg();
	return 0;
}

static int kxte9_release(struct inode *inode, struct file *file)
{
	int open_count;

	atomic_dec(&kxte9_dev_open_count);
	open_count = (int)atomic_read(&kxte9_dev_open_count);

	if(open_count == 0)
		kxte9_disable();

	kxte9_dbg("opened %d times\n",\
			 atomic_read(&kxte9_dev_open_count));
	return 0;
}

static int kxte9_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
	   unsigned long arg)
{
	char buffer[IOCTL_BUFFER_SIZE];
	void __user *data;
	u8 reg_buffer = 0x00;
	const u8 set = 0xFF, unset = 0x00;
	int retval=0, val_int=0;
	short val_short=0;

	switch (cmd) {
		case KXTE9_IOCTL_GET_COUNT:
			data = (void __user *) arg;
			if(data == NULL){
				retval = -EFAULT;
				goto err_out;
			}
			retval = kxte9_get_count(buffer, sizeof(buffer));
			if(retval < 0)
				goto err_out;

			if(copy_to_user(data, buffer, sizeof(buffer))) {
				retval = -EFAULT;
				goto err_out;
			}
			break;

		case KXTE9_IOCTL_GET_MG:
			data = (void __user *) arg;
			if(data == NULL){
				retval = -EFAULT;
				goto err_out;
			}
			retval = kxte9_get_mg(buffer, sizeof(buffer));
			if(retval < 0)
				goto err_out;

			if(copy_to_user(data, buffer, sizeof(buffer))) {
				retval = -EFAULT;
				goto err_out;
			}
			break;

		case KXTE9_IOCTL_ENABLE_OUTPUT:
			retval = kxte9_set_pc1_on();
			if(retval < 0)
				goto err_out;
			break;

		case KXTE9_IOCTL_DISABLE_OUTPUT:
			retval = kxte9_set_pc1_off();
			if(retval < 0)
				goto err_out;
			break;

		case KXTE9_IOCTL_GET_ENABLE:
			data = (void __user *) arg;
			if(data == NULL){
				retval = -EFAULT;
				goto err_out;
			}
			retval = kxte9_get_bits(CTRL_REG1, &reg_buffer, PC1_ON);
			if(retval < 0)
				goto err_out;

			val_short = (short)reg_buffer;

			if(copy_to_user(data, &val_short, sizeof(val_short))) {
				retval = -EFAULT;
				goto err_out;
			}
			break;

		case KXTE9_IOCTL_RESET:
			retval = kxte9_set_bits(RES_CTRL_REG3, CTRL_REG3, set, SRST);
			if(retval < 0)
				goto err_out;
			break;

		case KXTE9_IOCTL_UPDATE_ODR:
			data = (void __user *) arg;
			if(data == NULL){
				retval = -EFAULT;
				goto err_out;
			}
			if(copy_from_user(&val_int, data, sizeof(val_int))) {
				retval = -EFAULT;
				goto err_out;
			}

			mutex_lock(&te9->lock);
			te9->pdata->poll_interval = max(val_int, te9->pdata->min_interval);
			mutex_unlock(&te9->lock);

			retval = kxte9_update_odr(te9->pdata->poll_interval);
			if(retval < 0)
				goto err_out;
			break;

		case KXTE9_IOCTL_ENABLE_CTC:
			retval = kxte9_set_bits(RES_CTRL_REG3, CTRL_REG3, set, CTC);
			if(retval < 0)
				goto err_out;
			break;

		case KXTE9_IOCTL_DISABLE_CTC:
			retval = kxte9_set_bits(RES_CTRL_REG3, CTRL_REG3, unset, CTC);
			if(retval < 0)
				goto err_out;
			break;

		case KXTE9_IOCTL_GET_CT_RESP:
			data = (void __user *) arg;
			if(data == NULL){
				retval = -EFAULT;
				goto err_out;
			}
			retval = kxte9_get_byte(CT_RESP, &reg_buffer);
			if(retval < 0)
				goto err_out;

			buffer[0] = (char)reg_buffer;
			if(copy_to_user(data, buffer, sizeof(buffer))) {
				retval = -EFAULT;
				goto err_out;
			}
			break;

		default:
			retval = -ENOIOCTLCMD;
			break;
	}

err_out:
	return retval;
}

static struct file_operations kxte9_fops = {
	.owner = THIS_MODULE,
	.open = kxte9_open,
	.release = kxte9_release,
	.ioctl = kxte9_ioctl,
};

static struct miscdevice kxte9_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = NAME_DEV,
	.fops = &kxte9_fops,
};

static int kxte9_probe(void)
{
	int err = -1;

	te9 = kzalloc(sizeof(*te9), GFP_KERNEL);
	if(te9 == NULL) {
		errlog("failed to allocate memory for module data\n");
		err = -ENOMEM;
		goto err0;
	}
	te9->pdata = &kxte9_pdata;
	mutex_init(&te9->lock);
	mutex_lock(&te9->lock);

	x_count = kzalloc(gconf.avg_count, GFP_KERNEL);
	y_count = kzalloc(gconf.avg_count, GFP_KERNEL);
	z_count = kzalloc(gconf.avg_count, GFP_KERNEL);

	if ((1000000/gconf.samp)%1000)
		te9->pdata->poll_interval = 1000/gconf.samp + 1;
	else
		te9->pdata->poll_interval = 1000/gconf.samp;
	printk(KERN_INFO "G-Sensor Time Interval: %d ms\n", te9->pdata->poll_interval);

	android_gsensor_kobj = kobject_create_and_add("kxte9_gsensor", NULL);
	if (android_gsensor_kobj == NULL) {
		errlog("gsensor_sysfs_init:subsystem_register failed\n");
		err = -ENOMEM;
		goto err0;
	}

	err = sysfs_create_group(android_gsensor_kobj, &kxte9_attribute_group);
	if(err)
		goto err1;

	if(te9->pdata->init) {
		err = te9->pdata->init();
		if(err < 0)
			goto err2;
	}

	memset(te9->resume, 0, ARRAY_SIZE(te9->resume));
	te9->resume[RES_CTRL_REG1]	= te9->pdata->ctrl_reg1_init;
	te9->resume[RES_CTRL_REG3]	= te9->pdata->engine_odr_init;
	te9->resume[RES_INT_CTRL1]	= te9->pdata->int_ctrl_init;
	te9->resume[RES_TILT_TIMER]	= te9->pdata->tilt_timer_init;
	te9->resume[RES_WUF_TIMER]	= te9->pdata->wuf_timer_init;
	te9->resume[RES_B2S_TIMER]	= te9->pdata->b2s_timer_init;
	te9->resume[RES_WUF_THRESH]	= te9->pdata->wuf_thresh_init;
	te9->resume[RES_B2S_THRESH]	= te9->pdata->b2s_thresh_init;
	te9->hw_initialized = 0;
	te9->i2c_xfer_complete = 1;

#ifdef INT_MODE
	// init gpio
	SET_GPIO_GSENSOR_INT();
	INIT_WORK(&te9->irq_work, kxte9_irq_work_func);
#endif

	err = kxte9_device_power_on();
	if(err < 0)
		goto err3;
	atomic_set(&te9->enabled, 1);

	err = kxte9_verify();
	if(err < 0) {
		errlog("kxte9_verify failed\n");
		goto err4;
	}

	err = kxte9_update_odr(te9->pdata->poll_interval);
	if(err < 0) {
		errlog("update_odr failed\n");
		goto err4;
	}

	err = kxte9_input_init();
	if(err < 0)
		goto err4;

	err = misc_register(&kxte9_device);
	if(err) {
		errlog("misc. device failed to register.\n");
		goto err5;
	}

#ifdef CONFIG_HAS_EARLYSUSPEND
        te9->earlysuspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
        te9->earlysuspend.suspend = kxte9_early_suspend;
        te9->earlysuspend.resume = kxte9_late_resume;
        register_early_suspend(&te9->earlysuspend);
#endif

	atomic_set(&te9->enabled, 0);

#ifdef INT_MODE
	err = request_irq(gconf.irq, kxte9_isr,
			IRQF_TRIGGER_RISING | IRQF_DISABLED, "kxte9_irq", te9);
	if(err < 0) {
		pr_err("%s: request irq failed: %d\n", __func__, err);
		goto err6;
	}

#ifdef WM3445_A0
	// enable int
	ENABLE_SENSOR_INT(1);
#endif

	// enable kxte9
	/*err = kxte9_enable();
	if (err < 0) {
		errlog("kxte9_enable failed.\n");
		goto err6;
	}*/
#endif

	mutex_unlock(&te9->lock);
	//kxte9_dump_reg();

	return 0;

#ifdef INT_MODE
err6:
	misc_deregister(&kxte9_device);
#endif
err5:
	kxte9_input_cleanup();
err4:
	kxte9_device_power_off();
err3:
	if(te9->pdata->exit)
		te9->pdata->exit();
err2:
	//kfree(te9->pdata);
	sysfs_remove_group(android_gsensor_kobj, &kxte9_attribute_group);
err1:
	kobject_del(android_gsensor_kobj);
	mutex_unlock(&te9->lock);
	kfree(te9);
err0:
	return err;
}

static int kxte9_remove(void)
{
	cancel_delayed_work_sync(&te9->input_work);
#ifdef INT_MODE
	free_irq(gconf.irq, te9);
#endif
	kxte9_input_cleanup();
	misc_deregister(&kxte9_device);
	kxte9_device_power_off();
	if(te9->pdata->exit)
		te9->pdata->exit();
	kobject_del(android_gsensor_kobj);
	sysfs_remove_group(android_gsensor_kobj, &kxte9_attribute_group);
	kfree(te9);

	return 0;
}

#ifdef CONFIG_PM
#ifdef CONFIG_HAS_EARLYSUSPEND
static void kxte9_early_suspend(struct early_suspend *h)
{
	kxte9_dbg("start\n");
	te9->suspend = 1;
	kxte9_disable();
	kxte9_dbg("exit\n");
}

static void kxte9_late_resume(struct early_suspend *h)
{
	kxte9_dbg("start\n");
	te9->suspend = 0;
	kxte9_enable();
	kxte9_dbg("exit\n");
}
#endif

static int kxte9_resume(struct platform_device *pdev)
{
	return 0;
}

static int kxte9_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}
#endif

static void kxte9_platform_release(struct device *device)
{
	kxte9_dbg("Do nothing!!!\n");
    return;
}

static struct platform_device kxte9_plt_device = {
    .name = "kxte9",
    .id = 0,
    .dev = {
    	.release = kxte9_platform_release,
    },
};

static struct platform_driver kxte9_plt_driver = {
	.probe = NULL, //kxte9_probe,
	.remove = NULL, //kxte9_remove,
	.suspend	= kxte9_suspend,
	.resume = kxte9_resume,
	.driver = {
		.name = "kxte9",
	},
};

/*
 * Get the configure of sensor from u-boot.
 * Return: 1--success, 0--error.
 */
static int get_axisset(void)
{
	char varbuf[64];
	int n;
	int varlen;

	memset(varbuf, 0, sizeof(varbuf));
	varlen = sizeof(varbuf);
	if (wmt_getsyspara("wmt.io.gsensor", varbuf, &varlen)) {
		printk(KERN_DEBUG "KXTE9: wmt.io.gsensor not defined!\n");
		return 0;
	} else {
		n = sscanf(varbuf, "%d:%d:%d:%d:%d:%d:%d:%d:%d:%d",
				&gconf.op,
				&gconf.samp,
				&(gconf.xyz_axis[0][0]),
				&(gconf.xyz_axis[0][1]),
				&(gconf.xyz_axis[1][0]),
				&(gconf.xyz_axis[1][1]),
				&(gconf.xyz_axis[2][0]),
				&(gconf.xyz_axis[2][1]),
				&gconf.avg_count,
				&gconf.kxte9_8bit);
	
		if (n != 10) {
			kxte9_dbg(KERN_ERR "KXTE9: wmt.io.gsensor format is incorrect!\n");
			return 0;
		}
	}
	if (gconf.op != 1)
		return 0;
	printk(KERN_INFO "KXTE9 G-Sensor driver init\n");
	kxte9_dbg("AVG Count: %d, KXTE9 8bit: %d\n",
		gconf.avg_count,
		gconf.kxte9_8bit);
	if (gconf.samp > 100) {
		printk(KERN_ERR "Sample Rate can't be greater than 100 !\n");
		gconf.samp = 100;
	}
	if (gconf.avg_count == 0)
		gconf.avg_count = 1;
	if (gconf.kxte9_8bit > 1)
		gconf.kxte9_8bit = 1;
	kxte9_dbg("wmt.io.gsensor = %d:%d:%d:%d:%d:%d:%d:%d:%d:%d\n",
		gconf.op,
		gconf.samp,
		gconf.xyz_axis[0][0],
		gconf.xyz_axis[0][1],
		gconf.xyz_axis[1][0],
		gconf.xyz_axis[1][1],
		gconf.xyz_axis[2][0],
		gconf.xyz_axis[2][1],
		gconf.avg_count,
		gconf.kxte9_8bit);
	return 1;
}

static int get_gpioset(void)
{
	char varbuf[96];
	int n;
	int varlen;

	memset(varbuf, 0, sizeof(varbuf));
	varlen = sizeof(varbuf);
	if (wmt_getsyspara("wmt.gpt.gsensor", varbuf, &varlen)) {
		printk(KERN_DEBUG "Can't get gsensor's gpio config from u-boot! -> Use default config\n");
		return 0;
	} else {
		n = sscanf(varbuf, "%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x",
				&gconf.name,
				&gconf.bmp,
				&(gconf.ctraddr),
				&(gconf.ocaddr),
				&(gconf.idaddr),
				&(gconf.peaddr),
				&(gconf.pcaddr),
				&(gconf.itbmp),
				&(gconf.itaddr),
				&(gconf.isbmp),
				&(gconf.isaddr),
				&(gconf.irq));
		if (n != 12) {
			printk(KERN_ERR "wmt.gpt.gsensor format is incorrect in u-boot!!!\n");
			return 0;
		}

		gconf.ctraddr += WMT_MMAP_OFFSET;
		gconf.ocaddr += WMT_MMAP_OFFSET;
		gconf.idaddr += WMT_MMAP_OFFSET;
		gconf.peaddr += WMT_MMAP_OFFSET;
		gconf.pcaddr += WMT_MMAP_OFFSET;
		gconf.itaddr += WMT_MMAP_OFFSET;
		gconf.isaddr += WMT_MMAP_OFFSET;
	}
	return 1;
}

static int __init kxte9_init(void)
{
	int ret = 0;

	// parsing u-boot arg
	ret = get_axisset(); // get gsensor config from u-boot
	if (!ret)
		return -EINVAL;

	get_gpioset();

	// initail
	if ((ret = kxte9_probe()) != 0) {
		errlog(" Error for kxte9_probe !!!\n");
		return ret;
	}
	atomic_set(&kxte9_dev_open_count, 0);

	if ((ret = platform_device_register(&kxte9_plt_device)) != 0) {
		errlog("Can't register kxte9_plt_device platform devcie!!!\n");
		return ret;
	}
	if ((ret = platform_driver_register(&kxte9_plt_driver)) != 0) {
		errlog("Can't register kxte9_plt_driver platform driver!!!\n");
		return ret;
	}
	klog("gsensor_kxte9 driver is loaded successfully!\n");
	return ret;
}

static void __exit kxte9_exit(void)
{
    platform_driver_unregister(&kxte9_plt_driver);
    platform_device_unregister(&kxte9_plt_device);
    atomic_set(&kxte9_dev_open_count, 0);
    kxte9_remove();
}

module_init(kxte9_init);
module_exit(kxte9_exit);

MODULE_DESCRIPTION("KXTE9 accelerometer driver");
MODULE_AUTHOR("Kuching Tan <kuchingtan@kionix.com>");
MODULE_LICENSE("GPL");
MODULE_VERSION(VERSION_DEV);
