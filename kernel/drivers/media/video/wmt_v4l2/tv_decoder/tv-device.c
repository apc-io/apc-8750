/*++ 
 * linux/drivers/media/video/wmt_v4l2/tv_decoder/tv-device.c
 * WonderMedia v4l tv decoder device driver
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
#define TV_DEVICE_C

#include "../wmt-vid.h"
#include "tv-device.h"
#include <linux/module.h>

#define TV_DEV_I2C_ADDR     0x20
  
unsigned char ad7180_data[]={
/*0*/	0x04,0xC8,0x04,0x0C, 0x44,0x00,0x02,0x7F, 0x80,0x80,0x00,0x00, 0x36,0x7C,0x00,0x00,
/*1*/	0x4A,0x1B,0x00,0x08, 0x12,0x00,0x00,0x41, 0x93,0xF1,0x00,0x00, 0x00,0x40,0x00,0x00,
/*2*/	0x00,0x00,0x00,0xC0, 0x00,0x00,0x00,0x58, 0x00,0x00,0x00,0xE1, 0xAE,0xF2,0x00,0xF4,
/*3*/	0x00,0x1A,0x81,0x84, 0x00,0x00,0x7D,0xA0, 0x80,0xC0,0x10,0x05, 0x58,0xB2,0x64,0xE4,
			
/*4*/	0x90,0x01,0x7E,0xA4, 0xFF,0xB6,0x12,0x00, 0x00,0x00,0x00,0x00, 0x00,0xEF,0x08,0x08,
/*5*/	0x08,0x24,0x0B,0x4E, 0x80,0x00,0x10,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,/*0x58=0 field output,1 = field input*/
/*6*/	0x00,0x00,0x20,0x00, 0x00,0x00,0x00,0x03, 0x01,0x00,0x00,0xC0, 0x00,0x00,0x00,0x00,
/*7*/	0x00,0x00,0x00,0x10, 0x04,0x01,0x00,0x3F, 0xFF,0xFF,0xFF,0x1E, 0xC0,0x00,0x00,0x00,
			
/*8*/	0x00,0xC0,0x04,0x00, 0x0C,0x02,0x03,0x63, 0x5A,0x08,0x10,0x00, 0x40,0x00,0x40,0x00,
/*9*/	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 
/*A*/	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
/*B*/	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x02,
			
/*C*/	0x00,0x00,0x00,0x00, 0x00,0x81,0x00,0x00, 0x00,0x04,0x00,0x00, 0x69,0x00,0x01,0xB4,
/*D*/	0x00,0x10,0xFF,0xFF, 0x7F,0x7F,0x3E,0x08, 0x3C,0x08,0x3C,0x9B, 0xAC,0x4C,0x00,0x00,
/*E*/	0x14,0x80,0x80,0x80, 0x80,0x25,0x04,0x63, 0x65,0x14,0x63,0x55, 0x55,0x00,0x00,0x4A,
/*F*/	0x44,0x0C,0x32,0x00, 0x19,0xE0,0x69,0x10, 0x00,0x03,0xA0,0x40, 0x04,0x00,0x04,0x88		
};
#define AD_7180_DATA_SIZE 0x100

/*!*************************************************************************
* viddev_config(int mode)
* 
*  Public Function 
*  implement this function (interface) 
* 
*		
*  \retval none
*/
int viddev_config(int mode)
{

	static int vid_first_time=1;

	if(vid_first_time){
		vid_first_time=0;
//		ad_7180_init();
	}

	if(mode==VID601_S1_N){
		//DPRINTK("enter vid VID601_S1_N \n");
		wmt_vid_i2c_write(TV_DEV_I2C_ADDR, 0x31, 0x12);
		wmt_vid_i2c_write(TV_DEV_I2C_ADDR, 0x32, 0x81);
		wmt_vid_i2c_write(TV_DEV_I2C_ADDR, 0x33, 0x84);
		wmt_vid_i2c_write(TV_DEV_I2C_ADDR, 0x34, 0x00);
		wmt_vid_i2c_write(TV_DEV_I2C_ADDR, 0x35, 0x03);
		wmt_vid_i2c_write(TV_DEV_I2C_ADDR, 0x36, 0x00);
		wmt_vid_i2c_write(TV_DEV_I2C_ADDR, 0x37, 0x29);
		wmt_vid_i2c_write(TV_DEV_I2C_ADDR, 0xE5, 0x40);
		wmt_vid_i2c_write(TV_DEV_I2C_ADDR, 0xE6, 0x80);
		wmt_vid_i2c_write(TV_DEV_I2C_ADDR, 0xE7, 0x03);
		wmt_vid_i2c_write(TV_DEV_I2C_ADDR, 0x58, 0x00);
		
		//DPRINTK("\nDDR2 TV_DEV_I2C_ADDR Slave(Field)  1 NTSC!\n");
	}
	else if(mode==VID601_S2_N){ 
		wmt_vid_i2c_write(TV_DEV_I2C_ADDR, 0x31, 0x1A);
		wmt_vid_i2c_write(TV_DEV_I2C_ADDR, 0x32, 0x81);
		wmt_vid_i2c_write(TV_DEV_I2C_ADDR, 0x33, 0x84);
		wmt_vid_i2c_write(TV_DEV_I2C_ADDR, 0x34, 0x00);
		wmt_vid_i2c_write(TV_DEV_I2C_ADDR, 0x35, 0x03);
		wmt_vid_i2c_write(TV_DEV_I2C_ADDR, 0x36, 0x00);
		wmt_vid_i2c_write(TV_DEV_I2C_ADDR, 0x37, 0x29);
		wmt_vid_i2c_write(TV_DEV_I2C_ADDR, 0xE5, 0x40);
		wmt_vid_i2c_write(TV_DEV_I2C_ADDR, 0xE6, 0x80);
		wmt_vid_i2c_write(TV_DEV_I2C_ADDR, 0xE7, 0x03);
		wmt_vid_i2c_write(TV_DEV_I2C_ADDR, 0x58, 0x01);
		//DPRINTK("\nDDR2 TV_DEV_I2C_ADDR Slave(Vsync) 2 NTSC!\n");		
	}
	else if(mode==VID601_S1_P){
		wmt_vid_i2c_write(TV_DEV_I2C_ADDR, 0x31, 0x12);
		wmt_vid_i2c_write(TV_DEV_I2C_ADDR, 0x32, 0x81);
		wmt_vid_i2c_write(TV_DEV_I2C_ADDR, 0x33, 0x84);
		wmt_vid_i2c_write(TV_DEV_I2C_ADDR, 0x34, 0x00);
		wmt_vid_i2c_write(TV_DEV_I2C_ADDR, 0x35, 0x03);
		wmt_vid_i2c_write(TV_DEV_I2C_ADDR, 0x36, 0x00);
		wmt_vid_i2c_write(TV_DEV_I2C_ADDR, 0x37, 0x29);
		wmt_vid_i2c_write(TV_DEV_I2C_ADDR, 0xE8, 0x41);
		wmt_vid_i2c_write(TV_DEV_I2C_ADDR, 0xE9, 0x84);
		wmt_vid_i2c_write(TV_DEV_I2C_ADDR, 0xEA, 0x03);
		wmt_vid_i2c_write(TV_DEV_I2C_ADDR, 0x58, 0x00);
		//DPRINTK("\nDDR2 TV_DEV_I2C_ADDR Slave(Field) 1 PAL!\n");
	}
	else if(mode==VID601_S2_P){
		wmt_vid_i2c_write(TV_DEV_I2C_ADDR, 0x31, 0x12);
		wmt_vid_i2c_write(TV_DEV_I2C_ADDR, 0x32, 0x81);
		wmt_vid_i2c_write(TV_DEV_I2C_ADDR, 0x33, 0x84);
		wmt_vid_i2c_write(TV_DEV_I2C_ADDR, 0x34, 0x00);
		wmt_vid_i2c_write(TV_DEV_I2C_ADDR, 0x35, 0x03);
		wmt_vid_i2c_write(TV_DEV_I2C_ADDR, 0x36, 0x00);
		wmt_vid_i2c_write(TV_DEV_I2C_ADDR, 0x37, 0x29);
		wmt_vid_i2c_write(TV_DEV_I2C_ADDR, 0xE8, 0x41);
		wmt_vid_i2c_write(TV_DEV_I2C_ADDR, 0xE9, 0x84);
		wmt_vid_i2c_write(TV_DEV_I2C_ADDR, 0xEA, 0x03);
		wmt_vid_i2c_write(TV_DEV_I2C_ADDR, 0x58, 0x01);
		//DPRINTK("\nDDR2 TV_DEV_I2C_ADDR Slave(Vsync) 2 PAL!\n");		
	}
	
	return 0;
	
}
int tv_init_ext_device(int width, int height)
{
    char *array_addr;
    char  addr,data;
    int   array_size, i;
    char ret_data;
    int i2cval=0;

printk("tv_init_ext_device() W %d H %d\n",width,height);
    i2cval=wmt_vid_i2c_read(TV_DEV_I2C_ADDR,0x11);
    printk("ID val 0x%02x \n", i2cval);

#if 1
    if (width == 720)  {
    	 printk("\n");
        array_size = sizeof(ad7180_data);
        array_addr = ad7180_data;
    	 printk("array_size %d\n",array_size);
    }else {
        /* Not-supported Resolution */
        return -1;
    }
    for (i = 0; i <array_size ; i++){
        addr = i;
        data = array_addr[i];

        wmt_vid_i2c_write( TV_DEV_I2C_ADDR, addr, data);
      
    }
    #if 1 // SW_I2C_READ_ERR
    		while (1)
		{
			i2cval=wmt_vid_i2c_read(TV_DEV_I2C_ADDR,0x10);
			printk("i2cval 0x%02X \n",i2cval);
			if (i2cval&0x1)
				break;
		}
    #endif
    
#else

	     wmt_vid_i2c_write_page(TV_DEV_I2C_ADDR,0x0,ad7180_data,AD_7180_DATA_SIZE);
		printk("ad7180 start polling\n");
		while (1)
		{
			i2cval=wmt_vid_i2c_read(TV_DEV_I2C_ADDR,0x10);
			if (i2cval&0x1)
				break;
			printk("i2cval 0x%02X \n",i2cval);
		}
		printk("ad7180 end polling\n");

#endif
    
    viddev_config(VID656_N);
    
    return 0;
} /* End of cmos_exit_ov7670() */

/*!*************************************************************************
* viddev_getstatus(int mode)
* 
*  Public Function 
*  implement this function (interface) 
* 
*		
*  \retval none
*/
int viddev_get_status(int mode)
{
	char i2cval;
	int ret;

	if (mode==VIDDEV_STS_LOCK){
			i2cval = wmt_vid_i2c_read(TV_DEV_I2C_ADDR,0x10);
			if (i2cval & 0x1)
				ret=1;
			else
				ret=0;
				
	}else if  (mode==VIDDEV_STS_TVSYS){
			i2cval = wmt_vid_i2c_read(TV_DEV_I2C_ADDR,0x10);
			i2cval = (i2cval>>4)&0x7;

			if (i2cval==0x0)
				ret = VID_NTSC;
			else
				ret = VID_PAL;
			#if 0 // SW_I2C_READ_ERR
			   printk("tv i2cval %x\n",i2cval);
			   ret = VID_NTSC;
		      #endif
	}
	//printk("i2cval %x\n",i2cval);
	return ret;
	
}


/*------------------------------------------------------------------------------*/
/*--------------------End of Function Body -----------------------------------*/

#undef TV_DEVICE_C
