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
 * $Id: cr_barrier.c,v 1.16 2008/09/12 18:56:04 phargrov Exp $
 */

#include "cr_module.h"

// Helpers

#define CR_BARRIER_WAIT_COND(b)	((atomic_read(&((b)->count)) <= 0) || (b)->interrupted)

static inline int do_barrier_once(cr_barrier_t *barrier)
{
    smp_rmb();
    return !test_and_set_bit(0, &barrier->once);
}

// Initializer
void cr_barrier_init(cr_barrier_t *barrier, int count)
{
    init_waitqueue_head(&barrier->wait);
    atomic_set(&barrier->count, count);
    barrier->once = 0;
    barrier->interrupted = 0;
}

// Called to signal a barrier, possibly waking any waiters.
// Can be called by a third party if somebody dies unexpectedly.
void __cr_barrier_notify(cr_barrier_t *barrier)
{
    smp_wmb();
    if (atomic_dec_and_test(&barrier->count)) {
	wake_up(&barrier->wait);
    }
}

// Called to wait on a barrier
int __cr_barrier_wait(cr_barrier_t *barrier)
{
#if CRI_DEBUG
    DECLARE_WAIT_QUEUE_HEAD_ONSTACK(dummy);
    const int interval = 60;
    int sum = 0;
    do {
        wait_event_timeout(barrier->wait, CR_BARRIER_WAIT_COND(barrier), interval * HZ);
	if (CR_BARRIER_WAIT_COND(barrier)) break;
	sum += interval;
	CR_ERR("cr_barrier warning: tgid/pid %d/%d still blocked after %d seconds, with signal_pending=%d.", current->tgid, current->pid, sum, signal_pending(current));
    } while (1);
    smp_rmb();
    if (barrier->interrupted) {
	CR_ERR("cr_barrier error: interrupt on non-interruptible barrier");
    }
#else
    wait_event(barrier->wait, CR_BARRIER_WAIT_COND(barrier));
#endif
    return do_barrier_once(barrier);
}

// Called to interruptible wait on a barrier
int __cr_barrier_wait_interruptible(cr_barrier_t *barrier)
{
    int retval = wait_event_interruptible(barrier->wait, CR_BARRIER_WAIT_COND(barrier));
    if (retval != 0) {
	/* Interrupted.  Wake others. */
	barrier->interrupted = 1;
	smp_wmb();
	wake_up(&barrier->wait);
	retval = -EINTR;
    } else {
    	retval = do_barrier_once(barrier);
    }
    return retval;
}

// Called to signal a barrier, blocking for all others to do so too.
// See cr_barrier.h for more information.
int __cr_barrier_enter(cr_barrier_t *barrier)
{
    __cr_barrier_notify(barrier);
    return __cr_barrier_wait(barrier);
}

int __cr_barrier_enter_interruptible(cr_barrier_t *barrier)
{
    __cr_barrier_notify(barrier);
    return __cr_barrier_wait_interruptible(barrier);
}

// Called to test a barrier
int __cr_barrier_test(cr_barrier_t *barrier)
{
    return CR_BARRIER_WAIT_COND(barrier) && do_barrier_once(barrier);
}
