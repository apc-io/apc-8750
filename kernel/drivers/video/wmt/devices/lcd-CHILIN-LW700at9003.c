/*++ 
 * linux/drivers/video/wmt/lcd-CHILIN-LW700at9003.c
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

#define LCD_CHILIN_LW700AT9003_C
// #define DEBUG
/*----------------------- DEPENDENCE -----------------------------------------*/
#include "../lcd.h"

/*----------------------- PRIVATE MACRO --------------------------------------*/
/* #define  LCD_LW700AT9003_XXXX  xxxx    *//*Example*/


/*----------------------- PRIVATE CONSTANTS ----------------------------------*/
/* #define LCD_LW700AT9003_XXXX    1     *//*Example*/

/*----------------------- PRIVATE TYPE --------------------------------------*/
/* typedef  xxxx lcd_xxx_t; *//*Example*/

/*----------EXPORTED PRIVATE VARIABLES are defined in lcd.h  -------------*/
static void lcd_lw700at9003_power_on(void);
static void lcd_lw700at9003_power_off(void);

/*----------------------- INTERNAL PRIVATE VARIABLES ------------------------*/
/* int  lcd_xxx;        *//*Example*/
lcd_parm_t lcd_lw700at9003_parm = {
	.name = "CHILIN LW700AT9003",
	.fps = 48,						/* frame per second */
	.bits_per_pixel = 18,
	.capability = LCD_CAP_CLK_HI,
	.timing = {
		.pixel_clock = 33260000,	/* pixel clock */
		.option = 0,				/* option flags */

		.hsync = 40,				/* horizontal sync pulse */
		.hbp = 105,					/* horizontal back porch */
		.hpixel = 800,				/* horizontal pixel */
		.hfp = 105,					/* horizontal front porch */

		.vsync = 5,					/* vertical sync pulse */
		.vbp = 23,					/* vertical back porch */
		.vpixel = 480,				/* vertical pixel */
		.vfp = 22,					/* vertical front porch */
	},
	
	.initial = lcd_lw700at9003_power_on,
	.uninitial = lcd_lw700at9003_power_off,
};

/*--------------------- INTERNAL PRIVATE FUNCTIONS ---------------------------*/
/* void lcd_xxx(void); *//*Example*/

/*----------------------- Function Body --------------------------------------*/
static void lcd_lw700at9003_power_on(void)
{	
	DPRINT("lcd_lw700at9003_power_on\n");
	/* TODO */
}

static void lcd_lw700at9003_power_off(void)
{
	DPRINT("lcd_lw700at9003_power_off\n");
		
	/* TODO */
}

lcd_parm_t *lcd_lw700at9003_get_parm(int arg) 
{	
	lcd_lw700at9003_parm.bits_per_pixel = arg;
	return &lcd_lw700at9003_parm;
}

int lcd_lw700at9003_init(void){	
	int ret;	

	ret = lcd_panel_register(LCD_CHILIN_LW0700AT9003,(void *) lcd_lw700at9003_get_parm);	
	return ret;
} /* End of lcd_oem_init */
module_init(lcd_lw700at9003_init);

/*--------------------End of Function Body -----------------------------------*/
#undef LCD_CHILIN_LW700AT9003_C

