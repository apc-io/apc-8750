/*++ 
 * linux/drivers/video/wmt/sw_i2c.h
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

#ifndef _SWI2C_H_
#define _SWI2C_H_

typedef struct {
	unsigned int bit_mask;
	unsigned int gpio_en;
	unsigned int out_en;
	unsigned int data_in;
	unsigned int data_out;
	unsigned int pull_en_bit_mask;
	unsigned int pull_en;
} swi2c_reg_t;

typedef struct {
	swi2c_reg_t *scl_reg;
	swi2c_reg_t *sda_reg;
} swi2c_handle_t;

int wmt_swi2c_read(
	swi2c_handle_t *handle,
    char addr,
    char index,
    char *buf,
    int cnt
);

int wmt_swi2c_write(
    swi2c_handle_t *handle,
    char addr,
    char index,
    char *buf,
    int cnt
);

#endif

