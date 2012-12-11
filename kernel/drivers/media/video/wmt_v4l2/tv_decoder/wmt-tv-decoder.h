/*++ 
 * linux/drivers/media/video/wmt_v4l2/tv_decoder/wmt-tv-decoder.h
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
#ifndef TV_DECODER_H
/* To assert that only one occurrence is included */
#define TV_DECODER_H


/*-------------------- MODULE DEPENDENCY -------------------------------------*/

/*-------------------- EXPORTED PRIVATE CONSTANTS ----------------------------*/

#include "../wmt-vid.h"


#define MAX_FB_IN_QUEUE            10

/*-------------------- EXPORTED PRIVATE TYPES---------------------------------*/
/* typedef  void  viaapi_xxx_t;  *//*Example*/

typedef enum {
    STS_TV_DECODER_READY    = 0,
    STS_TV_DECODER_WAIT     = 0x0001,
    STS_TV_DECODER_RUNNING  = 0x0002,
    STS_TV_DECODER_FB_DONE  = 0x0004
} tv_decoder_status;

/*------------------------------------------------------------------------------
    Definitions of structures
------------------------------------------------------------------------------*/

typedef struct {
    VID_FB_M;
    
  #ifdef __KERNEL__
    struct list_head list;
  #endif
} tv_decoder_fb_t;

typedef struct {
  #ifdef __KERNEL__
    struct list_head  head;
  #endif

	unsigned int  frame_size;
	unsigned int  width;
	unsigned int  height;

	unsigned int  dft_y_addr;
	unsigned int  dft_c_addr;
    
    tv_decoder_fb_t     fb_pool[MAX_FB_IN_QUEUE];
    unsigned int  fb_cnt;
    
    unsigned int  streamoff;
    unsigned int  dqbuf;

    tv_decoder_status   _status;
    unsigned int  _timeout;
    
} tv_decoder_drvinfo_t;


/*-------------------- EXPORTED PRIVATE VARIABLES -----------------------------*/
#ifdef WMT_TV_DECODER_C 
    #define EXTERN
#else
    #define EXTERN   extern
#endif 

#undef EXTERN

/*--------------------- EXPORTED PRIVATE MACROS -------------------------------*/

/*--------------------- EXPORTED PRIVATE FUNCTIONS  ---------------------------*/
/* extern void  viaapi_xxxx(vdp_Void); *//*Example*/

int wmt_tv_decoder_open(tv_decoder_drvinfo_t *drv);
int wmt_tv_decoder_close(tv_decoder_drvinfo_t *drv);

int wmt_tv_decoder_querycap(tv_decoder_drvinfo_t *drv, struct v4l2_capability *cap);
int wmt_tv_decoder_s_fmt(tv_decoder_drvinfo_t *drv, struct v4l2_format *format);
int wmt_tv_decoder_reqbufs(tv_decoder_drvinfo_t *drv, struct v4l2_requestbuffers *rb);
int wmt_tv_decoder_querybuf(tv_decoder_drvinfo_t *drv, struct v4l2_buffer *buf);
int wmt_tv_decoder_qbuf(tv_decoder_drvinfo_t *drv, struct v4l2_buffer *buf);
int wmt_tv_decoder_dqbuf(tv_decoder_drvinfo_t *drv, struct v4l2_buffer *buf);
    
#endif /* ifndef TV_DECODER_H */

/*=== END wmt-tv_decoder.h ==========================================================*/
