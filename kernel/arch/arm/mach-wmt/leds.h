/*++
linux/arch/arm/mach-wmt/leds.h

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


#ifndef __ASM_ARCH_LEDS_H
#define __ASM_ARCH_LEDS_H

#include <asm/hardware.h>
#include <asm/arch/gpio.h>

extern void wmt_leds_event(led_event_t evt);

#endif /* __ASM_ARCH_LEDS_H */
