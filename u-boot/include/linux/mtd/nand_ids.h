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
 *  u-boot/include/linux/mtd/nand_ids.h
 *
 *  Copyright (c) 2000 David Woodhouse <dwmw2@mvhi.com>
 *                     Steven J. Hill <sjhill@cotw.com>
 *
 * $Id: nand_ids.h,v 1.1 2000/10/13 16:16:26 mdeans Exp $
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  Info:
 *   Contains standard defines and IDs for NAND flash devices
 *
 *  Changelog:
 *   01-31-2000 DMW     Created
 *   09-18-2000 SJH     Moved structure out of the Disk-On-Chip drivers
 *			so it can be used by other NAND flash device
 *			drivers. I also changed the copyright since none
 *			of the original contents of this file are specific
 *			to DoC devices. David can whack me with a baseball
 *			bat later if I did something naughty.
 *   10-11-2000 SJH     Added private NAND flash structure for driver
 *   2000-10-13 BE      Moved out of 'nand.h' - avoids duplication.
 */

#ifndef __LINUX_MTD_NAND_IDS_H
#define __LINUX_MTD_NAND_IDS_H
#if 0
static struct nand_flash_dev nand_flash_ids[] = {
	{"Toshiba TC5816BDC",     NAND_MFR_TOSHIBA, 0x64, 21, 1, 2, 0x1000, 0},
	{"Toshiba TC5832DC",      NAND_MFR_TOSHIBA, 0x6b, 22, 0, 2, 0x2000, 0},
	{"Toshiba TH58V128DC",    NAND_MFR_TOSHIBA, 0x73, 24, 0, 2, 0x4000, 0},
	{"Toshiba TC58256FT/DC",  NAND_MFR_TOSHIBA, 0x75, 25, 0, 2, 0x4000, 0},
	{"Toshiba TH58512FT",     NAND_MFR_TOSHIBA, 0x76, 26, 0, 3, 0x4000, 0},
	{"Toshiba TC58V32DC",     NAND_MFR_TOSHIBA, 0xe5, 22, 0, 2, 0x2000, 0},
	{"Toshiba TC58V64AFT/DC", NAND_MFR_TOSHIBA, 0xe6, 23, 0, 2, 0x2000, 0},
	{"Toshiba TC58V16BDC",    NAND_MFR_TOSHIBA, 0xea, 21, 1, 2, 0x1000, 0},
	{"Toshiba TH58100FT",     NAND_MFR_TOSHIBA, 0x79, 27, 0, 3, 0x4000, 0},
	{"Samsung KM29N16000",    NAND_MFR_SAMSUNG, 0x64, 21, 1, 2, 0x1000, 0},
	{"Samsung unknown 4Mb",   NAND_MFR_SAMSUNG, 0x6b, 22, 0, 2, 0x2000, 0},
	{"Samsung KM29U128T",     NAND_MFR_SAMSUNG, 0x73, 24, 0, 2, 0x4000, 0},
	{"Samsung KM29U256T",     NAND_MFR_SAMSUNG, 0x75, 25, 0, 2, 0x4000, 0},
	{"Samsung unknown 64Mb",  NAND_MFR_SAMSUNG, 0x76, 26, 0, 3, 0x4000, 0},
	{"Samsung KM29W32000",    NAND_MFR_SAMSUNG, 0xe3, 22, 0, 2, 0x2000, 0},
	{"Samsung unknown 4Mb",   NAND_MFR_SAMSUNG, 0xe5, 22, 0, 2, 0x2000, 0},
	{"Samsung KM29U64000",    NAND_MFR_SAMSUNG, 0xe6, 23, 0, 2, 0x2000, 0},
	{"Samsung KM29W16000",    NAND_MFR_SAMSUNG, 0xea, 21, 1, 2, 0x1000, 0},
	{"Samsung K9F5616Q0C",    NAND_MFR_SAMSUNG, 0x45, 25, 0, 2, 0x4000, 1},
	{"Samsung K9K1216Q0C",    NAND_MFR_SAMSUNG, 0x46, 26, 0, 3, 0x4000, 1},
	{0,}
};
#endif
#define NAND_TYPE_MLC 1
#define NAND_TYPE_SLC 0
#define WIDTH_8 0
static struct WMT_nand_flash_dev WMT_nand_flash_ids[] = {
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
	
	{0xADD314A5, 4096, 2048,  64, 0x40000, 5, 125, 127, 0, WIDTH_8, 1, 0, 1, NAND_TYPE_MLC,  4, 0x140A0C46, "HY27UT088G2M-T(P)"},
	{0xADF1801D, 1024, 2048,  64, 0x20000, 4,   0,   1, 0, WIDTH_8, 4, 0, 1, NAND_TYPE_SLC,  4, 0x140A0F64, "HY27UF081G2A"},
	{0xADF1001D, 1024, 2048,  64, 0x20000, 4,   0,   1, 0, WIDTH_8, 4, 0, 1, NAND_TYPE_SLC,  4, 0x140A0C46, "H27U1G8F2BFR"},
	{0xADD7949A, 2048, 8192, 218/*448*/,0x200000, 5,   0, 255, 0, WIDTH_8, 1, 0, 1, NAND_TYPE_MLC, 12/*24*/, 0x140A0C64, "HY27UBG8T2ATR"},
	{0x2C88044B, 4096, 8192, 218/*448*/,0x200000, 5,   0, 255, 0, WIDTH_8, 1, 0, 1, NAND_TYPE_MLC, 12/*24*/, 0x281E32C8, "MT29F64G08CBAAA"},
	//MAX 60 BAD BLOCKs
	{0xECD314A5, 4096, 2048,  64, 0x40000, 5, 127,   0, 0, WIDTH_8, 1, 0, 1, NAND_TYPE_MLC,  4, 0x140A0C64, "K9G8G08X0A"},
	//MAX 100 BAD BLOCKs
	{0xECD59429, 4096, 4096, 218, 0x80000, 5, 127,   0, 0, WIDTH_8, 1, 0, 1, NAND_TYPE_MLC, 12, 0x140A0F64, "K9GAG08UXD"},
	{0xECF10095, 1024, 2048,  64, 0x20000, 4,   0,   1, 0, WIDTH_8, 4, 0, 1, NAND_TYPE_SLC,  4, 0x140a1464, "K9F1G08U0B"},
	{0xEC75A5BD, 2048,  512,  16,  0x4000, 4,   0,   1, 5, WIDTH_8, 1, 1, 0, NAND_TYPE_SLC,  1, 0x230F1964, "K95608U0D"},
	{0xECD514B6, 4096, 4096, 128, 0x80000, 5, 127,   0, 0, WIDTH_8, 1, 0, 1, NAND_TYPE_MLC,  4, 0x140A0C64, "K9GAG08U0M"},
	{0xECD755B6, 8192, 4096, 128, 0x80000, 5, 127,   0, 0, WIDTH_8, 1, 0, 1, NAND_TYPE_MLC,  4, 0x140A0C64, "K9LBG08U0M"},
	{0xECD58472, 2048, 8192, 218/*436*/,0x100000, 5,   0, 127, 0, WIDTH_8, 1, 0, 1, NAND_TYPE_MLC, 12/*24*/, 0x190A0FFF, "K9GAG08U0E"},
	{0xECD7947A, 4096, 8192, 218/*448*/,0x100000, 5,   0, 127, 0, WIDTH_8, 1, 0, 1, NAND_TYPE_MLC, 12/*24*/, 0x190A0FFF, "K9GBG08U0A"},
	{0xECD59476, 2048, 8192, 218/*448*/,0x100000, 5,   0, 127, 0, WIDTH_8, 1, 0, 1, NAND_TYPE_MLC, 12/*24*/, 0x140A0CFF, "K9GAG08U0F"},
	
	{0x98D594BA, 4096, 4096, 218, 0x80000, 5,   0, 127, 0, WIDTH_8, 1, 0, 1, NAND_TYPE_MLC, 12, 0x190F0F70, "TC58NVG4D1DTG0"},
	{0x98D19015, 1024, 2048,  64, 0x20000, 4,   0,   1, 0, WIDTH_8, 4, 0, 1, NAND_TYPE_SLC,  4, 0x140A0C11, "TC58NVG0S3ETA00"},
	{0x98D59432, 2048, 8192, 218/*448*/,0x100000, 5,   0, 127, 0, WIDTH_8, 1, 0, 1, NAND_TYPE_MLC, 12/*24*/, 0x140A0C84, "TC58NVG4D2FTA00"},
	{0xADD59425, 4096, 4096, 218, 0x80000, 5, 125, 127, 0, WIDTH_8, 1, 0, 1, NAND_TYPE_MLC, 12, 0x140A0F64, "HY27UAG8T2A"},
	{0,}
	/*add new product item here.*/
};
/* Nand Product Table          We take HYNIX_NF_HY27UF081G2A for example.
struct NFC_info{
 DWORD dwFlashID;            //composed by 4 bytes of ID. For example:0xADF1801D
 DWORD dwBlockCount;      //block count of one chip. For example: 1024
 DWORD dwPageSize;       //page size. For example:2048(other value can be 512 or 4096)
 DWORD dwSpareSize;       //spare area size. For example:16(almost all kinds of nand is 16)
 DWORD dwBlockSize;       //block size = dwPageSize * PageCntPerBlock. For example:131072
 DWORD dwAddressCycle;      //address cycle 4 or 5
 DWORD dwBI0Position;      //BI0 page postion in block
 DWORD dwBI1Position;      //BI1 page postion in block
 DWORD dwBIOffset;       //BI offset in page 0 or 2048
 DWORD dwDataWidth;      //data with X8 = 0 or X16 = 1
 DWORD dwPageProgramLimit;     //chip can program PAGE_PROGRAM_LIMIT times within the same page
 DWORD dwSeqRowReadSupport;    //whether support sequential row read, 1 = support, 0 = not support
 DWORD dwSeqPageProgram;     //chip need sequential page program in a block. 1 = need
 DWORD dwNandType;       //MLC = 1 or SLC = 0
 DWORD dwECCBitNum;      //ECC bit number needed
 DWORD dwRWTimming;     //NFC Read/Write Pulse width and Read/Write hold time. default =0x12121010
 char cProductName[MAX_PRODUCT_NAME_LENGTH]; //product name. for example "HYNIX_NF_HY27UF081G2A"
};
*/

#endif /* __LINUX_MTD_NAND_IDS_H */
