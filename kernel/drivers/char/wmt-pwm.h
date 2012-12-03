/*++
linux/drivers/char/wmt-pwm.h

Copyright (c) 2008  WonderMedia Technologies, Inc.

This program is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software Foundation,
either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with
this program.  If not, see <http://www.gnu.org/licenses/>.

WonderMedia Technologies, Inc.
10F, 529, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C.
--*/

#ifndef WMT_PWM_H
/* To assert that only one occurrence is included */
#define WMT_PWM_H

/*--- wmt-pwm.h---------------------------------------------------------------
*   Copyright (C) 2009 WonderMedia Tech. Inc.
*
* MODULE       : wmt-pwm.h --
* AUTHOR       : Sam Shen
* DATE         : 2009/8/12
* DESCRIPTION  :
*------------------------------------------------------------------------------*/

/*--- History -------------------------------------------------------------------
*Version 0.01 , Sam Shen, 2009/8/12
*	First version
*
*------------------------------------------------------------------------------*/
/*-------------------- MODULE DEPENDENCY -------------------------------------*/
#include <linux/config.h>
#include <linux/types.h>
#include <linux/module.h>
#include <mach/hardware.h>

// Include your headers here

/*-------------------- EXPORTED PRIVATE CONSTANTS ----------------------------*/
/* #define  PWM_XXXX  1    *//*Example*/
#define PWM_NUM_MAX		4
#define PWM_PERIOD_VAL	599  //modified by howayhuo . org:1000

#define PWM_BASE_ADDR	PWM0_BASE_ADDR

// PWM Control Register
#define PWM_CTRL_REG_ADDR   (PWM_BASE_ADDR+0x00)
#define PWM_ENABLE			0x01
#define PWM_INVERT			0x02
#define PWM_AUTOLOAD		0x04
#define PWM_STOP_IMM		0x08
#define PWM_LOAD_PRESCALE	0x10
#define PWM_LOAD_PERIOD		0x20

// PWM Pre scalar
#define PWM_SCALAR_REG_ADDR (PWM_BASE_ADDR+0x04)
#define PWM_PRE_SCALE_MASK	0x3FF

// PWM Period value
#define PWM_PERIOD_REG_ADDR (PWM_BASE_ADDR+0x08)
#define PWM_PERIOD_MASK		0xFFF

// PWM Duty value
#define PWM_DUTY_REG_ADDR   (PWM_BASE_ADDR+0x0C)
#define PWM_DUTY_MASK		0xFFF

// PWM Timer Status
#define PWM_STS_REG_ADDR	(PWM_BASE_ADDR+0x40)
#define PWM_CTRL_UPDATE		0x01
#define PWM_SCALAR_UPDATE	0x02
#define PWM_PERIOD_UPDATE	0x04
#define PWM_DUTY_UPDATE		0x08


/*-------------------- EXPORTED PRIVATE TYPES---------------------------------*/
/* typedef  void  pwm_xxx_t;  *//*Example*/
typedef struct {
	int no;
	unsigned int value;
} pwm_ctrl_t;


/*-------------------- EXPORTED PRIVATE VARIABLES -----------------------------*/
#ifdef WMT_PWM_C /* allocate memory for variables only in wmt-pwm.c */
#       define EXTERN
#else
#       define EXTERN   extern
#endif /* ifdef WMT_PWM_C */

/* EXTERN int      pwm_xxxx; *//*Example*/

#undef EXTERN

/*--------------------- EXPORTED PRIVATE MACROS -------------------------------*/
#define PWM_IOC_MAGIC	'p'

// #define PWMIOSET_THRESHOLD	_IOW(PWM_IOC_MAGIC, 1, sizeof(int))
#define PWMIOSET_ENABLE			_IOW(PWM_IOC_MAGIC, 0, pwm_ctrl_t)
#define PWMIOSET_FREQ			_IOW(PWM_IOC_MAGIC, 1, pwm_ctrl_t)
#define PWMIOGET_FREQ			_IOWR(PWM_IOC_MAGIC, 1, pwm_ctrl_t)
#define PWMIOSET_LEVEL			_IOW(PWM_IOC_MAGIC, 2, pwm_ctrl_t)
#define PWMIOGET_LEVEL			_IOWR(PWM_IOC_MAGIC, 2, pwm_ctrl_t)

#define PWM_IOC_MAXNR	3

/*--------------------- EXPORTED PRIVATE FUNCTIONS  ---------------------------*/
/* extern void  pwm_xxxx(vdp_Void); *//*Example*/
void pwm_set_duty(int no,unsigned int duty) ;
unsigned int pwm_get_period(int no);
unsigned int pwm_get_duty(int no);
void pwm_set_enable(int no,int enable);
unsigned int pwm_get_enable(int no);
void pwm_set_scalar(int no,unsigned int scalar);
void pwm_set_period(int no,unsigned int period);
void pwm_set_control(int no,unsigned int ctrl);
void pwm_set_gpio(int no,int enable);
#endif /* ifndef WMT_PWM_H */

/*=== END wmt-pwm.h ==========================================================*/
