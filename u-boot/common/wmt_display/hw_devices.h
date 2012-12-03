/*++ 
 * linux/drivers/video/wmt/govm.c
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

#ifndef HW_DEVICES_H
#define HW_DEVICES_H

int sil902x_module_init(void);
int vt1632_module_init(void);
int lcd_module_init(void);
int lcd_oem_init(void);
int lcd_at070tn83_init(void);
int lcd_a080sn01_init(void);
int lcd_lw700at9003_init(void);
int lcd_ek08009_init(void);
int lcd_HSD101PFW2_init(void);
#endif
