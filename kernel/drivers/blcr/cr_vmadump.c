/*-------------------------------------------------------------------------
 *  cr_vmadump.c:  Multithread wrapper for VMADump's freeze/thaw routines
 *
 *  2007, The Regents of the University of California, through Lawrence
 *  Berkeley National Laboratory (subject to receipt of any required
 *  approvals from the U.S. Dept. of Energy).  All rights reserved.
 *
 *  Portions may be copyrighted by others, as may be noted in specific
 *  copyright notices within specific files.
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
 * $Id: cr_vmadump.c,v 1.17 2008/12/01 00:14:52 phargrov Exp $
 *
 * This file handles thread-management for dumps of multithreaded processes.
 *-----------------------------------------------------------------------*/

#include "cr_module.h"

/* Let exactly one thread-group leader do a full undump, and
 * ensure everyone else does REGSONLY.
 * This and the matching logic at dump time together ensure that we
 * preserve the thread-group structure that was spawned in user-space.
 *
 * Returns original pid (>0), or <0 on error
 */
long
cr_thaw_threads(cr_rstrt_proc_req_t *proc_req, int flags, int i_am_leader)
{
    struct pt_regs *regs = get_pt_regs(current);
    long retval = 0;

    if (i_am_leader) {
        down(&proc_req->serial_mutex);
	retval = vmadump_thaw_proc(proc_req, proc_req->file, regs, flags);
	if (retval < 0) {
	    proc_req->thaw_error = retval;
	}
	proc_req->done_leader = 1;
	wake_up(&proc_req->wait);
	up(&proc_req->serial_mutex);
    } else if (wait_event_interruptible(proc_req->wait, proc_req->done_leader) < 0) {
	retval = -EINTR;
    } else if (proc_req->thaw_error) {
	retval = proc_req->thaw_error;
    } else {
        down(&proc_req->serial_mutex);
	retval = vmadump_thaw_proc(proc_req, proc_req->file, regs,
				   flags | VMAD_DUMP_REGSONLY);
	if (retval < 0) {
	    proc_req->thaw_error = retval;
	}
        up(&proc_req->serial_mutex);
    }

    return retval;
}

/* Let exactly one thread-group leader do a full dump, and
 * ensure everyone else does REGSONLY.
 * This and the matching logic at undump time together ensure that we
 * preserve the thread-group structure that was spawned in user-space.
 *
 * Returns bytes written (>0), or <0 on error
 */
loff_t
cr_freeze_threads(cr_chkpt_proc_req_t *proc_req, int flags, int i_am_leader)
{
    struct pt_regs *regs = get_pt_regs(current);
    loff_t retval = 0;

    if (i_am_leader) {
	down(&proc_req->serial_mutex);
	retval = vmadump_freeze_proc(proc_req, proc_req->file, regs, flags | VMAD_DUMP_NOSHANON);
	
	proc_req->done_leader = 1;
	wake_up(&proc_req->wait);
	up(&proc_req->serial_mutex);
    } else if (wait_event_interruptible(proc_req->wait, proc_req->done_leader) < 0) {
	retval = -EINTR;
    } else {
        down(&proc_req->serial_mutex);
	retval = vmadump_freeze_proc(proc_req, proc_req->file, regs,
				     flags | VMAD_DUMP_REGSONLY);
        up(&proc_req->serial_mutex);
    }

    return retval;
}
