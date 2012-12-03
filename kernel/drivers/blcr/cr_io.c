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
 * $Id: cr_io.c,v 1.77.4.2 2009/03/26 04:22:43 phargrov Exp $
 */

/*
 * Implements basic read and write calls for kernel space
 */

#include "cr_module.h"

#include <asm/uaccess.h>
#include <linux/dnotify.h>
#include <linux/mman.h>

#if CRI_DEBUG
  /* Rates (as in 1-in-X) for artificial I/O faults */
  #include <linux/random.h>
  unsigned int cr_read_fault_rate = 0;
  unsigned int cr_write_fault_rate = 0;
#endif

/* Thin wrappers for args added over time: */

#if HAVE_4_ARG_VFS_MKNOD
  #define cr_vfs_mknod(_i,_d,_v,_m,_dev) vfs_mknod((_i),(_d),(_m),(_dev))
#elif HAVE_5_ARG_VFS_MKNOD
  #define cr_vfs_mknod(_i,_d,_v,_m,_dev) vfs_mknod((_i),(_d),(_v),(_m),(_dev))
#else
  #error "Don't know how to call vfs_mknod()"
#endif

#if HAVE_2_ARG_VFS_UNLINK
  #define cr_vfs_unlink(_i,_d,_v) vfs_unlink((_i),(_d))
#elif HAVE_3_ARG_VFS_UNLINK
  #define cr_vfs_unlink(_i,_d,_v) vfs_unlink((_i),(_d),(_v))
#else
  #error "Don't know how to call vfs_unlink()"
#endif

#if HAVE_2_ARG_NOTIFY_CHANGE
  #define cr_notify_change(_d,_v,_a) notify_change((_d),(_a))
#elif HAVE_3_ARG_NOTIFY_CHANGE
  #define cr_notify_change(_d,_v,_a) notify_change((_d),(_v),(_a))
#else
  #error "Don't know how to call notify_change()"
#endif

/* Avoid requesting any I/O > 64 MB (prevents problems on busted kernels).
 * Also avoids accidentally negative return values on 32-bit
 */
#define _CR_DFLT_IO_MAX 0x4000000UL
unsigned long cr_io_max = _CR_DFLT_IO_MAX;
unsigned long cr_io_max_mask = ~(_CR_DFLT_IO_MAX - 1);
#define CR_TRIM_XFER(_bytes) ((((_bytes) & cr_io_max_mask) ? cr_io_max : (_bytes)))

/* [uk]{read,write} basically ripped off from vmadump */

/* Loops on short reads, but return -EIO if vfs_read() returns zero */
ssize_t
cr_uread(cr_errbuf_t *eb, struct file *file, void *buf, size_t count)
{
    ssize_t retval;
    ssize_t bytes_left = count;
    char *p = buf;

    while (bytes_left) {
       const ssize_t r = vfs_read(file, p, CR_TRIM_XFER(bytes_left), &file->f_pos);
       if (r <= 0) {
	   CR_ERR_EB(eb, "vfs_read returned %ld", (long int)r);
	   retval = r;
	   if (!retval) retval = -EIO; /* Map zero -> EIO */
	   goto out;
       } 
       bytes_left -= r;
       p += r;
    }
    retval = count;
#if CRI_DEBUG
    if (cr_read_fault_rate) {
	unsigned int x;
	get_random_bytes(&x, sizeof(x));
	if (!(x % cr_read_fault_rate)) {
	    CR_INFO("injecting READ fault");
	    retval = -EFAULT;
	}
    }
#endif

out:
    return retval; 
}

/* Loops on short writes, but return -EIO if vfs_write() returns zero */
ssize_t
cr_uwrite(cr_errbuf_t *eb, struct file *file, const void *buf, size_t count)
{
    ssize_t retval;
    ssize_t bytes_left = count;
    const char *p = buf;

    while (bytes_left) {
       const ssize_t w = vfs_write(file, p, CR_TRIM_XFER(bytes_left), &file->f_pos);
       if (w <= 0) {
	   CR_ERR_EB(eb, "vfs_write returned %ld", (long int)w);
	   retval = w;
	   if (!retval) retval = -EIO; /* Map zero -> EIO */
	   goto out;
       } 
       bytes_left -= w;
       p += w;
    }
    retval = count;
#if CRI_DEBUG
    if (cr_write_fault_rate) {
	unsigned int x;
	get_random_bytes(&x, sizeof(x));
	if (!(x % cr_write_fault_rate)) {
	    CR_INFO("injecting WRITE fault");
	    retval = -EFAULT;
	}
    }
#endif

out:
    return retval; 
}

ssize_t
cr_kread(cr_errbuf_t *eb, struct file *file, void *buf, size_t count)
{
    ssize_t retval;
    mm_segment_t oldfs = get_fs();
    set_fs(KERNEL_DS);
    retval = cr_uread(eb, file, buf, count);
    set_fs(oldfs);
    return retval; 
}

ssize_t
cr_kwrite(cr_errbuf_t *eb, struct file *file, const void *buf, size_t count)
{
    ssize_t retval;
    mm_segment_t oldfs = get_fs();
    set_fs(KERNEL_DS);
    retval = cr_uwrite(eb, file, buf, count);
    set_fs(oldfs);
    return retval; 
}


/* Skip unused data
 * XXX: Could/should we just seek when possible?
 */
int cr_skip(struct file *filp, loff_t len)
{
    ssize_t r;
    void *buf;
    mm_segment_t oldfs;

    r = -ENOMEM;
    CR_NO_LOCKS();
    buf = (void *) __get_free_page(GFP_KERNEL);
    if (!buf) goto out;

    oldfs = get_fs();
    set_fs(KERNEL_DS);
    while (len) {
	r = vfs_read(filp, buf, (len > PAGE_SIZE) ? PAGE_SIZE : len, &filp->f_pos);
	if (r <= 0) break;
	len -= r;
    }
    set_fs(oldfs);
    free_page((unsigned long)buf);

    if (r == 0) r = -EIO; /* EOF */
out:
    return (int)r;
}

/*
 * cr_fgets
 *
 * Sort of like fgets.
 *   Returns length (including '\0') on success, or <0 on error.
 *   0 return means string is NULL, 1 means empty string.
 */
int
cr_fgets(cr_errbuf_t *eb, char *buf, int size, struct file *filp)
{
    int ret;
    int len = 0;

    /* Argument checking */
    ret = -EINVAL;
    if (!buf) {
        CR_ERR_EB(eb, "cr_fgets: NULL buffer pass as argument!");
        goto out;
    }
    if (size <= 0) {
        CR_ERR_EB(eb, "cr_fgets: Bad buffer size %d.", size);
        goto out;
    }

    ret = cr_kread(eb,filp, &len, sizeof(len));
    if (ret != sizeof(len)) {
        CR_ERR_EB(eb, "cr_fgets: read len returned %d", ret);
        goto out;
    }
    if ((len < 0) || (len > size)) {
        ret = -EINVAL;
        CR_ERR_EB(eb, "cr_fgets: Bad string length %d in file.", len);
        goto out;
    }

    if (len == 0) {
        /* old string was NULL.  i.e. never initialized rather than a
	 * pointer to an empty string */
        goto out;
    }

    ret = cr_kread(eb, filp, buf, len);
    if (ret != len) {
        CR_ERR_EB(eb, "cr_fgets: read buf returned %d", ret);
        goto out;
    }

out:
    return (ret < 0) ? ret : len;
}

/*
 * cr_fputs
 * 
 * Slightly different than fputs, since it writes the '\0' also, and refuses to
 * write anything larger than a PATH_MAX
 * 
 * Returns number of bytes written on success.
 */
int cr_fputs(cr_errbuf_t *eb, const char *buf, struct file *filp)
{
    int ret;
    int wrote=0;
    int len;

    ret = -EIO;

    /* we actually want to know when someone wrote NULL to an area, so we
     * can restore the NULL on restart. */
    if (!buf) {
        /* magic length of 0 for NULL buf */
        len = 0;
    } else {
        /* we write out the length WITH the '\0' termination. */
        len = strlen(buf)+1;
    }

    /* compare l+1 to avoid type issue wrt promotion of len+1 to unsigned
     * long in comparison against PATH_MAX */
    if ((len < 0) || (len+1 > PATH_MAX+1)) {
        CR_ERR_EB(eb, "cr_fputs:  String length (%d) out of bounds.", len);
        ret = -EINVAL;
        goto out;
    }

    ret = cr_kwrite(eb, filp, &len, sizeof(len));
    wrote += ret;
    if (ret != sizeof(len)) {
        CR_ERR_EB(eb, "cr_fputs: write len returned %d", ret);
        goto out;
    }
  
    if (len) {
        ret = cr_kwrite(eb, filp, buf, len);
        wrote += ret;
        if (ret != len) {
            CR_ERR_EB(eb, "cr_fputs: write buf returned %d", ret);
            goto out;
        }
    }

    ret = wrote;

out:
    return ret;
}

/* Read in a filename from the context file, obtaining memory from __getname */
const char *__cr_getname(cr_errbuf_t *eb, struct file *filp, int null_ok)
{
    char *name;
    int err;

    name = __getname();
    if (!name) {
        CR_ERR_EB(eb, "Couldn't allocate buffer for file name.");
        err = -ENOMEM;
        goto out;
    }

    /* now read out the name */
    err = cr_fgets(eb, name, PATH_MAX, filp);
    if (err < 0) {
        CR_ERR_EB(eb, "Bad read of filename.");
        goto out_free;
    } else if (err == 0) {
	if (!null_ok) err = -EIO;
	goto out_free;
    }

    return name;

out_free:
    __putname(name);
out:
    return (err < 0) ? ERR_PTR(err) : NULL;
}

/* Read in a filename from the context file, obtaining memory from __getname.
 * In addition, relocation is applied to the filename.
 */
const char *cr_getname(cr_errbuf_t *eb, cr_rstrt_relocate_t reloc, struct file *filp, int null_ok)
{
    const char *path = __cr_getname(eb, filp, null_ok);
    if (path && !IS_ERR(path) && reloc) {
	path = cr_relocate_path(reloc, path, 1);
    }

    return path;
}

/*
 * Copy the given number of bytes from one file to another.
 * Uses naive approach that should work for all types.
 * This is only used when a source file lacks a readpage method
 * (e.g. restart from a pipes or sockets).
 * XXX: Could we use non-blocking writes and double buffering?
 *
 * Note: Caller is responsible for checking count==0 or src_ppos==NULL.
 */
static loff_t
cr_sendfile_buffered(cr_errbuf_t *eb, struct file *dst_filp, struct file *src_filp, loff_t *src_ppos, loff_t count)
{
    const size_t maxsz = (4 << PAGE_SHIFT);
    mm_segment_t oldfs;
    loff_t retval = 0;
    loff_t bytes_left = count;
    char *buf;

    buf = vmalloc((count < maxsz) ? count : maxsz);
    retval = -ENOMEM;
    if (!buf) goto out_nobuf;

    retval = -EIO;
    oldfs = get_fs();
    set_fs(KERNEL_DS);

    while (bytes_left) {
        ssize_t buffered;
	char *p = buf;

	/* Read as much as we can in a single call */
	buffered = vfs_read(src_filp, p, (bytes_left < maxsz) ? bytes_left : maxsz, src_ppos);
        if (!buffered) goto out_eof;
        if (buffered < 0) {
	    CR_ERR_EB(eb, "vfs_read returned %ld", (long int)buffered);
	    retval = buffered;
	    goto out_err;
        } 

	/* Write as much as we read */
        while (buffered) {
            ssize_t w = vfs_write(dst_filp, p, buffered, &dst_filp->f_pos);
            if (!w) goto out_eof;
            if (w < 0) {
	        CR_ERR_EB(eb, "vfs_write returned %ld", (long int)w);
	        retval = w;
	        goto out_err;
            } 
            bytes_left -= w;
            buffered -= w;
            p += w;
	}
    }
out_eof:
    retval = count - bytes_left;
out_err:
    set_fs(oldfs);
    vfree(buf);
out_nobuf:
    return retval;
}

/*
 * sendfile for HUGETLBFS, via mmap
 */
#ifdef HPAGE_SIZE
/*
 * Like cr_sendfile_buffered(), but for HUGETLBFS source file.
 * Uses temporary mmap()s of a chunk of len HPAGE_SIZE at a time.
 *
 * Note: Caller is responsible for checking count==0 or {dst,src}_ppos==NULL.
 */
static loff_t
cr_sendfile_hugesrc(cr_errbuf_t *eb, struct file *dst_filp, struct file *src_filp, loff_t *src_ppos, loff_t count)
{
    loff_t bytes_left = count;
    loff_t retval;
    struct mm_struct *mm = current->mm;
    unsigned long map_addr = 0;
    unsigned long map_pgoff = 0;
    unsigned long map_flags = MAP_SHARED;

    CRI_ASSERT((count & (HPAGE_SIZE-1)) == 0);
    CRI_ASSERT(*src_ppos == 0);

    for (bytes_left = count; bytes_left; bytes_left -= HPAGE_SIZE) {
	unsigned long tmp;
	down_write(&mm->mmap_sem);
	tmp = do_mmap_pgoff(src_filp, map_addr, HPAGE_SIZE, PROT_READ, map_flags, map_pgoff);
	up_write(&mm->mmap_sem);
	if (IS_ERR((void*)tmp)) {
	    CR_ERR_EB(eb, "do_mmap(HUGE src file) returned %ld", (long)tmp);
	    retval = tmp;
	    goto out_unmap;
	}
	map_addr = tmp;
	map_pgoff += (HPAGE_SIZE >> PAGE_SHIFT);
	map_flags |= MAP_FIXED;
	retval = cr_uwrite(eb, dst_filp, (void *)map_addr, HPAGE_SIZE);
        if (retval < 0) goto out_err;
    }
    retval = count;
    *src_ppos = count;
out_unmap:
    if (map_addr) {
	(void)sys_munmap(map_addr, HPAGE_SIZE); // XXX: check for error (unless on error path already)?
    }
out_err:
    return retval;
}

/*
 * Like cr_sendfile_buffered(), but for HUGETLBFS destination file.
 * Uses temporary mmap()s of a chunk of len HPAGE_SIZE at a time.
 *
 * Note: Caller is responsible for checking count==0 or {dst,src}_ppos==NULL.
 */
static loff_t
cr_sendfile_hugedst(cr_errbuf_t *eb, struct file *dst_filp, struct file *src_filp, loff_t *src_ppos, loff_t count)
{
    loff_t bytes_left = count;
    loff_t retval;
    struct mm_struct *mm = current->mm;
    unsigned long map_addr = 0;
    unsigned long map_pgoff = 0;
    unsigned long map_flags = MAP_SHARED;

    CRI_ASSERT((count & (HPAGE_SIZE-1)) == 0);
    CRI_ASSERT(dst_filp->f_pos == 0);
    CRI_ASSERT(src_ppos = &src_filp->f_pos);

    for (bytes_left = count; bytes_left; bytes_left -= HPAGE_SIZE) {
	unsigned long tmp;
	down_write(&mm->mmap_sem);
	tmp = do_mmap_pgoff(dst_filp, map_addr, HPAGE_SIZE, PROT_READ|PROT_WRITE, map_flags, map_pgoff);
	up_write(&mm->mmap_sem);
	if (IS_ERR((void*)tmp)) {
	    CR_ERR_EB(eb, "do_mmap(HUGE dst file) returned %ld", (long)tmp);
	    retval = tmp;
	    goto out_err;
	}
	map_addr = tmp;
	map_pgoff += (HPAGE_SIZE >> PAGE_SHIFT);
	map_flags |= MAP_FIXED;
	retval = cr_uread(eb, src_filp, (void *)map_addr, HPAGE_SIZE);
        if (retval < 0) goto out_unmap;
    }
    retval = count;
    dst_filp->f_pos = count;
out_unmap:
    if (map_addr) {
	(void)sys_munmap(map_addr, HPAGE_SIZE); // XXX: check for error (unless on error path already)?
    }
out_err:
    return retval;
}
#endif

/*
 * sendfile via splice_direct_to_actor
 */
#if HAVE_SPLICE_DIRECT_TO_ACTOR
#include <linux/splice.h>

static int
cr_splice_actor(struct pipe_inode_info *pipe, struct splice_desc *sd)
{
    struct file *file = sd->u.file;
    return file->f_op->splice_write(pipe, file, &file->f_pos, sd->total_len, sd->flags);
}

static loff_t
cr_splice_direct(struct file *src_filp, loff_t *src_ppos, struct file *dst_filp, loff_t count)
{
    struct splice_desc sd = {
	.len            = count,
	.total_len      = count,
	.flags          = 0,
	.pos            = *src_ppos,
	.u.file         = dst_filp,
    };

    long ret = splice_direct_to_actor(src_filp, &sd, cr_splice_actor);
    if (ret > 0) {
        *src_ppos += ret;
    }

    return ret;
}

#endif // HAVE_SPLICE_DIRECT_TO_ACTOR

/*
 * sendfile via do_generic_file_read()
 */
#if HAVE_4_ARG_DO_GENERIC_FILE_READ
  #define cr_generic_file_read(_f,_p,_d,_a) do_generic_file_read((_f),(_p),(_d),(_a))
#elif HAVE_5_ARG_DO_GENERIC_FILE_READ
  /* RH9 and RHEL add a non-block flag */
  #define cr_generic_file_read(_f,_p,_d,_a) do_generic_file_read((_f),(_p),(_d),(_a),0)
#endif
#ifdef cr_generic_file_read

#if HAVE_READ_DESCRIPTOR_T_BUF
  #define CR_ACTOR_DATA(_desc) ((_desc)->buf)
#elif HAVE_READ_DESCRIPTOR_T_ARG_DATA
  #define CR_ACTOR_DATA(_desc) ((_desc)->arg.data)
#else
  #error
#endif

static int
cr_sendfile_actor(read_descriptor_t *desc, struct page *page, unsigned long offset, unsigned long size)
{ 
    struct file *dst_filp = (struct file *)CR_ACTOR_DATA(desc);
    unsigned long bytes_left;
    char *addr, *p;
    mm_segment_t oldfs;
    ssize_t w = 0;

    if (size > desc->count) {
	size = desc->count;
    }
    bytes_left = size;

    oldfs = get_fs();
    set_fs(KERNEL_DS);

    addr = kmap(page);
    p = addr + offset;
    while (bytes_left) {
	w = vfs_write(dst_filp, p, bytes_left, &dst_filp->f_pos);
        if (w <= 0) break;
	p += w;
	bytes_left -= w;
    }
    kunmap(page);

    set_fs(oldfs);

    if (w < 0) {
	desc->error = w;
	w = 0;
    } else {
	w = size - bytes_left;
	desc->count -= w;
	desc->written += w;
    }

    return (int)w;
}

static loff_t
cr_sendfile_generic_read(struct file *dst_filp, struct file *src_filp, loff_t *src_ppos, loff_t count)
{
    read_descriptor_t desc;
    loff_t retval = 0;

    CR_ACTOR_DATA(&desc) = (char *)dst_filp; /* Yes, (char*) cast is needed */

#if BITS_PER_LONG == 32
    while (count) {
	/* Max 1GB chunks to avoid overflow (negative values) in desc.written */
	size_t chunk = ((count > 0x40000000) ? 0x40000000 : count);
        desc.count = chunk;
	desc.written = 0;
	desc.error = 0;
	cr_generic_file_read(src_filp, src_ppos, &desc, cr_sendfile_actor);
	if (!desc.written) return desc.error;
	if (desc.written != chunk) return -EIO;
	retval += chunk;
	count -= chunk;
    }
#elif BITS_PER_LONG == 64
    desc.count = count;
    desc.written = 0;
    desc.error = 0;
    cr_generic_file_read(src_filp, src_ppos, &desc, cr_sendfile_actor);
    retval = desc.written;
    if (!retval) {
	retval = desc.error;
    } else if (retval != count) {
	retval = -EIO;
    }
#else
      #error "Unknown BITS_PER_LONG"
#endif

    return retval;
}

#endif // cr_generic_file_read


/* Copy the given number of bytes from one file to another.
 * Uses provided src_ppos for src_file (or src_file->f_pos if NULL).
 * "zerocopy" reads are used when the source has a readpage method.
 */
loff_t
cr_sendfile(cr_errbuf_t *eb, struct file *dst_filp, struct file *src_filp, loff_t *src_ppos, loff_t count)
{
#ifdef cr_generic_file_read
    struct address_space *mapping;
#endif

    if (!count) return 0;
    if (!src_ppos) src_ppos = &src_filp->f_pos;

#ifdef HPAGE_SIZE
    /* The generic methods don't work on hugetlbfs files */
    if (is_file_hugepages(src_filp)) {
	return cr_sendfile_hugesrc(eb, dst_filp, src_filp, src_ppos, count);
    }
    if (is_file_hugepages(dst_filp)) {
	return cr_sendfile_hugedst(eb, dst_filp, src_filp, src_ppos, count);
    }
#endif

#if HAVE_SPLICE_DIRECT_TO_ACTOR
    /* Our 1st choice is splice_direct_to_actor(), when available and allowed.  */
    if (src_filp->f_op && src_filp->f_op->splice_read &&
        dst_filp->f_op && dst_filp->f_op->splice_write) {
	return cr_splice_direct(src_filp, src_ppos, dst_filp, count);
    }
#endif
#ifdef cr_generic_file_read
    /* Our 2nd choice algorithm is do_generic_file_read() + vfs_write()
     * But this is not available w/ kernel >= 2.6.25 (hence the ifdef)
     * nor is it available when the source is not mmap()able.
     */
    mapping = src_filp->f_dentry->d_inode->i_mapping;
    if (mapping && mapping->a_ops && mapping->a_ops->readpage) {
	return cr_sendfile_generic_read(dst_filp, src_filp, src_ppos, count);
    }
#endif

    /* Final option is vfs_read() + vfs_write() */
    return cr_sendfile_buffered(eb, dst_filp, src_filp, src_ppos, count);
}

/* Caller is responsible for path_get()/path_put() */
static char *
cr_getpath(struct path *path, char *buf, int size)
{
    char *name = NULL;

    if (path->dentry == NULL) {
        CR_WARN("path->dentry is NULL!");
	goto out;
    }
    if (path->mnt == NULL) {
        CR_WARN("path->vfsmnt is NULL!");
	goto out;
    }

#if HAVE_NAMEIDATA_DENTRY
    name = d_path(path->dentry, path->mnt, buf, size);
#elif HAVE_NAMEIDATA_PATH
    name = d_path(path, buf, size);
#else
    #error
#endif

out:
    return name;
}

const char *
cr_location2path(cr_location_t *loc, char *buf, int size)
{
	CR_PATH_DECL(path);
	char *name = NULL;

	CR_KTRACE_FUNC_ENTRY();

	if (loc->filp) {
		/* It's a file */
		CR_PATH_GET_FILE(path, loc->filp);
    		name = cr_getpath(path, buf, size);
		path_put(path);
	} else if (loc->fs) {
		/* It's a directory */
    		CR_PATH_GET_FS(path, loc->fs->pwd);
    		name = cr_getpath(path, buf, size - 2);
		path_put(path);

		if (name) {
			int len = strlen(buf);
			buf[len+0] = '/';
			buf[len+1] = '.';
			buf[len+2] = '\0';
		}
	} else {
		/* I don't know what it is */
		name = "";
	}

	if (!name) {
		name = ERR_PTR(-EBADF);
	}

	return name;
}

/* Saves pathname, returning bytes written (or <0 on error),
 * NULL dentry yields saved string value of NULL (distinct from empty string).
 * Uses supplied buf, if any, or will alloc/free otherwise.
 */
int cr_save_pathname(cr_errbuf_t *eb, struct file *cr_filp, struct path *path, char *orig_buf, int size)
{
    int retval;
    const char *name = NULL;
    char *buf = orig_buf;

    /* Short cut on NULL path or dentry */
    if (!path || !path->dentry) {
	goto write;
    }

    /* Allocate buf if none was supplied */
    if (!buf) {
        retval = -ENOMEM;
        buf = __getname();
        if (!buf) {
            goto out;
        }
	size = PATH_MAX;
    }

    /* find the file name */
    name = cr_getpath(path, buf, size);
	CR_INFO("file = %s\n");
    if (name == NULL) {
        CR_ERR_EB(eb, "Bad or non/existant name!");
        retval = -EBADF;
        goto out_bad;
    }

    /* now write out the name */
write:
    retval = cr_fputs(eb, name, cr_filp);
    if (retval < 0) {
        CR_ERR_EB(eb, "cr_save_pathname - Bad file write! (cr_fputs returned %d)", retval);
        goto out_bad;
    }

out_bad:
    if (buf && !orig_buf) {
      __putname(buf);
    }
out:
    return retval;
}

int cr_save_filename(cr_errbuf_t *eb, struct file *cr_filp, struct file *filp, char *buf, int size) {
  int retval;

  if (!filp) {
    retval = cr_save_pathname(eb, cr_filp, NULL, buf, size);
  } else {
    CR_PATH_DECL(path);
    CR_PATH_GET_FILE(path, filp);
    retval = cr_save_pathname(eb, cr_filp, path, buf, size);
    path_put(path);
  }

  return retval;
}


/* based on linux/fs/namei.c:lookup_create */
/* Differs in that the balancing up() takes place on error */
static struct dentry *cr_lookup_create(struct nameidata *nd, int is_dir)
{
    struct dentry *dentry;

    dentry = lookup_create(nd, is_dir);
    if (IS_ERR(dentry)) {
      cr_inode_unlock(nd->nd_dentry->d_inode);
    }
    return dentry;
}

/*
 * cr_anonymous_rename
 *
 * Rewrite a filename to an anonymous value
 *
 * len is strlen and size is buffer size
 */
static char *
cr_anonymous_rename(cr_errbuf_t *eb, const char *in_buf, unsigned long id)
{
    size_t len;
    char *out_buf, *p;

    len = strlen(in_buf);
    if (len >= PATH_MAX) {
	/* XXX: probably not what we want to do */
	len = PATH_MAX - 1;
    }

    /* strdup() */
    out_buf = __getname();
    if (out_buf == NULL) {
	goto out;
    }
    memcpy(out_buf, in_buf, len+1);

    /* dirname() */
    p = out_buf + len;
    while ((p != out_buf) && (*p != '/')) {
	--p;
    }
    ++p;
    len = p - out_buf;

#if BITS_PER_LONG == 32
    if (len > PATH_MAX - 20) {
	CR_ERR_EB(eb, "cr_anonymous_rename - unlinked name too long for renaming");
	goto out_free;
    }
    sprintf(p, ".blcr_%04x.%08lx", (unsigned int)current->pid, id);
#elif BITS_PER_LONG == 64
    if (len > PATH_MAX - 24) {
	CR_ERR_EB(eb, "cr_anonymous_rename - unlinked name too long for renaming");
	goto out_free;
    }
    sprintf(p, ".blcr_%04x.%016lx", (unsigned int)current->pid, id);
#else
    #error "No value for BITS_PER_LONG"
#endif

    return out_buf;

out_free:
    __putname(out_buf);
out:
    return NULL;
}

/* Call permision() and dentry_open().
 * Caller should dget() and mntget() just as they would for dentry_open(). */
struct file *
cr_dentry_open(struct dentry *dentry, struct vfsmount *mnt, int flags)
{
    struct file *filp;
    int acc_mask = ("\000\004\002\006"[(flags)&O_ACCMODE]); /* ICK (from linux/fs/namei.c) */
    int err;

    err = cr_permission(dentry->d_inode, acc_mask, NULL);
    filp = ERR_PTR(err);
  #if HAVE_TASK_CRED
    if (!IS_ERR(filp)) filp = dentry_open(dentry, mnt, flags, cr_current_cred());
  #else
    if (!IS_ERR(filp)) filp = dentry_open(dentry, mnt, flags);
  #endif

    return filp;
}

/* cr_mknod - based on linux/fs/namei.c:sys_mknod
 *
 * Creates regular files or fifos (no devices) making them anonymous (unlinked)
 * if desired.
 * Returns a dentry for the resulting filesystem objects, and the corresponding
 * vfsmnt can be obtained in nd->mnt.  Together these two can be passed
 * to dentry_open() or cr_dentry_open(), even for an unlinked inode.
 * In the event of an error, no dput() or cr_path_release() is required,
 * otherwise they are.
 *
 * In the event that an object exists with the given name, it will be
 * check for the proper mode prior to return, yielding -EEXIST on conflict.
 */
struct dentry *
cr_mknod(cr_errbuf_t *eb, struct nameidata *nd, const char *name, int mode, unsigned long unlinked_id)
{
    struct dentry * dentry;
    int err;

    if (unlinked_id) {
	/* Generate a replacement name which we will use instead of the original one. */
	name = cr_anonymous_rename(eb, name, unlinked_id);
	if (!name) {
	    CR_ERR_EB(eb, "cr_mknod - failed to rename unlinked object");
	    err = -ENOMEM;
	    goto out;
	}
    }

    /* Prior to 2.6.26, lookup_create() would return an exisiting dentry.
     * Since 2.6.26, it returns -EEXIST if the dentry exists.  So, we first
     * check for an existing dentry.  For older kernels this is not required,
     * but is still correct.
     */
    err = path_lookup(name, LOOKUP_FOLLOW, nd);
    if (!err) {
	dentry = dget(nd->nd_dentry);
	err = -EEXIST; /* Forces mode validation below */
	goto have_it;
    }

    err = path_lookup(name, LOOKUP_PARENT, nd);
    if (err) {
	CR_KTRACE_UNEXPECTED("Couldn't path_lookup for mknod %s.  err=%d.", name, err);
	goto out_free;
    }

    dentry = cr_lookup_create(nd, 0);
    if (IS_ERR(dentry)) {
	err = PTR_ERR(dentry);
	CR_KTRACE_UNEXPECTED("Couldn't lookup_create for mknod %s.  err=%d.", name, err);
	goto out_release;
    }

    switch (mode & S_IFMT) {
    case S_IFREG:
	err = vfs_create(nd->nd_dentry->d_inode, dentry, mode, nd);
	break;
    case S_IFIFO:
	err = cr_vfs_mknod(nd->nd_dentry->d_inode, dentry, nd->nd_mnt, mode, 0 /* ignored */);
	break;
    default:
	CR_ERR_EB(eb, "Unknown/invalid type %d passed to cr_mknod %s.", (mode&S_IFMT), name);
	err = -EINVAL;
    }
    if (unlinked_id && !err) { /* Note that we don't unlink if we failed to create */
	dget(dentry);	/* ensure unlink doesn't destroy the dentry */
	/* Note possibility of silent failure here: */
	(void)cr_vfs_unlink(nd->nd_dentry->d_inode, dentry, nd->nd_mnt);
	dput(dentry);
    }
    cr_inode_unlock(nd->nd_dentry->d_inode);

have_it:
    if ((err == -EEXIST) && !((dentry->d_inode->i_mode ^ mode) & S_IFMT)) {
	/* We fall through and return the dentry */
    } else if (err) {
	CR_KTRACE_UNEXPECTED("Couldn't cr_mknod %s.  err=%d.", name, err);
	goto out_put;
    }

    if (unlinked_id) {
	__putname(name);
    }
    return dentry;

out_put:
    dput(dentry);
out_release:
    cr_path_release(nd);
out_free:
    if (unlinked_id) {
	__putname(name);
    }
out:
    return (struct dentry *)ERR_PTR(err);
}

/* Calls cr_mknod and then opens with the given flags, returning a (struct file *) */
struct file *
cr_filp_mknod(cr_errbuf_t *eb, const char *name, int mode, int flags, unsigned long unlinked_id) {
    struct nameidata nd;
    struct dentry * dentry;
    struct file *filp;

    /* mknod */
    dentry = cr_mknod(eb, &nd, name, mode, unlinked_id);
    if (IS_ERR(dentry)) {
	CR_KTRACE_UNEXPECTED("Failed to recreate %sfilesystem object %s, err=%d.",
			unlinked_id?"unlinked ":"", name, (int)PTR_ERR(dentry));
	filp = (struct file *)dentry;
	goto out;
    }

    /* now open it */
    filp = cr_dentry_open(dget(dentry), mntget(nd.nd_mnt), flags);
    if (IS_ERR(filp)) {
	CR_ERR_EB(eb, "Failed to reopen %sfilesystem object %s, err=%d.",
			unlinked_id?"unlinked ":"", name, (int)PTR_ERR(dentry));
        goto out_dput;
    }

    /* check that we actually got the expected type */
    if ((mode ^ filp->f_dentry->d_inode->i_mode) & S_IFMT) {
	CR_ERR_EB(eb, "Type conflict when recreating %sfilesystem object %s.",
			unlinked_id?"unlinked ":"", name);
	fput(filp);
	filp = ERR_PTR(-EEXIST);
	goto out_dput;
    }

out_dput:
    dput(dentry); 
    cr_path_release(&nd);
out:
    return filp;
}

/* Based on sys_fchmod() from linux 2.6.21 (mostly unchanged since 2.4.0). */
int cr_filp_chmod(struct file *filp, mode_t mode) {
    struct iattr newattrs;
    struct dentry *dentry = filp->f_dentry;
    struct inode *inode = dentry->d_inode;
    int retval;

    retval = -EROFS;
    if (IS_RDONLY(inode)) goto out;
    retval = -EPERM;
    if (IS_IMMUTABLE(inode) || IS_APPEND(inode)) goto out;

    cr_inode_lock(inode);
    newattrs.ia_mode = (mode == (mode_t)-1) ? inode->i_mode
					    : ((mode & S_IALLUGO)|(inode->i_mode & ~S_IALLUGO));
    newattrs.ia_valid = ATTR_MODE | ATTR_CTIME;
    retval = cr_notify_change(dentry, filp->f_vfsmnt, &newattrs);
    cr_inode_unlock(inode);

out:
    return retval;
}

/* Construct an unlinked file and read its data from the context file */
/* If the original file location doesn't work, try "/tmp/" */
struct file *cr_mkunlinked(cr_errbuf_t *eb, struct file *cr_filp, const char *name, int mode, int flags, loff_t size, unsigned long unlinked_id) {
    const char *tmpdir = "/tmp/"; /* XXX: Ick.  Note trailing '/' is required */
    struct file *filp;
    loff_t w;
    const int xmodes = (S_IRUSR | S_IWUSR);

#if CRI_DEBUG
    filp = ERR_PTR(-EINVAL);
    if (!unlinked_id) {
	CR_ERR_EB(eb, "Zero 'unlinked_id' passed to %s", __FUNCTION__);
	goto out;
    }
    if ((mode&S_IFMT) && ((mode&S_IFMT) != S_IFREG)) {
	CR_ERR_EB(eb, "Bad mode %d passed to %s", mode, __FUNCTION__);
	goto out;
    }
#endif

    /* make a new unlinked file, suitable for writting in the saved data */
    filp = cr_filp_mknod(eb, name, mode|xmodes|S_IFREG, O_LARGEFILE|O_RDWR, unlinked_id);
    if (IS_ERR(filp)) {
	filp = cr_filp_mknod(eb, tmpdir, mode|xmodes|S_IFREG, O_LARGEFILE|O_RDWR, unlinked_id);
    }
    if (IS_ERR(filp)) {
	CR_ERR_EB(eb, "Failed to recreate unlinked file %s, err=%d.", name, (int)PTR_ERR(filp));
	goto out;
    }

    /* and populate it with the saved data */
    /* XXX: If/when we split the create from the populate, then we'll need to either
     * do an ftruncate() here, or else defer the llseek() until post-populate.
     */
    w = cr_sendfile(eb, filp, cr_filp, NULL, size);
    if (w != size) {
	filp = ERR_PTR(w);
        if (w >= 0) filp = ERR_PTR(-EIO);
        goto out;
    }

    /* Now reopen with caller-requested flags, if different */
    if (filp->f_flags != flags) {
	struct file *tmp = cr_filp_reopen(filp, flags);
	(void)filp_close(filp, current->files);
	filp = tmp; /* Even on error. */
    }
    if (IS_ERR(filp)) goto out;

    /* If we forced S_I[RW]USR, then fix it so fstat() doesn't reveal any change. */
    if ((mode & xmodes) != xmodes) {
	int err = cr_filp_chmod(filp, mode);
	if (err < 0) {
	    filp_close(filp, current->files);
	    filp = ERR_PTR(err);
	}
    }

out:
    return filp;
}

/* Create a new link to an existing (but potentially unlinked) dentry.
 * If the target link already exists we return the dentry, but if the target
 * exists and is not a link to the desired object, we return -EEXIST.
 *
 * NOTE: We once tried to do this via vfs_link(), rather than sys_link().
 * That was "better" in the sense that it was able to link to unlinked
 * targets (which this cannot).  However, that was not working over NFS
 * for reasons I never did figure out. -PHH
 */
struct dentry *
cr_link(cr_errbuf_t *eb, struct path *old_path, const char *name)
{
    struct nameidata nd;
    char *buf, *old_name;
    struct dentry *new_dentry = NULL;
    int retval;
    mm_segment_t oldfs;
    
    /* Lookup the path to the "old" file.
     * This is the part that prevents us from linking to an unlinked target.
     */
    retval = -ENOMEM;
    buf = __getname();
    if (!buf) goto out;
    old_name = cr_getpath(old_path, buf, PATH_MAX);
    if (IS_ERR(old_name)) {
	retval = PTR_ERR(old_name);
	goto out_free;
    }

    /* Now sys_link() */
    oldfs = get_fs();
    set_fs(KERNEL_DS);
    retval = sys_link(old_name, name);
    set_fs(oldfs);
    if (retval == -EEXIST) {
	/* Keep going, it may be the one we want */
    } else if (retval < 0) {
	CR_ERR_EB(eb, "cr_link: sys_link(%s,%s) returned %d", old_name, name, retval);
	goto out_free;
    }

    /* Now get the dentry for the newly-created object.
     * YES, there is a potential race, but we check below that we have the right object.
     */
    retval = path_lookup(name, LOOKUP_FOLLOW, &nd);
    if (retval < 0) {
	CR_ERR_EB(eb, "cr_link: path_lookup(%s) returned %d", name, retval);
	goto out_free;
    }
    new_dentry = dget(nd.nd_dentry);
    cr_path_release(&nd);

    /* Check that we have a link to the desired object.
     * Needed for sys_link() == -EEXIST and for the link-to-lookup race.
     */
    if (new_dentry->d_inode != old_path->dentry->d_inode) {
	dput(new_dentry);
	retval = -EEXIST;
	goto out_free;
    }

out_free:
    __putname(buf);
out:
    return (retval < 0) ? ERR_PTR(retval) : new_dentry;
}

/* Reopen the same filesystem object as an exsiting filp.
 * Result is a distinct filp, with potentially different f_flags and f_mode.
 */
struct file *
cr_filp_reopen(const struct file *orig_filp, int new_flags)
{
    return orig_filp ? cr_dentry_open(dget(orig_filp->f_dentry),
				      mntget(orig_filp->f_vfsmnt),
				      new_flags)
		     : ERR_PTR(-EBADF);
}

/*
 * cr_fd_claim
 *
 * Atomically checks and claims an fd
 *
 * returns < 0 on conflict w/ an existing fd
 */
int
cr_fd_claim(int fd)
{
    cr_fdtable_t *fdt;
    int retval;

    /* Mark the fd in use */
    spin_lock(&current->files->file_lock);
    fdt = cr_fdtable(current->files);
    if (FD_ISSET(fd, fdt->open_fds)) {
	retval = -EBUSY;
    } else {
	retval = 0;
	FD_SET(fd, fdt->open_fds);
	FD_CLR(fd, fdt->close_on_exec);
    }
    spin_unlock(&current->files->file_lock);

    return retval;
}

/* Install a given filp in a given files_struct, with CLOEXEC set.
 * Safe for files != current->files.
 * Mostly cut-and-paste from linux-2.6.0/fs/fcntl.c:locate_fd()
 */
int cr_dup_other(struct files_struct *files, struct file *filp)
{
    unsigned int newfd;
    unsigned int start;
    unsigned int max_fds;
    int error;
    cr_fdtable_t *fdt;

    spin_lock(&files->file_lock);

repeat: 
    fdt = cr_fdtable(files);
    start = CR_NEXT_FD(files, fdt);
    newfd = start;
    max_fds = CR_MAX_FDS(fdt);
    if (start < max_fds) {
	newfd = find_next_zero_bit(fdt->open_fds->fds_bits,
				   max_fds, start);
    }

    /* XXX: Really shouldn't be using current here.
     * However, I haven't bothered to figure out the locking
     * requirements for using anything else.
     * XXX: Probably could just pass the limit in.
     */
    error = -EMFILE;
    if (newfd >= CR_RLIM(current)[RLIMIT_NOFILE].rlim_cur) {
	goto out;
    }

    error = expand_files(files, newfd);
    if (error < 0) {
	goto out;
    } else if (error) {
	/* grew - search again (also reacquires fdt) */
	goto repeat;
    }

    CR_NEXT_FD(files, fdt) = newfd + 1;

    /* Claim */
    FD_SET(newfd, fdt->open_fds);
    FD_SET(newfd, fdt->close_on_exec);

    /* Install */
    get_file(filp);
    rcu_assign_pointer(fdt->fd[newfd], filp);

    error = newfd;
    
out:
    spin_unlock(&files->file_lock);
    return error;
}

/* Wrapper for vfs_getattr() to ensure we have the most
 * up-to-date inode info if working on a network filesystem.
 */
int cr_fstat(cr_objectmap_t map, struct file *filp)
{
    struct dentry *dentry = filp->f_dentry;
    struct inode *inode = dentry->d_inode;
    char *map_key = 1 + (char *)dentry;
    int retval = 0;

    if (!inode->i_op->getattr) {
	/* Not a special fs, so trivially nothing to do */
    } else if (map && cr_find_object(map, map_key, NULL)) {
	/* Attrs up-to-date, so nothing to do */
    } else {
	struct kstat stat;
	retval = vfs_getattr(filp->f_vfsmnt, dentry, &stat);
	if (map && !retval) {
	    cr_insert_object(map, map_key, (void *)1UL, GFP_KERNEL);
	}
    }

    return retval;
}
