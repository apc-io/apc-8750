/*++ 
 * linux/drivers/video/wmt/lcd-EKING_EK08009-70135.c
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

#define LCD_EKING_EK08009_C
// #define DEBUG
/*----------------------- DEPENDENCE -----------------------------------------*/
#include "../lcd.h"

/*----------------------- PRIVATE MACRO --------------------------------------*/
/* #define  LCD_EK08009_XXXX  xxxx    *//*Example*/

/*----------------------- PRIVATE CONSTANTS ----------------------------------*/
/* #define LCD_EK08009_XXXX    1     *//*Example*/

/*----------------------- PRIVATE TYPE  --------------------------------------*/
/* typedef  xxxx lcd_xxx_t; *//*Example*/

/*----------EXPORTED PRIVATE VARIABLES are defined in lcd.h  -------------*/
static void lcd_ek08009_power_on(void);
static void lcd_ek08009_power_off(void);

/*----------------------- INTERNAL PRIVATE VARIABLES - -----------------------*/
/* int  lcd_xxx;        *//*Example*/
lcd_parm_t lcd_ek08009_parm = {
	.name = "EKING EK08009",
	.fps = 60,						/* frame per second */
	.bits_per_pixel = 18,
	.capability = LCD_CAP_CLK_HI,
    .timing = {
       .pixel_clock = 40000000,  /* pixel clock */
       .option = 0,               /* option flags */

       .hsync = 1,                /* horizontal sync pulse */
       .hbp = 46,                 /* horizontal back porch */
       .hpixel = 800,             /* horizontal pixel */
       .hfp = 210,                /* horizontal front porch */

       .vsync = 1,                /* vertical sync pulse */
       .vbp = 24,                 /* vertical back porch */
       .vpixel = 600,             /* vertical pixel */
       .vfp = 12,                 /* vertical front porch */
     },
	.initial = lcd_ek08009_power_on,
	.uninitial = lcd_ek08009_power_off,
};

/*--------------------- INTERNAL PRIVATE FUNCTIONS ---------------------------*/
/* void lcd_xxx(void); *//*Example*/

/*----------------------- Function Body --------------------------------------*/
static void lcd_ek08009_power_on(void)
{	
	DPRINT("lcd_ek08009_power_on\n");

	/* TODO */
}

static void lcd_ek08009_power_off(void)
{	
	DPRINT("lcd_ek08009_power_off\n");

	/* TODO */
}

lcd_parm_t *lcd_ek08009_get_parm(int arg) 
{	
	return &lcd_ek08009_parm;
}

int lcd_ek08009_init(void){
	int ret;	

	ret = lcd_panel_register(LCD_EKING_EK08009,(void *) lcd_ek08009_get_parm);	
	return ret;
} /* End of lcd_oem_init */
module_init(lcd_ek08009_init);

/*--------------------End of Function Body -----------------------------------*/
#undef LCD_EKING_EK08009_C
