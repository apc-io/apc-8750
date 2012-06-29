/* PVCS version log
** $Log:  $
 * 
 */
#ifndef COM_VIDEO_H
/* To assert that only one occurrence is included */
#define COM_VIDEO_H

/*--- com-video.h---------------------------------------------------------------
*   Copyright (C) 2008 WonderMedia Tech. Inc.
*
* MODULE       : com-video.h
* AUTHOR       : Willy Chuang
* DATE         : 2008/11/20
* DESCRIPTION  : 
*------------------------------------------------------------------------------*/

/*--- History ------------------------------------------------------------------- 
*Version 0.01 , Willy Chuang, 2008/11/20
*   First version
*
*------------------------------------------------------------------------------*/
/*-------------------- MODULE DEPENDENCY -------------------------------------*/

#ifndef CFG_LOADER
#ifndef __KERNEL__
#include <stdio.h>
#include <string.h>
//#include "../include/chiptop.h"  // for POST only
#endif
#endif
/*-------------------- EXPORTED PRIVATE CONSTANTS ----------------------------*/

#ifndef BIT
#define BIT(x)              (1<<x)
#endif

/*------------------------------------------------------------------------------

------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    Definitions of enum
------------------------------------------------------------------------------*/

typedef enum {
    VDO_COL_FMT_YUV420,
    VDO_COL_FMT_YUV422H,  
    VDO_COL_FMT_YUV422V,
    VDO_COL_FMT_YUV444,
    VDO_COL_FMT_YUV411,
    VDO_COL_FMT_GRAY,
    VDO_COL_FMT_ARGB,
    VDO_COL_FMT_AUTO,
    VDO_COL_FMT_RGB_888,
    VDO_COL_FMT_RGB_666,
    VDO_COL_FMT_RGB_565,
    VDO_COL_FMT_RGB_1555,
    VDO_COL_FMT_RGB_5551,
    VDO_COL_FMT_MAX,
    VDO_COL_FMT_UNKNOWN
} vdo_color_fmt;


/*------------------------------------------------------------------------------
    Definitions of Struct
------------------------------------------------------------------------------*/

typedef struct {
    /* Physical address for kernel space */
    unsigned int    y_addr;   /* Addr of Y plane in YUV domain or RGB plane in ARGB domain */
    unsigned int    c_addr;   /* C plane address */
    unsigned int    y_size;   /* Buffer size in bytes */
    unsigned int    c_size;   /* Buffer size in bytes */
    unsigned int    img_w;    /* width of valid image (unit: pixel) */
    unsigned int    img_h;    /* height of valid image (unit: line) */
    unsigned int    fb_w;     /* width of frame buffer (scanline offset) (unit: pixel)*/
    unsigned int    fb_h;     /* height of frame buffer (unit: line) */
    unsigned int    bpp;      /* bits per pixel (8/16/24/32) */
    
    vdo_color_fmt   col_fmt;  /* Color format on frame buffer */

    unsigned int    h_crop;   /* Horental Crop (unit: pixel) */
    unsigned int    v_crop;   /* Vertical Crop (unit: pixel) */
	
	unsigned int 	flag;	  /* frame flags */
} vdo_framebuf_t;

#define VDO_FLAG_INTERLACE		BIT(0)
#define VDO_FLAG_MOTION_VECTOR	BIT(1)		/* frame buffer with motion vector table after C frame */
#define VDO_FLAG_MB_ONE			BIT(2)		/* Y/C frame alloc in one mb */

typedef struct {
	unsigned int resx_src;       /* source x resolution */
	unsigned int resy_src;       /* source y resolution */
	unsigned int resx_virtual;   /* virtual x resolution */
	unsigned int resy_virtual;   /* virtual y resolution */
	unsigned int resx_visual;    /* visual x resolution */
	unsigned int resy_visual;    /* visual y resolution */
	unsigned int posx;           /* x position to display screen */
	unsigned int posy;           /* y postion to display screen */
	unsigned int offsetx;        /* x pixel offset from source left edge */
	unsigned int offsety;        /* y pixel offset from source top edge */
} vdo_view_t;

#ifdef CFG_LOADER
#ifndef __u32
#define  __u32 unsigned int
#endif

#ifndef __u16
#define  __u16 unsigned short
#endif

struct fb_bitfield {
	__u32 offset;			/* beginning of bitfield	*/
	__u32 length;			/* length of bitfield		*/
	__u32 msb_right;		/* != 0 : Most significant bit is */ 
					/* right */ 
};

struct fb_var_screeninfo {
	__u32 xres;			/* visible resolution		*/
	__u32 yres;
	__u32 xres_virtual;		/* virtual resolution		*/
	__u32 yres_virtual;
	__u32 xoffset;			/* offset from virtual to visible */
	__u32 yoffset;			/* resolution			*/

	__u32 bits_per_pixel;		/* guess what */
	__u32 grayscale;		/* != 0 Graylevels instead of colors */

	struct fb_bitfield red;		/* bitfield in fb mem if true color, */
	struct fb_bitfield green;	/* else only length is significant */
	struct fb_bitfield blue;
	struct fb_bitfield transp;	/* transparency			*/	

	__u32 nonstd;			/* != 0 Non standard pixel format */

	__u32 activate;			/* see FB_ACTIVATE_*		*/

	__u32 height;			/* height of picture in mm    */
	__u32 width;			/* width of picture in mm     */

	__u32 accel_flags;		/* (OBSOLETE) see fb_info.flags */

	/* Timing: All values in pixclocks, except pixclock (of course) */
	__u32 pixclock;			/* pixel clock in ps (pico seconds) */
	__u32 left_margin;		/* time from sync to picture	*/
	__u32 right_margin;		/* time from picture to sync	*/
	__u32 upper_margin;		/* time from sync to picture	*/
	__u32 lower_margin;
	__u32 hsync_len;		/* length of horizontal sync	*/
	__u32 vsync_len;		/* length of vertical sync	*/
	__u32 sync;			/* see FB_SYNC_*		*/
	__u32 vmode;			/* see FB_VMODE_*		*/
	__u32 rotate;			/* angle we rotate counter clockwise */
	__u32 reserved[5];		/* Reserved for future compatibility */
};

struct fb_fix_screeninfo {
	char id[16];			/* identification string eg "TT Builtin" */
	unsigned long smem_start;	/* Start of frame buffer mem */
					/* (physical address) */
	__u32 smem_len;			/* Length of frame buffer mem */
	__u32 type;			/* see FB_TYPE_*		*/
	__u32 type_aux;			/* Interleave for interleaved Planes */
	__u32 visual;			/* see FB_VISUAL_*		*/ 
	__u16 xpanstep;			/* zero if no hardware panning  */
	__u16 ypanstep;			/* zero if no hardware panning  */
	__u16 ywrapstep;		/* zero if no hardware ywrap    */
	__u32 line_length;		/* length of a line in bytes    */
	unsigned long mmio_start;	/* Start of Memory Mapped I/O   */
					/* (physical address) */
	__u32 mmio_len;			/* Length of Memory Mapped I/O  */
	__u32 accel;			/* Indicate to driver which	*/
					/*  specific chip/card we have	*/
	__u16 reserved[3];		/* Reserved for future compatibility */
};

struct fb_info {
	int node;
	int flags;
	struct fb_var_screeninfo var;	/* Current var */
	struct fb_fix_screeninfo fix;	/* Current fix */

	__u32 state;			/* Hardware state i.e suspend */
	void *fbcon_par;                /* fbcon use-only private area */
	/* From here on everything is device dependent */
	void *par;	
};
#endif
#endif /* ifndef COM_VIDEO_H */

/*=== END com-video.h ==========================================================*/
