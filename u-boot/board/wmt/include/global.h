/*++ 
Copyright (c) 2010 WonderMedia Technologies, Inc.

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software 
Foundation, either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details. You
should have received a copy of the GNU General Public License along with this
program. If not, see http://www.gnu.org/licenses/>.

WonderMedia Technologies, Inc.
10F, 529, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C.
--*/

#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#ifndef _COMMON_DEF_H
#include "common_def.h"
#endif

#ifndef _CONFIG_H_
#include "config.h"
#endif

#ifndef _CHIPTOP_H_
#include "chiptop.h"
#endif

#ifndef _USBREG_H_
#include "../../../drivers/wmt_udcreg.h"
#endif

#define writel(val, addr)  (*(volatile unsigned long *)(addr) = (val))
#define writew(val, addr)  (*(volatile unsigned short *)(addr) = (val))
#define writeb(val, addr)  (*(volatile unsigned char *)(addr) = (val))
#define readl(addr)   (*(volatile unsigned long *)(addr))
#define readw(addr)   (*(volatile unsigned short *)(addr))
#define readb(addr)   (*(volatile unsigned char *)(addr))

#endif  /* _GLOBAL_H_ */
