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
4F, 531, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C.
--*/

#include <config.h>
#include <common.h>
#include <command.h>
#include <version.h>
#include <stdarg.h>
#include <linux/types.h>
#include <devices.h>
#include <linux/stddef.h>
#include <asm/byteorder.h>

#include "vpp.h"
#include "vout.h"
#include "minivgui.h"

#define WMT_DISPLAY

#include "../../board/wmt/include/wmt_clk.h"

// extern int auto_pll_divisor(enum dev_id dev, enum clk_cmd cmd, int unit, int freq);

#include "hw_devices.h"
#include "wmt_display.h"
#include "vout.h"

struct wmt_display_param_t g_display_param;
struct wmt_display_param_t g_display_param2;
struct wmt_pwm_setting_t g_pwm_setting;
vpp_timing_t g_display_tmr;
struct gpio_operation_t g_lcd_pw_pin;
int g_display_vaild;
unsigned int g_fb_phy;
unsigned int g_img_phy;
int g_logo_x;
int g_logo_y;
int g_direct_path;

int vpp_dac_sense_enable;

extern void vpp_init(vout_info_t *info);
extern int vpp_config(vout_info_t *info);

struct fb_var_screeninfo vfb_var = {
	.xres           = 800,
	.yres           = 480,
	.xres_virtual   = 800,
	.yres_virtual   = 480,
	.bits_per_pixel = 16,
	.red            = {11, 5, 0},
	.green          = {5, 6, 0},
	.blue           = {0, 5, 0},
	.transp         = {0, 0, 0},
	//.activate       = FB_ACTIVATE_NOW | FB_ACTIVATE_FORCE,
	.height         = -1,
	.width          = -1,
	.pixclock       = (VPP_HD_DISP_RESX*VPP_HD_DISP_RESY*VPP_HD_DISP_FPS),
	.left_margin    = 40,
	.right_margin   = 24,
	.upper_margin   = 32,
	.lower_margin   = 11,
	.hsync_len      = 96,
	.vsync_len      = 2,
	//.vmode          = FB_VMODE_NONINTERLACED
};

char *video_str[] = {
	"SDA", "SDD" , "LCD", "DVI" , "HDMI",
	"DVO2HDMI", "LVDS", "VGA"
};
#if 0
/*--------------------------------------- BOOT LOGO ---------------------------------------*/
static int vo_boot_compatible(int arg)
{
	vout_mode_t mode;

	printf("vo_boot_compatible(%d)\n",arg);
	mode = (vout_mode_t) arg;
	switch(mode){
		case VOUT_VGA:
		case VOUT_DVI:
		case VOUT_DVO2HDMI:
		case VOUT_LCD:
		case VOUT_SD_ANALOG:
		case VOUT_SD_DIGITAL:
			return 1;
		default:
			break;
	}
	return 0;
}

static int vo_boot_visible(int arg)
{
	printf("vo_boot_visible(%d)\n",arg);

	return 0;
}

static int vo_boot_config(int arg)
{
	printf("vo_boot_config\n");
	return 0;
}

static int vo_boot_init(int arg)
{
	vout_t *vo;

	vo = vout_get_info(VOUT_BOOT);

	printf("vo_boot_init(%d)\n",arg);

	return 0;
}

static int vo_boot_uninit(int arg)
{
	printf("vo_boot_uninit(%d)\n",arg);
	return 0;
}

static int vo_boot_suspend(int arg)
{
	return 0;
}

static int vo_boot_resume(int arg)
{
	printf("vo_boot_resume(%d)\n",arg);
	return 0;
}

vout_t vo_boot_parm;
static int vo_boot_chkplug(int arg)
{
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
#endif
int lcd_poweron_ctl(char *name)
{
	char *p;
    char * endp;
    int i = 0;
    ulong   addr;
    ulong   val, org;
    char    op;


	p = getenv(name);
    if (!p) {
        printf("OEM LCD power-on control is not set\n");
        return 0;
    }

    while(1)
    {
        addr = simple_strtoul(p, &endp, 16);
        if( *endp == '\0')
            break;

        op = *endp;
        if ((op == ',') || ((op == ';'))) {
        	val = 0;
        	org = 1;
        	for (i = 0; i < 8; i++) {
        		val += ((addr&0x0f)*org);
        		addr >>= 4;
				org *= 10;
        	}
        	//printf("delay %d usec .....\n", val);
        	udelay(val);
        	goto nextcheck;
        }
        if( endp[1] == '~') {
            val = simple_strtoul(endp+2, &endp, 16);
            val = ~val;
        } else
            val = simple_strtoul(endp+1, &endp, 16);
		
		if (addr&0x03) {
        	printf(" address not alginment to 32bit , address = 0x%x\n",addr);
        	goto nextcheck;
    	}		
        //printf("  reg op: 0x%X %c 0x%X\n", addr, op, val);
		org = REG32_VAL(addr);
        switch(op)
        {
            case '|': org |= val; break;
            case '=': org = val ; break;
            case '&': org &= val; break;
            default:
                printf("Error, Unknown operator %c\n", op);
                break;
        }
        REG32_VAL(addr) = org;
nextcheck:
        if(*endp == '\0')
            break;
        p = endp + 1;
    }


    return 0;
}

#if 0
static int wmt_lcd_check(void)
{
	// op1 : lcd id , op2 : bpp
	if (g_display_param.op1 >= LCD_PANEL_MAX) {
		printf("error : LCD ID is %d , only support 0 ~ %d\n",g_display_param.op1,LCD_PANEL_MAX-1);
		goto lcdfailed;
	}

	switch( g_display_param.op2 ) {
		case 15:
		case 16:
		case 18:
		case 24:
			break;		
		default:
			printf("Warning  : unknow bpp %d , set to default 24bpp\n",g_display_param.op2);
			g_display_param.op2 = 24;
			break;
	}

	// if lcd id == oem ,get tmr from env
	if (g_display_param.op1 == LCD_WMT_OEM) {
		if ((g_display_vaild&TMRFROMENV) == 0) {
			if (parse_display_tmr(ENV_DISPLAY_TMR))
				goto lcdfailed;
		}
		vfb_var.xres = g_display_tmr.hpixel;
		vfb_var.yres = g_display_tmr.vpixel;
		vfb_var.xres_virtual = g_display_tmr.hpixel;
		vfb_var.yres_virtual = g_display_tmr.vpixel;
		vfb_var.pixclock = g_display_tmr.pixel_clock;
	} else {
		if ((g_display_param.resx == 0) || (g_display_param.resy == 0) || (g_display_param.fps_pixclk == 0))
			goto lcdfailed;
	}
	
	// get try pwm setting from env , run default if not set
	parse_pwm_params(ENV_DISPLAY_PWM, NULL);

	parse_gpio_operation(ENV_LCD_POWER);
	return 0;
lcdfailed:
	g_display_vaild = 0;
	return -1;	
}
#endif

static int alignmentMemory(unsigned int *memtotal)
{
	unsigned int memmax = 512; //should get from MC4

	if (*memtotal > 256) {         /* 512M */
		*memtotal = 512;
	} else if (*memtotal > 128) {  /* 256M */
		*memtotal = 256;
	} else if (*memtotal > 64) {   /* 128M */
		*memtotal = 128;
	} else if (*memtotal > 32) {   /* 64M */
		*memtotal = 64;
	} else if (*memtotal > 16) {   /* 32M */
		*memtotal = 32;
	} else {
		*memtotal = 0;
	}
	if ((*memtotal == 0) || (*memtotal > memmax)) {
		printf("error : memtotal = %dMByte , can not calculate framebuffer address\n", memtotal);
		return -1;
	}
	return 0;
}


static int getwmtfb(unsigned int memtotal)
{
	unsigned int tmp, ofs;

	tmp = *(unsigned int *)0xd8120000;
	tmp >>= 16;

	//wm8710 : pmem has been disable
	if (tmp == 0x3445) {
		g_fb_phy = memtotal;
		return 0;
	}

	switch(tmp) {
	case 0x3429:
	case 0x3465:
		ofs = 8;
		break;
	case 0x3451:
		ofs = 12;
		break;
	default: 
		printf("error : unknow chip %x\n", tmp);
		return -1;
	}

	tmp = memtotal;
	if (alignmentMemory(&tmp))
		return -1;
	g_fb_phy = tmp - ofs;
	return 0;
}

static int wmt_init_check(void)
{
	char *tmp;
	unsigned int memtotal = 0;
	long ps[6];

	g_fb_phy = 0;
	if ((tmp = getenv (ENV_DIRECTFB)) != NULL) {
		g_fb_phy = (unsigned int)simple_strtoul (tmp, NULL, 16);
	}

	if ((tmp = getenv (ENV_DIRECTPATH)) != NULL) {
		g_direct_path = (unsigned int)simple_strtoul (tmp, NULL, 10);
	} else {
		g_direct_path = 0;
	}

	if (g_fb_phy == 0)	{
		// we must check "memtotal" because we get framebuffer from it
		if ((tmp = getenv (ENV_MEMTOTAL)) != NULL) {
			memtotal = (unsigned int)simple_strtoul (tmp, NULL, 10);
		} else {
			printf("error : we need %s to calculate framebuffer address\n", ENV_MEMTOTAL);
			return -1;
		}
		if (g_direct_path != 0) {
			g_fb_phy = memtotal;
		} else {
			if (getwmtfb(memtotal))
				return -1;
		}
		g_fb_phy <<= 20;
	}

	if ((tmp = getenv (ENV_IMGADDR)) != NULL) {
		g_img_phy = (unsigned int)simple_strtoul (tmp, NULL, 16);
	} else {
		printf("%s is not found , command (display show) is invaild\n", ENV_IMGADDR);
		g_img_phy = 0xFFFF;
	}

	return 0;

checkfailed :
	g_display_vaild = 0;
	return -1;	
}

int uboot_vpp_alloc_framebuffer(unsigned int resx,unsigned int resy)
{
	vdo_framebuf_t *fb;
	unsigned int y_size, fb_size, fb_size1;
	unsigned int colfmt;
/*
	if( g_vpp.mb[0] ){
		printf("vpp_alloc_framebuffer : alreay alloc \n");
		return 0;
	}
*/
#ifdef CONFIG_VPP_FIX_BUFFER
	if( resx % 64 ){
		resx += (64 - (resx % 64));
	}
	y_size = resx*resy;
	fb_size1 = y_size*(vfb_var.bits_per_pixel>>3);
	resx = VPP_HD_MAX_RESX;
	resy = VPP_HD_MAX_RESY;
	colfmt = VDO_COL_FMT_YUV422H;
#else
	colfmt = g_vpp.govr->fb_p->fb.col_fmt;
#endif

	if( resx % 64 ){
		resx += (64 - (resx % 64));
	}
	y_size = resx * resy;

#ifdef CONFIG_VPP_FIX_BUFFER
	fb_size = y_size*(vfb_var.bits_per_pixel>>3);
#else
	fb_size = (colfmt==VDO_COL_FMT_ARGB)? (4 * y_size):(3 * y_size);
#endif
	y_size = (colfmt==VDO_COL_FMT_ARGB)? (fb_size):(y_size);

	g_vpp.mb[0] = g_fb_phy;
	g_vpp.mb[1] = g_vpp.mb[0];

	fb = &g_vpp.govr->fb_p->fb;	
	fb->y_addr = g_vpp.mb[1];
	fb->c_addr = 0;

	fb->y_size = y_size;
	fb->c_size = fb_size - y_size;
	//fb->fb_w = resx;
	//fb->fb_h = resy;

	fb->fb_w = vfb_var.xres;
	fb->fb_h = vfb_var.yres;
//	if (g_direct_path) {
		fb->y_addr = g_fb_phy;
		fb->c_addr = g_fb_phy + y_size;
	
#ifdef CONFIG_VPP_FIX_BUFFER
		memset((char *)g_fb_phy, 0, (fb_size1*2));
#else
		memset((char *)g_fb_phy, 0, (resx*resy*4));
#endif

//	}
	return 0;
}

void vpp_mod_init(void)
{
	//printf("[VPP] vpp_mod_init\n");
//	vppm_mod_init();

#ifdef WMT_FTBLK_GOVRH	
	govrh_mod_init();
#endif


#ifdef WMT_FTBLK_LCDC
	lcdc_mod_init();
#endif

#ifdef WMT_FTBLK_GOVW
	govw_mod_init();
#endif
/*
#ifdef WMT_FTBLK_SCL
	scl_mod_init();
#endif
*/
#ifdef WMT_FTBLK_GOVM
	govm_mod_init();
#endif
/*
#ifdef WMT_FTBLK_VPU
	vpu_mod_init();
#endif
*/

#ifdef WMT_FTBLK_DISP	
	disp_mod_init();
#endif

}

void vpp_device_init(void)
{
	auto_pll_divisor(DEV_PWM,CLK_ENABLE,0,0);
	lcd_module_init();
	lcd_oem_init();
	lcd_lw700at9003_init();
	lcd_at070tn83_init();
	lcd_a080sn01_init();
	lcd_ek08009_init();
	lcd_HSD101PFW2_init();
	
	vt1632_module_init();
	sil902x_module_init();
}

int wmt_graphic_init(void)
{
	lcd_parm_t *p_oem_lcd;
	vout_info_t info;

	g_direct_path = 0;
	if (wmt_init_check())
		return -1;

	if (g_direct_path) {
		vfb_var.bits_per_pixel = 32;
	}

	vpp_mod_init();
	vpp_device_init();
	
	g_vpp.govr = (vpp_mod_base_t*) p_govrh;
	g_vpp.govrh_preinit = 0;
	g_vpp.govr->fb_p->fb.col_fmt = VDO_COL_FMT_RGB_565;
	g_vpp.alloc_framebuf = uboot_vpp_alloc_framebuffer;

	info.resx = 1024;
	info.resy = 768;
	info.fps = 60;
	vpp_init(&info);

	g_display_param.vout = vpp_vo_boot_arg[0];

	if (g_display_param.vout == VPP_VOUT_LCD) {
		parse_pwm_params(ENV_DISPLAY_PWM, NULL);
		parse_gpio_operation(ENV_LCD_POWER);
		
		if ((g_display_vaild&PWMDEFMASK) == PWMDEFTP) {
				lcd_blt_set_pwm(g_pwm_setting.pwm_no, g_pwm_setting.duty, g_pwm_setting.period);
		} else {
			// fan : may need to check PWM power ..
			pwm_set_period(g_pwm_setting.pwm_no, g_pwm_setting.period-1);
			pwm_set_duty(g_pwm_setting.pwm_no, g_pwm_setting.duty-1);
			pwm_set_control(g_pwm_setting.pwm_no, (g_pwm_setting.duty-1)? 0x34:0x8);
			pwm_set_gpio(g_pwm_setting.pwm_no, g_pwm_setting.duty-1);
			pwm_set_scalar(g_pwm_setting.pwm_no, g_pwm_setting.scalar-1);
		}
	}

	vfb_var.xres = info.resx;
	vfb_var.yres = info.resy;

	vpp_config(&info);

	g_display_vaild |= DISPLAY_ENABLE;

	return 0;
}


#undef WMT_DISPLAY

