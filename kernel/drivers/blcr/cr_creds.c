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
 * $Id: cr_creds.c,v 1.384.8.2 2009/02/07 02:42:54 phargrov Exp $
 */

#include "cr_module.h"

struct cr_context_creds {
    uid_t uid,euid,suid;
    gid_t gid,egid,sgid;
    unsigned int ngroups;
#if HAVE_TASK_GROUP_INFO || HAVE_TASK_CRED
    const struct group_info *group_info;
  #define CR_HAVE_GROUP_INFO 1
#endif
    /* We write the ngroups gids as an array after this struct */
};

#if HAVE_TASK_CRED
  typedef const struct group_info *cr_group_info_t;
  #define cr_current_groups()	((current_cred())->group_info)
  #define CR_NGROUPS_MAX	NGROUPS_MAX
  #define CR_NGROUPS(gi)	((gi)->ngroups)
  #define CR_GROUP_AT(gi,idx)	GROUP_AT((gi), (idx))
#elif HAVE_TASK_GROUP_INFO
  typedef const struct group_info *cr_group_info_t;
  #define cr_current_groups()	(current->group_info)
  #define CR_NGROUPS_MAX	NGROUPS_MAX
  #define CR_NGROUPS(gi)	((gi)->ngroups)
  #define CR_GROUP_AT(gi,idx)	GROUP_AT((gi), (idx))
#else
  typedef const struct task_struct *cr_group_info_t;
  #define cr_current_groups()	(current)
  #define CR_NGROUPS_MAX	NGROUPS
  #define CR_NGROUPS(task)	((task)->ngroups)
  #define CR_GROUP_AT(task,idx)	((task)->groups[(idx)])
#endif

int cr_load_creds(cr_rstrt_proc_req_t *proc_req)
{
    cr_errbuf_t *eb = proc_req->req->errbuf;
    struct cr_context_creds cf_creds;
    int retval;
    gid_t *groups = NULL;

    CR_KTRACE_HIGH_LVL("%d: Restoring credentials", current->pid);

    retval = cr_kread(eb, proc_req->file, &cf_creds, sizeof(cf_creds));
    if (retval < sizeof(cf_creds)) {
	CR_ERR_PROC_REQ(proc_req, "credentials: read returned %d", retval);
        goto out;
    }
     
    if (cf_creds.ngroups > CR_NGROUPS_MAX) {
	CR_ERR_PROC_REQ(proc_req, "Invalid supplemental group count (%d)", (int)cf_creds.ngroups);
	retval = -EINVAL;
	goto out;
    } else if (cf_creds.ngroups) {
	size_t sizeof_groups = cf_creds.ngroups*sizeof(gid_t);
	groups = vmalloc(sizeof_groups);
	retval = -ENOMEM;
	if (groups == NULL) {
	    goto out;
	}

	retval = cr_kread(eb, proc_req->file, groups, sizeof_groups);
	if (retval < sizeof_groups) {
	    CR_ERR_PROC_REQ(proc_req, "groups: read returned %d", retval);
            goto out_vfree;
        }
    }

#if CR_RESTORE_IDS
    if (cf_creds.ngroups) {
      #if CR_HAVE_GROUP_INFO || defined(CR_KCODE_groups_search) || !defined(CR_KCODE_supplemental_group_member)
	cr_group_info_t gi = cr_current_groups();
      #endif
	int i;
	/* Search for any required "expansion" of the group set */
	for (i = 0; i < cf_creds.ngroups; ++i) {
	    gid_t g = groups[i];
	    int gid_ok = 0; /* Assume no match for this gid */

    #if defined(CR_KCODE_groups_search)
	    gid_ok = groups_search((struct group_info *)gi, g); // search is const, but not declared as such
    #elif defined(CR_KCODE_supplemental_group_member)
	    gid_ok = supplemental_group_member(g);
    #else
	    /* Just in case groups_search() or supplemental_group_member()
	     * is not found (each was static in some kernels) */
	    int j;
	    for (j = 0; j < gi->ngroups; ++j) {
                if (g == CR_GROUP_AT(gi, j)) {
		    gid_ok = 1; /* OK, g is in the existing set */
		    break;
		}
	    }
    #endif

	    if (!gid_ok) {
	        /* If we reach here then we've seen a supplemental group in the
	         * saved context, which is not present in the current list.
		 * The set_groups() call checks permissions for us.  If we fail,
		 * then we must not have enough credentials.
		 */
		mm_segment_t oldfs = get_fs();
		set_fs(KERNEL_DS);
		retval = sys_setgroups(cf_creds.ngroups, groups);
		set_fs(oldfs);
		if (retval < 0) {
		    CR_ERR_PROC_REQ(proc_req, "Failed to restore supplemental group(s).");
		    goto out_vfree;
		}
		CR_KTRACE_LOW_LVL("Restored %d supplemental group(s)", cf_creds.ngroups);
		break; /* no need to continue the i loop */
	    }
	}
  #if CR_HAVE_GROUP_INFO
	(void)cr_insert_object(proc_req->req->map, (void *)cf_creds.group_info, (void *)gi, GFP_KERNEL);
  #endif
    }
  #if CR_HAVE_GROUP_INFO
    else {
	// NOTE: requires restore order match save order
	struct group_info *found_group_info = NULL;

	if (!cr_find_object(proc_req->req->map, (void *)cf_creds.group_info, (void **)&found_group_info)) {
	   // Nothing to do
	} else if (found_group_info != cr_current_groups()) {
	    // validation and sort were done previously, but are not worth avoiding
	    set_current_groups(found_group_info);
	    CR_KTRACE_LOW_LVL("Reuse cached group_info %p", found_group_info);
	} else {
	    CR_KTRACE_LOW_LVL("Cached group_info == current");
	}
    }
  #endif

    {
	cr_cred_t my_cred = cr_current_cred();

	/* The set_setresgid() call checks permissions for us, always OK if no change . */
	retval = sys_setresgid(cf_creds.gid, cf_creds.egid, cf_creds.sgid);
	if (retval < 0) {
	    CR_ERR_PROC_REQ(proc_req, "Failed to restore real/effective/saved gids.");
	    goto out_vfree;
	}
	CR_KTRACE_LOW_LVL("Restored gids");

	/* The set_setresuid() call checks permissions for us, always OK if no change . */
	retval = sys_setresuid(cf_creds.uid, cf_creds.euid, cf_creds.suid);
	if (retval < 0) {
	    CR_ERR_PROC_REQ(proc_req, "Failed to restore real/effective/saved uids.");
	    goto out_vfree;
	}
	CR_KTRACE_LOW_LVL("Restored uids");

        /* 
	 * sys_setresuid sets current->mm->dumpable to suid_dumpable if the
	 * call was successful.  This can have some weird side-effects on
	 * restarted jobs, like /proc/self/fd will not be accessable.  
         *
	 * We should probably save this flag and restore it if we ever
	 * support real setuid checkpoints from the user, but for now we'll
	 * just set the flag to 1 if the user had permission to restore their
	 * credentials in the first place.  This should mimic the behavior
         * of exec.
         *
         * Set the dumpable flag for the process, taken from 2.6.22 fs/exec.c
         */
        if (my_cred->euid == my_cred->uid && my_cred->egid == my_cred->gid) {
            cr_set_dumpable(current->mm, 1);
        } else {
            cr_set_dumpable(current->mm, cr_suid_dumpable);
        }
    }
#endif	 /* CR_RESTORE_IDS */

out_vfree:
    vfree(groups);

out:
    return retval;
}

int cr_save_creds(cr_chkpt_proc_req_t *proc_req)
{
    cr_errbuf_t *eb = proc_req->req->errbuf;
    struct cr_context_creds cf_creds;
    size_t sizeof_groups;
    int bytes = 0;
    int result;
    cr_cred_t my_cred = cr_current_cred();
    cr_group_info_t gi;

    cf_creds.uid   = my_cred->uid;
    cf_creds.euid  = my_cred->euid;
    cf_creds.suid  = my_cred->suid;
    cf_creds.gid   = my_cred->gid;
    cf_creds.egid  = my_cred->egid;
    cf_creds.sgid  = my_cred->sgid;

    /* save the number of groups, so we know how many to read later */
    gi = cr_current_groups();
    cf_creds.ngroups = CR_NGROUPS(gi);
#if CR_HAVE_GROUP_INFO
    cf_creds.group_info = gi;
    /* We save the entire array only on the first occurance.
     * NOTE: we currently rely on the restore order matching the save order.
     */
    if (cr_insert_object(proc_req->req->map, (void *)gi, (void *)gi, GFP_KERNEL)) {
	/* Ensure we don't save it again, and signal to restart time as well */
	cf_creds.ngroups = 0;
    }
#endif
    sizeof_groups = cf_creds.ngroups*sizeof(gid_t);

    CR_KTRACE_HIGH_LVL("Writing credentials");

    result = cr_kwrite(eb, proc_req->file, &cf_creds, sizeof(cf_creds));
    if (result < sizeof(cf_creds)) {
	CR_ERR_PROC_REQ(proc_req, "credentials: write returned %d", result);
	goto out;
    }
    bytes += result;

#if CR_HAVE_GROUP_INFO
    if (sizeof_groups != 0) {
	/* copy current->groups into an array and write it out */
	int i;
	gid_t *groups;

	result = -ENOMEM;
	groups = vmalloc(sizeof_groups);
	if (groups == NULL) {
	    goto out;
	}

	for (i=0; i<cf_creds.ngroups; ++i) {
	    groups[i] = CR_GROUP_AT(gi, i);
	}

	result = cr_kwrite(eb, proc_req->file, groups, sizeof_groups);
	vfree(groups);
    }
#else
    result = cr_kwrite(eb, proc_req->file, current->groups, sizeof_groups);
#endif

    if (result < sizeof_groups) {
	CR_ERR_PROC_REQ(proc_req, "groups: write returned %d", result);
	goto out;
    }
    bytes += result;

out:
    return ((result < 0) ? result : bytes);
}
