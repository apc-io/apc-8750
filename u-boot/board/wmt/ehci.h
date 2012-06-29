/*-
 * Copyright (c) 2007-2008, Juniper Networks, Inc.
 * Copyright (c) 2008, Michael Trimarchi <trimarchimichael@yahoo.it>
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2 of
 * the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef USB_EHCI_H
#define USB_EHCI_H
//Charles
#ifndef BYTE
#define BYTE            unsigned char
#endif

#ifndef WORD
#define WORD            unsigned short
#endif

#ifndef DWORD
#define DWORD           unsigned long
#endif

#define CONFIG_SYS_HZ		1000
#define BA_EHCI_PCI      0xD8007800 /* USB 2.0 EHCI USB Host Configuration Base Address */
#define BA_EHCI_REG      0xD8007900 /* USB 2.0 EHCI USB Host Register Base Address */
#define CapRegBase BA_EHCI_REG
#define OpRegBase  (BA_EHCI_REG + 0x10)

#define USB_TEST_SUCCESS        0
#define USB_TEST_FAIL           1
#define USB_TEST_TIMEOUT        2

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

#if !defined(CONFIG_SYS_USB_EHCI_MAX_ROOT_PORTS)
#define CONFIG_SYS_USB_EHCI_MAX_ROOT_PORTS	4
#endif

/* (shifted) direction/type/recipient from the USB 2.0 spec, table 9.2 */
#define DeviceRequest \
	((USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE) << 8)

#define DeviceOutRequest \
	((USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_DEVICE) << 8)

#define InterfaceRequest \
	((USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_INTERFACE) << 8)

#define EndpointRequest \
	((USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_INTERFACE) << 8)

#define EndpointOutRequest \
	((USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_INTERFACE) << 8)

/*
 * Register Space.
 */
struct ehci_hccr {
	uint32_t cr_capbase;
#define HC_LENGTH(p)		(((p) >> 0) & 0x00ff)
#define HC_VERSION(p)		(((p) >> 16) & 0xffff)
	uint32_t cr_hcsparams;
#define HCS_PPC(p)		((p) & (1 << 4))
#define HCS_INDICATOR(p)	((p) & (1 << 16)) /* Port indicators */
#define HCS_N_PORTS(p)		(((p) >> 0) & 0xf)
	uint32_t cr_hccparams;
	uint8_t cr_hcsp_portrt[8];
} __attribute__ ((packed));

struct ehci_hcor {
	uint32_t or_usbcmd;
#define CMD_PARK	(1 << 11)		/* enable "park" */
#define CMD_PARK_CNT(c)	(((c) >> 8) & 3)	/* how many transfers to park */
#define CMD_ASE		(1 << 5)		/* async schedule enable */
#define CMD_LRESET	(1 << 7)		/* partial reset */
#define CMD_IAAD	(1 << 5)		/* "doorbell" interrupt */
#define CMD_PSE		(1 << 4)		/* periodic schedule enable */
#define CMD_RESET	(1 << 1)		/* reset HC not bus */
#define CMD_RUN		(1 << 0)		/* start/stop HC */
	uint32_t or_usbsts;
#define	STD_ASS		(1 << 15)
#define STS_HALT	(1 << 12)
	uint32_t or_usbintr;
#define INTR_UE         (1 << 0)                /* USB interrupt enable */
#define INTR_UEE        (1 << 1)                /* USB error interrupt enable */
#define INTR_PCE        (1 << 2)                /* Port change detect enable */
#define INTR_SEE        (1 << 4)                /* system error enable */
#define INTR_AAE        (1 << 5)                /* Interrupt on async adavance enable */
	uint32_t or_frindex;
	uint32_t or_ctrldssegment;
	uint32_t or_periodiclistbase;
	uint32_t or_asynclistaddr;
	uint32_t _reserved_[9];
	uint32_t or_configflag;
#define FLAG_CF		(1 << 0)	/* true:  we'll support "high speed" */
	uint32_t or_portsc[CONFIG_SYS_USB_EHCI_MAX_ROOT_PORTS];
	uint32_t or_systune;
} __attribute__ ((packed));

#define USBMODE		0x68		/* USB Device mode */
#define USBMODE_SDIS	(1 << 3)	/* Stream disable */
#define USBMODE_BE	(1 << 2)	/* BE/LE endiannes select */
#define USBMODE_CM_HC	(3 << 0)	/* host controller mode */
#define USBMODE_CM_IDLE	(0 << 0)	/* idle state */

/* Interface descriptor */
struct usb_linux_interface_descriptor {
	unsigned char	bLength;
	unsigned char	bDescriptorType;
	unsigned char	bInterfaceNumber;
	unsigned char	bAlternateSetting;
	unsigned char	bNumEndpoints;
	unsigned char	bInterfaceClass;
	unsigned char	bInterfaceSubClass;
	unsigned char	bInterfaceProtocol;
	unsigned char	iInterface;
} __attribute__ ((packed));

/* Configuration descriptor information.. */
struct usb_linux_config_descriptor {
	unsigned char	bLength;
	unsigned char	bDescriptorType;
	unsigned short	wTotalLength;
	unsigned char	bNumInterfaces;
	unsigned char	bConfigurationValue;
	unsigned char	iConfiguration;
	unsigned char	bmAttributes;
	unsigned char	MaxPower;
} __attribute__ ((packed));

#if defined CONFIG_EHCI_DESC_BIG_ENDIAN
#define	ehci_readl(x)		(*((volatile u32 *)(x)))
#define ehci_writel(a, b)	(*((volatile u32 *)(a)) = ((volatile u32)b))
#else
#define ehci_readl(x)		cpu_to_le32((*((volatile u32 *)(x))))
#define ehci_writel(a, b)	(*((volatile u32 *)(a)) = \
					cpu_to_le32(((volatile u32)b)))
#endif

#if defined CONFIG_EHCI_MMIO_BIG_ENDIAN
#define hc32_to_cpu(x)		be32_to_cpu((x))
#define cpu_to_hc32(x)		cpu_to_be32((x))
#else
#define hc32_to_cpu(x)		le32_to_cpu((x))
#define cpu_to_hc32(x)		cpu_to_le32((x))
#endif

#define EHCI_PS_WKOC_E		(1 << 22)	/* RW wake on over current */
#define EHCI_PS_WKDSCNNT_E	(1 << 21)	/* RW wake on disconnect */
#define EHCI_PS_WKCNNT_E	(1 << 20)	/* RW wake on connect */
#define EHCI_PS_PO		(1 << 13)	/* RW port owner */
#define EHCI_PS_PP		(1 << 12)	/* RW,RO port power */
#define EHCI_PS_LS		(3 << 10)	/* RO line status */
#define EHCI_PS_PR		(1 << 8)	/* RW port reset */
#define EHCI_PS_SUSP		(1 << 7)	/* RW suspend */
#define EHCI_PS_FPR		(1 << 6)	/* RW force port resume */
#define EHCI_PS_OCC		(1 << 5)	/* RWC over current change */
#define EHCI_PS_OCA		(1 << 4)	/* RO over current active */
#define EHCI_PS_PEC		(1 << 3)	/* RWC port enable change */
#define EHCI_PS_PE		(1 << 2)	/* RW port enable */
#define EHCI_PS_CSC		(1 << 1)	/* RWC connect status change */
#define EHCI_PS_CS		(1 << 0)	/* RO connect status */
#define EHCI_PS_CLEAR		(EHCI_PS_OCC | EHCI_PS_PEC | EHCI_PS_CSC)

#define EHCI_PS_IS_LOWSPEED(x)	(((x) & EHCI_PS_LS) == (1 << 10))

/*
 * Schedule Interface Space.
 *
 * IMPORTANT: Software must ensure that no interface data structure
 * reachable by the EHCI host controller spans a 4K page boundary!
 *
 * Periodic transfers (i.e. isochronous and interrupt transfers) are
 * not supported.
 */

/* Queue Element Transfer Descriptor (qTD). */
struct qTD {
	/* this part defined by EHCI spec */
	uint32_t qt_next;		/* see EHCI 3.5.1 */
#define	QT_NEXT_TERMINATE	1
	uint32_t qt_altnext;		/* see EHCI 3.5.2 */
	uint32_t qt_token;		/* see EHCI 3.5.3 */
	uint32_t qt_buffer[5];		/* see EHCI 3.5.4 */
	uint32_t qt_buffer_hi[5];	/* Appendix B */
	/* pad struct for 32 byte alignment */
	uint32_t unused[3];
};

/* Queue Head (QH). */
struct QH {
	uint32_t qh_link;
#define	QH_LINK_TERMINATE	1
#define	QH_LINK_TYPE_ITD	0
#define	QH_LINK_TYPE_QH		2
#define	QH_LINK_TYPE_SITD	4
#define	QH_LINK_TYPE_FSTN	6
	uint32_t qh_endpt1;
	uint32_t qh_endpt2;
	uint32_t qh_curtd;
	struct qTD qh_overlay;
	/*
	 * Add dummy fill value to make the size of this struct
	 * aligned to 32 bytes
	 */
	uint8_t fill[16];
};
//Charles
typedef struct _HCSPARAMS {
	DWORD    N_Ports:4;
	DWORD    PortPowerControl:1;
	DWORD    Reserved1:2;
	DWORD    PortRouteRule:1;/*bit 7*/
	DWORD    NumberPortPerCC:4;/*bit 8-11*/
	DWORD    NumberOfCC:4;/*bit 12-15*/
	DWORD    PortIndicator:1;/*bit 16*/
	DWORD    Reserved2:3;/*bit 17-19*/
	DWORD    DebugPortNum:4;/*bit 20-23*/
	DWORD    Reserved3:8;/*bit 24-31*/
} __attribute__ ((packed))  HCSPARAMS, *pHCSPARAMS;

typedef struct _HCCPARAMS {
	DWORD    _64BitAddress:1;/*bit 0*/
	DWORD    ProgrammableFrameList:1;/*bit 1*/
	DWORD    Reserved1:2;/*bit 2-3*/
	DWORD    IsoScheduleThreshold:4;/*bit 4-7*/
	DWORD    Reserved2:24;/* bit 8-31*/
} __attribute__ ((packed))  HCCPARAMS, *pHCCPARAMS;

typedef struct _HostControllerCapability {
	BYTE     CapLength;/*byte 0*/
	BYTE     Reserved;/*byte 1*/
	WORD     HCIVersion;/*byte 2*/
	HCSPARAMS HCSParams;/*byte 4*/
	HCCPARAMS HCCParams;/*byte 8*/
	BYTE     PortRoute[15];/*byte c-1b*/
} HCCapability, *pHCCapability;

typedef struct _EHCICommand {
	DWORD   RunStop:1;/*bit 0*/
	DWORD   HCReset:1;/*bit 1*/
	DWORD   FrameListSize:2;/*bit 2-3*/
	DWORD   PeriodicEnable:1;/*bit 4*/
	DWORD   AsynchronousEnable:1;/*bit 5*/
	DWORD   IntOnAsyAdvDoorbell:1;/*bit 6*/
	DWORD   LightHCReset:1;/*bit 7*/
	DWORD   Reserved1:8;/*bit 8-15*/
	DWORD   InterruptThreshold:8;/*bit 16-23*/
	DWORD   Reserved2:8;/*bit 24-31*/
}  __attribute__ ((packed)) EHCICommand, *pEHCICommand;

typedef struct _EHCIStatus {
	DWORD    USBInt:1;/* bit 0*/
	DWORD    USBErrInt:1;/* bit 1*/
	DWORD    PortChangeDetect:1;/* bit 2*/
	DWORD    FrameListRollover:1;/* bit 3*/
	DWORD    HostSysErr:1;/* bit 4*/
	DWORD    IntOnAsyncAdv:1;/* bit 5*/
	DWORD    Reserved1:6;/* bit 6-11*/
	DWORD    HCHalted:1;/* bit 12*/
	DWORD    Reclamation:1;/* bit 13*/
	DWORD    PeriodicSchStatus:1;/* bit 14*/
	DWORD    AsyncSchStatus:1;/* bit 15*/
	DWORD    Reserved2:16;/* bit 16-31*/
} __attribute__ ((packed))  EHCIStatus, *pEHCIStatus;

typedef struct _EHCIIntEnable {
	DWORD    USBIntEnable:1;/* bit 0*/
	DWORD    USBErrIntEnable:1;/* bit 1*/
	DWORD    PortChgIntEnable:1;/* bit 2*/
	DWORD    FrameListROEnable:1;/* bit 3*/
	DWORD    HostSysErrEnable:1;/* bit 4*/
	DWORD    IntOnAsyncAdvEnable:1;/* bit 5*/
	DWORD    Reserved:26;/* bit 6-31*/
}  __attribute__ ((packed)) EHCIIntEnable, *pEHCIIntEnable;

typedef struct _EHCIConfigFlag {
	DWORD    ConfigFlag:1;/* bit 0*/
	DWORD    Reserved:31;/* bit 1-31*/
} __attribute__ ((packed)) EHCIConfigFlag, *pEHCIConfigFlag;

typedef struct _EHCIPortControlStatus {
	DWORD   CurrentConStatus:1;/* bit 0*/
	DWORD   ConnectStatusChg:1;/* bit 1*/
	DWORD   PortEnDisable:1;/* bit 2*/
	DWORD   PortEnDisChg:1;/* bit 3*/
	DWORD   OverCurrentActive:1;/* bit 4*/
	DWORD   OverCurrentChg:1;/* bit 5*/
	DWORD   ForcePortResume:1;/* bit 6*/
	DWORD   Suspend:1;/* bit 7*/
	DWORD   PortReset:1;/* bit 8*/
	DWORD   HighSpdDevice:1;/* bit 9*/
	DWORD   LineStatus:2;/* bit 10-11*/
	DWORD   PortPower:1;/* bit 12*/
	DWORD   PortOwer:1;/* bit 13*/
	DWORD   PortIndicatorControl:2;/* bit 14-15*/
	DWORD   PortTestControl:4;/* bit 16-19*/
	DWORD   WakeOnConEnable:1;/* bit 20*/
	DWORD   WakeOnDisconEnable:1;/* bit 21*/
	DWORD   WakeOnOverCurrentEnable:1;/* bit 22*/
	DWORD   Reserved:9;/* bit 23-31*/
} __attribute__ ((packed)) EHCIPortCtrlStatus, *pEHCIPortCtrlStatus;

typedef struct _EHCIOperationalReg {
	EHCICommand     CommandReg;/* byte 00*/
	EHCIStatus      StatusReg;/* byte 04*/
	EHCIIntEnable   IntEnableReg;/* byte 08*/
	DWORD           FrameIndex;/* byte 0c*/
	DWORD           CTRLSegment;/* byte 10*/
	DWORD           PeriodicBase;/* byte 14*/
	DWORD           AsyncBase;/* byte 18*/
	DWORD           Reserved[9];/* byte 1C-3F*/
	EHCIConfigFlag  ConfigFlag;/* byte 40*/
	EHCIPortCtrlStatus Port[10];/* byte 44*/
} __attribute__ ((packed)) EHCIOPRegister, *pEHCIOPRegister;

/* Low level init functions */
int ehci_hcd_init(void);
int ehci_hcd_stop(void);
void ehci_config_port_owner(void);
int ehci_detect_device(void);
int ehci_enable_port(void);
unsigned char usb_ehci_initial(void);
unsigned char strtochar(unsigned char Value);

#define	IRQ_UHDC             43
#define   USB_PORT_A 1
#define   USB_PORT_B 2
#define   USB_PORT_C 4
#define   USB_PORT_D 8

#ifdef	USB_UHCI_DEBUG
int usb_display_td(struct qTD *td);
#endif

int usb_check_td(struct qTD *td);
int usb_check_bulk_td(struct qTD *td);
int parse_usb_param(char *name);
struct usb_operation_t {
	unsigned char enable;
	unsigned char port_num;	
};


#endif /* USB_EHCI_H */
