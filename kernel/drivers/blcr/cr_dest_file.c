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
 * $Id: cr_dest_file.c,v 1.27.8.1 2009/06/12 20:37:03 phargrov Exp $
 */

#include "cr_module.h"

#define CR_FILE_PATT	"context.%d"
#define CR_FILE_MAX	32	// XXX: better size?
#define CR_FILE_MODE	0400


// XXX: need comment here
// Note: we fput if appropriate.
//
// XXX: Are these checks sufficient/correct?
//
// XXX:  These need to return good error codes!
static int validate_file(struct file *file, int is_write)
{
	int retval = -EINVAL;
	struct dentry *dentry;
	struct inode *inode;

	if (file == NULL) {
	        retval = -EBADF;
		// Fall through to failure
	} else if (IS_ERR(file)) {
		retval = PTR_ERR(file);
		// Fall through to failure
	} else {
		dentry = file->f_dentry;
		inode = dentry->d_inode;
	
		if (inode->i_nlink > 1) {
		        retval = -EMLINK;
			// Don't dump to a file w/ multiple links
			// Fall through to failure
		} else if (d_unhashed(dentry)) {
			// Fail due to NFS or autofs timeouts.
			// Fall through to failure
		} else if (!S_ISREG(inode->i_mode) &&
			   !S_ISCHR(inode->i_mode) &&
			   !S_ISSOCK(inode->i_mode) &&
			   !S_ISFIFO(inode->i_mode)) {
			// Fall through to failure
		} else if(!(file->f_mode & (is_write ? FMODE_WRITE : FMODE_READ))) {
		        retval = -EBADF;
			// Fail because there is no write/read permission
			// Fall through to failure
		} else {
			// SUCCESS:
			return 0;
		}
	
		// Only failure exits here
		fput(file);
	}

	// Only failure exits here
	return retval;
}

// XXX: need comment here
static int do_init_reg(cr_location_t *loc, struct file *filp)
{
	int result;

	result = validate_file(filp, loc->is_write);
	if (!result) {
	    loc->filp = filp;
	    init_MUTEX(&loc->mutex);
	}

	return result;
}

// XXX: need comment here
// XXX: currently only check perms to create here.
// XXX: other checks needed?
static int do_init_dir(cr_location_t *loc, struct file *dirp)
{
	int result = 0;

	result = cr_permission(dirp->f_dentry->d_inode,
			       (loc->is_write ? MAY_WRITE : MAY_READ) | MAY_EXEC, NULL);
	if (!result) {
		loc->fs = copy_fs_struct(current->fs);
		if (loc->fs) {
			// replace the pwd with that of 'dirp'
			cr_set_pwd_file(loc->fs, dirp);
		} else {
			result = -EINVAL;
		}
	}

	// Error and normal paths exit here
	fput(dirp);	// We don't hold the filp for a directory
	return result;
}

// cr_loc_init(loc, fd, from)
//
// Validate and record data about the requested destination of a checkpoint
//
// Returns 0 on success, negative error code on failure.
//
// XXX: need to document return cases?
// XXX: need to be sure we make all the right checks.
int cr_loc_init(cr_errbuf_t *eb, cr_location_t *loc, int fd, struct file *from, int is_write)
{
	struct file *filp;

	memset(loc, 0, sizeof(*loc));
	loc->is_write = is_write;

	if (fd != CR_DEST_CWD) {
		filp = fget(fd);
	} else {
		// XXX: do we want to shortcut and just copy current->fs?
		filp = filp_open(".", O_RDONLY|O_NDELAY|O_DIRECTORY, 0);
	}

	if (!filp) {
	        CR_ERR_EB(eb, "invalid file descriptor %d received", fd);
		return -EINVAL;
	} else if (IS_ERR(filp)) {
		return PTR_ERR(filp);
	} else if (filp == from) {
	        CR_ERR_EB(eb, "file descriptor %d is the ctrl descriptor", fd);
		fput(filp);
		return -EINVAL;
	}

	switch (filp->f_dentry->d_inode->i_mode & S_IFMT) {
	case S_IFREG: case S_IFCHR: case S_IFIFO: case S_IFSOCK:
	        CR_KTRACE_LOW_LVL("Calling do_init_reg on fd %d", fd);
		return do_init_reg(loc, filp);
		break;

	case S_IFDIR:
		return do_init_dir(loc, filp);
		break;

	case S_IFBLK:
		// We don't deal with this case yet:
		// fall through...

	default:
	        CR_ERR_EB(eb, "unsupported file type");
		fput(filp);
		return -EINVAL;
	}
}

// cr_loc_free(loc)
//
// Free the checkpoint destination, decrementing use counts as appropriate.
void cr_loc_free(cr_location_t *loc)
{
	// For file/socket, etc.:
	if (loc->filp) {
		fput(loc->filp);
	}

	// For directory:
	if (loc->fs) {
		cr_free_fs_struct(loc->fs);
	}
}

// cr_loc_get(loc, shared)
//
// Returns the filp to write to.
// Sets 'shared' to indicate if the file is shared with other threads.
// If so, then the caller is responsible for any synchronization.
// The loc->mutex field is provided for this purpose.
//
struct file *cr_loc_get(cr_location_t *loc, int *shared)
{
	struct file *filp = NULL;

	if (loc->filp) {
		// Destination is a "regular" file.

		// Just return the filp.  We don't need to fget()
		// because the fget() done in cr_loc_init is sufficient.
		filp = loc->filp;

		if (shared) {
			*shared = 1;
		}
	} else if (loc->fs) {
		// Destination is a per-process file in a given directory.

		struct fs_struct *saved_fs;
		char filename[CR_FILE_MAX];
		int error;

		// Create a filename
		sprintf(filename, CR_FILE_PATT, current->pid);

		// Play with current->fs to open() in the destination dir
		saved_fs = current->fs;
		current->fs = loc->fs;
		filp = filp_open(filename,
				 O_NOFOLLOW | (loc->is_write ? (O_WRONLY | O_CREAT | O_TRUNC) : O_RDONLY),
				 CR_FILE_MODE);
		current->fs = saved_fs;

		error = validate_file(filp, loc->is_write);
		if (error) {
			filp = ERR_PTR(error);
		}

		if (shared) {
			*shared = 0;
		}
	}

	return filp;
}

// cr_loc_put()
//
// Put the filp written to.
// This is always paired with cr_loc_get() calls.
//
void cr_loc_put(cr_location_t *loc, struct file *filp)
{
	if (loc->filp) {
		// Dest is a "regular" file.  DO NOTHING
	} else if (loc->fs && filp) {
		// Dest is a directory.  Release the temporary filp.
		fput(filp);
	}
}


