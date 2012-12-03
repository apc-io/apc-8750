/*++
	arch/arm/mach-wmt/dma.c - DMA 4 driver

	Copyright (c) 2008  WonderMedia Technologies, Inc.

	This program is free software: you can redistribute it and/or modify it under the
	terms of the GNU General Public License as published by the Free Software Foundation,
	either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT
	ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
	PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	You should have received a copy of the GNU General Public License along with
	this program.  If not, see <http://www.gnu.org/licenses/>.

	WonderMedia Technologies, Inc.
	10F, 529, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C.
--*/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/errno.h>
#include <linux/proc_fs.h>
#include <linux/sysdev.h>
/*
#ifdef CONFIG_PM
#include <linux/pm.h>
#include <linux/delay.h>
#endif
*/

#include <linux/pm.h>
#include <linux/delay.h>

#include <asm/system.h>
#include <asm/irq.h>
#include <mach/hardware.h>
#include <asm/dma.h>
#include <asm/io.h>
#include <linux/dma-mapping.h>

#undef DEBUG
/*#define DEBUG*/

#ifdef DEBUG
#define DPRINTK(fmt, args...)   printk("%s: " fmt, __func__ , ## args)
#else
#define DPRINTK(x...)
#endif

#define MAX_DESCRIPT_SIZE SIZE_1KB

int revise_descript( dmach_t ch, struct dma_device_cfg_s device_cfg, struct dma_mem_reg_group_s dma_mem_reg);

extern unsigned int wmt_read_oscr(void); 

/*-----------------------------------------------------------------------------*/
/* DMA channel structure.*/
/*-----------------------------------------------------------------------------*/
struct dma_info_s {
	dmach_t  channel_no ;       /* channel no*/
	struct dma_regs_s *regs;           /* points to appropriate DMA registers*/
	int irq;                    /* IRQ Index used by the channel*/
	const char *device_id;      /* device name*/
	void (*callback)(void *data);    /* to call when buffers are done*/
	void *callback_data;        /* with private data ptr*/
	enum dma_device_e device_no ;    /* device no*/
	struct dma_device_cfg_s device_cfg ;  /* device cfg*/
	int in_use;                 /* Does someone own the channel*/
	int des0cnt;                 /*  descript 0 current index*/
	int des1cnt;                 /*descript 1 current index*/
	int max_des0cnt;      /*the largest descript 0 number*/
	int max_des1cnt;      /*the largest descript 1 number*/
	int residue_des0cnt ; /*residue descript 0 count need to be xfer*/
	int residue_des1cnt ; /*residue descript 1 count need to be xfer*/
	struct dma_descript_addr des_addr;
	dma_addr_t des0_phy_addr ;
	dma_addr_t des1_phy_addr ;
	unsigned int accu_size; /*accumlate size between the dma interrupt*/
	unsigned int descript_size; /*the max descript size*/

} ;

struct dma_int_s {
	unsigned int request_chans ;
	struct dma_regs_s *regs;

};

struct des_attribute {
	unsigned int end_descript;
	unsigned int interrupt_en;
	unsigned int size;
	unsigned int fmt;
	dma_addr_t data_addr;
	dma_addr_t branch_addr;
};


/*-----------------------------------------------------------------------------*/
/*  variable*/
/*-----------------------------------------------------------------------------*/
static struct dma_int_s    dma_int ;

static struct dma_info_s dma_chan[MAX_DMA_CHANNELS];

static struct dma_mem_reg_s *dma_mem_regs;

static dma_addr_t dma_mem_phy;

/* static spinlock_t dma_list_lock = SPIN_LOCK_UNLOCKED;*/
static DEFINE_SPINLOCK(dma_list_lock);

static unsigned int dma_irq_no[] = {
		IRQ_DMA_CH_0,
		IRQ_DMA_CH_1,
		IRQ_DMA_CH_2,
		IRQ_DMA_CH_3,
		IRQ_DMA_CH_4,
		IRQ_DMA_CH_5,
		IRQ_DMA_CH_6,
		IRQ_DMA_CH_7,
		IRQ_DMA_CH_8,
		IRQ_DMA_CH_9,
		IRQ_DMA_CH_10,
		IRQ_DMA_CH_11,
		IRQ_DMA_CH_12,
		IRQ_DMA_CH_13,
		IRQ_DMA_CH_14,
		IRQ_DMA_CH_15
};

struct dma_device_cfg_s dma_device_cfg_table[] = {
	/*   DeviceReq        , DefaultCCR , Source_0, Destination_0	*/
	{ SPI0_DMA_TX_REQ , 0x00000100 , 0, 0 , SIZE_4KB},  /*spi0*/
	{ SPI0_DMA_RX_REQ , 0x00000100 , 0, 0 , SIZE_4KB},  /*spi1*/
	{ SPI1_DMA_TX_REQ , 0x00000100 , 0, 0 , SIZE_4KB},
	{ SPI1_DMA_RX_REQ , 0x00000100 , 0, 0 , SIZE_4KB},
	{ UART_5_TX_DMA_REQ , UART_TX_DMA_CFG , 0, UART5_TX_FIFO , SIZE_1B},
	{ UART_5_RX_DMA_REQ , UART_RX_DMA_CFG , 0, UART5_RX_FIFO, SIZE_4KB},
	/* start from 0, above 5, below 6 */
	{ UART_0_TX_DMA_REQ , UART_TX_DMA_CFG, 0, UART0_TX_FIFO , SIZE_1B},  /*uart0*/
	{ UART_0_RX_DMA_REQ , UART_RX_DMA_CFG, 0, UART0_RX_FIFO , SIZE_4KB},  /*uart0*/
	{ UART_1_TX_DMA_REQ , UART_TX_DMA_CFG, 0, UART1_TX_FIFO , SIZE_1B},  /*uart1*/
	{ UART_1_RX_DMA_REQ , UART_RX_DMA_CFG, 0, UART1_RX_FIFO , SIZE_4KB},  /*uart1*/
	{ UART_2_TX_DMA_REQ , UART_TX_DMA_CFG, 0, UART2_TX_FIFO , SIZE_1B},  /*uart2*/
	/*start from 0, above 10, below 11 */
	{ UART_2_RX_DMA_REQ , UART_RX_DMA_CFG, 0, UART2_RX_FIFO , SIZE_4KB},  /*uart2*/
	{ UART_3_TX_DMA_REQ , UART_TX_DMA_CFG, 0, UART3_TX_FIFO , SIZE_1B},  /*uart3*/
	{ UART_3_RX_DMA_REQ , UART_RX_DMA_CFG, 0, UART3_RX_FIFO , SIZE_4KB},  /*uart3*/
	{ PCM_TX_DMA_REQ, PCM_TX_DMA_CFG, 0, PCM_TX_FIFO, SIZE_4KB},
	{ PCM_RX_DMA_REQ, PCM_RX_DMA_CFG, 0, PCM_RX_FIFO, SIZE_4KB},
	/*start from 0, above 15, below 16 */
	{ UART_4_TX_DMA_REQ , UART_TX_DMA_CFG, 0, UART4_TX_FIFO, SIZE_1B},
	{ UART_4_RX_DMA_REQ , UART_RX_DMA_CFG, 0, UART4_RX_FIFO, SIZE_4KB},
	{ AC97_TX_DMA_REQ , AC97_TX_DMA_CFG, 0, AC97_TX_FIFO , SIZE_16KB},
	{ AC97_RX_DMA_REQ , AC97_RX_DMA_CFG, 0, AC97_RX_FIFO , SIZE_16KB},
	{ AC97_MIC_DMA_REQ , AC97_RX_DMA_CFG, 0, AC97_MIC_FIFO , SIZE_16KB},
	/*start from 0, above 20, below 21 */
	{ AHB1_AUD_DMA_REQ_0 , I2S_RX_DMA_CFG, 0, I2S_RX_FIFO , SIZE_16KB},
	{ AHB1_AUD_DMA_REQ_1 , I2S_TX_DMA_CFG, 0, I2S_TX_FIFO , SIZE_16KB},
	{ AHB1_AUD_DMA_REQ_2 , I2S_RX_DMA_CFG, 0, SPDIF_RX_FIFO, SIZE_16KB},
	{ AHB1_AUD_DMA_REQ_3 , 0x00000100, 0, 0 , SIZE_4KB},
	{ AHB1_AUD_DMA_REQ_4 , 0x00000100, 0, 0 , SIZE_4KB},
	{ AHB1_AUD_DMA_REQ_5 , 0x00000100, 0, 0 , SIZE_4KB},
	{ AHB1_AUD_DMA_REQ_6 , 0x00000100, 0, 0 , SIZE_4KB},
	{ AHB1_AUD_DMA_REQ_7 , 0x00000100, 0, 0 , SIZE_4KB},
	{ SMARTCARD_TX_DMA_REQ , 0x00000100, 0, 0 , SIZE_4KB},/*smart card*/
	{ SMARTCARD_RX_DMA_REQ , 0x00000100, 0, 0 , SIZE_4KB},/*smart card*/
	/*start from 0, above 30, below 31 */
	{ DEVICE_RESERVED , 0x00000100, 0, 0 , SIZE_4KB},
	{ MEMORY_DMA_REQ , 0x2a800000, 0x0a200000	, 0x0a220000 , SIZE_4KB},
	{ DEVICE_RESERVED , 0x00000100, 0, 0 , SIZE_4KB},
};
EXPORT_SYMBOL(dma_device_cfg_table);

/*===========================================================================*/
/*  dma_irq_handler*/
/**/
/*  return: 0*/
/*===========================================================================*/
static irqreturn_t
dma_irq_handler(int irq, void *dev_id)
{
	int ch ;
	unsigned int global_st ;
	unsigned char channel_st ;
	struct dma_info_s *dma = NULL ;
	struct dma_mem_reg_group_s dma_mem_reg ;

	global_st = dma_int.regs->DMA_ISR & 0xFFFF ;

	for (ch = 0 ; ch < MAX_DMA_CHANNELS ; ++ch) {
		if (irq == dma_irq_no[ch]) {
                        channel_st = dma_int.regs->DMA_CCR_CH[ch] & DMA_EVT_ID_MASK;
			break ;
		}
	}
	DPRINTK("%s :irq no.%d\n", __func__, irq);
	

	if (ch >= MAX_DMA_CHANNELS) {
		printk(KERN_ERR "DMA : unknown DMA IRQ\n\r") ;
		return IRQ_HANDLED ;
	}
	/*
	 * ch active handling
	 */
	dma = &dma_chan[ch] ;
	dma_int.regs->DMA_ISR = 1 << ch ;
	/*
	 * Handle channel DMA error
	 */
	if ((channel_st == DMA_EVT_NO_STATUS)) {
		/*
		 * DMA request finished with no error	
		 * Vincent 2009/05/19
		 */
		dma_mem_reg = wmt_get_dma_pos_info(ch);
		revise_descript(ch, dma->device_cfg, dma_mem_reg);
	} else if ((channel_st != DMA_EVT_SUCCESS) && (channel_st != DMA_EVT_NO_STATUS)) {
		/* 1. clear error/abort status*/
		/* 2. re-program src/des/cnt reg 0 and 1*/
		/* 3. write "1" to csr bit6 to reset the buffer pointer to 0*/
		/* 4. re-enable dma channle*/
		printk(KERN_ERR "ch=%d status=0x%.2x err\n\r",
			ch, channel_st) ;
		/*
		 * dma->callback(dma->callback_data) ; 
		 * if callback runs, audio driver think this descp is done
		 * Vincent 2009/05/19
		 */
		wmt_resume_dma(ch);
		/* free buffer and callback to handle error*/
		return IRQ_HANDLED ;
	}
	/*
	* Decrease the channel descript usage indicator.
	*/
	if (dma->residue_des0cnt > 0)
		--dma->residue_des0cnt;
	if (dma->callback)
		dma->callback(dma->callback_data) ;

	return IRQ_HANDLED;
}

int create_fmt0_descript(
	dmach_t ch,
	struct dma_device_cfg_s device_cfg,
	struct des_attribute descript_attr)
{
	struct dma_info_s *dma ;
	struct dma_des_fmt0 descript;
	unsigned int ReqCnt = 0;
	unsigned int des_offset;
	descript.DataAddr = 0;
	descript.ReqCnt = 0;
	dma = &dma_chan[ch] ;
	des_offset = dma->des0cnt * sizeof(struct dma_des_fmt0)/sizeof(unsigned long);

	DPRINTK("[%s] : create fmt 0 descript size=%x\n", __func__, descript_attr.size);
	DPRINTK("[%s] : des0cnt = %x\n", __func__, dma->des0cnt);

	if ((ch >= MAX_DMA_CHANNELS) || (dma_chan[ch].in_use == 0)) {
		printk("%s: bad DMA identifier\n", __func__) ;
		return -EINVAL ;
	}

	if (dma->device_no != device_cfg.DeviceReqType) {
		printk("%s: bad Device_NO\n", __func__) ;
		return -ENODEV ;
	}

	descript.DataAddr = (unsigned long)descript_attr.data_addr;
	if (descript_attr.interrupt_en == 1)
		ReqCnt |= DMA_INTEN_DES;
	if (descript_attr.end_descript == 1)
		ReqCnt |= DMA_DES_END;
	if (descript_attr.size > (SIZE_64KB - 1))
		return -EOVERFLOW ;
	descript.ReqCnt = ReqCnt | descript_attr.size;
	DPRINTK("[%s]:des_offset = %d , des0 = 0x%x\n",
		 __func__, des_offset, dma->des_addr.des_0 + des_offset);
	*(dma->des_addr.des_0 + des_offset) = descript.ReqCnt;
	*(dma->des_addr.des_0 + des_offset + 1) = descript.DataAddr;
	++dma->des0cnt;
	return 0;

}

int create_fmt1_descript(
	dmach_t ch,
	struct dma_device_cfg_s device_cfg,
	struct des_attribute descript_attr)
{
	struct dma_info_s *dma ;
	struct dma_des_fmt1 descript;
	unsigned int ReqCnt = 0;
	unsigned int des_offset;
	descript.DataAddr = 0;
	descript.ReqCnt = 0;
	dma = &dma_chan[ch] ;
	des_offset = (dma->max_des0cnt - 1) * sizeof(struct dma_des_fmt0) / sizeof(unsigned long);

	DPRINTK("[%s]:create fmt 1 descript size=%x\n", __func__, descript_attr.size);
	DPRINTK("[%s]:des0cnt = %x\n", __func__, dma->des0cnt);
	DPRINTK("[%s]:branch_addr = %x\n", __func__, descript_attr.branch_addr);

	if ((ch >= MAX_DMA_CHANNELS) || (dma_chan[ch].in_use == 0)) {
		printk("%s: bad DMA identifier\n", __func__) ;
		return -EINVAL ;
	}

	if (dma->device_no != device_cfg.DeviceReqType) {
		printk("%s: bad Device_NO\n", __func__) ;
		return -ENODEV ;
	}
	/*if (descript_attr.size > (SIZE_64KB - 1))*/
	descript.DataAddr = (unsigned long)descript_attr.data_addr;
	descript.BrAddr = (unsigned long)descript_attr.branch_addr;
	if (descript_attr.interrupt_en == 1)
		ReqCnt |= DMA_INTEN_DES;
	if (descript_attr.end_descript == 1)
		ReqCnt |= DMA_DES_END;
	if (descript_attr.fmt == 1)
		ReqCnt |= DMA_FORMAT_DES1;
	if (descript_attr.size > (SIZE_64KB - 1))
		return -EOVERFLOW ;
	DPRINTK("[%s] : fmt_1 des_offset = %d , des0 = 0x%x\n",
		__func__, des_offset, dma->des_addr.des_0 + des_offset);
	descript.ReqCnt = ReqCnt | descript_attr.size;
	*(dma->des_addr.des_0 + des_offset) = descript.ReqCnt;
	*(dma->des_addr.des_0 + des_offset + 1) = descript.DataAddr;
	*(dma->des_addr.des_0 + des_offset + 2) = descript.BrAddr ;
	dma->des0cnt = 0;
	return 0;
}

int clear_last_descript(
	dmach_t ch,
	struct dma_device_cfg_s device_cfg)
{
	struct dma_info_s *dma ;
	unsigned int des_offset;
	dma = &dma_chan[ch] ;

	if ((dma->des0cnt - 1 >= 0))
		des_offset = (dma->des0cnt - 1) * sizeof(struct dma_des_fmt0) / sizeof(unsigned long);
	else
		des_offset = (dma->max_des0cnt - 1) * sizeof(struct dma_des_fmt0) / sizeof(unsigned long);

	if ((ch >= MAX_DMA_CHANNELS) || (dma_chan[ch].in_use == 0)) {
		printk("%s: bad DMA identifier\n", __func__) ;
		return -EINVAL ;
	}

	if (dma->device_no != device_cfg.DeviceReqType) {
		printk("%s: bad Device_NO\n", __func__) ;
		return -ENODEV ;
	}
	*(dma->des_addr.des_0 + des_offset) &= ~(DMA_DES_END);
	return 0;
}
int add_descript(
	dmach_t ch,
	struct dma_device_cfg_s device_cfg,
	dma_addr_t dma_ptr ,
	unsigned int size)
{
	struct dma_info_s *dma ;
	unsigned int residue_size;
	unsigned int xfer_size;
	unsigned int xfer_index;
	unsigned int ret = 0;
	struct des_attribute descript_attr;
	int need_add_descript_count = 0;
	dma = &dma_chan[ch] ;
	residue_size = size;
	xfer_index = 0; 
	need_add_descript_count = size/SIZE_32KB ;
	if (size%SIZE_32KB)
		++need_add_descript_count;
	if ((ch >= MAX_DMA_CHANNELS) || (dma_chan[ch].in_use == 0)) {
		printk("%s: bad DMA identifier\n", __func__) ;
		return -EINVAL ;
	}

	if (dma->device_no != device_cfg.DeviceReqType) {
		printk("%s: bad Device_NO\n", __func__) ;
		return -ENODEV ;
	}
	if ((dma->max_des0cnt - dma->residue_des0cnt) < need_add_descript_count) {
		printk("%s:dma descripts are full\n",__func__);
		return -EBUSY ;
	}
	while (residue_size > 0) {
		if (residue_size == size)
			ret = clear_last_descript(ch, device_cfg);

		xfer_size = residue_size;
		if (residue_size > SIZE_32KB) {
 			//xfer_size = residue_size - SIZE_32KB;
 			xfer_size =  SIZE_32KB;//vincent
			dma->accu_size += xfer_size;
			residue_size -= SIZE_32KB;
			dma_ptr += xfer_size * xfer_index;
		} else {
			xfer_size = residue_size;
			dma_ptr += xfer_size * xfer_index;
			dma->accu_size += xfer_size;
			residue_size = 0;
		}

		if (dma->des0cnt < dma->max_des0cnt - 1) {
			if (residue_size <= SIZE_32KB)
				descript_attr.end_descript = 1;
			if (dma->accu_size >= device_cfg.ChunkSize) {
				descript_attr.interrupt_en = 1;
				dma->accu_size = 0;
				dma->accu_size += xfer_size;
			}
			descript_attr.data_addr = dma_ptr;
			descript_attr.size = xfer_size ;
			descript_attr.fmt = 0;
			ret = create_fmt0_descript(ch,
				device_cfg,
				descript_attr);
		} else {
			if (residue_size <= SIZE_32KB)
				descript_attr.end_descript = 1;
			if (dma->accu_size >= device_cfg.ChunkSize) {
				descript_attr.interrupt_en = 1;
				dma->accu_size = 0;
				dma->accu_size += xfer_size;
			}
			descript_attr.data_addr = dma_ptr;
			descript_attr.branch_addr = dma->des0_phy_addr;
			descript_attr.size = xfer_size ;
			descript_attr.fmt = 1;
			ret = create_fmt1_descript(ch,
				device_cfg,
				descript_attr);
		}
		xfer_index++;
	}
	return xfer_index;
}

int revise_descript(
	dmach_t ch,
	struct dma_device_cfg_s device_cfg,
	struct dma_mem_reg_group_s dma_mem_reg)
{
	struct dma_info_s *dma ;
	unsigned long flags;
	unsigned int ret = 0;
	unsigned int des_offset = 0;
	unsigned int req_count = 0;
	unsigned int data_address = 0;
	unsigned int now_time = 0;
	unsigned int delay_time = 0;
	dma = &dma_chan[ch] ;
	if ((ch >= MAX_DMA_CHANNELS) || (dma_chan[ch].in_use == 0)) {
		printk("%s: bad DMA identifier\n", __func__) ;
		return -EINVAL ;
	}

	if (dma->device_no != device_cfg.DeviceReqType) {
		printk("%s: bad Device_NO\n", __func__) ;
		return -ENODEV ;
	}
	if (dma->regs->DMA_CCR_CH[ch] & (SYSTEM_DMA_RUN)) {
		spin_lock_irqsave(&dma_list_lock, flags);
		dma->regs->DMA_CCR_CH[ch] |= (DMA_UP_MEMREG_EN);/*update memory register before reading*/
		now_time = wmt_read_oscr();
		while (dma->regs->DMA_CCR_CH[ch] &  (DMA_UP_MEMREG_EN)) {
			delay_time = wmt_read_oscr() - now_time;
			if (delay_time > 15) {/*5us*/
				DPRINTK("[%d]Warnning:up_mem_reg did not clear[%x]\n", ch, dma->regs->DMA_CCR_CH[ch]);
				dma->regs->DMA_CCR_CH[ch] &= ~DMA_UP_MEMREG_EN;/*clear DMA_UP_MEMREG_EN*/
				break;
			}

		}
		spin_unlock_irqrestore(&dma_list_lock, flags);
	}
	req_count = dma_mem_reg.DMA_IF0RBR_CH;
	req_count &= (DMA_DES_REQCNT_MASK) ;/*Vincent 2009.5.4*/
	data_address = dma_mem_reg.DMA_IF0DAR_CH;
	des_offset = dma_mem_reg.DMA_IF0CPR_CH - dma->des0_phy_addr ;
	if (req_count  > 0) {
		*(dma->des_addr.des_0 + (des_offset / sizeof(unsigned long))) &= ~(DMA_DES_REQCNT_MASK);
		*(dma->des_addr.des_0 + (des_offset / sizeof(unsigned long)))  |= req_count;
		*(dma->des_addr.des_0 + (des_offset / sizeof(unsigned long)) + 1) = 0;
		*(dma->des_addr.des_0 + (des_offset / sizeof(unsigned long)) + 1) = data_address;
	} else {
		/*Vincent 2009/05/19*/
		if (des_offset < (dma->max_des0cnt - 1) * DMA_DES0_SIZE)
			dma_mem_reg.DMA_IF0CPR_CH += 8;
		else
			dma_mem_reg.DMA_IF0CPR_CH = dma->des0_phy_addr;

	}
	return ret ;
}
/*=============================================================================*/
/**/
/* 	wmt_start_dma - submit a data buffer for DMA*/
/*	Memory To Device or Device To Memory*/
/* 	@ch: identifier for the channel to use*/
/* 	@dma_ptr: buffer physical (or bus) start address*/
/*	@dma_ptr2: device FIFO address*/
/* 	@size: buffer size*/
/**/
/*	Memory To Memory*/
/* 	@ch: identifier for the channel to use*/
/* 	@dma_ptr: buffer physical (or bus) source start address*/
/*	@dma_ptr2: buffer physical (or bus) destination start address*/
/* 	@size: buffer size*/
/**/
/* 	This function hands the given data buffer to the hardware for DMA*/
/* 	access. If another buffer is already in flight then this buffer*/
/* 	will be queued so the DMA engine will switch to it automatically*/
/* 	when the previous one is done.  The DMA engine is actually toggling*/
/* 	between two buffers so at most 2 successful calls can be made before*/
/* 	one of them terminates and the callback function is called.*/
/**/
/* 	The @ch identifier is provided by a successful call to*/
/* 	wmt_request_dma().*/
/**/
/* 	The @size must not be larger than %MAX_DMA_SIZE.  If a given buffer*/
/* 	is larger than that then it's the caller's responsibility to split*/
/* 	it into smaller chunks and submit them separately. If this is the*/
/* 	case then a @size of %CUT_DMA_SIZE is recommended to avoid ending*/
/* 	up with too small chunks. The callback function can be used to chain*/
/* 	submissions of buffer chunks.*/
/**/
/* 	Error return values:*/
/* 	%-EOVERFLOW:	Given buffer size is too big.*/
/* 	%-EBUSY:	Both DMA buffers are already in use.*/
/* 	%-EAGAIN:	Both buffers were busy but one of them just completed*/
/* 			but the interrupt handler has to execute first.*/
/**/
/*	This function returs 0 on success.*/
/**/
/*=============================================================================*/
int wmt_start_dma(dmach_t ch, dma_addr_t dma_ptr, dma_addr_t dma_prt2, unsigned int size)
{
	unsigned long flags;
	int count ;
	int ret = 0;
	int descript_count = 0;
	struct dma_info_s *dma = &dma_chan[ch] ;
	/*dump_dma_regs(ch);*/

	DPRINTK("size = %x, chunksize=%x\n", size, dma->device_cfg.ChunkSize);

	if (size == 0)
		return -EINVAL ;

	local_irq_save(flags);

	descript_count = add_descript(ch, dma->device_cfg, dma_ptr, size);
	if (descript_count < 0){
		ret = -EBUSY;
		goto start_dma_out;
	}

	dma->residue_des0cnt += descript_count ;
 	if (dma->residue_des0cnt > dma->max_des0cnt - 1)
 	{
	//Vincent 2009/05/19
 	DPRINTK("%s: dma->residue_des0cnt(%d)  is too large\n",
 		__func__,dma->residue_des0cnt );
 	dma->residue_des0cnt = dma->max_des0cnt - 1;
 	
 	}
	DPRINTK("%s: ch = %d, dma_ptr =  0x%8.8x, size = %d (0x%8.8X)\n",
		__func__, ch , dma_ptr, size , size);

	/* Calculate burst count*/
	if (dma->regs->DMA_CCR_CH[ch] & SYSTEM_DMA_RUN) /*still run*/
		dma->regs->DMA_CCR_CH[ch] |= DMA_WAKE;
	else {
		count = size ;
		dma_mem_regs->mem_reg_group[ch].DMA_IF0CPR_CH = dma->des0_phy_addr;
		DPRINTK("dma descript 0 phy addr = 0x%x\n", dma_mem_regs->mem_reg_group[ch].DMA_IF0CPR_CH);
		if (dma->device_cfg.DeviceReqType == MEMORY_DMA_REQ)
			dma_mem_regs->mem_reg_group[ch].DMA_IF1CPR_CH = dma->des0_phy_addr;
		dma->regs->DMA_CCR_CH[ch] |= (SYSTEM_DMA_RUN | SYSTEM_DMA_REQ_EN);
	}

start_dma_out:
	local_irq_restore(flags);
	return ret ;
}

int wmt_wake_dma(
	dmach_t ch,
	dma_addr_t dma_ptr,
	dma_addr_t dma_prt2,
	unsigned int size)
{
	int ret = 0;
	struct dma_info_s *dma = &dma_chan[ch] ;
	/*dump_dma_regs(ch);*/

	DPRINTK("size = %x, chunksize=%x\n", size, dma->device_cfg.ChunkSize);

	if (size == 0)
		return -EINVAL ;
	if (size > dma->device_cfg.ChunkSize)
		return -EOVERFLOW ;
	if (dma->regs->DMA_CCR_CH[ch] & SYSTEM_DMA_RUN) { /*still run*/
		ret = add_descript(ch, dma->device_cfg, dma_ptr, size);
		dma->residue_des0cnt += ret ;
		dma->regs->DMA_CCR_CH[ch] |= DMA_WAKE;
	}
	return ret ;
}

/*=============================================================================*/
/**/
/*	wmt_request_dma - allocate one of the DMA chanels*/
/*      @channel: Pointer to the location of the allocated channel's identifier*/
/*	@device_id: An ascii name for the claiming device*/
/*	@device: The WMT peripheral targeted by this request*/
/*	@callback: Function to be called when the DMA completes*/
/*	@data: A cookie passed back to the callback function*/
/**/
/* 	This function will search for a free DMA channel and returns the*/
/* 	address of the hardware registers for that channel as the channel*/
/* 	identifier. This identifier is written to the location pointed by*/
/* 	@dma_regs. The list of possible values for @device are listed into*/
/* 	linux/include/asm-arm/arch-wmt/dma.h as a dma_device_t enum.*/
/**/
/* 	Note that reading from a port and writing to the same port are*/
/* 	actually considered as two different streams requiring separate*/
/* 	DMA registrations.*/
/**/
/* 	The @callback function is called from interrupt context when one*/
/* 	of the two possible DMA buffers in flight has terminated. That*/
/* 	function has to be small and efficient while posponing more complex*/
/* 	processing to a lower priority execution context.*/
/**/
/* 	If no channels are available, or if the desired @device is already in*/
/* 	use by another DMA channel, then an error code is returned.  This*/
/* 	function must be called before any other DMA calls.*/
/**/
/*      return: 0 if successful*/
/**/
/*=============================================================================*/
int wmt_request_dma(dmach_t *channel, const char *device_id, enum dma_device_e device,
	void (*callback)(void *data), void *callback_data)
{
	int ch  ;
	int descript_size = MAX_DESCRIPT_SIZE;
	struct dma_info_s *dma = NULL;
	*channel = -1;

	/**/
	/*  Ask for Free Channels*/
	/**/
	spin_lock(&dma_list_lock);
	for (ch = 1 ; ch < MAX_DMA_CHANNELS ; ++ch) {
		dma = &dma_chan[ch];
		if (dma->in_use == 0)
			break ;
	}
	if (ch >= MAX_DMA_CHANNELS) {
		DPRINTK("DMA %s: no free DMA channel available\n", device_id);
		return -EBUSY;
	}
	spin_unlock(&dma_list_lock);

	dma_int.request_chans |= (1 << ch) ;

	/**/
	/* Configure DMA channel settings.*/
	/**/
	*channel            = ch;
	dma->device_id      = device_id;
	dma->device_no      = device ;
	dma->callback       = callback;
	dma->callback_data  = callback_data ;
	dma->in_use         = 1 ;
	dma->des0cnt = 0 ;
	dma->des1cnt = 0;
	dma->accu_size = 0;
	dma->residue_des0cnt = 0;
	dma->residue_des1cnt = 0;
	dma->descript_size = descript_size;
	/**/
	/* clear status register*/
	/**/
	dma->regs->DMA_ISR = 1 << ch;
	dma_mem_regs->mem_reg_group[ch].DMA_IF0CPR_CH = 0x0; /*reset descript*/
	dma_mem_regs->mem_reg_group[ch].DMA_IF1CPR_CH = 0x0; /*reset descript*/
	dma->des_addr.des_0 = (unsigned long *)dma_alloc_coherent(
		NULL,
		descript_size,
		&dma->des0_phy_addr,
		GFP_KERNEL);
	dma->max_des0cnt = (descript_size - DMA_DES1_SIZE) / DMA_DES0_SIZE + 1;
	DPRINTK("descript 0 addr--- virt:0x%x , phy:0x%x\n", dma->des_addr.des_0, dma->des0_phy_addr);
	/**/
	/* setup default device*/
	/**/
	dma->device_cfg = dma_device_cfg_table[device] ;

	dma->regs->DMA_CCR_CH[ch] = (dma_device_cfg_table[device].DefaultCCR & DMA_USER_SET_MASK) ;

	if (device != MEMORY_DMA_REQ)
		/*need to set device req number*/
		dma->regs->DMA_CCR_CH[ch] |= device << DMA_REQ_ID_SHIFT;

	else {
		dma->des_addr.des_1 = (unsigned long *)dma_alloc_coherent(NULL,
				descript_size, &dma->des0_phy_addr, GFP_KERNEL);
		dma->max_des1cnt = (descript_size - DMA_DES1_SIZE)/DMA_DES0_SIZE + 1;
	}


	DPRINTK("requested dma ch=%d to device=%d\n", ch, device);

	return 0 ;      /* No error*/
}

/*=============================================================================*/
/**/
/*	wmt_clear_dma - clear DMA pointers*/
/*	@ch:identifier for the channel to use*/
/**/
/*	This clear any DMA state so the DMA engine is ready to restart*/
/*	with new buffers through wmt_start_dma(). Any buffers in flight*/
/*	are discarded.*/
/**/
/*	The @regs identifier is provided by a successful call to*/
/*	wmt_request_dma().*/
/**/
/*      return: NULL*/
/*=============================================================================*/
void wmt_clear_dma(dmach_t ch)
{
	unsigned long flags;
	struct dma_info_s *dma ;
	dma = &dma_chan[ch] ;

	local_irq_save(flags);
	/**/
	/* clear status register*/
	/**/
	dma->regs->DMA_CCR_CH[ch] &= ~(SYSTEM_DMA_REQ_EN);
	udelay(5);
	dma->regs->DMA_CCR_CH[ch] &= ~(SYSTEM_DMA_RUN);
	dma->regs->DMA_ISR = 1 << ch; /*write 1 clear*/
	dma->des0cnt = 0;
	dma->des1cnt = 0;
	dma->accu_size = 0;
	dma->residue_des0cnt = 0;
	dma->residue_des0cnt = 0;
	local_irq_restore(flags);
}

/*=============================================================================*/
/**/
/* 	wmt_free_dma - free a WMT DMA channel*/
/* 	@ch: identifier for the channel to free*/
/**/
/* 	This clears all activities on a given DMA channel and releases it*/
/* 	for future requests.  The @ch identifier is provided by a*/
/* 	successful call to wmt_request_dma().*/
/**/
/*      return: NULL*/
/**/
/*=============================================================================*/
void wmt_free_dma(dmach_t ch)
{
	struct dma_info_s *dma;
	enum dma_device_e dev_no ;

	if ((unsigned) ch >= MAX_DMA_CHANNELS) {
		DPRINTK("%s: bad DMA identifier\n", __func__);
		return ;
	}

	dma = &dma_chan[ch];
	if (dma->in_use == 0) {
		DPRINTK("%s: Trying to free DMA%d\n", __func__, ch);
		return;
	}

	if (dma->device_no == DEVICE_RESERVED) {
		DPRINTK("%s: Trying to free free DMA\n", __func__);
		return ;
	}

	wmt_clear_dma(ch);

	/**/
	/* Int*/
	/**/
	dma_int.request_chans &= ~(1 << ch) ;

	dev_no = dma->device_no ;
	dma_free_coherent(NULL,
		dma->descript_size,
		(void *)dma->des_addr.des_0,
		(dma_addr_t)dma->des0_phy_addr);
	if (dma->device_no == MEMORY_DMA_REQ) 
		dma_free_coherent(NULL,
			dma->descript_size,
			(void *)dma->des_addr.des_1,
			(dma_addr_t)dma->des1_phy_addr);

	dma->device_no  = DEVICE_RESERVED ;
	dma_chan[ch].device_id  = NULL ;
	dma_chan[ch].des0cnt = 0;
	dma_chan[ch].des1cnt = 0;
	dma_chan[ch].accu_size = 0;
	dma_chan[ch].residue_des0cnt = 0;
	dma_chan[ch].residue_des0cnt = 0;
	dma_chan[ch].max_des0cnt = 0;
	dma_chan[ch].max_des1cnt = 0;
	dma_chan[ch].in_use = 0;
}

/*=============================================================================*/
/**/
/*	wmt_reset_dma - reset a DMA channel*/
/*	@ch: identifier for the channel to use*/
/**/
/*	This function resets and reconfigure the given DMA channel. This is*/
/*	particularly useful after a sleep/wakeup event.*/
/**/
/*	The @ch identifier is provided by a successful call to*/
/*	request_dma().*/
/**/
/*      return: NULL*/
/**/
/*=============================================================================*/
void wmt_reset_dma(dmach_t ch)
{

	if (ch >= MAX_DMA_CHANNELS) {
		DPRINTK("%s: bad DMA identifier\n", __func__);
		return;
	}

	wmt_clear_dma(ch);

}

/*===========================================================================*/
/*  wmt_setup_dma*/
/**/
/*  Don't setup Dma channel while Dma is busy*/
/**/
/*  return: 0: success*/
/*===========================================================================*/
int wmt_setup_dma(dmach_t ch, struct dma_device_cfg_s device_cfg)
{
	struct dma_info_s *dma ;
	enum dma_device_e dev_no ;

	dma = &dma_chan[ch] ;

	if ((ch >= MAX_DMA_CHANNELS) || (dma_chan[ch].in_use == 0)) {
		printk("%s: bad DMA identifier\n", __func__) ;
		return -EINVAL ;
	}

	if (dma->device_no != device_cfg.DeviceReqType) {
		printk("%s: bad Device_NO\n", __func__) ;
		return -ENODEV ;
	}
	/**/
	/* Apply new device config to DMA interface.*/
	/**/
	dev_no = dma->device_no ;
	dma_device_cfg_table[dev_no] = device_cfg ;
	dma->device_cfg = dma_device_cfg_table[dev_no] ;

	/**/
	/* Clear status register.*/
	/**/
	dma->regs->DMA_ISR = 1<<ch;
#if 0
	//Vincent 2009/05/19
	dma_mem_regs->mem_reg_group[ch].DMA_IF0CPR_CH = 0x0; /*reset descript*/
	dma_mem_regs->mem_reg_group[ch].DMA_IF1CPR_CH = 0x0; /*reset descript*/
	dma->des0cnt = 0x0; /*reset des index*/
	dma->des1cnt = 0x0; /*reset des indxe*/
#endif
	DPRINTK("%s old CCR=0x%.8x\n", __func__, dma->regs->DMA_CCR_CH[ch]) ;

	/**/
	/* Apply new DMA config to DMA controller.*/
	/**/
	dma->regs->DMA_CCR_CH[ch] = dma_device_cfg_table[dev_no].DefaultCCR ;
	DPRINTK("%s new CCR=0x%.8x\n", __func__, dma_device_cfg_table[dev_no].DefaultCCR) ;
	DPRINTK("%s old CCR=0x%.8x\n", __func__, dma->regs->DMA_CCR_CH[ch].CCR) ;
	if (device_cfg.DeviceReqType  != MEMORY_DMA_REQ)
		dma->regs->DMA_CCR_CH[ch] |= device_cfg.DeviceReqType << DMA_REQ_ID_SHIFT;
	/*Device -> Memory(Read) && Memory(Write)i-->Device*/
	if (dev_no != MEMORY_DMA_REQ) {
		dma_mem_regs->mem_reg_group[ch].DMA_IF1CPR_CH = dma->device_cfg.MIF1addr;
		/*dma_mem_regs->mem_reg_group[ch].DMA_IF0CPR_CH = 0 ;*/
	}
	/*
	if (dev_no == MEMORY_DMA_REQ) {
		dma_mem_regs->mem_reg_group[ch].DMA_IF0CPR_CH = dma->des0_phy_addr;
		dma_mem_regs->mem_reg_group[ch].DMA_IF1CPR_CH = dma->des1_phy_addr;
	}
	*/
	DPRINTK("%s new CCR=0x%.8x\n", __func__, dma->regs->DMA_CCR_CH[ch]) ;

	return 0;
}

/*=============================================================================*/
/**/
/*	wmt_stop_dma - stop DMA in progress*/
/*	@regs: identifier for the channel to use*/
/**/
/*	This stops DMA without clearing buffer pointers. Unlike*/
/*	clear_dma() this allows subsequent use of resume_dma()*/
/*	or get_dma_pos().*/
/**/
/*	The @regs identifier is provided by a successful call to*/
/*	request_dma().*/
/**/
/*=============================================================================*/
void wmt_stop_dma(dmach_t ch)
{
	struct dma_info_s *dma;
	unsigned int now_time = 0;
	unsigned int delay_time = 0;

	if ((ch >= MAX_DMA_CHANNELS) ||
	     (dma_chan[ch].in_use == 0)) {
		DPRINTK("%s: bad DMA identifier\n", __func__);
		return;
	}

	dma = &dma_chan[ch];
	dma->regs->DMA_CCR_CH[ch] &= ~(SYSTEM_DMA_REQ_EN);
	udelay(5);
	if (dma->regs->DMA_CCR_CH[ch] & (SYSTEM_DMA_RUN)) {
		dma->regs->DMA_CCR_CH[ch] |= (DMA_UP_MEMREG_EN);/*update memory register before reading*/
		now_time = wmt_read_oscr();
		while (dma->regs->DMA_CCR_CH[ch] & (DMA_UP_MEMREG_EN)) {
			delay_time = wmt_read_oscr() - now_time;
			if (delay_time > 15) {/*5us*/
				DPRINTK("[%d]Warnning:up_mem_reg did not clear[%x]\n", ch, dma->regs->DMA_CCR_CH[ch]);
				dma->regs->DMA_CCR_CH[ch] &= ~DMA_UP_MEMREG_EN;/*clear DMA_UP_MEMREG_EN*/
				break;
			}
		}
	}
	dma->regs->DMA_CCR_CH[ch] &= ~SYSTEM_DMA_RUN;
}

/*=============================================================================*/
/**/
/*	wmt_resume_dma - resume DMA on a stopped channel*/
/*	@regs: identifier for the channel to use*/
/**/
/*	This resumes DMA on a channel previously stopped with*/
/*	wmt_stop_dma().*/
/**/
/*	The @regs identifier is provided by a successful call to*/
/*	wmt_request_dma().*/
/**/
/*=============================================================================*/
void wmt_resume_dma(dmach_t ch)
{
	struct dma_info_s *dma;
	struct dma_mem_reg_group_s dma_mem_reg ;
	dma = &dma_chan[ch] ;
	if (dma->regs->DMA_CCR_CH[ch] & DMA_ACTIVE) {/*if dma was active , disable dma first*/
		dma->regs->DMA_CCR_CH[ch] &= ~(SYSTEM_DMA_REQ_EN);
		udelay(5);
		dma->regs->DMA_CCR_CH[ch] &= ~(SYSTEM_DMA_RUN);
	}
	if ((ch >= MAX_DMA_CHANNELS) ||
		(dma_chan[ch].in_use == 0)) {
		DPRINTK("%s: bad DMA identifier\n", __func__);
		return;
	}
	dma_mem_reg = wmt_get_dma_pos_info(ch);
	revise_descript(ch, dma->device_cfg, dma_mem_reg);
	dma->regs->DMA_CCR_CH[ch] |= (SYSTEM_DMA_RUN | SYSTEM_DMA_REQ_EN);
}

/*=============================================================================*/
/**/
/*	wmt_get_dma_pos_info - return current DMA position*/
/*	@ch: identifier for the channel to use*/
/**/
/*	This function returns the current physical (or bus) address for the*/
/*	given DMA channel.  If the channel is running i.e. not in a stopped*/
/*	state then the caller must disable interrupts prior calling this*/
/*	function and process the returned value before re-enabling them to*/
/*	prevent races with the completion interrupt handler and the callback*/
/*	function. The validation of the returned value is the caller's*/
/*	responsibility as well -- the hardware seems to return out of range*/
/*	values when the DMA engine completes a buffer.*/
/**/
/*	The @ch identifier is provided by a successful call to*/
/*	wmt_request_dma().*/
/**/
/*=============================================================================*/
struct dma_mem_reg_group_s  wmt_get_dma_pos_info(dmach_t ch)
{
	struct dma_mem_reg_group_s dma_mem_reg;
	struct dma_info_s *dma;
	unsigned long flags;
	unsigned int now_time = 0;
	unsigned int delay_time = 0;
	dma = &dma_chan[ch];

	if ((ch >= MAX_DMA_CHANNELS) ||
	     (dma_chan[ch].in_use == 0)) {
		DPRINTK("%s: bad DMA identifier\n", __func__);
		goto out;
	}
	if (dma->regs->DMA_CCR_CH[ch] & (SYSTEM_DMA_RUN)) {
		spin_lock_irqsave(&dma_list_lock, flags);
		dma->regs->DMA_CCR_CH[ch] |= (DMA_UP_MEMREG_EN);/*update memory register before reading*/

		now_time = wmt_read_oscr();
		while (dma->regs->DMA_CCR_CH[ch] &  (DMA_UP_MEMREG_EN)) {
			delay_time = wmt_read_oscr() - now_time;
			if (delay_time > 15) {/*5us*/
				DPRINTK("[%d]Warnning:up_mem_reg did not clear[%x]\n", ch, dma->regs->DMA_CCR_CH[ch]);
				dma->regs->DMA_CCR_CH[ch] &= ~DMA_UP_MEMREG_EN;/*clear DMA_UP_MEMREG_EN*/
				break;
			}

		}

		spin_unlock_irqrestore(&dma_list_lock, flags);
	}

	dma_mem_reg.DMA_IF0BAR_CH = dma_mem_regs->mem_reg_group[ch].DMA_IF0BAR_CH;
	dma_mem_reg.DMA_IF0CPR_CH = dma_mem_regs->mem_reg_group[ch].DMA_IF0CPR_CH;
	dma_mem_reg.DMA_IF0RBR_CH = dma_mem_regs->mem_reg_group[ch].DMA_IF0RBR_CH;
	dma_mem_reg.DMA_IF0DAR_CH = dma_mem_regs->mem_reg_group[ch].DMA_IF0DAR_CH;
	dma_mem_reg.DMA_IF1BAR_CH = dma_mem_regs->mem_reg_group[ch].DMA_IF1BAR_CH;
	dma_mem_reg.DMA_IF1CPR_CH = dma_mem_regs->mem_reg_group[ch].DMA_IF1CPR_CH;
	dma_mem_reg.DMA_IF1RBR_CH = dma_mem_regs->mem_reg_group[ch].DMA_IF1RBR_CH;
	dma_mem_reg.DMA_IF1DAR_CH = dma_mem_regs->mem_reg_group[ch].DMA_IF1DAR_CH;

out:
	return dma_mem_reg;
}

/*=============================================================================*/
/**/
/*      wmt_get_dma_pos - return current DMA position*/
/*      @ch: identifier for the channel to use*/
/**/
/*      This function returns the current physical (or bus) address for the*/
/*      given DMA channel.  If the channel is running i.e. not in a stopped*/
/*      state then the caller must disable interrupts prior calling this*/
/*      function and process the returned value before re-enabling them to*/
/*      prevent races with the completion interrupt handler and the callback*/
/*      function. The validation of the returned value is the caller's*/
/*      responsibility as well -- the hardware seems to return out of range*/
/*      values when the DMA engine completes a buffer.*/
/**/
/*      The @ch identifier is provided by a successful call to*/
/*      wmt_request_dma().*/
/**/
/*=============================================================================*/
unsigned int  wmt_get_dma_pos(dmach_t ch)
{
	struct dma_mem_reg_group_s dma_mem_reg;
	struct dma_info_s *dma;
	unsigned long flags;
	unsigned int now_time;
	unsigned int delay_time;
	dma = &dma_chan[ch];

	if ((ch >= MAX_DMA_CHANNELS) ||
	     (dma_chan[ch].in_use == 0)) {
		DPRINTK("%s: bad DMA identifier\n", __func__);
		goto out;
	}
	if (dma->regs->DMA_CCR_CH[ch] & (SYSTEM_DMA_RUN)) {
		spin_lock_irqsave(&dma_list_lock, flags);
		dma->regs->DMA_CCR_CH[ch] |= (DMA_UP_MEMREG_EN);/*update memory register before reading*/
		now_time = wmt_read_oscr();
		while (dma->regs->DMA_CCR_CH[ch] & (DMA_UP_MEMREG_EN)) {
			delay_time = wmt_read_oscr() - now_time;
			if (delay_time > 15) {/*5us*/
				dma->regs->DMA_CCR_CH[ch] &= ~DMA_UP_MEMREG_EN;/*clear DMA_UP_MEMREG_EN*/
				break;
			}
		}
		spin_unlock_irqrestore(&dma_list_lock, flags);
	}

	dma_mem_reg.DMA_IF0BAR_CH = dma_mem_regs->mem_reg_group[ch].DMA_IF0BAR_CH;
	dma_mem_reg.DMA_IF0CPR_CH = dma_mem_regs->mem_reg_group[ch].DMA_IF0CPR_CH;
	dma_mem_reg.DMA_IF0RBR_CH = dma_mem_regs->mem_reg_group[ch].DMA_IF0RBR_CH;
	dma_mem_reg.DMA_IF0DAR_CH = dma_mem_regs->mem_reg_group[ch].DMA_IF0DAR_CH;
	dma_mem_reg.DMA_IF1BAR_CH = dma_mem_regs->mem_reg_group[ch].DMA_IF1BAR_CH;
	dma_mem_reg.DMA_IF1CPR_CH = dma_mem_regs->mem_reg_group[ch].DMA_IF1CPR_CH;
	dma_mem_reg.DMA_IF1RBR_CH = dma_mem_regs->mem_reg_group[ch].DMA_IF1RBR_CH;
	dma_mem_reg.DMA_IF1DAR_CH = dma_mem_regs->mem_reg_group[ch].DMA_IF1DAR_CH;

out:
	return dma_mem_reg.DMA_IF0DAR_CH;
}


/*===========================================================================*/
/*  wmt_dma_busy*/
/**/
/*  return: 1: busy , 0: not busy , other: fail*/
/*===========================================================================*/
int wmt_dma_busy(dmach_t ch)
{
	struct dma_info_s *dma;

	dma = &dma_chan[ch];
	if (ch >= MAX_DMA_CHANNELS || (dma_chan[ch].in_use == 0)) {
		DPRINTK("%s: bad DMA identifier\n", __func__);
		return -1 ;
	}

	if (dma->regs->DMA_ISR & (1<<ch)) {
		dma->regs->DMA_ISR = (1<<ch); /*write 1 clear*/
		return 0 ;
	} else
		return 1;
}

/*===========================================================================*/
/*  wmt_dump_dma_regs*/
/**/
/*  return: NULL*/
/*===========================================================================*/
void wmt_dump_dma_regs(dmach_t ch)
{
	struct dma_info_s *dma = &dma_chan[ch] ;
	struct dma_regs_s *regs  = dma->regs ;
	unsigned long flags;
	unsigned int now_time = 0;
	unsigned int delay_time = 0;

	printk("0x%8.8X : [0x%8.8X] GCR \n", \
		(unsigned int)&(regs->DMA_GCR) , (unsigned int)regs->DMA_GCR) ;
	printk("0x%8.8X : [0x%8.8X] MPRP \n", \
		(unsigned int)&(regs->DMA_MRPR) , (unsigned int)regs->DMA_MRPR) ;
	printk("0x%8.8X : [0x%8.8X] IER \n", \
		(unsigned int)&(regs->DMA_IER) , (unsigned int)regs->DMA_IER) ;
	printk("0x%8.8X : [0x%8.8X] ISR \n", \
		(unsigned int)&(regs->DMA_ISR) , (unsigned int)regs->DMA_ISR) ;
	printk("0x%8.8X : [0x%8.8X] TMR \n", \
		(unsigned int)&(regs->DMA_TMR) , (unsigned int)regs->DMA_TMR) ;
	printk("0x%8.8X : [0x%8.8X] CCR \n", \
		(unsigned int)&(regs->DMA_CCR_CH[ch]) , (unsigned int)regs->DMA_CCR_CH[ch]) ;

	if (dma->regs->DMA_CCR_CH[ch] & (SYSTEM_DMA_RUN)) {
		spin_lock_irqsave(&dma_list_lock, flags);
		dma->regs->DMA_CCR_CH[ch] |= (DMA_UP_MEMREG_EN);/*update memory register before reading*/

		now_time = wmt_read_oscr();
		while (dma->regs->DMA_CCR_CH[ch] &  (DMA_UP_MEMREG_EN)) {
			delay_time = wmt_read_oscr() - now_time;
			if (delay_time > 15) {/*5us*/
				DPRINTK("[%d]Warnning:up_mem_reg did not clear[%x]\n", ch, dma->regs->DMA_CCR_CH[ch]);
				dma->regs->DMA_CCR_CH[ch] &= ~DMA_UP_MEMREG_EN;/*clear DMA_UP_MEMREG_EN*/
				break;
			}

		}

		spin_unlock_irqrestore(&dma_list_lock, flags);
	}
	printk("0x%8.8X : [0x%8.8X] Residue Bytes 0 \n", \
		(unsigned int)&(dma_mem_regs->mem_reg_group[ch].DMA_IF0RBR_CH)
		, (unsigned int)dma_mem_regs->mem_reg_group[ch].DMA_IF0RBR_CH) ;
	printk("0x%8.8X : [0x%8.8X] Data Address 0 \n", \
		(unsigned int)&(dma_mem_regs->mem_reg_group[ch].DMA_IF0DAR_CH)
		, (unsigned int)dma_mem_regs->mem_reg_group[ch].DMA_IF0DAR_CH) ;
	printk("0x%8.8X : [0x%8.8X] Branch Address 0 \n", \
		(unsigned int)&(dma_mem_regs->mem_reg_group[ch].DMA_IF0BAR_CH)
		, (unsigned int)dma_mem_regs->mem_reg_group[ch].DMA_IF0BAR_CH) ;
	printk("0x%8.8X : [0x%8.8X] Command Pointer  0 \n", \
		(unsigned int)&(dma_mem_regs->mem_reg_group[ch].DMA_IF0CPR_CH)
		, (unsigned int)dma_mem_regs->mem_reg_group[ch].DMA_IF0CPR_CH) ;

	printk("0x%8.8X : [0x%8.8X] Residue Bytes 1 \n", \
		(unsigned int)&(dma_mem_regs->mem_reg_group[ch].DMA_IF1RBR_CH)
		, (unsigned int)dma_mem_regs->mem_reg_group[ch].DMA_IF1RBR_CH) ;
	printk("0x%8.8X : [0x%8.8X] Data Address 1 \n", \
		(unsigned int)&(dma_mem_regs->mem_reg_group[ch].DMA_IF1DAR_CH)
		, (unsigned int)dma_mem_regs->mem_reg_group[ch].DMA_IF1DAR_CH) ;
	printk("0x%8.8X : [0x%8.8X] Branch Address 1 \n", \
		(unsigned int)&(dma_mem_regs->mem_reg_group[ch].DMA_IF1BAR_CH)
		, (unsigned int)dma_mem_regs->mem_reg_group[ch].DMA_IF1BAR_CH) ;
	printk("0x%8.8X : [0x%8.8X] Command Pointer  1 \n", \
		(unsigned int)&(dma_mem_regs->mem_reg_group[ch].DMA_IF1CPR_CH)
		, (unsigned int)dma_mem_regs->mem_reg_group[ch].DMA_IF1CPR_CH) ;

}


#ifdef CONFIG_PM
int dma_suspend(struct sys_device *dev, pm_message_t state)
{
	return 0;
}

int dma_resume(struct sys_device *dev)
{
	struct dma_regs_s *dma_regs ;
	auto_pll_divisor(DEV_DMA, CLK_ENABLE, 0, 0);
	dma_regs = (struct dma_regs_s *) (io_p2v(DMA_CTRL_CFG_BASE_ADDR)) ;
	/*dma_regs->GCR = ( DMA_GCR_GDMA_ENABLE | DMA_GCR_PRIORITY_FIXED | DMA_GCR_GINT_ENABLE ) ;*/
	dma_regs->DMA_GCR |= DMA_GLOBAL_EN;
	dma_regs->DMA_ISR = ALL_INT_CLEAR;
	dma_regs->DMA_IER |= ALL_INT_EN;
	dma_regs->DMA_TMR &= ~SCHEDULE_RR_DISABLE; /*use RR schedule*/
	dma_regs->DMA_MRPR = (unsigned int)dma_mem_phy;
	return 0;
}

#else
#define dma_suspend NULL
#define dma_resume  NULL
#endif /* CONFIG_PM */


static struct sysdev_class dma_sysclass = {
	.name		= "wmt_dma",
	.suspend	= dma_suspend,
	.resume		= dma_resume,
};

static struct sys_device dma_device = {
	.id		= 1,
	.cls		= &dma_sysclass,
};

static int __init wmt_dma_init_devicefs(void)
{
	sysdev_class_register(&dma_sysclass);
	return sysdev_register(&dma_device);
}

device_initcall(wmt_dma_init_devicefs);


/* dma_read_proc()
 *
 * Entry for reading dma controller status
 *
 * We created a node in /proc/driver/dma
 * Using "cat /proc/driver/dma" for debugging.
 *
 * TODO: migrate to sysfs in the future.
 */
/*
static int dma_read_proc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	char *p = page;
	int len;

	p += sprintf(p, "dma global status , TBD\n");
	// TBD, using vt8500_dump_dma_regs()?
	len = (p - page) - off;
	if (len < 0)
		len = 0;
	*eof = len <= count;
	*start = page + off;

	return len;
}
*/
/*===========================================================================*/
/*  wmt_dma_init*/
/**/
/*  return: 0*/
/*===========================================================================*/
static int __init
wmt_dma_init(void)
{
	int ch ;
	int ret = 0;
	struct dma_regs_s *dma_regs ;
	/*
	*(volatile unsigned int *)(0xD8130254) |= BIT5;
	*/
	auto_pll_divisor(DEV_DMA, CLK_ENABLE, 0, 0);

	dma_regs = (struct dma_regs_s *) (io_p2v(DMA_CTRL_CFG_BASE_ADDR)) ;

	/**/
	/* software initial*/
	/**/
	dma_int.request_chans = 0 ;
	dma_int.regs = (struct dma_regs_s *) (io_p2v(DMA_CTRL_CFG_BASE_ADDR)) ;

	for (ch = 0 ; ch < MAX_DMA_CHANNELS ; ++ch) {
		dma_chan[ch].channel_no = ch ;
		dma_chan[ch].regs       = (struct dma_regs_s *) (io_p2v(DMA_CTRL_CFG_BASE_ADDR)) ;
		dma_chan[ch].irq        = ch ;
		dma_chan[ch].device_no  = DEVICE_RESERVED ;
		dma_chan[ch].in_use     = 0 ;
	}
	/*
	dma_mem_regs = (struct dma_mem_reg_s *)dma_alloc_coherent(NULL,
					sizeof(struct dma_mem_reg_s), &dma_mem_phy, GFP_KERNEL);
	*/
	dma_mem_regs = (struct dma_mem_reg_s *) (io_p2v((DMA_CTRL_CFG_BASE_ADDR + DMA_MEM_REG_OFFSET)));
	if (!dma_mem_regs) {
		printk("dma memory register allocate failed\n");
		ret = -1;
		return ret;
	}
	dma_mem_phy = DMA_CTRL_CFG_BASE_ADDR + DMA_MEM_REG_OFFSET;
	DPRINTK("MEM_REGS ADDR:Virt = 0x%x , Phy = 0x%x\n", dma_mem_regs , dma_mem_phy);

	if (dma_mem_phy & 0x000000FF) {/*8 DW alignment*/
		printk("dma memory registers did not 8 DW alignment");
		ret = -1;
		return ret;
	}

	/**/
	/* hardware initial*/
	/**/
	dma_regs->DMA_GCR |= DMA_SW_RST ;
	dma_regs->DMA_GCR |= DMA_GLOBAL_EN;
	dma_regs->DMA_ISR = ALL_INT_CLEAR;
	dma_regs->DMA_IER |= ALL_INT_EN;
	dma_regs->DMA_TMR &= ~SCHEDULE_RR_DISABLE; /*use RR schedule*/
	dma_regs->DMA_MRPR = (unsigned int)dma_mem_phy;
	DPRINTK("0x%8.8X : [0x%8.8X] DMA_GSR_REG \n", \
		(unsigned int)&dma_regs->GSR , dma_regs->GSR) ;

	for (ch = 0 ; ch < MAX_DMA_CHANNELS ; ++ch) {
		dma_mem_regs->mem_reg_group[ch].DMA_IF0CPR_CH = 0x100;
		dma_mem_regs->mem_reg_group[ch].DMA_IF1CPR_CH = 0x100;
		dma_regs->DMA_CCR_CH[ch] = 0x0;
	}
	DPRINTK("0x%8.8X : [0x%8.8X] DMA_GCR_REG \n", \
		(unsigned int)&dma_regs->DMA_GCR , dma_regs->DMA_GCR) ;
	for (ch = 0; ch < MAX_DMA_CHANNELS ; ++ch)
		request_irq(dma_irq_no[ch], dma_irq_handler, IRQF_DISABLED, "dma", NULL);

	return ret;
}

/*===========================================================================*/
/*  dma_exit*/
/**/
/*  return: 0*/
/*===========================================================================*/
static void __exit
wmt_dma_exit(void)
{
	int ch;
	/*remove_proc_entry("driver/dma", NULL);*/
	dma_free_coherent(NULL,
		sizeof(struct dma_mem_reg_s),
		dma_mem_regs,
		dma_mem_phy);

	for (ch = 0; ch < MAX_DMA_CHANNELS ; ++ch)
		free_irq(dma_irq_no[ch], NULL);
}

__initcall(wmt_dma_init);
__exitcall(wmt_dma_exit);

EXPORT_SYMBOL(wmt_request_dma);
EXPORT_SYMBOL(wmt_setup_dma);
EXPORT_SYMBOL(wmt_free_dma);
EXPORT_SYMBOL(wmt_clear_dma);
EXPORT_SYMBOL(wmt_reset_dma);
EXPORT_SYMBOL(wmt_start_dma);
EXPORT_SYMBOL(wmt_stop_dma);
EXPORT_SYMBOL(wmt_resume_dma);
EXPORT_SYMBOL(wmt_get_dma_pos_info);
EXPORT_SYMBOL(wmt_dma_busy);
EXPORT_SYMBOL(wmt_dump_dma_regs);
