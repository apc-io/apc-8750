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

#include <common.h>
#include <asm/io.h>
#include <net.h>
#include <miiphy.h>
#include "gmacif.h"
#include <asm/u-boot.h>
#include <configs/wmt.h>

#ifdef CONFIG_DRIVER_ETHER

#if (CONFIG_COMMANDS & CFG_CMD_NET)

/******************************************************************************
 *
 * Public u-boot interface functions below
 *
 *****************************************************************************/
int eth_init(bd_t *bd)
{
#if MACDBG
	printf("eth_init\n");
#endif
	return gmac_startio(bd);
 }

int eth_send(volatile void *packet, int length)
{
#if MACDBG
	printf("eth_send\n");
#endif
	return gmac_send(packet, length);
}

int eth_rx(void)
{
#if MACDBG
	printf("eth_rx\n");
#endif
	return gmac_receive();
}

void eth_halt(void)
{
#if MACDBG
	printf("eth_halt\n");
#endif
	gmac_halt();
	return;
}

#if defined(CONFIG_MII) || (CONFIG_COMMANDS & CFG_CMD_MII)
int  wmt_miiphy_read(char *devname, unsigned char addr,
		unsigned char reg, unsigned short *value)
{
	mii_read(devname, addr, reg, value);
	return 0;
}

int  wmt_miiphy_write(char *devname, unsigned char addr,
		unsigned char reg, unsigned short value)
{
	mii_write(devname, addr, reg, value);
	return 0;
}

#endif	/* defined(CONFIG_MII) || (CONFIG_COMMANDS & CFG_CMD_MII) */

int wmt_miiphy_initialize(bd_t *bis)
{
#if defined(CONFIG_MII) || (CONFIG_COMMANDS & CFG_CMD_MII)
	/* miiphy_register("wmtphy", wmt_miiphy_read, wmt_miiphy_write); */
#endif
	return 0;
}

#endif	/* CONFIG_COMMANDS & CFG_CMD_NET */

#endif	/* CONFIG_DRIVER_ETHER */
