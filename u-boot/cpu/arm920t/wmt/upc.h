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


#ifndef __UPC_H__
#define __UPC_H__

#if !defined(__TTYPE_H__)
#include "ttype.h"
#endif

/*---------------------  Export Definitions -------------------------*/
#define	INTERRUPT	void
typedef INTERRUPT(*PISR)(void);
/*---------------------  Export Classes  ----------------------------*/

/*---------------------  Export Variables  --------------------------*/

/*---------------------  Export Functions  --------------------------*/
#if defined(__cplusplus)
extern "C" {                            /* Assume C declarations for C++ */
#endif /* __cplusplus */

/* WMT Device do Memory-Mapped IO when in 32-bit environment */
#define VNSvInPortB(dwIOAddress, pbyData)		\
do {                                     		\
	volatile BYTE *pbyAddr = ((PBYTE)(dwIOAddress));  	\
	*(pbyData) = *pbyAddr;                   	\
} while (0)

#define VNSvInPortW(dwIOAddress, pwData)		\
do {                                    		\
	volatile WORD *pwAddr = ((PWORD)(dwIOAddress));  	\
	*(pwData) = *pwAddr;                    	\
} while (0)

#define VNSvInPortD(dwIOAddress, pdwData)		\
do {                                     		\
	volatile DWORD *pdwAddr = ((PDWORD)(dwIOAddress));	\
	*(pdwData) = *pdwAddr;                   	\
} while (0)

#define VNSvOutPortB(dwIOAddress, byData)		\
do {                                     		\
	volatile BYTE *pbyTmp = (PBYTE)(dwIOAddress);     	\
	*pbyTmp = (byData);                      	\
} while (0)

#define VNSvOutPortW(dwIOAddress, wData)		\
do {                                    		\
	volatile WORD *pwTmp = (PWORD)(dwIOAddress);     	\
	*pwTmp = (wData);                       	\
} while (0)

#define VNSvOutPortD(dwIOAddress, dwData)		\
do {                                     		\
	volatile DWORD *pdwTmp = (PDWORD)(dwIOAddress);   	\
	*pdwTmp = (dwData);                      	\
} while (0)

#define PCAvDelayByIO(wDelayUnit)    			\
do {                                 			\
	BYTE    byTemp;                      		\
	ULONG   ii;                          		\
	for (ii = 0; ii < (wDelayUnit); ii++)		\
		VNSvInPortB(0x0, &byTemp);           	\
} while (0)

#define PCBvInPortB VNSvInPortB
#define PCBvInPortW VNSvInPortW
#define PCBvInPortD VNSvInPortD
#define PCBvOutPortB VNSvOutPortB
#define PCBvOutPortW VNSvOutPortW
#define PCBvOutPortD VNSvOutPortD

#if defined(__cplusplus)
}                                       /* End of extern "C" { */
#endif /* __cplusplus */

#endif /* __UPC_H__ */
