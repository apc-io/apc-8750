/*++ 
 * linux/drivers/media/video/wmt_v4l2/wmt-vidreg.h
 * WonderMedia v4l video input device driver
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
#ifndef WMT_VIDREG_H
/* To assert that only one occurrence is included */
#define WMT_VIDREG_H

/*-------------------- MODULE DEPENDENCY -------------------------------------*/

/*-------------------- EXPORTED PRIVATE CONSTANTS ----------------------------*/

#define WMT_VID_IRQ          IRQ_VPP_IRQ10   // this may be changed by chip

#define REG_BASE_VID         VID_BASE_ADDR

/*------------------------------------------------------------------------------
    Definitions of VID Registers
    About following definitions, please refer "WM3437 VID/CMOS Register List"
    
    Prefix meanings:
    REG_VID_xxx: used for both TV deccoder and CMOS
    REG_VID_TVDEC_xxx: used for TV encoder only
    REG_VID_CMOS_XXX:  used for CMOS only
------------------------------------------------------------------------------*/
#define REG_VID_TVDEC_EN            (REG_BASE_VID + 0x00) 
#define REG_VID_TVDEC_CTRL          (REG_BASE_VID + 0x04) 
#define REG_VID_Y0_SA               (REG_BASE_VID + 0x60)
#define REG_VID_C0_SA               (REG_BASE_VID + 0x64) 
#define REG_VID_WIDTH               (REG_BASE_VID + 0x68) 
#define REG_VID_LINE_WIDTH          (REG_BASE_VID + 0x6C) 
#define REG_VID_HEIGHT              (REG_BASE_VID + 0x70) 
#define REG_VID_MEMIF_EN            (REG_BASE_VID + 0x74)
#define REG_VID_OUTPUT_FORMAT       (REG_BASE_VID + 0x78)
#define REG_VID_H_SCALE             (REG_BASE_VID + 0x7C)
#define REG_VID_CMOS_EN             (REG_BASE_VID + 0x80)
#define REG_VID_CMOS_PIXEL_SWAP     (REG_BASE_VID + 0x84) 

#define REG_VID_INT_CTRL            (REG_BASE_VID + 0xF0)
#define REG_VID_STS                 (REG_BASE_VID + 0xF4)



/*-------------------- EXPORTED PRIVATE TYPES---------------------------------*/
/* typedef  void  viaapi_xxx_t;  *//*Example*/

/*------------------------------------------------------------------------------
    Definitions of structures
------------------------------------------------------------------------------*/


/*-------------------- EXPORTED PRIVATE VARIABLES -----------------------------*/
#ifdef WMT_VID_C 
    #define EXTERN
#else
    #define EXTERN   extern
#endif 

#undef EXTERN

/*--------------------- EXPORTED PRIVATE MACROS -------------------------------*/

/*--------------------- EXPORTED PRIVATE FUNCTIONS  ---------------------------*/
/* extern void  viaapi_xxxx(vdp_Void); *//*Example*/

    
#endif /* ifndef WMT_VIDREG_H */

/*=== END wmt-vidreg.h ==========================================================*/
