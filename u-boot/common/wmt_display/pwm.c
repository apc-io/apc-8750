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

/*
 * ChangeLog
 *
 * 2010-08-05  Sam Shen  <samshen@wondermedia.com.tw>
 *     * Add License declaration and ChangeLog
 */


/*----------------------- DEPENDENCE -----------------------------------------*/
#include "vpp.h"
#include "wmt_display.h"
/*----------------------- Backlight --------------------------------------*/
#define pwm_write_reg(addr,val,wait)	\
	REG32_VAL(addr) = val;	\
	while(REG32_VAL(0xd8220040)&=wait);

void lcd_blt_enable(int no,int enable)
{
	pwm_set_enable(no,enable);

} /* End of vt8430_blt_enable */

void lcd_blt_set_level(int no,int level)
{
	return;
}

void lcd_blt_set_freq(int no,unsigned int freq)
{
	return;
}

void lcd_blt_set_pwm(int no, int level, int freq)
{
	int clock = auto_pll_divisor(DEV_PWM,GET_FREQ, 0, 0);
	int period, duty, scalar;

	clock = clock / freq;
	scalar = 0;
	period = 2000;

	while(period > 1023) {
		scalar++;
		period = clock / scalar;
	}

	duty = (period*level)/100;
	duty = (duty)? (duty-1):0;
	scalar = scalar-1;
	period = period -1;

	pwm_set_period(no,period);
	pwm_set_duty(no,duty);
	pwm_set_scalar(no,scalar);
	if (g_pwm_setting.config)
		pwm_set_control(no,(level)? 0x36:0x8);
	else
	pwm_set_control(no,(level)? 0x34:0x8);
	pwm_set_gpio(no,level);
}

void pwm_set_enable(int no,int enable)
{
	unsigned int addr,reg,reg1;

	addr = PWM_CTRL_REG_ADDR + (0x10 * no);	
	reg = REG32_VAL(addr);
	if( enable ){
		reg |= PWM_ENABLE;
	}
	else {
		reg &= ~PWM_ENABLE;
	}

	pwm_write_reg(addr,reg,PWM_CTRL_UPDATE << (4*no));
	reg1 = REG32_VAL(addr);
	pwm_set_gpio(no,enable);
}

void pwm_set_control(int no,unsigned int ctrl)
{
	unsigned int addr;

	addr = PWM_CTRL_REG_ADDR + (0x10 * no);
	pwm_write_reg(addr,ctrl,PWM_CTRL_UPDATE << (8*no));
} /* End of pwm_proc */

void pwm_set_scalar(int no,unsigned int scalar)
{
	unsigned int addr;

	addr = PWM_SCALAR_REG_ADDR + (0x10 * no);
	pwm_write_reg(addr,scalar,PWM_SCALAR_UPDATE << (8*no));
}

void pwm_set_period(int no,unsigned int period)
{
	unsigned int addr;

	addr = PWM_PERIOD_REG_ADDR + (0x10 * no);
	pwm_write_reg(addr,period,PWM_PERIOD_UPDATE << (8*no));
}

void pwm_set_duty(int no,unsigned int duty)
{
	unsigned int addr;

	addr = PWM_DUTY_REG_ADDR + (0x10 * no);
	pwm_write_reg(addr,duty,PWM_DUTY_UPDATE << (8*no));
}

unsigned int pwm_get_period(int no)
{
	unsigned int addr;

	addr = PWM_PERIOD_REG_ADDR + (0x10 * no);
	return (REG32_VAL(addr) & 0xFFF);
}

unsigned int pwm_get_duty(int no)
{
	unsigned int addr;

	addr = PWM_DUTY_REG_ADDR + (0x10 * no);
	return (REG32_VAL(addr) & 0xFFF);
}

void set_lcd_power(int on)
{
	unsigned int val;

	if ((g_lcd_pw_pin.ctl == 0) || (g_lcd_pw_pin.oc == 0) || (g_lcd_pw_pin.od == 0)) {
		printf("lcd power ping not define\n");
		return;
	}
	val = 1<<g_lcd_pw_pin.bitmap;	
	if (g_lcd_pw_pin.act == 0)
		on = ~on;
	REG32_VAL(g_lcd_pw_pin.ctl) |= val;
	REG32_VAL(g_lcd_pw_pin.oc) |= val;
	if (on) {
		REG32_VAL(g_lcd_pw_pin.od) |= val;
	} else {
		REG32_VAL(g_lcd_pw_pin.od) &= ~val;
	}
}
void pwm_set_gpio(int no,int enable)
{
	unsigned int pwm_pin;

	pwm_pin = (no==0)? 0x08:0x04;
	if( enable ) { 
		REG8_VAL(GPIO_OD_GP31_PWM_BYTE_ADDR) &= ~pwm_pin;
		REG8_VAL(GPIO_CTRL_GP31_PWM_BYTE_ADDR) &= ~pwm_pin;
	} else {
		REG8_VAL(GPIO_OD_GP31_PWM_BYTE_ADDR) &= ~pwm_pin;
		REG8_VAL(GPIO_OC_GP31_PWM_BYTE_ADDR) |= pwm_pin;
		REG8_VAL(GPIO_CTRL_GP31_PWM_BYTE_ADDR) |= pwm_pin;
	}
	/* set to PWM mode */
	if( no == 1 )
		REG32_VAL(GPIO_PIN_SHARING_SEL_4BYTE_ADDR) |= 0x10;
	pwm_pin = REG32_VAL(PWM_CTRL_REG_ADDR + (0x10 * no));
	if (((pwm_pin&PWM_ENABLE) != 0)&&(enable == 1))
		set_lcd_power(1);
	if (((pwm_pin&PWM_ENABLE) == 0)&&(enable == 0))
		set_lcd_power(0);
}


