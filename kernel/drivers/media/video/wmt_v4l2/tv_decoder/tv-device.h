/*++ 
 * linux/drivers/media/video/wmt_v4l2/tv_decoder/tv-dev.h
 * WonderMedia v4l tv decoder device driver
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
#ifndef TV_DEVICE_H
/* To assert that only one occurrence is included */
#define TV_DEVICE_H

/*-------------------- MODULE DEPENDENCY -------------------------------------*/

/*-------------------- EXPORTED PRIVATE CONSTANTS ----------------------------*/


/*-------------------- EXPORTED PRIVATE TYPES---------------------------------*/
/* typedef  void  viaapi_xxx_t;  *//*Example*/

/*------------------------------------------------------------------------------
    Definitions of structures
------------------------------------------------------------------------------*/

/*-------------------- EXPORTED PRIVATE VARIABLES -----------------------------*/
#ifdef TV_DEVICE_C 
    #define EXTERN
#else
    #define EXTERN   extern
#endif 

#undef EXTERN
typedef enum {
	VID656_N,
	VID656_P,
	VID601_S1_N,
	VID601_S1_P,	
	VID601_S2_N,
	VID601_S2_P,
	VID_AUTO_SWITCH_ENABLE,
	VID_AUTO_SWITCH_DISABLE,
}vid_mode_e;


typedef enum {
	VIDDEV_STS_LOCK,
	VIDDEV_STS_TVSYS,
}viddev_sts_e;



/*--------------------- EXPORTED PRIVATE MACROS -------------------------------*/

/*--------------------- EXPORTED PRIVATE FUNCTIONS  ---------------------------*/
/* extern void  viaapi_xxxx(vdp_Void); *//*Example*/

int viddev_get_status(int mode);
int viddev_config(int mode);
int tv_init_ext_device(int width, int height);

    
#endif /* ifndef TV_DEVICE_H */

/*=== END tv-device.h ==========================================================*/
