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


#include <config.h>
#include <common.h>
#include <command.h>
#include <version.h>
#include <stdarg.h>
#include <linux/types.h>
#include <devices.h>
#include <linux/stddef.h>
#include <asm/byteorder.h>

#include "wmt_display.h"
#include "com-vpp.h"

void excute_reg_op(char * p)
{
    ulong   addr;
    ulong   val, org;
    char    op;
    char * endp;

    while(1)
    {
        addr = simple_strtoul(p, &endp, 16);
        if( *endp == '\0')
            break;

        op = *endp;
        if( endp[1] == '~')
        {
            val = simple_strtoul(endp+2, &endp, 16);
            val = ~val;
        }
        else
        {
            val = simple_strtoul(endp+1, &endp, 16);
        }
		if (addr&0x03) {
        	printf(" address not alginment to 32bit , address = 0x%x\n",addr);
        	goto nextcheck;
    	}		
        printf("  reg op: 0x%X %c 0x%X\n", addr, op, val);
		org = REG32_VAL(addr);
        switch(op)
        {
            case '|': org |= val; break;
            case '=': org = val ; break;
            case '&': org &= val; break;
            default:
                printf("Error, Unknown operator %c\n", op);
                break;
        }
        REG32_VAL(addr) = org;
nextcheck:
        if(*endp == '\0')
            break;
        p = endp + 1;
    }
    return;
}


static int wmt_do_mbit (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	switch (argc) {
		case 0:
		case 1:	
			printf ("Usage:\n%s\n", cmdtp->usage);
		return 0;
		default:
			if (argc == 2) {
				excute_reg_op(argv[1]);
				return 0;
			}
	
			printf ("Usage:\n%s\n", cmdtp->usage);
			return 0;
	}
	return 0;
}

U_BOOT_CMD(
	mbit,	2,	1,	wmt_do_mbit,
	"memory bit operation : \n"
	"Format : mbit <parameter>\n",
	"Puepose : write bit to memory\n"
	"Example : mbit -int D8000012|~1\n"
	"          VAL32(0xD8000012) | 0xFFFFFFFE"

);
