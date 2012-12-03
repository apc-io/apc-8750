/*
 * Berkeley Lab Checkpoint/Restart (BLCR) for Linux is Copyright (c)
 * 2007, The Regents of the University of California, through Lawrence
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
 * $Id: cr_pipes.c,v 1.225.8.2 2009/06/12 20:37:03 phargrov Exp $
 */

#include "cr_module.h"
#include "cr_context.h"

#include <linux/pipe_fs_i.h>

#if !HAVE_PIPE_INODE_INFO_BASE && ((PIPE_BUFFERS * PAGE_SIZE) > CR_KMALLOC_MAX)
  #define CR_ALLOC_PIPEBUF(_sz)	vmalloc(_sz)
  #define CR_FREE_PIPEBUF(_buf)	vfree(_buf)
#else
  #define CR_ALLOC_PIPEBUF(_sz)	kmalloc((_sz),GFP_KERNEL)
  #define CR_FREE_PIPEBUF(_buf)	kfree(_buf)
#endif

#if HAVE_INODE_SEM
  #define cr_pipe_lock(_i)			down(PIPE_SEM(*(_i)))
  #define cr_pipe_lock_interruptible(_i)	down_interruptible(PIPE_SEM((*_i)))
  #define cr_pipe_unlock(_i)			up(PIPE_SEM(*(_i)))
#elif  HAVE_INODE_MUTEX
  #define cr_pipe_lock(_i)			mutex_lock(&(_i)->i_mutex)
  #define cr_pipe_lock_interruptible(_i)	mutex_lock_interruptible(&(_i)->i_mutex)
  #define cr_pipe_unlock(_i)			mutex_unlock(&(_i)->i_mutex)
#else
  #error "Unknown pipe lock type"
#endif 

/*
 * f_flags - same as in user space
 * f_mode  - funny kernel thing (can reconstruct from f_flags)
 *   00 - no permissions needed
 *   01 - read permission needed
 *   10 - write permission needed
 *   11 - read/write permissions needed
 */
static int
cr_posix_flags_to_f_mode(int posix_flags)
{
    return ((posix_flags+1) & O_ACCMODE);
}

/*
 * This is more or less the same thing as open(), with the major exception
 * that we DO NOT BLOCK.  (Normal open() does block in certain cases.)
 * We take the approach of first openning read-write.  Then a second
 * open is done with the real flags.  The presence of the first open
 * ensures the second will not block for lack of readers or writers.
 */
static int
cr_open_named_fifo(cr_errbuf_t *eb, int fd, struct dentry *dentry, struct vfsmount *mnt, int flags)
{
    int retval = 0;
    struct file *filp, *rw_filp;

    /* First we open R/W unconditionally.
     *
     * Note, since dentry_open() assumes caller has checked permissions, we
     * know this "extra" RDWR open will work regardless of true access rights.
     * THIS IS ALSO WHY WE MUST CLOSE rw_filp WHEN DONE WITH IT.
     */
  #if HAVE_TASK_CRED
    rw_filp = dentry_open(dget(dentry), mntget(mnt), O_RDWR, cr_current_cred());
  #else
    rw_filp = dentry_open(dget(dentry), mntget(mnt), O_RDWR);
  #endif
    retval = PTR_ERR(rw_filp);
    if (IS_ERR(rw_filp)) goto out;
	
    /* Now we open again w/ the desired permissions.
     * Note we don't short-cut on (flags==O_RDWR) because we want
     * the permission check that occurs in cr_filp_reopen().
     */
    filp = cr_filp_reopen(rw_filp, flags);
    filp_close(rw_filp, current->files);
    retval = PTR_ERR(filp);
    if (IS_ERR(filp)) goto out;

    /* Install it */
    retval = cr_fd_claim(fd);
    if (retval < 0) {
        CR_ERR_EB(eb, "File descriptor %d in use.", fd);
	goto out;
    }
    fd_install(fd, filp);

out:
    return retval;
}

/* Set up a new file pointer to an existing fifo */
static int
cr_couple_named_fifo(cr_rstrt_req_t *req, struct cr_file_info *file_info, 
		     struct cr_fifo *cf_fifo, struct file *first_filp,
		     const char *name)
{
    cr_errbuf_t *eb = req->errbuf;
    struct dentry *dentry = first_filp->f_dentry;
    struct vfsmount *mnt = first_filp->f_vfsmnt;
    struct dentry *new_dentry = NULL;
    int retval = -EINVAL;

    /*
     * Note that the following logic is certain to work if either ALL or NONE
     * of the original hard links existed at checkpoint time, and again either
     * ALL or NONE exist at restart time.
     * If only SOME links exist at either point in time, then this logic may
     * still work, depending on ordering.  However, even case in which this logic
     * fails to restart, it does so with an EEXIST or ENOENT, not a kernel Oops!
     */
    if (file_info->unlinked || vmad_dentry_unlinked(dentry)) {
	/* We currently can't link to an unlinked file (cr_link() limitation).  Also,
	 * we don't currently bother to create a new link that we immediately unlink,
	 * since only /proc/<pid>/fd will show the difference.
	 */
	new_dentry = dget(dentry);
    } else if (!cr_find_object(req->map, cf_fifo->fifo_dentry, (void **) &new_dentry)) {
	/* We are first to use this name, so ensure the link exists (and is correct).
	 * The link ensures that any post-restart open()s of this name will get
	 * the correct FIFO.
	 */
	CR_PATH_DECL(path);
	CR_PATH_GET_FILE(path, first_filp);
	new_dentry = cr_link(eb, path, name);
	path_put(path);
	if (IS_ERR(new_dentry)) {
	    retval = PTR_ERR(new_dentry);
	    CR_ERR_REQ(req, "cr_couple_named_fifo:  unable to link named fifo: %s", name);
	    goto out;
	}
	cr_insert_object(req->map, cf_fifo->fifo_dentry, new_dentry, GFP_KERNEL);
    } else {
	/* This name is already created, so just open it again */
	dget(new_dentry);
    }

    /* Now open it */
    if (!new_dentry->d_inode) {
	CR_ERR_REQ(req, "cr_couple_named_fifo:  NULL new_dentry->d_inode");
	retval = -EIO;
	goto out_dput;
    }
    retval = cr_open_named_fifo(eb, file_info->fd, new_dentry, mnt, cf_fifo->fifo_flags);

out_dput:
    dput(new_dentry);
out:
    return retval;
}

/* Set up a new file pointer to an existing pipe */
static int
cr_couple_unnamed_pipe(cr_rstrt_req_t *req, struct cr_file_info *file_info, 
	struct cr_fifo *cf_fifo, const struct file *first_filp)
{
    int retval;
    struct file *filp;

    CR_KTRACE_LOW_LVL("Opening FIFO ID %p", cf_fifo->fifo_id);

    /* i_writecount is not incremented in do_pipe(), but is decremented below close().
     * The value is never checked, so there is not a problem with letting that field
     * just become negative when one end of a pipe is closed.  However, if we call
     * cr_filp_reopen() asking for write permissions, there *is* a check of i_writecount
     * that count fail (returning -ETXTBUSY of all things).  So, we zero it here and
     * let its value assume a bogus positive value rather than a bogus negative one.
     */
    atomic_set(&first_filp->f_dentry->d_inode->i_writecount, 0);

    filp = cr_filp_reopen(first_filp, cf_fifo->fifo_flags);
    retval = PTR_ERR(filp);
    if (IS_ERR(filp)) {
	goto out;
    }

    /* install the fd */
    retval = cr_fd_claim(file_info->fd);
    if (retval) {
        /* this should never happen. */
        CR_ERR_REQ(req, "File descriptor %d in use.", file_info->fd);
        goto out;
    }
    fd_install(file_info->fd, filp);

out:
    return retval;
}

static int
cr_creat_named_fifo(cr_errbuf_t *eb, struct cr_file_info *file_info, 
		    const struct cr_fifo *cf_fifo, const char *name)
{
    int retval;
    struct nameidata nd;
    struct dentry * dentry;

    /* make a new FIFO, or find the existing one */
    dentry = cr_mknod(eb, &nd, name, cf_fifo->fifo_perms, file_info->unlinked ? file_info->fd+1 : 0);
    if (IS_ERR(dentry)) {
	retval = PTR_ERR(dentry);
	CR_ERR_EB(eb, "Failed to create named fifo %s.  err=%d.", name, retval);
	goto out;
    }

    /* open it */
    retval = cr_open_named_fifo(eb, file_info->fd, dentry, nd.nd_mnt, cf_fifo->fifo_flags);
    if (retval < 0) {
	CR_ERR_EB(eb, "Couldn't open %s.  err=%d.", name, retval);
	goto out_dput;
    }

    retval = 0;

out_dput:
    dput(dentry);
    cr_path_release(&nd);
out:
    return retval;
}

static int
cr_creat_unnamed_pipe(cr_errbuf_t *eb, struct cr_file_info *file_info, 
		      const struct cr_fifo *cf_fifo)
{
    int retval;
    int pipe_fds[2];
    int tmp_fd;
    int keep;

    retval = cr_do_pipe(pipe_fds);
    if (retval < 0) {
        CR_ERR_EB(eb, "do_pipe() failed (%d).", retval);
	goto out;
    }

    /* now have two pipe objects, close one of them. */
    switch (cr_posix_flags_to_f_mode(cf_fifo->fifo_flags)) {
    case FMODE_READ:
	keep = 0;
	break;

    case FMODE_WRITE:
	keep = 1;
	break;

    default:
        CR_ERR_EB(eb, "Bad access mode for pipe.");
        goto out;
    }

    sys_close(pipe_fds[!keep]);
    tmp_fd = pipe_fds[keep];

    /* put the fd into the right place */
    if (tmp_fd != file_info->fd) {
        retval = sys_dup2(tmp_fd, file_info->fd);
        (void)sys_close(tmp_fd); /* we ignore any error from close here. */
        if (retval < 0) {
            CR_ERR_EB(eb, "sys_dup2(%d,%d) failed.", tmp_fd, file_info->fd);
            goto out;
        }
    }

out:
    return retval;
}

static int
cr_restore_pipe_buf(cr_errbuf_t *eb, struct file *cf_filp, struct inode *p_inode, int buf_len, int do_not_restore_flag)
{
    struct pipe_inode_info *pipe;
    int retval;
    void *buf;

    if (!buf_len) {
        CR_KTRACE_LOW_LVL("Skipping empty pipe buffer %p", p_inode);
	retval = 0;
	goto out;
    }

    buf = (void *)CR_ALLOC_PIPEBUF(buf_len);
    if (!buf) {
        CR_ERR_EB(eb, "Unable to allocate memory for pipe buffer!");
	retval = -ENOMEM;
	goto out;
    }

    // XXX: need eb arg for kread
    retval = cr_kread(eb, cf_filp, buf, buf_len);
    if (retval != buf_len) {
	CR_ERR_EB(eb, "pipe fifo: read buf returned %d", retval);
	goto out_free;
    }

    if (do_not_restore_flag) {
        CR_KTRACE_LOW_LVL("Unrestored pipe buffer %p length = %d", p_inode, buf_len);
        retval = 0;
        goto out_free;
    }

    if (cr_pipe_lock_interruptible(p_inode)) {
        retval = -ERESTARTSYS;
        goto out_free;
    }

    pipe = p_inode->i_pipe;
#if HAVE_PIPE_INODE_INFO_BASE
    if (PIPE_LEN(*p_inode)) {
        /* someone put data in here already -- bail */
        retval = -EBUSY;
	goto out_up;
    }

    if (buf_len) {
	memcpy((void *)PIPE_BASE(*p_inode), buf, buf_len);
        PIPE_LEN(*p_inode) = buf_len;
        CR_KTRACE_LOW_LVL("Pipe buffer %p length = %d inode = %lu", p_inode, buf_len, p_inode->i_ino);
	retval = 0;
    }
#else
    {   struct pipe_inode_info *pipe = p_inode->i_pipe;
	char * p;
        int nrbufs;

	if (pipe->nrbufs != 0) {
            /* someone put data in here already -- bail */
            retval = -EBUSY;
	    goto out_up;
	}

	nrbufs = 0;
	p = buf;
	while (buf_len) {
	    size_t count = buf_len;
	    struct pipe_buffer *buf = pipe->bufs + nrbufs;
	    struct page *page = alloc_page(GFP_HIGHUSER);
	    if (count > PAGE_SIZE)
		count = PAGE_SIZE;
	    memcpy(kmap(page), p, count);
	    kunmap(page);
	    buf->page = page;
	    buf->offset = 0;
	    buf->len = count;
	    buf->ops = &anon_pipe_buf_ops;
	    ++nrbufs;
	    buf_len -= count;
	    p += count;
	}
	pipe->curbuf = 0;
	pipe->nrbufs = nrbufs;
    }
#endif

out_up:
    cr_pipe_unlock(p_inode);
out_free:
    CR_FREE_PIPEBUF(buf);
out:
    return retval;
}

/* 
 * pipe was instantiated already, so we connect our file descriptor
 * to the pipe inode.
 */
static int
cr_couple_fifo(cr_rstrt_req_t *req, struct cr_file_info *file_info, 
	       struct cr_fifo *cf_fifo, const char *name, int do_not_restore_flag)
{
    int retval;
    struct file *first_filp = NULL;

    retval = -EINVAL;
    if (do_not_restore_flag) {
	/* The only reason we don't return right here is validation of name */
    } else if (!cr_find_object(req->map, cf_fifo->fifo_id, (void **) &first_filp)) {
	CR_ERR_REQ(req, "cr_couple_fifo - unable to find previously restored fifo (id=%p)", cf_fifo->fifo_id);
	goto out;
    }

    CR_KTRACE_LOW_LVL("%s:  phase 2: connecting second end.", name);
    /* no obvious way to distinguish named pipes from unnamed besides 
     * filename.  anything with a '/' at the beginning is a named fifo, 
     * and anything with a 'p' is a pipe. */
    retval = 0;
    switch (name[0]) {
    case '/': /* named fifo */
        if (!do_not_restore_flag) {
            retval = cr_couple_named_fifo(req, file_info, cf_fifo, first_filp, name);
            if (retval < 0) {
                CR_ERR_REQ(req, "cr_couple_fifo:  unable to open named fifo: %s", name);
            }
        }
	break;

    case 'p': /* unnamed pipe */
        if (!do_not_restore_flag) {
            retval = cr_couple_unnamed_pipe(req, file_info, cf_fifo, first_filp);
            if (retval < 0) {
                CR_ERR_REQ(req, "cr_couple_fifo:  unable to open unnamed pipe: %s", name);
            }
        }
	break;

    default: /* garbage */
        retval = -EINVAL;
        CR_ERR_REQ(req, "bogus pipe filename in context file: %s", name);
        goto out;
    }

  out:
    return retval;
}

/* Returns a flip that caller must fput() */
static struct file *
cr_make_new_pipe(cr_rstrt_req_t *req, struct cr_file_info *file_info, 
	         struct cr_fifo *cf_fifo, const char *name, int do_not_restore_flag)
{
    cr_errbuf_t *eb = req->errbuf;
    struct file *p_filp = NULL;
    int err = 0;

    CR_KTRACE_LOW_LVL("%s:  Phase 1: Making new pipe.", name);

    switch (name[0]) {
    case '/': /* named fifo */
        if (do_not_restore_flag) goto out;
        err = cr_creat_named_fifo(eb, file_info, cf_fifo, name);
	break;

    case 'p': /* unnamed pipe */
        if (do_not_restore_flag) goto out;
	err = cr_creat_unnamed_pipe(eb, file_info, cf_fifo);
	break;

    default: /* garbage */
        err = -EINVAL;
        CR_ERR_REQ(req, "Bogus pipe filename in context file: %s", name);
    } 

    if (err < 0) goto out;

    /* Might be the new pipe fd, or might be the fd protected by do-not-restore */
    p_filp = fget(file_info->fd);
    if (!p_filp) {
        goto out;
    }

    /* now do the easy stuff (I'm not sure we should touch these -PHH) */
    p_filp->f_pos = cf_fifo->fifo_pos;
    p_filp->f_version = cf_fifo->fifo_version;
    /* p_filp->f_fop and f_flags initialized correctly by do_pipe/mknod */

    /* Map fifo_id (the pre-checkpoint inode) to the new file* for subsequent open(s) */
    cr_insert_object(req->map, cf_fifo->fifo_id, p_filp, GFP_KERNEL);

    /* And one to distinguish multiple hard links */
    cr_insert_object(req->map, cf_fifo->fifo_dentry, p_filp->f_dentry, GFP_KERNEL);

out:
    return (err < 0) ? ERR_PTR(err) : p_filp;
}

extern int 
cr_restore_open_fifo(cr_rstrt_proc_req_t *proc_req,
	struct cr_file_info *file_info, int do_not_restore_flag)
{
    cr_errbuf_t *eb = proc_req->req->errbuf;
    int retval = -ENOSYS;
    cr_rstrt_req_t *req = proc_req->req;
    struct file *cf_filp = proc_req->file;
    struct cr_fifo cf_fifo;
    const char *name;

    CR_KTRACE_FUNC_ENTRY();

    /* read fifo struct */
    retval = cr_kread(eb, cf_filp, &cf_fifo, sizeof(cf_fifo));
    if (retval != sizeof(cf_fifo)) {
	CR_ERR_REQ(req, "pipe fifo: read failed");
        goto out;
    }

    CR_KTRACE_LOW_LVL("   Open fifo: id == %p.", cf_fifo.fifo_id);

    if (cf_fifo.cr_type != cr_fifo_obj) {
        CR_ERR_REQ(req, "cr_restore_open_fifo.  Garbage found in context file.  Wrong object type.");
        retval = -EINVAL;
        goto out;
    }

    /* Force do_not_restore_flag OFF for "internal" pipes and ON for "external" ones */
    if (cf_fifo.fifo_internal) {
	do_not_restore_flag = 0;
	if (fcheck(file_info->fd) != NULL) {
	    sys_close(file_info->fd);
	}
    } else if (!do_not_restore_flag) {
	/* Deal with a dangling pipe by effectively dup()ing from cr_restart */
	struct file *old_filp, *filp;
	switch (cf_fifo.fifo_flags & O_ACCMODE) {
	case O_RDONLY:
	    old_filp = req->cr_restart_stdin;
	    break;
	case O_WRONLY:
	    old_filp = req->cr_restart_stdout;
	    break;
	default:
	    CR_ERR_REQ(req, "Invalid access mode %d while restoring external pipe",
			(cf_fifo.fifo_flags & O_ACCMODE));
	    retval = -EINVAL;
	    goto out;
	}
	filp = cr_filp_reopen(old_filp, cf_fifo.fifo_flags);
	if (IS_ERR(filp)) {
	    retval = PTR_ERR(filp);
	    CR_ERR_REQ(req, "Error %d from cr_filp_reopen() while restoring external pipe", retval);
	    goto out;
	}
	retval = cr_fd_claim(file_info->fd);
	if (retval < 0) {
	    CR_ERR_REQ(req, "File descriptor %d in use.", file_info->fd);
	    goto out;
	}
	fd_install(file_info->fd, filp);
	do_not_restore_flag = 1;
    }

    /* read file name */
    name = cr_getname(eb, req->relocate, cf_filp, 0);
    if (IS_ERR(name)) {
        CR_ERR_REQ(req, "pipe_name - Bad pathname read!");
        retval = PTR_ERR(name);
        goto out;
    }
    
    /* now, actually rebuild the thing. */
    if (cf_fifo.fifo_len == ((unsigned int)-1)) {
        retval = cr_couple_fifo(req, file_info, &cf_fifo, name,
                                do_not_restore_flag);
        if (retval < 0) {
            goto out_free;
        }

        /* 
         * the pipe buffer should have been restored when we first built the pipe
         */
    } else {
        struct file *p_filp;

        p_filp = cr_make_new_pipe(req, file_info, &cf_fifo, name,
                                  do_not_restore_flag);
	retval = PTR_ERR(p_filp);
        if (IS_ERR(p_filp) || !p_filp) {
            goto out_free;
        }

	/* now read the pipe buffer data, and optionally restore it */
	retval = cr_restore_pipe_buf(eb, cf_filp, p_filp->f_dentry->d_inode,
                                     cf_fifo.fifo_len, do_not_restore_flag);
	fput(p_filp);
	if (retval < 0) {
	    CR_ERR_REQ(req, "could not restore pipe buffer");
	    goto out_free;
	}
    }

out_free:
    __putname(name);
out:
    return retval;
}

/* Deterimine if a given FIFO is "internal", defined has having
 * at least one reader and one writer in the checkpoint set.
 *
 * XXX: This is almost certainly the worst possible way to do this.
 * Alas, I don't have a better way in mind that can be implemented
 * w/o need to move file saves/restores to the end (after memory of
 * all processes).
 */
static int
cr_fifo_is_internal(cr_chkpt_req_t *req, struct inode *inode) {
    cr_chkpt_proc_req_t *proc_req;
    int have_reader = 0;
    int have_writer = 0;
    unsigned long retval = 0; /* For size match w/ void* in object map */
    char *map_key = 1 + (char *)inode;

    read_lock(&req->lock);

    /* Look for cached value first.  Note "creative" use of key. */
    if (cr_find_object(req->map, map_key, (void **) &retval)) {
	goto out_cached;
    }

    list_for_each_entry(proc_req, &req->procs, list) {
	int max_fds, fd;
	cr_fdtable_t *fdt;
	struct task_struct *task = NULL;
	cr_task_t *cr_task;

	/* Find a non-dead task in proc_req, if any.
	 * XXX: Race against exits?  What lock should we hold here?
	 */
	list_for_each_entry(cr_task, &proc_req->tasks, proc_req_list) {
	    if (!cri_task_dead(cr_task->task)) {
		task = cr_task->task;
		break;
	    }
	}
	if (!task) continue;

	spin_lock(&task->files->file_lock);
	rcu_read_lock();
	fdt = cr_fdtable(task->files);
	max_fds = fdt->max_fds;	/* Never shrinks, right? */
        rcu_read_unlock();

	for (fd = 0; !retval && (fd < max_fds); ++fd) {
	    struct file *filp = fcheck_files(task->files, fd);
	    if (!filp) continue;
	    get_file(filp);
	    if (filp->f_dentry->d_inode == inode) {
		if (filp->f_mode & FMODE_WRITE) have_writer = 1;
		if (filp->f_mode & FMODE_READ) have_reader = 1;
	    }
	    fput(filp);
	    retval = (have_reader && have_writer);
	}

	spin_unlock(&task->files->file_lock);
	if (retval) break;
    }

    cr_insert_object(req->map, map_key, (void *)retval, GFP_ATOMIC /* we hold req->lock */);
out_cached:
    read_unlock(&req->lock);
    CR_KTRACE_FUNC_EXIT("Return '%sternal'", retval ? "in" : "ex");
    return (int)retval;
}

extern int 
cr_save_open_fifo(cr_chkpt_proc_req_t *proc_req, struct file *filp)
{
    cr_errbuf_t *eb = proc_req->req->errbuf;
    struct file *cf_filp = proc_req->file;
    struct cr_fifo cf_fifo;
    struct inode *inode;
    void *buf = NULL;
    int retval;

    retval = -EINVAL;
    if (!filp)
	goto out;

    inode = filp->f_dentry->d_inode;

    cf_fifo.cr_type = cr_fifo_obj;
    cf_fifo.fifo_id = inode;
    cf_fifo.fifo_dentry = filp->f_dentry;
    cf_fifo.fifo_len = -1;
    cf_fifo.fifo_internal = cr_fifo_is_internal(proc_req->req, inode);

    /* Note: cf_fifo.fifo_internal check makes us skip the pipebuf save for external pipes
     * since we currently don't even try to read the data back in for this case.
     */
    if (!cr_insert_object(proc_req->req->map, inode, inode, GFP_KERNEL) && cf_fifo.fifo_internal) {
	/* We are first to save: suck the data out of the pipe while holding the pipe semaphore */
	retval = -ERESTARTSYS;
	if (cr_pipe_lock_interruptible(inode)) {
	    goto out;
	}
    #if HAVE_PIPE_INODE_INFO_BASE
	{
	    cf_fifo.fifo_len = PIPE_LEN(*inode);
	    buf = CR_ALLOC_PIPEBUF(cf_fifo.fifo_len);
	    if (!buf) {
	        retval = -ENOMEM;
	        cr_pipe_unlock(inode);
	        goto out;
	    }
	    memcpy(buf, (void*)PIPE_BASE(*inode)+PIPE_START(*inode), cf_fifo.fifo_len);
	}
    #else
	{
	    struct pipe_inode_info *pipe = inode->i_pipe;
	    int i, curbuf;
	    char *p;

	    buf = CR_ALLOC_PIPEBUF(PIPE_BUFFERS * PAGE_SIZE);
	    if (!buf) {
	        retval = -ENOMEM;
	        cr_pipe_unlock(inode);
	        goto out;
	    }

	    p = buf;
	    cf_fifo.fifo_len = 0;
	    curbuf = pipe->curbuf;
	    for (i = 0; i < pipe->nrbufs; ++i) {
	        struct pipe_buffer *pbuf = pipe->bufs + curbuf;
	        const struct pipe_buf_operations *ops = pbuf->ops;
	        char *addr;
#if HAVE_PIPE_BUF_OPERATIONS_PIN
		int error = ops->pin(pipe, pbuf);
		if (error) {
	          retval = error;
	          cr_pipe_unlock(inode);
	          goto out_free;
	        }
		addr = ops->map(pipe, pbuf, 0);
	        memcpy(p, addr + pbuf->offset, pbuf->len);
		ops->unmap(pipe, pbuf, addr);
#elif HAVE_2_ARG_PIPE_OPS_UNMAP
	        addr = ops->map(filp, pipe, pbuf);
	        memcpy(p, addr + pbuf->offset, pbuf->len);
	        ops->unmap(pipe, pbuf);
#elif HAVE_3_ARG_PIPE_OPS_UNMAP
	        addr = ops->map(pipe, pbuf, 0);
	        memcpy(p, addr + pbuf->offset, pbuf->len);
	        ops->unmap(pipe, pbuf, addr);
#else
  #error "Unknown pipe buf operations"
#endif
	        p += pbuf->len;
	        cf_fifo.fifo_len += pbuf->len;
	        curbuf = (curbuf + 1) & (PIPE_BUFFERS-1);
	    }
	}
    #endif
	cr_pipe_unlock(inode);
    } else {
	/* We've saved this fifo previously */
	/* cf_fifo.fifo_len == -1 will identify this case at restart time. */
    }

    /* pipe perms - for FIFOs, need to know what to pass to mknod() */
    cf_fifo.fifo_perms = inode->i_mode;

    /* posix flags (for open) */
    cf_fifo.fifo_pos = filp->f_pos;
    cf_fifo.fifo_flags = filp->f_flags;
    cf_fifo.fifo_version = filp->f_version;

    /* write out fifo structure */
    retval = cr_kwrite(eb, cf_filp, &cf_fifo, sizeof(cf_fifo));
    if (retval != sizeof(cf_fifo)) {
	CR_ERR_PROC_REQ(proc_req, "pipe fifo: write failed");
	goto out_free;
    }

    /* write the filename out */
    retval = cr_save_filename(eb, cf_filp, filp, NULL, 0);
    if (retval < 0) {
	CR_ERR_PROC_REQ(proc_req, "Error saving pipe filename. (err=%d)", retval);
	goto out_free;
    }
   
    /* write out pipe data last (unless saved previously) */
    if (cf_fifo.fifo_len != ((unsigned int)-1)) {
	retval = cr_kwrite(eb, cf_filp, buf, cf_fifo.fifo_len);
	if (retval != cf_fifo.fifo_len) {
	    CR_ERR_PROC_REQ(proc_req, "pipe fifo: write buf failed");
	    goto out_free;
	}
    }

    retval = 0;

out_free:
    CR_FREE_PIPEBUF(buf);  /* NULL ok */
out:
    return retval;
}
