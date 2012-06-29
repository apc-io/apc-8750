/*
 * Copyright (c) 2008 WonderMedia Technologies, Inc. All Rights Reserved.
 *
 * This PROPRIETARY SOFTWARE is the property of WonderMedia Technologies, Inc.
 * and may contain trade secrets and/or other confidential information of
 * WonderMedia Technologies, Inc. This file shall not be disclosed to any third
 * party, in whole or in part, without prior written consent of WonderMedia.
 *
 * THIS PROPRIETARY SOFTWARE AND ANY RELATED DOCUMENTATION ARE PROVIDED AS IS,
 * WITH ALL FAULTS, AND WITHOUT WARRANTY OF ANY KIND EITHER EXPRESS OR IMPLIED,
 * AND WonderMedia TECHNOLOGIES, INC. DISCLAIMS ALL EXPRESS OR IMPLIED WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR
 * NON-INFRINGEMENT.
 */

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <asm/io.h>
#include <watchdog.h>
#include <nand.h>
/* #include <env.h> */
#include <image.h>
#include <linux/stddef.h>
#include <asm/byteorder.h>

#if 0 /* (CONFIG_COMMANDS & CFG_CMD_NANDRW) */

/* Product table.*/
struct NAND_PRODUCT_INFO  g_NAND_PRODUCT_TABLE[] =
{
/*
	{0xEC75A5BD, 2048, 512, 16, 16384, 4, 0, 1, 5, WIDTH_8,
	2, 1, 0, NAND_TYPE_SLC, "SAMSUNG_NF_K9F5608U0D"},
	{0xEC35A5BD, 2048, 512, 16, 16384, 4, 0, 1, 5, WIDTH_8,
   	2, 1, 0, NAND_TYPE_SLC, "SAMSUNG_NF_K9F5608U0D"},
	{0xEC76A5C0, 4096, 512, 16, 16384, 4, 0, 1, 5, WIDTH_8,
	1, 1, 0, NAND_TYPE_SLC, "SAMSUNG_NF_K9F1208U0B"},
	{0xEC36A5C0, 4096, 512, 16, 16384, 4, 0, 1, 5, WIDTH_8,
	1, 1, 0, NAND_TYPE_SLC, "SAMSUNG_NF_K9F1208U0B"},
	{0xECF10095, 1024, 2048, 64, 131072, 5, 0, 1, 0, WIDTH_8,
	4, 0, 1, NAND_TYPE_SLC, "SAMSUNG_NF_K9F1G08U0B"},
	{0xECA10095, 1024, 2048, 64, 131072, 5, 0, 1, 0, WIDTH_8,
	4, 0, 1, NAND_TYPE_SLC, "SAMSUNG_NF_K9F1G08U0B"},
	{0xECAA1095, 2048, 2048, 64, 131072, 5, 0, 1, 0, WIDTH_8,
	4, 0, 1, NAND_TYPE_SLC, "SAMSUNG_NF_K9F2G08UXA"},
	0xECDA1095, 2048, 2048, 64, 131072, 5, 0, 1, 0, WIDTH_8,
	4, 0, 1, NAND_TYPE_SLC, "SAMSUNG_NF_K9F2G08UXA"},
	0xECDC1095, 4096, 2048, 64, 131072, 5, 0, 1, 0, WIDTH_8,
	4, 0, 1, NAND_TYPE_SLC, "SAMSUNG_NF_K9F4G08U0A"},
	{0xECAC1095, 4096, 2048, 64, 131072, 5, 0, 1, 0, WIDTH_8,
	4, 0, 1, NAND_TYPE_SLC, "SAMSUNG_NF_K9F4G08U0A"},
	0xECD310A6, 4096, 4096, 128, 262144, 5, 0, 1, 0, WIDTH_8,
	4, 0, 1, NAND_TYPE_SLC, "SAMSUNG_NF_K9F8G08U0M"},
	{0x20DC8095, 4096, 2048, 64, 131072, 5, 0 , 1, 0, WIDTH_8,
	4, 0, 1, NAND_TYPE_SLC, "ST_NF_NAND04GW3B2B"},
	{0x20D38195, 8192, 2048, 64, 131072, 5, 0 , 1, 0, WIDTH_8,
	4, 0, 1, NAND_TYPE_SLC, "ST_NF_NAND08GW3B2A"},
	{0xADF1801D, 1024, 2048, 64, 131072, 4, 0, 1, 0, WIDTH_8,
	4, 0, 1, NAND_TYPE_SLC, "HYNIX_NF_HY27UF081G2A"},
	{0xADDC8095, 4096, 2048, 64, 131072, 5, 0, 1, 0, WIDTH_8,
	4, 0, 1, NAND_TYPE_SLC, "HYNIX_NF_HY27UF084G2M"},
	{0x20D314A5, 4096, 2048, 64, 262144, 5, 126, 127, 0, WIDTH_8,
	1, 0, 1, NAND_TYPE_MLC, "ST_NF_NAND08GW3C2A"},
	{0xADD314A5, 4096, 2048, 64, 262144, 5, 126, 127, 0, WIDTH_8,
	1, 0, 1, NAND_TYPE_MLC, "HYNIX_NF_HY27UT088G2M"},
	{0x98D585A5, 8192, 2048, 64, 262144, 5, 0, 127, 0, WIDTH_8,
	1, 0, 1, NAND_TYPE_MLC, "TOSHIBA_NF_TH58NVG4D4C"},
*/
	{0xADF1801D, 1024, 2048, 64, 131072, 4, 0, 1, 0, WIDTH_8,
	4, 0, 1, NAND_TYPE_SLC, 0x140A0F64, "HYNIX_HY27UF081G2A"},
	{0xADDC8095, 4096, 2048, 64, 131072, 5, 0, 1, 0, WIDTH_8,
	4, 0, 1, NAND_TYPE_SLC, 0x190A0F64, "HYNIX_HY27UF084G2M"},
	{0xADD314A5, 4096, 2048, 64, 0x40000, 5, 125, 127, 0, WIDTH_8,
	1, 0, 1, NAND_TYPE_MLC, 0x190A0F64 /* 0x140A0C46 */, "HYNIX_HY27UT088G2M"},
	{0xECF10095, 1024, 2048, 64, 131072, 5, 0, 1, 0, WIDTH_8,
	4, 0, 1, NAND_TYPE_SLC, 0x140A0C64 /* 0x140a1464 */, "SAMSUNG_K9F1G08U0B"},
	{0xECDAC115, 2048, 2048, 64, 131072, 5, 0, 1, 0, WIDTH_8,
	4, 0, 1, NAND_TYPE_SLC, 0x190F1964, "SAMSUNG_K9K2G08U0M"},	//tADL is not define in spec
	{0xECD314A5, 4096, 2048, 64, 262144, 5, 127, 127, 0, WIDTH_8,
	1, 0, 1, NAND_TYPE_MLC, 0x140A0F64 /* 0x140A0C64 */, "SAMSUNG_K9G8G08U0A"}, // tADL may too small
	{0xECDC1425, 2048, 2048, 64, 262144, 5, 127, 127, 0, WIDTH_8,
	1, 0, 1, NAND_TYPE_MLC, 0x140A0F64, "SAMSUNG_K9G4G08U0A"},
	{0xECD59429, 4096, 4096, 218, 0x80000, 5, 127, 127, 0, WIDTH_8,
	1, 0, 1, NAND_TYPE_MLC, 0x140A0F64, "SAMSUNG_K9GAG08U0M"},
	{0x98D594BA, 4096, 4096, 218, 0x80000, 5, 0, 127, 0, WIDTH_8,
	1, 0, 1, NAND_TYPE_MLC, 0x190F0F70, "TC58NVG4D1DTG0"}
};

#define WRITE_NAND16(d, addr) (*(volatile unsigned short *)(addr) = (unsigned short)d)
#define RAED_NAND16(d, addr) (d = *(volatile unsigned short *)(addr))
#define RAED_NAND8(d, addr) (d = *(volatile unsigned char *)(addr))
#define WRITE_NAND8(d, addr) (*(volatile unsigned char *)(addr) = (unsigned char)d)

#define USE_BBT
#define BBT_MAX_BLOCK 4
#define BBT_BITMAP 2
#define MAX_NAND_CHIP 2

WMT_NFC_CFG *pNFCRegs;
struct _NAND_PDMA_REG_ *pNand_PDma_Reg;
unsigned long *ReadDesc, *WriteDesc;
static unsigned int bbt_version;
unsigned int g_WMTNFCBASE;
WMTNF_INFO_t g_nfinfo[MAX_NAND_CHIP];
unsigned int *g_bbt;
unsigned int __NFC_BUF;

struct current_nand_info_t {
	unsigned int chip;
	unsigned int ecc;
	unsigned int eccbits;
	unsigned int maxchip;
};
struct current_nand_info_t g_ncurrent;
unsigned int g_nboundary;

#ifdef POST_TEST_FUNS
struct post_ecc_info {
	int pos[MAX_ECC_BIT];
	int pred[MAX_ECC_BIT];
	unsigned int cnt;
	unsigned int eccbytes;
};

struct post_ecc_info g_ecc_result;
unsigned int g_ecc_dcnt, g_ecc_rcnt;
#endif

int (*nfc_read_page)(unsigned int start,unsigned int maddr, WMTNF_INFO_t *info);

#define PMPMB_ADDR 0xd8130204
#define PMNAND_ADDR 0xd8130330
#define PMCS_ADDR 0xd8130000
#define WMT_PLL_CLOCK 25
#define NFC_MAX_CLOCK 50
#define NFC_SETUPTIME 15

#define SHOWME_ECC
/*#define NAND_NOBOUNDLY*/
#define NFC_TIMEING_CAL

/* ------------------------------------------------------------------------- */

int do_nandrw(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	unsigned int naddr, maddr, size;

	switch (argc) {
	case 0:
	case 1:
		printf("Usage:\n%s\n", cmdtp->usage);
		return 0;
	default:
		if (strncmp(argv[1], "w", 5) == 0) {
			maddr = simple_strtoul(argv[2], NULL, 16);
			naddr = simple_strtoul(argv[3], NULL, 16);
			size = simple_strtoul(argv[4], NULL, 16);
			if (!size) {
				printf("size=0\n");
				return 0;
			}
			WMTSaveImageToNAND(naddr, maddr, size);
			return 0;
		}

		if (strncmp(argv[1], "r", 5) == 0) {
			maddr = simple_strtoul(argv[3], NULL, 16);
			naddr = simple_strtoul(argv[2], NULL, 16);
			size = simple_strtoul(argv[4], NULL, 16);
			if (!size) {
				printf("size=0\n");
				return 0;
			}
			WMTLoadImageFormNAND(naddr, maddr, size);

			return 0;
		}
		if (strncmp(argv[1], "erase", 4) == 0 || strncmp(argv[1], "erase", 3) == 0) {
			if (strcmp(argv[2], "all") == 0) {
				printf("Erase all\n");
				WMTEraseNAND(0, 0, 1);
				return 0;
			} else if (strcmp(argv[2], "force") == 0) {
				printf("Erase all\n");
				WMTEraseNANDALL(1);

				return 0;
			} else if (strcmp(argv[2], "table") == 0) {
				printf("Erase Table\n");
				WMTEraseNANDALL(0);

				return 0;
			} else {
				maddr = simple_strtoul(argv[2], NULL, 16);
				naddr = simple_strtoul(argv[3], NULL, 16);
				WMTEraseNAND(maddr, naddr, 0);
			}
			return 0;
		}

		if (strncmp(argv[1], "tellme", 4) == 0) {
			naddr = simple_strtoul(argv[3], NULL, 16);
			if (strcmp(argv[2], "bad") == 0)
				tellme_badblock(naddr);
			if (strcmp(argv[2], "table") == 0)
				tellme_whereistable(naddr);
			if (strcmp(argv[2], "nand") == 0)
				tellme_nandinfo();
			return 0;
		}

		if (strncmp(argv[1], "bad", 3) == 0) {
			naddr = simple_strtoul(argv[2], NULL, 16);
			set_blockisbad(naddr);
			return 0;
		}
		if (strncmp(argv[1], "good", 3) == 0) {
			naddr = simple_strtoul(argv[2], NULL, 16);
			set_blockisgood(naddr);
			return 0;
		}
#ifdef POST_TEST_FUNS
		if(strncmp(argv[1], "test", 4) == 0) {
			if(strcmp(argv[2],"rw") == 0){
				naddr = simple_strtoul(argv[3], NULL, 16);
				test_nand_rw(naddr);
			}
			if(strcmp(argv[2],"ecc") == 0){
				naddr = simple_strtoul(argv[3], NULL, 16);
				test_nand_ecc(naddr);
			}
			if(strcmp(argv[2],"table") == 0){
				naddr = simple_strtoul(argv[3], NULL, 16);
				test_nand_table(naddr);
			}
			return 0;
		}
#endif
		printf("Usage:\n%s\n", cmdtp->usage);
		return 0;
	}
	return 0;
}

U_BOOT_CMD(
	nandrw,	5,	1,	do_nandrw,
	"nand    - NAND sub-system\n",
	"info  - show available NAND devices\n"
	"nand device [dev] - show or set current device\n"
	"nand read[.jffs2[s]]  addr off size\n"
	"nand write[.jffs2] addr off size - read/write `size' bytes starting\n"
	"    at offset `off' to/from memory address `addr'\n"
	"nand erase [clean] [off size] - erase `size' bytes from\n"
	"    offset `off' (entire device if not specified)\n"
	"nand bad - show bad blocks\n"
	"nand read.oob addr off size - read out-of-band data\n"
	"nand write.oob addr off size - read out-of-band data\n"
);

int NFC_WAIT_READY(void)
{
	int i = 0;

	while (1) {
		if (!(pNFCRegs->NFCRa & NFC_BUSY))
			break;
		if (i>>21)
			return -3;
		i++;
	}
	return 0;
}

int NAND_WAIT_READY(void)
{
	int i = 0;

	while (1) {
		if (pNFCRegs->NFCRb & B2R)
			break;
		if ((i>>21))
			return -1;
		i++;
	}
	pNFCRegs->NFCRb |= B2R;
	return 0;
}

int NFC_WAIT_CMD_READY(void)
{
	int i = 0;

	while (1) {
		if (!(pNFCRegs->NFCRa & NFC_CMD_RDY))
			break;
		if (i>>21)
			return -4;
		i++;
	}
	return 0;
}

int NAND_WAIT_IDLE(void)
{
	int i = 0;

	while (1) {
		if (pNFCRegs->NFCR1d & NFC_IDLE)
			break;
		if (i>>21)
			return -2;
		i++;
	}
	return 0;
}

int nfc_1bit_ecc_correct(unsigned int start, unsigned int maddr, WMTNF_INFO_t *info)
{
	unsigned int err = 0, bank = 0, i;
	unsigned int cp = 0, lp = 0;
	unsigned char data = 0;

	/* correct data area , first */
	err = pNFCRegs->NFCR20;
	if (err & 0xffffffff) {
		printf("data error err = %x\n", err);

		/* find which bank is err */
		for (i = 0; i < 32; i += 4) {
			if ((err>>i) & 0x0f) {
				cp = (err>>i) & 0x0f;
				bank = i>>2;
				lp++;
			}
		}
		if (lp != 1)
			return -ERR_ECC_BANK;
		if (cp & 0x0d)
			return -ERR_ECC_UNCORRECT;
		/* reset bank select */
		err = pNFCRegs->NFCR15;
		err &= 0xfffffffc;
		err |= ((bank+1)>>1);
		pNFCRegs->NFCR15 = err;
		/* correct data , now */
		if ((bank+1)%2) {
			lp = (pNFCRegs->NFCR1a & 0x1ff);
			cp = (pNFCRegs->NFCR1a>>9) & 0x07;
		} else {
			lp = (pNFCRegs->NFCR1b & 0x1ff);
			cp = (pNFCRegs->NFCR1b>>9) & 0x07;
		}
		bank = bank - 1;
		data = *(unsigned char *)(maddr+(bank*256)+lp);
		if (data & (1<<cp))
			data &= ~(1<<cp);
		else
			data |= (1<<cp);
		*(unsigned char *)(maddr+(bank*256)+lp) = data;
	}
	/* need to correct spare area ?	 */
	err = pNFCRegs->NFCR1f;
	if (err & 0x07) {
		printf(" red error \n");
		lp = (pNFCRegs->NFCR1c & 0x3f);
		cp = (pNFCRegs->NFCR1c>>8) & 0x07;
		data = pNFCRegs->FIFO[lp];
		if (data & (1<<cp))
			data &= ~(1<<cp);
		else
			data |= (1<<cp);
		pNFCRegs->FIFO[lp] = data;
	}
	return 0;
}

int nfc_1bit_read_page(
	unsigned int start,
	unsigned int maddr,
	WMTNF_INFO_t *info
)
{
	int rc = -1;

	/* printf("nfc_1bit_read_page : addr = 0x%x (len = 0x%x) \n",start,len); */
	rc = nand_read_page(start, maddr, info);
	if (rc < 0)
		return rc;
	if (pNFCRegs->NFCR1f & 0x07 || pNFCRegs->NFCR20 & 0xffffffff) {
		printf("ECC error \n");
		rc = nfc_1bit_ecc_correct(start, maddr, info);
		if (rc)
			return rc;
	}

	return 0;
}

#define ecc4bit_inetrrupt_enable 0x101
#define ecc4bit_inetrrupt_disable 0x0

void nfc_ecc_set(unsigned int type, WMTNF_INFO_t *nfinfo)
{
	
	if ((type&0xFF) == USE_HW_ECC) {
		g_ncurrent.ecc = type;
		/*printf("USE_HW_ECC ");*/
		type = type>>8;
		pNFCRegs->NFCR23 &= ENABLE_BCH_ECC;
    	pNFCRegs->NFCR23 |= type;
		pNFCRegs->NFCR15 &= (~(unsigned int)USE_SW_ECC);
		if (type != ECC1bit) {
			nfc_read_page = nfc_bch_read_page;
			pNFCRegs->NFCR24 = ecc4bit_inetrrupt_enable;
			pNFCRegs->NFCR12 &= (~OLDATA_EN);
			pNFCRegs->NFCR23 |= READ_RESUME; /* do for safty	 */
		} else {
			/* disable 4bit ecc interrupt and old structure  */
			/*printf("ECC1bit\n");*/
			nfc_read_page = nfc_1bit_read_page;
			/* make sure our status is clear before we start */
			pNFCRegs->NFCR1f |= 0x07;
			pNFCRegs->NFCR20 |= 0xffffffff;
			/*pNFCRegs->NFCR24 = ecc4bit_inetrrupt_disable;*/
			pNFCRegs->NFCR12 |= OLDATA_EN;
			pNFCRegs->NFCR23 |= READ_RESUME;/* do for safty */
		}
	} else {
		/*printf("USE_SW_ECC\n");*/
		pNFCRegs->NFCR24 = ecc4bit_inetrrupt_disable;
		g_ncurrent.ecc = USE_SW_ECC;
		nfc_read_page = nand_read_page;
		pNFCRegs->NFCR15 |= USE_SW_ECC;
		pNFCRegs->NFCR12 &= (~OLDATA_EN);
		//pNFCRegs->NFCR23 |= DIS_BCH_ECC;
	}
}

int set_nfc_ce(int ce)
{
	if (pNFCRegs == NULL)
		return -1;

	if (ce == 0)
		pNFCRegs->NFCR11 = 0xfe;
	else if (ce == 1)
		pNFCRegs->NFCR11 = 0xfd;
	else
		return -1;

	return 0;
}

int wmt_select_nand(unsigned int chip)
{
	unsigned int naddr, tmp;

	if (g_ncurrent.chip == chip)
		return 0;

	if (chip >= g_ncurrent.maxchip) {
		printf("MAX NAND FLASH = %d (input = %d)\n", g_ncurrent.maxchip, chip);
		return -1;
	}
	if (g_nfinfo[chip].id == FLASH_UNKNOW) {
		printf("NAND FLASH NOT FOUND ....\n");
		return -1;
	}

	if (g_nfinfo[chip].wmt_nfc.nfc12 != pNFCRegs->NFCR12)
		pNFCRegs->NFCR12 = g_nfinfo[chip].wmt_nfc.nfc12;

	if (g_nfinfo[chip].wmt_nfc.nfc14 != pNFCRegs->NFCR14)
		pNFCRegs->NFCR14 = g_nfinfo[chip].wmt_nfc.nfc14;

	if (g_nfinfo[chip].wmt_nfc.nfc17 != pNFCRegs->NFCR17)
		pNFCRegs->NFCR12 = g_nfinfo[chip].wmt_nfc.nfc17;

	tmp = *(volatile unsigned int *)PMNAND_ADDR;
	if ((tmp&0x1FF) != g_nfinfo[chip].wmt_nfc.divisor) {
		if (set_nand_div(g_nfinfo[chip].wmt_nfc.divisor))
			return -1;
	}

	if (g_ncurrent.ecc != g_nfinfo[chip].ecc)
		nfc_ecc_set(g_nfinfo[chip].ecc, &g_nfinfo[chip]);
	
	g_ncurrent.chip = chip;

	if (set_nfc_ce((int)g_nfinfo[chip].ce))
		return -1;

	naddr = g_nfinfo[chip].whereisbbt[0];
	tmp = g_nfinfo[chip].totblk>>4;
	if (tmp < g_nfinfo[chip].oobblock)
		tmp = g_nfinfo[chip].oobblock;

	return nand_read_block((unsigned int)&g_bbt[0], (naddr<<g_nfinfo[chip].erasesize), tmp, &g_nfinfo[chip]);
}

int set_nand_div(unsigned int div)
{
	unsigned int tmp = 0xFFFFFFFF, cnt = 0;

	*(volatile unsigned int *)PMNAND_ADDR = div;
	while(1) {
		tmp = *(volatile unsigned int *)PMCS_ADDR;
		if ((tmp&(1<<25)) == 0)
			break;
		cnt++;
		if (cnt&(1<<25)) {
			printf("Setting PMC Timeout ....\n");
			return -1;
		}
	}
	return 0;
}

int wmt_calc_clock(WMTNF_INFO_t *info, unsigned int spec_clk, int *T, int *divisor, int *Thold)
{
	unsigned int i, div1=0, div2, clk1, clk2=0, comp, T1=0, T2=0, clk, pllb;
	unsigned int tREA, tREH, Thold2, Ttmp, tADL, tWP;
	
	pllb = *(volatile unsigned char *)PMPMB_ADDR;
	printf("pllb=0x%x, spec_clk=0x%x\n", pllb, spec_clk);
	pllb &= 0x1F;
	if (pllb < 4)
		pllb = 1;
	tREA = (spec_clk>>24)&0xFF;
	tREH = (spec_clk>>16)&0xFF;
	tWP  = (spec_clk>>8)&0xFF;
	tADL = spec_clk&0xFF;
	for (i = 1; i < 16; i++) {
		if (NFC_MAX_CLOCK >= ((pllb*WMT_PLL_CLOCK)/i)) {
			div1 = i;
			break;
		}
	}

	clk1 = (1000 * div1)/(pllb*WMT_PLL_CLOCK);
	/*printf("clk1=%d, div1=%d, spec_clk=%d\n", clk1, div1, spec_clk);*/
	for (T1 = 1; T1 < 10; T1++) {
		if ((T1*clk1) >= (tREA + NFC_SETUPTIME)) /* 8.64 = tSKEW 5.64 + tDLY 2.42 + tSETUP 0.4 */
			break;
	}
	i = 1;
	while (i*clk1 <= tREH) {
		i++;
	}
	*Thold = i;
	printf("T1=%d, clk1=%d, div1=%d, Thold=%d, tREA=%d+delay(9)\n", T1, clk1, div1, *Thold, tREA);
	Ttmp = *T = T1;
	clk = clk1;
	*divisor = div1;
	div2 = div1;
	while (Ttmp > 1 && clk != 0) {
		div2++;
		clk2 = (1000 * div2)/(pllb*WMT_PLL_CLOCK);
		comp = 0;
		for (T2 = 1; T2 < Ttmp; T2++) {
			if ((T2*clk2) >= (tREA + NFC_SETUPTIME)) { /* 8.64 = tSKEW 5.64 + tDLY 2.42 + tSETUP 0.4 */
				Ttmp = T2;
				comp = 1;
				i = 1;
				while (i*clk2 <= tREH) {
					i++;
				}
				Thold2 = i;
				printf("T2=%d, clk2=%d, div2=%d, Thold2=%d, comp=1\n", T2, clk2, div2, Thold2);
				break;
			}
		}
		if (comp == 1) {
			clk1 = clk * (*T+*Thold) * info->oobblock;
			div1 = clk2 * (T2+Thold2) * info->oobblock;
			printf("Tim1=%d , Tim2=%d\n", clk1, div1);
			if ((clk * (*T+*Thold) * info->oobblock) > (clk2 * (T2+Thold2) * info->oobblock)) {
				*T = T2;
				clk = clk2;
				*divisor = div2;
				*Thold = Thold2;
			} else {
				printf("T2 is greater and not use\n");
			}
		}
	} /* end of while */
	//printf("Tadfasdfil\n");
	i = 1;
	*Thold |= 0x100; /* set write setup/hold time */
	while (((i*2+2)*clk) <= (tADL-tWP) || (i*clk) <= (tWP+1)) {/*+1*/
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
	
	printf("T=%d, clk=%d, divisor=%d, Thold=0x%x\n", *T, clk, *divisor, *Thold);
	if ((NFC_MAX_CLOCK < (pllb*WMT_PLL_CLOCK)/(*divisor)) || clk == 0 || *T == 0 || clk > 45)
		return 1;

	return 0;
}

int wmt_get_timing(WMTNF_INFO_t *info, unsigned int nand_timing)
{
	unsigned int T, Thold, divisor;	

	info->wmt_nfc.nfc14 = pNFCRegs->NFCR14;
	info->wmt_nfc.divisor = *(volatile unsigned int *)PMNAND_ADDR;
#ifdef NFC_TIMEING_CAL
	if (wmt_calc_clock(info, nand_timing, &T, &divisor, &Thold)) {
		printf("timming setting fail");
		return 1;
	}
	info->wmt_nfc.nfc14 = ((Thold&0xFF) << 12) + ((T + (Thold&0xFF)) << 8) +
		(((Thold>>8)&0xFF) << 4) + 2*((Thold>>8)&0xFF);	
	info->wmt_nfc.divisor = (divisor&0x1FF);
	printf("nfc 0x%x , divisor %x\r\n",info->wmt_nfc.nfc14,info->wmt_nfc.divisor);
#endif
	return 0;
}


int get_nand_info(int ce, WMTNF_INFO_t *info)
{
	unsigned int i, j, tmp, size;
	unsigned char id[5];
	char *buf;

	set_nfc_ce(ce);
	pNFCRegs->NFCR12 = WIDTH_8 | WP_DISABLE | DIRECT_MAP;
	pNFCRegs->NFCR14 &= 0xffff0000;/*prepare to set r/w cycle*/
	pNFCRegs->NFCR14 |= 0x2424; /* set r/w cycle  */

	if (nand_readID(id))
		return -1;

	tmp = (unsigned int)id[0];
	for (i = 1; i < 4; i++) {
		if (id[0] == id[i])
			break;
		tmp = (tmp<<8);
		tmp |= (unsigned int)id[i];
	}
	
	size = sizeof(g_NAND_PRODUCT_TABLE)/sizeof(struct NAND_PRODUCT_INFO);
	for (i = 0; i < size; i++) {
		if (g_NAND_PRODUCT_TABLE[i].dwFlashID == tmp)
			break;
	}
	if (i == size) {
		printf("Un-know id = 0x%x\n",tmp);
		return -1;
	}
	info->id = i;
	info->blksize = g_NAND_PRODUCT_TABLE[i].dwBlockSize;
	info->ce = ce;
	tmp = g_NAND_PRODUCT_TABLE[i].dwBlockSize/g_NAND_PRODUCT_TABLE[i].dwPageSize;
	tmp = (tmp/16);
	for (j = 0; j < 32; j++) {
		if (tmp&(1<<j))
			break;
	}
	if (j > 5)
		return -1;
	pNFCRegs->NFCR17 &= 0xffffff1f;
	pNFCRegs->NFCR17 |= (j<<5);
	info->wmt_nfc.nfc17 = pNFCRegs->NFCR17;

	info->erasesize = (j+4);
	info->oobblock = g_NAND_PRODUCT_TABLE[i].dwPageSize;
	info->oobsize = g_NAND_PRODUCT_TABLE[i].dwSpareSize;
	
	tmp = g_NAND_PRODUCT_TABLE[i].dwDataWidth | WP_DISABLE | DIRECT_MAP | SW_DIS_ECC;
	switch (info->oobblock) {
	case 512:
		tmp = (tmp|PAGE_512);
		info->page_shift = 8;
		info->col = 1;
		break;
	case 2048:
		tmp = (tmp|PAGE_2K);
		info->page_shift = 11;
		info->col = 2;
		break;
	case 4096:
		tmp = (tmp|PAGE_4K);
		info->page_shift = 12;
		info->col = 2;
		break;
	default:
		return -1;
	}

	pNFCRegs->NFCR12 = tmp;
	info->wmt_nfc.nfc12 = tmp;

	info->erasesize += info->page_shift;
	info->totblk = g_NAND_PRODUCT_TABLE[i].dwBlockCount;
	info->row = g_NAND_PRODUCT_TABLE[i].dwAddressCycle-info->col;

	info->page[0] = g_NAND_PRODUCT_TABLE[i].dwBI0Position<<info->page_shift;
	info->page[0] += g_NAND_PRODUCT_TABLE[i].dwBIOffset;
	info->page[1] = g_NAND_PRODUCT_TABLE[i].dwBI1Position<<info->page_shift;
	info->page[1] += g_NAND_PRODUCT_TABLE[i].dwBIOffset;

	wmt_get_timing(info, g_NAND_PRODUCT_TABLE[i].dwRWTimming);

	switch(info->oobblock) {
	case 512:
		nfc_ecc_set(USE_HW_1bit_ECC, info);
		info->ecc = USE_HW_1bit_ECC;
		break;
	case 2048:
		nfc_ecc_set(USE_HW_4bit_ECC, info);
		info->ecc = USE_HW_4bit_ECC;
		break;
	case 4096:	
		if (info->oobsize > 204) {
			nfc_ecc_set(USE_HW_12bit_ECC, info);
			info->ecc = USE_HW_12bit_ECC;
		} else if (info->oobsize > 165) {
			nfc_ecc_set(USE_HW_8bit_ECC, info);
			info->ecc = USE_HW_8bit_ECC;
		} else {
			nfc_ecc_set(USE_HW_4bit_ECC, info);
			info->ecc = USE_HW_4bit_ECC;
		}
		break;
	default:
		return -1;
	}

	if (__NFC_BUF == 0) {
		buf = malloc(info->oobblock+info->oobsize);
		__NFC_BUF = (unsigned int)&buf[0];
	}

	return 0;
}

int nfc_init(int find, int scan)
{
	unsigned int chip = 0, i;
	unsigned int naddr = 0, need = 0;
	int rc = -1;
	char *tmp;

	pNFCRegs = (WMT_NFC_CFG *) __NFC_BASE;
	g_WMTNFCBASE = __NFC_BASE;
	pNand_PDma_Reg = (struct _NAND_PDMA_REG_ *) (__NFC_BASE + 0x100);
	if (!ReadDesc)
		ReadDesc = malloc(128);
	if (!ReadDesc)
		printf("nfc_init alloc ReadDesc failed\n");

#ifdef NAND_NOBOUNDLY
	g_nboundary = 0xFFFFFFFF;
#else
	if (g_nboundary == 0) {
		if ((tmp = getenv ("wmt_nfresv")) != NULL) {
			naddr = (unsigned int)simple_strtoul (tmp, NULL, 16);
			g_nboundary = 0;
			i = 1;
			need = 0;
			while(1) {
				chip = naddr&0x0F;
				if (chip > 9) {
					printf("unknow parameter : hex or dec ? , using as default\n");
					g_nboundary = 64;
					break;
				}
				g_nboundary += (i*chip);
				i = i*10;
				naddr = naddr>>4;
				need += 4;
				if (need >= 32)
					break;
			}
			g_nboundary = g_nboundary<<20;
			chip = need = 0;
		} else
			g_nboundary = 64*1024*1024;
	}
#endif

#ifdef WMT_TLOGO
	g_nand_callback.read_callback = NULL;
	g_nand_callback.write_callback = NULL;
	g_nand_callback.erase_callback = NULL;
#endif

	i = 0;
	for (chip = 0; chip < MAX_NAND_CHIP; chip++) {
		g_nfinfo[chip].id = FLASH_UNKNOW;
		if (get_nand_info(chip, &g_nfinfo[i]) == 0) {
			if (find == 0) {
				i++;
				continue;
			}
			rc = find_bbt(&g_nfinfo[i]);
			if (rc < 0) {
				printf("find_bbt failed\n");
				return- 1;
			}
			if (rc > 0) {
				if (scan) {
					if (creat_bbt(&g_nfinfo[i])) {
						printf("find_bbt failed\n");
						return- 1;
					}
					need = 0;
					for (naddr = 0; naddr < (g_nfinfo[i].totblk-BBT_MAX_BLOCK); naddr++) {
						if (isbbtbadblock(&g_nfinfo[i], (naddr<<g_nfinfo[i].erasesize)))
							continue;

						rc = nand_erase(naddr<<g_nfinfo[i].erasesize);
						if (rc < 0 || rc & 0x01) {
							printf("Erase failed at block%d\n", naddr);
							update_bbt2ram(&g_nfinfo[i], (naddr<<g_nfinfo[i].erasesize));
							need = 1;
							continue;
						}
					}
					if (need) {
						if (update_bbt2flash(&g_nfinfo[i])) {
							g_nfinfo[i].id = FLASH_UNKNOW;
							continue;
						}
					}
				} else {
					printf("Chip%d : Table is not find .....\n", i);
					g_nfinfo[i].id = FLASH_UNKNOW;
					continue;
				}
			}
			i++;
		}
	}

	if ((g_nfinfo[0].id == FLASH_UNKNOW) && (g_nfinfo[1].id == FLASH_UNKNOW)) {
		printf("Err : Can not find any nand flash\n");
		return -1;
	}

	g_ncurrent.chip = 0xFFFF;

	g_ncurrent.maxchip = 1;

	//set_nand_clk();
	if (find)
		return wmt_select_nand(0);

	return 0;

}

void NanD_Address(int numbytes, unsigned int col, unsigned int row)
{
	unsigned char addr[10];
	unsigned int i = 0, tmp;
	unsigned int nandptr = 0;

	memset(addr, 0, 10);
	nandptr = g_WMTNFCBASE + 0x0c;
	if (numbytes == ADDR_COLUMN_PAGE) {
		for (i = 0; i < g_nfinfo[0].col; i++)
			addr[i] = (col>>(i*8));
		for (i = g_nfinfo[0].col; i < (g_nfinfo[0].row + g_nfinfo[0].col); i++)
			addr[i] = (row>>((i-g_nfinfo[0].col)*8));
		for (i = 0; i <= (g_nfinfo[0].col+g_nfinfo[0].row); i += 2, nandptr += 4) {
			tmp = (addr[i+1]<<8)+addr[i];
			WRITE_NAND16(tmp, nandptr);
			/* printf("write %x to %8.8x\n",tmp,nandptr); */
		}
	}
	if (numbytes == ADDR_COLUMN) {
			for (i = 0; i < g_nfinfo[0].col; i++)
				addr[i] = (col>>(i*8));
		for (i = 0; i < g_nfinfo[0].col; i += 2, nandptr += 4) {
			tmp = (addr[i+1]<<8)+addr[i];
			WRITE_NAND16(tmp, nandptr);
		}
	}
	if (numbytes == ADDR_PAGE) {
		for (i = 0; i < g_nfinfo[0].row; i++)
				addr[i] = (row>>(i*8));

		for (i = 0; i < g_nfinfo[0].row; i += 2, nandptr += 4) {
			tmp = (addr[i+1]<<8)+addr[i];
			WRITE_NAND16(tmp, nandptr);
		}
	}
}

int nfc_dma_cfg(unsigned int buf, unsigned int len, unsigned int wr)
{
	unsigned int status;

	if (!len || buf & 0x03) {
		printf("Error : length = %x , address = %x\r\n", len, buf);
		return -1;
	}

	if (!ReadDesc) {
		printf("alloc ReadDesc failed\n");
		return -1;
	}
	/*WriteDesc = ReadDesc + 0x800;*/
	WriteDesc = ReadDesc;
	pNFCRegs->NFCR8 = len-1;

	if (pNand_PDma_Reg->DMA_ISR & NAND_PDMA_IER_INT_STS)
		pNand_PDma_Reg->DMA_ISR = NAND_PDMA_IER_INT_STS;

	if (pNand_PDma_Reg->DMA_ISR & NAND_PDMA_IER_INT_STS) {
		printf("PDMA interrupt status can't be clear ");
		printf("pNand_PDma_Reg->DMA_ISR = 0x%8.8x \n", (unsigned int)pNand_PDma_Reg->DMA_ISR);
	}

	status = nand_init_pdma();
	if (status)
		printf("nand_init_pdma fail status = 0x%x", status);
	nand_alloc_desc_pool((wr) ? WriteDesc : ReadDesc);
	nand_init_long_desc((wr) ? WriteDesc : ReadDesc, len, (unsigned long *)buf, 0, 1);
	nand_config_pdma((wr) ? WriteDesc : ReadDesc, wr);
	return 0;
}

int nand_init_pdma(void)
{
	pNand_PDma_Reg->DMA_GCR = NAND_PDMA_GCR_SOFTRESET;
	pNand_PDma_Reg->DMA_GCR = NAND_PDMA_GCR_DMA_EN;
	if (pNand_PDma_Reg->DMA_GCR & NAND_PDMA_GCR_DMA_EN)
		return 0;
	else
		return 1;
}


int nand_free_pdma(void)
{
	pNand_PDma_Reg->DMA_DESPR	= 0;
	pNand_PDma_Reg->DMA_GCR = 0;
	return 0;
}


int nand_alloc_desc_pool(unsigned long *DescAddr)
{
	memset(DescAddr, 0, 128);
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

int nand_config_pdma(unsigned long *DescAddr, unsigned int dir)
{
	/*pNand_PDma_Reg->DMA_IER = NAND_PDMA_IER_INT_EN;*/
	pNand_PDma_Reg->DMA_DESPR = (unsigned long)DescAddr;
	if (dir == NAND_PDMA_READ)
		pNand_PDma_Reg->DMA_CCR |= NAND_PDMA_CCR_peripheral_to_IF;
	else
		pNand_PDma_Reg->DMA_CCR &= ~NAND_PDMA_CCR_IF_to_peripheral;

	pNand_PDma_Reg->DMA_CCR |= NAND_PDMA_CCR_RUN;

	return 0;	
}

int nand_pdma_handler(void)
{
	unsigned long status = 0;
	unsigned long count = 0;

	count = 0x100000;
	/*	 polling CSR TC status	*/
	do {
		count--;
		if (pNand_PDma_Reg->DMA_ISR & NAND_PDMA_IER_INT_STS) {
			status = pNand_PDma_Reg->DMA_CCR & NAND_PDMA_CCR_EvtCode;
			pNand_PDma_Reg->DMA_ISR &= NAND_PDMA_IER_INT_STS;
			break ;
		}
		if (count == 0) {
			printf("PDMA Time Out!\n");
			printf("pNand_PDma_Reg->DMA_CCR = 0x%8.8x\r\n",
			(unsigned int)pNand_PDma_Reg->DMA_CCR);

			break;
		}
	} while (1);
	if (status == NAND_PDMA_CCR_Evt_ff_underrun)
		printf("PDMA Buffer under run!\n");

	if (status == NAND_PDMA_CCR_Evt_ff_overrun)
		printf("PDMA Buffer over run!\n");

	if (status == NAND_PDMA_CCR_Evt_desp_read)
		printf("PDMA read Descriptor error!\n");

	if (status == NAND_PDMA_CCR_Evt_data_rw)
		printf("PDMA read/write memory descriptor error!\n");

	if (status == NAND_PDMA_CCR_Evt_early_end)
		printf("PDMA read early end!\n");

	if (count == 0) {
		printf("PDMA TimeOut!\n");
		while (1)
			;
	}

	return 0;
}

int NFC_CHECK_ECC(void)
{
	int i = 0;

	while (1) {
		if (!(pNFCRegs->NFCRa & NFC_BUSY))
			break;
		if (pNFCRegs->NFCR25 == (ERR_CORRECT | BCH_ERR))
			return 1;
		if (i>>21)
			return -3;
		i++;
	}
	if (pNFCRegs->NFCR25&BCH_ERR) {
		while(1) {
			if (pNFCRegs->NFCR25&ERR_CORRECT)
				return 1;
		}
	}
	return 0;
}

void nfc_bch_ecc_correct(unsigned int bitcnt, unsigned int maddr)
{
	unsigned int ofs, i = 0, type;
	unsigned int posptr;
	unsigned short err_ofs;
	unsigned int byte_ofs, bit_ofs;
	unsigned char err_data;
	unsigned int ofs1 = 0;

#ifdef SHOWME_ECC
	printf("error happen at 0x%x", maddr);
#endif
	posptr = g_WMTNFCBASE + 0x9c;
	type = pNFCRegs->NFCR26 & BANK_DR;
	ofs = (pNFCRegs->NFCR26 & BANK_NUM)>>8;
	if (type) {
#ifdef SHOWME_ECC
		printf(" ( %d ) red area ",ofs);
#endif
		ofs = g_WMTNFCBASE + 0xc0;
	} else {
#ifdef SHOWME_ECC
		printf(" ( %d ) data area ",ofs);
#endif
		ofs1 = (ofs*512);
		ofs = maddr + (ofs*512);
	}

	for (i = 0; i < bitcnt; i++, posptr+=2) {
		RAED_NAND16(err_ofs, posptr);
		byte_ofs = (unsigned int)(err_ofs>>3);
		bit_ofs = (unsigned int)(err_ofs & 0x07);
#ifdef POST_TEST_FUNS
	if(type) {
		g_ecc_result.pred[g_ecc_rcnt] = (byte_ofs*8)+bit_ofs;
		g_ecc_rcnt++;
		if (g_ecc_rcnt >= MAX_ECC_BIT)
			g_ecc_rcnt = 0;
	} else {
		g_ecc_result.pos[g_ecc_dcnt] = ((ofs1+byte_ofs)*8)+bit_ofs;		
		g_ecc_dcnt++;
		if (g_ecc_dcnt >= MAX_ECC_BIT)
			g_ecc_dcnt = 0;
	}
#endif
#ifdef SHOWME_ECC
		printf(" byte%d , bit%d \n",byte_ofs,bit_ofs);
#endif
		RAED_NAND8(err_data, (ofs+byte_ofs));
#ifdef SHOWME_ECC
		printf("org ( 0x%x ) 0x%x => ",(ofs+byte_ofs),err_data);
#endif
    	if (err_data & (1<<bit_ofs))
    		err_data &= ~(1<<bit_ofs);
    	else
    		err_data |= (1<<bit_ofs);
#ifdef SHOWME_ECC
		printf(" 0x%x\n",err_data);
#endif
		WRITE_NAND8(err_data, (ofs+byte_ofs));
	}
}

int nfc_bch_ecc_check(unsigned int maddr)
{
	unsigned int cnt = 0;
	int rc = 0;
	unsigned int posptr, ofs;
	unsigned short eofs;
	unsigned char edata;

	if (pNFCRegs->NFCR25 == (ERR_CORRECT|BCH_ERR)) {
		cnt = pNFCRegs->NFCR26 & BCH_ERR_CNT;
		if (cnt == BCH_ERR_CNT) {
			printf("Too Many Error\n");
			rc = -ERR_ECC_UNCORRECT;
		} else {
			if (!cnt) {
				printf("report ecc error : count =%d\n", cnt);
				pNFCRegs->NFCR23 |= READ_RESUME;
				rc = -ERR_UNKNOW;
			}
		}
		if (rc) {
			pNFCRegs->NFCR25 = (ERR_CORRECT|BCH_ERR);
			pNFCRegs->NFCR23 |= READ_RESUME;
			return rc;
		}

		nfc_bch_ecc_correct(cnt, maddr);
	}
	pNFCRegs->NFCR25 = (ERR_CORRECT|BCH_ERR);
	pNFCRegs->NFCR23 |= READ_RESUME;

	return 0;
}

/* always read one page */
int nfc_bch_read_page(
	unsigned int start,
	unsigned int maddr,
	WMTNF_INFO_t *info
)
{
	unsigned int row = 0, col = 0;
	int rc = -1, err = 0;

	/* First we calculate the starting page */
	row = start >> info->page_shift;
	/* Get raw starting column */
	col = start & (info->oobblock - 1);

	if (nfc_dma_cfg(maddr, info->oobblock, 0))
			return -ERR_DMA_CFG;

	/* addr */
	NanD_Address(ADDR_COLUMN_PAGE, col, row);
	/* set command 1 cycle */
	pNFCRegs->NFCR2 = (char)NAND_READ0;
	pNFCRegs->NFCRb |= B2R; /* write to clear */
	pNFCRegs->NFCR1 = DPAHSE_DISABLE|((info->row+info->col+1)<<1)|NFC_TRIGGER;
	if (info->oobblock > 512) {
		if (NFC_WAIT_CMD_READY())
			return -ERR_NFC_CMD;
		pNFCRegs->NFCR2 = (char)NAND_READ_CONFIRM;
		pNFCRegs->NFCRb |= B2R; /* write to clear */
		pNFCRegs->NFCR1 = NAND2NFC|1<<1|NFC_TRIGGER;
	}

	row = (info->oobblock/512)+1;
	for (col = 0; col < row; col++) {
		rc = NFC_CHECK_ECC();
		if (rc < 0)
			return -ERR_NFC_READY;
		else{
			if (rc)
				err = nfc_bch_ecc_check(maddr);
		}
		/* pNFCRegs->NFCR23 |= READ_RESUME; */
	}
	rc = nand_pdma_handler();
	nand_free_pdma();
	if (rc)
		return -rc;
	if (NAND_WAIT_READY())
			return -ERR_NAND_IDLE;
	if (err)
		return err;
	return 0;
}

int nand_read_status(unsigned char cmd)
{
	int cfg = 0, status = -1;

	pNFCRegs->NFCR2 = cmd;
	cfg = DPAHSE_DISABLE|NFC2NAND|(1<<1);
	pNFCRegs->NFCRb |= B2R; /* write to clear */
	pNFCRegs->NFCR1 = cfg|NFC_TRIGGER;
	if (NFC_WAIT_READY())
		return -ERR_NFC_READY;
	pNFCRegs->NFCRb |= B2R; /* write to clear */
	cfg = SING_RW|NAND2NFC;
	pNFCRegs->NFCR1 = cfg | NFC_TRIGGER;
	if (NFC_WAIT_READY())
		return -ERR_NFC_READY;
	if (NAND_WAIT_IDLE())
		return -ERR_NAND_IDLE;
	status = pNFCRegs->NFCR0 & 0xff;

	return status;
}

int nand_read_page(
	unsigned int start,
	unsigned int maddr,
	WMTNF_INFO_t *info
)
{
	unsigned int page = 0, col = 0;
	unsigned int size = 0;
	int rc = -1;

	/* First we calculate the starting page */
	page = start >> info->page_shift;
	/* Get raw starting column */
	col = start & (info->oobblock - 1);

	size = info->oobblock;
	if (g_ncurrent.ecc == USE_SW_ECC)
		size += info->oobsize;

	if (nfc_dma_cfg(maddr, size, 0))
		return -ERR_DMA_CFG;

	/* addr */
	NanD_Address(ADDR_COLUMN_PAGE, col, page);
	/* set command 1 cycle */
	pNFCRegs->NFCR2 = (char)NAND_READ0;
	pNFCRegs->NFCRb |= B2R; /* write to clear */
	if (g_nfinfo[0].oobblock > 512)
		pNFCRegs->NFCR1 = DPAHSE_DISABLE|((info->row+info->col+1)<<1)|NFC_TRIGGER;
	else
		pNFCRegs->NFCR1 = NAND2NFC|((info->row+info->col+1)<<1)|NFC_TRIGGER;
	if (info->oobblock > 512) {
		if (NFC_WAIT_CMD_READY())
			return -ERR_NFC_CMD;
		pNFCRegs->NFCR2 = (char)NAND_READ_CONFIRM;
		pNFCRegs->NFCRb |= B2R; /* write to clear */
		pNFCRegs->NFCR1 = NAND2NFC|1<<1|NFC_TRIGGER;
	}
	if (NFC_WAIT_READY())
			return -ERR_NFC_READY;

	rc = nand_pdma_handler();
	nand_free_pdma();
	if (rc)
		return -rc;

	if (NAND_WAIT_READY())
		return -ERR_NAND_IDLE;

	return 0;
}

int nand_erase(unsigned int start)
{
	unsigned int page = 0, col = 0;

	/* First we calculate the starting page */
	page = start >> g_nfinfo[0].page_shift;
	col = start & (g_nfinfo[0].oobblock - 1);
	/* addr */
	NanD_Address(ADDR_PAGE, col, page);

	pNFCRegs->NFCR2 = (char)NAND_ERASE_SET;
	/*  trigger */
	pNFCRegs->NFCR1 = DPAHSE_DISABLE|((g_nfinfo[0].row+1)<<1)|NFC_TRIGGER;
	/*for (j = 0; j < 0x2A; j += 4)
		printf("NFCR%x ~ NFCR%x = 0x%8.8x 0x%8.8x 0x%8.8x 0x%8.8x\r\n",
		j, j+3,
		*(unsigned int *)(&pNFCRegs->NFCR0+j+0),
		*(unsigned int *)(&pNFCRegs->NFCR0+j+1),
		*(unsigned int *)(&pNFCRegs->NFCR0+j+2),
		*(unsigned int *)(&pNFCRegs->NFCR0+j+3));*/
	if (NFC_WAIT_CMD_READY())/* kevin:wait cmd ready excluding data phase  */
		return -ERR_NFC_CMD;
	if (NAND_WAIT_IDLE())
		return -ERR_NAND_IDLE;
	pNFCRegs->NFCR2 = (char)NAND_ERASE_CONFIRM;
	pNFCRegs->NFCRb |= B2R; /* write to clear */
	pNFCRegs->NFCR1 = DPAHSE_DISABLE|(1<<1)|NFC_TRIGGER;
	if (NAND_WAIT_READY())
		return -ERR_NAND_READY;
	if (NAND_WAIT_IDLE())
		return -ERR_NAND_IDLE;
	/* status */
	return nand_read_status(NAND_STATUS);
}

/* always write one page size*/
int nand_page_program(
	unsigned int start,
	unsigned int maddr,
	WMTNF_INFO_t *info
)
{
	unsigned int row = 0, col = 0;
	unsigned int size;

	/* First we calculate the starting page */
	row = start >> info->page_shift;
	col = start & (info->oobblock - 1);

	size = info->oobblock;
	if (g_ncurrent.ecc == USE_SW_ECC)
		size += info->oobsize;

	if (nfc_dma_cfg(maddr, size, 1))
		return -ERR_DMA_CFG;

	/* reset nand	 */
	pNFCRegs->NFCR2 = (char)NAND_READ0;
	pNFCRegs->NFCR1 = DPAHSE_DISABLE|(1<<1)|NFC_TRIGGER;
	if (NFC_WAIT_READY())
		return -ERR_NFC_READY;

	NanD_Address(ADDR_COLUMN_PAGE, col, row);
	pNFCRegs->NFCR2 = (char)NAND_SEQIN;
	/*  trigger */
	pNFCRegs->NFCR1 = ((info->row+info->col+1)<<1)|NFC_TRIGGER;/* command 1 cycle  */
	if (NFC_WAIT_READY())/* wait command &data completed */
		return -ERR_NFC_READY;
	/* while (!(pNFCRegs->NFCR1d&NFC_IDLE));  */
	if (NAND_WAIT_IDLE())
		return -ERR_NAND_IDLE;
	pNFCRegs->NFCR2 = (char)NAND_PAGEPROG;
	pNFCRegs->NFCRb |= B2R; /* write to clear */
	pNFCRegs->NFCR1 = DPAHSE_DISABLE|(1<<1)|NFC_TRIGGER;/* command 2 cycle */
	if (NAND_WAIT_READY())
		return -ERR_NAND_READY;
	if (NAND_WAIT_IDLE())
		return -ERR_NAND_IDLE;

	/* status */
	return nand_read_status(NAND_STATUS);
}

int WMTEraseNANDALL(unsigned int all)
{
	unsigned int naddr = 0;

	if(check_block_table(0,0))
		return -1;

	if (wmt_select_nand(0))
		return -1;

	if (all) {
		for (naddr = 0; naddr < g_nfinfo[0].totblk; naddr++)
			nand_erase(naddr<<g_nfinfo[0].erasesize);
	} else {
		for (naddr = (g_nfinfo[0].totblk-BBT_MAX_BLOCK); naddr < g_nfinfo[0].totblk; naddr++)
			nand_erase(naddr<<g_nfinfo[0].erasesize);		
	}
	g_WMTNFCBASE = 0;
	return 0;
}

int WMTEraseNAND(unsigned int start, unsigned int nums, unsigned int all)
{
	unsigned int naddr, need = 0;
	unsigned int pos = 0, end = 0;
	int rc = -1;

	if(check_block_table(0,0))
		return -1;

	if (wmt_select_nand(0))
		return -1;

	pos = g_nfinfo[0].totblk-BBT_MAX_BLOCK;
	end = start+nums;
	if (all) {
		start = 0;
		end = (g_nfinfo[0].totblk-BBT_MAX_BLOCK);
	} else {
		if(end >= pos)
			end = pos;
	}
	if (start >= end)
		return 0;

	if (start >= pos) {
		printf("Table can not erase .....\n");
		return -1;
	}

	printf("Erase start at block%d end at block%d , bad pos block%d\n",start,end-1,pos);

	for (naddr = start; naddr < (g_nfinfo[0].totblk-BBT_MAX_BLOCK); naddr++) {
		if (naddr >= end)
			break;
		if (isbbtbadblock(&g_nfinfo[0], (naddr<<g_nfinfo[0].erasesize)))
			continue;
		rc = nand_erase(naddr<<g_nfinfo[0].erasesize);
		if (rc < 0 || rc & 0x01) {
			printf("Erase failed at block%d\n", naddr);
			update_bbt2ram(&g_nfinfo[0], (naddr<<g_nfinfo[0].erasesize));
			need = 1;
			continue;
		}
	}

	if (need)
		return update_bbt2flash(&g_nfinfo[0]);

	return 0;
}

int nand_readID(unsigned char *id)
{
	unsigned int cfg = 0, i = 0;
	int status = -1;

	pNFCRegs->NFCR2 = NAND_READID;
	pNFCRegs->NFCR3 = 0xff00;
	cfg = DPAHSE_DISABLE|(0x02<<1);
	pNFCRegs->NFCR1 = cfg|NFC_TRIGGER;	 /* cfg & start */
	status = NFC_WAIT_READY();
	if (status)
		return status;
	cfg = NAND2NFC|SING_RW;
	for (i = 0; i < 5; i++) {
		pNFCRegs->NFCR1 = cfg|NFC_TRIGGER;
		status = NFC_WAIT_READY();
		if (status)
			return status;
		status = NAND_WAIT_IDLE();
		if (status)
			return status;
		id[i] = (unsigned char)pNFCRegs->NFCR0;
	}

	return 0;
}

int nand_write_block(
	unsigned int start, 
	unsigned int naddr,
	unsigned int size,
	WMTNF_INFO_t *info,
	unsigned char *oob
)
{
	unsigned int i = 0, page = 0;
	unsigned int addr = start;
	int rc = -1;

	if (oob)
		memcpy((char *)&pNFCRegs->FIFO[0], (char *)&oob[0], 64);
	else
		memset((char *)&pNFCRegs->FIFO[0], 0xFF, 64);
/*
	if (info->oobsize > 64) {
		pNFCRegs->NFCRd |= 0x08;
		memset((char *)&pNFCRegs->FIFO[0], 0xFF, 64);
		pNFCRegs->NFCRd &= 0x07;
	}
*/
	page = size/info->oobblock;
	if (size&(info->oobblock-1))
		page++;

	for (i = 0; i < page; i++) {
		rc = nand_page_program(naddr, addr, info);
		if (rc < 0 || rc & 0x01)
			return rc;
		addr = addr + info->oobblock;
		naddr = naddr + (1<<info->page_shift);
	}

	return 0;
}

int check_nand_range(unsigned int *naddr, unsigned int size, unsigned int *chip)
{
	unsigned int i, num, addr, addr1;	

	if ((*naddr+size) > g_nboundary) {
		printf("Out of boundary \n");
		return -1;
	}

	*chip = FLASH_UNKNOW;
	addr1 = *naddr;
	for (i = 0; i < g_ncurrent.maxchip; i++) {
		if (g_nfinfo[i].id == FLASH_UNKNOW)			
			break;
		addr = addr1/g_nfinfo[i].blksize;
		num = size/g_nfinfo[i].blksize;
		if ((addr+num) <= (g_nfinfo[i].totblk-BBT_MAX_BLOCK)) {
			/*printf("start&end at chip%d\n", i);*/
			if (*chip == FLASH_UNKNOW) {
				*chip = i;
				*naddr = addr1;
			}
			return 0;
		} else if (addr < (g_nfinfo[i].totblk-BBT_MAX_BLOCK)) {
			/*printf("start at chip , end at others\n");*/
			if (*chip == FLASH_UNKNOW)
				*chip = i;
			addr1 = 0;
			num = (g_nfinfo[i].totblk-BBT_MAX_BLOCK-addr);
			size = size-(num*g_nfinfo[i].blksize);
		} else {
			/*printf("start&end at others\n");*/
			addr -= (g_nfinfo[i].totblk-BBT_MAX_BLOCK);
			addr1 = addr*g_nfinfo[i].blksize;
		}
	}
	return -1;
}

int wmt_change_nand(
	unsigned int *naddr,
	unsigned int *chip,
	unsigned int need,
	WMTNF_INFO_t *info
)
{

	if ((*naddr>>info->erasesize) >= (info->totblk-BBT_MAX_BLOCK)) {
		*chip = *chip+1;
		*naddr = 0;
		if (need) {
			if (update_bbt2flash(info))
				return -1;
		}
		return wmt_select_nand(*chip);
	}

	return 0;
}

int WMTSaveImageToNAND(unsigned int naddr, unsigned int saddr, unsigned int size)
{
	unsigned int need = 0, chip = FLASH_UNKNOW;
	int rc = -1;
	unsigned int maxblks = 0;

	if (check_block_table(1, 1))
		return -1;

	if (check_nand_range(&naddr, size, &chip))
		return -1;

	if (wmt_select_nand(chip))
		return -1;

	maxblks = g_nboundary/g_nfinfo[chip].blksize;
	naddr = naddr/g_nfinfo[chip].blksize;
	naddr = naddr<<g_nfinfo[chip].erasesize;
	
	size += (g_nfinfo[chip].oobblock-1);
	size &= ~(g_nfinfo[chip].oobblock-1);

	memset((char *)&pNFCRegs->FIFO[0], 0xFF, 64);
	need = 0;
	while(1) {
		if ((naddr&((1<<g_nfinfo[chip].erasesize)-1)) == 0) {
			if (isbbtbadblock(&g_nfinfo[chip], naddr)) {
				naddr = naddr+(1<<g_nfinfo[chip].erasesize);
				if (wmt_change_nand(&naddr, &chip, need, &g_nfinfo[chip]))
					return -1;
				continue;
			}
			if ((naddr>>g_nfinfo[chip].erasesize) >= maxblks) {
				printf("Out of boundly\n");
				return -1;
			}
			rc = nand_erase(naddr);
			if (rc < 0 || rc & 0x01) {
				printf("Erase failed at block%d\n", naddr>>g_nfinfo[chip].erasesize);
				update_bbt2ram(&g_nfinfo[chip], naddr);
				naddr = naddr+(1<<g_nfinfo[chip].erasesize);
				need = 1;
				if (wmt_change_nand(&naddr, &chip, need, &g_nfinfo[chip]))
					return -1;		
				continue;
			}
		}

		rc = nand_page_program(naddr, saddr, &g_nfinfo[chip]);
		if (rc < 0 || rc & 0x01) {
			update_bbt2ram(&g_nfinfo[chip], naddr);
			naddr = naddr>>g_nfinfo[chip].erasesize;			
			printf("Write block failed at block%d\n", naddr);
			naddr = ((naddr+1)<<g_nfinfo[chip].erasesize);
			saddr &= ~(g_nfinfo[chip].blksize-1);
			need = 1;
			if (wmt_change_nand(&naddr, &chip, need, &g_nfinfo[chip]))
				return -1;
			continue;
		}
		saddr = saddr + g_nfinfo[chip].oobblock;
		naddr = naddr + (1<<g_nfinfo[chip].page_shift);
		size = size-g_nfinfo[chip].oobblock;
		if (!size)
			break;
		if (wmt_change_nand(&naddr, &chip, need, &g_nfinfo[chip]))
			return -1;
	}

	if (need)
		return update_bbt2flash(&g_nfinfo[chip]);

	printf("\r\nWrite To NAND Flash OK\r\n");
	return 0;
}

int nand_read_block(
	unsigned int start,
	unsigned int naddr,
	unsigned int size,
	WMTNF_INFO_t *info
)
{
	unsigned int i = 0, page = 0;
	unsigned int addr = start;
	int rc = -1;

	page = size/info->oobblock;
	if (size&(info->oobblock-1))
		page++;
	for (i = 0; i < page; i++) {
		rc = nfc_read_page(naddr, addr, info);
		if (rc) {
			printf("Err at %x\n", naddr);
			return rc;
		}
		addr = addr+info->oobblock;
		naddr = naddr+(1<<info->page_shift);
	}
	return 0;
}

int WMTLoadImageFormNAND(unsigned int naddr, unsigned int maddr, unsigned int size)
{
	unsigned int ret, chip = FLASH_UNKNOW;
	int rc = -1;
	unsigned int maxblks = 0;

	if (check_block_table(1, 1))
		return -1;

	if (check_nand_range(&naddr, size, &chip))
		return -1;

	if (wmt_select_nand(chip))
		return -1;

	size += (g_nfinfo[chip].oobblock-1);
	size &= ~(g_nfinfo[chip].oobblock-1);
	
	maxblks = g_nboundary/g_nfinfo[chip].blksize;
	naddr = naddr/g_nfinfo[chip].oobblock;
	naddr = naddr<<g_nfinfo[chip].page_shift;

	while (1) {
		if ((naddr&((1<<g_nfinfo[chip].erasesize)-1)) == 0) {
			if (isbbtbadblock(&g_nfinfo[chip], naddr)) {
				naddr = naddr+(1<<g_nfinfo[chip].erasesize);
				if (wmt_change_nand(&naddr, &chip, 0, &g_nfinfo[chip]))
					return -1;
				continue;
			}
			if ((naddr>>g_nfinfo[chip].erasesize) >= maxblks) {
				printf("Out of boundly\n");
				return -1;
			}
		}
		rc = nfc_read_page(naddr, maddr, &g_nfinfo[chip]);
		if (rc) {
			printf("Err at %x\n", naddr);
			return rc;
		}
		maddr = maddr+g_nfinfo[chip].oobblock;
		naddr = naddr+(1<<g_nfinfo[chip].page_shift);
		size = size-g_nfinfo[chip].oobblock;
		if (!size) {
			printf("Read finsih\n");
			return 0;
		}
		if (wmt_change_nand(&naddr, &chip, 0, &g_nfinfo[chip]))
			return -1;
	}

	return -1;
}


int isbbtbadblock(WMTNF_INFO_t *info, unsigned int addr)
{
#ifdef USE_BBT
	unsigned int ffword = 0x03;

	if (!g_bbt)
		return -1;	
	addr >>= info->erasesize;
	if ((g_bbt[addr>>4] & (ffword<<((addr&0x0f)*2))) == 0) {
		/* printf("bbt : block%d is bad\n", addr); */
		return 1;
	}
#endif
	return 0;
}

int update_bbt2flash(WMTNF_INFO_t *info)
{
	unsigned int i, j, cnt;
	int rc = -1;
	unsigned char *pattern;
	unsigned char fifo[64];
	unsigned int ofs,ofs1;
	unsigned char bbt_pattern[] = {'B', 'b', 't', '0' };
	unsigned char mirror_pattern[] = {'1', 't', 'b', 'B' };

	if (!g_bbt) {
		printf("bbt = NULL\n");
		return -1;
	}

	j = 2;
	for (i = (info->totblk-1); i >= (info->totblk-BBT_MAX_BLOCK); i--) {
		if (isbbtbadblock(info, (i<<info->erasesize))) {
			//printf("block%d is bad\n",i);
			continue;
		}
		rc = nand_erase(i<<info->erasesize);
		if ((rc < 0) || ((rc&0x01) != 0)) {
			printf("Erase failed at block%d\n", i);
			update_bbt2ram(info, (i<<info->erasesize));
			continue;
		}
		j--;
		if (j == 0)
			break;
	}

	if (j) {
		printf("Not enough block to store bbt .....\n");
		return -1;
	}

	ofs = NAND_LARGE_DWRESERVED1_OFS;
	ofs1 = NAND_LARGE_DWRESERVED2_OFS;
	if (g_nfinfo[0].oobblock<=0x200) {
		ofs = NAND_SMALL_DWRESERVED1_OFS;
		ofs1 = NAND_SMALL_DWRESERVED2_OFS;
	}
	bbt_version++;
	cnt = 2;
	for (i = (info->totblk-1); i >= (info->totblk-BBT_MAX_BLOCK); i--) {
		if (isbbtbadblock(info, (i<<info->erasesize)))
			continue;
		if (cnt == 2) {
			info->whereisbbt[0] = i;
			pattern = bbt_pattern;
			printf("write bbt_pattern to block%d ...", i);
		} else {
			info->whereisbbt[1] = i;
			pattern = mirror_pattern;
			printf("write mirror_pattern to block%d ...", i);
		}
		memset(fifo, 0xff, 64);
		for (j = 0; j < 4; j++)
			fifo[ofs+j] = pattern[j];
		fifo[ofs1] = (unsigned char)bbt_version;
		rc = nand_write_block((unsigned int)&g_bbt[0], (i<<info->erasesize), (info->oobblock), info, fifo);
		if (rc) {
			update_bbt2ram(info, (i<<info->erasesize));
			bbt_version++;
			printf(" failed\n");
			continue;
		} else
			printf(" ok\n");
		cnt--;
		if (cnt == 0)
			break;
	}

	return 0;
}

int update_bbt2ram(WMTNF_INFO_t *info, unsigned int addr)
{
	unsigned int ffword = 0x03;

	if (!g_bbt)
		return -1;

	addr >>= info->erasesize;
	g_bbt[addr>>4] &= ~(ffword<<((addr&0x0f)*2));

	return 0;
}

int creat_bbt(WMTNF_INFO_t *info)
{
	unsigned int i, j = 0, count = 0;
	unsigned int page[2];
	unsigned int bpos[2];
	unsigned int ffword = 0x03;
	int rc = -1;
	char *ofs;

	printf("creat_bbt\n");
	/* scan bbt first */
	bpos[0] = info->page[0]&((1<<info->page_shift)-1);
	bpos[1] = info->page[1]&((1<<info->page_shift)-1);
	page[0] = info->page[0]>>info->page_shift<<info->page_shift;
	page[1] = info->page[1]>>info->page_shift<<info->page_shift;

	nfc_ecc_set(USE_SW_ECC, info);
	pNFCRegs->NFCR23 |= DIS_BCH_ECC;

	if (!g_bbt) {
		i = info->totblk>>4;
		if (i < info->oobblock)
			j = info->oobblock;
		else {
			j = i/info->oobblock;
			j = (j+1)*info->oobblock;
		}
		g_bbt = malloc(j);
		if (!g_bbt) {
			printf("alloc bbt failed\n");
			return -1;
		}
	}
	memset(g_bbt, 0xff, j);
	ofs = (char *)(__NFC_BUF+info->oobblock);
	for (i = 0; i < info->totblk; i++) {
		for (j = 0; j < 2; j++) {
			rc = nfc_read_page(
				(i<<info->erasesize)+page[j],
				__NFC_BUF,
				info
			);
			if (ofs[bpos[j]] != 0xff) {
				g_bbt[i>>4] &= ~(ffword<<((i&0x0f)*2));
				count++;
				printf("find bad block : block%d\n", i);
				break;
			}
		}
	}
	printf("Total find %d bad blocks\n", count);
	nfc_ecc_set(info->ecc, info);
	bbt_version = 0;

	return update_bbt2flash(info);
}

int get_pattern_small(
	unsigned int block,
	unsigned int *tag,
	unsigned int *version,
	WMTNF_INFO_t *info
)
{
	unsigned int pos, pos1;
	int rc = -1;

	rc = nfc_read_page((block<<info->erasesize), __NFC_BUF, info);
	if (rc) {
		printf("Read Tag failed rc=%d at %x\n", rc, (block<<info->erasesize));
		return rc;
	}
	pos = NAND_SMALL_DWRESERVED1_OFS;
	pos1 = NAND_SMALL_DWRESERVED2_OFS;
	*tag = pNFCRegs->FIFO[pos+3]|
		(pNFCRegs->FIFO[pos] << 8)|
		(pNFCRegs->FIFO[pos+1] << 16)|
		(pNFCRegs->FIFO[pos+2] << 24);
	*version = pNFCRegs->FIFO[pos1];

	return 0;
}

int get_pattern_large(
	unsigned int block,
	unsigned int *tag,
	unsigned int *version,
	WMTNF_INFO_t *info
)
{
	unsigned int pos, pos1;
	int rc = -1;

	rc = nfc_read_page((block<<info->erasesize), __NFC_BUF, info);
	if (rc) {
		printf("Read Tag failed rc=%d at %x\n", rc, (block<<info->erasesize));
		return rc;
	}
	pos = NAND_LARGE_DWRESERVED1_OFS;
	pos1 = NAND_LARGE_DWRESERVED2_OFS;
	*tag = pNFCRegs->FIFO[pos+3]|
		(pNFCRegs->FIFO[pos] << 8)|
		(pNFCRegs->FIFO[pos+1] << 16)|
		(pNFCRegs->FIFO[pos+2] << 24);
	*version = pNFCRegs->FIFO[pos1];

	return 0;
}

int WMT_check_pattern(
	unsigned int block,
	unsigned int *type,
	unsigned int *version,
	WMTNF_INFO_t *info
)
{
	unsigned int tag = 0;

	*type = 0;
	if (info->oobblock > 0x200)
		get_pattern_large(block, &tag, version, info);
	else
		get_pattern_small(block, &tag, version, info);
	printf("block%d tag=%x ", block, tag);
	switch (tag) {
	case 0x74624230:
		*type = 1;
		printf(" version =%x\n", *version);
		return 0;
	case 0x62743142:
		*type = 2;
		printf(" version =%x\n", *version);
		return 0;
	default:
		break;
	}
	printf(" no version\n");
	return 0;
}

int find_bbt(WMTNF_INFO_t *info)
{
	int rc = -1;
	unsigned int i, type;
	unsigned int count = 2;
	unsigned int version = 0xff;
	unsigned int size = 0;

	if (info->id == FLASH_UNKNOW)
		return -ERR_NAND_UNKNOW;

	info->whereisbbt[0] = info->whereisbbt[1] = 0xFFFFFFFF;
	for (i = info->totblk-1; i > (info->totblk-1-BBT_MAX_BLOCK); i--) {
		WMT_check_pattern(i, &type, &version, info);
		if (type) {
			info->whereisbbt[type-1] = i;
			count--;
			bbt_version = version;
		} else
			continue;
		if (!count)
			break;
	}

	if ((info->whereisbbt[0] == 0xFFFFFFFF) && (info->whereisbbt[1] == 0xFFFFFFFF))
		return 1;

	i = info->whereisbbt[0];
	if (info->whereisbbt[0] == 0xFFFFFFFF)
		i = info->whereisbbt[1];

	size = info->totblk>>4;
	if (size < info->oobblock)
		size = info->oobblock;
	else {
		type = size/info->oobblock;
		size = (type+1)*info->oobblock;
	}

	if (!g_bbt) {
		g_bbt = malloc(size);
		if (!g_bbt) {
			printf("alloc bbt failed\n");
			return -ERR_UNKNOW;
		}
	}
	rc = nand_read_block((unsigned int)&g_bbt[0], (i<<info->erasesize), size, info);
	if (rc)
		return rc;

	if ((info->whereisbbt[0] == 0xFFFFFFFF) || (info->whereisbbt[1] == 0xFFFFFFFF))
		return update_bbt2flash(info);

	return 0;
}

int tellme_badblock(unsigned int chip)
{
	int rc = -1;
	unsigned int naddr, count;

	if (g_WMTNFCBASE != __NFC_BASE) {
		rc = nfc_init(1, 0);
		if (rc) {
			printf("Init Flash Failed rc=%d\r\n", rc);
			return rc;
		}
	}

	if (wmt_select_nand(chip))
		return -1;

	count = 0;
	printf("Chip%d :\n", chip);
	for (naddr = 0; naddr < g_nfinfo[chip].totblk; naddr++) {
		if (isbbtbadblock(&g_nfinfo[chip], naddr<<g_nfinfo[chip].erasesize)) {
			printf("block%d is bad\n", naddr);
			count++;
		}
	}
	printf("Total %d Bad Block\n", count);

	return 0;
}

int tellme_whereistable(unsigned int chip)
{

	if (check_block_table(1, 0))
		return -1;

	if (g_nfinfo[chip].id == FLASH_UNKNOW) {
		printf("NAND FLASH NOT FOUND ....\n");
		return -1;
	}
	printf("Chip%d :\n", chip);
	printf("bbt_pattern store in block%d\n", g_nfinfo[chip].whereisbbt[0]);
	printf("mirror_pattern store in block%d\n", g_nfinfo[chip].whereisbbt[1]);

	return 0;
}

int tellme_nandinfo(void)
{
	unsigned int chip;
	int rc = -1;

	if (g_WMTNFCBASE != __NFC_BASE) {
		rc = nfc_init(0, 0);
		g_WMTNFCBASE = 0;
		if (rc) {
			printf("Init Flash Failed rc=%d\r\n", rc);
			return rc;
		}
	}
	for (chip = 0; chip < MAX_NAND_CHIP; chip++) {
		if (g_nfinfo[chip].id != FLASH_UNKNOW) {
 			printf("%s : Using CE%d\n", g_NAND_PRODUCT_TABLE[g_nfinfo[chip].id].cProductName, g_nfinfo[chip].ce);
			printf("page size = %d , Spare size = %d\n", g_nfinfo[chip].oobblock, g_nfinfo[chip].oobsize);
			printf("column cycle is %d row cycle is %d\n", g_nfinfo[chip].col, g_nfinfo[chip].row);
  			printf("Erase size is %d bit Total Blocks is %d\n", g_nfinfo[chip].erasesize, g_nfinfo[chip].totblk);
  		} else
  			printf("chip%d is not exit !!!!!!\n", chip); 
	}
	return 0;
}

int check_block_table(unsigned int find, unsigned int scan)
{
	int rc = -1;

	if ((g_WMTNFCBASE != __NFC_BASE) || (g_bbt == NULL)) {
		rc = nfc_init(find, scan);
		if (rc) {
			printf("Init Flash Failed rc=%d\r\n", rc);
			return rc;
		}
	}
	return 0;
}


#ifdef POST_TEST_FUNS

#ifndef __PMC_BASE
#define __PMC_BASE      0xD8130000      /* 64K */
#endif

#define OSTC_ADDR       (__PMC_BASE + 0x0120)
#define OSTA_ADDR       (__PMC_BASE + 0x0124)
#define OSCR_ADDR       (__PMC_BASE + 0x0110)

#define REG32_VAL(x) *(volatile unsigned int *)(x)

int ost_init(void)
{
	unsigned int tmp = 0;

	tmp = REG32_VAL(OSTC_ADDR);
	if ((tmp&0x01) == 0)
		REG32_VAL(OSTC_ADDR) = 1;
	return 0;
}

int read_ost(int *val)
{
	int sw_count = 300000;
	unsigned int tmp = 0;

	tmp = REG32_VAL(OSTC_ADDR);
	if ((tmp&0x02) == 0) {
		tmp |= 0x02;
		REG32_VAL(OSTC_ADDR) = tmp;
	}
	while(1) {
		tmp = REG32_VAL(OSTA_ADDR);
		if ((tmp&0x20) != 0x20)
			break;
		if (--sw_count == 0) {
			printf("Read OST Count Request Failed\n");
			break;
		}
	}
	*val = (int)REG32_VAL(OSCR_ADDR);

	return 0;
}

#define TEST_SRC_ADR (80*1024*1024)
#define TEST_DST_ADR (96*1024*1024)
#define TEST_FILE_SIZE (16*1024*1024)

int test_nand_rw(int cnt)
{
	int i, j, x, y;
	unsigned int naddr, size, blks = 0;
	unsigned int smaddr, dmaddr, page, pcnt;
	char *sbuf, *dbuf;
	int t1, t2;

	if(check_block_table(1,0))
		return -1;

	naddr = 0;
	smaddr = TEST_SRC_ADR;
	dmaddr = TEST_DST_ADR;
	size = TEST_FILE_SIZE;
	if (!cnt)
		cnt = 1;
	pcnt = g_nfinfo[0].blksize/g_nfinfo[0].oobblock;
	ost_init();
	for (j = 0; j < cnt; j++) {
		printf("\r\nTest%d ", j);
/*
		smaddr += j&0x0f;
		dmaddr += (j>>4)&0x0f;
*/
		printf("Src = 0x%x , Dst = 0x%x , Size = %dMB\n", smaddr, dmaddr, size>>20);
		blks = TEST_FILE_SIZE/g_nfinfo[0].blksize;
		read_ost(&t1);
		WMTEraseNAND((naddr/g_nfinfo[0].blksize),blks,0);
		read_ost(&t2);
		t1 = t2-t1;
		t1 = t1/3;
		printf("Erase time %d.%d ms\n",t1/1000,t1%1000);
		printf("Test Erase ..... ");
		page = naddr/g_nfinfo[0].oobblock;
		for (x = 0; x < size; x += g_nfinfo[0].oobblock) {
			if ((page&(pcnt-1)) == 0) {
				if (isbbtbadblock(&g_nfinfo[0], (page<<g_nfinfo[0].page_shift))) {
					page += pcnt;
					x += g_nfinfo[0].blksize;
					continue;
				}
			}
			for (y = 0; y < 64; y++)
				pNFCRegs->FIFO[y] = 0;
			memset((char *)__NFC_BUF, 0, g_nfinfo[0].oobblock);
			nfc_read_page((page<<g_nfinfo[0].page_shift), __NFC_BUF, &g_nfinfo[0]);

			for (y = 0; y < 24; y++) {
				if (pNFCRegs->FIFO[y] != 0xff) {
					printf("OOB : Compare Failed at block%d , page %d , ofs = %d (0x%x)\n", page/pcnt, page%pcnt, y, pNFCRegs->FIFO[y]);
					break;
					//return 1;
				}
			}
			dbuf = (char *)__NFC_BUF;
			for (y = 0; y < g_nfinfo[0].oobblock; y++) {
				if (*dbuf != 0xFF) {
					printf("Compare Failed at block%d , page %d , ofs = %d (0x%x)\n", page/pcnt, page%pcnt, y, *dbuf);						
					break;
					//return -1;
				}
				dbuf++;
			}

			page++;
		}
		printf("ok\n");

		read_ost(&t1);
		printf("Write to nand .....");
		if (WMTSaveImageToNAND(naddr, smaddr, size))
			return -1;
		read_ost(&t2);
		t1 = t2-t1;
		if (t1 > 0) {	
			t2 = ((size>>20)*100)/(t1/3000000);
			printf("Write Speed  %d.%d Mbyte/sec\n",t2/100,t2%100);
		}
		printf("Clean Read Buffer ......\n");
		memset((char *)dmaddr, 0, size);
		read_ost(&t1);
		printf("Start Read from nand ....");
		if (WMTLoadImageFormNAND(naddr, dmaddr, size))
			return -1;
		read_ost(&t2);
		t1 = t2-t1;
		if (t1 > 0) {
			t2 = ((size>>20)*100)/(t1/3000000);
			printf("Read Speed  %d.%d Mbyte/sec\n",t2/100,t2%100);
		}

		sbuf = (char *)smaddr;
		dbuf = (char *)dmaddr;		
		printf("Start Compare ... ");
		for (i = 0; i < size; i++) {
			if (*sbuf != *dbuf) {
				printf("Compare failed ofs = %d , src = 0x%x , dst = 0x%x\n",i,*sbuf,*dbuf);
				//return -1;
				break;
			}
			sbuf++;
			dbuf++;
		}
		printf("Compare Success size = %dMB\n", size>>20);

		if (j == (cnt-1))
			return 0;
		printf("Invert Src Data ...");
		sbuf = (char *)smaddr;
		for (i = 0; i < size; i++) {
			*sbuf = ~*sbuf;
			sbuf++;
		}
		printf("ok\n");
/*
		smaddr -= j&0x0f;
		dmaddr -= (j>>4)&0x0f;
*/
		page = naddr/g_nfinfo[0].blksize;
		y = 0;
		for (x = page; x < (page+blks); x++) {
			if (isbbtbadblock(&g_nfinfo[0], (x<<g_nfinfo[0].erasesize)))
				y++;
		}
		page = page+blks+y;
		if ((page+blks) >= (g_nfinfo[0].totblk-BBT_MAX_BLOCK))
			naddr = 0;
		else
			naddr = page*g_nfinfo[0].blksize;

	}
	return 0;
}

int find_next_bch_pattern(struct post_ecc_info *info, unsigned int type)
{
	unsigned int end = (g_nfinfo[0].oobblock<<3);
	int *buf;
	unsigned int i;

	buf = (int *)&info->pos[0];
	if (type) {
		buf = (int *)&info->pred[0];
		end = 24*8;
	} else {
		if (buf[0] > (end-info->cnt) && (buf[0] > 0))
			return 1;
	}
	/*printf("find_next_bch_pattern : ");*/
	i = 1;
	while(1) {
		buf[info->cnt-i]++;
		if (buf[info->cnt-i] <= (end-i)) {
			break;
		} else {
			if (info->cnt == i)
				break;
		}
		i++;
	}
	if (buf[0] > (end-info->cnt))
		return 1;

	i = info->cnt-i;
	for (;i < (info->cnt-1); i++) {
		buf[i+1] = buf[i]+1;
	}
/*
	for (i = 0; i < info->cnt; i++) {
		if (type)
			printf("red  : %d : byte %d , bit %d\n", i, info->pred[i]/8, info->pred[i]%8);
		else
			printf("data : %d : byte %d , bit %d\n", i, info->pos[i]/8, info->pos[i]%8);
	}
*/
	return 0;
}

void creat_bch_test_data(struct post_ecc_info *info, unsigned int maddr, unsigned int type)
{
	char *buf = (char *)maddr;
	unsigned int i, pos, pos1, j;
/*
	if (type)
		printf("red  : creat_bch_test_data 0x%x , %d ",maddr,info->cnt);
	else
		printf("data : creat_bch_test_data 0x%x %d ",maddr,info->cnt);
*/

	j = 0;
	for (i = 0; i < info->cnt; i++) {
		if (type) {
			pos = info->pos[i]/8;
			pos1 = info->pos[i]&0x07;
			j = pos>>9;
			buf = (char *)(maddr+j*info->eccbytes);
		} else {
			pos = info->pred[i]/8;
			pos1 = info->pred[i]&0x07;
			j = g_nfinfo[0].oobblock>>9;
			buf = (char *)(maddr+(info->eccbytes*j));
		}		
		/*printf(" pos %d , pos1 %d \n", pos, pos1);*/
		if (buf[pos]&(1<<pos1))
			buf[pos] &= ~(1<<pos1);
		else
			buf[pos] |= (1<<pos1);
	}

	/*printf(" ...ok\n");*/
}

int erase_bch_block(unsigned int *addr)
{
	int rc = -1, cnt = 0;
	unsigned int naddr = *addr;
	int need = 0, done = 1;

	while(done) {
		if (cnt > 5)
			done = 0;
		if (naddr >= g_nfinfo[0].totblk-BBT_MAX_BLOCK)
			naddr = 0;
		if (isbbtbadblock(&g_nfinfo[0], naddr)) {
			naddr = naddr>>g_nfinfo[0].erasesize;
			naddr++;
			naddr = naddr<<g_nfinfo[0].erasesize;
			continue;
		}
		rc = nand_erase(naddr);
		if ((rc < 0) || (rc&0x01)) {
			printf("Erase failed at block%d\n", naddr>>g_nfinfo[0].erasesize);
			update_bbt2ram(&g_nfinfo[0], naddr);
			need = 1;			
		}
		done = 0;		
	}
	*addr = naddr;
	if (need)
		return update_bbt2flash(&g_nfinfo[0]);
	return done;
}

int create_ecc_test_pattern(int type, unsigned int src, struct post_ecc_info *info)
{
	if (type) {
		if (find_next_bch_pattern(info, 0)) {
			printf("data scan end\n");
			return 1;
		}

		if (info->pred[0] != 0xFFFF) {
			if (find_next_bch_pattern(info, 1)) {
			//printf(" red scan end ");
			info->pred[0] = 0xFFFF;
			}
		}
	}
	creat_bch_test_data(info, src, 0);
	if (info->pred[0] != 0xFFFF)
		creat_bch_test_data(info, src, 1);
	return 0;
}

int write_bch_block(unsigned int *addr, unsigned int src, unsigned int *pages, struct post_ecc_info *info)
{
	unsigned int naddr, i;
	int need = 0, finish = 0, rc = -1;
	struct post_ecc_info info1;

	if (info)
		memcpy(&info1, info, sizeof(struct post_ecc_info));

	while(1) {
		if (erase_bch_block(addr)) {
			printf("Erase block failed\n");
			return -1;
		}
		naddr = *addr;
		for (i = 0; i < *pages; i++) {
			if (info)
				finish = create_ecc_test_pattern(1, src, info);

			rc = nand_page_program((naddr+(i<<g_nfinfo[0].page_shift)), src, &g_nfinfo[0]);
			if ((rc < 0) || (rc&0x01)) {
				printf("Write Failed at block%d , page %d , rc = %d\n", naddr>>g_nfinfo[0].erasesize, i, rc);
				update_bbt2ram(&g_nfinfo[0], naddr);				
				need = 1;
				naddr = naddr>>g_nfinfo[0].erasesize;
				naddr++;
				naddr = naddr<<g_nfinfo[0].erasesize;
				*addr = naddr;
				break;
			}
			if (info)
				create_ecc_test_pattern(0, src, info);
			rc = 0;
			if (finish == 1) {
				*pages = (i+1);
				break;
			}
		}

		if (rc) {
			if (info)
				memcpy(info, &info1, sizeof(struct post_ecc_info));
			finish = 0;
			continue;
		}
		break;
	}
	if (need)
		return update_bbt2flash(&g_nfinfo[0]);
	return 0;
}

#define POST_ECC_BUFFER 0x100000

int test_nand_ecc(unsigned int cnt)
{
	struct post_ecc_info info, binfo;
	unsigned int i, j;
	unsigned int cmp;
	int rc = -1;
	unsigned int src, dest, naddr, org;
	unsigned int tmp = 0x55aa55aa;
	unsigned int pcnt;


	if(check_block_table(1, 0))
		return -1;

	switch(g_nfinfo[0].ecc) {
	case USE_HW_1bit_ECC:
		info.cnt = 1;
		info.eccbytes = (512+0); /* need to check */
		break;
	case USE_HW_4bit_ECC:
		info.cnt = 4;
		info.eccbytes = 520;
		break;
	case USE_HW_8bit_ECC:
		info.cnt = 8;
		info.eccbytes = 528;
		break;
	case USE_HW_12bit_ECC:
		info.cnt = 12;
		info.eccbytes = (512+20);
		break;
	default:
		printf("Err : Un-know ECC bit\n");
		return -1;
	}
	if (cnt) {
		if (cnt > info.cnt) {
			printf("Too many ECC bit , input = %d , max = %d\n", cnt, info.cnt);
			return -1;
		}
		info.cnt = cnt;
	}

	for (i = 0; i < info.cnt; i++) {
		info.pos[i] = i;
		info.pred[i] = i;
	}

	info.pos[info.cnt-1]--;
	info.pred[info.cnt-1]--;

	org = POST_ECC_BUFFER;
	dest = org + g_nfinfo[0].oobblock;
	src = dest + g_nfinfo[0].oobblock;
	naddr = 0;
	pcnt = g_nfinfo[0].blksize/g_nfinfo[0].oobblock;

	memset((char *)org, 0, (1024*1024));

	for (i = 0; i < g_nfinfo[0].oobblock; i+=4)
		*(unsigned int *)(org+i) = tmp;

	for (i = 0; i < g_nfinfo[0].oobsize; i++)
		pNFCRegs->FIFO[i] = 0xff;

	nfc_ecc_set(g_nfinfo[0].ecc, &g_nfinfo[0]);
	tmp = 1;
	if (write_bch_block(&naddr, org, &tmp, NULL)) {
		printf("write_bch_block failed\n");
		return -1;
	}

	nfc_ecc_set(USE_SW_ECC, &g_nfinfo[0]);
	/*pNFCRegs->NFCR12 |= OLDATA_EN;*/
	pNFCRegs->NFCR23 |= DIS_BCH_ECC;

	rc = nfc_read_page(naddr, src, &g_nfinfo[0]);
	if (rc) {
		printf("Read Failed at block%d , page %d\n", naddr>>g_nfinfo[0].erasesize, i);
		return -1;
	}

	tmp = pcnt;
	while(1) {

		g_ecc_dcnt = g_ecc_rcnt = 0;

		nfc_ecc_set(USE_SW_ECC, &g_nfinfo[0]);
		pNFCRegs->NFCR12 |= OLDATA_EN;
		pNFCRegs->NFCR23 |= DIS_BCH_ECC;

		memcpy(&binfo, &info, sizeof(struct post_ecc_info));

		if (write_bch_block(&naddr, src, &tmp, &info)) {
			printf("write_bch_block failed\n");
			return -1;
		}

		memcpy(&info, &binfo, sizeof(struct post_ecc_info));

		nfc_ecc_set(g_nfinfo[0].ecc, &g_nfinfo[0]);

		for (i = 0; i < tmp; i++) {
			g_ecc_dcnt = g_ecc_rcnt = 0;
			rc = nfc_read_page((naddr+(i<<g_nfinfo[0].page_shift)), dest, &g_nfinfo[0]);
			if (rc) {
				printf("Read Failed at block%d , page %d\n", naddr>>g_nfinfo[0].erasesize, i);
				return -1;
			}

			cmp = 0;
			if (find_next_bch_pattern(&info, 0) == 0)
				cmp |= 0x01;
			if (info.pred[0] != 0xFFFF) {
				if (find_next_bch_pattern(&info, 1))
					info.pred[0] = 0xFFFF;
				else
					cmp |= 0x10;
			}

			if (cmp&0x11) {
				printf("Check ECC information : \n");
				if (cmp&0x01) {
					printf("Data Error : %d , Result : %d\n", info.cnt, g_ecc_dcnt);
					for (j = 0; j < info.cnt; j++)
						printf("byte%d , bit %d , result : byte%d , bit %d\n", \
							info.pos[j]/8, info.pos[j]%8, g_ecc_result.pos[j]/8, g_ecc_result.pos[j]%8);
				}
				if (cmp&0x10) {
					printf("Red Error : %d , Result : %d\n", info.cnt, g_ecc_rcnt);
					for (j = 0; j < info.cnt; j++)
						printf("byte%d , bit %d , result : byte%d , bit %d\n", \
							info.pred[j]/8, info.pred[j]%8, g_ecc_result.pred[j]/8, g_ecc_result.pred[j]%8);
				}
				if (cmp&0x01) {
					printf("Compare Date Area.... ");
					for (j = 0; j < g_nfinfo[0].oobblock; j++) {
						if (*(char *)(org+j) != *(char *)(dest+j)) {
							printf("Failed at , ofs =%d , src = 0x%x , dst = 0x%x\n", j, *(char *)(org+j), *(char *)(dest+j));
							return -1;
						}
					}
					printf("ok\n");
				}
				if (cmp&0x10) {
					printf("Compare Red Area.... ");
					for (j = 0; j < 24; j++) {
						if (0xff != pNFCRegs->FIFO[j]) {
						printf("Failed at , ofs =%d , src = 0xff , dst = 0x%x\n",	\
							j, pNFCRegs->FIFO[j]);
							return -1;
						}
					}
					printf("ok\n");
				}
			}
		}
		return 0;
		memcpy(&binfo, &info, sizeof(struct post_ecc_info));
		if (find_next_bch_pattern(&info, 0))
			return 0;
		memcpy(&info, &binfo, sizeof(struct post_ecc_info));

		if (tmp != pcnt)
			return 0;
		naddr = naddr>>g_nfinfo[0].erasesize;
		naddr++;
		if (naddr >= (g_nfinfo[0].totblk-BBT_MAX_BLOCK))
			naddr = 0;		
		naddr = naddr<<g_nfinfo[0].erasesize;
	}
		
}

int set_blockisbad(unsigned int blk)
{
	unsigned int ffword = 0x03;

	if (!g_bbt)
		return -1;	

	g_bbt[blk>>4] &= ~(ffword<<((blk&0x0f)*2));

	return update_bbt2flash(&g_nfinfo[0]);
}

int set_blockisgood(unsigned int blk)
{
	unsigned int ffword = 0x03;

	if (!g_bbt)
		return -1;	

	g_bbt[blk>>4] |= (ffword<<((blk&0x0f)*2));

	return update_bbt2flash(&g_nfinfo[0]);
}


int test_nand_table(unsigned int chip)
{
	int rc = -1;
	unsigned int i, j = 0, count = 0;
	unsigned int page[2];
	unsigned int bpos[2];
	unsigned int ffword = 0x03;
	char *ofs;

	if (nfc_init(0, 0)) {
		printf("init nand failed\n");
		return -1;
	}
	if (chip > 1)
		chip = 1;

	if (g_nfinfo[chip].id == FLASH_UNKNOW) {
		printf("NAND FLASH NOT FOUND ....\n");
		return -1;
	}

	if (g_nfinfo[chip].wmt_nfc.nfc12 != pNFCRegs->NFCR12)
		pNFCRegs->NFCR12 = g_nfinfo[chip].wmt_nfc.nfc12;

	if (g_nfinfo[chip].wmt_nfc.nfc14 != pNFCRegs->NFCR14)
		pNFCRegs->NFCR14 = g_nfinfo[chip].wmt_nfc.nfc14;

	if (g_nfinfo[chip].wmt_nfc.nfc17 != pNFCRegs->NFCR17)
		pNFCRegs->NFCR12 = g_nfinfo[chip].wmt_nfc.nfc17;

	if (g_ncurrent.ecc != g_nfinfo[chip].ecc)
		nfc_ecc_set(g_nfinfo[chip].ecc, &g_nfinfo[chip]);

	if (set_nfc_ce((int)g_nfinfo[chip].ce))
		return -1;

	printf("find bad block table ( chip%d , select = %x ) ......  \n",chip,pNFCRegs->NFCR11);
	rc = find_bbt(&g_nfinfo[chip]);
	if (rc > 0) {
		printf("creat_bbt\n");
		/* scan bbt first */
		bpos[0] = g_nfinfo[chip].page[0]&((1<<g_nfinfo[chip].page_shift)-1);
		bpos[1] = g_nfinfo[chip].page[1]&((1<<g_nfinfo[chip].page_shift)-1);
		page[0] = g_nfinfo[chip].page[0]>>g_nfinfo[chip].page_shift<<g_nfinfo[chip].page_shift;
		page[1] = g_nfinfo[chip].page[1]>>g_nfinfo[chip].page_shift<<g_nfinfo[chip].page_shift;

		nfc_ecc_set(USE_SW_ECC, &g_nfinfo[chip]);
		pNFCRegs->NFCR23 |= DIS_BCH_ECC;

		if (!g_bbt) {
			i = g_nfinfo[chip].totblk>>4;
			if (i < g_nfinfo[chip].oobblock)
				j = g_nfinfo[chip].oobblock;
			else {
				j = i/g_nfinfo[chip].oobblock;
				j = (j+1)*g_nfinfo[chip].oobblock;
			}
			g_bbt = malloc(j);
			if (!g_bbt) {
				printf("alloc bbt failed\n");
				return -1;
			}
		}
		memset(g_bbt, 0xff, j);
		ofs = (char *)(__NFC_BUF+g_nfinfo[chip].oobblock);
		for (i = 0; i < g_nfinfo[chip].totblk; i++) {
			for (j = 0; j < 2; j++) {
				rc = nfc_read_page(
					(i<<g_nfinfo[chip].erasesize)+page[j],
					__NFC_BUF,
					&g_nfinfo[chip]
				);
				if (ofs[bpos[j]] != 0xff) {
					g_bbt[i>>4] &= ~(ffword<<((i&0x0f)*2));
					count++;
					printf("find bad block : block%d\n", i);
					break;
				}
			}
/*
			if (i == 34) {
				printf("Test return \n");
				return 0;
			}
*/
		}
		printf("Total find %d bad blocks\n", count);
		//nfc_ecc_set(g_nfinfo[chip].ecc, &g_nfinfo[chip]);
		count = g_nfinfo[chip].blksize/g_nfinfo[chip].oobblock;
		for (i = 0; i < (count*g_nfinfo[chip].totblk); i++) {
			if ((i%count) == 0) {
				if (isbbtbadblock(&g_nfinfo[chip], (i<<g_nfinfo[chip].page_shift))) {
					i += count;
					i--;
					continue;
				}
				printf("Compare Block%d .... ",i/count);
			}
			memset((char *)__NFC_BUF, 0, (g_nfinfo[chip].oobblock+g_nfinfo[chip].oobsize));
			rc = nfc_read_page((i<<g_nfinfo[chip].page_shift), __NFC_BUF, &g_nfinfo[chip]);
			if (rc)
				return -1;
			rc = 0;
/*
			for (j = 0; j < 24; j++) {
				if (pNFCRegs->FIFO[j] != 0xFF) {
					printf("OOB Compare failed at block%d , page%d , ofs %d = 0x%x\n",i/count,i%count,j,pNFCRegs->FIFO[j]);
					rc = -1;
					break;
				}
			}
*/
			if (rc == 0) { 
				for (j = 0; j < (g_nfinfo[chip].oobblock+g_nfinfo[chip].oobsize); j++) {
					if (*(char *)(__NFC_BUF+j) != 0xFF) {
						printf("Compare failed at block%d , page%d , ofs %d = 0x%x\n",i/count,i%count,j,*(char *)(__NFC_BUF+j));
						rc = -1;
						break;
					}
				}
			}
			if (rc) {
				//break;
				i = i/count;
				i = (i+1)*count;
				i--;
			}
			if ((i%count) == (count-1))
				printf("ok\n");
		}
	}
/*
	if (rc) {		
		printf("read error page .... ");
		nfc_ecc_set(USE_SW_ECC, &g_nfinfo[chip]);
		i = i/count;
		i = ((i+1)*count)-1;
		printf(" block%d , page%d\n",i/count,i%count);

		rc = nfc_read_page((i<<g_nfinfo[chip].page_shift), __NFC_BUF, &g_nfinfo[chip]);
	}
*/
	return 0;
}
#endif
#endif
/* (CONFIG_COMMANDS & CFG_CMD_NANDRW) */
