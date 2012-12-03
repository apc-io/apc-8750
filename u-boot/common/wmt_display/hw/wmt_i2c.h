/*++
	linux/include/asm-arm/arch-wmt/wmt_i2c.h

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
/* Be sure that virtual mapping is defined right */


#ifndef _WMT_I2C_H_
#define _WMT_I2C_H_
/*-------------------- MODULE DEPENDENCY -------------------------------------*/

#include "wmt_mmap.h"
/*
 *   Refer I2C 0.1 Module
 *
 */
/*
    i2c api address

      Since i2c bus would probe all device which connect with it to add i2c adapter
   ,but we dont make sure which would connect with it. In order to reduce probe time
   ,we give a fake slave address for probing only.
   Be carefully, the i2c api address must be different from real device address 
*/

#define         WMT_I2C_API_I2C_ADDR            0x59           /*API address*/

/*
 * Address
 */
#define I2C_CR_ADDR                     (0x0000+I2C_BASE_ADDR)
#define I2C_TCR_ADDR                    (0x0002+I2C_BASE_ADDR)
#define I2C_CSR_ADDR                    (0x0004+I2C_BASE_ADDR)
#define I2C_ISR_ADDR                    (0x0006+I2C_BASE_ADDR)
#define I2C_IMR_ADDR                    (0x0008+I2C_BASE_ADDR)
#define I2C_CDR_ADDR                    (0x000A+I2C_BASE_ADDR)
#define I2C_TR_ADDR                     (0x000C+I2C_BASE_ADDR)
#define I2C_DIV_ADDR                    (0x000E+I2C_BASE_ADDR)

#define I2C1_CR_ADDR                     (0x0000+I2C1_BASE_ADDR)
#define I2C1_TCR_ADDR                    (0x0002+I2C1_BASE_ADDR)
#define I2C1_CSR_ADDR                    (0x0004+I2C1_BASE_ADDR)
#define I2C1_ISR_ADDR                    (0x0006+I2C1_BASE_ADDR)
#define I2C1_IMR_ADDR                    (0x0008+I2C1_BASE_ADDR)
#define I2C1_CDR_ADDR                    (0x000A+I2C1_BASE_ADDR)
#define I2C1_TR_ADDR                     (0x000C+I2C1_BASE_ADDR)
#define I2C1_DIV_ADDR                    (0x000E+I2C1_BASE_ADDR)
/* Slave Address*/
#define I2C_SCR_ADDR                    (0x0010+I2C_BASE_ADDR)
#define I2C_SSR_ADDR                    (0x0012+I2C_BASE_ADDR)
#define I2C_SISR_ADDR                   (0x0014+I2C_BASE_ADDR)
#define I2C_SIMR_ADDR                   (0x0016+I2C_BASE_ADDR)
#define I2C_SDR_ADDR                    (0x0018+I2C_BASE_ADDR)
#define I2C_STR_ADDR                    (0x001A+I2C_BASE_ADDR)

/*
 * Registers
 */
#define I2C_CR_REG                      REG16_PTR(0x0000+I2C_BASE_ADDR)
#define I2C_TCR_REG                     REG16_PTR(0x0002+I2C_BASE_ADDR)
#define I2C_CSR_REG                     REG16_PTR(0x0004+I2C_BASE_ADDR)
#define I2C_ISR_REG                     REG16_PTR(0x0006+I2C_BASE_ADDR)
#define I2C_IMR_REG                     REG16_PTR(0x0008+I2C_BASE_ADDR)
#define I2C_CDR_REG                     REG16_PTR(0x000A+I2C_BASE_ADDR)
#define I2C_TR_REG                      REG16_PTR(0x000C+I2C_BASE_ADDR)
#define I2C_DIV_REG                     REG16_PTR(0x000E+I2C_BASE_ADDR)

#define I2C1_CR_REG                      REG16_PTR(0x0000+I2C1_BASE_ADDR)
#define I2C1_TCR_REG                     REG16_PTR(0x0002+I2C1_BASE_ADDR)
#define I2C1_CSR_REG                     REG16_PTR(0x0004+I2C1_BASE_ADDR)
#define I2C1_ISR_REG                     REG16_PTR(0x0006+I2C1_BASE_ADDR)
#define I2C1_IMR_REG                     REG16_PTR(0x0008+I2C1_BASE_ADDR)
#define I2C1_CDR_REG                     REG16_PTR(0x000A+I2C1_BASE_ADDR)
#define I2C1_TR_REG                      REG16_PTR(0x000C+I2C1_BASE_ADDR)
#define I2C1_DIV_REG                     REG16_PTR(0x000E+I2C1_BASE_ADDR)
/* Slave Registers*/
#define I2C_SCR_REG                     REG16_PTR(0x0010+I2C_BASE_ADDR)
#define I2C_SSR_REG                     REG16_PTR(0x0012+I2C_BASE_ADDR)
#define I2C_SISR_REG                    REG16_PTR(0x0014+I2C_BASE_ADDR)
#define I2C_SIMR_REG                    REG16_PTR(0x0016+I2C_BASE_ADDR)
#define I2C_SDR_REG                     REG16_PTR(0x0018+I2C_BASE_ADDR)
#define I2C_STR_REG                     REG16_PTR(0x001A+I2C_BASE_ADDR)

/*
 * Val Registers
 */
#define I2C_CR_VAL                      REG16_VAL(0x0000+I2C_BASE_ADDR)
#define I2C_TCR_VAL                     REG16_VAL(0x0002+I2C_BASE_ADDR)
#define I2C_CSR_VAL                     REG16_VAL(0x0004+I2C_BASE_ADDR)
#define I2C_ISR_VAL                     REG16_VAL(0x0006+I2C_BASE_ADDR)
#define I2C_IMR_VAL                     REG16_VAL(0x0008+I2C_BASE_ADDR)
#define I2C_CDR_VAL                     REG16_VAL(0x000A+I2C_BASE_ADDR)
#define I2C_TR_VAL                      REG16_VAL(0x000C+I2C_BASE_ADDR)
#define I2C_DIV_VAL                     REG16_VAL(0x000E+I2C_BASE_ADDR)

#define I2C1_CR_VAL                      REG16_VAL(0x0000+I2C1_BASE_ADDR)
#define I2C1_TCR_VAL                     REG16_VAL(0x0002+I2C1_BASE_ADDR)
#define I2C1_CSR_VAL                     REG16_VAL(0x0004+I2C1_BASE_ADDR)
#define I2C1_ISR_VAL                     REG16_VAL(0x0006+I2C1_BASE_ADDR)
#define I2C1_IMR_VAL                     REG16_VAL(0x0008+I2C1_BASE_ADDR)
#define I2C1_CDR_VAL                     REG16_VAL(0x000A+I2C1_BASE_ADDR)
#define I2C1_TR_VAL                      REG16_VAL(0x000C+I2C1_BASE_ADDR)
#define I2C1_DIV_VAL                     REG16_VAL(0x000E+I2C1_BASE_ADDR)
/* Slave Val Registers*/
#define I2C_SCR_VAL                     REG16_VAL(0x0010+I2C_BASE_ADDR)
#define I2C_SSR_VAL                     REG16_VAL(0x0012+I2C_BASE_ADDR)
#define I2C_SISR_VAL                    REG16_VAL(0x0014+I2C_BASE_ADDR)
#define I2C_SIMR_VAL                    REG16_VAL(0x0016+I2C_BASE_ADDR)
#define I2C_SDR_VAL                     REG16_VAL(0x0018+I2C_BASE_ADDR)
#define I2C_STR_VAL                     REG16_VAL(0x001A+I2C_BASE_ADDR)

/*
 *  I2C_CR_REG
 *  I2C Controller Control
 */
/* Reserved [15:05] */
/* [04:04] -- PCLK_SLE tied to Zero */
#define I2C_CR_CPU_RDY                  0x0008
#define I2C_CR_TX_END                   0x0004
#define I2C_CR_TX_NEXT_NO_ACK           0x0002
#define I2C_CR_TX_NEXT_ACK              0x0000
#define I2C_CR_ENABLE                   0x0001
#define I2C_SLAV_MODE_SEL		0x8000
/*
 *  I2C_TCR_REG
 *  I2C Transfer Control
 *
 */
#define I2C_TCR_HS_MODE			0x2000		/* [13:13] */
#define I2C_TCR_STANDARD_MODE           0x0000          /* [15:15] */
#define I2C_TCR_FAST_MODE               0x8000
#define I2C_TCR_MASTER_WRITE            0x0000          /* [14:14] */
#define I2C_TCR_MASTER_READ             0x4000
/* Reserved [13:07] */
#define I2C_TCR_SLAVE_ADDR_MASK         0x007F          /* [06:00] */

/*
 *  I2C_CSR_REG
 *  I2C Status
 *
 */
/* Reserved [15:02] */
#define I2C_READY                       0x0002          /* [01:01] R */
#define I2C_BUSY                        0x0000
#define I2C_STATUS_MASK                 0x0002
#define I2C_CSR_RCV_ACK                 0x0000          /* [00:00] R */
#define I2C_CSR_RCV_NOT_ACK             0x0001
#define I2C_CSR_RCV_ACK_MASK            0x0001

/*
 *  I2C_ISR_REG
 *  I2C Interrupt Status
 *
 */
/* Reserved [15:03] */
#define I2C_ISR_SCL_TIME_OUT              0x0004          /* [02:02] R */
#define I2C_ISR_SCL_TIME_OUT_WRITE_CLEAR  0x0004
#define I2C_ISR_BYTE_END                  0x0002          /* [01:01] R */
#define I2C_ISR_BYTE_END_WRITE_CLEAR      0x0002
#define I2C_ISR_NACK_ADDR                 0x0001          /* [00:00] R */
#define I2C_ISR_NACK_ADDR_WRITE_CLEAR     0x0001

#define I2C_ISR_ALL_WRITE_CLEAR           0x0007
/*
 *  I2C_IMR_REG
 *  I2C Interrupt Mask
 *
 */
/* Reserved [15:03] */
#define I2C_IMR_SCL_TIME_OUT_MASK       0x0004          /* [02:02]  */
#define I2C_IMR_BYTE_END_MASK           0x0002          /* [01:01]  */
#define I2C_IMR_NACK_ADDR_MASK          0x0001          /* [00:00]  */

#define I2C_IMR_ALL_ENABLE              0x0007
/*
 *  I2C_CDR_REG
 *  I2C Data IO
 *
 */
#define I2C_CDR_DATA_READ_MASK          0xFF00          /* [15:08]  */
#define I2C_CDR_DATA_WRITE_MASK         0x00FF          /* [07:00]  */


/*
 *  I2C_TR_REG
 *  I2C Timer Parameters
 *
 */
#define I2C_TR_SCL_TIME_OUT_MASK        0xFF00          /* [15:08]  */
#define I2C_TR_FSTP_MASK                0x00FF          /* [07:00]  */

#define I2C_TR_STD_VALUE                0xFF64    /* standard mode*/
#define I2C_TR_FAST_VALUE               0xFF19    /* fast mode*/


/*
 *  I2C_DIV_REG
 *  I2C DIV
 *
 */
#define APB_96M_I2C_DIV                 7  /*Dean revised 2007/9/11 */
#define APB_166M_I2C_DIV                12 /*Dean revised 2008/5/9 */



/*
 *  I2C slave registers setting
 *
 */
#define HS_MASTER_CODE			0x0800

#define I2C_SLAVE_ADDR 0x59
#define I2C_SLAVE_MASK 0x007F
#define I2C_SLAVE_NACK BIT12
#define I2C_SLAVE_HS_MODE BIT14
#define I2C_SLAVE_EN BIT15

#define I2C_SISR_SCL_TIME_OUT              0x0004          /* [02:02] R */
#define I2C_SISR_SCL_TIME_OUT_WRITE_CLEAR  0x0004
#define I2C_SISR_BYTE_END                  0x0002          /* [01:01] R */
#define I2C_SISR_BYTE_END_WRITE_CLEAR      0x0002
#define I2C_SISR_DAT_REQ                      0x0001          /* [00:00] R */
#define I2C_SISR_DAT_REQ_WRITE_CLEAR          0x0001

#define I2C_SISR_ALL_WRITE_CLEAR           0x0007

#define I2C_SIMR_SCL_TIME_OUT_MASK       0x0004          /* [02:02]  */
#define I2C_SIMR_BYTE_END_MASK           0x0002          /* [01:01]  */
#define I2C_SIMR_NACK_ADDR_MASK          0x0001          /* [00:00]  */

#define I2C_SIMR_ALL_ENABLE              0x0007

#define I2C_SRCV_NACK BIT0
#define I2C_SREAD BIT1
#define I2C_SACT BIT2

#define I2C_SLAVE_WRITE_DATA_SHIFT 0
#define I2C_SLAVE_READ_DATA_SHIFT 8
#define I2C_SLAVE_READ_DATA_MASK 0xFF00
#define I2C_SLAVE_WRITE_DATA_MASK 0x00FF

enum i2c_mode_e {
	I2C_STANDARD_MODE = 0 ,
	I2C_FAST_MODE     = 1,
	I2C_HS_MODE	= 2,
};

struct i2c_regs_s {
	volatile unsigned short cr_reg;		/* IIC controller control register*/
	volatile unsigned short tcr_reg;		/* IIC controller transfer control register*/
	volatile unsigned short csr_reg;		/* IIC controller status register*/
	volatile unsigned short isr_reg;		/* IIC controller interrupt status register*/
	volatile unsigned short imr_reg;		/* IIC controller interrupt mask register*/
	volatile unsigned short cdr_reg;		/* IIC controller data I/O buffer register*/
	volatile unsigned short tr_reg;		/* IIC controller time parameter register*/
	volatile unsigned short div_reg;		/* IIC controller clock divider register*/
	volatile unsigned short scr_reg;		/* IIC slave controller control register*/
	volatile unsigned short cssr_reg;		/* IIC slave controller status register*/
	volatile unsigned short sisr_reg;	/* IIC slave controller interrupt status register*/
	volatile unsigned short simr_reg;	/* IIC slave controller interrupt mask register*/
	volatile unsigned short csdr_reg;		/* IIC slave controller data I/O buffer register*/
	volatile unsigned short str_reg;		/* IIC slave controller time parameter register*/
};

#define SUSPEND_NOTIFY 0
#define SUSPEND_SAVE_STATE 1
#define SUSPEND_DISABLE 2
#define SUSPEND_POWER_DOWN 3
#define RESUME_POWER_ON 0
#define RESUME_RESTORE_STATE 1
#define RESUME_ENABLE 2

#define I2C_ALGO_WMT                 0x00900000    /* via WMT on-chip i2c algo*/

#define I2C_ADAPTER_RETRIES             3
#define I2C_ALGO_UDELAY                 10
#define I2C_ALGO_TIMEOUT                500

#define MAX_MESSAGES            65536   /* maximum number of messages to send*/


#define I2C_SET_STANDARD_MODE           0x07A0
#define I2C_SET_FAST_MODE                       0x07A1

#if 0
struct i2c_algo_wmt_data {
	int  (*write_msg)(unsigned int slave_addr, char *buf, unsigned int length , int restart, int last) ;
	int  (*read_msg)(unsigned int slave_addr, char *buf, unsigned int length , int restart, int last) ;
	int  (*send_request)(struct i2c_msg *msg, int msg_num, int non_block);
#ifdef CONFIG_SND_SOC_VT1603
	int (*vt1603_write_for_read)(unsigned int slave_addr, char *buf, unsigned int length , int restart, int last);
#endif
	int  (*wait_bus_not_busy) (void);
	void (*reset) (void);
	void (*set_mode)(enum i2c_mode_e) ;
	int  udelay;
	int  timeout;
};
#endif

#endif
