/* include/linux/kxte9.h - KXTE9 accelerometer driver
 *
 * Copyright (C) 2010 Kionix, Inc.
 * Written by Kuching Tan <kuchingtan@kionix.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
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
 */

#ifndef __KXTE9_H__
#define __KXTE9_H__

#define KXTE9_I2C_ADDR	0x0F
/* CONTROL REGISTER 1 BITS */
#define TPE				0x01	/* tilt position function enable bit */
#define WUFE			0x02	/* wake-up function enable bit */
#define B2SE			0x04	/* back-to-sleep function enable bit */
#define ODR125E			0x20	/* 125Hz ODR mode */
#define ODR40E			0x18	/* initial ODR masks */
#define ODR10E			0x10
#define ODR3E			0x08
#define ODR1E			0x00
/* CONTROL REGISTER 3 BITS */
#define SRST			0x80	/* software reset */
#define CTC				0x10	/* communication-test function */
#define OB2S40			0x0C	/* back-to-sleep ODR masks */
#define OB2S10			0x08
#define OB2S3			0x04
#define OB2S1			0x00
#define OWUF40			0x03	/* wake-up ODR masks */
#define OWUF10			0x02
#define OWUF3			0x01
#define OWUF1			0x00
/* INTERRUPT CONTROL REGISTER 1 BITS */
#define KXTE9_IEN		0x10	/* interrupt enable */
#define KXTE9_IEA		0x08	/* interrupt polarity */
#define KXTE9_IEL		0x04	/* interrupt response */

/* Device Meta Data */
#define DESC_DEV		"KXTE9 3-axis Accelerometer"	// Device Description
#define VERSION_DEV		"1.1.7"
#define VER_MAJOR_DEV	1
#define	VER_MINOR_DEV	1
#define VER_MAINT_DEV	7
#define	MAX_G_DEV		(2.0f)		// Maximum G Level
#define	MAX_SENS_DEV	(1024.0f)	// Maximum Sensitivity
#define PWR_DEV			(0.03f)		// Typical Current

/* Input Device Name */
//#define INPUT_NAME_ACC	"kxte9_accel"
#define INPUT_NAME_ACC	"g-sensor"

/* Device name for kxte9 misc. device */
#define NAME_DEV	"kxte9"
#define DIR_DEV		"/dev/kxte9"

/* IOCTLs for kxte9 misc. device library */
#define KXTE9IO									0x92
#define KXTE9_IOCTL_GET_COUNT			_IOR(KXTE9IO, 0x01, int)
#define KXTE9_IOCTL_GET_MG				_IOR(KXTE9IO, 0x02, int)
#define KXTE9_IOCTL_ENABLE_OUTPUT		 _IO(KXTE9IO, 0x03)
#define KXTE9_IOCTL_DISABLE_OUTPUT		 _IO(KXTE9IO, 0x04)
#define KXTE9_IOCTL_GET_ENABLE			_IOR(KXTE9IO, 0x05, int)
#define KXTE9_IOCTL_RESET				 _IO(KXTE9IO, 0x06)
#define KXTE9_IOCTL_UPDATE_ODR			_IOW(KXTE9IO, 0x07, int)
#define KXTE9_IOCTL_ENABLE_CTC			 _IO(KXTE9IO, 0x08)
#define KXTE9_IOCTL_DISABLE_CTC			 _IO(KXTE9IO, 0x09)
#define KXTE9_IOCTL_GET_CT_RESP			_IOR(KXTE9IO, 0x0A, int)


#ifdef __KERNEL__
struct kxte9_platform_data {
	int poll_interval;
	int min_interval;
	/* the desired g-range, in milli-g (always 2000 for kxte9) */
	u8 g_range;
	/* used to compensate for alternate device placement within the host */
/*
	u8 axis_map_x;
	u8 axis_map_y;
	u8 axis_map_z;
	u8 negate_x;
	u8 negate_y;
	u8 negate_z;
*/
	/* initial configuration values, set during board configuration */
	u8 ctrl_reg1_init;
	u8 engine_odr_init;
	u8 int_ctrl_init;
	u8 tilt_timer_init;
	u8 wuf_timer_init;
	u8 b2s_timer_init;
	u8 wuf_thresh_init;
	u8 b2s_thresh_init;

	int (*init)(void);
	void (*exit)(void);
//NelsonTmp{
//	int gpio;
//}
};
#endif /* __KERNEL__ */

#endif  /* __KXTE9_H__ */

