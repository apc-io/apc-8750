/*++ 
Copyright (c) 2010 WonderMedia Technologies, Inc.

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software 
Foundation, either version 2 of the License, or (at your option) any later 
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details. You
should have received a copy of the GNU General Public License along with this
program. If not, see http://www.gnu.org/licenses/>.

WonderMedia Technologies, Inc.
10F, 529, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C.
--*/

#ifndef __CS7146_TS_H__
#define __CS7146_TS_H__

#define MAX_SPI_CLK    (2*1000*1000)
#define SPI_DEFAULT_CLK       (50*1000)/*50kHz*/ 
#define IDLE_DATA_NUM         4
#define POLLING_BITS_TOUT     0x20000
#define ADC_DATA(low, high)  ((((high) & 0x0F) << 8) + (low))

/*touchscreen AD7843*/
#define cs7146_start_bit BIT7
#define cs7146_a2_bit BIT6
#define cs7146_a1_bit BIT5
#define cs7146_a0_bit BIT4
#define cs7146_mode_bit BIT3
#define cs7146_ser_bit BIT2
#define cs7146_pd1_bit BIT1
#define cs7146_pd0_bit BIT0

#define CMD_X_12BIT (cs7146_start_bit|cs7146_a0_bit|cs7146_ser_bit);
#define CMD_Y_12BIT (cs7146_start_bit|cs7146_a2_bit|cs7146_a0_bit|cs7146_ser_bit);

/* touch panel type config   */
#define PANEL_TYPE_4WIRED     0x10
#define PANEL_TYPE_5WIRED     0x11

/* enable calibration or not */
#define CALIBRATION_ENABLE    0x01
#define CALIBRATION_DISABLE   0x00

/* CS7146 working mode       */
#define CS7146_TS_MODE      BIT1
#define CS7146_TEMP_MODE    BIT2
#define CS7146_BAT_MODE     BIT3

/* VT1603 touch panel state  */
#define TS_PENDOWN_STATE     0x00
#define TS_PENUP_STATE       0x01

/* CS7146 suspend now or not */
#define CS7146_NOT_SUSPEND   0x00
#define CS7146_SUSPEND       0x01

/* cs7146 bus type config    */
#ifdef  CONFIG_VT1603_TS_SPI
#define VT1603_TS_USE_SPI
#endif
#ifdef  CONFIG_VT1603_TS_I2C
#define VT1603_TS_USE_I2C
#endif
/*
 * struct cs7146_ts_drvdata - cs7146 driver data
 * @spi:                      spi device(slave) for cs7146
 * @input:                    touch panel as an input device 
 * @spinlock:                 spinlock
 * @work:                     work struct
 * @workqueue:                workqueue for interrupts processing
 * @gpio_irq:                 gpio interrupts, input is cs7146 gpio1
 * @ts_type:                  touch panel type, get from platform data
 * @sclk_div:                 SCLK dividor, get from platform data
 * @mode:                     cs7146 working mode, bat\temp\touch-panel
 * @ts_state:                 touch panel state, pen down or pen up
 * @is_suspend:               cs7146 is suspend now or not
 */
/* FIXME: is it better to use a union instead of macro defination? */
struct cs7146_ts_drvdata {
	struct spi_device *spi;
	struct input_dev *input;
	spinlock_t spinlock;
	struct delayed_work cs7146_delayed_work;
	struct workqueue_struct *workqueue;
	int gpio_irq;
	u8 ts_type;
	u8 sclk_div;
	u8 mode;
	u8 ts_state;
	u8 is_suspend;
};

/*
 * cs7146_ts_platform_data - cs7146 configuration data
 * @panel_type:              touch panel type: 4-wired or 5-wired
 * @cal_en:                  enable calibration circuit or not
 * @cal_sel:                 calibratin capacitor control bits
 * @shfit:                   conversion data shfit
 * @sclk_div:                initial value of sclk dividor
 * @soc_gpio_irq:            soc gpio interrupts, connect with cs7146 gpio1
 */
struct cs7146_ts_platform_data {
	u8  panel_type;
	u8  cal_en;
	u8  cal_sel:2;
	u8  shift;
	u8  sclk_div;
	u8  soc_gpio_irq;
	u16 reserve;
};

/*
 * struct cs7146_ts_pos - cs7146 position conversion data
 * @xpos:                 x position conversion data
 * @ypos:                 y position conversion data
 */
struct cs7146_ts_pos {
	u16 xpos;
	u16 ypos;
};

/* for cs7146 register ioctl     */
struct cs7146_reg_ioc {
	u8  reg_addr;
	u8  reg_val;
	u16 reserve;
};

struct calibration_parameter {
	int   a1;
	int   b1;
	int   c1;
	int   a2;
	int   b2;
	int   c2;
	int   delta;
};

#define TS_IOC_MAGIC  't'

#define TS_IOCTL_CAL_START    _IO(TS_IOC_MAGIC,   1)
#define TS_IOCTL_CAL_DONE     _IOW(TS_IOC_MAGIC,  2, int*)
#define TS_IOCTL_GET_RAWDATA  _IOR(TS_IOC_MAGIC,  3, int*)

#endif  /* __CS7146_TS_H__ */
