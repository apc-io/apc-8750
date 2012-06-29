/*++
linux/include/asm-arm/arch-wmt/kpad.h

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

#ifndef __ASM_ARCH_KPAD_H
#define __ASM_ARCH_KPAD_H

#include <linux/ioport.h>

/*
 * Keypad register set structure
 */
struct kpad_regs_s {
	unsigned int volatile kpmcr;
	unsigned int volatile kpdcr;
	unsigned int volatile kpicr;
	unsigned int volatile kpstr;
	unsigned int volatile kpmar;
	unsigned int volatile kpdsr;
	unsigned int volatile kpmmr;
	unsigned int volatile kprir;
	unsigned int volatile kpmr0;
	unsigned int volatile kpmr1;
	unsigned int volatile kpmr2;
	unsigned int volatile kpmr3;
	unsigned int volatile kpmir;
	unsigned int volatile kpdir;

};

struct multi_key_s {
	unsigned int even:8;            /* even col row input */
	unsigned int res0:8;            /* reserved bits      */
	unsigned int odd:8;             /* odd col row input  */
	unsigned int res1:7;            /* reserved bits      */
	unsigned int flag:1;            /* valid flag         */

};

/*
 * Keypad interrupt event counters.
 */
struct kpad_ints_s {
	/*
	 * Global Status.
	 */
	unsigned int mda;       /* Keypad matrix manual debounce active */
	unsigned int asa;       /* Keypad matrix automatic scan on activity */
	unsigned int asc;       /* Keypad matrix automatic scan completed */
	unsigned int dia;       /* Keypad direct input active */
	unsigned int err;       /* Error keypad interrupts */

};

/*
 * Context need to be saved while hibernation.
 */
struct kpad_saved_s {
	unsigned int kpmcr;
	unsigned int kpdcr;
	unsigned int kpicr;
	unsigned int kpmir;
	unsigned int kpdir;

};

/*
 * wmt keypad operation structure.
 */
struct wmt_kpad_s {
	/* Module reference counter */
	unsigned int ref;

	/* I/O Resource */
	struct resource *res;

	/* Keypad I/O register set. */
	struct kpad_regs_s *regs;

	/* Interrupt number and status counters. */
	unsigned int irq;
	struct kpad_ints_s ints;

};

#define KPAD_IO_SIZE            sizeof(struct kpad_regs_s)
#define KPAD_INTS_SIZE          sizeof(struct kpad_ints_s)

#endif /* __ASM_ARCH_KPAD_H */
