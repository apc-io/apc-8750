/*++
	drivers/i2c/busses/wmt_i2c_api.c

	Copyright (c) 2008 WonderMedia Technologies, Inc.

	This program is free software: you can redistribute it and/or modify it under the
	terms of the GNU General Public License as published by the Free Software Foundation,
	either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT
	ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
	PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	You should have received a copy of the GNU General Public License along with
	this program.  If not, see <http://www.gnu.org/licenses/>.

	WonderMedia Technologies, Inc.
	10F, 529, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C.
--*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/i2c.h>
#include <linux/i2c-id.h>
#include <linux/init.h>
#include <linux/time.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/interrupt.h>

#include <mach/hardware.h>
#include <asm/irq.h>
#include <mach/irqs.h>

 
#ifdef __KERNEL__

#undef DEBUG

#ifdef DEBUG
	#define DPRINTK		printk
#else
	#define DPRINTK(x...)
#endif 

#else
	#define DPRINTK    	printf

#endif
/*----------------------- INTERNAL PRIVATE MARCOS -------------------------*/
#define WMT_I2C_R 1
#define WMT_I2C_W 0
#define CTRL_GPIO21 GPIO_CTRL_GP21_I2C_BYTE_ADDR   
#define PU_EN_GPIO21 GPIO_PULL_EN_GP21_I2C_BYTE_ADDR
#define PU_CTRL_GPIO21 GPIO_PULL_CTRL_GP21_I2C_BYTE_ADDR


/*----------------------- INTERNAL PRIVATE FUNCTIONS -------------------------*/
int  wmt_i2c_api_attach(struct i2c_adapter *adapter);

int  wmt_i2c_api_probe(struct i2c_client *client, const struct i2c_device_id *id);
int  wmt_i2c_api_detach(struct i2c_client *client);
int  wmt_i2c_api_init(void) ;
void wmt_i2c_api_exit(void) ;
static int wmt_i2c_api_suspend(struct device *dev, pm_message_t state);
static int wmt_i2c_api_resume(struct device *dev);
extern unsigned int wmt_i2c0_is_master;
extern unsigned int wmt_i2c1_is_master;



/*----------------------- SPECIFY A MINOR NUMBER -------------------------------
* The id should be a unique ID. The range 0xf000 to 0xffff is reserved for local
* use, and you can use one of those until you start distributing the driver.
*/
#define 	I2C_DRIVER_ID_WMT_I2C_API		0xf055		/*default address, Should be changed*/


/*----------------------- EXTRA CLIENT DATA ------------------------------------
* The client structure has a special `data' field that can point to any
* structure at all. You can use this to keep client-specific data.
*/
struct wmt_i2c_api_data 
{		
	unsigned char	chip_rev ;
};


/*----------------------- CLIENT ADDRESS OF I2C SLAVE CHIP -------------------*/

static const unsigned short normal_addr[] = { WMT_I2C_API_I2C_ADDR, I2C_CLIENT_END };
static const unsigned short ignore[] 		= { I2C_CLIENT_END };
/*static unsigned short forces[] 		= { I2C_CLIENT_END };*/

static struct i2c_client_address_data addr_data = 
{
	.normal_i2c			= normal_addr,
	.probe				= ignore,
	.ignore				= ignore,
	.forces				= NULL,
};


/*----------------------- THE DRIVER STRUCTURE ---------------------------------
* A driver structure contains general access routines.
*/
static struct i2c_driver wmt_i2c_api_driver ;

static const struct i2c_device_id wmt_id[] = {
        {"WMT-I2C-API", 0},
        {} 
};      
MODULE_DEVICE_TABLE(i2c, wmt_id);

static struct i2c_driver wmt_i2c_api_driver = {
	.id_table		= wmt_id,
	.attach_adapter = wmt_i2c_api_attach,
	/*
	.detach_client	= wmt_i2c_api_detach,
	*/
	.remove 	= wmt_i2c_api_detach,
	.probe		= wmt_i2c_api_probe,
 	.command		= NULL,
	.driver = {
		.name   = "WMT-I2C-API",
		.owner = THIS_MODULE,
 		.suspend = wmt_i2c_api_suspend,
 		.resume = wmt_i2c_api_resume,
        },
	.address_data = &addr_data,
};

/*----------------------- DEFINE I2C CLIENT ------------------------------------
* A client structure specific information like the actual I2C address.
*/
static struct i2c_client *i2c_api_client[2];


/*----------------------- INTERNAL PRIVATE VARIABLES -------------------------*/
/*
static int wmt_i2c_api_i2c_id = 0;
*/
static int wmt_i2c_api_i2c_initialized = 0;
static int wmt_i2c_api_client_count = 0;

struct i2c_regs_s wmt_i2c_reg ;
struct i2c_regs_s wmt_i2c1_reg ;
static volatile struct i2c_regs_s *wmt_i2c_reg_addr = (struct i2c_regs_s *) I2C1_BASE_ADDR;
static volatile struct i2c_regs_s *wmt_i2c1_reg_addr = (struct i2c_regs_s *) I2C0_BASE_ADDR;
static unsigned int  wmt_i2c_suspend_flag = 0;
/*----------------------- PRIVATE CONSTANTS ----------------------------------*/



/*----------------------- Function Body --------------------------------------*/
/*************************************************************************
* i2c_api_register_write - 
* 
* RETURNS: 
* 
*/
void i2c_api_register_write(int index, u8 data)
{
	int ret, retry ;
	
	unsigned char p_data[2] ;
	struct i2c_msg wr[1] ;

	if(i2c_api_client[0] == NULL) {		
		DPRINTK("[i2c_api_register_write] i2c_api_client is NULL \n");
		return ;
	}
    
	p_data[0] = (unsigned char)index ;
	p_data[1] = data ;

	wr[0].addr  = (*i2c_api_client)->addr ;
	wr[0].flags = WMT_I2C_W ;
	wr[0].len   = 2 ;
	wr[0].buf   = p_data ;

	retry =3;
	while(retry){
		ret = i2c_transfer((*i2c_api_client)->adapter, wr, 1);
		if (ret != 1) {
			retry--;
			DPRINTK("[i2c_api_register_write: retry] %d times left \n", retry);
		} else
			retry=0;
	} 
		
	if (ret != 1) {
		DPRINTK("[i2c_api_register_write] write fail \n");
		return ;
	}
	DPRINTK("[i2c_api_register_write] OK! \n");
	return ;
}
/*************************************************************************
* i2c_api_page_write - 
* 
* RETURNS: 
* 
*/
void i2c_api_page_write(int index, u8 data[], int count)
{
	int  i,ret, retry ;

	unsigned char p_data[count +1] ; /*pdata[count +1]*/
	struct i2c_msg wr[1] ;

	if(*i2c_api_client == NULL) {		
		DPRINTK("[i2c_api_page_write] i2c_api_client is NULL \n");
		return ;
	}

	p_data[0] = (unsigned char)index ;
	for (i = 0 ; i < count; i++)
		;
	p_data[1+i] = data[i] ;

	wr[0].addr  = (*i2c_api_client)->addr ;
	wr[0].flags = WMT_I2C_W ;
	wr[0].len   = count+1 ;
	wr[0].buf   = p_data ;

	retry =3;
	while(retry){
		ret = i2c_transfer((*i2c_api_client)->adapter, wr, 1);
		DPRINTK("[i2c_api_page_write: retry] ret =%d \n", ret);
		if (ret != 1) {
			retry--;
			DPRINTK("[i2c_api_page_write: retry] %d times left \n", retry);
		} else
			retry=0;
	}    
	if (ret != 1) {
		DPRINTK("[i2c_api_page_write] write fail \n");
		return ;
	}
	DPRINTK("[i2c_api_page_write] OK! \n");
	return ;

}
/*************************************************************************
* wmt_i2c_xfer_if- 
* 
* RETURNS: 
* 
*/
void wmt_i2c_xfer_if(struct i2c_msg *msg)
{
	int  ret;

	if (*i2c_api_client == NULL) {		
		DPRINTK("[%s] i2c_api_client is NULL \n", __func__);
		return ;
	}
	ret = i2c_transfer((i2c_api_client[0])->adapter, msg, 1);
	if (ret <= 0) {
		DPRINTK("[%s] fail \n", __func__);
		return ;
	}
	DPRINTK("[%s] OK! \n", __func__);
	return ;

}

/*************************************************************************
* wmt_i2c_xfer_continue_if- 
* 
* RETURNS: 
* 
*/
void wmt_i2c_xfer_continue_if(struct i2c_msg *msg, unsigned int num)
{
	int  ret;

	if (*i2c_api_client == NULL) {		
		DPRINTK("[%s] i2c_api_client is NULL \n", __func__);
		return ;
	}
	ret = i2c_transfer((i2c_api_client[1])->adapter, msg, num);
	if (ret <= 0) {
		DPRINTK("[%s] fail \n", __func__);
		return ;
	}
	DPRINTK("[%s] OK! \n", __func__);
	return ;

}

/*************************************************************************
* wmt_i2c_xfer_continue_if_2 
* 
* RETURNS: 
* 
*/
int wmt_i2c_xfer_continue_if_2(struct i2c_msg *msg, unsigned int num)
{
	int  ret = 1;

	if(*i2c_api_client == NULL) {		
		DPRINTK("[%s] i2c_api_client is NULL \n", __func__);
		return -1;
	}
	ret = i2c_transfer((i2c_api_client[1])->adapter, msg, num);
	if (ret <= 0) {
		DPRINTK("[%s] fail \n", __func__);
		return ret;
	}
	DPRINTK("[%s] OK! \n", __func__);
	return ret;

}
/*************************************************************************
* wmt_i2c_xfer_continue_if_3 
* 
* Private Function by Dean 2009/12/22
* 
* RETURNS: 
* 
*/
int wmt_i2c_xfer_continue_if_3(struct i2c_msg *msg, unsigned int num, int bus_id)
{
	int  ret = 1;
	int i = 0; 

	if (bus_id == 1)
		bus_id = 0;
	else if (bus_id == 0)
		bus_id = 1;
	else
		return -ENODEV;

	if (bus_id > wmt_i2c_api_client_count)
		bus_id = wmt_i2c_api_client_count;

	if (i2c_api_client[bus_id] == NULL) {		
		DPRINTK("[%s] i2c_api_client is NULL \n", __func__);
		return -1;
	}
	if (num > 1) {
		for (i = 0; i < num - 1; ++i)
			msg[i].flags |= I2C_M_NOSTART;
	}
	ret = i2c_transfer((i2c_api_client[bus_id])->adapter, msg, num);
	if (ret <= 0) {
		DPRINTK("[%s] fail \n", __func__);
		return ret;
	}
	DPRINTK("[%s] OK! \n", __func__);
	return ret;

}

/*************************************************************************
* wmt_i2c_xfer_continue_if_4
*
* Private Function by Dean 2009/12/22
*
* RETURNS:
*
*/
int wmt_i2c_xfer_continue_if_4(struct i2c_msg *msg, unsigned int num, int bus_id)
{
	int  ret = 1;
	int i = 0;

	if (bus_id > wmt_i2c_api_client_count)
		bus_id = wmt_i2c_api_client_count;

	if (i2c_api_client[bus_id] == NULL) {
		DPRINTK("[%s] i2c_api_client is NULL \n", __func__);
		return -ENODEV;
	}
	if (num > 1) {
		for (i = 0; i < num - 1; ++i)
			msg[i].flags |= I2C_M_NOSTART;
	}
	ret = i2c_transfer((i2c_api_client[bus_id])->adapter, msg, num);
	if (ret <= 0) {
		DPRINTK("[%s] fail \n", __func__);
		return ret;
	}
	DPRINTK("[%s] OK! \n", __func__);
	return ret;
}

/*
* i2c_api_register_read - 
* 
* RETURNS: 
* 
*/
int i2c_api_register_read(int index)
{
	int ret, retry ;

	unsigned char reg_index[1] = { 0 } ;
	unsigned char p_data[1] = { 0 } ;
	struct i2c_msg 	wr[1] ;
	struct i2c_msg 	rd[1] ;

	if (*i2c_api_client == NULL) {		
		DPRINTK("[i2c_api_register_read] i2c_api_client is NULL \n");
		return 0 ;
	}

	/* step1: write address out */
	reg_index[0] = (unsigned char)index ;
	p_data[0]    = 0 ;

	wr[0].addr  = (*i2c_api_client)->addr  ;  
	wr[0].flags = WMT_I2C_W ;
	wr[0].len   = 1 ;    
	wr[0].buf   = reg_index ;    

	retry = 3;
	while (retry) {
		ret = i2c_transfer((*i2c_api_client)->adapter, wr, 1);
		if (ret != 1) {
			retry--;
			DPRINTK("[i2c_api_register_read: retry] %d times left \n", retry);
		} else
			retry=0;
	} 
	    
	if (ret != 1) {
		DPRINTK("[i2c_api_register_read] write fail \n");
		return 0 ;
	}

	/* step2: read data in */
	rd[0].addr  = (*i2c_api_client)->addr  ;
	rd[0].flags = WMT_I2C_R ;
	rd[0].len   = 1 ; 
	rd[0].buf   = p_data  ;   

	retry =3;
	while (retry) {
		ret = i2c_transfer((*i2c_api_client)->adapter, rd, 1);
		if (ret != 1) {
			retry--;
			DPRINTK("[i2c_api_register_read: retry] %d times left \n", retry);
		} else
			retry=0;
	} 
	      
	if (ret != 1) {
		DPRINTK("[i2c_api_register_read] read fail \n");
		return 0 ;
	}

	return p_data[0];    

}
/*************************************************************************
* i2c_api_page_read - 
* 
* RETURNS: 
* 
*/
int i2c_api_page_read(int index, int count)
{
	int i,ret, retry ;

	unsigned char 	reg_index[1] = { 0 } ;
	unsigned char 	p_data[count]   ;
	struct i2c_msg 	wr[1] ;
	struct i2c_msg 	rd[1] ;

	if (*i2c_api_client == NULL) {		
		DPRINTK("[i2c_api_page_read] i2c_api_client is NULL \n");
		return 0 ;
	}

	/* step1: write address out */
	reg_index[0] = (unsigned char)index ;
	for (i=0 ; i < count ; i++)
		;
	p_data[i] = 0 ;

	wr[0].addr = (*i2c_api_client)->addr  ;  
	wr[0].flags = WMT_I2C_W ;
	wr[0].len   = 1 ;    
	wr[0].buf   = reg_index ;    
	retry =3;
	while (retry) {
		ret = i2c_transfer((*i2c_api_client)->adapter, wr, 1);
		if (ret != 1) {
			retry--;
			DPRINTK("[i2c_api_page_read: retry] %d times left \n", retry);
		} else
			retry=0;
	}

	if (ret != 1) {
		DPRINTK("[i2c_api_page_read] write fail \n");
		return 0 ;
	}

	/* step2: read data in */
	rd[0].addr  = (*i2c_api_client)->addr  ;
	rd[0].flags = WMT_I2C_R ;
	rd[0].len   = count ; 
	rd[0].buf   = p_data  ;    
	retry = 3;
	while (retry) {
		ret = i2c_transfer((*i2c_api_client)->adapter, rd, 1);
		DPRINTK("[i2c_api_page_read: retry] ret =%d \n", ret);				
		if (ret != 1) {
			retry--;
			DPRINTK("[i2c_api_page_read: retry] %d times left \n", retry);
		} else
			retry=0;
	}

	if (ret != 1) {
		DPRINTK("[i2c_api_page_read] read fail \n");
		return 0 ;
	}
	return p_data[0];    
   
}

/*************************************************************************
* wmt_i2c_api_attach - 
* 
* Private Function by Paul Kwong, 2007/5/4
* 
* RETURNS: 
* 
*/
int wmt_i2c_api_attach(struct i2c_adapter *adap)
{
	struct i2c_board_info info;
	memset(&info, 0, sizeof(struct i2c_board_info));
        strlcpy(info.type, "wmt_i2c_api", I2C_NAME_SIZE);
        info.addr = WMT_I2C_API_I2C_ADDR;
	if (adap->nr == 0) {/*i2c 0*/
		*i2c_api_client = i2c_new_device(adap, &info);
		(*i2c_api_client)->dev.driver = &wmt_i2c_api_driver.driver;
	}
	if (adap->nr == 1)/*i2c 1*/
		*(i2c_api_client + 1) = i2c_new_device(adap, &info);
	++wmt_i2c_api_client_count;

	return 0;
}

/*************************************************************************
* wmt_i2c_api_probe - 
* 
* RETURNS: 
* 
*/
int wmt_i2c_api_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	return 0;
}

/*************************************************************************
* wmt_i2c_api_detach - 
* 
* RETURNS: 
* 
*/
int wmt_i2c_api_detach(struct i2c_client *client)
{	
	struct i2c_adapter *adap = client->adapter;
	int adap_id = i2c_adapter_id(adap);
		
	kfree(i2c_get_clientdata(client));	
	/*
	kfree(client);
	*/
	i2c_api_client[adap_id] = NULL ;
	--wmt_i2c_api_client_count;
	return 0;
}

/*!*************************************************************************
* wmt_i2c_api_suspend()
* 
*/
static int wmt_i2c_api_suspend(
        struct device *dev,		/*!<; // a pointer point to struct device */ 
        pm_message_t state					/*!<; // state of suspend */ 
)
{
	printk("i2c suspend\n");
	if (!wmt_i2c_suspend_flag) {
		/*store port 1*/
		wmt_i2c_reg.imr_reg = wmt_i2c_reg_addr->imr_reg;
		wmt_i2c_reg.tr_reg = wmt_i2c_reg_addr->tr_reg;
		wmt_i2c_reg.div_reg = wmt_i2c_reg_addr->div_reg;
		/*store port 0*/
		wmt_i2c1_reg.imr_reg = wmt_i2c1_reg_addr->imr_reg;
		wmt_i2c1_reg.tr_reg = wmt_i2c1_reg_addr->tr_reg;
		wmt_i2c1_reg.div_reg = wmt_i2c1_reg_addr->div_reg;
		wmt_i2c_suspend_flag = 1;
	}
	return 0;
} /* End of wmt_i2c_api_suspend() */

/*!*************************************************************************
* wmt_i2c_api_resume()
* 
*/
static int wmt_i2c_api_resume(
        struct device *dev    /*!<; // a pointer point to struct device */
)
{
	unsigned short tmp;
	
	if (wmt_i2c_suspend_flag) {
		printk("i2c resume\n");
		wmt_i2c_suspend_flag = 0;
#ifdef CONFIG_I2C1_WMT
		if (wmt_i2c1_is_master == 1) {
			auto_pll_divisor(DEV_I2C1, CLK_ENABLE, 0, 0);
			auto_pll_divisor(DEV_I2C1, SET_DIV, 2, 20);/*20M Hz*/
			GPIO_CTRL_GP21_I2C_BYTE_VAL &= ~(BIT2 | BIT3);
			GPIO_PULL_EN_GP21_I2C_BYTE_VAL |= (BIT2 | BIT3);
			GPIO_PULL_CTRL_GP21_I2C_BYTE_VAL |= (BIT2 | BIT3);
#if 0
			*(volatile unsigned int *)CTRL_GPIO21 &= ~(BIT2 | BIT3);
			*(volatile unsigned int *)PU_EN_GPIO21 |= (BIT2 | BIT3);
			*(volatile unsigned int *)PU_CTRL_GPIO21 |= (BIT2 | BIT3);
#endif
			wmt_i2c_reg_addr->cr_reg  = 0 ;
			wmt_i2c_reg_addr->div_reg = wmt_i2c_reg.div_reg;
			wmt_i2c_reg_addr->imr_reg = wmt_i2c_reg.imr_reg;
			wmt_i2c_reg_addr->cr_reg = 0x001 ;
			tmp = wmt_i2c_reg_addr->isr_reg; /* read clear*/
			wmt_i2c_reg_addr->tcr_reg = wmt_i2c_reg.tcr_reg;
		}
#endif
#ifdef CONFIG_I2C_WMT
		if (wmt_i2c0_is_master == 1) {
			auto_pll_divisor(DEV_I2C0, CLK_ENABLE, 0, 0);
			auto_pll_divisor(DEV_I2C0, SET_DIV, 2, 20);/*20M Hz*/
			GPIO_CTRL_GP21_I2C_BYTE_VAL &= ~(BIT0 | BIT1);
			GPIO_PULL_EN_GP21_I2C_BYTE_VAL |= (BIT0 | BIT1);
			GPIO_PULL_CTRL_GP21_I2C_BYTE_VAL |= (BIT0 | BIT1);
#if 0
			*(volatile unsigned int *)CTRL_GPIO21 &= ~(BIT0 | BIT1);
			*(volatile unsigned int *)PU_EN_GPIO21 |= (BIT0 | BIT1);
			*(volatile unsigned int *)PU_CTRL_GPIO21 |= (BIT0 | BIT1);
#endif
			wmt_i2c1_reg_addr->cr_reg  = 0 ;
			wmt_i2c1_reg_addr->div_reg = wmt_i2c1_reg.div_reg;
			wmt_i2c1_reg_addr->imr_reg = wmt_i2c1_reg.imr_reg;
			wmt_i2c1_reg_addr->tr_reg = wmt_i2c1_reg.tr_reg;
			wmt_i2c1_reg_addr->cr_reg = 0x001 ;
			tmp = wmt_i2c1_reg_addr->isr_reg; /* read clear*/
		}
#endif
	}
	return 0;
} /* End of wmt_i2c_api_resume() */

/*************************************************************************
* wmt_i2c_api_init - 
* 
* RETURNS: 
* 
*/
int wmt_i2c_api_init(void)
{	
	int res;
	
	if (!wmt_i2c_api_i2c_initialized){
		if ((res = i2c_add_driver(&wmt_i2c_api_driver))) {
			printk("[wmt_i2c_api_init] Driver registration failed, module not inserted.\n");
			wmt_i2c_api_exit();
			return res;
		}
	}
	printk(KERN_INFO "[wmt_i2c_api_init] wmt_i2c_api_init.\n");
	wmt_i2c_api_i2c_initialized++;
	return 0 ;	
}

/*************************************************************************
* wmt_i2c_api_exit - 
* 
* RETURNS: 
* 
*/
void wmt_i2c_api_exit(void)
{	
	if (wmt_i2c_api_i2c_initialized == 1) {
		i2c_del_driver(&wmt_i2c_api_driver);
	}
	wmt_i2c_api_i2c_initialized --;	
}

/*************************************************************************
* wmt_i2c_api_i2c_init - 
* 
* RETURNS: 
* 
*/
static int __init  wmt_i2c_api_i2c_init(void)
{	
	printk("[wmt_i2c_api_i2c_init]\n");
	wmt_i2c_api_init() ;
	return 0 ;	
}

/*************************************************************************
* wmt_i2c_api_i2c_exit - 
* 
* RETURNS: 
* 
*/
static void __exit  wmt_i2c_api_i2c_exit(void)
{	
	wmt_i2c_api_exit() ;
	printk("[wmt_i2c_api_i2c_exit] Driver delete !\n");
}

/*----------------------- End of Function Body -------------------------------*/

MODULE_AUTHOR("WonderMedia Technologies, Inc.");
MODULE_DESCRIPTION("WMT I2C wmt_i2c_api Driver");
MODULE_LICENSE("GPL");

module_init(wmt_i2c_api_i2c_init);
module_exit(wmt_i2c_api_i2c_exit);

EXPORT_SYMBOL(i2c_api_register_write);
EXPORT_SYMBOL(i2c_api_register_read);
EXPORT_SYMBOL(i2c_api_page_write);
EXPORT_SYMBOL(i2c_api_page_read);
EXPORT_SYMBOL(wmt_i2c_xfer_if);
EXPORT_SYMBOL(wmt_i2c_xfer_continue_if);
EXPORT_SYMBOL(wmt_i2c_xfer_continue_if_2);
EXPORT_SYMBOL(wmt_i2c_xfer_continue_if_3);
EXPORT_SYMBOL(wmt_i2c_xfer_continue_if_4);
