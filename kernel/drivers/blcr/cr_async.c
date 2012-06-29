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
 * $Id: cr_async.c,v 1.31 2008/08/21 05:22:49 phargrov Exp $
 *
 *
 * This file deals with Phase1 threads/tasks.
 * The filename "cr_async.c" is legacy from when these were
 * known as "asynchronous threads/tasks".
 */

#include "cr_module.h"

#include <asm/uaccess.h>

// cr_phase1_release(filp, priv)
//
// Remove Phase1 tasks from the task list when the file is closed.
// Note that the close might be implictly at exit and could also
// come from a task other than the one originally registered.
//
// XXX: could keep a hash mapping phase1_filp -> cr_task rather than
// walking the entire list each time.
void cr_phase1_release(struct file *filp, cr_pdata_t *priv)
{
	cr_task_t *cr_task, *next;

	write_lock(&cr_task_lock);
	list_for_each_entry_safe(cr_task, next, &cr_task_list, task_list) {
		if ((cr_task->filp == filp) && (cr_task->phase == CR_PHASE1)) {
			cr_task->phase = CR_NO_PHASE;
			cr_task->filp = NULL;
			cr_task->fd = -1;
			__cr_task_put(cr_task);
		}
	}
	write_unlock(&cr_task_lock);
}

static
int do_suspend(struct file *filp, struct timeval *tvp)
{
	cr_task_t	*cr_task;
	unsigned long	timeout;
	int		retval = 0;
	DECLARE_WAIT_QUEUE_HEAD_ONSTACK(dummy);

	CR_KTRACE_FUNC_ENTRY();

	cr_task = cr_task_get(current);
	if (!cr_task) {
		retval = -EINVAL;
	        CR_KTRACE_FUNC_EXIT("due to -EINVAL");
		goto out;
	}

	// Decode the optional timeout
	timeout = tvp ? timeval_to_jiffies(tvp) : MAX_SCHEDULE_TIMEOUT;

	// Block until signal is pending or timeout
	// NOTE: Condition is checked only to ensure we don't sleep if already pending
	timeout = wait_event_interruptible_timeout(dummy, cr_task->chkpt_req, timeout);

	if (tvp) {
		jiffies_to_timeval(timeout, tvp);
	}

	if (!cr_task->chkpt_req && timeout) {
		// caught a signal other than the checkpoint trigger
		retval = -EINTR;
		CR_KTRACE_FUNC_EXIT("with pending signal");
	} else if (cr_task->chkpt_req) {
		CR_KTRACE_FUNC_EXIT("with pending checkpoint");
	} else if (!timeout) {
		CR_KTRACE_FUNC_EXIT("with expired timeout");
	} else {
		CR_KTRACE_FUNC_EXIT("for unknown reason");
	}

	cr_task_put(cr_task);
out:
	return retval;
}

// cr_suspend(timeval)
//
// Wait for a checkpoint request to arrive.
// Argument is a struct timeval for a bounded wait, or NULL to block forever.
//
// Returns:
//	<0 on errors (including interrupted sleep)
//	0 otherwise
//
int cr_suspend(struct file *filp, struct timeval __user *arg)
{
	struct timeval	timeval;
	int		retval;

	CR_KTRACE_FUNC_ENTRY();

	retval = -EFAULT;
	if (arg && copy_from_user(&timeval, arg, sizeof(timeval))) {
		goto out;
	} 

	retval = do_suspend(filp, arg ? &timeval : NULL);

	if (arg) {
		// like select() we fail silently if user timeval is R/O
		(void)put_user(timeval.tv_sec, &arg->tv_sec);
		(void)put_user(timeval.tv_usec, &arg->tv_usec);
	}

out:
	CR_KTRACE_FUNC_EXIT("Returning %d", retval);
	return retval;
}

// cr_phase1_register(file, arg)
//
// Registers the calling task as having a Phase1 checkpoint handler.
// Returns:
// 	0 on success
// 	-EAGAIN if a checkpoint is pending for this task
// 	-EEXIST if this task is already registered
// 	other < 0 on other error conditions
int cr_phase1_register(struct file *filp, int arg)
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
		// Can't register a Phase1 handler task
		// while a checkpoint or restart is in progress.
		goto out;
	}

	retval = -EEXIST;
	if (cr_task->phase == CR_PHASE1) {
		// Duplicate registration
		goto out;
	}

	retval = -EINVAL;
	if (cr_task->phase == CR_PHASE2) {
		// Conflicting registration
		goto out;
	}

	retval = 0;
	cr_task->phase = CR_PHASE1;
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

#ifdef CONFIG_COMPAT
int cr_suspend32(struct file *filp, struct compat_timeval __user *arg)
{
	struct timeval	timeval;
	int		retval;

	CR_KTRACE_FUNC_ENTRY();

	retval = -EFAULT;
        if (arg && (!access_ok(VERIFY_READ, arg, sizeof(*arg)) ||
		    __get_user(timeval.tv_sec, &arg->tv_sec) ||
		    __get_user(timeval.tv_usec, &arg->tv_usec))) {
		goto out;
	} 

	retval = do_suspend(filp, arg ? &timeval : NULL);

	if (arg) {
		// like select() we fail silently if user timeval is R/O
		(void)put_user(timeval.tv_sec, &arg->tv_sec);
		(void)put_user(timeval.tv_usec, &arg->tv_usec);
	}

out:
	CR_KTRACE_FUNC_EXIT("Returning %d", retval);
	return retval;
}
#endif // CONFIG_COMPAT
