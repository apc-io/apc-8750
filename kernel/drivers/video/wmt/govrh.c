/*++ 
 * linux/drivers/video/wmt/govrh.c
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

#define GOVRH_C
// #define DEBUG

#include "govrh.h"

#ifdef WMT_FTBLK_GOVRH
void govrh_reg_dump(void)
{
	U32 addr,reg;

	DPRINT("========== GOVRH register dump ==========\n");
	vpp_reg_dump(REG_GOVRH_BASE1_BEGIN,REG_GOVRH_BASE1_END-REG_GOVRH_BASE1_BEGIN);
	vpp_reg_dump(REG_GOVRH_BASE2_BEGIN,REG_GOVRH_BASE2_END-REG_GOVRH_BASE2_BEGIN);

	addr = GOVRH_BASE2_ADDR+0x0;
	reg = vppif_reg32_in(addr);
	DPRINT("---------- GOVRH TG1 ----------\n");	
	DPRINT("TG enable %d, Twin mode %d\n",reg & BIT0,reg & BIT8);
	DPRINT("DVO clk %d,Read cyc %d\n",vpp_get_base_clock(VPP_MOD_GOVRH),vppif_reg32_read(GOVRH_READ_CYC));
	DPRINT("H total %d, Sync %d, beg %d, end %d\n",vppif_reg32_read(GOVRH_H_ALLPXL),vppif_reg32_read(GOVRH_HDMI_HSYNW),
		vppif_reg32_read(GOVRH_ACTPX_BG),vppif_reg32_read(GOVRH_ACTPX_END));
	DPRINT("V total %d, Sync %d, beg %d, end %d\n",vppif_reg32_read(GOVRH_V_ALLLN),vppif_reg32_read(GOVRH_HDMI_VBISW),
		vppif_reg32_read(GOVRH_ACTLN_BG),vppif_reg32_read(GOVRH_ACTLN_END));
	DPRINT("VBIE %d,PVBI %d\n",vppif_reg32_read(GOVRH_VBIE_LINE),vppif_reg32_read(GOVRH_PVBI_LINE));
	DPRINT("---------- GOVRH TG2 ----------\n");
	DPRINT("H total %d, Sync %d, beg %d, end %d\n",vppif_reg32_read(GOVRH_H_ALLPXL2),vppif_reg32_read(GOVRH_HDMI_HSYNW2),
		vppif_reg32_read(GOVRH_ACTPX_BG2),vppif_reg32_read(GOVRH_ACTPX_END2));
	DPRINT("V total %d, Sync %d, beg %d, end %d\n",vppif_reg32_read(GOVRH_V_ALLLN2),vppif_reg32_read(GOVRH_HDMI_VBISW2),
		vppif_reg32_read(GOVRH_ACTLN_BG2),vppif_reg32_read(GOVRH_ACTLN_END2));
	DPRINT("VBIE %d,PVBI %d\n",vppif_reg32_read(GOVRH_VBIE_LINE2),vppif_reg32_read(GOVRH_PVBI_LINE2));
	DPRINT("-------------------------------\n");	
	DPRINT("DVO enable %d, polarity %s,data width %d bits\n",vppif_reg32_read(GOVRH_DVO_ENABLE),
		(vppif_reg32_read(GOVRH_DVO_SYNC_POLAR))? "Low":"High",(vppif_reg32_read(GOVRH_DVO_OUTWIDTH))? 12:24);
	DPRINT("DVO color format %s\n",vpp_colfmt_str[govrh_get_dvo_color_format()]);
	DPRINT("color bar enable %d,mode %d,inv %d\n",vppif_reg32_read(GOVRH_CB_ENABLE),vppif_reg32_read(GOVRH_CB_MODE),
		vppif_reg32_read(GOVRH_CB_INVERSION));
#ifdef WMT_FTBLK_GOVRH_VGA
	DPRINT("VGA HSync %d,VSync %d\n",vppif_reg32_read(GOVRH_VGA_HSYNW),vppif_reg32_read(GOVRH_VGA_VSYNW));
	DPRINT("VGA H polar %s,V polar %s\n",(vppif_reg32_read(GOVRH_VGA_HSYN_POLAR))? "Low":"High",
		(vppif_reg32_read(GOVRH_VGA_VSYN_POLAR))? "Low":"High");	
#endif
	
	DPRINT("Contrast 0x%x,Brightness 0x%x\n",vppif_reg32_in(REG_GOVRH_CONTRAST),vppif_reg32_in(REG_GOVRH_BRIGHTNESS));
	DPRINT("CSC mode %s\n",(vppif_reg32_read(GOVRH_CSC_MOD))?"YUV2RGB":"RGB2YUV");
#ifdef WMT_FTBLK_GOVRH_VGA
	DPRINT("VGA CSC %d\n",vppif_reg32_read(GOVRH_VGA_YUV2RGB_ENABLE));
#endif
	DPRINT("DVO CSC %d\n",vppif_reg32_read(GOVRH_DVO_YUV2RGB_ENABLE));
	DPRINT("CSC enable DVO %d,DISP %d,LVDS %d,HDMI %d\n",vppif_reg32_read(GOVRH_DVO_YUV2RGB_ENABLE),
		vppif_reg32_read(GOVRH_DISP_CSC_ENABLE),vppif_reg32_read(GOVRH_LVDS_CSC_ENABLE),vppif_reg32_read(GOVRH_HDMI_CSC_ENABLE));

	DPRINT("H264 %d\n",vppif_reg32_read(GOVRH_H264_INPUT_ENABLE));
	DPRINT("source format %s\n",(vppif_reg32_read(GOVRH_INFMT))?"field":"frame");
	DPRINT("Y addr 0x%x, C addr 0x%x\n",vppif_reg32_in(REG_GOVRH_YSA),vppif_reg32_in(REG_GOVRH_CSA));
	DPRINT("Active width %d,frame buf width %d\n",vppif_reg32_read(GOVRH_AWIDTH),vppif_reg32_read(GOVRH_FWIDTH));
	DPRINT("V crop %d, H crop %d\n",vppif_reg32_read(GOVRH_VCROP),vppif_reg32_read(GOVRH_HCROP));
	DPRINT("colr format %s\n",vpp_colfmt_str[govrh_get_color_format()]);
#ifdef WMT_FTBLK_GOVRH_DAC
	DPRINT("DAC PwrDn %d\n",vppif_reg32_read(GOVRH_DAC_PWRDN));
#endif

#ifdef WMT_FTBLK_GOVRH_CURSOR
	DPRINT("---------- CURSOR -------------\n");
	DPRINT("enable %d,field %d\n",vppif_reg32_read(GOVRH_CUR_ENABLE),vppif_reg32_read(GOVRH_CUR_OUT_FIELD));
	DPRINT("width %d,fb width %d\n",vppif_reg32_read(GOVRH_CUR_WIDTH),vppif_reg32_read(GOVRH_CUR_FB_WIDTH));
	DPRINT("crop(%d,%d)\n",vppif_reg32_read(GOVRH_CUR_HCROP),vppif_reg32_read(GOVRH_CUR_VCROP));
	DPRINT("coord H(%d,%d),V(%d,%d)\n",vppif_reg32_read(GOVRH_CUR_H_START),vppif_reg32_read(GOVRH_CUR_H_END)
		,vppif_reg32_read(GOVRH_CUR_V_START),vppif_reg32_read(GOVRH_CUR_V_END));
	DPRINT("color key enable 0x%x,color key 0x%x,alpha enable %d\n",vppif_reg32_read(GOVRH_CUR_COLKEY_ENABLE)
		,vppif_reg32_read(GOVRH_CUR_COLKEY),vppif_reg32_read(GOVRH_CUR_ALPHA_ENABLE));	
#endif
}


vpp_flag_t govrh_set_tg_enable(vpp_flag_t enable)
{
	switch (enable) {
	case VPP_FLAG_ENABLE:
		vppif_reg32_write(GOVRH_TG_ENABLE, enable);
		break;
	case VPP_FLAG_DISABLE:
		if( vppif_reg32_read(GOVRH_TG_ENABLE) ){
			vppif_reg32_write(GOVRH_TG_ENABLE, enable);
			g_vpp.govrh_field = VPP_FIELD_BOTTOM;
		}
		break;
	default:
		return VPP_FLAG_ERROR;
	}
	return VPP_FLAG_SUCCESS;
}

unsigned int govrh_set_clock(unsigned int pixel_clock)
{
	int pmc_clk = 0;

//	DPRINT("[GOVRH] set clock %d\n",pixel_clock);
	if( pixel_clock == 27000000 ){
		vppif_reg32_out(PM_CTRL_BASE_ADDR+0x208,0x02350001);
		while ((*(volatile unsigned int *)(PM_CTRL_BASE_ADDR+0x18))&0x3F0038);
		vppif_reg32_out(PM_CTRL_BASE_ADDR+0x36c,0x19);
		pmc_clk = pixel_clock;
	}
	else {
		pmc_clk = auto_pll_divisor(DEV_DVO,SET_PLLDIV,0,pixel_clock);
	}
//	DPRINT("[GOVRH] set clock %d --> %d\n",pixel_clock,pmc_clk);
#ifdef CONFIG_WMT_HDMI
	g_vpp.hdmi_pixel_clock = pmc_clk;
#endif
	return 0;
}

void govrh_set_tg1(vpp_clock_t *timing)
{
	vppif_reg32_write(GOVRH_TG_MODE,0);
	vppif_reg32_write(GOVRH_OUTFMT,0);

	vppif_reg32_write(GOVRH_READ_CYC, timing->read_cycle);

	vppif_reg32_write(GOVRH_ACTPX_BG, timing->begin_pixel_of_active);
	vppif_reg32_write(GOVRH_ACTPX_END, timing->end_pixel_of_active);
	vppif_reg32_write(GOVRH_H_ALLPXL, timing->total_pixel_of_line);

	vppif_reg32_write(GOVRH_ACTLN_BG, timing->begin_line_of_active);
	vppif_reg32_write(GOVRH_ACTLN_END, timing->end_line_of_active);
	vppif_reg32_write(GOVRH_V_ALLLN, timing->total_line_of_frame);

	vppif_reg32_write(GOVRH_VBIE_LINE, timing->line_number_between_VBIS_VBIE);
	vppif_reg32_write(GOVRH_PVBI_LINE, timing->line_number_between_PVBI_VBIS);

	vppif_reg32_write(GOVRH_HDMI_HSYNW,timing->hsync);
	vppif_reg32_write(GOVRH_HDMI_VBISW,timing->vsync);

	DBGMSG("[GOVRH] tg1 H beg %d,end %d,total %d\n",
		timing->begin_pixel_of_active,timing->end_pixel_of_active,timing->total_pixel_of_line);
	DBGMSG("[GOVRH] tg1 V beg %d,end %d,total %d\n",
		timing->begin_line_of_active,timing->end_line_of_active,timing->total_line_of_frame);
}

void govrh_set_tg2(vpp_clock_t *timing)
{
	vppif_reg32_write(GOVRH_TG_MODE,1);
	vppif_reg32_write(GOVRH_OUTFMT,1);

	vppif_reg32_write(GOVRH_ACTPX_BG2, timing->begin_pixel_of_active);
	vppif_reg32_write(GOVRH_ACTPX_END2, timing->end_pixel_of_active);
	vppif_reg32_write(GOVRH_H_ALLPXL2, timing->total_pixel_of_line);

	vppif_reg32_write(GOVRH_ACTLN_BG2, timing->begin_line_of_active);
	vppif_reg32_write(GOVRH_ACTLN_END2, timing->end_line_of_active);
	vppif_reg32_write(GOVRH_V_ALLLN2, timing->total_line_of_frame);

	vppif_reg32_write(GOVRH_VBIE_LINE2, timing->line_number_between_VBIS_VBIE);
	vppif_reg32_write(GOVRH_PVBI_LINE2, timing->line_number_between_PVBI_VBIS);

	vppif_reg32_write(GOVRH_HDMI_HSYNW2,timing->hsync);
	vppif_reg32_write(GOVRH_HDMI_VBISW2,timing->vsync + 1);
	
	DBGMSG("[GOVRH] tg2 H beg %d,end %d,total %d\n",
		timing->begin_pixel_of_active,timing->end_pixel_of_active,timing->total_pixel_of_line);
	DBGMSG("[GOVRH] tg2 V beg %d,end %d,total %d\n",
		timing->begin_line_of_active,timing->end_line_of_active,timing->total_line_of_frame);
}

void govrh_get_tg(vpp_clock_t * tmr)
{
	tmr->read_cycle = vppif_reg32_read(GOVRH_READ_CYC);
	tmr->total_pixel_of_line = vppif_reg32_read(GOVRH_H_ALLPXL);
	tmr->begin_pixel_of_active = vppif_reg32_read(GOVRH_ACTPX_BG);
	tmr->end_pixel_of_active = vppif_reg32_read(GOVRH_ACTPX_END);

	tmr->total_line_of_frame = vppif_reg32_read(GOVRH_V_ALLLN);
	tmr->begin_line_of_active = vppif_reg32_read(GOVRH_ACTLN_BG);
	tmr->end_line_of_active = vppif_reg32_read(GOVRH_ACTLN_END);

	tmr->line_number_between_VBIS_VBIE = vppif_reg32_read(GOVRH_VBIE_LINE);
	tmr->line_number_between_PVBI_VBIS = vppif_reg32_read(GOVRH_PVBI_LINE);

	tmr->hsync = vppif_reg32_read(GOVRH_HDMI_HSYNW);
	tmr->vsync = vppif_reg32_read(GOVRH_HDMI_VBISW) - 1;

#ifdef PATCH_GOVRH_VSYNC_OVERLAP
	if( p_govrh->h_pixel ){
		tmr->total_line_of_frame *= 2;
		tmr->begin_line_of_active *= 2 ;
		tmr->end_line_of_active *= 2;
	}
#endif

	if( vppif_reg32_read(GOVRH_TG_MODE) ){
		tmr->total_line_of_frame += vppif_reg32_read(GOVRH_V_ALLLN2);
		tmr->begin_line_of_active += vppif_reg32_read(GOVRH_ACTLN_BG2);
		tmr->end_line_of_active += vppif_reg32_read(GOVRH_ACTLN_END2);
	}
}

vpp_int_err_t govrh_get_int_status(void)
{
	vpp_int_err_t int_sts;

	int_sts = 0;
	if (vppif_reg32_in(REG_GOVRH_INT) & 0x200) {
		int_sts |= VPP_INT_ERR_GOVRH_MIF;
	}
	return int_sts;
}

void govrh_clean_int_status(vpp_int_err_t int_sts)
{
	if (int_sts & VPP_INT_ERR_GOVRH_MIF) {
		vppif_reg16_out(REG_GOVRH_INT+0x2,0x2);
	}
}

void govrh_set_int_enable(vpp_flag_t enable, vpp_int_err_t int_bit)
{
	//clean status first before enable/disable interrupt
	govrh_clean_int_status(int_bit);

	if (int_bit & VPP_INT_ERR_GOVRH_MIF) {
		vppif_reg32_write(GOVRH_INT_MEM_ENABLE, enable);
	}
}

void govrh_set_dvo_enable(vpp_flag_t enable)
{
	vppif_reg32_write(GOVRH_DVO_ENABLE, enable);
	if( enable ){
		vppif_reg32_out(GPIO_BASE_ADDR+0x44,0x0);	// Disable DVO/CCIR656 GPIO
	}
	else {
		vppif_reg32_out(GPIO_BASE_ADDR+0x44,0xFFFFFFFF);	// Enable GPIO
		vppif_reg32_out(GPIO_BASE_ADDR+0x84,0x0);			// GPIO output enable
		vppif_reg32_out(GPIO_BASE_ADDR+0x484,0x0);			// GPIO pull disable
		vppif_reg32_out(GPIO_BASE_ADDR+0x4C4,0x0);			// 1: pull up, 0: pull down
	}
}

void govrh_set_dvo_sync_polar(vpp_flag_t hsync,vpp_flag_t vsync)
{
	vppif_reg32_write(GOVRH_DVO_SYNC_POLAR,hsync);
#ifdef GOVRH_DVO_VSYNC_POLAR	
	vppif_reg32_write(GOVRH_DVO_VSYNC_POLAR,vsync);
#endif
}

vdo_color_fmt govrh_get_dvo_color_format(void)
{
	if(	vppif_reg32_read(GOVRH_DVO_RGB) ){
		return VDO_COL_FMT_ARGB;
	}
	if( vppif_reg32_read(GOVRH_DVO_YUV422) ){
		return VDO_COL_FMT_YUV422H;
	}
	return VDO_COL_FMT_YUV444;
}

void govrh_set_dvo_color_format(vdo_color_fmt fmt)
{
	switch( fmt ){
		case VDO_COL_FMT_ARGB:
			vppif_reg32_write(GOVRH_DVO_RGB,0x1);
			vppif_reg32_write(GOVRH_DVO_YUV422,0x0);
			break;
		case VDO_COL_FMT_YUV422H:
			vppif_reg32_write(GOVRH_DVO_RGB,0x0);
			vppif_reg32_write(GOVRH_DVO_YUV422,0x1);
			break;
		case VDO_COL_FMT_YUV444:			
		default:
			vppif_reg32_write(GOVRH_DVO_RGB,0x0);
			vppif_reg32_write(GOVRH_DVO_YUV422,0x0);
			break;
	}
}

vpp_flag_t govrh_set_dvo_outdatw(vpp_datawidht_t width)
{
	switch (width) {
	case VPP_DATAWIDHT_12:
		vppif_reg32_write(GOVRH_DVO_OUTWIDTH, 1);
		break;
	case VPP_DATAWIDHT_24:
		vppif_reg32_write(GOVRH_DVO_OUTWIDTH, 0);
		break;
	default:
		return VPP_FLAG_ERROR;
	}
	return VPP_FLAG_SUCCESS;
}

void govrh_set_dvo_clock_delay(int inverse,int delay)
{
	vppif_reg32_write(GOVRH_DVO_CLK_INV,inverse);
	vppif_reg32_write(GOVRH_DVO_CLK_DLY,delay);
}

void govrh_set_colorbar(vpp_flag_t enable,int mode,int inv)
{
	vppif_reg32_write(GOVRH_CB_ENABLE, enable);
	vppif_reg32_write(GOVRH_CB_MODE, mode);
	vppif_reg32_write(GOVRH_CB_INVERSION, inv);
}

#ifdef WMT_FTBLK_GOVRH_VGA
void govrh_set_vga_enable(vpp_flag_t enable)
{
	if( enable ){
		vppif_reg8_out(GPIO_BASE_ADDR+0x49,0x0);		// Disable VGA GPIO		
	}
	else {
		vppif_reg8_out(GPIO_BASE_ADDR+0x49,0xFF);		// Enable GPIO
		vppif_reg8_out(GPIO_BASE_ADDR+0x89,0x0);		// GPIO output enable
		vppif_reg8_out(GPIO_BASE_ADDR+0x489,0xFF);		// GPIO pull enable
		vppif_reg8_out(GPIO_BASE_ADDR+0x4C9,0x0);		// 1: pull up, 0: pull down
	}
}

void govrh_set_VGA_sync(U32 hsync, U32 vsync)
{
	vppif_reg32_write(GOVRH_VGA_HSYNW, hsync);
	vppif_reg32_write(GOVRH_VGA_VSYNW, vsync);
}

void govrh_set_VGA_sync_polar(U32 hsync, U32 vsync)
{
	vppif_reg32_write(GOVRH_VGA_HSYN_POLAR, hsync);
	vppif_reg32_write(GOVRH_VGA_VSYN_POLAR, vsync);
}
#endif

#if 0
int govrh_monitor_DISP_DAC_sense(void)
{
	unsigned int pwrdn;

	vppif_reg32_write(GOVRH_DAC_TEST_A,0x55);
	vppif_reg32_write(GOVRH_DAC_TEST_B,0x0);
	vppif_reg32_write(GOVRH_DAC_TEST_C,0x0);
	vppif_reg32_write(GOVRH_DAC_TEST_ENABLE,0x1);

	pwrdn = (vppif_reg32_in(REG_GOVRH_DAC_CON))?0x00:0x01;
	vppif_reg32_write(GOVRH_DAC_TEST_ENABLE,0x0);	
}
#endif

#ifdef WMT_FTBLK_GOVRH_DAC
#ifdef WMT_FTBLK_GOVRH_DAC_THREE
void govrh_DAC_set_sense_sel(int tv)
{
	vppif_reg32_write(GOVRH_DAC_MODE,(tv)? 1:0);
//	vppif_reg32_write(GOVRH_DAC_NOR_ENABLE,(tv)? 0x7:0);	// 1:Normail, 0:LP
	vppif_reg32_write(GOVRH_DAC_DAF,0x4);
	vppif_reg32_write(GOVRH_TEST_MODE,(tv)? 0xD:0x1);		// bit4=1 sda plug sense fail
}

void govrh_DAC_set_sense_value(int no,unsigned int lp_val,unsigned int normal_val)
{
	vppif_reg32_write(REG_GOVRH_DAC_LP_SENSE_VAL,0x3FF << (10*no), 10*no, lp_val);
	vppif_reg32_write(REG_GOVRH_DAC_VAL,0x3FF << (10*no), 10*no, normal_val);
}

int govrh_DAC_get_connect(int no)
{
	unsigned int reg;

	reg = vppif_reg32_in(REG_GOVRH_DAC_CON);
	return (reg & (0x4 >> no))? 1:0;
}

void govrh_DAC_set_pwrdn(vpp_flag_t enable)
{
	vppif_reg32_write(GOVRH_DAC_PWRDN, enable);
}

unsigned int govrh_DAC_monitor_sense(void)
{
	unsigned int sense;

	vppif_reg32_write(GOVRH_DAC_MANUAL_SENSE, 0x01);
	vppif_reg32_write(GOVRH_DAC_SCD_ENABLE, 0x00);
	udelay(1);
	sense = vppif_reg32_in(REG_GOVRH_DAC_CON);
	vppif_reg32_write(GOVRH_DAC_SCD_ENABLE, 0x01);
	vppif_reg32_write(GOVRH_DAC_MANUAL_SENSE, 0x00);
//	DPRINT("[GOVRH] DAC sense 0x%x\n",sense);
	return sense;
}
#else
void govrh_set_DAC_pwrdn(vpp_flag_t enable)
{
	vppif_reg32_write(GOVRH_DAC_PWRDN, enable);
}

int govrh_monitor_DAC_sense(void)
{
	static unsigned int govrh_vbis_cnt = 0;
//	unsigned int i;
	unsigned int pwrdn;
	static unsigned int pre_pwrdn;

	govrh_vbis_cnt++;
	if( (govrh_vbis_cnt % p_govrh->vga_dac_sense_cnt) != 0 ){
		return -1;
	}

#if 1
	if( vppif_reg32_read(GOVRH_DAC_PWRDN) == 1 ){
		vppif_reg32_write(GOVRH_DAC_PWRDN, 0);	/* DAC power on */
		govrh_vbis_cnt--;
		return -1;
	}
#else
	vppif_reg32_write(GOVRH_DAC_PWRDN, 0);	/* DAC power on */
	/* wait status stable */
	for (i = 0; i < vpp_dac_sense_nop; i++) {
		nop();
	}
#endif
	
	vppif_reg32_write(GOVRH_DAC_MANUAL_SENSE, 0x01);
	vppif_reg32_write(GOVRH_DAC_SCD_ENABLE, 0x00);

	pwrdn = (vppif_reg32_in(REG_GOVRH_DAC_CON))?0x00:0x01;
	vppif_reg32_write(GOVRH_DAC_SCD_ENABLE, 0x01);

	vppif_reg32_write(GOVRH_DAC_MANUAL_SENSE, 0x00);
	vppif_reg32_write(GOVRH_DAC_PWRDN, pwrdn);	/* set DAC power */

//	DPRINT("[GOVRH] DAC sense 0x%x\n",pwrdn);

	if( pre_pwrdn != pwrdn ){
		DPRINT("[GOVRH] VGA DAC %s\n",(pwrdn)?"DISCONNECT":"CONNECT");
		pre_pwrdn = pwrdn;
		govrh_set_vga_enable((pwrdn == 0));
	}
	return (pwrdn==0);
}
#endif
#endif

void govrh_set_contrast(int level)
{
	vppif_reg32_write(GOVRH_CONTRAST_YAF, level);
	vppif_reg32_write(GOVRH_CONTRAST_PBAF, level);
	vppif_reg32_write(GOVRH_CONTRAST_PRAF, level);
}

int govrh_get_contrast(void)
{
	return vppif_reg32_read(GOVRH_CONTRAST_YAF);
}

void govrh_set_brightness(int level)
{
	vppif_reg32_write(GOVRH_BRIGHTNESS_Y, level);
}

int govrh_get_brightness(void)
{
	return vppif_reg32_read(GOVRH_BRIGHTNESS_Y);
}

void govrh_set_MIF_enable(vpp_flag_t enable)
{
	vppif_reg32_write(GOVRH_MIF_ENABLE, enable);
}

vpp_flag_t govrh_set_data_format(vdo_color_fmt format)
{
	if (format < VDO_COL_FMT_ARGB) {
		vppif_reg32_write(GOVRH_RGB_MODE, 0);
	}

	switch (format) {
	case VDO_COL_FMT_YUV420:
		vppif_reg32_write(GOVRH_COLFMT, 1);
		vppif_reg32_write(GOVRH_COLFMT2, 0);
		break;
	case VDO_COL_FMT_YUV422H:
		vppif_reg32_write(GOVRH_COLFMT, 0);
		vppif_reg32_write(GOVRH_COLFMT2, 0);
		break;
	case VDO_COL_FMT_YUV444:
		vppif_reg32_write(GOVRH_COLFMT, 0);
		vppif_reg32_write(GOVRH_COLFMT2, 1);
		break;
	case VDO_COL_FMT_ARGB:
		vppif_reg32_write(GOVRH_RGB_MODE, 1);
		break;
	case VDO_COL_FMT_RGB_1555:
		vppif_reg32_write(GOVRH_RGB_MODE, 2);
		break;
	case VDO_COL_FMT_RGB_565:
		vppif_reg32_write(GOVRH_RGB_MODE, 3);
		break;
	default:
		DBGMSG("*E* check the parameter\n");
		return VPP_FLAG_ERROR;
	}
#ifdef WMT_FTBLK_GOVRH_CURSOR
	govrh_CUR_set_colfmt(format);
#endif
	return VPP_FLAG_SUCCESS;
}

vdo_color_fmt govrh_get_color_format(void)
{
	switch( vppif_reg32_read(GOVRH_RGB_MODE) ){
		case 1: return VDO_COL_FMT_ARGB;
		case 2: return VDO_COL_FMT_RGB_1555;
		case 3: return VDO_COL_FMT_RGB_565;
		default: break;
	}
	if( vppif_reg32_read(GOVRH_COLFMT2) ){
		return VDO_COL_FMT_YUV444;
	}
	if( vppif_reg32_read(GOVRH_COLFMT) ){
		return VDO_COL_FMT_YUV420;
	}
	return VDO_COL_FMT_YUV422H;
}

vpp_flag_t govrh_set_source_format(vpp_display_format_t format)
{
	switch (format) {
	case VPP_DISP_FMT_FRAME:
	case VPP_DISP_FMT_FIELD:
		vppif_reg32_write(GOVRH_INFMT, format);
		break;
	default:
		DBGMSG("*E* check the parameter\n");
		return VPP_FLAG_ERROR;
	}
	return VPP_FLAG_SUCCESS;
}

void govrh_set_output_format(vpp_display_format_t field)
{
	vppif_reg32_write(GOVRH_OUTFMT,field);
}

void govrh_set_fb_addr(U32 y_addr,U32 c_addr)
{
//	DBGMSG("y_addr:0x%08x, c_addr:0x%08x\n", (U32) y_addr,(U32) c_addr);

	vppif_reg32_out(REG_GOVRH_YSA,y_addr);
	vppif_reg32_out(REG_GOVRH_CSA,c_addr);
}

void govrh_get_fb_addr(U32 *y_addr, U32 *c_addr)
{
	*y_addr = vppif_reg32_in(REG_GOVRH_YSA);
	*c_addr = vppif_reg32_in(REG_GOVRH_CSA);
//	DBGMSG("y_addr:0x%08x, c_addr:0x%08x\n",(U32) *y_addr,(U32) *c_addr);
}

#define GOVRH_CURSOR_RIGHT_POS		6
void govrh_set_fb_info(U32 width, U32 act_width, U32 x_offset, U32 y_offset)
{
//	DBGMSG("set fb info fb_w %d,img_w %d,x %d,y %d\n",width,act_width,x_offset,y_offset);

	vppif_reg32_write(GOVRH_AWIDTH, act_width); // +3);
	vppif_reg32_write(GOVRH_FWIDTH, width);
	vppif_reg32_write(GOVRH_VCROP, x_offset);
	vppif_reg32_write(GOVRH_HCROP, y_offset);

#ifdef WMT_FTBLK_GOVRH_CURSOR
	// patch for cursor postion over when change resolution
	if( (vppif_reg32_read(GOVRH_ACTPX_END) - vppif_reg32_read(GOVRH_ACTPX_BG)) != vppif_reg32_read(GOVRH_AWIDTH)){
		if( vppif_reg32_read(GOVRH_CUR_H_END) >= (vppif_reg32_read(GOVRH_AWIDTH) - GOVRH_CURSOR_RIGHT_POS) ){
			int pos;

			pos = vppif_reg32_read(GOVRH_AWIDTH) - GOVRH_CURSOR_RIGHT_POS;
			vppif_reg32_write(GOVRH_CUR_H_END,pos);
			pos -= vppif_reg32_read(GOVRH_CUR_WIDTH);
			vppif_reg32_write(GOVRH_CUR_H_START,pos+1);

			p_cursor->posx = pos;
		}
	}
#endif
}

void govrh_get_fb_info(U32 * width, U32 * act_width, U32 * x_offset, U32 * y_offset)
{
	*act_width = vppif_reg32_read(GOVRH_AWIDTH); // -3;
	*width = vppif_reg32_read(GOVRH_FWIDTH);
	*x_offset = vppif_reg32_read(GOVRH_VCROP);
	*y_offset = vppif_reg32_read(GOVRH_HCROP);
}

void govrh_set_fifo_index(U32 index)
{
	vppif_reg32_write(GOVRH_FIFO_IND, index);
}

vpp_flag_t govrh_set_reg_level(vpp_reglevel_t level)
{
	switch (level) {
	case VPP_REG_LEVEL_1:
		vppif_reg32_write(GOVRH_REG_LEVEL, 0x0);
		break;
	case VPP_REG_LEVEL_2:
		vppif_reg32_write(GOVRH_REG_LEVEL, 0x1);
		break;
	default:
		DBGMSG("*E* check the parameter\n");
		return VPP_FLAG_ERROR;
	}
	return VPP_FLAG_SUCCESS;
}

void govrh_set_reg_update(vpp_flag_t enable)
{
	vppif_reg32_write(GOVRH_REG_UPDATE, enable);
}

void govrh_set_tg(vpp_clock_t *tmr,unsigned int pixel_clock)
{
	govrh_set_tg_enable(0);
	tmr->read_cycle = govrh_set_clock(pixel_clock);
	govrh_set_tg1(tmr);
	govrh_set_tg_enable(1);	
}

void govrh_set_timing(vpp_timing_t *timing)
{
//	int cnt;
	vpp_timing_t ptr;
	unsigned int clk_delay = 0;
	vpp_clock_t t1;

	ptr = *timing;
	if( ptr.option & VPP_OPT_HSCALE_UP ){
		ptr.hpixel *= 2;
		DBGMSG("[GOVRH] H scale up\n");
	}
	vpp_trans_timing(VPP_MOD_GOVRH,&ptr,&t1,1);
	t1.read_cycle = govrh_set_clock(ptr.pixel_clock);
	if( ptr.option & VPP_OPT_INTERLACE ){
		t1.total_line_of_frame += 1;
		t1.begin_line_of_active += 1;
		t1.end_line_of_active += 1;
	}
	DBGMSG("pix clk %d,rd cyc %d\n",ptr.pixel_clock,t1.read_cycle);

	govrh_set_tg_enable(0);
	govrh_set_tg1(&t1);
#ifndef PATCH_GOVRH_VSYNC_OVERLAP
//	cnt = t1.total_line_of_frame;
	if( ptr.option & VPP_OPT_INTERLACE ){
		vpp_clock_t t2;
		
		//ptr = *(timing+1);
		ptr = *timing;
		ptr.vbp += 1;
		if( ptr.option & VPP_OPT_HSCALE_UP ){	// 480i & 576i repeat 2 pixel
			ptr.hpixel *= 2;
		}
		vpp_trans_timing(VPP_MOD_GOVRH,&ptr,&t2,1);
		t2.total_line_of_frame -= 1;
		govrh_set_tg2(&t2);
		vppif_reg32_write(GOVRH_HDMI_VBISW,timing->vsync+1);
//		cnt += t2.total_line_of_frame;
	}
#endif
	govrh_set_output_format((ptr.option & VPP_OPT_INTERLACE)? 1:0);
	vppif_reg32_write(GOVRH_HSCALE_UP,(ptr.option & VPP_OPT_HSCALE_UP)? 1:0);

#ifdef WMT_FTBLK_GOVRH_VGA
{
	unsigned int line_pixel;
	
	line_pixel = t1.total_pixel_of_line;
	govrh_set_VGA_sync(ptr.hsync, ptr.vsync * line_pixel);
	govrh_set_VGA_sync_polar((ptr.option & VPP_VGA_HSYNC_POLAR_HI)?0:1,(ptr.option & VPP_VGA_VSYNC_POLAR_HI)?0:1);
	govrh_set_dvo_sync_polar((ptr.option & VPP_VGA_HSYNC_POLAR_HI)?0:1,(ptr.option & VPP_VGA_VSYNC_POLAR_HI)?0:1);
}
#endif

	clk_delay = ( vppif_reg32_read(GOVRH_DVO_OUTWIDTH) )? 0x15:0x820;	// 1-12bit, 0-24bit
	govrh_set_dvo_clock_delay(((clk_delay & 0x1000)!=0x0),clk_delay & 0xFFF);
#ifdef PATCH_GOVRH_VSYNC_OVERLAP
	vppif_reg32_write(GOVRH_VSYNC_OFFSET,vppif_reg32_read(GOVRH_H_ALLPXL));
	if( ptr.vfp > 1 ){
		vppif_reg32_write(GOVRH_ACTLN_BG,vppif_reg32_read(GOVRH_ACTLN_BG)+1);
		vppif_reg32_write(GOVRH_ACTLN_END,vppif_reg32_read(GOVRH_ACTLN_END)+1);
	}
	if( ptr.option & VPP_OPT_INTERLACE ){
		p_govrh->h_pixel = t1.total_pixel_of_line;
		DBGMSG("[GOVRH] vsync overlap patch (interlace)\n");		
	}
	else {
		p_govrh->h_pixel = 0;
		DBGMSG("[GOVRH] vsync overlap patch (progressive)\n");
	}
	p_govrh->v_line = t1.total_line_of_frame;
#else
	vppif_reg32_out(REG_GOVRH_VSYNC_OFFSET,(ptr.option & VPP_OPT_INTERLACE)? ((vppif_reg32_read(GOVRH_H_ALLPXL)/2) | BIT16):0);
#endif
//	vppif_reg32_write(GOVRH_TG_ENABLE, 1);
}

void govrh_set_csc_mode(vpp_csc_t mode)
{
	vdo_color_fmt src_fmt,dst_fmt;
	unsigned int enable;

	const U32 csc_mode_parm[VPP_CSC_MAX][9] = {
		{0x000004a8, 0x04a80662, 0x1cbf1e70, 0x081204a8, 0x00000000, 0x00010001, 0x00000001, 0x00000003, 0x18},	// YUV2RGB_SDTV_0_255
		{0x00000400, 0x0400057c, 0x1d351ea8, 0x06ee0400, 0x00000000, 0x00010001, 0x00000001, 0x00000001, 0x18},	// YUV2RGB_SDTV_16_235
		{0x000004a8, 0x04a8072c, 0x1ddd1f26, 0x087604a8, 0x00000000, 0x00010001, 0x00000001, 0x00000003, 0x18},	// YUV2RGB_HDTV_0_255
		{0x00000400, 0x04000629, 0x1e2a1f45, 0x07440400, 0x00000000, 0x00010001, 0x00000001, 0x00000001, 0x18},	// YUV2RGB_HDTV_16_235
		{0x00000400, 0x0400059c, 0x1d251ea0, 0x07170400, 0x00000000, 0x00010001, 0x00000001, 0x00000001, 0x18},	// YUV2RGB_JFIF_0_255
		{0x00000400, 0x0400057c, 0x1d351ea8, 0x06ee0400, 0x00010000, 0x00010001, 0x00000001, 0x00000001, 0x18},	// YUV2RGB_SMPTE170M
		{0x00000400, 0x0400064d, 0x1e001f19, 0x074f0400, 0x00010000, 0x00010001, 0x00000001, 0x00000003, 0x18},	// YUV2RGB_SMPTE240M
		{0x02040107, 0x1f680064, 0x01c21ed6, 0x1e8701c2, 0x00001fb7, 0x01010021, 0x00000101, 0x00000000, 0x1C},	// RGB2YUV_SDTV_0_255
		{0x02590132, 0x1f500075, 0x020b1ea5, 0x1e4a020b, 0x00001fab, 0x01010001, 0x00000101, 0x00000000, 0x1C},	// RGB2YUV_SDTV_16_235
		{0x027500bb, 0x1f99003f, 0x01c21ea6, 0x1e6701c2, 0x00001fd7, 0x01010021, 0x00000101, 0x00000000, 0x1C},	// RGB2YUV_HDTV_0_255
		{0x02dc00da, 0x1f88004a, 0x020b1e6d, 0x1e25020b, 0x00001fd0, 0x01010001, 0x00000101, 0x00000000, 0x1C},	// RGB2YUV_HDTV_16_235
		{0x02590132, 0x1f530075, 0x02001ead, 0x1e530200, 0x00001fad, 0x01010001, 0x00000101, 0x00000000, 0x1C},	// RGB2YUV_JFIF_0_255
		{0x02590132, 0x1f500075, 0x020b1ea5, 0x1e4a020b, 0x00011fab, 0x01010101, 0x00000000, 0x00000000, 0x1C},	// RGB2YUV_SMPTE170M
		{0x02ce00d9, 0x1f890059, 0x02001e77, 0x1e380200, 0x00011fc8, 0x01010101, 0x00000000, 0x00000000, 0x1C},	// RGB2YUV_SMPTE240M
	};

	enable = 0;
	src_fmt = (govrh_get_color_format()<VDO_COL_FMT_ARGB)? VDO_COL_FMT_YUV444:VDO_COL_FMT_ARGB;
	dst_fmt = (govrh_get_dvo_color_format()<VDO_COL_FMT_ARGB)? VDO_COL_FMT_YUV444:VDO_COL_FMT_ARGB;
	if( src_fmt == VDO_COL_FMT_ARGB ){
		enable |= (dst_fmt != VDO_COL_FMT_ARGB)? 0x01:0x00;	// DVO
	}
	else {
		enable |= (dst_fmt == VDO_COL_FMT_ARGB)? 0x01:0x00;	// DVO
		enable |= 0x2;	// VGA
	}
#ifdef WMT_FTBLK_DISP
	if( src_fmt == VDO_COL_FMT_ARGB ){
		if( disp_get_color_format() < VDO_COL_FMT_ARGB){
			enable |= BIT2;
		}
	}
	else {
		if( disp_get_color_format() >= VDO_COL_FMT_ARGB){
			enable |= BIT2;
		}
	}
#endif
#ifdef WMT_FTBLK_LVDS
	if( src_fmt == VDO_COL_FMT_ARGB ){
		if( lvds_get_colfmt() < VDO_COL_FMT_ARGB){
			enable |= BIT3;
		}
	}
	else {
		if( lvds_get_colfmt() >= VDO_COL_FMT_ARGB){
			enable |= BIT3;
		}
	}
#endif
#ifdef WMT_FTBLK_HDMI
	if( src_fmt == VDO_COL_FMT_ARGB ){
		if( hdmi_get_output_colfmt() < VDO_COL_FMT_ARGB){
			enable |= BIT4;
		}
	}
	else {
		if( hdmi_get_output_colfmt() >= VDO_COL_FMT_ARGB){
			enable |= BIT4;
		}
	}
#endif
	mode = vpp_check_csc_mode(mode,src_fmt,dst_fmt,enable);
	if( mode >= VPP_CSC_MAX ){
		vppif_reg32_write(GOVRH_DVO_YUV2RGB_ENABLE,0);
#ifdef WMT_FTBLK_GOVRH_VGA
		vppif_reg32_write( GOVRH_VGA_YUV2RGB_ENABLE,0);
#endif
#ifdef WMT_FTBLK_DISP
		vppif_reg32_write( GOVRH_DISP_CSC_ENABLE,0);
#endif
#ifdef WMT_FTBLK_LVDS
		vppif_reg32_write( GOVRH_LVDS_CSC_ENABLE,0);
#endif
#ifdef WMT_FTBLK_HDMI
		vppif_reg32_write(GOVRH_HDMI_CSC_ENABLE,0);
#endif
		return;
	}

	vppif_reg32_out(REG_GOVRH_DMACSC_COEF0, csc_mode_parm[mode][0]);
	vppif_reg32_out(REG_GOVRH_DMACSC_COEF1, csc_mode_parm[mode][1]);
	vppif_reg32_out(REG_GOVRH_DMACSC_COEF2, csc_mode_parm[mode][2]);
	vppif_reg32_out(REG_GOVRH_DMACSC_COEF3, csc_mode_parm[mode][3]);
	vppif_reg32_out(REG_GOVRH_DMACSC_COEF4, csc_mode_parm[mode][4]);
	vppif_reg32_out(REG_GOVRH_DMACSC_COEF5, csc_mode_parm[mode][5]);
	vppif_reg32_out(REG_GOVRH_DMACSC_COEF6, csc_mode_parm[mode][6]);
	vppif_reg32_out(REG_GOVRH_CSC_MODE, csc_mode_parm[mode][7]);

	/* enable bit0:DVO, bit1:VGA, bit2:DISP, bit3:LVDS, bit4:HDMI */
#ifdef WMT_FTBLK_DISP
	vppif_reg32_write( GOVRH_DISP_CSC_ENABLE,(enable & BIT2)?1:0);
#endif
#ifdef WMT_FTBLK_LVDS
	vppif_reg32_write( GOVRH_LVDS_CSC_ENABLE,(enable & BIT3)?1:0);
#endif
#ifdef WMT_FTBLK_HDMI
	vppif_reg32_write( GOVRH_HDMI_CSC_ENABLE,(enable & BIT4)?1:0);
#endif
	enable = enable & 0x3;	
	vppif_reg32_write(REG_GOVRH_YUV2RGB, 0x1F, 0, (csc_mode_parm[mode][8] | enable));
}

void govrh_set_framebuffer(vdo_framebuf_t *fb)
{
	govrh_set_MIF_enable(VPP_FLAG_DISABLE);
	govrh_set_fb_addr(fb->y_addr, fb->c_addr);
	govrh_set_data_format(fb->col_fmt);
	govrh_set_fb_info(fb->fb_w, fb->img_w, fb->h_crop, fb->v_crop);
	govrh_set_source_format(vpp_get_fb_field(fb));
	govrh_set_csc_mode(p_govrh->fb_p->csc_mode);
	// govrh TG
//	govrh_set_video_mode(fb->img_w,fb->img_h,p_govrh->fb_p->framerate,p_govrh->vmode);
	govrh_set_MIF_enable(VPP_FLAG_ENABLE);	
}

void govrh_get_framebuffer(vdo_framebuf_t *fb)
{
	govrh_get_fb_addr(&fb->y_addr,&fb->c_addr);
	fb->col_fmt = govrh_get_color_format();
	govrh_get_fb_info(&fb->fb_w, &fb->img_w, &fb->h_crop, &fb->v_crop);
	fb->flag = vppif_reg32_read(GOVRH_INFMT)? VDO_FLAG_INTERLACE:0;
}

#ifdef GOVRH_H264_FMT
void govrh_set_media_format(vpp_flag_t h264)
{
	vppif_reg32_write(GOVRH_H264_FMT,h264);
}
#endif

/*----------------------- GOVRH DISP --------------------------------------*/
#ifdef WMT_FTBLK_DISP
void govrh_DISP_set_enable(vpp_flag_t enable)
{
	/* 1: enable DISP, and disable DVO/VGA */
	vppif_reg32_write(GOVRH_DISP_SEL,enable);

	/* TODO : set VPP DAC SEL */
//	vppif_reg32_write(WM8510_VPP_BASE+0x14,BIT0,0,enable);
}
#endif /* WMT_FTBLK_DISP */

/*----------------------- GOVRH IGS --------------------------------------*/
#ifdef WMT_FTBLK_GOVRH_IGS
void govrh_IGS_set_mode(int no,int mode_18bit,int msb)
{
	if( no == 0 ){ // DVO
		vppif_reg32_write(GOVRH_IGS_MODE,mode_18bit);	// 0-24bit,10-18bit,01-555,11-565
		vppif_reg32_write(GOVRH_LDI_MODE,msb);			// 0-lsb,1-msb
	}
	else { // LVDS
		vppif_reg32_write(GOVRH_IGS_MODE2,mode_18bit);	// 0-24bit,10-18bit,01-555,11-565
		vppif_reg32_write(GOVRH_LDI_MODE2,msb);			// 0-lsb,1-msb
	}
}

#if(WMT_SUB_PID == WMT_PID_8710_B)
void govrh_IGS_set_RGB_swap(int mode)
{
	// 0-RGB [7-0], 1-RGB [0-7], 2-BGR [7-0], 3-BGR [0-7]
	vppif_reg32_write(GOVRH_RGB_SWAP,mode);
}
#endif
#endif /* WMT_FTBLK_GOVRH_IGS */

/*----------------------- GOVRH CURSOR --------------------------------------*/
#ifdef WMT_FTBLK_GOVRH_CURSOR
void govrh_CUR_set_enable(vpp_flag_t enable)
{
	DPRINT("[CUR] govrh_CUR_set_enable %d\n",enable);

	vppif_reg32_write(GOVRH_CUR_ENABLE,enable);
	p_cursor->enable = enable;
}

void govrh_CUR_set_fb_addr(U32 y_addr,U32 c_addr)
{
	vppif_reg32_out(REG_GOVRH_CUR_ADDR,y_addr);
}

void govrh_CUR_get_fb_addr(U32 *y_addr, U32 *c_addr)
{
	*y_addr = vppif_reg32_in(REG_GOVRH_CUR_ADDR);
	*c_addr = 0;
}

void govrh_CUR_set_coordinate(unsigned int x1,unsigned int y1,unsigned int x2,unsigned int y2)
{
	vppif_reg32_write(GOVRH_CUR_H_START,x1);
	vppif_reg32_write(GOVRH_CUR_H_END,x2);
	vppif_reg32_write(GOVRH_CUR_V_START,y1);
	vppif_reg32_write(GOVRH_CUR_V_END,y2);
}

void govrh_CUR_set_position(unsigned int x,unsigned int y)
{
	unsigned int w,h;

//	w = vppif_reg32_read(GOVRH_CUR_H_END) - vppif_reg32_read(GOVRH_CUR_H_START);
	w = vppif_reg32_read(GOVRH_CUR_WIDTH) - 1;
	h = vppif_reg32_read(GOVRH_CUR_V_END) - vppif_reg32_read(GOVRH_CUR_V_START);
	govrh_CUR_set_coordinate(x,y,x+w,y+h);
}

void govrh_CUR_set_color_key(int enable,int alpha,unsigned int colkey)
{
	vppif_reg32_write(GOVRH_CUR_COLKEY,colkey);
	vppif_reg32_write(GOVRH_CUR_ALPHA_ENABLE,0); //alpha);
	vppif_reg32_write(GOVRH_CUR_COLKEY_ENABLE,0); //enable);

	if( !(alpha) && enable ){
		int i,j;
		unsigned int *ptr1,*ptr2;
		int yuv2rgb;

		ptr1 = (unsigned int *)mb_phys_to_virt(vppif_reg32_in(REG_GOVRH_CUR_ADDR));
		for(i=0;i<p_cursor->fb_p->fb.fb_h;i++){
			for(j=0;j<p_cursor->fb_p->fb.fb_w;j++){
				if((*ptr1 & 0xFFFFFF) == (colkey & 0xFFFFFF)){
					*ptr1 &= 0xFFFFFF;
				}
				else {
					*ptr1 |= 0xFF000000;
				}
				ptr1++;
			}
		}

		yuv2rgb = (p_cursor->colfmt < VDO_COL_FMT_ARGB)? 1:0;
		ptr1 = (unsigned int*) mb_phys_to_virt(p_cursor->cursor_addr1);
		ptr2 = (unsigned int*) mb_phys_to_virt(p_cursor->cursor_addr2);
		for(i=0;i<p_cursor->fb_p->fb.fb_h;i++){
			for(j=0;j<p_cursor->fb_p->fb.fb_w;j++){
				*ptr2 = vpp_convert_colfmt(yuv2rgb, *ptr1);
				ptr1++;
				ptr2++;
			}
		}
	}
}

void govrh_CUR_set_colfmt(vdo_color_fmt colfmt)
{
	colfmt = (colfmt < VDO_COL_FMT_ARGB)? VDO_COL_FMT_YUV444:VDO_COL_FMT_ARGB;
//	DPRINT("[CUR] set colfmt %s\n",vpp_colfmt_str[colfmt]);
	if( p_cursor->fb_p->fb.col_fmt == colfmt )
		return;

	p_cursor->fb_p->fb.col_fmt = colfmt;
	vppif_reg32_out(REG_GOVRH_CUR_ADDR,(p_cursor->colfmt==colfmt)? p_cursor->cursor_addr1:p_cursor->cursor_addr2);
}

int govrh_irqproc_set_position(void *arg)
{
	int begin;
	int pos_x,pos_y;
	int crop_x,crop_y;
	int width,height;
	int govrh_w,govrh_h;

	govrh_w = vppif_reg32_read(GOVRH_ACTPX_END) - vppif_reg32_read(GOVRH_ACTPX_BG);
	govrh_h = vppif_reg32_read(GOVRH_ACTLN_END) - vppif_reg32_read(GOVRH_ACTLN_BG);
	if( vppif_reg32_read(GOVRH_TG_MODE) ){
		govrh_h *= 2;
	}

	// cursor H
	width = p_cursor->fb_p->fb.img_w;
	begin = p_cursor->posx - p_cursor->hotspot_x;
	if( begin < 0 ){	// over left edge
		pos_x = 0;
		crop_x = 0 - begin;
		width = width - crop_x;
	}
	else {
		pos_x = begin;
		crop_x = 0;
	}
	if( (begin + p_cursor->fb_p->fb.img_w) > govrh_w ){ // over right edge
		width = govrh_w - begin;
	}
//	DPRINT("[CURSOR] H pos %d,hotspot %d,posx %d,crop %d,width %d\n",
//		p_cursor->posx,p_cursor->hotspot_x,pos_x,crop_x,width);

	// cursor V
	height = p_cursor->fb_p->fb.img_h;
	begin = p_cursor->posy - p_cursor->hotspot_y;
	if( begin < 0 ){ // over top edge
		pos_y = 0;
		crop_y = 0 - begin;
		height = height - crop_y;
	}
	else {
		pos_y = begin;
		crop_y = 0;
	}
	if( (begin + p_cursor->fb_p->fb.img_h) > govrh_h ){ // over right edge
		height = govrh_h - begin;
	}

	if( vppif_reg32_read(GOVRH_TG_MODE) ){
		vppif_reg32_write(GOVRH_CUR_FB_WIDTH,p_cursor->fb_p->fb.fb_w*2);
		pos_y /= 2;
		crop_y /= 2;
		height /= 2;
	}
	else {
		vppif_reg32_write(GOVRH_CUR_FB_WIDTH,p_cursor->fb_p->fb.fb_w);
	}
	
//	DPRINT("[CURSOR] V pos %d,hotspot %d,posx %d,crop %d,height %d\n",
//		p_cursor->posy,p_cursor->hotspot_y,pos_y,crop_y,height);
	
	vppif_reg32_write(GOVRH_CUR_WIDTH,width);
	vppif_reg32_write(GOVRH_CUR_VCROP,crop_y);
	vppif_reg32_write(GOVRH_CUR_HCROP,crop_x);
	govrh_CUR_set_coordinate(pos_x,pos_y,pos_x+width-1,pos_y+height-1);
	return 0;		
}

void govrh_CUR_proc_view(int read,vdo_view_t *view)
{
	vdo_framebuf_t *fb;

	fb = &p_cursor->fb_p->fb;
	if( read ){
		view->resx_src = fb->fb_w;
		view->resy_src = fb->fb_h;
		view->resx_virtual = fb->img_w;
		view->resy_virtual = fb->img_h;
		view->resx_visual = fb->img_w;
		view->resy_visual = fb->img_h;
		view->posx = p_cursor->posx;
		view->posy = p_cursor->posy;
		view->offsetx = fb->h_crop;
		view->offsety = fb->v_crop;
	}
	else {
		p_cursor->posx = view->posx;
		p_cursor->posy = view->posy;
		p_cursor->chg_flag = 1;
	}
}

void govrh_CUR_set_framebuffer(vdo_framebuf_t *fb)
{
	DPRINT("[CUR] govrh_CUR_set_framebuffer\n");

	vppif_reg32_out(REG_GOVRH_CUR_ADDR,fb->y_addr);
	vppif_reg32_write(GOVRH_CUR_WIDTH,fb->img_w);
	if(vppif_reg32_read(GOVRH_TG_MODE)){
		vppif_reg32_write(GOVRH_CUR_FB_WIDTH,fb->fb_w*2);
	}
	else {
		vppif_reg32_write(GOVRH_CUR_FB_WIDTH,fb->fb_w);
	}
	vppif_reg32_write(GOVRH_CUR_VCROP,fb->v_crop);
	vppif_reg32_write(GOVRH_CUR_HCROP,fb->h_crop);
	vppif_reg32_write(GOVRH_CUR_OUT_FIELD,vpp_get_fb_field(fb));
	govrh_irqproc_set_position(0);
	p_cursor->colfmt = fb->col_fmt;
	p_cursor->cursor_addr1 = fb->y_addr;
	if( p_cursor->cursor_addr2 == 0 ){
		p_cursor->cursor_addr2 = mb_alloc(64*64*4);
	}
	vpp_cache_sync();
}

void govrh_CUR_init(void *base)
{

}
#endif /* WMT_FTBLK_GOVRH_CURSOR */

/*----------------------- GOVRH external in WM8440 --------------------------------------*/
void govrh_set_dual_disp(int disp,int govrh)
{
#ifdef GOVRH_DISP_DUAL
	unsigned int reg;	
	if( disp ){
		reg = (govrh)? 0x2:0x1;
	}
	else {
		reg = 0x0;
	}
	vppif_reg32_write(GOVRH_DISP_DUAL,reg);
#endif
}

/*----------------------- GOVRH MODULE API --------------------------------------*/
#ifdef CONFIG_PM
static unsigned int *govrh_reg_bk2;
static unsigned int govrh_pm_enable,govrh_pm_tg;
static unsigned int govrh_vo_clock;
void govrh_suspend(int sts)
{
	switch( sts ){
		case 0:	// disable module
			govrh_pm_enable = vppif_reg32_read(GOVRH_MIF_ENABLE);
			vppif_reg32_write(GOVRH_MIF_ENABLE,0);
			break;
		case 1: // disable tg
			govrh_pm_tg = vppif_reg32_read(GOVRH_TG_ENABLE);
			govrh_set_tg_enable(0);
			break;
		case 2:	// backup register
			p_govrh->reg_bk = vpp_backup_reg(REG_GOVRH_BASE1_BEGIN,(REG_GOVRH_BASE1_END-REG_GOVRH_BASE1_BEGIN));
			govrh_reg_bk2 = vpp_backup_reg(REG_GOVRH_BASE2_BEGIN,(REG_GOVRH_BASE2_END-REG_GOVRH_BASE2_BEGIN));
			govrh_vo_clock = vpp_get_base_clock(VPP_MOD_GOVRH);
			break;
		default:
			break;
	}
}

void govrh_resume(int sts)
{
	switch( sts ){
		case 0:	// restore register
			govrh_set_clock(govrh_vo_clock);
			vpp_restore_reg(REG_GOVRH_BASE2_BEGIN,(REG_GOVRH_BASE2_END-REG_GOVRH_BASE2_BEGIN),govrh_reg_bk2);		/* 0x00 - 0xE8 */
			vpp_restore_reg(REG_GOVRH_BASE1_BEGIN,(REG_GOVRH_BASE1_END-REG_GOVRH_BASE1_BEGIN),p_govrh->reg_bk);		/* 0x00 - 0xE8 */
			govrh_reg_bk2 = 0;
			p_govrh->reg_bk = 0;
			break;
		case 1:	// enable module
			vppif_reg32_write(GOVRH_MIF_ENABLE,govrh_pm_enable);
			break;
		case 2: // enable tg
			govrh_set_tg_enable(govrh_pm_tg);
			break;
		default:
			break;
	}
}
#else
#define govrh_suspend NULL
#define govrh_resume NULL
#endif

void govrh_init(void *base)
{
	govrh_mod_t *mod_p;

	mod_p = (govrh_mod_t *) base;

	govrh_set_reg_level(VPP_REG_LEVEL_1);
	govrh_set_colorbar(VPP_FLAG_DISABLE, 1, 0);
	govrh_set_tg_enable(VPP_FLAG_ENABLE);
	govrh_set_dvo_color_format(VDO_COL_FMT_ARGB);
	govrh_set_dvo_enable(VPP_FLAG_DISABLE);
#ifdef WMT_FTBLK_GOVRH_DAC_THREE
//	vppif_reg32_out(REG_GOVRH_DAC_VAL,GOVRH_DAC_SENSE_VALUE << 16 | GOVRH_DAC_SENSE_VALUE);
	govrh_DAC_set_sense_value(0,0x1AE,0x1AE);
	govrh_DAC_set_sense_value(1,0x1AE,0x1AE);
	govrh_DAC_set_sense_value(2,0x1AE,0x1AE);
	govrh_DAC_set_sense_sel(0);
	govrh_DAC_set_pwrdn(VPP_FLAG_ENABLE);	
#else
#ifdef WMT_FTBLK_GOVRH_DAC
	govrh_set_DAC_pwrdn(VPP_FLAG_ENABLE);
#endif
#endif
	govrh_set_framebuffer(&mod_p->fb_p->fb);
	govrh_set_fifo_index(0xf);
	govrh_set_contrast(0x10);
	govrh_set_dual_disp(0,1);
	govrh_set_brightness(0x0);
	govrh_set_csc_mode(mod_p->fb_p->csc_mode);
#if(WMT_SUB_PID == WMT_PID_8710_B)
	vppif_reg32_write(GOVRH_CUR_COLKEY_INVERT,1);
#endif
	
#ifdef WMT_FTBLK_GOVRH_DAC
  	vppif_reg32_out(REG_GOVRH_DAC_TEST, 0x7000000);
	vppif_reg32_out(REG_GOVRH_DAC_MOD, 0x00770002);
#endif	
	govrh_set_reg_update(VPP_FLAG_ENABLE);
	govrh_set_tg_enable(VPP_FLAG_ENABLE);
	govrh_set_MIF_enable(VPP_FLAG_ENABLE);
	govrh_set_int_enable(VPP_FLAG_ENABLE,mod_p->int_catch);
}

int govrh_mod_init(void)
{
	govrh_mod_t *mod_p;
	vpp_fb_base_t *mod_fb_p;
	vdo_framebuf_t *fb_p;

	mod_p = (govrh_mod_t *) vpp_mod_register(VPP_MOD_GOVRH,sizeof(govrh_mod_t),VPP_MOD_FLAG_FRAMEBUF);
	if( !mod_p ){
		DPRINT("*E* GOVRH module register fail\n");
		return -1;
	}

	/* module member variable */
	mod_p->int_catch = VPP_INT_NULL; // VPP_INT_ERR_GOVRH_MIF;
	mod_p->vga_dac_sense_cnt = VPP_DAC_SENSE_SECOND * 60;
	mod_p->vga_dac_sense_val = 0x300;

	/* module member function */
	mod_p->init = govrh_init;
	mod_p->dump_reg = govrh_reg_dump;
	mod_p->set_enable = govrh_set_MIF_enable;
	mod_p->set_colorbar = govrh_set_colorbar;
	mod_p->set_tg = govrh_set_tg;
	mod_p->get_tg = govrh_get_tg;
	mod_p->get_sts = govrh_get_int_status;
	mod_p->clr_sts = govrh_clean_int_status;
	mod_p->suspend = govrh_suspend;
	mod_p->resume = govrh_resume;

	/* module frame buffer */
	mod_fb_p = mod_p->fb_p;
	mod_fb_p->csc_mode = VPP_CSC_RGB2YUV_JFIF_0_255;	
	mod_fb_p->framerate = VPP_HD_DISP_FPS;
	mod_fb_p->media_fmt = VPP_MEDIA_FMT_MPEG;
	mod_fb_p->wait_ready = 0;
	mod_fb_p->capability = BIT(VDO_COL_FMT_YUV420) | BIT(VDO_COL_FMT_YUV422H) | BIT(VDO_COL_FMT_YUV444) | BIT(VDO_COL_FMT_ARGB)
							| BIT(VDO_COL_FMT_RGB_1555) | BIT(VDO_COL_FMT_RGB_565)
							| VPP_FB_FLAG_CSC | VPP_FB_FLAG_MEDIA | VPP_FB_FLAG_FIELD;

	/* module frame buffer member function */
	mod_fb_p->set_framebuf = govrh_set_framebuffer;
	mod_fb_p->set_addr = govrh_set_fb_addr;
	mod_fb_p->get_addr = govrh_get_fb_addr;
	mod_fb_p->set_csc = govrh_set_csc_mode;
	mod_fb_p->get_color_fmt = govrh_get_color_format;
	mod_fb_p->set_color_fmt = (void *) govrh_set_data_format;
	
	fb_p = &mod_p->fb_p->fb;
	fb_p->y_addr = 0;
	fb_p->c_addr = 0;
	fb_p->col_fmt = VPP_GOVW_FB_COLFMT;
	fb_p->img_w = VPP_HD_DISP_RESX;
	fb_p->img_h = VPP_HD_DISP_RESY;
	fb_p->fb_w = VPP_HD_MAX_RESX;
	fb_p->fb_h = VPP_HD_MAX_RESY;
	fb_p->h_crop = 0;
	fb_p->v_crop = 0;
	fb_p->flag = 0;
	
	p_govrh = mod_p;

#ifdef WMT_FTBLK_GOVRH_CURSOR
	mod_p = (govrh_mod_t *) vpp_mod_register(VPP_MOD_CURSOR,sizeof(govrh_cursor_mod_t),VPP_MOD_FLAG_FRAMEBUF);
	if( !mod_p ){
		DPRINT("*E* CURSOR module register fail\n");
		return -1;
	}

	/* module member function */
	mod_p->init = govrh_CUR_init;
	mod_p->set_enable = govrh_CUR_set_enable;

	/* module frame buffer */
	mod_fb_p = mod_p->fb_p;
	mod_fb_p->csc_mode = VPP_CSC_RGB2YUV_JFIF_0_255;	
	mod_fb_p->framerate = VPP_HD_DISP_FPS;
	mod_fb_p->media_fmt = VPP_MEDIA_FMT_MPEG;
	mod_fb_p->wait_ready = 0;
	mod_fb_p->capability = BIT(VDO_COL_FMT_YUV420) | BIT(VDO_COL_FMT_YUV422H) | BIT(VDO_COL_FMT_YUV444) | BIT(VDO_COL_FMT_ARGB)
							| VPP_FB_FLAG_CSC | VPP_FB_FLAG_MEDIA | VPP_FB_FLAG_FIELD;

	/* module frame buffer member function */
	mod_fb_p->set_framebuf = govrh_CUR_set_framebuffer;
	mod_fb_p->set_addr = govrh_CUR_set_fb_addr;
	mod_fb_p->get_addr = govrh_CUR_get_fb_addr;
//	mod_fb_p->set_csc = govrh_set_csc_mode;
//	mod_fb_p->get_color_fmt = govrh_get_color_format;
//	mod_fb_p->set_color_fmt = (void *) govrh_set_data_format;
	mod_fb_p->fn_view = govrh_CUR_proc_view;
	
	fb_p = &mod_p->fb_p->fb;
	fb_p->y_addr = 0;
	fb_p->c_addr = 0;
	fb_p->col_fmt = VDO_COL_FMT_YUV444;
	fb_p->img_w = VPP_HD_DISP_RESX;
	fb_p->img_h = VPP_HD_DISP_RESY;
	fb_p->fb_w = VPP_HD_MAX_RESX;
	fb_p->fb_h = VPP_HD_MAX_RESY;
	fb_p->h_crop = 0;
	fb_p->v_crop = 0;
	fb_p->flag = 0;
	
	p_cursor = (govrh_cursor_mod_t *) mod_p;
#endif
	
	return 0;
}
module_init(govrh_mod_init);
#endif				//WMT_FTBLK_GOVRH
