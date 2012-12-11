/*++ 
 * linux/drivers/video/wmt/vout.h
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

#ifndef VOUT_H
/* To assert that only one occurrence is included */
#define VOUT_H
/*-------------------- MODULE DEPENDENCY -------------------------------------*/
#include "vpp.h"
#include "sw_i2c.h"
#include "edid.h"

/*	following is the C++ header	*/
#ifdef	__cplusplus
extern	"C" {
#endif

/*-------------------- EXPORTED PRIVATE CONSTANTS ----------------------------*/
/* #define  VO_XXXX  1    *//*Example*/
// #define CONFIG_VOUT_EDID_ALLOC

/*-------------------- EXPORTED PRIVATE TYPES---------------------------------*/
/* typedef  void  vo_xxx_t;  *//*Example*/
typedef enum {
	VOUT_SD_ANALOG,
	VOUT_SD_DIGITAL,
	VOUT_LCD,
	VOUT_DVI,
	VOUT_HDMI,
	VOUT_DVO2HDMI,
	VOUT_LVDS,
	VOUT_VGA,
	VOUT_BOOT,
	VOUT_MODE_MAX,
	VOUT_MODE_ALL = VOUT_MODE_MAX
} vout_mode_t;

typedef enum {
	VOCTL_INIT,
	VOCTL_UNINIT,
	VOCTL_COMPATIBLE,
	VOCTL_VISIBLE,
	VOCTL_CONFIG,
	VOCTL_SUSPEND,
	VOCTL_RESUME,
	VOCTL_BRIGHTNESS,
	VOCTL_CONTRAST,
	VOCTL_CHKPLUG,
} vout_ctrl_t;	

typedef struct {
	int type;
	int resx;
	int resy;
	int bpp;
	int fps;
	unsigned int pixclk;
	unsigned int option;
} vout_info_t;

typedef struct {
	int fmt;			// sample bits
	int sample_rate;	// sample rate
	int channel;		// channel count
} vout_audio_t;

typedef struct vout_dev_ops {
	vout_mode_t mode;
	struct vout_dev_ops *next;
	
	int (*init)(void);
	void (*set_power_down)(int enable);
	int (*set_mode)(unsigned int *option);
	int (*config)(vout_info_t *info);
	int (*check_plugin)(int hotplug);
	int (*get_edid)(char *buf);
	int (*set_audio)(vout_audio_t *arg);
	int (*interrupt)(void);
	void (*poll)(void);
} vout_dev_ops_t;

typedef struct {
	int (*init)(int arg);
	int (*uninit)(int arg);
	int (*compatible)(int arg);
	int (*visible)(int arg);
	int (*config)(int arg);
	int (*suspend)(int arg);
	int (*resume)(int arg);
	int (*brightness)(int arg);
	int (*contrast)(int arg);
	int (*chkplug)(int arg);
	int (*get_edid)(int arg);
} vout_ops_t;

typedef struct {
	unsigned int status;	// VPP_VOUT_STS_XXX defined in com-vpp.h
	vout_ops_t *ops;
	vout_dev_ops_t *dev_ops;
	char name[10];
	unsigned int option[3];
	unsigned int vo_option;
	int resx;
	int resy;
	int pixclk;
#ifdef CONFIG_VOUT_EDID_ALLOC
	char *edid;
#else
	char edid[128*EDID_BLOCK_MAX];
#endif
} vout_t;

/*-------------------- EXPORTED PRIVATE VARIABLES -----------------------------*/
#ifdef VOUT_C /* allocate memory for variables only in vout.c */
#define EXTERN
#else
#define EXTERN   extern

#endif /* ifdef VOUT_C */

/* EXTERN int      vo_xxx; *//*Example*/
EXTERN int (*vout_board_info)(int arg);

#undef EXTERN

/*--------------------- EXPORTED PRIVATE MACROS -------------------------------*/
/* #define VO_XXX_YYY   xxxx *//*Example*/
/*--------------------- EXPORTED PRIVATE FUNCTIONS  ---------------------------*/
/* extern void  vo_xxx(void); *//*Example*/

int vout_register(vout_mode_t mode,vout_t *vo);
int vout_unregister(vout_mode_t mode);
int vout_device_register(vout_dev_ops_t *ops);
vout_dev_ops_t *vout_get_device(vout_dev_ops_t *ops);

int vout_config(vout_mode_t mode,vout_info_t *info);
int vout_set_mode(vout_mode_t mode,int on);
int vout_set_blank(vout_mode_t mode,int on);
int vout_control(vout_mode_t mode,int cmd,int arg);
int vout_suspend(vout_mode_t mode,int level);
int vout_resume(vout_mode_t mode,int level);
int vout_chkplug(vout_mode_t mode);
vout_t *vout_get_info(vout_mode_t mode);
void vout_set_int_type(int type);

int vout_init(vout_info_t *info);
int vout_exit(void);
void vout_plug_detect(vout_mode_t mode);
void vo_set_lcd_id(int id);
int vo_get_lcd_id(void);
int vo_i2c_proc(int id,unsigned int addr,unsigned int index,char *pdata,int len);
vpp_tvsys_t vo_res_to_tvsys(unsigned int resx,unsigned int resy,int interlace);
void vo_tvsys_to_res(vpp_tvsys_t tvsys,vout_info_t *info);
int vout_set_audio(vout_audio_t *arg);
void vout_change_status(vout_t *vo,int cmd,int arg,int ret);
char *vout_get_edid(vout_mode_t mode);
int vout_check_ratio_16_9(unsigned int resx,unsigned int resy);
vpp_timing_t *vout_find_video_mode(int no,vout_info_t *info);
int vout_find_edid_support_mode(unsigned int *resx,unsigned int *resy,unsigned int *fps,int r_16_9);

#ifdef	__cplusplus
}
#endif	

#endif /* ifndef VOUT_H */

/*=== END vout.h ==========================================================*/
