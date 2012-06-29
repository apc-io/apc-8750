/*++ 
 * linux/drivers/input/rmtctl/wmt-rmtctl.h
 * WonderMedia input remote control driver
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


#define REG_BASE_IR             	CIR_BASE_ADDR

#define IRSWRST				REG_BASE_IR+0x00			// [0x00] IR Software Reset register
#define IRCTL				REG_BASE_IR+0x04			// [0x04] IR Control register
#define IRCTL_2								REG_BASE_IR+0x08			// [0x08] IR Control register
#define IRSTS								REG_BASE_IR+0x0c			// [0x0c] IR Status register
#define IRDATA(i)								REG_BASE_IR+0x10+i*0x4		// [0x10-0x20] IR Received Data register
#define PARAMETER(i)							REG_BASE_IR+0x24+i*0x4		// [0x24-0x3c]IR Parameter Register for Remote Controller Vendor "NEC"
#define NEC_REPEAT_TIME_OUT_CTRL			REG_BASE_IR+0x40			// [0X40] 
#define NEC_REPEAT_TIME_OUT_COUNT		REG_BASE_IR+0x44			// [0X44] 
#define NEC_REPEAT_TIME_OUT_STS			REG_BASE_IR+0x48			// [0X48]
#define JVC_CONTI_CTRL						REG_BASE_IR+0x50			// [0X50] 
#define JVC_CONTI_COUNT					REG_BASE_IR+0x54			// [0X54] 
#define JVC_CONTI_STS						REG_BASE_IR+0x58			// [0X58] 
#define INT_MASK_CTRL						REG_BASE_IR+0x60			// [0X60] 
#define INT_MASK_COUNT						REG_BASE_IR+0x64			// [0X64] 
#define INT_MASK_STS						REG_BASE_IR+0x68			// [0X68] 
#define WAKEUP_CMD1(i)						REG_BASE_IR+0x70+i*0x4		// [0X70-0x80]
#define WAKEUP_CMD2(i)						REG_BASE_IR+0x84+i*0x4		// [0X84-0x94] 
#define WAKEUP_CTRL						REG_BASE_IR+0x98			// [0X98] 
#define WAKEUP_STS							REG_BASE_IR+0x9c			// [0X9C] 
#define IRFSM								REG_BASE_IR+0xa0			// [0Xa0] 
#define IRHSPMC								REG_BASE_IR+0xa4			// [0xa4] IR Host-Synchronous-Pulse Measure Counter register
#define IRHSPTC								REG_BASE_IR+0xa8			// [0xa8] IR Host-Synchronous-Pulse Tolerance Counter register


