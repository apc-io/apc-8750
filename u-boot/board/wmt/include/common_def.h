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

#ifndef _COMMON_DEF_H
#define _COMMON_DEF_H

/*
 *    Common Constant define
 */

/*
 *      PTR and VAL
 */

#define REG32_VAL(addr) (*(volatile unsigned int *)(addr))
#define REG16_VAL(addr) (*(volatile unsigned short *)(addr))
#define REG8_VAL(addr)  (*(volatile unsigned char *)(addr))

#define REG32_PTR(addr) ((volatile unsigned int *)(addr))
#define REG16_PTR(addr) ((volatile unsigned short *)(addr))
#define REG8_PTR(addr)  ((volatile unsigned char *)(addr))

/*
 *      PTR and VAL for Memory
 */
#define MEM32_VAL(addr) (*(volatile unsigned int *)(addr))
#define MEM16_VAL(addr) (*(volatile unsigned short *)(addr))
#define MEM8_VAL(addr)  (*(volatile unsigned char *)(addr))

#define MEM32_PTR(addr) ((volatile unsigned int *)(addr))
#define MEM16_PTR(addr) ((volatile unsigned short *)(addr))
#define MEM8_PTR(addr)  ((volatile unsigned char *)(addr))


#define U32 unsigned int
#define U16 unsigned short
#define S32 int
#define S16 short int
#define U8  unsigned char
#define S8  char

/*
 * ------------------------------------------
 */
#define BIT0                0x00000001
#define BIT1                0x00000002
#define BIT2                0x00000004
#define BIT3                0x00000008
#define BIT4                0x00000010
#define BIT5                0x00000020
#define BIT6                0x00000040
#define BIT7                0x00000080
#define BIT8                0x00000100
#define BIT9                0x00000200
#define BIT10               0x00000400
#define BIT11               0x00000800
#define BIT12               0x00001000
#define BIT13               0x00002000
#define BIT14               0x00004000
#define BIT15               0x00008000
#define BIT16               0x00010000
#define BIT17               0x00020000
#define BIT18               0x00040000
#define BIT19               0x00080000
#define BIT20               0x00100000
#define BIT21               0x00200000
#define BIT22               0x00400000
#define BIT23               0x00800000
#define BIT24               0x01000000
#define BIT25               0x02000000
#define BIT26               0x04000000
#define BIT27               0x08000000
#define BIT28               0x10000000
#define BIT29               0x20000000
#define BIT30               0x40000000
#define BIT31               0x80000000

/*
 * -----------------------------------------
 */
#define SIZE_1B             0x00000001
#define SIZE_2B             0x00000002
#define SIZE_4B             0x00000004
#define SIZE_8B             0x00000008
#define SIZE_16B            0x00000010
#define SIZE_32B            0x00000020
#define SIZE_64B            0x00000040
#define SIZE_128B           0x00000080
#define SIZE_256B           0x00000100
#define SIZE_512B           0x00000200
#define SIZE_1KB            0x00000400
#define SIZE_2KB            0x00000800
#define SIZE_4KB            0x00001000
#define SIZE_8KB            0x00002000
#define SIZE_16KB           0x00004000
#define SIZE_32KB           0x00008000
#define SIZE_64KB           0x00010000
#define SIZE_128KB          0x00020000
#define SIZE_256KB          0x00040000
#define SIZE_512KB          0x00080000
#define SIZE_1MB            0x00100000
#define SIZE_2MB            0x00200000
#define SIZE_4MB            0x00400000
#define SIZE_8MB            0x00800000
#define SIZE_16MB           0x01000000
#define SIZE_32MB           0x02000000
#define SIZE_64MB           0x04000000
#define SIZE_128MB          0x08000000
#define SIZE_256MB          0x10000000
#define SIZE_512MB          0x20000000
#define SIZE_1GB            0x40000000
#define SIZE_2GB            0x80000000

/*
 * Get any byte from a word
 */
#define GET_LE_BYTE0(x)          ((unsigned char)((x) & 0xFF))
#define GET_LE_BYTE1(x)          ((unsigned char)((x) >> 8 & 0xFF))
#define GET_LE_BYTE2(x)          ((unsigned char)((x) >> 16 & 0xFF))
#define GET_LE_BYTE3(x)          ((unsigned char)((x) >> 24 & 0xFF))

/*
 *  !!! Special Note !!! for packed
 *
 *  use packed that will treat all member as "char" type.
 *  Please use "packed" very carefully.
 *
 *  We should take care to use "packed"
 *  Make sure that each item in the structure will have the same align.
 *  ======================================================================
 */
#ifdef __GNUC__
    #define MAKE_PACKED(X) X __attribute__((packed))

#else
   #define MAKE_PACKED(X) __packed X
   #define __FUNCTION__ __func__

#endif
/*
Example for packed structure:
------------------------------
typedef MAKE_PACKED( struct Test1_s
{
    unsigned short s1 ;
    unsigned short s2 ;
    unsigned int   i1 ;
    unsigned int   i2 ;
} ) Test1_t  ;
*/
#endif /* _COMMON_DEF_H */
