/*++ 
 * linux/drivers/video/wmt/dev-vpp.c
 * WonderMedia video post processor (VPP) driver
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

#define DEV_VPP_C
// #define DEBUG

// #include <fcntl.h>
// #include <unistd.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/errno.h>
#include <asm/uaccess.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/fb.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <asm/page.h>
#include <linux/mm.h>
#include <linux/proc_fs.h>
#include <linux/sysctl.h>
#include <linux/delay.h>
#include <linux/netlink.h>
#include <net/sock.h>

#include "vpp.h"

#define VPP_DEBUG

#define VPP_PROC_NUM		10
#define VPP_DISP_FB_MAX		10
#define VPP_DISP_FB_NUM		4

typedef struct {
	int (*func)(void *arg);
	void *arg;
	struct list_head list;
	vpp_int_t type;
	struct semaphore sem;
} vpp_proc_t;

typedef struct {
	vpp_dispfb_t parm;
	vpp_pts_t pts;
	struct list_head list;
} vpp_dispfb_parm_t;

struct list_head vpp_disp_fb_list;
struct list_head vpp_disp_free_list;
#ifdef WMT_FTBLK_PIP
struct list_head vpp_pip_fb_list;
#endif
vpp_dispfb_parm_t vpp_disp_fb_array[VPP_DISP_FB_MAX];

typedef struct {
	struct list_head list;
	struct tasklet_struct tasklet;
	void (*proc)(int arg);
} vpp_irqproc_t;

vpp_irqproc_t *vpp_irqproc_array[32];
struct list_head vpp_free_list;
vpp_proc_t vpp_proc_array[VPP_PROC_NUM];

extern void vpp_init(vout_info_t *info);
extern int vpp_config(vout_info_t *info);

extern struct fb_var_screeninfo vfb_var;
int vpp_disp_fb_cnt(struct list_head *list);
void vpp_set_path(int arg,vdo_framebuf_t *fb);

#ifdef CONFIG_VPP_VBIE_FREE_MB
static unsigned int vpp_free_y_addr;
static unsigned int vpp_free_c_addr;
#endif
static unsigned int vpp_cur_dispfb_y_addr;
static unsigned int vpp_cur_dispfb_c_addr;
static unsigned int vpp_pre_dispfb_y_addr;
static unsigned int vpp_pre_dispfb_c_addr;

int vpp_dac_sense_enable = 1;

#ifdef PATCH_VPU_HALT_GOVW_TG_ERR
vpp_path_t vpp_govm_path;
int vpp_govw_tg_err_patch;
int vpp_govw_tg_err_patch_cnt = 0;
#endif

#ifdef CONFIG_SCL_DIRECT_PATH
#define VPP_SCL_MB_MAX	3
#define VPP_SCL_FB_MAX	5
int vpp_scl_dirpath;
struct list_head vpp_scl_fb_list;
struct list_head vpp_scl_free_list;
vpp_dispfb_parm_t vpp_scl_fb_array[VPP_SCL_FB_MAX];
#endif

#define CONFIG_VPP_NOTIFY
#ifdef CONFIG_VPP_NOTIFY
#define VPP_NETLINK_PROC_MAX		2

typedef struct {
	__u32 pid;
	rwlock_t lock;
} vpp_netlink_proc_t;

vpp_netlink_proc_t vpp_netlink_proc[VPP_NETLINK_PROC_MAX];

static struct sock *vpp_nlfd;

/* netlink receive routine */
DECLARE_MUTEX(vpp_netlink_receive_sem); 

#endif

/*----------------------- VPP API in Linux Kernel --------------------------------------*/
#ifdef CONFIG_VPP_NOTIFY
vpp_netlink_proc_t *vpp_netlink_get_proc(int no)
{
	if( no == 0 ) 
		return 0;
	if( no > VPP_NETLINK_PROC_MAX ) 
		return 0;
	return &vpp_netlink_proc[no-1];
}

static void vpp_netlink_receive(struct sk_buff *skb)
{
	struct nlmsghdr * nlh = NULL;
	vpp_netlink_proc_t *proc;

	if(down_trylock(&vpp_netlink_receive_sem))  
		return;

	if(skb->len >= sizeof(struct nlmsghdr)){
		nlh = nlmsg_hdr(skb);
		if((nlh->nlmsg_len >= sizeof(struct nlmsghdr))
			&& (skb->len >= nlh->nlmsg_len)){
			if((proc = vpp_netlink_get_proc(nlh->nlmsg_type))){
				write_lock_bh(proc->lock);
				proc->pid =nlh->nlmsg_pid;
				write_unlock_bh(proc->lock);
				printk("[VPP] rx user pid 0x%x\n",proc->pid);
			}
		}
	}
	up(&vpp_netlink_receive_sem);         
}

void vpp_netlink_notify(int no,int cmd,int arg)
{
	int ret;
	int size;
	unsigned char *old_tail;
   	struct sk_buff *skb;
   	struct nlmsghdr *nlh;
	vpp_netlink_proc_t *proc;

//	DPRINT("[VPP] netlink notify %d,cmd %d,0x%x\n",no,cmd,arg);

	if( !(proc = vpp_netlink_get_proc(no)) )
		return;

	switch(cmd){
		case DEVICE_RX_DATA:
			size = NLMSG_SPACE(sizeof(struct wmt_cec_msg));  
			break;
		case DEVICE_PLUG_IN:
		case DEVICE_PLUG_OUT:
			size = NLMSG_SPACE(sizeof(0));  			
			break;
		default:
			return;
	}

	skb = alloc_skb(size, GFP_ATOMIC);
	if(skb==NULL) 
		return;
	old_tail = skb->tail;     
	nlh = NLMSG_PUT(skb, 0, 0, 0, size-sizeof(*nlh)); 
	nlh->nlmsg_len = skb->tail - old_tail;

	switch(cmd){
		case DEVICE_RX_DATA:
			nlh->nlmsg_type = DEVICE_RX_DATA;
			memcpy(NLMSG_DATA(nlh),(struct wmt_cec_msg *)arg,sizeof(struct wmt_cec_msg));
			break;
		case DEVICE_PLUG_IN:
		case DEVICE_PLUG_OUT:
			if( arg ){
				nlh->nlmsg_type = DEVICE_PLUG_IN;
				nlh->nlmsg_flags = edid_get_hdmi_phy_addr();
			}
			else {
				nlh->nlmsg_type = DEVICE_PLUG_OUT;
			}
			size = NLMSG_SPACE(sizeof(0));  			
			break;
		default:
			return;
	}
	
	NETLINK_CB(skb).pid = 0;
	NETLINK_CB(skb).dst_group = 0;

   	if(proc->pid!=0){
   		ret = netlink_unicast(vpp_nlfd, skb, proc->pid, MSG_DONTWAIT);
		return;
	}
nlmsg_failure: //NLMSG_PUT go to
	if(skb!=NULL)
		kfree_skb(skb);
   	return;
}

#endif

static DECLARE_MUTEX(vpp_sem);
void vpp_set_mutex(int lock)
{
	if( lock )
		down(&vpp_sem);
	else
		up(&vpp_sem);
}

#ifdef VPP_DEBUG
#define VPP_DBG_TMR_NUM		3
//#define VPP_DBG_DIAG_NUM	100
#ifdef VPP_DBG_DIAG_NUM
char vpp_dbg_diag_str[VPP_DBG_DIAG_NUM][100];
int vpp_dbg_diag_index;
int vpp_dbg_diag_delay;
#endif

/*!*************************************************************************
* vpp_dbg_show()
* 
* Private Function by Sam Shen, 2009/01/17
*/
/*!
* \brief	show debug message with time period
*		
* \retval  None
*/ 
void vpp_dbg_show(int level,int tmr,char *str)
{
	static struct timeval pre_tv[VPP_DBG_TMR_NUM];
	struct timeval tv;
	unsigned int tm_usec = 0;

	if( vpp_check_dbg_level(level)==0 )
		return;

	if( tmr && (tmr <= VPP_DBG_TMR_NUM) ){
		do_gettimeofday(&tv);
		if( pre_tv[tmr-1].tv_sec ){
			tm_usec = ( tv.tv_sec == pre_tv[tmr-1].tv_sec )? (tv.tv_usec - pre_tv[tmr-1].tv_usec):(1000000 + tv.tv_usec - pre_tv[tmr-1].tv_usec);
		}
		pre_tv[tmr-1] = tv;
	}

#ifdef VPP_DBG_DIAG_NUM
	if( level == VPP_DBGLVL_DIAG ){
		if( str ){
			char *ptr = &vpp_dbg_diag_str[vpp_dbg_diag_index][0];
			sprintf(ptr,"%s (%d,%d)(T%d %d usec)",str,(int)tv.tv_sec,(int)tv.tv_usec,tmr,(int) tm_usec);
			vpp_dbg_diag_index = (vpp_dbg_diag_index + 1) % VPP_DBG_DIAG_NUM;
		}

		if( vpp_dbg_diag_delay ){
			vpp_dbg_diag_delay--;
			if( vpp_dbg_diag_delay == 0 ){
				int i;
				
				DPRINT("----- VPP DIAG -----\n");
				for(i=0;i<VPP_DBG_DIAG_NUM;i++){
					DPRINT("%02d : %s\n",i,&vpp_dbg_diag_str[vpp_dbg_diag_index][0]);
					vpp_dbg_diag_index = (vpp_dbg_diag_index + 1) % VPP_DBG_DIAG_NUM;				
				}
			}
		}
		return;
	}
#endif
	
	if( str ) {
		if( tmr ){
			DPRINT("[VPP] %s (T%d period %d usec)\n",str,tmr-1,(int) tm_usec);
		}
		else {
			DPRINT("[VPP] %s\n",str);
		}
	}
} /* End of vpp_dbg_show */

static void vpp_dbg_show_val1(int level,int tmr,char *str,int val)
{
	if( vpp_check_dbg_level(level) ){
		char buf[50];

		sprintf(buf,"%s 0x%x",str,val);
		vpp_dbg_show(level,tmr,buf);
	}
}

static DECLARE_WAIT_QUEUE_HEAD(vpp_dbg_wq);
int vpp_dbg_wait_flag;
void vpp_dbg_wait(char *str)
{
	DPRINT("[VPP] vpp_dbg_wait(%s)\n",str);
	wait_event_interruptible(vpp_dbg_wq, (vpp_dbg_wait_flag));
	vpp_dbg_wait_flag = 0;
	DPRINT("[VPP] Exit vpp_dbg_wait\n");
}
#else
void vpp_dbg_show(int level,int tmr,char *str){}
static void vpp_dbg_show_val1(int level,int tmr,char *str,int val){}
void vpp_dbg_wait(char *str){}
#endif

/*!*************************************************************************
* vpp_i2c0_read()
* 
* Private Function by Sam Shen, 2009/02/26
*/
/*!
* \brief	read i2c bus0
*		
* \retval  None
*/ 
int vpp_i2c0_read(unsigned int addr,unsigned int index,char *pdata,int len) 
{
#ifdef CONFIG_I2C1_WMT
	extern int wmt_i2c_xfer_continue_if_3(struct i2c_msg *msg, unsigned int num, int bus_id);
#endif
	struct i2c_msg msg[2];
	unsigned char buf[len+1];

	addr = (addr >> 1);	
	memset(buf,0x55,len+1);
    buf[0] = index;
	buf[1] = 0x0;

    msg[0].addr = addr;
    msg[0].flags = 0 ;
	msg[0].flags &= ~(I2C_M_RD);
	msg[0].len = 1;
    msg[0].buf = buf;

	msg[1].addr = addr;
	msg[1].flags = 0 ;
	msg[1].flags |= (I2C_M_RD);
	msg[1].len = len;
	msg[1].buf = buf;

#ifdef CONFIG_I2C1_WMT
	wmt_i2c_xfer_continue_if_3(msg, 2, 1);
#endif
	memcpy(pdata,buf,len);
	return 0;
}

int vpp_i2c_bus_read(int id,unsigned int addr,unsigned int index,char *pdata,int len)
{
	int ret = 0;

//	DPRINT("[VPP] vpp_i2c_bus_read(%d,%x,%d,%d)\n",id,addr,index,len);

	switch(id){
		case 0 ... 0xF:	// hw i2c
			{
			unsigned char buf[len+1];
#ifdef CONFIG_I2C_WMT			
			struct i2c_msg msg[2];

			addr = (addr >> 1);	
			memset(buf,0x55,len+1);
		    buf[0] = index;
			buf[1] = 0x0;

		    msg[0].addr = addr;
		    msg[0].flags = 0 ;
			msg[0].flags &= ~(I2C_M_RD);
			msg[0].len = 1;
		    msg[0].buf = buf;

			msg[1].addr = addr;
			msg[1].flags = 0 ;
			msg[1].flags |= (I2C_M_RD);
			msg[1].len = len;
			msg[1].buf = buf;
			ret = wmt_i2c_xfer_continue_if_4(msg,2,id);
#endif
			memcpy(pdata,buf,len);
			}
			break;
		default:
			vo_i2c_proc((id & 0xF),(addr | BIT0),index,pdata,len);
			break;
	}
	return ret;
}

int vpp_i2c_bus_write(int id,unsigned int addr,unsigned int index,char *pdata,int len)
{
	int ret = 0;

//	DPRINT("[VPP] vpp_i2c_bus_write(%d,%x,%d,%d)\n",id,addr,index,len);
	
	switch(id){
		case 0 ... 0xF:	// hw i2c
#ifdef CONFIG_I2C_WMT
			{
		    struct i2c_msg msg[1];
			unsigned char buf[len+1];

			addr = (addr >> 1);
		    buf[0] = index;
			memcpy(&buf[1],pdata,len);
		    msg[0].addr = addr;
		    msg[0].flags = 0 ;
		    msg[0].flags &= ~(I2C_M_RD);
		    msg[0].len = len+1;
		    msg[0].buf = buf;

		    ret = wmt_i2c_xfer_continue_if_4(msg,1,id);
			}
#endif
			break;
		default:
			vo_i2c_proc((id & 0xF),(addr & ~BIT0),index,pdata,len);			
			break;
	}
	return ret;
}

/*----------------------- Linux Kernel feature --------------------------------------*/
static int __init vpp_get_boot_arg
(
	char *str			/*!<; // argument string */
)
{
	sscanf(str,"%d:%d:%d:%d:%d:%d",&vpp_vo_boot_arg[0],&vpp_vo_boot_arg[1],&vpp_vo_boot_arg[2],&vpp_vo_boot_arg[3],&vpp_vo_boot_arg[4],&vpp_vo_boot_arg[5]);
	switch( vpp_vo_boot_arg[0] ){
		case VOUT_SD_ANALOG:
		case VOUT_SD_DIGITAL:
		case VOUT_LCD:
		case VOUT_DVI:
		case VOUT_HDMI:
		case VOUT_DVO2HDMI:
		case VOUT_VGA:
			break;
		default:
			vpp_vo_boot_arg[0] = VOUT_MODE_MAX;
			return -1;
	}
	DPRINT("[VPP] vpp boot arg %s opt %d,%d, %dx%d@%d\n",vpp_vout_str[vpp_vo_boot_arg[0]],vpp_vo_boot_arg[1],vpp_vo_boot_arg[2],
														vpp_vo_boot_arg[3],vpp_vo_boot_arg[4],vpp_vo_boot_arg[5]);
  	return 1;
} /* End of lcd_arg_panel_id */
__setup("wmtvo=", vpp_get_boot_arg);

static int __init vpp_get_boot_arg2
(
	char *str			/*!<; // argument string */
)
{
	sscanf(str,"%d:%d:%d",&vpp_vo_boot_arg2[0],&vpp_vo_boot_arg2[1],&vpp_vo_boot_arg2[2]);
	switch( vpp_vo_boot_arg[0] ){
		case VOUT_SD_ANALOG:
		case VOUT_SD_DIGITAL:
		case VOUT_LCD:
		case VOUT_DVI:
		case VOUT_HDMI:
		case VOUT_DVO2HDMI:
		case VOUT_VGA:
			break;
		default:
			vpp_vo_boot_arg2[0] = VOUT_MODE_MAX;
			return -1;
	}
	DPRINT("[VPP] vpp boot arg2 %s opt %d,%d\n",vpp_vout_str[vpp_vo_boot_arg2[0]],vpp_vo_boot_arg2[1],vpp_vo_boot_arg2[2]);
  	return 1;
} /* End of lcd_arg_panel_id */
__setup("wmtvo2=", vpp_get_boot_arg2);

#ifdef CONFIG_PROC_FS
#define CONFIG_VPP_PROC
#ifdef CONFIG_VPP_PROC
unsigned int vpp_proc_value;
char vpp_proc_str[16];
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
static int vpp_do_proc(ctl_table * ctl,int write,void *buffer,size_t * len,loff_t *ppos)
#else
static int vpp_do_proc(ctl_table * ctl,int write,struct file *file,void *buffer,size_t * len,loff_t *ppos)
#endif
{	
	int ret;	

	if( !write ){
		switch( ctl->ctl_name ){
			case 1:
				vpp_proc_value = g_vpp.dbg_msg_level;
				break;
			case 5:
				vpp_proc_value = p_vpu->dei_mode;
				break;
#ifdef WMT_FTBLK_DISP
			case 6:
				vpp_proc_value = p_disp->dac_sense_val;
				break;
#endif
			case 7:
				vpp_proc_value = g_vpp.disp_fb_max;
				break;
#ifdef CONFIG_GOVW_FPS_AUTO_ADJUST
			case 8:
				vpp_proc_value = g_vpp.govw_tg_dynamic;
				break;
#endif
			case 9:
				vpp_proc_value = g_vpp.govw_skip_all;
				break;
			case 10:
				vpp_proc_value = g_vpp.video_quality_mode;
				break;
			case 11:
				vpp_proc_value = g_vpp.scale_keep_ratio;
				break;
			case 13:
				vpp_proc_value = g_vpp.disp_fb_keep;
				break;
			case 14:
				vpp_proc_value = g_vpp.govrh_field;
				break;
			case 15:
				vpp_proc_value = p_vpu->scale_mode;
				break;
			case 16:
				vpp_proc_value = p_scl->scale_mode;
				break;
			case 17:
				vpp_proc_value = p_vpu->skip_fb;
				break;
			case 18:
				vpp_proc_value = p_vpu->underrun_cnt;
				break;
			case 19:
				vpp_proc_value = g_vpp.vpu_skip_all;
				break;
			case 20:
				vpp_proc_value = g_vpp.govw_hfp;
				break;
			case 21:
				vpp_proc_value = g_vpp.govw_hbp;
				break;
			case 22:
				vpp_proc_value = g_vpp.govw_vfp;
				break;
			case 23:
				vpp_proc_value = g_vpp.govw_vbp;
				break;
			case 24:
				vpp_proc_value = g_vpp.hdmi_audio_interface;
				break;
			case 25:
				vpp_proc_value = g_vpp.hdmi_cp_enable;
				break;
			case 26:
				vpp_proc_value = vpp_get_base_clock(VPP_MOD_GOVRH);
				break;
			case 27:
#ifdef CONFIG_VPP_GOVRH_FBSYNC
				vpp_proc_value = g_vpp.fbsync_enable;
#endif
				break;
			case 28:
				vpp_proc_value = g_vpp.hdmi_ctrl;
				break;
			case 29:
				vpp_proc_value = g_vpp.hdmi_audio_pb4;
				break;
			case 30:
				vpp_proc_value = g_vpp.hdmi_audio_pb1;
				break;
			case 31:
				vpp_proc_value = g_vpp.hdmi_i2c_freq;
				break;
			case 32:
				vpp_proc_value = g_vpp.hdmi_i2c_udelay;
				break;
			case 34:
				vpp_proc_value = g_vpp.chg_res_blank;
				break;
#ifdef VPP_DEBUG
			case 35:
				vpp_proc_value = vpp_dbg_wait_flag;
				break;
#endif
			case 37:
				vpp_proc_value = edid_msg_enable;
				break;
			case 38:
				vpp_proc_value = g_vpp.dbg_flag;
				break;
			default:
				break;
		}
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
	ret = proc_dointvec(ctl, write, buffer, len, ppos);
#else
	ret = proc_dointvec(ctl, write, file, buffer, len, ppos);
#endif
	if( write ){
		switch( ctl->ctl_name ){
			case 1:
				DPRINT("---------- VPP debug level ----------\n");
				DPRINT("0-disable,255-show all\n");
				DPRINT("1-scale,2-disp fb,3-interrupt,4-TG\n");
				DPRINT("5-ioctl,6-diag,7-deinterlace\n");
				DPRINT("-------------------------------------\n");
				g_vpp.dbg_msg_level = vpp_proc_value;
				break;
#ifdef CONFIG_LCD_WMT			
			case 3:
				lcd_blt_set_level(lcd_blt_id,lcd_blt_level);
				break;
			case 4:
				lcd_blt_set_freq(lcd_blt_id,lcd_blt_freq);
				break;
#endif			
#ifdef WMT_FTBLK_VPU
			case 5:
				p_vpu->dei_mode = vpp_proc_value;
				vpu_dei_set_mode(p_vpu->dei_mode);
				break;
#endif
#ifdef WMT_FTBLK_DISP
			case 6:
				p_disp->dac_sense_val = vpp_proc_value;
				break;
#endif
			case 7:
				g_vpp.disp_fb_max = vpp_proc_value;
				break;
#ifdef CONFIG_GOVW_FPS_AUTO_ADJUST
			case 8:
				g_vpp.govw_tg_dynamic = vpp_proc_value;
				break;
#endif
			case 9:
				g_vpp.govw_skip_all = vpp_proc_value;
				break;
			case 10:
				g_vpp.video_quality_mode = vpp_proc_value;
				vpp_set_video_quality(g_vpp.video_quality_mode);
				break;
			case 11:
				g_vpp.scale_keep_ratio = vpp_proc_value;
				break;
#ifdef CONFIG_WMT_EDID
			case 12:
				{
					char *edid_buf;
					vout_t *vo;

					vo = vout_get_info(vpp_proc_value);
					vo->status &= ~VPP_VOUT_STS_EDID;
					if( (edid_buf = vout_get_edid(vpp_proc_value)) ){
						edid_dump(edid_buf);
						if( edid_parse(edid_buf) == 0 ){
							DPRINT("*E* parse EDID fail\n");
						}
					}
					else {
						DPRINT("*E* read EDID fail\n");
					}
				}
				break;
#endif
			case 13:
				g_vpp.disp_fb_keep = (vpp_proc_value > 3)? 3:vpp_proc_value;
				break;
			case 14:
				g_vpp.govrh_field = (g_vpp.govrh_field == VPP_FIELD_TOP)? VPP_FIELD_BOTTOM:VPP_FIELD_TOP;
				break;
			case 15:
			case 16:
				DPRINT("---------- scale mode ----------\n");
				DPRINT("0-recursive normal\n");
				DPRINT("1-recursive sw bilinear\n");
				DPRINT("2-recursive hw bilinear\n");
				DPRINT("3-realtime noraml (quality but x/32 limit)\n");
				DPRINT("4-realtime bilinear (fit edge but skip line)\n");
				DPRINT("-------------------------------------\n");
				if( ctl->ctl_name == 15 ){
					p_vpu->scale_mode = vpp_proc_value;
				}
				else {
					p_scl->scale_mode = vpp_proc_value;
				}
				break;
			case 17:
				p_vpu->skip_fb = vpp_proc_value;
				break;
			case 18:
				p_vpu->underrun_cnt = vpp_proc_value;
				g_vpp.vpu_skip_all = 0;
				g_vpp.govw_skip_all = 0;
				break;
			case 19:
				g_vpp.vpu_skip_all = vpp_proc_value;
				break;
			case 20:
				g_vpp.govw_hfp = vpp_proc_value;
				break;
			case 21:
				g_vpp.govw_hbp = vpp_proc_value;
				break;
			case 22:
				g_vpp.govw_vfp = vpp_proc_value;
				break;
			case 23:
				g_vpp.govw_vbp = vpp_proc_value;
				break;
			case 24:
				g_vpp.hdmi_audio_interface = vpp_proc_value;
				break;
			case 25:
				{
#ifdef WMT_FTBLK_HDMI
				hdmi_set_cp_enable(vpp_proc_value);
#endif				
				g_vpp.hdmi_cp_enable = vpp_proc_value;
				}
				break;
			case 26:
				govrh_set_clock(vpp_proc_value);
				DPRINT("[HDMI] set pixclk %d\n",vpp_proc_value);
				break;
			case 27:
#ifdef CONFIG_VPP_GOVRH_FBSYNC
				g_vpp.fbsync_enable = vpp_proc_value;
#endif
				break;
			case 28:
				g_vpp.hdmi_ctrl = vpp_proc_value;
				break;
			case 29:
				g_vpp.hdmi_audio_pb4 = vpp_proc_value;
				break;
			case 30:
				g_vpp.hdmi_audio_pb1 = vpp_proc_value;
				break;
			case 31:
				g_vpp.hdmi_i2c_freq	= vpp_proc_value;
				break;
			case 32:
				g_vpp.hdmi_i2c_udelay = vpp_proc_value;
				break;
#if 0
			case 33:
				{
					char *edid_buf;
					vout_t *vo;

					int i,err_read,err_parse1,err_parse2;
					extern int edid_check_block(char *edid);

					err_read = err_parse1 = err_parse2 = 0;
					g_vpp.dbg_hdmi_ddc_ctrl_err = 0;
					g_vpp.dbg_hdmi_ddc_read_err = 0;
					g_vpp.dbg_hdmi_ddc_crc_err = 0;
					vo = vout_get_info(VOUT_HDMI);
					for(i=0;i<vpp_proc_value;i++){
						vpp_lock();
						vo->status &= ~VPP_VOUT_STS_EDID;
						if( (edid_buf = vout_get_edid(VOUT_HDMI)) ){
							if( edid_check_block(edid_buf) ){
								err_parse1++;
								edid_dump(edid_buf);
							}
#if 0
							if( edid_check_block(&edid_buf[128]) ){
								err_parse2++;
								edid_dump(&edid_buf[128]);
							}
#endif
							if( i == 0 ){
								DPRINT("dump first EDID\n");
								edid_dump(edid_buf);
								edid_dump(&edid_buf[128]);
							}
						}
						else {
							err_read++;
						}
						vpp_unlock();
					}
					DPRINT("----- edid test result -----\n");
					DPRINT(" test cnt %d,rd err %d\n",vpp_proc_value,err_read);
					DPRINT(" parse err1 %d,parser err2 %d\n",err_parse1,err_parse2);
					DPRINT(" ctrl %d, read %d, crc %d\n",g_vpp.dbg_hdmi_ddc_ctrl_err
						,g_vpp.dbg_hdmi_ddc_read_err,g_vpp.dbg_hdmi_ddc_crc_err);
					DPRINT("----------------------------\n");
				}
				break;
#endif
			case 34:
				// smooth govrh fb when change resolution
				if( g_vpp.chg_res_blank && (vpp_proc_value == 0)){
					g_vpp.govw_skip_all = 0;
				}
				g_vpp.chg_res_blank = vpp_proc_value;
				break;
#ifdef VPP_DEBUG
			case 35:
				vpp_dbg_wait_flag = vpp_proc_value;
				wake_up(&vpp_dbg_wq);
				break;
#endif
			case 36:
				{
				spinlock_t vd_irqlock = SPIN_LOCK_UNLOCKED;
				unsigned long vd_lock_flags;

				spin_lock_irqsave(&vd_irqlock, vd_lock_flags);
				mdelay(vpp_proc_value);
				spin_unlock_irqrestore(&vd_irqlock, vd_lock_flags);
				}
				break;
			case 37:
				edid_msg_enable = vpp_proc_value;
				break;
			case 38:
				g_vpp.dbg_flag = vpp_proc_value;
				break;
			default:
				break;
		}
	}	
	return ret;
}

	struct proc_dir_entry *vpp_proc_dir = 0;

	static ctl_table vpp_table[] = {
	    {
			.ctl_name	= 1,
			.procname	= "dbg_msg",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
	    {
			.ctl_name	= 2,
			.procname	= "dac_sense_en",
			.data		= &vpp_dac_sense_enable,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &proc_dointvec,
		},
#ifdef CONFIG_LCD_BACKLIGHT_WMT
	    {
			.ctl_name	= 3,
			.procname	= "lcd_blt_level",
			.data		= &lcd_blt_level,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
	    {
			.ctl_name	= 4,
			.procname	= "lcd_blt_freq",
			.data		= &lcd_blt_freq,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
#endif
		{
			.ctl_name 	= 5,
			.procname	= "dei_mode",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 6,
			.procname	= "tv_dac_sense_val",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 7,
			.procname	= "disp_fb_max",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 8,
			.procname	= "govw_dynamic_fps",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 9,
			.procname	= "govw_skip_all",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 10,
			.procname	= "video_quality_mode",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 11,
			.procname	= "scale_keep_ratio",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 12,
			.procname	= "vout_edid",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 13,
			.procname	= "disp_fb_keep",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 14,
			.procname	= "govrh_cur_field",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 15,
			.procname	= "vpu_scale_mode",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 16,
			.procname	= "scl_scale_mode",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 17,
			.procname	= "vpu_err_skip",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 18,
			.procname	= "vpu_underrun_cnt",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 19,
			.procname	= "vpu_skip_all",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 20,
			.procname	= "govw_hfp",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 21,
			.procname	= "govw_hbp",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 22,
			.procname	= "govw_vfp",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 23,
			.procname	= "govw_vbp",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 24,
			.procname	= "hdmi_audio_interface",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 25,
			.procname	= "hdmi_cp_enable",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 26,
			.procname	= "pixel_clock",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 27,
			.procname	= "govrh_fbsync",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 28,
			.procname	= "hdmi_ctrl",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 29,
			.procname	= "hdmi_audio_pb4",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 30,
			.procname	= "hdmi_audio_pb1",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 31,
			.procname	= "hdmi_i2c_freq",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 32,
			.procname	= "hdmi_i2c_udelay",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 33,
			.procname	= "hdmi_i2c_test",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 34,
			.procname	= "chg_res_fb_clr",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 35,
			.procname	= "dbg_wait",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 36,
			.procname	= "vo_mode",
			.data		= vpp_proc_str,
			.maxlen		= 12,
			.mode		= 0666,
			.proc_handler = &proc_dostring,
		},
		{
			.ctl_name 	= 37,
			.procname	= "edid_msg",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{
			.ctl_name 	= 38,
			.procname	= "dbg_flag",
			.data		= &vpp_proc_value,
			.maxlen		= sizeof(int),
			.mode		= 0666,
			.proc_handler = &vpp_do_proc,
		},
		{ .ctl_name = 0 }
	};

	static ctl_table vpp_root_table[] = {
		{
			.ctl_name	= CTL_DEV,
			.procname	= "vpp",	// create path ==> /proc/sys/vpp
			.mode		= 0555,
			.child 		= vpp_table
		},
		{ .ctl_name = 0 }
	};
	static struct ctl_table_header *vpp_table_header;
#endif

/*!*************************************************************************
* vpp_sts_read_proc()
* 
* Private Function by Sam Shen, 2009/01/17
*/
/*!
* \brief	vpp stauts read proc
*		
* \retval  None
*/ 
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
static int vpp_sts_read_proc(char *buf, char **start, off_t offset, int len,int *eof,void *data)
#else
static int vpp_sts_read_proc(char *buf, char **start, off_t offset, int len)
#endif
{
	unsigned int yaddr,caddr;
	char *p = buf;
	static struct timeval pre_tv;
	struct timeval tv;
	unsigned int tm_usec;
	
	p += sprintf(p, "--- VPP HW status ---\n");
#ifdef WMT_FTBLK_GOVRH	
	p += sprintf(p, "GOVRH memory read underrun error %d,cnt %d\n",(vppif_reg32_in(REG_GOVRH_INT) & 0x200)?1:0,g_vpp.govr_underrun_cnt);
	p_govrh->clr_sts(VPP_INT_ALL);
#endif

#ifdef WMT_FTBLK_GOVW
	p += sprintf(p, "---------------------------------------\n");
	p += sprintf(p, "GOVW TG error %d,cnt %d\n",vppif_reg32_read(GOVW_INTSTS_TGERR),g_vpp.govw_tg_err_cnt);
	p += sprintf(p, "GOVW Y fifo overflow %d\n",vppif_reg32_read(GOVW_INTSTS_MIFYERR));
	p += sprintf(p, "GOVW C fifo overflow %d\n",vppif_reg32_read(GOVW_INTSTS_MIFCERR));
	p_govw->clr_sts(VPP_INT_ALL);
#endif

#ifdef WMT_FTBLK_GOVM
	p += sprintf(p, "---------------------------------------\n");
	p += sprintf(p, "GOVM VPU not ready %d,cnt %d\n",(vppif_reg32_read(GOVM_INTSTS_VPU_READY))?0:1,g_vpp.govw_vpu_not_ready_cnt);
	p += sprintf(p, "GOVM GE not ready %d,cnt %d\n",(vppif_reg32_read(GOVM_INTSTS_GE_READY))?0:1,g_vpp.govw_ge_not_ready_cnt);
	p += sprintf(p, "GE not ready G1 %d, G2 %d\n",vppif_reg32_read(GE1_BASE_ADDR+0xF4,BIT0,0),vppif_reg32_read(GE1_BASE_ADDR+0xF4,BIT1,1));
	REG32_VAL(GE1_BASE_ADDR+0xf4) |= 0x3;
#ifdef WMT_FTBLK_PIP
	p += sprintf(p, "GOVM PIP not ready %d\n",(vppif_reg32_read(GOVM_INTSTS_PIP_READY))?0:1);
	p += sprintf(p, "GOVM PIP Y error %d\n",vppif_reg32_read(GOVM_INT_PIP_Y_ERR));
	p += sprintf(p, "GOVM PIP C error %d\n",vppif_reg32_read(GOVM_INT_PIP_C_ERR));
#endif
	p_govm->clr_sts(VPP_INT_ALL);
#endif

#ifdef WMT_FTBLK_SCL	
	p += sprintf(p, "---------------------------------------\n");
	p += sprintf(p, "SCL TG error %d\n",vppif_reg32_read(SCL_INTSTS_TGERR));
	p += sprintf(p, "SCLR MIF1 read error %d\n",vppif_reg32_read(SCLR_INTSTS_R1MIFERR));
	p += sprintf(p, "SCLR MIF2 read error %d\n",vppif_reg32_read(SCLR_INTSTS_R2MIFERR));
	p += sprintf(p, "SCLW RGB fifo overflow %d\n",vppif_reg32_read(SCLW_INTSTS_MIFRGBERR));
	p += sprintf(p, "SCLW Y fifo overflow %d\n",vppif_reg32_read(SCLW_INTSTS_MIFYERR));
	p += sprintf(p, "SCLW C fifo overflow %d\n",vppif_reg32_read(SCLW_INTSTS_MIFCERR));
	p_scl->clr_sts(VPP_INT_ALL);
#endif

#ifdef WMT_FTBLK_VPU
	p += sprintf(p, "---------------------------------------\n");
	p += sprintf(p, "VPU TG error %d\n",vppif_reg32_read(VPU_INTSTS_TGERR));
	p += sprintf(p, "VPUR MIF1 read error %d\n",vppif_reg32_read(VPU_R_INTSTS_R1MIFERR));
	p += sprintf(p, "VPUR MIF2 read error %d\n",vppif_reg32_read(VPU_R2_MIF_ERR));	
	p += sprintf(p, "VPUW Y fifo overflow %d,cnt %d\n",vppif_reg32_read(VPU_W_MIF_YERR),g_vpp.vpu_y_err_cnt);
//	p += sprintf(p, "VPUW C fifo overflow %d,cnt %d\n",vppif_reg32_read(VPU_W_MIF_CERR),g_vpp.vpu_c_err_cnt);
	p += sprintf(p, "VPU scale underrun %d,cnt %d\n",vppif_reg32_read(VPU_W_MIF_CERR),g_vpp.vpu_c_err_cnt);
	p += sprintf(p, "VPUW RGB fifo overflow %d\n",vppif_reg32_read(VPU_W_MIF_RGBERR));
	p += sprintf(p, "VPU MVR fifo overflow %d\n",vppif_reg32_read(VPU_MVR_MIF_ERR));
	p_vpu->clr_sts(VPP_INT_ALL);
#endif

	if( REG32_VAL(GE3_BASE_ADDR+0x50) < vppif_reg32_read(GOVM_DISP_X_CR) ){
		p += sprintf(p, "*E* GE resx %d < GOV resx %d\n",REG32_VAL(GE3_BASE_ADDR+0x50),vppif_reg32_read(GOVM_DISP_X_CR));
	}
	if( REG32_VAL(GE3_BASE_ADDR+0x54) < vppif_reg32_read(GOVM_DISP_Y_CR) ){
		p += sprintf(p, "*E* GE resy %d < GOV resy %d\n",REG32_VAL(GE3_BASE_ADDR+0x54),vppif_reg32_read(GOVM_DISP_Y_CR));
	}

	p += sprintf(p, "---------------------------------------\n");
	p += sprintf(p, "(6A8.0)G1 Enable %d,(6AC.0)G2 Enable %d\n",REG32_VAL(GE3_BASE_ADDR+0xa8),REG32_VAL(GE3_BASE_ADDR+0xac));
	p += sprintf(p, "(880.0)GOVRH Enable %d,(900.0)TG %d\n",vppif_reg32_read(GOVRH_MIF_ENABLE),vppif_reg32_read(GOVRH_TG_ENABLE));
	p += sprintf(p, "(C84.8)GOVW Enable %d,(CA0.0)TG %d\n",vppif_reg32_read(GOVW_HD_MIF_ENABLE),vppif_reg32_read(GOVW_TG_ENABLE));
	p += sprintf(p, "(308.0)GOVM path GE %d,(30C.0) VPU %d\n",vppif_reg32_read(GOVM_GE_SOURCE),vppif_reg32_read(GOVM_VPU_SOURCE));	
	p += sprintf(p, "(1C0.0)VPUR1 Enable %d,(1C0.1)R2 Enable %d\n",vppif_reg32_read(VPU_R_MIF_ENABLE),vppif_reg32_read(VPU_R_MIF2_ENABLE));
	p += sprintf(p, "(1E0.0)VPUW Enable %d,(1A0.0)TG %d\n",vppif_reg32_read(VPU_W_MIF_EN),vppif_reg32_read(VPU_TG_ENABLE));	

	p += sprintf(p, "--- VPP fb Address ---\n");
	
	p += sprintf(p, "GOV mb 0x%x 0x%x\n",g_vpp.mb[0],g_vpp.mb[1]);	
#ifdef WMT_FTBLK_VPU
	vpu_r_get_fb_addr(&yaddr,&caddr);
	p += sprintf(p, "VPU fb addr Y(0x%x) 0x%x, C(0x%x) 0x%x\n",REG_VPU_R_Y1SA,yaddr,REG_VPU_R_C1SA,caddr);
#else
	sclr_get_fb_addr(&yaddr,&caddr);
	p += sprintf(p, "VPU fb addr Y(0x%x) 0x%x, C(0x%x) 0x%x\n",REG_SCLR_YSA,yaddr,REG_SCLR_CSA,caddr);
#endif

#ifdef WMT_FTBLK_GOVW
	govw_get_hd_fb_addr(&yaddr,&caddr);
	p += sprintf(p, "GOVW fb addr Y(0x%x) 0x%x, C(0x%x) 0x%x\n",REG_GOVW_HD_YSA,yaddr,REG_GOVW_HD_CSA,caddr);	
#endif
#ifdef WMT_FTBLK_GOVRH
	govrh_get_fb_addr(&yaddr,&caddr);
	p += sprintf(p, "GOVRH fb addr Y(0x%x) 0x%x, C(0x%x) 0x%x\n",REG_GOVRH_YSA,yaddr,REG_GOVRH_CSA,caddr);	
#endif
	p += sprintf(p, "--- VPP SW status ---\n");
	p += sprintf(p, "vpp path %s\n",vpp_vpath_str[g_vpp.vpp_path]);
	
	do_gettimeofday(&tv);
	tm_usec=(tv.tv_sec==pre_tv.tv_sec)? (tv.tv_usec-pre_tv.tv_usec):(1000000*(tv.tv_sec-pre_tv.tv_sec)+tv.tv_usec-pre_tv.tv_usec);
	p += sprintf(p, "Time period %d usec\n",(int) tm_usec);
	p += sprintf(p, "GOVR fps %d,GOVW fps %d\n",(1000000*g_vpp.govrh_vbis_cnt/tm_usec),(1000000*g_vpp.govw_vbis_cnt/tm_usec));
	pre_tv = tv;
	
	p += sprintf(p, "GOVW VBIS INT cnt %d\n",g_vpp.govw_vbis_cnt);
	p += sprintf(p, "GOVW PVBI INT cnt %d (toggle dual buf)\n",g_vpp.govw_pvbi_cnt);
	p += sprintf(p, "GOVW TG ERR INT cnt %d\n",g_vpp.govw_tg_err_cnt);

	p += sprintf(p, "--- disp fb status ---\n");
	p += sprintf(p, "DISP fb isr cnt %d\n",g_vpp.disp_fb_isr_cnt);
	p += sprintf(p, "queue max %d,full cnt %d\n",g_vpp.disp_fb_max,g_vpp.disp_fb_full_cnt);
	p += sprintf(p, "VPU disp fb cnt %d, skip %d\n",g_vpp.disp_cnt,g_vpp.disp_skip_cnt);
#ifdef WMT_FTBLK_PIP
	p += sprintf(p, "PIP disp fb cnt %d\n",g_vpp.pip_disp_cnt);
	p += sprintf(p, "Queue cnt disp:%d,pip %d\n",vpp_disp_fb_cnt(&vpp_disp_fb_list),vpp_disp_fb_cnt(&vpp_pip_fb_list));
	g_vpp.pip_disp_cnt = 0;	
#else
	p += sprintf(p, "Queue cnt disp:%d\n",vpp_disp_fb_cnt(&vpp_disp_fb_list));
#endif
#ifdef PATCH_VPU_HALT_GOVW_TG_ERR
	p += sprintf(p, "GOVW TG err patch cnt %d,step %d\n",vpp_govw_tg_err_patch_cnt,vpp_govw_tg_err_patch);
#endif
	g_vpp.govrh_vbis_cnt = 0;
	g_vpp.govw_vbis_cnt = 0;
	g_vpp.govw_pvbi_cnt = 0;
	g_vpp.disp_fb_isr_cnt = 0;
	g_vpp.disp_fb_full_cnt = 0;
	g_vpp.disp_cnt = 0;
	g_vpp.govw_tg_err_cnt = 0;
	g_vpp.disp_skip_cnt = 0;
	g_vpp.govw_vpu_not_ready_cnt = 0;
	g_vpp.govw_ge_not_ready_cnt = 0;
	g_vpp.vpu_y_err_cnt = 0;
	g_vpp.vpu_c_err_cnt = 0;
	g_vpp.govr_underrun_cnt = 0;

#ifdef CONFIG_SCL_DIRECT_PATH_DEBUG
	p += sprintf(p, "--- scl fb ---\n");
	p += sprintf(p, "fb cnt %d,Avg %d ms\n",g_vpp.scl_fb_cnt,(g_vpp.scl_fb_cnt)? (g_vpp.scl_fb_tmr/g_vpp.scl_fb_cnt):0);
	p += sprintf(p, "fb mb in use %d,over %d\n",g_vpp.scl_fb_mb_used,g_vpp.scl_fb_mb_over);
	p += sprintf(p, "In queue %d\n",vpp_disp_fb_cnt(&vpp_scl_fb_list));

	g_vpp.scl_fb_cnt = 0;
	g_vpp.scl_fb_tmr = 0;
	g_vpp.scl_fb_mb_over = 0;
#endif
	return (p - buf);
} /* End of vpp_sts_read_proc */

/*!*************************************************************************
* vpp_reg_read_proc()
* 
* Private Function by Sam Shen, 2009/01/17
*/
/*!
* \brief	vpp register read proc
*		
* \retval  None
*/ 
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
static int vpp_reg_read_proc(char *buf, char **start, off_t offset, int len,int *eof,void *data)
#else
static int vpp_reg_read_proc(char *buf,char **start,off_t offset,int len)
#endif
{
	char *p = buf;
	vpp_mod_base_t *mod_p;
	int i;

	DPRINT("Product ID:0x%x\n",vpp_get_chipid());
	for(i=0;i<VPP_MOD_MAX;i++){
		mod_p = vpp_mod_get_base(i);
		if( mod_p && mod_p->dump_reg ){
			mod_p->dump_reg();
		}
	}
#ifdef 	WMT_FTBLK_HDMI
	hdmi_reg_dump();
#endif
#ifdef WMT_FTBLK_LVDS
	lvds_reg_dump();
#endif
	
//	p += sprintf(p, "Dump VPP HW register by kernel message\n");
	
	return (p-buf);
} /* End of vpp_reg_read_proc */

static char *vpp_show_module(vpp_mod_t mod,char *p)
{
	vpp_mod_base_t *mod_p;
	vpp_fb_base_t *fb_p;
	vdo_framebuf_t *fb;

	mod_p = vpp_mod_get_base(mod);
	p += sprintf(p, "int catch 0x%x\n",mod_p->int_catch);
	
	fb_p = mod_p->fb_p;
	if( fb_p ){
		fb = &fb_p->fb;
		p += sprintf(p, "----- frame buffer -----\n");
		p += sprintf(p, "Y addr 0x%x, size %d\n",fb->y_addr,fb->y_size);
		p += sprintf(p, "C addr 0x%x, size %d\n",fb->c_addr,fb->c_size);
		p += sprintf(p, "W %d, H %d, FB W %d, H %d\n",fb->img_w,fb->img_h,fb->fb_w,fb->fb_h);
		p += sprintf(p, "bpp %d, color fmt %s\n",fb->bpp,vpp_colfmt_str[fb->col_fmt]);
		p += sprintf(p, "H crop %d, V crop %d\n",fb->h_crop,fb->v_crop);
		p += sprintf(p, "flag 0x%x\n",fb->flag);
		p += sprintf(p, "CSC mode %d,frame rate %d\n",fb_p->csc_mode,fb_p->framerate);
		p += sprintf(p, "media fmt %d,wait ready %d\n",fb_p->media_fmt,fb_p->wait_ready);
	}
	return p;
}

/*!*************************************************************************
* vpp_info_read_proc()
* 
* Private Function by Sam Shen, 2009/01/17
*/
/*!
* \brief	vpp infomation read proc
*		
* \retval  None
*/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
static int vpp_info_read_proc(char *buf, char **start, off_t offset, int len,int *eof,void *data)
#else
static int vpp_info_read_proc(char *buf,char **start,off_t offset,int len)
#endif
{
	char *p = buf;

	p += sprintf(p, "========== VPP ==========\n");
	p += sprintf(p, "vpp path %s\n",vpp_vpath_str[g_vpp.vpp_path]);
	p += sprintf(p, "mb0 0x%x,mb1 0x%x\n",g_vpp.mb[0],g_vpp.mb[1]);

#ifdef WMT_FTBLK_GOVRH
	p += sprintf(p, "========== GOVRH ==========\n");
	p += sprintf(p, "VGA DAC SENSE cnt %d\n",p_govrh->vga_dac_sense_cnt);
	p = vpp_show_module(VPP_MOD_GOVRH,p);
#endif

	p += sprintf(p, "========== GOVW ==========\n");
	p = vpp_show_module(VPP_MOD_GOVW,p);

	p += sprintf(p, "========== GOVM ==========\n");
	p += sprintf(p, "path 0x%x\n",p_govm->path);
	p = vpp_show_module(VPP_MOD_GOVM,p);

	p += sprintf(p, "========== VPU ==========\n");
	p += sprintf(p, "visual res (%d,%d),pos (%d,%d)\n",p_vpu->resx_visual,p_vpu->resy_visual,p_vpu->posx,p_vpu->posy);
	p = vpp_show_module(VPP_MOD_VPU,p);

	p += sprintf(p, "========== SCLR ==========\n");
	p = vpp_show_module(VPP_MOD_SCL,p);

	p += sprintf(p, "========== SCLW ==========\n");
	p = vpp_show_module(VPP_MOD_SCLW,p);
	return (p-buf);
} /* End of vpp_info_read_proc */
#endif

/*!*************************************************************************
* vpp_set_audio()
* 
* Private Function by Sam Shen, 2010/08/05
*/
/*!
* \brief	set audio parameters
*		
* \retval  None
*/ 
int vpp_set_audio(int format,int sample_rate,int channel)
{
	vout_audio_t info;

	DPRINT("[VPP] set audio(fmt %d,rate %d,ch %d)\n",format,sample_rate,channel);
	info.fmt = format;
	info.sample_rate = sample_rate;
	info.channel = channel;
	return vout_set_audio(&info);
}

/*!*************************************************************************
* vpp_get_info()
* 
* Private Function by Sam Shen, 2009/08/06
*/
/*!
* \brief	get current vpp info
*		
* \retval  None
*/ 
void vpp_get_info(struct fb_var_screeninfo *var)
{
	var->xres = vfb_var.xres;
	var->yres = vfb_var.yres;
	var->xres_virtual = var->xres;
	var->yres_virtual = var->yres;
//	var->pixclock = vfb_var.pixclock;
	var->pixclock = vpp_get_base_clock(VPP_MOD_GOVRH) / 1000;
	var->pixclock = KHZ2PICOS(var->pixclock);
//	DPRINT("%s(%dx%d,%d)\n",__FUNCTION__,var->xres,var->yres,var->pixclock);
}

/*!*************************************************************************
* vpp_fbsync_cal_fps()
* 
* Private Function by Sam Shen, 2010/10/28
*/
/*!
* \brief	calucate govrh fb sync with media fps
*		
* \retval  None
*/ 
#ifdef CONFIG_VPP_GOVRH_FBSYNC
void vpp_fbsync_cal_fps(void)
{
	unsigned int gcd,total,frame;

	gcd = vpp_get_gcd(p_govrh->fb_p->framerate,p_govw->fb_p->framerate);
	total = p_govrh->fb_p->framerate / gcd;
	frame = p_govw->fb_p->framerate / gcd;

	if( frame > total ) frame = total;
	g_vpp.fbsync_frame = frame;
	g_vpp.fbsync_step = total / frame;
	g_vpp.fbsync_substep = ( total % frame )? (frame / (total % frame)):0;
	if( g_vpp.fbsync_step == g_vpp.fbsync_substep ){
		g_vpp.fbsync_step = g_vpp.fbsync_step + 1;
		g_vpp.fbsync_substep = total - (g_vpp.fbsync_step * frame);
	}
	
	g_vpp.fbsync_cnt = 1;
	g_vpp.fbsync_vsync = g_vpp.fbsync_step;
	if( vpp_check_dbg_level(VPP_DBGLVL_SYNCFB) ){
		char buf[50];

		DPRINT("[VPP] govrh fps %d,govw fps %d,gcd %d\n",p_govrh->fb_p->framerate,p_govw->fb_p->framerate,gcd);
		DPRINT("[VPP] total %d,frame %d\n",total,frame);
		sprintf(buf,"sync frame %d,step %d,sub %d",g_vpp.fbsync_frame,g_vpp.fbsync_step,g_vpp.fbsync_substep);
		vpp_dbg_show(VPP_DBGLVL_SYNCFB,3,buf);
	}

	// patch for pre interrupt interval not enough bug
	if(vppif_reg32_read(GOVRH_PVBI_LINE) < 0x4){
		vppif_reg32_write(GOVRH_PVBI_LINE,0x4);
	}
}
#endif

/*!*************************************************************************
* vpp_disp_fb_cnt()
* 
* Private Function by Sam Shen, 2009/03/05
*/
/*!
* \brief	clear display frame buffer queue
*		
* \retval  0 - success
*/ 
int vpp_disp_fb_cnt(struct list_head *list)
{
	struct list_head *ptr;
	int cnt;
	
	vpp_lock();
	cnt = 0;
	ptr = list;
	while( ptr->next != list ){
		ptr = ptr->next;
		cnt++;
	}
	vpp_unlock();
	return cnt;
}

/*!*************************************************************************
* vpp_disp_fb_compare()
* 
* Private Function by Sam Shen, 2010/11/25
*/
/*!
* \brief	compare two frame buffer info without Y/C address and size
*		
* \retval  0 - not match, 1 - same
*/ 
int vpp_disp_fb_compare(vdo_framebuf_t *fb1,vdo_framebuf_t *fb2)
{
	if( fb1->img_w != fb2->img_w ) return 0;
	if( fb1->img_h != fb2->img_h ) return 0;
	if( fb1->fb_w != fb2->fb_w ) return 0;
	if( fb1->fb_h != fb2->fb_h ) return 0;
	if( fb1->col_fmt != fb2->col_fmt ) return 0;
	if( fb1->h_crop != fb2->h_crop ) return 0;
	if( fb1->v_crop != fb2->v_crop ) return 0;
	if( fb1->flag != fb2->flag ) return 0;
	return 1;
} /* End of vpp_disp_fb_compare */

/*!*************************************************************************
* vpp_disp_fb_add()
* 
* Private Function by Sam Shen, 2009/02/02
*/
/*!
* \brief	add display frame to display queue
*		
* \retval  None
*/ 
static int vpp_disp_fb_add
(
	vpp_dispfb_t *fb		/*!<; // display frame pointer */
)
{
	vpp_dispfb_parm_t *entry;
	struct list_head *ptr;
	unsigned int yaddr,caddr;
	struct list_head *fb_list;

#ifdef CONFIG_GOVW_FPS_AUTO_ADJUST
	if( g_vpp.govw_tg_dynamic ){
		g_vpp.govw_tg_rtn_cnt = 0;
		vpp_govw_dynamic_tg_set_rcyc(g_vpp.govw_tg_rcyc);
	}
#endif

#ifdef CONFIG_SCL_DIRECT_PATH
	if( fb->flag & VPP_FLAG_DISPFB_SCL ){
		if( list_empty(&vpp_scl_free_list) ){
			return -1;
		}
	}
	else {
		if( list_empty(&vpp_disp_free_list) ){
			return -1;
		}
	}
#else
	if( list_empty(&vpp_disp_free_list) ){
		return -1;
	}
#endif

	vpp_lock();
	ptr = vpp_disp_free_list.next;

#ifdef CONFIG_SCL_DIRECT_PATH
	if( fb->flag & VPP_FLAG_DISPFB_SCL ){
		fb_list = &vpp_scl_fb_list;
		fb->flag &= ~VPP_FLAG_DISPFB_SCL;
		ptr = vpp_scl_free_list.next;
	} else 
#endif
	if( (fb->flag & VPP_FLAG_DISPFB_PIP) == 0 ){
		fb_list = &vpp_disp_fb_list;
		if( g_vpp.disp_fb_cnt >= g_vpp.disp_fb_max ){
			if( vpp_check_dbg_level(VPP_DBGLVL_DISPFB) ){
				char buf[50];

				sprintf(buf,"*W* add dispfb full max %d cnt %d",g_vpp.disp_fb_max,g_vpp.disp_fb_cnt);
				vpp_dbg_show(VPP_DBGLVL_DISPFB,1,buf);	
			}
			g_vpp.disp_fb_full_cnt++;
			vpp_unlock();
			return -1;
		}
		g_vpp.disp_fb_cnt++;
	}
#ifdef WMT_FTBLK_PIP
	else {
		fb_list = &vpp_pip_fb_list;
	}
#else
	else {
		vpp_unlock();
		return 0;
	}
#endif

	entry = list_entry(ptr,vpp_dispfb_parm_t,list);
	list_del_init(ptr);
	entry->parm = *fb;

#if 1	// patch for VPU bilinear mode
	if( ((fb->flag & VPP_FLAG_DISPFB_PIP) == 0) && !(g_vpp.vpp_path)){
		if(entry->parm.flag & VPP_FLAG_DISPFB_INFO){
			if( vpp_disp_fb_compare(&entry->parm.info,&p_vpu->fb_p->fb) ){
				entry->parm.flag &= ~VPP_FLAG_DISPFB_INFO;
				entry->parm.flag |= VPP_FLAG_DISPFB_ADDR;
				entry->parm.yaddr = entry->parm.info.y_addr;
				entry->parm.caddr = entry->parm.info.c_addr;
			}
		}
	}
#endif	

	memcpy(&entry->pts,&g_vpp.frame_pts,sizeof(vpp_pts_t));
	list_add_tail(&entry->list,fb_list);

#ifdef CONFIG_VPP_PATH_SWITCH_SCL_GOVR
	if( fb->flag & VPP_FLAG_DISPFB_MB_GOVR ){
		fb->flag &= ~VPP_FLAG_DISPFB_MB_GOVR;
		vpp_unlock();
		return 0;
	}
#endif
	yaddr = caddr = 0;
	if( entry->parm.flag & VPP_FLAG_DISPFB_ADDR ){
		yaddr = entry->parm.yaddr;
		caddr = entry->parm.caddr;
	}
	else if(entry->parm.flag & VPP_FLAG_DISPFB_INFO){
		yaddr = entry->parm.info.y_addr;
		caddr = entry->parm.info.c_addr;
	}

	if( (entry->parm.flag & VPP_FLAG_DISPFB_MB_NO) == 0 ){
		if( yaddr ){
			mb_get(yaddr);
		}
		
		if( caddr && !(entry->parm.flag & VPP_FLAG_DISPFB_MB_ONE)){
			mb_get(caddr);
		}
	}

	if( vpp_check_dbg_level(VPP_DBGLVL_DISPFB) ){
		char buf[50];

		if(fb_list == &vpp_disp_fb_list){
			sprintf(buf,"add %s Y 0x%x C 0x%x(Q %d/%d,E %d/%d)",(fb_list == &vpp_disp_fb_list)?"dispfb":"sclfb",yaddr,caddr,
				g_vpp.disp_fb_cnt,g_vpp.disp_fb_max,g_vpp.disp_cnt,g_vpp.disp_fb_isr_cnt);
		}
		else {
			sprintf(buf,"add %s Y 0x%x C 0x%x(Q %d/%d,MB %d/%d,cnt %d,I %d,W %d)\n","sclfb",yaddr,caddr,
				vpp_disp_fb_cnt(&vpp_scl_fb_list),VPP_SCL_FB_MAX,g_vpp.scl_fb_mb_used,VPP_SCL_MB_MAX,
				g_vpp.scl_fb_cnt,g_vpp.scl_fb_mb_index,g_vpp.scl_fb_mb_on_work);
		}
		vpp_dbg_show(VPP_DBGLVL_DISPFB,1,buf);
	}
	
	vpp_unlock();
	return 0;
} /* End of vpp_disp_fb_add */

void vpp_disp_fb_free(unsigned int y_addr,unsigned int c_addr)
{
	#define FREE_QUEUE_MAX	4
	static int cnt = 0;
	static unsigned int y_addr_queue[FREE_QUEUE_MAX];
	static unsigned int c_addr_queue[FREE_QUEUE_MAX];
	int i;

	if( y_addr == 0 ){
		for(i=0;i<cnt;i++){
			y_addr = y_addr_queue[i];
			c_addr = c_addr_queue[i];
			if( y_addr ) mb_put(y_addr);
			if( c_addr ) mb_put(c_addr);
			y_addr_queue[i] = c_addr_queue[i] = 0;
		}
		cnt = 0;
	}
	else {
		y_addr_queue[cnt] = y_addr;
		c_addr_queue[cnt] = c_addr;
		cnt++;		
		if( cnt > g_vpp.disp_fb_keep ){
			y_addr = y_addr_queue[0];
			c_addr = c_addr_queue[0];
			cnt--;
			
			for(i=0;i<cnt;i++){
				y_addr_queue[i] = y_addr_queue[i+1];
				c_addr_queue[i] = c_addr_queue[i+1];
			}

			if( y_addr ) mb_put(y_addr);
			if( c_addr ) mb_put(c_addr);
			if( vpp_check_dbg_level(VPP_DBGLVL_DISPFB) ){
				char buf[50];

				sprintf(buf,"free dispfb Y 0x%x C 0x%x,keep %d",y_addr,c_addr,cnt);
				vpp_dbg_show(VPP_DBGLVL_DISPFB,2,buf);
			}
		}
	}
}

/*!*************************************************************************
* vpp_disp_fb_clr()
* 
* Private Function by Sam Shen, 2009/03/05
*/
/*!
* \brief	clear display frame buffer queue
*		
* \retval  0 - success
*/ 
static int vpp_disp_fb_clr(int pip,int chk_view)
{
	vpp_dispfb_parm_t *entry;
	struct list_head *fb_list,*ptr;
	unsigned int yaddr,caddr;

	vpp_lock();

	if( vpp_check_dbg_level(VPP_DBGLVL_DISPFB) ){
		DPRINT("[VPP] %s\n",__FUNCTION__);
	}

#ifdef WMT_FTBLK_PIP	
	fb_list = (pip)? &vpp_pip_fb_list:&vpp_disp_fb_list;
	yaddr = (pip)? p_pip->pre_yaddr:vpp_pre_dispfb_y_addr;
	caddr = (pip)? p_pip->pre_caddr:vpp_pre_dispfb_c_addr;

	if( yaddr ) mb_put(yaddr);
	if( caddr )	mb_put(caddr);
	if( pip ){
		p_pip->pre_yaddr = 0;
		p_pip->pre_caddr = 0;
	}
	else {
		vpp_disp_fb_free(0,0);
		vpp_pre_dispfb_y_addr = 0;
		vpp_pre_dispfb_c_addr = 0;
		g_vpp.disp_fb_cnt = 0;
	}
#else
	fb_list = &vpp_disp_fb_list;
	yaddr = vpp_pre_dispfb_y_addr;
	caddr = vpp_pre_dispfb_c_addr;
	vpp_pre_dispfb_y_addr = 0;
	vpp_pre_dispfb_c_addr = 0;
	if( yaddr ) mb_put(yaddr);
	if( caddr ) mb_put(caddr);
	if( yaddr && vpp_check_dbg_level(VPP_DBGLVL_DISPFB) ){
		char buf[50];

		sprintf(buf,"free dispfb Y 0x%x C 0x%x",yaddr,caddr);
		vpp_dbg_show(VPP_DBGLVL_DISPFB,2,buf);
	}
	g_vpp.disp_fb_cnt = 0;		
#endif

	while( !list_empty(fb_list) ){
		ptr = fb_list->next;
		entry = list_entry(ptr,vpp_dispfb_parm_t,list);
		list_del_init(ptr);
		list_add_tail(&entry->list,&vpp_disp_free_list);
		if( entry->parm.flag & VPP_FLAG_DISPFB_ADDR ){
			yaddr = entry->parm.yaddr;
			caddr = entry->parm.caddr;
		}
		else if(entry->parm.flag & VPP_FLAG_DISPFB_INFO){
			yaddr = entry->parm.info.y_addr;
			caddr = entry->parm.info.c_addr;
		}

		if( chk_view ){
			if( entry->parm.flag & VPP_FLAG_DISPFB_ADDR ){
				vpp_fb_base_t *mod_fb_p;
				
				yaddr = entry->parm.yaddr;
				caddr = entry->parm.caddr;
				mod_fb_p = vpp_mod_get_fb_base(VPP_MOD_VPU);
				mod_fb_p->set_addr(yaddr,caddr);
				mod_fb_p->fb.y_addr = yaddr;
				mod_fb_p->fb.c_addr = caddr;
			}

			if( entry->parm.flag & VPP_FLAG_DISPFB_INFO ){
				vpp_fb_base_t *mod_fb_p;

				mod_fb_p = vpp_mod_get_fb_base(VPP_MOD_VPU);
				yaddr = entry->parm.info.y_addr;
				caddr = entry->parm.info.c_addr;
				mod_fb_p->fb = entry->parm.info;
				mod_fb_p->set_framebuf(&mod_fb_p->fb);
			}
			
			if( entry->parm.flag & VPP_FLAG_DISPFB_VIEW ){
				vpp_set_video_scale(&entry->parm.view);
			}
		}

		if( (entry->parm.flag & VPP_FLAG_DISPFB_MB_NO) == 0 ){
			if( yaddr ){
				mb_put(yaddr);
			}
			
			if( caddr && !(entry->parm.flag & VPP_FLAG_DISPFB_MB_ONE)){
				mb_put(caddr);
			}
			
			if( yaddr && vpp_check_dbg_level(VPP_DBGLVL_DISPFB) ){
				char buf[50];

				sprintf(buf,"free dispfb Y 0x%x C 0x%x",yaddr,caddr);
				vpp_dbg_show(VPP_DBGLVL_DISPFB,2,buf);
			}
		}
		else {
			if( yaddr && vpp_check_dbg_level(VPP_DBGLVL_DISPFB) ){
				char buf[50];

				sprintf(buf,"free dispfb2 Y 0x%x C 0x%x",yaddr,caddr);
				vpp_dbg_show(VPP_DBGLVL_DISPFB,2,buf);
			}
		}
	}

#ifdef CONFIG_SCL_DIRECT_PATH
	fb_list = &vpp_scl_fb_list;
	while( !list_empty(fb_list) ){
		ptr = fb_list->next;
		if( g_vpp.scl_fb_mb_on_work ){
			if( fb_list == ptr->next )
				break;
			ptr = ptr->next;
		}
		entry = list_entry(ptr,vpp_dispfb_parm_t,list);
		list_del_init(ptr);
		list_add_tail(&entry->list,&vpp_scl_free_list);
		if( entry->parm.flag & VPP_FLAG_DISPFB_ADDR ){
			yaddr = entry->parm.yaddr;
			caddr = entry->parm.caddr;
		}
		else if(entry->parm.flag & VPP_FLAG_DISPFB_INFO){
			yaddr = entry->parm.info.y_addr;
			caddr = entry->parm.info.c_addr;
		}

		if( (entry->parm.flag & VPP_FLAG_DISPFB_MB_NO) == 0 ){
			if( yaddr ){
				mb_put(yaddr);
			}
			
			if( caddr && !(entry->parm.flag & VPP_FLAG_DISPFB_MB_ONE)){
				mb_put(caddr);
			}
			
			if( yaddr && vpp_check_dbg_level(VPP_DBGLVL_DISPFB) ){
				char buf[50];

				sprintf(buf,"free sclfb Y 0x%x C 0x%x",yaddr,caddr);
				vpp_dbg_show(VPP_DBGLVL_DISPFB,2,buf);
			}
		}
		else {
			if( yaddr && vpp_check_dbg_level(VPP_DBGLVL_DISPFB) ){
				char buf[50];

				sprintf(buf,"free sclfb2 Y 0x%x C 0x%x",yaddr,caddr);
				vpp_dbg_show(VPP_DBGLVL_DISPFB,2,buf);
			}
		}
	}
//	g_vpp.scl_fb_mb_index = 0;
	g_vpp.scl_fb_mb_used = 0;
#endif

	if( vpp_check_dbg_level(VPP_DBGLVL_DISPFB) ){
		DPRINT("[VPP] Exit %s\n",__FUNCTION__);
	}

	vpp_unlock();
	return 0;
} /* End of vpp_disp_fb_clr */

/*!*************************************************************************
* vpp_disp_fb_isr()
* 
* Private Function by Sam Shen, 2009/02/02
*/
/*!
* \brief	interrupt service for display frame
*		
* \retval  status
*/ 
static int vpp_disp_fb_isr(void)
{
	vpp_mod_t mod;
	vpp_dispfb_parm_t *entry;
	struct list_head *ptr;
	static int isr_cnt = 0;

	vpp_lock();

#ifdef CONFIG_VPP_DISPFB_FREE_POSTPONE
	if( vpp_pre_dispfb_y_addr ){
		vpp_disp_fb_free(vpp_pre_dispfb_y_addr,vpp_pre_dispfb_c_addr);
		vpp_pre_dispfb_y_addr = 0;
		vpp_pre_dispfb_c_addr = 0;
	}
#endif
	
	ptr = 0;
	isr_cnt++;
#ifdef CONFIG_VPP_GOVRH_FBSYNC
	g_vpp.fbsync_isrcnt++;
	if( g_vpp.fbsync_enable ){
		if( g_vpp.vpp_path ){
			if( isr_cnt < g_vpp.fbsync_vsync ){
				vpp_unlock();
				return 0;
			}
		}
	}
#endif
	g_vpp.disp_fb_isr_cnt++;
	if( !list_empty(&vpp_disp_fb_list) ){
		unsigned int yaddr,caddr;

		mod = ( g_vpp.vpp_path )? VPP_MOD_GOVRH:VPP_MOD_VPU;
		yaddr = caddr = 0;		
		ptr = vpp_disp_fb_list.next;
		entry = list_entry(ptr,vpp_dispfb_parm_t,list);
		memcpy(&g_vpp.govw_pts,&entry->pts,sizeof(vpp_pts_t));

		if( g_vpp.vpu_skip_all ){
			g_vpp.disp_skip_cnt++;			
		}
		else {
#ifdef CONFIG_VPP_GOVRH_FBSYNC			
			if( g_vpp.vpp_path ){
				if( g_vpp.fbsync_enable ){
					g_vpp.fbsync_cnt++;
					isr_cnt=0;					
					if( g_vpp.fbsync_cnt > g_vpp.fbsync_frame ){
						g_vpp.fbsync_cnt = 1;
						g_vpp.fbsync_vsync = g_vpp.fbsync_step;
						g_vpp.fbsync_isrcnt = 0;
						if( g_vpp.fbsync_substep < 0 ){
							if( g_vpp.fbsync_cnt <= ( g_vpp.fbsync_substep * (-1))){
								g_vpp.fbsync_vsync -= 1;
							}
						}
					}
					else {
						g_vpp.fbsync_vsync = g_vpp.fbsync_step;
						if( g_vpp.fbsync_substep < 0 ){
							// if( g_vpp.fbsync_cnt > (g_vpp.fbsync_frame + g_vpp.fbsync_substep )){
							if( g_vpp.fbsync_cnt <= ( g_vpp.fbsync_substep * (-1))){
								g_vpp.fbsync_vsync -= 1;
							}
						}
						else if( g_vpp.fbsync_substep ){
							if( (g_vpp.fbsync_cnt % g_vpp.fbsync_substep) == 0 ){
								g_vpp.fbsync_vsync += 1;
							}
						}
					}
					
					if( vpp_check_dbg_level(VPP_DBGLVL_SYNCFB) ){
						char buf[50];

						sprintf(buf,"sync frame %d,sync cnt %d,vsync %d,isr %d",g_vpp.fbsync_frame,
							g_vpp.fbsync_cnt,g_vpp.fbsync_vsync,g_vpp.fbsync_isrcnt );
						vpp_dbg_show(VPP_DBGLVL_SYNCFB,3,buf);
					}
				}
			}
#endif
			if( g_vpp.vpp_path ){
				mod = VPP_MOD_GOVRH;
			}
			else {
				mod = VPP_MOD_VPU;
#ifdef WMT_FTBLK_VPU
				vpu_set_reg_update(VPP_FLAG_DISABLE);
#else
				scl_set_reg_update(VPP_FLAG_DISABLE);
#endif
				govm_set_reg_update(VPP_FLAG_DISABLE);
			}

			if( entry->parm.flag & VPP_FLAG_DISPFB_ADDR ){
				vpp_fb_base_t *mod_fb_p;
				
				yaddr = entry->parm.yaddr;
				caddr = entry->parm.caddr;
				mod_fb_p = vpp_mod_get_fb_base(mod);
				mod_fb_p->set_addr(yaddr,caddr);
				mod_fb_p->fb.y_addr = yaddr;
				mod_fb_p->fb.c_addr = caddr;
			}

			if( entry->parm.flag & VPP_FLAG_DISPFB_INFO ){
				vpp_fb_base_t *mod_fb_p;

				mod_fb_p = vpp_mod_get_fb_base(mod);
				yaddr = entry->parm.info.y_addr;
				caddr = entry->parm.info.c_addr;
				if( mod == VPP_MOD_GOVRH ){
#ifdef WMT_FTBLK_GOVRH
					vpp_display_format_t field;

					if( entry->parm.info.col_fmt != mod_fb_p->fb.col_fmt ){
						mod_fb_p->fb.col_fmt = entry->parm.info.col_fmt;
						govrh_set_data_format(mod_fb_p->fb.col_fmt);
						mod_fb_p->set_csc(mod_fb_p->csc_mode);
					}
#ifdef CONFIG_VPP_PATH_SWITCH_SCL_GOVR
					if( entry->parm.info.fb_w != mod_fb_p->fb.fb_w ){
						vppif_reg32_write(GOVRH_FWIDTH, entry->parm.info.fb_w);
					}
#endif
					field = (entry->parm.info.flag & VDO_FLAG_INTERLACE)?VPP_DISP_FMT_FIELD:VPP_DISP_FMT_FRAME;
					govrh_set_source_format(field);
					govrh_set_fb_addr(yaddr,caddr);
					mod_fb_p->fb = entry->parm.info;
#endif
				}
				else {
					mod_fb_p->fb = entry->parm.info;
					mod_fb_p->set_framebuf(&mod_fb_p->fb);
				}
			}

			if( entry->parm.flag & VPP_FLAG_DISPFB_VIEW ){
				vpp_set_video_scale(&entry->parm.view);
			}

			if( mod == VPP_MOD_VPU ){
#ifdef WMT_FTBLK_VPU
				vpu_set_reg_update(VPP_FLAG_ENABLE);
#else
				scl_set_reg_update(VPP_FLAG_ENABLE);
#endif
				govm_set_reg_update(VPP_FLAG_ENABLE);
			}
		}

		list_del_init(ptr);
		list_add_tail(&entry->list,&vpp_disp_free_list);
		g_vpp.disp_fb_cnt--;

		if( g_vpp.vpu_skip_all == 0 ){		
			if( vpp_check_dbg_level(VPP_DBGLVL_DISPFB) ){
				char buf[50];

				sprintf(buf,"show disp fb Y 0x%x C 0x%x (Q %d/%d,E %d/%d)",yaddr,caddr,
						g_vpp.disp_fb_cnt,g_vpp.disp_fb_max,g_vpp.disp_cnt,g_vpp.disp_fb_isr_cnt);
				vpp_dbg_show(VPP_DBGLVL_DISPFB,2,buf);
			}

#ifdef CONFIG_VPP_VBIE_FREE_MB
			if( vpp_pre_dispfb_y_addr )
				DPRINT("[VPP] *W* pre dispfb not free\n");

			vpp_pre_dispfb_y_addr = vpp_cur_dispfb_y_addr;
			vpp_pre_dispfb_c_addr = vpp_cur_dispfb_c_addr;
			vpp_cur_dispfb_y_addr = (yaddr)? yaddr:0;
			vpp_cur_dispfb_c_addr = (caddr && !(entry->parm.flag & VPP_FLAG_DISPFB_MB_ONE))? caddr:0;
#else
#ifndef CONFIG_VPP_DISPFB_FREE_POSTPONE
			vpp_disp_fb_free(vpp_pre_dispfb_y_addr,vpp_pre_dispfb_c_addr);
#endif
			vpp_pre_dispfb_y_addr = vpp_cur_dispfb_y_addr;
			vpp_pre_dispfb_c_addr = vpp_cur_dispfb_c_addr;
			if( entry->parm.flag & VPP_FLAG_DISPFB_MB_NO ){
				vpp_cur_dispfb_y_addr = 0;
				vpp_cur_dispfb_c_addr = 0;
#ifdef CONFIG_SCL_DIRECT_PATH_DEBUG
				g_vpp.scl_fb_mb_used--;
#endif
			}
			else {
				vpp_cur_dispfb_y_addr = (yaddr)? yaddr:0;
				vpp_cur_dispfb_c_addr = (caddr && !(entry->parm.flag & VPP_FLAG_DISPFB_MB_ONE))? caddr:0;
			}
#endif		
			g_vpp.disp_cnt++;
		}
	}
	
#ifdef CONFIG_VPP_DYNAMIC_DEI
	#define VPP_DEI_CHECK_PERIOD	150

	if( vppif_reg32_read(VPU_DEI_ENABLE) && (p_vpu->dei_mode == VPP_DEI_DYNAMIC) ){
		static int dei_cnt = VPP_DEI_CHECK_PERIOD;
		static unsigned int weave_sum,bob_sum;
		static unsigned int pre_weave_sum,pre_bob_sum;
		unsigned int usum,vsum;
		static vpp_deinterlace_t dei_mode = 0;
		unsigned int weave_diff,bob_diff,cur_diff;
		
		switch( dei_cnt ){
			case 2:
				if( dei_mode != VPP_DEI_ADAPTIVE_ONE ){
					g_vpp.govw_skip_frame = 1;
				}
				vpu_dei_set_mode(VPP_DEI_ADAPTIVE_ONE);
				break;
			case 1:
				vpu_dei_get_sum(&weave_sum,&usum,&vsum);
				if( dei_mode != VPP_DEI_FIELD ){
					g_vpp.govw_skip_frame = 1;
				}
				vpu_dei_set_mode(VPP_DEI_FIELD);				
				break;
			case 0:
				vpu_dei_get_sum(&bob_sum,&usum,&vsum);
				if( (vpp_calculate_diff(bob_sum,pre_bob_sum)<100000) 
					&& (vpp_calculate_diff(weave_sum,pre_weave_sum)<100000)){
					dei_mode = VPP_DEI_WEAVE;
				}
				else {
					dei_mode = ( bob_sum > (2*weave_sum) )? VPP_DEI_FIELD:VPP_DEI_ADAPTIVE_ONE;
				}
				bob_diff = vpp_calculate_diff(bob_sum,pre_bob_sum);
				weave_diff = vpp_calculate_diff(weave_sum,pre_weave_sum);
				cur_diff = vpp_calculate_diff(weave_sum,bob_sum);
				pre_bob_sum = bob_sum;
				pre_weave_sum = weave_sum;
				vpu_dei_set_mode(dei_mode);
				dei_cnt = VPP_DEI_CHECK_PERIOD;
				if( vpp_check_dbg_level(VPP_DBGLVL_DEI) ){
					static vpp_deinterlace_t pre_mode = 0;
					DPRINT("[VPP] bob %d,weave %d,diff bob %d,weave %d,cur %d\n",bob_sum,weave_sum,bob_diff,weave_diff,cur_diff);
					if( pre_mode != dei_mode ){
						DPRINT("[VPP] dei mode %d -> %d\n",pre_mode,dei_mode);
						pre_mode = dei_mode;
					}
				}
				break;
			default:
				break;
		}
		dei_cnt--;
	}
#endif

#ifdef WMT_FTBLK_PIP
	if( !list_empty(&vpp_pip_fb_list) ){
		unsigned int yaddr,caddr;
		
		ptr = vpp_pip_fb_list.next;
		entry = list_entry(ptr,vpp_dispfb_parm_t,list);
		
		yaddr = caddr = 0;
		if( entry->parm.flag & VPP_FLAG_DISPFB_ADDR ){
			yaddr = entry->parm.yaddr;
			caddr = entry->parm.caddr;
			p_pip->fb_p->set_addr(yaddr,caddr);
		}

		if( entry->parm.flag & VPP_FLAG_DISPFB_INFO ){
			yaddr = entry->parm.info.y_addr;
			caddr = entry->parm.info.c_addr;
			p_pip->fb_p->fb = entry->parm.info;
			p_pip->fb_p->set_framebuf(&p_pip->fb_p->fb);
		}

		if( entry->parm.flag & VPP_FLAG_DISPFB_VIEW ){
			p_pip->fb_p->fn_view(VPP_FLAG_WR,&entry->parm.view);
		}

		list_del_init(ptr);
		list_add_tail(&entry->list,&vpp_disp_free_list);

		if( p_pip->pre_yaddr ) mb_put(p_pip->pre_yaddr);
		if( p_pip->pre_caddr ) mb_put(p_pip->pre_caddr);
		p_pip->pre_yaddr = (yaddr)? yaddr:0;
		p_pip->pre_caddr = (caddr && !(entry->parm.flag & VPP_FLAG_DISPFB_MB_ONE))? caddr:0;
		g_vpp.pip_disp_cnt++;
	}
#endif
	vpp_unlock();
	return 0;
} /* End of vpp_disp_fb_isr */

#ifdef CONFIG_SCL_DIRECT_PATH
void vpp_scl_fb_init(void)
{
	int i;
	
	INIT_LIST_HEAD(&vpp_scl_free_list);
	INIT_LIST_HEAD(&vpp_scl_fb_list);
	for(i=0;i<VPP_SCL_FB_MAX;i++)
		list_add_tail(&vpp_scl_fb_array[i].list,&vpp_scl_free_list);
	g_vpp.scl_fb_mb_clear = 0xFF;
}

int vpp_scl_fb_get_mb(unsigned int *yaddr,unsigned int *caddr,int next)
{
	if( next == 0 ){
		if( g_vpp.scl_fb_mb_used >= VPP_SCL_MB_MAX ){
			return 1;
		}
	}
get_mb_label:
#ifdef CONFIG_VPP_GOVW_FRAMEBUF
	if( g_vpp.scl_fb_mb_index < g_vpp.mb_govw_cnt ){
		*yaddr = g_vpp.mb_govw[g_vpp.scl_fb_mb_index];
		*caddr = g_vpp.mb_govw[g_vpp.scl_fb_mb_index] + p_govw->fb_p->fb.y_size;
#else
	if( g_vpp.scl_fb_mb_index < VPP_MB_ALLOC_NUM ){
		*yaddr = g_vpp.mb[g_vpp.scl_fb_mb_index];
		*caddr = g_vpp.mb[g_vpp.scl_fb_mb_index] + p_govw->fb_p->fb.y_size;
#endif
	}
	else {
		unsigned int offset;
		offset = REG32_VAL(GE3_BASE_ADDR+0x50);
		offset *= REG32_VAL(GE3_BASE_ADDR+0x54);
		offset *= 4;
		if( g_vpp.scl_fb_mb_index > VPP_MB_ALLOC_NUM ){
			offset += ((p_govw->fb_p->fb.y_size + p_govw->fb_p->fb.c_size)*(g_vpp.scl_fb_mb_index-VPP_MB_ALLOC_NUM));
		}
		offset += (64 - (offset%64));
		
		*yaddr = (num_physpages << PAGE_SHIFT) + offset;
		*caddr = (num_physpages << PAGE_SHIFT) + offset + p_govw->fb_p->fb.y_size;
	}

	if( next ){
		g_vpp.scl_fb_mb_index++;
#ifdef CONFIG_VPP_GOVW_FRAMEBUF
		if(g_vpp.scl_fb_mb_index >= g_vpp.mb_govw_cnt){
#else
		if(g_vpp.scl_fb_mb_index >= VPP_SCL_MB_MAX){
#endif
			g_vpp.scl_fb_mb_index = 0;
		}

#ifdef CONFIG_SCL_DIRECT_PATH_DEBUG
		g_vpp.scl_fb_mb_used++;
		if( g_vpp.scl_fb_mb_used > VPP_SCL_MB_MAX ){
			DPRINT("[VPP] *W* scl mb use %d(max %d)\n",g_vpp.scl_fb_mb_used,VPP_SCL_MB_MAX);
			g_vpp.scl_fb_mb_over++;
		}
#endif
	}
	else {
		// skip for mb is current govr fb
		if( *yaddr == vppif_reg32_in(REG_GOVRH_YSA) ){
			if( g_vpp.scl_fb_mb_used == 0 ){
				g_vpp.scl_fb_mb_index++;
#ifdef CONFIG_VPP_GOVW_FRAMEBUF
				if(g_vpp.scl_fb_mb_index >= g_vpp.mb_govw_cnt){
#else
				if(g_vpp.scl_fb_mb_index >= VPP_SCL_MB_MAX){
#endif
					g_vpp.scl_fb_mb_index = 0;
				}
				goto get_mb_label;
			}
			return 1;
		}
	}
	return 0;
}

#ifdef CONFIG_SCL_DIRECT_PATH_DEBUG
void vpp_scl_fb_cal_tmr(int begin)
{
	static struct timeval pre_tv;

	if( begin ){
		do_gettimeofday(&pre_tv);
	}
	else {
		struct timeval tv;
		unsigned int tm_usec = 0;
		
		do_gettimeofday(&tv);
		if( pre_tv.tv_sec ){
			tm_usec = ( tv.tv_sec == pre_tv.tv_sec )? (tv.tv_usec - pre_tv.tv_usec):(1000000 + tv.tv_usec - pre_tv.tv_usec);
			g_vpp.scl_fb_tmr += (tm_usec / 1000);
		}
	}
}
#endif	

void vpp_scl_fb_set_size(vdo_framebuf_t *fb)
{
	unsigned int line_byte;
	unsigned int offset;

#if 0	// debug for clear memory issue
	g_vpp.scl_fb_mb_clear = 0xFF;
#else
	if( (fb->img_w == p_vpu->resx_visual) && (fb->img_h == p_vpu->resy_visual) ){
		g_vpp.scl_fb_mb_clear = 0;
		return;
	}
#endif

	fb->y_addr += p_vpu->posx + p_vpu->posy * p_govw->fb_p->fb.fb_w;
	line_byte = p_govw->fb_p->fb.fb_w;
	offset = p_vpu->posx;
	switch( p_govw->fb_p->fb.col_fmt ){
		case VDO_COL_FMT_YUV420:
		case VDO_COL_FMT_YUV422H:			
			offset = (offset / 2) * 2;
			break;
		case VDO_COL_FMT_YUV444:
			line_byte *= 2;
			break;
		default:
			break;
	}
	fb->c_addr += offset + p_vpu->posy * line_byte;
#if 0
	fb->y_addr &= ~0x3F;
	fb->c_addr &= ~0x3F;
#else
	if( fb->fb_w % 64 ) fb->fb_w += (64 - (fb->fb_w % 64));
#endif

	fb->img_w = p_vpu->resx_visual;
	fb->img_h = p_vpu->resy_visual;
	if( g_vpp.scl_fb_mb_clear & (0x1 << g_vpp.scl_fb_mb_index) ){
		unsigned int yaddr,caddr;

#ifdef CONFIG_VPP_GOVW_FRAMEBUF
		yaddr = g_vpp.mb_govw[g_vpp.scl_fb_mb_index];
		caddr = g_vpp.mb_govw[g_vpp.scl_fb_mb_index] + p_govw->fb_p->fb.y_size;
#else		
		yaddr = g_vpp.mb[g_vpp.scl_fb_mb_index];
		caddr = g_vpp.mb[g_vpp.scl_fb_mb_index] + p_govw->fb_p->fb.y_size;
#endif
		yaddr = (unsigned int) mb_phys_to_virt(yaddr);
		caddr = (unsigned int) mb_phys_to_virt(caddr);
		memset((void *)yaddr,0x0,p_govw->fb_p->fb.y_size);
		memset((void *)caddr,0x80,p_govw->fb_p->fb.y_size);
		
		g_vpp.scl_fb_mb_clear &= ~(0x1 << g_vpp.scl_fb_mb_index);
//		DPRINT("[VPP] clr scl fb mb %d\n",g_vpp.scl_fb_mb_index);
	}
}

void vpp_scl_fb_isr(void)
{
	static vpp_dispfb_parm_t *entry = 0;
	vdo_framebuf_t src_fb,dst_fb;
	unsigned int yaddr,caddr;
	unsigned int pre_yaddr,pre_caddr;
	extern int scl_scale_complete;

	// scale complete, add to disp fb queue
	if( scl_scale_complete ){
		vpp_scl_fb_get_mb(&yaddr,&caddr,1);
		pre_yaddr = pre_caddr = 0;
		// add dst to disp fb
		if( entry->parm.flag & VPP_FLAG_DISPFB_ADDR ){
			pre_yaddr = entry->parm.yaddr;
			pre_caddr = entry->parm.caddr;
			entry->parm.yaddr = yaddr;
			entry->parm.caddr = caddr;
		}

		if( entry->parm.flag & VPP_FLAG_DISPFB_INFO ){
			pre_yaddr = entry->parm.info.y_addr;
			pre_caddr = entry->parm.info.c_addr;
			entry->parm.info.y_addr = yaddr;
			entry->parm.info.c_addr = caddr;
		}

//		DPRINT("[VPP] SCL DIRPATH SCALE COMPLETE (0x%x,0x%x)->(0x%x,0x%x)\n",pre_yaddr,pre_caddr,yaddr,caddr);

		pre_caddr = (pre_caddr && !(entry->parm.flag & VPP_FLAG_DISPFB_MB_ONE))? pre_caddr:0;
		vpp_disp_fb_free(pre_yaddr,pre_caddr);

		entry->parm.flag = (VPP_FLAG_DISPFB_MB_ONE + VPP_FLAG_DISPFB_MB_NO);
#ifdef CONFIG_VPP_PATH_SWITCH_SCL_GOVR
		if( (p_govw->fb_p->fb.col_fmt != p_govrh->fb_p->fb.col_fmt) || 
			(p_govw->fb_p->fb.fb_w != p_govrh->fb_p->fb.fb_w)){
			entry->parm.info.y_addr = yaddr;
			entry->parm.info.c_addr = caddr;
			entry->parm.info.col_fmt = p_govw->fb_p->fb.col_fmt;
			entry->parm.info.fb_w = p_govw->fb_p->fb.fb_w;
			entry->parm.info.flag = 0;
			entry->parm.flag |= VPP_FLAG_DISPFB_INFO;
		}
		else {
#else
		if( 1 ){
#endif
			entry->parm.yaddr = yaddr;
			entry->parm.caddr = caddr;
			entry->parm.flag |= VPP_FLAG_DISPFB_ADDR;
		}
		vpp_disp_fb_add(&entry->parm);

		// free src fb
		list_del_init(vpp_scl_fb_list.next);
		list_add_tail(&entry->list,&vpp_scl_free_list);
		scl_scale_complete = 0;
		g_vpp.scl_fb_mb_on_work = 0;
		entry = 0;
//		DPRINT("[VPP] SCL DIRPATH SCALE END\n");
	}

	if( g_vpp.scl_fb_mb_on_work )	// in scaling ...
		return;

	// if scale queue not empty
	if( list_empty(&vpp_scl_fb_list) )
		return;

//	DPRINT("[VPP] SCL DIRPATH SCALE isr\n");

	yaddr = caddr = 0;		
	// get scale queue to src fb
	entry = list_entry(vpp_scl_fb_list.next,vpp_dispfb_parm_t,list);
	if( entry->parm.flag & VPP_FLAG_DISPFB_ADDR ){
		yaddr = entry->parm.yaddr;
		caddr = entry->parm.caddr;
	}

	if( entry->parm.flag & VPP_FLAG_DISPFB_INFO ){
		yaddr = entry->parm.info.y_addr;
		caddr = entry->parm.info.c_addr;
		if( vpp_disp_fb_compare(&p_vpu->fb_p->fb,&entry->parm.info) == 0 ){
			p_vpu->fb_p->fb = entry->parm.info;
			g_vpp.scl_fb_mb_clear = 0xFF;
		}
	}

	if( entry->parm.flag & VPP_FLAG_DISPFB_VIEW ){
		vpp_set_video_scale(&entry->parm.view);
	}

#ifdef CONFIG_VPP_PATH_SWITCH_SCL_GOVR	// if no scale then pass to govr path
	if( (p_vpu->resx_visual == p_vpu->fb_p->fb.img_w) &&
		(p_vpu->resy_visual == p_vpu->fb_p->fb.img_h) && 
		(p_vpu->resx_visual == p_govw->fb_p->fb.img_w) &&
		(p_vpu->resy_visual == p_govw->fb_p->fb.img_h)
		){
		list_del_init(vpp_scl_fb_list.next);
		list_add_tail(&entry->list,&vpp_scl_free_list);
		entry->parm.flag |= VPP_FLAG_DISPFB_MB_GOVR;
		vpp_disp_fb_add(&entry->parm);
		return;
	}
#endif

	if( yaddr ){
		src_fb = p_vpu->fb_p->fb;
		src_fb.y_addr = yaddr;
		src_fb.c_addr = caddr;

		// alloc mb for dst fb
		dst_fb = p_govw->fb_p->fb;
		if( vpp_scl_fb_get_mb(&dst_fb.y_addr,&dst_fb.c_addr,0) ){
			return;
		}

		vpp_scl_fb_set_size(&dst_fb);

//		DPRINT("[VPP] SCL DIRPATH SCALE (0x%x,0x%x)->(0x%x,0x%x)\n",yaddr,caddr,dst_fb.y_addr,dst_fb.c_addr);

#ifdef CONFIG_SCL_DIRECT_PATH_DEBUG
		g_vpp.scl_fb_cnt++;
		vpp_scl_fb_cal_tmr(1);
#endif

#if 1
		// scale next frame
		p_scl->scale_sync = 0;
		g_vpp.scl_fb_mb_on_work = 1;
		if( vpp_set_recursive_scale(&src_fb,&dst_fb) ){
			DPRINT("[VPP] *E* scl fb scale fail\n");
		}
#endif
	}
	else {
		list_del_init(vpp_scl_fb_list.next);
		list_add_tail(&entry->list,&vpp_scl_free_list);
	}
}
#endif

#ifdef CONFIG_GOVW_SCL_PATH
struct list_head vpp_vpu_fb_list;
static unsigned int vpp_cur_vpufb_y_addr;
static unsigned int vpp_cur_vpufb_c_addr;
static unsigned int vpp_pre_vpufb_y_addr;
static unsigned int vpp_pre_vpufb_c_addr;

static int vpp_vpu_fb_add
(
	vpp_dispfb_t *fb		/*!<; // display frame pointer */
)
{
	vpp_dispfb_parm_t *entry;
	struct list_head *ptr;
	unsigned int yaddr,caddr;
	struct list_head *fb_list;

	if( list_empty(&vpp_disp_free_list) ){
		return -1;
	}

	vpp_lock();
	ptr = vpp_disp_free_list.next;
	fb_list = &vpp_vpu_fb_list;
	if( g_vpp.vpu_fb_cnt >= g_vpp.disp_fb_max ){
		if( vpp_check_dbg_level(VPP_DBGLVL_DISPFB) ){
			char buf[50];

			sprintf(buf,"*W* add vpufb full max %d cnt %d",g_vpp.disp_fb_max,g_vpp.vpu_fb_cnt);
			vpp_dbg_show(VPP_DBGLVL_DISPFB,1,buf);	
		}
		g_vpp.vpu_fb_full_cnt++;
		vpp_unlock();
		return -1;
	}
	g_vpp.vpu_fb_cnt++;

	entry = list_entry(ptr,vpp_dispfb_parm_t,list);
	list_del_init(ptr);
	entry->parm = *fb;

#if 1	// patch for VPU bilinear mode
	if(entry->parm.flag & VPP_FLAG_DISPFB_INFO){
		if( vpp_disp_fb_compare(&entry->parm.info,&p_vpu->fb_p->fb) ){
			entry->parm.flag &= ~VPP_FLAG_DISPFB_INFO;
			entry->parm.flag |= VPP_FLAG_DISPFB_ADDR;
			entry->parm.yaddr = entry->parm.info.y_addr;
			entry->parm.caddr = entry->parm.info.c_addr;
		}
	}
#endif	

	memcpy(&entry->pts,&g_vpp.frame_pts,sizeof(vpp_pts_t));
	list_add_tail(&entry->list,fb_list);
	yaddr = caddr = 0;
	if( entry->parm.flag & VPP_FLAG_DISPFB_ADDR ){
		yaddr = entry->parm.yaddr;
		caddr = entry->parm.caddr;
	}
	else if(entry->parm.flag & VPP_FLAG_DISPFB_INFO){
		yaddr = entry->parm.info.y_addr;
		caddr = entry->parm.info.c_addr;
	}

	if( (entry->parm.flag & VPP_FLAG_DISPFB_MB_NO) == 0 ){
		if( yaddr ){
			mb_get(yaddr);
		}
		
		if( caddr && !(entry->parm.flag & VPP_FLAG_DISPFB_MB_ONE)){
			mb_get(caddr);
		}
	}

	if( vpp_check_dbg_level(VPP_DBGLVL_DISPFB) ){
		char buf[50];

		sprintf(buf,"add %s Y 0x%x C 0x%x(Q %d/%d,E %d/%d)","vpufb",yaddr,caddr,
			g_vpp.vpu_fb_cnt,g_vpp.disp_fb_max,g_vpp.disp_cnt,g_vpp.disp_fb_isr_cnt);
		vpp_dbg_show(VPP_DBGLVL_DISPFB,1,buf);
	}
	
	vpp_unlock();
	return 0;
} /* End of vpp_disp_fb_add */

static int vpp_vpu_fb_isr(void)
{
	vpp_mod_t mod;
	vpp_dispfb_parm_t *entry;
	struct list_head *ptr;
	static int isr_cnt = 0;

	vpp_lock();

#ifdef CONFIG_VPP_DISPFB_FREE_POSTPONE
	if( vpp_pre_vpufb_y_addr ){
		vpp_disp_fb_free(vpp_pre_vpufb_y_addr,vpp_pre_vpufb_c_addr);
		vpp_pre_vpufb_y_addr = 0;
		vpp_pre_vpufb_c_addr = 0;
	}
#endif
	
	ptr = 0;
	isr_cnt++;
	g_vpp.vpu_fb_isr_cnt++;
	if( !list_empty(&vpp_vpu_fb_list) ){
		unsigned int yaddr,caddr;

		mod = VPP_MOD_VPU;
		yaddr = caddr = 0;		
		ptr = vpp_vpu_fb_list.next;
		entry = list_entry(ptr,vpp_dispfb_parm_t,list);
		memcpy(&g_vpp.govw_pts,&entry->pts,sizeof(vpp_pts_t));

		vpu_set_reg_update(VPP_FLAG_DISABLE);
		govm_set_reg_update(VPP_FLAG_DISABLE);

		if( entry->parm.flag & VPP_FLAG_DISPFB_ADDR ){
			vpp_fb_base_t *mod_fb_p;
			
			yaddr = entry->parm.yaddr;
			caddr = entry->parm.caddr;
			mod_fb_p = vpp_mod_get_fb_base(mod);
			mod_fb_p->set_addr(yaddr,caddr);
			mod_fb_p->fb.y_addr = yaddr;
			mod_fb_p->fb.c_addr = caddr;
		}

		if( entry->parm.flag & VPP_FLAG_DISPFB_INFO ){
			vpp_fb_base_t *mod_fb_p;

			mod_fb_p = vpp_mod_get_fb_base(mod);
			yaddr = entry->parm.info.y_addr;
			caddr = entry->parm.info.c_addr;
			mod_fb_p->fb = entry->parm.info;
			mod_fb_p->set_framebuf(&mod_fb_p->fb);
		}

		if( entry->parm.flag & VPP_FLAG_DISPFB_VIEW ){
			vpp_set_video_scale(&entry->parm.view);
		}

		vpu_set_reg_update(VPP_FLAG_ENABLE);
		govm_set_reg_update(VPP_FLAG_ENABLE);

		list_del_init(ptr);
		list_add_tail(&entry->list,&vpp_disp_free_list);
		g_vpp.vpu_fb_cnt--;

		if( vpp_check_dbg_level(VPP_DBGLVL_DISPFB) ){
			char buf[50];

			sprintf(buf,"show disp fb Y 0x%x C 0x%x (Q %d/%d,E %d/%d)",yaddr,caddr,
					g_vpp.vpu_fb_cnt,g_vpp.disp_fb_max,g_vpp.vpu_cnt,g_vpp.vpu_fb_isr_cnt);
			vpp_dbg_show(VPP_DBGLVL_DISPFB,2,buf);
		}

		vpp_pre_vpufb_y_addr = vpp_cur_vpufb_y_addr;
		vpp_pre_vpufb_c_addr = vpp_cur_vpufb_c_addr;
		if( entry->parm.flag & VPP_FLAG_DISPFB_MB_NO ){
			vpp_cur_vpufb_y_addr = 0;
			vpp_cur_vpufb_c_addr = 0;
		}
		else {
			vpp_cur_vpufb_y_addr = (yaddr)? yaddr:0;
			vpp_cur_vpufb_c_addr = (caddr && !(entry->parm.flag & VPP_FLAG_DISPFB_MB_ONE))? caddr:0;
		}
		g_vpp.vpu_cnt++;
	}		
	vpp_unlock();
	return 0;
} /* End of vpp_disp_fb_isr */

#endif


#ifdef CONFIG_VPP_STREAM_CAPTURE
unsigned int vpp_mb_get_mask(unsigned int phy)
{
	int i;
	unsigned int mask;

#ifdef CONFIG_VPP_GOVW_FRAMEBUF
	for(i=0;i<g_vpp.mb_govw_cnt;i++){
		if( g_vpp.mb_govw[i] == phy )
			break;
	}
	if( i >= g_vpp.mb_govw_cnt ){
		return 0;
	}
#else
	for(i=0;i<VPP_MB_ALLOC_NUM;i++){
		if( g_vpp.mb[i] == phy )
			break;
	}
	if( i >= VPP_MB_ALLOC_NUM ){
		return 0;
	}
#endif
	mask = 0x1 << i;
	return mask;
}

int vpp_mb_get(unsigned int phy)
{
	unsigned int mask;
	int i,cnt;

	if( g_vpp.stream_mb_sync_flag ){	// not new mb updated
		vpp_dbg_show(VPP_DBGLVL_STREAM,0,"[VPP] *W* mb_get addr not update");
		return -1;
	}

#ifdef CONFIG_VPP_GOVW_FRAMEBUF
	for(i=0,cnt=0;i<g_vpp.mb_govw_cnt;i++){
		if( g_vpp.stream_mb_lock & (0x1 << i))
			cnt++;
	}

	if( cnt >= (g_vpp.mb_govw_cnt - 2) ){
		vpp_dbg_show(VPP_DBGLVL_STREAM,0,"[VPP] *W* mb_get addr not free");
		return -1;
	}
#endif
	if( (mask = vpp_mb_get_mask(phy)) == 0 ){
		vpp_dbg_show(VPP_DBGLVL_STREAM,0,"[VPP] *W* mb_get invalid addr");
		return -1;
	}
	if( g_vpp.stream_mb_lock & mask ){
		vpp_dbg_show(VPP_DBGLVL_STREAM,0,"[VPP] *W* mb_get lock addr");	
		return -1;
	}
	g_vpp.stream_mb_lock |= mask;
	g_vpp.stream_mb_sync_flag = 1;
	if( vpp_check_dbg_level(VPP_DBGLVL_STREAM) ){
		char buf[50];

		sprintf(buf,"stream mb get 0x%x,mask 0x%x",phy,mask);
		vpp_dbg_show(VPP_DBGLVL_STREAM,1,buf);
	}
	return 0;
}

int vpp_mb_put(unsigned int phy)
{
	unsigned int mask;

	if( phy == 0 ){
		g_vpp.stream_mb_lock = 0;
		return 0;
	}

	if( (mask = vpp_mb_get_mask(phy)) == 0 ){
		DPRINT("[VPP] *W* mb_put addr 0x%x\n",phy);
		return 1;
	}
	if( !(g_vpp.stream_mb_lock & mask) ){
		DPRINT("[VPP] *W* mb_put nonlock addr 0x%x\n",phy);
	}
	g_vpp.stream_mb_lock &= ~mask;
	if( vpp_check_dbg_level(VPP_DBGLVL_STREAM) ){
		char buf[50];

		sprintf(buf,"stream mb put 0x%x,mask 0x%x",phy,mask);
		vpp_dbg_show(VPP_DBGLVL_STREAM,2,buf);
	}
	return 0;
}

#endif

/*!*************************************************************************
* vpp_govw_int_routine()
* 
* Private Function by Sam Shen, 2009/01/17
*/
/*!
* \brief	govw interrupt routine
*		
* \retval  None
*/
void vpp_govw_int_routine(void)
{
#ifdef CONFIG_VPP_DUAL_BUFFER
	unsigned int govr_y,govr_c;
	unsigned int govw_y,govw_c;

	if( g_vpp.govw_fb_cnt ){
		g_vpp.govw_fb_cnt--;
		if( g_vpp.govw_fb_cnt == 0 ){
			vpp_set_govw_tg(VPP_FLAG_DISABLE);
		}
	}

	if( g_vpp.govw_skip_all ){
		return;
	}

	if( g_vpp.govw_skip_frame ){
		g_vpp.govw_skip_frame--;
		vpp_dbg_show(VPP_DBGLVL_DIAG,3,"GOVW skip");
		return;
	}

#ifdef CONFIG_VPP_GOVW_FRAMEBUF
{
	int i;
	
	govw_y = p_govw->fb_p->fb.y_addr;
	for(i=0;i<g_vpp.mb_govw_cnt;i++){
		if( govw_y == g_vpp.mb_govw[i] )
			break;
	}
	if( i >= g_vpp.mb_govw_cnt )
		i = 0;
	govw_y = g_vpp.mb_govw[i];
	govw_c = govw_y + g_vpp.mb_govw_y_size;

	do {
		i++;
		if( i >= g_vpp.mb_govw_cnt ){
			i = 0;
		}
#ifdef CONFIG_VPP_STREAM_CAPTURE
		if( g_vpp.stream_enable ){
			if( g_vpp.stream_mb_lock & (0x1 << i) ){
				continue;
			}
		}
		break;
#endif
	} while(1);
	govr_y = g_vpp.mb_govw[i];
	govr_c = govr_y + g_vpp.mb_govw_y_size;
}
#else
	govr_y = g_vpp.govr->fb_p->fb.y_addr;
	govr_c = g_vpp.govr->fb_p->fb.c_addr;
	govw_y = p_govw->fb_p->fb.y_addr;
	govw_c = p_govw->fb_p->fb.c_addr;
#endif

#ifdef CONFIG_GOVW_SCL_PATH
	if( g_vpp.vpp_path == VPP_VPATH_GOVW_SCL ){
		// add govw_y & govw_c to disp_fb queue
		// get new fb to govw
	}
	else 
#endif
	{
		g_vpp.govr->fb_p->set_addr(govw_y,govw_c);
		p_govw->fb_p->set_addr(govr_y,govr_c);
		
		g_vpp.govr->fb_p->fb.y_addr = govw_y;
		g_vpp.govr->fb_p->fb.c_addr = govw_c;
		p_govw->fb_p->fb.y_addr = govr_y;
		p_govw->fb_p->fb.c_addr = govr_c;
		memcpy(&g_vpp.disp_pts,&g_vpp.govw_pts,sizeof(vpp_pts_t));
#ifdef CONFIG_VPP_STREAM_CAPTURE	
		g_vpp.stream_mb_sync_flag = 0;
#endif
	}
#endif	
} /* End of vpp_govw_int_routine */

/*!*************************************************************************
* vpp_irqproc_get_no()
* 
* Private Function by Sam Shen, 2010/08/03
*/
/*!
* \brief	get vpp int no from a mask
*		
* \retval  irq no
*/ 
int vpp_irqproc_get_no(vpp_int_t vpp_int)
{
	int i;
	unsigned int mask;

	if( vpp_int == 0 )
		return 0xFF;
		
	for(i=0,mask=0x1;i<32;i++,mask<<=1){
		if( vpp_int & mask )
			break;
	}
	return i;
} /* End of vpp_irqproc_get_no */

/*!*************************************************************************
* vpp_irqproc_get_entry()
* 
* Private Function by Sam Shen, 2010/08/03
*/
/*!
* \brief	get irqproc entry
*		
* \retval  entry pointer
*/ 
vpp_irqproc_t *vpp_irqproc_get_entry(vpp_int_t vpp_int)
{
	int no;

	no = vpp_irqproc_get_no(vpp_int);
	if( no >= 32 ) 
		return 0;
	return vpp_irqproc_array[no];
} /* End of vpp_irqproc_get_entry */

/*!*************************************************************************
* vpp_irqproc_do_tasklet()
* 
* Private Function by Sam Shen, 2010/08/03
*/
/*!
* \brief	irqproc tasklet
*		
* \retval  None
*/ 
static void vpp_irqproc_do_tasklet
(
	unsigned long data		/*!<; // tasklet input data */
)
{
	vpp_proc_t *entry;
	struct list_head *ptr;
	vpp_irqproc_t *irqproc;

	vpp_lock();
	irqproc = vpp_irqproc_get_entry(data);
	if( irqproc ){
		do {
			if( list_empty(&irqproc->list) )
				break;

			/* get task from work head queue */
			ptr = (&irqproc->list)->next;
			entry = list_entry(ptr,vpp_proc_t,list);
			if( entry->func ){
				if( entry->func(entry->arg) )
					break;
			}
			list_del_init(ptr);
			list_add_tail(&entry->list,&vpp_free_list);
			up(&entry->sem);
//			DPRINT("[VPP] vpp_irqproc_do_tasklet\n");
		} while(1);
	}
	vpp_unlock();
} /* End of vpp_irqproc_do_tasklet */

/*!*************************************************************************
* vpp_irqproc_new()
* 
* Private Function by Sam Shen, 2010/08/03
*/
/*!
* \brief	Create a irqproc entry and init
*		
* \retval  Status
*/ 
int vpp_irqproc_new
(
	vpp_int_t vpp_int,
	void (*proc)(int arg)
)
{
	int no;
	vpp_irqproc_t *irqproc;

	no = vpp_irqproc_get_no(vpp_int);
	if( no >= 32 ) 
		return 1;
	
	irqproc = kmalloc(sizeof(vpp_irqproc_t),GFP_KERNEL);
	vpp_irqproc_array[no] = irqproc;
	INIT_LIST_HEAD(&irqproc->list);
	tasklet_init(&irqproc->tasklet,vpp_irqproc_do_tasklet,vpp_int);
	irqproc->proc = proc;
	return 0;
} /* End of vpp_irqproc_new */

/*!*************************************************************************
* vpp_irqproc_work()
* 
* Private Function by Sam Shen, 2010/08/03
*/
/*!
* \brief	Insert a task to work in vpp irq and wait complete
*		
* \retval  Status
*/ 
int vpp_irqproc_work
(
	vpp_int_t type,
	int (*func)(void *argc),
	void *arg,
	int wait
)
{
	int ret;
	vpp_proc_t *entry;
	struct list_head *ptr;
	vpp_irqproc_t *irqproc;
	int enable_interrupt = 0;

	// DPRINT("[VPP] vpp_irqproc_work(type 0x%x,wait %d)\n",type,wait);

	if( (vpp_free_list.next == 0) || list_empty(&vpp_free_list) ){
		if( func ) func(arg);
		return 0;
	}

	if( vppm_get_int_enable(type) == 0 ){
		vppm_set_int_enable(1,type);
		enable_interrupt = 1;
	}

	ret = 0;
	vpp_lock();
	
	ptr = vpp_free_list.next;
	entry = list_entry(ptr,vpp_proc_t,list);
	list_del_init(ptr);
	entry->func = func;
	entry->arg = arg;
	entry->type = type;
	init_MUTEX(&entry->sem);
	down(&entry->sem);
	
	irqproc = vpp_irqproc_get_entry(type);
	if( irqproc ){
		list_add_tail(&entry->list,&irqproc->list);
	}
	else {
		irqproc = vpp_irqproc_array[31];
		list_add_tail(&entry->list,&irqproc->list);
	}
	vpp_unlock();
	if( wait ) {
		ret = down_timeout(&entry->sem,HZ);
		if( ret ){
			DPRINT("[VPP] *W* vpp_irqproc_work timeout(type 0x%x)\n",type);
			list_del_init(ptr);
			list_add_tail(ptr,&vpp_free_list);
		}
	}

	if( enable_interrupt ){
		vppm_set_int_enable(0,type);
	}
	return ret;
} /* End of vpp_irqproc_work */

int vpp_irqproc_chg_path(int arg)
{
	vdo_framebuf_t *fb;
	
	fb = &p_govrh->fb_p->fb;
	govrh_set_reg_update(0);
	govrh_set_data_format(fb->col_fmt);
	govrh_set_csc_mode(p_govrh->fb_p->csc_mode);
	govrh_set_fb_info(fb->fb_w, fb->img_w, fb->h_crop, fb->v_crop);
	govrh_set_fb_addr(fb->y_addr, fb->c_addr);
	govrh_set_reg_update(1);
//	DPRINT("[VPP] irqproc chg_path %d\n",arg);
	return 0;
}

#ifdef WMT_FTBLK_GOVRH
void vpp_irqproc_govrh_vbis(int arg)
{
	if( arg == 0 ){
		g_vpp.govrh_vbis_cnt++;
		g_vpp.govrh_field = (g_vpp.govrh_field == VPP_FIELD_TOP)? VPP_FIELD_BOTTOM:VPP_FIELD_TOP;
		if( vppif_reg32_read(GOVRH_TG_MODE) && (g_vpp.govrh_field != VPP_FIELD_TOP) ){
			/* interlace bottom field */
		}
		else {
			vout_plug_detect(VOUT_VGA);
		}
		vout_plug_detect(VOUT_HDMI);
	}
}

void vpp_irqproc_govrh_pvbi(int arg)
{
	if( arg == 0 ){
#ifdef WMT_FTBLK_GOVRH_CURSOR
		if( p_cursor->enable ){
			if( p_cursor->chg_flag ){
				p_cursor->hide_cnt = 0;
				vppif_reg32_write(GOVRH_CUR_ENABLE,1);
			}
			else {
				p_cursor->hide_cnt++;
				if( p_cursor->hide_cnt > (GOVRH_CURSOR_HIDE_TIME * p_govrh->fb_p->framerate) ){
					vppif_reg32_write(GOVRH_CUR_ENABLE,0);
				}
			}
		}

		if( p_cursor->chg_flag ){
			govrh_irqproc_set_position(0);
			p_cursor->chg_flag = 0;
		}
#endif

		if( !g_vpp.vpp_path_no_ready ){
			switch( g_vpp.vpp_path ){
				case VPP_VPATH_GOVR:
					vpp_disp_fb_isr();
					break;
#ifdef CONFIG_SCL_DIRECT_PATH				
				case VPP_VPATH_SCL:
					vpp_disp_fb_isr();
					vpp_scl_fb_isr();
					break;
#endif
#ifdef CONFIG_GOVW_SCL_PATH
				case VPP_VPATH_GOVW_SCL:
					vpp_disp_fb_isr();
					break;
#endif
				default:
					break;
			}
		}
	}
}

#ifdef PATCH_GOVRH_VSYNC_OVERLAP
void vpp_irqproc_govrh_vbie(int arg)
{
	if( arg == 0 ){
		int field;

		field = vppif_reg32_read(GOVRH_FIELD_STATUS);
//		if( vppif_reg32_read(GOVRH_OUTFMT) ){ // interlace mode
		if( p_govrh->h_pixel ){
			vppif_reg32_write(GOVRH_VSYNC_OFFSET,(field)? (p_govrh->h_pixel):(p_govrh->h_pixel/2));
			vppif_reg32_write(GOVRH_V_ALLLN,(field)? (p_govrh->v_line-1):(p_govrh->v_line));
		}
		vppif_reg32_write(GOVRH_FIELD_INVERT,(field)? 0:1);
	}
}
#endif
#endif

void vpp_irqproc_govw_pvbi(int arg)
{
	if( arg == 0 ){
#ifndef CONFIG_GOVW_FBSWAP_VBIE
		if( !g_vpp.vpp_path_no_ready ){
			switch(g_vpp.vpp_path){
				case VPP_VPATH_GOVW:
					vpp_govw_int_routine();
					vpp_disp_fb_isr();
					break;
#ifdef CONFIG_GOVW_SCL_PATH
				case VPP_VPATH_GOVW_SCL:
					vpp_govw_int_routine();
					vpp_disp_fb_isr_ext(0);
					break;
#endif
				default:
					break;
			}
		}
#endif
	}

	if( arg == 1 ){
		g_vpp.govw_pvbi_cnt++;
#ifdef CONFIG_GOVW_FPS_AUTO_ADJUST
		vpp_govw_dynamic_tg(0);
#endif
#ifdef CONFIG_VPP_VBIE_FREE_MB
		if( vpp_free_y_addr ) {
			if( vpp_free_y_addr == vpp_cur_dispfb_y_addr ){
				DPRINT("[VPP] *W* cur & free fb are same 0x%x\n",vpp_free_y_addr);
			}
			mb_put(vpp_free_y_addr);
		}
		if( vpp_free_c_addr ) mb_put(vpp_free_c_addr);

		if( vpp_free_y_addr ){				
			if( vpp_check_dbg_level(VPP_DBGLVL_DISPFB) ){
				char buf[50];

				sprintf(buf,"free disp fb Y 0x%x C 0x%x",vpp_free_y_addr,vpp_free_c_addr);
				vpp_dbg_show(VPP_DBGLVL_DISPFB,2,buf);
			}
		}
		vpp_free_y_addr = 0;
		vpp_free_c_addr = 0;				
#endif
	}
}

void vpp_irqproc_govw_vbis(int arg)
{
	if( arg == 0 ){
 #ifdef PATCH_VPU_HALT_GOVW_TG_ERR2
		static unsigned int w_ypxlwid;
 
 		if( vpp_govw_tg_err_patch == 2 ){
			vppif_reg32_write(VPU_R_YPXLWID,1);
			w_ypxlwid = vppif_reg32_read(VPU_W_YPXLWID);
			vppif_reg32_write(VPU_W_YPXLWID,1);
			vpp_govw_tg_err_patch = 3;
		}
		else if( vpp_govw_tg_err_patch == 3 ){
			unsigned int temp;
			
			vppif_reg32_write(GOVW_HD_MIF_ENABLE,1);

			temp = p_vpu->fb_p->fb.img_w;
			vppif_reg32_write(VPU_HXWIDTH,temp);
			vppif_reg32_write(VPU_R_YPXLWID,temp);
			vppif_reg32_write(VPU_W_YPXLWID,w_ypxlwid);
			temp = temp * 16 / p_vpu->resx_visual;
			vppif_reg32_write(VPU_H_STEP,temp);
			vppif_reg32_write(VPU_HBI_MODE,3);

			vpp_govw_tg_err_patch = 0;
//	if( (vpp_govw_tg_err_patch_cnt % 1000) == 0 ){
				DPRINT("[GOVW TG INT] 2/3 scale,VBIE,pxlw=1(VBIS),ret(VBIS),%d\n",vpp_govw_tg_err_patch_cnt);
//	}
			vpp_govw_tg_err_patch_cnt++;
			// vpp_reg_dump(VPU_BASE_ADDR,256);
		}
#endif
	}

	if( arg == 1 ){
		g_vpp.govw_vbis_cnt++;
		if( vpp_check_dbg_level(VPP_DBGLVL_INT) ){
		    static struct timeval pre_tv;
		    struct timeval tv;
		    unsigned int tm_usec,fps;

			do_gettimeofday(&tv);
			tm_usec=(tv.tv_sec==pre_tv.tv_sec)? (tv.tv_usec-pre_tv.tv_usec):(1000000*(tv.tv_sec-pre_tv.tv_sec)+tv.tv_usec-pre_tv.tv_usec);
			fps = 1000000 / tm_usec;
			if( fps < (p_govw->fb_p->framerate - 10) ){
				DPRINT("[VPP] *W* fps %d,ori %d\n",fps,p_govw->fb_p->framerate);
			}
			pre_tv = tv;
		}
	}
}
	
void vpp_irqproc_govw_vbie(int arg)
{
	if( arg == 0 ){
#ifdef CONFIG_VPP_VBIE_FREE_MB
		vpp_free_y_addr = vpp_pre_dispfb_y_addr;
		vpp_pre_dispfb_y_addr = 0;
		vpp_free_c_addr = vpp_pre_dispfb_c_addr;
		vpp_pre_dispfb_c_addr = 0;
#endif
#ifdef CONFIG_GOVW_FBSWAP_VBIE
		vpp_govw_int_routine();
		vpp_disp_fb_isr();
#endif
#ifdef PATCH_VPU_HALT_GOVW_TG_ERR
#ifdef PATCH_VPU_HALT_GOVW_TG_ERR1
		if( vpp_govw_tg_err_patch ){
			unsigned int resx;
			unsigned int temp;
			unsigned int w_ypxlwid;

			vppif_reg32_write(VPU_R_YPXLWID,1);
			w_ypxlwid = vppif_reg32_read(VPU_W_YPXLWID);
			vppif_reg32_write(VPU_W_YPXLWID,1);

			vppif_reg32_write(GOVW_HD_MIF_ENABLE,1);

			resx = p_vpu->fb_p->fb.img_w;
			if(resx % 2) resx -= 1;
			vppif_reg32_write(VPU_HXWIDTH,resx);
			vppif_reg32_write(VPU_R_YPXLWID,resx);
			vppif_reg32_write(VPU_W_YPXLWID,w_ypxlwid);
			temp = resx * 16 / p_vpu->resx_visual;
			vppif_reg32_write(VPU_H_STEP,temp);
			temp = (resx * 16) % p_vpu->resx_visual;
			vppif_reg32_write(VPU_H_SUBSTEP,temp);
			vppif_reg32_write(VPU_HBI_MODE,3);

			vpp_govw_tg_err_patch = 0;
			// if( (vpp_govw_tg_err_patch_cnt % 10000) == 0 ){
				DPRINT("[GOVW TG INT] 2/3 scale,ret(VBIE),%d,resx %d,vis %d\n"
							,vpp_govw_tg_err_patch_cnt,resx,p_vpu->resx_visual);
			//}
			vpp_govw_tg_err_patch_cnt++;
			// vpp_reg_dump(VPU_BASE_ADDR,256);
		}
#endif
#ifdef PATCH_VPU_HALT_GOVW_TG_ERR2
		if( vpp_govw_tg_err_patch == 1 ){
			vpp_govw_tg_err_patch = 2;
		}
#endif
		if( vpp_govm_path ){
			govm_set_in_path(vpp_govm_path,1);
			vpp_govm_path = 0;
			DPRINT("[GOVW TG INT] Off GOVM path,On(VBIE)\n");
		}
#endif
	}

	if( arg == 1 ){
#ifdef CONFIG_GOVW_FPS_AUTO_ADJUST
		vpp_govw_dynamic_tg(0);
#endif
		vpp_dbg_show(VPP_DBGLVL_DIAG,3,"GOVW VBIE");
	}
}

#ifdef WMT_FTBLK_DISP /* sw patch : interlace tv mode field swap */
void vpp_irqproc_disp_vbis(int arg)
{
	if( arg == 0 ){
		if( vppif_reg32_read(DISP_INPUT_FIELD) ){
			if( disp_get_cur_field() ){
				vppif_reg32_out(GOVRH_BASE1_ADDR+0xe8,0x2);					
			}
			else {
				vppif_reg32_out(GOVRH_BASE1_ADDR+0xe8,0x3);
			}
		}
		vout_plug_detect(VOUT_SD_ANALOG);
	}
}
#endif

#if 0
int vpp_irqproc_enable_vpu_path_cnt;
int vpp_irqproc_enable_vpu_path(void *arg)
{
	vpp_irqproc_enable_vpu_path_cnt--;
	if( vpp_irqproc_enable_vpu_path_cnt )
		return 1;
	
	vppif_reg32_write(GOVM_VPU_SOURCE, 1);
	DPRINT("[VPP] irqproc enable vpu path\n");
	return 0;
}
#endif

#ifdef WMT_FTBLK_DISP
int vpp_irqproc_disable_disp(void *arg)
{
	if( disp_get_cur_field() ){
		vppif_reg32_write(DISP_EN,0);
		return 0;
	}
	return 1;
}
#endif

int vpp_irqproc_enable_vpu(void *arg)
{
	vpu_set_tg_enable((vpp_flag_t)arg);
	return 0;
}

int vpp_irqproc_monitor_dac_sense(void *arg)
{
	unsigned int *sense;

	sense = (unsigned int *) arg;
	*sense = govrh_DAC_monitor_sense();
	return 0;
}

/*!*************************************************************************
* vpp_interrupt_routine()
* 
* Private Function by Sam Shen, 2009/01/06
*/
/*!
* \brief	vpp interrupt routine
*		
* \retval  None
*/ 
static irqreturn_t vpp_interrupt_routine
(
	int irq, 				/*!<; // irq id */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
	void *dev_id 			/*!<; // device id */
#else	
	void *dev_id, 			/*!<; // device id */
	struct pt_regs *regs	/*!<; // reg pointer */
#endif
)
{
	vpp_int_t int_sts;

	switch(irq){
#ifdef WMT_FTBLK_VPU
		case VPP_IRQ_VPU:
			int_sts = p_vpu->get_sts();
			p_vpu->clr_sts(int_sts);
            vpp_dbg_show_val1(VPP_DBGLVL_INT,0,"[VPP] VPU INT",int_sts);
			g_vpp.govw_skip_frame = p_vpu->skip_fb;
			if( int_sts & VPP_INT_ERR_VPUW_MIFY )
				g_vpp.vpu_y_err_cnt++;
			if( int_sts & VPP_INT_ERR_VPUW_MIFC ){
				g_vpp.vpu_c_err_cnt++;
				if(p_vpu->underrun_cnt){
					p_vpu->underrun_cnt--;
					if(p_vpu->underrun_cnt==0){
						// g_vpp.govw_skip_all = 1;
						g_vpp.govw_fb_cnt = 1;
						g_vpp.vpu_skip_all = 1;
						DPRINT("[VPP] *E* skip all GOVW & VPU fb\n");
					}
				}
			}
			break;
#endif
		case VPP_IRQ_VPPM:	/* VPP */
			int_sts = p_vppm->get_sts();
			g_vpp.vga_intsts = vppif_reg32_in(REG_VPP_INTSTS);
			p_vppm->clr_sts(int_sts+VPP_INT_GOVRH_VBIE);
			{
				int i;
				unsigned int mask;
				vpp_irqproc_t *irqproc;

#if 1
				if( int_sts & VPP_INT_GOVW_PVBI ){
					if( int_sts & VPP_INT_GOVW_VBIS ){
						int_sts &= ~VPP_INT_GOVW_PVBI;
					}
				}
#endif
				for(i=0,mask=0x1;(i<32) && int_sts;i++,mask<<=1){
					if( (int_sts & mask) == 0 )
						continue;

					if( (irqproc = vpp_irqproc_array[i]) ){
						irqproc->proc(0);	// pre irq handle
						
						if( list_empty(&irqproc->list) == 0 )
							tasklet_schedule(&irqproc->tasklet);

						irqproc->proc(1);	// post irq handle
						
						if( vpp_check_dbg_level(VPP_DBGLVL_INT) ){
							if( mask == VPP_INT_GOVW_PVBI ){
								if( int_sts & VPP_INT_GOVW_VBIS ){
									DPRINT("[VPP] *W* pvbi over vbi (hw)\n");
								}
								else if( vppif_reg32_in(REG_VPP_INTSTS) & BIT1 ){
									DPRINT("[VPP] *W* pvbi over vbi (sw)\n");
								}
							}
						}
					}
					else {
						irqproc = vpp_irqproc_array[31];
						if( list_empty(&irqproc->list) == 0 ){
							vpp_proc_t *entry;
							struct list_head *ptr;

							ptr = (&irqproc->list)->next;
							entry = list_entry(ptr,vpp_proc_t,list);
							if( entry->type == mask )
								tasklet_schedule(&irqproc->tasklet);
						}							
					}
					int_sts &= ~mask;
				}
			}
			break;
#ifdef WMT_FTBLK_SCL
		case VPP_IRQ_SCL:	/* SCL */
			int_sts = p_scl->get_sts();
			p_scl->clr_sts(int_sts);
            vpp_dbg_show_val1(VPP_DBGLVL_INT,0,"[VPP] SCL INT",int_sts);
			break;
#endif			
		case VPP_IRQ_GOVM:	/* GOVM */
			int_sts = p_govm->get_sts();
			p_govm->clr_sts(int_sts);
			vpp_dbg_show_val1(VPP_DBGLVL_INT,0,"[VPP] GOVM INT",int_sts);
			break;
		case VPP_IRQ_GOVW:	/* GOVW */
			int_sts = p_govw->get_sts();
			vpp_dbg_show_val1(VPP_DBGLVL_INT,0,"[VPP] GOVW INT",int_sts);
			g_vpp.govw_tg_err_cnt++;			
			if( int_sts & VPP_INT_ERR_GOVW_TG ){
				int flag = 0;
				
				if( vppif_reg32_read(GOVM_INTSTS_VPU_READY) == 0 ){
					g_vpp.govw_vpu_not_ready_cnt++;
					flag = 1;
				}
				if( vppif_reg32_read(GOVM_INTSTS_GE_READY) == 0 ){
					g_vpp.govw_ge_not_ready_cnt++;
					flag = 1;
				}
#ifdef PATCH_VPU_HALT_GOVW_TG_ERR
				if( flag && (vpp_govm_path==0)&&(vpp_govw_tg_err_patch==0)){
					if( vppif_reg32_read(VPU_HBI_MODE) == 3){
						unsigned int temp;
						unsigned int resx;

						vppif_reg32_write(GOVW_HD_MIF_ENABLE,0);

						vppif_reg32_out(REG_VPU_HTB0,0x1C);
						vppif_reg32_out(REG_VPU_HPTR,0x2);
						vppif_reg32_out(REG_VPU_HRES_TB,0x2);
						resx = p_vpu->resx_visual;
						if( p_vpu->resx_visual % 2 ){
							resx *= 2;
						}
						temp = resx * 3 / 2;
						vppif_reg32_write(VPU_HXWIDTH,temp);
						vppif_reg32_write(VPU_R_YPXLWID,temp);
						temp = temp * 16 / resx;
						vppif_reg32_write(VPU_H_STEP,temp);
						vppif_reg32_write(VPU_HBI_MODE,0);
						vpp_govw_tg_err_patch = 1;
					}
					else {
						vpp_govw_tg_err_patch = 0;
						vpp_govm_path = vpp_get_govm_path();
						govm_set_in_path(vpp_govm_path,0);
#if 0
						vpp_vpu_sw_reset();
#endif
					}
				}
#endif				
				if( vpp_check_dbg_level(VPP_DBGLVL_INT) ){
					DPRINT("[VPP] GOVW TG err %d,GE %d,VPU %d\n",g_vpp.govw_tg_err_cnt,
						g_vpp.govw_ge_not_ready_cnt,g_vpp.govw_vpu_not_ready_cnt);
				}
			}
			p_govw->clr_sts(int_sts);
#ifdef CONFIG_VPP_GOVW_TG_ERR_DROP_FRAME
			if( vpp_disp_fb_cnt(&vpp_disp_fb_list) > 1 ){
				vpp_disp_fb_isr();		// drop display frame when bandwidth not enouth
				g_vpp.disp_skip_cnt++;
				vpp_dbg_show(VPP_DBGLVL_DISPFB,0,"skip disp fb");
			}
#endif
#ifdef CONFIG_GOVW_FPS_AUTO_ADJUST
			else {
				vpp_govw_dynamic_tg(1);
			}
#endif
#ifdef VPP_DBG_DIAG_NUM
			vpp_dbg_show(VPP_DBGLVL_DIAG,3,"GOVW Err");
			vpp_dbg_diag_delay = 10;
#endif
			g_vpp.govw_skip_frame = 3;
			break;
#ifdef WMT_FTBLK_GOVRH
		case VPP_IRQ_GOVR:	/* GOVR */
			int_sts = p_govrh->get_sts();
			p_govrh->clr_sts(int_sts);
			vpp_dbg_show_val1(VPP_DBGLVL_INT,0,"[VPP] GOVR INT",int_sts);
			g_vpp.govr_underrun_cnt++;
#ifdef VPP_DBG_DIAG_NUM
			vpp_dbg_show(VPP_DBGLVL_DIAG,3,"GOVR MIF Err");
			vpp_dbg_diag_delay = 10;
#endif
			break;
#endif			
		default:
			DPRINT("*E* invalid vpp isr\n");
			break;
	}
	return IRQ_HANDLED;
} /* End of vpp_interrupt_routine */

void vpp_set_path(int arg,vdo_framebuf_t *fb)
{
	if( g_vpp.vpp_path == arg )
		return;

	g_vpp.vpp_path_no_ready = 1;

	DPRINT("[VPP] vpp_set_path(%s-->%s)\n",vpp_vpath_str[g_vpp.vpp_path],vpp_vpath_str[arg]);
	vpp_disp_fb_clr(0,1);

	// pre video path process
	switch( g_vpp.vpp_path ){
		case VPP_VPATH_GOVW:	// VPU,GE fb -> Queue -> VPU,GE -> GOVW -> GOVW fb -> GOVR fb -> GOVR
			// govw disable
			vpp_set_govw_tg(VPP_FLAG_DISABLE);
			g_vpp.vpp_path_ori_fb = g_vpp.govr->fb_p->fb;
			break;
		case VPP_VPATH_SCL:		// VPU fb -> Queue -> SCL -> SCL fb -> Queue -> GOVR fb -> GOVR
			if( g_vpp.scl_fb_mb_on_work ){
				extern int scl_scale_complete;
				
				p_scl->scale_finish();
				scl_scale_complete = 0;
				g_vpp.scl_fb_mb_on_work = 0;
				DPRINT("[VPP] wait scl complete\n");
			}
		case VPP_VPATH_GOVR:	// VPU fb -> Queue -> GOVR fb -> GOVR
		case VPP_VPATH_GE:		// GE fb -> pan display -> GOVR fb -> GOVR
			if( arg != VPP_VPATH_GOVW ){
				p_govrh->fb_p->fb = g_vpp.vpp_path_ori_fb;
				vpp_clr_framebuf(VPP_MOD_GOVRS);
				vpp_irqproc_work(VPP_INT_GOVRH_PVBI,(void *)vpp_irqproc_chg_path,0,1);
			}
			break;
		case VPP_VPATH_VPU:		// VPU fb -> Queue -> VPU -> GOVR
			vpu_set_direct_path(VPP_FLAG_DISABLE);
			govw_set_tg_enable(VPP_FLAG_ENABLE);
			govrh_set_MIF_enable(VPP_FLAG_ENABLE);
			vpp_set_vppm_int_enable(VPP_INT_GOVRH_PVBI+VPP_INT_GOVRH_VBIS,0);
			break;
		default:
			break;
	}

	g_vpp.vpp_path = arg;

	// new video path process
	switch( g_vpp.vpp_path ){
		case VPP_VPATH_SCL:
			g_vpp.scl_fb_mb_clear = 0xFF;
#ifdef CONFIG_VPP_GOVW_FRAMEBUF
			g_vpp.scl_fb_mb_index = (g_vpp.govr->fb_p->fb.y_addr == g_vpp.mb_govw[0])? 1:2;
#else
			g_vpp.scl_fb_mb_index = (g_vpp.govr->fb_p->fb.y_addr == g_vpp.mb[0])? 1:2;
#endif
		case VPP_VPATH_GOVR:
		case VPP_VPATH_GE:
#ifdef CONFIG_VPP_GOVRH_FBSYNC
			vpp_fbsync_cal_fps();
#endif

			// pvbi should more than ###
			if( vppif_reg32_read(GOVRH_PVBI_LINE) < 5 ){
				vppif_reg32_write(GOVRH_PVBI_LINE,5);
			}
			vpp_set_vppm_int_enable(VPP_INT_GOVRH_PVBI+VPP_INT_GOVRH_VBIS,VPP_FLAG_ENABLE);

			// set ge fb to govrh
			if( fb ){
				if( (fb->img_w == p_govrh->fb_p->fb.img_w) && (fb->img_h == p_govrh->fb_p->fb.img_h) ){
					p_govrh->fb_p->fb = *fb;
					vpp_irqproc_work(VPP_INT_GOVRH_PVBI,(void *)vpp_irqproc_chg_path,0,1);
				}
			}
			break;
		case VPP_VPATH_VPU:
#ifdef CONFIG_VPP_GOVRH_FBSYNC
			vpp_fbsync_cal_fps();
#endif
			vpu_set_direct_path(VPP_FLAG_ENABLE);
			vpp_wait_vsync();
			vpp_wait_vsync();
			vpp_wait_vsync();
			govw_set_tg_enable(VPP_FLAG_DISABLE);
			govrh_set_MIF_enable(VPP_FLAG_DISABLE);

			vppif_reg32_out(MEMORY_CTRL_V4_CFG_BASE_ADDR+0x8,0x600000);
			vppif_reg32_out(MEMORY_CTRL_V4_CFG_BASE_ADDR+0xC,0x0ff00000);
			vpp_set_vppm_int_enable(VPP_INT_GOVRH_PVBI+VPP_INT_GOVRH_VBIS,1);
			break;
		case VPP_VPATH_GOVW:
			g_vpp.govw_skip_all = 1;

			p_govrh->fb_p->fb = g_vpp.vpp_path_ori_fb;

			vppif_reg32_write(GOVM_VPU_SOURCE,0);

			// govw enable
			vpp_lock();
			vpp_set_govw_tg(VPP_FLAG_ENABLE);
			vpp_unlock();

			vpp_irqproc_work(VPP_INT_GOVW_VBIE,0,0,1);
			vpp_vpu_sw_reset();
			vpp_irqproc_work(VPP_INT_GOVW_VBIE,0,0,1);
			vppif_reg32_write(GOVM_VPU_SOURCE,1);

			// update govrh fb
			govw_set_hd_fb_addr(p_govrh->fb_p->fb.y_addr, p_govrh->fb_p->fb.c_addr);
			vpp_irqproc_work(VPP_INT_GOVW_VBIE,0,0,1);
			
			// update govw fb
			govw_set_hd_fb_addr(p_govw->fb_p->fb.y_addr, p_govw->fb_p->fb.c_addr);
			vpp_irqproc_work(VPP_INT_GOVW_VBIE,0,0,1);

			vpp_irqproc_work(VPP_INT_GOVRH_PVBI,(void *)vpp_irqproc_chg_path,0,1);
			vpp_set_vppm_int_enable(VPP_INT_GOVRH_PVBI+VPP_INT_GOVRH_VBIS,VPP_FLAG_DISABLE);
			g_vpp.govw_skip_all = 0;
			break;
		default:
			break;
	}
	g_vpp.vpp_path_no_ready = 0;
}

/*!*************************************************************************
* vpp_alloc_framebuffer()
* 
* Private Function by Sam Shen, 2009/01/06
*/
/*!
* \brief	vpp alloc frame buffer
*		
* \retval  None
*/
int vpp_alloc_framebuffer(unsigned int resx,unsigned int resy)
{
	vdo_framebuf_t *fb;
	unsigned int y_size;
	unsigned int fb_size;
	unsigned int colfmt;
	int i;

	vpp_lock();
#ifdef CONFIG_VPP_GOVW_FRAMEBUF
	if( g_vpp.mb[0] ){
	}
	else {
		unsigned int mb_resx,mb_resy;

		// allocate mb
		mb_resx = VPP_HD_MAX_RESX;
		mb_resy = VPP_HD_MAX_RESY;
		colfmt = VPP_GOVW_FB_COLFMT;
		
		if( mb_resx % 64 ){
			mb_resx += (64 - (mb_resx % 64));
		}
		y_size = mb_resx * mb_resy;
		switch(colfmt){
			case VDO_COL_FMT_ARGB:
				fb_size = 4 * y_size;
				break;
			default:
			case VDO_COL_FMT_YUV444:
				fb_size = 3 * y_size;
				break;
			case VDO_COL_FMT_YUV422H:
				fb_size = 2 * y_size;
				break;
			case VDO_COL_FMT_YUV420:
				fb_size = 3 * y_size / 2;
				break;
		}
		y_size = (colfmt==VDO_COL_FMT_ARGB)? (fb_size):(y_size);
		g_vpp.mb_fb_size = fb_size;
		g_vpp.mb_y_size = y_size;

		for(i=0;i<VPP_MB_ALLOC_NUM;i++){
			g_vpp.mb[i] = mb_alloc(fb_size);
//			DPRINT("[VPP] alloc mb 0x%x,fb %d,y %d\n",g_vpp.mb[i],fb_size,y_size);
		}
	}

	if( resx % 64 ){
		resx += (64 - (resx % 64));
	}
	y_size = resx * resy;
	// colfmt = g_vpp.govr->fb_p->fb.col_fmt;
	colfmt = VPP_GOVW_FB_COLFMT;
	switch(colfmt){
		case VDO_COL_FMT_ARGB:
			fb_size = 4 * y_size;
			break;
		default:
		case VDO_COL_FMT_YUV444:
			fb_size = 3 * y_size;
			break;
		case VDO_COL_FMT_YUV422H:
			fb_size = 2 * y_size;
			break;
		case VDO_COL_FMT_YUV420:
			fb_size = 3 * y_size / 2;
			break;
	}
	y_size = (colfmt==VDO_COL_FMT_ARGB)? (fb_size):(y_size);
	g_vpp.mb_govw_y_size = y_size;
{
	int index = 0,offset = 0;
	unsigned int size = g_vpp.mb_fb_size;

	g_vpp.mb_govw_cnt = VPP_GOV_MB_ALLOC_NUM;
	for(i=0;i<VPP_GOV_MB_ALLOC_NUM;i++){
		if( size < fb_size ){
			index++;
			if( index >= VPP_MB_ALLOC_NUM ){
				index = 0;
				g_vpp.mb_govw_cnt = i;
				break;
			}
			offset=0;
			size = g_vpp.mb_fb_size;
		}
		g_vpp.mb_govw[i] = g_vpp.mb[index] + offset;
		size -= fb_size;
		offset += fb_size;
//		DPRINT("[VPP] govw mb %d 0x%x\n",i,g_vpp.mb_govw[i]);
	}
}
#else
#ifdef CONFIG_VPP_FIX_BUFFER
	if( g_vpp.mb[0] ){
		vpp_unlock();
		return 0;
	}

	resx = VPP_HD_MAX_RESX;
	resy = VPP_HD_MAX_RESY;
	colfmt = VPP_GOVW_FB_COLFMT;
#else
	if( g_vpp.mb[0] ){
		if( (g_vpp.govr->fb_p->fb.fb_w == resx) && (g_vpp.govr->fb_p->fb.fb_h == resy) ){
			return 0;
		}
		for(i=0;i<VPP_MB_ALLOC_NUM;i++){
			mb_free(g_vpp.mb[i]);
		}
	}
	colfmt = g_vpp.govr->fb_p->fb.col_fmt;
#endif	

	if( resx % 64 ){
		resx += (64 - (resx % 64));
	}
	y_size = resx * resy;
#ifdef CONFIG_VPP_FIX_BUFFER
	switch(colfmt){
		case VDO_COL_FMT_ARGB:
			fb_size = 4 * y_size;
			break;
		default:
		case VDO_COL_FMT_YUV444:
			fb_size = 3 * y_size;
			break;
		case VDO_COL_FMT_YUV422H:
			fb_size = 2 * y_size;
			break;
		case VDO_COL_FMT_YUV420:
			fb_size = 3 * y_size / 2;
			break;
	}
#else
	fb_size = (colfmt==VDO_COL_FMT_ARGB)? (4 * y_size):(3 * y_size);
#endif
	y_size = (colfmt==VDO_COL_FMT_ARGB)? (fb_size):(y_size);
	g_vpp.mb_y_size = y_size;

#ifdef CONFIG_VPP_DUAL_BUFFER
	for(i=0;i<VPP_MB_ALLOC_NUM;i++){
		g_vpp.mb[i] = mb_alloc(fb_size);
	}
#else
	g_vpp.mb[1] = g_vpp.mb[0];
#endif
#endif

	fb = &p_govw->fb_p->fb;
#ifdef CONFIG_VPP_GOVW_FRAMEBUF
	fb->y_addr = g_vpp.mb_govw[0];
	fb->c_addr = g_vpp.mb_govw[0] + y_size;
#else
	fb->y_addr = g_vpp.mb[0];
	fb->c_addr = g_vpp.mb[0] + y_size;
#endif
	fb->y_size = y_size;
	fb->c_size = fb_size - y_size;
	fb->fb_w = resx;
	fb->fb_h = resy;

	fb = &g_vpp.govr->fb_p->fb;
#ifdef CONFIG_VPP_GOVW_FRAMEBUF
	fb->y_addr = g_vpp.mb_govw[1];
	fb->c_addr = g_vpp.mb_govw[1] + y_size;
#else
	fb->y_addr = g_vpp.mb[1];
	fb->c_addr = g_vpp.mb[1] + y_size;
#endif
	fb->y_size = y_size;
	fb->c_size = fb_size - y_size;
	fb->fb_w = resx;
	fb->fb_h = resy;

#if 1
	if( g_vpp.chg_res_blank ){
		int i;
		unsigned int *ptr;
		unsigned int yaddr,caddr;
		
	//	vpp_dbg_show(VPP_DBGLVL_ALL,1,"clr fb begin");
		g_vpp.govw_skip_all = 1;

		govw_get_hd_fb_addr(&yaddr,&caddr);
//		DPRINT("[VPP] govw fb y(0x%x),c(0x%x)\n",yaddr,caddr);
//		DPRINT("[VPP] clr fb y(0x%x,%d),c(0x%x,%d)\n",fb->y_addr,fb->y_size,fb->c_addr,fb->c_size); mdelay(100);

		ptr = mb_phys_to_virt(fb->y_addr);
		for(i=0;i<fb->y_size;i+=4,ptr++)
			*ptr = 0x0;
		ptr = mb_phys_to_virt(fb->c_addr);
		for(i=0;i<fb->c_size;i+=4,ptr++)
			*ptr = 0x80808080;
	//	memset(phys_to_virt(fb->y_addr),0,fb->y_size);
	//	memset(phys_to_virt(fb->c_addr),0,fb->c_size);
	//	vpp_dbg_show(VPP_DBGLVL_ALL,1,"clr fb end");

	}
#endif

//	DBGMSG("alloc frame buf %dx%d, size %d\n",resx,resy,fb_size);
//	DBGMSG("mb0 0x%x,mb1 0x%x\n",g_vpp.mb[0],g_vpp.mb[1]);
	vpp_unlock();
	return 0;
} /* End of vpp_alloc_framebuffer */

/*!*************************************************************************
* vpp_get_sys_parameter()
* 
* Private Function by Sam Shen, 2010/01/27
*/
/*!
* \brief	vpp device initialize
*		
* \retval  None
*/ 
void vpp_get_sys_parameter(void)
{
	unsigned char buf[40];
	int varlen = 40;
	extern int wmt_getsyspara(char *varname, unsigned char *varval, int *varlen);

	// vpp attribute by default
	g_vpp.disp_fb_max = VPP_DISP_FB_NUM;
	g_vpp.disp_fb_cnt = 0;
#ifdef CONFIG_GOVW_FPS_AUTO_ADJUST
	g_vpp.govw_tg_dynamic = 0;
#endif
	g_vpp.video_quality_mode = 1;
	g_vpp.scale_keep_ratio = 1;
	g_vpp.govrh_field = VPP_FIELD_BOTTOM;
	g_vpp.disp_fb_keep = 0;
#ifdef CONFIG_VPP_GOVRH_FBSYNC
	g_vpp.fbsync_enable = 1;
#endif
	g_vpp.hdmi_audio_interface = VPP_HDMI_AUDIO_SPDIF;
	g_vpp.hdmi_cp_enable = 1;
	g_vpp.govw_vfp = 5;
	g_vpp.govw_vbp = 5;
	g_vpp.vpp_path = VPP_VPATH_GE;
	g_vpp.vpu_path = VPP_VPATH_GOVW;
#ifdef CONFIG_VPP_GE_DIRECT_PATH
	g_vpp.ge_path = VPP_VPATH_GE;
#else
	g_vpp.ge_path = VPP_VPATH_GOVW;
#endif
#ifdef WMT_FTBLK_GOVRH
	g_vpp.govr = (vpp_mod_base_t*) p_govrh;
#elif defined(WMT_FTBLK_LCDC)
	g_vpp.govr = (vpp_mod_base_t*) p_lcdc;
#endif

#if 0
	if( wmt_getsyspara("wmt.display.direct_path",buf,&varlen) == 0 ){
		sscanf(buf,"%d",&g_vpp.direct_path);
		DPRINT("[VPP] direct path %d\n",g_vpp.direct_path);
	}
#endif

	if( wmt_getsyspara("wmt.display.hdmi_audio_inf",buf,&varlen) == 0 ){
		if( memcmp(buf,"i2s",3) == 0 ){
//			DPRINT("[VPP] hdmi audio i2s\n");
			g_vpp.hdmi_audio_interface = VPP_HDMI_AUDIO_I2S;
		}
		else if( memcmp(buf,"spdif",5) == 0 ){
//			DPRINT("[VPP] hdmi audio spdif\n");
			g_vpp.hdmi_audio_interface = VPP_HDMI_AUDIO_SPDIF;
		}	
	}

	if( wmt_getsyspara("wmt.display.hdmi.vmode",buf,&varlen) == 0 ){
		if( memcmp(buf,"720p",4) == 0 ){
			g_vpp.hdmi_video_mode = 720;
		}
		else if( memcmp(buf,"1080p",5) == 0 ){
			g_vpp.hdmi_video_mode = 1080;
		}
		else {
			g_vpp.hdmi_video_mode = 0;
		}
		DPRINT("[VPP] HDMI video mode %d\n",g_vpp.hdmi_video_mode);
	}
} /* End of vpp_get_sys_parameter */

/*----------------------- Linux Kernel interface --------------------------------------*/
/*!*************************************************************************
* vpp_dev_init()
* 
* Private Function by Sam Shen, 2009/01/06
*/
/*!
* \brief	vpp device initialize
*		
* \retval  None
*/ 
int vpp_dev_init(void)
{
	int i;
	vout_info_t info;

	// vpp attribute by uboot parameter
	vpp_get_sys_parameter();

	g_vpp.alloc_framebuf = vpp_alloc_framebuffer;

	// init module
//	p_scl->int_catch = VPP_INT_ERR_SCL_TG | VPP_INT_ERR_SCLR1_MIF | VPP_INT_ERR_SCLR2_MIF 
//						| VPP_INT_ERR_SCLW_MIFRGB | VPP_INT_ERR_SCLW_MIFY |	VPP_INT_ERR_SCLW_MIFC;
#ifdef CONFIG_GOVW_FBSWAP_VBIE
	p_vppm->int_catch = VPP_INT_GOVW_VBIS | VPP_INT_GOVW_VBIE;
#else
	p_vppm->int_catch = VPP_INT_GOVW_PVBI | VPP_INT_GOVW_VBIS | VPP_INT_GOVW_VBIE;
#endif

#ifdef CONFIG_VPP_VBIE_FREE_MB
	p_vppm->int_catch |= VPP_INT_GOVW_VBIE;
#endif

#ifdef CONFIG_VPP_GOVW_TG_ERR_DROP_FRAME			
	p_govw->int_catch = VPP_INT_ERR_GOVW_TG; // | VPP_INT_ERR_GOVW_MIFY | VPP_INT_ERR_GOVW_MIFC;
#endif
//	p_govm->int_catch = VPP_INT_ERR_GOVM_VPU | VPP_INT_ERR_GOVM_GE;

#if 0
	/* check govrh preinit for uboot logo function */
#ifdef WMT_FTBLK_GOVRH
	g_vpp.govrh_preinit = vppif_reg32_read(GOVRH_MIF_ENABLE);
	if( g_vpp.govrh_preinit ){
		p_vppm->int_catch &= ~VPP_INT_GOVW_PVBI;
	}
	
	if( g_vpp.vpp_path ){
	    p_vppm->int_catch |= (VPP_INT_GOVRH_PVBI+VPP_INT_GOVRH_VBIS);
	}

#ifdef PATCH_GOVRH_VSYNC_OVERLAP
	p_vppm->int_catch |= VPP_INT_GOVRH_VBIE;
#endif	
#else	
	g_vpp.govrh_preinit = 0;
#endif
	DPRINT("vpp_init(boot logo %d)\n",g_vpp.govrh_preinit);
#endif

	vpp_init(&info);
	vfb_var.xres = info.resx;
	vfb_var.yres = info.resy;
	vfb_var.pixclock = KHZ2PICOS(info.pixclk/1000);

	// init disp fb queue
	INIT_LIST_HEAD(&vpp_disp_free_list);
	INIT_LIST_HEAD(&vpp_disp_fb_list);
#ifdef WMT_FTBLK_PIP
	INIT_LIST_HEAD(&vpp_pip_fb_list);
#endif
	for(i=0;i<VPP_DISP_FB_MAX;i++)
		list_add_tail(&vpp_disp_fb_array[i].list,&vpp_disp_free_list);

#ifdef CONFIG_SCL_DIRECT_PATH 
	vpp_scl_fb_init();
#endif

#ifdef CONFIG_VPP_INTERRUPT
	// init irq proc	
	INIT_LIST_HEAD(&vpp_free_list);

	vpp_irqproc_new(VPP_INT_MAX,0);
#ifdef WMT_FTBLK_GOVRH
	vpp_irqproc_new(VPP_INT_GOVRH_PVBI,vpp_irqproc_govrh_pvbi);
	vpp_irqproc_new(VPP_INT_GOVRH_VBIS,vpp_irqproc_govrh_vbis);
#ifdef PATCH_GOVRH_VSYNC_OVERLAP
	vpp_irqproc_new(VPP_INT_GOVRH_VBIE,vpp_irqproc_govrh_vbie);
#endif	
#endif
	vpp_irqproc_new(VPP_INT_GOVW_PVBI,vpp_irqproc_govw_pvbi);
	vpp_irqproc_new(VPP_INT_GOVW_VBIS,vpp_irqproc_govw_vbis);
	vpp_irqproc_new(VPP_INT_GOVW_VBIE,vpp_irqproc_govw_vbie);
#ifdef WMT_FTBLK_DISP
	vpp_irqproc_new(VPP_INT_DISP_VBIS,vpp_irqproc_disp_vbis);
#endif

	for(i=0;i<VPP_PROC_NUM;i++)
		list_add_tail(&vpp_proc_array[i].list,&vpp_free_list);

	// init interrupt service routine
#ifdef WMT_FTBLK_SCL
	if ( vpp_request_irq(VPP_IRQ_SCL, vpp_interrupt_routine, SA_INTERRUPT, "scl", (void *)&g_vpp) ) {
		DPRINT("*E* request VPP ISR fail\n");
		return -1;
	}
#endif	

	if ( vpp_request_irq(VPP_IRQ_VPPM, vpp_interrupt_routine, SA_INTERRUPT, "vpp", (void *)&g_vpp) ) {
		DPRINT("*E* request VPP ISR fail\n");
		return -1;
	}

	if ( vpp_request_irq(VPP_IRQ_GOVM, vpp_interrupt_routine, SA_INTERRUPT, "govm", (void *)&g_vpp) ) {
		DPRINT("*E* request VPP ISR fail\n");
		return -1;
	}

	if ( vpp_request_irq(VPP_IRQ_GOVW, vpp_interrupt_routine, SA_INTERRUPT, "govw", (void *)&g_vpp) ) {
		DPRINT("*E* request VPP ISR fail\n");
		return -1;
	}

#ifdef WMT_FTBLK_GOVRH
	if ( vpp_request_irq(VPP_IRQ_GOVR, vpp_interrupt_routine, SA_INTERRUPT, "govr", (void *)&g_vpp) ) {
		DPRINT("*E* request VPP ISR fail\n");
		return -1;
	}
#endif	
#ifdef WMT_FTBLK_VPU
	if ( vpp_request_irq(VPP_IRQ_VPU, vpp_interrupt_routine, SA_INTERRUPT, "vpu", (void *)&g_vpp) ) {
		DPRINT("*E* request VPU ISR fail\n");
		return -1;
	}
#endif	
#endif

#ifdef CONFIG_VPP_PROC
	// init system proc
	if( vpp_proc_dir == 0 ){
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
		struct proc_dir_entry *res;
		
		vpp_proc_dir = proc_mkdir("driver/vpp", NULL);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32)
		vpp_proc_dir->owner = THIS_MODULE;
#endif
		
		res=create_proc_entry("sts", 0, vpp_proc_dir);
	    if (res) {
    	    res->read_proc = vpp_sts_read_proc;
	    }
		res=create_proc_entry("reg", 0, vpp_proc_dir);
	    if (res) {
    	    res->read_proc = vpp_reg_read_proc;
	    }
		res=create_proc_entry("info", 0, vpp_proc_dir);
	    if (res) {
    	    res->read_proc = vpp_info_read_proc;
	    }
		vpp_table_header = register_sysctl_table(vpp_root_table);
#else
		
		vpp_proc_dir = proc_mkdir("driver/vpp", NULL);
		vpp_proc_dir->owner = THIS_MODULE;
		create_proc_info_entry("sts", 0, vpp_proc_dir, vpp_sts_read_proc);
		create_proc_info_entry("reg", 0, vpp_proc_dir, vpp_reg_read_proc);
		create_proc_info_entry("info", 0, vpp_proc_dir, vpp_info_read_proc);		

		vpp_table_header = register_sysctl_table(vpp_root_table, 1);
#endif	
	}
#endif

#ifdef CONFIG_VPP_NOTIFY
	vpp_netlink_proc[0].pid=0;
	vpp_netlink_proc[1].pid=0;	
	vpp_nlfd = netlink_kernel_create(&init_net,NETLINK_CEC_TEST, 0, vpp_netlink_receive,NULL,THIS_MODULE);
	if(!vpp_nlfd){
		DPRINT(KERN_ERR "can not create a netlink socket\n");
	}
#endif
	return 0;
} /* End of vpp_dev_init */
module_init(vpp_dev_init);

/*!*************************************************************************
* vpp_exit()
* 
* Private Function by Sam Shen, 2009/01/06
*/
/*!
* \brief	vpp device exit
*		
* \retval  None
*/ 
int vpp_exit(struct fb_info *info)
{
	DBGMSG("vpp_exit\n");

	vout_exit();
	unregister_sysctl_table(vpp_table_header);

	return 0;
} /* End of vpp_exit */


unsigned int vpp_pmc_258_bk;
unsigned int vpp_pmc_25c_bk;
/*!*************************************************************************
* vpp_suspend()
* 
* Private Function by Sam Shen, 2009/01/06
*/
/*!
* \brief	vpp suspend
*		
* \retval  None
*/ 
int	vpp_suspend(int state)
{
	vpp_mod_base_t *mod_p;
	int i;

	DPRINT("vpp_suspend\n");
	vout_suspend(VOUT_MODE_ALL,state);

	// disable module
	for(i=0;i<VPP_MOD_MAX;i++){
		mod_p = vpp_mod_get_base(i);
		if( mod_p && mod_p->suspend ){
			mod_p->suspend(0);
		}
	}
#ifdef WMT_FTBLK_HDMI
	hdmi_suspend(0);
#endif
#ifdef WMT_FTBLK_LVDS
	lvds_suspend(0);
#endif
	// wait
	mdelay(100);

	// disable tg
	for(i=0;i<VPP_MOD_MAX;i++){
		if( i == VPP_MOD_GOVW )
			continue;
		
		mod_p = vpp_mod_get_base(i);
		if( mod_p && mod_p->suspend ){
			mod_p->suspend(1);
		}
	}
#ifdef WMT_FTBLK_HDMI
	hdmi_suspend(1);
#endif
#ifdef WMT_FTBLK_LVDS
	lvds_suspend(1);
#endif	
	// backup registers
	for(i=0;i<VPP_MOD_MAX;i++){
		mod_p = vpp_mod_get_base(i);
		if( mod_p && mod_p->suspend ){
			mod_p->suspend(2);
		}
	}
#ifdef WMT_FTBLK_HDMI
	hdmi_suspend(2);
#endif
#ifdef WMT_FTBLK_LVDS
	lvds_suspend(2);
#endif
	mdelay(100);
	p_govw->suspend(1);
	return 0;
} /* End of vpp_suspend */

/*!*************************************************************************
* vpp_resume()
* 
* Private Function by Sam Shen, 2009/01/06
*/
/*!
* \brief	vpp driver resume
*		
* \retval  None
*/ 
int	vpp_resume(void)
{
	vpp_mod_base_t *mod_p;
	int i;

	DPRINT("vpp_resume\n");

	// restore registers
	for(i=0;i<VPP_MOD_MAX;i++){
		mod_p = vpp_mod_get_base(i);
		if( mod_p && mod_p->resume ){
			mod_p->resume(0);
		}
	}
#ifdef WMT_FTBLK_LVDS
	lvds_resume(0);
#endif
#ifdef WMT_FTBLK_HDMI
	hdmi_resume(0);
#endif
	// enable tg
	for(i=0;i<VPP_MOD_MAX;i++){
		mod_p = vpp_mod_get_base(i);
		if( mod_p && mod_p->resume ){
			mod_p->resume(1);
		}
	}
#ifdef WMT_FTBLK_LVDS
	lvds_resume(1);
#endif
#ifdef WMT_FTBLK_HDMI
	hdmi_resume(1);
#endif
	// wait
	mdelay(100);

	// enable module
	for(i=0;i<VPP_MOD_MAX;i++){
		mod_p = vpp_mod_get_base(i);
		if( mod_p && mod_p->resume ){
			mod_p->resume(2);
		}
	}
#ifdef WMT_FTBLK_LVDS
	lvds_resume(2);
#endif
#ifdef WMT_FTBLK_HDMI
	hdmi_resume(2);
#endif
	vout_resume(VOUT_MODE_ALL,0);
	return 0;
} /* End of vpp_resume */

/*!*************************************************************************
* vpp_check_mmap_offset()
* 
* Private Function by Sam Shen, 2009/01/06
*/
/*!
* \brief	check mmap offset
*		
* \retval  None
*/ 
int vpp_check_mmap_offset(dma_addr_t offset)
{
	vdo_framebuf_t *fb;
	int i;

//	DBGMSG("vpp_check_mmap_offset 0x%x\r\n",offset);

	for(i=0;i<VPP_MOD_MAX;i++){
		fb = vpp_mod_get_framebuf(i);
		if( fb ){
			if( (offset >= fb->y_addr) && (offset < (fb->y_addr + fb->y_size))){
//				DBGMSG("mmap to mod %d Y frame buffer\r\n",i);
				return 0;
			}

			if( (offset >= fb->c_addr) && (offset < (fb->c_addr + fb->c_size))){
//				DBGMSG("mmap to mod %d C frame buffer\r\n",i);
				return 0;
			}
		}
	}
	return -1;
} /* End of vpp_check_mmap_offset */

/*!*************************************************************************
* vpp_mmap()
* 
* Private Function by Sam Shen, 2009/01/06
*/
/*!
* \brief	vpp driver mmap
*		
* \retval  None
*/ 
int vpp_mmap(struct vm_area_struct *vma)
{
//	DBGMSG("vpp_mmap\n");

	/* which buffer need to remap */
	if( vpp_check_mmap_offset(vma->vm_pgoff << PAGE_SHIFT) != 0 ){
		DPRINT("*E* vpp_mmap 0x%x\n",(int) vma->vm_pgoff << PAGE_SHIFT);
		return -EINVAL;
	}

//	DBGMSG("Enter vpp_mmap remap 0x%x\n",(int) (vma->vm_pgoff << PAGE_SHIFT));
	
	vma->vm_flags |= VM_IO | VM_RESERVED;
	vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
	if (remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff,vma->vm_end - vma->vm_start, vma->vm_page_prot))
		return -EAGAIN;

	return 0;
} /* End of vpp_mmap */

/*!*************************************************************************
* vpp_wait_vsync()
* 
* Private Function by Sam Shen, 2009/01/06
*/
/*!
* \brief	wait govw frame ready
*		
* \retval  None
*/ 
void vpp_wait_vsync(void)
{
//	if( g_vpp.direct_path || g_vpp.ge_direct_path ){
	if( vppif_reg32_read(GOVW_TG_ENABLE) == 0 ){
		vpp_irqproc_work(VPP_INT_GOVRH_VBIS,0,0,1);
	}
	else {
		vpp_irqproc_work(VPP_INT_GOVW_VBIS,0,0,1);
	}
	return;
} /* End of vpp_wait_vsync */

/*!*************************************************************************
* vpp_set_par()
* 
* Private Function by Sam Shen, 2009/01/06
*/
/*!
* \brief	vpp config for resolution change
*		
* \retval  None
*/ 
int vpp_set_par(struct fb_info *info)
{
	vout_info_t vo_info;

#if(WMT_SUB_PID == WMT_PID_8505)
	if( (info->var.xres > 1024) || (info->var.yres > 600) ){
		DPRINT("*E* WM8505 not support (%dx%d)\n",info->var.xres,info->var.yres);
		return -1;
	}
#endif

	vpp_set_mutex(1);
	if( g_vpp.chg_res_blank ){
		g_vpp.govw_skip_all = 1;
	}

	vpp_set_govw_tg(VPP_FLAG_DISABLE);

	if( g_vpp.chg_res_blank ){
		vpp_clr_framebuf(VPP_MOD_GOVRH);
	}

	g_vpp.resx = info->var.xres;
	g_vpp.resy = info->var.yres;
	vo_info.resx = info->var.xres;
	vo_info.resy = info->var.yres;
	if( info->var.pixclock > 1000000 ){	// ut_vpp will set pixclk not khz2picos
		vo_info.pixclk = info->var.pixclock;
	}
	else {
		vo_info.pixclk = KHZ2PICOS(info->var.pixclock) * 1000;
	}
	vo_info.bpp = info->var.bits_per_pixel;
	vo_info.fps = vo_info.pixclk / (info->var.xres * info->var.yres);

//	DPRINT("%s(%dx%d,%d)\n",__FUNCTION__,info->var.xres,info->var.yres,info->var.pixclock);
	vpp_config(&vo_info);

	if( (vo_info.resx != info->var.xres) || (vo_info.resy != info->var.yres) ){
		DBGMSG("vout mode update (%dx%d)\n",vo_info.resx,vo_info.resy);
		info->var.xres = vo_info.resx;
		info->var.yres = vo_info.resy;
	}
	g_vpp.ge_direct_init = 1;
	vpp_set_mutex(0);
	return 0;
}

/*!*************************************************************************
* vpp_common_ioctl()
* 
* Private Function by Sam Shen, 2009/01/06
*/
/*!
* \brief	vpp driver common ioctl
*		
* \retval  None
*/ 
int vpp_common_ioctl(unsigned int cmd,unsigned long arg)
{
	vpp_mod_base_t *mod_p;
	vpp_fb_base_t *mod_fb_p;
	int retval = 0;

	switch(cmd){
		case VPPIO_VPPGET_INFO:
			{
			int i;
			vpp_cap_t parm;

			parm.chip_id = vpp_get_chipid();
			parm.version = 0x01;
			parm.resx_max = VPP_HD_MAX_RESX;
			parm.resy_max = VPP_HD_MAX_RESY;
			parm.pixel_clk = 400000000;
			parm.module = 0x0;
			for(i=0;i<VPP_MOD_MAX;i++){
				mod_p = vpp_mod_get_base(i);
				if( mod_p ){
					parm.module |= (0x01 << i);
				}
			}
			parm.option = 0x0;
			copy_to_user( (void *)arg, (void *) &parm, sizeof(vpp_cap_t));
			}
			break;
		case VPPIO_VPPSET_INFO:
			{
			vpp_cap_t parm;

			copy_from_user((void *)&parm,(const void *)arg,sizeof(vpp_cap_t));
			}
			break;
		case VPPIO_I2CSET_BYTE:
			{
			vpp_i2c_t parm;
			unsigned int id;

			copy_from_user( (void *) &parm, (const void *)arg, sizeof(vpp_i2c_t));
			id = (parm.addr & 0x0000FF00) >> 8;
			vpp_i2c_bus_write(id,(parm.addr & 0xFF),parm.index,(char *)&parm.val,1);
			}
			break;
		case VPPIO_I2CGET_BYTE:
			{
			vpp_i2c_t parm;
			unsigned int id;
			int len;

			copy_from_user( (void *) &parm, (const void *)arg, sizeof(vpp_i2c_t));
			id = (parm.addr & 0x0000FF00) >> 8;
			len = parm.val;
			{
				unsigned char buf[len];
				
				vpp_i2c_bus_read(id,(parm.addr & 0xFF),parm.index,buf,len);
				parm.val = buf[0];
			}
			copy_to_user( (void *)arg, (void *) &parm, sizeof(vpp_i2c_t));
			}
			break;
		case VPPIO_VPPSET_DIRECTPATH:
			// DPRINT("[VPP] set vpu path %s\n",vpp_vpath_str[arg]);
			g_vpp.vpu_path = (arg == VPP_VPATH_AUTO)? VPP_VPATH_AUTO_DEFAULT:arg;
#if 0	// debug for mermory clear issue
			{
				vout_t *vo;

				vo = vout_get_info(VOUT_HDMI);
				if( vo->status & VPP_VOUT_STS_ACTIVE )
					g_vpp.vpu_path = VPP_VPATH_SCL;
			}
#endif
			{
				vpp_video_path_t vpath;
				
				vpath = (vpp_get_govm_path() & VPP_PATH_GOVM_IN_VPU)? g_vpp.vpu_path:g_vpp.ge_path;
				vpp_pan_display(0,0,(vpath == g_vpp.ge_path));
			}
			break;
		case VPPIO_VPPSET_FBDISP:
			{
				vpp_dispfb_t parm;

				copy_from_user( (void *) &parm, (const void *)arg, sizeof(vpp_dispfb_t));

				if( vpp_check_dbg_level(VPP_DBGLVL_IOCTL) ){
//					DPRINT("[VPP] set fbdisp, flag 0x%x\n",parm.flag);
				}

				switch( g_vpp.vpp_path ){
					case VPP_VPATH_SCL:
						parm.flag |= VPP_FLAG_DISPFB_SCL;
						break;
					case VPP_VPATH_GOVR:
						break;
#ifdef CONFIG_GOVW_SCL_PATH
					case VPP_VPATH_GOVW_SCL:
						if( (retval = vpp_vpu_fb_add(&parm)) ){
							vpp_dbg_show(VPP_DBGLVL_DISPFB,1,"add vpu fb full");
						}
						return retval;
#endif
					default:
						break;
				}

				retval = vpp_disp_fb_add(&parm);
				if( retval ){
					vpp_dbg_show(VPP_DBGLVL_DISPFB,1,"add disp fb full");
				}
				else {
					// vpp_set_dbg_gpio(4,0xFF);
				}
			}
			break;
		case VPPIO_VPPGET_FBDISP:
			{
				vpp_dispfb_info_t parm;

				parm.queue_cnt = g_vpp.disp_fb_max;
				parm.cur_cnt = g_vpp.disp_fb_cnt;
				parm.isr_cnt = g_vpp.disp_fb_isr_cnt;
				parm.disp_cnt = g_vpp.disp_cnt;
				parm.skip_cnt = g_vpp.disp_skip_cnt;
				parm.full_cnt = g_vpp.disp_fb_full_cnt;

				g_vpp.disp_fb_isr_cnt = 0;
				g_vpp.disp_cnt = 0;
				g_vpp.disp_skip_cnt = 0;
				g_vpp.disp_fb_full_cnt = 0;
				copy_to_user( (void *)arg, (void *) &parm, sizeof(vpp_dispfb_info_t));
			}
			break;
		case VPPIO_WAIT_FRAME:
			{
				int i;
				for(i=0;i<arg;i++){
					vpp_wait_vsync();
				}
			}
			break;
		case VPPIO_MODULE_FRAMERATE:
			{
				vpp_mod_arg_t parm;

				copy_from_user( (void *) &parm, (const void *)arg, sizeof(vpp_mod_arg_t));
				if( !(mod_fb_p = vpp_mod_get_fb_base(parm.mod)) )
					break;
				if( parm.read ){
					parm.arg1 = mod_fb_p->framerate;
					copy_to_user( (void *)arg, (void *) &parm, sizeof(vpp_mod_arg_t));
				}
				else {
					mod_fb_p->framerate = parm.arg1;
					if( parm.mod == VPP_MOD_GOVW ){
#ifdef CONFIG_VPP_GOVRH_FBSYNC
							vpp_fbsync_cal_fps();
#endif
							mod_fb_p->set_framebuf(&mod_fb_p->fb);
					}
				}
			}
			break;
		case VPPIO_MODULE_ENABLE:
			{
				vpp_mod_arg_t parm;
				
				copy_from_user( (void *) &parm, (const void *)arg, sizeof(vpp_mod_arg_t));
				if(!(mod_p = vpp_mod_get_base(parm.mod)))
					break;
				if( parm.read ){
					
				}
				else {
					mod_p->set_enable(parm.arg1);
					if( parm.mod == VPP_MOD_CURSOR ){
						vpp_set_vppm_int_enable(VPP_INT_GOVRH_PVBI,parm.arg1);
					}
				}
			}
			break;
		case VPPIO_MODULE_TIMING:
			{
				vpp_mod_timing_t parm;
				vpp_clock_t clock;

				copy_from_user( (void *) &parm, (const void *)arg, sizeof(vpp_mod_timing_t));
				if( !(mod_p = vpp_mod_get_base(parm.mod)) )
					break;
				if( parm.read ){
					mod_p->get_tg(&clock);					
					vpp_trans_timing(parm.mod,&parm.tmr,&clock,0);
					copy_to_user( (void *)arg, (void *) &parm, sizeof(vpp_mod_timing_t));
				}
				else {
					vpp_alloc_framebuffer(parm.tmr.hpixel,parm.tmr.vpixel);
					vpp_mod_set_timing(parm.mod,&parm.tmr);
//					vpp_trans_timing(parm.mod,&parm.tmr,&clock,1);					
//					mod_p->set_tg(&clock);
				}
			}
			break;
		case VPPIO_MODULE_FBADDR:
			{
				vpp_mod_arg_t parm;

				copy_from_user( (void *) &parm, (const void *)arg, sizeof(vpp_mod_arg_t));
				if( !(mod_fb_p = vpp_mod_get_fb_base(parm.mod)) )
					break;
				if( parm.read ){
					mod_fb_p->get_addr(&parm.arg1,&parm.arg2);
					copy_to_user( (void *)arg, (void *) &parm, sizeof(vpp_mod_arg_t));
				}
				else {
					mod_fb_p->set_addr(parm.arg1,parm.arg2);
				}
			}
			break;
		case VPPIO_MODULE_FBINFO:
			{
				vpp_mod_fbinfo_t parm;

				copy_from_user( (void *) &parm, (const void *)arg, sizeof(vpp_mod_fbinfo_t));
				if( !(mod_fb_p = vpp_mod_get_fb_base(parm.mod)) )
					break;
				
				if( parm.read ){
					parm.fb = mod_fb_p->fb;
					copy_to_user( (void *)arg, (void *) &parm, sizeof(vpp_mod_fbinfo_t));
				}
				else {
					mod_fb_p->fb = parm.fb;
					mod_fb_p->set_framebuf(&parm.fb);
				}
			}
			break;
		case VPPIO_MODULE_VIEW:
			{
				vpp_mod_view_t parm;
				copy_from_user( (void *) &parm, (const void *)arg, sizeof(vpp_mod_view_t));
				if( !(mod_fb_p = vpp_mod_get_fb_base(parm.mod)) )
					break;
				if( parm.read ){
					mod_fb_p->fn_view(VPP_FLAG_RD,&parm.view);
					copy_to_user( (void *)arg, (void *) &parm, sizeof(vpp_mod_view_t));
				}
				else {
					mod_fb_p->fn_view(0,&parm.view);
				}
			}
			break;
		case VPPIO_VPPGET_PTS:
			copy_to_user( (void *)arg, (void *) &g_vpp.disp_pts, sizeof(vpp_pts_t));
			break;
		case VPPIO_VPPSET_PTS:
			copy_from_user( (void *) &g_vpp.frame_pts, (const void *)arg, sizeof(vpp_pts_t));
			{
				int i;
				for(i=0;i<sizeof(vpp_pts_t);i++){
					if( g_vpp.frame_pts.pts[i] )
						break;
				}
				if( i == sizeof(vpp_pts_t )){
					memset(&g_vpp.govw_pts,0x0,sizeof(vpp_pts_t));
					memset(&g_vpp.disp_pts,0x0,sizeof(vpp_pts_t));
				}
			}
			break;
#ifdef CONFIG_VPP_STREAM_CAPTURE
		case VPPIO_STREAM_ENABLE:
			g_vpp.stream_enable = arg;
			g_vpp.stream_mb_sync_flag = 0;
			DPRINT("[VPP] VPPIO_STREAM_ENABLE %d\n",g_vpp.stream_enable);
			vpp_pan_display(0,0,(vpp_get_govm_path() & VPP_PATH_GOVM_IN_VPU)?0:1);
			if(!g_vpp.stream_enable){
				vpp_lock();
				vpp_mb_put(0);
				vpp_unlock();
			}
			break;
		case VPPIO_STREAM_GETFB:
			vpp_lock();
			copy_to_user( (void *)arg, (void *) &p_govrh->fb_p->fb,sizeof(vdo_framebuf_t));
			retval = vpp_mb_get(p_govrh->fb_p->fb.y_addr);
			vpp_unlock();
			break;
		case VPPIO_STREAM_PUTFB:
			{
				vdo_framebuf_t fb;
				copy_from_user( (void *) &fb, (const void *)arg, sizeof(vdo_framebuf_t));
				vpp_lock();
				vpp_mb_put(fb.y_addr);
				vpp_unlock();
			}
			break;
#endif
		case VPPIO_MODULE_CSC:
			{
				vpp_mod_arg_t parm;
				copy_from_user( (void *) &parm, (const void *)arg, sizeof(vpp_mod_arg_t));
				if( !(mod_fb_p = vpp_mod_get_fb_base(parm.mod)) )
					break;
				if( parm.read ){
					parm.arg1 = mod_fb_p->csc_mode;
					copy_to_user( (void *)arg, (void *) &parm, sizeof(vpp_mod_arg_t));
				}
				else {
					mod_fb_p->csc_mode = parm.arg1;
					mod_fb_p->set_csc(mod_fb_p->csc_mode);
				}
			}
			break;
		default:
			retval = -ENOTTY;
			break;
	}
	return retval;
} /* End of vpp_common_ioctl */

/*!*************************************************************************
* vout_ioctl()
* 
* Private Function by Sam Shen, 2009/01/06
*/
/*!
* \brief	video out ioctl
*		
* \retval  None
*/ 
int vout_ioctl(unsigned int cmd,unsigned long arg)
{
	int retval = 0;

//	DBGMSG("vout_ioctl\n");

	switch(cmd){
		case VPPIO_VOGET_INFO:
			{
			vpp_vout_info_t parm;
			vout_t *vo;
			int num;

//			DBGMSG("VPPIO_VOGET_INFO\n");

			copy_from_user( (void *) &parm, (const void *)arg, sizeof(vpp_vout_info_t));
			num = parm.num;
			if( num >= VOUT_MODE_MAX ){
				retval = -ENOTTY;
				break;
			}
			memset(&parm,0,sizeof(vpp_vout_info_t));
			vo = vout_get_info(num);
			if( vo ){
				int dac_bk;
				
				parm.num = num;
				parm.status = vo->status;
				strncpy(parm.name,vo->name,10);

				// get current plugin stauts
				dac_bk = vppif_reg32_read(GOVRH_DAC_PWRDN);
				switch(parm.num){
					case VOUT_BOOT:
						break;
					case VOUT_DVO2HDMI:
						if( vo->ops->chkplug(1) )
							parm.status |= VPP_VOUT_STS_PLUGIN;
						else 
							parm.status &= ~VPP_VOUT_STS_PLUGIN;
						break;
					case VOUT_SD_ANALOG:
					case VOUT_VGA:
						if( dac_bk == 0 ){
							break;
						}
					default:
						if( vout_chkplug(num) )
							parm.status |= VPP_VOUT_STS_PLUGIN;
						else 
							parm.status &= ~VPP_VOUT_STS_PLUGIN;
						break;
				}
				vppif_reg32_write(GOVRH_DAC_PWRDN, dac_bk);
			}
			copy_to_user( (void *)arg, (const void *) &parm, sizeof(vpp_vout_info_t));
			
			}
			break;
		case VPPIO_VOSET_MODE:
			{
			vpp_vout_parm_t parm;
			
			copy_from_user( (void *) &parm, (const void *)arg, sizeof(vpp_vout_parm_t));
			retval = vout_set_mode(parm.num, parm.arg);
			}
			break;
		case VPPIO_VOSET_BLANK:
			{
			vpp_vout_parm_t parm;
			
			copy_from_user( (void *) &parm, (const void *)arg, sizeof(vpp_vout_parm_t));
			retval = vout_set_blank(parm.num,parm.arg);
			}
			break;
#ifdef WMT_FTBLK_GOVRH			
		case VPPIO_VOSET_DACSENSE:
			{
			vpp_vout_parm_t parm;
			
			copy_from_user( (void *) &parm, (const void *)arg, sizeof(vpp_vout_parm_t));
#ifdef WMT_FTBLK_GOVRH_VGA
			if( parm.num == VOUT_VGA ){
				/* TODO: dac sense timer */
				if( parm.arg == 0 ){
					govrh_DAC_set_pwrdn(VPP_FLAG_DISABLE);
				}
			}
#endif			
			}
			break;
		case VPPIO_VOSET_BRIGHTNESS:
			{
			vpp_vout_parm_t parm;
			
			copy_from_user( (void *) &parm, (const void *)arg, sizeof(vpp_vout_parm_t));
			govrh_set_brightness(parm.arg);
			}
			break;
		case VPPIO_VOGET_BRIGHTNESS:
			{
			vpp_vout_parm_t parm;
			
			parm.num = 0;
			parm.arg = govrh_get_brightness();
			copy_to_user((void *)arg,(void *)&parm, sizeof(vpp_vout_parm_t));
			}
			break;
		case VPPIO_VOSET_CONTRAST:
			{
			vpp_vout_parm_t parm;
			
			copy_from_user( (void *) &parm, (const void *)arg, sizeof(vpp_vout_parm_t));
			govrh_set_contrast(parm.arg);
			}
			break;
		case VPPIO_VOGET_CONTRAST:
			{
			vpp_vout_parm_t parm;
			
			parm.num = 0;
			parm.arg = govrh_get_contrast();
			copy_to_user((void *)arg,(void *) &parm, sizeof(vpp_vout_parm_t));
			}
			break;
#endif			
		case VPPIO_VOSET_OPTION:
			{
			vpp_vout_option_t option;
			vout_t *vo;
			int num;

			copy_from_user( (void *) &option, (const void *)arg, sizeof(vpp_vout_option_t));
			num = option.num;
			if( num >= VOUT_MODE_MAX ){
				retval = -ENOTTY;
				break;
			}
			vo = vout_get_info(num);
			if( vo ){
				vo->option[0] = option.option[0];
				vo->option[1] = option.option[1];
				vo->option[2] = option.option[2];
				vout_set_mode(num,(vo->status & VPP_VOUT_STS_ACTIVE)? 1:0);
			}
			}
			break;
		case VPPIO_VOGET_OPTION:
			{
			vpp_vout_option_t option;
			vout_t *vo;
			int num;

			copy_from_user( (void *) &option, (const void *)arg, sizeof(vpp_vout_option_t));
			num = option.num;
			if( num >= VOUT_MODE_MAX ){
				retval = -ENOTTY;
				break;
			}
			memset(&option,0,sizeof(vpp_vout_info_t));			
			vo = vout_get_info(num);
			if( vo ){
				option.num = num;
				option.option[0] = vo->option[0];
				option.option[1] = vo->option[1];
				option.option[2] = vo->option[2];
			}
			copy_to_user( (void *)arg, (const void *) &option, sizeof(vpp_vout_option_t));
			}
			break;
		case VPPIO_VOUT_VMODE:
			{
			vpp_vout_vmode_t parm;
			int i;
			vpp_timing_t *vmode;
			unsigned int resx,resy,fps;
			unsigned int pre_resx,pre_resy,pre_fps;
			int index,from_index;
			int support;
			unsigned int option,pre_option;
			vpp_timing_t *timing;
			
			copy_from_user( (void *) &parm, (const void *)arg, sizeof(vpp_vout_vmode_t));
			from_index = parm.num;
			parm.num = 0;
#ifdef CONFIG_VPP_DEMO
			parm.parm[parm.num].resx = 1920;
			parm.parm[parm.num].resy = 1080;
			parm.parm[parm.num].fps = 60;
			parm.parm[parm.num].option = 0;
			parm.num++;
#else
#ifdef CONFIG_WMT_EDID
			{
				vout_t *vo;
				if(!(vo = vout_get_info(parm.mode))){
					goto vout_vmode_end;
				}
				
				if( !(vo->status & VPP_VOUT_STS_PLUGIN) ){
					DPRINT("*W* not plugin\n");
					goto vout_vmode_end;
				}
				
				if( vout_get_edid(parm.mode) == 0 ){
					DPRINT("*W* read EDID fail\n");
					goto vout_vmode_end;
				}
				if( edid_parse(vo->edid) == 0 ){
					DPRINT("*W* parse EDID fail\n");
					goto vout_vmode_end;
				}
			}
#endif
			index = 0;
			resx = resy = fps = option = 0;
			pre_resx = pre_resy = pre_fps = pre_option = 0;
			for(i=0;;i++){
				vmode = (vpp_timing_t *) &vpp_video_mode_table[i];
				if( vmode->pixel_clock == 0 ) 
					break;
				resx = vmode->hpixel;
				resy = vmode->vpixel;
				fps = vpp_get_video_mode_fps(vmode);
				if( vmode->option & VPP_OPT_INTERLACE ){
					resy *= 2;
					i++;
				}
				option = fps & EDID_TMR_FREQ;
				option |= (vmode->option & VPP_OPT_INTERLACE)? EDID_TMR_INTERLACE:0;
				if( (pre_resx == resx) && (pre_resy == resy) && (pre_fps == fps) && (pre_option == option)){
					continue;
				}
				pre_resx = resx;
				pre_resy = resy;
				pre_fps = fps;
				pre_option = option;
				support = 0;
				
#ifdef CONFIG_WMT_EDID
				if( edid_find_support(resx,resy,option,&timing) ){
#else
				if( 1 ){
#endif
					support = 1;
				}

				switch( parm.mode ){
					case VPP_VOUT_HDMI:
					case VPP_VOUT_DVO2HDMI:
						if( g_vpp.hdmi_video_mode ){
							if( resy > g_vpp.hdmi_video_mode ){
								support = 0;
							}
						}
						break;
					default:
						break;
				}

				if( support ){
					if( index >= from_index ){
						parm.parm[parm.num].resx = resx;
						parm.parm[parm.num].resy = resy;
						parm.parm[parm.num].fps = fps;
						parm.parm[parm.num].option = vmode->option;
						parm.num++;
					}
					index++;
					if( parm.num >= VPP_VOUT_VMODE_NUM )
						break;
				}
			}
#ifdef CONFIG_WMT_EDID
vout_vmode_end:
#endif
#endif
			// DPRINT("[VPP] get support vmode %d\n",parm.num);
			copy_to_user( (void *)arg, (const void *) &parm, sizeof(vpp_vout_vmode_t));
			}
			break;
		case VPPIO_VOGET_EDID:
			{
				vpp_vout_edid_t parm;
				char *edid;
				vout_t *vo;
				int size;
				
				copy_from_user( (void *) &parm, (const void *)arg, sizeof(vpp_vout_edid_t));
				size = 0;
#ifdef CONFIG_WMT_EDID
				if(!(vo = vout_get_info(parm.mode))){
					goto vout_edid_end;
				}

				if( !(vo->status & VPP_VOUT_STS_PLUGIN) ){
					DPRINT("*W* not plugin\n");
					goto vout_edid_end;
				}
				
				if( (edid = vout_get_edid(parm.mode)) == 0 ){
					DPRINT("*W* read EDID fail\n");
					goto vout_edid_end;
				}
				size = (edid[0x7E] + 1) * 128;
				if( size > parm.size )
					size = parm.size;
				copy_to_user((void *) parm.buf, (void *) edid, size);
vout_edid_end:	
#endif
				parm.size = size;
				copy_to_user( (void *)arg, (const void *) &parm, sizeof(vpp_vout_edid_t));
			}
			break;
		case VPPIO_VOGET_CP_INFO:
			{
				vpp_vout_cp_info_t parm;
				vout_t *vo;
				int num;

				copy_from_user( (void *) &parm, (const void *)arg, sizeof(vpp_vout_cp_info_t));
				num = parm.num;
				if( num >= VOUT_MODE_MAX ){
					retval = -ENOTTY;
					break;
				}
				memset(&parm,0,sizeof(vpp_vout_cp_info_t));
				vo = vout_get_info(num);
				if( vo ){
					switch( num ){
						case VOUT_DVO2HDMI:
						case VOUT_HDMI:
							parm.bksv[0] = g_vpp.hdmi_bksv[0];
							parm.bksv[1] = g_vpp.hdmi_bksv[1];
							break;
						default:
							parm.bksv[0] = parm.bksv[1] = 0;
							break;
					}
				}
				copy_to_user( (void *)arg, (const void *) &parm, sizeof(vpp_vout_cp_info_t));
			}
			break;
		case VPPIO_VOSET_CP_KEY:
#ifdef WMT_FTBLK_HDMI
			if( g_vpp.hdmi_cp_p == 0 ){
				g_vpp.hdmi_cp_p = (char *)kmalloc(sizeof(vpp_vout_cp_key_t),GFP_KERNEL);
			}
			if( g_vpp.hdmi_cp_p ){
				copy_from_user( (void *) g_vpp.hdmi_cp_p, (const void *)arg, sizeof(vpp_vout_cp_key_t));
				if( hdmi_cp ){
					hdmi_cp->init();
				}
			}
#endif
			break;
#ifdef WMT_FTBLK_HDMI
		case VPPIO_VOSET_AUDIO_PASSTHRU:
			vppif_reg32_write(HDMI_AUD_SUB_PACKET,(arg)?0xF:0x0);
			break;
#endif
		default:
			retval = -ENOTTY;
			break;
	}
	return retval;
} /* End of vout_ioctl */

/*!*************************************************************************
* govr_ioctl()
* 
* Private Function by Sam Shen, 2009/01/06
*/
/*!
* \brief	govr module ioctl
*		
* \retval  None
*/ 
int govr_ioctl(unsigned int cmd,unsigned long arg)
{
	int retval = 0;
	
 	switch(cmd){
#ifdef WMT_FTBLK_GOVRH		
		case VPPIO_GOVRSET_DVO:
			{
			vdo_dvo_parm_t parm;

			copy_from_user( (void *) &parm, (const void *)arg, sizeof(vdo_dvo_parm_t));
			govrh_set_dvo_enable(parm.enable);
			govrh_set_dvo_color_format(parm.color_fmt);
			govrh_set_dvo_clock_delay(parm.clk_inv,parm.clk_delay);
			govrh_set_dvo_outdatw(parm.data_w);
			govrh_set_dvo_sync_polar(parm.sync_polar,parm.vsync_polar);
			p_govrh->fb_p->set_csc(p_govrh->fb_p->csc_mode);
			}
			break;
#endif
#ifdef WMT_FTBLK_GOVRH_CURSOR
		case VPPIO_GOVRSET_CUR_COLKEY:
			{
			vpp_mod_arg_t parm;

			copy_from_user( (void *) &parm, (const void *)arg, sizeof(vdo_dvo_parm_t));
			govrh_CUR_set_color_key(VPP_FLAG_ENABLE,0,parm.arg1);
			}
			break;
		case VPPIO_GOVRSET_CUR_HOTSPOT:
			{
			vpp_mod_arg_t parm;
			vdo_view_t view;

			copy_from_user( (void *) &parm, (const void *)arg, sizeof(vdo_dvo_parm_t));
			p_cursor->hotspot_x = parm.arg1;
			p_cursor->hotspot_y = parm.arg2;
			view.posx = p_cursor->posx;
			view.posy = p_cursor->posy;
			p_cursor->fb_p->fn_view(0,&view);
			}
			break;
#endif
		default:
			retval = -ENOTTY;
			break;
	}
	return retval;
} /* End of govr_ioctl */

/*!*************************************************************************
* govw_ioctl()
* 
* Private Function by Sam Shen, 2009/01/06
*/
/*!
* \brief	govw module ioctl
*		
* \retval  None
*/ 
int govw_ioctl(unsigned int cmd,unsigned long arg)
{
	int retval = 0;
	
	switch(cmd){
		case VPPIO_GOVW_ENABLE:
			vpp_set_govw_tg(arg);
			break;
		default:
			retval = -ENOTTY;
			break;
	}
	return retval;
} /* End of govw_ioctl */

/*!*************************************************************************
* govm_ioctl()
* 
* Private Function by Sam Shen, 2009/01/06
*/
/*!
* \brief	govm module ioctl
*		
* \retval  None
*/ 
int govm_ioctl(unsigned int cmd,unsigned long arg)
{
	int retval = 0;

	switch(cmd){
		case VPPIO_GOVMSET_SRCPATH:
			{
			vpp_src_path_t parm;
			vpp_video_path_t vpath;

			copy_from_user( (void *) &parm, (const void *)arg, sizeof(vpp_src_path_t));
			// fix first line show green line when enable vpu path in full screen
			if((parm.src_path & VPP_PATH_GOVM_IN_VPU) && parm.enable ){
				if(	vppif_reg32_read(GOVM_VPU_Y_STA_CR) == 0 ){
					g_vpp.govw_skip_frame = 2;
				}
			}
#ifdef PATCH_VPU_HALT_GOVW_TG_ERR
			vpp_lock();
			if( vpp_govm_path ){
				extern int vpp_govm_path1;
				if( parm.enable ){
					vpp_govm_path |= parm.src_path;
					vpp_govm_path1 |= parm.src_path; 
				}
				else {
					vpp_govm_path &= ~parm.src_path;
					vpp_govm_path1 &= ~parm.src_path;
				}
			}
			else {
				vpp_set_govm_path(parm.src_path,parm.enable);
			}
			vpp_unlock();
#else
			vpp_set_govm_path(parm.src_path,parm.enable);
#endif
			vpath = (vpp_get_govm_path() & VPP_PATH_GOVM_IN_VPU)? g_vpp.vpu_path:g_vpp.ge_path;
//			DPRINT("[VPP] set src path 0x%x,enable %d,vpp path %s,cur path %s\n",parm.src_path,parm.enable,
//				vpp_vpath_str[vpath],vpp_vpath_str[g_vpp.vpp_path]);

			vpp_pan_display(0,0,(vpath == g_vpp.ge_path));
			}
			break;
		case VPPIO_GOVMGET_SRCPATH:
			retval = vpp_get_govm_path();
			break;
		case VPPIO_GOVMSET_ALPHA:
			{
			vpp_alpha_parm_t parm;

			copy_from_user( (void *) &parm, (const void *)arg, sizeof(vpp_alpha_parm_t));				
			govm_set_alpha_mode(parm.enable,parm.mode,parm.A,parm.B);
			}
			break;
		case VPPIO_GOVMSET_GAMMA:
			govm_set_gamma_mode(arg);
			break;
		case VPPIO_GOVMSET_CLAMPING:
			govm_set_clamping_enable(arg);
			break;		
		default:
			retval = -ENOTTY;
			break;
	}
	return retval;
} /* End of govm_ioctl */

/*!*************************************************************************
* vpu_ioctl()
* 
* Private Function by Sam Shen, 2009/01/06
*/
/*!
* \brief	vpu module ioctl
*		
* \retval  None
*/ 
int vpu_ioctl(unsigned int cmd,unsigned long arg)
{
	int retval = 0;
	
	switch(cmd){
		case VPPIO_VPUSET_VIEW:
			{
			vdo_view_t view;

			copy_from_user( (void *) &view, (const void *)arg, sizeof(vdo_view_t));
			p_vpu->fb_p->fn_view(0,&view);
			if( vpp_check_dbg_level(VPP_DBGLVL_SCALE) ){
				DPRINT("[VPP] set view\n");
			}
			}
			break;
		case VPPIO_VPUGET_VIEW:
			{
			vdo_view_t view;

			p_vpu->fb_p->fn_view(1,&view);
			copy_to_user( (void *)arg, (void *) &view, sizeof(vdo_view_t));
			}
			break;
		case VPPIO_VPUSET_FBDISP:
			{
			vpp_dispfb_t parm;
			
			copy_from_user( (void *) &parm, (const void *)arg, sizeof(vpp_dispfb_t));
			retval = vpp_disp_fb_add(&parm);
			}
			break;
		case VPPIO_VPU_CLR_FBDISP:
			retval = vpp_disp_fb_clr(0,0);
			break;	
		default:
			retval = -ENOTTY;
			break;
	}
	return retval;
} /* End of vpu_ioctl */

/*!*************************************************************************
* scl_ioctl()
* 
* Private Function by Sam Shen, 2009/01/06
*/
/*!
* \brief	scl module ioctl
*		
* \retval  None
*/ 
int scl_ioctl(unsigned int cmd,unsigned long arg)
{
	int retval = 0;

	if( g_vpp.vpp_path == VPP_VPATH_SCL ){
		return -ENOTTY;
	}

	switch(cmd){
		case VPPIO_SCL_SCALE_ASYNC:
		case VPPIO_SCL_SCALE:
			{
			vpp_scale_t parm;

			p_scl->scale_sync = (cmd==VPPIO_SCL_SCALE)? 1:0;
			copy_from_user( (void *) &parm, (const void *)arg, sizeof(vpp_scale_t));
			retval = vpp_set_recursive_scale(&parm.src_fb,&parm.dst_fb);
			copy_to_user( (void *) arg, (void *) &parm, sizeof(vpp_scale_t));
			}
			break;
#ifdef WMT_FTBLK_SCL					
		case VPPIO_SCL_DROP_LINE_ENABLE:
			scl_set_drop_line(arg);
			break;
#endif			
		case VPPIO_SCL_SCALE_FINISH:
			retval = p_scl->scale_finish();
			break;
		default:
			retval = -ENOTTY;
			break;
	}
	return retval;
} /* End of scl_ioctl */

/*!*************************************************************************
* vpp_ioctl()
* 
* Private Function by Sam Shen, 2009/01/06
*/
/*!
* \brief	vpp driver ioctl
*		
* \retval  None
*/ 
int vpp_ioctl(unsigned int cmd,unsigned long arg)
{
	int retval = 0;
	int err = 0;

//	DBGMSG("vpp_ioctl\n");

	switch( _IOC_TYPE(cmd) ){
		case VPPIO_MAGIC:
			break;
		default:
			return -ENOTTY;
	}
	
	/* check argument area */
	if( _IOC_DIR(cmd) & _IOC_READ )
		err = !access_ok( VERIFY_WRITE, (void __user *) arg, _IOC_SIZE(cmd));
	else if ( _IOC_DIR(cmd) & _IOC_WRITE )
		err = !access_ok( VERIFY_READ, (void __user *) arg, _IOC_SIZE(cmd));
	
	if( err ) return -EFAULT;

	if( vpp_check_dbg_level(VPP_DBGLVL_IOCTL) ){
		switch( cmd ){
			case VPPIO_VPPSET_FBDISP:
			case VPPIO_VPPGET_FBDISP:
			case VPPIO_MODULE_VIEW:		// cursor pos
				break;
			default:
				DPRINT("[VPP] ioctl cmd 0x%x,arg 0x%x\n",_IOC_NR(cmd),(int)arg);
				break;
		}
	}

	vpp_set_mutex(1);
	switch(_IOC_NR(cmd)){
		case VPPIO_VPP_BASE ... (VPPIO_VOUT_BASE-1):
//			DBGMSG("VPP command ioctl\n");
			retval = vpp_common_ioctl(cmd,arg);
			break;
		case VPPIO_VOUT_BASE ... (VPPIO_GOVR_BASE-1):
//			DBGMSG("VOUT ioctl\n");
			retval = vout_ioctl(cmd,arg);
			break;
		case VPPIO_GOVR_BASE ... (VPPIO_GOVW_BASE-1):
//			DBGMSG("GOVR ioctl\n");
			retval = govr_ioctl(cmd,arg);
			break;
		case VPPIO_GOVW_BASE ... (VPPIO_GOVM_BASE-1):
//			DBGMSG("GOVW ioctl\n");
			retval = govw_ioctl(cmd,arg);
			break;
		case VPPIO_GOVM_BASE ... (VPPIO_VPU_BASE-1):
//			DBGMSG("GOVM ioctl\n");
			retval = govm_ioctl(cmd,arg);
			break;
		case VPPIO_VPU_BASE ... (VPPIO_SCL_BASE-1):
//			DBGMSG("VPU ioctl\n");
			retval = vpu_ioctl(cmd,arg);
			break;
		case VPPIO_SCL_BASE ... (VPPIO_MAX-1):
//			DBGMSG("SCL ioctl\n");
			retval = scl_ioctl(cmd,arg);
			break;
		default:
			retval = -ENOTTY;
			break;
	}
	vpp_set_mutex(0);

	if( vpp_check_dbg_level(VPP_DBGLVL_IOCTL) ){
		switch( cmd ){
			case VPPIO_VPPSET_FBDISP:
			case VPPIO_VPPGET_FBDISP:
			case VPPIO_MODULE_VIEW:
				break;
			default:
				DPRINT("[VPP] ioctl cmd 0x%x,ret 0x%x\n",_IOC_NR(cmd),(int)retval);
				break;
		}
	}
	return retval;
} /* End of vpp_ioctl */

void vpp_var_to_fb(struct fb_var_screeninfo *var, struct fb_info *info,vdo_framebuf_t *fb)
{
	extern unsigned int fb_egl_swap;
	unsigned int addr;

	if( var ){
		addr = var->yoffset * var->xres_virtual + var->xoffset;
		addr *= var->bits_per_pixel >> 3;
		addr += info->fix.smem_start;
		
		fb->y_addr = addr;
		fb->img_w = var->xres;
		fb->img_h = var->yres;
		fb->fb_w = var->xres_virtual;
		fb->fb_h = var->yres_virtual;
		fb->h_crop = 0;
		fb->v_crop = 0;
		fb->flag = 0;
		fb->c_addr = 0;

		switch (var->bits_per_pixel) {
			case 16:
				if ((info->var.red.length == 5) &&
					(info->var.green.length == 6) &&
					(info->var.blue.length == 5)) {
					fb->col_fmt = VDO_COL_FMT_RGB_565;
				} else if ((info->var.red.length == 5) &&
					(info->var.green.length == 5) &&
					(info->var.blue.length == 5)) {
					fb->col_fmt = VDO_COL_FMT_RGB_1555;
				} else {
					fb->col_fmt = VDO_COL_FMT_RGB_5551;
				}
				break;
			case 32:
				fb->col_fmt = VDO_COL_FMT_ARGB;
				break;
			default:
				fb->col_fmt = VDO_COL_FMT_RGB_565;
				break;
		}
	}
	
	if (fb_egl_swap != 0)
		fb->y_addr = fb_egl_swap;
}

int vpp_pan_display(struct fb_var_screeninfo *var, struct fb_info *info,int enable)
{
#ifdef CONFIG_VPP_GE_DIRECT_PATH
	vpp_video_path_t path;
	static vdo_framebuf_t fb;
	extern unsigned int fb_egl_swap;

    if( g_vpp.ge_direct_init == 0 )
		return 0;

	path = (enable)? g_vpp.ge_path:g_vpp.vpu_path;
#ifdef CONFIG_VPP_STREAM_CAPTURE
	if( g_vpp.stream_enable ){
		path = VPP_VPATH_GOVW;
		enable = 0;
	}
#endif
		
	if( g_vpp.vpp_path != path ){
#ifdef PATCH_VPU_HALT_GOVW_TG_ERR
		if( vpp_govm_path ){
			// DPRINT("[VPP] skip chg GE direct path %d\n",enable);
			return 0;
		}
#endif
		if( enable ){
			vpp_var_to_fb(var,info,&fb);
		}

//		DPRINT("[VPP] pan_display %s\n",vpp_vpath_str[path]);
		vpp_set_path(path,(enable)?&fb:0);
	}
	else {
		vdo_color_fmt govr_colfmt;

		govr_colfmt = p_govrh->fb_p->get_color_fmt();
		vpp_var_to_fb(var,info,&fb);
		if( enable ){
			if( var && (var->xres == p_govrh->fb_p->fb.img_w) && (var->yres == p_govrh->fb_p->fb.img_h) ){
				govrh_set_fb_addr(fb.y_addr, 0);
				if( govr_colfmt != fb.col_fmt ){
					p_govrh->fb_p->set_color_fmt(fb.col_fmt);
				}
			}
		}
//		DPRINT("[VPP] GE 0x%x egl 0x%x\n",addr,fb_egl_swap);
	}
#endif
	return 0;
}

struct timer_list vpp_config_timer;
void vpp_config_tmr(int status)
{
	vout_set_blank(VOUT_MODE_ALL,0);
//	DPRINT("[VPP] %s\n",__FUNCTION__);
}

void vpp_set_vout_enable_timer(void)
{
	init_timer(&vpp_config_timer);
	vpp_config_timer.data = 0;
	vpp_config_timer.function = (void *) vpp_config_tmr;
	vpp_config_timer.expires = jiffies + HZ / 2;
	add_timer(&vpp_config_timer);
}

