/*++ 
Copyright (c) 2010 WonderMedia Technologies, Inc.

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software 
Foundation, either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details. You
should have received a copy of the GNU General Public License along with this
program. If not, see http://www.gnu.org/licenses/>.

WonderMedia Technologies, Inc.
10F, 529, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C.
--*/

/*
 * serial.c - serial support for the WMT evaluation board
 */

#include <common.h>

#ifdef CFG_WMT_SERIAL

#include <asm/arch/hardware.h>
#include "uart.h"
#include "gpio.h"

DECLARE_GLOBAL_DATA_PTR;

PUART_REG	pUart_Reg = (PUART_REG)BA_UART0;
PGPIO_REG	pGpioReg = (PGPIO_REG)BA_GPIO;

/*
 * Function purpose:
 *				UART initialization
 */
int serial_init(void)
{
	/* Switch the Uart's pin from default GPIO into uart function pins for UART0/UART1 */
	pGpioReg->CTRL_UR_LPC_SF &= ~(GPIO_UR0_ALL | GPIO_UR1_ALL);

	serial_setbrg();

	return 0;
}

/*
 * Function purpose:
 *				Output character data through UART
 */
#ifdef CONFIG_WMT_UDC		  	
unsigned int do_puts=0;
#endif
void
serial_putc(const char c)
{
#ifdef CONFIG_WMT_UDC		  	
    char tmp[2];
    
    tmp[0] = c;
    tmp[1] = 0;
    
    if(( c != '\r' )&&(do_puts == 0))
    {
        usbtty_putc(c);
    }
#endif  
	while (1) {
		if ((pUart_Reg->URUSR & URUSR_TXDBSY) == 0) 	/* TX data not busy */
		break;
	}

	pUart_Reg->URTDR = c;
	if (c == '\n')
		serial_putc('\r');
}

/*
 * Function purpose:
 *				Output string data through UART
 */
void
serial_puts(const char *s)
{
#ifdef CONFIG_WMT_UDC		  	
    int tmp=0;
    
    do_puts = 1;
#endif
	while (*s)
  	{
		serial_putc(*s++);
#ifdef CONFIG_WMT_UDC		  	
      		tmp++;
#endif
}
#ifdef CONFIG_WMT_UDC		  	
    do_puts = 0;
    usbtty_puts ((const char *)((int)s-tmp));
#endif
}
/*
 * Function purpose:
 *				Require character data from UART
 */
int
serial_getc(void)
{
#ifdef CONFIG_WMT_UDC		  
	int tmp=0;
	char c;

	while (1)
	{
		if(pUart_Reg->URUSR & URUSR_RXDRDY)
			{
				tmp++;
		    break;
		  }
		if(c = usbtty_getc())
		    break;
	}

	return tmp?(pUart_Reg->URRDR&0xff):c;
#else
	while (!(pUart_Reg->URUSR & URUSR_RXDRDY));

	return pUart_Reg->URRDR&0xff;
#endif
}

/*
 * Function purpose:
 *				return RXDRDY status bit in Uart status register
 */
int
serial_tstc(void)
{
#ifdef CONFIG_WMT_UDC		  
	return (((pUart_Reg->URUSR & URUSR_RXDRDY)||usbtty_getc()) ? 1 : 0);
#else
	return ((pUart_Reg->URUSR & URUSR_RXDRDY) ? 1 : 0);
#endif		
}

/*
 * Function purpose:
 *				Uart baud rate setting and initialization
 * Note:
 *				Relative settings according to UART/IrDA
 *				Programmer Guide Revision 0.13
 */
void
serial_setbrg(void)
{
	DECLARE_GLOBAL_DATA_PTR;
	unsigned long TmpDiv = 0;
	unsigned long TmpBkr = 0;

	switch (gd->baudrate) {
	/*
	case BR_921K6:
		TmpDiv = UART_BR_921K6;
		TmpBkr = UART_BK_921K6;
		break;

	case BR_460K8:
		TmpDiv = UART_BR_460K8;
		TmpBkr = UART_BK_460K8;
		break;

	case BR_230K4:
		TmpDiv = UART_BR_230K4;
		TmpBkr = UART_BK_230K4;
		break;
	*/

	case BR_57K6:
		TmpDiv = UART_BR_57K6;
		TmpBkr = UART_BK_57K6;
		break;
	/*
	case BR_28K8:
		TmpDiv = UART_BR_28K8;
		TmpBkr = UART_BK_28K8;
		break;

	case BR_14K4:
		TmpDiv = UART_BR_14K4;
		TmpBkr = UART_BK_14K4;
		break;

	case BR_7K2:
		TmpDiv = UART_BR_7K2;
		TmpBkr = UART_BK_7K2;
		break;

	case BR_3K6:
		TmpDiv = UART_BR_3K6;
		TmpBkr = UART_BK_3K6;
		break;
	*/

	case BR_38K4:
		TmpDiv = UART_BR_38K4;
		TmpBkr = UART_BK_38K4;
		break;

	case BR_19K2:
		TmpDiv = UART_BR_19K2;
		TmpBkr = UART_BK_19K2;
		break;

	case BR_9K6:
		TmpDiv = UART_BR_9K6;
		TmpBkr = UART_BK_9K6;
		break;

	case BR_115K2:
	default:
		TmpDiv = UART_BR_115K2;
		TmpBkr = UART_BK_115K2;
		break;
	}

	pUart_Reg->URDIV = TmpDiv;
	pUart_Reg->URBKR = TmpBkr;

	/* Disable TX,RX */
	pUart_Reg->URLCR = 0;

	/* Disable all interrupt */
	pUart_Reg->URIER = 0;

	/* Clear all interrupt status until all zeros */
	/*
	while (pUart_Reg->URISR) {
		pUart_Reg->URISR = pUart_Reg->URISR;
	}
	*/

	/* Reset Tx,Rx Fifo until all zeros */
	while (pUart_Reg->URFCR) {
		/* Reset TX,RX Fifo */
		pUart_Reg->URFCR = URFCR_TXFRST | URFCR_RXFRST;
	}

	/* Disable Fifo */
	pUart_Reg->URFCR &= (~URFCR_FIFOEN);

	/* 8-N-1 */
	pUart_Reg->URLCR |= (URLCR_DLEN8B & ~URLCR_STBLEN2b & ~URLCR_PYTEN);

	/* Enable TX,RX */
	pUart_Reg->URLCR |= URLCR_RXEN | URLCR_TXEN;
}

#endif	/* CFG_WMT_SERIAL */
