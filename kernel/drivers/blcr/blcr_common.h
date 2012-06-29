/* 
 * Berkeley Lab Checkpoint/Restart (BLCR) for Linux is Copyright (c)
 * 2008, The Regents of the University of California, through Lawrence
 * Berkeley National Laboratory (subject to receipt of any required
 * approvals from the U.S. Dept. of Energy).  All rights reserved.
 *
 * Portions may be copyrighted by others, as may be noted in specific
 * copyright notices within specific files.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id: blcr_common.h.in,v 1.50.4.1 2009/02/14 01:50:12 phargrov Exp $
 *
 * This file contains definitions of BLCR types and constants that are
 * common to both user-space and kernel-space.
 */

#ifndef _BLCR_COMMON_H
#define _BLCR_COMMON_H	1

#ifndef __KERNEL__
#  include <features.h>
#  include <sys/ioctl.h>
#  include <sys/types.h>
   __BEGIN_DECLS
#else
#  include <linux/types.h>
#  include <linux/sched.h>
#  include <asm/ioctl.h>
#endif

#if defined(__GNUC__) && ((100 * __GNUC__ + __GNUC_MINOR__) >= 302)
  #define _CR_DEPRECATED  __attribute__((__deprecated__))
#else
  #define _CR_DEPRECATED /* empty */
#endif

// Get things specific to the current /proc implementation.
// This includes the actual entry points.
#include "blcr_proc.h"

// The signal number allocated for checkpoint:
#define CR_SIGNUM	64

/* version numbers */
#define CR_RELEASE_MAJOR 0
#define CR_RELEASE_MINOR 8
#define CR_RELEASE_PATCH 2
#define CR_RELEASE_VERSION "0.8.2"

#define LIBCR_MAJOR 0
#define LIBCR_MINOR 5
#define LIBCR_PATCH 2
#define LIBCR_VERSION "0.5.2"

#define CR_MODULE_MAJOR 0
#define CR_MODULE_MINOR 10
#define CR_MODULE_PATCH 1
#define CR_MODULE_VERSION "0.10.1"

/* CR-specific error codes:
 * ------------------------
 * i386 currently uses 1-125 for standard errno codes, and kernel-only ones
 * from 512-528.   Alpha/IA64 are similar.
 *
 * NOTE: You add new error codes by APPENDNG to blcr_errcodes.h.
 * Changing/reordering any of the existing error codes requires
 * incrementing the major version numbers of BOTH the kernel/libcr and
 * libcr/user interfaces.  Adding a new error requires incrementing the minor
 * version number of both.
 */
#define CR_ERROR_DEF(name, desc)    name,
enum {
    CR_MIN_ERRCODE = 2353,
#include "blcr_errcodes.h"
    CR_MAX_ERRCODE
};
#undef CR_ERROR_DEF

// basic boolean type for atomic bit operations
typedef long cr_bool_t;

// Types of context files to dump
typedef enum {
  cr_format_vmadump = 0,
  // cr_format_coredump, NO LONGER SUPPORTED
} cr_format_t;

//
// Definitions for a checkpoint request:
//

// Scope of a checkpoint request:
typedef enum {
	// CR_SCOPE_TASK,	single task NO LONGER SUPPORTED
	CR_SCOPE_PROC,	// All threads in this process
	CR_SCOPE_PGRP,	// All processes in the process group
	CR_SCOPE_SESS,	// All processes in the session
	CR_SCOPE_TREE,	// This process and all descendants
} cr_scope_t;

/* cr_checkpoint() callback flags */
enum {
    CR_CHECKPOINT_READY = 0,	    	/* checkpoint me */
    CR_CHECKPOINT_TEMP_FAILURE = 1,	/* cancel checkpoint and continue */
    CR_CHECKPOINT_PERM_FAILURE = 2,	/* cancel checkpoint, and kill me */
    CR_CHECKPOINT_OMIT = 4,		/* continue checkpoint, but w/o my process */
    /* Following are for internal use only: */
    _CR_CHECKPOINT_STUB = 0x4000,	/* caller is a "stub" handler - must fit in 15 bits */
};
#define CR_CHECKPOINT_TEMP_FAILURE_CODE(_v) (CR_CHECKPOINT_TEMP_FAILURE | ((_v)<<16))
#define CR_CHECKPOINT_PERM_FAILURE_CODE(_v) (CR_CHECKPOINT_PERM_FAILURE | ((_v)<<16))

#define CR_CHECKPOINT_ABORT_MASK (CR_CHECKPOINT_TEMP_FAILURE|CR_CHECKPOINT_PERM_FAILURE|CR_CHECKPOINT_OMIT)

// Setting cr_fd = CR_DEST_CWD sends context files to the cwd of the requester.
// Using (-1) could cause confusion with unchecked error returns from open().
#define CR_DEST_CWD	(-4096)

// The actual checkpoint request structure:
// XXX: If you make changes to this structure:
//  1) Modify cr_module.h to keep the "compat" version up-to-date
//  2) Be sure to advance CR_MODULE_* in configure.ac to avoid caller/callee mismatch
struct cr_chkpt_args {
	pid_t			cr_target;	// Who to checkpoint
	cr_scope_t		cr_scope;	// Scope of the checkpoint
	int			cr_fd;		// Destination of the context
	unsigned int		cr_secs;	// Seconds for target to
						// respond. 0 == unbounded
// XXX: Should one or more of the following fields move to opts?
	cr_format_t		dump_format;	// Format to use for this dump
	int			signal;		// Sent after checkpoint
	unsigned int		flags;		// See below...
};

// Structure to propagate a checkpoint to another process used by 
// cr_forward_checkpoint
// XXX: If you make changes to this structure:
//  1) Modify cr_module.h to keep the "compat" version up-to-date
//  2) Be sure to advance CR_MODULE_* in configure.ac to avoid caller/callee mismatch
struct cr_fwd_args {
	pid_t			cr_target;	// Who to checkpoint
	cr_scope_t		cr_scope;	// Scope of the checkpoint
};

//
// Flag values to OR together for 'flags' field of cr_chkpt_args
//
// CR_CHKPT_PROHIBIT_SELF
//	When given this flag, a checkpoint request will return failure (with
//	errno=EDEADLK) if the requestor's task would be included in the
//	checkpoint.  Note that in general checkpointing one's own task is
//	legal and only results in deadlock when critical sections and
//	blocking calls to CR_OP_CHKPT_DONE interact.
#define CR_CHKPT_PROHIBIT_SELF		0x00000001
// CR_CHKPT_PTRACED_*
//	Without either of these flags, requesting a checkpoint of a ptraced
//	process will fail with errno=CR_EPTRACED.
//	CR_CHKPT_PTRACED_ALLOW
//	    When given this flag, ptraced processes will be treated as any
//	    other process would.  Since BLCR interacts with the processes
//	    to be checkpointed using signals and sysem calls, the debugger
//	    (or other ptracer) must "continue" the process until the
//	    checkpoint has been taken.
//	CR_CHKPT_PTRACED_SKIP
//	    When given this flag, ptraced processes are silently excluded
//	    from the checkpoint request (and their children if the scope
//	    is CR_SCOPE_TREE).  An error will result ONLY if this leads to
//	    zero processes checkpointed.
//	NOTE: These flags are mutually exclusive.
#define CR_CHKPT_PTRACED_ALLOW		0x00000002
#define CR_CHKPT_PTRACED_SKIP		0x00000004
// CR_CHKPT_PTRACER_SKIP
//	Without these this flag, requesting a checkpoint of a process which
//	is ptracing others will fail with errno=CR_EPTRACER.
//	When given this flag, such processes are silently excluded from the
//	checkpoint request (and their children if the scope is CR_SCOPE_TREE).
//	An error will result ONLY if this leads to zero processes
//	checkpointed.
#define CR_CHKPT_PTRACER_SKIP		0x00000008
// CR_CHKPT_ASYNC_ERR
//	When this flag is passed most errors that would otherwise be reported
//	at request time are instead reported at CR_OP_CHKPT_REAP.
#define CR_CHKPT_ASYNC_ERR		0x00000010
// CR_CHKPT_DUMP_*
//	Request dump of optional portions of memory:
//	    CR_CHKPT_DUMP_EXEC      dump the executable
//	    CR_CHKPT_DUMP_PRIVATE   dump private mapped files
//	    CR_CHKPT_DUMP_SHARED    dump shared mapped files
//	    CR_CHKPT_DUMP_ALL       dump all optional portions of memory
#define CR_CHKPT_DUMP_EXEC      0x0200  /* BLCR will dump the executable  */
#define CR_CHKPT_DUMP_PRIVATE   0x0400  /* BLCR will dump private file maps  */
#define CR_CHKPT_DUMP_SHARED    0x0800  /* BLCR will dump shared file maps  */
#define CR_CHKPT_DUMP_ALL       (CR_CHKPT_DUMP_EXEC|CR_CHKPT_DUMP_PRIVATE|CR_CHKPT_DUMP_SHARED)

//
// Definitions for a restart request:
//

// path pairs for restart-time relocations
// XXX: If you make changes to this structure:
//  1) Modify cr_module.h to keep the "compat" version up-to-date
//  2) Be sure to advance CR_MODULE_* in configure.ac to avoid caller/callee mismatch
struct cr_rstrt_relocate_pair {
	const char *oldpath;
	const char *newpath;
};
struct cr_rstrt_relocate {
	unsigned int    count;
	struct cr_rstrt_relocate_pair
			path[0]; // GNU (not ISO C99) variable length array
};
// Bytes to allocate for struct cr_rstrt_relocate with _cnt path entries:
#define CR_RSTRT_RELOCATE_SIZE(_cnt) (sizeof(struct cr_rstrt_relocate) + \
				      (_cnt) * sizeof(struct cr_rstrt_relocate_pair))
// Maximum number of path entries supported in struct cr_rstrt_relocate
#define CR_MAX_RSTRT_RELOC	16

// The actual restart request structure:
// XXX: If you make changes to this structure:
//  1) Modify cr_module.h to keep the "compat" version up-to-date
//  2) Be sure to advance CR_MODULE_* in configure.ac to avoid caller/callee mismatch
struct cr_rstrt_args {
	int			cr_fd;		// Source of the context
// XXX: Should one or more of the following fields move to opts?
	int			signal;		// delivered just before callbacks run
	struct cr_rstrt_relocate *relocate;	// mapping of paths to new locations
	unsigned int		flags;		// See below...
};

//
// Flag values to OR together for 'flags' field of cr_rstrt_args
//
//
// CR_RSTRT_ASYNC_ERR
//	When this flag is passed most errors that would otherwise be reported
//	at request time are instead reported at CR_OP_RSTRT_REAP.
#define CR_RSTRT_ASYNC_ERR		0x00000001
// CR_RSTRT_RESTORE_PID
//      When this flag is passes, the pid and tgid of tasks are restored.
#define CR_RSTRT_RESTORE_PID		0x00000002
// CR_RSTRT_RESTORE_PGID
//      When this flag is passes, the process group ids of task are restored.
#define CR_RSTRT_RESTORE_PGID		0x00000004
// CR_RSTRT_RESTORE_SID
//      When this flag is passes, the session ids of task are restored.
#define CR_RSTRT_RESTORE_SID		0x00000008

// Structure for returning processes to spawn
// XXX: If you make changes to this structure:
//  1) Modify cr_module.h to keep the "compat" version up-to-date
//  2) Be sure to advance CR_MODULE_* in configure.ac to avoid caller/callee mismatch
struct cr_procs_tbl {
	int	threads;
	int	clone_flags;
};

// Structure for returning checkpoint info */
// XXX: If you make changes to this structure:
//  1) Modify cr_module.h to keep the "compat" version up-to-date
//  2) Be sure to advance CR_MODULE_* in configure.ac to avoid caller/callee mismatch
//  The 'dest' field is a buffer of len >= PATH_MAX, and the result is a string:
//	Empty if the destination is not in the file system
//	Full path if destination is a file, device, fifo, etc.
//	Path ending in "/." if destination is a directory.
struct cr_chkpt_info {
	pid_t	requester;
	pid_t	target;
	int	scope;
	int	signal;
	char	*dest;
};

// Structure for requesting error/warning log
// XXX: If you make changes to this structure:
//  1) Modify cr_module.h to keep the "compat" version up-to-date
//  2) Be sure to advance CR_MODULE_* in configure.ac to avoid caller/callee mismatch
struct cr_log_args {
	unsigned int	len;
	char 		*buf;
};

// Flags to OP_HAND_DONE and to cr_hold_ctrl()
#define CR_HOLD_READ -1
#define CR_HOLD_NONE  0
#define CR_HOLD_CONT  1
#define CR_HOLD_RSTRT 2
#define CR_HOLD_BOTH  (CR_HOLD_CONT|CR_HOLD_RSTRT)
#define CR_HOLD_DFLT  4

#ifndef __KERNEL__
  __END_DECLS
#endif

#endif
