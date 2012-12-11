/*++ 
 * linux/drivers/video/wmt/vpp.h
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

#include "vpp-osif.h"
#include "./hw/wmt-vpp-hw.h"
#include "com-vpp.h"
#include "vout.h"

#ifdef CFG_LOADER
	#ifdef __KERNEL__
	#undef __KERNEL__
	#endif
#endif

#ifndef VPP_H
#define VPP_H

/* VPP feature config */
/* 0 - right edge black edge, 1 - scale more to cut right edge for fit visual window */
// #define CONFIG_VPP_SCALEDWN_FIT_RIGHT_EDGE		// WM8510 don't support bcz SCLR and GOVW width should equal
#define CONFIG_VPP_INTERRUPT
#define CONFIG_VPP_DUAL_BUFFER
#define CONFIG_VPP_FIX_BUFFER
// #define CONFIG_VPP_ALLOC_VPU_FB
#define CONFIG_VPP_GOVW_TG_ERR_DROP_FRAME		// drop display frame when govw tg error (bandwidth not enough)
// #define CONFIG_GOVW_FPS_AUTO_ADJUST
// #define CONFIG_GOVW_FBSWAP_VBIE
#ifdef VPU_DEI_ENABLE
#define CONFIG_VPP_DYNAMIC_DEI
#endif
//#define CONFIG_VPP_VBIE_FREE_MB
#define CONFIG_VPP_GOVRH_FBSYNC				// direct path govrh fb swap sync with media fps
// #define CONFIG_VPP_DEMO					// HDMI EDID, CP disable
// #define CONFIG_VPP_PM					// dynamic power management
#define CONFIG_VPP_SCALE_TABLE_FREE			// bilinear scale mode by table free
#define CONFIG_VPP_DISPFB_FREE_POSTPONE		// free display fb in next vsync
#define CONFIG_VPP_PATH_SWITCH_SCL_GOVR		// dynamic switch scl or govr by vpu view
#define CONFIG_VPP_STREAM_CAPTURE			// stream capture current video display
#define CONFIG_VPP_GOVW_FRAMEBUF			// refine govw frame buffer swap

#define CONFIG_VPP_GE_DIRECT_PATH
// #define CONFIG_VPU_DIRECT_PATH
#define CONFIG_SCL_DIRECT_PATH
#ifdef CONFIG_SCL_DIRECT_PATH
#define CONFIG_SCL_DIRECT_PATH_DEBUG
#define VPP_MB_ALLOC_NUM 3
#else
#define VPP_MB_ALLOC_NUM 2
#endif
//#define CONFIG_GOVW_SCL_PATH

// VPP constant define
#ifdef CONFIG_VPP_FIX_BUFFER
#define VPP_GOVW_FB_COLFMT				VDO_COL_FMT_YUV422H
#else
#define VPP_GOVW_FB_COLFMT				VDO_COL_FMT_YUV444
#endif
#define VPP_DAC_SENSE_SECOND			5
#define VPP_VOUT_FRAMERATE_DEFAULT		60
#define GOVRH_DAC_SENSE_VALUE			0x42	// 0x55

#define VPP_SCALE_UP_RATIO_H			31
#define VPP_SCALE_DN_RATIO_H			32
#define VPP_SCALE_UP_RATIO_V			31
// #ifdef WMT_FTBLK_SCL_VSCL_32
#if 1
#define VPP_SCALE_DN_RATIO_V			32
#else
#define VPP_SCALE_DN_RATIO_V			16
#endif

#define VPP_GOV_MB_ALLOC_NUM			(VPP_MB_ALLOC_NUM*2)

typedef enum {
	VPP_INT_NULL = 0,
	VPP_INT_ALL = 0xffffffff,

	VPP_INT_GOVRH_PVBI = BIT0,
	VPP_INT_GOVRH_VBIS = BIT1,	//write done	
	VPP_INT_GOVRH_VBIE = BIT2,

	VPP_INT_GOVW_PVBI = BIT3,
	VPP_INT_GOVW_VBIS = BIT4,
	VPP_INT_GOVW_VBIE = BIT5,

	VPP_INT_DISP_PVBI = BIT6,
	VPP_INT_DISP_VBIS = BIT7,
	VPP_INT_DISP_VBIE = BIT8,

	VPP_INT_LCD_EOF = BIT9,

	VPP_INT_SCL_PVBI = BIT12,
	VPP_INT_SCL_VBIS = BIT13,
	VPP_INT_SCL_VBIE = BIT14,

	VPP_INT_VPU_PVBI = BIT15,
	VPP_INT_VPU_VBIS = BIT16,
	VPP_INT_VPU_VBIE = BIT17,

	VPP_INT_MAX = BIT31,

} vpp_int_t;

typedef enum {
	/* SCL */
	VPP_INT_ERR_SCL_TG = BIT0,
	VPP_INT_ERR_SCLR1_MIF = BIT1,
	VPP_INT_ERR_SCLR2_MIF = BIT2,
	VPP_INT_ERR_SCLW_MIFRGB = BIT3,
	VPP_INT_ERR_SCLW_MIFY = BIT4,
	VPP_INT_ERR_SCLW_MIFC = BIT5,

	/* VPU */
	VPP_INT_ERR_VPU_TG = BIT6,
	VPP_INT_ERR_VPUR1_MIF = BIT7,
	VPP_INT_ERR_VPUR2_MIF = BIT8,
	VPP_INT_ERR_VPUW_MIFRGB = BIT9,
	VPP_INT_ERR_VPUW_MIFY = BIT10,
	VPP_INT_ERR_VPUW_MIFC = BIT11,
	VPP_INT_ERR_VPU_MVR = BIT25,

	/* GOVW */
	VPP_INT_ERR_GOVM_VPU = BIT12,
	VPP_INT_ERR_GOVM_GE = BIT13,
	VPP_INT_ERR_GOVM_SPU = BIT14,
	VPP_INT_ERR_GOVM_PIP = BIT15,

	/* GOVW */
	VPP_INT_ERR_GOVW_TG = BIT16,	//def at govw
	VPP_INT_ERR_GOVW_MIFY = BIT17,	//def at govw
	VPP_INT_ERR_GOVW_MIFC = BIT18,	//def at govw

	/* GOVRS */
	VPP_INT_ERR_GOVRS_MIF = BIT19,	

	/* GOVRH */
	VPP_INT_ERR_GOVRH_MIF = BIT20,	

	/* PIP */
	VPP_INT_ERR_PIP_Y = BIT21,
	VPP_INT_ERR_PIP_C = BIT22,

	/* LCD */
	VPP_INT_ERR_LCD_UNDERRUN = BIT23,
	VPP_INT_ERR_LCD_OVERFLOW = BIT24,
} vpp_int_err_t;

// VPP FB capability flag
#define VPP_FB_FLAG_COLFMT		0xFFFF
#define VPP_FB_FLAG_SCALE		BIT(16)
#define VPP_FB_FLAG_CSC			BIT(17)
#define VPP_FB_FLAG_MEDIA		BIT(18)
#define VPP_FB_FLAG_FIELD		BIT(19)

typedef struct {
	vdo_framebuf_t fb;
	vpp_csc_t csc_mode;
	int	framerate;
	vpp_media_format_t media_fmt;
	int wait_ready;
	unsigned int capability;

	void (*set_framebuf)(vdo_framebuf_t *fb);
	void (*set_addr)(unsigned int yaddr,unsigned int caddr);
	void (*get_addr)(unsigned int *yaddr,unsigned int *caddr);
	void (*set_csc)(vpp_csc_t mode);
	vdo_color_fmt (*get_color_fmt)(void);
	void (*set_color_fmt)(vdo_color_fmt colfmt);
	void (*fn_view)(int read,vdo_view_t *view);
} vpp_fb_base_t;

#define VPP_MOD_BASE \
	vpp_mod_t mod; /* module id*/\
	unsigned int int_catch; /* interrupt catch */\
	vpp_fb_base_t *fb_p; /* framebuf base pointer */\
	unsigned int *reg_bk; /* register backup pointer */\
	void  (*init)(void *base); /* module initial */\
	void (*dump_reg)(void); /* dump hardware register */\
	void (*set_enable)(vpp_flag_t enable); /* module enable/disable */\
	void (*set_colorbar)(vpp_flag_t enable,int mode,int inv); /* hw colorbar enable/disable & mode */\
	void (*set_tg)(vpp_clock_t *tmr,unsigned int pixel_clock); /* set timing */\
	void (*get_tg)(vpp_clock_t *tmr); /* get timing */\
	unsigned int (*get_sts)(void); /* get interrupt or error status */\
	void (*clr_sts)(unsigned int sts); /* clear interrupt or error status */\
	void (*suspend)(int sts); /* module suspend */\
	void (*resume)(int sts) /* module resume */
/* End of vpp_mod_base_t */

typedef struct {
	VPP_MOD_BASE;
} vpp_mod_base_t;

#define VPP_MOD_FLAG_FRAMEBUF	BIT(0)

typedef enum {
	VPP_SCALE_MODE_REC_TABLE,	// old design but 1/32 limit
	VPP_SCALE_MODE_RECURSIVE,	// no recusive table, not smooth than bilinear mode
	VPP_SCALE_MODE_BILINEAR,	// more smooth but less than 1/2 will drop line
	VPP_SCALE_MODE_ADAPTIVE,	// scale down 1-1/2 bilinear mode, other recursive mode
	VPP_SCALE_MODE_MAX
} vpp_scale_mode_t;

typedef enum {
	VPP_HDMI_AUDIO_I2S,
	VPP_HDMI_AUDIO_SPDIF,
	VPP_HDMI_AUDIO_MAX
} vpp_hdmi_audio_inf_t;

#ifdef WMT_FTBLK_GOVM
#include "vppm.h"
#endif

#ifdef WMT_FTBLK_VPU
#include "vpu.h"
#endif

#include "lcd.h"

#ifndef CFG_LOADER
// #ifdef WMT_FTBLK_SCL
#include "scl.h"
// #endif
#endif
/*
#ifdef WMT_FTBLK_GE
#include "ge.h"
#endif
*/
#ifdef WMT_FTBLK_GOVM
#include "govm.h"
#endif
#ifdef WMT_FTBLK_GOVW
#include "govw.h"
#endif
#ifdef WMT_FTBLK_GOVRS
#include "govrs.h"
#endif
#ifdef WMT_FTBLK_GOVRH
#include "govrh.h"
#endif
#ifdef WMT_FTBLK_DISP
#include "disp.h"
#endif
#ifdef WMT_FTBLK_LCDC
#include "lcdc.h"
#endif
#ifdef WMT_FTBLK_LVDS
#include "lvds.h"
#endif
// #ifdef WMT_FTBLK_HDMI
#include "hdmi.h"
#ifndef CFG_LOADER
#include "cec.h"
#endif
// #endif
#ifdef CONFIG_WMT_EDID
#include "edid.h"
#endif

typedef enum {
	VPP_DBGLVL_DISABLE = 0x0,
	VPP_DBGLVL_SCALE = 1,
	VPP_DBGLVL_DISPFB = 2,
	VPP_DBGLVL_INT = 3,
	VPP_DBGLVL_TG = 4,
	VPP_DBGLVL_IOCTL = 5,
	VPP_DBGLVL_DIAG = 6,
	VPP_DBGLVL_DEI = 7,
	VPP_DBGLVL_SYNCFB = 8,
	VPP_DBGLVL_STREAM = 9,
	VPP_DBGLVL_ALL = 0xFF,
} vpp_dbg_level_t;

typedef struct {
	// internal parameter
	int vo_enable;
	int govrh_preinit;
	vpp_mod_base_t *govr;	// module pointer
	int chg_res_blank;
	int (*alloc_framebuf)(unsigned int resx,unsigned int resy);

	// vout
	int vga_enable;
	int vga_intsts;
	
	// hdmi
	int hdmi_video_mode;	// 0-auto,720,1080
	vpp_hdmi_audio_inf_t hdmi_audio_interface;	// 0-I2S, 1-SPDIF
	int hdmi_cp_enable;		// 0-off, 1-on
	int hdmi_audio_channel;
	int hdmi_audio_freq;
	unsigned int hdmi_ctrl;
	unsigned int hdmi_audio_pb1;
	unsigned int hdmi_audio_pb4;
	unsigned int hdmi_i2c_freq;
	unsigned int hdmi_i2c_udelay;
	int hdmi_init;	
	int (*cypher_func)(void *arg);
	unsigned int hdmi_bksv[2];
	char *hdmi_cp_p;
	int hdmi_3d_type;
	unsigned int hdmi_pixel_clock;
	int hdmi_certify_flag;

	// govrh
	int govrh_field;

	// govw
	int govw_skip_frame;
	int govw_skip_all;
	int govw_hfp;
	int govw_hbp;
	int govw_vfp;
	int govw_vbp;

	// video parameter
	int video_quality_mode;	// 1: quality mode, 0: performance mode (drop line)

	// scale parameter
	int vpu_skip_all;
	int scale_keep_ratio;

	// alloc frame buffer
	unsigned int mb[VPP_MB_ALLOC_NUM];
	unsigned int mb_y_size;
	unsigned int resx;
	unsigned int resy;
#ifdef CONFIG_VPP_GOVW_FRAMEBUF
	unsigned int mb_fb_size;
	unsigned int mb_govw[VPP_GOV_MB_ALLOC_NUM];
	unsigned int mb_govw_y_size;
	int mb_govw_cnt;
#endif

	// display framebuf queue
	int disp_fb_max;
	int disp_fb_cnt;
	int disp_fb_keep;
	int disp_fb_isr_cnt;			// count for disp fb isr
	int disp_fb_full_cnt;		// count for disp fb queue full
	int disp_cnt;				// count for vpu fb display
	int disp_skip_cnt;			// count for skip disp fb ( TG error or sw skip )
#ifdef WMT_FTBLK_PIP
	int pip_disp_cnt;			// count for pip fb display
#endif
#ifdef CONFIG_GOVW_SCL_PATH
	int vpu_fb_cnt;				// count for current vpu fb
	int vpu_fb_isr_cnt;			// count for vpu fb isr
	int vpu_fb_full_cnt;		// count for vpu fb queue full
	int vpu_cnt;				// count for vpu fb display
#endif

	// direct path
	vpp_video_path_t vpp_path;	
	vpp_video_path_t vpu_path;
	vpp_video_path_t ge_path;
	int vpp_path_no_ready;
	
	vdo_framebuf_t vpp_path_ori_fb;
	int ge_direct_init;

	// govrh fb sync
#ifdef CONFIG_VPP_GOVRH_FBSYNC
	int fbsync_enable;
	int fbsync_frame;
	int fbsync_step;
	int fbsync_substep;
	int fbsync_cnt;
	int fbsync_vsync;
	int fbsync_isrcnt;
#endif

	// VO PTS
	vpp_pts_t frame_pts;
	vpp_pts_t govw_pts;
	vpp_pts_t disp_pts;

#ifdef CONFIG_GOVW_FPS_AUTO_ADJUST
	// auto adjust fps for bandwidth
	int govw_tg_dynamic;
	unsigned int govw_tg_rcyc;
	unsigned int govw_tg_rtn_cnt;
	unsigned int govw_tg_rtn_max;
#endif

	// debug
	int dbg_msg_level;			// debug message level
	int govw_fb_cnt;			// debug for disable GOVW TG after show how many govw fb
	int dbg_flag;

	// hw interrupt	
	int govw_vbis_cnt;			// count for GOVW VBIS interrupt
	int govw_pvbi_cnt;			// count for GOVW PVBI interrupt
	int govrh_vbis_cnt;			// count for GOVR VBIS interrupt

	// hw error interrupt
	int govw_tg_err_cnt;		// count for GOVW TG error interrupt
	int govw_vpu_not_ready_cnt;	// count for GOVW TG error in VPU not ready
	int govw_ge_not_ready_cnt;	// count for GOVW TG error in GE not ready

	int vpu_y_err_cnt;			// count for VPU Y error interrupt
	int vpu_c_err_cnt;			// count for VPU C error interrupt

	int govr_underrun_cnt;		// count for GOVR underrun interrupt

	// HDMI DDC debug
	int dbg_hdmi_ddc_ctrl_err;
	int dbg_hdmi_ddc_read_err;
	int dbg_hdmi_ddc_crc_err;

#ifdef CONFIG_SCL_DIRECT_PATH_DEBUG
	// scale frame buffer queue
	int scl_fb_cnt;
	int scl_fb_tmr;
	int scl_fb_mb_used;
	int scl_fb_mb_over;
	int scl_fb_mb_index;
	int scl_fb_mb_clear;
	int scl_fb_mb_on_work;
#endif

#ifdef CONFIG_VPP_STREAM_CAPTURE
	// stream capture current video display
	int stream_enable;
	unsigned int stream_mb_lock;
	int stream_mb_sync_flag;
#endif
} vpp_info_t;

typedef struct {
	unsigned int pixel_clock;
	unsigned int PLL;
	unsigned int divisor;
	unsigned int rd_cyc;
} vpp_base_clock_t;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef VPP_C
#define EXTERN

const unsigned int vpp_csc_parm[VPP_CSC_MAX][7] = {
	{0x000004a8, 0x04a80662, 0x1cbf1e70, 0x081204a8, 0x00010000, 0x00010001, 0x00000101},	//YUV2RGB_SDTV_0_255
	{0x00000400, 0x0400057c, 0x1d351ea8, 0x06ee0400, 0x00010000, 0x00010001, 0x00000001},	//YUV2RGB_SDTV_16_235
	{0x000004a8, 0x04a8072c, 0x1ddd1f26, 0x087604a8, 0x00010000, 0x00010001, 0x00000101},	//YUV2RGB_HDTV_0_255
	{0x00000400, 0x04000629, 0x1e2a1f45, 0x07440400, 0x00010000, 0x00010001, 0x00000001},	//YUV2RGB_HDTV_16_235
	{0x00000400, 0x0400059c, 0x1d251ea0, 0x07170400, 0x00010000, 0x00010001, 0x00000001},	//YUV2RGB_JFIF_0_255	
	{0x00000400, 0x0400057c, 0x1d351ea8, 0x06ee0400, 0x00010000, 0x00010001, 0x00000001},	//YUV2RGB_SMPTE170M
	{0x00000400, 0x0400064d, 0x1e001f19, 0x074f0400, 0x00010000, 0x00010001, 0x00000001},	//YUV2RGB_SMPTE240M
	{0x02040107, 0x1f680064, 0x01c21ed6, 0x1e8701c2, 0x00211fb7, 0x01010101, 0x00000000},	//RGB2YUV_SDTV_0_255
	{0x02590132, 0x1f500075, 0x020b1ea5, 0x1e4a020b, 0x00011fab, 0x01010101, 0x00000000},	//RGB2YUV_SDTV_16_235
	{0x027500bb, 0x1f99003f, 0x01c21ea6, 0x1e6701c2, 0x00211fd7, 0x01010101, 0x00000000},	//RGB2YUV_HDTV_0_255
	{0x02dc00da, 0x1f88004a, 0x020b1e6d, 0x1e25020b, 0x00011fd0, 0x01010101, 0x00000000},	//RGB2YUV_HDTV_16_235
	{0x02590132, 0x1f530075, 0x02001ead, 0x1e530200, 0x00011fad, 0x00ff00ff, 0x00000000},	//RGB2YUV_JFIF_0_255
	{0x02590132, 0x1f500075, 0x020b1ea5, 0x1e4a020b, 0x00011fab, 0x01010101, 0x00000000},	//RGB2YUV_SMPTE170M
	{0x02ce00d9, 0x1f890059, 0x02001e77, 0x1e380200, 0x00011fc8, 0x01010101, 0x00000000},	//RGB2YUV_SMPTE240M
};

const vpp_timing_t vpp_video_mode_table[] = {
	{	/* 640x480@60 DMT/CEA861 */
	25175000,				/* pixel clock */	
	VPP_OPT_FPS_VAL(60)+VPP_OPT_HDMI_VIC_VAL(HDMI_640x480p60_4x3),	/* option */
	96, 48, 640, 16,		/* H sync, bp, pixel, fp */
	2, 33, 480, 10			/* V sync, bp, line, fp */
	},
	{	/* 640x480@60 CVT */
	23750000,				/* pixel clock */				
	VPP_OPT_FPS_VAL(60)+VPP_VGA_VSYNC_POLAR_HI, /* option */
	64, 80, 640, 16,		/* H sync, bp, pixel, fp */
	4, 13, 480, 3			/* V sync, bp, line, fp */
	},
	{	/* 640x480@75 DMT */
	31500000,				/* pixel clock */				
	VPP_OPT_FPS_VAL(75),	/* option */
	64, 120, 640, 16,		/* H sync, bp, pixel, fp */
	3, 16, 480, 1			/* V sync, bp, line, fp */
	},
	{	/* 640x480@75 CVT */
	30750000,				/* pixel clock */				
	VPP_OPT_FPS_VAL(75)+VPP_VGA_VSYNC_POLAR_HI, /* option */
	64, 88, 640, 24,		/* H sync, bp, pixel, fp */
	4, 17, 480, 3			/* V sync, bp, line, fp */
	},
	{	/* 720x480p@60 CEA861 */
	27027060,				/* pixel clock */				
	VPP_OPT_FPS_VAL(60)+VPP_OPT_HDMI_VIC_VAL(HDMI_720x480p60_4x3),	/* option */
	62, 60, 720, 16,		/* H sync, bp, pixel, fp */
	6, 30, 480, 9			/* V sync, bp, line, fp */
	},
	{	/* 720x480i@60 CEA861 */ /* Twin mode */
	27000000,				/* pixel clock */
	VPP_OPT_FPS_VAL(60)+VPP_OPT_HDMI_VIC_VAL(HDMI_1440x480i60_4x3)+VPP_OPT_INTERLACE+VPP_OPT_HSCALE_UP,		/* option */
	124, 114, 720, 38,		/* H sync, bp, pixel, fp */
	3, 15, 240, 4			/* V sync, bp, line, fp */
	},
	{	/* 720x480i@60 CEA861 */
	27000000,				/* pixel clock */
	VPP_OPT_FPS_VAL(60)+VPP_OPT_HDMI_VIC_VAL(HDMI_1440x480i60_4x3)+VPP_OPT_INTERLACE+VPP_OPT_HSCALE_UP,		/* option */
	124, 114, 720, 38,		/* H sync, bp, pixel, fp */
	3, 16, 240, 4			/* V sync, bp, line, fp */
	},
	{	/* 720x576p@50 CEA861 */
	27000000,				/* pixel clock */				
	VPP_OPT_FPS_VAL(50)+VPP_OPT_HDMI_VIC_VAL(HDMI_720x576p50_4x3),						/* option */
	64, 68, 720, 12,		/* H sync, bp, pixel, fp */
	5, 39, 576, 5			/* V sync, bp, line, fp */
	},
	{	/* 720x576i@50  */ /* Twin mode */
	27000050,				/* pixel clock */
	VPP_OPT_FPS_VAL(50)+VPP_OPT_HDMI_VIC_VAL(HDMI_1440x576i50_4x3)+VPP_OPT_INTERLACE+VPP_OPT_HSCALE_UP,		/* option */
	126, 138, 720, 24,		/* H sync, bp, pixel, fp */
	3, 19, 288, 2			/* V sync, bp, line, fp */
	},
	{	/* 720x576i@50 CEA861 */
	27000050,				/* pixel clock */
	VPP_OPT_FPS_VAL(50)+VPP_OPT_HDMI_VIC_VAL(HDMI_1440x576i50_4x3)+VPP_OPT_INTERLACE+VPP_OPT_HSCALE_UP,		/* option */
	126, 138, 720, 24,		/* H sync, bp, pixel, fp */
	3, 20, 288, 2			/* V sync, bp, line, fp */
	},
	{	/* 800x480@60 CVT */
	29500000,				/* pixel clock */				
	VPP_OPT_FPS_VAL(60)+VPP_VGA_VSYNC_POLAR_HI, /* option */
	72, 96, 800, 24,		/* H sync, bp, pixel, fp */
	7, 10, 480, 3			/* V sync, bp, line, fp */
	},
	{	/* 800x480@75 CVT */
	38500000,				/* pixel clock */				
	VPP_OPT_FPS_VAL(75)+VPP_VGA_VSYNC_POLAR_HI, /* option */
	80, 112, 800, 32,		/* H sync, bp, pixel, fp */
	7, 14, 480, 3			/* V sync, bp, line, fp */
	},
	{	/* 800x600@60 DMT */
	40000000,				/* pixel clock */				
	VPP_OPT_FPS_VAL(60)+VPP_VGA_HSYNC_POLAR_HI+VPP_VGA_VSYNC_POLAR_HI, 		/* option */
	128, 88, 800, 40,		/* H sync, bp, pixel, fp */
	4, 23, 600, 1			/* V sync, bp, line, fp */
	},
	{	/* 800x600@60 CVT */
	38250000,				/* pixel clock */
	VPP_OPT_FPS_VAL(60)+VPP_VGA_VSYNC_POLAR_HI, /* option */
	80, 112, 800, 32,		/* H sync, bp, pixel, fp */
	4, 17, 600, 3			/* V sync, bp, line, fp */
	},
	{	/* 800x600@75 DMT */
	49500000,				/* pixel clock */
	VPP_OPT_FPS_VAL(75)+VPP_VGA_HSYNC_POLAR_HI+VPP_VGA_VSYNC_POLAR_HI, 		/* option */
	80, 160, 800, 16,		/* H sync, bp, pixel, fp */
	3, 21, 600, 1			/* V sync, bp, line, fp */
	},
	{	/* 800x600@75 CVT */
	49000000,				/* pixel clock */
	VPP_OPT_FPS_VAL(75)+VPP_VGA_VSYNC_POLAR_HI, /* option */
	80, 120, 800, 40,		/* H sync, bp, pixel, fp */
	4, 22, 600, 3			/* V sync, bp, line, fp */
	},
	{	/* 848x480@60 DMT */
	33750000,				/* pixel clock */
	VPP_OPT_FPS_VAL(60)+VPP_VGA_HSYNC_POLAR_HI+VPP_VGA_VSYNC_POLAR_HI, /* option */
	112, 112, 848, 16,		/* H sync, bp, pixel, fp */
	8, 23, 480, 6			/* V sync, bp, line, fp */
	},
	{	/* 1024x600@60 DMT */
	49000000,				/* pixel clock */
	VPP_OPT_FPS_VAL(60)+VPP_VGA_VSYNC_POLAR_HI, /* option */
	104, 144, 1024, 40,		/* H sync, bp, pixel, fp */
	10, 11, 600, 3			/* V sync, bp, line, fp */
	},
	{	/* 1024x768@60 DMT */
	65000000,				/* pixel clock */
	VPP_OPT_FPS_VAL(60),	/* option */
	136, 160, 1024, 24,		/* H sync, bp, pixel, fp */
	6, 29, 768, 3			/* V sync, bp, line, fp */
	},
	{	/* 1024x768@60 CVT */
	63500000,				/* pixel clock */
	VPP_OPT_FPS_VAL(60)+VPP_VGA_VSYNC_POLAR_HI, /* option */
	104, 152, 1024, 48,		/* H sync, bp, pixel, fp */
	4, 23, 768, 3			/* V sync, bp, line, fp */
	},
	{	/* 1024x768@75 DMT */
	78750000,				/* pixel clock */
	VPP_OPT_FPS_VAL(75)+VPP_VGA_HSYNC_POLAR_HI+VPP_VGA_VSYNC_POLAR_HI, 		/* option */
	96, 176, 1024, 16,		/* H sync, bp, pixel, fp */
	3, 28, 768, 1			/* V sync, bp, line, fp */
	},
	{	/* 1024x768@75 CVT */
	82000000,				/* pixel clock */
	VPP_OPT_FPS_VAL(75)+VPP_VGA_VSYNC_POLAR_HI, /* option */
	104, 168, 1024, 64,		/* H sync, bp, pixel, fp */
	4, 30, 768, 3			/* V sync, bp, line, fp */
	},
	{	/* 1152x864@60 CVT */
	81750000,				/* pixel clock */
	VPP_OPT_FPS_VAL(60)+VPP_VGA_VSYNC_POLAR_HI, /* option */
	120, 184, 1152, 64,		/* H sync, bp, pixel, fp */
	4, 26, 864, 3			/* V sync, bp, line, fp */
	},
	{	/* 1152x864@75 DMT */
	108000000,				/* pixel clock */
	VPP_OPT_FPS_VAL(75)+VPP_VGA_HSYNC_POLAR_HI+VPP_VGA_VSYNC_POLAR_HI, 		/* option */
	128, 256, 1152, 64,		/* H sync, bp, pixel, fp */
	3, 32, 864, 1			/* V sync, bp, line, fp */
	},
	{	/* 1152x864@75 CVT */
	104000000,				/* pixel clock */
	VPP_OPT_FPS_VAL(75)+VPP_VGA_VSYNC_POLAR_HI, 		/* option */
	120, 192, 1152, 72,		/* H sync, bp, pixel, fp */
	4, 34, 864, 3			/* V sync, bp, line, fp */
	},
	{	/* 1280x720@60 CEA861 */
	74250060,				/* pixel clock */
	VPP_OPT_FPS_VAL(60)+VPP_OPT_HDMI_VIC_VAL(HDMI_1280x720p60_16x9)+VPP_VGA_HSYNC_POLAR_HI+VPP_VGA_VSYNC_POLAR_HI,		/* option */
	40, 220, 1280, 110,		/* H sync, bp, pixel, fp */
	5, 20, 720, 5			/* V sync, bp, line, fp */
	},
	{	/* 1280x720@50 CEA861 */
	74250050,				/* pixel clock */
	VPP_OPT_FPS_VAL(50)+VPP_OPT_HDMI_VIC_VAL(HDMI_1280x720p50_16x9)+VPP_VGA_HSYNC_POLAR_HI+VPP_VGA_VSYNC_POLAR_HI,		/* option */
	40, 220, 1280, 440,		/* H sync, bp, pixel, fp */
	5, 20, 720, 5			/* V sync, bp, line, fp */
	},
	{	/* 1280x720@60 CVT */
	74500000,				/* pixel clock */
	VPP_OPT_FPS_VAL(60)+VPP_VGA_VSYNC_POLAR_HI, /* option */
	128, 192, 1280, 64,		/* H sync, bp, pixel, fp */
	5, 20, 720, 3			/* V sync, bp, line, fp */
	},
	{	/* 1280x720@75 CVT */
	95750000,				/* pixel clock */
	VPP_OPT_FPS_VAL(75)+VPP_VGA_VSYNC_POLAR_HI, /* option */
	128, 208, 1280, 80,		/* H sync, bp, pixel, fp */
	5, 27, 720, 3			/* V sync, bp, line, fp */
	},
	{	/* 1280x768@60 DMT/CVT */
	79500000,				/* pixel clock */
	VPP_OPT_FPS_VAL(60)+VPP_VGA_VSYNC_POLAR_HI,	/* option */
	128, 192, 1280, 64,		/* H sync, bp, pixel, fp */
	7, 20, 768, 3			/* V sync, bp, line, fp */
	},
	{	/* 1280x768@75 DMT/CVT */
	102250000,				/* pixel clock */
	VPP_OPT_FPS_VAL(75)+VPP_VGA_VSYNC_POLAR_HI,	/* option */
	128, 208, 1280, 80,		/* H sync, bp, pixel, fp */
	7, 27, 768, 3			/* V sync, bp, line, fp */
	},
	{	/* 1280x800@60 DMT/CVT */
	83500000,				/* pixel clock */
	VPP_OPT_FPS_VAL(60)+VPP_VGA_VSYNC_POLAR_HI,	/* option */
	128, 200, 1280, 72,		/* H sync, bp, pixel, fp */
	6, 22, 800, 3			/* V sync, bp, line, fp */
	},
	{	/* 1280x800@75 DMT/CVT */
	106500000,				/* pixel clock */
	VPP_OPT_FPS_VAL(75)+VPP_VGA_VSYNC_POLAR_HI,	/* option */
	128, 208, 1280, 80,		/* H sync, bp, pixel, fp */
	6, 29, 800, 3			/* V sync, bp, line, fp */
	},
	{	/* 1280x960@60 DMT */
	108000000,				/* pixel clock */
	VPP_OPT_FPS_VAL(60)+VPP_VGA_HSYNC_POLAR_HI+VPP_VGA_VSYNC_POLAR_HI, 		/* option */
	112, 312, 1280, 96,		/* H sync, bp, pixel, fp */
	3, 36, 960, 1			/* V sync, bp, line, fp */
	},
	{	/* 1280x960@60 CVT */
	101250000,				/* pixel clock */
	VPP_OPT_FPS_VAL(60)+VPP_VGA_VSYNC_POLAR_HI, /* option */
	128, 208, 1280, 80,		/* H sync, bp, pixel, fp */
	4, 29, 960, 3			/* V sync, bp, line, fp */
	},
	{	/* 1280x960@75 CVT */
	130000000,				/* pixel clock */
	VPP_OPT_FPS_VAL(75)+VPP_VGA_VSYNC_POLAR_HI, /* option */
	136, 224, 1280, 88,		/* H sync, bp, pixel, fp */
	4, 38, 960, 3			/* V sync, bp, line, fp */
	},
	{	/* 1280x1024@60 DMT */
	108000000,				/* pixel clock */
	VPP_OPT_FPS_VAL(60)+VPP_VGA_HSYNC_POLAR_HI+VPP_VGA_VSYNC_POLAR_HI, 		/* option */
	112, 248, 1280, 48,		/* H sync, bp, pixel, fp */
	3, 38, 1024, 1			/* V sync, bp, line, fp */
	},
	{	/* 1280x1024@60 CVT */
	109000000,				/* pixel clock */
	VPP_OPT_FPS_VAL(60)+VPP_VGA_VSYNC_POLAR_HI, /* option */
	136, 216, 1280, 80,		/* H sync, bp, pixel, fp */
	7, 29, 1024, 3			/* V sync, bp, line, fp */
	},
	{	/* 1280x1024@75 DMT */
	135000000,				/* pixel clock */
	VPP_OPT_FPS_VAL(75)+VPP_VGA_HSYNC_POLAR_HI+VPP_VGA_VSYNC_POLAR_HI, 		/* option */
	144, 248, 1280, 16,		/* H sync, bp, pixel, fp */
	3, 38, 1024, 1			/* V sync, bp, line, fp */
	},
	{	/* 1280x1024@75 CVT */
	138750000,				/* pixel clock */
	VPP_OPT_FPS_VAL(75)+VPP_VGA_VSYNC_POLAR_HI, /* option */
	136, 224, 1280, 88,		/* H sync, bp, pixel, fp */
	7, 38, 1024, 3			/* V sync, bp, line, fp */
	},
	{	/* 1360x768@60 */
	85500000,				/* pixel clock */
	VPP_OPT_FPS_VAL(60)+VPP_VGA_HSYNC_POLAR_HI+VPP_VGA_VSYNC_POLAR_HI, 		/* option */
	112, 256, 1360, 64,		/* H sync, bp, pixel, fp */
	6, 18, 768, 3			/* V sync, bp, line, fp */
	},
	{	/* 1366x768@60 */
	85500000,				/* pixel clock */
	VPP_OPT_FPS_VAL(60)+VPP_VGA_HSYNC_POLAR_HI+VPP_VGA_VSYNC_POLAR_HI, 		/* option */
	143, 213, 1366, 70,		/* H sync, bp, pixel, fp */
	3, 24, 768, 3			/* V sync, bp, line, fp */
	},
	{	/* 1400x1050@60 DMT/CVT */
	121750000,				/* pixel clock */
	VPP_OPT_FPS_VAL(60)+VPP_VGA_VSYNC_POLAR_HI,	/* option */
	144, 232, 1400, 88,		/* H sync, bp, pixel, fp */
	4, 32, 1050, 3			/* V sync, bp, line, fp */
	},
	{	/* 1400x1050@60+R DMT/CVT */
	101000000,				/* pixel clock */
	VPP_OPT_FPS_VAL(60)+VPP_VGA_VSYNC_POLAR_HI,	/* option */
	32, 80, 1400, 48,		/* H sync, bp, pixel, fp */
	4, 23, 1050, 3			/* V sync, bp, line, fp */
	},
	{	/* 1440x480p@60 CEA861 */
	54054000,				/* pixel clock */
	VPP_OPT_FPS_VAL(60)+VPP_OPT_HDMI_VIC_VAL(HDMI_1440x480p60_16x9),	/* option */
	124, 120, 1440, 32,		/* H sync, bp, pixel, fp */
	6, 30, 480, 9			/* V sync, bp, line, fp */
	},
	{	/* 1440x900@60 DMT/CVT */
	106500000,				/* pixel clock */
	VPP_OPT_FPS_VAL(60)+VPP_VGA_VSYNC_POLAR_HI,	/* option */
	152, 232, 1440, 80,		/* H sync, bp, pixel, fp */
	6, 25, 900, 3			/* V sync, bp, line, fp */
	},
	{	/* 1440x900@75 DMT/CVT */
	136750000,				/* pixel clock */
	VPP_OPT_FPS_VAL(75)+VPP_VGA_VSYNC_POLAR_HI,	/* option */
	152, 248, 1440, 96,		/* H sync, bp, pixel, fp */
	6, 33, 900, 3			/* V sync, bp, line, fp */
	},
	{	/* 1600x1200@60 DMT/CVT */
	162000000,				/* pixel clock */
	VPP_OPT_FPS_VAL(60)+VPP_VGA_VSYNC_POLAR_HI,	/* option */
	192, 304, 1600, 64,		/* H sync, bp, pixel, fp */
	3, 46, 1200, 1			/* V sync, bp, line, fp */
	},
	{	/* 1680x1050@60 DMT/CVT */
	146250000,				/* pixel clock */
	VPP_OPT_FPS_VAL(60)+VPP_VGA_VSYNC_POLAR_HI,	/* option */
	176, 280, 1680, 104,	/* H sync, bp, pixel, fp */
	6, 30, 1050, 3			/* V sync, bp, line, fp */
	},
	{	/* 1920x1080p@60  */
	148500000,				/* pixel clock */
	VPP_OPT_FPS_VAL(60)+VPP_OPT_HDMI_VIC_VAL(HDMI_1920x1080p60_16x9)+VPP_VGA_HSYNC_POLAR_HI+VPP_VGA_VSYNC_POLAR_HI,	/* option */
	44, 148, 1920, 88,		/* H sync, bp, pixel, fp */
	5, 36, 1080, 4			/* V sync, bp, line, fp */
	},
	{	/* 1920x1080i@60  */ /* Twin mode */
	74250060,				/* pixel clock */
	VPP_OPT_FPS_VAL(60)+VPP_OPT_HDMI_VIC_VAL(HDMI_1920x1080i60_16x9)+VPP_OPT_INTERLACE+VPP_VGA_HSYNC_POLAR_HI+VPP_VGA_VSYNC_POLAR_HI,	/* option */
	44, 148, 1920, 88,		/* H sync, bp, pixel, fp */
	5, 15, 540, 2			/* V sync, bp, line, fp */
	},
	{	/* 1920x1080i@60 CEA861 */
	74250060,				/* pixel clock */
	VPP_OPT_FPS_VAL(60)+VPP_OPT_HDMI_VIC_VAL(HDMI_1920x1080i60_16x9)+VPP_OPT_INTERLACE+VPP_VGA_HSYNC_POLAR_HI+VPP_VGA_VSYNC_POLAR_HI,	/* option */
	44, 148, 1920, 88,		/* H sync, bp, pixel, fp */
	5, 16, 540, 2			/* V sync, bp, line, fp */
	},
	{	/* 1920x1080i@50  */ /* Twin mode */
	74250050,				/* pixel clock */
	VPP_OPT_FPS_VAL(50)+VPP_OPT_HDMI_VIC_VAL(HDMI_1920x1080i50_16x9)+VPP_OPT_INTERLACE+VPP_VGA_HSYNC_POLAR_HI+VPP_VGA_VSYNC_POLAR_HI,	/* option */
	44, 148, 1920, 528,		/* H sync, bp, pixel, fp */
	5, 15, 540, 2			/* V sync, bp, line, fp */
	},
	{	/* 1920x1080i@50 CEA861 */
	74250050,				/* pixel clock */
	VPP_OPT_FPS_VAL(50)+VPP_OPT_HDMI_VIC_VAL(HDMI_1920x1080i50_16x9)+VPP_OPT_INTERLACE+VPP_VGA_HSYNC_POLAR_HI+VPP_VGA_VSYNC_POLAR_HI,	/* option */
	44, 148, 1920, 528,		/* H sync, bp, pixel, fp */
	5, 16, 540, 2			/* V sync, bp, line, fp */
	},
	{	/* 1920x1080p@25 CEA861 */
	74250025,				/* pixel clock */
	VPP_OPT_FPS_VAL(25)+VPP_OPT_HDMI_VIC_VAL(HDMI_1920x1080p25_16x9)+VPP_VGA_HSYNC_POLAR_HI+VPP_VGA_VSYNC_POLAR_HI,	/* option */
	44, 148, 1920, 528,		/* H sync, bp, pixel, fp */
	5, 36, 1080, 4			/* V sync, bp, line, fp */
	},
	{	/* 1920x1080p@30 CEA861 */
	74250030,				/* pixel clock */
	VPP_OPT_FPS_VAL(30)+VPP_OPT_HDMI_VIC_VAL(HDMI_1920x1080p30_16x9)+VPP_VGA_HSYNC_POLAR_HI+VPP_VGA_VSYNC_POLAR_HI,						/* option */
	44, 148, 1920, 88,		/* H sync, bp, pixel, fp */
	5, 36, 1080, 4			/* V sync, bp, line, fp */
	},
	{	/* 1920x1080p@50  */
	148500050,				/* pixel clock */
	VPP_OPT_FPS_VAL(50)+VPP_OPT_HDMI_VIC_VAL(HDMI_1920x1080p50_16x9)+VPP_VGA_HSYNC_POLAR_HI+VPP_VGA_VSYNC_POLAR_HI,	/* option */
	44, 148, 1920, 528,		/* H sync, bp, pixel, fp */
	5, 36, 1080, 4			/* V sync, bp, line, fp */
	},
	{	/* 1920x1200p@60+R DMT/CVT */
	154000000,				/* pixel clock */
	VPP_OPT_FPS_VAL(60)+VPP_VGA_VSYNC_POLAR_HI,	/* option */
	32, 80, 1920, 48,		/* H sync, bp, pixel, fp */
	6, 26, 1200, 3			/* V sync, bp, line, fp */
	},
	{	/* 1920x1200p@60 DMT/CVT */
	193250000,				/* pixel clock */
	VPP_OPT_FPS_VAL(60)+VPP_VGA_VSYNC_POLAR_HI,	/* option */
	200, 336, 1920, 136,	/* H sync, bp, pixel, fp */
	6, 36, 1200, 3			/* V sync, bp, line, fp */
	},
	{ 0 }
};

const char *vpp_vout_str[] = {"SDA","SDD","LCD","DVI","HDMI","DVO2HDMI","DVO","VGA","BOOT"};
unsigned int vpp_vo_boot_arg[6] = {0xFF};
unsigned int vpp_vo_boot_arg2[6] = {0xFF};
char *vpp_colfmt_str[] = {"YUV420","YUV422H","YUV422V","YUV444","YUV411","GRAY","ARGB","AUTO","RGB888","RGB666","RGB565","RGB1555","RGB5551"};
const char *vpp_vpath_str[] = {"GOVW","GOVR","GE","VPU","SCL","MAX"};

#else
#define EXTERN extern

extern const unsigned int vpp_csc_parm[VPP_CSC_MAX][7];
extern char *vpp_colfmt_str[];
extern char *vpp_vpath_str[];
extern const vpp_timing_t vpp_video_mode_table[];
extern const char *vpp_vout_str[];
extern unsigned int vpp_vo_boot_arg[6];
extern unsigned int vpp_vo_boot_arg2[6];

#endif

EXTERN vpp_info_t g_vpp;

static __inline__ int vpp_get_hdmi_spdif(void)
{
	return (g_vpp.hdmi_audio_interface == VPP_HDMI_AUDIO_SPDIF)? 1:0;
}

//Internal functions
EXTERN int get_key(void);
EXTERN U8 vppif_reg8_in(U32 offset);
EXTERN U8 vppif_reg8_out(U32 offset, U8 val);
EXTERN U16 vppif_reg16_in(U32 offset);
EXTERN U16 vppif_reg16_out(U32 offset, U16 val);
EXTERN U32 vppif_reg32_in(U32 offset);
EXTERN U32 vppif_reg32_out(U32 offset, U32 val);
EXTERN U32 vppif_reg32_write(U32 offset, U32 mask, U32 shift, U32 val);
EXTERN U32 vppif_reg32_read(U32 offset, U32 mask, U32 shift);
EXTERN U32 vppif_reg32_mask(U32 offset, U32 mask, U32 shift);
EXTERN int vpp_check_dbg_level(vpp_dbg_level_t level);
EXTERN void vpp_set_dbg_gpio(int no,int value);
EXTERN unsigned int vpp_get_chipid(void);

//Export functions
EXTERN void vpp_mod_unregister(vpp_mod_t mod);
EXTERN vpp_mod_base_t *vpp_mod_register(vpp_mod_t mod,int size,unsigned int flags);
EXTERN vpp_mod_base_t *vpp_mod_get_base(vpp_mod_t mod);
EXTERN vpp_fb_base_t *vpp_mod_get_fb_base(vpp_mod_t mod);
EXTERN vdo_framebuf_t *vpp_mod_get_framebuf(vpp_mod_t mod);
EXTERN void vpp_mod_set_timing(vpp_mod_t mod,vpp_timing_t *tmr);
EXTERN void vpp_mod_init(void);

EXTERN void vpp_clr_framebuf(vpp_mod_t mod);
EXTERN unsigned int vpp_get_base_clock(vpp_mod_t mod);
EXTERN void vpp_set_video_scale(vdo_view_t *vw);
EXTERN int vpp_set_recursive_scale(vdo_framebuf_t *src_fb,vdo_framebuf_t *dst_fb);
EXTERN vpp_display_format_t vpp_get_fb_field(vdo_framebuf_t *fb);
EXTERN void vpp_wait_vsync(void);
EXTERN int vpp_irqproc_flag(vpp_int_t type, int * flag, int wait);
EXTERN int vpp_irqproc_work(vpp_int_t type, int(* func)(void * argc), void * arg, int wait);
EXTERN int vpp_get_gcd(int A, int B);
EXTERN unsigned int vpp_calculate_diff(unsigned int val1,unsigned int val2);
EXTERN void vpp_check_scale_ratio(int *src,int *dst,int max,int min);
EXTERN void vpp_check_bilinear_arg(unsigned int *src,unsigned int *dst,unsigned int max);
EXTERN void vpp_calculate_timing(vpp_mod_t mod,unsigned int fps,vpp_clock_t *tmr);
EXTERN void vpp_fill_framebuffer(vdo_framebuf_t *fb,unsigned int x,unsigned int y,unsigned int w,unsigned int h,unsigned int color);
EXTERN vpp_csc_t vpp_check_csc_mode(vpp_csc_t mode,vdo_color_fmt src_fmt,vdo_color_fmt dst_fmt,unsigned int flags);
EXTERN void vpp_trans_timing(vpp_mod_t mod,vpp_timing_t *tmr,vpp_clock_t *hw_tmr,int to_hw);
EXTERN void vpp_fill_pattern(vpp_mod_t mod,int no,int arg);
EXTERN unsigned int vpp_get_vmode_pixel_clock(unsigned int resx,unsigned int resy,unsigned int fps);
EXTERN vpp_timing_t *vpp_get_video_mode(unsigned int resx,unsigned int resy,unsigned int pixel_clock,int *index);
EXTERN vpp_timing_t *vpp_get_video_mode_ext(unsigned int resx,unsigned int resy,unsigned int pixel_clock,unsigned int option);
EXTERN void vpp_set_video_mode(unsigned int resx,unsigned int resy,unsigned int pixel_clock);
EXTERN void vpp_set_video_quality(int mode);
EXTERN unsigned int vpp_get_video_mode_fps(vpp_timing_t *timing);
EXTERN void vpp_calculate_clock(vpp_base_clock_t *clk,unsigned int div_max,unsigned int base_mask);
EXTERN void vpp_govw_dynamic_tg_set_rcyc(int rcyc);
EXTERN void vpp_govw_dynamic_tg(int err);
EXTERN void vpp_set_vppm_int_enable(vpp_int_t int_bit,int enable);
EXTERN __inline__ void vpp_cache_sync(void);

#ifdef __KERNEL__
/* dev-vpp.c */
EXTERN void vpp_get_info(struct fb_var_screeninfo *var);
EXTERN int vpp_set_par(struct fb_info *info);
EXTERN int vpp_mmap(struct vm_area_struct *vma);
EXTERN int vpp_ioctl(unsigned int cmd,unsigned long arg);
EXTERN int vpp_dev_init(void);
EXTERN int vpp_i2c_write(unsigned int addr,unsigned int index,char *pdata,int len);
EXTERN int vpp_i2c_read(unsigned int addr,unsigned int index,char *pdata,int len);
EXTERN int vpp_i2c_enhanced_ddc_read(unsigned int addr,unsigned int index,char *pdata,int len);
EXTERN int vpp_i2c0_read(unsigned int addr,unsigned int index,char *pdata,int len) ;
EXTERN int vpp_suspend(int state);
EXTERN int vpp_resume(void);
EXTERN unsigned int *vpp_backup_reg(unsigned int addr,unsigned int size);
EXTERN int vpp_restore_reg(unsigned int addr,unsigned int size,unsigned int *reg_ptr);
EXTERN void vpp_backup_reg2(unsigned int addr,unsigned int size,unsigned int *ptr);
EXTERN void vpp_restore_reg2(unsigned int addr,unsigned int size,unsigned int *reg_ptr);
EXTERN int vpp_pan_display(struct fb_var_screeninfo *var, struct fb_info *info,int enable);
EXTERN void vpp_set_vout_enable_timer(void);
EXTERN void vpp_netlink_notify(int no,int cmd,int arg);
#endif

EXTERN void vpp_reg_dump(unsigned int addr,int size);
EXTERN int vpp_irqproc_disable_disp(void *arg);
EXTERN int vpp_irqproc_enable_vpu(void *arg);
EXTERN int vpp_irqproc_monitor_dac_sense(void *arg);
EXTERN unsigned int vpp_convert_colfmt(int yuv2rgb,unsigned int data);

EXTERN void vpp_set_govw_tg(int enable);
EXTERN void vpp_set_govm_path(vpp_path_t in_path, vpp_flag_t enable);
EXTERN vpp_path_t vpp_get_govm_path(void);
EXTERN void vpp_set_power_mgr(int arg);
EXTERN void vpp_vpu_sw_reset(void);

EXTERN void vpp_show_timing(vpp_timing_t *tmr,vpp_clock_t *clk);
EXTERN void vpp_show_framebuf(char *str,vdo_framebuf_t *fb);
EXTERN void vpp_show_view(char *str,vdo_view_t *view);
EXTERN void vpp_dbg_wait(char *str);

#undef EXTERN

#ifdef __cplusplus
}
#endif
#endif				//VPP_H
