/* 
 * Berkeley Lab Checkpoint/Restart (BLCR) for Linux is Copyright (c)
 * 2003, The Regents of the University of California, through Lawrence
 * Berkeley National Laboratory (subject to receipt of any required
 * approvals from the U.S. Dept. of Energy).  All rights reserved.
 *
 * Portions may be copyrighted by others, as may be noted in specific
 * copyright notices within specific files.
 *
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
 * $Id: blcr_ksyms.h,v 1.18 2008/12/02 00:17:42 phargrov Exp $
 */

#ifndef _CR_KSYMS_H
#define _CR_KSYMS_H	1

/* Give the linker a literal address for a symbol.
 * Note that the case is preserved in the preprocessor symbols.
 *
 * Note that the CR_IMPORT_K*() must come AFTER the last use in a given
 * source file, or in a separate source file.  Otherwise, the assembler
 * may generate PC-relative calls that the ELF-validation code in some
 * 2.6.x kernels (e.g. FC3) will reject.  To meet this requirement,
 * BLCR is currently putting all IMPORTS in a separate kernel module.
 *
 *
 * Example:
 * #if defined(CR_KDATA_foo) && CR_KDATA_foo
 *    CR_IMPORT_KDATA(foo)
 * #endif
 *
 * Or, if you have the address
 *    _CR_IMPORT_KDATA(foo, 0xabcd0123)
 */

#include "blcr_config.h"

#ifndef EXPORT_SYMBOL_GPL
  #define EXPORT_SYMBOL_GPL EXPORT_SYMBOL
#endif

#if defined(__i386__) || defined(__x86_64__)
  #ifdef CONFIG_RELOCATABLE
    #define _CR_RELOC_KSYM(_addr) (_addr - CR_EXPORTED_KCODE_register_chrdev + register_chrdev)
  #else
    #define _CR_RELOC_KSYM(_addr) _addr
  #endif
  #define _CR_IMPORT_KSYM1(_name, _addr, _type)        \
	 __asm__ (".weak " #_name "\n\t"               \
		  ".type " #_name "," #_type "\n\t"    \
		  ".equ  " #_name "," #_addr "\n\t");
  #define _CR_IMPORT_KSYM(_name, _addr, _type) _CR_IMPORT_KSYM1(_name, _addr, _type) EXPORT_SYMBOL_GPL(_name);
  #define _CR_IMPORT_KDATA(_name,_addr) _CR_IMPORT_KSYM(_name, _CR_RELOC_KSYM(_addr), @object)
  #define _CR_IMPORT_KCODE(_name,_addr) _CR_IMPORT_KSYM(_name, _CR_RELOC_KSYM(_addr), @function)
#elif defined(__powerpc__) && (BITS_PER_LONG == 64)
  #define _CR_IMPORT_KSYM1(_name, _addr, _type)        \
	 __asm__ (".weak " #_name "\n\t"               \
		  ".type " #_name "," #_type "\n\t"    \
		  ".equ  " #_name "," #_addr "\n\t");
  #define _CR_IMPORT_KSYM(_name, _addr, _type) _CR_IMPORT_KSYM1(_name, _addr, _type) EXPORT_SYMBOL_GPL(_name);
  #define _CR_IMPORT_KDATA(_name,_addr) _CR_IMPORT_KSYM(_name, _addr, @object)
  #define _CR_IMPORT_KCODE(_name,_addr) _CR_IMPORT_KSYM(_name, _addr, @object)
#elif defined(__powerpc__) && (BITS_PER_LONG == 32)
  #define _CR_IMPORT_KSYM1(_name, _addr, _type)        \
	 __asm__ (".weak " #_name "\n\t"               \
		  ".type " #_name "," #_type "\n\t"    \
		  ".equ  " #_name "," #_addr "\n\t");
  #define _CR_IMPORT_KSYM(_name, _addr, _type) _CR_IMPORT_KSYM1(_name, _addr, _type) EXPORT_SYMBOL_GPL(_name);
  #define _CR_IMPORT_KDATA(_name,_addr) _CR_IMPORT_KSYM(_name, _addr, @object)
  #define _CR_IMPORT_KCODE(_name,_addr) _CR_IMPORT_KSYM(_name, _addr, @function)
#elif defined(__arm__)
  #define _CR_IMPORT_KSYM1(_name, _addr, _type)        \
       __asm__ (".weak " #_name "\n\t"               \
                ".type " #_name "," #_type "\n\t"    \
                ".equ  " #_name "," #_addr "\n\t");
  #define _CR_IMPORT_KSYM(_name, _addr, _type) _CR_IMPORT_KSYM1(_name, _addr, _type) EXPORT_SYMBOL_GPL(_name);
  #define _CR_IMPORT_KDATA(_name,_addr) _CR_IMPORT_KSYM(_name, _addr, %object)
  #define _CR_IMPORT_KCODE(_name,_addr) _CR_IMPORT_KSYM(_name, _addr, %function)
#elif defined(__sparc__)
  #define _CR_IMPORT_KSYM1(_name, _addr, _type)        \
       __asm__ (".weak " #_name "\n\t"               \
                ".type " #_name "," #_type "\n\t"    \
                ".equ  " #_name "," #_addr "\n\t");
  #define _CR_IMPORT_KSYM(_name, _addr, _type) _CR_IMPORT_KSYM1(_name, _addr, _type) EXPORT_SYMBOL_GPL(_name);
  #define _CR_IMPORT_KDATA(_name,_addr) _CR_IMPORT_KSYM(_name, _addr, @object)
  #define _CR_IMPORT_KCODE(_name,_addr) _CR_IMPORT_KSYM(_name, _addr, @function)
#else
  #error "Add your architecture to blcr_ksyms.h"
#endif
#define CR_IMPORT_KDATA(_name) _CR_IMPORT_KDATA(_name, CR_KDATA_##_name)
#define CR_IMPORT_KCODE(_name) _CR_IMPORT_KCODE(_name, CR_KCODE_##_name)

#endif
