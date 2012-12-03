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
4F, 531, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C.
--*/

#ifndef WMT_OST_H
/* To assert that only one occurrence is included */
#define WMT_OST_H

typedef struct
{
	volatile unsigned long	ostm[4];				// [Rx100-Rx10C] OS Timer Match Register0-3
	volatile unsigned long	ostct;					// [Rx110-113] OS Timer Counter Register
	volatile unsigned long	osts;					// [Rx114-117] OS Timer Status Register
	volatile unsigned long	ostwe;					// [Rx118-Rx11B]
	volatile unsigned long	ostie;					// [Rx11C-Rx11F]
	volatile unsigned long	ostctrl;				// [Rx120-Rx123] OS Timer Control Register
	volatile unsigned long	ostas;					// [Rx124-Rx127] OS Timer Access Status Register
} WMT_OST_REG;

int wmt_delayus(int us);
int wmt_read_ostc(int *val);

#endif
/*=== END wmt-ost.h ==========================================================*/
