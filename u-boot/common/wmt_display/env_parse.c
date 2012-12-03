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

#include "lcd.h"
#include "wmt_display.h"

#if 0
int parse_display_params(char *name,long *ps)
{
    enum
    {
        idx_vout,
        idx_op1,
        idx_op2,
        idx_resx,
        idx_resy,
        idx_fps_pixclk,
        idx_max
    };

	char *p;
//    long ps[idx_max];
    char * endp;
    int i = 0;

    p = getenv(name);
    if (!p) {
        printf("please set %s\n", name);
        return -1;
    } else
        printf("display param (setting): %s\n", p);

    while (i < idx_max) {
        ps[i++] = simple_strtoul(p, &endp, 10);
        if (*endp == '\0')
            break;
        p = endp + 1;

        if (*p == '\0')
            break;
    }

    if (i != idx_max) {
        printf("display param Error: need %d arg count, but get %d\n", idx_max, i);
        return -1;
    }
    return 0;
}
#endif

int parse_gpio_operation(char *name)
{
    enum
    {
		idx_id,
		idx_act,
		idx_bitmap,
		idx_ctl,
		idx_oc,
		idx_od,
		idx_max
    };

	char *p;
    long ps[idx_max];
    char * endp;
    int i = 0;

	p = getenv(name);
    if (!p) {
        printf("please set %s , for lcd power control pin\n", name);
        return -1;
    } else
        printf("lcd power control pin (setting): %s\n", p);

    while (i < idx_max) {
        ps[i++] = simple_strtoul(p, &endp, 16);
        if (*endp == '\0')
            break;
        p = endp + 1;

        if (*p == '\0')
            break;        
    }
	g_lcd_pw_pin.id = ps[0];
	g_lcd_pw_pin.act = ps[1];
	g_lcd_pw_pin.bitmap = ps[2];
	g_lcd_pw_pin.ctl = ps[3];
	g_lcd_pw_pin.oc = ps[4];
	g_lcd_pw_pin.od = ps[5];
	return 0;
}

#if 0
int parse_display_tmr(char *name)
{
    enum
    {
        idx_pixel_clock,
        idx_option,
        idx_hsync,
        idx_hbp,
        idx_hpixel,
        idx_hfp,
		idx_vsync,
		idx_vbp,
		idx_vpixel,
		idx_vfp,
        idx_max
    };

	char *p;
    long ps[idx_max];
    char * endp;
    int i = 0;

    p = getenv(name);
    if (!p) {
        printf("please set %s , for timing setting\n", name);
        return -1;
    } else
        printf("display tmr (setting): %s\n", p);

	g_display_vaild &= ~TMRFROMENV;
    while (i < idx_max) {
        ps[i++] = simple_strtoul(p, &endp, 10);
        if (*endp == '\0')
            break;
        p = endp + 1;

        if (*p == '\0')
            break;
    }

    if (i != idx_max) {
        printf("display tmr Error: need %d arg count, but get %d\n", idx_max, i);
        return -1;
    }

	g_display_tmr.pixel_clock = ps[idx_pixel_clock]*1000;
	g_display_tmr.option = ps[idx_option];
	
	g_display_tmr.hsync = ps[idx_hsync];
	g_display_tmr.hbp = ps[idx_hbp];
	g_display_tmr.hpixel = ps[idx_hpixel];
	g_display_tmr.hfp = ps[idx_hfp];
	
	g_display_tmr.vsync = ps[idx_vsync];
	g_display_tmr.vbp = ps[idx_vbp];
	g_display_tmr.vpixel = ps[idx_vpixel];
	g_display_tmr.vfp = ps[idx_vfp];

	g_display_vaild |= TMRFROMENV;

    return 0;
}
#endif

/*
 val is used if setting comes from command , like
 	#display setpwm 0:2000:75
 otherwise , we get pwm setting from env ("name")
*/
int parse_pwm_params(char *name, char *val)
{
//pwmparam: no,scalar,period,duty
//comment:
//  no: PWM no. Range: 0 ~ 3
//  scalar: PWM scalar. Range: 1 ~ 1024
//  period: PWM period. Range: 1 ~ 4096
//  duty: PWM duty. Range: 1 ~ 4096
//  -- Note: All PWM parameters are denoted using decimal number
    enum
    {
        idx_pwm_no,
        idx_scalar,
        idx_period,
        idx_duty,
        idx_max
    };

    long ps[idx_max];
    char * p;
    char * p1;
    char * endp;
    int i = 0;
    struct wmt_pwm_setting_t setting;

	g_display_vaild &= ~PWMDEFMASK;
	g_display_vaild |= PWMDEFTP;
	g_pwm_setting.pwm_no = 0;
	g_pwm_setting.scalar = 0;
	g_pwm_setting.period = 2000;
	g_pwm_setting.duty = 75;

    if (val == NULL) {
    	p = getenv(name);
    	if (!p) {    	
        	printf("%s is not found , run PWM param (tp default) = 0:2000:75\n", name);
        	return 0;
    	}
	} else
		p = val;

	p1 = p;
    while (i < idx_max) {
        ps[i++] = simple_strtoul(p, &endp, 10);
        if (*endp == '\0')
            break;
        p = endp + 1;

        if (*p == '\0')
            break;
	}

	memset(&setting, 0, sizeof(struct wmt_pwm_setting_t));
	setting.config = ps[idx_pwm_no] >> 4;
	ps[idx_pwm_no] &= 0xf;
	if (i == idx_duty) { // run as taipei define ( freq , level )
		printf("PWM param (tp setting) : freq,level = %s\n", p1);
    	if(ps[idx_pwm_no] >= 0 && ps[idx_pwm_no] <= 3)
        	setting.pwm_no = ps[idx_pwm_no];
    	else {
        	printf("PWM param Error: no = %d, it's range should be 0 ~3\n", ps[idx_pwm_no]);
        	printf("PWM param (tp default) = 0:2000:750\n");
        	return 0;
    	}
		if (ps[idx_scalar] != 0)
			setting.period = ps[idx_scalar];
		else {
			printf("PWM param error : freq = 0\n");
        	printf("PWM param (tp default) = 0:2000:75\n");
        	return 0;
		}
		setting.duty = ps[idx_period];
	} else if (i == idx_max) {
		printf("PWM param (sz setting) : scalar,period,duty = %s\n", p1);
		g_display_vaild &= ~PWMDEFMASK;
		g_display_vaild |= PWMDEFSZ;
		g_pwm_setting.pwm_no = 0;
		g_pwm_setting.scalar = 319;
		g_pwm_setting.period = 1000;
		g_pwm_setting.duty = 750;

    	if(ps[idx_pwm_no] >= 0 && ps[idx_pwm_no] <= 3)
        	setting.pwm_no = ps[idx_pwm_no];
    	else {
        	printf("PWM param Error: no = %d, it's range should be 0 ~3\n", ps[idx_pwm_no]);
        	printf("So use defualt PWM param = 0:319:1000:750\n");
        	return 0;
    	}

		if (ps[idx_scalar] > 0 && ps[idx_scalar] <= 1024)
        	setting.scalar = ps[idx_scalar];
    	else {
        	printf("PWM param Error: scalar = %d, it's range should be 1 ~ 1024\n", ps[idx_scalar]);
        	printf("So use defualt PWM param = 0:319:1000:750\n");
        	return 0;
    	}

    	if (ps[idx_period] > 0 && ps[idx_period] < 4096)
        	setting.period = ps[idx_period];
    	else {
        	printf("PWM param Error: period = %d, it's range should be 1 ~ 4096\n", ps[idx_period]);
        	printf("So use defualt PWM param = 0:319:1000:750\n");
        	return 0;
    	}

    	if (ps[idx_duty] > 0 && ps[idx_duty] < 4096)
        	setting.duty = ps[idx_duty];
    	else {
        	printf("PWM param Error: duty = %d, it's range should be 0 ~ 4096\n", ps[idx_duty]);
        	printf("So use defualt PWM param = 0:319:1000:750\n");
        	return 0;
    	}
	} else {
		printf("PWM param Error: need %d arg count, but get %d\n", idx_max, i);
		printf("PWM param (default) = 0:2000:75 \n");
		return 0;
	}
	memcpy(&g_pwm_setting, &setting, sizeof(struct wmt_pwm_setting_t));
    return 0;
}
