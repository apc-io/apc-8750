/*
 * Berkeley Lab Checkpoint/Restart (BLCR) for Linux is Copyright (c)
 * 2003, The Regents of the University of California, through Lawrence
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
 * $Id: cr_context.h,v 1.72 2008/11/24 04:11:16 phargrov Exp $
 */

/*
 * Defines data structures stored in context files
 *
 * XXX:  Need a full specification of the file format one of these days
 */

#ifndef _CR_CONTEXT_H
#define _CR_CONTEXT_H        1

#define CR_MAGIC {'C', 'R'}

/*
 * XXX:  Think about putting an enum here to identify the structs as we
 * read them in (write them out).
 */

struct cr_context_file_header {
    int magic[2];
    int version;
    cr_scope_t scope;
    int arch_id;
};

struct cr_section_header {
    int num_threads;
    int clone_flags;
    int tmp_fd;
};

#define CR_PARENT_INIT	((struct task_struct *)-1)

struct cr_context_tasklinkage {
    struct task_struct *myptr;          /* Pointer to original task_struct */

    /* This is enough to restore all the linkage information for both 2.4 and 2.6
     * versions of the task_struct.  The remaining linkage can all be inferred
     * from these.  Though we don't restore the children in "age" order.
     */
    struct task_struct *real_parent;	/* real parent process (when being debugged) */
    struct task_struct *parent;		/* parent process */

    pid_t pid;
    pid_t pgrp;
#if 0
    /* We don't handle this, and it changed type on us around 2.6.21 */
    pid_t tty_old_pgrp;
#endif
    pid_t session;
    pid_t tgid;
    int leader;
    int exit_signal;

    struct {
        int		stopped;
	cr_phase_t	phase;
	int		fd;
	struct k_sigaction handler_sa;
	void *		orig_filp;
    	unsigned long	chkpt_flags;
    } cr_task;
};

/* Macros for reading linkage from a task
 * REAL_PARENT = My parent, not changed by ptrace
 * PARENT      = REAL_PARENT unless I am ptrace'd
 * CHILD       = Youngest child
 */
#if HAVE_TASK_REAL_PARENT
  #define CR_REAL_PARENT(T)	((T)->real_parent)
#else
  #undef CR_REAL_PARENT
#endif
#if HAVE_TASK_PARENT
  #define CR_PARENT(T)	((T)->parent)
#else
  #define CR_PARENT(T)	((T)->real_parent)
#endif
#define CR_CHILD(T)		(((T)->children.prev == &(T)->children) ? NULL : list_entry((T)->children.prev,struct task_struct,sibling))

#if HAVE_TASK_CHILD_REAPER
    #define cr_child_reaper task_child_reaper(current)
#elif HAVE_CHILD_REAPER
    #define cr_child_reaper child_reaper(current)
#elif defined(CR_KDATA_child_reaper)
    #define cr_child_reaper child_reaper
#else
    static inline struct task_struct *__cr_child_reaper(void) {
	struct pid_namespace *ns = task_active_pid_ns(current);
	return ns->child_reaper;
    }
    #define cr_child_reaper __cr_child_reaper()
#endif

/*
 * cr_type is one of these.
 */
typedef enum {
    cr_bad_obj,
    cr_fs_obj,
    cr_files_obj,
    cr_file_obj,
    cr_chrdev_obj,
    cr_dup_obj,
    cr_file_info_obj,
    cr_open_file_obj,
    cr_fifo_obj,
    cr_eofiles_obj,
    cr_dir_obj,
    cr_open_dir_obj,
} cr_obj_t;

struct cr_files_struct {
    cr_obj_t cr_obj;               /* cr_files_obj */
    int cr_max_fds;
    int cr_next_fd;
};

typedef enum {
    cr_bad_file_type=0,
    cr_open_file=1,
    cr_open_directory,
    cr_open_link,
    cr_open_fifo,
    cr_open_socket,
    cr_open_chr,
    cr_open_blk,
    cr_open_dup,
    cr_open_chkpt_req,
    cr_unknown_file=99,
    cr_end_of_files,
} cr_file_type_t;

/*
 * Stores "generic" data - applicable to any file.  (e.g. descriptor number,
 * close-on-exec flag, etc.)
 */
struct cr_file_info {
    cr_obj_t       cr_type;              /* cr_file_info_obj */
    cr_file_type_t cr_file_type;
    int		fd;
    int		cloexec;
    int		unlinked;
    void *	orig_filp;
};

struct cr_open_file {
    cr_obj_t cr_type; /* cr_open_file_obj */

    void *file_id;	/* pre-checkpoint inode for matching */
    mode_t i_mode;
    loff_t f_pos;
    loff_t i_size;
    unsigned int f_flags;
};

struct cr_open_dir {
    cr_obj_t cr_type; /* cr_open_dir_obj */

    mode_t i_mode;
    loff_t f_pos;
    unsigned int f_flags;
};

struct cr_chrdev {
    cr_obj_t cr_type; /* cr_open_chr */
    unsigned int cr_major;
    unsigned int cr_minor;
    mode_t i_mode;
    unsigned int f_flags;
};

struct cr_dup {
    cr_obj_t cr_type; /* cr_open_dup */
    /* nothing more */
};

struct cr_eofiles {
    cr_obj_t cr_type; /* cr_eofls_obj */
};

struct cr_fifo {
   cr_obj_t cr_type; /* cr_fifo_obj */
   void *fifo_id;         /* inode ptr at checkpoint time */
   int fifo_internal;	/* both reader and writer in the checkpoint? */
   unsigned int fifo_len;
   loff_t fifo_pos;
   unsigned long fifo_version;
   unsigned int fifo_flags;
   int fifo_perms; /* for mknod */
   void *fifo_dentry; /* f_dentry value at checkpoint time */
};

/*
 * Prototypes only needed from the save and restore paths
 */

// cr_vmadump.c
extern loff_t cr_freeze_threads(cr_chkpt_proc_req_t *proc_req, int flags,
				int i_am_leader);
extern long cr_thaw_threads(cr_rstrt_proc_req_t *proc_req, int flags,
                            int i_am_leader);

// cr_mmaps.c
extern long cr_save_mmaps_maps(cr_chkpt_proc_req_t *proc_req);
extern loff_t cr_save_mmaps_data(cr_chkpt_proc_req_t *proc_req);
extern long cr_load_mmaps_maps(cr_rstrt_proc_req_t *proc_req);
extern long cr_load_mmaps_data(cr_rstrt_proc_req_t *proc_req);

// cr_timers.c
extern void cr_pause_itimers(struct itimerval *itimers);
extern void cr_resume_itimers(struct itimerval *itimers);
extern long cr_load_itimers(cr_rstrt_proc_req_t *proc_req);
extern long cr_save_itimers(cr_chkpt_proc_req_t *proc_req);

// cr_pipes.c
extern int cr_restore_open_fifo(cr_rstrt_proc_req_t *proc_req, struct cr_file_info *file_info, int do_not_restore_flag);
extern int cr_save_open_fifo(cr_chkpt_proc_req_t *proc_req, struct file *filp);

#endif /* _CR_CONTEXT_H */
