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

#include <config.h>
#include <common.h>
#include <asm/errno.h>
#include <asm/arch/hardware.h>
#include <part.h>
#include <asm-arm/arch-wmt/common_def.h>

#include <dma.h>
#include <malloc.h>

static struct dma_state dma_status;
static struct DMA_INFO dma_chan[DMA_CHANNELS];

static struct DMA_DESCRIPT_ADDR descript_chan[DMA_CHANNELS];

/*
* [Table Description]
*	Default dma channel setting for each peripheral device
* [Arguments]
*	DeviceReq -> device dma request source
*	DefaultCCR -> default channel configuration register setting
*    Source_0/Destination_0 -> address settings for buffer 0
*	Source_1/Destination_1 -> address settings for buffer 1
*/
struct dma_device_cfg dma_device_cfg_table[] = {
/*   DeviceReq        , DefaultCCR , Source_0, Destination_0	*/
    { SPI0_DMA_TX_REQ , 0x00000100 , 0, 0 } , /*spi0*/
    { SPI0_DMA_RX_REQ , 0x00000100 , 0, 0 } , /*spi1*/
    { SPI1_DMA_TX_REQ , 0x00000100 , 0, 0 } ,
    { SPI1_DMA_RX_REQ , 0x00000100 , 0, 0 } ,
    { SPI2_DMA_TX_REQ , 0x00000100 , 0, 0 } ,
    { SPI2_DMA_RX_REQ , 0x00000100 , 0, 0 } ,
    /* start from 0, above 5, below 6 */
    { UART_0_TX_DMA_REQ , 0x00000100, 0, 0 } , /* uart2*/
    { UART_0_RX_DMA_REQ , 0x00000100, 0, 0 } , /* uart2*/
    { UART_1_TX_DMA_REQ , 0x00000100, 0, 0 } ,
    { UART_1_RX_DMA_REQ , 0x00000100, 0, 0 } ,
    { UART_2_TX_DMA_REQ , 0x00000100, 0, 0 } , /* uart1*/
    /*start from 0, above 10, below 11 */
    { UART_2_RX_DMA_REQ , 0x00000100, 0, 0 } , /* uart1*/
    { UART_3_TX_DMA_REQ , 0x00000100, 0, 0 } ,
    { UART_3_RX_DMA_REQ , 0x00000100, 0, 0 } ,
    { I2S_TX_DMA_REQ   	, 0x00000100, 0, 0 } ,
    { I2S_RX_DMA_REQ   	, 0x00000100, 0, 0 } ,
    /*start from 0, above 15, below 16 */
    { UART_4_TX_DMA_REQ , 0x00000100, 0, 0} ,
    { UART_4_RX_DMA_REQ , 0x00000100, 0, 0} ,
    { AC97_PCM_TX_DMA_REQ , 0x00000100, 0, 0} ,
    { AC97_PCM_RX_DMA_REQ , 0x00000100, 0, 0} ,
    { AC97_MIC_DMA_REQ , 0x00000100, 0, 0} ,
    /*start from 0, above 20, below 21 */
    { DEVICE_RESERVED , 0x00000100, 0, 0} ,
    { DEVICE_RESERVED , 0x00000100, 0, 0} ,
    { DEVICE_RESERVED , 0x00000100, 0, 0} ,
    { DEVICE_RESERVED , 0x00000100, 0, 0} ,
    { DEVICE_RESERVED , 0x00000100, 0, 0} ,
    { DEVICE_RESERVED , 0x00000100, 0, 0} ,
    { DEVICE_RESERVED , 0x00000100, 0, 0} ,
    { DEVICE_RESERVED , 0x00000100, 0, 0} ,
    { UART_5_TX_DMA_REQ , 0x00000100, 0, 0} ,
    { UART_5_RX_DMA_REQ , 0x00000100, 0, 0} ,
    /*start from 0, above 30, below 31 */
    { DEVICE_RESERVED , 0x00000100, 0, 0} ,
    { MEMORY_DMA_REQ , 0x2a800000, 0x0a200000	, 0x0a220000} ,
    { DEVICE_RESERVED , 0x00000100, 0, 0} ,
};

/*
* [Routine Description]
*	inital DMA
*		1. init dma_status and DMA_INFO structure for each dma channels
*		2. init dma channel registers for each channels
*		3. dma controller enable
*		it is necessary to allocate space to store 
*		memory registers and descript register.
*		So we allocate these space while initing at the first time
*/
void init_dma(void)
{
	int ch;
	struct DMA_REG *dma_regs ;
	struct DMA_MEM_REG *dma_mem_regs;

	static unsigned int first_init = 0;
	static unsigned int dma_mem_regs_base;
	dma_regs = (struct DMA_REG *)BA_DMA;
	unsigned long addr;

	printf("DMA INIT\n");

	/*
	// software initial
	*/
	dma_status.request_chans = 0 ;
	dma_status.regs = (struct DMA_REG *) BA_DMA;

	for (ch = 0 ; ch < DMA_CHANNELS ; ++ch) {
		if (first_init) {
			free((unsigned long *)dma_chan[ch].DES_ADDR.DES_0);
			free((unsigned long *)dma_chan[ch].DES_ADDR.DES_1_2);
			free_descriptstack(ch);
		}
		//des_0 = calloc(SIZE_8KB,sizeof(int));
		//des_1 = calloc(SIZE_8KB,sizeof(int));
		

		dma_chan[ch].DES_ADDR.DES_0 = descript_chan[ch].despstack_0.addr_base;
		dma_chan[ch].DES_ADDR.DES_1_2 = descript_chan[ch].despstack_1_2.addr_base;
		dma_chan[ch].channel_no = ch ;
		dma_chan[ch].regs       = (struct DMA_REG *) BA_DMA;
		dma_chan[ch].device_no  = DEVICE_RESERVED ;
		dma_chan[ch].in_use     = 0 ;
		dma_chan[ch].descript_cnt	= 0;
	}
	/*
	* hardware initial
	*/
    
	dma_status.regs->DMA_GCR |= DMA_SW_RST ;
    //REG32_VAL(0xD8001000 + 0x40) |= DMA_SW_RST;
    //REG32_VAL(0xD8001800 + 0x40) |= DMA_GLOBAL_EN;
	//*(volatile unsigned int *)BA_DMA |= DMA_SW_RST;
	

	
	dma_status.regs->DMA_GCR |= DMA_GLOBAL_EN;
	dma_status.regs->DMA_ISR = ALL_INT_CLEAR;
	dma_status.regs->DMA_IER |= ALL_INT_EN;
	dma_status.regs->DMA_TMR &= ~SCHEDULE_RR_DISABLE; /*use RR schedule*/
	
    
	if (first_init)
			free((unsigned int *)dma_mem_regs_base);
			
    
	dma_mem_regs = (struct DMA_MEM_REG *)malloc((sizeof(struct DMA_MEM_REG)+256));
	dma_mem_regs_base = (unsigned int)dma_mem_regs;
	
	addr = (unsigned long)dma_mem_regs;
	addr &= (unsigned long)0xFFFFFF00;
	addr += 256;
	dma_mem_regs = (struct DMA_MEM_REG *) addr;
	//(unsigned long)dma_mem_regs &= (unsigned long)0xFFFFFF00;
	//(unsigned long)dma_mem_regs += 256;
	dma_status.regs->DMA_MRPR = (unsigned long)dma_mem_regs;
	
    
	for (ch = 0 ; ch < DMA_CHANNELS ; ++ch) {
		dma_chan[ch].mem_regs = (struct DMA_MEM_REG *)dma_mem_regs;
		dma_mem_regs->MEM_REG_GROUP[ch].DMA_IF0CPR_CH = 0x0;
		dma_mem_regs->MEM_REG_GROUP[ch].DMA_IF1CPR_CH = 0x0;
		dma_status.regs->DMA_CCR_CH[ch] = 0x0;
	}
	
	
	if(!first_init)
		first_init++;
		
	printf("DMA INIT finish\n");

}

void disable_dma(void)
{
	dma_status.regs->DMA_GCR &= (~DMA_GLOBAL_EN);
}

void enable_dma(void)
{
	unsigned int ch = 0;
	dma_status.regs->DMA_GCR |= DMA_GLOBAL_EN;
	for (ch = 0; ch < DMA_CHANNELS; ++ch)
		dma_status.regs->DMA_CCR_CH[ch] |= SYSTEM_DMA_RUN;
}


/*
* [Routine Description]
*	request a free dma channel
*		1. search free dma channel,
*		2. update arguments to specific DMA_INFO structure
*		3. re-setting dma channel registers according to new configuration
* [Arguments]
*	channel -> interrupt number
*	device_id -> device id string
*	device -> dma_device structure
*	callback -> device isr function
* [Return]
*	0 -> success
*	-1 -> fail
*/
int request_dma(unsigned int *channel, const char *device_id, enum dma_device device)
{
	int ch;
	struct DMA_INFO *dma;
	*channel = -1;

	/*
	*  Ask for Free Channels
	*/

	for (ch = 0 ; ch < DMA_CHANNELS ; ++ch) {
		dma = &dma_chan[ch];
		if (dma->in_use == 0)
			break ;
	}

	if (ch >= DMA_CHANNELS) {
		printf("DMA : %s no free DMA channel available\n\r", device_id);
		return -1; /* EBUSY*/
	}

	/*
	* Request IRQ, all DMA Share one IRQ, so we do request only request_chans is zero
	*/
	//if (!(dma_status.request_chans & (1 << ch))) {
		//set_irq_handlers(dma_irq_mapping[ch], dma_irq_handler) ;
		//unmask_interrupt(dma_irq_mapping[ch]) ;
	//}

	dma_status.request_chans = (1 << ch) ;

	*channel           		= ch;
	dma->device_id      	= device_id;
	dma->device_no      	= device ;
	//dma->callback       	= callback;
	dma->in_use         	= 1;
	dma->descript_cnt         	= 0;

	/*
	* clear status register
	*/
	dma->regs->DMA_ISR |= 1<<ch;
	dma->mem_regs->MEM_REG_GROUP[ch].DMA_IF0CPR_CH = 0x0; /*reset descript*/
	dma->mem_regs->MEM_REG_GROUP[ch].DMA_IF1CPR_CH = 0x0; /*reset descript*/
	/*set shadow descript addr*/
	descript_chan[ch].DES_0 = dma->DES_ADDR.DES_0;
	descript_chan[ch].DES_1_2 = dma->DES_ADDR.DES_1_2;
	/*
	* setup default device
	*/
	dma->device_cfg = dma_device_cfg_table[device] ;

	/*
	*Critical 3: It should be filled with DefaultCCR
	*/
	/*Dean:To do*/
	dma->regs->DMA_CCR_CH[ch] = (dma_device_cfg_table[device].DefaultCCR & DMA_USER_SET_MASK) ;
	if (device != MEMORY_DMA_REQ)
		dma->regs->DMA_CCR_CH[ch] |= device << DMA_REQ_ID_SHIFT;


	#ifdef CONFIG_EMU
	printf("DMA : requested dma device %d\n\r", device);
	#endif /*CONFIG_EMU*/
	return 0;
}

/*
* [Routine Description]
*	clear specific dma channel register
* [Arguments]
*	ch -> channel number
*/
void clear_dma(unsigned int ch)
{
	struct DMA_INFO *dma;
	dma = &dma_chan[ch] ;

	/*
	* clear status register
	*/
	dma->regs->DMA_CCR_CH[ch] = 0x0;
	dma->regs->DMA_ISR |= 0x1<<ch;
	dma->mem_regs->MEM_REG_GROUP[ch].DMA_IF0CPR_CH = 0x0; /*clear M_0 descript address*/
	dma->mem_regs->MEM_REG_GROUP[ch].DMA_IF1CPR_CH = 0x0; /*clear M_1/2 descript address*/
	dma->descript_cnt = 0x0;
	free((unsigned long *)dma->DES_ADDR.DES_0);
	free((unsigned long *)dma->DES_ADDR.DES_1_2);
}

/*
* [Routine Description]
*	release specific dma channel
*		1. check whether free correct dma channel
*		2. clear_dma()
*		3. clear specific DMA_INFO structure and SCC register setting for dma request source
* [Arguments]
*	ch -> channel number
*/
void free_dma(unsigned int ch)
{
	struct DMA_INFO *dma;
	enum dma_device dev_no ;

	if (ch >= MAX_DMA_CHANNELS) {
		printf("DMA : bad DMA identifier\n\r");
			return ;
	}

	dma = &dma_chan[ch];
	if (dma->in_use == 0) {
		#ifdef CONFIG_EMU
		printf("DMA : trying to free free dma channe %dl\n\r", ch);
		#endif
		return;
	}

	if (dma->device_no == DEVICE_RESERVED) {
		printf("DMA : trying to free free dma %d\n\r", ch);
		return ;
	}

	clear_dma(ch);

	dma_status.request_chans &= ~(1 << ch);
	//if (dma_status.request_chans == 0)
	//	unset_irq_handlers(dma_irq_mapping[ch]);


	dev_no = dma->device_no ;
	/*if(dev_no != MEMORY_DMA_REQ)
	dma->scc_regs->DMA_REQ_CSR[ dev_no ] = ( SCC_CSR_DISABLE | SCC_CSR_SYSTEM_DMA | 0 ) ;*/
	dma->device_no  = DEVICE_RESERVED ;
	dma->device_id  = NULL ;
	dma->in_use = 0;
}

/*
* [Routine Description]
*	setup dma configure table with relative dma channel and re-setting dma channel registers
*		1. check whether setup correct dma channel
*		2. clear dma relative status register
*		3. update dma_device_cfg table
*		4. re-setting dma channel registers
* [Arguments]
*	ch -> channel number
*	device_cfg -> dma_device_cfg structure
* [Return]
*	0 -> success
*	1 -> fail
*/
int setup_dma(unsigned int ch, struct dma_device_cfg device_cfg)
{
	struct DMA_INFO *dma;
	enum dma_device dev_no ;

	dma = &dma_chan[ch];

	if (ch >= MAX_DMA_CHANNELS || (dma_chan[ch].in_use == 0)) {
		printf("DMA : bad DMA identifier\n\r");
		return -1 ; /* EINVAL*/
	}

	if (dma->device_no  != device_cfg.DeviceReqType) {
		printf("DMA : bad Device_NO\n\r");
		return -1 ;
	}

	//printf("CCR setting ch%d = %x\n", ch, device_cfg.DefaultCCR);

	dev_no = dma->device_no  ;
	dma_device_cfg_table[dev_no] = device_cfg ;
	dma->device_cfg = dma_device_cfg_table[dev_no] ;

	/*
	* clear status register
	*/
	dma->regs->DMA_ISR |= 1<<ch;
	dma->mem_regs->MEM_REG_GROUP[ch].DMA_IF0CPR_CH = 0x0; /*reset descript*/
	dma->mem_regs->MEM_REG_GROUP[ch].DMA_IF1CPR_CH = 0x0; /*reset descript*/

	dma->regs->DMA_CCR_CH[ch] = dma_device_cfg_table[dev_no].DefaultCCR ;
	if (device_cfg.DeviceReqType  != MEMORY_DMA_REQ)
		dma->regs->DMA_CCR_CH[ch] |= device_cfg.DeviceReqType << DMA_REQ_ID_SHIFT;

	/*Device -> Memory(Read)*/
	//if (dma->regs->DMA_CCR_CH[ch] & DEVICE_TO_MEM) {
		/* 0x00010000*/
	//	dma->mem_regs->MEM_REG_GROUP[ch].DMA_IF1CPR_CH = dma->device_cfg.MIF1addr;
	//	dma->mem_regs->MEM_REG_GROUP[ch].DMA_IF0CPR_CH = 0 ;
	//} else {/* Memory(Write)i-->Device*/
	//	dma->mem_regs->MEM_REG_GROUP[ch].DMA_IF0CPR_CH = 0 ;
	//	dma->mem_regs->MEM_REG_GROUP[ch].DMA_IF1CPR_CH = dma->device_cfg.MIF1addr ;
	//}
	//if (dev_no == MEMORY_DMA_REQ) {
		/*
		dma->mem_regs->MEM_REG_GROUP[ch].DMA_IF0CPR_CH = dma->device_cfg.MIF0addr;
		dma->mem_regs->MEM_REG_GROUP[ch].DMA_IF1CPR_CH = dma->device_cfg.MIF1addr;
		*/
		descript_chan[ch].DES_0 = dma->DES_ADDR.DES_0; /*we set descript size to 2 KB*/
		descript_chan[ch].DES_1_2 = dma->DES_ADDR.DES_1_2; /*we set descript size to 2 KB*/
	//}
	return 0;
}



/*===================================================*/
/* [Routine Description]*/
/*	check relative dma channel whether enable or disable*/
/* [Arguments]*/
/*	ch -> channel number*/
/* [Return]*/
/* 	1 -> busy(enable)*/
/*	0 -> not busy(disable)*/
/*	other -> fail*/
/*===================================================*/
int dma_busy(unsigned int ch)
{
	struct DMA_INFO *dma;
	/*
	unsigned int newtimer = 0;
	*/
	dma = &dma_chan[ch];
	if ((ch >= MAX_DMA_CHANNELS) || (dma_chan[ch].in_use == 0)) {
		printf("DMA : bad DMA identifier\n\r");
		return -1 ; /* EINVAL*/
	}

	if (dma->regs->DMA_CCR_CH[ch] & DMA_ACTIVE)
		return 1 ;
	else
		/*
		newtimer = ReadTimer();
		printf("time = %d\n",(newtimer-global_timer));
		*/
		return 0;

}

/*===================================================*/
/* [Routine Description]*/
/*	check terminal count bit of relative dma channel*/
/* [Arguments]*/
/*	ch -> channel number*/
/* [Return]*/
/* 	0 -> complete*/
/*	1 -> not complete*/
/*	other -> fail*/
/*===================================================*/
int dma_complete(unsigned int ch)
{
	struct DMA_INFO *dma;

	dma = &dma_chan[ch];
	if ((ch >= MAX_DMA_CHANNELS) || (dma_chan[ch].in_use == 0)) {
		printf("DMA : bad DMA identifier\n\r");
		return -1 ; /* EINVAL*/
	}
	/*
	if ((dma->regs->DMA_CCR_CH[ch]&DMA_EVT_ID_MASK) == DMA_EVT_SUCCESS)
		return 0 ;
	*/
	if (dma->regs->DMA_CCR_CH[ch] & DMA_P0_COMPLETE)
		return 0 ;
	else
		return 1;

}

/*===================================================*/
/* [Routine Description]*/
/*	start specific dma channel transfer*/
/*		1. check whether data size or buffer usage correctly*/
/*		2. configure relative dma channel registers*/
/*		3. dma channel enable*/
/* [Arguments]*/
/*	ch -> channel number*/
/*	dma_ptr1 -> source/destionation address for memory-to-peripheral/peripheral-to-memory transfer*/
/*			      or source address for memory-to-memory transfer*/
/*	dma_ptr2 -> destination address for memory-to-memory transfer*/
/*	size -> transfer data size*/
/* [Return]*/
/*	0 -> start transfer*/
/*	-1 -> fail*/
/*===================================================*/
int start_dma(unsigned long ch, unsigned long dma_ptr1, unsigned long dma_ptr2, unsigned long size)
{
	int count ;
	int residue_count;
	int des_cnt;
	unsigned int current_des0;
	struct DMA_INFO *dma ;
	enum dma_device dev_no ;

	/*
	global_timer = 0
	SetTimer();
	*/
	dma  = &dma_chan[ch] ;

	if (size == 0)
		return -1 ;

	dev_no = dma->device_no  ;
	count = size;
	residue_count = size;
	current_des0 = descript_chan[ch].DES_0;
	//if (dev_no == MEMORY_DMA_REQ) {
		dma->mem_regs->MEM_REG_GROUP[ch].DMA_IF0CPR_CH = descript_chan[ch].DES_0;
		dma->mem_regs->MEM_REG_GROUP[ch].DMA_IF1CPR_CH = descript_chan[ch].DES_1_2;
		des_cnt = prepare_dma_descript(current_des0, dma_ptr1, residue_count);
		dma->descript_cnt += des_cnt;
		current_des0 += des_cnt*DMA_DES0_SIZE;
		descript_chan[ch].DES_0 = current_des0;
		dma->descript_cnt = prepare_dma_descript(descript_chan[ch].DES_1_2, dma_ptr2, count);
		descript_chan[ch].DES_1_2 += dma->descript_cnt*DMA_DES0_SIZE;

	//} else {
	//	dma->descript_cnt = prepare_dma_descript(descript_chan[ch].DES_0, dma_ptr1, count);
	//	dma->mem_regs->MEM_REG_GROUP[ch].DMA_IF0CPR_CH = descript_chan[ch].DES_0;
	//}
	//dma->descript_cnt--;
	/*
	global_timer = ReadTimer();
	*/
	dma->regs->DMA_CCR_CH[ch] &= ~(SYSTEM_DMA_RUN);
	dma->regs->DMA_CCR_CH[ch] |= SYSTEM_DMA_RUN;
	printf("IF0CPR ch%d = %x\n", ch, dma->mem_regs->MEM_REG_GROUP[ch].DMA_IF0CPR_CH);
	printf("IF1CPR ch%d = %x\n", ch, dma->mem_regs->MEM_REG_GROUP[ch].DMA_IF1CPR_CH);

	return 0;
}

/*===================================================*/
/* [Routine Description]*/
/*	dma interrupt service routine*/
/*		1. check which dma channel interrupt event occur*/
/*		2. clear interrupt event*/
/*		3. call to relative device isr routine*/
/*===================================================*/
void handle_dma_irq(unsigned long ch)
{
	unsigned int global_status ;
	unsigned char chan_status ; /* fix to use 8-bit character, temp comment */
	struct DMA_INFO *dma ;
	
	if (ch >= DMA_CHANNELS){
		printf("DMA : unknown DMA IRQ\n\r") ;
		return ;
    }
	dma = &dma_chan[ch] ;
	
    //while(1){
    
	global_status = (dma_status.regs->DMA_ISR & 0xFFFF) ;
    while(!(global_status & (0x0001 << ch))){
        global_status = (dma_status.regs->DMA_ISR & 0xFFFF);
    }
    
	//while (1) {
	//	if (dma_busy(ch) != 1)
	//		break;
	//}
	
    //printf("DMA%d, %x : \n", ch, dma_status.regs->DMA_CCR_CH[ch]);

    if (global_status & (0x0001 << ch)) {
		chan_status = dma_status.regs->DMA_CCR_CH[ch] ;
		if ((chan_status & DMA_EVT_ID_MASK) == DMA_EVT_SUCCESS){
		    dma_status.regs->DMA_ISR = 1<<ch; /* Harry@Feb.28.2005*/
		    //break ;  /* do one servce, each time*/
		    
		}
	}


	/*
	* If there is any DMA error flag turned on.
	*/
	if ((chan_status & DMA_EVT_ID_MASK) != DMA_EVT_SUCCESS) {
		/* 1. clear error/abort status*/
		/* 2. re-program src/des/cnt reg 0 and 1*/
		/* 3. write "1" to csr bit6 to reset the buffer pointer to 0*/
		/* 4. re-enable dma channle*/

		/*
		 * I hold in here and try to look what is the error.
		 */
		while (1)
			;
		return ;
	}

	/*
	  * Decrease the channel buffer usage indicator.
	*/
	//}
	return;
}
/*===================================================*/
/* [Routine Description]*/
/*	create dma descript format 0*/
/*		1. clear the last descript end bit and interrupt bit*/
/*		2. count how many descripts need to be setting*/
/*		3. set descript */
/* [Arguments]*/
/*	start_descript---> the first descript address*/
/*	start_addr----> the first data address*/
/*	size----> the size needing to transfer*/
/*===================================================*/
int prepare_dma_descript(unsigned int start_descript, unsigned int start_addr, unsigned int size)
{
	unsigned int need_descriptcnt ;
	unsigned int residue_size;
	unsigned int i;
	unsigned des_col0;
	des_col0 = 0;
	need_descriptcnt = size/SIZE_32KB;
	residue_size = size%SIZE_32KB;
	struct DMA_DES_FMT0 *descript_addr;
	descript_addr = (struct DMA_DES_FMT0 *)start_descript;
	--descript_addr;
	descript_addr->REQCNT &= ~(DMA_DES_END|DMA_INTEN_DES);
	++descript_addr;
	for (i = 0; i < need_descriptcnt; ++i) {
		des_col0 = 0;
		des_col0 = SIZE_32KB;
		descript_addr->REQCNT = des_col0;
		descript_addr->DATAADDR = start_addr + i*SIZE_32KB;
		++descript_addr;
	}
	if (residue_size != 0) {
		need_descriptcnt++;
		des_col0 = 0;
		des_col0 = (DMA_DES_END|DMA_INTEN_DES|residue_size);
		descript_addr->REQCNT = des_col0;
		descript_addr->DATAADDR = start_addr + i*SIZE_32KB;
	} else {
		--descript_addr;
		descript_addr->REQCNT |= DMA_DES_END|DMA_INTEN_DES;
	}
	return need_descriptcnt;
}
/*===================================================*/
/* [Routine Description]*/
/*	create dma descript format 1*/
/* [Arguments]*/
/*	start_descript---> the first descript address*/
/*	branch_descript---> branch descirpt*/
/*	start_addr----> the first data address*/
/*	size----> the size needing to transfer*/
/*	last--->*/
/*			0:did not the last descript*/
/*			1:the last descript*/
/*===================================================*/
int prepare_dma_long_descript(unsigned int start_descript, unsigned int branch_descript,
			unsigned int start_addr, unsigned int size, unsigned int last_des)
{
	unsigned int need_descriptcnt ;
	unsigned des_col0;
	need_descriptcnt = 1;
	if (size >= 0x10000) {
		printf("reqCount too big\n");
		return 1;
	}
	des_col0 = 0;
	struct DMA_DES_FMT1 *descript_addr;
	descript_addr = (struct DMA_DES_FMT1 *)start_descript;
	des_col0 = 0;
	des_col0 = DMA_FORMAT_DES1|TRAN_SIZE;
	descript_addr->REQCNT = des_col0;
	descript_addr->DATAADDR = start_addr ;
	descript_addr->BRADDR = branch_descript ;

	if (last_des != 0) {
		need_descriptcnt++;
		des_col0 = 0;
		des_col0 |= (DMA_DES_END|DMA_INTEN_DES);
	}
	++descript_addr;
	return need_descriptcnt;
}

void wake_dma_channel(unsigned int ch)
{
	dma_status.regs->DMA_CCR_CH[ch] |= DMA_WAKE;
}
/*===================================================*/
/* [Routine Description]*/
/*	add descipt from the end of the last transfer descript*/
/* [Arguments]*/
/*	ch--->dma channel number*/
/*	source_addr--->the source data address */
/*	dest_addr--->the destination data address*/
/*	transfer_size--->the size you want transfer*/
/*===================================================*/
void add_descript(unsigned int ch, unsigned int source_addr,
		unsigned int dest_addr, unsigned int transfer_size)
{
	unsigned des_cnt;
	struct DMA_INFO *dma ;
	dma = &dma_chan[ch];
	prepare_dma_descript(descript_chan[ch].DES_0, source_addr, transfer_size);
	des_cnt = prepare_dma_descript(descript_chan[ch].DES_1_2, dest_addr, transfer_size);
	dma->descript_cnt += des_cnt ;
	descript_chan[ch].DES_0 += des_cnt*DMA_DES0_SIZE ;
	descript_chan[ch].DES_1_2 += des_cnt*DMA_DES0_SIZE;
}

void init_descriptstack(unsigned int ch)
{
    struct DMA_DES_FMT1 *descript_addr;
    descript_chan[ch].despstack_0.addr_base = (unsigned int)calloc(SIZE_8KB + 8,sizeof(int));
	descript_chan[ch].despstack_1_2.addr_base = (unsigned int)calloc(SIZE_8KB + 8,sizeof(int));
    descript_addr = (struct DMA_DES_FMT1 *)(descript_chan[ch].despstack_0.addr_base);
    descript_addr += (descript_chan[ch].despstack_0.length - 1);
    descript_addr->BRADDR = descript_chan[ch].despstack_0.addr_base;
    descript_addr->REQCNT |= DMA_FORMAT_DES1;
    descript_addr = (struct DMA_DES_FMT1 *)(descript_chan[ch].despstack_1_2.addr_base);
    descript_addr += (descript_chan[ch].despstack_1_2.length - 1);
    descript_addr->BRADDR = descript_chan[ch].despstack_1_2.addr_base;
    descript_addr->REQCNT |= DMA_FORMAT_DES1;
    
    descript_chan[ch].despstack_0.length = 1024;
    descript_chan[ch].despstack_1_2.length = 1024;
    descript_chan[ch].despstack_0.cur_index = 0;
    descript_chan[ch].despstack_1_2.cur_index = 0;
    descript_chan[ch].despstack_0.head_index = 0;
    descript_chan[ch].despstack_1_2.head_index = 0;
    descript_chan[ch].despstack_0.freedesp_num = 1024;
    descript_chan[ch].despstack_1_2.freedesp_num = 1024; 
    descript_chan[ch].despstack_0.mem_startaddr = 0;
    descript_chan[ch].despstack_1_2.mem_startaddr = 0;
    descript_chan[ch].despstack_0.mem_length = 0;
    descript_chan[ch].despstack_1_2.mem_length = 0;
}   

void reset_descriptstack(unsigned int ch)
{
    //descript_chan[ch].despstack_0.length = 1024;
    //descript_chan[ch].despstack_1_2.length = 1024;
    descript_chan[ch].despstack_0.cur_index = 0;
    descript_chan[ch].despstack_1_2.cur_index = 0;
    descript_chan[ch].despstack_0.head_index = 0;
    descript_chan[ch].despstack_1_2.head_index = 0;
    descript_chan[ch].despstack_0.freedesp_num = 1024;
    descript_chan[ch].despstack_1_2.freedesp_num = 1024;  
    descript_chan[ch].despstack_0.mem_startaddr = 0;
    descript_chan[ch].despstack_1_2.mem_startaddr = 0;
    descript_chan[ch].despstack_0.mem_length = 0;
    descript_chan[ch].despstack_1_2.mem_length = 0;
}

void free_descriptstack(unsigned int ch)
{
	free((unsigned long *)(descript_chan[ch].despstack_0.addr_base));
	free((unsigned long *)(descript_chan[ch].despstack_1_2.addr_base));
}

int set_descript(struct desp_stack * pdesp_stack, unsigned int start_addr, unsigned int size)
{
	unsigned int need_descriptcnt;
	unsigned int residue_size;
	unsigned int i;
    struct DMA_DES_FMT0 *descript_addr;
    
    if(size == 0){
        printf("ERROR: zero bytes is requested!\n");
        return 0;
    }    
    
	need_descriptcnt = size/SIZE_32KB;
	residue_size = size%SIZE_32KB;

    if(need_descriptcnt >= pdesp_stack->freedesp_num)
    {
        need_descriptcnt = pdesp_stack->freedesp_num;
    }
	if(size > need_descriptcnt * SIZE_32KB && need_descriptcnt != 0)
	{
	    pdesp_stack->mem_startaddr = start_addr + need_descriptcnt * SIZE_32KB;
	    pdesp_stack->mem_length = size - need_descriptcnt * SIZE_32KB;
	}
	else
	{
	    pdesp_stack->mem_startaddr = 0;
	    pdesp_stack->mem_length = 0;
	}
	//printf("residue = %x\n", pdesp_stack->mem_length);
	descript_addr = (struct DMA_DES_FMT0 *)(pdesp_stack->addr_base);
	if(pdesp_stack->cur_index == 0)
	{
	   	descript_addr += (pdesp_stack->length - 1); 
	}
	else
	{
	    descript_addr += (pdesp_stack->cur_index - 1);
	}
	descript_addr->REQCNT &= ~(DMA_DES_END|DMA_INTEN_DES);
	if(pdesp_stack->cur_index == 0)
	{
	   	descript_addr -= (pdesp_stack->length - 1); 
	}
	else
	{
	    ++descript_addr;;
	}
	
	pdesp_stack->head_index = pdesp_stack->cur_index;
	
	for (i = 0; i < need_descriptcnt; ++i) {
		descript_addr->REQCNT |= SIZE_32KB;
		if(pdesp_stack->cur_index == (pdesp_stack->length - 1))
		{
		    descript_addr->REQCNT |= DMA_FORMAT_DES1;
		    descript_addr = (struct DMA_DES_FMT0 *)(pdesp_stack->addr_base);
		    pdesp_stack->cur_index = 0;
		}
		else
		{
		    ++descript_addr;
		    pdesp_stack->cur_index ++;
		}
		descript_addr->DATAADDR = start_addr + i * SIZE_32KB;
		pdesp_stack->freedesp_num--;
	}
	
	if (residue_size != 0 && pdesp_stack->freedesp_num > 0) {
		descript_addr->REQCNT |= (DMA_DES_END|DMA_INTEN_DES|residue_size);
		if(pdesp_stack->cur_index == (pdesp_stack->length - 1))
		{
		    descript_addr->REQCNT |= DMA_FORMAT_DES1;
		    pdesp_stack->cur_index = 0;
		}
		else
		{
		    pdesp_stack->cur_index ++;
		}
		descript_addr->DATAADDR = start_addr + i * SIZE_32KB;
		pdesp_stack->freedesp_num--;
		need_descriptcnt ++;
	} 
	else 
	{
	    if(pdesp_stack->cur_index == 0){
		    descript_addr = (struct DMA_DES_FMT0 *)(pdesp_stack->addr_base);
		    descript_addr += (pdesp_stack->length - 1);
		}
		else{
		    descript_addr --;
		}
		descript_addr->REQCNT |= DMA_DES_END|DMA_INTEN_DES;
	}
	return need_descriptcnt;
}

int handle_transfer(unsigned long ch, unsigned long dma_ptr1, unsigned long dma_ptr2, unsigned long size)
{
	int count ;
	int residue_count;
	int des_cnt;
	struct DMA_INFO *dma ;
	enum dma_device dev_no ;
	int firststart;

    firststart = 1;
	dma  = &dma_chan[ch] ;

	if (size == 0)
		return -1 ;

	dev_no = dma->device_no;
	count = size;
	residue_count = size;
    do{
		des_cnt = set_descript(&(descript_chan[ch].despstack_0), dma_ptr1, residue_count);
	    printf("residue ch%d = %x\n", ch, descript_chan[ch].despstack_0.mem_length);
		dma->descript_cnt += des_cnt;
		descript_chan[ch].DES_0 = descript_chan[ch].despstack_0.addr_base + descript_chan[ch].despstack_0.head_index * DMA_DES0_SIZE;
		dma->mem_regs->MEM_REG_GROUP[ch].DMA_IF0CPR_CH = descript_chan[ch].DES_0;
		dma->descript_cnt = set_descript(&descript_chan[ch].despstack_1_2, dma_ptr2, count);
	    //printf("Stack 1 Base ch%d = %x\n", ch, descript_chan[ch].despstack_1_2.addr_base);
		descript_chan[ch].DES_1_2 = descript_chan[ch].despstack_1_2.addr_base + descript_chan[ch].despstack_1_2.head_index * DMA_DES0_SIZE;
        dma->mem_regs->MEM_REG_GROUP[ch].DMA_IF1CPR_CH = descript_chan[ch].DES_1_2;
        if(firststart){
    	    dma->regs->DMA_CCR_CH[ch] &= ~(SYSTEM_DMA_RUN);
	        dma->regs->DMA_CCR_CH[ch] |= SYSTEM_DMA_RUN;
	        firststart = 0;
	    }
	    else
	    {
	        dma->regs->DMA_CCR_CH[ch] |= DMA_WAKE;
	    }
	    //printf("IF0CPR ch%d = %x\n", ch, dma->mem_regs->MEM_REG_GROUP[ch].DMA_IF0CPR_CH);
	   // printf("IF1CPR ch%d = %x\n", ch, dma->mem_regs->MEM_REG_GROUP[ch].DMA_IF1CPR_CH);

        //printf("DMA%d : handle irq begin\n", ch);
        handle_dma_irq(ch);
        descript_chan[ch].despstack_0.head_index += dma->descript_cnt;
        descript_chan[ch].despstack_0.freedesp_num += dma->descript_cnt;
        descript_chan[ch].despstack_1_2.head_index += dma->descript_cnt;
        descript_chan[ch].despstack_1_2.freedesp_num += dma->descript_cnt;
        //printf("DMA%d : handle irq OK\n", ch);
	    while (1) {
		    if (dma_busy(ch) != 1)
			    break;
    	}
	    //printf("DMA%d : no busy\n", ch);
    	while (1) {
	    	if (dma_complete(ch) == 0)
		    	break;
    	}
    }
    while(descript_chan[ch].despstack_0.mem_length > 0);
   
	return 0;
}
