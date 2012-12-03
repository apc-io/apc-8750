/*++
linux/drivers/mtd/nand/wmt_nand.c

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

#include <linux/config.h>
#include <linux/module.h>
/*#include <linux/types.h>*/
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/string.h>
#include <linux/ioport.h>
/*#include <linux/platform_device.h>*/
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>
/*#include <linux/clk.h>*/

#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>
/*#include <linux/mtd/partitions.h>*/
#include <linux/interrupt.h>
#include <linux/completion.h>
#include <linux/reboot.h> //Lch
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/dma.h>
#include <asm/sizes.h>
#include <mach/irqs.h>
#include <mach/hardware.h>
#include "wmt_nand.h"


#ifndef memzero
#define memzero(s, n)     memset ((s), 0, (n))
#endif
/*
* NAND_PAGE_SIZE = 2048 or 4096:
* support Harming ECC and BCH ECC
*
* NAND_PAGE_SIZE = 512:
* Only support Harming ECC according to the demand of filesystem
* so need open macro:
* #define NAND_HARMING_ECC
*
*/

/*#define NAND_DEBUG*/
unsigned int wmt_version;
#ifdef CONFIG_MTD_PARTITIONS
#include <linux/mtd/partitions.h>

struct mtd_partition nand_partitions[] = {
	{
		.name		= "w-load-NAND",
		.offset		= 0x00000000,
		.size		= 0x00020000,
	},
	{
		.name		= "u-boot env. cfg. 1-NAND",
		.offset		= MTDPART_OFS_APPEND,
		.size		= 0x00120000,
	},
	{
		.name		= "u-boot-NAND",
		.offset		= MTDPART_OFS_APPEND,
		.size		= 0x00120000,
	},
	{
		.name		= "kernel-NAND",
		.offset		= MTDPART_OFS_APPEND,
		.size		= 0x00800000,
	},
	{
		.name		= "initrd-NAND",
		.offset		= MTDPART_OFS_APPEND,
		.size		= 0x00580000,
	},
	{
		.name		= "filesystem-NAND",
		.offset		= MTDPART_OFS_APPEND,
		.size		= 0x10000000,
	},
	{
		.name		= "u-boot-logo",
		.offset		= MTDPART_OFS_APPEND,
		.size		= 0x200000,
	},
	{
		.name		= "kernel-logo",
		.offset		= MTDPART_OFS_APPEND,
		.size		= 0x400000,
	},
	{
		.name		= "android-data",
		.offset		= MTDPART_OFS_APPEND,
		.size		= 0x40000000,
	},
	{
		.name		= "android-cache",
		.offset		= MTDPART_OFS_APPEND,
		.size		= 0x8000000,
	},
	{
		.name		= "LocalDisk",
		.offset		= MTDPART_OFS_APPEND,
		.size		= MTDPART_SIZ_FULL,
	},
};
EXPORT_SYMBOL(nand_partitions);
#endif

#ifdef CONFIG_MTD_NAND_WMT_HWECC

	static int MAX_CHIP = CONFIG_MTD_NAND_CHIP_NUM;
	static int hardware_ecc = 1;

	#if (CONFIG_MTD_NAND_HM_ECC == 1)
		#define NAND_HARMING_ECC
	#else
		#undef NAND_HARMING_ECC
	#endif

	#if 0
		#ifdef CONFIG_MTD_NAND_WMT_HW_BCH_ECC
		static int eccmode = 1;    /* BCH ECC */
		#else
		static int eccmode = 0;    /* Harming ECC */
		#endif
	#endif

#else		/* if CONFIG_MTD_NAND_WMT_HWECC else */
	#define NAND_HARMING_ECC
	#define MAX_CHIP 1
	static int hardware_ecc = 0;
#endif

/* dannier mask: if(page size == 512) then define PAGE_ADDR
 * so, in driver, when we see ifndef PAGE_ADDR then replace
 * the code with if(mtd->writesize != 512) 2009/03/16
 */
/*
#ifdef (NAND_PAGE_SIZE == 512)
#define PAGE_ADDR  // support sequential read
#ifdef PAGE_ADDR
	#undef PAGE_READ_COUNTER
#else
	#define PAGE_READ_COUNTER  //  read 256 bytes twice
#endif
#endif
*/


/*
*  ecc_type = 0 : Harming ECC
*  ecc_type = 1 : BCH ECC
*/
#ifndef NAND_HARMING_ECC
	static int ecc_type  = 1;
#else
	static int ecc_type  = 0;
#endif

#ifndef NAND_HARMING_ECC
	/*static int nandtype = 1;*/    /* support nand new data structure */
#endif


/*
 * hardware specific Out Of Band information
*/

/*
* new oob placement block for use with hardware ecc generation
*/

static struct nand_ecclayout wmt_oobinfo_2048 = {
/* nand flash new structure and BCH ECC oob info */
	.eccbytes = 7,
	.eccpos = { 24, 25, 26, 27, 28, 29, 30},
	.oobavail = 24,
	.oobfree = {{0, 24} }
};


static struct nand_ecclayout wmt_12bit_oobinfo_4096 = {
/* nand flash old structure and Harming ECC oob info */
	.eccbytes = 20,
	.eccpos = { 24, 25, 26, 27, 28, 29, 30,
							31, 32, 33, 34, 35, 36, 37,
							38, 39, 40, 41, 42, 43},
	.oobavail = 24,
	.oobfree = {{0, 24} }
};
/*
static struct nand_ecclayout wmt_8bit_oobinfo_4096 = {
// nand flash old structure and Harming ECC oob info
	.eccbytes = 13,
	.eccpos = { 24, 25, 26, 27, 28, 29, 30,
							31, 32, 33, 34, 35, 36},
	.oobavail = 24,
	.oobfree = {{0, 24} }
};
static struct nand_ecclayout wmt_oobinfo_8192 = {
// nand flash new structure and Harming ECC oob info
	.eccbytes = 42,
	.eccpos = { 24, 25, 26, 27, 28, 29, 30,
							31, 32, 33, 34, 35, 36, 37,
							38, 39, 40, 41, 42, 43, 44,
							45, 46, 47, 48, 49, 50, 51,
							52, 53, 54, 55, 56, 57, 58,
							59, 60, 61, 62, 63, 64, 65},
	.oobavail = 24,
	.oobfree = {{0, 24} }
};*/

static struct nand_ecclayout wmt_oobinfo_4096 = {
/* nand flash old structure and Harming ECC oob info */
	.eccbytes = 7,
	.eccpos = { 24, 25, 26, 27, 28, 29, 30},
	.oobavail = 24,
	.oobfree = {{0, 24} }
};
#ifdef NAND_HARMING_ECC
	static struct nand_ecclayout wmt_hm_oobinfo_2048 = {
	/*  nand flash old structure and Harming ECC oob info */
		.eccbytes = 14,
		.eccpos = { 32, 33, 34, 36, 37, 38, 40, 41, 42, 44, 45, 46, 48, 49},
		.oobavail = 32,
		.oobfree = {{0, 32} }
	};


	static struct nand_ecclayout wmt_hm_oobinfo_4096 = {
		/*  nand flash old structure and Harming ECC oob info */
		.eccbytes = 27,
		.eccpos = {
			64, 65, 66, 68, 69, 70, 72, 73, 74, 76, 77, 78,
			80, 81, 82, 84, 85, 86, 88, 89, 90, 92, 93, 94,
			96, 97, 98},
		.oobavail = 64,
		.oobfree = {{0, 64} }
	};
#endif

	static struct nand_ecclayout wmt_oobinfo_512 = {
	#if 0
		/* #if eccmode */   /*  nand flash new structure and BCH ECC oob info */
		.eccbytes = 10,
		.eccpos = { 0, 1, 2, 3, 4, 5, 6, 13, 14, 15},
		.oobfree = {{7, 6} }
		/* #else */        /*  nand flash old structure and Harming ECC oob info */
	#endif
	    .eccbytes = 8,
	    .eccpos = { 4, 5, 6, 8, 9, 10, 12, 13},
	    .oobfree = {{0, 4},{7, 1},{11,1},{14,2}}
		 /* #endif */
	};

/*#if (NAND_PAGE_SIZE == 2048)
EXPORT_SYMBOL(wmt_oobinfo_2048);
EXPORT_SYMBOL(wmt_hm_oobinfo_2048);
#else
 #if (NAND_PAGE_SIZE == 4096)
 EXPORT_SYMBOL(wmt_oobinfo_4096);
 EXPORT_SYMBOL(wmt_hm_oobinfo_4096);
 #endif
#endif*/
/* Ick. The BBT code really ought to be able to work this bit out
	 for itself from the above, at least for the 2KiB case
*/
#if 1
	static uint8_t wmt_bbt_pattern_2048[] = { 'B', 'b', 't', '0' };
	static uint8_t wmt_mirror_pattern_2048[] = { '1', 't', 'b', 'B' };
#endif
/*static uint8_t wmt_bbt_pattern_2048[] = { 0xFF, 0xFF };*/
/*static uint8_t wmt_mirror_pattern_2048[] = { 0xFF, 0xFF };*/


/*static uint8_t wmt_bbt_pattern_512[] = { 0xBB };*/
/*static uint8_t wmt_mirror_pattern_512[] = { 0xBC };*/

spinlock_t *nand_lock;

static struct nand_bbt_descr wmt_bbt_main_descr_2048 = {
	.options = NAND_BBT_LASTBLOCK | NAND_BBT_CREATE | NAND_BBT_WRITE
		| NAND_BBT_2BIT | NAND_BBT_VERSION | NAND_BBT_PERCHIP,
	.offs =	4,
	.len = 4,
	.veroffs = 0,
	.maxblocks = 4,
	.pattern = wmt_bbt_pattern_2048,
};

static struct nand_bbt_descr wmt_bbt_mirror_descr_2048 = {
	.options = NAND_BBT_LASTBLOCK | NAND_BBT_CREATE | NAND_BBT_WRITE
		| NAND_BBT_2BIT | NAND_BBT_VERSION | NAND_BBT_PERCHIP,
	.offs =	4,
	.len = 4,
	.veroffs = 0,
	.maxblocks = 4,
	.pattern = wmt_mirror_pattern_2048,
};

static struct nand_bbt_descr wmt_bbt_main_descr_512 = {
	.options = NAND_BBT_LASTBLOCK | NAND_BBT_CREATE | NAND_BBT_WRITE
		| NAND_BBT_2BIT | NAND_BBT_VERSION | NAND_BBT_PERCHIP,
	.offs =	0,
	.len = 4,
	.veroffs = 14,
	.maxblocks = 4,
	.pattern = wmt_bbt_pattern_2048,
};

static struct nand_bbt_descr wmt_bbt_mirror_descr_512 = {
	.options = NAND_BBT_LASTBLOCK | NAND_BBT_CREATE | NAND_BBT_WRITE
		| NAND_BBT_2BIT | NAND_BBT_VERSION | NAND_BBT_PERCHIP,
	.offs =	0,
	.len = 4,
	.veroffs = 14,
	.maxblocks = 4,
	.pattern = wmt_mirror_pattern_2048,
};


/* controller and mtd information */

/*static void print_register(struct mtd_info *mtd);*/

struct wmt_nand_set {
	int	nr_chips;
	int	nr_partitions;
	char *name;
	int *nr_map;
	struct mtd_partition *partitions;
};

#if 0
	struct wmt_platform_nand {
		/* timing information for controller, all times in nanoseconds */

		int	tacls;	/* time for active CLE/ALE to nWE/nOE */
		int	twrph0;	/* active time for nWE/nOE */
		int	twrph1;	/* time for release CLE/ALE from nWE/nOE inactive */

		int nr_sets;
		struct wmt_nand_set *sets;
		void (*select_chip)(struct s3c2410_nand_set *, int chip);
	}
#endif

struct wmt_nand_info;

struct wmt_nand_mtd {
	struct mtd_info	mtd;
	struct nand_chip chip;
	/*struct wmt_nand_set* set;*/
	struct wmt_nand_info *info;
	int	scan_res;
};

/* overview of the wmt nand state */

struct wmt_nand_info {
	/* mtd info */
	struct nand_hw_control controller;
	struct wmt_nand_mtd *mtds;
	struct wmt_platform_nand *platform;

	/* device info */
	struct device *device;
	struct resource *area;
	void __iomem *reg;
	int cpu_type;
	int datalen;
	int nr_data;
	int data_pos;
	int page_addr;
	dma_addr_t dmaaddr;
	int dma_finish;
	int phase;
	void *done_data;	/* completion	data */
	unsigned int isr_cmd;
	//void (*done)(void *data);/*	completion function	*/
	unsigned char *dmabuf;
};

/* conversion functions */

static struct wmt_nand_mtd *wmt_nand_mtd_toours(struct mtd_info *mtd)
{
	return container_of(mtd, struct wmt_nand_mtd, mtd);
}

static struct wmt_nand_info *wmt_nand_mtd_toinfo(struct mtd_info *mtd)
{
	return wmt_nand_mtd_toours(mtd)->info;
}

/*
static struct wmt_nand_info *to_nand_info(struct platform_device *dev)
{
	return platform_get_drvdata(dev);
}
*/
/*
static struct platform_device *to_platform(struct device *dev)
{
	return container_of(dev, struct platform_device, dev);
}
*/
#if 0
static struct wmt_platform_nand *to_nand_plat(struct platform_device *dev)
{
	return dev->dev.platform_data;
}
#endif

/*
 * type : HW or SW ECc
 *
*/
static void nfc_ecc_set(struct wmt_nand_info *info, int type)
{
/* struct wmt_nand_info *info = wmt_nand_mtd_toinfo(mtd);*/

	if (type == hardware_ecc)
		writeb(readb(info->reg + WMT_NFC_MISC_CTRL) & 0xfb, info->reg + WMT_NFC_MISC_CTRL);
	else
		writeb(readb(info->reg + WMT_NFC_MISC_CTRL) | 0x04, info->reg + WMT_NFC_MISC_CTRL);
}

void copy_filename (char *dst, char *src, int size)
{
	if (*src && (*src == '"')) {
		++src;
		--size;
	}

	while ((--size > 0) && *src && (*src != '"')) {
		*dst++ = *src++;
	}
	*dst = '\0';
}

/*
 * Get the flash and manufacturer id and lookup if the type is supported
 */
static struct WMT_nand_flash_dev *get_flash_type(struct mtd_info *mtd,
						  struct nand_chip *chip,
						  int busw, int *maf_id, int CE, unsigned int *spec_clk)
{
	struct WMT_nand_flash_dev *type = NULL, type_env;
	int i, dev_id, maf_idx, ret = 0;
	unsigned int id = 0;
	char varval[200], *s = NULL, *tmp, varname[] = "wmt.io.nand";
	unsigned int varlen = 200, value;

	/* Select the device */
	chip->select_chip(mtd, CE);

	/*  reset test: edwardwan add for debug 20071229  start*/
	chip->cmdfunc(mtd, NAND_CMD_RESET, -1, -1);
	/*  reset test: edwardwan add for debug 20071229  end*/

	/* Send the command for reading device ID */
	chip->cmdfunc(mtd, NAND_CMD_READID, 0x00, -1);

	/* Read manufacturer and device IDs */
	*maf_id = chip->read_byte(mtd);
	for (i = 0; i < 3; i++) {
		dev_id = chip->read_byte(mtd);
		id += ((unsigned char)dev_id) <<((2-i)*8);
	}
	#ifdef NAND_DEBUG
	printk("nand chip device maf_id is %x, and dev_id is %x\n",*maf_id,dev_id);
	#endif

	/* Lookup the flash id */
	/*for (i = 0; nand_flash_ids[i].name != NULL; i++) {
		if (dev_id == nand_flash_ids[i].id) {
			type =  &nand_flash_ids[i];*/
	for (i = 0; WMT_nand_flash_ids[i].dwFlashID != 0; i++) {
		if (((unsigned int)id + ((*maf_id)<<24)) == WMT_nand_flash_ids[i].dwFlashID) {
			type =  &WMT_nand_flash_ids[i];
			/*printk("find nand chip device id i=%d\n", i);*/
			break;
		}
	}
	value = *((volatile unsigned int *)(GPIO_BASE_ADDR + 0x100));
	if (!((value&6) == 2))
		ret = wmt_getsyspara(varname, varval, &varlen);
	if (!ret && !((value&6) == 2)) {
		s = varval;
		value = simple_strtoul(s, &tmp, 16);
		type_env.dwFlashID = value; s = tmp+1;
		value = simple_strtoul(s, &tmp, 16);
		type_env.dwBlockCount = value; s = tmp+1;
		value = simple_strtoul(s, &tmp, 16);
		type_env.dwPageSize = value; s = tmp+1;
		value = simple_strtoul(s, &tmp, 16);
		type_env.dwSpareSize = value; s = tmp+1;
		value = simple_strtoul(s, &tmp, 16);
		type_env.dwBlockSize = value; s = tmp+1;
		value = simple_strtoul(s, &tmp, 16);
		type_env.dwAddressCycle = value; s = tmp+1;
		value = simple_strtoul(s, &tmp, 16);
		type_env.dwBI0Position = value; s = tmp+1;
		value = simple_strtoul(s, &tmp, 16);
		type_env.dwBI1Position = value; s = tmp+1;
		value = simple_strtoul(s, &tmp, 16);
		type_env.dwBIOffset = value; s = tmp+1;
		value = simple_strtoul(s, &tmp, 16);
		type_env.dwDataWidth = value; s = tmp+1;
		value = simple_strtoul(s, &tmp, 16);
		type_env.dwPageProgramLimit = value; s = tmp+1;
		value = simple_strtoul(s, &tmp, 16);
		type_env.dwSeqRowReadSupport = value; s = tmp+1;
		value = simple_strtoul(s, &tmp, 16);
		type_env.dwSeqPageProgram = value; s = tmp+1;
		value = simple_strtoul(s, &tmp, 16);
		type_env.dwNandType = value; s = tmp+1;
		value = simple_strtoul(s, &tmp, 16);
		type_env.dwRWTimming = value; s = tmp+1;
		copy_filename(type_env.ProductName, s, MAX_PRODUCT_NAME_LENGTH);

		if (type_env.dwBlockCount < 1024 || type_env.dwBlockCount > 8192) {
			printk(KERN_INFO "dwBlockCount = 0x%x is abnormal\n", type_env.dwBlockCount);
			goto list;
		}
		if (type_env.dwPageSize < 512  || type_env.dwPageSize > 8192) {
			printk(KERN_INFO "dwPageSize = 0x%x is abnormal\n", type_env.dwPageSize);
			goto list;
		}
		if (type_env.dwPageSize > 512)
			type_env.options = NAND_SAMSUNG_LP_OPTIONS | NAND_NO_READRDY | NAND_NO_AUTOINCR;
		if (type_env.dwBlockSize < (1024*64)  || type_env.dwBlockSize > (8192*64)) {
			printk(KERN_INFO "dwBlockSize = 0x%x is abnormal\n", type_env.dwBlockSize);
			goto list;
		}
		if (type_env.dwAddressCycle < 3  || type_env.dwAddressCycle > 5) {
			printk(KERN_INFO "dwAddressCycle = 0x%x is abnoraml\n", type_env.dwAddressCycle);
			goto list;
		}
		if (type_env.dwBI0Position != 0  &&
		type_env.dwBI0Position > ((type_env.dwBlockSize/type_env.dwPageSize)-1)) {
			printk(KERN_INFO "dwBI0Position = 0x%x is abnoraml\n", type_env.dwBI0Position);
			goto list;
		}
		if (type_env.dwBI1Position != 0  &&
		type_env.dwBI1Position > ((type_env.dwBlockSize/type_env.dwPageSize)-1)) {
			printk(KERN_INFO "dwBI1Position = 0x%x is abnoraml\n", type_env.dwBI1Position);
			goto list;
		}
		if (type_env.dwBIOffset != 0 && type_env.dwBIOffset != 5) {
			printk(KERN_INFO "dwBIOffset = 0x%x is abnoraml\n", type_env.dwBIOffset);
			goto list;
		}
		if (type_env.dwDataWidth != 0/* && type_env.dwDataWidth != 1*/) {
			printk(KERN_INFO "dwDataWidth = 0x%x is abnoraml\n", type_env.dwDataWidth);
			goto list;
		}
		/*printk(KERN_INFO "dwFlashID = 0x%x\n", type_env.dwFlashID);
		printk(KERN_INFO "dwBlockCount = 0x%x\n", type_env.dwBlockCount);
		printk(KERN_INFO "dwPageSize = 0x%x\n", type_env.dwPageSize);
		printk(KERN_INFO "dwSpareSize = 0x%x\n", type_env.dwSpareSize);
		printk(KERN_INFO "dwBlockSize = 0x%x\n", type_env.dwBlockSize);
		printk(KERN_INFO "dwAddressCycle = 0x%x\n", type_env.dwAddressCycle);
		printk(KERN_INFO "dwBI0Position = 0x%x\n", type_env.dwBI0Position);
		printk(KERN_INFO "dwBI1Position = 0x%x\n", type_env.dwBI1Position);
		printk(KERN_INFO "dwBIOffset = 0x%x\n", type_env.dwBIOffset);
		printk(KERN_INFO "dwDataWidth = 0x%x\n", type_env.dwDataWidth);
		printk(KERN_INFO "dwPageProgramLimit = 0x%x\n", type_env.dwPageProgramLimit);
		printk(KERN_INFO "dwSeqRowReadSupport = 0x%x\n", type_env.dwSeqRowReadSupport);
		printk(KERN_INFO "dwSeqPageProgram = 0x%x\n", type_env.dwSeqPageProgram);
		printk(KERN_INFO "dwNandType = 0x%x\n", type_env.dwNandType);
		printk(KERN_INFO "dwECCBitNum = 0x%x\n", type_env.dwECCBitNum);
		printk(KERN_INFO "dwRWTimming = 0x%x\n", type_env.dwRWTimming);
		printk(KERN_INFO "cProductName = %s\n", type_env.ProductName);*/
		if (((unsigned int)id + ((*maf_id)<<24)) == type_env.dwFlashID) {
			if (type)
				printk(KERN_INFO "flash id has been established in flash id list\n");
			type = &type_env;
		}
	}
list:

	if (!type) {
		/*printk("before ERR_PTR(-ENODEV)\n");*/
		return ERR_PTR(-ENODEV);
	}
	if (!mtd->name)
		/*mtd->name = type->name;*/
		/*mtd->name = type->ProductName;*/
		mtd->name = "WMT.nand";

	/*chip->chipsize = type->chipsize << 20;*/
	chip->chipsize = (uint64_t)type->dwBlockCount * (uint64_t)type->dwBlockSize;

	/* get all information from table */
	chip->cellinfo = type->dwNandType << 2;
	mtd->writesize = type->dwPageSize;
	mtd->oobsize = type->dwSpareSize;
	mtd->erasesize = type->dwBlockSize;
	busw = type->dwDataWidth ? NAND_BUSWIDTH_16 : 0;
	if (mtd->oobsize >= 218)
		ECC8BIT_ENGINE = 1;
	*spec_clk = type->dwRWTimming;

	#ifdef CONFIG_MTD_PARTITIONS
	/*if (((*(volatile unsigned long *)(GPIO_BASE_ADDR + 0x100))&2) == 2) {*/
	nand_partitions[0].size = type->dwBlockSize;	/*wloader-NAND*/
	nand_partitions[1].offset = type->dwBlockSize;/*u-boot-env*/
	nand_partitions[1].size = type->dwBlockSize * 9;
	nand_partitions[2].offset = type->dwBlockSize * 10;/*u-boot NAND*/
	nand_partitions[2].size = type->dwBlockSize * 9;
	nand_partitions[3].offset = type->dwBlockSize * 19;/*kernel-NAND*/
	nand_partitions[3].size = 0x800000;
	nand_partitions[4].offset = type->dwBlockSize * 19 + 0x800000;/*initrd-NAND*/
	nand_partitions[4].size = 0x600000;
	nand_partitions[5].offset = type->dwBlockSize * 19 + 0x800000 + 0x600000;/*filesystem-NAND*/
	nand_partitions[5].size = 0x10000000;
	nand_partitions[6].offset = type->dwBlockSize * 19 + 0x800000 + 0x600000 + 0x10000000;/*u-boot Logo*/
	nand_partitions[6].size = 0x200000;
	nand_partitions[7].offset = type->dwBlockSize * 19 +
	0x800000 + 0x600000 + 0x10000000 + 0x200000;/*kernel Logo*/
	nand_partitions[7].size = 0x400000;
	nand_partitions[8].offset = type->dwBlockSize * 19 +
	0x800000 + 0x600000 + 0x10000000 + 0x200000 + 0x400000;/*android-data*/
	nand_partitions[8].size = 0x40000000;
	nand_partitions[9].offset = type->dwBlockSize * 19 +
	0x800000 + 0x600000 + 0x10000000 + 0x200000 + 0x400000 + 0x40000000;/*android-cache*/
	nand_partitions[9].size = 0x8000000;
	nand_partitions[10].offset = type->dwBlockSize * 19 +
	0x800000 + 0x600000 + 0x10000000 + 0x200000 + 0x400000 + 0x40000000 + 0x8000000;/*LocalDisk*/
	if ((chip->chipsize>>30) >= 4)
		nand_partitions[10].size = 0x80000000;
	else
		nand_partitions[10].size = MTDPART_SIZ_FULL;/*chip->chipsize - (type->dwBlockSize * 19 +
	0x800000 + 0x600000 + 0x10000000 + 0x200000 + 0x400000 + 0x20000000 + 0x8000000);*/
	/*}*/
	#endif

#if 0
	/* Newer devices have all the information in additional id bytes */
	if (!type->pagesize) {
		int extid;
		/*printk("calculate nand chip device type\n");*/
		/* The 3rd id byte holds MLC / multichip data */
		chip->cellinfo = chip->read_byte(mtd);
		/* The 4th id byte is the important one */
		extid = chip->read_byte(mtd);
		/* Calc pagesize */
		mtd->writesize = 1024 << (extid & 0x3);
		extid >>= 2;
		/* Calc oobsize */
		/* DannierChen add support 4096+218 page size */
		if((extid&0x03) == 2) {
			mtd->oobsize = 218;
			ECC8BIT_ENGINE = 1;
		} else
			mtd->oobsize = (8 << (extid & 0x01)) * (mtd->writesize >> 9);
		extid >>= 2;
		/* Calc blocksize. Blocksize is multiples of 64KiB */
		mtd->erasesize = (64 * 1024) << (extid & 0x03);
		extid >>= 2;
		/* Get buswidth information */
		busw = (extid & 0x01) ? NAND_BUSWIDTH_16 : 0;

	} else {
		/*
		 * Old devices have chip data hardcoded in the device id table
		 */
		/*printk("find nand chip device id, and give device type\n");*/
		mtd->erasesize = type->erasesize;
		mtd->writesize = type->pagesize;
		mtd->oobsize = mtd->writesize / 32;
		busw = type->options & NAND_BUSWIDTH_16;
	}
#endif /* end of (#if 0) masked my Dannier*/

	/* Try to identify manufacturer */
	for (maf_idx = 0; nand_manuf_ids[maf_idx].id != 0x0; maf_idx++) {
		if (nand_manuf_ids[maf_idx].id == *maf_id)
			break;
	}

	/*
	 * Check, if buswidth is correct. Hardware drivers should set
	 * chip correct !
	 */
	if (busw != (chip->options & NAND_BUSWIDTH_16)) {
		printk(KERN_INFO "NAND device: Manufacturer ID:"
				" 0x%02x, Chip ID: 0x%02x (%s %s)\n", *maf_id,
				/*dev_id, nand_manuf_ids[maf_idx].name, mtd->name);*/
				id, nand_manuf_ids[maf_idx].name, mtd->name);
		printk(KERN_WARNING "NAND bus width %d instead %d bit\n",
				(chip->options & NAND_BUSWIDTH_16) ? 16 : 8,
				busw ? 16 : 8);
		return ERR_PTR(-EINVAL);
	}

	/* Calculate the address shift from the page size */
	chip->page_shift = ffs(mtd->writesize) - 1;
	/* Convert chipsize to number of pages per chip -1. */
	chip->pagemask = (chip->chipsize >> chip->page_shift) - 1;

	chip->bbt_erase_shift = chip->phys_erase_shift =
		ffs(mtd->erasesize) - 1;
	chip->chip_shift = ffs(chip->chipsize) - 1;

	/* Set the bad block position */
	chip->badblockpos = mtd->writesize > 512 ?
		NAND_LARGE_BADBLOCK_POS : NAND_SMALL_BADBLOCK_POS;

	/* Get chip options, preserve non chip based options */
	chip->options &= ~NAND_CHIPOPTIONS_MSK;
	chip->options |= type->options & NAND_CHIPOPTIONS_MSK;

	/*
	 * Set chip as a default. Board drivers can override it, if necessary
	 */
	chip->options |= NAND_NO_AUTOINCR;

	/* Check if chip is a not a samsung device. Do not clear the
	 * options for chips which are not having an extended id.
	 */
	/*if (*maf_id != NAND_MFR_SAMSUNG && !type->pagesize)*//* Dannier:to support new table*/
	if (*maf_id != NAND_MFR_SAMSUNG && type->dwPageSize > 512)
		chip->options &= ~NAND_SAMSUNG_LP_OPTIONS;

	/* Check for AND chips with 4 page planes */
	/*if (chip->options & NAND_4PAGE_ARRAY)
		chip->erase_cmd = multi_erase_cmd;
	else
		chip->erase_cmd = single_erase_cmd;*/

	/* Do not replace user supplied command function ! */
	/*if (mtd->writesize > 512 && chip->cmdfunc == nand_command)
		chip->cmdfunc = nand_command_lp;*/

	printk(KERN_INFO "NAND device: Manufacturer ID:"
			/*" 0x%02x, Chip ID: 0x%02x (%s %s)\n", *maf_id, dev_id,
			nand_manuf_ids[maf_idx].name, type->name);*/
			/* Dannier 2009-10-07: add to support new table*/
			" 0x%02x, Chip ID: 0x%02x (%s %s)\n", *maf_id, id,
			nand_manuf_ids[maf_idx].name, type->ProductName);

	return type;
}
static int wmt_calc_clock(struct mtd_info *mtd, unsigned int spec_clk, int *T, int *divisor, int *Thold)
{
	unsigned int i, div1=0, div2, clk1, clk2=0, comp, T1=0, T2=0, clk, PLLB;
	unsigned int tREA, tREH, Thold2, Ttmp, tADL, tWP;
	
	/*print_register(mtd);*/
	PLLB = *(volatile unsigned int *)PMPMB_ADDR;
PLLB = (((PLLB>>16)&0xFF)+1)/((((PLLB>>8)&0x3F)+1)*(1<<(PLLB&7)));
	printk(KERN_WARNING "PLLB=0x%x, spec_clk=0x%x\n", PLLB, spec_clk);
	tREA = (spec_clk>>24)&0xFF;
	tREH = (spec_clk>>16)&0xFF;
	tWP  = (spec_clk>>8)&0xFF;
	tADL = spec_clk&0xFF;
	for (i = 1; i < 16; i++) {
		if (MAX_SPEED_MHZ >= (PLLB*SOURCE_CLOCK)/i) {
			div1 = i;
			break;
		}
	}

	clk1 = (1000 * div1)/(PLLB*SOURCE_CLOCK);
	//printk("clk1=%d, div1=%d, spec_clk=%d\n", clk1, div1, spec_clk);
	for (T1 = 1; T1 < 10; T1++) {
		if ((T1*clk1) >= (tREA + MAX_READ_DELAY))
			break;
	}
	i = 1;
	while (i*clk1 <= tREH) {
		i++;
	}
	*Thold = i;
	printk("T1=%d, clk1=%d, div1=%d, Thold=%d, tREA=%d+delay(%d)\n", T1, clk1, div1, *Thold, tREA, MAX_READ_DELAY);
	Ttmp = *T = T1;
	clk = clk1;
	*divisor = div1;
	div2 = div1;
	while (Ttmp > 1 && clk != 0) {
		div2++;
		clk2 = (1000 * div2)/(PLLB*SOURCE_CLOCK);
		comp = 0;
		for (T2 = 1; T2 < Ttmp; T2++) {
			if ((T2*clk2) >= (tREA + MAX_READ_DELAY)) {
				Ttmp = T2;
				comp = 1;
				i = 1;
				while (i*clk2 <= tREH) {
					i++;
				}
				Thold2 = i;
				printk("T2=%d, clk2=%d, div2=%d, Thold2=%d, comp=1\n", T2, clk2, div2, Thold2);
				break;
			}
		}
		if (comp == 1) {
			clk1 = clk * (*T+*Thold) * mtd->writesize;
			div1 = clk2 * (T2+Thold2) * mtd->writesize;
			printk("Tim1=%d , Tim2=%d\n", clk1, div1);
			if ((clk * (*T+*Thold) * mtd->writesize) > (clk2 * (T2+Thold2) * mtd->writesize)) {
				*T = T2;
				clk = clk2;
				*divisor = div2;
				*Thold = Thold2;
			} else {
				printk("T2 is greater and not use\n");
			}
		}
	} /* end of while */
	//printk("Tadfasdfil\n");
	i = 1;
	*Thold |= 0x100; /* set write setup/hold time */
	while (((i*2+2)*clk) <= (tADL-tWP) || (i*clk) <= (tWP+MAX_WRITE_DELAY)) {
		*Thold += 0x100;
		i++;
	}
	/* set write hold time */
	/* tWP > tWH*/
	/*i = 1;
	*Thold |= 0x10000;
	while (((i*2+2)*clk) <= tADL || (i*clk) < tWP) {
		*Thold += 0x10000;
		i++;
	}*/
	
	printk("T=%d, clk=%d, divisor=%d, Thold=0x%x\n", *T, clk, *divisor, *Thold);
	if ((MAX_SPEED_MHZ < (PLLB*SOURCE_CLOCK)/(*divisor)) || clk == 0 || *T == 0 || clk > 45)
		return 1;

	return 0;
}

static int wmt_nfc_init(struct wmt_nand_info *info, struct mtd_info *mtd)
{
	int busw, nand_maf_id, page_per_block, CE = 0;
	unsigned int spec_clk, T, Thold, divisor, status = 0;
	struct nand_chip *chip = mtd->priv;
	struct WMT_nand_flash_dev *type;
page_per_block = *(volatile unsigned long *)PMNAND_ADDR;
	printk(KERN_WARNING "1PMNAND_ADDR=0x%x \n", page_per_block);
	/* Get buswidth to select the correct functions */
	busw = chip->options & NAND_BUSWIDTH_16;

	/*  struct wmt_nand_info *info = wmt_nand_mtd_toinfo(mtd);*/
	/* enable chip select */
	/* new structure  */

	/* old structure */
	/* writeb(PAGE_2K|WIDTH_8|WP_DISABLE|DIRECT_MAP|OLDDATA_EN,
	info->reg + WMT_NFC_NAND_TYPE_SEL);*/
	/* writeb((PAGE_2K|WIDTH_8|WP_DISABLE|DIRECT_MAP)&(~OLDDATA_EN),
	info->reg + WMT_NFC_NAND_TYPE_SEL);*/

	writeb((PAGE_2K|WIDTH_8|WP_DISABLE|DIRECT_MAP|CHECK_ALLFF)&(~OLDDATA_EN),
	info->reg + WMT_NFC_NAND_TYPE_SEL);
	
	writel(readl(info->reg + WMT_NFC_READ_CYCLE_PULE_CTRL) & 0xffff0000,
	info->reg + WMT_NFC_READ_CYCLE_PULE_CTRL);
	writel(readl(info->reg + WMT_NFC_READ_CYCLE_PULE_CTRL) | NFC_RWTimming,
	info->reg + WMT_NFC_READ_CYCLE_PULE_CTRL);
	
	writel(B2R,	info->reg + WMT_NFC_HOST_STAT_CHANGE);
	writel(0x0,	info->reg + WMT_NFC_CALC_CTRL);
	
	/* DannierChen add to identify the page size and block size */
	if (chip_swap == 1) {
		/*printk(KERN_WARNING "resume: set chip_swap to 0!\n");*/
		chip_swap = 0;
	}
	type = get_flash_type(mtd, chip, busw, &nand_maf_id, CE, &spec_clk);
	if (IS_ERR(type)) {
		CE++;
		type = get_flash_type(mtd, chip, busw, &nand_maf_id, CE, &spec_clk);
		if (IS_ERR(type)) {
			printk(KERN_WARNING "No NAND device found!!!\n");
			chip->select_chip(mtd, -1);
			if (nand_maf_id) {
				printk(KERN_WARNING "nand flash identify fail(unknow id), nand_maf_id=0x%x\n", nand_maf_id);
				/*return PTR_ERR(type);*/
			}
			return 2;
		} else {
			/*printk(KERN_WARNING "chip swap to chip 1!!!\n");*/
			chip_swap = 1;
		}
	}

	page_per_block = chip->phys_erase_shift - chip->page_shift;
	/*printk(KERN_INFO "page per block = 2^%x \n", page_per_block);*/
	switch (page_per_block) {
	case 4:
		writel(0x1F, info->reg + WMT_NFC_PAGESIZE_DIVIDER_SEL); /*32 page per block*/
	break;
	case 5:/*32 page per block*/
		writel((1 << 5) | 0x1F, info->reg + WMT_NFC_PAGESIZE_DIVIDER_SEL);
	break;
	case 6:/*64 page per block*/
		writel((2 << 5) | 0x1F, info->reg + WMT_NFC_PAGESIZE_DIVIDER_SEL);
	break;
	case 7:/*128 page per block*/
		writel((3 << 5) | 0x1F, info->reg + WMT_NFC_PAGESIZE_DIVIDER_SEL);
	break;
	case 8:/*256 page per block*/
		writel((4 << 5) | 0x1F, info->reg + WMT_NFC_PAGESIZE_DIVIDER_SEL);
	break;
	case 9:/*512 page per block*/
		writel((5 << 5) | 0x1F, info->reg + WMT_NFC_PAGESIZE_DIVIDER_SEL);
	break;
	default:
		printk(KERN_ERR "page per block not support\n");
	break;
	}

	#ifdef NAND_DEBUG
	printk(KERN_ERR "mtd->writesize = %x \n", mtd->writesize);
	#endif
	if (mtd->writesize == 2048) {
		/* page size == 2048*/
		#ifndef NAND_HARMING_ECC
			writeb((PAGE_2K|WIDTH_8|WP_DISABLE|DIRECT_MAP|CHECK_ALLFF)&(~OLDDATA_EN),
			info->reg + WMT_NFC_NAND_TYPE_SEL);
		#else
			if (ecc_type == 1)
				writeb((PAGE_2K|WIDTH_8|WP_DISABLE|DIRECT_MAP|CHECK_ALLFF) & (~OLDDATA_EN),
				info->reg + WMT_NFC_NAND_TYPE_SEL);
			else
				writeb(PAGE_2K|WIDTH_8|WP_DISABLE|DIRECT_MAP|CHECK_ALLFF|OLDDATA_EN,
				info->reg + WMT_NFC_NAND_TYPE_SEL);/*old structure*/
		#endif
	/*} else if (mtd->writesize == 4096) { To support read page twice*/
	} else if (mtd->writesize == 4096 || mtd->writesize == 8192) {
		/* page size == 4096*/
		#ifndef NAND_HARMING_ECC
			writeb((PAGE_4K|WIDTH_8|WP_DISABLE|DIRECT_MAP|CHECK_ALLFF)&(~OLDDATA_EN),
			info->reg + WMT_NFC_NAND_TYPE_SEL);
		#else
			if (ecc_type == 1)
				writeb((PAGE_4K|WIDTH_8|WP_DISABLE|DIRECT_MAP|CHECK_ALLFF) & (~OLDDATA_EN),
				info->reg + WMT_NFC_NAND_TYPE_SEL);
			else
				writeb(PAGE_4K|WIDTH_8|WP_DISABLE|DIRECT_MAP|CHECK_ALLFF|OLDDATA_EN,
				info->reg + WMT_NFC_NAND_TYPE_SEL);
		#endif
	/*} else if (mtd->writesize == 8192) {
		// page size == 8192//
		#ifndef NAND_HARMING_ECC
			writeb((PAGE_8K|WIDTH_8|WP_DISABLE|DIRECT_MAP|CHECK_ALLFF)&(~OLDDATA_EN),
			info->reg + WMT_NFC_NAND_TYPE_SEL);
		#else
			if (ecc_type == 1)
				writeb((PAGE_8K|WIDTH_8|WP_DISABLE|DIRECT_MAP|CHECK_ALLFF) & (~OLDDATA_EN),
				info->reg + WMT_NFC_NAND_TYPE_SEL);
			else
				writeb(PAGE_8K|WIDTH_8|WP_DISABLE|DIRECT_MAP|CHECK_ALLFF|OLDDATA_EN,
				info->reg + WMT_NFC_NAND_TYPE_SEL);
		#endif*/
	} else {
		/*  writeb(PAGE_512|WIDTH_8|WP_DISABLE|DIRECT_MAP|OLDDATA_EN,
		info->reg + WMT_NFC_NAND_TYPE_SEL); */ /*old structure*/
		writeb((PAGE_512|WIDTH_8|WP_DISABLE|DIRECT_MAP)&(~OLDDATA_EN),
		info->reg + WMT_NFC_NAND_TYPE_SEL); /*new structure*/
	}
	chip->select_chip(mtd, -1);
	status = wmt_calc_clock(mtd, spec_clk, &T, &divisor, &Thold);
	if (status) {
		printk("timming setting fail");
		return 1;
	}
	NFC_RWTimming = ((Thold&0xFF) << 12) + ((T + (Thold&0xFF)) << 8) +
	/* nand write timing 1T2T has bug, will cause write fail only can set 2T4T */
	(2 << 4) + 4;/*(((Thold>>8)&0xFF) << 4) + 2*((Thold>>8)&0xFF);*/
	*(volatile unsigned long *)PMNAND_ADDR = (divisor&NFC_ClockMask);
	while ((*(volatile unsigned long *)(PMCS_ADDR+0x18))&0x3F0038)
	;

	divisor = *(volatile unsigned long *)PMNAND_ADDR;
	printk(KERN_WARNING "divisor PMNAND_ADDR is set 0x%x, NFC_timing=0x%x\n", divisor, NFC_RWTimming);
	writel(readl(info->reg + WMT_NFC_READ_CYCLE_PULE_CTRL) & 0xffff0000,
	info->reg + WMT_NFC_READ_CYCLE_PULE_CTRL);
	writel(readl(info->reg + WMT_NFC_READ_CYCLE_PULE_CTRL) | NFC_RWTimming,
	info->reg + WMT_NFC_READ_CYCLE_PULE_CTRL);
	
	return 0;
}

#if 0
static void disable_redunt_out_bch_ctrl(struct wmt_nand_info *info, int flag)
{
	if (flag == 1)
		writeb(readb(info->reg + WMT_NFC_CALC_CTRL)|0x02, info->reg + WMT_NFC_CALC_CTRL);
	else
		writeb(readb(info->reg + WMT_NFC_CALC_CTRL)&0xfd, info->reg + WMT_NFC_CALC_CTRL);
}
static void redunt_read_hm_ecc_ctrl(struct wmt_nand_info *info, int flag)
{
	if (flag == 1)
		writeb(readb(info->reg + WMT_NFC_SMC_ENABLE) | 0x02, info->reg + WMT_NFC_SMC_ENABLE);
	else
		writeb(readb(info->reg + WMT_NFC_SMC_ENABLE) & 0xfd, info->reg + WMT_NFC_SMC_ENABLE);
}
#endif

static void set_ecc_engine(struct wmt_nand_info *info, int type)
{
	/*struct wmt_nand_info *info = wmt_nand_mtd_toinfo(mtd);*/
	writel(readl(info->reg + WMT_NFC_ECC_BCH_CTRL) & 0xfffffff8, info->reg + WMT_NFC_ECC_BCH_CTRL);
	writel(readl(info->reg + WMT_NFC_ECC_BCH_CTRL) | type, info->reg + WMT_NFC_ECC_BCH_CTRL);

	/* enable BCH ecc interrupt and new structure */
	/*if (type == ECC16bit || type == ECC24bitPer1K) { to support read page twice*/
	if (type == ECC16bit) {
		/*printk(KERN_ERR "set_ecc_engine for bch 16 bit\n");*/
		writew(eccBCH_inetrrupt_enable, info->reg + WMT_NFC_ECC_BCH_INT_MASK);
		writel(readl(info->reg + WMT_NFC_ECC_BCH_CTRL) | READ_RESUME,
		info->reg + WMT_NFC_ECC_BCH_CTRL);
		if (ecc_type == 1)
			writel(readl(info->reg + WMT_NFC_NAND_TYPE_SEL) & (~OLDDATA_EN),
			info->reg + WMT_NFC_NAND_TYPE_SEL);
		else
			writel(readl(info->reg + WMT_NFC_NAND_TYPE_SEL) | OLDDATA_EN,
			info->reg + WMT_NFC_NAND_TYPE_SEL);
	} else if (type == ECC12bit) {
		/*printk(KERN_ERR "set_ecc_engine for bch 12 bit\n");*/
		writew(eccBCH_inetrrupt_enable, info->reg + WMT_NFC_ECC_BCH_INT_MASK);
		writel(readl(info->reg + WMT_NFC_ECC_BCH_CTRL) | READ_RESUME,
		info->reg + WMT_NFC_ECC_BCH_CTRL);
		if (ecc_type == 1)
			writel(readl(info->reg + WMT_NFC_NAND_TYPE_SEL) & (~OLDDATA_EN),
			info->reg + WMT_NFC_NAND_TYPE_SEL);
		else
			writel(readl(info->reg + WMT_NFC_NAND_TYPE_SEL) | OLDDATA_EN,
			info->reg + WMT_NFC_NAND_TYPE_SEL);
	} else if (type == ECC8bit) {
		/*printk(KERN_ERR "set_ecc_engine for bch 8 bit\n");*/
		writew(eccBCH_inetrrupt_enable, info->reg + WMT_NFC_ECC_BCH_INT_MASK);
		writel(readl(info->reg + WMT_NFC_ECC_BCH_CTRL) | READ_RESUME,
		info->reg + WMT_NFC_ECC_BCH_CTRL);
		if (ecc_type == 1)
			writel(readl(info->reg + WMT_NFC_NAND_TYPE_SEL) & (~OLDDATA_EN),
			info->reg + WMT_NFC_NAND_TYPE_SEL);
		else
			writel(readl(info->reg + WMT_NFC_NAND_TYPE_SEL) | OLDDATA_EN,
			info->reg + WMT_NFC_NAND_TYPE_SEL);
	} else if (type == ECC4bit) {
		/*printk(KERN_ERR "set_ecc_engine for bch 4 bit\n");*/
		writew(eccBCH_inetrrupt_enable, info->reg + WMT_NFC_ECC_BCH_INT_MASK);
		writel(readl(info->reg + WMT_NFC_ECC_BCH_CTRL) | READ_RESUME,
		info->reg + WMT_NFC_ECC_BCH_CTRL);
		if (ecc_type == 1)
			writel(readl(info->reg + WMT_NFC_NAND_TYPE_SEL) & (~OLDDATA_EN),
			info->reg + WMT_NFC_NAND_TYPE_SEL);
		else
			writel(readl(info->reg + WMT_NFC_NAND_TYPE_SEL) | OLDDATA_EN,
			info->reg + WMT_NFC_NAND_TYPE_SEL);
	}	else { /*disable 4bit ecc interrupt and old structure*/
		writew(eccBCH_inetrrupt_disable, info->reg + WMT_NFC_ECC_BCH_INT_MASK);
		writel(readl(info->reg + WMT_NFC_ECC_BCH_CTRL) | READ_RESUME,
		info->reg + WMT_NFC_ECC_BCH_CTRL);
		#ifdef NAND_DEBUG
		printk(KERN_ERR "set_ecc_engine for harmming\n");
		#endif
		if (ecc_type == 1)
			writel(readl(info->reg + WMT_NFC_NAND_TYPE_SEL) & (~OLDDATA_EN),
			info->reg + WMT_NFC_NAND_TYPE_SEL);
		else
			writel(readl(info->reg + WMT_NFC_NAND_TYPE_SEL) | OLDDATA_EN,
			info->reg + WMT_NFC_NAND_TYPE_SEL);
	}
}


static int wmt_nand_ready(struct mtd_info *mtd)
{
	struct wmt_nand_info *info = wmt_nand_mtd_toinfo(mtd);
	//unsigned int b2r_stat;
	int i = 0;

	while (1)	{
		if (readb(info->reg + WMT_NFC_HOST_STAT_CHANGE) & B2R)
			break;
		if ((++i>>20)) {
			printk(KERN_ERR "nand flash is not ready\n");
			/*print_register(mtd);*/
			/*    while (1);*/
			return -1;
		}
	}
	//b2r_stat = readb(info->reg + WMT_NFC_HOST_STAT_CHANGE);
	writeb(B2R, info->reg + WMT_NFC_HOST_STAT_CHANGE);
	if (readb(info->reg + WMT_NFC_HOST_STAT_CHANGE) & B2R)	{
		printk(KERN_ERR "NFC err : B2R status not clean\n");
		return -2;
	}
	return 0;
}


static int wmt_nfc_transfer_ready(struct mtd_info *mtd)
{
	struct wmt_nand_info *info = wmt_nand_mtd_toinfo(mtd);
	int i = 0;

	while (1)	{
		if (!(readb(info->reg + WMT_NFC_MISC_STAT_PORT) & NFC_BUSY))
			break;

		if (++i>>20)
			return -3;
	}
	return 0;
}
/* Vincent  2008.11.3*/
static int wmt_wait_chip_ready(struct mtd_info *mtd)
{
	struct wmt_nand_info *info = wmt_nand_mtd_toinfo(mtd);
	int i = 0;

	while (1) {
		if ((readb(info->reg + WMT_NFC_MISC_STAT_PORT) & FLASH_RDY))
			break;
		if (++i>>20)
			return -3;
	}
	return 0;
}
static int wmt_wait_cmd_ready(struct mtd_info *mtd)
{
	struct wmt_nand_info *info = wmt_nand_mtd_toinfo(mtd);
	int i = 0;

	while (1)	{
		if (!(readb(info->reg + WMT_NFC_MISC_STAT_PORT) & NFC_CMD_RDY))
			break;
		if (++i>>20)
			return -3;
	}
	return 0;
}

/* #if (NAND_PAGE_SIZE == 512) Vincent 2008.11.4
static int wmt_wait_dma_ready(struct mtd_info *mtd)
{
	struct wmt_nand_info *info = wmt_nand_mtd_toinfo(mtd);
	int i = 0;

	while (1) {
		if (!(readb(info->reg + NFC_IDLE) & 0x02))
			break;
		if (++i>>20) {
			printk(KERN_ERR"\r DMA NOT Ready!\n");
			print_register(mtd);
			return -3;
		}
	}
	return 0;
}
#endif  Vincent 2008.11.4*/

static void wmt_wait_nfc_ready(struct wmt_nand_info *info)
{
	unsigned int bank_stat1, i = 0;
	while (1) {
		bank_stat1 = readw(info->reg + WMT_NFC_ECC_BCH_INT_STAT1);/* Vincent 2008.11.14 */
		if (!(readb(info->reg + WMT_NFC_MISC_STAT_PORT) & NFC_BUSY))
			break;
		else if ((bank_stat1 & 0x101) == (ERR_CORRECT | BCH_ERR))
			break;

		if (i>>20)
			return;
		i++;
	}
}

static void bit_correct(uint8_t *c, uint8_t pos)
{
	c[0] = (((c[0] ^ (0x01<<pos)) & (0x01<<pos)) | (c[0] & (~(0x01<<pos))));
	#if 0
	temp = info->dmabuf[bch_err_idx[0] >> 3];
	temp >>= ((bch_err_idx[0] & 0x07) - 1);
	#endif
}

/*
 * flag = 0, need check BCH ECC
 * flag = 1, don't check ECC
 * flag = 2, need check Harming ECC
 *
*/

static int wmt_nfc_wait_idle(struct mtd_info *mtd, unsigned int flag, int command,
int column, unsigned int page_addr)
{
	struct wmt_nand_info *info = wmt_nand_mtd_toinfo(mtd);
	int i = 0;

	#ifndef NAND_HARMING_ECC/*Vincent 2008.11.14*/
	int page_step, oob_step;
	/*unsigned int bch_err_pos[4], bch_ecc_idx;*/
	if (mtd->writesize < 8192)
		page_step = 1 + mtd->writesize/512;
	else
		page_step = 1 + mtd->writesize/1024;
	oob_step = 1;
	#endif

	while (1) {
		#ifdef NAND_DEBUG
		printk(KERN_WARNING "waiting NFC idle......\n");
		#endif
		if (readb(info->reg + WMT_NFC_IDLE_STAT) & NFC_IDLE)
			break;

		if (flag == 1 || flag == 2) {
			#ifdef NAND_DEBUG
			printk(KERN_NOTICE "\r in wait idle ECC OOB MODE(9)0x024\n");
			print_register(mtd);
			#endif
			;
			#if 0
			else if (flag == 2) {
				/* bad block check: don't need HM ECC correct
				* and read oob don't need check ecc */
				if (readb(info->reg + WMT_NFC_REDUNT_ECC_STAT) & 0x07) {
					printk(KERN_ERR "There are ecc err in reduntant area---------\n");
					mtd->ecc_stats.failed++;
					return -1;
				} else if (readb(info->reg + WMT_NFC_BANK18_ECC_STAT) & 0xffffffff) {
					printk(KERN_ERR "There are ecc err in data area--------------\n");
					mtd->ecc_stats.failed++;
					return -1;
				}
			}
			#endif
		} else {
			/* check BCH */
			#ifndef NAND_HARMING_ECC/*Vincent 2008.11.14*/
			if (command == NAND_CMD_READ0) {
				#ifdef NAND_DEBUG
				printk(KERN_NOTICE "in nfc_wait_idle(): Read data \n");
				#endif
				/*if(page_addr == 0x343fb) {
					printk(KERN_NOTICE "page_step:%x, page_addr0x%x\n", page_step, page_addr);
					int j;
					for (j = 0; j < 0xA8; j += 16)
						printk(KERN_NOTICE "NFCR%x ~ NFCR%x = 0x%8.8x 0x%8.8x 0x%8.8x 0x%8.8x\r\n",
						j/4, (j+12)/4,
						readl(info->reg + j + 0),
						readl(info->reg + j + 4),
						readl(info->reg + j + 8),
						readl(info->reg + j + 12));
				}*/
				#if 0
				for (k = 0; k < page_step; k++) {
					wmt_wait_nfc_ready(info);
					bank_stat1 = readw(info->reg + WMT_NFC_ECC_BCH_INT_STAT1);
					if ((bank_stat1 & 0x101) == (ERR_CORRECT | BCH_ERR)) {
						bank_stat2 = readw(info->reg + WMT_NFC_ECC_BCH_INT_STAT2);
						/* 0: data, 1: reduntant area */
						data_redunt_flag = bank_stat2 & 0x800;
						#ifdef NAND_DEBUG
						printk(KERN_NOTICE" BCH Read data ecc eror command: %x"
						" column:%x, page_addr:%x\n", command, column, page_addr);
						printk(KERN_NOTICE "in test check bch 4/8 bit ecc correct \n");
						#endif
						if (data_redunt_flag) {
							bch4bit_redunt_ecc_correct(mtd);

						} else {
							if (info->phase)
								bch4bit_data_ecc_correct(mtd, 4096);
							else
								bch4bit_data_ecc_correct(mtd, 0);
						}
					}
				}
				#endif
			} else if (command == NAND_CMD_READOOB) {
				/* for reduntant area */
				#ifdef NAND_DEBUG
				printk(KERN_NOTICE "in nfc_wait_idle(): Read oob data \n");
				//#endif
				bank_stat1 = readw(info->reg + WMT_NFC_ECC_BCH_INT_STAT1);
				if ((bank_stat1 & 0x101) == (ERR_CORRECT | BCH_ERR)) {
					if (redunt_err_mark == 0) {
						redunt_err_mark = 1;
						/*printk(KERN_NOTICE " read redunt redundant mark set to 1\n");*/
					}
					bank_stat2 = readw(info->reg + WMT_NFC_ECC_BCH_INT_STAT2);
					#ifdef NAND_DEBUG
					printk(KERN_NOTICE" BCH Read oob ecc eror command: %x"
					" column:%x, page_addr:%x\n", command, column, page_addr);
					#endif
					wmt_wait_nfc_ready(info);
					bch4bit_redunt_ecc_correct(mtd);
				}
				#endif
			}
			#endif
		}
		if (i>>20)
			return -1;
		i++;
	}
	#ifndef NAND_HARMING_ECC/*Vincent 2008.11.14*/
	/* continue read next bank and calc BCH ECC */
	if (ecc_type == 1)
		writew(readw(info->reg + WMT_NFC_ECC_BCH_CTRL) | READ_RESUME,
		info->reg + WMT_NFC_ECC_BCH_CTRL);
	#endif

	return 0;
}

void bch4bit_data_ecc_correct(struct mtd_info *mtd, int phase)
{
	int i;
	struct wmt_nand_info *info = wmt_nand_mtd_toinfo(mtd);
	unsigned int bch_err_pos[12], bank_stat1, bank_stat2, bch_ecc_idx, bank, ecc_engine;

	bank_stat1 = readw(info->reg + WMT_NFC_ECC_BCH_INT_STAT1);
	if ((bank_stat1 & 0x101) == (ERR_CORRECT | BCH_ERR)) {
		/* BCH ECC err process */
		bank_stat2 = readw(info->reg + WMT_NFC_ECC_BCH_INT_STAT2);
		bch_ecc_idx = bank_stat2 & BCH_ERR_CNT;
		bank = (bank_stat2 & 0x700) >> 8;
		/* for data area */
		#ifdef NAND_DEBUG
		printk(KERN_NOTICE "in nfc_wait_idle(): Read data \n");
		#endif
		ecc_engine = readl(info->reg + WMT_NFC_ECC_BCH_CTRL)&ECC_MODE;
		if (bch_ecc_idx >= 0x1F) {
			/* BCH ECC code of 512 bytes data which is all "FF" */
			if ((readl((unsigned int *)(info->reg+ECC_FIFO_0) + bank * 4) == 0xffffffff) &&
			((readl((unsigned int *)(info->reg+ECC_FIFO_1) + bank * 4) & 0xffffff) == 0xffffff)) {
				#ifdef NAND_DEBUG
				printk(KERN_WARNING "in nfc_wait_idle(): BCH ECC code is all FF\n");
				#endif
				writew(readw(info->reg + WMT_NFC_ECC_BCH_INT_STAT1)|(ERR_CORRECT|BCH_ERR),
				info->reg + WMT_NFC_ECC_BCH_INT_STAT1);
				writew(readw(info->reg + WMT_NFC_ECC_BCH_CTRL) | READ_RESUME,
				info->reg + WMT_NFC_ECC_BCH_CTRL);
				return;
			}
			#ifdef NAND_DEBUG
			printk(KERN_NOTICE "reg1 is %x\n",
			readl((unsigned int *)(info->reg+ECC_FIFO_0) + bank * 4));
			printk(KERN_NOTICE "reg2 is %x\n",
			readl((unsigned int *)(info->reg+ECC_FIFO_1) + bank * 4));
			#endif
			printk(KERN_ERR "in nfc_wait_idle(): data area uncorrected err \n");

			writew(readw(info->reg + WMT_NFC_ECC_BCH_INT_STAT1)|(ERR_CORRECT | BCH_ERR),
			info->reg + WMT_NFC_ECC_BCH_INT_STAT1);
			writew(readw(info->reg + WMT_NFC_ECC_BCH_CTRL) | READ_RESUME,
			info->reg + WMT_NFC_ECC_BCH_CTRL);
			mtd->ecc_stats.failed++;
			return; /* uncorrected err */
		}

		/* mtd->ecc_stats.corrected += (bank_stat2 & BCH_ERR_CNT);*/
		/* BCH ECC correct */
		#ifdef NAND_DEBUG
		printk(KERN_NOTICE "data area %d bit corrected err on bank %d \n", bch_ecc_idx, bank);
		#endif
		for (i = 0; i < bch_ecc_idx; i++) {
			bch_err_pos[i] = (readw(info->reg +	WMT_NFC_ECC_BCH_ERR_POS1 + 2*i) & 0x1fff);
			if((bch_err_pos[i] >> 3) < 512)
				bit_correct(&info->dmabuf[512 * bank + phase +(bch_err_pos[i] >> 3)], bch_err_pos[i] & 0x07);

			#ifdef NAND_DEBUG
			printk(KERN_NOTICE "in nfc_wait_idle(): data area %xth ecc error position is byte%d bit%d\n",
			i, 512 * bank + phase + (bch_err_pos[i] >> 3), (bch_err_pos[i] & 0x07));
			#endif
		}
	} /* end of if ((bank_stat1 & 0x101) */
	/* continue read next bank and calc BCH ECC */
	writew(readw(info->reg + WMT_NFC_ECC_BCH_INT_STAT1)|(ERR_CORRECT | BCH_ERR),
	info->reg + WMT_NFC_ECC_BCH_INT_STAT1);
	writew(readw(info->reg + WMT_NFC_ECC_BCH_CTRL) | READ_RESUME,
	info->reg + WMT_NFC_ECC_BCH_CTRL);
}

void bch4bit_redunt_ecc_correct(struct mtd_info *mtd)
{
	int i;
	struct wmt_nand_info *info = wmt_nand_mtd_toinfo(mtd);
	unsigned int bch_err_pos[12], bank_stat1, bank_stat2, bch_ecc_idx, ecc_engine;

	/* BCH ECC err process */
	bank_stat2 = readw(info->reg + WMT_NFC_ECC_BCH_INT_STAT2);
	bch_ecc_idx = bank_stat2 & BCH_ERR_CNT;

	/* bank = (bank_stat2 & 0x700) >> 8; */
	bank_stat1 = readw(info->reg + WMT_NFC_ECC_BCH_INT_STAT1);
	if ((bank_stat1 & 0x101) == (ERR_CORRECT | BCH_ERR)) {
		/* mtd->ecc_stats.corrected += (bank_stat2 & BCH_ERR_CNT);*/
		/* BCH ECC correct */
		/* for reduntant area */
		ecc_engine = readl(info->reg + WMT_NFC_ECC_BCH_CTRL)&ECC_MODE;
		if (bch_ecc_idx >= 0x1F) {
			writew(readw(info->reg + WMT_NFC_ECC_BCH_INT_STAT1)|(ERR_CORRECT | BCH_ERR),
			info->reg + WMT_NFC_ECC_BCH_INT_STAT1);
			writew(readw(info->reg + WMT_NFC_ECC_BCH_CTRL) | READ_RESUME,
			info->reg + WMT_NFC_ECC_BCH_CTRL);
			printk(KERN_ERR
			"in nfc_wait_idle(): redunt area uncorrected err bch_ecc_idx:%x\n", bch_ecc_idx);
			mtd->ecc_stats.failed++;
			return;
			/* return -4;*/  /* uncorrected err */
		}
		/* mtd->ecc_stats.corrected += (bank_stat2 & BCH_ERR_CNT);*/
		/* BCH ECC correct */
		#ifdef NAND_DEBUG
		printk(KERN_NOTICE "reduntant %d bit corrected error\n", bch_ecc_idx);
		#endif
		for (i = 0; i < bch_ecc_idx; i++) {
			bch_err_pos[i] = (readw(info->reg + WMT_NFC_ECC_BCH_ERR_POS1 + 2*i) & 0x7ff);
			if((bch_err_pos[i] >> 3) < 64)
			bit_correct((uint8_t *)(info->reg+ECC_FIFO_0)+(bch_err_pos[i] >> 3),
			(bch_err_pos[i] & 0x07));

			#ifdef NAND_DEBUG
			printk(KERN_NOTICE "in nfc_wait_idle(): redunt %xth ecc error position is byte%d bit%d\n",
			i, (bch_err_pos[i] >> 3), (bch_err_pos[i] & 0x07));
			#endif
		}
	}
	/* continue read next bank and calc BCH ECC */
	writew(readw(info->reg + WMT_NFC_ECC_BCH_INT_STAT1)|(ERR_CORRECT | BCH_ERR),
	info->reg + WMT_NFC_ECC_BCH_INT_STAT1);
	writew(readw(info->reg + WMT_NFC_ECC_BCH_CTRL) | READ_RESUME,
	info->reg + WMT_NFC_ECC_BCH_CTRL);
}

/*
*   [Routine Description]
*	read status
*   [Arguments]
*	cmd : nand read status command
*   [Return]
*	the result of command
*/
static int wmt_read_nand_status(struct mtd_info *mtd, unsigned char cmd)
{
	struct wmt_nand_info *info = wmt_nand_mtd_toinfo(mtd);
	int cfg = 0, status = -1;
	unsigned int b2r_stat;

	writeb(cmd, info->reg + WMT_NFC_COMPORT0);
	cfg = DPAHSE_DISABLE|NFC2NAND|(1<<1);

	b2r_stat = readb(info->reg + WMT_NFC_HOST_STAT_CHANGE);
	writeb(B2R|b2r_stat, info->reg + WMT_NFC_HOST_STAT_CHANGE);

	writeb(cfg|NFC_TRIGGER, info->reg + WMT_NFC_COMCTRL);
	status = wmt_wait_cmd_ready(mtd);
	if (status) {
		printk(KERN_ERR "NFC command transfer1 is not ready\n");
		writew(readw(info->reg + WMT_NFC_ECC_BCH_CTRL) | READ_RESUME,
		info->reg + WMT_NFC_ECC_BCH_CTRL);
		return status;
	}
	b2r_stat = readb(info->reg + WMT_NFC_HOST_STAT_CHANGE);
	writeb(B2R|b2r_stat, info->reg + WMT_NFC_HOST_STAT_CHANGE);

	cfg = SING_RW|NAND2NFC;
	writeb(cfg|NFC_TRIGGER, info->reg + WMT_NFC_COMCTRL);

	status = wmt_wait_cmd_ready(mtd);
	if (status) {
		printk(KERN_ERR "NFC command transfer2 is not ready\n");
		writew(readw(info->reg + WMT_NFC_ECC_BCH_CTRL) | READ_RESUME,
		info->reg + WMT_NFC_ECC_BCH_CTRL);
		return status;
	}
	status = wmt_nfc_transfer_ready(mtd);
	/* status = wmt_nand_wait_idle(mtd);*/
	if (status) {
		printk(KERN_ERR "NFC IO transfer is not ready\n");
		writew(readw(info->reg + WMT_NFC_ECC_BCH_CTRL) | READ_RESUME,
		info->reg + WMT_NFC_ECC_BCH_CTRL);
		/*print_register(mtd);*/
		return status;
	}
		 /* return read status  */
	/*   return readb(info->reg + WMT_NFC_DATAPORT) & 0xff;*/
	#ifdef NAND_DEBUG
	printk(KERN_NOTICE "read status is %x\n", readb(info->reg + WMT_NFC_DATAPORT) & 0xff);
	#endif
	info->datalen = 0;
	info->dmabuf[0] = readb(info->reg + WMT_NFC_DATAPORT) & 0xff;
	status = info->dmabuf[0];
	return status;
}


/* data_flag = 0:  set data ecc fifo */
static int wmt_nfc_dma_cfg(struct mtd_info *mtd, unsigned int len, unsigned int wr,
int data_flag, int Nbank)
{
	struct wmt_nand_info *info = wmt_nand_mtd_toinfo(mtd);
	int status;
	unsigned long *ReadDesc, *WriteDesc;
	ReadDesc = (unsigned long *)(info->dmabuf + mtd->writesize + mtd->oobsize
	+ (16-(mtd->oobsize)%16) + 0x100);
	WriteDesc = (unsigned long *)(info->dmabuf + mtd->writesize + mtd->oobsize
	+ (16-(mtd->oobsize)%16) + 0x200);
	/*
	printk(KERN_ERR "info->dmabuf = 0x%x\r\n", (unsigned int) info->dmabuf);
	printk(KERN_ERR "info->dmaaddr = 0x%x\r\n", (unsigned int) info->dmaaddr);
	printk(KERN_ERR "ReadDesc addr = 0x%x\r\n", (unsigned int) ReadDesc);
	printk(KERN_ERR "WriteDesc addr = 0x%x\r\n", (unsigned int) WriteDesc);
	*/

	if (len == 0)	{
		printk(KERN_ERR "DMA transfer length = 0\r\n");
		return 1;
	}
	if (data_flag == 0) {
		/* data:  set data ecc fifo */
		if (mtd->writesize == 512) {
			writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_0));
			writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_1));
			writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_2));
			writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_3));
		} else { /* pagesize = 2048 or 4096 */
			if (mtd->writesize == 2048)
				writeb(readb(info->reg + WMT_NFC_CALC_CTRL) & 0xF7, info->reg + WMT_NFC_CALC_CTRL);
			else { /*if (NAND_PAGE_SIZE == 4096)*/
				writeb(readb(info->reg + WMT_NFC_CALC_CTRL) & 0xF7, info->reg + WMT_NFC_CALC_CTRL);

				writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_0));
				writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_1));
				writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_2));
				writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_3));
		
				writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_4));
				writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_5));
				writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_6));
				writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_7));
		
				writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_8));
				writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_9));
				writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_a));
				writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_b));
		
				writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_c));
				writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_d));
				writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_e));
				writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_f));
				writeb(readb(info->reg + WMT_NFC_CALC_CTRL) | 0x08, info->reg + WMT_NFC_CALC_CTRL);
			}
			writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_0));
			writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_1));
			writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_2));
			writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_3));
	
			writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_4));
			writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_5));
			writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_6));
			writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_7));
	
			writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_8));
			writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_9));
			writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_a));
			writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_b));
	
			writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_c));
			writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_d));
			writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_e));
			writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_f));
			writeb(readb(info->reg + WMT_NFC_CALC_CTRL) & 0xF7, info->reg + WMT_NFC_CALC_CTRL);
		}
	} else if (data_flag == 1) {
		/* reduntant area:  set reduntant data ecc fifo  BCH ECC */
		writeb(readb(info->reg + WMT_NFC_CALC_CTRL) & 0xF7, info->reg + WMT_NFC_CALC_CTRL);
		writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_0));
		writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_1));
		writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_2));
		writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_3));
		writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_4));
		writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_5));
		writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_6));
		writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_7));
		writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_8));
		writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_9));
		writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_a));
		writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_b));
		writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_c));
		writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_d));
		writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_e));
		writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_f));
		if (mtd->writesize == 4096 || mtd->writesize == 8192) {
			#ifdef NAND_HARMING_ECC
			writeb(readb(info->reg + WMT_NFC_CALC_CTRL) | 0x08, info->reg + WMT_NFC_CALC_CTRL);
			writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_0));
			writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_1));
			writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_2));
			writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_3));
			writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_4));
			writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_5));
			writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_6));
			writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_7));
			writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_8));
			writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_9));
			writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_a));
			writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_b));
			writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_c));
			writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_d));
			writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_e));
			writel(0xffffffff, (unsigned int *)(info->reg + ECC_FIFO_f));
			writeb(readb(info->reg + WMT_NFC_CALC_CTRL) & 0xF7, info->reg + WMT_NFC_CALC_CTRL);
			#endif
		}
#if 0 /*Vincent 2008.11.4*/
	if (mtd->writesize == 2048) {
		writeb(readb(info->reg + WMT_NFC_CALC_CTRL) & 0xF7, info->reg + WMT_NFC_CALC_CTRL);
		if (Nbank == 0)
			writel(readl(info->reg + ECC_FIFO_3) | 0xffffff00, info->reg + ECC_FIFO_3);
		else if (Nbank == 1)
			writel(readl(info->reg + ECC_FIFO_7) | 0xffffff00, info->reg + ECC_FIFO_7);
		else if (Nbank == 2)
			writel(readl(info->reg + ECC_FIFO_b) | 0xffffff00, info->reg + ECC_FIFO_b);
		else if (Nbank == 3)
			writel(readl(info->reg + ECC_FIFO_f) | 0xffffff00, info->reg + ECC_FIFO_f);
	} else if (mtd->writesize == 4096 || mtd->writesize == 8192) {
		writeb(readb(info->reg + WMT_NFC_CALC_CTRL) & 0xF7, info->reg + WMT_NFC_CALC_CTRL);
	
		if (Nbank == 0)
			writel(readl(info->reg + ECC_FIFO_3) | 0xffffff00, info->reg + ECC_FIFO_3);
		else if (Nbank == 1)
			writel(readl(info->reg + ECC_FIFO_7) | 0xffffff00, info->reg + ECC_FIFO_7);
		else if (Nbank == 2)
			writel(readl(info->reg + ECC_FIFO_b) | 0xffffff00, info->reg + ECC_FIFO_b);
		else if (Nbank == 3)
			writel(readl(info->reg + ECC_FIFO_f) | 0xffffff00, info->reg + ECC_FIFO_f);
	
		writeb(readb(info->reg + WMT_NFC_CALC_CTRL) | 0x08, info->reg + WMT_NFC_CALC_CTRL);
	
		if (Nbank == 0)
			writel(readl(info->reg + ECC_FIFO_3) | 0xffffff00, info->reg + ECC_FIFO_3);
		else if (Nbank == 1)
			writel(readl(info->reg + ECC_FIFO_7) | 0xffffff00, info->reg + ECC_FIFO_7);
		else if (Nbank == 2)
			writel(readl(info->reg + ECC_FIFO_b) | 0xffffff00, info->reg + ECC_FIFO_b);
		else if (Nbank == 3)
			writel(readl(info->reg + ECC_FIFO_f) | 0xffffff00, info->reg + ECC_FIFO_f);
	} else {  /* pagesize = 512  BCH ECC */
		writel(readl(info->reg + ECC_FIFO_3) | 0xffffff00, info->reg + ECC_FIFO_3);
	}
#endif /* end of #if1 */
	} else if (data_flag == 2) {
		/* reduntant area:  set reduntant data ecc fifo  Harming ECC */
		#if 0
		if (Nbank == 0)
			writel(readl(info->reg + ECC_FIFO_3) | 0xffffff00, info->reg + ECC_FIFO_3);
	
			else if (Nbank == 1)
				writel(readl(info->reg + ECC_FIFO_7) | 0xffffff00, info->reg + ECC_FIFO_7);
			else if (Nbank == 2)
				writel(readl(info->reg + ECC_FIFO_b) | 0xffffff00, info->reg + ECC_FIFO_b);
			else if (Nbank == 3)
				writel(readl(info->reg + ECC_FIFO_f) | 0xffffff00, info->reg + ECC_FIFO_f);
		#endif
		if (mtd->writesize == 2048)
			writel(readl(info->reg + ECC_FIFO_c) | 0xffff0000, info->reg + ECC_FIFO_c);
		else   /* pagesize = 512  Harming ECC */
			writel(readl(info->reg + ECC_FIFO_3) | 0xfffff00, info->reg + ECC_FIFO_3);
	}
	writew(len - 1, info->reg + WMT_NFC_DMA_COUNTER);
	if (readl(info->reg + NFC_DMA_ISR) & NAND_PDMA_IER_INT_STS)
		writel(NAND_PDMA_IER_INT_STS, info->reg + NFC_DMA_ISR);

	if (readl(info->reg + NFC_DMA_ISR) & NAND_PDMA_IER_INT_STS) {
		printk(KERN_ERR "PDMA interrupt status can't be clear ");
		printk(KERN_ERR "NFC_DMA_ISR = 0x%8.8x \n", (unsigned int)readl(info->reg + NFC_DMA_ISR));
	}

	status = nand_init_pdma(mtd);
	if (status)
		printk(KERN_ERR "nand_init_pdma fail status = 0x%x", status);
	nand_alloc_desc_pool((wr) ? WriteDesc : ReadDesc);
	/*nand_init_short_desc((wr)?WriteDesc : ReadDesc, len, (unsigned long *)buf);*/
	if (Nbank == 2 && mtd->writesize >= 8192) {
		/*printk(KERN_ERR "dma phase2 wr=%d,len=0x%x\n", wr, len);*/
		nand_init_long_desc((wr) ? WriteDesc : ReadDesc, len,
		(unsigned long *)((unsigned long)info->dmaaddr + 4096), 0, 1);
	} else {
		/*printk(KERN_ERR "dma phase1 wr=%d,len=0x%x\n", wr, len);*/
		nand_init_long_desc((wr) ? WriteDesc : ReadDesc, len, (unsigned long *)info->dmaaddr, 0, 1);
	}
	nand_config_pdma(mtd,
	(wr) ? (unsigned long *)(info->dmaaddr + mtd->writesize + mtd->oobsize + (16-(mtd->oobsize)%16) + 0x200)
	: (unsigned long *)(info->dmaaddr + mtd->writesize + mtd->oobsize + (16-(mtd->oobsize)%16) + 0x100), wr);

	return 0;
}

int nand_init_pdma(struct mtd_info *mtd)
{
	struct wmt_nand_info *info = wmt_nand_mtd_toinfo(mtd);

	writel(NAND_PDMA_GCR_SOFTRESET, info->reg + NFC_DMA_GCR);
	writel(NAND_PDMA_GCR_DMA_EN, info->reg + NFC_DMA_GCR);
	if (readl(info->reg + NFC_DMA_GCR) & NAND_PDMA_GCR_DMA_EN)
		return 0;
	else
		return 1;
}


int nand_free_pdma(struct mtd_info *mtd)
{
	struct wmt_nand_info *info = wmt_nand_mtd_toinfo(mtd);
	writel(0, info->reg + NFC_DMA_DESPR);
	writel(0, info->reg + NFC_DMA_GCR);
	return 0;
}


int nand_alloc_desc_pool(unsigned long *DescAddr)
{
	memset(DescAddr, 0x00, 0x100);
	return 0;
}

int nand_init_short_desc(unsigned long *DescAddr, unsigned int ReqCount, unsigned long *BufferAddr)
{
	struct _NAND_PDMA_DESC_S *CurDes_S;
	CurDes_S = (struct _NAND_PDMA_DESC_S *) DescAddr;
	CurDes_S->ReqCount = ReqCount;
	CurDes_S->i = 1;
	CurDes_S->end = 1;
	CurDes_S->format = 0;
	CurDes_S->DataBufferAddr = (unsigned long)BufferAddr;
	return 0;
}

int nand_init_long_desc(unsigned long *DescAddr, unsigned int ReqCount, unsigned long *BufferAddr,
unsigned long *BranchAddr, int End)
{
	struct _NAND_PDMA_DESC_L *CurDes_L;
	CurDes_L = (struct _NAND_PDMA_DESC_L *) DescAddr;
	CurDes_L->ReqCount = ReqCount;
	CurDes_L->i = 0;
	CurDes_L->format = 1;
	CurDes_L->DataBufferAddr = (unsigned long)BufferAddr;
	CurDes_L->BranchAddr = (unsigned long)BranchAddr;
	if (End) {
		CurDes_L->end = 1;
		CurDes_L->i = 1;
	}

	return 0;
}
/*
int nand_config_desc(unsigned long *DescAddr, unsigned long *BufferAddr, int Blk_Cnt)
{
	int i = 0 ;
	unsigned long *CurDes = DescAddr;

	nand_alloc_desc_pool(CurDes);


	for (i = 0 ; i < 3 ; i++) {
		nand_init_short_desc(CurDes, 0x80, BufferAddr);
		BufferAddr += (i * 0x80);
		CurDes += (i * sizeof(NAND_PDMA_DESC_S));
	}
	if (Blk_Cnt > 1) {
		nand_init_long_desc(CurDes, 0x80, BufferAddr, CurDes + sizeof(NAND_PDMA_DESC_L), 0);
		BufferAddr += (i * 0x80);
		CurDes += (i * sizeof(NAND_PDMA_DESC_L));

		nand_init_long_desc(CurDes, (Blk_Cnt - 1) * 512, BufferAddr,
		CurDes + sizeof(NAND_PDMA_DESC_L), 1);
	} else {
		nand_init_long_desc(CurDes, 0x80, BufferAddr, CurDes + sizeof(NAND_PDMA_DESC_L), 1);
	}

	return 0;
}
*/

int nand_config_pdma(struct mtd_info *mtd, unsigned long *DescAddr, unsigned int dir)
{
	struct wmt_nand_info *info = wmt_nand_mtd_toinfo(mtd);

		writel(NAND_PDMA_IER_INT_EN, info->reg + NFC_DMA_IER);
	writel((unsigned long)DescAddr, info->reg + NFC_DMA_DESPR);
	if (dir == NAND_PDMA_READ)
		writel(readl(info->reg + NFC_DMA_CCR)|NAND_PDMA_CCR_peripheral_to_IF,
		info->reg + NFC_DMA_CCR);
	else
		writel(readl(info->reg + NFC_DMA_CCR)&(~NAND_PDMA_CCR_IF_to_peripheral),
		info->reg + NFC_DMA_CCR);

	/*mask_interrupt(IRQ_NFC_DMA);*/
	writel(readl(info->reg + NFC_DMA_CCR)|NAND_PDMA_CCR_RUN, info->reg + NFC_DMA_CCR);
	/*printk(KERN_ERR "NFC_DMA_CCR = 0x%8.8x\r\n", readl(info->reg + NFC_DMA_CCR));*/
	/*print_register(mtd);*/
	return 0;
}

int nand_pdma_handler(struct mtd_info *mtd)
{
	unsigned long status = 0;
	unsigned long count = 0;
	struct wmt_nand_info *info = wmt_nand_mtd_toinfo(mtd);

	count = 0x100000;
#if 0
	/*	 polling CSR TC status	*/
	if (!(readl(info->reg + NFC_DMA_CCR)|NAND_PDMA_CCR_peripheral_to_IF)) {
	do {
		count--;
		if (readl(info->reg + NFC_DMA_ISR) & NAND_PDMA_IER_INT_STS) {
			status = readl(info->reg + NFC_DMA_CCR) & NAND_PDMA_CCR_EvtCode;
			writel(readl(info->reg + NFC_DMA_ISR)&NAND_PDMA_IER_INT_STS, info->reg + NFC_DMA_ISR);
			printk(KERN_ERR "NFC_DMA_ISR = 0x%8.8x\r\n",
			(unsigned int)readl(info->reg + NFC_DMA_ISR));
			break;
		}
		if (count == 0) {
			printk(KERN_ERR "PDMA Time Out!\n");
			printk(KERN_ERR "NFC_DMA_CCR = 0x%8.8x\r\n",
			(unsigned int)readl(info->reg + NFC_DMA_CCR));
			/*print_register(mtd);*/
			count = 0x100000;
			/*break;*/
		}
	} while (1);
} else
#endif
	status = readl(info->reg + NFC_DMA_CCR) & NAND_PDMA_CCR_EvtCode;
	if (status == NAND_PDMA_CCR_Evt_ff_underrun)
		printk(KERN_ERR "PDMA Buffer under run!\n");

	if (status == NAND_PDMA_CCR_Evt_ff_overrun)
		printk(KERN_ERR "PDMA Buffer over run!\n");

	if (status == NAND_PDMA_CCR_Evt_desp_read)
		printk(KERN_ERR "PDMA read Descriptor error!\n");

	if (status == NAND_PDMA_CCR_Evt_data_rw)
		printk(KERN_ERR "PDMA read/write memory descriptor error!\n");

	if (status == NAND_PDMA_CCR_Evt_early_end)
		printk(KERN_ERR "PDMA read early end!\n");

	if (count == 0) {
		printk(KERN_ERR "PDMA TimeOut!\n");
		while (1)
			;
	}
	return 0;
}


static int wmt_nand_readID(struct mtd_info *mtd)
{
	struct wmt_nand_info *info = wmt_nand_mtd_toinfo(mtd);
	unsigned int cfg = 0, i = 0;
	int status = -1;

	writeb(NAND_CMD_READID, info->reg + WMT_NFC_COMPORT0);
	writeb(0x00, info->reg + WMT_NFC_COMPORT1_2);
	cfg = DPAHSE_DISABLE|(0x02<<1);
	writeb(cfg|NFC_TRIGGER, info->reg + WMT_NFC_COMCTRL);

	status = wmt_wait_cmd_ready(mtd);
	/*	status = wmt_nfc_ready(mtd);*/

	if (status) {
		printk(KERN_ERR "in wmt_nand_readID(): wait cmd is not ready\n");
		writew(readw(info->reg + WMT_NFC_ECC_BCH_CTRL) | READ_RESUME,
		info->reg + WMT_NFC_ECC_BCH_CTRL);
		return status;
	}
	cfg = NAND2NFC|SING_RW;
	for (i = 0; i < 5; i++) {
		writeb(cfg|NFC_TRIGGER, info->reg + WMT_NFC_COMCTRL);
		status = wmt_wait_cmd_ready(mtd);
		/*	status = wmt_nfc_ready(mtd);*/
		if (status)
				return status;
		status = wmt_nfc_transfer_ready(mtd);
		/* status = wmt_nand_wait_idle(mtd);*/
		if (status) {
			printk(KERN_ERR "in wmt_nand_readID(): wait transfer cmd is not ready\n");
			return status;
		}
		info->dmabuf[i] = readb(info->reg + WMT_NFC_DATAPORT) & 0xff;

		#ifdef NAND_DEBUG
			printk(KERN_NOTICE "readID is %x\n", readb(info->reg + WMT_NFC_DATAPORT));
		#endif
	}
	info->datalen = 0;
	return 0;
}


static int wmt_device_ready(struct mtd_info *mtd)
{
	struct wmt_nand_info *info = wmt_nand_mtd_toinfo(mtd);
	return readb(info->reg + WMT_NFC_MISC_STAT_PORT) & 0x01;
}


static void wmt_nand_enable_hwecc(struct mtd_info *mtd, int mode)
{
	struct wmt_nand_info *info = wmt_nand_mtd_toinfo(mtd);
	if (mode == hardware_ecc)
		writeb(readb(info->reg + WMT_NFC_MISC_CTRL) & 0xfb, info->reg + WMT_NFC_MISC_CTRL);
	else
		writeb(readb(info->reg + WMT_NFC_MISC_CTRL) | 0x04, info->reg + WMT_NFC_MISC_CTRL);
}

#if 0
static void print_register(struct mtd_info *mtd)
{
	struct wmt_nand_info *info = wmt_nand_mtd_toinfo(mtd);
	int j;
	printk(KERN_NOTICE "\r NFC_MISC_STAT_PORT: %8.8x\n", readb(info->reg + WMT_NFC_MISC_STAT_PORT));
	printk(KERN_NOTICE "\r NFC_DMA_COUNTER: %8.8x\n", readw(info->reg + WMT_NFC_DMA_COUNTER));
	printk(KERN_NOTICE "\rDMA Register: NFC_DMA_GCR   0x100 %8.8x\n", readl(info->reg + NFC_DMA_GCR));
	printk(KERN_NOTICE "\rDMA Register: NFC_DMA_IER   0x104 %8.8x\n", readl(info->reg + NFC_DMA_IER));
	printk(KERN_NOTICE "\rDMA Register: NFC_DMA_ISR   0x108 %8.8x\n", readl(info->reg + NFC_DMA_ISR));
	printk(KERN_NOTICE "\rDMA Register: NFC_DMA_DESPR 0x10C %8.8x\n", readl(info->reg + NFC_DMA_DESPR));
	printk(KERN_NOTICE "\rDMA Register: NFC_DMA_RBR   0x110 %8.8x\n", readl(info->reg + NFC_DMA_RBR));
	printk(KERN_NOTICE "\rDMA Register: NFC_DMA_DAR   0x114 %8.8x\n", readl(info->reg + NFC_DMA_DAR));
	printk(KERN_NOTICE "\rDMA Register: NFC_DMA_BAR   0x118 %8.8x\n", readl(info->reg + NFC_DMA_BAR));
	printk(KERN_NOTICE "\rDMA Register: NFC_DMA_CPR   0x11C %8.8x\n", readl(info->reg + NFC_DMA_CPR));
	printk(KERN_NOTICE "\rDMA Register: NFC_DMA_CCR   0x120 %8.8x\n", readl(info->reg + NFC_DMA_CCR));
	printk(KERN_NOTICE "\rECC Register: ECC MODE(23)  0x08C %8.8x\n",
	readl(info->reg + WMT_NFC_ECC_BCH_CTRL));
	printk(KERN_NOTICE "\rECC Register: OOB MODE(9)   0x024 %8.8x\n",
	readl(info->reg + WMT_NFC_SMC_ENABLE));

	printk(KERN_NOTICE "\rHarming ECC Register: WMT_NFC_REDUNT_ECC_STAT(0x7C) %8.8x\n",
	readb(info->reg + WMT_NFC_REDUNT_ECC_STAT));
	printk(KERN_NOTICE "\rHarming ECC Register: WMT_NFC_BANK18_ECC_STAT(0x80) %8.8x\n",
	readl(info->reg + WMT_NFC_BANK18_ECC_STAT));

	printk(KERN_NOTICE "\rBCH ECC Register: WMT_NFC_ECC_BCH_CTRL(0x8c) %8.8x\n",
	readl(info->reg + WMT_NFC_ECC_BCH_CTRL));
	printk(KERN_NOTICE "\rBCH ECC Register: WMT_NFC_ECC_BCH_INT_STAT1(0x94) %8.8x\n",
	readl(info->reg + WMT_NFC_ECC_BCH_INT_STAT1));
	printk(KERN_NOTICE "\rBCH ECC Register: WMT_NFC_ECC_BCH_INT_STAT2(0x98) %8.8x\n",
	readl(info->reg + WMT_NFC_ECC_BCH_INT_STAT2));
	printk(KERN_NOTICE "\rBCH ECC Register: WMT_NFC_ECC_BCH_ERR_POS1(0x9c) %8.8x\n",
	readl(info->reg + WMT_NFC_ECC_BCH_ERR_POS1));
	printk(KERN_NOTICE "\rBCH ECC Register: WMT_NFC_ECC_BCH_ERR_POS2(0xa0) %8.8x\n",
	readl(info->reg + WMT_NFC_ECC_BCH_ERR_POS2));

	printk(KERN_NOTICE "\rNFC Register: WMT_NFC_NAND_TYPE_SEL(0x48) %8.8x\n",
	readl(info->reg + WMT_NFC_NAND_TYPE_SEL));
	printk(KERN_NOTICE "\rNFC Register: WMT_NFC_MISC_CTRL(0x54) %8.8x\n",
	readl(info->reg + WMT_NFC_MISC_CTRL));
	printk(KERN_NOTICE "\rNFC Register: WMT_NFC_PAGESIZE_DIVIDER_SEL(0x5c) %8.8x\n",
	readl(info->reg + WMT_NFC_PAGESIZE_DIVIDER_SEL));
	
	for (j = 0; j < 0x130; j += 16)                                                 
			printk(KERN_NOTICE "NFCR%x ~ NFCR%x = 0x%8.8x 0x%8.8x 0x%8.8x 0x%8.8x\r\n",
			j/4, (j+12)/4,                                                             
			readl(info->reg + j + 0),                                                  
			readl(info->reg + j + 4),                                                  
			readl(info->reg + j + 8),                                                  
			readl(info->reg + j + 12));
}
#endif

/*
 * wmt_nand_cmdfunc - Send command to NAND large page device
 * @mtd:	MTD device structure
 * @command:	the command to be sent
 * @column:	the column address for this command, -1 if none
 * @page_addr:	the page address for this command, -1 if none
 *
 * Send command to NAND device. This is the version for the new large page
 * devices We dont have the separate regions as we have in the small page
 * devices.  We must emulate NAND_CMD_READOOB to keep the code compatible.
 */
static void wmt_nand_cmdfunc(struct mtd_info *mtd, unsigned command, int column, int page_addr)
{
	struct wmt_nand_info *info = wmt_nand_mtd_toinfo(mtd);
	struct nand_chip *chip = mtd->priv;
	unsigned int addr_cycle = 0, b2r_stat;
	int status = -1;
	unsigned int ecc_err_pos, bank_stat, redunt_stat, bank_stat1;
	int readcmd; /*add by vincent 20080805*/ /*Vincent 2008.11.3*/
	int mycolumn = column, mypage_addr = page_addr; /*add by vincent 20080805*/
	extern spinlock_t *nand_lock;
	DECLARE_COMPLETION(complete);
	#ifdef NAND_DEBUG
	printk(KERN_NOTICE "enter in wmt_nand_cmdfunc() command: %x column:%x, page_addr:%x\n",
	command, column, page_addr);
	#endif
	info->isr_cmd = command;
	info->phase = first4k218;
	if (readl(info->reg + WMT_NFC_ECC_BCH_CTRL) & DIS_BCH_ECC)
		info->phase = 2;
	/*printk(KERN_NOTICE "\rWMT_NFC_MISC_STAT_PORT: %x\n",
	readb(info->reg + WMT_NFC_MISC_STAT_PORT));*/
	/* Emulate NAND_CMD_READOOB and oob layout need to deal with specially */

	/*if (command == 0x80 || command == 0x60)
		for (j = 0; j < 0xA8; j += 16)                                                 
			printk(KERN_NOTICE "NFCR%x ~ NFCR%x = 0x%8.8x 0x%8.8x 0x%8.8x 0x%8.8x\r\n",
			j/4, (j+12)/4,                                                             
			readl(info->reg + j + 0),                                                  
			readl(info->reg + j + 4),                                                  
			readl(info->reg + j + 8),                                                  
			readl(info->reg + j + 12));*/
	if (((*(volatile unsigned long *)(GPIO_BASE_ADDR + 0x100))&6) == 2)
		spin_lock(nand_lock);
#if 1

	if (command == NAND_CMD_READOOB || command == NAND_CMD_READ0) {

	#ifdef PAGE_READ_COUNTER
	/*if (mtd->writesize == 512)*/
read_again:
	#endif


		if (command == NAND_CMD_READOOB) {
			#ifdef NAND_DEBUG
			printk(KERN_NOTICE "in wmt_nand_cmdfunc(): readoob column %x\n", column);
			#endif
			/*  memcpy(info->dmabuf + info->datalen, 0x00, 64);*/
			#ifndef NAND_HARMING_ECC
			if (ecc_type == 1) {
				writeb(0x07, info->reg + WMT_NFC_REDUNT_ECC_STAT_MASK);
				info->done_data = &complete;
				/*if (mtd->writesize == 2048)*/
					/*wmt_nfc_dma_cfg(mtd, 16, 0, -1, -1);*/
				/*printk(KERN_NOTICE "
				(ecc_type = 1 BCH support): page size 2048 readoob column %x\n", column);*/
				/*else*/ /*if (NAND_PAGE_SIZE == 4096)*/
					/*wmt_nfc_dma_cfg(mtd, 32, 0, -1, -1);*/
					/*printk(KERN_NOTICE " (ecc_type = 1 BCH support):
					page size 4096 readoob column %x\n", column);*/
			}	else {
			#endif
				/*printk(KERN_NOTICE
				"(ecc_type = 0 HARMING support): readoob column %x\n", column);*/
				/* redunt_read_hm_ecc_ctrl(info, 1);*/
				if (mtd->writesize == 2048) {
					column += mtd->writesize;
					wmt_nfc_dma_cfg(mtd, 64, 0, -1, -1);
				} else if (mtd->writesize == 4096) {
					column += mtd->writesize;
					wmt_nfc_dma_cfg(mtd, 128, 0, -1, -1);
				} else { /* page_size == 512 */
					/*  chip enable:  enable CE0*/
					/* write to clear B2R */
					b2r_stat = readb(info->reg + WMT_NFC_HOST_STAT_CHANGE);
					writeb(B2R|b2r_stat, info->reg + WMT_NFC_HOST_STAT_CHANGE);
					/* printk(KERN_NOTICE "READOOB: RB is %d\n", b2r_stat & 0x02);*/
	
					writeb(0xfe, info->reg + WMT_NFC_CHIP_ENABLE_CTRL);  /* select CE0 */
	
					/* printk(KERN_NOTICE "read oob--in wmt_nand_cmdfunc(): config dma\n");*/
							#ifdef PAGE_READ_COUNTER
					/* not use DMA, but must config transfer length */
					writew(16 - 1, info->reg + WMT_NFC_DMA_COUNTER);
					writeb(readb(info->reg + WMT_NFC_SMC_ENABLE) | 0x02,
					info->reg + WMT_NFC_SMC_ENABLE);
							#else
					//printk(KERN_NOTICE " read oob--in wmt_nand_cmdfunc(): config dma : NOT support read counter\n");
					wmt_nfc_dma_cfg(mtd, 16, 0, -1, -1);
							#endif
					/* info->datalen = mtd->writesize;*/
				}
			#ifndef NAND_HARMING_ECC
			}
			#endif
		} else {
		/* nand cmd read data area */
			if (mtd->writesize == 512) {
				/* write to clear B2R */
				b2r_stat = readb(info->reg + WMT_NFC_HOST_STAT_CHANGE);
				writeb(B2R|b2r_stat, info->reg + WMT_NFC_HOST_STAT_CHANGE);
				/* chip enable:  enable CE0*/
				writeb(0xfe, info->reg + WMT_NFC_CHIP_ENABLE_CTRL);
			}

			#ifdef PAGE_READ_COUNTER
			if (command == NAND_CMD_READ0)
				wmt_nfc_dma_cfg(mtd, 256, 0, -1, -1);  /* 1: read, 0:data, -1: */
			else
				wmt_nfc_dma_cfg(mtd, 256, 0, -1, 0xff);  /* 1: read, 0:data, -1: */
			#else
read_second4k218:
			info->dma_finish = 0;
			writeb(0x0F, info->reg + WMT_NFC_REDUNT_ECC_STAT_MASK);
			info->done_data = &complete;
			if (info->phase == second4k218) {
				addr_cycle = 0;
				column = second4k218;
				page_addr = mypage_addr;
				wmt_nfc_dma_cfg(mtd, ((mtd->writesize < 8192)? mtd->writesize : mtd->writesize/2), 0, -1, 2);
			} else
				/*printk(KERN_NOTICE "config dma: read page_addr:%x\n", page_addr);*/
				/* 1: read, 0:data, -1:  */
				/*printk(KERN_NOTICE "read data area column %x writesize %x\n", column, mtd->writesize);*/
				if (info->phase == 2) {
					wmt_nfc_dma_cfg(mtd, ((mtd->writesize < 8192)? mtd->writesize : (mtd->writesize+mtd->oobsize-(mtd->oobsize%16))), 0, -1, -1);
				} else
					wmt_nfc_dma_cfg(mtd, ((mtd->writesize < 8192)? mtd->writesize : mtd->writesize/2), 0, -1, -1);
				/*print_register(mtd);*/
			#endif
		}

		info->datalen = 0;
		/* write to clear B2R */
		b2r_stat = readb(info->reg + WMT_NFC_HOST_STAT_CHANGE);
		writeb(B2R|b2r_stat, info->reg + WMT_NFC_HOST_STAT_CHANGE);
		/* printk(KERN_NOTICE "RB is %d\n", b2r_stat & 0x02);*/

		if (column != -1) {
			writeb(column, info->reg + WMT_NFC_COMPORT1_2);
			addr_cycle++;
			/*#ifndef PAGE_ADDR*/
			if (mtd->writesize != 512) {
				writeb(column >> 8, (unsigned char *)(info->reg + WMT_NFC_COMPORT1_2) + 1);
				addr_cycle++;
			}
			/*#endif*/
			if (page_addr != -1) {
				/*#ifndef PAGE_ADDR*/
				if (mtd->writesize != 512) {
					writeb(page_addr, info->reg + WMT_NFC_COMPORT3_4);
					page_addr >>= 8;
					writeb(page_addr, (unsigned char *)(info->reg + WMT_NFC_COMPORT3_4) + 1);
					addr_cycle += 2;
					/*#else*/
				} else { 
					writeb(page_addr, (unsigned char *)(info->reg + WMT_NFC_COMPORT1_2) + 1);
					page_addr >>= 8;
					writeb(page_addr, info->reg + WMT_NFC_COMPORT3_4);
					addr_cycle += 2;
				}
				/*#endif*/

				if (mtd->writesize == 2048) {
					/* One more address cycle for devices > 128MiB */
					if (chip->chipsize > (128 << 20)) {
						page_addr >>= 8;
						/*#ifndef PAGE_ADDR*/
						if (mtd->writesize != 512)
							writeb(page_addr, info->reg + WMT_NFC_COMPORT5_6);
							/*#else*/
						else
							writeb(page_addr,
							(unsigned char *)(info->reg + WMT_NFC_COMPORT3_4) + 1);
						/*#endif*/
						addr_cycle++;
					}
				} else if (mtd->writesize == 4096) {
					/* One more address cycle for devices > 256MiB */
					if (chip->chipsize > (256 << 20)) {
						page_addr >>= 8;
						/*#ifndef PAGE_ADDR*/
						if (mtd->writesize != 512)
							writeb(page_addr, info->reg + WMT_NFC_COMPORT5_6);
							/*#else*/
						else
							writeb(page_addr,
							(unsigned char *)(info->reg + WMT_NFC_COMPORT3_4) + 1);
						/*#endif*/
						addr_cycle++;
					}
				} else if (mtd->writesize == 8192) {
					/* One more address cycle for devices > 512MiB */
					if (chip->chipsize > (512 << 20)) {
						page_addr >>= 8;
						/*#ifndef PAGE_ADDR*/
						if (mtd->writesize != 512)
							writeb(page_addr, info->reg + WMT_NFC_COMPORT5_6);
							/*#else*/
						else
							writeb(page_addr,
							(unsigned char *)(info->reg + WMT_NFC_COMPORT3_4) + 1);
						/*#endif*/
						addr_cycle++;
					}
				} else {/*page size 512*/
					/* One more address cycle for devices > 32MiB */
					if (chip->chipsize > (32 << 20)) {
						page_addr >>= 8;
						/*#ifndef PAGE_ADDR*/
						if (mtd->writesize != 512)
							writeb(page_addr, info->reg + WMT_NFC_COMPORT5_6);
						/*#else*/
						else
							writeb(page_addr,
							(unsigned char *)(info->reg + WMT_NFC_COMPORT3_4) + 1);
						/*#endif*/
						addr_cycle++;
					}
				}
			}
		/* } else if (page_addr != -1) {*/
		} else if ((page_addr != -1) && (column == -1)) {
			writeb(page_addr & 0xff, info->reg + WMT_NFC_COMPORT1_2);
			page_addr >>= 8;
			writeb(page_addr & 0xff, (unsigned char *)(info->reg + WMT_NFC_COMPORT1_2) + 1);
			addr_cycle += 2;

			if (mtd->writesize == 2048) {
				/* One more address cycle for devices > 128MiB */
				if (chip->chipsize > (128 << 20)) {
					page_addr >>= 8;
					writeb(page_addr & 0xff,
					info->reg + WMT_NFC_COMPORT3_4);
					addr_cycle++;
				}
			} else if (mtd->writesize == 4096) {
				/* One more address cycle for devices > 256MiB */
				if (chip->chipsize > (256 << 20)) {
					page_addr >>= 8;
					writeb(page_addr & 0xff,
					info->reg + WMT_NFC_COMPORT3_4);
					addr_cycle++;
				}
			} else if (mtd->writesize == 8192) {
				/* One more address cycle for devices > 512MiB */
				if (chip->chipsize > (512 << 20)) {
					page_addr >>= 8;
					writeb(page_addr & 0xff,
					info->reg + WMT_NFC_COMPORT3_4);
					addr_cycle++;
				}
			} else {/*page size = 512 bytes */
				/* One more address cycle for devices > 32MiB */
				if (chip->chipsize > (32 << 20)) {

					/* One more address cycle for devices > 128MiB */
					/* if (chip->chipsize > (128 << 20)) {*/
					page_addr >>= 8;
					/*  writeb(page_addr,
					info->reg + WMT_NFC_COMPORT3_4 + 1); */
					/* before, may be a little error */
					writeb(page_addr & 0xff,
					info->reg + WMT_NFC_COMPORT3_4);
					addr_cycle++;
				}
			}
	} 
	#ifdef NAND_HARMING_ECC /* HAMMing ECC */
	writeb(0x07, info->reg + WMT_NFC_REDUNT_ECC_STAT);
	writel(0xffffffff, info->reg + WMT_NFC_BANK18_ECC_STAT);
	#else  /*Vincent 2008.11.3*/
	bank_stat1 = readw(info->reg + WMT_NFC_ECC_BCH_INT_STAT1);
	writew(bank_stat1|0x101, info->reg + WMT_NFC_ECC_BCH_INT_STAT1);
	#endif

	if (mtd->writesize == 512) {
		/* printk(KERN_NOTICE "pagesize=512 command mode\n");*/
		/* printk(KERN_NOTICE "command is %x\n", command);*/
		writeb(command, info->reg + WMT_NFC_COMPORT0);
		writeb(NAND2NFC | ((addr_cycle + 1)<<1)|NFC_TRIGGER,
		info->reg + WMT_NFC_COMCTRL);

		status = wmt_nand_ready(mtd);
		if (status)	{
			printk(KERN_ERR "nand flash is not ready : %x\n",
			readb(info->reg + WMT_NFC_MISC_STAT_PORT));
			/*print_register(mtd);*/
			writeb(readb(info->reg + WMT_NFC_SMC_ENABLE) & 0xfd,
			info->reg + WMT_NFC_SMC_ENABLE);
			/* return;*/
		}
		status = wmt_nfc_transfer_ready(mtd);
		/*status = wmt_wait_dma_ready(mtd);*/
		if (status) {
			printk(KERN_ERR "wait transfer command and data is not finished : %x\n",
			readb(info->reg + WMT_NFC_MISC_STAT_PORT));
			/*print_register(mtd);*/
			writew(readw(info->reg + WMT_NFC_ECC_BCH_CTRL) | READ_RESUME,
			info->reg + WMT_NFC_ECC_BCH_CTRL);
			writeb(readb(info->reg + WMT_NFC_SMC_ENABLE) & 0xfd,
			info->reg + WMT_NFC_SMC_ENABLE);
			/*  while (1);*/
			/*  printk(KERN_NOTICE "dma transfer is not ready\n");*/
			/*  return;*/
		}
		/*  chip disable:  disable CE*/
		/* printk(KERN_NOTICE "Disable CE0-------------------\n");*/
		/* writeb(0xff, info->reg + WMT_NFC_CHIP_ENABLE_CTRL);*/  /* disable CE0 */
	
		status = wmt_nfc_wait_idle(mtd, 1, -1, -1, -1);
		if (status) {
			printk(KERN_ERR "wait transfer data and nfc is not idle : %x\n",
			readb(info->reg + WMT_NFC_MISC_STAT_PORT));
			/*print_register(mtd);*/
			writew(readw(info->reg + WMT_NFC_ECC_BCH_CTRL) | READ_RESUME,
			info->reg + WMT_NFC_ECC_BCH_CTRL);
			writeb(readb(info->reg + WMT_NFC_SMC_ENABLE) & 0xfd,
			info->reg + WMT_NFC_SMC_ENABLE);
			/* return;*/
		}
		/*print_register(mtd);*/
		if (!(readw(info->reg + WMT_NFC_SMC_ENABLE)&2)) {
			status = nand_pdma_handler(mtd);
			/*printk(KERN_ERR "check status pdma handler status= %x \n", status);*/
			nand_free_pdma(mtd);
			if (status)
				printk(KERN_ERR "dma transfer data time out: %x\n",
				readb(info->reg + WMT_NFC_MISC_STAT_PORT));
		}	else
			printk(KERN_NOTICE "WMT_NFC_SMC_ENABLE = 2\n");
		/* disable CE0 */
		writeb(0xff, info->reg + WMT_NFC_CHIP_ENABLE_CTRL);
	} else { /* else of page size = 512 */
		/* printk(KERN_NOTICE "\r ECC OOB MODE(9)0x024 %8.8x\n",
		readl(info->reg + WMT_NFC_SMC_ENABLE));*/
		#ifdef NAND_HARMING_ECC/*Vincent 2008.11.4*/

		/*printk( "page2k/4k, command =0x%x\n", command);*/
		status = wmt_wait_chip_ready(mtd); /*Vincent 2008.11.3*/
		if (status)
			printk(KERN_ERR "The chip is not ready\n");
	
		writeb(NAND_CMD_READ0, info->reg + WMT_NFC_COMPORT0);
		writeb(DPAHSE_DISABLE|((addr_cycle + 1)<<1)|NFC_TRIGGER,
		info->reg + WMT_NFC_COMCTRL);
		/* wait all command + address sequence finish status */
		status = wmt_wait_cmd_ready(mtd);
		/* status = wmt_nfc_ready(mtd); */
		if (status)	{
			/* printk(KERN_ERR "wait transfer command is not ready : %x\n",
			readb(info->reg + WMT_NFC_MISC_STAT_PORT));*/
			/*print_register(mtd);*/
			/* writew(readw(info->reg + WMT_NFC_ECC_BCH_CTRL) | READ_RESUME,
			info->reg + WMT_NFC_ECC_BCH_CTRL);*/
			/* while (1);*/
			printk(KERN_NOTICE "dma transfer is not ready 2k or 4k page\n");
			/* return;*/
		}
		/* wait device idle 1: don't check ecc*/
		status = wmt_nfc_wait_idle(mtd, 1, -1, -1, -1);
		/*print_register(mtd);*/
		if (status)	{
			printk(KERN_ERR "wait transfer command and nfc is not idle : %x\n",
			readb(info->reg + WMT_NFC_MISC_STAT_PORT));
			/*print_register(mtd);*/
			writew(readw(info->reg + WMT_NFC_ECC_BCH_CTRL) | READ_RESUME,
			info->reg + WMT_NFC_ECC_BCH_CTRL);
			/* while (1);*/
			/* printk(KERN_NOTICE "dma transfer is not ready\n");*/
			/* return;*/
		}

		/* write to clear B2R */
		b2r_stat = readb(info->reg + WMT_NFC_HOST_STAT_CHANGE);
		writeb(B2R|b2r_stat, info->reg + WMT_NFC_HOST_STAT_CHANGE);
	
		writeb(NAND_CMD_READSTART, info->reg + WMT_NFC_COMPORT0);
		writeb(NAND2NFC|(1<<1)|NFC_TRIGGER, info->reg + WMT_NFC_COMCTRL);
		/*print_register(mtd);*/
		/*wait busy to ready int status*/
		status = wmt_nand_ready(mtd);
		if (status) {
			printk(KERN_ERR "readstart: nand flash is not ready\n");
			/*print_register(mtd);*/
			writew(readw(info->reg + WMT_NFC_ECC_BCH_CTRL) | READ_RESUME,
			info->reg + WMT_NFC_ECC_BCH_CTRL);
			/* while (1);*/
			/* return;*/
		}
		wmt_wait_nfc_ready(info); /*Vincent 2008.11.3*/
		/* printk(KERN_NOTICE "\rreadstart: nand flash is ready\n");*/
		#if 0
		status = wmt_nfc_transfer_ready(mtd);
		if (status)	{
			printk(KERN_ERR "wait transfer data is not ready: %x\n",
			readb(info->reg + WMT_NFC_MISC_STAT_PORT));
			print_register(mtd);
			while (1)
				;
			return;
		}
		#endif

		#ifdef NAND_BBT_BCH_ECC
		if (ecc_type == 0) {
		#endif
			/* use HAMMING ECC */
			/*printk(KERN_ERR "use HAMMing to read command = %x \n", command);*/
			status = wmt_nfc_wait_idle(mtd, 2, command, mycolumn, mypage_addr);
	
		#ifdef NAND_BBT_BCH_ECC
		}	else {
			/* use BCH ECC */
			/*print_register(mtd);*/
			/*printk(KERN_ERR "use BCH to read command = %x \n", command);*/
			status = wmt_nfc_wait_idle(mtd, 0, command, mycolumn, mypage_addr);
			if (command == NAND_CMD_READOOB) {
				/* disable side info read operation */
				/* disable_redunt_out_bch_ctrl(info, 0);*/
				writeb(readb(info->reg + WMT_NFC_SMC_ENABLE) & 0xfd,
				info->reg + WMT_NFC_SMC_ENABLE);
				/* writeb(readb(info->reg + WMT_NFC_CALC_CTRL) &0xFFFFFFFD,
				info->reg + WMT_NFC_CALC_CTRL);*/ /*Vincent 2008.11.3*/
			}
		}
		#endif

		if (status) {
			if (status == -4)
				return;
			printk(KERN_ERR "wmt_nfc_wait_idle status =%d\n", status);
			printk(KERN_ERR "command =0x%x\n", command);
			printk(KERN_ERR "Read ERR ,NFC is not idle\n");
			print_register(mtd);
			writew(readw(info->reg + WMT_NFC_ECC_BCH_CTRL) | READ_RESUME,
			info->reg + WMT_NFC_ECC_BCH_CTRL);
			/* while (1);*/
			return;
		}

		/*print_register(mtd);*/
		if (!(readw(info->reg + WMT_NFC_SMC_ENABLE)&2)) {
			status = nand_pdma_handler(mtd);
			/*printk(KERN_ERR "check status pdma handler status= %x \n", status);*/
			nand_free_pdma(mtd);
			if (status)
				return;
		}

		#else /* Vincent 2008.11.4 #else of #ifdef NAND_HARMING_ECC */

		b2r_stat = readb(info->reg + WMT_NFC_CHIP_ENABLE_CTRL);
		if ((b2r_stat&3) == 3) {
			printk(KERN_NOTICE "chip 0, or 1, is not select chip_sel=%x\n", b2r_stat);
			writeb(0xfe, info->reg + WMT_NFC_CHIP_ENABLE_CTRL);
		}
		status = wmt_wait_chip_ready(mtd); /*Vincent 2008.11.3*/
		if (status)
			printk(KERN_ERR "The chip is not ready\n");
		writeb(NAND_CMD_READ0, info->reg + WMT_NFC_COMPORT0);
		if (addr_cycle == 4)
			writeb(NAND_CMD_READSTART, info->reg + WMT_NFC_COMPORT5_6);
		else if (addr_cycle == 5)
			writeb(NAND_CMD_READSTART, (unsigned char *)(info->reg + WMT_NFC_COMPORT5_6) + 1);
	
		writeb(NAND2NFC|MUL_CMDS|((addr_cycle + 2)<<1)|NFC_TRIGGER, info->reg + WMT_NFC_COMCTRL);

#if 0
		status = wmt_nand_ready(mtd);
		if (redunt_err_mark == 2) {
			redunt_err_mark = 3;
			disable_redunt_out_bch_ctrl(info, 1); /* disable redundant output */
		}

		if (status) {
			printk(KERN_ERR "readstart: nand flash is not ready\n");
			/*print_register(mtd);*/
			writew(readw(info->reg + WMT_NFC_ECC_BCH_CTRL) | READ_RESUME,
			info->reg + WMT_NFC_ECC_BCH_CTRL);
			/* while(1);*/
			/* return;*/
		}
		
		wmt_wait_nfc_ready(info); /*Vincent 2008.11.3*/
#endif		
		if (!(readw(info->reg + WMT_NFC_SMC_ENABLE)&2)) {
			//printk("read wait for completion");
			wait_for_completion_timeout(&complete, NFC_TIMEOUT_TIME);
			status = nand_pdma_handler(mtd);
			/*printk(KERN_ERR "check status pdma handler status= %x \n", status);*/
			nand_free_pdma(mtd);
			if (status)
				printk(KERN_ERR "dma transfer data time out: %x\n",
				readb(info->reg + WMT_NFC_MISC_STAT_PORT));

			wmt_nfc_transfer_ready(mtd);
			status = wmt_nand_ready(mtd);
			if (status)
				printk(KERN_NOTICE"B2R not clear status=0x%x\n", status);
			writeb(0x80, info->reg + WMT_NFC_REDUNT_ECC_STAT_MASK);	
		} else {	/* read oob has no dma but assert B2R status */
			wait_for_completion_timeout(&complete, NFC_TIMEOUT_TIME);
			status = wmt_nfc_transfer_ready(mtd);
			if (status)
				printk(KERN_NOTICE"wait NFC_BUSY time out\n");
			//wmt_nand_ready(mtd);
			writeb(0x80, info->reg + WMT_NFC_REDUNT_ECC_STAT_MASK);	
		}
		status = wmt_nfc_wait_idle(mtd, 0, command, mycolumn, mypage_addr);
		if (command == NAND_CMD_READOOB) {
			/* disable_redunt_out_bch_ctrl(info, 0);*/
			/* writeb(readb(info->reg + WMT_NFC_CALC_CTRL) &0xFFFFFFFD ,
			info->reg + WMT_NFC_CALC_CTRL);*/ /*Vincent 2008.11.3*/
		}

		if (status) {
			if (status == -4)
				return;
			printk(KERN_ERR "wmt_nfc_wait_idle status =%d\n", status);
			printk(KERN_ERR "command =0x%x\n", command);
			printk(KERN_ERR "Read ERR ,NFC is not idle\n");
			/*print_register(mtd);*/
			writew(readw(info->reg + WMT_NFC_ECC_BCH_CTRL) | READ_RESUME,
			info->reg + WMT_NFC_ECC_BCH_CTRL);
			/*while(1);*/
			return;
		}

		if (mtd->writesize >= 8192 && (column == 0 && command == NAND_CMD_READ0) && info->phase == first4k218) {
			info->phase = second4k218;
			goto read_second4k218;
		}
		#endif/*Vincent 2008.11.4*/
	} /* end of else of page size = 512 */

	if (mtd->writesize == 512) {
		#ifdef PAGE_READ_COUNTER
		addr_cycle = 0;
		if (command == NAND_CMD_READ0) {
			command = NAND_CMD_READ1;
			goto read_again;
		}	else if (command == NAND_CMD_READ1) {
			command = NAND_CMD_READOOB;
			goto read_again;
		}
		#endif
	}


	#ifdef NAND_BBT_BCH_ECC
	if (ecc_type == 0) {
	#endif
		if (mtd->writesize == 512) {
			/*printk(KERN_NOTICE "2 addr_cycle:%x\n",addr_cycle);*/
			/*for (j = 0; j < 0xA8; j += 16)                                                 
			printk(KERN_NOTICE "NFCR%x ~ NFCR%x = 0x%8.8x 0x%8.8x 0x%8.8x 0x%8.8x\r\n",
			j/4, (j+12)/4,                                                             
			readl(info->reg + j + 0),                                                  
			readl(info->reg + j + 4),                                                  
			readl(info->reg + j + 8),                                                  
			readl(info->reg + j + 12));*/
			/* use HAMMING ECC but page not 512 and read oob area */
			#ifdef NAND_DEBUG
			printk(KERN_NOTICE "in wmt_nand_cmdfunc(): Read oob data \n");
			#endif
			redunt_stat = readb(info->reg + WMT_NFC_REDUNT_ECC_STAT);

			if (redunt_stat) {
				printk(KERN_NOTICE
				" Read OOB redundant ecc eror command: %x column:%x, page_addr:%x\n",
				command, mycolumn, mypage_addr);
				printk(KERN_NOTICE "redunt_stat:%x\n", redunt_stat);
			}
			if (redunt_stat & 0x05) {
					printk(KERN_ERR "There are uncorrected ecc error in reduntant area--\n");
					mtd->ecc_stats.failed++;
					return;
			} else if (redunt_stat & 0x02) {
				#ifdef NAND_DEBUG
				printk(KERN_WARNING "There are 1 bit ecc error in reduntant data area--\n");
				#endif
				/* Vincent  modify 2008.10.13*/
				ecc_err_pos = readw(info->reg + WMT_NFC_REDUNT_AREA_PARITY_STAT);
				bit_correct((unsigned char *)info->reg+ECC_FIFO_0 + (ecc_err_pos & 0x3f),
				(ecc_err_pos >> 8) & 0x07);
				/* mtd->ecc_stats.corrected++;*/
			}

			/* read data area with hamming ecc correct */
			/* use HAMMING ECC but page not 512 and read data area */
			#ifdef NAND_DEBUG
			printk(KERN_NOTICE "in wmt_nand_cmdfunc(): Read data \n");
			#endif

			bank_stat = readl(info->reg + WMT_NFC_BANK18_ECC_STAT);
			redunt_stat = readb(info->reg + WMT_NFC_REDUNT_ECC_STAT);
			/* memcpy(chip->oob_poi, info->reg+ECC_FIFO_0, 64);*/
			if ((bank_stat&0x77) || redunt_stat) {
				printk(KERN_NOTICE " Read data ecc eror command: %x column:%x, page_addr:%x\n",
				command, column, mypage_addr);
				printk(KERN_NOTICE "error block addr: 0x%x page_addr:0x%x\n",
				mypage_addr>>(chip->phys_erase_shift - chip->page_shift), mypage_addr&0x3F);
				printk(KERN_NOTICE " bank_stat:0x%x, redunt_stat:0x%x\n",
				bank_stat, redunt_stat);
			}
			nand_hamming_ecc_1bit_correct(mtd);

			/*for (j = 0; j < 0xA8; j += 16)                                                 
			printk(KERN_NOTICE "NFCR%x ~ NFCR%x = 0x%8.8x 0x%8.8x 0x%8.8x 0x%8.8x\r\n",
			j/4, (j+12)/4,                                                             
			readl(info->reg + j + 0),                                                  
			readl(info->reg + j + 4),                                                  
			readl(info->reg + j + 8),                                                  
			readl(info->reg + j + 12));*/
			//while(j);
		} else { /*NAND_PAGE_SIZE != 512)*/
			if (command == NAND_CMD_READOOB) {
				/* use HAMMING ECC but page not 512 and read oob area */
				#ifdef NAND_DEBUG
				printk(KERN_NOTICE "in wmt_nand_cmdfunc(): Read oob data \n");
				#endif
				redunt_stat = readb(info->reg + WMT_NFC_REDUNT_ECC_STAT);
	
				if (redunt_stat) {
					printk(KERN_NOTICE
					" Read OOB redundant ecc eror command: %x column:%x, page_addr:%x\n",
					command, mycolumn, mypage_addr);
					printk(KERN_NOTICE "redunt_stat:%x\n", redunt_stat);
				}
				if (redunt_stat & 0x05) {
						printk(KERN_ERR "There are uncorrected ecc error in reduntant area--\n");
						mtd->ecc_stats.failed++;
						return;
				} else if (redunt_stat & 0x02) {
					#ifdef NAND_DEBUG
					printk(KERN_WARNING "There are 1 bit ecc error in reduntant data area--\n");
					#endif
					/* Vincent  modify 2008.10.13*/
					ecc_err_pos = readw(info->reg + WMT_NFC_REDUNT_AREA_PARITY_STAT);
					bit_correct((unsigned char *)info->reg+ECC_FIFO_0 + (ecc_err_pos & 0x3f),
					(ecc_err_pos >> 8) & 0x07);
					/* mtd->ecc_stats.corrected++;*/
				}
			}	else {
				/* read data area with hamming ecc correct */
				/* use HAMMING ECC but page not 512 and read data area */
				#ifdef NAND_DEBUG
				printk(KERN_NOTICE "in wmt_nand_cmdfunc(): Read data \n");
				#endif
	
				bank_stat = readl(info->reg + WMT_NFC_BANK18_ECC_STAT);
				redunt_stat = readb(info->reg + WMT_NFC_REDUNT_ECC_STAT);
				/* memcpy(chip->oob_poi, info->reg+ECC_FIFO_0, 64);*/
				if ((bank_stat&0x77777777) || redunt_stat) {
					printk(KERN_NOTICE " Read data ecc eror command: %x column:%x, page_addr:%x\n",
					command, column, mypage_addr);
					printk(KERN_NOTICE "error block addr: 0x%x page_addr:0x%x\n",
					mypage_addr>>(chip->phys_erase_shift - chip->page_shift), mypage_addr&0x3F);
					printk(KERN_NOTICE " bank_stat:0x%x, redunt_stat:0x%x\n",
					bank_stat, redunt_stat);
				}
				nand_hamming_ecc_1bit_correct(mtd);
			}
		}
	#ifdef NAND_BBT_BCH_ECC
	}
	#endif
	return;
	}/* end of if (command == NAND_CMD_READOOB || command == NAND_CMD_READ0) { */
#endif /* end of #if 1 */

	switch (command) {
	case NAND_CMD_SEQIN:
		/*add by vincent 20080805*/
		if (mtd->writesize  == 512) {
			if (column >= mtd->writesize) {
				/* OOB area */
				column -= mtd->writesize;
				readcmd = NAND_CMD_READOOB;
			} else if (column < 256) {
				/* First 256 bytes --> READ0 */
				readcmd = NAND_CMD_READ0;
			} else {
				column -= 256;
				readcmd = NAND_CMD_READ1;
			}
			writeb(readcmd, info->reg + WMT_NFC_COMPORT0);
			writeb(DPAHSE_DISABLE | (1<<1) | NFC_TRIGGER, info->reg + WMT_NFC_COMCTRL);
			wmt_wait_nfc_ready(info);
		}

	/*add by vincent 20080805*/
	case NAND_CMD_ERASE1:
		/* printk(KERN_NOTICE "command is %x\n", command);*/
		if (column != -1) {
			writeb(column, info->reg + WMT_NFC_COMPORT1_2);
			addr_cycle++;
			/*#ifndef PAGE_ADDR*/
			if (mtd->writesize != 512) {
				writeb(column >> 8, (unsigned char *)(info->reg + WMT_NFC_COMPORT1_2) + 1);
				addr_cycle++;
			}/*#endif*/
			if (page_addr != -1) {
				/*#ifndef PAGE_ADDR*/
				if (mtd->writesize != 512) {
					writeb(page_addr, info->reg + WMT_NFC_COMPORT3_4);
					page_addr >>= 8;
					writeb(page_addr, (unsigned char *)(info->reg + WMT_NFC_COMPORT3_4) + 1);
					addr_cycle += 2;
				/*#else*/
				} else {
					writeb(page_addr, (unsigned char *)(info->reg + WMT_NFC_COMPORT1_2) + 1);
					page_addr >>= 8;
					writeb(page_addr, info->reg + WMT_NFC_COMPORT3_4);
					addr_cycle += 2;
				} /*#endif*/

				if (mtd->writesize == 2048) {
				/* One more address cycle for devices > 128MiB */
					if (chip->chipsize > (128 << 20)) {
						page_addr >>= 8;
						/*#ifndef PAGE_ADDR*/
						if (mtd->writesize != 512)
							writeb(page_addr, info->reg + WMT_NFC_COMPORT5_6);
						else /*#else*/
							writeb(page_addr, (unsigned char *)(info->reg + WMT_NFC_COMPORT3_4) + 1);
						/*#endif*/
						addr_cycle++;
					}
				} else if (mtd->writesize == 4096) {
					/* One more address cycle for devices > 256MiB */
					if (chip->chipsize > (256 << 20)) {
						page_addr >>= 8;
						/*#ifndef PAGE_ADDR*/
						if (mtd->writesize != 512)
							writeb(page_addr, info->reg + WMT_NFC_COMPORT5_6);
						else /*#else*/
							writeb(page_addr, (unsigned char *)(info->reg + WMT_NFC_COMPORT3_4) + 1);
						/*#endif*/
						addr_cycle++;
					}
				} else if (mtd->writesize == 8192) {
					/* One more address cycle for devices > 512MiB */
					if (chip->chipsize > (512 << 20)) {
						page_addr >>= 8;
						if (mtd->writesize != 512)
							writeb(page_addr, info->reg + WMT_NFC_COMPORT5_6);
						addr_cycle++;
					}
				} else {
					/* One more address cycle for devices > 32MiB */
					if (chip->chipsize > (32 << 20)) {
						page_addr >>= 8;
						/*#ifndef PAGE_ADDR*/
						if (mtd->writesize != 512)
							writeb(page_addr, info->reg + WMT_NFC_COMPORT5_6);
						else /*#else*/
							writeb(page_addr, (unsigned char *)(info->reg + WMT_NFC_COMPORT3_4) + 1);
						/*#endif*/
						addr_cycle++;
					}
				}
			}
		/*} else if (page_addr != -1) {*/
		} else if ((page_addr != -1) && (column == -1)) {
			writeb(page_addr & 0xff, info->reg + WMT_NFC_COMPORT1_2);
			page_addr >>= 8;
			writeb(page_addr & 0xff, (unsigned char *)(info->reg + WMT_NFC_COMPORT1_2) + 1);
			addr_cycle += 2;

			if (mtd->writesize == 2048) {
				/* One more address cycle for devices > 128MiB */
				if (chip->chipsize > (128 << 20)) {
					page_addr >>= 8;
					writeb(page_addr, info->reg + WMT_NFC_COMPORT3_4);
					addr_cycle++;
				}
			} else if (mtd->writesize == 4096) {
				/* One more address cycle for devices > 256MiB */
				if (chip->chipsize > (256 << 20)) {
					page_addr >>= 8;
					writeb(page_addr, info->reg + WMT_NFC_COMPORT3_4);
					addr_cycle++;
				}
			} else if (mtd->writesize == 8192) {
				/* One more address cycle for devices > 512MiB */
				if (chip->chipsize > (512 << 20)) {
					page_addr >>= 8;
					writeb(page_addr, info->reg + WMT_NFC_COMPORT3_4);
					addr_cycle++;
				}
			} else {
				/* One more address cycle for devices > 32MiB */
				if (chip->chipsize > (32 << 20)) {
					page_addr >>= 8;
					writeb(page_addr, info->reg + WMT_NFC_COMPORT3_4);
					addr_cycle++;
				}
			}
		}

		/* set command 1 cycle */
		writeb(command, info->reg + WMT_NFC_COMPORT0);
		if (command == NAND_CMD_SEQIN) {
			info->done_data = &complete;
			writeb(((addr_cycle + 1)<<1)|NFC_TRIGGER, info->reg + WMT_NFC_COMCTRL);
		} else {
			/* writeb(read(info->reg + WMT_NFC_NAND_TYPE_SEL) | WP_DISABLE ,
			info->reg + WMT_NFC_NAND_TYPE_SEL);*/
			writeb(DPAHSE_DISABLE|((addr_cycle + 1)<<1)|NFC_TRIGGER,
			info->reg + WMT_NFC_COMCTRL);
		}

		if (command == NAND_CMD_ERASE1) {
			status = wmt_wait_cmd_ready(mtd);
			/* status = wmt_nfc_ready(mtd); */
			if (status)
					printk(KERN_ERR "command is not ready\n");
					writew(readw(info->reg + WMT_NFC_ECC_BCH_CTRL) | READ_RESUME,
					info->reg + WMT_NFC_ECC_BCH_CTRL);
		}	else {
			/* wmt_nfc_transfer_ready(mtd);*/
			#ifdef NAND_HARMING_ECC
			status = 0;
			status = wmt_nfc_transfer_ready(mtd);
			#else /*Vincent 2008.11.4*/
			wmt_wait_nfc_ready(info);
			status = wmt_nfc_transfer_ready(mtd);
			wait_for_completion_timeout(&complete, NFC_TIMEOUT_TIME);
			/*status = wmt_wait_dma_ready(mtd);*/ /*dannier mask*/
			#endif
			if (status)	{
				printk(KERN_ERR "dma transfer data is not ready: %x\n",
				readb(info->reg + WMT_NFC_MISC_STAT_PORT));
				writew(readw(info->reg + WMT_NFC_ECC_BCH_CTRL) | READ_RESUME,
				info->reg + WMT_NFC_ECC_BCH_CTRL);
				/*printk(KERN_NOTICE "\rwait transfer data is not ready: %x\n",
				readb(info->reg + WMT_NFC_MISC_STAT_PORT));*/
				/*print_register(mtd);*/
				/* while (1);*/
				/* return;*/
			}
			/* if (command == NAND_CMD_SEQIN)*/
			/* wmt_wait_dma_ready(mtd);*/
		}
		return;

	case NAND_CMD_PAGEPROG:
		/* case NAND_CMD_READSTART:*/
	case NAND_CMD_ERASE2:
		/* printk(KERN_NOTICE "command is %x\n", command);*/
		writeb(command, info->reg + WMT_NFC_COMPORT0);
		b2r_stat = readb(info->reg + WMT_NFC_HOST_STAT_CHANGE);
		if (B2R&b2r_stat) {
			printk(KERN_NOTICE"flash B2R status assert command=0x%x\n",command);
			writeb(B2R|b2r_stat, info->reg + WMT_NFC_HOST_STAT_CHANGE);
			status = wmt_wait_chip_ready(mtd); /*Vincent 2008.11.3*/
			if (status)
				printk(KERN_NOTICE"The chip is not ready\n");
		}

		if (NAND_CMD_ERASE2 == command)
			writeb(0x07, info->reg + WMT_NFC_REDUNT_ECC_STAT_MASK);
		info->done_data = &complete;
		writeb(DPAHSE_DISABLE|(1<<1)|NFC_TRIGGER, info->reg + WMT_NFC_COMCTRL);

		info->datalen = 0;
		wait_for_completion_timeout(&complete, NFC_TIMEOUT_TIME);
		writeb(0x80, info->reg + WMT_NFC_REDUNT_ECC_STAT_MASK);
		#if 0  /* for debug */
		if (command == NAND_CMD_ERASE2) {
			wmt_read_nand_status(mtd, NAND_CMD_STATUS);
			if ((readb(info->reg + WMT_NFC_DATAPORT) & 0xff) == 0xc0) {
				printk(KERN_NOTICE "wmt_func: erase block OK\n");
				printk(KERN_NOTICE "read nand status is %x\n",
				readb(info->reg + WMT_NFC_DATAPORT) & 0xff);
			}	else
				printk(KERN_NOTICE "wmt_func: erase block failed\n");
		}
		#endif

		status = wmt_nfc_wait_idle(mtd, 1, 1, -1, -1); /* write page, don't check ecc */
		if (status < 0) {
			printk(KERN_ERR "page program or erase err, nand controller is not idle\n");
			/*print_register(mtd);*/
			/* writew(readw(info->reg + WMT_NFC_ECC_BCH_CTRL) | READ_RESUME,
			info->reg + WMT_NFC_ECC_BCH_CTRL);*/
			/* while (1);*/
			/* return;*/
			/* }*/
			#if 0
			status = wmt_read_nand_status(mtd, NAND_CMD_STATUS);
			if (status < 0)
				printk(KERN_NOTICE "\rNFC or NAND is not ready\n");
			else if (status & NAND_STATUS_FAIL)
				printk(KERN_NOTICE "\r status : fail\n");
			else if (!(status & NAND_STATUS_READY))
				printk(KERN_NOTICE "\r status : busy\n");
			else if (!(status & NAND_STATUS_WP))
				printk(KERN_NOTICE "\r status : protect\n");
			#endif
			return;
		}

		return;

	case NAND_CMD_RESET:

		if (!chip->dev_ready)
			break;
		udelay(chip->chip_delay);
		writeb(command, info->reg + WMT_NFC_COMPORT0);
		/* write to clear B2R */
		b2r_stat = readb(info->reg + WMT_NFC_HOST_STAT_CHANGE);
		writeb(B2R|b2r_stat, info->reg + WMT_NFC_HOST_STAT_CHANGE);

		writeb(DPAHSE_DISABLE|(0x01<<1)|NFC_TRIGGER, info->reg + WMT_NFC_COMCTRL);
		status = wmt_nand_ready(mtd);
		if (status) {
			printk(KERN_ERR "Reset err, nand device is not ready\n");
			writew(readw(info->reg + WMT_NFC_ECC_BCH_CTRL) | READ_RESUME,
			info->reg + WMT_NFC_ECC_BCH_CTRL);
		}

		wmt_read_nand_status(mtd, NAND_CMD_STATUS);
		/*  while (!(chip->read_byte(mtd) & NAND_STATUS_READY));*/
		while (!((readb(info->reg + WMT_NFC_DATAPORT) & 0xff) & NAND_STATUS_READY))
			;
		#ifdef NAND_DEBUG
		printk(KERN_NOTICE "Reset status is ok\n");
		#endif
		return;

	case NAND_CMD_READID:

		status = wmt_nand_readID(mtd);
		#ifdef NAND_DEBUG
		printk(KERN_NOTICE "readID status is %d\n", status);
		#endif
		return;

	case NAND_CMD_STATUS:

		wmt_read_nand_status(mtd, command);
		return;

	case NAND_CMD_RNDIN:
		if (column != -1) {
			writeb(column, info->reg + WMT_NFC_COMPORT1_2);
			addr_cycle++;
			if (mtd->writesize != 512) {
				writeb(column >> 8, (unsigned char *)(info->reg + WMT_NFC_COMPORT1_2) + 1);
				addr_cycle++;
			}
		}
		info->done_data = &complete;
		/* set command 1 cycle */
		writeb(command, info->reg + WMT_NFC_COMPORT0);

		writeb(((addr_cycle + 1)<<1)|NFC_TRIGGER, info->reg + WMT_NFC_COMCTRL);
		wait_for_completion_timeout(&complete, NFC_TIMEOUT_TIME);
		status = wmt_nfc_wait_idle(mtd, 1, -1, -1, -1); /* don't check ecc, wait nfc idle */
		/*  status = wmt_wait_cmd_ready(mtd);*/
		/* status = wmt_nfc_ready(mtd);*/
		if (status)
			printk(KERN_ERR "Ramdom input err: nfc is not idle\n");

		return;

	case NAND_CMD_RNDOUT:

		if (column != -1) {
			writeb(column, info->reg + WMT_NFC_COMPORT1_2);
			writeb(column, info->reg + WMT_NFC_COMPORT1_2 + 1);
			addr_cycle += 2;
		}

		/* CLEAR ECC BIT */
		writeb(0x07, info->reg + WMT_NFC_REDUNT_ECC_STAT);
		writel(0xffffffff, info->reg + WMT_NFC_BANK18_ECC_STAT);
		/* write to clear B2R */
		b2r_stat = readb(info->reg + WMT_NFC_HOST_STAT_CHANGE);
		writeb(B2R|b2r_stat, info->reg + WMT_NFC_HOST_STAT_CHANGE);

		/* set command 1 cycle */
		writeb(command, info->reg + WMT_NFC_COMPORT0);

		writeb(DPAHSE_DISABLE|((addr_cycle + 1)<<1)|NFC_TRIGGER, info->reg + WMT_NFC_COMCTRL);

		status = wmt_wait_cmd_ready(mtd);
		/* status = wmt_nfc_ready(mtd);*/
		if (status) {
			printk(KERN_ERR "Ramdom output err: nfc command is not ready\n");
			/* return;*/
		}

		writeb(NAND_CMD_RNDOUTSTART, info->reg + WMT_NFC_COMPORT0);
		/* write to clear B2R */
		b2r_stat = readb(info->reg + WMT_NFC_HOST_STAT_CHANGE);
		writeb(B2R|b2r_stat, info->reg + WMT_NFC_HOST_STAT_CHANGE);

		writeb(NAND2NFC|(1<<1)|NFC_TRIGGER, info->reg + WMT_NFC_COMCTRL);

		status = wmt_wait_cmd_ready(mtd);
		/* status = wmt_nand_ready(mtd);*/
		if (status) {
			printk(KERN_ERR "Ramdom output err: nfc io transfer is not finished\n");
			/* return;*/
		}
		/* reduntant aera check ecc, wait nfc idle */
		status = wmt_nfc_wait_idle(mtd, 0, -1, -1, -1);
		/* status = wmt_nand_wait_idle(mtd);*/
		if (status)
			printk(KERN_ERR "Ramdom output err: nfc is not idle\n");
		return;


	case NAND_CMD_STATUS_ERROR:
	case NAND_CMD_STATUS_ERROR0:
		udelay(chip->chip_delay);
		return;


	default:
		/*
		 * If we don't have access to the busy pin, we apply the given
		 * command delay
		 */

		/* trigger command and addrress cycle */

		if (!chip->dev_ready) {
			udelay(chip->chip_delay);
			return;
		}
	}
	/* Apply this short delay always to ensure that we do wait tWB in */
	/* any case on any machine.*/
	/* ndelay(100);*/
	wmt_device_ready(mtd);
	if (((*(volatile unsigned long *)(GPIO_BASE_ADDR + 0x100))&6) == 2)
		spin_unlock(nand_lock);
}


static void wmt_nand_select_chip(struct mtd_info *mtd, int chipnr)
{
	struct wmt_nand_info *info = wmt_nand_mtd_toinfo(mtd);
	unsigned int b2r_stat;
	#ifdef NAND_DEBUG
	printk(KERN_NOTICE "\r enter in wmt_nand_select_chip()\n");
	#endif
	if (!((*(volatile unsigned long *)PMCEU_ADDR)&0x0010000))
		*(volatile unsigned long *)PMCEU_ADDR |= (0x0010000);
	if (chipnr > 1)
		printk(KERN_WARNING "There are only support two chip sets\n");

	b2r_stat = readb(info->reg + WMT_NFC_HOST_STAT_CHANGE);
	writeb(B2R|b2r_stat, info->reg + WMT_NFC_HOST_STAT_CHANGE);

	if (chip_swap == 0)
	/* select CE0 */
		writeb(~(1<<chipnr), info->reg + WMT_NFC_CHIP_ENABLE_CTRL);
	/* select CE1 */
	else
		writeb(~(2<<chipnr), info->reg + WMT_NFC_CHIP_ENABLE_CTRL);
}

void nand_hamming_ecc_1bit_correct(struct mtd_info *mtd)
{
	struct wmt_nand_info *info = wmt_nand_mtd_toinfo(mtd);
	unsigned int ecc_err_pos, bank_stat, redunt_stat;

	/* use HAMMING ECC but page not 512 and read data area */
	#ifdef NAND_DEBUG
	printk(KERN_NOTICE "in wmt_nand_cmdfunc(): Read data \n");
	#endif

	bank_stat = readl(info->reg + WMT_NFC_BANK18_ECC_STAT);
	redunt_stat = readb(info->reg + WMT_NFC_REDUNT_ECC_STAT);

	if (bank_stat & 0x5555) {
		printk(KERN_ERR "There are uncorrected ecc error in data area--\n");
		mtd->ecc_stats.failed++;
		return;
	} else if (redunt_stat & 0x05) {
		printk(KERN_ERR "There are uncorrected ecc error in reduntant area--\n");
		mtd->ecc_stats.failed++;
		return;
	} else if (((mtd->writesize == 2048) && (bank_stat & 0x2222)) ||
	((mtd->writesize == 4096) && (bank_stat & 0x22222222)) ||
	((mtd->writesize == 512) && (bank_stat & 0x22))) {
		/*#ifdef NAND_DEBUG*/
		/*printk(KERN_NOTICE
		"There are 1 bit ecc error in data area----at column = %d,	page_addr = %d\n",
		mycolumn, mypage_addr);*/
		/*#endif*/

		/*bank_sel = readb(info->reg + WMT_NFC_MISC_CTRL);*/
		writeb(readb(info->reg + WMT_NFC_MISC_CTRL) & 0xfc, info->reg + WMT_NFC_MISC_CTRL);

		if (bank_stat & 0x02) {
			ecc_err_pos = readw(info->reg + WMT_NFC_ODD_BANK_PARITY_STAT);
			printk(KERN_NOTICE "bank 1 error BYTE: %x bit:%x\n",
			ecc_err_pos & 0x1ff, (ecc_err_pos >> 9) & 0x07);
			printk(KERN_NOTICE "error value :%x\n",
			*(uint8_t *)&info->dmabuf[ecc_err_pos & 0x1ff]);
			bit_correct(&info->dmabuf[ecc_err_pos & 0x1ff], (ecc_err_pos >> 9) & 0x07);
			printk(KERN_NOTICE "correct value :%x\n", *(uint8_t *)&info->dmabuf[ecc_err_pos & 0x1ff]);
			/* mtd->ecc_stats.corrected++;*/
		} else if (bank_stat & 0x20) {
			ecc_err_pos = readw(info->reg + WMT_NFC_EVEN_BANK_PARITY_STAT);
			printk(KERN_NOTICE "bank 2 error BYTE: %x bit:%x\n",
			(ecc_err_pos & 0x1ff)+512, (ecc_err_pos >> 9) & 0x07);
			printk(KERN_NOTICE "error value :%x\n",
			*(uint8_t *)&info->dmabuf[512 + (ecc_err_pos & 0x1ff)]);
			bit_correct(&info->dmabuf[512 + (ecc_err_pos & 0x1ff)], (ecc_err_pos >> 9) & 0x07);
			printk(KERN_NOTICE "correct value :%x\n",
			*(uint8_t *)&info->dmabuf[512 + (ecc_err_pos & 0x1ff)]);
			/* mtd->ecc_stats.corrected++;*/
		}
		if (mtd->writesize == 2048) {
			writeb(readb(info->reg + WMT_NFC_MISC_CTRL) | 0x1, info->reg + WMT_NFC_MISC_CTRL);
	
			if (bank_stat & 0x0200) {
				ecc_err_pos = readw(info->reg + WMT_NFC_ODD_BANK_PARITY_STAT);
				printk(KERN_NOTICE "bank 3 error BYTE: %x bit:%x\n",
				(ecc_err_pos & 0x1ff)+1024, (ecc_err_pos >> 9) & 0x07);
				printk(KERN_NOTICE "error value :%x\n",
				*(uint8_t *)&info->dmabuf[1024 + (ecc_err_pos & 0x1ff)]);
				bit_correct(&info->dmabuf[1024 + (ecc_err_pos & 0x1ff)], (ecc_err_pos >> 9) & 0x07);
				printk(KERN_NOTICE "correct value :%x\n",
				*(uint8_t *)&info->dmabuf[1024 + (ecc_err_pos & 0x1ff)]);
				/* mtd->ecc_stats.corrected++;*/
			} else if (bank_stat & 0x2000) {
				ecc_err_pos = readw(info->reg + WMT_NFC_EVEN_BANK_PARITY_STAT);
				printk(KERN_NOTICE "bank 4 error BYTE: %x bit:%x\n",
				(ecc_err_pos & 0x1ff)+1536, (ecc_err_pos >> 9) & 0x07);
				printk(KERN_NOTICE "error value :%x\n",
				*(uint8_t *)&info->dmabuf[1536 + (ecc_err_pos & 0x1ff)]);
				bit_correct(&info->dmabuf[1536 + (ecc_err_pos & 0x1ff)], (ecc_err_pos >> 9) & 0x07);
				printk(KERN_NOTICE "correct value :%x\n",
				*(uint8_t *)&info->dmabuf[1536 + (ecc_err_pos & 0x1ff)]);
				/* mtd->ecc_stats.corrected++;*/
			}
		} else if (mtd->writesize == 4096) {
			writeb(readb(info->reg + WMT_NFC_MISC_CTRL) | 0x1 , info->reg + WMT_NFC_MISC_CTRL);
	
			if (bank_stat & 0x0200) {
				ecc_err_pos = readw(info->reg + WMT_NFC_ODD_BANK_PARITY_STAT);
				printk(KERN_NOTICE "bank 3 error BYTE: %x bit:%x\n",
				(ecc_err_pos & 0x1ff)+1024, (ecc_err_pos >> 9) & 0x07);
				printk(KERN_NOTICE "error value :%x\n",
				*(uint8_t *)&info->dmabuf[1024 + (ecc_err_pos & 0x1ff)]);
				bit_correct(&info->dmabuf[1024 + (ecc_err_pos & 0x1ff)], (ecc_err_pos >> 9) & 0x07);
				printk(KERN_NOTICE "correct value :%x\n",
				*(uint8_t *)&info->dmabuf[1024 + (ecc_err_pos & 0x1ff)]);
				/*   mtd->ecc_stats.corrected++;*/
			} else if (bank_stat & 0x2000) {
				ecc_err_pos = readw(info->reg + WMT_NFC_EVEN_BANK_PARITY_STAT);
				printk(KERN_NOTICE "bank 4 error BYTE: %x bit:%x\n",
				(ecc_err_pos & 0x1ff)+1536, (ecc_err_pos >> 9) & 0x7);
				printk(KERN_NOTICE "error value :%x\n",
				*(uint8_t *)&info->dmabuf[1536 + (ecc_err_pos & 0x1ff)]);
				bit_correct(&info->dmabuf[1536 + (ecc_err_pos & 0x1ff)], (ecc_err_pos >> 9) & 0x07);
				printk(KERN_NOTICE "correct value :%x\n",
				*(uint8_t *)&info->dmabuf[1536 + (ecc_err_pos & 0x1ff)]);
			} else if (bank_stat & 0x020000) {
				writeb((readb(info->reg + WMT_NFC_MISC_CTRL) & 0xFC) | 0x2,
				info->reg + WMT_NFC_MISC_CTRL);
				ecc_err_pos = readw(info->reg + WMT_NFC_ODD_BANK_PARITY_STAT);
				printk(KERN_NOTICE "bank 5 error BYTE: %x bit:%x\n",
				(ecc_err_pos & 0x1ff)+2048, (ecc_err_pos >> 9) & 0x07);
				printk(KERN_NOTICE "error value :%x\n",
				*(uint8_t *)&info->dmabuf[2048 + (ecc_err_pos & 0x1ff)]);
				bit_correct(&info->dmabuf[2048 + (ecc_err_pos & 0x1ff)], (ecc_err_pos >> 9) & 0x07);
				printk(KERN_NOTICE "correct value :%x\n",
				*(uint8_t *)&info->dmabuf[2048 + (ecc_err_pos & 0x1ff)]);
			} else if (bank_stat & 0x200000) {
				writeb((readb(info->reg + WMT_NFC_MISC_CTRL) & 0xFC) | 0x2,
				info->reg + WMT_NFC_MISC_CTRL);
				ecc_err_pos = readw(info->reg + WMT_NFC_EVEN_BANK_PARITY_STAT);
				printk(KERN_NOTICE "bank 6 error BYTE: %x bit:%x\n",
				(ecc_err_pos & 0x1ff)+2560, (ecc_err_pos >> 9) & 0x07);
				printk(KERN_NOTICE "error value :%x\n",
				*(uint8_t *)&info->dmabuf[2560 + (ecc_err_pos & 0x1ff)]);
				bit_correct(&info->dmabuf[2560 + (ecc_err_pos & 0x1ff)], (ecc_err_pos >> 9) & 0x07);
				printk(KERN_NOTICE "correct value :%x\n",
				*(uint8_t *)&info->dmabuf[2560 + (ecc_err_pos & 0x1ff)]);
			} else if (bank_stat & 0x02000000) {
				writeb(readb(info->reg + WMT_NFC_MISC_CTRL) | 0x3, info->reg + WMT_NFC_MISC_CTRL);
				ecc_err_pos = readw(info->reg + WMT_NFC_ODD_BANK_PARITY_STAT);
				printk(KERN_NOTICE "bank 7 error BYTE: %x bit:%x\n",
				(ecc_err_pos & 0x1ff)+3072, (ecc_err_pos >> 9) & 0x07);
				printk(KERN_NOTICE "error value :%x\n",
				*(uint8_t *)&info->dmabuf[3072 + (ecc_err_pos & 0x1ff)]);
				bit_correct(&info->dmabuf[3072 + (ecc_err_pos & 0x1ff)], (ecc_err_pos >> 9) & 0x07);
				printk(KERN_NOTICE "correct value :%x\n",
				*(uint8_t *)&info->dmabuf[3072 + (ecc_err_pos & 0x1ff)]);
			} else if (bank_stat & 0x20000000) {
				writeb(readb(info->reg + WMT_NFC_MISC_CTRL) | 0x3, info->reg + WMT_NFC_MISC_CTRL);
				ecc_err_pos = readw(info->reg + WMT_NFC_EVEN_BANK_PARITY_STAT);
				printk(KERN_NOTICE "bank 8 error BYTE: %x bit:%x\n",
				(ecc_err_pos & 0x1ff)+3584, (ecc_err_pos >> 9) & 0x07);
				printk(KERN_NOTICE "error value :%x\n",
				*(uint8_t *)&info->dmabuf[3584 + (ecc_err_pos & 0x1ff)]);
				bit_correct(&info->dmabuf[3584 + (ecc_err_pos & 0x1ff)], (ecc_err_pos >> 9) & 0x07);
				printk(KERN_NOTICE "correct value :%x\n",
				*(uint8_t *)&info->dmabuf[3584 + (ecc_err_pos & 0x1ff)]);
			}
		}
	} else if (redunt_stat & 0x02) {
		/* printk(KERN_WARNING "There are 1 bit ecc error in reduntant data area--\n");*/
		/* memcpy(chip->oob_poi, info->reg+ECC_FIFO_0, 64);*/
		ecc_err_pos = readw(info->reg + WMT_NFC_REDUNT_AREA_PARITY_STAT);
		printk(KERN_NOTICE "oob area error BYTE: %x bit:%x\n",
		ecc_err_pos & 0x3f, (ecc_err_pos >> 8) & 0x07);
		printk(KERN_NOTICE "error value :%x\n",
		*((unsigned char *)info->reg+ECC_FIFO_0 + (ecc_err_pos & 0x3f)));
		bit_correct((unsigned char *)info->reg+ECC_FIFO_0 + (ecc_err_pos & 0x3f),
		(ecc_err_pos >> 8) & 0x07);
		printk(KERN_NOTICE "error value :%x\n",
		*((unsigned char *)info->reg+ECC_FIFO_0 + (ecc_err_pos & 0x3f)));
		/* mtd->ecc_stats.corrected++;*/
	}
}
static void wmt_nand_write_buf(struct mtd_info *mtd, const uint8_t *buf, int len)
{
	struct wmt_nand_info *info = wmt_nand_mtd_toinfo(mtd);
	#ifdef NAND_DEBUG
	printk(KERN_NOTICE "enter in wmt_nand_write_buf()\n");
	#endif
	memcpy(info->dmabuf + info->datalen, buf, len);

	info->datalen += len;
}

static void wmt_nand_read_buf(struct mtd_info *mtd, uint8_t *buf, int len)
{
	struct wmt_nand_info *info = wmt_nand_mtd_toinfo(mtd);
	#ifdef NAND_DEBUG
	printk(KERN_NOTICE "enter in wmt_nand_read_buf() len: %x infoDatalen :%x\n", len, info->datalen);
	#endif

	memcpy(buf, info->dmabuf + info->datalen, len);
	info->datalen += len;
}

static uint8_t wmt_read_byte(struct mtd_info *mtd)
{
	/* struct wmt_nand_mtd *nmtd = mtd->priv;*/
	/* struct wmt_nand_info *info = wmt_nand_mtd_toinfo(mtd);*/
	uint8_t d;
	#ifdef NAND_DEBUG
	printk(KERN_NOTICE "enter in wmt_nand_read_byte()\n");
	#endif

	/* d = readb(info->reg + WMT_NFC_DATAPORT) & 0xff;*/
	 wmt_nand_read_buf(mtd, &d, 1);
	/* via_dev_dbg(&nmtd->info->platform->dev, "Read %02x\n", d);*/
	/* via_dev_dbg(info->platform->dev, "Read %02x\n", d);*/

	return d;
}

static int wmt_nand_read_oob(struct mtd_info *mtd, struct nand_chip *chip, int page, int sndcmd)
{
	struct wmt_nand_info *info = wmt_nand_mtd_toinfo(mtd);
	uint8_t *buf = chip->oob_poi;
	/* int length = mtd->oobsize;  */ /* prepad = chip->ecc.prepad, bytes = chip->ecc.bytes;*/
	/* int chunk = chip->ecc.bytes + chip->ecc.prepad + chip->ecc.postpad;*/
	/* int eccsize = chip->ecc.size;*/
	uint8_t *bufpoi = buf;
	/* struct nand_oobfree *free = chip->ecc.layout->oobfree;*/
	/* uint32_t boffs;*/
	/* int pos;   */ /* toread, sndrnd = 1;*/

	#ifndef NAND_HARMING_ECC
	/*int i;*/
	int pos;   /* toread, sndrnd = 1;*/
	int eccsize = chip->ecc.size;
	/*int chunk = chip->ecc.bytes + chip->ecc.prepad + chip->ecc.postpad;*/
	#endif

	#ifdef NAND_DEBUG
	printk(KERN_NOTICE "\r enter in wmt_nand_read_oob()\n");
	#endif
	/* info->datalen = mtd->writesize;*/  /* oob data is placed in after info->dmabuf[2047]  */

	#ifdef NAND_HARMING_ECC
	/*info->datalen = 0;*/
	/* pos = mtd->writesize;*/
	/* chip->cmdfunc(mtd, NAND_CMD_READOOB, pos, page);*/
	chip->cmdfunc(mtd, NAND_CMD_READOOB, 0, page);
	/* #if (NAND_PAGE_SIZE == 512)*/
	/*       ;*/
	/* writeb(readb(info->reg + WMT_NFC_SMC_ENABLE) | 0x02, info->reg + WMT_NFC_SMC_ENABLE);*/
	/* memcpy(chip->oob_poi, info->reg+ECC_FIFO_0, mtd->oobsize);*/
	/* #else*/
	chip->read_buf(mtd, bufpoi, 64);
	/* #endif*/
	//------------------------------------------
	#if 0
	/* for debug  */
	int i;
	printk(KERN_NOTICE "data aera is -------------------------\n");
	for (i = 0; i < 64; i += 4) {
		printk(KERN_NOTICE "%x %x %x %x\n",
		info->dmabuf[i], info->dmabuf[i+1], info->dmabuf[i+2], info->dmabuf[i+3]);
	 }
	printk(KERN_NOTICE "spare aera is -------------------------\n");
	for (i = 0; i < mtd->oobsize; i += 4)
		printk(KERN_NOTICE "%x %x %x %x\n",
		chip->oob_poi[i], chip->oob_poi[i+1], chip->oob_poi[i+2], chip->oob_poi[i+3]);
	#endif
	//while(i);
	//---------------------------------------------

	#else
	/*  for (i = 0; i < chip->ecc.steps; i++) {*/
	/*for (i = 0; i < 4; i++) {*/
	writeb(readb(info->reg + WMT_NFC_SMC_ENABLE) | 0x2,
	info->reg + WMT_NFC_SMC_ENABLE);
	if (mtd->writesize < 8192)
		pos = (eccsize + chip->ecc.bytes) * chip->ecc.steps;/*+ i * (eccsize + chunk);*/
	else
		pos = 8192 + 218 + 0xA0;//or 4096 + 0xA0
	chip->cmdfunc(mtd, NAND_CMD_READOOB, pos, page);
	/*chip->read_buf(mtd, bufpoi, 32);*/
	memcpy(bufpoi, info->reg+ECC_FIFO_0, 64);
	writeb(readb(info->reg + WMT_NFC_SMC_ENABLE) & 0xfd,
	info->reg + WMT_NFC_SMC_ENABLE);
		/*chip->read_buf(mtd, bufpoi + i * 16, 16);*/
	/*}*/
	#endif

	return 1;
}


/*
 * wmt_nand_read_bb_oob - OOB data read function
 * @mtd:	mtd info structure
 * @chip:	nand chip info structure
 * @page:	page number to read
 * @sndcmd:	flag whether to issue read command or not
 */
static int wmt_nand_read_bb_oob(struct mtd_info *mtd, struct nand_chip *chip,
int page, int sndcmd)
{
	unsigned int tmp, bch;
	struct wmt_nand_info *info = wmt_nand_mtd_toinfo(mtd);
	#ifdef NAND_DEBUG
	printk(KERN_NOTICE "\r enter in wmt_nand_read_bb_oob()\n");
	#endif
	tmp = readl(info->reg + WMT_NFC_NAND_TYPE_SEL);
	bch = readl(info->reg + WMT_NFC_ECC_BCH_CTRL);
	writeb((tmp & 0xfffffffc)+3,	info->reg + WMT_NFC_NAND_TYPE_SEL);
	writel(readl(info->reg + WMT_NFC_ECC_BCH_CTRL) & 0xfffffff8, info->reg + WMT_NFC_ECC_BCH_CTRL);
	writel(readl(info->reg + WMT_NFC_ECC_BCH_CTRL) | DIS_BCH_ECC, info->reg + WMT_NFC_ECC_BCH_CTRL);
	writew(eccBCH_inetrrupt_disable, info->reg + WMT_NFC_ECC_BCH_INT_MASK);
	writel(readl(info->reg + WMT_NFC_ECC_BCH_CTRL) | READ_RESUME,
	info->reg + WMT_NFC_ECC_BCH_CTRL);
	//ecc_type = 0;
	//nfc_ecc_set(info, 0);   /* off hardware ecc  */
	//set_ecc_engine(info, 0);  /* harming ECC structure for bad block check*/
	/* chip->ecc.layout = &wmt_hm_oobinfo_2048;*/

	if (sndcmd) {
		chip->cmdfunc(mtd, NAND_CMD_READ0, 0, page);
		sndcmd = 0;
	}
	info->datalen = mtd->writesize;
	chip->read_buf(mtd, chip->oob_poi, 64);
	//memcpy(chip->oob_poi, info->reg+ECC_FIFO_0, 64);
	//nfc_ecc_set(info, 1);   /* on hardware ecc  */
	writel(readl(info->reg + WMT_NFC_ECC_BCH_CTRL) & 0xfffffff7, info->reg + WMT_NFC_ECC_BCH_CTRL);
	writel(readl(info->reg + WMT_NFC_ECC_BCH_CTRL) | bch, info->reg + WMT_NFC_ECC_BCH_CTRL);
	writeb(tmp,	info->reg + WMT_NFC_NAND_TYPE_SEL);
	writew(eccBCH_inetrrupt_disable, info->reg + WMT_NFC_ECC_BCH_INT_MASK);
	writel(readl(info->reg + WMT_NFC_ECC_BCH_CTRL) | READ_RESUME,
	info->reg + WMT_NFC_ECC_BCH_CTRL);
	#ifndef NAND_HARMING_ECC
	//ecc_type = 1;
	if (ECC8BIT_ENGINE == 1)
		set_ecc_engine(info, 3);  /* BCH ECC structure 12bit ecc engine*/
	else
		set_ecc_engine(info, 1);  /* BCH ECC structure 4bit ecc engine*/

	#else
		if (mtd->writesize != 512)
			set_ecc_engine(info, 0);  /* harming ECC structure for bad block check*/

	#endif
	return sndcmd;
}



static int wmt_nand_write_oob(struct mtd_info *mtd, struct nand_chip *chip, int page)
{
	struct wmt_nand_info *info = wmt_nand_mtd_toinfo(mtd);

	#ifndef NAND_HARMING_ECC
	/*int i;*/
	unsigned int b2r_stat;
	/*int chunk = chip->ecc.bytes + chip->ecc.prepad + chip->ecc.postpad;*/
	int eccsize = chip->ecc.size; /* length = mtd->oobsize;  */
	/* prepad = chip->ecc.prepad, bytes = chip->ecc.bytes;*/
	#endif

	int pos, status = 0;
	/*int steps = chip->ecc.steps;*/  /* Vincent 2008.11.4*/
	const uint8_t *bufpoi = chip->oob_poi;
	/* struct nand_oobfree *free = chip->ecc.layout->oobfree;*/
	/* uint32_t boffs;*/
	#ifdef NAND_DEBUG
	printk(KERN_NOTICE "\r enter in wmt_nand_write_oob()\n");
	#endif
	/*
	 * data-ecc-data-ecc ... ecc-oob
	 * or
	 * 512  7     1     5    0    3
	 * data-ecc-prepad-data-pad-oobecc ....
	 */

	/* info->datalen = mtd->writesize; */ /* oob data is placed in after info->dmabuf[2047]  */
#ifdef NAND_HARMING_ECC
	info->datalen = 0;
	pos = mtd->writesize;
	chip->write_buf(mtd, bufpoi, mtd->oobsize);
	wmt_nfc_dma_cfg(mtd, mtd->oobsize, 1, 2, -1);  /* 64 or 16 bytes   */
	chip->cmdfunc(mtd, NAND_CMD_SEQIN, pos, page);
	chip->cmdfunc(mtd, NAND_CMD_PAGEPROG, -1, -1);
	status = chip->waitfunc(mtd, chip);

	if (status & NAND_STATUS_FAIL)
		return -EIO;

	return 0;
#else
	/* 	for (i = 0; i < steps; i++) {*/
	/*for (i = 0; i < 4; i++) {*/
	b2r_stat = readb(info->reg + WMT_NFC_HOST_STAT_CHANGE);
	writeb(B2R|b2r_stat, info->reg + WMT_NFC_HOST_STAT_CHANGE);

	info->datalen = 0;
	/*chip->write_buf(mtd, bufpoi, 32);*/
	memcpy(info->reg+ECC_FIFO_0, bufpoi, 32);
	pos = eccsize * chip->ecc.steps + 8*4;
	/*pos = eccsize + i * (eccsize + chunk);*/
	/*wmt_nfc_dma_cfg(mtd, 32, 1, 1, i);*/
	chip->cmdfunc(mtd, NAND_CMD_SEQIN, pos, page);

	chip->cmdfunc(mtd, NAND_CMD_PAGEPROG, -1, -1);
	/* printk(KERN_NOTICE "\r in wmt_nand_write_oob_new(): waitfunc_1\n");*/
	status = chip->waitfunc(mtd, chip);
	/* printk(KERN_NOTICE "\r in wmt_nand_write_oob_new(): waitfunc_2\n");*/
	if (status & NAND_STATUS_FAIL)
		return -EIO;
	/* } */
	return 0;
#endif
}


/**
 * wmt_nand_read_page - hardware ecc syndrom based page read
 * @mtd:	mtd info structure
 * @chip:	nand chip info structure
 * @buf:	buffer to store read data
 *
 * The hw generator calculates the error syndrome automatically. Therefor
 * we need a special oob layout and handling.
 */
static int wmt_nand_read_page(struct mtd_info *mtd, struct nand_chip *chip,
						 uint8_t *buf, int page)
{
	struct wmt_nand_info *info = wmt_nand_mtd_toinfo(mtd);

	#ifdef NAND_DEBUG
		printk(KERN_NOTICE "\r enter in wmt_nand_read_page()\n");
	#endif
	info->datalen = 0;
	chip->read_buf(mtd, buf, mtd->writesize);
	#ifdef NAND_DEBUG
	printk(KERN_NOTICE "\r enter in nand_read_page(): mtd->writesize is %d and oobsize is %d\n",
	mtd->writesize, mtd->oobsize);
	#endif
	/* memcpy(chip->oob_poi, info->reg+ECC_FIFO_0, mtd->oobsize);*/
	/* memset(chip->oob_poi, 0xff, mtd->oobsize);*/
	if (mtd->writesize == 2048) {
		/* writeb(readb(info->reg + WMT_NFC_CALC_CTRL) & 0xF7,
		info->reg + WMT_NFC_CALC_CTRL);*/
		memcpy(chip->oob_poi, info->reg+ECC_FIFO_0, mtd->oobsize);
	} else if (mtd->writesize == 4096 || mtd->writesize == 8192) {
		/* dannier test 0x34 are used or not when not hamming mode*/
		writeb(readb(info->reg + WMT_NFC_CALC_CTRL) & 0xF7,
		info->reg + WMT_NFC_CALC_CTRL);
		memcpy(chip->oob_poi, info->reg+ECC_FIFO_0, 64);
		#ifdef NAND_HARMING_ECC
		writeb(readb(info->reg + WMT_NFC_CALC_CTRL) | 0x08,
		info->reg + WMT_NFC_CALC_CTRL);
		memcpy(chip->oob_poi+64, info->reg+ECC_FIFO_0, 64);
		#endif
	} else {  /* pagesize = 512 */
		/* only reduntant area read enable */
		memcpy(chip->oob_poi, info->reg+ECC_FIFO_0, mtd->oobsize);
		#ifndef PAGE_READ_COUNTER
			writeb(readb(info->reg + WMT_NFC_SMC_ENABLE) & 0xfd,
			info->reg + WMT_NFC_SMC_ENABLE);
		#endif
		/* memcpy(chip->oob_poi, info->reg+ECC_FIFO_0, mtd->oobsize);*/
	}

	#if 0
	/* for debug  */
	int i;
	/* printk(KERN_NOTICE "data aera is -------------------------\n");*/
	/* for (i = 0; i < mtd->writesize; i += 4) {*/
	/* printk(KERN_NOTICE "%x %x %x %x\n",
	info->dmabuf[i], info->dmabuf[i+1], info->dmabuf[i+2], info->dmabuf[i+3]);*/
	/* }*/
	printk(KERN_NOTICE "spare aera is -------------------------\n");
	for (i = 0; i < mtd->oobsize; i += 4)
		printk(KERN_NOTICE "%x %x %x %x\n",
		chip->oob_poi[i], chip->oob_poi[i+1], chip->oob_poi[i+2], chip->oob_poi[i+3]);
	#endif

 /*   memcpy(chip->oob_poi, info->reg+ECC_FIFO_0, mtd->oobavail);*/
 /*   chip->read_buf(mtd, chip->oob_poi, mtd->oobavail); */  /* ????  */
	return 0;
}

/**
 *  wmt_nand_write_page_lowlevel - hardware ecc syndrom based page write
 *  @mtd:    mtd info structure
 *  @chip:  nand chip info structure
 *  @buf:  data buffer
 *
 *  The hw generator calculates the error syndrome automatically. Therefor
 *  we need a special oob layout and handling.
 *
 */
static void wmt_nand_write_page_lowlevel(struct mtd_info *mtd, struct nand_chip *chip, const uint8_t *buf)
{
	struct wmt_nand_info *info = wmt_nand_mtd_toinfo(mtd);
	#ifdef NAND_DEBUG
	printk(KERN_NOTICE "enter in wmt_nand_page_write_lowlevel() writesize %x\n", mtd->writesize);
	#endif
	info->dma_finish = 0;
	writeb(0x07, info->reg + WMT_NFC_REDUNT_ECC_STAT_MASK);
	if (mtd->writesize < 8192) {
		info->datalen = 0;
		chip->write_buf(mtd, buf, mtd->writesize);
		wmt_nfc_dma_cfg(mtd, mtd->writesize, 1, 0, -1);  /*  2048bytes  */
	} else if (mtd->writesize >= 8192 && info->datalen == first4k218) {
		/*printk(KERN_NOTICE "enter1 in wmt_nand_page_write_lowlevel() writesize 0x%x,info->datalen = 0x%x\n",
		mtd->writesize, info->datalen);*/
		chip->write_buf(mtd, buf, mtd->writesize);
		wmt_nfc_dma_cfg(mtd, mtd->writesize/2, 1, 0, -1);  /*  2048bytes  */
	} else if (info->datalen == mtd->writesize) {
		wmt_nfc_dma_cfg(mtd, mtd->writesize/2, 1, 0, 2);  /*  2048bytes  */
		/*printk(KERN_NOTICE "enter2 in wmt_nand_page_write_lowlevel() writesize 0x%x,info->datalen = 0x%x\n",
		mtd->writesize, info->datalen);*/
	}

	/* for debug  */
	#if 0
	int i;
	/* printk(KERN_NOTICE "data aera is -------------------------\n");*/
	/* for (i = 0; i < mtd->writesize; i += 4) {*/
	/* printk(KERN_NOTICE "%x %x %x %x\n",
	info->dmabuf[i], info->dmabuf[i+1], info->dmabuf[i+2], info->dmabuf[i+3]);*/
	/* info->dmabuf[i] = 0;*/
	/* info->dmabuf[i+1] = 0;*/
	/* info->dmabuf[i+2] = 0;*/
	/* info->dmabuf[i+3] = 0;*/
	/*  }*/
	printk(KERN_NOTICE "spare aera is -------------------------\n");
	for (i = 0; i < mtd->oobsize; i += 4)
		printk(KERN_NOTICE "%x %x %x %x\n",
		chip->oob_poi[i], chip->oob_poi[i+1], chip->oob_poi[i+2], chip->oob_poi[i+3]);
	#endif

	if (mtd->writesize == 2048) {
		/* writeb(readb(info->reg + WMT_NFC_CALC_CTRL) & 0xF7, info->reg + WMT_NFC_CALC_CTRL);*/
		memcpy(info->reg+ECC_FIFO_0, chip->oob_poi, mtd->oobsize);
		/* solve a hardware bug --- bank 3, byte 7, bit 7 error  */
		/*     writel(0xfeffffff, info->reg + ECC_FIFO_e);*/
	} else if (mtd->writesize == 4096 || mtd->writesize == 8192) {
		writeb(readb(info->reg + WMT_NFC_CALC_CTRL) & 0xF7,
		info->reg + WMT_NFC_CALC_CTRL);
		memcpy(info->reg+ECC_FIFO_0, chip->oob_poi, 64);
		#ifdef NAND_HARMING_ECC
		writeb(readb(info->reg + WMT_NFC_CALC_CTRL) | 0x08,
		info->reg + WMT_NFC_CALC_CTRL);
		memcpy(info->reg+ECC_FIFO_0, chip->oob_poi+64, 64);
		#endif
		/* solve a hardware bug --- bank 7, byte 7, bit 7 error  */
		/* writel(0xfeffffff, info->reg + ECC_FIFO_e);*/
	} else {
		memcpy(info->reg+ECC_FIFO_0, chip->oob_poi, mtd->oobsize);
	}
	/* memcpy(info->reg+ECC_FIFO_0, chip->oob_poi, mtd->oobavail);*/
	/* chip->write_buf(mtd, chip->oob_poi, mtd->oobsize);*/
}


static int wmt_nand_write_page(struct mtd_info *mtd, struct nand_chip *chip,
const uint8_t *buf, int page, int cached, int raw)
{
	int status;
	//#ifdef NAND_BBT_BCH_ECC
	struct wmt_nand_info *info = wmt_nand_mtd_toinfo(mtd);
	//#endif
	#ifdef NAND_DEBUG
	printk(KERN_NOTICE "enter in wmt_nand_write_page()\n");
	printk(KERN_NOTICE "raw = %d, and ecc_type = %d\n", raw, ecc_type);
	#endif


	#ifdef NAND_BBT_BCH_ECC
	if (raw == 1 && ecc_type == 1) {  /* nand old structure  */
		/*#ifdef NAND_DEBUG*/
		printk(KERN_NOTICE "old structure: enter in wmt_nand_write_page()\n");
		/*#endif*/
		ecc_type = 0;
		set_ecc_engine(info, 0);  /* old structure for file system write*/
		if (mtd->writesize == 2048)
			chip->ecc.layout = &wmt_hm_oobinfo_2048;
		else
			chip->ecc.layout = &wmt_hm_oobinfo_4096;
	}	else if (raw == 0 && ecc_type == 0) {  /* nand new structure  */
		/*#ifdef NAND_DEBUG*/
		printk(KERN_NOTICE "new structure: enter in wmt_nand_write_page()\n");
		/*#endif*/
		ecc_type = 1;
		if (ECC8BIT_ENGINE == 1)
			set_ecc_engine(info, 3); /* new structure for bad block check*/
		else
			set_ecc_engine(info, 1); /* new structure for bad block check*/

		if (mtd->writesize == 2048)
			chip->ecc.layout = &wmt_oobinfo_2048;
		else {
			if (ECC8BIT_ENGINE == 1)
				chip->ecc.layout = &wmt_12bit_oobinfo_4096;
				//chip->ecc.layout = &wmt_8bit_oobinfo_4096;
			else
				chip->ecc.layout = &wmt_oobinfo_4096;
		}
	}
	#endif


	#if 0
	#ifndef NAND_HARMING_ECC
	if (raw == 1 && ecc_type == 0) {  /* nand new structure  */
		#ifdef NAND_DEBUG
		printk(KERN_NOTICE "New structure: enter in wmt_nand_write_page()\n");
		#endif
		ecc_type = 1;
		if (ECC8BIT_ENGINE == 1)
		set_ecc_engine(info, 3);
		else
		set_ecc_engine(info, 1);  /* new structure */

		chip->ecc.layout = &wmt_oobinfo_2048;
	}	else if (raw == 0 && ecc_type == 1) {  /* nand old structure  */
		#ifdef NAND_DEBUG
		printk(KERN_NOTICE "Old structure: enter in wmt_nand_write_page()\n");
		#endif
		ecc_type = 0;
		set_ecc_engine(info, 0);  /* old structure for bad block check*/
		chip->ecc.layout = &wmt_hm_oobinfo_2048;
	}
	#endif
	#endif

	/*printk(KERN_NOTICE "wmt_nand_write_page() write begin\n");*/
	info->datalen = 0;
	chip->ecc.write_page(mtd, chip, buf);
		/*   }*/
	chip->cmdfunc(mtd, NAND_CMD_SEQIN, 0x00, page);
	status = nand_pdma_handler(mtd);
	nand_free_pdma(mtd);
	if (status)
		printk(KERN_ERR "check write pdma handler status= %x \n", status);

	if (mtd->writesize >= 8192) {
		chip->ecc.write_page(mtd, chip, buf);
		chip->cmdfunc(mtd, NAND_CMD_RNDIN, second4k218, page);
		status = nand_pdma_handler(mtd);
	nand_free_pdma(mtd);
	}
	/*
	 * *   * Cached progamming disabled for now, Not sure if its worth the
	 * *       * trouble. The speed gain is not very impressive. (2.3->2.6Mib/s)
	 * *           */
	cached = 0;

	if (!cached || !(chip->options & NAND_CACHEPRG)) {

		chip->cmdfunc(mtd, NAND_CMD_PAGEPROG, -1, -1);
		status = chip->waitfunc(mtd, chip);
		writeb(0x80, info->reg + WMT_NFC_REDUNT_ECC_STAT_MASK);
		/*
		* * See if operation failed and additional status checks are
		* * available
		* *      */
		if ((status & NAND_STATUS_FAIL) && (chip->errstat))
			status = chip->errstat(mtd, chip, FL_WRITING, status,	page);

		if (status & NAND_STATUS_FAIL)
			return -EIO;
	} else {
		chip->cmdfunc(mtd, NAND_CMD_CACHEDPROG, -1, -1);
		status = chip->waitfunc(mtd, chip);
	}


	#ifdef CONFIG_MTD_NAND_VERIFY_WRITE
	/* Send command to read back the data */
	chip->cmdfunc(mtd, NAND_CMD_READ0, 0, page);

	if (chip->verify_buf(mtd, buf, mtd->writesize))
		return -EIO;
	#endif
	return 0;
}


/* wmt_nand_init_chip
 *
 * init a single instance of an chip
 */

static void wmt_nand_init_chip(struct wmt_nand_info *info, struct wmt_nand_mtd *nmtd)
{
	struct nand_chip *chip = &nmtd->chip;
	struct mtd_info *mtd = &nmtd->mtd;

	#if 0
	chip->cmdfunc      = wmt_nand_cmdfunc;
	chip->dev_ready    = wmt_device_ready;
	chip->read_byte    = wmt_read_byte;
	chip->write_buf    = wmt_nand_write_buf;
	chip->read_buf     = wmt_nand_read_buf;
	chip->select_chip  = wmt_nand_select_chip;
	chip->chip_delay   = 20;
	chip->priv	   = nmtd;
	chip->options	   = 0;
	chip->controller   = &info->controller;
	#endif

	/* chip->cmd_ctrl  = wmt_nand_hwcontrol;*/
	#if 0
	switch (info->cpu_type) {
	case TYPE_wmt:
		break;

	case TYPE_vt8620:
		break;

	case TYPE_vt8610:
		break;
	}
	#endif

	/* nmtd->set	   = set;*/
	if (hardware_ecc) {
		/*	chip->ecc.calculate = wmt_nand_calculate_ecc;*/
		/*	chip->ecc.correct   = wmt_nand_correct_data;*/

		if (mtd->writesize == 2048) {
			chip->ecc.size      = 512;
			chip->ecc.bytes     = 8;
			chip->ecc.steps     = 4;
			/*chip->ecc.layout    = &wmt_oobinfo_2048;*/
			chip->ecc.prepad    = 1;
			chip->ecc.postpad   = 8;
		} else if (mtd->writesize == 4096 || mtd->writesize == 8192) {
			chip->ecc.size      = 512;
			if (ECC8BIT_ENGINE == 1)
				chip->ecc.bytes     = 20;
			//chip->ecc.bytes     = 16;
			else
			chip->ecc.bytes     = 8;

			chip->ecc.steps     = 8;
			/*chip->ecc.layout    = &wmt_oobinfo_4096;*/
			chip->ecc.prepad    = 1;
			chip->ecc.postpad   = 8;

		} else {   /*  512 page   */
			chip->ecc.size      = 512;
			chip->ecc.bytes      = 3;
			chip->ecc.steps     = 1;
			/*chip->ecc.layout    = &wmt_oobinfo_512;*/
			chip->ecc.prepad    = 4;
			chip->ecc.postpad   = 9;
		}

		chip->write_page = wmt_nand_write_page;
		chip->ecc.write_page = wmt_nand_write_page_lowlevel;
		chip->ecc.write_oob = wmt_nand_write_oob;
		chip->ecc.read_page = wmt_nand_read_page;
		chip->ecc.read_oob = wmt_nand_read_oob;

		chip->ecc.read_bb_oob = wmt_nand_read_bb_oob;

		/*	switch (info->cpu_type) {*/
		/*	case TYPE_wmt:*/
		chip->ecc.hwctl	    = wmt_nand_enable_hwecc;
		/*	chip->ecc.calculate = wmt_nand_calculate_ecc;*/
		/*	break;*/
	#if 0
	case TYPE_vt8620:
		chip->ecc.hwctl     = vt8620_nand_enable_hwecc;
		chip->ecc.calculate = vt86203_nand_calculate_ecc;
		break;

	case TYPE_vt8610:
		chip->ecc.hwctl     = vt8610_nand_enable_hwecc;
		chip->ecc.calculate = vt8610_nand_calculate_ecc;
		break;
		}
	#endif
	} else
		chip->ecc.mode	    = NAND_ECC_SOFT;
}


static int wmt_nand_remove(struct platform_device *pdev)
{
	struct wmt_nand_info *info = dev_get_drvdata(&pdev->dev);

	/*  struct mtd_info *mtd = dev_get_drvdata(pdev);*/
	dev_set_drvdata(&pdev->dev, NULL);
	/*  platform_set_drvdata(pdev, NULL);*/
	/*  dev_set_drvdata(pdev, NULL);*/
	if (info == NULL)
		return 0;

	/* first thing we need to do is release all our mtds
	 * and their partitions, then go through freeing the
	 * resources used
	 */

	if (info->mtds != NULL) {
		struct wmt_nand_mtd *ptr = info->mtds;
	/* int mtdno;*/

	/* for (mtdno = 0; mtdno < info->mtd_count; mtdno++, ptr++) {*/
	/*     pr_debug("releasing mtd %d (%p)\n", mtdno, ptr);*/
		nand_release(&ptr->mtd);
	/*  }*/
		kfree(info->mtds);
	}

	/* free the common resources */

	if (info->reg != NULL) {
		//iounmap(info->reg);
		info->reg = NULL;
	}

	if (info->area != NULL) {
		release_resource(info->area);
		kfree(info->area);
		info->area = NULL;
	}
	kfree(info);
	return 0;
}

#ifdef CONFIG_MTD_CMDLINE_PARTS

extern int mtdpart_setup(char *);

static int __init add_dynamic_parts(struct mtd_info *mtd)
{
	static const char *part_parsers[] = { "cmdlinepart", NULL };
	struct mtd_partition *parts;
	/*const struct omap_flash_part_config *cfg;
	char *part_str = NULL;
	size_t part_str_len;*/
	int c;

	/*cfg = omap_get_var_config(OMAP_TAG_FLASH_PART, &part_str_len);
	if (cfg != NULL) {
		part_str = kmalloc(part_str_len + 1, GFP_KERNEL);
		if (part_str == NULL)
			return -ENOMEM;
		memcpy(part_str, cfg->part_table, part_str_len);
		part_str[part_str_len] = '\0';
		mtdpart_setup(part_str);
	}*/
	c = parse_mtd_partitions(mtd, part_parsers, &parts, 0);
	/*if (part_str != NULL) {
		mtdpart_setup(NULL);
		kfree(part_str);
	}*/
	if (c <= 0)
		return -1;

	add_mtd_partitions(mtd, parts, c);

	return 0;
}

#else

static inline int add_dynamic_parts(struct mtd_info *mtd)
{
	return -1;
}

#endif

int search_mtd_table(char *string, char *ret)
{
	int i, err = 0;
	for (i = 0; i < MAX_MTD_DEVICES; i++) {
		if (!&nand_partitions[i]) {
			err = 1;
			break;
		}
		printk(KERN_DEBUG "MTD dev%d size: %8.8llx \"%s\"\n",
		i, nand_partitions[i].size, nand_partitions[i].name);
		if (strcmp(string, nand_partitions[i].name) == 0) {
			*ret = i;
			break;
		}
	}
	return err;
}
	
/*Lch */
static int wmt_recovery_call(struct notifier_block *nb, unsigned long code, void *_cmd)
{
	struct mtd_info *mtd;
	int err = 0, ret = 0;
	char ret1 = 0;
	printk(KERN_EMERG "Lch enter wmt_recovery_call.\n");

	mtd = container_of(nb, struct mtd_info, reboot_notifier);

	if((code == SYS_RESTART) && _cmd) {
		char *cmd = _cmd;
		if  (!strcmp(cmd, "recovery")) {
			err = search_mtd_table("android-data", &ret1);
			ret = (int)ret1;
			if (!err) {
			//	printk(KERN_EMERG "Lch jump2 android-data wmt_recovery_call.ret =%d\n",ret);
				struct erase_info einfo;
				loff_t to;
				memset(&einfo, 0, sizeof(einfo));
				to = nand_partitions[ret].offset;
				einfo.mtd = mtd;
				einfo.addr = (unsigned long)to;
				einfo.len = nand_partitions[ret].size;

			//	printk("android-data einfo.addr is %8.8x\n",einfo.addr);
			//	printk("android-data einfo.len is %8.8x\n",einfo.len);
			//	printk("android-data nand_partitions[%d].offset is %8.8x\n",ret,nand_partitions[ret].offset);
			//	printk("android-data nand_partitions[%d].size is %8.8x\n",ret,nand_partitions[ret].size);
				ret = nand_erase_nand(mtd, &einfo, 0xFF);
				if (ret < 0)
					printk("enand_erase_nand result is %x\n",ret);
			}

			err = search_mtd_table("android-cache", &ret1);
			ret = (int)ret1;
			if (!err) {
			//	printk(KERN_EMERG "Lch jump3 wmt_recovery_call.android-cache ret=%d\n",ret);
				struct erase_info einfo;
				loff_t to;
				memset(&einfo, 0, sizeof(einfo));
				to = nand_partitions[ret].offset;
				einfo.mtd = mtd;
				einfo.addr = (unsigned long)to;
				einfo.len = nand_partitions[ret].size;

			//	printk("android-cache einfo.addr is %8.8x\n",einfo.addr);
			//	printk("android-cache einfo.len is %8.8x\n",einfo.len);
			//	printk("android-data nand_partitions[%d].offset is %8.8x\n",ret,nand_partitions[ret].offset);
			//	printk("android-data nand_partitions[%d].size is %8.8x\n",ret,nand_partitions[ret].size);
				ret = nand_erase_nand(mtd, &einfo, 0xFF);
				if (ret < 0)
					printk("enand_erase_nand result is %x\n",ret);
			}
		}
	}

	return NOTIFY_DONE;
}

/**********************************************************************
Name  	 : nfc_pdma_isr
Function    :.
Calls		:
Called by	:
Parameter :
Author 	 : Dannier Chen
History	:
***********************************************************************/
static irqreturn_t nfc_pdma_isr(int irq, void *dev_id)
{
	struct wmt_nand_info *info = (struct wmt_nand_info *)dev_id;
	disable_irq_nosync(irq);
	//spin_lock(&host->lock);

	writel(0, info->reg + NFC_DMA_IER);
	writel(/*readl(info->reg + NFC_DMA_ISR)&*/NAND_PDMA_IER_INT_STS, info->reg + NFC_DMA_ISR);
	//printk(" pdmaisr ");
	info->dma_finish = 1;
	WARN_ON(info->done_data == NULL);
	complete(info->done_data);
	info->done_data = NULL;
	//info->done = NULL;
	//spin_unlock(&host->lock);
	enable_irq(irq);

	return IRQ_HANDLED;
}

/**********************************************************************
Name  	 : nfc_regular_isr
Function    :.
Calls		:
Called by	:
Parameter :
Author 	 : Dannier Chen
History	:
***********************************************************************/
//static irqreturn_t nfc_regular_isr(int irq, void *dev_id, struct pt_regs *regs)
irqreturn_t nfc_regular_isr(int irq, void *dev_id)
{

	struct wmt_nand_info *info = dev_id;
	struct mtd_info	*mtd = &info->mtds->mtd;
	unsigned int bank_stat1, bank_stat2=0;

	disable_irq_nosync(irq);
	//spin_lock(&host->lock);
	/* only erase/write/read_oob operation enable B2R interrupt */
	bank_stat1 = readw(info->reg + WMT_NFC_ECC_BCH_INT_STAT1);
	if (info->isr_cmd != 0) {
		//printk("isrCMD=0x%x\n", info->isr_cmd);
		if (readb(info->reg + WMT_NFC_HOST_STAT_CHANGE) & B2R) {
			writeb(B2R, info->reg + WMT_NFC_HOST_STAT_CHANGE);
			if (readb(info->reg + WMT_NFC_HOST_STAT_CHANGE) & B2R)
				printk("isr B2R staus can't clear\n");
			WARN_ON(info->done_data == NULL);
			complete(info->done_data);
			info->done_data = NULL;
		}
	}
	if (info->isr_cmd == 0 || info->isr_cmd == 0x50) {
			if (bank_stat1&(ERR_CORRECT | BCH_ERR)) {
				while (bank_stat1 != (ERR_CORRECT | BCH_ERR)) {
					//printk("1ECC status = 0x%x\n", bank_stat1);
					bank_stat2++;
					bank_stat1 = readw(info->reg + WMT_NFC_ECC_BCH_INT_STAT1);
					//printk("2ECC status = 0x%x\n", bank_stat2);
				}
				//printk("dma isrCMD=0x%x\n", info->isr_cmd);
				//printk("isrRe ecc err count = 0x%x\n", bank_stat2);
				if ((bank_stat1 & (ERR_CORRECT | BCH_ERR)) == (ERR_CORRECT | BCH_ERR)) {
					bank_stat2 = readw(info->reg + WMT_NFC_ECC_BCH_INT_STAT2);
					/* 0: data, 1: reduntant area */
					//data_redunt_flag = bank_stat2 & 0x800;
					#ifdef NAND_DEBUG
					printk(KERN_NOTICE" BCH Read data ecc eror command: %x"
					" column:%x, page_addr:%x\n", command, column, page_addr);
					printk(KERN_NOTICE "in test check bch 4/8 bit ecc correct \n");
					#endif
					if (bank_stat2 & 0x800) {
						bch4bit_redunt_ecc_correct(mtd);
					} else {
						if (info->phase)
							bch4bit_data_ecc_correct(mtd, 4096);
						else
							bch4bit_data_ecc_correct(mtd, 0);
					}
				} else
					printk("eccErr hw corr status not set bank_stat1=0x%x\n", bank_stat1);
		} /*else {
			printk("regular NOT isrWr busy\n");
		}*/
	}
	//spin_unlock(&host->lock);
	enable_irq(irq);

	return IRQ_HANDLED;
}

static int wmt_nand_probe(struct platform_device *pdev)
{
	/* struct wmt_platform_nand *plat = to_nand_plat(pdev);*/
	/*struct device *dev = &pdev->dev;*/
	struct wmt_nand_info *info;
	struct wmt_nand_mtd *nmtd;
	struct mtd_info *mtd;
	/*	struct wmt_nand_set *sets; */ /*  extend more chips and partitions structure*/
	struct resource *res;
	unsigned int varlen;
	char ret1;
	int err = 0, ret = 0, status = 0;
	unsigned char varval[100], tmp[100];
	int size;
	/*	int nr_sets;*/
	/*	int setno;*/
	*(volatile unsigned int *)(GPIO_BASE_ADDR + 0x200) &= ~(1<<1); /*PIN_SHARE_SDMMC1_NAND*/
	wmt_version = *(unsigned int *)(SYSTEM_CFG_CTRL_BASE_ADDR);
	/*	printk(KERN_NOTICE "CHIP version is %x\n", wmt_version);*/
	if (wmt_version == 0x34000101)
		return -1;  /* A0 chip not support nand flash */
	/*end wmt_revision: VT3400 A1 and Later...  */

	pr_debug("wmt_nand_probe(%p)\n", pdev);

	info = kmalloc(sizeof(*info), GFP_KERNEL);
	if (info == NULL) {
		dev_err(&pdev->dev, "no memory for flash info\n");
		err = -ENOMEM;
		goto exit_error;
	}

	memzero(info, sizeof(*info));
	dev_set_drvdata(&pdev->dev, info);
	platform_get_resource(pdev, IORESOURCE_MEM, 0);
	
	ret = request_irq(IRQ_NFC,
					nfc_regular_isr,
					IRQF_SHARED,			//SA_SHIRQ, /*SA_INTERRUPT, * that is okay?*/	//zhf: modified by James Tian, should be IRQF_SHARED?
					"NFC",
					(void *)info);
	if (ret) {
		printk(KERN_ALERT "[NFC driver] Failed to register regular ISR!\n");
		goto unmap;
	}

	ret = request_irq(IRQ_NFC_DMA,
					nfc_pdma_isr,
					IRQF_DISABLED,	//	SA_INTERRUPT,  //zhf: modified by James Tian
					"NFC",
					(void *)info);
	if (ret) {
		printk(KERN_ALERT "[NFC driver] Failed to register DMA ISR!\n");
		goto fr_regular_isr;
	}
	spin_lock_init(&info->controller.lock);
	init_waitqueue_head(&info->controller.wq);

	/* allocate and map the resource */

	/* currently we assume we have the one resource */
	res  = pdev->resource;
	size = res->end - res->start + 1;

	info->area = request_mem_region(res->start, size, pdev->name);

	if (info->area == NULL) {
		dev_err(&pdev->dev, "cannot reserve register region\n");
		err = -ENOENT;
		goto exit_error;
	}

	info->device     = &pdev->dev;
	/*	info->platform   = plat;*/
	info->reg = (void __iomem *)NF_CTRL_CFG_BASE_ADDR;/*ioremap(res->start, size);*/
	/*	info->cpu_type   = cpu_type;*/

	if (info->reg == NULL) {
		dev_err(&pdev->dev, "cannot reserve register region\n");
		err = -EIO;
		goto exit_error;
	}

/*
 * * extend more partitions
 *
	err = wmt_nand_inithw(info, pdev);
		if (err != 0)
		goto exit_error;

	sets = (plat != NULL) ? plat->sets : NULL;
	nr_sets = (plat != NULL) ? plat->nr_sets : 1;

	info->mtd_count = nr_sets;
*/
	/* allocate our information */

/*	size = nr_sets * sizeof(*info->mtds);*/
	size = sizeof(*info->mtds);
	info->mtds = kmalloc(size, GFP_KERNEL);
	if (info->mtds == NULL) {
		dev_err(&pdev->dev, "failed to allocate mtd storage\n");
		err = -ENOMEM;
		goto exit_error;
	}

	memzero(info->mtds, size);

	/* initialise all possible chips */

	nmtd = info->mtds;

	/* initialise the hardware */
	/*wmt_nfc_init(info, &nmtd->mtd);*/
	nfc_ecc_set(info, 1);  /* on hw ecc */
	
	mtd = &nmtd->mtd;
	/*	info->dmabuf = dma_alloc_coherent(&pdev->dev, 2112 + sizeof(struct nand_buffers), */
	/*  &info->dmaaddr, GFP_KERNEL);*/
	/*if (mtd->writesize == 2048) {*/
		/*info->dmabuf = dma_alloc_coherent(&pdev->dev, 2112, &info->dmaaddr, GFP_KERNEL);*/
		/*info->dmabuf = dma_alloc_coherent(&pdev->dev, 2112 + 0x300, &info->dmaaddr, GFP_KERNEL);
	} else {
		if (mtd->writesize == 4096)*/
			info->dmabuf = dma_alloc_coherent(&pdev->dev, 8640 + 0x300, &info->dmaaddr, GFP_KERNEL);
	/*	else
			info->dmabuf = dma_alloc_coherent(&pdev->dev, 528 + 0x300, &info->dmaaddr, GFP_KERNEL);
	}*/
	if (!info->dmabuf && (info->dmaaddr & 0x0f)) {
		err = -ENOMEM;
		goto out_free_dma;
	}
	/*	nmtd->chip.buffers = (void *)info->dmabuf + 2112;*/

	nmtd->chip.cmdfunc      = wmt_nand_cmdfunc;
	nmtd->chip.dev_ready    = wmt_device_ready;
	nmtd->chip.read_byte    = wmt_read_byte;
	nmtd->chip.write_buf    = wmt_nand_write_buf;
	nmtd->chip.read_buf     = wmt_nand_read_buf;
	nmtd->chip.select_chip  = wmt_nand_select_chip;
	nmtd->chip.chip_delay   = 20;
	nmtd->chip.priv	   = nmtd;
	nmtd->chip.options	   = NAND_BBT_LASTBLOCK | NAND_USE_FLASH_BBT | NAND_BBT_PERCHIP;
	/*	nmtd->chip.options	   = 0;*/
	/*	nmtd->chip.controller   = &info->controller;*/

	/*nmtd->chip.ecc.steps     = 1;
		nmtd->chip.ecc.prepad    = 1;
		nmtd->chip.ecc.postpad   = 8;*/

	nmtd->chip.ecc.mode	    = NAND_ECC_HW;
	/*nmtd->chip.ecc.mode	    = 0;*/


	/*	for (setno = 0; setno < nr_sets; setno++, nmtd++)*/
	#ifdef NAND_DEBUG
	printk(KERN_NOTICE "initialising (%p, info %p)\n", nmtd, info);
	#endif
	/*	wmt_nand_init_chip(info, nmtd, sets);*/

	/* Set up DMA address */
	/*writel(info->dmaaddr & 0xffffffff, info->reg + NFC_DMA_DAR);*/

	/*info->dmabuf = readl(info->reg + WMT_NFC_DMA_TRANS_CONFIG);*/

	/* nmtd->nand.chip_delay = 0;*/

	/* Enable the following for a flash based bad block table */
	/*	nmtd->nand.options = NAND_USE_FLASH_BBT | NAND_NO_AUTOINCR | NAND_OWN_BUFFERS;*/

	#if 1
	if (mtd->writesize == 512) {
		printk(KERN_NOTICE "wmt_oobinfo_512 \n");
		nmtd->chip.ecc.layout = &wmt_oobinfo_512;
		nmtd->chip.bbt_td = &wmt_bbt_main_descr_512;
		nmtd->chip.bbt_md = &wmt_bbt_mirror_descr_512;
	} else { /*if (NAND_PAGE_SIZE == 4096 or 2048)*/
		nmtd->chip.bbt_td = &wmt_bbt_main_descr_2048;
		nmtd->chip.bbt_md = &wmt_bbt_mirror_descr_2048;
	}
	#endif

	/*nmtd->scan_res = nand_scan(&nmtd->mtd, (sets) ? sets->nr_chips : 1);*/

	nmtd->info	   = info;
	nmtd->mtd.priv	   = &nmtd->chip;
	nmtd->mtd.owner    = THIS_MODULE;
	nmtd->mtd.reboot_notifier.notifier_call = wmt_recovery_call;//Lch

	set_ecc_engine(info, 1);
	/*writeb((PAGE_2K|WIDTH_8|WP_DISABLE|DIRECT_MAP|CHECK_ALLFF)&(~OLDDATA_EN),
	info->reg + WMT_NFC_NAND_TYPE_SEL);
 	writeb(0xfe, info->reg + WMT_NFC_CHIP_ENABLE_CTRL); // chip0 enable 
	 Send the command for reading device ID */
	/*if (!(wmt_nand_readID(&nmtd->mtd))) {
		 //Read manufacturer and device IDs 
		nand_id[0] = nmtd->chip.read_byte(&nmtd->mtd);
		nand_id[1] = nmtd->chip.read_byte(&nmtd->mtd);
		nand_id[2] = nmtd->chip.read_byte(&nmtd->mtd);
		nand_id[3] = nmtd->chip.read_byte(&nmtd->mtd);
		nand_id[4] = nmtd->chip.read_byte(&nmtd->mtd);
		printk("nand chip device maf_id is %x, and dev_id is %x\n", nand_id[0], nand_id[1]);
		if (nand_id[0] == 0x98 && nand_id[1] == 0xd5 && ((nand_id[3]>>2)&3) == 2)
			ECC8BIT_ENGINE = 1;
	}*/
	info->datalen = 0;
	ret1 = wmt_nfc_init(info, &nmtd->mtd);
	if (ret1 == 2)
		goto out_free_dma;
	writeb(0xff, info->reg + WMT_NFC_CHIP_ENABLE_CTRL); //chip disable 
	/*for (j = 0; j < 0xA8; j += 16)
				printk(KERN_NOTICE "NFCR%x ~ NFCR%x = 0x%8.8x 0x%8.8x 0x%8.8x 0x%8.8x\r\n",
				j/4, (j+12)/4,
				readl(info->reg + j + 0),
				readl(info->reg + j + 4),
				readl(info->reg + j + 8),
				readl(info->reg + j + 12));*/
	wmt_nand_init_chip(info, nmtd);
	#ifndef NAND_HARMING_ECC
	if (ECC8BIT_ENGINE == 1) {
		printk(KERN_NOTICE "BCH ECC 12BIT \n");
		set_ecc_engine(info, 3);  /* BCH ECC new structure */
	} else
		/*printk(KERN_NOTICE "BCH ECC 4BIT \n");*/
		set_ecc_engine(info, 1);  /* BCH ECC new structure */

	#else
	#ifdef NAND_BBT_BCH_ECC
		ecc_type = 1;
		if (ECC8BIT_ENGINE == 1) {
		printk(KERN_NOTICE "BBT BCH ECC 12BIT \n");
		set_ecc_engine(info, 3); /* write bbt with BCH ECC new structure */
	 } else
		/*printk(KERN_NOTICE "BBT BCH ECC 4BIT \n");*/
		set_ecc_engine(info, 1); /* write bbt with BCH ECC new structure */

	#else
		set_ecc_engine(info, 0);  /* Harming ECC  */
	#endif
	#endif

	if (mtd->writesize == 2048) {
		#ifndef NAND_HARMING_ECC
			nmtd->chip.ecc.layout = &wmt_oobinfo_2048;
		#else
			/* printk(KERN_NOTICE "hm_oob_2048 \n");*/
			#ifdef NAND_BBT_BCH_ECC
			nmtd->chip.ecc.layout = &wmt_oobinfo_2048;
			#else
			nmtd->chip.ecc.layout = &wmt_hm_oobinfo_2048;
			#endif
		#endif
	} else if (mtd->writesize == 4096 || mtd->writesize == 8192) {
		#ifndef NAND_HARMING_ECC
		if(ECC8BIT_ENGINE == 1)
			nmtd->chip.ecc.layout = &wmt_12bit_oobinfo_4096;
			//nmtd->chip.ecc.layout = &wmt_8bit_oobinfo_4096;
		else
			nmtd->chip.ecc.layout = &wmt_oobinfo_4096;
		#else
			#ifdef NAND_BBT_BCH_ECC
			if(ECC8BIT_ENGINE == 1)
				nmtd->chip.ecc.layout = &wmt_12bit_oobinfo_4096;
				//nmtd->chip.ecc.layout = &wmt_8bit_oobinfo_4096;
			else
				nmtd->chip.ecc.layout = &wmt_oobinfo_4096;
			#else
			nmtd->chip.ecc.layout = &wmt_hm_oobinfo_4096;
			#endif
		#endif
	} else {
		nmtd->chip.ecc.layout = &wmt_oobinfo_512;
		nmtd->chip.bbt_td = &wmt_bbt_main_descr_512;
		nmtd->chip.bbt_md = &wmt_bbt_mirror_descr_512;
	}


	nmtd->scan_res = nand_scan(&nmtd->mtd, MAX_CHIP);


	if (nmtd->scan_res == 0) {
		/* wmt_nand_add_partition(info, nmtd, sets);*/

		#ifndef NAND_HARMING_ECC
		/* set nand flash new structure */
		/* writew(readw(info->reg + WMT_NFC_ECC_BCH_INT_MASK) | 0x101,
		info->reg + WMT_NFC_ECC_BCH_INT_MASK);*/

		/* ecc_type = 1;*/
		/* for test */
		#if 0
		/* wmt_bch_ecc_format_nandflash(&nmtd->mtd, -1);*/
		printk(KERN_NOTICE "formating ok\n");
		read__ecccode_test(&nmtd->mtd, &nmtd->chip, 0);
		#endif
		/* set_ecc_engine(info, nandtype);*/
		/* nmtd->chip.ecc.layout = &wmt_oobinfo_2048;*/
		#endif


		#ifdef NAND_HARMING_ECC
		ecc_type = 0;
		if (mtd->writesize == 2048) {
			nmtd->chip.ecc.layout = &wmt_hm_oobinfo_2048;
		} else if (mtd->writesize == 4096 || mtd->writesize == 8192)
			nmtd->chip.ecc.layout = &wmt_hm_oobinfo_4096;

		*(volatile unsigned long *)PMCEU_ADDR |= (0x0010000);/*add by vincent*/
		set_ecc_engine(info, 0);  /* Harming ECC  */
		*(volatile unsigned long *)PMCEU_ADDR &= ~(0x0010000);/*add by vincent*/
		#endif


	#ifdef CONFIG_MTD_PARTITIONS
		#ifdef CONFIG_MTD_CMDLINE_PARTS
		err = add_dynamic_parts(&nmtd->mtd);
		if (err < 0) {
			printk(KERN_ERR "WMT_nand: uboot no dynamic partitions defined, use default static\n");
			/*err = -ENODEV;
			nand_release(mtd);*/
			add_mtd_partitions(&nmtd->mtd, nand_partitions, ARRAY_SIZE(nand_partitions));
		}
		#else
		add_mtd_partitions(&nmtd->mtd, nand_partitions, ARRAY_SIZE(nand_partitions));
		#endif
	#else
		add_mtd_device(&nmtd->mtd);
	#endif
	}

	err = search_mtd_table("u-boot-logo", &ret1);
	ret = (int) ret1;
	varlen = 100;
	status = wmt_getsyspara("wmt.nfc.mtd.u-boot-logo", tmp, &varlen);
	sprintf(varval, "0x%llx", nand_partitions[ret].offset);
	if (!status || (strcmp(varval, tmp) == 0))
		status = 0;
	if (!err && status) {
		ret = wmt_setsyspara("wmt.nfc.mtd.u-boot-logo", varval);
		if (ret)
			printk(KERN_NOTICE "write u-boot-logo offset to env fail\n");
	} else if (err)
		printk(KERN_NOTICE "search u-boot-logo partition fail\n");

	err = search_mtd_table("kernel-logo", &ret1);
	ret = (int) ret1;
	varlen = 100;
	status = wmt_getsyspara("wmt.nfc.mtd.kernel-logo", tmp, &varlen);
	sprintf(varval, "0x%llx", nand_partitions[ret].offset);
	if (!status || (strcmp(varval, tmp) == 0))
		status = 0;
	if (!err && status) {
		ret = wmt_setsyspara("wmt.nfc.mtd.kernel-logo", varval);
		if (ret)
			printk(KERN_NOTICE "write kernel-logo offset to env fail\n");
	} else if (err)
		printk(KERN_NOTICE "search kernel-logo partition fail\n");

	if (((*(volatile unsigned long *)(GPIO_BASE_ADDR + 0x100))&6) == 2)
		spin_lock_init(nand_lock);

	register_reboot_notifier(&mtd->reboot_notifier);//Lch
	printk(KERN_NOTICE "nand initialised ok\n");
	return 0;

out_free_dma:
	/*if (mtd->writesize == 2048)
		dma_free_coherent(&pdev->dev, 2112 + 0x300, info->dmabuf, info->dmaaddr);
	else if (mtd->writesize == 4096)*/
		dma_free_coherent(&pdev->dev, 8640/*4224*/ + 0x300, info->dmabuf, info->dmaaddr);
	/*else
		dma_free_coherent(&pdev->dev, 528 + 0x300, info->dmabuf, info->dmaaddr);*/
fr_regular_isr:
unmap:
exit_error:
	wmt_nand_remove(pdev);

	if (err == 0)
		err = -EINVAL;
	return err;
}

/* PM Support */
#ifdef CONFIG_PM
int wmt_nand_suspend(struct platform_device *pdev, pm_message_t state)
{
	/*struct wmt_nand_info *info = dev_get_drvdata(&pdev->dev);*/

	if (((*(volatile unsigned long *)(GPIO_BASE_ADDR + 0x100))&6) == 2) {
			*(volatile unsigned long *)PMCEU_ADDR |= (0x0010000);
		/*writel(0x0, info->reg + WMT_NFC_COMPORT0);
		writel(0x0, info->reg + WMT_NFC_COMPORT1_2);
		writel(0x0, info->reg + WMT_NFC_COMPORT3_4);
		writel(0x30, info->reg + WMT_NFC_COMPORT5_6);
		writel(0x1FF, info->reg + WMT_NFC_DMA_COUNTER);
		writel(0x41, info->reg + WMT_NFC_NAND_TYPE_SEL);
		writel(0x12123636, info->reg + WMT_NFC_READ_CYCLE_PULE_CTRL);
		writel(0x0, info->reg + WMT_NFC_CHIP_ENABLE_CTRL);
		writel(0x2, info->reg + WMT_NFC_CALC_CTRL);
		writel(0x101, info->reg + WMT_NFC_ECC_BCH_INT_MASK);
		writel(0x1, info->reg + WMT_NFC_NANDFLASH_BOOT);*/
		*(volatile unsigned long *)(NF_CTRL_CFG_BASE_ADDR + 0x88) |= (1<<5);
	printk(KERN_NOTICE "reset nand boot register NF_CTRL_CFG_BASE_ADDR + 0x88\n");
	*(volatile unsigned long *)(NF_CTRL_CFG_BASE_ADDR + 0x88) &= ~(1<<5);
	}
	printk(KERN_NOTICE "wmt_nand_suspend\n");
	return 0;
}

int wmt_nand_resume(struct platform_device *pdev)
{
	struct wmt_nand_info *info = dev_get_drvdata(&pdev->dev);
	struct wmt_nand_mtd *nmtd;
	int ret;
	*(volatile unsigned long *)PMCEU_ADDR |= (0x0010000);/*add by vincent*/
	if (info) {
			nmtd = info->mtds;
		if (((*(volatile unsigned long *)(GPIO_BASE_ADDR + 0x100))&6) == 2)
			writel(0x0, info->reg + WMT_NFC_NANDFLASH_BOOT);
		/* initialise the hardware */
		ret = wmt_nfc_init(info, &nmtd->mtd);
		if (ret == 2)
			while(ret);
		nfc_ecc_set(info, 1);  /* on hw ecc */
		/* Set up DMA address */
		/*writel(info->dmaaddr & 0xffffffff, info->reg + NFC_DMA_DAR);*/

		if (ecc_type == 1) { /* nand new structure  */
			if (ECC8BIT_ENGINE == 1)
				set_ecc_engine(info, 3); /* BCH ECC */
			else
				set_ecc_engine(info, 1); /* BCH ECC */
		} else
			set_ecc_engine(info, 0);  /* Harmming ECC */

		printk(KERN_NOTICE "wmt_nand_resume OK\n");
	} else
		printk(KERN_NOTICE "wmt_nand_resume error\n");

	*(volatile unsigned long *)PMCEU_ADDR &= ~(0x0010000);/*add by vincent*/
	return 0;
}

#else /* else of #define PM */
#define wmt_nand_suspend NULL
#define wmt_nand_resume NULL
#endif

/*struct platform_driver wmt_nand_driver = {*/
struct platform_driver wmt_nand_driver = {
	.driver.name	= "nand",
	.probe = wmt_nand_probe,
	.remove = wmt_nand_remove,
	.suspend = wmt_nand_suspend,
	.resume = wmt_nand_resume
	/*
	.driiver = {
	.name	= "wmt-nand",
	.owner	= THIS_MODULE,
	},
	*/
};

static int __init wmt_nand_init(void)
{
	//printk(KERN_NOTICE "NAND Driver, WonderMedia Technologies, Inc\n");
	return platform_driver_register(&wmt_nand_driver);
}

static void __exit wmt_nand_exit(void)
{
	platform_driver_unregister(&wmt_nand_driver);
}

module_init(wmt_nand_init);
module_exit(wmt_nand_exit);

MODULE_AUTHOR("WonderMedia Technologies, Inc.");
MODULE_DESCRIPTION("WMT [Nand Flash Interface] driver");
MODULE_LICENSE("GPL");
