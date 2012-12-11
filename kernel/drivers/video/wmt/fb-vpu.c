/*++ 
 * linux/drivers/video/wmt/fb-vpu.c
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

#define FB_VPU_C
// #define DEBUG
/*----------------------- DEPENDENCE -----------------------------------------*/
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
#include <linux/fb.h>
#include <linux/dma-mapping.h>
#include <asm/page.h>
#include <linux/mm.h>

#include "vpp.h"

//
/*----------------------- PRIVATE MACRO --------------------------------------*/
#define DEVICE_NAME "WMFB-VPU" /* appear in /proc/devices & /proc/vt8430fb */

/*----------------------- PRIVATE CONSTANTS ----------------------------------*/
/* #define FBUT_XXXX    1     *//*Example*/

// #define SZ_1K 0x400
#define FB_VRAM_SIZE (VPP_HD_MAX_RESX * VPP_HD_MAX_RESY * 4)
#define B2M(size) (size >> 20)

/*----------------------- PRIVATE TYPE  --------------------------------------*/
/* typedef  xxxx fbut_xxx_t; *//*Example*/

/*----------EXPORTED PRIVATE VARIABLES are defined in vt8430-vt8430fb.h  -------------*/
/*----------------------- INTERNAL PRIVATE VARIABLES - -----------------------*/
/* int  fbut_xxx;        *//*Example*/

static struct fb_fix_screeninfo __initdata vfb_fix = {
	.id             = "WMT VPU",
	.smem_start     = 0,
	.smem_len       = FB_VRAM_SIZE,
	.type           = FB_TYPE_PACKED_PIXELS,
	.type_aux       = 0,
	.visual         = FB_VISUAL_TRUECOLOR,
	.xpanstep       = 1,
	.ypanstep       = 1,
	.ywrapstep      = 1,
	.line_length    = 0,
	.mmio_start     = 0xD8050000,
	.mmio_len       = 0x0700,
	.accel          = FB_ACCEL_NONE
};

struct fb_var_screeninfo vfb_var = {
	.xres           = VPP_HD_DISP_RESX,
	.yres           = VPP_HD_DISP_RESY,
	.xres_virtual   = VPP_HD_DISP_RESX,
	.yres_virtual   = VPP_HD_DISP_RESY,
	.bits_per_pixel = 16,
	.red            = {11, 5, 0},
	.green          = {5, 6, 0},
	.blue           = {0, 5, 0},
	.transp         = {0, 0, 0},
	.activate       = FB_ACTIVATE_NOW | FB_ACTIVATE_FORCE,
	.height         = -1,
	.width          = -1,
	.pixclock       = (VPP_HD_DISP_RESX*VPP_HD_DISP_RESY*VPP_HD_DISP_FPS),
	.left_margin    = 40,
	.right_margin   = 24,
	.upper_margin   = 32,
	.lower_margin   = 11,
	.hsync_len      = 96,
	.vsync_len      = 2,
	.vmode          = FB_VMODE_NONINTERLACED
};

/*--------------------- INTERNAL PRIVATE FUNCTIONS ---------------------------*/
/* void fbut_xxx(void); *//*Example*/

/*----------------------- Function Body --------------------------------------*/

/*!*************************************************************************
* vfb_open()
* 
* Private Function by Sam Shen, 2008/12/23
*/
/*!
* \brief
*
* \retval  0 if success
*/
static int vfb_open
(
	struct fb_info *info,   /*!<; // a pointer point to struct fb_info */
	int user                /*!<; // user space mode */
)
{
	DBGMSG("Enter vfb_open\n");
	return 0;
} /* End of vfb_open */

/*!*************************************************************************
* vfb_release()
* 
* Private Function by Sam Shen, 2008/12/23
*/
/*!
* \brief
*
* \retval  0 if success
*/
static int vfb_release
(
	struct fb_info *info,   /*!<; // a pointer point to struct fb_info */
	int user                /*!<; // user space mode */
)
{
	DBGMSG("Enter vfb_release\n");
	return 0;
} /* End of vfb_release */

/*!*************************************************************************
* vfb_show_var()
* 
* Private Function by Sam Shen, 2008/12/23
*/
/*!
* \brief
*
* \retval  None
*/
void vfb_show_var(struct fb_var_screeninfo *var)
{
	printk("-----------------------------\n");
	printk("[wmfb] vt8430fb_show_var\n");
	printk("visible %d, %d\n",var->xres,var->yres);
	printk("virtual %d, %d\n",var->xres_virtual,var->yres_virtual);
	printk("offset %d, %d\n",var->xoffset,var->yoffset);
	printk("bpp %d, grayscale %d\n",var->bits_per_pixel,var->grayscale);
//	printk("bit field r %d, g %d, b %d, t %d\n",(int)var->red,(int)var->green,(int)var->blue,(int)var->transp);
	printk("nonstd %d, activate %d, height %d, width %d\n",var->nonstd,var->activate,var->height,var->width);
	printk("accel %d\n",var->accel_flags);
	printk("-----------------------------\n");
	printk("pixclk %d\n",var->pixclock);
	printk("left %d,right %d,upper %d,lower %d\n",var->left_margin,var->right_margin,var->upper_margin,var->lower_margin);
	printk("hsync %d,vsync %d,sync %d\n",var->hsync_len,var->vsync_len,var->sync);
	printk("vmode %d,rotate %d\n",var->vmode,var->rotate);
	printk("-----------------------------\n");
	return;
}

/*!*************************************************************************
* vfb_check_var()
* 
* Private Function by Sam Shen, 2008/12/23
*/
/*!
* \brief        checks var and eventually tweaks it to something supported
*
* \retval  0 if success
*/
static int vfb_check_var
(
	struct fb_var_screeninfo *var,  /*!<; // a pointer point to struct fb_var_screeninfo ( user input ) */
	struct fb_info *info            /*!<; // a pointer point to struct fb_info */
)
{

	DBGMSG("Enter vfb_check_var\n");

	DBGMSG("src-xres:%d,yres:%d\n", var->xres,var->yres);

	DBGMSG("dest-xres:%d,yres:%d\n", var->xres,var->yres);

	var->xres_virtual = var->xres;
	var->yres_virtual = info->fix.smem_len / 
		(var->xres_virtual * (var->bits_per_pixel >> 3));

	switch (var->bits_per_pixel) {
	case 1:
	case 8:
		if (var->red.offset > 8) {
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
	case 16:                /* ARGB 1555 */
		if (var->transp.length) {
			var->red.offset = 10;
			var->red.length = 5;
			var->green.offset = 5;
			var->green.length = 5;
			var->blue.offset = 0;
			var->blue.length = 5;
			var->transp.offset = 15;
			var->transp.length = 1;
		} else {        /* RGB 565 */
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
	case 24:                /* RGB 888 */
		var->red.offset = 16;
		var->red.length = 8;
		var->green.offset = 8;
		var->green.length = 8;
		var->blue.offset = 0;
		var->blue.length = 8;
		var->transp.offset = 0;
		var->transp.length = 0;
		break;
	case 32:                /* ARGB 8888 */
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
} /* End of vfb_check_var */

/*!*************************************************************************
* vfb_set_par()
* 
* Private Function by Sam Shen, 2008/12/23
*/
/*!
* \brief   set the video mode according to info->var
*
* \retval  0 if success
*/
static int vfb_set_par
(
	struct fb_info *info /*!<; // a pointer point to struct fb_info */
)
{
	struct fb_var_screeninfo *var = &info->var;  

	DBGMSG("Enter vfb_set_par\n");
	
	/* init your hardware here */
	/* ... */    
	if (var->bits_per_pixel == 8) {
		info->fix.visual = FB_VISUAL_PSEUDOCOLOR;
	} else {
		info->fix.visual = FB_VISUAL_TRUECOLOR;
	}
	info->fix.line_length = var->xres_virtual * var->bits_per_pixel / 8;    

	vpp_set_par(info);

	return 0;
} /* End of vt8430fb_set_par */

/*!*************************************************************************
* vfb_setcolreg()
* 
* Private Function by Sam Shen, 2008/12/23
*/
/*!
* \brief   blank display
*
* \retval  0 if success
*/ 
static int vfb_setcolreg
(
	unsigned regno,         /*!<; // register no */
	unsigned red,           /*!<; // red color map */
	unsigned green,         /*!<; // green color map */
	unsigned blue,          /*!<; // blue color map */
	unsigned transp,        /*!<; // transp map */
	struct fb_info *info    /*!<; // a pointer point to struct fb_info */
)
{
	return 0;

} /* End of vfb_setcolreg */

/*!*************************************************************************
* vfb_pan_display()
* 
* Private Function by Sam Shen, 2008/12/23
*/
/*!
* \brief
*       Pan (or wrap, depending on the `vmode' field) the display using the
*       `xoffset' and `yoffset' fields of the `var' structure.
*       If the values don't fit, return -EINVAL.
*
* This call looks only at xoffset, yoffset and the FB_VMODE_YWRAP flag
*
* \retval  0 if success
*/ 
static int vfb_pan_display
(
	struct fb_var_screeninfo *var,  /*!<; // a pointer point to struct fb_var_screeninfo */
	struct fb_info *info            /*!<; // a pointer point to struct fb_info */
)
{
	DBGMSG("Enter vfb_pan_display\n");

	return 0;
} /* End of vfb_pan_display */

/*!*************************************************************************
* vfb_ioctl()
* 
* Private Function by Sam Shen, 2008/12/23
*/
/*!
* \brief   perform fb specific ioctl (optional)
*
* \retval  0 if success
*/ 
static int vfb_ioctl
(
	struct fb_info *info,    /*!<; // a pointer point to struct fb_info */
	unsigned int cmd,       /*!<; // ioctl command */
	unsigned long arg       /*!<; // a argument pointer */
)
{
	int retval = 0;

//	printk("Enter vfb_ioctl %x\n",cmd);

	switch (_IOC_TYPE(cmd)) {
	case VPPIO_MAGIC:
		retval = vpp_ioctl(cmd,arg);
		break;
	default:
		break;
	}
	return retval;
} /* End of vfb_ioctl */

/*!*************************************************************************
* vfb_mmap()
* 
* Private Function by Sam Shen, 2008/12/23
*/
/*!
* \brief   perform fb specific mmap
*
* \retval  0 if success
*/ 
static int vfb_mmap
(
	struct fb_info *info,           /*!<; // a pointer point to struct fb_info */
	struct vm_area_struct *vma      /*!<; // a pointer point to struct vm_area_struct */
)
{
	unsigned long off;
	unsigned long start;
	u32 len;

	DBGMSG("Enter vfb_mmap\n");

	if (vpp_mmap(vma) == 0)
		return 0;

	/* frame buffer memory */
	start = info->fix.smem_start;
	off = vma->vm_pgoff << PAGE_SHIFT;
	len = PAGE_ALIGN((start & ~PAGE_MASK) + info->fix.smem_len);
	if (off >= len) {
		/* memory mapped io */
		off -= len;
		if (info->var.accel_flags) {
			return -EINVAL;
		}
		start = info->fix.mmio_start;
		len = PAGE_ALIGN((start & ~PAGE_MASK) + info->fix.mmio_len);
	}

	start &= PAGE_MASK;
	if ((vma->vm_end - vma->vm_start + off) > len)
		return -EINVAL;
	off += start;
	vma->vm_pgoff = off >> PAGE_SHIFT;
	vma->vm_flags |= VM_IO | VM_RESERVED;
#if defined(__i386__) || defined(__x86_64__)
	printk("_i386_ \n");
	if (boot_cpu_data.x86 > 3)
		pgprot_val(vma->vm_page_prot) |= _PAGE_PCD;
#elif defined(__ia64__) || defined(__arm__) || defined(__sh__)
	vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
#endif
	if (remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff, vma->vm_end - vma->vm_start, vma->vm_page_prot))
		return -EAGAIN;

	return 0;
} /* End of vfb_mmap */


/*!*************************************************************************
* vfb_hw_cursor()
* 
* Private Function by Sam Shen, 2008/12/23
*/
/*!
* \brief   A null function to replace the soft_cursor(). 
*
* \retval  0 if success
*/ 

int vfb_hw_cursor(struct fb_info *info, struct fb_cursor *cursor)
{
	return 0;
} /* End of vfb_hw_cursor */

/***************************************************************************
	driver file operations struct define
****************************************************************************/
static struct fb_ops vfb_ops = {
	.owner          = THIS_MODULE,
	.fb_open        = vfb_open,
	.fb_release     = vfb_release,
	//.fb_read      = vfb_read,
	//.fb_write     = vfb_write,
	.fb_check_var   = vfb_check_var,
	.fb_set_par     = vfb_set_par,
	.fb_setcolreg   = vfb_setcolreg,
	.fb_pan_display = vfb_pan_display,
	.fb_fillrect    = cfb_fillrect,
	.fb_copyarea    = cfb_copyarea,
	.fb_imageblit   = cfb_imageblit,
#ifdef CONFIG_VIA	
	.fb_blank       = ge_blank,
#endif	
	.fb_cursor      = vfb_hw_cursor,
	.fb_ioctl       = vfb_ioctl,
	.fb_mmap        = vfb_mmap,
};

/*!*************************************************************************
* vfb_probe()
* 
* Private Function by Sam Shen, 2008/12/23
*/
/*!
* \brief
*
* \retval  0 if success
*/ 
static int __init vfb_probe
(
	struct platform_device *dev /*!<; // a pointer point to struct device */
)
{
	struct fb_info *info;
	int cmap_len; // , retval;
	//dma_addr_t map_dma;
	//u32 *map_cpu;
//	u32 map_size;
	char mode_option[20];

	sprintf(mode_option,"%dx%d@%d",VPP_HD_DISP_RESX,VPP_HD_DISP_RESY,VPP_HD_DISP_FPS);
//	printk("[vfb] %s\n",mode_option);

	/*  Dynamically allocate memory for fb_info and par.*/
	info = framebuffer_alloc(sizeof(u32) * 16, &dev->dev);
	if (!info) {
		release_mem_region(info->fix.smem_start, info->fix.smem_len);
		return -ENOMEM;
	}

	/* Set default fb_info */
	info->fbops = &vfb_ops;
	info->fix = vfb_fix;

	/* Auto detect VRAM */
// #ifdef CONFIG_VIA	
//	if (info->fix.smem_start == 0) 
//		info->fix.smem_start = ge_vram_addr(B2M(FB_VRAM_SIZE));
// #endif

#if 0
	/* Set video memory */
	if (!request_mem_region(info->fix.smem_start, 
		info->fix.smem_len, "viafb")) {
		printk(KERN_WARNING
			"viafb: abort, cannot reserve video memory at 0x%lx\n",
			info->fix.smem_start);
	}

	info->screen_base = ioremap(info->fix.smem_start, info->fix.smem_len);
	if (!info->screen_base) {
		printk(KERN_ERR
			"viafb: abort, cannot ioremap video memory 0x%x @ 0x%lx\n",
			info->fix.smem_len, info->fix.smem_start);
		return -EIO;
	}

	printk(KERN_INFO "viafb: framebuffer at 0x%lx, mapped to 0x%p, "
		"using %dk, total %dk\n",
		info->fix.smem_start, info->screen_base,
		info->fix.smem_len, info->fix.smem_len);

	/* 
	 *  Do as a normal fbdev does, but allocate a larger memory for GE. 
	 */
	map_size = PAGE_ALIGN(info->fix.smem_len);
#endif

	/*
	 * The pseudopalette is an 16-member array for fbcon.
	 */
	info->pseudo_palette = info->par;
	info->par = NULL;
	info->flags = FBINFO_DEFAULT; // flag for fbcon.

	/* 
	 *  This has to been done !!! 
	 */
	cmap_len = 256; // Be the same as VESA.
	fb_alloc_cmap(&info->cmap, cmap_len, 0);

	/*
	 *  The following is done in the case of 
	 *  having hardware with a static mode.
	 */
	info->var = vfb_var;

	/*
	 * This should give a reasonable default video mode.
	 */
#if 0
	retval = fb_find_mode(&info->var, info, mode_option, NULL, 0, NULL, 8);
//	vt8430fb_show_var(&info->var);

	if (!retval || retval == 4)
		return -EINVAL;
#endif
	/*
	 *  For drivers that can...
	 */
	vfb_check_var(&info->var, info);

	/*
	 *  It's safe to allow fbcon to do it for you.
	 *  But in this case, we need it here.
	 */
	vfb_set_par(info);

	if (register_framebuffer(info) < 0){
		return -EINVAL;
	}
	printk(KERN_INFO "fb%d: %s frame buffer device\n", info->node, info->fix.id);
	dev_set_drvdata(&dev->dev, info);
//
//	printk("vfb_probe out\n\r");

	return 0;
} /* End of vfb_probe */

/*!*************************************************************************
* vfb_remove()
* 
* Private Function by Sam Shen, 2008/12/23
*/
/*!
* \brief
*
* \retval  0 if success
*/ 
static int vfb_remove
(
	struct platform_device *dev /*!<; // a pointer point to struct device */
)
{
	struct fb_info *info = dev_get_drvdata(&dev->dev);

	if (info) {
		unregister_framebuffer(info);
		fb_dealloc_cmap(&info->cmap);
		framebuffer_release(info);
	}

	return 0;
}

#ifdef CONFIG_PM
/*!*************************************************************************
* vfb_suspend()
* 
* Private Function by Sam Shen, 2008/12/23
*/
/*!
* \brief
*
* \retval  0 if success
*/ 
static int vfb_suspend
(
	struct platform_device *pDev,     /*!<; // a pointer point to struct device */
	pm_message_t state				/*!<; // suspend state */
)
{
	DBGMSG("Enter vfb_suspend\n");

	vpp_suspend(state.event);
	return 0;
} /* End of vfb_suspend */

/*!*************************************************************************
* vfb_resume()
* 
* Private Function by Sam Shen, 2008/12/23
*/
/*!
* \brief
*
* \retval  0 if success
*/ 
static int vfb_resume
(
	struct platform_device *pDev 	/*!<; // a pointer point to struct device */
)
{
	DBGMSG("Enter vfb_resume\n");

	vpp_resume();
	return 0;
} /* End of vfb_resume */
#else
#define vfb_suspend NULL
#define vfb_resume NULL
#endif

/***************************************************************************
	device driver struct define
****************************************************************************/
static struct platform_driver vfb_driver = {
	.driver.name    = "wmtvfb", // This name should equal to platform device name.
	.driver.bus     = &platform_bus_type,
	.probe          = vfb_probe,
	.remove         = vfb_remove,
	.suspend        = vfb_suspend,
	.resume         = vfb_resume,
};

/***************************************************************************
	platform device struct define
****************************************************************************/
static u64 vfb_dma_mask = 0xffffffffUL;
static struct platform_device vfb_device = {
	.name   = "wmtvfb",
	.dev    = {
		.dma_mask = &vfb_dma_mask,
		.coherent_dma_mask = ~0,
	},

#if 0
	.id     = 0,
	.dev    = {
		.release = vfb_platform_release,
	},
	.num_resources  = 0,    /* ARRAY_SIZE(vfb_resources), */
	.resource       = NULL, /* vfb_resources, */
#endif
};

/*!*************************************************************************
* vfb_init()
* 
* Private Function by Sam Shen, 2008/12/23
*/
/*!
* \brief
*
* \retval  0 if success
*/ 
static int __init vfb_init(void)
{
	int ret;

	/*
	 *  For kernel boot options (in 'video=vfb:<options>' format)
	 */
	ret = platform_driver_register(&vfb_driver);
	if (!ret) {
		ret = platform_device_register(&vfb_device);
		if (ret)
			platform_driver_unregister(&vfb_driver);
	}

	return ret;

} /* End of vfb_init */
module_init(vfb_init);

/*!*************************************************************************
* vfb_exit()
* 
* Private Function by Sam Shen, 2008/12/23
*/
/*!
* \brief
*
* \retval  0 if success
*/ 
static void __exit vfb_exit(void)
{
	printk(KERN_ALERT "Enter vfb_exit\n");

	platform_driver_unregister(&vfb_driver);
	platform_device_unregister(&vfb_device);
	return;
} /* End of vfb_exit */
module_exit(vfb_exit);

MODULE_AUTHOR("WonderMedia SW Team Sam Shen");
MODULE_DESCRIPTION("vfb device driver");
MODULE_LICENSE("GPL");

/*--------------------End of Function Body -----------------------------------*/

#undef FB_VPU_C
