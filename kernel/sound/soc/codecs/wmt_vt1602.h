/*++ 
 * linux/sound/soc/codecs/wmt_vt1602.h
 * WonderMedia I2S audio driver for ALSA
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


#ifndef _VT1602_H
#define _VT1602_H

/* VT1602 register space */

#define VT1602_LINVOL    0x00
#define VT1602_RINVOL    0x01
#define VT1602_LOUT1V    0x02
#define VT1602_ROUT1V    0x03
#define VT1602_ADCDAC    0x05
#define VT1602_IFACE     0x07
#define VT1602_SRATE     0x08
#define VT1602_LDAC      0x0a
#define VT1602_RDAC      0x0b
#define VT1602_BASS      0x0c
#define VT1602_TREBLE    0x0d
#define VT1602_RESET     0x0f
#define VT1602_3D        0x10
#define VT1602_ALC1      0x11
#define VT1602_ALC2      0x12
#define VT1602_ALC3      0x13
#define VT1602_NGATE     0x14
#define VT1602_LADC      0x15
#define VT1602_RADC      0x16
#define VT1602_ADCTL1    0x17
#define VT1602_ADCTL2    0x18
#define VT1602_PWR1      0x19
#define VT1602_PWR2      0x1a
#define VT1602_ADCTL3    0x1b
#define VT1602_ADCIN     0x1f
#define VT1602_LADCIN    0x20
#define VT1602_RADCIN    0x21
#define VT1602_LOUTM1    0x22
#define VT1602_LOUTM2    0x23
#define VT1602_ROUTM1    0x24
#define VT1602_ROUTM2    0x25
#define VT1602_MOUTM1    0x26
#define VT1602_MOUTM2    0x27
#define VT1602_LOUT2V    0x28
#define VT1602_ROUT2V    0x29
#define VT1602_MOUTV     0x2a

#define VT1602_CACHE_REGNUM 0x2a

#define VT1602_SYSCLK	0

struct vt1602_setup_data {
	int i2c_bus;
	unsigned short i2c_address;
};

extern struct snd_soc_dai vt1602_dai;
extern struct snd_soc_codec_device soc_codec_dev_vt1602;

#endif
