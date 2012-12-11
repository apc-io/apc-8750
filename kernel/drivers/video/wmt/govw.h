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

#ifdef WMT_FTBLK_GOVW

#ifndef GOVW_H
#define GOVW_H

typedef struct {
	VPP_MOD_BASE;
} govw_mod_t;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef GOVW_C
#define EXTERN
#else
#define EXTERN extern
#endif

EXTERN govw_mod_t *p_govw;

EXTERN void govw_set_tg_enable(vpp_flag_t enable);
EXTERN void govw_set_hd_mif_enable(vpp_flag_t enable);
EXTERN vpp_flag_t govw_set_hd_color_format(vdo_color_fmt format);
EXTERN vdo_color_fmt govw_get_hd_color_format(void);
EXTERN void govw_set_hd_fb_addr(U32 y_addr,U32 c_addr);
EXTERN void govw_get_hd_fb_addr(U32 * y_addr,U32 * c_addr);
EXTERN void govw_set_hd_width(U32 width, U32 fb_width);
EXTERN void govw_get_hd_width(U32 *width,U32 *fb_width);
EXTERN int govw_mod_init(void);
EXTERN void govw_set_hd_framebuffer(vdo_framebuf_t *fb);
EXTERN void govw_set_reg_level(vpp_reglevel_t level);

#undef EXTERN

#ifdef __cplusplus
}
#endif
#endif				//GOVW_H
#endif				//WMT_FTBLK_GOVW
