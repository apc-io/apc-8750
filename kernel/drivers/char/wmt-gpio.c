/*++
linux/drivers/char/wmt-gpio.c

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
#include <linux/proc_fs.h>
#include <linux/jiffies.h>
#include <linux/major.h>
#include <linux/delay.h>
#include <linux/sysctl.h>
#include <linux/sysctl.h>

#define WMT_GPOI_DEV_C
#include "wmt-gpio-dev.h"

#define GPIO_DEVICE_NAME "wmtgpio"
static int gpio_dev_major;
static struct cdev *gpio_cdev = NULL;
struct class *class_gpio_dev;
struct device *gpio_device;
struct gpio_list *g_gpio_list = NULL;
struct gpio_list *g_list_now = NULL;


#define GPIO_PHY_BASE_ADDR (GPIO_BASE_ADDR-WMT_MMAP_OFFSET)

extern int wmt_getsyspara(char *varname, unsigned char *varval, int *varlen);
DECLARE_MUTEX(gpio_dev_lock);

static int gpio_dev_open
(
	struct inode *inode,	/*!<; //[IN] a pointer point to struct inode */
	struct file *filp		/*!<; //[IN] a pointer point to struct file  */
)
{
	//printk("gpio_dev_open\n"); //fan
	return 0;
}


static int gpio_dev_release
(
	struct inode *inode,	/*!<; //[IN] a pointer point to struct inode */
	struct file *filp		/*!<; //[IN] a pointer point to struct file  */
)
{
	return 0;
}

static int init_gpio_list(struct gpio_list *list)
{
	list->addr = 0;
	list->bitmap = 0;
	list->regval = 0;
	list->prev = NULL;
	list->next = NULL;

	return 0;	
}
/*
static int tellme_gpio_list(void)
{
	struct gpio_list *list;
	int i = 0;

	if (g_gpio_list == NULL) {
		printk("there is no gpio list\n");
		return 0;
	}
	list = g_gpio_list;
	while(1) {
		printk("list%d : \n",i);
		printk("address = 0x%x  , bitmap = 0x%x\n", list->addr, list->bitmap);
		list = list->next;
		if (list == NULL)
			break;
	}
	return 0;
}
*/
static int free_gpio_list(void)
{
	struct gpio_list *prev, *now;

	prev = g_gpio_list->prev;
	now = g_gpio_list;
	while(1) {
		kfree(now);
		now = prev;
		if (now == NULL)
			break;
		prev = now->prev;
	}
	g_gpio_list = NULL;
	g_list_now = NULL;

	return 0;
}

static struct gpio_list * search_gpio_list(struct wmt_reg_op_t *reg)
{
	struct gpio_list *list;

	if (g_gpio_list == NULL) {
		printk("there is no gpio list\n");
		return NULL;
	}
	list = g_gpio_list;
	while(1) {
		if (list->addr == reg->addr)
			return list;
		if (list->next == NULL)
			break;
		list = list->next;
	};
	return NULL;
}

static int add_gpio_list(struct gpio_list *new, struct gpio_list *head)
{
	new->prev = head;
	head->next = new;
	g_list_now = new;

	return 0;
}

static int write_gpio_list(struct wmt_reg_op_t *reg)
{
	struct gpio_list *list;

	if (g_gpio_list == NULL) {
		g_gpio_list = (struct gpio_list *) kmalloc(sizeof(struct gpio_list), GFP_KERNEL);
		if (g_gpio_list == NULL) {
			printk("error : alloc gpio list failed\n");
			return -1;
		}
		init_gpio_list(g_gpio_list);
		list = g_gpio_list;
		g_list_now = g_gpio_list;
	} else {
		list = search_gpio_list(reg);
		if (list == NULL) {
			list = (struct gpio_list *) kmalloc(sizeof(struct gpio_list), GFP_KERNEL);
			if (list == NULL) {
				printk("error : alloc gpio list failed\n");
				return -1;
			}
			init_gpio_list(list);
			add_gpio_list(list, g_list_now);
		}
	}

	if ((reg->addr&0xffff0000) == GPIO_PHY_BASE_ADDR)
		reg->addr = (reg->addr+WMT_MMAP_OFFSET);
	list->addr = reg->addr;
	list->bitmap |= reg->bitmap;

	return 0;
}

static int write_wmt_reg(struct wmt_reg_op_t *regop)
{
	unsigned int tmp;

	if ((regop->addr == 0) || (regop->bitmap == 0)) {
		printk("gpio device error :  address = %x , value = 0x%x\n",regop->addr,regop->bitmap);
		return -1;
	}
	if (write_gpio_list(regop))
		return -1;
/*
	printk("fan w address = 0x%x\n",regop->addr);
	printk("fan w reg->bitmap = 0x%x\n",regop->bitmap);
	printk("fan w reg->regval = 0x%x\n",regop->regval);
*/
	regop->regval &= regop->bitmap;
	tmp = REG32_VAL(regop->addr);
	tmp &= ~(regop->bitmap);
	tmp |= regop->regval;
	REG32_VAL(regop->addr) = tmp;

	return 0;
}

static int read_wmt_reg(struct wmt_reg_op_t *regop)
{
	unsigned int tmp;

	if ((regop->addr == 0) || (regop->bitmap == 0)) {
		printk("gpio device error :  address = %x , value = 0x%x\n",regop->addr,regop->bitmap);
		return -1;
	}
/*
	printk("fan r address = 0x%x\n",regop->addr);
	printk("fan r reg->bitmap = %d\n",regop->bitmap);
	printk("fan r reg->regval = %d\n",regop->regval);
*/

	if ((regop->addr&0xffff0000) == GPIO_PHY_BASE_ADDR)
		regop->addr = (regop->addr+WMT_MMAP_OFFSET);

	tmp = REG32_VAL(regop->addr);
	tmp &= regop->bitmap;
	regop->regval = tmp;

	return 0;
}

//fan
extern void wmt_drop_pagecache(void);
extern void wmt_drop_slab(void);

static int gpio_dev_ioctl
(
	struct inode *inode,	/*!<; //[IN] a pointer point to struct inode */
	struct file *filp,		/*!<; //[IN] a pointer point to struct file  */
	unsigned int cmd,		/*!<; // please add parameters description her*/
	unsigned long arg		/*!<; // please add parameters description her*/
)
{
	int err = 0;
	int retval = 0;
	struct gpio_cfg_t cfg;
	struct wmt_reg_op_t regop;
	unsigned int drop_caches;

	/* check type and number, if fail return ENOTTY */
	if( _IOC_TYPE(cmd) != GPIO_IOC_MAGIC ) return -ENOTTY;
	if( _IOC_NR(cmd) >= GPIO_IOC_MAXNR ) return -ENOTTY;

	/* check argument area */
	if( _IOC_DIR(cmd) & _IOC_READ ) {
		err = !access_ok( VERIFY_WRITE, (void __user *) arg, _IOC_SIZE(cmd));
		if (err)
			printk("VERIFY_WRITE failed");
	}
	else if ( _IOC_DIR(cmd) & _IOC_WRITE ) {
		err = !access_ok( VERIFY_READ, (void __user *) arg, _IOC_SIZE(cmd));
		if (err)
			printk("VERIFY_READ failed");
	}
	
	if( err ) return -EFAULT;

	down(&gpio_dev_lock);

	switch (cmd) {
	case GPIOCFG:
		if (copy_from_user(&cfg, (void *) arg, sizeof(struct gpio_cfg_t)))
			return -EFAULT;
		if (write_wmt_reg(&cfg.ctl)) return -EFAULT;
		if (write_wmt_reg(&cfg.oc)) return -EFAULT;
		break;
	case GPIOWREG:
		if (copy_from_user(&regop, (void *) arg, sizeof(struct wmt_reg_op_t)))
			return -EFAULT;
		if (write_wmt_reg(&regop)) return -EFAULT;
		break;
	case GPIORREG:
		if (copy_from_user(&regop, (void *) arg, sizeof(struct wmt_reg_op_t)))
			return -EFAULT;

		if (read_wmt_reg(&regop)) return -EFAULT;

		if (copy_to_user((void *)arg, (void *) &regop, sizeof(struct wmt_reg_op_t)))
			return -EFAULT;
		break;
	case FREESYSCACHES:
		drop_caches = (unsigned int)arg;
		//printk("fan , FREESYSCACHES , drop_caches = %d\n",drop_caches); //fan
		if (drop_caches & 1)
			wmt_drop_pagecache();
		if (drop_caches & 2)
			wmt_drop_slab();
	break;
	default:
		retval = -ENOTTY;
		break;
	}

	up(&gpio_dev_lock);

	return retval;
}

/*!*************************************************************************
	driver file operations struct define
****************************************************************************/
struct file_operations gpio_dev_fops = {
	.owner = THIS_MODULE,
	.open = gpio_dev_open,
	.ioctl = gpio_dev_ioctl,
	.release = gpio_dev_release,
};

static int gpio_dev_probe
(
	struct platform_device *pdev	/*!<; // please add parameters description her*/
)
{
	return 0;
}


static int gpio_dev_remove
(
	struct platform_device *dev	/*!<; // please add parameters description her*/
)
{
	if (gpio_cdev)
		cdev_del(gpio_cdev);

	return 0;
}


static int gpio_dev_suspend
(
	struct platform_device *pdev,     /*!<; // a pointer point to struct device */
	pm_message_t state		/*!<; // suspend state */
)
{
	struct gpio_list *list;
	//int i = 0;

	if (g_gpio_list == NULL) {
		printk("there is no gpio list\n");
		return 0;
	}
	list = g_gpio_list;
	while(1) {
		//printk("list%d : \n",i);
		//printk("address = 0x%x  , bitmap = 0x%x\n", list->addr, list->bitmap);
		list->regval = REG32_VAL(list->addr)&list->bitmap;
		//printk("regval = 0x%x\n", list->regval);
		list = list->next;
		if (list == NULL)
			break;
		//i++;
	}

	return 0;
}


static int gpio_dev_resume
(
	struct platform_device *pdev 	/*!<; // a pointer point to struct device */
)
{

	struct gpio_list *list;
	//int i = 0;

	if (g_gpio_list == NULL) {
		printk("there is no gpio list\n");
		return 0;
	}
	list = g_gpio_list;
	while(1) {
		//printk("list%d : \n",i);
		//printk("address = 0x%x  , bitmap = 0x%x\n", list->addr, list->bitmap);
		//printk("regval = 0x%x\n", list->regval);
		REG32_VAL(list->addr) = list->regval;
		list = list->next;
		if (list == NULL)
			break;
		//i++;
	}

	return 0;
}

/*!*************************************************************************
	device driver struct define
****************************************************************************/
static struct platform_driver gpio_dev_driver = {
	.driver.name           = "wmtgpio", // This name should equal to platform device name.
	.probe          = gpio_dev_probe,
	.remove         = gpio_dev_remove,
	.suspend        = gpio_dev_suspend,
	.resume         = gpio_dev_resume
};


static void gpio_dev_platform_release
(
	struct device *device	/*!<; // please add parameters description her*/
)
{
}

/*!*************************************************************************
	platform device struct define
****************************************************************************/


static struct platform_device gpio_dev_device = {
	.name           = "wmtgpio",
	.id             = 0,
	.dev            = 	{	.release = gpio_dev_platform_release,
						},
};


static int gpio_dev_init(void)
{
	int ret;
	dev_t dev_no;

	gpio_dev_major = register_chrdev (0, GPIO_DEVICE_NAME, &gpio_dev_fops);
	if (gpio_dev_major < 0) {
		printk("get gpio_dev_major failed\n");
		return -EFAULT;
	}
	
	printk("mknod /dev/%s c %d 0\n", GPIO_DEVICE_NAME, gpio_dev_major); 

	class_gpio_dev = class_create(THIS_MODULE, GPIO_DEVICE_NAME);
	if (IS_ERR(class_gpio_dev)) {
		ret = PTR_ERR(class_gpio_dev);
		printk(KERN_ERR "Can't create class : %s !!\n",GPIO_DEVICE_NAME);
		return ret;
	}

	dev_no = MKDEV(gpio_dev_major, 0);
	gpio_device = device_create(class_gpio_dev, NULL, dev_no, NULL, GPIO_DEVICE_NAME);
    if (IS_ERR(gpio_device)) {
    	ret = PTR_ERR(gpio_device);
    	printk(KERN_ERR "Failed to create device %s !!!",GPIO_DEVICE_NAME);
    	return ret;
    }

	ret = platform_device_register(&gpio_dev_device);
	if (ret != 0)
		return -ENODEV;

	ret = platform_driver_register(&gpio_dev_driver);
	if (ret != 0) {
		platform_device_unregister(&gpio_dev_device);
		return -ENODEV;
	}

	return ret;
}

module_init(gpio_dev_init);


static void gpio_dev_exit(void)
{
	dev_t dev_no;

	free_gpio_list();
	platform_driver_unregister(&gpio_dev_driver);
	platform_device_unregister(&gpio_dev_device);
	dev_no = MKDEV(gpio_dev_major, 0);
	device_destroy(class_gpio_dev, dev_no);
    class_destroy(class_gpio_dev);
	unregister_chrdev(gpio_dev_major, GPIO_DEVICE_NAME);

	return;
}

module_exit(gpio_dev_exit);

MODULE_AUTHOR("WonderMedia Technologies, Inc.");
MODULE_DESCRIPTION("WMT [gpio device] driver");
MODULE_LICENSE("GPL");

#undef WMT_GPOI_DEV_C

