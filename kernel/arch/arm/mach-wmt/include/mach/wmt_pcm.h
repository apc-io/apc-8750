/*++
linux/include/asm-arm/arch-wmt/wmt_pcm.h

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
#error "You must include hardware.h, not vt8500_pcm.h"
#endif

#ifndef __VT8500_PCM_H
#define __VT8500_PCM_H

/*
 *   Refer vt8500 pcm register 1.61
 *
 */
/* #define PCM_BASE_ADDR                                   0xF8160000  // 64K */

/*
 * Address
 */
#define PCM_CR_ADDR                     (0x0000+PCM_BASE_ADDR)
#define PCM_SR_ADDR                     (0x0004+PCM_BASE_ADDR)
/* Reserved 0x0008 ~ 0x000F */
#define PCM_DFCR_ADDR                   (0x0008+PCM_BASE_ADDR)
#define PCM_DIVR_ADDR                   (0x000C+PCM_BASE_ADDR)
/* Reserved 0x0020 ~ 0x007F */
#define PCM_TFIFO_ADDR                  (0x0010+PCM_BASE_ADDR)
#define PCM_TFIFO_1_ADDR                (0x0014+PCM_BASE_ADDR)

#define PCM_RFIFO_ADDR                  (0x0030+PCM_BASE_ADDR)
#define PCM_RFIFO_1_ADDR                (0x0034+PCM_BASE_ADDR)
/* Reserved 0x0100 ~ 0xFFFF */

#endif /* __VT8500_I2S_H */
