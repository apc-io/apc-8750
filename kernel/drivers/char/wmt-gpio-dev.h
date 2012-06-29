/*++
linux/drivers/char/wmt-gpio-dev.h

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

#ifndef WMT_GPIO_DEV_H
/* To assert that only one occurrence is included */
#define WMT_GPIO_DEV_H

/*-------------------- MODULE DEPENDENCY -------------------------------------*/
#include <linux/config.h>
#include <linux/types.h>
#include <linux/module.h>
#include <mach/hardware.h>

// Include your headers here



/*-------------------- EXPORTED PRIVATE TYPES---------------------------------*/
struct wmt_reg_op_t{
	unsigned int addr;
	unsigned int bitmap;
	unsigned int regval;
};

struct gpio_operation_t {
	struct  wmt_reg_op_t ctl;
	struct  wmt_reg_op_t oc;
	struct  wmt_reg_op_t od;
	struct  wmt_reg_op_t id;
};

struct gpio_cfg_t {
	struct  wmt_reg_op_t ctl;
	struct  wmt_reg_op_t oc;
};


/*-------------------- EXPORTED PRIVATE VARIABLES -----------------------------*/
#ifdef WMT_GPOI_DEV_C

struct gpio_list {
	unsigned int addr;
	unsigned int bitmap;
	unsigned int regval;
	struct gpio_list *prev;
	struct gpio_list *next;
};

#endif 


/*--------------------- EXPORTED PRIVATE MACROS -------------------------------*/
//#define GPIO_IOC_MAGIC	'g'
#define GPIO_IOC_MAGIC	'6'

#define GPIOCFG _IOW(GPIO_IOC_MAGIC, 1, void *)
#define GPIOWREG _IOW(GPIO_IOC_MAGIC, 2, void *)
#define GPIORREG _IOWR(GPIO_IOC_MAGIC, 3, void *)

#define FREESYSCACHES _IO(GPIO_IOC_MAGIC, 4)


#define GPIO_IOC_MAXNR	5

#endif
/*=== END wmt-gpio-dev.h ==========================================================*/
