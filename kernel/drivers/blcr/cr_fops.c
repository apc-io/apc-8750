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
 * $Id: cr_fops.c,v 1.69 2008/08/16 00:17:09 phargrov Exp $
 */

#include "cr_module.h"

/**
 * cr_hand_complete - wrapper/helper for CR_OP_HAND_DONE
 * @file: (struct file *) of the control node
 *
 * DESCRIPTION:
 * This function dispatches to the checkpoint-time or restart-time
 * post-handler "task_complete" code.
 */
int cr_hand_complete(struct file *filp, unsigned int flags)
{
    cr_task_t *cr_task;
    int retval;

    CR_KTRACE_FUNC_ENTRY();

    retval = -EINVAL;
    if (flags & ~CR_HOLD_BOTH) {
        CR_ERR("%s: invalid flags", __FUNCTION__);
	goto out_no_task;
    }

    // Lookup this task
    retval = -ESRCH;
    cr_task = cr_task_get(current);
    if (!cr_task) {
	// No matching task found.
        CR_ERR("%s: No matching task found!", __FUNCTION__);
	goto out_no_task;
    }

    // Dispatch according to the matching request
    if (cr_task->chkpt_proc_req) {
	retval = cr_chkpt_task_complete(cr_task, flags & CR_HOLD_CONT);
    } else if (cr_task->rstrt_proc_req) {
	retval = cr_rstrt_task_complete(cr_task, flags & CR_HOLD_RSTRT, /* need_lock= */ 1);
    } else {
	// No matching request found.
        CR_ERR("%s: No matching request found!", __FUNCTION__);
    }

    cr_task_put(cr_task);

out_no_task:
    CR_KTRACE_FUNC_EXIT("Returning %d", retval);
    return retval;
}

/**
 * cr_hand_abort - wrapper/helper for CR_OP_HAND_ABORT
 * @file: (struct file *) of the control node
 * @flags: abort type
 *
 * DESCRIPTION:
 * This function dispatches to the checkpoint-time or restart-time
 * "abort" code.
 */
int cr_hand_abort(struct file *filp, unsigned int flags)
{
    cr_task_t *cr_task;
    int retval;

    CR_KTRACE_FUNC_ENTRY();

    // Lookup this task
    retval = -ESRCH;
    cr_task = cr_task_get(current);
    if (!cr_task) {
	// No matching task found.
        CR_ERR("%s: No matching task found!", __FUNCTION__);
	goto out_no_task;
    }

    // Dispatch according to the matching request
    if (cr_task->chkpt_proc_req) {
	retval = cr_chkpt_abort(cr_task, flags);
    } else if (cr_task->rstrt_proc_req) {
	retval = cr_rstrt_abort(cr_task, flags);
    } else {
	// No matching request found.
        CR_ERR("%s: No matching request found!", __FUNCTION__);
    }

    cr_task_put(cr_task);

out_no_task:
    CR_KTRACE_FUNC_EXIT("Returning %d", retval);
    return retval;
}

/**
 * ctrl_ioctl - ioctl() method for the C/R control node
 * @inode: (struct inode *) of the control node
 * @file: (struct file *) of the control node
 * @op: The operation/command to perform
 * @arg: The (optional) argument to the command as an unsigned long
 *
 * DESCRIPTION:
 * This function is the ioctl() method in the file_operations for
 * the C/R control node.  It acts as a dispatcher to the functions
 * which do the real work.
 */
static int ctrl_ioctl(struct inode *inode, struct file *file,
		      unsigned int op, unsigned long arg)
{
	CR_KTRACE_FUNC_ENTRY("op=%08x arg=0x%lx", op, arg);

	switch(op) {
	//
	// Calls from libcr:
	//
	// XXX: When adding cases here, also add to the ioctl32 init/cleanup
	//      code below and to the big switch in cr_compat.c.
	case CR_OP_HAND_CHKPT:
		return cr_dump_self(file, arg);

	case CR_OP_HAND_ABORT:
		return cr_hand_abort(file, arg);

	case CR_OP_HAND_SUSP:
		return cr_suspend(file, (struct timeval __user *)arg);

	case CR_OP_HAND_PHASE1:
		return cr_phase1_register(file, (int)arg);

	case CR_OP_HAND_PHASE2:
		return cr_phase2_register(file, (int)arg);

	case CR_OP_HAND_SRC:
		return cr_rstrt_src(file, (char __user *)arg);

	case CR_OP_HAND_CHKPT_INFO:
		return cr_chkpt_info(file, (struct cr_chkpt_info __user *)arg);

	case CR_OP_HAND_DONE:
		return cr_hand_complete(file, arg);


	//
	// Calls from cr_checkpoint:
	//
	case CR_OP_CHKPT_REQ:
		return cr_chkpt_req(file, (struct cr_chkpt_args __user *)arg);

	case CR_OP_CHKPT_REAP:
		return cr_chkpt_reap(file);

	case CR_OP_CHKPT_FWD:
		return cr_chkpt_fwd(file, (struct cr_fwd_args __user *)arg);

	case CR_OP_CHKPT_LOG:
		return cr_chkpt_log(file, (struct cr_log_args __user *)arg);

	//
	// Calls from cr_restart:
	//
	case CR_OP_RSTRT_REQ:
		return cr_rstrt_request_restart(file, (struct cr_rstrt_args __user *)arg);

	case CR_OP_RSTRT_REAP:
		return cr_rstrt_reap(file);

	case CR_OP_RSTRT_CHILD:
		return cr_rstrt_child(file);

	case CR_OP_RSTRT_PROCS:
		return cr_rstrt_procs(file, (struct cr_procs_tbl __user *)arg);

	case CR_OP_RSTRT_LOG:
		return cr_rstrt_log(file, (struct cr_log_args __user *)arg);

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

static int ctrl_open(struct inode * inode, struct file * file)
{
	cr_pdata_t *priv;

	CR_KTRACE_FUNC_ENTRY();

	CR_NO_LOCKS();
	priv = cr_kmem_cache_zalloc(cr_pdata_t, cr_pdata_cachep, GFP_KERNEL);
	if (priv) {
		file->private_data = (void*)priv;
	}

	return priv ? 0 : -ENOMEM;
}

static int ctrl_release(struct inode * inode, struct file * file)
{
	cr_pdata_t *priv;

	CR_KTRACE_FUNC_ENTRY();

	priv = file->private_data;
	if (priv) {
		cr_chkpt_req_release(file, priv);
		cr_rstrt_req_release(file, priv);
		cr_phase1_release(file, priv);
		cr_phase2_release(file, priv);
		kmem_cache_free(cr_pdata_cachep, priv);
		file->private_data = NULL;
	}

	return 0;
}

static unsigned int ctrl_poll(struct file *filp, poll_table *wait)
{
	unsigned int mask;
	cr_pdata_t *priv;

	CR_KTRACE_FUNC_ENTRY();

	priv = filp->private_data;
        if (priv && priv->rstrt_req) {
		mask = cr_rstrt_poll(filp, wait);
	} else if (priv && priv->chkpt_req) {
		mask = cr_chkpt_poll(filp, wait);
	} else {
		mask = POLLERR;
	}

	return mask;
}

/*
 * Ops for the C/R control node
 */
struct file_operations cr_ctrl_fops =
{
	owner:		THIS_MODULE,
	open:		ctrl_open,
	release:	ctrl_release,
	ioctl:		ctrl_ioctl,
	poll:		ctrl_poll,
#if defined(CONFIG_COMPAT) && defined(HAVE_COMPAT_IOCTL)
	compat_ioctl:	cr_compat_ctrl_ioctl,
#endif
};


/*
 * Ops for files we didn't/couldn't reconnect.
 * XXX: These will get more tuning as time goes on.
 */
#ifdef CR_ENABLE_BOGUS_FOPS

static loff_t cr_llseek_EIO(struct file *f, loff_t o, int w)
	{ return (loff_t)(-EIO); }
static ssize_t cr_read_EIO(struct file *f, char *b, size_t s, loff_t *o)
	{ return (ssize_t)(-EIO); }
static ssize_t cr_write_EIO(struct file *f, const char *b, size_t s, loff_t *o)
	{ return (ssize_t)(-EIO); }
static int cr_readdir_EIO(struct file *f, void *b, filldir_t fd)
	{ return -EIO; }
static int cr_ioctl_EIO(struct inode *i, struct file *f, unsigned int cmd, unsigned long op)
	{ return -EIO; }
static int cr_mmap_EIO(struct file *f, struct vm_area_struct *v)
	{ return -EIO; }
static int cr_open_EIO(struct inode *i, struct file *f)
	{ return -EIO; }
static int cr_flush_EIO(struct file *f)
	{ return -EIO; }
static int cr_fsync_EIO(struct file *f, struct dentry *d, int s)
	{ return -EIO; }
static int cr_fasync_EIO(int fd, struct file *f, int a)
	{ return -EIO; }
static int cr_lock_EIO(struct file *f, int op, struct file_lock *fl)
	{ return -EIO; }
static ssize_t cr_readv_EIO(struct file *f, const struct iovec *i, unsigned long c, loff_t *o)
	{ return (ssize_t)(-EIO); }
static ssize_t cr_writev_EIO(struct file *f, const struct iovec *i, unsigned long c, loff_t *o)
	{ return (ssize_t)(-EIO); }
#if 0 /* Not present in all kernels */
static ssize_t cr_sendpage_EIO(struct file *f, struct page *p, int off, size_t s, loff_t *o, int fl)
	{ return (ssize_t)(-EIO); }
static unsigned long cr_get_unmapped_area_EIO(struct file *f, unsigned long a, unsigned long b, unsigned long c, unsigned long d)
	{ return (unsigned long)(-EIO); }
#endif

struct file_operations cr_generic_bogus_fops =
{
        owner: THIS_MODULE,
        poll: NULL,	/* XXX: Not yet sure what we want to do here */
        release: NULL,	/* Just let the close proceed */
        llseek: cr_llseek_EIO,
        read: cr_read_EIO,
        write: cr_write_EIO,
        readdir: cr_readdir_EIO,
        ioctl: cr_ioctl_EIO,
        mmap: cr_mmap_EIO,
        open: cr_open_EIO,
        flush: cr_flush_EIO,
        fsync: cr_fsync_EIO,
        fasync: cr_fasync_EIO,
        lock: cr_lock_EIO,
        readv: cr_readv_EIO,
        writev: cr_writev_EIO,
#if 0 /* Not present in all kernels */
        sendpage: cr_sendpage_EIO,
        get_unmapped_area: cr_get_unmapped_area_EIO,
#endif
};

/* based on pipefs code from linux/fs/pipe.c */
static struct vfsmount *cr_blcr_mnt;
static int cr_delete_dentry(struct dentry *dentry) { return 1; }
static struct dentry_operations cr_dentry_operations = {
        d_delete: cr_delete_dentry,
};
struct inode * cr_get_bogus_inode(umode_t mode)
{
    struct inode *inode = new_inode(cr_blcr_mnt->mnt_sb);
                                                                                                              
    if (inode) {
	inode->i_fop = &cr_generic_bogus_fops;
	inode->i_state = I_DIRTY; /* so mark_inode_dirty() will ignore us */
	inode->i_mode = mode;
	inode->i_uid = current->fsuid;
	inode->i_gid = current->fsgid;
	inode->i_atime = inode->i_mtime = inode->i_ctime = CURRENT_TIME;
	inode->i_blksize = PAGE_SIZE;
    }

    return inode;
}
#define BLCRFS_MAGIC 0x626c6372	/* 'b' 'l' 'c' 'r' */
static int blcrfs_statfs(struct super_block *sb, struct statfs *buf)
{
    buf->f_type = BLCRFS_MAGIC;
    buf->f_bsize = 1024;
    buf->f_namelen = 255;
    return 0;
}
static struct super_operations blcrfs_ops = {
	statfs:		blcrfs_statfs,
};
static struct super_block * blcrfs_read_super(struct super_block *sb, void *data, int silent)
{
	struct inode *root = new_inode(sb);
	if (!root)
		return NULL;
	root->i_mode = S_IFDIR | S_IRUSR | S_IWUSR;
	root->i_uid = root->i_gid = 0;
	root->i_atime = root->i_mtime = root->i_ctime = CURRENT_TIME;
	sb->s_blocksize = 1024;
	sb->s_blocksize_bits = 10;
	sb->s_magic = BLCRFS_MAGIC;
	sb->s_op = &blcrfs_ops;
	sb->s_root = d_alloc(NULL, &(const struct qstr) { "blcr:", 5, 0 });
	if (!sb->s_root) {
		iput(root);
		return NULL;
	}
	sb->s_root->d_sb = sb;
	sb->s_root->d_parent = sb->s_root;
	d_instantiate(sb->s_root, root);
	return sb;
}
static DECLARE_FSTYPE(blcr_fs_type, "blcrfs", blcrfs_read_super, FS_NOMOUNT);
int cr_fs_init(void)
{
        int err = register_filesystem(&blcr_fs_type);
#if 0	/* XXX: if we kern_mount() then we can't rmmod :-( */
        if (!err) {
                cr_blcr_mnt = kern_mount(&blcr_fs_type);
                err = PTR_ERR(cr_blcr_mnt);
                if (IS_ERR(cr_blcr_mnt))
                        unregister_filesystem(&blcr_fs_type);
                else
                        err = 0;
        }
#endif
        return err;
}
int cr_fs_cleanup(void)
{
        unregister_filesystem(&blcr_fs_type);
#if 0
        mntput(cr_blcr_mnt);
#endif
	return 0;
}
#endif /* CR_ENABLE_BOGUS_FOPS */

#if defined(CONFIG_COMPAT) && !defined(HAVE_COMPAT_IOCTL)

#include <linux/ioctl32.h>

static int ctrl_ioctl32(unsigned int fd, unsigned int op, unsigned long arg, struct file *filp) {
  return cr_compat_ctrl_ioctl(filp, op, arg);
}

int cr_init_ioctl32(void)
{
	register_ioctl32_conversion(CR_IOCTL32_MAP(CR_OP_HAND_CHKPT), &ctrl_ioctl32);
	register_ioctl32_conversion(CR_IOCTL32_MAP(CR_OP_HAND_ABORT), &ctrl_ioctl32);
	register_ioctl32_conversion(CR_IOCTL32_MAP(CR_OP_HAND_SUSP), &ctrl_ioctl32);
	register_ioctl32_conversion(CR_IOCTL32_MAP(CR_OP_HAND_PHASE1), &ctrl_ioctl32);
	register_ioctl32_conversion(CR_IOCTL32_MAP(CR_OP_HAND_PHASE2), &ctrl_ioctl32);
	register_ioctl32_conversion(CR_IOCTL32_MAP(CR_OP_HAND_SRC), &ctrl_ioctl32);
	register_ioctl32_conversion(CR_IOCTL32_MAP(CR_OP_HAND_CHKPT_INFO), &ctrl_ioctl32);
	register_ioctl32_conversion(CR_IOCTL32_MAP(CR_OP_HAND_DONE), &ctrl_ioctl32);
	register_ioctl32_conversion(CR_IOCTL32_MAP(CR_OP_CHKPT_REQ), &ctrl_ioctl32);
	register_ioctl32_conversion(CR_IOCTL32_MAP(CR_OP_CHKPT_REAP), &ctrl_ioctl32);
	register_ioctl32_conversion(CR_IOCTL32_MAP(CR_OP_CHKPT_FWD), &ctrl_ioctl32);
	register_ioctl32_conversion(CR_IOCTL32_MAP(CR_OP_CHKPT_LOG), &ctrl_ioctl32);
	register_ioctl32_conversion(CR_IOCTL32_MAP(CR_OP_RSTRT_REQ), &ctrl_ioctl32);
	register_ioctl32_conversion(CR_IOCTL32_MAP(CR_OP_RSTRT_REAP), &ctrl_ioctl32);
	register_ioctl32_conversion(CR_IOCTL32_MAP(CR_OP_RSTRT_CHILD), &ctrl_ioctl32);
	register_ioctl32_conversion(CR_IOCTL32_MAP(CR_OP_RSTRT_PROCS), &ctrl_ioctl32);
	register_ioctl32_conversion(CR_IOCTL32_MAP(CR_OP_RSTRT_LOG), &ctrl_ioctl32);
	register_ioctl32_conversion(CR_IOCTL32_MAP(CR_OP_VERSION), &ctrl_ioctl32);
	return 0;
}

int cr_cleanup_ioctl32(void)
{
	unregister_ioctl32_conversion(CR_IOCTL32_MAP(CR_OP_HAND_CHKPT));
	unregister_ioctl32_conversion(CR_IOCTL32_MAP(CR_OP_HAND_ABORT));
	unregister_ioctl32_conversion(CR_IOCTL32_MAP(CR_OP_HAND_SUSP));
	unregister_ioctl32_conversion(CR_IOCTL32_MAP(CR_OP_HAND_PHASE1));
	unregister_ioctl32_conversion(CR_IOCTL32_MAP(CR_OP_HAND_PHASE2));
	unregister_ioctl32_conversion(CR_IOCTL32_MAP(CR_OP_HAND_SRC));
	unregister_ioctl32_conversion(CR_IOCTL32_MAP(CR_OP_HAND_CHKPT_INFO));
	unregister_ioctl32_conversion(CR_IOCTL32_MAP(CR_OP_HAND_DONE));
	unregister_ioctl32_conversion(CR_IOCTL32_MAP(CR_OP_CHKPT_REQ));
	unregister_ioctl32_conversion(CR_IOCTL32_MAP(CR_OP_CHKPT_REAP));
	unregister_ioctl32_conversion(CR_IOCTL32_MAP(CR_OP_CHKPT_FWD));
	unregister_ioctl32_conversion(CR_IOCTL32_MAP(CR_OP_CHKPT_LOG));
	unregister_ioctl32_conversion(CR_IOCTL32_MAP(CR_OP_RSTRT_REQ));
	unregister_ioctl32_conversion(CR_IOCTL32_MAP(CR_OP_RSTRT_REAP));
	unregister_ioctl32_conversion(CR_IOCTL32_MAP(CR_OP_RSTRT_CHILD));
	unregister_ioctl32_conversion(CR_IOCTL32_MAP(CR_OP_RSTRT_PROCS));
	unregister_ioctl32_conversion(CR_IOCTL32_MAP(CR_OP_RSTRT_LOG));
	unregister_ioctl32_conversion(CR_IOCTL32_MAP(CR_OP_VERSION));
	return 0;
}
#endif
