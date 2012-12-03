/*++ 
 * linux/drivers/video/wmt/vout-wmt.c
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

#include "vpp.h"

// #define DEBUG
#ifdef DEBUG
#define VPPMSG(fmt, args...) DPRINT("[VO] %s: " fmt, __FUNCTION__ , ## args)
#else
#define VPPMSG(fmt, args...) do {} while(0)
#endif

#define CONFIG_LCD_VGA_DUAL_OUTPUT
//#define CONFIG_HDMI_QUANTUMDATA

#ifndef CFG_LOADER
static int vo_plug_flag;
#endif
vout_mode_t vo_plug_mode;
int (*vo_plug_func)(int hotplug);
extern int vpp_dac_sense_enable;
vout_mode_t dvo_vout_mode;
vout_mode_t int_vout_mode;
int hdmi_vout_mode;
vpp_timing_t vo_oem_tmr;
int hdmi_cur_plugin;
vout_mode_t vo_poll_mode;
int lvds_plug;

// GPIO 7 & 9
swi2c_reg_t vo_gpio_scl = {
	.bit_mask = BIT7,
	.gpio_en = (__GPIO_BASE + 0x40),
	.out_en = (__GPIO_BASE + 0x80),
	.data_in = (__GPIO_BASE + 0x00),
	.data_out = (__GPIO_BASE + 0xC0),
	.pull_en = (__GPIO_BASE + 0x480),
	.pull_en_bit_mask = BIT7,

};

swi2c_reg_t vo_gpio_sda = {
	.bit_mask = BIT9,
	.gpio_en = (__GPIO_BASE + 0x40),
	.out_en = (__GPIO_BASE + 0x80),
	.data_in = (__GPIO_BASE + 0x00),
	.data_out = (__GPIO_BASE + 0xC0),
	.pull_en = (__GPIO_BASE + 0x480),
	.pull_en_bit_mask = BIT9,
};

// I2C0 SCL & SDA
swi2c_reg_t vo_i2c0_scl = {
	.bit_mask = BIT8,
	.gpio_en = (__GPIO_BASE + 0x54),
	.out_en = (__GPIO_BASE + 0x94),
	.data_in = (__GPIO_BASE + 0x14),
	.data_out = (__GPIO_BASE + 0xD4),
	.pull_en = (__GPIO_BASE + 0x494),
	.pull_en_bit_mask = BIT8,
};

swi2c_reg_t vo_i2c0_sda = {
	.bit_mask = BIT9,
	.gpio_en = (__GPIO_BASE + 0x54),
	.out_en = (__GPIO_BASE + 0x94),
	.data_in = (__GPIO_BASE + 0x14),
	.data_out = (__GPIO_BASE + 0xD4),
	.pull_en = (__GPIO_BASE + 0x494),
	.pull_en_bit_mask =  BIT9,
};

swi2c_handle_t vo_swi2c_vga = { 
	.scl_reg = &vo_i2c0_scl,
	.sda_reg = &vo_i2c0_sda,
};

swi2c_handle_t vo_swi2c_dvi = { 
	.scl_reg = &vo_gpio_scl,
	.sda_reg = &vo_gpio_sda,
};

#ifndef CFG_LOADER
extern int wmt_getsyspara(char *varname, unsigned char *varval, int *varlen);
extern void hdmi_config_audio(vout_audio_t *info);
#endif

/*--------------------------------------- API ---------------------------------------*/
int vo_i2c_proc(int id,unsigned int addr,unsigned int index,char *pdata,int len)
{
	swi2c_handle_t *handle = 0;
	int ret = 0;

	switch(id){
		case 0:	// vga
			handle = &vo_swi2c_vga;
			break;
		case 1:	// dvi
			handle = &vo_swi2c_dvi;
			break;
		default:
			break;
	}

	if( handle ){
		if( wmt_swi2c_check(handle) ){
			return -1;
		}
		
		if( addr & 0x1 ){	// read
			*pdata = 0xff;
#ifdef CONFIG_WMT_EDID			
			ret = wmt_swi2c_read(handle,addr & ~0x1,index,pdata,len);
#else
			ret = -1;
#endif
#if 0
			DPRINT("[VO] read id %d,addr 0x%x,index 0x%x,len %d,data 0x%x,ret 0x%x\n",id,addr,index,len,*pdata,ret);
			{
				int i;
				for(i=0;i<len;i++){
					if( (i % 16) == 0 ){
						DPRINT("\n[VO] 0x%02x : ",i);
					}
					DPRINT("%02x ",pdata[i]);
				}
				DPRINT("\n");
			}
#endif
		}
		else {	// write
			DPRINT("*W* not support sw i2c write\n");
		}
	}
	return ret;
}

#ifndef CFG_LOADER
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
static void vo_do_plug(struct work_struct *ptr)
#else
static void vo_do_plug
(
	void *ptr		/*!<; // work input data */
)
#endif
{
	vout_t *vo;
	int plugin;

	if( vo_plug_func == 0 )
		return;

 	govrh_set_dvo_enable(1);
	plugin = vo_plug_func(1);
	govrh_set_dvo_enable(plugin);
	vo = vout_get_info(vo_plug_mode);
	vout_change_status(vo,VOCTL_CHKPLUG,0,plugin);
	vo_plug_flag = 0;
	VPPMSG(KERN_DEBUG "vo_do_plug %d\n",plugin);
	vppif_reg8_out(GPIO_BASE_ADDR+0x300+VPP_VOINT_NO,0x80);	// GPIO irq enable
	vpp_netlink_notify(USER_PID,DEVICE_PLUG_IN,plugin);
	return;
}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
DECLARE_DELAYED_WORK(vo_plug_work, vo_do_plug);
#else
DECLARE_WORK(vo_plug_work,vo_do_plug,0);
#endif

static irqreturn_t vo_plug_interrupt_routine
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
	VPPMSG(KERN_DEBUG "vo_plug_interrupt_routine\n");
	if( (vppif_reg8_in(GPIO_BASE_ADDR+0x360) &  (0x1<<VPP_VOINT_NO)) == 0 )
		return IRQ_NONE;

	vppif_reg8_out(GPIO_BASE_ADDR+0x360, 0x1<<VPP_VOINT_NO);	// clear int status
#ifdef __KERNEL__
//	if( vo_plug_flag == 0 ){
		vppif_reg8_out(GPIO_BASE_ADDR+0x300+VPP_VOINT_NO,0x00);	// GPIO irq disable
		schedule_delayed_work(&vo_plug_work, HZ/5);
		vo_plug_flag = 1;
//	}
#else
	if( vo_plug_func ) vo_do_plug(0);
#endif
	return IRQ_HANDLED;	
}

#define CONFIG_VO_POLL_WORKQUEUE
struct timer_list vo_poll_timer;
#ifdef CONFIG_VO_POLL_WORKQUEUE
static void vo_do_poll(struct work_struct *ptr)
{
	vout_t *vo;

	vo = vout_get_info(VOUT_DVO2HDMI);
	if( vo->dev_ops ){
		vo->dev_ops->poll();
	}
	mod_timer(&vo_poll_timer,jiffies + msecs_to_jiffies(vo_poll_timer.data));
	return;
}

DECLARE_DELAYED_WORK(vo_poll_work, vo_do_poll);
#else
struct tasklet_struct vo_poll_tasklet;
static void vo_do_poll_tasklet
(
	unsigned long data		/*!<; // tasklet input data */
)
{
	vout_t *vo;

	vpp_lock();
	vo = vout_get_info(VOUT_DVO2HDMI);
	if( vo->dev_ops ){
		vo->dev_ops->poll();
	}
	mod_timer(&vo_poll_timer,jiffies + msecs_to_jiffies(vo_poll_timer.data));
	vpp_unlock();
}
#endif

void vo_do_poll_tmr(int ms)
{
#ifdef CONFIG_VO_POLL_WORKQUEUE
	schedule_delayed_work(&vo_poll_work,msecs_to_jiffies(ms));
#else
	tasklet_schedule(&vo_poll_tasklet);
#endif
}

static void vo_set_poll(vpp_vout_t mode,int on,int ms)
{
	if( on ){
		vo_poll_mode = mode;
		if( vo_poll_timer.function ){
			vo_poll_timer.data = ms/2;
			mod_timer(&vo_poll_timer,jiffies + msecs_to_jiffies(vo_poll_timer.data));
		}
		else {
			init_timer(&vo_poll_timer);
			vo_poll_timer.data = ms/2;
			vo_poll_timer.function = (void *) vo_do_poll_tmr;
			vo_poll_timer.expires = jiffies + msecs_to_jiffies(vo_poll_timer.data);
			add_timer(&vo_poll_timer);
		}
#ifndef CONFIG_VO_POLL_WORKQUEUE
		tasklet_init(&vo_poll_tasklet,vo_do_poll_tasklet,0);
#endif
	}
	else {
		del_timer(&vo_poll_timer);
#ifndef CONFIG_VO_POLL_WORKQUEUE
		tasklet_kill(&vo_poll_tasklet);
#endif
		vo_poll_mode = VPP_VOUT_MAX;
	}
}
#endif //fan

void vout_set_int_type(int type)
{
	unsigned char reg;

	reg = vppif_reg8_in(GPIO_BASE_ADDR+0x300+VPP_VOINT_NO);
	reg &= ~0x3;
	switch( type ){
		case 0:	// low level
		case 1:	// high level
		case 2:	// falling edge
		case 3:	// rising edge
			reg |= type;
			break;
		default:
			break;
	}
	vppif_reg8_out(GPIO_BASE_ADDR+0x300+VPP_VOINT_NO,reg);
}

static void vo_plug_enable(int enable,void *func,vout_mode_t mode)
{
	VPPMSG(KERN_DEBUG "vo_plug_enable(%d)\n",enable);
	vo_plug_mode = mode;
#ifdef CONFIG_WMT_EXT_DEV_PLUG_DISABLE
	vo_plug_func = 0;
 	govrh_set_dvo_enable(enable);
#else
	vo_plug_func = func;
	if( vo_plug_func == 0 )
		return;

	if( enable ){
		vppif_reg32_write(GPIO_BASE_ADDR+0x40,0x1<<VPP_VOINT_NO,VPP_VOINT_NO,0x0);	// GPIO disable
		vppif_reg32_write(GPIO_BASE_ADDR+0x80,0x1<<VPP_VOINT_NO,VPP_VOINT_NO,0x0);	// GPIO input mode
		vppif_reg32_write(GPIO_BASE_ADDR+0x480,0x1<<VPP_VOINT_NO,VPP_VOINT_NO,0x1);	// GPIO pull enable
		vppif_reg32_write(GPIO_BASE_ADDR+0x4c0,0x1<<VPP_VOINT_NO,VPP_VOINT_NO,0x1);	// GPIO pull-up
#ifndef CFG_LOADER
		vo_do_plug(0);
		if ( vpp_request_irq(IRQ_GPIO, vo_plug_interrupt_routine, IRQF_SHARED, "vo plug", (void *) &vo_plug_mode) ) {
			VPPMSG("*E* request GPIO ISR fail\n");
		}
		vppif_reg8_out(GPIO_BASE_ADDR+0x300+VPP_VOINT_NO,0x80);					// GPIO irq enable
	}
	else {
		vpp_free_irq(IRQ_GPIO,(void *) &vo_plug_mode);
#endif //fan
	}
#endif	
}

/*--------------------------------------- VGA ---------------------------------------*/
#ifdef WMT_FTBLK_VOUT_VGA
static int vo_vga_compatible(int arg)
{
	vout_mode_t mode;

	VPPMSG(KERN_DEBUG "vo_vga_compatible(%d)\n",arg);
	mode = (vout_mode_t) arg;
	switch(mode){
		case VOUT_BOOT:
		case VOUT_SD_DIGITAL:
		case VOUT_SD_ANALOG:
#ifndef CONFIG_LCD_VGA_DUAL_OUTPUT
		case VOUT_LCD:
#endif
			return 1;
		default:
			break;
	}
	return 0;
}

static int vo_vga_visible(int arg)
{
	VPPMSG(KERN_DEBUG "vo_vga_visible(%d)\n",arg);
	if( arg ){
		govrh_set_vga_enable(VPP_FLAG_ENABLE);
	}
	else {
		govrh_set_vga_enable(VPP_FLAG_DISABLE);
	}
	return 0;
}

static int vo_vga_config(int arg)
{
	int conf;
	vout_t *vo;
	vout_info_t *vo_info;

	vo_info = (vout_info_t *) arg;
	conf = 1;
	if( vppif_reg32_read(GOVRH_DVO_ENABLE) ){
		vo = vout_get_info(VOUT_DVI);
		if( vo && (vo->status & VPP_VOUT_STS_ACTIVE ) ){
			conf = 0;
		}

		vo = vout_get_info(VOUT_DVO2HDMI);
		if( vo && (vo->status & VPP_VOUT_STS_ACTIVE ) ){
			conf = 0;
		}

		vo = vout_get_info(VOUT_LCD);
		if( vo && (vo->status & VPP_VOUT_STS_ACTIVE ) ){
			vpp_clock_t tmr;

			govrh_get_tg(&tmr);
			govrh_set_VGA_sync(tmr.hsync,tmr.vsync * tmr.total_pixel_of_line);
			govrh_set_VGA_sync_polar(1,1);
			conf = 0;
		}
	}

	vo = vout_get_info(VOUT_SD_DIGITAL);
	if( vo && (vo->status & VPP_VOUT_STS_ACTIVE ) ){
		conf = 0;
	}

	vo = vout_get_info(VOUT_SD_ANALOG);
	if( vo && (vo->status & VPP_VOUT_STS_ACTIVE ) ){
		conf = 0;
	}

	vo = vout_get_info(VOUT_HDMI);
	if( vo && (vo->status & VPP_VOUT_STS_ACTIVE ) ){
		conf = 0;
	}

	VPPMSG(KERN_DEBUG "vo_vga_config %d\n",conf);

	if( conf ){	
		vppif_reg32_write(GOVRH_ACTPX_BG,vppif_reg32_read(GOVRH_ACTPX_BG)-1);
		vppif_reg32_write(GOVRH_ACTPX_END,vppif_reg32_read(GOVRH_ACTPX_END)-1);
	}
#ifdef CFG_LOADER
	govrh_set_vga_enable(1);
	govrh_DAC_set_pwrdn(0);
#endif
	return 0;
}

static int vo_vga_init(int arg)
{
	VPPMSG(KERN_DEBUG "vo_vga_init(%d)\n",arg);

	govrh_set_vga_enable(VPP_FLAG_ENABLE);
#ifdef CONFIG_WMT_INT_DEV_PLUG_DISABLE
	vpp_dac_sense_enable = 0;
	govrh_DAC_set_pwrdn(VPP_FLAG_DISABLE);
#else
	vpp_set_vppm_int_enable(VPP_INT_GOVRH_VBIS,VPP_FLAG_ENABLE);
#endif
	govrh_DAC_set_sense_value(0,p_govrh->vga_dac_sense_val,p_govrh->vga_dac_sense_val);
	govrh_DAC_set_sense_value(1,p_govrh->vga_dac_sense_val,p_govrh->vga_dac_sense_val);
	govrh_DAC_set_sense_value(2,p_govrh->vga_dac_sense_val,p_govrh->vga_dac_sense_val);
	g_vpp.vga_enable = 1;
	return 0;
}

static int vo_vga_uninit(int arg)
{
	VPPMSG(KERN_DEBUG "vo_vga_uninit(%d)\n",arg);
	g_vpp.vga_enable = 0;
	vpp_set_vppm_int_enable(VPP_INT_GOVRH_VBIS,VPP_FLAG_DISABLE);
	govrh_set_vga_enable(VPP_FLAG_DISABLE);
	govrh_DAC_set_pwrdn(VPP_FLAG_ENABLE);
	return 0;
}

static int vo_vga_suspend(int arg)
{
	VPPMSG(KERN_DEBUG "vo_vga_suspend(%d)\n",arg);
	govrh_set_vga_enable(VPP_FLAG_DISABLE);
	return 0;
}

static int vo_vga_resume(int arg)
{
	VPPMSG(KERN_DEBUG "vo_vga_resume(%d)\n",arg);
	govrh_set_vga_enable(VPP_FLAG_ENABLE);
	return 0;
}

static int vo_vga_chkplug(int arg)
{
	int plugin,sense;
	vout_t *vo;

	vo = vout_get_info(VOUT_VGA);
	govrh_DAC_set_sense_value(0,p_govrh->vga_dac_sense_val,p_govrh->vga_dac_sense_val);
	govrh_DAC_set_sense_value(1,p_govrh->vga_dac_sense_val,p_govrh->vga_dac_sense_val);
	govrh_DAC_set_sense_value(2,p_govrh->vga_dac_sense_val,p_govrh->vga_dac_sense_val);
	govrh_DAC_set_pwrdn(0);
#ifdef __KERNEL__
	vpp_irqproc_work(VPP_INT_GOVRH_PVBI,vpp_irqproc_monitor_dac_sense,&sense,1);
#else
	sense = govrh_DAC_monitor_sense();
#endif
	plugin = ((sense & 0x7) == 0x7)? 1:0;
	govrh_DAC_set_pwrdn((plugin)? 0:1);
	vout_change_status(vo,VOCTL_CHKPLUG,0,plugin);	
	VPPMSG("vo_vga_chkplug %d,0x%x\n",plugin,sense);
	return plugin;
}

#ifdef __KERNEL__
int vo_vga_plugin;
static void vo_vga_do_notify(struct work_struct *ptr)
{
	vpp_netlink_notify(USER_PID,DEVICE_PLUG_IN,vo_vga_plugin);
}
DECLARE_DELAYED_WORK(vo_vga_notify_work, vo_vga_do_notify);
#endif

int vo_vga_plug_detect(void)
{
	static unsigned int sense_cnt = 0;
	static unsigned int pre_plugin;	
	unsigned int plugin;

	if( g_vpp.vga_enable == 0 ){
		return -1;
	}

	sense_cnt++;
	if( (sense_cnt % p_govrh->vga_dac_sense_cnt) != 0 ){
		return -1;
	}

	if( (vppif_reg32_in(REG_VPP_INTSTS) & BIT10) || (g_vpp.vga_intsts & BIT10) ){
		sense_cnt--;
//		DPRINT("*W* vga detect over vbie %x %x\n",vppif_reg32_in(REG_VPP_INTSTS),g_vpp.vga_intsts);
		return -1;
	}

	if( vppif_reg32_read(GOVRH_DAC_PWRDN) == 1 ){
		vppif_reg32_write(GOVRH_DAC_PWRDN, 0);	/* DAC power on */
		sense_cnt--;
		return -1;
	}

	plugin = (govrh_DAC_monitor_sense() & BIT2)? 1:0;
	govrh_DAC_set_pwrdn((plugin)? 0:1);	
	if( plugin != pre_plugin ){
		govrh_set_vga_enable(plugin);
		pre_plugin = plugin;
		DPRINT("[VOUT] VGA PLUG %s\n",(plugin)?"IN":"OUT");
#ifdef __KERNEL__
		vo_vga_plugin = plugin;
		schedule_delayed_work(&vo_vga_notify_work, HZ/10);
#endif
	}
	return plugin;
}

static int vo_vga_get_edid(int arg)
{
	char *buf;
	int ret = 1;

	buf = (char *) arg;
#ifdef CONFIG_WMT_EDID
	if( (ret = vo_i2c_proc(0,0xA0 | BIT(0),0,&buf[0],128)) == 0 ){
		if( buf[0x7E] ){
			ret = vo_i2c_proc(0,0xA0 | BIT(0),128,&buf[128],128);
		}
	}
#endif
	return ret;
}

vout_ops_t vo_vga_ops = 
{
	.init = vo_vga_init,
	.uninit = vo_vga_uninit,
	.compatible = vo_vga_compatible,
	.visible = vo_vga_visible,
	.config = vo_vga_config,
	.suspend = vo_vga_suspend,
	.resume = vo_vga_resume,
	.chkplug = vo_vga_chkplug,
	.get_edid = vo_vga_get_edid,
};

vout_t vo_vga_parm = {
	.ops = &vo_vga_ops,
	.name = "VGA",
	.option[0] = 0,
	.option[1] = 0,
	.option[2] = 0,
	.resx = VPP_HD_DISP_RESX,
	.resy = VPP_HD_DISP_RESY,
	.pixclk = (VPP_HD_DISP_RESX*VPP_HD_DISP_RESY*VPP_HD_DISP_FPS),
};
#endif /* WMT_FTBLK_VOUT_VGA */

/*--------------------------------------- DVI ---------------------------------------*/
#ifdef WMT_FTBLK_VOUT_DVI
static int vo_dvi_compatible(int arg)
{
	vout_mode_t mode;

	VPPMSG(KERN_DEBUG "vo_dvi_compatible(%d)\n",arg);
	mode = (vout_mode_t) arg;
	switch(mode){
		case VOUT_BOOT:
		case VOUT_SD_DIGITAL:
//		case VOUT_SD_ANALOG:
		case VOUT_DVO2HDMI:
		case VOUT_LCD:
			return 1;
		default:
			break;
	}
	return 0;
}

static int vo_dvi_visible(int arg)
{
	VPPMSG(KERN_DEBUG "vo_dvi_visible(%d)\n",arg);
	govrh_set_dvo_enable(arg);
	return 0;
}

static int vo_dvi_config(int arg)
{
	vout_info_t *vo_info;
	vout_t *vo;	

	VPPMSG(KERN_DEBUG "vo_dvi_config\n");

	vo_info = (vout_info_t *) arg;
	vo = vout_get_info(VOUT_DVI);	
	if( vo->dev_ops ){
		vo->dev_ops->config(vo_info);
	}
	return 0;
}

static int vo_dvi_init(int arg)
{
	vout_t *vo;

	VPPMSG(KERN_DEBUG "vo_dvi_init(%d)\n",arg);

	vo = vout_get_info(VOUT_DVI);
	vo->vo_option = vo->option[1];
	if( vo->dev_ops ){
		vo->dev_ops->set_mode(&vo->option[0]);
		vo->dev_ops->set_power_down(VPP_FLAG_DISABLE);
		vo_plug_enable(VPP_FLAG_ENABLE,vo->dev_ops->check_plugin,VOUT_DVI);			
	}
	govrh_set_dvo_color_format(vo->option[0]);
	govrh_set_dvo_outdatw(vo->option[1]);
	p_govrh->fb_p->set_csc(p_govrh->fb_p->csc_mode);
	govrh_set_dvo_enable(VPP_FLAG_ENABLE);
	return 0;
}

static int vo_dvi_uninit(int arg)
{
	vout_t *vo;
	
	VPPMSG(KERN_DEBUG "vo_dvi_uninit(%d)\n",arg);

	vo_plug_enable(VPP_FLAG_DISABLE,0,VOUT_MODE_MAX);
	vo = vout_get_info(VOUT_DVI);
	if( vo->dev_ops ){
		vo->dev_ops->set_power_down(VPP_FLAG_ENABLE);
	}
	govrh_set_dvo_enable(VPP_FLAG_DISABLE);
	return 0;
}

static int vo_dvi_suspend(int arg)
{
	vout_t *vo;

	VPPMSG(KERN_DEBUG "vo_dvi_suspend(%d)\n",arg);
	vo_plug_enable(VPP_FLAG_DISABLE,vo_plug_func,VOUT_MODE_MAX);
	vo = vout_get_info(VOUT_DVI);
	if( vo->dev_ops ){
		vo->dev_ops->set_power_down(VPP_FLAG_ENABLE);
	}
	govrh_set_dvo_enable(VPP_FLAG_DISABLE);
	return 0;
}

static int vo_dvi_resume(int arg)
{
	vout_t *vo;

	VPPMSG(KERN_DEBUG "vo_dvi_resume(%d)\n",arg);
	vo = vout_get_info(VOUT_DVI);
	if( vo->dev_ops ){
		vo->dev_ops->init();
	}
	govrh_set_dvo_enable(VPP_FLAG_ENABLE);
	vo_plug_enable(VPP_FLAG_ENABLE,vo_plug_func,VOUT_DVI);
	return 0;
}

static int vo_dvi_chkplug(int arg)
{
	vout_t *vo;
	int plugin = 0;

	vo = vout_get_info(VOUT_DVI);
	if( vo->dev_ops ){
		plugin = vo->dev_ops->check_plugin(0);
	}
	VPPMSG(KERN_DEBUG "vo_dvi_chkplug %d\n",plugin);
	return plugin;
}

static int vo_dvi_get_edid(int arg)
{
	char *buf;

	buf = (char *) arg;
#ifdef CONFIG_WMT_EDID
	if( vo_i2c_proc(1,0xA0 | BIT(0),0,&buf[0],128) == 0 ){
		if( buf[0x7E] ){
			vo_i2c_proc(1,0xA0 | BIT(0),128,&buf[128],128);
		}
	}
#endif
	return 0;
}

vout_ops_t vo_dvi_ops = 
{
	.init = vo_dvi_init,
	.uninit = vo_dvi_uninit,
	.compatible = vo_dvi_compatible,
	.visible = vo_dvi_visible,
	.config = vo_dvi_config,
	.suspend = vo_dvi_suspend,
	.resume = vo_dvi_resume,
	.chkplug = vo_dvi_chkplug,
	.get_edid = vo_dvi_get_edid,
};

vout_t vo_dvi_parm = {
	.ops = &vo_dvi_ops,
	.name = "DVI",
	.option[0] = VDO_COL_FMT_ARGB,
	.option[1] = VPP_DATAWIDHT_24,
	.option[2] = 0,
	.resx = VPP_HD_DISP_RESX,
	.resy = VPP_HD_DISP_RESY,
	.pixclk = (VPP_HD_DISP_RESX*VPP_HD_DISP_RESY*VPP_HD_DISP_FPS),
};
#endif /* WMT_FTBLK_VOUT_DVI */

/*--------------------------------------- DVO2HDMI ---------------------------------------*/
#ifdef WMT_FTBLK_VOUT_DVO2HDMI
static int vo_dvo2hdmi_init(int arg)
{
	vout_t *vo;
	vdo_color_fmt colfmt;

	VPPMSG(KERN_DEBUG "vo_dvo2hdmi_init(%d)\n",arg);

	vo = vout_get_info(VOUT_DVO2HDMI);
	vo->vo_option = vo->option[1];
#if 0
	// EDID not support, default RGB mode
	if( edid_info.option & EDID_OPT_VALID ){
		switch( vo->option[0] ){
			case VDO_COL_FMT_YUV422H:
			case VDO_COL_FMT_YUV422V:
				if((edid_info.option & EDID_OPT_YUV422)==0)
					vo->option[0] = VDO_COL_FMT_ARGB;
				break;
			case VDO_COL_FMT_YUV444:
				if((edid_info.option & EDID_OPT_YUV444)==0)
					vo->option[0] = VDO_COL_FMT_ARGB;
				break;
			default:
				break;
		}
	}
#endif
	if( vo->dev_ops ){
		vo->dev_ops->set_mode(&vo->option[0]);
		vo_plug_enable(VPP_FLAG_ENABLE,vo->dev_ops->interrupt,VOUT_DVO2HDMI);
		vout_change_status(vo,VOCTL_CHKPLUG,0,vo->dev_ops->check_plugin(0));
	}
	colfmt = (vo->option[0]==VDO_COL_FMT_YUV422V)? VDO_COL_FMT_YUV422H:vo->option[0];
	govrh_set_dvo_color_format(colfmt);
	govrh_set_dvo_outdatw(vo->option[1] & BIT0);
	p_govrh->fb_p->set_csc(p_govrh->fb_p->csc_mode);
#ifdef __KERNEL__
	vo_set_poll(VOUT_DVO2HDMI,(vo->dev_ops->poll)?1:0,100);
#endif
	return 0;
}

static int vo_dvo2hdmi_uninit(int arg)
{
	vout_t *vo;

	VPPMSG(KERN_DEBUG "vo_dvo2hdmi_uninit(%d)\n",arg);
	vo = vout_get_info(VOUT_DVO2HDMI);
	vo_plug_enable(VPP_FLAG_DISABLE,0,VOUT_MODE_MAX);
#ifdef __KERNEL__
	vo_set_poll(VOUT_DVO2HDMI,0,100);
#endif
	if( vo->dev_ops ){
		vo->dev_ops->set_power_down(1);
	}	
	return 0;
}

static int vo_dvo2hdmi_suspend(int arg)
{
	vout_t *vo;
	
	VPPMSG(KERN_DEBUG "vo_dvo2hdmi_suspend(%d)\n",arg);
	vo = vout_get_info(VOUT_DVO2HDMI);	
	vo_plug_enable(VPP_FLAG_DISABLE,vo_plug_func,VOUT_MODE_MAX);
	if( vo->dev_ops ){
		vo->dev_ops->set_power_down(1);
	}
	govrh_set_dvo_enable(VPP_FLAG_DISABLE);
	return 0;
}

static int vo_dvo2hdmi_resume(int arg)
{
	vout_t *vo;
	
	VPPMSG(KERN_DEBUG "vo_dvo2hdmi_resume(%d)\n",arg);
	vo = vout_get_info(VOUT_DVO2HDMI);
	if( vo->dev_ops ){
		vo->dev_ops->init();
	}
	govrh_set_dvo_enable(VPP_FLAG_ENABLE);
	vo_plug_enable(VPP_FLAG_ENABLE,vo_plug_func,VOUT_DVO2HDMI);
	return 0;
}

static int vo_dvo2hdmi_compatible(int arg)
{
	vout_mode_t mode;

	VPPMSG(KERN_DEBUG "vo_dvo2hdmi_compatible(%d)\n",arg);
	mode = (vout_mode_t) arg;
	switch(mode){
		case VOUT_BOOT:
//		case VOUT_VGA:
		case VOUT_DVI:
		case VOUT_SD_DIGITAL:
		case VOUT_SD_ANALOG:
		case VOUT_LCD:
			return 1;
		default:
			break;
	}
	return 0;
}

static int vo_dvo2hdmi_visible(int arg)
{
	vout_t *vo;
	
	VPPMSG(KERN_DEBUG "vo_dvo2hdmi_visible(%d)\n",arg);

	vo = vout_get_info(VOUT_DVO2HDMI);
	if( vo->dev_ops ){
		vppif_reg32_write(GPIO_BASE_ADDR+0x44,0xFFFFFF,0,(arg)?0:0xFFFFFF);	// Enable GPIO
//		vo->dev_ops->set_power_down((arg)?0:1);
	}
	else {
		govrh_set_dvo_enable(arg);
	}
	return 0;
}

static int vo_dvo2hdmi_config(int arg)
{
	vout_info_t *vo_info;
	unsigned int reg;
	unsigned int sense;
	vout_t *vo;

	VPPMSG(KERN_DEBUG "vo_dvo2hdmi_config\n");

	vo = vout_get_info(VOUT_DVO2HDMI);
	vo_info = (vout_info_t *) arg;
	sense = vpp_dac_sense_enable;
	vpp_dac_sense_enable = 0;
#ifdef WMT_FTBLK_GOVRH_DAC		
	reg = vppif_reg32_read(GOVRH_DAC_PWRDN);
	vppif_reg32_write(GOVRH_DAC_PWRDN,0xFF);
#endif	
	govrh_set_tg_enable(VPP_FLAG_ENABLE);
	if( vo->dev_ops ){
		vo->dev_ops->config(vo_info);
	}
	govrh_set_tg_enable(VPP_FLAG_DISABLE);
#ifdef WMT_FTBLK_GOVRH_DAC		
	vppif_reg32_write(GOVRH_DAC_PWRDN,reg);
#endif	
	vpp_dac_sense_enable = sense;
	return 0;
}

static int vo_dvo2hdmi_chkplug(int arg)
{
	vout_t *vo;
	int plugin = 0;

	if( arg == 0 )
		mdelay(200); // wait plugin status

	vo = vout_get_info(VOUT_DVO2HDMI);
	if( vo->dev_ops ){
		plugin = vo->dev_ops->check_plugin(0);
	}
	VPPMSG(KERN_DEBUG "vo_dvo2hdmi_chkplug %d\n",plugin);
	return plugin;
}

static int vo_dvo2hdmi_get_edid(int arg)
{
	vout_t *vo;

	VPPMSG(KERN_DEBUG "vo_dvo2hdmi_get_edid\n");
	vo = vout_get_info(VOUT_DVO2HDMI);
#ifdef CONFIG_WMT_EDID
	if( vo->dev_ops ){
		vo->dev_ops->get_edid((char *)arg);
	}
#endif
	return 0;
}

vout_ops_t vo_dvo2hdmi_ops = 
{
	.init = vo_dvo2hdmi_init,
	.uninit = vo_dvo2hdmi_uninit,
	.compatible = vo_dvo2hdmi_compatible,
	.visible = vo_dvo2hdmi_visible,
	.config = vo_dvo2hdmi_config,
	.suspend = vo_dvo2hdmi_suspend,
	.resume = vo_dvo2hdmi_resume,
	.chkplug = vo_dvo2hdmi_chkplug,
	.get_edid = vo_dvo2hdmi_get_edid,
};

vout_t vo_dvo2hdmi_parm = {
	.ops = &vo_dvo2hdmi_ops,
	.name = "DVO2HDMI",
	.option[0] = VDO_COL_FMT_ARGB,
	.option[1] = VPP_DATAWIDHT_24,
	.option[2] = 0,
#ifdef CONFIG_HDMI_QUANTUMDATA
	.resx = 720,
	.resy = 480,
	.pixclk = 27027060,
#else
	.resx = 1280,
	.resy = 720,
	.pixclk = 74250060,
#endif
};
#endif /* WMT_FTBLK_VOUT_DVO2HDMI */

/*--------------------------------------- LCD ---------------------------------------*/
#ifdef WMT_FTBLK_VOUT_LCD
static int vo_lcd_compatible(int arg)
{
	vout_mode_t mode;

	VPPMSG(KERN_DEBUG "vo_lcd_compatible(%d)\n",arg);
	mode = (vout_mode_t) arg;
	switch(mode){
		case VOUT_BOOT:
		case VOUT_DVO2HDMI:
		case VOUT_DVI:
		case VOUT_SD_DIGITAL:
		case VOUT_SD_ANALOG:
#ifndef CONFIG_LCD_VGA_DUAL_OUTPUT
		case VOUT_VGA:
#endif
			return 1;
		default:
			break;
	}
	return 0;
}

static int vo_lcd_visible(int arg)
{
	vout_t *vo;

	VPPMSG(KERN_DEBUG "vo_lcd_visible(%d)\n",arg);
	vo = vout_get_info(VOUT_LCD);	
	govrh_set_dvo_enable(arg);
#ifndef CFG_LOADER
	if( vo->dev_ops ){
		vo->dev_ops->set_power_down(!arg);
	}
#endif
	return 0;
}

static int vo_lcd_config(int arg)
{
	vout_t *vo;
	vout_info_t *vo_info;

	VPPMSG(KERN_DEBUG "vo_lcd_config\n");

	vo_info = (vout_info_t *) arg;
	vo = vout_get_info(VOUT_LCD);
	if( vo->dev_ops ){
		vo->dev_ops->config(vo_info);
	}
	govrh_set_dvo_clock_delay((vo->option[2] & LCD_CAP_CLK_HI)? 0:1, 0);
	return 0;
}

static int vo_lcd_init(int arg)
{
	vout_t *vo;
	unsigned int capability;

	VPPMSG(KERN_DEBUG "vo_lcd_init(%d)\n",arg);

	vo = vout_get_info(VOUT_LCD);
	if( vo->dev_ops ){
		vo->dev_ops->set_mode(&vo->option[0]);
	}
	govrh_set_dvo_enable(VPP_FLAG_ENABLE);
	govrh_set_dvo_color_format(VDO_COL_FMT_ARGB);
	govrh_set_dvo_outdatw(VPP_DATAWIDHT_24);
	capability = vo->option[2];
	govrh_set_dvo_clock_delay((capability & LCD_CAP_CLK_HI)? 0:1, 0);
	govrh_set_dvo_sync_polar((capability & LCD_CAP_HSYNC_HI)? 0:1,(capability & LCD_CAP_VSYNC_HI)? 0:1);
	p_govrh->fb_p->set_csc(p_govrh->fb_p->csc_mode);
	switch( vo->option[1] ){
		case 15:
			govrh_IGS_set_mode(0,1,1);
			break;
		case 16:
			govrh_IGS_set_mode(0,3,1);
			break;
		case 18:
			govrh_IGS_set_mode(0,2,1);
			break;
		case 24:
		default:
			govrh_IGS_set_mode(0,0,0);
			break;
	}
	g_vpp.vo_enable = 1;
	return 0;
}

static int vo_lcd_uninit(int arg)
{
	vout_t *vo;
	
	VPPMSG(KERN_DEBUG "vo_lcd_uninit(%d)\n",arg);
	govrh_set_dvo_enable(VPP_FLAG_DISABLE);
	vo = vout_get_info(VOUT_LCD);
	if( vo->dev_ops ){
		vo->dev_ops->set_mode(0);
	}
	return 0;
}

static int vo_lcd_suspend(int arg)
{
	vout_t *vo;

	VPPMSG(KERN_DEBUG "vo_lcd_suspend(%d)\n",arg);

	vo = vout_get_info(VOUT_LCD);	
	if( vo->dev_ops ){
		vo->dev_ops->set_power_down(1);
	}
	govrh_set_dvo_enable(VPP_FLAG_DISABLE);
	return 0;
}

static int vo_lcd_resume(int arg)
{
	vout_t *vo;
	
	VPPMSG(KERN_DEBUG "vo_lcd_resume(%d)\n",arg);

	govrh_set_dvo_enable(VPP_FLAG_ENABLE);
	mdelay(150);
	vo = vout_get_info(VOUT_LCD);	
	if( vo->dev_ops ){
#ifndef CONFIG_ANDROID 
		vo->dev_ops->set_power_down(0);
#endif
	}
	return 0;
}

static int vo_lcd_chkplug(int arg)
{
	vout_t *vo;
	
	vo = vout_get_info(VOUT_LCD);	
	if( vo->dev_ops ){
		return vo->dev_ops->check_plugin(0);
	}
	return 0;
}

vout_ops_t vo_lcd_ops = 
{
	.init = vo_lcd_init,
	.uninit = vo_lcd_uninit,
	.compatible = vo_lcd_compatible,
	.visible = vo_lcd_visible,
	.config = vo_lcd_config,
	.suspend = vo_lcd_suspend,
	.resume = vo_lcd_resume,
	.chkplug = vo_lcd_chkplug,
};
#endif /* WMT_FTBLK_VOUT_LCD */

vout_t vo_lcd_parm = {
#ifdef WMT_FTBLK_VOUT_LCD
	.ops = &vo_lcd_ops,
#else
	.ops = 0,
#endif
	.name = "LCD",
	.option[0] = LCD_PANEL_MAX,		/* [LCD] option1 : panel id */
	.option[1] = 24,				/* [LCD] option2 : bit per pixel */
	.option[2] = 0,					/* [LCD] option3 : capability */
};

void vo_set_lcd_id(int id)
{
	vo_lcd_parm.option[0] = id;
}

int vo_get_lcd_id(void)
{
	return vo_lcd_parm.option[0];
}

/*--------------------------------------- SD_ANALOG ---------------------------------------*/
#ifdef WMT_FTBLK_VOUT_SDA
static int vo_sda_compatible(int arg)
{
	vout_mode_t mode;

	VPPMSG(KERN_DEBUG "vo_sda_compatible(%d)\n",arg);
	mode = (vout_mode_t) arg;
	switch(mode){
		case VOUT_VGA:
//		case VOUT_DVI:
		case VOUT_DVO2HDMI:
		case VOUT_LCD:
		case VOUT_SD_DIGITAL:
		case VOUT_HDMI:
			return 1;
		default:
			break;
	}
	return 0;
}

static int vo_sda_visible(int arg)
{
	VPPMSG(KERN_DEBUG "vo_sda_visible(%d)\n",arg);
	if( arg ){
		disp_set_enable(VPP_FLAG_ENABLE);
	}
	else {
		disp_set_enable(VPP_FLAG_DISABLE);
	}
	return 0;
}

vpp_tvsys_t vo_res_to_tvsys(unsigned int resx,unsigned int resy,int interlace)
{
	if( resx == 1280 )
		return VPP_TVSYS_720P;

	if( resx == 1920 ){
		if( interlace )
			return VPP_TVSYS_1080I;
		return VPP_TVSYS_1080P;
	}
	
	if( resx != 720 ){
		return VPP_TVSYS_MAX;
	}

	if( resy == 576 ){
		return VPP_TVSYS_PAL;
	}
	return VPP_TVSYS_NTSC;
}

void vo_tvsys_to_res(vpp_tvsys_t tvsys,vout_info_t *info)
{
	switch(tvsys){
		default:
		case VPP_TVSYS_NTSC:
			info->resx = 720;
			info->resy = 480;
			info->pixclk = 27027060;
			break;
		case VPP_TVSYS_PAL:
			info->resx = 720;
			info->resy = 576;
			info->pixclk = 27000000;
			break;	
		case VPP_TVSYS_720P:
			info->resx = 1280;
			info->resy = 720;
			info->pixclk = 74250000;
			break;
		case VPP_TVSYS_1080I:
			info->resx = 1920;
			info->resy = 1080;
			info->pixclk = 74250060;
			break;
		case VPP_TVSYS_1080P:
			info->resx = 1920;
			info->resy = 1080;
			info->pixclk = 148500000;
			break;
	}
}

static int vo_sda_config(int arg)
{
	vout_info_t *vo_info;
	
	VPPMSG(KERN_DEBUG "vo_sda_config\n");

	vo_info = (vout_info_t *) arg;
	return 0;
}

static int vo_sda_init(int arg)
{
	vout_t *vo;

	vo = vout_get_info(VOUT_SD_ANALOG);

	VPPMSG(KERN_DEBUG "vo_sda_init(%d)\n",arg);

	p_disp->tvsys = vo->option[0];
	p_disp->tvconn = vo->option[1];
	vo->vo_option = (p_disp->tvsys == VPP_TVSYS_1080I)? VOUT_OPT_INTERLACE:0;
	govrh_DISP_set_enable(VPP_FLAG_ENABLE);
	vppm_set_DAC_select(1);
	govrh_DAC_set_sense_value(0,p_disp->dac_sense_val,p_disp->dac_sense_val);
	govrh_DAC_set_sense_value(1,p_disp->dac_sense_val,p_disp->dac_sense_val);
	govrh_DAC_set_sense_value(2,p_disp->dac_sense_val,p_disp->dac_sense_val);
	govrh_DAC_set_sense_sel(1);
	govrh_set_dual_disp(1,1);
	disp_EXTTV_set_enable(VPP_FLAG_DISABLE);
#ifdef CONFIG_WMT_INT_DEV_PLUG_DISABLE
	vpp_dac_sense_enable = 0;
	disp_DAC_set_pwrdn(0xFF,VPP_FLAG_DISABLE);
#endif	
	return 0;
}

static int vo_sda_uninit(int arg)
{
	VPPMSG(KERN_DEBUG "vo_sda_uninit(%d)\n",arg);
	disp_set_enable(VPP_FLAG_DISABLE);
	govrh_DISP_set_enable(VPP_FLAG_DISABLE);
	disp_DAC_set_pwrdn(0xFF,VPP_FLAG_ENABLE);
	vppm_set_DAC_select(0);
	govrh_DAC_set_sense_sel(0);
	govrh_set_dual_disp(0,1);	
	vppif_reg32_write(GOVRH_HSCALE_UP,0);
	return 0;
}

static int vo_sda_suspend(int arg)
{
	VPPMSG(KERN_DEBUG "vo_sda_suspend(%d)\n",arg);
	disp_set_enable(VPP_FLAG_DISABLE);
	disp_DAC_set_pwrdn(0xFF,VPP_FLAG_ENABLE);	
	return 0;
}

static int vo_sda_resume(int arg)
{
	VPPMSG(KERN_DEBUG "vo_sda_resume(%d)\n",arg);
	vppm_set_DAC_select(1);	
	disp_set_mode(p_disp->tvsys, p_disp->tvconn);	
//	disp_set_enable(VPP_FLAG_ENABLE);
	return 0;
}

static int vo_sda_chkplug(int arg)
{
	int plugin,sense;

	govrh_DAC_set_pwrdn(0);
#ifdef __KERNEL__
	vpp_irqproc_work(VPP_INT_GOVRH_PVBI,vpp_irqproc_monitor_dac_sense,&sense,1);
#else
	sense = govrh_DAC_monitor_sense();
#endif
	plugin = (sense)? 1:0;
	govrh_DAC_set_pwrdn((plugin)? 0:1);
	VPPMSG(KERN_DEBUG "vo_sda_chkplug %d,0x%x\n",plugin,sense);
	return plugin;
}

int vo_sda_plug_detect(void)
{
	static unsigned int sense_cnt = 0;
	static unsigned int pre_plugin;
	unsigned int plugin,sense_val;

	if( vppif_reg32_read(DISP_EXTTV_EN) ){
		return -1;
	}

	sense_cnt++;
	if( (sense_cnt % p_disp->dac_sense_cnt) != 0 ){
		return -1;
	}

	if( vppif_reg32_read(GOVRH_DAC_PWRDN) == 1 ){
		vppif_reg32_write(GOVRH_DAC_PWRDN, 0);	/* DAC power on */
		sense_cnt--;
		return -1;
	}

	vppif_reg32_write(VPP_DAC_SEL,0);
	sense_val = govrh_DAC_monitor_sense();
	vppif_reg32_write(VPP_DAC_SEL,1);
 	plugin = (sense_val)? 1:0;
	govrh_DAC_set_pwrdn((plugin)? 0:1);
	if( plugin != pre_plugin ){
		pre_plugin = plugin;
		DPRINT("[VOUT] SDA PLUG %s,0x%x\n",(plugin)?"IN":"OUT",sense_val);
	}
	return plugin;
}

vout_ops_t vo_sda_ops = 
{
	.init = vo_sda_init,
	.uninit = vo_sda_uninit,
	.compatible = vo_sda_compatible,
	.visible = vo_sda_visible,
	.config = vo_sda_config,
	.suspend = vo_sda_suspend,
	.resume = vo_sda_resume,
	.chkplug = vo_sda_chkplug,
};

vout_t vo_sda_parm = {
	.ops = &vo_sda_ops,
	.name = "SD_ANALOG",
	.option[0] = VPP_TVSYS_NTSC,
	.option[1] = VPP_TVCONN_CVBS,
	.option[2] = 0,
	.resx = 720,
	.resy = 480,
	.pixclk = 27000000,
};
#endif /* WMT_FTBLK_VOUT_SDA */

/*--------------------------------------- SD_DIGITAL ---------------------------------------*/
#ifdef WMT_FTBLK_VOUT_SDD
static int vo_sdd_compatible(int arg)
{
	vout_mode_t mode;

	VPPMSG(KERN_DEBUG "vo_sdd_compatible(%d)\n",arg);
	mode = (vout_mode_t) arg;
	switch(mode){
		case VOUT_VGA:
		case VOUT_DVI:
		case VOUT_DVO2HDMI:
		case VOUT_LCD:
		case VOUT_SD_ANALOG:
			return 1;
		default:
			break;
	}
	return 0;
}

static int vo_sdd_visible(int arg)
{
	VPPMSG(KERN_DEBUG "vo_sdd_visible(%d)\n",arg);
	if( arg ){
		disp_set_enable(VPP_FLAG_ENABLE);
	}
	else {
		disp_set_enable(VPP_FLAG_DISABLE);
	}
	return 0;
}

static int vo_sdd_config(int arg)
{
	vout_t *vo;
	vout_info_t *vo_info;

	VPPMSG(KERN_DEBUG "vo_sdd_config\n");

	vo = vout_get_info(VOUT_SD_DIGITAL);
	vo_info = (vout_info_t *) arg;
	if( vo->dev_ops ){
		vo->dev_ops->set_mode(&vo->option[0]);
	}
	return 0;
}

static int vo_sdd_init(int arg)
{
	vout_t *vo;

	VPPMSG(KERN_DEBUG "vo_sdd_init(%d)\n",arg);
	vo = vout_get_info(VOUT_SD_DIGITAL);
	p_disp->tvsys = vo->option[0];
	p_disp->tvconn = vo->option[1];
	vo->vo_option = (p_disp->tvsys == VPP_TVSYS_1080I)? VOUT_OPT_INTERLACE:0;	
	govrh_DISP_set_enable(VPP_FLAG_ENABLE);
	vppm_set_DAC_select(1);
	disp_EXTTV_set_enable(VPP_FLAG_ENABLE);
	disp_set_enable(VPP_FLAG_ENABLE);
	govrh_set_dvo_enable(VPP_FLAG_ENABLE);	// enable GPIO setting
	if( vo->dev_ops ){
		vo->dev_ops->set_mode(&vo->option[0]);
	}
	return 0;
}

static int vo_sdd_uninit(int arg)
{
	vout_t *vo;

	VPPMSG(KERN_DEBUG "vo_sdd_uninit(%d)\n",arg);
	vo = vout_get_info(VOUT_SD_DIGITAL);
	disp_set_enable(VPP_FLAG_DISABLE);
	disp_EXTTV_set_enable(VPP_FLAG_DISABLE);
	govrh_DISP_set_enable(VPP_FLAG_DISABLE);
	vppm_set_DAC_select(0);
	if( vo->dev_ops ){
		vo->dev_ops->set_power_down(VPP_FLAG_ENABLE);
	}
	govrh_set_dvo_enable(VPP_FLAG_DISABLE);	// disable GPIO setting
	return 0;
}

static int vo_sdd_suspend(int arg)
{
	vout_t *vo;

	VPPMSG(KERN_DEBUG "vo_sdd_suspend(%d)\n",arg);
	vo = vout_get_info(VOUT_SD_DIGITAL);
	if( vo->dev_ops ){
		vo->dev_ops->set_power_down(VPP_FLAG_ENABLE);
	}
	disp_set_enable(VPP_FLAG_DISABLE);
	return 0;
}

static int vo_sdd_resume(int arg)
{
	vout_t *vo;

	VPPMSG(KERN_DEBUG "vo_sdd_resume(%d)\n",arg);
	vo = vout_get_info(VOUT_SD_DIGITAL);
	REG32_VAL(GPIO_BASE_ADDR+0x44) = 0x0;
	REG32_VAL(GPIO_BASE_ADDR+0x48) = 0x0;
	vppm_set_DAC_select(1);
	disp_set_mode(p_disp->tvsys, p_disp->tvconn);
	if( vo->dev_ops ){
		vo->dev_ops->set_mode(&vo->option[0]);
	}
	return 0;
}

static int vo_sdd_chkplug(int arg)
{
	vout_t *vo;
	int plugin = 0;

	vo = vout_get_info(VOUT_SD_DIGITAL);
	if( vo->dev_ops ){
		plugin = vo->dev_ops->check_plugin(0);
	}
	return plugin;
}

vout_ops_t vo_sdd_ops = 
{
	.init = vo_sdd_init,
	.uninit = vo_sdd_uninit,
	.compatible = vo_sdd_compatible,
	.visible = vo_sdd_visible,
	.config = vo_sdd_config,
	.suspend = vo_sdd_suspend,
	.resume = vo_sdd_resume,
	.chkplug = vo_sdd_chkplug,
};

vout_t vo_sdd_parm = {
	.ops = &vo_sdd_ops,
	.name = "SD_DIGITAL",
	.option[0] = VPP_TVSYS_NTSC,
	.option[1] = VPP_TVCONN_CVBS,
	.option[2] = 0,
	.resx = 720,
	.resy = 480,
	.pixclk = 27000000,
};
#endif /* WMT_FTBLK_VOUT_SDD */

/*--------------------------------------- BOOT LOGO ---------------------------------------*/
#ifdef WMT_FTBLK_VOUT_BOOT
static int vo_boot_compatible(int arg)
{
	vout_mode_t mode;

	VPPMSG(KERN_DEBUG "vo_boot_compatible(%d)\n",arg);
	mode = (vout_mode_t) arg;
	switch(mode){
		case VOUT_VGA:
		case VOUT_DVI:
		case VOUT_DVO2HDMI:
		case VOUT_LCD:
		case VOUT_SD_ANALOG:
		case VOUT_SD_DIGITAL:
		case VOUT_HDMI:
		case VOUT_LVDS:
			return 1;
		default:
			break;
	}
	return 0;
}

static int vo_boot_visible(int arg)
{
	VPPMSG(KERN_DEBUG "vo_boot_visible(%d)\n",arg);
#ifdef WMT_FTBLK_GOVRH
	govrh_set_dvo_enable(arg);
#endif
#ifdef CONFIG_LCD_BACKLIGHT_WMT
	if( g_vpp.vo_enable ){
		lcd_blt_enable(lcd_blt_id,1);
	}
#endif
	return 0;
}

static int vo_boot_config(int arg)
{
	vout_info_t *vo_info;

	VPPMSG(KERN_DEBUG "vo_boot_config\n");

	vo_info = (vout_info_t *) arg;
	if( int_vout_mode != VOUT_MODE_MAX ) vout_set_mode(int_vout_mode,1);
	if( dvo_vout_mode != VOUT_MODE_MAX ) vout_set_mode(dvo_vout_mode,1);
	if( hdmi_vout_mode ) vout_set_mode(VOUT_HDMI,1);
	if ( g_vpp.govrh_preinit == 0 ){
		vout_config(VOUT_MODE_ALL,vo_info);
	}
	VPPMSG(KERN_DEBUG "Exit vo_boot_config\n");
	return 0;
}

static int vo_boot_init(int arg)
{
	vout_t *vo;

	vo = vout_get_info(VOUT_BOOT);

	VPPMSG(KERN_DEBUG "vo_boot_init(%d)\n",arg);
#if 0			
// #ifdef WMT_FTBLK_GOVRH
	govrh_get_framebuffer(&g_vpp.govr->fb_p->fb);
	p_govw->fb_p->fb.col_fmt = g_vpp.govr->fb_p->fb.col_fmt;
	if( g_vpp.vpp_path ){
		if( (p_govw->fb_p->capability & BIT(p_govrh->fb_p->fb.col_fmt)) == 0 ){
			g_vpp.govr->fb_p->fb.col_fmt = VDO_COL_FMT_YUV444;
			p_govw->fb_p->fb.col_fmt = VDO_COL_FMT_YUV444;
		}
		g_vpp.vpp_path_ori_fb = g_vpp.govr->fb_p->fb;
		vpp_set_govw_tg(VPP_FLAG_DISABLE);
		vppm_set_int_enable(VPP_FLAG_ENABLE,VPP_INT_GOVRH_PVBI+VPP_INT_GOVRH_VBIS);
	}
	else {
		if( (p_govw->fb_p->capability & BIT(p_govrh->fb_p->fb.col_fmt)) == 0 ){
#ifdef CONFIG_LCD_BACKLIGHT_WMT
			lcd_blt_enable(lcd_blt_id,0);
#endif
			g_vpp.govr->fb_p->fb.col_fmt = VDO_COL_FMT_YUV444;
			p_govw->fb_p->fb.col_fmt = VDO_COL_FMT_YUV444;
			g_vpp.govr->fb_p->set_color_fmt(VDO_COL_FMT_YUV444);
			govrh_set_csc_mode(p_govrh->fb_p->csc_mode);
			g_vpp.vo_enable = 1;
		}
	}
#else
#endif
	return 0;
}

static int vo_boot_uninit(int arg)
{
	VPPMSG(KERN_DEBUG "vo_boot_uninit(%d)\n",arg);
	return 0;
}

static int vo_boot_suspend(int arg)
{
	if( int_vout_mode != VOUT_MODE_MAX ) vout_set_mode(int_vout_mode,1);			
	if( dvo_vout_mode != VOUT_MODE_MAX ) vout_set_mode(dvo_vout_mode,1);
	vout_suspend(VOUT_MODE_ALL,arg);
	return 0;
}

static int vo_boot_resume(int arg)
{
	VPPMSG(KERN_DEBUG "vo_boot_resume(%d)\n",arg);
	return 0;
}

vout_t vo_boot_parm;
static int vo_boot_chkplug(int arg)
{
	vout_t *vo = 0;
	char buf[100];
	int varlen = 100;

	if( wmt_getsyspara("wmt.display.regop",(unsigned char *)buf,&varlen) == 0){
	    unsigned int addr;
	    unsigned int val;
	    char op;
	    char *p,*endp;

		p = buf;
	    while(1){
	        addr = simple_strtoul(p, &endp, 16);
	        if( *endp == '\0')
	            break;

	        op = *endp;
	        if( endp[1] == '~'){
	            val = simple_strtoul(endp+2, &endp, 16);
	            val = ~val;
	        }
	        else {
	            val = simple_strtoul(endp+1, &endp, 16);
	        }

	        printk("  reg op: 0x%X %c 0x%X\n", addr, op, val);
	        switch(op){
	            case '|': REG32_VAL(addr) |= val; break;
	            case '=': REG32_VAL(addr) = val; break;
	            case '&': REG32_VAL(addr) &= val; break;
	            default:
	                printk("Error, Unknown operator %c\n", op);
	        }

	        if(*endp == '\0')
	            break;
	        p = endp + 1;
	    }
	}

	if( wmt_getsyspara("wmt.display.param",(unsigned char *)buf,&varlen) == 0){
		printk("[VOUT] param %s\n",buf);
		vpp_parse_param(buf,(unsigned int *)vpp_vo_boot_arg,6);
		printk("[VOUT] boot parm vo %s opt %d,%d, %dx%d@%d\n",vpp_vout_str[vpp_vo_boot_arg[0]],vpp_vo_boot_arg[1],vpp_vo_boot_arg[2],
														vpp_vo_boot_arg[3],vpp_vo_boot_arg[4],vpp_vo_boot_arg[5]);
	}
	
	if( wmt_getsyspara("wmt.display.param2",(unsigned char *)buf,&varlen) == 0){
		printk("[VOUT] param2 %s\n",buf);
		vpp_parse_param(buf,(unsigned int *)vpp_vo_boot_arg2,6);
		printk("[VOUT] boot parm vo2 %s opt %d,%d, %dx%d@%d\n",vpp_vout_str[vpp_vo_boot_arg2[0]],vpp_vo_boot_arg2[1],vpp_vo_boot_arg2[2],
														vpp_vo_boot_arg2[3],vpp_vo_boot_arg2[4],vpp_vo_boot_arg2[5]);
	}

	if( vpp_vo_boot_arg[0] < VOUT_MODE_MAX ){	// boot argument
		int oem_tmr = 0;
		vpp_timing_t *tp = 0;

		vo = vout_get_info(vpp_vo_boot_arg[0]);
		if( vo == 0 ){
			DPRINT("[VOUT] *W* uboot param dev not exist\n");
			vpp_vo_boot_arg[0] = VOUT_MODE_MAX;
			return 0;
		}
		vo->option[0] = vpp_vo_boot_arg[1];
		vo->option[1] = vpp_vo_boot_arg[2];
		if( vpp_vo_boot_arg[0]==VOUT_LCD ){ // LCD OEM
			if( vpp_vo_boot_arg[1]==0 ){
				oem_tmr = 1;
			}
			tp = &vo_oem_tmr;				
			tp->hpixel = vpp_vo_boot_arg[3];
			tp->vpixel = vpp_vo_boot_arg[4];
			tp->pixel_clock = vpp_vo_boot_arg[5];

#ifdef CONFIG_LCD_BACKLIGHT_WMT
			if( wmt_getsyspara("wmt.display.pwm",(unsigned char *)buf,&varlen) == 0 ){
				unsigned int id,parm1,parm2,parm3;
				unsigned int param[4];
				int parm_cnt;
				
				printk("[VOUT] pwm %s\n",buf);
				parm_cnt = vpp_parse_param(buf,(unsigned int *)param,4);
				id = param[0];
				parm1 = param[1];
				parm2 = param[2];
				parm3 = param[3];
				id = id & 0xF; // bit0-3 pwm number,bit4 invert
				if( parm_cnt == 3 ){
					lcd_blt_id = id;
					lcd_blt_freq = parm1;
					lcd_blt_level = parm2;
					lcd_blt_set_freq(id, lcd_blt_freq);
					lcd_blt_set_level(id, lcd_blt_level);
					DPRINT("[VOUT] blt %d,freq %d,level %d\n",lcd_blt_id,lcd_blt_freq,lcd_blt_level);
				}
				else {
#ifndef CFG_LOADER
					lcd_blt_set_pwm(id,parm1,parm2,parm3);
#endif
					DPRINT("[VOUT] blt %d,scalar %d,period %d,duty %d\n",lcd_blt_id,parm1,parm2,parm3);
				}
			}
#endif
		}
		else if( vpp_vo_boot_arg[0]==VOUT_LVDS ){
			DPRINT("[VOUT] LVDS %d:%d:%d\n",vpp_vo_boot_arg[0],vpp_vo_boot_arg[1],vpp_vo_boot_arg[2]);
			if( vpp_vo_boot_arg[1] == 0 ){
				char *edid;
				if( (edid = vout_get_edid(VOUT_LVDS)) ){
					edid_parse(edid);
					if( edid_info.detail_timing[0].pixel_clock ){
						tp = &edid_info.detail_timing[0];
						vo_oem_tmr = *tp;
						lvds_plug = 1;
					}
				}
				else {
					DPRINT("edid fail\n");
				}
			}
			else {
				if( (p_lcd = lcd_get_parm((lcd_panel_t) vpp_vo_boot_arg[1],vpp_vo_boot_arg[2])) ){
					tp = &p_lcd->timing;
					vo_oem_tmr = *tp;
					lvds_plug = 1;
				}
			}

			if( lvds_plug ){
				DPRINT("[LVDS] %dx%d\n",tp->hpixel,tp->vpixel);
			}
		}
		else if((vpp_vo_boot_arg[3]==0) || (vpp_vo_boot_arg[4]==0) || (vpp_vo_boot_arg[5]==0)){
			oem_tmr = 1;
		}
		else {
			int index = 0;
			tp = vpp_get_video_mode(vpp_vo_boot_arg[3], vpp_vo_boot_arg[4], vpp_vo_boot_arg[5],&index);
		}

		if( oem_tmr ){
			if( wmt_getsyspara("wmt.display.tmr",(unsigned char *)buf,&varlen) == 0 ){
				tp = &vo_oem_tmr;				
				DPRINT("[VOUT] tmr %s\n",buf);
				{
				unsigned int param[10];
				
				vpp_parse_param(buf,param,10);
				tp->pixel_clock = param[0];
				tp->option = param[1];
				tp->hsync = param[2];
				tp->hbp = param[3];
				tp->hpixel = param[4];
				tp->hfp = param[5];
				tp->vsync = param[6];
				tp->vbp = param[7];
				tp->vpixel = param[8];
				tp->vfp = param[9];
				}
				tp->pixel_clock *= 1000;
				DPRINT("[VOUT] tmr pixclk %d,option 0x%x,hsync %d,hbp %d,hpixel %d,hfp %d,vsync %d,vbp %d,vpixel %d,vfp %d\n",
							tp->pixel_clock,tp->option,tp->hsync,tp->hbp,tp->hpixel,tp->hfp,tp->vsync,tp->vbp,tp->vpixel,tp->vfp);
			}
			else {
				oem_tmr = 0;
			}
		}
		vo_boot_parm.resx = tp->hpixel;
		vo_boot_parm.resy = tp->vpixel;
		if( tp->option & VPP_OPT_INTERLACE ){
			vo_boot_parm.resy *= 2;
		}
		vo_boot_parm.pixclk = (oem_tmr)? 0:tp->pixel_clock;	// To use OEM timing , pixel clock equal 0
		vo_oem_tmr.pixel_clock = (oem_tmr)? vo_oem_tmr.pixel_clock:0;
		dvo_vout_mode = vpp_vo_boot_arg[0];
		if( vpp_vo_boot_arg2[0] < VOUT_MODE_MAX ){
			vout_t *vo2;
			vo2 = vout_get_info(vpp_vo_boot_arg2[0]);
			if( vo2 == 0 ){
				DPRINT("[VOUT] *W* uboot param2 dev not exist\n");
				vpp_vo_boot_arg2[0] = VOUT_MODE_MAX;
				return 0;
			}
			vo2->option[0] = vpp_vo_boot_arg2[1];
			vo2->option[1] = vpp_vo_boot_arg2[2];
			int_vout_mode = vpp_vo_boot_arg2[0];
		}
		int_vout_mode = (vpp_vo_boot_arg2[0] < VOUT_MODE_MAX)? vpp_vo_boot_arg2[0]:VOUT_MODE_MAX;
		printk("[VOUT] boot argument vo1 %d,vo2 %d\n",dvo_vout_mode,int_vout_mode);
		return 1;
	}
	hdmi_vout_mode = 1;
//vo_boot_chkplug_end:
	return 0;
}

vout_ops_t vo_boot_ops = 
{
	.init = vo_boot_init,
	.uninit = vo_boot_uninit,
	.compatible = vo_boot_compatible,
	.visible = vo_boot_visible,
	.config = vo_boot_config,
	.suspend = vo_boot_suspend,
	.resume = vo_boot_resume,
	.chkplug = vo_boot_chkplug,
};

vout_t vo_boot_parm = {
	.ops = &vo_boot_ops,
	.name = "BOOT",
	.option[0] = 0,
	.option[1] = 0,
	.option[2] = 0,
	.resx = 0,
	.resy = 0,
	.pixclk = 0,
};
#endif //fan
/*--------------------------------------- HDMI ---------------------------------------*/
#ifdef WMT_FTBLK_VOUT_HDMI
#define VOUT_HDMI_CP_TIME 3	// should more than 2 seconds

#ifdef __KERNEL__
struct timer_list hdmi_cp_timer;
#endif

void vo_hdmi_cp_set_enable_tmr(int sec)
{
	VPPMSG("[HDMI] set enable tmr %d sec\n",sec);

	if( sec == 0 ){
		hdmi_set_cp_enable(VPP_FLAG_ENABLE);
		return ;
	}
	
#ifdef __KERNEL__
	if( hdmi_cp_timer.function ){
		mod_timer(&hdmi_cp_timer,(jiffies + sec * HZ));
	}
	else {
		init_timer(&hdmi_cp_timer);
		hdmi_cp_timer.data = VPP_FLAG_ENABLE;
		hdmi_cp_timer.function = (void *) hdmi_set_cp_enable;
		hdmi_cp_timer.expires = jiffies + sec * HZ;
		add_timer(&hdmi_cp_timer);
	}
#else
	mdelay(sec*1000);
	hdmi_set_cp_enable(VPP_FLAG_ENABLE);
#endif	
}

#ifndef CFG_LOADER
static irqreturn_t vo_hdmi_cp_interrupt
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
	vout_t *vo;

//	DPRINT("vo_hdmi_cp_interrupt %d\n",irq);
	vo = vout_get_info(VOUT_HDMI);
	switch( hdmi_check_cp_int() ){
		case 1:
			hdmi_set_cp_enable(VPP_FLAG_DISABLE);
			vo_hdmi_cp_set_enable_tmr(VOUT_HDMI_CP_TIME);
			vo->status &= ~VPP_VOUT_STS_CONTENT_PROTECT;
			break;
		case 0:
			vo->status |= VPP_VOUT_STS_CONTENT_PROTECT;
#ifndef CONFIG_HDMI_QUANTUMDATA
			if( g_vpp.hdmi_bksv[0] == 0 ){
				hdmi_get_bksv(&g_vpp.hdmi_bksv[0]);
				VPPMSG("[VOUT] get BKSV 0x%x 0x%x\n",g_vpp.hdmi_bksv[0],g_vpp.hdmi_bksv[1]);
			}
#endif
			break;
		case 2:
			hdmi_ri_tm_cnt = 3 * 30;
			break;
		default:
			break;
	}
	return IRQ_HANDLED;	
}

#ifdef __KERNEL__
static void vo_hdmi_do_plug(struct work_struct *ptr)
#else
static void vo_hdmi_do_plug(void)
#endif
{
	vout_t *vo;
	int plugin;

	plugin = hdmi_check_plugin(1);
	vo = vout_get_info(VOUT_HDMI);
	vout_change_status(vo,VOCTL_CHKPLUG,0,plugin);
	VPPMSG(KERN_DEBUG "vo_hdmi_do_plug %d\n",plugin);
	if( plugin == 0 ){
		g_vpp.hdmi_bksv[0] = g_vpp.hdmi_bksv[1] = 0;
	}
	return;
}
DECLARE_DELAYED_WORK(vo_hdmi_plug_work, vo_hdmi_do_plug);

static irqreturn_t vo_hdmi_plug_interrupt
(
	int irq, 				/*!<; // irq id */
	void *dev_id 			/*!<; // device id */
)
{
//	DPRINT("vo_hdmi_plug_interrupt %d\n",irq);
	hdmi_clear_plug_status();
#ifdef __KERNEL__
	schedule_delayed_work(&vo_hdmi_plug_work, HZ/10);
#else
	vo_hdmi_do_plug(0);
#endif
	return IRQ_HANDLED;	
}
#endif //fan

static int vo_hdmi_init(int arg)
{
	vout_t *vo;

	VPPMSG("[VOUT] vo_hdmi_init(%d)\n",arg);

	vo = vout_get_info(VOUT_HDMI);
	vo->vo_option = vo->option[1];
	vout_change_status(vout_get_info(VOUT_HDMI),VOCTL_CHKPLUG,0,hdmi_check_plugin(0));
	lvds_set_enable(0);
	hdmi_enable_plugin(1);
#ifndef CFG_LOADER
	if ( vpp_request_irq(VPP_IRQ_HDMI_CP, vo_hdmi_cp_interrupt, SA_INTERRUPT, "hdmi cp", (void *) 0) ) {
		VPPMSG("*E* request HDMI ISR fail\n");
	}
	if ( vpp_request_irq(VPP_IRQ_HDMI_HPDH, vo_hdmi_plug_interrupt, SA_INTERRUPT, "hdmi plug", (void *) 0) ) {
		VPPMSG("*E* request HDMI ISR fail\n");
	}
	if ( vpp_request_irq(VPP_IRQ_HDMI_HPDL, vo_hdmi_plug_interrupt, SA_INTERRUPT, "hdmi plug", (void *) 0) ) {
		VPPMSG("*E* request HDMI ISR fail\n");
	}
#endif
	return 0;
}

static int vo_hdmi_uninit(int arg)
{
	VPPMSG(KERN_DEBUG "vo_hdmi_uninit(%d)\n",arg);
	hdmi_enable_plugin(0);
	hdmi_set_cp_enable(VPP_FLAG_DISABLE);
	hdmi_set_enable(VPP_FLAG_DISABLE);
#ifndef CFG_LOADER
	vpp_free_irq(VPP_IRQ_HDMI_CP,(void *) 0);
	vpp_free_irq(VPP_IRQ_HDMI_HPDH,(void *) 0);
	vpp_free_irq(VPP_IRQ_HDMI_HPDL,(void *) 0);
#endif
	return 0;
}

static int vo_hdmi_suspend(int arg)
{
	VPPMSG(KERN_DEBUG "vo_hdmi_suspend(%d)\n",arg);
	hdmi_set_enable(VPP_FLAG_DISABLE);
	return 0;
}

static int vo_hdmi_resume(int arg)
{
	VPPMSG(KERN_DEBUG "vo_hdmi_resume(%d)\n",arg);
	hdmi_set_enable(VPP_FLAG_ENABLE);
	return 0;
}

static int vo_hdmi_compatible(int arg)
{
	vout_mode_t mode;

	VPPMSG(KERN_DEBUG "vo_hdmi_compatible(%d)\n",arg);
	mode = (vout_mode_t) arg;
	switch(mode){
		case VOUT_LVDS:
		case VOUT_SD_ANALOG:
		case VOUT_SD_DIGITAL:
		case VOUT_BOOT:
			return 1;
		default:
			break;
	}
	return 0;
}

static int vo_hdmi_visible(int arg)
{
	VPPMSG(KERN_DEBUG "vo_hdmi_visible(%d)\n",arg);

	hdmi_set_enable(arg);
	if( g_vpp.hdmi_cp_enable && arg ){
		vo_hdmi_cp_set_enable_tmr(2);
	}
	else {
		hdmi_set_cp_enable(VPP_FLAG_DISABLE);
	}
	return 0;
}

static int vo_hdmi_config(int arg)
{
	vout_t *vo;
	vout_info_t *vo_info;
	hdmi_info_t hdmi_info;
	vdo_color_fmt colfmt;
	unsigned int option = 0;

	hdmi_set_enable(0);

	vo = vout_get_info(VOUT_HDMI);	
	vo_info = (vout_info_t *) arg;

	VPPMSG(KERN_DEBUG "vo_hdmi_config(%dx%d@%d)\n",vo_info->resx,vo_info->resy,vo_info->fps);
	VPPMSG(KERN_DEBUG "vo_hdmi_config(op 0x%x,0x%x)\n",vo->option[0],vo->option[1]);
	
	colfmt = (vo->option[0]==VDO_COL_FMT_YUV422V)? VDO_COL_FMT_YUV422H:vo->option[0];
	hdmi_cur_plugin = hdmi_check_plugin(0);
	if( (hdmi_cur_plugin ) ){
#ifdef CONFIG_WMT_EDID
		unsigned char *buf;
		
		buf = (unsigned char *)vout_get_edid(VOUT_HDMI);
		option = edid_parse_option(buf);
#endif
#ifdef CONFIG_VPP_DEMO
		option |= EDID_OPT_HDMI + EDID_OPT_AUDIO;
#endif
	}

	hdmi_info.option = option;
	hdmi_info.channel = g_vpp.hdmi_audio_channel;
	hdmi_info.freq = g_vpp.hdmi_audio_freq;
	hdmi_info.outfmt = colfmt;
	hdmi_info.vic = VPP_GET_OPT_HDMI_VIC(vo_info->option);
	govrh_set_csc_mode(p_govrh->fb_p->csc_mode);
	hdmi_set_sync_low_active((vo_info->option & VPP_VGA_HSYNC_POLAR_HI)? 0:1,(vo_info->option & VPP_VGA_VSYNC_POLAR_HI)? 0:1);
	hdmi_config(&hdmi_info);
#ifdef CONFIG_HDMI_QUANTUMDATA
	hdmi_set_cp_enable((g_vpp.hdmi_cp_enable && hdmi_cur_plugin));
#else
	if( g_vpp.hdmi_cp_enable && hdmi_cur_plugin ){
		vo_hdmi_cp_set_enable_tmr(2);
	}
	else {
		hdmi_set_cp_enable(VPP_FLAG_DISABLE);
	}
#endif
	hdmi_set_enable(hdmi_cur_plugin);
	return 0;
}

static int vo_hdmi_chkplug(int arg)
{
	int plugin;
	plugin = hdmi_get_plugin();
	VPPMSG(KERN_DEBUG "vo_hdmi_chkplug %d\n",plugin);	
	return plugin;
}

static int vo_hdmi_get_edid(int arg)
{
	char *buf;
#ifdef CONFIG_WMT_EDID
	int i,cnt;
#endif
	VPPMSG(KERN_DEBUG "vo_hdmi_get_edid\n");
	buf = (char *) arg;
#ifdef CONFIG_WMT_EDID
	memset(&buf[0],0x0,128*EDID_BLOCK_MAX);
	if( hdmi_DDC_read(0xA0,0x0,&buf[0],128) ){
		DPRINT("[VOUT] *E* hdmi read edid\n");
		return 1;
	}

	if( edid_checksum((unsigned char *)buf,128) ){
//		vpp_dbg_gpio(0);
		DPRINT("[VOUT] *E* hdmi checksum\n");
//		edid_dump(buf);
//		vpp_dbg_gpio(1);
		g_vpp.dbg_hdmi_ddc_crc_err++;
		return 1;
	}

	cnt = buf[0x7E];
	if( cnt >= 3 ) cnt = 3;
	for(i=1;i<=cnt;i++){
		hdmi_DDC_read(0xA0,0x80*i,&buf[128*i],128);
	}
#endif
	return 0;
}

vout_ops_t vo_hdmi_ops = 
{
	.init = vo_hdmi_init,
	.uninit = vo_hdmi_uninit,
	.compatible = vo_hdmi_compatible,
	.visible = vo_hdmi_visible,
	.config = vo_hdmi_config,
	.suspend = vo_hdmi_suspend,
	.resume = vo_hdmi_resume,
	.chkplug = vo_hdmi_chkplug,
	.get_edid = vo_hdmi_get_edid,
};

vout_t vo_hdmi_parm = {
	.ops = &vo_hdmi_ops,
	.name = "HDMI",
	.option[0] = VDO_COL_FMT_ARGB,
	.option[1] = VPP_DATAWIDHT_24,
	.option[2] = 0,
#ifdef CONFIG_HDMI_QUANTUMDATA
	.resx = 720,
	.resy = 480,
	.pixclk = 27027060,
#else
	.resx = 1280,
	.resy = 720,
	.pixclk = 74250060,
#endif
};
#endif /* WMT_FTBLK_VOUT_HDMI */

/*--------------------------------------- LVDS ---------------------------------------*/
#ifdef WMT_FTBLK_VOUT_LVDS
static int vo_lvds_compatible(int arg)
{
	vout_mode_t mode;

	VPPMSG(KERN_DEBUG "vo_lvds_compatible(%d)\n",arg);
	mode = (vout_mode_t) arg;
	switch(mode){
		case VOUT_BOOT:
		case VOUT_DVO2HDMI:
		case VOUT_DVI:
		case VOUT_SD_DIGITAL:
		case VOUT_SD_ANALOG:
		case VOUT_VGA:
		case VOUT_HDMI:
			return 1;
		default:
			break;
	}
	return 0;
}

static int vo_lvds_visible(int arg)
{
	vout_t *vo;

	VPPMSG(KERN_DEBUG "vo_lvds_visible(%d)\n",arg);
	vo = vout_get_info(VOUT_LVDS);
	lvds_set_enable(arg);
	return 0;
}

static int vo_lvds_config(int arg)
{
	vout_t *vo;
	vout_info_t *vo_info;

	VPPMSG(KERN_DEBUG "vo_lvds_config\n");

	vo_info = (vout_info_t *) arg;
	vo = vout_get_info(VOUT_LVDS);
	return 0;
}

static int vo_lvds_init(int arg)
{
	vout_t *vo;
	unsigned int capability;

	VPPMSG(KERN_DEBUG "vo_lvds_init(%d)\n",arg);

	vo = vout_get_info(VOUT_LVDS);
	capability = vo->option[2];
	lvds_set_power_down(VPP_FLAG_DISABLE);
	lvds_set_enable(VPP_FLAG_ENABLE);
	govrh_set_csc_mode(p_govrh->fb_p->csc_mode);
	switch( vo->option[1] ){
		case 15:
			lvds_set_rgb_type(1);
			break;
		case 16:
			lvds_set_rgb_type(3);
			break;
		case 18:
			lvds_set_rgb_type(2);
			break;
		case 24:
		default:
			lvds_set_rgb_type(0);
			break;
	}
	return 0;
}

static int vo_lvds_uninit(int arg)
{
	vout_t *vo;
	
	VPPMSG(KERN_DEBUG "vo_lvds_uninit(%d)\n",arg);
	lvds_set_enable(VPP_FLAG_DISABLE);
	vo = vout_get_info(VOUT_LVDS);
	lvds_set_power_down(VPP_FLAG_ENABLE);
	return 0;
}

static int vo_lvds_suspend(int arg)
{
	vout_t *vo;

	VPPMSG(KERN_DEBUG "vo_lvds_suspend(%d)\n",arg);

	vo = vout_get_info(VOUT_LVDS);	
	lvds_set_enable(VPP_FLAG_DISABLE);
	return 0;
}

static int vo_lvds_resume(int arg)
{
	vout_t *vo;
	
	VPPMSG(KERN_DEBUG "vo_lvds_resume(%d)\n",arg);

	lvds_set_enable(VPP_FLAG_ENABLE);
	mdelay(150);
	vo = vout_get_info(VOUT_LVDS);
	return 0;
}

static int vo_lvds_chkplug(int arg)
{
	vout_t *vo;

	VPPMSG(KERN_DEBUG "vo_lvds_chkplug\n");
	
	vo = vout_get_info(VOUT_LVDS);
	return lvds_plug;
}

vout_ops_t vo_lvds_ops = 
{
	.init = vo_lvds_init,
	.uninit = vo_lvds_uninit,
	.compatible = vo_lvds_compatible,
	.visible = vo_lvds_visible,
	.config = vo_lvds_config,
	.suspend = vo_lvds_suspend,
	.resume = vo_lvds_resume,
	.chkplug = vo_lvds_chkplug,
#ifdef WMT_FTBLK_VOUT_HDMI
	.get_edid = vo_hdmi_get_edid,
#endif
};

vout_t vo_lvds_parm = {
	.ops = &vo_lvds_ops,
	.name = "LVDS",
	.option[0] = VDO_COL_FMT_ARGB,
	.option[1] = 24,
	.option[2] = 0,
	.resx = 1024,
	.resy = 768,
	.pixclk = 65000000,
};
#endif /* WMT_FTBLK_VOUT_LVDS */

vout_t *vo_parm_table[] = {
#ifdef WMT_FTBLK_VOUT_SDA
	&vo_sda_parm,
#else
	0,
#endif

#ifdef WMT_FTBLK_VOUT_SDD
	&vo_sdd_parm,
#else
	0,
#endif

#ifdef WMT_FTBLK_VOUT_LCD
	&vo_lcd_parm,
#else
	0,
#endif

#ifdef WMT_FTBLK_VOUT_DVI
	&vo_dvi_parm,
#else
	0,
#endif

#ifdef WMT_FTBLK_VOUT_HDMI
	&vo_hdmi_parm,
#else
	0,
#endif

#ifdef WMT_FTBLK_VOUT_DVO2HDMI	
	&vo_dvo2hdmi_parm,
#else
	0,
#endif

#ifdef WMT_FTBLK_VOUT_LVDS
	&vo_lvds_parm,
#else
	0,
#endif

#ifdef WMT_FTBLK_VOUT_VGA
	&vo_vga_parm,
#else
	0,
#endif

#ifdef WMT_FTBLK_VOUT_BOOT
	&vo_boot_parm,
#else
	0,
#endif
};

/*--------------------------------------- API ---------------------------------------*/
#ifndef CFG_LOADER
int vout_set_audio(vout_audio_t *arg)
{
	vout_t *vout;
	int ret = 0;

	vout = vout_get_info(VOUT_DVO2HDMI);
	if( vout && (vout->status & VPP_VOUT_STS_PLUGIN)){
		if( vout->dev_ops->set_audio ){
			vout->dev_ops->set_audio(arg);
		}
	}

#ifdef WMT_FTBLK_VOUT_HDMI
	vout = vout_get_info(VOUT_HDMI);
//	if( vout && (vout->status & VPP_VOUT_STS_PLUGIN)){
	if( vout ){
		hdmi_config_audio(arg);
		ret = 1;
	}
#endif
	return ret;
}

void vout_plug_detect(vout_mode_t mode)
{
	int plugin = -1;

	if( vpp_dac_sense_enable == 0 ){
		return;
	}

	switch(mode){
#ifdef WMT_FTBLK_VOUT_VGA		
		case VOUT_VGA:
			plugin = vo_vga_plug_detect();
			break;
#endif
#ifdef WMT_FTBLK_VOUT_SDA
		case VOUT_SD_ANALOG:
			plugin = vo_sda_plug_detect();
			break;
#endif
#ifdef WMT_FTBLK_VOUT_HDMI
		case VOUT_HDMI:
			if( hdmi_poll_cp_status() ){
				vo_hdmi_cp_set_enable_tmr(VOUT_HDMI_CP_TIME);
			}
			break;
#endif
		default:
			break;
	}

	if( plugin >= 0 ){
		vout_t *vo;

		vo = vout_get_info(mode);
		vout_change_status(vo,VOCTL_CHKPLUG,0,plugin);
	}
}
#endif
#define VOUT_EXTDEV_DEFAULT_MODE	VOUT_DVI	// default external vout (VT1632 hw mode)
vout_mode_t vout_extdev_probe(void)
{
	vout_mode_t mode = VOUT_MODE_MAX;
	vout_t *parm = 0;
	vout_dev_ops_t *dev = 0;

	do {
		dev = vout_get_device(dev);
		if( dev == 0 ) break;
		if( dev->init() == 0 ){
			mode = dev->mode;
			parm = vo_parm_table[mode];
			parm->dev_ops = dev;
			break;
		}
		if( dev->mode == VOUT_EXTDEV_DEFAULT_MODE ){
			mode = dev->mode;
			parm = vo_parm_table[mode];
			parm->dev_ops = 0; // dev;
		}
	} while(1);

	if( mode == VOUT_MODE_MAX ){
		mode = VOUT_EXTDEV_DEFAULT_MODE;
		parm = vo_parm_table[mode];
		parm->dev_ops = 0; // dev;
	}

	if( (mode == VOUT_EXTDEV_DEFAULT_MODE) && (parm->dev_ops == 0) ){
		DPRINT("[VT1632] DVI ext device (hw mode)\n");
	}

	vout_register(mode,parm);	
	printk("[VOUT] ext dev : %s\n",(parm)? parm->name:"NO");
	return mode;
}

#define VOUT_INTDEV_DEFAULT_MODE	VOUT_VGA	// default internal vout (VGA)
vout_mode_t vout_intdev_probe(void)
{
	vout_mode_t mode = VOUT_MODE_MAX;

#ifdef WMT_FTBLK_VOUT_VGA
	vout_register(VOUT_VGA,&vo_vga_parm);
	mode = VOUT_VGA;
#endif
#ifdef WMT_FTBLK_VOUT_SDA
	vout_register(VOUT_SD_ANALOG,&vo_sda_parm);
#endif
#ifdef WMT_FTBLK_LVDS
	if(	(!g_vpp.govrh_preinit) && (vppif_reg32_read(HDMI_ENABLE)==0))
		lvds_init();
#endif
#ifdef WMT_FTBLK_VOUT_LVDS
	vout_register(VOUT_LVDS,&vo_lvds_parm);
#endif
#ifdef WMT_FTBLK_VOUT_HDMI
	hdmi_init();
	vout_register(VOUT_HDMI,&vo_hdmi_parm);
#endif
	printk("[VOUT] int dev : %s\n",(mode != VOUT_MODE_MAX)? vo_parm_table[mode]->name:"NO");
	return mode;
}

vout_mode_t vout_priority[] = { VOUT_BOOT, VOUT_LCD, VOUT_LVDS, VOUT_HDMI, VOUT_DVO2HDMI, VOUT_DVI, VOUT_SD_DIGITAL, VOUT_VGA, VOUT_SD_ANALOG, VOUT_MODE_MAX };
#ifdef CFG_LOADER
extern vout_t vo_boot_parm;
#endif
int vout_init(vout_info_t *info)
{
	vout_t *vo = 0;
	int sense_enable;
	
	VPPMSG(KERN_ALERT "vo_init_wmt\n");

	/* check video out device */
	dvo_vout_mode = vout_extdev_probe();
	int_vout_mode = vout_intdev_probe();

	/* disable video out by uboot param */
	{
		unsigned int temp = 10;
		unsigned char buf[10],*endp;
		int i;
	
		if( wmt_getsyspara("wmt.display.vout.disable",buf,&temp) == 0 ){
			temp = simple_strtoul(buf, &endp, 16);
			for(i=0;i<VOUT_MODE_MAX;i++){
				if( temp & (0x1 << i) ){
					vout_unregister(i);
				}
			}
		}
	}
	
    vout_register(VOUT_BOOT,&vo_boot_parm);
	{
		int i;
		int match = 0;
		unsigned int user_resx,user_resy,user_freq;
#ifndef CFG_LOADER
		unsigned char buf[40];
		int varlen = 40;

		if( wmt_getsyspara("wmt.display.user_res",buf,&varlen) ){
			user_resx = user_resy = user_freq = 0;
		}
		else {
			sscanf(buf,"%dx%d@%d",&user_resx,&user_resy,&user_freq);
			DPRINT("user res %dx%d@%d\n",user_resx,user_resy,user_freq);
		}

		if( wmt_getsyspara("wmt.display.sda_tvsys",buf,&varlen) == 0 ){
			int tvsys;

			vo = vout_get_info(VOUT_SD_ANALOG);
			sscanf(buf,"%d",&tvsys);
			DPRINT("sda tvsys %d\n",tvsys);
			vo->option[0] = tvsys;
			int_vout_mode = VOUT_MODE_MAX;
			switch( tvsys ){
				case VPP_TVSYS_NTSC:
					vo->resx = 720;
					vo->resy = 480;
					break;
				case VPP_TVSYS_PAL:
					vo->resx = 720;
					vo->resy = 576;
					break;
			}
		}
#else
		user_resx = user_resy = user_freq = 0;
#endif
		// detect plugin
		sense_enable = vpp_dac_sense_enable;
		vpp_dac_sense_enable = 0;
		for(i=0;vout_priority[i]!=VOUT_MODE_MAX;i++){
			if( vout_get_info(vout_priority[i]) ){
				if( vout_chkplug(vout_priority[i]) ){
					if( vout_priority[i] == VOUT_HDMI )
						hdmi_vout_mode = 1;
					vo = vout_get_info(vout_priority[i]);
					if( user_resx == 0 ){
						if( vout_priority[i] == VOUT_VGA ){
							user_resx = 1440;
							user_resy = 1024;
							user_freq = 60;
						}
						else {
							user_resx = vo->resx;
							user_resy = vo->resy;
							user_freq = VPP_HD_DISP_FPS;
						}
					}
					
#ifdef CONFIG_WMT_EDID
					if( vout_get_edid(vout_priority[i]) ){
						int edid_option;
						
						if( (edid_option = edid_parse((unsigned char *)vo->edid)) == 0 ){
							DPRINT("*E* EDID data fail\n");
						}
						else {
							vpp_timing_t *edid_timing;
							if( edid_find_support(user_resx,user_resy,user_freq,&edid_timing) ){
								match = 1;
							}
							else {
								if( vout_find_edid_support_mode(&user_resx,&user_resy,&user_freq,
															(edid_option & EDID_OPT_16_9)? 1:0) ){
									match = 1;
								}
							}
						}
					}
					else {
						if( vout_priority[i] != VOUT_BOOT ){
							DPRINT("*E* read EDID fail (%s)\n",vpp_vout_str[vout_priority[i]]);
						}
					}
#endif

					if( match ){
						info->resx = user_resx;
						info->resy = user_resy;
						info->fps = user_freq;
					}
					else { 	// use default
						info->resx = vo->resx;
						info->resy = vo->resy;
						info->fps = vo->pixclk;
						match = 1;
					}
					break;
				}
			}
		}
		vpp_dac_sense_enable = sense_enable;
		
		if( match == 0 ){ // use default in no uboot parameter
			info->resx = 1024;
			info->resy = 768;
			info->fps = 60;
		}
		
		if ( g_vpp.govrh_preinit ){
			if( vout_priority[i] == VOUT_SD_ANALOG ){
				int_vout_mode = VOUT_SD_ANALOG;
				dvo_vout_mode = VOUT_MODE_MAX;
				hdmi_vout_mode = 0;
			}
			vout_set_mode(VOUT_BOOT,1);
		}
		else {
			if( vout_priority[i] != VOUT_SD_ANALOG ){
			    if( (dvo_vout_mode != VOUT_MODE_MAX) && (dvo_vout_mode !=vout_priority[i]) ) vout_set_mode(dvo_vout_mode,1);
			    if( (int_vout_mode != VOUT_MODE_MAX) && (int_vout_mode !=vout_priority[i]) ) vout_set_mode(int_vout_mode,1);
				if( hdmi_vout_mode ) vout_set_mode(VOUT_HDMI,1);
			}
		    if( vout_priority[i] < VOUT_BOOT ) vout_set_mode(vout_priority[i],1);
        }
	}
	info->pixclk = (info->fps>1000000)? info->fps:(info->resx*info->resy*info->fps);
	printk("dvo %d,int %d,hdmi %d\n",dvo_vout_mode,int_vout_mode,hdmi_vout_mode);
	printk(KERN_ALERT "vo_init_wmt (%s %dx%d,%d)\n",(vo)? vo->name:"NO",info->resx,info->resy,info->fps);
	return 0;
}

int vout_exit(void)
{
	vout_set_blank(VOUT_MODE_ALL,1);
	govrh_set_MIF_enable(VPP_FLAG_DISABLE);
	return 0;
}
