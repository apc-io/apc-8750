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


#ifndef _GMACIF_H_
#define _GMACIF_H_


/*---------------------  Export Definitions -------------------------*/
#define BA_MAC0                   0xd8004000  /* MAC0 Base Address */
#define BA_MAC1                   0xd8005000  /* MAC1 Base Address */
#define GMEMORY_BUFFER_START      0x03C00000  /* Start address of allocated memory buffer */
#define MAX_NET_DEVICE            4           /* max. # of the devices */
#define W_MAX_TIMEOUT             0x0FFFU
#define MACDBG                    0           /* Switch to control debug message (0/1 = Disable/Enable) */

#define PCI_Configuration_Space_Offset 0x100
#define VEE                     0x5c
#define VMSTS                   0x7c
#define MaxTimeOut              1000

#define __MMIO__
/*---------------------  Export Types  ------------------------------*/

/*---------------------  Export Macros ------------------------------*/

/*---------------------  Export Classes  ----------------------------*/

/*---------------------  Export Variables  --------------------------*/

/*---------------------  Export Functions  --------------------------*/
#ifdef __cplusplus
extern "C" {                            /* Assume C declarations for C++ */
#endif /* __cplusplus */

#include <asm/u-boot.h>

int gmac_startio(bd_t *bd);
int gmac_send(volatile void *packet, int length);
int gmac_receive(void);
void gmac_halt(void);
void mii_read(char *devname, unsigned char addr, unsigned char reg, unsigned short *value);
void mii_write(char *devname, unsigned char addr, unsigned char reg, unsigned short value)	;

#ifdef __cplusplus
}                                       /* End of extern "C" { */
#endif /* __cplusplus */

#endif  /* _GMACIF_H_ */
