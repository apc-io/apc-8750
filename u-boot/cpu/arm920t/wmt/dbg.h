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

#ifndef __DBG_H__
#define __DBG_H__


/*---------------------  Export Definitions -------------------------*/
#define MACDBG 1
#if MACDBG
#define MACDBG_INFO    ((ULONG)0x00000001)
#define MACDBG_ISR     ((ULONG)0x00000002)
#define MACDBG_MAC     ((ULONG)0x00000004)
#define MACDBG_SROM    ((ULONG)0x00000008)
#define MACDBG_MIB     ((ULONG)0x00000010)

extern ULONG GMacDebugLevel;
#define MacDump(LEVEL, STRING) 				\
	do {                           			\
		if (GMacDebugLevel & (LEVEL)) {		\
			printf STRING;                 	\
		}                              		\
	} while (0)
#else
#define MacDump(LEVEL, STRING) do { ; } while (0)
#endif

/*---------------------  Export Classes  ----------------------------*/

/*---------------------  Export Variables  --------------------------*/

/*---------------------  Export Functions  --------------------------*/

#endif  /* __DBG_H__ */

