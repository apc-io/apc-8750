/*++ 
 * linux/drivers/video/wmt/vpp-osif.h
 * WonderMedia video post processor (VPP) driver
 *
 * Copyright c 2010  WonderMedia  Technologies, Inc.
 *
 * This program is free software: you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation, either version 2 of the License, or 
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
 * WonderMedia Technologies, Inc.
 * 4F, 533, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C
--*/

#ifndef VPP_OSIF_H
#define VPP_OSIF_H

/*-------------------- DEPENDENCY -------------------------------------*/
#ifdef CFG_LOADER
	#define CONFIG_WMT_EDID
	#define CONFIG_WMT_EXT_DEV_PLUG_DISABLE

	#include <common.h>
	#include <malloc.h>
	#include "hw_devices.h"
	#include "hw/wmt_mmap.h"	
	#include "hw/wmt-pwm.h"
	#include "hw/wmt-ost.h"
	#include "hw/wmt_gpio.h"	
	#include "wmt_display.h"
	#include "../../board/wmt/include/wmt_clk.h"
	#include "../../board/wmt/include/i2c.h"
	extern int auto_pll_divisor(enum dev_id dev, enum clk_cmd cmd, int unit, int freq);
	extern struct fb_var_screeninfo vfb_var;
	extern int g_direct_path;
	#ifdef __KERNEL__
	#undef __KERNEL__
	#endif
	#ifndef bool
	#define bool char
	#endif
/* -------------------------------------------------- */
#elif defined(__KERNEL__)
	#include <linux/version.h>
	#include <linux/kernel.h>
	#include <linux/string.h>
	#include <linux/delay.h>
	#include <linux/timer.h>
	#include <linux/interrupt.h>
	#include <linux/spinlock.h>
	#include <linux/slab.h>
	#include <linux/module.h>
	#include <linux/i2c.h>
	#include <linux/wmt-mb.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
    #include <mach/hardware.h>
    #include <mach/wmt_mmap.h>

    #define SA_INTERRUPT IRQF_DISABLED
#else
    #include <asm/arch-wmt/hardware.h>
    #include <asm/arch/wmt_mmap.h>
#endif
/* -------------------------------------------------- */
#else	// __KERNEL__
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>

    #define __ASM_ARCH_HARDWARE_H
  	#include "../include/wmt_mmap.h"
	#include "../pmc/wmt_clk.h"
#endif

/*	following is the C++ header	*/
#ifdef	__cplusplus
extern	"C" {
#endif

/*-------------------- EXPORTED PRIVATE CONSTANTS ----------------------------*/
#ifdef __KERNEL__
	#define THE_MB_USER			"VPP-MB"
	#define DPRINT printk
/* -------------------------------------------------- */
#else
	#define REG32_VAL(addr) (*(volatile unsigned int *)(addr))
	#define REG16_VAL(addr) (*(volatile unsigned short *)(addr))
	#define REG8_VAL(addr)  (*(volatile unsigned char *)(addr))

	#define U32 unsigned int
	#define U16 unsigned short
	#define U8 unsigned char

	#define mb_alloc(a)	malloc(a)
	#define kmalloc(a,b) 	malloc(a)
	#define kfree(a)		free(a)
	#define GFP_KERNEL		0
	#define module_init(a)	

	#define DPRINT printf
	#define phys_to_virt(a)                 (a)
	#define virt_to_phys(a)                 (a)
	#define EXPORT_SYMBOL(a)

	#define IRQ_GPIO		0
	#define IRQF_SHARED 	0
	#define IRQF_DISABLED	0
	#define SA_INTERRUPT	0
	#define KERN_ALERT
	#define KERN_ERR
	#define KERN_DEBUG
	#define mdelay(x) wmt_delayus(1000*x)
	#define udelay(x) wmt_delayus(x)
	#define printk printf
	#define BIT(x) (1<<x)
#endif

/*-------------------- EXPORTED PRIVATE TYPES---------------------------------*/
/* typedef  void  hdmi_xxx_t;  *//*Example*/

/*-------------------- EXPORTED PRIVATE VARIABLES -----------------------------*/
#ifdef VPP_OSIF_C
#define EXTERN
#else
#define EXTERN   extern
#endif /* ifdef VPP_OSIF_C */

/* EXTERN int      hdmi_xxx; *//*Example*/

#undef EXTERN

/*--------------------- EXPORTED PRIVATE MACROS -------------------------------*/
/* #define HDMI_XXX_YYY   xxxx *//*Example*/
#ifdef DEBUG
#define DBGMSG(fmt, args...)  DPRINT("%s: " fmt, __FUNCTION__ , ## args)
#else
#define DBGMSG(fmt, args...) do {} while(0)
#endif

/*--------------------- EXPORTED PRIVATE FUNCTIONS  ---------------------------*/
/* extern void  hdmi_xxx(void); *//*Example*/

#ifdef __KERNEL__
	void vpp_dbg_show(int level,int tmr,char *str);
	extern void wmt_i2c_xfer_continue_if(struct i2c_msg *msg, unsigned int num);
	extern void wmt_i2c_xfer_if(struct i2c_msg *msg);
	extern int wmt_i2c_xfer_continue_if_4(struct i2c_msg *msg, unsigned int num, int bus_id);
#else
#ifndef CFG_LOADER
	void vpp_initialization(int FunctionNumber);
	void udelay(int us);
	void mdelay(int ms);
	extern int auto_pll_divisor(enum dev_id dev, enum clk_cmd cmd, int unit, int freq);
	extern void udelay(int us);
#else
	extern int wmt_getsyspara(char *varname, unsigned char *varval, int *varlen);
#endif
#endif

int vpp_request_irq(unsigned int irq_no,void *routine,unsigned int flags,char *name,void *arg);
void vpp_free_irq(unsigned int irq_no,void *arg);
int vpp_parse_param(char *buf,unsigned int *param,int cnt);
void vpp_lock(void);
void vpp_unlock(void);

#ifdef	__cplusplus
}
#endif
#endif //VPP_OSIF_H

