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

#ifndef __RHINE_DEBUG__
#define __RHINE_DEBUG__

#define BA_MC3			0xD8000000	// DDR/DDR2 Memory Controller Configuration
#define BA_MSC			0xD800B000	// MemoryStick Controller Base Address
#define	BA_MSCDMA		0xD800B100	// MemoryStick Controller DMA Base Address
#define BA_DMA         	0xD8001000  // system dma Base Address
#define BA_RTC			0xD8100000	// RTC Base Address
#define BA_GPIO			0xD8110000	// GPIO Base Address
#define BA_SCC			0xD8120000	// System Configuration Control Base Address
#define BA_PMC			0xD8130000	// Power Management Control Base Address
#define BA_IC			0xD8140000	// Interrupt Controller Base Address
#define BA_UART0 		0xD8200000	// UART0 Base Address
#define BA_UART1 		0xD8210000	// UART1 Base Address
#define BA_SPI			0xD8240000	// SPI Base Address
#define BA_I2C			0xD8280000	// I2C Base Address
#define BA_PATAHC       0xD8008000  // PATA HC Base Address
#ifdef CONFIG_VT3358
#define BA_PATAREG     	0xD8008100  // PATA Data /Conmmand Base Address
#define BA_PATASG       0xD8008180  // PATA SG Base Address
#define BA_PATAHC_PCI   0xD8008000  // PATA PCI Base Address 
#define BA_PATACSREG	0xD8008146  // PATA Control/Status Base Address
#else
#define BA_PATAREG     	0xD8008170  // PATA Data /Conmmand Base Address
#define BA_PATASG       0xD8008500  // PATA SG Base Address
#define BA_PATAHC_PCI   0xD8008600  // PATA PCI Base Address 
#define BA_PATACSREG	0xD8008376  // PATA Control/Status Base Address
#endif
#define BA_SATAHC       0xD800D000  // SATA HC Base Address
#define BA_SATAHC_PCI   0xD800D100  // SATA HC PCI Base Address
#define BA_SATAREG     	0xD800D2F0  // SATA Primary Register
#define BA_SATASG       0xD800D300  // SATA SG Base Address
#define BA_SATACSREG    0xD800D4F6  // SATA Contril/Status Base Address
#define BA_R5A		0xD800E000  // R5A Base Address
//USB Host Controller (UHCI & EHCI) 
#define BA_USB          0xD8007000  // USB CPAI Register Base Address
#define BA_EHCI_PCI     0xD8007100  // USB 2.0 EHCI USB Host Configuration Base Address
#define BA_EHCI_REG     0xD8007200  // USB 2.0 EHCI USB Host Register Base Address
#define BA_UHCI_PCI     0xD8007300  // USB 1.1 UHCI USB Host Configuration Base Address
#define BA_UHCI_REG     0xD8007400  // USB 1.1 UHCI USB Host Register Base Address
#define BA_CFC		0xD800C000  // CFC Base Address
#define BA_DMACFC	0xD800C100  // CFC Dma Base Address 
#define BA_I2S		0xD8220000  // I2S Base Address
#define BA_PCM		0xD82D0000  // PCM Base Address

#define BA_PCISATAREG   0xC0000AF0
#define BA_PCISATAREG2  0xC0000A70
#define BA_PCIPATAREG   0xC00001F0
#define BA_PCISATACSREG 0xC0000AFA
#define BA_PCISATACSREG2 0xC0000A7A
#define BA_PCIPATACSREG 0xC00001FA
#define BA_PCISATASG    0xC000CC00
#define BA_PCIPATASG    0xC000CC10

typedef struct _GPIO_REG_ {
	volatile unsigned long CTRL_UR_LPC_SF;		// [Rx00] GPIO Enable Control Register for UARTS,LPC,SF
	volatile unsigned long CTRL_SD_IIC_SPI_MS;	// [Rx04] GPIO Enable Control Register for SD,IIC,SPI,MS
	volatile unsigned long CTRL_CF;				// [Rx08] GPIO Enable Control Register for CF
	volatile unsigned long CTRL_NF;				// [Rx0C] GPIO Enable Control Register for NF
	volatile unsigned long CTRL_PCIAD;			// [Rx10] GPIO Enable Control for PCI AD BUS
	volatile unsigned long CTRL_PCI;				// [Rx14] GPIO Eanble Control for Non PCI AD BUS
	volatile unsigned long CTRL_IDE;				// [Rx18] GPIO Enable Control for IDE
	volatile unsigned long CTRL_I2S_PCM_SATA2NDLED;				// [Rx1C]

	volatile unsigned long OC_UR_LPC_SF;		// [Rx20] GPIO Output Control Register for UARTS,LPC,SF
	volatile unsigned long OC_SD_IIC_SPI_MS;	// [Rx24] GPIO Output Control Register for SD,IIC,SPI,MS
	volatile unsigned long OC_CF;				// [Rx28] GPIO Output Control Register for CF
	volatile unsigned long OC_NF;				// [Rx2C] GPIO Output Control Register for NF
	volatile unsigned long OC_PCIAD;			// [Rx30] GPIO Output Control Register for PCI AD BUS
	volatile unsigned long OC_PCI;				// [Rx34] GPIO Output Control Register for Non PCI AD BUS
	volatile unsigned long OC_IDE;				// [Rx38] GPIO Output Control Register for IDE
	volatile unsigned long OC_GPIO;				// [Rx3C] GPIO Output Control Register for GPIO
	volatile unsigned long OD_UR_LPC_SF;		// [Rx40] GPIO Output Data Register for UARTS,LPC,SF

	volatile unsigned long OD_SD_IIC_SPI_MS;	// [Rx44] GPIO Output Data Register for SD,IIC,SPI,MS
	volatile unsigned long OD_CF;				// [Rx48] GPIO Output Data Register for CF
	volatile unsigned long OD_NF;				// [Rx4C] GPIO Output Data Register for NF
	volatile unsigned long OD_PCIAD;			// [Rx50] GPIO Output Data Register for PCIAD
	volatile unsigned long OD_PCI;				// [Rx54] GPIO Output Data Register for Non PCI AD BUS
	volatile unsigned long OD_IDE;				// [Rx58] GPIO Output Data Register for IDE
	volatile unsigned long OD_GPIO;				// [Rx5C] GPIO Output Data Register for GPIO

	volatile unsigned long ID_UR_LPC_SF;		// [Rx60] GPIO Input Data Register for UARTS,LPC,SF
	volatile unsigned long ID_SD_IIC_SPI_MS;	// [Rx64] GPIO Input Data Register for SD,IIC,SPI,MS
	volatile unsigned long ID_CF;				// [Rx68] GPIO Input Data Register for CF
	volatile unsigned long ID_NF;				// [Rx6C] GPIO Input Data Register for NF
	volatile unsigned long ID_PCIAD;			// [Rx70] GPIO Input Data Register for PCIAD
	volatile unsigned long ID_PCI;				// [Rx74] GPIO Input Data Register for Non PCI AD BUS
	volatile unsigned long ID_IDE;				// [Rx78] GPIO Input Data Register for IDE
	volatile unsigned long ID_GPIO;				// [Rx7C] GPIO Input Data Register for GPIO

	volatile unsigned long RESV_80[0x20];		// [Rx80-RxFF]

	volatile unsigned long STRAP_STS;			// [Rx100] Strapping Option Status Register
	volatile unsigned long RESV_104;			// [Rx104] 
	volatile unsigned long AHB_CTRL;			// [Rx108] AHB Control Register
	volatile unsigned long RESV_10C[0x3D];		// [Rx10C-Rx1FF]
	
	volatile unsigned long SEL_CF_NF;			// [Rx200] CF/NF Selection Register
	volatile unsigned long SEL_SATA_SPINUP;		// [Rx204] Serial ATA Spinup Control Register
	volatile unsigned long SEL_PCM_SATALED;		// [Rx208] PCM Data Out/SATA 2nd LED Selection Register
	volatile unsigned long SEL_I2S_SATALED;		// [Rx20C] I2S Data Out/SATA 2nd LED Selection Register
	volatile unsigned long PCI_DELAY_TEST;		// [Rx210]
	volatile unsigned long PCI_RESET;			// [Rx214]
	volatile unsigned long RESV_218[2];			// [Rx218 - Rx21F]
	volatile unsigned long PCI_1X_DELAY;		// [Rx220]
	volatile unsigned long PCI_2X_DELAY;		// [Rx224]
	volatile unsigned long PCI_2X_RELAY_INVERT;	// [Rx228]
	volatile unsigned long RESV_22C[0x35];		// [Rx22C - Rx2FF]

	volatile unsigned long INT_REQ_TYPE;		// [Rx300] GPIO Interrupt Request Type Register
	volatile unsigned long INT_REQ_STS;			// [Rx304] GPIO Interrupt Request Status Register
	volatile unsigned long RESV_308[0x3E];		// [Rx308-Rx3FF]

	volatile unsigned long IDE_IO_DRV;			// [Rx400] IDE I/O Drive Strength Register
	volatile unsigned long MS_IO_DRV;			// [Rx404] MS I/O Drive Strength and Slew Rate Register
	volatile unsigned long PCI_IO_DRV;			// [Rx408] PCI Bus I/O Drive Strength and Slew Rate Register
	volatile unsigned long SD_IO_DRV;			// [Rx40C] SD I/O Drive Strength and Slwe Rate Register
	volatile unsigned long SF_IO_DRV;			// [Rx410] Serial Flash Clock I/O Drive Strength and Slew Rate Register
	volatile unsigned long SPI_IO_DRV;			// [Rx414] SPI I/O Drive Strength and Slew Rate Register
}GPIO_REG,*PGPIO_REG;

#endif
