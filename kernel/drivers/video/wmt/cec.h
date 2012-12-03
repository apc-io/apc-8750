/*++ 
 * linux/drivers/video/wmt/cec.h
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
#include "vpp-osif.h"
#include "./hw/wmt-vpp-hw.h"
#include "vpp.h"
#include "com-cec.h"

#ifdef WMT_FTBLK_CEC

#ifndef CEC_H
#define CEC_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CEC_C
#define EXTERN
#else
#define EXTERN extern
#endif

EXTERN void wmt_cec_tx_data(char *buf,int size);
EXTERN void wmt_cec_clr_int(int sts);
EXTERN int wmt_cec_get_int(void);
EXTERN void wmt_cec_enable_int(int no,int enable);

EXTERN void wmt_cec_enable_loopback(int enable);
EXTERN void wmt_cec_tx_data(char *buf,int size);
EXTERN int wmt_cec_rx_data(char *buf);
EXTERN void wmt_cec_hotplug_notify(int plug_status);
EXTERN void wmt_cec_do_hotplug_notify(int no,int plug_status);

EXTERN void wmt_cec_init_hw(void);
EXTERN void wmt_cec_set_logical_addr(int no,char addr,int enable);
EXTERN void wmt_cec_rx_enable(int enable);
EXTERN void wmt_cec_do_suspend(void);
EXTERN void wmt_cec_do_resume(void);
EXTERN void wmt_cec_reg_dump(void);

#undef EXTERN
#ifdef __cplusplus
}
#endif
#endif				// CEC_H
#endif				// WMT_FTBLK_CEC

