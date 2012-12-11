/*++ 
 * linux/drivers/video/wmt/vpu.h
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

#include "vpp.h"

#ifndef VPU_H
#define VPU_H

typedef struct {
	VPP_MOD_BASE;

	unsigned int resx_visual;
	unsigned int resy_visual;
	unsigned int posx;
	unsigned int posy;
	
	unsigned int resx_virtual_scale;
	unsigned int resy_virtual_scale;
	unsigned int resx_visual_scale;
	unsigned int resy_visual_scale;

	unsigned int pre_img_w;
	unsigned int pre_img_h;
	unsigned int pre_vis_w;
	unsigned int pre_vis_h;
	
	vpp_deinterlace_t dei_mode;
	vpp_scale_mode_t scale_mode;
	unsigned int skip_fb;			// if vpu scale underrun happen will skip # govw frame update
	unsigned int underrun_cnt;		// if vpu scale underrun happen will dec 1, this counter to 0 will disable vpu fb update
} vpu_mod_t;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef VPU_C
#define EXTERN
#else
#define EXTERN extern
#endif

EXTERN vpu_mod_t *p_vpu;

#ifdef WMT_FTBLK_VPU

EXTERN void vpu_set_enable(vpp_flag_t enable);
EXTERN void vpu_set_reg_update(vpp_flag_t enable);
EXTERN void vpu_set_reg_read_select(int level);
EXTERN void vpu_set_drop_line(vpp_flag_t enable);
EXTERN void vpu_set_scale(unsigned int SRC_W,unsigned int SRC_H,unsigned int DST_W,unsigned int DST_H);
EXTERN vpp_mod_t vpu_get_timing_master(void);
EXTERN void vpu_set_tg_enable(vpp_flag_t enable);

EXTERN vdo_color_fmt vpu_r_get_color_format(void);
EXTERN void vpu_r_get_fb_info(unsigned int * p_y_buffer, unsigned int * p_y_pixel, unsigned int * x_offset, unsigned int * y_offset);
EXTERN void vpu_r_get_fb_addr(unsigned int * y_addr, unsigned int * c_addr);
EXTERN void vpu_r_set_fb_addr(unsigned int y_addr, unsigned int c_addr);
EXTERN void vpu_r_set_framebuffer(vdo_framebuf_t * fb);
EXTERN void vpu_r_set_mif_enable(vpp_flag_t enable);
EXTERN void vpu_r_set_mif2_enable(vpp_flag_t enable);
EXTERN void vpu_r_set_width(unsigned int y_pixel, unsigned int y_buffer);
EXTERN void vpu_r_set_crop(unsigned int h_crop, unsigned int v_crop);

EXTERN vdo_color_fmt vpu_w_get_color_format(void);
EXTERN void vpu_w_set_fb_width(unsigned int width, unsigned int buf_width);
EXTERN void vpu_w_set_mif_enable(vpp_flag_t enable);

EXTERN void vpu_dei_set_mode(vpp_deinterlace_t mode);
EXTERN void vpu_dei_set_enable(vpp_flag_t enable);
EXTERN void vpu_dei_get_sum(unsigned int *ysum,unsigned int *usum,unsigned int *vsum);

EXTERN void vpu_int_set_enable(vpp_flag_t enable, vpp_int_t int_bit);
EXTERN vpp_int_err_t vpu_int_get_status(void);
EXTERN void vpu_int_clean_status(vpp_int_err_t int_sts);
EXTERN int vpu_mod_init(void);

EXTERN void vpu_mvr_set_enable(vpp_flag_t enable);
EXTERN void vpu_mvr_set_addr(unsigned int addr);
EXTERN void vpu_mvr_set_width(unsigned int width,unsigned int fb_w);
EXTERN void vpu_mvr_set_field_mode(vpp_flag_t enable);
EXTERN void vpu_set_direct_path(vpp_flag_t enable);
EXTERN int vpu_get_direct_path(void);

#endif				//WMT_FTBLK_VPU

#undef EXTERN

#ifdef __cplusplus
}
#endif
#endif				//VPU_H
