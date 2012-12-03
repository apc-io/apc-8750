/*++ 
 * linux/drivers/media/video/wmt_v4l2/tv_decoder/wmt-tv-decoder.c
 * WonderMedia v4l tv decoder device driver
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


#define WMT_TV_DECODER_C

#include <asm/uaccess.h>    //VERIFY_WRITE
#include <linux/cdev.h>     //cdev
#include <linux/major.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>//platform_bus_type
#include <linux/i2c.h>      //I2C_M_RD
#include "../../../../video/wmt/memblock.h"

/* V4L2 */
#include <linux/videodev2.h>
#include <linux/dma-mapping.h>
#ifdef CONFIG_VIDEO_V4L1_COMPAT
#include <linux/videodev.h>
#endif
//#include <media/video-buf.h>
#include <media/v4l2-common.h>

#include "wmt-tv-decoder.h"
#include "tv-device.h"
#include "../wmt-vid.h"

//#define TV_DECODER_REG_TRACE
#ifdef TV_DECODER_REG_TRACE
#define TV_DECODER_REG_SET32(addr, val)  \
        printk("REG_SET:0x%x -> 0x%0x\n", addr, val);\
        REG32_VAL(addr) = (val)
#else
#define TV_DECODER_REG_SET32(addr, val)      REG32_VAL(addr) = (val)
#endif


//#define TV_DECODER_DEBUG    /* Flag to enable debug message */
//#define TV_DECODER_DEBUG_DETAIL
//#define TV_DECODER_TRACE

#define VID_REG_SET32(addr, val)      REG32_VAL(addr) = (val)


#ifdef TV_DECODER_DEBUG
  #define DBG_MSG(fmt, args...)    PRINT("{%s} " fmt, __FUNCTION__ , ## args)
#else
  #define DBG_MSG(fmt, args...)
#endif

#ifdef TV_DECODER_DEBUG_DETAIL
#define DBG_DETAIL(fmt, args...)   PRINT("{%s} " fmt, __FUNCTION__ , ## args)
#else
#define DBG_DETAIL(fmt, args...)
#endif

#ifdef TV_DECODER_TRACE
  #define TRACE(fmt, args...)      PRINT("{%s}:  " fmt, __FUNCTION__ , ## args)
#else
  #define TRACE(fmt, args...) 
#endif

#define DBG_ERR(fmt, args...)      PRINT("*E* {%s} " fmt, __FUNCTION__ , ## args)


#define THE_MB_USER              "TV_DECODER-MB"

#define ALIGN64(a)               (((a)+63) & (~63))


struct tv_decoder_dev_s {
	/* module parameters */
    tv_decoder_drvinfo_t  drvinfo;
	char *buf;

	/* char dev struct */
	struct cdev cdev;
};


static struct tv_decoder_dev_s dev_s;
static struct file *gfilp;

//static int tv_decoder_dev_ref = 0; /* is device open */


#ifdef __KERNEL__
DECLARE_WAIT_QUEUE_HEAD(tv_decoder_wait);
#endif

static spinlock_t tv_decoder_lock;

static int tv_decoder_dev_major = VID_MAJOR;
static int tv_decoder_dev_minor = 1;
static int tv_decoder_dev_nr = 1;
static int tv_decoder_dev_ref = 0; /* is device open */
static struct tv_decoder_dev_s tv_decoder_dev;


static struct workqueue_struct *vid_device_notify_work;
static struct work_struct vid_work;
static struct timer_list vid_timer;
static int vid_auto_swithch_en=0;



extern void wmt_vid_set_common_mode(vid_tvsys_e tvsys);
cmos_uboot_env_t tv_decoder_env;
extern int wmt_getsyspara(char *varname, unsigned char *varval, int *varlen);

extern void power_ctrl(bool en , unsigned int active_level, unsigned int in_bitnum,unsigned int reg_gpio_en ,  unsigned int reg_gpio_od,  unsigned int reg_gpio_oc);

int get_tv_decoder_uboot_env()
{
   	char varbuf2[128];
	int n;
	int varlen;
	unsigned int reggpioen,reggpiood,reggpiooc;
	char bitnum,active_level,dummy;
		
	//get the wmt.gpo.tv_decoder
	memset(varbuf2, 0, sizeof(varbuf2));
	varlen = sizeof(varbuf2);


    	if (wmt_getsyspara("wmt.gpo.tv_decoder", varbuf2, &varlen)) {
		printk("Can't get tv_decoder config in u-boot!!!!\n");
		tv_decoder_env.en=0;
		return -1;
	} else {
		n = sscanf(varbuf2, "%d:%d:%d:%08x:%08x:%08x:%d:%d:%08x:%08x:%08x",
					&tv_decoder_env.dev_tot_num,
					&tv_decoder_env.dev1_pwr_active_level,
					&tv_decoder_env.dev1_pwr_bitnum,
					&tv_decoder_env.dev1_pwr_reg_gpio_en,
					&tv_decoder_env.dev1_pwr_reg_gpio_od,
					&tv_decoder_env.dev1_pwr_reg_gpio_oc,
					
					&tv_decoder_env.dev2_pwr_active_level,
					&tv_decoder_env.dev2_pwr_bitnum,
					&tv_decoder_env.dev2_pwr_reg_gpio_en,
					&tv_decoder_env.dev2_pwr_reg_gpio_od,
					&tv_decoder_env.dev2_pwr_reg_gpio_oc			
		);
		
		if (n <= 5) 
		{
			printk("tv_decoder format is error in u-boot!!!\n");
			tv_decoder_env.en=0;
		}else{
		
		tv_decoder_env.en=1;
			tv_decoder_env.dev1_pwr_reg_gpio_en =  GPIO_BASE_ADDR + (tv_decoder_env.dev1_pwr_reg_gpio_en & 0xfff);
			tv_decoder_env.dev1_pwr_reg_gpio_od = GPIO_BASE_ADDR + (tv_decoder_env.dev1_pwr_reg_gpio_od & 0xfff);
			tv_decoder_env.dev1_pwr_reg_gpio_oc = GPIO_BASE_ADDR + (tv_decoder_env.dev1_pwr_reg_gpio_oc & 0xfff);

			tv_decoder_env.dev2_pwr_reg_gpio_en =  GPIO_BASE_ADDR + (tv_decoder_env.dev2_pwr_reg_gpio_en & 0xfff);
			tv_decoder_env.dev2_pwr_reg_gpio_od = GPIO_BASE_ADDR + (tv_decoder_env.dev2_pwr_reg_gpio_od & 0xfff);
			tv_decoder_env.dev2_pwr_reg_gpio_oc = GPIO_BASE_ADDR + (tv_decoder_env.dev2_pwr_reg_gpio_oc & 0xfff);
		}
	}

	//get the gpio i2c setting 
	memset(varbuf2, 0, sizeof(varbuf2));
	varlen = sizeof(varbuf2);

    	if (wmt_getsyspara("wmt.tv_decoder.i2c_gpio", varbuf2, &varlen)) {
		printk("Can't get wmt.tv_decoder.i2c_gpio config in u-boot!!!!\n");
		tv_decoder_env.i2c_gpio_en = 0;

	} else {
		n = sscanf(varbuf2, "%d:%d:%d:%08x:%08x:%08x:%08x:%08x:%d:%d:%08x:%08x:%08x:%08x:%08x",
					&tv_decoder_env.i2c_gpio_en,
					
					&tv_decoder_env.i2c_gpio_scl_binum,
					&tv_decoder_env.reg_i2c_gpio_scl_gpio_pe_bitnum,
					&tv_decoder_env.reg_i2c_gpio_scl_gpio_in,
					&tv_decoder_env.reg_i2c_gpio_scl_gpio_en,
					&tv_decoder_env.reg_i2c_gpio_scl_gpio_od,
					&tv_decoder_env.reg_i2c_gpio_scl_gpio_oc,
					&tv_decoder_env.reg_i2c_gpio_scl_gpio_pe,
					
					&tv_decoder_env.i2c_gpio_sda_binum,
					&tv_decoder_env.reg_i2c_gpio_sda_gpio_pe_bitnum,
					&tv_decoder_env.reg_i2c_gpio_sda_gpio_in,
					&tv_decoder_env.reg_i2c_gpio_sda_gpio_en,
					&tv_decoder_env.reg_i2c_gpio_sda_gpio_od,
					&tv_decoder_env.reg_i2c_gpio_sda_gpio_oc,
					&tv_decoder_env.reg_i2c_gpio_sda_gpio_pe

		);
		if (n != 15) 
		{
			printk("Don't get wmt.tv_decoder.i2c_gpio \n");
			tv_decoder_env.i2c_gpio_en = 0;

		}else{
		tv_decoder_env.reg_i2c_gpio_scl_gpio_in = GPIO_BASE_ADDR + (tv_decoder_env.reg_i2c_gpio_scl_gpio_in &0xfff);
		tv_decoder_env.reg_i2c_gpio_scl_gpio_en = GPIO_BASE_ADDR + (tv_decoder_env.reg_i2c_gpio_scl_gpio_en &0xfff);
		tv_decoder_env.reg_i2c_gpio_scl_gpio_od = GPIO_BASE_ADDR + (tv_decoder_env.reg_i2c_gpio_scl_gpio_od &0xfff);
		tv_decoder_env.reg_i2c_gpio_scl_gpio_oc = GPIO_BASE_ADDR + (tv_decoder_env.reg_i2c_gpio_scl_gpio_oc &0xfff);
		tv_decoder_env.reg_i2c_gpio_scl_gpio_pe = GPIO_BASE_ADDR + (tv_decoder_env.reg_i2c_gpio_scl_gpio_pe &0xfff);


		tv_decoder_env.reg_i2c_gpio_sda_gpio_in = GPIO_BASE_ADDR + (tv_decoder_env.reg_i2c_gpio_sda_gpio_in &0xfff);
		tv_decoder_env.reg_i2c_gpio_sda_gpio_en = GPIO_BASE_ADDR + (tv_decoder_env.reg_i2c_gpio_sda_gpio_en &0xfff);
		tv_decoder_env.reg_i2c_gpio_sda_gpio_od = GPIO_BASE_ADDR + (tv_decoder_env.reg_i2c_gpio_sda_gpio_od &0xfff);
		tv_decoder_env.reg_i2c_gpio_sda_gpio_oc = GPIO_BASE_ADDR + (tv_decoder_env.reg_i2c_gpio_sda_gpio_oc &0xfff);
		tv_decoder_env.reg_i2c_gpio_sda_gpio_pe = GPIO_BASE_ADDR + (tv_decoder_env.reg_i2c_gpio_sda_gpio_pe &0xfff);
		}

	}

       print_info :

		printk("tv_decoder UBOOT ARG\n");
		printk("get the tv_decoder param  setting wmt.tv_decoder.i2c_gpio : %d:%d:%d:0x%08x:0x%08x:0x%08x:0x%08x:0x%08x:%d:%d:0x%08x:0x%08x:0x%08x:0x%08x:0x%08x\n",
					tv_decoder_env.i2c_gpio_en,				
					tv_decoder_env.i2c_gpio_scl_binum,
					tv_decoder_env.reg_i2c_gpio_scl_gpio_pe_bitnum,
					tv_decoder_env.reg_i2c_gpio_scl_gpio_in,
					tv_decoder_env.reg_i2c_gpio_scl_gpio_en,
					tv_decoder_env.reg_i2c_gpio_scl_gpio_od,
					tv_decoder_env.reg_i2c_gpio_scl_gpio_oc,
					tv_decoder_env.reg_i2c_gpio_scl_gpio_pe,
					
					tv_decoder_env.i2c_gpio_sda_binum,
					tv_decoder_env.reg_i2c_gpio_sda_gpio_pe_bitnum,
					tv_decoder_env.reg_i2c_gpio_sda_gpio_in,
					tv_decoder_env.reg_i2c_gpio_sda_gpio_en,
					tv_decoder_env.reg_i2c_gpio_sda_gpio_od,
					tv_decoder_env.reg_i2c_gpio_sda_gpio_oc,
					tv_decoder_env.reg_i2c_gpio_sda_gpio_pe

		);

		printk("get the tv_decoder gpio setting : %d:%d:%d:0x%08x:0x%08x:0x%08x:%d:%d:0x%08x:0x%08x:0x%08x\n",
					tv_decoder_env.dev_tot_num,
					tv_decoder_env.dev1_pwr_active_level,
					tv_decoder_env.dev1_pwr_bitnum,
					tv_decoder_env.dev1_pwr_reg_gpio_en,
					tv_decoder_env.dev1_pwr_reg_gpio_od,
					tv_decoder_env.dev1_pwr_reg_gpio_oc,
					
					tv_decoder_env.dev2_pwr_active_level,
					tv_decoder_env.dev2_pwr_bitnum,
					tv_decoder_env.dev2_pwr_reg_gpio_en,
					tv_decoder_env.dev2_pwr_reg_gpio_od,
					tv_decoder_env.dev2_pwr_reg_gpio_oc			
		);

	return 0;
}

/*-------------------------------Body Functions------------------------------------*/

int timeoutcnt=0  ;

char tvsys_old=0x0,lock_sts_old=0;



void  do_vid_work(void)
{
	  char tvsys,lock_sts;
		tvsys = viddev_get_status(VIDDEV_STS_TVSYS);
		//printk("do_vid_work() %d \n",tvsys);
		if (tvsys != tvsys_old)
		{
			if (tvsys==VID_NTSC)
			{
				 wmt_vid_set_common_mode(VID_NTSC);
				printk("set tv mode N\n");
  
			}else{
				 wmt_vid_set_common_mode(VID_PAL);
				printk("set tv mode P\n");
			}
		}
		tvsys_old = tvsys;
		lock_sts = viddev_get_status( VIDDEV_STS_LOCK );


		if (!lock_sts_old)
			VID_REG_SET32( REG_BASE_VID+0x00, 0x100 );

		if ((lock_sts)&&(!lock_sts_old))
		{
			DBG_MSG("!!!!!!!!!!!!!!reset decoder \n");
			VID_REG_SET32( REG_BASE_VID+0x00, 0x100 );
			VID_REG_SET32( REG_BASE_VID+0x00, 0x101 );
		}
	
		lock_sts_old=lock_sts;


		return;
}

static void vid_timeout(unsigned long data)
{
        if (vid_auto_swithch_en)
	{
 	 del_timer(&vid_timer);
	 	 
	  vid_timer.expires = jiffies + 1*100;//this is 3 sec for  3*100
	  vid_timer.function = vid_timeout;

	  queue_work(vid_device_notify_work, &vid_work);

	  add_timer(&vid_timer);
	}
	return;
}
static int tv_decoder_enable(tv_decoder_drvinfo_t *drv, int en)
{
    drv->streamoff = (en ^ 0x1);

    TV_DECODER_REG_SET32( REG_BASE_VID+0x0, en);	/* enable TV_DECODER */
    
    return 0;
} /* End of tv_decoder_enable() */  

static int print_queue(tv_decoder_drvinfo_t *drv)
{
#ifdef TV_DECODER_DEBUG
    struct list_head *next;
    tv_decoder_fb_t  *fb;
    
    TRACE("Enter\n");

    list_for_each(next, &drv->head) {
        fb = (tv_decoder_fb_t *) list_entry(next, tv_decoder_fb_t, list);
        DBG_MSG("[%d] y_addr: 0x%x, c_addr: 0x%x\n",
                            fb->id, fb->y_addr, fb->c_addr );
    }
    TRACE("Leave\n");
#endif
    return 0;
} /* End of print_queue();*/

static int put_queue(tv_decoder_drvinfo_t *drv, tv_decoder_fb_t *fb_in)
{
    TRACE("Enter\n");

    list_add_tail(&fb_in->list, &drv->head);
    print_queue(drv);

    TRACE("Leave\n");

    return 0;
} /* End of put_queue() */

static int pop_queue(tv_decoder_drvinfo_t *drv, tv_decoder_fb_t *fb_in)
{
    TRACE("Enter\n");

    list_del(&fb_in->list);
    print_queue(drv);
    
    TRACE("Leave\n");

    return 0;
} /* End of pop_queue() */

/*------------------------------------------------------------------------------
    Export V4L2 functions for TV_DECODER  
------------------------------------------------------------------------------*/

int tv_decoder_querycap(struct file *file, void *fh, struct v4l2_capability *cap)
{

    TRACE("Enter\n");

    memset(cap, 0, sizeof(*cap));
    strlcpy(cap->driver, "wm85xx", sizeof(cap->driver));

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
EXPORT_SYMBOL(tv_decoder_querycap);

int tv_decoder_enum_fmt_cap(struct file *file, void *fh, struct v4l2_fmtdesc *f)
{
    DBG_ERR("Not support now!\n");
	return 0;
}
EXPORT_SYMBOL(tv_decoder_enum_fmt_cap);

int tv_decoder_g_fmt_cap(struct file *file, void *fh, struct v4l2_format *f)
{
    DBG_ERR("Not support now!\n");
	return 0;
}
EXPORT_SYMBOL(tv_decoder_g_fmt_cap);

int tv_decoder_s_fmt_cap(struct file *file, void *fh, struct v4l2_format *f)
{
    struct tv_decoder_dev_s *dev = (struct tv_decoder_dev_s *)file->private_data;
    tv_decoder_drvinfo_t *drv = &dev->drvinfo;
    int ret;

    TRACE("Enter\n");
    
    if( (dev == 0) || (drv == 0) )
        return -1;

	drv->width  = f->fmt.pix.width;
	drv->height = f->fmt.pix.height;
    drv->frame_size = ALIGN64(drv->width) * drv->height << 1; // "<< 1" is for YC422
   
    DBG_MSG(" width:      %d\n", drv->width);
    DBG_MSG(" height:     %d\n", drv->height);
    DBG_MSG(" frame_size: %d\n", drv->frame_size);

    ret = tv_init_ext_device(drv->width, drv->height);

    if( ret != 0 ) {
        DBG_ERR("Initial TV_DECODER sensor for %d x %d fail!\n", drv->width, drv->height);
        return -1;
    }
    ret = wmt_vid_set_mode(drv->width, drv->height);
    
    TRACE("Leave\n");
    
    return 0;
}
EXPORT_SYMBOL(tv_decoder_s_fmt_cap);

int tv_decoder_try_fmt_cap(struct file *file, void *fh, struct v4l2_format *f)
{
    return tv_decoder_s_fmt_cap(file, fh, f);
}
EXPORT_SYMBOL(tv_decoder_try_fmt_cap);

int tv_decoder_reqbufs(struct file *file, void *fh, struct v4l2_requestbuffers *rb)
{
    struct tv_decoder_dev_s *dev = (struct tv_decoder_dev_s *)file->private_data;
    tv_decoder_drvinfo_t *drv = &dev->drvinfo;

    tv_decoder_fb_t *fb;
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
        if( fb->y_addr == 0 ) {
            /* Allocate MB memory size fail */
            DBG_ERR("[%d] Allocate MB memory (%d) fail!\n", i, drv->frame_size);
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
EXPORT_SYMBOL(tv_decoder_reqbufs);

int tv_decoder_querybuf(struct file *file, void *fh, struct v4l2_buffer *b)
{
    struct tv_decoder_dev_s *dev = (struct tv_decoder_dev_s *)file->private_data;
    tv_decoder_drvinfo_t *drv = &dev->drvinfo;

    TRACE("Enter (index: %d)\n", b->index);

    if( b->index < MAX_FB_IN_QUEUE ) {
        tv_decoder_fb_t *fb = &drv->fb_pool[b->index];
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
EXPORT_SYMBOL(tv_decoder_querybuf);

int tv_decoder_qbuf(struct file *file, void *fh, struct v4l2_buffer *b)
{
    struct tv_decoder_dev_s *dev = (struct tv_decoder_dev_s *)file->private_data;
    tv_decoder_drvinfo_t *drv = &dev->drvinfo;

    tv_decoder_fb_t *fb;

    TRACE("Enter (index: %d)\n", b->index);
    
    fb = &drv->fb_pool[b->index];
    
    put_queue(drv, fb);

    TRACE("Leave (index: %d)\n", b->index);

	return 0;
}
EXPORT_SYMBOL(tv_decoder_qbuf);

int tv_decoder_dqbuf(struct file *file, void *fh, struct v4l2_buffer *b)
{
    struct tv_decoder_dev_s *dev = (struct tv_decoder_dev_s *)file->private_data;
    tv_decoder_drvinfo_t    *drv = &dev->drvinfo;
    unsigned long      flags =0;
    tv_decoder_fb_t *fb = 0;

    TRACE("Enter (index: %d)\n", b->index);

    if( drv->streamoff ) {
        struct list_head  *next;

        /* TV_DECODER sensor did not work now */
        list_for_each(next, &drv->head) {
            fb = (tv_decoder_fb_t *) list_entry(next, tv_decoder_fb_t, list);
            if( fb ) {
                pop_queue(drv, fb);    
                break;
            }
        }
        goto EXIT_tv_decoder_dqbuf;
    }
    
    fb = (tv_decoder_fb_t *)wmt_vid_get_cur_fb();
    
    spin_lock_irqsave(&tv_decoder_lock, flags);

    DBG_MSG("Set DQBUF on\n");
    drv->dqbuf  = 1;

    spin_unlock_irqrestore(&tv_decoder_lock, flags);
    
    if( (wait_event_interruptible_timeout( tv_decoder_wait,
                      (drv->_status & STS_TV_DECODER_FB_DONE), drv->_timeout) == 0) ){
        DBG_ERR("TV_DECODER Time out in %d ms\n", drv->_timeout);
    }
    drv->_status &= (~STS_TV_DECODER_FB_DONE);

    DBG_MSG("[%d] fb: 0x%p\n", fb->id, fb);
    pop_queue(drv, fb);    

    b->length   = drv->frame_size;
    b->m.offset = fb->y_addr;
    b->index    = fb->id;

EXIT_tv_decoder_dqbuf:
    TRACE("Leave (index: %d)\n", b->index);

    return 0;
}
EXPORT_SYMBOL(tv_decoder_dqbuf);

int tv_decoder_s_std(struct file *file, void *fh, v4l2_std_id a)
{
    DBG_ERR("Not support now!\n");
	return 0;
}
EXPORT_SYMBOL(tv_decoder_s_std);

int tv_decoder_enum_input(struct file *file, void *fh, struct v4l2_input *inp)
{
    DBG_ERR("Not support now!\n");
	return 0;
}
EXPORT_SYMBOL(tv_decoder_enum_input);

int tv_decoder_g_input(struct file *file, void *fh, unsigned int *i)
{
    DBG_ERR("Not support now!\n");
	return 0;
}
EXPORT_SYMBOL(tv_decoder_g_input);

int tv_decoder_s_input(struct file *file, void *fh, unsigned int i)
{
    DBG_ERR("Not support now!\n");
	return 0;
}
EXPORT_SYMBOL(tv_decoder_s_input);

int tv_decoder_queryctrl(struct file *file, void *fh, struct v4l2_queryctrl *a)
{
    DBG_ERR("Not support now!\n");
	return 0;
}
EXPORT_SYMBOL(tv_decoder_queryctrl);

int tv_decoder_g_ctrl(struct file *file, void *fh, struct v4l2_control *a)
{
    DBG_ERR("Not support now!\n");
	return 0;
}
EXPORT_SYMBOL(tv_decoder_g_ctrl);

int tv_decoder_s_ctrl(struct file *file, void *fh, struct v4l2_control *a)
{
    DBG_ERR("Not support now!\n");
    return 0;
}
EXPORT_SYMBOL(tv_decoder_s_ctrl);

int tv_decoder_streamon(struct file *file, void *fh, enum v4l2_buf_type i)
{
    struct tv_decoder_dev_s *dev = (struct tv_decoder_dev_s *)file->private_data;
    tv_decoder_drvinfo_t *drv = &dev->drvinfo;
    tv_decoder_fb_t  *fb;

    TRACE("Enter\n");

    fb = (tv_decoder_fb_t *) list_entry(drv->head.next, tv_decoder_fb_t, list);

    wmt_vid_set_cur_fb((vid_fb_t *)fb);

    tv_decoder_enable(drv, 1);

    vid_auto_swithch_en =1;
    del_timer(&vid_timer);
    vid_timer.expires = jiffies + 1*100;//this is 3 sec
    vid_timer.function = vid_timeout;
    add_timer(&vid_timer);


    
    TRACE("Leave\n");

    return 0;
}
EXPORT_SYMBOL(tv_decoder_streamon);

int tv_decoder_streamoff(struct file *file, void *fh, enum v4l2_buf_type i)
{
    struct tv_decoder_dev_s *dev = (struct tv_decoder_dev_s *)file->private_data;
    tv_decoder_drvinfo_t *drv = &dev->drvinfo;

    TRACE("Enter\n");

    tv_decoder_enable(drv, 0);
    
    TRACE("Leave\n");

	return 0;
}
EXPORT_SYMBOL(tv_decoder_streamoff);

int tv_decoder_mmap(struct file *file, struct vm_area_struct *vma)
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
EXPORT_SYMBOL(tv_decoder_mmap);


/*!*************************************************************************
* tv_decoder_isr
* 
* Public Function by Max Chen, 2010/05/28
*/
/*!
* \brief
*       init TV_DECODER module
* \retval  0 if success
*/ 
#ifdef __KERNEL__
static irqreturn_t tv_decoder_isr(int irq,void *dev_in,struct pt_regs *regs)
#else
static void tv_decoder_isr(void)
#endif
{
    struct tv_decoder_dev_s *dev = (struct tv_decoder_dev_s *)dev_in;
    tv_decoder_drvinfo_t    *drv;
    struct list_head  *next;
    tv_decoder_fb_t *cur_fb, *fb;
    unsigned long flags =0;
    
    TRACE("Enter\n");

    
    if( dev == 0 ) {        
        return IRQ_NONE;
    }
    drv = &dev->drvinfo;

    if( drv->dqbuf == 1 ) {
        spin_lock_irqsave(&tv_decoder_lock, flags);
        drv->dqbuf = 0;
        DBG_MSG("Set DBBUF off\n");
        spin_unlock_irqrestore(&tv_decoder_lock, flags);

        cur_fb = (tv_decoder_fb_t *)wmt_vid_get_cur_fb();
        list_for_each(next, &drv->head) {
            fb = (tv_decoder_fb_t *) list_entry(next, tv_decoder_fb_t, list);
            if( fb == cur_fb ) {
                /*-------------------------------------------------------------- 
                    Get next FB 
                --------------------------------------------------------------*/
                fb = (tv_decoder_fb_t *) list_entry(cur_fb->list.next, tv_decoder_fb_t, list);
                cur_fb->is_busy = 0;
                cur_fb->done    = 1;
                DBG_MSG("[%d] done: %d, is_busy: %d\n", cur_fb->id, cur_fb->done, cur_fb->is_busy);
                DBG_MSG("[%d] New FB done: %d, is_busy: %d\n", fb->id, fb->done, fb->is_busy);

                wmt_vid_set_cur_fb((vid_fb_t *)fb); 

                drv->_status |= STS_TV_DECODER_FB_DONE;
                wake_up_interruptible(&tv_decoder_wait);
                break;
            }
        }
    } /* if( drv->dqbuf == 1 ) */
 	TV_DECODER_REG_SET32(REG_VID_INT_CTRL, REG32_VAL(REG_VID_INT_CTRL));

    TRACE("Leave\n");

#ifdef __KERNEL__
    return IRQ_HANDLED;
#endif
} /* End of tv_decoder_isr() */

/*!*************************************************************************
* tv_decoder_open
* 
* Private Function by Max Chen, 2009/1/25
*/
/*!
* \brief
*       init TV_DECODER module
* \retval  0 if success
*/ 
static int tv_dec_open(struct file *filp)
{
	struct tv_decoder_dev_s *dev=&dev_s;

       tv_decoder_drvinfo_t    *drv ;
	int  ret = 0;

       TRACE("Enter\n");
    /*--------------------------------------------------------------------------
        Step 1:
    --------------------------------------------------------------------------*/

	if (!tv_decoder_env.en)
	{
		printk("please set the tv decoder uboot env\n");
		return -1;
	}
	
	if(tv_decoder_dev_ref)
		return -EBUSY;
	
	tv_decoder_dev_ref++;
	
	gfilp = filp;
	
	filp->private_data = dev;
	drv = &dev->drvinfo;


  /*--------------------------------------------------------------------------
        Step 2:
    --------------------------------------------------------------------------*/
    	memset(&dev_s, 0, sizeof(tv_decoder_drvinfo_t));

   wmt_vid_open(VID_MODE_TVDEC, &tv_decoder_env);

    drv->width  = 720;
    drv->height = 576;

    drv->_timeout = 100;   // ms
    drv->_status  = STS_TV_DECODER_READY;
    drv->dqbuf    = 0;

    drv->frame_size = ALIGN64(drv->width) * drv->height << 1; // "<< 1" is for YC422

    drv->dft_y_addr = mb_alloc(drv->frame_size);
    if( drv->dft_y_addr == 0 ) {
        DBG_ERR("Allocate MB memory (%d) fail!\n", drv->frame_size);
        return -EINVAL; 
    }
    drv->dft_c_addr = drv->dft_y_addr + (drv->frame_size / 2);

    wmt_vid_set_addr(drv->dft_y_addr, drv->dft_c_addr);
    
    tv_decoder_enable(drv, 0);

	/* init std_head */
	INIT_LIST_HEAD(&drv->head);

    spin_lock_init(&tv_decoder_lock);

    /*--------------------------------------------------------------------------
        Step 3:
    --------------------------------------------------------------------------*/
  	if (request_irq(WMT_VID_IRQ , &tv_decoder_isr, IRQF_SHARED, "wmt-tv_decoder", (void *)dev) < 0) {      
		DBG_MSG(KERN_INFO "TV_DECODER: Failed to register TV_DECODER irq %i\n", WMT_VID_IRQ);
	}

	vid_device_notify_work = create_workqueue("vid_notify");
	if (!vid_device_notify_work) {
		printk("vid notify work queue creat error");
	}else{
		DBG_MSG("vid notify work queue creat ok");
	}
	INIT_WORK(&vid_work,(void *) do_vid_work);
  	init_timer(&vid_timer);

	timeoutcnt=0 ;tvsys_old=0x0;



  	

    return ret;
} /* End of tv_decoder_open() */

/*!*************************************************************************
* tv_decoder_release
* 
* Private Function by Max Chen, 2009/1/25
*/
/*!
* \brief
*       release TV_DECODER module
* \retval  0 if success
*/ 
static int tv_dec_release(struct file *filp)
{
    struct tv_decoder_dev_s *dev = (struct tv_decoder_dev_s *)filp->private_data;
    tv_decoder_drvinfo_t *drv = 0;
    tv_decoder_fb_t *fb;
    int  i;    

    TRACE("Enter\n");
    printk("tv_dec_release() \n");
    if( dev ) {
        drv = &dev->drvinfo;
    }
    
    DBG_MSG("dev: 0x%x, drv: 0x%x\n", (unsigned int)dev, (unsigned int)drv);
    
    if( drv ) {
        /* Free memory */
        for(i=0; i<drv->fb_cnt; i++) {
            fb = &drv->fb_pool[i];
            if( fb->y_addr ) {
        	    mb_free(fb->y_addr);
        	    memset(fb, 0, sizeof(tv_decoder_fb_t));
            }
        }
    }
    mb_free(drv->dft_y_addr);
        del_timer(&vid_timer);

    wmt_vid_close(VID_MODE_TVDEC);

    //tv_decoder_exit_device(drv->width, drv->height);

	free_irq(WMT_VID_IRQ, (void *)dev);
	tv_decoder_dev_ref--;

    TRACE("Leave\n");

    return 0;    
} /* End of cmos_release() */




int tv_decoder_open(struct file *filp)
{
    return tv_dec_open( filp);
}
EXPORT_SYMBOL(tv_decoder_open);


int tv_decoder_release( struct file *filp)
{
    return tv_dec_release( filp);
}
EXPORT_SYMBOL(tv_decoder_release);

/*!*************************************************************************
	driver file operations struct define
****************************************************************************/
struct file_operations tv_dec_fops = {
	.owner = THIS_MODULE,
	.open = tv_dec_open,	
	.release = tv_dec_release,
};


/*!*************************************************************************
* tv_decoder_probe
* 
* Private Function by Max Chen, 2009/1/25
*/
/*!
* \brief
*		
* \retval  0 if success
*/ 
static int tv_dec_probe(struct device *dev)
{
	int ret = 0;
	dev_t dev_no;
	struct cdev *cdev;

    TRACE("Enter\n");

	dev_no = MKDEV(tv_decoder_dev_major,tv_decoder_dev_minor);

	/* register char device */
	// cdev = cdev_alloc();
	cdev = &tv_decoder_dev.cdev;
	cdev_init(cdev,&tv_dec_fops);
	ret = cdev_add(cdev,dev_no,1);		
	if( ret ){
		printk(KERN_ALERT "*E* register char dev \n");
		return ret;
	}
    TRACE("Leave\n");

	return ret;
} /* End of tv_decoder_probe() */



/*!*************************************************************************
* tv_decoder_remove
* 
* Private Function by Max Chen, 2009/1/25
*/
/*!
* \brief
*		
* \retval  0 if success
*/ 
static int tv_dec_remove(struct device *dev)
{
	struct cdev *cdev;

	cdev = &tv_decoder_dev.cdev;	
	cdev_del(cdev);
	
	printk( KERN_ALERT "Enter tv_decoder_remove \n");
	return 0;
} /* End of tv_decoder_remove() */

/*!*************************************************************************
* tv_decoder_suspend
* 
* Private Function by Max Chen, 2009/1/25
*/
/*!
* \brief
*		
* \retval  0 if success
*/ 
static int tv_dec_suspend(struct device * dev, pm_message_t state)
{
    TRACE("Enter\n");
    switch (state.event) {
        case PM_EVENT_SUSPEND:
        case PM_EVENT_FREEZE:
        case PM_EVENT_PRETHAW:
        default:
            DBG_ERR("Not implemented now!\n");
            break;	
    }
    TRACE("Leave\n");

	return 0;
} /* End of tv_decoder_suspend() */

/*!*************************************************************************
* tv_decoder_resume
* 
* Private Function by Max Chen, 2009/1/25
*/
/*!
* \brief
*		
* \retval  0 if success
*/ 
static int tv_dec_resume(struct device * dev) //struct device *dev, u32 level)
{
    DBG_ERR("Not implemented now!\n");
    
	return 0;
} /* End of tv_decoder_resume() */

/*!*************************************************************************
	device driver struct define
****************************************************************************/


static struct platform_driver  tv_dec_driver = {
	/* 
	 * Platform bus will compare the driver name 
	 * with the platform device name. 
	 */
	.driver.name = "tv_decoder",
	.remove = tv_dec_remove,
	.suspend = tv_dec_suspend,
	.resume = tv_dec_resume
};






/*!*************************************************************************
* tv_decoder_platform_release
* 
* Private Function by Max Chen, 2009/1/25
*/
/*!
* \brief
*		
* \retval  0 if success
*/ 
static void tv_dec_platform_release(struct device *device)
{
} /* End of tv_decoder_platform_release() */

/*!*************************************************************************
	platform device struct define
****************************************************************************/

static struct platform_device tv_dec_device = {
	.name           = "tv_decoder",
	.id             = 0,
	.dev            = 	{	.release = tv_dec_platform_release,
						},
	.num_resources  = 0,		/* ARRAY_SIZE(tv_decoder_resources), */
	.resource       = NULL,		/* tv_decoder_resources, */
};




/*!*************************************************************************
* tv_decoder_init
* 
* Private Function by Max Chen, 2009/1/25
*/
/*!
* \brief
*		
* \retval  0 if success
*/ 
static int tv_dec_init(void)
{
	int ret;
	dev_t dev_no; 
	
    TRACE("Enter\n");
  if (get_tv_decoder_uboot_env())
    	 return -1;

	if (platform_device_register(&tv_dec_device))//add by jay,for modules support
		return -1;
    
	ret = platform_driver_probe(&tv_dec_driver, tv_dec_probe);

    TRACE("Leave\n");

	return ret;
} /* End of tv_decoder_init() */

module_init(tv_dec_init);

/*!*************************************************************************
* tv_decoder_exit
* 
* Private Function by Max Chen, 2009/1/25
*/
/*!
* \brief
*		
* \retval  0 if success
*/ 
static void tv_dec_exit(void)
{
	dev_t dev_no;

    TRACE("Enter\n");
	printk(KERN_ALERT "Enter tv_decoder_exit\n");
	
	driver_unregister(&tv_dec_driver);
	platform_device_unregister(&tv_dec_device);
	dev_no = MKDEV(tv_decoder_dev_major,tv_decoder_dev_minor);	
	unregister_chrdev_region(dev_no,tv_decoder_dev_nr);
    TRACE("Leave\n");

	return;
} /* End of tv_decoder_exit() */

module_exit(tv_dec_exit);

MODULE_AUTHOR("WonderMedia SW Team Max Chen");
MODULE_DESCRIPTION("tv_decoder device driver");
MODULE_LICENSE("GPL");

/*--------------------End of Function Body -----------------------------------*/
#undef DBG_MSG
#undef DBG_DETAIL
#undef TRACE
#undef DBG_ERR

#undef WMT_TV_DECODER_C

