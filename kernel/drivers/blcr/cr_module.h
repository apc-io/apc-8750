/*
 * Berkeley Lab Checkpoint/Restart (BLCR) for Linux is Copyright (c)
 * 2008, The Regents of the University of California, through Lawrence
 * Berkeley National Laboratory (subject to receipt of any required
 * approvals from the U.S. Dept. of Energy).  All rights reserved.
 *
 * Portions may be copyrighted by others, as may be noted in specific
 * copyright notices within specific files.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id: cr_module.h,v 1.285.4.2 2009/06/07 03:27:36 phargrov Exp $
 */

#ifndef _CR_MODULE_H
#define _CR_MODULE_H        1
#define _IN_CR_MODULE_H     1

#include "blcr_config.h"	// Configuration

#include <linux/module.h>
#include <linux/autoconf.h>

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/version.h>
#include <linux/list.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/mount.h>
#include <linux/slab.h>
#include <linux/highmem.h>
#include <linux/spinlock.h>
#include <linux/vmalloc.h>
#include <linux/poll.h>
#include <asm/atomic.h>
#include <linux/namei.h>
#if HAVE_LINUX_SYSCALLS_H
  #include <linux/syscalls.h>           /* for sys_close, etc. */
#endif
#include <linux/rcupdate.h>
#include <linux/hash.h>
#if defined(CONFIG_COMPAT)
  #include <linux/compat.h>
  /* This mapping works only because we only pass pointers and longs */
  #define CR_IOCTL32_MAP(x)	((((x)&0xff0000) == 0x80000) ? (((x)&~0xff0000)|0x40000) : (x))
  #define CR_IOCTL32_UNMAP(x)	((((x)&0xff0000) == 0x40000) ? (((x)&~0xff0000)|0x80000) : (x))
#endif

#include "blcr_common.h"	// For the stuff common to user space
#include "blcr_imports.h"	// Prototypes for non-exported and/or non-prototyped kernel symbols

#include "arch/arm/cr_arch.h"		// Architecture-specific bits
#include "cr_barrier.h"		// Code for kernel barriers.
#include "cr_ktrace.h"		// Code for trace messages

// forward decls/typedefs for use w/i vmadump
struct cr_chkpt_preq_s;
typedef struct cr_chkpt_preq_s cr_chkpt_proc_req_t;
struct cr_rstrt_preq_s;
typedef struct cr_rstrt_preq_s cr_rstrt_proc_req_t;
#include "vmadump.h"

#include "cr_kcompat.h"		// For providing the illusion of a uniform kernel platform

#undef _IN_CR_MODULE_H


// Current and oldest-supported versions of the context file format
// 
// Must advance CR_CONTEXT_VERSION in each public release that generates
// context files not readable by the previous release.
// Must correct CR_CONTEXT_VERSION_MIN in any public release that cannot
// read context files produced by older versions.
#define CR_CONTEXT_VERSION 8
#define CR_CONTEXT_VERSION_MIN 8

// cr_objectmap_t is an opaque type
struct cr_objectmap_s;
typedef struct cr_objectmap_s *cr_objectmap_t;

// cr_rstrt_relocate_t is an opaque type
struct cr_rstrt_relocate_s;
typedef struct cr_rstrt_relocate_s *cr_rstrt_relocate_t;

// Foward type decls:
struct cr_mmaps_desc;

// Source/Destination location info for a checkpoint
typedef	struct cr_location_s {
	int			is_write;

	// file pointer (NULL iff location is a directory)
	struct file *		filp;
	struct semaphore	mutex;

	// fs struct pointer (NULL iff location is not a directory)
	struct fs_struct *	fs;
} cr_location_t;

typedef struct cr_work_s {
	struct list_head	list;
	void			(*f)(struct cr_work_s *);
} cr_work_t;
#define CR_INIT_WORK(_work, _func) do { \
	(_work)->f = (_func);           \
	INIT_LIST_HEAD(&(_work)->list); \
    } while (0)

// memory pools for our most common types
extern cr_kmem_cache_ptr cr_pdata_cachep;
extern cr_kmem_cache_ptr cr_task_cachep;
extern cr_kmem_cache_ptr cr_chkpt_req_cachep;
extern cr_kmem_cache_ptr cr_chkpt_proc_req_cachep;
extern cr_kmem_cache_ptr cr_rstrt_req_cachep;
extern cr_kmem_cache_ptr cr_rstrt_proc_req_cachep;

// Kernel-side tracking of a checkpoint request
struct cr_chkpt_preq_s { // grumble... need short name for KMEM_CACHE()
	struct list_head	list;
	int			ref_count;	// (non-atomic) count of references
	const struct mm_struct	*mm;
	struct cr_chkpt_req_s	*req;

	struct list_head	tasks;		// Tasks in this proc_req (protected by req->lock) */
	int			thread_count;
        struct file            *file;		// per-process context file (may equal master)
	struct semaphore        serial_mutex;   // globally order steps in req
	cr_barrier_t		phase_barrier;		// between phases 1 & 2
	cr_barrier_t		predump_barrier;	// at start of dump
	cr_barrier_t		vmadump_barrier;	// at end of vmadump
	int			omit;
	struct k_sigaction	saved_sa;	// Space to save user-registered sigaction.
	struct k_sigaction	forced_sa;	// Space to save blcr-registered sigaction.

	cr_bool_t               have_leader;
	cr_bool_t               done_init;
	cr_bool_t               done_header;
	cr_bool_t               done_linkage;
	cr_bool_t               done_fs;
	cr_bool_t               done_mmaps_maps;
	cr_bool_t               done_itimers;
	cr_bool_t               done_files;
	cr_bool_t               done_mmaps_data;
	cr_bool_t               done_fini;

	/* To ensure req->signal (if non-zero) is delivered correctly */
	cr_barrier_t		pre_complete_barrier;
	cr_barrier_t		post_complete_barrier;

	/* For vmadump thread management */
	wait_queue_head_t	wait;
	cr_bool_t               done_leader;

	/* For special mmap()s tracking */
	int                     mmaps_cnt;
	struct cr_mmaps_desc   *mmaps_tbl;

	/* For pause/resume and save of itimers */
	struct itimerval	itimers[3];
	cr_bool_t               done_resume_itimers;

	cr_bool_t		duplicate_flag;

	/* For fd to pass to the signal handler */
	int			ctrl_fd;	// >= 0 if any of our threads are "registered"...
	int			tmp_fd;		// ...else open() at trigger, close() in OP_HAND_CHKPT
};
typedef struct cr_chkpt_req_s {
	pid_t			requester;	// who requested the checkpoint
	pid_t			target;		// who is to be checkpointed
	atomic_t		ref_count;	// reference count
	atomic_t		completed;	// count of successful completions
	rwlock_t		lock;		// spinlock for task_list & wait
	wait_queue_head_t	wait;		// place for requester to wait
	struct list_head	tasks;		// list of target cr_task_t's
	struct list_head	procs;		// list of target cr_chkpt_proc_req_t's
	cr_location_t		dest;		// checkpoint destination
	struct semaphore        serial_mutex;   // mutex for i/o serialization
        cr_bool_t               done_header;
        cr_bool_t               done_trailer;
	unsigned int		flags;		// flags supplied by requestor
	int			result;		// value returned by REAP
	int			die;		// flag indicating we must die
	int			has_expiration;	// 'expiration' is used
	unsigned long		expiration;	// limit on time to chkpt
	cr_work_t		work;
        cr_format_t             dump_format;    // Format to use for this dump
	int			signal;		// sent after CHECKPOINT
	cr_scope_t              checkpoint_scope;
	cr_objectmap_t          map;
	cr_barrier_t            preshared_barrier; // before files & mmaps_data
	cr_barrier_t		postdump_barrier; // at end of dump
	struct file		*ctrl_file;
	cr_errbuf_t		*errbuf;
} cr_chkpt_req_t;

#define CR_CHKPT_RESTARTED ((cr_chkpt_req_t *)1UL)

typedef volatile enum {
	CR_RSTRT_STATE_REQUESTER,	// Requester needs to fork() child to read context
	CR_RSTRT_STATE_CHILD,		// A child is reading the context file
	CR_RSTRT_STATE_DONE		// Finished, possibly in error
} cr_rstrt_state_t;

struct cr_rstrt_preq_s { // grumble... need short name for KMEM_CACHE()
	struct list_head	list;
	const struct mm_struct	*mm;
	struct cr_rstrt_req_s	*req;

	cr_bool_t               have_leader;
        cr_bool_t               done_fs;
	cr_bool_t               done_mmaps_maps;
	cr_bool_t               done_itimers;
        cr_bool_t               done_files;
	cr_bool_t               done_mmaps_data;
	cr_bool_t               done_hide_cr_fds;
	cr_bool_t               done_close_cr_fds;
	cr_bool_t               done_linkage;
	int                     clone_flags;
	int	                clones_needed;
	struct list_head	linkage;
	int                     linkage_size;
	int                     thread_count;
	struct list_head	tasks;
	struct semaphore        serial_mutex;
        struct file            *file;		// per-process context file (may equal master)
	atomic_t		final_counter;
	/* For vmadump thread management and inter-thread communication */
	cr_barrier_t		pre_vmadump_barrier;
	cr_barrier_t		post_vmadump_barrier;
	wait_queue_head_t	wait;
	cr_bool_t               done_leader;
	int			thaw_error;

	/* To ensure req->signal (if non-zero) is delivered correctly */
	cr_barrier_t		pre_complete_barrier;
	cr_barrier_t		post_complete_barrier;

	/* For special mmap()s tracking */
	int                     mmaps_cnt;
	struct cr_mmaps_desc   *mmaps_tbl;

	/* For restore of itimers */
	struct itimerval	itimers[3];
	cr_bool_t               done_resume_itimers;

	int			tmp_fd;		// fd, if any, openned at trigger time
};
typedef struct cr_rstrt_req_s {
	pid_t			requester;	// who requested the restart
        cr_location_t           src;
	atomic_t                ref_count;
	struct task_struct     *cr_restart_task;
	struct file            *cr_restart_stdin;
	struct file            *cr_restart_stdout;
	int	                need_procs;
	cr_rstrt_state_t        state;
	wait_queue_head_t	wait;		// place for requester to wait for state changes
	int			result;		// value returned by REAP
	int			die;		// indicates spawned threads must die
	unsigned int		flags;		// flags supplied by requestor
	cr_objectmap_t          map;		// map from old object addrs to new
        struct file            *file0;		// "master" context file
	cr_barrier_t		barrier;
	rwlock_t		lock;		// protects procs and tasks lists
	struct list_head	procs;		// list of target cr_rstrt_proc_req_t's
	struct list_head	tasks;		// list of target tasks not yet completed
	struct list_head	linkage;	// list of "linkage" entries
	cr_scope_t		scope;
	int			signal;
	cr_work_t		work;
	cr_rstrt_relocate_t	relocate;	// For path relocations
	cr_errbuf_t		*errbuf;
} cr_rstrt_req_t;

typedef enum {
	CR_NO_PHASE = 0,
	CR_PHASE1,
	CR_PHASE2
} cr_phase_t;

// For forming a list of tasks.
typedef struct cr_task_s {
	struct list_head	task_list;	// All tasks
	struct list_head	req_list;	// Tasks in same req
	struct list_head	proc_req_list;	// Tasks in same proc_req
	atomic_t		ref_count;
	struct task_struct	*task;
	cr_chkpt_req_t		*chkpt_req;
	cr_chkpt_proc_req_t	*chkpt_proc_req;
	cr_rstrt_req_t		*rstrt_req;
	cr_rstrt_proc_req_t	*rstrt_proc_req;
	cr_phase_t		phase;		// When setting phase, we also set the next three:
	struct k_sigaction	handler_sa;	//   sigaction in place when phase was set
	struct file		*filp;		//   filp used to set phase
	int			fd;		//   fd "registered" with phase
	int			stopped;
	int			step;		// Step in progress, for recovery if task dies
	u32			self_exec_id;   // For detection of ill-timed exec()
	unsigned long		chkpt_flags;	// flags supplied at checkpoint time
} cr_task_t;

// Private data attached to an instance of the file
typedef struct cr_pdata_s {
	cr_chkpt_req_t		*chkpt_req;
	cr_rstrt_req_t	        *rstrt_req;
} cr_pdata_t;


/* Steps for checkpoint - named for the syncronization that comes next: */
enum {
	CR_CHKPT_STEP_PRESHARED = 0,
	CR_CHKPT_STEP_PHASE,
	CR_CHKPT_STEP_PREDUMP,
	CR_CHKPT_STEP_VMADUMP,
	CR_CHKPT_STEP_POSTDUMP,
	CR_CHKPT_STEP_PRE_COMPLETE,
	CR_CHKPT_STEP_POST_COMPLETE,
	CR_CHKPT_STEP_DONE
};

/* Steps for restart - named for the syncronization that comes next: */
enum {
	CR_RSTRT_STEP_PRE_VMADUMP = 0,
	CR_RSTRT_STEP_POST_VMADUMP,
	CR_RSTRT_STEP_REQ_BARRIER,
	CR_RSTRT_STEP_PRE_COMPLETE,
	CR_RSTRT_STEP_POST_COMPLETE,
	CR_RSTRT_STEP_DONE
};

#if CRI_DEBUG || 1 /* Always on for now */
  #define CR_ASSERT_STEP_inner(_t, _s, _cmp) \
    do { \
      if (!((_t)->step _cmp (_s))) \
        CR_ERR("%s:%d Expect step " #_cmp " " #_s "(%d) but found %d", __FUNCTION__, __LINE__, (_s), (_t)->step); \
    } while (0)
#else
  #define CR_ASSERT_STEP_inner(_t, _s, _cmp)	do {} while(0)
#endif
#define CR_ASSERT_STEP_EQ(_t, _s) CR_ASSERT_STEP_inner(_t, _s, ==)
#define CR_ASSERT_STEP_NE(_t, _s) CR_ASSERT_STEP_inner(_t, _s, !=)
#define CR_ASSERT_STEP_GE(_t, _s) CR_ASSERT_STEP_inner(_t, _s, >=)
#define CR_ASSERT_STEP_GT(_t, _s) CR_ASSERT_STEP_inner(_t, _s, >)
#define CR_ASSERT_STEP_LE(_t, _s) CR_ASSERT_STEP_inner(_t, _s, <=)
#define CR_ASSERT_STEP_LT(_t, _s) CR_ASSERT_STEP_inner(_t, _s, <)

// cr_proc.c
extern int cr_proc_init(void);
extern void cr_proc_cleanup(void);

// cr_fops.c
extern struct file_operations cr_ctrl_fops;
extern int cr_hand_complete(struct file *filp, unsigned int flags);
extern int cr_hand_abort(struct file *filp, unsigned int flags);
#if defined(CONFIG_COMPAT) && !defined(HAVE_COMPAT_IOCTL)
extern int cr_init_ioctl32(void);
extern int cr_cleanup_ioctl32(void);
#endif

// cr_dump_self.c
extern int cr_dump_self(struct file *filp, unsigned long flags);

// cr_chkpt_req.c
extern int cr_chkpt_req(struct file *filp, struct cr_chkpt_args __user *arg);
extern unsigned int cr_chkpt_poll(struct file *filp, poll_table *wait);
extern int cr_chkpt_reap(struct file *filp);
extern int cr_chkpt_log(struct file *filp, struct cr_log_args __user *arg);
extern int cr_chkpt_task_complete(cr_task_t *cr_task, int block);
extern int cr_chkpt_fwd(struct file *filp, struct cr_fwd_args __user *arg);
extern void cr_chkpt_req_release(struct file *filp, cr_pdata_t *priv);
extern int cr_signal_predump_barrier(cr_task_t *cr_task, int block);
extern void cr_signal_phase_barrier(cr_task_t *cr_task, int block, int need_lock);
extern void cr_chkpt_advance_to(cr_task_t *cr_task, int step, int hold_lock);
extern int cr_chkpt_abort(cr_task_t *cr_task, unsigned int flags);
extern int cr_chkpt_info(struct file *filp, struct cr_chkpt_info __user *arg);

// cr_async.c
extern int cr_suspend(struct file *filp, struct timeval __user *arg);
extern int cr_phase1_register(struct file *filp, int arg);
extern void cr_phase1_release(struct file *filp, cr_pdata_t *priv);

// cr_sync.c
extern int cr_phase2_register(struct file *filp, int arg);
extern void cr_phase2_release(struct file *filp, cr_pdata_t *priv);

// cr_task.c
extern rwlock_t cr_task_lock;
extern struct list_head cr_task_list;
extern cr_task_t *__cr_task_get(struct task_struct *task, int create);
extern void __cr_task_put(cr_task_t *cr_task);
extern cr_task_t *cr_task_get(struct task_struct *task);
extern void cr_task_put(cr_task_t *cr_task);

// cr_rstrt_req.c
extern int cr_rstrt_request_restart(struct file *filp, struct cr_rstrt_args __user *arg);
extern unsigned int cr_rstrt_poll(struct file *filp, poll_table *wait);
extern int cr_rstrt_reap(struct file *filp);
extern int cr_rstrt_log(struct file *filp, struct cr_log_args __user *arg);
extern int cr_rstrt_child(struct file *filp);
extern int cr_rstrt_procs(struct file *filp, struct cr_procs_tbl __user *arg);
extern int cr_rstrt_src(struct file *filp, char __user *arg);
extern int cr_rstrt_req_release(struct file *filp, cr_pdata_t *priv);
extern int cr_rstrt_task_complete(cr_task_t *cr_task, int block, int need_lock);
extern int cr_rstrt_abort(cr_task_t *cr_task, unsigned int flags);
extern int cr_rstrt_init(void);
extern void cr_rstrt_cleanup(void);

// cr_dest_file.c
extern int cr_loc_init(cr_errbuf_t *eb, cr_location_t *loc, int fd, struct file *from, int is_write);
extern void cr_loc_free(cr_location_t *loc);
extern struct file *cr_loc_get(cr_location_t *loc, int *shared);
extern void cr_loc_put(cr_location_t *loc, struct file *filp);

// cr_trigger.c
extern int __cr_trigger_phase1(cr_chkpt_proc_req_t *proc_req);
extern int __cr_trigger_phase1_only(cr_chkpt_proc_req_t *req);
extern int cr_trigger_phase1(cr_chkpt_req_t *req);
extern int __cr_trigger_phase2(cr_chkpt_proc_req_t *proc_req);
extern int cr_trigger_phase2(cr_chkpt_req_t *req, cr_chkpt_proc_req_t *proc_req);

// cr_io.c
extern ssize_t cr_uread(cr_errbuf_t *eb, struct file * file, void *buf, size_t count);
extern ssize_t cr_uwrite(cr_errbuf_t *eb, struct file * file, const void *buf, size_t count);
extern ssize_t cr_kread(cr_errbuf_t *eb, struct file * file, void *buf, size_t count);
extern ssize_t cr_kwrite(cr_errbuf_t *eb, struct file * file, const void *buf, size_t count);
extern int cr_skip(struct file *filp, loff_t len);
extern int cr_fgets(cr_errbuf_t *eb, char *buf, int size, struct file *filp);
extern int cr_fputs(cr_errbuf_t *eb, const char *buf, struct file *filp);
extern int cr_save_pathname(cr_errbuf_t *eb, struct file *cr_filp, struct path *path, char *buf, int size);
extern int cr_save_filename(cr_errbuf_t *eb, struct file *cr_filp, struct file *filp, char *buf, int size);
extern const char *__cr_getname(cr_errbuf_t *eb, struct file *filp, int null_ok);
extern const char *cr_getname(cr_errbuf_t *eb, cr_rstrt_relocate_t reloc, struct file *filp, int null_ok);
extern const char *cr_location2path(cr_location_t *loc, char *buf, int size);
extern struct file * cr_dentry_open(struct dentry *dentry, struct vfsmount *mnt, int flags);
extern struct dentry *cr_mknod(cr_errbuf_t *eb, struct nameidata *nd, const char *name, int mode, unsigned long unlinked_id);
extern struct file *cr_filp_mknod(cr_errbuf_t *eb, const char *name, int mode, int flags, unsigned long unlinked_id);
extern int cr_filp_chmod(struct file *filp, mode_t mode);
extern struct file *cr_mkunlinked(cr_errbuf_t *eb, struct file *cr_filp, const char *name, int mode, int flags, loff_t size, unsigned long unlinked_id);
extern loff_t cr_sendfile(cr_errbuf_t *eb, struct file *dst_filp, struct file *src_filp, loff_t *src_ppos, loff_t count);
extern struct dentry *cr_link(cr_errbuf_t *eb, struct path *old_path, const char *name);
extern struct file *cr_filp_reopen(const struct file *orig_filp, int new_flags);
extern int cr_fd_claim(int fd);
extern int cr_dup_other(struct files_struct *files, struct file *filp);
extern int cr_fstat(cr_objectmap_t, struct file *filp);

// cr_objects.c
extern int cr_object_init(void);
extern void cr_object_cleanup(void);
extern cr_objectmap_t cr_alloc_objectmap(void);
extern void cr_release_objectmap(cr_objectmap_t);
extern int cr_find_object(cr_objectmap_t, void *, void **);
extern int cr_insert_object(cr_objectmap_t, void *, void *, gfp_t flags);
extern int cr_remove_object(cr_objectmap_t, void *);

#ifdef CONFIG_COMPAT
// cr_compat.c
extern long cr_compat_ctrl_ioctl(struct file *, unsigned, unsigned long);
#endif

// cr_watchdog.c
extern void cr_wd_add(cr_work_t *work);
extern int cr_wd_del(cr_work_t *work);
#define __cr_wd_del(_work) list_del_init(&(_work)->list)
extern void cr_wd_flush(void);

// cr_relocate.c
extern const char *cr_relocate_path(cr_rstrt_relocate_t reloc, const char *path, int put_old);
extern void cr_free_reloc(cr_rstrt_relocate_t reloc);
extern int cr_read_reloc(cr_rstrt_req_t *req, /*struct cr_rstrt_relocate*/ void __user *arg);

// cr_creds.c
extern int cr_load_creds(cr_rstrt_proc_req_t *proc_req);
extern int cr_save_creds(cr_chkpt_proc_req_t *proc_req);

// 32-bit compat bits in various files:
#ifdef CONFIG_COMPAT
struct cr_compat_fwd_args {
	compat_pid_t	cr_target;
	compat_int_t	cr_scope;	/* enum */
};
extern int cr_chkpt_fwd32(struct file *filp, struct cr_compat_fwd_args __user *arg);

extern int cr_suspend32(struct file *filp, struct compat_timeval __user *arg);

struct cr_compat_chkpt_info {
	compat_pid_t	requester;
	compat_pid_t	target;
	compat_int_t	scope;
	compat_int_t	signal;
	compat_uptr_t	dest;
};
extern int cr_chkpt_info32(struct file *filp, struct cr_compat_chkpt_info __user *arg);

struct cr_compat_log_args {
	compat_uint_t	len;
	compat_uptr_t	buf;
};
extern int cr_chkpt_log32(struct file *filp, struct cr_compat_log_args __user *arg);
extern int cr_rstrt_log32(struct file *filp, struct cr_compat_log_args __user *arg);

struct cr_compat_procs_tbl {
	compat_int_t	threads;
	compat_int_t	clone_flags;
};
extern int cr_rstrt_procs32(struct file *filp, struct cr_compat_procs_tbl __user *arg);

struct cr_compat_chkpt_args {
	compat_pid_t	cr_target;
	compat_int_t	cr_scope;	/* enum */
	compat_int_t	cr_fd;
	compat_uint_t	cr_secs;
	compat_int_t	dump_format;	/* enum */
	compat_int_t	signal;
	compat_uint_t	flags;
};
extern int cr_chkpt_req32(struct file *file, struct cr_compat_chkpt_args __user *req);

struct cr_compat_rstrt_args {
	compat_int_t	cr_fd;
	compat_int_t	signal;
	compat_uptr_t	relocate;
	compat_int_t	flags;
};
extern int cr_rstrt_request_restart32(struct file *file, struct cr_compat_rstrt_args __user *req);

struct cr_compat_rstrt_relocate {
	compat_uint_t	count;
	struct {
		compat_uptr_t	oldpath;
		compat_uptr_t	newpath;
	} path[0]; // GNU (not ISO C99) variable length array
};
extern int cr_read_reloc32(cr_rstrt_req_t *req, /*struct cr_compat_rstrt_relocate*/ void __user *arg);
#endif // CONFIG_COMPAT

#endif
