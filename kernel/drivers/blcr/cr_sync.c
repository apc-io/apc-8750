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
 * $Id: cr_sync.c,v 1.16 2008/08/21 05:22:49 phargrov Exp $
 *
 *
 * This file deals with Phase2 threads/tasks.
 * The filename "cr_sync.c" is legacy from when these were
 * known as "synchronous threads/tasks".
 */

#include "cr_module.h"

// cr_phase2_release(filp, priv)
//
// Remove Phase2 tasks from the task list when the file is closed.
// Note that the close might be implictly at exit and could also
// come from a task other than the one originally registered.
//
// XXX: could keep a hash mapping phase2_filp -> cr_task rather than
// walking the entire list each time.
void cr_phase2_release(struct file *filp, cr_pdata_t *priv)
{
	cr_task_t *cr_task, *next;

	write_lock(&cr_task_lock);
	list_for_each_entry_safe(cr_task, next, &cr_task_list, task_list) {
		if ((cr_task->filp == filp) && (cr_task->phase == CR_PHASE2)) {
			cr_task->phase = CR_NO_PHASE;
			cr_task->filp = NULL;
			cr_task->fd = -1;
			__cr_task_put(cr_task);
		}
	}
	write_unlock(&cr_task_lock);
}

// cr_phase2_register(file, arg)
//
// Registers the calling task as having a Phase2 checkpoint handler.
// Returns:
// 	0 on success
// 	-EAGAIN if a checkpoint is pending for this task
// 	-EEXIST if this task is already registered
// 	other < 0 on other error conditions
int cr_phase2_register(struct file *filp, int arg)
{
	cr_task_t	*cr_task;
	int		retval;

	CR_KTRACE_FUNC_ENTRY();

	write_lock(&cr_task_lock);
	cr_task = __cr_task_get(current, 1);

	retval = -ENOMEM;
	if (!cr_task) {
		// Can't create the task list entry.
		goto out_noput;
	}

	retval = -EAGAIN;
	if (cr_task->chkpt_req || cr_task->rstrt_req) {
		// Can't register a Phase2 handler task
		// while a checkpoint or restart is in progress.
		goto out;
	}

	retval = -EEXIST;
	if (cr_task->phase == CR_PHASE2) {
		// Duplicate registration
		goto out;
	}

	retval = -EINVAL;
	if (cr_task->phase == CR_PHASE1) {
		// Conflicting registration
		goto out;
	}

	retval = 0;
	cr_task->phase = CR_PHASE2;
	cr_task->filp = filp;
	cr_task->fd = arg;
	atomic_inc(&cr_task->ref_count);

	// We assume our proper signal handler is registered at this time.
	// So, we save it for later use in case the user has overwitten it.
	CR_SIGNAL_LOCK(current);
	cr_task->handler_sa = CR_SIGACTION(current, CR_SIGNUM);
	CR_SIGNAL_UNLOCK(current);

out:
	__cr_task_put(cr_task);
out_noput:
	write_unlock(&cr_task_lock);

	return retval;
}
