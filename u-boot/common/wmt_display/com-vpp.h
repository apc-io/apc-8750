/*++ 
 * linux/drivers/video/wmt/com-vpp.h
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

/*
 * ChangeLog
 *
 * 2010-08-05  Sam Shen
 *     * Add License declaration and ChangeLog
 */

#ifndef COM_VPP_H
#define COM_VPP_H

#ifdef CFG_LOADER 
	#include "com-video.h"

	#define U32 unsigned int
	#define U16 unsigned short
	#define U8 unsigned char
#elif defined __KERNEL__
    #include <linux/types.h>
    #include <linux/fb.h>
    #include <asm/io.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
    #include <mach/common_def.h>
    #include <mach/com-video.h>
#else
    #include <asm/arch/common_def.h>
    #include <asm/arch/com-video.h>
#endif
#else
    #include "com-video.h"
    #define REG32_VAL(addr) (*(volatile unsigned int *)(addr))
    #define REG16_VAL(addr) (*(volatile unsigned short *)(addr))
    #define REG8_VAL(addr)  (*(volatile unsigned char *)(addr))
    
    #define U32 unsigned int
    #define U16 unsigned short
    #define U8 unsigned char
#endif

#define VPP_OLD_API

#define VPP_NEW_FBUF_MANAGER

#define VPP_AHB_CLK				250000000

#ifdef CONFIG_MAX_RESX
	#define VPP_HD_MAX_RESX		CONFIG_MAX_RESX
#else
	#define VPP_HD_MAX_RESX		1920
#endif

#ifdef CONFIG_MAX_RESY
	#define VPP_HD_MAX_RESY		CONFIG_MAX_RESY
#else
	#define VPP_HD_MAX_RESY		1200
#endif

#ifdef CONFIG_DEFAULT_RESX
	#define VPP_HD_DISP_RESX	CONFIG_DEFAULT_RESX
#else
	#define VPP_HD_DISP_RESX		1024
#endif	

#ifdef CONFIG_DEFAULT_RESY
	#define VPP_HD_DISP_RESY	CONFIG_DEFAULT_RESY
#else
	#define VPP_HD_DISP_RESY		768
#endif

#ifdef CONFIG_DEFAULT_FPS
	#define VPP_HD_DISP_FPS		CONFIG_DEFAULT_FPS
#else
	#define VPP_HD_DISP_FPS			60
#endif

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

#define VPP_YUV_BLACK		0x00,0x80,0x80	//Y, Cr, Cb
#define VPP_YUV_WHITE		0xff,0x80,0x80
#define VPP_YUV_RED			0x51,0xf0,0x5a
#define VPP_YUV_GREEN		0x90,0x22,0x35
#define VPP_YUV_BLUE		0x28,0x6d,0xf0
#define VPP_RGB32_BLACK		0x00,0x00,0x00,0x00

#define VPP_COL_RGB32_BLACK	0x000000
#define VPP_COL_RGB32_WHITE	0xFFFFFF
#define VPP_COL_RGB32_RED	0xFF0000
#define VPP_COL_RGB32_GREEN	0x00FF00
#define VPP_COL_RGB32_BLUE	0x0000FF

#define VPP_COL_BLACK		0x008080	//Y, Cr, Cb
#define VPP_COL_WHITE		0xff8080
#define VPP_COL_RED			0x41d464
#define VPP_COL_GREEN		0x902235
#define VPP_COL_BLUE		0x2372d4

#define VPP_MAGNUM(s, e) ((2^((s)-(e)+1))-1)

typedef enum {
	VPP_FBUF_GOVW_1,
	VPP_FBUF_GOVW_2,
	VPP_FBUF_SCLW_1,
	VPP_FBUF_SCLR_1,
	VPP_FBUF_MAX
} vpp_fbuf_t;

typedef enum {
	VPP_FLAG_NULL = 0,
	VPP_FLAG_ENABLE = 1,
	VPP_FLAG_DISABLE = 0,
	VPP_FLAG_TRUE = 1,
	VPP_FLAG_FALSE = 0,
	VPP_FLAG_ZERO = 0,
	VPP_FLAG_ONE = 1,
	VPP_FLAG_SUCCESS = 1,
	VPP_FLAG_ERROR = 0,
	VPP_FLAG_RD = 1,
	VPP_FLAG_WR = 0,
} vpp_flag_t;

typedef enum {
	VPP_MOD_GOVRS,
	VPP_MOD_GOVRH,
	VPP_MOD_DISP,
	VPP_MOD_GOVW,	
	VPP_MOD_GOVM,	
	VPP_MOD_SCL,
	VPP_MOD_SCLW,
	VPP_MOD_VPU,
	VPP_MOD_VPUW,	
	VPP_MOD_PIP,	
	VPP_MOD_VPPM,
	VPP_MOD_LCDC,
	VPP_MOD_CURSOR,
	VPP_MOD_MAX
} vpp_mod_t;

typedef enum {
	VPP_VOUT_SDA = 0,
	VPP_VOUT_SDD = 1,
	VPP_VOUT_LCD = 2,
	VPP_VOUT_DVI = 3,
	VPP_VOUT_HDMI = 4,
	VPP_VOUT_DVO2HDMI = 5,
	VPP_VOUT_LVDS = 6,
	VPP_VOUT_VGA = 7,
	VPP_VOUT_MAX
} vpp_vout_t;

typedef enum {
	VPP_OUTDEV_TV_NORMAL,	//NTSC 720x480 or PAL 720x576
	VPP_OUTDEV_VGA,
	VPP_OUTDEV_DVO,
	VPP_OUTDEV_MAX
} vpp_output_device_t;

typedef enum {
	VPP_ALPHA_VIDEO,
	VPP_ALPHA_GE,
	VPP_ALPHA_MAX,
} vpp_alpha_t;

typedef enum {
	VPP_DISP_FMT_FRAME,	//Progressive
	VPP_DISP_FMT_FIELD,	//Interlace
	VPP_DISP_FMT_MAX,
} vpp_display_format_t;

typedef enum {
	VPP_MEDIA_FMT_MPEG,
	VPP_MEDIA_FMT_H264,
	VPP_MEDIA_FMT_MAX,
} vpp_media_format_t;

typedef enum {
	VPP_PATH_NULL = 0,
	VPP_PATH_ALL = 0xffffffff,
	//in
	VPP_PATH_GOVM_IN_VPU = BIT0,
	VPP_PATH_GOVM_IN_SCL = BIT1,
	VPP_PATH_GOVM_IN_GE = BIT2,
	VPP_PATH_GOVM_IN_PIP = BIT3,
	VPP_PATH_GOVM_IN_SPU = BIT4,
	VPP_PATH_GOVM_IN_CUR = BIT5,
	//out
	VPP_PATH_SCL_OUT_REALTIME = BIT10,
	VPP_PATH_SCL_OUT_RECURSIVE = BIT11,
} vpp_path_t;

typedef enum {			/* don't change this order */
	VPP_CSC_YUV2RGB2_MIN,
	VPP_CSC_YUV2RGB_SDTV_0_255 = VPP_CSC_YUV2RGB2_MIN,
	VPP_CSC_YUV2RGB_SDTV_16_235,
	VPP_CSC_YUV2RGB_HDTV_0_255,
	VPP_CSC_YUV2RGB_HDTV_16_235,
	VPP_CSC_YUV2RGB_JFIF_0_255,
	VPP_CSC_YUV2RGB_SMPTE170M,
	VPP_CSC_YUV2RGB_SMPTE240M,	
	VPP_CSC_RGB2YUV_MIN,
	VPP_CSC_RGB2YUV_SDTV_0_255 = VPP_CSC_RGB2YUV_MIN,
	VPP_CSC_RGB2YUV_SDTV_16_235,
	VPP_CSC_RGB2YUV_HDTV_0_255,
	VPP_CSC_RGB2YUV_HDTV_16_235,
	VPP_CSC_RGB2YUV_JFIF_0_255,
	VPP_CSC_RGB2YUV_SMPTE170M,
	VPP_CSC_RGB2YUV_SMPTE240M,	
	VPP_CSC_MAX,
	VPP_CSC_BYPASS
} vpp_csc_t;

typedef enum {
	VPP_REG_LEVEL_1,
	VPP_REG_LEVEL_2,
	VPP_REG_LEVEL_MAX,
} vpp_reglevel_t;

typedef enum {
	VPP_BPP_1,
	VPP_BPP_2,
	VPP_BPP_4,
	VPP_BPP_8,
	VPP_BPP_12,
	VPP_BPP_16_565,
	VPP_BPP_16_555 = VPP_BPP_16_565,
	VPP_BPP_18,
	VPP_BPP_24,
	VPP_BPP_MAX,
} vpp_bpp_t;

typedef enum {
	VPP_GAMMA_DISABLE,
	VPP_GAMMA_22,
	VPP_GAMMA_24,
	VPP_GAMMA_28,
	VPP_GAMMA_MAX,
} vpp_gamma_t;

typedef enum {
	VPP_DATAWIDHT_12,
	VPP_DATAWIDHT_24,
	VPP_DATAWIDHT_MAX,
} vpp_datawidht_t;

typedef enum {
	VPP_ADDR_NORMAL,
	VPP_ADDR_BURST,
	VPP_ADDR_MAX,
} vpp_addr_mode_t;

typedef enum {
	VPP_DEI_WEAVE,
	VPP_DEI_BOB,
	VPP_DEI_ADAPTIVE_ONE,
	VPP_DEI_ADAPTIVE_THREE,
	VPP_DEI_FIELD,
	VPP_DEI_MOTION_VECTOR,
	VPP_DEI_MAX,
	VPP_DEI_DYNAMIC,
} vpp_deinterlace_t;

typedef enum {
	VPP_DIR_THERE,
	VPP_DIR_TWO,
	VPP_DIR_FOUR,
	VPP_DIR_FIVE,
	VPP_DIR_MAX,
} vpp_direction_t;

typedef enum {
	VPP_FIELD_TOP,
	VPP_FIELD_BOTTOM,
	VPP_FIELD_MAX,
} vpp_field_t;

typedef enum {
	VPP_TVSYS_NTSC,
	VPP_TVSYS_NTSCJ,
	VPP_TVSYS_NTSC443,
	VPP_TVSYS_PAL,
	VPP_TVSYS_PALM,
	VPP_TVSYS_PAL60,
	VPP_TVSYS_PALN,
	VPP_TVSYS_PALNC,
	VPP_TVSYS_720P,
	VPP_TVSYS_1080I,
	VPP_TVSYS_1080P,
	VPP_TVSYS_MAX
} vpp_tvsys_t;

typedef enum {
	VPP_TVCONN_YCBCR,
	VPP_TVCONN_SCART,
	VPP_TVCONN_YPBPR,
	VPP_TVCONN_VGA,
	VPP_TVCONN_SVIDEO,
	VPP_TVCONN_CVBS,
	VPP_TVCONN_MAX
} vpp_tvconn_t;

#define VPP_OPT_INTERLACE			0x01
#define VPP_VGA_HSYNC_POLAR_HI		0x02
#define VPP_VGA_VSYNC_POLAR_HI		0x04
#define VPP_DVO_SYNC_POLAR_HI		0x08
#define VPP_DVO_VSYNC_POLAR_HI		0x10
#define VPP_OPT_HSCALE_UP			0x20
#define VPP_OPT_FPS_MASK			0xFF000000
#define VPP_OPT_HDMI_VIC_MASK		0x00FF0000

#define VPP_GET_OPT_FPS(option)		((option & VPP_OPT_FPS_MASK) >> 24)
#define VPP_SET_OPT_FPS(option,fps)	((option & ~VPP_OPT_FPS_MASK) | (fps << 24))
#define VPP_OPT_FPS_VAL(fps)		(fps << 24)

#define VPP_GET_OPT_HDMI_VIC(option)		((option & VPP_OPT_HDMI_VIC_MASK) >> 16)
#define VPP_SET_OPT_HDMI_VIC(option,vic)	((option & ~VPP_OPT_HDMI_VIC_MASK) | (vic << 16))
#define VPP_OPT_HDMI_VIC_VAL(vic)			(vic << 16)

typedef struct {
	unsigned int pixel_clock;	
	unsigned int option;

	unsigned int hsync;
	unsigned int hbp;
	unsigned int hpixel;
	unsigned int hfp;

	unsigned int vsync;
	unsigned int vbp;
	unsigned int vpixel;
	unsigned int vfp;
} vpp_timing_t;

typedef struct {
	int read_cycle;
	
	unsigned int total_pixel_of_line;
	unsigned int begin_pixel_of_active;
	unsigned int end_pixel_of_active;
	
	unsigned int total_line_of_frame;
	unsigned int begin_line_of_active;
	unsigned int end_line_of_active;

	unsigned int hsync;
	unsigned int vsync;
	unsigned int line_number_between_VBIS_VBIE;
	unsigned int line_number_between_PVBI_VBIS;
} vpp_clock_t;

typedef struct _vpp_image_t {
	U32 y_addr;
	U32 c_addr;
	vdo_color_fmt src_col_fmt;
} vpp_image_t;

typedef struct {
	vdo_framebuf_t src_fb;
	vdo_framebuf_t dst_fb;
} vpp_scale_t;

#define VPP_VOUT_STS_REGISTER	0x01
#define VPP_VOUT_STS_ACTIVE		0x02
#define VPP_VOUT_STS_PLUGIN		0x04
#define VPP_VOUT_STS_EDID		0x08
#define VPP_VOUT_STS_BLANK		0x10
#define VPP_VOUT_STS_POWERDN	0x20
#define VPP_VOUT_STS_CONTENT_PROTECT 0x40
typedef struct {
	int num;
	unsigned int status;
	char name[10];
} vpp_vout_info_t;

typedef struct {
	int num;
	int arg;
} vpp_vout_parm_t;

/* 	=============================================================================
	vout option
	=============================================================================
	vout		id 		option1						option2
	SDA			0		tvsys						tvconn
	SDD			1		tvsys						tvconn
	LCD			2		lcd id						bit per pixel
	DVI			3		colfmt						b0(0:12bit,1:24bit),b1(1:interlace)
	HDMI		4		colfmt						b0(0:12bit,1:24bit),b1(1:interlace)
	DVO2HDMI	5		colfmt						b0(0:12bit,1:24bit),b1(1:interlace)
	LVDS		6		lcd id						bit per pixel
	VGA			7		
	=============================================================================
	tvsys : 0-NTSC,3-PAL,8-720p,9-1080i,10-1080p
	tvconn: 0-YCbCr,1-SCART,2-YPbPr,3-VGA,4-SVideo,5-CVBS
	colfmt: 0-420,1-422H,3-444,6-ARGB,8-RGB24,9-RGB18,10-RGB16
	lcdid :	0-OEM(VGA 1024x768),1-Chilin(7"800x480),2-Innolux(7"800x480),3-AUO(800x600),
			4-Eking(800x600),5-Hannstar(10"1024x600)
	============================================================================== */
#define VOUT_OPT_DWIDTH24	BIT(0)
#define VOUT_OPT_INTERLACE	BIT(1)

typedef struct {
	int num;
	unsigned int option[3];
} vpp_vout_option_t;

typedef struct {
	int num;
	vpp_timing_t tmr;
} vpp_vout_tmr_t;

typedef struct {
	unsigned int yaddr;
	unsigned int caddr;
} vpp_fbaddr_t;

typedef struct {
	unsigned int chip_id;
	unsigned int version;
	unsigned int resx_max;
	unsigned int resy_max;
	unsigned int pixel_clk;
	unsigned int module;
	unsigned int option;
} vpp_cap_t;

typedef struct {
	vpp_flag_t enable;
	vpp_alpha_t mode;
	int A;
	int B;
} vpp_alpha_parm_t;

typedef struct {
	vpp_path_t src_path;
	vpp_flag_t enable;
} vpp_src_path_t;

typedef struct {
	vpp_flag_t enable;
	vpp_flag_t sync_polar;
	vpp_flag_t vsync_polar;
	vdo_color_fmt color_fmt;
	vpp_datawidht_t data_w;
	int clk_inv;
	int clk_delay;
} vdo_dvo_parm_t;

typedef struct {
	unsigned int addr;
	unsigned int index;
	unsigned int val;
} vpp_i2c_t;

#define VPP_FLAG_DISPFB_ADDR	BIT(0)
#define VPP_FLAG_DISPFB_INFO	BIT(1)
#define VPP_FLAG_DISPFB_VIEW	BIT(2)
#define VPP_FLAG_DISPFB_PIP		BIT(3)
#define VPP_FLAG_DISPFB_MB_ONE	BIT(4)
#define VPP_FLAG_DISPFB_SCL		BIT(5)
#define VPP_FLAG_DISPFB_MB_NO	BIT(6)
#define VPP_FLAG_DISPFB_MB_GOVR BIT(7)
typedef struct {
	unsigned int yaddr;
	unsigned int caddr;
	vdo_framebuf_t info;
	vdo_view_t view;

	unsigned int flag;
} vpp_dispfb_t;

typedef struct {
	vpp_mod_t mod;
	int read;
	unsigned int arg1;
	unsigned int arg2;
} vpp_mod_arg_t;

typedef struct {
	vpp_mod_t mod;
	int read;	
	vpp_timing_t tmr;
} vpp_mod_timing_t;

typedef struct {
	vpp_mod_t mod;
	int read;	
	vdo_framebuf_t fb;
} vpp_mod_fbinfo_t;

typedef struct {
	vpp_mod_t mod;
	int read;
	vdo_view_t view;
} vpp_mod_view_t;

typedef struct {
	char pts[8];
} vpp_pts_t;

typedef struct {
	unsigned int resx;
	unsigned int resy;
	unsigned int fps;
	unsigned int option;
} vpp_vmode_parm_t;

#define VPP_VOUT_VMODE_NUM	20
typedef struct {
	vpp_vout_t mode;
	int num;
	vpp_vmode_parm_t parm[VPP_VOUT_VMODE_NUM];
} vpp_vout_vmode_t;

typedef struct {
	int queue_cnt;
	int cur_cnt;
	int isr_cnt;
	int disp_cnt;
	int skip_cnt;
	int full_cnt;
	int reserved[2];
} vpp_dispfb_info_t;

typedef struct {
	vpp_vout_t mode;
	int size;
	char *buf;
} vpp_vout_edid_t;

typedef struct {
	int num;
	unsigned int bksv[2];
} vpp_vout_cp_info_t;

#define VPP_VOUT_CP_NUM		336
typedef struct {
	char key[VPP_VOUT_CP_NUM];
} vpp_vout_cp_key_t;

typedef enum {
	VPP_VPATH_GOVW,		// VPU,GE fb --> VPU,GE --> GOVW --> GOVW fb --> GOVR fb --> GOVR
	VPP_VPATH_GOVR,		// VPU fb --> GOVR fb --> GOVR
	VPP_VPATH_GE,		// GE fb --> GOVR fb --> GOVR
	VPP_VPATH_VPU,		// VPU fb --> VPU --> GOVR
	VPP_VPATH_SCL,		// VPU fb --> SCL --> SCL fb --> GOVR fb --> GOVR
	VPP_VPATH_AUTO,
	VPP_VPATH_GOVW_SCL,	// VPU,GE fb--> VPU,GE --> GOVW --> GOVW fb --> SCL --> SCL fb --> GOVR fb --> GOVR
	VPP_VPATH_MAX
} vpp_video_path_t;

#define VPPIO_MAGIC		'f'

/* VPP common ioctl command */
#define VPPIO_VPP_BASE				0x0
#define VPPIO_VPPGET_INFO			_IOR(VPPIO_MAGIC,VPPIO_VPP_BASE+0,vpp_cap_t)
#define VPPIO_VPPSET_INFO			_IOW(VPPIO_MAGIC,VPPIO_VPP_BASE+0,vpp_cap_t)
#define VPPIO_I2CSET_BYTE			_IOW(VPPIO_MAGIC,VPPIO_VPP_BASE+1,vpp_i2c_t)
#define VPPIO_I2CGET_BYTE			_IOR(VPPIO_MAGIC,VPPIO_VPP_BASE+1,vpp_i2c_t)
#define VPPIO_MODULE_FRAMERATE		_IOWR(VPPIO_MAGIC,VPPIO_VPP_BASE+2,vpp_mod_arg_t)
#define VPPIO_MODULE_ENABLE			_IOWR(VPPIO_MAGIC,VPPIO_VPP_BASE+3,vpp_mod_arg_t)
#define VPPIO_MODULE_TIMING			_IOWR(VPPIO_MAGIC,VPPIO_VPP_BASE+4,vpp_mod_timing_t)
#define VPPIO_MODULE_FBADDR			_IOWR(VPPIO_MAGIC,VPPIO_VPP_BASE+5,vpp_mod_arg_t)
#define VPPIO_MODULE_FBINFO			_IOWR(VPPIO_MAGIC,VPPIO_VPP_BASE+6,vpp_mod_fbinfo_t)
#define VPPIO_VPPSET_DIRECTPATH		_IO(VPPIO_MAGIC,VPPIO_VPP_BASE+7)
#define VPPIO_VPPSET_FBDISP			_IOW(VPPIO_MAGIC,VPPIO_VPP_BASE+8,vpp_dispfb_t)
#define VPPIO_VPPGET_FBDISP			_IOW(VPPIO_MAGIC,VPPIO_VPP_BASE+8,vpp_dispfb_info_t)
#define VPPIO_WAIT_FRAME			_IO(VPPIO_MAGIC,VPPIO_VPP_BASE+9)
#define VPPIO_MODULE_VIEW			_IOWR(VPPIO_MAGIC,VPPIO_VPP_BASE+10,vpp_mod_view_t)
#define VPPIO_VPPGET_PTS			_IOR(VPPIO_MAGIC,VPPIO_VPP_BASE+11,vpp_pts_t)
#define VPPIO_VPPSET_PTS			_IOW(VPPIO_MAGIC,VPPIO_VPP_BASE+11,vpp_pts_t)
#define VPPIO_STREAM_ENABLE			_IO(VPPIO_MAGIC,VPPIO_VPP_BASE+12)
#define VPPIO_STREAM_GETFB			_IOR(VPPIO_MAGIC,VPPIO_VPP_BASE+13,vdo_framebuf_t)
#define VPPIO_STREAM_PUTFB			_IOW(VPPIO_MAGIC,VPPIO_VPP_BASE+13,vdo_framebuf_t)
#define VPPIO_MODULE_CSC			_IOWR(VPPIO_MAGIC,VPPIO_VPP_BASE+14,vpp_mod_arg_t)

/* VOUT ioctl command */
#define VPPIO_VOUT_BASE				0x10
#define VPPIO_VOGET_INFO			_IOR(VPPIO_MAGIC,VPPIO_VOUT_BASE+0,vpp_vout_info_t)
#define VPPIO_VOSET_MODE			_IOW(VPPIO_MAGIC,VPPIO_VOUT_BASE+1,vpp_vout_parm_t)
#define VPPIO_VOSET_BLANK			_IOW(VPPIO_MAGIC,VPPIO_VOUT_BASE+2,vpp_vout_parm_t)
#define VPPIO_VOSET_DACSENSE		_IOW(VPPIO_MAGIC,VPPIO_VOUT_BASE+3,vpp_vout_parm_t)
#define VPPIO_VOSET_BRIGHTNESS		_IOW(VPPIO_MAGIC,VPPIO_VOUT_BASE+4,vpp_vout_parm_t)
#define VPPIO_VOGET_BRIGHTNESS		_IOR(VPPIO_MAGIC,VPPIO_VOUT_BASE+4,vpp_vout_parm_t)
#define VPPIO_VOSET_CONTRAST		_IOW(VPPIO_MAGIC,VPPIO_VOUT_BASE+5,vpp_vout_parm_t)
#define VPPIO_VOGET_CONTRAST		_IOR(VPPIO_MAGIC,VPPIO_VOUT_BASE+5,vpp_vout_parm_t)
#define VPPIO_VOSET_OPTION			_IOW(VPPIO_MAGIC,VPPIO_VOUT_BASE+6,vpp_vout_option_t)
#define VPPIO_VOGET_OPTION			_IOR(VPPIO_MAGIC,VPPIO_VOUT_BASE+6,vpp_vout_option_t)
#define VPPIO_VOUT_VMODE			_IOWR(VPPIO_MAGIC,VPPIO_VOUT_BASE+7,vpp_vout_vmode_t)
#define VPPIO_VOGET_EDID			_IOR(VPPIO_MAGIC,VPPIO_VOUT_BASE+8,vpp_vout_edid_t)
#define VPPIO_VOGET_CP_INFO			_IOR(VPPIO_MAGIC,VPPIO_VOUT_BASE+9,vpp_vout_cp_info_t)
#define VPPIO_VOSET_CP_KEY			_IOW(VPPIO_MAGIC,VPPIO_VOUT_BASE+10,vpp_vout_cp_key_t)
#define VPPIO_VOSET_AUDIO_PASSTHRU	_IO(VPPIO_MAGIC,VPPIO_VOUT_BASE+11)

/* GOVR ioctl command */
#define VPPIO_GOVR_BASE				0x20
#define VPPIO_GOVRSET_DVO			_IOW(VPPIO_MAGIC,VPPIO_GOVR_BASE+0,vdo_dvo_parm_t)
#define VPPIO_GOVRSET_CUR_COLKEY	_IOW(VPPIO_MAGIC,VPPIO_GOVR_BASE+1,vpp_mod_arg_t)
#define VPPIO_GOVRSET_CUR_HOTSPOT	_IOW(VPPIO_MAGIC,VPPIO_GOVR_BASE+2,vpp_mod_arg_t)

/* GOVW ioctl command */
#define VPPIO_GOVW_BASE				0x30
#define VPPIO_GOVW_ENABLE			_IO(VPPIO_MAGIC,VPPIO_GOVW_BASE+0)

/* GOVM ioctl command */		
#define VPPIO_GOVM_BASE				0x40
#define VPPIO_GOVMSET_SRCPATH		_IOW(VPPIO_MAGIC,VPPIO_GOVM_BASE+0,vpp_src_path_t)
#define VPPIO_GOVMGET_SRCPATH		_IO(VPPIO_MAGIC,VPPIO_GOVM_BASE+0)
#define VPPIO_GOVMSET_ALPHA			_IOW(VPPIO_MAGIC,VPPIO_GOVM_BASE+1,vpp_alpha_parm_t)
#define VPPIO_GOVMSET_GAMMA			_IO(VPPIO_MAGIC,VPPIO_GOVM_BASE+2)
#define VPPIO_GOVMSET_CLAMPING 		_IO(VPPIO_MAGIC,VPPIO_GOVM_BASE+3)

/* VPU ioctl command */
#define VPPIO_VPU_BASE				0x50
#define VPPIO_VPUSET_VIEW			_IOW(VPPIO_MAGIC,VPPIO_VPU_BASE+0,vdo_view_t)
#define VPPIO_VPUGET_VIEW			_IOR(VPPIO_MAGIC,VPPIO_VPU_BASE+0,vdo_view_t)
#define VPPIO_VPUSET_FBDISP			_IOW(VPPIO_MAGIC,VPPIO_VPU_BASE+1,vpp_dispfb_t)
#define VPPIO_VPU_CLR_FBDISP		_IO(VPPIO_MAGIC,VPPIO_VPU_BASE+2)

/* SCL ioctl command */
#define VPPIO_SCL_BASE				0x60
#define VPPIO_SCL_SCALE				_IOWR(VPPIO_MAGIC,VPPIO_SCL_BASE+0,vpp_scale_t)
#define VPPIO_SCL_DROP_LINE_ENABLE  _IO(VPPIO_MAGIC,VPPIO_SCL_BASE+1)
#define VPPIO_SCL_SCALE_ASYNC		_IOWR(VPPIO_MAGIC,VPPIO_SCL_BASE+2,vpp_scale_t)
#define VPPIO_SCL_SCALE_FINISH		_IO(VPPIO_MAGIC,VPPIO_SCL_BASE+3)

#define VPPIO_MAX					0x70
#endif //COM_VPP_H
