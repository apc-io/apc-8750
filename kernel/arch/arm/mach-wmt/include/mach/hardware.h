/*++
linux/include/asm-arm/arch-wmt/hardware.h

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

#ifndef __ASM_ARCH_HARDWARE_H
#define __ASM_ARCH_HARDWARE_H

/*
 * Those are statically mapped PCMCIA IO space for designs using it as a
 * generic IO bus, typically with ISA parts, hardwired IDE interfaces, etc.
 * The actual PCMCIA code is mapping required IO region at run time.
 */

/*#define PCMCIA_IO_0_BASE      0xf6000000 */
/*#define PCMCIA_IO_1_BASE      0xf7000000 */


/*
 * We requires absolute addresses i.e. (PCMCIA_IO_0_BASE + 0x3f8) for
 * in*()/out*() macros to be usable for all cases.
 */
#define PCIO_BASE               0

/*
 * WMT internal I/O mappings, designed as offset addressing
 *
 *      phys != virt
 */
#define PIO_BASE                0xFE000000      /* physical start of IO space */
#define VIO_BASE                0xFE000000      /* virtual start of IO space */
#define VIO_OFFSET              0               /* x = Virtual IO address offset */
#define IO_SIZE                 0x01000000      /* 16 MB */

#define io_p2v(x)               (x)
#define io_v2p(x)               (x)

#ifndef __ASSEMBLY__    /* C language */
#include <asm/types.h>

#if 0                   /* Method 1, straight forward */

/*
 * Register pointer
 */
# define REG32_PTR(x)           ((volatile u32 *)io_p2v(x))
# define REG16_PTR(x)           ((volatile u16 *)io_p2v(x))
# define REG8_PTR(x)            ((volatile u8  *)io_p2v(x))

/*
 * Register value
 */
# define REG32_VAL(x)           (*(REG32_PTR(x)))
# define REG16_VAL(x)           (*(REG16_PTR(x)))
# define REG8_VAL(x)            (*(REG8_PTR(x)))

#else                   /* Method 2, GNU's original method */

/*
 * This REGxx_VAL() version gives the same results as the one above,
 * except that we are fooling gcc somehow so it generates far better and
 * smaller assembly code for access to contigous registers.  It's a shame
 * that gcc doesn't guess this by itself.
 */
typedef struct {
	volatile u32 offset[4096];      /* 4K * 4 = SZ_16K */

} __regbase32;

typedef struct {
	volatile u16 offset[4096];      /* 4K * 2 = SZ_8K */

} __regbase16;

typedef struct {
	volatile u8 offset[4096];       /* 4K * 1 = SZ_4K */

} __regbase8;

# define __REG32P(x)    (((__regbase32 *)((x)&~4095))->offset[((x)&4095)>>2])
# define __REG16P(x)    (((__regbase16 *)((x)&~4095))->offset[((x)&4095)>>1])
# define __REG8P(x)     (((__regbase8 *)((x)&~4095))->offset[((x)&4095)>>0])
# define __REGP(x)      (((__regbase32 *)((x)&~4095))->offset[((x)&4095)>>2])

/*
 * Register pointer
 */
# define REG32_PTR(x)   (&(__REG32P(io_p2v(x))))
# define REG16_PTR(x)   (&(__REG16P(io_p2v(x))))
# define REG8_PTR(x)    (&(__REG8P(io_p2v(x))))

/*
 * Register value
 */
# define REG32_VAL(x)   __REG32P(io_p2v(x))
# define REG16_VAL(x)   __REG16P(io_p2v(x))
# define REG8_VAL(x)    __REG8P(io_p2v(x))

#endif

/*
 * General 32-bit Register value
 */
# define __REG(x)       REG32_VAL((x))

/*
 * Pointer and Value for Memory
 */
# define MEM32_PTR(x)           REG32_PTR(x)
# define MEM16_PTR(x)           REG16_PTR(x)
# define MEM8_PTR(x)            REG8_PTR(x)

# define MEM32_VAL(x)           REG32_VAL(x)
# define MEM16_VAL(x)           REG16_VAL(x)
# define MEM8_VAL(x)            REG8_VAL(x)

/*
 * Physical Register from virtual address
 */
# define __PREG32(x)    (io_v2p((u32)&(x)))
# define __PREG16(x)    (io_v2p((u16)&(x)))
# define __PREG8(x)     (io_v2p((u8)&(x)))

# define __PREG(x)      __PREG32(x)

#else                   /* Assembly */

# define __REG(x)       io_p2v(x)
# define __PREG(x)      io_v2p(x)

# define REG32_PTR(x)           io_p2v(x)
# define REG16_PTR(x)           io_p2v(x)
# define REG8_PTR(x)            io_p2v(x)

#endif

#include "wmt.h"     /* Memory map entry */
#include "../../wmt_clk.h"

/*
 * VT8500 GPIO edge detection for IRQs:
 * IRQs are generated on High, Low, Falling-Edge, and Rising-Edge.
 * This must be called *before* the corresponding IRQ is registered.
 */
#define GPIO_HIGH               0
#define GPIO_LOW                1
#define GPIO_FALLING            2
#define GPIO_RISING             3

/*{JHT 2007/01/31 */
#define pcibios_assign_all_busses()     1
#define PCIBIOS_MIN_IO          0x6000
#define PCIBIOS_MIN_MEM         0x50000000
/*}JHT 2007/01/31 */
/*#define PCIMEM_BASE		0xe8000000*/

#endif  /* __ASM_ARCH_HARDWARE_H */
