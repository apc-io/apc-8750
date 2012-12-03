/*++ 
Copyright (c) 2010 WonderMedia Technologies, Inc.

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software 
Foundation, either version 2 of the License, or (at your option) any later 
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details. You
should have received a copy of the GNU General Public License along with this
program. If not, see http://www.gnu.org/licenses/>.

WonderMedia Technologies, Inc.
10F, 529, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C.
--*/

#include <linux/config.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/ioport.h>
#include <linux/vmalloc.h>
#include <linux/init.h>
#include <linux/mtd/compatmac.h>
#include <linux/mtd/mtd.h>
#include <asm/io.h>
#include <linux/jiffies.h>//wlh added 2.4/2008
#include <linux/mtd/partitions.h>
#include <linux/platform_device.h>
#include <mach/hardware.h>

#include "wmt_nor.h"


#define DRIVER_NAME        "MTD EON NOR"
unsigned int MTDNOR_PHY_ADDR;
#define NOR_BOOT_FLASH_BASE_0 (0xFE000000)
#define NOR_BOOT_FLASH_BASE_1 (0xFE800000)
#define NOR_BOOT_FLASH_BASE_2 (0xFF000000)
#define NOR_BOOT_FLASH_BASE_3 (0xFF800000)
#define SPI_BOOT_FLASH_BASE_0 (0xEE000000)
#define SPI_BOOT_FLASH_BASE_1 (0xEE800000)
#define SPI_BOOT_FLASH_BASE_2 (0xEF000000)
#define SPI_BOOT_FLASH_BASE_3 (0xEF800000)
#define NOR_FLASH_8M 6
#define PIN_SHARE_NOR 1
#define NOR_FLASH_TYPE  0
#define NAND_FLASH_TYPE 1
#define SPI_FLASH_TYPE  2
#define BOOT_TYPE_8BIT  0
#define BOOT_TYPE_16BIT 1
#define EON_CE_PLUSE_WIDTH  20 /*20 ns*/
#define EON_READ_INITIAL_WIDTH  90 /*20 ns*/
#define NOR_CLOCK_EN	0x020000 // BIT 17
#define CONFIG_EON_FLASH    1
#define MAX_NOR_SECTOR	135
/* NOTE - CONFIG_FLASH_16BIT means the CPU interface is 16-bit, it
 *        has nothing to do with the flash chip being 8-bit or 16-bit.
 */
#define CONFIG_FLASH_16BIT

#ifdef CONFIG_FLASH_8BIT
		typedef unsigned char FLASH_PORT_WIDTH;
		typedef volatile unsigned char FLASH_PORT_WIDTHV;
		#define	FLASH_ID_MASK	   0xFF
#else
		#ifdef CONFIG_FLASH_16BIT
				typedef unsigned short FLASH_PORT_WIDTH;
			typedef volatile unsigned short FLASH_PORT_WIDTHV;
			#define SWAP(x)               __swab16(x)
	#define	FLASH_ID_MASK	   0xFFFF
		#else
	typedef unsigned long FLASH_PORT_WIDTH;
	typedef volatile unsigned long FLASH_PORT_WIDTHV;
				#define SWAP(x)               __swab32(x)
	#define	FLASH_ID_MASK	  0xFFFFFFFF
		#endif
#endif

#define FPW	FLASH_PORT_WIDTH
#define FPWV	FLASH_PORT_WIDTHV

struct wmt_nor_info_t{
		struct mtd_info *normtd;
		struct nor_reg_t *reg;
		void *io_base;
		unsigned long flash_id;
		unsigned long sector_count;
		unsigned long start[MAX_NOR_SECTOR];
		unsigned long protect[MAX_NOR_SECTOR];
};

#ifdef CONFIG_MTD_PARTITIONS

/* only bootable devices have a default partitioning */
/*static*/ struct mtd_partition nor_partitions[] = {
	{
		.name		= "filesystem-NOR",
		.offset	= 0x00000000,
		#if (CONFIG_MTD_NOR_ERASE_SIZE >= 128)
		.size		= MTDNOR_CHIP_SIZE*MTDNOR_CHIP_NUM - MTDNOR_ERASE_SIZE*32,
		#else
		.size		= MTDNOR_CHIP_SIZE*MTDNOR_CHIP_NUM - MTDNOR_ERASE_SIZE*48,
		#endif
	},
	{
		.name		= "kernel-NOR",
		#if (CONFIG_MTD_NOR_ERASE_SIZE >= 128)
		.offset	= MTDNOR_CHIP_SIZE*MTDNOR_CHIP_NUM - MTDNOR_ERASE_SIZE*32,
		.size		= MTDNOR_ERASE_SIZE*26,
		#else
		.offset	= MTDNOR_CHIP_SIZE*MTDNOR_CHIP_NUM - MTDNOR_ERASE_SIZE*48,
		.size		= MTDNOR_ERASE_SIZE*40,
		#endif
	},
	{
		.name           = "u-boot-NOR",
		#if (CONFIG_MTD_NOR_ERASE_SIZE >= 128)
		.offset         = MTDNOR_CHIP_SIZE*MTDNOR_CHIP_NUM - MTDNOR_ERASE_SIZE*6,
		.size           = MTDNOR_ERASE_SIZE*3,
		#else
		.offset         = MTDNOR_CHIP_SIZE*MTDNOR_CHIP_NUM - MTDNOR_ERASE_SIZE*8,
		.size           = MTDNOR_ERASE_SIZE*5,
		#endif
	},
	{
		.name           = "u-boot env. cfg. 1-NOR",
		.offset         = MTDNOR_CHIP_SIZE*MTDNOR_CHIP_NUM - MTDNOR_ERASE_SIZE*3,
		.size           = MTDNOR_ERASE_SIZE,
	},
	{
		.name           = "u-boot env. cfg. 2-NOR",
		.offset         = MTDNOR_CHIP_SIZE*MTDNOR_CHIP_NUM - MTDNOR_ERASE_SIZE*2,
		.size           = MTDNOR_ERASE_SIZE,
	},
	{
		.name           = "w-load-NOR",
		.offset         = MTDNOR_CHIP_SIZE*MTDNOR_CHIP_NUM - MTDNOR_ERASE_SIZE*1,
		.size           = MTDNOR_ERASE_SIZE,
	}

};

static const char *part_probes[] = { /* "RedBoot", */ "cmdlinepart", NULL };
static struct mtd_partition *parts;
#endif

#define BIT_SEQUENCE_ERROR	0x00300030
#define BIT_TIMEOUT		0x80000000

#define ERR_OK        0x0
#define ERR_TIMOUT        0x11
#define ERR_PROG_ERROR        0x22

//static unsigned int phy_flash_addr=0xff000000;
//void * nor_base_addr = NULL;

int nor_flash_sector_erase( unsigned long addr1, unsigned char* nor_base_addr)
{
	FPWV *addr;
	//unsigned char *base;		/* first address in bank */
	FPWV *base;		/* first address in bank */
	unsigned long timeout;

	REG32_VAL(PMCEU_ADDR) |= NOR_CLOCK_EN; //Turn on the clock;
	#ifdef DEBUG
		//printk("\nEnter nor_erase\n");
		//flag = disable_interrupts();
		//printk("\nThe instr->addr is 0x%x\n",addr1);
	#endif

	addr = (FPWV *)(nor_base_addr + addr1);

	#ifdef DEBUG
	//printk("\nThe addr = nor_base_addr + (FPWV *)addr1; is 0x%x\n", addr);
	#endif
	timeout = 0x30000000;

	base = (FPWV *)(nor_base_addr + MTDNOR_CHIP_SIZE*(addr1/MTDNOR_CHIP_SIZE));

	#ifdef DEBUG
	//printk("\nThe base is 0x%x\n",base);
	#endif
	local_irq_disable();
	base[0x0555] = (FPW)0x00AA00AA;	/* unlock */
	base[0x02AA] = (FPW)0x00550055;	/* unlock */
	base[0x0555] = (FPW)0x00800080;	/* erase mode */
	base[0x0555] = (FPW)0x00AA00AA;	/* unlock */
	base[0x02AA] = (FPW)0x00550055;	/* unlock */
	*addr = (FPW)0x00300030;	/* erase sector */
	local_irq_enable();
	while ((*addr & (FPW)0x00800080) != (FPW)0x00800080) {
		//printk("\nErase start *addr=0x%x!\n", *addr);
			if(!timeout--)break;
	}
	if((*addr & (FPW)0x00800080) != (FPW)0x00800080) {
			printk("\nErase error!\n");
			return 1;
	}

	return 0;//ERR_OK;
}



// We could store these in the mtd structure, but we only support 1 device..
//static struct mtd_info *mtd_info;
static int nor_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	int ret, find = 0, sector = 0, i, chip_num, chip_addr;
	struct wmt_nor_info_t *info = (struct wmt_nor_info_t*)mtd->priv;
	unsigned char *nor_base_addr = info->io_base;
	unsigned int len, sector_size;

	REG32_VAL(PMCEU_ADDR) |= NOR_CLOCK_EN; //Turn on the clock;
	len = instr->len;
	/*printk( "Nor flash_erase() instr->len 0x%x \n", (unsigned long)instr->len);
	printk( "Nor flash_erase() addr 0x%x \n", (unsigned long)instr->addr);
	printk( "Nor flash_erase() mtd->size 0x%x \n", (unsigned long)mtd->size);
	printk( "Nor flash_erase() info->sector_count %d \n", info->sector_count);
	printk( "Nor flash_erase() len = 0x%lx \n", len);*/

	chip_num = instr->addr/MTDNOR_CHIP_SIZE;
	chip_addr = instr->addr%MTDNOR_CHIP_SIZE;

	for (i = 0; i < info->sector_count; i++) {
		/*printk( "erase() info->start[i] 0x%x \n", info->start[i]);*/
		if ((chip_addr >= info->start[i]) &&
		(chip_addr < ((i+1) == info->sector_count ? MTDNOR_CHIP_SIZE : info->start[i+1]))) {
			find = 1;
			sector = i;
			break;
		}
		/*printk( "erase() info->start[i+1] 0x%x \n", info->start[i+1]);*/
	}
	if ((find != 1) || ((instr->addr + instr->len) > mtd->size) || chip_num >= MTDNOR_CHIP_NUM) {
		/*printk( "Nor flash_erase() out of mtd size, addr + len = 0x%lx \n",
		(unsigned long)instr->addr + (unsigned long)instr->len);*/
		return -EINVAL;
	}
	for (i = sector; i < info->sector_count; i++) {
		/*printk( "Nor flash_erase() addr = 0x%lx \n",
		(unsigned long)(MTDNOR_CHIP_SIZE*chip_num + info->start[i]));*/
		ret = nor_flash_sector_erase(MTDNOR_CHIP_SIZE*chip_num + info->start[i], nor_base_addr);
		if (ret != ERR_OK) {
			printk( "Nor flash_erase() error at address 0x%lx \n",
			(unsigned long)(MTDNOR_CHIP_SIZE*chip_num + info->start[i]));
			return -EINVAL;
		}
		sector_size = (((i+1) == info->sector_count) ? MTDNOR_CHIP_SIZE : info->start[i+1]) - info->start[i];
		/*printk( "Nor flash_erase() sector_size = 0x%lx \n", sector_size);*/
		len = len - sector_size;
		if (len <= 0)
			break;
	}

	instr->state = MTD_ERASE_DONE;
	mtd_erase_callback(instr);

	REG32_VAL(PMCEU_ADDR) &= ~(NOR_CLOCK_EN);//Turn off the clock

	return 0;
}

static int nor_readid(struct mtd_info *mtd)
{
	unsigned int MKid_DEVid, MKid, DEVid, DEVid1, i;
	struct wmt_nor_info_t *info = (struct wmt_nor_info_t*)mtd->priv;
	unsigned char *nor_base_addr = info->io_base;
	FPWV *base;/* first address in bank */
	base = (FPWV*)nor_base_addr;

	local_irq_disable();
	base[0x0555] = (FPW)0x00AA00AA;	/* unlock */
	base[0x02AA] = (FPW)0x00550055;	/* unlock */
	base[0x0555] = (FPW)0x00900090;	/* selects Intel or AMD */
	local_irq_enable();
	MKid = base[0x0];
	DEVid = base[0x1];
	MKid_DEVid = ((MKid&0xFFFF)<<16) + (DEVid&0xFFFF);
	printk( "nor flash MKid_DEVid 0x%x \n", MKid_DEVid);

	switch (MKid_DEVid) {

	case NX_MK_M29W640G:
	/*case NX_MK_M29W128G:*/
		info->flash_id = NX_MK_M29W640G;
		/*info->size = 0x00800000;*/
		DEVid1 = NOR_IDALL((base[0xE]&0xFFFF), (base[0xF]&0xFFFF));
		printk( "nor flash DEVid1 0x%x \n", DEVid1);

		if (NX_M29W640GT == DEVid1) {
			info->sector_count = 135;
			for (i = 0; i < info->sector_count; i++) {
				if (i < 127) {
					info->start[i] = (i * 0x10000);
					info->protect[i] = 0;
				} else {
					info->start[i] = 127 * 0x10000 + ((i-127) * 0x2000);
					info->protect[i] = 0;
				}
			}
		} else if (NX_M29W640GB == DEVid1) {
			info->sector_count = 135;
			for (i = 0; i < info->sector_count; i++) {
				if (i < 8) {
					info->start[i] = (i * 0x10000);
					info->protect[i] = 0;
				} else {
					info->start[i] = 8 * 0x10000 + ((i-8) * 0x2000);
					info->protect[i] = 0;
				}
			}
		} else if (NX_M29W640GH == DEVid1 || NX_M29W640GL == DEVid1) {
			info->sector_count = 128;
			for (i = 0; i < info->sector_count; i++) {
				info->start[i] = (i * 0x10000);
				info->protect[i] = 0;
			}
		} else if (NX_M29W128GH == DEVid1 || NX_M29W128GL == DEVid1) {
			info->sector_count = 128;
			for (i = 0; i < info->sector_count; i++) {
					info->start[i] = (i * 0x20000);
					info->protect[i] = 0;
			}
		}
		break;

	case E0N_MK_EN29LV640T:
	case MX_MK_EN29LV640T:
		info->flash_id = MKid_DEVid;
		/*info->size = 0x00800000;*/
		info->sector_count = 135;
		for (i = 0; i < info->sector_count; i++) {
			if (i < 127) {
				info->start[i] = (i * 0x10000);
				info->protect[i] = 0;
			} else {
				info->start[i] = 127 * 0x10000 + ((i-127) * 0x2000);
				info->protect[i] = 0;
			}
		}
		break;

	case E0N_MK_EN29LV640B:
		info->flash_id = E0N_MK_EN29LV640B;
		/*info->size = 0x00800000;*/
		info->sector_count = 135;
		for (i = 0; i < info->sector_count; i++) {
			if (i < 8) {
				info->start[i] = i * 0x10000;
				info->protect[i] = 0;
			} else {
				info->start[i] = 8 * 0x10000 + ((i-8) * 0x2000);
				info->protect[i] = 0;
			}
		}
		break;

	case E0N_MK_EN29LV640H:
	case E0N_MK_EN29LV640U:
		info->flash_id = MKid_DEVid;
		/*info->size = 0x00800000;*/
		info->sector_count = 128;
		for (i = 0; i < info->sector_count; i++) {
			info->start[i] = (i * 0x10000);
		}
		break;

	case MX_MK_EN29GL256E:
	info->flash_id = MKid_DEVid;
		info->sector_count = 256;
		for (i = 0; i < info->sector_count; i++) {
			info->start[i] = (i * 0x20000);
		}
		break;

	default:
		printk( "nor flash type unkonwn id = 0x%x\n", MKid_DEVid);
		info->flash_id = FLASH_UNKNOWN;
		/*info->size = 0x00800000;*/
		info->sector_count = 128;
		for (i = 0; i < info->sector_count; i++) {
			info->start[i] = (i * 0x10000);
		}
		break;
	}

	*base = (FPW)0x00F000F0;	/* NUM Read Mode reset*/
	return 0;
}

static int nor_read(struct mtd_info *mtd, loff_t from, size_t len,
			 size_t *retlen, u_char *buf)
{
	struct wmt_nor_info_t *info = (struct wmt_nor_info_t *)mtd->priv;
	unsigned char *nor_base_addr = info->io_base;

	REG32_VAL(PMCEU_ADDR) |= NOR_CLOCK_EN; //Turn on the clock;

		//printk("nor_read(pos:%x, len:%x)\n", (long)from, (long)len);
	#ifdef DEBUG
		//printk("\nEnter nor_read\n");
	#endif
	if (from + len > mtd->size) {
		printk("nor_read() out of bounds (%lx > %lx)\n", (long)(from + len), (long)mtd->size);
		return -EINVAL;
	}

	memcpy_fromio(buf,(unsigned char *)(nor_base_addr+from),len);

	*retlen=len;

	REG32_VAL(PMCEU_ADDR) &= ~(NOR_CLOCK_EN);//Turn off the clock

	return 0;
}

static int write_word_eon (FPWV *dest, FPW data,unsigned char* nor_base_addr)
{
	unsigned long j1;

	//printk("\nEnter the write_word_eon,\n");
	//printk("\nThe dest is 0x%x",dest);
	//printk("\nThe data is %d",data);
	//ulong start;
	//int flag;
	int res = 0;	/* result, assume success	*/
	//unsigned char *base;		/* first address in flash bank	*/
	FPWV *base;		/* first address in flash bank	*/

	/* Check if Flash is (sufficiently) erased */
	if ((*dest & data) != data) {
		 return (2);
	}

	base = (FPWV*)nor_base_addr;//KellyHou
	//base = (FPWV*)((unsigned int)dest&0xfe000000);
	//printk("\nThe base is 0x%x\n",base);
	/* Disable interrupts which might cause a timeout here */
	//flag = disable_interrupts();
	local_irq_disable();
	base[0x0555] = (FPW)0x00AA00AA;	/* unlock */
	base[0x02AA] = (FPW)0x00550055;	/* unlock */
	base[0x0555] = (FPW)0x00A000A0;	/* selects program mode */

	*dest = data;		/* start programming the data	*/

	/* re-enable interrupts if necessary */
	//if (flag)
	//enable_interrupts();
	local_irq_enable();
	//start = get_timer (0);

	/* data polling for D7 */
	j1=jiffies +HZ*3;
	while (time_before(jiffies,j1) && res == 0 && (*dest & (FPW)0x00800080) != (data & (FPW)0x00800080));// {
	//if (get_timer(start) > CFG_FLASH_WRITE_TOUT) {
	//  *dest = (FPW)0x00F000F0;	/* reset bank */
		//res = 1;
	//}
	//}
	if(!time_before(jiffies,j1)) {
		printk("\nWrite Time out \n");
		res=1;
	}


	return (res);
}

int nor_flash_sector_write(u_char *nor_base_addr, loff_t to, size_t len, u_char *buf)
{
	int i = 0, retlen = 0;
	unsigned char tmp_data;
	FPW data;

	//printk("nor_write(pos:0x%x, len:0x%x )\n", (long)to, (long)len);
	REG32_VAL(PMCEU_ADDR) |= NOR_CLOCK_EN; //Turn on the clock;

	#ifdef DEBUG
	//printk("\nEnter nor_write\n");
	//printk("\nto :%d\n",to);
	//printk("\nlen:%d\n",len);
	#endif

	//memcpy_toio (((u_char*)(nor_base_addr+to)), buf, len);

	for (i = 0; i < len/2; i++) {
		write_word_eon ( (FPWV *)(nor_base_addr +to+i*2), *((FPW *)(buf+i*2)),
		nor_base_addr + MTDNOR_CHIP_SIZE*((to+i*2)/MTDNOR_CHIP_SIZE));
	}

	switch((len-2*i))	{
	case 0:
		retlen = len;
		break;
	case 1:
		#ifdef DEBUG
		//printk("\ odd request len occur\n");
		#endif
		tmp_data=*((unsigned char *)(buf+(len-1)));
		data=(FPW)tmp_data;
		data &=0xffff;
		write_word_eon((FPWV *)(nor_base_addr +to+(len-1)),
		data, nor_base_addr + MTDNOR_CHIP_SIZE*((to+len-1)/MTDNOR_CHIP_SIZE));
		#ifdef DEBUG
		//printk("\nThe given data is %d,The writed data is%d\n",tmp_data,*(unsigned char *)(nor_base_addr +to+(len-1)));
		#endif
		//write_word_eon_char((volatile unsigned char *)(nor_base_addr +to+(len-1)),*((unsigned char *)(buf+(len-1))));
		//write_word_eon_char(()(nor_base_addr +to+len),*(buf+len))
		retlen = len;
		break;
		default:
		printk("\nError:(2*i-len) is %d; i is %d;len is :%d.\n",(2*i-len),i,len);
	}

	REG32_VAL(PMCEU_ADDR) &= ~(NOR_CLOCK_EN);//Turn off the clock

	return retlen;
}

static int nor_write(struct mtd_info *mtd, loff_t to, size_t len,
				size_t *retlen, const u_char *buf)
{
	struct wmt_nor_info_t *info = (struct wmt_nor_info_t*)mtd->priv;
	unsigned char *nor_base_addr = info->io_base;


	if (to + len > mtd->size)	{
		printk("nor_write() out of bounds (%ld > %ld)\n", (long)(to + len), (long)mtd->size);
		return -EINVAL;
	}

	*retlen = nor_flash_sector_write(nor_base_addr, to, len, (u_char *)buf);

	return 0;
}
/*20100709 KellyHou: Add for get boot flash type from GPIO setting */
unsigned char get_boot_flash_type(void)
{
		unsigned int val = 0;
		unsigned char rc = SPI_FLASH_TYPE;

		val = (unsigned int)GPIO_STRAP_STATUS_VAL;

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
		default:
						break;
		}

		return rc;
}
int calculate_bit(unsigned int value)
{
	int ret = 0;
	while (value) {
		value >>= 1;
		ret++;
	}
	return ret;
}
/*20100709 KellyHou: end*/

unsigned int calc_ngc_feq(void)
{
	unsigned int en_count, result;
	en_count = auto_pll_divisor(DEV_NOR, CLK_ENABLE, 0, 0);

	result = auto_pll_divisor(DEV_NOR, GET_FREQ, 0, 0);
	printk("\n The result(nor controler freq) is %d en_count=%d\n", result/1000000, en_count);//2008.2.28

	return result/1000000;
}
int init_nor_reg(struct nor_reg_t *nor_reg)
{
	unsigned int ngc_feq = calc_ngc_feq();
	unsigned int ngc_cycle= 1000 /ngc_feq;/* (1000*1000*1000)/(ngc_feq*1000*1000) , ns*/
	unsigned int ritr;
	unsigned int cetr;
	unsigned int val = 0;
	int nor_size_offset;
	unsigned char flashtype = SPI_FLASH_TYPE;

	if (ngc_cycle <= 0) {
		printk("init_nor_reg error ,the ngc_cycle is little than 0\n");
		return -1;
	}
	printk("Calc the nor flash controler work freq is %d,and the cycle is %d ns.\n",ngc_feq,ngc_cycle);

	/*Read initial cycle timing register*/
	ritr = 0;
	/*CE change turnaround timing regsiter*/
	cetr = 0;
	while (cetr*ngc_cycle<EON_CE_PLUSE_WIDTH)
	cetr++;

	cetr = cetr-1;
	if (cetr <0 )return -1;

	while (ritr*ngc_cycle<EON_READ_INITIAL_WIDTH)
	ritr++;

	cetr <<= 8;
	printk("The cetr is %d\n",cetr);
	ritr <<= 3;
	printk("The ritr is %d\n",ritr);

	nor_reg->NOR0_READ_TIMING |= (cetr|ritr);
	nor_reg->NOR1_READ_TIMING |= (cetr|ritr);
	nor_reg->NOR2_READ_TIMING |= (cetr|ritr);
	nor_reg->NOR3_READ_TIMING |= (cetr|ritr);

/*20100709 KellyHou: change for we have three flash type in WM3451*/
	val = (unsigned int)GPIO_STRAP_STATUS_VAL;//__GPIO_BASE + 0x100
	nor_reg->NOR0_CR |= 1;
	nor_reg->NOR1_CR |= 1;
	nor_reg->NOR2_CR |= 1;
	nor_reg->NOR3_CR |= 1;
	nor_size_offset = calculate_bit((unsigned int)MTDNOR_CHIP_SIZE/(1024*1024)) + 2;
	flashtype = get_boot_flash_type();
	if(flashtype == NOR_FLASH_TYPE) {
		MTDNOR_PHY_ADDR = (0xFFFFFFFF - (MTDNOR_CHIP_SIZE * MTDNOR_CHIP_NUM)) + 1;
		nor_reg->NOR_BASE_0 = ((0xFFFFFFFF - (MTDNOR_CHIP_SIZE*4))+1) | nor_size_offset;
		nor_reg->NOR_BASE_1 = ((0xFFFFFFFF - (MTDNOR_CHIP_SIZE*3))+1) | nor_size_offset;
		nor_reg->NOR_BASE_2 = ((0xFFFFFFFF - (MTDNOR_CHIP_SIZE*2))+1) | nor_size_offset;
		nor_reg->NOR_BASE_3 = ((0xFFFFFFFF - (MTDNOR_CHIP_SIZE*1))+1) | nor_size_offset;
		if ((val>>3) & 0x1) {// Serial Flash Address Size/NAND Flash Bit Width
			printk("nor boot with 16bit mode\n");
			nor_reg->NOR0_CR |= 1;
			nor_reg->NOR1_CR |= 1;
			nor_reg->NOR2_CR |= 1;
			nor_reg->NOR3_CR |= 1;
		} else {
			printk("nor boot with 8bit mode\n");
			nor_reg->NOR0_CR &= ~1;
			nor_reg->NOR1_CR &= ~1;
			nor_reg->NOR2_CR &= ~1;
			nor_reg->NOR3_CR &= ~1;
		}
	} else if(flashtype == SPI_FLASH_TYPE){
			/*MTDNOR_PHY_ADDR = (0xEFFFFFFF - (MTDNOR_CHIP_SIZE * MTDNOR_CHIP_NUM)) + 1;
			nor_reg->NOR_BASE_0 = ((0xEFFFFFFF - (MTDNOR_CHIP_SIZE*4))+1) | nor_size_offset;
			nor_reg->NOR_BASE_1 = ((0xEFFFFFFF - (MTDNOR_CHIP_SIZE*3))+1) | nor_size_offset;
			nor_reg->NOR_BASE_2 = ((0xEFFFFFFF - (MTDNOR_CHIP_SIZE*2))+1) | nor_size_offset;
			nor_reg->NOR_BASE_3 = ((0xEFFFFFFF - (MTDNOR_CHIP_SIZE*1))+1) | nor_size_offset;
		if ((val>>1) & 0x1) {
			printk("nor flash running on 16bit mode\n");
			nor_reg->NOR0_CR |= 1;
			nor_reg->NOR1_CR |= 1;
			nor_reg->NOR2_CR |= 1;
			nor_reg->NOR3_CR |= 1;
		} else {
			printk("nor flash running on 8bit mode\n");
			nor_reg->NOR0_CR &= ~1;
			nor_reg->NOR1_CR &= ~1;
			nor_reg->NOR2_CR &= ~1;
			nor_reg->NOR3_CR &= ~1;
		}*/
		// Nor and SF can not coexistance
		return -ENXIO;
	} else {// flashtype is NAND_FLASH_TYPE,or others
		// Nor and Nand can not coexistance
		return -ENXIO;
	}
/*2010-07-23 KellyHou: Add End*/
		return 0;
}

int mtdnor_init_device(struct mtd_info *mtd, unsigned long size, char *name)
{
#ifdef CONFIG_MTD_PARTITIONS
	int tmp;
#endif

	//memset(mtd, 0, sizeof(*mtd));
	mtd->name = name;
	mtd->type = MTD_NORFLASH;
	mtd->flags = MTD_CAP_NORFLASH;
	mtd->size = size;
	mtd->erasesize = MTDNOR_ERASE_SIZE;
	mtd->owner = THIS_MODULE;
	mtd->erase = nor_erase;
	mtd->read = nor_read;
	mtd->write = nor_write;
	//mtd->suspend=nor_suspend;
	//mtd->resume=nor_resume;
	mtd->writesize = 1;
	// nor reg base address
	//nor_reg = (struct nor_reg_t *)NOR_BASE_ADDR;
	//print_reg();

#ifdef CONFIG_MTD_PARTITIONS
	tmp = parse_mtd_partitions(mtd, part_probes, &parts, 0);
	add_mtd_partitions(mtd, nor_partitions, ARRAY_SIZE(nor_partitions));
#else
	if (add_mtd_device(mtd)) {
		printk(/*KERN_ERR*/ "add mtd device error\n");
		return -EIO;
	}
#endif

		return 0;
}

static int wmt_nor_probe(struct platform_device *pdev)
{

	int ret = 0;
	struct wmt_nor_info_t  *info;
	REG32_VAL(PMCEU_ADDR) |= NOR_CLOCK_EN;// 20100720 KellyHou: add to turn on the clock

	info = kzalloc(sizeof(*info),GFP_KERNEL);
	if(info == NULL) {
		dev_err( &pdev->dev,"No memeory for nor flash info\n");
		ret = -ENOMEM;
		goto exit_error;
	}
	dev_set_drvdata(&pdev->dev,info);

	info->reg = (struct nor_reg_t *)NOR_BASE_ADDR;
	if(init_nor_reg(info->reg))
		return -ENXIO;

	info->io_base = (unsigned char*)ioremap(MTDNOR_PHY_ADDR, MTDNOR_CHIP_SIZE * MTDNOR_CHIP_NUM);
	if(info->io_base == NULL)	{
		dev_err(&pdev->dev,"cannot reserve register region\n");
		ret = -EIO;
		goto exit_error;
	}

	info->normtd = kzalloc(sizeof(struct mtd_info), GFP_KERNEL);
	if (!info->normtd)
		return -ENOMEM;
	ret = mtdnor_init_device(info->normtd, (MTDNOR_CHIP_SIZE * MTDNOR_CHIP_NUM),
		"mtdnor device");
	if (ret) {
		//kfree(info->normtd);
		//info->normtd = NULL;
		goto exit_error;
	}

	info->normtd->priv = info;
		nor_readid(info->normtd);

	/*Turn off the clock*/
	REG32_VAL(PMCEU_ADDR) &= ~(NOR_CLOCK_EN);

exit_error:
		return ret;
}

static int wmt_nor_remove(struct platform_device *pdev)
{
	struct wmt_nor_info_t *info = dev_get_drvdata(&pdev->dev);

		if (info->normtd) {
				del_mtd_device(info->normtd);
				kfree(info->normtd);
		}
		if(info->io_base)
			iounmap(info->io_base);

		return 0;
}
#ifdef CONFIG_PM

int wmt_nor_suspend(struct platform_device *pdev, pm_message_t state)
{
		unsigned int boot_value;

/*2010-07-23 KellyHou: Change */
		/*Judge whether NOR boot*/
	boot_value = get_boot_flash_type();
	if(boot_value == NOR_FLASH_TYPE)
		REG32_VAL(PMCEU_ADDR) |= NOR_CLOCK_EN;
/*2010-07-23 KellyHou: Change End*/

	printk(KERN_INFO "wmt_nor_suspend\n");

	return 0;
}

int wmt_nor_resume(struct platform_device *pdev)
{
	struct wmt_nor_info_t *info = dev_get_drvdata(&pdev->dev);

	/*Turn on the clock */
	 REG32_VAL(PMCEU_ADDR) |= NOR_CLOCK_EN;
	if(info->reg) {
		if(init_nor_reg(info->reg))
			printk("WMT_nor_resume :restore error!\n");
	}
	/*Turn off the clock */
	REG32_VAL(PMCEU_ADDR) &= ~(NOR_CLOCK_EN);

	printk(KERN_INFO "wmt_nor_resume\n");

	return 0;
}

#else
#define wmt_nor_suspend NULL
#define wmt_nor_resume NULL
#endif


struct platform_driver wmt_nor_driver = {
	.driver.name	= "nor",
	.probe = wmt_nor_probe,
	.remove = wmt_nor_remove,
	.suspend = wmt_nor_suspend,
	.resume = wmt_nor_resume
};

static int __init wmt_nor_init(void)
{
		/*printk(KERN_INFO "WMT NOR Flash Driver, WonderMedia Technologies, Inc\n");*/
		return platform_driver_register(&wmt_nor_driver);
}

static void __exit wmt_nor_exit(void)
{
		platform_driver_unregister(&wmt_nor_driver);
}

module_init(wmt_nor_init);
module_exit(wmt_nor_exit);

MODULE_AUTHOR("WonderMedia Technologies, Inc.");
MODULE_DESCRIPTION("WMT [NOR] driver");
MODULE_LICENSE("GPL");
