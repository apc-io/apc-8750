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
 * $Id: cr_timers.c,v 1.7 2008/08/18 23:16:17 phargrov Exp $
 */

#include "cr_module.h"

#include <asm/uaccess.h>
#include <linux/syscalls.h>
void cr_pause_itimers(struct itimerval *itimers) {
	static struct itimerval cr_zero_itimer = {
		it_interval:	{ tv_sec: 0, tv_usec: 0},
		it_value:	{ tv_sec: 0, tv_usec: 0},
	};
	mm_segment_t oldfs = get_fs();
	set_fs(KERNEL_DS);
	(void)sys_setitimer(ITIMER_REAL,    &cr_zero_itimer, itimers+0);
	(void)sys_setitimer(ITIMER_VIRTUAL, &cr_zero_itimer, itimers+1);
	(void)sys_setitimer(ITIMER_PROF,    &cr_zero_itimer, itimers+2);
	set_fs(oldfs);
}

void cr_resume_itimers(struct itimerval *itimers) {
	mm_segment_t oldfs = get_fs();
	set_fs(KERNEL_DS);
	(void)sys_setitimer(ITIMER_REAL,    itimers+0, NULL);
	(void)sys_setitimer(ITIMER_VIRTUAL, itimers+1, NULL);
	(void)sys_setitimer(ITIMER_PROF,    itimers+2, NULL);
	set_fs(oldfs);
}

long cr_load_itimers(cr_rstrt_proc_req_t *proc_req) {
	cr_errbuf_t *eb = proc_req->req->errbuf;
	long retval = cr_kread(eb, proc_req->file, &proc_req->itimers, sizeof(proc_req->itimers));
	if (retval != sizeof(proc_req->itimers)) {
		CR_ERR_PROC_REQ(proc_req, "itimers: read returned %ld", retval);
	}
	return retval;
}

long cr_save_itimers(cr_chkpt_proc_req_t *proc_req) {
	cr_errbuf_t *eb = proc_req->req->errbuf;
	long retval = cr_kwrite(eb, proc_req->file, &proc_req->itimers, sizeof(proc_req->itimers));
	if (retval != sizeof(proc_req->itimers)) {
		CR_ERR_PROC_REQ(proc_req, "itimers: write returned %ld", retval);
	}
	return retval;
}
