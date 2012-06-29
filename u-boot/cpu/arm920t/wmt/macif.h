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


#ifndef _MACIF_H_
#define _MACIF_H_

#include "ttype.h"
/*---------------------  Export Definitions -------------------------*/
#define BA_MAC0                0xd8004000    /* MAC0 Base Address */
#define BA_MAC1                0xd8005000    /* MAC1 Base Address */
#define MEMORY_BUFFER_START    0x01000000    /* Start address of allocated memory buffer */
#define CB_MAX_NET_DEVICE      1             /* max. # of the devices */
#define W_MAX_TIMEOUT          0x0FFFU
#define MACDBG                 0            /* Switch to control debug message (0/1 = Disable/Enable) */
#define IRQ_ETH0               10
#define IRQ_ETH1               17
/* Definition for diagnose test condition */
#define DIAG_AUTO_TEST         0x0001
#define DIAG_NO_WAIT           0x0002
#define DIAG_REPORT            0x0004
#define DIAG_NO_CABLE_TEST     0x0008
#define DIAG_EXT_LOOPBACK      0x0010

/* 2008/11/11 RichardHsu-s */
#define Gmac_PCI_VEE_WRITE      0x58
#define Gmac_PCI_VEE_READ       0x5c
#define Gmac_PCI_VEE_Waddr      0x5a
#define Gmac_PCI_VEE_Raddr      0x5e
#define Gmac_PCI_VEE_Status     0x5f
#define Gmac_Jumper_Strapping3  0x9b
/* 2008/11/11 RichardHsu-e */

#define PCI_Configuration_Space_Offset 0x100
#define VEE                     0x5c
#define VMSTS                   0x7c
#define MaxTimeOut              1000

extern BOOL g_bInit;

/*---------------------  Export Types  ------------------------------*/

/*---------------------  Export Macros ------------------------------*/

/*---------------------  Export Classes  ----------------------------*/

/*---------------------  Export Variables  --------------------------*/

/*---------------------  Export Functions  --------------------------*/
#ifdef __cplusplus
extern "C" {                            /* Assume C declarations for C++ */
#endif /* __cplusplus */

#include <asm/u-boot.h>

int s_i32GmacWriteVee(unsigned short i16BDF, unsigned char i8Addr, unsigned short i16Data);
int s_i32GmacReadVee(unsigned short i16BDF, unsigned char i8Addr, unsigned short *pi16Data);

/*
int mac_startio(bd_t* bd  );
int mac_send(volatile void *packet, int length);
int mac_receive(void);
void mac_halt(void);
void mii_read(char *devname, unsigned char addr,unsigned char reg, unsigned short * value);
void mii_write(char *devname, unsigned char addr,unsigned char reg, unsigned short value)	;
*/
#ifdef __cplusplus
}                                       /* End of extern "C" { */
#endif /* __cplusplus */

#endif  /* _MACIF_H_ */
