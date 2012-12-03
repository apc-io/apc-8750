/*++ 
 * linux/drivers/video/wmt/vppm.h
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

//#ifdef WMT_FTBLK_VPP

#ifndef VPPM_H
#define VPPM_H

typedef struct {
	VPP_MOD_BASE;
} vppm_mod_t;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef VPPM_C
#define EXTERN
#else
#define EXTERN extern
#endif

EXTERN vppm_mod_t *p_vppm;

EXTERN void vppm_set_int_enable(vpp_flag_t enable, vpp_int_t int_bit);
EXTERN int vppm_get_int_enable(vpp_int_t int_bit);
EXTERN vpp_int_t vppm_get_int_status(void);
EXTERN void vppm_clean_int_status(vpp_int_t int_sts);
EXTERN void vppm_set_module_reset(vpp_mod_t mod_bit); //ok
EXTERN int vppm_mod_init(void);
#ifdef WMT_FTBLK_DISP
EXTERN void vppm_set_DAC_select(int tvmode);
#endif

#undef EXTERN

#ifdef __cplusplus
}
#endif
#endif				//VPP_H
//#endif				//WMT_FTBLK_VPP
