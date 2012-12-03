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
 * $Id: cr_compat.c,v 1.29 2008/08/16 00:17:09 phargrov Exp $
 *
 * This file is for calls to 64-bit kernels from 32-bit apps.
 */

#include "cr_module.h"

#if defined(CONFIG_COMPAT) /* File empty otherwise */

#include <asm/uaccess.h>

/* Wrapper for 32bit callers */
long cr_compat_ctrl_ioctl(struct file *file, unsigned op, unsigned long arg)
{
	CR_KTRACE_FUNC_ENTRY("op=%08x arg=0x%lx", op, arg);

	switch(CR_IOCTL32_UNMAP(op)) {
	//
	// Calls from libcr:
	//
	case CR_OP_HAND_CHKPT:
		return cr_dump_self(file, arg);

	case CR_OP_HAND_ABORT:
		return cr_hand_abort(file, arg);

	case CR_OP_HAND_SUSP:
		return cr_suspend32(file, (struct compat_timeval __user *)compat_ptr(arg));

	case CR_OP_HAND_PHASE1:
		return cr_phase1_register(file, arg);

	case CR_OP_HAND_PHASE2:
		return cr_phase2_register(file, arg);

	case CR_OP_HAND_SRC:
		return cr_rstrt_src(file, (char __user *)compat_ptr(arg));

	case CR_OP_HAND_CHKPT_INFO:
		return cr_chkpt_info32(file, (struct cr_compat_chkpt_info __user *)compat_ptr(arg));

	case CR_OP_HAND_DONE:
		return cr_hand_complete(file, arg);

	//
	// Calls from cr_checkpoint:
	//
	case CR_OP_CHKPT_REQ:
		return cr_chkpt_req32(file, (struct cr_compat_chkpt_args __user *)compat_ptr(arg));

	case CR_OP_CHKPT_REAP:
		return cr_chkpt_reap(file);

	case CR_OP_CHKPT_FWD:
		return cr_chkpt_fwd32(file, (struct cr_compat_fwd_args __user *)compat_ptr(arg));

	case CR_OP_CHKPT_LOG:
		return cr_chkpt_log32(file, (struct cr_compat_log_args __user *)compat_ptr(arg));

	//
	// Calls from cr_restart:
	//
	case CR_OP_RSTRT_REQ:
		return cr_rstrt_request_restart32(file, (struct cr_compat_rstrt_args __user *)compat_ptr(arg));

	case CR_OP_RSTRT_REAP:
		return cr_rstrt_reap(file);

	case CR_OP_RSTRT_CHILD:
		return cr_rstrt_child(file);

	case CR_OP_RSTRT_PROCS:
		return cr_rstrt_procs32(file, (struct cr_compat_procs_tbl __user *)compat_ptr(arg));

	case CR_OP_RSTRT_LOG:
		return cr_rstrt_log32(file, (struct cr_compat_log_args __user *)compat_ptr(arg));

	//
	// General calls
	//
	case CR_OP_VERSION:
	    {
		// Simple enough to do here
		// User-space must have equal MAJOR and MINOR <= kernel
		// If we supported multiple major versions then this is
		// where we could do the "personallity switching".
		unsigned long major = (arg >> 16);
		unsigned long minor = (arg & 0xffff);
                // If major==0, must match minor; otherwise user-MINOR <= kernel-MINOR
                int match = (major == CR_MODULE_MAJOR) &&
                            (major ? (minor <= CR_MODULE_MINOR)
                                   : (minor == CR_MODULE_MINOR));
		if (!match) {
		    CR_WARN("request from pid %d for unsupported version %d.%d",
			    (int)current->pid, (int)major, (int)minor);
		}
		return match ? 0 : -CR_EVERSION;
	    }

	default:
		CR_KTRACE_BADPARM("unknown op %x", _IOC_NR(op));
		return -ENOTTY;
	}
}

#endif /* CONFIG_COMPAT */
