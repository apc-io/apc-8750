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
 * $Id: cr_dump_self.c,v 1.228.8.1 2009/03/26 04:22:43 phargrov Exp $
 */

#include "cr_module.h"

#include <linux/smp_lock.h>
#include <linux/pipe_fs_i.h>
#include <linux/binfmts.h>
#include <linux/major.h>

#include "vmadump.h"
#include "cr_context.h"

/*
 * XXX:  Fix to return number of bytes written
 */
static int cr_save_linkage(cr_chkpt_proc_req_t *proc_req, struct file *filp)
{
    cr_chkpt_req_t *req = proc_req->req;
    cr_errbuf_t *eb = req->errbuf;
    struct cr_context_tasklinkage *linkage;
    int linkage_size;
    cr_task_t *cr_task;
    struct task_struct *task;
    int task_count = proc_req->thread_count;
    int task_num;
    int retval;

    /* check task count, just ensure non-zero and not too big for kmalloc */
    linkage = NULL;
    linkage_size = sizeof(*linkage) * task_count;
    if ((task_count > 0) && linkage_size <= CR_KMALLOC_MAX) {
        /* allocate a cr_context_linkage large enough to hold all of the
         * available tasks */
        linkage = kmalloc(linkage_size, GFP_KERNEL);
    }
    if (linkage == NULL) {
        retval = -ENOMEM;
	goto out;
    }

    read_lock(&tasklist_lock);
    read_lock(&req->lock);
    /* loop over each task, and copy important fields into the linkage */
    task_num = 0;
    list_for_each_entry(cr_task, &proc_req->tasks, proc_req_list) {
	if (cr_task == NULL) {
	    CR_ERR_REQ(req, "NULL cr_task found on task list!");
	    retval = -EINVAL;
	    goto out_nulltask;
	}
	task = cr_task->task;

	/* XXX: can a task that has exited still be on the task list?  If so,
	 * how would we ever now? 
	 *   -- Must we even care?  Even if the task_struct is garbage, it
	 *   only matters we'll build a new process around it... 
	 */

	linkage[task_num].myptr       = task;
#ifdef CR_REAL_PARENT
	linkage[task_num].real_parent = CR_REAL_PARENT(task);
#endif
	linkage[task_num].parent      = CR_PARENT(task);

	/* Handle init specially */
#ifdef CR_REAL_PARENT
	if (linkage[task_num].real_parent == cr_child_reaper) {
	    linkage[task_num].real_parent = CR_PARENT_INIT;
	}
#endif
	if (linkage[task_num].parent == cr_child_reaper) {
	    linkage[task_num].parent = CR_PARENT_INIT;
	}

	if ((task->pid <= 0) || (task->pid >= PID_MAX_LIMIT)) {
	   CR_ERR_REQ(req, "Saving an invalid pid %d", task->pid);
	}

	linkage[task_num].pid     = task->pid;
	linkage[task_num].tgid    = task->tgid;
	linkage[task_num].pgrp    = cr_task_pgrp(task);
#if 0 /* We don't do anything with this yet, and it changed type around 2.6.21 */
	linkage[task_num].tty_old_pgrp = cr_task_tty_old_pgrp(task);
#endif
	linkage[task_num].session = cr_task_session(task);
	linkage[task_num].leader  = cr_task_leader(task);
	linkage[task_num].exit_signal  = task->exit_signal;

	linkage[task_num].cr_task.stopped   = cr_task->stopped;
	linkage[task_num].cr_task.phase     = cr_task->phase;
	linkage[task_num].cr_task.fd        = cr_task->fd;
	linkage[task_num].cr_task.handler_sa = cr_task->handler_sa;
	linkage[task_num].cr_task.orig_filp = cr_task->filp;
	linkage[task_num].cr_task.chkpt_flags = cr_task->chkpt_flags;
	++task_num;
    }

out_nulltask:
    read_unlock(&req->lock);
    read_unlock(&tasklist_lock);

    /* This will catch the goto, and a possible race condition */
    if (task_num != task_count) {
        CR_ERR_REQ(req, "wrong number of tasks counted: task_count = %d, task_num = %d", 
	    task_count, task_num);
	retval = -EINVAL;
	goto out_free;
    }

    /* write the length of the linkage structure to the context file */
    retval = cr_kwrite(eb, filp, &linkage_size, sizeof(linkage_size));
    if (retval != sizeof(linkage_size)) {
	CR_ERR_REQ(req, "linkage: write size returned %d", retval);
	goto out_free;
    }

    /* now write the linkage structure */
    retval = cr_kwrite(eb, filp, linkage, linkage_size);
    if (retval != linkage_size) {
	CR_ERR_REQ(req, "linkage: write table returned %d", retval);
	goto out_free;
    }

out_free:
    kfree(linkage);
out:
    return retval;
}

static int 
cr_save_fs_struct(cr_chkpt_proc_req_t *proc_req)
{
    cr_errbuf_t *eb = proc_req->req->errbuf;
    CR_PATH_DECL(root_path);
    CR_PATH_DECL(pwd_path);
    struct file *cf_filp = proc_req->file;
    int retval;
    int umask;
    void *buf;

    buf = __getname();
    if (!buf) {
        retval = -ENOMEM;
        goto out_nomem;
    }

    read_lock(&current->fs->lock);
    umask = current->fs->umask;
    CR_PATH_GET_FS(root_path, current->fs->root);
    CR_PATH_GET_FS(pwd_path, current->fs->pwd);
    read_unlock(&current->fs->lock);

    /* save umask */
    retval = cr_kwrite(eb, cf_filp, &umask, sizeof(umask));
    if (retval != sizeof(umask)) {
        CR_ERR_PROC_REQ(proc_req, "umask: write returned %d", retval);
        goto out_writerr;
    }

    /* save root */
    retval = cr_save_pathname(eb, cf_filp, root_path, buf, PATH_MAX);
    if (retval < 0) {
        CR_ERR_PROC_REQ(proc_req, "root: Bad file write!");
        goto out_writerr;
    }

    /* save pwd */
    retval = cr_save_pathname(eb, cf_filp, pwd_path, buf, PATH_MAX);
    if (retval < 0) {
        CR_ERR_PROC_REQ(proc_req, "pwd: Bad file write!");
        goto out_writerr;
    }

out_writerr:
    path_put(root_path);
    path_put(pwd_path);
    __putname(buf);
out_nomem:
    return retval;
}

/*
 * Saves the files_struct
 *
 * This looks silly, but we actually restore the values of max_fds array
 * later.  Since we know how many files are open, we don't have to guess
 * the way get_unused_fd does.
 *
 * returns max_fds or error
 */
static int cr_save_files_struct(cr_chkpt_proc_req_t *proc_req, struct files_struct *files)
{
    cr_errbuf_t *eb = proc_req->req->errbuf;
    cr_fdtable_t *fdt;
    struct cr_files_struct cr_fs;
    struct file *cf_filp = proc_req->file;
    int retval=0;

    cr_fs.cr_obj = cr_files_obj; 

#if !HAVE_STRUCT_FDTABLE
    spin_lock(&files->file_lock);
#endif

    rcu_read_lock();
    fdt = cr_fdtable(files);
    cr_fs.cr_max_fds   = fdt->max_fds;
    cr_fs.cr_next_fd   = CR_NEXT_FD(files, fdt);
    rcu_read_unlock();

#if !HAVE_STRUCT_FDTABLE
    spin_unlock(&files->file_lock);
#endif

    retval = cr_kwrite(eb, cf_filp, &cr_fs, sizeof(cr_fs));
    if (retval != sizeof(cr_fs)) {
        CR_ERR_PROC_REQ(proc_req, "files struct: write returned %d", retval);
        goto out_writerr;
    }

    retval = cr_fs.cr_max_fds;
out_writerr:
    return retval;
}

/*
 * cr_get_fd_info
 *
 * Saves flags associated with the file descriptor.  (stored in files_struct)
 *
 * Must be called with the file_lock held, unless HAVE_STRUCT_FDTABLE for RCU.
 * 
 * Assumes that the file is open.  (descriptor refers to an open file in fd)
 */
static int
cr_get_fd_info(struct files_struct *files, int fd, struct cr_file_info *file_info)
{
    cr_fdtable_t *fdt;

    rcu_read_lock();
    fdt = cr_fdtable(files);
    if (FD_ISSET(fd, fdt->open_fds)) {
        file_info->fd      = fd;
        file_info->cloexec = FD_ISSET(fd, fdt->close_on_exec);
        file_info->orig_filp = fcheck(fd);
    } else {
        CR_WARN("cr_get_fd_info: Called on closed file!");
    }
    rcu_read_unlock();

    return 0;
}

static int
cr_is_open_chkpt_req(cr_chkpt_proc_req_t *proc_req, struct file *filp)
{
    struct dentry *ctrl = proc_req->req->ctrl_file->f_dentry;
    struct dentry *dentry = filp->f_dentry;
    cr_pdata_t *priv = filp->private_data;
    return (((dentry == ctrl) || (dentry->d_inode == ctrl->d_inode)) && priv && priv->chkpt_req);
}

static int
cr_get_file_info(cr_chkpt_proc_req_t *proc_req, struct file *filp, struct cr_file_info *file_info)
{
    struct dentry *dentry = filp->f_dentry;
    int retval=0;

    file_info->unlinked = vmad_dentry_unlinked(dentry);

    if (cr_insert_object(proc_req->req->map, file_info->orig_filp, file_info->orig_filp, GFP_KERNEL)) {
        /* Was in the object table (and thus is dup) */
        file_info->cr_file_type = cr_open_dup;
    } else if (cr_is_open_chkpt_req(proc_req, filp)) {
	file_info->cr_file_type = cr_open_chkpt_req;
    } else {
	switch (dentry->d_inode->i_mode & S_IFMT) {
	case S_IFREG:
	    file_info->cr_file_type = cr_open_file;
	    break;
	case S_IFDIR:
	    file_info->cr_file_type = cr_open_directory;
	    break;
	case S_IFLNK:
	    file_info->cr_file_type = cr_open_link;
	    break;
	case S_IFIFO:
	    file_info->cr_file_type = cr_open_fifo;
	    break;
	case S_IFSOCK:
	    file_info->cr_file_type = cr_open_socket;
	    break;
	case S_IFCHR:
	    file_info->cr_file_type = cr_open_chr;
	    break;
	case S_IFBLK:
	    file_info->cr_file_type = cr_open_blk;
	    break;
	default: /* completely unknown */
            file_info->cr_file_type = cr_unknown_file;
        }
    }

    return retval;
}

static int
cr_save_file_info(cr_chkpt_proc_req_t *proc_req, struct cr_file_info *file_info)
{
    cr_errbuf_t *eb = proc_req->req->errbuf;
    struct file *cf_filp = proc_req->file;
    int retval;

    retval = cr_kwrite(eb, cf_filp, file_info, sizeof(*file_info));
    if (retval != sizeof(*file_info)) {
        CR_ERR_PROC_REQ(proc_req, "file_info: write returned %d", retval);
        goto out;
    }

    retval = 0;

out:
    return retval;
}

static int
cr_save_open_file(cr_chkpt_proc_req_t *proc_req, struct file *filp)
{
    cr_errbuf_t *eb = proc_req->req->errbuf;
    struct file *cf_filp = proc_req->file;                /* the context file */
    struct cr_open_file open_file;
    struct dentry *dentry = filp->f_dentry;
    struct inode *inode = dentry->d_inode;
    int retval = -EBADF;

    open_file.cr_type = cr_open_file_obj;
    /* XXX:  Fix to use better representation. */
    open_file.file_id = inode;
    open_file.i_mode  = inode->i_mode;
    open_file.i_size  = i_size_read(inode);
    open_file.f_flags = filp->f_flags;
    open_file.f_pos   = filp->f_pos;

    /* write out the open_file struct */
    retval = cr_kwrite(eb, cf_filp, &open_file, sizeof(open_file));
    if (retval != sizeof(open_file)) {
        CR_ERR_PROC_REQ(proc_req, "open_file: write returned %d", retval);
        goto out;
    }

    /* Write the filename */
    retval = cr_save_filename(eb, cf_filp, filp, NULL, 0);
    if (retval < 0) {
        CR_ERR_PROC_REQ(proc_req, "cr_save_open_file - Bad file write (filename)!");
    }
    /*add by jay, do not handle /dev/__properties__ on android2.3*/
    if(!strncmp(dentry->d_name.name,"__properties__",strlen("__properties__"))){
    		goto out;
    }
    /* If unlinked, first instance saves data too */
    if (vmad_dentry_unlinked(dentry) && !cr_insert_object(proc_req->req->map, inode, inode, GFP_KERNEL)) {
	loff_t size = open_file.i_size;
	loff_t src_pos = 0;
	loff_t tmp = cr_sendfile(eb, cf_filp, filp, &src_pos, size);
	if (tmp != size) {
            CR_ERR_PROC_REQ(proc_req, "%s: copy-out of unlinked file returned %d", __FUNCTION__, (int)tmp);
	    retval = (tmp < 0) ? tmp : -EIO;
            goto out;
        }
    }

out:
    return retval;
}

static int 
cr_save_open_dir(cr_chkpt_proc_req_t *proc_req, struct file *filp)
{
    cr_errbuf_t *eb = proc_req->req->errbuf;
    struct file *cf_filp = proc_req->file;                /* the context file */
    struct cr_open_dir open_dir;
    int retval = -EBADF;

    open_dir.cr_type = cr_open_dir_obj;
    /* XXX:  Fix to use better representation. */
    open_dir.i_mode  = filp->f_dentry->d_inode->i_mode;
    open_dir.f_flags = filp->f_flags;
    open_dir.f_pos   = filp->f_pos;

    /* write out the open_dir struct */
    retval = cr_kwrite(eb, cf_filp, &open_dir, sizeof(open_dir));
    if (retval != sizeof(open_dir)) {
        CR_ERR_PROC_REQ(proc_req, "open_dir: write returned %d", retval);
        goto out;
    }

    retval = cr_save_filename(eb, cf_filp, filp, NULL, 0);
    if (retval < 0) {
        CR_ERR_PROC_REQ(proc_req, "cr_save_open_dir - Bad file write (filename)!");
    }

out:
    return retval;
}

static int 
cr_save_open_link(cr_chkpt_proc_req_t *proc_req, struct file *filp)
{
    int retval = -ENOSYS;

    return retval;
}

static int 
cr_save_open_socket(cr_chkpt_proc_req_t *proc_req, struct file *filp)
{
    int retval = -ENOSYS;

    /* XXX:  Hack to do absolutely nothing here... */
    CR_WARN_PROC_REQ(proc_req, "warning: skipped a socket.");
    retval = 0;

    return retval;
}

/*
 * Saves major and minor numbers, plus mode and flags for reopen.
 */
static int 
cr_save_open_chr(cr_chkpt_proc_req_t *proc_req, struct file *filp)
{
    cr_errbuf_t *eb = proc_req->req->errbuf;
    struct cr_chrdev cf_chrdev;
    struct file *cf_filp = proc_req->file;
    struct inode *inode;
    int retval;

    retval = -EINVAL;
    if (!filp) 
      goto out;

    inode = filp->f_dentry->d_inode;

    cf_chrdev.cr_type  = cr_open_chr;
    if (cr_task_tty(current) && (cr_task_tty(current) == (struct tty_struct *)filp->private_data)) {
	/* Map CTTY -> /dev/tty */
	cf_chrdev.cr_major = TTYAUX_MAJOR;
	cf_chrdev.cr_minor = 0;
    } else {
	cf_chrdev.cr_major = MAJOR(inode->i_rdev); 
	cf_chrdev.cr_minor = MINOR(inode->i_rdev);
    }
    cf_chrdev.i_mode  = filp->f_dentry->d_inode->i_mode;
    cf_chrdev.f_flags = filp->f_flags;

    retval = cr_kwrite(eb, cf_filp, &cf_chrdev, sizeof(cf_chrdev));
    if (retval != sizeof(cf_chrdev)) {
        CR_ERR_PROC_REQ(proc_req, "open_chr: write returned %d", retval);
	goto out;
    }
    retval = 0;

out:
    return retval;
}

static int 
cr_save_open_blk(cr_chkpt_proc_req_t *proc_req, struct file *filp)
{
    int retval = -ENOSYS;

    return retval;
}

/*
 * dup info is associated with a file descriptor, not a file,
 * so it's saved in cr_save_file_info, and nothing is saved here,
 * other than a placeholder data structure.
 */
static int 
cr_save_open_dup(cr_chkpt_proc_req_t *proc_req, struct file *filp)
{
    cr_errbuf_t *eb = proc_req->req->errbuf;
    struct cr_dup cf_dup;
    struct file *cf_filp = proc_req->file;
    int retval;

    retval = -EINVAL;
    if (!filp) 
      goto out;

    /* placeholder... just in case we need to do something later */
    cf_dup.cr_type  = cr_open_dup;

    retval = cr_kwrite(eb, cf_filp, &cf_dup, sizeof(cf_dup));
    if (retval != sizeof(cf_dup)) {
        CR_ERR_PROC_REQ(proc_req, "open_dup: write returned %d", retval);
	goto out;
    }
    retval = 0;

out:
    return retval;
}

static int
cr_save_open_chkpt_req(cr_chkpt_proc_req_t *proc_req, struct file *filp)
{
    /* Only differences from regular files are at restart */
    /* XXX: could save time/space by specializing? */
    return cr_save_open_file(proc_req, filp);
}

static int
cr_save_file_locks(cr_chkpt_proc_req_t *proc_req, struct file *filp)
{
    int retval = -ENOSYS;

    return retval;
}

static int
cr_save_all_files(cr_chkpt_proc_req_t *proc_req)
{
    int retval;
    int fd;
    struct file *cf_filp = proc_req->file;
    struct cr_file_info file_info;
    struct file *filp;
    int max_fds;

    CR_KTRACE_FUNC_ENTRY("");

    /* save the files info, and get max_fds as a side-effect */
    CR_KTRACE_HIGH_LVL("    ...files_struct");
    retval = cr_save_files_struct(proc_req, current->files);
    if (retval < 0) {
        CR_ERR_PROC_REQ(proc_req, "cr_save_all_files: Error saving files_struct");
        goto out_nolocks;
    }
    max_fds = retval;

    CR_KTRACE_HIGH_LVL("    ...files");
    /* now save the per file info */
    spin_lock(&current->files->file_lock);
    for (fd = 0; fd < max_fds; ++fd) {
        /*
         * We have to do our own fget here to avoid a possible race on
         * file close.  (Probably impossible, but just to be on the safe
         * side.
         */

        /* loop around again if the file is not open or not to be saved */
        filp = fcheck(fd);
        if (! filp) {
            continue;
        }
        if (filp == cf_filp) {
            CR_KTRACE_LOW_LVL("fd %d is the context file - skipping", fd);
            continue;
        }

        get_file(filp);

        // CR_KTRACE_LOW_LVL("fd %d is open", fd);

	memset(&file_info, 0, sizeof(file_info));
	file_info.cr_type = cr_file_info_obj;

        /* now save data for this file descriptor */
        cr_get_fd_info(current->files, fd, &file_info);

        spin_unlock(&current->files->file_lock);

        /* Ensure up-to-date inode information (e.g. on a network fs) */
        retval = cr_fstat(proc_req->req->map, filp);
        if (retval) {
            CR_ERR_PROC_REQ(proc_req, "Unable to fstat() file"); 
            fput(filp);
            goto out_nolocks;
        }

        /* write out the file header */
        retval = cr_get_file_info(proc_req, filp, &file_info);
        if (retval) {
            retval = -EBADF;
            CR_ERR_PROC_REQ(proc_req, "Unable to determine file info!"); 
            fput(filp);
            goto out_nolocks;
        }
        retval = cr_save_file_info(proc_req, &file_info);
        if (retval < 0) {
            CR_ERR_PROC_REQ(proc_req, "%s: cr_save_file_info failed", __FUNCTION__);
            fput(filp);
            goto out_nolocks;
        }

        switch(file_info.cr_file_type) {
        case cr_open_file:
            CR_KTRACE_LOW_LVL("    ...%d is regular file.", fd);
            retval = cr_save_open_file(proc_req, filp);
            break;
        case cr_open_directory:
            CR_KTRACE_LOW_LVL("    ...%d is open directory.", fd);
            retval = cr_save_open_dir(proc_req, filp);
            break; 
        case cr_open_link:
            CR_KTRACE_LOW_LVL("    ...%d is open symlink.", fd);
            retval = cr_save_open_link(proc_req, filp);
            break;
        case cr_open_fifo:
            CR_KTRACE_LOW_LVL("    ...%d is open fifo.", fd);
            retval = cr_save_open_fifo(proc_req, filp);
            break;
        case cr_open_socket:
            CR_KTRACE_LOW_LVL("    ...%d is open socket.", fd);
            retval = cr_save_open_socket(proc_req, filp);
            break;
        case cr_open_chr:
            CR_KTRACE_LOW_LVL("    ...%d is open character device.", fd);
            retval = cr_save_open_chr(proc_req, filp);
            break;
        case cr_open_blk:
            CR_KTRACE_LOW_LVL("    ...%d is an open block device.", fd);
            retval = cr_save_open_blk(proc_req, filp);
            break;
        case cr_open_dup:
            CR_KTRACE_LOW_LVL("    ...%d is dup of %p", fd, file_info.orig_filp);
            retval = cr_save_open_dup(proc_req, filp);
            break;
        case cr_open_chkpt_req:
            CR_KTRACE_LOW_LVL("    ...%d is a checkpoint request.", fd);
            retval = cr_save_open_chkpt_req(proc_req, filp);
            break;
        case cr_bad_file_type:
            /* fall through */
        default:
            retval = -EBADF;
            break;
        }

        if (retval < 0) {
            fput(filp);
            retval = -EBADF;
            CR_ERR_PROC_REQ(proc_req, "Unable to save open file!");
            goto out_nolocks;
        }

        /* Now write out any locks we had on the file. */
        // CR_KTRACE_LOW_LVL("    ...locks on %d", fd);
        retval = cr_save_file_locks(proc_req, filp);

        /* We did the fget() manually with the lock held. */
        fput(filp);

        spin_lock(&current->files->file_lock);
    }
    spin_unlock(&current->files->file_lock);

    /* now write one last record to indicate that there are no files left. */
    memset(&file_info, 0, sizeof(file_info));
    file_info.cr_type = cr_file_info_obj;
    file_info.cr_file_type = cr_end_of_files;
    file_info.fd = -1;
    retval = cr_save_file_info(proc_req, &file_info);
    if (retval < 0) {
        CR_ERR_PROC_REQ(proc_req, "%s: cr_save_file_info failed", __FUNCTION__);
        goto out_nolocks;
    }

out_nolocks:
    return retval;
}

static int
cr_save_file_header(cr_chkpt_req_t *req, struct file *filp)
{
    int result;
    cr_errbuf_t *eb = req->errbuf;
    struct cr_context_file_header cf_fhead;

    result = 0;

    CR_KTRACE_LOW_LVL("Dumping file header");

    cf_fhead.magic[0] = 'C';
    cf_fhead.magic[1] = 'R';
    cf_fhead.version = CR_CONTEXT_VERSION;
    cf_fhead.scope = req->checkpoint_scope;
    cf_fhead.arch_id = VMAD_ARCH;
    result = cr_kwrite(eb, filp, &cf_fhead, sizeof(cf_fhead));
    if (result != sizeof(cf_fhead)) {
        CR_ERR_REQ(req, "file_header: write returned %d", result);
	goto out;
    }

    result = 0;

  out:
    return result;
}

static int
cr_save_header(cr_chkpt_proc_req_t *proc_req, struct file *filp)
{
    int result;
    struct cr_section_header cf_head;
    int thread_count = proc_req ? proc_req->thread_count : 0;
    cr_errbuf_t *eb = proc_req ? proc_req->req->errbuf : NULL;

    result = 0;

    CR_KTRACE_LOW_LVL("Dumping header for %d threads", thread_count);

    cf_head.num_threads = thread_count;

    /* If !proc_req, the rest are meaningless */

    cf_head.clone_flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND;
    if (atomic_read(&current->signal->count) > 1) {
	cf_head.clone_flags |= CLONE_THREAD | CLONE_DETACHED;
    }

    cf_head.tmp_fd = proc_req ? proc_req->tmp_fd : -1;

    result = cr_kwrite(eb, filp, &cf_head, sizeof(cf_head));
    if (result != sizeof(cf_head)) {
        CR_ERR_PROC_REQ(proc_req, "proc_header: write returned %d", result);
	goto out;
    }

    result = 0;

  out:
    return result;
}

// Create a vmadump file
static int cr_do_vmadump(cr_task_t *cr_task, int i_am_leader)
{
    cr_chkpt_req_t *req = cr_task->chkpt_req;
    cr_chkpt_proc_req_t *proc_req = cr_task->chkpt_proc_req;
    struct file *filp = proc_req->file;
    int result=0;
    loff_t bytes;

    CR_NO_LOCKS();

    /* First arrival grabs mutex and starts to work...
       later arrivals find the work is all done */
    down(&proc_req->serial_mutex);

    /* Write out the header(s) */
    if (!test_and_set_bit(0, &proc_req->done_header)) {
        /* Determine surviving thread count */
        struct list_head *l;
        int count = 0;
        list_for_each(l, &proc_req->tasks) { ++count; }
        if (count != proc_req->thread_count) {
            CR_WARN_PROC_REQ(proc_req, "Adjusting thread count for tgid %d from %d to %d",
                    current->tgid, proc_req->thread_count, count);
            proc_req->thread_count = count;
        }

        CR_KTRACE_HIGH_LVL("Preparing to dump %d threads of %s", count, current->comm);
        result = cr_save_header(proc_req, filp);
        if (result < 0) {
            goto out_early_mutex;
        }
    }

    /* Now dump out the task linkage */
    if (!test_and_set_bit(0, &proc_req->done_linkage)) {
        CR_KTRACE_LOW_LVL("Writing the per-process linkage.");
        result = cr_save_linkage(proc_req, filp);
        if (result < 0) {
	    goto out_early_mutex;
        }
    }
    up(&proc_req->serial_mutex);

    /* Now use vmadump to write out the state for the thread.
       ALL threads call this and serialization is done within */
    CR_KTRACE_LOW_LVL("Writing vmadump.");
    CR_ASSERT_STEP_EQ(cr_task, CR_CHKPT_STEP_VMADUMP);

    result = 0;
    bytes = cr_freeze_threads(proc_req, req->flags, i_am_leader);
    if (bytes < 0) {
	result = (int)bytes;
	CR_ERR_PROC_REQ(proc_req, "cr_freeze_threads failed (%d)", result);
	CR_BARRIER_NOTIFY(cr_task, &proc_req->vmadump_barrier);
	goto out;
    }
    CR_KTRACE_LOW_LVL("thread finished vmadump");

    /* Need a barrier here to ensure all threads write their regs before the next
     * write to the file.
     */
    if (CR_BARRIER_ENTER(cr_task, &proc_req->vmadump_barrier)) {
	CR_KTRACE_LOW_LVL("process finished vmadump");
    }

    down(&proc_req->serial_mutex);
    if (!test_and_set_bit(0, &proc_req->done_fs)) {
        /* dump fs_struct (cwd, umask, etc.) */
        CR_KTRACE_HIGH_LVL("Writing the fs struct...");
        result = cr_save_fs_struct(proc_req);
        if (result < 0) {
	    goto out_mutex;
        }
    }

    if (!test_and_set_bit(0, &proc_req->done_mmaps_maps)) {
        CR_KTRACE_HIGH_LVL("Writing the mmap()s table (if any)...");
        result = cr_save_mmaps_maps(proc_req);
        if (result < 0) {
	    goto out_mutex;
        }
    }

    if (!test_and_set_bit(0, &proc_req->done_itimers)) {
        /* itimers */
        CR_KTRACE_HIGH_LVL("Writing POSIX interval timers...");
        result = cr_save_itimers(proc_req);
        if (result < 0) {
            goto out_mutex;
        }
    }

    /* Wait for all tasks in all procs to leave user space before saving files
     * (due to pipe bufs) and shared memory, to ensure shared state is not changing.
     * XXX: We could/should replace this barrier w/ one much later by moving the
     * save of shared state to the end of the context file, after all processes
     * have saved VMDump+files.
     * NOTE: This is a WAIT (as opposed to ENTER), so it doesn't matter if one or
     * many threads do this.  So, it is inside the serial_mutex-protected critical
     * section for simplicity.
     */
    CR_ASSERT_STEP_GT(cr_task, CR_CHKPT_STEP_PRESHARED);
    cr_barrier_wait(&req->preshared_barrier);

    if (!test_and_set_bit(0, &proc_req->done_mmaps_data)) {
        CR_KTRACE_HIGH_LVL("Writing mmap()ed pages (if any)...");
        bytes = cr_save_mmaps_data(proc_req);
        if (bytes < 0) {
	    result = (int)bytes;
	    goto out_mutex;
        }
    }

    if (!test_and_set_bit(0, &proc_req->done_files)) {
        /* dump the open files */
        CR_KTRACE_HIGH_LVL("Writing the open file section...");
        result = cr_save_all_files(proc_req);
        if (result < 0) {
	    goto out_mutex;
        }
    }

    result = 0; // XXX
out_mutex:
    up(&proc_req->serial_mutex);
out:
    return result;

out_early_mutex:
    CR_BARRIER_NOTIFY(cr_task, &proc_req->vmadump_barrier);
    goto out_mutex;
}

// Dump out to the given file
//
// XXX: When we define an options struct, it will need to be passed in too

/*
 * XXX:  Might want to change this a little (lot) when we support multiple
 * context files.  Could be a separate format for directory set, etc.  i.e.
 * cr_format_contextfile, cr_format_contextdir?
 */
static int cr_do_dump(int dump_format, cr_task_t *cr_task, int i_am_leader)
{
	int result = -EINVAL;

	switch(dump_format) {
	case cr_format_vmadump:
		result = cr_do_vmadump(cr_task, i_am_leader);
		break;

	default:
		CR_ERR_PROC_REQ(cr_task->chkpt_proc_req, "unrecognized dump_format '%d'", 
				  dump_format);
		break;
	}

	return result;
}


/*
 * Public functions
 */

/**
 * cr_dump_self - dump 'current'
 * @filp: The file on which the request was received.
 * @flag: Flags to modify behavior.
 *
 * DESCRIPTION:
 * Dumps the current process to a file descriptor supplied by the
 * requester.
 *
 * This is the function that the handlers will call into when
 * the CHECKPOINT portion of callbacks are finished.
 *
 * Currently the 'flags' argument is unused.
 */
int cr_dump_self(struct file *filp, unsigned long flags)
{
	cr_chkpt_proc_req_t *proc_req;
	struct file *dest_filp;
	cr_chkpt_req_t *req;
	cr_task_t *cr_task;
	sigset_t sig_blocked;
	int shared;
	int omit = 0;
	int result = 0;
	int once;

	CR_KTRACE_FUNC_ENTRY("flags=0x%lx", flags);

	// Lookup this task
	cr_task = cr_task_get(current);
	if (!cr_task) {
		// No matching task found.
		return -ESRCH;
	}

	// Find the matching request
	if (!(req = cr_task->chkpt_req) || !(proc_req = cr_task->chkpt_proc_req)) {
		// No matching request found.
		cr_task_put(cr_task);
		return -ESRCH;
	}

	cr_task->chkpt_flags = flags;

	// Ensure shared state save can start as soon as all tasks reach kernel space
        CR_ASSERT_STEP_EQ(cr_task, CR_CHKPT_STEP_PRESHARED);
	CR_BARRIER_NOTIFY(cr_task, &req->preshared_barrier);
	once = cr_barrier_test(&req->preshared_barrier);

	// If the handlers have all responded, then we should not "expire".
	// So exactly one task zeros req->has_expiration.
	if (req->has_expiration && once) {
		write_lock(&req->lock);
		req->has_expiration = 0;
		write_unlock(&req->lock);
	}

	// XXX: Once we define the options structure, this is the place
	// to merge the per-process options (attached to 'filp'?) and the
	// options passed by the requester.

        // Find the destination
	dest_filp = cr_loc_get(&req->dest, &shared);
	if (IS_ERR(dest_filp)) {
		return PTR_ERR(dest_filp);
	}

	// Before we go off and block on any barriers, block all but SIGKILL.
	// NOTE: this even blocks SIGSTOP!
	// The previous mask is saved in sig_blocked.
	siginitsetinv(&sig_blocked, sigmask(SIGKILL));
	sigprocmask(SIG_SETMASK, &sig_blocked, &sig_blocked);

	// Ensure that exactly one caller (over all procs) writes the file header
	down(&req->serial_mutex);
	read_lock(&req->lock);
	if ((omit = proc_req->omit) || (result = req->result)) {
		goto cleanup_locked_and_down;
	}
	read_unlock(&req->lock);
	if (!test_and_set_bit(0, &req->done_header)) {
            result = cr_save_file_header(req, dest_filp);
            if (result < 0) {
		req->result = result;
            }
	    /* result is checked after phase barrier */
	}
	up(&req->serial_mutex);

	// If we have a Phase1 then start Phase2 when complete.
	cr_signal_phase_barrier(cr_task, /* block= */1, /* need_lock= */1);

	/* Check to see if error/abort has occurred */
	read_lock(&req->lock);
	if (result || (omit = proc_req->omit) || (result = req->result)) {
		goto cleanup_locked;
	}
	read_unlock(&req->lock);

	// Synchronize to ensure all tasks in the current process have stopped running.
	once = cr_signal_predump_barrier(cr_task, /* block= */ 1);
	if (once < 0) {
		goto cleanup_unlocked;
	}

	/* check again for abort/error */
	read_lock(&req->lock);
	if (result || (omit = proc_req->omit) || (result = req->result)) {
		goto cleanup_locked;
	}
	read_unlock(&req->lock);

	// One task pauses the itimers
	if (once) {
		cr_pause_itimers(proc_req->itimers);
		// vmadump_barrier ensures this is written before reading
	}
	
	// Ensure that exactly one caller (per procs) performs the init 
	down(&proc_req->serial_mutex);
	if (!test_and_set_bit(0, &proc_req->done_init)) {
	    proc_req->file = dest_filp;
	    /* Acquire dest mutex (if any) on behalf of this process */
	    if (shared)  {
		down(&req->dest.mutex);
	    }
        }
	up(&proc_req->serial_mutex);

	// Send the dump to the proper destination filp.
	{
            // Distinguish a single thread group leader (no race here) */
	    int i_am_leader = (thread_group_leader(current) && !test_and_set_bit(0, &proc_req->have_leader));

	    result = cr_do_dump(req->dump_format, cr_task, i_am_leader);

	    // If we experienced an error for the first time then
	    // save it in the request so the requester will learn of it.
	    if (result && !req->result) {
		    req->result = result;
	    }
	}

	// Release dest mutex (if any) exactly once
	// NOTE: assumes ALL writes complete before ANY return from cr_do_dump().
	if (!test_and_set_bit(0, &proc_req->done_fini) && shared) {
	    up(&req->dest.mutex);
	}

	// Synchronize to ensure all tasks have saved before we continue
        CR_ASSERT_STEP_EQ(cr_task, CR_CHKPT_STEP_POSTDUMP);
	result = CR_BARRIER_ENTER_INTERRUPTIBLE(cr_task, &req->postdump_barrier);

	// Ensure that exactly one caller (across all processes) writes the file trailer
	down(&req->serial_mutex);
	if (result < 0) { // Check postdump_barrier result
	    req->result = result;
	} else if (!test_and_set_bit(0, &req->done_trailer)) {
            CR_KTRACE_LOW_LVL("Writing the trailer.");
            result = cr_save_header(NULL, dest_filp);
            if (result < 0) {
		req->result = result;
            }
        }
	result = req->result;
	up(&req->serial_mutex);
	if (result) {
	    goto out;
	}

	// One task resumes the itimers (and "early" signal)
        if (!test_and_set_bit(0, &proc_req->done_resume_itimers)) {
            cr_resume_itimers(proc_req->itimers);
	    if (req->signal < 0) {
		cr_kill_process(current, -(req->signal));
	    }
        }

out:
	atomic_inc(&req->completed);
	cr_loc_put(&req->dest, dest_filp);
	if (req->die || result || (flags & _CR_CHECKPOINT_STUB)) {
		// this task will not call the HAND_DONE ioctl, so finish up now.
		cr_chkpt_task_complete(cr_task, 1);
	}

out_omit:
	// Restore saved signal mask
	sigprocmask(SIG_SETMASK, &sig_blocked, NULL);

	// Release the task, balancing the cr_task_get() above.
	cr_task_put(cr_task);

	CR_KTRACE_HIGH_LVL("Kernel-level checkpoint completed.");
	CR_KTRACE_FUNC_EXIT("Returning %d", result);
	return result;

	//--------------------
cleanup_locked_and_down:
	up(&req->serial_mutex);
cleanup_locked:
	read_unlock(&req->lock);
cleanup_unlocked:
	if (omit) {
	    cr_loc_put(&req->dest, dest_filp); // BEFORE abort releases req
	    cr_chkpt_abort(cr_task, CR_CHECKPOINT_OMIT);
	    result = -CR_EOMITTED;
	    goto out_omit;
	}
	cr_chkpt_advance_to(cr_task, CR_CHKPT_STEP_PRE_COMPLETE, 0);
	CR_ASSERT_STEP_GT(cr_task, CR_CHKPT_STEP_POSTDUMP);
	cr_barrier_wait_interruptible(&req->postdump_barrier);
	goto out;
}
