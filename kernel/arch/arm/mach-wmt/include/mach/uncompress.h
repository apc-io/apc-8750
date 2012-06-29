/*++
linux/include/asm-arm/arch-wmt/uncompress.h

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

#include "hardware.h"

/*
 * The following code assumes the serial port has already been
 * initialized by the bootloader.  We search for the first enabled
 * port in the most probable order.  If you didn't setup a port in
 * your bootloader then nothing will appear (which might be desired).
 */

/*
 * Macros to manipulate UART port.
 */
#define UART(x)         (*(volatile unsigned long *)(serial_port + (x)))
/*
 * This one is unique bacause UART TX FIFO access is 8-bit write.
 */
#define UART_URTXF      (*(volatile unsigned char *)(serial_port + URTXF))

static void putstr(const char *s)
{
	unsigned long serial_port;

	do {
		/*
		 * If UART0 ready, do puts().
		 */
		serial_port = UART0_PHY_BASE_ADDR;

		if (UART(URLCR) & URLCR_TXEN)
			break;

		/*
		 * Else if UART2 ready, do puts().
		 */
		serial_port = UART1_PHY_BASE_ADDR;

		if (UART(URLCR) & URLCR_TXEN)
			break;

		/*
		 * Return if there is no UART ready.
		 */
		return;

	} while (0);

	/*
	 * Force to use register mode.
	 */
	UART(URFCR) &= ~URFCR_FIFOEN;

	for (; *s; s++) {
		/*
		 * wait for space in the UART's transmiter
		 */
		while (UART(URUSR) & URUSR_TXDBSY)
			;

		/*
		 * Send the character out.
		 */
		UART(URTDR) = *s;

		/*
		 * if there comes a LF, also do CR...
		 *
		 * Line Feed == '\n' == 10
		 */
		if (*s == 10) {
			while (UART(URUSR) & URUSR_TXDBSY)
				;
			/*
			 * CR = Carriage Return == '\r' == 13
			 */
			UART(URTDR) = 13;
		}
	}
}

/*
 * Nothing to do for these
 */
#define arch_decomp_setup()
#define arch_decomp_wdog()
