/*++ 
 * WonderMedia common interface for all video related drivers
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
 * 10F, 529, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C
--*//* PVCS version log
** $Log:  $
 * 
 */
#ifndef COM_VIDEO_H
/* To assert that only one occurrence is included */
#define COM_VIDEO_H

/*-------------------- MODULE DEPENDENCY -------------------------------------*/
#ifndef __KERNEL__
#include <stdio.h>
#include <string.h>
//#include "../include/chiptop.h"  // for POST only
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


#endif /* ifndef COM_VIDEO_H */

/*=== END com-video.h ==========================================================*/
