/*++ 
 * linux/drivers/video/wmt/disp-attr.c
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

#define DISP_ATTR_C
// #define DEBUG
/*----------------------- DEPENDENCE -----------------------------------------*/
#include "disp.h"

#ifdef WMT_FTBLK_DISP
/*----------------------- PRIVATE MACRO --------------------------------------*/

/*----------------------- PRIVATE CONSTANTS ----------------------------------*/
/* #define DISP_XXXX    1     *//*Example*/
#define HW_DISP_LAST_HP				28
#define HW_DISP_DAC_SENSE           0x028e028e

/*----------------------- PRIVATE TYPE  --------------------------------------*/
/* typedef  xxxx disp_xxx_t; *//*Example*/

/*----------EXPORTED PRIVATE VARIABLES are defined in disp.h  -------------*/
/*----------------------- INTERNAL PRIVATE VARIABLES - -----------------------*/
/* int  disp_xxx;        *//*Example*/
disp_attr_t disp_attr_tbl[11]={
	{	/* NTSC */
	/*DISP_EN*/  1, /*IT_MOD_ONLY*/   0, /*DAC_OFF*/  0x0,
	//mode
	/*CB_ENABLE*/  0, /*LINE_SEL*/   1,
	/*VOS*/   1, /*PAL_N*/   0, /*PAL_NC*/   0,
	//Gain
	/*YAF*/ 0x60, /*UAF*/ 0x66, /*VAF*/ 0x8e,
	/*GAF*/ 0xb0, /*BAF*/ 0xb0, /*RAF*/ 0xb0,
	/*PYAF*/ 0xa0, /*PBAF*/ 0xaa, /*PRAF*/ 0xaa,
	/*SPYAF*/ 0xa0, /*SPBAF*/ 0xaa, /*SPRAF*/ 0xaa,
	//Sync
	/*SYNC_YPbPr*/ 0x2, /*SYNC_RGB*/ 2 ,
	/*THSYNC_WIDTH*/ 0x7f, /*SYNC_STEP*/ 0x1d,
	/*BLANK_LEVEL*/ 0x8f, /*BLANK_LEVEL2*/ 0x30, /*BLACK_LEVEL*/ 0x1a,
	//Burst
	/*FSCI */ 0x21ed7c00,
	/*PHASE_ADJ*//*197*/0x90, /*HUE_ADJ*//* 0x46*/0x83,
	/*BURST_AMP*/ 0x4b, /*BURST_STEP*/ 0x6,
	/*TBURST_START*/ 0x8d, /*TBURST_END*/ 0xd6,
	/*SW_SHIFT*/ 0x1, /*SHIFT_TYPE*/ 0x2,
	//Timing
	/*F_DIFF*/ 0x1, /*V_DIFF*/ 0x20c, /*H_DIFF*/0x2ea,
	/*FRAME_TOTAL*/ 0x20c, /*TH_TOTAL*/ 0x359,
	/*TVIDEO_START*/ 0xe0, /*TVIDEO_END */ 0x680,
	/*VBIE_LINE */   0x29, /*LAST_HP*/ HW_DISP_LAST_HP ,
	//Progressive
	/*P_GAF*/  0xe0, /*P_BAF*/ 0xe0, /*P_RAF*/  0xe0,
	/*P_PYAF*/ 0x8c, /*P_PBAF*/ 0x4c, /*P_PRAF*/ 0x4c,
	/*P_SPYAF*/ 0x9e, /*P_SPBAF*/ 0xa0, /*P_SPRAF*/ 0xa0,
	/*VGA_GAF*/ 0xa5, /*VGA_BAF*/ 0x90, /*VGA_RAF*/ 0x85,
	/*P_SYNC_YPbPr*/ 0x2, /*P_SYNC_RGB*/ 0x2  , /*P_SYNC_VGA*/ 0x0  ,
	/*P_THSYNC_WIDTH*/ 0x23, /*P_SYNC_STEP*/ 0x23,
	/*P_BLANK_LEVEL*/ 0x10a, /*P_BLANK_LEVEL2*/ 0xb8, /*P_BLACK_LEVEL*/ 0x10,
	/*DAC_SENSE*/ HW_DISP_DAC_SENSE, /*Reg18*/ 0x02, /*YC_DELAY*/ 0x3, /* P_YCDLY */ 0x0,
	},

	{	//NTSC-J
	/*DISP_EN*/  1, /*IT_MOD_ONLY*/   0, /*DAC_OFF*/  0x07,
	//mode
	/*CB_ENABLE*/  0, /*LINE_SEL*/   1,
	/*VOS*/   1, /*PAL_N*/   0, /*PAL_NC*/   0,
	//Gain
	/*YAF*/ 0x5e, /*UAF*/ 0x4f, /*VAF*/ 0x70,
	/*GAF*/ 0xb0, /*BAF*/ 0xb0, /*RAF*/ 0xb0,
	/*PYAF*/ 0x90, /*PBAF*/ 0x90, /*PRAF*/ 0x90,
	/*SPYAF*/ 0x90, /*SPBAF*/ 0x90, /*SPRAF*/ 0x90,
	//Sync
	/*SYNC_YPbPr*/ 0x2, /*SYNC_RGB*/ 2 ,
	/*THSYNC_WIDTH*/ 0x7f, /*SYNC_STEP*/ 0x18,
	/*BLANK_LEVEL*/ 0x40, /*BLANK_LEVEL2*/ 0x30, /*BLACK_LEVEL*/ 0x0,
	//Burst
	/*FSCI */ 0x21ed7c00,
	/*PHASE_ADJ*/ 354, /*HUE_ADJ*/ 0x3e,
	/*BURST_AMP*/ 0x43, /*BURST_STEP*/ 0x46,
	/*TBURST_START*/ 0x92, /*TBURST_END*/ 0xd6,
	/*SW_SHIFT*/ 0x0, /*SHIFT_TYPE*/ 0x0,
	//Timing
	/*F_DIFF*/ 0x1, /*V_DIFF*/ 0x20c, /*H_DIFF*/ 0x2ea,
	/*FRAME_TOTAL*/ 0x20c, /*TH_TOTAL*/ 0x359,
	/*TVIDEO_START*/ 0xe0, /*TVIDEO_END */ 0x680,
	/*VBIE_LINE */   0x29, /*LAST_HP*/  HW_DISP_LAST_HP,
	//Progressive
	/*P_GAF*/  0xe0, /*P_BAF*/ 0xe0, /*P_RAF*/  0xe0,
  	/*P_PYAF*/ 0x44, /*P_PBAF*/ 0xc0, /*P_PRAF*/ 0xc0,
	/*P_SPYAF*/ 0x44, /*P_SPBAF*/ 0xc0, /*P_SPRAF*/ 0xc0,
	/*VGA_GAF*/ 0xc0, /*VGA_BAF*/ 0xc0, /*VGA_RAF*/ 0xc0,
	/*P_SYNC_YPbPr*/ 0x2, /*P_SYNC_RGB*/ 0x2  , /*P_SYNC_VGA*/ 0x7  ,
	/*P_THSYNC_WIDTH*/ 0x23, /*P_SYNC_STEP*/ 0x23,
  	/*P_BLANK_LEVEL*/ 0x80, /*P_BLANK_LEVEL2*/ 0x180, /*P_BLACK_LEVEL*/ 0x8,
	/*DAC_SENSE*/ HW_DISP_DAC_SENSE, /*Reg18*/ 0x02, /*YC_DELAY*/ 0x2, /* P_YCDLY */ 0x0,
	},

	{	//NTSC-443
	/*DISP_EN*/  1, /*IT_MOD_ONLY*/   0, /*DAC_OFF*/  0x07,
	//mode
	/*CB_ENABLE*/  0, /*LINE_SEL*/   1,
	/*VOS*/   1, /*PAL_N*/   0, /*PAL_NC*/   0,
	//Gain
	/*YAF*/ 0x5e, /*UAF*/ 0x4f, /*VAF*/ 0x70,
	/*GAF*/ 0xb0, /*BAF*/ 0xb0, /*RAF*/ 0xb0,
	/*PYAF*/ 0x90, /*PBAF*/ 0x90, /*PRAF*/ 0x90,
	/*SPYAF*/ 0x90, /*SPBAF*/ 0x90, /*SPRAF*/ 0x90,
	//Sync
	/*SYNC_YPbPr*/ 0x2, /*SYNC_RGB*/ 2 ,
	/*THSYNC_WIDTH*/ 0x7f, /*SYNC_STEP*/ 0x18,
	/*BLANK_LEVEL*/ 0x40, /*BLANK_LEVEL2*/ 0x30, /*BLACK_LEVEL*/ 0x16,
	//Burst
	/*FSCI */ 0x21ed7c00,
	/*PHASE_ADJ*/ 530, /*HUE_ADJ*/ 0x40,
	/*BURST_AMP*/ 0x43, /*BURST_STEP*/ 0x46,
	/*TBURST_START*/ 0x92, /*TBURST_END*/ 0xd6,
	/*SW_SHIFT*/ 0x0, /*SHIFT_TYPE*/ 0x0,
	//Timing
	/*F_DIFF*/ 1, /*V_DIFF*/ 0x20c, /*H_DIFF*/ 0x2ea,
	/*FRAME_TOTAL*/ 0x20c, /*TH_TOTAL*/ 0x359,
	/*TVIDEO_START*/ 0xe0, /*TVIDEO_END */ 0x680,
	/*VBIE_LINE */   0x29, /*LAST_HP*/  HW_DISP_LAST_HP,
	//Progressive
	/*P_GAF*/  0xe0, /*P_BAF*/ 0xe0, /*P_RAF*/  0xe0,
  	/*P_PYAF*/ 0x44, /*P_PBAF*/ 0xc0, /*P_PRAF*/ 0xc0,
	/*P_SPYAF*/ 0x44, /*P_SPBAF*/ 0xc0, /*P_SPRAF*/ 0xc0,
	/*VGA_GAF*/ 0xc0, /*VGA_BAF*/ 0xc0, /*VGA_RAF*/ 0xc0,
	/*P_SYNC_YPbPr*/ 0x2, /*P_SYNC_RGB*/ 0x2  , /*P_SYNC_VGA*/ 0x7  ,
	/*P_THSYNC_WIDTH*/ 0x23, /*P_SYNC_STEP*/ 0x23,
  	/*P_BLANK_LEVEL*/ 0x80, /*P_BLANK_LEVEL2*/ 0x180, /*P_BLACK_LEVEL*/ 0x8,
	/*DAC_SENSE*/ HW_DISP_DAC_SENSE, /*Reg18*/ 0x02, /*YC_DELAY*/ 0x2, /* P_YCDLY */ 0x0,
	},

	{	//PAL
	/*DISP_EN*/  1, /*IT_MOD_ONLY*/   0, /*DAC_OFF*/  0x0,
	//mode
	/*CB_ENABLE*/  0, /*LINE_SEL*/   0,
	/*VOS*/  0, /*PAL_N*/   0, /*PAL_NC*/   0,
	//Gain
	/*YAF*/ 0x55, /*UAF*/ 0x55, /*VAF*/ 0x79,
	/*GAF*/ 0xb7, /*BAF*/ 0xb7, /*RAF*/ 0xb7,
	/*PYAF*/ 0xa0, /*PBAF*/ 0xaa, /*PRAF*/ 0xaa,
	/*SPYAF*/ 0xa0, /*SPBAF*/ 0xaa, /*SPRAF*/ 0xaa,
	//Sync
	/*SYNC_YPbPr*/ 0x2, /*SYNC_RGB*/ 0x2 ,
	/*THSYNC_WIDTH*/ 0x7f, /*SYNC_STEP*/ 0x28,
	/*BLANK_LEVEL*/ 0x95, /*BLANK_LEVEL2*/ 0x30, /*BLACK_LEVEL*/ 0x00,
	//Burst
	/*FSCI */ 0x2a060acb,
	/*PHASE_ADJ*/ /*812*/1220, /*HUE_ADJ*/ /*0x34*/0x55,
	/*BURST_AMP*/ 0x4f, /*BURST_STEP*/ 0xe,
	/*TBURST_START*/ 0x96, /*TBURST_END*/ 0xd2,
	/*SW_SHIFT*/ 0x0, /*SHIFT_TYPE*/ 0x0,
	//Timing
	/*F_DIFF*/ 0, /*V_DIFF*/ 1, /*H_DIFF*/ 0x2e8,
	/*FRAME_TOTAL*/ 0x270, /*TH_TOTAL*/ 0x35f,
	/*TVIDEO_START*/ 0xf0, /*TVIDEO_END */ 0x690,
	/*VBIE_LINE */   0x29, /*LAST_HP*/  HW_DISP_LAST_HP,
	//Progressive
	/*P_GAF*/  0xb0, /*P_BAF*/ 0xb0, /*P_RAF*/  0xb0,
	/*P_PYAF*/ 0x90, /*P_PBAF*/ 0x4c, /*P_PRAF*/ 0x4c,
	/*P_SPYAF*/ 0x9a, /*P_SPBAF*/ 0x9d, /*P_SPRAF*/ 0xa0,
	/*VGA_GAF*/ 0xc0, /*VGA_BAF*/ 0xc0, /*VGA_RAF*/ 0xc0,
	/*P_SYNC_YPbPr*/ 0x2, /*P_SYNC_RGB*/ 0x2  , /*P_SYNC_VGA*/ 0x7  ,
	/*P_THSYNC_WIDTH*/ 0x3f, /*P_SYNC_STEP*/ 0x2d,
  	/*P_BLANK_LEVEL*/ 0x12a, /*P_BLANK_LEVEL2*/ 0xb8, /*P_BLACK_LEVEL*/ 0x01,
	/*DAC_SENSE*/ HW_DISP_DAC_SENSE, /*Reg18*/ 0x02, /*YC_DELAY*/ 0x2, /* P_YCDLY */ 0x0,
	},

	{	//PALM
	/*DISP_EN*/  1, /*IT_MOD_ONLY*/   1, /*DAC_OFF*/  0x0,
	//mode
	/*CB_ENABLE*/  0, /*LINE_SEL*/   1,
	/*VOS*/   0, /*PAL_N*/   0, /*PAL_NC*/   0,
	//Gain
	/*YAF*/ 0x30, /*UAF*/ 0x2d, /*VAF*/ 0x40,
	/*GAF*/ 0xb0, /*BAF*/ 0xb0, /*RAF*/ 0xb0,
	/*PYAF*/ 0xb0, /*PBAF*/ 0xb0, /*PRAF*/ 0xb0,
	/*SPYAF*/ 0x5a, /*SPBAF*/ 0x5e, /*SPRAF*/ 0x87,
	//Sync
	/*SYNC_YPbPr*/ 0x2, /*SYNC_RGB*/ 2 ,
	/*THSYNC_WIDTH*/ 0x7f, /*SYNC_STEP*/ 0x65,
	/*BLANK_LEVEL*/ 0x40, /*BLANK_LEVEL2*/ 0x30, /*BLACK_LEVEL*/ 0x30,
	//Burst
	/*FSCI */ 0x21E6EFA3,
	/*PHASE_ADJ*/ 1870, /*HUE_ADJ*/ 0x55,
	/*BURST_AMP*/ 0x36, /*BURST_STEP*/ 0xc0,
	/*TBURST_START*/ 147, /*TBURST_END*/ 214,
	/*SW_SHIFT*/ 0x1, /*SHIFT_TYPE*/ 0x01,
	//Timing
	/*F_DIFF*/ 0, /*V_DIFF*/ 4, /*H_DIFF*/ 772,
	/*FRAME_TOTAL*/ 524, /*TH_TOTAL*/ 857,
	/*TVIDEO_START*/ 224, /*TVIDEO_END */ 1664,
	/*VBIE_LINE */   41, /*LAST_HP*/  HW_DISP_LAST_HP,
	//Progressive
	/*P_GAF*/  0xb0, /*P_BAF*/ 0xb0, /*P_RAF*/  0xb0,
	/*P_PYAF*/ 0xb0, /*P_PBAF*/ 0xb0, /*P_PRAF*/ 0xb0,
	/*P_SPYAF*/ 0x5a, /*P_SPBAF*/ 0x5e, /*P_SPRAF*/ 0x87,
	/*VGA_GAF*/ 0xc0, /*VGA_BAF*/ 0xc0, /*VGA_RAF*/ 0xc0,
	/*P_SYNC_YPbPr*/ 0x2, /*P_SYNC_RGB*/ 0x2  , /*P_SYNC_VGA*/ 0x7  ,
	/*P_THSYNC_WIDTH*/ 63, /*P_SYNC_STEP*/ 0xf0,
  	/*P_BLANK_LEVEL*/ 0xe0, /*P_BLANK_LEVEL2*/ 0x180, /*P_BLACK_LEVEL*/ 0x8,
	/*DAC_SENSE*/ HW_DISP_DAC_SENSE, /*Reg18*/ 0x02, /*YC_DELAY*/ 0x2, /* P_YCDLY */ 0x0,
	},

	{	//PAL60
	/*DISP_EN*/  1, /*IT_MOD_ONLY*/   0, /*DAC_OFF*/  0x0,
	//mode
	/*CB_ENABLE*/  0, /*LINE_SEL*/   1,
	/*VOS*/   0, /*PAL_N*/   0, /*PAL_NC*/   0,
	//Gain
	/*YAF*/ 0x30, /*UAF*/ 0x2d, /*VAF*/ 0x40,
	/*GAF*/ 0xb0, /*BAF*/ 0xb0, /*RAF*/ 0xb0,
	/*PYAF*/ 0xb0, /*PBAF*/ 0xb0, /*PRAF*/ 0xb0,
	/*SPYAF*/ 0xb0, /*SPBAF*/ 0xb0, /*SPRAF*/ 0xb0,
	//Sync
	/*SYNC_YPbPr*/ 0x2, /*SYNC_RGB*/ 0x2,
	/*THSYNC_WIDTH*/ 0x7f, /*SYNC_STEP*/ 0x18,
	/*BLANK_LEVEL*/ 0x70, /*BLANK_LEVEL2*/ 0x70, /*BLACK_LEVEL*/ 0x15,
	//Burst
	/*FSCI */ 0x2A098ACB,
	/*PHASE_ADJ*/ 1870, /*HUE_ADJ*/ 0x55,
	/*BURST_AMP*/ 61, /*BURST_STEP*/ 0x46,
	/*TBURST_START*/ 147, /*TBURST_END*/ 214,
	/*SW_SHIFT*/ 0x1, /*SHIFT_TYPE*/ 0x0,
	//Timing
	/*F_DIFF*/ 0, /*V_DIFF*/ 5, /*H_DIFF*/ 744,
	/*FRAME_TOTAL*/ 524, /*TH_TOTAL*/ 857,
	/*TVIDEO_START*/ 228, /*TVIDEO_END */ 1668,
	/*VBIE_LINE */   41, /*LAST_HP*/  HW_DISP_LAST_HP,
	//Progressive
	/*P_GAF*/  0xb0, /*P_BAF*/ 0xb0, /*P_RAF*/  0xb0,
	/*P_PYAF*/ 0xb0, /*P_PBAF*/ 0xb0, /*P_PRAF*/ 0xb0,
	/*P_SPYAF*/ 0xb0, /*P_SPBAF*/ 0xb0, /*P_SPRAF*/ 0xb0,
	/*VGA_GAF*/ 0xc0, /*VGA_BAF*/ 0xc0, /*VGA_RAF*/ 0xc0,
	/*P_SYNC_YPbPr*/ 0x2, /*P_SYNC_RGB*/ 0x2  , /*P_SYNC_VGA*/ 0x7  ,
	/*P_THSYNC_WIDTH*/ 63, /*P_SYNC_STEP*/ 0xf0,
  	/*P_BLANK_LEVEL*/ 0xe0, /*P_BLANK_LEVEL2*/ 0x180, /*P_BLACK_LEVEL*/ 0x8,
	/*DAC_SENSE*/ HW_DISP_DAC_SENSE, /*Reg18*/ 0x02, /*YC_DELAY*/ 0x2, /* P_YCDLY */ 0x0,
	},

	{	//PAL-N
	/*DISP_EN*/  1, /*IT_MOD_ONLY*/   1, /*DAC_OFF*/  0x0,
	//mode
	/*CB_ENABLE*/  0, /*LINE_SEL*/   0,
	/*VOS*/  0, /*PAL_N*/   1, /*PAL_NC*/   0,
	//Gain
	/*YAF*/ 0x30, /*UAF*/ 0x2d, /*VAF*/ 0x40,
	/*GAF*/ 0xb0, /*BAF*/ 0xb0, /*RAF*/ 0xb0,
	/*PYAF*/ 0xb0, /*PBAF*/ 0xb0, /*PRAF*/ 0xb0,
	/*SPYAF*/ 0xb0, /*SPBAF*/ 0xb0, /*SPRAF*/ 0xb0,
	//Sync
	/*SYNC_YPbPr*/ 0x2, /*SYNC_RGB*/ 0x2 ,
	/*THSYNC_WIDTH*/ 0x7f, /*SYNC_STEP*/ 0x19,
	/*BLANK_LEVEL*/ 0x7c, /*BLANK_LEVEL2*/ 0x7c, /*BLACK_LEVEL*/ 0x15,
	//Burst
	/*FSCI */ 0x2A098ACB,
	/*PHASE_ADJ*/ 1220, /*HUE_ADJ*/ 0x55,
	/*BURST_AMP*/ 0x42, /*BURST_STEP*/ 0x3a,
	/*TBURST_START*/ 149, /*TBURST_END*/ 210,
	/*SW_SHIFT*/ 0x1, /*SHIFT_TYPE*/ 0x11,
	//Timing
	/*F_DIFF*/ 0, /*V_DIFF*/ 2, /*H_DIFF*/ 768,
	/*FRAME_TOTAL*/ 624, /*TH_TOTAL*/ 863,
	/*TVIDEO_START*/ 240, /*TVIDEO_END */ 1680,
	/*VBIE_LINE */   41, /*LAST_HP*/  HW_DISP_LAST_HP,
	//Progressive
	/*P_GAF*/  0xb0, /*P_BAF*/ 0xb0, /*P_RAF*/  0xb0,
	/*P_PYAF*/ 0xb0, /*P_PBAF*/ 0xb0, /*P_PRAF*/ 0xb0,
	/*P_SPYAF*/ 0xb0, /*P_SPBAF*/ 0xb0, /*P_SPRAF*/ 0xb0,
	/*VGA_GAF*/ 0xc0, /*VGA_BAF*/ 0xc0, /*VGA_RAF*/ 0xc0,
	/*P_SYNC_YPbPr*/ 0x2, /*P_SYNC_RGB*/ 0x2  , /*P_SYNC_VGA*/ 0x7  ,
	/*P_THSYNC_WIDTH*/ 63, /*P_SYNC_STEP*/ 0xf0,
  	/*P_BLANK_LEVEL*/ 0xe0, /*P_BLANK_LEVEL2*/ 0x180, /*P_BLACK_LEVEL*/ 0x8,
	/*DAC_SENSE*/ HW_DISP_DAC_SENSE, /*Reg18*/ 0x02, /*YC_DELAY*/ 0x2, /* P_YCDLY */ 0x0,
	},

	{	//PAL-Nc
	/*DISP_EN*/  1, /*IT_MOD_ONLY*/   1, /*DAC_OFF*/  0x0,
	//mode
	/*CB_ENABLE*/  0, /*LINE_SEL*/   0,
	/*VOS*/  0, /*PAL_N*/   0, /*PAL_NC*/   0,
	//Gain
	/*YAF*/ 0x30, /*UAF*/ 0x2d, /*VAF*/ 0x40,
	/*GAF*/ 0xb0, /*BAF*/ 0xb0, /*RAF*/ 0xb0,
	/*PYAF*/ 0xb0, /*PBAF*/ 0xb0, /*PRAF*/ 0xb0,
	/*SPYAF*/ 0xb0, /*SPBAF*/ 0xb0, /*SPRAF*/ 0xb0,
	//Sync
	/*SYNC_YPbPr*/ 0x2, /*SYNC_RGB*/ 0x2 ,
	/*THSYNC_WIDTH*/ 0x7f, /*SYNC_STEP*/ 0x19,
	/*BLANK_LEVEL*/ 0x7c, /*BLANK_LEVEL2*/ 0x7c, /*BLACK_LEVEL*/ 0x15,
	//Burst
	/*FSCI */ 0x21F6941E,
	/*PHASE_ADJ*/ 1220, /*HUE_ADJ*/ 0x55,
	/*BURST_AMP*/ 0x42, /*BURST_STEP*/ 0x3a,
	/*TBURST_START*/ 149, /*TBURST_END*/ 210,
	/*SW_SHIFT*/ 0x1, /*SHIFT_TYPE*/ 0x01,
	//Timing
	/*F_DIFF*/ 0, /*V_DIFF*/ 2, /*H_DIFF*/ 768,
	/*FRAME_TOTAL*/ 624, /*TH_TOTAL*/ 863,
	/*TVIDEO_START*/ 240, /*TVIDEO_END */ 1680,
	/*VBIE_LINE */   41, /*LAST_HP*/  HW_DISP_LAST_HP,
	//Progressive
	/*P_GAF*/  0xb0, /*P_BAF*/ 0xb0, /*P_RAF*/  0xb0,
	/*P_PYAF*/ 0xb0, /*P_PBAF*/ 0xb0, /*P_PRAF*/ 0xb0,
	/*P_SPYAF*/ 0xb0, /*P_SPBAF*/ 0xb0, /*P_SPRAF*/ 0xb0,
	/*VGA_GAF*/ 0xc0, /*VGA_BAF*/ 0xc0, /*VGA_RAF*/ 0xc0,
	/*P_SYNC_YPbPr*/ 0x2, /*P_SYNC_RGB*/ 0x2  , /*P_SYNC_VGA*/ 0x7  ,
	/*P_THSYNC_WIDTH*/ 63, /*P_SYNC_STEP*/ 0xf0,
  	/*P_BLANK_LEVEL*/ 0xe0, /*P_BLANK_LEVEL2*/ 0x180, /*P_BLACK_LEVEL*/ 0x8,
	/*DAC_SENSE*/ HW_DISP_DAC_SENSE, /*Reg18*/ 0x02, /*YC_DELAY*/ 0x2, /* P_YCDLY */ 0x0,
	},

	{	/* 720P */
	/*DISP_EN*/  1, /*IT_MOD_ONLY*/   0, /*DAC_OFF*/  0x0,
	//mode
	/*CB_ENABLE*/  0, /*LINE_SEL*/   0,
	/*VOS*/   0, /*PAL_N*/   0, /*PAL_NC*/   0,
	//Gain
	/*YAF*/ 0x5e, /*UAF*/ 0x5e, /*VAF*/ 0x5e,
	/*GAF*/ 0xb0, /*BAF*/ 0xb0, /*RAF*/ 0xb0,
	/*PYAF*/ 0xa0, /*PBAF*/ 0xaa, /*PRAF*/ 0xaa,
	/*SPYAF*/ 0xa0, /*SPBAF*/ 0xaa, /*SPRAF*/ 0xaa,
	//Sync
	/*SYNC_YPbPr*/ 0x2, /*SYNC_RGB*/ 2 ,
	/*THSYNC_WIDTH*/ 0x7f, /*SYNC_STEP*/ 0x1d,
	/*BLANK_LEVEL*/ 0x81, /*BLANK_LEVEL2*/ 0x30, /*BLACK_LEVEL*/ 0x0,
	//Burst
	/*FSCI */ 0x2a098acb,
	/*PHASE_ADJ*//*197*/0x4C4, /*HUE_ADJ*//* 0x46*/0x55,
	/*BURST_AMP*/ 0x3d, /*BURST_STEP*/ 0x32,
	/*TBURST_START*/ 0xd2, /*TBURST_END*/ 0x96,
	/*SW_SHIFT*/ 0x0, /*SHIFT_TYPE*/ 0x0,
	//Timing
	/*F_DIFF*/ 0x1, /*V_DIFF*/ 0x20c, /*H_DIFF*/0x2ea,
	/*FRAME_TOTAL*/ 0x20c, /*TH_TOTAL*/ 0x671,
	/*TVIDEO_START*/ 0xe0, /*TVIDEO_END */ 0x680,
	/*VBIE_LINE */   0x29, /*LAST_HP*/ HW_DISP_LAST_HP ,
	//Progressive
	/*P_GAF*/  0xe0, /*P_BAF*/ 0xe0, /*P_RAF*/  0xe0,
	/*P_PYAF*/ 0x90, /*P_PBAF*/ 0x4c, /*P_PRAF*/ 0x4c,
	/*P_SPYAF*/ 0x9e, /*P_SPBAF*/ 0xa0, /*P_SPRAF*/ 0xa0,
	/*VGA_GAF*/ 0xa5, /*VGA_BAF*/ 0x90, /*VGA_RAF*/ 0x85,
	/*P_SYNC_YPbPr*/ 0x2, /*P_SYNC_RGB*/ 0x2  , /*P_SYNC_VGA*/ 0x0  ,
	/*P_THSYNC_WIDTH*/ 0x4f, /*P_SYNC_STEP*/ 0x50,
	/*P_BLANK_LEVEL*/ 0xf0, /*P_BLANK_LEVEL2*/ 0xb8, /*P_BLACK_LEVEL*/ 0x10,
	/*DAC_SENSE*/ HW_DISP_DAC_SENSE, /*Reg18*/ 0x02, /*YC_DELAY*/ 0x3, /* P_YCDLY */ 0x0,
	},
	{	/* 1080I */
	/*DISP_EN*/  1, /*IT_MOD_ONLY*/   1, /*DAC_OFF*/  0x0,
	//mode
	/*CB_ENABLE*/  0, /*LINE_SEL*/   0,
	/*VOS*/   0, /*PAL_N*/   0, /*PAL_NC*/   0,
	//Gain
	/*YAF*/ 0x4c, /*UAF*/ 0x40, /*VAF*/ 0x5b,
	/*GAF*/ 0xb0, /*BAF*/ 0xb0, /*RAF*/ 0xb0,
	/*PYAF*/ 0x90, /*PBAF*/ 0x4c, /*PRAF*/ 0x4c,
	/*SPYAF*/ 0xa0, /*SPBAF*/ 0xaa, /*SPRAF*/ 0xaa,
	//Sync
	/*SYNC_YPbPr*/ 0x2, /*SYNC_RGB*/ 2 ,
	/*THSYNC_WIDTH*/ 0x57, /*SYNC_STEP*/ 0x30,
	/*BLANK_LEVEL*/ 0x76, /*BLANK_LEVEL2*/ 0x17, /*BLACK_LEVEL*/ 0x15,
	//Burst
	/*FSCI */ 0x21F07BD6,
	/*PHASE_ADJ*//*197*/0x74E, /*HUE_ADJ*//* 0x46*/0x8,
	/*BURST_AMP*/ 0x3b, /*BURST_STEP*/ 0x46,
	/*TBURST_START*/ 0xd2, /*TBURST_END*/ 0x8b,
	/*SW_SHIFT*/ 0x0, /*SHIFT_TYPE*/ 0x0,
	//Timing
	/*F_DIFF*/ 0x1, /*V_DIFF*/ 0x20c, /*H_DIFF*/0x2ea,
	/*FRAME_TOTAL*/ 0x20c, /*TH_TOTAL*/ 0x899,
	/*TVIDEO_START*/ 0xe0, /*TVIDEO_END */ 0x680,
	/*VBIE_LINE */   0x29, /*LAST_HP*/ HW_DISP_LAST_HP ,
	//Progressive
	/*P_GAF*/  0xe0, /*P_BAF*/ 0xe0, /*P_RAF*/  0xe0,
	/*P_PYAF*/ 0x80, /*P_PBAF*/ 0x80, /*P_PRAF*/ 0x80,
	/*P_SPYAF*/ 0x9e, /*P_SPBAF*/ 0xa0, /*P_SPRAF*/ 0xa0,
	/*VGA_GAF*/ 0xa5, /*VGA_BAF*/ 0x90, /*VGA_RAF*/ 0x85,
	/*P_SYNC_YPbPr*/ 0x2, /*P_SYNC_RGB*/ 0x2  , /*P_SYNC_VGA*/ 0x0  ,
	/*P_THSYNC_WIDTH*/ 0x23, /*P_SYNC_STEP*/ 0x23,
	/*P_BLANK_LEVEL*/ 0x0, /*P_BLANK_LEVEL2*/ 0x10, /*P_BLACK_LEVEL*/ 0x10,
	/*DAC_SENSE*/ HW_DISP_DAC_SENSE, /*Reg18*/ 0x02, /*YC_DELAY*/ 0x3, /* P_YCDLY */ 0x0,
	},
	{	/* 1080P */
	/*DISP_EN*/  1, /*IT_MOD_ONLY*/   1, /*DAC_OFF*/  0x0,
	//mode
	/*CB_ENABLE*/  0, /*LINE_SEL*/   0,
	/*VOS*/   0, /*PAL_N*/   0, /*PAL_NC*/   0,
	//Gain
	/*YAF*/ 0x4c, /*UAF*/ 0x40, /*VAF*/ 0x5b,
	/*GAF*/ 0xb0, /*BAF*/ 0xb0, /*RAF*/ 0xb0,
	/*PYAF*/ 0x80, /*PBAF*/ 0x80, /*PRAF*/ 0x80,
	/*SPYAF*/ 0xa0, /*SPBAF*/ 0xaa, /*SPRAF*/ 0xaa,
	//Sync
	/*SYNC_YPbPr*/ 0x2, /*SYNC_RGB*/ 2 ,
	/*THSYNC_WIDTH*/ 0x58, /*SYNC_STEP*/ 0x30,
	/*BLANK_LEVEL*/ 0x81, /*BLANK_LEVEL2*/ 0x30, /*BLACK_LEVEL*/ 0x15,
	//Burst
	/*FSCI */ 0x21F07BD6,
	/*PHASE_ADJ*//*197*/0x74E, /*HUE_ADJ*//* 0x46*/0x8,
	/*BURST_AMP*/ 0x3b, /*BURST_STEP*/ 0x46,
	/*TBURST_START*/ 0xd2, /*TBURST_END*/ 0x8b,
	/*SW_SHIFT*/ 0x0, /*SHIFT_TYPE*/ 0x0,
	//Timing
	/*F_DIFF*/ 0x1, /*V_DIFF*/ 0x20c, /*H_DIFF*/0x2ea,
	/*FRAME_TOTAL*/ 0x20c, /*TH_TOTAL*/ 0x897,
	/*TVIDEO_START*/ 0xe0, /*TVIDEO_END */ 0x680,
	/*VBIE_LINE */   0x29, /*LAST_HP*/ HW_DISP_LAST_HP ,
	//Progressive
	/*P_GAF*/  0xe0, /*P_BAF*/ 0xe0, /*P_RAF*/  0xe0,
	/*P_PYAF*/ 0x8d, /*P_PBAF*/ 0x4c, /*P_PRAF*/ 0x4c,
	/*P_SPYAF*/ 0x9e, /*P_SPBAF*/ 0xa0, /*P_SPRAF*/ 0xa0,
	/*VGA_GAF*/ 0xa5, /*VGA_BAF*/ 0x90, /*VGA_RAF*/ 0x85,
	/*P_SYNC_YPbPr*/ 0x2, /*P_SYNC_RGB*/ 0x2  , /*P_SYNC_VGA*/ 0x0  ,
	/*P_THSYNC_WIDTH*/ 0x57, /*P_SYNC_STEP*/ 0x50,
	/*P_BLANK_LEVEL*/ 0xea, /*P_BLANK_LEVEL2*/ 0x0, /*P_BLACK_LEVEL*/ 0x15,
	/*DAC_SENSE*/ HW_DISP_DAC_SENSE, /*Reg18*/ 0x02, /*YC_DELAY*/ 0x3, /* P_YCDLY */ 0x0,
	},
};

disp_timing_t disp_timing_tbl[9] = {
	{	// NTSC-I
	19, 258, 261, 20, 259, 262, 524,
//	12, 5, 1, 524, 746, 224, 1664,
	12, 5, 0, 0, 0, 224, 1664,
	0, 0, 0, 0, 0,	
	},
	{	// PAL-I
	22, 309, 311, 23, 310, 313, 624,
//	19, 8, 0, 1, 744, 287, 1727,
	19, 8, 0, 0, 0, 224, 1664,
	0, 0, 0, 0, 0,	
	},
	{	// NTSC-P
	19, 258, 261, 20, 259, 262, 524,
	25, 10, 1, 524, 746, 224, 1664,
	0, 0, 0, 0, 0,
	},
	{	// PAL-P
	22, 309, 311, 23, 310, 313, 624,
	39, 16, 0, 1, 744, 224, 1664,
	0, 0, 0, 0, 0,
	},
	{	// EXT-NTSC-I
	19, 258, 261, 20, 259, 262, 524,
	12, 5, 0, 0, 0, 224, 1664,
	0, 2, 1665, 275, 1715,
	},
	{	// EXT-PAL-I
	22, 309, 311, 23, 310, 313, 624,
	19, 8, 0, 1, 47, 287, 1727,
	0, 0, 0, 287, 1727,
	},
	{	// 720P
	22, 309, 311, 23, 310, 313, 749,
	39, 16, 0, 1, 744, 593, 3153,
	0, 0, 0, 0, 0,
	},
	{	// 1080I
	20, 559, 562, 20, 559, 562, 1124,
	12, 5, 0, 0, 0, 435, 4275,
	0, 0, 0, 0, 0,
	},
	{	// 1080P
	20, 559, 562, 20, 559, 562, 1124,
	12, 5, 0, 0, 0, 464, 4304,
	0, 0, 0, 0, 0,
	}
};

/*--------------------- INTERNAL PRIVATE FUNCTIONS ---------------------------*/
/* void disp_xxx(void); *//*Example*/

/*----------------------- Function Body --------------------------------------*/
disp_attr_t *disp_get_attribute(vpp_tvsys_t type)
{
	return &disp_attr_tbl[type];
}	/* End of disp_get_attribute */

disp_timing_t *disp_get_timing(vpp_tvsys_t tvsys,vpp_flag_t interlace,vpp_flag_t ext)
{
	int type;

	switch( tvsys ){
		default:
		case VPP_TVSYS_NTSC:
			if( ext ){
				type = 4;
			}
			else {
				type = (interlace)? 0:2;
			}
			break;
		case VPP_TVSYS_PAL:
			if( ext ){
				type = 5;
			}
			else {
				type = (interlace)? 1:3;
			}
			break;
		case VPP_TVSYS_720P:
			type = 6;
			break;
		case VPP_TVSYS_1080I:
			type = 7;
			break;
		case VPP_TVSYS_1080P:
			type = 8;
			break;
	}
	return &disp_timing_tbl[type];
}


#endif	//WMT_FTBLK_DISP

