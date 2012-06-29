/*++ 
 * linux/drivers/video/wmt/vt1632.c
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

#define VT1632_C
// #define DEBUG
/*----------------------- DEPENDENCE -----------------------------------------*/
#include "../vout.h"

/*----------------------- PRIVATE MACRO --------------------------------------*/
/* #define  VT1632_XXXX  xxxx    *//*Example*/

/*----------------------- PRIVATE CONSTANTS ----------------------------------*/
/* #define VT1632_XXXX    1     *//*Example*/
#define VT1632_ADDR 0x10

/*----------------------- PRIVATE TYPE  --------------------------------------*/
/* typedef  xxxx vt1632_xxx_t; *//*Example*/

/*----------EXPORTED PRIVATE VARIABLES are defined in vt1632.h  -------------*/
/*----------------------- INTERNAL PRIVATE VARIABLES - -----------------------*/
/* int  vt1632_xxx;        *//*Example*/
static int vt1632_not_ready;

/*--------------------- INTERNAL PRIVATE FUNCTIONS ---------------------------*/
/* void vt1632_xxx(void); *//*Example*/

/*----------------------- Function Body --------------------------------------*/

/*the define and struct i2c_msg were declared int linux/i2c.h*/
int vt1632_check_plugin(int hotplug)
{
	unsigned char buf[1];
	int plugin;

	if( vt1632_not_ready )
		return 1;
	
	vpp_i2c_read(VT1632_ADDR,0x9,buf,1);
	plugin = (buf[0]&0x4)? 1:0;
	DPRINT("[VT1632] DVI plug%s\n",(plugin)?"in":"out");
	vout_set_int_type((plugin)?2:3);
	return plugin;
}

int vt1632_init(void)
{
	unsigned char buf[16];

	vt1632_not_ready = 1;
	vpp_i2c_read(VT1632_ADDR, 0x0, buf, 2);
	if( (buf[0] != 0x06) || (buf[1] != 0x11) ){	// check vender id
		return -1;
	}
	vt1632_not_ready = 0;

	buf[0x0] = 0x37;
	buf[0x1] = 0x20;
	vpp_i2c_write(VT1632_ADDR,0x8,buf,2);
	DPRINT("[VT1632] DVI ext device\n");
	return 0;
}

int vt1632_set_mode(unsigned int *option)
{
	unsigned char buf[1];
	vpp_datawidht_t dwidth;

	if( vt1632_not_ready )
		return -1;

	dwidth = option[1];
	DBGMSG("vt1632_set_mode(%d)\n",(dwidth)?24:12);

	vpp_i2c_read(VT1632_ADDR,0x8,buf,1);
	if( dwidth == VPP_DATAWIDHT_12 ){
		buf[0] &= ~BIT2;
		buf[0] |= BIT3;
	}
	else {
		buf[0] |= BIT2;
		buf[0] &= ~BIT3;		
	}
	vpp_i2c_write(VT1632_ADDR,0x8,buf,1);
	return 0;
}

void vt1632_set_power_down(int enable)
{
	unsigned char buf[1];

	if( vt1632_not_ready )
		return;

	DBGMSG("vt1632_set_power_down(%d)\n",enable);

	vpp_i2c_read(VT1632_ADDR,0x8,buf,1);
	if( enable ){
		buf[0] &= ~BIT0;
	}
	else {
		buf[0] |= BIT0;
	}
	vpp_i2c_write(VT1632_ADDR,0x8,buf,1);
}

int vt1632_config(vout_info_t *info)
{
	return 0;
}

int vt1632_get_edid(char *buf)
{
	return 0;
}

/*----------------------- vout device plugin --------------------------------------*/
vout_dev_ops_t vt1632_vout_dev_ops = {
	.mode = VOUT_DVI,

	.init = vt1632_init,
	.set_power_down = vt1632_set_power_down,
	.set_mode = vt1632_set_mode,
	.config = vt1632_config,
	.check_plugin = vt1632_check_plugin,
	.get_edid = vt1632_get_edid,
};

int vt1632_module_init(void)
{	
	vout_device_register(&vt1632_vout_dev_ops);
	return 0;
} /* End of vt1632_module_init */
module_init(vt1632_module_init);
/*--------------------End of Function Body -----------------------------------*/
#undef VT1632_C

