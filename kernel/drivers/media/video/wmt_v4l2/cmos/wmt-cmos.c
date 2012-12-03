/*++ 
 * linux/drivers/media/video/wmt_v4l2/cmos/wmt-cmos.c
 * WonderMedia v4l cmos device driver
 *
 * Copyright c 2010  WonderMedia  Technologies, Inc.
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


#define WMT_CMOS_C

#include <linux/interrupt.h>
#include <linux/major.h>
#include <linux/platform_device.h>//platform_bus_type

#include <linux/i2c.h>      //I2C_M_RD
#include <linux/wmt-mb.h>

/* V4L2 */
#include <linux/videodev2.h>
#include <linux/dma-mapping.h>
#ifdef CONFIG_VIDEO_V4L1_COMPAT
#include <linux/videodev.h>
#endif

#include <media/v4l2-common.h>
#include <linux/module.h>

#include "wmt-cmos.h"
#include "cmos-dev.h"


//#define CMOS_REG_TRACE
#ifdef CMOS_REG_TRACE
#define CMOS_REG_SET32(addr, val)  \
        PRINT("REG_SET:0x%x -> 0x%0x\n", addr, val);\
        REG32_VAL(addr) = (val)
#else
#define CMOS_REG_SET32(addr, val)      REG32_VAL(addr) = (val)
#endif


//#define CMOS_DEBUG    /* Flag to enable debug message */
//#define CMOS_TRACE

#ifdef CMOS_DEBUG
  #define DBG_MSG(fmt, args...)    printk("{%s} " fmt, __FUNCTION__ , ## args)
#else
  #define DBG_MSG(fmt, args...)
#endif


#ifdef CMOS_TRACE
  #define TRACE(fmt, args...)      printk("{%s}:  " fmt, __FUNCTION__ , ## args)
#else
  #define TRACE(fmt, args...) 
#endif

#define DBG_ERR(fmt, args...)      printk("*E* {%s} " fmt, __FUNCTION__ , ## args)


#define THE_MB_USER              "CMOS-MB"

#define ALIGN64(a)               (((a)+63) & (~63))

extern int wmt_getsyspara(char *varname, unsigned char *varval, int *varlen);

struct cmos_dev_s {
	/* module parameters */
    cmos_drvinfo_t  drvinfo;
	char *buf;

	/* char dev struct */
	struct cdev cdev;
};

#ifdef __KERNEL__
DECLARE_WAIT_QUEUE_HEAD(cmos_wait);
#endif

static spinlock_t cmos_lock;
static int cmos_dev_ref = 0; /* is device open */
struct cmos_dev_s dev_s;


static int cmos_dev_major = VID_MAJOR;
static int cmos_dev_minor = 1;
static int cmos_dev_nr = 1;
struct file *gfilp;






cmos_uboot_env_t cmos_env;


int get_cmos_uboot_env(void)
{
   	char varbuf2[128];
	int n;
	int varlen;
	
		
	//get the wmt.gpo.cmos
	memset(varbuf2, 0, sizeof(varbuf2));
	varlen = sizeof(varbuf2);


    	if (wmt_getsyspara("wmt.gpo.cmos", varbuf2, &varlen)) {
		printk("Can't get cmos config in u-boot!!!!\n");
		cmos_env.en=0;
		return -1;
	} else {
		n = sscanf(varbuf2, "%d:%d:%d:%08x:%08x:%08x:%d:%d:%08x:%08x:%08x",
					&cmos_env.dev_tot_num,
					&cmos_env.dev1_pwr_active_level,
					&cmos_env.dev1_pwr_bitnum,
					&cmos_env.dev1_pwr_reg_gpio_en,
					&cmos_env.dev1_pwr_reg_gpio_od,
					&cmos_env.dev1_pwr_reg_gpio_oc,
					
					&cmos_env.dev2_pwr_active_level,
					&cmos_env.dev2_pwr_bitnum,
					&cmos_env.dev2_pwr_reg_gpio_en,
					&cmos_env.dev2_pwr_reg_gpio_od,
					&cmos_env.dev2_pwr_reg_gpio_oc			
		);
		
		if (n <= 5) 
		{
			printk("cmos format is error in u-boot!!!\n");
			cmos_env.en=0;
		}else{
		
		cmos_env.en=1;
			cmos_env.dev1_pwr_reg_gpio_en =  GPIO_BASE_ADDR + (cmos_env.dev1_pwr_reg_gpio_en & 0xfff);
			cmos_env.dev1_pwr_reg_gpio_od = GPIO_BASE_ADDR + (cmos_env.dev1_pwr_reg_gpio_od & 0xfff);
			cmos_env.dev1_pwr_reg_gpio_oc = GPIO_BASE_ADDR + (cmos_env.dev1_pwr_reg_gpio_oc & 0xfff);

			cmos_env.dev2_pwr_reg_gpio_en =  GPIO_BASE_ADDR + (cmos_env.dev2_pwr_reg_gpio_en & 0xfff);
			cmos_env.dev2_pwr_reg_gpio_od = GPIO_BASE_ADDR + (cmos_env.dev2_pwr_reg_gpio_od & 0xfff);
			cmos_env.dev2_pwr_reg_gpio_oc = GPIO_BASE_ADDR + (cmos_env.dev2_pwr_reg_gpio_oc & 0xfff);
		}
	}

	//get the wmt.camera.param
	memset(varbuf2, 0, sizeof(varbuf2));
	varlen = sizeof(varbuf2);

    	if (wmt_getsyspara("wmt.camera.param", varbuf2, &varlen)) {
		printk("Can't get wmt.camera.param config in u-boot!!!!\n");
		cmos_env.v_flip = 0;
		cmos_env.h_flip = 1;

	} else {
		n = sscanf(varbuf2, "%d:%d:%d",
					&cmos_env.dummy,
					&cmos_env.v_flip,
					&cmos_env.h_flip
		);
		if (n != 3) {
			printk("get wmt.camera.param error !!!\n");
			cmos_env.v_flip = 0;
			cmos_env.h_flip = 1;

		}

	}

	//get the gpio i2c setting 
	memset(varbuf2, 0, sizeof(varbuf2));
	varlen = sizeof(varbuf2);

    	if (wmt_getsyspara("wmt.camera.i2c_gpio", varbuf2, &varlen)) {
		printk("Can't get wmt.camera.i2c_gpio config in u-boot!!!!\n");
		cmos_env.i2c_gpio_en = 0;

	} else {
		n = sscanf(varbuf2, "%d:%d:%d:%08x:%08x:%08x:%08x:%08x:%d:%d:%08x:%08x:%08x:%08x:%08x",
					&cmos_env.i2c_gpio_en,
					
					&cmos_env.i2c_gpio_scl_binum,
					&cmos_env.reg_i2c_gpio_scl_gpio_pe_bitnum,
					&cmos_env.reg_i2c_gpio_scl_gpio_in,
					&cmos_env.reg_i2c_gpio_scl_gpio_en,
					&cmos_env.reg_i2c_gpio_scl_gpio_od,
					&cmos_env.reg_i2c_gpio_scl_gpio_oc,
					&cmos_env.reg_i2c_gpio_scl_gpio_pe,
					
					&cmos_env.i2c_gpio_sda_binum,
					&cmos_env.reg_i2c_gpio_sda_gpio_pe_bitnum,
					&cmos_env.reg_i2c_gpio_sda_gpio_in,
					&cmos_env.reg_i2c_gpio_sda_gpio_en,
					&cmos_env.reg_i2c_gpio_sda_gpio_od,
					&cmos_env.reg_i2c_gpio_sda_gpio_oc,
					&cmos_env.reg_i2c_gpio_sda_gpio_pe

		);
		if (n != 15) 
		{
			printk("Don't get wmt.camera.i2c_gpio \n");
			cmos_env.i2c_gpio_en = 0;

		}else{
		cmos_env.reg_i2c_gpio_scl_gpio_in = GPIO_BASE_ADDR + (cmos_env.reg_i2c_gpio_scl_gpio_in &0xfff);
		cmos_env.reg_i2c_gpio_scl_gpio_en = GPIO_BASE_ADDR + (cmos_env.reg_i2c_gpio_scl_gpio_en &0xfff);
		cmos_env.reg_i2c_gpio_scl_gpio_od = GPIO_BASE_ADDR + (cmos_env.reg_i2c_gpio_scl_gpio_od &0xfff);
		cmos_env.reg_i2c_gpio_scl_gpio_oc = GPIO_BASE_ADDR + (cmos_env.reg_i2c_gpio_scl_gpio_oc &0xfff);
		cmos_env.reg_i2c_gpio_scl_gpio_pe = GPIO_BASE_ADDR + (cmos_env.reg_i2c_gpio_scl_gpio_pe &0xfff);


		cmos_env.reg_i2c_gpio_sda_gpio_in = GPIO_BASE_ADDR + (cmos_env.reg_i2c_gpio_sda_gpio_in &0xfff);
		cmos_env.reg_i2c_gpio_sda_gpio_en = GPIO_BASE_ADDR + (cmos_env.reg_i2c_gpio_sda_gpio_en &0xfff);
		cmos_env.reg_i2c_gpio_sda_gpio_od = GPIO_BASE_ADDR + (cmos_env.reg_i2c_gpio_sda_gpio_od &0xfff);
		cmos_env.reg_i2c_gpio_sda_gpio_oc = GPIO_BASE_ADDR + (cmos_env.reg_i2c_gpio_sda_gpio_oc &0xfff);
		cmos_env.reg_i2c_gpio_sda_gpio_pe = GPIO_BASE_ADDR + (cmos_env.reg_i2c_gpio_sda_gpio_pe &0xfff);
		}

	}


		printk("CMOS UBOOT ARG\n");
		printk("get the cmos param  setting wmt.camera.i2c_gpio : %d:%d:%d:0x%08x:0x%08x:0x%08x:0x%08x:0x%08x:%d:%d:0x%08x:0x%08x:0x%08x:0x%08x:0x%08x\n",
					cmos_env.i2c_gpio_en,				
					cmos_env.i2c_gpio_scl_binum,
					cmos_env.reg_i2c_gpio_scl_gpio_pe_bitnum,
					cmos_env.reg_i2c_gpio_scl_gpio_in,
					cmos_env.reg_i2c_gpio_scl_gpio_en,
					cmos_env.reg_i2c_gpio_scl_gpio_od,
					cmos_env.reg_i2c_gpio_scl_gpio_oc,
					cmos_env.reg_i2c_gpio_scl_gpio_pe,
					
					cmos_env.i2c_gpio_sda_binum,
					cmos_env.reg_i2c_gpio_sda_gpio_pe_bitnum,
					cmos_env.reg_i2c_gpio_sda_gpio_in,
					cmos_env.reg_i2c_gpio_sda_gpio_en,
					cmos_env.reg_i2c_gpio_sda_gpio_od,
					cmos_env.reg_i2c_gpio_sda_gpio_oc,
					cmos_env.reg_i2c_gpio_sda_gpio_pe

		);

		printk("get the cmos gpio setting : %d:%d:%d:0x%08x:0x%08x:0x%08x:%d:%d:0x%08x:0x%08x:0x%08x\n",
					cmos_env.dev_tot_num,
					cmos_env.dev1_pwr_active_level,
					cmos_env.dev1_pwr_bitnum,
					cmos_env.dev1_pwr_reg_gpio_en,
					cmos_env.dev1_pwr_reg_gpio_od,
					cmos_env.dev1_pwr_reg_gpio_oc,
					
					cmos_env.dev2_pwr_active_level,
					cmos_env.dev2_pwr_bitnum,
					cmos_env.dev2_pwr_reg_gpio_en,
					cmos_env.dev2_pwr_reg_gpio_od,
					cmos_env.dev2_pwr_reg_gpio_oc			
		);
		printk("get the cmos param  setting : %d:%d:%d\n",
					cmos_env.dummy,
					cmos_env.v_flip,
					cmos_env.h_flip
		);
	return 0;
}



void power_ctrl(bool en , unsigned int active_level, unsigned int in_bitnum,unsigned int reg_gpio_en ,  unsigned int reg_gpio_od,  unsigned int reg_gpio_oc)
{
 	unsigned int bitnum ;
 	bool level;
	bitnum = 1 << in_bitnum ;
	//	unsigned int reggpioen,reggpiood,reggpiooc;
      printk("  bitnum 0x%08x 0x%08x 0x%08x 0x%08x \n",bitnum, reg_gpio_en, reg_gpio_od, reg_gpio_oc );

	REG8_VAL(reg_gpio_en) |= bitnum;
	REG8_VAL(reg_gpio_od) |= bitnum;


	if (active_level)
		level =en;
	else
		level =!en;
	
	if (level)
	{
		printk("set cmos gpio to high \n");
		REG8_VAL(reg_gpio_oc) |= bitnum;//high , turn off  the CMOS power
	}else{
		printk("set cmos gpio to low \n");
		REG8_VAL(reg_gpio_oc) &= ~bitnum;//low , turn on  the CMOS power
	}
	
}


/*-------------------------------Body Functions------------------------------------*/

static int cam_enable(cmos_drvinfo_t *drv, int en)
{
    drv->streamoff = (en ^ 0x1);

    CMOS_REG_SET32( REG_VID_CMOS_EN, en);	/* enable CMOS */
    
    return 0;
} /* End of cam_enable() */

static int print_queue(cmos_drvinfo_t *drv)
{
#ifdef CMOS_DEBUG
    struct list_head *next;
    cmos_fb_t  *fb;
    
    TRACE("Enter\n");

    list_for_each(next, &drv->head) {
        fb = (cmos_fb_t *) list_entry(next, cmos_fb_t, list);
        DBG_MSG("[%d] y_addr: 0x%x, c_addr: 0x%x\n",
                            fb->id, fb->y_addr, fb->c_addr );
    }
    TRACE("Leave\n");
#endif
    return 0;
} /* End of print_queue();*/

static int put_queue(cmos_drvinfo_t *drv, cmos_fb_t *fb_in)
{
    TRACE("Enter\n");

    list_add_tail(&fb_in->list, &drv->head);

    print_queue(drv);

    TRACE("Leave\n");

    return 0;
} /* End of put_queue() */

static int pop_queue(cmos_drvinfo_t *drv, cmos_fb_t *fb_in)
{
    TRACE("Enter\n");

    list_del(&fb_in->list);
    print_queue(drv);
    
    TRACE("Leave\n");

    return 0;
} /* End of pop_queue() */

/*------------------------------------------------------------------------------
    Export V4L2 functions for CMOS camera 
------------------------------------------------------------------------------*/

int cmos_cam_querycap(struct file *file, void *fh, struct v4l2_capability *cap)
{

    TRACE("Enter\n");

    memset(cap, 0, sizeof(*cap));
    strlcpy(cap->driver, "wm85xx", sizeof(cap->driver));

// FIXME
//    strlcpy(cap->card, em28xx_boards[dev->model].name, sizeof(cap->card));
//    strlcpy(cap->bus_info, dev->udev->dev.bus_id, sizeof(cap->bus_info));
//    strlcpy(cap->card, dev->ext->name, sizeof(cap->card));
//    sprintf(cap->bus_info,"PCI:%s", pci_name(dev->pci));
#ifndef KERNEL_VERSION
#define KERNEL_VERSION(a,b,c) ((a)*65536+(b)*256+(c))
#endif

    cap->version = KERNEL_VERSION(0, 0, 1);
    cap->capabilities =
        V4L2_CAP_VIDEO_CAPTURE |
        V4L2_CAP_READWRITE | 
        V4L2_CAP_STREAMING;
        
    TRACE("Leave\n");

    return 0;
}
EXPORT_SYMBOL(cmos_cam_querycap);


 int cmos_cam_cropcap(struct file *file, void *__fh,
					struct v4l2_cropcap *cropcap)
{
    DBG_ERR("Not support now!\n");
    return 0;
}

EXPORT_SYMBOL(cmos_cam_cropcap);


int cmos_cam_enum_fmt_cap(struct file *file, void *fh, struct v4l2_fmtdesc *f)
{
    DBG_ERR("Not support now!\n");
	return 0;
}
EXPORT_SYMBOL(cmos_cam_enum_fmt_cap);

int cmos_cam_g_fmt_cap(struct file *file, void *fh, struct v4l2_format *f)
{
    DBG_ERR("Not support now!\n");
	return 0;
}
EXPORT_SYMBOL(cmos_cam_g_fmt_cap);
//os_cam_s_fmt_cap
int cmos_cam_s_fmt_cap(struct file *file, void *fh, struct v4l2_format *f)
{
    struct cmos_dev_s *dev = (struct cmos_dev_s *)file->private_data;
    cmos_drvinfo_t *drv = &dev->drvinfo;
    int dev_id = f->fmt.pix.priv; // enable which device
    
    int ret;

    TRACE("Enter\n");

	drv->width  = f->fmt.pix.width;
	drv->height = f->fmt.pix.height;
    	drv->frame_size = ALIGN64(drv->width) * drv->height << 1; // "<< 1" is for YC422
   
    DBG_MSG(" width:      %d\n", drv->width);
    DBG_MSG(" height:     %d\n", drv->height);
    DBG_MSG(" frame_size: %d\n", drv->frame_size);

    printk(">> dev_id %d  \n", dev_id);
    if ( cmos_env.dev_tot_num == 1)
    {
		power_ctrl(1, cmos_env.dev1_pwr_active_level, cmos_env.dev1_pwr_bitnum, cmos_env.dev1_pwr_reg_gpio_en, cmos_env.dev1_pwr_reg_gpio_od,cmos_env.dev1_pwr_reg_gpio_oc);
    }else{
    
	    if (dev_id == 0)
	    {
		//note that tuen off one before turn on another
		power_ctrl(0, cmos_env.dev2_pwr_active_level, cmos_env.dev2_pwr_bitnum, cmos_env.dev2_pwr_reg_gpio_en, cmos_env.dev2_pwr_reg_gpio_od,cmos_env.dev2_pwr_reg_gpio_oc);
		power_ctrl(1, cmos_env.dev1_pwr_active_level, cmos_env.dev1_pwr_bitnum, cmos_env.dev1_pwr_reg_gpio_en, cmos_env.dev1_pwr_reg_gpio_od,cmos_env.dev1_pwr_reg_gpio_oc);
	    }else{

		power_ctrl(0, cmos_env.dev1_pwr_active_level, cmos_env.dev1_pwr_bitnum, cmos_env.dev1_pwr_reg_gpio_en, cmos_env.dev1_pwr_reg_gpio_od,cmos_env.dev1_pwr_reg_gpio_oc);
		power_ctrl(1, cmos_env.dev2_pwr_active_level, cmos_env.dev2_pwr_bitnum, cmos_env.dev2_pwr_reg_gpio_en, cmos_env.dev2_pwr_reg_gpio_od,cmos_env.dev2_pwr_reg_gpio_oc);
	    }
    }
    ret = cmos_init_ext_device(drv->width, drv->height, cmos_env.v_flip, cmos_env.h_flip);

    if( ret != 0 ) {
        DBG_ERR("Initial CMOS sensor for %d x %d fail!\n", drv->width, drv->height);
        return -1;
    }
    ret = wmt_vid_set_mode(drv->width, drv->height);
    
    
    TRACE("Leave\n");

    return 0;
}
EXPORT_SYMBOL(cmos_cam_s_fmt_cap);

int cmos_cam_try_fmt_cap(struct file *file, void *fh, struct v4l2_format *f)
{
    return cmos_cam_s_fmt_cap(file, fh, f);
}
EXPORT_SYMBOL(cmos_cam_try_fmt_cap);

int cmos_cam_reqbufs(struct file *file, void *fh, struct v4l2_requestbuffers *rb)
{
    struct cmos_dev_s *dev = (struct cmos_dev_s *)file->private_data;
    cmos_drvinfo_t *drv = &dev->drvinfo;

    cmos_fb_t *fb;
    int  i, j;

    TRACE("Enter\n");

    if((rb->type != V4L2_BUF_TYPE_VIDEO_CAPTURE) || (rb->memory != V4L2_MEMORY_MMAP)
        || (rb->count > MAX_FB_IN_QUEUE) ) {
        printk("rb->type: 0x%x, rb->memory: 0x%x, rb->count: %d\n", 
                                               rb->type, rb->memory, rb->count);
        return -EINVAL;
    }
 
    drv->fb_cnt = rb->count;

    DBG_MSG(" Frame Size: %d Bytes\n", drv->frame_size);
    DBG_MSG(" fb_cnt:     %d\n", drv->fb_cnt);

    /* Allocate memory */
    for(i=0; i<drv->fb_cnt; i++) {
        fb = &drv->fb_pool[i];
        fb->y_addr = mb_alloc(drv->frame_size);
        printk("%d CMOS MB_allocate size %d\n", i,drv->frame_size);
        if( fb->y_addr == 0 ) {
            /* Allocate MB memory size fail */
            DBG_ERR("[%d] >> Allocate MB memory (%d) fail!\n", i, drv->frame_size);
            for(j=0; j<i; j++) {
                fb = &drv->fb_pool[j];
        	    mb_free(fb->y_addr);
            }
            return -EINVAL; 
        }
        fb->c_addr  = fb->y_addr + ALIGN64(drv->width) * drv->height;
        fb->id = i;
        fb->is_busy = 0;
        fb->done    = 0;
        DBG_MSG("[%d] fb: 0x%x, y_addr: 0x%x, c_addr: 0x%x\n", 
                   i, (unsigned int)fb, fb->y_addr, fb->c_addr);
    }
    TRACE("Leave\n");

	return 0;
}
EXPORT_SYMBOL(cmos_cam_reqbufs);

int cmos_cam_querybuf(struct file *file, void *fh, struct v4l2_buffer *b)
{
    struct cmos_dev_s *dev = (struct cmos_dev_s *)file->private_data;
    cmos_drvinfo_t *drv = &dev->drvinfo;

    TRACE("Enter (index: %d)\n", b->index);

    if( b->index < MAX_FB_IN_QUEUE ) {
        cmos_fb_t *fb = &drv->fb_pool[b->index];
        b->length   = drv->frame_size;
        b->m.offset = fb->y_addr;
    }
    else {
        b->length   = 0;
        b->m.offset = 0;
    }
    DBG_MSG(" b->length:     %d\n", b->length);
    DBG_MSG(" b->m.offset: 0x%x\n", b->m.offset);

    TRACE("Leave (index: %d)\n", b->index);

	return 0;
}
EXPORT_SYMBOL(cmos_cam_querybuf);

int cmos_cam_qbuf(struct file *file, void *fh, struct v4l2_buffer *b)
{
    struct cmos_dev_s *dev = (struct cmos_dev_s *)file->private_data;
    cmos_drvinfo_t *drv = &dev->drvinfo;

    cmos_fb_t *fb;

    TRACE("Enter (index: %d)\n", b->index);
    
    fb = &drv->fb_pool[b->index];
    put_queue(drv, fb);

    TRACE("Leave (index: %d)\n", b->index);

	return 0;
}
EXPORT_SYMBOL(cmos_cam_qbuf);

int cmos_cam_dqbuf(struct file *file, void *fh, struct v4l2_buffer *b)
{
    struct cmos_dev_s *dev = (struct cmos_dev_s *)file->private_data;
    cmos_drvinfo_t    *drv = &dev->drvinfo;
    unsigned long      flags =0;
    cmos_fb_t *fb = 0;

    TRACE("Enter (index: %d)\n", b->index);

    if( drv->streamoff ) {
        struct list_head  *next;

        /* CMOS sensor did not work now */
        list_for_each(next, &drv->head) {
            fb = (cmos_fb_t *) list_entry(next, cmos_fb_t, list);
            if( fb ) {
                pop_queue(drv, fb);    
                break;
            }
        }
        goto EXIT_cmos_cam_dqbuf;
    }
    
    fb = (cmos_fb_t *)wmt_vid_get_cur_fb();
    
    spin_lock_irqsave(&cmos_lock, flags);

    DBG_MSG("Set DQBUF on\n");
    drv->dqbuf  = 1;

    spin_unlock_irqrestore(&cmos_lock, flags);
    
    if( (wait_event_interruptible_timeout( cmos_wait,
                      (drv->_status & STS_CMOS_FB_DONE), drv->_timeout) == 0) ){
        DBG_ERR("CMOS Time out in %d ms\n", drv->_timeout);
    }
    drv->_status &= (~STS_CMOS_FB_DONE);

    DBG_MSG("[%d] fb: 0x%p\n", fb->id, fb);
    pop_queue(drv, fb);    

    b->length   = drv->frame_size;
    b->m.offset = fb->y_addr;
    b->index    = fb->id;

EXIT_cmos_cam_dqbuf:
    TRACE("Leave (index: %d)\n", b->index);

    return 0;
}
EXPORT_SYMBOL(cmos_cam_dqbuf);

int cmos_cam_s_std(struct file *file, void *fh, v4l2_std_id a)
{
    DBG_ERR("Not support now!\n");
	return 0;
}
EXPORT_SYMBOL(cmos_cam_s_std);

int cmos_cam_enum_input(struct file *file, void *fh, struct v4l2_input *inp)
{
    DBG_ERR("Not support now!\n");
	return 0;
}
EXPORT_SYMBOL(cmos_cam_enum_input);

int cmos_cam_g_input(struct file *file, void *fh, unsigned int *i)
{
    DBG_ERR("Not support now!\n");
	return 0;
}
EXPORT_SYMBOL(cmos_cam_g_input);

int cmos_cam_s_input(struct file *file, void *fh, unsigned int i)
{
    DBG_ERR("Not support now!\n");
	return 0;
}
EXPORT_SYMBOL(cmos_cam_s_input);

int cmos_cam_queryctrl(struct file *file, void *fh, struct v4l2_queryctrl *a)
{
    DBG_ERR("Not support now!\n");
	return 0;
}
EXPORT_SYMBOL(cmos_cam_queryctrl);

int cmos_cam_g_ctrl(struct file *file, void *fh, struct v4l2_control *a)
{
    DBG_ERR("Not support now!\n");
	return 0;
}
EXPORT_SYMBOL(cmos_cam_g_ctrl);

int cmos_cam_s_ctrl(struct file *file, void *fh, struct v4l2_control *a)
{
    DBG_ERR("Not support now!\n");
    return 0;
}
EXPORT_SYMBOL(cmos_cam_s_ctrl);

int cmos_cam_streamon(struct file *file, void *fh, enum v4l2_buf_type i)
{
    struct cmos_dev_s *dev = (struct cmos_dev_s *)file->private_data;
    cmos_drvinfo_t *drv = &dev->drvinfo;
    cmos_fb_t  *fb;

    TRACE("Enter\n");

    fb = (cmos_fb_t *) list_entry(drv->head.next, cmos_fb_t, list);

    wmt_vid_set_cur_fb((vid_fb_t *)fb);

    cam_enable(drv, 1);
    
    TRACE("Leave\n");

    return 0;
}
EXPORT_SYMBOL(cmos_cam_streamon);

int cmos_cam_streamoff(struct file *file, void *fh, enum v4l2_buf_type i)
{
    struct cmos_dev_s *dev = (struct cmos_dev_s *)file->private_data;
    cmos_drvinfo_t *drv = &dev->drvinfo;

    TRACE("Enter\n");

    cam_enable(drv, 0);
    
    TRACE("Leave\n");

	return 0;
}
EXPORT_SYMBOL(cmos_cam_streamoff);

int cmos_cam_mmap(struct file *file, struct vm_area_struct *vma)
{
    int ret = 0;
    
    TRACE("Enter\n");

	vma->vm_flags |= VM_IO | VM_RESERVED;
	vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
	if (remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff,
	                    vma->vm_end - vma->vm_start, vma->vm_page_prot)){
		ret = -EAGAIN;
    }
    TRACE("Leave\n");
    
    return ret;
}
EXPORT_SYMBOL(cmos_cam_mmap);

/*!*************************************************************************
* cmos_isr
* 
* Public Function by Willy Chuang, 2010/05/28
*/
/*!
* \brief
*       init CMOS module
* \retval  0 if success
*/ 
#ifdef __KERNEL__
static irqreturn_t cmos_isr(int irq,void *dev_in,struct pt_regs *regs)
#else
static void cmos_isr(void)
#endif
{
    struct cmos_dev_s *dev = (struct cmos_dev_s *)dev_in;
    cmos_drvinfo_t    *drv;
    struct list_head  *next;
    cmos_fb_t *cur_fb, *fb;
    unsigned long flags =0;
    int mb_count=0;   
    TRACE("Enter\n");
    if( dev == 0 ) {        
        return IRQ_NONE;
    }
    drv = &dev->drvinfo;

    if( drv->dqbuf == 1 ) {
        spin_lock_irqsave(&cmos_lock, flags);
        drv->dqbuf = 0;
        DBG_MSG("Set DBBUF off\n");
        spin_unlock_irqrestore(&cmos_lock, flags);

        cur_fb = (cmos_fb_t *)wmt_vid_get_cur_fb();
        list_for_each(next, &drv->head) {
            fb = (cmos_fb_t *) list_entry(next, cmos_fb_t, list);
            if( fb == cur_fb ) {
                /*-------------------------------------------------------------- 
                    Get next FB 
                --------------------------------------------------------------*/
                fb = (cmos_fb_t *) list_entry(cur_fb->list.next, cmos_fb_t, list);

                do{
		     if (((vid_fb_t *)fb)->y_addr != 0)
			  mb_count = mb_counter(((vid_fb_t *)fb)->y_addr);
	                if (mb_count !=1)
	                {
	                	fb = (cmos_fb_t *) list_entry(fb->list.next, cmos_fb_t, list);
				DBG_MSG("mbcount skip 0x%08x mb_count %d \n",((vid_fb_t *)fb)->y_addr,mb_count);
				continue;
	                }
                }while((mb_count !=1)&&(fb != cur_fb));
                fb->is_busy = 0;
                fb->done    = 1;
                DBG_MSG("[%d] done: %d, is_busy: %d\n", cur_fb->id, cur_fb->done, cur_fb->is_busy);
                DBG_MSG("[%d] New FB done: %d, is_busy: %d\n", fb->id, fb->done, fb->is_busy);

                wmt_vid_set_cur_fb((vid_fb_t *)fb); 

                drv->_status |= STS_CMOS_FB_DONE;
                wake_up_interruptible(&cmos_wait);
                break;
            }
        }
    } /* if( drv->dqbuf == 1 ) */
 	CMOS_REG_SET32(REG_VID_INT_CTRL, REG32_VAL(REG_VID_INT_CTRL));

    TRACE("Leave\n");

#ifdef __KERNEL__
    return IRQ_HANDLED;
#endif
} /* End of cmos_isr() */

/*!*************************************************************************
* cmos_open
* 
* Private Function by Max Chen, 2009/1/25
*/
/*!
* \brief
*       init CMOS module
* \retval  0 if success
*/ 
static int cmos_open(struct file *filp)
{
	struct cmos_dev_s *dev=&dev_s;
       cmos_drvinfo_t    *drv ;
	int  ret = 0;

       TRACE("Enter\n");
    /*--------------------------------------------------------------------------
        Step 1:
    --------------------------------------------------------------------------*/
	if (!cmos_env.en)
	{
		printk("please set the cmos uboot env\n");
		printk("setenv wmt.gpo.cmos 1:0:7:D8110040:D8110080:D81100C0\n");
		return -1;
	}
	if(cmos_dev_ref)
		return -EBUSY;
	
	cmos_dev_ref++;
	
	gfilp = filp;
	
	filp->private_data = dev;
	drv = &dev->drvinfo;


  /*--------------------------------------------------------------------------
        Step 2:
    --------------------------------------------------------------------------*/
    	memset(&dev_s, 0, sizeof(cmos_drvinfo_t));
	wmt_vid_open(VID_MODE_CMOS, &cmos_env);

	drv->width  = 640;
	drv->height = 480;
	drv->_timeout = 100;   // ms
	drv->_status  = STS_CMOS_READY;
	drv->dqbuf    = 0;
	drv->frame_size = ALIGN64(drv->width) * drv->height << 1; // "<< 1" is for YC422
	drv->dft_y_addr = mb_alloc(drv->frame_size);
	if( drv->dft_y_addr == 0 ) {
	        DBG_ERR("Allocate MB memory (%d) fail!\n", drv->frame_size);
       	 return -EINVAL; 
       }
	drv->dft_c_addr = drv->dft_y_addr + ALIGN64(drv->width) * drv->height;
	wmt_vid_set_addr(drv->dft_y_addr, drv->dft_c_addr);
    
    	cam_enable(drv, 0);
 
    	/* init std_head */
    	INIT_LIST_HEAD(&drv->head);

    	spin_lock_init(&cmos_lock);

    /*--------------------------------------------------------------------------
        Step 3:
    --------------------------------------------------------------------------*/

    	if (request_irq(WMT_VID_IRQ ,(void *)&cmos_isr, IRQF_SHARED, "wmt-cmos", (void *)dev) < 0) {      
		DBG_MSG(KERN_INFO "CMOS: Failed to register CMOS irq %i\n", WMT_VID_IRQ);
    	}

    	TRACE("Leave\n");

    	return ret;
} /* End of cmos_open() */

/*!*************************************************************************
* cmos_release
* 
* Private Function by Max Chen, 2009/1/25
*/
/*!
* \brief
*       release CMOS module
* \retval  0 if success
*/ 

static int cmos_release(struct file *filp)
{
    struct cmos_dev_s *dev = (struct cmos_dev_s *)filp->private_data;
    cmos_drvinfo_t *drv = 0;
    cmos_fb_t *fb;
    int  i;    

    TRACE("Enter\n");
    printk("cmos_release() \n");
    if( dev ) {
        drv = &dev->drvinfo;
    }


    power_ctrl(0, cmos_env.dev1_pwr_active_level, cmos_env.dev1_pwr_bitnum, cmos_env.dev1_pwr_reg_gpio_en, cmos_env.dev1_pwr_reg_gpio_od,cmos_env.dev1_pwr_reg_gpio_oc);
    if ( cmos_env.dev_tot_num == 2)
    {
	power_ctrl(0, cmos_env.dev2_pwr_active_level, cmos_env.dev2_pwr_bitnum, cmos_env.dev2_pwr_reg_gpio_en, cmos_env.dev2_pwr_reg_gpio_od,cmos_env.dev2_pwr_reg_gpio_oc);
    }

    DBG_MSG("dev: 0x%x, drv: 0x%x\n", (unsigned int)dev, (unsigned int)drv);
    
    if( drv ) {
        /* Free memory */
        for(i=0; i<drv->fb_cnt; i++) {
            fb = &drv->fb_pool[i];
            if( fb->y_addr ) {
        	    mb_free(fb->y_addr);
        	    memset(fb, 0, sizeof(cmos_fb_t));
            }
        }
    }
    mb_free(drv->dft_y_addr);
    
    wmt_vid_close(VID_MODE_CMOS);

    cmos_exit_device(drv->width, drv->height);

	free_irq(WMT_VID_IRQ, (void *)dev);
	cmos_dev_ref--;

    TRACE("Leave\n");

    return 0;    
} /* End of cmos_release() */

int cmos_cam_open( struct file *filp)
{
    return cmos_open( filp);
}
EXPORT_SYMBOL(cmos_cam_open);


int cmos_cam_release(struct file *filp)
{
    return cmos_release( filp);
}
EXPORT_SYMBOL(cmos_cam_release);
/*!*************************************************************************
	driver file operations struct define
****************************************************************************/
struct file_operations cmos_fops = {
	.owner = THIS_MODULE,
	.open = cmos_open,	
	.release = cmos_release,
};

/*!*************************************************************************
* cmos_probe
* 
* Private Function by Max Chen, 2009/1/25
*/
/*!
* \brief
*		
* \retval  0 if success
*/ 
static int cmos_probe(struct device *dev)
{
	int ret = 0;
	dev_t dev_no;
	struct cdev *cdev;

    TRACE("Enter\n");

	dev_no = MKDEV(cmos_dev_major,cmos_dev_minor);

	/* register char device */
	// cdev = cdev_alloc();
	cdev = &dev_s.cdev;
	cdev_init(cdev,&cmos_fops);
	ret = cdev_add(cdev,dev_no,1);		
	if( ret ){
		printk(KERN_ALERT "*E* register char dev \n");
		return ret;
	}
	//printk( KERN_ALERT "/dev/%s major number %d, minor number %d\n", DEVICE_NAME, cmos_dev_major,cmos_dev_minor);
    TRACE("Leave\n");

	return ret;
} /* End of cmos_probe() */

/*!*************************************************************************
* cmos_remove
* 
* Private Function by Max Chen, 2009/1/25
*/
/*!
* \brief
*		
* \retval  0 if success
*/ 
static int cmos_remove(struct device *dev)
{
	struct cdev *cdev;

	cdev = &dev_s.cdev;	
	cdev_del(cdev);
	
	printk( KERN_ALERT "Enter cmos_remove \n");
	return 0;
} /* End of cmos_remove() */

/*!*************************************************************************
* cmos_suspend
* 
* Private Function by Max Chen, 2009/1/25
*/
/*!
* \brief
*		
* \retval  0 if success
*/ 
static int cmos_suspend(struct device * dev, pm_message_t state)
{
    TRACE("Enter\n");
    printk("cmos_suspend()\n");
    
    if (cmos_dev_ref)
    		cmos_release(gfilp);

    TRACE("Leave\n");

	return 0;
} /* End of cmos_suspend() */

/*!*************************************************************************
* cmos_resume
* 
* Private Function by Max Chen, 2009/1/25
*/
/*!
* \brief
*		
* \retval  0 if success
*/ 
static int cmos_resume(struct device * dev) //struct device *dev, u32 level)
{
    printk("cmos_resume()\n");
    
	return 0;
} /* End of cmos_resume() */

/*!*************************************************************************
	device driver struct define
****************************************************************************/


static struct platform_driver  cmos_driver = {
	/* 
	 * Platform bus will compare the driver name 
	 * with the platform device name. 
	 */
	.driver.name = "cmos",
	.remove = cmos_remove,
	.suspend = cmos_suspend,
	.resume = cmos_resume
};




/*!*************************************************************************
* cmos_platform_release
* 
* Private Function by Max Chen, 2009/1/25
*/
/*!
* \brief
*		
* \retval  0 if success
*/ 
static void cmos_platform_release(struct device *device)
{
} /* End of cmos_platform_release() */

/*!*************************************************************************
	platform device struct define
****************************************************************************/


static u64 cmos_dma_mask = 0xffffffff;

static struct platform_device cmos_device = {
	/* 
	 * Platform bus will compare the driver name 
	 * with the platform device name. 
	 */
	.name = "cmos",
	.id = 0,
	.dev = { 
		.release = cmos_platform_release,
		.dma_mask = &cmos_dma_mask,
	},
	.num_resources = 0,     /* ARRAY_SIZE(rmtctl_resources), */
	.resource = NULL,       /* rmtctl_resources, */
};








/*!*************************************************************************
* cmos_init
* 
* Private Function by Max Chen, 2009/1/25
*/
/*!
* \brief
*		
* \retval  0 if success
*/ 
static int cmos_init(void)
{
	int ret;
	dev_t dev_no; 
	
    TRACE("Enter\n");
    if (get_cmos_uboot_env())
    	 return -1;
    
	if (platform_device_register(&cmos_device))//add by jay,for modules support
		return -1;
    
	ret = platform_driver_probe(&cmos_driver, cmos_probe);

     TRACE("Leave\n");

	return ret;
} /* End of cmos_init() */


module_init(cmos_init);

/*!*************************************************************************
* cmos_exit
* 
* Private Function by Max Chen, 2009/1/25
*/
/*!
* \brief
*		
* \retval  0 if success
*/ 
static void cmos_exit(void)
{
	dev_t dev_no;

    TRACE("Enter\n");
	printk(KERN_ALERT "Enter cmos_exit\n");
	
	driver_unregister(&cmos_driver);
	platform_device_unregister(&cmos_device);
	dev_no = MKDEV(cmos_dev_major,cmos_dev_minor);	
	unregister_chrdev_region(dev_no,cmos_dev_nr);
    TRACE("Leave\n");

	return;
} /* End of cmos_exit() */

module_exit(cmos_exit);

MODULE_AUTHOR("WonderMedia SW Team Max Chen");
MODULE_DESCRIPTION("cmos device driver");
MODULE_LICENSE("GPL");
/*--------------------End of Function Body -----------------------------------*/
#undef DBG_MSG
#undef DBG_DETAIL
#undef TRACE
#undef DBG_ERR

#undef WMT_CMOS_C

