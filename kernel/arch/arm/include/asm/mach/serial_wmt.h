/*
 * linux/include/asm-arm/mach/serial_wmt.h
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
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

struct uart_port;
struct uart_info;

/*
 * This is a temporary structure for registering these
 * functions; it is intended to be discarded after boot.
 */
struct wmt_port_fns {
	void	(*set_mctrl)(struct uart_port *, u_int);
	u_int	(*get_mctrl)(struct uart_port *);
	void	(*pm)(struct uart_port *, u_int, u_int);
	int	(*set_wake)(struct uart_port *, u_int);
};

#if defined(CONFIG_SERIAL_WMT)
void wmt_register_uart_fns(struct wmt_port_fns *fns);
void wmt_register_uart(int idx, int port);
#else
#define wmt_register_uart_fns(fns) do { } while (0)
#define wmt_register_uart(idx, port) do { } while (0)
#endif
