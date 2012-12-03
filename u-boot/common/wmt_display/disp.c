/*++ 
 * linux/drivers/video/wmt/disp.c
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

/*
 * ChangeLog
 *
 * 2010-08-05  Sam Shen
 *     * Add License declaration and ChangeLog
 */

#define DISP_C
// #define DEBUG
/*----------------------- DEPENDENCE -----------------------------------------*/
#include "disp.h"

#ifdef WMT_FTBLK_DISP
/*----------------------- PRIVATE MACRO --------------------------------------*/

/*----------------------- PRIVATE CONSTANTS ----------------------------------*/
/* #define DISP_XXXX    1     *//*Example*/

/*----------------------- PRIVATE TYPE  --------------------------------------*/
/* typedef  xxxx disp_xxx_t; *//*Example*/

/*----------EXPORTED PRIVATE VARIABLES are defined in disp.h  -------------*/
/*----------------------- INTERNAL PRIVATE VARIABLES - -----------------------*/
/* int  disp_xxx;        *//*Example*/

/*--------------------- INTERNAL PRIVATE FUNCTIONS ---------------------------*/
/* void disp_xxx(void); *//*Example*/

/*----------------------- Function Body --------------------------------------*/
void disp_reg_dump(void)
{
	DPRINT("========== DISP register dump ==========\n");
	vpp_reg_dump(REG_DISP_BASE1_BEGIN,REG_DISP_BASE1_END-REG_DISP_BASE1_BEGIN);
	vpp_reg_dump(REG_DISP_BASE2_BEGIN,REG_DISP_BASE2_END-REG_DISP_BASE2_BEGIN);

	DPRINT("---------- DISP CTRL ----------\n");
	DPRINT("Enable %d, EXTTV Enable %d\n",vppif_reg32_read(DISP_EN),vppif_reg32_read(DISP_EXTTV_EN));
	DPRINT("TV sys %s, line sel %d\n",(vppif_reg32_read(DISP_TVSYS_NTSC))?"NTSC":"PAL",(vppif_reg32_read(DISP_LINE_SEL_525))?525:625);
	DPRINT("COLORBAR %d, Dot Crawl %d,Clamp %d\n",vppif_reg32_read(DISP_COLORBAR_EN),vppif_reg32_read(DISP_DOT_CRAWL),
		vppif_reg32_read(DISP_CLAMP_EN));
#ifdef REG_DISP_DAC_INV_TVCLK
	DPRINT("inv clken %d\n",vppif_reg32_read(DISP_INVTVCLK_EN));
#endif
	DPRINT("DAC PwrDn A:%d, B:%d, C:%d\n",vppif_reg32_read(DISP_DAC_B_PWRDN),vppif_reg32_read(DISP_DAC_C_PWRDN),vppif_reg32_read(DISP_DAC_D_PWRDN));

	DPRINT("---------- DISP Timing ----------\n");
	DPRINT("H total %d, beg %d, end %d\n",vppif_reg32_read(DISP_TG_TOTAL_PIXEL),
											vppif_reg32_read(DISP_TG_H_STA),vppif_reg32_read(DISP_TG_H_END));
	DPRINT("V total %d, VBIE %d, PVBI %d\n",vppif_reg32_read(DISP_TG_TOTAL_LINE),
											vppif_reg32_read(DISP_TG_VBIE),vppif_reg32_read(DISP_TG_PVBI));
	DPRINT("F1 total %d, beg %d, end %d\n",vppif_reg32_read(DISP_TG_F1_TOTAL_LINE),
											vppif_reg32_read(DISP_TG_F1_BEG),vppif_reg32_read(DISP_TG_F1_END));
	DPRINT("F2 total %d, beg %d, end %d\n",vppif_reg32_read(DISP_TG_F2_TOTAL_LINE),
											vppif_reg32_read(DISP_TG_F2_BEG),vppif_reg32_read(DISP_TG_F2_END));	
	DPRINT("DIFF F %d,V %d,H %d\n",vppif_reg32_read(DISP_TG_FIELD_DIFF),
											vppif_reg32_read(DISP_TG_V_DIFF),vppif_reg32_read(DISP_TG_H_DIFF));
	DPRINT("sync step %d, width %d\n",vppif_reg32_read(DISP_SYNC_STEP),vppif_reg32_read(DISP_SYNC_WIDTH));
	DPRINT("burst step %d,beg %d,end %d\n",vppif_reg32_read(DISP_BURST_STEP),
											vppif_reg32_read(DISP_BURST_STA),vppif_reg32_read(DISP_BURST_END));

	DPRINT("sw vbiend %d,P F1 beg %d,end %d\n",vppif_reg32_read(DISP_P_SW_VBIEND),
											vppif_reg32_read(DISP_P_F1_BEG),vppif_reg32_read(DISP_P_F1_END));
	DPRINT("---------- DISP Ext Timing ----------\n");	
	DPRINT("beg %d,end %d\n",vppif_reg32_read(DISP_EXT_TG_STA),vppif_reg32_read(DISP_EXT_TG_END));
	DPRINT("DIFF F %d,V %d,H %d\n",vppif_reg32_read(DISP_EXT_F_DIFF),
											vppif_reg32_read(DISP_EXT_V_DIFF),vppif_reg32_read(DISP_EXT_H_DIFF));

	DPRINT("---------- DISP VGA ----------\n");
	DPRINT("Vsync %d, Hsync %d\n",vppif_reg32_read(DISP_VGA_VSYNC),vppif_reg32_read(DISP_VGA_HSYNC));	
	DPRINT("Polar H %d, V %d\n",vppif_reg32_read(DISP_VGA_HSYNC_POLAR),vppif_reg32_read(DISP_VGA_VSYNC_POLAR));

}

void disp_set_enable(vpp_flag_t enable)
{
	if( enable  || 	vppif_reg32_read(DISP_EXTTV_EN)){
		vppif_reg32_write(DISP_EN,enable);
	}
	else {
		if( vppif_reg32_read(DISP_EN) ){
			vppm_set_int_enable(1,VPP_INT_DISP_VBIE);
#ifdef __KERNEL__
			vpp_irqproc_work(VPP_INT_DISP_VBIE,vpp_irqproc_disable_disp,0,1);
#else
			vppif_reg32_write(DISP_EN,0);
#endif
			vppm_set_int_enable(0,VPP_INT_DISP_VBIE);
		}
	}
	vppm_set_int_enable(enable,VPP_INT_DISP_VBIS);
}

void disp_set_field(vpp_flag_t field)
{
	vppif_reg32_write(DISP_INPUT_FIELD,field);
}

void disp_set_clamping(vpp_flag_t enable)
{
	vppif_reg32_write(DISP_CLAMP_EN,enable);
}

void disp_set_colorbar(vpp_flag_t enable,int width, int inverse)
{
	vppif_reg32_write(DISP_COLORBAR_EN,enable);
}

int disp_get_cur_field(void)
{
	if( vppif_reg32_read(DISP_ECD_FIELD) ){
		return 1;	// Bottom field
	}
	return 0;	// top field
}

/*----------------------- DISP EXTTV --------------------------------------*/
void disp_EXTTV_set_enable(vpp_flag_t enable)
{
	vppif_reg32_write(DISP_EXTTV_EN,enable);
	vppif_reg32_write(GOVRH_DAC_PWRDN,enable);
}

/*----------------------- DISP VGA --------------------------------------*/
void disp_VGA_set_polar(vpp_flag_t hsync_neg,vpp_flag_t vsync_neg)
{
	vppif_reg32_write(DISP_VGA_HSYNC_POLAR,hsync_neg);
	vppif_reg32_write(DISP_VGA_VSYNC_POLAR,vsync_neg);
}

/*----------------------- DISP DAC --------------------------------------*/
void disp_DAC_set_pwrdn(unsigned int dac,vpp_flag_t pwrdn)
{
	if( dac & BIT0 ){	// DAC A
		vppif_reg32_write(DISP_DAC_B_PWRDN,pwrdn);
	}
	if( dac & BIT1 ){	// DAC B
		vppif_reg32_write(DISP_DAC_C_PWRDN,pwrdn);
	}
	if( dac & BIT2 ){	// DAC C
		vppif_reg32_write(DISP_DAC_D_PWRDN,pwrdn);
	}
}

int disp_DAC_sense(void)
{
	static unsigned int sense_cnt = 0;
	unsigned int pwrdn;
	static unsigned int pre_pwrdn;

	if( vppif_reg32_read(DISP_EXTTV_EN) ){
		return -1;
	}

	sense_cnt++;
	if( (sense_cnt % p_disp->dac_sense_cnt) != 0 ){
		return -1;
	}

	switch( p_disp->tvconn ){
		case VPP_TVCONN_CVBS:
			disp_DAC_set_pwrdn(0x1,0x0);
			break;
		case VPP_TVCONN_SVIDEO:
			disp_DAC_set_pwrdn(0x7,0x0);
//			disp_DAC_set_pwrdn(0x4,0x1);
			break;
		default:
			disp_DAC_set_pwrdn(0x7,0x0);
			break;
	}

	vppif_reg32_write(VPP_DAC_SEL,0x0);
	vppif_reg32_write(GOVRH_DAC_MANUAL_SENSE,0x0);
	vppif_reg32_write(GOVRH_DAC_TEST_ENABLE,1);
	vppif_reg32_write(GOVRH_DAC_TEST_A,p_disp->dac_sense_val);
	vppif_reg32_write(GOVRH_DAC_TEST_B,0);
	vppif_reg32_write(GOVRH_DAC_TEST_C,0);
	pwrdn = (vppif_reg32_in(REG_GOVRH_DAC_CON))?0x01:0x00;

	vppif_reg32_write(GOVRH_DAC_TEST_A,0);
	vppif_reg32_write(GOVRH_DAC_TEST_B,p_disp->dac_sense_val);
	vppif_reg32_write(GOVRH_DAC_TEST_C,0x0);
	pwrdn |= (vppif_reg32_in(REG_GOVRH_DAC_CON))?0x02:0x00;

	vppif_reg32_write(GOVRH_DAC_TEST_A,0);
	vppif_reg32_write(GOVRH_DAC_TEST_B,0);
	vppif_reg32_write(GOVRH_DAC_TEST_C,p_disp->dac_sense_val);
	pwrdn |= (vppif_reg32_in(REG_GOVRH_DAC_CON))?0x04:0x00;

	switch( p_disp->tvconn ){
		case VPP_TVCONN_CVBS:
			pwrdn &= 0x1;
			break;
		case VPP_TVCONN_SVIDEO:
			pwrdn |= 0x06;
			break;
		default:
			break;
	}

	vppif_reg32_write(GOVRH_DAC_TEST_ENABLE,0);
	vppif_reg32_write(VPP_DAC_SEL,0x1);
	disp_DAC_set_pwrdn(~pwrdn,1);

	if( pre_pwrdn != pwrdn ){
		DPRINT("[DISP] TV DAC 0x%x\n",pwrdn);
		pre_pwrdn = pwrdn;
	}
	return (pwrdn==0);
}

/*----------------------- DISP interface --------------------------------------*/
void disp_patch(vpp_tvsys_t tvsys,vpp_tvconn_t tvconn)
{
	if( tvsys == VPP_TVSYS_NTSC ){
		switch( tvconn ){
			case VPP_TVCONN_CVBS:
			case VPP_TVCONN_SVIDEO:
				vppif_reg32_out(DISP_BASE_ADDR+0x14,0x21f07c00);
				vppif_reg32_out(DISP_BASE_ADDR+0x1c,0x02494a66);
				vppif_reg32_out(DISP_BASE_ADDR+0x18,0x3);
				vppif_reg32_out(DISP_BASE_ADDR+0x30,0x3);
				vppif_reg32_out(REG_DISP_DACMOD,0x01c70c00);
				vppif_reg32_out(DISP_BASE_ADDR+0x2c,0x0);
				vppif_reg32_out(DISP_BASE_ADDR+0x24,0x16);
				vppif_reg32_out(DISP_BASE_ADDR+0x44,0x3a);
				vppif_reg32_out(DISP_BASE_ADDR+0x3c,0xc7f);
				vppif_reg32_out(DISP_BASE_ADDR+0x28,0x00300070);
				vppif_reg32_out(DISP_BASE_ADDR+0x34,0x359);
				vppif_reg32_out(DISP_BASE_ADDR+0x40,0xcd08a);
				vppif_reg32_out(DISP_BASE_ADDR+0x10,0xB0);
				break;
			case VPP_TVCONN_YCBCR:
				vppif_reg32_out(DISP_BASE_ADDR+0x20,0x00905050);
				vppif_reg32_out(DISP_BASE_ADDR+0x28,0x00300078);
				break;
			case VPP_TVCONN_YPBPR:
				vppif_reg32_out(DISP_BASE_ADDR+0xa4,0x10017100);
				break;
			default:
				break;
		}
	}

	if( tvsys == VPP_TVSYS_PAL ){
		switch( tvconn ){
			case VPP_TVCONN_CVBS:
			case VPP_TVCONN_SVIDEO:
				vppif_reg32_out(REG_DISP_DACMOD,0x01c70c00);
				vppif_reg32_out(DISP_BASE_ADDR+0x14,0x2a098acb);
				vppif_reg32_out(DISP_BASE_ADDR+0x1c,0x02505070);
				vppif_reg32_out(DISP_BASE_ADDR+0x2c,0x0);
				vppif_reg32_out(DISP_BASE_ADDR+0x28,0x0030007a);
				vppif_reg32_out(DISP_BASE_ADDR+0x44,0x35);
				vppif_reg32_out(DISP_BASE_ADDR+0x38,0xd00e0);
				break;
			case VPP_TVCONN_YCBCR:
				vppif_reg32_out(DISP_BASE_ADDR+0x20,0x009c5555);
				vppif_reg32_out(DISP_BASE_ADDR+0x28,0x00300075);
				break;
			case VPP_TVCONN_YPBPR:
				vppif_reg32_out(DISP_BASE_ADDR+0xa4,0x01017100);
				break;
			default:
				break;
		}
	}
}

void disp_set_mode(vpp_tvsys_t tvsys,vpp_tvconn_t tvconn)
{
	disp_attr_t *p_attr;
	disp_timing_t *p_tim;
	unsigned int dac_sel[]={0,1,4,5,6,6};
	unsigned int IT_SYNC,PR_SYNC;
	unsigned int IT_AF,PR_AF;
	vpp_flag_t interlace;
	int h_scale_up;

	interlace = VPP_FLAG_TRUE;
	h_scale_up = 0;
	p_attr = disp_get_attribute(tvsys);
	switch( tvconn ){
		case VPP_TVCONN_YCBCR:	// YUV-I
			IT_SYNC = p_attr->SYNC_YPbPr;
			PR_SYNC = p_attr->P_SYNC_YPbPr;
			IT_AF = (p_attr->PYAF<<16)+(p_attr->PBAF<<8)+(p_attr->PRAF);
			PR_AF = (p_attr->P_PYAF<<16)+(p_attr->P_PBAF<<8)+(p_attr->P_PRAF);
			break;
		case VPP_TVCONN_SCART:	// RGB-I
			IT_SYNC = p_attr->SYNC_RGB;
			PR_SYNC = p_attr->P_SYNC_RGB;
			IT_AF= (p_attr->GAF<<16)+ (p_attr->BAF<<8)+(p_attr->RAF);
			PR_AF= (p_attr->P_GAF<<16)+ (p_attr->P_BAF<<8)+(p_attr->P_RAF);
			break;
		case VPP_TVCONN_YPBPR:	// YUV-P
			IT_SYNC = p_attr->SYNC_YPbPr;
			PR_SYNC = p_attr->P_SYNC_YPbPr;
			IT_AF = (p_attr->PYAF<<16)+(p_attr->PBAF<<8)+(p_attr->PRAF);
			PR_AF = (p_attr->P_PYAF<<16)+(p_attr->P_PBAF<<8)+(p_attr->P_PRAF);
			interlace = VPP_FLAG_FALSE;
			break;
		case VPP_TVCONN_VGA:	// RGB-P
			IT_SYNC = p_attr->SYNC_RGB;
			PR_SYNC = p_attr->P_SYNC_RGB;
			IT_AF= (p_attr->GAF<<16)+ (p_attr->BAF<<8)+(p_attr->RAF);
			PR_AF= (p_attr->VGA_GAF<<16)+ (p_attr->VGA_BAF<<8)+(p_attr->VGA_RAF);
			interlace = VPP_FLAG_FALSE;
			break;
		case VPP_TVCONN_SVIDEO:	// YC
		case VPP_TVCONN_CVBS:	// composite YC
			IT_SYNC = p_attr->SYNC_YPbPr;
			PR_SYNC = p_attr->P_SYNC_YPbPr;
			IT_AF= (p_attr->SPYAF<<16)+ (p_attr->SPBAF<<8)+(p_attr->SPRAF);
			PR_AF= (p_attr->P_SPYAF<<16)+ (p_attr->P_SPBAF<<8)+(p_attr->P_SPRAF);
			break;
		default:
			return;
	}

	disp_set_enable(VPP_FLAG_DISABLE);
	govrh_set_tg_enable(VPP_FLAG_DISABLE);

#ifdef WMT_FTBLK_DISP_HD
	{
		unsigned int reg;
		unsigned int pixclk;

		pixclk = 27000000;
		vppif_reg32_write(DISP_1080I_1T,0);
		switch(tvsys){
			case VPP_TVSYS_720P:
				reg = 0x23;
				interlace = 0;
				pixclk = 74250000;
				break;
			case VPP_TVSYS_1080I:
				vppif_reg32_write(DISP_1080I_1T,1);
				reg = 0x27;
				interlace = 1;
				pixclk = 74250000;
				break;
			case VPP_TVSYS_1080P:
				reg = 0x33;
				interlace = 0;
				pixclk = 148500000;
				break;
			default:
				reg = (interlace)? 0x0:0x20;
				h_scale_up = (interlace)? 1:0;
				break;
		}
		vppif_reg32_out(REG_DISP_HD_CTL,reg);
		govrh_set_clock(pixclk);
	}
#else
	govrh_set_clock(27000000);
#endif
	vppif_reg32_write(GOVRH_HSCALE_UP,h_scale_up);
	govrh_set_output_format(interlace);

	// REG_DISP_CTRL,0x00
	vppif_reg32_write(DISP_INPUT_FIELD,interlace);
	vppif_reg32_write(DISP_SW_SHIFT,p_attr->SW_SHIFT);
	vppif_reg32_write(DISP_SHIFT_TYPE,p_attr->SHIFT_TYPE);
	vppif_reg32_write(DISP_I_MODE_ONLY,p_attr->IT_MOD_ONLY);

	// REG_DISP_OPMOD,0x04
	vppif_reg32_write(DISP_PAL_NC_MODE,p_attr->PAL_NC);
	vppif_reg32_write(DISP_PAL_N_MODE,p_attr->PAL_N);
	vppif_reg32_write(DISP_LINE_SEL_525,p_attr->LINE_SEL);
	vppif_reg32_write(DISP_TVSYS_NTSC,p_attr->VOS);

	// REG_DISP_DACMOD,0x08
	vppif_reg32_write(DISP_DAC_SEL,dac_sel[tvconn]);
	if( vppif_reg32_read(DISP_EXTTV_EN) ){
		vppif_reg32_write(REG_DISP_DACMOD,0x4E,1,0xF);
	}
	else {
		vppif_reg32_write(REG_DISP_DACMOD,0x4E,1,p_attr->DAC_OFF);
	}

	// REG_DISP_FSCI_CFG,0x10
	vppif_reg32_write(DISP_FSCI_PHASE_ADJ,p_attr->PHASE_ADJ);

	// REG_DISP_FSCI_VAL,0x14
	vppif_reg32_out(REG_DISP_FSCI_VAL,p_attr->FSCI);

	// REG_DISP_AMP,0x1C
	vppif_reg32_write(REG_DISP_AMP,0x7000000,24,IT_SYNC);
	vppif_reg32_write(DISP_SYNC_Y_CONTRAST,p_attr->YAF);
	vppif_reg32_write(DISP_SYNC_U_SATURATION,p_attr->UAF);
	vppif_reg32_write(DISP_SYNC_V_SATURATION,p_attr->VAF);

	// REG_DISP_PSAMP,0x20
	vppif_reg32_out(REG_DISP_PSAMP,IT_AF);

	// REG_DISP_BRI,0x24
	vppif_reg32_write(DISP_BLACK_LEVEL,p_attr->BLACK_LEVEL);

	// REG_DISP_PEDESTAL,0x28
	vppif_reg32_write(DISP_BLANK_2LEVEL,p_attr->BLANK_LEVEL2);
	vppif_reg32_write(DISP_BLANK_LEVEL,p_attr->BLANK_LEVEL);

	// REG_DISP_HSE,0x2C
	vppif_reg32_write(DISP_HUE_ADJUST,p_attr->HUE_ADJ);

	// REG_DISP_YCDLY,0x30
	vppif_reg32_out(REG_DISP_YCDLY,p_attr->YC_DELAY);

	// REG_DISP_HCOORD,0x34
	vppif_reg32_write(DISP_TG_TOTAL_PIXEL,p_attr->TH_TOTAL);

	// REG_DISP_SYNC,0x3C
	vppif_reg32_write(DISP_SYNC_STEP,p_attr->SYNC_STEP);
	vppif_reg32_write(DISP_SYNC_WIDTH,p_attr->THSYNC_WIDTH);

	// REG_DISP_BURST1,0x40
	vppif_reg32_write(DISP_BURST_STEP,p_attr->BURST_STEP);
	vppif_reg32_write(DISP_BURST_END,p_attr->TBURST_END);
	vppif_reg32_write(DISP_BURST_STA,p_attr->TBURST_START);

	// REG_DISP_BURST2,0x44
	vppif_reg32_write(DISP_BURST_AMP,p_attr->BURST_AMP);

	// Timing
	p_tim = disp_get_timing(tvsys,interlace,vppif_reg32_read(DISP_EXTTV_EN));

	vppif_reg32_write(DISP_TG_H_STA,p_tim->H_BEGIN);
#ifdef WMT_FTBLK_DISP_HD
	vppif_reg32_write(DISP_TG_H_START,(p_tim->H_BEGIN & ~0x1ff)>>9);
#endif
	vppif_reg32_write(DISP_TG_H_END,p_tim->H_END);

	vppif_reg32_write(DISP_TG_FIELD_DIFF,p_tim->F_DIFF);
	vppif_reg32_write(DISP_TG_V_DIFF,p_tim->V_DIFF);
	vppif_reg32_write(DISP_TG_H_DIFF,p_tim->H_DIFF);
	vppif_reg32_write(DISP_TG_TOTAL_LINE,p_tim->FRAME_TOTAL);
	vppif_reg32_write(DISP_FRAME_TOTAL_HD,(p_tim->FRAME_TOTAL & ~0x3FF) >> 10);

	vppif_reg32_write(DISP_TG_VBIE,p_tim->VBIE_LINE);
	vppif_reg32_write(DISP_TG_PVBI,p_tim->PVBI_LINE);
	vppif_reg32_write(DISP_TG_F2_END,p_tim->F2_END);
	vppif_reg32_write(DISP_TG_F1_END,p_tim->F1_END);

	vppif_reg32_write(DISP_TG_F2_TOTAL_LINE,p_tim->F2_TOTAL);
	vppif_reg32_write(DISP_TG_F2_BEG,p_tim->F2_BEGIN);
	vppif_reg32_write(DISP_TG_F1_TOTAL_LINE,p_tim->F1_TOTAL);
	vppif_reg32_write(DISP_TG_F1_BEG,p_tim->F1_BEGIN);

	// external TV timing	
	vppif_reg32_write(DISP_EXT_TG_END,p_tim->EXT_H_END);
	vppif_reg32_write(DISP_EXT_TG_STA,p_tim->EXT_H_BEGIN);

	vppif_reg32_write(DISP_EXT_F_DIFF,p_tim->EXT_F_DIFF);
	vppif_reg32_write(DISP_EXT_V_DIFF,p_tim->EXT_V_DIFF);
	vppif_reg32_write(DISP_EXT_H_DIFF,p_tim->EXT_H_DIFF);

	// REG_DISP_P_AMP,0xA0
	vppif_reg32_write(REG_DISP_P_AMP,0x7000000,24,PR_SYNC);
	vppif_reg32_write(REG_DISP_P_AMP,0xFFFFFF,0,PR_AF);

	// REG_DISP_P_YLVL,0xA4
	vppif_reg32_write(DISP_P_BLACK_LEVEL,p_attr->P_BLACK_LEVEL);
	vppif_reg32_write(DISP_P_BLANK_LEVEL2,p_attr->P_BLANK_LEVEL2);
	vppif_reg32_write(DISP_P_BLANK_LEVEL,p_attr->P_BLANK_LEVEL);

	// REG_DISP_P_YCDLY,0xA8
	vppif_reg32_out(REG_DISP_P_YCDLY,p_attr->P_YCDLY);

	// REG_DISP_P_SYNC,0xAC
	vppif_reg32_write(DISP_P_SYNC_STEP,p_attr->P_SYNC_STEP);
	vppif_reg32_write(DISP_P_SYNC_WIDTH,p_attr->P_THSYNC_WIDTH);

	vppif_reg32_out(REG_DISP_P_TIMING,0x1);		// clk inverse

#ifdef WMT_FTBLK_DISP_CSC
	vppif_reg32_out(REG_DISP_CSC_ENABLE,((tvconn==VPP_TVCONN_YPBPR)||(tvconn==VPP_TVCONN_YCBCR))? 3:0);
#endif

	disp_patch(tvsys,tvconn);
	
	disp_set_enable(VPP_FLAG_ENABLE);
	
}

void disp_set_tvsys(vpp_tvsys_t tvsys)
{
	p_disp->tvsys = tvsys;
	disp_set_mode(p_disp->tvsys,p_disp->tvconn);
}

void disp_set_tvconn(vpp_tvconn_t tvconn)
{
	p_disp->tvconn = tvconn;
	disp_set_mode(p_disp->tvsys,p_disp->tvconn);
}

vdo_color_fmt disp_get_color_format(void)
{
	vdo_color_fmt colfmt;

	switch( p_disp->tvconn ){
		case VPP_TVCONN_YCBCR:
		case VPP_TVCONN_YPBPR:
		case VPP_TVCONN_SVIDEO:
		case VPP_TVCONN_CVBS:
		default:
			colfmt = VDO_COL_FMT_YUV444;
			break;
		case VPP_TVCONN_SCART:
		case VPP_TVCONN_VGA:
			colfmt = VDO_COL_FMT_ARGB;
			break;
	}
	return colfmt;
}

#ifdef CONFIG_PM
static unsigned int *disp_reg_bk2;
static unsigned int disp_pm_enable;
void disp_suspend(int sts)
{
	switch( sts ){
		case 0:	// disable module
			disp_pm_enable = vppif_reg32_read(DISP_EN);
			disp_set_enable(0);
			break;
		case 1: // disable tg
			break;
		case 2:	// backup register
			p_disp->reg_bk = vpp_backup_reg(REG_DISP_BASE1_BEGIN,(REG_DISP_BASE1_END-REG_DISP_BASE1_BEGIN));
			disp_reg_bk2 = vpp_backup_reg(REG_DISP_BASE2_BEGIN,(REG_DISP_BASE2_END-REG_DISP_BASE2_BEGIN));
			break;
		default:
			break;
	}
}

void disp_resume(int sts)
{
	switch( sts ){
		case 0:	// restore register
			vpp_restore_reg(REG_DISP_BASE1_BEGIN,(REG_DISP_BASE1_END-REG_DISP_BASE1_BEGIN),p_disp->reg_bk);
			vpp_restore_reg(REG_DISP_BASE2_BEGIN,(REG_DISP_BASE2_END-REG_DISP_BASE2_BEGIN),disp_reg_bk2);
			disp_reg_bk2 = 0;
			p_disp->reg_bk = 0;			
			break;
		case 1:	// enable module
			disp_set_enable(disp_pm_enable);
			break;
		case 2: // enable tg
			break;
		default:
			break;
	}
}
#else
#define disp_suspend NULL
#define disp_resume NULL
#endif

void disp_init(void *base)
{
	disp_mod_t *mod_p;

	mod_p = (disp_mod_t *) base;

	disp_set_colorbar(VPP_FLAG_DISABLE,0,0);
	vppif_reg32_write(DISP_CLAMP_EN,1);
	vppif_reg32_write(DISP_DAC_HPS,0x7);
	vppif_reg32_write(DISP_DAC_RESET,0x7);
	vppif_reg32_write(DISP_DAC_A_2X,1);
	vppif_reg32_write(DISP_DAC_BCD_2X,1);
	vppif_reg32_out(REG_DISP_DACVAL,0x028e028e);
	vppif_reg32_out(REG_DISP_FIL,0x2);
	vppif_reg32_out(REG_DISP_VGA,0x06a4003c);
	disp_DAC_set_pwrdn(0xFF,VPP_FLAG_ENABLE);
	vppif_reg32_out(REG_DISP_VBIS_TMR,0x8);

	vppif_reg32_out(REG_DISP_FILTER1,0x07fd07fc );	// filter1,2
	vppif_reg32_out(REG_DISP_FILTER2,0x07fb07fc);	// filter3,4
	vppif_reg32_out(REG_DISP_FILTER3,0x07ff0004);	// filter5,6
	vppif_reg32_out(REG_DISP_FILTER4,0x000d0015);	// filter7,8
	vppif_reg32_out(REG_DISP_FILTER5,0x00220030);	// filter9,10
	vppif_reg32_out(REG_DISP_FILTER6,0x003e004c);	// filter11,12
	vppif_reg32_out(REG_DISP_FILTER7,0x0057005f);	// filter13,14
	vppif_reg32_out(REG_DISP_FILTER8,0x00000064);	// filter15

#ifdef WMT_FTBLK_DISP_CSC
	vppif_reg32_out(REG_DISP_CSC1, 0x00000220);	//CSC1
	vppif_reg32_out(REG_DISP_CSC2, 0x00000000);	//CSC2
	vppif_reg32_out(REG_DISP_CSC3, 0x00000400);	//CSC3
	vppif_reg32_out(REG_DISP_CSC4, 0x00000000);	//CSC4
	vppif_reg32_out(REG_DISP_CSC5, 0x00000400);	//CSC5
	vppif_reg32_out(REG_DISP_CSC6, 0x00010001);	//CSC6
	vppif_reg32_out(REG_DISP_CSC7, 0x00000001);	//CSC6
	vppif_reg32_out(REG_DISP_CSC, 0x0);	//CSC_CTL
#endif	
}

int disp_mod_init(void)
{
	disp_mod_t *mod_p;

	mod_p = (disp_mod_t *) vpp_mod_register(VPP_MOD_DISP,sizeof(disp_mod_t),0);
	if( !mod_p ){
		DPRINT("*E* DISP module register fail\n");
		return -1;
	}

	/* module member variable */
	mod_p->int_catch = VPP_INT_NULL;
	mod_p->tvsys = VPP_TVSYS_NTSC;
	mod_p->tvconn = VPP_TVCONN_CVBS;
	mod_p->dac_sense_cnt = 300;
	mod_p->dac_sense_val = 0x1AE;

	/* module member function */
	mod_p->init = disp_init;
	mod_p->dump_reg = disp_reg_dump;
	mod_p->set_enable = disp_set_enable;
	mod_p->set_colorbar = disp_set_colorbar;
	mod_p->suspend = disp_suspend;
	mod_p->resume = disp_resume;

	p_disp = mod_p;
	return 0;
}
module_init(disp_mod_init);
#endif	//WMT_FTBLK_DISP
