/*++ 
 * linux/drivers/video/wmt/scl.c
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

#define SCL_C
// #define DEBUG

#include "scl.h"
 
#ifdef WMT_FTBLK_SCL
void scl_reg_dump(void)
{
	DPRINT("========== SCL register dump ==========\n");
	vpp_reg_dump(REG_SCL_BASE1_BEGIN,REG_SCL_BASE1_END-REG_SCL_BASE1_BEGIN);
	vpp_reg_dump(REG_SCL_BASE2_BEGIN,REG_SCL_BASE2_END-REG_SCL_BASE2_BEGIN);

	DPRINT("---------- SCL scale ----------\n");	
	DPRINT("scale enable %d\n",vppif_reg32_read(SCL_ALU_ENABLE));
	DPRINT("scale width H %d,V %d\n",vppif_reg32_read(SCL_HXWIDTH),vppif_reg32_read(SCL_VXWIDTH));	
	DPRINT("H scale up %d,V scale up %d\n",vppif_reg32_read(SCL_HSCLUP_ENABLE),vppif_reg32_read(SCL_VSCLUP_ENABLE));
	DPRINT("H sub step %d,thr %d,step %d,sub step cnt %d,i step cnt %d\n",vppif_reg32_read(SCL_H_SUBSTEP),
		vppif_reg32_read(SCL_H_THR),vppif_reg32_read(SCL_H_STEP),vppif_reg32_read(SCL_H_I_SUBSTEPCNT),vppif_reg32_read(SCL_H_I_STEPCNT));
	DPRINT("V sub step %d,thr %d,step %d,sub step cnt %d,i step cnt %d\n",vppif_reg32_read(SCL_V_SUBSTEP),
		vppif_reg32_read(SCL_V_THR),vppif_reg32_read(SCL_V_STEP),vppif_reg32_read(SCL_V_I_SUBSTEPCNT),vppif_reg32_read(SCL_V_I_STEPCNT));
	DPRINT("CSC enable %d,CSC clamp %d\n",vppif_reg32_read(SCL_CSC_ENABLE),vppif_reg32_read(SCL_CSC_CLAMP_ENABLE));

	DPRINT("---------- SCL TG ----------\n");	
	DPRINT("TG source : %s\n",(vppif_reg32_read(SCL_TG_GOVWTG_ENABLE))?"GOVW":"SCL");
	DPRINT("TG enable %d, wait ready enable %d\n",vppif_reg32_read(SCL_TG_ENABLE),vppif_reg32_read(SCL_TG_WATCHDOG_ENABLE));	
	DPRINT("clk %d,Read cyc %d\n",vpp_get_base_clock(VPP_MOD_SCL),vppif_reg32_read(SCL_TG_RDCYC));
	DPRINT("H total %d, beg %d, end %d\n",vppif_reg32_read(SCL_TG_H_ALLPIXEL),vppif_reg32_read(SCL_TG_H_ACTBG),vppif_reg32_read(SCL_TG_H_ACTEND));
	DPRINT("V total %d, beg %d, end %d\n",vppif_reg32_read(SCL_TG_V_ALLLINE),vppif_reg32_read(SCL_TG_V_ACTBG),vppif_reg32_read(SCL_TG_V_ACTEND));
	DPRINT("VBIE %d,PVBI %d\n",vppif_reg32_read(SCL_TG_VBIE),vppif_reg32_read(SCL_TG_PVBI));
	DPRINT("Watch dog %d\n",vppif_reg32_read(SCL_TG_WATCHDOG_VALUE));

	DPRINT("---------- SCLR FB ----------\n");	
	DPRINT("SCLR MIF enable %d,MIF2 enable %d\n",vppif_reg32_read(SCLR_MIF_ENABLE),vppif_reg32_read(SCLR_MIF2_ENABLE));
	DPRINT("color format %s\n",vpp_colfmt_str[sclr_get_color_format()]);
	DPRINT("color bar enable %d,mode %d,inv %d\n",vppif_reg32_read(SCLR_COLBAR_ENABLE),vppif_reg32_read(SCLR_COLBAR_MODE),
		vppif_reg32_read(SCLR_COLBAR_INVERSION));
	DPRINT("sourc mode : %s,H264 %d\n",(vppif_reg32_read(SCLR_TAR_DISP_FMT))?"field":"frame",vppif_reg32_read(SCLR_MEDIAFMT_H264));
	DPRINT("Y addr 0x%x, C addr 0x%x\n",vppif_reg32_in(REG_SCLR_YSA),vppif_reg32_in(REG_SCLR_CSA));
#ifdef REG_SCLR_YSA2
	DPRINT("Y addr2 0x%x, C addr2 0x%x\n",vppif_reg32_in(REG_SCLR_YSA2),vppif_reg32_in(REG_SCLR_CSA2));
#endif
	DPRINT("width %d, fb width %d\n",vppif_reg32_read(SCLR_YPXLWID),vppif_reg32_read(SCLR_YBUFWID));
	DPRINT("H crop %d, V crop %d\n",vppif_reg32_read(SCLR_HCROP),vppif_reg32_read(SCLR_VCROP));
	
	DPRINT("---------- SCLW FB ----------\n");	
	DPRINT("SCLW MIF enable %d\n",vppif_reg32_read(SCLW_MIF_ENABLE));
	DPRINT("color format %s\n",vpp_colfmt_str[sclw_get_color_format()]);	
	DPRINT("Y addr 0x%x, C addr 0x%x\n",vppif_reg32_in(REG_SCLW_YSA),vppif_reg32_in(REG_SCLW_CSA));
	DPRINT("Y width %d, fb width %d\n",vppif_reg32_read(SCLW_YPXLWID),vppif_reg32_read(SCLW_YBUFWID));
	DPRINT("C width %d, fb width %d\n",vppif_reg32_read(SCLW_CPXLWID),vppif_reg32_read(SCLW_CBUFWID));	
	DPRINT("Y err %d, C err %d\n",vppif_reg32_read(SCLW_INTSTS_MIFYERR),vppif_reg32_read(SCLW_INTSTS_MIFCERR));		

}

void scl_set_enable(vpp_flag_t enable)
{
	vppif_reg32_write(SCL_ALU_ENABLE, enable);
}

void scl_set_reg_update(vpp_flag_t enable)
{
	vppif_reg32_write(SCL_REG_UPDATE, enable);
}

void scl_set_reg_level(vpp_reglevel_t level)
{
	switch (level) {
	case VPP_REG_LEVEL_1:
		vppif_reg32_write(SCL_REG_LEVEL, 0x0);
		break;
	case VPP_REG_LEVEL_2:
		vppif_reg32_write(SCL_REG_LEVEL, 0x1);
		break;
	default:
		DBGMSG("*E* check the parameter.\n");
		return;
	}
}

void scl_set_int_enable(vpp_flag_t enable, vpp_int_t int_bit)
{
	//clean status first before enable/disable interrupt
	scl_clean_int_status(int_bit);

	if (int_bit & VPP_INT_ERR_SCL_TG) {
		vppif_reg32_write(SCLW_INT_TGERR_ENABLE, enable);
	}
	if (int_bit & VPP_INT_ERR_SCLR1_MIF) {
		vppif_reg32_write(SCLW_INT_R1MIF_ENABLE, enable);
	}
	if (int_bit & VPP_INT_ERR_SCLR2_MIF) {
		vppif_reg32_write(SCLW_INT_R2MIF_ENABLE, enable);
	}
	if (int_bit & VPP_INT_ERR_SCLW_MIFRGB) {
		vppif_reg32_write(SCLW_INT_WMIFRGB_ENABLE, enable);
	}
	if (int_bit & VPP_INT_ERR_SCLW_MIFY) {
		vppif_reg32_write(SCLW_INT_WMIFYERR_ENABLE, enable);
	}
	if (int_bit & VPP_INT_ERR_SCLW_MIFC) {
		vppif_reg32_write(SCLW_INT_WMIFCERR_ENABLE, enable);
	}
}

vpp_int_err_t scl_get_int_status(void)
{
	vpp_int_err_t int_sts;

	int_sts = 0;
	if (vppif_reg32_read(SCL_INTSTS_TGERR))
		int_sts |= VPP_INT_ERR_SCL_TG;
	if (vppif_reg32_read(SCLR_INTSTS_R1MIFERR))
		int_sts |= VPP_INT_ERR_SCLR1_MIF;
	if (vppif_reg32_read(SCLR_INTSTS_R2MIFERR))
		int_sts |= VPP_INT_ERR_SCLR2_MIF;
	if (vppif_reg32_read(SCLW_INTSTS_MIFRGBERR))
		int_sts |= VPP_INT_ERR_SCLW_MIFRGB;
	if (vppif_reg32_read(SCLW_INTSTS_MIFYERR))
		int_sts |= VPP_INT_ERR_SCLW_MIFY;
	if (vppif_reg32_read(SCLW_INTSTS_MIFCERR))
		int_sts |= VPP_INT_ERR_SCLW_MIFC;
	
	return int_sts;
}

void scl_clean_int_status(vpp_int_err_t int_sts)
{
	if (int_sts & VPP_INT_ERR_SCL_TG) {
		vppif_reg32_out(REG_SCL_TG_STS+0x0,BIT0);
	}
	if (int_sts & VPP_INT_ERR_SCLR1_MIF) {
		vppif_reg8_out(REG_SCLR_FIFO_CTL+0x1,BIT0);
	}
	if (int_sts & VPP_INT_ERR_SCLR2_MIF) {
		vppif_reg8_out(REG_SCLR_FIFO_CTL+0x1,BIT1);
	}
	if (int_sts & VPP_INT_ERR_SCLW_MIFRGB) {
		vppif_reg32_out(REG_SCLW_FF_CTL,BIT16);
	}
	if (int_sts & VPP_INT_ERR_SCLW_MIFY) {
		vppif_reg32_out(REG_SCLW_FF_CTL,BIT8);
	}
	if (int_sts & VPP_INT_ERR_SCLW_MIFC) {
		vppif_reg32_out(REG_SCLW_FF_CTL,BIT0);
	}
}

void scl_set_csc_mode(vpp_csc_t mode)
{
	vdo_color_fmt src_fmt,dst_fmt;

	src_fmt = sclr_get_color_format();
	dst_fmt = ( scl_get_timing_master() == VPP_MOD_SCL )? sclw_get_color_format():govw_get_hd_color_format();
	mode = vpp_check_csc_mode(mode,src_fmt,dst_fmt,0);

	if (mode >= VPP_CSC_MAX) {
		vppif_reg32_write(SCL_CSC_ENABLE, VPP_FLAG_DISABLE);
	} else {
		vppif_reg32_out(REG_SCL_CSC1, vpp_csc_parm[mode][0]);	//CSC1
		vppif_reg32_out(REG_SCL_CSC2, vpp_csc_parm[mode][1]);	//CSC2
		vppif_reg32_out(REG_SCL_CSC3, vpp_csc_parm[mode][2]);	//CSC3
		vppif_reg32_out(REG_SCL_CSC4, vpp_csc_parm[mode][3]);	//CSC4
		vppif_reg32_out(REG_SCL_CSC5, vpp_csc_parm[mode][4]);	//CSC5
		vppif_reg32_out(REG_SCL_CSC6, vpp_csc_parm[mode][5]);	//CSC6
		vppif_reg32_out(REG_SCL_CSC_CTL, vpp_csc_parm[mode][6]);	//CSC_CTL
		vppif_reg32_write(SCL_CSC_ENABLE, VPP_FLAG_ENABLE);
	}
}

//STILL_IMAGE
void scl_set_scale_enable(vpp_flag_t vscl_enable, vpp_flag_t hscl_enable)
{
	DBGMSG("V %d,H %d\n",vscl_enable,hscl_enable);
	vppif_reg32_write(SCL_VSCLUP_ENABLE,vscl_enable);
	vppif_reg32_write(SCL_HSCLUP_ENABLE,hscl_enable);
}

//REALTIME
void scl_set_V_scale(int A, int B)
{
	unsigned int V_STEP;
	unsigned int V_SUB_STEP;
	unsigned int V_THR_DIV2;

//	DBGMSG("vpu_set_V_scale(%d,%d)\r\n", A, B);
	if( A > B ){
		V_STEP = (B -1) * 16 / A;
	   	V_SUB_STEP = (B -1) * 16  % A;
	}
	else {
		V_STEP = (16 * B / A);
		V_SUB_STEP = ((16 * B) % A);
	}
	V_THR_DIV2 = A;
//	DBGMSG("V step %d,sub step %d, div2 %d\r\n", V_STEP, V_SUB_STEP, V_THR_DIV2);

#ifdef SCL_DST_VXWIDTH
	vppif_reg32_write(SCL_DST_VXWIDTH,(A>B)?A:B);
#endif
	vppif_reg32_write(SCL_VXWIDTH,B);
	vppif_reg32_write(SCL_V_STEP, V_STEP);
	vppif_reg32_write(SCL_V_SUBSTEP, V_SUB_STEP);
	vppif_reg32_write(SCL_V_THR, V_THR_DIV2);
	vppif_reg32_write(SCL_V_I_SUBSTEPCNT, 0);
}

void scl_set_H_scale(int A, int B)
{
	unsigned int H_STEP;
	unsigned int H_SUB_STEP;
	unsigned int H_THR_DIV2;

//	DBGMSG("vpu_set_H_scale(%d,%d)\r\n", A, B);
	if( A > B ){
		H_STEP = (B -1) * 16 / A;
	   	H_SUB_STEP = (B -1) * 16  % A;
	}
	else {
		H_STEP = (16 * B / A);
		H_SUB_STEP = ((16 * B) % A);
	}
	H_THR_DIV2 = A;
//	DBGMSG("H step %d,sub step %d, div2 %d\r\n", H_STEP, H_SUB_STEP, H_THR_DIV2);

	vppif_reg32_write(SCL_HXWIDTH,((A>B)?A:B));
	vppif_reg32_write(SCL_H_STEP, H_STEP);
	vppif_reg32_write(SCL_H_SUBSTEP, H_SUB_STEP);
	vppif_reg32_write(SCL_H_THR, H_THR_DIV2);
	vppif_reg32_write(SCL_H_I_SUBSTEPCNT, 0);
}

void scl_set_crop(int offset_x, int offset_y)
{
	//offset_x &= VPU_CROP_ALIGN_MASK;      /* ~0x7 */
	offset_x &= ~0xf;

	vppif_reg32_write(SCL_H_I_STEPCNT, offset_x * 16);
	vppif_reg32_write(SCL_V_I_STEPCNT, offset_y * 16);
	//vppif_reg32_write(VPU_SCA_THR, 0xFF); ?
	DBGMSG("[VPU] crop - x : 0x%x, y : 0x%x \r\n", offset_x * 16, offset_y * 16);
}

//TG
void scl_set_tg_enable(vpp_flag_t enable)
{
	vppif_reg32_write(SCL_TG_ENABLE, enable);
}

unsigned int scl_set_clock(unsigned int pixel_clock)
{
	unsigned int rd_cyc;
	rd_cyc = vpp_get_base_clock(VPP_MOD_SCL) / pixel_clock;
	return rd_cyc;
}

void scl_set_timing(vpp_clock_t *timing,unsigned int pixel_clock)
{
	timing->read_cycle = scl_set_clock(pixel_clock * 2) - 1;

	if(1){
//	if( vppif_reg32_read(SCL_BILINEAR_H) || vppif_reg32_read(SCL_BILINEAR_V) ){
		timing->read_cycle = ( timing->read_cycle < WMT_SCL_RCYC_MIN )? WMT_SCL_RCYC_MIN:timing->read_cycle;
//		timing->read_cycle = ( timing->read_cycle < 2 )? 2:timing->read_cycle;
	}
	else {	// rec table old design should be more than 3T
		timing->read_cycle = ( timing->read_cycle < 2 )? 2:timing->read_cycle;
	}
	timing->read_cycle = ( timing->read_cycle > 255 )? 0xFF:timing->read_cycle;

	vppif_reg32_write(SCL_TG_RDCYC, timing->read_cycle);
	vppif_reg32_write(SCL_TG_H_ALLPIXEL, timing->total_pixel_of_line);
	vppif_reg32_write(SCL_TG_H_ACTBG, timing->begin_pixel_of_active);
	vppif_reg32_write(SCL_TG_H_ACTEND, timing->end_pixel_of_active);
	vppif_reg32_write(SCL_TG_V_ALLLINE, timing->total_line_of_frame);
	vppif_reg32_write(SCL_TG_V_ACTBG, timing->begin_line_of_active);
	vppif_reg32_write(SCL_TG_V_ACTEND, timing->end_line_of_active);
	vppif_reg32_write(SCL_TG_VBIE, timing->line_number_between_VBIS_VBIE);
	vppif_reg32_write(SCL_TG_PVBI, timing->line_number_between_PVBI_VBIS);
}

void scl_get_timing(vpp_clock_t * p_timing)
{
	p_timing->read_cycle = vppif_reg32_read(SCL_TG_RDCYC);
	p_timing->total_pixel_of_line = vppif_reg32_read(SCL_TG_H_ALLPIXEL);
	p_timing->begin_pixel_of_active = vppif_reg32_read(SCL_TG_H_ACTBG);
	p_timing->end_pixel_of_active = vppif_reg32_read(SCL_TG_H_ACTEND);
	p_timing->total_line_of_frame = vppif_reg32_read(SCL_TG_V_ALLLINE);
	p_timing->begin_line_of_active = vppif_reg32_read(SCL_TG_V_ACTBG);
	p_timing->end_line_of_active = vppif_reg32_read(SCL_TG_V_ACTEND);
	p_timing->line_number_between_VBIS_VBIE = vppif_reg32_read(SCL_TG_VBIE);
	p_timing->line_number_between_PVBI_VBIS = vppif_reg32_read(SCL_TG_PVBI);
}

void scl_set_watchdog(U32 count)
{
	if (0 != count) {
		vppif_reg32_write(SCL_TG_WATCHDOG_VALUE, count);
		vppif_reg32_write(SCL_TG_WATCHDOG_ENABLE, VPP_FLAG_TRUE);
	} else {
		vppif_reg32_write(SCL_TG_WATCHDOG_ENABLE, VPP_FLAG_FALSE);
	}
}

void scl_set_timing_master(vpp_mod_t mod_bit)
{
	if (VPP_MOD_SCL == mod_bit) {
		vppif_reg32_write(SCL_TG_GOVWTG_ENABLE, VPP_FLAG_DISABLE);
	} 
	else if (VPP_MOD_GOVW == mod_bit) {
		vppif_reg32_write(SCL_TG_GOVWTG_ENABLE, VPP_FLAG_ENABLE);
	} 
	else {
		DBGMSG("*E* check the parameter.\n");
		return;
	}
}

vpp_mod_t scl_get_timing_master(void)
{
	if( vppif_reg32_read(SCL_TG_GOVWTG_ENABLE) ){
		return VPP_MOD_GOVW;
	}
	return VPP_MOD_SCL;
}

void scl_set_drop_line(vpp_flag_t enable)
{
	vppif_reg32_write(SCL_SCLDW_METHOD, enable);
}

//SCLR
void sclr_set_mif_enable(vpp_flag_t enable)
{
	vppif_reg32_write(SCLR_MIF_ENABLE, enable);
}

void sclr_set_mif2_enable(vpp_flag_t enable)
{
	vppif_reg32_write(SCLR_MIF2_ENABLE, enable);
}

void sclr_set_colorbar(vpp_flag_t enable,int width, int inverse)
{
	vppif_reg32_write(SCLR_COLBAR_MODE, width);
	vppif_reg32_write(SCLR_COLBAR_INVERSION, inverse);
	vppif_reg32_write(SCLR_COLBAR_ENABLE, enable);
}

void sclr_set_field_mode(vpp_display_format_t fmt)
{
	vppif_reg32_write(SCLR_SRC_DISP_FMT,fmt);
}

void sclr_set_display_format(vpp_display_format_t source, vpp_display_format_t target)
{
	//source
	switch (source) {
	case VPP_DISP_FMT_FRAME:
		vppif_reg32_write(SCLR_SRC_DISP_FMT, 0x0);
		break;
	case VPP_DISP_FMT_FIELD:
		vppif_reg32_write(SCLR_SRC_DISP_FMT, 0x1);
		break;
	default:
		DBGMSG("*E* check the parameter.\n");
		return;
	}
	
	//target
	switch (target) {
	case VPP_DISP_FMT_FRAME:
		vppif_reg32_write(SCLR_TAR_DISP_FMT, 0x0);
		break;
	case VPP_DISP_FMT_FIELD:
		vppif_reg32_write(SCLR_TAR_DISP_FMT, 0x1);
		break;
	default:
		DBGMSG("*E* check the parameter.\n");
		return;
	}
}

void sclr_set_color_format(vdo_color_fmt format)
{
#if(WMT_SUB_PID >= WMT_PID_8710_B)
	if( format >= VDO_COL_FMT_ARGB ){
		if( format == VDO_COL_FMT_RGB_565 ){
			vppif_reg32_write(SCLR_RGB_MODE, 0x1);
		}
		else {
			vppif_reg32_write(SCLR_RGB_MODE, 0x3);
		}
		return;
	}
	vppif_reg32_write(SCLR_RGB_MODE, 0x0);
#endif
	switch (format) {
	case VDO_COL_FMT_ARGB:
		vppif_reg32_write(SCLR_COLFMT_RGB, 0x1);
		break;
	case VDO_COL_FMT_YUV444:
		vppif_reg32_write(SCLR_COLFMT_RGB, 0x0);
		vppif_reg32_write(SCLR_COLFMT_YUV, 0x2);
		break;
	case VDO_COL_FMT_YUV422H:
		vppif_reg32_write(SCLR_COLFMT_RGB, 0x0);
		vppif_reg32_write(SCLR_COLFMT_YUV, 0x0);
		break;
	case VDO_COL_FMT_YUV420:
		vppif_reg32_write(SCLR_COLFMT_RGB, 0x0);
		vppif_reg32_write(SCLR_COLFMT_YUV, 0x1);
		break;
	default:
		DBGMSG("*E* check the parameter.\n");
		return;
	}
}

vdo_color_fmt sclr_get_color_format(void)
{
#if(WMT_SUB_PID >= WMT_PID_8710_B)
	switch( vppif_reg32_read(SCLR_RGB_MODE) ){
		case 0x1: return VDO_COL_FMT_RGB_565;
		case 0x3: return VDO_COL_FMT_ARGB;
		default:
			break;
	}
#else
	if( vppif_reg32_read(SCLR_COLFMT_RGB) ){
		return VDO_COL_FMT_ARGB;
	}
#endif
	switch( vppif_reg32_read(SCLR_COLFMT_YUV) ){
		case 0: return VDO_COL_FMT_YUV422H;
		case 1: return VDO_COL_FMT_YUV420;
		case 2: return VDO_COL_FMT_YUV444;
		default: break;		
	}
	return VDO_COL_FMT_YUV444;
}

void sclr_set_media_format(vpp_media_format_t format)
{
	switch (format) {
	case VPP_MEDIA_FMT_MPEG:
		vppif_reg32_write(SCLR_MEDIAFMT_H264, 0x0);
		break;
	case VPP_MEDIA_FMT_H264:
		vppif_reg32_write(SCLR_MEDIAFMT_H264, 0x1);
		break;
	default:
		DBGMSG("*E* check the parameter.\n");
		return;
	}
}

void sclr_set_fb_addr(U32 y_addr, U32 c_addr)
{
	unsigned int line_y,line_c;
	unsigned int offset_y,offset_c;

	DBGMSG("y_addr:0x%08x, c_addr:0x%08x\n",y_addr,c_addr);

	offset_y = offset_c = 0;
	line_y = line_c = vppif_reg32_read(SCLR_YBUFWID);
	switch(sclr_get_color_format()){
		case VDO_COL_FMT_YUV420:
			offset_c /= 2;
			line_c = 0;
			break;
		case VDO_COL_FMT_YUV422H:
			break;
		case VDO_COL_FMT_ARGB:
			offset_y *= 4;
			line_y *= 4;
			break;
		case VDO_COL_FMT_RGB_565:
			offset_y *= 2;
			line_y *= 2;
			break;
		default:
			offset_c *= 2;
			line_c *= 2;
			break;
	}
	vppif_reg32_out(REG_SCLR_YSA,y_addr+offset_y);
	vppif_reg32_out(REG_SCLR_CSA,c_addr+offset_c);
#ifdef REG_SCLR_YSA2
	vppif_reg32_out(REG_SCLR_YSA2,y_addr+offset_y+line_y);
	vppif_reg32_out(REG_SCLR_CSA2,c_addr+offset_c+line_c);
#endif
}

void sclr_get_fb_addr(U32 * y_addr, U32 * c_addr)
{
	*y_addr = vppif_reg32_in(REG_SCLR_YSA);
	*c_addr = vppif_reg32_in(REG_SCLR_CSA);
//	DBGMSG("y_addr:0x%08x, c_addr:0x%08x\n", *y_addr, *c_addr);
}

void sclr_set_width(U32 y_pixel, U32 y_buffer)
{
	vppif_reg32_write(SCLR_YPXLWID, y_pixel);
	vppif_reg32_write(SCLR_YBUFWID, y_buffer);
}

void sclr_get_width(U32 * p_y_pixel, U32 * p_y_buffer)
{
	*p_y_pixel = vppif_reg32_read(SCLR_YPXLWID);
	*p_y_buffer = vppif_reg32_read(SCLR_YBUFWID);
}

void sclr_set_crop(U32 h_crop, U32 v_crop)
{
	vppif_reg32_write(SCLR_HCROP, h_crop);
	vppif_reg32_write(SCLR_VCROP, v_crop);
}

void sclr_get_fb_info(U32 * width, U32 * act_width, U32 * x_offset,U32 * y_offset)
{
	*width = vppif_reg32_read(SCLR_YBUFWID);
	*act_width = vppif_reg32_read(SCLR_YPXLWID);
	*x_offset = vppif_reg32_read(SCLR_HCROP);
	*y_offset = vppif_reg32_read(SCLR_VCROP);
}

void sclr_set_threshold(U32 value)
{
	vppif_reg32_write(SCLR_FIFO_THR, value);
}

//SCLW
void sclw_set_mif_enable(vpp_flag_t enable)
{
	vppif_reg32_write(SCLW_MIF_ENABLE, enable);
}

void sclw_set_color_format(vdo_color_fmt format)
{
#if(WMT_SUB_PID == WMT_PID_8710_A)
	switch (format) {
	case VDO_COL_FMT_ARGB:
		vppif_reg32_write(SCLW_COLFMT_RGB, 1);
		break;
	case VDO_COL_FMT_YUV444:
		vppif_reg32_write(SCLW_COLFMT_RGB, 0);
		vppif_reg32_write(SCLW_COLFMT_YUV, 0);
		break;
	case VDO_COL_FMT_YUV422H:
		vppif_reg32_write(SCLW_COLFMT_RGB, 0);
		vppif_reg32_write(SCLW_COLFMT_YUV, 1);
		break;
	default:
		DBGMSG("*E* check the parameter.\n");
		return;
	}
#else
	// 0-888(4 byte), 1-5515(2 byte), 2-666(4 byte), 3-565(2 byte)
	switch (format) {
	case VDO_COL_FMT_RGB_666:
		vppif_reg32_write(SCLW_COLFMT_RGB, 1);
		vppif_reg32_write(SCL_IGS_MODE,2);
		break;
	case VDO_COL_FMT_RGB_565:
		vppif_reg32_write(SCLW_COLFMT_RGB, 1);
		vppif_reg32_write(SCL_IGS_MODE,3);
		break;
	case VDO_COL_FMT_RGB_1555:
		vppif_reg32_write(SCLW_COLFMT_RGB, 1);
		vppif_reg32_write(SCL_IGS_MODE,1);
		break;
	case VDO_COL_FMT_ARGB:
		vppif_reg32_write(SCLW_COLFMT_RGB, 1);
		vppif_reg32_write(SCL_IGS_MODE,0);
		break;
	case VDO_COL_FMT_YUV444:
		vppif_reg32_write(SCLW_COLFMT_RGB, 0);
		vppif_reg32_write(SCLW_COLFMT_YUV, 0);
		vppif_reg32_write(SCL_IGS_MODE,0);
		break;
	case VDO_COL_FMT_YUV422H:
	case VDO_COL_FMT_YUV420:
		vppif_reg32_write(SCLW_COLFMT_RGB, 0);
		vppif_reg32_write(SCLW_COLFMT_YUV, 1);
		vppif_reg32_write(SCL_IGS_MODE,0);
		break;
	default:
		DBGMSG("*E* check the parameter.\n");
		return;
	}
#endif
}

vdo_color_fmt sclw_get_color_format(void)
{
#if(WMT_SUB_PID == WMT_PID_8710_A)
	if( vppif_reg32_read(SCLW_COLFMT_RGB) )
		return VDO_COL_FMT_ARGB;
#else
	if( vppif_reg32_read(SCLW_COLFMT_RGB) ){
		switch(vppif_reg32_read(SCL_IGS_MODE)){
			case 0: return VDO_COL_FMT_ARGB;
			case 1: return VDO_COL_FMT_RGB_1555;
			case 2: return VDO_COL_FMT_RGB_666;
			case 3: return VDO_COL_FMT_RGB_565;
		}
	}
#endif

	if( vppif_reg32_read(SCLW_COLFMT_YUV) ){
		return VDO_COL_FMT_YUV422H;
	}
	return VDO_COL_FMT_YUV444;
}

#if(WMT_SUB_PID >= WMT_PID_8710_B)
void sclw_set_alpha(int enable,char data)
{
	vppif_reg32_write(SCL_FIXED_ALPHA_DATA,data);
	vppif_reg32_write(SCL_FIXED_ALPHA_ENABLE,enable);
}
#endif

void sclw_set_field_mode(vpp_display_format_t fmt)
{
	vppif_reg32_write(SCLR_TAR_DISP_FMT,fmt);
}

void sclw_set_fb_addr(U32 y_addr,U32 c_addr)
{
	DBGMSG("y_addr:0x%08x, c_addr:0x%08x\n",y_addr,c_addr);
//	if( (y_addr & 0x3f) || (c_addr & 0x3f) ){
//		DPRINT("[SCL] *E* addr should align 64\n");
//	}
	vppif_reg32_out(REG_SCLW_YSA,y_addr);
	vppif_reg32_out(REG_SCLW_CSA,c_addr);
}

void sclw_get_fb_addr(U32 *y_addr,U32 *c_addr)
{
	*y_addr = vppif_reg32_in(REG_SCLW_YSA);
	*c_addr = vppif_reg32_in(REG_SCLW_CSA);
	DBGMSG("y_addr:0x%08x, c_addr:0x%08x\n", *y_addr, *c_addr);
}

void sclw_set_fb_width(U32 width,U32 buf_width)
{
	vppif_reg32_write(SCLW_YPXLWID, width);
	vppif_reg32_write(SCLW_YBUFWID, buf_width);
	if( sclw_get_color_format() == VDO_COL_FMT_YUV444 ){
		vppif_reg32_write(SCLW_CPXLWID, width);
		vppif_reg32_write(SCLW_CBUFWID, buf_width * 2);
	}
	else {
		vppif_reg32_write(SCLW_CPXLWID, width / 2);
		vppif_reg32_write(SCLW_CBUFWID, buf_width);
	}
}

void sclw_get_fb_width(U32 *width,U32 *buf_width)
{
	*width = vppif_reg32_read(SCLW_YPXLWID);
	*buf_width = vppif_reg32_read(SCLW_YBUFWID);
}

void scl_set_V_scale_ptr(unsigned int init_val,unsigned int ret_val)
{
	unsigned int reg;

	reg = ((init_val) << 16) + (ret_val);
	vppif_reg32_out(REG_SCL_VPTR,reg);	

#ifdef WMT_FTBLK_SCL_VSCL_32
	/* init V scale registers */
	vppif_reg32_out(REG_SCL_VSCL_TB0,0x1);
	vppif_reg32_out(REG_SCL_VSCL_TB1,0x0);
	vppif_reg32_out(REG_SCL_VRES_TB,0x0);
	vppif_reg32_out(REG_SCL_VDIV_TB0,0x0);
	vppif_reg32_out(REG_SCL_VDIV_TB1,0x0);
	vppif_reg32_out(REG_SCL_VDIV_TB2,0x0);
	vppif_reg32_out(REG_SCL_VDIV_TB3,0x0);
	vppif_reg32_out(REG_SCL_VDIV_TB4,0x0);
	vppif_reg32_out(REG_SCL_VDIV_TB5,0x0);
	vppif_reg32_out(REG_SCL_VDIV_TB6,0x0);
	vppif_reg32_out(REG_SCL_VDIV_TB7,0x0);
#else
	/* init V scale registers */
	vppif_reg32_out(REG_SCL_VSCL_TB,0x1);
	vppif_reg32_out(REG_SCL_VRES_TB,0x0);
	vppif_reg32_out(REG_SCL_VDIV_TB0,0x0);
	vppif_reg32_out(REG_SCL_VDIV_TB1,0x0);
	vppif_reg32_out(REG_SCL_VDIV_TB2,0x0);
	vppif_reg32_out(REG_SCL_VDIV_TB3,0x0);
#endif

#if(WMT_SUB_PID >= WMT_PID_8710_B)
	vppif_reg32_out(REG_SCL_VDIV_TB8,0x0);
	vppif_reg32_out(REG_SCL_VDIV_TB9,0x0);
	vppif_reg32_out(REG_SCL_VDIV_TB10,0x0);
	vppif_reg32_out(REG_SCL_VDIV_TB11,0x0);
	vppif_reg32_out(REG_SCL_VDIV_TB12,0x0);
	vppif_reg32_out(REG_SCL_VDIV_TB13,0x0);
	vppif_reg32_out(REG_SCL_VDIV_TB14,0x0);
	vppif_reg32_out(REG_SCL_VDIV_TB15,0x0);
#endif

#ifdef WMT_FTBLK_SCL_BILINEAR
	vppif_reg32_write(SCL_BILINEAR_V,0);
#endif
}

void scl_set_V_scale_table(int no,int value)
{
	/* value 
 	3: The interpolation line data ((current line data + previous line data) /DIV) will be latched to next stage 
 	1: The current line data will be latched to next stage
 	0: The current line will be by held
 	*/
#ifdef WMT_FTBLK_SCL_VSCL_32
	int reg_offset = no / 16;
	int tab_offset = no % 16;

	vppif_reg32_write(REG_SCL_VSCL_TB0+ (4*reg_offset), (0x3 << (2*tab_offset)), (2*tab_offset), value);
#else
	vppif_reg32_write(REG_SCL_VSCL_TB, (0x3 << (no*2)), no*2, value);
#endif
}

void scl_set_V_rec_table(int no,int value)
{
	/* value 
	0: The current line data will be written back into line buffer
	1: The interpolation line data ((current line data + previous line data) /DIV) will be written back into line buffer
	*/
	vppif_reg32_write(REG_SCL_VRES_TB, (0x1 << no), no, value);
}

void scl_set_V_div_table(int no,int value)
{
#if(WMT_SUB_PID == WMT_PID_8710_A)
	int reg_offset = no / 4;
	int tab_offset = no % 4;

	vppif_reg32_write(REG_SCL_VDIV_TB0 + (4*reg_offset),(0xFF << (8*tab_offset)),(8*tab_offset),value);
#else
	unsigned int reg_tb;
	int reg_offset = no / 2;
	int tab_offset = no % 2;

	reg_tb = ( reg_offset < 8 )? REG_SCL_VDIV_TB0:REG_SCL_VDIV_TB8;
	reg_offset = (reg_offset < 8)? reg_offset:(reg_offset - 8);
	reg_tb += (4*reg_offset);
	vppif_reg32_write(reg_tb,(0x1FFF << (16*tab_offset)),(16*tab_offset),value);
#endif	
	
}

void scl_set_H_scale_ptr(unsigned int init_val,unsigned int ret_val)
{
	unsigned int reg;

	reg = ((init_val) << 16) + (ret_val);
	vppif_reg32_out(REG_SCL_HPTR,reg);	

	/* init V scale registers */
	vppif_reg32_out(REG_SCL_HSCL_TB0,0x1);
	vppif_reg32_out(REG_SCL_HSCL_TB1,0x0);
	vppif_reg32_out(REG_SCL_HRES_TB,0x0);
	vppif_reg32_out(REG_SCL_HDIV_TB0,0x0);
	vppif_reg32_out(REG_SCL_HDIV_TB1,0x0);
	vppif_reg32_out(REG_SCL_HDIV_TB2,0x0);
	vppif_reg32_out(REG_SCL_HDIV_TB3,0x0);
	vppif_reg32_out(REG_SCL_HDIV_TB4,0x0);
	vppif_reg32_out(REG_SCL_HDIV_TB5,0x0);
	vppif_reg32_out(REG_SCL_HDIV_TB6,0x0);
	vppif_reg32_out(REG_SCL_HDIV_TB7,0x0);
#if(WMT_SUB_PID >= WMT_PID_8710_B)
	vppif_reg32_out(REG_SCL_HDIV_TB8,0x0);
	vppif_reg32_out(REG_SCL_HDIV_TB9,0x0);
	vppif_reg32_out(REG_SCL_HDIV_TB10,0x0);
	vppif_reg32_out(REG_SCL_HDIV_TB11,0x0);
	vppif_reg32_out(REG_SCL_HDIV_TB12,0x0);
	vppif_reg32_out(REG_SCL_HDIV_TB13,0x0);
	vppif_reg32_out(REG_SCL_HDIV_TB14,0x0);
	vppif_reg32_out(REG_SCL_HDIV_TB15,0x0);
#endif
	vppif_reg32_write(SCL_BILINEAR_H,0);
}

void scl_set_H_scale_table(int no,int value)
{
	int reg_offset = no / 16;
	int tab_offset = no % 16;

	/* value 
 	3: The interpolation line data ((current line data + previous line data) /DIV) will be latched to next stage 
 	1: The current line data will be latched to next stage
 	0: The current line will be by held
 	*/
	vppif_reg32_write(REG_SCL_HSCL_TB0+ (4*reg_offset), (0x3 << (2*tab_offset)), (2*tab_offset), value);
}

void scl_set_H_rec_table(int no,int value)
{
	/* value 
	0: The current line data will be written back into line buffer
	1: The interpolation line data ((current line data + previous line data) /DIV) will be written back into line buffer
	*/
	vppif_reg32_write(REG_SCL_HRES_TB, (0x1 << no), no, value);
}

void scl_set_H_div_table(int no,int value)
{
#if(WMT_SUB_PID == WMT_PID_8710_A)
	int reg_offset = no / 4;
	int tab_offset = no % 4;

	vppif_reg32_write(REG_SCL_HDIV_TB0 + (4*reg_offset),(0xFF << (8*tab_offset)),(8*tab_offset),value);
#else
	unsigned int reg_tb;
	int reg_offset = no / 2;
	int tab_offset = no % 2;

	reg_tb = ( reg_offset < 8 )? REG_SCL_HDIV_TB0:REG_SCL_HDIV_TB8;
	reg_offset = (reg_offset < 8)? reg_offset:(reg_offset - 8);
	reg_tb += (4*reg_offset);
	vppif_reg32_write(reg_tb,(0x1FFF << (16*tab_offset)),(16*tab_offset),value);
#endif

}

#ifdef WMT_FTBLK_SCL_BILINEAR
static void scl_scale_bilinear(int src,int dst,int h)
{
	unsigned int tb0,tb1;
	unsigned int pos,index;
	int i;

	index = 1;
	tb0 = 0xc;
	tb1 = 0x0;
	for(i=2;i<src;i++){
		pos = src * index;
		pos = (pos / dst) + ((pos % dst)? 1:0);
		if( pos == i ){
			if(i<16)
				tb0 = tb0 + (0x3 << (i*2));
			else 
				tb1 = tb1 + (0x3 << ((i-16)*2));
			index++;
		}
	}

	index = 0;
	while( index == 0 ){
		if( src <= 16 )
			index = tb0 & (0x3 << ((src-1)*2));
		else
			index = tb1 & (0x3 << ((src-17)*2));
		if( index == 0 ){
			if( (tb0 & 0xc0000000) == 0 )
				tb1 = tb1 << 2;
			else 
				tb1 = (tb1 << 2) + 0x3;
			tb0 = tb0 << 2;
		}
	}

	if( h ){
		vppif_reg32_write(SCL_BILINEAR_H,1);
		vppif_reg32_out(REG_SCL_HSCL_TB0,tb0);
		vppif_reg32_out(REG_SCL_HSCL_TB1,tb1);
		vppif_reg32_out(REG_SCL_HPTR,src-1);
	}
	else {
		vppif_reg32_write(SCL_BILINEAR_V,1);
		vppif_reg32_out(REG_SCL_VSCL_TB0,tb0);
		vppif_reg32_out(REG_SCL_VSCL_TB1,tb1);
		vppif_reg32_out(REG_SCL_VPTR,src-1);
	}
}
#endif

#if 0
static void scl_set_RT_bilinear(int src,int dst,int htb)
{
	int i,index;
	int integer_pos = 0;
	int decimal_pos = 0;
	int threshold;
	int scl_tb = 0,div_tb = 0;
	int pos;

	pos = src * 100 / dst;
	threshold = ( (dst * 2) > src )? 25:50;
	for (i=0,index=0; i<src; i++){
		if( i == integer_pos ){
			if (decimal_pos <= threshold){
				scl_tb = 0x1;
				div_tb = 0x0;
			}
			else if (decimal_pos > threshold){
				scl_tb = 0x0;
				div_tb = 0x0;
			}
		}
		else if( i > integer_pos ){
			if(decimal_pos > threshold){
				scl_tb = 0x3;
				div_tb = 0x80;
			}
			else{
				scl_tb = 0x0;
				div_tb = 0x0;
			}
		}
		else if( i < integer_pos ){
			scl_tb = 0x0;
			div_tb = 0x0;
		}

		if( scl_tb != 0x0 ){
			index++;
			integer_pos = (index * pos) / 100;
			decimal_pos = (index * pos) % 100;
		}

		if( htb ){
			scl_set_H_rec_table(i,0);
			scl_set_H_scale_table(i,scl_tb);
			scl_set_H_div_table(i,div_tb);
		}
		else {
			scl_set_V_rec_table(i,0);
			scl_set_V_scale_table(i,scl_tb);
			scl_set_V_div_table(i,div_tb);
		}
	}
}
#endif

static void scl_set_RT_htb(int Src_Width, int Width)
{
	int A,B;
	int gcd;
	int i, j;
	int pre,cur;
	int index;
	int diff;

	gcd = vpp_get_gcd(Src_Width,Width);
	A = Width / gcd;
	B = Src_Width / gcd;

//	printk("H Src %d,Dst %d, (%d,%d)\n",Src_Width,Width,A,B);

	if ((Src_Width == Width) || (B>32)){
		scl_set_H_scale_ptr(0,0);
		return;
	}

	scl_set_H_scale_ptr(0,B-1);
	if( p_scl->scale_mode != VPP_SCALE_MODE_REC_TABLE ){
		return;
	}

	for(i=0,pre=0,index=0;i<A;i++){
		cur = (B*(i+1))/A;
		diff = cur - pre;
		pre = cur;
		if( diff == 1 ){
			scl_set_H_rec_table(index,0);	// first 0, other 1
			scl_set_H_scale_table(index,1);	// diff(1) 1, diff(>1) last 3,other 0
			scl_set_H_div_table(index,0);	// last 256/diff, other 0			
			index++;
		}
		else {
			/* 1st */
			scl_set_H_rec_table(index,0);	// first 0, other 1
			scl_set_H_scale_table(index,0);	// diff(1) 1, diff(>1) last 3,other 0
			scl_set_H_div_table(index,0);	// last 256/diff, other 0
			index++;

			/* middle */
			for(j=0;j<(diff-2);j++){
				scl_set_H_rec_table(index,1);	// first 0, other 1
				scl_set_H_scale_table(index,0);	// diff(1) 1, diff(>1) last 3,other 0
				scl_set_H_div_table(index,0);	// last 256/diff, other 0
				index++;
			}

			/* last */
			scl_set_H_rec_table(index,1);	// first 0, other 1
			scl_set_H_scale_table(index,3);	// diff(1) 1, diff(>1) last 3,other 0
			scl_set_H_div_table(index,(WMT_SCL_H_DIV_MAX/diff));	// last 256/diff, other 0
			index++;
		}
	}
}

static void scl_set_RT_vtb(int Src_Height, int Height)
{
	int A,B;
	int gcd;
	int i, j;
	int pre,cur;
	int index;
	int diff;

	gcd = vpp_get_gcd(Src_Height,Height);
	A = Height / gcd;
	B = Src_Height / gcd;
#ifdef WMT_FTBLK_SCL_VSCL_32
	if ((Src_Height == Height) || (B>32)){
#else
	if ((Src_Height == Height) || (B>16)){
#endif
		scl_set_V_scale_ptr(0,0);
		return;
	}

	scl_set_V_scale_ptr(0,B-1);
	if( p_scl->scale_mode != VPP_SCALE_MODE_REC_TABLE ){
		return;
	}

	for(i=0,pre=0,index=0;i<A;i++){
		cur = (B*(i+1))/A;
		diff = cur - pre;
		pre = cur;
		if( diff == 1 ){
			scl_set_V_rec_table(index,0);	// first 0, other 1
			scl_set_V_scale_table(index,1);	// diff(1) 1, diff(>1) last 3,other 0
			scl_set_V_div_table(index,0);	// last 256/diff, other 0			
			index++;
		}
		else {
			/* 1st */
			scl_set_V_rec_table(index,0);	// first 0, other 1
			scl_set_V_scale_table(index,0);	// diff(1) 1, diff(>1) last 3,other 0
			scl_set_V_div_table(index,0);	// last 256/diff, other 0
			index++;

			/* middle */
			for(j=0;j<(diff-2);j++){
				scl_set_V_rec_table(index,1);	// first 0, other 1
				scl_set_V_scale_table(index,0);	// diff(1) 1, diff(>1) last 3,other 0
				scl_set_V_div_table(index,0);	// last 256/diff, other 0
				index++;
			}

			/* last */
			scl_set_V_rec_table(index,1);	// first 0, other 1
			scl_set_V_scale_table(index,3);	// diff(1) 1, diff(>1) last 3,other 0
			scl_set_V_div_table(index,(WMT_SCL_V_DIV_MAX/diff));	// last 256/diff, other 0
			index++;
		}
	}
}

static void scl_set_scale_RT(unsigned int src,unsigned int dst,int horizontal)
{
//	DBGMSG("scale RT(s %d,d %d,is H %d)\n",src,dst,horizontal);

	if( horizontal ){
		scl_set_RT_htb(src,dst);
	}
	else {
		scl_set_RT_vtb(src,dst);		
	}
}

static void scl_set_scale_PP(unsigned int src,unsigned int dst,int horizontal)
{
	int gcd;

//	DBGMSG("scale PP(s %d,d %d,is H %d)\n",src,dst,horizontal);
	
	//gcd = scl_get_gcd(src,dst);
	gcd = 1;
	src /= gcd;
	dst /= gcd;

	if( horizontal ){
		scl_set_H_scale(dst,src);
	}
	else {
		scl_set_V_scale(dst,src);
	}
}

void scl_set_scale(unsigned int SRC_W,unsigned int SRC_H,unsigned int DST_W,unsigned int DST_H)
{
	int h_scale_up;
	int v_scale_up;

	DBGMSG("[SCL] src(%dx%d),dst(%dx%d)\n",SRC_W,SRC_H,DST_W,DST_H);

	h_scale_up = ( DST_W > SRC_W )? 1:0;
	v_scale_up = ( DST_H > SRC_H )? 1:0;

	if( ((DST_W / SRC_W) >= 32) || ((DST_W / SRC_W) < 1/32)  ){
		DBGMSG("*W* SCL H scale rate invalid\n");
	}

	if( ((DST_H / SRC_H) >= 32) || ((DST_H / SRC_H) < 1/32)  ){
		DBGMSG("*W* SCL V scale rate invalid\n");
	}

#ifndef WMT_FTBLK_SCL_VSCL_32
	if( (vppif_reg32_read(SCL_TG_GOVWTG_ENABLE) == 0) && ((DST_H / SRC_H) < 1/16)  ){
		DBGMSG("*W* SCL V scale rate invalid2\n");
	}
#endif	

//	DBGMSG("scale H %d,V %d\n",h_scale_up,v_scale_up);

	sclr_set_mif2_enable(VPP_FLAG_DISABLE);
	scl_set_scale_PP(SRC_W,DST_W,1);
	if( h_scale_up ){ // max 32
		vppif_reg32_out(REG_SCL_HSCL_TB0,3);
		vppif_reg32_out(REG_SCL_HPTR,0);
		vppif_reg32_write(SCL_BILINEAR_H,0);
	}
	else { // min 1/32
		vppif_reg32_out(REG_SCL_HSCL_TB0,1);
		scl_set_scale_RT(SRC_W,DST_W,1);
	}

	scl_set_scale_PP(SRC_H,DST_H,0);
	if( v_scale_up ){ // max 32	
		vppif_reg32_out(REG_SCL_VSCL_TB,3);
		vppif_reg32_out(REG_SCL_VPTR,0);
#ifdef WMT_FTBLK_SCL_BILINEAR
		vppif_reg32_write(SCL_BILINEAR_V,0);
#endif
	}
	else { // min 1/16
		scl_set_scale_RT(SRC_H,DST_H,0);
		// mif2 enable in real quality mode,only mif1 in SCL recursive mode 
		if( vppif_reg32_read(SCL_TG_GOVWTG_ENABLE) && (vppif_reg32_read(SCL_SCLDW_METHOD)==0)){
			sclr_set_mif2_enable(VPP_FLAG_ENABLE);
		}
	}
	scl_set_scale_enable(v_scale_up, h_scale_up);
}

void sclr_set_framebuffer(vdo_framebuf_t *inbuf)
{
#if 0
	if( (inbuf->col_fmt >= VDO_COL_FMT_ARGB) && (inbuf->bpp != 32)){
	    if( vpp_check_dbg_level(VPP_DBGLVL_SCALE) ){
    		DPRINT("[SCLR] *W* only support ARGB 32bits\n");
        }
	}
#endif
	sclr_set_color_format(inbuf->col_fmt);
	sclr_set_crop(inbuf->h_crop, inbuf->v_crop);
	sclr_set_width(inbuf->img_w, inbuf->fb_w);
	sclr_set_fb_addr(inbuf->y_addr, inbuf->c_addr);
	sclr_set_field_mode(vpp_get_fb_field(inbuf));
}

void sclw_set_framebuffer(vdo_framebuf_t *fb)
{
	vdo_framebuf_t *scl_fb;	// source
	vpp_clock_t timing;
	unsigned int pixel_clock;
	int h,v;

	sclw_set_fb_addr(fb->y_addr, fb->c_addr);
	sclw_set_color_format(fb->col_fmt);
	sclw_set_fb_width(fb->img_w, fb->fb_w);
	sclw_set_field_mode(vpp_get_fb_field(fb));

	scl_fb = &p_scl->fb_p->fb;
	scl_set_scale(scl_fb->img_w, scl_fb->img_h, fb->img_w, fb->img_h);
#ifdef WMT_FTBLK_SCL_BILINEAR_TRUE
	{
		int rec_h,rec_v;

		h = rec_h = 0;
		if( scl_fb->img_w > fb->img_w ){ // scale down
			switch(p_scl->scale_mode){
				case VPP_SCALE_MODE_ADAPTIVE:
					if( (fb->img_w * 2) > scl_fb->img_w ){	// 1 > mode(3) > 1/2
						h = 1; // bilinear mode
					}
					else {
						rec_h = 1;	// recursive mode
					}
					break;
				case VPP_SCALE_MODE_BILINEAR:
					h = 1; // bilinear mode
					break;
				case VPP_SCALE_MODE_RECURSIVE:
					rec_h = 1;	// recursive mode
					break;
				default:
					break;
			}
		}
			
		v = rec_v = 0;
		if( scl_fb->img_h > fb->img_h ){ // scale down
			switch(p_scl->scale_mode){
				case VPP_SCALE_MODE_ADAPTIVE:
					if( (fb->img_h * 2) > scl_fb->img_h ){	// 1 > mode(3) > 1/2
						v = 1; // bilinear mode
					}
					else {
						rec_v = 1;	// recursive mode
					}
					break;
				case VPP_SCALE_MODE_BILINEAR:
					v = 1; // bilinear mode
					break;
				case VPP_SCALE_MODE_RECURSIVE:
					rec_v = 1;	// recursive mode
					break;
				default:
					break;
			}
		}

		if( p_scl->scale_mode != VPP_SCALE_MODE_REC_TABLE ){
			vppif_reg32_out(REG_SCL_HRES_TB,0);
			vppif_reg32_out(REG_SCL_VRES_TB,0);
		}
		vppif_reg32_write(SCL_BILINEAR_H,h);
		vppif_reg32_write(SCL_BILINEAR_V,v);
		vppif_reg32_write(SCL_RECURSIVE_H,rec_h);
		vppif_reg32_write(SCL_RECURSIVE_V,rec_v);

		// patch for vertical bilinear mode
		if( v ){
			vppif_reg32_write(SCL_VXWIDTH,vppif_reg32_read(SCL_VXWIDTH)-1);
			vppif_reg32_write(SCL_DST_VXWIDTH,vppif_reg32_read(SCL_VXWIDTH));
		}
		sclr_set_mif2_enable((v)? VPP_FLAG_ENABLE:VPP_FLAG_DISABLE);
		
#ifdef CONFIG_VPP_SCALE_TABLE_FREE
//		DPRINT("[SCL] scl w %d,h %d, sclw w %d,h %d\n",scl_fb->img_w,scl_fb->img_h,fb->img_w,fb->img_h);
#endif
//		DPRINT("[SCL] bil h %d,v %d, rec h %d,v %d\n",h,v,rec_h,rec_v);
	}
#endif
	
	p_scl->fb_p->set_csc(p_scl->fb_p->csc_mode);
			
	// scl TG
	timing.total_pixel_of_line = (fb->img_w > scl_fb->img_w)?fb->img_w:scl_fb->img_w;	
	if( v ){
		timing.total_line_of_frame = fb->img_h;
	}
	else {
		timing.total_line_of_frame = (fb->img_h> scl_fb->img_h)?fb->img_h:scl_fb->img_h;
	}

	timing.begin_pixel_of_active = 60;
	timing.end_pixel_of_active = timing.total_pixel_of_line + 60;
	timing.total_pixel_of_line = timing.total_pixel_of_line + 120;
	timing.begin_line_of_active = 8;
	timing.end_line_of_active = timing.total_line_of_frame + 8;
	timing.total_line_of_frame = timing.total_line_of_frame + 16;
	timing.line_number_between_VBIS_VBIE = 4;
	timing.line_number_between_PVBI_VBIS = 1;
	pixel_clock = timing.total_pixel_of_line * timing.total_line_of_frame * p_scl->fb_p->framerate;
	scl_set_timing(&timing,pixel_clock);
}

void scl_init(void *base)
{
	vpu_mod_t *mod_p;
	vpp_fb_base_t *fb_p;

	mod_p = (vpu_mod_t *) base;
	fb_p = mod_p->fb_p;

	scl_set_reg_level(VPP_REG_LEVEL_1);
	scl_set_tg_enable(VPP_FLAG_DISABLE);
	scl_set_enable(VPP_FLAG_DISABLE);
	scl_set_int_enable(VPP_FLAG_DISABLE, VPP_INT_ALL);
	sclr_set_mif_enable(VPP_FLAG_DISABLE);
	sclr_set_mif2_enable(VPP_FLAG_DISABLE);
	sclr_set_colorbar(VPP_FLAG_DISABLE,0,0);

	//enable
	scl_set_int_enable(VPP_FLAG_ENABLE, mod_p->int_catch);
	scl_set_watchdog(fb_p->wait_ready);
	scl_set_csc_mode(fb_p->csc_mode);
	sclr_set_media_format(fb_p->media_fmt);
	sclr_set_threshold(0xf);

	sclr_set_mif_enable(VPP_FLAG_ENABLE);
	sclr_set_mif2_enable(VPP_FLAG_ENABLE);
	scl_set_enable(VPP_FLAG_ENABLE);
	scl_set_reg_update(VPP_FLAG_ENABLE);
	scl_set_tg_enable(VPP_FLAG_DISABLE);
}

void sclw_init(void *base)
{
	sclw_set_mif_enable(VPP_FLAG_DISABLE);
	sclw_set_fb_width(VPP_HD_DISP_RESX, VPP_HD_MAX_RESX);
//	vppif_reg32_write(SCL_SCLDW_METHOD,0x1);	// drop line enable
}

void scl_vpu_set_framebuffer(vdo_framebuf_t *fb)
{
	static unsigned int img_w,img_h,vis_w,vis_h;
	unsigned int SRC_W,SRC_H,DST_W,DST_H;

	if( (img_w != fb->img_w) || (img_h != fb->img_h) 
		|| (vis_w != p_vpu->resx_visual) || (vis_h != p_vpu->resy_visual) ){
		SRC_W = img_w = fb->img_w;
		SRC_H = img_h = fb->img_h;
		DST_W = vis_w = p_vpu->resx_visual;
		DST_H = vis_h = p_vpu->resy_visual;

		vpp_check_scale_ratio(&SRC_W,&DST_W,32,32);
#ifdef WMT_FTBLK_SCL_VSCL_32		
		vpp_check_scale_ratio(&SRC_H,&DST_H,32,31);
#else
		vpp_check_scale_ratio(&SRC_H,&DST_H,32,16);
#endif
		p_vpu->resx_virtual_scale = SRC_W;
		p_vpu->resy_virtual_scale = SRC_H;
		p_vpu->resx_visual_scale = DST_W;
		p_vpu->resy_visual_scale = DST_H;
	}
	else {
		SRC_W = p_vpu->resx_virtual_scale;
		SRC_H = p_vpu->resy_virtual_scale;
		DST_W = p_vpu->resx_visual_scale;
		DST_H = p_vpu->resy_visual_scale;
	}
	sclw_set_fb_width(DST_W,fb->fb_w);
	sclr_set_framebuffer(fb);
	sclr_set_width(SRC_W, fb->fb_w);
	scl_set_scale(SRC_W,SRC_H,DST_W,DST_H);
	scl_set_csc_mode(p_vpu->fb_p->csc_mode);
}

void scl_vpu_proc_view(int read,vdo_view_t *view)
{
	vdo_framebuf_t *fb;

	fb = &p_vpu->fb_p->fb;
	if( read ){
		view->resx_src = fb->fb_w;
		view->resy_src = fb->fb_h;
		view->resx_virtual = fb->img_w;
		view->resy_virtual = fb->img_h;
		view->resx_visual = p_vpu->resx_visual;
		view->resy_visual = p_vpu->resy_visual;
		view->posx = p_vpu->posx;
		view->posy = p_vpu->posy;
		view->offsetx = fb->h_crop;
		view->offsety = fb->v_crop;
	}
	else {
		vpp_set_video_scale(view);
	}
}

int scl_scale_gov_path_in;
unsigned int scl_scale_govw_mif_en = 0;
int scl_scale_complete;
#ifdef __KERNEL__
static struct work_struct scl_proc_scale_wq;
DECLARE_WAIT_QUEUE_HEAD(scl_proc_scale_event);
#define SCL_COMPLETE_INT VPP_INT_SCL_PVBI

static void scl_proc_scale_complete_work(struct work_struct *work)
#else
static void scl_proc_scale_complete_work(void)
#endif
{
//	DPRINT("[SCL] scl_proc_scale_complete_work\n");

#if 0	// patch for memory guard issue
	vppm_set_int_enable(VPP_FLAG_ENABLE,VPP_INT_SCL_VBIS);
	vpp_irqproc_work(VPP_INT_SCL_VBIS,0,0,1);
	scl_set_tg_enable(VPP_FLAG_DISABLE);
	vppm_set_int_enable(VPP_FLAG_DISABLE,VPP_INT_SCL_VBIS);

	scl_set_tg_enable(VPP_FLAG_ENABLE);		// patch for memory guard issue
	scl_set_tg_enable(VPP_FLAG_DISABLE);	// patch for memory guard issue
#endif	
#ifndef WMT_FTBLK_VPU
	scl_set_timing_master(VPP_MOD_GOVW);
	p_vpu->fb_p->set_framebuf(&p_vpu->fb_p->fb);
	g_vpp.govw_skip_frame = 1;
#endif
	scl_scale_complete = 1;
#ifdef __KERNEL__
#ifdef CONFIG_SCL_DIRECT_PATH_DEBUG
{
	extern void vpp_scl_fb_cal_tmr(int begin);
	vpp_scl_fb_cal_tmr(0);
}
#endif
	wake_up_interruptible(&scl_proc_scale_event);
#endif
}

int scl_proc_scale_complete(void *arg)
{
#if 0
	unsigned int reg;
	int ret = 0;
#endif
//	DPRINT("[SCL] scl_proc_scale_complete\n");

	while( vppif_reg32_read(SCL_INTSTS_TGERR) ){
		unsigned int rd_cyc;
		
		// add rd_cyc to retry
		rd_cyc = vppif_reg32_read(SCL_TG_RDCYC);
		if( rd_cyc > 0xFD ){
			DPRINT("[VPU] *E* scale error\n");
			goto scale_end;
		}
		if( vpp_check_dbg_level(VPP_DBGLVL_SCALE) ){
			DPRINT("[VPU] scale retry, rcyc 0x%x\n",rd_cyc);
		}
		rd_cyc = rd_cyc + (0xFF - rd_cyc) / 2;
		vppif_reg32_write(SCL_TG_RDCYC,rd_cyc);
		vppif_reg32_out(REG_SCL_TG_STS+0x0,BIT0);
		vppif_reg32_out(REG_SCLW_FF_CTL,0x10101);

		DPRINT("[SCL] *E* scale error\n");
	}
#if 0
	if( (reg = (vppif_reg32_in(REG_SCLR_FIFO_CTL) & ~0xF)) ){
		vppif_reg32_out(REG_SCLR_FIFO_CTL,0x300);
		DPRINT("[SCL] *E* SCLR 0x%x\n",reg);
		ret = 1;
	}
	if( (reg = vppif_reg32_in(REG_SCLW_FF_CTL)) ){
		vppif_reg32_out(REG_SCLW_FF_CTL,0x10101);
		DPRINT("[SCL] *E* SCLW 0x%x\n",reg);
		ret = 1;
	}
	if( ret ){
		scl_set_tg_enable(VPP_FLAG_ENABLE);
		return 1;
	}
#endif
scale_end:	
	vppm_set_int_enable(VPP_FLAG_DISABLE,SCL_COMPLETE_INT);
	sclw_set_mif_enable(VPP_FLAG_DISABLE);

#ifdef __KERNEL__
	INIT_WORK(&scl_proc_scale_wq, scl_proc_scale_complete_work);
	schedule_work(&scl_proc_scale_wq);
#else
	scl_proc_scale_complete_work();
#endif
	return 0;
}

int scl_proc_scale_finish(void)
{
#ifdef __KERNEL__
	int ret;

//	DPRINT("[SCL] scl_proc_scale_finish\n");

    ret = wait_event_interruptible_timeout(scl_proc_scale_event, (scl_scale_complete != 0), 3*HZ);
	if( ret == 0 ){ // timeout
		DPRINT("[VPU] *E* wait scale timeout\n");
		return -1;
	}
#endif
	return 0;
}

#define CONFIG_VPP_CHECK_SCL_STATUS
int scl_do_recursive_scale(vdo_framebuf_t *src_fb,vdo_framebuf_t *dst_fb)
{
	int ret = 0;

	scl_set_timing_master(VPP_MOD_SCL);

	p_scl->fb_p->fb = *src_fb;
	p_scl->fb_p->set_framebuf(src_fb);

#ifdef PATCH_SCL_SCALEDN_GARBAGE_EDGE
	// The ¡§start address¡¨  and ¡§start address + Horizontal active size¡¨ have to be 
	// avoiding the following offset failed address offset : ( 34 , 36 , 38 ) + n*64
{
	unsigned int beg,end;
	unsigned int temp;

	beg = dst_fb->y_addr;
	end = dst_fb->y_addr + dst_fb->img_w - 1;

	temp = beg % 64;
	if( (temp >=34) && (temp <=38) ){
		temp = 40 - temp;
		dst_fb->y_addr += temp;
		dst_fb->c_addr += temp * ((dst_fb->col_fmt==VDO_COL_FMT_YUV444)?2:1);
		beg += temp;
	}

	temp = end % 64;
	if( (temp >=34) && (temp <=38) ){
		end -= (temp - 32);
		dst_fb->img_w = end - beg;
	}
}
#endif

	p_sclw->fb_p->fb = *dst_fb;		
	p_sclw->fb_p->set_framebuf(dst_fb);

	// scale process
#if 1
// #ifdef SCL_ONESHOT_ENABLE
	vppif_reg32_write(SCL_ONESHOT_ENABLE,1);
#endif
	sclw_set_mif_enable(VPP_FLAG_ENABLE);
	scl_set_tg_enable(VPP_FLAG_ENABLE);
#ifdef CONFIG_VPP_CHECK_SCL_STATUS	
	vppif_reg32_out(REG_SCL_TG_STS+0x0,BIT0);
	vppif_reg32_out(REG_SCLW_FF_CTL,0x10101);
#endif	
	scl_scale_complete = 0;
	vppm_set_int_enable(VPP_FLAG_ENABLE,SCL_COMPLETE_INT);
	if( p_scl->scale_sync ){
		vpp_irqproc_work(SCL_COMPLETE_INT,scl_proc_scale_complete,0,1);
		scl_proc_scale_finish();
	}
	else {
		vpp_irqproc_work(SCL_COMPLETE_INT,scl_proc_scale_complete,0,0);
	}
	return ret;
}

int scl_proc_scale(vdo_framebuf_t *src_fb,vdo_framebuf_t *dst_fb)
{
	int ret = 0;

	if( dst_fb->col_fmt == VDO_COL_FMT_YUV420 ){
		int size;
		unsigned int buf;
		vdo_framebuf_t dfb;

		dfb = *dst_fb;	// backup dst fb
		
		// alloc memory
		size = dst_fb->img_w * dst_fb->img_h + 64;
		if( !(buf = (unsigned int) kmalloc(size,GFP_KERNEL)) ){
			DPRINT("[SCL] *E* malloc fail %d\n",size);
			return -1;
		}

		if( buf % 64 ){
			buf = buf + (64 - (buf % 64));
		}

		// scale for Y
		dst_fb->c_addr = buf;
		if( (ret = scl_do_recursive_scale(src_fb,dst_fb)) == 0 ){
			// V 1/2 scale for C
			dst_fb->y_addr = buf;
			dst_fb->c_addr = dfb.c_addr;
			dst_fb->img_h = dfb.img_h / 2;
			ret = scl_do_recursive_scale(src_fb,dst_fb);
		}
		kfree((void *)buf);
		*dst_fb = dfb;	// restore dst fb
	}
	else {
		ret = scl_do_recursive_scale(src_fb,dst_fb);
	}
	return ret;
}

#ifdef CONFIG_PM
static unsigned int *scl_pm_bk2;
static unsigned int scl_pm_enable,scl_pm_tg;
static unsigned int scl_pm_r_mif1,scl_pm_r_mif2,scl_pm_w_mif;
void scl_suspend(int sts)
{
	switch( sts ){
		case 0:	// disable module
			scl_pm_enable = vppif_reg32_read(SCL_ALU_ENABLE);
			vppif_reg32_write(SCL_ALU_ENABLE,0);
			scl_pm_r_mif1 = vppif_reg32_read(SCLR_MIF_ENABLE);
			scl_pm_r_mif2 = vppif_reg32_read(SCLR_MIF2_ENABLE);
			vppif_reg32_write(SCLR_MIF_ENABLE,0);
			vppif_reg32_write(SCLR_MIF2_ENABLE,0);
			scl_pm_w_mif = vppif_reg32_read(SCLW_MIF_ENABLE);
			vppif_reg32_write(SCLW_MIF_ENABLE,0);
			break;
		case 1: // disable tg
			scl_pm_tg = vppif_reg32_read(SCL_TG_ENABLE);
			vppif_reg32_write(SCL_TG_ENABLE,0);
			break;
		case 2:	// backup register
			p_scl->reg_bk = vpp_backup_reg(REG_SCL_BASE1_BEGIN,(REG_SCL_BASE1_END-REG_SCL_BASE1_BEGIN));
			scl_pm_bk2 = vpp_backup_reg(REG_SCL_BASE2_BEGIN,(REG_SCL_BASE2_END-REG_SCL_BASE2_BEGIN));
			break;
		default:
			break;
	}
}

void scl_resume(int sts)
{
	switch( sts ){
		case 0:	// restore register
			vpp_restore_reg(REG_SCL_BASE1_BEGIN,(REG_SCL_BASE1_END-REG_SCL_BASE1_BEGIN),p_scl->reg_bk);
			vpp_restore_reg(REG_SCL_BASE2_BEGIN,(REG_SCL_BASE2_END-REG_SCL_BASE2_BEGIN),scl_pm_bk2);
			p_scl->reg_bk = 0;
			scl_pm_bk2 = 0;
			break;
		case 1:	// enable module
			vppif_reg32_write(SCLW_MIF_ENABLE,scl_pm_w_mif);
			vppif_reg32_write(SCLR_MIF_ENABLE,scl_pm_r_mif1);
			vppif_reg32_write(SCLR_MIF2_ENABLE,scl_pm_r_mif2);
			vppif_reg32_write(SCL_ALU_ENABLE,scl_pm_enable);
			break;
		case 2: // enable tg
			vppif_reg32_write(SCL_TG_ENABLE,scl_pm_tg);
			break;
		default:
			break;
	}
}
#else
#define scl_suspend NULL
#define scl_resume NULL
#endif

void scl_vpu_init(void *base)
{
	vpu_mod_t *mod_p;

	mod_p = (vpu_mod_t *) base;

	sclr_set_framebuffer(&mod_p->fb_p->fb);
	scl_set_timing_master(VPP_MOD_GOVW);	
}

int scl_mod_init(void)
{
	vpp_fb_base_t *mod_fb_p;
	vdo_framebuf_t *fb_p;

	/* -------------------- SCL module -------------------- */
	{
		scl_mod_t *scl_mod_p;
		
		scl_mod_p = (scl_mod_t *) vpp_mod_register(VPP_MOD_SCL,sizeof(scl_mod_t),VPP_MOD_FLAG_FRAMEBUF);
		if( !scl_mod_p ){
			DPRINT("*E* SCL module register fail\n");
			return -1;
		}

		/* module member variable */
		scl_mod_p->int_catch = VPP_INT_NULL;
		scl_mod_p->scale_mode = VPP_SCALE_MODE_RECURSIVE;

		/* module member function */
		scl_mod_p->init = scl_init;
		scl_mod_p->set_enable = scl_set_enable;
		scl_mod_p->set_colorbar = sclr_set_colorbar;
		scl_mod_p->dump_reg = scl_reg_dump;
		scl_mod_p->get_sts = scl_get_int_status;
		scl_mod_p->clr_sts = scl_clean_int_status;
		scl_mod_p->scale = scl_proc_scale;
		scl_mod_p->scale_finish = scl_proc_scale_finish;
		scl_mod_p->suspend = scl_suspend;
		scl_mod_p->resume = scl_resume;

		/* module frame buffer variable */
		mod_fb_p = scl_mod_p->fb_p;
		fb_p = &mod_fb_p->fb;

		fb_p->y_addr = 0;
		fb_p->c_addr = 0;
		fb_p->col_fmt = VDO_COL_FMT_YUV422H;
		fb_p->img_w = VPP_HD_DISP_RESX;
		fb_p->img_h = VPP_HD_DISP_RESY;
		fb_p->fb_w = VPP_HD_MAX_RESX;
		fb_p->fb_h = VPP_HD_MAX_RESY;
		fb_p->h_crop = 0;
		fb_p->v_crop = 0;

		/* module frame buffer member function */
		mod_fb_p->csc_mode = VPP_CSC_RGB2YUV_JFIF_0_255;
		mod_fb_p->set_framebuf = sclr_set_framebuffer;
		mod_fb_p->set_addr = sclr_set_fb_addr;
		mod_fb_p->get_addr = sclr_get_fb_addr;
		mod_fb_p->set_csc = scl_set_csc_mode;
		mod_fb_p->framerate = 0x7fffffff;
		mod_fb_p->wait_ready = 0xffffffff;
		mod_fb_p->capability = BIT(VDO_COL_FMT_YUV420) | BIT(VDO_COL_FMT_YUV422H) | BIT(VDO_COL_FMT_YUV444) 
			| BIT(VDO_COL_FMT_ARGB) | BIT(VDO_COL_FMT_RGB_565) | VPP_FB_FLAG_CSC | VPP_FB_FLAG_FIELD;

		p_scl = scl_mod_p;
	}
	
	/* -------------------- SCLW module -------------------- */
	{
		sclw_mod_t *sclw_mod_p;
		
		sclw_mod_p = (sclw_mod_t *) vpp_mod_register(VPP_MOD_SCLW,sizeof(sclw_mod_t),VPP_MOD_FLAG_FRAMEBUF);
		if( !sclw_mod_p ){
			DPRINT("*E* SCLW module register fail\n");
			return -1;
		}

		/* module member variable */
		sclw_mod_p->int_catch = VPP_INT_NULL;

		/* module member function */
		sclw_mod_p->init = sclw_init;
		sclw_mod_p->set_enable = sclw_set_mif_enable;

		/* module frame buffer */
		mod_fb_p = sclw_mod_p->fb_p;
		fb_p = &mod_fb_p->fb;

		fb_p->y_addr = 0;
		fb_p->c_addr = 0;
		fb_p->col_fmt = VDO_COL_FMT_YUV422H;
		fb_p->img_w = VPP_HD_DISP_RESX;
		fb_p->img_h = VPP_HD_DISP_RESY;
		fb_p->fb_w = VPP_HD_MAX_RESX;
		fb_p->fb_h = VPP_HD_MAX_RESY;
		fb_p->h_crop = 0;
		fb_p->v_crop = 0;

		/* module frame buffer member function */
		mod_fb_p->csc_mode = VPP_CSC_RGB2YUV_JFIF_0_255;
		mod_fb_p->set_framebuf = sclw_set_framebuffer;
		mod_fb_p->set_addr = sclw_set_fb_addr;
		mod_fb_p->get_addr = sclw_get_fb_addr;
		mod_fb_p->set_csc = scl_set_csc_mode;
		mod_fb_p->wait_ready = 0xffffffff;
		mod_fb_p->capability = BIT(VDO_COL_FMT_YUV422H) | BIT(VDO_COL_FMT_YUV444) | BIT(VDO_COL_FMT_ARGB)
							| BIT(VDO_COL_FMT_RGB_565) | VPP_FB_FLAG_CSC | BIT(VDO_COL_FMT_YUV420);

		p_sclw = sclw_mod_p;
	}

#ifndef WMT_FTBLK_VPU
	/* -------------------- VPU module -------------------- */
	{
		vpu_mod_t *vpu_mod_p;
		
		vpu_mod_p = (vpu_mod_t *) vpp_mod_register(VPP_MOD_VPU,sizeof(vpu_mod_t),VPP_MOD_FLAG_FRAMEBUF);
		if( !vpu_mod_p ){
			DPRINT("*E* VPU module register fail\n");
			return -1;
		}

		/* module member variable */
		vpu_mod_p->int_catch = VPP_INT_NULL;
		vpu_mod_p->resx_visual = VPP_HD_DISP_RESX;
		vpu_mod_p->resy_visual = VPP_HD_DISP_RESY;
		vpu_mod_p->posx = 0;
		vpu_mod_p->posy = 0;

		/* module member function */
		vpu_mod_p->init = scl_vpu_init;
		vpu_mod_p->set_enable = sclr_set_mif_enable;
		vpu_mod_p->set_colorbar = sclr_set_colorbar;

		/* module frame buffer */
		mod_fb_p = vpu_mod_p->fb_p;
		fb_p = &mod_fb_p->fb;
		
		fb_p->y_addr = 0;
		fb_p->c_addr = 0;
		fb_p->col_fmt = VDO_COL_FMT_YUV422H;
		fb_p->img_w = VPP_HD_DISP_RESX;
		fb_p->img_h = VPP_HD_DISP_RESY;
		fb_p->fb_w = VPP_HD_MAX_RESX;
		fb_p->fb_h = VPP_HD_MAX_RESY;
		fb_p->h_crop = 0;
		fb_p->v_crop = 0;

		/* module frame buffer member function */
		mod_fb_p->csc_mode = VPP_CSC_RGB2YUV_JFIF_0_255;
		mod_fb_p->set_framebuf = scl_vpu_set_framebuffer;
		mod_fb_p->set_addr = sclr_set_fb_addr;
		mod_fb_p->get_addr = sclr_get_fb_addr;
		mod_fb_p->set_csc = scl_set_csc_mode;
		mod_fb_p->fn_view = scl_vpu_proc_view;
		mod_fb_p->wait_ready = 0x1fff;
		mod_fb_p->capability = BIT(VDO_COL_FMT_YUV420) | BIT(VDO_COL_FMT_YUV422H) | BIT(VDO_COL_FMT_YUV444) 
							| BIT(VDO_COL_FMT_ARGB) | VPP_FB_FLAG_CSC | VPP_FB_FLAG_FIELD;

		p_vpu = vpu_mod_p;
	}
#endif
	return 0;
}
module_init(scl_mod_init);
#endif				//WMT_FTBLK_SCL
