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

#undef	DEBUG
#undef	VERBOSE

#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/list.h>
#include <linux/interrupt.h>
#include <linux/proc_fs.h>
#include <linux/mm.h>
#include <linux/moduleparam.h>
#include <linux/device.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <linux/usb/otg.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/string.h>

#include <asm/byteorder.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/system.h>
#include <asm/unaligned.h>
#include <asm/mach-types.h>

#include "udc_wmt.h"
//#define FULL_SPEED_ONLY
/*#define HW_BUG_HIGH_SPEED_PHY*/
#undef	USB_TRACE

/* bulk DMA seems to be behaving for both IN and OUT */
#define	USE_DMA

/*#define DEBUG_UDC_ISR_TIMER*/
#undef DEBUG_UDC_ISR_TIMER

/*#define RNDIS_INFO_DEBUG_BULK_OUT*/
#undef RNDIS_INFO_DEBUG_BULK_OUT

/*#define RNDIS_INFO_DEBUG_BULK_IN*/
#undef RNDIS_INFO_DEBUG_BULK_IN

/*#define MSC_COMPLIANCE_TEST*/
#undef MSC_COMPLIANCE_TEST

/*#define UDC_A1_SELF_POWER_ENABLE*/

/*-----------------------------------------------------------------------------*/

#define	DRIVER_DESC	"VIA UDC driver"
#define	DRIVER_VERSION	"3 December 2007"

#define	DMA_ADDR_INVALID	(~(dma_addr_t)0)

static unsigned fifo_mode;

/* "modprobe vt8500_udc fifo_mode=42", or else as a kernel
 * boot parameter "vt8500_udc:fifo_mode=42"
 */

module_param(fifo_mode, uint, 0);
MODULE_PARM_DESC(fifo_mode, "endpoint setup (0 == default)");

static unsigned use_dma = 1;

module_param(use_dma, bool, 0);
MODULE_PARM_DESC(use_dma, "enable/disable DMA");

static const char driver_name[] = "wmotgdev";/*"wmt_udc";*/
static const char driver_desc[] = DRIVER_DESC;

static struct vt8500_udc *udc;
static struct device *pDMADescLI, *pDMADescSI;
static struct device *pDMADescLO, *pDMADescSO;
dma_addr_t UdcPdmaPhyAddrLI, UdcPdmaPhyAddrSI;
//static unsigned int UdcPdmaVirAddrLI, UdcPdmaVirAddrSI;
static UINT UdcPdmaVirAddrLI, UdcPdmaVirAddrSI;
dma_addr_t UdcPdmaPhyAddrLO, UdcPdmaPhyAddrSO;
//static unsigned int UdcPdmaVirAddrLO, UdcPdmaVirAddrSO;
static UINT UdcPdmaVirAddrLO, UdcPdmaVirAddrSO;


#ifdef OTGIP
static struct USB_GLOBAL_REG *pGlobalReg;
#endif
/*static struct UDC_REGISTER *udc_reg;*/

static volatile struct UDC_REGISTER *pDevReg;
static volatile struct UDC_DMA_REG *pUdcDmaReg;

PSETUPCOMMAND pSetupCommand;
UCHAR *pSetupCommandBuf;
UCHAR *SetupBuf;
UCHAR *IntBuf;

UCHAR ControlState; /* the state the control pipe*/
UCHAR USBState;
UCHAR TestMode;

/* Basic Define*/
//#define REG32               *(volatile unsigned int *)
//#define REG_GET32(addr)		(REG32(addr))		/* Read 32 bits Register */

static unsigned long	irq_flags;//gri

volatile unsigned char *pUSBMiscControlRegister5;

unsigned int interrupt_transfer_size;

static void wmt_pdma_reset(void);
static void wmt_pdma0_reset(void);
static void wmt_pdma1_reset(void);

/* 1 : storage */
static unsigned int gadget_mode=0;
static unsigned int gadget_mount=0;
static unsigned int uevent_mount=0;
static unsigned int gadget_connect=0;
static unsigned int first_mount=0;


struct work_struct	mount_thread;
struct work_struct	umount_thread;
struct work_struct	reinsmod_thread;
struct work_struct	online_thread;
struct work_struct	offline_thread;
struct work_struct	done_thread;



struct list_head	done_main_list;
spinlock_t			      gri_lock;
static unsigned long	gri_flags;//gri

static unsigned int uevent_id=0;
static unsigned int handled_id=0;

/**
 *
 *	Run these two functions to execute shell script to mount / umount storage
 *	
 */
#if 0
static int run_online(void)
{
    kobject_uevent(&udc->dev->kobj, 2);    
	return 0;	
}
#endif

static void run_offline (struct work_struct *work)
{
	if (udc->driver){
//		printk(KERN_INFO "disconnect\n"); //gri
		udc->driver->disconnect(&udc->gadget);   
	}
//	return 0;	
}

static void run_done (struct work_struct *work)
{
		struct vt8500_req * req;

		spin_lock_irqsave(&gri_lock, gri_flags);		
//		printk(KERN_INFO "d1\n"); //gri
		while (!list_empty(&done_main_list)){
//			printk(KERN_INFO "d3\n"); //gri
			req = list_entry(done_main_list.next, struct vt8500_req, mem_list);
			list_del_init(&req->mem_list);
			spin_unlock_irqrestore(&gri_lock, gri_flags);				
			req->req.complete(&req->ep->ep, &req->req);
			spin_lock_irqsave(&gri_lock, gri_flags);		
		}
//		printk(KERN_INFO "d2\n"); //gri
		spin_unlock_irqrestore(&gri_lock, gri_flags);				
//	return 0;	
}

static void run_mount (struct work_struct *work)
{
//	int ret;
//    char *argv[] = { "/system/etc/wmt/script/usb.sh", "", NULL };
//	char *envp[] =
//		{ "ACTION=mount", NULL };

//	int ret;
//    char *argv[] = { "/system/etc/wmt/script/usb.sh mount", "", NULL };
//	char *envp[] =
//		{ NULL };
//
//	ret = call_usermodehelper(argv[0], argv, envp, 0);

//    unsigned int uevent_id_last;
//    uevent_id_last = uevent_id;
		handled_id++;
		if (handled_id < uevent_id)
		{
      printk(KERN_INFO "cancel mount gadget_mount=%d uevent_id=%d\n",gadget_mount,uevent_id);
			return;
		}		
		
		
    msleep_interruptible(300);
    if ((handled_id == uevent_id) && (uevent_mount!=gadget_mount)){
        uevent_mount=gadget_mount;
        kobject_uevent(&udc->dev->kobj, KOBJ_ADD);
        printk(KERN_INFO "mount_thread\n");
    } else {
        printk(KERN_INFO "cancel mount uevent_mount=%d gadget_mount=%d uevent_id=%d\n",uevent_mount,gadget_mount,uevent_id);
    }
//	return ret;
//	return 0;	
}

static void run_umount(struct work_struct *work)
{
//	int ret;
//    char *argv[] = { "/system/etc/wmt/script/usb.sh", "", NULL };
//	char *envp[] =
//		{ "ACTION=umount", NULL };

//	int ret;
//    char *argv[] = { "/system/etc/wmt/script/usb.sh umount", "", NULL };
//	char *envp[] =
//		{ NULL };
//
//	ret = call_usermodehelper(argv[0], argv, envp, 0);

//    unsigned int uevent_id_last;
//    uevent_id_last = uevent_id;
		handled_id++;
		if (handled_id < uevent_id)
		{
      printk(KERN_INFO "cancel umount gadget_mount=%d uevent_id=%d\n",gadget_mount,uevent_id);
			return;	
		}

    msleep_interruptible(300);
    if ((handled_id == uevent_id) && (uevent_mount!=gadget_mount)){
        uevent_mount=gadget_mount;        
        kobject_uevent(&udc->dev->kobj, KOBJ_REMOVE);
        printk(KERN_INFO "umount_thread\n");
    } else {
        printk(KERN_INFO "cancel umount uevent_mount=%d gadget_mount=%d uevent_id=%d\n",uevent_mount,gadget_mount,uevent_id);
    }
    
//	return ret;
//	return 0;	
}

static void run_reinsmod(struct work_struct *work)
{
//	int ret;
//    char *argv[] = { "/system/etc/wmt/script/usb.sh", "", NULL };
//	char *envp[] =
//		{ "ACTION=reinsmod", NULL };

	int ret;
    char *argv[] = { "/system/etc/wmt/script/usb.sh reinsmod", "", NULL };
	char *envp[] =
		{ NULL };

	ret = call_usermodehelper(argv[0], argv, envp, 0);
//	return ret;
}

static void run_script(int dwAction)
{
//    printk(KERN_INFO "gri run_script %d %d %d\n",dwAction,gadget_mode
//        ,gadget_mount);
#if 0
    if (dwAction == 4){
        if (gadget_mode)
        {
            schedule_work(&online_thread);
            printk(KERN_INFO "on_line\n");             
        }
    } else if (dwAction == 3){
        if (gadget_mode)
        {
            schedule_work(&offline_thread);
            printk(KERN_INFO "off_line\n");             
        }
    } else if (dwAction == 2){
#endif
		if (dwAction == 8){
        schedule_work(&done_thread);
    }else if (dwAction == 7){    
        schedule_work(&offline_thread);
    }else if (dwAction == 2){    
//        if (gadget_mode && ((gadget_mount== 0) || (first_mount==0)))
        if ((gadget_mount== 0) || (first_mount==0))            
        {
            if (!first_mount){
                uevent_mount= 0;
                first_mount=1;        
            }        
            gadget_mount=1;
            uevent_id++;
            schedule_work(&mount_thread);
            printk(KERN_INFO "mount_thread\n");            
        }
    } else if (dwAction){
        if (gadget_mode && gadget_connect && ((gadget_mount== 1) || (first_mount==0)))
        {
            if (!first_mount){
                uevent_mount= 1;
                first_mount=1;        
            }
            gadget_mount=0;
            uevent_id++;
            schedule_work(&umount_thread);
            printk(KERN_INFO "umount_thread 01\n");
        }    
    }else {
//        gadget_mount=0;
//        schedule_work(&umount_thread);    
//        printk(KERN_INFO "umount_thread 02\n");
    }
    
}


void udc_device_dump_register(void)
{
   volatile unsigned int  address;
   volatile unsigned char temp8;
   int i;

   for (i = 0x20; i <= 0x3F; i++) {  /*0x20~0x3F  (0x3F-0x20 + 1) /4 = 8 DWORD*/
      address =  (USB_UDC_REG_BASE + i);
      temp8 = REG_GET8(address);

      INFO("[UDC Device] Offset[0x%8.8X] = 0x%2.2X \n", address, temp8);
   }

   for (i = 0x308; i <= 0x30D; i++) { /*0x308~0x30D*/
      address =  (USB_UDC_REG_BASE + i);
      temp8 = REG_GET8(address);

      INFO("[UDC Device] Offset[0x%8.8X] = 0x%2.2X \n", address, temp8);
   }

   for (i = 0x310; i <= 0x317; i++) { /*0x310~0x317*/
      address =  (USB_UDC_REG_BASE + i);
      temp8 = REG_GET8(address);

      INFO("[UDC Device] Offset[0x%8.8X] = 0x%2.2X \n", address, temp8);
   }

   INFO("[UDC Device] Dump Reigster PASS!\n");

} /*void udc_device_dump_register(void)*/

void udc_bulk_dma_dump_register(void)
{
   volatile unsigned int  temp32;
   int i;

   for (i = 0; i <= 0x10; i++) { /*0x100 ~ 0x113 (0x113 - 0x100 + 1) = 0x14 /4 = 5*/
      temp32 = REG_GET32(USB_UDC_DMA_REG_BASE + i*4);
      INFO("[UDC Bulk DMA] Offset[0x%8.8X] = 0x%8.8X \n", (USB_UDC_DMA_REG_BASE + i*4), temp32);
   }
   INFO("[UDC Bulk DMAD] Dump Reigster PASS!\n");

} /*void udc_bulk_dma_dump_register(void)*/

static int wmt_ep_enable(struct usb_ep *_ep,
		const struct usb_endpoint_descriptor *desc)
{
	struct vt8500_ep	*ep = container_of(_ep, struct vt8500_ep, ep);
	struct vt8500_udc	*udc;
//	unsigned long	flags;
	u16		maxp;

    DBG("wmt_ep_enable() %s\n", ep ? ep->ep.name : NULL);
//	printk(KERN_INFO "gri wmt_ep_enable() %s\n", ep ? ep->ep.name : NULL);

	/* catch various bogus parameters*/
	if (!_ep || !desc || ep->desc
			|| desc->bDescriptorType != USB_DT_ENDPOINT
			|| ep->bEndpointAddress != desc->bEndpointAddress
			|| ep->maxpacket < le16_to_cpu(desc->wMaxPacketSize)) {
		VDBG("ep->bEndpointAddress = 0x%08X\n", ep->bEndpointAddress);
		VDBG("desc->bEndpointAddres = 0x%08X\n", desc->bEndpointAddress);
		VDBG("ep->maxpacket =0x%08X\n", ep->maxpacket);
		VDBG("desc->wMaxPacketSize =0x%08X\n", desc->wMaxPacketSize);

		VDBG("_ep =0x%08X\n", (unsigned int)_ep);
		VDBG("desc =0x%08X\n", (unsigned int)desc);
		VDBG("ep->desc =0x%08X\n", (unsigned int)ep->desc);

		/*DBG("%s, bad ep or descriptor\n", __FUNCTION__);*/
		return -EINVAL;
	}

	maxp = le16_to_cpu(desc->wMaxPacketSize);

	if ((desc->bmAttributes == USB_ENDPOINT_XFER_BULK
				&& maxp != ep->maxpacket)
			|| desc->wMaxPacketSize > ep->maxpacket
			|| !desc->wMaxPacketSize) {
		DBG("[wmt_ep_enable]bad %s maxpacket\n", _ep->name);
		return -ERANGE;
	}

	if (desc->bmAttributes == USB_ENDPOINT_XFER_ISOC) {
		DBG("%s, ISO nyet\n", _ep->name);
		return -EDOM;
	}

	/* xfer types must match, except that interrupt ~= bulk*/
	if (ep->bmAttributes != desc->bmAttributes
			&& ep->bmAttributes != USB_ENDPOINT_XFER_BULK
			&& desc->bmAttributes != USB_ENDPOINT_XFER_INT) {
		DBG("[wmt_ep_enable], %s type mismatch\n", _ep->name);
		return -EINVAL;
	}

	udc = ep->udc;

	spin_lock_irqsave(&udc->lock, irq_flags);

	ep->desc = desc;
	ep->irqs = 0;
	ep->stopped = 0;
	ep->ep.maxpacket = maxp;

	ep->has_dma = 1;
	ep->ackwait = 0;

	switch ((ep->bEndpointAddress & 0x7F)) {
	case 0:/*Control In/Out*/
		pDevReg->ControlEpControl = EP_RUN + EP_ENABLEDMA;
	break;

	case 1:/*Bulk In*/
		pDevReg->Bulk1EpControl = EP_RUN + EP_ENABLEDMA;
	break;

	case 2:/*Bulk Out*/
		pDevReg->Bulk2EpControl = EP_RUN + EP_ENABLEDMA;
	break;

	case 3:/*Interrupt In*/
		pDevReg->InterruptEpControl = EP_RUN + EP_ENABLEDMA;
	break;
	}

	spin_unlock_irqrestore(&udc->lock, irq_flags);
	VDBG("%s enabled\n", _ep->name);
//	printk(KERN_INFO "gri enabled %s\n", ep ? ep->ep.name : NULL);
	
	return 0;
} /*static int wmt_ep_enable()*/

static void nuke(struct vt8500_ep *, int status);

static int wmt_ep_disable(struct usb_ep *_ep)
{
    struct vt8500_ep	*ep = container_of(_ep, struct vt8500_ep, ep);
//	unsigned long	flags;

    DBG("wmt_ep_disable() %s\n", ep ? ep->ep.name : NULL);

	if (!_ep || !ep->desc) {
		DBG("[wmt_ep_disable], %s not enabled\n",
			_ep ? ep->ep.name : NULL);
		return -EINVAL;
	}

	spin_lock_irqsave(&ep->udc->lock, irq_flags);
	ep->desc = 0;
	nuke(ep, -ESHUTDOWN);
	ep->ep.maxpacket = ep->maxpacket;
	ep->has_dma = 0;

	del_timer(&ep->timer);

	switch ((ep->bEndpointAddress & 0x7F)) {
	case 0:/*Control In/Out*/
		pDevReg->ControlEpControl &= 0xFC;
	break;

	case 1:/*Bulk In*/
		pDevReg->Bulk1EpControl &= 0xFC;
	break;

	case 2:/*Bulk Out*/
		pDevReg->Bulk2EpControl &= 0xFC;
	break;

	case 3:/*Interrupt In*/
		pDevReg->InterruptEpControl &= 0xFC;
	break;
	}

	spin_unlock_irqrestore(&ep->udc->lock, irq_flags);

	VDBG("%s disabled\n", _ep->name);
	return 0;
} /*static int wmt_ep_disable()*/


static struct usb_request *
wmt_alloc_request(struct usb_ep *_ep, gfp_t gfp_flags)
{
	struct vt8500_req	*req;
	struct vt8500_ep	*ep = NULL;

//	DBG("gri wmt_alloc_request() %s\n", ep->name);
	
	ep = container_of(_ep, struct vt8500_ep, ep);
//  printk(KERN_INFO "gri wmt_alloc_request() %s\n", ep->name);
//  if ((ep->bEndpointAddress & 0x7F) > 4)
//      return NULL;     


  /*if(ep->bEndpointAddress == 3)*/
  /*   INFO("wmt_alloc_request() %s\n", ep->name);*/

	req = kmalloc(sizeof *req, gfp_flags);
	if (req) {
		memset(req, 0, sizeof *req);
		req->req.dma = DMA_ADDR_INVALID;
		INIT_LIST_HEAD(&req->queue);
	}
	return &req->req;/*struct usb_request for file_storage.o*/
} /*wmt_alloc_request()*/

static void
wmt_free_request(struct usb_ep *_ep, struct usb_request *_req)
{
	struct vt8500_req	*req = container_of(_req, struct vt8500_req, req);
  struct vt8500_ep	*ep = NULL;

	DBG("wmt_free_request() %s\n", ep->name);
	ep = container_of(_ep, struct vt8500_ep, ep);

	/*if(ep->bEndpointAddress == 3)*/
	/*	 INFO("wmt_free_request() %s\n", ep->name);*/

	if (_req)
		kfree(req);
} /*wmt_free_request()*/

/*EP0 - Control 256 bytes*/
/*EP2,3 Bulk In/Out 16384 bytes (16K)*/
/*file_storeage.c - req->buf*/
/*struct usb_request req;*/
static void wmt_pdma_init(struct device *dev);

static void *
wmt_alloc_buffer(
	struct usb_ep	*_ep,
	unsigned	bytes,
	dma_addr_t	*dma,
	gfp_t		gfp_flags
)
{
	void *retval = NULL;
	struct vt8500_ep	*ep;

  DBG("wmt_alloc_buffer() %s\n", _ep->name);
	ep = container_of(_ep, struct vt8500_ep, ep);

	retval = dma_alloc_coherent(ep->udc->dev, bytes, dma, gfp_flags);

	ep->temp_buffer_address = (unsigned int)retval;
	ep->temp_dma_phy_address = (u32)*dma;

	ep->temp_buffer_address2 = 0;
	ep->temp_dma_phy_address2 = 0;

	DBG("wmt_alloc_buffer() retval(0x%08X) dma(0x%08X)\n", (unsigned int)retval, (u32)*dma);

	if (ep->bEndpointAddress == 0) {/*Control*/
		if ((unsigned int)retval & 0x00000FFF) {
			dma_free_coherent(ep->udc->dev, bytes, retval, *dma);

			bytes = (bytes + 4096); /*4k bytes align*/
			retval = dma_alloc_coherent(ep->udc->dev, bytes, dma, gfp_flags);

			retval = (void *)((unsigned int)retval & 0xFFFFF000); /*4k bytes align*/
			dma = (void *)((unsigned int)dma & 0xFFFFF000);       /*4k bytes align*/
			ep->temp_buffer_address2 = (unsigned int)retval;
			ep->temp_dma_phy_address2 = (u32)*dma;
			INFO("wmt_alloc_buffer(non-align) retval(0x%08X) dma(0x%08X) bytes(0x%08X)\n"
				, (unsigned int)retval, (u32)*dma, (u32)bytes);
		}
	}

	if (((ep->bEndpointAddress & 0x7F) == 1) || (ep->bEndpointAddress == 2)) {/*Bulk In and Out*/
		if ((unsigned int)retval & 0x00000003) {
			dma_free_coherent(ep->udc->dev, bytes, retval, *dma);

			bytes = (bytes + 4); /*4 bytes align*/
			retval = dma_alloc_coherent(ep->udc->dev, bytes, dma, gfp_flags);

			retval = (void *)((unsigned int)retval & 0xFFFFFFFC); /*4 bytes align*/
			dma = (void *)((unsigned int)dma & 0xFFFFFFFC);    /*4 bytes align*/
			ep->temp_buffer_address2 = (unsigned int)retval;
			ep->temp_dma_phy_address2 = (u32)*dma;
			INFO("wmt_alloc_buffer(non-align) retval(0x%08X) dma(0x%08X) bytes(0x%08X)\n"
				, (unsigned int)retval, (u32)*dma, (u32)bytes);
		}
	}

	return retval;
}

static void wmt_free_buffer(
	struct usb_ep	*_ep,
	void *buf,
	dma_addr_t	dma,
	unsigned	bytes
)
{
	struct vt8500_ep	*ep;

    DBG("wmt_free_buffer() %s\n", _ep->name);

	ep = container_of(_ep, struct vt8500_ep, ep);

	if (ep->bEndpointAddress == 0) {/*Control*/
		if (ep->temp_dma_phy_address != (u32)dma) {
			INFO("wmt_free_buffer(non-align) ep->temp_dma_phy_address");
			INFO("(0x%08X) retval(0x%08X) dma(0x%08X) bytes(0x%08X)\n"
				, ep->temp_dma_phy_address, (unsigned int)buf, (u32)dma, (u32)bytes);
			bytes = (bytes + 4096); /*4k bytes align*/
			buf = (void *)ep->temp_buffer_address2;
			dma = ep->temp_dma_phy_address2;
			INFO("wmt_free_buffer(non-align) retval(0x%08X) dma(0x%08X) bytes(0x%08X)\n"
				, (unsigned int)buf, (u32)dma, (u32)bytes);
		}
	}

	if (((ep->bEndpointAddress & 0x7F) == 1) || (ep->bEndpointAddress == 2)) {/*Bulk In and Out*/
		if (ep->temp_dma_phy_address != (u32)dma) {
			INFO("wmt_free_buffer(non-align) ep->temp_dma_phy_address");
			INFO("(0x%08X) retval(0x%08X) dma(0x%08X) bytes(0x%08X)\n"
				, ep->temp_dma_phy_address, (unsigned int)buf, (u32)dma, (u32)bytes);
			bytes = (bytes + 4); /*4 bytes align*/
			buf = (void *)ep->temp_buffer_address2;
			dma = ep->temp_dma_phy_address2;
			INFO("wmt_free_buffer(non-align) retval(0x%08X) dma(0x%08X) bytes(0x%08X)\n"
				, (unsigned int)buf, (u32)dma, (u32)bytes);
		}
	}

	if ((dma != 0) || (buf != 0))/*patch RDNDIS Gadget Driver ERROR (EP1 : interrupt in)*/
		dma_free_coherent(ep->udc->dev, bytes, buf, dma);
}


static void ep0_status(struct vt8500_udc *udc)
{
  struct vt8500_ep	*ep;

  ep = &udc->ep[0];

	if (udc->ep0_in) {	 /*OUT STATUS*/
		if (udc->ep0_status_0_byte == 1) {
			udc->ep0_status_0_byte = 0;

			/* the data phase is OUT*/

			if (ep->toggle_bit)
				pDevReg->ControlDesTbytes = 0x40|CTRLXFER_DATA1;
			else
				pDevReg->ControlDesTbytes = 0x40|CTRLXFER_DATA0;

			pDevReg->ControlDesControl = CTRLXFER_OUT+CTRLXFER_IOC;
			pDevReg->ControlDesStatus = CTRLXFER_ACTIVE;
			ControlState = CONTROLSTATE_STATUS;
		} /*if(udc->ep0_status_0_byte == 1)*/
	} else {  /*IN STATUS*/
		if (udc->ep0_status_0_byte == 1) {
			udc->ep0_status_0_byte = 0;

			if (ep->toggle_bit)
				pDevReg->ControlDesTbytes = 0x00 | CTRLXFER_DATA1;
			else
				pDevReg->ControlDesTbytes = 0x00 | CTRLXFER_DATA0;

			pDevReg->ControlDesTbytes = 0x00 | CTRLXFER_DATA1; /* zero length*/
			pDevReg->ControlDesControl = CTRLXFER_IN + CTRLXFER_IOC;
			pDevReg->ControlDesStatus = CTRLXFER_ACTIVE;
			ControlState = CONTROLSTATE_STATUS;
		} /*if(udc->ep0_status_0_byte == 1)*/
	}

} /*static void ep0_status(struct vt8500_udc *udc)*/


static void
done(struct vt8500_ep *ep, struct vt8500_req *req, int status)
{
	unsigned		stopped = ep->stopped;

    DBG("done() %s\n", ep ? ep->ep.name : NULL);

	list_del_init(&req->queue);

    /*#define	EINPROGRESS	115*/	/* Operation now in progress */
	if (req->req.status == -EINPROGRESS)
		req->req.status = status;
	else {
		if (status != 0)
			status = req->req.status;
	}

	DBG("complete %s req %p stat %d len %u/%u\n",
	ep->ep.name, &req->req, status,
	req->req.actual, req->req.length);

	/* don't modify queue heads during completion callback*/
	ep->stopped = 1;

	if ((ep->bEndpointAddress & 0x7F) == 0){
			//spin_unlock(&ep->udc->lock);
		spin_unlock_irqrestore(&ep->udc->lock, irq_flags);
	/*callback to file-backed usb storage gadget(usb_storage.c)*/
//	VDBG("done - before req->req.complete()\n");
		req->req.complete(&ep->ep, &req->req);
		spin_lock_irqsave(&ep->udc->lock, irq_flags);
	} else {
#if 1
		req->ep = ep;
		INIT_LIST_HEAD (&req->mem_list);
		spin_unlock_irqrestore(&ep->udc->lock, irq_flags);
		//spin_lock(&gri_lock);
		spin_lock_irqsave(&gri_lock, gri_flags);		
		list_add_tail(&req->mem_list, &done_main_list);	
		//spin_unlock(&gri_lock);
		spin_unlock_irqrestore(&gri_lock, gri_flags);
		//ppudc = ep->udc;
		run_script(8);
		spin_lock_irqsave(&ep->udc->lock, irq_flags);		
#else
		spin_unlock_irqrestore(&ep->udc->lock, irq_flags);
	/*callback to file-backed usb storage gadget(usb_storage.c)*/
//	VDBG("done - before req->req.complete()\n");
		req->req.complete(&ep->ep, &req->req);
		spin_lock_irqsave(&ep->udc->lock, irq_flags);
#endif		
	}
	//VDBG("done - after  req->req.complete()\n");
	//spin_lock(&ep->udc->lock);
//	spin_lock_irqsave(&ep->udc->lock, irq_flags);

	if ((ep->bEndpointAddress & 0x7F) == 0)/*Control*/
		if (req->req.actual != 0)
			ep->udc->ep0_status_0_byte = 1;

	ep->stopped = stopped;

} /*done(struct vt8500_ep *ep, struct vt8500_req *req, int status)*/

static u32 dma_src_len(struct vt8500_ep *ep, struct vt8500_req *req)
{
	/*dma_addr_t	end;*/
	/*U32 ep_trans_len;*/
	unsigned start = 0 , end = 0;
	u32 temp32 = 0;

	start = req->req.length;

	if ((ep->bEndpointAddress & 0x7F) ==  0) {/*Control In*/
		end = (pDevReg->ControlDesTbytes & 0x7F);
		temp32 = (ep->ep_fifo_length - end);
		return temp32;
	} else if ((ep->bEndpointAddress & 0x7F) ==  1) {/*Bulk In*/
		end =  (pDevReg->Bulk1DesTbytes2 & 0x03) << 16;
		end |=  pDevReg->Bulk1DesTbytes1 << 8;
		end |=  pDevReg->Bulk1DesTbytes0;
	} else if ((ep->bEndpointAddress & 0x7F) ==  3)/*Interrupt In*/
		end = (pDevReg->InterruptDes >> 4);

	temp32 = (start - end);
	return temp32;
} /*static u32 dma_src_len()*/

static u32 dma_dest_len(struct vt8500_ep *ep, struct vt8500_req *req)
{
	/*dma_addr_t	end;*/
	unsigned start = 0 , end = 0;
	u32 temp32 = 0;

	start = req->req.length;

	if ((ep->bEndpointAddress & 0x7F) ==  0) {/*Control Out*/
		end = (pDevReg->ControlDesTbytes & 0x7F);
		temp32 = (ep->ep_fifo_length - end);
		return temp32;
	} else if (ep->bEndpointAddress ==  2) {/*Bulk Out*/
		end =  (pDevReg->Bulk2DesTbytes2 & 0x03) << 16;
		end |=  pDevReg->Bulk2DesTbytes1 << 8;
		end |=  pDevReg->Bulk2DesTbytes0;
	}

	temp32 = (start - end);
	return temp32;
} /*static u32 dma_dest_len()*/

/* Each USB transfer request using DMA maps to one or more DMA transfers.
 * When DMA completion isn't request completion, the UDC continues with
 * the next DMA transfer for that USB transfer.
 */

static void wmt_udc_pdma_des_prepare(unsigned int size, unsigned int dma_phy
							, unsigned char des_type, unsigned char channel)
{
	unsigned int res_size = size;
	volatile unsigned int trans_dma_phy = dma_phy, des_phy_addr = 0, des_vir_addr = 0;
	unsigned int cut_size = 0x40;

	switch (des_type) {
	case DESCRIPTOT_TYPE_SHORT:
	{
		struct _UDC_PDMA_DESC_S *pDMADescS;

		if (channel == TRANS_OUT) {
			des_phy_addr = (unsigned int) UdcPdmaPhyAddrSO;
			des_vir_addr = UdcPdmaVirAddrSO;
			pUdcDmaReg->DMA_Descriptor_Point1 = (unsigned int)(des_phy_addr);
			cut_size = pDevReg->Bulk2EpMaxLen & 0x3ff;
		}	else if (channel == TRANS_IN) {
			des_phy_addr = (unsigned int) UdcPdmaPhyAddrSI;
			des_vir_addr = UdcPdmaVirAddrSI;
			pUdcDmaReg->DMA_Descriptor_Point0 = (unsigned int)(des_phy_addr);
			cut_size = pDevReg->Bulk1EpMaxLen & 0x3ff;
		}	else
			DBG("!! wrong channel %d\n", channel);

		while (res_size) {
			pDMADescS = (struct _UDC_PDMA_DESC_S *) des_vir_addr;

			pDMADescS->Data_Addr = trans_dma_phy;
			pDMADescS->D_Word0_Bits.Format = 0;

//			if (res_size <= 65535) {
			if (res_size <= 32767) {                
				pDMADescS->D_Word0_Bits.i = 1;
				pDMADescS->D_Word0_Bits.End = 1;
				pDMADescS->D_Word0_Bits.ReqCount = res_size;

				res_size -= res_size;
			} else {
				pDMADescS->D_Word0_Bits.i = 0;
				pDMADescS->D_Word0_Bits.End = 0;
				pDMADescS->D_Word0_Bits.ReqCount = 0x8000 - cut_size;

				res_size -= 0x8000;
				des_vir_addr += (unsigned int)DES_S_SIZE;
				trans_dma_phy += (unsigned int)0x8000;
			}
		}
		break;
	}

	case DESCRIPTOT_TYPE_LONG:
	{
		struct _UDC_PDMA_DESC_L *pDMADescL;

		if (channel == TRANS_OUT) {
			des_phy_addr = (unsigned int) UdcPdmaPhyAddrLO;
			des_vir_addr = UdcPdmaVirAddrLO;
			pUdcDmaReg->DMA_Descriptor_Point1 = (unsigned int)(des_phy_addr);
			cut_size = pDevReg->Bulk2EpMaxLen & 0x3ff;

//        printk(KERN_INFO "wmt_udc_pdma_des_prepare() pUdcDmaReg->DMA_Descriptor_Point=0x%08X", 
//        pUdcDmaReg->DMA_Descriptor_Point1); //gri

//        printk(KERN_INFO "des_vir_addr=0x%08X", 
//        des_vir_addr); //gri            
        
		}	else if (channel == TRANS_IN) {
			des_phy_addr = (unsigned int) UdcPdmaPhyAddrLI;
			des_vir_addr = UdcPdmaVirAddrLI;
			pUdcDmaReg->DMA_Descriptor_Point0 = (unsigned int)(des_phy_addr);
			cut_size = pDevReg->Bulk1EpMaxLen & 0x3ff;

//        printk(KERN_INFO "wmt_udc_pdma_des_prepare() pUdcDmaReg->DMA_Descriptor_Point=0x%08X", 
//        pUdcDmaReg->DMA_Descriptor_Point0); //gri
        
//        printk(KERN_INFO "des_vir_addr=0x%08X", 
//        des_vir_addr); //gri    
           
		}	else
			DBG("!! wrong channel %d\n", channel);

        memset((void *)des_vir_addr, 0, 0x100);

		while (res_size) {
			pDMADescL = (struct _UDC_PDMA_DESC_L *) des_vir_addr;

			pDMADescL->Data_Addr = trans_dma_phy;
			pDMADescL->D_Word0_Bits.Format = 1;

//			if (res_size <= 65535) {
			if (res_size <= 32767) {                
				pDMADescL->D_Word0_Bits.i = 1;
				pDMADescL->D_Word0_Bits.End = 1;
				pDMADescL->Branch_addr = 0;
				pDMADescL->D_Word0_Bits.ReqCount = res_size;

				res_size -= res_size;
			} else {
				pDMADescL->D_Word0_Bits.i = 0;
				pDMADescL->D_Word0_Bits.End = 0;
				pDMADescL->Branch_addr = des_vir_addr + (unsigned int)DES_L_SIZE;
				pDMADescL->D_Word0_Bits.ReqCount = 0x8000 - cut_size;

				res_size -= 0x8000;
				des_vir_addr += (unsigned int)DES_L_SIZE;
				trans_dma_phy += (unsigned int)0x8000;
			}
		}

//        printk(KERN_INFO "pDMADescL(0x%08X)\n", 
//        des_vir_addr); //gri        
		break;
	}

	case DESCRIPTOT_TYPE_MIX:
	break;
	default:
	break;
	}
} /*wmt_udc_pdma_des_prepare*/

static void next_in_dma(struct vt8500_ep *ep, struct vt8500_req *req)
{
	u32 temp32;
	u32 dma_ccr = 0;
	unsigned int length;
	u32	dcmd;
    u32	buf;
    int	is_in, i;
    u8	*pctrlbuf, *pintbuf;

//    printk(KERN_INFO "next_in_dma s\n"); //gri

    dcmd = length = req->req.length - req->req.actual;/*wmt_ep_queue() : req->req.actual = 0*/
    buf = ((req->req.dma + req->req.actual) & 0xFFFFFFFC);

    is_in = 1;/*ep->bEndpointAddress & USB_DIR_IN;*/

    DBG("next_in_dma() %s\n", ep ? ep->ep.name : NULL);

#ifdef RNDIS_INFO_DEBUG_BULK_IN
	if ((ep->bEndpointAddress & 0x7F) == 1)
		INFO("next_in_dma() %s\n", ep ? ep->ep.name : NULL);
#endif

	if (((ep->bEndpointAddress & 0x7F) == 1) && (req->req.dma == 0xFFFFFFFF) && (ep->rndis == 0)) {
		void *retval;
		dma_addr_t	dma;
		unsigned int dma_length = 65536;

printk(KERN_INFO "rndis xxxxx %d\n",req->req.length);

		retval = dma_alloc_coherent(ep->udc->dev,
							dma_length, &dma, GFP_ATOMIC);

		ep->rndis_buffer_address = (unsigned int)retval;
		ep->rndis_dma_phy_address = (u32)dma;
		ep->rndis_buffer_length = dma_length;
		ep->rndis = 1;

	}
	if (((ep->bEndpointAddress & 0x7F) == 1) && (ep->rndis == 1) && (req->req.length > ep->rndis_buffer_length)) {
		//void *retval;
		//dma_addr_t	dma;            

printk(KERN_INFO "rndis ooooobb %d\n",req->req.length);
#if 0
			dma_free_coherent (ep->udc->dev,
			ep->rndis_buffer_length,
			(void *)ep->rndis_buffer_address, (dma_addr_t)ep->rndis_dma_phy_address);	

		retval = dma_alloc_coherent(ep->udc->dev,
							req->req.length, &dma, GFP_ATOMIC);

		ep->rndis_buffer_address = (unsigned int)retval;
		ep->rndis_dma_phy_address = (u32)dma;
		ep->rndis_buffer_length = req->req.length;
		ep->rndis = 1;
#endif		
	}	

	if ((ep->bEndpointAddress & 0x7F) == 0) {/*Control*/
		ep->ep_fifo_length = 0;

		if (length >= 64)
			length = 64;

		ep->ep_fifo_length = length;
		pctrlbuf = (u8 *)(req->req.buf + req->req.actual);

		for (i = 0; i < length; i++)
			SetupBuf[i] = pctrlbuf[i];

		if (ep->toggle_bit)
			pDevReg->ControlDesTbytes = length | CTRLXFER_DATA1;
		else
			pDevReg->ControlDesTbytes = length | CTRLXFER_DATA0;

		pDevReg->ControlDesControl = CTRLXFER_IN + CTRLXFER_IOC;
		pDevReg->ControlDesStatus = CTRLXFER_ACTIVE;
		ControlState = CONTROLSTATE_DATA;

		if (ep->toggle_bit == 0)
			ep->toggle_bit = 1;
		else
			ep->toggle_bit = 0;
	} else if (((ep->bEndpointAddress & 0x7F) ==  1)) {/*Bulk In*/
		ep->stall_more_processing  = 0;

//        printk(KERN_INFO "next_in_dma %d %d %d\n", length, req->req.length,req->req.actual); //gri

		if (dcmd > UBE_MAX_DMA)
			dcmd = UBE_MAX_DMA;

		if (pDevReg->Bulk1EpControl & EP_STALL) {
			ep->stall_more_processing = 1;
			ep->temp_dcmd = dcmd;
		}

		if (ep->rndis == 1) {
			memcpy((void *)((u32)ep->rndis_buffer_address), (void *)((u32)req->req.buf), length);
			wmt_udc_pdma_des_prepare(dcmd,
			((ep->rndis_dma_phy_address + req->req.actual) & 0xFFFFFFFC),
			DESCRIPTOT_TYPE_LONG, TRANS_IN);
		} else
			wmt_udc_pdma_des_prepare(dcmd, buf, DESCRIPTOT_TYPE_LONG, TRANS_IN);

		if (pDevReg->Bulk1EpControl & EP_STALL)
			ep->temp_bulk_dma_addr = buf;

		if (pDevReg->Bulk1EpControl & EP_STALL)
			ep->temp_dma_ccr = dma_ccr;

		pDevReg->Bulk1DesStatus = 0x00;
		pDevReg->Bulk1DesTbytes2 |= (dcmd >> 16) & 0x3;
		pDevReg->Bulk1DesTbytes1 = (dcmd >> 8) & 0xFF;
		pDevReg->Bulk1DesTbytes0 = dcmd & 0xFF;

		/* set endpoint data toggle*/
		if (is_in) {
			if (ep->toggle_bit)
				pDevReg->Bulk1DesTbytes2 |= 0x40;/* BULKXFER_DATA1;*/
			else
				pDevReg->Bulk1DesTbytes2 &= 0xBF;/*(~BULKXFER_DATA1);*/

			pDevReg->Bulk1DesStatus = (BULKXFER_IOC | BULKXFER_IN);
		} /*if(is_in)*/

		if (pDevReg->Bulk1EpControl & EP_STALL)
			ep->ep_stall_toggle_bit = ep->toggle_bit;


		/*if((ep->bEndpointAddress & 0x7F) != 0)//!Control*/
		if (req->req.length > ep->maxpacket) {
			/*ex : 512 /64 = 8  8 % 2 = 0*/
			temp32 = (req->req.length + ep->maxpacket - 1) / ep->maxpacket;
			ep->toggle_bit = ((temp32 + ep->toggle_bit) % 2);
		} else {
			if (ep->toggle_bit == 0)
				ep->toggle_bit = 1;
			else
				ep->toggle_bit = 0;
		}

		/* DMA Channel Control Reg*/
		/* Software Request + Channel Enable*/
		dma_ccr = 0;
		dma_ccr = DMA_RUN;
		if (!is_in)
			dma_ccr |= DMA_TRANS_OUT_DIR;

		if (dcmd) /* PDMA can not support 0 byte transfer*/
			pUdcDmaReg->DMA_Context_Control0 = dma_ccr;

		pDevReg->Bulk1DesStatus |= BULKXFER_ACTIVE;
	} else if ((ep->bEndpointAddress & 0x7F) == 3) {/*Interrupt In*/
		if (dcmd > INT_FIFO_SIZE)
			dcmd = INT_FIFO_SIZE;
		interrupt_transfer_size = dcmd;

		pintbuf = (u8 *)(req->req.buf);/* + req->req.actual);*/

		for (i = req->req.actual; i < (req->req.actual + dcmd); i++)
			IntBuf[(i-req->req.actual)] = pintbuf[i];

		pDevReg->InterruptDes = 0x00;
		pDevReg->InterruptDes = (dcmd << 4);

		if (ep->toggle_bit)
			pDevReg->InterruptDes |= INTXFER_DATA1;
		else
			pDevReg->InterruptDes &= 0xF7;

		if (ep->toggle_bit == 0)
			ep->toggle_bit = 1;
		else
			ep->toggle_bit = 0;

		pDevReg->InterruptDes |= INTXFER_IOC;
		pDevReg->InterruptDes |= INTXFER_ACTIVE;
	}

	DBG("req->req.dma 0x%08X \n", req->req.dma);

#ifdef MSC_COMPLIANCE_TEST
    INFO("req->req.dma 0x%08X \n", req->req.dma);
#endif

	req->dma_bytes = length;

//printk(KERN_INFO "next_in_dma e\n"); //gri

} /*static void next_in_dma()*/

static void finish_in_dma(struct vt8500_ep *ep, struct vt8500_req *req, int status)
{
//    printk(KERN_INFO "finish_in_dma()s\n"); //gri

	DBG("finish_in_dma() %s\n", ep ? ep->ep.name : NULL);

	if (status == 0) {   /* Normal complete!*/
		if ((ep->bEndpointAddress & 0x7F) == 3)
			req->req.actual += interrupt_transfer_size;
		else
			req->req.actual += dma_src_len(ep, req);/*req->dma_bytes;*/

		/* return if this request needs to send data or zlp*/
		if (req->req.actual < req->req.length)
			return;

		if (req->req.zero
		&& req->dma_bytes != 0
		&& (req->req.actual % ep->maxpacket) == 0)
			return;

	} else
		req->req.actual += dma_src_len(ep, req);

#ifdef RNDIS_INFO_DEBUG_BULK_IN
	if ((ep->bEndpointAddress & 0x7F) == 1)
		INFO("finish_in_dma()e %s req->req.actual(0x%08X) req->req.length(0x%08X)\n",
			ep ? ep->ep.name : NULL, req->req.actual, req->req.length);
#endif

	done(ep, req, status);

//printk(KERN_INFO "finish_in_dma() %s req->req.actual(0x%08X) req->req.length(0x%08X)\n", 
//ep ? ep->ep.name : NULL, req->req.actual, req->req.length); //gri


} /*static void finish_in_dma()*/

static void next_out_dma(struct vt8500_ep *ep, struct vt8500_req *req)
{
	/*unsigned packets;*/
	u32 dma_ccr = 0;
	u32	dcmd;
	u32	buf;
	int	is_in;

//    printk(KERN_INFO "next_out_dma s\n"); //gri

	is_in = 0;/*ep->bEndpointAddress & USB_DIR_IN;*/

#ifdef RNDIS_INFO_DEBUG_BULK_OUT
	if (ep->bEndpointAddress == 2)
		INFO("next_out_dma() %s\n", ep ? ep->ep.name : NULL);
#endif

	DBG("next_out_dma() %s\n", ep ? ep->ep.name : NULL);

	dcmd = req->dma_bytes = req->req.length - req->req.actual;
	buf = ((req->req.dma + req->req.actual) & 0xFFFFFFFC);

	if ((ep->bEndpointAddress == 2) && (req->req.dma == 0xFFFFFFFF) && (ep->rndis == 0)) {
		void *retval;
		dma_addr_t	dma;            
		unsigned int dma_length = 65536;

printk(KERN_INFO "rndis ooooo %d\n",req->req.length);

		retval = dma_alloc_coherent(ep->udc->dev,
							dma_length, &dma, GFP_ATOMIC);

		ep->rndis_buffer_address = (unsigned int)retval;
		ep->rndis_dma_phy_address = (u32)dma;
		ep->rndis_buffer_length = dma_length;
		ep->rndis = 1;
	}
	if ((ep->bEndpointAddress == 2) && (ep->rndis == 1) && (req->req.length > ep->rndis_buffer_length)) {
//		void *retval;
//		dma_addr_t	dma;            

printk(KERN_INFO "rndis ooooobb %d\n",req->req.length);
#if 0
			dma_free_coherent (ep->udc->dev,
			ep->rndis_buffer_length,
			(void *)ep->rndis_buffer_address, (dma_addr_t)ep->rndis_dma_phy_address);	

		retval = dma_alloc_coherent(ep->udc->dev,
							req->req.length, &dma, GFP_ATOMIC);

		ep->rndis_buffer_address = (unsigned int)retval;
		ep->rndis_dma_phy_address = (u32)dma;
		ep->rndis_buffer_length = req->req.length;
		ep->rndis = 1;
#endif		
	}

	if (ep->bEndpointAddress == 0) {/*Control*/
		ep->ep_fifo_length = 0;

		if (dcmd >= 64)
			dcmd = 64;

		ep->ep_fifo_length = dcmd;

		if (ep->toggle_bit)
			pDevReg->ControlDesTbytes = dcmd | CTRLXFER_DATA1;
		else
			pDevReg->ControlDesTbytes = dcmd | CTRLXFER_DATA0;

		pDevReg->ControlDesControl = CTRLXFER_OUT+CTRLXFER_IOC;
		pDevReg->ControlDesStatus = CTRLXFER_ACTIVE;
		ControlState = CONTROLSTATE_DATA;

		if (ep->toggle_bit == 0)
			ep->toggle_bit = 1;
		else
			ep->toggle_bit = 0;

	} else if (ep->bEndpointAddress  == 2) {/*Bulk Out*/
		ep->stall_more_processing  = 0;

//        printk(KERN_INFO "next_out_dma %d %d %d\n", req->dma_bytes, req->req.length,req->req.actual); //gri

		/*if(req->dma_bytes == 64)*/
		ep->udc->cbw_virtual_address =  (((u32)req->req.buf + req->req.actual) & 0xFFFFFFFC);

		if (dcmd > UBE_MAX_DMA)
			dcmd = UBE_MAX_DMA;

		if (pDevReg->Bulk2EpControl & EP_STALL) {
			ep->stall_more_processing = 1;
			ep->temp_dcmd = dcmd;
		}
		/* Set Address*/
		if (ep->rndis == 1)
			wmt_udc_pdma_des_prepare(dcmd,
				((ep->rndis_dma_phy_address + req->req.actual) & 0xFFFFFFFC),
				DESCRIPTOT_TYPE_LONG, TRANS_OUT);
		else
			wmt_udc_pdma_des_prepare(dcmd, buf, DESCRIPTOT_TYPE_LONG, TRANS_OUT);

		if (pDevReg->Bulk2EpControl & EP_STALL)
			ep->temp_bulk_dma_addr = buf;

		if (pDevReg->Bulk2EpControl & EP_STALL)
			ep->temp_dma_ccr = dma_ccr;

		/* DMA Global Controller Reg*/
		/* DMA Controller Enable +*/
		/* DMA Global Interrupt Enable(if any TC, error, or abort status in any channels occurs)*/

		pDevReg->Bulk2DesStatus = 0x00;
		pDevReg->Bulk2DesTbytes2 |= (dcmd >> 16) & 0x3;
		pDevReg->Bulk2DesTbytes1 = (dcmd >> 8) & 0xFF;
		pDevReg->Bulk2DesTbytes0 = dcmd & 0xFF;

		/* set endpoint data toggle*/
		if (ep->toggle_bit)
			pDevReg->Bulk2DesTbytes2 |= BULKXFER_DATA1;
		else
			pDevReg->Bulk2DesTbytes2 &= 0x3F;/*BULKXFER_DATA0;*/

		pDevReg->Bulk2DesStatus = BULKXFER_IOC;/*| BULKXFER_IN;*/

		if (pDevReg->Bulk2EpControl & EP_STALL)
			ep->ep_stall_toggle_bit = ep->toggle_bit;

		dma_ccr = 0;
		dma_ccr = DMA_RUN;
		if (!is_in)
			dma_ccr |= DMA_TRANS_OUT_DIR;

		pUdcDmaReg->DMA_Context_Control1 = dma_ccr;

		pDevReg->Bulk2DesStatus |= BULKXFER_ACTIVE;
	   /*udc_device_dump_register();*/
	}

	VDBG("req->req.dma 0x%08X \n", req->req.dma);

//printk(KERN_INFO "next_out_dma e\n"); //gri

} /*static void next_out_dma()*/

static void
finish_out_dma(struct vt8500_ep *ep, struct vt8500_req *req, int status)
{
	u16	count;
	u8 temp8;
	u32 temp32;
    /*u8	bulk_dma_csr;*/

//    printk(KERN_INFO "finish_out_dma s\n"); //gri

	DBG("finish_out_dma() %s\n", ep ? ep->ep.name : NULL);

	count = dma_dest_len(ep, req);

	if (ep->bEndpointAddress == 0) {/*Control*/
		u8 *pctrlbuf;
		int i;

		pctrlbuf = (u8 *)(req->req.buf + req->req.actual);
		for (i = 0; i < count; i++)
			pctrlbuf[i] = SetupBuf[i];

		/*INFO("finish_out_dma() %s\n", ep ? ep->ep.name : NULL);*/
		/*dump_bulk_buffer((req->req.buf + req->req.actual), count);*/
	}

	count += req->req.actual;

	if (count <= req->req.length)
		req->req.actual = count;

	if (ep->bEndpointAddress == 0) {/*Control*/
		if (req->req.actual < req->req.length) {
			temp8 = pDevReg->ControlDesStatus;

			if ((temp8 & CTRLXFER_SHORTPKT) == 0)
				return; /*Continue...*/
		}
	} else if (ep->bEndpointAddress == 2) {
	
        if (pDevReg->Bulk2DesTbytes2 & 0x80)
            ep->toggle_bit= 1;
        else
            ep->toggle_bit= 0;	

//        while((pUdcDmaReg->DMA_Context_Control1_Bis.EventCode != 0xf) &&
//        (pUdcDmaReg->DMA_Context_Control1_Bis.EventCode != 0x5));

//        printk(KERN_INFO "finish_out_dma() %s req->actual(%d) req->length(%d) toggle_bit(%8x) add %x\n", 
//        ep ? ep->ep.name : NULL, req->req.actual, req->req.length, (pDevReg->Bulk2DesTbytes2), (unsigned int)pDevReg);//gri
        //      INFO("finish_out_dma() %s req->actual(%d) req->length(%d) toggle_bit(%8x) add %x\n",
        //          ep ? ep->ep.name : NULL, req->req.actual, req->req.length, (pDevReg->Bulk2DesTbytes2), (unsigned int)pDevReg);


        {
            {
                    unsigned int gri_t_d;
                    unsigned int gri_count=0;
                    unsigned int dma_count;
                    do{
                        gri_t_d=pUdcDmaReg->DMA_ISR ;
                        gri_t_d &= 0x2;
                        gri_count++;
                        if (gri_count & 0x10){
                            gri_count=0;
//                            printk(KERN_INFO "pUdcDmaReg->DMA_Context_Control1 0x%08x\n", &(pUdcDmaReg->DMA_Context_Control1));                            
                            printk(KERN_INFO "XXXXXXXXXXX 0x%08x\n",pUdcDmaReg->DMA_Context_Control1);
                            dma_count = req->req.length - pUdcDmaReg->DMA_Residual_Bytes1_Bits.ResidualBytes;                            
//                            printk(KERN_INFO "CC 0x%08x 0x%08x\n", dma_count, count); 
                            if (pUdcDmaReg->DMA_Context_Control1_Bis.Run == 0)
                                break;
                            if ((count == dma_count) || (count == 0)){
//                            printk(KERN_INFO "XXXXXXXXXXX 0x%08x\n",pUdcDmaReg->DMA_Context_Control1);
//                            printk(KERN_INFO "CC 0x%08x 0x%08x\n", dma_count, count); 
//                            while(1);
                                pUdcDmaReg->DMA_Context_Control1_Bis.Run = 0;                            
                                break;
                            }
                        }
                    }while(!gri_t_d);
            }                           
        }

          
		if (req->req.actual < req->req.length) {
			DBG("finish_out_dma() req->actual < req->req.length\n");
			temp8 = pDevReg->Bulk2DesStatus;

			if ((temp8 & BULKXFER_SHORTPKT) == 0)
				return; /*Continue...*/
			else { /*Short Package.*/
				pDevReg->Bulk2EpControl |= EP_DMALIGHTRESET;

				while (pDevReg->Bulk2EpControl & EP_DMALIGHTRESET)
					;
				pDevReg->Bulk2EpControl = EP_RUN + EP_ENABLEDMA;
			}
		}
	}
	/* rx completion*/
	/*UDC_DMA_IRQ_EN_REG &= ~UDC_RX_EOT_IE(ep->dma_channel);*/

//#ifdef RNDIS_INFO_DEBUG_BULK_OUT
//	if (ep->bEndpointAddress == 2)
//		INFO("finish_out_dma() %s req->actual(%d) req->length(%d) toggle_bit(%8x) add %x\n",
//			ep ? ep->ep.name : NULL, req->req.actual, req->req.length, (pDevReg->Bulk2DesTbytes2), (unsigned int)pDevReg);
//#endif

  /*dump_bulk_buffer(req->req.buf, req->req.actual);*/

	if ((ep->bEndpointAddress == 2) && (ep->rndis == 1)) {
		memcpy((void *)((u32)req->req.buf), (void *)((u32)ep->rndis_buffer_address), req->req.actual);

		#ifdef RNDIS_INFO_DEBUG_BULK_OUT
		/*if ((req->req.actual % 4 == 3))// && (b_message == 0))*/
		/*dump_bulk_buffer(req->req.buf, req->req.actual);*/
		#endif
#if 0
		if ((req->req.length == 1600) || (req->req.length == 2048)) {/*F.S. [1600]  : H.S. [2048]*/
			/*ex : 512 /64 = 8  8 % 2 = 0*/
			temp32 = (req->req.actual + ep->maxpacket - 1) / ep->maxpacket;
			ep->toggle_bit = ((temp32 + ep->toggle_bit) % 2);
		} else
			INFO("Different Length for Bulk Out (%08d) Toggle Bit would Error\n"
				, req->req.length);
#else
#if 0
        if (req->req.length > ep->maxpacket) {
            /*ex : 512 /64 = 8  8 % 2 = 0*/
            temp32 = (req->req.actual + ep->maxpacket - 1) / ep->maxpacket;
            ep->toggle_bit = ((temp32 + ep->toggle_bit) % 2);
        } else {
            if (ep->toggle_bit == 0)
                ep->toggle_bit = 1;
            else
                ep->toggle_bit = 0;
            }
#endif

#endif
	} else {
#if 1
        if (ep->bEndpointAddress == 0){
    		/*GigaHsu-B 2008.5.15 : Add this caculate toggle from next_out_dma() :
    			fixed sideshow gadget issue.*/
    		if (req->req.length > ep->maxpacket) {
    			/*ex : 512 /64 = 8  8 % 2 = 0*/
    			temp32 = (req->req.actual + ep->maxpacket - 1) / ep->maxpacket;
    			ep->toggle_bit = ((temp32 + ep->toggle_bit) % 2);
    		} else {
    			if (ep->toggle_bit == 0)
    				ep->toggle_bit = 1;
    			else
    				ep->toggle_bit = 0;
            }
        }
#endif

    /*GigaHsu-E 2008.5.15*/
  }

  done(ep, req, status);

//printk(KERN_INFO "finish_out_dma e\n"); //gri

} /*finish_out_dma()*/

static void dma_irq(u8 addr)
{
	struct vt8500_ep	*ep;
	struct vt8500_req	*req;
	/*u32 temp32;*/
	/*u32 i;*/

	ep = &udc->ep[addr & 0x7f];

	if ((ep->bEndpointAddress & 0x7F) == 1) {/*Bulk In*/

		/* IN dma: tx to host*/
		if (!list_empty(&ep->queue)) {
			req = container_of(ep->queue.next, struct vt8500_req, queue);
			finish_in_dma(ep, req, 0);
		}

		while (pDevReg->Bulk1EpControl & EP_COMPLETEINT)
			;

		if (!list_empty(&ep->queue)) {
			req = container_of(ep->queue.next, struct vt8500_req, queue);
			next_in_dma(ep, req);
		}
	} else if (ep->bEndpointAddress == 2) {/*Bulk Out*/

		/* OUT dma: rx from host*/
		if (!list_empty(&ep->queue)) {
			req = container_of(ep->queue.next, struct vt8500_req, queue);
			finish_out_dma(ep, req, 0);
		}

		while (pDevReg->Bulk2EpControl & EP_COMPLETEINT)
			;

		if (!list_empty(&ep->queue)) {
			req = container_of(ep->queue.next, struct vt8500_req, queue);
			next_out_dma(ep, req);
		}
	} else if ((ep->bEndpointAddress & 0x7F) == 3) {/*Interrupt In*/

		/* IN dma: tx to host*/
		if (!list_empty(&ep->queue)) {
			req = container_of(ep->queue.next, struct vt8500_req, queue);
			finish_in_dma(ep, req, 0);
		}

		while (pDevReg->InterruptEpControl & EP_COMPLETEINT)
			;
		if (!list_empty(&ep->queue)) {
			req = container_of(ep->queue.next, struct vt8500_req, queue);
			next_in_dma(ep, req);
		}
	} /*EP3 : Interrupt In*/

} /*static void dma_irq()*/


static void pullup_enable(struct vt8500_udc *udc)
{
	INFO("gri pullup_enable()\n");
	/*Enable port control's FSM to enable 1.5k pull-up on D+.*/
	pDevReg->PhyMisc &= 0x0F;/*Hardware auto-mode*/
	/*pDevReg->PhyMisc |= 0x50;*/

} /*static void pullup_enable()*/



/*-------------------------------------------------------------------------*/
/*file_storage.c ep0_queue() - control*/
/*file_storage.c start_transfer() - bulk*/
static int
wmt_ep_queue(struct usb_ep *_ep, struct usb_request *_req, gfp_t gfp_flags)
{
	struct vt8500_ep	*ep = container_of(_ep, struct vt8500_ep, ep);
	struct vt8500_req	*req = container_of(_req, struct vt8500_req, req);
	/*struct vt8500_udc	*udc;*/
//	unsigned long	flags;
	unsigned char   empty_data = 0;

	DBG("wmt_ep_queue() %s\n", ep ? ep->ep.name : NULL);
//    INFO("gri wmt_ep_queue() %s\n", ep ? ep->ep.name : NULL);

//    if ((ep->bEndpointAddress & 0x7F) > 4)
//		return -EINVAL;        

#ifdef RNDIS_INFO_DEBUG_BULK_OUT
	if ((ep->bEndpointAddress == 3))/* || ((ep->bEndpointAddress & 0x7F) == 2))*/
		INFO("wmt_ep_queue() %s\n", ep ? ep->ep.name : NULL);
#endif

	/* catch various bogus parameters*/
	if (!_req || !req->req.complete || !req->req.buf
			|| !list_empty(&req->queue)) {
		DBG("[wmt_ep_queue], bad params\n");
		return -EINVAL;
	}
	if (!_ep || (!ep->desc && ep->bEndpointAddress)) {
		DBG("[wmt_ep_queue], bad ep\n");
		return -EINVAL;
	}

//#ifdef RNDIS_INFO_DEBUG_BULK_OUT
//	if ((ep->bEndpointAddress == 2))/* || ((ep->bEndpointAddress & 0x7F) == 1))*/
//		INFO("wmt_ep_queue() %s queue req %p, len %d buf %p\n",
//		ep->ep.name, _req, _req->length, _req->buf);
//#endif

//#ifdef MSC_COMPLIANCE_TEST
//	if ((ep->bEndpointAddress & 0x7F) == 1)
//		INFO("wmt_ep_queue() %s queue req %p, len %d buf %p\n",
//			ep->ep.name, _req, _req->length, _req->buf);
//#endif

   DBG("wmt_ep_queue() %s queue req %p, len %d buf %p\n",
		ep->ep.name, _req, _req->length, _req->buf);

	spin_lock_irqsave(&udc->lock, irq_flags);

	req->req.status = -EINPROGRESS;
	req->req.actual = 0;

	/* maybe kickstart non-iso i/o queues*/

	if (list_empty(&ep->queue) && !ep->stopped && !ep->ackwait) {
		int	is_in;

		if (ep->bEndpointAddress == 0) {
			if (!udc->ep0_pending || !list_empty(&ep->queue)) {
				spin_unlock_irqrestore(&udc->lock, irq_flags);
				return -EL2HLT;
				}

			/* empty DATA stage?*/
			is_in = udc->ep0_in;

			if (!req->req.length) { /*status 0 bytes*/

				/* chip became CONFIGURED or ADDRESSED*/
				/* earlier; drivers may already have queued*/
				/* requests to non-control endpoints*/
				/**/
				/*Neil mark code
				if (udc->ep0_set_config)
				{
				if (!udc->ep0_reset_config)
				{

				}
				}
				*/
				/* STATUS is reverse direction*/

				udc->ep0_status_0_byte = 1;
				ep0_status(udc);

				/* cleanup*/
				udc->ep0_pending = 0;
				done(ep, req, 0);
				empty_data = 1;
				/*if (is_in)*/
				/*req = 0; //cause page fault.*/
/*neil mark code
			// non-empty DATA stage
			} else if (req->req.length) {
				if (is_in)
				{
				//UDC_EP_NUM_REG = UDC_EP_SEL|UDC_EP_DIR;
				//udc_reg->EP0ControlStatus_Bits.Direction = 1;//Control In
				}
				else
				{
				//udc_reg->EP0ControlStatus_Bits.Direction = 0; //Control OUT
				}*/
			}

			if (req->req.length) {
				udc->bulk_out_dma_write_error = 0;
				(is_in ? next_in_dma : next_out_dma)(ep, req);
			}
		} else {
			is_in = ep->bEndpointAddress & USB_DIR_IN;
			
			if (req != 0){
				list_add_tail(&req->queue, &ep->queue);
				empty_data = 1;
			}
//			else
//			{
//				printk(KERN_INFO "xxx %x %x\n",req,empty_data); //gri
//			}			
			/*if (!ep->has_dma)*/
			/*	use_ep(ep, UDC_EP_SEL);*/
			/* if ISO: SOF IRQs must be enabled/disabled!*/
			(is_in ? next_in_dma : next_out_dma)(ep, req);
		}
	}
	/* irq handler advances the queue*/
	if ((req != 0) && (empty_data == 0))
		list_add_tail(&req->queue, &ep->queue);

	spin_unlock_irqrestore(&udc->lock, irq_flags);

	return 0;
} /*wmt_ep_queue()*/

static int wmt_ep_dequeue(struct usb_ep *_ep, struct usb_request *_req)
{
	struct vt8500_ep *ep = container_of(_ep, struct vt8500_ep, ep);
	struct vt8500_req	*req;
//	unsigned long	flags;

	if (!_ep || !_req)
		return -EINVAL;

	spin_lock_irqsave(&ep->udc->lock, irq_flags);

  DBG("wmt_ep_dequeue() %s\n", ep ? ep->ep.name : NULL);
	/* make sure it's actually queued on this endpoint*/
	list_for_each_entry(req, &ep->queue, queue) {
		if (&req->req == _req)
			break;
	}
	if (&req->req != _req) {
		spin_unlock_irqrestore(&ep->udc->lock, irq_flags);
		return -EINVAL;
	}

	done(ep, req, -ECONNRESET);

	spin_unlock_irqrestore(&ep->udc->lock, irq_flags);

	return 0;
} /*static int wmt_ep_dequeue()*/

/*-------------------------------------------------------------------------*/

static int wmt_ep_set_halt(struct usb_ep *_ep, int value)
{
	struct vt8500_ep *ep = container_of(_ep, struct vt8500_ep, ep);
//	unsigned long	flags;
	int		status = -EOPNOTSUPP;

	spin_lock_irqsave(&ep->udc->lock, irq_flags);

	ep->toggle_bit = 0; /*patch CLEAR_FEATURE ENDPOINT_HALT*/
    DBG("wmt_ep_set_halt() %s\n", ep ? ep->ep.name : NULL);

	if (value) {
		u8 temp32bytes[31];

		memset(temp32bytes, 0, 31);
		temp32bytes[0] =  0xEF;
		temp32bytes[1] =  0xBE;
		temp32bytes[2] =  0xAD;
		temp32bytes[3] =  0xDE;

		/*usb_ep_set_halt() - value == 1 stall*/

		switch ((ep->bEndpointAddress & 0x7F)) {
		case 0:/*Control In/Out*/
			pDevReg->ControlEpControl |= EP_STALL;
		break;

		case 1:/*Bulk In*/
			pDevReg->Bulk1EpControl |= EP_STALL;
		break;

		case 2:/*Bulk Out*/
			pDevReg->Bulk2EpControl |= EP_STALL;
		break;

		case 3:/*Interrupt In*/
			pDevReg->InterruptEpControl |= EP_STALL;
		break;
		}

		ep->stall = 1;
		/*DBG("wmt_ep_set_halt(1) HALT CSR(0x%08X)\n", *ep->reg_control_status);*/
		status = 0;

		if (memcmp(temp32bytes, (void *)ep->udc->cbw_virtual_address, 4) == 0)
			udc->file_storage_set_halt = 1; /*forbid to CLEAR FEATURE*/
	} else {/*usb_ep_clear_halt - value == 0  reset*/

		/**ep->reg_control_status &= 0xFFFFFFFB;*/
		switch ((ep->bEndpointAddress & 0x7F)) {
		case 0:/*Control In/Out*/
			pDevReg->ControlEpControl &= 0xF7;
		break;

		case 1:/*Bulk In*/
			pDevReg->Bulk1EpControl &= 0xF7;
		break;

		case 2:/*Bulk Out*/
			pDevReg->Bulk2EpControl &= 0xF7;
		break;

		case 3:/*Interrupt In*/
			pDevReg->InterruptEpControl &= 0xF7;
		break;
		}

		ep->stall = 0;
		status = 0;
		udc->file_storage_set_halt = 0;
	}

	VDBG("%s %s halt stat %d\n", ep->ep.name,
		value ? "set" : "clear", status);
	spin_unlock_irqrestore(&ep->udc->lock, irq_flags);

	return status;
} /*static int wmt_ep_set_halt()*/

static struct usb_ep_ops wmt_ep_ops = {
	.enable		= wmt_ep_enable,
	.disable	= wmt_ep_disable,

	.alloc_request	= wmt_alloc_request,
	.free_request	= wmt_free_request,

	.alloc_buffer	= wmt_alloc_buffer,
	.free_buffer	= wmt_free_buffer,

	.queue		= wmt_ep_queue,
	.dequeue  	= wmt_ep_dequeue,

	.set_halt	= wmt_ep_set_halt,
	.run_script = run_script,
	/* fifo_status ... report bytes in fifo*/
	/* fifo_flush ... flush fifo*/
};

static void wmt_ep_setup_csr(char *name, u8 addr, u8 type, unsigned maxp)
{
	struct vt8500_ep	*ep;
	/*u32	   epn_rxtx = 0;*/
	/*U8     temp_adr;*/

    VDBG("wmt_ep_setup_csr()\n");

	/* OUT endpoints first, then IN*/

	ep = &udc->ep[addr & 0x7f];

    VDBG("wmt_ep_setup()\n");

	/* OUT endpoints first, then IN*/
	ep = &udc->ep[addr & 0x7f];

	if (type == USB_ENDPOINT_XFER_CONTROL) {
		/*pDevReg->ControlEpControl;*/
		pDevReg->ControlEpReserved = 0;
		pDevReg->ControlEpMaxLen = (maxp & 0xFF);
		pDevReg->ControlEpEpNum = 0;
	} else if (type == USB_ENDPOINT_XFER_BULK) {
		if (addr & USB_DIR_IN) {
			/*pDevReg->Bulk1EpControl;*/
			pDevReg->Bulk1EpOutEpNum = (addr & 0x7f) << 4;
			pDevReg->Bulk1EpMaxLen = (maxp & 0xFF);
			pDevReg->Bulk1EpInEpNum = (((addr & 0x7f) << 4) | ((maxp & 0x700) >> 8));
		} else {
			/*pDevReg->Bulk2EpControl;*/
			pDevReg->Bulk2EpOutEpNum = (addr & 0x7f) << 4;
			pDevReg->Bulk2EpMaxLen = (maxp & 0xFF);
			pDevReg->Bulk2EpInEpNum = (((addr & 0x7f) << 4) | ((maxp & 0x700) >> 8));
		}
	} else if (type == USB_ENDPOINT_XFER_INT) {
		/*pDevReg->InterruptEpControl; // Interrupt endpoint control           - 2C*/
		/* Interrupt endpoint reserved byte     - 2D*/
		pDevReg->InterruptReserved = (addr & 0x7f) << 4;
		pDevReg->InterruptEpMaxLen = (maxp & 0xFF); /* Interrupt maximum transfer length    - 2E*/
		pDevReg->InterruptEpEpNum = (((addr & 0x7f) << 4) | ((maxp & 0x700) >> 8));
	}

	/*Setting address pointer ...*/

    VDBG("wmt_ep_setup_csr() - %s addr %02x  maxp %d\n",
		name, addr, maxp);

	ep->toggle_bit = 0;
	ep->ep.maxpacket = ep->maxpacket = maxp;
} /*tatic void wmt_ep_setup_csr()*/

static void
wmt_udc_csr(struct vt8500_udc *udc)
{
	/* abolish any previous hardware state*/

	DBG("wmt_udc_csr()\n");

	if (udc->gadget.speed == USB_SPEED_FULL) {
		wmt_ep_setup_csr("ep1in" "-bulk", (USB_DIR_IN | 1), USB_ENDPOINT_XFER_BULK, 64);
		wmt_ep_setup_csr("ep2out" "-bulk", (USB_DIR_OUT | 2), USB_ENDPOINT_XFER_BULK, 64);
	} else if (udc->gadget.speed == USB_SPEED_HIGH) {
		wmt_ep_setup_csr("ep1in" "-bulk", (USB_DIR_IN | 1), USB_ENDPOINT_XFER_BULK, 512);
		wmt_ep_setup_csr("ep2out" "-bulk", (USB_DIR_OUT | 2), USB_ENDPOINT_XFER_BULK, 512);
	} else if (udc->gadget.speed == USB_SPEED_UNKNOWN) {
		wmt_ep_setup_csr("ep1in" "-bulk", (USB_DIR_IN | 1), USB_ENDPOINT_XFER_BULK, 512);
		wmt_ep_setup_csr("ep2out" "-bulk", (USB_DIR_OUT | 2), USB_ENDPOINT_XFER_BULK, 512);
	}
} /*wmt_udc_csr(void)*/

static int wmt_get_frame(struct usb_gadget *gadget)
{
    DBG("wmt_get_frame()\n");
    return 0;
}

static int wmt_wakeup(struct usb_gadget *gadget)
{
    /*struct vt8500_udc	*udc;*/
	/*unsigned long	flags;*/
	int		retval = 0;
	DBG("wmt_wakeup()\n");
/*
	udc = container_of(gadget, struct vt8500_udc, gadget);

	spin_lock_irqsave(&udc->lock, irq_flags);
	if (udc->devstat & UDC_SUS) {
		// NOTE:  OTG spec erratum says that OTG devices may
		// issue wakeups without host enable.
		//
		if (udc->devstat & (UDC_B_HNP_ENABLE|UDC_R_WK_OK)) {
			DBG("remote wakeup...\n");
			UDC_SYSCON2_REG = UDC_RMT_WKP;
			retval = 0;
		}

	// NOTE:  non-OTG systems may use SRP TOO...
	} else if (!(udc->devstat & UDC_ATT)) {
		if (udc->transceiver)
			retval = otg_start_srp(udc->transceiver);
	}
	spin_unlock_irqrestore(&udc->lock, irq_flags);
*/
#if 0
    if (pDevReg->SelfPowerConnect & 0x01)  //connect
    {                    
        gadget_connect=1;
        run_script(1);               
        wmt_pdma_reset();               
    }
    else {//disconnct
        spin_unlock(&udc->lock);
        if (udc->driver)
            udc->driver->disconnect(&udc->gadget);                    
        gadget_connect=0;                               
        run_script(2);
        if (gadget_mode)
            schedule_work(&reinsmod_thread);                
        spin_lock(&udc->lock);
    }                
#endif

	return retval;
}

static int
wmt_set_selfpowered(struct usb_gadget *gadget, int is_selfpowered)
{
    DBG("wmt_set_selfpowered()\n");
/*	struct vt8500_udc	*udc;
	unsigned long	flags;
	u16		syscon1;

	udc = container_of(gadget, struct vt8500_udc, gadget);
	spin_lock_irqsave(&udc->lock, irq_flags);
	syscon1 = UDC_SYSCON1_REG;
	if (is_selfpowered)
		syscon1 |= UDC_SELF_PWR;
	else
		syscon1 &= ~UDC_SELF_PWR;
	UDC_SYSCON1_REG = syscon1;
	spin_unlock_irqrestore(&udc->lock, irq_flags);
*/
	return 0;
}

/*
static int can_pullup(struct vt8500_udc *udc)
{
	return udc->driver && udc->softconnect && udc->vbus_active;
}
*/
static void pullup_disable(struct vt8500_udc *udc)
{
	INFO("pullup_disable()\n");
	/*Hold port control's FSM from enter device mode.*/
	pDevReg->PhyMisc &= 0x0F;
	pDevReg->PhyMisc |= 0x10;

    gadget_connect=0;
    run_script(2);
    
} /*static void pullup_disable()*/

/*
 * Called by whatever detects VBUS sessions:  external transceiver
 * driver, or maybe GPIO0 VBUS IRQ.  May request 48 MHz clock.
 */
static int wmt_vbus_session(struct usb_gadget *gadget, int is_active)
{
    DBG("wmt_vbus_session()\n");
/*	struct vt8500_udc	*udc;
	unsigned long	flags;

	udc = container_of(gadget, struct vt8500_udc, gadget);
	spin_lock_irqsave(&udc->lock, irq_flags);
	VDBG("VBUS %s\n", is_active ? "on" : "off");
	udc->vbus_active = (is_active != 0);
	if (cpu_is_vt850015xx()) {
		// "software" detect, ignored if !VBUS_MODE_1510
		if (is_active)
			FUNC_MUX_CTRL_0_REG |= VBUS_CTRL_1510;
		else
			FUNC_MUX_CTRL_0_REG &= ~VBUS_CTRL_1510;
	}
	if (can_pullup(udc))
		pullup_enable(udc);
	else
		pullup_disable(udc);
	spin_unlock_irqrestore(&udc->lock, irq_flags);
*/
	return 0;
}

static int wmt_vbus_draw(struct usb_gadget *gadget, unsigned mA)
{
    DBG("wmt_vbus_draw()\n");
/*	struct vt8500_udc	*udc;

	udc = container_of(gadget, struct vt8500_udc, gadget);
	if (udc->transceiver)
		return otg_set_power(udc->transceiver, mA);
*/
	return -EOPNOTSUPP;
}

static int wmt_pullup(struct usb_gadget *gadget, int is_on)
{
	struct vt8500_udc	*udc;
//	unsigned long	flags;

    DBG("wmt_pullup()\n");
	INFO("gri wmt_pullup()\n");

	udc = container_of(gadget, struct vt8500_udc, gadget);
	spin_lock_irqsave(&udc->lock, irq_flags);
	udc->softconnect = (is_on != 0);
//	if (can_pullup(udc))
        
	if (udc->softconnect)        
		pullup_enable(udc);
	else
		pullup_disable(udc);
	spin_unlock_irqrestore(&udc->lock, irq_flags);

	return 0;
}

static struct usb_gadget_ops wmt_gadget_ops = {
	.get_frame		= wmt_get_frame,
	.wakeup			= wmt_wakeup,
	.set_selfpowered	= wmt_set_selfpowered,
	.vbus_session		= wmt_vbus_session,
	.vbus_draw		= wmt_vbus_draw,
	.pullup			= wmt_pullup,
};

/* dequeue ALL requests; caller holds udc->lock */
static void nuke(struct vt8500_ep *ep, int status)
{
	struct vt8500_req	*req;

	DBG("nuke()\n");
	ep->stopped = 1;

	while (!list_empty(&ep->queue)) {
		req = list_entry(ep->queue.next, struct vt8500_req, queue);
		done(ep, req, status);
	}
} /*void nuke()*/

/* caller holds udc->lock */
static void udc_quiesce(struct vt8500_udc *udc)
{
	struct vt8500_ep	*ep;
    DBG("udc_quiesce()\n");

	udc->gadget.speed = USB_SPEED_UNKNOWN;
	nuke(&udc->ep[0], -ESHUTDOWN);
	list_for_each_entry(ep, &udc->gadget.ep_list, ep.ep_list)
		nuke(ep, -ESHUTDOWN);
} /*void udc_quiesce()*/

/*
static void update_otg(struct vt8500_udc *udc)
{
	u16	devstat;

	if (!udc->gadget.is_otg)
		return;

	if (OTG_CTRL_REG & OTG_ID)
		devstat = UDC_DEVSTAT_REG;
	else
		devstat = 0;

	udc->gadget.b_hnp_enable = !!(devstat & UDC_B_HNP_ENABLE);
	udc->gadget.a_hnp_support = !!(devstat & UDC_A_HNP_SUPPORT);
	udc->gadget.a_alt_hnp_support = !!(devstat & UDC_A_ALT_HNP_SUPPORT);

	// Enable HNP early, avoiding races on suspend irq path.
	// ASSUMES OTG state machine B_BUS_REQ input is true.
	//
	if (udc->gadget.b_hnp_enable)
		OTG_CTRL_REG = (OTG_CTRL_REG | OTG_B_HNPEN | OTG_B_BUSREQ)
				& ~OTG_PULLUP;

}//static void update_otg()
*/


/* define the prepare result codes*/
#define RESPOK		0
#define RESPFAIL	1
u16 Control_Length;     			/* the length of transfer for current control transfer*/
u16 Configure_Length;


/*UDC_IS_SETUP_INT*/
static void udc_control_prepare_data_resp(void)/*(struct vt8500_udc *udc, u32 udc_irq_src)*/
{
	struct vt8500_ep	*ep0 = &udc->ep[0];
	struct vt8500_req	*req = 0;
	/*UCHAR Result = RESPFAIL;*/
	/*unsigned char CurXferLength = 0;*/
	/*unsigned char Control_Length = 0;*/
	int i;
	u8 test_mode_enable = 0;

	Configure_Length = 0;

	/*ep0->irqs++;*/
	DBG("ep0_irq()\n");
	ep0->toggle_bit = 1;

	if (!list_empty(&ep0->queue))
		req = container_of(ep0->queue.next, struct vt8500_req, queue);

	/* SETUP starts all control transfers*/
	{
		union u {
			u8			bytes[8];
			struct usb_ctrlrequest	r;
		} u;
		int			status = -EINVAL;
		struct vt8500_ep *ep = &udc->ep[0];
		int    ep0_status_phase_0_byte = 0;

		nuke(ep0, 0);
		/* read the (latest) SETUP message*/
		for (i = 0; i <= 7; i++)
			u.bytes[i] = pSetupCommandBuf[i];

		le32_to_cpus(&u.r.wValue);
		le32_to_cpus(&u.r.wIndex);
		le32_to_cpus(&u.r.wLength);

		/* Delegate almost all control requests to the gadget driver,*/
		/* except for a handful of ch9 status/feature requests that*/
		/* hardware doesn't autodecode _and_ the gadget API hides.*/
		/**/
		udc->ep0_in = (u.r.bRequestType & USB_DIR_IN) != 0;
		udc->ep0_set_config = 0;
		udc->ep0_pending = 1;
		udc->ep0_in_status = 0;

		ep0->stopped = 0;
		ep0->ackwait = 0;

		if ((u.r.bRequestType & USB_RECIP_OTHER) == USB_RECIP_OTHER) {/*USB_RECIP_OTHER(0x03)*/
			status = 0;

			/*INFO("ep0_irq() setup command[0]=(0x%08X)\n",u.dword[0]);*/
			/*INFO("ep0_irq() setup command[1]=(0x%08X)\n",u.dword[1]);*/
			/*INFO("ep0_irq() address(%08X)", udc_reg->AddressControl);*/
			goto delegate;
		}

		switch (u.r.bRequest) {
		case USB_REQ_SET_CONFIGURATION:
			/* udc needs to know when ep != 0 is valid*/
			/*SETUP 00.09 v0001 i0000 l0000*/
			if (u.r.bRequestType != USB_RECIP_DEVICE)/*USB_RECIP_DEVICE(0x00)*/
				goto delegate;
			if (u.r.wLength != 0)
				goto do_stall;
			udc->ep0_set_config = 1;
			udc->ep0_reset_config = (u.r.wValue == 0);
			VDBG("set config %d\n", u.r.wValue);

			if (u.r.wValue == 0)
				USBState = USBSTATE_ADDRESS;
			else
			 USBState = USBSTATE_CONFIGED;
			/* update udc NOW since gadget driver may start*/
			/* queueing requests immediately; clear config*/
			/* later if it fails the request.*/
			/**/

			udc->ep[0].toggle_bit = 0;
			udc->ep[1].toggle_bit = 0;
			udc->ep[2].toggle_bit = 0;
			udc->ep[3].toggle_bit = 0;
			udc->ep[4].toggle_bit = 0;
			udc->ep[5].toggle_bit = 0;//gri
			udc->ep[6].toggle_bit = 0;//gri


			status = 0;
			goto delegate;

		/* Giga Hsu : 2007.6.6 This would cause set interface status 0
			return to fast and cause bulk endpoint in out error*/
		case USB_REQ_SET_INTERFACE:

			VDBG("set interface %d\n", u.r.wValue);
			status = 0;

			udc->ep[0].toggle_bit = 0;
			udc->ep[1].toggle_bit = 0;
			udc->ep[2].toggle_bit = 0;
			udc->ep[3].toggle_bit = 0;
			udc->ep[4].toggle_bit = 0;
			udc->ep[5].toggle_bit = 0;//gri
			udc->ep[6].toggle_bit = 0;//gri

			goto delegate;
		    /*break;*/

		case USB_REQ_CLEAR_FEATURE:
			/* clear endpoint halt*/

			if (u.r.bRequestType != USB_RECIP_ENDPOINT)
				goto delegate;

			if (u.r.wValue != USB_ENDPOINT_HALT
			|| u.r.wLength != 0)
				goto do_stall;

			ep = &udc->ep[u.r.wIndex & 0xf];

			if (ep != ep0) {
				if (ep->bmAttributes == USB_ENDPOINT_XFER_ISOC || !ep->desc)
					goto do_stall;

				if (udc->file_storage_set_halt == 0) {
					switch ((ep->bEndpointAddress & 0x7F)) {
					case 0:/*Control In/Out*/
					   pDevReg->ControlEpControl &= 0xF7;
					break;

					case 1:/*Bulk In*/

						pDevReg->Bulk1EpControl &= 0xF7;
						#ifdef MSC_COMPLIANCE_TEST
						udc_bulk_dma_dump_register();
						udc_device_dump_register();
						#endif
						if (ep->stall_more_processing == 1) {
							u32 dma_ccr = 0;

							ep->stall_more_processing = 0;
							wmt_pdma0_reset();
							wmt_udc_pdma_des_prepare(ep->temp_dcmd,
								ep->temp_bulk_dma_addr,
								DESCRIPTOT_TYPE_LONG, TRANS_IN);

							pDevReg->Bulk1DesStatus = 0x00;
							pDevReg->Bulk1DesTbytes2 |=
								(ep->temp_dcmd >> 16) & 0x3;
							pDevReg->Bulk1DesTbytes1 =
								(ep->temp_dcmd >> 8) & 0xFF;
							pDevReg->Bulk1DesTbytes0 =
								ep->temp_dcmd & 0xFF;

							/* set endpoint data toggle*/
							if (ep->ep_stall_toggle_bit) {
								/* BULKXFER_DATA1;*/
								pDevReg->Bulk1DesTbytes2 |= 0x40;
							} else {
								/*(~BULKXFER_DATA1);*/
								pDevReg->Bulk1DesTbytes2 &= 0xBF;
							}

							pDevReg->Bulk1DesStatus =
								(BULKXFER_IOC | BULKXFER_IN);

							/* DMA Channel Control Reg*/
							/* Software Request + Channel Enable*/
							dma_ccr = 0;
							dma_ccr = DMA_RUN;
							pUdcDmaReg->DMA_Context_Control0 = dma_ccr; //neil

							pDevReg->Bulk1DesStatus |= BULKXFER_ACTIVE;
						}
					break;

					case 2:/*Bulk Out*/
						pDevReg->Bulk2EpControl &= 0xF7;

						if (ep->stall_more_processing == 1) {
							u32 dma_ccr = 0;
							ep->stall_more_processing  = 0;
							wmt_pdma1_reset();
						//	wmt_pdma_reset();
							wmt_udc_pdma_des_prepare(ep->temp_dcmd,
								ep->temp_bulk_dma_addr,
							DESCRIPTOT_TYPE_LONG, TRANS_OUT);
							/* DMA Global Controller Reg*/
							/* DMA Controller Enable +*/
							/* DMA Global Interrupt Enable(if any TC, error,
								or abort status in any channels occurs)*/

							pDevReg->Bulk2DesStatus = 0x00;
							pDevReg->Bulk2DesTbytes2 |=
								(ep->temp_dcmd >> 16) & 0x3;
							pDevReg->Bulk2DesTbytes1 =
								(ep->temp_dcmd >> 8) & 0xFF;
							pDevReg->Bulk2DesTbytes0 =
								ep->temp_dcmd & 0xFF;

							if (ep->ep_stall_toggle_bit)
								pDevReg->Bulk2DesTbytes2 |= BULKXFER_DATA1;
							else
								pDevReg->Bulk2DesTbytes2 &= 0x3F;

							pDevReg->Bulk2DesStatus = BULKXFER_IOC;

							/* DMA Channel Control Reg*/
							/* Software Request + Channel Enable*/

							/*udc_bulk_dma_dump_register();*/
							dma_ccr = 0;
							dma_ccr = DMA_RUN;
							dma_ccr |= DMA_TRANS_OUT_DIR;
							pUdcDmaReg->DMA_Context_Control1 = dma_ccr;
							pDevReg->Bulk2DesStatus |= BULKXFER_ACTIVE;
						}
					break;

					case 3:/*Interrupt In*/
						pDevReg->InterruptEpControl &= 0xF7;
					break;
					}

					ep->stall = 0;
					ep->stopped = 0;
					/**ep->reg_irp_descriptor = ep->temp_irp_descriptor;*/
				}
			} /*if (ep != ep0)*/

			ep0_status_phase_0_byte = 1;
			VDBG("%s halt cleared by host\n", ep->name);
			/*goto ep0out_status_stage;*/
			status = 0;
			udc->ep0_pending = 0;
		/*break;*/
			goto delegate;

		case USB_REQ_SET_FEATURE:
			/* set endpoint halt*/

			if ((u.r.wValue == USB_DEVICE_TEST_MODE)) {/*(USBState == USBSTATE_DEFAULT) &&*/
				TestMode = (UCHAR)(u.r.wIndex >> 8);
				test_mode_enable = 1;
				INFO("USB_REQ_SET_FEATURE - TestMode (0x%02X)\n", TestMode);

				/*ControlState = CONTROLSTATE_STATUS;*/
				ep0_status_phase_0_byte = 1;
				status = 0;
				break;
			}

			if (u.r.bRequestType != USB_RECIP_ENDPOINT)
				goto delegate;

			if (u.r.wValue != USB_ENDPOINT_HALT || u.r.wLength != 0)
				goto do_stall;

			ep = &udc->ep[u.r.wIndex & 0xf];

			if (ep->bmAttributes == USB_ENDPOINT_XFER_ISOC || ep == ep0 || !ep->desc)
				goto do_stall;

			switch ((ep->bEndpointAddress & 0x7F)) {
			case 0:/*Control In/Out*/
				pDevReg->ControlEpControl |= EP_STALL;
			break;

			case 1:/*Bulk In*/
				pDevReg->Bulk1EpControl |= EP_STALL;
			break;

			case 2:/*Bulk Out*/
				pDevReg->Bulk2EpControl |= EP_STALL;
			break;

			case 3:/*Interrupt In*/
				pDevReg->InterruptEpControl |= EP_STALL;
			break;
			}
			ep->stall = 1;
			ep->stopped = 1;

			ep0_status_phase_0_byte = 1;
			/*use_ep(ep, 0);*/
			/* can't halt if fifo isn't empty...*/
			/*UDC_CTRL_REG = UDC_CLR_EP;*/
			/*UDC_CTRL_REG = UDC_SET_HALT;*/
			VDBG("%s halted by host\n", ep->name);
			/*ep0out_status_stage:*/
			status = 0;

			udc->ep0_pending = 0;
			/*break;*/
			goto delegate;

		case USB_REQ_GET_STATUS:
			/* return interface status.  if we were pedantic,*/
			/* we'd detect non-existent interfaces, and stall.*/
			/**/

			if (u.r.bRequestType == (USB_DIR_IN|USB_RECIP_ENDPOINT)) {
				ep = &udc->ep[u.r.wIndex & 0xf];

				if (ep->stall == 1) {
					udc->ep0_in_status = 0x01;

					/*GgiaHsu-B 2007.08.10 : patch HW Bug :
						MSC Compliance Test : Error Recovery Items.*/
					ep = &udc->ep[3];
					if ((udc->file_storage_set_halt == 1) && (ep->stall == 1))
						ep->stall = 1;
					/*GgiaHsu-E 2007.08.10 :
						--------------------------------------------*/

					ep = &udc->ep[0];
					ep->stall = 0;
				} else
					udc->ep0_in_status = 0x00;

				VDBG("GET_STATUS, interface wIndex(0x%02X) ep0_in_status(0x%04X)\n"
					, u.r.wIndex, udc->ep0_in_status);
			} else
				udc->ep0_in_status = 0x00;

			/* return two zero bytes*/
			status = 0;

			/* next, status stage*/
			goto delegate;
			break;

		case USB_REQ_SET_ADDRESS:
			if (u.r.bRequestType == USB_RECIP_DEVICE) { /*USB_RECIP_DEVICE(0x00)*/
				if (USBState == USBSTATE_DEFAULT) {
					if (u.r.wValue != 0)
						USBState = USBSTATE_ADDRESS;
				} else if (USBState == USBSTATE_ADDRESS) {
					if (u.r.wValue == 0)
						USBState = USBSTATE_DEFAULT;
				}

				pDevReg->DeviceAddr |= (DEVADDR_ADDRCHANGE | u.r.wValue);
				VDBG("USB_REQ_SET_ADDRESS 0x%03d\n", u.r.wValue);
				ControlState = CONTROLSTATE_STATUS;
				ep0_status_phase_0_byte = 1;
				status = 0;
			}
		break;

		case USB_BULK_RESET_REQUEST:

			VDBG("USB_BULK_RESET_REQUEST\n");
			udc->file_storage_set_halt = 0;

			udc->ep[0].toggle_bit = 0;
			udc->ep[1].toggle_bit = 0;
			udc->ep[2].toggle_bit = 0;
			udc->ep[3].toggle_bit = 0;
			udc->ep[4].toggle_bit = 0;
			udc->ep[5].toggle_bit = 0;//gri
			udc->ep[6].toggle_bit = 0;//gri

			status = 0;

			goto delegate;
			/*break;*/

		default:
delegate:
			VDBG("SETUP %02x.%02x v%04x i%04x l%04x\n",
			u.r.bRequestType, u.r.bRequest,
			u.r.wValue, u.r.wIndex, u.r.wLength);
			  /*
			// The gadget driver may return an error here,
			// causing an immediate protocol stall.
			//
			// Else it must issue a response, either queueing a
			// response buffer for the DATA stage, or halting ep0
			// (causing a protocol stall, not a real halt).  A
			// zero length buffer means no DATA stage.
			//
			// It's fine to issue that response after the setup()
			// call returns, and this IRQ was handled.
			//
			*/
			udc->ep0_setup = 1;
			spin_unlock(&udc->lock);
			/*usb gadget driver prepare setup data phase(control in)*/
			status = udc->driver->setup(&udc->gadget, &u.r); /*usb_gadget_driver->setup()*/
			spin_lock(&udc->lock);
			udc->ep0_setup = 0;
		} /*switch (u.r.bRequest)*/

		if (ep0_status_phase_0_byte == 1) {/* && (udc->ep[0].stall==0))*/
			udc->ep0_status_0_byte = 1;
			if (test_mode_enable == 1)
				ep0_status(udc);
		}

		if (status < 0) {
do_stall:
			/* ep = &udc->ep[0];*/
			/**ep->reg_control_status |= UDC_EP0_STALL;*/
			/* fail in the command parsing*/
			pDevReg->ControlEpControl |= EP_STALL;  /* stall the pipe*/
			ControlState = CONTROLSTATE_SETUP;      /* go back to setup state*/

			ep->stall = 1;
			DBG("Setup Command STALL : req %02x.%02x protocol STALL; stat %d\n",
			u.r.bRequestType, u.r.bRequest, status);

			udc->ep0_pending = 0;
		} /*if (status < 0)*/
	} /*if (udc_irq_src & UDC_IS_SETUP_INT)*/
} /*udc_control_prepare_data_resp()*/

#define OTG_FLAGS (UDC_B_HNP_ENABLE|UDC_A_HNP_SUPPORT|UDC_A_ALT_HNP_SUPPORT)

static void devstate_irq(struct vt8500_udc *udc, u32 port0_status)
{
	udc->usb_connect = 0;

	if (pDevReg->IntEnable & INTENABLE_DEVICERESET) {
		/*vt3357 hw issue : clear all device register.*/
		/*payload & address needs re-set again...*/
		pDevReg->IntEnable |= INTENABLE_SUSPENDDETECT;
		VDBG("devstate_irq() - Global Reset...\n");
//printk("[devstate_irq]0xd8009834 = 0x%8.8x\n",*(volatile unsigned int *)(USB_IP_BASE+0x2034));

		//spin_unlock(&udc->lock);
//		spin_unlock_irqrestore(&udc->lock, irq_flags);
		
		if (udc->driver) {
		    udc->gadget.speed = USB_SPEED_UNKNOWN;
//		    ppudc = udc;
//				run_script(7); 
//				udc->driver->disconnect(&udc->gadget);
		}
		//spin_lock(&udc->lock);
//		spin_lock_irqsave(&udc->lock, irq_flags);

		udc->ep0_set_config = 0;
		pDevReg->Bulk1DesStatus = 0;

		udc->ep[0].toggle_bit = 0;
		udc->ep[1].toggle_bit = 0;
		udc->ep[2].toggle_bit = 0;
		udc->ep[3].toggle_bit = 0;
		udc->ep[4].toggle_bit = 0;
		udc->ep[5].toggle_bit = 0;//gri
		udc->ep[6].toggle_bit = 0;//gri


		udc->ep[0].rndis = 0;
		udc->ep[1].rndis = 0;
		udc->ep[2].rndis = 0;
		udc->ep[3].rndis = 0;
		udc->ep[4].rndis = 0;
		udc->ep[5].rndis = 0;//gri
		udc->ep[6].rndis = 0;//gri    

		udc->file_storage_set_halt = 0;
		udc->bulk_out_dma_write_error = 0;

		udc->gadget.speed = USB_SPEED_UNKNOWN;
		wmt_pdma_reset();

        gadget_connect=0;                               
        run_script(2);

        
		/*vt8500_usb_device_reg_dump();*/

	} /*if (pDevReg->IntEnable & INTENABLE_DEVICERESET)*/

	if (pDevReg->IntEnable & INTENABLE_SUSPENDDETECT) {
		VDBG("devstate_irq() - Global Suspend...\n");
		pDevReg->IntEnable |= INTENABLE_SUSPENDDETECT;

#if 0
		if (udc->driver->suspend) {
			//spin_unlock(&udc->lock);
			spin_unlock_irqrestore(&udc->lock, irq_flags);
			VDBG("suspend, gadget %s\n", udc->driver->driver.name);
			printk(KERN_INFO "suspend\n"); //gri
			udc->driver->suspend(&udc->gadget);
			//spin_lock(&udc->lock);
			spin_lock_irqsave(&udc->lock, irq_flags);
		} /*if (udc->driver->suspend)*/

    gadget_connect=0;                               
    run_script(2);
#endif
        
	} /*if (pDevReg->IntEnable & INTENABLE_SUSPENDDETECT)*/

	if (pDevReg->IntEnable & INTENABLE_RESUMEDETECT) {
		pDevReg->IntEnable |= INTENABLE_RESUMEDETECT;
		VDBG("devstate_irq() - Global Resume...\n");
#if 0		
		if (udc->driver->resume) {
			//spin_unlock(&udc->lock);
			spin_unlock_irqrestore(&udc->lock, irq_flags);
			VDBG("resume, gadget %s\n", udc->driver->driver.name);
			printk(KERN_INFO "resume\n"); //gri
			udc->driver->resume(&udc->gadget);
			//spin_lock(&udc->lock);
			spin_lock_irqsave(&udc->lock, irq_flags);
		} /*if (udc->driver->resume)*/
#endif		
	} /*if (pDevReg->IntEnable & INTENABLE_RESUMEDETECT)*/

/*#ifdef UDC_A1_SELF_POWER_ENABLE*/
	/* USB Bus Connection Change*/
	/* clear connection change event*/
	if (pDevReg->PortControl & PORTCTRL_SELFPOWER)/* Device port control register         - 22)*/
		if (pDevReg->PortControl & PORTCTRL_CONNECTCHANGE)/* Device port control register   - 22)*/
			pDevReg->PortControl |= PORTCTRL_CONNECTCHANGE;/*   0x02 // connection change bit*/

/*#endif*/

	if (pDevReg->PortControl & PORTCTRL_FULLSPEEDMODE) {
		udc->gadget.speed = USB_SPEED_FULL;
		udc->usb_connect = 1;
		/*2007-8.27 GigaHsu : enable float, reset, suspend and resume IE*/
		/*after host controller connected.*/
		VDBG("devstate_irq() - full speed host connect...\n");
	} /*if(pDevReg->PortControl & PORTCTRL_FULLSPEEDMODE)*/

	if (pDevReg->PortControl & PORTCTRL_HIGHSPEEDMODE) {
		udc->gadget.speed = USB_SPEED_HIGH;
		udc->usb_connect = 1;
		/*2007-8.27 GigaHsu : enable float, reset, suspend and resume IE*/
		/*after host controller connected.*/
		VDBG("devstate_irq() - high speed host connect...\n");
	} /*if(pDevReg->PortControl & PORTCTRL_HIGHSPEEDMODE)*/

} /*static void devstate_irq()*/

void USB_ControlXferComplete(void)
{
	struct vt8500_ep	*ep;
	struct vt8500_req	*req;

	ep = &udc->ep[0];
	/* when ever a setup received, the Control state will reset*/
	/* check for the valid bit of the contol descriptor*/
	/*DBG("USB_ControlXferComplete()\n");*/
	if (pDevReg->ControlDesControl & CTRLXFER_CMDVALID) {
		ep->toggle_bit = 1;
		if (udc->usb_connect == 0) {
			if (pDevReg->PortControl & PORTCTRL_FULLSPEEDMODE) {
				udc->gadget.speed = USB_SPEED_FULL;
				udc->usb_connect = 1;
				udc->gadget.is_dualspeed = 1;
				/*2007-8.27 GigaHsu : enable float, reset, suspend and resume IE*/
				/*after host controller connected.*/
				wmt_udc_csr(udc);
				VDBG("devstate_irq() - full speed host connect...\n");
				USBState = USBSTATE_DEFAULT;
			} /*if(pDevReg->PortControl & PORTCTRL_FULLSPEEDMODE)*/

			if (pDevReg->PortControl & PORTCTRL_HIGHSPEEDMODE) {
				udc->gadget.speed = USB_SPEED_HIGH;
				udc->usb_connect = 1;
				udc->gadget.is_dualspeed = 1;
				/*2007-8.27 GigaHsu : enable float, reset, suspend and resume IE*/
				/*after host controller connected.*/
				wmt_udc_csr(udc);
				VDBG("devstate_irq() - high speed host connect...\n");
				USBState = USBSTATE_DEFAULT;
			} /*if(pDevReg->PortControl & PORTCTRL_HIGHSPEEDMODE)*/
		}
		/* HP11_Begin*/
		/* clear the command valid bit*/
		/* pDevReg->ControlDesControl |= CTRLXFER_CMDVALID;*/
		/* always clear control stall when SETUP received*/
		pDevReg->ControlEpControl &= 0x17; /* clear the stall*/
		ControlState = CONTROLSTATE_DATA;
		udc_control_prepare_data_resp();  /*processing SETUP command ...*/
		pDevReg->ControlDesControl &= 0xEF;/*(~CTRLXFER_CMDVALID);*/
		/*HP11_End*/
		return;
	}

	if (udc->ep0_in) {
		/* IN dma: tx to host*/
		if (!list_empty(&ep->queue)) { /* >64 bytes for prepare DATA*/
			req = container_of(ep->queue.next, struct vt8500_req, queue);
			VDBG("dma_irq : finish_in_dma() EP0  %s\n", ep ? ep->ep.name : NULL);
			finish_in_dma(ep, req, 0);
		}

		while (pDevReg->ControlEpControl & EP_COMPLETEINT) /* clear the event*/
			;
		if (!list_empty(&ep->queue)) { /* >64 bytes for prepare DATA*/
			req = container_of(ep->queue.next, struct vt8500_req, queue);
			VDBG("dma_irq : next_in_dma() EP0  %s\n", ep ? ep->ep.name : NULL);
			next_in_dma(ep, req);
		}
  } else {/*ep0 out*/
		/* OUT dma: rx from host*/
		if (!list_empty(&ep->queue)) {
			req = container_of(ep->queue.next, struct vt8500_req, queue);
			finish_out_dma(ep, req, 0);
		}

		while (pDevReg->ControlEpControl & EP_COMPLETEINT) /* clear the event*/
			;
		if (!list_empty(&ep->queue)) { /* >64 bytes for prepare DATA*/
			req = container_of(ep->queue.next, struct vt8500_req, queue);
			next_out_dma(ep, req);
		}
	}

} /*void USB_ControlXferComplete(void)*/
static irqreturn_t
wmt_udc_dma_irq(int irq, void *_udc)//, struct pt_regs *r)
{
	irqreturn_t	status = IRQ_NONE;
	u32	bulk_dma_csr;

	bulk_dma_csr = pUdcDmaReg->DMA_ISR;
	/*printk("[udc_dma_isr]dma int_sts = 0x%8.8x\n",pUdcDmaReg->DMA_ISR);*/

	if (bulk_dma_csr & DMA_INTERRUPT_STATUS0) {
		/*	printk("[udc_dma_isr]channel0 event = 0x%8.8x\n",pUdcDmaReg->DMA_Context_Control0);*/
		pUdcDmaReg->DMA_ISR |= DMA_INTERRUPT_STATUS0;
		status = IRQ_HANDLED;
	}

	if (bulk_dma_csr & DMA_INTERRUPT_STATUS1) {
		/*	printk("[udc_dma_isr]channel1 event = 0x%8.8x\n",pUdcDmaReg->DMA_Context_Control1);*/
		pUdcDmaReg->DMA_ISR |= DMA_INTERRUPT_STATUS1;
		status = IRQ_HANDLED;
	}

	return status;
}

void reset_udc(void)
{
/*	if (!((*(volatile unsigned int *)(PM_CTRL_BASE_ADDR + 0x254))&0x00000080))*/
/*		*(volatile unsigned int *)(PM_CTRL_BASE_ADDR + 0x254) |= 0x00000080;*/      /*DPM needed*/

pDevReg->CommandStatus |= USBREG_RESETCONTROLLER;

	if ((*(volatile unsigned char *)(USB_IP_BASE + 0x249))&0x04) {
		*(volatile unsigned char*)(USB_IP_BASE + 0x249)&= ~0x04;
		mdelay(1);
	}

	pUSBMiscControlRegister5 = (unsigned char *)(USB_UDC_REG_BASE + 0x1A0);
	*pUSBMiscControlRegister5 = 0x01;/*USB in normal operation*/
	/* reset Bulk descriptor control*/
	pDevReg->Bulk1EpControl = 0; /* stop the bulk DMA*/
	while (pDevReg->Bulk1EpControl & EP_ACTIVE) /* wait the DMA stopped*/
		;

	pDevReg->Bulk2EpControl = 0; /* stop the bulk DMA*/
	while (pDevReg->Bulk2EpControl & EP_ACTIVE) /* wait the DMA stopped*/
		;

	pDevReg->Bulk1DesStatus = 0x00;
	pDevReg->Bulk2DesStatus = 0x00;

	pDevReg->Bulk1DesTbytes0 = 0;
	pDevReg->Bulk1DesTbytes1 = 0;
	pDevReg->Bulk1DesTbytes2 = 0;

	pDevReg->Bulk2DesTbytes0 = 0;
	pDevReg->Bulk2DesTbytes1 = 0;
	pDevReg->Bulk2DesTbytes2 = 0;

	/* enable DMA and run the control endpoint*/
	pDevReg->ControlEpControl = EP_RUN + EP_ENABLEDMA;
	/* enable DMA and run the bulk endpoint*/
	pDevReg->Bulk1EpControl = EP_RUN + EP_ENABLEDMA;
	pDevReg->Bulk2EpControl = EP_RUN + EP_ENABLEDMA;
	pDevReg->InterruptEpControl = EP_RUN + EP_ENABLEDMA;
	/* enable DMA and run the interrupt endpoint*/
	/* UsbControlRegister.InterruptEpControl = EP_RUN+EP_ENABLEDMA;*/
	/* run the USB controller*/
	pDevReg->MiscControl3 = 0x3d;
	pDevReg->PortControl |= PORTCTRL_SELFPOWER;/* Device port control register - 22*/
	pDevReg->CommandStatus = USBREG_RUNCONTROLLER;

	ControlState = CONTROLSTATE_SETUP;
	USBState = USBSTATE_DEFAULT;
	TestMode = 0;

	/*status = wmt_udc_setup(odev, xceiv);*/
	wmt_ep_setup_csr("ep0", 0, USB_ENDPOINT_XFER_CONTROL, 64);
	wmt_ep_setup_csr("ep1in" "-bulk", (USB_DIR_IN | 1), USB_ENDPOINT_XFER_BULK, 512);
	wmt_ep_setup_csr("ep2out" "-bulk", (USB_DIR_OUT | 2), USB_ENDPOINT_XFER_BULK, 512);
	wmt_ep_setup_csr("ep3in""-int", (USB_DIR_IN | 3), USB_ENDPOINT_XFER_INT, 8);

	pDevReg->SelfPowerConnect |= 0x10;//Neil

	/* enable all interrupt*/
#ifdef FULL_SPEED_ONLY	
	pDevReg->IntEnable = (INTENABLE_ALL | INTENABLE_FULLSPEEDONLY) ;/*0x70*/
#else
	pDevReg->IntEnable = INTENABLE_ALL;/*0x70*/
#endif	

	/* set IOC on the Setup decscriptor to accept the Setup request*/
	pDevReg->ControlDesControl = CTRLXFER_IOC;
/*Neil_080731*/
/* pullup_disable here and enable in /arch/arm/kernel/apm.c:395
	apm_ioctl() to patch issue signal fail when resume when recive*/
/* set_configuration time out.*/
/*    pullup_enable(udc);//usb_gadget_register_driver()*/
	wmt_pdma_reset();
	pullup_enable(udc);
	pDevReg->FunctionPatchEn |= 0x20; /* HW attach process evaluation enable bit*/
/*	pullup_disable(udc);*/
/*Neil_080731*/
	
}

static irqreturn_t
wmt_udc_irq(int irq, void *_udc)//, struct pt_regs *r)
{
	struct vt8500_udc	*udc = _udc;
  u32 udc_irq_src;
	irqreturn_t	status = IRQ_NONE;
//	unsigned long	flags;
#ifdef OTGIP
	u32 global_irq_src;
#endif
	/*u32 i;*/

	spin_lock_irqsave(&udc->lock, irq_flags);

  DBG("wmt_udc_irq()\n");

#ifdef OTGIP
	/*Global Interrupt Status*/
	global_irq_src = pGlobalReg->UHDC_Interrupt_Status;

	if (global_irq_src & UHDC_INT_UDC) {/*UDC Core + UDC DMA1 + UDC DMA2 */
		if (global_irq_src & UHDC_INT_UDC_CORE) {/*UDC Core*/
#endif
			/*connection interrupt*/
		if (pDevReg->SelfPowerConnect & 0x02) {
			//struct usb_gadget_driver *driver = udc->driver;
			
			if (pDevReg->SelfPowerConnect & 0x01)  //connect
            {         
#if 0            
//                run_script(4);
                gadget_connect=1;

                run_script(1);
#endif                
//				wmt_pdma_reset();               
reset_udc();
            }
			else {//disconnct
				//spin_unlock(&udc->lock);
//				spin_unlock_irqrestore(&udc->lock, irq_flags);
//				if (udc->driver)
//					udc->driver->disconnect(&udc->gadget);
//				ppudc = udc;
				printk(KERN_INFO "disconnect 1\n"); //gri
				run_script(7);                
                gadget_connect=0;                               
                run_script(2);
                
		pullup_disable(udc);                
                
//                if (gadget_mode)
//                    schedule_work(&reinsmod_thread);                
//                run_script(3);                
				//spin_lock(&udc->lock);
//				spin_lock_irqsave(&udc->lock, irq_flags);
			}
			
			status = IRQ_HANDLED;
			pDevReg->SelfPowerConnect |= 0x02;
		}

			/* Device Global Interrupt Pending Status*/
			udc_irq_src = pDevReg->CommandStatus;

			/* Device state change (usb ch9 stuff)*/
			if (udc_irq_src & USBREG_BUSACTINT) {/* the bus activity interrupt occured*/
				/*caused by Port 0 Statsu Change*/
				devstate_irq(_udc, udc_irq_src);
				status = IRQ_HANDLED;
				pDevReg->CommandStatus |= USBREG_BUSACTINT;
			}

			if (udc_irq_src & USBREG_BABBLEINT) {/* the Babble interrupt ocuured*/
				/* check for Control endpoint for BABBLE error*/
				if (pDevReg->ControlEpControl & EP_BABBLE) {
					/* the Control endpoint is encounted the babble error*/
					/* stall the endpoint and clear the babble condition*/
					pDevReg->ControlEpControl |= (EP_BABBLE + EP_STALL);
					INFO("vt8430_udc_irq() - EP0 Babble Detect!\n");
					udc->ep[0].stopped = 1;
					udc->ep[0].stall = 1;
				}

				if (pDevReg->Bulk1EpControl & EP_BABBLE) {
					/* the Bulk endpoint is encounted the babble error*/
					/* stall the endpoint and clear the babble condition*/
					pDevReg->Bulk1EpControl |= (EP_BABBLE + EP_STALL);
					INFO("vt8430_udc_irq() - EP1 Babble Detect!\n");
					udc->ep[1].stopped = 1;
					udc->ep[1].stall = 1;
				}

				if (pDevReg->Bulk2EpControl & EP_BABBLE) {
					/* the Bulk endpoint is encounted the babble error*/
					/* stall the endpoint and clear the babble condition*/
					pDevReg->Bulk2EpControl |= (EP_BABBLE + EP_STALL);
					INFO("vt8430_udc_irq() - EP2 Babble Detect!\n");
					udc->ep[2].stopped = 1;
					udc->ep[2].stall = 1;
				}
				status = IRQ_HANDLED;
			}

			if (udc_irq_src & USBREG_COMPLETEINT) {/* the complete inerrupt occured*/
				if (pDevReg->ControlEpControl & EP_COMPLETEINT) {
					/* the control transfer complete event*/
					pDevReg->ControlEpControl |= EP_COMPLETEINT; /* clear the event*/
					USB_ControlXferComplete();
#if 0
//                run_script(4);
                gadget_connect=1;

                run_script(1);
#endif                     

				}

				if (pDevReg->Bulk1EpControl & EP_COMPLETEINT) {
					/* the bulk transfer complete event*/
					pDevReg->Bulk1EpControl |= EP_COMPLETEINT;
					/*DBG("USB_Bulk 1 DMA()\n");*/
					dma_irq(0x81);
#if 1            
                    gadget_connect=1;
                    run_script(1);
#endif                     
				}

				if (pDevReg->Bulk2EpControl & EP_COMPLETEINT) {
					/* the bulk transfer complete event*/
					pDevReg->Bulk2EpControl |= EP_COMPLETEINT;
					/*DBG("USB_Bulk 2 DMA()\n");*/
					dma_irq(2);
#if 1            
                    gadget_connect=1;
                    run_script(1);
#endif                                         
				}

				if (pDevReg->InterruptEpControl & EP_COMPLETEINT) {
					/* the bulk transfer complete event*/
					pDevReg->InterruptEpControl |= EP_COMPLETEINT;
					/*DBG("USB_INT 3 DMA()\n");*/
					dma_irq(0x83);
#if 1            
                    gadget_connect=1;
                    run_script(1);
#endif                                         
				}
				status = IRQ_HANDLED;
			}

			if	(udc->ep0_status_0_byte == 1)/* && (udc_ep[0].stall==0))*/
				ep0_status(udc);

			if (ControlState == CONTROLSTATE_STATUS) {
				/* checking for test mode*/
				if (TestMode != 0x00) {
					pDevReg->CommandStatus &= 0x1F;
					pDevReg->CommandStatus |= USBREG_RESETCONTROLLER;
					while (pDevReg->CommandStatus & USBREG_RESETCONTROLLER)
						;
					/* HW attach process evaluation enable bit*/
					pDevReg->FunctionPatchEn |= 0x20;
					/* Device port control register - 22*/
					pDevReg->PortControl |= PORTCTRL_SELFPOWER;

					/*GigaHsu 2008.1.26 : Don't set RUN bit while TestMode enable*/
					/* setting the test mode*/
					switch (TestMode) {
					case UDC_TEST_MODE_NOT_ENABLED:/*0*/
						INFO("UDC_TEST_MODE_NOT_ENABLED (0x%02X)\n", TestMode);
					break;

					case UDC_TEST_J_STATE:/*1*/
						pDevReg->CommandStatus = USBREG_TEST_J;
						INFO("UDC_TEST_J_STATE wIndex(0x%02X)\n", TestMode);
					break;

					case UDC_TEST_K_STATE:/*2*/
						pDevReg->CommandStatus = USBREG_TEST_K;
						INFO("UDC_TEST_K_STATE wIndex(0x%02X)\n", TestMode);
					break;

					case UDC_TEST_SE0_NAK:/*3*/
						pDevReg->CommandStatus = USBREG_SE0_NAK;
						INFO("UDC_TEST_SE0_NAK wIndex(0x%02X)\n", TestMode);
					break;

					case UDC_TEST_PACKET:/*4*/
						pDevReg->CommandStatus = USBREG_TESTPACKET;
						INFO("UDC_TEST_PACKET wIndex(0x%02X)\n", TestMode);
					break;

					case UDC_TEST_FRORCE_ENABLE:/*5*/
						pDevReg->CommandStatus = USBREG_FORCEENABLE;
						INFO("UDC_TEST_FRORCE_ENABLE wIndex(0x%02X)\n", TestMode);
					break;

					case UDC_TEST_EYE_PATTERN:/*6*/
						pDevReg->CommandStatus = USBREG_TESTEYE;
						INFO("UDC_TEST_EYE_PATTERN wIndex(0x%02X)\n", TestMode);
					break;
					} /*switch(TestMode)*/

					INFO("UDC - CommandStatus(0x%02X)\n", pDevReg->CommandStatus);
					/* stop the 8051*/
					/*ChipTopRegister.Config |= CT_STOP8051;*/
				}
				ControlState = CONTROLSTATE_SETUP; /* go back SETUP state*/
			}
#ifdef OTGIP
		}
		if (global_irq_src & UHDC_INT_UDC_DMA1)/*UDC Bulk DMA 1*/
			wmt_udc_dma_irq(UDC_IRQ_USB, udc);//, r);

		if (global_irq_src & UHDC_INT_UDC_DMA2)/*UDC Bulk DMA 1*/
			wmt_udc_dma_irq(UDC_IRQ_USB, udc);//, r);

	}
#endif
   spin_unlock_irqrestore(&udc->lock, irq_flags);

	return status;
} /*wmt_udc_irq()*/

/* workaround for seemingly-lost IRQs for RX ACKs... */
#define PIO_OUT_TIMEOUT	(jiffies + HZ/3)

#ifdef DEBUG_UDC_ISR_TIMER
static void  wmt_udc_softirq(unsigned long _udc)
{
	struct vt8500_udc	*udc = (void *) _udc;
	/*unsigned long	flags;*/
	struct pt_regs r;

	/*spin_lock_irqsave(&udc->lock, irq_flags);*/
	INFO("wmt_udc_softirq()\n");
	wmt_udc_irq(UDC_IRQ_USB, udc, &r);
	mod_timer(&udc->timer, PIO_OUT_TIMEOUT);
	/*spin_unlock_irqrestore(&udc->lock, irq_flags);*/
}
#endif

static void __init wmt_ep_setup(char *name, u8 addr, u8 type, unsigned maxp);

int usb_gadget_register_driver(struct usb_gadget_driver *driver)
{
	int		status = -ENODEV;
	struct vt8500_ep	*ep;
//	unsigned long	flags;

	DBG("usb_gadget_register_driver()\n");

	/* basic sanity tests */
	if (!udc)
		return -ENODEV;
	if (!driver
			/* FIXME if otg, check:  driver->is_otg*/
			|| driver->speed < USB_SPEED_FULL
			|| !driver->bind
//			|| !driver->unbind
			|| !driver->setup){
//        printk(KERN_INFO "gri usb_gadget_register_driver f2\n");
//        printk(KERN_INFO "gri driver=%x driver->speed=%x driver->bind=%x driver->unbind=%x driver->setup=%x\n",
//            (unsigned int)driver,(unsigned int)driver->speed,(unsigned int)driver->bind,(unsigned int)driver->unbind,(unsigned int)driver->setup);
		return -EINVAL;
       }

	spin_lock_irqsave(&udc->lock, irq_flags);
	if (udc->driver) {
		spin_unlock_irqrestore(&udc->lock, irq_flags);
		return -EBUSY;
	}

	/* reset state */
	list_for_each_entry(ep, &udc->gadget.ep_list, ep.ep_list) {
		ep->irqs = 0;
		if (ep->bmAttributes == USB_ENDPOINT_XFER_ISOC)
			continue;
		/*use_ep(ep, 0);*/
		/*UDC_CTRL_REG = UDC_SET_HALT;*/
	}
	udc->ep0_pending = 0;
	udc->ep0_status_0_byte = 0;
	udc->ep[0].irqs = 0;
	udc->softconnect = 1;
	udc->file_storage_set_halt = 0;
	/* hook up the driver */
	driver->driver.bus = 0;
	udc->driver = driver;
	udc->gadget.dev.driver = &driver->driver;
	spin_unlock_irqrestore(&udc->lock, irq_flags);

printk(KERN_INFO "bind to %s --> %d\n", driver->driver.name, status);

	status = driver->bind(&udc->gadget);
	if (status) {
		/*          %s -> g_file_storage*/
		DBG("bind to %s --> %d\n", driver->driver.name, status);
//        printk(KERN_INFO "gri usb_gadget_register_driver %d\n", status);
		udc->gadget.dev.driver = 0;
		udc->driver = 0;
		goto done;
	}

	DBG("bound to driver %s\n", driver->driver.name);/*udc: bound to driver g_file_storage*/
	pUSBMiscControlRegister5 = (unsigned char *)(USB_UDC_REG_BASE + 0x1A0);

	*pUSBMiscControlRegister5 = 0x01;/*USB in normal operation*/
	/* reset Bulk descriptor control*/
	pDevReg->Bulk1EpControl = 0; /* stop the bulk DMA*/
	while (pDevReg->Bulk1EpControl & EP_ACTIVE)
		; /* wait the DMA stopped*/

	pDevReg->Bulk2EpControl = 0; /* stop the bulk DMA*/
	while (pDevReg->Bulk2EpControl & EP_ACTIVE)
		; /* wait the DMA stopped*/

	pDevReg->Bulk1DesStatus = 0x00;
	pDevReg->Bulk2DesStatus = 0x00;

	pDevReg->Bulk1DesTbytes0 = 0;
	pDevReg->Bulk1DesTbytes1 = 0;
	pDevReg->Bulk1DesTbytes2 = 0;

	pDevReg->Bulk2DesTbytes0 = 0;
	pDevReg->Bulk2DesTbytes1 = 0;
	pDevReg->Bulk2DesTbytes2 = 0;

	/* enable DMA and run the control endpoint*/
	pDevReg->ControlEpControl = EP_RUN + EP_ENABLEDMA;
	/* enable DMA and run the bulk endpoint*/
	pDevReg->Bulk1EpControl = EP_RUN + EP_ENABLEDMA;
	pDevReg->Bulk2EpControl = EP_RUN + EP_ENABLEDMA;
	pDevReg->InterruptEpControl = EP_RUN + EP_ENABLEDMA;
	/* enable DMA and run the interrupt endpoint*/
	/* UsbControlRegister.InterruptEpControl = EP_RUN+EP_ENABLEDMA;*/
	/* run the USB controller*/
	pDevReg->MiscControl3 = 0x3d;


	pDevReg->PortControl |= PORTCTRL_SELFPOWER;/* Device port control register - 22*/

	pDevReg->CommandStatus = USBREG_RUNCONTROLLER;

	ControlState = CONTROLSTATE_SETUP;
	USBState = USBSTATE_DEFAULT;
	TestMode = 0;

	wmt_ep_setup_csr("ep0", 0, USB_ENDPOINT_XFER_CONTROL, 64);
	wmt_ep_setup_csr("ep1in" "-bulk", (USB_DIR_IN | 1), USB_ENDPOINT_XFER_BULK, 512);
	wmt_ep_setup_csr("ep2out" "-bulk", (USB_DIR_OUT | 2), USB_ENDPOINT_XFER_BULK, 512);
	wmt_ep_setup_csr("ep3in""-int", (USB_DIR_IN | 3), USB_ENDPOINT_XFER_INT, 8);
	wmt_ep_setup_csr("ep5in" "-bulk", (USB_DIR_IN | 5), USB_ENDPOINT_XFER_BULK, 512);//gri
	wmt_ep_setup_csr("ep6out" "-bulk", (USB_DIR_OUT | 6), USB_ENDPOINT_XFER_BULK, 512);//gri

	/* enable all interrupt*/
#ifdef FULL_SPEED_ONLY	
	pDevReg->IntEnable = (INTENABLE_ALL | INTENABLE_FULLSPEEDONLY) ;/*0x70*/
#else
	pDevReg->IntEnable = INTENABLE_ALL;/*0x70*/
#endif
	/* set IOC on the Setup decscriptor to accept the Setup request*/
	pDevReg->ControlDesControl = CTRLXFER_IOC;
	wmt_pdma_reset();/*NeilChen*/

if (!strcmp(driver->driver.name,"g_file_storage"))
{
    gadget_mode= 1;//storage
//    gadget_mount=0;
    
#if 0   
        if (gadget_mode && (gadget_mount== 0))
        {
            gadget_mount=1;
            run_umount();
        }            
#endif  

	pullup_enable(udc);/*usb_gadget_register_driver()*/  
//    run_script(1);
}
else
    gadget_mode = 0;

	pDevReg->FunctionPatchEn |= 0x20; /* HW attach process evaluation enable bit*/

#ifdef DEBUG_UDC_ISR_TIMER
	init_timer(&udc->timer);
	udc->timer.function = wmt_udc_softirq;
	udc->timer.data = (unsigned long) udc;
	add_timer(&udc->timer);
#endif

done:
	return status;
} /*int usb_gadget_register_driver ()*/

EXPORT_SYMBOL(usb_gadget_register_driver);

int usb_gadget_unregister_driver(struct usb_gadget_driver *driver)
{
//	unsigned long	flags;
	int		status = -ENODEV;

	if (!udc)
		return -ENODEV;
	if (!driver || driver != udc->driver)
		return -EINVAL;

	spin_lock_irqsave(&udc->lock, irq_flags);
	udc_quiesce(udc);
	spin_unlock_irqrestore(&udc->lock, irq_flags);

	driver->unbind(&udc->gadget);
	udc->gadget.dev.driver = 0;
	udc->driver = 0;

    gadget_mode=0;
//    gadget_mount=0;

	pDevReg->IntEnable &= 0x8F;/*INTENABLE_ALL(0x70)*/
	/* set IOC on the Setup decscriptor to accept the Setup request*/
	pDevReg->ControlDesControl &= 0x7F;/*CTRLXFER_IOC;*/

	if (udc->transceiver)
		(void) otg_set_peripheral(udc->transceiver, 0);
	else {
		pullup_disable(udc);
		pDevReg->FunctionPatchEn &= ~0x20;
	}

	if (TestMode != 0x00) {
		pDevReg->CommandStatus &= 0x1F;

		pUSBMiscControlRegister5 = (unsigned char *)(USB_UDC_REG_BASE + 0x1A0);
		*pUSBMiscControlRegister5 = 0x00;/* USB PHY in power down  & USB in reset*/

		INFO("usb_gadget_unregister_driver() RESET_UDC OK!\n");
		TestMode = 0;
	}

	DBG("unregistered driver '%s'\n", driver->driver.name);
	return status;
} /*int usb_gadget_unregister_driver ()*/
EXPORT_SYMBOL(usb_gadget_unregister_driver);


/*-------------------------------------------------------------------------*/

#ifdef CONFIG_USB_vt8500_PROC

#include <linux/seq_file.h>

static const char proc_filename[] = "driver/udc";

#define FOURBITS "%s%s%s%s"
#define EIGHTBITS FOURBITS FOURBITS

static void proc_ep_show(struct seq_file *s, struct vt8500_ep *ep)
{

} /*proc_ep_show()*/


static char *trx_mode(unsigned m)
{
	switch (m) {
	case 3:
	case 0:		return "6wire";
	case 1:		return "4wire";
	case 2:		return "3wire";
	default:	return "unknown";
	}
}

static int proc_udc_show(struct seq_file *s, void *_)
{
	return 0;
}

static int proc_udc_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_udc_show, 0);
}

static struct file_operations proc_ops = {
	.open		= proc_udc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static void create_proc_file(void)
{
	struct proc_dir_entry *pde;

	pde = create_proc_entry(proc_filename, 0, NULL);
	if (pde)
		pde->proc_fops = &proc_ops;
}

static void remove_proc_file(void)
{
	remove_proc_entry(proc_filename, 0);
}

#else

static inline void create_proc_file(void) {}
static inline void remove_proc_file(void) {}

#endif

/*-------------------------------------------------------------------------*/

/* Before this controller can enumerate, we need to pick an endpoint
 * configuration, or "fifo_mode"  That involves allocating 2KB of packet
 * buffer space among the endpoints we'll be operating.
 */
static void __init
wmt_ep_setup(char *name, u8 addr, u8 type,
		unsigned maxp)
{
	struct vt8500_ep	*ep;

	VDBG("wmt_ep_setup()\n");

	/* OUT endpoints first, then IN*/
	ep = &udc->ep[addr & 0x7f];

	if (type == USB_ENDPOINT_XFER_CONTROL) {
		/*pDevReg->ControlEpControl;*/
		pDevReg->ControlEpReserved = 0;
		pDevReg->ControlEpMaxLen = (maxp & 0xFF);
		pDevReg->ControlEpEpNum = 0;
	} else if (type == USB_ENDPOINT_XFER_BULK) {
		if (addr & USB_DIR_IN) {
			/*pDevReg->Bulk1EpControl;*/
			pDevReg->Bulk1EpOutEpNum = (addr & 0x7f) << 4;
			pDevReg->Bulk1EpMaxLen = (maxp & 0xFF);
			pDevReg->Bulk1EpInEpNum = (((addr & 0x7f) << 4) | ((maxp & 0x700) >> 8));
		} else {
			/*pDevReg->Bulk2EpControl;*/
			pDevReg->Bulk2EpOutEpNum = (addr & 0x7f) << 4;
			pDevReg->Bulk2EpMaxLen = (maxp & 0xFF);
			pDevReg->Bulk2EpInEpNum = (((addr & 0x7f) << 4) | ((maxp & 0x700) >> 8));
		}
	} else if (type == USB_ENDPOINT_XFER_INT) {
		/*pDevReg->InterruptEpControl; // Interrupt endpoint control           - 2C*/
		pDevReg->InterruptReserved = (addr & 0x7f) << 4;/*Interrupt endpoint reserved byte  - 2D*/
		pDevReg->InterruptEpMaxLen = (maxp & 0xFF); /* Interrupt maximum transfer length    - 2E*/
		pDevReg->InterruptEpEpNum = (((addr & 0x7f) << 4) | ((maxp & 0x700) >> 8));
	}
	VDBG("wmt_ep_setup() - %s addr %02x  maxp %d\n",
		name, addr, maxp);

	strlcpy(ep->name, name, sizeof ep->name);
	INIT_LIST_HEAD(&ep->queue);
	ep->bEndpointAddress = addr;
	ep->bmAttributes = type;
	ep->double_buf = 0;/*dbuf;*/
	ep->udc = udc;
	ep->toggle_bit = 0;
	ep->ep.name = ep->name;
	ep->ep.ops = &wmt_ep_ops;/*struct usb_ep_ops*/
	ep->ep.maxpacket = ep->maxpacket = maxp;
	list_add_tail(&ep->ep.ep_list, &udc->gadget.ep_list);

	/*return buf;*/
} /*wmt_ep_setup()*/

static void wmt_udc_release(struct device *dev)
{
	DBG("wmt_udc_release()\n");
	complete(udc->done);
	kfree(udc);
	udc = 0;
} /*wmt_udc_release()*/

static int __init
wmt_udc_setup(struct platform_device *odev, struct otg_transceiver *xceiv)
{
	DBG("wmt_udc_setup()\n");
	udc = kmalloc(sizeof *udc, GFP_KERNEL);

	if (!udc)
		return -ENOMEM;

	memset(udc, 0, sizeof *udc);
	spin_lock_init(&udc->lock);
	spin_lock_init(&gri_lock);

	udc->gadget.ops = &wmt_gadget_ops;
	udc->gadget.ep0 = &udc->ep[0].ep;
	INIT_LIST_HEAD(&udc->gadget.ep_list);
	/*INIT_LIST_HEAD(&udc->iso);*/
	udc->gadget.speed = USB_SPEED_UNKNOWN;
	udc->gadget.name = driver_name;

	device_initialize(&udc->gadget.dev);
	//strcpy(udc->gadget.dev.bus_id, "gadget");
	dev_set_name(&udc->gadget.dev, "gadget");
	udc->gadget.dev.release = wmt_udc_release;
	udc->transceiver = NULL;
	/* ep0 is special; put it right after the SETUP buffer*/
	wmt_ep_setup("ep0", 0, USB_ENDPOINT_XFER_CONTROL, 64);

	list_del_init(&udc->ep[0].ep.ep_list);

#define VT8430_BULK_EP(name, addr) \
	wmt_ep_setup(name "-bulk", addr, \
		USB_ENDPOINT_XFER_BULK, 512);

/*usb_ch9.h*/
/*#define USB_DIR_OUT			0		*//* to device */
/*#define USB_DIR_IN			0x80		*//* to host */
#define VT8430_INT_EP(name, addr, maxp) \
	wmt_ep_setup(name "-int", addr, \
		USB_ENDPOINT_XFER_INT, maxp);

	VT8430_BULK_EP("ep1in",  USB_DIR_IN  | 1);
	VT8430_BULK_EP("ep2out", USB_DIR_OUT | 2);
	VT8430_BULK_EP("ep5in",  USB_DIR_IN  | 5);//gri
	VT8430_BULK_EP("ep6out", USB_DIR_OUT | 6);//gri
	VT8430_INT_EP("ep3in",  USB_DIR_IN  | 3, 8);

	/*INFO("fifo mode %d, %d bytes not used\n", fifo_mode, 2048 - buf);*/
	return 0;
} /*wmt_udc_setup()*/

static void wmt_pdma_reset(void)
{

	if (!pUdcDmaReg->DMA_Global_Bits.DMAConrollerEnable)
		pUdcDmaReg->DMA_Global_Bits.DMAConrollerEnable = 1;/*enable DMA*/

	pUdcDmaReg->DMA_Global_Bits.SoftwareReset = 1;
	while (pUdcDmaReg->DMA_Global_Bits.SoftwareReset)/*wait reset complete*/
		;

	pUdcDmaReg->DMA_Global_Bits.DMAConrollerEnable = 1;/*enable DMA*/
	pUdcDmaReg->DMA_IER_Bits.DMAInterruptEnable0 = 1;
	pUdcDmaReg->DMA_IER_Bits.DMAInterruptEnable1 = 1;

	pUdcDmaReg->DMA_Context_Control0_Bis.TransDir = 0;
	pUdcDmaReg->DMA_Context_Control1_Bis.TransDir = 1;

	/*descriptor initial*/

} /*wmt_pdma_init*/

static void wmt_pdma0_reset(void)
{

	if (!pUdcDmaReg->DMA_Global_Bits.DMAConrollerEnable)
		pUdcDmaReg->DMA_Global_Bits.DMAConrollerEnable = 1;/*enable DMA*/

	pUdcDmaReg->DMA_Global_Bits.SoftwareReset0 = 1;
	while (pUdcDmaReg->DMA_Global_Bits.SoftwareReset0)/*wait reset complete*/
		;

	pUdcDmaReg->DMA_IER_Bits.DMAInterruptEnable0 = 1;

	pUdcDmaReg->DMA_Context_Control0_Bis.TransDir = 0;
	
	/*descriptor initial*/

} /*wmt_pdma_init*/


static void wmt_pdma1_reset(void)
{

	if (!pUdcDmaReg->DMA_Global_Bits.DMAConrollerEnable)
		pUdcDmaReg->DMA_Global_Bits.DMAConrollerEnable = 1;/*enable DMA*/

	pUdcDmaReg->DMA_Global_Bits.SoftwareReset1 = 1;
	while (pUdcDmaReg->DMA_Global_Bits.SoftwareReset1)/*wait reset complete*/
		;

	pUdcDmaReg->DMA_IER_Bits.DMAInterruptEnable1 = 1;

	pUdcDmaReg->DMA_Context_Control1_Bis.TransDir = 1;
	
	/*descriptor initial*/

} /*wmt_pdma_init*/


/*static void wmt_pdma_init(struct device *dev)*/
static void wmt_pdma_init(struct device *dev)
{

	UdcPdmaVirAddrLI = (unsigned int) dma_alloc_coherent(pDMADescLI, (size_t)0x100, (dma_addr_t *)(&UdcPdmaPhyAddrLI), GFP_KERNEL|GFP_ATOMIC);
	UdcPdmaVirAddrSI = (unsigned int) dma_alloc_coherent(pDMADescSI, (size_t)0x100, (dma_addr_t *)(&UdcPdmaPhyAddrSI), GFP_KERNEL|GFP_ATOMIC);
	UdcPdmaVirAddrLO = (unsigned int) dma_alloc_coherent(pDMADescLO, (size_t)0x100, (dma_addr_t *)(&UdcPdmaPhyAddrLO), GFP_KERNEL|GFP_ATOMIC);
	UdcPdmaVirAddrSO = (unsigned int) dma_alloc_coherent(pDMADescSO, (size_t)0x100, (dma_addr_t *)(&UdcPdmaPhyAddrSO), GFP_KERNEL|GFP_ATOMIC);

	pUdcDmaReg->DMA_Global_Bits.DMAConrollerEnable = 1;/*enable DMA*/
	pUdcDmaReg->DMA_Global_Bits.SoftwareReset = 1;
	while (pUdcDmaReg->DMA_Global_Bits.SoftwareReset)/*wait reset complete*/
		;
	pUdcDmaReg->DMA_Global_Bits.DMAConrollerEnable = 1;/*enable DMA*/
	pUdcDmaReg->DMA_IER_Bits.DMAInterruptEnable0 = 1;
	pUdcDmaReg->DMA_IER_Bits.DMAInterruptEnable1 = 1;

	pUdcDmaReg->DMA_Context_Control0_Bis.TransDir = 0;
	pUdcDmaReg->DMA_Context_Control1_Bis.TransDir = 1;

} /*wmt_pdma_init*/

/*static int __init wmt_udc_probe(struct device *dev)*/
static int __init wmt_udc_probe(struct platform_device *pdev)
{
/*	struct platform_device	*odev = to_platform_device(dev);*/
	struct device *dev = &pdev->dev;
	int			status = -ENODEV;
	struct otg_transceiver	*xceiv = 0;

	DBG("wmt_udc_probe()\n");

	/*UDC Register Space         0x400~0x7EF*/
	
	INIT_LIST_HEAD (&done_main_list);
	
	pDevReg    = (struct UDC_REGISTER *)USB_UDC_REG_BASE;
	pUdcDmaReg = (struct UDC_DMA_REG *)USB_UDC_DMA_REG_BASE;
	pSetupCommand = (PSETUPCOMMAND)(USB_UDC_REG_BASE + 0x300);
	pSetupCommandBuf = (unsigned char *)(USB_UDC_REG_BASE + 0x300);
	SetupBuf = (UCHAR *)(USB_UDC_REG_BASE + 0x340);
	IntBuf = (UCHAR *)(USB_UDC_REG_BASE + 0x40);
	pUSBMiscControlRegister5 = (unsigned char *)(USB_UDC_REG_BASE + 0x1A0);

#ifdef OTGIP
	/*UHDC Global Register Space 0x7F0~0x7F7*/
	pGlobalReg = (struct USB_GLOBAL_REG *) USB_GLOBAL_REG_BASE;
#endif

	*pUSBMiscControlRegister5 = 0x01;
	pDevReg->CommandStatus &= 0x1F;
	pDevReg->CommandStatus |= USBREG_RESETCONTROLLER;

	while (pDevReg->CommandStatus & USBREG_RESETCONTROLLER)
		;

	wmt_pdma_init(dev);

	pDevReg->Bulk1EpControl = 0; /* stop the bulk DMA*/
	while (pDevReg->Bulk1EpControl & EP_ACTIVE) /* wait the DMA stopped*/
		;
	pDevReg->Bulk2EpControl = 0; /* stop the bulk DMA*/
	while (pDevReg->Bulk2EpControl & EP_ACTIVE) /* wait the DMA stopped*/
		;
	pDevReg->Bulk1DesStatus = 0x00;
	pDevReg->Bulk2DesStatus = 0x00;

	pDevReg->Bulk1DesTbytes0 = 0;
	pDevReg->Bulk1DesTbytes1 = 0;
	pDevReg->Bulk1DesTbytes2 = 0;

	pDevReg->Bulk2DesTbytes0 = 0;
	pDevReg->Bulk2DesTbytes1 = 0;
	pDevReg->Bulk2DesTbytes2 = 0;

	/* enable DMA and run the control endpoint*/
	pDevReg->ControlEpControl = EP_RUN + EP_ENABLEDMA;
	/* enable DMA and run the bulk endpoint*/
	pDevReg->Bulk1EpControl = EP_RUN + EP_ENABLEDMA;
	pDevReg->Bulk2EpControl = EP_RUN + EP_ENABLEDMA;
	pDevReg->InterruptEpControl = EP_RUN + EP_ENABLEDMA;
	/* enable DMA and run the interrupt endpoint*/
	/* UsbControlRegister.InterruptEpControl = EP_RUN+EP_ENABLEDMA;*/
	/* run the USB controller*/
	pDevReg->MiscControl3 = 0x3d;

	/* HW attach process evaluation enable bit For WM3426 and after project*/
	/*pDevReg->FunctionPatchEn |= 0x20;*/
#ifdef HW_BUG_HIGH_SPEED_PHY
	pDevReg->MiscControl0 &= ~0x80;
#endif
	pDevReg->MiscControl0 |= 0x02;
	pDevReg->CommandStatus = USBREG_RUNCONTROLLER;

	ControlState = CONTROLSTATE_SETUP;
	USBState = USBSTATE_DEFAULT;
	TestMode = 0;

	status = wmt_udc_setup(pdev, xceiv);
	if (status)
		goto cleanup0;

	xceiv = 0;

	/*udc->chip_version = tmp8;*/
	/* "udc" is now valid*/
	pullup_disable(udc);

	udc->gadget.is_otg = 0;/*(config->otg != 0);*/
	udc->dev = dev;
	udc->gadget.is_dualspeed = 1;
	udc->ep0_status_0_byte = 0;
	udc->usb_connect = 0;
	/* USB general purpose IRQ:  ep0, state changes, dma, etc*/
	status = request_irq(UDC_IRQ_USB, wmt_udc_irq,
//		(SA_INTERRUPT | SA_SHIRQ | SA_SAMPLE_RANDOM), driver_name, udc);
//		(IRQF_DISABLED | IRQF_SHARED | IRQF_SAMPLE_RANDOM), driver_name, udc);		
		(IRQF_SHARED | IRQF_SAMPLE_RANDOM), driver_name, udc);//gri
	pDevReg->SelfPowerConnect |= 0x10;//Neil

#ifndef OTGIP
	status = request_irq(UDC_IRQ_DMA, wmt_udc_dma_irq,
//		(SA_INTERRUPT | SA_SHIRQ | SA_SAMPLE_RANDOM), driver_name, udc);
//		(IRQF_DISABLED | IRQF_SHARED | IRQF_SAMPLE_RANDOM), driver_name, udc);
		(IRQF_SHARED | IRQF_SAMPLE_RANDOM), driver_name, udc);		
#endif
	   /*SA_SAMPLE_RANDOM, driver_name, udc);*/
	if (status != 0) {
		ERR("can't get irq %d, err %d\n",
		UDC_IRQ_USB, status);
		goto cleanup1;
	} else
		INFO("wmt_udc_probe - request_irq(0x%02X) pass!\n", UDC_IRQ_USB);

	create_proc_file();
	device_add(&udc->gadget.dev);

	return 0;

/*cleanup2:*/
	free_irq(UDC_IRQ_USB, udc);
    INFO("wmt_udc_probe - free_irq(0x%02X)?\n", UDC_IRQ_USB);
cleanup1:
	kfree(udc);
	udc = 0;

cleanup0:
	if (xceiv)
		put_device(xceiv->dev);
	/*release_mem_region(odev->resource[0].start,*/
	/*		odev->resource[0].end - odev->resource[0].start + 1);*/
	return status;

} /*wmt_udc_probe()*/

static int __exit wmt_udc_remove(struct platform_device *pdev)
{
	/*struct platform_device	*odev = to_platform_device(dev);*/
	DECLARE_COMPLETION(done);
	DBG("wmt_udc_remove()\n");

	if (!udc)
		return -ENODEV;

	udc->done = &done;

	pullup_disable(udc);
	if (udc->transceiver) {
		put_device(udc->transceiver->dev);
		udc->transceiver = 0;
	}
	/*UDC_SYSCON1_REG = 0;*/

	remove_proc_file();

	free_irq(UDC_IRQ_USB, udc);

	device_unregister(&udc->gadget.dev);
	wait_for_completion(&done);

	return 0;
} /*wmt_udc_remove()*/

/* suspend/resume/wakeup from sysfs (echo > power/state) */

static int wmt_udc_suspend(struct platform_device *pdev, pm_message_t state)
{
	DBG("wmt_udc_suspend()\n");


//    spin_unlock(&udc->lock);
    if (udc->driver){
//    		ppudc = udc;
				run_script(7); 
//        udc->driver->disconnect(&udc->gadget);                    
    }
    gadget_connect=0;                               
    run_script(2);
//    spin_lock(&udc->lock);



	pDevReg->CommandStatus &= 0x1F;

	pUSBMiscControlRegister5 = (unsigned char *)(USB_UDC_REG_BASE + 0x1A0);
	/**pUSBMiscControlRegister5 = 0x02;// USB PHY in power down  & USB in reset*/
	*pUSBMiscControlRegister5 = 0x00;/*USB in reset*/

	TestMode = 0;
	pDevReg->FunctionPatchEn &= ~0x20;

/*	if ((state == 3) && (level == 3))*/
/*		*(volatile unsigned int *)(PM_CTRL_BASE_ADDR + 0x254) &= ~0x00000080;*/   /*DPM needed*/
	*(volatile unsigned char*)(USB_IP_BASE + 0x249) |= 0x04;

	return 0;

} /*wmt_udc_suspend()*/

static int wmt_udc_resume(struct platform_device *pdev)
{
	DBG("wmt_udc_resume()\n");
#if 0	
/*	if (!((*(volatile unsigned int *)(PM_CTRL_BASE_ADDR + 0x254))&0x00000080))*/
/*		*(volatile unsigned int *)(PM_CTRL_BASE_ADDR + 0x254) |= 0x00000080;*/      /*DPM needed*/
	if ((*(volatile unsigned char *)(USB_IP_BASE + 0x249))&0x04) {
		*(volatile unsigned char*)(USB_IP_BASE + 0x249)&= ~0x04;
		mdelay(1);
	}

	pUSBMiscControlRegister5 = (unsigned char *)(USB_UDC_REG_BASE + 0x1A0);
	*pUSBMiscControlRegister5 = 0x01;/*USB in normal operation*/
	/* reset Bulk descriptor control*/
	pDevReg->Bulk1EpControl = 0; /* stop the bulk DMA*/
	while (pDevReg->Bulk1EpControl & EP_ACTIVE) /* wait the DMA stopped*/
		;

	pDevReg->Bulk2EpControl = 0; /* stop the bulk DMA*/
	while (pDevReg->Bulk2EpControl & EP_ACTIVE) /* wait the DMA stopped*/
		;

	pDevReg->Bulk1DesStatus = 0x00;
	pDevReg->Bulk2DesStatus = 0x00;

	pDevReg->Bulk1DesTbytes0 = 0;
	pDevReg->Bulk1DesTbytes1 = 0;
	pDevReg->Bulk1DesTbytes2 = 0;

	pDevReg->Bulk2DesTbytes0 = 0;
	pDevReg->Bulk2DesTbytes1 = 0;
	pDevReg->Bulk2DesTbytes2 = 0;

	/* enable DMA and run the control endpoint*/
	pDevReg->ControlEpControl = EP_RUN + EP_ENABLEDMA;
	/* enable DMA and run the bulk endpoint*/
	pDevReg->Bulk1EpControl = EP_RUN + EP_ENABLEDMA;
	pDevReg->Bulk2EpControl = EP_RUN + EP_ENABLEDMA;
	pDevReg->InterruptEpControl = EP_RUN + EP_ENABLEDMA;
	/* enable DMA and run the interrupt endpoint*/
	/* UsbControlRegister.InterruptEpControl = EP_RUN+EP_ENABLEDMA;*/
	/* run the USB controller*/
	pDevReg->MiscControl3 = 0x3d;
	pDevReg->PortControl |= PORTCTRL_SELFPOWER;/* Device port control register - 22*/
	pDevReg->CommandStatus = USBREG_RUNCONTROLLER;

	ControlState = CONTROLSTATE_SETUP;
	USBState = USBSTATE_DEFAULT;
	TestMode = 0;

	/*status = wmt_udc_setup(odev, xceiv);*/
	wmt_ep_setup_csr("ep0", 0, USB_ENDPOINT_XFER_CONTROL, 64);
	wmt_ep_setup_csr("ep1in" "-bulk", (USB_DIR_IN | 1), USB_ENDPOINT_XFER_BULK, 512);
	wmt_ep_setup_csr("ep2out" "-bulk", (USB_DIR_OUT | 2), USB_ENDPOINT_XFER_BULK, 512);
	wmt_ep_setup_csr("ep3in""-int", (USB_DIR_IN | 3), USB_ENDPOINT_XFER_INT, 8);

	pDevReg->SelfPowerConnect |= 0x10;//Neil

	/* enable all interrupt*/
#ifdef FULL_SPEED_ONLY	
	pDevReg->IntEnable = (INTENABLE_ALL | INTENABLE_FULLSPEEDONLY) ;/*0x70*/
#else
	pDevReg->IntEnable = INTENABLE_ALL;/*0x70*/
#endif	

	/* set IOC on the Setup decscriptor to accept the Setup request*/
	pDevReg->ControlDesControl = CTRLXFER_IOC;
/*Neil_080731*/
/* pullup_disable here and enable in /arch/arm/kernel/apm.c:395
	apm_ioctl() to patch issue signal fail when resume when recive*/
/* set_configuration time out.*/
/*    pullup_enable(udc);//usb_gadget_register_driver()*/
	wmt_pdma_reset();
	pullup_enable(udc);
	pDevReg->FunctionPatchEn |= 0x20; /* HW attach process evaluation enable bit*/
/*	pullup_disable(udc);*/
/*Neil_080731*/
#else
reset_udc();
#endif
	return wmt_wakeup(&udc->gadget);
} /*wmt_udc_resume()*/

/*-------------------------------------------------------------------------*/

static struct platform_driver udc_driver = {
	.driver.name       = (char *) driver_name,
	.probe     	= wmt_udc_probe,
	.remove   	= __exit_p(wmt_udc_remove),
	.suspend 	= wmt_udc_suspend,
	.resume   	= wmt_udc_resume,
};

static struct resource wmt_udc_resources[] = {
	[0] = {
		.start  = (USB_IP_BASE + 0x2000),
		.end    = (USB_IP_BASE + 0x2400),
		.flags  = IORESOURCE_MEM,
	},
};
static u64 wmt_udc_dma_mask = 0xFFFFF000;

static struct platform_device wmt_udc_device = {
	.name           = (char *) driver_name,
	.id             = 0,
	.dev            = {
		.dma_mask = &wmt_udc_dma_mask,
		.coherent_dma_mask = ~0,
	},
	.num_resources  = ARRAY_SIZE(wmt_udc_resources),
	.resource       = wmt_udc_resources,
};
static int __init udc_init(void)
{
	INFO("%s, version: " DRIVER_VERSION
		"%s\n", driver_desc,
		use_dma ?  " (dma)" : "");

	DBG("udc_init()\n");

INIT_WORK(&mount_thread, run_mount);
INIT_WORK(&umount_thread, run_umount);
INIT_WORK(&reinsmod_thread, run_reinsmod);
//INIT_WORK(&online_thread, run_online);
INIT_WORK(&offline_thread, run_offline);
INIT_WORK(&done_thread, run_done);

platform_device_register(&wmt_udc_device);
	return platform_driver_register(&udc_driver);
} /*udc_init()*/

module_init(udc_init);

static void __exit udc_exit(void)
{
    DBG("udc_exit()\n");
	//driver_unregister(&udc_driver);
	platform_driver_unregister(&udc_driver); 
	platform_device_unregister(&wmt_udc_device);    
} /*udc_exit()*/

module_exit(udc_exit);

MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");
