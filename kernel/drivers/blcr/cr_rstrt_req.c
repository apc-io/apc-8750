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
 * $Id: cr_rstrt_req.c,v 1.393.4.2 2009/06/07 03:27:36 phargrov Exp $
 */

#include "cr_module.h"

#include <linux/pipe_fs_i.h>
#include <linux/time.h>
#include <linux/mman.h>
#include <linux/major.h>
#include <linux/pid.h>
#include <asm/uaccess.h>

#include <linux/binfmts.h>

#include "cr_context.h"
#include "vmadump.h"
//#include <stdlib.h>
#ifndef PID_MAX
   /* XXX: Is this always true? */
  #define PID_MAX 0x8000
#endif

typedef struct cr_linkage_s {
	struct list_head		list;
	struct cr_context_tasklinkage	link;
	struct task_struct		*tl_task;
	cr_task_t			*cr_task;
	int				reserved;
} cr_linkage_entry_t;

static cr_kmem_cache_ptr cr_linkage_cachep = NULL;

#define CR_RESTORE_PIDS_MASK	(CR_RSTRT_RESTORE_PID | CR_RSTRT_RESTORE_PGID | CR_RSTRT_RESTORE_SID)

static void cr_release_ids(struct list_head *list_p);
static int cr_restore_linkage(cr_rstrt_req_t *req);
static void rstrt_watchdog(cr_work_t *work);

/*
 * alloc_rstrt_request
 */
static cr_rstrt_req_t *
alloc_rstrt_req(void)
{
    static struct lock_class_key lock_key;
    cr_rstrt_req_t *req = NULL;
    cr_objectmap_t map;

    if (!CR_MODULE_GET()) {
	CR_ERR("Restart request after rmmod!");
	req = ERR_PTR(-EINVAL);
	goto out_rmmod;
    }
 
    map = cr_alloc_objectmap();
    if (!map) {
	goto out_modput;
    }

    req = cr_kmem_cache_zalloc(*req, cr_rstrt_req_cachep, GFP_ATOMIC);
    if (req) {
	CR_KTRACE_ALLOC("Alloc cr_rstrt_req_t %p", req);
        atomic_set(&req->ref_count, 1);
	req->requester = current->tgid;
	get_task_struct(current);
	req->cr_restart_task = current;
	req->cr_restart_stdin = fget(0);
	req->cr_restart_stdout = fget(1);
	req->state = CR_RSTRT_STATE_REQUESTER;
	init_waitqueue_head(&req->wait);
	cr_barrier_init(&req->barrier, 1);
	req->result = 0;
	req->die = 0;
	req->map = map;
	rwlock_init(&req->lock);
	lockdep_set_class(&req->lock, &lock_key);
	INIT_LIST_HEAD(&req->tasks);
	INIT_LIST_HEAD(&req->procs);
	INIT_LIST_HEAD(&req->linkage);
	CR_INIT_WORK(&req->work, &rstrt_watchdog);
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

static void
release_rstrt_req(cr_rstrt_req_t *req)
{
    CR_KTRACE_REFCNT("ref count is approximately %d", 
		     atomic_read(&req->ref_count));

    CRI_ASSERT(atomic_read(&req->ref_count));
    if (atomic_dec_and_test(&req->ref_count)) {
	cr_rstrt_proc_req_t *proc_req, *next;
	list_for_each_entry_safe(proc_req, next, &req->procs, list) {
	    cr_release_ids(&proc_req->linkage);
            if (proc_req->mmaps_tbl) {
	        vfree(proc_req->mmaps_tbl);
	    }
#if CRI_DEBUG
	    if (proc_req->tmp_fd >= 0) {
		CR_ERR("Leaking tmp_fd");
	    }
#endif
	    kmem_cache_free(cr_rstrt_proc_req_cachep, proc_req);
	}
	cr_release_ids(&req->linkage);
	CR_KTRACE_REFCNT("Releasing source file");
	cr_loc_put(&req->src, req->file0);
	cr_loc_free(&req->src);
	CR_KTRACE_REFCNT("fput()'ing stdin/out");
	fput(req->cr_restart_stdin);
	fput(req->cr_restart_stdout);
	CR_KTRACE_REFCNT("put_task_struct(cr_restart_task)");
	put_task_struct(req->cr_restart_task);
	cr_release_objectmap(req->map);
	cr_free_reloc(req->relocate);
	cr_errbuf_free(req->errbuf);
        kmem_cache_free(cr_rstrt_req_cachep, req);
        CR_MODULE_PUT();
	CR_KTRACE_ALLOC("Free cr_rstrt_req_t %p", req);
    }
}

/*
 * cr_load_linkage
 *
 * Loads the linkage structure stored in the context file into a new
 * linked list of elements, returns linkage count (or error)
 *
 * This linkage structure is used by cr_restore_linkage() to set pid,
 * tgid, pgid and sid; and to reconstruct the graph of processes,
 */
static int cr_load_linkage(cr_rstrt_proc_req_t *proc_req)
{
    struct list_head *list_p = &proc_req->linkage;
    cr_errbuf_t *eb = proc_req->req->errbuf;
    struct file *filp = proc_req->file;
    struct cr_context_tasklinkage *linkage;
    int linkage_bytes;                  /* size of linkage stored on disk */
    int linkage_size;                   /* number of entries in array */
    int retval;
    int i;

    /* find out how large the linkage structure is */
    retval = cr_kread(eb, filp, &linkage_bytes, sizeof(linkage_bytes));
    if (retval < sizeof(linkage_bytes)) {
	CR_ERR_PROC_REQ(proc_req, "linkage: read size returned %d", retval);
        goto out;
    }

    /* allocate memory for the linkage structure */
    CR_NO_LOCKS();
    linkage = NULL;
    linkage_size = 0;
    if (linkage_bytes < CR_KMALLOC_MAX) {
        linkage = kmalloc(linkage_bytes, GFP_KERNEL);
	linkage_size = linkage_bytes/sizeof(*linkage);
    }
    if (linkage == NULL || linkage_size == 0) {
        CR_ERR_PROC_REQ(proc_req, "Too many processes (size = %d)", linkage_bytes);
        retval = -ENOMEM;
	goto out;
    }

    /* Now load the linkage from disk */
    retval = cr_kread(eb, filp, linkage, linkage_bytes);
    if (retval < linkage_bytes) {
	CR_ERR_PROC_REQ(proc_req, "linkage: read table returned %d", retval);
        goto out_free;
    }

    /* Build linked list */
    retval = -ENOMEM;
    for (i=0; i<linkage_size; ++i) {
	cr_linkage_entry_t *entry = cr_kmem_cache_zalloc(*entry, cr_linkage_cachep, GFP_KERNEL);
	if (!entry) {
    	    cr_linkage_entry_t *next;
	    list_for_each_entry_safe(entry, next, list_p, list) {
	        kmem_cache_free(cr_linkage_cachep, entry);
	    }
	    INIT_LIST_HEAD(list_p);
	    CR_ERR_PROC_REQ(proc_req, "No memory to build linkage table entry");
	    goto out_free;
	}
	entry->link = linkage[i];
#if 0 /* Redundant since we now use cr_kmem_cache_zalloc() */
	entry->tl_task = NULL;  /* NOTE: needed to check for missing tasks in cr restore linkage */
	entry->cr_task = NULL;
	entry->reserved = 0;
#endif

	list_add_tail(&entry->list, list_p);
#if CRI_DEBUG
	/* Reverse the list to trigger any bugs related to pid_link() */
	list_move(&entry->list, list_p);
#endif
    }

    retval = linkage_size;

out_free:
    kfree(linkage);
out:
    return retval;
}

/*
 * cr_linkage_find_real_parent
 *
 * Figure out who your parent process ought to be.
 *
 * As a "Safety net" we default to becoming a child of the
 * cr_restart process if the original parent is not found.
 */
struct task_struct *
cr_linkage_find_real_parent(cr_rstrt_req_t *req, struct task_struct *old_parent)
{
    struct task_struct *parent = NULL;

    if (!cr_find_object(req->map, old_parent, (void **) &parent)) {
        /* No entry for this process, we'll reattatch it as a new child
         * of cr_restart. */
        parent = req->cr_restart_task;
    }

    return parent;
}

/* Code based heavily on kernel/pid.c */
#include <linux/threads.h>
#define BITS_PER_PAGE (8*PAGE_SIZE)
#define BITS_PER_PAGE_MASK (BITS_PER_PAGE-1)
#if defined(CR_KDATA_pidmap_array)
  #define cr_pidmap(nr)		&(pidmap_array[(nr) / BITS_PER_PAGE])
  #define cr_pidmap_alloc()	((void *)get_zeroed_page(GFP_KERNEL))
  #define cr_pidmap_free(_p)	free_page((unsigned long)(_p))
#elif defined(CR_KDATA_init_pspace) || defined(CR_KDATA_init_pid_ns)
  #define cr_pidmap(nr)		&(cr_init_pid_ns.pidmap[(nr) / BITS_PER_PAGE])
  #define cr_pidmap_alloc()	kzalloc(PAGE_SIZE, GFP_KERNEL)
  #define cr_pidmap_free(_p)	kfree(_p)
#else
  #error "Can't locate pidmap pages"
#endif
static inline void
_cr_free_pid(int nr) /* Call w/ tasklist_lock held for writing */
{
#if HAVE_ALLOC_PID
    CRI_ASSERT(nr);
    free_pid(cr_find_pid(nr));
#else
    struct pidmap *map = cr_pidmap(nr);
    int offset = nr & BITS_PER_PAGE_MASK;

    CRI_ASSERT(nr);
    clear_bit(offset, map->page);
    atomic_inc(&map->nr_free);
#endif
}
#if !defined(CR_KDATA_pidmap_lock)
  /* Not SMP.  Define here to allow same code to compile on SMP and UP */
  static CR_DEFINE_SPINLOCK(pidmap_lock);
#endif
static int
_cr_alloc_pid(int nr) /* MUST call with no locks held */
{
    struct pidmap *map;
    int offset = (nr & BITS_PER_PAGE_MASK);
    int result = 0;	/* assume failure */

    CR_NO_LOCKS();

    if ((nr <= 0) || (nr >= PID_MAX_LIMIT)) {
        CR_ERR("Invalid pid %d", nr);
        return 0;
    }

    /* make sure we have a page */
    map = cr_pidmap(nr);
    if (unlikely(map->page == NULL)) {
	void *page = cr_pidmap_alloc();

	spin_lock_irq(&pidmap_lock);
	if (map->page) {
	    /* lost a race, free our page */
	    cr_pidmap_free(page);
	} else {
	    map->page = page;
	}
	spin_unlock_irq(&pidmap_lock);
    }
    /* XXX assert (map->page != NULL) */

    if (!test_and_set_bit(offset, map->page)) {
    #if HAVE_ALLOC_PID
	struct pid *pid;
	rcu_read_lock();
	pid = cr_find_pid(nr);
	rcu_read_unlock();
	if (!pid) {
	#if HAVE_0_ARG_ALLOC_PID
            #define pid_hashfn(nr) hash_long((unsigned long)nr, pidhash_shift)
	    enum pid_type type;
	    pid = kmem_cache_alloc(pid_cachep, GFP_KERNEL);

	    if (!pid) {
	        clear_bit(offset, map->page);
	        goto out;
	    }
	    atomic_set(&pid->count, 1);
	    pid->nr = nr;
	    for (type = 0; type < PIDTYPE_MAX; ++type) {
	        INIT_HLIST_HEAD(&pid->tasks[type]);
	    }

	    spin_lock_irq(&pidmap_lock);
	    hlist_add_head_rcu(&pid->pid_chain, &pid_hash[pid_hashfn(nr)]);
	    spin_unlock_irq(&pidmap_lock);
	#elif HAVE_1_ARG_ALLOC_PID
	    // XXX: Need multi-level support
            #define pid_hashfn(nr,ns) hash_long((unsigned long)nr + (unsigned long)ns, pidhash_shift)
	    enum pid_type type;
	    struct upid *upid;
	    pid = kmem_cache_alloc(init_pid_ns.pid_cachep, GFP_KERNEL);

	    if (!pid) {
	        clear_bit(offset, map->page);
	        goto out;
	    }
	    pid->numbers[0].nr = nr;
	    pid->numbers[0].ns = &init_pid_ns;
	    pid->level = init_pid_ns.level;
	    atomic_set(&pid->count, 1);
	    for (type = 0; type < PIDTYPE_MAX; ++type) {
	        INIT_HLIST_HEAD(&pid->tasks[type]);
	    }

	    spin_lock_irq(&pidmap_lock);
	    upid = &pid->numbers[0];
	    hlist_add_head_rcu(&upid->pid_chain, &pid_hash[pid_hashfn(upid->nr, upid->ns)]);
	    spin_unlock_irq(&pidmap_lock);
	#endif
	}
    #endif
	atomic_dec(&map->nr_free);
	/*CRI_ASSERT(!cr_have_pid(PIDTYPE_PID, nr));
	CRI_ASSERT(!cr_have_pid(PIDTYPE_PGID, nr));
	CRI_ASSERT(!cr_have_pid(PIDTYPE_SID, nr));*/
	result = 1;
    }

#if HAVE_ALLOC_PID
out:
#endif
    return result;
}

/* Checks if any of the ids in 'entry' requested by 'mask' are
 * already registered by 'other'.
 * Returns modified 'mask' with bits cleared for any matches.
 */
static int
cr_match_ids(const cr_linkage_entry_t *entry, const cr_linkage_entry_t *other, int mask)
{
    int reserved = other->reserved;

    if (mask & CR_RSTRT_RESTORE_PID) {
	int id = entry->link.pid;
	if (((reserved & CR_RSTRT_RESTORE_PGID) && (id == entry->link.pgrp)) ||
	    ((reserved & CR_RSTRT_RESTORE_SID)  && (id == entry->link.session))) {
	    mask &= ~CR_RSTRT_RESTORE_PID;
	}
    }

    if (mask & CR_RSTRT_RESTORE_PGID) {
	int id = entry->link.pgrp;
	if (((reserved & CR_RSTRT_RESTORE_PID)  && (id == entry->link.pid)) ||
	    ((reserved & CR_RSTRT_RESTORE_PGID) && (id == entry->link.pgrp)) ||
	    ((reserved & CR_RSTRT_RESTORE_SID)  && (id == entry->link.session))) {
	    mask &= ~CR_RSTRT_RESTORE_PGID;
	}
    }

    if (mask & CR_RSTRT_RESTORE_SID) {
	int id = entry->link.session;
	if (((reserved & CR_RSTRT_RESTORE_PID)  && (id == entry->link.pid)) ||
	    ((reserved & CR_RSTRT_RESTORE_PGID) && (id == entry->link.pgrp)) ||
	    ((reserved & CR_RSTRT_RESTORE_SID)  && (id == entry->link.session))) {
	    mask &= ~CR_RSTRT_RESTORE_SID;
	}
    }

    return mask;
}

/* Reserve all the pids we need
 * Returns non-zero on failure
 */
static int
cr_reserve_ids(cr_rstrt_proc_req_t *proc_req)
{
    struct list_head *list_p = &proc_req->linkage;
    cr_rstrt_req_t *req = proc_req->req;
    cr_linkage_entry_t *entry, *next;
    int pid_flags = req->flags & CR_RESTORE_PIDS_MASK;
    int retval = 0;
    
    CR_KTRACE_HIGH_LVL("Now reserving required ids... ");

    /* Don't need to worry about session IDs, since exactly one member
     * of the session, the leader, will have a matching PID.  (When that
     * leader exits the entire session dies).
     * XXX: verify that and simplify the code if needed
     */

#if CRI_DEBUG
    /* This otherwise pointless code ensures that a debug build
     * will exercise the _cr_free_pid() code that is otherwise
     * almost never reached.
     */
    entry = list_entry(list_p->next, cr_linkage_entry_t, list);
    if (_cr_alloc_pid(entry->link.pid)) {
	_cr_free_pid(entry->link.pid);
    }
#endif

    list_for_each_entry_safe(entry, next, list_p, list) {
	int mask = pid_flags;
	cr_linkage_entry_t *other, *next2;

	/* Only thread group leaders can change pgrp or session */
	if (entry->link.pid != entry->link.tgid) {
	    mask &= CR_RSTRT_RESTORE_PID;
	    if (!mask) continue;
	}

	/* First try to allocated each needed identifier */
	CRI_ASSERT(entry->reserved == 0);
	if ((mask & CR_RSTRT_RESTORE_PID) && _cr_alloc_pid(entry->link.pid)) {
	    entry->reserved |= CR_RSTRT_RESTORE_PID;
	}
	if ((mask & CR_RSTRT_RESTORE_PGID) && _cr_alloc_pid(entry->link.pgrp)) {
	    entry->reserved |= CR_RSTRT_RESTORE_PGID;
	}
	if ((mask & CR_RSTRT_RESTORE_SID) && _cr_alloc_pid(entry->link.session)) {
	    entry->reserved |= CR_RSTRT_RESTORE_SID;
	}

	mask &= ~entry->reserved;
	if (!mask) continue;

	/* Next see of the needed ids have been allocated by an earlier thread of this
	 * process or by our own self (e.g. process group leaders have pid == pgid) */
	list_for_each_entry_safe(other, next2, &proc_req->linkage, list) {
	    int tmp = cr_match_ids(entry, other, mask);
	    if (tmp != mask) {
		// Move to head on assumption of "clustered" searches
		list_move(&other->list, &proc_req->linkage);
	    }
	    mask = tmp;
	    if (!mask) goto done;
	    if (other == entry) break; // No need to search past self
	}

	/* Finally check the rest of the req
	 * XXX: This is not "safe" until we ensure list_splice + search are atomic,
	 * but restores are currently serialized anyway.
	 */
	list_for_each_entry_safe(other, next2, &req->linkage, list) {
	    int tmp = cr_match_ids(entry, other, mask);
	    if (tmp != mask) {
		// Move to head on assumption of "clustered" searches
		list_move(&other->list, &req->linkage);
	    }
	    mask = tmp;
	    if (!mask) goto done;
	}

	if (mask) {
	    if (mask & CR_RSTRT_RESTORE_PID) {
		CR_ERR_PROC_REQ(proc_req, "found pid %d in use", entry->link.pid);
	    }
	    if (mask & CR_RSTRT_RESTORE_PGID) {
		CR_ERR_PROC_REQ(proc_req, "found pgrp %d in use", entry->link.pgrp);
	    }
	    if (mask & CR_RSTRT_RESTORE_SID) {
		CR_ERR_PROC_REQ(proc_req, "found session %d in use", entry->link.session);
	    }

	    retval = -EBUSY;
	    /* continue looping to log all conflicts... */
	}
done:	(void)0; // Because gcc may complain about label at end of a loop
    }

    return retval;
}

static void
cr_release_ids(struct list_head *list_p)
{
    cr_linkage_entry_t *entry, *next;
    
    write_lock_irq(&tasklist_lock);

    list_for_each_entry(entry, list_p, list) {
	int mask = entry->reserved;
	entry->reserved = 0;

	if (mask & CR_RSTRT_RESTORE_PID) {
	    _cr_free_pid(entry->link.pid);
	}
	if (mask & CR_RSTRT_RESTORE_PGID) {
	    _cr_free_pid(entry->link.pgrp);
	}
	if (mask & CR_RSTRT_RESTORE_SID) {
	    _cr_free_pid(entry->link.session);
	}
    }

    write_unlock_irq(&tasklist_lock);

    /* Free the list in a second pass, w/o the tasklist_lock held */
    list_for_each_entry_safe(entry, next, list_p, list) {
	kmem_cache_free(cr_linkage_cachep, entry);
    }
    INIT_LIST_HEAD(list_p);
}

static
int do_rstrt_request_restart(struct file *filp, struct cr_rstrt_args *ureq, 
                             int (*reloc_reader)(cr_rstrt_req_t *, void __user *))
{
    struct cr_context_file_header cf_header;
    cr_pdata_t *priv;
    cr_rstrt_req_t *req;
    cr_errbuf_t *eb;
    int retval;

    CR_KTRACE_FUNC_ENTRY();

    /* Make sure that we don't have any requests already on this fd */
    retval = -EAGAIN;
    priv = filp->private_data;
    if (priv->rstrt_req || priv->chkpt_req) {
	goto out;
    }

    /* Allocate a request structure for bookkeeping */
    retval = -ENOMEM;
    req = alloc_rstrt_req();
    if (req == NULL) {
	goto out;
    } else if (IS_ERR(req)) {
        return PTR_ERR(req);
	goto out;
    }
    priv->rstrt_req = req;
    eb = req->errbuf;

    // get/validate relocation records
    if (ureq->relocate) {
	retval = reloc_reader(req, ureq->relocate);
	if (retval) {
	    goto out_free_req;
	}
    }
    
    req->flags = ureq->flags;

    // validate signal number
    retval = -EINVAL;
    if (!valid_signal(ureq->signal) && !valid_signal(-ureq->signal)) {
	CR_ERR_REQ(req, "invalid signal value: %d", ureq->signal);
	goto out_free_req;
    }
    req->signal = ureq->signal;
    
    /* Check/get the file descriptor */
    retval = cr_loc_init(eb, &req->src, ureq->cr_fd, filp, /* is_write= */ 0);
    if (retval) {
	CR_ERR_REQ(req, "failed to initialize source file descriptor");
        goto out_free_req;
    }
    if (req->src.fs) {
	CR_ERR_REQ(req, "source descriptor must not be a directory");
	retval = -EISDIR;
	goto out_free_req;
    }
    req->file0 = cr_loc_get(&req->src, NULL);
    if (IS_ERR(req->file0)) {
	retval = PTR_ERR(req->file0);
	req->file0 = NULL; /* wouldn't want to fput(ERRCODE) */
        goto out_free_req;
    }

    /* Read in the context file header */
    retval = cr_kread(eb, req->file0, &cf_header, sizeof(cf_header));
    if (retval != sizeof(cf_header)) {
	CR_ERR_REQ(req, "failed to read file header");
	goto out_free_req;
    }

    CR_KTRACE_HIGH_LVL("cr_magic = %d %d, " 
		       "cr_version = %d, "
		       "scope = %d, "
		       "arch = %d.",
		       cf_header.magic[0], cf_header.magic[1],
		       cf_header.version, cf_header.scope, cf_header.arch_id);

    // Check the header fields...
    retval = -EINVAL;
    {   // ... magic
        static int the_magic[2] = CR_MAGIC;
        if ((the_magic[0] != cf_header.magic[0]) ||
            (the_magic[1] != cf_header.magic[1])) {
	    CR_ERR_REQ(req, "file header has invalid signature");
	    goto out_free_req;
	}
    }
    // ... file format version
    if ((cf_header.version > CR_CONTEXT_VERSION) ||
        (cf_header.version < CR_CONTEXT_VERSION_MIN)) {
	CR_ERR_REQ(req, "file header has incorrect/unsupported version");
	goto out_free_req;
    }
    // ... architecture
    retval = -CR_EBADARCH;
    if (cf_header.arch_id != VMAD_ARCH) {
	CR_ERR_REQ(req, "file header has incorrect architecture (%d, but expecting %d)", cf_header.arch_id, VMAD_ARCH);
	goto out_free_req;
    }

    req->scope = cf_header.scope;  // Currently unused
    

    // add watchdog, which gets its own reference
    req->need_procs = 1;
    atomic_inc(&req->ref_count);
    cr_wd_add(&req->work);

    /* Normal case.  We keep the req, since we will need it again */
    return 0;

out_free_req:
    if (req->flags & CR_RSTRT_ASYNC_ERR) {
	req->result = retval;
	retval = 0;
    } else {
	release_rstrt_req(req);
	priv->rstrt_req = NULL;
    }
    return retval;


out:
    return retval;
}

// cr_rstrt_req(user_req)
//
// Processes a restart request received from user space.
//
// Returns 0 on sucess
// Returns negative on failure
// XXX: Return number of children to start when doing process groups?
int cr_rstrt_request_restart(struct file *filp, struct cr_rstrt_args __user *arg)
{
    struct cr_rstrt_args ureq;
    int retval;

    CR_KTRACE_FUNC_ENTRY();

    retval = -EFAULT;
    if (copy_from_user(&ureq, arg, sizeof(ureq))) {
        goto out;
    }

    retval = do_rstrt_request_restart(filp, &ureq, cr_read_reloc);

out:
    CR_KTRACE_FUNC_EXIT("Returning %d", retval);
    return retval;
}

static
int do_rstrt_procs(struct file *filp, struct cr_section_header *cf_header)
{
    cr_pdata_t *priv;
    cr_rstrt_req_t *req; 
    cr_errbuf_t *eb;
    int retval, threads;

    CR_KTRACE_FUNC_ENTRY();

    retval = -EINVAL;
    if (filp == NULL) {
        CR_ERR("%s: Called with NULL file structure", __FUNCTION__);
        goto out_noreq;
    }

    priv = filp->private_data;
    if (priv == NULL) {
        CR_ERR("%s: private_data is NULL!", __FUNCTION__);
        goto out_noreq;
    }

    req = priv->rstrt_req;
    if (req == NULL) {
	CR_ERR("%s: No rstrt_req attached to filp!", __FUNCTION__);
        goto out_noreq;
    }

    if (req->result) {
	// Already marked done
	retval = 0;
        goto out;
    }

    eb = req->errbuf;
    while (!req->die && (req->state == CR_RSTRT_STATE_CHILD) && !signal_pending(current)) {
	/* Wait for child to finish w/ the file */
	unsigned long timeout = MAX_SCHEDULE_TIMEOUT;
	timeout = wait_event_interruptible_timeout(req->wait, (req->die || (req->state != CR_RSTRT_STATE_CHILD)), timeout);
    }
    if (signal_pending(current)) {
	retval = -EINTR;
	goto out;
    } else if (req->die || (req->state == CR_RSTRT_STATE_DONE)) {
	req->need_procs = 0;
	retval = 0;
	goto out;
    }

    /* Read in the context header */
    retval = cr_kread(eb, req->file0, cf_header, sizeof(*cf_header));
    if (retval != sizeof(*cf_header)) {
	CR_ERR_REQ(req, "proc_header: read returned %d", retval);
	goto out;
    }

    threads = cf_header->num_threads;

    if (threads == 0  /* EOF */) {
	// XXX: additional serialization will be needed when restores are parallel
        retval = cr_restore_linkage(req);
        if (retval) {
	    req->die = 1;
	    goto out;
        }

        /* XXX A hack to return a "lead" pid (a tgid, actually).
         * Currently we look for the first task that is a child of cr_restart.
         * The assumption was that there
         * is a single "lead" to waitpid() for, which might not be the case.
         * Looking for session or process group leaders would be good in the
         * case of multiple children of cr-restart.
         */
        if (!req->result) { /* don't clobber an error indication */
            cr_linkage_entry_t *entry;
	    list_for_each_entry(entry, &req->linkage, list) {
                if (entry->link.parent == req->cr_restart_task) {
		    req->result = entry->link.tgid;
	            break;
                }
	    }
        }

	CR_KTRACE_HIGH_LVL("Kernel-level restore completed.");
	req->need_procs = 0;
	retval = 0;
    } else {
        cr_rstrt_proc_req_t *proc_req;

	CR_NO_LOCKS();
	proc_req = cr_kmem_cache_zalloc(*proc_req, cr_rstrt_proc_req_cachep, GFP_KERNEL);
	retval = -ENOMEM;
	if (!proc_req) {
	    goto out;
	}

        init_MUTEX(&proc_req->serial_mutex);
	init_waitqueue_head(&proc_req->wait);
	INIT_LIST_HEAD(&proc_req->tasks);
	INIT_LIST_HEAD(&proc_req->linkage);

        /* Set number of threads for this one. */
        proc_req->thread_count = threads;

        /* Now initialize the barrier count so we can block later */
        atomic_set(&proc_req->final_counter, threads);
        cr_barrier_init(&proc_req->pre_vmadump_barrier, threads);
        cr_barrier_init(&proc_req->post_vmadump_barrier, threads);
        cr_barrier_init(&proc_req->pre_complete_barrier, threads);
        cr_barrier_init(&proc_req->post_complete_barrier, threads);

	proc_req->req = req;
    	proc_req->file = req->file0;	// XXX: KLUDGE
	proc_req->clone_flags = cf_header->clone_flags;
	proc_req->tmp_fd = cf_header->tmp_fd;

	write_lock(&req->lock);
	list_add(&proc_req->list, &req->procs);
	write_unlock(&req->lock);

        /* Normal case.  */
        req->state = CR_RSTRT_STATE_CHILD;
        retval = 1;
    }

out:
    if (retval != 1) {
	cr_barrier_notify(&req->barrier);
    }
out_noreq:
    CR_KTRACE_FUNC_EXIT("Returning %d", retval);
    return retval;
}

// cr_rstrt_procs()
//
// Called by requester to get "next" process
//
// Returns > 0 if 1 or more processes remain, 0 if none.
// Returns negative on failure
int cr_rstrt_procs(struct file *filp, struct cr_procs_tbl __user *arg)
{
    struct cr_section_header cf_header;
    int retval;

    CR_KTRACE_FUNC_ENTRY();

    retval = do_rstrt_procs(filp, &cf_header);

    if (retval == 1) {
	/* Copy the process-header info to user space */
	if (!access_ok(VERIFY_WRITE, arg, sizeof(*arg)) ||
	    __put_user(cf_header.num_threads, &arg->threads) ||
	    __put_user(cf_header.clone_flags, &arg->clone_flags)) {
	    retval = -EFAULT;
        }
    }

    CR_KTRACE_FUNC_EXIT("Returning %d", retval);
    return retval;
}

// Check that a request is "done", from the point of view of the
// requestor.
// Returns >0 if completed, 0 if not completed, <0 on error
// Call w/o holding the locks
static
int check_done(cr_rstrt_req_t *req)
{
    int retval;

    write_lock(&req->lock);
    retval = list_empty(&req->tasks) && !req->need_procs;
    write_unlock(&req->lock);

    // Try to remove from watchdog list if done
    // This is only an optimization, not a correctness requirement
    if (retval && cr_wd_del(&req->work)) {
	// Release watchdog's ref_count.
	// The current task (the requester) must hold an additional reference.
	CRI_ASSERT(atomic_read(&req->ref_count) >= 2);
	(void)release_rstrt_req(req);
    }

    return retval;
}

// poll method for file w/ associated restart request
unsigned int
cr_rstrt_poll(struct file *filp, poll_table *wait)
{
	cr_pdata_t		*priv;
	cr_rstrt_req_t		*req;

	priv = filp->private_data;
	if (!priv) {
		return POLLERR;
	}
	req = priv->rstrt_req;
	if (!req) {
		return POLLERR;
	}

	poll_wait(filp, &req->wait, wait);

	return check_done(req) ? (POLLIN | POLLRDNORM) : 0;
}

// cr_rstrt_reap()
//
// Reap a completed restart.
// Returns:
// 	req->result on success (pid of restarted "leader")
// 	-EINVAL if no un-reaped restart request is associated w/ this fd
// 	-EAGAIN if the request is not completed
int
cr_rstrt_reap(struct file *filp)
{
    cr_pdata_t *priv;
    cr_rstrt_req_t *req;
    int retval;

    CR_KTRACE_FUNC_ENTRY();

    retval = -EINVAL;
    priv = filp->private_data;
    if (priv) {
	req = priv->rstrt_req;
	if (req) {
	    retval = -EAGAIN;
	    if (check_done(req)) {
		priv->rstrt_req = NULL;
		retval = req->result;
		(void) release_rstrt_req(req);
	    }
	}
    }

    CR_KTRACE_FUNC_EXIT("Returning %d", retval);
    return retval;
}

/*
 * cr_linkage_find_by_old_pid
 *
 * Returns a pointer to a linkage entry with l.pid == pid
 * NULL if not found
 */
cr_linkage_entry_t *
cr_linkage_find_by_old_pid(struct list_head *list_p, int pid)
{
    cr_linkage_entry_t *entry;

    list_for_each_entry(entry, list_p, list) {
        if (entry->link.pid == pid) {
	    goto out_found;
	}
    }
    entry = NULL;

out_found:
    return entry;
}

static void
cr_change_pid(struct task_struct *task, enum pid_type type, int nr) {
#if HAVE_2_6_0_ATTACH_PID
    detach_pid(task, type);
    attach_pid(task, type, nr);
#else
    struct pid *pid = cr_find_pid(nr);
    CRI_ASSERT(pid != NULL);
  #if HAVE_CHANGE_PID
    change_pid(task, type, pid);
  #elif HAVE_2_6_22_ATTACH_PID || HAVE_2_6_26_ATTACH_PID
    detach_pid(task, type);
    attach_pid(task, type, pid);
  #else
    #error "Unknown change_pid() or attach_pid() variant"
  #endif
#endif
}

#if !HAVE_REMOVE_LINKS
  #if defined(remove_parent)
    #define REMOVE_LINKS(task)	remove_parent(task)
    #define SET_LINKS(task)	add_parent(task)
  #else
    #define REMOVE_LINKS(task)	list_del_init(&(task)->sibling)
    #define SET_LINKS(task)	list_move_tail(&(task)->sibling, &CR_PARENT(task)->children)
  #endif
#endif

/* 
 * cr_restore_linkage
 *
 * NOTE:  Must only be called by a single thread.  This function acts on
 * every task contained in the restart request.  All others must be blocked
 * to prevent return to user space prior to entry into this function.
 *
 * Acquires and releases tasklist_lock held for writing.
 */
static int
cr_restore_linkage(cr_rstrt_req_t *req)
{
    int pid_flags = req->flags & CR_RESTORE_PIDS_MASK;
    cr_linkage_entry_t *entry;
    int retval = 0;

    CR_KTRACE_HIGH_LVL("Now restoring the ids and linkage... ");

    /* Add linkage entry for init */
    cr_insert_object(req->map, CR_PARENT_INIT, cr_child_reaper, GFP_KERNEL);

    write_lock(&req->lock);

    /*
     * Make a pass through to translate old task pointers to new ones
     */
    list_for_each_entry(entry, &req->linkage, list) {
	if (!entry->tl_task) {
	    /* No new task corresponding to this entry! */
	    CR_ERR_REQ(req, "ERROR: No task found for pid %d", entry->link.pid);
	    retval = -EINVAL;
	    goto out;
	}

#ifdef CR_REAL_PARENT
	entry->link.real_parent = cr_linkage_find_real_parent(req, entry->link.real_parent);
        entry->link.parent = entry->link.real_parent; /* XXX: until/unless we support ptrace */
#else
	entry->link.parent = cr_linkage_find_real_parent(req, entry->link.parent);
#endif
    }

    /* Now ensure nothing changes that might conflict */
    write_lock_irq(&tasklist_lock);

    /* Final chance to give up...
     *
     * Try to avoid a possible race against req->cr_restart_task exiting
     * before we get to restore the linkage
     * XXX: Note that right now current == cr_restart, but might not be true later.
     */
    if (cri_task_dead(req->cr_restart_task)) {
        CR_ERR_REQ(req, "Unrestorable linkage: cr_restart_task exited prematurely!");
        retval = -EINVAL; /* XXX: other? */
	goto out_write_unlock;	
    }

#if HAVE_PIDTYPE_TGID && HAVE_TASK_PIDS_PID_LINK
    if (pid_flags & CR_RSTRT_RESTORE_PID) {
	cr_linkage_entry_t *next;
	list_for_each_entry_safe(entry, next, &req->linkage, list) {
	    /* Need to ensure thread group leaders precede the rest of
	     * the threads so that link_pid() has something to link to.
	     */
	    if (entry->link.pid == entry->link.tgid) {
		list_move(&entry->list, &req->linkage);
	    }
	    /* Also need to unhash all tgids before rehashing any */
	    detach_pid(entry->tl_task, PIDTYPE_TGID);
	}
    }
#endif

    /* Now change the entire world... */
    list_for_each_entry(entry, &req->linkage, list) {
	struct task_struct *task = entry->tl_task;
	int is_leader = (entry->link.pid == entry->link.tgid);

	/* thread_group is already correct by virtue of how we create threads */
	CRI_ASSERT(is_leader == thread_group_leader(task));

	REMOVE_LINKS(task);
        CR_PARENT(task)      = entry->link.parent;
#ifdef CR_REAL_PARENT
	CR_REAL_PARENT(task) = entry->link.real_parent;
#endif
	SET_LINKS(task);

	if (pid_flags & CR_RSTRT_RESTORE_PID) {
	    cr_change_pid(task, PIDTYPE_PID, entry->link.pid);

#if HAVE_PIDTYPE_TGID
  #if HAVE_TASK_PIDS_PID
	    cr_change_pid(task, PIDTYPE_TGID, entry->link.tgid);
  #elif HAVE_TASK_PIDS_PID_LINK
	    if (is_leader) {
		CRI_ASSERT(HAVE_2_6_0_ATTACH_PID);
		attach_pid(task, PIDTYPE_TGID, entry->link.tgid);
	    } else {
		link_pid(task, task->pids + PIDTYPE_TGID,
			 &task->group_leader->pids[PIDTYPE_TGID].pid);
	    }
  #else
    #error "HAVE_PIDTYPE_TGID w/ unknown linkage type"
  #endif
#endif

	    task->tgid = entry->link.tgid;
    	    task->pid = entry->link.pid;
	} else {
	    // To help return correct tgid to requester
	    entry->link.tgid = task->tgid;
	}

	if (is_leader) {
	    if (pid_flags & CR_RSTRT_RESTORE_PGID) {
		cr_change_pid(task, PIDTYPE_PGID, entry->link.pgrp);
		cr_set_pgrp(task, entry->link.pgrp);
	    }
	    if (pid_flags & CR_RSTRT_RESTORE_SID) {
		cr_change_pid(task, PIDTYPE_SID, entry->link.session);
		cr_set_sid(task, entry->link.session);
		cr_task_leader(task) = entry->link.leader;
		cr_task_tty(task) = NULL;  // no CTTY
	    }
	}

	task->exit_signal = entry->link.exit_signal;
	/* XXX:  We don't handle these correctly yet */
#if 0
	task->tty_old_pgrp = entry->link.tty_old_pgrp;
#endif

	/* XXX: these exec_ids really aren't correct.
	 * XXX: If this changes, be sure to fix watchdog exec() detection too.
	 */
	if (is_leader) {
	    /* To look like we called exec(): */
	    task->parent_exec_id = req->cr_restart_task->self_exec_id;
	    task->self_exec_id = req->cr_restart_task->self_exec_id + 1;
	} else {
	    /* To look like we were fork()ed by leader, who called exec(): */
	    task->parent_exec_id = req->cr_restart_task->self_exec_id + 1;
	    task->self_exec_id = req->cr_restart_task->self_exec_id + 1;
	}
	entry->cr_task->self_exec_id = task->self_exec_id;

	entry->reserved = 0; /* We've consumed the reservation */
    }

out_write_unlock:
    write_unlock_irq(&tasklist_lock);

    /* on success retval is zero when we get here */

out:
    write_unlock(&req->lock);
    return retval;
}

/*
 * cr_restore_files_struct
 *
 * Restore the files_struct (file table).
 *
 * This function just expands our fd array to the correct size,
 * returning max_fds or error.
 *
 * In the future, if we need any information that applies to all opened files
 * that info can be placed here.
 */
static int
cr_restore_files_struct(cr_rstrt_proc_req_t *proc_req)
{
    cr_errbuf_t *eb = proc_req->req->errbuf;
    struct cr_files_struct cr_fs;
    struct file *cf_filp = proc_req->file;
    int retval = 0;

    retval = cr_kread(eb, cf_filp, &cr_fs, sizeof(cr_fs));
    if (retval != sizeof(cr_fs)) {
	CR_ERR_PROC_REQ(proc_req, "files_struct: read returned %d", retval);
	goto out;
    }

    retval = 0;
    if (cr_fs.cr_obj != cr_files_obj) {
	CR_ERR_PROC_REQ(proc_req, "cr_fs: Invalid cr_files_struct!");
	retval = -EINVAL;
	goto out;
    }

    /* check to see if we have too many files open */
    retval = -EMFILE;
    if (((unsigned long) cr_fs.cr_max_fds) > CR_RLIM(current)[RLIMIT_NOFILE].rlim_cur) {
        CR_KTRACE_HIGH_LVL("Too many files %d > %lu", cr_fs.cr_max_fds, CR_RLIM(current)[RLIMIT_NOFILE].rlim_cur);
	goto out;
    }

    spin_lock(&current->files->file_lock);

    retval = expand_files(current->files, cr_fs.cr_max_fds-1);
    if (retval < 0) {
	CR_ERR_PROC_REQ(proc_req, "Failed to resize the fd array. (err=%d)", retval);
	goto out_unlock;
    }

    CR_NEXT_FD(current->files,cr_fdtable(current->files)) = cr_fs.cr_next_fd;
    retval = cr_fs.cr_max_fds;
 
out_unlock:
    spin_unlock(&current->files->file_lock);
out:
    return retval;
}

/*
 * cr_load_file_info
 * 
 * Reads in a file_info from disk, returns it.
 */
static int
cr_load_file_info(cr_rstrt_proc_req_t *proc_req, struct cr_file_info *file_info)
{
    cr_errbuf_t *eb = proc_req->req->errbuf;
    struct file *cf_filp = proc_req->file;
    int retval;

    CR_KTRACE_FUNC_ENTRY();

    retval = cr_kread(eb, cf_filp, file_info, sizeof(*file_info));
    if (retval != sizeof(*file_info)) {
	CR_ERR_PROC_REQ(proc_req, "file_info: read returned %d", retval);
	goto out;
    }

    retval = -EINVAL;
    if (file_info->cr_type != cr_file_info_obj) {
        CR_ERR_PROC_REQ(proc_req, "cr_load_file_info: Garbage in context file! (type=%d)", file_info->cr_type);
	goto out;
    }

    retval = 0;

out:
    return retval;
}

/*
 * cr_open_fd: open a file - accepts file descriptor as argument
 * 
 * NOTE:  Does not attempt to expand the fdset array.
 * Use cr_restore_files_struct to expand fdset to the correct size.
 *
 * NOTE:  Does not check limits on number of open files.
 */ 
static struct file *
cr_open_fd(int fd, const char *path, int flags, int mode)
{
    int rc;
    struct file *filp;

    /* check to see if we have too many files open */
    filp = ERR_PTR(-EMFILE);
    if (fd >= CR_RLIM(current)[RLIMIT_NOFILE].rlim_cur)
	goto out;

#if 0 /* If we were paranoid */
    filp = ERR_PTR(-EMFILE);
    if (fd >= current->files->max_fds)
	goto out;
#endif

    /* Mark the fd in use */
    rc = cr_fd_claim(fd);
    /* make sure it's not in use already.  shouldn't ever happen */
    if (rc) {
        CR_WARN("File opened unexpectedly.");
	filp = ERR_PTR(rc);
        goto out;
    }

    /* open the file */
    filp = filp_open(path, flags, mode);
    if (IS_ERR(filp)) {
        goto out;
    }

    /* install the filp in the fd array -- note race see fd_install */
    fd_install(fd, filp);

out:
    return filp;
}

static int
cr_claim_install_fd(int fd, struct file *filp)
{
    int rc;

    rc = cr_fd_claim(fd);
    if (rc) {
        CR_WARN("File opened unexpectedly.");
    } else {
	fd_install(fd, filp);
    }

    return rc;
}

/* cr_set_f_pos
 *
 * simple wrapper around sys_lseek
 */
static inline int
cr_set_f_pos(int fd, loff_t offset)
{
    return (sys_lseek(fd, offset, 0) == offset) ? 0 : -EINVAL;
}

/*
 * cr_restore_open_file
 *
 * reopens files
 */
static int
cr_restore_open_file(cr_rstrt_proc_req_t *proc_req, struct cr_file_info *file_info, int do_not_restore)
{
    cr_errbuf_t *eb = proc_req->req->errbuf;
    int retval;
    const char *name;
    struct file *cf_filp = proc_req->file;
    struct file *filp;
    struct cr_open_file open_file;
    cr_rstrt_relocate_t reloc = proc_req->req->relocate;

    CR_KTRACE_FUNC_ENTRY();

#if 0
    /* XXX: Hack!!! We don't "dnr" files (see bug 1987 for why) */
    if (fcheck(file_info->fd) != NULL) {
	sys_close(file_info->fd);
    }
    do_not_restore = 0;
#endif

    /* read in the cr_open_file struct */
    retval = cr_kread(eb, cf_filp, &open_file, sizeof(open_file));
    if (retval != sizeof(open_file)) {
	CR_ERR_PROC_REQ(proc_req, "open_file: read returned %d", retval);
        goto out;
    }

    if (open_file.cr_type != cr_open_file_obj) {
        CR_ERR_PROC_REQ(proc_req, "cr_restore_open_file.  Garbage in context file!");
        retval = -EINVAL;
        goto out;
    }

    /* now read out the name */
    name = cr_getname(eb, reloc, cf_filp, 0);
    if (IS_ERR(name)) {
        CR_ERR_PROC_REQ(proc_req, "cr_restore_open_file - Bad pathname read!");
        retval = PTR_ERR(name);
        goto out;
    }
	
    /*do not handle /dev/properties for android2.3 */
    if(!strncmp(name,"/dev/__properties__",strlen("/dev/__properties__"))) {	    
	 retval = 0;
	 goto out_free;
    }
#if 1
    /* XXX: Hack!!! We don't "dnr" files (see bug 1987 for why) */
    if (fcheck(file_info->fd) != NULL) {
	sys_close(file_info->fd);
    }
    do_not_restore = 0;
#endif

    if (file_info->unlinked) {
	int found = cr_find_object(proc_req->req->map, open_file.file_id, (void **) &filp);
	if (found) {
	    /* An earlier restore has done the data, we need only open the unlinked file */
	    retval = 0;
	    if (do_not_restore) goto out_free;
	    filp = cr_filp_reopen(filp, open_file.f_flags);
	    retval = PTR_ERR(filp);
	    if (IS_ERR(filp)) goto out_free;
	} else {
	    /* Create and populate the unlinked file */
	    if (do_not_restore) {
		retval = cr_skip(cf_filp, open_file.i_size);
		goto out_free;
	    }
	    filp = cr_mkunlinked(eb, cf_filp, name, open_file.i_mode, open_file.f_flags, open_file.i_size, (unsigned long)open_file.file_id);
	    retval = PTR_ERR(filp);
	    if (IS_ERR(filp)) goto out_free;
	    cr_insert_object(proc_req->req->map, open_file.file_id, (void *) filp, GFP_KERNEL);
	}
	retval = cr_claim_install_fd(file_info->fd, filp);
    } else {
        retval = 0;
        if (do_not_restore) goto out_free;

        /* reopen file */
        filp = cr_open_fd(file_info->fd, name, open_file.f_flags, open_file.i_mode);
        if (IS_ERR(filp)) {
	    CR_ERR_PROC_REQ(proc_req, "Failed to open file '%s'", name);
	    retval = PTR_ERR(filp);
            goto out_free;
        }
        if (S_ISDIR(filp->f_dentry->d_inode->i_mode)) {
	    retval = -EISDIR;
            goto out_free;
        }

        /* restore previous size */
        if (!S_ISREG(filp->f_dentry->d_inode->i_mode)) {
	    /* Skip the truncation on non-regular files */
        } else if (filp->f_mode & FMODE_WRITE) {
            retval = sys_ftruncate(file_info->fd, open_file.i_size);
            if (retval < 0) {
                CR_ERR_PROC_REQ(proc_req, "Couldn't restore file length.");
	        goto out_free;
            }
        }
    }

    /* restore position in file */
    retval = cr_set_f_pos(file_info->fd, open_file.f_pos);
    if (retval < 0) {
        CR_ERR_PROC_REQ(proc_req, "Couldn't restore file pointer for file '%s'.", name);
	goto out_free;
    }

out_free:
    __putname(name);
out:
    return retval;
}

/*
 * cr_restore_open_dir
 *
 * reopens directories
 */
static int 
cr_restore_open_dir(cr_rstrt_proc_req_t *proc_req, struct cr_file_info *file_info, int do_not_restore)
{
    cr_errbuf_t *eb = proc_req->req->errbuf;
    int retval;
    const char *name;
    struct file *cf_filp = proc_req->file;
    struct file *filp;
    struct cr_open_dir open_dir;
    cr_rstrt_relocate_t reloc = proc_req->req->relocate;

    CR_KTRACE_FUNC_ENTRY();

#if 1
    /* XXX: Hack!!! We don't "dnr" dirs */
    if (fcheck(file_info->fd) != NULL) {
	sys_close(file_info->fd);
    }
    do_not_restore = 0;
#endif

    /* read in the cr_open_dir struct */
    retval = cr_kread(eb, cf_filp, &open_dir, sizeof(open_dir));
    if (retval != sizeof(open_dir)) {
	CR_ERR_PROC_REQ(proc_req, "open_dir: read returned %d", retval);
        goto out;
    }

    if (open_dir.cr_type != cr_open_dir_obj) {
        CR_ERR_PROC_REQ(proc_req, "cr_restore_open_dir.  Garbage in context file!");
        retval = -EINVAL;
        goto out;
    }

    /* now read out the name */
    name = cr_getname(eb, reloc, cf_filp, 0);
    if (IS_ERR(name)) {
        CR_ERR_PROC_REQ(proc_req, "cr_restore_open_dir - Bad pathname read!");
        retval = PTR_ERR(name);
        goto out;
    }

    if (do_not_restore) {
	/* XXX: Do we really want non-dir to ever replace dir? */
        retval = 0;
	goto out_free;
    }

    /* reopen directory */
    filp = cr_open_fd(file_info->fd, name, open_dir.f_flags, open_dir.i_mode);
    if (IS_ERR(filp)) {
	retval = PTR_ERR(filp);
        goto out_free;
    }
    if (!S_ISDIR(filp->f_dentry->d_inode->i_mode)) {
	retval = -ENOTDIR;
        goto out_free;
    }

    /* restore position */
    retval = cr_set_f_pos(file_info->fd, open_dir.f_pos);
    if (retval < 0) {
        CR_ERR_PROC_REQ(proc_req, "Couldn't restore file pointer for directory '%s'.", name);
	goto out_free;
    }

out_free:
    __putname(name);
out:
    return retval;
}

static int 
cr_restore_open_link(cr_rstrt_proc_req_t *proc_req, struct cr_file_info *file_info, int do_not_restore)
{
    int retval = -ENOSYS;

    CR_KTRACE_FUNC_ENTRY();

    if (do_not_restore) {
        retval = 0;
    }

    return retval;
}

static int 
cr_restore_open_socket(cr_rstrt_proc_req_t *proc_req, struct cr_file_info *file_info, int do_not_restore)
{
    int retval = -ENOSYS;

    CR_KTRACE_FUNC_ENTRY();

    if (do_not_restore) {
        retval = 0;
    }

    /* XXX:  Kludge to recover w/o restoring a socket */
    retval = 0;

    return retval;
}

static int 
cr_restore_open_chr(cr_rstrt_proc_req_t *proc_req, struct cr_file_info *file_info, int do_not_restore)
{
    cr_errbuf_t *eb = proc_req->req->errbuf;
    int retval = -ENOSYS;
    struct cr_chrdev cf_chrdev;
    struct file *cf_filp = proc_req->file;
    const char *path = NULL;

    CR_KTRACE_FUNC_ENTRY();

    /* read in the cr_chrdev struct */
    retval = cr_kread(eb, cf_filp, &cf_chrdev, sizeof(cf_chrdev));
    if (retval != sizeof(cf_chrdev)) {
	CR_ERR_PROC_REQ(proc_req, "open_chr: read returned %d", retval);
        goto out;
    }

    /* XXX:  Validate type */
    retval = -EINVAL;
    if (cf_chrdev.cr_type != cr_open_chr) {
        CR_ERR_PROC_REQ(proc_req, "cr_restore_open_chr: Garbage in context file.");
        goto out;
    }

    /* Handle any cases we know about.
     * In general, we don't support character devices unless we know they
     * are safe.
     */
    switch (cf_chrdev.cr_major) {
    case MEM_MAJOR: {
	static const char *tbl[] = { NULL,		/* 0 */
				     "/dev/mem",
				     "/dev/kmem",
				     "/dev/null",
				     "/dev/port",	/* 4 */
				     "/dev/zero",
				     "/dev/core",
				     "/dev/full",
				     "/dev/random",	/* 8 */
				     "/dev/urandom",
				     "/dev/aio",
				     "/dev/kmsg" };
	unsigned int minor = cf_chrdev.cr_minor;
	if (minor < (sizeof(tbl)/sizeof(char *))) {
	    path = tbl[minor];
	}
#if 1
	if (path != NULL) {
	    /* XXX: Hack!!! We don't "dnr" devs other than tty (see bug 2288 for why) */
	    if (fcheck(file_info->fd) != NULL) {
		sys_close(file_info->fd);
	    }
	    do_not_restore = 0;
	}
#endif
	break;
    }
    case TTYAUX_MAJOR:
	switch (cf_chrdev.cr_minor) {
	    case 0: path = "/dev/tty"; break;
	}
	break;
    }

    CR_KTRACE_LOW_LVL("   Character device.  major=%u, minor=%u path='%s'",
		      cf_chrdev.cr_major, cf_chrdev.cr_minor, path ? path : "(unknown)");

    retval = 0; /* Default is to silently ignore */

    if (do_not_restore) {
	goto out;
    }

    if (path != NULL) {
	struct file *file = cr_open_fd(file_info->fd, path, cf_chrdev.f_flags, cf_chrdev.i_mode);
	if (IS_ERR(file)) {
	    CR_ERR_PROC_REQ(proc_req, "Failed to open chrdev major=%u minor=%u path='%s')",
		    cf_chrdev.cr_major, cf_chrdev.cr_minor, path);
	    retval = PTR_ERR(file);
	}
    }

out:
    return retval;
}

static int 
cr_restore_open_blk(cr_rstrt_proc_req_t *proc_req, struct cr_file_info *file_info, int do_not_restore)
{
    int retval = -ENOSYS;

    CR_KTRACE_FUNC_ENTRY();

    if (do_not_restore) {
        retval = 0;
    }

    return retval;
}

static int
cr_dup2(cr_rstrt_req_t *req, struct file *old_filp, int fd)
{
    struct file *new_filp;
    int retval = 0;

    /* translate the filp to its new value */
    if (!cr_find_object(req->map, old_filp, (void **) &new_filp)) {
        CR_ERR_REQ(req, "cr_dup2: Could not locate restored filp %p.", old_filp);
	retval = -EINVAL;
        goto out;
    }
    if (new_filp == NULL) {
	/* The file was silently ignored before - continue to do so. */
        CR_WARN("cr_dup2: Skipping dup of ignored filp %p.", old_filp);
	goto out;
    }
    get_file(new_filp);

    /* allocate the fd */
    retval = cr_fd_claim(fd);
    if (retval) {
        /* this should never happen.  */
        CR_ERR_REQ(req, "cr_dup2: File descriptor %d in use.", fd);
	fput(new_filp);
        goto out;
    }

    /* install it */
    fd_install(fd, new_filp);

out:
    return retval;
}

static int 
cr_restore_open_dup(cr_rstrt_proc_req_t *proc_req, struct cr_file_info *file_info, int do_not_restore)
{
    cr_errbuf_t *eb = proc_req->req->errbuf;
    int retval = -ENOSYS;
    struct cr_dup cf_dup;
    struct file *cf_filp = proc_req->file;

    CR_KTRACE_FUNC_ENTRY();

    /* read in the cr_dup struct */
    retval = cr_kread(eb, cf_filp, &cf_dup, sizeof(cf_dup));
    if (retval != sizeof(cf_dup)) {
	CR_ERR_PROC_REQ(proc_req, "open_dup: read returned %d", retval);
        goto out;
    }

    /* Validate type (the only field we've got) */
    retval = -EINVAL;
    if (cf_dup.cr_type != cr_open_dup) {
        CR_ERR_PROC_REQ(proc_req, "cr_restore_open_dup: Garbage in context file.");
        goto out;
    }

    /* ALWAYS honor dups so that overrides will catch all instances together */
    if (fcheck(file_info->fd) != NULL) {
	sys_close(file_info->fd);
    }

    retval = cr_dup2(proc_req->req, file_info->orig_filp, file_info->fd);
    if (retval<0) {
        CR_ERR_PROC_REQ(proc_req, "cr_restore_open_dup: Could not cr_dup2(%p -> %d).", file_info->orig_filp, file_info->fd);
        goto out;
    }
    CR_KTRACE_LOW_LVL("cr_dup2(%p -> %d) successful", file_info->orig_filp, file_info->fd);

    retval = 0;

out:
    return retval;
}

static int
cr_restore_open_chkpt_req(cr_rstrt_proc_req_t *proc_req, struct cr_file_info *file_info, int do_not_restore)
{
    int retval;
    struct file *filp;
    cr_pdata_t *priv;

    CR_KTRACE_FUNC_ENTRY();
    retval = cr_restore_open_file(proc_req, file_info, do_not_restore);

    if (retval < 0) goto out;

    filp = fcheck(file_info->fd);
    if (!filp || IS_ERR(filp)) {
	CR_ERR_PROC_REQ(proc_req, "Bogus restore of ctrl descriptor");
	retval = filp ? PTR_ERR(filp) : -EINVAL;
	goto out;
    }

    priv = filp->private_data;
    if (!priv || priv->chkpt_req) {
	CR_ERR_PROC_REQ(proc_req, "Unexpected/invalid private data in restored ctrl descriptor");
	retval = -EINVAL;
	goto out;
    }

    priv->chkpt_req = CR_CHKPT_RESTARTED;

out:
    return retval;
}

static int 
cr_end_of_file_restore(cr_rstrt_proc_req_t *proc_req, struct cr_file_info *file_info)
{
    int retval = 0;

    CR_KTRACE_FUNC_ENTRY();

    /* Nothing to do here yet */

    return retval;
}

static int
cr_restore_file_locks(cr_rstrt_proc_req_t *proc_req, struct cr_file_info *file_info)
{
    int retval = -ENOSYS;

    CR_KTRACE_FUNC_ENTRY();

    /* Nothing to do here yet */

    return retval;
}

static int
cr_finish_all_file_restores(cr_rstrt_proc_req_t *proc_req, int max_fds)
{
    int retval = -ENOSYS;
    int fd;

    CR_KTRACE_FUNC_ENTRY();

    spin_lock(&current->files->file_lock);
    CR_KTRACE_LOW_LVL("current->files=%p", current->files);
    for(fd=0; fd<max_fds; ++fd) {
      struct file *filp;
      filp = fcheck(fd);
      if (filp) {
        CR_KTRACE_LOW_LVL("fd=%d filp=%p filp->dentry=%p d_count=%d d_inode=%p", fd, filp, filp->f_dentry, atomic_read(&filp->f_dentry->d_count), filp->f_dentry->d_inode);
      }
    }
    spin_unlock(&current->files->file_lock);

    return retval;
}

/*
 * cr_restore_all_files
 *
 * Main entry point for file recovery
 *
 * In the future, may want to set this so that it manipulates file "objects"
 * with load / restore methods
 */
static int
cr_restore_all_files(cr_rstrt_proc_req_t *proc_req)
{
    int do_not_restore_flag;
    int i, retval, max_fds;
    struct cr_file_info file_info;
    struct files_struct *files = current->files;
    struct file *filp;
    cr_fdtable_t *fdt;
	char tmp[20];
    /* close-on-exec() of caller's fds before we start restoring */
    CR_KTRACE_HIGH_LVL("close-on-exec of callers files");
    spin_lock(&files->file_lock);
    fdt = cr_fdtable(files);
    for (i = 0; i < fdt->max_fds; ++i) {
	if (FD_ISSET(i, fdt->close_on_exec)) {
		
		spin_unlock(&files->file_lock);
	  sys_close(i);
	  spin_lock(&files->file_lock);
	  fdt = cr_fdtable(files);
		
        }
    }
    spin_unlock(&files->file_lock);

	
    /* load the files struct */
    CR_KTRACE_HIGH_LVL("recovering fs_struct...");
    retval = cr_restore_files_struct(proc_req);
    if (retval < 0) {
        CR_ERR_PROC_REQ(proc_req, "Unable to cr_restore_files_struct");
        goto out;
    }

	max_fds = retval;

    file_info.cr_file_type = cr_bad_file_type;
    /* now do per file stuff */
    while (1) {
        retval = cr_load_file_info(proc_req, &file_info);
	if (retval < 0) {
            CR_ERR_PROC_REQ(proc_req, "Error loading file_info.");
            goto out;
	}


	if (file_info.cr_file_type == cr_end_of_files) {
            /* finished */
            retval = cr_end_of_file_restore(proc_req, &file_info);
            break;
	}

	/* Take first-cut at dnr flag.  Note some cases will ignore this:
	 * 1) dups always follow the dupped fd
	 * 2) internal pipes (those w/ both a reader and writter in the checkpoint)
	 * 3) normal files currently ignore dnr (for bug 1987)
	 */
        do_not_restore_flag = (fcheck(file_info.fd) != NULL);

        CR_KTRACE_LOW_LVL("   fd=%d dnr=%d", file_info.fd, do_not_restore_flag);

        switch(file_info.cr_file_type) {
        case cr_open_chkpt_req:
            retval = cr_restore_open_chkpt_req(proc_req, &file_info, do_not_restore_flag);
            break;
        case cr_open_file:
            retval = cr_restore_open_file(proc_req, &file_info, do_not_restore_flag);
            break;
        case cr_open_directory:
            retval = cr_restore_open_dir(proc_req, &file_info, do_not_restore_flag);
            break;
        case cr_open_link:
            retval = cr_restore_open_link(proc_req, &file_info, do_not_restore_flag);
            break;
        case cr_open_fifo:
            retval = cr_restore_open_fifo(proc_req, &file_info, do_not_restore_flag);
            break;
        case cr_open_socket:
            retval = cr_restore_open_socket(proc_req, &file_info, do_not_restore_flag);
            break;
        case cr_open_chr:
            retval = cr_restore_open_chr(proc_req, &file_info, do_not_restore_flag);
            break;
        case cr_open_blk:
            retval = cr_restore_open_blk(proc_req, &file_info, do_not_restore_flag);
            break;
        case cr_open_dup:
            retval = cr_restore_open_dup(proc_req, &file_info, do_not_restore_flag);
            break;
        case cr_bad_file_type:
            /* fall through */
        default:
            CR_ERR_PROC_REQ(proc_req, "Unknown file type fd=%d type=%d", file_info.fd, file_info.cr_file_type);
            retval = -EBADF;
            goto out;
            break;
        }

        if (retval < 0) {
            CR_ERR_PROC_REQ(proc_req, "%s [%d]:  Unable to restore fd %d (type=%d,err=%d)",
                   __FUNCTION__, current->pid, file_info.fd, file_info.cr_file_type, retval);
            goto out;
        }

	/* Restore close-on-exec flag */
        spin_lock(&files->file_lock);
	fdt = cr_fdtable(files);
        if (file_info.cloexec) {
	    FD_SET(file_info.fd, fdt->close_on_exec);
        } else {
	    FD_CLR(file_info.fd, fdt->close_on_exec);
	}
	spin_unlock(&files->file_lock);

        /* now acquire locks (unimplemented) */
        cr_restore_file_locks(proc_req, &file_info);

	spin_lock(&files->file_lock);
	filp = fcheck(file_info.fd);
	spin_unlock(&files->file_lock);
        cr_insert_object(proc_req->req->map, file_info.orig_filp, filp, GFP_KERNEL);
    };

    /* now for the second pass.  The above restore routines should have taken
     * care of loading all the data.  the easy work, now we do everything 
     * that has to be deferred until we have all the data
     */
    cr_finish_all_file_restores(proc_req, max_fds);

	spin_lock(&files->file_lock);
	fdt = cr_fdtable(files);

	spin_unlock(&files->file_lock);

out:
    return retval;
} 

/*
 * cr_chdir
 *
 * Our own version of the chdir() system call.  Differs in that it accepts
 * pathnames in kernel space, and calls path_init, path_walk, and path_release
 * directly.
 * 
 */
static int
cr_chdir(const char *path)
{
    struct nameidata nd;
    int retval;

    /* lookup the path */
    retval = path_lookup(path,LOOKUP_FOLLOW|LOOKUP_DIRECTORY,&nd);
    if (retval)
        goto out;

    retval = cr_permission(nd.nd_dentry->d_inode,MAY_EXEC,&nd);
    if (retval)
        goto dput_and_out;

    cr_set_pwd_nd(current->fs, &nd);

dput_and_out:
    cr_path_release(&nd);
out:
    return retval;
}

/*
 * cr_restore_fs
 *
 * This function does the work of restoring the data in our fs_struct, i.e.
 * the current working directory, and the umask.
 *
 * In the future, this may be modified to also restore a process after a
 * chroot(), if desirable.
 * 
 */
static int
cr_restore_fs_struct(cr_rstrt_proc_req_t *proc_req)
{
    cr_errbuf_t *eb = proc_req->req->errbuf;
    int retval;
    int umask;
    const char *name;
    struct file *cf_filp = proc_req->file;
    cr_rstrt_relocate_t reloc = proc_req->req->relocate;

    /* restore the umask */
    retval = cr_kread(eb, cf_filp, &umask, sizeof(umask));
    if (retval < sizeof(umask)) {
	CR_ERR_PROC_REQ(proc_req, "umask: read returned %d", retval);
        goto out_readerr;
    }

    /* read in root */
    name = cr_getname(eb, reloc, cf_filp, 0);
    if (IS_ERR(name)) {
        CR_ERR_PROC_REQ(proc_req, "root: Bad file read!");
        goto out_badname;
    }
    /* should change root directory here. */
    /* cr_chroot() */
    __putname(name);

    /* read in the current working directory */
    name = cr_getname(eb, reloc, cf_filp, 0);
    if (IS_ERR(name)) {
        CR_ERR_PROC_REQ(proc_req, "pwd: Bad file read!");
        goto out_badname;
    }

    /* now change the current working directory */
    retval = cr_chdir(name);
    if (retval < 0) {
        CR_ERR_PROC_REQ(proc_req, "cr_chdir:  Unable to change directory to %s", name);
	goto out_put;
    }    
out_put:
    __putname(name);
    return retval;

out_badname:
    retval = PTR_ERR(name);
out_readerr:
    return retval;
}

/*
 * This does half of a close.  It removes the fd from a files_struct, but doesn't
 * actually close the fd.
 */
int
cr_hide_filp(struct file *filp, struct files_struct *files)
{
    cr_fdtable_t *fdt;
    int found;
    int fd;

    found = 0;

    spin_lock(&files->file_lock);
    fdt = cr_fdtable(files);
    for (fd = 0; fd < fdt->max_fds; ++fd) {
	if (filp == fdt->fd[fd]) {
		
	    ++found;
            rcu_assign_pointer(fdt->fd[fd], NULL);
	    FD_CLR(fd, fdt->close_on_exec);
	    break;
	}
    }
    spin_unlock(&files->file_lock);

    if (found) {
	put_unused_fd(fd);
    } else {
	CR_ERR("File descriptor closed unexpectedly!");
    }

    return found;
}

static void cr_signal_rstrt_complete_barrier(cr_task_t *cr_task, int block) {
	cr_rstrt_proc_req_t *proc_req = cr_task->rstrt_proc_req;
	cr_rstrt_req_t *req = proc_req->req;

	CR_ASSERT_STEP_EQ(cr_task, CR_RSTRT_STEP_PRE_COMPLETE); 
	CR_BARRIER_NOTIFY(cr_task, &proc_req->pre_complete_barrier);

	if (cr_barrier_once_interruptible(&proc_req->pre_complete_barrier, block) > 0) {
		struct task_struct *task = cr_task->task;
		int signal = 0;

		// close the temporary fd unless we are failing or called by watchdog
		if ((proc_req->tmp_fd >= 0) && !req->die && (task == current)) {
			int rc = sys_close(proc_req->tmp_fd);
			if (rc) {
				CR_ERR_PROC_REQ(proc_req, "Failed to close tmp_fd (err=%d)", rc);
			}
		}
		proc_req->tmp_fd =  -1;

		// raise a signal if needed
		if (req->die) {
			signal = SIGKILL;
		} else if ((req->signal > 0) && (req->result >= 0)) {
			signal = req->signal;
		} else if (cr_task->stopped) {
			signal = SIGSTOP;
		}
		if (signal) {
			cr_kill_process(cr_task->task, signal);
		}
	}
}

// Must be called w/ req->lock held or when exclusive
// access to the request list is otherwise guaranteed.
static void cr_rstrt_advance_to(cr_task_t *cr_task, int end_step) {
	cr_rstrt_proc_req_t *proc_req = cr_task->rstrt_proc_req;
	cr_rstrt_req_t *req = proc_req->req;

	CR_ASSERT_STEP_LE(cr_task, end_step);
	switch (cr_task->step) {
	case CR_RSTRT_STEP_PRE_VMADUMP:
		if (cr_task->step == end_step) break;
		CR_BARRIER_NOTIFY(cr_task, &proc_req->pre_vmadump_barrier);
		// fall through...
	case CR_RSTRT_STEP_POST_VMADUMP:
		if (cr_task->step == end_step) break;
		CR_BARRIER_NOTIFY(cr_task, &proc_req->post_vmadump_barrier);
		// fall through...
	case CR_RSTRT_STEP_REQ_BARRIER:
		if (cr_task->step == end_step) break;
		CR_BARRIER_NOTIFY(cr_task, &req->barrier);
		// fall through...
	case CR_RSTRT_STEP_PRE_COMPLETE:
		if (cr_task->step == end_step) break;
		cr_signal_rstrt_complete_barrier(cr_task, /* block= */0);
		// fall through...
	case CR_RSTRT_STEP_POST_COMPLETE:
		if (cr_task->step == end_step) break;
		CR_BARRIER_NOTIFY(cr_task, &proc_req->post_complete_barrier);
		// fall through...
	case CR_RSTRT_STEP_DONE:
		break;

	default:
		CR_ERR_REQ(req, "Invalid cr_task->step %d", cr_task->step);
	}
}

// __delete_from_req(req, cr_task)
// Remove a given task from the request lists, waking waiter(s) if needed.
//
// Must be called w/ cr_task_lock held for writing.
// Must be called w/ req->lock held for writing.
static void __delete_from_req(cr_rstrt_req_t *req, cr_task_t *cr_task)
{
    list_del_init(&cr_task->req_list);
    list_del_init(&cr_task->proc_req_list);
    if (list_empty(&req->tasks)) {
        wake_up(&req->wait);
    }
    cr_task->rstrt_req = NULL;
    cr_task->rstrt_proc_req = NULL;
    cr_task->step = 0;
    __cr_task_put(cr_task);
}

// Must be called w/ cr_task_lock held for writing.
// Must be called w/ req->lock held for writing.
static void delete_dead_task(cr_rstrt_req_t *req, cr_task_t *cr_task)
{
    cr_rstrt_advance_to(cr_task, CR_RSTRT_STEP_DONE);
    __delete_from_req(req, cr_task);
    release_rstrt_req(req);
}

// "lookup" a cr_task for current
static cr_task_t *
lookup_task(cr_rstrt_req_t *req)
{
    cr_rstrt_proc_req_t *proc_req, *tmp;
    cr_task_t *result = NULL;
    const struct mm_struct *mm = current->mm;

    write_lock(&cr_task_lock);
    write_lock(&req->lock);

    // Find the corresponding proc_req
    proc_req = NULL;
    list_for_each_entry(tmp, &req->procs, list) {
	if (tmp->mm == mm) {
	    proc_req = tmp;
	    break;
	} else if (tmp->mm == NULL) {
	    proc_req = tmp; // "wildcard"
	}
    }
    if (!proc_req) {
	CR_ERR_REQ(req, "%s: No rstrt_proc_req for this thread!", __FUNCTION__);
    	result = ERR_PTR(-EINVAL);
	goto out;
    }
    proc_req->mm = mm; // In case match was the wildcard

    // Get an entry for the task
    result = __cr_task_get(current, 1);
    if (result == NULL) {
	result = ERR_PTR(-ENOMEM);
	goto out;
    }
    result->rstrt_proc_req = proc_req;
    result->self_exec_id = current->self_exec_id;

    // Close a race against checkpoint requests
    // AFTER setting result->rstrt_proc_req to prevent new requests
    // BEFORE adding to the task lists (which are also used by the chkpt req)
    if (result->chkpt_proc_req) {
	CR_KTRACE_LOW_LVL("Omitting restart child from checkpoint");
	write_unlock(&req->lock);
	write_unlock(&cr_task_lock);

	cr_chkpt_abort(result, CR_CHECKPOINT_OMIT);
	CR_SIGNAL_LOCK(current);
	sigaddset(&current->blocked, CR_SIGNUM);
	recalc_sigpending();
	CR_SIGNAL_UNLOCK(current);

	write_lock(&cr_task_lock);
	write_lock(&req->lock);
    }

    // Add to the lists of tasks
    list_add_tail(&result->proc_req_list, &proc_req->tasks);
    list_add_tail(&result->req_list, &req->tasks);
    req->need_procs = 0;
out:
    write_unlock(&req->lock);
    write_unlock(&cr_task_lock);

    return result;
}

// cr_rstrt_task_complete
//
// Cleanup
int cr_rstrt_task_complete(cr_task_t *cr_task, int block, int need_lock)
{
    cr_rstrt_proc_req_t *proc_req = cr_task->rstrt_proc_req;
    cr_rstrt_req_t *req = proc_req->req;

    cr_signal_rstrt_complete_barrier(cr_task, block);
    CR_BARRIER_NOTIFY(cr_task, &proc_req->post_complete_barrier);
    if (block) (void)cr_barrier_wait_interruptible(&proc_req->post_complete_barrier);
    CR_ASSERT_STEP_EQ(cr_task, CR_RSTRT_STEP_DONE); 

    if (need_lock) { write_lock(&cr_task_lock); }
    write_lock(&req->lock);
    __delete_from_req(req, cr_task);
    write_unlock(&req->lock);
    if (need_lock) { write_unlock(&cr_task_lock); }
    release_rstrt_req(req);

    return 0;
}

// cr_rstrt_child
//
// Invoked by a freshly created child process.
//
// The real "restart" part of reconstructing a process image happens here.
//
// Returns pid of cr_restart on success, or a negative on failure
int cr_rstrt_child(struct file *filp)
{
    cr_pdata_t *priv;
    cr_rstrt_req_t *req; 
    cr_rstrt_proc_req_t *proc_req; 
    cr_errbuf_t *eb;
    cr_task_t *cr_task = NULL;
    int retval;
    int old_pid = -1;
    cr_linkage_entry_t *my_linkage = NULL;
    unsigned long chkpt_flags = 0;

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

    req = priv->rstrt_req;
    if (req == NULL) {
	CR_ERR("%s: No rstrt_req attached to filp!", __FUNCTION__);
        goto out;
    }

    eb = req->errbuf;
    cr_task = lookup_task(req);
    if (IS_ERR(cr_task)) {
	retval = PTR_ERR(cr_task);
        goto out;
    }
    proc_req = cr_task->rstrt_proc_req;

    /* Forget any past registrations */
    if (cr_task->filp) {
	cr_task->phase = CR_NO_PHASE;
	cr_task->filp = NULL;
	cr_task->fd = -1;
	memset(&cr_task->handler_sa, 0, sizeof(cr_task->handler_sa));
	cr_task_put(cr_task); // The ref from the registration
    }

    atomic_inc(&req->ref_count);
    atomic_inc(&req->barrier.count);

    CR_ASSERT_STEP_EQ(cr_task, CR_RSTRT_STEP_PRE_VMADUMP); 
    CR_BARRIER_NOTIFY(cr_task, &proc_req->pre_vmadump_barrier);

    down(&proc_req->serial_mutex);
    if (req->die) {
	up(&proc_req->serial_mutex);
	goto out_release;
    }

    /* Initialize parts of req relevant to restoring task linkage */
    if (!test_and_set_bit(0, &proc_req->done_linkage)) {
	/* First load the linkage */
	retval = cr_load_linkage(proc_req);
        if (retval < 0) {
            CR_ERR_PROC_REQ(proc_req, "Error reading linkage structure! err=%d", retval);
	    retval = -ENOMEM;
	    req->die = 1;
	    up(&proc_req->serial_mutex);
	    goto out_release;
        }

        /* Now we reserve all the ids (pid, pgid, sid) we will need.
         * Note that this can get_zeroed_page(), and thus no locks are held yet.
         */
        retval = cr_reserve_ids(proc_req);
        if (retval) {
	    req->die = 1;
	    up(&proc_req->serial_mutex);
	    goto out_release;	
	}

	/* Move this process's linkage to the global list */
	write_lock(&req->lock);
        list_splice_init(&proc_req->linkage, &req->linkage);
	write_unlock(&req->lock);
    }
    up(&proc_req->serial_mutex);

    /* Before thaw can raise signals, make sure we've blocked them all.
     * The thaw step will replace this with the proper mask later.
     */
    {
	sigset_t sig_blocked;
	siginitsetinv(&sig_blocked, sigmask(SIGKILL)|sigmask(SIGSTOP));
	sigprocmask(SIG_SETMASK, &sig_blocked, NULL);
    }

    /* Must wait until all threads have safely left user space before thaw munmap()s. */
    retval = cr_barrier_wait_interruptible(&proc_req->pre_vmadump_barrier);
    if (retval < 0) {
	req->die = 1;
        goto out_release;
    }

    /* Thaw thyself out (called by all threads) */
    {
	/* Distinguish a single thread group leader (no race here) */
	int i_am_leader = (thread_group_leader(current) && !test_and_set_bit(0, &proc_req->have_leader));

        long err = cr_thaw_threads(proc_req, 0, i_am_leader);
	if (err < 0) {
	    CR_ERR_PROC_REQ(proc_req, "thaw_threads returned error, aborting. %ld", err);
	    if (req->result >= 0) req->result = err;
	    req->die = 1;
            goto out_release;
	} else {
	    old_pid = err;
	}

	/* Need a barrier here to ensure all threads read their regs
	 * before the next read from the file.  We notify here, and
	 * will wait before the next read.
	 */
	CR_ASSERT_STEP_EQ(cr_task, CR_RSTRT_STEP_POST_VMADUMP); 
	CR_BARRIER_NOTIFY(cr_task, &proc_req->post_vmadump_barrier);

	retval = 0;
    }

    /* Associate your old PID with your task in the linkage */
    my_linkage = cr_linkage_find_by_old_pid(&req->linkage, old_pid);

    /* Map old->new task pointers for linkage restore (all threads) */
    if (my_linkage == NULL) {
        CR_ERR_PROC_REQ(proc_req, "%s [%d]: ERROR:  No linkage entry for my PID %d",
	       __FUNCTION__, current->pid, old_pid);
	/*
	 * NOTE:  We DO NOT ABORT here.  Instead, we wait for the linkage to be restored,
	 * and rely on the cr_restore_linkage code to notice the problem (a NULL tl_task).
	 */
	cr_task->stopped = 0;
    } else {
	get_task_struct(current);
	cr_insert_object(req->map, my_linkage->link.myptr, current, GFP_KERNEL);
        my_linkage->tl_task = current;
        my_linkage->cr_task = cr_task;
	cr_task->stopped = my_linkage->link.cr_task.stopped;
	chkpt_flags = my_linkage->link.cr_task.chkpt_flags;
    }

    /* Log for debugging (or other use) */
    CR_KTRACE_HIGH_LVL("Formerly %s PID %d", current->comm, old_pid);

    /* Finish the post_vmadump barrier here, before files code can read context file */
    retval = cr_barrier_wait_interruptible(&proc_req->post_vmadump_barrier);
    if (retval < 0) {
	req->die = 1;
        goto out_release;
    }

    /* --- File recovery starts here --- */
    /* Exactly one thread should close the rstrt_req file descriptor(s)
     * but they should all wait until this has been done. */
    /* NOTE:  Past this point, you must jump to out_close to abort.  We are removing
     * our reference to the fd, which means do_exit() can't clean up after us. */ 

    down(&proc_req->serial_mutex);
    if (req->die) {
	up(&proc_req->serial_mutex);
	goto out_put_task;
    }

    if (!test_and_set_bit(0, &proc_req->done_hide_cr_fds)) {
        /* Remove the rstrt_req descriptor from our open files. */
        cr_hide_filp(filp, current->files);
        cr_hide_filp(proc_req->file, current->files);
    }

    /* restore fs_struct */
    /* XXX:  put the bit tests inside the restore routines when we do process
     * groups and sessions.  This way, they can just decide they want to skip it.
     */
    if (!test_and_set_bit(0, &proc_req->done_fs)) {
        retval = cr_restore_fs_struct(proc_req);
        if (retval < 0) {
            CR_ERR_PROC_REQ(proc_req, "%s [%d]:  Unable to restore fs!  (err=%d)",
                   __FUNCTION__, current->pid, retval);
	    if (req->result >= 0) req->result = retval;
	    req->die = 1;
            goto out_close;
            // XXX:  same concern
        }
    }

    /* restore special mmap()s (but not yet the pages) */
    if (!test_and_set_bit(0, &proc_req->done_mmaps_maps)) {
	CR_KTRACE_LOW_LVL("%d: loading mmap()s table", current->pid);
        retval = cr_load_mmaps_maps(proc_req);
        if (retval < 0) {
            CR_ERR_PROC_REQ(proc_req, "%s [%d]:  Unable to load mmap()s table!  (err=%d)",
                   __FUNCTION__, current->pid, retval);
	    if (req->result >= 0) req->result = retval;
	    req->die = 1;
	    goto out_close;
        }
    }

    /* load POSIX interval timers (but don't yet activate) */
    if (!test_and_set_bit(0, &proc_req->done_itimers)) {
	CR_KTRACE_HIGH_LVL("Reading POSIX interval timers...");
	retval = cr_load_itimers(proc_req);
	if (retval < 0) {
	    if (req->result >= 0) req->result = retval;
	    req->die = 1;
	    goto out_close;
	}
    }

    /* restore special mmap()ed pages */
    if (!test_and_set_bit(0, &proc_req->done_mmaps_data)) {
        CR_KTRACE_HIGH_LVL("Reading mmap()ed pages (if any)...");
        retval = cr_load_mmaps_data(proc_req);
        if (retval < 0) {
            CR_ERR_PROC_REQ(proc_req, "%s [%d]:  Unable to load mmap()ed data!  (err=%d)",
                   __FUNCTION__, current->pid, retval);
	    if (req->result >= 0) req->result = retval;
	    req->die = 1;
	    goto out_close;
        }
    }

    /* restore the open files */
    if (!test_and_set_bit(0, &proc_req->done_files)) {
        retval = cr_restore_all_files(proc_req);
        if (retval < 0) {
            CR_ERR_PROC_REQ(proc_req, "%s [%d]:  Unable to restore files!  (err=%d)",
                   __FUNCTION__, current->pid, retval);
	    if (req->result >= 0) req->result = retval;
	    req->die = 1;
            goto out_close;
            // XXX:  same concern
        }
    }

    /* close the request descriptor */
    /* XXX: Need to handle differently when we split to multiple files? */
out_close:
    if (!test_and_set_bit(0, &proc_req->done_close_cr_fds)) {
	int rc;	/* don't overwrite error code w/ 0 */
	CR_KTRACE_LOW_LVL("%d: closing request descriptor", current->pid);
	rc = filp_close(filp, current->files);
	if (rc < 0) {
	    CR_ERR_PROC_REQ(proc_req, "Error closing checkpoint control descriptor!  "
		   "(err=%d)", rc);
	    retval = rc;
	    if (req->result >= 0) req->result = retval;
	    req->die = 1;
	    goto out_mutex;
	}

	CR_KTRACE_LOW_LVL("%d: closing context file descriptor", current->pid);
	rc = filp_close(proc_req->file, current->files);
	if (rc < 0) {
	    CR_ERR_PROC_REQ(proc_req, "Error closing context file descriptor!"
		   "(err=%d)", rc);
	    retval = rc;
	    if (req->result >= 0) req->result = retval;
	    req->die = 1;
	    goto out_mutex;
	}
    }

    retval = 0;
out_mutex:
    up(&proc_req->serial_mutex);

    /* restore handlers (if any)  (all threads) */
    if (!req->die) {
	/* XXX: move to cr_rstrt_task_complete()? */
	if ((my_linkage != NULL) && (my_linkage->link.cr_task.phase != CR_NO_PHASE)) {
    	    if (!cr_find_object(req->map, my_linkage->link.cr_task.orig_filp, (void **) &cr_task->filp)) {
		CR_ERR_PROC_REQ(proc_req, "unable to locate checkpoint control descriptor");
	        if (req->result >= 0) req->result = retval;
	        req->die = 1;
                goto out_put_task;
	    }
	    cr_task->phase = my_linkage->link.cr_task.phase;
	    cr_task->fd = my_linkage->link.cr_task.fd;
	    cr_task->handler_sa = my_linkage->link.cr_task.handler_sa;
	    atomic_inc(&cr_task->ref_count);
	}
    }
	    
out_put_task:
    if (my_linkage) {
        /* Balance the get_task_struct() call made when task was added to object map */
	put_task_struct(current);
    }
out_release:
    /* Signal any barriers we skipped due to error path gotos */
    cr_rstrt_advance_to(cr_task, CR_RSTRT_STEP_REQ_BARRIER);

    /* On failure we need to ensure everyone fails, and inform the requester */
    if ((req->result >= 0) && (retval < 0)) {
	req->result = retval;
    }
    if (retval == 0) {
        /* Return > zero for success restart.  We return the pid of cr_restart, which we want anyway. */
        retval = req->requester;
    }

    /* done reading the context file - release it back to the requester */
    if (atomic_dec_and_test(&proc_req->final_counter)) {
        req->state = CR_RSTRT_STATE_REQUESTER;
        wake_up(&req->wait);
    }

    /* Wait for the requester and all tasks to reach end-of-file */
    CR_ASSERT_STEP_EQ(cr_task, CR_RSTRT_STEP_REQ_BARRIER); 
    CR_BARRIER_NOTIFY(cr_task, &req->barrier);
    if (!req->die) {
        cr_barrier_wait(&req->barrier);
    }

    // One task resumes the itimers (and "early" signals)
    if (!test_and_set_bit(0, &proc_req->done_resume_itimers)) {
	cr_resume_itimers(proc_req->itimers);
	if (req->signal < 0) {
	    cr_kill_process(current, -(req->signal));
	}
    }

    CR_ASSERT_STEP_EQ(cr_task, CR_RSTRT_STEP_PRE_COMPLETE); 

    if (req->die || (chkpt_flags & _CR_CHECKPOINT_STUB)) {
        // task has no handlers, so finish up now.
	cr_rstrt_task_complete(cr_task, /* block = */ 1, /* need_lock = */ 1);
    }
out:
    CR_KTRACE_FUNC_EXIT("Returning %d", retval);
    return retval;
}

int cr_rstrt_req_release(struct file *filp, cr_pdata_t *priv)
{
    cr_rstrt_req_t *req;

    // If we are a requester, release the outstanding request.
    req = priv->rstrt_req;
    if (req) {
        (void)release_rstrt_req(req);
    }

    return 0;
}

int cr_rstrt_src(struct file *filp, char *ubuf)
{
    cr_task_t *cr_task;
    cr_rstrt_proc_req_t *proc_req;
    cr_rstrt_req_t *req;
    char *buf;
    const char *path = NULL;
    int retval;

    CR_KTRACE_FUNC_ENTRY();

    retval = -ENOMEM;
    buf = __getname();
    if (!buf) {
    	goto out_nobuf;
    }

    retval = -ESRCH;
    cr_task = cr_task_get(current);
    if (!cr_task) {
	goto out;
    }
    proc_req = cr_task->rstrt_proc_req;
    cr_task_put(cr_task);
    if (!proc_req) {
	goto out;
    }
    req = proc_req->req;
    if (!req) {
	goto out;
    }

    path = cr_location2path(&(req->src), buf, PATH_MAX);
    retval = 0;
    if (IS_ERR(path)) {
        retval = PTR_ERR(path);
    } else if (!path) {
        retval = -EBADF;
    } else if (copy_to_user(ubuf, path, 1+strlen(path))) {
        retval = -EFAULT;
    }

out:
    __putname(buf);
out_nobuf:

    return retval;
}

// rstrt_watchdog(req)
//
// Called periodically to cleanup after zombies in a restart request
//
static void rstrt_watchdog(cr_work_t *work)
{
    cr_rstrt_req_t *req = container_of(work, cr_rstrt_req_t, work);
    cr_task_t *cr_task, *next;
    int finished;

    write_lock(&cr_task_lock);
    write_lock(&req->lock);

    list_for_each_entry_safe(cr_task, next, &req->tasks, req_list) {
	struct task_struct *task = cr_task->task;
	if ((task->self_exec_id - cr_task->self_exec_id) > 1) {
		CR_WARN_PROC_REQ(cr_task->rstrt_proc_req,
			"%s: tgid/pid %d/%d exec()ed '%s' during restart",
			__FUNCTION__, task->tgid, task->pid, task->comm);
		// Fall through to delete_dead_task()
	} else if (cri_task_dead(task)) {
		int signo = task->exit_code & 0x7f;
		CR_WARN_PROC_REQ(cr_task->rstrt_proc_req,
			"%s: '%s' (tgid/pid %d/%d) exited with "
			"%s %d during restart",
			__FUNCTION__, task->comm,
			task->tgid, task->pid,
			signo ? "signal" : "code",
			signo ? signo : (task->exit_code & 0xff00) >> 8);
		// Fall through to delete_dead_task()
	} else {
		continue; // DO NOT fall through to delete_dead_task
	}

	// Note that the watchdog and each restarted task
	// hold a reference, so this delete_dead_task()
	// can't destroy req.
	CRI_ASSERT(atomic_read(&req->ref_count) >= 2);
	delete_dead_task(req, cr_task);
    }

    // We are done if procs have arrived, but are all gone now
    finished = !req->need_procs && list_empty(&req->tasks);

    // We are also done if the requester died before spawning the procs
    finished |= (1 == atomic_read(&req->ref_count));

    write_unlock(&req->lock);
    write_unlock(&cr_task_lock);

    if (finished) {
	__cr_wd_del(work);
	release_rstrt_req(req);
    }
}

/**
 * cr_rstrt_abort
 * @cr_task: The task requesting the abort
 * @flags:  abort type
 *
 * Returns:   0 if restart successfully aborted.
 */
int cr_rstrt_abort(cr_task_t *cr_task, unsigned int flags)
{
	cr_rstrt_proc_req_t *proc_req;
	cr_rstrt_req_t *req;
	int result;

	CR_KTRACE_FUNC_ENTRY("flags=0x%x", flags);

	/* Lookup the request */
	result = -ESRCH;
	proc_req = cr_task->rstrt_proc_req;
	if (!proc_req || !proc_req->req) {
		CR_ERR("%s: No matching req found!", __FUNCTION__);
		goto out;
	}
	req = proc_req->req;

	if (!flags) {
	    /* Ignore flags=0 case */
	    result = 0;
	    goto out;
	}

	write_lock(&cr_task_lock);
	write_lock(&req->lock);

	result = req->result = -CR_ERSTRTABRT;
	req->die = 1;	/* tell all procs to kill themselves */
	send_sig_info(SIGKILL, NULL, current); /* kill self */

	delete_dead_task(req, cr_task);

	write_unlock(&req->lock);
	write_unlock(&cr_task_lock);

out:
	CR_KTRACE_FUNC_EXIT("Returning %d", result);
	return result;
}

static
int do_rstrt_log(struct file *filp, char __user *buf, unsigned int len)
{
    cr_pdata_t *priv;
    cr_rstrt_req_t *req; 
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
    req = priv->rstrt_req;
    if (req == NULL) {
	CR_ERR("%s: No rstrt_req attached to filp!", __FUNCTION__);
        goto out;
    }

    retval = cr_errbuf_read(buf, len, req->errbuf);

out:
    CR_KTRACE_FUNC_EXIT("Returning %d", retval);
    return retval;
}

/**
 * cr_rstrt_log
 * @filp: The filp for the restart request
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
int cr_rstrt_log(struct file *filp, struct cr_log_args __user *arg)
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

    retval = do_rstrt_log(filp, buf, len);

out:
    CR_KTRACE_FUNC_EXIT("Returning %d", retval);
    return retval;
}

#ifdef CONFIG_COMPAT
int cr_rstrt_request_restart32(struct file *file, struct cr_compat_rstrt_args __user *req)
{
	struct cr_rstrt_args ureq;
	compat_uptr_t relocate;
	int retval;

	CR_KTRACE_FUNC_ENTRY();

	retval = -EFAULT;
	if (!access_ok(VERIFY_READ, req, sizeof(*req)) ||
	    __get_user(ureq.cr_fd, &req->cr_fd) ||
	    __get_user(ureq.signal, &req->signal) ||
	    __get_user(relocate, &req->relocate) ||
	    __get_user(ureq.flags, &req->flags)) {
		goto out;
	}
	ureq.relocate = compat_ptr(relocate);

	retval = do_rstrt_request_restart(file, &ureq, cr_read_reloc32);

out:
	CR_KTRACE_FUNC_EXIT("Returning %d", retval);
	return retval;
}

int cr_rstrt_procs32(struct file *filp, struct cr_compat_procs_tbl __user *arg)
{
    struct cr_section_header cf_header;
    int retval;

    CR_KTRACE_FUNC_ENTRY();

    retval = do_rstrt_procs(filp, &cf_header);

    if (retval == 1) {
	/* Copy the process-header info to user space */
	if (!access_ok(VERIFY_WRITE, arg, sizeof(*arg)) ||
	    __put_user(cf_header.num_threads, &arg->threads) ||
	    __put_user(cf_header.clone_flags, &arg->clone_flags)) {
	    retval = -EFAULT;
        }
    }

    CR_KTRACE_FUNC_EXIT("Returning %d", retval);
    return retval;
}

int cr_rstrt_log32(struct file *filp, struct cr_compat_log_args __user *arg)
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

    retval = do_rstrt_log(filp, compat_ptr(buf), len);

out:
    CR_KTRACE_FUNC_EXIT("Returning %d", retval);
    return retval;
}
#endif /* CONFIG_COMPAT */

int
cr_rstrt_init(void)
{
    int retval = 0;
    cr_linkage_cachep = KMEM_CACHE(cr_linkage_s, 0);
    if (!cr_linkage_cachep) {
	retval = -ENOMEM;
    }
    return retval;
}

void
cr_rstrt_cleanup(void)
{
    if (cr_linkage_cachep) kmem_cache_destroy(cr_linkage_cachep);
}
