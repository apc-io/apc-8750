/* 
 * Berkeley Lab Checkpoint/Restart (BLCR) for Linux is Copyright (c)
 * 2003, The Regents of the University of California, through Lawrence
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
 * $Id: blcr_ioctl.h,v 1.38 2008/08/16 00:17:11 phargrov Exp $
 */

#ifndef _CR_IOCTL_H
#define _CR_IOCTL_H	1

#define CR_IOCTL_BASE	0xA1


//
// ioctl()s for libcr/callback code:
//   

//  CR_OP_HAND_CHKPT(unsigned long flags)
//	Called from libcr when the callbacks have run their CHECKPOINT
//      code and the thread is prepared for the kernel to save it.
#define CR_OP_HAND_CHKPT	_IOW (CR_IOCTL_BASE, 0x01, unsigned long)

//  CR_OP_HAND_ABORT(unsigned long flags)
//      Called from libcr when a callback wishes to abort the checkpoint/restart.
//      If flags is set to CR_CHECKPOINT_PERM_FAILURE, all processes in the
//      checkpoint are killed.
#define CR_OP_HAND_ABORT	_IOW (CR_IOCTL_BASE, 0x02, unsigned long)

// 0x03 was CR_OP_HAND_CONT

// 0x04 was CR_OP_HAND_RSTRT

//  CR_OP_HAND_PHASE1(long fd)
//	This is how a Phase1 thread/task announces itself to the kernel.
#define CR_OP_HAND_PHASE1	_IOW  (CR_IOCTL_BASE, 0x05, long)

//  CR_OP_HAND_SUSP(struct timeval * timeout)
//	Block in the kernel until one of the following three things happens:
//	1) A checkpoint request arrives for this thread.
//	2) A signal arrives for this thread.
//	3) The timeout expires.
//	The timeout argument works exactly as in select(2).
#define CR_OP_HAND_SUSP		_IOWR(CR_IOCTL_BASE, 0x06, struct timeval *)

//  CR_OP_HAND_PHASE2(long fd)
//	This is how a Phase2 thread/task announces itself to the kernel.
#define CR_OP_HAND_PHASE2	_IOW  (CR_IOCTL_BASE, 0x07, long)

//  CR_OP_HAND_DONE(unsigned long flags)
//	Called from libcr when the callbacks have run to completion
//      and the thread is just about to resume execution
//	(returning from the signal handler or from cr_leave_cs).
//	This is the chance for the kernel to SIGSTOP us if we are to
//	be placed in a STOPPED state.
#define CR_OP_HAND_DONE		_IOW (CR_IOCTL_BASE, 0x08, unsigned long)

//  CR_OP_HAND_SRC(char *buffer)
//	This is how libcr queries the restart source.
//	The argument is a buffer of len >= PATH_MAX
//	The result is a string:
//        Empty if the source is not in the file system
//        Full path if source is a file, device, fifo, etc.
//	  Path ending in "/." if source is a directory.
#define CR_OP_HAND_SRC	_IOR  (CR_IOCTL_BASE, 0x09, char *)

//  CR_OP_HAND_CHKPT_INFO(struct cr_chkpt_info *)
//	This is how cr_get_checkpoint_info() is implemented.
#define CR_OP_HAND_CHKPT_INFO	_IOR  (CR_IOCTL_BASE, 0x0a, struct cr_chkpt_info *)

//
// ioctl()s for cr_checkpoint:
//   

// CR_OP_CHKPT_REQ(struct cr_chkpt_args *args)
//	Called to request a checkpoint of a task/proc/pgrp/session.
#define CR_OP_CHKPT_REQ	_IOW (CR_IOCTL_BASE, 0x10, struct cr_chkpt_args *)

// 0x11 was CR_OP_CHKPT_DONE

// CR_OP_CHKPT_REAP(void)
//	Called to reap a completed checkpoint.
//	This is necessary if you wish to issue a new request on the same
//	file descriptor, and is recommended as a polite way to cleanup.
//	However, closing the fd (or exiting) will do this implicitly.
#define CR_OP_CHKPT_REAP	_IO  (CR_IOCTL_BASE, 0x12)

// CR_OP_CHKPT_FWD(struct cr_fwd_args *args)
//      Called to propagate a checkpoint request to another process.
#define CR_OP_CHKPT_FWD _IOW (CR_IOCTL_BASE, 0x13, struct cr_fwd_args *)

// CR_OP_CHKPT_LOG(struct cr_log_args *args)
//	Called to collect error/warning log from a completed checkpoint.
//	arg.buf is the space allocated for the result
//	arg.len is the length of buf
//	Return value is value of arg.len required to receive full log.
//	arg.buf is always nul-terminated if return >= 0.
#define CR_OP_CHKPT_LOG	_IOR  (CR_IOCTL_BASE, 0x14, struct cr_log_args *)

//
// ioctl()s for cr_restart:
//   

// CR_OP_RSTRT_REQ(struct cr_rstrt_args *args)
//	Called to request a restart from a given file descriptor.
#define CR_OP_RSTRT_REQ		_IOW (CR_IOCTL_BASE, 0x20,\
				      struct cr_rstrt_args *)

// 0x21 was CR_OP_RSTRT_DONE

// CR_OP_RSTRT_REAP(void)
//	Called to reap a completed restart.
//	This is necessary if you wish to issue a new request on the same
//	file descriptor, and is recommended as a polite way to cleanup.
//	However, closing the fd (or exiting) will do this implicitly.
#define CR_OP_RSTRT_REAP	_IO  (CR_IOCTL_BASE, 0x22)

// CR_OP_RSTRT_CHILD(void)
//      Called to overlay a process with a restart image.
//      Acts like exec().  Will not return unless there has been an error.
#define CR_OP_RSTRT_CHILD	_IO  (CR_IOCTL_BASE, 0x23)

// 0x24 : was OP_RSTRT_CLONES
// 0x25 : was OP_RSTRT_FORKS

// CR_OP_RSTRT_PROCS(struct cr_procs_tbl **)
//      Called to determine if more processes are needed.
//	Returns 0 if done, positive if not, or negative on error.
//	Stores info on needed procs to supplied address.
#define CR_OP_RSTRT_PROCS	_IOR (CR_IOCTL_BASE, 0x26,\
				      struct cr_procs_tbl *)

// CR_OP_RSTRT_LOG(struct cr_log_args *args)
//	Called to collect error/warning log from a completed restart.
//	arg.buf is the space allocated for the result
//	arg.len is the length of buf
//	Return value is value of arg.len required to receive full log.
//	arg.buf is always nul-terminated if return >= 0.
#define CR_OP_RSTRT_LOG	_IOR  (CR_IOCTL_BASE, 0x27, struct cr_log_args *)

//
// ioctl()s for general use:
//   

// CR_OP_VERSION(unsigned long version)
//	Called to verify the kernel supports the specified version
//	where version = (CR_MODULE_MAJOR << 16) | CR_MODULE_MINOR
//      Return of 0 is success, meaning we support that version.
#define CR_OP_VERSION		_IOW  (CR_IOCTL_BASE, 0x30, unsigned long)


#endif
