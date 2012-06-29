/*++ 
Copyright (c) 2010 WonderMedia Technologies, Inc.

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software 
Foundation, either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details. You
should have received a copy of the GNU General Public License along with this
program. If not, see http://www.gnu.org/licenses/>.

WonderMedia Technologies, Inc.
10F, 529, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C.
--*/

#ifndef __I2C_H__
#define __I2C_H__

/*
 *   I2C register struct
 *
 */

#define BA_I2C0 0xD8280000
#define BA_I2C1 0xD8320000

struct I2C_REG {
	volatile unsigned short IICCR;		/* IIC controller control register*/
	volatile unsigned short IICTCR;		/* IIC controller transfer control register*/
	volatile unsigned short IICSR;		/* IIC controller status register*/
	volatile unsigned short IICISR;		/* IIC controller interrupt status register*/
	volatile unsigned short IICIMR;		/* IIC controller interrupt mask register*/
	volatile unsigned short IICDR;		/* IIC controller data I/O buffer register*/
	volatile unsigned short IICTR;		/* IIC controller time parameter register*/
	volatile unsigned short IICDIV;		/* IIC controller clock divider register*/
	volatile unsigned short IICSCR;		/* IIC slave controller control register*/
	volatile unsigned short IICSSR;		/* IIC slave controller status register*/
	volatile unsigned short IICSISR;	/* IIC slave controller interrupt status register*/
	volatile unsigned short IICSIMR;	/* IIC slave controller interrupt mask register*/
	volatile unsigned short IICSDR;		/* IIC slave controller data I/O buffer register*/
	volatile unsigned short IICSTR;		/* IIC slave controller time parameter register*/
};


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
#define I2C_SLAV_MODE_SEL BIT15

/*
 *  I2C_TCR_REG
 *  I2C Transfer Control
 *
 */
#define I2C_TCR_HS_MODE			0x02000		/* [13:13] */
#define I2C_TCR_MODE_SEL		0x0000		/*i determine by BIT15 */
#define I2C_TCR_STANDARD_MODE           0x0000          /* [15:15] */
#define I2C_TCR_FAST_MODE               0x8000
#define I2C_TCR_MODE_MASK               0x8000
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

#define I2C_TR_STD_VALUE                0x8064    /* standard mode*/
#define I2C_TR_FAST_VALUE               0x8019    /* fast mode*/

/*
 *  I2C_DIV_REG  [Refer to "DS_VT8420_100.pdf" page 188]
 *  I2C DIV
 *
 */

#define APB_48M_I2C_DIV                 3  /*  48/(3+1) = 12*/
#define APB_60M_I2C_DIV                 4  /*  60/(4+1) = 12*/
#define APB_72M_I2C_DIV                 5  /*  72/(5+1) = 12*/
#define APB_84M_I2C_DIV                 6  /*  84/(6+1) = 12*/
#define APB_96M_I2C_DIV                 7  /*  96/(7+1) = 12*/
#define APB_108M_I2C_DIV                8  /* 108/(8+1) = 12*/
#define APB_120M_I2C_DIV                9  /* 120/(9+1) = 12*/
#define APB_132M_I2C_DIV                10  /* 132/(10+1) = 12*/
#define APB_144M_I2C_DIV                11  /* 144/(11+1) = 12*/
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



/*---------------------------------------------------------------------------------------*/
/* I2C Data structure*/
/*---------------------------------------------------------------------------------------*/


enum i2c_mode_s {
    I2C_STANDARD_MODE = 0 ,
    I2C_FAST_MODE     = 1 ,
    I2C_HS_MODE		= 2,
};

/*
unsigned char g_data[10];
*/

struct i2c_s {
    struct I2C_REG *regs;
    int		irq_no ;
    enum i2c_mode_s	i2c_mode ;
    int		isr_nack ;
    int		isr_byte_end ;
    int		isr_timeout ;

} ;

/* I2C Message - used for pure i2c transaction*/
/**/
/* flags*/
#define I2C_M_RD                0x01
#define I2C_M_WR                0x00

struct i2c_msg_s {
    unsigned short addr;        /* slave address*/
    unsigned short flags;       /* flags*/
    unsigned short len;         /* msg length*/
    unsigned char *buf;        /* pointer to msg data*/
} ;

/*---------------------------------------------------------------------------------------*/
/* Defined in the i2cif.c*/
/*---------------------------------------------------------------------------------------*/
int i2c_register_write(int index, unsigned char data, unsigned char address);

int i2c_register_read(int index, unsigned char address);

void i2c_init(int speed, int slaveaddr);

int i2c_write_short(int index, unsigned short data, unsigned char address);

void set_div(char value);

void dump_reg(void);

int delay_ms(unsigned int time);

int i2c_transfer(struct i2c_msg_s *msgs, int num);

void i2c1_init(int speed, int slaveaddr);

int i2c1_transfer(struct i2c_msg_s *msgs, int num);
#endif /* __I2C_Hi__*/
