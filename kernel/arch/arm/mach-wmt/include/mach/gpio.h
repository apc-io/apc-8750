/*++
linux/include/asm-arm/arch-wmt/gpio.h

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

#ifndef __ASM_ARCH_GPIO_H
#define __ASM_ARCH_GPIO_H

/*
 * For GPIO_BASE + 0x0020 + (0x0020*n), n=0 to 2
 */
typedef struct gpio_uart_s {             /* total 32 bits */
	/* UART1 */
	unsigned char uart1_cts:1;
	unsigned char uart1_rts:1;
	unsigned char uart1_rxd:1;
	unsigned char uart1_txd:1;
	unsigned char reserve1:4;
	/* UART2 */
	unsigned char uart2_cts:1;
	unsigned char uart2_rts:1;
	unsigned char uart2_rxd:1;
	unsigned char uart2_txd:1;
	unsigned char reserve2:4;
	/* UART3 */
	unsigned char uart3_cts:1;
	unsigned char uart3_rts:1;
	unsigned char uart3_rxd:1;
	unsigned char uart3_txd:1;
	unsigned char reserve3:4;
	/* UART4 */
	unsigned char uart4_cts:1;
	unsigned char uart4_rts:1;
	unsigned char uart4_rxd:1;
	unsigned char uart4_txd:1;
	unsigned char reserve4:4;

} gpio_uart_t;

/*
 * For GPIO_BASE + 0x0024 + (0x0020*n), n=0 to 2
 */
typedef struct gpio_spi_s {              /* total 32 bits */
	/* SPI1 */
	unsigned char spi1_clk:1;
	unsigned char spi1_miso:1;
	unsigned char spi1_mosi:1;
	unsigned char spi1_ssn:1;
	unsigned char reserve1:4;
	/* SPI2 */
	unsigned char spi2_clk:1;
	unsigned char spi2_miso:1;
	unsigned char spi2_mosi:1;
	unsigned char spi2_ssn:1;
	unsigned char reserve2:4;
	/* SPI3 */
	unsigned char spi3_clk:1;
	unsigned char spi3_miso:1;
	unsigned char spi3_mosi:1;
	unsigned char spi3_ssn:1;
	unsigned char reserve3:4;
	/* SPI4 */
	unsigned char spi4_clk:1;
	unsigned char spi4_miso:1;
	unsigned char spi4_mosi:1;
	unsigned char spi4_ssn:1;
	unsigned char reserve4:4;

} gpio_spi_t;

/*
 * For GPIO_BASE + 0x0028 + (0x0020*n), n=0 to 2
 */
typedef struct gpio_kpad_s {             /* total 32 bits */
	/* KPAD_COW[7:0] */
	unsigned char cow0:1;
	unsigned char cow1:1;
	unsigned char cow2:1;
	unsigned char cow3:1;
	unsigned char cow4:1;
	unsigned char cow5:1;
	unsigned char cow6:1;
	unsigned char cow7:1;
	/* KPAD_ROW[7:0] */
	unsigned char row0:1;
	unsigned char row1:1;
	unsigned char row2:1;
	unsigned char row3:1;
	unsigned char row4:1;
	unsigned char row5:1;
	unsigned char row6:1;
	unsigned char row7:1;
	/* VIC_DATA[7:0] */
	unsigned char data0:1;
	unsigned char data1:1;
	unsigned char data2:1;
	unsigned char data3:1;
	unsigned char data4:1;
	unsigned char data5:1;
	unsigned char data6:1;
	unsigned char data7:1;
	/* VIC misc pins */
	unsigned char clk:1;
	unsigned char vreq:1;
	unsigned char vsync:1;
	unsigned char hsync:1;
	unsigned char reserv1:4;

} gpio_kpad_t;

/*
 * For GPIO_BASE + 0x002C + (0x0020*n), n=0 to 2
 */
typedef struct gpio_misc_s {             /* total 32 bits */
	/* SDMMC */
	unsigned char sd_data0:1;
	unsigned char sd_data1:1;
	unsigned char sd_data2:1;
	unsigned char sd_data3:1;
	unsigned char sd_clk:1;
	unsigned char sd_cmd:1;
	unsigned char reverse1:2;
	/* I2S */
	unsigned char i2s_sclk:1;
	unsigned char i2s_sysclk:1;
	unsigned char i2s_sdi:1;
	unsigned char i2s_sdo:1;
	unsigned char i2s_ws:1;
	/* Wake-up[4:2] */
	unsigned char wakeup2:1;
	unsigned char wakeup3:1;
	unsigned char wakeup4:1;
	/* I2C */
	unsigned char i2c_scl:1;
	unsigned char i2c_sda:1;
	/* Wake-up[1:0] */
	unsigned char wakeup0:1;
	unsigned char wakeup1:1;
	/* AC'97 */
	unsigned char ac97_bclk:1;
	unsigned char ac97_sdi:1;
	unsigned char ac97_sdo:1;
	unsigned char ac97_sync:1;
	/* PWM_OUT[3:0] */
	unsigned char pwm0:1;
	unsigned char pwm1:1;
	unsigned char pwm2:1;
	unsigned char pwm3:1;
	/* PCM */
	unsigned char pcm_clk:1;
	unsigned char pcm_in:1;
	unsigned char pcm_out:1;
	unsigned char pcm_sync:1;

} gpio_misc_t;

/*
 * For GPIO_BASE + 0x0084
 */
typedef struct gpio_irqt_s {
	/* GPIO_IRQ[9:0] type */
	unsigned char  irqt0:2;
	unsigned char  irqt1:2;
	unsigned char  irqt2:2;
	unsigned char  irqt3:2;
	unsigned char  irqt4:2;
	unsigned char  irqt5:2;
	unsigned char  irqt6:2;
	unsigned char  irqt7:2;
	unsigned char  irqt8:2;
	unsigned char  irqt9:2;
	unsigned char  reverse1:4;
	unsigned char  reverse2;

} gpio_irqt_t;

#endif /* __ASM_ARCH_GPIO_H */
