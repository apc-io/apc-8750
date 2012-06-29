/*-------------------------------------------------------------------------
 *  vmadump_arm:  ARM specific dumping/undumping routines
 *
 *  Copyright (C) 1999-2001 by Erik Hendriks <erik@hendriks.cx>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id: vmadump_arm.c,v 1.8 2008/06/04 22:19:15 phargrov Exp $
 *
 *  Experimental ARM support contributed by Anton V. Uzunov
 *  <anton.uzunov@dsto.defence.gov.au> of the Australian Government
 *  Department of Defence, Defence Science and Technology Organisation.
 *
 *  ARM-specific questions should be directed to blcr-arm@hpcrd.lbl.gov.
 *
 *  THIS FILE IS SPECIFIC TO BLCR <http://ftg.lbl.gov/checkpoint>
 *-----------------------------------------------------------------------*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>

#define __VMADUMP_INTERNAL__
#include "vmadump.h"

#ifdef VMAD_DEBUG_FLAG
  #define VMAD_DEBUG(arg...)	CR_KTRACE_DEBUG( arg )
#else
  #define VMAD_DEBUG(arg...)
#endif

long vmadump_store_cpu(cr_chkpt_proc_req_t *ctx, struct file *file,
                       struct pt_regs *regs)
{
  long bytes = 0;
  long r;
  int i;
#if HAVE_THREAD_INFO_TP_VALUE
  struct thread_info *thread = current_thread_info();
#endif
  struct task_struct *tsk = current;
  struct thread_struct *thread_fault_info = &tsk->thread;

  // Store struct pt_regs
  r = write_kern(ctx, file, regs, sizeof(*regs));
  if (r != sizeof(*regs)) goto err;
  bytes += r;
  // debug trace
  for (i = 0; i < 17; i++) {
    VMAD_DEBUG( "vmadump module: regs[ %d ] == %ld", i, regs->uregs[ i ] );
  }

  // Store thread's fault info
  r = write_kern(ctx, file, &thread_fault_info->address,
                 sizeof(thread_fault_info->address));
  if (r != sizeof(thread_fault_info->address)) goto err;
  bytes += r;
  VMAD_DEBUG( "vmadump: thread_fault_info->address == %ld",
              thread_fault_info->address );
  r = write_kern(ctx, file, &thread_fault_info->trap_no,
                 sizeof(thread_fault_info->trap_no));
  if (r != sizeof(thread_fault_info->trap_no)) goto err;
  bytes += r;
  VMAD_DEBUG( "vmadump: thread_fault_info->trap_no == %ld",
              thread_fault_info->trap_no );
  r = write_kern(ctx, file, &thread_fault_info->error_code,
                 sizeof(thread_fault_info->error_code));
  if (r != sizeof(thread_fault_info->error_code)) goto err;
  bytes += r;
  VMAD_DEBUG( "vmadump: thread_fault_info->error_code == %ld",
              thread_fault_info->error_code );

#if HAVE_THREAD_INFO_TP_VALUE
  // Store thread-specific pointer
  r = write_kern(ctx, file, &thread->tp_value,
                 sizeof(thread->tp_value));
  if (r != sizeof(thread->tp_value)) goto err;
  bytes += r;
  VMAD_DEBUG( "vmadump: thread->tp_value == %ld",
              thread->tp_value );
#endif

  return( bytes );

err:
  if (r >= 0) r = -EIO;
  return( r );
}


int vmadump_restore_cpu(cr_rstrt_proc_req_t *ctx, struct file *file,
                        struct pt_regs *regs)
{
  struct thread_info *thread = current_thread_info();
  struct pt_regs regtmp;
  int r;
  int i;

  // read in pt_regs struct and make sure the regs are somewhat valid
  r = read_kern(ctx, file, &regtmp, sizeof(regtmp));
  if (r != sizeof(regtmp) || !valid_user_regs(&regtmp))
    goto bad_read;
  // restore CPU registers
  memcpy(regs, &regtmp, sizeof(regtmp));
  // debug trace
  for (i = 0; i < 17; i++) {
    VMAD_DEBUG( "vmadump module: regs[ %d ] == %ld", i, regs->uregs[ i ] );
  }

  // restore thread fault info
  r = read_kern(ctx, file, &current->thread.address,
                sizeof(current->thread.address));
  if (r != sizeof(current->thread.address)) goto bad_read;
  VMAD_DEBUG( "vmadump: current->thread.address == %ld",
              current->thread.address );
  r = read_kern(ctx, file, &current->thread.trap_no,
                sizeof(current->thread.trap_no));
  if (r != sizeof(current->thread.trap_no)) goto bad_read;
  VMAD_DEBUG( "vmadump: current->thread.trap_no == %ld",
              current->thread.trap_no );
  r = read_kern(ctx, file, &current->thread.error_code,
                sizeof(current->thread.error_code));
  if (r != sizeof(current->thread.error_code)) goto bad_read;
  VMAD_DEBUG( "vmadump: current->thread.error_code == %ld",
              current->thread.error_code );
  
#if HAVE_THREAD_INFO_TP_VALUE
  // retore thread-specific pointer
  r = read_kern(ctx, file, &thread->tp_value,
                 sizeof(thread->tp_value));
  if (r != sizeof(thread->tp_value)) goto bad_read;
  VMAD_DEBUG( "vmadump: thread->tp_value == %ld",
              thread->tp_value );
 #if defined(CONFIG_HAS_TLS_REG)
  asm ("mcr p15, 0, %0, c13, c0, 3" : : "r" (thread->tp_value) );
 #elif !defined(CONFIG_TLS_REG_EMUL)
  #if defined(CR_KCODE___kuser_helper_start) /* NPTL support code in 2.6.12 and later */
   *((unsigned int *)0xffff0ff0) = (thread->tp_value);
  #else
   // The 2.6.11 kernel did this differently.
   *((unsigned int *)0xffff0ffc) = (thread->tp_value);
  #endif
 #endif
#endif

  return( 0 );

bad_read:
  if (r >= 0) r = -EIO;
  return( r );
}

/*
 * Local variables:
 * c-basic-offset: 2
 * End:
 */
