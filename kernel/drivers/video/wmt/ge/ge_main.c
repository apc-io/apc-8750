/*
 * Copyright (c) 2008-2011 WonderMedia Technologies, Inc. All Rights Reserved.
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
 */

/* Written by Vincent Chen, WonderMedia Technologies, Inc., 2008-2011 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/errno.h>
#include <linux/uaccess.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/fb.h>
#include <linux/dma-mapping.h>
#include <asm/page.h>
#include <linux/mm.h>
#include <linux/sched.h>

#include "ge_accel.h"
#include "ge_ioctl.h"
#include "gmp.h"

#define GMP
#define MALI
#define APPEND_MEMBLK 1

#ifdef MALI
#include "../mali.h"
static struct mali_device *malidev;
#endif /* MALI */

#ifndef FBIO_WAITFORVSYNC
#define FBIO_WAITFORVSYNC _IOW('F', 0x20, u_int32_t)
#endif

extern int wmt_getsyspara(char *varname, unsigned char *varval, int *varlen);

struct ge_param_s {
	int enabled;
	int buflen1; /* Frame buffer memory */
	int buflen2; /* Internal buffer */
	int buflen3; /* Internal buffer */
};

static struct gmp *gmp;
static struct ge_param_s ge_param;

static struct fb_fix_screeninfo __initdata gefb_fix = {
	.id             = "gefb",
	.smem_start     = 0,
	.smem_len       = 0,
	.type           = FB_TYPE_PACKED_PIXELS,
	.type_aux       = 0,
	.visual         = FB_VISUAL_TRUECOLOR,
	.xpanstep       = 1,
	.ypanstep       = 1,
	.ywrapstep      = 1,
	.line_length    = 0,
	.mmio_start     = 0xd8050000,
	.mmio_len       = 0x0700,
	.accel          = FB_ACCEL_WMT
};

static struct fb_var_screeninfo __initdata gefb_var = {
	.xres           = CONFIG_DEFAULT_RESX,
	.yres           = CONFIG_DEFAULT_RESY,
	.xres_virtual   = CONFIG_DEFAULT_RESX,
	.yres_virtual   = CONFIG_DEFAULT_RESY,
	/*
	.bits_per_pixel = 32,
	.red            = {16, 8, 0},
	.green          = {8, 8, 0},
	.blue           = {0, 8, 0},
	.transp         = {0, 0, 0},
	*/
	.bits_per_pixel = 16,
	.red            = {11, 5, 0},
	.green          = {5, 6, 0},
	.blue           = {0, 5, 0},
	.transp         = {0, 0, 0},
	.activate       = FB_ACTIVATE_NOW | FB_ACTIVATE_FORCE,
	.height         = -1,
	.width          = -1,
	.pixclock       = 39721,
	.left_margin    = 40,
	.right_margin   = 24,
	.upper_margin   = 32,
	.lower_margin   = 11,
	.hsync_len      = 96,
	.vsync_len      = 2,
	.vmode          = FB_VMODE_NONINTERLACED
};

static int gefb_open(struct fb_info *info, int user)
{
	return 0;
}

static int gefb_release(struct fb_info *info, int user)
{
	return ge_release(info);
}

static int gefb_check_var(struct fb_var_screeninfo *var,
			      struct fb_info *info)
{
	/*
	printk(KERN_CRIT "%s: var->yoffset = %d, info->var.yoffset = %d, "
			"info->flags = 0x%08x, var->activate = 0x%08x\n",
			__func__, var->yoffset, info->var.yoffset,
			info->flags, var->activate);
	*/

	switch (var->bits_per_pixel) {
	case 1:
	case 8:
		if (var->red.offset > 8) {
			/* LUT8 */
			var->red.offset = 0;
			var->red.length = 8;
			var->green.offset = 0;
			var->green.length = 8;
			var->blue.offset = 0;
			var->blue.length = 8;
			var->transp.offset = 0;
			var->transp.length = 0;
		}
		break;
	case 16:
		if (var->transp.length) {
			/* ARGB 1555 */
			var->red.offset = 10;
			var->red.length = 5;
			var->green.offset = 5;
			var->green.length = 5;
			var->blue.offset = 0;
			var->blue.length = 5;
			var->transp.offset = 15;
			var->transp.length = 1;
		} else {
			/* RGB 565 */
			var->red.offset = 11;
			var->red.length = 5;
			var->green.offset = 5;
			var->green.length = 6;
			var->blue.offset = 0;
			var->blue.length = 5;
			var->transp.offset = 0;
			var->transp.length = 0;
		}
		break;
	case 24:
		/* RGB 888 */
		var->red.offset = 16;
		var->red.length = 8;
		var->green.offset = 8;
		var->green.length = 8;
		var->blue.offset = 0;
		var->blue.length = 8;
		var->transp.offset = 0;
		var->transp.length = 0;
		break;
	case 32:
		/* ARGB 8888 */
		var->red.offset = 16;
		var->red.length = 8;
		var->green.offset = 8;
		var->green.length = 8;
		var->blue.offset = 0;
		var->blue.length = 8;
		var->transp.offset = 0;
		var->transp.length = 0;
		break;
	}
	return 0;
}

static int gefb_set_par(struct fb_info *info)
{
	struct fb_var_screeninfo *var = &info->var;

	/* init your hardware here */
	if (var->bits_per_pixel == 8)
		info->fix.visual = FB_VISUAL_PSEUDOCOLOR;
	else
		info->fix.visual = FB_VISUAL_TRUECOLOR;

	info->fix.line_length = var->xres_virtual * var->bits_per_pixel / 8;

	if (ge_init(info))
		return -ENOMEM;

	return 0;
}

static int gefb_setcolreg(unsigned regno, unsigned red,
			      unsigned green, unsigned blue,
			      unsigned transp, struct fb_info *info)
{
	if (regno >= 256)  /* no. of hw registers */
		return -EINVAL;

	/* grayscale */

	if (info->var.grayscale) {
		/* grayscale = 0.30*R + 0.59*G + 0.11*B */
		red = green = blue = (red * 77 + green * 151 + blue * 28) >> 8;
	}

	ge_setcolreg(regno, red, green, blue, transp, info);

	/*  The following is for fbcon. */

	if (info->fix.visual == FB_VISUAL_TRUECOLOR ||
		info->fix.visual == FB_VISUAL_DIRECTCOLOR) {

		if (regno >= 16)
			return -EINVAL;

		switch (info->var.bits_per_pixel) {
		case 16:
			((unsigned int *)(info->pseudo_palette))[regno] =
				(red & 0xf800) |
				((green & 0xfc00) >> 5) |
				((blue & 0xf800) >> 11);
				break;
		case 24:
		case 32:
			red   >>= 8;
			green >>= 8;
			blue  >>= 8;
			((unsigned int *)(info->pseudo_palette))[regno] =
				(red << info->var.red.offset) |
				(green << info->var.green.offset) |
				(blue  << info->var.blue.offset);
			break;
		}
	}
	return 0;
}

static int gefb_pan_display(struct fb_var_screeninfo *var,
				struct fb_info *info)
{
	ge_pan_display(var, info);

	return 0;
}

static int gefb_ioctl(struct fb_info *info, unsigned int cmd,
			  unsigned long arg)
{
	int retval = 0;

	if (_IOC_TYPE(cmd) == GEIO_MAGIC)
		return ge_ioctl(info, cmd, arg);

	switch (cmd) {
	case FBIO_WAITFORVSYNC:
		ge_vo_wait_vsync();
		break;
	default:
		break;
	}

	return retval;
}

int gefb_hw_cursor(struct fb_info *info, struct fb_cursor *cursor)
{
	return 0;
}

int gefb_sync(struct fb_info *info)
{
	return ge_sync(info);
}

static struct fb_ops gefb_ops = {
	.owner          = THIS_MODULE,
	.fb_open        = gefb_open,
	.fb_release     = gefb_release,
	.fb_check_var   = gefb_check_var,
	.fb_set_par     = gefb_set_par,
	.fb_setcolreg   = gefb_setcolreg,
	.fb_pan_display = gefb_pan_display,
	.fb_fillrect    = cfb_fillrect,
	.fb_copyarea    = cfb_copyarea,
	.fb_imageblit   = cfb_imageblit,
	.fb_blank       = ge_blank,
	.fb_cursor      = gefb_hw_cursor,
	.fb_ioctl       = gefb_ioctl,
	.fb_sync	= gefb_sync,
};

static int __init gefb_setup(char *options)
{
	char *this_opt;
	unsigned long val;

	if (!options || !*options)
		return 0;

	/* Use video=gefb:vtotal:N as boot parameter, N is MBytes */

	while ((this_opt = strsep(&options, ",")) != NULL) {
		if (!*this_opt)
			continue;
		if (!strncmp(this_opt, "vtotal:", 7)) {
			if (!strict_strtoul(this_opt + 7, 10, &val))
				ge_param.buflen1 = val << 20;
		}
	}

	return 0;
}

#if APPEND_MEMBLK
static int get_mbsize(void)
{
	unsigned char buf[32];
	int varlen = 32;
	int val;

	if (wmt_getsyspara("mbsize", buf, &varlen) == 0) {
		sscanf(buf, "%dM", &val);
	} else {
		val = 0;
	}
	/*
	printk("detected memblk of %d MBytes\n", val);
	*/

	return val;
}
#endif /* APPEND_MEMBLK */

static int get_ge_param(struct ge_param_s *param)
{
	unsigned char buf[32];
	int varlen = 32;

	/* wmt.ge.param 1:-1:0:0 */

	if (wmt_getsyspara("wmt.ge.param", buf, &varlen) == 0) {
		sscanf(buf, "%d:%d:%d:%d", &param->enabled,
			&param->buflen1, &param->buflen2, &param->buflen3);
	} else {
		param->enabled =  1;
		param->buflen1 = -1; /* auto */
		param->buflen2 =  0;
		param->buflen3 =  0;
	}

	printk(KERN_INFO "wmt.ge.param = %d:%d:%d:%d\n", param->enabled,
		param->buflen1, param->buflen2, param->buflen3);

	/* MBytes -> Bytes */
	if (param->buflen1 > 0)
		param->buflen1 <<= 20;
	if (param->buflen2 > 0)
		param->buflen2 <<= 20;
	if (param->buflen3 > 0)
		param->buflen3 <<= 20;

	return 0;
}

static int __init gefb_probe(struct platform_device *dev)
{
	struct fb_info *info;
	int cmap_len, retval;
	unsigned int chip_id;
	char mode_option[] = "1024x768@60";
	unsigned int smem_start;
	unsigned int smem_len;
	unsigned int len;
	unsigned int min_smem_len;

	/* Allocate fb_info and par.*/
	info = framebuffer_alloc(sizeof(unsigned int) * 16, &dev->dev);
	if (!info)
		return -ENOMEM;

	/* Set default fb_info */
	info->fbops = &gefb_ops;
	info->fix = gefb_fix;

	chip_id = SCC_CHIP_ID >> 16;

	info->var = gefb_var;
	ge_vo_get_default_var(&info->var);

	smem_start = (num_physpages << PAGE_SHIFT);
	smem_len = phy_mem_end() - smem_start;

#ifdef MALI
	/* Set mali validation region */
	malidev = create_mali_device();
	if (malidev) {
		malidev->get_memory_base(&smem_start);
		malidev->get_memory_size(&len);
		smem_start += len;
		smem_len -= len;
		malidev->set_mem_validation_base(smem_start);
		malidev->set_mem_validation_size(smem_len);
	}
#endif /* MALI */

	/* Get minimal size for frame buffer */
	len = info->var.xres * info->var.yres *
		(info->var.bits_per_pixel >> 3);
	len *= GE_FB_NUM;
#ifdef GMP
	min_smem_len = (len + GMP_MASK) & ~GMP_MASK;
#else
	min_smem_len = (len + PAGE_MASK) & ~PAGE_MASK;
#endif

#if APPEND_MEMBLK
	len = get_mbsize() << 20;
	if (smem_len > (min_smem_len + len))
		smem_len -= len;
#endif /* APPEND_MEMBLK */

	/* Set frame buffer region */
	get_ge_param(&ge_param);

	info->fix.smem_start = smem_start;

	if (ge_param.buflen1 < 0)
		ge_param.buflen1 = smem_len;

	info->fix.smem_len = ge_param.buflen1;

#ifdef GMP
	/* Use fbdev's invisible area */
	if (smem_len >= (min_smem_len + (4 << GMP_SHIFT))) {
		smem_start += min_smem_len;
		smem_len -= min_smem_len;
		gmp_set_log_level(GMP_LOG_ERR);
		register_gmp_device(smem_start, smem_len);
		gmp = create_gmp(smem_start, smem_len);
	} else
		gmp = NULL;
#endif /* GMP */

	if (!request_mem_region(info->fix.smem_start,
		info->fix.smem_len, "gefb")) {
		printk(KERN_WARNING
			"%s: request memory region failed at %p\n",
			__func__, (void *)info->fix.smem_start);
	}

	info->screen_base = ioremap(info->fix.smem_start,
		info->fix.smem_len);
	if (!info->screen_base) {
		printk(KERN_ERR "%s: ioremap fail %d bytes at %p\n",
			__func__, info->fix.smem_len,
			(void *)info->fix.smem_start);
		return -EIO;
	}

	printk(KERN_INFO "gefb: framebuffer at %p, mapped to %p, "
		"using %dk, total %dk\n",
		(void *)info->fix.smem_start, (void *)info->screen_base,
		info->fix.smem_len >> 10, info->fix.smem_len >> 10);

	/*
	 * The pseudopalette is an 16-member array for fbcon.
	 */
	info->pseudo_palette = info->par;
	info->par = NULL;
	info->flags = FBINFO_DEFAULT;	/* flag for fbcon */

	/*
	 * This should give a reasonable default video mode.
	 */
	retval = fb_find_mode(&info->var, info, mode_option,
			      NULL, 0, NULL, 8);

	if (!retval || retval == 4)
		return -EINVAL;

	/*
	 *  This has to been done !!!
	 */
	cmap_len = 256;	/* Be the same as VESA */
	retval = fb_alloc_cmap(&info->cmap, cmap_len, 0);
	if (retval < 0)
		printk(KERN_ERR "%s: fb_alloc_cmap fail.\n", __func__);

	/*
	 *  The following is done in the case of
	 *  having hardware with a static mode.
	 */
	info->var = gefb_var;

	/*
	 *  Get setting from video output device.
	 */
	ge_vo_get_default_var(&info->var);

	/*
	 *  For drivers that can...
	 */
	gefb_check_var(&info->var, info);

	/*
	 *  Apply setting
	 */
	gefb_set_par(info);
	gefb_pan_display(&info->var, info);

	if (register_framebuffer(info) < 0) {
		ge_exit(info);
		return -EINVAL;
	}
	dev_set_drvdata(&dev->dev, info);

#ifdef CONFIG_LOGO_WMT_ANIMATION
	/* Run boot animation */
    {
        extern void run_boot_animation(struct fb_info *info);
        run_boot_animation(info);
    }
#endif

	return 0;
}

static int gefb_remove(struct platform_device *dev)
{
	struct fb_info *info = dev_get_drvdata(&dev->dev);

	if (info) {
		gmp_free(gmp, info->fix.smem_start);
		ge_exit(info);
		unregister_framebuffer(info);
		fb_dealloc_cmap(&info->cmap);
		framebuffer_release(info);
	}
	return 0;
}

static int gefb_suspend(struct platform_device *dev, pm_message_t state)
{
	struct fb_info *info = dev_get_drvdata(&dev->dev);

	if (info)
		ge_suspend(info);

#ifdef MALI
	if (malidev)
		malidev->suspend(1);
#endif

	return 0;
}

static int gefb_resume(struct platform_device *dev)
{
	struct fb_info *info = dev_get_drvdata(&dev->dev);

	if (info)
		ge_resume(info);

#ifdef MALI
	if (malidev)
		malidev->resume(1);
#endif

	return 0;
}

static struct platform_driver gefb_driver = {
	.driver.name    = "gefb",
	.probe          = gefb_probe,
	.remove         = gefb_remove,
	.suspend        = gefb_suspend,
	.resume         = gefb_resume,
};

static u64 vt8430_fb_dma_mask = 0xffffffffUL;
static struct platform_device gefb_device = {
	.name   = "gefb",
	.dev    = {
		.dma_mask = &vt8430_fb_dma_mask,
		.coherent_dma_mask = ~0,
	},
};

static int __init gefb_init(void)
{
	int ret;
	char *option = NULL;

	fb_get_options("gefb", &option);
	gefb_setup(option);

	ret = platform_driver_register(&gefb_driver);
	if (!ret) {
		ret = platform_device_register(&gefb_device);
		if (ret)
			platform_driver_unregister(&gefb_driver);
	}

	return ret;
}
module_init(gefb_init);

static void __exit gefb_exit(void)
{
	release_gmp(gmp);
	unregister_gmp_device();

	release_mali_device(malidev);

	platform_driver_unregister(&gefb_driver);
	platform_device_unregister(&gefb_device);
	return;
}

module_exit(gefb_exit);

MODULE_AUTHOR("WonderMedia Technologies, Inc.");
MODULE_DESCRIPTION("WMT GE driver");
MODULE_LICENSE("GPL");

