/* 
 * Berkeley Lab Checkpoint/Restart (BLCR) for Linux is Copyright (c)
 * 2007, The Regents of the University of California, through Lawrence
 * Berkeley National Laboratory (subject to receipt of any required
 * approvals from the U.S. Dept. of Energy).  All rights reserved.
 *
 * Portions may be copyrighted by others, as may be noted in specific
 * copyright notices within specific files.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id: blcr_errcodes.h,v 1.13 2008/05/11 02:18:45 phargrov Exp $
 */

/*
 * This file is included twice, once to #define the error codes, and once to
 * map the error codes to their descriptions for cr_strerror().  
 *
 * Do not put anything besides CR_ERROR_DEF() macros in this file.
 * Always APPEND to the file, to avoid renumbering existing error codes.
 */

CR_ERROR_DEF(CR_ETEMPFAIL, "Temporary error: checkpoint cancelled")
CR_ERROR_DEF(CR_EPERMFAIL, "Permanent failure: checkpoint cancelled and job killed")
CR_ERROR_DEF(CR_ENOSUPPORT, "Checkpoint support not linked into one or more processes")
CR_ERROR_DEF(CR_EVERSION, "Requested kernel interface version is not supported")
CR_ERROR_DEF(CR_EOMITTED, "Process omitted from checkpoint at a callback's request")
CR_ERROR_DEF(CR_EBADARCH, "Context file does not match the kernel architecture")
CR_ERROR_DEF(CR_EPTRACED, "One or more target processes is being ptraced")
CR_ERROR_DEF(CR_EPTRACER, "One or more target processes has ptrace children")
CR_ERROR_DEF(CR_ERSTRTABRT, "Restart aborted: restart cancelled and job killed")
CR_ERROR_DEF(CR_ERESTARTED, "Request is invalid across a restart")
CR_ERROR_DEF(CR_ENOTCB, "Call is only valid from a checkpoint callback")
CR_ERROR_DEF(CR_ENOINIT, "Calling thread has not called cr_init()")
