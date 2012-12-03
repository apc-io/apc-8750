/*
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

/* For UBOOT */
#include <common.h>
#include <environment.h>

#include "spi_flash.h"

/* ST M25P64 CMD */
#define SF_CMD_WREN      0x06
#define SF_CMD_WRDI      0x04
#define SF_CMD_RDID      0x9F
#define SF_CMD_RDSR      0x05
#define SF_CMD_WRSR      0x01
#define SF_CMD_READ      0x03
#define SF_CMD_FAST_READ 0x0B
#define SF_CMD_PP        0x02
#define SF_CMD_SE        0xD8
#define SF_CMD_BE        0xC7
#define SF_CMD_RES       0xAB

#define SF_BIT_WR_PROT_ERR     0x20 /* [5:5] */
#define SF_BIT_MEM_REGION_ERR  0x10 /* [4:4] */
#define SF_BIT_PWR_DWN_ACC_ERR 0x8  /* [3:3] */
#define SF_BIT_PCMD_OP_ERR     0x4  /* [2:2] */
#define SF_BIT_PCMD_ACC_ERR    0x2  /* [1:1] */
#define SF_BIT_MASLOCK_ERR     0x1  /* [0:0] */

#define BIT_SEQUENCE_ERROR	0x00300030
#define BIT_TIMEOUT		0x80000000
#define ARRAY_SIZE(a)       (sizeof(a) / sizeof(a[0]))

/* SPI Interface Configuration Register(0x40) */
#define SF_MANUAL_MODE 0x40

/* SPI Programmable Command Mode Control Register(0x200) */
#define SF_RUN_CMD   0x01

/*
 * Chip ID list
 */
#define EON_MANUF      0x1C
#define NUMONYX_MANUF  0x20
#define MXIC_MANUF     0xC2
#define SPANSION_MANUF 0x01
#define SST_MANUF      0xBF
#define WB_MANUF       0xEF
#define ATMEL_MANUF    0x1F

/* EON */
#define EON_25P16_ID   0x2015 /* 2 MB */
#define EON_25P64_ID   0x2017 /* 8 MB */
#define EON_25Q64_ID   0x3017 /* 8 MB */
#define EON_25F40_ID   0x3113 /* 512 KB */
#define EON_25F16_ID   0x3115 /* 2 MB */

/* NUMONYX */
#define	NX_25P16_ID	   0x2015 /* 2 MB */
#define	NX_25P64_ID	   0x2017 /* 8 MB */

/* MXIC */
#define	MX_L512_ID     0x2010 /* 64 KB , 4KB*/
#define	MX_L1605D_ID   0x2015 /* 2 MB */
#define	MX_L3205D_ID   0x2016 /* 4 MB */
#define	MX_L6405D_ID   0x2017 /* 8 MB */
#define	MX_L1635D_ID   0x2415 /* 2 MB */
#define	MX_L3235D_ID   0x5E16 /* 4 MB */
#define	MX_L12805D_ID  0x2018 /* 16 MB */

/* SPANSION */
#define SPAN_FL016A_ID 0x0214 /* 2 MB */
#define SPAN_FL064A_ID 0x0216 /* 8 MB */

/* SST */
#define SST_VF016B_ID  0x2541 /* 2 MB */

/* WinBond */
#define WB_X16A_ID     0x3015 /* 2 MB */
#define WB_X32_ID      0x3016 /* 4 MB */
#define WB_X64_ID      0x3017 /* 8 MB */
#define WB_X128_ID     0x4018 /* 16 MB */

/* ATMEL */
#define AT_25DF041A_ID 0x4401 /* 512KB */

#define SF_IDALL(x, y)	((x<<16)|y)

struct wm_sf_dev_t {
	ulong id;
	ulong size;
	ulong sector_size;
};

struct wm_sf_dev_t sf_ids[] = {
	/* {Device ID, Total Size, Sector Size} */
	/* EON */
	{SF_IDALL(EON_MANUF, EON_25P16_ID), 0x200000, 0x10000},
	{SF_IDALL(EON_MANUF, EON_25P64_ID), 0x800000, 0x10000},
	{SF_IDALL(EON_MANUF, EON_25F40_ID), 0x80000, 0x10000},
	{SF_IDALL(EON_MANUF, EON_25F16_ID), 0x200000, 0x10000},
	{SF_IDALL(EON_MANUF, EON_25Q64_ID), 0x800000, 0x10000},
	/* NUMONYX */
	{SF_IDALL(NUMONYX_MANUF, NX_25P16_ID), 0x200000, 0x10000},
	{SF_IDALL(NUMONYX_MANUF, NX_25P64_ID), 0x800000, 0x10000},
	/* MXIC */
	{SF_IDALL(MXIC_MANUF, MX_L512_ID), 0x10000, 0x10000},
	{SF_IDALL(MXIC_MANUF, MX_L1605D_ID), 0x200000, 0x10000},
	{SF_IDALL(MXIC_MANUF, MX_L3205D_ID), 0x400000, 0x10000},
	{SF_IDALL(MXIC_MANUF, MX_L6405D_ID), 0x800000, 0x10000},
	{SF_IDALL(MXIC_MANUF, MX_L1635D_ID), 0x200000, 0x10000},
	{SF_IDALL(MXIC_MANUF, MX_L3235D_ID), 0x400000, 0x10000},
	{SF_IDALL(MXIC_MANUF, MX_L12805D_ID), 0x1000000, 0x10000},
	/* SPANSION */
	{SF_IDALL(SPANSION_MANUF, SPAN_FL016A_ID), 0x200000, 0x10000},
	{SF_IDALL(SPANSION_MANUF, SPAN_FL064A_ID), 0x800000, 0x10000},
	/* SST */
	{SF_IDALL(SST_MANUF, SST_VF016B_ID), 0x200000, 0x10000},
	/*WinBond*/
	{SF_IDALL(WB_MANUF, WB_X16A_ID), 0x200000, 0x10000},
	{SF_IDALL(WB_MANUF, WB_X32_ID), 0x400000, 0x10000},
	{SF_IDALL(WB_MANUF, WB_X64_ID), 0x800000, 0x10000},
	{SF_IDALL(WB_MANUF, WB_X128_ID), 0x1000000, 0x10000},
	{SF_IDALL(ATMEL_MANUF, AT_25DF041A_ID), 0x80000, 0x10000},
};

sfreg_t *sfreg ;

static unsigned int phy_flash_addr_0;
static unsigned int phy_flash_addr_1;

static int spi_flash_sector_erase(unsigned long addr);
static int flash_error(unsigned long code);
static int flash_env_init(void);
static int init_spi_flash(void);
extern flash_info_t flash_info_spi[CFG_MAX_SPI_FLASH_BANKS] ;/* info for FLASH chips */

#ifdef CFG_FLASH_PROTECTION
static void flash_syn_real_protect(flash_info_t *info);
#endif

/*-----------------------------------------------------------------------
 * Function
 */

#ifdef CFG_FLASH_PROTECTION
static void flash_syn_real_protect(flash_info_t *info)
{
}
#endif

#ifdef CFG_FLASH_PROTECTION
/*-----------------------------------------------------------------------
 */
int spi_flash_real_protect(flash_info_t *info, long sector, int prot)

{
	int rc = 0;
	return rc;
}
#endif

static int flash_env_init()
{
	/* SF reg base address */
	sfreg = (sfreg_t *)SF_BASE_ADDR ;

	return 0 ;
}

static int init_spi_flash()
{
	unsigned long rl_data;
	unsigned long phy_flash_size_0 = 0, phy_flash_size_1 = 0;
	int i, k;

	/* SPI module configuration */
	/* SPI Read Write Control Register @ bit 0:	SPI Flash Read Speed Field */
	sfreg->SPI_RD_WR_CTR = 0x1 ; /* read status register & set fast read command */

	for (i = 0; i < CFG_MAX_SPI_FLASH_BANKS; i++) {

		sfreg->SPI_RD_WR_CTR = 0x11 ; /* read ID */
		if (i == 0)
			flash_info_spi[i].flash_id = sfreg->SPI_MEM_0_SR_ACC ;
		else
			flash_info_spi[i].flash_id = sfreg->SPI_MEM_1_SR_ACC ;

		sfreg->SPI_RD_WR_CTR = 0x1 ; /* read status */

		for (k = 0; k < ARRAY_SIZE(sf_ids); k++) {
			if (flash_info_spi[i].flash_id == sf_ids[k].id) {
				printf("SF%1d: ManufID = %X, DeviceID = %X\n",
					i,
					flash_info_spi[i].flash_id >> 16,
					flash_info_spi[i].flash_id & 0xFFFF);
				flash_info_spi[i].size = sf_ids[k].size;
				flash_info_spi[i].sector_count = sf_ids[k].size/sf_ids[k].sector_size;
				break;
			}
			if (k == (ARRAY_SIZE(sf_ids) - 1)) {
				printf("SF%1d: ManufID = %X, DeviceID = %X (Missing or Unknown FLASH)\n",
					i,
					flash_info_spi[i].flash_id >> 16,
					flash_info_spi[i].flash_id & 0xFFFF);
				printf("     Use Default - Total size = 8MB, Sector size = 64KB\n", i);
				flash_info_spi[i].size = sf_ids[1].size;
				flash_info_spi[i].sector_count = sf_ids[1].size/sf_ids[1].sector_size;
			}
		}

		if (((flash_info_spi[i].flash_id >> 16) == ATMEL_MANUF) &&
			(sfreg->SPI_MEM_0_SR_ACC & 0x04)) {
			printf("     Global Unprotect SF%d !!\n", i);
			sfreg->SPI_INTF_CFG |= SF_MANUAL_MODE;  /* enter programmable command mode */
			sfreg->SPI_PROG_CMD_WBF[0] = SF_CMD_WREN; /* Write Enable command */
			sfreg->SPI_PROG_CMD_CTR = (0x01000000 | (i<<1)); /* set size and chip select */
			sfreg->SPI_PROG_CMD_CTR |= SF_RUN_CMD;  /* enable programmable command */
			while (sfreg->SPI_PROG_CMD_CTR & SF_RUN_CMD)
				;
			sfreg->SPI_PROG_CMD_WBF[0] = SF_CMD_WRSR; /* Write Status Register command */
			sfreg->SPI_PROG_CMD_WBF[1] = 0x00; /* Global Unprotect */
			sfreg->SPI_PROG_CMD_CTR = (0x02000000 | (i<<1)); /* set size and chip select */
			sfreg->SPI_PROG_CMD_CTR |= SF_RUN_CMD;  /* enable programmable command */
			while (sfreg->SPI_PROG_CMD_CTR & SF_RUN_CMD)
				;
			sfreg->SPI_INTF_CFG &= ~SF_MANUAL_MODE; /* exit programmable command mode */
		}
	}

	/* SPI Chip Select 0 Configuration Register */
	/* bit 31-16:	Starting Address = 16'hff80 @ bit 11-8:	Memory Size = 4'b1000 (8 MB) */
	phy_flash_addr_0 = (0xFFFFFFFF - flash_info_spi[0].size) + 1;
	phy_flash_addr_1 = phy_flash_addr_0 - flash_info_spi[1].size;
	for (i = 0; i < 32; i++) {
		if ((flash_info_spi[0].size >> i) == 0x8000)
			phy_flash_size_0 = i;
		if ((flash_info_spi[1].size >> i) == 0x8000)
			phy_flash_size_1 = i;
	}

	sfreg->CHIP_SEL_0_CFG = (phy_flash_addr_0 | (phy_flash_size_0 << 8));
	rl_data = sfreg->CHIP_SEL_0_CFG ;

	sfreg->CHIP_SEL_1_CFG = (phy_flash_addr_1 | (phy_flash_size_1 << 8));
	rl_data = sfreg->CHIP_SEL_1_CFG ;

	sfreg->SPI_INTF_CFG = 0x00030000 ;
	return 0 ;
}

static int spi_flash_sector_erase(unsigned long addr)
{
	unsigned long rl_data ;
	unsigned long temp ;
	int rc ;

	/* SPI module chip erase */
	/* SPI flash write enable control register: write enable on chip sel 0 */
	if (addr >= phy_flash_addr_0) {
		sfreg->SPI_WR_EN_CTR = SF_CS0_WR_EN ;

		/* select sector to erase */
		addr &= 0xFFFF0000 ;
		sfreg->SPI_ER_START_ADDR = addr ;

		/* SPI flash erase control register: start chip erase */
		/* Auto clear when transmit finishes. */
		sfreg->SPI_ER_CTR = SF_SEC_ER_EN ;

		/* poll status reg of chip 0 for chip erase */
		do {
			temp = sfreg->SPI_MEM_0_SR_ACC ;
			temp = sfreg->SPI_MEM_0_SR_ACC ;

			/* please SPI flash data sheet */
			if ((temp & 0x1) == 0x0)
				break ;

			rc = flash_error(sfreg->SPI_ERROR_STATUS);
			if (rc != ERR_OK)
				return rc ;

		} while (get_timer_masked() < CFG_FLASH_ERASE_TOUT);

		if (get_timer_masked() >= CFG_FLASH_ERASE_TOUT) {
			rl_data = BIT_TIMEOUT ;
			rc = flash_error(rl_data) ;
			return rc;
		}

		sfreg->SPI_WR_EN_CTR = SF_CS0_WR_DIS ;
		return ERR_OK ;
	} else {
		sfreg->SPI_WR_EN_CTR = SF_CS1_WR_EN ;

		/* select sector to erase */
		addr &= 0xFFFF0000 ;
		sfreg->SPI_ER_START_ADDR = addr ;

		/* SPI flash erase control register: start chip erase */
		/* Auto clear when transmit finishes. */
		sfreg->SPI_ER_CTR = SF_SEC_ER_EN ;

		/* poll status reg of chip 0 for chip erase */
		do {
			temp = sfreg->SPI_MEM_1_SR_ACC ;
			temp = sfreg->SPI_MEM_1_SR_ACC ;

			/* please SPI flash data sheet */
			if ((temp & 0x1) == 0x0)
				break ;

			rc = flash_error(sfreg->SPI_ERROR_STATUS);
			if (rc != ERR_OK)
				return rc ;

		} while (get_timer_masked() < CFG_FLASH_ERASE_TOUT);

		if (get_timer_masked() >= CFG_FLASH_ERASE_TOUT) {
			rl_data = BIT_TIMEOUT ;
			rc = flash_error(rl_data) ;
			return rc;
		}

		sfreg->SPI_WR_EN_CTR = SF_CS1_WR_DIS ;
		return ERR_OK ;
	}
}

static int flash_error(unsigned long code)
{
	/* check Timeout */
	if (code & BIT_TIMEOUT) {
		puts("Serial Flash Timeout\n"); /* For UBOOT */
	/*	printf("Serial Flash Timeout\n"); // For DLL */
		sfreg->SPI_ERROR_STATUS = 0x3F ; /* write 1 to clear status */
		return ERR_TIMOUT;
	}

	if (code & SF_BIT_WR_PROT_ERR) {
		puts("Serial Flash Write Protect Error\n") ; /* For UBOOT */
	/*	printf("Serial Flash Write Protect Error\n") ; // For DLL */
		sfreg->SPI_ERROR_STATUS = 0x3F ; /* write 1 to clear status */
		return ERR_PROG_ERROR;
	}

	if (code & SF_BIT_MEM_REGION_ERR) {
		puts("Serial Flash Memory Region Error\n") ; /* For UBOOT */
	/*	printf("Serial Flash Memory Region Error\n") ; // For DLL */
		sfreg->SPI_ERROR_STATUS = 0x3F ; /* write 1 to clear status */
		return ERR_PROG_ERROR;
	}

	if (code & SF_BIT_PWR_DWN_ACC_ERR) {
		puts("Serial Flash Power Down Access Error\n") ; /* For UBOOT */
	/*	printf("Serial Flash Power Down Access Error\n") ; // For DLL */
		sfreg->SPI_ERROR_STATUS = 0x3F ; /* write 1 to clear status */
		return ERR_PROG_ERROR;
	}


	if (code & SF_BIT_PCMD_OP_ERR) {
		puts("Serial Flash Program CMD OP Error\n") ; /* For UBOOT */
	/*	printf("Serial Flash Program CMD OP Error\n") ; // For DLL */
		sfreg->SPI_ERROR_STATUS = 0x3F ; /* write 1 to clear status */
		return ERR_PROG_ERROR;
	}


	if (code & SF_BIT_PCMD_ACC_ERR) {
		puts("Serial Flash Program CMD OP Access Error\n") ; /* For UBOOT */
	/*	printf("Serial Flash Program CMD OP Access Error\n") ; // For DLL */
		sfreg->SPI_ERROR_STATUS = 0x3F ; /* write 1 to clear status */
		return ERR_PROG_ERROR;
	}


	if (code & SF_BIT_MASLOCK_ERR) {
		puts("Serial Flash Master Lock Error\n") ; /* For UBOOT */
	/*	printf("Serial Flash Master Lock Error\n") ; // For DLL */
		sfreg->SPI_ERROR_STATUS = 0x3F ; /* write 1 to clear status */
		return ERR_PROG_ERROR;
	}

	/* OK, no error */
	return ERR_OK;
}


/******************************************************************************
 *
 * Public u-boot interface functions below
 *
 *****************************************************************************/

/***************************************************************************
 *
 * Flash initialization
 *
 *****************************************************************************/

unsigned long spi_flash_init(void)
{
	int i, j;
	unsigned long size = 0;

	flash_env_init() ;
	init_spi_flash() ;

	for (i = 0; i < CFG_MAX_SPI_FLASH_BANKS; i++) {
		unsigned long flashbase = 0 ;

		/* For UBOOT */
		 memset(flash_info_spi[i].protect, 0, CFG_MAX_FLASH_SECT);
		/*
			for (k = 0 ; k < 128 ; k++) // For DLL, not For UBOOT
			      flash_info_spi[i].protect[k] = 0 ;
		*/
		if (i == 0)
			flashbase = phy_flash_addr_0;
		else
			flashbase = phy_flash_addr_1;

		for (j = 0; j < flash_info_spi[i].sector_count; j++) {
			if (j >= CFG_MAX_FLASH_SECT) {
				printf("Error: Sector count of SPI Flash[%d] exceeds %d\n",
					i,
					CFG_MAX_FLASH_SECT);
				break;
			}
			flash_info_spi[i].start[j] = flashbase;

			/* uniform sector size */
			flashbase += (flash_info_spi[i].size/flash_info_spi[i].sector_count);
		}
		size += flash_info_spi[i].size;
	}

	/*
	 * Protect monitor and environment sectors
	 */
/* For UBOOT
	flash_protect (FLAG_PROTECT_SET,
			CFG_FLASH_BASE,
			CFG_FLASH_BASE + monitor_flash_len - 1,
			&flash_info_spi[0]);

	flash_protect (FLAG_PROTECT_SET,
			CFG_ENV_ADDR,
			CFG_ENV_ADDR + CFG_ENV_SIZE - 1, &flash_info_spi[0]);

#ifdef CFG_ENV_ADDR_REDUND
	flash_protect (FLAG_PROTECT_SET,
			CFG_ENV_ADDR_REDUND,
			CFG_ENV_ADDR_REDUND + CFG_ENV_SIZE_REDUND - 1,
			&flash_info_spi[0]);
#endif
*/
	return size;
}


void spi_flash_print_info(flash_info_t *info)
{
	int i;

	if (info->flash_id == FLASH_UNKNOWN) {
		printf("missing or unknown FLASH type\n");
		return;
	}

	printf("SST SPI Flash(25P64A-8MB)\n");
	printf("  Sector Start Addresses:");
	for (i = 0; i < info->sector_count; i++) {
		if ((i % 3) == 0)
			printf("\n   ");

		printf("[%3d]%08lX%s", i,
			info->start[i],
			info->protect[i] ? " (RO)" : "     ");
	}
	printf("\n");

}

int spi_flash_erase(flash_info_t *info, int s_first, int s_last)
{
	int iflag, prot, sect;
	int rc = ERR_OK;
/* For UBOOT */
#ifdef USE_920T_MMU
	int cflag;
#endif

	/* debug("flash_erase: s_first %d  s_last %d\n", s_first, s_last); */

	if ((s_first < 0) || (s_first > s_last))
		return ERR_INVAL;

 /* For UBOOT */
	prot = 0;
	for (sect = s_first; sect <= s_last; ++sect) {
		if (info->protect[sect])
			prot++;
	}

	if (prot)
		printf("\n- Warning: %d protected sectors will not be erased!\n", prot);
	else
		printf("\n");

	/*
	 * Disable interrupts which might cause a timeout
	 * here. Remember that our exception vectors are
	 * at address 0 in the flash, and we don't want a
	 * (ticker) exception to happen while the flash
	 * chip is in programming mode.
	 */

#ifdef USE_920T_MMU
	cflag = dcache_status();
	dcache_disable();
#endif
	iflag = disable_interrupts();

	/* Start erase on unprotected sectors */
	for (sect = s_first; sect <= s_last  && !ctrlc() /*For UBOOT */; sect++) {
		/* For UBOOT */
		debug("Erasing sector %2d @ %08lX... ",
		sect, info->start[sect]);

		/*  printf("Erasing sector %2d @ %08lX... ",  // For DLL */
		/*			sect, info->start[sect]); */

		/* arm simple, non interrupt dependent timer */
		/* For UBOOT */
		reset_timer_masked();

		if (info->protect[sect] == 0) {	/* not protected */
			unsigned long addr ;
			addr = info->start[sect] ;


			rc = spi_flash_sector_erase(addr) ;

			if (rc != ERR_OK)
				goto outahere;

			printf("ok.\n");
		} else {
			/* it was protected */
			printf("protected!\n");
		}
	}

outahere:
	/* allow flash to settle - wait 10 ms */
	/* For UBOOT */
	udelay_masked(10000);

	if (iflag)
		enable_interrupts();

#ifdef USE_920T_MMU
	if (cflag)
		dcache_enable();
#endif

	return rc;
}

int spi_write_buff(
	flash_info_t *info,
	unsigned char *src /*For UBOOT*/,
	unsigned long addr,
	unsigned long cnt
	)
{
	unsigned long wp;
	int iflag;
	int rc;
	unsigned char *data;

#ifdef USE_920T_MMU
	cflag = dcache_status();
	dcache_disable();
#endif
	iflag = disable_interrupts();

	sfreg->SPI_WR_EN_CTR = 0x3 ;

	rc = ERR_OK;
	wp = addr;

	data = (unsigned char *)addr;
	while (cnt) {
		*data = *src;
		cnt--;
		src++;
		data++;
	}

	sfreg->SPI_WR_EN_CTR = 0x0 ;

/* For UBOOT */
	if (iflag)
		enable_interrupts();

#ifdef USE_920T_MMU
	if (cflag)
		dcache_enable();
#endif

	return rc ;
}
