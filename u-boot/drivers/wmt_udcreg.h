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

#ifndef _USBREG_H_
#define _USBREG_H_

//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>

#define UINT              volatile unsigned int
#define USHORT            volatile unsigned short
#define UCHAR             volatile unsigned char 
#define ULL		          volatile unsigned long long

//#define DWORD             volatile unsigned long
//#define WORD              volatile unsigned short
//#define BYTE              volatile unsigned char

// Basic Define
#define REG32               *(volatile unsigned int *)
#define REG16               *(volatile unsigned short *)
#define REG8                *(volatile unsigned char *)

#define REG_GET32(addr)		( REG32(addr) )		/* Read 32 bits Register */
#define REG_GET16(addr)		( REG16(addr) )		/* Read 16 bits Register */
#define REG_GET8(addr)		( REG8(addr) )		/* Read  8 bits Register */

#define REG_SET32(addr,val)	( REG32(addr) = (val) )	/* Write 32 bits Register */
#define REG_SET16(addr,val)	( REG16(addr) = (val) )	/* Write 16 bits Register */
#define REG_SET8(addr,val)	( REG8(addr)  = (val) )	/* Write  8 bits Register */

#define MEM_GET32(addr)		( REG32(addr) )		/* Read  32 bits Memory */
#define MEM_SET32(addr,val)	( REG32(addr) = (val) )	/* Write 32 bits Memory */

/*HW reg-----------------------------------------------------------------*/
#ifndef OTGIP
#define UDC_IRQ_DMA 				2
#endif
/*#define DWORD             volatile unsigned long*/
/*#define WORD              volatile unsigned short*/
/*#define BYTE              volatile unsigned char*/
#define USB_IP_BASE						0xD8007800
/*UDC Register Space         0x400~0x7EF*/
#define USB_UDC_REG_BASE			(USB_IP_BASE + 0x400)
/*Bulk Endpoint DMA Register Space 0x100*/
#define USB_UDC_DMA_REG_BASE	(USB_IP_BASE + 0x500)
#ifdef OTGIP
/*UHDC Global Register Space 0x7F0~0x7F7*/
#define USB_GLOBAL_REG_BASE 	(USB_IP_BASE + 0x7f0)
#endif

#define UDC_REG(offset)                REG32_PTR(USB_UDC_REG_BASE + offset)

#define OTG_HOST_MEM_BASE               0x01400000/*20MB*/

#define USB_EP0_CTRL_DMA_BASE           OTG_HOST_MEM_BASE         /*64 bytes*/
#define USB_EP1_BULK_IN_DMA_BASE        0x01400040/*OTG_HOST_MEM_BASE + 64 */   /*64B +64KB*/
#define USB_EP2_BULK_OUT_DMA_BASE       0x01410040/*OTG_HOST_MEM_BASE + 65536*/ /* ~ END*/
#define USB_DOWNLOAD_KERNEL_RAM_ADDR    USB_EP2_BULK_OUT_DMA_BASE

/* Basic Define*/
#define REG32               *(volatile unsigned int *)
#define REG16               *(volatile unsigned short *)
#define REG8                *(volatile unsigned char *)

#define REG_GET32(addr)		( REG32(addr) )		/* Read 32 bits Register */
#define REG_GET16(addr)		( REG16(addr) )		/* Read 16 bits Register */
#define REG_GET8(addr)		( REG8(addr) )		/* Read  8 bits Register */

#define REG_SET32(addr, val)	( REG32(addr) = (val) )	/* Write 32 bits Register */
#define REG_SET16(addr, val)	( REG16(addr) = (val) )	/* Write 16 bits Register */
#define REG_SET8(addr, val)	( REG8(addr)  = (val) )	/* Write  8 bits Register */

#define MEM_GET32(addr)		( REG32(addr) )		/* Read  32 bits Memory */
#define MEM_SET32(addr, val)	( REG32(addr) = (val) )	/* Write 32 bits Memory */

/*#define USB_TIMEOUT         0x100*/ /*256*/
/*#define USB_TEST_SUCCESS    1*/
/*#define USB_TEST_FAIL       0*/

/* define the state of the USB device*/
#define USBSTATE_ATTACHED       0x00            /* device attached to host*/
#define USBSTATE_POWERED        0x01            /* device obtain power*/
#define USBSTATE_DEFAULT        0x02            /* default working state*/
#define USBSTATE_ADDRESS        0x04            /* device has get the address*/
#define USBSTATE_CONFIGED       0x10            /* deivce has set the configuration*/
#define USBSTATE_SUSPEND        0x20            /* No bus activity device goes to suspend */

#define UDC_TEST_J_STATE           1
#define UDC_TEST_K_STATE           2
#define UDC_TEST_SE0_NAK           3
#define UDC_TEST_PACKET            4
#define UDC_TEST_FRORCE_ENABLE     5
#define UDC_TEST_EYE_PATTERN       6
#define UDC_TEST_MODE_NOT_ENABLED  0

/*Define the USB register bit value*/
/* USB device commmand/status register  - 20*/
#define USBREG_RUNCONTROLLER     0x01           /* set the controller run*/
#define USBREG_RESETCONTROLLER   0x02           /* reset the device contrller*/
#define USBREG_COMPLETEINT       0x04           /* the completion interrupt*/
#define USBREG_BABBLEINT         0x08           /* the babble interrupt*/
#define USBREG_BUSACTINT         0x10           /* the bus activity interrupt*/
#define USBREG_TEST_J            0x20           /* set test J state*/
#define USBREG_TEST_K            0x40           /* set test k state*/
#define USBREG_SE0_NAK           0x60           /* set test SE0 and NAK*/
#define USBREG_TESTPACKET        0x80           /* set device sending test packet*/
#define USBREG_FORCEENABLE       0xA0           /* force port enable*/
#define USBREG_TESTEYE           0xC0           /* Send test eye pattern packet*/

/* The device address register          - 21*/
#define DEVADDR_ADDRCHANGE       0x80           /* the address change bit for device address register*/

/* Device port control register         - 22*/
#define PORTCTRL_CONNECTSTATUS   0x01           /* current connection status*/
#define PORTCTRL_CONNECTCHANGE   0x02           /* connection change bit*/
#define PORTCTRL_SELFPOWER       0x04           /* the device is in the self power configuration*/
#define PORTCTRL_FORCERESUME     0x08           /* force deivce to wake the host*/
#define PORTCTRL_HIGHSPEEDMODE   0x10           /* device is in the high speed mode*/
#define PORTCTRL_FULLSPEEDMODE   0x20           /* device is in the full speed mode*/

/* Device itnerrupt enable register     - 23*/
#define INTENABLE_DEVICERESET    0x01           /* this bit is set when the bus reset is completed*/
#define INTENABLE_SUSPENDDETECT  0x02           /* device controller detect the suspend signal*/
#define INTENABLE_RESUMEDETECT   0x04           /* device controller detect the resume signal*/
#define INTENABLE_COMPLETEINTEN  0x10           /* enable completion interrupt*/
#define INTENABLE_BABBLEINTEN    0x20           /* enable babble interrupt*/
#define INTENABLE_BUSACTINTEN    0x40           /* enable bus activity interrupt*/
#define INTENABLE_ALL            0x70           /* enable all interrupt */
#define INTENABLE_FULLSPEEDONLY  0x80           /* force device to support full speed only*/

/* Control endpoint control             - 24*/
/* Bulk endpoint control                - 28*/
/* Interrupt endpoint control           - 2C*/
/* Bulk endpoint control                - 0x310*/
#define EP_RUN                   0x01           /* run the enpoint DMA*/
#define EP_ENABLEDMA             0x02           /* enable the DMA engine to run the schedule*/
#define EP_DMALIGHTRESET         0x04           /* light reset the DMA engine*/
#define EP_STALL                 0x08           /* set the endpoint to stall*/
#define EP_ACTIVE                0x10           /* the endpoint DMA is active*/
#define EP_COMPLETEINT           0x20           /* endpoint transfer complete interrupt*/
#define EP_BABBLE                0x40           /* endpoint babble interrupt*/

/* Control transfer descriptor status byte  - 30*/
#define CTRLXFER_XACTERR         0x01           /* trnasacion error of control transfer*/
#define CTRLXFER_BABBLE          0x02           /* babbe error of control transfer*/
#define CTRLXFER_SHORTPKT        0x04           /* shortpacket detect of control transfer*/
#define CTRLXFER_ACTIVE          0x08           /* transfer active of control transfer*/

#define CTRLXFER_IN              0x08           /* I/O control bit 0 for OUT , 1 for In*/
#define CTRLXFER_OUT             0x00           /**/
#define CTRLXFER_CMDVALID        0x10           /* setup command is valid*/
#define CTRLXFER_IOC             0x80           /* determine to gen interrupt for completion or not*/
#define CTRLXFER_DATA1           0x80           /* for data_toggle 1*/
#define CTRLXFER_DATA0           0x00           /* for data_toggle 0*/

/* Bulk endpoint transfer descriptor status            - 34*/
/* Bulk endpoint transfer descriptor status            - 0x314 */
#define BULKXFER_XACTERR         0x01           /* trnasacion error of bulk transfer*/
#define BULKXFER_BABBLE          0x02           /* babbe error of bulk transfer*/
#define BULKXFER_SHORTPKT        0X04           /* shortpacket detect of bulk transfer*/
#define BULKXFER_ACTIVE          0x08           /*  transfer active of bulk transfer*/
#define BULKXFER_IN              0x10           /* I/O control bit 0 for OUT , 1 for In*/
#define BULKXFER_OUT             0x00
/*HP05_Begin*/
#define BULKXFER_CBWBUF          0x20           /* enable CBW dedicated buffer*/
/*HP05_End*/
#define BULKXFER_IOC             0x80           /* determine to generate interrupt for completion or not*/
#define BULKXFER_DATA1           0x80           /* for data_toggle 1*/
#define BULKXFER_DATA0           0x00           /* for data_toggle 0*/

/* Interupt endpoint transfer descriptor - 33*/
#define INTXFER_XACTERR          0x01           /* trnasacion error for interrupt transfer*/
#define INTXFER_ACTIVE           0x02           /* transfer active*/
#define INTXFER_IOC              0x04           /* determine to generate interrupt for completion or not*/
#define INTXFER_DATA1            0x08           /* for data_toggle 1*/
#define INTXFER_DATA0            0x00           /* for data_toggle 0*/

/*UDC 0x400 ~ 0x7FF*/
struct UDC_REGISTER{
	UCHAR           Reserved0[0x20];    /* 0x00~0x19 Reserved 0*/
	UCHAR           CommandStatus;      /* USB device commmand/status register  - 20*/
	UCHAR           DeviceAddr;         /* The device address register          - 21*/

	UCHAR           PortControl;        /* Device port control register         - 22*/
	UCHAR           IntEnable;          /* Device itnerrupt enable register     - 23*/

	UCHAR           ControlEpControl;   /* Control endpoint control             - 24*/
	UCHAR           ControlEpReserved;  /* Control endpoint reserved byte       - 25*/
	UCHAR           ControlEpMaxLen;    /* max length of control endpoint       - 26*/
	UCHAR           ControlEpEpNum;     /* The control endpoint number          - 27*/

	UCHAR           Bulk1EpControl;      /* Bulk endpoint control                - 28*/
	UCHAR           Bulk1EpOutEpNum;     /* Bulk out endpoint number             - 29*/
	UCHAR           Bulk1EpMaxLen;       /* Bulk maximum length                  - 2A*/
	UCHAR           Bulk1EpInEpNum;      /* Bulk In endpoint number              - 2B*/

	UCHAR           InterruptEpControl; /* Interrupt endpoint control           - 2C*/
	UCHAR           InterruptReserved;  /* Interrupt endpoint reserved byte     - 2D*/
	UCHAR           InterruptEpMaxLen;  /* Interrupt maximum transfer length    - 2E*/
	UCHAR           InterruptEpEpNum;   /* interrupt endpoint number            - 2F*/

	UCHAR           ControlDesStatus;   /* Control transfer descriptor status byte  - 30*/
	UCHAR           ControlDesControl;  /* Control transfer descriptor control byte - 31*/
	UCHAR           ControlDesTbytes;   /* Control transfer descriptor total bytes  - 32*/

	UCHAR           InterruptDes;       /* Interupt endpoint transfer descriptor - 33*/

	UCHAR           Bulk1DesStatus;      /* Bulk endpoint transfer descriptor status            - 34*/
	UCHAR           Bulk1DesTbytes0;     /* Bulk endpoint transfer descriptor control           - 35*/
	UCHAR           Bulk1DesTbytes1;     /* Bulk endpoint transfer descriptor lower total bytes - 36*/
	UCHAR           Bulk1DesTbytes2;     /* Bulk endpoint transfer descriptor higer total bytes - 37*/

	UCHAR           MacToParameter;     /* USB2.0 MAC timeout parameter register - 38*/
	UCHAR           PhyMisc;            /* PHY misc control register             - 39*/
	UCHAR           MiscControl0;       /* Misc. control register 0              - 3A*/
	UCHAR           MiscControl1;       /* Misc. control register 1              - 3B*/
	UCHAR           MiscControl2;       /* Misc. control register 2              - 3C*/
	UCHAR           MiscControl3;       /* Misc. control register 3              - 3D*/
	UCHAR           MACDelay;           /* MAC receiver enable delay parameter   - 3E*/
	UCHAR           FunctionPatchEn;    /* Function Patch Enable bits            - 3F*/

	UCHAR           InterruptInBuf[32]; /* Interrupt In Endpoint Data Buffer     - 40 ~ 5F (32 bytes) for CBW(31 bytes)*/

	UCHAR           Reserved1[0x2A0];   /* 0x060~0x2FF ; 0x2FF - 0x60 + 1 = 0x2A0  for Bulk Endpoint 01 02 DMA Source / Destination*/
	UCHAR           SetupCommand[8];    /* 0x300~0x307 */
	UCHAR           PHYControlBit;      /* 0x308 Phy Control Bit */
	UCHAR           Reserved2[2];       /* 0x309 ~ 0x30A*/
	UCHAR           NewUSBPhyTestControl;/* 0x30B New USB 1.1 and USB 2.0 PHY test control bit*/
	UCHAR           NewUSBPhyTestStatus; /* 0x30C New USB 1.1 and USB 2.0 PHY test status bit*/
	UCHAR           SelfPowerConnect;           /* 0x30D Debug pins for USBC side control bit*/

	UCHAR           Reserved3[2];           /* 0x30E ~ 0x30F*/

	UCHAR           Bulk2EpControl;      /* Bulk endpoint control                - 0x310*/
	UCHAR           Bulk2EpOutEpNum;     /* Bulk out endpoint number             - 0x311*/
	UCHAR           Bulk2EpMaxLen;       /* Bulk maximum length                  - 0x312*/
	UCHAR           Bulk2EpInEpNum;      /* Bulk In endpoint number              - 0x313*/

	UCHAR           Bulk2DesStatus;      /* Bulk endpoint transfer descriptor status            - 0x314*/
	UCHAR           Bulk2DesTbytes0;     /* Bulk endpoint transfer descriptor control           - 0x315*/
	UCHAR           Bulk2DesTbytes1;     /* Bulk endpoint transfer descriptor lower total bytes - 0x316*/
	UCHAR           Bulk2DesTbytes2;     /* Bulk endpoint transfer descriptor higer total bytes - 0x317*/

	UCHAR           Reserved4[0x28];     /*0x318 ~ 0x33F ; 0x33F - 0x318 + 1 = 0x28*/

	UCHAR           ControlDataBuf[64];  /*0x340~0x37F Control Endpoint Data Buffer */
};

/*DMA_Context_Control*/
/*1:IF0-->peripheral 0:peripher-->IF0*/
#define DMA_TRANS_OUT_DIR						0x00400000
/*1:Run 0:Stop Scheduled*/
#define DMA_RUN									0x00000080

/*DMA Interrupt Enable*/
/*enable interfac 0 interrupt*/
#define DMA_INTERRUPT_ENABLE0		0x00000001
/*enable interfac 1 interrupt*/
#define DMA_INTERRUPT_ENABLE1		0x00000002

/*enable interfac 0 status*/
#define DMA_INTERRUPT_STATUS0		0x00000001
/*enable interfac 1 status*/
#define DMA_INTERRUPT_STATUS1		0x00000002

struct UDC_DMA_REG {
		union { /*Rx00 - DMA Global Register (0x100~0x103)*/
			UINT DMA_Global;
			struct {
				UINT DMAConrollerEnable:1;
				UINT Reserved0:7;
				UINT SoftwareReset:1;
				UINT SoftwareReset0:1;
				UINT SoftwareReset1:1;
				UINT Reserved1:21;
			} DMA_Global_Bits;
		};

		union { /*Rx04 - DMA Interrupt Enable (0x104~x107)*/
			UINT DMA_IER;
			struct {
				UINT DMAInterruptEnable0:1;
				UINT DMAInterruptEnable1:1;
				UINT Reserved:30;
			} DMA_IER_Bits;
		};

		union { /*Rx08 - DMA Interrupt Status Register (0x108~0x10b)*/
			UINT DMA_ISR;
			struct {
				UINT InterruptStatus0:1;
				UINT InterruptStatus1:1;
				UINT Reserved:30;
			} DMA_ISR_Bits;
		};

		/*Interface 0*/
		union {/*Rx10c - DMA Memory-Descriptor Point Register dor Interdace 0 (0x10c~0x10f)*/
			UINT DMA_Descriptor_Point0;
			struct {
				UINT Reserved:2;
				UINT DesBaseAddr:30;
			} DMA_Descriptor_Point0_Bits;
		};

		union { /*Rx110 - DMA Residual Bytes Register for Interface 0 (Rx110~Rx103)*/
			UINT DMA_Residual_Bytes0;
			struct {
				UINT ResidualBytes:16;
				UINT IntEn:1;
				UINT Reseved:13;
				UINT Format:1;
				UINT End:1;
			} DMA_Residual_Bytes0_Bits;
		};

		/*Rx114 - DMA Data Address Register for Interface 0 (0x114~0x117)*/
		UINT DMA_Data_Addr0;

		union {/*Rx118 DMA Branch Address Register for INterface 0(0x118~0x11b)*/
			UINT DMA_Branch_Addr0;
			struct {
				UINT Reserved:4;
				UINT BranchDesxriptorAddress:28;
			} DMA_Branch_Addr0_Bits;
		};

		union {/*Rx11c DMA command Pointer Register for Interrface 0(0x11c~0x11f)*/
			UINT Descriptor_Addr0;
			struct {
				UINT Reserved:4;
				UINT DescriptorAddr:28;
			} Descriptor_Addr0_Bits;
		};

		union {/*Rx120 DMA Context Control Register for Interfaxe 0(0x120~0x123)*/
			UINT DMA_Context_Control0;
			struct {
				UINT EventCode:4;
				UINT Reserved0:3;
				UINT Run:1;
				UINT Active:1;
				UINT P0Cmpl:1;
				UINT Reserved1:2;
				UINT Prot:4;
				UINT Reserved2:6;
				UINT TransDir:1;
				UINT Reserved3:9;
			} DMA_Context_Control0_Bis;
		};

		/*Interface 1*/
		union {/*Rx124 - DMA Memory-Descriptor Point Register dor Interdace 1 (0x124~0x127)*/
			UINT DMA_Descriptor_Point1;
			struct {
				UINT Reserved:2;
				UINT DesBaseAddr:30;
			} DMA_Descriptor_Point1_Bits;
		};

		union { /*Rx128 - DMA Residual Bytes Register for Interface 1 (Rx128~Rx12b)*/
			UINT DMA_Residual_Bytes1;
			struct {
				UINT ResidualBytes:16;
				UINT IntEn:1;
				UINT Reseved:13;
				UINT Format:1;
				UINT End:1;
			} DMA_Residual_Bytes1_Bits;
		};

		/*Rx12c - DMA Data Address Register for Interface 1 (0x12c~0x12f)*/
		UINT DMA_Data_Addr1;

		union {/*Rx130 DMA Branch Address Register for INterface 10(0x130~0x133)*/
			UINT DMA_Branch_Addr1;
			struct {
				UINT Reserved:4;
				UINT BranchDesxriptorAddress:28;
			} DMA_Branch_Addr1_Bits;
		};

		union {/*Rx134 DMA command Pointer Register for Interrface 1(0x134~0x137)*/
			UINT Descriptor_Addr1;
			struct {
				UINT Reserved:4;
				UINT DescriptorAddr:28;
			} Descriptor_Addr1_Bits;
		};

		union {/*Rx138DMA Context Control Register for Interfaxe 1(0x138~0x13b)*/
			UINT DMA_Context_Control1;
			struct {
				UINT EventCode:4;
				UINT Reserved0:3;
				UINT Run:1;
				UINT Active:1;
				UINT P0Cmpl:1;
				UINT Reserved1:2;
				UINT Prot:4;
				UINT Reserved2:6;
				UINT TransDir:1;
				UINT Reserved3:9;
			} DMA_Context_Control1_Bis;
		};

};

#define DESCRIPTOT_TYPE_SHORT	0
#define DESCRIPTOT_TYPE_LONG	1
#define DESCRIPTOT_TYPE_MIX		2

#define TRANS_OUT	1
#define TRANS_IN	0

#define DES_L_SIZE	0x10
#define DES_S_SIZE	0x4


/*
 *	PDMA Descriptor short
 */
struct _UDC_PDMA_DESC_S {
	union {
		UINT D_Word0;
		struct {
			UINT ReqCount:16;	/* bit  0 -15	-Request count*/
			UINT i:1;					/* bit	16		-interrupt*/
			UINT Reserved:13;
			UINT Format:1;		/* bit	30		-The descriptor format*/
			UINT End:1;				/* bit	31-End flag of descriptor list*/
		} D_Word0_Bits;
	};

	UINT Data_Addr;				/* bit	0-31		-Data Buffer address*/
};

/*
 *	PDMA Descriptor long
 */

struct _UDC_PDMA_DESC_L {
	union {
		UINT D_Word0;
		struct {
			UINT ReqCount:16;	/* bit  0 -15-Request count*/
			UINT i:1;					/* bit	16-interrupt*/
			UINT Reserved:13;
			UINT Format:1;		/* bit	30-The descriptor format*/
			UINT End:1;				/* bit	31-End flag of descriptor list*/
		} D_Word0_Bits;
	};

	UINT Data_Addr;				/* bit	0-31		-Data Buffer address*/

	union {
		UINT Branch_addr;
		struct {
			UINT Reserved:2;
			UINT BranchAddr:30;	/* bit	31-2	-Descriptor Branch address*/
		} Branch_addr_Bits;
	};

	UINT Reserve0;
};

/*Rx00 - UHDC Interrupt Status Register*/
#define UHDC_INT_UDC      0x0700
#define UHDC_INT_UDC_DMA2 0x0400
#define UHDC_INT_UDC_DMA1 0x0200
#define UHDC_INT_UDC_CORE 0x0100

#ifdef OTGIP
/*UHDC Global Register Space : 0x7F0~0x7F7*/
struct USB_GLOBAL_REG {
	union { /*Rx00 - UHDC Interrupt Status Register*/
		USHORT UHDC_Interrupt_Status;
		struct {
			USHORT EHCIInt:1; 	  /* [0] EHCI Interrupt bit*/
			USHORT UHCIInt:1;	  /* [1] UHCI Interrupt bit*/
			USHORT Reserved0:6;  /* reserved for future use*/
			USHORT UDCCoreInt:1; /* [8]  UDC Core Interrupt bit(IOC/Babble/Bus Activity)*/
			USHORT UDCDMA1Int:1; /* [9]  DMA1 Interrupt bit (Bulk Transfer)*/
			USHORT UDCDMA2Int:1; /* [10] DMA2 Interrupt bit (Bulk Transfer)*/
			USHORT Reserved1:5;  /* reserved for future use*/
		} UHDC_Interrupt_Status_Bits;
	};

	union { /*Rx01 - UHDC ID Status Register*/
		UCHAR  UHDC_ID_Status;
		struct {
			UCHAR DualRoleID:1; /* Dual-Role USB Controller ID Value*/
			UCHAR Reserved:7;   /* reserved for future use*/
		} UHDC_ID_Status_Bits;
	};

	union { /*Rx02 - UHDC Offchip Power Control Register*/
		UCHAR  UHDC_OffChip_PowerControl;
		struct {
			UCHAR  PowerSupplyMode:1; /* Power Supply Mode Select (1)SW mode (0)HW mode*/
			UCHAR  PowerSupplySWEn:1; /* Power Supply SW Mode Enable bit */
			UCHAR Reserved:6;         /* reserved for future use    */
		} UHDC_OffChip_PowerControl_Bits;
	};

	union { /*Rx03 - UHDC Clock Reset Control Register*/
		USHORT UHDC_Clock_Reset_Control;
		struct {
			USHORT AHBClkGatingEn:1;          /* [0] AHB clock Gating Enable bit*/
			USHORT HCSuspendClkGatingEn:1;    /* [1] Host Controller Suspend Clock Gating Enable bit*/
			USHORT DCSuspendClkGatingEn:1;    /* [2] Device Controller Suspend Clock Gating Enable bit*/
			USHORT DataReceivingClkGatingEn:1;/* [3] Controller Data-Receiving Clock Gating Enable bit*/
			USHORT Phy120MClkGatingEn:1;      /* [4] Phy 120M Clock Gating Enable bit*/
			USHORT Phy60MClkGatingEn:1;       /* [5] Phy  60M Clock Gating Enable bit*/
			USHORT Phy48MClkGatingEn:1;       /* [6] Phy  48M Clock Gating Enable bit */
			USHORT Reserved0:1;               /* [7] reserved for future use*/
			USHORT AHBResetGatingEn:1;        /* [8] AHB Bus Reset Gating Enable bit*/
			USHORT PowerOnResetGatingEn:1;    /* [9] Power-on Reset Gating Enable bit  */                                     
			USHORT Reserved1:6;               /* [15:10] reserved for future use*/
		} UHDC_Clock_Reset_Control_Bits;
	};

	union { /*Rx04 - UHDC Pullup/Pulldown Control Register*/
		USHORT UHDC_Pull_Up_Down_Control;
		struct {
			USHORT PullUpDownSWControlEn:1;   /* [0] Pullup/Pulldown SW Control Enable bit*/
			USHORT DPLinePullUpEn:1;          /* [1] DP Line Pullup Enable bit*/
			USHORT DPLinePullDownEn:1;        /* [2] DP Line Pulldown Enable bit*/
			USHORT DMLinePullDownEn:1;        /* [3] DM Line Pulldown Enable bit*/
			USHORT HSTerminationRegisterEn:1; /* [4] High Speed Termination Register On bit*/
			USHORT Reserved1:11;              /* [15:5] reserved for future use*/
		} UHDC_Pull_Up_Down_Control_Bits;
	};
};
#endif

#define UDC_FULL_SPEED 0
#define UDC_HIGH_SPEED 1
/* Define the setup command buffer*/

#define MAX_TRANSFER_LENGTH 0x200  /* original is 0x20000, shift 16bits for minimum block size is 0x200*/
/*HP04_End*/

/* Define the interrupt transfer buffer*/
#define INTBUFFERSTART          0x308           /* the start byte of interrupt transfer buffer*/
#define INTBUFFEREND            0x30F           /* the last byte of interrupt transfer buffer*/
#define INTBUFFERSIZE           0x08            /* the size of the interrupt transfer buffer*/

/* Define the control endpoint buffer*/
#define CONTROLBUFFERSTART      0x340           /* the start byte of the control data buffer*/
#define CONTROLBUFFEREND        0x37F           /* the last byte of the control data buffer*/
#define CONTROLBUFFERSIZE       0x40            /* the size of the control data buffer*/

/* Define the bulk transfer buffer*/
#define BULKBUFFERSTART         0x800           /* the start byte of the bulk transfer buffer*/
#define BULKBUFFEREND           0xFFF           /* the last byte of the bulk transfer buffer*/
#define BULKBUFFERSIZE          0x800           /* the size of the bulk transfer buffer*/

/* define the endpoint number for each endpoint*/
#define BULKINEPNUM             0x01
/*HP01_begin*/
#define BULKOUTEPNUM            0x02
/*HP01_End*/
#define INTEPNUM                0x03
#define MAXEPNUM                0x04
/* define the structure of string table for default strings and vendor specified strings*/

/* define the IO state of USB to handle */
#define IOSTATE_CBWIDLE         0x00            /* usb is idle and set the 8051 clock to stop*/
#define IOSTATE_ACCEPTCBW       0x01            /* the usb accept a new CBW*/
#define IOSTATE_DEVICEPREPAREIO 0x02            /* the state will call deivce startio to prepare IO*/
#define IOSTATE_STARTIO         0x03            /* the CBW send to device to start IO operation*/
#define IOSTATE_CSW             0x04            /* the state to send CSW back to host*/
#define IOSTATE_CSWCOMPLETE     0x05            /* the CSW is returned, maybe need some more process*/
#define IOSTATE_TIMEOUT         0x06            /* the IO is hang to long and it is timeout*/
#define IOSTATE_FWDOWNLOAD      0x07            /* Download the new firmware.*/
#define IOSTATE_WAITFORCBW		0x08			/* In this state, the controller is waiting for CBW*/
#define IOSTATE_CSWIDLE			0x09			/* the idle state for wait for CSW complete, 8051 will stopped*/

/*VT3214 Define */
/* define the state of Control endpoint*/
#define CONTROLSTATE_SETUP		0x00			/* the control endpoint is in the Setup state*/
#define CONTROLSTATE_DATA		0x01			/* the control endpoint is in the Data state*/
#define CONTROLSTATE_STATUS		0x02			/* the control endpoint is in the Status state*/


/* VT3300 PACK Size Defination*/
#define UCE_MAX_DMA		((unsigned)64)
#define UIE_MAX_DMA		((unsigned)8)
#define UBE_MAX_DMA		((unsigned)0x20000) /* max length = 128K */
#define EP0_FIFO_SIZE	((unsigned)64)
#define BULK_FIFO_SIZE	((unsigned)512)
#define INT_FIFO_SIZE	((unsigned)8)

/* Define the setup command buffer*/
typedef struct _SETUP_COMMAND
{
unsigned char               RequestType;        /* the Request type of the SETUP command*/
unsigned char               Request;            /* the Request code of the SETUP command*/
unsigned char               ValueLow;           /* the lower value field of the SETUP command*/
unsigned char               ValueHigh;          /* the higher value field of the SETUP command*/
unsigned char               IndexLow;           /* the lower index field of the SETUP command*/
unsigned char               IndexHigh;          /* the higher index field of the SETUP command*/
unsigned char               LengthLow;          /* the lower length  field of the SETUP command*/
unsigned char               LengthHigh;         /* the higher length field of the SETUP command*/
} SETUPCOMMAND, *PSETUPCOMMAND;

/*HW reg-----------------------------------------------------------------*/
//Function Prototype---------------------------------------------------------------------------------
void usb_isr(void);
void config_usb_global_register(void);
void otg_enable_internal_debug_out(void);

//USB Global Test------------------------------------------------------------------------------------
void usb_global_startio(void);

void usb_global_dump_register(void);
void usb_global_port_status(volatile unsigned char port_num);
void usb_global_regulator_autopower(volatile unsigned char port_num);
void usb_global_port_pwr_enable(volatile unsigned char port_num);
void usb_global_port_pwr_disable(void);
void usb_global_regulator_turn_off(volatile unsigned char port_num);
void usb_global_regulator_turn_on(volatile unsigned char port_num);

void usb_global_ulpi_phy_write(volatile unsigned char port_num, volatile unsigned char addr, volatile unsigned char data);
unsigned char usb_global_ulpi_phy_read(volatile unsigned char port_num, volatile unsigned char addr);

void usb_global_bist_control(volatile unsigned char port_num);

//USB Host Test-------------------------------------------------------------------------------------
void usb_host_startio(void);
void usb_high_host_dump_register(void);
void usb_full_host_dump_register(void);
void usb_host_sof_en(volatile unsigned char enable);
unsigned char usb_port0_get_desc(void);

//OTG Test------------------------------------------------------------------------------------------
void usb_otg_startio(void);
void usb_otg_dump_register(void);
void usb_otg_srp_test(unsigned char port_num);
void usb_otg_srp_sess_req(void);

void usb_otg_a_set_b_hnp_en(volatile unsigned char control_bits);

void usb_otg_b_hnp_enable(volatile unsigned char control_bits);
void usb_otg_b_bus_req(volatile unsigned char control_bits);

unsigned char usb_otg_a_get_descriptor(void);
unsigned char usb_otg_b_get_descriptor(void);

//Device Test---------------------------------------------------------------------------------------
void usb_device_startio(void);
void usb_device_dump_register(void);
void usb_device_enable_1_5K_pull_up(unsigned char hnp_enable);
unsigned char usb_vt3104_get_desc_port0(unsigned char is_high_speed);
void usb_force_full_speed(void);

void FillData(UCHAR *Buf, int Size, int InitValue, int Method);
int CheckSetup(UINT SetupLo, UINT SetupHi);
int CheckMem(UCHAR *Mem0, UCHAR *Mem1, int Size);
//-----------------------------------------------------------------------------------------------------
// define pass() and fail() to replace original function call
#define pass() return(USB_TEST_SUCCESS)
#define fail() return(USB_TEST_FAIL)

void wmt_ep_setup(char *name, u8 addr, u8 type,unsigned maxp);
int wmt_udc_setup(void);
int wmt_udc_irq(void);
void dma_irq(int addr);
int BULK_XFER(UCHAR direction,UINT length);
void BULK_EndPoint_EN(unsigned short EP2_max,unsigned short EP3_max,unsigned short EP1_max);


//void udc_disconnect (void);
void 
GoCmd(
    char            *src,
    char            *dst,
    unsigned long   len
    );
//-----------------------------------------------------------------------------------------------------
/*---------------------- usbcore.h ----------------------------------*/

/*
 * Device State (c.f USB Spec 2.0 Figure 9-1)
 *
 * What state the usb device is in.
 *
 * Note the state does not change if the device is suspended, we simply set a
 * flag to show that it is suspended.
 *
 */
typedef enum usb_device_state {
	STATE_INIT,		/* just initialized */
	STATE_CREATED,		/* just created */
	STATE_ATTACHED,		/* we are attached */
	STATE_POWERED,		/* we have seen power indication (electrical bus signal) */
	STATE_DEFAULT,		/* we been reset */
	STATE_ADDRESSED,	/* we have been addressed (in default configuration) */
	STATE_CONFIGURED,	/* we have seen a set configuration device command */
	STATE_UNKNOWN,		/* destroyed */
} usb_device_state_t;

/*
 * Device status
 *
 * Overall state
 */
typedef enum usb_device_status {
	USBD_OPENING,		/* we are currently opening */
	USBD_OK,		/* ok to use */
	USBD_SUSPENDED,		/* we are currently suspended */
	USBD_CLOSING,		/* we are currently closing */
} usb_device_status_t;

typedef enum usb_device_event {

	DEVICE_UNKNOWN,		/* bi - unknown event */
	DEVICE_INIT,		/* bi  - initialize */
	DEVICE_CREATE,		/* bi  - */
	DEVICE_HUB_CONFIGURED,	/* bi  - bus has been plugged int */
	DEVICE_RESET,		/* bi  - hub has powered our port */

	DEVICE_ADDRESS_ASSIGNED,	/* ep0 - set address setup received */
	DEVICE_CONFIGURED,	/* ep0 - set configure setup received */
	DEVICE_SET_INTERFACE,	/* ep0 - set interface setup received */

	DEVICE_SET_FEATURE,	/* ep0 - set feature setup received */
	DEVICE_CLEAR_FEATURE,	/* ep0 - clear feature setup received */

	DEVICE_DE_CONFIGURED,	/* ep0 - set configure setup received for ?? */

	DEVICE_BUS_INACTIVE,	/* bi  - bus in inactive (no SOF packets) */
	DEVICE_BUS_ACTIVITY,	/* bi  - bus is active again */

	DEVICE_POWER_INTERRUPTION,	/* bi  - hub has depowered our port */
	DEVICE_HUB_RESET,	/* bi  - bus has been unplugged */
	DEVICE_DESTROY,		/* bi  - device instance should be destroyed */

	DEVICE_HOTPLUG,		/* bi  - a hotplug event has occured */

	DEVICE_FUNCTION_PRIVATE,	/* function - private */

} usb_device_event_t;


/* USB Requests
 *
 */

struct usb_device_request {
	u8 bmRequestType;
	u8 bRequest;
	u16 wValue;
	u16 wIndex;
	u16 wLength;
} __attribute__ ((packed));


/* USB Status
 *
 */
typedef enum urb_send_status {
	SEND_IN_PROGRESS,
	SEND_FINISHED_OK,
	SEND_FINISHED_ERROR,
	RECV_READY,
	RECV_OK,
	RECV_ERROR
} urb_send_status_t;

typedef struct urb_link {
	struct urb_link *next;
	struct urb_link *prev;
} urb_link;
#define URB_BUF_SIZE 128 /* in linux we'd malloc this, but in u-boot we prefer static data */
struct urb {

	struct usb_endpoint_instance *endpoint;
	struct usb_device_instance *device;

	struct usb_device_request device_request;	/* contents of received SETUP packet */

	struct urb_link link;	/* embedded struct for circular doubly linked list of urbs */

	u8* buffer;
	unsigned int buffer_length;
	unsigned int actual_length;

	urb_send_status_t status;
	int data;

	u16 buffer_data[URB_BUF_SIZE];	/* data received (OUT) or being sent (IN) */
};
/* Endpoint configuration
 *
 * Per endpoint configuration data. Used to track which function driver owns
 * an endpoint.
 *
 */
struct usb_endpoint_instance {
	int endpoint_address;	/* logical endpoint address */

	/* control */
	int status;		/* halted */
	int state;		/* available for use by bus interface driver */

	/* receive side */
	struct urb_link rcv;	/* received urbs */
	struct urb_link rdy;	/* empty urbs ready to receive */
	struct urb *rcv_urb;	/* active urb */
	int rcv_attributes;	/* copy of bmAttributes from endpoint descriptor */
	int rcv_packetSize;	/* maximum packet size from endpoint descriptor */
	int rcv_transferSize;	/* maximum transfer size from function driver */
	int rcv_queue;

	/* transmit side */
	struct urb_link tx;	/* urbs ready to transmit */
	struct urb_link done;	/* transmitted urbs */
	struct urb *tx_urb;	/* active urb */
	int tx_attributes;	/* copy of bmAttributes from endpoint descriptor */
	int tx_packetSize;	/* maximum packet size from endpoint descriptor */
	int tx_transferSize;	/* maximum transfer size from function driver */
	int tx_queue;

	int sent;		/* data already sent */
	int last;		/* data sent in last packet XXX do we need this */
};
/*
 * standard usb descriptor structures
 */

struct usb_endpoint_descriptor {
	u8 bLength;
	u8 bDescriptorType;	/* 0x5 */
	u8 bEndpointAddress;
	u8 bmAttributes;
	u16 wMaxPacketSize;
	u8 bInterval;
} __attribute__ ((packed));

struct usb_interface_descriptor {
	u8 bLength;
	u8 bDescriptorType;	/* 0x04 */
	u8 bInterfaceNumber;
	u8 bAlternateSetting;
	u8 bNumEndpoints;
	u8 bInterfaceClass;
	u8 bInterfaceSubClass;
	u8 bInterfaceProtocol;
	u8 iInterface;
} __attribute__ ((packed));

struct usb_configuration_descriptor {
	u8 bLength;
	u8 bDescriptorType;	/* 0x2 */
	u16 wTotalLength;
	u8 bNumInterfaces;
	u8 bConfigurationValue;
	u8 iConfiguration;
	u8 bmAttributes;
	u8 bMaxPower;
} __attribute__ ((packed));

struct usb_device_descriptor {
	u8 bLength;
	u8 bDescriptorType;	/* 0x01 */
	u16 bcdUSB;
	u8 bDeviceClass;
	u8 bDeviceSubClass;
	u8 bDeviceProtocol;
	u8 bMaxPacketSize0;
	u16 idVendor;
	u16 idProduct;
	u16 bcdDevice;
	u8 iManufacturer;
	u8 iProduct;
	u8 iSerialNumber;
	u8 bNumConfigurations;
} __attribute__ ((packed));

struct usb_string_descriptor {
	u8 bLength;
	u8 bDescriptorType;	/* 0x03 */
	u16 wData[0];
} __attribute__ ((packed));

struct usb_generic_descriptor {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubtype;
} __attribute__ ((packed));

struct usb_alternate_instance {
	struct usb_interface_descriptor *interface_descriptor;

	int endpoints;
	int *endpoint_transfersize_array;
	struct usb_endpoint_descriptor **endpoints_descriptor_array;
};

struct usb_interface_instance {
	int alternates;
	struct usb_alternate_instance *alternates_instance_array;
};

struct usb_configuration_instance {
	int interfaces;
	struct usb_configuration_descriptor *configuration_descriptor;
	struct usb_interface_instance *interface_instance_array;
};

/* USB Device Instance
 *
 * For each physical bus interface we create a logical device structure. This
 * tracks all of the required state to track the USB HOST's view of the device.
 *
 * Keep track of the device configuration for a real physical bus interface,
 * this includes the bus interface, multiple function drivers, the current
 * configuration and the current state.
 *
 * This will show:
 *	the specific bus interface driver
 *	the default endpoint 0 driver
 *	the configured function driver
 *	device state
 *	device status
 *	endpoint list
 */

struct usb_device_instance {

	/* generic */
	char *name;
	struct usb_device_descriptor *device_descriptor;	/* per device descriptor */

	void (*event) (struct usb_device_instance *device, usb_device_event_t event, int data);

	/* bus interface */
	struct usb_bus_instance *bus;	/* which bus interface driver */

	/* configuration descriptors */
	int configurations;
	struct usb_configuration_instance *configuration_instance_array;

	/* device state */
	usb_device_state_t device_state;	/* current USB Device state */
	usb_device_state_t device_previous_state;	/* current USB Device state */

	u8 address;		/* current address (zero is default) */
	u8 configuration;	/* current show configuration (zero is default) */
	u8 interface;		/* current interface (zero is default) */
	u8 alternate;		/* alternate flag */

	usb_device_status_t status;	/* device status */

	int urbs_queued;	/* number of submitted urbs */

	/* Shouldn't need to make this atomic, all we need is a change indicator */
	unsigned long usbd_rxtx_timestamp;
	unsigned long usbd_last_rxtx_timestamp;

};

/* Bus Interface configuration structure
 *
 * This is allocated for each configured instance of a bus interface driver.
 *
 * The privdata pointer may be used by the bus interface driver to store private
 * per instance state information.
 */
struct usb_bus_instance {

	struct usb_device_instance *device;
	struct usb_endpoint_instance *endpoint_array;	/* array of available configured endpoints */

	int max_endpoints;	/* maximimum number of rx enpoints */
	unsigned char			maxpacketsize;

	unsigned int serial_number;
	char *serial_number_str;
	void *privdata;		/* private data for the bus interface */

};

/*--------------------------------------------------------------------------------------*/

void udc_endpoint_write (struct usb_endpoint_instance *endpoint);

#endif
