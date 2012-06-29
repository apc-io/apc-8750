/*++ 
 * linux/drivers/video/wmt/govm.c
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

/*
 * ChangeLog
 *
 * 2010-08-05  Sam Shen
 *     * Add License declaration and ChangeLog
 */

#define LCD_AUO_A080SN01_C
// #define DEBUG
/*----------------------- DEPENDENCE -----------------------------------------*/
#include "../lcd.h"

/*----------------------- PRIVATE MACRO --------------------------------------*/
/* #define  LCD_A080SN01_XXXX  xxxx    *//*Example*/

/*----------------------- PRIVATE CONSTANTS ----------------------------------*/
/* #define LCD_A080SN01_XXXX    1     *//*Example*/

/*----------------------- PRIVATE TYPE  --------------------------------------*/
/* typedef  xxxx lcd_xxx_t; *//*Example*/

/*----------EXPORTED PRIVATE VARIABLES are defined in lcd.h  -------------*/
static void lcd_a080sn01_initial(void);

/*----------------------- INTERNAL PRIVATE VARIABLES - -----------------------*/
/* int  lcd_xxx;        *//*Example*/
lcd_parm_t lcd_a080sn01_parm = {
	.name = "AUO A080SN01",
	.fps = 60,						/* frame per second */
	.bits_per_pixel = 24,
	.capability = 0,
	.timing = {
		.pixel_clock = 40000000,	/* pixel clock */
		.option = 0,				/* option flags */

		.hsync = 128,				/* horizontal sync pulse */
		.hbp = 88,					/* horizontal back porch */
		.hpixel = 800,				/* horizontal pixel */
		.hfp = 40,					/* horizontal front porch */

		.vsync = 3,					/* vertical sync pulse */
		.vbp = 24,					/* vertical back porch */
		.vpixel = 600,				/* vertical pixel */
		.vfp = 1,					/* vertical front porch */
	},
	
	.initial = lcd_a080sn01_initial,		
};

/*--------------------- INTERNAL PRIVATE FUNCTIONS ---------------------------*/
/* void lcd_xxx(void); *//*Example*/

/*----------------------- Function Body --------------------------------------*/
static void lcd_a080sn01_initial(void)
{	
	DPRINT("lcd_a080sn01_initial\n");
		
	/* TODO */
}

lcd_parm_t *lcd_a080sn01_get_parm(int arg) 
{	
	return &lcd_a080sn01_parm;
}

int lcd_a080sn01_init(void){	
	int ret;	

	ret = lcd_panel_register(LCD_AUO_A080SN01,(void *) lcd_a080sn01_get_parm);	
	return ret;
} /* End of lcd_a080sn01_init */
module_init(lcd_a080sn01_init);

/*--------------------End of Function Body -----------------------------------*/
#undef LCD_AUO_A080SN01_C

