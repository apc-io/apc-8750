/*++
linux/arch/arm/mach-wmt/board.c

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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/cpufreq.h>
#include <linux/ioport.h>
#include <linux/tty.h>
#include <linux/mm.h>
#include <linux/errno.h>
#include <linux/serial_core.h>
#include <linux/delay.h>

#include <mach/hardware.h>
#include <asm/system.h>
#include <asm/mach-types.h>
#include <asm/irq.h>
#include <asm/setup.h>
#include <asm/page.h>
#include <asm/pgtable.h>
#include <asm/tlbflush.h>
#include <asm/sizes.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/serial_wmt.h>

#include "generic.h"

static void __init
wmt_fixup(struct machine_desc *desc, struct tag *tags,
					char **cmdline, struct meminfo *mi)
{
#ifdef CONFIG_WMT_FIXUP_ATAG

	struct tag *t = tags;

	/*FIXME, Harry, remove following while ATAG passing from bootloader is ok.*/
	t->hdr.tag              = ATAG_CORE;
	t->hdr.size             = tag_size(tag_core);
	t->u.core.flags         = 0;
	t->u.core.pagesize      = PAGE_SIZE;
	t->u.core.rootdev       = (RAMDISK_MAJOR << 8) | 0;
	t = tag_next(t);

	t->hdr.tag              = ATAG_MEM;
	t->hdr.size             = tag_size(tag_mem32);
	t->u.mem.start          = 0x00000000;
	t->u.mem.size           = 64 * 1024 * 1024;
	t = tag_next(t);
	/**/
	/* ramdisk.size = decompressed ramdisk size in _kilo_ bytes.*/
	/**/
	t->hdr.tag              = ATAG_RAMDISK;
	t->hdr.size             = tag_size(tag_ramdisk);
	t->u.ramdisk.flags      = 1;
	t->u.ramdisk.size       = 8 * 1024;
	t->u.ramdisk.start      = 0;
	t = tag_next(t);
	/**/
	/* initrd.size = size of compressed ramdisk image in bytes.*/
	/**/
	t->hdr.tag              = ATAG_INITRD2;
	t->hdr.size             = tag_size(tag_initrd);
	t->u.initrd.start       = 0x01000000;   /* physical*/
	/*t->u.initrd.size        = 3 * 1024 * 1024;*/
	t->u.initrd.size        = 8 * 1024 * 1024;      /* depend on the size of ramdisk.gz*/
	t = tag_next(t);

	t->hdr.tag = ATAG_NONE;
	t->hdr.size = 0;
#endif
}

/* map wmt physical io address to virtual address */
static struct map_desc wmt_io_desc[] __initdata = {
	{
			.virtual    = 0xFE000000,
			.length     = SZ_16M,
			.pfn        = __phys_to_pfn(0xD8000000),
			.type       = MT_DEVICE
	}
};

static void __init wmt_map_io(void)
{
	iotable_init(wmt_io_desc, ARRAY_SIZE(wmt_io_desc));

	wmt_register_uart(0, 0);	/* mount ttyS0 (or ttyVT0) to UART0*/
	wmt_register_uart(1, 1);	/* mount ttyS1 to UART1*/
	wmt_register_uart(2, 2);	/* mount ttyS2 to UART2*/
	wmt_register_uart(3, 3);	/* mount ttyS3 to UART3*/
	wmt_register_uart(4, 4);	/* mount ttyS4 to UART4*/
	wmt_register_uart(5, 5);	/* mount ttyS5 to UART5*/
}

extern struct sys_timer wmt_timer;

MACHINE_START(WMT, "WMT")
#ifdef CONFIG_WMT_USE_BOOTLOADER_ATAG
	.boot_params	= 0x00000100,
#endif
	.fixup        = &wmt_fixup,
	.map_io       = &wmt_map_io,
	.init_irq     = wmt_init_irq,
	.timer        = &wmt_timer,
MACHINE_END

