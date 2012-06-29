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


#ifndef __GISR_H__
#define __GISR_H__

#if !defined(__TTYPE_H__)
#include "ttype.h"
#endif

#if !defined(__ADAPTER_H__)
#include "adapter.h"
#endif

/*---------------------  Export Definitions -------------------------*/

/*---------------------  Export Classes  ----------------------------*/

/*---------------------  Export Variables  --------------------------*/

/*---------------------  Export Functions  --------------------------*/
#ifdef __cplusplus
extern "C" {                            /* Assume C declarations for C++ */
#endif /* __cplusplus */
VOID
PCAvIntEnable(
    void
    );

VOID
PCAvIntDisable(
    void
    );

DWORD
CRCdwGetCrc32(
	PBYTE   pbyData,
	UINT    cbByte
);

/* supports max 6 cards */
void ISRvIsrForNetwork_Card0(void);
void ISRvIsrForNetwork_Card1(void);
void ISRvIsrForNetwork_Card2(void);
void ISRvIsrForNetwork_Card3(void);


/* interrupt service routine for adapter      */
void GISRvIsrForNetwork(PSAdapterInfo pAdapter);


#if defined(__cplusplus)
}                                       /* End of extern "C" { */
#endif /* __cplusplus */

#endif /* __ISR_H__ */
