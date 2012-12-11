/*++ 
 * WonderMedia Memory Block driver 
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
 * 10F, 529, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C
--*/
#ifndef MEMBLOCK_H
#define MEMBLOCK_H

#include <linux/types.h>

struct prdt_struct{
	unsigned int addr;
	unsigned short size;
	unsigned short reserve : 15;
	unsigned short EDT : 1;
}__attribute__((packed)) ;

// 1 presents the task init that would never be released
#define MB_DEF_TGID				0

#ifdef CONFIG_WMT_MB_RESERVE_FROM_IO
#define mb_virt_to_phys(v)		mb_do_virt_to_phys(v,MB_DEF_TGID,THE_MB_USER)
#define mb_phys_to_virt(p)		mb_do_phys_to_virt(p,MB_DEF_TGID,THE_MB_USER)
#else
#define mb_virt_to_phys(v)		virt_to_phys(v)
#define mb_phys_to_virt(p)		phys_to_virt(p)
#endif
#define mb_user_to_virt(u)		mb_do_user_to_virt(u,MB_DEF_TGID,THE_MB_USER)
#define mb_user_to_phys(u)		mb_do_user_to_phys(u,MB_DEF_TGID,THE_MB_USER)
/* all addresses are physical */
#define mb_alloc(s)				mb_do_alloc(s,MB_DEF_TGID,THE_MB_USER)
#define mb_free(p)				mb_do_free(p,MB_DEF_TGID,THE_MB_USER)
#define mb_get(p)				mb_do_get(p,MB_DEF_TGID,THE_MB_USER)
#define mb_put(p)				mb_do_put(p,MB_DEF_TGID,THE_MB_USER)
#define mb_counter(p)			mb_do_counter(p,THE_MB_USER)

void *mb_do_phys_to_virt(unsigned long phys, pid_t, char *);
unsigned long mb_do_virt_to_phys(void *virt, pid_t, char *);
unsigned long mb_do_user_to_virt(unsigned long, pid_t, char *);
unsigned long mb_do_user_to_phys(unsigned long, pid_t, char *);
unsigned long mb_do_alloc(unsigned long, pid_t, char *);
int mb_do_free(unsigned long, pid_t, char *);
int mb_do_get(unsigned long, pid_t, char *);
int mb_do_put(unsigned long, pid_t, char *);
int mb_do_counter(unsigned long, char *);

int user_to_prdt(
	unsigned long user, 
	unsigned int size, 
	struct prdt_struct *next,
	unsigned int items);

#endif
