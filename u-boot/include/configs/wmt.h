/*
 * Configuation settings for the WMT evaluation board
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

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_ARM920T    1	/* This is an ARM920T CPU core  */
#define CONFIG_WMT        1	/* in a WMT SoC	*/

/* input clock of PLL */
/* the WMT EVB uses 250MHz clock */
#define CONFIG_SYS_CLK_FREQ	250MHz

#undef CONFIG_USE_IRQ	/* we don't need IRQ/FIQ stuff */

#define CONFIG_MISC_INIT_R

#define CONFIG_CMDLINE_TAG		1	/* enable passing of ATAGs  */
#define CONFIG_SETUP_MEMORY_TAGS	1

/*
 * USB device configuration
 */
/*
#define CONFIG_USB_DEVICE	1
#define CONFIG_USB_TTY		1
#define CONFIG_USBD_VENDORID		0x040D
#define CONFIG_USBD_PRODUCTID		0x8435//0x8453
#define CONFIG_USBD_MANUFACTURER	"VIA Technologies,Inc."
#define CONFIG_USBD_PRODUCT_NAME	"VT8430 USB Device"
#define CONFIG_WMT_UDC 1
*/
/*
 * Size of malloc() pool
 */
#define CFG_MALLOC_LEN	(CFG_ENV_SIZE + 128*1024)
#define CFG_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */

/* WMT UART */
#define CFG_WMT_SERIAL

/*
 * select serial console configuration
 */
#define CONFIG_SERIAL1	1	/* we use SERIAL 1 on WMT EVB */

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_CONS_INDEX	1
#define CONFIG_BAUDRATE	115200
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

#define CONFIG_AUTO_COMPLETE 1	/* add autocompletion support */
#define CONFIG_HUSH_PARSER   1	/* use "hush" command parser 	*/
/*
*CharlesTu,2011.3.24,enable CMD_USB and usb storage ehci and uhci
*/

/* #define CONFIG_COMMANDS (CFG_CMD_NAND | CFG_CMD_DHCP | CFG_CMD_PING | CFG_CMD_MII | CONFIG_CMD_DFL) */
#define CONFIG_COMMANDS		(CFG_CMD_MMC | \
				CFG_CMD_FAT | \
				CFG_CMD_IDE | \
				CFG_CMD_DHCP | \
				CFG_CMD_PING | \
				CFG_CMD_MII | \
				CONFIG_CMD_DFL | \
				CFG_CMD_NAND | \
				CFG_CMD_ENV | \
				CFG_CMD_I2C | \
				CFG_CMD_USB)
#define  CONFIG_USB_STORAGE 1  
#define CONFIG_DOS_PARTITION  1  
/*default ehci driver */
#define CONFIG_USB_EHCI		1
#define CONFIG_USB_UHCI		1

#define CONFIG_MMC 1

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

#define CONFIG_BOOTDELAY	3
#define CONFIG_ZERO_BOOTDELAY_CHECK     /* check keypress when bootdelay = 0 */
#define CONFIG_BOOTCOMMAND	"cp.b ff200000 01000000 $(rootfs_size); bootm ff000000"
/* {JHT 2007/07/26 Change the root=/dev/ram0 into root=dev/ram0 */
/* -#define CONFIG_BOOTARGS "mem=128M root=/dev/ram0 rw initrd=0x01000000,32768K console=ttyS0,115200n8" */
#define CONFIG_BOOTARGS		"mem=127M root=dev/ram0 rw initrd=0x01000000,32768K console=ttyS0,115200n8"
/* }JHT-2007/07/26 */

/*
 * Hardware drivers
 */
#define CONFIG_USB_ETHER_ASIX   1
#define CONFIG_CMD_NET 		 1
#define CONFIG_CMD_PING 		 1  
#define CONFIG_CMD_DHCP            1

#ifdef CONFIG_DRIVER_ETHER
#undef CONFIG_NET_MULTI 				/* Using WMT MAC */
#undef CONFIG_USB_HOST_ETHER  
#else    								
#define CONFIG_USB_HOST_ETHER  1 	/* Using usb ethernet adapter */
#define CONFIG_NET_MULTI 		 1
#endif

#define CONFIG_ETHADDR    00:40:63:00:00:00
#define CONFIG_NETMASK    255.255.255.0	/* talk on MY local net */
#define CONFIG_IPADDR     192.168.1.2	  /* static IP I currently own */
#define CONFIG_SERVERIP   192.168.1.1	  /* current IP of my dev pc */
#define CONFIG_BOOTFILE   "uzImage.bin"	/* Name of the image to load with TFTP */

#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE  115200  /* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	1	      /* which serial port to use */
#endif

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP                     /* undef to save memory     */
#define CFG_PROMPT    "WMT # "           /* Monitor Command Prompt   */
#define CFG_CBSIZE    1024               /* Console I/O Buffer Size  */
/* Print Buffer Size */
#define CFG_PBSIZE	(CFG_CBSIZE+sizeof(CFG_PROMPT)+16)
#define CFG_MAXARGS	32                   /* max number of command args */
#define CFG_BARGSIZE	CFG_CBSIZE         /* Boot Argument Buffer Size  */

#define CFG_MEMTEST_START	0x00000000     /* memtest works on	*/
#define CFG_MEMTEST_END		0x03F00000     /* 63 MB in DRAM	*/

#undef  CFG_CLKS_IN_HZ  /* everything, incl board info, in Hz */

#define	CFG_LOAD_ADDR		0x02000000	/* default load address	*/
#define CONFIG_LOADADDR		CFG_LOAD_ADDR

#define	CONFIG_VERSION_VARIABLE	1		/* include version env variable */

#define CONFIG_ZDE_KERNEL_DEBUG			/* ZDE kernel debugging support */

/* Give WMT TIMERBAES, Give Timer clock in Hz
 * This time eat 12MHz clk and increase its count at 3MHz.
 */
#define	CFG_HZ			3000000		/* OS timer increase its count at 3MHz */

/*-----------------------------------------------------------------------
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	(128*1024)	/* regular stack */
#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ	(4*1024)	/* IRQ stack */
#define CONFIG_STACKSIZE_FIQ	(4*1024)	/* FIQ stack */
#endif

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	1	   /* we have 1 bank of DRAM */
#define PHYS_SDRAM_1      0x00000000 /* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE 0x08000000 /* 128 MB */
#define PHYS_FLASH_1      0xFF000000 /* Flash Bank #1 */ /* nick span */
#define PHYS_FLASH_2      0xFF800000 /* Flash Bank #2 */ /* nick span */
#define CFG_FLASH_BASE		PHYS_FLASH_1

#if (NOR_BOOT_ERASE_SIZE_KB == 128)
#define CFG_NOR_FLASH_BASE_3    0xFE000000 /* nor boot nor flash bank 3 addr*/
#define CFG_NOR_FLASH_BASE_2    0xFC000000 /* nor boot nor flash bank 2 addr*/
#define CFG_NOR_FLASH_BASE_1    0xFA000000 /* nor boot nor flash bank 1 addr*/
#define CFG_NOR_FLASH_BASE_0    0xF8000000 /* nor boot nor flash bank 0 addr*/
#define SPI_BOOT_FLASH_BASE_3   0xEE000000 /* spi boot nor flash bank 3 addr*/
#define SPI_BOOT_FLASH_BASE_2   0xEC000000 /* spi boot nor flash bank 2 addr*/
#define SPI_BOOT_FLASH_BASE_1   0xEA000000 /* spi boot nor flash bank 1 addr*/
#define SPI_BOOT_FLASH_BASE_0   0xE8000000 /* spi boot nor flash bank 0 addr*/
#else
#define CFG_NOR_FLASH_BASE_3    0xFF800000 /* nor boot nor flash bank 3 addr*/
#define CFG_NOR_FLASH_BASE_2    0xFF000000 /* nor boot nor flash bank 2 addr*/
#define CFG_NOR_FLASH_BASE_1    0xFE800000 /* nor boot nor flash bank 1 addr*/
#define CFG_NOR_FLASH_BASE_0    0xFE000000 /* nor boot nor flash bank 0 addr*/
#define SPI_BOOT_FLASH_BASE_3   0xEF800000 /* spi boot nor flash bank 3 addr*/
#define SPI_BOOT_FLASH_BASE_2   0xEF000000 /* spi boot nor flash bank 2 addr*/
#define SPI_BOOT_FLASH_BASE_1   0xEE800000 /* spi boot nor flash bank 1 addr*/
#define SPI_BOOT_FLASH_BASE_0   0xEE000000 /* spi boot nor flash bank 0 addr*/
#endif

#define NOR_FLASH_128K       0x0 /* nor flash offset address range 128K*/
#define NOR_FLASH_256K       0x1 /* nor flash offset address range 256K*/
#define NOR_FLASH_512K       0x2 /* nor flash offset address range 512K*/
#define NOR_FLASH_1M         0x3 /* nor flash offset address range 1M*/
#define NOR_FLASH_2M         0x4 /* nor flash offset address range 2M*/
#define NOR_FLASH_4M         0x5 /* nor flash offset address range 4M*/
#define NOR_FLASH_8M         0x6 /* nor flash offset address range 8M*/
#define NOR_FLASH_16M        0x7 /* nor flash offset address range 16M*/
#define NOR_FLASH_32M        0x8 /* nor flash offset address range 32M*/
#define NOR_FLASH_64M        0x9 /* nor flash offset address range 64M*/
#define NOR_FLASH_128M       0xa /* nor flash offset address range 128M*/
#define NOR_FLASH_256M       0xb /* nor flash offset address range 256M*/
#define NOR_FLASH_512M       0xc /* nor flash offset address range 512M*/
#define NOR_FLASH_1G         0xd /* nor flash offset address range 1G*/
#define NOR_FLASH_2G         0xe /* nor flash offset address range 2G*/
#define NOR_FLASH_4G         0xf /* nor flash offset address range 4G*/
/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CFG_MAX_FLASH_BANKS	     1	/* max number of memory banks */
#define CFG_MAX_SPI_FLASH_BANKS	 2	/* max number of spi flash banks*/
#define CFG_MAX_NOR_FLASH_BANKS	 4	/* max number of nor flash banks */
#define CFG_MAX_NAND_FLASH_BANKS 4	/* max number of nand flash banks */
#if (CFG_MAX_FLASH_BANKS < CFG_MAX_SPI_FLASH_BANKS)
	#undef CFG_MAX_FLASH_BANKS
	#define CFG_MAX_FLASH_BANKS CFG_MAX_SPI_FLASH_BANKS
#endif
#if (CFG_MAX_FLASH_BANKS < CFG_MAX_NOR_FLASH_BANKS)
	#undef CFG_MAX_FLASH_BANKS
	#define CFG_MAX_FLASH_BANKS CFG_MAX_NOR_FLASH_BANKS
#endif
#if (CFG_MAX_FLASH_BANKS < CFG_MAX_NAND_FLASH_BANKS)
	#undef CFG_MAX_FLASH_BANKS
	#define CFG_MAX_FLASH_BANKS CFG_MAX_NAND_FLASH_BANKS
#endif


#define PHYS_FLASH_SIZE		0x01000000	/* 16MB = 8MB X 2 */
#define CFG_MAX_FLASH_SECT	(512)		  /* max number of sectors on one chip */
/* addr of environment */

/* timeout values are in ticks */
#define CFG_FLASH_ERASE_TOUT	(3*CFG_HZ)	/* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT	(3*CFG_HZ)	/* Timeout for Flash Write */

/* #define ENV_IS_EMBEDDED 1 */
//#define CFG_NAND_BOOT
#define CFG_MAX_NAND_DEVICE 1	/* Max number of NAND devices */
#define NAND_ChipID_UNKNOWN	0x00
#define SECTORSIZE          512
#define NAND_MAX_FLOORS 1
#define NAND_MAX_CHIPS 1
/* ----------boot on nand flash---start---*/
#ifdef CFG_NAND_BOOT
#define CFG_ENV_IS_IN_NAND	1
#define CFG_ENV_BLOCK_OFFSET    0x1  /* environment starts on block ? (1~9) */
#define CFG_ENV_BLOCK_LENGTH    0x9  /* environment partition length 9 blocks */
/* ----------boot on nand flash---end-----*/
#else
#define CFG_ENV_IS_IN_FLASH	1
#if (NOR_BOOT_ERASE_SIZE_KB ==128)
	#define CFG_ENV_SIZE      0x20000     /* Total Size of Environment Sector */
	#define CFG_ENV_SECT_SIZE	0x20000     /* 64KB */
	#define CFG_ENV_OFFSET    0x00FA0000  /* environment starts here  */
#else
	#define CFG_ENV_SIZE      0x10000     /* Total Size of Environment Sector */
	#define CFG_ENV_SECT_SIZE	0x10000     /* 64KB */
	#define CFG_ENV_OFFSET    0x00FD0000  /* environment starts here  */
#endif
#define CFG_ENV_ADDR		(CFG_FLASH_BASE + CFG_ENV_OFFSET)
/* Address and size of Redundant Environment Sector */
#define CFG_ENV_ADDR_REDUND	(CFG_ENV_ADDR + CFG_ENV_SIZE)
#define CFG_ENV_SIZE_REDUND	(CFG_ENV_SIZE)
#endif

#define CONFIG_CRC32_VERIFY	1

/*
nand flash uboot info setting by Dannier Chen -start-
*/
#define REG_NFCR(nand_base_reg, NFCR0)         (REG32_PTR(__XDP_BASE_ADDR+NFCR0))

#define NAND_ENABLE_CE(nand) do \
{ \
	*(((volatile __u8 *)(nand->IO_ADDR)) + 0x44) = 0xFE; \
} while(0)

#define NAND_DISABLE_CE(nand) do \
{ \
	*(((volatile __u8 *)(nand->IO_ADDR)) + 0x44) = 0xFF; \
} while(0)

/*
nand flash uboot info setting by Dannier Chen -end-
*/

#define CFG_IDE_MAXBUS		1/* max. 1 IDE bus		*/
#define CFG_IDE_MAXDEVICE	1/* max. 1 drive per IDE bus	*/
/* Offset for normal register accesses	*/
#define CFG_ATA_REG_OFFSET	0
#define CFG_ATA_BASE_ADDR	0xD8008170

/* Offset for data I/O			*/
#define CFG_ATA_DATA_OFFSET	0
#define CFG_ATA_IDE0_OFFSET 0

#define CFG_HUSH_PARSER	1 /* use "hush" command parser */
#define CFG_PROMPT_HUSH_PS2 "> "

#define CONFIG_HARD_I2C			/* I2c with hardware support */
#define CFG_I2C_SPEED		400000	/* I2C speed and slave address */
#define CFG_I2C_SLAVE		0x7F

#endif	/* __CONFIG_H */
