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

/* Generic Memory Provider */

#ifdef __KERNEL__
#include <asm/cacheflush.h>
#include "gmp.h"
#else
#include <linux/fb.h>
#include <linux/errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include <gmp.h>
#include <ge_ioctl.h>
#endif

#ifdef __BIONIC__
#include <android/log.h>
#endif

#ifndef __LINUX__
#if !defined(__KERNEL__) && !defined(__POST__)
#define __LINUX__
#endif
#endif

#define gmp_log __gmp_log
#define gmp_trace() \
	gmp_log(GMP_LOG_DEBUG, "[gmp] trace %s at line %d\n", \
	__func__, __LINE__)

#ifdef __KERNEL__
static int gmp_device_lock(void);
static int gmp_device_unlock(void);
#endif

static int gmp_log_level = GMP_LOG_ERR;

static int __gmp_log(int level, const char *fmt, ...)
{
        va_list ap;
	/* omit non-critial messages */
	if (level > gmp_log_level)
		return 0;
        va_start(ap, fmt);
#if defined(__KERNEL__)
        return vprintk(fmt, ap);
#elif defined(__BIONIC__)
        return __android_log_vprint(ANDROID_LOG_DEBUG, "gmp", fmt, ap);
#else
        return vprintf(fmt, ap);
#endif
}

#ifdef __LINUX__
static int open_fb_device(void)
{
	int fd;

	fd = open("/dev/fb0", O_RDWR);
	if (fd < 0)
		fd = open("/dev/graphics/fb0", O_RDWR, 0);
	if (fd < 0) {
		return -1;
	}
	return fd;
}
#endif

static pid_t gmp_get_owner(void)
{
	/* Kernel's current->tgid equals to user process's getpid() */
#if defined(__KERNEL__)
	return current->tgid;
#elif defined(__LINUX__)
	return getpid();
#else
	return (pid_t) 0;
#endif
}

static size_t gmp_get_order(size_t val)
{
	int order;

	order = 0;

	if (val) {
		val--;
		val >>= GMP_SHIFT;
		while (val) {
			val >>= 1;
			order++;
		}
	}

	return order;
}

/*
static void gmp_split_unused(struct gmdesc *gmdesc)
{
	struct gmdesc *p = gmdesc;
	struct gmdesc *next;

	while (p) {
		next = next_gmdesc(gmp, p);
		if (!p->used && p->order)
			p->order = 0;
		p = next;
	}
}
*/

static int gmp_split(struct gmp *gmp, struct gmdesc *gmdesc)
{
	struct gmdesc *newdesc;
	unsigned long ofs;

	if (gmdesc && gmdesc->order) {
		gmdesc->order--;
		newdesc = gmdesc + GMP_NR(gmdesc);
		ofs = GMP_SIZE(gmdesc);
		if (newdesc->pa != gmdesc->pa + ofs) {
			gmp_log(GMP_LOG_ERR,
				   "[gmp] split failure (%p != %p)\n",
				   (void *) newdesc->pa,
				   (void *) (gmdesc->pa + ofs));
			return -EFAULT;
		}
		newdesc->order = gmdesc->order;
		newdesc->used  = gmdesc->used;
		newdesc->tag   = GMP_TAG_DEFAULT;
		newdesc->tail  = gmdesc->tail;
		gmdesc->tail = 0;
		return 0;
	}

	gmp_log(GMP_LOG_ERR, "[gmp] split failure\n");

	return -EINVAL;
}

static int gmp_merge(struct gmp *gmp, struct gmdesc *gmdesc)
{
	unsigned long start;
	int unaligned;
	struct gmdesc *p;

	start = gmdesc->pa - gmp->gmdesc->pa;
	unaligned = (start >> (gmdesc->order + GMP_SHIFT)) & 1;

	if (gmdesc->used || unaligned)
		return -EINVAL;

	if (next_gmdesc(gmp, gmdesc)) {
		p = next_gmdesc(gmp, gmdesc);
		if (p->used)
			return -EINVAL;

		gmp_merge(gmp, p);

		if (p->order == gmdesc->order) {
			gmdesc->order++;
			gmdesc->tail = p->tail;
			p->tail = 0;
			while (gmp_merge(gmp, gmdesc) == 0);
			return 0;
		}
	}

	return -EINVAL;
}

struct gmdesc *next_gmdesc(struct gmp *gmp, struct gmdesc *gmdesc)
{
	struct gmdesc *p;

	p = gmdesc;

	if (p->tail)
		return NULL;

	p += GMP_NR(p);

	return p;
}

struct gmdesc *prev_gmdesc(struct gmp *gmp, struct gmdesc *gmdesc)
{
	struct gmdesc *p;
	struct gmdesc *next;

	if (gmdesc == gmp->gmdesc)
		return NULL;

	next = gmp->gmdesc;
	while (next != gmdesc) {
		p = next;
		next = next_gmdesc(gmp, p);
	}
	if (next == gmdesc)
		return p;

	return NULL;
}

void gmp_reset(struct gmp *gmp)
{
	struct gmdesc *gmdesc;
	struct gmdesc *p;
	unsigned long addr;
	int gmdesc_nr;
	int len;

	if (!gmp)
		return;

	gmp_trace();

	gmp_log(GMP_LOG_CRIT, "[gmp] reset memory descriptors\n");

	gmdesc = gmp->gmdesc;
	gmdesc_nr = gmp->len >> GMP_SHIFT;
	len = sizeof(struct gmdesc) * gmdesc_nr;

	if (gmdesc_nr == 0)
		return;

	/* initialize memory descriptors */
	memset(gmdesc, 0, len);
	p = gmdesc;
	addr = gmp->pa;
	while (gmdesc_nr--) {
		p->pa = addr;
		p->tag = GMP_TAG_DEFAULT;
		p++;
		addr += (1 << GMP_SHIFT);
	}
	p--;
	p->tail = 1;

	/* defrag memory (forward) */
	while (p) {
		gmp_merge(gmp, p);
		p = prev_gmdesc(gmp, p);
	}

	/* claim 1st block for descriptors */
	gmp_alloc(gmp, len, gmp_ctag('g', 'm', 'p', ' '));

	/* memory owned by pid -1 was never released */
	gmp->gmdesc->owner = (pid_t) -1;
}

int gmp_lock(struct gmp *gmp)
{
#if defined(__KERNEL__)
	return gmp_device_lock();
#elif defined(__LINUX__)
	return ioctl(gmp->hnd, GMP_LOCK, 1);
#else
	return 0;
#endif
}

int gmp_unlock(struct gmp *gmp)
{
#if defined(__KERNEL__)
	return gmp_device_unlock();
#elif defined(__LINUX__)
	return ioctl(gmp->hnd, GMP_LOCK, 0);
#else
	return 0;
#endif
}

static size_t size_of_gmdesc_array(struct gmp *gmp)
{
	size_t gmdesc_nr;
	size_t len;

	gmdesc_nr = gmp->len >> GMP_SHIFT;
	len = sizeof(struct gmdesc) * gmdesc_nr;

	return len;
}

static void gmp_flush_gmdesc(struct gmp *gmp)
{
	unsigned long gmdesc_start;
	int gmdesc_size;

	gmdesc_start = gmp->pa;
	gmdesc_size = size_of_gmdesc_array(gmp);
	gmp_cache_flush(gmp, gmdesc_start, gmdesc_size);
}

struct gmp *create_gmp(unsigned long pa, size_t size)
{
	struct gmp *gmp;
	size_t len;
	int hnd;

	gmp_trace();

	hnd = 0;

#ifdef __LINUX__
	if ((hnd = open("/dev/gmp", O_RDWR)) < 0) {
		gmp_log(GMP_LOG_ERR,
			"[gmp] cannot open device\n");
		return NULL;
	}
	if (size == 0)
		ioctl(hnd, GMP_GET_SIZE, &size);
	if (pa == 0)
		ioctl(hnd, GMP_GET_PHYS, &pa);
#endif

	if ((pa & GMP_MASK) || pa == 0) {
		gmp_log(GMP_LOG_ERR,
			"[gmp] Cannot use unaligned memory %p\n",
			(void *) pa);
		return NULL;
	}

	if ((size & GMP_MASK) || size == 0) {
		gmp_log(GMP_LOG_ERR,
			"[gmp] Memory size (%d) is invalid\n",
			(int) size);
		return NULL;
	}

#ifdef __KERNEL__
	gmp = (struct gmp *) kcalloc(1, sizeof(struct gmp), GFP_KERNEL);
#else
	gmp = (struct gmp *) calloc(1, sizeof(struct gmp));
#endif
	gmp->pa = pa;
	gmp->len = size;
	gmp->hnd = hnd;

	len = size_of_gmdesc_array(gmp);
	gmp->gmdesc = (struct gmdesc *) gmp_ioremap(gmp, gmp->pa, len);
	if (!gmp->gmdesc)
		gmp_log(GMP_LOG_ERR,
			"[gmp] cannot ioremap %p for %d bytes\n",
			gmp->pa, len);

	gmp_flush_gmdesc(gmp);

#ifdef __KERNEL__
	/* initial memory descriptor once */
	if (gmp->gmdesc && !gmp->gmdesc->used) {
		gmp_reset(gmp);
		gmp_print_status(gmp);
	}
#endif
	/* recovery damaged descriptor */
	if (gmp->gmdesc && gmp->gmdesc->pa != gmp->pa) {
		gmp_log(GMP_LOG_ERR, "[gmp] recovery damaged descriptors\n");
		gmp_reset(gmp);
	}

	return gmp;
}

void release_gmp(struct gmp *gmp)
{
	size_t len;

	if (!gmp)
		return;

	gmp_trace();

	gmp_invalidate(gmp, gmp_get_owner());
	len = size_of_gmdesc_array(gmp);
	gmp_iounmap(gmp, (void *) gmp->gmdesc, len);

#ifdef __LINUX__
	if (gmp->hnd)
		close(gmp->hnd);
#endif
#ifdef __KERNEL__
	kfree(gmp);
#else
	free(gmp);
#endif
}

void gmp_invalidate(struct gmp *gmp, pid_t pid)
{
	struct gmdesc *p;

	gmp_trace();

	if (!gmp)
		return;

	/* Release all memory owned by pid */
	p = gmp->gmdesc;
	while (p) {
		if (p->owner == pid) {
			p->used = 0;
			p->tag = GMP_TAG_DEFAULT;
			p->owner = 0;
		}
		p = next_gmdesc(gmp, p);
	}

	/* Defrag memory */
	p = gmp->gmdesc;
	while (p) {
		gmp_merge(gmp, p);
		p = next_gmdesc(gmp, p);
	}
}

void gmp_cache_flush(struct gmp *gmp, unsigned long physaddr, size_t size)
{
#if defined(__LINUX__)
	struct gmp_region region;
	region.pa = physaddr;
	region.len = size;
	ioctl((int) gmp->hnd, GMP_CACHE_FLUSH, &region);
#elif defined(__KERNEL__)
	void *flush_start = ioremap(physaddr, size);
	void *flush_end = flush_start + size;
	if (flush_start) {
		dmac_flush_range(flush_start, flush_end);
		outer_flush_range(__pa(flush_start), __pa(flush_end));
		iounmap(flush_start);
	}
#endif
}

void *gmp_ioremap(struct gmp *gmp, unsigned long physaddr, size_t size)
{
#if defined(__LINUX__)
	struct fb_fix_screeninfo fsinfo;
	int fd;
	off_t off;
	void *va;
	unsigned long smem_start;
	int err;

	gmp_trace();

	/* try gmp */

	fd = (int) gmp->hnd;
	err = 0;

	if (ioctl(fd, GMP_GET_PHYS, &smem_start) < 0) {
		gmp_log(GMP_LOG_ERR, "[gmp] ioctl GMP_GET_PHYS error\n");
		err++;
	}

	if (physaddr < smem_start) {
		gmp_log(GMP_LOG_INFO, "[gmp] cannot map %p below %p\n",
			physaddr, smem_start);
		err++;
	}

	if (!err) {
		off = ((physaddr - smem_start) + 0xfff) & ~0xfff;
		va = mmap((void *) 0, size,
		PROT_READ|PROT_WRITE, MAP_SHARED, fd, off);
		if (va != MAP_FAILED)
			return va;
	}

	/* try linuxfb */

	fd = open_fb_device();

	ioctl(fd, FBIOGET_FSCREENINFO, &fsinfo);

	if (physaddr < fsinfo.smem_start) {
		gmp_log(GMP_LOG_INFO, "[gmp] cannot map %p below %p\n",
			physaddr, fsinfo.smem_start);
		close(fd);
		return NULL;
	}

	off = ((physaddr - fsinfo.smem_start) + 0xfff) & ~0xfff;
	va = mmap((void *) 0, size,
		PROT_READ|PROT_WRITE, MAP_SHARED, fd, off);

	close(fd);

	if (va == MAP_FAILED) {
		gmp_log(GMP_LOG_ERR, "[gmp] cannot map %p for %d bytes\n",
			physaddr, size);
		return NULL;
	}

	return va;
#elif defined(__KERNEL__)
	return ioremap(physaddr, size);
#elif defined(__POST__)
	return (void *) physaddr;
#else
	return NULL;
#endif
}

void gmp_iounmap(struct gmp *gmp, void *va, size_t size)
{
	gmp_trace();

#if defined(__LINUX__)
	if (va)
		munmap(va, size);
#elif defined(__KERNEL__)
	if (va)
		iounmap(va);
#endif
}

void gmp_get_status(struct gmp *gmp, int *used, int *avail)
{
	struct gmdesc *p;
	unsigned int i_used;
	unsigned int i_avail;

	gmp_trace();

	i_used = 0;
	i_avail = 0;
	p = gmp->gmdesc;

	while (p) {
		if (p->used)
			i_used += GMP_SIZE(p);
		else
			i_avail += GMP_SIZE(p);
		p = next_gmdesc(gmp, p);
	}

	*used = i_used;
	*avail = i_avail;
}

unsigned int gmp_itag(unsigned int value)
{
	unsigned int value2;
	char *s1 = (char *) &value;
	char *s2 = (char *) &value2;
	/* return if value is ascii code */
	if (s1[0] >= 0x20)
		return value;
	/* convert integer to ascii code */
	s2[0] = s1[3];
	s2[1] = s1[2];
	s2[2] = s1[1];
	s2[3] = s1[0];
	if (s2[0] < 0x30)
		s2[0] += 0x30;
	if (s2[1] < 0x30)
		s2[1] += 0x30;
	if (s2[2] < 0x30)
		s2[2] += 0x30;
	if (s2[3] < 0x30)
		s2[3] += 0x30;
	return value2;
}

unsigned int gmp_ctag(char a, char b, char c, char d)
{
	unsigned int value;
	char *s = (char *) &value;
	s[0] = a;
	s[1] = b;
	s[2] = c;
	s[3] = d;
	return value;
}

void gmp_print_status(struct gmp *gmp)
{
	struct gmdesc *p;
	const char *tag;
	unsigned int i_used;
	unsigned int i_avail;

	if (gmp_log_level < GMP_LOG_INFO)
		return;

	gmp_trace();

	i_used = 0;
	i_avail = 0;
	p = gmp->gmdesc;

	while (p) {
		tag = (const char *) &p->tag;
		gmp_log(GMP_LOG_DEBUG,
			"[gmp] pa: 0x%08lx, order: %d, used: %d, "
			"tag: %c%c%c%c, owner: %d\n",
			p->pa, p->order, p->used,
			tag[0], tag[1], tag[2], tag[3],
			p->owner);
		if (p->used) {
			i_used += 1 << (p->order + GMP_SHIFT - 10);
		} else {
			i_avail += 1 << (p->order + GMP_SHIFT - 10);
		}
		p = next_gmdesc(gmp, p);
	}

	gmp_log(GMP_LOG_INFO, "[gmp] used: %d KB, free: %d KB\n",
		   i_used, i_avail);
}

unsigned long gmp_alloc(struct gmp *gmp, size_t size, unsigned int tag)
{
	struct gmdesc *p;
	int order;
	pid_t pid;

	gmp_trace();

	gmp_log(GMP_LOG_INFO, "[gmp] alloc %d bytes\n", (int) size);

	pid = gmp_get_owner();
	order = gmp_get_order(size);

	p = gmp->gmdesc;

	while (p) {
		if (!p->used && p->order >= order) {
			if (p->order == order)
				break;
			while (p->order > order) {
				gmp_split(gmp, p);
			}
			break;
		}
		p = next_gmdesc(gmp, p);
	}

	if (p && !p->used) {
		p->used = 1;
		p->tag = gmp_itag(tag);
		p->owner = pid;

		gmp_print_status(gmp);

		return p->pa; /* success */
	}

	gmp_log(GMP_LOG_ERR, "[gmp] cannot allocate %d bytes\n", size);

	gmp_print_status(gmp);

	return 0;
}

void gmp_free(struct gmp *gmp, unsigned long pa)
{
	struct gmdesc *p;
	pid_t pid;

	gmp_trace();

	pid = gmp_get_owner();

	gmp_log(GMP_LOG_INFO, "[gmp] free %p\n", (void *) pa);

	p = gmp->gmdesc;
	while (p) {
		if (p->pa == pa && p->owner == pid)
			break;
		p = next_gmdesc(gmp, p);
	}
	if (p) {
		p->used  = 0;
		p->tag   = GMP_TAG_DEFAULT;
		p->owner = 0;
	}

	/* memory defragment */
	p = gmp->gmdesc;
	while (p) {
		gmp_merge(gmp, p);
		p = next_gmdesc(gmp, p);
	}

	gmp_print_status(gmp);
}

void gmp_set_log_level(int level)
{
	gmp_log_level = level;
}

/* GMP Linux Kernel Driver */
/*
 * GMP device has to register before using GMP API.
 *
 * int register_gmp_device(unsigned long physaddr, unsigned long size)
 * int unregister_gmp_device(void)
 */

#ifdef __KERNEL__
#include <asm/uaccess.h>
#include <linux/cdev.h>
#include <linux/proc_fs.h>
#include <linux/mm.h>
#include <linux/fs.h>
#ifndef LINUX_VERSION_CODE
#include <linux/version.h>
#endif /* LINUX_VERSION_CODE */
#define GMP_DEV_NAME		"gmp"
static int gmp_device_open(struct inode *inode, struct file *filp);
static int gmp_device_release(struct inode *inode, struct file *filp);
static int gmp_device_mmap(struct file *file, struct vm_area_struct * vma);
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,36)
static int gmp_device_ioctl(struct inode *inode, struct file *filp,
			    unsigned int command, unsigned long arg);
static DECLARE_MUTEX(gmp_sem);
#else
static long gmp_device_ioctl(struct file *filp, unsigned int cmd,
			     unsigned long arg);
static DEFINE_SEMAPHORE(gmp_sem);
#endif
static DEFINE_MUTEX(gmp_mutex);
static struct task_struct *gmp_sem_owner;
static struct gmp *g_gmp;
static struct proc_dir_entry *gmp_procfs_dir;
static struct proc_dir_entry *gmp_procfs_status_file;
static int init_procfs_gmp(void);
static void exit_procfs_gmp(void);
static int gmp_busy_flag;
static struct class *gmp_device_class;
static dev_t gmp_id;
static unsigned long gmp_smem_start;
static unsigned long gmp_smem_len;

static const struct file_operations gmp_device_fops = {
	.open		= gmp_device_open,
	.release	= gmp_device_release,
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,32)
	.ioctl		= gmp_device_ioctl,
#else
	.unlocked_ioctl = gmp_device_ioctl,
#endif
	.mmap		= gmp_device_mmap,
	.owner		= THIS_MODULE,
};

static struct cdev gmp_device = {
	.owner = THIS_MODULE,
};

int register_gmp_device(unsigned long physaddr, unsigned long size)
{
	int err;
	dev_t dev;

	if (physaddr == 0 || size == 0)
		return -ENOMEM;

	err = alloc_chrdev_region(&dev, 0, 1, GMP_DEV_NAME);
	if (err) {
		printk(KERN_ERR "%s: register_chrdev_region = %d\n",
			__func__, err);
		return -EFAULT;
	}

	cdev_init(&gmp_device, &gmp_device_fops);

	err = cdev_add(&gmp_device, dev, 1);

	if (err) {
		printk(KERN_ERR "%s: cdev_add = %d\n", __func__, err);
		unregister_chrdev_region(dev, 1);
		return -EFAULT;
	}

	printk("gmp was registered as device (%d,%d)\n",
		MAJOR(dev), MINOR(dev));

	/* let udev to handle /dev/gmp_device */
	gmp_device_class = class_create(THIS_MODULE, GMP_DEV_NAME);
	device_create(gmp_device_class, NULL, dev, NULL, "%s",
			GMP_DEV_NAME);

	gmp_id = dev;

	/* should not request_mem_region here */
	/* request_mem_region(physaddr, size, GMP_DEV_NAME); */

	g_gmp = NULL;
	gmp_busy_flag = 0;

	gmp_set_log_level(GMP_LOG_ERR);
	g_gmp = create_gmp(physaddr, size);

	gmp_smem_start = physaddr;
	gmp_smem_len = size;

	init_procfs_gmp();

	return 0;
}

int unregister_gmp_device(void)
{
	dev_t dev;

	dev = gmp_id;

	exit_procfs_gmp();

	if (g_gmp) {
		release_gmp(g_gmp);
		g_gmp = NULL;
	}

	/* let udev to handle /dev/gmp_device */
	device_destroy(gmp_device_class, dev);
	class_destroy(gmp_device_class);

	unregister_chrdev_region(dev, 1);
	cdev_del(&gmp_device);

	return 0;
}

static int gmp_device_lock(void)
{
        int ret;
        ret = down_interruptible(&gmp_sem);
	if (ret == 0) {
		mutex_lock(&gmp_mutex);
		gmp_sem_owner = current;
		gmp_busy_flag = 1;
		mutex_unlock(&gmp_mutex);
	}
	return ret;
}

static int gmp_device_unlock(void)
{
	if (gmp_sem_owner == current) {
		up(&gmp_sem);
		mutex_lock(&gmp_mutex);
		gmp_busy_flag = 0;
		mutex_unlock(&gmp_mutex);
	} else
		return -1;
	return 0;
}

static int gmp_device_open(struct inode *inode, struct file *filp)
{
	filp->private_data = (void *) current->tgid;

	gmp_log(GMP_LOG_DEBUG, "%s: %s, pid %d, tgid %d\n",
		__func__, current->comm, current->pid, current->tgid);

	return 0;
}

static int gmp_device_release(struct inode *inode, struct file *filp)
{
	pid_t pid;

	gmp_device_unlock();

	pid = (pid_t) filp->private_data;

	mutex_lock(&gmp_mutex);
	gmp_invalidate(g_gmp, pid);
	mutex_unlock(&gmp_mutex);

	gmp_log(GMP_LOG_DEBUG, "%s: %s, pid %d, tgid %d\n",
		__func__, current->comm, current->pid, current->tgid);

	return 0;
}

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,36)
static int gmp_device_ioctl(struct inode *inode, struct file *filp,
	unsigned int cmd, unsigned long arg)
#else
static long gmp_device_ioctl(struct file *filp, unsigned int cmd,
			     unsigned long arg)
#endif
{
	void __user *argp;
	int ret;


	argp = (void __user *)arg;

	switch (cmd) {
	case GMP_GET_PHYS:
		mutex_lock(&gmp_mutex);
		ret = copy_to_user(argp, &gmp_smem_start,
			sizeof(unsigned long));
		mutex_unlock(&gmp_mutex);
                break;
	case GMP_GET_SIZE:
		mutex_lock(&gmp_mutex);
		ret = copy_to_user(argp, &gmp_smem_len,
			sizeof(unsigned long));
		mutex_unlock(&gmp_mutex);
                break;
	case GMP_LOCK:
		if (arg)
			ret = gmp_device_lock();
		else
			ret = gmp_device_unlock();
                break;
	case GMP_CACHE_FLUSH:
		{
			struct gmp_region region;
			if (copy_from_user(&region, argp,
				sizeof(struct gmp_region)))
				return -EFAULT;
			gmp_cache_flush(g_gmp, region.pa, region.len);
			ret = 0;
			break;
		}
	case GMP_CACHE_FLUSH_ALL:
		gmp_cache_flush(g_gmp, gmp_smem_start, gmp_smem_len);
		ret = 0;
		break;
	default:
		ret = -1;
	}


	return ret;
}

static int gmp_device_mmap(struct file *file, struct vm_area_struct * vma)
{
	unsigned long off;
	unsigned long start;
	u32 len;

	if (vma->vm_pgoff > (~0UL >> PAGE_SHIFT)) {
		gmp_log(GMP_LOG_ERR, "[gmp] gmp_device_mmap fail #1\n");
		return -EINVAL;
	}
	off = vma->vm_pgoff << PAGE_SHIFT;

	start = gmp_smem_start;
	len = PAGE_ALIGN((start & ~PAGE_MASK) + gmp_smem_len);

	if (off >= len) {
		gmp_log(GMP_LOG_ERR, "[gmp] gmp_device_mmap fail #2\n");
		return -EINVAL;
	}
	start &= PAGE_MASK;
	if ((vma->vm_end - vma->vm_start + off) > len) {
		gmp_log(GMP_LOG_ERR, "[gmp] gmp_device_mmap fail #3\n");
		return -EINVAL;
	}
	off += start;
	vma->vm_pgoff = off >> PAGE_SHIFT;

	vma->vm_flags |= VM_IO | VM_RESERVED;

	if (io_remap_pfn_range(vma, vma->vm_start, off >> PAGE_SHIFT,
		vma->vm_end - vma->vm_start, vma->vm_page_prot)) {
		gmp_log(GMP_LOG_ERR, "[gmp] gmp_device_mmap fail #3\n");
		return -EAGAIN;
	}

	return 0;
}

static int gmp_proc_read_status(char *page, char **start,
				   off_t off, int count,
				   int *eof, void *data)
{
	struct gmdesc *p;
	const char *tag;
	unsigned int i_used;
	unsigned int i_avail;
	int len;
	char *s;
	int ret;
	struct gmp *gmp;

	gmp = (struct gmp *) data;

	/* recovery damaged descriptor */
	if (gmp->gmdesc && gmp->gmdesc->pa != gmp->pa) {
		gmp_log(GMP_LOG_ERR, "[gmp] recovery damaged descriptors\n");
		gmp_reset(gmp);
	}

	p = gmp->gmdesc;
	s = page;
	ret = 0;
	i_used = 0;
	i_avail = 0;

	while (p) {
		tag = (const char *) &p->tag;
		len = sprintf(
			s,
			"[gmp] pa: 0x%08lx, order: %d, used: %d, "
			"tag: %c%c%c%c, owner: %d, size: %d KB\n",
			p->pa, p->order, p->used,
			tag[0], tag[1], tag[2], tag[3],
			p->owner, 1 << (p->order + GMP_SHIFT - 10));
		s += len;
		ret += len;
		if (p->used)
			i_used += 1 << (p->order + GMP_SHIFT - 10);
		else
			i_avail += 1 << (p->order + GMP_SHIFT - 10);
		p = next_gmdesc(gmp, p);
	}

	len = sprintf(s, "[gmp] used: %d KB, free: %d KB, busy: %d\n",
		i_used, i_avail, gmp_busy_flag);
	ret += len;

	return ret;
}

static int __init init_procfs_gmp(void)
{
	int ret = 0;

	gmp_procfs_dir = proc_mkdir("gmp", NULL);

	if (gmp_procfs_dir == NULL) {
		ret = -ENOMEM;
		goto out;
	}

	gmp_procfs_status_file = create_proc_read_entry("meminfo", 0444,
		gmp_procfs_dir, gmp_proc_read_status, NULL);

	if (gmp_procfs_status_file == NULL) {
		ret = -ENOMEM;
		goto no_status;
	}

	gmp_procfs_status_file->data = (void *) g_gmp;

	return 0;

no_status:			      
	remove_proc_entry("gmp", NULL);
out:
	return ret;

}

static void exit_procfs_gmp(void)
{
	remove_proc_entry("meminfo", gmp_procfs_dir);
	remove_proc_entry("gmp", NULL);
}

#endif /* __KERNEL__ */

