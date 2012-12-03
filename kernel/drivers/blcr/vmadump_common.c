/*-------------------------------------------------------------------------
 *  vmadump.c:  Virtual Memory Area dump/restore routines
 *
 *  Copyright (C) 1999-2001 by Erik Hendriks <erik@hendriks.cx>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id: vmadump_common.c,v 1.86 2008/12/17 03:29:10 phargrov Exp $
 *
 * THIS VERSION MODIFIED FOR BLCR <http://ftg.lbl.gov/checkpoint>
 *-----------------------------------------------------------------------*/
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>

#include <linux/sched.h>

#include <linux/slab.h>
#include <linux/file.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/binfmts.h>
#include <linux/smp_lock.h>
#include <linux/unistd.h>
#include <linux/personality.h>
#include <linux/highmem.h>
#include <linux/ptrace.h>
#ifdef HAVE_LINUX_SYSCALLS_H
  #include <linux/syscalls.h>	/* for mprotect, etc. */
#endif
#include <linux/init.h>
#include <linux/prctl.h>
#include <asm/pgtable.h>
#include <asm/pgalloc.h>
#include <asm/processor.h>
#include <asm/uaccess.h>
#include <asm/bitops.h>
#include <asm/siginfo.h> //add by jay
#define __VMADUMP_INTERNAL__
#include "vmadump.h"

static char vmad_magic[3] = VMAD_MAGIC;

MODULE_AUTHOR("Erik Hendriks <erik@hendriks.cx> and the BLCR Team http://ftg.lbl.gov/checkpoint");
MODULE_DESCRIPTION("VMADump - Virtual Memory Area Dumper (modified for " PACKAGE_STRING ")");
MODULE_LICENSE("GPL");

/* A note about symbols...
 *
 * This module requires the following extra symbols from the kernel:
 *
 * sys_mprotect
 * do_exit
 * do_sigaction
 * do_execve
 * [and more w/ BLCR...]
 */

# if 0 /* BLCR uses its own I/O paths instead */
/*--------------------------------------------------------------------
 *  Some utility stuff for reading and writing and misc kernel syscalls.
 *------------------------------------------------------------------*/
static
ssize_t default_read_file(struct vmadump_map_ctx *ctx, struct file *file,
			  void *buf, size_t count) {
    return vfs_read(file, buf, count, &file->f_pos);
}

static
ssize_t read_user(struct vmadump_map_ctx *ctx,
		  struct file *file, void *buf, size_t count) {
    ssize_t r, bytes = count;
    ssize_t (*rfunc)(struct vmadump_map_ctx *ctx, struct file *file,
		     void *buf, size_t count);
    rfunc = (ctx && ctx->read_file) ? ctx->read_file : default_read_file;
    while (bytes) {
#if BITS_PER_LONG == 32
	/* Avoid accidentally negative return values */
	r = rfunc(ctx, file, buf, ((bytes > 0x40000000) ? 0x40000000 : bytes));
#elif BITS_PER_LONG == 64
	r = rfunc(ctx, file, buf, bytes);
#else
	#error "Unknown BITS_PER_LONG"
#endif  
	if (r < 0)  return r;
	if (r == 0) return count - bytes;
	bytes -= r; buf = (char *)buf + r;
    }
    return count;
}

ssize_t read_kern(struct vmadump_map_ctx *ctx,
		  struct file *file, void *buf, size_t count) {
    ssize_t err;
    mm_segment_t oldfs;
    oldfs = get_fs(); set_fs(KERNEL_DS);
    err = read_user(ctx, file, buf, count);
    set_fs(oldfs);
    return err;
}

static
ssize_t default_write_file(struct vmadump_map_ctx *ctx, struct file *file,
			   const void *buf, size_t count) {
    return vfs_write(file, buf, count, &file->f_pos);
}

static
ssize_t write_user(struct vmadump_map_ctx *ctx, struct file *file,
		   const void *buf, size_t count) {
    ssize_t w, bytes = count;
    ssize_t (*wfunc)(struct vmadump_map_ctx *ctx, struct file *file,
		     const void *buf, size_t count);
    wfunc = (ctx && ctx->write_file) ? ctx->write_file : default_write_file;
    while (bytes) {
#if BITS_PER_LONG == 32
	/* Avoid accidentally negative return values */
	w = wfunc(ctx, file, buf, ((bytes > 0x40000000) ? 0x40000000 : bytes));
#elif BITS_PER_LONG == 64
	w = wfunc(ctx, file, buf, bytes);
#else  
	#error "Unknown BITS_PER_LONG"
#endif  
	if (w < 0)  return w;
	if (w == 0) return count - bytes;
	bytes -= w; buf = (char *)buf + w;
    }
    return count;
}

ssize_t write_kern(struct vmadump_map_ctx *ctx, struct file *file,
		   const void *buf, size_t count) {
    ssize_t err;
    mm_segment_t oldfs;
    oldfs = get_fs(); set_fs(KERNEL_DS);
    err = write_user(ctx, file, buf, count);
    set_fs(oldfs);
    return err;
}
#endif

#if 0 /* Not needed/maintained for BLCR */
/*--------------------------------------------------------------------
 *  Library list stuff.
 *------------------------------------------------------------------*/
/* XXX This should be more configurable than this - this is idea of
 *     figuring out what's a library  */
struct liblist_entry {
    struct list_head list;
    char libname[0];
};

static LIST_HEAD(liblist);

static
void liblist_clear(void) {
    struct liblist_entry *entry;
    while (!list_empty(&liblist)) {
	entry = list_entry(liblist.next, struct liblist_entry, list);
	list_del(liblist.next);
	kfree(entry);
    }
}

static
struct liblist_entry *
liblist_find(const char *filename) {
    struct list_head *p;
    struct liblist_entry *entry;
    p = liblist.next;
    while (p != &liblist) {
	entry = list_entry(p, struct liblist_entry, list);
	if (strcmp(entry->libname, filename) == 0)
	    return entry;
	p = p->next;
    }
    return 0;
}

static
int liblist_add(char *filename) {
    struct liblist_entry *entry;

    /* Don't add things twice */
    if (liblist_find(filename)) return 0;

    entry = kmalloc(sizeof(*entry)+strlen(filename)+1, GFP_KERNEL);
    if (!entry) return -ENOMEM;
    strcpy(entry->libname, filename);
    list_add(&entry->list, &liblist);
    return 0;
}

static
int liblist_del(char *filename) {
    struct liblist_entry *entry;
    entry = liblist_find(filename);
    if (entry) {
	list_del(&entry->list);
	kfree(entry);
	return 0;
    }
    return -ENOENT;
}

/* Returns the size of our library list converted to text */
static
int liblist_size(void) {
    int len;
    struct list_head *p;
    struct liblist_entry *entry;

    len = 0;
    p = liblist.next;
    while (p != &liblist) {
	entry = list_entry(p, struct liblist_entry, list);
	len += strlen(entry->libname)+1;
	p = p->next;
    }
    len++;			/* for trailing null. */
    return len;
}

static
int do_lib_op(int request, char *buf, int size) {
    int err, len;
    char *filename;
    struct list_head *p;
    struct liblist_entry *entry;

    switch(request) {
    case VMAD_LIB_CLEAR:
	if (!capable(CAP_SYS_ADMIN)) return -EPERM;
	liblist_clear();
	return 0;

    case VMAD_LIB_ADD:
	if (!capable(CAP_SYS_ADMIN)) return -EPERM;
	filename = getname(buf);
	if (IS_ERR(filename)) return PTR_ERR(filename);
	err = liblist_add(filename);
	putname(filename);
	return err;

    case VMAD_LIB_DEL:
	if (!capable(CAP_SYS_ADMIN)) return -EPERM;
	filename = getname(buf);
	if (IS_ERR(filename)) return PTR_ERR(filename);
	err = liblist_del(filename);
	putname(filename);
	return err;

    case VMAD_LIB_SIZE:
	return liblist_size();

    case VMAD_LIB_LIST:
	len = liblist_size();
	if (len > size) return -ENOSPC;
	size = len;
	/* Copy all those strings out to user space. */
	p = liblist.next;
	while (p != &liblist) {
	    entry = list_entry(p, struct liblist_entry, list);
	    len = strlen(entry->libname);
	    if (copy_to_user(buf, entry->libname, len)) return EFAULT;
	    buf += len;
	    put_user('\0', buf++);
	    p = p->next;
	}
	put_user('\0', buf);
	return size;

    default:
	return -EINVAL;
    }
}
#endif

static
char *default_map_name(struct file *f, char *buffer, int size) {
#if HAVE_NAMEIDATA_DENTRY
    return d_path(f->f_dentry, f->f_vfsmnt, buffer, size);
#elif HAVE_NAMEIDATA_PATH
    return d_path(&f->f_path, buffer, size);
#else
    #error
#endif
}


#if 0 /* Not needed/maintained for BLCR */
/* this is gonna be handled with contexts too */
static
int is_library(const char *filename) {
    return (liblist_find(filename) != 0);
}
#else
  #define is_library(FILE) 0
#endif


static
struct file *default_map_open(cr_rstrt_proc_req_t *ctx, const char *filename, int flags)
{
	struct file *filp;
	const char *reloc_filename;

	reloc_filename = cr_relocate_path(ctx->req->relocate, filename, 0);
	if (IS_ERR(reloc_filename)) {
		filp = (struct file *)reloc_filename;
		goto out;
	}

	filp = filp_open(reloc_filename, flags, 0);
	if (IS_ERR(filp)) {
		if (reloc_filename != filename) {
			CR_ERR_CTX(ctx, "failed to open '%s', relocated from '%s'",
				reloc_filename, filename);
		}
		/* Caller prints flags and return value */
	}
	if (reloc_filename != filename) {
		__putname(reloc_filename);
	}

out:
	return filp;
}


#if 0 /* Hooks code not needed/maintained for BLCR */
/*--------------------------------------------------------------------
 *  Dump hook stuff.
 *------------------------------------------------------------------*/
struct hook_t {
    struct list_head list;
    struct vmadump_hook *hook;
};
struct vmadump_hook_handle {
    int rw;			/* for checking on the user */
    struct vmadump_map_ctx *ctx;
    struct file *file;
};

static struct rw_semaphore hook_lock;
static LIST_HEAD(hooks);

int vmadump_add_hook(struct vmadump_hook *hook) {
    struct hook_t *h, *new_h;
    struct list_head *l;

    new_h = kmalloc(sizeof(*new_h), GFP_KERNEL);
    new_h->hook = hook;

    down_write(&hook_lock);
    /* check to make sure that this hook isn't being added twice */
    for (l = hooks.next; l != &hooks; l = l->next) {
	h = list_entry(l, struct hook_t, list);
	if (h->hook == hook) {
	    up_write(&hook_lock);
	    kfree(new_h);
	    return -EEXIST;
	}
    }
    list_add_tail(&new_h->list, &hooks);
    up_write(&hook_lock);
    printk(KERN_INFO "vmadump: Registered hook \"%s\"\n", hook->tag);
    return 0;
}

int vmadump_del_hook(struct vmadump_hook *hook) {
    struct hook_t *h;
    struct list_head *l;

    down_write(&hook_lock);
    for (l = hooks.next; l != &hooks; l = l->next) {
	h = list_entry(l, struct hook_t, list);

	if (h->hook == hook) {
	    list_del(&h->list);
	    up_write(&hook_lock);
	    printk(KERN_INFO "vmadump: Unregistered hook \"%s\"\n", hook->tag);
	    kfree(h);
	    return 0;
	}
    }
    up_write(&hook_lock);
    return -ENOENT;
}

struct vmadump_callback_handle {
    int rw;
    struct vmadump_map_ctx *ctx;
    struct file *file;
};

/* Call every hook freeze function */
static
int do_freeze_hooks(struct vmadump_map_ctx *ctx, struct file *file,
		    struct pt_regs *regs, int flags) {
    long bytes = 0, r;
    struct hook_t *h;
    struct list_head *l;
    struct vmadump_hook_header hookhdr;
    struct vmadump_hook_handle hh = { 1, ctx, file};

    down_read(&hook_lock);
    for (l = hooks.next; l != &hooks; l = l->next) {
	h = list_entry(l, struct hook_t, list);
	r = h->hook->freeze(&hh, regs, flags);
	if (r < 0) {
	    up_read(&hook_lock);
	    return r;
	}
	bytes += r;
    }
    up_read(&hook_lock);

    /* Terminate the list of hooks */
    memset(&hookhdr, 0, sizeof(hookhdr));
    r = write_kern(ctx, file, &hookhdr, sizeof(hookhdr));
    if (r < 0) return r;
    if (r != sizeof(hookhdr)) return -EIO;
    bytes += r;

    return bytes;
}

static
long skip_data(struct vmadump_map_ctx *ctx, struct file *file, long len) {
    long r = 0;
    void *page;
    page = (void *) __get_free_page(GFP_KERNEL);
    if (!page)
	return -ENOMEM;

    while (len > 0) {
	r = read_kern(ctx, file, page, (len>PAGE_SIZE) ? PAGE_SIZE : len);
	if (r <= 0) break;
	len -= r;
    }
    free_page((long) page);

    if (r == 0) r = -EIO;	/* end of file.... */
    return 0;
}

static
int do_thaw_hooks(struct vmadump_map_ctx *ctx, struct file *file,
		  struct pt_regs *regs) {
    long r;
    struct hook_t *h;
    struct list_head *l;
    struct vmadump_hook_header hookhdr;
    struct vmadump_hook_handle hh = { 0, ctx, file};

    r = read_kern(ctx, file, &hookhdr, sizeof(hookhdr));
    if (r != sizeof(hookhdr)) goto err;
    while (hookhdr.tag[0]) {
	/* Do a bit of sanity checking on this dump header */
	hookhdr.tag[VMAD_HOOK_TAG_LEN-1] = 0; /* null terminate that string... */
	if (hookhdr.size <= 0) {
	    r = -EINVAL;
	    goto err;
	}

	/* See if we find a matching hook */
	down_read(&hook_lock);
	for (l = hooks.next; l != &hooks; l = l->next) {
	    h = list_entry(l, struct hook_t, list);
	    if (strcmp(hookhdr.tag, h->hook->tag) == 0) {
		r = h->hook->thaw(&hh, &hookhdr, regs);
		break;
	    }
	}
	if (l == &hooks)
	    r = skip_data(ctx, file, hookhdr.size);
	up_read(&hook_lock);
	if (r) goto err;

	r = read_kern(ctx, file, &hookhdr, sizeof(hookhdr));
	if (r != sizeof(hookhdr)) goto err;
    }
    return 0;

 err:
    if (r >= 0) r = -EIO;
    return r;
}

/* read/write calls for use by hooks */
ssize_t vmadump_read_u(struct vmadump_hook_handle *h, void *buf, size_t count){
    if (h->rw != 0) return -EINVAL;
    return read_user(h->ctx, h->file, buf, count);
}

ssize_t vmadump_read_k(struct vmadump_hook_handle *h, void *buf, size_t count){
    ssize_t err;
    mm_segment_t oldfs;
    oldfs = get_fs(); set_fs(KERNEL_DS);
    err = vmadump_read_u(h, buf, count);
    set_fs(oldfs);
    return err;

}

ssize_t vmadump_write_u(struct vmadump_hook_handle *h,
			const void *buf, size_t count) {
    if (h->rw != 1) return -EINVAL;
    return write_user(h->ctx, h->file, buf, count);
}

ssize_t vmadump_write_k(struct vmadump_hook_handle *h,
			const void *buf, size_t count) {
    ssize_t err;
    mm_segment_t oldfs;
    oldfs = get_fs(); set_fs(KERNEL_DS);
    err = vmadump_write_u(h, buf, count);
    set_fs(oldfs);
    return err;
}
#endif

/*
 * directio_start
 *
 * The caller MUST ensure that the filp is aligned properly (usually to 
 * 512 bytes) before calling this routine.  I've aligned everything to page 
 * size.
 */
static
unsigned int
directio_start(struct file *filp)
{
    unsigned int saved_flags = filp->f_flags;

    /*
     * This was basically ripped out of setfl() in 2.6.22, with some #if's
     * added based on 2.6.1->2 and 2.6.7->8 changes to that function.
     *
     * The alternative is to call sys_fcntl() directly.  This is a good idea.
     * But right now I'm not in the mood to walk the fd array to identify the
     * proper file descriptor.
     *
     * XXX:  Do that.
     */

  #if HAVE_FILE_F_MAPPING
    /* Order matters, if kernel has both f_mapping and i_mapping use f_mapping */
    if (!filp->f_mapping || !filp->f_mapping->a_ops ||
        !filp->f_mapping->a_ops->direct_IO)
        goto out;
  #elif HAVE_INODE_I_MAPPING
    struct inode *inode = filp->f_dentry->d_inode;
    if (!inode->i_mapping || !inode->i_mapping->a_ops ||
        !inode->i_mapping->a_ops->direct_IO)
        goto out;
  #else
    #error
  #endif

  #if HAVE_FILE_OPERATIONS_CHECK_FLAGS
    if (filp->f_op && filp->f_op->check_flags &&
        filp->f_op->check_flags(saved_flags | O_DIRECT))
        goto out;
  #else
    /* OK */
  #endif

    /* The lock_kernel() does appear necessary here because both
     * setfl() and ioctl() use it when modifying f_flags.  So, to
     * prevent corruption of f_flags, we take the lock too.
     * Of course if somebody is changing f_flags on the context
     * fd while we are actively writing... not good.
     */
    lock_kernel();
    filp->f_flags |= O_DIRECT;
    unlock_kernel();

out:
    return saved_flags;
}

static
void directio_stop(struct file *filp, unsigned int saved_flags)
{
    lock_kernel();
    filp->f_flags = saved_flags;
    unlock_kernel();
}

/* sys_prctl "by value" */
int vmadump_prctl_v(int option, unsigned long arg2) {
    return sys_prctl(option, arg2, 0, 0, 0);
}

/* sys_prctl "by reference" */
int vmadump_prctl_k(int option, void *arg2) {
    int err;
    mm_segment_t oldfs;
    oldfs = get_fs(); set_fs(KERNEL_DS);
    err = sys_prctl(option, (unsigned long)arg2, 0, 0, 0);
    set_fs(oldfs);
    return err;
}

int vmadump_sigaltstack_k(const stack_t *ss, stack_t *oss) {
    int err;
    mm_segment_t oldfs;
    oldfs = get_fs(); set_fs(KERNEL_DS);
    err = do_sigaltstack(ss, oss, 0);
    set_fs(oldfs);
    return err;
}

/*--------------------------------------------------------------------
 *  Process "thawing" routines.
 *------------------------------------------------------------------*/
static
int mmap_file(cr_rstrt_proc_req_t *ctx,
	      const struct vmadump_vma_header *head, char *filename,
	      unsigned long flags) {
    struct file *file;
    long mapaddr;
    int open_flags;
    unsigned long prot;

    if (head->flags & VM_MAYSHARE) {
	if (head->flags & VM_MAYWRITE) {
	    open_flags = (head->flags & (VM_MAYREAD|VM_MAYEXEC)) ? O_RDWR : O_WRONLY;
	    prot = PROT_WRITE;
	} else {
	    open_flags = O_RDONLY;
	    prot = 0;
	}
	if (head->flags & VM_MAYREAD)  prot |= PROT_READ;
	if (head->flags & VM_MAYEXEC)  prot |= PROT_EXEC;
    } else {
	open_flags = O_RDONLY;
	prot = PROT_READ|PROT_WRITE|PROT_EXEC;
    }

    /* This is a lot like open w/o a file descriptor */
#if 0 /* Not supported in BLCR */
    if (ctx && ctx->map_open)
	file = ctx->map_open(ctx, filename, open_flags);
    else
#endif
	file = default_map_open(ctx, filename, open_flags);
    if (IS_ERR(file)) {
	CR_ERR_CTX(ctx, "open('%s', 0x%x) failed: %d", filename, open_flags, (int)PTR_ERR(file));
	return PTR_ERR(file);
    }

    down_write(&current->mm->mmap_sem);
    mapaddr = do_mmap(file, head->start, head->end - head->start,
		     prot, flags, head->offset);
    up_write(&current->mm->mmap_sem);
    fput(file);
    if (mapaddr != head->start)
	CR_ERR_CTX(ctx, "do_mmap(<file>, %p, %p, ...) failed: %p",
	       (void *) head->start, (void *) (head->end-head->start),
	       (void *) mapaddr);
    return (mapaddr == head->start) ? 0 : mapaddr;
}

/* Reads in the header giving the the number of bytes of "fill" to
 * achieve alignment, and returns that value in *buf_len.
 * ONLY if "fill" is less than VMAD_CHUNKHEADER_MIN bytes is the
 * corresponding padding read here.
 */
static long
load_page_list_header(cr_rstrt_proc_req_t * ctx, struct file *file,
                      void *buf, unsigned int *buf_len)
{
    struct vmadump_page_list_header header;
    long bytes = 0;
    long r;

    /* load in the header so we know how many bytes of alignment there are */
    r = read_kern(ctx, file, &header, sizeof(header));
    if (r != sizeof(header)) goto err;
    bytes += r;

    /* now read in the padding if too small to fit page headers */
    if (!header.fill) {
        /* No padding at all */
    } else if (header.fill < VMAD_CHUNKHEADER_MIN) {
        /* TODO: use cr_skip(), which may someday seek */
        r = read_kern(ctx, file, buf, header.fill);
        if (r != header.fill) goto err;
        bytes += r;
    } else {
        /* Padding is large enough to use for the page headers */
        *buf_len = header.fill;
    }

    return bytes;

err:
    if (r >= 0) r = -EIO;	/* map short reads to EIO */
    return r;
}

/*
 * Returns 0 or 1 on success.
 * 0 = EOF, meaning no more pages are available
 * 1 = Caller must call again to load more pages
 * <0 Some error
 */
long load_page_chunks(cr_rstrt_proc_req_t * ctx, struct file *file,
                      struct vmadump_page_header *headers, int sizeof_headers,
                      int is_exec)
{
    unsigned long old_filp_flags;
    long r = 1;
    const int max_chunks = sizeof_headers/sizeof(*headers);
    int i;

    /* Now load the chunk page */
    r = read_kern(ctx, file, headers, sizeof_headers);
    if (r != sizeof_headers) { goto bad_read; }

    /* spin up direct IO */
    old_filp_flags = directio_start(file);

    /* load each chunk */
    for (i = 0; i < max_chunks; ++i) {
        const long len = headers[i].num_pages << PAGE_SHIFT;
        const unsigned long page_start = headers[i].start;

        if (page_start == VMAD_END_OF_CHUNKS) {
            /* EOF */
            r = 0;
            break;
        }

        r = read_user(ctx, file, (void *) page_start, len);
        if (r != len) { goto bad_read; }
        if (is_exec) { flush_icache_range(page_start, page_start + len); }
        r = 1;
    }

    /* disable direct IO */
    directio_stop(file, old_filp_flags);

    return r;

bad_read:
    if (r >= 0) { r = -EIO; } /* map short reads to EIO */
    return r;
}

int vmadump_load_page_list(cr_rstrt_proc_req_t *ctx,
			   struct file *file, int is_exec)
{
    struct vmadump_page_header *chunks;
    long r;
    unsigned int sizeof_chunks = VMAD_CHUNKHEADER_SIZE;

    chunks = (struct vmadump_page_header *) kmalloc(sizeof_chunks, GFP_KERNEL);
    if (chunks == NULL) {
        r = -ENOMEM;
        goto err;
    }

    /* handle alignment padding - either skip it or use it for first batch of chunks */
    r = load_page_list_header(ctx, file, chunks, &sizeof_chunks);
    if (r < 0) { goto out_free; }

    /* now load all the page chunks */
    do {
        r = load_page_chunks(ctx, file, chunks, sizeof_chunks, is_exec);
        if (r < 0) { goto out_free; }
        sizeof_chunks = VMAD_CHUNKHEADER_SIZE; /* After the first, all chunk arrays are this size */
    } while (r > 0);

out_free:
    kfree(chunks);
err:
    return r;
}

static
int load_map(cr_rstrt_proc_req_t *ctx,
	     struct file *file, struct vmadump_vma_header *head) {
    long r;
    unsigned long mmap_prot, mmap_flags, addr;

    if (head->namelen == VMAD_NAMELEN_ARCH) {
#if VMAD_HAVE_ARCH_MAPS
	return vmad_load_arch_map(ctx, file, head);
#else
        CR_ERR_CTX(ctx, "Found an arch-specific mapping in a kernel without any???");
        return -EINVAL;
#endif
    }

    mmap_prot  = 0;
    mmap_flags = MAP_FIXED | ((head->flags & VM_MAYSHARE) ? MAP_SHARED : MAP_PRIVATE);
    if (head->flags & VM_READ)  mmap_prot |= PROT_READ;
    if (head->flags & VM_WRITE) mmap_prot |= PROT_WRITE;
    if (head->flags & VM_EXEC)  mmap_prot |= PROT_EXEC;
    if (head->flags & VM_GROWSDOWN) mmap_flags |= MAP_GROWSDOWN;
    if (head->flags & VM_EXECUTABLE) mmap_flags |= MAP_EXECUTABLE;
    if (head->flags & VM_DENYWRITE) mmap_flags |= MAP_DENYWRITE;

    if (head->namelen > 0) {
	char *filename;
	if (head->namelen > PAGE_SIZE) {
	    CR_ERR_CTX(ctx, "thaw: bogus namelen %d", (int) head->namelen);
	    return -EINVAL;
	}
	filename = kmalloc(head->namelen+1,GFP_KERNEL);
	if (!filename) {
	    r = -ENOMEM;
	    goto err;
	}
	r = read_kern(ctx, file, filename, head->namelen);
	if (r != head->namelen) {
	    kfree(filename);
	    goto err;
	}
	filename[head->namelen] = 0;  
	r = mmap_file(ctx, head, filename, mmap_flags);
	if (r) {
	    CR_ERR_CTX(ctx, "mmap failed: %s", filename);
	    kfree(filename);
	    return r;
	}
	kfree(filename);
    } else {
	/* Load the data from the dump file */
	down_write(&current->mm->mmap_sem);
	addr = do_mmap(0, head->start, head->end - head->start,
		       mmap_prot|PROT_WRITE, mmap_flags, 0);
	up_write(&current->mm->mmap_sem);
	if (addr != head->start) {
	    CR_ERR_CTX(ctx, "do_mmap(0, %08lx, %08lx, ...) = 0x%08lx (failed)",
		   head->start, head->end - head->start, addr);
	    return -EINVAL;
	}
    }

    /* Read in patched pages */
    r = vmadump_load_page_list(ctx, file, (mmap_prot & PROT_EXEC));
    if (r) goto err;

    if (sys_mprotect(head->start,head->end - head->start, mmap_prot))
	CR_ERR_CTX(ctx, "thaw: mprotect failed. (ignoring)");
    return 0;

 err:
    if (r >= 0) r = -EIO;	/* map short reads to EIO */
    return r;
}

static
int vmadump_load_sigpending(cr_rstrt_proc_req_t *ctx, struct file *file,
				struct sigpending *sigpending, int shared) {
    sigset_t pending_set;
    int flags;
    long r;

    r = read_kern(ctx, file, &flags, sizeof(flags));
    if (r != sizeof(flags)) goto bad_read;

    while (flags != -1) {
	struct siginfo info;

	r = read_kern(ctx, file, &info, sizeof(info));
	if (r != sizeof(info)) goto bad_read;

	/* XXX: if/when we do support posix timers directly, we might
	 * need to deal with SIGQUEUE_PREALLOC in flags.
	 * For now there is  no harm in ignoring it.
	 */
	if (shared) {
	    read_lock(&tasklist_lock);
	    r = group_send_sig_info(info.si_signo, &info, current);
	    read_unlock(&tasklist_lock);
	} else {
	    r = send_sig_info(info.si_signo, &info, current);
	}

	r = read_kern(ctx, file, &flags, sizeof(flags));
	if (r != sizeof(flags)) goto bad_read;
    }

    /* Corresponding signal set comes last */
    r = read_kern(ctx, file, &pending_set, sizeof(pending_set));
    if (r != sizeof(pending_set)) goto bad_read;
    spin_lock_irq(&current->sighand->siglock);
    sigorsets(&sigpending->signal, &sigpending->signal, &pending_set);
    recalc_sigpending();
    spin_unlock_irq(&current->sighand->siglock);

    return 0;

 bad_read:
    if (r >= 0) r = -EIO;	/* Map short writes to EIO */
    return r;
}

/* Values of (struct vmad_prctl).type */
enum vmad_prctl_type {
   vmad_prctl_int_ref,  /* integer read by reference, written by value*/
   vmad_prctl_int_val,  /* integer read and written by value */
   vmad_prctl_comm,     /* string of length TASK_COMM_LEN, set by reference */
};

struct vmad_prctl {
    int option;  /* First argument to prctl() to set */
    int type;    /* How to handle 2nd argument */
};

static
int vmadump_load_prctl(cr_rstrt_proc_req_t *ctx, struct file *file) {
    struct vmad_prctl header;
    long r;

    r = read_kern(ctx, file, &header, sizeof(header));
    if (r != sizeof(header)) goto bad_read;
    while (header.option) { /* option==0 is end of the list */
        switch (header.type) {
        case vmad_prctl_int_val:
        case vmad_prctl_int_ref: {
            unsigned int value;

            r = read_kern(ctx, file, &value, sizeof(value));
            if (r != sizeof(value)) goto bad_read;
            r = vmadump_prctl_v(header.option, value);
            break;
            }

        case vmad_prctl_comm: {
#if defined(PR_SET_NAME)
            char value[TASK_COMM_LEN];

            r = read_kern(ctx, file, &value, sizeof(value));
            if (r != sizeof(value)) goto bad_read;
            r = vmadump_prctl_k(header.option, &value);
#else
            if (sizeof(current->comm) != TASK_COMM_LEN) {
                CR_ERR_CTX(ctx, "vmadump: TASK_COMM_LEN changed?");
                r = -EINVAL;
                goto bad_read;
            }
            r = read_kern(ctx, file, current->comm, TASK_COMM_LEN);
            if (r != TASK_COMM_LEN) goto bad_read;
#endif
            break;
            }

        default:
            CR_ERR_CTX(ctx, "vmadump: prtcl %d has unknown type code %d",
                       header.option, header.type);
            goto bad_read;
        }

#if 0 /* Confusing when issued, for instance, for PR_SET_SECCOMP where permission is denied */
        if (r < 0) {
            CR_WARN_CTX(ctx, "vmadump warning: failed to set prtcl(%d) err %ld", header.option, r);
        }
#endif

        r = read_kern(ctx, file, &header, sizeof(header));
        if (r != sizeof(header)) goto bad_read;
    }

 bad_read:
    if (r >= 0) r = -EIO;	/* Map short writes to EIO */
    return r;
}

long vmadump_thaw_proc(cr_rstrt_proc_req_t *ctx,
		       struct file *file, struct pt_regs *regs,
		       int flags) {
    long r;
    pid_t pid;
#if BITS_PER_LONG == 64
    unsigned long orig_personality = personality(current->personality);
#endif

    {
    struct vmadump_header header;

    /*--- First some sanity checking ---*/
    r = read_kern(ctx, file, &header, sizeof(header));
    if (r != sizeof(header)) {
	CR_ERR_CTX(ctx, "vmadump: failed to read header: %ld", r);
	return -EINVAL;
    }
    if (memcmp(header.magic, vmad_magic, sizeof(header.magic))) {
	CR_ERR_CTX(ctx, "vmadump: invalid signature");
	return -EINVAL;
    }
    if (header.fmt_vers != VMAD_FMT_VERS) {
	CR_ERR_CTX(ctx, "vmadump: dump version mistmatch. dump=%d; "
	       "kernel=%d", (int)header.fmt_vers, (int)VMAD_FMT_VERS);
	return -EINVAL;
    }
    if (header.arch != VMAD_ARCH) {
	CR_ERR_CTX(ctx, "vmadump: architecture mismatch.");
	return -EINVAL;
    }

    if (header.major != ((LINUX_VERSION_CODE >> 16) & 0xFF) ||
	header.minor != ((LINUX_VERSION_CODE >> 8) & 0xFF)
#if defined(STRICT_VERSION_CHECK) && STRICT_VERSION_CHECK
	||header.patch != (LINUX_VERSION_CODE & 0xFF)
#endif
	) {
	CR_ERR_CTX(ctx, "vmadump: kernel version mismatch.");
	return -EINVAL;
    }
    }

    /* Ummm... Point of no-return is here.... maybe try to move this
     * down a bit? */

#if 0 /* Now handled w/ other prctl values */
    if (!(flags & VMAD_DUMP_REGSONLY)) {
        /* Read our new comm */
        r = read_kern(ctx, file, current->comm, sizeof(current->comm));
        if (r != sizeof(current->comm)) goto bad_read;
    }
#endif
    vmadump_load_prctl(ctx, file);

    /* PID, provided only for the benefit of our caller */
    {
	r = read_kern(ctx, file, &pid, sizeof(pid));
	if (r != sizeof(pid)) goto bad_read;
    }

    /* Credentials, important to do before we try to open any files */
    {
	r = cr_load_creds(ctx);
	if (r < 0) goto bad_read;
    }

    /*
     * CPU-specific register restore stuff
     *
     * Note that we're not presuming that our current regs pointer
     * points to anything even vaguely reasonable.  This is done to
     * support bproc type kernel threads that have never been user
     * processes before.
     */
    r = vmadump_restore_cpu(ctx, file, regs);
    if (r) goto bad_read;

    /*--- Signal information ---------------------------------------*/
    {
    int i;
    stack_t              sig_stack;
    sigset_t             sig_blocked;
    struct k_sigaction   sig_action;

    /* Restore sigaltstack */
    r = read_kern(ctx, file, &sig_stack, sizeof(sig_stack));
    if (r != sizeof(sig_stack)) goto bad_read;
    r = vmadump_sigaltstack_k(&sig_stack, NULL);
    if (r < 0) goto bad_read;

    /* Install sets of blocked and pending signals */
    r = read_kern(ctx, file, &sig_blocked, sizeof(sig_blocked));
    if (r != sizeof(sig_blocked)) goto bad_read;

    sigdelsetmask(&sig_blocked, sigmask(SIGKILL)|sigmask(SIGSTOP));
    spin_lock_irq(&current->sighand->siglock);
    memcpy(&current->blocked, &sig_blocked, sizeof(sig_blocked));
    recalc_sigpending();
    spin_unlock_irq(&current->sighand->siglock);

    r = vmadump_load_sigpending(ctx, file, &current->pending, 0);
    if (r < 0) goto bad_read;
    if ((atomic_read(&current->signal->count) == 1) || !(flags & VMAD_DUMP_REGSONLY)) {
	/* Restore shared queue if not actually shared, or if we are the "leader" */
	r = vmadump_load_sigpending(ctx, file, &current->signal->shared_pending, 1);
	if (r < 0) goto bad_read;
    }

    if (!(flags & VMAD_DUMP_REGSONLY)) {
        for (i=0; i < _NSIG; i++) {
	    r = read_kern(ctx, file, &sig_action, sizeof(sig_action));
	    if (r != sizeof(sig_action)) goto bad_read;

	    if (i != SIGKILL-1 && i != SIGSTOP-1) {
	        r = do_sigaction(i+1, &sig_action, 0);
	        if (r) goto bad_read;
	    }
        }
    }
    }

    /*--- Misc other stuff -----------------------------------------*/
    {				/* our tid ptr */
    r = read_kern(ctx, file, &current->clear_child_tid,
		  sizeof(current->clear_child_tid));
    if (r != sizeof(current->clear_child_tid)) { goto bad_read; }
    }

    {				/* personality */
    unsigned long tmp_personality;
    r = read_kern(ctx, file, &tmp_personality, sizeof(tmp_personality));
    if (r != sizeof(tmp_personality)) { goto bad_read; }
    set_personality(tmp_personality);
    }

    /*--- Memory map meta data -------------------------------------*/
    if (!(flags & VMAD_DUMP_REGSONLY)) {
    struct mm_struct *mm;
    struct vm_area_struct *map;
    struct vmadump_mm_info mm_info;
    struct vmadump_vma_header mapheader;
#if HAVE_MM_MMAP_BASE
    unsigned long mmap_base;
#endif
    int map_count;

#if BITS_PER_LONG == 64
    /* If restarting a 32-bit application from a 64-bit cr_restart, we need
     * to reset TIF_32BIT when unmapping the pages, otherwise do_munmap
     * enters an infinite loop!
     */
    #if !defined(TIF_32BIT) && defined(TIF_IA32)
      #define TIF_32BIT TIF_IA32
    #endif
    int fiddle_tif_32bit = test_thread_flag(TIF_32BIT) && (orig_personality != PER_LINUX32);
#endif
    
    mm = current->mm;

    r = read_kern(ctx, file, &mm_info, sizeof(mm_info));
    if (r != sizeof(mm_info)) { goto bad_read; }

#if HAVE_MM_MMAP_BASE
    r = read_kern(ctx, file, &mmap_base, sizeof(mmap_base));
    if (r != sizeof(mmap_base)) { goto bad_read; }
#endif

#if BITS_PER_LONG == 64
    if (fiddle_tif_32bit) {
	clear_thread_flag(TIF_32BIT);
    }
#endif

    /* Purge current maps - I'm sure there's a way to keep theses around
     * incase creation of the new ones fails in some unfortunate way... */
    while(mm->mmap) {
	map = mm->mmap;
	map_count = mm->map_count;
	r = do_munmap(mm, map->vm_start, map->vm_end - map->vm_start);
	if (r) {
	    CR_ERR_CTX(ctx, "do_munmap(%lu, %lu) = %d",
		   map->vm_start, map->vm_end-map->vm_start, (int)r);
	}
	if (map_count == mm->map_count) {
	    CR_ERR_CTX(ctx, "do_munmap() loop stuck.");
	    r = -EINVAL;
	    goto bad_read;
	}
    }

#if BITS_PER_LONG == 64
    if (fiddle_tif_32bit) {
        set_thread_flag(TIF_32BIT);
    }
#endif

    /* (re)set mm paramaters which influence get_unmapped_area() */
    down_write(&current->mm->mmap_sem);
#if HAVE_MM_TASK_SIZE
    mm->task_size  = TASK_SIZE;
#endif
#if defined(CR_KCODE_arch_pick_mmap_layout)
    arch_pick_mmap_layout(mm);
#endif
#if HAVE_MM_MMAP_BASE
    /* want to restore these even if arch_pick_mmap_layout() set them */
    mm->mmap_base = mmap_base;
    mm->free_area_cache = mmap_base;
#else
    mm->free_area_cache = TASK_UNMAPPED_BASE;
#endif
#if HAVE_MM_CACHED_HOLE_SIZE
    mm->cached_hole_size = ~0UL;
#endif
    up_write(&current->mm->mmap_sem);

    /* Load new map data */
    r = read_kern(ctx, file, &mapheader, sizeof(mapheader));
    while (r == sizeof(mapheader) &&
	   (mapheader.start != ~0 || mapheader.end != ~0)) {
	if ((r = load_map(ctx, file, &mapheader))) goto bad_read;
	r = read_kern(ctx, file, &mapheader, sizeof(mapheader));
    }
    if (r != sizeof(mapheader)) goto bad_read;

    down_write(&current->mm->mmap_sem);
    mm->start_code = mm_info.start_code;
    mm->end_code   = mm_info.end_code;
    mm->start_data = mm_info.start_data;
    mm->end_data   = mm_info.end_data;
    mm->start_brk  = mm_info.start_brk;
    mm->brk        = mm_info.brk;
    mm->start_stack= mm_info.start_stack;
    /* FIX ME: validate these pointers */
    mm->arg_start  = mm_info.arg_start;
    mm->arg_end    = mm_info.arg_end;
    mm->env_start  = mm_info.env_start;
    mm->env_end    = mm_info.env_end;

    up_write(&current->mm->mmap_sem);
    }

#if 0 /* Hooks code not needed/maintained for BLCR */
    /*--- Call external thaw hooks ---------------------------------*/
    r = do_thaw_hooks(ctx, file, regs);
    if (r) goto bad_read;
#endif

    return pid;

 bad_read:
    if (r >= 0) r = -EIO;
    return r;
}

/*--------------------------------------------------------------------
 *  Process "freezing" routines
 *------------------------------------------------------------------*/

/* This routine walks the page tables.
 * If addr is a huge page, returns NULL and sets *pagep non-null
 * If addr is not in the page tables, returns NULL and sets *pagep=NULL
 * Otherwise returns non-NULL (w/ *pagep undefined) and caller must pte_unmap().
 */
static inline
pte_t *vmad_follow_addr(struct page **pagep, struct mm_struct *mm, unsigned long addr) {
    pgd_t *pgd;
#ifdef PTRS_PER_PUD
    pud_t *pud;
#endif
    pmd_t *pmd;

#if !defined(CONFIG_HUGETLBFS)
    /* Nothing to do here */
#elif HAVE_4_ARG_FOLLOW_HUGE_ADDR
    struct vm_area_struct *vma = hugepage_vma(mm, addr);
    if (vma) {
	*pagep = follow_huge_addr(mm, vma, addr, 0);
	return NULL;
    }
#elif HAVE_3_ARG_FOLLOW_HUGE_ADDR
    struct page *pg = follow_huge_addr(mm, addr, 0);
    if (!IS_ERR(pg)) {
	*pagep = pg;
	return NULL;
    }
#else
    #error "No way to call follow_huge_addr()"
#endif
    *pagep = NULL;
    pgd = pgd_offset(mm, addr);
    if (pgd_none(*pgd)) return NULL;
#ifdef PTRS_PER_PUD
    pud = pud_offset(pgd, addr);
    if (pud_none(*pud)) return NULL;
    pmd = pmd_offset(pud, addr);
#else
    pmd = pmd_offset(pgd, addr);
#endif
    if (pmd_none(*pmd)) return NULL;
#ifdef CONFIG_HUGETLBFS
    if (pmd_huge(*pmd)) {
	*pagep = follow_huge_pmd(mm, addr, pmd, 0);
	return NULL;
    }
#endif
    return pte_offset_map(pmd, addr);
}

/* This routine checks if a page from a filemap has been copied via
 * copy on write.  Basically, this is just checking to see if the page
 * is still a member of the map or not.  Note this this should not end
 * up copying random things from VM_IO regions. */
static
int addr_copied(struct mm_struct *mm, unsigned long addr) {
    pte_t *ptep;
    struct page *pg;
    int ret;
#if !HAVE_PAGEANON
    #define PageAnon(pg) (!(pg)->mapping)
#endif

    spin_lock(&mm->page_table_lock);
    ptep = vmad_follow_addr(&pg, mm, addr);
    if (ptep) {
	pte_t pte = *ptep;
	pte_unmap(ptep);
	if (pte_present(pte)) {
	    pg = pte_page(pte);
	    ret = PageAnon(pg);
	} else {
	    /* pte_none is false for a swapped (written) page */
	    ret = !pte_none(pte);
	}
    } else {
	ret = pg && PageAnon(pg);
    }
    spin_unlock(&mm->page_table_lock);
    return ret;
}

/* This is the version for working on a region that is a file map.  In
 * this case we need to fault the page in to check for zero.  This
 * isn't a big deal since we'll be faulting in for sending anyway if
 * it's not.  */
static
int addr_nonzero_file(struct mm_struct *mm, unsigned long addr) {
    int i;
    unsigned long val = 0;

    /* Simple zero check */
    for (i=0; i < (PAGE_SIZE/sizeof(long)); i++) {
	/* We ignore EFAULT and presume that it's zero here */
	get_user(val, (((long*)addr)+i));
	if (val) return 1;
    }
    return 0;
}

/* This version is for use on regions which are *NOT* file maps.  Here
 * we look at the page tables to see if a page is zero.  If it's never
 * been faulted in, we know it's zero - and we don't fault it in while
 * checking for this. */
static
int addr_nonzero(struct mm_struct *mm, unsigned long addr) {
    int i;
    struct page *pg;
    pte_t *ptep;
    unsigned long val;

    spin_lock(&mm->page_table_lock);
    ptep = vmad_follow_addr(&pg, mm, addr);
    if (ptep) {
	pte_t pte = *ptep;
	pte_unmap(ptep);
	if (pte_none(pte)) goto out_zero; /* Never faulted */
	if (pte_present(pte) && (pte_page(pte) == ZERO_PAGE(addr))) goto out_zero; /* Only READ faulted */
    } else if (!pg) {
	goto out_zero;
    }
    spin_unlock(&mm->page_table_lock);

    /* Ok, the page could be non-zero - check it... */
    for (i=0; i < (PAGE_SIZE/sizeof(long)); i++) {
	get_user(val, (((long*)addr)+i));
	if (val) return 1;
    }
    return 0;

 out_zero:
    spin_unlock(&mm->page_table_lock);
    return 0;
}

/* Writes out the header giving the reader the number of bytes of
 * "fill", and returns that value in *buf_len.
 * ONLY if "fill" is smaller than VMAD_CHUNKHEADER_MIN bytes is
 * the corresponding padding written here.
 */
static long 
store_page_list_header(cr_chkpt_proc_req_t *ctx, struct file *file, 
                       void *buf, unsigned int *buf_len)
{
    struct vmadump_page_list_header header;
    long bytes = 0;
    long r;
    unsigned int fill;

    fill = PAGE_SIZE - ((file->f_pos + sizeof(header)) & (PAGE_SIZE - 1));

    header.fill = fill;
    r = write_kern(ctx, file, &header, sizeof(header));
    if (r != sizeof(header))
        goto bad_write;
    bytes += r;

    if (!fill) {
        /* no padding at all */
    } else if (fill < VMAD_CHUNKHEADER_MIN) {
        /* TODO: seek when possible? */
        r = write_kern(ctx, file, buf, fill);
        if (r != fill)
            goto bad_write;
        bytes += r;
    } else {
        /* Padding is large enough to use for the page headers */
        *buf_len = fill;
    }

    return bytes;

bad_write:
    if (r >= 0)
        r = -EIO;               /* Map short writes to EIO */
    return r;
}

/* 
 * Returns < 0 on failure, or written byte count on success.
 */
static 
long store_page_chunks(cr_chkpt_proc_req_t *ctx, struct file *file,
		      struct vmadump_page_header *headers,
		      int sizeof_headers)
{
    unsigned long old_filp_flags;
    unsigned long chunk_start;
    long r, bytes = 0;
    int i;

    const int num_headers = sizeof_headers/sizeof(*headers);

    r = write_kern(ctx, file, headers, sizeof_headers);
    if (r != sizeof_headers) goto bad_write;
    bytes += r;

    /* Avoid directio_start/stop if there is no page I/O */
    if (headers[0].start == VMAD_END_OF_CHUNKS) goto empty;

    /*
     * This routine attempts to set up direct IO for the chunk writes.
     * If direct IO is not available, it does nothing.
     */
    old_filp_flags = directio_start(file);

    for (i=0; i<num_headers; ++i) {
	const long len = headers[i].num_pages << PAGE_SHIFT;

	chunk_start = headers[i].start;

        if (chunk_start == VMAD_END_OF_CHUNKS) {
            /* EOF */
            break;
        }

	r = write_user(ctx, file, (void *)chunk_start, len);
	if (r != len) goto bad_write;
	bytes += r;
    }

    directio_stop(file, old_filp_flags);

empty:
    return bytes;

bad_write:
    if (r >= 0) r = -EIO;	/* Map short writes to EIO */
    return r;
}

/* Add an element to the chunks array, and if its full or "finished" then
 * write the chunks array.  Elements with zero pages are omitted.
 * "Finished" is defined as having start == VMAD_END_OF_CHUNKS in this chunk.
 *
 * Returns the number of bytes written (could be zero), or <0 on error.
 * If data was written, then sizeof_chunks and chunk_number are reset.
 * 
 * chunks - the chunks array to write
 * *sizeof_chunks - length of chunks in BYTES
 * *chunk_number - next unoccupied entry in array
 */
static inline loff_t
write_chunk(cr_chkpt_proc_req_t *ctx, struct file *file,
             struct vmadump_page_header *chunks, unsigned int *sizeof_chunks,
             int *chunk_number, unsigned long start, unsigned long num_pages)
{
    long r = 0;

    /* Add an element to the array unless is would be empty*/
    if (num_pages || (start == VMAD_END_OF_CHUNKS)) {
        const int max_chunks = *sizeof_chunks/sizeof(*chunks);
        int index;

        index = (*chunk_number)++;
        chunks[index].start = start;
        chunks[index].num_pages = num_pages;

        /* Write the array if full or finished */
        if (((index + 1) >= max_chunks) || (start == VMAD_END_OF_CHUNKS)) {
            r = store_page_chunks(ctx, file, chunks, *sizeof_chunks);
            *sizeof_chunks = VMAD_CHUNKHEADER_SIZE;
            *chunk_number = 0;
        }
    }
    return r;
}

/*
 * SOMEDAY: we may want a field in vmadump_page_header for page hashes
 *
 * SOMEDAY: add a buffer argument through some of these routines
 * so that we can put compression in there.
 */
static loff_t
store_page_list(cr_chkpt_proc_req_t * ctx, struct file *file,
		unsigned long start, unsigned long end,
		int (*need_to_save) (struct mm_struct * mm, unsigned long))
{
    long r;
    loff_t bytes = 0;
    unsigned long addr;
    unsigned long chunk_start, chunk_end, num_contig_pages;
    struct vmadump_page_header *chunks;
    int chunk_number;
    unsigned int sizeof_chunks = VMAD_CHUNKHEADER_SIZE;

    /* A page 'chunk' is a contiguous range of pages in virtual memory.
     * 
     * We identify the chunks, and store them in the chunks array.
     * Our calls to write_chunk() will write all of the chunks out when needed,
     * and will start filling the chunks array back from the beginning.
     */
    chunks =
        (struct vmadump_page_header *) cr_kzalloc(sizeof_chunks, GFP_KERNEL);
    if (chunks == NULL) {
        r = -ENOMEM;
    }

    /*
     * TODO:  Move this alignment step outward, perhaps as far as freeze_proc?
     */

    /* Handle alignment of the current file pointer to a page boundary for direct IO.
     * The "fill" required to acheive alignment is used for the first array of chunks
     * if large enough.  Otherwise it is written as zeros for padding.
     */
    r = store_page_list_header(ctx, file, chunks, &sizeof_chunks);
    if (r < 0) {
        goto out_kfree;
    }
    bytes += r;

    /*
     * Ok, this loop does all the work.  We're going to make a pass through.
     */
    chunk_start = chunk_end = start;
    num_contig_pages = 0;
    chunk_number = 0;
    for (addr = start; addr < end; addr += PAGE_SIZE) {
        /* The topmost if clause (need_to_save) is to identify things like 
         * unmodified pages that can be reread from disk, or pages that were 
         * allocated and never touched (zero pages).  */
        if (need_to_save(current->mm, addr)) {
            /* test for contiguous pages.  (chunk_end == addr)
             *
             * break up a contiguous page range if too large, (num < ...)
             */
            if (chunk_end == addr) {
                num_contig_pages++;
            } else {
                r = write_chunk(ctx, file, chunks,
                                &sizeof_chunks, &chunk_number,
                                chunk_start, num_contig_pages);
                if (r < 0) goto out_io;
                bytes += r;

                /* Start a new chunk */
                chunk_start = addr;
                num_contig_pages = 1;
            }

            /* this is part of the current chunk */
            chunk_end = addr + PAGE_SIZE;
        }
    }

    /* store the last chunk */
    r = write_chunk(ctx, file, chunks,
                    &sizeof_chunks, &chunk_number,
                    chunk_start, num_contig_pages);
    if (r < 0) goto out_io;
    bytes += r;

    /* now store a record marking the end of the chunks, 
     * which should force writting of the chunks array */
    r = write_chunk(ctx, file, chunks,
                    &sizeof_chunks, &chunk_number,
                    VMAD_END_OF_CHUNKS, 0);
    if (r < 0) goto out_io;
    if (r == 0) {
        /* This absolutely should not happen.  At the very least an EOF
         * record should have been written. */
        printk("write_chunks unexpectedly returned 0!\n");
    }
    bytes += r;

out_io:
out_kfree:
    kfree(chunks);

    if (r < 0) {
        return r;
    } else {
        return bytes;
    }
}

loff_t vmadump_store_page_list(cr_chkpt_proc_req_t *ctx, struct file *file,
			       unsigned long start, unsigned long end) {
	return store_page_list(ctx, file, start, end, addr_nonzero_file);
}

loff_t vmadump_store_dirty_page_list(cr_chkpt_proc_req_t *ctx, struct file *file,
			       unsigned long start, unsigned long end) {
	return store_page_list(ctx, file, start, end, addr_copied);
}

static
loff_t store_map(cr_chkpt_proc_req_t *ctx, struct file *file,
	         struct vm_area_struct *map, int flags) {
    loff_t bytes;
    struct vmadump_vma_header head;
    char *filename=0;
    char *buffer = 0;
    loff_t r;
    unsigned long start, end;
    int isfilemap = 0;

#if VMAD_HAVE_ARCH_MAPS
    r = vmad_store_arch_map(ctx, file, map, flags);
    if (r) return r;
#endif

    head.start   = map->vm_start;
    head.end     = map->vm_end;
    head.flags   = map->vm_flags;
    head.namelen = 0;
    head.offset  = map->vm_pgoff << PAGE_SHIFT; /* XXX LFS! */

    /* Decide Whether or not we're gonna store the map's contents or
     * a reference to the file they came from */
    if (map->vm_file) {
	unsigned short vmflags = map->vm_flags;
	buffer = (char *) __get_free_page(GFP_KERNEL);
	if (!buffer) { return -ENOMEM; }
#if 0 /* Not supported in BLCR */
	if (ctx && ctx->map_name)
	    filename = ctx->map_name(ctx, map->vm_file, buffer, PAGE_SIZE);
	else
#endif
	    filename = default_map_name(map->vm_file, buffer, PAGE_SIZE);
	head.namelen = strlen(filename);

	if (vmflags & VM_IO) {
	    /* Region is an IO map. */

	    /* Never store the contents of a VM_IO region */
	} else if (vmad_is_special_mmap(map, flags)) {
		/* Let BLCR deal with it */	
		free_page((long)buffer);
		return 0;
	} else if (vmad_dentry_unlinked(map->vm_file->f_dentry)) {
	    /* Region is an unlinked file - store contents, not filename */	
	    head.namelen = 0;//jay
	} else if (vmflags & VM_EXECUTABLE) {
	    /* Region is an executable */
	    if (flags & VMAD_DUMP_EXEC)
		head.namelen = 0;
	} else if (is_library(filename)) {
	    /* Region is a library */	
	    if (flags & VMAD_DUMP_LIBS)
		head.namelen = 0;
	} else {
	    /* Region is something else */
	    if (flags & VMAD_DUMP_OTHER)
		head.namelen=0;
	}
	isfilemap = 1;
    }

    start     = map->vm_start;
    end       = map->vm_end;
    /* Release the mm_sem here to avoid deadlocks with page faults and
     * write locks that may happen during the writes.  (We can't use
     * the "map" pointer beyond this point. */
    up_read(&current->mm->mmap_sem);

    /* Spit out the section header */
    r = write_kern(ctx, file, &head, sizeof(head));
    if (r != sizeof(head)) goto err;
    bytes = r;

    if (head.namelen > 0) {
	/* Store the filename */
	r = write_kern(ctx, file, filename, head.namelen);
	if (r != head.namelen) goto err;
	bytes += r;
	r = store_page_list(ctx, file, start, end, addr_copied);

	if (r < 0) goto err;
	bytes += r;
    } else {
	/* Store the contents of the VMA as defined by start, end */
	r = store_page_list(ctx, file, start, end,
			    isfilemap ? addr_nonzero_file : addr_nonzero);
	if (r < 0) goto err;
	bytes += r;
    }
    if (buffer)   free_page((long)buffer);
    down_read(&current->mm->mmap_sem);
    return bytes;

 err:
    if (r >= 0) r = -EIO;	/* Map short writes to EIO */
    if (buffer)   free_page((long)buffer);
    down_read(&current->mm->mmap_sem);
    return r;
}

static
long vmadump_store_sigpending(cr_chkpt_proc_req_t *ctx, struct file *file,
				struct sigpending *sigpending) {
    sigset_t pending_set;
    long bytes = 0, r;
    struct list_head *elem;
    int flags;

    /* We know the list can't shrink (as all threads are stopped),
     * So, we are playing a bit loose w/ the locking.
     */
    spin_lock_irq(&current->sighand->siglock);
    list_for_each(elem, &sigpending->list) {
	struct sigqueue *q = list_entry(elem, struct sigqueue, list);
	struct siginfo tmp_info, info;
	mm_segment_t oldfs;

	flags = q->flags;
	tmp_info = q->info;
	spin_unlock_irq(&current->sighand->siglock);

	/* copy_siginfo_to_user() copies only non-padding fields */
	memset(&info, 0, sizeof(info));
	oldfs = get_fs(); set_fs(KERNEL_DS);
	r = copy_siginfo_to_user(&info, &tmp_info);
	set_fs(oldfs);

	if (r) goto err;

	r = write_kern(ctx, file, &flags, sizeof(flags));
	if (r != sizeof(flags)) goto err;
	bytes += r;
	r = write_kern(ctx, file, &info, sizeof(info));
	if (r != sizeof(info)) goto err;
	bytes += r;

	spin_lock_irq(&current->sighand->siglock);
    }
    memcpy(&pending_set, &sigpending->signal, sizeof(pending_set));
    spin_unlock_irq(&current->sighand->siglock);

    /* Flags == -1 marks end of the list */
    flags = -1;
    r = write_kern(ctx, file, &flags, sizeof(flags));
    if (r != sizeof(flags)) goto err;
    bytes += r;

    /* Corresponding signal set comes last */
    r = write_kern(ctx, file, &pending_set, sizeof(pending_set));
    if (r != sizeof(pending_set)) goto err;
    bytes += r;

    return bytes;

 err:
    if (r >= 0) r = -EIO;	/* Map short writes to EIO */
    return r;
}

static
long vmadump_store_prctl_aux(cr_chkpt_proc_req_t *ctx, struct file *file,
		             int option, int type, void *addr, int len) {
    struct vmad_prctl header;
    long r;
    long bytes = 0;

    header.option = option;
    header.type = type;
    r = write_kern(ctx, file, &header, sizeof(header));
    if (r != sizeof(header)) goto err;
    bytes += r;

    if (len) {
        r = write_kern(ctx, file, addr, len);
        if (r != len) goto err;
        bytes += r;
    }

    return bytes;

 err:
    if (r >= 0) r = -EIO;	/* Map short writes to EIO */
    return r;
}

static
int vmadump_store_prctl(cr_chkpt_proc_req_t *ctx, struct file *file) {
    long r;
    long bytes = 0;
    int i;
    const static struct {
        int type;
        int get_option;
        int set_option;
    } vmad_prctl_tbl[] = {
    #if defined(PR_GET_PDEATHSIG)
        {vmad_prctl_int_ref, PR_GET_PDEATHSIG, PR_SET_PDEATHSIG},
    #endif
    #if defined(PR_GET_DUMPABLE)
        {vmad_prctl_int_val, PR_GET_DUMPABLE, PR_SET_DUMPABLE},
    #endif
    #if defined(PR_GET_UNALIGN) && defined(GET_UNALIGN_CTL)
        {vmad_prctl_int_ref, PR_GET_UNALIGN, PR_SET_UNALIGN},
    #endif
    #if defined(PR_GET_KEEPCAPS)
        {vmad_prctl_int_val, PR_GET_KEEPCAPS, PR_SET_KEEPCAPS},
    #endif
    #if defined(PR_GET_FPEMU) && defined(GET_FPEMU_CTL)
        {vmad_prctl_int_ref, PR_GET_FPEMU, PR_SET_FPEMU},
    #endif
    #if defined(PR_GET_FPEXC) && defined(GET_FPEXC_CTL)
        {vmad_prctl_int_ref, PR_GET_FPEXC, PR_SET_FPEXC},
    #endif
    #if defined(PR_GET_TIMING)
        {vmad_prctl_int_val, PR_GET_TIMING, PR_SET_TIMING},
    #endif
    #if defined(PR_GET_ENDIAN) && defined(GET_ENDIAN)
        {vmad_prctl_int_ref, PR_GET_ENDIAN, PR_SET_ENDIAN},
    #endif
    #if defined(PR_GET_SECCOMP)
        {vmad_prctl_int_val, PR_GET_SECCOMP, PR_SET_SECCOMP},
    #endif
    #if defined(PR_GET_TSC) && defined(GET_TSC_CTL)
        {vmad_prctl_int_ref, PR_GET_TSC, PR_SET_TSC},
    #endif
    #if defined(PR_GET_TIMERSLACK)
        {vmad_prctl_int_val, PR_GET_TIMERSLACK, PR_SET_TIMERSLACK},
    #endif
    };

    /* First the comm, which was not handled via prctl prior to 2.6.9 */
#if defined(PR_GET_NAME)
    {
        char value[TASK_COMM_LEN];

        r = vmadump_prctl_k(PR_GET_NAME, &value);
        if (r < 0) goto err;
        r = vmadump_store_prctl_aux(ctx, file,
                                    PR_SET_NAME, vmad_prctl_comm,
                                    &value, sizeof(value));
        if (r < 0) goto err;
        bytes += r;
    }
#else
    /* Value of header.option is ignored, but use PR_SET_NAME to lessen confusion */
    if (sizeof(current->comm) != TASK_COMM_LEN) {
        CR_ERR_CTX(ctx, "vmadump: TASK_COMM_LEN changed?");
        r = -EINVAL;
        goto err;
    }
    r = vmadump_store_prctl_aux(ctx, file,
                                15 /* PR_SET_NAME */, vmad_prctl_comm,
                                &current->comm, TASK_COMM_LEN);
    if (r < 0) goto err;
    bytes += r;
#endif

    /* Now all the rest */
    for (i = 0; i < sizeof(vmad_prctl_tbl)/sizeof(vmad_prctl_tbl[0]); ++i) {
        unsigned int value;
        const int type = vmad_prctl_tbl[i].type;
        const int get = vmad_prctl_tbl[i].get_option;
        const int set = vmad_prctl_tbl[i].set_option;

        switch (vmad_prctl_tbl[i].type) {
        case vmad_prctl_int_val:
            r = vmadump_prctl_v(get, 0);
            if (!IS_ERR((void *)r)) { /* ICK */
                value = (unsigned int)r;
                r = 0;
            }
            break;

        case vmad_prctl_int_ref:
            r = vmadump_prctl_k(get, &value);
            break;
        }

        if (r < 0) {
#if 0 /* Confusing when issued, for instance, for PR_GET_ENDIAN where CPU lacks support */
            CR_WARN_CTX(ctx, "vmadump warning: failed to read prtcl(%d) err %ld", get, r);
#endif
            continue;
        }

        r = vmadump_store_prctl_aux(ctx, file, set, type, &value, sizeof(value));
        if (r < 0) goto err;
        bytes += r;
    }

    /* Write the list terminator */
    r = vmadump_store_prctl_aux(ctx, file, 0, 0, NULL, 0);
    if (r < 0) goto err;
    bytes += r;

    return bytes;

 err:
    if (r >= 0) r = -EIO;	/* Map short writes to EIO */
    return r;
}

loff_t vmadump_freeze_proc(cr_chkpt_proc_req_t *ctx, struct file *file,
			   struct pt_regs *regs, int flags) {
    loff_t r, bytes=0;
    static struct vmadump_header header ={VMAD_MAGIC, VMAD_FMT_VERS, VMAD_ARCH,
					  (LINUX_VERSION_CODE >> 16) & 0xFF,
					  (LINUX_VERSION_CODE >> 8) & 0xFF,
					  LINUX_VERSION_CODE & 0xFF };

    /*--- Write out the file header ---*/
    r = write_kern(ctx, file, &header, sizeof(header));
    if (r != sizeof(header)) goto err;
    bytes += r;

#if 0 /* Now handled w/ other prctl values */
    if (!(flags & VMAD_DUMP_REGSONLY)) {
        r = write_kern(ctx, file, current->comm, sizeof(current->comm));
        if (r != sizeof(current->comm)) goto err;
        bytes += r;
    }
#endif
    vmadump_store_prctl(ctx, file);

    /*--- PID ---*/
    r = write_kern(ctx, file, &current->pid, sizeof(current->pid));
    if (r != sizeof(current->pid)) goto err;
    bytes += r;

    /*--- Credentials (uids, gids and supplemental ids) ---*/
    r = cr_save_creds(ctx);
    if (r < 0) goto err;
    bytes += r;

    /*--- CPU State Information ------------------------------------*/
    r = vmadump_store_cpu(ctx, file, regs);
    if (r < 0) goto err;
    bytes += r;

    /*--- Signal information ---------------------------------------*/
    {
    int i;
    stack_t              sig_stack;
    sigset_t             sig_blocked;
    struct k_sigaction   sig_action;

    r = vmadump_sigaltstack_k(NULL, &sig_stack);
    if (r < 0) goto err;
    r = write_kern(ctx, file, &sig_stack, sizeof(sig_stack));
    if (r != sizeof(sig_stack)) goto err;
    bytes += r;

    spin_lock_irq(&current->sighand->siglock);
    memcpy(&sig_blocked, &current->blocked, sizeof(sig_blocked));
    spin_unlock_irq(&current->sighand->siglock);
    r = write_kern(ctx, file, &sig_blocked, sizeof(sig_blocked));
    if (r != sizeof(sig_blocked)) goto err;
    bytes += r;

    r = vmadump_store_sigpending(ctx, file, &current->pending);
    if (r < 0) goto err;
    bytes += r;
    if ((atomic_read(&current->signal->count) == 1) || !(flags & VMAD_DUMP_REGSONLY)) {
	/* Dump shared queue if not actually shared, or if we are the "leader" */
	r = vmadump_store_sigpending(ctx, file, &current->signal->shared_pending);
	if (r < 0) goto err;
	bytes += r;
    }

    if (!(flags & VMAD_DUMP_REGSONLY)) {
        for (i=0; i < _NSIG; i++) {
	    spin_lock_irq(&current->sighand->siglock);
	    memcpy(&sig_action, &current->sighand->action[i], sizeof(sig_action));
	    spin_unlock_irq(&current->sighand->siglock);

	    r = write_kern(ctx, file, &sig_action, sizeof(sig_action));
	    if (r != sizeof(sig_action)) goto err;
	    bytes += r;
        }
    }
    }

    /*--- Misc other stuff -----------------------------------------*/
    r = write_kern(ctx, file, &current->clear_child_tid,
		   sizeof(current->clear_child_tid));
    if (r != sizeof(current->clear_child_tid)) goto err;
    bytes += r;

    {				/* personality */
    unsigned long tmp_personality = current->personality;
    r = write_kern(ctx, file, &tmp_personality, sizeof(tmp_personality));
    if (r != sizeof(tmp_personality)) goto err;
    bytes += r;
    }

    /* XXX Will we need FUTEX related stuff here as well? */

    /*--- Memory Information ---------------------------------------*/
    if (!(flags & VMAD_DUMP_REGSONLY)) {
    struct vm_area_struct     *map, *next_map;
    struct vmadump_mm_info     mm_info;
    struct mm_struct          *mm = current->mm;
    struct vmadump_vma_header  term;
    unsigned long              next_addr;
#if HAVE_MM_MMAP_BASE
    unsigned long mmap_base;
#endif

    down_read(&mm->mmap_sem);
    mm_info.start_code  = mm->start_code;
    mm_info.end_code    = mm->end_code;
    mm_info.start_data  = mm->start_data;
    mm_info.end_data    = mm->end_data;
    mm_info.start_brk   = mm->start_brk;
    mm_info.brk         = mm->brk;
    mm_info.start_stack = mm->start_stack;
    mm_info.arg_start   = mm->arg_start;
    mm_info.arg_end     = mm->arg_end;
    mm_info.env_start   = mm->env_start;
    mm_info.env_end     = mm->env_end;
#if HAVE_MM_MMAP_BASE
    mmap_base           = mm->mmap_base;
#endif
    up_read(&mm->mmap_sem);

    r = write_kern(ctx, file, &mm_info, sizeof(mm_info));
    if (r != sizeof(mm_info)) goto err;
    bytes += r;

#if HAVE_MM_MMAP_BASE
    r = write_kern(ctx, file, &mmap_base, sizeof(mmap_base));
    if (r != sizeof(mmap_base)) goto err;
    bytes += r;
#endif

    down_read(&mm->mmap_sem);
    next_map = mm->mmap;
    next_addr = next_map ? next_map->vm_start : 0;
    while (next_map) {
	/* Call vma_find() to find the map we're looking for.  We
	 * have to do it this way because store_map needs to release
	 * the mmap_sem. */
	map = find_vma(mm, next_addr);
	if (map != next_map) break;
	next_map = map->vm_next;
	next_addr = next_map ? next_map->vm_start : 0;
	r = store_map(ctx, file, map, flags);
	if (r < 0) {
	    up_read(&mm->mmap_sem);
	    goto err;
	}
	bytes += r;
    }
    up_read(&mm->mmap_sem);

    /* Terminate maps list */
    term.start = term.end = ~0L;
    r = write_kern(ctx, file, &term, sizeof(term));
    if (r != sizeof(term)) goto err;
    bytes += r;
    }

#if 0 /* Hooks code not needed/maintained for BLCR */
    /*--- Call freeze hooks ----------------------------------------*/
    r = do_freeze_hooks(ctx, file, regs, flags);
    if (r < 0) goto err;
    bytes += r;
#endif

    return bytes;

 err:
    if (r >= 0) r = -EIO;	/* Map short writes to EIO */
    return r;
}

/*--------------------------------------------------------------------
 * syscall interface
 *------------------------------------------------------------------*/
#if 0 /* Not needed/maintained for BLCR */
long do_vmadump(long op, long arg0, long arg1, struct pt_regs *regs) {
    long retval;
    struct file *file;

    switch (op) {
    case VMAD_DO_DUMP:
	if (arg1 & ~VMAD_FLAG_USER_MASK) {
	    retval = -EINVAL;
	    break;
	}
	if ((file = fget(arg0))) {
	    retval = vmadump_freeze_proc(0, file, regs, arg1, 1);
	    fput(file);
	} else
	    retval = -EBADF;
	break;
    case VMAD_DO_UNDUMP:
	if ((file = fget(arg0))) {
	    retval = vmadump_thaw_proc(0, file, regs, 0, NULL);
	    fput(file);
	} else
	    retval = -EBADF;

	/* Un-dump is a whole lot like exec() */
	if (retval == 0) {
	    if (current->euid == current->uid && current->egid == current->gid)
		current->mm->dumpable = 1;
	    current->did_exec = 1;
	    current->self_exec_id++;
	    if (current->ptrace & PT_PTRACED)
                send_sig(SIGTRAP, current, 0);
	}
	break;
    case VMAD_DO_EXECDUMP: {
	struct vmadump_execdump_args args;
	char * filename;

	if (copy_from_user(&args, (void *) arg0, sizeof(args))) {
	    retval = -EFAULT;
	    break;
	}

	if (args.flags & ~VMAD_FLAG_USER_MASK) {
	    retval = -EINVAL;
	    break;
	}

	filename = getname(args.arg0);
	retval = PTR_ERR(filename);
	if (IS_ERR(filename)) break;

	file = fget(args.fd);
	if (!file) {
	    retval = -EBADF;
	    putname(filename);
	    break;
	}

	retval = do_execve(filename, (char **)args.argv,
			   (char **)args.envp, regs);
	putname(filename);
	if (retval) {
	    fput(file);
	    break;
	}

	/* Check to make sure we're actually allowed to dump :) */
	if (!current->mm->dumpable) { 
	    fput(file);
	    do_exit(-EPERM);
	}

	retval = vmadump_freeze_proc(0, file, regs, args.flags, 1);
	fput(file);
	if (retval > 0) retval = 0;
	do_exit(retval);		/* Ok, we're done... */
	/* NOT REACHED */
    } break;

    case VMAD_LIB_CLEAR:
    case VMAD_LIB_ADD:
    case VMAD_LIB_DEL:
    case VMAD_LIB_LIST:
    case VMAD_LIB_SIZE:
	lock_kernel();		/* very very lazy... */
	retval = do_lib_op(op, (char *) arg0, arg1);
	unlock_kernel();
	break;

    default:
	retval = -EINVAL;
    }
    return retval;
}
#endif

/*--------------------------------------------------------------------
 *  New binary format code
 *------------------------------------------------------------------*/
#ifdef HAVE_BINFMT_VMADUMP

static
int load_vmadump(struct linux_binprm *bprm, struct pt_regs *regs) {
    int retval;
    struct vmadump_header *header;

    header = (struct vmadump_header *) bprm->buf;
    if (memcmp(header->magic, vmad_magic, sizeof(header->magic)) != 0  ||
	header->fmt_vers != VMAD_FMT_VERS ||
	header->major != ((LINUX_VERSION_CODE >> 16) & 0xFF) ||
	header->minor != ((LINUX_VERSION_CODE >> 8) & 0xFF)
#if defined(STRICT_VERSION_CHECK) && STRICT_VERSION_CHECK
	|| header->patch != (LINUX_VERSION_CODE & 0xFF)
#endif
	)
	return -ENOEXEC;

    if (!bprm->file->f_op || !bprm->file->f_op->mmap)
	return -ENOEXEC;

    retval = vmadump_thaw_proc(0, bprm->file, regs, 0, NULL);
    if (retval == 0) {
	if (current->euid == current->uid && current->egid == current->gid)
	    current->mm->dumpable = 1;
	current->did_exec = 1;
	current->self_exec_id++;
#if 0
        if (current->exec_domain && current->exec_domain->module)
	    __MOD_DEC_USE_COUNT(current->exec_domain->module);
        if (current->binfmt && current->binfmt->module)
	    __MOD_DEC_USE_COUNT(current->binfmt->module);
	current->exec_domain = 0;
#endif
	current->binfmt = 0;
    }
    return retval;
}

struct linux_binfmt vmadump_fmt = {
    0,THIS_MODULE,load_vmadump,0,0,0
};
#endif



/* This is some stuff that allows vmadump to latch onto the BProc
 * syscall for testing purposes. */
#ifdef CONFIG_BPROC
static int syscall = 0;
MODULE_PARM(syscall, "i");
MODULE_PARM_DESC(syscall,
"Syscall number to allow calling VMADump directly.  The default (0) means "
"no VMADump syscall.  There is no bounds check on the syscall number so "
"be careful with this option.");
#endif

int __init vmadump_init_module(void) {
    CR_INFO("vmadump: (from bproc-%s) Erik Hendriks "
           "<erik@hendriks.cx>", __stringify(BPROC_VERSION));
    CR_INFO("vmadump: Modified for %s <%s>",
           PACKAGE_STRING, PACKAGE_BUGREPORT);

    /* Check for correct blcr_imports */
#if 0
	if (strcmp(blcr_config_timestamp, BLCR_CONFIG_TIMESTAMP)) {
	CR_ERR("vmadump: Module blcr_imports timestamp (%s) does not match that of blcr_vmadump (" BLCR_CONFIG_TIMESTAMP ").", blcr_config_timestamp);
	return -EINVAL;
    }
#endif
#if 0 /* Not needed/maintained for BLCR */
    init_rwsem(&hook_lock);
#endif
#ifdef CONFIG_BPROC
    {
	/*extern struct rw_semaphore do_bproc_lock;*/
	extern long (*do_bproc_ptr)(long,long,long,struct pt_regs *);
	/*down_write(&do_bproc_lock);*/
	do_bproc_ptr = 0;
	if (syscall) {
	    if (do_bproc_ptr)
		printk("vmadump: BProc syscall hook is occupied. "
		       "Can't attach to BProc syscall.\n");
	    else {
		do_bproc_ptr = do_vmadump;
		printk("vmadump: Attached to BProc syscall.\n");
	    }
	}
	/*up_write(&do_bproc_lock);*/
    }
#endif
#ifdef HAVE_BINFMT_VMADUMP
    register_binfmt(&vmadump_fmt);
#endif
    return 0;
}

void __exit vmadump_cleanup_module(void) {
#if 0 /* Not needed/maintained for BLCR */
    liblist_clear();
#endif
#ifdef HAVE_BINFMT_VMADUMP
    unregister_binfmt(&vmadump_fmt);
#endif

#ifdef CONFIG_BPROC
    {
	/*extern struct rw_semaphore do_bproc_lock;*/
	extern long (*do_bproc_ptr)(long,long,long,struct pt_regs *);
	if (syscall) {
	    /*down_write(&do_bproc_lock);*/
	    if (do_bproc_ptr == do_vmadump)
		do_bproc_ptr = 0;
	    /*up_write(&do_bproc_lock);*/
	}
    }
#endif
}

#if 0 /* EXPORTs are not needed/maintained for BLCR */
EXPORT_SYMBOL(vmadump_freeze_proc);
EXPORT_SYMBOL(vmadump_thaw_proc);
EXPORT_SYMBOL(do_vmadump);

EXPORT_SYMBOL(vmadump_add_hook);
EXPORT_SYMBOL(vmadump_del_hook);
EXPORT_SYMBOL(vmadump_read_u);
EXPORT_SYMBOL(vmadump_read_k);
EXPORT_SYMBOL(vmadump_write_u);
EXPORT_SYMBOL(vmadump_write_k);
#endif

/*
 * Local variables:
 * c-basic-offset: 4
 * End:
 */
