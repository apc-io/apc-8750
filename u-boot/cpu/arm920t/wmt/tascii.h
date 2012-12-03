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

#ifndef __TASCII_H__
#define __TASCII_H__

/*---------------------  Export Definitions -------------------------*/

/*
 * CONTROL ASCII CODE
 */
#define MC_NULL         0x00            /* null char */
#define MC_SOH          0x01            /* Ctrl+A : */
#define MC_STX          0x02            /* Ctrl+B : */
#define MC_ETX          0x03            /* Ctrl+C : */
#define MC_EOT          0x04            /* Ctrl+D : end of text */
#define MC_ENQ          0x05            /* Ctrl+E : */
#define MC_ACK          0x06            /* Ctrl+F : */

#define MC_BELL         0x07            /* Ctrl+G : bell sound */
#define MC_BACKSPACE    0x08            /* Ctrl+H : backspace */
#define MC_TAB          0x09            /* Ctrl+I : harizontal tab */
#define MC_LINEFEED     0x0A            /* Ctrl+J : line feed */
/*                                                  EOL(end of line for UNIX) */
#define MC_VTAB         0x0B            /* Ctrl+K : vertical tab */
#define MC_FORMFEED     0x0C            /* Ctrl+L : form feed */
#define MC_ENTER        0x0D            /* Ctrl+M : carriage return */

#define MC_SO           0x0E            /* Ctrl+N : */
#define MC_SI           0x0F            /* Ctrl+O : */
#define MC_DLE          0x10            /* Ctrl+P : */

#define MC_XON          0x11            /* Ctrl+Q : device control 1 */
/*                                                  xon */
#define MC_DC2          0x12            /* Ctrl+R : */
#define MC_XOFF         0x13            /* Ctrl+S : device conteol 3 */
/*                                                  xoff */
#define MC_DC4          0x14            /* Ctrl+T : */
#define MC_NAK          0x15            /* Ctrl+U : */
#define MC_SYN          0x16            /* Ctrl+V : */
#define MC_ETB          0x17            /* Ctrl+W : */
#define MC_CAN          0x18            /* Ctrl+X : */
#define MC_EM           0x19            /* Ctrl+Y : */

#define MC_SUB          0x1A            /* Ctrl+Z : */
/*                                                  EOF(end of file for MSDOS) */
#define MC_ESCAPE       0x1B            /* Ctrl+[ : escape */

#define MC_FS           0x1C            /* Ctrl+\ : */
#define MC_GS           0x1D            /* Ctrl+] : */
#define MC_RS           0x1E            /* Ctrl+^ : */
#define MC_US           0x1F            /* Ctrl+_ : */

/*
 * PRINTABLE ASCII CODE
 */
#define MC_SPACE        0x20            /* space char */

#define MC_0            0x30            /* 0 */
#define MC_1            0x31            /* 1 */
#define MC_2            0x32            /* 2 */
#define MC_3            0x33            /* 3 */
#define MC_4            0x34            /* 4 */
#define MC_5            0x35            /* 5 */
#define MC_6            0x36            /* 6 */
#define MC_7            0x37            /* 7 */
#define MC_8            0x38            /* 8 */
#define MC_9            0x39            /* 9 */

#define MC_A            0x41            /* A */
#define MC_B            0x42            /* B */
#define MC_C            0x43            /* C */
#define MC_D            0x44            /* D */
#define MC_E            0x45            /* E */
#define MC_F            0x46            /* F */
#define MC_G            0x47            /* G */
#define MC_H            0x48            /* H */
#define MC_I            0x49            /* I */
#define MC_J            0x4A            /* J */
#define MC_K            0x4B            /* K */
#define MC_L            0x4C            /* L */
#define MC_M            0x4D            /* M */
#define MC_N            0x4E            /* N */
#define MC_O            0x4F            /* O */
#define MC_P            0x50            /* P */
#define MC_Q            0x51            /* Q */
#define MC_R            0x52            /* R */
#define MC_S            0x53            /* S */
#define MC_T            0x54            /* T */
#define MC_U            0x55            /* U */
#define MC_V            0x56            /* V */
#define MC_W            0x57            /* W */
#define MC_X            0x58            /* X */
#define MC_Y            0x59            /* Y */
#define MC_Z            0x5A            /* Z */

#define MC_a            0x61            /* a */
#define MC_b            0x62            /* b */
#define MC_c            0x63            /* c */
#define MC_d            0x64            /* d */
#define MC_e            0x65            /* e */
#define MC_f            0x66            /* f */
#define MC_g            0x67            /* g */
#define MC_h            0x68            /* h */
#define MC_i            0x69            /* i */
#define MC_j            0x6A            /* j */
#define MC_k            0x6B            /* k */
#define MC_l            0x6C            /* l */
#define MC_m            0x6D            /* m */
#define MC_n            0x6E            /* n */
#define MC_o            0x6F            /* o */
#define MC_p            0x70            /* p */
#define MC_q            0x71            /* q */
#define MC_r            0x72            /* r */
#define MC_s            0x73            /* s */
#define MC_t            0x74            /* t */
#define MC_u            0x75            /* u */
#define MC_v            0x76            /* v */
#define MC_w            0x77            /* w */
#define MC_x            0x78            /* x */
#define MC_y            0x79            /* y */
#define MC_z            0x7A            /* z */

/*---------------------  Export Classes  ----------------------------*/

/*---------------------  Export Variables  --------------------------*/

/*---------------------  Export Functions  --------------------------*/

#endif /* __TASCII_H__ */
