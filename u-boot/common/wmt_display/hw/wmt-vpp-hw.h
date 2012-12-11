/*++ 
 * linux/drivers/video/wmt/hw/wmt-vpp-hw.h
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

#ifndef WMT_VPP_HW_H
#define WMT_VPP_HW_H

/*-------------------- EXPORTED PRIVATE CONSTANTS ----------------------------*/
/*
* Product ID / Project ID
* 84xx series: 8420/3300, 8430/3357, 8435/3437
* 85xx series: 8500/3400, 8510/3426, 8520/3429
*/
//84xx series, (1-100) with VDU & DSP
#define VIA_PID_8420	10	// 3300
#define VIA_PID_8430	12	// 3357
#define WMT_PID_8435	14	// 3437
#define WMT_PID_8440	16	// 3451
#define WMT_PID_8425	18	// 3429
#define WMT_PID_8710	20	// 3445

//85xx series, (101-200)
#define VIA_PID_8500	110	// 3400
#define WMT_PID_8505	111
#define WMT_PID_8510	112	// 3426*

#define WMT_PID_8710_A	1
#define WMT_PID_8710_B	2

//current pid
#define WMT_CUR_PID		WMT_PID_8710
#define WMT_SUB_PID		WMT_PID_8710_B

// #define WMT_SUB_PID		WMT_PID_8505
#ifndef WMT_SUB_PID
	#define WMT_SUB_PID		0
#endif

// sw patch
#define PATCH_VPU_HALT_GOVW_TG_ERR		// [wm8710A1-B0] vpu patch : vpu hang when GE not ready ( VPU )
#ifdef PATCH_VPU_HALT_GOVW_TG_ERR
#define PATCH_VPU_HALT_GOVW_TG_ERR1		// scale 2/3, ret (VBIE)
//#define PATCH_VPU_HALT_GOVW_TG_ERR2	// scale 2/3, VBIE, pxlw =1 (VBIS), ret (VBIS)
#endif
#define PATCH_GOVRH_CURSOR_PREVBI		// [wm8710B0] pre vbi should more 6 to avoid garbage line
#define PATCH_VPU_H_BILINEAR			// [wm8710B0] vpu H bilinear will make TG error ( VPU )
// #define PATCH_GOVRH_VSYNC_OVERLAP	// govrh patch : govrh vsync shall overlap hsync 1 pixel
// #define PATCH_GOVW_TG_ENABLE			// govw tg on/off should disable govm path
#define PATCH_SCL_SCALEDN_GARBAGE_EDGE	// scl garbage edge in scale down and H pos 34

// VPP interrupt map to irq
#define VPP_IRQ_SCL_FINISH	IRQ_VPP_IRQ0
#define VPP_IRQ_SCL			IRQ_VPP_IRQ1
#define VPP_IRQ_SCL444_TG	IRQ_VPP_IRQ2
#define VPP_IRQ_VPPM		IRQ_VPP_IRQ3
#define VPP_IRQ_GOVW_TG		IRQ_VPP_IRQ4
#define VPP_IRQ_GOVW		IRQ_VPP_IRQ5
#define VPP_IRQ_GOVM		IRQ_VPP_IRQ6
#define VPP_IRQ_GE			IRQ_VPP_IRQ7
#define VPP_IRQ_GOVRH_TG	IRQ_VPP_IRQ8
#define VPP_IRQ_DVO			IRQ_VPP_IRQ9
#define VPP_IRQ_VID			IRQ_VPP_IRQ10
#define VPP_IRQ_GOVR		IRQ_VPP_IRQ11
#define VPP_IRQ_GOVRS_TG	IRQ_VPP_IRQ12
#define VPP_IRQ_VPU			IRQ_VPP_IRQ13
#define VPP_IRQ_VPU_TG		IRQ_VPP_IRQ14
#define VPP_IRQ_HDMI_CP		IRQ_VPP_IRQ15
#define VPP_IRQ_HDMI_HPDH	IRQ_VPP_IRQ16
#define VPP_IRQ_HDMI_HPDL	IRQ_VPP_IRQ17
#define VPP_IRQ_18			IRQ_VPP_IRQ18

// platform hw define
#define VPP_DIV_VPP			VPP_DIV_NA12
#define VPP_VOINT_NO		0
#define VPP_BLT_PWM_NUM		0
#define VPP_I2C_ID			1

#define VPP_VPATH_AUTO_DEFAULT	VPP_VPATH_SCL

// vout enable
#ifndef CFG_LOADER
#ifdef CONFIG_WMT_VGA
#define WMT_FTBLK_VOUT_VGA
#endif
#define WMT_FTBLK_VOUT_DVI
#define WMT_FTBLK_VOUT_DVO2HDMI
#define WMT_FTBLK_VOUT_LCD
#ifdef CONFIG_WMT_INTTV
#define WMT_FTBLK_VOUT_SDA
#endif
#define WMT_FTBLK_VOUT_SDD
#ifdef CONFIG_WMT_HDMI
#define WMT_FTBLK_VOUT_HDMI
#endif
#define WMT_FTBLK_VOUT_LVDS
#define WMT_FTBLK_VOUT_BOOT
/* ----------------------------------------- */
#else
#define CONFIG_WMT_HDMI
#define WMT_FTBLK_VOUT_BOOT
#define WMT_FTBLK_VOUT_VGA
#define WMT_FTBLK_VOUT_DVI
#define WMT_FTBLK_VOUT_LCD
#define WMT_FTBLK_VOUT_DVO2HDMI
#define WMT_FTBLK_VOUT_HDMI
#define WMT_FTBLK_VOUT_SDA
#define WMT_FTBLK_VOUT_LVDS
#endif
/*-------------------- DEPENDENCY -------------------------------------*/
#ifndef CFG_LOADER
#include "wmt-vpp-reg.h"
#include "wmt-vpu-reg.h"
#include "wmt-scl-reg.h"
#include "wmt-govm-reg.h"
#include "wmt-govw-reg.h"
#include "wmt-govrh-reg.h"
#include "wmt-disp-reg.h"
#include "wmt-lvds-reg.h"
#include "wmt-hdmi-reg.h"
#include "wmt-cec-reg.h"
/* ----------------------------------------- */
#else
#include "wmt-vpp-reg.h"
#include "wmt-govrh-reg.h"
#include "wmt-lvds-reg.h"
#include "wmt-hdmi-reg.h"
#include "wmt-disp-reg.h"
#endif
#endif //WMT_VPP_HW_H

