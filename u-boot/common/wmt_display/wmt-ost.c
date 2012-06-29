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
#include <common.h>
#include "vpp.h"

static WMT_OST_REG *pWMTOST;

#ifndef NULL
#define NULL 0
#endif

int wmt_write_ostc(void)
{
	unsigned int sw_counter = 30000;
	
	while( pWMTOST->ostas & 0x10 ) {
		if ( --sw_counter == 0 ) { // Need to be considered
			printf("Count Write Active Busy\n");
			return -1;
		}
	}
	return 0;
}

int wmt_init_ost(void)
{
	//printf("wmt_init_ostimer\n");

	if (pWMTOST == NULL)
		pWMTOST = (WMT_OST_REG *)0xd8130100;

	if (pWMTOST->ostctrl&0x01)
		return 0;

	pWMTOST->ostctrl = 0;
	pWMTOST->ostwe = 0;
	
	if (wmt_write_ostc())
		return -1;

	pWMTOST->ostct = 0;

	pWMTOST->ostctrl = 1;
	
	return 0;
}

int wmt_read_ostc(int *val)
{
	unsigned int sw_counter = 300000;

	if (pWMTOST == NULL) {
		if (wmt_init_ost())
			return -1;
	}

	if ( (pWMTOST->ostctrl & 0x02 ) == 0 )
		pWMTOST->ostctrl |= 0x02;

    // Check OS Timer Count Value is Valid
	while ( (pWMTOST->ostas & 0x20) == 0x20 ) {
		if ( --sw_counter == 0 ) { // Need to be considered
			printf("Read Count Request Fail\n");
			break;
		}
	}
	//*val = (int)pWMTOST->ostct;   //Charles
	*val = (int)pWMTOST->ostct/3;
	return 0;
}

int wmt_delayus(int us)
{
	int count = 100;
	int before,after;

	//us = us*3;	

	if (wmt_read_ostc(&before))
		return -1;
	while(1) {
		while(--count);
		if (wmt_read_ostc(&after))
			return -1;
		if ((after - before) >= us) {
			//printf("request = %d , result = %d\n",us,(after - before));
			break;
		}
		count = 100;
	}
	return 0;
}

