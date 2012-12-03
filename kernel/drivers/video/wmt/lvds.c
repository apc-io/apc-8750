/*++ 
 * linux/drivers/video/wmt/lvds.c
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

#define LVDS_C
// #define DEBUG
/*----------------------- DEPENDENCE -----------------------------------------*/
#include "lvds.h"

#ifdef WMT_FTBLK_LVDS
/*----------------------- PRIVATE MACRO --------------------------------------*/

/*----------------------- PRIVATE CONSTANTS ----------------------------------*/
/* #define LVDS_XXXX    1     *//*Example*/

/*----------------------- PRIVATE TYPE  --------------------------------------*/
/* typedef  xxxx lvds_xxx_t; *//*Example*/

/*----------EXPORTED PRIVATE VARIABLES are defined in lvds.h  -------------*/
/*----------------------- INTERNAL PRIVATE VARIABLES - -----------------------*/
/* int  lvds_xxx;        *//*Example*/

/*--------------------- INTERNAL PRIVATE FUNCTIONS ---------------------------*/
/* void lvds_xxx(void); *//*Example*/

/*----------------------- Function Body --------------------------------------*/
void lvds_set_power_down(int pwrdn)
{
	vppif_reg32_write(LVDS_PD,pwrdn);
}

void lvds_set_enable(vpp_flag_t enable)
{
	vppif_reg32_write(LVDS_CTL,(enable)? 2:0);	// LVDS(0x2) or HDMI(0x0)
	vppif_reg32_write(LVDS_TRE_EN,(enable)?0:1);
}

void lvds_set_rgb_type(int mode)
{
	/* 0:888, 1-555, 2-666, 3-565 */
	vppif_reg32_write(LVDS_IGS_BPP_TYPE,mode);
}

vdo_color_fmt lvds_get_colfmt(void)
{
	return VDO_COL_FMT_ARGB;
}

/*----------------------- Module API --------------------------------------*/
void lvds_reg_dump(void)
{
	DPRINT("========== LVDS register dump ==========\n");
	vpp_reg_dump(REG_LVDS_BEGIN,REG_LVDS_END-REG_LVDS_BEGIN);

	DPRINT("---------- LVDS common ----------\n");
	DPRINT("test %d,dual chan %d,inv clk %d\n",vppif_reg32_read(LVDS_TEST),vppif_reg32_read(LVDS_DUAL_CHANNEL),vppif_reg32_read(LVDS_INV_CLK));
	DPRINT("ldi shift left %d,IGS bpp type %d\n",vppif_reg32_read(LVDS_LDI_SHIFT_LEFT),vppif_reg32_read(LVDS_IGS_BPP_TYPE));
	DPRINT("rsen %d,pll ready %d\n",vppif_reg32_read(LVDS_RSEN),vppif_reg32_read(LVDS_PLL_READY));
}

#ifdef CONFIG_PM
static unsigned int *lvds_pm_bk;
void lvds_suspend(int sts)
{
	switch( sts ){
		case 0:	// disable module
			break;
		case 1: // disable tg
			break;
		case 2:	// backup register
			lvds_pm_bk = vpp_backup_reg(REG_LVDS_BEGIN,(REG_LVDS_END-REG_LVDS_BEGIN));
			break;
		default:
			break;
	}
}

void lvds_resume(int sts)
{
	switch( sts ){
		case 0:	// restore register
			vpp_restore_reg(REG_LVDS_BEGIN,(REG_LVDS_END-REG_LVDS_BEGIN),lvds_pm_bk);
			lvds_pm_bk = 0;
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
#define lvds_suspend NULL
#define lvds_resume NULL
#endif

void lvds_init(void)
{
	vppif_reg32_write(LVDS_REG_LEVEL,1);
	vppif_reg32_write(LVDS_REG_UPDATE,1);
	vppif_reg32_write(LVDS_PLL_R_F,0x1);
	vppif_reg32_write(LVDS_PLL_CPSET,0x1);
	vppif_reg32_write(LVDS_TRE_EN,0x1);

	// patch for A1 
	vppif_reg32_write(LVDS_TEST,0x2);
	vppif_reg32_write(LVDS_LDI_SHIFT_LEFT,1);
}

#endif /* WMT_FTBLK_LVDS */

