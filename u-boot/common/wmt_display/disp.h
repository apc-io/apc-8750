/*++ 
 * linux/drivers/video/wmt/disp.h
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

#ifdef WMT_FTBLK_DISP

#ifndef DISP_H
#define DISP_H

typedef struct {
	unsigned char DISP_ENABLE;
	unsigned char IT_MOD_ONLY;
	unsigned char DAC_OFF;
	
	//mode
	unsigned char CB_ENABLE;
	unsigned char LINE_SEL;
	unsigned char VOS;
	unsigned char PAL_N;
	unsigned char PAL_NC;
	
	//gain
	unsigned char YAF;
	unsigned char UAF;
	unsigned char VAF;
	unsigned char GAF;
	unsigned char BAF;
	unsigned char RAF;
	unsigned char PYAF;
	unsigned char PBAF;
	unsigned char PRAF;
	unsigned char SPYAF;
	unsigned char SPBAF;
	unsigned char SPRAF;
	
	//sync
	unsigned char  SYNC_YPbPr;
	unsigned char  SYNC_RGB;
	unsigned char  THSYNC_WIDTH;
	unsigned short SYNC_STEP;
	unsigned short BLANK_LEVEL;
	unsigned short BLANK_LEVEL2;
	unsigned char  BLACK_LEVEL;
	
	//burst
	int FSCI ;
	unsigned short PHASE_ADJ;
	unsigned short HUE_ADJ;
	unsigned short BURST_AMP;
	unsigned short BURST_STEP;
	unsigned short TBURST_START;
	unsigned short TBURST_END;
	unsigned char  SW_SHIFT;
	unsigned char  SHIFT_TYPE;
	
	//timing
	unsigned short F_DIFF;
	unsigned short V_DIFF;
	unsigned short H_DIFF;
	unsigned short FRAME_TOTAL;
	unsigned short TH_TOTAL;
	unsigned short TVIDEO_START;
	unsigned short TVIDEO_END ;
	unsigned short VBIE_LINE ;
	unsigned char  LAST_HP;
	
	//progressive
	unsigned char P_GAF;
	unsigned char P_BAF;
	unsigned char P_RAF;
	unsigned char P_PYAF;
	unsigned char P_PBAF;
	unsigned char P_PRAF;
	unsigned char P_SPYAF;
	unsigned char P_SPBAF;
	unsigned char P_SPRAF;
	unsigned char VGA_GAF;
	unsigned char VGA_BAF;
	unsigned char VGA_RAF;
	unsigned char P_SYNC_YPbPr;
	unsigned char P_SYNC_RGB;
	unsigned char P_SYNC_VGA;
	unsigned char P_THSYNC_WIDTH;
	unsigned short P_SYNC_STEP;
	unsigned short P_BLANK_LEVEL;
	unsigned short P_BLANK_LEVEL2;
	unsigned char P_BLACK_LEVEL;

	// 
	unsigned int DAC_SENSE;
	unsigned int Reg18;
	unsigned short YC_DELAY;
	unsigned short P_YCDLY;
}disp_attr_t;

typedef struct {
	unsigned int F1_BEGIN;
	unsigned int F1_END;
	unsigned int F1_TOTAL;
	unsigned int F2_BEGIN;
	unsigned int F2_END;
	unsigned int F2_TOTAL;
	unsigned int FRAME_TOTAL;
	
	unsigned int VBIE_LINE;
	unsigned int PVBI_LINE;
	unsigned int F_DIFF;
	unsigned int V_DIFF;
	unsigned int H_DIFF;
	
	unsigned int H_BEGIN;
	unsigned int H_END;
	
	unsigned int EXT_F_DIFF;
	unsigned int EXT_V_DIFF;
	unsigned int EXT_H_DIFF;
	unsigned int EXT_H_BEGIN;
	unsigned int EXT_H_END;
} disp_timing_t;

typedef struct {
	VPP_MOD_BASE;

	vpp_tvsys_t tvsys;
	vpp_tvconn_t tvconn;
	unsigned int dac_sense_cnt;
	unsigned int dac_sense_val;
} disp_mod_t;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef DISP_C
#define EXTERN
#else
#define EXTERN extern
#endif

EXTERN disp_mod_t *p_disp;

EXTERN disp_attr_t *disp_get_attribute(vpp_tvsys_t type);
EXTERN disp_timing_t *disp_get_timing(vpp_tvsys_t tvsys,vpp_flag_t interlace,vpp_flag_t ext);

EXTERN void disp_set_tvsys(vpp_tvsys_t tvsys);
EXTERN void disp_set_tvconn(vpp_tvconn_t tvconn);
EXTERN void disp_set_colorbar(vpp_flag_t enable,int width, int inverse);
EXTERN void disp_set_enable(vpp_flag_t enable);
EXTERN void disp_EXTTV_set_enable(vpp_flag_t enable);
EXTERN void disp_DAC_set_pwrdn(unsigned int dac,vpp_flag_t pwrdn);
EXTERN int disp_mod_init(void);
EXTERN void disp_set_mode(vpp_tvsys_t tvsys,vpp_tvconn_t tvconn);
EXTERN int disp_DAC_sense(void);
EXTERN vdo_color_fmt disp_get_color_format(void);
EXTERN int disp_get_cur_field(void);

#undef EXTERN

#ifdef __cplusplus
}
#endif
#endif	//DISP_H
#endif	//WMT_FTBLK_DISP
