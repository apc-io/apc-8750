/*++
linux/include/asm-arm/arch-wmt/wmt_scc.h

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
#error "You must include hardware.h, not vt8500_scc.h"
#endif

#ifndef __VT8500_SCC_H
#define __VT8500_SCC_H

/*
 *   Refer SCC module register set.pdf ver. 0.15
 *
 */

/*#define SYSTEM_CFG_CTRL_BASE_ADDR                       0xF8400000  // 64K */

/*
 * Address
 */
#define SCC_VT8500_ID_ADDR                  (0x0000+SYSTEM_CFG_CTRL_BASE_ADDR)

#define SCC_DMA_REQ0_CSR_ADDR               (0x0020+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ1_CSR_ADDR               (0x0021+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ2_CSR_ADDR               (0x0022+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ3_CSR_ADDR               (0x0023+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ4_CSR_ADDR               (0x0024+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ5_CSR_ADDR               (0x0025+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ6_CSR_ADDR               (0x0026+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ7_CSR_ADDR               (0x0027+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ8_CSR_ADDR               (0x0028+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ9_CSR_ADDR               (0x0029+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ10_CSR_ADDR              (0x002A+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ11_CSR_ADDR              (0x002B+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ12_CSR_ADDR              (0x002C+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ13_CSR_ADDR              (0x002D+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ14_CSR_ADDR              (0x002E+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ15_CSR_ADDR              (0x002F+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ16_CSR_ADDR              (0x0030+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ17_CSR_ADDR              (0x0031+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ18_CSR_ADDR              (0x0032+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ19_CSR_ADDR              (0x0033+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ20_CSR_ADDR              (0x0034+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ21_CSR_ADDR              (0x0035+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ22_CSR_ADDR              (0x0036+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ23_CSR_ADDR              (0x0037+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ24_CSR_ADDR              (0x0038+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ25_CSR_ADDR              (0x0039+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ26_CSR_ADDR              (0x003A+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ27_CSR_ADDR              (0x003B+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ28_CSR_ADDR              (0x003C+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ29_CSR_ADDR              (0x003D+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ30_CSR_ADDR              (0x003E+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ31_CSR_ADDR              (0x003F+SYSTEM_CFG_CTRL_BASE_ADDR)
/*
 * Registers
 */
#define SCC_VT8500_ID_REG                   REG32_PTR(0x0000+SYSTEM_CFG_CTRL_BASE_ADDR)
/* Reserved 0x04 ~ 0x1F */
#define SCC_DMA_REQ0_CSR_REG                REG8_PTR(0x0020+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ1_CSR_REG                REG8_PTR(0x0021+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ2_CSR_REG                REG8_PTR(0x0022+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ3_CSR_REG                REG8_PTR(0x0023+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ4_CSR_REG                REG8_PTR(0x0024+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ5_CSR_REG                REG8_PTR(0x0025+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ6_CSR_REG                REG8_PTR(0x0026+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ7_CSR_REG                REG8_PTR(0x0027+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ8_CSR_REG                REG8_PTR(0x0028+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ9_CSR_REG                REG8_PTR(0x0029+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ10_CSR_REG               REG8_PTR(0x002A+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ11_CSR_REG               REG8_PTR(0x002B+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ12_CSR_REG               REG8_PTR(0x002C+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ13_CSR_REG               REG8_PTR(0x002D+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ14_CSR_REG               REG8_PTR(0x002E+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ15_CSR_REG               REG8_PTR(0x002F+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ16_CSR_REG               REG8_PTR(0x0030+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ17_CSR_REG               REG8_PTR(0x0031+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ18_CSR_REG               REG8_PTR(0x0032+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ19_CSR_REG               REG8_PTR(0x0033+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ20_CSR_REG               REG8_PTR(0x0034+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ21_CSR_REG               REG8_PTR(0x0035+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ22_CSR_REG               REG8_PTR(0x0036+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ23_CSR_REG               REG8_PTR(0x0037+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ24_CSR_REG               REG8_PTR(0x0038+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ25_CSR_REG               REG8_PTR(0x0039+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ26_CSR_REG               REG8_PTR(0x003A+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ27_CSR_REG               REG8_PTR(0x003B+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ28_CSR_REG               REG8_PTR(0x003C+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ29_CSR_REG               REG8_PTR(0x003D+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ30_CSR_REG               REG8_PTR(0x003E+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ31_CSR_REG               REG8_PTR(0x003F+SYSTEM_CFG_CTRL_BASE_ADDR)


/*
 * VAL Registers
 */
#define SCC_VT8500_ID_VAL                   REG32_VAL(0x0000+SYSTEM_CFG_CTRL_BASE_ADDR)
/* Reserved 0x04 ~ 0x1F */
#define SCC_DMA_REQ0_CSR_VAL                REG8_VAL(0x0020+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ1_CSR_VAL                REG8_VAL(0x0021+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ2_CSR_VAL                REG8_VAL(0x0022+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ3_CSR_VAL                REG8_VAL(0x0023+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ4_CSR_VAL                REG8_VAL(0x0024+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ5_CSR_VAL                REG8_VAL(0x0025+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ6_CSR_VAL                REG8_VAL(0x0026+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ7_CSR_VAL                REG8_VAL(0x0027+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ8_CSR_VAL                REG8_VAL(0x0028+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ9_CSR_VAL                REG8_VAL(0x0029+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ10_CSR_VAL               REG8_VAL(0x002A+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ11_CSR_VAL               REG8_VAL(0x002B+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ12_CSR_VAL               REG8_VAL(0x002C+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ13_CSR_VAL               REG8_VAL(0x002D+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ14_CSR_VAL               REG8_VAL(0x002E+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ15_CSR_VAL               REG8_VAL(0x002F+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ16_CSR_VAL               REG8_VAL(0x0030+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ17_CSR_VAL               REG8_VAL(0x0031+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ18_CSR_VAL               REG8_VAL(0x0032+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ19_CSR_VAL               REG8_VAL(0x0033+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ20_CSR_VAL               REG8_VAL(0x0034+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ21_CSR_VAL               REG8_VAL(0x0035+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ22_CSR_VAL               REG8_VAL(0x0036+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ23_CSR_VAL               REG8_VAL(0x0037+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ24_CSR_VAL               REG8_VAL(0x0038+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ25_CSR_VAL               REG8_VAL(0x0039+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ26_CSR_VAL               REG8_VAL(0x003A+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ27_CSR_VAL               REG8_VAL(0x003B+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ28_CSR_VAL               REG8_VAL(0x003C+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ29_CSR_VAL               REG8_VAL(0x003D+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ30_CSR_VAL               REG8_VAL(0x003E+SYSTEM_CFG_CTRL_BASE_ADDR)
#define SCC_DMA_REQ31_CSR_VAL               REG8_VAL(0x003F+SYSTEM_CFG_CTRL_BASE_ADDR)

/*
 *  SCC_VT8500_ID_REG
 *  VT8500 ID
 *
 */
#define SCC_ID_PART_NUMBER_MASK             0xFFFF0000
#define SCC_ID_MAJOR_MASK                   0x0000FF00
#define SCC_ID_METAL_MASK                   0x000000FF
#define SCC_VT8500_ID_MASK                  0xFFFFFFFF
#define SCC_ID_DEFAULT_PART_NUMBER          0x33000000
#define SCC_ID_MAJOR_01                     0x00000100
#define SCC_ID_METAL_01                     0x00000001
#define SCC_VT8500_ID_01                    (SCC_ID_DEFAULT_PART_NUMBER|SCC_ID_MAJOR_01|SCC_ID_METAL_01)

/*
 *  SCC_DMA_REQX_CSR_REG
 *  DMA request x channel select register
 *
 */
/* Reserved [07:05] */
#define SCC_CSR_ENABLE                      0x10
#define SCC_CSR_DISABLE                     0x00
#define SCC_CSR_SYSTEM_DMA                  0x00
#define SCC_CSR_DSP_DMA                     0x08
#define SCC_CSR_DMA_CHANNEL_0               0x00
#define SCC_CSR_DMA_CHANNEL_1               0x01
#define SCC_CSR_DMA_CHANNEL_2               0x02
#define SCC_CSR_DMA_CHANNEL_3               0x03
#define SCC_CSR_DMA_CHANNEL_4               0x04
#define SCC_CSR_DMA_CHANNEL_5               0x05
#define SCC_CSR_DMA_CHANNEL_6               0x06
#define SCC_CSR_DMA_CHANNEL_7               0x07


/*
 *  DMA Table
 *
 */
/*
#define AC97_MIC_DMA_REQ                    0   // ac97
#define AC97_PCM_RX_DMA_REQ                 1   // ac97
#define AC97_PCM_TX_DMA_REQ                 2   // ac97
#define APB_PCM_RX_REQ                      3   // pcm
#define APB_PCM_TX_REQ                      4   // pcm
#define APB_SDHOST_REQ                      5   // sdhost
#define APB_SPI_1_RX_REQ                    6   // spi1
#define APB_SPI_1_TX_REQ                    7   // spi1
#define APB_SPI_2_RX_REQ                    8   // spi2
#define APB_SPI_2_TX_REQ                    9   // spi2
#define APB_SPI_3_RX_REQ                    10  // spi3
#define APB_SPI_3_TX_REQ                    11  // spi3
#define APB_SPI_4_RX_REQ                    12  // spi4
#define APB_SPI_4_TX_REQ                    13  // spi4
#define GPIO_0_DMA_REQ                      14  // gpio
#define GPIO_1_DMA_REQ                      15  // gpio
#define GPIO_2_DMA_REQ                      16  // gpio
#define GPIO_3_DMA_REQ                      17  // gpio
#define I2S_RX_DMA_REQ                      18  // i2s
#define I2S_TX_DMA_REQ                      19  // i2s
#define UART_1_RX_DMA_REQ                   20  // uart1
#define UART_1_TX_DMA_REQ                   21  // uart1
#define UART_2_RX_DMA_REQ                   22  // uart2
#define UART_2_TX_DMA_REQ                   23  // uart2
#define UART_3_RX_DMA_REQ                   24  // uart3
#define UART_3_TX_DMA_REQ                   25  // uart3
#define UART_4_RX_DMA_REQ                   26  // uart4
#define UART_4_TX_DMA_REQ                   27  // uart4
*/


/* 28 - 31 *: Tied-off */

#endif /* __VT8500_SCC_H */
