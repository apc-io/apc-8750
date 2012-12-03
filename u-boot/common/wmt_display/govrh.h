/*++ 
 * linux/drivers/video/wmt/govm.c
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

#include "vpp.h"

#ifdef WMT_FTBLK_GOVRH

#ifndef GOVRH_H
#define GOVRH_H

typedef struct {
	VPP_MOD_BASE;
	
	unsigned int vga_dac_sense_cnt;
	unsigned int vga_dac_sense_val;
	unsigned int h_pixel;	// vsync overlap patch
	unsigned int v_line;	// vsync overlap patch
} govrh_mod_t;

#ifdef WMT_FTBLK_GOVRH_CURSOR
#define GOVRH_CURSOR_HIDE_TIME	15
typedef struct {
	VPP_MOD_BASE;

	unsigned int posx;
	unsigned int posy;
	unsigned int hotspot_x;
	unsigned int hotspot_y;
	int chg_flag;
	vdo_color_fmt colfmt;
	unsigned int cursor_addr1;
	unsigned int cursor_addr2;
	int enable;
	int hide_cnt;
} govrh_cursor_mod_t;
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef GOVRH_C
#define EXTERN
#else
#define EXTERN extern
#endif

EXTERN govrh_mod_t *p_govrh;
#ifdef WMT_FTBLK_GOVRH_CURSOR
EXTERN govrh_cursor_mod_t *p_cursor;
#endif

EXTERN vpp_flag_t govrh_set_tg_enable(vpp_flag_t enable);
EXTERN void govrh_set_dvo_enable(vpp_flag_t enable);
EXTERN void govrh_set_dvo_sync_polar(vpp_flag_t hsync,vpp_flag_t vsync);
EXTERN vpp_flag_t govrh_set_dvo_outdatw(vpp_datawidht_t width);
EXTERN void govrh_set_dvo_clock_delay(int inverse,int delay);
EXTERN void govrh_set_colorbar(vpp_flag_t enable,int mode,int inv);
EXTERN void govrh_set_vga_enable(vpp_flag_t enable);
EXTERN void govrh_set_VGA_sync(U32 hsync, U32 vsync);
EXTERN void govrh_set_VGA_sync_polar(U32 hsync, U32 vsync);
EXTERN int govrh_monitor_DAC_sense(void);
EXTERN void govrh_set_DAC_pwrdn(vpp_flag_t enable);
EXTERN void govrh_set_contrast(int level);
EXTERN int govrh_get_contrast(void);
EXTERN void govrh_set_brightness(int level);
EXTERN int govrh_get_brightness(void);
EXTERN void govrh_set_MIF_enable(vpp_flag_t enable);
EXTERN vpp_flag_t govrh_set_data_format(vdo_color_fmt format);
EXTERN vdo_color_fmt govrh_get_color_format(void);
EXTERN vpp_flag_t govrh_set_source_format(vpp_display_format_t format);
EXTERN void govrh_set_output_format(vpp_display_format_t field);
EXTERN void govrh_set_fb_addr(U32 y_addr,U32 c_addr);
EXTERN void govrh_get_fb_addr(U32 *y_addr,U32 *c_addr);
//	EXTERN vpp_flag_t govrh_set_output_format(vpp_display_format_t format);
EXTERN void govrh_set_fb_info(U32 width, U32 act_width, U32 x_offset,U32 y_offset);
EXTERN void govrh_get_fb_info(U32 * width, U32 * act_width,U32 * x_offset, U32 * y_offset);
EXTERN void govrh_set_fifo_index(U32 index);
EXTERN vpp_flag_t govrh_set_reg_level(vpp_reglevel_t level);
EXTERN void govrh_set_reg_update(vpp_flag_t enable);
EXTERN void govrh_set_csc_mode(vpp_csc_t mode);
EXTERN void govrh_set_framebuffer(vdo_framebuf_t *inbuf);
EXTERN void govrh_get_framebuffer(vdo_framebuf_t *fb);
EXTERN vdo_color_fmt govrh_get_dvo_color_format(void);
EXTERN void govrh_set_dvo_color_format(vdo_color_fmt fmt);
EXTERN vpp_int_err_t govrh_get_int_status(void);
EXTERN void govrh_clean_int_status(vpp_int_err_t int_sts);
EXTERN unsigned int govrh_set_clock(unsigned int pixel_clock);
EXTERN void govrh_set_timing(vpp_timing_t *timing);
EXTERN void govrh_get_tg(vpp_clock_t * tmr);

#ifdef WMT_FTBLK_GOVRH_IGS
EXTERN void govrh_IGS_set_mode(int no,int mode_18bit,int msb);
EXTERN void govrh_IGS_set_RGB_swap(int mode);
#endif
#ifdef WMT_FTBLK_DISP
EXTERN void govrh_DISP_set_enable(vpp_flag_t enable);
#endif
EXTERN int govrh_mod_init(void);

#ifdef WMT_FTBLK_GOVRH_CURSOR
EXTERN void govrh_CUR_set_enable(vpp_flag_t enable);
EXTERN void govrh_CUR_set_framebuffer(vdo_framebuf_t *fb);
EXTERN void govrh_CUR_set_coordinate(unsigned int x1,unsigned int y1,unsigned int x2,unsigned int y2);
EXTERN void govrh_CUR_set_position(unsigned int x,unsigned int y);
EXTERN void govrh_CUR_set_color_key(int enable,int alpha,unsigned int colkey);
EXTERN void govrh_CUR_set_colfmt(vdo_color_fmt colfmt);
EXTERN int govrh_irqproc_set_position(void *arg);
#endif

#ifdef WMT_FTBLK_GOVRH_DAC
EXTERN void govrh_DAC_set_sense_sel(int tv);
EXTERN void govrh_DAC_set_pwrdn(vpp_flag_t enable);
EXTERN unsigned int govrh_DAC_monitor_sense(void);
EXTERN void govrh_DAC_set_sense_value(int no,unsigned int lp_val,unsigned int normal_val);
#endif

#ifdef GOVRH_DISP_DUAL
void govrh_set_dual_disp(int disp,int govrh);
#endif

#undef EXTERN

#ifdef __cplusplus
}
#endif
#endif				//GOVRH_H
#endif				//WMT_FTBLK_GOVRH
