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
 * $Id: cr_module.c,v 1.51 2008/12/16 00:45:07 phargrov Exp $
 */

#include "cr_module.h"

#include <linux/init.h>
#include <linux/moduleparam.h>

/* Find some always-exported symbols needed to validate System.map */
#include <linux/unistd.h>		/* for __NR_close */

MODULE_AUTHOR("Lawrence Berkeley National Lab " PACKAGE_BUGREPORT);
MODULE_DESCRIPTION("Berkeley Lab Checkpoint/Restart (BLCR) kernel module");
MODULE_LICENSE("GPL");

#if CR_KERNEL_TRACING
  module_param(cr_ktrace_mask, uint, 0644);
  MODULE_PARM_DESC(cr_ktrace_mask, 
		   "Mask of ORed tracing flags: see cr_ktrace.h for list");
#endif

#if CRI_DEBUG
  extern unsigned int cr_read_fault_rate;
  extern unsigned int cr_write_fault_rate;
  module_param(cr_read_fault_rate, uint, 0644);
  module_param(cr_write_fault_rate, uint, 0644);
  MODULE_PARM_DESC(cr_read_fault_rate, 
		   "Injection rate (1-in-value) of artificial read faults");
  MODULE_PARM_DESC(cr_write_fault_rate, 
		   "Injection rate (1-in-value) of artificial write faults");
#endif

extern unsigned long cr_io_max, cr_io_max_mask;
module_param(cr_io_max, ulong, 0644);
MODULE_PARM_DESC(cr_io_max, "Maximum size of an I/O request (must be a power of 2)");

cr_kmem_cache_ptr cr_pdata_cachep = NULL;
cr_kmem_cache_ptr cr_task_cachep = NULL;
cr_kmem_cache_ptr cr_chkpt_req_cachep = NULL;
cr_kmem_cache_ptr cr_chkpt_proc_req_cachep = NULL;
cr_kmem_cache_ptr cr_rstrt_req_cachep = NULL;
cr_kmem_cache_ptr cr_rstrt_proc_req_cachep = NULL;

static int __init cr_init_module(void)
{
	int err;

	err = vmadump_init_module();
	if (err) return err;

	CR_INFO("Berkeley Lab Checkpoint/Restart (BLCR) module version " PACKAGE_VERSION ".");
#if CR_KERNEL_TRACING 
	CR_INFO("  Tracing enabled (trace_mask=0x%x)", cr_ktrace_mask);
#endif
	CR_INFO("  Parameter cr_io_max = 0x%lx", cr_io_max);
#if CRI_DEBUG
	CR_INFO("  Parameter cr_read_fault_rate  = %d", cr_read_fault_rate);
	CR_INFO("  Parameter cr_write_fault_rate = %d", cr_write_fault_rate);
#endif
	CR_INFO("  Supports kernel interface version " CR_MODULE_VERSION ".");
#if CR_CONTEXT_VERSION == CR_CONTEXT_VERSION_MIN
	CR_INFO("  Supports context file format version %d.", CR_CONTEXT_VERSION);
#else
	CR_INFO("  Supports context file format versions %d though %d.",
		CR_CONTEXT_VERSION_MIN, CR_CONTEXT_VERSION);
#endif
	CR_INFO(PACKAGE_BUGREPORT);

	/* Check for correct blcr_imports */
#if 0
	if (strcmp(blcr_config_timestamp, BLCR_CONFIG_TIMESTAMP)) {
		CR_ERR("Module blcr_imports timestamp (%s) does not match that of blcr (" BLCR_CONFIG_TIMESTAMP ").", blcr_config_timestamp);
		return -EINVAL;
	}
#endif
	/* validate/setup for I/O max */
	if (cr_io_max & (cr_io_max - 1)) {
		CR_ERR("Module parameter cr_io_max must be a power of 2");
		return -EINVAL;
	}
	cr_io_max_mask = ~(cr_io_max - 1);

	err = cr_proc_init();
	if (err) {
		goto bad_proc_init;
	}

#ifdef CR_ENABLE_BOGUS_FOPS
	err = cr_fs_init();
	if (err) {
		goto bad_fs_init;
	}
#endif
#if defined(CONFIG_COMPAT) && !defined(HAVE_COMPAT_IOCTL)
	err = cr_init_ioctl32();
	if (err) {
		goto bad_ioctl32;
	}
#endif

	err = cr_rstrt_init();
	if (err) {
		goto bad_rstrt_init;
	}

	err = cr_object_init();
	if (err) {
		goto bad_object_init;
	}

	err = -ENOMEM;
	cr_pdata_cachep = KMEM_CACHE(cr_pdata_s, 0);
	if (!cr_pdata_cachep) goto no_pdata_cachep;
	cr_task_cachep = KMEM_CACHE(cr_task_s, 0);
	if (!cr_task_cachep) goto no_task_cachep;
	cr_chkpt_req_cachep = KMEM_CACHE(cr_chkpt_req_s, 0);
	if (!cr_chkpt_req_cachep) goto no_chkpt_req_cachep;
	cr_chkpt_proc_req_cachep = KMEM_CACHE(cr_chkpt_preq_s, 0);
	if (!cr_chkpt_proc_req_cachep) goto no_chkpt_proc_req_cachep;
	cr_rstrt_req_cachep = KMEM_CACHE(cr_rstrt_req_s, 0);
	if (!cr_rstrt_req_cachep) goto no_rstrt_req_cachep;
	cr_rstrt_proc_req_cachep = KMEM_CACHE(cr_rstrt_preq_s, 0);
	if (!cr_rstrt_proc_req_cachep) goto no_rstrt_proc_req_cachep;

	return 0;

no_rstrt_proc_req_cachep:
	kmem_cache_destroy(cr_rstrt_req_cachep);
no_rstrt_req_cachep:
	kmem_cache_destroy(cr_chkpt_proc_req_cachep);
no_chkpt_proc_req_cachep:
	kmem_cache_destroy(cr_chkpt_req_cachep);
no_chkpt_req_cachep:
	kmem_cache_destroy(cr_task_cachep);
no_task_cachep:
	kmem_cache_destroy(cr_pdata_cachep);
no_pdata_cachep:
	cr_object_cleanup();
bad_object_init:
	cr_rstrt_cleanup();
bad_rstrt_init:
#if defined(CONFIG_COMPAT) && !defined(HAVE_COMPAT_IOCTL)
	cr_cleanup_ioctl32();
bad_ioctl32:
#endif
#ifdef CR_ENABLE_BOGUS_FOPS
	cr_fs_cleanup();
bad_fs_init:
#endif
	cr_proc_cleanup();
bad_proc_init:
	return err;
}

static void __exit cr_cleanup_module(void)
{
#if defined(CONFIG_COMPAT) && !defined(HAVE_COMPAT_IOCTL)
	cr_cleanup_ioctl32();
#endif
#ifdef CR_ENABLE_BOGUS_FOPS
	cr_fs_cleanup();
#endif
	cr_proc_cleanup();
	cr_wd_flush();
	cr_object_cleanup();
	cr_rstrt_cleanup();
	kmem_cache_destroy(cr_rstrt_proc_req_cachep);
	kmem_cache_destroy(cr_rstrt_req_cachep);
	kmem_cache_destroy(cr_chkpt_proc_req_cachep);
	kmem_cache_destroy(cr_chkpt_req_cachep);
	kmem_cache_destroy(cr_task_cachep);
	kmem_cache_destroy(cr_pdata_cachep);
	CR_INFO("Checkpoint/Restart module removed");

	vmadump_cleanup_module();
}

module_init(cr_init_module);
module_exit(cr_cleanup_module);
