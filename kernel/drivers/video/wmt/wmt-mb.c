/*++ 
 * WonderMedia Memory Block driver 
 *
 * Copyright c 2010  WonderMedia  Technologies, Inc.
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
 * 10F, 529, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C
--*/
/*----------------------------------------------------------------------------
 * MODULE       : driver/video/wmt/memblock.c
 * AUTHOR       : Jason Lin
 * DATE         : 2008/12/08
 * DESCRIPTION  : Physical memory management driver
 * HISTORY      : 
 * Version 0.01 , Jason Lin, 2008/12/08
 * First version
*------------------------------------------------------------------------------*/
/* Concept:
 *          memory 		
 *          area 1			  memory
 *  (pfn)   (pages)           block    			pg_info
 *  +51F00+---------+ --> +------------+ +0 	=start
 *        |   ...   |     |    MB 0    | size	=(start-end)*page_size=50*4096
 *  +51F50+---------+     +------------+ +50	=end
 *        |    .    |  
 *        |    .    |  
 *        |    .    |     
 *  +52C00+- - - - -+     +------------+ +D00	=start
 *        |   ...   |     |    MB 1    | size	=(start-end)*page_size=400*4096
 *  +53000+---------+     +------------+ +1100	=end
 *        |         |     
 */
#ifndef MEMBLOCK_C
#define MEMBLOCK_C
#endif

#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/errno.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/mman.h>
#include <linux/list.h>
#include <linux/proc_fs.h>
#include <asm/page.h>
#include <asm/uaccess.h>
#include <asm/cacheflush.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/major.h>
#include <linux/sched.h>
#include <linux/swap.h>

#include "com-mb.h"
#include <linux/wmt-mb.h>

#define THE_MB_USER			"WMT-MB"

#define MB_DEBUG
#define MB_INFO(fmt,args...)	printk(KERN_INFO "["THE_MB_USER"] " fmt , ## args)
#define MB_WARN(fmt,args...)	printk(KERN_WARNING "["THE_MB_USER" *W*] " fmt, ## args)
#define MB_ERROR(fmt,args...)	printk(KERN_ERR "["THE_MB_USER" *E*] " fmt , ## args)
#ifdef MB_DEBUG
	#define MB_IOCTLDBG(fmt, args...)	\
		if(MBMSG_LEVEL){	\
			printk("\t"KERN_DEBUG "["THE_MB_USER"] %s: " fmt, __FUNCTION__ , ## args);	\
		}	
	#define MB_DBG(fmt, args...)	\
		if(MBMSG_LEVEL > 1){	\
			printk("\t"KERN_DEBUG "["THE_MB_USER"] %s: " fmt, __FUNCTION__ , ## args);	\
		}	
	#define MB_WDBG(fmt, args...)	\
		if(MBMSG_LEVEL > 2){	\
			printk("\t"KERN_DEBUG "["THE_MB_USER"] %s: " fmt, __FUNCTION__ , ## args);	\
		}	
#else
	#define MB_IOCTLDBG(fmt, args...)
	#define MB_DBG(fmt, args...)
	#define MB_WDBG(fmt, args...)
#endif

#ifdef CONFIG_WMT_MB_RESERVE_FROM_IO
#include <linux/fb.h>
#define MBIO_FSCREENINFO FBIOGET_FSCREENINFO
#define MBA_MAX_ORDER		20 // 2^20 pages = 4 GB
#else
#define MBA_MAX_ORDER		(MAX_ORDER - 1) // 2^10 pages = 4 MB
#endif
#define MBA_MIN_ORDER		(MAX_ORDER - 5)	// 2^6  pages = 256 KB

#define MBAFLAG_STATIC		0x80000000	// static, without release if empty

#define MBFIND_VIRT			0x00000000	// search virt address
#define MBFIND_PHYS			0x00000001	// search phys address

#define MBUFIND_USER		0x00000001	// search user address
#define MBUFIND_VIRT		0x00000002	// search kernel virt address
#define MBUFIND_PHYS		0x00000004	// search kernel phys address
#define MBUFIND_ALL			0x00000010	// search all mbu
#define MBUFIND_CREATOR		0x00000020	// search creator
#define MBUFIND_MMAP		0x00000040	// search mbu which mapping from user
#define MBUFIND_GETPUT		0x00000080	// search mbu which came from get/put
#define MBUFIND_ADDRMASK	(MBUFIND_USER|MBUFIND_VIRT|MBUFIND_PHYS)

#define MBPROC_BUFSIZE		16*1024
#define MBA_SHOW_BUFSIZE	8*1024
#define MB_SHOW_BUFSIZE		4*1024

// #define MB_IN_USE(mb)		(mb->count.counter || mb->creator || !list_empty(&mb->mbu_list))
#define MB_COUNT(mb)		atomic_read(&(mb)->count)
#define MB_IN_USE(mb)		(MB_COUNT(mb)) // count != 0
#define PAGE_KB(a)			((a)*(PAGE_SIZE/1024))
#define MB_COMBIND_MBA(h,t)	\
	(h)->pgi.pfn_end = (t)->pgi.pfn_end;	\
	(h)->pages += (t)->pages;	\
	(h)->tot_free_pages += (t)->tot_free_pages;	\
	(h)->max_available_pages += (t)->max_available_pages;	\
	list_del(&((t)->mba_list));	\
	wmt_mbah->nr_mba--;	\
	kmem_cache_free(wmt_mbah->mba_cachep, t);

struct mb_user;

struct page_info{
	unsigned long			pfn_start;	// start pfn of this memory block
	unsigned long			pfn_end;	// end pfn of this memory block
};

struct mba_host_struct{
	/* MB area link list link all MBAs */
	struct list_head		mba_list;

	/* page information of this MBA */
	unsigned int			nr_mba;

	/* total pages of all manager MBAs */
	unsigned long 			tot_pages;

	/* total static pages of all manager MBAs */
	unsigned long			tot_static_pages;

	/* total free pages of all manager MBAs */
	unsigned long			tot_free_pages;

	/* max free pages of all manager MBAs */
	unsigned long	 		max_available_pages;

	/* allocator of MBA,
	   use slab to prevent memory from fragment. */
	struct kmem_cache		*mba_cachep;

	/* allocator of MB,
	   use slab to prevent memory from fragment. */
	struct kmem_cache		*mb_cachep;

	/* allocator of mb_user,
	   use slab to prevent memory from fragment. */
	struct kmem_cache		*mbu_cachep;

	/* allocator of mb_task_info,
	   use slab to prevent memory from fragment. */
	struct kmem_cache			*mbti_cachep;
};

struct mb_area_struct{
	/* MB area link list link all MBAs */
	struct list_head		mba_list;

	/* link list link to all MBs 
       belong to this MBA */
	struct list_head		mb_list;

	/* pointer point to dedicated MBAH */
	struct mba_host_struct	*mbah;

	/* flags of MBA */
	unsigned int			flags;

	/* prefech record task */
	pid_t					tgid;	// task pid

	/* start physical address of this MBA */
	unsigned long			phys;

	/* start virtual address of this MBA */
	void					*virt;

	/* size of this MB area. Normally, 
	   MAX kernel permitted size */
	unsigned long 			pages;

	/* page information of this MBA */
	struct page_info		pgi;

	/* page information of this MBA */
	unsigned int			nr_mb;

	/* cur total free pages of this MBA */
	unsigned long 			tot_free_pages;

	/* cur max free pages of this MBA */
	unsigned long	 		max_available_pages;
};

/*
 *	element of memory block,
 *	minimal size limitation is PAGE_SIZE 
 */
struct mb_struct{
	/* MB link list link all MBs 
       in dedicated MBA */
	struct list_head		mb_list;

	/* pointer point to dedicated MBA */
	struct mb_area_struct	*mba;

	/* MB kernel page information */
	struct page_info		pgi;

	/* start physical address of this MB */
	unsigned long			phys;

	/* start virtual address of this MB */
	void					*virt;

	/* allocate size */
	unsigned long			size;

	/* current MB use count, 
       release until zero */
	atomic_t				count;

	/* point to owner created the mb.
	   this enlisted in mbu_list */
	struct mb_user			*creator;

	/* use for trace the user of mb */
	struct list_head		mbu_list;
};

/* type 0 kernel space user
		1 user space user */
// unsigned int			type:1;
/* owner 0 put/get the mb
		 1 owner who allocate the mb */
// unsigned int			owner:1;
struct mb_user{
	/* user link list link all users 
	   (include creator) of belonged MB */
	struct list_head		mbu_list;

	/* the mb to which this user belong */
	struct mb_struct		*mb;

	/* task to which user belonged,
	   user space user only */
	pid_t					tgid;	// task pid

	/* mb_mmap and MBIO_GET   : user address
	   mb_get and mb->creator : physical address */
	unsigned long			addr;

	/* kernel space user: mb size 
	   user space user:   mmap size 
	   zero: 			  owner - user space allocate but not mapped yet
	   					  not owner -come from get/put */
	unsigned long			size;

	/* user name for recoder */
	char	 				the_user[TASK_COMM_LEN+1];
};

struct mb_task_info{
	/* task link list link all tasks which use MBDev */  
	struct list_head	mbti_list;
	pid_t				tgid;	// task pid
	atomic_t			count;  // multi open record
	char	 			task_name[TASK_COMM_LEN+1];
	struct task_struct	*task;
};

#ifdef CONFIG_WMT_MB_SIZE
static int MB_TOTAL_SIZE = CONFIG_WMT_MB_SIZE * 1024;
#else
static int MB_TOTAL_SIZE = 32 * 1024;
#endif
static struct mba_host_struct *wmt_mbah = NULL;
static struct mb_task_info wmt_mbti;
struct file_operations mb_fops;
static unsigned char MBMSG_LEVEL = 0;
static unsigned char MBMAX_ORDER = 0;
static unsigned char MBMIN_ORDER = 0;
static unsigned char USR2PRDT_METHOD = 0;

/* read/write spinlock for multientry protection. */
static spinlock_t mb_do_lock;
static spinlock_t mb_search_lock;
static spinlock_t mb_ioctl_lock;
static spinlock_t mb_task_mm_lock;
static spinlock_t mb_task_lock;
static struct page *pg_user[12800]; // 12800 pages = 50 MB
static char show_mb_buffer[MB_SHOW_BUFSIZE];
static char show_mba_buffer[MBA_SHOW_BUFSIZE];

static int __user_to_prdt(
	unsigned long user, 
	unsigned int size, 
	struct prdt_struct *next,
	unsigned int items)
{
	struct mm_struct *mm = current->mm;
	struct vm_area_struct *vma;
	struct prdt_struct *prev = NULL, *ori_prdt = next;
	unsigned long from, end, pmd_end, pte_end, offset, ori_user = user;
	unsigned int idx, pgdidx, pmdidx, pteidx, prdidx, ori_size = size;
	pgd_t * pgd;
	pmd_t * pmd;
	pte_t * pte;

	if(!next || ((size/PAGE_SIZE)+2) > items){
		MB_WARN("PRD table space not enough (ptr %p at least %lu).\n",next,(size/PAGE_SIZE)+2);
		return -EINVAL;
	}

	MB_DBG("Memory(%#lx,%d) PRDT(%p,%d)\n",user,size,next,items);

	while(size > 0){
		if (!(vma = find_vma(mm, user))){
			MB_WARN("user addr %lx not found in task %s\n",user,current->comm);
			next->EDT = 1;
			goto fault;
		}

		MB_WDBG("VMA found: start %lx end %lx\n",vma->vm_start,vma->vm_end);
		from = vma->vm_start;
		end = PAGE_ALIGN(vma->vm_end);
		offset = user - from;
		pgd = pgd_offset(mm, from);
		pgdidx = prdidx = 0;
		do {	// PGD
			if(!(pmd = pmd_offset(pgd, from)))
				goto fault;
			// from start to PGDIR alignment
			pmd_end = (from + PGDIR_SIZE) & PGDIR_MASK;
			pmd_end = min(pmd_end,end);
			MB_WDBG("\t[PGD%3d] *(%p)=%lx start %lx to %lx\n",
				pgdidx++,pgd,pgd_val(*pgd),from,pmd_end);
			pmdidx = 0;
			do {	// PMD
				if(!(pte = pte_offset_map(pmd, from)))
					goto fault;
				// from start to PMD alignment
				pte_end = (from + PMD_SIZE) & PMD_MASK;
				pte_end = min(pte_end,pmd_end);
				MB_WDBG("\t\t[PMD%3d] *(%p)=%lx start %lx to %lx\n",
					pmdidx++,pmd,pmd_val(*pmd),from,pte_end);
				pteidx = 0;
				do {	// PTE
					unsigned long pfn = pte_pfn(*pte);
					struct page *pg = pfn_to_page(pfn);
					void *virt = page_address(pg);
					dma_addr_t phys = virt_to_phys(virt);

					MB_WDBG("\t\t\t[PTE%3d] PTE *(%p)=%lx pfn %lx page %p vir %p phys %x vma %lx\n",
						pteidx++,pte,pte_val(*pte),pfn,pg,virt,phys,from);
					if(offset < PAGE_SIZE){
						if(!pfn_valid(pte_pfn(*pte))){
							MB_WARN("pfn %ld of user space %lx is invalid\n",pfn,from);
							goto fault;
						}
						// Check it could combind with previous one
						if( prev && 
							(prev->size <= ((1 << 16) - (2 * PAGE_SIZE))) && // prd size boundary check, MAX 60K
							((prev->addr + prev->size) == phys)){ // page continuity check
							prev->size += min((unsigned long)size,PAGE_SIZE-offset);
						}
						else{	// create new one
							prev = next++;
							prev->addr = phys + offset;
							prev->size = min((unsigned long)size,PAGE_SIZE-offset);
							prdidx++;
							items--;
							if(!items){
								prev->EDT = 1;
								goto fault;
							}
						}
						size -= min((unsigned long)size,PAGE_SIZE-offset);;
						prev->reserve = 0;
						prev->EDT = (size)?0:1;
						MB_WDBG("\t\t\t\t[PRD %3d] %p start %x size %x edt %x rest %x\n",
							prdidx,prev,prev->addr,prev->size,prev->EDT,size);
						if(prev->EDT)
							goto finished;
						offset = 0;
					}
					else
						offset -= PAGE_SIZE;
					
					from += PAGE_SIZE;
					pte++;
#ifdef MB_WORDY
					msleep(30);
#endif
				} while (from && (from < pte_end));
				pmd++;
			} while (from && (from < pmd_end));
			pgd++;
		} while (from && (from < end));
		user = from;
	}
finished:
	MB_WDBG("PRDT %p, from %lx size %d\n",ori_prdt,ori_user,ori_size);
	for(idx = 0;;idx++){
		MB_WDBG("PRDT[%d] adddr %x size %d EDT %d\n",idx,ori_prdt[idx].addr,ori_prdt[idx].size,ori_prdt[idx].EDT);
		dmac_flush_range(__va(ori_prdt[idx].addr), __va(ori_prdt[idx].addr + ori_prdt[idx].size));
		outer_flush_range(ori_prdt[idx].addr, ori_prdt[idx].addr + ori_prdt[idx].size);
		if(ori_prdt[idx].EDT)
			break;
	}
	return 0;
fault:
	MB_WARN("USER TO PRDT unfinished, remain size %d\n",size);
	return -EFAULT;
}

static int __user_to_prdt1(
	unsigned long user, 
	unsigned int size, 
	struct prdt_struct *next,
	unsigned int items)
{
	void *ptr_start, *ptr_end, *ptr, *virt = NULL;
	int res, pg_size, pg_idx = 0, nr_pages = 0;
	struct vm_area_struct *vma;

	ptr_start = ptr = (void *)user;
	ptr_end = ptr_start + size;
	MB_INFO("Memory(%#lx,%d) PRDT(%p,%d)\n",user,size,next,items);

	vma = find_vma(current->mm, user);
	/* For kernel direct-mapped memory, take the easy way */
	if (vma && (vma->vm_flags & VM_IO) && (vma->vm_pgoff)){
		unsigned long phys = 0;
		// this will catch, kernel-allocated, mmaped-to-usermode addresses
		phys = (vma->vm_pgoff << PAGE_SHIFT) + (user - vma->vm_start);
		virt = __va(phys);
		MB_INFO("kernel-alloc, mmaped-to-user addr user %lx virt %p phys %lx",user,virt,phys);
		BUG_ON(1);
		return -EFAULT;
	}

	nr_pages = (size + (PAGE_SIZE - 1)) / PAGE_SIZE;
	if(!next || ((size/PAGE_SIZE)+2) > items || nr_pages > 2560){
		MB_WARN("PRD table space not enough (ptr %p pages %d prdt %d/%lu).\n",
			next,nr_pages,items,(size/PAGE_SIZE)+2);
		return -EINVAL;
	}

	memset(pg_user,0x0,sizeof(struct page *)*2560);
	down_read(&current->mm->mmap_sem);
	res = get_user_pages(current, current->mm, (unsigned long)ptr, nr_pages, 1, 0, pg_user, NULL);
	while(res > 0 && size > 0){
		pg_size = PAGE_SIZE - ((unsigned long)ptr & ~PAGE_MASK);
		virt = page_address(pg_user[pg_idx])+((unsigned long)ptr & ~PAGE_MASK);
		if(pg_size > size) pg_size = size;
		MB_DBG("[1]Get %d-th user page s %d/%d u %p v %p p %lx\n",pg_idx,pg_size,size,ptr,virt,__pa(virt));
		if( (next->addr + next->size) != __pa(virt) || 
			(next->size + pg_size) >= 65536 || !pg_idx){
			if(pg_idx){
				next->EDT = 0;
				next++;
			}
			memset(next,0x0,sizeof(struct prdt_struct));
			next->addr = __pa(virt);
		}
		next->size += pg_size;
		next->EDT = 1;
		size -= pg_size;
		ptr += pg_size;
		pg_idx++;
	}
	next->EDT = 1;
	up_read(&current->mm->mmap_sem);
	return size;
}

static int __user_to_prdt2(
	unsigned long user, 
	unsigned int size, 
	struct prdt_struct *next,
	unsigned int items)
{
	void *ptr_start, *ptr_end, *ptr, *virt = NULL;
	int res, pg_size, pg_idx = 0, ret = 0;
	struct vm_area_struct *vma;
	struct page *pages = NULL;

	ptr_start = ptr = (void *)user;
	ptr_end = ptr_start + size;
	MB_INFO("Memory(%#lx,%d) PRDT(%p,%d)\n",user,size,next,items);

	vma = find_vma(current->mm, user);
	/* For kernel direct-mapped memory, take the easy way */
	if (vma && (vma->vm_flags & VM_IO) && (vma->vm_pgoff)){
		unsigned long phys = 0;
		// this will catch, kernel-allocated, mmaped-to-usermode addresses
		phys = (vma->vm_pgoff << PAGE_SHIFT) + (user - vma->vm_start);
		virt = __va(phys);
		MB_INFO("kernel-alloc, mmaped-to-user addr user %lx virt %p phys %lx",user,virt,phys);
		BUG_ON(1);
		return -EFAULT;
	}

	if(!next || ((size/PAGE_SIZE)+2) > items){
		MB_WARN("PRD table space not enough (ptr %p at least %lu).\n",next,(size/PAGE_SIZE)+2);
		return -EINVAL;
	}

	while(size){
		down_read(&current->mm->mmap_sem);
		res = get_user_pages(current, current->mm, (unsigned long)ptr, 1, 1, 0, &pages, NULL);
		up_read(&current->mm->mmap_sem);
		pg_size = PAGE_SIZE - ((unsigned long)ptr & ~PAGE_MASK);
		if(res != 1){
			MB_ERROR("Get %d-th user pages (size %d/%d user %p) fail\n",pg_idx,pg_size,size,ptr);
			next->EDT = 1;
			ret = -EFAULT;
			break;
		}
		virt = page_address(&pages[0])+((unsigned long)ptr & ~PAGE_MASK);
		if(pg_size > size) pg_size = size;
		MB_DBG("Get %d-th user page s %d/%d u %p v %p p %lx\n",pg_idx,pg_size,size,ptr,virt,__pa(virt));
		if( (next->addr + next->size) != __pa(virt) || 
			(next->size + pg_size) >= 65536 || !pg_idx){
			if(pg_idx){
				next->EDT = 0;
				next++;
			}
			memset(next,0x0,sizeof(struct prdt_struct));
			next->addr = __pa(virt);
		}
		next->size += pg_size;
		next->EDT = 1;
		size -= pg_size;
		ptr += pg_size;
		pg_idx++;
	}
	return ret;
}

static void show_prdt(struct prdt_struct *next)
{
	int idx = 1;
	while(!next->EDT){
		MB_INFO("PRDT %d-th item: addr %x size %d EDT %d\n",
			idx,next->addr,next->size,next->EDT);
		idx++;
		next++;
	}
	MB_INFO("PRDT last(%d-th) item: addr %x size %d EDT %d\n",
		idx,next->addr,next->size,next->EDT);
}

// return address is guaranteeed only under page alignment
void* user_to_virt(unsigned long user)
{
	struct vm_area_struct *vma;
	unsigned long flags;
	void *virt = NULL;

	spin_lock_irqsave(&mb_task_mm_lock, flags);
	vma = find_vma(current->mm, user);
	/* For kernel direct-mapped memory, take the easy way */
	if (vma && (vma->vm_flags & VM_IO) && (vma->vm_pgoff)){
		unsigned long phys = 0;
		// this will catch, kernel-allocated, mmaped-to-usermode addresses
		phys = (vma->vm_pgoff << PAGE_SHIFT) + (user - vma->vm_start);
		virt = __va(phys);
		MB_INFO("%s kernel-alloc, mmaped-to-user addr user %lx virt %p phys %lx",__FUNCTION__,user,virt,phys);
	}
	else {
		/* otherwise, use get_user_pages() for general userland pages */
		int res, nr_pages = 1;
		struct page *pages;
		down_read(&current->mm->mmap_sem);
		res = get_user_pages(current, current->mm, user, nr_pages, 1, 0, &pages, NULL);
		up_read(&current->mm->mmap_sem);
		if (res == nr_pages)
			virt = page_address(&pages[0]) + (user & ~PAGE_MASK);
		MB_INFO("%s userland addr user %lx virt %p",__FUNCTION__,user,virt);
	}
    spin_unlock_irqrestore(&mb_task_mm_lock, flags);
	return virt;
}

int user_to_prdt(
	unsigned long user, 
	unsigned int size, 
	struct prdt_struct *next,
	unsigned int items)
{
	unsigned long flags;
	int ret;
	spin_lock_irqsave(&mb_task_mm_lock, flags);
	switch(USR2PRDT_METHOD){
		case 1:	ret = __user_to_prdt1(user, size, next, items);	break;
		case 2: ret = __user_to_prdt2(user, size, next, items);	break;
		default: ret = __user_to_prdt(user, size, next, items);	break;
	}
    spin_unlock_irqrestore(&mb_task_mm_lock, flags);
	if(MBMSG_LEVEL > 1)
		show_prdt(next);
	return ret;
}

static int mb_show_mb(
	struct mb_struct *mb, 
	char *msg, 
	char *str, 
	int size
)
{
	struct mb_user *mbu;
	char *p;

	if(!str && MBMSG_LEVEL <= 1)
		return 0;

	if(!mb)
		return 0;

	memset(show_mb_buffer,0x0,MB_SHOW_BUFSIZE);
	p = show_mb_buffer;
	p += sprintf(p,"%s%s[MB,%p] %p %08lx %4ldKB [%4lx,%4lx] %3x\n",
					msg?msg:"",msg?" ":"",mb,mb->virt,mb->phys,mb->size/1024,
					mb->pgi.pfn_start,mb->pgi.pfn_end,MB_COUNT(mb));

	if(!list_empty(&mb->mbu_list)){
		list_for_each_entry(mbu, &mb->mbu_list, mbu_list){
			char tmp[256];
			sprintf(tmp,"            [%s%s,%p] %08lx %4ldKB TGID %4x %s\n",
						(mbu==mb->creator)?"O":" ",(mbu->tgid==MB_DEF_TGID)?"K":"U",
						mbu,mbu->addr,mbu->size/1024,mbu->tgid,mbu->the_user);
			if((MB_SHOW_BUFSIZE - (p - show_mb_buffer)) < (strlen(tmp) + 1)){
				p += sprintf(p,	"\n\n               more ...\n");
				break;
			}
			p += sprintf(p,"%s",tmp);
		}
	}
	if(str){
		if(strlen(show_mb_buffer) < size)
			size = strlen(show_mb_buffer);
		strncpy(str, show_mb_buffer, size);
	}
	else{
		size = strlen(show_mb_buffer);
		MB_DBG("%s",show_mb_buffer);
	}

	return size;
}

static int mb_show_mba(
	struct mb_area_struct *mba, 
	char *msg, 
	char *str, 
	int size, 
	int follow
)
{
	struct mb_struct *mb;
	char *p;
	int idx = 1;

	if(!str && MBMSG_LEVEL <= 1)
		return 0;

	if(!mba)
		return 0;

	memset(show_mba_buffer,0x0,MBA_SHOW_BUFSIZE);

	p = show_mba_buffer;
	if(msg)
		p += sprintf(p,"%s ",msg);
	p += sprintf(p,"[%p] %p %08lx %3ldMB [%5lx,%5lx] %3x %6ldKB/%6ldKB   %s\n",
					mba,mba->virt,mba->phys,PAGE_KB(mba->pages)/1024,
					mba->pgi.pfn_start,mba->pgi.pfn_end,mba->nr_mb,
					PAGE_KB(mba->max_available_pages),
					PAGE_KB(mba->tot_free_pages),
					(mba->flags | MBAFLAG_STATIC)?"S":"D");

	if(follow){
		p += sprintf(p,	"    index -    [MemBlock] VirtAddr PhysAddr   size [  zs,  ze] cnt\n");
		list_for_each_entry(mb, &mba->mb_list, mb_list){
			if((MBA_SHOW_BUFSIZE - (p - show_mba_buffer)) < 22){
				p += sprintf(p,	"\n\n        more ...\n");
				break;
			}
			p += sprintf(p,"    %5d - ",idx++);
			p += mb_show_mb(mb,NULL,p,MBA_SHOW_BUFSIZE - (p - show_mba_buffer) - 2); // -2 is for \n and zero
		}
	}

	p += sprintf(p,"\n");

	if(str){
		if(strlen(show_mba_buffer) < size)
			size = strlen(show_mba_buffer);
		strncpy(str, show_mba_buffer, size);
	}
	else{
		size = strlen(show_mba_buffer);
		MB_DBG("%s",show_mba_buffer);
	}

	return size;
}

// return physical address
#ifdef CONFIG_WMT_MB_RESERVE_FROM_IO
// mb_do_lock locked
static void *mb_alloc_pages(unsigned long *phys, unsigned long *nr_pages)
{
	unsigned int order = get_order(num_physpages << PAGE_SHIFT);
	unsigned int size = *nr_pages << PAGE_SHIFT;
	unsigned long addr = ((1 << order) - *nr_pages) << PAGE_SHIFT;
	void *virt = NULL;

	if(!nr_pages || !phys){
		MB_WARN("mb_alloc_pages fail. unknown argument\n");
		return NULL;
	}

	*phys = 0;
	if(*nr_pages == 0 ){
		MB_WARN("mb_alloc_pages zero size\n");
		return NULL;
	}

	if(check_region(addr,size)){
		MB_WARN("allocate mem region fail. addr %#lx size %d(KB)\n", addr, size/1024);
		return NULL;
	}

	if (!request_mem_region(addr, size, "memblock")) {
		MB_WARN("request memory region failed. addr %#lx size %d(KB).\n", addr, size/1024);
		return NULL;
	}

	virt = ioremap(addr, size);
    if (!virt) {
		MB_WARN("cannot ioremap memory. addr %#lx size %d(KB).\n", addr, size/1024);
		release_mem_region(addr, size);
		return NULL;
    }

	*phys = addr;
	MB_IOCTLDBG("allocate mem region. addr V %p P %#lx size %d(KB)\n", virt, addr, size/1024);

	return virt;
}

// mb_do_lock locked
static void mb_free_pages(void *virt, unsigned long phys, unsigned int nr_pages)
{
	iounmap(virt);
	MB_IOCTLDBG("release mem region. addr V %p P %#lx size %d(KB)\n", virt, phys, 4*nr_pages);
	return release_mem_region(phys, nr_pages << PAGE_SHIFT);
}

#else
// mb_do_lock locked
static void *mb_alloc_pages(unsigned long *phys, unsigned long *nr_pages)
{
	unsigned int index, order;
	unsigned long virt = 0;
	struct zone *zone;

	if(!nr_pages || !phys){
		MB_WARN("mb_alloc_pages fail. unknown argument\n");
		return NULL;
	}

	*phys = 0;
	if(*nr_pages == 0 ){
		MB_WARN("mb_alloc_pages zero size\n");
		return NULL;
	}

	order = get_order(*nr_pages * PAGE_SIZE);
	order = max(order,(unsigned int)(MBA_MIN_ORDER));
	MB_DBG(" %ld/%d pages, order %d\n",*nr_pages, 1 << order, order);
	if(order > MBMAX_ORDER){
		MB_WARN("mb_alloc_mba fail. page out of size, %ld/%d/%d\n",
			*nr_pages,(1<<order),(1<<MBMAX_ORDER));
		return NULL;
	}

	zone = (first_online_pgdat())->node_zones;
	for(index = order; index < MAX_ORDER; index++){
		if(zone->free_area[index].nr_free){
			virt = __get_free_pages(GFP_KERNEL | GFP_DMA, order);
			break;
		}
	}

	if(virt) {
		*nr_pages = 1 << order;
		*phys = __pa(virt);
		for(index = 0; index < *nr_pages; index++){
			SetPageReserved(virt_to_page(virt + index * PAGE_SIZE));
		}
		MB_IOCTLDBG("allocate mem region. addr V %#lx P %#lx size %ld(KB)\n", 
			virt, *phys, PAGE_KB(*nr_pages));
	}
    else {
		struct free_area *fa;
		MB_WARN("__get_free_pages fail! (pages: %d free %lu)\n",
			1 << order,(unsigned long)nr_free_pages());
		zone = (first_online_pgdat())->node_zones;
		fa = zone->free_area;
		MB_WARN("DMA ZONE: %ld*4kB %ld*8kB "
			"%ld*16kB %ld*32kB %ld*64kB "
			"%ld*128kB %ld*256kB %ld*512kB "
			"%ld*1024kB %ld*2048kB %ld*4096kB "
			"= %ldkB\n",fa[0].nr_free,fa[1].nr_free,
			fa[2].nr_free,fa[3].nr_free,fa[4].nr_free,
			fa[5].nr_free,fa[6].nr_free,fa[7].nr_free,
			fa[8].nr_free,fa[9].nr_free,fa[10].nr_free,
			PAGE_KB(nr_free_pages()));
    }

	return (void *)virt;
}

// mb_do_lock locked
static void mb_free_pages(void *virt, unsigned long phys, unsigned int nr_pages)
{
	unsigned int index;

	if(!phys || !nr_pages){
		MB_WARN("mb_free_pages unknow addr V %p P %#lx size %d pages\n", virt, phys, nr_pages);
		return;
	}

	for(index = 0; index < nr_pages; index++){
		ClearPageReserved(virt_to_page((unsigned long)virt + index * PAGE_SIZE));
	}

	free_pages((unsigned long)virt, get_order(nr_pages * PAGE_SIZE));
}
#endif

// mb_do_lock locked
static struct mb_area_struct * mb_alloc_mba(unsigned int pages)
{
	struct mba_host_struct *mbah = wmt_mbah;
	struct mb_area_struct *mba;
	
	if(!mbah || !pages){
		MB_WARN("mb_alloc_mba fail. mbah %p, pages %d\n", mbah, pages);
		return NULL;
	}

	mba = kmem_cache_alloc(mbah->mba_cachep, GFP_KERNEL);
	if(!mba){
		MB_WARN("mb_alloc_mba fail. out of memory\n");
		return NULL;
	}

	memset(mba,0x0,sizeof(struct mb_area_struct));
	mba->pages = pages;
	mba->virt = mb_alloc_pages(&mba->phys,&mba->pages);
	if(!mba->virt){
		MB_WARN("mb_alloc_mba fail. no available space\n");
		kmem_cache_free(mbah->mba_cachep, mba);
		return NULL;
	}

	// initialization	
	INIT_LIST_HEAD(&mba->mb_list);
	INIT_LIST_HEAD(&mba->mba_list);
	mba->mbah = mbah;
	mba->tot_free_pages = mba->max_available_pages = mba->pages;
	mba->pgi.pfn_start = mba->phys >> PAGE_SHIFT;
	mba->pgi.pfn_end = mba->pgi.pfn_start + mba->pages;
	list_add_tail(&mba->mba_list, &mbah->mba_list);

	// update MBA host
	mbah->nr_mba++;
	mbah->tot_pages += mba->pages;
	mbah->tot_free_pages += mba->tot_free_pages;
	mbah->max_available_pages = 
		max(mbah->max_available_pages,mba->max_available_pages);
	mb_show_mba(mba,"alloc", NULL, 0, 0);

	return mba;
}

// mb_do_lock locked
static int mb_free_mba(struct mb_area_struct *mba)
{
	struct mba_host_struct *mbah;
	struct mb_area_struct *entry;

	if(!mba || !mba->mbah || mba->nr_mb){
		MB_WARN("mb_free_mba fail. unknow arg.(%p,%p,%x)\n",
			mba,mba?mba->mbah:NULL,mba?mba->nr_mb:0);
		return -EFAULT;
	}

	mbah = mba->mbah;
// safty check start
	list_for_each_entry(entry, &mbah->mba_list, mba_list){
		if(entry == mba)
			break;
	}

	if(entry != mba){
		MB_WARN("mb_free_mba fail. unknow MBA %p\n",mba);
		return -EFAULT;
	}
// safty check end
	if(mba->flags & MBAFLAG_STATIC)
		return 0;

	mb_show_mba(mba,"ReleaseMBA", NULL, 0, 0);

	// free mba
	list_del(&mba->mba_list);
	mb_free_pages(mba->virt, mba->phys,mba->pages);
	mbah->nr_mba--;
	mbah->tot_free_pages -= mba->tot_free_pages;
	mbah->tot_pages -= mba->pages;
	kmem_cache_free(mbah->mba_cachep, mba);
	mba = NULL;

	// update max mb size
	mbah->max_available_pages = 0;
	list_for_each_entry(entry, &mbah->mba_list, mba_list){
		mbah->max_available_pages = 
			max(mbah->max_available_pages,entry->max_available_pages);
	}
	
	return 0;
}

// mb_do_lock locked
static struct mb_struct * mb_alloc_mb(struct mb_area_struct *mba, unsigned long size)
{
	struct mba_host_struct *mbah;
	struct mb_struct *mb,*entry;
	struct list_head *next;
	unsigned long pages,zs,ze;

	if(!mba || !mba->mbah || !size){
		MB_WARN("mb_alloc_mb fail. unknow arg.(%p,%p,%lx)\n",
			mba,mba?mba->mbah:NULL,size);
		return NULL;
	}

	mbah = mba->mbah;
	size = PAGE_ALIGN(size);
	pages = size >> PAGE_SHIFT;

	// mb free size is not enough
	if(mba->max_available_pages < pages){
		MB_WARN("mb_alloc_mb fail. no available space in MBA (%lx<%lx)\n",
			pages,mba->max_available_pages);
		return NULL;
	}

	// search available zone
	// zone start from mba start
	ze = zs = mba->pgi.pfn_start;
	next = &mba->mb_list;
	list_for_each_entry(entry, &mba->mb_list, mb_list){
		next = &entry->mb_list;
		ze = entry->pgi.pfn_start;
		if((ze - zs) >= pages)
			break;
		zs = entry->pgi.pfn_end;
	}

	if(zs >= ze){
		next = &mba->mb_list;
		ze = mba->pgi.pfn_end;
	}

	// impossible, something wrong
	if((ze - zs) < pages){
		MB_WARN("something wrong in MBA %p when allocate MB.\n",mba);
		return NULL;
	}

	MB_DBG("Zone finding start %lx end %lx size %lx for size %lx\n",
		zs,ze,ze - zs,pages);

	mb = kmem_cache_alloc(mbah->mb_cachep, GFP_KERNEL);
	if(!mb){
		MB_WARN("mb_alloc_mb mba_cachep out of memory\n");
		return NULL;
	}

	memset(mb, 0x0, sizeof(struct mb_struct));
	INIT_LIST_HEAD(&mb->mb_list);
	INIT_LIST_HEAD(&mb->mbu_list);
	mb->mba = mba;
	mb->pgi.pfn_start = zs;
	mb->pgi.pfn_end = mb->pgi.pfn_start + pages;
	mb->size = size;
	mb->phys = mba->phys + ((zs - mba->pgi.pfn_start) << PAGE_SHIFT);
	mb->virt = mba->virt + (mb->phys - mba->phys);
	list_add_tail(&mb->mb_list, next);

	mba->nr_mb++;
	mba->tot_free_pages -= pages;
	mbah->tot_free_pages -= pages;

	// update mba
	zs = mba->pgi.pfn_start;
	mba->max_available_pages = 0;
	list_for_each_entry(entry, &mba->mb_list, mb_list){
		mba->max_available_pages = 
			max(entry->pgi.pfn_start - zs,mba->max_available_pages);
		zs = entry->pgi.pfn_end;
	}
	mba->max_available_pages = 
		max(mba->pgi.pfn_end - zs,mba->max_available_pages);

	// update mbah
	mbah->max_available_pages = 0;
	list_for_each_entry(mba, &mbah->mba_list, mba_list){
		mbah->max_available_pages = 
			max(mbah->max_available_pages,mba->max_available_pages);
	}

	mb_show_mb(mb,"alloc",NULL,0);

	return mb;
}

// mb_do_lock locked
static int mb_free_mb(struct mb_struct *mb)
{
	unsigned long zs,nr_pages;
	struct mba_host_struct *mbah;
	struct mb_area_struct *mba;
	struct mb_struct *entry;

	if(!mb){
		MB_WARN("mb_free_mb fail. unknow MB %p.\n",mb);
		return -EFAULT;
	}
	
	if(MB_IN_USE(mb)){
		MB_WARN("mb_free_mb fail. invalid arg.(%p,%x,%lx,%ldKB)\n",
			mb,MB_COUNT(mb),mb->phys,mb->size/1024);
		return -EINVAL;
	}

	mba = mb->mba;
// safty check start
	if(!mba || !mba->mbah){
		MB_WARN("mb_free_mb fail. unknow para.(%p,%p)\n",mba,mba?mba->mbah:NULL);
		return -EFAULT;
	}

	list_for_each_entry(entry, &mba->mb_list, mb_list){
		if(entry == mb)
			break;
	}

	if(entry != mb){
		MB_WARN("mb_free_mb fail. unknow MB %p\n",mb);
		return -EFAULT;
	}
// safty check end

	mbah = mba->mbah;

	mb_show_mb(mb,"Retrieve unused MB",NULL,0);

	// free mb
	list_del(&mb->mb_list);
	kmem_cache_free(mbah->mb_cachep, mb);
	mba->nr_mb--;
	mb = NULL;

	// unused mba, release it
	if(!mba->nr_mb && !(mba->flags & MBAFLAG_STATIC))
		return mb_free_mba(mba);

	// update max mb size and free mb size
	mbah->tot_free_pages -= mba->tot_free_pages;	// sub old one and then add new one
	zs = mba->pgi.pfn_start;
	mba->tot_free_pages = mba->max_available_pages = nr_pages = 0;
	list_for_each_entry(entry, &mba->mb_list, mb_list){
		nr_pages = max(entry->pgi.pfn_start - zs,nr_pages);
		mba->tot_free_pages += entry->pgi.pfn_start - zs;
		zs = entry->pgi.pfn_end;
	}
	mba->tot_free_pages += mba->pgi.pfn_end - zs;
	mba->max_available_pages = max(mba->pgi.pfn_end - zs,nr_pages);
	mbah->tot_free_pages += mba->tot_free_pages;	// add new one
	mbah->max_available_pages = 
		max(mbah->max_available_pages,mba->max_available_pages);

	return 0;
}

// type 0 - virtual address
//      1 - physical address
static struct mb_struct * mb_find_mb(void *addr, int type)
{
	struct mba_host_struct *mbah = wmt_mbah;
	struct mb_area_struct *mba;
	struct mb_struct *mb;
	unsigned long flags;

	if(!addr || (type != MBFIND_PHYS && type != MBFIND_VIRT)){
		MB_WARN("mb_find_mb fail. unknow type %d addr %p\n",type,addr);
		return NULL;
	}

	MB_DBG("IN, type %x addr %p\n",type,addr);

	spin_lock_irqsave(&mb_search_lock, flags);

	list_for_each_entry(mba, &mbah->mba_list, mba_list){
		void *saddr = (type==MBFIND_PHYS)?(void *)(mba->phys):mba->virt;
		void *eaddr = saddr + mba->pages*PAGE_SIZE;
		if(addr < saddr || addr > eaddr)
			continue; // address out of mba range
		list_for_each_entry(mb, &mba->mb_list, mb_list){
			void *mbaddr = (type==MBFIND_PHYS)?(void *)(mb->phys):mb->virt;
			if(addr >= mbaddr && addr < (mbaddr + mb->size)){
				spin_unlock_irqrestore(&mb_search_lock, flags);
				MB_DBG("OUT, mb vaddr %p paddr %lx size %ld(KB) cnt %d\n",
					mb->virt,mb->phys,mb->size/1024,MB_COUNT(mb));
				return mb;
			}
		}
	}

	spin_unlock_irqrestore(&mb_search_lock, flags);

	MB_DBG("OUT, NULL mb\n");

	return NULL;
}

/*
addr - search address
tgid - task pid
st   - search type (MBUFIND_XXX)
	MBUFIND_USER: user address
	MBUFIND_VIRT: kernel virt address
	MBUFIND_PHYS: kernel phys address
	MBUFIND_ALL:		
		1. mbu->tgid == tgid 
		2. tgid == MB_DEF_TGID, address and one of MBUFIND_VIRT and MBUFIND_PHYS must be set
		   tgid != MB_DEF_TGID, address and MBUFIND_USER must be set
	MBUFIND_CREATOR:
		1. mbu->tgid == tgid 
		2. tgid == MB_DEF_TGID, address and one of MBUFIND_VIRT and MBUFIND_PHYS must be set
		   tgid != MB_DEF_TGID, address and MBUFIND_USER must be set
		3. mbu == mb->creator
		4. mbu->size == mb->size
	MBUFIND_GETPUT:
		1. mbu->tgid == tgid
		2. tgid == MB_DEF_TGID, address and MBUFIND_PHYS must be set (mbu->addr == mb->phys)
		   tgid != MB_DEF_TGID, address and MBUFIND_USER must be set (mbu->addr == user address)
		3. mbu->size == 0
	MBUFIND_MMAP:
		1. mbu->tgid == tgid
		2. address and MBUFIND_USER must be set
		3. mbu->size != 0
*/
static struct mb_user * mb_find_mbu(void *addr, pid_t tgid, int st)
{
	struct mba_host_struct *mbah = wmt_mbah;
	struct mb_area_struct *mba = NULL;
	struct mb_struct *mb = NULL;
	struct mb_user *mbu = NULL;
	unsigned long flags;

	if(!addr || !(st & MBUFIND_ADDRMASK)){
		MB_WARN("mb_find_mbu fail. unknow addr %p\n",addr);
		return NULL;
	}

	MB_DBG("IN, addr %p search type %x TGID %x CurTask %s\n",
		addr,st,tgid,current->comm);

	spin_lock_irqsave(&mb_search_lock, flags);
	list_for_each_entry(mba, &mbah->mba_list, mba_list){
		if(st & (MBUFIND_VIRT | MBUFIND_PHYS)){
			void *saddr = (st&MBUFIND_PHYS)?(void *)(mba->phys):mba->virt;
			void *eaddr = saddr + mba->pages*PAGE_SIZE;
			if(addr < saddr || addr > eaddr)
				continue; // address out of mba range
		}
		list_for_each_entry(mb, &mba->mb_list, mb_list){
			if((st & MBUFIND_PHYS) && (addr != (void *)mb->phys))
				continue; // virtual address not match
			if((st & MBUFIND_VIRT) && (addr != mb->virt))
				continue; // physical address not match
			list_for_each_entry(mbu, &mb->mbu_list, mbu_list){
				if(mbu->tgid != tgid) continue; // tgid not match
				if((st & MBUFIND_USER) && (addr != (void *)mbu->addr))
					continue; // user address not match
				if(st & MBUFIND_ALL)
					goto leave;
				if(st & MBUFIND_CREATOR){
					mbu = mb->creator;
					goto leave;
				}
				if(st & MBUFIND_MMAP && mbu->size)
					goto leave;
				if(st & MBUFIND_GETPUT && mbu->addr && !mbu->size)
					goto leave;
			}
		}
	}
    mbu = NULL;

leave:
	if(mbu){
		MB_DBG("OUT, mbu %p(mb %p TGID %x addr %#lx size %ld(KB) user %s)\n",
			mbu,mbu->mb,mbu->tgid,mbu->addr,mbu->size/1024,mbu->the_user);
	}
	else{
		MB_DBG("OUT, NULL mbu\n");
	}

	spin_unlock_irqrestore(&mb_search_lock, flags);

	return mbu;
}

static void mb_update_mbah(void)
{
	unsigned long mbah_free = 0, mbah_max = 0;
	struct mba_host_struct *mbah = wmt_mbah;
	struct mb_area_struct *mba = NULL;
	struct mb_struct *mb = NULL;

	if(!mbah){
		MB_WARN("mb_update_mbah fail. unknow mbah\n");
		return;
	}

	list_for_each_entry(mba, &mbah->mba_list, mba_list){
		unsigned long mba_free = 0, mba_max = 0, nr_pages = 0;
		unsigned long zs = mba->pgi.pfn_start; // zone start
		list_for_each_entry(mb, &mba->mb_list, mb_list){
			nr_pages = max(mb->pgi.pfn_start - zs,nr_pages);
			mba_free += mb->pgi.pfn_start - zs;
			zs = mb->pgi.pfn_end;
		}
		mba_free += mba->pgi.pfn_end - zs;
		mbah_free += mba_free;
		mba_max = max(mba->pgi.pfn_end - zs,nr_pages);
		mbah_max = max(mbah_max,mba_max);
	}

	mbah->tot_free_pages = mbah_free;
	mbah->max_available_pages = mbah_max;

	return;
}

// called from kernel only, mb_do_lock isn't needed
void *mb_do_phys_to_virt(unsigned long phys, pid_t tgid, char *name)
{
	struct mb_struct *mb = NULL;
	void *virt = NULL;

	MB_DBG("IN, Phys %lx TGID %x NAME %s\n", phys, tgid, name);
	mb = mb_find_mb((void *)phys, MBFIND_PHYS);
	if(mb)
		virt = mb->virt + (phys - mb->phys);
	else{
		virt = __va(phys);
		MB_WARN("mb_do_phys_to_virt fail. addr %lx not found\n",phys);
	}

	MB_DBG("OUT, Virt %p\n",virt);

	return virt;
}

// called from kernel only, mb_do_lock isn't needed
unsigned long mb_do_virt_to_phys(void *virt, pid_t tgid, char *name)
{
	struct mb_struct *mb = NULL;
	unsigned long phys = 0x0;

	MB_DBG("IN, Virt %p TGID %x NAME %s\n", virt, tgid, name);
	mb = mb_find_mb(virt, MBFIND_VIRT);
	if(mb)
		phys = mb->phys + (virt - mb->virt);
	else{
		phys = __pa(virt);
		MB_WARN("mb_do_virt_to_phys fail. addr %p not found\n",virt);
	}

	MB_DBG("OUT, Phys %lx\n",phys);

	return phys;
}

unsigned long mb_do_user_to_phys(unsigned long user, pid_t tgid, char *name)
{
	unsigned long flags, phys = 0;
	struct mb_user *mbu = NULL;

	MB_DBG("IN, usr %lx TGID %x name %s\n", user, tgid, name);

	spin_lock_irqsave(&mb_do_lock, flags);
	mbu = mb_find_mbu((void *)user,tgid,MBUFIND_USER|MBUFIND_ALL);
	if(!mbu){
		spin_unlock_irqrestore(&mb_do_lock, flags);
		MB_WARN("mb_do_user_to_phys fail. unknow addr %lx\n",user);
		return 0;
	}

	if(!mbu->mb)
		MB_WARN("mb_do_user_to_phys fail. unknow memory block.\n");
	else
		phys = mbu->mb->phys;

	spin_unlock_irqrestore(&mb_do_lock, flags);

	MB_DBG("OUT, Phys %#lx\n", phys);

	return phys;
}

unsigned long mb_do_user_to_virt(	unsigned long user,	pid_t tgid, char *name)
{
	struct mb_user *mbu = NULL;
	unsigned long flags;
	void *virt = NULL;

	MB_DBG("IN, usr %lx TGID %x name %s\n", user, tgid, name);

	spin_lock_irqsave(&mb_do_lock, flags);
	mbu = mb_find_mbu((void *)user,tgid,MBUFIND_USER|MBUFIND_ALL);
	if(!mbu){
		spin_unlock_irqrestore(&mb_do_lock, flags);
		MB_WARN("mb_do_user_to_virt fail. unknow addr %lx\n",user);
		return 0;
	}

	if(!mbu->mb)
		MB_WARN("mb_do_user_to_virt fail. unknow memory block.\n");
	else
		virt = mbu->mb->virt;

	spin_unlock_irqrestore(&mb_do_lock, flags);

	MB_DBG("OUT, Virt %p\n", virt);

	return (unsigned long)virt;
}

// physical address
int mb_do_counter(unsigned long addr, char *name)
{
	struct mb_struct *mb;
	unsigned long flags;
	int counter = -1;

	MB_DBG("IN, addr %#lx name %s\n",addr,name);

	if(!addr || !wmt_mbah){
		MB_WARN("mb_do_counter fail. invalid paramaters %p/%lx/%s\n",wmt_mbah,addr,name);
		return -EINVAL;
	}
	spin_lock_irqsave(&mb_do_lock, flags);
	mb = mb_find_mb((void *)addr, MBFIND_PHYS);
	counter = (mb)?MB_COUNT(mb):-1;
	spin_unlock_irqrestore(&mb_do_lock, flags);

	MB_DBG("OUT, addr %#lx count %d\n", addr, counter);

	return counter;
}

unsigned long mb_do_alloc(unsigned long size, pid_t tgid, char *name)
{
	struct mb_area_struct *entry,*mba = NULL;
	struct mb_struct *mb = NULL;
	struct mb_user *mbu = NULL;
	unsigned long flags,addr;
	unsigned int pages;
	size_t ns; // name size

	if(!name){
		MB_WARN("mb_alloc fail. null user %s tgid %x\n", name, tgid);
		return 0;
	}

	size = PAGE_ALIGN(size);
	pages = size >> PAGE_SHIFT;

	if(!pages){
		MB_WARN("mb_alloc unavailable size %x(KB)\n",4*pages);
		return 0;
	}

	mbu = kmem_cache_alloc(wmt_mbah->mbu_cachep, GFP_KERNEL);
	if(!mbu){
		MB_WARN("mbu_cachep out of memory.\n");
		return 0;
	}
	memset(mbu,0x0,sizeof(struct mb_user));

	MB_DBG("IN, TGID %x size %ld(KB) name %s\n",tgid,size/1024,name);

	spin_lock_irqsave(&mb_do_lock, flags);

	if(pages > wmt_mbah->max_available_pages){
#ifdef CONFIG_WMT_MB_DYNAMIC_ALLOCATE_SUPPORT
		if(!(mba = mb_alloc_mba(pages)))
			MB_WARN("mb_alloc create MBA fail, tot %u/%lu/%lu/%lu mba %d\n",
				pages,wmt_mbah->max_available_pages,wmt_mbah->tot_free_pages,
				wmt_mbah->tot_pages,wmt_mbah->nr_mba);
#else
		MB_DBG("MB DYNAMIC ALLOCATED is not suported, tot %u/%lu/%lu/%lu pages\n",
			pages,wmt_mbah->max_available_pages,wmt_mbah->tot_free_pages,
			wmt_mbah->tot_pages);
		goto error;
#endif
	}
	else{
		list_for_each_entry(entry, &wmt_mbah->mba_list, mba_list){
			if(entry->max_available_pages >= pages){
				mba = entry;
				break;
			}
		}
	}
	
	if(!mba){
		MB_WARN("mb_alloc dedicated MBA not found\n");
		goto error;
	}

	if(!(mb = mb_alloc_mb(mba, size))){
		MB_WARN("mb_alloc create MB fail\n");
		goto error;
	}

	INIT_LIST_HEAD(&mbu->mbu_list);
	mbu->mb = mb;
	mbu->tgid = tgid;
	mbu->size = mb->size;
	// mbu->addr = 0; // address of creator is 0
	ns = min((size_t)TASK_COMM_LEN,strlen(name));
	strncpy(mbu->the_user, name, ns);
	mbu->the_user[ns] = 0;
	list_add_tail(&mbu->mbu_list, &mb->mbu_list);
	atomic_inc(&mb->count);
	mb->creator = mbu;
	addr = mb->phys;
	spin_unlock_irqrestore(&mb_do_lock, flags);

	MB_DBG("OUT, Addr %lx TGID %x size %ld(KB) name %s\n",addr,tgid,size/1024,name);

	return addr;

error:

	kmem_cache_free(wmt_mbah->mbu_cachep, mbu);
	spin_unlock_irqrestore(&mb_do_lock, flags);
	return 0;
}

// physical address
int mb_do_free(unsigned long addr, pid_t tgid, char *name)
{
	struct mb_struct *mb;
	struct mb_user *mbu;
	unsigned long flags;
	int ret;

	if(!addr || !name){
		MB_WARN("mb_free fail. invalid paramaters %lx/%s\n",addr,name);
		return -EINVAL;
	}

	MB_DBG("IN, TGID %x addr %#lx name %s\n",tgid,addr,name);

	spin_lock_irqsave(&mb_do_lock, flags);
	mbu = mb_find_mbu((void *)addr,tgid,MBUFIND_PHYS|MBUFIND_CREATOR);
	if(!mbu || !mbu->mb){
		spin_unlock_irqrestore(&mb_do_lock, flags);
		MB_WARN("mb_free unknow addr %lx\n",addr);
		return -EINVAL;
	}

	if(strncmp(mbu->the_user,name,min(strlen(name),(size_t)TASK_COMM_LEN))){
		MB_DBG("Owner no match. (%s/%s)\n", mbu->the_user, name);
	}

	mb = mbu->mb;

	list_del(&mbu->mbu_list);
	kmem_cache_free(wmt_mbah->mbu_cachep, mbu);
	mb->creator = NULL;
	atomic_dec(&mb->count);
	if(MB_IN_USE(mb)){
		MB_DBG("<mb_free, MB %8lx still in use. Cnt %d>\n",mb->phys,MB_COUNT(mb));
		spin_unlock_irqrestore(&mb_do_lock, flags);
		return 0;
	}

	ret = mb_free_mb(mb);
	spin_unlock_irqrestore(&mb_do_lock, flags);

	MB_DBG("OUT\n");

	return ret;
}

int mb_do_get(unsigned long addr, pid_t tgid, char *name)
{
	struct mb_user *mbu = NULL, *newmbu;
	struct mb_struct *mb = NULL;
	unsigned long flags;
	size_t ns; // name size

	if(!addr || !name){
		MB_WARN("mb_do_get fail. invalid paramaters %lx/%s\n",addr,name);
		return -EINVAL;
	}

	newmbu = kmem_cache_alloc(wmt_mbah->mbu_cachep, GFP_KERNEL);
	if(!newmbu){
		MB_DBG("<mbu_cachep out of memory.>\n");
		return -ENOMEM;
	}

	MB_DBG("IN, TGID %x addr %#lx name %s\n",tgid,addr,name);

	spin_lock_irqsave(&mb_do_lock, flags);
	if(tgid != MB_DEF_TGID){ // find user address, only exist on MMAP
		mbu = mb_find_mbu((void *)addr, tgid, MBUFIND_USER|MBUFIND_MMAP);
		mb = (mbu)?mbu->mb:NULL;
	}
	else
		mb = mb_find_mb((void *)addr, MBFIND_PHYS);

	if(!mb){
		spin_unlock_irqrestore(&mb_do_lock, flags);
		kmem_cache_free(wmt_mbah->mbu_cachep, newmbu);
		MB_WARN("mb_get unknown mb addr %8lx name %s\n",addr,name);
		return -EFAULT;
	}

	memset(newmbu,0x0,sizeof(struct mb_user));
	INIT_LIST_HEAD(&newmbu->mbu_list);
	newmbu->addr = addr;
	newmbu->mb = mb;
	newmbu->tgid = tgid; // if come from kernel function call, TGID should be default
	ns = min((size_t)TASK_COMM_LEN,strlen(name));
	strncpy(newmbu->the_user, name, ns);
	newmbu->the_user[ns] = 0;
	atomic_inc(&mb->count);
	list_add_tail( &newmbu->mbu_list, &mb->mbu_list);

	spin_unlock_irqrestore(&mb_do_lock, flags);

	MB_DBG("out, [mbu] addr %lx %s [mb] addr %lx cnt %d\n",
		addr,newmbu->the_user,mb->phys,MB_COUNT(mb));

	return 0;
}

int mb_do_put(unsigned long addr, pid_t tgid, char *name)
{
	struct mb_struct *mb;
	struct mb_user *mbu;
	unsigned long flags;

	if(!addr || !name){
		MB_WARN("mb_do_put fail. invalid paramaters %lx/%s\n",addr,name);
		return -EINVAL;
	}

	MB_DBG("IN, TGID %x addr %#lx name %s\n",tgid,addr,name);

	spin_lock_irqsave(&mb_do_lock, flags);
	if(tgid == MB_DEF_TGID)
		mbu = mb_find_mbu((void *)addr,tgid,MBUFIND_PHYS|MBUFIND_GETPUT);
	else
		mbu = mb_find_mbu((void *)addr,tgid,MBUFIND_USER|MBUFIND_GETPUT);

	if(!mbu){
		spin_unlock_irqrestore(&mb_do_lock, flags);
		MB_WARN("mb_put unknow virt %8lx name %s\n",addr,name);
		return -EINVAL;
	}

	mb = mbu->mb;
	if(!MB_IN_USE(mb)){
		spin_unlock_irqrestore(&mb_do_lock, flags);
		MB_WARN("memory block %p unbalance.\n",mb);
		return -EPERM;
	}

	list_del_init(&mbu->mbu_list);
	atomic_dec(&mb->count);
	// retrieve unused memory block
	if(!MB_IN_USE(mb))
		mb_free_mb(mb);

	spin_unlock_irqrestore(&mb_do_lock, flags);
	kmem_cache_free(wmt_mbah->mbu_cachep, mbu);

	MB_DBG("out, [mbu] addr %lx %s [mb] addr %lx cnt %d\n",
		addr,mbu->the_user,mb->phys,MB_COUNT(mb) - 1);

	return 0;
}

#define DEVICE_NAME "Memory Block"

static int mb_dev_major = MB_MAJOR;
static int mb_dev_minor = 0;
static int mb_dev_nr = 1;
static struct cdev *mb_cdev = NULL;

static int mb_open(struct inode *inode, struct file *filp)
{
	struct mb_task_info *mbti = NULL;
	unsigned long flags;
	int task_exist = 0;
	size_t ns;

	if(filp->private_data){
		MB_ERROR("none empty private data.\n");
		return -EFAULT;
	}

	spin_lock_irqsave(&mb_task_lock, flags);
	list_for_each_entry(mbti, &wmt_mbti.mbti_list, mbti_list){
		if(mbti->task && mbti->tgid == current->tgid){
			task_exist = 1;
			break;
		}
	}
	if(!task_exist){
		mbti = kmem_cache_alloc(wmt_mbah->mbti_cachep, GFP_KERNEL);
		if(!mbti){
			spin_unlock_irqrestore(&mb_task_lock, flags);
			MB_ERROR("out of memory (mb_task_info).\n");
			return -EFAULT;
		}
		memset(mbti,0x0,sizeof(struct mb_task_info));
		INIT_LIST_HEAD(&mbti->mbti_list);
		mbti->task = current;
		mbti->tgid = current->tgid;
		ns = min((size_t)TASK_COMM_LEN,strlen(current->comm));
		strncpy(mbti->task_name, current->comm, ns);
		mbti->task_name[ns] = 0;
		atomic_set(&mbti->count,0);
		list_add_tail(&mbti->mbti_list, &wmt_mbti.mbti_list);
	}
	atomic_inc(&mbti->count);
	MB_DBG("mb driver is opened by task(%p) %s TGID %x count %d.\n",
		current,current->comm,current->tgid,atomic_read(&(mbti->count)));
	filp->private_data = (void *)mbti;
	spin_unlock_irqrestore(&mb_task_lock, flags);

	return 0;
}

static int mb_release(struct inode *inode, struct file *filp)
{
	struct mb_area_struct *mba;
	struct mb_struct *mb;
	struct mb_user *mbu;
	struct mb_task_info *cmbti = NULL, *mbti = NULL;
	unsigned long flags, tflags;
	int task_exist = 0;

	cmbti = (struct mb_task_info*)(filp->private_data);
	if(!cmbti){
		MB_ERROR("none empty private data.\n");
		return -EFAULT;
	}

	spin_lock_irqsave(&mb_task_lock, tflags);
	list_for_each_entry(mbti, &wmt_mbti.mbti_list, mbti_list){
		if(mbti->task && mbti->tgid == cmbti->tgid){
			atomic_dec(&mbti->count);
			task_exist = 1;
			break;
		}
	}
	if(!task_exist){
		spin_unlock_irqrestore(&mb_task_lock, tflags);
		MB_INFO("mb driver is closed by unknown task %s TGID %x.\n",
			current->comm,current->tgid);
		return 0;
	}

	MB_DBG("mb driver is closed by task %s TGID %x cnt %d.\n",
		mbti->task_name,mbti->tgid,atomic_read(&mbti->count));
	if(atomic_read(&mbti->count)){
		spin_unlock_irqrestore(&mb_task_lock, tflags);
		return 0;
	}

	spin_lock_irqsave(&mb_ioctl_lock, flags);
RESCAN_MBA:
	// munmap virtual memroy and retrieve unused MB
	list_for_each_entry(mba, &wmt_mbah->mba_list, mba_list){
#ifdef CONFIG_WMT_MB_DYNAMIC_ALLOCATE_SUPPORT
		if(mba->tgid == mbti->tgid){ // free prefetched mba marker
			wmt_mbah->tot_static_pages -= mba->pages;
			mba->flags &= ~(MBAFLAG_STATIC);
			mba->tgid = MB_DEF_TGID;
			if(!mba->nr_mb){
				mb_free_mba(mba);
				goto RESCAN_MBA; // ugly jump coz mba link is dirty
			}
		}
#endif		
		list_for_each_entry(mb, &mba->mb_list, mb_list){
RESCAN_MBU:
			list_for_each_entry(mbu, &mb->mbu_list, mbu_list){
				if(mbu->tgid == mbti->tgid && mb->creator != mbu){
					MB_INFO("Stop mb operation (op \"%s\" addr %lx size %lu(KB) acted by %s) (Cnt %d)\n",
						(mbu->size)?"mmap":"get",mbu->addr,mbu->size/1024,mbu->the_user,MB_COUNT(mb)-1);
					list_del_init(&mbu->mbu_list);
					kmem_cache_free(wmt_mbah->mbu_cachep, mbu);
					atomic_dec(&mb->count);
					goto RESCAN_MBU;
				}
			}
			if(MB_COUNT(mb) == 1 && mb->creator != NULL && mb->creator->tgid == mbti->tgid){
				mbu = mb->creator;
				MB_INFO("Recycle mb addr %lx size %lu allocated by %s\n",
					mb->phys,mb->size,mb->creator->the_user);
				mb_do_free(mb->phys,mbti->tgid,mb->creator->the_user);
				goto RESCAN_MBA; // ugly jump coz mba link is dirty
			}
		}
	}

	mb_update_mbah();
	list_del(&mbti->mbti_list);
	kmem_cache_free(wmt_mbah->mbti_cachep, mbti);
	filp->private_data = NULL;
	spin_unlock_irqrestore(&mb_ioctl_lock, flags);
	spin_unlock_irqrestore(&mb_task_lock, tflags);

	return 0;
}

static int mb_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct mb_task_info *mbti;
	struct mb_struct *mb = NULL;
	struct mb_user *mbu = NULL;
	unsigned long value,flags,phys,virt,size;
	pid_t tgid = MB_DEF_TGID;
	int ret = 0;

	/* check type and number, if fail return ENOTTY */
#ifdef CONFIG_WMT_MB_RESERVE_FROM_IO
	if( _IOC_TYPE(cmd) != MB_IOC_MAGIC && cmd != FBIOGET_FSCREENINFO )	return -ENOTTY;
#else
	if( _IOC_TYPE(cmd) != MB_IOC_MAGIC )	return -ENOTTY;
#endif

	/* check argument area */
	if( _IOC_DIR(cmd) & _IOC_READ )
		ret = !access_ok( VERIFY_WRITE, (void __user *) arg, _IOC_SIZE(cmd));
	else if ( _IOC_DIR(cmd) & _IOC_WRITE )
		ret = !access_ok( VERIFY_READ, (void __user *) arg, _IOC_SIZE(cmd));

	if( ret )	return -EFAULT;

	/* check task validation */
	if(!filp || !filp->private_data){
		MB_ERROR("mmap with null file info or task.\n");
		return -EFAULT;
	}

	mbti = (struct mb_task_info*)(filp->private_data);
	tgid = mbti->tgid;
	if(tgid != current->tgid)
		MB_WARN("ioctl btw diff tasks Open: task %s TGID %x Curr: task %s TGID %x.\n",
			mbti->task_name,tgid,current->comm,current->tgid);

	switch(cmd){
		// _IOWR(MB_IOC_MAGIC, 0, unsigned long) 
		// 		I: size					
		//		O: physical address
		case MBIO_MALLOC:
			get_user(value, (unsigned long *)arg);
			phys = mb_do_alloc(value,tgid,mbti->task_name);
			if(!phys){
				MB_DBG("MBIO_MALLOC size %ld fail.\n\n",value);
				return -EFAULT;
			}
			MB_IOCTLDBG("<MBIO_MALLOC addr %lx size %ld>\n\n",phys,value);
			put_user(phys, (unsigned long *)arg);
			break;
		
		// _IOWR(MB_IOC_MAGIC, 1, unsigned long) 
		// 		I: user address if map, physical address if not map		
		//		O: ummap size
		case MBIO_FREE: // if not unmap, unmap first
			get_user(value, (unsigned long *)arg);
			spin_lock_irqsave(&mb_ioctl_lock, flags);
			size = value;
			if((mbu = mb_find_mbu((void *)value,tgid,MBUFIND_USER|MBUFIND_MMAP)) != NULL){
				size = mbu->size;
				value = mbu->mb->phys;
				list_del(&mbu->mbu_list);
				atomic_dec(&mbu->mb->count);
				kmem_cache_free(wmt_mbah->mbu_cachep, mbu);
			}
			else{
				MB_DBG("MBIO_FREE. (MMAP addr %lx not found)\n",value);
			}
			ret = mb_do_free(value,tgid,mbti->task_name);
			spin_unlock_irqrestore(&mb_ioctl_lock, flags);
			if(ret < 0)
				MB_WARN("MBIO_FREE fail (addr %lx not found)\n",value);
			put_user(size, (unsigned long *)arg);	// return mmap size for ummap
			MB_IOCTLDBG("<MBIO_FREE addr %lx out %ld>\n\n",value,size);
			break;

		// _IOWR(MB_IOC_MAGIC, 2, unsigned long) 
		// 		I: user address			
		//		O: ummap size
		case MBIO_UNMAP:
			get_user(value, (unsigned long *)arg);
			spin_lock_irqsave(&mb_ioctl_lock, flags);
			if((mbu = mb_find_mbu((void *)value,tgid,MBUFIND_USER|MBUFIND_MMAP)) == NULL){
				spin_unlock_irqrestore(&mb_ioctl_lock, flags);
				MB_WARN("MBIO_UNMAP fail (addr %lx not found)\n\n",value);
				return -EFAULT;
			}
			mb = mbu->mb;
			size = mbu->size;
			list_del(&mbu->mbu_list);
			kmem_cache_free(wmt_mbah->mbu_cachep, mbu);
			atomic_dec(&mb->count);
			if(MB_COUNT(mb) <= 0){
				MB_WARN("<MBIO_UNMAP, MB %8lx count %d weird>\n\n",mb->phys,MB_COUNT(mb));
			}
			spin_unlock_irqrestore(&mb_ioctl_lock, flags);
			put_user(size, (unsigned long *)arg);	// return mmap size for ummap
			return 0;
/*
			if(MB_IN_USE(mb)){
				spin_unlock_irqrestore(&mb_ioctl_lock, flags);
				put_user(size, (unsigned long *)arg);	// return mmap size for ummap
				MB_DBG("<MBIO_UNMAP, MB %8lx still in use.>\n",mb->phys);
				return 0;
			}
			ret = mb_free_mb(mb);
			phys = mb->phys;
			spin_unlock_irqrestore(&mb_ioctl_lock, flags);
			MB_IOCTLDBG("<MBIO_UNMAP addr %lx/%lx size %lx>\n",value,phys,size);
			put_user(size, (unsigned long *)arg);	// return mmap size for ummap
			break;
*/			

		// _IOWR(MB_IOC_MAGIC, 3, unsigned long) 
		// 		I: phys address			
		//		O: mb size
		case MBIO_MBSIZE:
			get_user(value, (unsigned long *)arg);
			spin_lock_irqsave(&mb_ioctl_lock, flags);
			if((mb = mb_find_mb((void *)value,MBFIND_PHYS)) == NULL){
				spin_unlock_irqrestore(&mb_ioctl_lock, flags);
				MB_WARN("MBIO_UNMAP user %lx fail. (addr not found)\n\n",value);
				return -EFAULT;
			}
			size = mb->size;
			spin_unlock_irqrestore(&mb_ioctl_lock, flags);
			MB_IOCTLDBG("<MBIO_MBSIZE addr %lx size %ld>\n\n",value,size);
			put_user(size, (unsigned long *)arg);	// return mb size
			break;

		// _IOR (MB_IOC_MAGIC, 4, unsigned long) 
		// 		O: max free mba size
		case MBIO_MAX_AVAILABLE_SIZE:
			put_user(wmt_mbah->max_available_pages*PAGE_SIZE, (unsigned long *)arg);
			MB_IOCTLDBG("<MBIO_MAX_MB_SIZE. size %ld KB>\n\n",
				PAGE_KB(wmt_mbah->max_available_pages));
			break;

		// _IOW (MB_IOC_MAGIC, 5, unsigned long) 
		// 		I: user address
		case MBIO_GET:
			get_user(value, (unsigned long *)arg);
			ret = mb_do_get(value,tgid,mbti->task_name);
			if(MBMSG_LEVEL){
				spin_lock_irqsave(&mb_ioctl_lock, flags);
				mbu = mb_find_mbu((void *)value,tgid,MBUFIND_USER|MBUFIND_GETPUT);
				MB_IOCTLDBG("<MBIO_GET. addr %lx cnt %x>\n\n",
					value,mbu?MB_COUNT(mbu->mb):0xffffffff);
				spin_unlock_irqrestore(&mb_ioctl_lock, flags);
			}
			break;

		// _IOW (MB_IOC_MAGIC, 6, unsigned long) 
		// 		I: user address
		case MBIO_PUT:
			get_user(value, (unsigned long *)arg);
			if(MBMSG_LEVEL){
				spin_lock_irqsave(&mb_ioctl_lock, flags);
				mbu = mb_find_mbu((void *)value,tgid,MBUFIND_USER|MBUFIND_GETPUT);
				MB_IOCTLDBG("<MBIO_PUT. virt %lx cnt %x>\n\n",
					value,mbu?(MB_COUNT(mbu->mb) - 1):0xffffffff);
				spin_unlock_irqrestore(&mb_ioctl_lock, flags);
			}
			ret = mb_do_put(value,tgid,mbti->task_name);
			break;

		// _IOWR(MB_IOC_MAGIC, 7, unsigned long) 
		// 		I: user address		
		//		O: virt address
		case MBIO_USER_TO_VIRT:
			get_user(value, (unsigned long *)arg);
			virt = mb_do_user_to_virt(value, tgid, mbti->task_name);
			put_user(virt, (unsigned long *)arg);
			MB_IOCTLDBG("<MBIO_USER_TO_VERT. user %8lx virt %lx>\n\n",
				value,virt);
			break;

		// _IOWR(MB_IOC_MAGIC, 8, unsigned long) 
		// 		I: user address		
		//		O: phys address
		case MBIO_USER_TO_PHYS:
			get_user(value, (unsigned long *)arg);
			phys = mb_do_user_to_phys(value, tgid, mbti->task_name);
			put_user(phys, (unsigned long *)arg);
			MB_IOCTLDBG("<MBIO_USER_TO_PHYS. user %8lx phys %lx>\n\n",
				value,phys);
			break;

#ifdef CONFIG_WMT_MB_RESERVE_FROM_IO
		// This IOCTRL is provide AP to recognize MB region as framebuffer,
		// then can access mb memory directly.
		case MBIO_FSCREENINFO:
		{
			struct fb_fix_screeninfo ffs;
			struct mb_area_struct *mba = NULL;

			if(!wmt_mbah || list_empty(&wmt_mbah->mba_list)){
				MB_WARN("MBIO_FSCREENINFO: MB driver does not init.\n");
				return -EFAULT;
			}

			mba = (struct mb_area_struct *)wmt_mbah->mba_list.next;
			ffs.smem_start = mba->phys;
			ffs.smem_len = wmt_mbah->tot_static_pages*PAGE_SIZE;
			ret = copy_to_user((void __user *)arg, &ffs, sizeof(ffs)) ? -EFAULT : 0;
			MB_IOCTLDBG("<MBIO_FSCREENINFO. phys %lx len %x>\n",
                ffs.smem_start,ffs.smem_len);
			break;
		}
#endif

		// _IOW (MB_IOC_MAGIC, 9, unsigned long) 
		// 		I: size
		case MBIO_PREFETCH:
		{
			unsigned long fetch_size = 0;
			get_user(value, (unsigned long *)arg);
#ifdef CONFIG_WMT_MB_DYNAMIC_ALLOCATE_SUPPORT
{
			unsigned long try_size = 0;
			struct mb_area_struct *mba;
			
			MB_IOCTLDBG("<MBIO_PREFETCH. size %ld KB>\n",value/1024);
			spin_lock_irqsave(&mb_ioctl_lock, flags);
			while(try_size < value){
				int order;
				order = get_order(value-fetch_size);
				order = min(order,(int)MBMAX_ORDER);
				try_size += (1<<order)*PAGE_SIZE;
				mba = mb_alloc_mba(1 << order);
				if(mba){
					mba->flags |= MBAFLAG_STATIC;
					mba->tgid = current->tgid;
					wmt_mbah->tot_static_pages += mba->pages;
					fetch_size += mba->pages * PAGE_SIZE;
				}
			}
			spin_unlock_irqrestore(&mb_ioctl_lock, flags);
			MB_INFO("Pre-fetch BUFFER for %s (SIZE %ld / %ld kB)\n",
				current->comm,fetch_size/1024,value/1024);
}
#else
			MB_INFO("Pre-fetch BUFFER for %s (SIZE %ld/%ld kB)(Dynamic Allocated Not Support)\n\n",
				current->comm,fetch_size/1024,value/1024);
#endif
			break;
		}
			
		// _IOW (MB_IOC_MAGIC,10, unsigned long) 
		// 		O: static mba size
		case MBIO_STATIC_SIZE:
			put_user(wmt_mbah->tot_static_pages*PAGE_SIZE, (unsigned long *)arg);
			MB_IOCTLDBG("<MBIO_STATIC_SIZE. size %ld KB>\n\n",
				PAGE_KB(wmt_mbah->tot_static_pages));
			break;

		// _IOWR(MB_IOC_MAGIC,11, unsigned long) 
		// 		I: physical address	
		//		O: use counter
		case MBIO_MB_USER_COUNT:
			get_user(value, (unsigned long *)arg);

			spin_lock_irqsave(&mb_ioctl_lock, flags);
			mb = mb_find_mb((void *)value,MBFIND_PHYS);
			if(mb == NULL)
				size = 0;
			else
				size = MB_COUNT(mb);
			spin_unlock_irqrestore(&mb_ioctl_lock, flags);

			put_user(size, (unsigned long *)arg);
			MB_IOCTLDBG("<MBIO_MB_USER_COUNT. addr %lx count %ld>\n\n",value,size);
			break;

		// _IO  (MB_IOC_MAGIC,12)
		case MBIO_FORCE_RESET:
		{
			struct mb_area_struct *mba;
			spin_lock_irqsave(&mb_ioctl_lock, flags);
RESCAN_MBA:
			list_for_each_entry(mba, &wmt_mbah->mba_list, mba_list){
				list_for_each_entry(mb, &mba->mb_list, mb_list){
					list_for_each_entry(mbu, &mb->mbu_list, mbu_list){
						list_del(&mbu->mbu_list);
						kmem_cache_free(wmt_mbah->mbu_cachep, mbu);
					}
					mb->creator = NULL;
					atomic_set(&mb->count, 0);
					mb_free_mb(mb);
					goto RESCAN_MBA;	// because mba link maybe breaken
				}
			}
			mb_update_mbah();
			spin_unlock_irqrestore(&mb_ioctl_lock, flags);
			MB_IOCTLDBG("<MBIO_FORCE_RESET OK.>\n\n");
			break;
		}

		default:
			ret = -ENOTTY;
			break;
	}

	return ret;
}

static int mb_mmap(struct file *filp, struct vm_area_struct *vma)
{
	struct mb_user *mbu = NULL;
	struct mb_task_info *mbti;
	struct mb_struct *mb;
	unsigned long off, flags;
	unsigned int len;
	pid_t tgid;
	size_t ns;
		
	if(!filp || !filp->private_data){
		MB_ERROR("mmap with null file info or task.\n");
		return -EFAULT;
	}

	mbti = (struct mb_task_info*)(filp->private_data);
	tgid = mbti->tgid;
	if(tgid != current->tgid)
		MB_WARN("mmap btw diff tasks Open: task %s TGID %x Curr: task %s TGID %x.\n",
			mbti->task_name,tgid,current->comm,current->tgid);

	off = vma->vm_pgoff << PAGE_SHIFT;
	MB_DBG("IN, offset %lx TGID %x name %s\n",off,tgid,mbti->task_name);
#ifdef CONFIG_WMT_MB_RESERVE_FROM_IO
	/* if page offset under MB total size, it means page offset 
	   is not physical address from MB_alloc and then map function 
	   works like framebuffer. 
	   Suppose MB total size will not cross MBAH phys start address */
	if(vma->vm_pgoff <= wmt_mbah->tot_static_pages){
		unsigned long mbtotal = wmt_mbah->tot_static_pages << PAGE_SHIFT;
		struct mb_area_struct *mba = NULL;
		if((vma->vm_end - vma->vm_start + off) > mbtotal){
			MB_ERROR("mmap io fail. (out of range s %lx e %lx off %lx len %lx)\n",
				vma->vm_start,vma->vm_end,off,mbtotal);
			return -EINVAL;
		}
		mba = (struct mb_area_struct *)wmt_mbah->mba_list.next;
		vma->vm_pgoff += mba->phys >> PAGE_SHIFT;
		vma->vm_flags |= VM_IO | VM_RESERVED;
		if (io_remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff,
			vma->vm_end - vma->vm_start, vma->vm_page_prot)) {
			MB_ERROR("mmap io fail. (io_remap_pfn_range fail pfn %lx len %lx)\n",
				vma->vm_pgoff,vma->vm_end - vma->vm_start);
			return -EAGAIN;
		}
		MB_IOCTLDBG("<MB_MAP IO from %lx to %lx/%lx size %ld>\n\n",
			vma->vm_pgoff << PAGE_SHIFT,vma->vm_start,vma->vm_end,
			vma->vm_end - vma->vm_start);
		return 0;
	}
#endif

	mbu = kmem_cache_alloc(wmt_mbah->mbu_cachep, GFP_KERNEL);
	if(!mbu){
		MB_ERROR("mbu_cachep out of memory.\n");
		return -ENOMEM;
	}

	spin_lock_irqsave(&mb_ioctl_lock, flags);
	mb = mb_find_mb((void *)off,MBFIND_PHYS);
	if(!mb){
		spin_unlock_irqrestore(&mb_ioctl_lock, flags);
		kmem_cache_free(wmt_mbah->mbu_cachep, mbu);
		MB_ERROR("map addr %lx not exist in MB.\n",off);
		return -EINVAL;
	}

	len = PAGE_ALIGN(mb->phys + mb->size);
	if ((vma->vm_end - vma->vm_start + off) > len){
		spin_unlock_irqrestore(&mb_ioctl_lock, flags);
		kmem_cache_free(wmt_mbah->mbu_cachep, mbu);
		return -EINVAL;
	}

	vma->vm_flags |= VM_IO | VM_RESERVED;
	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	if (io_remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff,
			     vma->vm_end - vma->vm_start, vma->vm_page_prot)){
		spin_unlock_irqrestore(&mb_ioctl_lock, flags);
		kmem_cache_free(wmt_mbah->mbu_cachep, mbu);
		MB_ERROR("virtual address memory map fail.\n");
		return -EAGAIN;
	}

	memset(mbu,0x0,sizeof(struct mb_user));
	INIT_LIST_HEAD(&mbu->mbu_list);
	mbu->tgid = tgid;
	mbu->mb = mb;
	mbu->addr = vma->vm_start;
	mbu->size = vma->vm_end - vma->vm_start;
	ns = min(strlen(mbti->task_name),(size_t)TASK_COMM_LEN);
	strncpy(mbu->the_user, mbti->task_name, ns);
	mbu->the_user[ns] = 0;
	list_add_tail(&mbu->mbu_list,&mb->mbu_list);
	atomic_inc(&mb->count);

	off = mb->phys;
	spin_unlock_irqrestore(&mb_ioctl_lock, flags);

	MB_IOCTLDBG("<MB_MAP from %lx to %lx/%lx size %ld mbcnt %d>\n\n",
		off,vma->vm_start,vma->vm_end,(vma->vm_end - vma->vm_start),MB_COUNT(mb));
	
	return 0;
}

/*!*************************************************************************
	driver file operations struct define
****************************************************************************/
struct file_operations mb_fops = {
	.owner = THIS_MODULE,
	.open = mb_open,
	.release = mb_release,
	.ioctl = mb_ioctl,
	.mmap = mb_mmap,
};

static int mb_probe(struct platform_device *dev)
{
	struct mb_area_struct *mba = NULL, *bind = NULL;
	unsigned long flags;
	int ret = 0;
	dev_t dev_no;

	dev_no = MKDEV(mb_dev_major,mb_dev_minor);

	if(wmt_mbah){
		MB_ERROR("%s dirty.\n",DEVICE_NAME);
		return -EINVAL;
	}

	/* register char device */
	mb_cdev = cdev_alloc();
	if(!mb_cdev){
		MB_ERROR("alloc dev error.\n");
		return -ENOMEM;
	}

	cdev_init(mb_cdev,&mb_fops);
	ret = cdev_add(mb_cdev,dev_no,1);

	if(ret){
		MB_ERROR("reg char dev error(%d).\n",ret);
		cdev_del(mb_cdev);
		return ret;
	}

	wmt_mbah = (struct mba_host_struct *)kmalloc(sizeof(struct mba_host_struct), GFP_KERNEL);
	if(!wmt_mbah){
		MB_ERROR("out of memory (mb_area_struct).\n");
		cdev_del(mb_cdev);
		return -ENOMEM;
	}
	memset(wmt_mbah, 0x0, sizeof(struct mba_host_struct));

	INIT_LIST_HEAD(&wmt_mbah->mba_list);
	wmt_mbah->mba_cachep = kmem_cache_create("mb_area_struct", 
											 sizeof(struct mb_area_struct),
											 0, 
											 SLAB_HWCACHE_ALIGN,
											 NULL);
	if(!wmt_mbah->mba_cachep){
		MB_ERROR("out of memory (mba_cachep).\n");
		kfree(wmt_mbah);
		cdev_del(mb_cdev);
		return -ENOMEM;
	}

	INIT_LIST_HEAD(&wmt_mbah->mba_list);
	wmt_mbah->mb_cachep = kmem_cache_create("mb_struct", 
											sizeof(struct mb_struct),
											0, 
											SLAB_HWCACHE_ALIGN,
											NULL);
	if(!wmt_mbah->mb_cachep){
		MB_ERROR("out of memory (mb_cachep).\n");
		kmem_cache_destroy(wmt_mbah->mba_cachep);
		kfree(wmt_mbah);
		cdev_del(mb_cdev);
		return -ENOMEM;
	}

	wmt_mbah->mbu_cachep = kmem_cache_create("mb_user", 
											sizeof(struct mb_user),
											0, 
											SLAB_HWCACHE_ALIGN,
											NULL);
	if(!wmt_mbah->mbu_cachep){
		MB_ERROR("out of memory (mbu_cachep).\n");
		kmem_cache_destroy(wmt_mbah->mb_cachep);
		kmem_cache_destroy(wmt_mbah->mba_cachep);
		kfree(wmt_mbah);
		cdev_del(mb_cdev);
		return -ENOMEM;
	}

	wmt_mbah->mbti_cachep = kmem_cache_create("mb_task_info", 
											sizeof(struct mb_task_info),
											0, 
											SLAB_HWCACHE_ALIGN,
											NULL);
	if(!wmt_mbah->mbti_cachep){
		MB_ERROR("out of memory (mbti_cachep).\n");
		kmem_cache_destroy(wmt_mbah->mbu_cachep);
		kmem_cache_destroy(wmt_mbah->mb_cachep);
		kmem_cache_destroy(wmt_mbah->mba_cachep);
		kfree(wmt_mbah);
		cdev_del(mb_cdev);
		return -ENOMEM;
	}

	MBMAX_ORDER = MBA_MAX_ORDER;
	MBMIN_ORDER = MBA_MIN_ORDER;
	INIT_LIST_HEAD(&wmt_mbti.mbti_list);

    spin_lock_init(&mb_do_lock);
    spin_lock_init(&mb_search_lock);
    spin_lock_init(&mb_ioctl_lock);
    spin_lock_init(&mb_task_mm_lock);
    spin_lock_init(&mb_task_lock);

	MB_INFO("Preparing VIDEO BUFFER (SIZE %d kB) ...\n",MB_TOTAL_SIZE);
	spin_lock_irqsave(&mb_do_lock,flags);
	while(PAGE_KB(wmt_mbah->tot_static_pages) < MB_TOTAL_SIZE){
		int pages;
		pages = (MB_TOTAL_SIZE*1024 - wmt_mbah->tot_static_pages*PAGE_SIZE) >> PAGE_SHIFT;
		pages = min(pages,(int)(1<<MBMAX_ORDER));
		mba = mb_alloc_mba(pages);
		if(mba){
			mba->flags |= MBAFLAG_STATIC;
			mba->tgid = MB_DEF_TGID;
			wmt_mbah->tot_static_pages += mba->pages;
			// conbine to continue mba if possible
			if(bind && bind->pgi.pfn_start == mba->pgi.pfn_end){
				MB_COMBIND_MBA(mba,bind);
				bind = mba;
			}
			else if(bind && bind->pgi.pfn_end == mba->pgi.pfn_start){
				MB_COMBIND_MBA(bind,mba);
			}
			else
				bind = mba;
		}
	}
	spin_unlock_irqrestore(&mb_do_lock,flags);

	mb_update_mbah();
	MB_INFO("MAX MB Area size: Max %ld Kb Min %ld Kb\n",
		PAGE_KB(1 << MBMAX_ORDER),PAGE_KB(1 << MBMIN_ORDER));

	MB_INFO("prob /dev/%s major %d, minor %d\n", 
		DEVICE_NAME, mb_dev_major,mb_dev_minor);

	return ret;
}

static int mb_remove(struct platform_device *dev)
{
	if(mb_cdev)
		cdev_del(mb_cdev);

	if(wmt_mbah){
		if(wmt_mbah->mba_cachep)
			kmem_cache_destroy(wmt_mbah->mba_cachep);
		if(wmt_mbah->mb_cachep)
			kmem_cache_destroy(wmt_mbah->mb_cachep);
		if(wmt_mbah->mbu_cachep)
			kmem_cache_destroy(wmt_mbah->mbu_cachep);
		if(wmt_mbah->mbti_cachep)
			kmem_cache_destroy(wmt_mbah->mbti_cachep);
		kfree(wmt_mbah);
	}

	MB_INFO("MB dev remove \n");
	return 0;
}

static int mb_suspend(struct platform_device *dev, pm_message_t state)
{
	MB_DBG("MB get suspend, Event %d.\n",state.event);
	return 0;
}

static int mb_resume(struct platform_device *dev)
{
	MB_DBG("MB get resume.\n");
	return 0;
}

int mb_area_read_proc( 
	char *buf, char **start, off_t off, 
	int count, int *eof, void *data
)
{
	struct mb_area_struct *mba;
	struct free_area *fa;
	struct zone *zone;
	unsigned long flags;
	unsigned int idx = 1;
    char *p = buf, *base = (char *)data;
	int datalen = 0,len;

	if(!data || !count){
		p += sprintf(p, "no resource for mb_area read proc. (%p,%d)\n", data, count);
		return p-buf;
	}

	if(!wmt_mbah){
		p += sprintf(p, "no MB area host existed.\n");
		return p-buf;
	}

	spin_lock_irqsave(&mb_ioctl_lock, flags);
	if(!off){ // re read mb area information
		p = base;
		memset(p,0x0,MBPROC_BUFSIZE);

		// show memory block information
		p += sprintf(p,"MESSAGE LEVEL:    %8d   /%8lx\n",MBMSG_LEVEL,__pa(&MBMSG_LEVEL));
		p += sprintf(p,"STATIC MB SIZE:     %3ld MB   /  %3d MB\n",
					PAGE_KB(wmt_mbah->tot_static_pages)/1024,MB_TOTAL_SIZE/1024);
		p += sprintf(p,"MAX MBA ORDER:	  %8d   /%8lx\n",
					MBMAX_ORDER,__pa(&MBMAX_ORDER));
		p += sprintf(p,"MIN MBA ORDER:	  %8d   /%8lx\n\n",
					MBMIN_ORDER,__pa(&MBMIN_ORDER));
		p += sprintf(p,"USER TO PRDT METHOD: %8d   /%8lx\n\n",
					USR2PRDT_METHOD,__pa(&USR2PRDT_METHOD));
		p += sprintf(p,"total MB areas:  %8d\n",wmt_mbah->nr_mba);
		p += sprintf(p,"total size:      %8ld kB\n",PAGE_KB(wmt_mbah->tot_pages));
		p += sprintf(p,"total free size: %8ld kB\n",PAGE_KB(wmt_mbah->tot_free_pages));
		p += sprintf(p,"max MB size:     %8ld kB\n\n",PAGE_KB(wmt_mbah->max_available_pages));
		
		list_for_each_entry(mba, &wmt_mbah->mba_list, mba_list){ 
			p += sprintf(p, "(ID) [MB Area] VirtAddr PhysAddr  size [   zs,   ze]"
							" MBs      Max/    Free Ste\n");
			len = (int)(p-base);
			if((MBPROC_BUFSIZE - len) < 12){
				p += sprintf(p, " more ...\n");
				break;
			}
			p += sprintf(p,"(%2d)",idx++);
			len = (int)(p-base);
			// show all MBs
			p += mb_show_mba(mba, NULL, p, MBPROC_BUFSIZE - len - 2, 1); // -2 is for \n and zero
			p += sprintf(p,"\n");
		}
		// show memory fragment
		zone = (first_online_pgdat())->node_zones;
		fa = zone->free_area;
		p += sprintf( p, "DMA ZONE:\n");
		for(idx = 0; idx < MAX_ORDER; idx++){
			p += sprintf( p, " %5ld * %5ldkB = %5ld kB\n",
						fa[idx].nr_free,PAGE_KB((1<<idx)),fa[idx].nr_free*PAGE_KB((1<<idx)));
		}
		p += sprintf( p, " ------------------------------\n");
		p += sprintf( p, "               + = %ld kB\n\n",PAGE_KB((unsigned long)nr_free_pages()));
	}
	spin_unlock_irqrestore(&mb_ioctl_lock, flags);
	datalen = strlen(base);
	if(off >= datalen){
		*eof = 1;
		return 0;
	}
	len = min((int)(datalen - off),count);
	memcpy(buf, &base[off], len);
	*start = (char *)len; // for case1: *start < page, mass read data

    return len;
}

static struct platform_driver mb_driver = {
	.driver.name    = "wmt_mb", // This name should equal to platform device name.
	.probe          = mb_probe,
	.remove         = mb_remove,
	.suspend        = mb_suspend,
	.resume         = mb_resume
};

static void mb_platform_release(struct device *device)
{
	return;
}

static struct platform_device mb_device = {
	.name           = "wmt_mb",
	.id             = 0,
	.dev            = 	{	.release =  mb_platform_release,
#if 0
							.dma_mask = &spi_dma_mask,
							.coherent_dma_mask = ~0,
#endif
						},
	.num_resources  = 0,		/* ARRAY_SIZE(spi_resources), */
	.resource       = NULL,		/* spi_resources, */
};

extern int wmt_getsyspara(char *varname, unsigned char *varval, int *varlen);
static int __init mb_init(void)
{
	int varlen = 32, ret = -1, mbtotal = 0;
	unsigned char buf[32];
	dev_t dev_no;

	if( !mb_dev_major ){
		MB_ERROR("unknow %s device.(%d)\n",DEVICE_NAME,mb_dev_major);
		return -ENODEV;
	}

	dev_no = MKDEV(mb_dev_major,mb_dev_minor);
	ret = register_chrdev_region(dev_no,mb_dev_nr,"wmt_mb");
	if( ret < 0 ){
		MB_ERROR("can't get %s device major %d\n",DEVICE_NAME,mb_dev_major);
		return ret;
	}

	if (wmt_getsyspara("mbsize", buf, &varlen) == 0) {
		sscanf(buf,"%dM",&mbtotal);
		mbtotal *= 1024;
		MB_INFO("Set MB total size %d KB\n",mbtotal);	
		if(mbtotal > 0)
			MB_TOTAL_SIZE = mbtotal;
	}
	
#ifdef CONFIG_PROC_FS
{
	void * proc_buffer = kmalloc(MBPROC_BUFSIZE, GFP_KERNEL); // for save proc data
	struct proc_dir_entry *res = NULL;
	if(proc_buffer){
		memset(proc_buffer,0x0,MBPROC_BUFSIZE);
		res = create_proc_entry("mbinfo", 0, NULL);
		if (res) {
			res->read_proc = mb_area_read_proc;
			res->data = proc_buffer;
			MB_DBG("create MB proc\n");
		}
		else
			kfree(proc_buffer);
	}
}
#endif

	ret = platform_driver_register(&mb_driver);
	if (!ret) {
		ret = platform_device_register(&mb_device);
		if(ret)
			platform_driver_unregister(&mb_driver);
	}

	return ret;
}

// because mb will not exit from system, not need to release proc resource
static void __exit mb_exit(void)
{
	dev_t dev_no;

	platform_driver_unregister(&mb_driver);
	platform_device_unregister(&mb_device);
	dev_no = MKDEV(mb_dev_major,mb_dev_minor);
	unregister_chrdev_region(dev_no,mb_dev_nr);

	return;
}

EXPORT_SYMBOL(user_to_virt);
EXPORT_SYMBOL(user_to_prdt);
EXPORT_SYMBOL(mb_do_virt_to_phys);
EXPORT_SYMBOL(mb_do_phys_to_virt);
EXPORT_SYMBOL(mb_do_user_to_virt);
EXPORT_SYMBOL(mb_do_user_to_phys);
EXPORT_SYMBOL(mb_do_alloc);
EXPORT_SYMBOL(mb_do_free);
EXPORT_SYMBOL(mb_do_put);
EXPORT_SYMBOL(mb_do_get);
EXPORT_SYMBOL(mb_do_counter);

fs_initcall(mb_init);
module_exit(mb_exit);

MODULE_AUTHOR("WonderMedia Technologies, Inc.");
MODULE_DESCRIPTION("memory block driver");
MODULE_LICENSE("GPL");
