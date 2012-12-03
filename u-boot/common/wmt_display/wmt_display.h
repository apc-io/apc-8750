/*++ 
Copyright (c) 2010 WonderMedia Technologies, Inc.

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software 
Foundation, either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details. You
should have received a copy of the GNU General Public License along with this
program. If not, see http://www.gnu.org/licenses/>.

WonderMedia Technologies, Inc.
4F, 531, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C.
--*/

#ifndef __WM_LOGO_H__
#define __WM_LOGO_H__

#include "vpp.h"

typedef struct
{
    int     width;
    int     height;
    int     lineBytes;
    unsigned int bits_per_pixel;
	char *	startAddr;
}mv_surface;

typedef struct
{
    int	     left;
    int      top;
    int      right;
    int      bottom;
}mv_Rect;


#define ENV_DISPLAY_PARM "wmt.display.param"
#define ENV_DISPLAY_PARM2 "wmt.display.param2"
#define ENV_DISPLAY_TMR "wmt.display.tmr"
#define ENV_DISPLAY_PWM "wmt.display.pwm"
#define ENV_MEMTOTAL	"memtotal"
#define ENV_IMGADDR	"wmt.display.logoaddr"
#define ENV_LCD_POWERON	"wmt.lcd.poweron"
#define ENV_DIRECTPATH "wmt.display.direct_path"
#define ENV_DIRECTFB "wmt.display.fb"
#define ENV_PMEM "wmt.pmem.param"
#define ENV_LCD_POWER "wmt.gpo.lcd"

struct wmt_display_param_t {
	unsigned int vout;
	unsigned int op1;
	unsigned int op2;
	unsigned int resx;
	unsigned int resy;
	unsigned int fps_pixclk;
};

struct wmt_pwm_setting_t {
	unsigned int pwm_no;
	unsigned int scalar;
	unsigned int period;
	unsigned int duty;
	unsigned int config;
};

struct gpio_operation_t {
	unsigned int id;
	unsigned int act;
	unsigned int bitmap;
	unsigned int ctl;
	unsigned int oc;
	unsigned int od;
};

#ifdef	WMT_DISPLAY
#define EXTERN
#else
#define EXTERN extern
#endif

EXTERN struct wmt_display_param_t g_display_param;
EXTERN struct wmt_display_param_t g_display_param2;
EXTERN struct wmt_pwm_setting_t g_pwm_setting;
EXTERN vpp_timing_t g_display_tmr;
EXTERN struct gpio_operation_t g_lcd_pw_pin;
EXTERN unsigned int g_fb_phy;
EXTERN unsigned int g_img_phy;
EXTERN int g_logo_x;
EXTERN int g_logo_y;

/*
 0 : not initial
 not 0 : 
 bit 0 :
 	1 : initial ok
 	0 : not initial
 bit1 :
	1 : get tmr from env
	0 : get tmr auto
 bit[2:3] :
 	00 : run pwm default (freq, level)
 	01 : run pwm from env (freq, level)
 	10 : run pwm from env (scalar, period, duty)
 	11 : mask for pwm env config
*/
EXTERN int g_display_vaild;

#define DISPLAY_ENABLE 1
#define TMRFROMENV 1<<1
#define PWMDEFTP 1<<2
#define PWMDEFSZ 1<<3
#define PWMDEFMASK 0x0c

// cmd_textout
#define CHAR_WIDTH 12
#define CHAR_HEIGHT 22  // 16

int display_init(int on, int force);
int mv_loadBmp(unsigned char* fileBuffer);
int parse_display_tmr(char *name);
int parse_pwm_params(char *name, char *val);
int parse_gpio_operation(char *name);
int wmt_graphic_init(void);
int uboot_vpp_alloc_framebuffer(unsigned int resx,unsigned int resy);
void lcd_blt_set_pwm(int no, int level, int freq);
int vpp_i2c_write(unsigned int addr,unsigned int index, unsigned char *pdata,int len);
int vpp_i2c_read(unsigned int addr,unsigned int index, unsigned char *pdata,int len);
#undef EXTERN


#endif /* __WM_LOGO_H__ */
