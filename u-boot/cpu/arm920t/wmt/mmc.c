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
/*
 * (C) Copyright 2003
 * Kyle Harris, Nexus Technologies, Inc. kharris@nexus-tech.net
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

#include <config.h>
#include <common.h>
#include <malloc.h>
#include <mmc.h>
#include <asm/errno.h>
#include <asm/arch/hardware.h>
#include <part.h>
#include "ttype.h"
#include "./sdc.h"
#include <asm-arm/arch-wmt/common_def.h>
#include "../../../board/wmt/include/wmt_clk.h"
//#define CONFIG_MMC

#ifdef CONFIG_MMC
/***********************************************************************************/
#define BA_SDC_1 			0xD800A400	// SD Controller Base Address
#define BA_SDCDMA_1 		0xD800A500	// SD Controller DMA Base Address
#define BA_SDC 				0xD800A000	// SD Controller Base Address
#define BA_SDCDMA 			0xD800A100	// SD Controller DMA Base Address
#define BA_SDC_2 			0xD800A800	// SD Controller Base Address
#define BA_SDCDMA_2 		0xD800A900	// SD Controller DMA Base Address

#define SDMMC_SET_BLKSIZE(pHC, b)  {pHC->blklen &= 0xF800; pHC->blklen |= ((b) & 0x7FF); }

// PMC module reg
#define PMC_BASE_ADDR 				0XD8130000
#define PMC_SD1_CLK					REG32_PTR(PMC_BASE_ADDR + 0x324)
#define PMC_SD_CLK  				REG32_PTR(PMC_BASE_ADDR + 0x328 )
#define PMC_Enable_SD_CLK  			REG32_PTR(PMC_BASE_ADDR + 0x254 )
#define PMC_PLLB_MULTIPLIER  		REG32_PTR(PMC_BASE_ADDR + 0x204 )
#define PMC_SD_CLOCK_EN         	0x40000

// GPIO module reg
#define GPIO_BASE_ADDR 		0xD8110000
#define GPIO_SD_ENABLE    	REG32_PTR( GPIO_BASE_ADDR + 0x40 )


PWMT_SDMMC_REG 	pSd_Reg = (PWMT_SDMMC_REG) BA_SDC;
struct _SD_PDMA_REG_ *pSd_PDma_Reg = (struct _SD_PDMA_REG_ *) BA_SDCDMA;
PGPIO_REG 	GpioReg = (PGPIO_REG) GPIO_BASE_ADDR;
//static unsigned long RCA;
sd_info_t *SDDevInfo;
sd_info_t SD0DevInfo;
sd_info_t SD1DevInfo;
sd_info_t SD2DevInfo;

static unsigned int Bitmode ;
static unsigned short chip_id;
static block_dev_desc_t *mmc_dev;
static block_dev_desc_t mmc0_dev;
static block_dev_desc_t mmc1_dev;
static block_dev_desc_t mmc2_dev;

static int mmc_ready = 0;
static int mmc0_ready = 0;
static int mmc1_ready = 0;
static int mmc2_ready = 0;


#define SDPRINTK(x...)       do { } while (0)//printf(x)
extern int auto_pll_divisor(enum dev_id dev, enum clk_cmd cmd, int unit, int freq);

/**********************************************************************************/
int sd_command(unsigned char command, unsigned char cmdtype, unsigned int  arg, unsigned char rsptype);
int sd_config_pdma(unsigned long *DescAddr, char dir);
unsigned int get_chip_version(void);

void Change_SD_host(int host_num)
{
	switch (host_num) {
	case 0:
		pSd_Reg = (PWMT_SDMMC_REG) BA_SDC;
		pSd_PDma_Reg = (struct _SD_PDMA_REG_ *) BA_SDCDMA;
		SDDevInfo = &SD0DevInfo;
		mmc_dev = &mmc0_dev;
		mmc_ready = mmc0_ready;
		break;
	case 1:
		if (chip_id == 0x3437 || 
            chip_id == 0x3429 || 
            chip_id == 0x3451 || 
            chip_id == 0x3445) {
            
			pSd_Reg = (PWMT_SDMMC_REG) BA_SDC_1;
			pSd_PDma_Reg = (struct _SD_PDMA_REG_ *) BA_SDCDMA_1;
			SDDevInfo = &SD1DevInfo;
			mmc_dev = &mmc1_dev;
			mmc_ready = mmc1_ready;
		}
		break;
	case 2:
		if (chip_id == 0x3445) {
			pSd_Reg = (PWMT_SDMMC_REG) BA_SDC_2;
			pSd_PDma_Reg = (struct _SD_PDMA_REG_ *) BA_SDCDMA_2;
			SDDevInfo = &SD2DevInfo;
			mmc_dev = &mmc2_dev;
			mmc_ready = mmc2_ready;
		}
		break;
	default:
		break;
	}
}
int SD_Init_PDMA(void)
{
	pSd_PDma_Reg->DMA_GCR = SD_PDMA_GCR_SOFTRESET;
	pSd_PDma_Reg->DMA_GCR = SD_PDMA_GCR_DMA_EN;
	if (pSd_PDma_Reg->DMA_GCR & SD_PDMA_GCR_DMA_EN)
		return 0;
	else
		return 1;
}

int sd_init_long_desc(unsigned long *DescAddr, unsigned int ReqCount, unsigned long *BufferAddr, unsigned long *BranchAddr, int End)
{
	struct _SD_PDMA_DESC_L *CurDes_L;
	CurDes_L = (struct _SD_PDMA_DESC_L *) DescAddr;
	//Bug, Fix by leoli @2009.4
	CurDes_L->ReqCount = (u16) ReqCount;
	CurDes_L->i = 0;
	CurDes_L->format = 1;
	CurDes_L->DataBufferAddr = BufferAddr;
	CurDes_L->BranchAddr = BranchAddr;
	if (End) {
		CurDes_L->end = 1;
		CurDes_L->i = 1;
	}

	return 0;
}

int sd_pdma_handler(void)
{
	unsigned long status = SD_PDMA_CCR_Evt_no_status;
	long count = 0;
/*	SDPRINTK("Before polling DMA status:\nSTS[0-3]: 0x%x  0x%x 0x%x 0x%x \n",pSd_Reg->str0,
		pSd_Reg->str1, pSd_Reg->str2, pSd_Reg->str3);*/
	count = 0x10000000;
	/*	 polling CSR TC status	*/
	
	do {
		count--;
		if (pSd_PDma_Reg->DMA_ISR & SD_PDMA_IER_INT_STS) {
			//SDPRINTK("pSd_PDma_Reg->DMA_CCR = 0x%lx!\n", pSd_PDma_Reg->DMA_CCR );
			status = pSd_PDma_Reg->DMA_CCR & SD_PDMA_CCR_EvtCode;
			pSd_PDma_Reg->DMA_ISR |= SD_PDMA_IER_INT_STS;
			break ;
		}
		if (count == 0) {
			SDPRINTK("SDPDMA Time Out!\n");
			break;
		}
	} while (1);
	
	//SDPRINTK("\nPolling CSR TC status: 0x%lx!\n",status);
	if (status == SD_PDMA_CCR_Evt_ff_underrun)
		SDPRINTK("PDMA Buffer under run!\n");

	if (status == SD_PDMA_CCR_Evt_ff_overrun)
		SDPRINTK("PDMA Buffer over run!\n");

	if (status == SD_PDMA_CCR_Evt_desp_read)
		SDPRINTK("PDMA read Descriptor error!\n");

	if (status == SD_PDMA_CCR_Evt_data_rw)
		SDPRINTK("PDMA read/write memory error!\n");

	if (status == SD_PDMA_CCR_Evt_early_end)
		SDPRINTK("PDMA read early end!\n");

	if (count == 0)	{
		SDPRINTK("SDPDMA TimeOut!\n");
		return -1;
	}

	count = 0x10000000;
	/* check write direction */
	// Leo fix bug here, never go into the branch.
	if ((pSd_PDma_Reg->DMA_CCR & SD_PDMA_CCR_peripheral_to_IF) == SD_PDMA_CCR_IF_to_peripheral) {
		if (pSd_Reg->cmdi == WRITE_MULTIPLE_BLOCK) {
			while (!(pSd_Reg->str0 & 0x10)) {
			count--;
				if (count == 0) {
					printf("SD Control Data TimeOut!\n");
					return -1;
				}
			}
		}
		while (!(pSd_Reg->str0 & 0x30)) {
			count--;
			if (count == 0) {
				printf("SD Control Data TimeOut!\n");
				return -1;
			}
		}
	}
	//printf("SD handler count = %x\n",count);
	return 0 ;
}

void sdmmcSendExCSD(unsigned long *ahb_mem_data_in)
{
	unsigned char Resp1[4], tmp;
	struct _SD_PDMA_DESC_L ReadDesc;
	//bzero(ahb_mem_data_in, 512);
	memset(ahb_mem_data_in, 0x0, 512);
	pSd_Reg->blkcnt= 0x01 ;

	/*	 set normal speed bus mode	*/
	pSd_Reg->extctl &= 0x7F ;
	
	/*	 set time-out value	*/
	pSd_Reg->rtor = 0x1F ;
	pSd_Reg->ctlr |= 0x08 ;   /*	 response FIFO reset	*/

	pSd_Reg->str0 |= 0xFF;
	pSd_Reg->str1 |= 0xFF;
	
	SD_Init_PDMA();
	sd_init_long_desc(
		(unsigned long*)(&ReadDesc),
		512,
		ahb_mem_data_in,
		(unsigned long *)(&ReadDesc + sizeof(struct _SD_PDMA_DESC_L)),
		1
	);
	sd_config_pdma((unsigned long*)(&ReadDesc), SD_PDMA_READ);

	if (sd_command(SEND_EXT_CSD, 2, 0x0, R1) != 0) {
		SDPRINTK("\nErr : send extend csd! \n");
		return;
	}

    tmp = pSd_Reg->resr[0];
	Resp1[0] =  pSd_Reg->resr[1];
	Resp1[1] =  pSd_Reg->resr[2];
	Resp1[2] =  pSd_Reg->resr[3];
	Resp1[3] =  pSd_Reg->resr[4];
	/*
	SDPRINTK("SWT response[31:24] =%x \n\r",Resp1[0]);
	SDPRINTK("SWT response[23:16] =%x \n\r",Resp1[1]);
	SDPRINTK("SWT response[15:8] =%x \n\r", Resp1[2]);
	SDPRINTK("SWT response[7:0]  =%x \n\r", Resp1[3]);
	*/
	/*	while (ost_delay_ms(200) != OST_NO_ERROR);	*/

	sd_pdma_handler();

	/*	*IMR0_REG = 0x08;						// Clark - RX buffer half full intr enable	*/

	
	return;

}

int sd_free_pdma(void)
{
	pSd_PDma_Reg->DMA_DESPR	= 0;
	pSd_PDma_Reg->DMA_GCR = 0;
	//SDPRINTK("sd_free_pdma okay!\n");
	return 0;
}

int sdmmcSwitch(void)
{
	unsigned char *SCR ;
	unsigned char tmp ;
	unsigned long *ahb_mem_data_in = (unsigned long *)malloc(64);
	struct _SD_PDMA_DESC_L ReadDesc;
	memset(ahb_mem_data_in, 0x0, 64);
	//bzero(ahb_mem_data_in, 64);
	
	SDDevInfo->HighSpeedSupport = SD_FALSE;
    SDDevInfo->MMCMaxClockRate = SD_Clk_25MHz;

	pSd_Reg->blklen = 0x003F;
	pSd_Reg->blkcnt = 0x00;
	pSd_Reg->ctlr |= 0x08 ;


	/*	 set normal speed bus mode	*/
	pSd_Reg->extctl &= 0x7F ;

	pSd_Reg->str0 |= 0xFF;
	pSd_Reg->str1 |= 0xFF;

	/*	Config Read DMA	*/
	SD_Init_PDMA();
	sd_init_long_desc(
		(unsigned long *)(&ReadDesc),
		64,
		ahb_mem_data_in,
		(unsigned long *)(&ReadDesc + sizeof(struct _SD_PDMA_DESC_L)),
		1
	);
	sd_config_pdma((unsigned long *)(&ReadDesc), SD_PDMA_READ);

	/*	(1)SD_SWITCH:Check function
	 *     check Card support High speed mode
	 */
	if (sd_command(0x6, 2, 0x00FFFF01, R1) != 0) {
		SDPRINTK("\nErr : send switch cmd! \n");
		return -1;
	}

	sd_pdma_handler();

	SCR = (unsigned char *)(ahb_mem_data_in);
	tmp = *(SCR+16) ;
	if ((tmp & 0x0F) == 0x01) {
		SDDevInfo->HighSpeedSupport = SD_TRUE;
        SDDevInfo->MMCMaxClockRate = SD_Clk_44MHz;
    }

	/*	while (ost_delay_ms(100) != OST_NO_ERROR);	*/

	if (SDDevInfo->HighSpeedSupport == SD_TRUE) {
		pSd_Reg->ctlr |= 0x08 ;

		pSd_Reg->str0 |= 0xFF;
		pSd_Reg->str1 |= 0xFF;
		
//		sd_alloc_desc_pool(ReadDesc);
		sd_init_long_desc(
			(unsigned long *)(&ReadDesc),
			64,
			ahb_mem_data_in,
			(unsigned long *)(&ReadDesc + sizeof(struct _SD_PDMA_DESC_L)),
			1
		);
		sd_config_pdma((unsigned long *)(&ReadDesc), SD_PDMA_READ);
		
		/*	(1)SD_SWITCH:SET function
		 *     switch card to High speend mode
		 */
		if (sd_command(0x6, 2, 0x80FFFF01, R1) != 0) {
			SDPRINTK("\nErr : send switch cmd! \n");
			return -1;
		}

		sd_pdma_handler();

		/*	free dma	*/
		sd_free_pdma();

		SCR = (unsigned char *)(ahb_mem_data_in) ;
		tmp = *(SCR+16) ;

		if ((tmp & 0x0F) == 0x0F) {
			SDDevInfo->HighSpeedSupport = SD_FALSE;
            SDDevInfo->MMCMaxClockRate = SD_Clk_25MHz;
			SDPRINTK("SD Card NormSpeed!\n") ;
			return 0 ;
		} else{
			SDPRINTK("SD Card High Speed!\n") ;
		}

	}

	return 0 ;
}

int sd_send_scr(void)
{

  unsigned long sd_status;
  unsigned char rb_data ;


	sd_status = sd_command( SD_SEND_SCR ,2 , 0 ,R1 );
	if (sd_status != 0 )
  		return -1 ;

	while ((pSd_Reg->str0 & BLK_DONE) != BLK_DONE) {
		if (pSd_Reg->str1 & DATA_TIMEOUT) {
			rb_data = pSd_Reg->str1 ;
			rb_data = pSd_Reg->str1 ;
			rb_data = pSd_Reg->str1 ;
			return -1 ;
		}
	}

	rb_data = pSd_Reg->str0 ;
	rb_data = pSd_Reg->str0 ;
	rb_data = pSd_Reg->str0 ;

	return 0 ;
}

void card_nop(void)
{
  int i = 2000 ;
  do{
     i--;
  }while( i!=0 ) ;
}

//------------------------------------------------------
//send SD command (non-data related command) to SD card
//
//arg depends on command type
//
//-----------------------------------------------------
int sd_command(
	unsigned char command,
	unsigned char cmdtype,
	unsigned int  arg,
	unsigned char rsptype
	)
{

	unsigned char rb_data ;


	/* set command*/
	pSd_Reg->cmdi = command ;
	/* set command argument*/
	pSd_Reg->cmdarg0 = (BYTE)arg;
	pSd_Reg->cmdarg1 = (BYTE)(arg >> 8);/*move 8 bits*/
	pSd_Reg->cmdarg2 = (BYTE)(arg>> 16);/*move 16 bits*/
	pSd_Reg->cmdarg3 = (BYTE)(arg >> 24);/*move 24 bits*/
	pSd_Reg->respt = rsptype;
  
    /*clear all status registers*/
    pSd_Reg->str0 &= 0xff;
    pSd_Reg->str1 &= 0xff;
    pSd_Reg->str2 &= 0xff;
    pSd_Reg->str3 &= 0xff;
    
    /*Choose SD mode*/
    if (chip_id == 0x3426) {
    	pSd_Reg->busm &= 0xEF;
    } else {
    	pSd_Reg->busm |= 0x10;
    } 
    	
    pSd_Reg->ctlr &= 0x0F;
    pSd_Reg->ctlr |= (cmdtype << 4);
  
  
    /*send out the command*/
    pSd_Reg->ctlr |= CMD_START;
    rb_data = pSd_Reg->ctlr ;
	
    /*wait for Ctrl[busy] clear*/
    while((pSd_Reg->ctlr & 0x1) != 0)
		;
    /*wait for command completion, STS1[cmd-rsp-done]*/
    while((pSd_Reg->str1 & 0x4) != 0x4) {
        if((pSd_Reg->str1 & 0x2) == 0x2) {
			rb_data = pSd_Reg->str1 ;
			rb_data = pSd_Reg->str1 ;
			rb_data = pSd_Reg->ctlr ;
			break ;
        }
    }
  
    /*SD status : command-response time-out*/
    if((pSd_Reg->str1 & 0x4) == 0x4) {
        rb_data = pSd_Reg->str1 ;
        rb_data = pSd_Reg->str1 ;
        rb_data = pSd_Reg->ctlr ;
        return  -1 ;
    }

    return 0 ;
}

int sd_app_command(
	unsigned char command,
 	unsigned char cmdtype,
 	unsigned int  arg,
 	unsigned char rsptype
 	)
{
  unsigned long  sd_status;
  
  if(command == SD_SEND_SCR) {
    /*change blk length to 8 (SCR register is 8byte)*/
    pSd_Reg->blklen  = 0x7;
    pSd_Reg->blkcnt  = 0x00 ;
  }

  sd_status = sd_command(APP_CMD, 0, SDDevInfo->RCA, R1) ;

  /*while( ost_delay_ms(200) != OST_NO_ERROR );*/
  if(sd_status != 0) /*not successfully on CMD55*/
    return -1 ;
  /*while( ost_delay_ms(1000) != OST_NO_ERROR );*/

  if(command == SD_SEND_SCR) {
    sd_status = sd_send_scr() ;

    if(sd_status != 0)
      return -1 ;
  } else {
    sd_status = sd_command(command, cmdtype, arg, rsptype) ;

    if(sd_status != 0)
      return -1 ;
  }

  return 0 ;
}

int sd_alloc_desc_pool(unsigned long *DescAddr)
{
	//bzero(DescAddr, 0x1000);
	return 0;
}

int sd_init_short_desc(unsigned long *DescAddr, unsigned int ReqCount, unsigned long *BufferAddr)
{
	struct _SD_PDMA_DESC_S *CurDes_S;
	CurDes_S = (struct _SD_PDMA_DESC_S *) DescAddr;
	CurDes_S->ReqCount = ReqCount;
	CurDes_S->i = 0;
	CurDes_S->format = 0;
	CurDes_S->DataBufferAddr = BufferAddr;
	return 0;
}

int sd_init_short_desc_with_end(unsigned long *DescAddr, unsigned int ReqCount, unsigned long *BufferAddr,int end)
{
	struct _SD_PDMA_DESC_S *CurDes_S = (struct _SD_PDMA_DESC_S *) DescAddr;
	sd_init_short_desc(DescAddr,ReqCount,BufferAddr);
	CurDes_S->end = end;
	return 0;

}

#define MAX_LONG_DES_LEN ((1<<16) - 32) //2^15


void sd_config_desc(unsigned long *DescAddr, unsigned long *BufferAddr, int Blk_Cnt)
{
	struct _SD_PDMA_DESC_S *CurDes_S = (struct _SD_PDMA_DESC_S *)DescAddr;

	CurDes_S->ReqCount = 512*Blk_Cnt;
	CurDes_S->i = 0;
	CurDes_S->format = 0;
	CurDes_S->end = 1;
	CurDes_S->DataBufferAddr = BufferAddr;

}


void sd_dump_memory(int where, int how_many)
{
	unsigned long *ahb_mem_data_in = (unsigned long *)(0x4600000) ;
	unsigned long * CurValue = NULL;
	int Blk_Cnt = how_many;
	int loop = 0;
	for( CurValue=ahb_mem_data_in+( where*512/4); 
	       loop <Blk_Cnt*512/4; 
		   loop++, CurValue++)
	{
		if(loop %8 ==0)
			SDPRINTK("\n0x%8x:	0x%8lx",(where*512+loop*4),*CurValue);
		else
			SDPRINTK("  0x%8lx",*CurValue);
	}	
}
int sd_config_pdma(unsigned long *DescAddr, char dir)
{
	pSd_PDma_Reg->DMA_IER = SD_PDMA_IER_INT_EN;
	pSd_PDma_Reg->DMA_DESPR = DescAddr;
	if (dir == SD_PDMA_WRITE)
		pSd_PDma_Reg->DMA_CCR &= SD_PDMA_CCR_IF_to_peripheral;
	else
		pSd_PDma_Reg->DMA_CCR |= SD_PDMA_CCR_peripheral_to_IF;
	pSd_PDma_Reg->DMA_CCR |= SD_PDMA_CCR_RUN;
	return 0;
}

int SD_Init(void)
{
	struct _SD_PDMA_DESC_L ReadDesc;
	unsigned long *ahb_mem_data_in = (unsigned long *)malloc(8);
	unsigned char *mmc_ext_csd = (unsigned char *)malloc(512);
	memset(ahb_mem_data_in, 0x0, 8);
	memset(mmc_ext_csd, 0x0, 512);
	unsigned char	csd[16];
	unsigned int 	i;
	unsigned char	tmp, powerupOK, FourBitSupport = SD_FALSE;
	unsigned int	pwr_range0, pwr_range1, pwr_range2, pwr_range3, temp ;
	unsigned char	Read_Blk_Len, C_Size_Multi;
	unsigned short	C_Size, blkLength;

	unsigned long 	Card_Capacity, retryCount, sdstatus;

	unsigned char   scr[8], card_status[5], card_status_ready[5] ;
	unsigned int    RCA_t1, RCA_t2 ;

	SDDevInfo->RCAHi = 0x0 ;
	SDDevInfo->RCALo = 0x0 ;
	SDDevInfo->RCA = 0 ;
  	udelay(1000);
	
	/*	(1)send CMD0 to set all sd cards into Idle state	*/
	sdstatus = sd_command(GO_IDLE_STATE, 0, 0, R0);
	if (sdstatus != 0)
		goto err;


	/*	(2)check card type and its voltage range 	*/
	powerupOK = SD_FALSE;
	retryCount = 0;
	SDDevInfo->SDVersion = SD1_1;

	sdstatus = sd_command(SEND_IF_COND, 0, 0x000001AA, R1);
	retryCount = 0;
	while ((sdstatus != 0) && (retryCount < 5)) {			/*	retry 5 times	*/
		sdstatus = sd_command(SEND_IF_COND, 0, 0x000001AA, R1);
		retryCount++;
	}

	if (sdstatus == 0) {
		SDDevInfo->MMC_Card = SD_FALSE ;
		SDDevInfo->SD_Card = SD_TRUE ;

		tmp = pSd_Reg->resr[0];			/*	1st byte is cmd index, no use	*/
		pwr_range0 = pSd_Reg->resr[1];	/*	2st byte is resvert argument, no use	*/
		pwr_range1 = pSd_Reg->resr[2];	/*	3st byte is resvert argument, no use	*/
		pwr_range2 = pSd_Reg->resr[3];	/*	4st byte is VHS	*/
		pwr_range3 = pSd_Reg->resr[4];	/*	5st byte is Echo check pattern	*/

		if ((pwr_range2 == 0x01) && (pwr_range3 == 0xAA)) {	/*	 operation condition accepted by card	*/
			SDDevInfo->SDVersion = SD2_0;
			SDPRINTK("\nSD2_0  : SDHC card!\n") ;
		} else{
			printf("0x%x, 0x%x, 0x%x, 0x%x\n", pwr_range0, pwr_range1, pwr_range2, pwr_range3);
			SDPRINTK("\nSD2_0 Err  : power fail !\n") ;
			goto err;
		}
	} else {
		/*	2006/10/13 janshiue SEND_IF_COND(CMD8) retry 5 times no response
		 *  it means SD card below ver 1.1 or MMC card
		 *  F/W resend RESET(CMD0) to restart initial procedure.
		 */
		SDDevInfo->SDVersion = SD1_1;
		sdstatus = sd_command(GO_IDLE_STATE, 0, 0, R0);

		if (sdstatus != 0) {
			SDPRINTK("\nSD1_1 Err  : idle fail !\n") ;
			goto err;
		}
	}

	/*	for 1.0 and 1.1	*/
	if (SDDevInfo->SDVersion != SD2_0) {
		/*	ACMD41 to get card's require voltage range	*/
		sdstatus = sd_app_command(SD_APP_OP_COND, 0, 0, R3) ;
		retryCount = 0;
		/*	retry ACMD41	*/
		while ((sdstatus != 0) && (retryCount < 20)) {
			sdstatus = sd_app_command(SD_APP_OP_COND, 0, 0, R3) ;
			retryCount++;
			card_nop() ;
			card_nop() ;
			card_nop() ;
		}

		if (sdstatus == 0) {
			SDDevInfo->MMC_Card = SD_FALSE;
			SDDevInfo->SD_Card = SD_TRUE;
			SDPRINTK("\nInit  : SD Card !\n") ;
		} else {
			SDDevInfo->MMCMaxClockRate = SD_Clk_15MHz ;
			SDDevInfo->MMC_Card = SD_TRUE;
			SDDevInfo->SD_Card = SD_FALSE;
			SDPRINTK("\nInit  : MMC Card !\n") ;
		}

	}


	/*	(i)check MMC card or not	*/
	if (SDDevInfo->SD_Card == SD_FALSE) {
		sdstatus = sd_command(GO_IDLE_STATE, 0, 0, R0);
		if (sdstatus != 0) {
			SDPRINTK("\nMMC Err  : idle fail !\n") ;
			goto err;
		}
		/*	CMD1 to get card's require voltage range	*/
		sdstatus = sd_command(SEND_OP_COND, 0x0, 0x40fc0000, R3) ;
		retryCount = 0;
		/*	retry CMD1	*/
		while ((sdstatus != 0) && (retryCount < 10)) {
			sdstatus = sd_command(SEND_OP_COND, 0x0, 0x40fc0000, R3);
			retryCount++;
		}
		if (sdstatus == 0) {
			SDDevInfo->MMC_Card = SD_TRUE;
			SDDevInfo->SD_Card = SD_FALSE;
		} else {
			SDDevInfo->MMC_Card = SD_FALSE;
			SDDevInfo->SD_Card = SD_FALSE;
		}
	}

	/*	no response to SD_APP_OP_COND and SEND_OP_COND	*/
	/*	card type can't be judged	*/
	if ((SDDevInfo->MMC_Card == SD_FALSE) && (SDDevInfo->SD_Card == SD_FALSE)) {
		SDPRINTK("\nErr  : unknown type !\n") ;
		goto err;
	}

	if (SDDevInfo->SDVersion == SD2_0)
		sdstatus = sd_app_command(SD_APP_OP_COND, 0, 0x40ff8000, R3);	/*	 2.7~3.6 v	*/



	/*	get the retun OCR content	*/
	tmp = pSd_Reg->resr[0];			/*	1st byte is cmd index, no use	*/
	pwr_range0 = pSd_Reg->resr[1];	/*	OCR[31:24]	*/
	pwr_range1 = pSd_Reg->resr[2];	/*	OCR[23:16]	*/
	pwr_range2 = pSd_Reg->resr[3];	/*	OCR[15:8]	*/
	pwr_range3 = pSd_Reg->resr[4];	/*	OCR[7:0]	*/

	if (SDDevInfo->SDVersion == SD2_0) {
		/*	 to support SD2.0 High capacity card	*/
		pwr_range0 |= HostCapacitySupport;
	}

	/*	(3)power the card and wait for card power phase complete	*/

	/*	retry CMD1 or ACMD41 till powerup ok	*/
	retryCount = 0;
	powerupOK = SD_FALSE;
	/*	add timeout to SD power up retry check (ACMD41 & CMD1). It is for plugin-plugout issue.	*/
	
	udelay(64000);	
	//add timeout to SD power up retry check (ACMD41 & CMD1). It is for plugin-plugout issue.
	
	while (powerupOK == SD_FALSE) {
		if (SDDevInfo->MMC_Card == SD_TRUE)
			sdstatus = sd_command(SEND_OP_COND, 0, 0x40fc0000, R3);
		else{
			temp = (pwr_range0 << 24)|(pwr_range1 << 16)|(pwr_range2 << 8)|(pwr_range3) ;
			sdstatus = sd_app_command(SD_APP_OP_COND, 0, temp, R3) ;
		}
		/*	chek OCR[31] bit set or not	*/
		tmp = pSd_Reg->resr[0]; /*	1st byte is cmd index	*/
		tmp = pSd_Reg->resr[1]; /*	OCR[31:24]	*/
		/* Save OCR Register */
		SDDevInfo->ocr[0] = pSd_Reg->resr[1];
		SDDevInfo->ocr[1] = pSd_Reg->resr[2];
		SDDevInfo->ocr[2] = pSd_Reg->resr[3];
		SDDevInfo->ocr[3] = pSd_Reg->resr[4];
		
		if (tmp & 0x80) {
			/*	check CardCapacityStatus	MMCA 4.2 must check this*/
			if (tmp & 0x40)
				SDDevInfo->CardCapacityStatus = HighCapacity;
			else
				SDDevInfo->CardCapacityStatus = StandardCapacity;
			

			powerupOK = SD_TRUE;
		} else{
			for (i = 0; i < 20; i++) {
				tmp = pSd_Reg->resr[i] ;
				card_nop() ;
				card_nop() ;
				card_nop() ;
			}
		}


		retryCount++;
		udelay(64000);				

		if (retryCount > 36) {
			SDPRINTK("\nSD Err  : power fail 1 !\n") ;
			goto err;
		}

	}

	if (sdstatus != 0) {
		SDPRINTK("\nSD Err  : power fail 2 !\n") ;
		goto err;
	}
	SDPRINTK("\nInit  : Read OCR OK !\n") ;
	/*	(4)read card CID, RCA	*/
	sdstatus = sd_command(ALL_SEND_CID, 0, 0x0, R2);
	if (sdstatus != 0) {
		SDPRINTK("\n Err  :send CID fail !\n") ;
		goto err;
	}
	/* Save CID Register*/
	SDDevInfo->cid[0] = pSd_Reg->resr[1];
	SDDevInfo->cid[1] = pSd_Reg->resr[2];
	SDDevInfo->cid[2] = pSd_Reg->resr[3];
	SDDevInfo->cid[3] = pSd_Reg->resr[4];
	SDDevInfo->cid[4] = pSd_Reg->resr[5];
	SDDevInfo->cid[5] = pSd_Reg->resr[6];
	SDDevInfo->cid[6] = pSd_Reg->resr[7];
	SDDevInfo->cid[7] = pSd_Reg->resr[8];
	SDDevInfo->cid[8] = pSd_Reg->resr[9];
	SDDevInfo->cid[9] = pSd_Reg->resr[10];
	SDDevInfo->cid[10] = pSd_Reg->resr[11];
	SDDevInfo->cid[11] = pSd_Reg->resr[12];
	SDDevInfo->cid[12] = pSd_Reg->resr[13];
	SDDevInfo->cid[13] = pSd_Reg->resr[14];
	SDDevInfo->cid[14] = pSd_Reg->resr[15];


	/**************************************************
	 *if SD Card, get RCA from card
	 *if MMC_Card, controller need to assign RCA (0xffc1) for card
	 **************************************************/
	if (SDDevInfo->SD_Card == SD_TRUE) {
		sdstatus = sd_command(SEND_RELATIVE_ADDR, 0, 0x0, R6);
		if (sdstatus != 0) {
			SDPRINTK("\nSD Err  :send RCA fail !\n") ;
			goto err;
		}

		tmp = pSd_Reg->resr[1];
		SDDevInfo->RCAHi = tmp;
		tmp = pSd_Reg->resr[2];
		SDDevInfo->RCALo = tmp;
	} else{
		/*	PKtest 9/5 adjust the timeOut register value	*/
		sdstatus = sd_command(SET_RELATIVE_ADDR, 0, 0xffc10000, R1);
		if (sdstatus != 0) {
			SDPRINTK("\nMMC Err  :send RCA fail !\n") ;
			goto err;
		}

		SDDevInfo->RCAHi = 0xff;
		SDDevInfo->RCALo = 0xc1;
	}

	/*	(5)read card CSD to know card size, capability	*/

	RCA_t1 = 0x0 ;
	RCA_t2 = 0x0 ;
	RCA_t1 |= SDDevInfo->RCAHi;
	RCA_t2 |= SDDevInfo->RCALo;
	SDDevInfo->RCA = (RCA_t1 << 24) | (RCA_t2 << 16) ;

	sdstatus = sd_command(SEND_CSD, 0, SDDevInfo->RCA, R2);
	if (sdstatus != 0) {
		SDPRINTK("\nErr  :send CSD fail !\n") ;
		goto err;
	}

	/* Save CSD Register*/
	for (i = 0; i < 15; i++) {
		csd[i]= pSd_Reg->resr[i+1];
		SDDevInfo->csd[i] = pSd_Reg->resr[i+1];
	}

	Read_Blk_Len = csd[5] & 0x0f;

	if ((SDDevInfo->SD_Card == SD_TRUE) && (csd[0] == 0x40)) {
		/*	 CSD 2.0	*/
		Card_Capacity = ((((csd[7] & 0x3F) << 8)|csd[8]) << 8) | csd[9];
		Card_Capacity++;

		/* SD2.0 SPEC : memory capacity = (C_SIZE + 1) * 512 K Byte
		 * Card_Capacity is base on 512 bytes,
		 * thus return Card_Capacity *((512 * 1024)/512)
		 */
		Card_Capacity = Card_Capacity * 1024;
		SDDevInfo->SDCard_Size = Card_Capacity;
	}

	if ((SDDevInfo->MMC_Card == SD_TRUE) || (csd[0] == 0x00)) {
		/*	MMC card or SD 1.1 below	*/
		C_Size = (((csd[6] & 0x03) << 8) | csd[7]) << 2 | (csd[8] & 0xC0) >> 6;
		C_Size_Multi = (csd[9] & 0x3) << 1 | (csd[10] & 0x80) >> 7;
		blkLength = ((unsigned short)1) << Read_Blk_Len;
		Card_Capacity = ((((unsigned long)(C_Size + 1)) << (C_Size_Multi + 2)) * blkLength) / 512;
		SDDevInfo->SDCard_Size = Card_Capacity;
	}

	/*	(6)select card	*/
	sdstatus = sd_command(SELECT_DESELECT_CARD, 0, SDDevInfo->RCA, R1) ;
	if (sdstatus != 0) {
		SDPRINTK("\nErr  :deselect card fail !\n") ;
		goto err;
	}

	sdstatus = sd_command(SEND_STATUS, 0, SDDevInfo->RCA, R1);
	if (sdstatus != 0)
		goto err;

	for (i = 0; i <= 4; i++) {
		card_status[i] = 0 ;
		card_status_ready[i] = 0 ;
	}
	card_status_ready[0] = 0x0 ;
	card_status_ready[1] = 0x0 ;
	card_status_ready[2] = 0x0 ;
	card_status_ready[3] = 0x09 ;
	card_status_ready[4] = 0x0 ;

	for (i = 1; i <= 4; i++)
		card_status[i] = pSd_Reg->resr[i] ;

	for (i = 1; i <= 4; i++) {
		if (card_status[i] != card_status_ready[i]) {
			SDPRINTK("\n pSd_Reg->Rsp[1]=%X ! \n", pSd_Reg->resr[1]) ;
			SDPRINTK("\n pSd_Reg->Rsp[2]=%X ! \n", pSd_Reg->resr[2]) ;
			SDPRINTK("\n pSd_Reg->Rsp[3]=%X ! \n", pSd_Reg->resr[3]) ;
			SDPRINTK("\n pSd_Reg->Rsp[4]=%X ! \n", pSd_Reg->resr[4]) ;

			SDPRINTK("\n Card Status Error ! \n") ;
			goto err;
		}
	}
	SDPRINTK("\nInit  : card selected !\n") ;
	/*	(7)read card SCR to know card support 4bit mode or not	*/
	if (SDDevInfo->SD_Card == SD_TRUE) {
		SDPRINTK("\nInit  : SD card set mode !\n") ;
		FourBitSupport = SD_FALSE;

		SD_Init_PDMA();
	/*	sd_alloc_desc_pool(ReadDesc);*/
		sd_init_long_desc(
			(unsigned long *)&ReadDesc,
			8,
			ahb_mem_data_in,
			(unsigned long *)(&ReadDesc + sizeof(struct _SD_PDMA_DESC_L)),
			1
		);
		sd_config_pdma((unsigned long *)&ReadDesc, SD_PDMA_READ);

		sdstatus = sd_app_command(SD_SEND_SCR, 2, 0x0, R1) ;
		if (sdstatus != 0)
			goto err;

		sdstatus = sd_pdma_handler();

		if (sdstatus != 0)
			goto err;

		/* Save SCR Register*/
		for (i = 0; i < 8; i++) {
			scr[i] = *((uchar *)ahb_mem_data_in + i) ;
			SDDevInfo->scr[i] = *((uchar *)ahb_mem_data_in + i) ;
		}
		/*	free dma	*/
		sd_free_pdma();


		tmp = scr[0] ;
		if ((tmp & 0x0f) != 0x00)
			SDDevInfo->HighSpeedSupport = SD_TRUE;

		tmp = scr[1] ;
		if ((tmp & 0x0f) == 0x05)
			FourBitSupport = SD_TRUE;
	}

	/*	(8)set blklength and bus width	*/

	/*	set block length to be 512bytes	*/
	blkLength = 0;
	blkLength = ((unsigned short)1) << Read_Blk_Len;
	if (blkLength != 512) {
		SDPRINTK("\nset blocklen fun. !\n") ;
		sdstatus = sd_command(SET_BLOCKLEN, 0, 0x200, R1);
		if (sdstatus != 0) {
			SDPRINTK("\nErr  :set blocklen fail !\n") ;
			goto err;
		}
	}
	
	pSd_Reg->blklen |= 0x1FF ;

	//if SD card support 4bit bus set to 4bit mode transfer
 	if (SDDevInfo->SD_Card == SD_TRUE) {
		SDDevInfo->MMCMaxClockRate = SD_Clk_25MHz ;
		if (FourBitSupport == SD_TRUE) {
			if (Bitmode == 0) {
		  		sdstatus = sd_app_command(SET_BUS_WIDTH ,0 ,0x2 ,R1 );
		  		if (sdstatus != 0 )
					goto err;
	
				/*	set BusMode register to 4bit mode	*/
		  		pSd_Reg->busm |= FOURBIT_MODE;
				SDPRINTK("\nSD:Four Bits Mode!\n") ;
			} else{
	    			pSd_Reg->busm &= ONEBIT_MODE ;
				SDPRINTK("\nSD:One Bit Mode!\n") ;
			}
		}
		/*	Check SD Card High Speend Support	*/
		if (SDDevInfo->HighSpeedSupport == SD_TRUE) {
    			//SDPRINTK("\nsdmmcSwitch func.\n") ;

            SDDevInfo->MMCMaxClockRate = SD_Clk_44MHz;	
		 	if ( sdmmcSwitch() != 0 ) {
		     		SDPRINTK("\nErr: sdmmcSwitch func.\n") ;
		     		pSd_Reg->blklen = 0x01FF ;
				goto err;
		 	}
		
			pSd_Reg->blklen = 0x01FF ;
		}
	}


	//int tmp1 ;
	if  (SDDevInfo->MMC_Card == SD_TRUE) {
		SDPRINTK("\nInit  :MMC card set mode !\n") ;
		/*	save the MMCVersion	*/
		SDDevInfo->MMCVersion = (csd[0] >> 2) & 0x0f;

		/*	check spec version	*/
		if (SDDevInfo->MMCVersion < MMC4_01) {
       		pSd_Reg->busm &= ONEBIT_MODE ;
			SDPRINTK("\nMMC1_0 :One Bit Mode!\n") ;

		}
		if (SDDevInfo->MMCVersion >= MMC4_01) {

			sdmmcSendExCSD((unsigned long *)mmc_ext_csd);
			/* Save MMC Extended CSD Register */
			for (i = 0; i < 512; i += 4) {
				SDDevInfo->ext_csd[i] = *(mmc_ext_csd + i);
				SDDevInfo->ext_csd[i + 1] = *(mmc_ext_csd + i + 1);
				SDDevInfo->ext_csd[i + 2] = *(mmc_ext_csd + i + 2);
				SDDevInfo->ext_csd[i + 3] = *(mmc_ext_csd + i + 3);
				/*SDPRINTK("CMD %x SCR[%x] =%x \n", pSd_Reg->Cmd, i, *(mmc_ext_csd + i));*/
			}
			Card_Capacity = SDDevInfo->ext_csd[212] << 0 | 
							SDDevInfo->ext_csd[213] << 8 |
							SDDevInfo->ext_csd[214] << 16 |
							SDDevInfo->ext_csd[215] << 24;
			if (Card_Capacity)
				SDDevInfo->SDCard_Size = Card_Capacity;
			
			tmp = *(mmc_ext_csd + 196);
		  	if (tmp & BIT1) {	/* high speed and max clock rate at 52Mhz*/
				 /*high speed mode 52Mhz*/
			 	SDDevInfo->MMCMaxClockRate = SD_Clk_44MHz;	
			 	SDDevInfo->HighSpeedSupport = SD_TRUE;
		    	SDPRINTK("\nMMC : CLK 48!\n");
	    		} else if(tmp ==0x01) {/* high speed and max clock rate at 26Mhz*/
			  	SDDevInfo->MMCMaxClockRate = SD_Clk_25MHz;
			  	SDDevInfo->HighSpeedSupport = SD_FALSE;
		  		SDPRINTK("\nMMC : CLK 24!\n");
		  	} else {
			  	SDDevInfo->MMCMaxClockRate = SD_Clk_15MHz ;/*Clk_15;*/
			  	SDDevInfo->HighSpeedSupport = SD_FALSE;		
		  		SDPRINTK("\nMMC : CLK 15!\n");
			}

			if (SDDevInfo->HighSpeedSupport == SD_TRUE) {
				SDPRINTK("\nInit  :MMC card High Speed Support !\n") ;
				/*	 Card support High Speed interface	*/
				sdstatus = sd_command(SWITCH_MMC, 0, 0x03B90100, R1);

				/*add delay to patch eMMC for MMCA 4.3*/
				udelay(10000);

				sdstatus = sd_command(SEND_STATUS, 0, SDDevInfo->RCA, R1);
				for (i = 1; i <= 4; i++)
	         			card_status[i]=pSd_Reg->resr[i] ;

				if ((card_status[3] & 0x80) == 0x00) {
					/*	switch command ok	*/
					/*	check SWITCH to High speed timming SUCCESS	*/

					sdmmcSendExCSD((unsigned long *)mmc_ext_csd);
					tmp = *(mmc_ext_csd + 185);

					if (tmp == 0x01) {/* switch to high speed timming*/
						SDDevInfo->HighSpeedSupport = SD_TRUE;
						SDPRINTK("\nMMC : High speed is true !\n");
					} else {	/* switch failer*/
						SDDevInfo->HighSpeedSupport = SD_FALSE;
						SDPRINTK("\nMMC : High speed is false !\n");
					}
				}
			}

			/* Use SWITCH command to set MMC 4.0 card to 4bit mode
			 * access = 0x3 write byte
			 * index = 183 point to BUS_WIDTH field
			 * value = 1 indicate 4 bit bus, 2 indicate 8 bit bus
			 */
			if (Bitmode == 0)
				Bitmode = 4;
			if (Bitmode == 0) {
				sdstatus = sd_command(SWITCH_MMC, 0, 0x03B70200, R1);

				sdstatus = sd_command(SEND_STATUS, 0, SDDevInfo->RCA, R1);
				for (i = 1; i <= 4; i++)
	    				card_status[i]=pSd_Reg->resr[i] ;

				if ((card_status[3]  & 0x80) == 0x00) { /*	switch command ok	*/
					/*	set BusMode register to 8bit mode	*/
					/*
					//SDReg.BusMode |= FOURBIT_MODE;
					//SDDevInfo->MMC40 = TRUE;
					*/
					pSd_Reg->extctl |= EIGHTBIT_MODE;			
					SDDevInfo->MMCVersion = MMC4_01;
					SDPRINTK("\nMMC4_0 : Eight Bits Mode !\n");
				} else {
					/*	 can't switch to 8bit mode, try 4bit mode	*/

					/*2006/03/08 marked by janshiue, Since if card don't support
					 *           8 bit mode the card still MMC 4.0 card, it should
					 *           perform at 1 or 4 bit mode. if set SDDevInfo->MMCVersion to
					 *           MMC3_123. it will also disable the high speed support.
					 */
					/*//SDDevInfo->MMCVersion = MMC4_01;*/

					sdstatus = sd_command(SWITCH_MMC, 0, 0x03B70100, R1);

					sdstatus = sd_command(SEND_STATUS, 0, SDDevInfo->RCA, R1);
					for (i = 1; i <= 4; i++)
	    					card_status[i]=pSd_Reg->resr[i] ;

					if ((card_status[3] & 0x80) == 0x00) {
						/*	switch command ok	*/
						/*	set BusMode register to 4bit mode	*/
				 		pSd_Reg->busm |= FOURBIT_MODE;
						SDPRINTK("\nMMC4_0:Four Bits Mode!\n") ;
						SDDevInfo->MMCVersion = MMC4_01;
					}

				}
			}
			if (Bitmode == 4) {
				/*switch to 4 bit mode*/

				sdstatus = sd_command(SWITCH_MMC, 0, 0x03B70100, R1);

				sdstatus = sd_command(SEND_STATUS, 0, SDDevInfo->RCA, R1);
				for (i = 1; i <= 4; i++)
					card_status[i] = pSd_Reg->resr[i] ;

				if ((card_status[3] & 0x80) == 0x00) {
					/*	switch command ok	*/
					/*	set BusMode register to 4bit mode	*/
					pSd_Reg->busm |= FOURBIT_MODE;
					SDPRINTK("\nMMC4_0:Four Bits Mode!\n") ;
					SDDevInfo->MMCVersion = MMC4_01;
				}
			}
			if (Bitmode == 1) {
				sdstatus = sd_command(SWITCH_MMC, 0, 0x03B70000, R1);
				sdstatus = sd_command(SEND_STATUS, 0, SDDevInfo->RCA, R1);
				for (i = 1; i <= 4; i++)
					card_status[i] = pSd_Reg->resr[i] ;

				if ((card_status[3] & 0x80) == 0x00) {
					/*	switch command ok	*/
					/*	set BusMode register to 1bit mode	*/
					pSd_Reg->busm &= ONEBIT_MODE ;
					SDPRINTK("\nMMC4_0 :One Bit Mode!\n") ;
					SDDevInfo->MMCVersion = MMC4_01;
				}
			}

		}
	}
	free(mmc_ext_csd);
	free(ahb_mem_data_in);
	return 0;
err:
	free(mmc_ext_csd);
	free(ahb_mem_data_in);
	return -1;
}


int SD_Initialization(void)
{
    unsigned char sdstatus = 0;
  
    Bitmode = 0 ; // eight/four bits mode
    //card type is unknown here
    SDDevInfo->MMC_Card = SD_FALSE ;
    SDDevInfo->SD_Card = SD_FALSE ;
    SDDevInfo->SD_IO_Card = SD_FALSE ;
  
    //get SD/MMC card info
    SDDevInfo->InitOK = SD_FALSE ;
    //SDDevInfo->MMCMaxClockRate = Clk_375;
    sdstatus = SD_Init() ;  
    //error handle during initialization
    if(sdstatus != 0)
    {
        printf("\nInitial SD/MMC Card Fail!\n");
        return -1 ;
    }
    else
    {
        SDDevInfo->InitOK = SD_TRUE ;
        printf("\nInitial SD/MMC Card OK!\n");
    }
    
    return 0 ;
}



/**********************************************************************************/
extern int
fat_register_device(block_dev_desc_t *dev_desc, int part_no);

block_dev_desc_t * mmc_get_dev(int dev)
{
	if (dev == 0)
		return ((block_dev_desc_t *)&mmc0_dev);
	else if (dev == 1)
		return ((block_dev_desc_t *)&mmc1_dev);
	else if (dev == 2)
		return ((block_dev_desc_t *)&mmc2_dev);
}

/*
 * FIXME needs to read cid and csd info to determine block size
 * and other parameters
 */
//static uchar mmc_buf[MMC_BLOCK_SIZE];
//static mmc_csd_t mmc_csd;


int
/****************************************************/
mmc_block_read_singleblock(uchar *dst, ulong src)
/****************************************************/
{
	unsigned long sd_status;
	struct _SD_PDMA_DESC_S ReadDesc;
	/*SDPRINTK("\n Read Single block begin \n");*/
	long count =1000000;
	while(1) {
		sd_status = sd_command(SEND_STATUS, 0, SDDevInfo->RCA, R1);
		if (sd_status != 0)
			return -1 ;
		if (pSd_Reg->resr[3] == 0x09)
			break;
		count--;
		if (count == 0) {
			SDPRINTK("mmc_block_read_singleblock Time Out!\n");
			break;
		}
		
	}


	pSd_Reg->blkcnt = 0x01 ;

	pSd_Reg->str0 |= 0xFF;
	pSd_Reg->str1 |= 0xFF;
	
	/*	Config Read DMA	*/
	SD_Init_PDMA();
	sd_config_desc((unsigned long *)(&ReadDesc), (unsigned long*)dst, 1);
	sd_config_pdma((unsigned long *)(&ReadDesc), SD_PDMA_READ);

	sd_status = sd_command(READ_SINGLE_BLOCK, 2, src, R1) ;
	if (sd_status != 0) {
		SDPRINTK("\nRead single Block CMD Err !\n");
		return -1 ;
	}

	
	sd_status = sd_pdma_handler();
	if (sd_status != 0) {
		pSd_Reg->timeval = 0xefff;
	/*	SDPRINTK("pSd_Reg->Cmd = 0x%x\n", pSd_Reg->Cmd);
		SDPRINTK("pSd_Reg->STS0 = 0x%x\n", pSd_Reg->ctrl);
		SDPRINTK("pSd_Reg->DmaTout0 = 0x%x\n", pSd_Reg->DmaTout[0]);
		SDPRINTK("pSd_Reg->DmaTout1 = 0x%x\n", pSd_Reg->DmaTout[1]);
		SDPRINTK("pSd_Reg->STS1 = 0x%x\n", pSd_Reg->str1);
		SDPRINTK("pSd_Reg->BlkCnt[0] = 0x%x\n", pSd_Reg->blkcnt);
		SDPRINTK("pSd_Reg->Cbcr[0] = 0x%x\n", pSd_Reg->Cbcr[0]);
		SDPRINTK("pSd_Reg->Cbcr[1] = 0x%x\n", pSd_Reg->Cbcr[1]);
		SDPRINTK("pSd_Reg->Res3[0] = 0x%x\n", pSd_Reg->Res3[0]);
		SDPRINTK("pSd_Reg->Res3[1] = 0x%x\n", pSd_Reg->Res3[1]);*/
		return -1 ;
	}

	/*	free dma	*/
	sd_free_pdma();
	/*SDPRINTK("\n Read Single block end \n");*/
	return 0 ;

}

int
/****************************************************/
mmc_block_read_multiblock(uchar *dst, ulong src, ulong ulBlockCount)
/****************************************************/
{
    unsigned long sd_status;
	struct _SD_PDMA_DESC_S ReadDesc;
	long count =1000000;
	while(1) {
		sd_status = sd_command(SEND_STATUS, 0, SDDevInfo->RCA, R1);
		if (sd_status != 0)
			return -1 ;
		if (pSd_Reg->resr[3] == 0x09)
			break;
		count--;
		if (count == 0) {
			SDPRINTK("mmc_block_read_multiblock Time Out!\n");
			break;
		}
		
	}

	/*SDPRINTK("\n Read Multi block begin \n");*/
	while (ulBlockCount > 127) {
		pSd_Reg->blkcnt = (unsigned short)(127  & 0xFFFF);
		pSd_Reg->ctlr |= 0x08 ;   /*	 response FIFO reset,	*/

		pSd_Reg->str0 |= 0xFF;
		pSd_Reg->str1 |= 0xFF;
		
		/*	Config Read DMA	*/
		SD_Init_PDMA();
		sd_config_desc((unsigned long *)(&ReadDesc), (unsigned long*)dst, 127);
		sd_config_pdma((unsigned long *)(&ReadDesc), SD_PDMA_READ);
		//pSd_Reg->extctl |= 0x01;

		sd_status = sd_command(READ_MULTIPLE_BLOCK, 4, src, R1) ;
		if (sd_status != 0) {
			SDPRINTK("\nRead Multi Block CMD Err !\n");
			return -1 ;
		}

		sd_status = sd_pdma_handler();
		if (sd_status != 0) {
			pSd_Reg->timeval = 0xefff;
		/*	SDPRINTK("pSd_Reg->Cmd = 0x%x\n", pSd_Reg->Cmd);
		SDPRINTK("pSd_Reg->STS0 = 0x%x\n", pSd_Reg->ctrl);
		SDPRINTK("pSd_Reg->DmaTout0 = 0x%x\n", pSd_Reg->DmaTout[0]);
		SDPRINTK("pSd_Reg->DmaTout1 = 0x%x\n", pSd_Reg->DmaTout[1]);
		SDPRINTK("pSd_Reg->STS1 = 0x%x\n", pSd_Reg->str1);
		SDPRINTK("pSd_Reg->BlkCnt[0] = 0x%x\n", pSd_Reg->blkcnt);
		SDPRINTK("pSd_Reg->Cbcr[0] = 0x%x\n", pSd_Reg->Cbcr[0]);
		SDPRINTK("pSd_Reg->Cbcr[1] = 0x%x\n", pSd_Reg->Cbcr[1]);
		SDPRINTK("pSd_Reg->Res3[0] = 0x%x\n", pSd_Reg->Res3[0]);
		SDPRINTK("pSd_Reg->Res3[1] = 0x%x\n", pSd_Reg->Res3[1]);*/
			return -1 ;
		}

		/*	free dma	*/
		sd_free_pdma();
	
		/*	 send CMD12 : stop command	*/
		pSd_Reg->ctlr |= 0x08 ;   /*	 response FIFO reset,	*/
		sd_status = sd_command(STOP_TRANSMISSION, 7, src, R1b) ;
		if (sd_status != 0)
			return -1 ;
		ulBlockCount -= 127;

		if (SDDevInfo->CardCapacityStatus == 0)
			src += 127*512;
		else
			src += 127;
		dst += 127*512;

		count =1000000;
		while(1) {
			sd_status = sd_command(SEND_STATUS, 0, SDDevInfo->RCA, R1);
			if (sd_status != 0)
				return -1 ;
			if (pSd_Reg->resr[3] == 0x09)
				break;
			count--;
			if (count == 0) {
				SDPRINTK("mmc_block_write_multiblock Time Out!\n");
				break;
			}
		
		}
		
	}
	if (ulBlockCount) {
		pSd_Reg->blkcnt = (unsigned short)(ulBlockCount  & 0xFFFF);
		pSd_Reg->ctlr |= 0x08 ;   /*	 response FIFO reset,	*/

		pSd_Reg->str0 |= 0xFF;
		pSd_Reg->str1 |= 0xFF;

		/*	Config Read DMA	*/
		SD_Init_PDMA();
		sd_config_desc((unsigned long *)(&ReadDesc), (unsigned long*)dst, ulBlockCount);
		sd_config_pdma((unsigned long *)(&ReadDesc), SD_PDMA_READ);
		//pSd_Reg->extctl |= 0x01;

		sd_status = sd_command(READ_MULTIPLE_BLOCK, 4, src, R1) ;
		if (sd_status != 0) {
			SDPRINTK("\nRead Multi Block CMD Err !\n");
			return -1 ;
		}

		sd_status = sd_pdma_handler();
		if (sd_status != 0) {
			pSd_Reg->timeval = 0xefff;
		/*	SDPRINTK("pSd_Reg->Cmd = 0x%x\n", pSd_Reg->Cmd);
		SDPRINTK("pSd_Reg->STS0 = 0x%x\n", pSd_Reg->ctrl);
		SDPRINTK("pSd_Reg->DmaTout0 = 0x%x\n", pSd_Reg->DmaTout[0]);
		SDPRINTK("pSd_Reg->DmaTout1 = 0x%x\n", pSd_Reg->DmaTout[1]);
		SDPRINTK("pSd_Reg->STS1 = 0x%x\n", pSd_Reg->str1);
		SDPRINTK("pSd_Reg->BlkCnt[0] = 0x%x\n", pSd_Reg->blkcnt);
		SDPRINTK("pSd_Reg->Cbcr[0] = 0x%x\n", pSd_Reg->Cbcr[0]);
		SDPRINTK("pSd_Reg->Cbcr[1] = 0x%x\n", pSd_Reg->Cbcr[1]);
		SDPRINTK("pSd_Reg->Res3[0] = 0x%x\n", pSd_Reg->Res3[0]);
		SDPRINTK("pSd_Reg->Res3[1] = 0x%x\n", pSd_Reg->Res3[1]);*/
			return -1 ;
		}

		/*	free dma	*/
		sd_free_pdma();
	
		/*	 send CMD12 : stop command	*/
		pSd_Reg->ctlr |= 0x08 ;   /*	 response FIFO reset,	*/
		sd_status = sd_command(STOP_TRANSMISSION, 7, src, R1b) ;
		if (sd_status != 0)
			return -1 ;

		count =1000000;
		while(1) {
			sd_status = sd_command(SEND_STATUS, 0, SDDevInfo->RCA, R1);
			if (sd_status != 0)
				return -1 ;
			if (pSd_Reg->resr[3] == 0x09)
				break;
			count--;
			if (count == 0) {
				SDPRINTK("mmc_block_write_multiblock Time Out!\n");
				break;
			}
		
		}
		
	}
	/*SDPRINTK("\nRead multi Block CMD OK !\n");*/
	return 0 ;
}

int
/****************************************************/
mmc_block_write_singleblock(uchar *src, ulong ulBlockStart)
/****************************************************/
{

	unsigned long sd_status;
	struct _SD_PDMA_DESC_S WriteDesc;
	//SDPRINTK("\n Write Single block begin \n");
	long count =0x1000000;
	
	pSd_Reg->ctlr |= 0x08 ;   /*	 response FIFO reset,	*/
	pSd_Reg->str0 |= 0x30;
	
	while(1) {
		sd_status = sd_command(SEND_STATUS, 0, SDDevInfo->RCA, R1);
		if (sd_status != 0)
			return -1 ;
		if (pSd_Reg->resr[3] == 0x09)
			break;
		count--;
		if (count == 0) {
			SDPRINTK("mmc_block_write_singleblock Time Out!\n");
			break;
		}
		
	}

	pSd_Reg->blkcnt = 0x01 ;
	
	pSd_Reg->str0 |= 0xFF;
	pSd_Reg->str1 |= 0xFF;
	/*	Config Read DMA	*/
	SD_Init_PDMA();
	sd_config_desc((unsigned long *)(&WriteDesc), (unsigned long*)src, 1);
	sd_config_pdma((unsigned long *)(&WriteDesc), SD_PDMA_WRITE);

	sd_status = sd_command(WRITE_SINGLE_BLOCK, 1, ulBlockStart, R1) ;
	if (sd_status != 0) {
		SDPRINTK("\nRead single Block CMD Err !\n");
		return -1 ;
	}

	sd_status = sd_pdma_handler();
	if (sd_status != 0) {
		pSd_Reg->timeval = 0xefff;
		return -1 ;
	}

	/*	free dma	*/
	sd_free_pdma();
	//SDPRINTK("\n Write Single block end \n");

	count =0x1000000;
	while(1) {
		sd_status = sd_command(SEND_STATUS, 0, SDDevInfo->RCA, R1);
		if (sd_status != 0)
			return -1 ;
		if (pSd_Reg->resr[3] == 0x09)
			break;
		count--;
		if (count == 0) {
			SDPRINTK("mmc_block_write_multiblock Time Out!\n");
			break;
		}
	
	}
	return 0 ;

}

int
/****************************************************/
mmc_block_write_multiblock(uchar *src, ulong ulBlockStart, ulong ulBlockCount)
/****************************************************/
{
    unsigned long sd_status;
	long count =0x1000000;
	struct _SD_PDMA_DESC_S WriteDesc;

	//SDPRINTK("\n Write Multi block begin ulBlockStart %d, ulBlockCount %d\n", ulBlockStart, ulBlockCount);
	while(1) {
		sd_status = sd_command(SEND_STATUS, 0, SDDevInfo->RCA, R1);
		if (sd_status != 0)
			return -1 ;
		if (pSd_Reg->resr[3] == 0x09)
			break;
		count--;
		if (count == 0) {
			SDPRINTK("mmc_block_write_multiblock Time Out!\n");
			break;
		}
		
	}
	while(ulBlockCount > 127) {
		pSd_Reg->blkcnt = (unsigned short)(127  & 0xFFFF);
		pSd_Reg->ctlr |= 0x08 ;   /*	 response FIFO reset,	*/
		pSd_Reg->str0 |= 0xFF;
		pSd_Reg->str1 |= 0xFF;
		/*	Config Read DMA	*/
		SD_Init_PDMA();
		sd_config_desc((unsigned long *)(&WriteDesc), (unsigned long*)src, 127);
		sd_config_pdma((unsigned long *)(&WriteDesc), SD_PDMA_WRITE);
		//pSd_Reg->extctl |= 0x01;

		sd_status = sd_command(WRITE_MULTIPLE_BLOCK, 3, ulBlockStart, R1) ;
		if (sd_status != 0) {
			SDPRINTK("\nWrite Multi Block CMD Err !\n");
			return -1 ;
		}

		/*sd_status = vt3400_sd_dma_handler() ;*/
		sd_status = sd_pdma_handler();
		if (sd_status != 0) {
			pSd_Reg->timeval = 0xefff;
			return -1 ;
		}
		
		/*	free dma	*/
		sd_free_pdma();
	
		/*	 send CMD12 : stop command	*/
		pSd_Reg->ctlr |= 0x08 ;   /*	 response FIFO reset,	*/
		sd_status = sd_command(STOP_TRANSMISSION, 7, (unsigned int)src, R1b) ;
		if (sd_status != 0)
			return -1 ;
		if (SDDevInfo->CardCapacityStatus == 0)
			ulBlockStart += 127*512;
		else
			ulBlockStart += 127;
		ulBlockCount -= 127;
		src += 127*512;
		
		
		count =0x1000000;
		while(1) {
			sd_status = sd_command(SEND_STATUS, 0, SDDevInfo->RCA, R1);
			if (sd_status != 0) {
				printf("mmc_block_write_multiblock fail!\n");
				return -1 ;
			}
			if (pSd_Reg->resr[3] == 0x09)
				break;
			count--;
			if (count == 0) {
				printf("mmc_block_write_multiblock Time Out!\n");
				break;
			}
		
		}
	}

	if (ulBlockCount) {
		pSd_Reg->blkcnt = (unsigned short)(ulBlockCount  & 0xFFFF);
		pSd_Reg->ctlr |= 0x08 ;   /*	 response FIFO reset,	*/
		pSd_Reg->str0 |= 0xFF;
		pSd_Reg->str1 |= 0xFF;
		/*	Config Read DMA	*/
		SD_Init_PDMA();
		sd_config_desc((unsigned long *)(&WriteDesc), (unsigned long*)src, ulBlockCount);
		sd_config_pdma((unsigned long *)(&WriteDesc), SD_PDMA_WRITE);
		//pSd_Reg->extctl |= 0x01;

		sd_status = sd_command(WRITE_MULTIPLE_BLOCK, 3, ulBlockStart, R1) ;
		if (sd_status != 0) {
			SDPRINTK("\nWrite Multi Block CMD last Err ! ulBlockStart %d ulBlockCount %d\n", ulBlockStart, ulBlockCount);
			return -1 ;
		}


		sd_status = sd_pdma_handler();
		if (sd_status != 0) {
			pSd_Reg->timeval = 0xefff;
			return -1 ;
		}

		/*	free dma	*/
		sd_free_pdma();
	
		/*	 send CMD12 : stop command	*/
		pSd_Reg->ctlr |= 0x08 ;   /*	 response FIFO reset,	*/
		sd_status = sd_command(STOP_TRANSMISSION, 7, (unsigned int)src, R1b) ;
		if (sd_status != 0)
			return -1 ;
		
		count =0x1000000;
		while(1) {
			sd_status = sd_command(SEND_STATUS, 0, SDDevInfo->RCA, R1);
			if (sd_status != 0)
				return -1 ;
			if (pSd_Reg->resr[3] == 0x09)
				break;
			count--;
			if (count == 0) {
				printf("mmc_block_write_multiblock Time Out!\n");
				break;
			}
		
		}
	}
	SDPRINTK("\n Write multi Block CMD OK !\n");
	return 0 ;
}

int
/****************************************************/
mmc_read(ulong src, uchar *dst, int size)
/****************************************************/
{
	ulong end, part_start, part_end, aligned_start, aligned_end;
	ulong mmc_block_size, mmc_block_address, ulBlockCount;

	if (size == 0) {
		return 0;
	}

	if (!mmc_ready) {
		printf("Please initial the MMC first\n");
		return -1;
	}

	mmc_block_size = MMC_BLOCK_SIZE;
	mmc_block_address = ~(mmc_block_size - 1);

	src -= CFG_MMC_BASE;
	end = src + size;
	part_start = ~mmc_block_address & src;
	part_end = ~mmc_block_address & end;
	aligned_start = mmc_block_address & src;
	aligned_end = mmc_block_address & end;
	ulBlockCount = (aligned_end-src)/MMC_BLOCK_SIZE;

	if (ulBlockCount ==1) {
		/*This is for single block read*/
		/*for (; src < aligned_end; src += mmc_block_size, dst += mmc_block_size) {*/
			if ((mmc_block_read_singleblock((uchar *)dst, (src>>9*SDDevInfo->CardCapacityStatus))) < 0) {
				return -1;
			}
		/*}*/
	} else {
		/*This is for multi block read*/
		if (mmc_block_read_multiblock((uchar*)dst, (src>>9*SDDevInfo->CardCapacityStatus), ulBlockCount) < 0) {
			return -1;
		}
	}
	return 0;
}

int
/****************************************************/
mmc_write(uchar *src, ulong dst, int size)
/****************************************************/
{

	return 0;
}


ulong
/****************************************************/
mmc_bread(int dev_num, ulong blknr, ulong blkcnt, ulong *dst)
/****************************************************/
{
	ulong src = 0;
	//ulong src_size = 0; //20120105 by eason
	//ulong SDCard_Size = SDDevInfo->SDCard_Size;
	//printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
	//printf(" dst 0x%x  dev_num 0x%x, blknr 0x%x, blkcnt 0x%x \n", dst,  dev_num, blknr, blkcnt);
	//printf(" SDDevInfo->CardCapacityStatus %d \n", SDDevInfo->CardCapacityStatus);
	if(!blkcnt)
		return blkcnt;
	
	/*select host*/
	Change_SD_host(dev_num);

	if (!mmc_ready) {
		printf("Please initial the MMC first\n");
		return -1;
	}
	
	src = SDDevInfo->CardCapacityStatus ? blknr : (blknr*MMC_BLOCK_SIZE);
	/*ount SD size by block based, a block = 512 bytes */	
	//src_size = SDDevInfo->CardCapacityStatus ? src : (src/MMC_BLOCK_SIZE); 
	//if(src_size > SDCard_Size) {  /*if reas address > SDCard size , the part_address is error */
	//	printf("mmc_bwrite part_offset error\n"); 
	//	return -1;
    //}
	if (blkcnt ==1) {
		/*This is for single block read*/
			if ((mmc_block_read_singleblock((uchar *)dst, src)) < 0) {
				return -1;
			}
	} else {
		/*This is for multi block read*/
		if (mmc_block_read_multiblock((uchar*)dst, src, blkcnt) < 0) {
			return -1;
		}
	}
	return blkcnt;
}


ulong
/****************************************************/
mmc_bwrite(int dev_num, ulong blknr, ulong blkcnt, ulong *src)
/****************************************************/
{
	ulong dst = 0;
	//ulong dst_size = 0;
	//ulong SDCard_Size = SDDevInfo->SDCard_Size;
	dst = SDDevInfo->CardCapacityStatus ? blknr : (blknr*MMC_BLOCK_SIZE);

	/* count SD size by block based, a block = 512 bytes */
	//dst_size = SDDevInfo->CardCapacityStatus ? dst : (dst/MMC_BLOCK_SIZE); 
	//printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
	//printf("[mmc_bwrite] dst 0x%x src 0x%x dev_num 0x%x, blknr 0x%x, blkcnt 0x%x \n", dst, src, dev_num, blknr, blkcnt);
	if(!blkcnt)
		return blkcnt;

	/*select host*/
	Change_SD_host(dev_num);

	if (!mmc_ready) {
		printf("Please initial the MMC first\n");
		return -1;
	}
	//if(dst_size > SDCard_Size) {  /*if write address > SDCard size , the part_offset is error */ 
	//	printf("mmc_bwrite part_offset error\n");
	//	return -1;
    //}
	if (blkcnt ==1) {
		/*This is for single block read*/
			if ((mmc_block_write_singleblock((uchar *)src, dst)) < 0) {
				return -1;
			}
	} else {
		/*This is for multi block read*/
		if (mmc_block_write_multiblock((uchar*)src, dst, blkcnt) < 0) {
			return -1;
		}
	}
	return blkcnt;
}

int sd_set_clock(int Frequence)
{

	if (Frequence == SD_Clk_Auto) {
		/*	Check SD Card	*/
		if (SDDevInfo->SD_Card == SD_TRUE) {
			if (SDDevInfo->HighSpeedSupport == SD_TRUE)
				Frequence = 6;	/*	50Mhz	*/
			else
				Frequence = 3;	/*	25Mhz	*/
		}

		/*	Check MMC Card	*/
		if (SDDevInfo->MMC_Card == SD_TRUE) {
			if (SDDevInfo->MMCMaxClockRate == Clk_48)
				Frequence = 6;	/*	50Mhz	*/
			else if (SDDevInfo->MMCMaxClockRate == Clk_24)
				Frequence = 3;	/*	25Mhz	*/
			else if (SDDevInfo->MMCMaxClockRate == Clk_15)
				Frequence = 1;	/*	15Mhz	*/
		}
	}

	switch (Frequence) {
	case SD_Clk_Auto:
		break;
	case SD_Clk_15MHz:
		pSd_Reg->extctl &= ~0x80;  /*  Disable High Speed timing */
		if (SDDevInfo->CtrlID == 0) {
			auto_pll_divisor(DEV_SDMMC0, SET_DIV, 2, 15*2);
			SDPRINTK("ATSMB Host0 15MHz \n");
        }
		if (SDDevInfo->CtrlID == 1) {
			auto_pll_divisor(DEV_SDMMC1, SET_DIV, 2, 15*2);
			SDPRINTK("ATSMB Host1 15MHz \n");
		}
		if (SDDevInfo->CtrlID == 2) {
			auto_pll_divisor(DEV_SDMMC2, SET_DIV, 2, 15*2);
			SDPRINTK("ATSMB Host2 15MHz \n");
		}
		break;
	case SD_Clk_20MHz:
		pSd_Reg->extctl &= ~0x80;  /*  Disable High Speed timing */
		if (SDDevInfo->CtrlID == 0) {
			auto_pll_divisor(DEV_SDMMC0, SET_DIV, 2, 20*2);
			SDPRINTK("ATSMB Host0 20MHz \n");
        }
		if (SDDevInfo->CtrlID == 1) {
			auto_pll_divisor(DEV_SDMMC1, SET_DIV, 2, 20*2);
			SDPRINTK("ATSMB Host1 20MHz \n");
		}
		if (SDDevInfo->CtrlID == 2) {
			auto_pll_divisor(DEV_SDMMC2, SET_DIV, 2, 20*2);
			SDPRINTK("ATSMB Host2 20MHz \n");
		}
		break;
	case SD_Clk_25MHz:
		pSd_Reg->extctl &= ~0x80;  /*  Disable High Speed timing */
		if (SDDevInfo->CtrlID == 0) {
			auto_pll_divisor(DEV_SDMMC0, SET_DIV, 2, 24*2);
			SDPRINTK("ATSMB Host0 25MHz \n");
        }
		if (SDDevInfo->CtrlID == 1) {
			auto_pll_divisor(DEV_SDMMC1, SET_DIV, 2, 24*2);
			SDPRINTK("ATSMB Host1 25MHz \n");
		}
		if (SDDevInfo->CtrlID == 2) {
			auto_pll_divisor(DEV_SDMMC2, SET_DIV, 2, 24*2);
			SDPRINTK("ATSMB Host2 25MHz \n");
		}
		break;
	case SD_Clk_33MHz:
		if (SDDevInfo->SD_Card == SD_TRUE) {
			pSd_Reg->extctl |= 0x80;   /*  Enable High Speed timing */
		}
		if (SDDevInfo->MMC_Card == SD_TRUE) {
			pSd_Reg->extctl &= ~0x80;   /*  Disable High Speed timing */
		}
		if (SDDevInfo->CtrlID == 0) {
			auto_pll_divisor(DEV_SDMMC0, SET_DIV, 2, 33*2);
			SDPRINTK("ATSMB Host0 33MHz \n");
        }
		if (SDDevInfo->CtrlID == 1) {
			auto_pll_divisor(DEV_SDMMC1, SET_DIV, 2, 33*2);
			SDPRINTK("ATSMB Host1 33MHz \n");
		}
		if (SDDevInfo->CtrlID == 2) {
			auto_pll_divisor(DEV_SDMMC2, SET_DIV, 2, 33*2);
			SDPRINTK("ATSMB Host2 33MHz \n");
		}
		break;
	case SD_Clk_40MHz:
		if (SDDevInfo->SD_Card == SD_TRUE) {
			pSd_Reg->extctl |= 0x80;   /*  Enable High Speed timing */
		}
		if (SDDevInfo->MMC_Card == SD_TRUE) {
			pSd_Reg->extctl &= ~0x80;   /*  Disable High Speed timing */
		}
		if (SDDevInfo->CtrlID == 0) {
			auto_pll_divisor(DEV_SDMMC0, SET_DIV, 2, 40*2);
			SDPRINTK("ATSMB Host0 40MHz \n");
        }
		if (SDDevInfo->CtrlID == 1) {
			auto_pll_divisor(DEV_SDMMC1, SET_DIV, 2, 40*2);
			SDPRINTK("ATSMB Host1 40MHz \n");
		}
		if (SDDevInfo->CtrlID == 2) {
			auto_pll_divisor(DEV_SDMMC2, SET_DIV, 2, 40*2);
			SDPRINTK("ATSMB Host2 40MHz \n");
		}
		break;
    case SD_Clk_44MHz:
		if (SDDevInfo->SD_Card == SD_TRUE) {
			pSd_Reg->extctl |= 0x80;   /*  Enable High Speed timing */
		}
		if (SDDevInfo->MMC_Card == SD_TRUE) {
			pSd_Reg->extctl &= ~0x80;   /*  Disable High Speed timing */
		}
		if (SDDevInfo->CtrlID == 0) {
			auto_pll_divisor(DEV_SDMMC0, SET_DIV, 2, 44*2);
			SDPRINTK("ATSMB Host0 44MHz \n");
        }
		if (SDDevInfo->CtrlID == 1) {
			auto_pll_divisor(DEV_SDMMC1, SET_DIV, 2, 44*2);
			SDPRINTK("ATSMB Host1 44MHz \n");
		}
		if (SDDevInfo->CtrlID == 2) {
			auto_pll_divisor(DEV_SDMMC2, SET_DIV, 2, 44*2);
			SDPRINTK("ATSMB Host2 44MHz \n");
		}
		break;
	case SD_Clk_50MHz:
		if (SDDevInfo->SD_Card == SD_TRUE) {
			pSd_Reg->extctl |= 0x80;   /*  Enable High Speed timing */
		}
		if (SDDevInfo->MMC_Card == SD_TRUE) {
			pSd_Reg->extctl &= ~0x80;   /*  Disable High Speed timing */
		}
		if (SDDevInfo->CtrlID == 0) {
			auto_pll_divisor(DEV_SDMMC0, SET_DIV, 2, 50*2);
			SDPRINTK("ATSMB Host0 50MHz \n");
        }
		if (SDDevInfo->CtrlID == 1) {
			auto_pll_divisor(DEV_SDMMC1, SET_DIV, 2, 50*2);
			SDPRINTK("ATSMB Host1 50MHz \n");
		}
		if (SDDevInfo->CtrlID == 2) {
			auto_pll_divisor(DEV_SDMMC2, SET_DIV, 2, 50*2);
			SDPRINTK("ATSMB Host2 50MHz \n");
		}
		break;
	case SD_Clk_400KHz:
		pSd_Reg->extctl &= ~0x80;  /*  Disable High Speed timing */
		if (SDDevInfo->CtrlID == 0) {
			auto_pll_divisor(DEV_SDMMC0, SET_DIV, 1, 400*2);
			SDPRINTK("ATSMB Host0 400KHz \n");
        }
		if (SDDevInfo->CtrlID == 1) {
			auto_pll_divisor(DEV_SDMMC1, SET_DIV, 1, 400*2);
			SDPRINTK("ATSMB Host1 400KHz \n");
		}
		if (SDDevInfo->CtrlID == 2) {
			auto_pll_divisor(DEV_SDMMC2, SET_DIV, 1, 400*2);
			SDPRINTK("ATSMB Host2 400KHz \n");
		}
		break;
	default:
		break;
	}

	return 0;
}

int
mmc_init(int verbose, int device_num)
/****************************************************/
{
	unsigned long sd_status;


	int Fre = 3;//3
	
	get_chip_version();
	if (chip_id == 0x3437 || 
        chip_id == 0x3429 || 
        chip_id == 0x3451) {
		if (device_num > 1)
			device_num = 1;
		if (device_num < 0)
			device_num = 0;
	} else if (chip_id == 0x3445){
		if (device_num > 2)
			device_num = 2;
		if (device_num < 0)
			device_num = 0;
	} else {
		device_num = 0;
	}
	
    if (device_num == 0)
        auto_pll_divisor(DEV_SDMMC0, CLK_ENABLE, 0, 0);
	if (device_num == 1)
        auto_pll_divisor(DEV_SDMMC1, CLK_ENABLE, 0, 0);
	if (device_num == 2)
        auto_pll_divisor(DEV_SDMMC2, CLK_ENABLE, 0, 0);
    
	Change_SD_host(device_num);
	memset(SDDevInfo, 0x0, sizeof(sd_info_t));
	SDDevInfo->CtrlID = (char)device_num;
	
    SD_Controller_Powerup();
	
	sd_status=SD_Initialization() ; // for Emul.
	if (sd_status != 0 )
       		return -1 ;
	Fre = SDDevInfo->MMCMaxClockRate;
	switch( Fre )
	{
		case SD_Clk_400KHz:
			sd_set_clock(SD_Clk_400KHz);      //400Khz
			printf( "SD/MMC clock is 400Khz\n");
			break;
		case SD_Clk_15MHz:
			sd_set_clock(SD_Clk_15MHz);      //15Mhz
			printf( "SD/MMC clock is 15Mhz\n");
			break;
		case SD_Clk_25MHz:
			sd_set_clock(SD_Clk_25MHz);     //25Mhz
			printf( "SD/MMC clock is 25Mhz\n");
			break;
		case SD_Clk_44MHz:
			sd_set_clock(SD_Clk_44MHz);     //25Mhz
			printf( "SD/MMC clock is 44Mhz\n");
			break;
		case SD_Clk_50MHz:
			sd_set_clock(SD_Clk_50MHz);      //48Mhz
			printf( "SD/MMC clock is 50Mhz\n");
			break;
		default:
			break;

	}

	// set normal speed bus mode
 	//pSd_Reg->extctl &= 0x7F;
	
	pSd_Reg->blkcnt = 0x01 ;
	pSd_Reg->ctlr |= 0x08 ;   // response FIFO reset,


	mmc_dev->if_type = IF_TYPE_MMC;
	mmc_dev->part_type = PART_TYPE_DOS;
	mmc_dev->dev = device_num;
	mmc_dev->lun = 0;
	mmc_dev->type = 0;
	/* FIXME fill in the correct size (is set to 32MByte) */
	mmc_dev->blksz = 512;
	mmc_dev->lba = SDDevInfo->SDCard_Size;
	mmc_dev->removable = 0;
	mmc_dev->block_read = mmc_bread;
	mmc_dev->block_write = mmc_bwrite;

	if (device_num == 0)
		mmc0_ready = 1;
	if (device_num == 1)
		mmc1_ready = 1;
	if (device_num == 2)
		mmc2_ready = 1;
	mmc_ready = 1;
	
    printf( "register mmc device\n");
	SDPRINTK( "SDCard_Size  = %ld MB\n",SDDevInfo->SDCard_Size / (2*1024));
	fat_register_device(mmc_dev,1); /* partitions start counting with 1 */

	return 0;
}

int
mmc_ident(block_dev_desc_t *dev)
{
	return 0;
}

int
mmc2info(ulong addr)
{
	/* FIXME hard codes to 32 MB device */
	if (addr >= CFG_MMC_BASE && addr < CFG_MMC_BASE + 0x02000000) {
		return 1;
	}
	return 0;
}

unsigned int get_chip_version(void)
{
	static unsigned int chip_version = 0;
	unsigned int val;

	if( chip_version )
		return chip_version;

    chip_id = *(unsigned short*) 0xD8120002;
	switch( chip_id ){
		case 0x3357:
			val = *(unsigned int*)(0xd8120700);
			*(unsigned int*)(0xd8120700) = val;

			val = *(unsigned int*)(0xd8120700);
			chip_version = ((0xA + ((val & 0x0F) - 0x02)) << 4) + ((val & 0xF0) >> 4);
			break;
		case 0x3400:
			val = *(unsigned int*) 0xD8120000;
			chip_version = ((val & 0xF00) >> 4) + 0x90 + ((val & 0xFF) - 1);
			break;
		case 0x3426:
			pSd_Reg = (PWMT_SDMMC_REG) BA_SDC;
			pSd_PDma_Reg = (struct _SD_PDMA_REG_ *) BA_SDCDMA;
			val = *(unsigned int*) 0xD8120000;
			chip_version = ((val & 0xF00) >> 4) + 0x90 + ((val & 0xFF) - 1);
			break;
		default:
			val = *(unsigned int*) 0xD8120000;
			chip_version = ((val & 0xF00) >> 4) + 0x90 + ((val & 0xFF) - 1);
			break;
	}
	chip_version = (chip_id << 16) | chip_version;
	return chip_version;
}



void SD_Controller_Powerup(void)
{

#define GPIO_PIN_Sharing				REG32_PTR(GPIO_BASE_ADDR + 0x200)
	
	
/*SD0*/
#define GPIO_INPUT_DATA_SDCardDetect	REG8_PTR(GPIO_BASE_ADDR + 0x003)
#define GPIO_INPUT_DATA_SD				REG8_PTR(GPIO_BASE_ADDR + 0x00E)
//#define GPIO_INPUT_DATA_SD0PWRSW		REG8_PTR(GPIO_BASE_ADDR + 0x012)
	
#define GPIO_Enable_SDCardDetect		REG8_PTR(GPIO_BASE_ADDR + 0x043)
#define GPIO_Enable_SD					REG8_PTR(GPIO_BASE_ADDR + 0x04E)
#define GPIO_Enable_SD0PWRSW			REG8_PTR(GPIO_BASE_ADDR + 0x052)
	
#define GPIO_OUTPUT_Enable_SDCardDetect	REG8_PTR(GPIO_BASE_ADDR + 0x083)
#define GPIO_OUTPUT_Enable_SD			REG8_PTR(GPIO_BASE_ADDR + 0x08E)
#define GPIO_OUTPUT_Enable_SD0PWRSW		REG8_PTR(GPIO_BASE_ADDR + 0x092)
	
#define GPIO_OUTPUT_DATA_SDCardDetect	REG8_PTR(GPIO_BASE_ADDR + 0x0C3)
#define GPIO_OUTPUT_DATA_SD				REG8_PTR(GPIO_BASE_ADDR + 0x0CE)
#define GPIO_OUTPUT_DATA_SD0PWRSW		REG8_PTR(GPIO_BASE_ADDR + 0x0D2)
	
#define GPIO_PULL_Enable_SDCardDetect	REG8_PTR(GPIO_BASE_ADDR + 0x483)
#define GPIO_PULL_Enable_SD				REG8_PTR(GPIO_BASE_ADDR + 0x48E)
#define GPIO_PULL_Enable_SD0PWRSW		REG8_PTR(GPIO_BASE_ADDR + 0x492)
	
#define GPIO_PULL_Control_SDCardDetect	REG8_PTR(GPIO_BASE_ADDR + 0x4C3)
#define GPIO_PULL_Control_SD			REG8_PTR(GPIO_BASE_ADDR + 0x4CE)
#define GPIO_PULL_Control_SD0PWRSW 		REG8_PTR(GPIO_BASE_ADDR + 0x4D0)


/*SD1*/
#define GPIO_INPUT_DATA_SD1Data			REG8_PTR(GPIO_BASE_ADDR + 0x00F)
#define GPIO_INPUT_DATA_SD1CMD			REG8_PTR(GPIO_BASE_ADDR + 0x01C)
#define GPIO_INPUT_DATA_SD1CLK			REG8_PTR(GPIO_BASE_ADDR + 0x01C)
#define GPIO_INPUT_DATA_SD1CardDetect	REG8_PTR(GPIO_BASE_ADDR + 0x01D)
#define GPIO_INPUT_DATA_SD1WP			REG8_PTR(GPIO_BASE_ADDR + 0x01D)
#define GPIO_INPUT_DATA_SD1PWRSW		REG8_PTR(GPIO_BASE_ADDR + 0x01D)
#define GPIO_INPUT_DATA_SD1RSTN			REG8_PTR(GPIO_BASE_ADDR + 0x01D)

#define GPIO_Enable_SD1Data			REG8_PTR(GPIO_BASE_ADDR + 0x04F)
#define GPIO_Enable_SD1CMD			REG8_PTR(GPIO_BASE_ADDR + 0x05C)
#define GPIO_Enable_SD1CLK			REG8_PTR(GPIO_BASE_ADDR + 0x05C)
#define GPIO_Enable_SD1CardDetect	REG8_PTR(GPIO_BASE_ADDR + 0x05D)
#define GPIO_Enable_SD1WP			REG8_PTR(GPIO_BASE_ADDR + 0x05D)
#define GPIO_Enable_SD1PWRSW		REG8_PTR(GPIO_BASE_ADDR + 0x05D)
#define GPIO_Enable_SD1RSTN			REG8_PTR(GPIO_BASE_ADDR + 0x05D)

#define GPIO_OUTPUT_Enable_SD1Data			REG8_PTR(GPIO_BASE_ADDR + 0x08F)
#define GPIO_OUTPUT_Enable_SD1CMD			REG8_PTR(GPIO_BASE_ADDR + 0x09C)
#define GPIO_OUTPUT_Enable_SD1CLK			REG8_PTR(GPIO_BASE_ADDR + 0x09C)
#define GPIO_OUTPUT_Enable_SD1CardDetect	REG8_PTR(GPIO_BASE_ADDR + 0x09D)
#define GPIO_OUTPUT_Enable_SD1WP			REG8_PTR(GPIO_BASE_ADDR + 0x09D)
#define GPIO_OUTPUT_Enable_SD1PWRSW			REG8_PTR(GPIO_BASE_ADDR + 0x09D)
#define GPIO_OUTPUT_Enable_SD1RSTN			REG8_PTR(GPIO_BASE_ADDR + 0x09D)

#define GPIO_OUTPUT_DATA_SD1Data		REG8_PTR(GPIO_BASE_ADDR + 0x0CF)
#define GPIO_OUTPUT_DATA_SD1CMD			REG8_PTR(GPIO_BASE_ADDR + 0x0DC)
#define GPIO_OUTPUT_DATA_SD1CLK			REG8_PTR(GPIO_BASE_ADDR + 0x0DC)
#define GPIO_OUTPUT_DATA_SD1CardDetect	REG8_PTR(GPIO_BASE_ADDR + 0x0DD)
#define GPIO_OUTPUT_DATA_SD1WP			REG8_PTR(GPIO_BASE_ADDR + 0x0DD)
#define GPIO_OUTPUT_DATA_SD1PWRSW		REG8_PTR(GPIO_BASE_ADDR + 0x0DD)
#define GPIO_OUTPUT_DATA_SD1RSTN		REG8_PTR(GPIO_BASE_ADDR + 0x0DD)

#define GPIO_PULL_Enable_SD1Data		REG8_PTR(GPIO_BASE_ADDR + 0x48F)
#define GPIO_PULL_Enable_SD1CMD			REG8_PTR(GPIO_BASE_ADDR + 0x49C)
#define GPIO_PULL_Enable_SD1CLK			REG8_PTR(GPIO_BASE_ADDR + 0x49C)
#define GPIO_PULL_Enable_SD1CardDetect	REG8_PTR(GPIO_BASE_ADDR + 0x49D)
#define GPIO_PULL_Enable_SD1WP			REG8_PTR(GPIO_BASE_ADDR + 0x49D)
#define GPIO_PULL_Enable_SD1PWRSW		REG8_PTR(GPIO_BASE_ADDR + 0x49D)
#define GPIO_PULL_Enable_SD1RSTN		REG8_PTR(GPIO_BASE_ADDR + 0x49D)

#define GPIO_PULL_Control_SD1Data		REG8_PTR(GPIO_BASE_ADDR + 0x4CF)
#define GPIO_PULL_Control_SD1CMD		REG8_PTR(GPIO_BASE_ADDR + 0x4DC)
#define GPIO_PULL_Control_SD1CLK		REG8_PTR(GPIO_BASE_ADDR + 0x4DC)
#define GPIO_PULL_Control_SD1CardDetect	REG8_PTR(GPIO_BASE_ADDR + 0x4DD)
#define GPIO_PULL_Control_SD1WP			REG8_PTR(GPIO_BASE_ADDR + 0x4DD)
#define GPIO_PULL_Control_SD1PWRSW		REG8_PTR(GPIO_BASE_ADDR + 0x4DD)
#define GPIO_PULL_Control_SD1RSTN		REG8_PTR(GPIO_BASE_ADDR + 0x4DD)


/*SD2*/
#define GPIO_INPUT_DATA_SD2Data			REG8_PTR(GPIO_BASE_ADDR + 0x018)
#define GPIO_INPUT_DATA_SD2CMD			REG8_PTR(GPIO_BASE_ADDR + 0x018)
#define GPIO_INPUT_DATA_SD2CLK			REG8_PTR(GPIO_BASE_ADDR + 0x018)
#define GPIO_INPUT_DATA_SD2CardDetect	REG8_PTR(GPIO_BASE_ADDR + 0x018)
#define GPIO_INPUT_DATA_SD2WP			REG8_PTR(GPIO_BASE_ADDR + 0x018)
#define GPIO_INPUT_DATA_SD2PWRSW		REG8_PTR(GPIO_BASE_ADDR + 0x019)

#define GPIO_Enable_SD2Data			REG8_PTR(GPIO_BASE_ADDR + 0x058)
#define GPIO_Enable_SD2CMD			REG8_PTR(GPIO_BASE_ADDR + 0x058)
#define GPIO_Enable_SD2CLK			REG8_PTR(GPIO_BASE_ADDR + 0x058)
#define GPIO_Enable_SD2CardDetect	REG8_PTR(GPIO_BASE_ADDR + 0x058)
#define GPIO_Enable_SD2WP			REG8_PTR(GPIO_BASE_ADDR + 0x058)
#define GPIO_Enable_SD2PWRSW		REG8_PTR(GPIO_BASE_ADDR + 0x059)

#define GPIO_OUTPUT_Enable_SD2Data			REG8_PTR(GPIO_BASE_ADDR + 0x098)
#define GPIO_OUTPUT_Enable_SD2CMD			REG8_PTR(GPIO_BASE_ADDR + 0x098)
#define GPIO_OUTPUT_Enable_SD2CLK			REG8_PTR(GPIO_BASE_ADDR + 0x098)
#define GPIO_OUTPUT_Enable_SD2CardDetect	REG8_PTR(GPIO_BASE_ADDR + 0x098)
#define GPIO_OUTPUT_Enable_SD2WP			REG8_PTR(GPIO_BASE_ADDR + 0x098)
#define GPIO_OUTPUT_Enable_SD2PWRSW			REG8_PTR(GPIO_BASE_ADDR + 0x099)

#define GPIO_OUTPUT_DATA_SD2Data		REG8_PTR(GPIO_BASE_ADDR + 0x0D8)
#define GPIO_OUTPUT_DATA_SD2CMD			REG8_PTR(GPIO_BASE_ADDR + 0x0D8)
#define GPIO_OUTPUT_DATA_SD2CLK			REG8_PTR(GPIO_BASE_ADDR + 0x0D8)
#define GPIO_OUTPUT_DATA_SD2CardDetect	REG8_PTR(GPIO_BASE_ADDR + 0x0D8)
#define GPIO_OUTPUT_DATA_SD2WP			REG8_PTR(GPIO_BASE_ADDR + 0x0D8)
#define GPIO_OUTPUT_DATA_SD2PWRSW		REG8_PTR(GPIO_BASE_ADDR + 0x0D9)

#define GPIO_PULL_Enable_SD2Data		REG8_PTR(GPIO_BASE_ADDR + 0x498)
#define GPIO_PULL_Enable_SD2CMD			REG8_PTR(GPIO_BASE_ADDR + 0x498)
#define GPIO_PULL_Enable_SD2CLK			REG8_PTR(GPIO_BASE_ADDR + 0x498)
#define GPIO_PULL_Enable_SD2CardDetect	REG8_PTR(GPIO_BASE_ADDR + 0x498)
#define GPIO_PULL_Enable_SD2WP			REG8_PTR(GPIO_BASE_ADDR + 0x498)
#define GPIO_PULL_Enable_SD2PWRSW		REG8_PTR(GPIO_BASE_ADDR + 0x499)

#define GPIO_PULL_Control_SD2Data		REG8_PTR(GPIO_BASE_ADDR + 0x4D8)
#define GPIO_PULL_Control_SD2CMD		REG8_PTR(GPIO_BASE_ADDR + 0x4D8)
#define GPIO_PULL_Control_SD2CLK		REG8_PTR(GPIO_BASE_ADDR + 0x4D8)
#define GPIO_PULL_Control_SD2CardDetect	REG8_PTR(GPIO_BASE_ADDR + 0x4D8)
#define GPIO_PULL_Control_SD2WP			REG8_PTR(GPIO_BASE_ADDR + 0x4D8)
#define GPIO_PULL_Control_SD2PWRSW		REG8_PTR(GPIO_BASE_ADDR + 0x4D9)


   
/* SD pin */
#define GPIO_SD0_CD				BIT4
#define	GPIO_SD0_Data			(BIT7 | BIT6 | BIT5 | BIT4)
#define	GPIO_SD0_WriteProtect	BIT3
#define	GPIO_SD0_Command		BIT2
#define	GPIO_SD0_Clock			BIT1
#define	GPIO_SD0_POWER			BIT5


#define	GPIO_SD1_Data			(BIT7 | BIT6 | BIT5 | BIT4 | BIT3 | BIT2 | BIT1 | BIT0)
#define	GPIO_SD1_Command		BIT1
#define	GPIO_SD1_Clock			BIT0
#define GPIO_SD1_CD				BIT4
#define	GPIO_SD1_WriteProtect	BIT3
#define	GPIO_SD1_POWER			BIT2
#define GPIO_SD1_RSTN			BIT1

#define	GPIO_SD2_Data			(BIT2 | BIT5 | BIT4 | BIT1)
#define	GPIO_SD2_Command		BIT6
#define	GPIO_SD2_Clock			BIT7
#define GPIO_SD2_CD				BIT0
#define	GPIO_SD2_WriteProtect	BIT3
#define	GPIO_SD2_POWER			BIT1

/* PIN share switch */
#define GPIO_SD0_PinShare		BIT8
#define GPIO_SD1_PinShare		BIT1
#define GPIO_SD2_PinShare		BIT6

#define SD0_CARD_PWR	BIT1
#define SD1_CARD_PWR	BIT2
#define SD2_CARD_PWR	BIT3


	unsigned long SD0_PIN, SD1_PIN;
 
	SD0_PIN = ( GPIO_SD0_Clock | GPIO_SD0_Command | GPIO_SD0_WriteProtect | GPIO_SD0_Data);
	
    
	/*	 set normal speed bus mode	*/
	pSd_Reg->extctl= 0x0 ;
	pSd_Reg->busm = 0x0 ;   /*	 response FIFO reset,	*/
		
	/*  disable SD Card power  */
	if (SDDevInfo->CtrlID == 0) {
		/*set SD0 power pin as GPO pin*/		
		*GPIO_Enable_SD0PWRSW |= GPIO_SD0_POWER; /*Set power pin as GPIO pin*/
		*GPIO_OUTPUT_Enable_SD0PWRSW |= GPIO_SD0_POWER; /*Set output pin*/

		/*set internal pull up*/
		*GPIO_PULL_Control_SD0PWRSW |= GPIO_SD0_POWER;

		/*set internal pull enable*/
		*GPIO_PULL_Enable_SD0PWRSW |= GPIO_SD0_POWER;

		/*disable SD0 power*/
		*GPIO_OUTPUT_DATA_SD0PWRSW |= GPIO_SD0_POWER;
	} else if (SDDevInfo->CtrlID == 1) {
		/*set SD1 power pin as GPO pin*/
		*GPIO_Enable_SD1PWRSW |= GPIO_SD1_POWER; /*Set power pin as GPIO pin*/
		*GPIO_OUTPUT_Enable_SD1PWRSW |= GPIO_SD1_POWER; /*Set output pin*/

		/*set internal pull up*/
		*GPIO_PULL_Control_SD1PWRSW |= GPIO_SD1_POWER;

		/*set internal pull enable*/
		*GPIO_PULL_Enable_SD1PWRSW |= GPIO_SD1_POWER;

		/*disable SD1 power*/
		*GPIO_OUTPUT_DATA_SD1PWRSW |= GPIO_SD1_POWER;
	} else if (SDDevInfo->CtrlID == 2) {
		/*set SD2 power pin as GPO pin*/
		*GPIO_Enable_SD2PWRSW |= GPIO_SD2_POWER; /*Set power pin as GPIO pin*/
		*GPIO_OUTPUT_Enable_SD2PWRSW |= GPIO_SD2_POWER; /*Set output pin*/

		/*set internal pull up*/
		*GPIO_PULL_Control_SD2PWRSW |= GPIO_SD2_POWER;

		/*set internal pull enable*/
		*GPIO_PULL_Enable_SD2PWRSW |= GPIO_SD2_POWER;

		/*disable SD2 power*/
		*GPIO_OUTPUT_DATA_SD2PWRSW |= GPIO_SD2_POWER;
	}

		
	/*  Config SD PIN share  */
	if (SDDevInfo->CtrlID == 0) {
		*GPIO_PIN_Sharing &= ~GPIO_SD0_PinShare;
	} else if (SDDevInfo->CtrlID == 1) {	
		*GPIO_PIN_Sharing |= GPIO_SD1_PinShare;
	} else if (SDDevInfo->CtrlID == 2) {
		*GPIO_PIN_Sharing |= GPIO_SD2_PinShare;
	}
	/* do not config GPIO_SD1_CD because ISR has already rum,	* config card detect will issue ISR storm.			 */

	/*  Config SD to GPIO  */
	if (SDDevInfo->CtrlID == 0) {
		*GPIO_Enable_SD = SD0_PIN;
	    *GPIO_Enable_SDCardDetect |=  GPIO_SD0_CD;
	} else if (SDDevInfo->CtrlID == 1) {
		*GPIO_Enable_SD1Data |= GPIO_SD1_Data;
		*GPIO_Enable_SD1CMD |= GPIO_SD1_Command;
		*GPIO_Enable_SD1CLK |= GPIO_SD1_Clock;
	    *GPIO_Enable_SD1CardDetect |= GPIO_SD1_CD;
		*GPIO_Enable_SD1WP |= GPIO_SD1_WriteProtect;
		*GPIO_Enable_SD1RSTN |= GPIO_SD1_RSTN;
	} else if (SDDevInfo->CtrlID == 2) {
		*GPIO_Enable_SD2Data |= GPIO_SD2_Data;
		*GPIO_Enable_SD2CMD |= GPIO_SD2_Command;
		*GPIO_Enable_SD2CLK |= GPIO_SD2_Clock;
	    *GPIO_Enable_SD2CardDetect |= GPIO_SD2_CD;
		*GPIO_Enable_SD2WP |= GPIO_SD2_WriteProtect;
	}


	/*  SD all pins output low  */
	if (SDDevInfo->CtrlID == 0) {
		*GPIO_OUTPUT_DATA_SD &= ~SD0_PIN;
        *GPIO_OUTPUT_DATA_SDCardDetect &= ~GPIO_SD0_CD;
	} else if (SDDevInfo->CtrlID == 1) {
		*GPIO_OUTPUT_DATA_SD1Data &= ~GPIO_SD1_Data;
		*GPIO_OUTPUT_DATA_SD1CMD &= ~GPIO_SD1_Command;
		*GPIO_OUTPUT_DATA_SD1CLK &= ~GPIO_SD1_Clock;
        *GPIO_OUTPUT_DATA_SD1CardDetect &= ~GPIO_SD1_CD;
		*GPIO_OUTPUT_DATA_SD1WP &= ~GPIO_SD1_WriteProtect;
		*GPIO_OUTPUT_DATA_SD1RSTN &= ~GPIO_SD1_RSTN;
	} else if (SDDevInfo->CtrlID == 2) {
		*GPIO_OUTPUT_DATA_SD2Data &= ~GPIO_SD2_Data;
		*GPIO_OUTPUT_DATA_SD2CMD &= ~GPIO_SD2_Command;
		*GPIO_OUTPUT_DATA_SD2CLK &= ~GPIO_SD2_Clock;
        *GPIO_OUTPUT_DATA_SD2CardDetect &= ~GPIO_SD2_CD;
		*GPIO_OUTPUT_DATA_SD2WP &= ~GPIO_SD2_WriteProtect;
	}
	

	/*  Config SD to GPO   */	
	if (SDDevInfo->CtrlID == 0) {
		*GPIO_OUTPUT_Enable_SD |= SD0_PIN;
        *GPIO_OUTPUT_Enable_SDCardDetect |= GPIO_SD0_CD;
	} else if (SDDevInfo->CtrlID == 1) {
		*GPIO_OUTPUT_Enable_SD1Data |= GPIO_SD1_Data;
		*GPIO_OUTPUT_Enable_SD1CMD |= GPIO_SD1_Command;
		*GPIO_OUTPUT_Enable_SD1CLK |= GPIO_SD1_Clock;
	    *GPIO_OUTPUT_Enable_SD1CardDetect |= GPIO_SD1_CD;
		*GPIO_OUTPUT_Enable_SD1WP |= GPIO_SD1_WriteProtect;
		*GPIO_OUTPUT_Enable_SD1RSTN |= GPIO_SD1_RSTN;
	} else if (SDDevInfo->CtrlID == 2) {
		*GPIO_OUTPUT_Enable_SD2Data |= GPIO_SD2_Data;
		*GPIO_OUTPUT_Enable_SD2CMD |= GPIO_SD2_Command;
		*GPIO_OUTPUT_Enable_SD2CLK |= GPIO_SD2_Clock;
	    *GPIO_OUTPUT_Enable_SD2CardDetect |= GPIO_SD2_CD;
		*GPIO_OUTPUT_Enable_SD2WP |= GPIO_SD2_WriteProtect;
	}		

	/* stop SD output clock */	
	pSd_Reg->busm &= ~BIT4;
	udelay(100000);
		
	/* Pull up/down resister of SD Bus */
	if (SDDevInfo->CtrlID == 0) {
		/*Disable Clock & CMD Pull enable*/
        *GPIO_PULL_Enable_SD &= ~( GPIO_SD0_Clock | GPIO_SD0_Command);

		/*Set CD ,WP ,DATA pin pull up*/
		*GPIO_PULL_Control_SDCardDetect |= GPIO_SD0_CD;
	    *GPIO_PULL_Control_SD |= ( SD0_PIN ^ GPIO_SD0_Clock );
		
		/*Enable CD ,WP ,DATA  internal pull*/
		*GPIO_PULL_Enable_SDCardDetect |= GPIO_SD0_CD;
	    *GPIO_PULL_Enable_SD |= ( SD0_PIN ^ ( GPIO_SD0_Clock | GPIO_SD0_Command ));
        
	} else if (SDDevInfo->CtrlID == 1) {
		/*Disable Clock & CMD Pull enable*/
		*GPIO_PULL_Enable_SD1CLK &= ~GPIO_SD1_Clock;
		*GPIO_PULL_Enable_SD1CMD &= ~GPIO_SD1_Command;
		
		/*Set CD ,WP ,DATA pin pull up*/
	    *GPIO_PULL_Control_SD1CardDetect |= GPIO_SD1_CD;
		*GPIO_PULL_Control_SD1Data |= GPIO_SD1_Data;
		*GPIO_PULL_Control_SD1WP |= GPIO_SD1_WriteProtect;
		
		/*Enable CD ,WP ,DATA  internal pull*/
		*GPIO_PULL_Enable_SD1CardDetect |= GPIO_SD1_CD;
		*GPIO_PULL_Enable_SD1Data |= GPIO_SD1_Data;
		*GPIO_PULL_Enable_SD1WP |= GPIO_SD1_WriteProtect;
	} else if (SDDevInfo->CtrlID == 2) {
		/*Disable Clock & CMD Pull enable*/
		*GPIO_PULL_Enable_SD2CLK &= ~GPIO_SD2_Clock;
		*GPIO_PULL_Enable_SD2CMD &= ~GPIO_SD2_Command;
		
		/*Set CD ,WP ,DATA pin pull up*/
	    *GPIO_PULL_Control_SD2CardDetect |= GPIO_SD2_CD;
		*GPIO_PULL_Control_SD2Data |= GPIO_SD2_Data;
		*GPIO_PULL_Control_SD2WP |= GPIO_SD2_WriteProtect;
		
		/*Enable CD ,WP ,DATA  internal pull*/
		*GPIO_PULL_Enable_SD2CardDetect |= GPIO_SD2_CD;
		*GPIO_PULL_Enable_SD2Data |= GPIO_SD2_Data;
		*GPIO_PULL_Enable_SD2WP |= GPIO_SD2_WriteProtect;
	}
	
	udelay(10000);
	/*enable SD power*/
	if (SDDevInfo->CtrlID == 0) {
		/*Enable SD0 power*/
		*GPIO_OUTPUT_DATA_SD0PWRSW &= ~GPIO_SD0_POWER;
	} else if (SDDevInfo->CtrlID == 1) {
		/*Ensable SD1 power*/
		*GPIO_OUTPUT_DATA_SD1PWRSW &= ~GPIO_SD1_POWER;
	} else if (SDDevInfo->CtrlID == 2) {
		/*Ensable SD2 power*/
		*GPIO_OUTPUT_DATA_SD2PWRSW &= ~GPIO_SD2_POWER;
	}
	
			
	/*	issue softReset to SD controller	*/
	pSd_Reg->busm= SOFT_RESET;

	/*	 automatic clock freezing enable	*/
	pSd_Reg->str2= DIS_FORCECLK; 	

	pSd_Reg->timeval= 0xefff;
	SD_Init_PDMA();

	/*	clear card_insert in STS0[7]	*/
	pSd_Reg->str0= DEVICE_INS;

	/*	 SD clock = 400Khz	*/
	sd_set_clock(SD_Clk_400KHz);

	/* enable SD output clock */
	pSd_Reg->busm |= 0x10;
	udelay(10000);


	/* Config SD1 back to function  */
	if (SDDevInfo->CtrlID == 0) {
		*GPIO_Enable_SD &= ~( SD0_PIN );
	    *GPIO_Enable_SDCardDetect &= ~GPIO_SD0_CD;
	} else if (SDDevInfo->CtrlID == 1) {
		*GPIO_Enable_SD1Data &= ~GPIO_SD1_Data;
		*GPIO_Enable_SD1CMD &= ~GPIO_SD1_Command;
		*GPIO_Enable_SD1CLK &= ~GPIO_SD1_Clock;
	    *GPIO_Enable_SD1CardDetect &= ~GPIO_SD1_CD;
		*GPIO_Enable_SD1WP&= ~GPIO_SD1_WriteProtect;
	} else if (SDDevInfo->CtrlID == 2) {
		*GPIO_Enable_SD2Data &= ~GPIO_SD2_Data;
		*GPIO_Enable_SD2CMD &= ~GPIO_SD2_Command;
		*GPIO_Enable_SD2CLK &= ~GPIO_SD2_Clock;
	    *GPIO_Enable_SD2CardDetect &= ~GPIO_SD2_CD;
		*GPIO_Enable_SD2WP&= ~GPIO_SD2_WriteProtect;
	}
     
	/*	issue softReset to SD controller	*/
	//pSd_Reg->busm= SOFT_RESET;
	card_nop();
}


int SD_card_inserted(void)
{
	int ret;
	SD_Controller_Powerup();
	
   	sd_command(GO_IDLE_STATE,0,0x0,R0); /*reset SD Card*/
   	ret = sd_command(APP_CMD,0,0x0,R1);
	if(!ret)
		ret = sd_command(SD_APP_OP_COND,0, 0x0,R3); /*send ACMD41  to get card's require voltage range*/
	if(!ret)
		return 1; /*inserted*/
	else
		return 0; /*not inserted*/
	
	
}

#endif	/* CONFIG_MMC */
