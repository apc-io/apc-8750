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

#include <common.h>

#ifdef CONFIG_ZDE_KERNEL_DEBUG

#define SemiSWI 0x123456

/*************************************************/
/* Semihosting extension SWI Operating Numbers   */
/*************************************************/
#define SYS_BOOTLOADOS	0x6002

/*
 * For manipulate CP15 cache and TLB control register. (R1)
 */
#define CR_M	(1 << 0)	/* MMU enable */
#define CR_A	(1 << 1)	/* Alignment abort enable */
#define CR_C	(1 << 2)	/* Dcache enable */
#define CR_W	(1 << 3)	/* Write buffer enable */
/* bit[4:7] are reserved */
#define CR_S	(1 << 8)	/* System MMU protection */
#define CR_R	(1 << 9)	/* ROM MMU protection */
/* bit[10:11] are reserved */
#define CR_I	(1 << 12)	/* Icache enable */
#define CR_V	(1 << 13)	/* Vectors relocated to 0xffff0000 */
/* bit[14:31] are reserved */

typedef struct __semi_sys_bootloados {
	unsigned int os_type;           /* 0x0: normal application; 0x1: Linux */
	unsigned int board_arch;        /* 0x0: Reserved; 0x4A (vt8253C) just pass to kernel R1 */
	unsigned int kernel_addr;       /* kernel address 0xC0008000 */
	unsigned int rootfs_addr;       /* rootfs address 0xC8000000 */
	unsigned int loading_type;      /* 0x0: boot loader handle; 0x1: Host use JTAG download */
	unsigned int vector_mask;       /* 0x0: turn off all to clear all vector */
	unsigned int is_semihosting;    /* 0x0: disable; 0x1: enable */
} semi_sys_bootloados_t;

/*
 * Function Name: do_SemiSWI()
 * Purpose: put op code in r0, put value or pointer in r1, send swi
 * Return Value: value in r0
 */
static inline int do_SemiSWI(int swi_op, void *arg)
{
  int value;
  asm volatile ("mov r0, %1; mov r1, %2; swi %a3; mov %0, r0"
       : "=r" (value) /* Outputs */
       : "r" (swi_op), "r" (arg), "i" (SemiSWI) /* Inputs */
       : "r0", "r1", "r2", "r3", "ip", "lr", "memory", "cc"
		/* Clobbers r0 and r1, and lr if in supervisor mode */);

  return value;
}

static void cp15_set_vector_low(void)
{
	volatile unsigned int ctrl;

	asm volatile
	(
		"mrc    p15, 0, %0, c1, c0, 0" : "=r" (ctrl)
	);
	ctrl &= ~CR_V;
	asm volatile
	(
		"mcr    p15, 0, %0, c1, c0, 0" : : "r" (ctrl)
	);
}

/*
 * Function Name: semi_cbootloados()
 * Purpose: Send SWI SYS_BOOTLOADOS to notify converter
 *          that we have load kernel and/or ramdisk successfully
 * Return Value: Void return
 */
void semi_bootloados(void)
{
	semi_sys_bootloados_t semi_sys_bootloados;

	/* To support ZDE Linux kernel debugging, */
	/* Ensure vector table is at 0x0 */
	cp15_set_vector_low();

	semi_sys_bootloados.os_type          = 1;
	semi_sys_bootloados.board_arch       = 10002;   /* 10002 mean vt3300 */
	semi_sys_bootloados.kernel_addr      = 0x00008000 ;
	semi_sys_bootloados.rootfs_addr      = 0x01000000 ;
	semi_sys_bootloados.loading_type     = 0;
	semi_sys_bootloados.vector_mask      = 0;
	semi_sys_bootloados.is_semihosting   = 0;

	do_SemiSWI(SYS_BOOTLOADOS, &semi_sys_bootloados);
}

#endif	/* CONFIG_ZDE_KERNEL_DEBUG */
