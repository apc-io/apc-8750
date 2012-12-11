#ifndef GE_REGS_H
#define GE_REGS_H 1

#if !defined(__KERNEL__) && !defined(__POST__)
#ifndef __LINUX__
#define __LINUX__
#endif /* __LINUX__ */
#endif

#ifdef __KERNEL__
#include <linux/uaccess.h>
#include <asm/page.h>
#include <linux/semaphore.h>
#include <linux/ioctl.h>
#include <linux/mm.h>
#include <linux/fb.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/dma-mapping.h>
#include <mach/hardware.h>
#ifndef LINUX_VERSION_CODE
#include <linux/version.h>
#endif /* LINUX_VERSION_CODE */
#endif

#ifdef __POST__
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#endif

#ifdef __LINUX__
#include <linux/fb.h>
#include <linux/errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#endif

#include "gmp.h"
#include "ge_regs_def.h"

#ifdef __LINUX__
#include <ge_plugin.h>
#endif

#if defined(__KERNEL__)
#define SCC_CHIP_ID	(*(unsigned int *)SYSTEM_CFG_CTRL_BASE_ADDR)
#elif defined(__POST__)
#define SCC_CHIP_ID	(*(unsigned int *)0xd8120000)
#else
#define SCC_CHIP_ID	(0)
#endif

#ifdef __KERNEL__
#define GE_MMIO_START	(0xd8050000 + WMT_MMAP_OFFSET)
#define GE_MMIO_OFFSET	0x400
#else
#define GE_MMIO_START	(0xd8050000)
#define GE_MMIO_OFFSET	0x400
#endif

#ifndef BIT0
#define BIT0	0x00000001
#define BIT1	0x00000002
#define BIT2	0x00000004
#define BIT3	0x00000008
#define BIT4	0x00000010
#define BIT5	0x00000020
#define BIT6	0x00000040
#define BIT7	0x00000080
#define BIT8	0x00000100
#define BIT9	0x00000200
#define BIT10	0x00000400
#define BIT11	0x00000800
#define BIT12	0x00001000
#define BIT13	0x00002000
#define BIT14	0x00004000
#define BIT15	0x00008000
#define BIT16	0x00010000
#define BIT17	0x00020000
#define BIT18	0x00040000
#define BIT19	0x00080000
#define BIT20	0x00100000
#define BIT21	0x00200000
#define BIT22	0x00400000
#define BIT23	0x00800000
#define BIT24	0x01000000
#define BIT25	0x02000000
#define BIT26	0x04000000
#define BIT27	0x08000000
#define BIT28	0x10000000
#define BIT29	0x20000000
#define BIT30	0x40000000
#define BIT31	0x80000000
#endif

#ifdef __POST__
#define EPERM            1      /* Operation not permitted */
#define EINTR            4      /* Interrupted system call */
#define EIO              5      /* I/O error */
#define EAGAIN          11      /* Try again */
#define ENOMEM          12      /* Out of memory */
#define EACCES          13      /* Permission denied */
#define EFAULT          14      /* Bad address */
#define EBUSY           16      /* Device or resource busy */
#define EINVAL          22      /* Invalid argument */
#endif

#ifndef REG_SET32
#define REG_SET32(addr, val)	(*(volatile unsigned int *)(addr)) = val
#define REG_GET32(addr)		(*(volatile unsigned int *)(addr))
#define REG_VAL32(addr)		(*(volatile unsigned int *)(addr))
#define REG_SET16(addr, val)	(*(volatile unsigned short *)(addr)) = val
#define REG_GET16(addr)		(*(volatile unsigned short *)(addr))
#define REG_VAL16(addr)		(*(volatile unsigned short *)(addr))
#define REG_SET8(addr, val)	(*(volatile unsigned char *)(addr)) = val
#define REG_GET8(addr)		(*(volatile unsigned char *)(addr))
#define REG_VAL8(addr)		(*(volatile unsigned char *)(addr))
#endif

/* GL.h */

/* Pixel Formats
 *
 * RGB24 - RGB (24bpp), interleaved format, R-G-B order
 * BGR24 - RGB (24bpp), interleaved format, B-G-R order
 * YUV444P - YUV 4:4:4 (24bpp), basic planar format, Y-U-V (Y/Cb/Cr) order
 * YUV422P - YUV 4:2:2 (16bpp), basic planar format, Y-U-V (Y/Cb/Cr) order
 * YUV420P (aka "I420") - YUV 4:2:0 (12bpp), basic planar format, Y-U-V (Y/Cb/Cr) order
 * YVU420P (aka "YV12") - YUV 4:2:0 (12bpp), basic planar format, Y-V-U (Y/Cr/Cb) order
 * YUV422I (aka "YUY2") - YUV 4:2:2 (16bpp), interleaved format, Y-U-Y-V (Y/Cb/Y/Cr) order
 * YVU422I (aka "YVYU") - YUV 4:2:2 (16bpp), interleaved format, Y-V-Y-U (Y/Cr/Y/Cb) order
 * I  = interleaved
 * P  = planar
 * SP = semi planar
 */

/* invalid old pixel format */
/*
#define GSPF_FMT(cd, hm) (((cd) << 4) | (hm & 0xf))
#define GSPF_RGB32  GSPF_FMT(3, 0)
#define GSPF_ARGB   GSPF_FMT(3, 1)
#define GSPF_RGB24  GSPF_FMT(2, 0)
#define GSPF_RGB16  GSPF_FMT(1, 0)
#define GSPF_RGB555 GSPF_FMT(1, 1)
#define GSPF_RGB454 GSPF_FMT(1, 2)
#define GSPF_LUT8   GSPF_FMT(0, 0)
#define GSPF_RGB_MASK   0xff
#define GSPF_YUV_MASK	0xff00
*/

#define GSPF_XXX(fmt)             ((fmt >> 13) & 7)
#define GSPF_YUV(fmt)             ((fmt >> 10) & 7)
#define GSPF_PLANES(fmt)          (((fmt >> 8) & 3) + 1)
#define GSPF_VARIANT(fmt)         ((fmt >> 5) & 7)
#define GSPF_BPP(fmt)             ((fmt & 0x1f) + 1)

#define GSPF_BITS_PER_PIXEL(fmt)  GSPF_BPP(fmt)
#define GSPF_BYTES_PER_PIXEL(fmt) ((GSPF_BPP(fmt) + 4) >> 3)

/*
 * yuv: 0 = RGB, 1 = YUV411, 2 = YUV420, 3 = YUV422, 4 = YUV444, 5 = AYUV,
 *      7 = VPU
 */

#define GSPF_FMT(xxx, yuv, planes, variant, bpp) \
	((xxx & 7) << 13 | \
	(yuv & 7) << 10 | \
	((planes - 1) & 3) << 8 | \
	((variant & 7) << 5) | \
	((bpp - 1) & 0x1f))

#define GSPF_RGB32  GSPF_FMT(0, 0, 1, 0, 32)
#define GSPF_ARGB   GSPF_FMT(0, 0, 1, 1, 32)
#define GSPF_RGB24  GSPF_FMT(0, 0, 1, 0, 24)
#define GSPF_RGB16  GSPF_FMT(0, 0, 1, 0, 16)
#define GSPF_RGB555 GSPF_FMT(0, 0, 1, 1, 16)
#define GSPF_RGB454 GSPF_FMT(0, 0, 1, 2, 16)
#define GSPF_LUT8   GSPF_FMT(0, 0, 1, 0, 8)

#define GSPF_RAW8  GSPF_LUT8
#define GSPF_RAW16 GSPF_RGB16
#define GSPF_RAW24 GSPF_RGB24
#define GSPF_RAW32 GSPF_RGB32

/* 12 bit   YUV (8 bit Y plane followed by 8 bit quarter size U/V planes) */
#define GSPF_I420 GSPF_FMT(0, 2, 3, 0, 12)

/* 12 bit   YUV (8 bit Y plane followed by 8 bit quarter size V/U planes) */
#define GSPF_YV12 GSPF_FMT(0, 2, 3, 1, 12)

/* 12 bit   YUV (8 bit Y plane followed by one 16 bit quarter size Cb|Cr [7:0|7:0] plane) */
#define GSPF_NV12 GSPF_FMT(0, 2, 2, 0, 12)

/* 12 bit   YUV (8 bit Y plane followed by one 16 bit quarter size Cr|Cb [7:0|7:0] plane) */
#define GSPF_NV21 GSPF_FMT(0, 2, 2, 1, 12)

/* 16 bit   YUV (4 byte/ 2 pixel, macropixel contains CbYCrY [31:0]) */
#define GSPF_YUY2 GSPF_FMT(0, 2, 1, 0, 16)

/* 16 bit   YUV (4 byte/ 2 pixel, macropixel contains YCbYCr [31:0]) */
#define GSPF_UYVY GSPF_FMT(0, 2, 1, 1, 16)

/* 16 bit   YUV (8 bit Y plane followed by one 16 bit half width Cb|Cr [7:0|7:0] plane) */
#define GSPF_NV16 GSPF_FMT(0, 3, 2, 0, 16)

/* 32 bit  AYUV (4 byte, alpha 8@24, Y 8@16, Cb 8@8, Cr 8@0) */
#define GSPF_AYUV GSPF_FMT(0, 5, 1, 0, 32)

#define GSPF_YUV411SP	GSPF_FMT(0, 1, 2, 0, 12) /* for WonderMedia only */
#define GSPF_YUV420P	GSPF_I420
#define GSPF_YUV420SP	GSPF_NV12
#define GSPF_YUV422I	GSPF_YUY2
#define GSPF_YUV422SP	GSPF_NV16
#define GSPF_YUV444SP	GSPF_FMT(0, 4, 2, 0, 24) /* for WonderMedia only */
#define GSPF_VPU	GSPF_FMT(0, 7, 2, 0, 32) /* for WonderMedia only */

/* GE commands */
#define GECMD_BLIT		0x1
#define GECMD_TEXT		0x2
#define GECMD_BLIT_DMA		0x3 /* Obsoleted */
#define GECMD_BEZIER		0x4 /* Obsoleted */
#define GECMD_LINE		0x7 /* Obsoleted */
#define GECMD_ROTATE		0x8
#define GECMD_MIRROR		0x9
#define GECMD_DMA		0xa /* YC->RGB, WM8650 only */
#define GECMD_YUY2_YUV420	0xb
#define GECMD_C422_C420		0xc /* Obsoleted */
#define GECMD_C444_C420		0xd /* Obsoleted */
#define GECMD_ALPHA_BITBLT	0xe
#define GECMD_GOVR_DIRECT	0xf

/* AMX CSC table */
#define AMX_CSC_DEFAULT			0
#define AMX_CSC_RGB_YC_SDTV_16_235	1
#define AMX_CSC_RGB_YC_SDTV_0_255	2
#define AMX_CSC_RGB_YC_HDTV_16_235	3
#define AMX_CSC_RGB_YC_HDTV_0_255	4
#define AMX_CSC_RGB_YC_JFIF_0_255	5
#define AMX_CSC_RGB_YC_SMPTE_170M	6
#define AMX_CSC_RGB_YC_SMPTE_240M	7
#define AMX_CSC_RGB_BYPASS		8
#define AMX_CSC_YC_RGB_SDTV_16_235	9
#define AMX_CSC_YC_RGB_SDTV_0_255	10
#define AMX_CSC_YC_RGB_HDTV_16_235	11
#define AMX_CSC_YC_RGB_HDTV_0_255	12
#define AMX_CSC_YC_RGB_JFIF_0_255	13
#define AMX_CSC_YC_RGB_SMPTE_170M	14
#define AMX_CSC_YC_RGB_SMPTE_240M	15
#define AMX_CSC_YC_BYPASS		16

/* AMX CSC table (WM8650 only) */
#define AMX_CSC_SDTV_16_235	AMX_CSC_RGB_YC_SDTV_16_235
#define AMX_CSC_SDTV_0_255	AMX_CSC_RGB_YC_SDTV_0_255
#define AMX_CSC_HDTV_16_235	AMX_CSC_RGB_YC_HDTV_16_235
#define AMX_CSC_HDTV_0_255	AMX_CSC_RGB_YC_HDTV_0_255
#define AMX_CSC_JFIF_0_255	AMX_CSC_RGB_YC_JFIF_0_255
#define AMX_CSC_SMPTE_170M	AMX_CSC_RGB_YC_SMPTE_170M
#define AMX_CSC_SMPTE_240M	AMX_CSC_RGB_YC_SMPTE_240M

/* GE CSC table */
#define GE_CSC_DEFAULT			0
#define GE_CSC_RGB_YC_SDTV_16_235	1
#define GE_CSC_RGB_YC_SDTV_0_255	2
#define GE_CSC_RGB_YC_HDTV_16_235	3
#define GE_CSC_RGB_YC_HDTV_0_255	4
#define GE_CSC_RGB_YC_JFIF_0_255	5
#define GE_CSC_RGB_YC_SMPTE_170M	6
#define GE_CSC_RGB_YC_SMPTE_240M	7
#define GE_CSC_RGB_BYPASS		8
#define GE_CSC_YC_RGB_SDTV_16_235	9
#define GE_CSC_YC_RGB_SDTV_0_255	10
#define GE_CSC_YC_RGB_HDTV_16_235	11
#define GE_CSC_YC_RGB_HDTV_0_255	12
#define GE_CSC_YC_RGB_JFIF_0_255	13
#define GE_CSC_YC_RGB_SMPTE_170M	14
#define GE_CSC_YC_RGB_SMPTE_240M	15
#define GE_CSC_YC_BYPASS		16

#define GSCAPS_NONE	0x00000000
#define GSCAPS_GMP	0x00000001
#define GSCAPS_FB	0x00000002
#define GSCAPS_MB	0x00000004
#define GSCAPS_OS	0x00000008

typedef struct ge_regs_8850 ge_regs_t;

typedef struct {
	void *mmio;
	unsigned int fd;
	unsigned int id;
	unsigned int config;
	unsigned int data;
	unsigned int loglevel;
	struct gmp *gmp;
	struct ge_plugin *plugin;
	void *priv;
} ge_info_t;

typedef struct {
	unsigned long addr;
	unsigned long xres;
	unsigned long yres;
	unsigned long xres_virtual;
	unsigned long yres_virtual;
	unsigned long x;
	unsigned long y;
	unsigned long pixelformat;
	unsigned long planesize[4];
	unsigned long color;
	unsigned long blitting_flags;
	unsigned long src_blend_func;
	unsigned long dst_blend_func;
	unsigned long src_color_key;
	unsigned long dst_color_key;
	unsigned long rop;
	unsigned long mmio;
	unsigned long caps;
} ge_surface_t;

typedef struct {
	unsigned long x;
	unsigned long y;
	unsigned long w;
	unsigned long h;
} ge_rect_t;

#ifdef __KERNEL__
extern wait_queue_head_t ge_wq;
#endif

extern ge_info_t *create_ge_info(void);
extern void release_ge_info(ge_info_t *geinfo);
extern int ge_sys_init(ge_info_t **geinfo, void *priv); /* deprecated */
extern int ge_sys_exit(ge_info_t *geinfo, void *priv);  /* deprecated */
extern int ge_get_chip_id(ge_info_t *geinfo, unsigned int *chip_id);
extern int ge_lock(ge_info_t *geinfo);
extern int ge_unlock(ge_info_t *geinfo);
extern int ge_trylock(ge_info_t *geinfo);
extern void ge_wait_sync(ge_info_t *geinfo);
extern void WAIT_PXD_INT(void);

extern int ge_set_src_pixelformat(ge_info_t *geinfo, unsigned int pixelformat);
extern int ge_set_dst_pixelformat(ge_info_t *geinfo, unsigned int pixelformat);
extern int ge_set_destination(ge_info_t *geinfo, ge_surface_t *dst);
extern int ge_set_source(ge_info_t *geinfo, ge_surface_t *src);
extern int ge_set_mask(ge_info_t *geinfo, ge_surface_t *src);
extern int ge_set_command(ge_info_t *geinfo, unsigned int cmd,
	unsigned int rop);
extern unsigned int ge_get_color(ge_info_t *geinfo,
	unsigned int r, unsigned int g, unsigned int b, unsigned int a,
	unsigned int pixfmt);
extern void ge_set_color(ge_info_t *geinfo,
	unsigned int r, unsigned int g, unsigned int b, unsigned int a,
	unsigned int pixfmt);
extern void ge_set_sck(ge_info_t *geinfo,
	unsigned int r, unsigned int g, unsigned int b, unsigned int a,
	unsigned int pixfmt);
extern void ge_set_dck(ge_info_t *geinfo,
	unsigned int r, unsigned int g, unsigned int b, unsigned int a,
	unsigned int pixfmt);
extern int ge_cmd_blit(ge_info_t *geinfo);
extern int ge_cmd_fillrect(ge_info_t *geinfo);
extern int ge_cmd_rotate(ge_info_t *geinfo, unsigned int arc);
extern int ge_cmd_mirror(ge_info_t *geinfo, int mode);

extern int amx_show_surface(ge_info_t *geinfo, int id, ge_surface_t *s,
	int x, int y);
extern int amx_get_surface(ge_info_t *geinfo, int id, ge_surface_t *s);
extern int amx_hide_surface(ge_info_t *geinfo, int id);
extern int amx_set_colorkey(ge_info_t *geinfo, int id, int enable,
	unsigned int r, unsigned int g, unsigned int b, unsigned int pixfmt);
extern int amx_set_alpha(ge_info_t *geinfo, int id, unsigned int alpha);
extern int amx_enable_pixel_alpha_8650(ge_info_t *geinfo, int g1_en, int g2_en);
extern int amx_enable_pixel_alpha_8710(ge_info_t *geinfo, int g1_en, int g2_en);
extern int amx_enable_pixel_alpha(ge_info_t *geinfo, int g1_en, int g2_en);
extern int amx_sync(ge_info_t *geinfo);

/* WM8510 */
extern int amx_set_alpha_8510_eco2(ge_info_t *geinfo, unsigned int alpha);
extern int amx_set_alpha_8435(ge_info_t *geinfo, unsigned int alpha);
extern int amx_set_csc(ge_info_t *geinfo, int table_id);

/* WM8710 B0 */
/*
extern int amx_get_y_surface(ge_info_t *geinfo, int id, ge_surface_t *s);
extern int amx_get_c_surface(ge_info_t *geinfo, int id, ge_surface_t *s);
extern int amx_set_y_surface(ge_info_t *geinfo, int id, ge_surface_t *s);
extern int amx_set_c_surface(ge_info_t *geinfo, int id, ge_surface_t *s);
*/

/* WM3481 */
int ge_set_govr_source(ge_info_t *geinfo, ge_surface_t *src, int x, int y);
int ge_set_govr_destination(ge_info_t *geinfo, ge_surface_t *des, int x, int y);
int ge_set_govr_size(ge_info_t *geinfo, int w, int h);
int ge_set_direct_write(ge_info_t *geinfo, ge_surface_t *dw);
void ge_set_govr_fixed_alpha(ge_info_t *geinfo, int src_hit, int des_hit,
	int src_alpha, int des_alpha);

#endif /* GE_REGS_H */

