/*++ 
 * linux/drivers/media/video/wmt_v4l2/cmos/cmos-dev-siv.h
 * WonderMedia v4l cmos device driver
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

#ifndef CMOS_DEV_SIV_H
/* To assert that only one occurrence is included */
#define CMOS_DEV_SIV_H
/*-------------------- MODULE DEPENDENCY -------------------------------------*/

/*-------------------- EXPORTED PRIVATE CONSTANTS ----------------------------*/


/*-------------------- EXPORTED PRIVATE TYPES---------------------------------*/

/*------------------------------------------------------------------------------
    Definitions of structures
------------------------------------------------------------------------------*/

/*-------------------- EXPORTED PRIVATE VARIABLES -----------------------------*/
#ifdef CMOS_DEV_SIV_C 
    #define EXTERN
#else
    #define EXTERN   extern
#endif 

#undef EXTERN

/*--------------------- EXPORTED PRIVATE MACROS -------------------------------*/

/*--------------------- EXPORTED PRIVATE FUNCTIONS  ---------------------------*/

int cmos_siv120b_identify(void);
int cmos_init_siv120b(int width, int height, int cmos_v_flip, int cmos_h_flip);
int cmos_exit_siv120b(void);


int cmos_siv120d_identify(void);
int cmos_init_siv120d(int width, int height, int cmos_v_flip, int cmos_h_flip);
int cmos_exit_siv120d(void);


    
#endif /* ifndef CMOS_DEV_SIV_H */

/*=== END cmos-dev-siv.h ==========================================================*/
