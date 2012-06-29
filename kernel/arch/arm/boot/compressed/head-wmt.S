/**
 *	linux/arch/arm/boot/compressed/head-wmt.S
 *
 *	WonderMedia SoC specific tweaks. This is merged into head.S by the linker.
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

#include <linux/linkage.h>
#include <asm/mach-types.h>

	.section        ".start", "ax"

__wmt_start:

	@ Preserve r8/r7 i.e. kernel entry values
	@ Data cache might be active.
	@ Be sure to flush kernel binary out of the cache,
	@ whatever state it is, before it is turned off.
	@ This is done by fetching through currently executed
	@ memory to be sure we hit the same cache.
	bic	r2, pc, #0x1f
	add	r3, r2, #0x4000			@ 16 kb is quite enough...
1:	ldr	r0, [r2], #32
	teq	r2, r3
	bne	1b
	mcr	p15, 0, r0, c7, c10, 4		@ drain WB
	mcr	p15, 0, r0, c7, c7, 0		@ flush I & D caches

	@ disabling MMU and caches
	mrc	p15, 0, r0, c1, c0, 0		@ read control reg
	bic	r0, r0, #0x0d			@ clear WB, DC, MMU
	bic	r0, r0, #0x1000			@ clear Icache
	mcr	p15, 0, r0, c1, c0, 0		@ write to CP15 cache and TLB control register 1
