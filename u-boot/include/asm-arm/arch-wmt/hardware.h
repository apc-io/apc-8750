/*
 * Memory Map for WMT SoC
 *
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

#ifndef __HARDWARE_H
#define __HARDWARE_H

/*
 * Internal APB Slaves Memory Address Map
 */
#define BA_MC3      0xD8000000	/* DDR/DDR2 Memory Controller Configuration */
#define BA_DMA      0xD8001800	/* System Dma Base Address */
#define BA_SF       0xD8002000	/* Serial Flash Memory Controller Configuration */
#define BA_LPC      0xD8003000	/* LPC Memory Controller Configuration */
#define BA_ENET_0   0xD8004000	/* Ethernet MAC 0 Configuration */
#define BA_ENET_1   0xD8005000	/* Ethernet MAC 1 Configuration */
#define BA_CIPHER   0xD8006000	/* Security Acceleration Engine Configuration */
#define BA_UHC      0xD8007000	/* USB 2.0 Host Controller Configuration */
#define BA_PATA     0xD8008000	/* PATA Controller Configuration */
#define BA_NFC      0xD8009000	/* NANAD Controller Configuration */
#define BA_SDC      0xD800A000	/* SD/SDIO/MMC Controller Configuration */
#define BA_MSC      0xD800B000	/* Memory Stick/PRO Configuration */
#define BA_CFC      0xD800C000	/* Compact Flash Controller Configuration */
#define BA_SATA     0xD800D000	/* SATA Controller Configuration */
#define BA_XOR      0xD800E000	/* XOR Engine Configuration */
#define BA_SUPERIO  0xD8040000	/* LPC(Super IO) */
#define BA_RTC      0xD8100000	/* Real Time Clock (RTC) */
#define BA_GPIO     0xD8110000	/* GPIO */
#define BA_SCC      0xD8120000	/* System Configuration Control */
#define BA_PMC      0xD8130000	/* Power Management Control */
#define BA_IC       0xD8140000	/* Interrupt Controller */
#define BA_UART0    0xD8200000	/* UART0 */
#define BA_UART1    0xD8210000	/* UART1 */
#define BA_SPI      0xD8240000	/* SPI */
#define BA_I2C      0xD8280000	/* I2C */

#endif	/* __HARDWARE_H */
