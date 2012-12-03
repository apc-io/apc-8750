/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * Copyright (c) 2008 WonderMedia Technologies, Inc.
 *
 * This program is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software Foundation,
 * either version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * WonderMedia Technologies, Inc.
 * 10F, 529, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C.
 */

#ifndef _NOR_FLASH_8BIT_H_
#define _NOR_FLASH_8BIT_H_

#include <common.h>
#include <mpc8xx.h>

unsigned long nor_flash_8b_init(void);
void nor_flash_8b_print_info(flash_info_t *info);
int nor_flash_8b_erase(flash_info_t *info, int s_first, int s_last);
int nor_flash_8b_write_buff(flash_info_t *info, uchar *src, ulong addr, ulong cnt);
#ifdef CFG_FLASH_PROTECTION
int nor_flash_8b_real_protect(flash_info_t *info, long sector, int prot);
#endif

#endif /*_NOR_FLASH_8BIT_H_*/
