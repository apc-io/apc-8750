/*++ 
 * linux/drivers/video/wmt/ad7393.c
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

#define AD7393_C
// #define DEBUG
/*----------------------- DEPENDENCE -----------------------------------------*/
#include "../vout.h"

/*----------------------- PRIVATE MACRO --------------------------------------*/
/* #define  AD7393_XXXX  xxxx    *//*Example*/

/*----------------------- PRIVATE CONSTANTS ----------------------------------*/
/* #define AD7393_XXXX    1     *//*Example*/
#define AD7393_ADDR 	0x54
#define AD7393_REG_NUM	208

/*----------------------- PRIVATE TYPE  --------------------------------------*/
/* typedef  xxxx ad7393_xxx_t; *//*Example*/

/*----------EXPORTED PRIVATE VARIABLES are defined in ad7393.h  -------------*/
/*----------------------- INTERNAL PRIVATE VARIABLES - -----------------------*/
/* int  ad7393_xxx;        *//*Example*/
unsigned char AD7393_NTSC_INIT_REG[] ={                                                                                                                                                    
/*      00   01   02   03   04   05   06   07   08   09   0A   0B   0C   0D   0E   0F */       
/*00*/ 0x1C,0x00,0x20,0x03,0xF0,0x4E,0x0E,0x24,0x92,0x7C,0x00,0x00,0x00,0x00,0x00,0x00,        
/*10*/ 0x10,0x1B,0xFF,0x01,0xFF,0x3C,0x5F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,        
/*20*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,        
/*30*/ 0x00,0x00,0x00,0x68,0x48,0x00,0xA0,0x80,0x80,0x02,0x00,0x00,0x00,0x00,0x00,0x00,        
/*40*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,        
/*50*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,        
/*60*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,        
/*70*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,        
/*80*/ 0x50,0x00,0xCB,0x04,0x00,0x00,0x02,0x00,0x00,0x00,0x08,0x00,0x1F,0x7C,0xF0,0x21,        
/*90*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,        
/*A0*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*B0*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3B,0x00,0x42,0x81,0x19,
/*C0*/ 0x10,0x70,0x5E,0x12,0x80,0x26,0x4A,0x70,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00 
};

/*--------------------- INTERNAL PRIVATE FUNCTIONS ---------------------------*/
/* void ad7393_xxx(void); *//*Example*/

/*----------------------- Function Body --------------------------------------*/

/*--------------------End of Function Body -----------------------------------*/
int ad7393_check_plugin(int hotplug)
{
#if 1
	return 1;
#else
	char reg;
	int plugin;

	vpp_i2c_read(AD7393_ADDR,0x10,&reg,1);
	plugin = (reg & 0x1)? 0:1;	/* bit0: Y, bit1: C */
	if( (reg & 0x02) == 0 ){
		plugin |= 0x2;
	}
	printk("[AD7393] 0x%x, plug%s\n",reg,(plugin)?"in":"out");
	return plugin;
#endif	
}

void ad7393_set_power_down(int enable)
{
	char reg;

	DBGMSG("power down %d\n",enable);

	if( enable ){
		reg = 0x3;
	}
	else {
		reg = 0x1C;
	}
	vpp_i2c_write(AD7393_ADDR,0x0,&reg,1);
}

int ad7393_set_mode(unsigned int *option)
{
	int i;
	unsigned char wr_buf[AD7393_REG_NUM];
	vpp_tvsys_t tvsys;
	vpp_tvconn_t tvconn;

	tvsys = option[0];
	tvconn = option[1];
	DBGMSG("set mode : tvsys %d,tvconn %d\n",tvsys,tvconn);

	wr_buf[0] = 0x2;
	vpp_i2c_write(AD7393_ADDR, 0x17, wr_buf, 1 );	// reset

	for(i=0;i<1000;i++);

	memcpy(wr_buf,AD7393_NTSC_INIT_REG,AD7393_REG_NUM);
	switch(tvconn){
		case VPP_TVCONN_YCBCR:
		case VPP_TVCONN_YPBPR:
			wr_buf[0x82] = 0xC9;
			break;			
		case VPP_TVCONN_SCART:
		case VPP_TVCONN_VGA:
			wr_buf[0x02] = 0x10;
			wr_buf[0x82] = 0xC9;
			break;
		default:
		case VPP_TVCONN_SVIDEO:
		case VPP_TVCONN_CVBS:
			wr_buf[0x82] = 0xCB;			
			break;
	}

	if( tvsys == VPP_TVSYS_PAL ){
		wr_buf[0x80] = 0x51;
		wr_buf[0x82] &= ~0x8;
		wr_buf[0x8c] = 0xCB;
		wr_buf[0x8d] = 0x8A;
		wr_buf[0x8e] = 0x09;
		wr_buf[0x8f] = 0x2A;
	}

	vpp_i2c_write(AD7393_ADDR, 0x0, wr_buf, AD7393_REG_NUM);
	return 0;
}	

int ad7393_init(void)
{
	unsigned char buf[2];

	buf[0] = 0xff;
	vpp_i2c_read(AD7393_ADDR, 0x0, buf, 1);
	if( (buf[0] == 0x0) || (buf[0] == 0xff) ){
		return -1;
	}
	DPRINT("[AD7393] TV ext device\n");
	return 0;
}

int ad7393_config(vout_info_t *info)
{
	return 0;
}

int ad7393_get_edid(char *buf)
{
	return 0;
}

/*----------------------- vout device plugin --------------------------------------*/
vout_dev_ops_t ad7393_vout_dev_ops = {
	.mode = VOUT_SD_DIGITAL,

	.init = ad7393_init,
	.set_power_down = ad7393_set_power_down,
	.set_mode = ad7393_set_mode,
	.config = ad7393_config,
	.check_plugin = ad7393_check_plugin,
	.get_edid = ad7393_get_edid,
};

static int ad7393_module_init(void)
{	
	vout_device_register(&ad7393_vout_dev_ops);
	return 0;
} /* End of ad7393_module_init */
module_init(ad7393_module_init);
/*--------------------End of Function Body -----------------------------------*/

#undef AD7393_C

