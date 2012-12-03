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
 * $Id: cr_ktrace.c,v 1.15 2008/12/11 06:33:39 phargrov Exp $
 *
 *
 * Tracing and error/warning output for checkpoint/restart kernel code.
 */

#include "cr_module.h"

#include <asm/uaccess.h>
#include "cr_ktrace.h"

static char  cr_trace_buf[4096];
static CR_DEFINE_SPINLOCK(cr_trace_lock);

#if CR_KERNEL_TRACING

unsigned int cr_ktrace_mask = CR_KTRACE_DEFAULT;

void
cr_print_trace(const char *file, int line, const char *func, const char * fmt, ...)
{
	va_list args;
	const char *p;

	spin_lock(&cr_trace_lock);

	va_start(args, fmt);
	vsprintf(cr_trace_buf, fmt, args);
	va_end(args);

	/* simple basename implementation: */
        for (p = file; *p != '\0'; ++p) {
	    if (*p == '/') {
		file = p + 1;
	    }
	}

	printk(KERN_INFO "%s <%s:%d>, pid %d: %s\n", func, file, line,
	       current->pid, cr_trace_buf);

	spin_unlock(&cr_trace_lock);
}

#endif 

/*
 * Error/warning generation
 */

static CR_DEFINE_SPINLOCK(cr_errbuf_lock);

struct cr_errbuf_s {
    unsigned short	used, remain;
    char		buffer[0];  // GNU (not ISO C99) variable length array
};

cr_errbuf_t *cr_errbuf_alloc(void) {
    cr_errbuf_t *p = (cr_errbuf_t *)__get_free_page(GFP_KERNEL);
    p->used = 0;
    p->remain = PAGE_SIZE - sizeof(cr_errbuf_t);
    return p;
}

void cr_errbuf_free(cr_errbuf_t *eb) {
    free_page((unsigned long)eb);
}

static int
cr_errbuf_vprintf(const char *lvl, cr_errbuf_t *eb, const char * fmt, va_list args)
{
    char *p;
    int len;

    if (!eb || !eb->remain) {
#ifndef CR_QUIET
	spin_lock(&cr_trace_lock);
	vsprintf(cr_trace_buf, fmt, args);
	printk("%sblcr: %s", lvl, cr_trace_buf);
	spin_unlock(&cr_trace_lock);
#endif
	return 0;
    }

    spin_lock(&cr_errbuf_lock);

    p = eb->buffer + eb->used;
    len = vsnprintf(p, eb->remain, fmt, args);

    if (len > 0) {
#ifndef CR_QUIET
	printk("%sblcr: %s", lvl, p);
#endif
	if (len > eb->remain) {
	    eb->used += eb->remain;
	    eb->remain = 0;
	} else {
	    eb->remain -= len;
	    eb->used += len;
	}
    }

    spin_unlock(&cr_errbuf_lock);

    return len;
}
    
int cr_errbuf_printf(const char *lvl, cr_errbuf_t *eb, const char * fmt, ...)
{
    va_list args;
    int len;

    va_start(args, fmt);
    len = cr_errbuf_vprintf(lvl, eb, fmt, args);
    va_end(args);

    return len;
}

/*
 * Copy eb contents up to len bytes (including final nul)
 * Returns space that would be required to accept full contents.
 * If len is zero, buf is ignored.
 */
int cr_errbuf_read(char /*__user*/ *buf, unsigned int len, cr_errbuf_t *eb)
{
    if (!eb) {
	return -EINVAL;
    }

    if (len) {
	/* Don't read past used space (rest of buffer uninitialized == security risk)
	 * or write beyond user's buffer.
	 */
	len = min((len-1), (unsigned int)(eb->used));

	/* Copy to user if non-empty */
	if (len && copy_to_user(buf, eb->buffer, len)) {
	    return -EFAULT;
	}

	/* Add nul termination */
	if (put_user('\0', buf+len)) {
	    return -EFAULT;
	}
    }

    return eb->used ? (eb->used + 1) : 0;
}


