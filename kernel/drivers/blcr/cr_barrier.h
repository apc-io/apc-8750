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
 * $Id: cr_barrier.h,v 1.19 2008/06/26 00:25:29 phargrov Exp $
 */

#ifndef _CR_BARRIER_H
#define _CR_BARRIER_H        1

#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/wait.h>

// Single-use barrier which can be "notified" by a "third party" (such
// as when a task dies).
//
// A call to cr_barrier_{wait,enter}() will not return until the barrier
// has been signalled "count" times.  The term "signalling the barrier"
// is taken to mean either a call to cr_barrier_enter() or to
// cr_barrier_notify().
//
// Exactly one call to cr_barrier_{wait,enter,test}{,interruptible}() will return > 0.

typedef struct cr_barrier_s {
	wait_queue_head_t	wait;
	atomic_t		count;	// How many remain
	unsigned long		once;
	int			interrupted;
} cr_barrier_t;

extern void cr_barrier_init(cr_barrier_t *barrier, int count);
extern int __cr_barrier_enter(cr_barrier_t *barrier);

/* split-phase treatment */
extern void __cr_barrier_notify(cr_barrier_t *barrier);
extern int __cr_barrier_wait(cr_barrier_t *barrier);
extern int __cr_barrier_test(cr_barrier_t *barrier);

/* interruptible variants (returns if *any* waiter is interrupted).
 * Upon interruption returns -EINTR to the interrupted task only.
 * The other tasks will wake w/ the normal 0 or 1 return.
 * Not to be mixed w/ non-interruptible variants on the same barrier.
 */
extern int __cr_barrier_enter_interruptible(cr_barrier_t *barrier);
extern int __cr_barrier_wait_interruptible(cr_barrier_t *barrier);

/* Debugging wrapper that detects underflow */
#if CRI_DEBUG
    #define _cr_barrier_uflow_wrap_void(_name, b) do { \
	cr_barrier_t *_b = (b); \
	__##_name(_b); \
	if (atomic_read(&_b->count) < 0) \
	    CR_KTRACE_DEBUG(#_name ": underflow on " #b); \
    } while(0)
    #define _cr_barrier_uflow_wrap_int(_name, b) ({ \
	cr_barrier_t *_b = (b); \
	int _res = __##_name(_b); \
	if (atomic_read(&_b->count) < 0) \
	    CR_KTRACE_DEBUG(#_name ": underflow on " #b); \
	(_res); \
    })
#else
    #define _cr_barrier_uflow_wrap_void(_name, b)	__##_name(b)
    #define _cr_barrier_uflow_wrap_int(_name, b)	__##_name(b)
#endif

/* Tracing wrappers 
 * Note that CR_KTRACE_BARRIER(...) preprocesses to empty when not tracing.
 */
#define cr_barrier_notify(b) (void)({ \
	_cr_barrier_uflow_wrap_void(cr_barrier_notify, b);\
	CR_KTRACE_BARRIER("NOTIFY(" #b ")");\
    })
#define cr_barrier_enter(b) ({ \
	int _res; \
	CR_KTRACE_BARRIER("ENTER(" #b ") begin");\
	_res = _cr_barrier_uflow_wrap_int(cr_barrier_enter, b);\
	CR_KTRACE_BARRIER("ENTER(" #b ") returning %d", _res);\
	(_res); \
    })
#define cr_barrier_enter_interruptible(b) ({ \
	int _res; \
	CR_KTRACE_BARRIER("ENTER_INTERRUPTIBLE(" #b ") begin");\
	_res = _cr_barrier_uflow_wrap_int(cr_barrier_enter_interruptible, b);\
	CR_KTRACE_BARRIER("ENTER_INTERRUPTIBLE(" #b ") returning %d", _res);\
	(_res); \
    })
#define cr_barrier_wait(b) ({ \
	int _res; \
	CR_KTRACE_BARRIER("WAIT(" #b ") begin");\
	_res = __cr_barrier_wait(b);\
	CR_KTRACE_BARRIER("WAIT(" #b ") returning %d", _res);\
	(_res); \
    })
#define cr_barrier_wait_interruptible(b) ({ \
	int _res; \
	CR_KTRACE_BARRIER("WAIT_INTERRUPTIBLE(" #b ") begin");\
	_res = __cr_barrier_wait_interruptible(b);\
	CR_KTRACE_BARRIER("WAIT_INTERRUPTIBLE(" #b ") returning %d", _res);\
	(_res); \
    })
#define cr_barrier_test(b) ({ \
	int _res = __cr_barrier_test(b);\
	CR_KTRACE_BARRIER("TEST(" #b ") returning %d", _res);\
	(_res); \
    })

/* Macro wrapper to help keep cr_task->step accurate */
#define _cr_barrier_step_wrap(_name, _t, _b)	((_t)->step++, _name(_b))
#define CR_BARRIER_NOTIFY(_t,_b)		_cr_barrier_step_wrap(cr_barrier_notify,_t,_b)
#define CR_BARRIER_ENTER(_t,_b)			_cr_barrier_step_wrap(cr_barrier_enter,_t,_b)
#define CR_BARRIER_ENTER_INTERRUPTIBLE(_t,_b)	_cr_barrier_step_wrap(cr_barrier_enter_interruptible,_t,_b)

/* Macros for optionally blocking "read" of a barrier */
#define cr_barrier_once(b, block) ({ \
	cr_barrier_t *_b = (b); \
	int _res, _block = (block); \
	if (_block) CR_KTRACE_BARRIER("ONCE(" #b ", 1) begin");\
	_res = _block ? __cr_barrier_wait(_b) : __cr_barrier_test(_b); \
	CR_KTRACE_BARRIER("ONCE(" #b ", %d) returning %d", _block, _res);\
	(_res); \
    })
#define cr_barrier_once_interruptible(b, block) ({ \
	cr_barrier_t *_b = (b); \
	int _res, _block = (block); \
	if (_block) CR_KTRACE_BARRIER("ONCE_INTERRUPTIBLE(" #b ", 1) begin");\
	_res = _block ? __cr_barrier_wait_interruptible(_b) : __cr_barrier_test(_b); \
	CR_KTRACE_BARRIER("ONCE_INTERRUPTIBLE(" #b ", %d) returning %d", _block, _res);\
	(_res); \
    })
	
#endif
