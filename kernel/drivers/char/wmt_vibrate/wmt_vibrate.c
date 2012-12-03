/*++

	Some descriptions of such software. Copyright (c) 2010  WonderMedia Technologies, Inc.

	This program is free software: you can redistribute it and/or modify it under the
	terms of the GNU General Public License as published by the Free Software Foundation,
	either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT
	ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
	PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	You should have received a copy of the GNU General Public License along with
	this program.  If not, see <http://www.gnu.org/licenses/>.

	WonderMedia Technologies, Inc.
	2010-2-26, HangYan, ShenZhen
--*/

#include <linux/module.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/stat.h>
#include <linux/workqueue.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/mach-types.h>
#include <mach/hardware.h>

#include "wmt_vibrate.h"
#include "timed_output.h"

/* Constant Macro */
#define WMT_VIBRATE_DEVICE_NAME "wmt_vibrate"
#define WMT_VIBRATE_MAJOR 37
//#define WMT_VIBRATE_MINOR 0
#define WMT_VIBRATE_DELAY 250 //ms

/*************************Function declare************************************/
static void enable_timedoutput(struct timed_output_dev *sdev, int timeout);
static int gettime_timedoutput(struct timed_output_dev *sdev);

//#define DEBUG
#ifdef DEBUG
#define vibrate_dbg(fmt, args...) printk(KERN_ALERT "[%s]: " fmt, __FUNCTION__ , ## args)
#else
#define vibrate_dbg(fmt, args...)
#endif

static struct timer_list vibrate_timer;

/************************Data type and local variable****************/
struct vibrate_device {
	int vibratile; // 0: shouldn't vibrate, 1:should vibrate
	struct mutex mlock;
	struct class* dev_class;
	struct device *device;
	unsigned int vibtimeout;
	struct work_struct	work;
	struct timed_output_dev vibtimedev;
	/* setting attributy */
	int name;
	int active;
	unsigned int bmp;
	unsigned int ctraddr;
	unsigned int ocaddr;
	unsigned int odaddr;
	unsigned int bitx;
};

static struct vibrate_device l_vibratedev = {
	.vibratile = 0,
	.dev_class = NULL,
	.device = NULL,
	.vibtimeout = 0,
	.vibtimedev = {
		.name = "vibrator",
		.enable = enable_timedoutput,
		.get_time = gettime_timedoutput,
	},
	/* default reg setting: Use GPIO1 to control motor */
	.name = 1,
	.active = 1,
	.bmp = 0x2,
	.ctraddr = GPIO_BASE_ADDR + 0x40,
	.ocaddr = GPIO_BASE_ADDR + 0x80,
	.odaddr = GPIO_BASE_ADDR + 0xc0,
};


/************************Function implement*******************/
static void wmt_disable_vibrator(unsigned long unused)
{
	if (l_vibratedev.active == 0)
		REG32_VAL(l_vibratedev.odaddr) |= l_vibratedev.bmp;
	else
		REG32_VAL(l_vibratedev.odaddr) &= ~l_vibratedev.bmp;
}

static void enable_timedoutput(struct timed_output_dev *sdev, int timeout)
{
	/*
	mutex_lock(&l_vibratedev.mlock);
	l_vibratedev.vibtimeout = timeout;
	mutex_unlock(&l_vibratedev.mlock);
	//wakeup work to vibrate
	if (l_vibratedev.vibtimeout)
		schedule_work(&l_vibratedev.work);
	*/
	del_timer_sync(&vibrate_timer);

	if (l_vibratedev.active == 0)
		REG32_VAL(l_vibratedev.odaddr) &= ~l_vibratedev.bmp; /* low active */
	else
		REG32_VAL(l_vibratedev.odaddr) |= l_vibratedev.bmp; /* high active */

	mod_timer(&vibrate_timer,  jiffies +  msecs_to_jiffies(timeout));
}

static int gettime_timedoutput(struct timed_output_dev *sdev)
{
	return l_vibratedev.vibtimeout;
}

void wmt_do_vibrate(void)
{
	unsigned int vibtime = 0;

	REG32_VAL(l_vibratedev.ctraddr) |= l_vibratedev.bmp;
	REG32_VAL(l_vibratedev.ocaddr) |= l_vibratedev.bmp;

	if (l_vibratedev.active == 0)
		REG32_VAL(l_vibratedev.odaddr) &= ~l_vibratedev.bmp; /* low active */
	else
		REG32_VAL(l_vibratedev.odaddr) |= l_vibratedev.bmp; /* high active */
	// delay
	//if (l_vibratedev.vibtimeout < WMT_VIBRATE_DELAY)
	//{
	//	vibtime = WMT_VIBRATE_DELAY;
	//} else
	//{
		vibtime = l_vibratedev.vibtimeout;
	//}
	msleep(vibtime);
	if (l_vibratedev.active == 0)
		REG32_VAL(l_vibratedev.odaddr) |= l_vibratedev.bmp;
	else
		REG32_VAL(l_vibratedev.odaddr) &= ~l_vibratedev.bmp;
}


// return 0: unvibratile, 1: vibratile
int wmt_is_vibratile(void)
{
	return l_vibratedev.vibratile;
}

static int wmt_vibrate_release(struct inode *inode, struct file *file)
{
	return 0;
}

static int wmt_vibrate_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int wmt_vibrate_ioctl(struct inode *ip, struct file *fp,
			     unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	void __user *argp = (void __user *)arg;
	u8 val8;

	mutex_lock(&l_vibratedev.mlock);
	switch (cmd) {
	case WMT_VIBRATE_ENABLE:
		if (copy_from_user(&val8, argp, sizeof(val8))) {
			ret = -EFAULT;
			break;
		}
		l_vibratedev.vibratile= (val8 != 0)? 1:0;
		/*
		if (l_vibratedev.vibratile != 0){
			wmt_do_vibrate();
		}
		*/
		break;
	default:
		ret = -EINVAL;
	}
	mutex_unlock(&l_vibratedev.mlock);
	return ret;
}

static const struct file_operations vibrate_fops = {
	.owner		= THIS_MODULE,
	.open		= wmt_vibrate_open,
	.release	= wmt_vibrate_release,
	.ioctl		= wmt_vibrate_ioctl,
};

/*
static int __devinit wmt_vibrate_probe(struct platform_device *dev)
{
}

static int __devexit wmt_vibrate_remove(struct platform_device *dev)
{
	return 0;
}

static void wmt_vibrate_shutdown(struct platform_device *dev)
{
}

*/

static void vibrate_gpio_init(void)
{
	REG32_VAL(l_vibratedev.ctraddr) |= l_vibratedev.bmp;
	REG32_VAL(l_vibratedev.ocaddr) |= l_vibratedev.bmp;
	if (l_vibratedev.active == 0)
		REG32_VAL(l_vibratedev.odaddr) |= l_vibratedev.bmp;
	else
		REG32_VAL(l_vibratedev.odaddr) &= ~l_vibratedev.bmp;
}

#ifdef CONFIG_PM
static int wmt_vibrate_suspend(struct platform_device *dev, pm_message_t state)
{
	return 0;
}

static int wmt_vibrate_resume(struct platform_device *dev)
{
	vibrate_gpio_init();
	return 0;
}
#else
#define wmt_vibrate_suspend	NULL
#define wmt_vibrate_resume	NULL
#endif


static struct platform_driver vibrate_driver = {
	.driver		= {
		.name	= WMT_VIBRATE_DEVICE_NAME,
		.owner	= THIS_MODULE,
	},
	//.probe		= wmt_vibrate_probe,
	//.remove		= __devexit_p(wmt_vibrate_remove),
	//.shutdown	= wmt_vibrate_shutdown,
	.suspend	= wmt_vibrate_suspend,
	.resume		= wmt_vibrate_resume,
};

static struct platform_device *vibrate_platform_device;

/*
static ssize_t show_classname(struct class *class, char *buf)
{
	return sprintf(buf, "%s\n", WMT_VIBRATE_DEVICE_NAME);
}

static CLASS_ATTR(vibrate, 0777, show_classname, NULL);
*/

//static struct class_attribute cls_attrs = __ATTR(WMT_VIBRATE_DEVICE_NAME, 0777, NULL, NULL);

// dev_attr_wmt_vibrate

/*
static ssize_t show_vibtime(struct device *dev, struct device_attribute *attr,
			  char *buf)
{

	return sprintf(buf, "%dms\n", l_vibratedev.vibtimeout);
}

static ssize_t set_vibtime(struct device *dev, struct device_attribute *attr,
			  const char *buf, size_t count)
{
	unsigned long val;

	if (strict_strtoul(buf, 10, &val) != 0)
	{
		return count;
	}
	return count;
}

static DEVICE_ATTR(vibtimeout, 0666, show_devname, NULL);
*/

static void vib_work_handler(struct work_struct *work)
{
	wmt_do_vibrate();
}

extern int wmt_getsyspara(char *varname, unsigned char *varval, int *varlen);
static int get_vibratorset(void* param)
{
	char buf[128];
	unsigned int ubootval[6];
	int ret = 0;
	int varlen = 127;

	/* read the paraqm from */
	memset(buf ,0, sizeof(buf));
	memset(ubootval, 0, sizeof(ubootval));
	if (wmt_getsyspara("wmt.gpo.vibrator", buf, &varlen)) {
		printk(KERN_WARNING "wmt.gpo.vibrator isn't set in u-boot env! -> Use default\n");
		return -1;
	}
	ret = sscanf(buf, "%x:%x:%x:%x:%x:%x",
				       &ubootval[0],
				       &ubootval[1],
				       &ubootval[2],
				       &ubootval[3],
				       &ubootval[4],
				       &ubootval[5]);
	if (ret != 6) {
		printk(KERN_ERR "wmt.gpo.vibrator format is incorrect in u-boot\n");
		return -2;
	}
	vibrate_dbg("wmt.gpo.vibrator = %x:%x:%x:%x:%x:%x\n",
		ubootval[0],
		ubootval[1],
		ubootval[2],
		ubootval[3],
		ubootval[4],
		ubootval[5]);
	l_vibratedev.name = ubootval[0];
	l_vibratedev.active = ubootval[1];
	l_vibratedev.bmp = ubootval[2];
	l_vibratedev.ctraddr = ubootval[3] + WMT_MMAP_OFFSET;
	l_vibratedev.ocaddr = ubootval[4] + WMT_MMAP_OFFSET;
	l_vibratedev.odaddr = ubootval[5] + WMT_MMAP_OFFSET;
	return 0;
}

static int __init wmt_vibrate_init(void)
{
	int error = 0;

	/* get vibrator setting */
	get_vibratorset(&l_vibratedev);

	vibrate_gpio_init();

	/* other initial */
	mutex_init(&l_vibratedev.mlock);
	//INIT_WORK(&l_vibratedev.work, vib_work_handler);
	setup_timer(&vibrate_timer, wmt_disable_vibrator, 0);

	/* register char device */
	if (register_chrdev (WMT_VIBRATE_MAJOR, WMT_VIBRATE_DEVICE_NAME, &vibrate_fops)) {
		printk (KERN_ERR "wmt vibrate: unable to get major %d\n", WMT_VIBRATE_MAJOR);
		error = -EIO;
		goto initend;
	}
	/* (mknod /dev/wmt_vibrate c 37 0) */
	l_vibratedev.dev_class = class_create(THIS_MODULE, WMT_VIBRATE_DEVICE_NAME);
	if (IS_ERR(l_vibratedev.dev_class))
	{
		error = PTR_ERR(l_vibratedev.dev_class);
		printk(KERN_ERR "Can't class_create vibrate device !!\n");
		goto initend1;
	}
	/*
	if (class_create_file(l_vibratedev.dev_class, &class_attr_vibrate) < 0)
	{
		printk(KERN_ERR "Can't add class attr !\n");
		return -1;
	}
	*/

	l_vibratedev.device = device_create(l_vibratedev.dev_class, NULL, MKDEV(WMT_VIBRATE_MAJOR, 0), NULL, WMT_VIBRATE_DEVICE_NAME);
    if (IS_ERR(l_vibratedev.device))
    {
    	error = PTR_ERR(l_vibratedev.device);
    	printk(KERN_ERR "Failed to create device %s !!!",WMT_VIBRATE_DEVICE_NAME);
    	goto initend2;
    }
    /*
    if (device_create_file(l_vibratedev.device, &dev_attr_vibrate))
    {
    	printk(KERN_ERR "Can't add device attr!!!\n");
    	return -1;
    }
    */

	/* create '/sys/class/timed_output/vibrator/enable' */
	if ((error = timed_output_dev_register(&l_vibratedev.vibtimedev)) != 0)
	{
		goto initend3;
	}

	error = platform_driver_register(&vibrate_driver);
	if (error)
		goto exit_timed_unregsiter;

	vibrate_platform_device = platform_device_alloc(WMT_VIBRATE_DEVICE_NAME, -1);
	if (!vibrate_platform_device) {
		error = -ENOMEM;
		printk(KERN_ERR "Can't alloc vibrate_platform_device!!!\n");
		goto initend4;
	}

	error = platform_device_add(vibrate_platform_device);
	if (error)
		goto initend5;

	printk(KERN_ALERT"WMT vibrater driver load successfully!\n");
	return 0;

initend5:
	platform_device_put(vibrate_platform_device);
initend4:
	platform_driver_unregister(&vibrate_driver);
exit_timed_unregsiter:
	timed_output_dev_unregister(&l_vibratedev.vibtimedev);
initend3:
	device_destroy(l_vibratedev.dev_class, MKDEV(WMT_VIBRATE_MAJOR, 0));
initend2:
	class_destroy(l_vibratedev.dev_class);
initend1:
	unregister_chrdev(WMT_VIBRATE_MAJOR, WMT_VIBRATE_DEVICE_NAME);
initend:
	return error;
}

static void __exit wmt_vibrate_exit(void)
{
	platform_device_unregister(vibrate_platform_device);
	platform_driver_unregister(&vibrate_driver);
	timed_output_dev_unregister(&l_vibratedev.vibtimedev);
	device_destroy(l_vibratedev.dev_class, MKDEV(WMT_VIBRATE_MAJOR, 0));
    class_destroy(l_vibratedev.dev_class);
	unregister_chrdev(WMT_VIBRATE_MAJOR, WMT_VIBRATE_DEVICE_NAME);

	printk(KERN_ALERT "WMT vibrate driver is removed.\n");
}

module_init(wmt_vibrate_init);
module_exit(wmt_vibrate_exit);


EXPORT_SYMBOL(wmt_do_vibrate);
EXPORT_SYMBOL(wmt_is_vibratile);

MODULE_DESCRIPTION("Vibrate Device driver");
MODULE_LICENSE("GPL");
