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
 * $Id: cr_chkpt_req.c,v 1.264.6.2 2009/03/06 19:36:11 phargrov Exp $
 */

#include "cr_module.h"

#include <linux/time.h>
#include <asm/uaccess.h>

// release_request()
//
// Release a request structure, free if reference count drops to zero.
//
// Returns non-zero if the request has actually been freed.
//
// No locks needed
static int release_request(cr_chkpt_req_t *req)
{
	int result;

	CRI_ASSERT(atomic_read(&req->ref_count));
	result = atomic_dec_and_test(&req->ref_count);
	if (result) {
		CRI_ASSERT(list_empty(&req->procs));
		cr_loc_free(&req->dest);
		cr_release_objectmap(req->map);
		fput(req->ctrl_file);
		cr_errbuf_free(req->errbuf);
		kmem_cache_free(cr_chkpt_req_cachep, req);
		CR_MODULE_PUT();
		CR_KTRACE_ALLOC("Free cr_chkpt_req_t %p", req);
	}

	return result;
}

// release_proc_req()
//
// Try to free a proc_req and associated resources
static void release_proc_req(cr_chkpt_proc_req_t *proc_req)
{
	proc_req->ref_count -= 1;

	if (proc_req->ref_count == 0) {
		CRI_ASSERT(list_empty(&proc_req->tasks));
		list_del_init(&proc_req->list);
		if (proc_req->mmaps_tbl) {
			vfree(proc_req->mmaps_tbl);
		}
#if CRI_DEBUG
		if (proc_req->tmp_fd >= 0) {
	    		CR_ERR("Leaking tmp_fd");
		}
#endif
		kmem_cache_free(cr_chkpt_proc_req_cachep, proc_req);
	}
}

// __delete_from_req(req, cr_task)
//
// Remove a given task from the request list and try to free resources.
//
// Must be called w/ cr_task_lock held for writing.
// Must be called w/ req->lock held for writing or when exclusive
// access to the request list is otherwise guaranteed.
static void __delete_from_req(cr_chkpt_req_t *req, cr_task_t *cr_task)
{
	list_del_init(&cr_task->req_list);
	list_del_init(&cr_task->proc_req_list);
	if (list_empty(&req->tasks)) {
		wake_up(&req->wait);
	}
	release_proc_req(cr_task->chkpt_proc_req);
	cr_task->chkpt_req = NULL;
	cr_task->chkpt_proc_req = NULL;
	cr_task->step = 0;
	__cr_task_put(cr_task);
}

// delete_from_req(req, cr_task)
//
// Remove a given task from the request list and try to free resources.
//
// Call w/o holding locks
static void delete_from_req(cr_chkpt_req_t *req, cr_task_t *cr_task)
{
	write_lock(&cr_task_lock);
	write_lock(&req->lock);
	__delete_from_req(req, cr_task);
	write_unlock(&req->lock);
	write_unlock(&cr_task_lock);
}

// delete_dead_task(req, cr_task)
//
// Remove a dead (or dying) task from the request
//
// Must be called w/ cr_task_lock held for writing.
// Must be called w/ req->lock held for writing or when exclusive
// access to the request list is otherwise guaranteed.
//
// We don't (yet) raise any error flags here because it is legal
// for us to lose a race against an exiting process/thread.
// The reap path will catch the case in which ALL tasks died.
static void delete_dead_task(cr_chkpt_req_t *req, cr_task_t *cr_task)
{
	cr_chkpt_advance_to(cr_task, CR_CHKPT_STEP_DONE, 1);
	__delete_from_req(req, cr_task);
	(void)release_request(req);
}

// check_done(req)
// 
// Returns non-zero if the request can be reaped.
//
// Call w/o holding the locks
static int check_done(cr_chkpt_req_t *req)
{
	int retval;

	write_lock(&req->lock);
	retval = list_empty(&req->tasks);
	write_unlock(&req->lock);

	// Try to remove from watchdog list if done
	// This is only an optimization, not a correctness requirement
	if (retval && cr_wd_del(&req->work)) {
	    // Release watchdog's ref_count.
	    // The current task (the requester) must hold an additional reference.
	    CRI_ASSERT(atomic_read(&req->ref_count) >= 2);
	    (void)release_request(req);
	}

	return retval;
}

// chkpt_watchdog(req)
//
// Called periodically to cleanup after zombies and to enforce
// the time limit on a checkpoint request.
//
// XXX: no time limit yet
static void chkpt_watchdog(cr_work_t *work)
{
	cr_chkpt_req_t *req = container_of(work, cr_chkpt_req_t, work);
	cr_task_t *cr_task, *next;
	int empty;

	write_lock(&cr_task_lock);
	write_lock(&req->lock);

	list_for_each_entry_safe(cr_task, next, &req->tasks, req_list) {
		struct task_struct *task = cr_task->task;
		if (task->self_exec_id != cr_task->self_exec_id) {
			CR_WARN_PROC_REQ(cr_task->rstrt_proc_req,
				"%s: tgid/pid %d/%d exec()ed '%s' during checkpoint",
				__FUNCTION__, task->tgid, task->pid, task->comm);
			// fall through to delete_dead_task()
		} else if (cri_task_dead(task)) {
			int signo = task->exit_code & 0x7f;
			CR_WARN_PROC_REQ(cr_task->chkpt_proc_req,
				"%s: '%s' (tgid/pid %d/%d) exited with "
				"%s %d during checkpoint",
				__FUNCTION__, task->comm,
			       	task->tgid, task->pid,
				signo ? "signal" : "code",
				signo ? signo : (task->exit_code & 0xff00) >> 8);
			// Raise an error if we killed it
			// XXX: this is now unlikely, given that we check
			// for the signal handler at request time.  Remove?
			if (signo == CR_SIGNUM) {
				req->result = -CR_ENOSUPPORT;
			}
			// fall through to delete_dead_task()
		} else {
#if CRI_DEBUG
			CR_SIGNAL_LOCK(task);
			if (sigismember(&task->blocked, CR_SIGNUM) && 
			    sigismember(&task->pending.signal, CR_SIGNUM)) {
				CR_ERR("Task %d (%s) has CR_SIGNUM blocked",
					task->pid, task->comm);
			}
			// XXX: Need 1-arg recalc_sigpending() or equivalent
			//sigdelset(&task->blocked, CR_SIGNUM);
			//cr_recalc_sigpending_tsk(task)
			CR_SIGNAL_UNLOCK(task);
#endif
			continue; // DO NOT fall through to delete_dead_task
		}

		// Note that the watchdog and each target task
		// hold a reference, so this delete_dead_task()
		// can't destroy req.
		CRI_ASSERT(atomic_read(&req->ref_count) >= 2);
		delete_dead_task(req, cr_task);
	}
	empty = list_empty(&req->tasks);

	if (!empty && req->has_expiration && time_after_eq(jiffies, req->expiration)) {
		CR_INFO("%s: timeout detected", __FUNCTION__);
		// XXX: handle timeout case here
#if 1 // XXX: temporary alternative
		req->has_expiration = 0;
#endif
	}

	write_unlock(&req->lock);
	write_unlock(&cr_task_lock);

	if (empty) {
		__cr_wd_del(work);
		release_request(req);
	}
}

// alloc_request()
//
// Allocate and initialize a request structure
//
// No locks needed
static cr_chkpt_req_t * alloc_request(struct file *ctrl_file)
{
	static struct lock_class_key lock_key;
	cr_chkpt_req_t *req = NULL;
	cr_objectmap_t map;

	if (!CR_MODULE_GET()) {
		CR_ERR("Checkpoint request after rmmod!");
		req = ERR_PTR(-EINVAL);
		goto out_rmmod;
	}

        map = cr_alloc_objectmap();
	if (!map) {
                goto out_modput;
	}

	CR_NO_LOCKS();
	req = cr_kmem_cache_zalloc(*req, cr_chkpt_req_cachep, GFP_KERNEL);
	if (req) {
		CR_KTRACE_ALLOC("Alloc cr_chkpt_req_t %p", req);
		atomic_set(&req->ref_count, 1);
		atomic_set(&req->completed, 0);
		INIT_LIST_HEAD(&req->tasks);
		INIT_LIST_HEAD(&req->procs);
		init_waitqueue_head(&req->wait);
		req->requester = current->tgid;
		rwlock_init(&req->lock);
		lockdep_set_class(&req->lock, &lock_key);
		req->result = 0;
		req->has_expiration = 0;
		req->map = map;
		req->die = 0;
	        init_MUTEX(&req->serial_mutex);
		CR_INIT_WORK(&req->work, &chkpt_watchdog);
		cr_barrier_init(&req->preshared_barrier, 0);
		cr_barrier_init(&req->postdump_barrier, 0);
		req->ctrl_file = cr_filp_reopen(ctrl_file, O_WRONLY);
		req->errbuf = cr_errbuf_alloc();
	} else {
                goto out_freemap;
	}

	return req;

out_freemap:
        cr_release_objectmap(map);
out_modput:
	CR_MODULE_PUT();
out_rmmod:
	return req;
}

#define  CR_PTRACED_FLAG_MASK ( CR_CHKPT_PTRACED_ALLOW  | \
				CR_CHKPT_PTRACED_SKIP)
#define  CR_PTRACER_FLAG_MASK	CR_CHKPT_PTRACER_SKIP

static int cr_is_ptrace_child(struct task_struct *task)
{
#if HAVE_TASK_PTRACE
	return (task->ptrace & PT_PTRACED);
#elif HAVE_TASK_PTRACEES
	struct utrace_attached_engine *engine;
	rcu_read_lock();
	engine = utrace_attach(task, UTRACE_ATTACH_MATCH_OPS, &ptrace_utrace_ops, 0);
	rcu_read_unlock();
	return (!IS_ERR(engine));
#else
	return 0; // XXX: No ptrace support!?   Should this be a build-time error?
#endif
}

static int cr_is_ptrace_parent(struct task_struct *task)
{
#if HAVE_TASK_PTRACEES
	return !list_empty(&task->ptracees);
#elif HAVE_TASK_PTRACED
	return !list_empty(&task->ptraced);
#elif HAVE_TASK_PTRACE
	struct task_struct *child;
	CR_DO_EACH_CHILD(child, task) {
		if (child->ptrace & PT_PTRACED) {
			return 1;
		}
	} CR_WHILE_EACH_CHILD(child, task);
	return 0;
#else
	return 0; // XXX: No ptrace support!?   Should this be a build-time error?
#endif
}

// "lookup" a proc (thread group) by task->mm
// req->lock required unless otherwise serialized
// alloction is GFP_ATOMIC because tasklist lock is normally held
static cr_chkpt_proc_req_t *
lookup_proc_req(cr_chkpt_req_t *req, struct task_struct *task)
{
	cr_chkpt_proc_req_t *result = NULL;
	cr_chkpt_proc_req_t *tmp;
	const struct mm_struct *mm = task->mm;

	list_for_each_entry(tmp, &req->procs, list) {
	    if (tmp->mm == mm) {
		result = tmp;
		break;
	    }
	}
	if (!result) {
	    result = cr_kmem_cache_zalloc(*result, cr_chkpt_proc_req_cachep, GFP_ATOMIC);
	    // XXX: should handle NULL!
	    result->mm = mm;
	    result->req = req;
	    INIT_LIST_HEAD(&result->tasks);
	    cr_barrier_init(&result->phase_barrier, 0);
	    cr_barrier_init(&result->predump_barrier, 0);
	    cr_barrier_init(&result->vmadump_barrier, 0);
	    cr_barrier_init(&result->pre_complete_barrier, 0);
	    cr_barrier_init(&result->post_complete_barrier, 0);
	    init_MUTEX(&result->serial_mutex);
	    list_add_tail(&result->list, &req->procs);
	    init_waitqueue_head(&result->wait);
	    result->saved_sa.sa.sa_handler = SIG_ERR;
	    result->forced_sa.sa.sa_handler = SIG_ERR;
	    result->ctrl_fd = -1;
	    result->tmp_fd = -1;
	}

	result->ref_count++;
	return result;
}

// cr_is_dumpable(task)
#if HAVE_MM_DUMPABLE
    #define __cr_mm_get_dumpable(mm) ((mm)->dumpable)
#elif defined(CR_KCODE_get_dumpable)
    #define __cr_mm_get_dumpable(mm) get_dumpable(mm)
#else
    #error
#endif
#define cr_is_dumpable(t) \
	(((t)->mm) && (__cr_mm_get_dumpable((t)->mm) == 1))

// bad_perms(task)
//
// Verify that the 'current' task is permitted to checkpoint the
// indicated task.  The checks are based on ensuring that the
// invoking user "owns" the task and will not expose sensitive data.
//
// Must call w/ tasklist_lock held to prevent task from disappearing.
//
// Returns 0 if OK.
// Returns non-zero error code if permission should be denied.
//
static int bad_perms(struct task_struct *task)
{
	cr_cred_t cred = cr_current_cred(), tcred;
	int result = 0;

	// We are always supposed to check capable/suser last.
	task_lock(task);
	tcred = cr_task_cred(task);
	if ((!cr_is_dumpable(task) ||
	     ((cred->euid != tcred->suid) && (cred->euid != tcred->uid) &&
	      (cred->uid  != tcred->suid) && (cred->uid  != tcred->uid)))
	    && !cr_capable(CAP_KILL)) {
		result = -EPERM;
	}
	task_unlock(task);

	return result;
}

// add_task(req, task)
//
// This function first verfies permissions to checkpoint the task.
// This function next records the req->task and task->chkpt_req associations.
// The req->task association in kept by the task_list field of the req.
// The task->chkpt_req association is kept by searching the list of requests,
// but could be put in a hash later.
//
// req->lock is not needed here because we are sole user.
// cr_task_lock must be held for the lookups.
//
// Returns 0 in the normal case.
// Returns non-zero error code on failure, such as out-of-memory.
//
static int add_task(cr_chkpt_req_t *req, struct task_struct *task)
{
	cr_chkpt_proc_req_t *proc_req = lookup_proc_req(req, task);
	cr_task_t *cr_task;
	int result;

	CR_KTRACE_FUNC_ENTRY("task=%p (%d)", task, task->pid);

	result = 0;

	/* XXX: Now that we check in add_proc(), these checks might be redundant */
	if (cri_task_zombie(task)) {
		CR_WARN_REQ(req, "Skipped zombie task %d - a post-restart wait() will not find this task", task->pid);
		goto out;
	} else if (cri_task_dead(task)) {
		goto out;
	}

	result = bad_perms(task);
	if (result) {
		CR_ERR_REQ(req, "Bad permissions to checkpoint task %d", task->pid);
		// Bad permissions
		goto out;
	}

	// Check for ptrace CHILD
	if (cr_is_ptrace_child(task)) {
		int flags = (req->flags & CR_PTRACED_FLAG_MASK);
		if (!flags) {
		    // Ptraced procs not allowed
		    result = -CR_EPTRACED;
		    goto out;
		} else if (flags == CR_CHKPT_PTRACED_SKIP) {
		    // Note that result==0
		    goto out;
		}
	}

	// Check for ptrace PARENT
	if (cr_is_ptrace_parent(task)) {
		int flags = (req->flags & CR_PTRACER_FLAG_MASK);
		result = (flags == CR_CHKPT_PTRACER_SKIP) ? 0 : -CR_EPTRACER;
		goto out;
	}

	cr_task = __cr_task_get(task, 1);

	result = -ENOMEM;
	if (!cr_task) {
		// No memory
		goto out;
	}

        if ((cr_task->chkpt_req == req) ||
	    ((proc_req->req == req) && proc_req->omit)) {
                if (proc_req->duplicate_flag != 1) {
                        /*
                         * This should NEVER happen
                         */
                        CR_ERR("Tried to add_task %d twice!", task->pid);
                        result = -EINVAL;
                } else {
			// Move task from the original list to the new list
			// This is only required for build_req_tree(), but is always safe.
			// NOTE: If (!cr_task->chkpt_req) then we'll filter it off of
			// the list before triggers are sent.
			list_move_tail(&cr_task->req_list, &req->tasks);
			CR_KTRACE_LOW_LVL("Confirmed duplicate task %d", task->pid);
			result = 0;
		}
		__cr_task_put(cr_task);
                goto out;
        } else if (cr_task->chkpt_req || cr_task->rstrt_proc_req) {
	        result = -EBUSY;
		// Overlapping request
		CR_KTRACE_LOW_LVL("Request overlap when checkpointing task %d", task->pid);
		__cr_task_put(cr_task);
		goto out;
	}

	/* XXX: This is a good spot to check the task for proper
	 * checkpointing support (i.e. the signal handler).
	 * However, when we move away from the signal this will
	 * need to go away. */
	if (cr_task->fd < 0) {
	    __sighandler_t handler;
	    CR_SIGNAL_LOCK(task);
	    handler = CR_SIGNAL_HAND(task, CR_SIGNUM);
	    CR_SIGNAL_UNLOCK(task);
	    if ((handler == SIG_DFL) || (handler == SIG_IGN)) {
		result = -CR_ENOSUPPORT;
		__cr_task_put(cr_task);
		goto out;
	    }
	} else {
	    // Identify our saved handler, in case user has overwritten.
	    proc_req->forced_sa = cr_task->handler_sa;
	    proc_req->ctrl_fd = cr_task->fd;
	}

	result = 0;
	if (cr_task->phase == CR_PHASE1) {
		atomic_inc(&proc_req->phase_barrier.count);
	}
	++proc_req->thread_count;
	atomic_inc(&proc_req->predump_barrier.count);
	atomic_inc(&proc_req->vmadump_barrier.count);
	atomic_inc(&proc_req->pre_complete_barrier.count);
	atomic_inc(&proc_req->post_complete_barrier.count);
	atomic_inc(&req->preshared_barrier.count);
	atomic_inc(&req->postdump_barrier.count);
	cr_task->chkpt_req = req;
	cr_task->chkpt_proc_req = proc_req;
	list_add_tail(&cr_task->req_list, &req->tasks);
	list_add_tail(&cr_task->proc_req_list, &proc_req->tasks);
	atomic_inc(&req->ref_count);
	proc_req->ref_count++;

out:
	release_proc_req(proc_req);
	return result;
}

// add_proc(req, task)
//
// loop over add_task for all threads in the proc
//
// Returns 0 in the normal case.
// Returns non-zero error code on failure, such as out-of-memory.
//
static int add_proc(cr_chkpt_req_t *req, struct task_struct *proc)
{
	struct task_struct *task;
	int result = 0;

	CR_KTRACE_LOW_LVL("Add proc pid=%d", proc->pid);

	if (cri_task_zombie(proc)) {
		/* XXX: W/ NPTL the non-main threads could still be live */
		CR_WARN_REQ(req, "Skipped zombie process %d - a post-restart wait() will not find this process", proc->tgid);
		goto out;
	} else if (cri_task_dead(proc)) {
		goto out;
	}

	if (!proc->mm || !proc->signal) {
		result = -EPERM;	/* tried to checkpoint a kernel thread or something ?? */
       	} else if (atomic_read(&proc->signal->count) == atomic_read(&proc->mm->mm_users)) {
		/* NEW (nptl) pthreads (or single threaded) */
		CR_DO_EACH_TASK_TGID(proc->tgid, task) {
			result = add_task(req, task);
			if (result) {
				break;
			}
		} CR_WHILE_EACH_TASK_TGID(proc->tgid, task);
	} else {
		/* OLD (linuthreads) pthreads */
		CR_DO_EACH_TASK_PROC(proc, task) {
			result = add_task(req, task);
			if (result) {
				break;
			}
		} CR_WHILE_EACH_TASK_PROC(proc, task);
	}

out:
	return result;
}

// build_req_proc(req, target)
//
// Locates the tasks which comprise a potentially multi-threaded
// process and verifies that the current task has permissions
// necessary to checkpoint them.
// A multi-threaded process may be named by the pid of any thread.
//
// Returns 0 if the tasks have been located and permissions verified.
// Returns non-zero error code otherwise.
//
static int build_req_proc(cr_chkpt_req_t *req, pid_t target)
{
	struct task_struct *proc;
	int result = -ESRCH;

	read_lock(&tasklist_lock);
	proc = target ? cr_find_task_by_pid(target)
		      : current; // Default target is self
	if (proc) {
		/* Get all the tasks in the process */
		result = add_proc(req, proc);
	}
	read_unlock(&tasklist_lock);

	return result;
}

// build_req_pgrp(req, target)
//
// Locates the tasks with a given process group id and verifies that the
// current task has permissions necessary to checkpoint them.
//
// Returns 0 if the tasks have been located and permissions verified.
// Returns non-zero error code otherwise.
//
static int build_req_pgrp(cr_chkpt_req_t *req, pid_t target)
{
	struct task_struct *proc;
	int result = -ESRCH;

	if (!target) {
		// Default target is own pgrp
		target = cr_task_pgrp(current);
	}

	read_lock(&tasklist_lock);
	CR_DO_EACH_TASK_PGID(target, proc) {
		result = add_proc(req, proc);
		if (result) {
			break;
		}
	} CR_WHILE_EACH_TASK_PGID(target, proc);
	read_unlock(&tasklist_lock);

	return result;
}

// build_req_sess(req, target)
//
// Locates the tasks with a given session id and verifies that the
// current task has permissions necessary to checkpoint them.
//
// Returns 0 if the tasks have been located and permissions verified.
// Returns non-zero error code otherwise.
//
static int build_req_sess(cr_chkpt_req_t *req, pid_t target)
{
	struct task_struct *proc;
	int result = -ESRCH;

	if (!target) {
		// Default target is own session
		target = cr_task_session(current);
	}

	read_lock(&tasklist_lock);
	CR_DO_EACH_TASK_SID(target, proc) {
		result = add_proc(req, proc);
		if (result) {
			break;
		}
	} CR_WHILE_EACH_TASK_SID(target, proc);
	read_unlock(&tasklist_lock);

	return result;
}

// build_req_tree(req, target)
//
// Locates the tasks which comprise a process tree (a process and
// all of its decendents).
// Note that if a parent-grandchild relation exists, but the child
// has exited, then this grandchild is excluded.
//
// Returns 0 if the tasks have been located and permissions verified.
// Returns non-zero error code otherwise.
//
// Implements a breadth-first-search "in-place" using the req's own
// task list.
//
// Note: Depending on LinuxThreads vs NPTL the members of the root
// task's thread group may either be its children or its siblings.
// So, we unconditionally add the entire thread group of the root.
// All other tasks are found via the lists of children, taking care
// not to revisit the members of the root task's thread group.
//
static int build_req_tree(cr_chkpt_req_t *req, pid_t target)
{
	struct task_struct *task;
	int result = -ESRCH;

        CR_KTRACE_LOW_LVL("in build_req_tree");

	read_lock(&tasklist_lock);
	task = target ? cr_find_task_by_pid(target)
		      : current; // Default target is self
	if (task) {
		cr_task_t *cr_task;
		struct mm_struct *root_mm = task->mm;

		/* Add "root" and all of its thread group */
		result = add_proc(req, task);
		if (result) {
			goto out_fail;
		}

                CR_KTRACE_LOW_LVL("scanning children");
		list_for_each_entry(cr_task, &req->tasks, req_list) {
			struct task_struct *child;
			task = cr_task->task;

                	CR_KTRACE_LOW_LVL("found child %d", task->pid);

			/* Add all the children - we get their children in the list_for_each() */
			CR_DO_EACH_CHILD(child, task) {

				if (child->mm == root_mm) continue;	/* Already visited */
				result = add_task(req, child);
				if (result) {
					goto out_fail;
				}
			} CR_WHILE_EACH_CHILD(child, task);
		}
	}
out_fail:
	read_unlock(&tasklist_lock);

	return result;	
}

static int build_req(cr_chkpt_req_t *req, cr_scope_t scope, pid_t target)
{
	int result = -EINVAL;

	switch(scope) {
	case CR_SCOPE_PROC:
		result = build_req_proc(req, target);
		break;

	case CR_SCOPE_PGRP:
		result = build_req_pgrp(req, target);
		break;

	case CR_SCOPE_SESS:
		result = build_req_sess(req, target);
		break;

	case CR_SCOPE_TREE:
		result = build_req_tree(req, target);
		break;
	}


	// Validate the task list
	if (result) {
		// Keep existing failure 
	} else if (req->flags & CR_CHKPT_PROHIBIT_SELF) {
		cr_task_t *cr_task;

		// If the request says to prohibit "self", then scan
		// the list for the requester, returning EDEADLK if found.
		list_for_each_entry(cr_task, &req->tasks, req_list) {
			if (cr_task->task->tgid == req->requester) {
				result = -EDEADLK;
				break;
			}
		}
	}

	return result;
}

static int cr_log_request(cr_chkpt_req_t *req, const char *verb, cr_scope_t scope, pid_t target, int signal)
{
#if CR_KERNEL_TRACING
	pid_t orig_target = target;
#endif
	const char *scope_msg = "";
	int result = 0;

	switch (scope) {
	case CR_SCOPE_PROC:
		if (!target) target = current->tgid;
		scope_msg = "process";
		break;
	case CR_SCOPE_PGRP:
		if (!target) target = cr_task_pgrp(current);
		scope_msg = "process group";
		break;
	case CR_SCOPE_SESS:
		if (!target) target = cr_task_session(current);
		scope_msg = "session";
		break;
	case CR_SCOPE_TREE:
		if (!target) target = current->tgid;
		scope_msg = "process tree";
		break;

	default:
		CR_ERR_REQ(req, "invalid scope value: %d", scope);
		result = -EINVAL;
                goto out;
	}

	if (!valid_signal(signal) && !valid_signal(-signal)) {
		CR_ERR_REQ(req, "invalid signal value: %d", signal);
		result = -EINVAL;
	}
	if (signal) {
		CR_KTRACE_HIGH_LVL("%s %s%s %d with signal %d", verb,
				   orig_target ? "" : "its own ",
				   scope_msg, target, signal);
	} else {
		CR_KTRACE_HIGH_LVL("%s %s%s %d", verb,
				   orig_target ? "" : "its own ",
				   scope_msg, target);
	}

out:
	return result;
}

static
int do_chkpt_req(struct file *filp, struct cr_chkpt_args *ureq)
{
	cr_pdata_t		*priv;
	cr_chkpt_req_t		*req;
	int result = 0;
	int did_retry = 0;

	CR_KTRACE_FUNC_ENTRY();

	// Check that we don't have a request on this fd already
	result = -EAGAIN;
	priv = filp->private_data;
	if (priv->chkpt_req) {
		goto out;
	}

	CR_KTRACE_LOW_LVL(" checkpoint params:  secs= %u, fd=%d",
			  ureq->cr_secs, ureq->cr_fd);

retry:
	// Allocate the kernel's structure to track the request
	req = alloc_request(filp);
	if (!req) {
		result = -ENOMEM;
		goto out;
	} else if (IS_ERR(req)) {
		result = PTR_ERR(req);
		goto out;
	}
	if (did_retry) {
		CR_WARN_REQ(req, "Retry request on -CR_ENOSUPPORT");
	}

	// XXX: Also copy the options if not NULL
	req->dump_format = ureq->dump_format;
	req->signal = ureq->signal;
	req->flags = ureq->flags;
	req->target = ureq->cr_target;
	req->checkpoint_scope = ureq->cr_scope;

	// Write simple syslog message while checking scope and signal arguments
	result = cr_log_request(req, "checkpointing", ureq->cr_scope, ureq->cr_target, ureq->signal);
	if (result)  {
		goto out_release;
	}

	// Validate the destination file descriptor
	result = cr_loc_init(req->errbuf, &req->dest, ureq->cr_fd, filp, /* is_write= */ 1);
	if (result) {
		CR_ERR_REQ(req, "Failed to initialize destination file descriptor");
		goto out_release;
	}

	// Hold the lock needed to ensure the request is constructed
	// atomically w.r.t. registration of Phase[12] checkpoint tasks
	// and other checkpoint requests.
	write_lock(&cr_task_lock);

	// Build the list of tasks
	result = build_req(req, ureq->cr_scope, ureq->cr_target);

	// Either cleanup or finalize
	if (result) {
		cr_task_t *cr_task, *next;

		// XXX: is this enough cleanup?
		//
		// req->lock not needed because we are sole user of req
		list_for_each_entry_safe(cr_task, next, &req->tasks, req_list) {
			__delete_from_req(req, cr_task);
			// Note that we (the requester) and each target task
			// hold a reference, so this release_request() can't 
			// destroy req.
			CRI_ASSERT(atomic_read(&req->ref_count) >= 2);
			(void)release_request(req);
		}

		write_unlock(&cr_task_lock);

		// Retry *ONCE* on result = -CR_ENOSUPPORT
		// We sleep 0.5s and then try again in the hope that the
		// CR_ENOSUPPORT was due to a "new born" process that
		// had not yet registered the signal handler.
		if ((result == -CR_ENOSUPPORT) && !did_retry) {
			DECLARE_WAIT_QUEUE_HEAD_ONSTACK(dummy);
			release_request(req);
			(void)wait_event_interruptible_timeout(dummy, 0, HZ/2);
			did_retry = 1;
			goto retry;
		}

		goto out_release;
	} else {
		// Send the triggers
		cr_trigger_phase1(req);

		// Release the lock
		write_unlock(&cr_task_lock);

		// add watchdog timer, which constitutes one reference:
		if (ureq->cr_secs
#if BITS_PER_LONG == 32
			       	&& (ureq->cr_secs < (MAX_JIFFY_OFFSET / HZ))
#endif
				                                           ) {
			req->expiration = jiffies + HZ * ureq->cr_secs;
			req->has_expiration = 1;
		} else {
			// Either the requester specified no expiration or
			// the requested expiration is too large to handle.
			req->has_expiration = 0;
		}
		// watchdog gets its own reference
		atomic_inc(&req->ref_count);
		cr_wd_add(&req->work);

		priv->chkpt_req = req;
	}

out:
	return result;

out_release:
	if (ureq->flags & CR_CHKPT_ASYNC_ERR) {
		// Save the error for later reporting at REAP
		req->result = result;
		priv->chkpt_req = req;
		result = 0;
	} else {
		release_request(req);
	}
	goto out;
}

// cr_chkpt_req(user_req)
//
// Processes a checkpoint request received from user space.
// Dispatches to build_req_*() to build a list of tasks.
// Calls cr_trigger_phase1() to trigger the checkpoint.
//
// Returns 0 on success, or a negative error code on failure.
int cr_chkpt_req(struct file *filp, struct cr_chkpt_args __user *arg)
{
	struct cr_chkpt_args ureq;
	int result;

	CR_KTRACE_FUNC_ENTRY();

	// Copy the user's request
	result = -EFAULT;
	if (copy_from_user(&ureq, arg, sizeof(ureq))) {
		goto out;
	}

	result = do_chkpt_req(filp, &ureq);

out:
	CR_KTRACE_FUNC_EXIT("Returning %d", result);
	return result;
}

static void
duplicate_set(cr_chkpt_req_t *req, int val)
{
	cr_chkpt_proc_req_t *proc_req;
        list_for_each_entry(proc_req, &req->procs, list) {
                proc_req->duplicate_flag = val;
        }
}

static
int do_chkpt_fwd(struct file *filp, pid_t target, cr_scope_t scope)
{
	cr_task_t *my_cr_task;
	cr_chkpt_req_t		*req = NULL;
	int result = 0;
	int did_retry = 0;
        LIST_HEAD(old_req_tasks);

	CR_KTRACE_FUNC_ENTRY();

        // Lookup this task and find the matching request
        result = -ESRCH;
        my_cr_task = cr_task_get(current);
	if (!my_cr_task) {
		goto out;
	}
	req = my_cr_task->chkpt_req;
       	cr_task_put(my_cr_task);
        if (!req) {
		goto out;
        }

	// Write simple syslog message while validating scope argument
	result = cr_log_request(req, "forwarding to", scope, target, 0);
	if (result)  {
		goto out;
	}

retry:
	// Hold the lock needed to ensure the request is constructed
	// atomically w.r.t. registration of Phase[12] checkpoint tasks
	// and other checkpoint requests.
	write_lock(&cr_task_lock);
        write_lock(&req->lock);

        /* Move existing list to a safe place */
        list_splice_init(&req->tasks, &old_req_tasks);

        /*
         * Mark all proc_req's with the duplicate flag
         */
        duplicate_set(req, 1);

	// Build the list of (additonal) tasks
	result = build_req(req, scope, target);

	// Send the triggers or clean up
	if (result) {
		cr_task_t *cr_task, *next;

                /* clean up code.  We aborted (possibly part way) while adding
                 * processes to the request.  We now want to undo everything
                 * that we may have set up earlier, and restore the req to
                 * it's state before any of this forwarding ever occurred.
                 */
		list_for_each_entry_safe(cr_task, next, &req->tasks, req_list) {
	                CR_KTRACE_LOW_LVL("cleaning up task: %d", cr_task->task->pid);

			// duplicates will move back to the orginal list below
			if (cr_task->chkpt_proc_req->duplicate_flag) continue;

                        // undo barriers, remove from req, and release the req.
			// Note that the requester and each target task
			// hold a reference, so this delete_dead_task()
			// can't destroy req.
			CRI_ASSERT(atomic_read(&req->ref_count) >= 2);
			delete_dead_task(req, cr_task);
		}
	} else {
                cr_chkpt_proc_req_t *proc_req;
                cr_task_t *cr_task, *next;

                // Filter omitted tasks that were on list only for build_req_tree()
                list_for_each_entry_safe(cr_task, next, &req->tasks, req_list) {
	            if (!cr_task->chkpt_req) {
			list_del_init(&cr_task->req_list);
		    }
                }
#if CRI_DEBUG
                /* dump the tasks out */
                list_for_each_entry(cr_task, &old_req_tasks, req_list) {
	            CR_KTRACE_LOW_LVL("old task: %d", cr_task->task->pid);
                }
                list_for_each_entry(cr_task, &req->tasks, req_list) {
		    int is_dup = cr_task->chkpt_proc_req->duplicate_flag;
		    CR_KTRACE_LOW_LVL("%s task: %d", (is_dup ? "dup" : "new"), cr_task->task->pid);
                }
#endif

		// Send the triggers to non-duplicates
                read_lock(&tasklist_lock);
                list_for_each_entry(proc_req, &req->procs, list) {
			if (proc_req->duplicate_flag) continue;
			__cr_trigger_phase1(proc_req);
		}
                read_unlock(&tasklist_lock);
	}

        /* clear the duplicate flag */
        duplicate_set(req, 0);

        /* Merge the new task list back with the original. */
        list_splice_init(&old_req_tasks, &req->tasks);

	write_unlock(&req->lock);
	write_unlock(&cr_task_lock);

	// Retry *ONCE* on result = -CR_ENOSUPPORT
	// We sleep 0.5s and then try again in the hope that the
	// CR_ENOSUPPORT was due to a "new born" process that
	// had not yet registered the signal handler.
	if ((result == -CR_ENOSUPPORT) && !did_retry) {
		DECLARE_WAIT_QUEUE_HEAD_ONSTACK(dummy);
		CR_WARN_REQ(req, "Retry forwarding on -CR_ENOSUPPORT");
		(void)wait_event_interruptible_timeout(dummy, 0, HZ/2);
		did_retry = 1;
		goto retry;
	}

out:
	CR_KTRACE_FUNC_EXIT("Returning %d", result);
	return result;
}

// cr_chkpt_fwd(user_req)
//
// Propagates a checkpoint request to a different target.
int cr_chkpt_fwd(struct file *filp, struct cr_fwd_args __user *arg)
{
	pid_t target;
	cr_scope_t scope;
	int result;

	CR_KTRACE_FUNC_ENTRY();

	// Collect the user's args
	result = -EFAULT;
	if (!access_ok(VERIFY_READ, arg, sizeof(*arg)) ||
	    __get_user(target, &arg->cr_target) ||
	    __get_user(scope, &arg->cr_scope)) {
                goto out;
	}

	result = do_chkpt_fwd(filp, target, scope);

out:
	CR_KTRACE_FUNC_EXIT("Returning %d", result);
	return result;
}

// need_lock: non-zero means we must acquire req->lock.  zero means we already hold it.
void cr_signal_phase_barrier(cr_task_t *cr_task, int block, int need_lock)
{
	CR_ASSERT_STEP_EQ(cr_task, CR_CHKPT_STEP_PHASE);
	cr_task->step++;
	if (cr_task->phase == CR_PHASE1) {
		cr_chkpt_proc_req_t *proc_req = cr_task->chkpt_proc_req;

		cr_barrier_notify(&proc_req->phase_barrier); //step++ outside of if()
                if (cr_barrier_once(&proc_req->phase_barrier, block)) {
			cr_chkpt_req_t *req = cr_task->chkpt_req;
			if (need_lock) read_lock(&req->lock);
                	read_lock(&tasklist_lock);
                        __cr_trigger_phase2(proc_req);
                	read_unlock(&tasklist_lock);
			if (need_lock) read_unlock(&req->lock);
                }
        }
}

// cr_signal_predump_barrier()
// Signal the per-process predump barrier
// performing the associated per-process fixups
int cr_signal_predump_barrier(cr_task_t *cr_task, int block)
{
	cr_chkpt_proc_req_t *proc_req = cr_task->chkpt_proc_req;
	struct task_struct *task = cr_task->task;
	int once;

	CR_ASSERT_STEP_EQ(cr_task, CR_CHKPT_STEP_PREDUMP);
	CR_BARRIER_NOTIFY(cr_task, &proc_req->predump_barrier);
	once = cr_barrier_once_interruptible(&proc_req->predump_barrier, block);

	/* XXX: Do we need more care w.r.t. dead/dying tasks?  Might we race here?
	 */
	if (cri_task_dead(task)) {
		// Don't check 'once' - if proc is already dead cleanup is pointless (and possibly unsafe)
	} else if (once > 0) {
		// restore any saved signal handler
	       	if (proc_req->saved_sa.sa.sa_handler != SIG_ERR) {
			CR_SIGNAL_LOCK(task);
    			CR_SIGACTION(task, CR_SIGNUM) = proc_req->saved_sa;
			CR_SIGNAL_UNLOCK(task);
			// NOTE: We don't reverse our forced unblocking of CR_SIGNUM
			// here because the signal return path would just undo it.
		}
	}

	return once;
}

// cr_signal_chkpt_complete_barrier()
// Signal the per-process pre_complete barrier
// performing the associated per-process fixups
static void cr_signal_chkpt_complete_barrier(cr_task_t *cr_task, int block)
{
	cr_chkpt_proc_req_t *proc_req = cr_task->chkpt_proc_req;
	cr_chkpt_req_t *req = cr_task->chkpt_req;

	CR_ASSERT_STEP_EQ(cr_task, CR_CHKPT_STEP_PRE_COMPLETE);
	CR_BARRIER_NOTIFY(cr_task, &proc_req->pre_complete_barrier);

	if (cr_barrier_once_interruptible(&proc_req->pre_complete_barrier, block) > 0) {
		struct task_struct *task = cr_task->task;
		int signal = 0;

		// close the temporary fd unless called from the watchdog
		if ((proc_req->tmp_fd >= 0) && (task == current)) {
			int rc = sys_close(proc_req->tmp_fd);
			if (rc) {
				CR_ERR_REQ(req, "Failed to close tmp_fd (err=%d)", rc);
			}
			proc_req->tmp_fd =  -1;
		}

		// raise a signal if needed
		if (req->die) {
			signal = SIGKILL;
		} else if ((req->signal > 0) && !proc_req->omit) {
			signal = req->signal;
		} else if (cr_task->stopped) {
			signal = SIGSTOP;
		}
		if (signal) {
			cr_kill_process(task, signal);
		}
	}
}


/**
 * cr_chkpt_abort
 * @cr_task: The task requesting the abort
 * @flags:  abort type
 *
 * Returns:   0 if checkpoint successfully aborted.
 */
int cr_chkpt_abort(cr_task_t *cr_task, unsigned int flags)
{
	cr_chkpt_proc_req_t *proc_req;
	cr_chkpt_req_t *req;
	int user_flags = ((flags >> 16) & 0xffff);
	int result;
	int block = 0;

	CR_KTRACE_FUNC_ENTRY("flags=0x%x", flags);

	/* Lookup the request */
	result = -ESRCH;
	proc_req = cr_task->chkpt_proc_req;
	if (!proc_req || !proc_req->req) {
		CR_ERR("%s: No matching req found!", __FUNCTION__);
		goto out;
	}
	req = proc_req->req;
	result = 0;

	write_lock(&cr_task_lock);
	write_lock(&req->lock);
	
	/* Set status flag of checkpoint to aborted: causes other tasks in
	 * checkpoint (either globally or locally) to abort by end of predump barrier. */
	switch (flags & 0xffff) {
	    case CR_CHECKPOINT_PERM_FAILURE:
		proc_req->omit = 1; /* for bug2526's race w/ forwarding */
		result = user_flags ? -user_flags : -CR_EPERMFAIL;
		req->result = result;
		req->die = 1;	/* tell peers to kill themselves */
		send_sig_info(SIGKILL, NULL, current); /* kill self */
		break;

	    case CR_CHECKPOINT_TEMP_FAILURE:
		proc_req->omit = 1; /* for bug2526's race w/ forwarding */
		req->signal = 0; /* suppress post-checkpoint signal delivery */
		result = user_flags ? -user_flags : -CR_ETEMPFAIL;
		req->result = result;
		break;

	    case CR_CHECKPOINT_OMIT:
		proc_req->omit = 1; /* tell same-proc peers not to checkpoint */
		result = -CR_EOMITTED;

		/* We need to block for our process to complete checkpoint.
		 * However, need to do so *after* dropping the locks.
		 * These ensure req and proc_req are not destroyed.
		 */
		atomic_inc(&req->ref_count);
		proc_req->ref_count++;
		block = 1;
		break;

	    case 0:
		/* Do nothing */
		CR_WARN("%s: called w/ pointless zero argument", __FUNCTION__);
		goto out_no_delete;
		break;

	    default:
		CR_ERR("%s: called w/ invalid argument", __FUNCTION__);
		result = -EINVAL;
		goto out_no_delete;
	}

	delete_dead_task(req, cr_task);
out_no_delete:
	write_unlock(&req->lock);
	write_unlock(&cr_task_lock);

	if (block) {
		CR_KTRACE_LOW_LVL("blocking for proces to complete (count=%d)", atomic_read(&proc_req->predump_barrier.count));
		CR_ASSERT_STEP_EQ(cr_task, 0); // Was reset by delete_dead_task
		cr_barrier_wait_interruptible(&proc_req->predump_barrier);
		release_proc_req(proc_req);
		(void)release_request(req);
	}

out:
	CR_KTRACE_FUNC_EXIT("Returning %d", result);
	return result;
}

// cr_chkpt_task_complete(cr_task, block)
// 
// Routine to release a given cr_ckpt_req_t.
// Called when a task has completed checkpointing itself (or aborts).
int cr_chkpt_task_complete(cr_task_t *cr_task, int block)
{
	cr_chkpt_req_t *req = cr_task->chkpt_req;
	cr_chkpt_proc_req_t *proc_req = cr_task->chkpt_proc_req;

	cr_signal_chkpt_complete_barrier(cr_task, block);

	CR_BARRIER_NOTIFY(cr_task, &proc_req->post_complete_barrier);
	if (block) (void)cr_barrier_wait_interruptible(&proc_req->post_complete_barrier);
	CR_ASSERT_STEP_EQ(cr_task, CR_CHKPT_STEP_DONE);

	delete_from_req(req, cr_task);
	release_request(req);

	return 0;
}

// cr_chkpt_req_release(filp, priv)
//
// Routine to release checkpoint request structures
// when a task closes the file (or exits).
//
// Must be called BEFORE cr_phase[12]_release
void cr_chkpt_req_release(struct file *filp, cr_pdata_t *priv)
{
	cr_chkpt_req_t *req;

	// If we are a requester, release the outstanding request.
	req = priv->chkpt_req;
	if (req && (req != CR_CHKPT_RESTARTED)) {
		(void)release_request(req);
	}
}

// poll method for file w/ associated checkpoint request
unsigned int
cr_chkpt_poll(struct file *filp, poll_table *wait)
{
	cr_pdata_t		*priv;
	cr_chkpt_req_t		*req;

	priv = filp->private_data;
	if (!priv) {
		return POLLERR;
	}
	req = priv->chkpt_req;
	if (!req) {
		return POLLERR;
	} else if (req == CR_CHKPT_RESTARTED) {
		return POLLIN | POLLRDNORM;
	}

	poll_wait(filp, &req->wait, wait);

	return check_done(req) ? (POLLIN | POLLRDNORM) : 0;
}

// cr_chkpt_reap()
//
// Reap a completed checkpoint.
// Returns:
// 	req->result on success
// 	-EINVAL if no un-reaped checkpoint request is associated w/ this fd
// 	-EAGAIN if the request is not completed
//	-ESRCH if no task was checkpointed (possibly because they all exited on us)
//	-CR_ERESTARTED if task has restarted from a checkpoint of itself
int cr_chkpt_reap(struct file *filp)
{
	cr_pdata_t		*priv;
	cr_chkpt_req_t		*req;
	int			retval;	

	retval = -EINVAL;
	priv = filp->private_data;
	if (priv) {
		req = priv->chkpt_req;
		if (req == CR_CHKPT_RESTARTED) {
			retval = -CR_ERESTARTED;
		} else if (req) {
			retval = -EAGAIN;
			if (check_done(req)) {
				priv->chkpt_req = NULL;
				retval = req->result;
				// req->result might be zero because nobody was actually checkpointed
				if (!retval && !atomic_read(&req->completed)) {
					retval = -ESRCH;
				}
				(void)release_request(req);
			}
		}
	}

	return retval;
}

static
cr_chkpt_req_t *do_chkpt_info(struct file *filp, char __user *ubuf)
{
	cr_chkpt_req_t *req = NULL;
	cr_task_t *cr_task;
	char *buf;
	const char *path = NULL;
	int retval;

	CR_KTRACE_FUNC_ENTRY();

	retval = -ESRCH;
	cr_task = cr_task_get(current);
	if (!cr_task) {
		goto out;
	}
	req = cr_task->chkpt_req;
	cr_task_put(cr_task);
	if (!req) {
		goto out;
	}

	retval = -ENOMEM;
	buf = __getname();
	if (!buf) {
		goto out;
	}

	path = cr_location2path(&(req->dest), buf, PATH_MAX);
	retval = 0;
	if (IS_ERR(path)) {
		retval = PTR_ERR(path);
	} else if (!path) {
		retval = -EBADF;
	} else if (copy_to_user(ubuf, path, 1+strlen(path))) {
		retval = -EFAULT;
	}

	__putname(buf);
out:
	CR_KTRACE_FUNC_EXIT("Returning %d", retval);
	return retval ? ERR_PTR(retval) : req;
}

int cr_chkpt_info(struct file *filp, struct cr_chkpt_info __user *arg)
{
    cr_chkpt_req_t *req;
    char __user *ubuf;
    int retval;

    CR_KTRACE_FUNC_ENTRY();

    retval = -EFAULT;
    if (get_user(ubuf, &(arg->dest))) {
	goto out;
    }

    req = do_chkpt_info(filp, ubuf);
    retval = PTR_ERR(req);
    if (IS_ERR(req)) {
	goto out;
    }

    retval = -EFAULT;
    if (!access_ok(VERIFY_WRITE, arg, sizeof(*arg)) ||
	__put_user(req->requester, &arg->requester) ||
	__put_user(req->target, &arg->target) ||
	__put_user(req->checkpoint_scope, &arg->scope) ||
	__put_user(req->signal, &arg->signal)) {
	goto out;
    }

    retval = 0;

out:
    CR_KTRACE_FUNC_EXIT("Returning %d", retval);
    return retval;
}

// Last arg indicates if we hold the req->lock.
void cr_chkpt_advance_to(cr_task_t *cr_task, int end_step, int hold_lock) {
	cr_chkpt_req_t *req = cr_task->chkpt_req;
	cr_chkpt_proc_req_t *proc_req = cr_task->chkpt_proc_req;

	CR_ASSERT_STEP_LE(cr_task, end_step);
	switch (cr_task->step) {
	case CR_CHKPT_STEP_PRESHARED:
		if (cr_task->step == end_step) break;
		CR_BARRIER_NOTIFY(cr_task, &req->preshared_barrier);
		// fall through...
	case CR_CHKPT_STEP_PHASE:
		if (cr_task->step == end_step) break;
		cr_signal_phase_barrier(cr_task, /* block= */0, /* need_lock= */ !hold_lock);
		// fall through...
	case CR_CHKPT_STEP_PREDUMP:
		if (cr_task->step == end_step) break;
		cr_signal_predump_barrier(cr_task, /* block= */0);
		// fall through...
	case CR_CHKPT_STEP_VMADUMP:
		if (cr_task->step == end_step) break;
		CR_BARRIER_NOTIFY(cr_task, &proc_req->vmadump_barrier);
		// fall through...
	case CR_CHKPT_STEP_POSTDUMP:
		if (cr_task->step == end_step) break;
		CR_BARRIER_NOTIFY(cr_task, &req->postdump_barrier);
		// fall through...
	case CR_CHKPT_STEP_PRE_COMPLETE:
		if (cr_task->step == end_step) break;
		cr_signal_chkpt_complete_barrier(cr_task, /* block= */0);
		// fall through...
	case CR_CHKPT_STEP_POST_COMPLETE:
		if (cr_task->step == end_step) break;
		CR_BARRIER_NOTIFY(cr_task, &proc_req->post_complete_barrier);
		// fall through...
	case CR_CHKPT_STEP_DONE:
		break;

	default:
		CR_ERR("Invalid cr_task->step %d", cr_task->step);
	}
}

static
int do_chkpt_log(struct file *filp, char __user *buf, unsigned int len)
{
    cr_pdata_t *priv;
    cr_chkpt_req_t *req; 
    int retval;

    CR_KTRACE_FUNC_ENTRY();

    retval = -EINVAL;
    if (filp == NULL) {
        CR_ERR("%s: Called with NULL file structure", __FUNCTION__);
        goto out;
    }
    priv = filp->private_data;
    if (priv == NULL) {
        CR_ERR("%s: private_data is NULL!", __FUNCTION__);
        goto out;
    }
    req = priv->chkpt_req;
    if (req == NULL) {
	CR_ERR("%s: No chkpt_req attached to filp!", __FUNCTION__);
        goto out;
    } else if (req == CR_CHKPT_RESTARTED) {
	return 0;
    }

    retval = cr_errbuf_read(buf, len, req->errbuf);

out:
    CR_KTRACE_FUNC_EXIT("Returning %d", retval);
    return retval;
}

/**
 * cr_chkpt_log
 * @filp: The filp for the checkpoint request
 * @arg:  Pointer to user's struct cr_log_args
 *
 * Copies up to arg->len bytes to arg->buf.
 * For any non-error return arg->buf is nul-terminated (even for the
 * case of an empty log).
 *
 * The caller can determine the space required by calling w/ arg->len
 * set to zero.
 *
 * Returns:   Number of bytes required to retrieve full log.
 */
int cr_chkpt_log(struct file *filp, struct cr_log_args __user *arg)
{
    unsigned int len;
    char __user *buf;
    int retval;

    CR_KTRACE_FUNC_ENTRY();

    retval = -EFAULT;
    if (!access_ok(VERIFY_READ, arg, sizeof(*arg)) ||
	__get_user(len, &arg->len) ||
	__get_user(buf, &arg->buf)) {
	goto out;
    }

    retval = do_chkpt_log(filp, buf, len);

out:
    CR_KTRACE_FUNC_EXIT("Returning %d", retval);
    return retval;
}

#ifdef CONFIG_COMPAT
int cr_chkpt_req32(struct file *file, struct cr_compat_chkpt_args __user *req)
{
	struct cr_chkpt_args ureq;
	int retval;

	CR_KTRACE_FUNC_ENTRY();

	retval = -EFAULT;
	if (!access_ok(VERIFY_READ, req, sizeof(*req)) ||
	    __get_user(ureq.cr_target, &req->cr_target) ||
	    __get_user(ureq.cr_scope, &req->cr_scope) ||
	    __get_user(ureq.cr_fd, &req->cr_fd) ||	
	    __get_user(ureq.cr_secs, &req->cr_secs) ||
	    __get_user(ureq.dump_format, &req->dump_format) ||
	    __get_user(ureq.signal, &req->signal) ||
	    __get_user(ureq.flags, &req->flags)) {
		goto out;
	}

	retval = do_chkpt_req(file, &ureq);

out:
	CR_KTRACE_FUNC_EXIT("Returning %d", retval);
	return retval;
}

int cr_chkpt_fwd32(struct file *filp, struct cr_compat_fwd_args __user *arg)
{
    pid_t target;
    cr_scope_t scope;
    int result;

    CR_KTRACE_FUNC_ENTRY();

    // Collect the user's args
    result = -EFAULT;
    if (!access_ok(VERIFY_READ, arg, sizeof(*arg)) ||
	__get_user(target, &arg->cr_target) ||
	__get_user(scope, &arg->cr_scope)) {
	goto out;
    }

    result = do_chkpt_fwd(filp, target, scope);

out:
    CR_KTRACE_FUNC_EXIT("Returning %d", result);
    return result;
}

int cr_chkpt_info32(struct file *filp, struct cr_compat_chkpt_info __user *arg)
{
    cr_chkpt_req_t *req;
    compat_uptr_t ubuf;
    int retval;

    CR_KTRACE_FUNC_ENTRY();

    retval = -EFAULT;
    if (get_user(ubuf, &(arg->dest))) {
	goto out;
    }

    req = do_chkpt_info(filp, compat_ptr(ubuf));
    retval = PTR_ERR(req);
    if (IS_ERR(req)) {
	goto out;
    }

    retval = -EFAULT;
    if (!access_ok(VERIFY_WRITE, arg, sizeof(*arg)) ||
	__put_user(req->requester, &arg->requester) ||
	__put_user(req->target, &arg->target) ||
	__put_user(req->checkpoint_scope, &arg->scope) ||
	__put_user(req->signal, &arg->signal)) {
	goto out;
    }

    retval = 0;

out:
    CR_KTRACE_FUNC_EXIT("Returning %d", retval);
    return retval;
}

int cr_chkpt_log32(struct file *filp, struct cr_compat_log_args __user *arg)
{
    unsigned int len;
    compat_uptr_t buf;
    int retval;

    CR_KTRACE_FUNC_ENTRY();

    retval = -EFAULT;
    if (!access_ok(VERIFY_READ, arg, sizeof(*arg)) ||
	__get_user(len, &arg->len) ||
	__get_user(buf, &arg->buf)) {
	goto out;
    }

    retval = do_chkpt_log(filp, compat_ptr(buf), len);

out:
    CR_KTRACE_FUNC_EXIT("Returning %d", retval);
    return retval;
}
#endif /* CONFIG_COMPAT */
