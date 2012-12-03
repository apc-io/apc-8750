/*++ 
Copyright (c) 2010 WonderMedia Technologies, Inc.

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software 
Foundation, either version 2 of the License, or (at your option) any later 
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details. You
should have received a copy of the GNU General Public License along with this
program. If not, see http://www.gnu.org/licenses/>.

WonderMedia Technologies, Inc.
10F, 529, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C.
--*/

/*
 *  linux/include/asm-arm/mach/multicard.h
 */
#ifndef ASMARM_MACH_Multicard_H
#define ASMARM_MACH_Multicard_H

#define  MSP_ID  0x01
#define  XDC_ID  0x02
#define  SDC_ID  0x04
#define  CFC_ID  0x08

#define  NO_ACTION   0
#define  HAVE_ACTION 1
#define  CLR_ACTION  2 // clear card-change int status

struct context {
	void *cf_cxt;
	void *ms_cxt;
	void *xd_cxt;
	void *sd_cxt;
} ;
//int common_func( void * dev_id, int id);
//int card_exist( void * dev_id, int id);

int common_func( int id,int self);
int card_exist( int id);

irqreturn_t atmsb_regular_isr(int irq, void *dev_id, struct pt_regs *regs);
irqreturn_t via_ata_interrupt(int irq, void *dev_instance, struct pt_regs *regs);
irqreturn_t atsmb_regular_isr(int irq, void *dev_id);
irqreturn_t atxdb_regular_isr(int irq, void *dev_id, struct pt_regs *regs);
#endif
