/*++ 
 * linux/drivers/video/wmt/register/wm8710/wmt-vpp-reg.h
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

#ifndef WMT_VPP_REG_H
#define WMT_VPP_REG_H

#define REG_VPP_BEGIN				(VPP_BASE_ADDR+0x00)
#define REG_VPP_INTSTS				(VPP_BASE_ADDR+0x04)
#define REG_VPP_INTEN				(VPP_BASE_ADDR+0x08)
#define REG_VPP_WATCH_SEL			(VPP_BASE_ADDR+0x0c)
#define REG_VPP_SWRST1_SEL			(VPP_BASE_ADDR+0x10)
#define REG_VPP_SWRST2_SEL			(VPP_BASE_ADDR+0x14)
#define REG_VPP_DAC_SEL				(VPP_BASE_ADDR+0x18)
#if(WMT_SUB_PID == WMT_PID_8710_A)
#define REG_VPP_END					(VPP_BASE_ADDR+0x18)
#else
#define REG_VPP_SWRST3_SEL			(VPP_BASE_ADDR+0x1C)
#define REG_VPP_SSCG				(VPP_BASE_ADDR+0x20)
#define REG_VPP_END					(VPP_BASE_ADDR+0x20)
#endif

//REG_VPP_INTSTS,0x04
#define VPP_DISP_INTSTS_VBIE		REG_VPP_INTSTS,BIT26,26
#define VPP_DISP_INTSTS_VBIS		REG_VPP_INTSTS,BIT25,25
#define VPP_DISP_INTSTS_PVBI		REG_VPP_INTSTS,BIT24,24
#define VPP_DISP_INTSTS				REG_VPP_INTSTS,0x7000000,24
#define VPP_SCL_INTSTS_VBIE			REG_VPP_INTSTS,BIT18,18
#define VPP_SCL_INTSTS_VBIS			REG_VPP_INTSTS,BIT17,17
#define VPP_SCL_INTSTS_PVBI			REG_VPP_INTSTS,BIT16,16
#define VPP_SCL_INTSTS				REG_VPP_INTSTS, 0x70000, 16
#define VPP_GOVRH_INTSTS_VBIE		REG_VPP_INTSTS,BIT10,10
#define VPP_GOVRH_INTSTS_VBIS		REG_VPP_INTSTS,BIT9,9
#define VPP_GOVRH_INTSTS_PVBI		REG_VPP_INTSTS,BIT8,8
#define VPP_GOVRH_INTSTS			REG_VPP_INTSTS, 0x700, 8
#define VPP_VPU_INTSTS_VBIE			REG_VPP_INTSTS,BIT6,6
#define VPP_VPU_INTSTS_VBIS			REG_VPP_INTSTS,BIT5,5
#define VPP_VPU_INTSTS_PVBI			REG_VPP_INTSTS,BIT4,4
#define VPP_VPU_INTSTS				REG_VPP_INTSTS,0x70,4
#define VPP_GOVW_INTSTS_VBIE		REG_VPP_INTSTS,BIT2,2
#define VPP_GOVW_INTSTS_VBIS		REG_VPP_INTSTS,BIT1,1
#define VPP_GOVW_INTSTS_PVBI		REG_VPP_INTSTS,BIT0,0
#define VPP_GOVW_INTSTS				REG_VPP_INTSTS, 0x07, 0

//REG_VPP_INTEN,0x08
#define VPP_DISP_INTEN_VBIE			REG_VPP_INTEN,BIT26,26
#define VPP_DISP_INTEN_VBIS			REG_VPP_INTEN,BIT25,25
#define VPP_DISP_INTEN_PVBI			REG_VPP_INTEN,BIT24,24
#define VPP_DISP_INTEN				REG_VPP_INTEN,0x7000000,24
#define VPP_SCL_INTEN_VBIE			REG_VPP_INTEN,BIT18,18
#define VPP_SCL_INTEN_VBIS			REG_VPP_INTEN,BIT17,17
#define VPP_SCL_INTEN_PVBI			REG_VPP_INTEN,BIT16,16
#define VPP_SCL_INTEN				REG_VPP_INTEN, 0x70000, 16
#define VPP_GOVRH_INTEN_VBIE		REG_VPP_INTEN,BIT10,10
#define VPP_GOVRH_INTEN_VBIS		REG_VPP_INTEN,BIT9,9
#define VPP_GOVRH_INTEN_PVBI		REG_VPP_INTEN,BIT8,8
#define VPP_GOVRH_INTEN				REG_VPP_INTEN, 0x700, 8
#define VPP_VPU_INTEN_VBIE			REG_VPP_INTEN,BIT6,6
#define VPP_VPU_INTEN_VBIS			REG_VPP_INTEN,BIT5,5
#define VPP_VPU_INTEN_PVBI			REG_VPP_INTEN,BIT4,4
#define VPP_VPU_INTEN				REG_VPP_INTEN,0x70,4
#define VPP_GOVW_INTEN_VBIE			REG_VPP_INTEN,BIT2,2
#define VPP_GOVW_INTEN_VBIS			REG_VPP_INTEN,BIT1,1
#define VPP_GOVW_INTEN_PVBI			REG_VPP_INTEN,BIT0,0
#define VPP_GOVW_INTEN				REG_VPP_INTEN, 0x7, 0

//REG_VPP_WATCH_SEL,0x0c
#define VPP_WATCH_SEL				REG_VPP_WATCH_SEL, 0x1F, 0

//REG_VPP_SWRST1_SEL,0x10
#define VPP_VPU_RST					REG_VPP_SWRST1_SEL,BIT24,24
#define VPP_GE_RST					REG_VPP_SWRST1_SEL,BIT16,16
#define VPP_VID_RST					REG_VPP_SWRST1_SEL,BIT8,8
#define VPP_SCL_RST					REG_VPP_SWRST1_SEL,BIT0,0

//REG_VPP_SWRST2_SEL,0x14
#define VPP_DISP_RST				REG_VPP_SWRST2_SEL,BIT24,24
#define VPP_GOVW_RST				REG_VPP_SWRST2_SEL,BIT16,16
#define VPP_CEC_RST					REG_VPP_SWRST2_SEL,BIT12,12
#define VPP_DVO_RST					REG_VPP_SWRST2_SEL,BIT8,8
#define VPP_LVDS_RST				REG_VPP_SWRST2_SEL,BIT4,4
#define VPP_GOVRH_RST				REG_VPP_SWRST2_SEL,BIT0,0

//REG_VPP_DAC_SEL,0x18
#define VPP_DAC_SEL					REG_VPP_DAC_SEL,BIT0,0
#define VPP_DAC_SEL_TV				1
#define VPP_DAC_SEL_VGA				0

#if(WMT_SUB_PID >= WMT_PID_8710_B)
//REG_VPP_SWRST3_SEL,0x1C
#define VPP_HDMI_RST				REG_VPP_SWRST3_SEL,BIT0,0
#define VPP_DDC_RST					REG_VPP_SWRST3_SEL,BIT8,8

//REG_VPP_SSCG,0x20
#define VPP_SSCG_DISABLE			REG_VPP_SSCG,BIT0,0
#endif

#endif				/* WMT_VPP_REG_H */

