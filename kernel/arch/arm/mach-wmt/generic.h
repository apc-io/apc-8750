/*++
linux/arch/arm/mach-wmt/generic.h

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

#ifndef __wmt_GENERIC_H
#define __wmt_GENERIC_H

extern void __init wmt_init_irq(void);

#define SET_BANK(__nr, __start, __size) \
					mi->bank[__nr].start = (__start), \
					mi->bank[__nr].size = (__size), \
					mi->bank[__nr].node = (((unsigned)(__start) - PHYS_OFFSET) >> 27)

/*
 * CPUFreq interface
 */
struct cpufreq_policy;

/*
 * System clocks interface
 */
extern unsigned int wm8425_arm_khz(void);
extern unsigned int wm8425_ahb_khz(void);

#endif  /* __wmt_GENERIC_H */
