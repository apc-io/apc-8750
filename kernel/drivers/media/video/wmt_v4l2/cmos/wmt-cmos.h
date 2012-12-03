/*++ 
 * linux/drivers/media/video/wmt_v4l2/cmos/wmt-cmos.h
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

#ifndef WMT_CMOS_H
/* To assert that only one occurrence is included */
#define WMT_CMOS_H

/*-------------------- MODULE DEPENDENCY -------------------------------------*/

/*-------------------- EXPORTED PRIVATE CONSTANTS ----------------------------*/

#include "../wmt-vid.h"


#define MAX_FB_IN_QUEUE            10

/*-------------------- EXPORTED PRIVATE TYPES---------------------------------*/
/* typedef  void  viaapi_xxx_t;  *//*Example*/

typedef enum {
    STS_CMOS_READY    = 0,
    STS_CMOS_WAIT     = 0x0001,
    STS_CMOS_RUNNING  = 0x0002,
    STS_CMOS_FB_DONE  = 0x0004
} cmos_status;

/*------------------------------------------------------------------------------
    Definitions of structures
------------------------------------------------------------------------------*/

typedef struct {
    VID_FB_M;
    
  #ifdef __KERNEL__
    struct list_head list;
  #endif
} cmos_fb_t;

typedef struct {
  #ifdef __KERNEL__
    struct list_head  head;
  #endif

	unsigned int  frame_size;
	unsigned int  width;
	unsigned int  height;

	unsigned int  dft_y_addr;
	unsigned int  dft_c_addr;
    
    cmos_fb_t     fb_pool[MAX_FB_IN_QUEUE];
    unsigned int  fb_cnt;
    
    unsigned int  streamoff;
    unsigned int  dqbuf;

    cmos_status   _status;
    unsigned int  _timeout;
    
} cmos_drvinfo_t;


/*-------------------- EXPORTED PRIVATE VARIABLES -----------------------------*/
#ifdef WMT_CMOS_C 
    #define EXTERN
#else
    #define EXTERN   extern
#endif 

#undef EXTERN

/*--------------------- EXPORTED PRIVATE MACROS -------------------------------*/

/*--------------------- EXPORTED PRIVATE FUNCTIONS  ---------------------------*/
/* extern void  viaapi_xxxx(vdp_Void); *//*Example*/

int wmt_cmos_open(cmos_drvinfo_t *drv);
int wmt_cmos_close(cmos_drvinfo_t *drv);

int wmt_cmos_querycap(cmos_drvinfo_t *drv, struct v4l2_capability *cap);
int wmt_cmos_s_fmt(cmos_drvinfo_t *drv, struct v4l2_format *format);
int wmt_cmos_reqbufs(cmos_drvinfo_t *drv, struct v4l2_requestbuffers *rb);
int wmt_cmos_querybuf(cmos_drvinfo_t *drv, struct v4l2_buffer *buf);
int wmt_cmos_qbuf(cmos_drvinfo_t *drv, struct v4l2_buffer *buf);
int wmt_cmos_dqbuf(cmos_drvinfo_t *drv, struct v4l2_buffer *buf);
    
#endif /* ifndef WMT_CMOS_H */

/*=== END wmt-cmos.h ==========================================================*/
