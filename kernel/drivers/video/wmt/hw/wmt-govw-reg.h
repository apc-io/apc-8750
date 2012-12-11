/*++ 
 * linux/drivers/video/wmt/hw/wmt-govw-reg.h
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

#ifndef WMT_GOVW_REG_H
#define WMT_GOVW_REG_H

#define WMT_FTBLK_GOVW

#define WMT_GOVW_RCYC_MIN	2		// 3T

#define REG_GOVW_BEGIN				(GOVW_BASE_ADDR+0x00)
#define REG_GOVW_REG_STS			(GOVW_BASE_ADDR+0x00)
#define REG_GOVW_RGB				(GOVW_BASE_ADDR+0x0C)
#define REG_GOVW_CSC_COEF1			(GOVW_BASE_ADDR+0x10)
#define REG_GOVW_CSC_COEF2			(GOVW_BASE_ADDR+0x14)
#define REG_GOVW_CSC_COEF3			(GOVW_BASE_ADDR+0x18)
#define REG_GOVW_CSC_COEF4			(GOVW_BASE_ADDR+0x1C)
#define REG_GOVW_CSC_COEF5			(GOVW_BASE_ADDR+0x20)
#define REG_GOVW_CSC_COEF6			(GOVW_BASE_ADDR+0x24)
#define REG_GOVW_CSC_MODE			(GOVW_BASE_ADDR+0x28)
#define REG_GOVW_CSC_ENABLE			(GOVW_BASE_ADDR+0x2C)
#define REG_GOVW_HD_STS				(GOVW_BASE_ADDR+0x84)
#define REG_GOVW_HD_YSA				(GOVW_BASE_ADDR+0x88)
#define REG_GOVW_HD_CSA				(GOVW_BASE_ADDR+0x8c)
#define REG_GOVW_HD_YBUF			(GOVW_BASE_ADDR+0x90)
#define REG_GOVW_HD_CBUF			(GOVW_BASE_ADDR+0x94)
#define REG_GOVW_HD_YPXL			(GOVW_BASE_ADDR+0x98)
#define REG_GOVW_HD_CPXL			(GOVW_BASE_ADDR+0x9c)
#define REG_GOVW_TG_STS				(GOVW_BASE_ADDR+0xa0)
#define REG_GOVW_TG_RD_CYC			(GOVW_BASE_ADDR+0xa4)
#define REG_GOVW_TG_H_PXL			(GOVW_BASE_ADDR+0xa8)
#define REG_GOVW_TG_V_LNE			(GOVW_BASE_ADDR+0xac)
#define REG_GOVW_TG_V_ACTBG			(GOVW_BASE_ADDR+0xb0)
#define REG_GOVW_TG_V_ACTEND		(GOVW_BASE_ADDR+0xb4)
#define REG_GOVW_TG_H_ACTBG			(GOVW_BASE_ADDR+0xb8)
#define REG_GOVW_TG_H_ACTEND		(GOVW_BASE_ADDR+0xbc)
#define REG_GOVW_TG_VBIE			(GOVW_BASE_ADDR+0xc0)
#define REG_GOVW_TG_PVBI			(GOVW_BASE_ADDR+0xc4)
#define REG_GOVW_TG_WATCHDOG		(GOVW_BASE_ADDR+0xc8)
#define REG_GOVW_INT				(GOVW_BASE_ADDR+0xcc)
#define REG_GOVW_END				(GOVW_BASE_ADDR+0xcc)

//REG_GOVW_REG_STS,0x00
#define GOVW_REG_LEVEL				REG_GOVW_REG_STS,BIT8,8
#define GOVW_REG_UPDATE				REG_GOVW_REG_STS,BIT0,0		// hw reg update in GOVW VBIE

// REG_GOVW_RGB,0x0C
#define GOVW_RGB_MODE				REG_GOVW_RGB,0x300,8
#define GOVW_COLFMT_RGB				REG_GOVW_RGB,BIT0,0

// 0x10 - 0x24 CSC

// REG_GOVW_CSC_MODE,0x28
#define GOVW_CSC_SUB16				REG_GOVW_CSC_MODE,BIT8,8
#define GOVW_CSC_RGB2YUV			REG_GOVW_CSC_MODE,BIT0,0

// REG_GOVW_CSC_ENABLE,0x2C
#define GOVW_CSC_ENABLE			REG_GOVW_CSC_ENABLE,BIT0,0

//REG_GOVW_HD_STS,0x84
#define GOVW_HD_MIF_ENABLE			REG_GOVW_HD_STS,BIT8,8
#define GOVW_HD_COLFMT				REG_GOVW_HD_STS,BIT0,0

//REG_GOVW_HD_YSA,0x88

//REG_GOVW_HD_CSA,0x8c

//REG_GOVW_HD_YBUF,0x90
#define GOVW_HD_YBUFWID				REG_GOVW_HD_YBUF, 0x7FF, 0

//REG_GOVW_HD_CBUF,0x94
#define GOVW_HD_CBUFWID				REG_GOVW_HD_CBUF, 0xFFF, 0

//REG_GOVW_HD_YPXL,0x98
#define GOVW_HD_YPXLWID				REG_GOVW_HD_YPXL, 0x7FF, 0

//REG_GOVW_HD_CPXL,0x9c
#define GOVW_HD_CPXLWID				REG_GOVW_HD_CPXL, 0x7FF, 0

//REG_GOVW_TG_STS,0xa0
#if(WMT_SUB_PID == WMT_PID_8710_B)
#define GOVW_TG_AUTO_TURN_OFF		REG_GOVW_TG_STS,BIT24,24	// WDT timeout auto turn off TG
#define GOVW_TG_TURN_OFF_IMMEDIATE	REG_GOVW_TG_STS,BIT16,16	// TG turn off in VBIS
#endif
#define GOVW_TG_WATCHDOG_ENABLE		REG_GOVW_TG_STS,BIT8,8
#define GOVW_TG_ENABLE				REG_GOVW_TG_STS,BIT0,0		// turn off in VBIS,turn on immediate

//REG_GOVW_TG_RD_CYC,0xa4
#define GOVW_TG_RDCYC				REG_GOVW_TG_RD_CYC, 0xFF, 0

//REG_GOVW_TG_H_PXL,0xa8
#define GOVW_TG_H_ALLPIXEL			REG_GOVW_TG_H_PXL, 0xFFF, 0

//REG_GOVW_TG_V_LNE,0xac
#define GOVW_TG_V_ALLLINE			REG_GOVW_TG_V_LNE, 0x7FF, 0

//REG_GOVW_TG_V_ACTBG,0xb0
#define GOVW_TG_V_ACTBG				REG_GOVW_TG_V_ACTBG, 0xFF, 0

//REG_GOVW_TG_V_ACTEND,0xb4
#define GOVW_TG_V_ACTEND			REG_GOVW_TG_V_ACTEND, 0x7FF, 0

//REG_GOVW_TG_H_ACTBG,0xb8
#define GOVW_TG_H_ACTBG				REG_GOVW_TG_H_ACTBG, 0x3FF, 0

//REG_GOVW_TG_H_ACTEND,0xbc
#define GOVW_TG_H_ACTEND			REG_GOVW_TG_H_ACTEND, 0xFFF, 0

//REG_GOVW_TG_VBIE,0xc0
#define GOVW_TG_VBIE				REG_GOVW_TG_VBIE, 0x7F, 0

//REG_GOVW_TG_PVBI,0xc4
#if(WMT_SUB_PID == WMT_PID_8710_A)
#define GOVW_TG_PVBI				REG_GOVW_TG_PVBI, 0x1F, 0
#else
#define GOVW_TG_PVBI				REG_GOVW_TG_PVBI, 0x7F, 0
#endif

//REG_GOVW_TG_WATCHDOG,0xc8
#if(WMT_SUB_PID == WMT_PID_8710_A)
#define GOVW_TG_WATCHDOG_VALUE		REG_GOVW_TG_WATCHDOG, 0x1FFF, 0
#else
#define GOVW_TG_WATCHDOG_VALUE		REG_GOVW_TG_WATCHDOG, 0xFFFFFFFF, 0
#endif

//REG_GOVW_INT,0xcc
#define GOVW_INTSTS_TGERR			REG_GOVW_INT,BIT20,20
#define GOVW_INTSTS_MIFYERR			REG_GOVW_INT,BIT19,19
#define GOVW_INTSTS_MIFCERR			REG_GOVW_INT,BIT18,18
#define GOVW_INT_TGERR_ENABLE		REG_GOVW_INT,BIT4,4
#define GOVW_INT_MIFYERR_ENABLE		REG_GOVW_INT,BIT3,3
#define GOVW_INT_MIFCERR_ENABLE		REG_GOVW_INT,BIT2,2

#endif				/* WMT_GOVM_REG_H */

