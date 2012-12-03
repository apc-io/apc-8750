/*++ 
 * linux/drivers/media/video/wmt_v4l2/cmos/cmos-dev.c
 * WonderMedia v4l cmos device driver
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
#define CMOS_DEV_C

#include "../wmt-vid.h"
#include "cmos-dev.h"
#include "cmos-dev-ov.h"
#include "cmos-dev-siv.h"
#include "cmos-dev-gc.h"

#include <linux/module.h>

//#define CMOS_OV_DEBUG    /* Flag to enable debug message */
//#define CMOS_OV_TRACE

#ifdef CMOS_OV_DEBUG
  #define DBG(fmt, args...)    PRINT("{%s} " fmt, __FUNCTION__ , ## args)
#else
  #define DBG(fmt, args...)
#endif


#ifdef CMOS_OV_TRACE
  #define TRACE(fmt, args...)      PRINT("{%s}:  " fmt, __FUNCTION__ , ## args)
#else
  #define TRACE(fmt, args...) 
#endif

#define DBG_ERR(fmt, args...)      PRINT("*E* {%s} " fmt, __FUNCTION__ , ## args)




typedef struct  {
	int (*init)( int width, int height, int cmos_v_flip, int cmos_h_flip);
	int (*exit)(void);
	int (*identify)(void);
} cmos_dev_ops_t;



//cmos_dev_ops_t cmos_dev_ov_7675_ops;

cmos_dev_ops_t cmos_dev_ov7675_ops = {
	
	.init = cmos_init_ov7675,
	.identify = cmos_ov7675_identify,
	.exit = cmos_exit_ov7675,	
};

cmos_dev_ops_t cmos_dev_ov2640_ops = {
	
	.init = cmos_init_ov2640,
	.identify = cmos_ov2640_identify,
	.exit = cmos_exit_ov2640,	
};

cmos_dev_ops_t cmos_dev_gc0308_ops = {
	
	.init = cmos_init_gc0308,
	.identify = cmos_gc0308_identify,
	.exit = cmos_exit_gc0308,	
};

cmos_dev_ops_t cmos_dev_gc0307_ops = {
	
	.init = cmos_init_gc0307,
	.identify = cmos_gc0307_identify,
	.exit = cmos_exit_gc0307,	
};

cmos_dev_ops_t cmos_dev_siv120b_ops = {
	
	.init = cmos_init_siv120b,
	.identify = cmos_siv120b_identify,
	.exit = cmos_exit_siv120b,	
};

cmos_dev_ops_t cmos_dev_siv120d_ops = {
	
	.init = cmos_init_siv120d,
	.identify = cmos_siv120d_identify,
	.exit = cmos_exit_siv120d,	
};

#define CMOS_DEV_TOT_NUM 6

cmos_dev_ops_t *cmos_dev_array[CMOS_DEV_TOT_NUM] = 
{  
    &cmos_dev_ov7675_ops , &cmos_dev_gc0308_ops, &cmos_dev_siv120d_ops,
    &cmos_dev_siv120b_ops,  &cmos_dev_gc0307_ops, &cmos_dev_ov2640_ops,
};


unsigned char cmos_v_flip = 0;
unsigned char cmos_h_flip = 0;



int cmos_exit_device(int width, int height)
{
    return 0;
} /* End of cmos_exit_device() */






int cmos_init_ext_device(int width, int height,int v_flip,int h_flip)
{
       cmos_dev_ops_t  *cmos_dev_ops;
	int i = 0;

	cmos_v_flip = v_flip;
	cmos_h_flip = h_flip;

      printk(">> cmos_init_ext_devices() \n");


      for (i = 0 ; i< CMOS_DEV_TOT_NUM ;i++)
      	{
      	
      		cmos_dev_ops = cmos_dev_array[i];
      			
		if (cmos_dev_ops->identify() == 0)
		{
			printk(" init cmos device %d\n" , i);
			cmos_dev_ops->init(width, height, cmos_v_flip, cmos_h_flip);
			return 0;
		}
      	}
       printk("Dont find any cmos device\n");
       return -1;
       

} /* End of cmos_exit_ov7670() */




/*------------------------------------------------------------------------------*/
/*--------------------End of Function Body -----------------------------------*/

#undef CMOS_DEV_C
