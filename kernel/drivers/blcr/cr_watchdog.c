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
 * $Id: cr_watchdog.c,v 1.7 2008/08/21 05:22:49 phargrov Exp $
 */

#include "cr_module.h"

#include <linux/time.h>
#include <linux/workqueue.h>

// Interval for the watchdog to wakeup HZ = 1s
#define CR_WD_INTERVAL	(HZ)

#if HAVE_STRUCT_DELAYED_WORK
  typedef struct work_struct * cr_work_arg_t;
  #define CR_DECLARE_DELAYED_WORK DECLARE_DELAYED_WORK
#else /* struct work_struct */
  typedef void *cr_work_arg_t;
  #define CR_DECLARE_DELAYED_WORK(_a,_b) DECLARE_WORK((_a),(_b),NULL)
#endif

static void cr_wd_run(cr_work_arg_t arg);
static CR_DECLARE_DELAYED_WORK(cr_wd_work, cr_wd_run);
static DECLARE_MUTEX(cr_wd_sem);
static LIST_HEAD(cr_wd_list);
static int cr_wd_live = 0;

static void cr_wd_run(cr_work_arg_t arg)
{
	cr_work_t *work, *next;

	down(&cr_wd_sem);
	list_for_each_entry_safe(work, next, &cr_wd_list, list) {
		(work->f)(work);
	}
	if (list_empty(&cr_wd_list)) {
		cr_wd_live = 0;
	} else {
		schedule_delayed_work(&cr_wd_work, CR_WD_INTERVAL);
	}
	up(&cr_wd_sem);
}

void cr_wd_add(cr_work_t *work)
{
	down(&cr_wd_sem);
	list_add_tail(&work->list, &cr_wd_list);
	if (!cr_wd_live) {
		cr_wd_live = 1;
		schedule_delayed_work(&cr_wd_work, CR_WD_INTERVAL);
	}
	up(&cr_wd_sem);
}

/* Returns
 * - non-zero IFF actually removed
 * - zero if was not on the work list yet/anymore
 */
int cr_wd_del(cr_work_t *work)
{
	int retval;

	down(&cr_wd_sem);
	retval = !list_empty(&work->list);
	if (retval) __cr_wd_del(work);
	up(&cr_wd_sem);

	return retval;
}

void cr_wd_flush(void) {
	cancel_delayed_work(&cr_wd_work);
	flush_scheduled_work();
}
