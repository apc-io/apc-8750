/*
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * Copyright (c) 2008 WonderMedia Technologies, Inc.
 *
 * This program is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software Foundation,
 * either version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * WonderMedia Technologies, Inc.
 * 10F, 529, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C.
 */

#ifndef __UART_H_
#define __UART_H_

typedef struct _UART_REG_ {
	volatile unsigned long URTDR;             /* Transmit data register */
	volatile unsigned long URRDR;             /* Receive data register */
	volatile unsigned long URDIV;             /* UART clock divisor & Bard rate divisor */
	volatile unsigned long URLCR;             /* Line control register */
	volatile unsigned long URICR;             /* IrDA control register */
	volatile unsigned long URIER;             /* Interrupt enable register */
	volatile unsigned long URISR;             /* Interrupt status register */
	volatile unsigned long URUSR;             /* Status register */
	volatile unsigned long URFCR;             /* Fifo control register */
	volatile unsigned long URFIDX;            /* Fifo index register */
	volatile unsigned long URBKR;             /* Break signal count value register */
	volatile unsigned long URTOD;             /* UART time out divisor register */
	volatile unsigned long Resv30_FFF[0x3F4]; /* 0x0030 - 0x0FFF Reserved */
	volatile unsigned char TXFifo[32];        /* 0x1000 - 0x101F 16x8 bit asynchronous Fifo */
	volatile unsigned char RXFifo[32];        /* 0x1020 - 0x103F 16x10 bit asynchronous Fifo */
} UART_REG, *PUART_REG;

/*
 * (URRDR) Receive Data Regiser Description
 */
#define URRDR_PER		0x100	/* Parity Error. This bit is the same as URISR[8] */
#define URRDR_FER		0x200	/* Frame Error. This bit is the same as URISR[9] */

/*
 * (URLCR) UART Line Control Register Description
 */
#define URLCR_TXEN     0x1   /* Transmit Enable */
#define URLCR_RXEN     0x2   /* Receive Enable */
#define URLCR_DLEN8B 	 0x4   /* Data length 8 bit mode enable */
#define URLCR_STBLEN2b 0x8   /* 2 Stop bit enable */
#define URLCR_PYTEN		 0x10  /* Parity Enable */
#define URLCR_PTYODD	 0x20	 /* Even Parity mode selected */
#define URLCR_RTS      0x40	 /* Request To Send.                                               */
/*                              Software control RTS modem signal, used when IrDA is disabled. */
#define URLCR_LPBEN    0x80	 /* Loop Back enable */
#define URLCR_DMAEN    0x100 /* DMA enable                                                         */
/*                              0 : Disable DMA mode                                               */
/*                              1 : Enable UART DMA mode. The UART must be configured in Fifo mode */
/*                              if DMA is enabled.                                                 */
#define URLCR_BKINIT   0x200 /* BlueTooth break signal initiation */
#define URLCR_PSLVERR  0x400 /* Enable APB interface Error Response */

/*
 * (URICR) IrDA Control Register Description
 */
#define URICR_IREN		0x1		/* Infrared Enable */

/*
 * (URIER) UART Interrupt Enable Register Description
 */
#define URIER_TXDE    0x1     /* Enable TX Data Register Empty Interrupt */
#define URIER_RXDF    0x2     /* Enable RX Data Register Full Interrupt */
#define URIER_TXFAE   0x4     /* Enable TX Fifo Almost Empty Interrupt(Threshold) */
#define URIER_TXFE    0x8     /* Enable TX Fifo Empty Interrupt */
#define URIER_RXFAF   0x10    /* Enable Rx Fifo Almost Full Interrupt(Threshold) */
#define URIER_RXFF    0x20	  /* Enable Rx Fifo Full Interrupt */
#define URIER_TXDUDR  0x40	  /* Enable for TX Under Run Interrupt */
#define URIER_RXDOVER 0x80	  /* Enable for Rx Over Run Interrupt */
#define URIER_PER     0x100	  /* Enable for Parity Error Interrupt */
#define URIER_FER     0x200	  /* Enable for Frame Error Interrupt */
#define URIER_MODM    0x400	  /* Enable for Modem control signal - CTS interrupt */
#define URIER_RXTOUT  0x800	  /* Enable for Receive TimeOut Interrupt */
#define URIER_BK      0x1000  /* Enable for Break Signal Done Interrupt */
#define URIER_ERR     0x2000  /* Enable for APB Error Respone Interrupt */

/*
 * (URISR) UART Interrupt Status Register Description
 */
#define URISR_TXDE    0x1     /* TX Data Register Empty */
#define URISR_RXDF    0x2     /* RX Data Register Full */
#define URISR_TXFAE   0x4     /* TX Fifo Almost Empty(Threshold) */
#define URISR_TXFE    0x8     /* TX Fifo Empty */
#define URISR_RXFAF   0x10    /* Rx Fifo Almost Full (Threshold) */
#define URISR_RXFF    0x20    /* Rx Fifo Full */
#define URISR_TXDUDR  0x40    /* TX Under Run */
#define URISR_RXDOVER	0x80    /* Rx Over Run */
#define URISR_PER     0x100	  /* Parity Error */
#define URISR_FER     0x200	  /* Frame Error */
#define URISR_MODM    0x400	  /* Modem control signal */
#define URISR_RXTOUT  0x800	  /* Receive TimeOut */
#define URISR_BK      0x1000  /* Break Signal Done */
#define URISR_ERR     0x2000  /* APB Error Respone */

/*
 * (URUSR) UART Status Register Description
 */
#define URUSR_TXON    0x1		/* Transmitter On */
#define URUSR_TXDBSY  0x2		/* Transmitter Busy */
#define URUSR_RXON    0x4		/* Receiver On */
#define URUSR_RXDRDY  0x8		/* Receiver Ready */
#define URUSR_CTS     0x10	/* Clear To Send Current status of CTS signal */

/*
 * (URFCR) UART Fifo Control Register Description
 */
#define URFCR_FIFOEN	0x1		/* Fifo Enable */
#define URFCR_TRAIL		0x2		/* RX Fifo Trailing data flush */
#define URFCR_TXFRST	0x4		/* TX Fifo Reset */
#define URFCR_RXFRST	0x8		/* Rx Fifo Reset */
#define URFCR_TXFLV_0	0x0000
#define URFCR_TXFLV_1	0x0010
#define URFCR_TXFLV_2	0x0020
#define URFCR_TXFLV_3	0x0030
#define URFCR_TXFLV_4	0x0040
#define URFCR_TXFLV_5	0x0050
#define URFCR_TXFLV_6	0x0060
#define URFCR_TXFLV_7	0x0070
#define URFCR_TXFLV_8	0x0080
#define URFCR_TXFLV_9	0x0090
#define URFCR_TXFLV_10	0x00A0
#define URFCR_TXFLV_11	0x00B0
#define URFCR_TXFLV_12	0x00C0
#define URFCR_TXFLV_13	0x00D0
#define URFCR_TXFLV_14	0x00E0
#define URFCR_TXFLV_15	0x00F0
#define UFFCR_RXFLV_0	0x0000
#define UFFCR_RXFLV_1	0x0100
#define UFFCR_RXFLV_2	0x0200
#define UFFCR_RXFLV_3	0x0300
#define UFFCR_RXFLV_4	0x0400
#define UFFCR_RXFLV_5	0x0500
#define UFFCR_RXFLV_6	0x0600
#define UFFCR_RXFLV_7	0x0700
#define UFFCR_RXFLV_8	0x0800
#define UFFCR_RXFLV_9	0x0900
#define UFFCR_RXFLV_10	0x0A00
#define UFFCR_RXFLV_11	0x0B00
#define UFFCR_RXFLV_12	0x0C00
#define UFFCR_RXFLV_13	0x0D00
#define UFFCR_RXFLV_14	0x0E00
#define UFFCR_RXFLV_15	0x0F00

/*
 * UART Controller Constant Definition
 */
#define UART_BR_921K6	0x10000	/* Uart Baud Rate Setting into 921.6KHz */
#define UART_BR_460K8	0x10001	/* Uart Baud Rate Setting into 468.8KHz */
#define UART_BR_230K4	0x10003	/* Uart Baud Rate Setting into 230.4KHz */
#define UART_BR_115K2	0x10007	/* Uart Baud Rate Setting into 115.2KHz */
#define UART_BR_57K6	0x1000F	/* Uart Baud Rate Setting into 57.6KHz */
#define UART_BR_38K4	0x2000F	/* Uart Baud Rate Setting into 38.4KHz */
#define UART_BR_28K8	0x1001F	/* Uart Baud Rate Setting into 28.8KHz */
#define UART_BR_19K2	0x2001F	/* Uart Baud Rate Setting into 19.2KHz */
#define UART_BR_14K4	0x1003F	/* Uart Baud Rate Setting into 14.4KHz */
#define UART_BR_9K6		0x2003F	/* Uart Baud Rate Setting into 9.6KHz */
#define UART_BR_7K2		0x1007F	/* Uart Baud Rate Setting into 7.2KHz */
#define UART_BR_3K6		0x100FF	/* Uart Baud Rate Setting into 3.6KHz */

#define UART_BK_921K6	3775	/* Uart Break Signal Count Setting into 921.6KHz */
#define UART_BK_460K8	1920	/* Uart Break Signal Count Setting into 468.8KHz */
#define UART_BK_230K4	944		/* Uart Break Signal Count Setting into 230.4KHz */
#define UART_BK_115K2	472		/* Uart Break Signal Count Setting into 115.2KHz */
#define UART_BK_57K6	236		/* Uart Break Signal Count Setting into 57.6KHz */
#define UART_BK_38K4	157		/* Uart Break Signal Count Setting into 38.4KHz */
#define UART_BK_28K8	118		/* Uart Break Signal Count Setting into 28.8KHz */
#define UART_BK_19K2	79		/* Uart Break Signal Count Setting into 19.2KHz */
#define UART_BK_14K4	59		/* Uart Break Signal Count Setting into 14.4KHz */
#define UART_BK_9K6		39		/* Uart Break Signal Count Setting into 9.6KHz */
#define UART_BK_7K2		30		/* Uart Break Signal Count Setting into 7.2KHz */
#define UART_BK_3K6		15		/* Uart Break Signal Count Setting into 3.6KHz */

#define BR_921K6		921600	/* 921.6KHz */
#define BR_460K8		460800	/* 468.8KHz */
#define BR_230K4		230400	/* 230.4KHz */
#define BR_115K2		115200	/* 115.2KHz */
#define BR_57K6			57600	/* 57.6KHz */
#define BR_38K4			38400	/* 3.8.4KHz */
#define BR_28K8			28800	/* 28.8KHz */
#define BR_19K2			19200	/* 19.2KHz */
#define BR_14K4			14400	/* 14.4KHz */
#define BR_9K6			9600	/* 9.6KHz */
#define BR_7K2			7200	/* 7.2KHz */
#define BR_3K6			3600	/* 3.6KHz */

/* #define UART_CLK_SOURCE 24000000 */

/*
 * UART Function Return Status Constant Definitions
 */
#define UR_SUCCESS  0x0        /* return success */
#define UR_FAIL     0xFFFFFFFF /* return fail */
#define UR_RXNOTRDY	0x2        /* return RX not Ready */
#define UART0       0
#define UART1       1

/*
 * UART Public functions declaration
 *
 * Defined in uart.c
 */
ulong
init_uart(
	uchar control_number,
	ulong baud_rate
	);

ulong
set_uart_DMA_mode(
	uchar control_number
	);

ulong
set_uart_fifo_mode(
	uchar control_number
	);

ulong
set_uart_reg_mode(
	uchar control_number
	);

ulong
get_ch_from_uart(
	uchar control_number
	);

ulong
write_ch_to_uart(
	uchar control_number,
	uchar ch
	);

ulong
get_ch_from_uart_fifo(
	uchar control_number
	);

ulong
write_ch_to_uart_fifo(
	uchar control_number,
	uchar ch
	);

ulong
uart_loopback_mode(
	uchar control_number,
	uchar enable
	);

/*
 * Defined in the uartif.c
 */
void
uart_initialization(
	int FunctionNumber
	);

void uart_startio(void);

void
ugets(
	char *p_dest,
	int len
	);

void
uputs(
	char *str
	);

void
uprintf(
	char *format, ...
	);

#endif /* __UART_H_ */
