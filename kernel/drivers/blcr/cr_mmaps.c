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
 * $Id: cr_mmaps.c,v 1.36.8.1 2009/03/26 04:22:43 phargrov Exp $
 *
 * This file handles special memory mappings
 *
 */

#include "cr_module.h"
#include <linux/mman.h>

// Descriptor for a special mapping
// Used both in-memory and on-disk
struct cr_mmaps_desc {
  /* What is mapped: */
    void *		mmaps_id; /* The (struct inode *) at checkpoint time */
    loff_t		i_size;
    int			type;
  /* Where and how is it mapped: */
    unsigned long	start, end;
    unsigned long	flags;
    unsigned long	pgoff;  /* units of PAGE_SIZE */
};

// Vales for desc->type
enum {
  CR_SHANON_FILE,
  CR_SHANON_SHMEM,
  CR_SHANON_HUGETLB,
  CR_PSE,
};

/* Read the mapping metadata from the context file
 *
 * Returns 0 on success, or <0 on error
 */
long
cr_load_mmaps_maps(cr_rstrt_proc_req_t *proc_req)
{
    cr_errbuf_t *eb = proc_req->req->errbuf;
    struct cr_mmaps_desc *desc_tbl = NULL;
    size_t size = 0;
    int count = 0;
    int retval;

    /* Read size (in bytes) of table */
    retval = cr_kread(eb, proc_req->file, &size, sizeof(size));
    if (retval != sizeof(size)) {
        CR_ERR_PROC_REQ(proc_req, "mmaps_maps: read of size failed");
        goto err;
    }
    if (!size) goto out_notbl;
    count = size / sizeof(struct cr_mmaps_desc);
#if CRI_DEBUG
    if (size != count * sizeof(struct cr_mmaps_desc)) {
        CR_ERR_PROC_REQ(proc_req, "read invalid mmaps tbl size: %ld is not a multiple of %ld bytes", (long)size, (long)sizeof(struct cr_mmaps_desc));
        retval = -EINVAL;
        goto err;
    }
#endif

    /* Allocate space */
    desc_tbl = vmalloc(size);
    if (!desc_tbl) {
	CR_ERR_PROC_REQ(proc_req, "Failed to allocate %ld bytes for mmaps table", (long)size);
        retval = -ENOMEM;
        goto err;
    }

    /* Read the table */
    retval = cr_kread(eb, proc_req->file, desc_tbl, size);
    if (retval != size) {
        CR_ERR_PROC_REQ(proc_req, "mmaps_maps: read of table failed");
        goto err;
    }

    /* Install */
out_notbl:
    proc_req->mmaps_cnt = count;
    proc_req->mmaps_tbl = desc_tbl;

err:
    return retval;
}

static struct file *
cr_regenerate(cr_rstrt_proc_req_t *proc_req,
	      const struct cr_mmaps_desc *desc, loff_t size) {
    cr_rstrt_relocate_t reloc = proc_req->req->relocate;
    cr_errbuf_t *eb = proc_req->req->errbuf;
    struct file *cr_filp = proc_req->file;
    int mode, flags;
    struct file *filp;
    const char *name;

    mode = 0;
    if (desc->flags & VM_MAYREAD)  mode |= S_IRUSR;
    if (desc->flags & VM_MAYWRITE) mode |= S_IWUSR;
    if (desc->flags & VM_MAYEXEC)  mode |= S_IXUSR;

    switch (desc->flags & (VM_MAYREAD|VM_MAYWRITE)) {
    case VM_MAYREAD:	 flags = O_RDONLY; break;
    case VM_MAYWRITE:	 flags = O_WRONLY; break;
    default:		 flags = O_RDWR; break;
    }

    /* read original filename from context file */
    name = cr_getname(eb, reloc, cr_filp, 0);
	CR_INFO("file = %s\n",name);
    if (IS_ERR(name)) {
	filp = (struct file *)name;
	goto out_nopage;
    }

    filp = cr_mkunlinked(eb, cr_filp, name, mode, flags, size, (unsigned long)desc->mmaps_id);
    if (IS_ERR(filp)) {
	CR_ERR_PROC_REQ(proc_req, "cr_mkunlinked returned error %d", (int)PTR_ERR(filp));
    }

    __putname(name);
out_nopage:
    return filp;
}

/* Read the data from the context file
 *
 * Returns 0 on success, or <0 on error
 */
long
cr_load_mmaps_data(cr_rstrt_proc_req_t *proc_req)
{
    cr_errbuf_t *eb = proc_req->req->errbuf;
    const int count = proc_req->mmaps_cnt;
    const struct cr_mmaps_desc *desc = proc_req->mmaps_tbl;
    struct mm_struct *mm = current->mm;
    struct vmadump_vma_header mapheader;
    int retval, i;
    loff_t r;

    retval = 0;
    for (i=0; i<count; ++i, ++desc) {
        struct file *filp = NULL;
	unsigned long flags;
	unsigned long len = desc->end - desc->start;
	unsigned long map_addr;
	int found, prot = 0;
	    
	prot = 0;
	flags = MAP_FIXED | ((desc->flags & VM_MAYSHARE) ? MAP_SHARED : MAP_PRIVATE);
        if (desc->flags & VM_READ)  prot |= PROT_READ;
        if (desc->flags & VM_WRITE) prot |= PROT_WRITE;
        if (desc->flags & VM_EXEC)  prot |= PROT_EXEC;
        if (desc->flags & VM_GROWSDOWN) flags |= MAP_GROWSDOWN;
        if (desc->flags & VM_EXECUTABLE) flags |= MAP_EXECUTABLE;

	/* "mmaps_id" was checkpoint-time inode, but we map to the first-restored filp */
        found = cr_find_object(proc_req->req->map, desc->mmaps_id, (void **)&filp);
   
	if (found) {
	    /* Nothing to prepare */
	} else if (desc->type != CR_SHANON_SHMEM) {
	    /* create before mmap() */
	    filp = cr_regenerate(proc_req, desc, desc->i_size);
	    if (IS_ERR(filp)) {
		retval = PTR_ERR(filp);
		CR_ERR_PROC_REQ(proc_req, "cr_regenerate returned %d", retval);
		goto err;
	    }
	} else {
	    const int mmap_prot = PROT_READ|PROT_WRITE|PROT_EXEC;
            struct vm_area_struct *map;

	    /* mmap() at offset 0, even if it needs to move later */
	    down_write(&mm->mmap_sem);
	    map_addr = do_mmap(NULL, desc->start, len, mmap_prot, flags, 0);
            map = find_vma(mm, desc->start);
	    filp = (map && (map->vm_start == desc->start)) ? map->vm_file : NULL;
	    up_write(&mm->mmap_sem);
	    if (!filp) {
		goto fail;
	    }

	    /* Populate *after* mmap() */
	    r = cr_sendfile(eb, filp, proc_req->file, NULL, desc->i_size);
	    if (r != desc->i_size) {
		CR_ERR_PROC_REQ(proc_req, "read returned %d on copy-in of mmap()ed data", (int)r);
		retval = (r < 0) ? r : -EIO;
		goto err;
	    }

	    /* Fall through to 2nd mmap to ensure correct offset and protection */
	    get_file(filp); /* So (re)mmap doesn't drop it */
	}

        down_write(&mm->mmap_sem);
	map_addr = do_mmap(filp, desc->start, len, prot|PROT_WRITE, flags, (desc->pgoff << PAGE_SHIFT));
	up_write(&mm->mmap_sem);

        if (!found) {
            (void)cr_insert_object(proc_req->req->map, desc->mmaps_id, (void *)filp, GFP_KERNEL);
	    fput(filp);
	}

	if (!filp || (map_addr != desc->start)) {
fail:
	    CR_ERR_PROC_REQ(proc_req, "Failed to locate newborn mmap()ed space");
	    retval = -EINVAL;
	    goto err;
	}
    }

    /* Load dirty pages of maps if any */
    r = cr_kread(eb, proc_req->file, &mapheader, sizeof(mapheader));
    while (r == sizeof(mapheader) && (mapheader.start != ~0 || mapheader.end != ~0)) {
	unsigned long dirty_pages_prot=0;
    	if (mapheader.flags & VM_READ)  dirty_pages_prot |= PROT_READ;
    	if (mapheader.flags & VM_WRITE) dirty_pages_prot |= PROT_WRITE;
    	if (mapheader.flags & VM_EXEC)  dirty_pages_prot |= PROT_EXEC;
	r = vmadump_load_page_list(proc_req, proc_req->file, (dirty_pages_prot & PROT_EXEC));
    	if (r) {
    		CR_ERR_PROC_REQ(proc_req, "Error in vmadump_load_page_list");
    		goto err;
    	}
    	if (sys_mprotect(mapheader.start,mapheader.end - mapheader.start, dirty_pages_prot)) {
		CR_WARN_PROC_REQ(proc_req, "mprotect failed. (ignoring)");
	}
	r = cr_kread(eb, proc_req->file, &mapheader, sizeof(mapheader));
    }
    if (r != sizeof(mapheader)) {
	CR_ERR_PROC_REQ(proc_req, "mmaps_data: read header returned %d", (int)r);
    }
err:
    return retval;
}

/* Save the mappings metadata for the current mm.
 * They are both written to the context file, and saved in the proc_req
 * for later use.
 *
 * Returns bytes written (>=0), or <0 on error
 */
long
cr_save_mmaps_maps(cr_chkpt_proc_req_t *proc_req)
{
    cr_errbuf_t *eb = proc_req->req->errbuf;
    int flags = proc_req->req->flags;
    struct mm_struct *mm = current->mm;
    struct vm_area_struct *map;
    struct cr_mmaps_desc *desc_tbl = NULL;
    struct file **file_tbl = NULL;
    size_t size = 0;
    int i, count = 0;
    int retval;

    down_read(&mm->mmap_sem);
    /* First pass just counts maps ... */
    for (map = mm->mmap; map ; map = map->vm_next) {
	if (!map->vm_file) continue;
	/* Ensure up-to-date inode information (e.g. on a network fs) */
	retval = cr_fstat(proc_req->req->map, map->vm_file);
	if (retval) {
	    CR_ERR_PROC_REQ(proc_req, "mmaps_maps: fstat of mapped file failed");
	    goto err;
	}
	count += !!vmad_is_special_mmap(map, flags);
    }
    /* ... then we allocate the table ... */
	count = 0;//jay
    if (!count) goto out_notbl;
    size = count * sizeof(struct cr_mmaps_desc);
    desc_tbl = vmalloc(size);
    file_tbl = vmalloc(count*sizeof(*file_tbl));
    retval = -ENOMEM;
    if (!desc_tbl || !file_tbl) {
        up_read(&mm->mmap_sem);
	vfree(desc_tbl);
	goto err;
    }
    /* ... and the second pass fills the table. */
    for (i = 0, map = mm->mmap; (i < count) && map ; map = map->vm_next) {
	if (map->vm_file && vmad_is_special_mmap(map, flags)) {
	    struct file *filp = map->vm_file;

	    desc_tbl[i].mmaps_id  = filp->f_dentry->d_inode;
	    desc_tbl[i].i_size    = i_size_read(filp->f_dentry->d_inode);
	    desc_tbl[i].start     = map->vm_start;
	    desc_tbl[i].end       = map->vm_end;
	    desc_tbl[i].flags     = map->vm_flags;
	    desc_tbl[i].pgoff     = map->vm_pgoff;
	    file_tbl[i] = filp;

	    if (is_vm_hugetlb_page(map)) {
		desc_tbl[i].type = CR_SHANON_HUGETLB;
		if (!(map->vm_flags & VM_MAYSHARE)) {
		    desc_tbl[i].i_size = 0;
		}
	    } else if (!vmad_dentry_unlinked(filp->f_dentry)) {
	    	desc_tbl[i].type = CR_PSE;
#if defined(CONFIG_SHMEM) || defined(CR_KDATA_shmem_file_operations)
	    } else if (filp->f_op == &shmem_file_operations) {
		desc_tbl[i].type = CR_SHANON_SHMEM;//jay
#endif
#if defined(CONFIG_TINY_SHMEM)
	    } else if (filp->f_op == &ramfs_file_operations) {
		desc_tbl[i].type = CR_SHANON_SHMEM;
#endif
	    } else {
		desc_tbl[i].type = CR_SHANON_FILE;
	    }
	    ++i;
	}
    }
out_notbl:
    proc_req->mmaps_cnt = count;
    proc_req->mmaps_tbl = desc_tbl;
    up_read(&mm->mmap_sem);

    /* write the size in bytes */
    retval = cr_kwrite(eb, proc_req->file, &size, sizeof(size));
    if (retval != sizeof(size)) {
        CR_ERR_PROC_REQ(proc_req, "mmaps_maps: write of size failed");
        goto err;
    }

    /* write the table (if any) */
    if (size) {
	long w = cr_kwrite(eb, proc_req->file, desc_tbl, size);
        if (w != size) {
            CR_ERR_PROC_REQ(proc_req, "mmaps_maps: write of table failed");
            goto err;
        }
        retval += w;
    }

    /* We've saved the (struct inode *) as the "mmaps_id", because only it
     * is unique in the case of multiple open()s of the same file.
     * However, we need the (struct file *) later to help us save the data.
     * So, we replace the in-memory mmaps_id field w/ the filp.
     */
    for (i = 0; i < count; ++i) {
	desc_tbl[i].mmaps_id = file_tbl[i];
	CR_INFO("0x%x----0x%x type=0x%x flags=0x%x\n",desc_tbl[i].start,desc_tbl[i].end,desc_tbl[i].type,desc_tbl[i].flags);
    }

err:
    vfree(file_tbl);
    return retval;
}

/* Save the mapped data for the current mm.
 *
 * Returns bytes written (>=0), or <0 on error
 */
loff_t
cr_save_mmaps_data(cr_chkpt_proc_req_t *proc_req)
{
    cr_errbuf_t *eb = proc_req->req->errbuf;
    const int count = proc_req->mmaps_cnt;
    const struct cr_mmaps_desc *desc = proc_req->mmaps_tbl;
    loff_t w, retval;
    struct vmadump_vma_header head;
    int i;

    retval = 0;
    for (i = 0; i < count; ++i, ++desc) {
	struct file *filp = (struct file *)desc->mmaps_id;
	struct inode *inode = filp->f_dentry->d_inode;
	loff_t size = desc->i_size;
	loff_t src_pos = 0;

	/* NOTE: we currently rely on the restore order matching the save order */
        if (cr_insert_object(proc_req->req->map, inode, (void *)1UL, GFP_KERNEL)) {
	    /* Somebody dumped this one already */
	    continue;
	}

	if (desc->type != CR_SHANON_SHMEM) {
	    w = cr_save_filename(eb, proc_req->file, filp, NULL, 0);
	    if (w < 0) {
	        retval = w;
	        goto err;
	    }
	    retval += w;
	}

	w = cr_sendfile(eb, proc_req->file, filp, &src_pos, size);
	if (w != size) {
	    CR_ERR_PROC_REQ(proc_req, "write returned %d on copy-out of mmap()ed data", (int)w);
	    retval = (w < 0) ? w : -EIO;
	    goto err;
	}
        retval += w;
    }

    /* Save any dirty pages now */
    desc = proc_req->mmaps_tbl;
    for (i = 0; i < count; ++i, ++desc) {
	if ((desc->type != CR_PSE) && (desc->type != CR_SHANON_HUGETLB)) continue;
	if (desc->flags & VM_SHARED) continue; /* any dirty pages were saved w/ file */

        /* generate the header */
	head.start   = desc->start;
	head.end     = desc->end;
	head.flags   = desc->flags;
	head.namelen = 0;
	head.offset  = desc->pgoff << PAGE_SHIFT;
	w = cr_kwrite(eb, proc_req->file, &head, sizeof(head));
	if (w != sizeof(head)) goto bad_write;
	retval += w;

	/* dump the dirty pages */
	if (desc->type == CR_PSE) {
	    w = vmadump_store_dirty_page_list(proc_req, proc_req->file, desc->start, desc->end);
	} else {
	    w = vmadump_store_page_list(proc_req, proc_req->file, desc->start, desc->end);
	}
	if (w < 0) goto err;
	retval += w;
    }
    /* Terminate maps list */
    head.start = head.end = ~0L;
    w = cr_kwrite(eb, proc_req->file, &head, sizeof(head));
    if (w != sizeof(head)) goto bad_write;
    retval += w;

err:
    return retval;

bad_write:
   CR_ERR_PROC_REQ(proc_req, "mmaps_data: write header returned %d", (int)w);
   return w;
}
