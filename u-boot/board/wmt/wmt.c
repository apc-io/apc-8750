/*
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * Copyright (c) 2008 WonderMedia Technologies, Inc.
 *
 * This program is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software Foundation,
 * either version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * WonderMedia Technologies, Inc.
 * 10F, 529, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C.
 */

#include <common.h>
#if defined(CONFIG_WMT)
#include <./configs/wmt.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

static inline void delay(unsigned long loops)
{
	__asm__ volatile ("1:\n"
		"subs %0, %1, #1\n"
		"bne 1b" : "=r" (loops) : "0" (loops));
}

/*
 * Miscellaneous platform dependent initialisations
 */

int board_init(void)
{
	gd->bd->bi_arch_number = MACH_TYPE_WMT;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0x00000100;

/* this speeds up your boot a quite a bit.  However to make it
 *  work, you need make sure your kernel startup flush bug is fixed.
 *  ... rkw ...
 */
	icache_enable();

	return 0;
}

int misc_init_r(void)
{
	/* currently empty */
	return 0;
}

/******************************
 Routine:
 Description:
******************************/
int dram_init(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	return 0;
}

/* Dummy function to avoid linker complaints */
void __aeabi_unwind_cpp_pr0(void)
{
}
