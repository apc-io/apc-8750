/*++ 
 * linux/drivers/video/wmt/lvds.h
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

#ifndef LVDS_H
/* To assert that only one occurrence is included */
#define LVDS_H
/*-------------------- MODULE DEPENDENCY -------------------------------------*/

/*	following is the C++ header	*/
#ifdef	__cplusplus
extern	"C" {
#endif

/*-------------------- EXPORTED PRIVATE CONSTANTS ----------------------------*/
/* #define  LVDS_XXXX  1    *//*Example*/

/*-------------------- EXPORTED PRIVATE TYPES---------------------------------*/
/* typedef  void  lvds_xxx_t;  *//*Example*/

/*-------------------- EXPORTED PRIVATE VARIABLES -----------------------------*/
#ifdef LVDS_C
#define EXTERN
#else
#define EXTERN   extern
#endif /* ifdef LVDS_C */

/* EXTERN int      lvds_xxx; *//*Example*/

#undef EXTERN

/*--------------------- EXPORTED PRIVATE MACROS -------------------------------*/
/* #define LVDS_XXX_YYY   xxxx *//*Example*/
/*--------------------- EXPORTED PRIVATE FUNCTIONS  ---------------------------*/
/* extern void  lvds_xxx(void); *//*Example*/
void lvds_set_enable(vpp_flag_t enable);
void lvds_set_rgb_type(int mode);
vdo_color_fmt lvds_get_colfmt(void);
void lvds_reg_dump(void);
void lvds_suspend(int sts);
void lvds_resume(int sts);
void lvds_init(void);
void lvds_set_power_down(int pwrdn);

#ifdef	__cplusplus
}
#endif	
#endif /* ifndef LVDS_H */

