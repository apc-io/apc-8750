/*++ 
 * linux/drivers/video/wmt/com-cec.h
 * WonderMedia HDMI CEC driver
 *
 * Copyright c 2011  WonderMedia  Technologies, Inc.
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
 * 2011-04-11  Sam Shen
 *     * Add License declaration and ChangeLog
 */

#ifndef COM_CEC_H
#define COM_CEC_H

#if defined(__cplusplus)
extern "C" {
#endif

/* define for notify AP */
#define USER_PID 1
#define WP_PID	 2
#define USER_PIX_MAX 2
#define NETLINK_CEC_TEST (MAX_LINKS-2)

#define DEVICE_RX_DATA	0
#define DEVICE_PLUG_IN 	2
#define DEVICE_PLUG_OUT 3

#define MAX_MSG_BYTE 16
#define MSG_ABORT 0xff

typedef struct wmt_cec_msg{
	char msglen;
	char msgdata[MAX_MSG_BYTE];
} wmt_cec_msg_t;

typedef struct wmt_phy_addr {
	unsigned int phy_addr;
} wmt_phy_addr_t;

#define WMT_CEC_IOC_MAGIC           'c'
#define WMT_CEC_IOC_MAXNR           3
#define CECIO_TX_DATA				_IOW(WMT_CEC_IOC_MAGIC,0,wmt_cec_msg_t)
#define CECIO_TX_LOGADDR			_IO(WMT_CEC_IOC_MAGIC,1)
#define CECIO_RX_PHYADDR			_IOR(WMT_CEC_IOC_MAGIC,2,wmt_phy_addr_t)

#if defined(__cplusplus)
}
#endif

#endif

