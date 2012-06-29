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

#ifndef _GPIO_H_
#define _GPIO_H_

typedef struct _GPIO_REG_ {
	volatile unsigned long CTRL_UR_LPC_SF;     /* [Rx00] GPIO Enable Control Register for UARTS,LPC,SF */
	volatile unsigned long CTRL_SD_IIC_SPI_MS; /* [Rx04] GPIO Enable Control Register for SD,IIC,SPI,MS */
	volatile unsigned long CTRL_CF;            /* [Rx08] GPIO Enable Control Register for CF */
	volatile unsigned long CTRL_NF;            /* [Rx0C] GPIO Enable Control Register for NF */
	volatile unsigned long CTRL_PCIAD;         /* [Rx10] GPIO Enable Control for PCI AD BUS */
	volatile unsigned long CTRL_PCI;           /* [Rx14] GPIO Eanble Control for Non PCI AD BUS */
	volatile unsigned long CTRL_IDE;           /* [Rx18] GPIO Enable Control for IDE */
	volatile unsigned long RESV_1C;            /* [Rx1C] */

	volatile unsigned long OC_UR_LPC_SF;     /* [Rx20] GPIO Output Control Register for UARTS,LPC,SF */
	volatile unsigned long OC_SD_IIC_SPI_MS; /* [Rx24] GPIO Output Control Register for SD,IIC,SPI,MS */
	volatile unsigned long OC_CF;            /* [Rx28] GPIO Output Control Register for CF */
	volatile unsigned long OC_NF;            /* [Rx2C] GPIO Output Control Register for NF */
	volatile unsigned long OC_PCIAD;         /* [Rx30] GPIO Output Control Register for PCI AD BUS */
	volatile unsigned long OC_PCI;           /* [Rx34] GPIO Output Control Register for Non PCI AD BUS */
	volatile unsigned long OC_IDE;           /* [Rx38] GPIO Output Control Register for IDE */
	volatile unsigned long OC_GPIO;          /* [Rx3C] GPIO Output Control Register for GPIO */
	volatile unsigned long OD_UR_LPC_SF;     /* [Rx40] GPIO Output Data Register for UARTS,LPC,SF */

	volatile unsigned long OD_SD_IIC_SPI_MS; /* [Rx44] GPIO Output Data Register for SD,IIC,SPI,MS */
	volatile unsigned long OD_CF;            /* [Rx48] GPIO Output Data Register for CF */
	volatile unsigned long OD_NF;            /* [Rx4C] GPIO Output Data Register for NF */
	volatile unsigned long OD_PCIAD;         /* [Rx50] GPIO Output Data Register for PCIAD */
	volatile unsigned long OD_PCI;           /* [Rx54] GPIO Output Data Register for Non PCI AD BUS */
	volatile unsigned long OD_IDE;           /* [Rx58] GPIO Output Data Register for IDE */
	volatile unsigned long OD_GPIO;          /* [Rx5C] GPIO Output Data Register for GPIO */

	volatile unsigned long ID_UR_LPC_SF;     /* [Rx60] GPIO Input Data Register for UARTS,LPC,SF */
	volatile unsigned long ID_SD_IIC_SPI_MS; /* [Rx64] GPIO Input Data Register for SD,IIC,SPI,MS */
	volatile unsigned long ID_CF;            /* [Rx68] GPIO Input Data Register for CF */
	volatile unsigned long ID_NF;            /* [Rx6C] GPIO Input Data Register for NF */
	volatile unsigned long ID_PCIAD;         /* [Rx70] GPIO Input Data Register for PCIAD */
	volatile unsigned long ID_PCI;           /* [Rx74] GPIO Input Data Register for Non PCI AD BUS */
	volatile unsigned long ID_IDE;           /* [Rx78] GPIO Input Data Register for IDE */
	volatile unsigned long ID_GPIO;          /* [Rx7C] GPIO Input Data Register for GPIO */

	volatile unsigned long RESV_80[0x20];    /* [Rx80-RxFF] */

	volatile unsigned long STRAP_STS;        /* [Rx100] Strapping Option Status Register */
	volatile unsigned long RESV_104;         /* [Rx104]  */
	volatile unsigned long AHB_CTRL;         /* [Rx108] AHB Control Register */
	volatile unsigned long RESV_10C[0x3D];   /* [Rx10C-Rx1FF] */

	volatile unsigned long SEL_CF_NF;        /* [Rx200] CF/NF Selection Register */
	volatile unsigned long RESV_204[0x3F];   /* [Rx204-Rx2FF] */

	volatile unsigned long INT_REQ_TYPE;     /* [Rx300] GPIO Interrupt Request Type Register */
	volatile unsigned long INT_REQ_STS;      /* [Rx304] GPIO Interrupt Request Status Register */
	volatile unsigned long RESV_308[0x3E];   /* [Rx308-Rx3FF] */

	volatile unsigned long IDE_IO_DRV;       /* [Rx400] IDE I/O Drive Strength Register */
	volatile unsigned long MS_IO_DRV;        /* [Rx404] MS I/O Drive Strength and Slew Rate Register */
	volatile unsigned long PCI_IO_DRV;       /* [Rx408] PCI Bus I/O Drive Strength and Slew */
	/*                                           Rate Register                              */
	volatile unsigned long SD_IO_DRV;        /* [Rx40C] SD I/O Drive Strength and Slwe Rate Register */
	volatile unsigned long SF_IO_DRV;        /* [Rx410] Serial Flash Clock I/O Drive Strength */
	/*                                           and Slew Rate Register */
	volatile unsigned long SPI_IO_DRV;       /* [Rx414] SPI I/O Drive Strength and Slew Rate Register */
} GPIO_REG, *PGPIO_REG;

/*
 * volatile unsigned long CTRL_UR_LPC_SF; //[Rx00] GPIO Enable Control Register for UARTS,LPC,SF
 */
#define GPIO_UR0_CTS			0x1			/* UART0 CTS signal GPIO Enable */
#define GPIO_UR0_RTS			0x2			/* UART0 RTS signal GPIO Enable */
#define GPIO_UR0_RXD			0x4
#define GPIO_UR0_TXD			0x8
#define GPIO_UR0_ALL			0xF

#define GPIO_UR1_CTS			0x100		/* UART1 CTS signal GPIO Enable */
#define GPIO_UR1_RTS			0x200		/* UART1 RTS signal GPIO Enable */
#define GPIO_UR1_RXD			0x400
#define GPIO_UR1_TXD			0x800
#define GPIO_UR1_ALL			0xF00

#define GPIO_LPC_AD4      0xF0000
#define GPIO_LPC_CLK      0x100000
#define GPIO_LPC_DRQ      0x200000
#define GPIO_LPC_FRAME    0x400000
#define GPIO_LPC_RESET    0x800000
#define GPIO_LPC_SERIRQ   0x1000000

#define GPIO_SF_CS2				0x6000000
#define GPIO_SF_CLK				0x8000000
#define GPIO_SF_DI				0x10000000
#define GPIO_SF_DO				0x20000000

/*
 * volatile unsigned long CTRL_SD_IIC_SPI_MS; //[Rx04] GPIO Enable Control Register for SD,IIC,SPI,MS
 */
#define GPIO_SD_DATA4     0xF

#define GPIO_MMC_DATA4    0xF0

#define GPIO_SD_CLK       0x100
#define GPIO_SD_CMD       0x200

#define GPIO_IIC_SCL      0x400
#define GPIO_IIC_SDA      0x800

#define GPIO_SPI_CLK			0x1000
#define GPIO_SPI_MISO			0x2000
#define GPIO_SPI_MOSI			0x4000
#define GPIO_SPI_SS				0x8000

#define GPIO_MS_ALL				0x7F0000
#define GPIO_MS_DATA4			0xF0000
#define GPIO_MS_BS				0x100000
#define GPIO_MS_CLK				0x200000
#define GPIO_MS_INS				0x400000

/*
 * volatile unsigned long CTRL_CF; //[Rx08] GPIO Enable Control Register for CF
 */
#define GPIO_CF_DATA16    0xFFFF
#define GPIO_CF_AD3       0x70000
#define GPIO_CF_CD2       0x180000
#define GPIO_CF_CE2       0x600000
#define GPIO_CF_IOR       0x800000
#define GPIO_CF_IOW       0x1000000
#define GPIO_CF_WAIT      0x2000000
#define GPIO_CF_INTRQ     0x4000000

/*
 * volatile unsigned long CTRL_NF; //[Rx0C] GPIO Enable Control Register for NF
 */
#define GPIO_NF_CE8       0xFF
#define GPIO_NF_ALE       0x100
#define GPIO_NF_CLE       0x200
#define GPIO_NF_RD        0x400
#define GPIO_NF_RE        0x800
#define GPIO_NF_WE        0x1000
#define GPIO_NF_WP        0x2000
#define GPIO_NF_WPD       0x4000

/*
 * volatile unsigned long CTRL_PCIAD; //[Rx10] GPIO Enable Control for PCI AD BUS
 */
#define GPIO_PCI_ADALL    0xFFFFFFFF
#define GPIO_PCI_AD32     0xFFFFFFFF

/*
 * volatile unsigned long CTRL_PCI; //[Rx14] GPIO Eanble Control for Non PCI AD BUS
 */
#define GPIO_PCI_NOADALL  0x1FFFFFFF
#define GPIO_PCI_GNT4     0xF
#define GPIO_PCI_REQ4     0xF0
#define GPIO_PCI_BE4      0xF00
#define GPIO_PCI_INTA     0x1000
#define GPIO_PCI_INTB     0x2000
#define GPIO_PCI_INTC     0x4000
#define GPIO_PCI_INTD     0x8000
#define GPIO_PCI_DEVSEL   0x10000
#define GPIO_PCI_FRAME    0x20000
#define GPIO_PCI_IRDY     0x40000
#define GPIO_PCI_M66EN    0x80000
#define GPIO_PCI_PAR      0x100000
#define GPIO_PCI_PERR     0x200000
#define GPIO_PCI_SERR     0x400000
#define GPIO_PCI_STOP     0x800000
#define GPIO_PCI_TRDY     0x1000000

/*
 * volatile unsigned long CTRL_IDE; //[Rx18] GPIO Enable Control for IDE
 */
#define GPIO_IDE_DD16     0xFFFF
#define GPIO_IDE_DA3      0x70000
#define GPIO_IDE_CS1      0x80000
#define GPIO_IDE_CS3      0x100000
#define GPIO_IDE_DACK     0x200000
#define GPIO_IDE_DREQ     0x400000
#define GPIO_IDE_INTRQ    0x800000
#define GPIO_IDE_IOR      0x1000000
#define GPIO_IDE_IORDY    0x2000000
#define GPIO_IDE_IOW      0x4000000

/*
 * volatile unsigned long OC_UR_LPC_SF; //[Rx20] GPIO Output Control Register for UARTS,LPC,SF
 */
#define GPIO_OE_UR0_CTS    0x1
#define GPIO_OE_UR0_RTS    0x2
#define GPIO_OE_UR0_RXD    0x4
#define GPIO_OE_UR0_TXD    0x8
#define GPIO_OE_UR1_CTS    0x100
#define GPIO_OE_UR1_RTS    0x200
#define GPIO_OE_UR1_RXD    0x400
#define GPIO_OE_UR1_TXD    0x800
#define GPIO_OE_LPC_AD4    0xF0000
#define GPIO_OE_LPC_CLK    0x100000
#define GPIO_OE_LPC_DRQ    0x200000
#define GPIO_OE_LPC_FRAME  0x400000
#define GPIO_OE_LPC_RESET  0x800000
#define GPIO_OE_LPC_SERIRQ 0x1000000
#define GPIO_OE_SF_CS2     0x6000000
#define GPIO_OE_SF_CLK     0x8000000
#define GPIO_OE_SF_DI      0x10000000
#define GPIO_OE_SF_DO      0x20000000

/*
 * volatile unsigned long OC_SD_IIC_SPI_MS; //[Rx24] GPIO Output Control Register for SD,IIC,SPI,MS
 */
#define GPIO_OE_SD_DATA4  0xF
#define GPIO_OE_MMC_DATA4 0xF0
#define GPIO_OE_SD_CLK    0x100
#define GPIO_OE_SD_CMD    0x200
#define GPIO_OE_IIC_SDA   0x400
#define GPIO_OE_IIC_SCL   0x800
#define GPIO_OE_SPI_CLK   0x1000
#define GPIO_OE_SPI_MISO  0x2000
#define GPIO_OE_SPI_MOSI  0x4000
#define GPIO_OE_SPI_SS    0x8000
#define GPIO_OE_MS_DATA0  0x10000
#define GPIO_OE_MS_DATA1  0x20000
#define GPIO_OE_MS_DATA2  0x40000
#define GPIO_OE_MS_DATA3  0x80000
#define GPIO_OE_MS_BS     0x100000
#define GPIO_OE_MS_CLK    0x200000
#define GPIO_OE_MS_INS    0x400000

/*
 * volatile unsigned long OC_CF; //[Rx28] GPIO Output Control Register for CF
 */
#define GPIO_OE_CF_DATA16 0xFFFF
#define GPIO_OE_CF_ADDR3  0x70000
#define GPIO_OE_CF_CD2    0x180000
#define GPIO_OE_CF_CE2    0x600000
#define GPIO_OE_CF_IORD   0x800000
#define GPIO_OE_CF_IOW    0x1000000
#define GPIO_OE_CF_WAIT   0x2000000
#define GPIO_OE_CF_INTRQ  0x4000000

/*
 * volatile unsigned long OC_NF; //[Rx2C] GPIO Output Control Register for NF
 */
#define GPIO_OE_NF_CE8    0xFF
#define GPIO_OE_NF_ALE    0x100
#define GPIO_OE_NF_CLE    0x200
#define GPIO_OE_NF_RD     0x400
#define GPIO_OE_NF_RE     0x800
#define GPIO_OE_NF_WE     0x1000
#define GPIO_OE_NF_WP     0x2000
#define GPIO_OE_NF_WPD    0x4000

/*
 * volatile unsigned long OC_PCIAD; //[Rx30] GPIO Output Control Register for PCI AD BUS
 */
#define GPIO_OE_PCI_AD32		0xFFFFFFFF

/*
 * volatile unsigned long OC_PCI; //[Rx34] GPIO Output Control Register for Non PCI AD BUS
 */
#define GPIO_OE_PCI_GNT4    0xF
#define GPIO_OE_PCI_REQ4    0xF0
#define GPIO_OE_PCI_BE4     0xF00
#define GPIO_OE_PCI_INTA    0x1000
#define GPIO_OE_PCI_INTB    0x2000
#define GPIO_OE_PCI_INTC    0x4000
#define GPIO_OE_PCI_INTD    0x8000
#define GPIO_OE_PCI_DEVSEL  0x10000
#define GPIO_OE_PCI_FRAME   0x20000
#define GPIO_OE_PCI_IRDY    0x40000
#define GPIO_OE_PCI_M66EN   0x80000
#define GPIO_OE_PCI_PAR     0x100000
#define GPIO_OE_PCI_PERR    0x200000
#define GPIO_OE_PCI_SERR    0x400000
#define GPIO_OE_PCI_STOP    0x800000
#define GPIO_OE_PCI_TRDY    0x1000000

/*
 * volatile unsigned long OC_IDE; //[Rx38] GPIO Output Control Register for IDE
 */
#define GPIO_OE_IDE_DATA16  0xFFFF
#define GPIO_OE_IDE_ADDR3   0x70000
#define GPIO_OE_IDE_CS1     0x80000
#define GPIO_OE_IDE_CS3     0x100000
#define GPIO_OE_IDE_DACK    0x200000
#define GPIO_OE_IDE_DREQ    0x400000
#define GPIO_OE_IDE_INTRQ   0x800000
#define GPIO_OE_IDE_IOR     0x1000000
#define GPIO_OE_IDE_IORDY   0x2000000
#define GPIO_OE_IDE_IOW     0x4000000

/*
 * volatile unsigned long OC_GPIO; //[Rx3C] GPIO Output Control Register for GPIO
 */
#define GPIO_OE_GPIO_ALL		0xFF
#define GPIO_OE_GPIO_0			0x1
#define GPIO_OE_GPIO_1			0x2
#define GPIO_OE_GPIO_2			0x4
#define GPIO_OE_GPIO_3			0x8
#define GPIO_OE_GPIO_4			0x10
#define GPIO_OE_GPIO_5			0x20
#define GPIO_OE_GPIO_6			0x40
#define GPIO_OE_GPIO_7			0x80

#endif	/*_GPIO_H_ */
