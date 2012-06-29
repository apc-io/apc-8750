/*++ 
 * linux/sound/soc/codecs/vt1603.h
 * WonderMedia audio driver for ALSA
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

#ifndef _VT1603_H
#define _VT1603_H

/* VT1603 register space */

///////////vt1603//////////////////
#define BIT0	0x00000001
#define BIT1	0x00000002
#define BIT2	0x00000004
#define BIT3	0x00000008
#define BIT4	0x00000010
#define BIT5	0x00000020
#define BIT6	0x00000040
#define BIT7	0x00000080
#define BIT8	0x00000100

#define VT1603_RESET 0x15

#define VT1603_R00  0x00
#define VT1603_R01  0x01
#define VT1603_R02  0x02
#define VT1603_R03  0x03
#define VT1603_R04  0x04
#define VT1603_R05  0x05
#define VT1603_R06  0x06
#define VT1603_R07  0x07
#define VT1603_R08  0x08
#define VT1603_R09  0x09
#define VT1603_R0a  0x0a
#define VT1603_R0b  0x0b
#define VT1603_R0c  0x0c
#define VT1603_R0d  0x0d
#define VT1603_R0e  0x0e
#define VT1603_R0f  0x0f
#define VT1603_R10  0x10
#define VT1603_R11  0x11
#define VT1603_R12  0x12
#define VT1603_R13  0x13
#define VT1603_R15  0x15
#define VT1603_R19  0x19
#define VT1603_R1b  0x1b
#define VT1603_R1c  0x1c
#define VT1603_R1d  0x1d
#define VT1603_R20  0x20
#define VT1603_R21  0x21
#define VT1603_R23  0x23
#define VT1603_R24  0x24
#define VT1603_R25  0x25
#define VT1603_R28  0x28
#define VT1603_R29  0x29
#define VT1603_R2a  0x2a
#define VT1603_R2b  0x2b
#define VT1603_R2c  0x2c
#define VT1603_R2d  0x2d
#define VT1603_R40  0x40
#define VT1603_R41  0x41
#define VT1603_R42  0x42
#define VT1603_R47  0x47
#define VT1603_R51  0x51
#define VT1603_R52  0x52
#define VT1603_R53  0x53
#define VT1603_R5f  0x5f
#define VT1603_R60  0x60
#define VT1603_R61  0x61
#define VT1603_R62  0x62
#define VT1603_R63  0x63
#define VT1603_R64  0x64
#define VT1603_R65  0x65
#define VT1603_R66  0x66
#define VT1603_R67  0x67
#define VT1603_R68  0x68
#define VT1603_R69  0x69
#define VT1603_R6a  0x6a
#define VT1603_R6b  0x6b
#define VT1603_R6d  0x6d
#define VT1603_R6e  0x6e
#define VT1603_R70  0x70
#define VT1603_R71  0x71
#define VT1603_R72  0x72
#define VT1603_R73  0x73
#define VT1603_R77  0x77
#define VT1603_R79  0x79
#define VT1603_R7a  0x7a
#define VT1603_R7b  0x7b
#define VT1603_R7c  0x7c
#define VT1603_R82  0x82
#define VT1603_R87  0x87
#define VT1603_R88  0x88
#define VT1603_R8a  0x8a
#define VT1603_R8e  0x8e
#define VT1603_R90  0x90
#define VT1603_R91  0x91
#define VT1603_R92  0x92
#define VT1603_R93  0x93
#define VT1603_R95  0x95
#define VT1603_R96  0x96
#define VT1603_R97  0x97

#define VT1603_IRQ IRQ_GPIO

struct vt1603_setup_data {
	int i2c_bus;
	unsigned short i2c_address;
};

extern struct snd_soc_dai vt1603_dai;
extern struct snd_soc_codec_device soc_codec_dev_vt1603;

#endif
