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
 * $Id: cr_task.c,v 1.21.6.1 2009/03/06 19:26:28 phargrov Exp $
 */

#include "cr_module.h"

// As a simple convention, the routines here which start w/ '__'
// assume you hold the proper locks, while others are to be called
// without holding the locks.

// List of all tasks for which the C/R module has information.
// This is a place to keep all the data that one would consider
// adding to (struct task_struct) if C/R were a patch rather
// than a module.
// TODO: use a hash rather than a list.
LIST_HEAD(cr_task_list);

// Read/write spinlock to protect cr_task_list.
//
// This lock nests OUTSIDE the kernel's tasklist_lock.
//
// This lock nests OUTSIDE any request-specific locks which
// are linked off the cr_task_t.
CR_DEFINE_RWLOCK(cr_task_lock);

// __cr_task_get(task, create)
//
// Finds an entry for the given task is one exists.
// If (create != 0) will create one when none exists.
//
// Note that even if (create != 0) we could return NULL if
// unable to allocate the memory.  This is because calling
// cr_kmem_cache_zalloc() w/ other than GFP_ATOMIC might sleep while
// holding the spinlock.  Since we cannot know in this routine what
// other locks the caller might hold, it is the caller's problem
// to deal with a NULL return.
// XXX: can we fix this problem?
//
// XXX: This could be implemented w/ a hash rather than a linear search.
//
// Must be called w/ cr_task_lock held (for writing if create != 0).
cr_task_t *__cr_task_get(struct task_struct *task, int create)
{
	cr_task_t *cr_task;

	list_for_each_entry(cr_task, &cr_task_list, task_list) {
		if (cr_task->task == task) {
			atomic_inc(&cr_task->ref_count);
			return cr_task;
		}
	}

	cr_task = NULL;
	if (create) {
#if CRI_DEBUG
		if (!CR_MODULE_GET()) {
			CR_ERR("Checkpoint API call after rmmod!");
			cr_task = ERR_PTR(-EINVAL);
			goto out;
		}
#endif
 
		cr_task = cr_kmem_cache_zalloc(*cr_task, cr_task_cachep, GFP_ATOMIC);

		if (cr_task) {
			atomic_set(&cr_task->ref_count, 1);
                	cr_task->task = task;
                	cr_task->fd = -1;
                	cr_task->self_exec_id = task->self_exec_id;
                	INIT_LIST_HEAD(&cr_task->req_list);
			INIT_LIST_HEAD(&cr_task->proc_req_list);

			get_task_struct(task);
			list_add_tail(&cr_task->task_list, &cr_task_list);
#if CRI_DEBUG
			CR_KTRACE_REFCNT("Alloc cr_task_t %p for pid %d", cr_task, task->pid);
		} else {
			CR_MODULE_PUT();
#endif
		}
	}

#if CRI_DEBUG
out:
#endif
	return cr_task;
}

// __cr_task_put(cr_task)
//
// Drop one reference to a cr_task.
// If this is the last reference then free the resources.
//
// Must be called w/ cr_task_lock held for writing.
void __cr_task_put(cr_task_t *cr_task)
{
	CRI_ASSERT(atomic_read(&cr_task->ref_count));
	if (atomic_dec_and_test(&cr_task->ref_count)) {
		list_del(&cr_task->task_list);
		put_task_struct(cr_task->task);
		kmem_cache_free(cr_task_cachep, cr_task);
#if CRI_DEBUG
		CR_MODULE_PUT();
		CR_KTRACE_REFCNT("Free cr_task_t %p", cr_task);
#endif
	} else if (atomic_read(&cr_task->ref_count) <= 0) {
		CR_WARN("%s [%d]:  WARNING:  Unbalanced __cr_task_put on cr_task_t %p", __FUNCTION__, current->pid, cr_task);
	}
}

// cr_task_get(task)
//
// Routine to find the request corresponding to a given task
// Called when a task begins checkpointing itself.
// Can also be called to see if any request is outstanding for
// the given task.
//
// Called w/o holding the cr_task_lock.
cr_task_t *cr_task_get(struct task_struct *task)
{
	cr_task_t *cr_task;

	read_lock(&cr_task_lock);
	cr_task = __cr_task_get(task, 0);
	read_unlock(&cr_task_lock);

	return cr_task;
}

// cr_task_put(cr_task)
//
// Drop one reference to a cr_task.
// If this is the last reference then free the resources.
//
// Called w/o holding the cr_task_lock.
void cr_task_put (cr_task_t *cr_task)
{
	write_lock(&cr_task_lock);
	__cr_task_put(cr_task);
	write_unlock(&cr_task_lock);
}
