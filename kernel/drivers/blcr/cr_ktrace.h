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
 * $Id: cr_ktrace.h,v 1.17 2008/08/18 23:20:07 phargrov Exp $
 *
 *
 * Tracing and error/warning output for checkpoint/restart kernel code.
 */
#ifndef _CR_KTRACE_H
#define _CR_KTRACE_H

#include "cr_module.h"

/* 
 * Error/warning message generation
 * For problems (mostly) that indicate BLCR-internal problems such as
 * bad assumptions about data structures, or bad usage of the internal
 * interfaces.
 *
 * By default, regular messages (CR_INFO, CR_WARN, CR_ERR) are always on:
 * #define CR_QUIET to turn off.
 */
#define CR_QUIET
#ifdef CR_QUIET
# define CR_ERR(fmt, args...)
# define CR_WARN(fmt, args...)
# define CR_INFO(fmt, args...)
#else
# define CR_ERR(fmt, args...)	printk(KERN_ERR	    "blcr: " fmt "\n", ## args)
# define CR_WARN(fmt, args...)	printk(KERN_WARNING "blcr: " fmt "\n", ## args)
# define CR_INFO(fmt, args...)	printk(KERN_INFO    "blcr: " fmt "\n", ## args)
#endif


/*
 * For reporting problems w/ a specific user request
 */
#define _CR_EB_OUT(lvl, eb, fmt, args...) \
       cr_errbuf_printf(lvl, eb, fmt "\n", ## args)

/* Error/warning to specific errbuf */
#define CR_ERR_EB(eb, fmt, args...) \
       _CR_EB_OUT(KERN_ERR, eb, fmt, ## args)
#define CR_WARN_EB(eb, fmt, args...) \
       _CR_EB_OUT(KERN_WARNING, eb, fmt, ## args)

/* Error/warning to errbuf of given req */
/* NOTE: macro works same for chkpt or rstrt req */
#define _cr_req_eb(_req) ({     \
    typeof(_req) _tmp = (_req); \
    _tmp ? _tmp->errbuf : NULL; \
  })
#define CR_ERR_REQ(req, fmt, args...)  \
       CR_ERR_EB(_cr_req_eb(req), fmt, ## args)
#define CR_WARN_REQ(req, fmt, args...)  \
       CR_WARN_EB(_cr_req_eb(req), fmt, ## args)

/* Error/warning to errbuf of given proc_req */
/* NOTE: macro works same for chkpt or rstrt proc_req */
#define _cr_proc_req_eb(_proc_req) ({     \
    typeof(_proc_req) _tmp = (_proc_req); \
    _tmp ? _tmp->req->errbuf : NULL; \
  })
#define CR_ERR_PROC_REQ(proc_req, fmt, args...)  \
       CR_ERR_EB(_cr_proc_req_eb(proc_req), fmt, ## args)
#define CR_WARN_PROC_REQ(proc_req, fmt, args...)  \
       CR_WARN_EB(_cr_proc_req_eb(proc_req), fmt, ## args)

/* Opaque type */
struct cr_errbuf_s;
typedef struct cr_errbuf_s cr_errbuf_t;

extern cr_errbuf_t *cr_errbuf_alloc(void);
extern void cr_errbuf_free(cr_errbuf_t *eb);
extern int cr_errbuf_read(char /*__user*/ *buf, unsigned int len, cr_errbuf_t *eb);

extern int cr_errbuf_printf(const char *lvl, cr_errbuf_t *eb, const char * fmt, ...)
		__attribute__((format (printf, 3, 4)));


/* 
 * Tracing messages:  by default, turned off.  #define CR_USE_KTRACE to turn
 * on tracing, and set what types of tracing msgs to see by setting
 * cr_ktrace_mask (default is all tracing msgs).
 */

#if CR_KERNEL_TRACING
  /* for debugging ONLY: remove calls before checking in */
# define CR_KTRACE_MARK()		printk(KERN_ERR	    "@ %s:%d\n", __FILE__, __LINE__)
# define CR_KTRACE_DEBUG(args...)	\
	 CR_KTRACE(CR_KTRACETYPE_DEBUG, args)
  /* for major high-level events ex: "phase 2 entered" */
# define CR_KTRACE_HIGH_LVL(args...)	\
	 CR_KTRACE(CR_KTRACETYPE_HIGH_LVL, args)
  /* more detailed events ex: "woke up thread 4" */
# define CR_KTRACE_LOW_LVL(args...)	\
	 CR_KTRACE(CR_KTRACETYPE_LOW_LVL, args)
  /* entering function */
# define CR_KTRACE_FUNC_ENTRY(args...)	\
	 CR_KTRACE(CR_KTRACETYPE_FUNC_ENTRY, "entering " args)
  /* exiting function */
# define CR_KTRACE_FUNC_EXIT(args...)	\
	 CR_KTRACE(CR_KTRACETYPE_FUNC_EXIT, "leaving " args)
  /* refcounting */
# define CR_KTRACE_REFCNT(args...)	\
	 CR_KTRACE(CR_KTRACETYPE_REFCNT, args)
  /* resource (de)allocation */
# define CR_KTRACE_ALLOC(args...)	\
	 CR_KTRACE(CR_KTRACETYPE_ALLOC, args)
  /* fixed limit reached */
# define CR_KTRACE_LIMIT(args...)	\
	 CR_KTRACE(CR_KTRACETYPE_LIMIT, args)
  /* bad parameter recv'd */
# define CR_KTRACE_BADPARM(args...)	\
	 CR_KTRACE(CR_KTRACETYPE_BADPARM, "EINVAL: " args)
  /* unusual case encountered: */
# define CR_KTRACE_UNEXPECTED(args...)	\
	 CR_KTRACE(CR_KTRACETYPE_UNEXPECTED, args)
  /* barrier operations: */
# define CR_KTRACE_BARRIER(args...)	\
	 CR_KTRACE(CR_KTRACETYPE_BARRIER, args)


/* Tracing flags:
 *  - Flags that are normally on go in lowest 2 bytes, 
 *    and less commonly used ones in upper 2 bytes
 */
# define CR_KTRACETYPE_DEBUG		0x00000001 
# define CR_KTRACETYPE_HIGH_LVL		0x00000002 
# define CR_KTRACETYPE_LIMIT		0x00000004 
# define CR_KTRACETYPE_BADPARM		0x00000008
# define CR_KTRACETYPE_UNEXPECTED	0x00000010

# define CR_KTRACETYPE_LOW_LVL		0x00010000 
# define CR_KTRACETYPE_FUNC_ENTRY	0x00020000 
# define CR_KTRACETYPE_FUNC_EXIT	0x00040000 
# define CR_KTRACETYPE_REFCNT		0x00080000 
# define CR_KTRACETYPE_ALLOC		0x00100000 
# define CR_KTRACETYPE_BARRIER		0x00200000 

/*  Common combinations, and default tracing setting */
# define CR_KTRACE_NONE	   0x00000000
# define CR_KTRACE_ALL	   0xFFFFFFFF
# define CR_KTRACE_FUNCS   (CR_KTRACETYPE_FUNC_ENTRY|CR_KTRACETYPE_FUNC_EXIT)
# define CR_KTRACE_USUAL_SUSPECTS 0x0000ffff
# define CR_KTRACE_DEFAULT CR_KTRACE_USUAL_SUSPECTS
//# define CR_KTRACE_DEFAULT CR_KTRACE_ALL

# define CR_KTRACE(type, args...)                                            \
	 if (cr_ktrace_mask & (type)) {                                      \
		cr_print_trace(__FILE__, __LINE__, __FUNCTION__, args);      \
	 }
 extern unsigned int cr_ktrace_mask;
 extern void 
 cr_print_trace(const char *file, int line, const char *func, const char *fmt, ...)
		__attribute__((format (printf, 4, 5)));
#else
# define CR_KTRACE_MARK()
# define CR_KTRACE(args...)
# define CR_KTRACE_HIGH_LVL(args...)
# define CR_KTRACE_LOW_LVL(args...)
# define CR_KTRACE_FUNC_ENTRY(args...)
# define CR_KTRACE_FUNC_EXIT(args...)
# define CR_KTRACE_REFCNT(args...)
# define CR_KTRACE_ALLOC(args...)
# define CR_KTRACE_LIMIT(args...)
# define CR_KTRACE_BADPARM(args...)
# define CR_KTRACE_UNEXPECTED(args...)
# define CR_KTRACE_BARRIER(args...)
#endif  /* CR_KERNEL_TRACING */

/* Not quite tracing, but this is as good a place as any for this */
#if CRI_DEBUG
  #define CRI_ASSERT(cond) BUG_ON(!(cond))
#else
  #define CRI_ASSERT(cond) do {} while (0)
#endif


#endif  /* _CR_KTRACE_H */
