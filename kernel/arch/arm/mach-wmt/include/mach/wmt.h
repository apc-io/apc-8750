/*++
linux/include/asm-arm/arch-wmt/wmt.h

Copyright (c) 2008  WonderMedia Technologies, Inc.

This program is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software Foundation,
either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with
this program.  If not, see <http://www.gnu.org/licenses/>.

WonderMedia Technologies, Inc.
10F, 529, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C.
--*/

/* Be sure that virtual mapping is defined right */
#ifndef __ASM_ARCH_HARDWARE_H
#error "You must include hardware.h, not wmt.h"
#endif

#ifndef __WMT_H
#define __WMT_H
#include "wmt_mmap.h"        /* register base address definitions */
#include "common_def.h"      /* common define */
#include "wmt_mc.h"          /* memory controller */
#include "wmt_dma.h"         /* dma controller */
#include "wmt_scc.h"         /* system configuration controller */
#include "wmt_sdmmc.h"       /* sd/mmc card controller */
#include "wmt_uart.h"        /* uart controller */
#include "wmt_rtc.h"         /* real time clock */
#include "wmt_gpio.h"        /* gpio controller */
#include "wmt_pmc.h"         /* power management controller */
#include "wmt_intc.h"        /* interrupt controller */
#include "wmt_sf.h"          /* spi flash controller */
#include "wmt_ac97.h"        /* ac97 controller */
#include "wmt_i2s.h"         /* i2s controller */
#include "wmt_pcm.h"         /* pcm controller */
#include "wmt_evb.h"         /* evb definitions */
#include "wmt_i2c.h"         /* i2c address */
#include "wmt_kpad.h"        /* kpad address */
#include "wmt_i8042.h"       /* KBDC address */

#ifndef __ASSEMBLY__
extern unsigned int processor_id;
#endif

#define CPU_REVISION		(processor_id & 0xf)  /* to be removed */
#define CPU_VT8500_ID		(0x00123000)          /* need to review with arch/kernel/setup.c */
#define	CPU_VT8500_MASK		(0xfffffff0)

#endif /* __WMT_H */
