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
 * $Id: cr_trigger.c,v 1.39 2008/12/05 23:15:19 phargrov Exp $
 */

#include "cr_module.h"

//
// Private functions
//

// do_trigger(cr_task, proc_req)
//
// Sends a trigger (currently just signal 63 or 64) to the given task.
// If the task is STOPPED, we also wake it so it can receive the
// signal.
// Must be called with tasklist_lock held for reading to prevent task from exiting.
//
// XXX: Should (probably) never fail, but could return -EAGAIN if the
// kernel can't queue anymore RT signals at this time.
// XXX: When we finally move away from the signal, the trigger will
// need to invoke the cr_task->handler, or the default trigger if NULL.
static int
do_trigger(cr_task_t *cr_task, cr_chkpt_proc_req_t *proc_req)
{
	struct task_struct *task = cr_task->task;
	struct siginfo info;
	int retval = 0;

	if (cri_task_dead(task)) {
	    CR_KTRACE_LOW_LVL("not triggering dead task pid %d (%s)", task->pid, task->comm);
	    goto out;
	}

	retval = proc_req->ctrl_fd;
	if (retval < 0) {
		retval = proc_req->tmp_fd;
		if (retval < 0) {
			/* Force an instance of the ctrl fd into the target's file table
			 * (exactly once per process regardless of thread count).
			 * XXX: Ideally we'd be smarter about this:
			 *   STOP other tasks before doing this to avoid racing against
			 *   anything they might be doing with their own files.
			 */
			retval = cr_dup_other(task->files, proc_req->req->ctrl_file);
			if (retval < 0) goto out;
			proc_req->tmp_fd = retval;
		}
	}
	CR_ASSERT_STEP_EQ(cr_task, 0);

	memset(&info, 0, sizeof(info));	/* security */

	info.si_signo = CR_SIGNUM;
	info.si_errno = 0;
	info.si_code = SI_KERNEL;
	info.si_pid = retval;

	/* Note that we don't recalc_sigpending.  It is not needed, because
	   bad things would have happened if CR_SIGNUM was already pending. */
	CR_SIGNAL_LOCK(task);
	if ((proc_req->saved_sa.sa.sa_handler == SIG_ERR) &&
	    (proc_req->forced_sa.sa.sa_handler != SIG_ERR)) {
	    // Force our saved handler (if any), in case user has overwritten.
	    // Yes, there is a very small race before we actually send the signal.
	    proc_req->saved_sa = CR_SIGACTION(task, CR_SIGNUM);
	    CR_SIGACTION(task, CR_SIGNUM) = proc_req->forced_sa;
	}
	sigdelset(&task->blocked, CR_SIGNUM);
	CR_SIGNAL_UNLOCK(task);

	retval = send_sig_info(CR_SIGNUM, &info, task);
	CR_KTRACE_LOW_LVL("triggered pid %d (%s) w/ retval=%d", task->pid, task->comm, retval);

	/* Save the "stoppedness" of the task for later restore */
	cr_task->stopped = (task->state == TASK_STOPPED);
	if (!retval && (cr_task->stopped)) {
		wake_up_process(task);
	}
	cr_task->self_exec_id = task->self_exec_id;
out:
	return retval;
}

//
// __cr_trigger_phase1_only(req)
//
// Sends a trigger for phase1:
//    All Phase1 tasks (if any) are sent the trigger.
//    The caller must ensure that Phase2 triggers are sent
// Must hold tasklist_lock for reading
//
int __cr_trigger_phase1_only(cr_chkpt_proc_req_t *proc_req)
{
        cr_task_t *cr_task;
        int found = 0;

        list_for_each_entry(cr_task, &proc_req->tasks, proc_req_list) {
                if (cr_task->phase == CR_PHASE1) {
                        found = 1;
                        do_trigger(cr_task, proc_req);
                        // XXX: check error conditions
                }
        }

        return found;
}

//
// __cr_trigger_phase1(proc_req)
//
// Sends a trigger for phase1:
//    All Phase1 tasks (if any) are sent the trigger.
//    If no Phase1 tasks exist then skip to Phase2.
// Must hold req->lock, or otherwise ensure exclusive access to req->tasks
// Must hold tasklist_lock for reading
//
// XXX: Should (probably) never fail, thus currently always returns 0.
//
int __cr_trigger_phase1(cr_chkpt_proc_req_t *proc_req)
{
        int found;

        found = __cr_trigger_phase1_only(proc_req);

        // If no Phase1 tasks were found in this proc, then skip to Phase2
        if (!found) {
                __cr_trigger_phase2(proc_req);
        }

        return 0;
}

//
// cr_trigger_phase1(req)
//
// Sends a trigger for phase1:
//    All Phase1 tasks (if any) are sent the trigger.
//    If no Phase1 tasks exist then skip to Phase2.
//
// XXX: Should (probably) never fail, thus currently always returns 0.
//
int cr_trigger_phase1(cr_chkpt_req_t *req)
{
        cr_chkpt_proc_req_t *proc_req;
        int retval = 0;

        read_lock(&req->lock);
        read_lock(&tasklist_lock);

        // Send trigger to all of the Phase1 tasks on the list
        list_for_each_entry(proc_req, &req->procs, list) {
                retval = __cr_trigger_phase1(proc_req);
                if (retval) break;
        }

        read_unlock(&tasklist_lock);
        read_unlock(&req->lock);

        return retval;
}

//
// __cr_trigger_phase2(proc_req)
//
// Sends the trigger the Phase2 (ie normal/application) tasks.
// Must hold req->lock, or otherwise ensure exclusive access to req->tasks
// Must hold tasklist_lock for reading
//
// XXX: Should (probably) never fail, thus currently always returns 0.
//
int __cr_trigger_phase2(cr_chkpt_proc_req_t *proc_req)
{
        cr_task_t *cr_task;

        // Send trigger to all of the Phase2 (and "undeclared") tasks.
        list_for_each_entry(cr_task, &proc_req->tasks, proc_req_list) {
                if (cr_task->phase != CR_PHASE1) {
                        do_trigger(cr_task, proc_req);
                        // XXX: check error conditions
                }
        }

        return 0;
}

//
// cr_trigger_phase2(req, proc_req)
//
// Sends the trigger the Phase2 (ie normal/application) tasks.
//
// XXX: Should (probably) never fail, thus currently always returns 0.
//
int cr_trigger_phase2(cr_chkpt_req_t *req, cr_chkpt_proc_req_t *proc_req)
{
        int retval;

        read_lock(&req->lock);
        read_lock(&tasklist_lock);
        retval = __cr_trigger_phase2(proc_req);
        read_unlock(&tasklist_lock);
        read_unlock(&req->lock);

        return retval;
}
