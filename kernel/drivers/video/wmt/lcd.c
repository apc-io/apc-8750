/*++ 
 * linux/drivers/video/wmt/lcd.c
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

#define LCD_C
// #define DEBUG
/*----------------------- DEPENDENCE -----------------------------------------*/
#include "lcd.h"
#include "vout.h"

/*----------------------- PRIVATE MACRO --------------------------------------*/
/* #define  LCD_XXXX  xxxx    *//*Example*/

/*----------------------- PRIVATE CONSTANTS ----------------------------------*/
/* #define LCD_XXXX    1     *//*Example*/

/*----------------------- PRIVATE TYPE  --------------------------------------*/
/* typedef  xxxx lcd_xxx_t; *//*Example*/
typedef struct {
	lcd_parm_t* (*get_parm)(int arg);
} lcd_device_t;

/*----------EXPORTED PRIVATE VARIABLES are defined in lcd.h  -------------*/
/*----------------------- INTERNAL PRIVATE VARIABLES - -----------------------*/
/* int  lcd_xxx;        *//*Example*/
lcd_device_t lcd_device_array[LCD_PANEL_MAX];
int lcd_panel_on = 1;
int lcd_pwm_enable;

extern vout_t vo_lcd_parm;

/*--------------------- INTERNAL PRIVATE FUNCTIONS ---------------------------*/
/* void lcd_xxx(void); *//*Example*/
#ifdef CONFIG_PWM_WMT
extern void pwm_set_enable(int no,int enable);
extern void pwm_set_freq(int no,unsigned int freq);
extern void pwm_set_level(int no,unsigned int level);

extern void pwm_set_scalar(int no,unsigned int scalar);
extern void pwm_set_period(int no,unsigned int period);
extern void pwm_set_duty(int no,unsigned int duty) ;
#endif

/*----------------------- Function Body --------------------------------------*/
/*----------------------- Backlight --------------------------------------*/
#ifndef CFG_LOADER
void lcd_blt_enable(int no,int enable)
{
	if( lcd_pwm_enable ) return;
#ifdef CONFIG_LCD_BACKLIGHT_WMT
	pwm_set_enable(no,enable);
#endif	
	return;
} /* End of vt8430_blt_enable */

void lcd_blt_set_level(int no,int level)
{
	if( lcd_pwm_enable ) return;
#ifdef CONFIG_LCD_BACKLIGHT_WMT
	pwm_set_level(no,level);
#endif	
	return;
}

void lcd_blt_set_freq(int no,unsigned int freq)
{
	if( lcd_pwm_enable ) return;
#ifdef CONFIG_LCD_BACKLIGHT_WMT
	pwm_set_freq(no,freq);
#endif	
	return;
}

void lcd_blt_set_pwm(int no,unsigned int scalar,unsigned int period,unsigned int duty)
{
	lcd_pwm_enable = 1;
	lcd_blt_id = no;
#ifdef CONFIG_PWM_WMT
	pwm_set_scalar(lcd_blt_id,scalar);
	pwm_set_period(lcd_blt_id,period);
	pwm_set_duty(lcd_blt_id,duty);
#endif					
}

/*----------------------- LCD --------------------------------------*/
static int __init lcd_arg_panel_id
(
	char *str			/*!<; // argument string */
)
{
	lcd_panel_t lcd_panel_id;
	sscanf(str,"%d",(int *) &lcd_panel_id);
	if( lcd_panel_id >= LCD_PANEL_MAX ){
		lcd_panel_id = LCD_PANEL_MAX;
	}
	DBGMSG(KERN_INFO "set lcd panel id = %d\n",lcd_panel_id);	
	vo_set_lcd_id(lcd_panel_id);
  	return 1;
} /* End of lcd_arg_panel_id */

__setup("lcdid=", lcd_arg_panel_id);
#endif
int lcd_panel_register
(
	int no,						/*!<; //[IN] device number */
	void (*get_parm)(int mode)	/*!<; //[IN] get info function pointer */
)
{
	lcd_device_t *p;

	if( no >= LCD_PANEL_MAX ){
		DBGMSG(KERN_ERR "*E* lcd device no max is %d !\n",LCD_PANEL_MAX);
		return -1;
	}

	p = &lcd_device_array[no];
	if( p->get_parm ){
		DBGMSG(KERN_ERR "*E* lcd device %d exist !\n",no);
		return -1;
	}
	p->get_parm = (void *) get_parm;
//	printk("lcd_device_register %d 0x%x\n",no,p->get_parm);
	return 0;
} /* End of lcd_device_register */

lcd_parm_t *lcd_get_parm(lcd_panel_t id,unsigned int arg)
{
	lcd_device_t *p;

	p = &lcd_device_array[id];
	if( p && p->get_parm ){
		return p->get_parm(arg);
	}
	return 0;
}

/*----------------------- vout device plugin --------------------------------------*/
void lcd_set_power_down(int enable)
{
	lcd_blt_enable(lcd_blt_id,!enable);
}

int lcd_set_mode(unsigned int *option)
{
	if( option ){
		if( (p_lcd = lcd_get_parm((lcd_panel_t) option[0],option[1])) ){
			printk("[LCD] %s (id %d,bpp %d)\n",p_lcd->name,option[0],option[1]);
		}
		else {
			printk("[LCD] *E* lcd %d not support\n",option[0]);
			return -1;
		}
		
		if( p_lcd->initial ){
			p_lcd->initial();
		}
		lcd_blt_set_level(lcd_blt_id,lcd_blt_level);
		lcd_blt_set_freq(lcd_blt_id,lcd_blt_freq);
		lcd_blt_enable(lcd_blt_id,0);
		option[2] = p_lcd->capability;
	}
	else {
		lcd_blt_enable(lcd_blt_id,0);
		if( p_lcd && p_lcd->uninitial ){
			p_lcd->uninitial();
		}
		p_lcd = 0;
	}
	return 0;
}

int lcd_check_plugin(int hotplug)
{
	return (p_lcd)? 1:0;
}

int lcd_config(vout_info_t *info)
{
//	info->resx = p_lcd->timing.hpixel;
//	info->resy = p_lcd->timing.vpixel;
//	info->fps = p_lcd->fps;
	return 0;
}

int lcd_init(void)
{
	extern vout_t vo_lcd_parm;
	vout_t *parm;
	lcd_panel_t lcd_panel_id;

    lcd_blt_id = VPP_BLT_PWM_NUM;
	lcd_blt_level = 75;
	lcd_blt_freq = 20000;

	// vo_set_lcd_id(LCD_CHILIN_LW0700AT9003);
	lcd_panel_id = vo_get_lcd_id();
	if( lcd_panel_id >= LCD_PANEL_MAX ){
		vout_register(VOUT_LCD,&vo_lcd_parm);
		return -1;
	}

	parm = &vo_lcd_parm;

#ifdef CFG_LOADER
	parm->option[0] = lcd_panel_id;			/* [LCD] option1 : panel id */
	parm->option[1] = g_display_param.op2;					/* [LCD] option2 : bit per pixel */
	p_lcd = lcd_get_parm(lcd_panel_id,g_display_param.op2);
#else
	parm->option[0] = lcd_panel_id;			/* [LCD] option1 : panel id */
	parm->option[1] = 24;					/* [LCD] option2 : bit per pixel */
	p_lcd = lcd_get_parm(lcd_panel_id,24);
#endif
	if( p_lcd == 0 ) return -1;
	parm->resx = p_lcd->timing.hpixel;
	parm->resy = p_lcd->timing.vpixel;
	parm->pixclk = p_lcd->timing.pixel_clock;
	return 0;
}

vout_dev_ops_t lcd_vout_dev_ops = {
	.mode = VOUT_LCD,

	.init = lcd_init,
	.set_power_down = lcd_set_power_down,
	.set_mode = lcd_set_mode,
	.config = lcd_config,
	.check_plugin = lcd_check_plugin,
//	.get_edid = lcd_get_edid,
};

int lcd_module_init(void)
{	
	extern vout_t *vo_parm_table[VOUT_MODE_MAX];

	vout_device_register(&lcd_vout_dev_ops);
	vo_parm_table[VOUT_LCD]->dev_ops = &lcd_vout_dev_ops;
	return 0;
} /* End of lcd_module_init */
module_init(lcd_module_init);
/*--------------------End of Function Body -----------------------------------*/
#undef LCD_C
