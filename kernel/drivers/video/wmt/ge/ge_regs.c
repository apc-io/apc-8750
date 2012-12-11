/*
 * Copyright (c) 2008-2011 WonderMedia Technologies, Inc. All Rights Reserved.
 *
 * This PROPRIETARY SOFTWARE is the property of WonderMedia Technologies, Inc.
 * and may contain trade secrets and/or other confidential information of
 * WonderMedia Technologies, Inc. This file shall not be disclosed to any
 * third party, in whole or in part, without prior written consent of
 * WonderMedia.
 *
 * THIS PROPRIETARY SOFTWARE AND ANY RELATED DOCUMENTATION ARE PROVIDED
 * AS IS, WITH ALL FAULTS, AND WITHOUT WARRANTY OF ANY KIND EITHER EXPRESS
 * OR IMPLIED, AND WONDERMEDIA TECHNOLOGIES, INC. DISCLAIMS ALL EXPRESS
 * OR IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
 * QUIET ENJOYMENT OR NON-INFRINGEMENT.
 */

/* Written by Vincent Chen, WonderMedia Technologies, Inc., 2008-2011 */

#include "ge_regs.h"
#include "ge_ioctl.h"

/* GE core functions */
#ifndef INLINE
#define INLINE
#endif

#if defined(__KERNEL__)
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
DECLARE_MUTEX(ge_sem);
#else
static DEFINE_SEMAPHORE(ge_sem);
#endif
struct task_struct *ge_sem_owner;

INLINE int ge_sys_init(ge_info_t **geinfo, void *priv)
{
	ge_sem_owner = NULL;

	*geinfo = kmalloc(sizeof(ge_info_t), GFP_KERNEL);

	if (*geinfo) {
		(*geinfo)->mmio = (void *)(GE_MMIO_START + GE_MMIO_OFFSET);
		(*geinfo)->fd = 0;
		(*geinfo)->id = SCC_CHIP_ID;
		(*geinfo)->config = 0;
		(*geinfo)->data = 0;
		(*geinfo)->gmp = NULL;
		(*geinfo)->priv = NULL;
	} else
		return -1;

	return 0;
}

INLINE int ge_sys_exit(ge_info_t *geinfo, void *priv)
{
	kfree(geinfo);

	return 0;
}

INLINE int ge_get_chip_id(ge_info_t *geinfo, unsigned int *chip_id)
{
	*chip_id = geinfo->id;

	return *chip_id ? 0 : -1;
}

INLINE void ge_wait_sync(ge_info_t *geinfo)
{
	volatile ge_regs_t *regs;

	if (!geinfo) {
		printk(KERN_ERR "%s: geinfo is null.\n", __func__);
		return;
	}

	regs = geinfo->mmio;

	if (regs->ge_int_en & BIT8) {
		if (regs->ge_status & BIT2) {
			wait_event_interruptible(ge_wq,
				!(regs->ge_status & BIT2));
		}
	} else {
		while (regs->ge_status & BIT2);
		regs->ge_int_flag |= ~0;
	}
}

INLINE int ge_lock(ge_info_t *geinfo)
{
	int ret;

	if (ge_sem_owner == current)
		return -EBUSY;

	ret = down_interruptible(&ge_sem);

	if (ret == 0)
		ge_sem_owner = current;

	return ret;
}

INLINE int ge_unlock(ge_info_t *geinfo)
{
	if (ge_sem_owner == current) {
		ge_sem_owner = NULL;
		up(&ge_sem);
	} else {
		return -EACCES;
	}

	return 0;
}

INLINE int ge_trylock(ge_info_t *geinfo)
{
	int ret;

	ret = down_trylock(&ge_sem);

	if (ret == 0)
		ge_sem_owner = current;

	return ret;
}
#elif defined(__LINUX__)
INLINE int ge_sys_init(ge_info_t **geinfo, void *priv)
{
	struct fb_fix_screeninfo fb_fix;
	unsigned int off;
	unsigned long p;
	int fd;
	unsigned int id;
	size_t len;

	fd = open("/dev/fb0", O_RDWR);
	if (fd < 0)
		fd = open("/dev/graphics/fb0", O_RDWR, 0);
	if (fd < 0) {
		perror("ge_sys_init");
		return -EINVAL;
	}
	*geinfo = (ge_info_t *) calloc(1, sizeof(ge_info_t));
	if (*geinfo == NULL) {
		perror("ge_sys_init");
		return -EINVAL;
	}
	(*geinfo)->fd = fd;

	ioctl(fd, FBIOGET_FSCREENINFO, &fb_fix);

	len = fb_fix.smem_len;
	off = (len + 0x0fff) & ~0x0fff;
	p = (unsigned long)mmap(0, fb_fix.mmio_len, PROT_READ|PROT_WRITE,
		MAP_SHARED, fd, off);
	p += GE_MMIO_OFFSET;
	(*geinfo)->mmio = (void *)p;

	ioctl(fd, GEIOGET_CHIP_ID, &id);
	(*geinfo)->id = id;
	(*geinfo)->config = 0;
	(*geinfo)->data = 0;
	(*geinfo)->gmp = NULL;
	(*geinfo)->priv = NULL;

	(*geinfo)->gmp = create_gmp(0, 0);

	return 0;
}

INLINE int ge_sys_exit(ge_info_t *geinfo, void *priv)
{
	struct fb_fix_screeninfo fb_fix;
	unsigned long p;

	release_gmp(geinfo->gmp);
	release_ge_plugin(geinfo->plugin);

	ioctl(geinfo->fd, FBIOGET_FSCREENINFO, &fb_fix);

	p = (unsigned long)geinfo->mmio;
	p -= GE_MMIO_OFFSET;

	munmap((void *)p, fb_fix.mmio_len);
	close(geinfo->fd);

	free(geinfo);

	return 0;
}

INLINE int ge_get_chip_id(ge_info_t *geinfo, unsigned int *chip_id)
{
	*chip_id = geinfo->id;

	return *chip_id ? 0 : -1;
}

INLINE void ge_wait_sync(ge_info_t *geinfo)
{
	volatile ge_regs_t *regs;

	regs = (ge_regs_t *)geinfo->mmio;

	if (regs->ge_int_en & BIT8) {
		ioctl(geinfo->fd, GEIO_WAIT_SYNC, 1);
	} else {
		while (regs->ge_status & BIT2);
		regs->ge_int_flag |= ~0;
	}
}

INLINE int ge_lock(ge_info_t *geinfo)
{
	int fd;
	int ret;

	fd = geinfo->fd;
	ret = ioctl(fd, GEIO_LOCK, 1);

	return ret;
}

INLINE int ge_unlock(ge_info_t *geinfo)
{
	int fd;
	int ret;

	fd = geinfo->fd;
	ret = ioctl(fd, GEIO_LOCK, 0);

	return ret;
}

INLINE int ge_trylock(ge_info_t *geinfo)
{
	int fd;
	int ret;

	fd = geinfo->fd;
	ret = ioctl(fd, GEIO_LOCK, 2);

	return ret;
}
#elif defined(__POST__)
static ge_info_t geinfo_static;

INLINE int ge_sys_init(ge_info_t **geinfo, void *priv)
{
	*geinfo = &geinfo_static;

	(*geinfo)->fd = 0;
	(*geinfo)->mmio = (void *)(GE_MMIO_START + GE_MMIO_OFFSET);
	(*geinfo)->id = SCC_CHIP_ID;
	(*geinfo)->config = 0;
	(*geinfo)->data = 0;
	(*geinfo)->gmp = NULL;
	(*geinfo)->priv = NULL;

	return 0;
}

INLINE int ge_sys_exit(ge_info_t *geinfo, void *priv)
{
	memset(geinfo, 0, sizeof(ge_info_t));

	return 0;
}

INLINE int ge_get_chip_id(ge_info_t *geinfo, unsigned int *chip_id)
{
	*chip_id = geinfo->id;

	return *chip_id ? 0 : -1;
}

INLINE void ge_wait_sync(ge_info_t *geinfo)
{
	volatile ge_regs_t *regs;

	regs = (ge_regs_t *)geinfo->mmio;
	regs->ge_int_en = 0; /* disable interrupt */

	while (regs->ge_status & BIT2);
	regs->ge_int_flag |= ~0;
}

INLINE int ge_lock(ge_info_t *geinfo)
{
	ge_wait_sync(geinfo);
	return 0;
}

INLINE int ge_unlock(ge_info_t *geinfo)
{
	ge_wait_sync(geinfo);
	return 0;
}

INLINE int ge_trylock(ge_info_t *geinfo)
{
	ge_wait_sync(geinfo);
	return 0;
}
#endif

ge_info_t *create_ge_info(void)
{
	ge_info_t *geinfo;
	if (ge_sys_init(&geinfo, NULL) < 0)
		return NULL;
	return geinfo;
}

void release_ge_info(ge_info_t *geinfo)
{
	ge_sys_exit(geinfo, NULL);
}

/* PUT SYSTEM INDEPENDANT FUNCTIONS HERE */

INLINE int ge_set_src_csc(ge_info_t *geinfo, int cscid)
{
	volatile ge_regs_t *regs;

	regs = (ge_regs_t *)geinfo->mmio;

	switch (cscid) {
	case GE_CSC_DEFAULT:
		/* do nothing */
		break;
	case GE_CSC_RGB_YC_SDTV_16_235:
		regs->src_csc_mode = 0;
		regs->src_y_sub_16_en = 0;
		regs->src_c1_coef = 0x132;
		regs->src_c2_coef = 0x259;
		regs->src_c3_coef = 0x75;
		regs->src_c4_coef = 0x1f50;
		regs->src_c5_coef = 0x1ea5;
		regs->src_c6_coef = 0x20b;
		regs->src_c7_coef = 0x20b;
		regs->src_c8_coef = 0x1e4a;
		regs->src_c9_coef = 0x1fab;
		regs->src_coef_i = 1;
		regs->src_coef_j = 0x101;
		regs->src_coef_k = 0x101;
		break;
	case GE_CSC_RGB_YC_SDTV_0_255:
		regs->src_csc_mode = 0;
		regs->src_y_sub_16_en = 0;
		regs->src_c1_coef = 0x107;
		regs->src_c2_coef = 0x204;
		regs->src_c3_coef = 0x64;
		regs->src_c4_coef = 0x1f68;
		regs->src_c5_coef = 0x1ed6;
		regs->src_c6_coef = 0x1c2;
		regs->src_c7_coef = 0x1c2;
		regs->src_c8_coef = 0x1e78;
		regs->src_c9_coef = 0x1fb7;
		regs->src_coef_i = 0x20;
		regs->src_coef_j = 0x101;
		regs->src_coef_k = 0x101;
		break;
	case GE_CSC_RGB_YC_HDTV_16_235:
		regs->src_csc_mode = 0;
		regs->src_y_sub_16_en = 0;
		regs->src_c1_coef = 0xda;
		regs->src_c2_coef = 0x2dc;
		regs->src_c3_coef = 0x4a;
		regs->src_c4_coef = 0x1f88;
		regs->src_c5_coef = 0x1e6d;
		regs->src_c6_coef = 0x20b;
		regs->src_c7_coef = 0x20b;
		regs->src_c8_coef = 0x1e25;
		regs->src_c9_coef = 0x1fd0;
		regs->src_coef_i = 1;
		regs->src_coef_j = 0x101;
		regs->src_coef_k = 0x101;
		break;
	case GE_CSC_RGB_YC_HDTV_0_255:
		regs->src_csc_mode = 0;
		regs->src_y_sub_16_en = 0;
		regs->src_c1_coef = 0xbb;
		regs->src_c2_coef = 0x275;
		regs->src_c3_coef = 0x3f;
		regs->src_c4_coef = 0x1f99;
		regs->src_c5_coef = 0x1ea6;
		regs->src_c6_coef = 0x1c2;
		regs->src_c7_coef = 0x1c2;
		regs->src_c8_coef = 0x1e67;
		regs->src_c9_coef = 0x1fd7;
		regs->src_coef_i = 0x21;
		regs->src_coef_j = 0x101;
		regs->src_coef_k = 0x101;
		break;
	case GE_CSC_RGB_YC_JFIF_0_255:
		regs->src_csc_mode = 0;
		regs->src_y_sub_16_en = 0;
		regs->src_c1_coef = 0x132;
		regs->src_c2_coef = 0x259;
		regs->src_c3_coef = 0x75;
		regs->src_c4_coef = 0x1f53;
		regs->src_c5_coef = 0x1ead;
		regs->src_c6_coef = 0x200;
		regs->src_c7_coef = 0x200;
		regs->src_c8_coef = 0x1e53;
		regs->src_c9_coef = 0x1fad;
		regs->src_coef_i = 0x1;
		regs->src_coef_j = 0x101;
		regs->src_coef_k = 0x101;
		break;
	case GE_CSC_RGB_YC_SMPTE_170M:
		regs->src_csc_mode = 0;
		regs->src_y_sub_16_en = 0;
		regs->src_c1_coef = 0x132;
		regs->src_c2_coef = 0x259;
		regs->src_c3_coef = 0x75;
		regs->src_c4_coef = 0x1f50;
		regs->src_c5_coef = 0x1ea5;
		regs->src_c6_coef = 0x20b;
		regs->src_c7_coef = 0x20b;
		regs->src_c8_coef = 0x1e4a;
		regs->src_c9_coef = 0x1fab;
		regs->src_coef_i = 1;
		regs->src_coef_j = 0x101;
		regs->src_coef_k = 0x101;
		break;
	case GE_CSC_RGB_YC_SMPTE_240M:
		regs->src_csc_mode = 0;
		regs->src_y_sub_16_en = 0;
		regs->src_c1_coef = 0xd9;
		regs->src_c2_coef = 0x2ce;
		regs->src_c3_coef = 0x59;
		regs->src_c4_coef = 0x2f89;
		regs->src_c5_coef = 0x1e77;
		regs->src_c6_coef = 0x200;
		regs->src_c7_coef = 0x200;
		regs->src_c8_coef = 0x1e38;
		regs->src_c9_coef = 0x1fc8;
		regs->src_coef_i = 1;
		regs->src_coef_j = 0x101;
		regs->src_coef_k = 0x101;
		break;
	case GE_CSC_RGB_BYPASS:
		regs->src_csc_mode = 0;
		regs->src_y_sub_16_en = 0;
		regs->src_c1_coef = 0x400;
		regs->src_c2_coef = 0;
		regs->src_c3_coef = 0;
		regs->src_c4_coef = 0x400;
		regs->src_c5_coef = 0;
		regs->src_c6_coef = 0;
		regs->src_c7_coef = 0x400;
		regs->src_c8_coef = 0;
		regs->src_c9_coef = 0;
		regs->src_coef_i = 1;
		regs->src_coef_j = 0x101;
		regs->src_coef_k = 0x101;
		break;
	case GE_CSC_YC_RGB_SDTV_16_235:
		regs->src_csc_mode = 1;
		regs->src_y_sub_16_en = 0;
		regs->src_c1_coef = 0x400;
		regs->src_c2_coef = 0;
		regs->src_c3_coef = 0x57c;
		regs->src_c4_coef = 0x400;
		regs->src_c5_coef = 0x1ea8;
		regs->src_c6_coef = 0x1d35;
		regs->src_c7_coef = 0x400;
		regs->src_c8_coef = 0x6ee;
		regs->src_c9_coef = 0;
		regs->src_coef_i = 1;
		regs->src_coef_j = 1;
		regs->src_coef_k = 1;
		break;
	case GE_CSC_YC_RGB_SDTV_0_255:
		regs->src_csc_mode = 1;
		regs->src_y_sub_16_en = 1;
		regs->src_c1_coef = 0x4a8;
		regs->src_c2_coef = 0;
		regs->src_c3_coef = 0x662;
		regs->src_c4_coef = 0x4a8;
		regs->src_c5_coef = 0x1e70;
		regs->src_c6_coef = 0x1cbf;
		regs->src_c7_coef = 0x4a8;
		regs->src_c8_coef = 0x812;
		regs->src_c9_coef = 0;
		regs->src_coef_i = 1;
		regs->src_coef_j = 1;
		regs->src_coef_k = 1;
		break;
	case GE_CSC_YC_RGB_HDTV_16_235:
		regs->src_csc_mode = 1;
		regs->src_y_sub_16_en = 0;
		regs->src_c1_coef = 0x400;
		regs->src_c2_coef = 0;
		regs->src_c3_coef = 0x629;
		regs->src_c4_coef = 0x400;
		regs->src_c5_coef = 0x1f45;
		regs->src_c6_coef = 0x1e2a;
		regs->src_c7_coef = 0x400;
		regs->src_c8_coef = 0x744;
		regs->src_c9_coef = 0;
		regs->src_coef_i = 1;
		regs->src_coef_j = 1;
		regs->src_coef_k = 1;
		break;
	case GE_CSC_YC_RGB_HDTV_0_255:
		regs->src_csc_mode = 1;
		regs->src_y_sub_16_en = 1;
		regs->src_c1_coef = 0x4a8;
		regs->src_c2_coef = 0;
		regs->src_c3_coef = 0x72c;
		regs->src_c4_coef = 0x4a8;
		regs->src_c5_coef = 0x1f26;
		regs->src_c6_coef = 0x1ddd;
		regs->src_c7_coef = 0x4a8;
		regs->src_c8_coef = 0x876;
		regs->src_c9_coef = 0;
		regs->src_coef_i = 1;
		regs->src_coef_j = 1;
		regs->src_coef_k = 1;
		break;
	case GE_CSC_YC_RGB_JFIF_0_255:
		regs->src_csc_mode = 1;
		regs->src_y_sub_16_en = 0;
		regs->src_c1_coef = 0x400;
		regs->src_c2_coef = 0;
		regs->src_c3_coef = 0x59c;
		regs->src_c4_coef = 0x400;
		regs->src_c5_coef = 0x1ea0;
		regs->src_c6_coef = 0x1d25;
		regs->src_c7_coef = 0x400;
		regs->src_c8_coef = 0x717;
		regs->src_c9_coef = 0;
		regs->src_coef_i = 1;
		regs->src_coef_j = 1;
		regs->src_coef_k = 1;
		break;
	case GE_CSC_YC_RGB_SMPTE_170M:
		regs->src_csc_mode = 1;
		regs->src_y_sub_16_en = 0;
		regs->src_c1_coef = 0x400;
		regs->src_c2_coef = 0;
		regs->src_c3_coef = 0x57c;
		regs->src_c4_coef = 0x400;
		regs->src_c5_coef = 0x1ea8;
		regs->src_c6_coef = 0x1d35;
		regs->src_c7_coef = 0x400;
		regs->src_c8_coef = 0x6ee;
		regs->src_c9_coef = 0;
		regs->src_coef_i = 1;
		regs->src_coef_j = 1;
		regs->src_coef_k = 1;
		break;
	case GE_CSC_YC_RGB_SMPTE_240M:
		regs->src_csc_mode = 1;
		regs->src_y_sub_16_en = 0;
		regs->src_c1_coef = 0x400;
		regs->src_c2_coef = 0;
		regs->src_c3_coef = 0x64d;
		regs->src_c4_coef = 0x400;
		regs->src_c5_coef = 0x1f19;
		regs->src_c6_coef = 0x1e00;
		regs->src_c7_coef = 0x400;
		regs->src_c8_coef = 0x74f;
		regs->src_c9_coef = 0;
		regs->src_coef_i = 1;
		regs->src_coef_j = 1;
		regs->src_coef_k = 1;
		break;
	case GE_CSC_YC_BYPASS:
		regs->src_csc_mode = 1;
		regs->src_y_sub_16_en = 0;
		regs->src_c1_coef = 400;
		regs->src_c2_coef = 0;
		regs->src_c3_coef = 0;
		regs->src_c4_coef = 400;
		regs->src_c5_coef = 0;
		regs->src_c6_coef = 0;
		regs->src_c7_coef = 400;
		regs->src_c8_coef = 0;
		regs->src_c9_coef = 0;
		regs->src_coef_i = 1;
		regs->src_coef_j = 1;
		regs->src_coef_k = 1;
		break;
	}

	return 0;
}

INLINE int ge_set_dst_csc(ge_info_t *geinfo, int cscid)
{
	volatile ge_regs_t *regs;

	regs = (ge_regs_t *)geinfo->mmio;

	switch (cscid) {
	case GE_CSC_DEFAULT:
		/* do nothing */
		break;
	case GE_CSC_RGB_YC_SDTV_16_235:
		regs->des_csc_mode = 0;
		regs->des_y_sub_16_en = 0;
		regs->des_c1_coef = 0x132;
		regs->des_c2_coef = 0x259;
		regs->des_c3_coef = 0x75;
		regs->des_c4_coef = 0x1f50;
		regs->des_c5_coef = 0x1ea5;
		regs->des_c6_coef = 0x20b;
		regs->des_c7_coef = 0x20b;
		regs->des_c8_coef = 0x1e4a;
		regs->des_c9_coef = 0x1fab;
		regs->des_coef_i = 1;
		regs->des_coef_j = 0x101;
		regs->des_coef_k = 0x101;
		break;
	case GE_CSC_RGB_YC_SDTV_0_255:
		regs->des_csc_mode = 0;
		regs->des_y_sub_16_en = 0;
		regs->des_c1_coef = 0x107;
		regs->des_c2_coef = 0x204;
		regs->des_c3_coef = 0x64;
		regs->des_c4_coef = 0x1f68;
		regs->des_c5_coef = 0x1ed6;
		regs->des_c6_coef = 0x1c2;
		regs->des_c7_coef = 0x1c2;
		regs->des_c8_coef = 0x1e78;
		regs->des_c9_coef = 0x1fb7;
		regs->des_coef_i = 0x20;
		regs->des_coef_j = 0x101;
		regs->des_coef_k = 0x101;
		break;
	case GE_CSC_RGB_YC_HDTV_16_235:
		regs->des_csc_mode = 0;
		regs->des_y_sub_16_en = 0;
		regs->des_c1_coef = 0xda;
		regs->des_c2_coef = 0x2dc;
		regs->des_c3_coef = 0x4a;
		regs->des_c4_coef = 0x1f88;
		regs->des_c5_coef = 0x1e6d;
		regs->des_c6_coef = 0x20b;
		regs->des_c7_coef = 0x20b;
		regs->des_c8_coef = 0x1e25;
		regs->des_c9_coef = 0x1fd0;
		regs->des_coef_i = 1;
		regs->des_coef_j = 0x101;
		regs->des_coef_k = 0x101;
		break;
	case GE_CSC_RGB_YC_HDTV_0_255:
		regs->des_csc_mode = 0;
		regs->des_y_sub_16_en = 0;
		regs->des_c1_coef = 0xbb;
		regs->des_c2_coef = 0x275;
		regs->des_c3_coef = 0x3f;
		regs->des_c4_coef = 0x1f99;
		regs->des_c5_coef = 0x1ea6;
		regs->des_c6_coef = 0x1c2;
		regs->des_c7_coef = 0x1c2;
		regs->des_c8_coef = 0x1e67;
		regs->des_c9_coef = 0x1fd7;
		regs->des_coef_i = 0x21;
		regs->des_coef_j = 0x101;
		regs->des_coef_k = 0x101;
		break;
	case GE_CSC_RGB_YC_JFIF_0_255:
		regs->des_csc_mode = 0;
		regs->des_y_sub_16_en = 0;
		regs->des_c1_coef = 0x132;
		regs->des_c2_coef = 0x259;
		regs->des_c3_coef = 0x75;
		regs->des_c4_coef = 0x1f53;
		regs->des_c5_coef = 0x1ead;
		regs->des_c6_coef = 0x200;
		regs->des_c7_coef = 0x200;
		regs->des_c8_coef = 0x1e53;
		regs->des_c9_coef = 0x1fad;
		regs->des_coef_i = 0x1;
		regs->des_coef_j = 0x101;
		regs->des_coef_k = 0x101;
		break;
	case GE_CSC_RGB_YC_SMPTE_170M:
		regs->des_csc_mode = 0;
		regs->des_y_sub_16_en = 0;
		regs->des_c1_coef = 0x132;
		regs->des_c2_coef = 0x259;
		regs->des_c3_coef = 0x75;
		regs->des_c4_coef = 0x1f50;
		regs->des_c5_coef = 0x1ea5;
		regs->des_c6_coef = 0x20b;
		regs->des_c7_coef = 0x20b;
		regs->des_c8_coef = 0x1e4a;
		regs->des_c9_coef = 0x1fab;
		regs->des_coef_i = 1;
		regs->des_coef_j = 0x101;
		regs->des_coef_k = 0x101;
		break;
	case GE_CSC_RGB_YC_SMPTE_240M:
		regs->des_csc_mode = 0;
		regs->des_y_sub_16_en = 0;
		regs->des_c1_coef = 0xd9;
		regs->des_c2_coef = 0x2ce;
		regs->des_c3_coef = 0x59;
		regs->des_c4_coef = 0x2f89;
		regs->des_c5_coef = 0x1e77;
		regs->des_c6_coef = 0x200;
		regs->des_c7_coef = 0x200;
		regs->des_c8_coef = 0x1e38;
		regs->des_c9_coef = 0x1fc8;
		regs->des_coef_i = 1;
		regs->des_coef_j = 0x101;
		regs->des_coef_k = 0x101;
		break;
	case GE_CSC_RGB_BYPASS:
		regs->des_csc_mode = 0;
		regs->des_y_sub_16_en = 0;
		regs->des_c1_coef = 0x400;
		regs->des_c2_coef = 0;
		regs->des_c3_coef = 0;
		regs->des_c4_coef = 0x400;
		regs->des_c5_coef = 0;
		regs->des_c6_coef = 0;
		regs->des_c7_coef = 0x400;
		regs->des_c8_coef = 0;
		regs->des_c9_coef = 0;
		regs->des_coef_i = 1;
		regs->des_coef_j = 0x101;
		regs->des_coef_k = 0x101;
		break;
	case GE_CSC_YC_RGB_SDTV_16_235:
		regs->des_csc_mode = 1;
		regs->des_y_sub_16_en = 0;
		regs->des_c1_coef = 0x400;
		regs->des_c2_coef = 0;
		regs->des_c3_coef = 0x57c;
		regs->des_c4_coef = 0x400;
		regs->des_c5_coef = 0x1ea8;
		regs->des_c6_coef = 0x1d35;
		regs->des_c7_coef = 0x400;
		regs->des_c8_coef = 0x6ee;
		regs->des_c9_coef = 0;
		regs->des_coef_i = 1;
		regs->des_coef_j = 1;
		regs->des_coef_k = 1;
		break;
	case GE_CSC_YC_RGB_SDTV_0_255:
		regs->des_csc_mode = 1;
		regs->des_y_sub_16_en = 1;
		regs->des_c1_coef = 0x4a8;
		regs->des_c2_coef = 0;
		regs->des_c3_coef = 0x662;
		regs->des_c4_coef = 0x4a8;
		regs->des_c5_coef = 0x1e70;
		regs->des_c6_coef = 0x1cbf;
		regs->des_c7_coef = 0x4a8;
		regs->des_c8_coef = 0x812;
		regs->des_c9_coef = 0;
		regs->des_coef_i = 1;
		regs->des_coef_j = 1;
		regs->des_coef_k = 1;
		break;
	case GE_CSC_YC_RGB_HDTV_16_235:
		regs->des_csc_mode = 1;
		regs->des_y_sub_16_en = 0;
		regs->des_c1_coef = 0x400;
		regs->des_c2_coef = 0;
		regs->des_c3_coef = 0x629;
		regs->des_c4_coef = 0x400;
		regs->des_c5_coef = 0x1f45;
		regs->des_c6_coef = 0x1e2a;
		regs->des_c7_coef = 0x400;
		regs->des_c8_coef = 0x744;
		regs->des_c9_coef = 0;
		regs->des_coef_i = 1;
		regs->des_coef_j = 1;
		regs->des_coef_k = 1;
		break;
	case GE_CSC_YC_RGB_HDTV_0_255:
		regs->des_csc_mode = 1;
		regs->des_y_sub_16_en = 1;
		regs->des_c1_coef = 0x4a8;
		regs->des_c2_coef = 0;
		regs->des_c3_coef = 0x72c;
		regs->des_c4_coef = 0x4a8;
		regs->des_c5_coef = 0x1f26;
		regs->des_c6_coef = 0x1ddd;
		regs->des_c7_coef = 0x4a8;
		regs->des_c8_coef = 0x876;
		regs->des_c9_coef = 0;
		regs->des_coef_i = 1;
		regs->des_coef_j = 1;
		regs->des_coef_k = 1;
		break;
	case GE_CSC_YC_RGB_JFIF_0_255:
		regs->des_csc_mode = 1;
		regs->des_y_sub_16_en = 0;
		regs->des_c1_coef = 0x400;
		regs->des_c2_coef = 0;
		regs->des_c3_coef = 0x59c;
		regs->des_c4_coef = 0x400;
		regs->des_c5_coef = 0x1ea0;
		regs->des_c6_coef = 0x1d25;
		regs->des_c7_coef = 0x400;
		regs->des_c8_coef = 0x717;
		regs->des_c9_coef = 0;
		regs->des_coef_i = 1;
		regs->des_coef_j = 1;
		regs->des_coef_k = 1;
		break;
	case GE_CSC_YC_RGB_SMPTE_170M:
		regs->des_csc_mode = 1;
		regs->des_y_sub_16_en = 0;
		regs->des_c1_coef = 0x400;
		regs->des_c2_coef = 0;
		regs->des_c3_coef = 0x57c;
		regs->des_c4_coef = 0x400;
		regs->des_c5_coef = 0x1ea8;
		regs->des_c6_coef = 0x1d35;
		regs->des_c7_coef = 0x400;
		regs->des_c8_coef = 0x6ee;
		regs->des_c9_coef = 0;
		regs->des_coef_i = 1;
		regs->des_coef_j = 1;
		regs->des_coef_k = 1;
		break;
	case GE_CSC_YC_RGB_SMPTE_240M:
		regs->des_csc_mode = 1;
		regs->des_y_sub_16_en = 0;
		regs->des_c1_coef = 0x400;
		regs->des_c2_coef = 0;
		regs->des_c3_coef = 0x64d;
		regs->des_c4_coef = 0x400;
		regs->des_c5_coef = 0x1f19;
		regs->des_c6_coef = 0x1e00;
		regs->des_c7_coef = 0x400;
		regs->des_c8_coef = 0x74f;
		regs->des_c9_coef = 0;
		regs->des_coef_i = 1;
		regs->des_coef_j = 1;
		regs->des_coef_k = 1;
		break;
	case GE_CSC_YC_BYPASS:
		regs->des_csc_mode = 1;
		regs->des_y_sub_16_en = 0;
		regs->des_c1_coef = 400;
		regs->des_c2_coef = 0;
		regs->des_c3_coef = 0;
		regs->des_c4_coef = 400;
		regs->des_c5_coef = 0;
		regs->des_c6_coef = 0;
		regs->des_c7_coef = 400;
		regs->des_c8_coef = 0;
		regs->des_c9_coef = 0;
		regs->des_coef_i = 1;
		regs->des_coef_j = 1;
		regs->des_coef_k = 1;
		break;
	}

	return 0;
}

static int ge_set_src_pixelformat_8710(ge_info_t *geinfo, unsigned int pixelformat)
{
	volatile struct ge_regs_8710 *regs;
	unsigned int major;
	unsigned int minor;
	int ret = 0;

	regs = (struct ge_regs_8710 *)geinfo->mmio;

	major = geinfo->id >> 16;
	minor = geinfo->id & 0xffff;

	switch (pixelformat) {
	case GSPF_RGB16:
		regs->src_indep_mode = 0x14; /* 10100b */
		break;
	case GSPF_RGB555:
		regs->src_indep_mode = 0x15; /* 10101b */
		break;
	case GSPF_RGB454:
		regs->src_indep_mode = 0x16; /* 10110b */
		break;
	case GSPF_RGB32:
	case GSPF_ARGB:
		regs->src_indep_mode = 0x1c; /* 11100b */
		break;
	default:
		regs->src_indep_mode = 0;
		ret = -1;
	}

	return ret;
}

static int ge_set_src_pixelformat_8850(ge_info_t *geinfo, unsigned int pixelformat)
{
	volatile ge_regs_t *regs;
	unsigned int major;
	unsigned int minor;
	int ret = 0;

	regs = (ge_regs_t *)geinfo->mmio;

	major = geinfo->id >> 16;
	minor = geinfo->id & 0xffff;

	switch (pixelformat) {
	case GSPF_RGB16:
		regs->src_yuv_en = 0;
		regs->src_color_depth = 1; /* 16 bpp */
		regs->src_hm_sel = 0;      /* 565 */
		regs->src_input_sel = 0;
		break;
	case GSPF_RGB555:
		regs->src_yuv_en = 0;
		regs->src_color_depth = 1; /* 16 bpp */
		regs->src_hm_sel = 1;      /* 555 */
		regs->src_input_sel = 0;
		break;
	case GSPF_RGB454:
		regs->src_yuv_en = 0;
		regs->src_color_depth = 1; /* 16 bpp */
		regs->src_hm_sel = 2;      /* 454 */
		regs->src_input_sel = 0;
		break;
	case GSPF_RGB32:
	case GSPF_ARGB:
		regs->src_yuv_en = 0;
		regs->src_color_depth = 3; /* 32 bpp */
		regs->src_input_sel = 0;
		break;
	case GSPF_YUV420SP:
		ge_set_src_csc(geinfo, GE_CSC_YC_RGB_JFIF_0_255);
		regs->src_yuv_en = 1;
		regs->src_fmt444 = 0;
		regs->src_vfmt = 1;
		regs->src_csc_en = 1;
		regs->src_color_depth = 3; /* 32 bpp */
		regs->src_input_sel = 0;
		break;
	case GSPF_YUV422SP:
		ge_set_src_csc(geinfo, GE_CSC_YC_RGB_JFIF_0_255);
		regs->src_yuv_en = 1;
		regs->src_fmt444 = 0;
		regs->src_vfmt = 0;
		regs->src_csc_en = 1;
		regs->src_color_depth = 3; /* 32 bpp */
		regs->src_input_sel = 0;
		break;
	case GSPF_YUV444SP:
		ge_set_src_csc(geinfo, GE_CSC_YC_RGB_JFIF_0_255);
		regs->src_yuv_en = 1;
		regs->src_fmt444 = 1;
		regs->src_csc_en = 1;
		regs->src_color_depth = 3; /* 32 bpp */
		regs->src_input_sel = 0;
		break;
	case GSPF_VPU:
		ge_set_src_csc(geinfo, GE_CSC_YC_RGB_JFIF_0_255);
		regs->src_yuv_en = 0;
		regs->src_csc_en = 1;
		regs->src_color_depth = 3; /* 32 bpp */
		regs->src_input_sel = 1;
		break;
	default:
		regs->src_color_depth = 3; /* 32 bpp */
		regs->src_input_sel = 0;
		ret = -1;
	}

	return ret;
}

INLINE int ge_set_src_pixelformat(ge_info_t *geinfo, unsigned int pixelformat)
{
	volatile ge_regs_t *regs;
	unsigned int major;
	unsigned int minor;

	regs = (ge_regs_t *)geinfo->mmio;

	major = geinfo->id >> 16;
	minor = geinfo->id & 0xffff;

	if (major == 0x3445) /* WM8710 B0 */
		return ge_set_src_pixelformat_8710(geinfo, pixelformat);

	return ge_set_src_pixelformat_8850(geinfo, pixelformat);
}

static int ge_set_dst_pixelformat_8710(ge_info_t *geinfo, unsigned int pixelformat)
{
	volatile struct ge_regs_8710 *regs;
	int ret = 0;

	regs = (struct ge_regs_8710 *)geinfo->mmio;

	switch (pixelformat) {
	case GSPF_LUT8:
		regs->color_depth = 0;
		break;
	case GSPF_RGB16:
		regs->color_depth = 1;
		regs->hm_sel = 0;
		break;
	case GSPF_RGB555:
		regs->color_depth = 1;
		regs->hm_sel = 1;
		break;
	case GSPF_RGB454:
		regs->color_depth = 1;
		regs->hm_sel = 2;
		break;
	case GSPF_RGB32:
	case GSPF_ARGB:
		regs->color_depth = 3;
		break;
	default:
		ret = -1;
	}

	return ret;
}

static int ge_set_dst_pixelformat_8850(ge_info_t *geinfo, unsigned int pixelformat)
{
	volatile ge_regs_t *regs;
	unsigned int major;
	unsigned int minor;
	int ret = 0;

	regs = (ge_regs_t *)geinfo->mmio;

	major = geinfo->id >> 16;
	minor = geinfo->id & 0xffff;

	switch (pixelformat) {
	case GSPF_RGB16:
		regs->des_yuv_en = 0;
		regs->des_color_depth = 1; /* 16 bpp */
		regs->des_hm_sel = 0;      /* 565 */
		break;
	case GSPF_RGB555:
		regs->des_yuv_en = 0;
		regs->des_color_depth = 1; /* 16 bpp */
		regs->des_hm_sel = 1;      /* 555 */
		break;
	case GSPF_RGB454:
		regs->des_yuv_en = 0;
		regs->des_color_depth = 1; /* 16 bpp */
		regs->des_hm_sel = 2;      /* 565 */
		break;
	case GSPF_RGB32:
	case GSPF_ARGB:
		regs->des_yuv_en = 0;
		regs->des_color_depth = 3; /* 32 bpp */
		break;
	case GSPF_YUV420SP:
		ge_set_dst_csc(geinfo, GE_CSC_YC_RGB_JFIF_0_255);
		regs->des_yuv_en = 1;
		regs->des_fmt444 = 0;
		regs->des_vfmt = 1;
		regs->des_csc_en = 1;
		regs->des_color_depth = 3; /* 32 bpp */
		break;
	case GSPF_YUV422SP:
		ge_set_dst_csc(geinfo, GE_CSC_YC_RGB_JFIF_0_255);
		regs->des_yuv_en = 1;
		regs->des_fmt444 = 0;
		regs->des_vfmt = 0;
		regs->des_csc_en = 1;
		regs->des_color_depth = 3; /* 32 bpp */
		break;
	case GSPF_YUV444SP:
		ge_set_dst_csc(geinfo, GE_CSC_YC_RGB_JFIF_0_255);
		regs->des_yuv_en = 1;
		regs->des_fmt444 = 1;
		regs->des_csc_en = 1;
		regs->des_color_depth = 3; /* 32 bpp */
		break;
	case GSPF_VPU:
		/* N/A. Only source path supports VPU */
	default:
		regs->des_yuv_en = 0;
		regs->des_csc_en = 0;
		regs->des_color_depth = 3; /* 32 bpp */
		ret = -1;
	}

	return ret;
}

INLINE int ge_set_dst_pixelformat(ge_info_t *geinfo, unsigned int pixelformat)
{
	volatile ge_regs_t *regs;
	unsigned int major;
	unsigned int minor;

	regs = (ge_regs_t *)geinfo->mmio;

	major = geinfo->id >> 16;
	minor = geinfo->id & 0xffff;

	if (major == 0x3445) /* WM8710 B0 */
		return ge_set_dst_pixelformat_8710(geinfo, pixelformat);

	return ge_set_dst_pixelformat_8850(geinfo, pixelformat);
}

INLINE int ge_set_dw_pixelformat(ge_info_t *geinfo, unsigned int pixelformat)
{
	volatile ge_regs_t *regs;
	unsigned int major;
	unsigned int minor;
	int ret = 0;

	regs = (ge_regs_t *)geinfo->mmio;

	major = geinfo->id >> 16;
	minor = geinfo->id & 0xffff;

	if (major == 0x3445) /* WM8710 B0 */
		return 0;

	/* IGS: 0:888, 1:555, 2:666, 3:565, 4:bypass, 5:454 */

	switch (pixelformat) {
	case GSPF_RGB16:
		regs->src_igs_mode = 3;   /* 565 */
		regs->des_igs_mode = 3;   /* 565 */
		regs->dw_color_depth = 1; /* 16 bpp */
		regs->dw_hm_sel = 0;      /* 565 */
		break;
	case GSPF_RGB555:
		regs->src_igs_mode = 1;   /* 555 */
		regs->des_igs_mode = 1;   /* 555 */
		regs->dw_color_depth = 1; /* 16 bpp */
		regs->dw_hm_sel = 1;      /* 555 */
		break;
	case GSPF_RGB454:
		regs->src_igs_mode = 5;   /* 454 */
		regs->des_igs_mode = 5;   /* 454 */
		regs->dw_color_depth = 1; /* 16 bpp */
		regs->dw_hm_sel = 2;      /* 454 */
		break;
	case GSPF_RGB32:
	case GSPF_ARGB:
		regs->src_igs_mode = 0;   /* 888 */
		regs->des_igs_mode = 0;   /* 888 */
		regs->dw_color_depth = 3; /* 32 bpp */
		break;
	case GSPF_YUV420SP:
	case GSPF_YUV422SP:
	case GSPF_YUV444SP:
	case GSPF_VPU:
		regs->src_igs_mode = 0;   /* 888 */
		regs->des_igs_mode = 0;   /* 888 */
		regs->dw_color_depth = 3; /* 32 bpp */
		break;
	default:
		ret = -1;
	}

	return ret;
}

INLINE int ge_set_direct_write(ge_info_t *geinfo, ge_surface_t *dw)
{
	volatile ge_regs_t *regs;
	unsigned int major;
	unsigned int minor;

	regs = (ge_regs_t *)geinfo->mmio;

	major = geinfo->id >> 16;
	minor = geinfo->id & 0xffff;

	if (major == 0x3445) /* WM8710 B0 */
		return 0;

	regs->dw_baddr = dw->addr;
	regs->dw_disp_w = dw->xres_virtual - 1;
	regs->dw_disp_h = dw->yres_virtual - 1;

	regs->dw_x_start = dw->x;
	regs->dw_y_start = dw->y;
	regs->dw_width = dw->xres - 1;
	regs->dw_height = dw->yres - 1;

	return ge_set_dw_pixelformat(geinfo, dw->pixelformat);
}


INLINE int ge_set_destination(ge_info_t *geinfo, ge_surface_t *dst)
{
	volatile ge_regs_t *regs;

	regs = (ge_regs_t *)geinfo->mmio;

	regs->des_baddr = dst->addr;
	regs->des_disp_w = dst->xres_virtual - 1;
	regs->des_disp_h = dst->yres_virtual - 1;

	regs->des_x_start = dst->x;
	regs->des_y_start = dst->y;
	regs->des_width = dst->xres - 1;
	regs->des_height = dst->yres - 1;

	ge_set_dst_pixelformat(geinfo, dst->pixelformat);

	return ge_set_direct_write(geinfo, dst);
}

INLINE int ge_set_source(ge_info_t *geinfo, ge_surface_t *src)
{
	volatile ge_regs_t *regs;

	regs = (ge_regs_t *)geinfo->mmio;

	regs->src_baddr = src->addr;
	regs->src_disp_w = src->xres_virtual - 1;
	regs->src_disp_h = src->yres_virtual - 1;

	regs->src_x_start = src->x;
	regs->src_y_start = src->y;
	regs->src_width = src->xres - 1;
	regs->src_height = src->yres - 1;

	ge_set_src_pixelformat(geinfo, src->pixelformat);

	return 0;
}

INLINE int ge_set_govr_source(ge_info_t *geinfo, ge_surface_t *src, int x, int y)
{
	volatile ge_regs_t *regs;

	regs = (ge_regs_t *)geinfo->mmio;
	ge_set_source(geinfo, src);
	regs->src_igs_mode = 0;   /* 888 */
	regs->govr_src_x_start = x;
	regs->govr_src_y_start = y;
	regs->govr_src_en = src->pixelformat == GSPF_VPU ? 0 : 1;

	return 0;
}

INLINE int ge_set_govr_destination(ge_info_t *geinfo, ge_surface_t *dst, int x, int y)
{
	volatile ge_regs_t *regs;

	regs = (ge_regs_t *)geinfo->mmio;
	ge_set_destination(geinfo, dst);
	regs->des_igs_mode = 0;   /* 888 */
	regs->govr_des_x_start = 0;
	regs->govr_des_y_start = 0;
	regs->govr_des_en = dst->pixelformat == GSPF_VPU ? 0 : 1;

	return 0;
}

INLINE int ge_set_govr_size(ge_info_t *geinfo, int w, int h)
{
	volatile ge_regs_t *regs;

	regs = (ge_regs_t *)geinfo->mmio;
	regs->dw_color_depth = 3; /* 32 bpp */
	regs->dw_disp_w = w - 1;
	regs->dw_disp_h = h - 1;

	return 0;
}

INLINE int ge_set_mask(ge_info_t *geinfo, ge_surface_t *src)
{
	volatile ge_regs_t *regs;

	regs = (ge_regs_t *)geinfo->mmio;

	regs->mask_baddr = src->addr;
	regs->mask_disp_w = src->xres_virtual - 1;
	regs->mask_disp_h = src->yres_virtual - 1;

	regs->mask_x_start = src->x;
	regs->mask_y_start = src->y;
	regs->mask_width = src->xres - 1;
	regs->mask_height = src->yres - 1;

	return 0;
}

INLINE int ge_set_command(ge_info_t *geinfo, unsigned int cmd, unsigned int rop)
{
	volatile ge_regs_t *regs;

	regs = (ge_regs_t *)geinfo->mmio;

	regs->ge_command = cmd;
	regs->rop_code = rop;	/* cc = s->d, 0f = p->d */
	regs->ge_eng_en = 1;
	regs->ge_fire = 1;

	return 0;
}

INLINE unsigned int ge_get_color(ge_info_t *geinfo,
	unsigned int r, unsigned int g, unsigned int b, unsigned int a,
	unsigned int pixfmt)
{
	unsigned int color;

	switch (pixfmt) {
	case GSPF_LUT8:
		color = a;
		break;
	case GSPF_RGB16:
		color = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
		break;
	case GSPF_RGB555:
		color = ((r >> 3) << 10) | ((g >> 3) << 5) | (b >> 3);
		break;
	case GSPF_RGB454:
		color = ((r >> 4) << 9) | ((g >> 3) << 4) | (b >> 4);
		break;
	case GSPF_RGB32:
	case GSPF_ARGB:
		color = (a << 24) | (r << 16) | (g << 8) | (b) ;
		break;
	default:
		color = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
		break;
	}

	return color;
}

INLINE void ge_set_color(ge_info_t *geinfo,
	unsigned int r, unsigned int g, unsigned int b, unsigned int a,
	unsigned int pixfmt)
{
	volatile ge_regs_t *regs;
	unsigned int color;

	color = ge_get_color(geinfo, r, g, b, a, pixfmt);

	regs = (ge_regs_t *)geinfo->mmio;
	regs->pat0_color = color;
}

INLINE void ge_set_sck(ge_info_t *geinfo,
	unsigned int r, unsigned int g, unsigned int b, unsigned int a,
	unsigned int pixfmt)
{
	volatile ge_regs_t *regs;
	unsigned int color;

	color = ge_get_color(geinfo, r, g, b, a, pixfmt);

	regs = (ge_regs_t *)geinfo->mmio;
	regs->src_ck = color;
	regs->ck_sel |= BIT2;
}

INLINE void ge_set_dck(ge_info_t *geinfo,
	unsigned int r, unsigned int g, unsigned int b, unsigned int a,
	unsigned int pixfmt)
{
	volatile ge_regs_t *regs;
	unsigned int color;

	color = ge_get_color(geinfo, r, g, b, a, pixfmt);

	regs = (ge_regs_t *)geinfo->mmio;
	regs->des_ck = color;
	regs->ck_sel |= BIT3 | BIT1;
}

INLINE int ge_cmd_blit(ge_info_t *geinfo)
{
	return ge_set_command(geinfo, GECMD_BLIT, 0xcc);
}

INLINE int ge_cmd_fillrect(ge_info_t *geinfo)
{
	return ge_set_command(geinfo, GECMD_BLIT, 0xf0);
}

INLINE int ge_cmd_rotate(ge_info_t *geinfo, unsigned int arc)
{
	volatile ge_regs_t *regs;

	regs = (ge_regs_t *)geinfo->mmio;

	switch (arc % 360) {
	case 0:
		regs->rotate_mode = 0;
		break;
	case 90:
		regs->rotate_mode = 1;
		break;
	case 180:
		regs->rotate_mode = 2;
		break;
	case 270:
		regs->rotate_mode = 3;
		break;
	default:
		break;
	}
	ge_set_command(geinfo, GECMD_ROTATE, 0);

	ge_wait_sync(geinfo); /* Required. */

	return 0;
}

INLINE int ge_cmd_mirror(ge_info_t *geinfo, int mode)
{
	volatile ge_regs_t *regs;

	regs = (ge_regs_t *)geinfo->mmio;

	regs->mirror_mode = mode & BIT0;

	ge_set_command(geinfo, GECMD_MIRROR, 0);
	ge_wait_sync(geinfo); /* Required. */

	return 0;
}

/* AMX barebone functions */

/* depercated */
INLINE int amx_show_surface(ge_info_t *geinfo,
	int id, ge_surface_t *s, int x, int y)
{
	volatile struct ge_regs_8710 *regs;
	volatile unsigned int *gov_regs;
	unsigned int cd; /* color depth */
	unsigned int hm; /* hi color mode */
	unsigned int gov_dspcr;

	if (GSPF_YUV(s->pixelformat))
		return -1;

	cd = GSPF_BYTES_PER_PIXEL(s->pixelformat) - 1;
	hm = GSPF_VARIANT(s->pixelformat);

	regs = (struct ge_regs_8710 *)geinfo->mmio;

	gov_regs = (unsigned int *)geinfo->mmio;
	gov_regs -= 64;
	gov_dspcr = gov_regs[7];

	if (gov_dspcr) {
		/* if gov was initialized ... */
		regs->disp_x_end = gov_dspcr & 0x7ff;
		regs->disp_y_end = gov_dspcr >> 16;
	} else {
		/* if gov is not initialized yet ... */
		regs->disp_x_end = s->xres_virtual - 1;
		regs->disp_y_end = s->yres_virtual - 1;
	}

	/*
	 * Register g1_yuv_mode_en was implemented since WM8710 B0.
	 * It returns zero on chips before WM8710 B0.
	 */

	if (id == 0) {
		/* G1 */
		regs->g1_cd = regs->g1_yuv_mode_en ? 3 : cd;
		regs->g1_amx_hm = hm;

		/* If FG being used, use BG. */
		if (regs->g1_fb_sel == 0) {
			regs->g1_bg_addr = s->addr;
			regs->g1_fb_sel = 1;
		} else {
			regs->g1_fg_addr = s->addr;
			regs->g1_fb_sel = 0;
		}

		regs->g1_fbw = s->xres_virtual;
		regs->g1_hcrop = s->x;
		regs->g1_vcrop = s->y;
		regs->g1_x_start =  x;
		regs->g1_x_end = x + s->xres - 1;
		regs->g1_y_start = y;
		regs->g1_y_end = y + s->yres - 1;
		regs->g1_amx_en = 1;
	} else {
		/* G2 */
		regs->g2_cd = regs->g2_yuv_mode_en ? 3 : cd;
		regs->g2_amx_hm = hm;

		/* If FG being used, use BG. */
		if (regs->g2_fb_sel == 0) {
			regs->g2_bg_addr = s->addr;
			regs->g2_fb_sel = 1;
		} else {
			regs->g2_fg_addr = s->addr;
			regs->g2_fb_sel = 0;
		}

		regs->g2_fbw = s->xres_virtual;
		regs->g2_hcrop = s->x;
		regs->g2_vcrop = s->y;
		regs->g2_x_start =  x;
		regs->g2_x_end = x + s->xres - 1;
		regs->g2_y_start = y;
		regs->g2_y_end = y + s->yres - 1;
		regs->g2_amx_en = 1;
	}

	return 0;
}

/* depercated */
INLINE int amx_get_surface(ge_info_t *geinfo, int id, ge_surface_t *s)
{
	volatile struct ge_regs_8710 *regs;
	unsigned int cd; /* color depth */
	unsigned int hm; /* hi color mode */

	regs = (struct ge_regs_8710 *)geinfo->mmio;

	/* Since AMX will be abandoned soon after WM8710, instead of
	 * supporting all pixel formats, we only check RGB16/RGB32 here.
	 */
	if (id == 0 && regs->g1_yuv_mode_en)
		return -1;
	if (id == 1 && regs->g2_yuv_mode_en)
		return -1;

	/*
	 * Register g1_yuv_mode_en was implemented since WM8710 B0.
	 * It returns zero on chips before WM8710 B0.
	 */

	if (id == 0) {
		/* G1 */
		cd = regs->g1_yuv_mode_en ? 0 : regs->g1_cd;
		hm = regs->g1_yuv_mode_en ? 0 : regs->g1_amx_hm;

		s->addr = regs->g1_fb_sel ?
			regs->g1_bg_addr : regs->g1_fg_addr;
		s->x = regs->g1_hcrop;
		s->y = regs->g1_vcrop;
		s->xres = regs->g1_x_end - regs->g1_x_start + 1;
		s->yres = regs->g1_y_end - regs->g1_y_start + 1;
		s->xres_virtual = regs->g1_fbw;
		s->yres_virtual = s->yres;
	} else {
		/* G2 */
		cd = regs->g2_yuv_mode_en ? 0 : regs->g2_cd;
		hm = regs->g2_yuv_mode_en ? 0 : regs->g2_amx_hm;

		s->addr = regs->g2_fb_sel ?
			regs->g2_bg_addr : regs->g2_fg_addr;
		s->x = regs->g2_hcrop;
		s->y = regs->g2_vcrop;
		s->xres = regs->g2_x_end - regs->g2_x_start + 1;
		s->yres = regs->g2_y_end - regs->g2_y_start + 1;
		s->xres_virtual = regs->g2_fbw;
		s->yres_virtual = s->yres;
	}

	if (id == 0)
		s->pixelformat = regs->g1_cd == 1 ? GSPF_RGB16 : GSPF_RGB32;
	if (id == 1)
		s->pixelformat = regs->g2_cd == 1 ? GSPF_RGB16 : GSPF_RGB32;

	return 0;
}

/* depercated */
INLINE int amx_hide_surface(ge_info_t *geinfo, int id)
{
	volatile struct ge_regs_8710 *regs;

	regs = (struct ge_regs_8710 *)geinfo->mmio;

	switch (id) {
	case 0:
		regs->g1_amx_en = 0;
		break;
	case 1:
		regs->g2_amx_en = 0;
		break;
	default:
		break;
	}
	return 0;
}

INLINE int amx_set_colorkey(ge_info_t *geinfo, int id, int enable,
	unsigned int r, unsigned int g, unsigned int b, unsigned int pixfmt)
{
	volatile struct ge_regs_8710 *regs;

	regs = (struct ge_regs_8710 *)geinfo->mmio;

	switch (pixfmt) {
	case GSPF_RGB16:
		r &= ~0x7;
		g &= ~0x3;
		b &= ~0x7;
		break;
	case GSPF_RGB555:
		r &= ~0x7;
		g &= ~0x7;
		b &= ~0x7;
		break;
	case GSPF_RGB454:
		r &= ~0xf;
		g &= ~0x7;
		b &= ~0xf;
		break;
	}

	switch (id) {
	case 0:
		regs->g1_ck_en = enable;
		regs->g1_c_key = enable ?
			((r << 16) | (g << 8) | (b)) : 0;
		break;
	case 1:
		regs->g2_ck_en = enable;
		regs->g2_c_key = enable ?
			((r << 16) | (g << 8) | (b)) : 0;
		break;
	default:
		break;
	}

	return 0;
}

INLINE int amx_set_alpha_8710(ge_info_t *geinfo, unsigned int alpha)
{
	volatile struct ge_regs_8710 *regs;

	alpha &= 0xff;

	regs = (struct ge_regs_8710 *)geinfo->mmio;

	regs->ge_amx_ctl = BIT4 | BIT3;
	regs->ge_fix_apa = 0xffff | alpha << 24;
	regs->ge_ck_apa = BIT10;

	regs->ge_amx2_ctl = BIT0;
	regs->ge_fix2_apa = 0xff;
	regs->ge_ck2_apa = BIT11 | BIT7 | BIT3;

	amx_enable_pixel_alpha_8710(geinfo, 0, 0);

	return 0;
}

INLINE int amx_enable_pixel_alpha_8710(ge_info_t *geinfo, int g1_en, int g2_en)
{
	/* Not updated */
	volatile struct ge_regs_8710 *regs;
	unsigned int type;
	unsigned int ge_amx_ctl;
	unsigned int ge_amx2_ctl;

	type = g1_en | (g2_en << 1);
	regs = (struct ge_regs_8710 *)geinfo->mmio;
	ge_amx_ctl = regs->ge_amx_ctl;
	ge_amx2_ctl = regs->ge_amx2_ctl;

	switch (type) {
	case 0:
		/* disable pixel alpha */
		ge_amx_ctl &= ~(BIT0 | BIT1 | BIT2);
		ge_amx2_ctl |= BIT2 | BIT4 | BIT6;
		break;
	case 1:
		/* enable pixel alpha for G1 only */
		ge_amx_ctl &= ~BIT3;
		ge_amx_ctl |= BIT0 | BIT2;
		ge_amx2_ctl &= ~(BIT2 | BIT6);
		ge_amx2_ctl |= BIT3 | BIT4 | BIT7;
		break;
	case 2:
		/* enable pixel alpha for G2 only */
		ge_amx_ctl &= ~BIT0;
		ge_amx_ctl |= BIT1 | BIT2 | BIT3 | BIT4;
		ge_amx2_ctl &= ~BIT4;
		ge_amx2_ctl |= BIT2 | BIT5 | BIT6;
		break;
	case 3:
		/* enable pixel alpha */
		ge_amx_ctl &= ~BIT5;
		ge_amx_ctl |= BIT0 | BIT1 | BIT2 | BIT3 | BIT4;
		ge_amx2_ctl &= ~(BIT2 | BIT4 | BIT6);
		ge_amx2_ctl |= BIT3 | BIT5 | BIT7;
		break;
	default:
		break;
	}

	regs->ge_amx_ctl = ge_amx_ctl;
	regs->ge_amx2_ctl = ge_amx2_ctl;

	return 0;
}

INLINE int amx_enable_pixel_alpha(ge_info_t *geinfo, int g1_en, int g2_en)
{
	amx_enable_pixel_alpha_8710(geinfo, g1_en, g2_en);

	return 0;
}

INLINE int amx_set_alpha(ge_info_t *geinfo, int id, unsigned int alpha)
{
	unsigned int major;
	unsigned int minor;

	major = geinfo->id >> 16;
	minor = geinfo->id & 0xffff;

	alpha &= 0xff;

	if (id)
		alpha = 0xff - alpha;

	amx_set_alpha_8710(geinfo, alpha);

	return 0;
}

INLINE int amx_sync(ge_info_t *geinfo)
{
	volatile struct ge_regs_8710 *regs;

	regs = (struct ge_regs_8710 *)geinfo->mmio;
	regs->ge_reg_sel = 0; /* select level1 registers */
	regs->ge_reg_upd = 1; /* update level2 registers */

	return 0;
}

INLINE int amx_set_csc_basic(ge_info_t *geinfo, int cscid)
{
	volatile struct ge_regs_8710 *regs;

	regs = (struct ge_regs_8710 *)geinfo->mmio;

	switch (cscid) {
	case AMX_CSC_DEFAULT:
		/* do nothing */
		break;
	case AMX_CSC_RGB_YC_SDTV_16_235:
		regs->c1_coef = 0x132;
		regs->c2_coef = 0x259;
		regs->c3_coef = 0x75;
		regs->c4_coef = 0x1f50;
		regs->c5_coef = 0x1ea5;
		regs->c6_coef = 0x20b;
		regs->c7_coef = 0x20b;
		regs->c8_coef = 0x1e4a;
		regs->c9_coef = 0x1fab;
		regs->coef_i = 1;
		regs->coef_j = 0x101;
		regs->coef_k = 0x101;
		break;
	case AMX_CSC_RGB_YC_SDTV_0_255:
		regs->c1_coef = 0x107;
		regs->c2_coef = 0x204;
		regs->c3_coef = 0x64;
		regs->c4_coef = 0x1f68;
		regs->c5_coef = 0x1ed6;
		regs->c6_coef = 0x1c2;
		regs->c7_coef = 0x1c2;
		regs->c8_coef = 0x1e78;
		regs->c9_coef = 0x1fb7;
		regs->coef_i = 0x20;
		regs->coef_j = 0x101;
		regs->coef_k = 0x101;
		break;
	case AMX_CSC_RGB_YC_HDTV_16_235:
		regs->c1_coef = 0xda;
		regs->c2_coef = 0x2dc;
		regs->c3_coef = 0x4a;
		regs->c4_coef = 0x1f88;
		regs->c5_coef = 0x1e6d;
		regs->c6_coef = 0x20b;
		regs->c7_coef = 0x20b;
		regs->c8_coef = 0x1e25;
		regs->c9_coef = 0x1fd0;
		regs->coef_i = 1;
		regs->coef_j = 0x101;
		regs->coef_k = 0x101;
		break;
	case AMX_CSC_RGB_YC_HDTV_0_255:
		regs->c1_coef = 0xbb;
		regs->c2_coef = 0x275;
		regs->c3_coef = 0x3f;
		regs->c4_coef = 0x1f99;
		regs->c5_coef = 0x1ea6;
		regs->c6_coef = 0x1c2;
		regs->c7_coef = 0x1c2;
		regs->c8_coef = 0x1e67;
		regs->c9_coef = 0x1fd7;
		regs->coef_i = 0x21;
		regs->coef_j = 0x101;
		regs->coef_k = 0x101;
		break;
	case AMX_CSC_RGB_YC_JFIF_0_255:
		regs->c1_coef = 0x132;
		regs->c2_coef = 0x259;
		regs->c3_coef = 0x75;
		regs->c4_coef = 0x1f53;
		regs->c5_coef = 0x1ead;
		regs->c6_coef = 0x200;
		regs->c7_coef = 0x200;
		regs->c8_coef = 0x1e53;
		regs->c9_coef = 0x1fad;
		regs->coef_i = 0x1;
		regs->coef_j = 0x101;
		regs->coef_k = 0x101;
		break;
	case AMX_CSC_RGB_YC_SMPTE_170M:
		regs->c1_coef = 0x132;
		regs->c2_coef = 0x259;
		regs->c3_coef = 0x75;
		regs->c4_coef = 0x1f50;
		regs->c5_coef = 0x1ea5;
		regs->c6_coef = 0x20b;
		regs->c7_coef = 0x20b;
		regs->c8_coef = 0x1e4a;
		regs->c9_coef = 0x1fab;
		regs->coef_i = 1;
		regs->coef_j = 0x101;
		regs->coef_k = 0x101;
		break;
	case AMX_CSC_RGB_YC_SMPTE_240M:
		regs->c1_coef = 0xd9;
		regs->c2_coef = 0x2ce;
		regs->c3_coef = 0x59;
		regs->c4_coef = 0x2f89;
		regs->c5_coef = 0x1e77;
		regs->c6_coef = 0x200;
		regs->c7_coef = 0x200;
		regs->c8_coef = 0x1e38;
		regs->c9_coef = 0x1fc8;
		regs->coef_i = 1;
		regs->coef_j = 0x101;
		regs->coef_k = 0x101;
		break;
	case AMX_CSC_RGB_BYPASS:
		regs->c1_coef = 0x400;
		regs->c2_coef = 0;
		regs->c3_coef = 0;
		regs->c4_coef = 0x400;
		regs->c5_coef = 0;
		regs->c6_coef = 0;
		regs->c7_coef = 0x400;
		regs->c8_coef = 0;
		regs->c9_coef = 0;
		regs->coef_i = 1;
		regs->coef_j = 0x101;
		regs->coef_k = 0x101;
		break;
	case AMX_CSC_YC_RGB_SDTV_16_235:
		regs->c1_coef = 0x400;
		regs->c2_coef = 0;
		regs->c3_coef = 0x57c;
		regs->c4_coef = 0x400;
		regs->c5_coef = 0x1ea8;
		regs->c6_coef = 0x1d35;
		regs->c7_coef = 0x400;
		regs->c8_coef = 0x6ee;
		regs->c9_coef = 0;
		regs->coef_i = 1;
		regs->coef_j = 1;
		regs->coef_k = 1;
		break;
	case AMX_CSC_YC_RGB_SDTV_0_255:
		regs->c1_coef = 0x4a8;
		regs->c2_coef = 0;
		regs->c3_coef = 0x662;
		regs->c4_coef = 0x4a8;
		regs->c5_coef = 0x1e70;
		regs->c6_coef = 0x1cbf;
		regs->c7_coef = 0x4a8;
		regs->c8_coef = 0x812;
		regs->c9_coef = 0;
		regs->coef_i = 1;
		regs->coef_j = 1;
		regs->coef_k = 1;
		break;
	case AMX_CSC_YC_RGB_HDTV_16_235:
		regs->c1_coef = 0x400;
		regs->c2_coef = 0;
		regs->c3_coef = 0x629;
		regs->c4_coef = 0x400;
		regs->c5_coef = 0x1f45;
		regs->c6_coef = 0x1e2a;
		regs->c7_coef = 0x400;
		regs->c8_coef = 0x744;
		regs->c9_coef = 0;
		regs->coef_i = 1;
		regs->coef_j = 1;
		regs->coef_k = 1;
		break;
	case AMX_CSC_YC_RGB_HDTV_0_255:
		regs->c1_coef = 0x4a8;
		regs->c2_coef = 0;
		regs->c3_coef = 0x72c;
		regs->c4_coef = 0x4a8;
		regs->c5_coef = 0x1f26;
		regs->c6_coef = 0x1ddd;
		regs->c7_coef = 0x4a8;
		regs->c8_coef = 0x876;
		regs->c9_coef = 0;
		regs->coef_i = 1;
		regs->coef_j = 1;
		regs->coef_k = 1;
		break;
	case AMX_CSC_YC_RGB_JFIF_0_255:
		regs->c1_coef = 0x400;
		regs->c2_coef = 0;
		regs->c3_coef = 0x59c;
		regs->c4_coef = 0x400;
		regs->c5_coef = 0x1ea0;
		regs->c6_coef = 0x1d25;
		regs->c7_coef = 0x400;
		regs->c8_coef = 0x717;
		regs->c9_coef = 0;
		regs->coef_i = 1;
		regs->coef_j = 1;
		regs->coef_k = 1;
		break;
	case AMX_CSC_YC_RGB_SMPTE_170M:
		regs->c1_coef = 0x400;
		regs->c2_coef = 0;
		regs->c3_coef = 0x57c;
		regs->c4_coef = 0x400;
		regs->c5_coef = 0x1ea8;
		regs->c6_coef = 0x1d35;
		regs->c7_coef = 0x400;
		regs->c8_coef = 0x6ee;
		regs->c9_coef = 0;
		regs->coef_i = 1;
		regs->coef_j = 1;
		regs->coef_k = 1;
		break;
	case AMX_CSC_YC_RGB_SMPTE_240M:
		regs->c1_coef = 0x400;
		regs->c2_coef = 0;
		regs->c3_coef = 0x64d;
		regs->c4_coef = 0x400;
		regs->c5_coef = 0x1f19;
		regs->c6_coef = 0x1e00;
		regs->c7_coef = 0x400;
		regs->c8_coef = 0x74f;
		regs->c9_coef = 0;
		regs->coef_i = 1;
		regs->coef_j = 1;
		regs->coef_k = 1;
		break;
	case AMX_CSC_YC_BYPASS:
		regs->c1_coef = 400;
		regs->c2_coef = 0;
		regs->c3_coef = 0;
		regs->c4_coef = 400;
		regs->c5_coef = 0;
		regs->c6_coef = 0;
		regs->c7_coef = 400;
		regs->c8_coef = 0;
		regs->c9_coef = 0;
		regs->coef_i = 1;
		regs->coef_j = 1;
		regs->coef_k = 1;
		break;
	}

	return 0;
}

INLINE int amx_set_csc_8710(ge_info_t *geinfo, int cscid)
{
	volatile struct ge_regs_8710 *regs;

	regs = (struct ge_regs_8710 *)geinfo->mmio;

	switch (cscid) {
	case AMX_CSC_DEFAULT:
		/* do nothing */
		break;
	case AMX_CSC_RGB_YC_SDTV_16_235:
		regs->ge_amx_csc_mode = 0;
		regs->ge_amx_y_sub_16_en = 0;
		break;
	case AMX_CSC_RGB_YC_SDTV_0_255:
		regs->ge_amx_csc_mode = 0;
		regs->ge_amx_y_sub_16_en = 0;
		break;
	case AMX_CSC_RGB_YC_HDTV_16_235:
		regs->ge_amx_csc_mode = 0;
		regs->ge_amx_y_sub_16_en = 0;
		break;
	case AMX_CSC_RGB_YC_HDTV_0_255:
		regs->ge_amx_csc_mode = 0;
		regs->ge_amx_y_sub_16_en = 0;
		break;
	case AMX_CSC_RGB_YC_JFIF_0_255:
		regs->ge_amx_csc_mode = 0;
		regs->ge_amx_y_sub_16_en = 0;
		break;
	case AMX_CSC_RGB_YC_SMPTE_170M:
		regs->ge_amx_csc_mode = 0;
		regs->ge_amx_y_sub_16_en = 0;
		break;
	case AMX_CSC_RGB_YC_SMPTE_240M:
		regs->ge_amx_csc_mode = 0;
		regs->ge_amx_y_sub_16_en = 0;
		break;
	case AMX_CSC_RGB_BYPASS:
		regs->ge_amx_csc_mode = 0;
		regs->ge_amx_y_sub_16_en = 0;
		break;
	case AMX_CSC_YC_RGB_SDTV_16_235:
		regs->ge_amx_csc_mode = 1;
		regs->ge_amx_y_sub_16_en = 0;
		break;
	case AMX_CSC_YC_RGB_SDTV_0_255:
		regs->ge_amx_csc_mode = 1;
		regs->ge_amx_y_sub_16_en = 1;
		break;
	case AMX_CSC_YC_RGB_HDTV_16_235:
		regs->ge_amx_csc_mode = 1;
		regs->ge_amx_y_sub_16_en = 0;
		break;
	case AMX_CSC_YC_RGB_HDTV_0_255:
		regs->ge_amx_csc_mode = 1;
		regs->ge_amx_y_sub_16_en = 1;
		break;
	case AMX_CSC_YC_RGB_JFIF_0_255:
		regs->ge_amx_csc_mode = 1;
		regs->ge_amx_y_sub_16_en = 0;
		break;
	case AMX_CSC_YC_RGB_SMPTE_170M:
		regs->ge_amx_csc_mode = 1;
		regs->ge_amx_y_sub_16_en = 0;
		break;
	case AMX_CSC_YC_RGB_SMPTE_240M:
		regs->ge_amx_csc_mode = 1;
		regs->ge_amx_y_sub_16_en = 0;
		break;
	case AMX_CSC_YC_BYPASS:
		regs->ge_amx_csc_mode = 1;
		regs->ge_amx_y_sub_16_en = 0;
		break;
	}

	regs->g1_yuv_outp_sel = 0; /* progressive */
	regs->g2_yuv_outp_sel = 0; /* progressive */

	return 0;
}

INLINE int amx_set_csc(ge_info_t *geinfo, int cscid)
{
	unsigned int major;
	unsigned int minor;

	amx_set_csc_basic(geinfo, cscid);

	major = geinfo->id >> 16;
	minor = geinfo->id & 0xffff;

	if (major == 0x3445) /* WM8710 B0 */
		amx_set_csc_8710(geinfo, cscid);

	return 0;
}

