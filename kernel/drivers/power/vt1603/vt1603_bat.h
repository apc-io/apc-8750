/*++
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

#ifndef __VT1603_BAT_H__
#define __VT1603_BAT_H__

#define POLL_TOUT                  100
#define DFLT_POLLING_BAT_INTERVAL  2000  // 2000 miliseconds
#define DFLT_BAT_VAL_AVG           1
#define DEV_NAME                   "vt1603_bat"
#define ADC_DATA(low, high)  ((((high) & 0x0F) << 8) + (low))

/* VT1603 touch panel state  */
#define TS_PENDOWN_STATE      0x00
#define TS_PENUP_STATE        0x01

/* vt1603 bus type config    */
#ifdef  CONFIG_VT1603_BAT_SPI
#define VT1603_BAT_USE_SPI
#define VT1603_MAX_SPI_CLK    (20*1000*1000)
#define SPI_DEFAULT_CLK       (12*1000*1000)
#define IDLE_DATA_NUM         5
#define VT1603_SPI_FIX_CS     0x00
#define VT1603_SPI_FAKE_CS    (0x7F - 1)
#define VT1603_SPI_BUS_0      0x00
#define VT1603_SPI_BUS_1      0x01
#define VT1603_REG_OP_R       0x00
#define VT1603_REG_OP_W       0x01
#endif

#ifdef  CONFIG_VT1603_BAT_I2C
#define VT1603_BAT_USE_I2C
#define VT1603_I2C_FIX_ADDR   0x1A
#define VT1603_I2C_FAKE_ADDR  0xEE
#define VT1603_I2C_WCMD       0x00
#define VT1603_I2C_RCMD       0x01
#define VT1603_I2C_RWCMD      0x02
#define VT1603_I2C_BUS_0      0x00
#define VT1603_I2C_BUS_1      0x01
#endif

#define BA_WAKEUP_SRC_0       BIT0
#define BA_WAKEUP_SRC_1       BIT1
#define BA_WAKEUP_SRC_2       BIT2
#define BA_WAKEUP_SRC_3       BIT3

/*
 * struct vt1603_bat_drvdata - vt1603 battery driver data
 * @spi:                      spi device(slave) for vt1603
 * @i2c:                      i2c client for vt1603 battery
 * @work:                     work struct
 */
struct vt1603_bat_drvdata {
#ifdef VT1603_BAT_USE_SPI
    struct spi_device *spi;
#endif
#ifdef VT1603_BAT_USE_I2C
    struct i2c_client *i2c;
#endif
    u16 bat_val;
    u32 time_stamp;
    u32 detect_time;
    u32 interval;
    struct work_struct work;
    struct timer_list bat_tmr;
};

struct vt1603_bat_platform_data {
#ifdef VT1603_BAT_USE_I2C
    int i2c_bus_id;
#endif
    u32 interval;
    u8  alarm_threshold;
    int wakeup_src;
};

/* VT1603 Register address */
#define VT1603_BTHD_REG       0x78
#define VT1603_BCLK_REG       0x88
#define VT1603_BAEN_REG       0x04

#define VT1603_PWC_REG        0xC0
#define VT1603_CR_REG         0xC1
#define VT1603_CCCR_REG       0xC2
#define VT1603_CDPR_REG       0xC3
#define VT1603_TSPC_REG       0xC4
#define VT1603_AMCR_REG       0xC7
#define VT1603_INTCR_REG      0xC8
#define VT1603_INTEN_REG      0xC9
#define VT1603_INTS_REG       0xCA
#define VT1603_DCR_REG        0xCB

#define VT1603_TODCL_REG      0xCC
#define VT1603_TODCH_REG      0xCD

#define VT1603_DATL_REG       0xCE
#define VT1603_DATH_REG       0xCF

#define VT1603_XPL_REG        0xD0
#define VT1603_XPH_REG        0xD1
#define VT1603_YPL_REG        0xD2
#define VT1603_YPH_REG        0xD3

#define VT1603_BATL_REG       0xD4
#define VT1603_BATH_REG       0xD5

#define VT1603_TEMPL_REG      0xD6
#define VT1603_TEMPH_REG      0xD7

#define VT1603_ERR8_REG       0xD8
#define VT1603_ERR7_REG       0xD9
#define VT1603_ERR6_REG       0xDA
#define VT1603_ERR5_REG       0xDB
#define VT1603_ERR4_REG       0xDC
#define VT1603_ERR3_REG       0xDD
#define VT1603_ERR2_REG       0xDE
#define VT1603_ERR1_REG       0xDF

#define VT1603_DBG8_REG       0xE0
#define VT1603_DBG7_REG       0xE1
#define VT1603_DBG6_REG       0xE2
#define VT1603_DBG5_REG       0xE3
#define VT1603_DBG4_REG       0xE4
#define VT1603_DBG3_REG       0xE5
#define VT1603_DBG2_REG       0xE6
#define VT1603_DBG1_REG       0xE7

/* for VT1603 GPIO1 interrupt setting */
#define VT1603_IMASK_REG27    27
#define VT1603_IMASK_REG28    28
#define VT1603_IMASK_REG29    29
#define VT1603_IPOL_REG33     33
#define VT1603_ISEL_REG36     36

extern int wmt_getsyspara(char *varname, unsigned char *varval, int *varlen);

#endif  /* __VT1603_TS_H__ */
