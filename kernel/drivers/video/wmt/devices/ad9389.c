/*++ 
 * linux/drivers/video/wmt/ad9389.c
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

#define AD9389_C
//#define DEBUG
/*----------------------- DEPENDENCE -----------------------------------------*/
#include "../vout.h"
#include <linux/delay.h>

/*----------------------- PRIVATE MACRO --------------------------------------*/
/* #define  AD9389_XXXX  xxxx    *//*Example*/

/*----------------------- PRIVATE CONSTANTS ----------------------------------*/
/* #define AD9389_XXXX    1     *//*Example*/
#define AD9389_ADDR 	0x72
#define AD9389_REG_NUM	203

/*----------------------- PRIVATE TYPE  --------------------------------------*/
/* typedef  xxxx ad9389_xxx_t; *//*Example*/
typedef enum {
	AD9389_VOMODE_HDMI_RGB_12,
	AD9389_VOMODE_HDMI_RGB_24,
	AD9389_VOMODE_HDMI_YUV444_12,
	AD9389_VOMODE_HDMI_YUV444_24,
	AD9389_VOMODE_HDMI_YUV422_24,
	AD9389_VOMODE_MAX
} ad9389_output_mode_t;

/*----------EXPORTED PRIVATE VARIABLES are defined in ad9389.h  -------------*/
/*----------------------- INTERNAL PRIVATE VARIABLES - -----------------------*/
/* int  ad9389_xxx;        *//*Example*/
const unsigned char ad9389_reg_yuv444_12[AD9389_REG_NUM]={
	3,0,24,0,33,33,192,0,0,0			//0~9
	,9,14,60,24,1,19,37,55,0,0			//10~19
	,0,10,67,2,6,98,4,168,0,0			//20~29
	,28,132,28,191,4,168,30,112,2,30	//30~39
	,0,0,4,168,8,18,27,172,0,0			//40~49
	,0,0,0,0,0,0,0,0,0,128				//50~59
	,0,4,16,0,0,16,224,126,248,32		//60~69
	,0,0,0,0,0,0,0,0,0,0				//70~79
	,0,0,0,0,0,0,0,0,0,0				//80~89
	,0,0,0,0,0,0,0,0,0,0				//90~99
	,0,0,0,0,0,0,0,0,0,0				//100~109
	,0,0,0,0,0,0,0,0,0,0				//110~119
	,0,0,0,0,0,0,0,0,0,0				//120~129
	,0,0,0,0,0,0,0,0,0,0				//130~139
	,0,0,0,0,0,0,0,0,192,0				//140~149
	,32,4,3,2,0,24,56,97,0,0			//150~159	//9e(158) and 9f(159) don't care
	,0,0,135,135,8,192,0,0,0,0			//160~169	//a0(158) don't care
	,0,0,0,0,64,22,147,1,213,56			//170~179
	,218,221,113,240,224,0,96,255,187,165		//180~189
	,144,131,105,220,148,122,0,112,30,0,36,3,	//190~202
};

static vdo_color_fmt ad9389_colfmt = VDO_COL_FMT_ARGB;
static vpp_datawidht_t ad9389_dwidth = VPP_DATAWIDHT_12;
static int ad9389_spdif_enable = 0;

/*--------------------- INTERNAL PRIVATE FUNCTIONS ---------------------------*/
/* void ad9389_xxx(void); *//*Example*/
int ad9389_set_mode(unsigned int *option);

/*----------------------- Function Body --------------------------------------*/
int ad9389_check_plugin(int hotplug)
{
	unsigned char buf[1];
	int plugin;
	unsigned int option[2];
	
	vpp_i2c_read(AD9389_ADDR,0x42,buf,1);
	plugin = (buf[0] & BIT6)? 1:0;
	printk("[AD9389] HDMI plug%s\n",(plugin)?"in":"out");

	buf[0] = 0x80;
	vpp_i2c_write(AD9389_ADDR,0x94,buf,1);	// enable INT
	buf[0] = 0x80;
	vpp_i2c_write(AD9389_ADDR,0x96,buf,1);	// clear HPD flag
	vout_set_int_type(2);	// GPIO0 3:rising edge, 2:falling edge

	if( !hotplug ) return plugin;

	option[0] = ad9389_colfmt;
	option[1] = ad9389_dwidth;
	option[1] |= (ad9389_spdif_enable)? 0x2:0x0;
	if( plugin ){
		do {
			buf[0] = 0x10;
			vpp_i2c_write(AD9389_ADDR,0x41,buf,1);
			vpp_i2c_read(AD9389_ADDR,0x41,buf,1);
			if((buf[0] & BIT6)==0)	// Power up 
				break;

			vpp_i2c_read(AD9389_ADDR,0x42,buf,1);
			plugin = (buf[0] & BIT6)? 1:0;
			if( plugin == 0 ) 
				return 0;			
		} while(1);
		
		ad9389_set_mode(&option[0]);
	}
	return plugin;
}

void ad9389_set_power_down(int enable)
{
	char reg;

	vpp_i2c_read(AD9389_ADDR,0x41,&reg,1);
	if( enable ){
		reg |= 0x40;
	}
	else {
		reg &= ~0x40;
	}
	vpp_i2c_write(AD9389_ADDR,0x41,&reg,1);
}

int ad9389_set_mode(unsigned int *option)
{
	vdo_color_fmt colfmt;
	vpp_datawidht_t dwidth;
	ad9389_output_mode_t mode;
	unsigned char wr_buf[AD9389_REG_NUM];

	colfmt = option[0];
	dwidth = option[1] & BIT0;
	
	ad9389_spdif_enable = ( option[1] & BIT1 )? 1:0;
	ad9389_colfmt = colfmt;
	ad9389_dwidth = dwidth;
	switch(colfmt){
		case VDO_COL_FMT_ARGB:
			mode = (dwidth == VPP_DATAWIDHT_12)? AD9389_VOMODE_HDMI_RGB_12:AD9389_VOMODE_HDMI_RGB_24;
			break;
		case VDO_COL_FMT_YUV444:
			mode = (dwidth == VPP_DATAWIDHT_12)? AD9389_VOMODE_HDMI_YUV444_12:AD9389_VOMODE_HDMI_YUV444_24;
			break;
		case VDO_COL_FMT_YUV422H:
			mode = (dwidth == VPP_DATAWIDHT_12)? AD9389_VOMODE_MAX:AD9389_VOMODE_HDMI_YUV422_24;
			break;
		default:
			mode = AD9389_VOMODE_MAX;
			break;
	}

	memcpy(wr_buf,ad9389_reg_yuv444_12,AD9389_REG_NUM);
	switch(mode){
		case AD9389_VOMODE_HDMI_RGB_12:
			DBGMSG("HDMI RGB 12bit mode\n");
			wr_buf[0x15] = 0x0A;
			wr_buf[0x16] = 0x02;
			wr_buf[0x45] = 0x0;
//			wr_buf[0x97] = 0x4;
//			wr_buf[0x98] = 0x3;
//			wr_buf[0xA5] = 0xC0;
			wr_buf[0xAF] = 0x16;
			wr_buf[0xBA] = 0xC0;
			break;
		case AD9389_VOMODE_HDMI_YUV444_12:
			DBGMSG("HDMI YUV444 12bit mode\n");
			wr_buf[0x15] = 0x0A;
			wr_buf[0x16] = 0x43;
			wr_buf[0x45] = 0x20;
//			wr_buf[0x97] = 0x4;
//			wr_buf[0x98] = 0x3;
//			wr_buf[0xA5] = 0xC0;
			wr_buf[0xAF] = 0x16;
			wr_buf[0xBA] = 0xC0;
			break;
		case AD9389_VOMODE_HDMI_RGB_24:
			DBGMSG("HDMI RGB 24bit mode\n");			
			wr_buf[0x15] = 0x0;
			wr_buf[0x16] = 0x02;
			wr_buf[0x45] = 0x0;
//			wr_buf[0x97] = 0x4;
//			wr_buf[0x98] = 0x3;
//			wr_buf[0xA5] = 0xC0;
			wr_buf[0xAF] = 0x16;
			wr_buf[0xBA] = 0x60;
			break;
		case AD9389_VOMODE_HDMI_YUV444_24:
			DBGMSG("HDMI YUV444 24bit mode\n");
			wr_buf[0x15] = 0x0;
			wr_buf[0x16] = 0x43;
			wr_buf[0x45] = 0x20;
//			wr_buf[0x97] = 0x4;
//			wr_buf[0x98] = 0x3;
//			wr_buf[0xA5] = 0xC0;
			wr_buf[0xAF] = 0x16;
			wr_buf[0xBA] = 0x60;
			break;
		case AD9389_VOMODE_HDMI_YUV422_24:
			DBGMSG("HDMI YUV422 24bit mode\n");
			wr_buf[0x15] = 0x02;
			wr_buf[0x16] = 0xCB;
			wr_buf[0x45] = 0x10;
//			wr_buf[0x97] = 0x4;
//			wr_buf[0x98] = 0x3;
//			wr_buf[0xA5] = 0xC0;
			wr_buf[0xAF] = 0x16;
			wr_buf[0xBA] = 0x60;
			break;
		default:
			DBGMSG("*E* invalid mode\n");
			return -1;
	}

	if( ad9389_spdif_enable ){
		wr_buf[0xA] |= 0x10;	// 0x9 i2s, 0x19 spdif
	}
	wr_buf[0x94] = 0x80;
	wr_buf[0x96] = 0x80;

#if 1
	vpp_i2c_write(AD9389_ADDR, 0x0, wr_buf, AD9389_REG_NUM);
#else
	ad9389_write_reg(0x15,&wr_buf[0x15],1);
	ad9389_write_reg(0x16,&wr_buf[0x16],1);
	ad9389_write_reg(0x45,&wr_buf[0x45],1);
	ad9389_write_reg(0x97,&wr_buf[0x97],1);
	ad9389_write_reg(0x98,&wr_buf[0x98],1);
	ad9389_write_reg(0xA5,&wr_buf[0xA5],1);
	ad9389_write_reg(0xAF,&wr_buf[0xAF],1);
	ad9389_write_reg(0xBA,&wr_buf[0xBA],1);
#endif
	return 0;
}	

int ad9389_config(vout_info_t *info)
{
	char buf[1];

	buf[0] = 0x80;
	if( ad9389_colfmt == VDO_COL_FMT_YUV422H ){
		switch( info->resx ){
			case 1440:	// 1440x480i/p, 1440x576i
				buf[0] = 0xC0;
				break;
			default:
				break;
		}
	}
	vpp_i2c_write(AD9389_ADDR, 0x3B, buf, 1);
	return 0;
}

int ad9389_get_edid(char *buf)
{
	mdelay(50);
	DBGMSG("get edid\n");

	vpp_i2c_read(0x7E, 0x0, buf, 128);
	if( buf[0x7F] ){
		vpp_i2c_read(0x7E, 128, &buf[128], 128);
	}
	return 0;
}

int ad9389_init(void)
{
	unsigned char buf[2];

	buf[0] = 0xff;
	vpp_i2c_read(0x72, 0xcf, buf, 1);
	if( buf[0] != 0x70 ){
		return -1;
	}
	DPRINT("[AD9389] HDMI ext device\n");
	return 0;
}
/*----------------------- vout device plugin --------------------------------------*/
vout_dev_ops_t ad9389_vout_dev_ops = {
	.mode = VOUT_DVO2HDMI,

	.init = ad9389_init,
	.set_power_down = ad9389_set_power_down,
	.set_mode = ad9389_set_mode,
	.config = ad9389_config,
	.check_plugin = ad9389_check_plugin,
	.get_edid = ad9389_get_edid,
};

static int ad9389_module_init(void)
{	
	vout_device_register(&ad9389_vout_dev_ops);
	return 0;
} /* End of ad9389_module_init */
module_init(ad9389_module_init);
/*--------------------End of Function Body -----------------------------------*/
#undef AD9389_C
