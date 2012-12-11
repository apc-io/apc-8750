/*
 * Copyright (c) 2008-2011 WonderMedia Technologies, Inc. All Rights Reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * WonderMedia Technologies, Inc.
 * 4F, 533, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C
 */

/* Written by Vincent Chen, WonderMedia Technologies, Inc., 2008-2011 */

#include <asm/cacheflush.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/spinlock.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include "mali.h"
#include <mach/hardware.h>

#define MALI_DEBUG 0
#define REG_MALI_BADDR (0xD8080000 + WMT_MMAP_OFFSET)

extern int wmt_getsyspara(char *varname, unsigned char *varval, int *varlen);
extern unsigned int *vpp_backup_reg(unsigned int addr, unsigned int size);
extern int vpp_restore_reg(unsigned int addr, unsigned int size,
	unsigned int *reg_ptr);

struct mali_param_s {
	int enabled;
	int buflen1; /* Dedicated phycical memory*/
	int buflen2; /* OS memory */
	int buflen3; /* UMP memory */
};

static spinlock_t mali_spinlock;

static struct mali_param_s mali_param;
static unsigned int mali_regs_save_start;   /* Mali register start address */
static unsigned int mali_regs_save_len;     /* Mali register length */
static unsigned int mali_regs_save_all = 0; /* 0 = Mali MMU only */
static unsigned int *mali_regs_save;
#if MALI_DEBUG
static unsigned int mali_regs_save_cksum;   /* Mali register checksum */
#endif

static int get_mali_param(struct mali_param_s *param);
static int mali_suspend(u32 cores);
static int mali_resume(u32 cores);
static void mali_enable_clock(int enab1le);
static void mali_enable_power(int enable);
static void mali_set_memory_base(unsigned int val);
static void mali_set_memory_size(unsigned int val);
static void mali_set_mem_validation_base(unsigned int val);
static void mali_set_mem_validation_size(unsigned int val);
static void mali_get_memory_base(unsigned int *val);
static void mali_get_memory_size(unsigned int *val);
static void mali_get_mem_validation_base(unsigned int *val);
static void mali_get_mem_validation_size(unsigned int *val);
#if MALI_DEBUG
static unsigned int mali_checksum(unsigned int *addr, int len);
static void mali_print_diff(unsigned int *src, unsigned int *dst, int len);
#endif

/* symbols for ARM's Mali.ko */
unsigned int mali_memory_base;
unsigned int mali_memory_size;
unsigned int mali_mem_validation_base;
unsigned int mali_mem_validation_size;

EXPORT_SYMBOL(mali_memory_base);
EXPORT_SYMBOL(mali_memory_size);
EXPORT_SYMBOL(mali_mem_validation_base);
EXPORT_SYMBOL(mali_mem_validation_size);

int mali_platform_init_impl(void *data);
int mali_platform_deinit_impl(void *data);
int mali_platform_powerdown_impl(u32 cores);
int mali_platform_powerup_impl(u32 cores);
void set_mali_parent_power_domain(struct platform_device* dev);

EXPORT_SYMBOL(mali_platform_init_impl);
EXPORT_SYMBOL(mali_platform_deinit_impl);
EXPORT_SYMBOL(mali_platform_powerdown_impl);
EXPORT_SYMBOL(mali_platform_powerup_impl);
EXPORT_SYMBOL(set_mali_parent_power_domain);

struct mali_device *create_mali_device(void)
{
	struct mali_device *dev;
       
	dev = kcalloc(1, sizeof(struct mali_device), GFP_KERNEL);
	dev->suspend = &mali_suspend;
	dev->resume = &mali_resume;
	dev->enable_clock = &mali_enable_clock;
	dev->enable_power = &mali_enable_power;
	dev->set_memory_base = &mali_set_memory_base;
	dev->set_memory_size = &mali_set_memory_size;
	dev->set_mem_validation_base = &mali_set_mem_validation_base;
	dev->set_mem_validation_size = &mali_set_mem_validation_size;
	dev->get_memory_base = &mali_get_memory_base;
	dev->get_memory_size = &mali_get_memory_size;
	dev->get_mem_validation_base = &mali_get_mem_validation_base;
	dev->get_mem_validation_size = &mali_get_mem_validation_size;

	return dev;
}

void release_mali_device(struct mali_device *dev)
{
	kfree(dev);
}

EXPORT_SYMBOL(create_mali_device);
EXPORT_SYMBOL(release_mali_device);

static int get_mali_param(struct mali_param_s *param)
{
	unsigned char buf[32];
	int varlen = 32;

	/* wmt.mali.param 1:-1:-1:-1 */

	if (wmt_getsyspara("wmt.mali.param", buf, &varlen) == 0) {
		sscanf(buf, "%d:%d:%d:%d", &param->enabled,
			&param->buflen1, &param->buflen2, &param->buflen3);
	} else {
		param->enabled =  1;
		param->buflen1 = -1; /* auto */
		param->buflen2 = -1; /* auto */
		param->buflen3 = -1; /* auto */
	}

	printk(KERN_INFO "wmt.mali.param = %d:%d:%d:%d\n", param->enabled,
		param->buflen1, param->buflen2, param->buflen3);

	if (param->enabled) {
		/* MBytes -> Bytes */
		if (param->buflen1 > 0)
			param->buflen1 <<= 20;
		if (param->buflen2 > 0)
			param->buflen2 <<= 20;
		if (param->buflen3 > 0)
			param->buflen3 <<= 20;
	} else {
		param->buflen1 = 0;
		param->buflen2 = 0;
		param->buflen3 = 0;
	}

	return 0;
}

static int mali_suspend(u32 cores)
{
	void *flush_start;
	void *flush_end;

#if MALI_DEBUG
	printk("mali_suspend(%d)\n", cores);
#endif

	if (wmt_power_dev(DEV_MALI, DEV_PWRSTS) != 1)
		return 0;

	spin_lock(&mali_spinlock);

	if (mali_regs_save) {
		printk("free mali_regs_save\n");
		kfree(mali_regs_save);
		mali_regs_save = NULL;
	}

	/* Mali: 0xd8080000 - 0xd8083024 */
	mali_regs_save_all = 0;

	if (mali_regs_save_all) {
		mali_regs_save_start = REG_MALI_BADDR;
		mali_regs_save_len = 0x3024;
	} else {
		mali_regs_save_start = REG_MALI_BADDR + 0x3000;
		mali_regs_save_len = 0x24;
	}

	/* Mali clock on */
	mali_enable_clock(1);

	flush_start = (void *) mali_regs_save_start;
	flush_end = flush_start + PAGE_ALIGN(mali_regs_save_len);
	dmac_flush_range(flush_start, flush_end);
	outer_flush_range(__pa(flush_start), __pa(flush_end));

	mali_regs_save = vpp_backup_reg(mali_regs_save_start,
			mali_regs_save_len);
#if MALI_DEBUG
	mali_regs_save_cksum = mali_checksum(mali_regs_save, mali_regs_save_len);
#endif

#if MALI_DEBUG
	printk("save mali registers\n");
#endif

	/* Mali clock off */
	mali_enable_clock(0);

	/* Mali power off */
	mali_enable_power(0);

	spin_unlock(&mali_spinlock);

	return 0;
}

static int mali_resume(u32 cores)
{
	void *flush_start;
	void *flush_end;
#if MALI_DEBUG
	unsigned int *regs_save;
	unsigned int cksum;
#endif

#if MALI_DEBUG
	printk("mali_resume(%d)\n", cores);
#endif

	if (wmt_power_dev(DEV_MALI, DEV_PWRSTS) == 1)
		return 0;

	spin_lock(&mali_spinlock);

	/* Mali power on */
	mali_enable_power(1);

	/* Mali clock on */
	mali_enable_clock(1);

	if (mali_regs_save) {
#if MALI_DEBUG
		regs_save =
			(unsigned int*) kmalloc(mali_regs_save_len, GFP_KERNEL);
		memcpy(regs_save, mali_regs_save, mali_regs_save_len);
#endif

		vpp_restore_reg(mali_regs_save_start,
				mali_regs_save_len,
				mali_regs_save);
		mali_regs_save = NULL;

		flush_start = (void *) mali_regs_save_start;
		flush_end = flush_start + PAGE_ALIGN(mali_regs_save_len);
		dmac_flush_range(flush_start, flush_end);
		outer_flush_range(__pa(flush_start), __pa(flush_end));

#if MALI_DEBUG
		cksum = mali_checksum((unsigned int *)mali_regs_save_start,
			mali_regs_save_len);

		printk("mali.cksum: 0x%08x -> 0x%08x\n",
			mali_regs_save_cksum, cksum);

		if (mali_regs_save_cksum != cksum)
			mali_print_diff((unsigned int *)mali_regs_save_start,
				regs_save, mali_regs_save_len);

		kfree(regs_save);
#endif
	}

	spin_unlock(&mali_spinlock);

	return 0;
}

static void mali_enable_clock(int enable)
{
	int clk_en;

	/*
	 * if your enable clock with auto_pll_divisor() twice,
	 * then you have to call it at least twice to disable clock.
	 * It is really bad.
	 */

	if (enable) {
		auto_pll_divisor(DEV_MALI, CLK_ENABLE, 0, 0);
#if MALI_DEBUG
		printk("Mali clock enabled\n");
#endif
	} else {
		do {
			clk_en = auto_pll_divisor(DEV_MALI, CLK_DISABLE, 0, 0);
		} while (clk_en);
#if MALI_DEBUG
		printk("Mali clock disabled\n");
#endif
	}
}

static void mali_enable_power(int enable)
{
	unsigned int stat;

	if (enable)
		wmt_power_dev(DEV_MALI, DEV_PWRON);
	else
		wmt_power_dev(DEV_MALI, DEV_PWROFF);

	stat = wmt_power_dev(DEV_MALI, DEV_PWRSTS);

#if MALI_DEBUG
	if (stat == 1)
		printk("Mali power enabled\n");
	else
		printk("Mali power disabled\n");
#endif
}

static void mali_set_memory_base(unsigned int val)
{
	spin_lock(&mali_spinlock);
	mali_memory_base = val;
	spin_unlock(&mali_spinlock);
}

static void mali_set_memory_size(unsigned int val)
{
	spin_lock(&mali_spinlock);
	mali_memory_size = val;
	spin_unlock(&mali_spinlock);
}

static void mali_set_mem_validation_base(unsigned int val)
{
	spin_lock(&mali_spinlock);
	mali_mem_validation_base = val;
	spin_unlock(&mali_spinlock);
}

static void mali_set_mem_validation_size(unsigned int val)
{
	spin_lock(&mali_spinlock);
	mali_mem_validation_size = val;
	spin_unlock(&mali_spinlock);
}

static void mali_get_memory_base(unsigned int *val)
{
	spin_lock(&mali_spinlock);
	*val = mali_memory_base;
	spin_unlock(&mali_spinlock);
}

static void mali_get_memory_size(unsigned int *val)
{
	spin_lock(&mali_spinlock);
	*val = mali_memory_size;
	spin_unlock(&mali_spinlock);
}

static void mali_get_mem_validation_base(unsigned int *val)
{
	spin_lock(&mali_spinlock);
	*val = mali_mem_validation_base;
	spin_unlock(&mali_spinlock);
}

static void mali_get_mem_validation_size(unsigned int *val)
{
	spin_lock(&mali_spinlock);
	*val = mali_mem_validation_size;
	spin_unlock(&mali_spinlock);
}

#if MALI_DEBUG
static unsigned int mali_checksum(unsigned int *addr, int len)
{
	unsigned int sum;

	sum = 0;

	while (len) {
		sum += *addr;
		addr++;
		len -= 4;
	}

	return sum;
}

static void mali_print_diff(unsigned int *src, unsigned int *dst, int len)
{
	while (len) {
		if (*src != *dst)
			printk("%p:0x%08x != %p:0x%08x\n",
				src, *src, dst, *dst);
		src++;
		dst++;
		len -= 4;
	}
}
#endif /* MALI_DEBUG */

/* Export functions */

int mali_platform_init_impl(void *data)
{
#if MALI_DEBUG
	printk("mali_platform_init_impl\n");
#endif
	return 0;
}

int mali_platform_deinit_impl(void *data)
{
#if MALI_DEBUG
	printk("mali_platform_deinit_impl\n");
#endif
	return 0;
}

int mali_platform_powerdown_impl(u32 cores)
{
#if MALI_DEBUG
	printk("mali_platform_powerdown_impl(%d)\n", cores);
#endif
	spin_lock(&mali_spinlock);
	mali_enable_clock(0);
	mali_enable_power(0);
	spin_unlock(&mali_spinlock);
	return 0;
}

int mali_platform_powerup_impl(u32 cores)
{
#if MALI_DEBUG
	printk("mali_platform_powerup_impl(%d)\n", cores);
#endif
	spin_lock(&mali_spinlock);
	mali_enable_power(1);
	mali_enable_clock(1);
	spin_unlock(&mali_spinlock);
	return 0;
}

void set_mali_parent_power_domain(struct platform_device* dev)
{
	/* No implemented yet */
}

static int __init mali_init(void)
{
	unsigned long smem_start;
	unsigned long smem_len;

	spin_lock_init(&mali_spinlock);
	mali_regs_save = NULL;

	smem_start = num_physpages << PAGE_SHIFT;

	get_mali_param(&mali_param);

	if (mali_param.buflen1 < 0)
		mali_param.buflen1 = 1 << 20; /* 1MB */

	smem_len = mali_param.buflen1;

	mali_set_memory_base(smem_start);
	mali_set_memory_size(smem_len);
	mali_set_mem_validation_base(0);
	mali_set_mem_validation_size(0);

	if (!mali_param.enabled)
		return -1;

	mali_enable_power(1);
	mali_enable_clock(1);

	return 0;
}

static void __exit mali_exit(void)
{
	mali_enable_clock(0);
	mali_enable_power(0);
}

module_init(mali_init);
module_exit(mali_exit);

MODULE_AUTHOR("WonderMedia Technologies, Inc.");
MODULE_DESCRIPTION("Mali PM Driver");
MODULE_LICENSE("GPL");

