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

#include <config.h>
#include <common.h>
#include <command.h>
#include <version.h>
#include <stdarg.h>
#include <linux/types.h>
#include <devices.h>
#include <linux/stddef.h>
#include <asm/byteorder.h>

#include "vpp.h"
#include "wmt_display.h"
#include "minivgui.h"
#include "hw/wmt-pwm.h"

extern struct fb_var_screeninfo vfb_var;
extern int text_x, text_y;

int display_init(int on, int force)
{
	mv_surface s;

	if ((g_display_vaild&DISPLAY_ENABLE) == DISPLAY_ENABLE) {
		if (force) { // force re-initial 
			if (g_display_param.vout == VPP_VOUT_LCD)
				lcd_blt_enable(g_pwm_setting.pwm_no, 0);
			govrh_set_dvo_enable(VPP_FLAG_DISABLE);
			govrh_set_tg_enable(VPP_FLAG_DISABLE);
			g_display_vaild = 0; 
		}
	} else 
		force = 1;
 
	if (force) {
		if (wmt_graphic_init()) {
			printf("wmt_graphic_init failed\n");
			return -1;
		} else
			printf("wmt_graphic_init ok\n");
		s.width = vfb_var.xres;
		s.height = vfb_var.yres;
		s.startAddr = (char *)g_fb_phy;
		s.bits_per_pixel = vfb_var.bits_per_pixel;
		s.lineBytes = s.width*(vfb_var.bits_per_pixel>>3);
		mv_initPrimary(&s);
	}
	if ((g_display_param.vout == VPP_VOUT_LCD) && (on != 0)) {
		printf("before lcd_blt_enable !!!!!!!!!\n");
		lcd_blt_enable(g_pwm_setting.pwm_no, 1);
		printf("lcd_blt_enable !!!!!!!!!\n");
	}
	text_x = 30;
	text_y = 30 - CHAR_HEIGHT;
	return 0;
}

int display_clear(void)
{

	return display_init(1, 0);
}

static int display_show(int format)
{
	if (g_img_phy == 0xFFFF) {
		printf("%s is not found , command (display show) is invaild\n", ENV_IMGADDR);
		return -1;
	}
	if (display_init(0, 0))
		return -1;

	printf("Loading BMP ..... ");
	if (mv_loadBmp((unsigned char *)g_img_phy)) {
		printf("failed\r\n");
		return -1;
	}
	printf("ok\n");
	if (g_display_param.vout == VPP_VOUT_LCD)
		lcd_blt_enable(g_pwm_setting.pwm_no, 1);

	return 0;
}

static int display_set_pwm(char *val, int on)
{

	if (val == NULL) {
		printf("error : can not get pwm parameter\n");
		return -1;
	}

	lcd_blt_enable(g_pwm_setting.pwm_no, 0);

	parse_pwm_params(NULL, val);

	if ((g_display_vaild&PWMDEFMASK) == PWMDEFTP) {
		lcd_blt_set_pwm(g_pwm_setting.pwm_no, g_pwm_setting.duty, g_pwm_setting.period);
	} else {
		// fan : may need to check PWM power ..
		pwm_set_period(g_pwm_setting.pwm_no, g_pwm_setting.period-1);
		pwm_set_duty(g_pwm_setting.pwm_no, g_pwm_setting.duty-1);
		pwm_set_control(g_pwm_setting.pwm_no, (g_pwm_setting.duty-1)? 0x35:0x8);
		pwm_set_gpio(g_pwm_setting.pwm_no, g_pwm_setting.duty-1);
		pwm_set_scalar(g_pwm_setting.pwm_no, g_pwm_setting.scalar-1);
	}

	lcd_blt_enable(g_pwm_setting.pwm_no, on);

	return 0;
}

static int wmt_display_main (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	int tmp;

	switch (argc) {
		case 0:
		case 1:	
			printf ("Usage:\n%s\n", cmdtp->usage);
		return 0;
		default:
			if (strncmp(argv[1], "init", 2) == 0) {
				if (argc == 3) {
					if (strcmp(argv[2], "force") == 0) {
						display_init(1, 1);
						return 0;
					}
				}
				display_init(1, 0);
				return 0;
			}
			if (strncmp(argv[1], "clear", 2) == 0) {
				display_clear();
				return 0;
			}
			if (strncmp(argv[1], "show", 2) == 0) {
                if (4 == argc) {
			        g_logo_x = simple_strtoul(argv[2], NULL, 10);
			        g_logo_y = simple_strtoul(argv[3], NULL, 10);
                    printf("(%d, %d) ", g_logo_x, g_logo_y);
                } else {
                    g_logo_x = -1;
                    g_logo_y = -1;
                }
                display_show(0);
				return 0;
			}
			if (strncmp(argv[1], "setpwm", 2) == 0) {
				if (argc == 3) {
					if (argv[2] != NULL) {
						display_set_pwm(argv[2], 1);
						return 0;
					}
				} else if (argc == 4) {
					tmp = simple_strtoul(argv[2], NULL, 10);
					if (argv[3] != NULL) {
						display_set_pwm(argv[3], tmp);
						return 0;
					}
				}
			}
			printf ("Usage:\n%s\n", cmdtp->usage);
			return 0;
	}
	return 0;
}

U_BOOT_CMD(
	display,	5,	1,	wmt_display_main,
	"show    - \n",
	NULL
);
