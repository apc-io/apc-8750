/*++ 
 * linux/drivers/video/wmt/govm.c
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

#define GOVM_C
// #define DEBUG

#include "govm.h"
 
#ifdef WMT_FTBLK_GOVM
void govm_reg_dump(void)
{
	DPRINT("========== GOVM register dump ==========\n");
	vpp_reg_dump(REG_GOVM_BEGIN,REG_GOVM_END-REG_GOVM_BEGIN);

	DPRINT("GE enable %d,VPU enable %d\n",vppif_reg32_read(GOVM_GE_SOURCE),vppif_reg32_read(GOVM_VPU_SOURCE));
	DPRINT("Display coordinate (%d,%d)\n",vppif_reg32_read(GOVM_DISP_X_CR),vppif_reg32_read(GOVM_DISP_Y_CR));
	DPRINT("VPU coordinate (%d,%d) - (%d,%d)\n",vppif_reg32_read(GOVM_VPU_X_STA_CR),vppif_reg32_read(GOVM_VPU_Y_STA_CR),
		vppif_reg32_read(GOVM_VPU_X_END_CR),vppif_reg32_read(GOVM_VPU_Y_END_CR));
	DPRINT("alpha enable %d,mode %d,A %d,B %d\n",vppif_reg32_read(GOVM_IND_MODE_ENABLE),vppif_reg32_read(GOVM_IND_MODE),
		vppif_reg32_read(GOVM_IND_ALPHA_A),vppif_reg32_read(GOVM_IND_ALPHA_B));
	DPRINT("color bar enable %d,mode %d,inv %d\n",vppif_reg32_read(GOVM_COLBAR_ENABLE),
		vppif_reg32_read(GOVM_COLBAR_MODE),vppif_reg32_read(GOVM_COLBAR_INVERSION));
	DPRINT("INT enable GOV %d\n",vppif_reg32_read(GOVM_INT_ENABLE));
	DPRINT("Gamma mode %d, clamp enable %d\n",vppif_reg32_read(GOVM_GAMMA_MODE),vppif_reg32_read(GOVM_CLAMPING_ENABLE));
}

void govm_set_in_path(vpp_path_t in_path, vpp_flag_t enable)
{
//#ifdef WMT_FTBLK_SCL
	if (VPP_PATH_GOVM_IN_VPU & in_path) {
#ifdef PATCH_GOVW_TG_ENABLE
		if( enable ){
			vppif_reg32_write(VPU_SCALAR_ENABLE,VPP_FLAG_ENABLE);
			vpp_vpu_sw_reset();
		}
		vppif_reg32_write(GOVM_VPU_SOURCE, enable);
		if( !enable ){
			vppif_reg32_write(VPU_SCALAR_ENABLE,VPP_FLAG_DISABLE);
		}
		vpu_int_set_enable(enable, VPP_INT_ERR_VPUW_MIFY + VPP_INT_ERR_VPUW_MIFC);
#else
		vppif_reg32_write(GOVM_VPU_SOURCE, enable);
#endif
	}
//#endif
#ifdef WMT_FTBLK_GE
	if (VPP_PATH_GOVM_IN_GE & in_path) {
		vppif_reg32_write(GOVM_GE_SOURCE, enable);
	}
#endif
#ifdef WMT_FTBLK_PIP
	if (VPP_PATH_GOVM_IN_PIP & in_path) {
		vppif_reg32_write(GOVM_PIP_SOURCE, enable);
	}
#endif
}

vpp_path_t govm_get_in_path(void)
{
	vpp_path_t ret;

	ret = 0;

//#ifdef WMT_FTBLK_SCL
	if (vppif_reg32_read(GOVM_VPU_SOURCE)){
		ret |= VPP_PATH_GOVM_IN_VPU;
	}
//#endif
#ifdef WMT_FTBLK_GE
	if (vppif_reg32_read(GOVM_GE_SOURCE)){
		ret |= VPP_PATH_GOVM_IN_GE;
	}
#endif
#ifdef WMT_FTBLK_PIP
	if (vppif_reg32_read(GOVM_PIP_SOURCE)){
		ret |= VPP_PATH_GOVM_IN_PIP;
	}
#endif
	return ret;
}

#ifdef GOVM_TV_MODE
vpp_flag_t govm_set_output_device(vpp_output_device_t device)
{
	switch (device) {
	case VPP_OUTDEV_TV_NORMAL:
		vppif_reg32_write(GOVM_TV_MODE, 0x0);
		break;
	default:
		return VPP_FLAG_ERROR;
	}
	return VPP_FLAG_SUCCESS;
}
#endif

void govm_set_blanking_enable(vpp_flag_t enable)
{
	vppif_reg32_write(GOVM_BLUE_SCREEN_ENABLE,enable);
}

void govm_set_blanking(U32 color)
{
	vppif_reg32_out(REG_GOVM_BNK,color);
}

void govm_set_disp_coordinate(U32 x, U32 y)
{
	vppif_reg32_write(GOVM_DISP_X_CR,x-1);
	vppif_reg32_write(GOVM_DISP_Y_CR,y-1);

	// modify GE coordinate
	REG32_VAL(GE3_BASE_ADDR+0x50) = x-1;
	REG32_VAL(GE3_BASE_ADDR+0x54) = y-1;
	REG32_VAL(GE3_BASE_ADDR+0x34) = x-1;
	REG32_VAL(GE3_BASE_ADDR+0x3c) = y-1;
}

void govm_get_disp_coordinate(U32 * p_x, U32 * p_y)
{
	*p_x = vppif_reg32_read(GOVM_DISP_X_CR);
	*p_y = vppif_reg32_read(GOVM_DISP_Y_CR);
}

void govm_set_vpu_coordinate(U32 x_s, U32 y_s, U32 x_e, U32 y_e)
{
	vppif_reg32_write(GOVM_VPU_X_STA_CR,x_s);
	vppif_reg32_write(GOVM_VPU_X_END_CR,x_e);
	vppif_reg32_write(GOVM_VPU_Y_STA_CR,y_s);
	vppif_reg32_write(GOVM_VPU_Y_END_CR,y_e);
}

void govm_set_vpu_position(U32 x1, U32 y1)
{
	U32 x2, y2;

	x2 = x1 + vppif_reg32_read(GOVM_VPU_X_END_CR) - vppif_reg32_read(GOVM_VPU_X_STA_CR);
	y2 = y1 + vppif_reg32_read(GOVM_VPU_Y_END_CR) - vppif_reg32_read(GOVM_VPU_Y_STA_CR);
	govm_set_vpu_coordinate(x1, y1, x2, y2);
}

void govm_set_alpha_mode(vpp_flag_t enable,vpp_alpha_t mode,int A,int B)
{
	vppif_reg32_write(GOVM_IND_MODE_ENABLE, enable);
	
	// mode A : (VPU)*A + (1-A)*Blanking
	// mode B : (VPU)*A + (GE)*B
	vppif_reg32_write(GOVM_IND_MODE,mode);
	vppif_reg32_write(GOVM_IND_ALPHA_A, A);
	vppif_reg32_write(GOVM_IND_ALPHA_B, B);
}

void govm_set_colorbar(vpp_flag_t enable,int width, int inverse)
{
	vppif_reg32_write(GOVM_COLBAR_ENABLE, enable);
	vppif_reg32_write(GOVM_COLBAR_MODE, width);
	vppif_reg32_write(GOVM_COLBAR_INVERSION, inverse);
}

void govm_set_int_enable(vpp_flag_t enable)
{
	vppif_reg32_write(GOVM_INT_ENABLE, enable);
}

void govm_set_reg_update(vpp_flag_t enable)
{
	vppif_reg32_write(GOVM_REG_UPDATE, enable);
}

void govm_set_reg_level(vpp_reglevel_t level)
{
	switch (level) {
	case VPP_REG_LEVEL_1:
		/*
		 * don't use the VPP_REG_LEVEL_1 directly, like 'vppif_reg32_write(GOVM_REG_LEVEL, VPP_REG_LEVEL_1)';
		 * because in others module, LEVEL 1 equal '0' maybe.
		 */
		vppif_reg32_write(GOVM_REG_LEVEL, 0x00);
		break;
	case VPP_REG_LEVEL_2:
		vppif_reg32_write(GOVM_REG_LEVEL, 0x01);
		break;
	default:
		DPRINT("*E* check the parameter.\n");
		return;
	}
	return;
}

void govm_set_gamma_mode(int mode)
{
	vppif_reg32_write(GOVM_GAMMA_MODE,mode);
}

void govm_set_clamping_enable(vpp_flag_t enable)
{
	vppif_reg32_write(GOVM_CLAMPING_ENABLE, enable);
}

/*----------------------- GOVM INTERRUPT --------------------------------------*/
void govm_int_set_enable(vpp_flag_t enable, vpp_int_err_t int_bit)
{
#ifdef WMT_FTBLK_PIP
	if (int_bit & VPP_INT_ERR_GOVM_PIP) {
		vppif_reg32_write(GOVM_INT_PIP_ENABLE, enable);
	}
	if (int_bit & VPP_INT_ERR_PIP_Y) {
		vppif_reg32_write(GOVM_INT_PIP_Y_ENABLE, enable);
	}
	if (int_bit & VPP_INT_ERR_PIP_C) {
		vppif_reg32_write(GOVM_INT_PIP_C_ENABLE, enable);
	}
#endif		
}

vpp_int_err_t govm_get_int_status(void)
{
	vpp_int_err_t int_sts;

	int_sts = 0;
	if (vppif_reg32_read(GOVM_INTSTS_GOVM_STATUS)){
		if( vppif_reg32_read(GOVM_INTSTS_VPU_READY) == 0 ){
			int_sts |= VPP_INT_ERR_GOVM_VPU;
		}
		
		if( vppif_reg32_read(GOVM_INTSTS_GE_READY) == 0 ){
			int_sts |= VPP_INT_ERR_GOVM_GE;
		}
		vppif_reg8_out(REG_GOVM_INT+0x0,BIT0);
	}

#ifdef WMT_FTBLK_SPU
	if( vppif_reg32_read(GOVM_INTSTS_SPU_READY) == 0 ){
		int_sts |= VPP_INT_ERR_GOVM_SPU;
	}
#endif
#ifdef WMT_FTBLK_PIP
	if (vppif_reg32_read(GOVM_INT_PIP_ERR)){
		if( vppif_reg32_read(GOVM_INTSTS_PIP_READY) == 0 ){
			int_sts |= VPP_INT_ERR_GOVM_PIP;
		}
	}
	if( vppif_reg32_read(GOVM_INT_PIP_Y_ERR) ){
		int_sts |= VPP_INT_ERR_PIP_Y;
	}
	if( vppif_reg32_read(GOVM_INT_PIP_C_ERR) ){
		int_sts |= VPP_INT_ERR_PIP_C;
	}
#endif
	return int_sts;
}

void govm_clean_int_status(vpp_int_err_t int_sts)
{
	if( (int_sts & VPP_INT_ERR_GOVM_VPU) || (int_sts & VPP_INT_ERR_GOVM_GE) ){
		vppif_reg8_out(REG_GOVM_INT+0x0,BIT0);
	}
#ifdef WMT_FTBLK_PIP
	if( (int_sts & VPP_INT_ERR_GOVM_PIP) ){
		vppif_reg8_out(REG_GOVM_INT+0x0,BIT1);
	}
	if( (int_sts & VPP_INT_ERR_PIP_Y) ){
		vppif_reg8_out(REG_GOVM_INT+0x0,BIT2);
	}
	if( (int_sts & VPP_INT_ERR_PIP_C) ){
		vppif_reg8_out(REG_GOVM_INT+0x0,BIT3);
	}
#endif
}

/*----------------------- GOVM PIP --------------------------------------*/
#ifdef WMT_FTBLK_PIP
void govm_set_pip_colorbar(vpp_flag_t enable,int width, int inverse)
{
#ifdef GOVM_PIP_COLBAR_ENABLE
	vppif_reg32_write(GOVM_PIP_COLBAR_ENABLE,enable);
#endif
}

void govm_set_dominate(vpp_flag_t pip)
{
	/* 0: PIP overlap VPU, 1: VPU overlap PIP */
	vppif_reg32_write(GOVM_PIP_DOMINATE,(pip)?0:1);
}

void govm_set_pip_coordinate(unsigned int x_s, unsigned int y_s, unsigned int x_e, unsigned int y_e)
{
	vppif_reg32_write(GOVM_PIP_X_STA_CR,x_s);
	vppif_reg32_write(GOVM_PIP_X_END_CR,x_e);
	vppif_reg32_write(GOVM_PIP_Y_STA_CR,y_s);
	vppif_reg32_write(GOVM_PIP_Y_END_CR,y_e);
}

void govm_set_pip_position(unsigned int x1, unsigned int y1)
{
	unsigned int x2, y2;

	x2 = x1 + vppif_reg32_read(GOVM_PIP_X_END_CR) - vppif_reg32_read(GOVM_PIP_X_STA_CR);
	y2 = y1 + vppif_reg32_read(GOVM_PIP_Y_END_CR) - vppif_reg32_read(GOVM_PIP_Y_STA_CR);
	govm_set_pip_coordinate(x1, y1, x2, y2);
}

void govm_set_pip_interlace(vpp_flag_t enable)
{
	vppif_reg32_write(GOVM_PIP_INTERLACE,enable);
}

void govm_set_pip_color_format(vdo_color_fmt format)
{
	switch (format) {
	case VDO_COL_FMT_ARGB:
		vppif_reg32_write(GOVM_PIP_COLFMT_RGB, 1);
		break;
	case VDO_COL_FMT_YUV444:
		vppif_reg32_write(GOVM_PIP_COLFMT_RGB, 0);
		vppif_reg32_write(GOVM_PIP_COLFMT_444, 1);
		vppif_reg32_write(GOVM_PIP_COLFMT_422, 0);
		break;
	case VDO_COL_FMT_YUV422H:
		vppif_reg32_write(GOVM_PIP_COLFMT_RGB, 0);
		vppif_reg32_write(GOVM_PIP_COLFMT_444, 0);
		vppif_reg32_write(GOVM_PIP_COLFMT_422, 0);
		break;
	case VDO_COL_FMT_YUV420:
		vppif_reg32_write(GOVM_PIP_COLFMT_RGB, 0);
		vppif_reg32_write(GOVM_PIP_COLFMT_444, 0);
		vppif_reg32_write(GOVM_PIP_COLFMT_422, 1);
		break;
	default:
		DPRINT("*E* check the parameter.\n");
		return;
	}

	if( format == VDO_COL_FMT_ARGB ){
		if( govw_get_hd_color_format() < VDO_COL_FMT_ARGB ){
			DPRINT("*E* govw & pip color fmt not match");
		}
	}
	else {
		if( govw_get_hd_color_format() >= VDO_COL_FMT_ARGB ){
			DPRINT("*E* govw & pip color fmt not match");
		}
	}
	
}

vdo_color_fmt govm_get_pip_color_format(void)
{
	if( vppif_reg32_read(GOVM_PIP_COLFMT_RGB) )
		return VDO_COL_FMT_ARGB;

	if( vppif_reg32_read(GOVM_PIP_COLFMT_444) ){
		return VDO_COL_FMT_YUV444;
	}

	if( vppif_reg32_read(GOVM_PIP_COLFMT_422) ){
		return VDO_COL_FMT_YUV420;
	}
	return VDO_COL_FMT_YUV422H;
}

void govm_set_pip_fb_addr(unsigned int y_addr,unsigned int c_addr)
{
	DBGMSG("y_addr:0x%08x, c_addr:0x%08x\n",y_addr,c_addr);
	vppif_reg32_out(REG_GOVM_PIP_Y_ADDR,y_addr);
	vppif_reg32_out(REG_GOVM_PIP_C_ADDR,c_addr);
}

void govm_get_pip_fb_addr(unsigned int *y_addr,unsigned int *c_addr)
{
	*y_addr = vppif_reg32_in(REG_GOVM_PIP_Y_ADDR);
	*c_addr = vppif_reg32_in(REG_GOVM_PIP_C_ADDR);
	DBGMSG("y_addr:0x%08x, c_addr:0x%08x\n", *y_addr, *c_addr);
}

void govm_set_pip_fb_width(unsigned int width,unsigned int buf_width)
{
	vppif_reg32_write(GOVM_PIP_XWIDTH, width);
	vppif_reg32_write(GOVM_PIP_FBUF_WIDTH, buf_width);
}

void govm_get_pip_fb_width(unsigned int *width,unsigned int *buf_width)
{
	*width = vppif_reg32_read(GOVM_PIP_XWIDTH);
	*buf_width = vppif_reg32_read(GOVM_PIP_FBUF_WIDTH);
}

void govm_set_pip_crop(unsigned int h_crop,unsigned int v_crop)
{
	vppif_reg32_write(GOVM_PIP_HCROP, h_crop);
	vppif_reg32_write(GOVM_PIP_VCROP, v_crop);
}

void govm_set_pip_media_format(vpp_media_format_t format)
{
	switch (format) {
	case VPP_MEDIA_FMT_MPEG:
		vppif_reg32_write(GOVM_PIP_SRC_H264, 0x0);
		break;
	case VPP_MEDIA_FMT_H264:
		vppif_reg32_write(GOVM_PIP_SRC_H264, 0x1);
		break;
	default:
		DBGMSG("*E* check the parameter.\n");
		return;
	}
}

void govm_set_pip_field(vpp_display_format_t field)
{
	vppif_reg32_write(GOVM_PIP_OUTFMT_FIELD,field);	/*0:Frame, 1:Field */
}

void govm_set_pip_framebuf(vdo_framebuf_t *fb)
{
	govm_set_pip_color_format(fb->col_fmt);
	govm_set_pip_fb_addr(fb->y_addr,fb->c_addr);
	govm_set_pip_fb_width(fb->img_w,fb->fb_w);
	govm_set_pip_crop(fb->h_crop,fb->v_crop);
	govm_set_pip_coordinate(p_pip->posx,p_pip->posy,p_pip->posx+fb->img_w-1,p_pip->posy+fb->img_h-1);
	p_vpu->fb_p->set_csc(p_vpu->fb_p->csc_mode);
	p_govw->fb_p->set_csc(p_govw->fb_p->csc_mode);
}

void govm_pip_proc_view(int read,vdo_view_t *view)
{
	vdo_framebuf_t *fb;

	fb = &p_pip->fb_p->fb;

	if( read ){
		view->resx_src = fb->fb_w;
		view->resy_src = fb->fb_h;
		view->resx_virtual = fb->img_w;
		view->resy_virtual = fb->img_h;
		view->resx_visual = p_pip->resx_visual;
		view->resy_visual = p_pip->resy_visual;
		view->posx = p_pip->posx;
		view->posy = p_pip->posy;
		view->offsetx = fb->h_crop;
		view->offsety = fb->v_crop;
	}
	else {
		fb->img_w = view->resx_virtual;
		fb->img_h = view->resy_virtual;
		fb->fb_w = view->resx_src;
		fb->fb_h = view->resy_src;
		fb->h_crop = view->offsetx;
		fb->v_crop = view->offsety;
		
		p_pip->posx = view->posx;
		p_pip->posy = view->posy;
		p_pip->resx_visual = view->resx_visual;
		p_pip->resy_visual = view->resy_visual;
		
		govm_set_pip_fb_width(view->resx_virtual,view->resx_src);
		govm_set_pip_coordinate(view->posx,view->posy,view->posx+view->resx_visual,view->posy+view->resy_visual);
	}
}
#endif /* WMT_FTBLK_PIP */

#ifdef CONFIG_PM
void govm_suspend(int sts)
{
	switch( sts ){
		case 0:	// disable module
			break;
		case 1: // disable tg
			break;
		case 2:	// backup register
			p_govm->reg_bk = vpp_backup_reg(REG_GOVM_BEGIN,(REG_GOVM_END-REG_GOVM_BEGIN));
			break;
		default:
			break;
	}
}

void govm_resume(int sts)
{
	switch( sts ){
		case 0:	// restore register
			vpp_restore_reg(REG_GOVM_BEGIN,(REG_GOVM_END-REG_GOVM_BEGIN),p_govm->reg_bk);
			p_govm->reg_bk = 0;			
			break;
		case 1:	// enable module
			break;
		case 2: // enable tg
			break;
		default:
			break;
	}
}
#else
#define govm_suspend NULL
#define govm_resume NULL
#endif

/*----------------------- GOVM INIT --------------------------------------*/
void govm_init(void *base)
{
	govm_mod_t *mod_p;

	mod_p = (govm_mod_t *) base;

	govm_set_colorbar(VPP_FLAG_DISABLE,0,0);
	govm_set_int_enable(VPP_FLAG_DISABLE);
	govm_set_blanking(VPP_COL_BLACK);
	govm_set_in_path(mod_p->path, VPP_FLAG_ENABLE);
	govm_set_vpu_coordinate(0,0,VPP_HD_DISP_RESX,VPP_HD_DISP_RESY);
	govm_set_disp_coordinate(VPP_HD_DISP_RESX,VPP_HD_DISP_RESY);
	if ((VPP_INT_ERR_GOVM_VPU & mod_p->int_catch) || (VPP_INT_ERR_GOVM_GE & mod_p->int_catch)) {
		govm_set_int_enable(VPP_FLAG_ENABLE);
	} else {
		govm_set_int_enable(VPP_FLAG_DISABLE);
	}
	govm_set_clamping_enable(VPP_FLAG_DISABLE);
	govm_set_reg_update(VPP_FLAG_ENABLE);
}

#ifdef WMT_FTBLK_PIP
void govm_pip_init(void *base)
{
	govm_set_dominate(VPP_FLAG_ENABLE);
	govm_set_pip_coordinate(0,0,VPP_HD_DISP_RESX-1,VPP_HD_DISP_RESY-1);
	govm_set_pip_interlace(VPP_FLAG_DISABLE);
	govm_set_pip_media_format(VPP_MEDIA_FMT_MPEG);
	govm_set_pip_field(VPP_DISP_FMT_FRAME);
}
#endif

int govm_mod_init(void)
{
	govm_mod_t *mod_p;

	mod_p = (govm_mod_t *) vpp_mod_register(VPP_MOD_GOVM,sizeof(govm_mod_t),0);
	if( !mod_p ){
		DPRINT("*E* GOVM module register fail\n");
		return -1;
	}

	/* module member variable */
	mod_p->path = VPP_PATH_NULL;
	mod_p->int_catch = VPP_INT_NULL;

	/* module member function */
	mod_p->init = govm_init;
	mod_p->dump_reg = govm_reg_dump;
	mod_p->set_colorbar = govm_set_colorbar;
	mod_p->get_sts = govm_get_int_status;
	mod_p->clr_sts = govm_clean_int_status;
	mod_p->suspend = govm_suspend;
	mod_p->resume = govm_resume;

	p_govm = mod_p;

#ifdef WMT_FTBLK_PIP
{
	pip_mod_t *pip_mod_p;
	vpp_fb_base_t *mod_fb_p;
	vdo_framebuf_t *fb_p;

	pip_mod_p = (pip_mod_t *) vpp_mod_register(VPP_MOD_PIP,sizeof(pip_mod_t),VPP_MOD_FLAG_FRAMEBUF);
	mod_fb_p = pip_mod_p->fb_p;

	/* module member variable */
	pip_mod_p->int_catch = VPP_INT_NULL;
	pip_mod_p->resx_visual = VPP_HD_DISP_RESX;
	pip_mod_p->resy_visual = VPP_HD_DISP_RESY;
	pip_mod_p->posx = pip_mod_p->posy = 0;
	pip_mod_p->pre_yaddr = pip_mod_p->pre_caddr = 0;

	/* module member function */
	pip_mod_p->init = govm_pip_init;
	pip_mod_p->set_colorbar = govm_set_pip_colorbar;

	p_pip = pip_mod_p;

	/* frame buffer variable */
	mod_fb_p->capability = BIT(VDO_COL_FMT_YUV420) | BIT(VDO_COL_FMT_YUV422H) | BIT(VDO_COL_FMT_YUV444) | BIT(VDO_COL_FMT_ARGB)
							| VPP_FB_FLAG_MEDIA | VPP_FB_FLAG_FIELD;

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
	fb_p->flag = 0;

	mod_fb_p->set_framebuf = govm_set_pip_framebuf;
	mod_fb_p->set_addr = govm_set_pip_fb_addr;
	mod_fb_p->get_addr = govm_get_pip_fb_addr;
	mod_fb_p->get_color_fmt = govm_get_pip_color_format;
	mod_fb_p->fn_view = govm_pip_proc_view;
}
#endif
	return 0;
}
module_init(govm_mod_init);
#endif				//WMT_FTBLK_GOVM
