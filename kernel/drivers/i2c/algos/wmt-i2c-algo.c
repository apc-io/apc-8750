/*++
	drivers/i2c/algos/wmt_i2c_algo.c

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

#define WMT_I2C_ALGO_C

#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/init.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/i2c.h>
#include <linux/i2c-id.h>

#include <mach/hardware.h>
#include <mach/wmt-i2c-bus.h>

#ifdef __KERNEL__

#ifdef DEBUG
	#define DPRINTK    	printk
#else
	#define DPRINTK(x...)
#endif

#else
#define DPRINTK    printf

#endif

static struct i2c_adapter *wmt_i2c_adap[2];

/*!*************************************************************************
* wmt_i2c_valid_messages()
*
* Private Function by Paul Kwong, 2007/1/12
*/
/*!
* \brief verify the input message
*
* \retval  1 if success
*/
static int wmt_i2c_valid_messages(
	struct i2c_msg msgs[], 	/*!<; //[IN] transfer data  */
	int num					/*!<; //[IN] transfer data length */
)
{
	int i;
	if (num < 1 || num > MAX_MESSAGES) {
		DPRINTK(KERN_INFO "Invalid number of messages (max=%d, num=%d)\n", MAX_MESSAGES, num);
		return -EINVAL;
	}

	/* check consistency of our messages */
	for (i = 0; i < num; i++) {
		if (&msgs[i] == NULL) {
			DPRINTK(KERN_INFO "Msgs is NULL\n");
			return -EINVAL;
		} else {
			if (msgs[i].buf == NULL) {
				DPRINTK(KERN_INFO "Length is less than zero\n");
				return -EINVAL;
			}
		}
	}

	return 1;
}
/*!*************************************************************************
* wmt_i2c_do_xfer()
*
* Private Function by Paul Kwong, 2007/1/12
*/
/*!
* \brief
*
* \retval  0 if success
*/
static int wmt_i2c_do_xfer(
	struct i2c_adapter *i2c_adap, 	/*!<; //[IN] a pointer point to struct inode */
	struct i2c_msg msgs[], 		/*!<; //[IN] transfer data  */
	int num						/*!<; //[IN] transfer data length */
)
{
	int i;
	struct i2c_algo_wmt_data *adap;
	int ret = 0 ;

	adap = i2c_adap->algo_data;

	/*ret = adap->wait_bus_not_busy();*/
	for (i = 0 ; i < 10; ++i)
		;
	if (ret < 0)
		return ret ;

	ret = adap->send_request(msgs, num, 0, 0, 0);

	return ret;

}

/*!*************************************************************************
* wmt_i2c_xfer()
*
* Private Function by Paul Kwong, 2007/1/12
*/
/*!
* \brief Transfer (read/write) data to i2c bus, wmt_i2c_do_xfer will be called to transfer
*
* \retval  0 if success
*/
static int wmt_i2c_xfer(
	struct i2c_adapter *i2c_adap, 	/*!<; //[IN] a pointer point to struct inode */
	struct i2c_msg msgs[], 		/*!<; //[IN] transfer data  */
	int num						/*!<; //[IN] transfer data length */
)
{
	int ret ;
	int i ;

	ret = wmt_i2c_valid_messages(msgs, num);
	if (ret < 0)
		return ret ;

	for (i = i2c_adap->retries ;  i >= 0; i--) {

		ret = wmt_i2c_do_xfer(i2c_adap, msgs, num);
		if (ret > 0)
			return ret ;
		DPRINTK(KERN_INFO"Retrying transmission \n");
		udelay(100);
	}

	DPRINTK(KERN_INFO"Retried %i times\n", i2c_adap->retries);
	return ret;

}
/*!*************************************************************************
* wmt_i2c_functionality()
*
* Private Function by Paul Kwong, 2007/1/12
*/
/*!
* \brief
*
* \retval  smbus functionality
*/
static u32 wmt_i2c_functionality(
	struct i2c_adapter *adapter		/*!<; //[IN] a pointer point to struct inode */
)
{
	/* Emulate the SMBUS functions*/
	return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL;
}

/*
 * send i2c_msg into fifo of i2c bus
 * msg: transfer data content
 * msg_num:number of transferring msg
 * bus_id :used to indicate which bus do device want to access
 */
int wmt_i2c_transfer(struct i2c_msg* msgs, int msg_num, int bus_id, void (*callback)(void *data), void *data)
{
	struct i2c_algo_wmt_data *adap;
	int ret = 0 ;

	adap = wmt_i2c_adap[bus_id]->algo_data;
	ret = adap->send_request(msgs, msg_num, 1, callback, data);
	return ret;
}

#if 0
/*!*************************************************************************
* wmt_i2c_control()
*
* Private Function by Paul Kwong, 2007/1/12
*/
/*!
* \brief To set i2c transfer mode
*
* \retval  0 if success
*/
static int wmt_i2c_control(
	struct i2c_adapter *i2c_adap, 	/*!<; //[IN] a pointer point to struct inode */
	unsigned int cmd, 				/*!<; //[IN] standard or fast mode */
	unsigned long arg				/*!<; //Not in used, but can't delete */
)
{
	int ret ;
	struct i2c_algo_wmt_data *adap = i2c_adap->algo_data;

	ret = 0 ;
	DPRINTK("wmt_i2c_control: cmd = 0x%8.8x \n", cmd);

	switch (cmd) {
	case I2C_SET_STANDARD_MODE:
		adap->set_mode(I2C_STANDARD_MODE);
		break ;
	case I2C_SET_FAST_MODE:
		adap->set_mode(I2C_FAST_MODE);
		break ;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}
#endif


struct i2c_algorithm wmt_i2c_algorithm  = {
	.master_xfer     =         wmt_i2c_xfer,
	.functionality   =         wmt_i2c_functionality,
};
/*!*************************************************************************
* wmt_i2c_add_bus()
*
* Private Function by Paul Kwong, 2007/1/12
*/
/*!
* \brief
*
* \retval  NULL
*/
int wmt_i2c_add_bus(struct i2c_adapter *i2c_adap)
{
	printk(KERN_INFO"i2c: adding %s.\n", i2c_adap->name);

	i2c_adap->algo = &wmt_i2c_algorithm;
	wmt_i2c_adap[i2c_adap->nr] = i2c_adap;

	/* register new adapter to i2c module... */
	/*
	i2c_add_adapter(i2c_adap);
	*/
	i2c_add_numbered_adapter(i2c_adap);

	/* adap->reset();*/

	return 0;
}
/*!*************************************************************************
* wmt_i2c_del_bus()
*
* Private Function by Paul Kwong, 2007/1/12
*/
/*!
* \brief
*
* \retval  NULL
*/
int wmt_i2c_del_bus(struct i2c_adapter *i2c_adap)
{
	int res;
	res = i2c_del_adapter(i2c_adap);
	if (res < 0)
		return res;

	printk(KERN_INFO "i2c: removing %s.\n", i2c_adap->name);

	return 0;
}
/*!*************************************************************************
* wmt_i2c_algo_init()
*
* Private Function by Paul Kwong, 2007/1/12
*/
/*!
* \brief
*
* \retval  NULL
*/
static int __init wmt_i2c_algo_init(void)
{
	printk(KERN_INFO "i2c: wmt algorithm module loaded.\n");
	return 0;
}

EXPORT_SYMBOL(wmt_i2c_add_bus);
EXPORT_SYMBOL(wmt_i2c_del_bus);

MODULE_AUTHOR("WonderMedia Technologies, Inc.");
MODULE_DESCRIPTION("WMT I2C ALGO Driver");
MODULE_LICENSE("GPL");

module_init(wmt_i2c_algo_init);

#undef WMT_I2C_ALGO_C
