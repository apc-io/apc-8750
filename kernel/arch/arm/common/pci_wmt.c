/**
 *  linux/arch/arm/common/pci_wmt.c
 *
 *	Copyright (c) 2008 WonderMedia Technologies, Inc.
 *
 *	This program is free software: you can redistribute it and/or modify it under the
 *	terms of the GNU General Public License as published by the Free Software Foundation,
 *	either version 2 of the License, or (at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful, but WITHOUT
 *	ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 *	PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *	You should have received a copy of the GNU General Public License along with
 *	this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *	WonderMedia Technologies, Inc.
 *	10F, 529, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C.
 */

#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/ptrace.h>
#include <linux/interrupt.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/ioport.h>

#include <asm/io.h>
#include <asm/system.h>

#include <asm/mach/pci.h>

#define PATA
/*#define SATA*/
#define USB_HOST
#define MAC
/*#define EXT_PCI*/

/* #define CONFIG_PCI_DEBUG */
ulong
PCI_GetConfigRegisterDWORD(
	int bus,
	int device,
	int fctn,
	int target
	);
void
PCI_SetConfigRegisterDWORD(
	int bus,
	int device,
	int fctn,
	int target,
	ulong data
	);
#ifdef PATA
void init_int_pata(void);
#endif
#ifdef SATA
void init_int_sata(void);
#endif
#ifdef USB_HOST
void init_int_usb(void);
#endif
#ifdef MAC
void init_int_mac(void);
#endif
#ifdef EXT_PCI
void init_ext_pci(void);
#endif

#define CONFIG_CMD(bus, devfn, where)   (0x80000000 | ((devfn) << 8) | ((where) & ~3))

static u32 pci_config_ba;
static u32 pci_config_addr;
static u32 pci_config_data;

#define MAX_PCI_DEV     0xC
#define INT_SATA        0
#define INT_PATA        1
#define INT_MAC0        2
#define INT_MAC1        3
#define INT_USB_EHCI    4
#define INT_USB_UHCI    5
#define INT_USB_UHCI2   6
#define EXT_PCI7        7
#define EXT_PCI8        8
#define EXT_PCI9        9
#define EXT_PCIA        0xA
#define EXT_PCIB        0xB

u32 pci_config_mask[MAX_PCI_DEV][8][0x10];
u32 pci_config_shadow[MAX_PCI_DEV][8][0x10];
u32 pci_config_ro[MAX_PCI_DEV][8][0x10];

#ifdef SATA
#define SATA_PCI_CONFIG (BA_SATA+0x100)	//0xFE00d100
#endif

static int
wmt_read_config(
		struct pci_bus *bus,
		unsigned int devfn,
		int where,
		int size,
		u32 *value
		)
{
	u32 bar, mask, devno, func;

	devno = devfn >> 3;
	func = devfn & 7;
	*value = 0xFFFFFFFF;

#ifndef EXT_PCI
	if (devno > 6)
		return 0;
#endif
		
	switch (devno) { /* Check the dev number */
	/* external PCI devices */
	case EXT_PCI7:
	case EXT_PCI8:
	case EXT_PCI9:
	case EXT_PCIA:
	case EXT_PCIB:
	{
		if ((where >= 0x10) && (where < 0x28)) {
			switch (size) {
			case 1:
				bar = (where & ~3)/4;
				mask = 0xFF << 8*(where & 3);
				*value = pci_config_shadow[devno][func][bar] & mask;
				*value = (*value) >> 8*(where & 3);
				break;

			case 2:
				bar = (where & ~3)/4;
				mask = 0xFFFF << 8*(where & 2);
				*value = pci_config_shadow[devno][func][bar] & mask;
				*value = (*value) >> 8*(where & 2);
				break;

			case 4:
				bar = (where & ~3)/4;
				mask = 0xFFFFFFFF;
				*value = pci_config_shadow[devno][func][bar] & mask;
			}
		} else {
			writel(CONFIG_CMD(bus, devfn, where), pci_config_addr);
			switch (size) {
			case 1:
				*value = readb(pci_config_data + (where&3));
				break;

			case 2:
				*value = readw(pci_config_data + (where&2));
				break;

			case 4:
				*value = readl(pci_config_data);
				break;
			}
		}
	}
	break;

	/* internal PCI devices */
#ifdef SATA
	case INT_SATA:
		if ((where >= 0xA0) && (where <= 0xAF)) {
			switch (size) {
			case 1:
				*value = inb((SATA_PCI_CONFIG + where));
				break;
			case 2:
				*value = inw((SATA_PCI_CONFIG + (where & ~1)));
				break;
			case 4:
				*value = inl((SATA_PCI_CONFIG + (where & ~3)));
				break;
			}
			break;
		}
#endif		
	case INT_PATA:
	case INT_MAC0:
	case INT_MAC1:
		if (devfn & 7)
			break;

		switch (size) {
		case 1:
			if ((where  < 0x40)) {
				bar = (where & ~3)/4;
				mask = 0xFF << 8*(where & 3);
				*value = pci_config_shadow[devno][0][bar] & mask;
				*value = (*value) >> 8*(where & 3);
			} else
				*value = 0;
			break;

		case 2:
			if (where < 0x40) {
				bar = (where & ~3)/4;
				mask = 0xFFFF << 8*(where & 2);
				*value = pci_config_shadow[devno][0][bar] & mask;
				*value = (*value) >> 8*(where & 2);
			} else
				*value = 0;
			break;

		case 4:
			if (where < 0x40) {
				bar = (where & ~3)/4;
				mask = 0xFFFFFFFF;
				*value = pci_config_shadow[devno][0][bar] & mask;
			} else
				*value = 0;
		}

		break;
	case INT_USB_UHCI:
	case INT_USB_UHCI2:
	case INT_USB_EHCI:
		if (devfn & 7)
			break;
		switch (size) {
		case 1:
			if (where  < 0x40) {
				bar = (where & ~3)/4;
				mask = 0xFF << 8*(where & 3);
				*value = pci_config_shadow[devno][0][bar] & mask;
				*value = (*value) >> 8*(where & 3);
			} else
				*value = 0;
			break;

		case 2:
			if (where < 0x40) {
				bar = (where & ~3)/4;
				mask = 0xFFFF << 8*(where & 2);
				*value = pci_config_shadow[devno][0][bar] & mask;
				*value = (*value) >> 8*(where & 2);
			} else if (where == 0x84) {
				if ((devno == INT_USB_UHCI)||(devno == INT_USB_UHCI2)) {
					bar = pci_config_shadow[devno][0][8];
					bar &= ~0x1;
				} else
					bar = pci_config_shadow[devno][0][4];	
				
				bar = bar - 0x100;
				/*CharlesTu,2011.02.16,modify ehci pci base address to vertual address 0xfe007800*/
				if (devno == INT_USB_EHCI)  
					bar = bar + WMT_MMAP_OFFSET;
					
				*value = * (volatile unsigned short *)(bar + where);
					
			}else
				*value = 0;
			break;

		case 4:
			if (where < 0x40) {
				bar = (where & ~3)/4;
				mask = 0xFFFFFFFF;
				*value = pci_config_shadow[devno][0][bar] & mask;
			} else
				*value = 0;
		}

	default:
			break;
	}

#ifdef CONFIG_PCI_DEBUG
	if (size == 1)
		printk("pci config read(B):dev:0x%02X fn:0x%02X where:0x%02X-> 0x%08X\n",
		devfn>>3,
		devfn&7,
		where,
		*value
		);
	else if (size == 2)
		printk("pci config read(W):dev:0x%02X fn:0x%02X where:0x%02X-> 0x%08X\n",
		devfn>>3,
		devfn&7,
		where,
		*value
		);
	else
		printk("pci config read(L):dev:0x%02X fn:0x%02X where:0x%02X-> 0x%08X\n",
		devfn>>3,
		devfn&7,
		where,
		*value
		);
#endif /* CONFIG_PCI_DEBUG */

	return PCIBIOS_SUCCESSFUL;
}

static int
wmt_write_config(struct pci_bus *bus, unsigned int devfn, int where,
					 int size, u32 value)
{
	u32 bar, mask, devno, func;

	devno = devfn >> 3;
	func = devfn & 7;
	switch (devno) { /* Check the dev number */
	/* external PCI devices */
	case EXT_PCI7:
	case EXT_PCI8:
	case EXT_PCI9:
	case EXT_PCIA:
	case EXT_PCIB:
	{
	if ((where >= 0x10) && (where < 0x28)) {
		switch (size) {
		case 1:
			if (where < 0x40) {
				bar = (where & ~3)/4;
				value = value << 8*(where & 3);
				mask = 0xFF << 8*(where & 3);
				/* clear the written byte content */
				pci_config_shadow[devno][func][bar] &= ~mask;
				/* set the written byte content */
				pci_config_shadow[devno][func][bar] |= (value & mask);
				/* only writing the bits which are writable and which is checked
					 by the pci_config_mask[][] */
				pci_config_shadow[devno][func][bar] &= pci_config_mask[devno][func][bar];
				/* set the read only bits which may be clear when written. */
				pci_config_shadow[devno][func][bar] |= pci_config_ro[devno][func][bar];
			}
			break;
		case 2:
			if (where < 0x40) {
				bar = (where & ~3)/4;
				value = value << 8*(where & 2);
				mask = 0xFFFF << 8*(where & 2);
				/* clear the written byte content */
				pci_config_shadow[devno][func][bar] &= ~mask;
				/* set the written byte content */
				pci_config_shadow[devno][func][bar] |= (value & mask);
				/* only writing the bits which are writable and which is checked
					 by the pci_config_mask[][] */
				pci_config_shadow[devno][func][bar] &= pci_config_mask[devno][func][bar];
				/* set the read only bits which may be clear when written. */
				pci_config_shadow[devno][func][bar] |= pci_config_ro[devno][func][bar];
			}
			break;
		case 4:
			if (where < 0x40) {
				bar = (where & ~3)/4;
				mask = 0xFFFFFFFF;
				/* clear the written byte content */
				pci_config_shadow[devno][func][bar] &= ~mask;
				/* set the written byte content */
				pci_config_shadow[devno][func][bar] |= (value & mask);
				/* only writing the bits which are writable and which is checked
					 by the pci_config_mask[][] */
				pci_config_shadow[devno][func][bar] &= pci_config_mask[devno][func][bar];
				/* set the read only bits which may be clear when written. */
				pci_config_shadow[devno][func][bar] |= pci_config_ro[devno][func][bar];
			}
			break;
		}
	} else {
		writel(CONFIG_CMD(bus, devfn, where), pci_config_addr);
		switch (size) {
		case 1:
			outb(value, pci_config_data + (where&3));
			break;
		case 2:
			outw(value, pci_config_data + (where&2));
			break;
		case 4:
			outl(value, pci_config_data);
			break;
		}
	}
	break;
	}
	break;

	/* internal PCI devices */
#ifdef SATA
	case INT_SATA:
	if ((where >= 0xA0) && (where <= 0xAF)) {
		switch (size) {
		case 1:
			outb(value, SATA_PCI_CONFIG + where);
			break;
		case 2:
			outw(value, SATA_PCI_CONFIG + (where & ~1));
			break;
		case 4:
			outl(value, SATA_PCI_CONFIG + (where & ~3));
			break;
		}
		break;
	}
#endif
	case INT_PATA:
	case INT_MAC0:
	case INT_MAC1:
		if (devfn & 7)
			break;

		switch (size) {
		case 1:
			if (where < 0x40) {
				bar = (where & ~3)/4;
				value = value << 8*(where & 3);
				mask = 0xFF << 8*(where & 3);
				/* clear the written byte content */
				pci_config_shadow[devno][0][bar] &= ~mask;
				/* set the written byte content */
				pci_config_shadow[devno][0][bar] |= (value & mask);
				/* only writing the bits which are writable and which is checked
					 by the pci_config_mask[][] */
				pci_config_shadow[devno][0][bar] &= pci_config_mask[devno][0][bar];
				/* set the read only bits which may be clear when written. */
				pci_config_shadow[devno][0][bar] |= pci_config_ro[devno][0][bar];
			}
			break;
		case 2:
			if (where < 0x40) {
				bar = (where & ~3)/4;
				value = value << 8*(where & 2);
				mask = 0xFFFF << 8*(where & 2);
				/* clear the written byte content */
				pci_config_shadow[devno][0][bar] &= ~mask;
				/* set the written byte content */
				pci_config_shadow[devno][0][bar] |= (value & mask);
				/* only writing the bits which are writable and which is checked
					 by the pci_config_mask[][] */
				pci_config_shadow[devno][0][bar] &= pci_config_mask[devno][0][bar];
				/* set the read only bits which may be clear when written. */
				pci_config_shadow[devno][0][bar] |= pci_config_ro[devno][0][bar];
			}
			break;
		case 4:
			if (where < 0x40) {
				bar = (where & ~3)/4;
				mask = 0xFFFFFFFF;
				/* clear the written byte content */
				pci_config_shadow[devno][0][bar] &= ~mask;
				/* set the written byte content */
				pci_config_shadow[devno][0][bar] |= (value & mask);
				/* only writing the bits which are writable and which is checked
					 by the pci_config_mask[][] */
				pci_config_shadow[devno][0][bar] &= pci_config_mask[devno][0][bar];
				/* set the read only bits which may be clear when written. */
				pci_config_shadow[devno][0][bar] |= pci_config_ro[devno][0][bar];
			}
			break;
		}
		break;	
	case INT_USB_UHCI:
	case INT_USB_UHCI2:
	case INT_USB_EHCI:
		if (devfn & 7)
			break;

		switch (size) {
		case 1:
			if (where < 0x40) {
				bar = (where & ~3)/4;
				value = value << 8*(where & 3);
				mask = 0xFF << 8*(where & 3);
				/* clear the written byte content */
				pci_config_shadow[devno][0][bar] &= ~mask;
				/* set the written byte content */
				pci_config_shadow[devno][0][bar] |= (value & mask);
				/* only writing the bits which are writable and which is checked
					 by the pci_config_mask[][] */
				pci_config_shadow[devno][0][bar] &= pci_config_mask[devno][0][bar];
				/* set the read only bits which may be clear when written. */
				pci_config_shadow[devno][0][bar] |= pci_config_ro[devno][0][bar];
			}
			break;
		case 2:
			if (where < 0x40) {
				bar = (where & ~3)/4;
				value = value << 8*(where & 2);
				mask = 0xFFFF << 8*(where & 2);
				/* clear the written byte content */
				pci_config_shadow[devno][0][bar] &= ~mask;
				/* set the written byte content */
				pci_config_shadow[devno][0][bar] |= (value & mask);
				/* only writing the bits which are writable and which is checked
					 by the pci_config_mask[][] */
				pci_config_shadow[devno][0][bar] &= pci_config_mask[devno][0][bar];
				/* set the read only bits which may be clear when written. */
				pci_config_shadow[devno][0][bar] |= pci_config_ro[devno][0][bar];
			} else if (where == 0x84) {
				if ((devno == INT_USB_UHCI)||(devno == INT_USB_UHCI2)) {
					bar = pci_config_shadow[devno][0][8];
					bar &= ~0x1;
				} else
					bar = pci_config_shadow[devno][0][4];	
				 
				bar = bar - 0x100;
				/*CharlesTu,2011.02.16,modify ehci pci base address to vertual address 0xfe007800*/
				if (devno == INT_USB_EHCI)  
					bar = bar + WMT_MMAP_OFFSET;
					
				* (volatile unsigned short *)(bar + where) = value;	
					
			}
			break;
		case 4:
			if (where < 0x40) {
				bar = (where & ~3)/4;
				mask = 0xFFFFFFFF;
				/* clear the written byte content */
				pci_config_shadow[devno][0][bar] &= ~mask;
				/* set the written byte content */
				pci_config_shadow[devno][0][bar] |= (value & mask);
				/* only writing the bits which are writable and which is checked
					 by the pci_config_mask[][] */
				pci_config_shadow[devno][0][bar] &= pci_config_mask[devno][0][bar];
				/* set the read only bits which may be clear when written. */
				pci_config_shadow[devno][0][bar] |= pci_config_ro[devno][0][bar];
			}
			break;
		}
		break;

	default:
	break;
	}

#ifdef CONFIG_PCI_DEBUG
	if (size == 1)
		printk("pci config write(B):dev:0x%02X fn:0x%02X where:0x%02X-> 0x%08X\n",
		devfn>>3,
		devfn&7,
		where,
		value
		);
	else if (size == 2)
		printk("pci config write(W):dev:0x%02X fn:0x%02X where:0x%02X-> 0x%08X\n",
		devfn>>3,
		devfn&7,
		where,
		value
		);
	else
		printk("pci config write(L):dev:0x%02X fn:0x%02X where:0x%02X-> 0x%08X\n",
		devfn>>3,
		devfn&7,
		where,
		value
		);
#endif /* CONFIG_PCI_DEBUG     */

	return PCIBIOS_SUCCESSFUL;
}

static struct pci_ops wmt_pci_ops = {
	.read	= wmt_read_config,
	.write	= wmt_write_config,
};

#ifdef PATA
void init_int_pata(void)
{
	if (0) {
		/* if (ARCH_VT8430) */
		pci_config_shadow[INT_PATA][0][0] = 0x13571106;
		pci_config_shadow[INT_PATA][0][1] = 0x02100005;
		/* {JHTseng 2007/03/02 Change the Byte1 from 8A into 8F,
		Otherwise, the resource will be clear by the kernel */
		pci_config_shadow[INT_PATA][0][2] = 0x01018F00;
		pci_config_shadow[INT_PATA][0][3] = 0x00002000;
		/* Mark 2007/03/12 Modify PCI BAR address, PATA's SG register
		base address from Secondary to Primary */
		//pci_config_shadow[INT_PATA][0][4] = 0xD8008271;
		pci_config_shadow[INT_PATA][0][4] = 0xFE008271;
		//pci_config_shadow[INT_PATA][0][5] = 0xD8008375;
		pci_config_shadow[INT_PATA][0][5] = 0xFE008375;
		pci_config_shadow[INT_PATA][0][6] = 0x0;
		pci_config_shadow[INT_PATA][0][7] = 0x0;
		//pci_config_shadow[INT_PATA][0][8] = 0xD8008509;
		pci_config_shadow[INT_PATA][0][8] = 0xFE008509;
		pci_config_shadow[INT_PATA][0][9] = 0x0;

		pci_config_shadow[INT_PATA][0][0xA] = 0x0;
		pci_config_shadow[INT_PATA][0][0xB] = 0x05811106;
		pci_config_shadow[INT_PATA][0][0xC] = 0x0;
		pci_config_shadow[INT_PATA][0][0xD] = 0x0;
		pci_config_shadow[INT_PATA][0][0xE] = 0x0;
		pci_config_shadow[INT_PATA][0][0xF] = 0x0103;

		pci_config_mask[INT_PATA][0][0] = 0x0;
		pci_config_mask[INT_PATA][0][1] = 0x0;
		pci_config_mask[INT_PATA][0][2] = 0x0;
		pci_config_mask[INT_PATA][0][3] = 0x0;

		pci_config_mask[INT_PATA][0][4] = 0xFFFFFFF8;
		pci_config_mask[INT_PATA][0][5] = 0xFFFFFFFC;
		pci_config_mask[INT_PATA][0][6] = 0x0;
		pci_config_mask[INT_PATA][0][7] = 0x0;
		pci_config_mask[INT_PATA][0][8] = 0xFFFFFFF0;
		pci_config_mask[INT_PATA][0][9] = 0x0;

		pci_config_mask[INT_PATA][0][0xA] = 0x0;
		pci_config_mask[INT_PATA][0][0xB] = 0x0;
		pci_config_mask[INT_PATA][0][0xC] = 0x0;
		pci_config_mask[INT_PATA][0][0xD] = 0x0;
		pci_config_mask[INT_PATA][0][0xE] = 0x0;
		pci_config_mask[INT_PATA][0][0xF] = 0x0;

		pci_config_ro[INT_PATA][0][0] = 0x13591106;
		pci_config_ro[INT_PATA][0][1] = 0x02100005;
		pci_config_ro[INT_PATA][0][2] = 0x01018A00;
		pci_config_ro[INT_PATA][0][3] = 0x00002000;

		pci_config_ro[INT_PATA][0][4] = 0x1;
		pci_config_ro[INT_PATA][0][5] = 0x1;
		pci_config_ro[INT_PATA][0][6] = 0x0;
		pci_config_ro[INT_PATA][0][7] = 0x0;
		pci_config_ro[INT_PATA][0][8] = 0x1;
		pci_config_ro[INT_PATA][0][9] = 0x0;

		pci_config_ro[INT_PATA][0][0xA] = 0x0;
		pci_config_ro[INT_PATA][0][0xB] = 0x05811106;
		pci_config_ro[INT_PATA][0][0xC] = 0x0;
		pci_config_ro[INT_PATA][0][0xD] = 0x0;
		pci_config_ro[INT_PATA][0][0xE] = 0x0;
		pci_config_ro[INT_PATA][0][0xF] = 0x0103;
	} else {
		pci_config_shadow[INT_PATA][0][0] = 0x13591106;
		pci_config_shadow[INT_PATA][0][1] = 0x02000005;
		/* {JHTseng 2007/03/02 Change the Byte1 from 8A into 8F,
		Otherwise, the resource will be clear by the kernel */
		pci_config_shadow[INT_PATA][0][2] = 0x01018F00;
		pci_config_shadow[INT_PATA][0][3] = 0x00000000;/* 0x00002000; */
		/* Mark 2007/03/12 Modify PCI BAR address, PATA's SG register
		base address from Secondary to Primary */
		//pci_config_shadow[INT_PATA][0][4] = 0xD8008101;/* 0xD8008271; */
		pci_config_shadow[INT_PATA][0][4] = 0xFE008101;/* 0xD8008271; */
		//pci_config_shadow[INT_PATA][0][5] = 0xD8008145;/* 0xD8008375; */
		pci_config_shadow[INT_PATA][0][5] = 0xFE008145;/* 0xD8008375; */
		pci_config_shadow[INT_PATA][0][6] = 0x0;
		pci_config_shadow[INT_PATA][0][7] = 0x0;
		//pci_config_shadow[INT_PATA][0][8] = 0xD8008181;
		pci_config_shadow[INT_PATA][0][8] = 0xFE008181;
		pci_config_shadow[INT_PATA][0][9] = 0x0;

		pci_config_shadow[INT_PATA][0][0xA] = 0x0;
		pci_config_shadow[INT_PATA][0][0xB] = 0x13581106;/* 0x05811106; */
		pci_config_shadow[INT_PATA][0][0xC] = 0x0;
		pci_config_shadow[INT_PATA][0][0xD] = 0x0;
		pci_config_shadow[INT_PATA][0][0xE] = 0x0;
		pci_config_shadow[INT_PATA][0][0xF] = 0x0103;

		pci_config_mask[INT_PATA][0][0] = 0x0;
		pci_config_mask[INT_PATA][0][1] = 0x0;
		pci_config_mask[INT_PATA][0][2] = 0x0;
		pci_config_mask[INT_PATA][0][3] = 0x0;

		pci_config_mask[INT_PATA][0][4] = 0xFFFFFFF8;
		pci_config_mask[INT_PATA][0][5] = 0xFFFFFFFC;
		pci_config_mask[INT_PATA][0][6] = 0x0;
		pci_config_mask[INT_PATA][0][7] = 0x0;
		pci_config_mask[INT_PATA][0][8] = 0xFFFFFFF0;
		pci_config_mask[INT_PATA][0][9] = 0x0;

		pci_config_mask[INT_PATA][0][0xA] = 0x0;
		pci_config_mask[INT_PATA][0][0xB] = 0x0;
		pci_config_mask[INT_PATA][0][0xC] = 0x0;
		pci_config_mask[INT_PATA][0][0xD] = 0x0;
		pci_config_mask[INT_PATA][0][0xE] = 0x0;
		pci_config_mask[INT_PATA][0][0xF] = 0x0;

		pci_config_ro[INT_PATA][0][0] = 0x13591106;
		pci_config_ro[INT_PATA][0][1] = 0x02000005;
		pci_config_ro[INT_PATA][0][2] = 0x01018A00;
		pci_config_ro[INT_PATA][0][3] = 0x00002000;

		pci_config_ro[INT_PATA][0][4] = 0x1;
		pci_config_ro[INT_PATA][0][5] = 0x1;
		pci_config_ro[INT_PATA][0][6] = 0x0;
		pci_config_ro[INT_PATA][0][7] = 0x0;
		pci_config_ro[INT_PATA][0][8] = 0x1;
		pci_config_ro[INT_PATA][0][9] = 0x0;

		pci_config_ro[INT_PATA][0][0xA] = 0x0;
		pci_config_ro[INT_PATA][0][0xB] = 0x13581106;/* 0x05811106; */
		pci_config_ro[INT_PATA][0][0xC] = 0x0;
		pci_config_ro[INT_PATA][0][0xD] = 0x0;
		pci_config_ro[INT_PATA][0][0xE] = 0x0;
		pci_config_ro[INT_PATA][0][0xF] = 0x0103;
	}
}
#endif

#ifdef SATA
void init_int_sata(void)
{
		pci_config_shadow[INT_SATA][0][0] = 0x23591106;
		pci_config_shadow[INT_SATA][0][1] = 0x02900007;
		pci_config_shadow[INT_SATA][0][2] = 0x01018f00;
		pci_config_shadow[INT_SATA][0][3] = 0x00001000;

		//pci_config_shadow[INT_SATA][0][4] = 0xd800d2f1;
		pci_config_shadow[INT_SATA][0][4] = 0xFE00d2f1;
		//pci_config_shadow[INT_SATA][0][5] = 0xd800d3f5;
		pci_config_shadow[INT_SATA][0][5] = 0xFE00d3f5;
		pci_config_shadow[INT_SATA][0][6] = 0x0;
		pci_config_shadow[INT_SATA][0][7] = 0x0;
		//pci_config_shadow[INT_SATA][0][8] = 0xd800d401;
		pci_config_shadow[INT_SATA][0][8] = 0xFE00d401;
		pci_config_shadow[INT_SATA][0][9] = 0x0;

		pci_config_shadow[INT_SATA][0][0xA] = 0x0;
		pci_config_shadow[INT_SATA][0][0xB] = 0x23591106;
		pci_config_shadow[INT_SATA][0][0xC] = 0x0;
		pci_config_shadow[INT_SATA][0][0xD] = 0x0;
		pci_config_shadow[INT_SATA][0][0xE] = 0x0;
		pci_config_shadow[INT_SATA][0][0xF] = 0x0104;

		pci_config_mask[INT_SATA][0][0] = 0x0;
		pci_config_mask[INT_SATA][0][1] = 0x00000477;
		pci_config_mask[INT_SATA][0][2] = 0x500;
		pci_config_mask[INT_SATA][0][3] = 0x0000F000;

		pci_config_mask[INT_SATA][0][4] = 0xFFFFFFF8;
		pci_config_mask[INT_SATA][0][5] = 0xFFFFFFFC;
		pci_config_mask[INT_SATA][0][6] = 0x0;
		pci_config_mask[INT_SATA][0][7] = 0x0;
		pci_config_mask[INT_SATA][0][8] = 0xFFFFFFF0;
		pci_config_mask[INT_SATA][0][9] = 0x0;

		pci_config_mask[INT_SATA][0][0xA] = 0x0;
		pci_config_mask[INT_SATA][0][0xB] = 0x0;
		pci_config_mask[INT_SATA][0][0xC] = 0x0;
		pci_config_mask[INT_SATA][0][0xD] = 0x0;
		pci_config_mask[INT_SATA][0][0xE] = 0x0;
		pci_config_mask[INT_SATA][0][0xF] = 0x0;

		pci_config_ro[INT_SATA][0][0] = 0x23591106;
		pci_config_ro[INT_SATA][0][1] = 0x02900000;
		pci_config_ro[INT_SATA][0][2] = 0x01018A00;
		pci_config_ro[INT_SATA][0][3] = 0x0;

		pci_config_ro[INT_SATA][0][4] = 0x1;
		pci_config_ro[INT_SATA][0][5] = 0x1;
		pci_config_ro[INT_SATA][0][6] = 0x0;
		pci_config_ro[INT_SATA][0][7] = 0x0;
		pci_config_ro[INT_SATA][0][8] = 0x1;
		pci_config_ro[INT_SATA][0][9] = 0x0;

		pci_config_ro[INT_SATA][0][0xA] = 0x0;
		pci_config_ro[INT_SATA][0][0xB] = 0x23591106;
		pci_config_ro[INT_SATA][0][0xC] = 0x0;
		pci_config_ro[INT_SATA][0][0xD] = 0x0;
		pci_config_ro[INT_SATA][0][0xE] = 0x0;
		pci_config_ro[INT_SATA][0][0xF] = 0x0104;
}
#endif

#ifdef USB_HOST
void init_int_usb(void)
{
		/* EHCI */
		pci_config_shadow[INT_USB_EHCI][0][0] = 0x31041106;
		pci_config_shadow[INT_USB_EHCI][0][1] = 0x02100000;
		pci_config_shadow[INT_USB_EHCI][0][2] = 0x0C032090;
		pci_config_shadow[INT_USB_EHCI][0][3] = 0x00801600;

		pci_config_shadow[INT_USB_EHCI][0][4] = 0xD8007900; /*  phy 0xD8007900; */
		pci_config_shadow[INT_USB_EHCI][0][5] = 0x00000000;
		pci_config_shadow[INT_USB_EHCI][0][6] = 0x0;
		pci_config_shadow[INT_USB_EHCI][0][7] = 0x0;
		pci_config_shadow[INT_USB_EHCI][0][8] = 0x00000000;
		pci_config_shadow[INT_USB_EHCI][0][9] = 0x0;

		pci_config_shadow[INT_USB_EHCI][0][0xA] = 0x0;
		pci_config_shadow[INT_USB_EHCI][0][0xB] = 0x31041106;
		pci_config_shadow[INT_USB_EHCI][0][0xC] = 0x0;
		pci_config_shadow[INT_USB_EHCI][0][0xD] = 0x0;
		pci_config_shadow[INT_USB_EHCI][0][0xE] = 0x0;
		pci_config_shadow[INT_USB_EHCI][0][0xF] = 0x041A;  /*  0x041A; for WM3445   */

		pci_config_mask[INT_USB_EHCI][0][0] = 0x0;
		pci_config_mask[INT_USB_EHCI][0][1] = 0x00000477;
		pci_config_mask[INT_USB_EHCI][0][2] = 0x00000000;
		pci_config_mask[INT_USB_EHCI][0][3] = 0x0000FFFF;

		pci_config_mask[INT_USB_EHCI][0][4] = 0xFFFFFF00;
		pci_config_mask[INT_USB_EHCI][0][5] = 0x0;
		pci_config_mask[INT_USB_EHCI][0][6] = 0x0;
		pci_config_mask[INT_USB_EHCI][0][7] = 0x0;
		pci_config_mask[INT_USB_EHCI][0][8] = 0x0;
		pci_config_mask[INT_USB_EHCI][0][9] = 0x0;

		pci_config_mask[INT_USB_EHCI][0][0xA] = 0x0;
		pci_config_mask[INT_USB_EHCI][0][0xB] = 0x0;
		pci_config_mask[INT_USB_EHCI][0][0xC] = 0x0;
		pci_config_mask[INT_USB_EHCI][0][0xD] = 0x0;
		pci_config_mask[INT_USB_EHCI][0][0xE] = 0x0;
		pci_config_mask[INT_USB_EHCI][0][0xF] = 0xFF;

		pci_config_ro[INT_USB_EHCI][0][0] = 0x31041106;
		pci_config_ro[INT_USB_EHCI][0][1] = 0x02100000;
		pci_config_ro[INT_USB_EHCI][0][2] = 0x0C032090;
		pci_config_ro[INT_USB_EHCI][0][3] = 0x00800000;

		pci_config_ro[INT_USB_EHCI][0][4] = 0x0;
		pci_config_ro[INT_USB_EHCI][0][5] = 0x0;
		pci_config_ro[INT_USB_EHCI][0][6] = 0x0;
		pci_config_ro[INT_USB_EHCI][0][7] = 0x0;
		pci_config_ro[INT_USB_EHCI][0][8] = 0x0;
		pci_config_ro[INT_USB_EHCI][0][9] = 0x0;

		pci_config_ro[INT_USB_EHCI][0][0xA] = 0x0;
		pci_config_ro[INT_USB_EHCI][0][0xB] = 0x31041106;
		pci_config_ro[INT_USB_EHCI][0][0xC] = 0x0;
		pci_config_ro[INT_USB_EHCI][0][0xD] = 0x0;
		pci_config_ro[INT_USB_EHCI][0][0xE] = 0x0;
		pci_config_ro[INT_USB_EHCI][0][0xF] = 0x100;

		/* UHCI */
		pci_config_shadow[INT_USB_UHCI][0][0] = 0x30381106;
		pci_config_shadow[INT_USB_UHCI][0][1] = 0x02100000;
		pci_config_shadow[INT_USB_UHCI][0][2] = 0x0C030090;
		pci_config_shadow[INT_USB_UHCI][0][3] = 0x00801600;

		pci_config_shadow[INT_USB_UHCI][0][4] = 0x00000000;
		pci_config_shadow[INT_USB_UHCI][0][5] = 0x00000000;
		pci_config_shadow[INT_USB_UHCI][0][6] = 0x0;
		pci_config_shadow[INT_USB_UHCI][0][7] = 0x0;
		//pci_config_shadow[INT_USB_UHCI][0][8] = 0xD8007B01; /*  0xD8007B01;  */
		pci_config_shadow[INT_USB_UHCI][0][8] = 0xFE007B01; 	/*  0xFE007B01;  */
		pci_config_shadow[INT_USB_UHCI][0][9] = 0x0;

		pci_config_shadow[INT_USB_UHCI][0][0xA] = 0x0;
		pci_config_shadow[INT_USB_UHCI][0][0xB] = 0x30381106;
		pci_config_shadow[INT_USB_UHCI][0][0xC] = 0x0;
		pci_config_shadow[INT_USB_UHCI][0][0xD] = 0x0;
		pci_config_shadow[INT_USB_UHCI][0][0xE] = 0x0;
		pci_config_shadow[INT_USB_UHCI][0][0xF] = 0x011A;   /*  0x01A; for WM3445 uhci */

		pci_config_mask[INT_USB_UHCI][0][0] = 0x0;
		pci_config_mask[INT_USB_UHCI][0][1] = 0x00000417;
		pci_config_mask[INT_USB_UHCI][0][2] = 0x00000000;
		pci_config_mask[INT_USB_UHCI][0][3] = 0x0000FFFF;

		pci_config_mask[INT_USB_UHCI][0][4] = 0x00000000;
		pci_config_mask[INT_USB_UHCI][0][5] = 0x00000000;
		pci_config_mask[INT_USB_UHCI][0][6] = 0x0;
		pci_config_mask[INT_USB_UHCI][0][7] = 0x0;
		pci_config_mask[INT_USB_UHCI][0][8] = 0xFFFFFFE0;
		pci_config_mask[INT_USB_UHCI][0][9] = 0x0;

		pci_config_mask[INT_USB_UHCI][0][0xA] = 0x0;
		pci_config_mask[INT_USB_UHCI][0][0xB] = 0x0;
		pci_config_mask[INT_USB_UHCI][0][0xC] = 0x0;
		pci_config_mask[INT_USB_UHCI][0][0xD] = 0x0;
		pci_config_mask[INT_USB_UHCI][0][0xE] = 0x0;
		pci_config_mask[INT_USB_UHCI][0][0xF] = 0x000000FF;

		pci_config_ro[INT_USB_UHCI][0][0] = 0x30381106;
		pci_config_ro[INT_USB_UHCI][0][1] = 0x02100000;
		pci_config_ro[INT_USB_UHCI][0][2] = 0x0C030090;
		pci_config_ro[INT_USB_UHCI][0][3] = 0x00800000;

		pci_config_ro[INT_USB_UHCI][0][4] = 0x00000000;
		pci_config_ro[INT_USB_UHCI][0][5] = 0x00000000;
		pci_config_ro[INT_USB_UHCI][0][6] = 0x0;
		pci_config_ro[INT_USB_UHCI][0][7] = 0x0;
		pci_config_ro[INT_USB_UHCI][0][8] = 0x1;
		pci_config_ro[INT_USB_UHCI][0][9] = 0x0;

		pci_config_ro[INT_USB_UHCI][0][0xA] = 0x0;
		pci_config_ro[INT_USB_UHCI][0][0xB] = 0x30381106;
		pci_config_ro[INT_USB_UHCI][0][0xC] = 0x0;
		pci_config_ro[INT_USB_UHCI][0][0xD] = 0x0;
		pci_config_ro[INT_USB_UHCI][0][0xE] = 0x0;
		pci_config_ro[INT_USB_UHCI][0][0xF] = 0x0100;

		/* UHCI2 */
		pci_config_shadow[INT_USB_UHCI2][0][0] = 0x30381106;
		pci_config_shadow[INT_USB_UHCI2][0][1] = 0x02100000;
		pci_config_shadow[INT_USB_UHCI2][0][2] = 0x0C030090;
		pci_config_shadow[INT_USB_UHCI2][0][3] = 0x00801600;

		pci_config_shadow[INT_USB_UHCI2][0][4] = 0x00000000;
		pci_config_shadow[INT_USB_UHCI2][0][5] = 0x00000000;
		pci_config_shadow[INT_USB_UHCI2][0][6] = 0x0;
		pci_config_shadow[INT_USB_UHCI2][0][7] = 0x0;
		//pci_config_shadow[INT_USB_UHCI2][0][8] = 0xD8008D01; /* phy 0xD8008D01;  */
		pci_config_shadow[INT_USB_UHCI2][0][8] = 0xFE008D01; /*  vertual 0xFE008D01;  */
		pci_config_shadow[INT_USB_UHCI2][0][9] = 0x0;

		pci_config_shadow[INT_USB_UHCI2][0][0xA] = 0x0;
		pci_config_shadow[INT_USB_UHCI2][0][0xB] = 0x30381106;
		pci_config_shadow[INT_USB_UHCI2][0][0xC] = 0x0;
		pci_config_shadow[INT_USB_UHCI2][0][0xD] = 0x0;
		pci_config_shadow[INT_USB_UHCI2][0][0xE] = 0x0;
		pci_config_shadow[INT_USB_UHCI2][0][0xF] = 0x011A;   /*  0x011A; for WM3445 uhci */

		pci_config_mask[INT_USB_UHCI2][0][0] = 0x0;
		pci_config_mask[INT_USB_UHCI2][0][1] = 0x00000417;
		pci_config_mask[INT_USB_UHCI2][0][2] = 0x00000000;
		pci_config_mask[INT_USB_UHCI2][0][3] = 0x0000FFFF;

		pci_config_mask[INT_USB_UHCI2][0][4] = 0x00000000;
		pci_config_mask[INT_USB_UHCI2][0][5] = 0x00000000;
		pci_config_mask[INT_USB_UHCI2][0][6] = 0x0;
		pci_config_mask[INT_USB_UHCI2][0][7] = 0x0;
		pci_config_mask[INT_USB_UHCI2][0][8] = 0xFFFFFFE0;
		pci_config_mask[INT_USB_UHCI2][0][9] = 0x0;

		pci_config_mask[INT_USB_UHCI2][0][0xA] = 0x0;
		pci_config_mask[INT_USB_UHCI2][0][0xB] = 0x0;
		pci_config_mask[INT_USB_UHCI2][0][0xC] = 0x0;
		pci_config_mask[INT_USB_UHCI2][0][0xD] = 0x0;
		pci_config_mask[INT_USB_UHCI2][0][0xE] = 0x0;
		pci_config_mask[INT_USB_UHCI2][0][0xF] = 0x000000FF;

		pci_config_ro[INT_USB_UHCI2][0][0] = 0x30381106;
		pci_config_ro[INT_USB_UHCI2][0][1] = 0x02100000;
		pci_config_ro[INT_USB_UHCI2][0][2] = 0x0C030090;
		pci_config_ro[INT_USB_UHCI2][0][3] = 0x00800000;

		pci_config_ro[INT_USB_UHCI2][0][4] = 0x00000000;
		pci_config_ro[INT_USB_UHCI2][0][5] = 0x00000000;
		pci_config_ro[INT_USB_UHCI2][0][6] = 0x0;
		pci_config_ro[INT_USB_UHCI2][0][7] = 0x0;
		pci_config_ro[INT_USB_UHCI2][0][8] = 0x1;
		pci_config_ro[INT_USB_UHCI2][0][9] = 0x0;

		pci_config_ro[INT_USB_UHCI2][0][0xA] = 0x0;
		pci_config_ro[INT_USB_UHCI2][0][0xB] = 0x30381106;
		pci_config_ro[INT_USB_UHCI2][0][0xC] = 0x0;
		pci_config_ro[INT_USB_UHCI2][0][0xD] = 0x0;
		pci_config_ro[INT_USB_UHCI2][0][0xE] = 0x0;
		pci_config_ro[INT_USB_UHCI2][0][0xF] = 0x0100;

}
#endif

#ifdef MAC
void init_int_mac(void)
{
		pci_config_shadow[INT_MAC0][0][0] = 0x31191106;
		pci_config_shadow[INT_MAC0][0][1] = 0x02100017;
		pci_config_shadow[INT_MAC0][0][2] = 0x02000084;
		pci_config_shadow[INT_MAC0][0][3] = 0x00004004;

		//pci_config_shadow[INT_MAC0][0][4] = 0xD8004001;
		pci_config_shadow[INT_MAC0][0][4] = 0xFE004001;
		pci_config_shadow[INT_MAC0][0][5] = 0xD8004000;
		pci_config_shadow[INT_MAC0][0][6] = 0x0;
		pci_config_shadow[INT_MAC0][0][7] = 0x0;
		pci_config_shadow[INT_MAC0][0][8] = 0x0;
		pci_config_shadow[INT_MAC0][0][9] = 0x0;

		pci_config_shadow[INT_MAC0][0][0xA] = 0x0;
		pci_config_shadow[INT_MAC0][0][0xB] = 0x01061106;
		pci_config_shadow[INT_MAC0][0][0xC] = 0x0;
		pci_config_shadow[INT_MAC0][0][0xD] = 0x0;
		pci_config_shadow[INT_MAC0][0][0xE] = 0x0;
		pci_config_shadow[INT_MAC0][0][0xF] = 0x010A;

		pci_config_mask[INT_MAC0][0][0] = 0x0;
		pci_config_mask[INT_MAC0][0][1] = 0x000003D7;
		pci_config_mask[INT_MAC0][0][2] = 0x0;
		pci_config_mask[INT_MAC0][0][3] = 0x0000F8FF;

		pci_config_mask[INT_MAC0][0][4] = 0xFFFFFF00;
		pci_config_mask[INT_MAC0][0][5] = 0xFFFFFF00;
		pci_config_mask[INT_MAC0][0][6] = 0x0;
		pci_config_mask[INT_MAC0][0][7] = 0x0;
		pci_config_mask[INT_MAC0][0][8] = 0x0;
		pci_config_mask[INT_MAC0][0][9] = 0x0;

		pci_config_mask[INT_MAC0][0][0xA] = 0x0;
		pci_config_mask[INT_MAC0][0][0xB] = 0x0;
		pci_config_mask[INT_MAC0][0][0xC] = 0x0;
		pci_config_mask[INT_MAC0][0][0xD] = 0x0;
		pci_config_mask[INT_MAC0][0][0xE] = 0x0;
		pci_config_mask[INT_MAC0][0][0xF] = 0xFF;

		pci_config_ro[INT_MAC0][0][0] = 0x31191106;
		pci_config_ro[INT_MAC0][0][1] = 0x02100000;
		pci_config_ro[INT_MAC0][0][2] = 0x0;
		pci_config_ro[INT_MAC0][0][3] = 0x0;

		pci_config_ro[INT_MAC0][0][4] = 0x1;
		pci_config_ro[INT_MAC0][0][5] = 0x0;
		pci_config_ro[INT_MAC0][0][6] = 0x0;
		pci_config_ro[INT_MAC0][0][7] = 0x0;
		pci_config_ro[INT_MAC0][0][8] = 0x0;
		pci_config_ro[INT_MAC0][0][9] = 0x0;

		pci_config_ro[INT_MAC0][0][0xA] = 0x0;
		pci_config_ro[INT_MAC0][0][0xB] = 0x01061106;
		pci_config_ro[INT_MAC0][0][0xC] = 0x0;
		pci_config_ro[INT_MAC0][0][0xD] = 0x0;
		pci_config_ro[INT_MAC0][0][0xE] = 0x0;
		pci_config_ro[INT_MAC0][0][0xF] = 0x10;

		if (0) {/* !ARCH_VT8430) */
			/* MAC1 */
			pci_config_shadow[INT_MAC1][0][0] = 0x31061106;
			pci_config_shadow[INT_MAC1][0][1] = 0x02100017;
			pci_config_shadow[INT_MAC1][0][2] = 0x02000084;
			pci_config_shadow[INT_MAC1][0][3] = 0x00004004;

			//pci_config_shadow[INT_MAC1][0][4] = 0xD8005001;
			pci_config_shadow[INT_MAC1][0][4] = 0xFE005001;
			pci_config_shadow[INT_MAC1][0][5] = 0xD8005000;
			pci_config_shadow[INT_MAC1][0][6] = 0x0;
			pci_config_shadow[INT_MAC1][0][7] = 0x0;
			pci_config_shadow[INT_MAC1][0][8] = 0x0;
			pci_config_shadow[INT_MAC1][0][9] = 0x0;

			pci_config_shadow[INT_MAC1][0][0xA] = 0x0;
			pci_config_shadow[INT_MAC1][0][0xB] = 0x01061106;
			pci_config_shadow[INT_MAC1][0][0xC] = 0x0;
			pci_config_shadow[INT_MAC1][0][0xD] = 0x0;
			pci_config_shadow[INT_MAC1][0][0xE] = 0x0;
			pci_config_shadow[INT_MAC1][0][0xF] = 0x01011;

			pci_config_mask[INT_MAC1][0][0] = 0x0;
			pci_config_mask[INT_MAC1][0][1] = 0x000003D7;
			pci_config_mask[INT_MAC1][0][2] = 0x0;
			pci_config_mask[INT_MAC1][0][3] = 0x0000F8FF;

			pci_config_mask[INT_MAC1][0][4] = 0xFFFFFF00;
			pci_config_mask[INT_MAC1][0][5] = 0xFFFFFF00;
			pci_config_mask[INT_MAC1][0][6] = 0x0;
			pci_config_mask[INT_MAC1][0][7] = 0x0;
			pci_config_mask[INT_MAC1][0][8] = 0x0;
			pci_config_mask[INT_MAC1][0][9] = 0x0;

			pci_config_mask[INT_MAC1][0][0xA] = 0x0;
			pci_config_mask[INT_MAC1][0][0xB] = 0x0;
			pci_config_mask[INT_MAC1][0][0xC] = 0x0;
			pci_config_mask[INT_MAC1][0][0xD] = 0x0;
			pci_config_mask[INT_MAC1][0][0xE] = 0x0;
			pci_config_mask[INT_MAC1][0][0xF] = 0xFF;

			pci_config_ro[INT_MAC1][0][0] = 0x31061106;
			pci_config_ro[INT_MAC1][0][1] = 0x02100000;
			pci_config_ro[INT_MAC1][0][2] = 0x0;
			pci_config_ro[INT_MAC1][0][3] = 0x0;

			pci_config_ro[INT_MAC1][0][4] = 0x1;
			pci_config_ro[INT_MAC1][0][5] = 0x0;
			pci_config_ro[INT_MAC1][0][6] = 0x0;
			pci_config_ro[INT_MAC1][0][7] = 0x0;
			pci_config_ro[INT_MAC1][0][8] = 0x0;
			pci_config_ro[INT_MAC1][0][9] = 0x0;

			pci_config_ro[INT_MAC1][0][0xA] = 0x0;
			pci_config_ro[INT_MAC1][0][0xB] = 0x01061106;
			pci_config_ro[INT_MAC1][0][0xC] = 0x0;
			pci_config_ro[INT_MAC1][0][0xD] = 0x0;
			pci_config_ro[INT_MAC1][0][0xE] = 0x0;
			pci_config_ro[INT_MAC1][0][0xF] = 0x10;
		}
}
#endif

#ifdef EXT_PCI
void init_ext_pci(void)
{
	int i, func, bar, size;
	u32 val, ori_val, new_val;
	/* windows size is 64KB */
	u32 io_base = 0x10000;
	/* windows size is 16MB, but according to the architecture spec,
		 the windows is 64MB. */
	u32 mem_base = 0xC2000000;

	/* assign resource for I/O, Memory and IRQ */
	for (i = EXT_PCI7; i <= EXT_PCIB; i++) {
		for (func = 0; func < 8; func++) {
			val = PCI_GetConfigRegisterDWORD(0, i, func, 0);
			if ((val != 0xFFFFFFFF) && (val != 0x0)) {
				/* PCI_INTA connect to IRQ44 */
				/* PCI_INTB connect to IRQ45 */
				/* PCI_INTC connect to IRQ46 */
				/* PCI_INTD connect to IRQ47 */
				u8 pci_int[4] = {44, 45, 46, 47};
				/* Dev8 PCI_INTA->PCI_INTA,
					      PCI_INTB->PCI_INTB,
					      PCI_INTC->PCI_INTC,
					      PCI_INTD->PCI_INTD */
				/* Dev9 PCI_INTA->PCI_INTB, ... */
				/* DevA PCI_INTA->PCI_INTC, ... */
				/* DevB PCI_INTA->PCI_INTD, ... */
				u8 pci_int_rout_table[4][4] =
				{ {0, 1, 2, 3}, {1, 2, 3, 0}, {2, 3, 0, 1}, {3, 0, 1, 2} };
				/* IRQ routing */
				/* Dev8 Dev9 DevA DevB */
				/* A    B    C    D */
				/* B    C    D    A */
				/* C    D    A    B */
				/* D    A    B    C */
				u8 irq_pin, irq_line;

				printk("pci vid&pid = 0x%08X\n", val);
				val = PCI_GetConfigRegisterDWORD(0, i, func, 0x3C);
				irq_pin = (val & 0xFF00)>>8;
				if (irq_pin) {
					/* irq_pin = 1->PCI_INTA according to the PCI spec */
					irq_pin--;
					#ifdef CONFIG_PCI_DEBUG
					printk("irq_pin:0x%02X\n", irq_pin);
					#endif /* CONFIG_PCI_DEBUG */
					irq_line = pci_int[pci_int_rout_table[(i-8)&3][irq_pin]];
					#ifdef CONFIG_PCI_DEBUG
					printk("irq_line:0x%02X\n", irq_line);
					#endif /* CONFIG_PCI_DEBUG */
					val = (val & 0xFFFFFF00) | irq_line;
					PCI_SetConfigRegisterDWORD(0, i, func, 0x3C, val);
					val = PCI_GetConfigRegisterDWORD(0, i, func, 0x3C);
					#ifdef CONFIG_PCI_DEBUG
					printk("[0x3C = 0x%08X]\n", val);
					#endif /* CONFIG_PCI_DEBUG */
				}

				for (bar = 0; bar < 6; bar++) {
					PCI_SetConfigRegisterDWORD(0, i, func, 0x10+bar*4, 0xFFFFFFFF);
					new_val = PCI_GetConfigRegisterDWORD(0, i, func, 0x10+bar*4);
					#ifdef CONFIG_PCI_DEBUG
					printk("[bar:%d] new val:0x%08X\n", bar, new_val);
					#endif /* CONFIG_PCI_DEBUG */
					if (new_val == 0)
							continue;

					if (new_val & 1) {   /* IO Space */
						size = new_val & ~0x1;
						size = ~size + 1;
						io_base -= size;
						io_base = io_base & ~(size-1);
						PCI_SetConfigRegisterDWORD(0, i, func, 0x10+bar*4, io_base);
						#ifdef CONFIG_PCI_DEBUG
						printk("io_base:0x%08X\n", io_base);
						#endif /* CONFIG_PCI_DEBUG */
						continue;
					} else { /* Memory Space */
						size = new_val & ~0xF;
						size = ~size + 1;
						mem_base -= size;
						mem_base = mem_base & ~(size-1);
						PCI_SetConfigRegisterDWORD(0, i, func, 0x10+bar*4, mem_base);
						#ifdef CONFIG_PCI_DEBUG
						printk("mem_base:0x%08X\n", mem_base);
						#endif /* CONFIG_PCI_DEBUG */
						continue;
					}
				}
			}
		}
	}

	for (i = EXT_PCI7; i <= EXT_PCIB; i++) {
		for (func = 0; func < 8; func++) {
			val = PCI_GetConfigRegisterDWORD(0, i, func, 0);
			if (val != 0xFFFFFFFF) {
				for (bar = 0; bar < 6; bar++) {
					ori_val = PCI_GetConfigRegisterDWORD(0, i, func, 0x10+bar*4);
					PCI_SetConfigRegisterDWORD(0, i, func, 0x10+bar*4, 0xFFFFFFFF);
					new_val = PCI_GetConfigRegisterDWORD(0, i, func, 0x10+bar*4);
					if (new_val == 0)
						continue;
					pci_config_mask[i][func][bar+4] = new_val;
					if (new_val & 1) { /* IO Space */
						pci_config_shadow[i][func][bar+4] =
						(0xC0000000 + ori_val) & ~1;
						size = new_val & ~1;
						size = ~size + 1;
						#ifdef CONFIG_PCI_DEBUG
						printk("(P)pci_config_shadow[i][func][bar]:0x%08X\n",
						pci_config_shadow[i][func][bar+4]);
						#endif /* CONFIG_PCI_DEBUG */
						pci_config_shadow[i][func][bar+4] = (ulong)ioremap_nocache(
						pci_config_shadow[i][func][bar+4], size);
						/* #ifdef CONFIG_PCI_DEBUG */
						/* PCI_SetConfigRegisterDWORD(0, i, 0, 4, 0x17); */
						/* printk("io:addr:0x%08X=0x%08X\n",
						   pci_config_shadow[i][bar+4],
						   *((ulong *)(pci_config_shadow[i][bar+4]))); */
						/* #endif //CONFIG_PCI_DEBUG */
						pci_config_ro[i][func][bar+4] = 1;
						pci_config_shadow[i][func][bar+4] |=
							pci_config_ro[i][func][bar+4];
					} else { /* Memory Space */
						pci_config_shadow[i][func][bar+4] = ori_val & ~0xF;
						size = new_val & ~0xF;
						size = ~size + 1;
						/* #ifdef CONFIG_PCI_DEBUG */
						/* printk("(p)pci_config_shadow[i][bar]:0x%08X\n",
						   pci_config_shadow[i][bar+4]); */
						/* printk("(v)pci_config_shadow[i][bar]:0x%08X\n",
						   ioremap_nocache(pci_config_shadow[i][bar+4], size)); */
						/* PCI_SetConfigRegisterDWORD(0, i, 0, 4, 0x17); */
						/* #endif //CONFIG_PCI_DEBUG */
						/* pci_config_shadow[i][bar+4] = (ulong)ioremap_nocache(
						   pci_config_shadow[i][bar+4], size); */
						pci_config_ro[i][func][bar+4] = new_val & 0xF;
						pci_config_shadow[i][func][bar+4] |=
							pci_config_ro[i][func][bar+4];
					}
					PCI_SetConfigRegisterDWORD(0, i, func, 0x10+bar*4, ori_val);
					#ifdef CONFIG_PCI_DEBUG
					printk("JHT [Dev:0x%0X] [Func:0x%0X] [Bar:0x%02X]:", i, func,bar);
					printk("(ori}0x%08X (new)0x%08X (shadow)0x%08X size 0x%04X\n"
					, ori_val, new_val, pci_config_shadow[i][bar+4], size);
					#endif /* CONFIG_PCI_DEBUG */
				}
			}
		}
	}
	/*
	 * PCI Bridge Memory Map is between 0xC0000:0000 - 0xC3FF:FFFF(64MB)
	 * The first 64KB is allocated for the PCI I/O Space, except for the
	 * 0xCF8 - 0xCFF(8Bytes) for the PCI Configuration
	 * Others are reserved for the MemorySpace.
	 */
	if (!request_region(0xC0000CF8, 8, "pci config")) {
		printk("WonderMidia Technology PCI: Unable to request region 0xCF8\n");
		return;
	}
}
#endif

/* void __init wmt_pci_preinit(void *sysdata) */
void __init wmt_pci_preinit(void)
{
	int i, j, bar;

	printk("PCI: WonderMidia Technology PCI Bridge\n");

	if (!pci_config_ba) {
		pci_config_ba = (ulong)ioremap_nocache(0xC0000CF8, 8);
		pci_config_addr = pci_config_ba;
		pci_config_data = pci_config_ba+4;
	}

	for (i = 0; i < MAX_PCI_DEV; i++) {
		for (j = 0; j < 8; j++) {
			for (bar = 0; bar < 0x10; bar++) {
				pci_config_shadow[i][j][bar] = 0xFFFFFFFF;
				pci_config_mask[i][j][bar] = 0;
				pci_config_ro[i][j][bar] = 0;
			}
		}
	}

#ifdef PATA
	init_int_pata();
#endif

#ifdef SATA
	init_int_sata();	
#endif

#ifdef USB_HOST
	init_int_usb();
#endif

#ifdef MAC
	init_int_mac();
#endif

#ifdef EXT_PCI
	init_ext_pci();
#endif

}

int __init wmt_pci_setup(int nr, struct pci_sys_data *sys)
{
	return (nr == 0);
}

struct pci_bus * __init wmt_pci_scan_bus(int nr, struct pci_sys_data *sysdata)
{
	if (nr == 0)
		return pci_scan_bus(0, &wmt_pci_ops, sysdata);

	return NULL;
}

/*
 *	[Description]
 *      Get the PCI Config Register for the specific PCI bus number,
 *      device number and function number in DWORD.
 *
 *  [Arguments]
 *      bus    : The target device's bus number.
 *      device : The target device's device number.
 *      funcn  : The target device's function number.
 *      target : The target device's PCI config register Offset.
 *
 *  [Return]
 *      The target device pci config register value will be returned.
 */
ulong
PCI_GetConfigRegisterDWORD(
	int bus,
	int device,
	int fctn,
	int target
	)
{
	outl(CONFIG_CMD(0, (device << 3) | fctn, target), pci_config_addr);
	return inl(pci_config_data);
}

/*
 *	[Description]
 *      Set the PCI Config Register for the specific PCI bus number,
 *      device number and function number in DWORD.
 *
 *  [Arguments]
 *      bus    : The target device's bus number.
 *      device : The target device's device number.
 *      funcn  : The target device's function number.
 *      target : The target device's PCI config register Offset.
 *      data   : The written data to the target PCI device.
 *
 *  [Return]
 *	    Return value:  1 if found, 0 not found
 */
void
PCI_SetConfigRegisterDWORD(
	int bus,
	int device,
	int fctn,
	int target,
	ulong data
	)
{
	outl(CONFIG_CMD(0, (device << 3) | fctn, target), pci_config_addr);
	outl(data, pci_config_data);
}
