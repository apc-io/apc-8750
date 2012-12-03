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

#include "vpp.h"

#ifdef WMT_FTBLK_GOVM

#ifndef GOVM_H
#define GOVM_H

typedef struct {
	VPP_MOD_BASE;

	vpp_path_t path;
} govm_mod_t;

#ifdef WMT_FTBLK_PIP
typedef struct {
	VPP_MOD_BASE;

	unsigned int resx_visual;
	unsigned int resy_visual;
	unsigned int posx;
	unsigned int posy;
	unsigned int pre_yaddr;
	unsigned int pre_caddr;
} pip_mod_t;

#endif

#ifdef __cplusplus
extern "C" {
#endif

#undef EXTERN
#ifdef GOVM_C
#define EXTERN
#else
#define EXTERN extern
#endif

EXTERN govm_mod_t *p_govm;

#ifdef WMT_FTBLK_PIP
EXTERN pip_mod_t *p_pip;
#endif

EXTERN void govm_reg_dump(void);
EXTERN void govm_set_in_path(vpp_path_t in_path, vpp_flag_t enable);
EXTERN vpp_path_t govm_get_in_path(void);
EXTERN vpp_flag_t govm_set_output_device(vpp_output_device_t device);
EXTERN void govm_set_blanking(U32 color);
EXTERN void govm_set_disp_coordinate(U32 x, U32 y);
EXTERN void govm_get_disp_coordinate(U32 * p_x, U32 * p_y);
EXTERN void govm_set_vpu_coordinate(U32 x_s, U32 y_s, U32 x_e, U32 y_e);
EXTERN void govm_set_vpu_position(U32 x1, U32 y1);
EXTERN void govm_set_alpha_mode(vpp_flag_t enable,vpp_alpha_t mode,int A,int B);
EXTERN void govm_set_colorbar(vpp_flag_t enable,int width,int inverse);
EXTERN void govm_set_int_enable(vpp_flag_t enable);
EXTERN void govm_set_reg_update(vpp_flag_t enable);
EXTERN void govm_set_reg_level(vpp_reglevel_t level);
EXTERN void govm_set_clamping_enable(vpp_flag_t enable);
EXTERN void govm_int_set_enable(vpp_flag_t enable, vpp_int_err_t int_bit);
EXTERN void govm_clean_int_status(vpp_int_err_t int_sts);
EXTERN vpp_int_err_t govm_get_int_status(void);
EXTERN void govm_set_gamma_mode(int mode);
EXTERN int govm_mod_init(void);

#ifdef WMT_FTBLK_PIP
EXTERN void govm_set_dominate(vpp_flag_t pip);
EXTERN void govm_set_pip_coordinate(unsigned int x_s, unsigned int y_s, unsigned int x_e, unsigned int y_e);
EXTERN void govm_set_pip_position(unsigned int x1, unsigned int y1);
EXTERN void govm_set_pip_interlace(vpp_flag_t enable);
EXTERN void govm_set_pip_field(vpp_display_format_t field);
EXTERN vdo_color_fmt govm_get_pip_color_format(void);
EXTERN void govm_set_pip_fb_width(unsigned int width,unsigned int buf_width);
#endif

#undef EXTERN

#ifdef __cplusplus
}
#endif
#endif				//GOVM_H
#endif				//WMT_FTBLK_GOVM
