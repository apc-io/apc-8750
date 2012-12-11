/*++ 
 * Copyright (c) 2011 WonderMedia Technologies, Inc.
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
 * 10F, 529, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C
--*/
#ifndef GE_ACCEL_H
#define GE_ACCEL_H

#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>

#ifndef __KERNEL__
#define __KERNEL__
#endif

#include "ge_regs.h"

#define GE_DEBUG 0
#define FB_ACCEL_WMT 0x8710
#define V_MAX 2048
#define H_MAX 2048
#define GE_FB_NUM 3

extern struct task_struct *ge_sem_owner;

unsigned int phy_mem_end(void);
unsigned int phy_mem_end_sub(unsigned int size);
int ge_init(struct fb_info *info);
int ge_exit(struct fb_info *info);
int ge_ioctl(struct fb_info *info, unsigned int cmd, unsigned long arg);
int ge_setcolreg(unsigned regno, unsigned red, unsigned green, unsigned blue,
	unsigned transp, struct fb_info *info);
int ge_pan_display(struct fb_var_screeninfo *var, struct fb_info *info);
int ge_sync(struct fb_info *info);
int ge_release(struct fb_info *info);
int ge_blank(int mode, struct fb_info *info);
int ge_suspend(struct fb_info *info);
int ge_resume(struct fb_info *info);
void ge_set_amx_colorkey(unsigned int color, int erase);
void ge_alpha_blend(unsigned int color);
void ge_simple_rotate(unsigned int phy_src, unsigned int phy_dst,
	int width, int height, int bpp, int arc);
void ge_simple_blit(unsigned int phy_src, unsigned int phy_dst,
	int width, int height, int bpp);
void ge_poll_vsync(void);
void ge_wait_vsync(void);

void ge_vo_get_default_var(struct fb_var_screeninfo *var);
void ge_vo_wait_vsync(void);
#endif
