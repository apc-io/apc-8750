/*
 *  drivers/mtd/nandids.c
 *
 *  Copyright (C) 2002 Thomas Gleixner (tglx@linutronix.de)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#include <linux/module.h>
#include <linux/mtd/nand.h>
uint32_t NFC_RWTimming = 0x2424;
uint32_t ECC8BIT_ENGINE = 0;
uint32_t redunt_err_mark = 0;
uint32_t NFC_ClockDivisor = 0x0c;
uint32_t NFC_ClockMask  = 0x1FF;
uint32_t chip_swap = 0;
/*
*	Chip ID list
*
*	Name. ID code, pagesize, chipsize in MegaByte, eraseblock size,
*	options
*
*	Pagesize; 0, 256, 512
*	0	get this information from the extended chip ID
+	256	256 Byte page size
*	512	512 Byte page size
*/
struct nand_flash_dev nand_flash_ids[] = {

#ifdef CONFIG_MTD_NAND_MUSEUM_IDS
	{"NAND 1MiB 5V 8-bit",		0x6e, 256, 1, 0x1000, 0},
	{"NAND 2MiB 5V 8-bit",		0x64, 256, 2, 0x1000, 0},
	{"NAND 4MiB 5V 8-bit",		0x6b, 512, 4, 0x2000, 0},
	{"NAND 1MiB 3,3V 8-bit",	0xe8, 256, 1, 0x1000, 0},
	{"NAND 1MiB 3,3V 8-bit",	0xec, 256, 1, 0x1000, 0},
	{"NAND 2MiB 3,3V 8-bit",	0xea, 256, 2, 0x1000, 0},
	{"NAND 4MiB 3,3V 8-bit",	0xd5, 512, 4, 0x2000, 0},
	{"NAND 4MiB 3,3V 8-bit",	0xe3, 512, 4, 0x2000, 0},
	{"NAND 4MiB 3,3V 8-bit",	0xe5, 512, 4, 0x2000, 0},
	{"NAND 8MiB 3,3V 8-bit",	0xd6, 512, 8, 0x2000, 0},

	{"NAND 8MiB 1,8V 8-bit",	0x39, 512, 8, 0x2000, 0},
	{"NAND 8MiB 3,3V 8-bit",	0xe6, 512, 8, 0x2000, 0},
	{"NAND 8MiB 1,8V 16-bit",	0x49, 512, 8, 0x2000, NAND_BUSWIDTH_16},
	{"NAND 8MiB 3,3V 16-bit",	0x59, 512, 8, 0x2000, NAND_BUSWIDTH_16},
#endif

	{"NAND 16MiB 1,8V 8-bit",	0x33, 512, 16, 0x4000, 0},
	{"NAND 16MiB 3,3V 8-bit",	0x73, 512, 16, 0x4000, 0},
	{"NAND 16MiB 1,8V 16-bit",	0x43, 512, 16, 0x4000, NAND_BUSWIDTH_16},
	{"NAND 16MiB 3,3V 16-bit",	0x53, 512, 16, 0x4000, NAND_BUSWIDTH_16},

	{"NAND 32MiB 1,8V 8-bit",	0x35, 512, 32, 0x4000, 0},
	{"NAND 32MiB 3,3V 8-bit",	0x75, 512, 32, 0x4000, 0},
	{"NAND 32MiB 1,8V 16-bit",	0x45, 512, 32, 0x4000, NAND_BUSWIDTH_16},
	{"NAND 32MiB 3,3V 16-bit",	0x55, 512, 32, 0x4000, NAND_BUSWIDTH_16},

	{"NAND 64MiB 1,8V 8-bit",	0x36, 512, 64, 0x4000, 0},
	{"NAND 64MiB 3,3V 8-bit",	0x76, 512, 64, 0x4000, 0},
	{"NAND 64MiB 1,8V 16-bit",	0x46, 512, 64, 0x4000, NAND_BUSWIDTH_16},
	{"NAND 64MiB 3,3V 16-bit",	0x56, 512, 64, 0x4000, NAND_BUSWIDTH_16},

	{"NAND 128MiB 1,8V 8-bit",	0x78, 512, 128, 0x4000, 0},
	{"NAND 128MiB 1,8V 8-bit",	0x39, 512, 128, 0x4000, 0},
	{"NAND 128MiB 3,3V 8-bit",	0x79, 512, 128, 0x4000, 0},
	{"NAND 128MiB 1,8V 16-bit",	0x72, 512, 128, 0x4000, NAND_BUSWIDTH_16},
	{"NAND 128MiB 1,8V 16-bit",	0x49, 512, 128, 0x4000, NAND_BUSWIDTH_16},
	{"NAND 128MiB 3,3V 16-bit",	0x74, 512, 128, 0x4000, NAND_BUSWIDTH_16},
	{"NAND 128MiB 3,3V 16-bit",	0x59, 512, 128, 0x4000, NAND_BUSWIDTH_16},

	{"NAND 256MiB 3,3V 8-bit",	0x71, 512, 256, 0x4000, 0},

	/*
	 * These are the new chips with large page size. The pagesize and the
	 * erasesize is determined from the extended id bytes
	 */
#define LP_OPTIONS (NAND_SAMSUNG_LP_OPTIONS | NAND_NO_READRDY | NAND_NO_AUTOINCR)
#define LP_OPTIONS16 (LP_OPTIONS | NAND_BUSWIDTH_16)

	/*512 Megabit */
	{"NAND 64MiB 1,8V 8-bit",	0xA2, 0,  64, 0, LP_OPTIONS},
	{"NAND 64MiB 3,3V 8-bit",	0xF2, 0,  64, 0, LP_OPTIONS},
	{"NAND 64MiB 1,8V 16-bit",	0xB2, 0,  64, 0, LP_OPTIONS16},
	{"NAND 64MiB 3,3V 16-bit",	0xC2, 0,  64, 0, LP_OPTIONS16},

	/* 1 Gigabit */
	{"NAND 128MiB 1,8V 8-bit",	0xA1, 0, 128, 0, LP_OPTIONS},
	{"NAND 128MiB 3,3V 8-bit",	0xF1, 0, 128, 0, LP_OPTIONS},
	{"NAND 128MiB 1,8V 16-bit",	0xB1, 0, 128, 0, LP_OPTIONS16},
	{"NAND 128MiB 3,3V 16-bit",	0xC1, 0, 128, 0, LP_OPTIONS16},

	/* 2 Gigabit */
	{"NAND 256MiB 1,8V 8-bit",	0xAA, 0, 256, 0, LP_OPTIONS},
	{"NAND 256MiB 3,3V 8-bit",	0xDA, 0, 256, 0, LP_OPTIONS},
	{"NAND 256MiB 1,8V 16-bit",	0xBA, 0, 256, 0, LP_OPTIONS16},
	{"NAND 256MiB 3,3V 16-bit",	0xCA, 0, 256, 0, LP_OPTIONS16},

	/* 4 Gigabit */
	{"NAND 512MiB 1,8V 8-bit",	0xAC, 0, 512, 0, LP_OPTIONS},
	{"NAND 512MiB 3,3V 8-bit",	0xDC, 0, 512, 0, LP_OPTIONS},
	{"NAND 512MiB 1,8V 16-bit",	0xBC, 0, 512, 0, LP_OPTIONS16},
	{"NAND 512MiB 3,3V 16-bit",	0xCC, 0, 512, 0, LP_OPTIONS16},

	/* 8 Gigabit */
	{"NAND 1GiB 1,8V 8-bit",	0xA3, 0, 1024, 0, LP_OPTIONS},
	{"NAND 1GiB 3,3V 8-bit",	0xD3, 0, 1024, 0, LP_OPTIONS},
	{"NAND 1GiB 1,8V 16-bit",	0xB3, 0, 1024, 0, LP_OPTIONS16},
	{"NAND 1GiB 3,3V 16-bit",	0xC3, 0, 1024, 0, LP_OPTIONS16},

	/* 16 Gigabit */
	{"NAND 2GiB 1,8V 8-bit",	0xA5, 0, 2048, 0, LP_OPTIONS},
	{"NAND 2GiB 3,3V 8-bit",	0xD5, 0, 2048, 0, LP_OPTIONS},
	{"NAND 2GiB 1,8V 16-bit",	0xB5, 0, 2048, 0, LP_OPTIONS16},
	{"NAND 2GiB 3,3V 16-bit",	0xC5, 0, 2048, 0, LP_OPTIONS16},

	/*
	 * Renesas AND 1 Gigabit. Those chips do not support extended id and
	 * have a strange page/block layout !  The chosen minimum erasesize is
	 * 4 * 2 * 2048 = 16384 Byte, as those chips have an array of 4 page
	 * planes 1 block = 2 pages, but due to plane arrangement the blocks
	 * 0-3 consists of page 0 + 4,1 + 5, 2 + 6, 3 + 7 Anyway JFFS2 would
	 * increase the eraseblock size so we chose a combined one which can be
	 * erased in one go There are more speed improvements for reads and
	 * writes possible, but not implemented now
	 */
	{"AND 128MiB 3,3V 8-bit",	0x01, 2048, 128, 0x4000,
	 NAND_IS_AND | NAND_NO_AUTOINCR |NAND_NO_READRDY | NAND_4PAGE_ARRAY |
	 BBT_AUTO_REFRESH
	},

	{NULL,}
};

#define NAND_TYPE_MLC 1
#define NAND_TYPE_SLC 0
#define WIDTH_8 0
struct WMT_nand_flash_dev WMT_nand_flash_ids[] = {
	/*{0xEC75A5BD, 2048, 512, 16, 16384, 4, 0, 1, 5, WIDTH_8,
   2, 1, 0, NAND_TYPE_SLC, 1, 0, 0x2424, "SAMSUNG_NF_K9F5608U0D"},    //2th tested. 1 chip.
  {0xEC35A5BD, 2048, 512, 16, 16384, 4, 0, 1, 5, WIDTH_8,
   2, 1, 0, NAND_TYPE_SLC, 1, 0, 0x2424, "SAMSUNG_NF_K9F5608U0D"},
  {0xEC76A5C0, 4096, 512, 16, 16384, 4, 0, 1, 5, WIDTH_8,
   1, 1, 0, NAND_TYPE_SLC, 1, 0, 0x2424, "SAMSUNG_NF_K9F1208U0B"},    //3th tested. 1 chip.
  {0xEC36A5C0, 4096, 512, 16, 16384, 4, 0, 1, 5, WIDTH_8,
   1, 1, 0, NAND_TYPE_SLC, 1, 0, 0x2424, "SAMSUNG_NF_K9F1208U0B"},
  {0xECF10095, 1024, 2048, 16, 131072, 5, 0, 1, 0, WIDTH_8,
   4, 0, 1, NAND_TYPE_SLC, 1, 0, 0x2424, "SAMSUNG_NF_K9F1G08U0B"},    //7th tested. 1 chip
  {0xECA10095, 1024, 2048, 16, 131072, 5, 0, 1, 0, WIDTH_8,
   4, 0, 1, NAND_TYPE_SLC, 1, 0, 0x2424, "SAMSUNG_NF_K9F1G08U0B"},
  {0xECAA1095, 2048, 2048, 16, 131072, 5, 0, 1, 0, WIDTH_8,
   4, 0, 1, NAND_TYPE_SLC, 1, 0, 0x2424, "SAMSUNG_NF_K9F2G08UXA"},    //4th tested. 1 chip.
  {0xECDA1095, 2048, 2048, 16, 131072, 5, 0, 1, 0, WIDTH_8,
   4, 0, 1, NAND_TYPE_SLC, 1, 0, 0x2424, "SAMSUNG_NF_K9F2G08UXA"},
  {0xECDC1095, 4096, 2048, 16, 131072, 5, 0, 1, 0, WIDTH_8,
   4, 0, 1, NAND_TYPE_SLC, 1, 0, 0x2424, "SAMSUNG_NF_K9F4G08U0A"},    //5th tested. 1 chip.
  {0xECAC1095, 4096, 2048, 16, 131072, 5, 0, 1, 0, WIDTH_8,
   4, 0, 1, NAND_TYPE_SLC, 1, 0, 0x2424, "SAMSUNG_NF_K9F4G08U0A"},
  {0xECD310A6, 4096, 4096, 16, 262144, 5, 0, 1, 0, WIDTH_8,
   4, 0, 1, NAND_TYPE_SLC, 1, 0, 0x2424, "SAMSUNG_NF_K9F8G08U0M"},    //6th tested. 1 chip.
  {0xECDA1425, 1024, 2048, 16, 262144, 5, 0, 127, 0, WIDTH_8,
   1, 0, 1, NAND_TYPE_MLC, 4, 0, 0x2424, "SAMSUNG_NF_K9G2G08U0M"},
  {0xECD514B6, 4096, 4096, 16, 524288, 5, 0, 127, 0, WIDTH_8,
   1, 0, 1, NAND_TYPE_MLC, 4, 0, 0x2424, "SAMSUNG_NF_K9GAG08U0M"},   
  {0xECD755B6, 8192, 4096, 16, 524288, 5, 0, 127, 0, WIDTH_8,
   1, 0, 1, NAND_TYPE_MLC, 4, 0, 0x2424, "SAMSUNG_NF_K9LBG08U0M"},    
  {0x20DC8095, 4096, 2048, 16, 131072, 5, 0 ,1, 0, WIDTH_8,
   4, 0, 1, NAND_TYPE_SLC, 1, 0, 0x2424, "ST_NF_NAND04GW3B2B"},   
  {0x20D38195, 8192, 2048, 16, 131072, 5, 0 ,1, 0, WIDTH_8,
   4, 0, 1, NAND_TYPE_SLC, 1, 0, 0x2424, "ST_NF_NAND08GW3B2A"},   
  {0xADF1801D, 1024, 2048, 16, 131072, 4, 0, 1, 0, WIDTH_8,
   4, 0, 1, NAND_TYPE_SLC, 1, 0, 0x2424, "HYNIX_NF_HY27UF081G2A"},   
  {0xADDC8095, 4096, 2048, 16, 131072, 5, 0, 1, 0, WIDTH_8,
   4, 0, 1, NAND_TYPE_SLC, 1, 0, 0x2424, "HYNIX_NF_HY27UF084G2M"},
  {0x20D314A5, 4096, 2048, 16, 262144, 5, 126, 127, 0, WIDTH_8,
   1, 0, 1, NAND_TYPE_MLC, 4, 0, 0x2424, "ST_NF_NAND08GW3C2A"},
  {0xADD314A5, 4096, 2048, 16, 262144, 5, 126, 127, 0, WIDTH_8,
   1, 0, 1, NAND_TYPE_MLC, 4, 0, 0x2424, "HYNIX_NF_HY27UT088G2M"},
  {0x98D585A5, 8192, 2048, 16, 262144, 5, 0, 127, 0, WIDTH_8,
   1, 0, 1, NAND_TYPE_MLC, 4, 0, 0x2424, "TOSHIBA_NF_TH58NVG4D4C"},*/
	
	{0xADD314A5, 4096, 2048,  64, 0x40000, 5, 125, 127, 0, WIDTH_8, 1, 0, 1, NAND_TYPE_MLC,  4, 0x140A0C46, "HY27UT088G2M-T(P)", LP_OPTIONS},
	{0xADF1801D, 1024, 2048,  64, 0x20000, 4,   0,   1, 0, WIDTH_8, 4, 0, 1, NAND_TYPE_SLC,  4, 0x140A0F64, "HY27UF081G2A", LP_OPTIONS},
	{0xADF1001D, 1024, 2048,  64, 0x20000, 4,   0,   1, 0, WIDTH_8, 4, 0, 1, NAND_TYPE_SLC,  4, 0x140A0C46, "H27U1G8F2BFR", LP_OPTIONS},
	{0xADD59425, 4096, 4096, 218, 0x80000, 5, 125, 127, 0, WIDTH_8, 1, 0, 1, NAND_TYPE_MLC, 12, 0x140A0F64, "HY27UAG8T2A", LP_OPTIONS},
	{0xADD7949A, 2048, 8192, 218/*448*/,0x200000, 5,   0, 255, 0, WIDTH_8, 1, 0, 1, NAND_TYPE_MLC, 12/*24*/, 0x140A0C64, "HY27UBG8T2ATR", LP_OPTIONS},
	//MAX 60 BAD BLOCKs
	{0xECD314A5, 4096, 2048,  64, 0x40000, 5, 127,   0, 0, WIDTH_8, 1, 0, 1, NAND_TYPE_MLC,  4, 0x140A0C64, "K9G8G08X0A", LP_OPTIONS},
	//MAX 100 BAD BLOCKs
	{0xECD59429, 4096, 4096, 218, 0x80000, 5, 127,   0, 0, WIDTH_8, 1, 0, 1, NAND_TYPE_MLC, 12, 0x140A0F64, "K9GAG08UXD", LP_OPTIONS},
	{0xECF10095, 1024, 2048,  64, 0x20000, 4,   0,   1, 0, WIDTH_8, 4, 0, 1, NAND_TYPE_SLC,  4, 0x140a1464, "K9F1G08U0B", LP_OPTIONS},
	{0xEC75A5BD, 2048,  512,  16,  0x4000, 4,   0,   1, 5, WIDTH_8, 1, 1, 0, NAND_TYPE_SLC,  1, 0x230F1964, "K95608U0D", 0},
	{0xECD514B6, 4096, 4096, 128, 0x80000, 5, 127,   0, 0, WIDTH_8, 1, 0, 1, NAND_TYPE_MLC,  4, 0x140A0C64, "K9GAG08U0M", LP_OPTIONS},
	{0xECD755B6, 8192, 4096, 128, 0x80000, 5, 127,   0, 0, WIDTH_8, 1, 0, 1, NAND_TYPE_MLC,  4, 0x140A0C64, "K9LBG08U0M", LP_OPTIONS},
	{0xECD58472, 2048, 8192, 218/*436*/,0x100000, 5,   0, 127, 0, WIDTH_8, 1, 0, 1, NAND_TYPE_MLC, 12/*24*/, 0x190A0FFF, "K9GAG08U0E", LP_OPTIONS},
	{0xECD59476, 2048, 8192, 218/*448*/,0x100000, 5,   0, 127, 0, WIDTH_8, 1, 0, 1, NAND_TYPE_MLC, 12/*24*/, 0x140A0CFF, "K9GAG08U0F", LP_OPTIONS},
	
	{0x98D594BA, 4096, 4096, 218, 0x80000, 5,   0, 127, 0, WIDTH_8, 1, 0, 1, NAND_TYPE_MLC, 12, 0x190F0F70, "TC58NVG4D1DTG0", LP_OPTIONS},
	{0x98D19015, 1024, 2048,  64, 0x20000, 4,   0,   1, 0, WIDTH_8, 4, 0, 1, NAND_TYPE_SLC,  4, 0x140A0C11, "TC58NVG0S3ETA00", LP_OPTIONS},
	{0x98D59432, 2048, 8192, 218/*448*/,0x100000, 5,   0, 127, 0, WIDTH_8, 1, 0, 1, NAND_TYPE_MLC, 12/*24*/, 0x140A0C84, "TC58NVG4D2FTA00", LP_OPTIONS},
	{0,}
	/*add new product item here.*/
};

/*
*	Manufacturer ID list
*/
struct nand_manufacturers nand_manuf_ids[] = {
	{NAND_MFR_TOSHIBA, "Toshiba"},
	{NAND_MFR_SAMSUNG, "Samsung"},
	{NAND_MFR_FUJITSU, "Fujitsu"},
	{NAND_MFR_NATIONAL, "National"},
	{NAND_MFR_RENESAS, "Renesas"},
	{NAND_MFR_STMICRO, "ST Micro"},
	{NAND_MFR_HYNIX, "Hynix"},
	{NAND_MFR_MICRON, "Micron"},
	{NAND_MFR_AMD, "AMD"},
	{0x0, "Unknown"}
};

EXPORT_SYMBOL(nand_manuf_ids);
EXPORT_SYMBOL(nand_flash_ids);
EXPORT_SYMBOL(WMT_nand_flash_ids);

EXPORT_SYMBOL(NFC_RWTimming);
EXPORT_SYMBOL(NFC_ClockDivisor);
EXPORT_SYMBOL(NFC_ClockMask);
EXPORT_SYMBOL(chip_swap);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Thomas Gleixner <tglx@linutronix.de>");
MODULE_DESCRIPTION("Nand device & manufacturer IDs");
