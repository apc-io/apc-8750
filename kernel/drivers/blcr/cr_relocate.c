/*
 *  Berkeley Lab Checkpoint/Restart (BLCR) for Linux is Copyright (c)
 *  2008, The Regents of the University of California, through Lawrence
 *  Berkeley National Laboratory (subject to receipt of any required
 *  approvals from the U.S. Dept. of Energy).  All rights reserved.
 *
 *  Portions may be copyrighted by others, as may be noted in specific
 *  copyright notices within specific files.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  $Id: cr_relocate.c,v 1.14 2008/12/12 02:10:12 phargrov Exp $
 *
 *  This file implements functions to handle file/directory relocation
 */

#include "cr_module.h"
#include <asm/uaccess.h>

struct cr_rstrt_relocate_s {
	unsigned int count;
	struct cr_rstrt_relocate_rec_s {
		const char *oldpath;
		const char *newpath;
		unsigned int old_len;
		unsigned int new_len;
	} path[0]; /* GNU (not ISO C99) variable length array */
};

// Bytes to allocate for struct cr_rstrt_relocate_x with _cnt path entries:
#define CR_RSTRT_RELOCATE_T_SIZE(_cnt) (sizeof(struct cr_rstrt_relocate_s) + \
                                        (_cnt) * sizeof(struct cr_rstrt_relocate_rec_s))

const char *
cr_relocate_path(cr_rstrt_relocate_t reloc, const char *path, int put_old)
{
    unsigned int i;
    struct cr_rstrt_relocate_rec_s *record;

    if (!reloc) {
	return path;
    }

    for (i = 0, record = &reloc->path[0]; i < reloc->count; i++, record++) {
	if (!strncmp(path, record->oldpath, record->old_len)) {
	    const char *suffix = path + record->old_len;
	    unsigned int suff_len;
	    char *reloc_path;

	    if ((*suffix != '\0') && (*suffix != '/')) {
		    /* Match not on a component boundary */
		    continue;
	    }

	    suff_len = strlen(suffix);
	    if ((record->new_len + suff_len) >= PATH_MAX) {
		return ERR_PTR(-ENAMETOOLONG);
	    }

	    reloc_path = __getname();
	    if (!reloc_path) {
		return ERR_PTR(-ENOMEM);
	    }

	    memcpy(reloc_path, record->newpath, record->new_len);
	    memcpy(reloc_path+record->new_len, suffix, suff_len+1);

	    CR_KTRACE_LOW_LVL("'%s' -> '%s'", path, reloc_path);

	    if (put_old) {
		__putname(path);
	    }

	    return reloc_path;
	}
    }

    return path;
}

/* Taken (almost) directly from linux-2.6.0/fs/namei.c:do_getname
 * Modified to return len (including nul byte)
 */
static int
my_getname(const char __user *filename, char *work_page)
{
    int result;
    unsigned long len = PATH_MAX;

    if ((unsigned long) filename >= TASK_SIZE) {
	if (!segment_eq(get_fs(), KERNEL_DS)) return -EFAULT;
    } else if (TASK_SIZE - (unsigned long) filename < PATH_MAX) {
	len = TASK_SIZE - (unsigned long) filename;
    }

    result = strncpy_from_user(work_page, filename, len);

    if (result >= len) {
	result = -ENAMETOOLONG;
    } else if (!result) {
	result = -EINVAL;
    } else {
	result += 1;
    }

    return result;
}

static
int do_read_reloc(cr_rstrt_req_t *req, unsigned int count, const struct cr_rstrt_relocate_pair *path)
{
    cr_rstrt_relocate_t reloc = NULL;
    unsigned int i;
    char *page = NULL;
    long result = 0;

    CR_NO_LOCKS();

    result = -ENOMEM;
    reloc = cr_kzalloc(CR_RSTRT_RELOCATE_T_SIZE(count), GFP_KERNEL);
    if (!reloc) {
	CR_ERR_REQ(req, "failed to allocate memory to store relocation records");
	goto out;
    }
    reloc->count = count;
    page = __getname();
    if (!page) {
	CR_ERR_REQ(req, "failed to allocate memory to parse relocation records");
	goto out_free;
    }

    /* Copy paths from user-space */
    for (i = 0; i < count; i++) {
	const char *oldpath = path[i].oldpath;
	const char *newpath = path[i].newpath;

	result = my_getname(oldpath, page);
	if (result < 0) {
	    goto out_put;
	}
	if (*page != '/') {
	    CR_ERR_REQ(req, "relocate.oldpath '%s' is not a full path", page);
	    result = -EINVAL;
	    goto out_put;
	}
	reloc->path[i].old_len = result - 1;
	reloc->path[i].oldpath = cr_kmemdup(page, result, GFP_KERNEL);
	if (!reloc->path[i].oldpath) {
	    CR_ERR_REQ(req, "failed to allocate memory to dup reloction record");
	    result = -ENOMEM;
	    goto out_put;
	}

	result = my_getname(newpath, page);
	if (result < 0) {
	    goto out_put;
	}
	if (*page != '/') {
	    CR_ERR_REQ(req, "relocate.newpath '%s' is not a full path", page);
	    result = -EINVAL;
	    goto out_put;
	}
	reloc->path[i].new_len = result - 1;
	reloc->path[i].newpath = cr_kmemdup(page, result, GFP_KERNEL);
	if (!reloc->path[i].newpath) {
	    CR_ERR_REQ(req, "failed to allocate memory to dup reloction record");
	    result = -ENOMEM;
	    goto out_put;
	}
    }

    __putname(page);
    req->relocate = reloc;
    return 0;

out_put:
    __putname(page);
out_free:
    cr_free_reloc(reloc);
out:
    return result;
}

int cr_read_reloc(cr_rstrt_req_t *req, void __user *void_arg) {
    struct cr_rstrt_relocate __user *arg = void_arg;
    struct cr_rstrt_relocate_pair *path = NULL;
    unsigned int count;
    size_t alloc_sz;
    int result = 0;

    CR_NO_LOCKS();

    if (get_user(count, &arg->count)) {
	result = -EFAULT;
	goto out;
    }

    if (!count) {
	/* result = 0 already */
	goto out;
    } else if (count > CR_MAX_RSTRT_RELOC) {
	CR_ERR_REQ(req, "Relocation count of %u exceedes max %d\n", count, CR_MAX_RSTRT_RELOC);
	result = -EINVAL;
	goto out;
    }

    result = -ENOMEM;
    alloc_sz = count * sizeof(struct cr_rstrt_relocate_pair);
    path = cr_kzalloc(alloc_sz, GFP_KERNEL);
    if (!path) {
	goto out;
    }

    /* Copy path pairs from user-space */
    result = -EFAULT;
    if (copy_from_user(path, &arg->path, alloc_sz)) {
	goto out_free;
    }

    result = do_read_reloc(req, count, path);

out_free:
    kfree(path);
out:
    return result;
}

void cr_free_reloc(cr_rstrt_relocate_t reloc) {
    if (reloc) {
	unsigned int i;
	for (i = 0; i < reloc->count; i++) {
	    kfree(reloc->path[i].oldpath);	/* kfree(NULL) OK */
	    kfree(reloc->path[i].newpath);	/* kfree(NULL) OK */
	}
	kfree(reloc);
    }
}

#ifdef CONFIG_COMPAT
int cr_read_reloc32(cr_rstrt_req_t *req, void __user *void_arg) {
    struct cr_compat_rstrt_relocate __user *arg = void_arg;
    struct cr_rstrt_relocate_pair *path = NULL;
    unsigned int count, i;
    size_t alloc_sz;
    int result = 0;

    CR_NO_LOCKS();

    if (get_user(count, &arg->count)) {
	result = -EFAULT;
	goto out;
    }

    if (!count) {
	/* result = 0 already */
	goto out;
    } else if (count > CR_MAX_RSTRT_RELOC) {
	CR_ERR_REQ(req, "Relocation count of %u exceedes max %d\n", count, CR_MAX_RSTRT_RELOC);
	result = -EINVAL;
	goto out;
    }

    result = -ENOMEM;
    alloc_sz = count * sizeof(struct cr_rstrt_relocate_pair);
    path = cr_kzalloc(alloc_sz, GFP_KERNEL);
    if (!path) {
	goto out;
    }

    /* Copy paths from user-space */
    result = -EFAULT;
    for (i = 0; i < count; i++) {
	compat_uptr_t oldptr, newptr;

	if (get_user(oldptr, &arg->path[i].oldpath) ||
	    get_user(newptr, &arg->path[i].newpath)) {
	    goto out_free;
	}
	path[i].oldpath = compat_ptr(oldptr);
	path[i].newpath = compat_ptr(newptr);
    }

    result = do_read_reloc(req, count, path);

out_free:
    kfree(path);
out:
    return result;
}
#endif // CONFIG_COMPAT

