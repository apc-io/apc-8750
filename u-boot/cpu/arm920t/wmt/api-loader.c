/*
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * Copyright (c) 2010 WonderMedia Technologies, Inc.
 *
 * This program is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software Foundation,
 * either version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * WonderMedia Technologies, Inc.
 * 10F, 529, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C.
 */

/*
 * api-loader.c - this is entry support for the WMT evaluation board to go to loader
 */
 
//#include <common.h>
#include <api_loader.h>

#define LOADER_ADDR												0xffff0000
#define HIBERNATION_ENTER_EXIT_CODE_BASE_ADDR	0xFFFFFFC0
#define DO_DVO_PLL_SET							      (HIBERNATION_ENTER_EXIT_CODE_BASE_ADDR + 0x38)
//#define wmt_debug
#ifdef wmt_debug
#define WMprintf(fmt, args...) printk("[%s]: " fmt, __FUNCTION__ , ## args)
#else 
#define WMprintf(fmt, args...)
#endif

int dvo_pll_set (unsigned int multi, unsigned char divisor, unsigned short resx, unsigned short resy)
{
	volatile unsigned int base = 0;
	unsigned int exec_at = (unsigned int)-1;
	int (*theKernel_dvo)(int from, unsigned int multi, unsigned char divisor, unsigned short resx, unsigned short resy);
	int retval = 0;

	WMprintf("entry 0x%8.8X 0x%8.8X %d %d\n",multi, divisor, resx, resy);
	/*enble SF clock*/
	/*REG32_VAL(PMCEU_ADDR) |= 0x00800000;*/

	/*jump to loader api to do something*/
/*	base = (unsigned int)ioremap_nocache(LOADER_ADDR, 0x10000);*/
	base = (unsigned int)LOADER_ADDR;
	exec_at = base + (DO_DVO_PLL_SET - LOADER_ADDR);
	theKernel_dvo = (int (*)(int from, unsigned int multi, unsigned char divisor, unsigned short resx, unsigned short resy))exec_at;
	
	retval = theKernel_dvo(1, multi, divisor, resx, resy);

/*	iounmap((void *)base);*/

	/*disable SF clock*/
/*	REG32_VAL(PMCEU_ADDR) &= ~0x00800000;*/

	WMprintf("exit!!(%d)\n",retval);

	return retval;
}

