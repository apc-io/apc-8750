/*++
linux/include/asm-arm/arch-wmt/serial.h

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

#ifndef __ASM_ARCH_SERIAL_H
#define __ASM_ARCH_SERIAL_H

/*
 * This assumes you have a 1.8432 MHz clock for your UART.
 *
 * It'd be nice if someone built a serial card with a 24.576 MHz
 * clock, since the 16550A is capable of handling a top speed of 1.5
 * megabits/second; but this requires the faster clock.
 *
 * TODO: Review BASE_BAUD definition.
 */
#define BASE_BAUD (1843200 / 16)

/* Standard COM flags */
#define STD_COM_FLAGS (ASYNC_BOOT_AUTOCONF | ASYNC_SKIP_TEST)


/*
 * Rather empty table...
 * Hardwired serial ports should be defined here.
 * PCMCIA will fill it dynamically.
 *
 * static struct old_serial_port old_serial_port[] = {
 *	SERIAL_PORT_DFNS // defined in <asm/serial.h>, used in driver/serial/8250.c
 * };
 *
 * TODO: Review if we really need it.
 */
#define STD_SERIAL_PORT_DEFNS	\
       /* UART	CLK     	PORT		IRQ	FLAGS		*/
	{ 0,	BASE_BAUD,	0, 		0,	STD_COM_FLAGS },
	{ 0,	BASE_BAUD,	0, 		0,	STD_COM_FLAGS },
	{ 0,	BASE_BAUD,	0, 		0,	STD_COM_FLAGS },
	{ 0,	BASE_BAUD,	0, 		0,	STD_COM_FLAGS }

#define EXTRA_SERIAL_PORT_DEFNS

#endif  /* __ASM_ARCH_SERIAL_H */
