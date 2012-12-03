/*
 *  linux/arch/arm/common/pci.c
 *
 *	Some descriptions of such software. Copyright (c) 2008 WonderMedia Technologies, Inc.
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
 *
 *  PCI bios-type initialisation for PCI machines
 *
 *  Bits taken from various places.
 */

#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/init.h>

#include <asm/irq.h>
#include <asm/mach/pci.h>
#include <asm/mach-types.h>

static int __init wmt_map_irq(struct pci_dev *dev, u8 slot, u8 pin)
{
    u8 int_line;

    pci_read_config_byte(dev, PCI_INTERRUPT_LINE, &int_line);
    return int_line;
}

extern void __init wmt_pci_preinit(void *sysdata);

static struct hw_pci wmt_pci __initdata = {
	.setup	    = wmt_pci_setup,
	.swizzle    = pci_std_swizzle,
	.map_irq    = wmt_map_irq,
	.nr_controllers = 1,
	.scan       = wmt_pci_scan_bus,
	.preinit    = wmt_pci_preinit
};

static int __init wmt_pci_init(void)
{
	/* {JHT} */
	printk("wmt_pci_init\n");
		pci_common_init(&wmt_pci);
	return 0;
}

subsys_initcall(wmt_pci_init);
