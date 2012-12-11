/*++ 
 * linux/drivers/video/wmt/dev-cec.c
 * WonderMedia video post processor (VPP) driver
 *
 * Copyright c 2011  WonderMedia  Technologies, Inc.
 *
 * This program is free software: you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation, either version 2 of the License, or 
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License for more details. 
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * WonderMedia Technologies, Inc.
 * 4F, 533, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C
--*/

#define DEV_CEC_C

// #define DEBUG
#include <linux/platform_device.h>
#include <linux/major.h>
#include <linux/cdev.h>
#include <linux/netlink.h>
#include <net/sock.h>
#include "cec.h"

#define DRIVER_NAME "wmt-cec"

#define TXNORSP 0x8000 //Tx no response interrupt
#define TXLSARB 0x2000 //Tx lose arbitration
#define TXDONE 	0x0100 //Tx done
#define RXFAIL 	0x0002 //Rx fail, details in Rx10
#define RXDONE 	0x0001 //Rx done

#define MAX_RETRY 3
#define MAX_TIMEOUT 5000

static int tx_state;

static struct cdev *wmt_cec_cdev = NULL;
static DECLARE_MUTEX(wmt_cec_sem);

struct wmt_cec_msg recv_msg;
static void wmt_cec_do_rx_notify(struct work_struct *ptr)
{
	vpp_netlink_notify(USER_PID,DEVICE_RX_DATA,(int)&recv_msg);
}
DECLARE_DELAYED_WORK(wmt_cec_rx_work, wmt_cec_do_rx_notify);


/* receive message interrupt handling */
static void wmt_cec_do_recv(void)
{
	memset(&recv_msg,0,sizeof(recv_msg));
	/* get received data */
	
	recv_msg.msglen = wmt_cec_rx_data(recv_msg.msgdata);
	if(recv_msg.msglen > MAX_MSG_BYTE) 
		recv_msg.msglen = MAX_MSG_BYTE;

	DBGMSG("read a received byte! msglen: %d\n",recv_msg.msglen);

	/* clear receive blockage */

	/* notify AP the received message, let AP decide what to response */
	schedule_delayed_work(&wmt_cec_rx_work, HZ/10);
}

/* check if logic address is valid */
static int bvalidaddr(char addr)
{
	if(addr>15) return 0;
	return 1;
}

/* make sure cec line is not busy */
static int tx_get_cecline(void)
{
	int timeout = 400;

	while (timeout-- > 0) {
		// if not busy
		if( 1 )
			return 0;
		msleep(1);
	}
	return -ETIMEDOUT;
}

/* transfer a time*/
DECLARE_COMPLETION(txcomp);
static int wmt_cec_do_xfer_one(char *msgdata, int msglen)
{
	int ret;
	unsigned long  jiffies;

	ret = tx_get_cecline();
	if (ret != 0) {
		ret = -EAGAIN;
	}

	wmt_cec_enable_int(0,1);	// enable tx done int
	wmt_cec_tx_data(msgdata,msglen);

	jiffies = msecs_to_jiffies((unsigned int)MAX_TIMEOUT);
	jiffies = wait_for_completion_timeout(&txcomp, jiffies);
	wmt_cec_enable_int(0,0);	// disable tx done int
	if(jiffies == 0){
     	return -EAGAIN;
	}

	if(tx_state == TXLSARB)
		return -EAGAIN;

	if( tx_state == TXNORSP)
		return -EAGAIN;

	return tx_state;
}

/* transfer a message, including retransmission if needed*/
static int wmt_cec_do_xfer(char *msgdata, int msglen)
{
	int retry;
	int ret;
	char srcaddr, tgtaddr;

	if(msglen<1){
		DPRINT("[CEC] viacec_xfer: invalid message, msglen is less than 1.\n");
		return -1;
	}

	srcaddr = (msgdata[0] & 0xf0)>>4;
	tgtaddr = (msgdata[0] & 0x0f);
	if(!bvalidaddr(srcaddr) || !bvalidaddr(tgtaddr)){
		DPRINT("[CEC] viacec_doxfer: invalid logic address in msg data.\n");
		return -1;
	}

	for (retry = 0; retry < MAX_RETRY; retry++) {
		ret = wmt_cec_do_xfer_one(msgdata,msglen);
		if (ret != -EAGAIN){		
			goto out;
		}
		DPRINT("[CEC] Retrying transmission (%d)\n", retry);
		udelay(100);
	}

	return -EAGAIN;
	
out:	
	
	/* if polling message ret is no-response, set logical address register, and enable slave mode */
	if(srcaddr == tgtaddr && msglen==1){
		if(ret==TXNORSP){
			DBGMSG("[CEC] cec logic address register is 0x%x\n",tgtaddr);
			return 0;
		}
		else return -1;
	}
	else if(ret == TXDONE)
		return 0;
	else return -1;
}

/* irq handling function */
irqreturn_t wmt_cec_do_irq(int irq, void *dev_id)
{
	int sts;

	sts = wmt_cec_get_int();
	wmt_cec_clr_int(sts);
	if( sts & BIT0 ){ /* tx done */
		DBGMSG("[CEC] write ok int\n");
		complete(&txcomp);
		tx_state = TXDONE;
	}
	if( sts & BIT1 ){ /* rx done */
		DBGMSG("[CEC] read ok int\n");
		wmt_cec_do_recv();
	}
	if( sts & BIT2 ){ /* rx error */
		DBGMSG("[CEC] read error int\n");
	}
	if( sts & BIT3 ){ /* tx arb fail */
		DBGMSG("[CEC] wr arb fail int\n");
		complete(&txcomp);
		tx_state = TXLSARB;
	}
	if( sts & BIT4 ){ /* tx no ack */
//		DBGMSG("[CEC] write no ack int(addr not match)\n");
		complete(&txcomp);
		tx_state = TXNORSP;
	}
	return IRQ_HANDLED;
}

static int wmt_cec_open(
    struct inode *inode, 
    struct file *filp
)
{
    int ret = 0;

	DBGMSG("[CEC] wmt_cec_open\n");

    down(&wmt_cec_sem);
	wmt_cec_rx_enable(1);
    up(&wmt_cec_sem);
    return ret;
} /* End of videodecoder_open() */

static int wmt_cec_release(
    struct inode *inode, 
    struct file *filp
)
{
    int ret = 0;

	DBGMSG("[CEC] wmt_cec_release\n");

    down(&wmt_cec_sem);
	wmt_cec_rx_enable(0);
    up(&wmt_cec_sem);
    return ret;
} /* End of videodecoder_release() */

static int wmt_cec_ioctl(
    struct inode *inode, 
    struct file *filp, 
    unsigned int cmd, 
    unsigned long arg
)
{
    int ret = -EINVAL;

	DBGMSG("[CEC] wmt_cec_ioctl 0x%x,0x%x\n",cmd,arg);

    /* check type and number, if fail return ENOTTY */
    if( _IOC_TYPE(cmd) != WMT_CEC_IOC_MAGIC )   return -ENOTTY;
    if( _IOC_NR(cmd) > WMT_CEC_IOC_MAXNR )   return -ENOTTY;

    /* check argument area */
    if( _IOC_DIR(cmd) & _IOC_READ )
        ret = !access_ok( VERIFY_WRITE, (void __user *) arg, _IOC_SIZE(cmd));
    else if ( _IOC_DIR(cmd) & _IOC_WRITE )
        ret = !access_ok( VERIFY_READ, (void __user *) arg, _IOC_SIZE(cmd));
	else 
		ret = 0;

    if( ret ) return -EFAULT;

    down(&wmt_cec_sem);
	switch(cmd){
		case CECIO_TX_DATA:
			{
				struct wmt_cec_msg msg;

				ret = copy_from_user( (void *) &msg, (const void *)arg, sizeof(struct wmt_cec_msg));
				wmt_cec_do_xfer(msg.msgdata,msg.msglen);
			}
			break;
		case CECIO_TX_LOGADDR:
			wmt_cec_set_logical_addr((arg & 0xFF00)>>8,arg & 0xFF,1);
			break;
		case CECIO_RX_PHYADDR:
			{
				wmt_phy_addr_t parm;

				parm.phy_addr = edid_get_hdmi_phy_addr();
				ret = copy_to_user((void *)arg,(void *)&parm,sizeof(wmt_phy_addr_t));
			}
			break;
		default:
			DBGMSG("[CEC] *W* wmt_cec_ioctl cmd 0x%x\n",cmd);
			break;
	}
    up(&wmt_cec_sem);
    return ret;
} /* End of videodecoder_ioctl() */

static int wmt_cec_mmap(
    struct file *filp, 
    struct vm_area_struct *vma
)
{
    int ret = -EINVAL;
    down(&wmt_cec_sem);
    up(&wmt_cec_sem);
    return ret;
}

static ssize_t wmt_cec_read(
    struct file *filp,
    char __user *buf,
    size_t count,
    loff_t *f_pos
)
{
    ssize_t ret = 0;
    down(&wmt_cec_sem);
    up(&wmt_cec_sem);
    return ret;
} /* videodecoder_read */

static ssize_t wmt_cec_write(
    struct file *filp,
    const char __user *buf,
    size_t count,
    loff_t *f_pos
)
{
    ssize_t ret = 0;
    down(&wmt_cec_sem);
    up(&wmt_cec_sem);
    return ret;
} /* End of videodecoder_write() */

struct file_operations wmt_cec_fops = {
    .owner          = THIS_MODULE,
    .open           = wmt_cec_open,
    .release        = wmt_cec_release,
    .read           = wmt_cec_read,
    .write          = wmt_cec_write,
    .ioctl          = wmt_cec_ioctl,
    .mmap           = wmt_cec_mmap,
};

/*!*************************************************************************
* wmt_cec_probe()
* 
* Private Function by Sam Shen, 2011/03/30
*/
/*!
* \brief
*
* \retval  0 if success
*/ 
static int __init wmt_cec_probe
(
	struct platform_device *dev /*!<; // a pointer point to struct device */
)
{
    dev_t dev_no;
	int ret;

	DBGMSG("[CEC] Enter wmt_cec_probe\n");

	wmt_cec_init_hw();
//	wmt_cec_set_logical_addr(0,0x4,1);
	wmt_cec_set_logical_addr(4,0xf,1);	// default set boardcast address
	wmt_cec_rx_enable(0);
//	wmt_cec_enable_loopback(1);

    dev_no = MKDEV(CEC_MAJOR, 0);
    ret = register_chrdev_region(dev_no,1,"wmtcec");
    if( ret < 0 ){
        printk("can't get %s device major %d\n",DRIVER_NAME, CEC_MAJOR);
        return ret;
    }

    /* register char device */
    wmt_cec_cdev = cdev_alloc();
    if(!wmt_cec_cdev){
        DPRINT("alloc dev error.\n");
        return -ENOMEM;
    }

    cdev_init(wmt_cec_cdev,&wmt_cec_fops);
    ret = cdev_add(wmt_cec_cdev,dev_no,1);
    if(ret){
        DPRINT("reg char dev error(%d).\n",ret);
        cdev_del(wmt_cec_cdev);
        return ret;
    }

	if ( vpp_request_irq(IRQ_VPP_IRQ20, wmt_cec_do_irq, SA_INTERRUPT, "cec", (void *) 0) ) {
		DPRINT("*E* request CEC ISR fail\n");
	}
	vppif_reg32_out(REG_CEC_INT_ENABLE,0x1f);

	DBGMSG("[CEC] Exit wmt_cec_probe(0x%x)\n",dev_no);
	return 0;
} /* End of wmt_cec_probe */

/*!*************************************************************************
* wmt_cec_remove()
* 
* Private Function by Sam Shen, 2011/03/30
*/
/*!
* \brief
*
* \retval  0 if success
*/ 
static int wmt_cec_remove
(
	struct platform_device *dev /*!<; // a pointer point to struct device */
)
{
	return 0;
} /* End of wmt_cec_remove */

#ifdef CONFIG_PM
/*!*************************************************************************
* wmt_cec_suspend()
* 
* Private Function by Sam Shen, 2011/03/30
*/
/*!
* \brief
*
* \retval  0 if success
*/ 
static int wmt_cec_suspend
(
	struct platform_device *pDev,     /*!<; // a pointer point to struct device */
	pm_message_t state				/*!<; // suspend state */
)
{
	DPRINT("Enter wmt_cec_suspend\n");
	wmt_cec_do_suspend();
	return 0;
} /* End of wmt_cec_suspend */

/*!*************************************************************************
* wmt_cec_resume()
* 
* Private Function by Sam Shen, 2011/03/30
*/
/*!
* \brief
*
* \retval  0 if success
*/ 
static int wmt_cec_resume
(
	struct platform_device *pDev 	/*!<; // a pointer point to struct device */
)
{
	DPRINT("Enter wmt_cec_resume\n");
	wmt_cec_do_resume();
	return 0;
} /* End of wmt_cec_resume */
#else
#define wmt_cec_suspend NULL
#define wmt_cec_resume NULL
#endif

/***************************************************************************
	device driver struct define
****************************************************************************/
static struct platform_driver wmt_cec_driver = {
	.driver.name    = "wmtcec", // This name should equal to platform device name.
	.driver.bus     = &platform_bus_type,
	.probe          = wmt_cec_probe,
	.remove         = wmt_cec_remove,
	.suspend        = wmt_cec_suspend,
	.resume         = wmt_cec_resume,
};

/***************************************************************************
	platform device struct define
****************************************************************************/
static u64 wmt_cec_dma_mask = 0xffffffffUL;
static struct platform_device wmt_cec_device = {
	.name   = "wmtcec",
	.dev    = {
		.dma_mask = &wmt_cec_dma_mask,
		.coherent_dma_mask = ~0,
	},

#if 0
	.id     = 0,
	.dev    = {
		.release = wmt_cec_platform_release,
	},
	.num_resources  = 0,    /* ARRAY_SIZE(wmt_cec_resources), */
	.resource       = NULL, /* wmt_cec_resources, */
#endif
};

static int __init wmt_cec_init(void)
{
	int ret;

	DBGMSG(KERN_ALERT "Enter wmt_cec_init\n");
	
	ret = platform_driver_register(&wmt_cec_driver);
	if (!ret) {
		ret = platform_device_register(&wmt_cec_device);
		if (ret)
			platform_driver_unregister(&wmt_cec_driver);
	}
	return ret;
}

static void __exit wmt_cec_exit(void)
{
	dev_t dev_no;
	
	DBGMSG(KERN_ALERT "Enter wmt_cec_exit\n");

	platform_driver_unregister(&wmt_cec_driver);
	platform_device_unregister(&wmt_cec_device);
    dev_no = MKDEV(CEC_MAJOR, 0);
    unregister_chrdev_region(dev_no,1);
	return;
}

module_init(wmt_cec_init);
module_exit(wmt_cec_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("WMT CEC driver");
MODULE_AUTHOR("WMT TECH");
MODULE_VERSION("1.0.0");

