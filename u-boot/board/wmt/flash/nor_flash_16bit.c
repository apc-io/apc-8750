/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

#include <common.h>
#include <linux/byteorder/swab.h>
#include <mpc8xx.h>
#include "nor_flash_16bit.h"

#define CONFIG_EON_FLASH 1
#define NOR_FLASH_TYPE 2
#define SPI_FLASH_TYPE 0

/* NOTE - CONFIG_FLASH_16BIT means the CPU interface is 16-bit, it
 *        has nothing to do with the flash chip being 8-bit or 16-bit.
 */
#define CONFIG_FLASH_16BIT 1
#ifdef CONFIG_FLASH_8BIT
	typedef unsigned char FLASH_PORT_WIDTH;
	typedef volatile unsigned char FLASH_PORT_WIDTHV;
	#define	FLASH_ID_MASK	0xFF
#else
#ifdef CONFIG_FLASH_16BIT
	typedef unsigned short FLASH_PORT_WIDTH;
	typedef volatile unsigned short FLASH_PORT_WIDTHV;
	#define SWAP(x)               __swab16(x)
	#define	FLASH_ID_MASK	0xFFFF
#else
	typedef unsigned long FLASH_PORT_WIDTH;
	typedef volatile unsigned long FLASH_PORT_WIDTHV;
	#define SWAP(x)               __swab32(x)
	#define	FLASH_ID_MASK	0xFFFFFFFF
#endif
#endif

#define FPW	FLASH_PORT_WIDTH
#define FPWV	FLASH_PORT_WIDTHV

#define ORMASK(size) ((-size) & OR_AM_MSK)

extern flash_info_t	flash_info_nor[CFG_MAX_NOR_FLASH_BANKS]; /* info for FLASH chips	*/

/*-----------------------------------------------------------------------
 * Functions
 */
static ulong nor_flash_get_size(FPWV *addr, flash_info_t *info);
static void flash_reset(flash_info_t *info);
static int write_word_intel(flash_info_t *info, FPWV *dest, FPW data);
static int write_word_amd(flash_info_t *info, FPWV *dest, FPW data);
static int write_word_eon(flash_info_t *info, FPWV *dest, FPW data);
static int write_word_mx(flash_info_t *info, FPWV *dest, FPW data);
static void flash_get_offsets(ulong base, flash_info_t *info);
#ifdef CFG_FLASH_PROTECTION
static void flash_sync_real_protect(flash_info_t *info);
#endif



/*-----------------------------------------------------------------------
 * nor_flash_init()
 *
 * sets up flash_info and returns size of FLASH (bytes)
 */
unsigned long nor_flash_16b_init(void)
{
	unsigned long size_b, size = 0;
	int i;
	unsigned int val = 0;
	unsigned long flash_base[4];
	val = *((volatile unsigned int *)(0xd8110100));
	if (((val>>1)&0x3) == NOR_FLASH_TYPE) {
		printf("nor boot only nor active\n");
		flash_base[0] = CFG_NOR_FLASH_BASE_0;
		flash_base[1] = CFG_NOR_FLASH_BASE_1;
		flash_base[2] = CFG_NOR_FLASH_BASE_2;
		flash_base[3] = CFG_NOR_FLASH_BASE_3;
		#if (NOR_BOOT_ERASE_SIZE_KB == 128)
		*(volatile unsigned int *)0xd8009460 = CFG_NOR_FLASH_BASE_0 | NOR_FLASH_32M;
		*(volatile unsigned int *)0xd8009464 = CFG_NOR_FLASH_BASE_1 | NOR_FLASH_32M;
		*(volatile unsigned int *)0xd8009468 = CFG_NOR_FLASH_BASE_2 | NOR_FLASH_32M;
		*(volatile unsigned int *)0xd800946c = CFG_NOR_FLASH_BASE_3 | NOR_FLASH_32M;
		#else
		*(volatile unsigned int *)0xd8009460 = CFG_NOR_FLASH_BASE_0 | NOR_FLASH_8M;
		*(volatile unsigned int *)0xd8009464 = CFG_NOR_FLASH_BASE_1 | NOR_FLASH_8M;
		*(volatile unsigned int *)0xd8009468 = CFG_NOR_FLASH_BASE_2 | NOR_FLASH_8M;
		*(volatile unsigned int *)0xd800946c = CFG_NOR_FLASH_BASE_3 | NOR_FLASH_8M;
		#endif
		/*Configure the NOR Flash clock setting*/
		*((volatile unsigned int *)(0xd8009408)) &= 0xffffff07;
		 *((volatile unsigned int *)(0xd8009408)) |= (0x1F<<3);//1F
	  	*((volatile unsigned int *)(0xd8009418)) &= 0xffffff07;
	 	*((volatile unsigned int *)(0xd8009418)) |= (0x1F<<3);
	  	*((volatile unsigned int *)(0xd8009428)) &= 0xffffff07;
	 	*((volatile unsigned int *)(0xd8009428)) |= (0x1F<<3);
	  	*((volatile unsigned int *)(0xd8009438)) &= 0xffffff07;
	 	*((volatile unsigned int *)(0xd8009438)) |= (0x1F<<3);
	} else if (((val>>1)&0x3) == SPI_FLASH_TYPE) {
	printf("spi boot, nor NOT active\n");
		/*flash_base[0] = SPI_BOOT_FLASH_BASE_0;
		flash_base[1] = SPI_BOOT_FLASH_BASE_1;
		flash_base[2] = SPI_BOOT_FLASH_BASE_2;
		flash_base[3] = SPI_BOOT_FLASH_BASE_3;
		*(volatile unsigned int *)0xd8009460 = SPI_BOOT_FLASH_BASE_0 | NOR_FLASH_8M;
		*(volatile unsigned int *)0xd8009464 = SPI_BOOT_FLASH_BASE_1 | NOR_FLASH_8M;
		*(volatile unsigned int *)0xd8009468 = SPI_BOOT_FLASH_BASE_2 | NOR_FLASH_8M;
		*(volatile unsigned int *)0xd800946c = SPI_BOOT_FLASH_BASE_3 | NOR_FLASH_8M;*/
	}

	/* Init: no FLASHes known */
	for (i = 0; i < CFG_MAX_NOR_FLASH_BANKS; ++i) {
		flash_info_nor[i].flash_id = FLASH_UNKNOWN;

		size_b = nor_flash_get_size((FPWV *)(flash_base[i]), &flash_info_nor[i]);
		flash_info_nor[i].size = size_b;
		size += size_b;

		if (flash_info_nor[i].flash_id == FLASH_UNKNOWN)
			printf("## Unknown FLASH on Bank %d - Size = 0x%08lx\n", i, size_b);

		/* Do this again (was done already in flast_get_size), just
		 * in case we move it when remap the FLASH.
		 */
		/* flash_get_offsets (flash_base[i], &flash_info_nor[i]); */
	}
#ifdef CFG_FLASH_PROTECTION
	/* read the hardware protection status (if any) into the
	 * protection array in flash_info.
	 */
	flash_sync_real_protect(&flash_info_nor[0]);
#endif

#if CFG_MONITOR_BASE >= CFG_NOR_FLASH_BASE_3
	/* monitor protection ON by default */
	flash_protect(
		FLAG_PROTECT_SET,
		CFG_MONITOR_BASE,
		CFG_MONITOR_BASE+CFG_MONITOR_LEN-1,
		&flash_info_nor[3]
	);
#endif

	return size;
}

/*-----------------------------------------------------------------------
 */
static void flash_reset(flash_info_t *info)
{
	FPWV *base = (FPWV *)(info->start[0]);

	/* Put FLASH back in read mode */
	if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_INTEL)
		*base = (FPW)0x00FF00FF;	/* Intel Read Mode */
	else if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_AMD)
		*base = (FPW)0x00F000F0;	/* AMD Read Mode */
	else if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_EON)
		*base = (FPW)0x00F000F0;	/* EON Read Mode */
	else if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_MX)
		*base = (FPW)0x00F000F0;	/* MX Read Mode */
  else if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_NUM)
		*base = (FPW)0x00F000F0;
	else
		*base = (FPW)0x00F000F0;
}

/*-----------------------------------------------------------------------
 */
static void flash_get_offsets(ulong base, flash_info_t *info)
{
	int i;

	/* set up sector start address table */
	if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_INTEL
	    && (info->flash_id & FLASH_BTYPE)) {
		int bootsect_size;	/* number of bytes/boot sector	*/
		int sect_size;		/* number of bytes/regular sector */

		bootsect_size = 0x00002000 * (sizeof(FPW)/2);
		sect_size =     0x00010000 * (sizeof(FPW)/2);

		/* set sector offsets for bottom boot block type	*/
		for (i = 0; i < 8; ++i)
			info->start[i] = base + (i * bootsect_size);

		for (i = 8; i < info->sector_count; i++)
			info->start[i] = base + ((i - 7) * sect_size);
	} else if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_AMD
		 && (info->flash_id & FLASH_TYPEMASK) == FLASH_AM640U) {

		int sect_size;		/* number of bytes/sector */

		sect_size = 0x00010000 * (sizeof(FPW)/2);

		/* set up sector start address table (uniform sector type) */
		for (i = 0; i < info->sector_count; i++)
			info->start[i] = base + (i * sect_size);
	} else if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_EON
		 && (info->flash_id & FLASH_TYPEMASK) == FLASH_EON29LV640U) {

		int sect_size;		/* number of bytes/sector */
#ifdef CONFIG_FLASH_8BIT
		sect_size = 0x00008000;
#else
		sect_size = 0x00010000 * (sizeof(FPW)/2);
#endif
		/* set up sector start address table (uniform sector type) */
		for (i = 0; i < info->sector_count; i++) {
			/* info->start[i] = base + (i * sect_size); */
			if (i < 127) {
				info->start[i] = base + (i * 0x10000);
				info->protect[i] = 0;
			} else {
				info->start[i] = base + 127 * 0x10000 + ((i-127) * 0x2000);
				info->protect[i] = 0;
			}
		}
	} else if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_MX
		 && (info->flash_id & FLASH_TYPEMASK) == FLASH_MX29LV640T) {

		/* set up sector start address table (uniform sector type) */
		for (i = 0; i < info->sector_count; i++) {
			if (i < 127) {
				info->start[i] = base + (i * 0x10000);
				info->protect[i] = 0;
			} else {
				info->start[i] = base + 127 * 0x10000 + ((i-127) * 0x2000);
				info->protect[i] = 0;
			}
		}
	} else if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_EON
		 && (info->flash_id & FLASH_TYPEMASK) == FLASH_EON29LV640B) {

		/* set up sector start address table (uniform sector type) */
		for (i = 0; i < info->sector_count; i++) {
			if (i < 8) {
				info->start[i] = base + (i * 0x2000);
				info->protect[i] = 0;
			} else {
				info->start[i] = base + 8 * 0x2000 + ((i-8) * 0x10000);
				info->protect[i] = 0;
			}
		}
	} else if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_MX
		 && (info->flash_id & FLASH_TYPEMASK) == FLASH_MX29LV640B) {

		/* set up sector start address table (uniform sector type) */
		for (i = 0; i < info->sector_count; i++) {
			if (i < 8) {
				info->start[i] = base + (i * 0x2000);
				info->protect[i] = 0;
			} else {
				info->start[i] = base + 8 * 0x2000 + ((i-8) * 0x10000);
				info->protect[i] = 0;
			}
		}
	} else if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_MX
		 && (info->flash_id & FLASH_TYPEMASK) == FLASH_MX29GL256E) {

		/* set up sector start address table (uniform sector type) */
		for (i = 0; i < info->sector_count; i++) {
				info->start[i] = base + (i * 0x20000);
				info->protect[i] = 0;
		}
	} else if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_EON
	 && (info->flash_id & FLASH_TYPEMASK) == FLASH_EON29LV640H) {
			int sect_size;		/* number of bytes/sector */
#ifdef CONFIG_FLASH_8BIT
		sect_size = 0x00008000;
#else
		sect_size = 0x00010000 * (sizeof(FPW)/2);
#endif
		for (i = 0; i < info->sector_count; i++)
			info->start[i] = base + (i * 0x10000);
	} else if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_NUM
	 && (info->flash_id & FLASH_TYPEMASK) == FLASH_NUM29W640GT) {

		for (i = 0; i < info->sector_count; i++) {
			if (i < 127) {
				info->start[i] = base + (i * 0x10000);
				info->protect[i] = 0;
			} else {
				info->start[i] = base + 127 * 0x10000 + ((i-127) * 0x2000);
				info->protect[i] = 0;
			}
		}
	} else if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_NUM
	 && (info->flash_id & FLASH_TYPEMASK) == FLASH_NUM29W640GB) {

		for (i = 0; i < info->sector_count; i++) {
			if (i < 8) {
				info->start[i] = base + (i * 0x2000);
				info->protect[i] = 0;
			} else {
				info->start[i] = base + 8 * 0x2000 + ((i-8) * 0x10000);
				info->protect[i] = 0;
			}
		}
	} else if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_NUM
	 && (info->flash_id & FLASH_TYPEMASK) == FLASH_NUM29W640GHL) {
		for (i = 0; i < info->sector_count; i++)
			info->start[i] = base + (i * 0x10000);
	} else if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_NUM
	 && (info->flash_id & FLASH_TYPEMASK) == FLASH_NUM29W128GHL) {
		for (i = 0; i < info->sector_count; i++)
			info->start[i] = base + (i * 0x20000);
	} else
		printf("set flash sector fail\n");
}

/*-----------------------------------------------------------------------
 */

void nor_flash_16b_print_info(flash_info_t *info)
{
	int i;
	uchar *boottype;
	uchar *bootletter;
	char *fmt;
	uchar botbootletter[] = "B";
	uchar topbootletter[] = "T";
	uchar botboottype[] = "bottom boot sector";
	uchar topboottype[] = "top boot sector";

	if (info->flash_id == FLASH_UNKNOWN) {
		printf("missing or unknown FLASH type\n");
		return;
	}

	switch (info->flash_id & FLASH_VENDMASK) {
	case FLASH_MAN_AMD:
		printf("AMD ");
		break;
	case FLASH_MAN_EON:
		printf("EON ");
	case FLASH_MAN_MX:
		printf("MX ");
		break;
	case FLASH_MAN_BM:
		printf("BRIGHT MICRO ");
		break;
	case FLASH_MAN_FUJ:
		printf("FUJITSU ");
		break;
	case FLASH_MAN_SST:
		printf("SST ");
		break;
	case FLASH_MAN_STM:
		printf("STM ");
		break;
	case FLASH_MAN_INTEL:
		printf("INTEL ");
		break;
	case FLASH_MAN_NUM:
		printf("NUM ");
		break;
	default:
		printf("Unknown Vendor ");
		break;
	}

	/* check for top or bottom boot, if it applies */
	if (info->flash_id & FLASH_BTYPE) {
		boottype = botboottype;
		bootletter = botbootletter;
	} else {
		boottype = topboottype;
		bootletter = topbootletter;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_AM640U:
		fmt = "29LV641D (64 Mbit, uniform sectors)\n";
		break;
	case FLASH_EON29LV640H:
#ifdef CONFIG_FLASH_8BIT
		fmt = "29LV640H (32 Mbit, uniform sectors)\n";
#else
		fmt = "29LV640H (64 Mbit, uniform sectors)\n";
#endif
		break;
	case FLASH_EON29LV640U:
	case FLASH_EON29LV640B:
#ifdef CONFIG_FLASH_8BIT
		fmt = "29LV641D (32 Mbit, uniform sectors)\n";
#else
		fmt = "29LV641D (64 Mbit, uniform sectors)\n";
#endif
		break;
	case FLASH_28F800C3B:
	case FLASH_28F800C3T:
		fmt = "28F800C3%s (8 Mbit, %s)\n";
		break;
	case FLASH_INTEL800B:
	case FLASH_INTEL800T:
		fmt = "28F800B3%s (8 Mbit, %s)\n";
		break;
	case FLASH_28F160C3B:
	case FLASH_28F160C3T:
		fmt = "28F160C3%s (16 Mbit, %s)\n";
		break;
	case FLASH_INTEL160B:
	case FLASH_INTEL160T:
		fmt = "28F160B3%s (16 Mbit, %s)\n";
		break;
	case FLASH_28F320C3B:
	case FLASH_28F320C3T:
		fmt = "28F320C3%s (32 Mbit, %s)\n";
		break;
	case FLASH_INTEL320B:
	case FLASH_INTEL320T:
		fmt = "28F320B3%s (32 Mbit, %s)\n";
		break;
	case FLASH_28F640C3B:
	case FLASH_28F640C3T:
		fmt = "28F640C3%s (64 Mbit, %s)\n";
		break;
	case FLASH_INTEL640B:
	case FLASH_INTEL640T:
		fmt = "28F640B3%s (64 Mbit, %s)\n";
		break;
	case FLASH_MX29LV640B:
	case FLASH_MX29LV640T:
		fmt = "29LV640D (64 Mbit, uniform sectors)\n";
		break;
	case FLASH_MX29GL256E:
		fmt = "29GL256E (256 Mbit, uniform sectors)\n";
		break;
	case FLASH_NUM29W640GT:
	case FLASH_NUM29W640GB:
	case FLASH_NUM29W640GHL:
		fmt = "29W640G (64 Mbit, uniform sectors)\n";
		break;
	case FLASH_NUM29W128GHL:
		fmt = "29W128GL (128 Mbit, uniform sectors)\n";
		break;
	default:
		fmt = "Unknown Chip Type\n";
		break;
	}

	printf(fmt, bootletter, boottype);

	printf("  Size: %ld MB in %d Sectors\n",
		info->size >> 20,
		info->sector_count);

	printf("  Sector Start Addresses:");

	for (i = 0; i < info->sector_count; ++i) {
		if ((i % 5) == 0)
			printf("\n   ");

		printf(" %08lX%s", info->start[i],
			info->protect[i] ? " (RO)" : "     ");
	}

	printf("\n");
}

/*-----------------------------------------------------------------------
 */

/*
 * The following code cannot be run from FLASH!
 */

static ulong nor_flash_get_size(FPWV *addr, flash_info_t *info)
{
	/* Write auto select command: read Manufacturer ID */
	int DevId, DevId1;
	/* Write auto select command sequence and test FLASH answer */
#ifdef	CONFIG_EON_FLASH
	addr[0x0555] = (FPW)0x00AA00AA;		/* for EON, Intel ignores this */
	addr[0x02AA] = (FPW)0x00550055;		/* for EON, Intel ignores this */
#endif
	addr[0x0555] = (FPW)0x00900090;		/* selects Intel or AMD */

	/* The manufacturer codes are only 1 byte, so just use 1 byte.
	 * This works for any bus width and any FLASH device width.
	 */
	DevId = NOR_IDALL((addr[0]&0xFFFF), (addr[1]&0xFFFF));
	DevId1 = NOR_IDALL((addr[14]&0xFFFF), (addr[15]&0xFFFF));
	printf("DevId = 0x%x\n", DevId);
	printf("DevId1 = 0x%x\n", DevId1);
	switch (addr[0] & 0xff) {

	case (uchar)AMD_MANUFACT:
		info->flash_id = FLASH_MAN_AMD;
		break;

	case (uchar)INTEL_MANUFACT:
		info->flash_id = FLASH_MAN_INTEL;
		break;

	case (uchar)EON_MANUFACT:
		info->flash_id = FLASH_MAN_EON;
		break;
	case (uchar)MX_MANUFACT:
		info->flash_id = FLASH_MAN_MX;
		break;
	case (uchar)NUM_MANUFACT:
		info->flash_id = FLASH_MAN_NUM;
		break;
	default:
		info->flash_id = FLASH_UNKNOWN;
		info->sector_count = 0;
		info->size = 0;
		break;
	}

	/* Check 16 bits or 32 bits of ID so work on 32 or 16 bit bus. */
	if (info->flash_id != FLASH_UNKNOWN)
		switch (addr[1]) {
		/* case (FPW)AMD_ID_LV640U: */ /* 29LV640 and 29LV641 have same ID */
		/* info->flash_id += FLASH_AM640U; */
		/* info->sector_count = 128; */
		/* info->size = 0x00800000 * (sizeof(FPW)/2); */
		/* break; */ /* => 8 or 16 MB	*/

		case (FPW)EON_ID_LV640U:
			/* 29LV640 and 29LV641 have same ID */
			info->flash_id += FLASH_EON29LV640U;
			info->sector_count = 128;
#ifdef CONFIG_FLASH_8BIT
			info->size = 0x00400000;
#else
			info->size = 0x00800000 * (sizeof(FPW)/2);
#endif

			break;
		case (FPW)EON_ID_LV640B:
			/* 29LV640 and 29LV641 have same ID */
			if(info->flash_id == FLASH_MAN_MX)
				info->flash_id += FLASH_MX29LV640B;
			else
				info->flash_id += FLASH_EON29LV640B;
			info->sector_count = 135;
			info->size = 0x00800000 * (sizeof(FPW)/2);
			break;			
		case (FPW)EON_ID_LV640H:
			if(info->flash_id == FLASH_MAN_NUM) {
				info->sector_count = 128;
				info->size = 0x0800000;
				switch (DevId1) {
					case NX_M29W640GT:
						info->flash_id += FLASH_NUM29W640GT;
						info->sector_count = 135;
						break;
					case NX_M29W640GB:
						info->flash_id += FLASH_NUM29W640GB;
						info->sector_count = 135;
						break;
					case NX_M29W640GH:
					case NX_M29W640GL:
						info->flash_id += FLASH_NUM29W640GHL;
						break;
					case NX_M29W128GH:
					case NX_M29W128GL:
						info->flash_id += FLASH_NUM29W128GHL;
						info->size = 0x01000000;
						break;
					default:
						info->flash_id += FLASH_NUM29W640GHL;
						break;
				}
			} else if(info->flash_id == FLASH_MAN_EON) {
				info->flash_id += FLASH_EON29LV640H;
				info->sector_count = 128;
				info->size = 0x00800000;
				//info->size = 0x00800000 * (sizeof(FPW)/2);
				break;
			} else if(info->flash_id == FLASH_MAN_MX) {
				info->flash_id += FLASH_MX29GL256E;
				info->sector_count = 256;
				info->size = 0x02000000;
			}
			break;
		case (FPW)EON_ID_LV640T:
			/* 29LV640 and 29LV641 have same ID */
			if(info->flash_id == FLASH_MAN_MX)
				info->flash_id += FLASH_MX29LV640T;
			else
				info->flash_id += FLASH_EON29LV640U;
			info->sector_count = 135;
			info->size = 0x00800000 * (sizeof(FPW)/2);

			break;

		case (FPW)INTEL_ID_28F800C3B:
			info->flash_id += FLASH_28F800C3B;
			info->sector_count = 23;
			info->size = 0x00100000 * (sizeof(FPW)/2);
			break;				/* => 1 or 2 MB		*/

		case (FPW)INTEL_ID_28F800B3B:
			info->flash_id += FLASH_INTEL800B;
			info->sector_count = 23;
			info->size = 0x00100000 * (sizeof(FPW)/2);
			break;				/* => 1 or 2 MB		*/

		case (FPW)INTEL_ID_28F160C3B:
			info->flash_id += FLASH_28F160C3B;
			info->sector_count = 39;
			info->size = 0x00200000 * (sizeof(FPW)/2);
			break;				/* => 2 or 4 MB		*/

		case (FPW)INTEL_ID_28F160B3B:
			info->flash_id += FLASH_INTEL160B;
			info->sector_count = 39;
			info->size = 0x00200000 * (sizeof(FPW)/2);
			break;				/* => 2 or 4 MB		*/

		case (FPW)INTEL_ID_28F320C3B:
			info->flash_id += FLASH_28F320C3B;
			info->sector_count = 71;
			info->size = 0x00400000 * (sizeof(FPW)/2);
			break;				/* => 4 or 8 MB		*/

		case (FPW)INTEL_ID_28F320B3B:
			info->flash_id += FLASH_INTEL320B;
			info->sector_count = 71;
			info->size = 0x00400000 * (sizeof(FPW)/2);
			break;				/* => 4 or 8 MB		*/

		case (FPW)INTEL_ID_28F640C3B:
			info->flash_id += FLASH_28F640C3B;
			info->sector_count = 135;
			info->size = 0x00800000 * (sizeof(FPW)/2);
			break;				/* => 8 or 16 MB	*/

		case (FPW)INTEL_ID_28F640B3B:
			info->flash_id += FLASH_INTEL640B;
			info->sector_count = 135;
			info->size = 0x00800000 * (sizeof(FPW)/2);
			break;				/* => 8 or 16 MB	*/

		/*case (FPW)MX_ID_LV640B:
			info->flash_id += FLASH_MX29LV640B;
			info->sector_count = 135;
			info->size = 0x00800000 * (sizeof(FPW)/2);
			break;

		case (FPW)MX_ID_LV640T:
			info->flash_id += FLASH_MX29LV640B;
			info->sector_count = 135;
			info->size = 0x00800000 * (sizeof(FPW)/2);
			break;*/

		default:
			info->flash_id = FLASH_UNKNOWN;
			info->sector_count = 0;
			info->size = 0;
			return 0;			/* => no or unknown flash */
		}

	flash_get_offsets((ulong)addr, info);

	/* Put FLASH back in read mode */
	flash_reset(info);

	return info->size;
}

#ifdef CFG_FLASH_PROTECTION
/*-----------------------------------------------------------------------
 */

static void flash_sync_real_protect(flash_info_t *info)
{
	FPWV *addr = (FPWV *)(info->start[0]);
	FPWV *sect;
	int i;

	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_28F800C3B:
	case FLASH_28F800C3T:
	case FLASH_28F160C3B:
	case FLASH_28F160C3T:
	case FLASH_28F320C3B:
	case FLASH_28F320C3T:
	case FLASH_28F640C3B:
	case FLASH_28F640C3T:
		/* check for protected sectors */
		*addr = (FPW)0x00900090;
		for (i = 0; i < info->sector_count; i++) {
			/* read sector protection at sector address, (A7 .. A0) = 0x02.
			* D0 = 1 for each device if protected.
			* If at least one device is protected the sector is marked
			* protected, but mixed protected and  unprotected devices
			* within a sector should never happen.
			*/
			sect = (FPWV *)(info->start[i]);
			info->protect[i] = (sect[2] & (FPW)(0x00010001)) ? 1 : 0;
		}

		/* Put FLASH back in read mode */
		flash_reset(info);
		break;
	case FLASH_EON29LV640H:
	case FLASH_EON29LV640U:
	case FLASH_EON29LV640B:
		for (i = 0; i < info->sector_count; i++) {
			/* check for protected sectors */
			sect = (FPWV *)(info->start[i]);
			sect[0x0555] = (FPW)0x00AA00AA;
			sect[0x02AA] = (FPW)0x00550055;
			sect[0x0555] = (FPW)0x00900090;

			info->protect[i] = (sect[2] & (FPW)(0x00010001)) ? 1 : 0;
		}
		/* Put FLASH back in read mode */
		flash_reset(info);
		break;
	case FLASH_MX29LV640T:
	case FLASH_MX29LV640B:
	case FLASH_NUM29W640GT:
	case FLASH_NUM29W640GB:
	case FLASH_NUM29W640GHL:
	case FLASH_NUM29W128GHL:
		for (i = 0; i < info->sector_count; i++) {
			/* check for protected sectors */
			sect = (FPWV *)(info->start[i]);
			sect[0x0555] = (FPW)0x00AA00AA;
			sect[0x02AA] = (FPW)0x00550055;
			sect[0x0555] = (FPW)0x00900090;

			info->protect[i] = (sect[2] & (FPW)(0x00010001)) ? 1 : 0;
		}
		/* Put FLASH back in read mode */
		flash_reset(info);
		break;
	case FLASH_AM640U:
	default:
		/* no hardware protect that we support */
		break;
	}
}
#endif

/*-----------------------------------------------------------------------
 */

int	nor_flash_16b_erase(flash_info_t *info, int s_first, int s_last)
{
	FPWV *addr;
	int flag, prot, sect;
	int intel = (info->flash_id & FLASH_VENDMASK) == FLASH_MAN_INTEL;
	ulong start, now, last;
	int rcode = 0;
	if ((s_first < 0) || (s_first > s_last)) {
		if (info->flash_id == FLASH_UNKNOWN)
			printf("- missing\n");
		else
			printf("- no sectors to erase\n");
		return 1;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_INTEL800B:
	case FLASH_INTEL160B:
	case FLASH_INTEL320B:
	case FLASH_INTEL640B:
	case FLASH_28F800C3B:
	case FLASH_28F160C3B:
	case FLASH_28F320C3B:
	case FLASH_28F640C3B:
	case FLASH_AM640U:
	case FLASH_EON29LV640H:
	case FLASH_EON29LV640U:
	case FLASH_EON29LV640B:
	case FLASH_MX29LV640B:
	case FLASH_MX29LV640T:
	case FLASH_MX29GL256E:
	case FLASH_NUM29W640GT:
	case FLASH_NUM29W640GB:
	case FLASH_NUM29W640GHL:
	case FLASH_NUM29W128GHL:
		break;
	case FLASH_UNKNOWN:
	default:
		printf("Can't erase unknown flash type %08lx - aborted\n",
			info->flash_id);
		return 1;
	}

	prot = 0;
	for (sect = s_first; sect <= s_last; ++sect) {
		if (info->protect[sect])
			prot++;
	}

	if (prot)
		printf("- Warning: %d protected sectors will not be erased!\n", prot);
	else
		printf("\n");

	/* start = get_timer(0); */
	/* last  = start; */

	/* Start erase on unprotected sectors */
	for (sect = s_first; sect <= s_last && rcode == 0; sect++) {

		if (info->protect[sect] != 0)	/* protected, skip it */
			continue;
		start = get_timer(0);
		last  = start;


		/* Disable interrupts which might cause a timeout here */
		flag = disable_interrupts();

		addr = (FPWV *)(info->start[sect]);
		if (intel) {
			*addr = (FPW)0x00500050; 	/* clear status register */
			*addr = (FPW)0x00200020; 	/* erase setup */
			*addr = (FPW)0x00D000D0;	/* erase confirm */
		} else {
			/* must be AMD style if not Intel */
			FPWV *base;		/* first address in bank */
			base = (FPWV *)(info->start[0]);
			base[0x0555] = (FPW)0x00AA00AA;	/* unlock */
			base[0x02AA] = (FPW)0x00550055;	/* unlock */
			base[0x0555] = (FPW)0x00800080;	/* erase mode */
			base[0x0555] = (FPW)0x00AA00AA;	/* unlock */
			base[0x02AA] = (FPW)0x00550055;	/* unlock */
			*addr = (FPW)0x00300030;	/* erase sector */
		}

		/* re-enable interrupts if necessary */
		if (flag)
			enable_interrupts();

		/* wait at least 50us for AMD, 80us for Intel.
		 * Let's wait 1 ms.
		 */
		udelay(1000);

		while ((*addr & (FPW)0x00800080) != (FPW)0x00800080) {
			now = get_timer(start);
			if (now > CFG_FLASH_ERASE_TOUT) {
				printf("Timeout\n");

				if (intel) {
					/* suspend erase	*/
					*addr = (FPW)0x00B000B0;
				}

				flash_reset(info);	/* reset to read mode */
				rcode = 1;		/* failed */
				break;
			}

			/* show that we're waiting */
			if ((now - last) > 1000) {	/* every second */
				putc('.');
				last = now;
			}
		}

		flash_reset(info);	/* reset to read mode	*/
	}

	printf(" done\n");
	return rcode;
}

/*-----------------------------------------------------------------------
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
int nor_flash_16b_write_buff(flash_info_t *info, uchar *src, ulong addr, ulong cnt)
{
	FPW data = 0; /* 16 or 32 bit word, matches flash bus width on MPC8XX */
	int bytes;	  /* number of bytes to program in current word		*/
	int left;	  /* number of bytes left to program			*/
	int i, res;

	for (left = cnt, res = 0;
		left > 0 && res == 0;
		addr += sizeof(data), left -= sizeof(data) - bytes) {

		bytes = addr & (sizeof(data) - 1);
		addr &= ~(sizeof(data) - 1);

		/* combine source and destination data so can program
		 * an entire word of 16 or 32 bits
		 */
		for (i = 0; i < sizeof(data); i++) {
			data <<= 8;
			if (i < bytes || i - bytes >= left)
				data += *((uchar *)addr + i);
			else
				data += *src++;
		}

		/* write one word to the flash */
		switch (info->flash_id & FLASH_VENDMASK) {
		case FLASH_MAN_AMD:
			res = write_word_amd(info, (FPWV *)addr, data);
		case FLASH_MAN_EON:
		case FLASH_MAN_NUM:
			res = write_word_eon(info, (FPWV *)addr, SWAP(data));
			break;
		case FLASH_MAN_MX:
			res = write_word_mx(info, (FPWV *)addr, SWAP(data));
			break;
		case FLASH_MAN_INTEL:
			res = write_word_intel(info, (FPWV *)addr, data);
			break;
		default:
			/* unknown flash type, error! */
			printf("missing or unknown FLASH type\n");
			res = 1;	/* not really a timeout, but gives error */
			break;
		}
	}

	return res;
}

/*-----------------------------------------------------------------------
 * Write a word to Flash for AMD FLASH
 * A word is 16 or 32 bits, whichever the bus width of the flash bank
 * (not an individual chip) is.
 *
 * returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
static int write_word_amd(flash_info_t *info, FPWV *dest, FPW data)
{
	ulong start;
	int flag;
	int res = 0;	/* result, assume success	*/
	FPWV *base;		/* first address in flash bank	*/

	/* Check if Flash is (sufficiently) erased */
	if ((*dest & data) != data)
		return 2;

	base = (FPWV *)(info->start[0]);

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	base[0x0555] = (FPW)0x00AA00AA;	/* unlock */
	base[0x02AA] = (FPW)0x00550055;	/* unlock */
	base[0x0555] = (FPW)0x00A000A0;	/* selects program mode */

	*dest = data;		/* start programming the data	*/

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts();

	start = get_timer(0);

	/* data polling for D7 */
	while (res == 0 && (*dest & (FPW)0x00800080) != (data & (FPW)0x00800080)) {
		if (get_timer(start) > CFG_FLASH_WRITE_TOUT) {
			*dest = (FPW)0x00F000F0;	/* reset bank */
			res = 1;
		}
	}

	return res;
}

/*-----------------------------------------------------------------------
 * Write a word to Flash for EON FLASH
 * A word is 16 or 32 bits, whichever the bus width of the flash bank
 * (not an individual chip) is.
 *
 * returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
static int write_word_eon(flash_info_t *info, FPWV *dest, FPW data)
{
	ulong start;
	int flag;
	int res = 0;	/* result, assume success	*/
	FPWV *base;		/* first address in flash bank	*/

	/* Check if Flash is (sufficiently) erased */
	if ((*dest & data) != data)
		return 2;

	base = (FPWV *)(info->start[0]);

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	base[0x0555] = (FPW)0x00AA00AA;	/* unlock */
	base[0x02AA] = (FPW)0x00550055;	/* unlock */
	base[0x0555] = (FPW)0x00A000A0;	/* selects program mode */

	*dest = data;		/* start programming the data	*/

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts();

	start = get_timer(0);

	/* data polling for D7 */
	while (res == 0 && (*dest & (FPW)0x00800080) != (data & (FPW)0x00800080)) {
		if (get_timer(start) > CFG_FLASH_WRITE_TOUT) {
			*dest = (FPW)0x00F000F0;	/* reset bank */
			res = 1;
		}
	}

	return res;
}

/*-----------------------------------------------------------------------
 * Write a word to Flash for MX FLASH
 * A word is 16 or 32 bits, whichever the bus width of the flash bank
 * (not an individual chip) is.
 *
 * returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
static int write_word_mx(flash_info_t *info, FPWV *dest, FPW data)
{
	ulong start;
	int flag;
	int res = 0;	/* result, assume success	*/
	FPWV *base;		/* first address in flash bank	*/

	/* Check if Flash is (sufficiently) erased */
	if ((*dest & data) != data)
		return 2;

	base = (FPWV *)(info->start[0]);

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	base[0x0555] = (FPW)0x00AA00AA;	/* unlock */
	base[0x02AA] = (FPW)0x00550055;	/* unlock */
	base[0x0555] = (FPW)0x00A000A0;	/* selects program mode */

	*dest = data;		/* start programming the data	*/

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts();

	start = get_timer(0);

	/* data polling for D7 */
	while (res == 0 && (*dest & (FPW)0x00800080) != (data & (FPW)0x00800080)) {
		if (get_timer(start) > CFG_FLASH_WRITE_TOUT) {
			*dest = (FPW)0x00F000F0;	/* reset bank */
			res = 1;
		}
	}

	return res;
}
/*-----------------------------------------------------------------------
 * Write a word to Flash for Intel FLASH
 * A word is 16 or 32 bits, whichever the bus width of the flash bank
 * (not an individual chip) is.
 *
 * returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
static int write_word_intel(flash_info_t *info, FPWV *dest, FPW data)
{
	ulong start;
	int flag;
	int res = 0;	/* result, assume success	*/

	/* Check if Flash is (sufficiently) erased */
	if ((*dest & data) != data)
		return 2;

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	*dest = (FPW)0x00500050;	/* clear status register	*/
	*dest = (FPW)0x00FF00FF;	/* make sure in read mode	*/
	*dest = (FPW)0x00400040;	/* program setup		*/

	*dest = data;		/* start programming the data	*/

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts();

	start = get_timer(0);

	while (res == 0 && (*dest & (FPW)0x00800080) != (FPW)0x00800080) {
		if (get_timer(start) > CFG_FLASH_WRITE_TOUT) {
			*dest = (FPW)0x00B000B0;	/* Suspend program	*/
			res = 1;
		}
	}

	if (res == 0 && (*dest & (FPW)0x00100010))
		res = 1;	/* write failed, time out error is close enough	*/

	*dest = (FPW)0x00500050;	/* clear status register	*/
	*dest = (FPW)0x00FF00FF;	/* make sure in read mode	*/

	return res;
}

#ifdef CFG_FLASH_PROTECTION
/*-----------------------------------------------------------------------
 */
int nor_flash_16b_real_protect(flash_info_t *info, long sector, int prot)
{
	int rcode = 0;		/* assume success */
	FPWV *addr;		/* address of sector */
	FPW value;

	addr = (FPWV *) (info->start[sector]);

	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_28F800C3B:
	case FLASH_28F800C3T:
	case FLASH_28F160C3B:
	case FLASH_28F160C3T:
	case FLASH_28F320C3B:
	case FLASH_28F320C3T:
	case FLASH_28F640C3B:
	case FLASH_28F640C3T:
		flash_reset(info);		/* make sure in read mode */
		*addr = (FPW) 0x00600060L;	/* lock command setup */
		if (prot)
			*addr = (FPW) 0x00010001L;	/* lock sector */
		else
			*addr = (FPW) 0x00D000D0L;	/* unlock sector */
		flash_reset(info);		/* reset to read mode */

		/* now see if it really is locked/unlocked as requested */
		*addr = (FPW) 0x00900090;
		/* read sector protection at sector address, (A7 .. A0) = 0x02.
		 * D0 = 1 for each device if protected.
		 * If at least one device is protected the sector is marked
		 * protected, but return failure. Mixed protected and
		 * unprotected devices within a sector should never happen.
		 */
		value = addr[2] & (FPW) 0x00010001;
		if (value == 0)
			info->protect[sector] = 0;
		else if (value == (FPW) 0x00010001)
			info->protect[sector] = 1;
		else {
			/* error, mixed protected and unprotected */
			rcode = 1;
			info->protect[sector] = 1;
		}
		if (info->protect[sector] != prot)
			rcode = 1;	/* failed to protect/unprotect as requested */

		/* reload all protection bits from hardware for now */
		flash_sync_real_protect(info);
		break;

	case FLASH_EON29LV640H:
	case FLASH_EON29LV640U:
	case FLASH_EON29LV640B:
	case FLASH_MX29LV640B:
	case FLASH_MX29LV640T:
	case FLASH_MX29GL256E:
	case FLASH_NUM29W640GT:
	case FLASH_NUM29W640GB:
	case FLASH_NUM29W640GHL:
	case FLASH_NUM29W128GHL:
		if (prot == 0)
			info->protect[sector] = 0;
		else if (prot == 1)
			info->protect[sector] = 1;
		else {
			/* error, mixed protected and unprotected */
			rcode = 1;
			info->protect[sector] = 1;
		}
		if (info->protect[sector] != prot)
			rcode = 1;	/* failed to protect/unprotect as requested */

		break;

	case FLASH_AM640U:
	default:
		/* no hardware protect that we support */
		info->protect[sector] = prot;
		break;
	}

	return rcode;
}
#endif
