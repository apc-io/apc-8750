/*-------------------------------------------------------------------------
 *  vmadump.h:  Definitions for VMADump
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
 * $Id: vmadump.h,v 1.47 2008/12/15 22:55:24 phargrov Exp $
 *
 *  THIS VERSION MODIFIED FOR BLCR <http://ftg.lbl.gov/checkpoint>
 *-----------------------------------------------------------------------*/
#ifndef _VMADUMP_H
#define _VMADUMP_H

/* Everything past this struct is architecture dependent */
struct vmadump_header {
    unsigned char magic[3];
    unsigned char fmt_vers;	/* version of dump format */
    unsigned char arch;
    unsigned char major;	/* Kernel rev dumped from */
    unsigned char minor;
    unsigned char patch;
};

struct vmadump_vma_header {
    unsigned long  start;	/* ~0 = end of list */
    unsigned long  end;
    unsigned long  flags;
    unsigned long  namelen;	/* 0 = data follows */
    unsigned long  offset;	/* file offset for mmap */
};

struct vmadump_page_header {
    unsigned long start;	/* ~0 = end of list */
    unsigned int num_pages;
};

struct vmadump_mm_info {
    unsigned long start_code, end_code;
    unsigned long start_data, end_data;
    unsigned long start_brk,  brk;
    unsigned long start_stack;
    unsigned long arg_start, arg_end;
    unsigned long env_start, env_end;
};

struct vmadump_page_list_header {
    unsigned int fill;
    /* May add hash list or other data here later */
};

#define VMAD_CHUNKHEADER_SIZE PAGE_SIZE
#define VMAD_CHUNKHEADER_MIN sizeof(struct vmadump_page_header)
#define VMAD_END_OF_CHUNKS ~0UL

#define VMAD_HOOK_TAG_LEN 16
struct vmadump_hook_header {
    char tag[VMAD_HOOK_TAG_LEN];
    long size;			/* NOT including the header */
};

#define VMAD_MAGIC {0x56,0x4d,0x41}
#define VMAD_FMT_VERS 6

#define VMAD_ARCH_i386   1
#define VMAD_ARCH_sparc  2
#define VMAD_ARCH_alpha  3
#define VMAD_ARCH_ppc    4
#define VMAD_ARCH_x86_64 5
#define VMAD_ARCH_ppc64  6
#define VMAD_ARCH_arm    7
#define VMAD_ARCH_sparc64 8

#if defined(__i386__)
#define VMAD_ARCH VMAD_ARCH_i386
#define HAVE_BINFMT_VMADUMP 1
#elif defined(__sparc__) && defined(__arch64__)
#define VMAD_ARCH VMAD_ARCH_sparc64
#elif defined(__sparc__)
#define VMAD_ARCH VMAD_ARCH_sparc
#define HAVE_BINFMT_VMADUMP 1
#elif defined(__alpha__)
#define VMAD_ARCH VMAD_ARCH_alpha
/* Alpha execve doesn't save enough state on the way into the kernel
 * to support complete state restoration, so no binfmt_vmadump on
 * alpha. */
#undef HAVE_BINFMT_VMADUMP
#elif defined(powerpc)
#define VMAD_ARCH VMAD_ARCH_ppc
#define HAVE_BINFMT_VMADUMP 1
#elif defined(__x86_64__)
#define VMAD_ARCH VMAD_ARCH_x86_64
/* The complications of syscall return in the mixed mode world make
 * this one not worth it... */
#undef  HAVE_BINFMT_VMADUMP
#elif defined(__powerpc64__)
#define VMAD_ARCH VMAD_ARCH_ppc64
#define HAVE_BINFMT_VMADUMP 1
#elif defined(__arm__)
#define VMAD_ARCH VMAD_ARCH_arm
#define HAVE_BINFMT_VMADUMP 1
#else
#error VMADUMP does not support this architecture 
#endif

/* Defs for the syscall interface */
#define VMAD_DO_DUMP       0x00 
#define VMAD_DO_UNDUMP     0x01
#define VMAD_DO_EXECDUMP   0x02

#define VMAD_LIB_CLEAR 0x30
#define VMAD_LIB_ADD   0x31
#define VMAD_LIB_DEL   0x32
#define VMAD_LIB_LIST  0x33
#define VMAD_LIB_SIZE  0x34

/*--- Flags for dump/undump ---*/
#define VMAD_DUMP_LIBS  1
#define VMAD_DUMP_EXEC  2
#define VMAD_DUMP_OTHER 4
#define VMAD_DUMP_ALL   7

#define VMAD_FLAG_BPROC 8
/* These are the flags that the user is allowed to set */
#define VMAD_FLAG_USER_MASK 0x7

struct vmadump_execdump_args {
    const char *arg0;
    char * const *argv;
    char * const *envp;
    
    int fd;
    int flags;
};

#include "blcr_vmadump.h"

#ifdef __KERNEL__
#include "cr_module.h"

extern int vmadump_init_module(void);
extern void vmadump_cleanup_module(void);

#if 0 /* BLCR uses its own "proc_req" structures in place of this structure */
struct vmadump_map_ctx {
    /* map_open:
     *  given a file, open it.  The file should be opened with
     *  the given flags.  The filename argument is a kernel space pointer.
     *  The semantics should be identical to:
     *    filp_open(filename, flags);
     *
     * map_name:
     *  given a file structure, return the canonical name for it.  The
     *  semantics should be identical to:
     *    d_path(file->f_dentry, file->f_vfsmnt, buffer, size);
     *
     * read_file, write_file:
     *  Alternate file read/write routines.  These should have the
     *  same semantics as a blocking sys_read and sys_write.  */
    struct file *(*map_open)  (struct vmadump_map_ctx *ctx,
			       const char *filename, int flags);
    char        *(*map_name)  (struct vmadump_map_ctx *ctx, struct file *,
			       char *buffer, int size);
    ssize_t      (*read_file) (struct vmadump_map_ctx *, struct file *,
			       void *, size_t);
    ssize_t      (*write_file)(struct vmadump_map_ctx *, struct file *,
			       const void *, size_t);
    void *private;
};
#endif


extern loff_t vmadump_store_page_list(cr_chkpt_proc_req_t *ctx, struct file *file,
				      unsigned long start, unsigned long end);
extern loff_t vmadump_store_dirty_page_list(cr_chkpt_proc_req_t *ctx, struct file *file,
				      unsigned long start, unsigned long end);
extern int vmadump_load_page_list(cr_rstrt_proc_req_t *ctx,
				  struct file *file, int is_exec);

extern loff_t vmadump_freeze_proc(cr_chkpt_proc_req_t *, struct file *file,
				  struct pt_regs *regs, int flags);
extern long vmadump_thaw_proc  (cr_rstrt_proc_req_t *, struct file *file,
				struct pt_regs *regs, int flags);

#if 0 /* Neither used nor maintened in BLCR */
extern long do_vmadump         (long op, long arg0, long arg1, struct pt_regs *regs);
#endif


#if 0 /* Neither used nor maintened in BLCR */
/*--- Interface for modules installing vmadump hooks ---------------*/
struct vmadump_hook_handle;
struct vmadump_hook {
    char *tag;			/* name */
    /* freeze:
     *
     * return value - total number of bytes written INCLUDING header(s)
     * 
     *
     */
    long (*freeze)(struct vmadump_hook_handle *h, struct pt_regs *regs,
		   int flags);
    int  (*thaw  )(struct vmadump_hook_handle *h,
		   struct vmadump_hook_header *hh,
		   struct pt_regs *regs);
};

extern int     vmadump_add_hook(struct vmadump_hook *hook);
extern int     vmadump_del_hook(struct vmadump_hook *hook);

/* Read/write routines for hooks */
extern ssize_t vmadump_read_u  (struct vmadump_hook_handle *h,
				void *buf, size_t count);
extern ssize_t vmadump_read_k  (struct vmadump_hook_handle *h,
				void *buf, size_t count);
extern ssize_t vmadump_write_u (struct vmadump_hook_handle *h,
				const void *buf, size_t count);
extern ssize_t vmadump_write_k (struct vmadump_hook_handle *h,
				const void *buf, size_t count);
#endif

#ifdef __VMADUMP_INTERNAL__
#if 0
#if defined(__i386__)
#define reg_arg0(r)      ((r)->ebx)
#define reg_arg1(r)      ((r)->ecx)
#define reg_arg2(r)      ((r)->edx)
#elif defined(__sparc__)
#define reg_arg0(r)      ((r)->u_regs[((r)->u_regs[UREG_G1]==0?1:0)+UREG_I0])
#define reg_arg1(r)      ((r)->u_regs[((r)->u_regs[UREG_G1]==0?1:0)+UREG_I1])
#define reg_arg2(r)      ((r)->u_regs[((r)->u_regs[UREG_G1]==0?1:0)+UREG_I2])
#elif defined(__alpha__)
#define reg_arg0(r)      ((r)->trap_a0)
#define reg_arg1(r)      ((r)->trap_a1)
#define reg_arg2(r)      ((r)->trap_a2)
#elif defined(powerpc)
#define reg_arg0(r)      ((r)->gpr[3])
#define reg_arg1(r)      ((r)->gpr[4])
#define reg_arg2(r)      ((r)->gpr[5])
#elif defined(__x86_64__)
#define reg_arg0(r)      ((r)->rdi)
#define reg_arg1(r)      ((r)->rsi)
#define reg_arg2(r)      ((r)->rdx)
#elif defined(__arm__)
#define reg_arg0(r)      ((r)->uregs[ 0 ])
#define reg_arg1(r)      ((r)->uregs[ 1 ])
#define reg_arg2(r)      ((r)->uregs[ 2 ])
#else
#error reg_arg[0-2] needs to be defined for this arch
#endif
#endif

#define CR_ERR_CTX CR_ERR_PROC_REQ
#define CR_WARN_CTX CR_WARN_PROC_REQ

int  vmadump_arch_init   (void);
void vmadump_arch_cleanup(void);
long vmadump_store_cpu  (cr_chkpt_proc_req_t *ctx, struct file *file,
			 struct pt_regs *regs);
int  vmadump_restore_cpu(cr_rstrt_proc_req_t *ctx, struct file *file,
			 struct pt_regs *regs);
#if 0 /* BLCR uses its own I/O paths */
extern ssize_t write_kern(struct vmadump_map_ctx *ctx, struct file *file,
			  const void *buf, size_t count);
extern ssize_t read_kern(struct vmadump_map_ctx *ctx, 
			 struct file *file, void *buf, size_t count);
#else
  /* Wrapper for I/O error reporting from vmadump. */
  #define io_wrap(_op,_ctx,_file,_buf,_count) \
		cr_##_op((_ctx)->req->errbuf,(_file),(_buf),(_count))
  #define write_kern(_ctx,_file,_buf,_count)	io_wrap(kwrite,_ctx,_file,_buf,_count)
  #define read_kern(_ctx,_file,_buf,_count)	io_wrap(kread,_ctx,_file,_buf,_count)
  #define write_user(_ctx,_file,_buf,_count)	io_wrap(uwrite,_ctx,_file,_buf,_count)
  #define read_user(_ctx,_file,_buf,_count)	io_wrap(uread,_ctx,_file,_buf,_count)
#endif
#if VMAD_HAVE_ARCH_MAPS
extern loff_t vmad_store_arch_map(cr_chkpt_proc_req_t *ctx,
				  struct file *file,
				  struct vm_area_struct *map, int flags);
extern int vmad_load_arch_map(cr_rstrt_proc_req_t *ctx,
			      struct file *file,
			      struct vmadump_vma_header *head);
#endif /* if VMAD_HAVE_ARCH_MAPS */
#endif /* ifdef __VMADUMP_INTERNAL__ */
#endif /* ifdef __KERNEL__ */
#endif

/*
 * Local variables:
 * c-basic-offset: 4
 * End:
 */
