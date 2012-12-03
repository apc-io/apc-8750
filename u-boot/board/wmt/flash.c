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
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * Some descriptions of such software. Copyright (c) 2008 WonderMedia Technologies, Inc.
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
#include <mpc8xx.h>
#include "flash/spi_flash.h"
#include "flash/nor_flash_8bit.h"
#include "flash/nor_flash_16bit.h"
/* #include "flash/nor_flash.h" */
#include "flash/nand_flash.h"
#include "include/extvars.h"

typedef enum _BOOT_FLASH_TYPE {
	NOR_FLASH_TYPE = 0,
	NAND_FLASH_TYPE ,
	SPI_FLASH_TYPE,
	MMC_FLASH_TYPE
} BOOT_FLASH_TYPE;

typedef enum _BOOT_BIT_TYPE {
	BOOT_TYPE_8BIT = 0,
	BOOT_TYPE_16BIT
} BOOT_BIT_TYPE;

BOOT_FLASH_TYPE flash_type = SPI_FLASH_TYPE;
BOOT_BIT_TYPE  nor_flash_bit = BOOT_TYPE_16BIT;
BOOT_BIT_TYPE  nand_flash_bit = BOOT_TYPE_16BIT;

flash_info_t flash_info_nor[CFG_MAX_NOR_FLASH_BANKS];
flash_info_t flash_info_spi[CFG_MAX_SPI_FLASH_BANKS];
flash_info_t flash_info_nand[CFG_MAX_NAND_FLASH_BANKS];

flash_info_t flash_info[CFG_MAX_FLASH_BANKS];
static unsigned int flash_count;
/*
 * get_boot_flash_type()
 */
BOOT_FLASH_TYPE get_boot_flash_type(void)
{
	unsigned int val = 0;
	unsigned char rc = SPI_FLASH_TYPE;

	val = *((volatile unsigned int *)(0xd8110100));
	if (chip_id == 0x3426) {
		switch ((val>>1) & 0x3) {
		case 0:
			rc = NOR_FLASH_TYPE;
			break;
		case 1:
			rc = NAND_FLASH_TYPE;
			break;
		case 2:
		case 3:
			rc = SPI_FLASH_TYPE;
			break;
		default:
			break;
		}
	} else if ((chip_id == 0x3437)||(chip_id == 0x3451)) {
		switch ((val>>1) & 0x1) {
		case 0:
			rc = SPI_FLASH_TYPE;
			break;
		case 1:
			rc = NAND_FLASH_TYPE;
			if ((((val>>19) & 0x1) == 0) && (chip_id == 0x3451))
				rc = NOR_FLASH_TYPE;
			break;
		default:
			break;
		}
	} else if (chip_id == 0x3445) {
		switch ((val>>1) & 0x3) {
		case 0:
			rc = SPI_FLASH_TYPE;
			break;
		case 1:
			rc = NAND_FLASH_TYPE;
			break;
		case 2:
			rc = NOR_FLASH_TYPE;
			break;
		case 3:
			rc = MMC_FLASH_TYPE;
			break;
		default:
			break;
		}
	}

	return rc;
}

/*
 * get_boot_nor_flash_bit_type()
 */
BOOT_BIT_TYPE get_boot_nor_flash_bit_type(void)
{
	unsigned int val = 0;
	unsigned char rc = BOOT_TYPE_16BIT;

	val = *((volatile unsigned int *)(0xd8110100));
	if (((val>>1)&0x3) == 2) {
		/* nor boot */
		switch ((val>>3)&0x1) {
		case 0:
			rc = BOOT_TYPE_8BIT;
			break;
		case 1:
			rc = BOOT_TYPE_16BIT;
			break;
		default:
			break;
		}
	}/*sf boot only nand active, nor don't need to init*/
	// else if (((val>>1)&0x3) == 0) {
		/* sf boot, nand active */
		/*switch ((val>>3)&0x1) {
		case 0:
			rc = BOOT_TYPE_8BIT;
			*(volatile unsigned int *)0xd8009400 &= ~1;
			*(volatile unsigned int *)0xd8009410 &= ~1;
			*(volatile unsigned int *)0xd8009420 &= ~1;
			*(volatile unsigned int *)0xd8009430 &= ~1;
			break;
		case 1:
		case 2:
			rc = BOOT_TYPE_16BIT;
			*(volatile unsigned int *)0xd8009400 |= 1;
			*(volatile unsigned int *)0xd8009410 |= 1;
			*(volatile unsigned int *)0xd8009420 |= 1;
			*(volatile unsigned int *)0xd8009430 |= 1;
			nor_flash_16b_init();
			break;
		default:
			break;
		}
	}*/
	return rc;
}

/*
 * get_boot_nand_flash_bit_type()
 */
BOOT_BIT_TYPE get_boot_nand_flash_bit_type(void)
{
	unsigned int val = 0;
	unsigned char rc = BOOT_TYPE_16BIT;

	val = *((volatile unsigned int *)(0xd8110100));

	if ((chip_id == 0x3426) || (chip_id == 0x3445))
		val = (val >> 3) & 0x1;
	else
		val = (val >> 2) & 0x1;

	switch (val) {
	case 0:
		rc = BOOT_TYPE_8BIT;
		break;
	case 1:
		rc = BOOT_TYPE_16BIT;
		break;
	default:
		break;
	}

	return rc;
}

static void print_flash_addr(flash_info_t *info)
{
	flash_info_t *p = NULL;
	int i;

	if (!flash_count)
		return;
	printf("flash:\n");
	for (i = 0, p = info; flash_count != 0; p++, i++, flash_count--) {
		printf("     Bank%d: %08lX -- %08lX", i+1, p->start[0], p->start[0]+p->size-1);

		if (p->flash_id == FLASH_UNKNOWN)
			printf(" (Missing or Unknown Flash)");
		printf("\n");
	}
}

/*
 * flash_init()
 *
 * sets up flash_info and returns size of FLASH (bytes)
 */
unsigned long flash_init(void)
{
	unsigned long rc = 0;
	int i;

	flash_type = get_boot_flash_type();
	nor_flash_bit = get_boot_nor_flash_bit_type();
	nand_flash_bit = get_boot_nand_flash_bit_type();
	flash_count = 0;
	/* because FLASH_UNKNOWN = 0XFFFffffF, SO WE USE FF INITIAL */
	memset(flash_info, 0xff, CFG_MAX_FLASH_BANKS * sizeof(flash_info_t));
	switch (flash_type) {
	case NOR_FLASH_TYPE:
		printf("boot from nor flash: ");
		flash_count = CFG_MAX_NOR_FLASH_BANKS;
		switch (nor_flash_bit) {
		case BOOT_TYPE_8BIT:
			printf("8bit mode.\n");
			rc = nor_flash_8b_init();
			break;
		case BOOT_TYPE_16BIT:
			printf("16bit mode.\n");
			rc = nor_flash_16b_init();
			break;
		}
		/* rc = nor_flash_init(); */
		for (i = 0; i < CFG_MAX_NOR_FLASH_BANKS; i++)
			memcpy(&flash_info[i], &flash_info_nor[i], sizeof(flash_info_t));
		break;
	case NAND_FLASH_TYPE:
		printf("boot from nand flash.\n");
		flash_count = CFG_MAX_NAND_FLASH_BANKS;
		rc = nand_flash_init();
		for (i = 0; i < CFG_MAX_NAND_FLASH_BANKS; i++)
			memcpy(&flash_info[i], &flash_info_nand[i], sizeof(flash_info_t));
		break;
	case SPI_FLASH_TYPE:
		printf("boot from spi flash.\n");
		flash_count = CFG_MAX_SPI_FLASH_BANKS;
		rc = spi_flash_init();
		for (i = 0; i < CFG_MAX_SPI_FLASH_BANKS; i++)
			memcpy(&flash_info[i], &flash_info_spi[i], sizeof(flash_info_t));
		break;
	default:
		break;
	}
	print_flash_addr(flash_info);

	return rc;
}

/*
 */
void flash_print_info(flash_info_t *info)
{
	switch (flash_type) {
	case NOR_FLASH_TYPE:
		switch (nor_flash_bit) {
		case BOOT_TYPE_8BIT:
			nor_flash_8b_print_info(info);
			break;
		case BOOT_TYPE_16BIT:
			nor_flash_16b_print_info(info);
			break;
		}

		/* nor_flash_print_info(info); */
		break;
	case NAND_FLASH_TYPE:
		nand_flash_print_info(info);
		break;
	case SPI_FLASH_TYPE:
		spi_flash_print_info(info);
		break;
	default:
		break;
	}
}

/*
 * flash_erase
 */
int	flash_erase(flash_info_t *info, int s_first, int s_last)
{
	int rc = 0;

	switch (flash_type) {
	case NOR_FLASH_TYPE:
		switch (nor_flash_bit) {
		case BOOT_TYPE_8BIT:
			rc = nor_flash_8b_erase(info, s_first, s_last);
			break;
		case BOOT_TYPE_16BIT:
			rc = nor_flash_16b_erase(info, s_first, s_last);
			break;
		}

		/* rc = nor_flash_erase(info, s_first, s_last); */
		break;
	case NAND_FLASH_TYPE:
		rc = nand_flash_erase(info, s_first, s_last);
		break;
	case SPI_FLASH_TYPE:
    /* sf boot, nand active */
		rc = spi_flash_erase(info, s_first, s_last);
		break;
	default:
		break;
	}

	return rc;
}

/*
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
int write_buff(flash_info_t *info, uchar *src, ulong addr, ulong cnt)
{
	int rc = 0;

	switch (flash_type) {
	case NOR_FLASH_TYPE:
		switch (nor_flash_bit) {
		case BOOT_TYPE_8BIT:
			rc = nor_flash_8b_write_buff(info, src, addr, cnt);
			break;
		case BOOT_TYPE_16BIT:
			rc = nor_flash_16b_write_buff(info, src, addr, cnt);
			break;
		}

		/* rc = nor_write_buff(info, src, addr, cnt); */
		break;
	case NAND_FLASH_TYPE:
		rc = nand_write_buff(info, src, addr, cnt);
		break;
	case SPI_FLASH_TYPE:
    /* sf boot, nand active */
		rc = spi_write_buff(info, src, addr, cnt);
		break;
	default:
		break;
	}

	return rc;
}
