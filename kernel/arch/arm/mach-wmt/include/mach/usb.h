/*++
linux/include/asm-arm/arch-wmt/usb.h

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

#ifndef	__ASM_ARCH_VT8500_USB_H
#define	__ASM_ARCH_VT8500_USB_H

/*#include <mach/board.h> */

/*
Define the base register for each USB controller
*/

#define GLOBAL_BASE			    0xD8007FF0/*UHDC Global Register Space 0x7F0~0x7F7 */
#define DEVICE_BASE			    0xD8007C00/*UDC Register Space         0x400~0x7EF */

#endif	/* __ASM_ARCH_VT8500_USB_H */
