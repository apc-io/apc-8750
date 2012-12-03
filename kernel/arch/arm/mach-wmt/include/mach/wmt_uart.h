/*++
linux/include/asm-arm/arch-wmt/wmt_uart.h

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

/* Be sure that virtual mapping is defined right */
#ifndef __ASM_ARCH_HARDWARE_H
#error "You must include hardware.h, not vt8500_uart.h"
#endif

#ifndef __WMT_UART_H
#define __WMT_UART_H

/*
 * Baud Rate Speed Calculation
 *
 * BR = Baud Rate
 *
 * BRD = Baud Rate Divisor
 *
 * UCLK = UART clock
 *
 * UCLK = APB_INPUT_CLOCK / (URDIV + 1), URDIV = UART clock divisor
 *
 * URDIV = (APB_INPUT_CLOCK / 12MHz) - 1
 *
 * BR = UCLK / (13 * (BRD + 1))
 *
 * BRD = (UCLK / (13 * BR)) - 1
 *
 * Note: UCLK *MUST* be equal to 12MHz.
 */

/*
 *   UART 0 : System Debug RS-232 (DB-9)
 */
#define UART0_URTDR_ADDR    (UART0_BASE_ADDR + 0x0000)
#define UART0_URRDR_ADDR    (UART0_BASE_ADDR + 0x0004)
#define UART0_URDIV_ADDR    (UART0_BASE_ADDR + 0x0008)
#define UART0_URLCR_ADDR    (UART0_BASE_ADDR + 0x000C)
#define UART0_URICR_ADDR    (UART0_BASE_ADDR + 0x0010)
#define UART0_URIER_ADDR    (UART0_BASE_ADDR + 0x0014)
#define UART0_URISR_ADDR    (UART0_BASE_ADDR + 0x0018)
#define UART0_URUSR_ADDR    (UART0_BASE_ADDR + 0x001C)
#define UART0_URFCR_ADDR    (UART0_BASE_ADDR + 0x0020)
#define UART0_URFIDX_ADDR   (UART0_BASE_ADDR + 0x0024)
#define UART0_URBKR_ADDR    (UART0_BASE_ADDR + 0x0028)
#define UART0_URTOD_ADDR    (UART0_BASE_ADDR + 0x002C)
#define UART0_URTXF_ADDR    (UART0_BASE_ADDR + 0x1000)
#define UART0_URRXF_ADDR    (UART0_BASE_ADDR + 0x1020)

#define UART0_URTDR_REG     REG32_PTR(UART0_URTDR_ADDR)     /* RW, Transmit data register */
#define UART0_URRDR_REG     REG32_PTR(UART0_URRDR_ADDR)     /* RO, Receive data register */
#define UART0_URDIV_REG     REG32_PTR(UART0_URDIV_ADDR)     /* RW, Baud rate divisor */
#define UART0_URLCR_REG     REG32_PTR(UART0_URLCR_ADDR)     /* RW, Line control register */
#define UART0_URICR_REG     REG32_PTR(UART0_URICR_ADDR)     /* RW, IrDA control register */
#define UART0_URIER_REG     REG32_PTR(UART0_URIER_ADDR)     /* RW, Interrupt enable register */
#define UART0_URISR_REG     REG32_PTR(UART0_URISR_ADDR)     /* RO, Interrupt status register */
#define UART0_URUSR_REG     REG32_PTR(UART0_URUSR_ADDR)     /* RO, UART status register */
#define UART0_URFCR_REG     REG32_PTR(UART0_URFCR_ADDR)     /* RW, FIFO control register */
#define UART0_URFIDX_REG    REG32_PTR(UART0_URFIDX_ADDR)    /* RO, FIFO index register */
#define UART0_URTOD_REG     REG32_PTR(UART0_URTOD_ADDR)     /* WR, UART clock divisor Register */
#define UART0_URBKR_REG     REG32_PTR(UART0_URBKR_ADDR)     /* RW, Break count-value register */

#define UART0_URTDR_VAL     REG32_VAL(UART0_URTDR_ADDR)
#define UART0_URRDR_VAL     REG32_VAL(UART0_URRDR_ADDR)
#define UART0_URDIV_VAL     REG32_VAL(UART0_URDIV_ADDR)
#define UART0_URLCR_VAL     REG32_VAL(UART0_URLCR_ADDR)
#define UART0_URICR_VAL     REG32_VAL(UART0_URICR_ADDR)
#define UART0_URIER_VAL     REG32_VAL(UART0_URIER_ADDR)
#define UART0_URISR_VAL     REG32_VAL(UART0_URISR_ADDR)
#define UART0_URUSR_VAL     REG32_VAL(UART0_URUSR_ADDR)
#define UART0_URFCR_VAL     REG32_VAL(UART0_URFCR_ADDR)
#define UART0_URFIDX_VAL    REG32_VAL(UART0_URFIDX_ADDR)
#define UART0_URTOD_VAL     REG32_VAL(UART0_URTOD_ADDR)
#define UART0_URBKR_VAL     REG32_VAL(UART0_URBKR_ADDR)

/*
 *   UART 1 : Hardware Loopback
 */
#define UART1_URTDR_ADDR    (UART1_BASE_ADDR + 0x0000)
#define UART1_URRDR_ADDR    (UART1_BASE_ADDR + 0x0004)
#define UART1_URDIV_ADDR    (UART1_BASE_ADDR + 0x0008)
#define UART1_URLCR_ADDR    (UART1_BASE_ADDR + 0x000C)
#define UART1_URICR_ADDR    (UART1_BASE_ADDR + 0x0010)
#define UART1_URIER_ADDR    (UART1_BASE_ADDR + 0x0014)
#define UART1_URISR_ADDR    (UART1_BASE_ADDR + 0x0018)
#define UART1_URUSR_ADDR    (UART1_BASE_ADDR + 0x001C)
#define UART1_URFCR_ADDR    (UART1_BASE_ADDR + 0x0020)
#define UART1_URFIDX_ADDR   (UART1_BASE_ADDR + 0x0024)
#define UART1_URBKR_ADDR    (UART1_BASE_ADDR + 0x0028)
#define UART1_URTOD_ADDR    (UART1_BASE_ADDR + 0x002C)
#define UART1_URTXF_ADDR    (UART1_BASE_ADDR + 0x1000)
#define UART1_URRXF_ADDR    (UART1_BASE_ADDR + 0x1020)

#define UART1_URTDR_REG     REG32_PTR(UART1_URTDR_ADDR)     /* RW, Transmit data register */
#define UART1_URRDR_REG     REG32_PTR(UART1_URRDR_ADDR)     /* RO, Receive data register */
#define UART1_URDIV_REG     REG32_PTR(UART1_URDIV_ADDR)     /* RW, Baud rate divisor */
#define UART1_URLCR_REG     REG32_PTR(UART1_URLCR_ADDR)     /* RW, Line control register */
#define UART1_URICR_REG     REG32_PTR(UART1_URICR_ADDR)     /* RW, IrDA control register */
#define UART1_URIER_REG     REG32_PTR(UART1_URIER_ADDR)     /* RW, Interrupt enable register */
#define UART1_URISR_REG     REG32_PTR(UART1_URISR_ADDR)     /* RO, Interrupt status register */
#define UART1_URUSR_REG     REG32_PTR(UART1_URUSR_ADDR)     /* RO, UART status register */
#define UART1_URFCR_REG     REG32_PTR(UART1_URFCR_ADDR)     /* RW, FIFO control register */
#define UART1_URFIDX_REG    REG32_PTR(UART1_URFIDX_ADDR)    /* RO, FIFO index register */
#define UART1_URTOD_REG     REG32_PTR(UART1_URTOD_ADDR)     /* WR, UART clock divisor Register */
#define UART1_URBKR_REG     REG32_PTR(UART1_URBKR_ADDR)     /* RW, Break count-value register */

#define UART1_URTDR_VAL     REG32_VAL(UART1_URTDR_ADDR)
#define UART1_URRDR_VAL     REG32_VAL(UART1_URRDR_ADDR)
#define UART1_URDIV_VAL     REG32_VAL(UART1_URDIV_ADDR)
#define UART1_URLCR_VAL     REG32_VAL(UART1_URLCR_ADDR)
#define UART1_URICR_VAL     REG32_VAL(UART1_URICR_ADDR)
#define UART1_URIER_VAL     REG32_VAL(UART1_URIER_ADDR)
#define UART1_URISR_VAL     REG32_VAL(UART1_URISR_ADDR)
#define UART1_URUSR_VAL     REG32_VAL(UART1_URUSR_ADDR)
#define UART1_URFCR_VAL     REG32_VAL(UART1_URFCR_ADDR)
#define UART1_URFIDX_VAL    REG32_VAL(UART1_URFIDX_ADDR)
#define UART1_URTOD_VAL     REG32_VAL(UART1_URTOD_ADDR)
#define UART1_URBKR_VAL     REG32_VAL(UART1_URBKR_ADDR)

/*
 *   UART 2 : External DB-9 connector
 */
#define UART2_URTDR_ADDR    (UART2_BASE_ADDR + 0x0000)
#define UART2_URRDR_ADDR    (UART2_BASE_ADDR + 0x0004)
#define UART2_URDIV_ADDR    (UART2_BASE_ADDR + 0x0008)
#define UART2_URLCR_ADDR    (UART2_BASE_ADDR + 0x000C)
#define UART2_URICR_ADDR    (UART2_BASE_ADDR + 0x0010)
#define UART2_URIER_ADDR    (UART2_BASE_ADDR + 0x0014)
#define UART2_URISR_ADDR    (UART2_BASE_ADDR + 0x0018)
#define UART2_URUSR_ADDR    (UART2_BASE_ADDR + 0x001C)
#define UART2_URFCR_ADDR    (UART2_BASE_ADDR + 0x0020)
#define UART2_URFIDX_ADDR   (UART2_BASE_ADDR + 0x0024)
#define UART2_URBKR_ADDR    (UART2_BASE_ADDR + 0x0028)
#define UART2_URTOD_ADDR    (UART2_BASE_ADDR + 0x002C)
#define UART2_URTXF_ADDR    (UART2_BASE_ADDR + 0x1000)
#define UART2_URRXF_ADDR    (UART2_BASE_ADDR + 0x1020)

#define UART2_URTDR_REG     REG32_PTR(UART2_URTDR_ADDR)     /* RW, Transmit data register */
#define UART2_URRDR_REG     REG32_PTR(UART2_URRDR_ADDR)     /* RO, Receive data register */
#define UART2_URDIV_REG     REG32_PTR(UART2_URDIV_ADDR)     /* RW, Baud rate divisor */
#define UART2_URLCR_REG     REG32_PTR(UART2_URLCR_ADDR)     /* RW, Line control register */
#define UART2_URICR_REG     REG32_PTR(UART2_URICR_ADDR)     /* RW, IrDA control register */
#define UART2_URIER_REG     REG32_PTR(UART2_URIER_ADDR)     /* RW, Interrupt enable register */
#define UART2_URISR_REG     REG32_PTR(UART2_URISR_ADDR)     /* RO, Interrupt status register */
#define UART2_URUSR_REG     REG32_PTR(UART2_URUSR_ADDR)     /* RO, UART status register */
#define UART2_URFCR_REG     REG32_PTR(UART2_URFCR_ADDR)     /* RW, FIFO control register */
#define UART2_URFIDX_REG    REG32_PTR(UART2_URFIDX_ADDR)    /* RO, FIFO index register */
#define UART2_URTOD_REG     REG32_PTR(UART2_URTOD_ADDR)     /* WR, UART clock divisor Register */
#define UART2_URBKR_REG     REG32_PTR(UART2_URBKR_ADDR)     /* RW, Break count-value register */

#define UART2_URTDR_VAL     REG32_VAL(UART2_URTDR_ADDR)
#define UART2_URRDR_VAL     REG32_VAL(UART2_URRDR_ADDR)
#define UART2_URDIV_VAL     REG32_VAL(UART2_URDIV_ADDR)
#define UART2_URLCR_VAL     REG32_VAL(UART2_URLCR_ADDR)
#define UART2_URICR_VAL     REG32_VAL(UART2_URICR_ADDR)
#define UART2_URIER_VAL     REG32_VAL(UART2_URIER_ADDR)
#define UART2_URISR_VAL     REG32_VAL(UART2_URISR_ADDR)
#define UART2_URUSR_VAL     REG32_VAL(UART2_URUSR_ADDR)
#define UART2_URFCR_VAL     REG32_VAL(UART2_URFCR_ADDR)
#define UART2_URFIDX_VAL    REG32_VAL(UART2_URFIDX_ADDR)
#define UART2_URTOD_VAL     REG32_VAL(UART2_URTOD_ADDR)
#define UART2_URBKR_VAL     REG32_VAL(UART2_URBKR_ADDR)

/*
 *   UART 3 : IR Sensor
 */

#define UART3_URTDR_ADDR    (UART3_BASE_ADDR + 0x0000)
#define UART3_URRDR_ADDR    (UART3_BASE_ADDR + 0x0004)
#define UART3_URBRD_ADDR    (UART3_BASE_ADDR + 0x0008)
#define UART3_URLCR_ADDR    (UART3_BASE_ADDR + 0x000C)
#define UART3_URICR_ADDR    (UART3_BASE_ADDR + 0x0010)
#define UART3_URIER_ADDR    (UART3_BASE_ADDR + 0x0014)
#define UART3_URISR_ADDR    (UART3_BASE_ADDR + 0x0018)
#define UART3_URUSR_ADDR    (UART3_BASE_ADDR + 0x001C)
#define UART3_URFCR_ADDR    (UART3_BASE_ADDR + 0x0020)
#define UART3_URFIDX_ADDR   (UART3_BASE_ADDR + 0x0024)
#define UART3_URBKR_ADDR    (UART3_BASE_ADDR + 0x0028)
#define UART3_URDIV_ADDR    (UART3_BASE_ADDR + 0x002C)
#define UART3_URTXF_ADDR    (UART3_BASE_ADDR + 0x0030)
#define UART3_URRXF_ADDR    (UART3_BASE_ADDR + 0x0040)

#define UART3_URTDR_REG     REG32_PTR(UART3_URTDR_ADDR)     /* RW, Transmit data register */
#define UART3_URRDR_REG     REG32_PTR(UART3_URRDR_ADDR)     /* RO, Receive data register */
#define UART3_URBRD_REG     REG32_PTR(UART3_URBRD_ADDR)     /* RW, Baud rate divisor */
#define UART3_URLCR_REG     REG32_PTR(UART3_URLCR_ADDR)     /* RW, Line control register */
#define UART3_URICR_REG     REG32_PTR(UART3_URICR_ADDR)     /* RW, IrDA control register */
#define UART3_URIER_REG     REG32_PTR(UART3_URIER_ADDR)     /* RW, Interrupt enable register */
#define UART3_URISR_REG     REG32_PTR(UART3_URISR_ADDR)     /* RO, Interrupt status register */
#define UART3_URUSR_REG     REG32_PTR(UART3_URUSR_ADDR)     /* RO, UART status register */
#define UART3_URFCR_REG     REG32_PTR(UART3_URFCR_ADDR)     /* RW, FIFO control register */
#define UART3_URFIDX_REG    REG32_PTR(UART3_URFIDX_ADDR)    /* RO, FIFO index register */
#define UART3_URDIV_REG     REG32_PTR(UART3_URDIV_ADDR)     /* WR, UART clock divisor Register */
#define UART3_URBKR_REG     REG32_PTR(UART3_URBKR_ADDR)     /* RW, Break count-value register */

#define UART3_URTDR_VAL     REG32_VAL(UART3_URTDR_ADDR)
#define UART3_URRDR_VAL     REG32_VAL(UART3_URRDR_ADDR)
#define UART3_URBRD_VAL     REG32_VAL(UART3_URBRD_ADDR)
#define UART3_URLCR_VAL     REG32_VAL(UART3_URLCR_ADDR)
#define UART3_URICR_VAL     REG32_VAL(UART3_URICR_ADDR)
#define UART3_URIER_VAL     REG32_VAL(UART3_URIER_ADDR)
#define UART3_URISR_VAL     REG32_VAL(UART3_URISR_ADDR)
#define UART3_URUSR_VAL     REG32_VAL(UART3_URUSR_ADDR)
#define UART3_URFCR_VAL     REG32_VAL(UART3_URFCR_ADDR)
#define UART3_URFIDX_VAL    REG32_VAL(UART3_URFIDX_ADDR)
#define UART3_URDIV_VAL     REG32_VAL(UART3_URDIV_ADDR)
#define UART3_URBKR_VAL     REG32_VAL(UART3_URBKR_ADDR)


/*
 *   UART 4 : IR Sensor
 */
#define UART4_URTDR_ADDR    (UART4_BASE_ADDR + 0x0000)
#define UART4_URRDR_ADDR    (UART4_BASE_ADDR + 0x0004)
#define UART4_URBRD_ADDR    (UART4_BASE_ADDR + 0x0008)
#define UART4_URLCR_ADDR    (UART4_BASE_ADDR + 0x000C)
#define UART4_URICR_ADDR    (UART4_BASE_ADDR + 0x0010)
#define UART4_URIER_ADDR    (UART4_BASE_ADDR + 0x0014)
#define UART4_URISR_ADDR    (UART4_BASE_ADDR + 0x0018)
#define UART4_URUSR_ADDR    (UART4_BASE_ADDR + 0x001C)
#define UART4_URFCR_ADDR    (UART4_BASE_ADDR + 0x0020)
#define UART4_URFIDX_ADDR   (UART4_BASE_ADDR + 0x0024)
#define UART4_URBKR_ADDR    (UART4_BASE_ADDR + 0x0028)
#define UART4_URDIV_ADDR    (UART4_BASE_ADDR + 0x002C)
#define UART4_URTXF_ADDR    (UART4_BASE_ADDR + 0x0030)
#define UART4_URRXF_ADDR    (UART4_BASE_ADDR + 0x0040)

#define UART4_URTDR_REG     REG32_PTR(UART4_URTDR_ADDR)     /* RW, Transmit data register */
#define UART4_URRDR_REG     REG32_PTR(UART4_URRDR_ADDR)     /* RO, Receive data register */
#define UART4_URBRD_REG     REG32_PTR(UART4_URBRD_ADDR)     /* RW, Baud rate divisor */
#define UART4_URLCR_REG     REG32_PTR(UART4_URLCR_ADDR)     /* RW, Line control register */
#define UART4_URICR_REG     REG32_PTR(UART4_URICR_ADDR)     /* RW, IrDA control register */
#define UART4_URIER_REG     REG32_PTR(UART4_URIER_ADDR)     /* RW, Interrupt enable register */
#define UART4_URISR_REG     REG32_PTR(UART4_URISR_ADDR)     /* RO, Interrupt status register */
#define UART4_URUSR_REG     REG32_PTR(UART4_URUSR_ADDR)     /* RO, UART status register */
#define UART4_URFCR_REG     REG32_PTR(UART4_URFCR_ADDR)     /* RW, FIFO control register */
#define UART4_URFIDX_REG    REG32_PTR(UART4_URFIDX_ADDR)    /* RO, FIFO index register */
#define UART4_URDIV_REG     REG32_PTR(UART4_URDIV_ADDR)     /* WR, UART clock divisor Register */
#define UART4_URBKR_REG     REG32_PTR(UART4_URBKR_ADDR)     /* RW, Break count-value register */

#define UART4_URTDR_VAL     REG32_VAL(UART4_URTDR_ADDR)
#define UART4_URRDR_VAL     REG32_VAL(UART4_URRDR_ADDR)
#define UART4_URBRD_VAL     REG32_VAL(UART4_URBRD_ADDR)
#define UART4_URLCR_VAL     REG32_VAL(UART4_URLCR_ADDR)
#define UART4_URICR_VAL     REG32_VAL(UART4_URICR_ADDR)
#define UART4_URIER_VAL     REG32_VAL(UART4_URIER_ADDR)
#define UART4_URISR_VAL     REG32_VAL(UART4_URISR_ADDR)
#define UART4_URUSR_VAL     REG32_VAL(UART4_URUSR_ADDR)
#define UART4_URFCR_VAL     REG32_VAL(UART4_URFCR_ADDR)
#define UART4_URFIDX_VAL    REG32_VAL(UART4_URFIDX_ADDR)
#define UART4_URDIV_VAL     REG32_VAL(UART4_URDIV_ADDR)
#define UART4_URBKR_VAL     REG32_VAL(UART4_URBKR_ADDR)


/*
 *   UART 5 : IR Sensor
 */
#define UART5_URTDR_ADDR    (UART5_BASE_ADDR + 0x0000)
#define UART5_URRDR_ADDR    (UART5_BASE_ADDR + 0x0004)
#define UART5_URBRD_ADDR    (UART5_BASE_ADDR + 0x0008)
#define UART5_URLCR_ADDR    (UART5_BASE_ADDR + 0x000C)
#define UART5_URICR_ADDR    (UART5_BASE_ADDR + 0x0010)
#define UART5_URIER_ADDR    (UART5_BASE_ADDR + 0x0014)
#define UART5_URISR_ADDR    (UART5_BASE_ADDR + 0x0018)
#define UART5_URUSR_ADDR    (UART5_BASE_ADDR + 0x001C)
#define UART5_URFCR_ADDR    (UART5_BASE_ADDR + 0x0020)
#define UART5_URFIDX_ADDR   (UART5_BASE_ADDR + 0x0024)
#define UART5_URBKR_ADDR    (UART5_BASE_ADDR + 0x0028)
#define UART5_URDIV_ADDR    (UART5_BASE_ADDR + 0x002C)
#define UART5_URTXF_ADDR    (UART5_BASE_ADDR + 0x0030)
#define UART5_URRXF_ADDR    (UART5_BASE_ADDR + 0x0040)

#define UART5_URTDR_REG     REG32_PTR(UART5_URTDR_ADDR)     /* RW, Transmit data register */
#define UART5_URRDR_REG     REG32_PTR(UART5_URRDR_ADDR)     /* RO, Receive data register */
#define UART5_URBRD_REG     REG32_PTR(UART5_URBRD_ADDR)     /* RW, Baud rate divisor */
#define UART5_URLCR_REG     REG32_PTR(UART5_URLCR_ADDR)     /* RW, Line control register */
#define UART5_URICR_REG     REG32_PTR(UART5_URICR_ADDR)     /* RW, IrDA control register */
#define UART5_URIER_REG     REG32_PTR(UART5_URIER_ADDR)     /* RW, Interrupt enable register */
#define UART5_URISR_REG     REG32_PTR(UART5_URISR_ADDR)     /* RO, Interrupt status register */
#define UART5_URUSR_REG     REG32_PTR(UART5_URUSR_ADDR)     /* RO, UART status register */
#define UART5_URFCR_REG     REG32_PTR(UART5_URFCR_ADDR)     /* RW, FIFO control register */
#define UART5_URFIDX_REG    REG32_PTR(UART5_URFIDX_ADDR)    /* RO, FIFO index register */
#define UART5_URDIV_REG     REG32_PTR(UART5_URDIV_ADDR)     /* WR, UART clock divisor Register */
#define UART5_URBKR_REG     REG32_PTR(UART5_URBKR_ADDR)     /* RW, Break count-value register */

#define UART5_URTDR_VAL     REG32_VAL(UART5_URTDR_ADDR)
#define UART5_URRDR_VAL     REG32_VAL(UART5_URRDR_ADDR)
#define UART5_URBRD_VAL     REG32_VAL(UART5_URBRD_ADDR)
#define UART5_URLCR_VAL     REG32_VAL(UART5_URLCR_ADDR)
#define UART5_URICR_VAL     REG32_VAL(UART5_URICR_ADDR)
#define UART5_URIER_VAL     REG32_VAL(UART5_URIER_ADDR)
#define UART5_URISR_VAL     REG32_VAL(UART5_URISR_ADDR)
#define UART5_URUSR_VAL     REG32_VAL(UART5_URUSR_ADDR)
#define UART5_URFCR_VAL     REG32_VAL(UART5_URFCR_ADDR)
#define UART5_URFIDX_VAL    REG32_VAL(UART5_URFIDX_ADDR)
#define UART5_URDIV_VAL     REG32_VAL(UART5_URDIV_ADDR)
#define UART5_URBKR_VAL     REG32_VAL(UART5_URBKR_ADDR)


/*
 * UART Line Control Register Bit Definitions
 */
#define URLCR_TXEN      BIT0    /* Transmit operation enabled           */
#define URLCR_RXEN      BIT1    /* Receive operation enabled            */
#define URLCR_DLEN      BIT2    /* Data length 0:7-bit 1:8-bit          */
#define URLCR_STBLEN    BIT3    /* Stop bit length 0:1-bit 1:2-bit      */
#define URLCR_PTYEN     BIT4    /* Parity bit  0:inactive 1:active      */
#define URLCR_PTYMODE   BIT5    /* Parity mode 0:evev 1:odd             */
/* Request to send. A software controlled RTS modem signal, used when IrDA is disableda */
#define URLCR_RTS       BIT6
#define URLCR_LPBEN     BIT7    /* Loopback mode 0:inactive 1:active    */
#define URLCR_DMAEN     BIT8    /* DMA enable. 0:inactive 1:active      */
#define URLCR_BKINIT    BIT9    /* Bluetooth break signal initiation.   */
/* Bit[10:31] are reserved. */

/*
 * UART Status Register Bit Definitions
 */
#define URUSR_TXON      BIT0    /* Transmission is active               */
#define URUSR_TXDBSY    BIT1    /* TX data is being loaded to TX port from either URTDR or TX FIFO */
#define URUSR_RXON      BIT2    /* Reception is active                  */
#define URUSR_RXDRDY    BIT3    /* RX data is ready in either URRDR or RX FIFO */
#define URUSR_CTS       BIT4    /* Status of CTS signal                 */
#define URUSR_MASK      ((1 << 5) - 1)  /* Mask for useful bits         */
/* Bit[5:31] are reserved. */

/*
 * UART Interrupt Enable Register Bit Definitions
 */
#define URIER_ETXDE     BIT0    /* Enable for TX data register empty    */
#define URIER_ERXDF     BIT1    /* Enable for RX data register full     */
#define URIER_ETXFAE    BIT2    /* Enable for TX FIFO almost full       */
#define URIER_ETXFE     BIT3    /* Enable for TX FIFO full              */
#define URIER_ERXFAF    BIT4    /* Enable for RX FIFO almost full       */
#define URIER_ERXFF     BIT5    /* Enable for RX FIFO full              */
#define URIER_ETXDUDR   BIT6    /* Enable for TX underrun               */
#define URIER_ERXDOVR   BIT7    /* Enable for RX overrun                */
#define URIER_EPER      BIT8    /* Enable for parity error              */
#define URIER_EFER      BIT9    /* Enable for frame error               */
#define URIER_EMODM     BIT10   /* Enable for modem control signal      */
#define URIER_ERXTOUT   BIT11   /* Enable for receive time out          */
#define URIER_EBK       BIT12   /* Enable for break signal done         */
/* Bit[13:31] are reserved. */

/*
 * UART Interrupt Status Register Bit Definitions
 */
#define URISR_TXDE      BIT0    /* TX data register empty               */
#define URISR_RXDF      BIT1    /* RX data register full                */
#define URISR_TXFAE     BIT2    /* TX FIFO almost empty                 */
#define URISR_TXFE      BIT3    /* TX FIFO empty                        */
#define URISR_RXFAF     BIT4    /* RX FIFO almost empty                 */
#define URISR_RXFF      BIT5    /* RX FIFO empty                        */
#define URISR_TXDUDR    BIT6    /* TX underrun                          */
#define URISR_RXDOVR    BIT7    /* RX overrun                           */
#define URISR_PER       BIT8    /* Parity error                         */
#define URISR_FER       BIT9    /* Frame error                          */

/* Toggle clear to send modem control signal. Used when IrDA is disabled*/
#define URISR_TCTS      BIT10
#define URISR_RXTOUT    BIT11   /* Receive time out                     */
#define URISR_BKDONE    BIT12   /* Break signal done                    */
#define URISR_MASK      ((1 << 13) - 1) /* Mask for useful bits         */
/* Bit[13:31] are reserved. */

/*
 * IrDA Mode Control Register Description
 */
#define URICR_IREN      BIT0    /* Set "1" to enable IrDA               */
/* Bit[1:31] are reserved. */

/*
 * UART FIFO Control Register Description
 */
#define URFCR_FIFOEN            BIT0
#define URFCR_TRAIL				BIT1
/* Bit[1:3] are reserved. */

/*
 * Macros for setting threshold value to TX or RX FIFO level setting.
 */
#define URFCR_FLVMASK           0xf                           /* FIFO threshold Level Mask */
#define URFCR_TXFLV(x)          (((x) & URFCR_FLVMASK) << 4)  /* TX FIFO threshold */
#define URFCR_RXFLV(x)          (((x) & URFCR_FLVMASK) << 8)  /* RX FIFO threshold */
/* Bit[12:31] are reserved. */

/*
 * UART Baud Rate Divisor Register Description.
 */
#define URBRD_BRDMASK           0x3ff                         /* Bit[0:9] are baud rate divisor */
#define URBRD_BRD(x)            ((x) & URBRD_BRDMASK)
/* Bit[10:31] are reserved. */

/*
 * UART FIFO Index Register Description.
 */
#define URFIDX_IDXMASK          0x1f
/*
 * Macros for getting URFIDX value to TX or RX FIFO index.
 */                           /* FIFO index Mask */
#define URFIDX_TXFIDX(x)        ((x) & URFIDX_IDXMASK)        /* Get TX FIFO remaing entries */
/* Bit[5:7] are reserved. */

#define URFIDX_RXFIDX(x)        (((x) >> 8) & URFIDX_IDXMASK) /* Get RX FIFO remaing entries */
/* Bit[13:31] are reserved. */

/*
 * UART Break Counter Value Register Description.
 */
#define URBKR_BCVMASK           0x0fff                        /* Bit[0:11] are break counter value */
#define URBKR_BCV(x)            ((x) & URBKR_BCVMASK)
/* Bit[12:31] are reserved. */

#define URFCR_TXFRST            0x4		/* TX Fifo Reset */
#define URFCR_RXFRST            0x8		/* Rx Fifo Reset */

/*
 * UART clock divisor Register Description.
 */
#define URDIV_DIVMASK           0xf0000                            /* Bit[16:19] are UART clock divisor */
#define URDIV_DIV(x)            (((x) >> 16) & URDIV_DIVMASK)
/* Bit[4:31] are reserved. */

/*
 * UART module registers offset, add by Harry temporary.
 */
#define URTDR                   0x0000
#define URRDR                   0x0004
#define URDIV                   0x0008
#define URLCR                   0x000C
#define URICR                   0x0010
#define URIER                   0x0014
#define URISR                   0x0018
#define URUSR                   0x001C
#define URFCR                   0x0020
#define URFIDX                  0x0024
#define URBKR                   0x0028
#define URTOD                   0x002C
#define URTXF                   0x01000
#define URRXF                   0x01020

/*
 * URBRD_BRD value simple examples.
 */
#define BRD_921600BPS           0x10000
#define BRD_460800BPS           0x10001
#define BRD_230400BPS           0x10003
#define BRD_115200BPS           0x10007
#define BRD_76800BPS            0x1000B
#define BRD_57600BPS            0x1000F
#define BRD_38400BPS            0x10017
#define BRD_28800BPS            0x1001F


/*
 * URBKR_BCV value simple examples.
 *
 * Simply calculated by (baud_rate * 0.004096)
 * then take the integer.
 */
#define BCV_921600BPS           3775
#define BCV_460800BPS           1887
#define BCV_230400BPS           944
#define BCV_115200BPS           472
#define BCV_76800BPS            315
#define BCV_57600BPS            236
#define BCV_38400BPS            157
#define BCV_28800BPS            118

/*
 * URDIV_DIV value simple examples.
 *
 * Followings generate UCLK = 12MHZ
 */
#define DIV_192MHZ              15
#define DIV_180MHZ              14
#define DIV_168MHZ              13
#define DIV_156MHZ              12
#define DIV_144MHZ              11
#define DIV_132MHZ              10
#define DIV_120MHZ              9
#define DIV_108MHZ              8
#define DIV_96MHZ               7
#define DIV_84MHZ               6
#define DIV_72MHZ               5
#define DIV_60MHZ               4
#define DIV_48MHZ               3
#define DIV_36MHZ               2
#define DIV_24MHZ               1
#define DIV_12MHZ               0

/*
 * Data mask used in RX FIFO or URRDR.
 */
#define RX_DATAMASK             0xff                    /* Bit[0:7] are reception data */
#define RX_PERMASK              0x01ff                  /* Bit[0:8] */
#define RX_FERMASK              0x03ff                  /* Bit[0:9] */

#endif /* __WMT_UART_H */
