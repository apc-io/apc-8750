/*
 * (C) Copyright 2003
 * Kyle Harris, kharris@nexus-tech.net
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>
#include <config.h>
#include <asm/errno.h>
#include <asm/arch/hardware.h>
#include <part.h>
#include <asm-arm/arch-wmt/common_def.h>

#if (CONFIG_COMMANDS & CFG_CMD_MMC)

#include <dma.h>

struct DMA_REG   *pDma_Reg = (struct DMA_REG *)BA_DMA;

#define Source1 0x6a10000
#define Source2 0x3b10000
#define Source3 0x3c10000
#define Source4 0x3d10000
#define Source5 0x3e10000
#define Source6 0x3f10000
#define Source7 0x4010000
#define Source8 0x4110000
#define Source9 0x4210000
#define Source10 0x4310000
#define Source11 0x4410000
#define Source12 0x4510000
#define Source13 0x4610000
#define Source14 0x4710000
#define Source15 0x4810000
#define Source16 0x4910000
#define Destination1 0x6b10000
#define Destination2 0x5b10000
#define Destination3 0x5c10000
#define Destination4 0x5d10000
#define Destination5 0x5e10000
#define Destination6 0x5f10000
#define Destination7 0x6010000
#define Destination8 0x6110000
#define Destination9 0x6210000
#define Destination10 0x6310000
#define Destination11 0x6410000
#define Destination12 0x6510000
#define Destination13 0x6610000
#define Destination14 0x6710000
#define Destination15 0x6810000
#define Destination16 0x6910000

int flag0;
static	ulong	base_address = 0;

extern int cmd_get_data_size(char *arg, int default_size);

void free_dma_channels(void)
{
	unsigned int ch = 0;
	for (ch = 0; ch < MAX_DMA_CHANNELS; ++ch)
		free_dma(ch);
}






int do_dma_cp ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	ulong	addr, dest, count , tempaddr;
	int	size;
	unsigned int ChannelNo;
	


	struct dma_device_cfg configuration = {
		MEMORY_DMA_REQ,
		(DMA_WRAP_1|DMA_BURST_8|DMA_SIZE_32|DMA_SG_MODE|DMA_SW_REQ),
		Source1,
		Destination1,
	} ;

	
	if (argc != 4) {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	/* Check for size specification.
	*/
	if ((size = cmd_get_data_size(argv[0], 4)) < 0)
		return 1;

	addr = simple_strtoul(argv[1], NULL, 16);
	addr += base_address;

	dest = simple_strtoul(argv[2], NULL, 16);
	dest += base_address;

	count = simple_strtoul(argv[3], NULL, 16);

	if (count == 0) {
		puts ("Zero length ???\n");
		return 1;
	}

	count *= size;
	
#ifndef CFG_NO_FLASH
	/* check if we are copying to Flash */
	if ( (addr2info(dest) != NULL)
#ifdef CONFIG_HAS_DATAFLASH
	   && (!addr_dataflash(addr))
#endif
	   ) {
		int rc;

		puts ("Copy to Flash... ");

		rc = flash_write ((char *)addr, dest, count);
		if (rc != 0) {
			flash_perror (rc);
			return (1);
		}
		puts ("done\n");
		return 0;
	}
#endif

	init_dma();
	if(addr >= 0xFE000000){
	    *(volatile unsigned int *)(0xD8330000) = 0x33013301;
    	*(volatile unsigned int *)(0xD8330008) = 0x10004;
	    *(volatile unsigned int *)(0xD8330010) = 0x10004;
    	*(volatile unsigned int *)(0xD8330020) = 0x809;
	    *(volatile unsigned int *)(0xD8330028) = 0x809;	
	    request_dma(&ChannelNo, "dmacp", I2S_TX_DMA_REQ);
	}
	else
    	request_dma(&ChannelNo, "dmacp", MEMORY_DMA_REQ);
    if(addr >= 0xFE000000){
        configuration.DeviceReqType = I2S_TX_DMA_REQ;
		configuration.DefaultCCR = (DMA_WRAP_1|DMA_BURST_8|DMA_SIZE_32|DMA_SG_MODE|DMA_UP_MEMREG_EN|DEVICE_TO_MEM);
		tempaddr = addr;
		addr = dest;
		dest = tempaddr;
    }
    init_descriptstack(ChannelNo);
	setup_dma(ChannelNo, configuration);
	
	//printf("ISR ch%d = %x\n", ChannelNo, pDma_Reg->DMA_ISR);
	//printf("IER ch%d = %x\n", ChannelNo, pDma_Reg->DMA_IER);
	//printf("CCR ch%d = %x\n", ChannelNo, pDma_Reg->DMA_CCR_CH[ChannelNo]);

	//{
	//    start_dma(ChannelNo, (unsigned long)addr, (unsigned long)dest, count);
    //    printf("DMA%d : handle irq begin\n", ChannelNo);
    //    handle_dma_irq(ChannelNo);
    //    printf("DMA%d : handle irq OK\n", ChannelNo);
    /*******************************************
	* wait for dma transfer complete and terminal count
	********************************************/
	//    while (1) {
	//	    if (dma_busy(ChannelNo) != 1)
	//		    break;
    //	}
	//    printf("DMA%d : no busy\n", ChannelNo);
    //	while (1) {
	//    	if (dma_complete(ChannelNo) == 0)
	//	    	break;
    //	}
	//}
    handle_transfer(ChannelNo, (unsigned long)addr, (unsigned long)dest, count);
    reset_descriptstack(ChannelNo);
    printf("DMA%d : transfer OK\n", ChannelNo);
	return 0;
}




U_BOOT_CMD(
	dmacp,    4,    1,    do_dma_cp,
	"dmacp     - dma memory copy\n",
	"[.b, .w, .l] source target count\n    - copy memory\n"
);


#endif	/* CFG_CMD_MMC */

