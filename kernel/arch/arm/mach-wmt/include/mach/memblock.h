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
#define MB_DEF_TGID				1

/* this function must be immediately called when kernel driver 
   get user address in direct ioctrl*/
#define mb_user_to_virt(u)		mb_do_user_to_virt(u,MB_DEF_TGID)
#define mb_user_to_phys(u)		mb_do_user_to_phys(u,MB_DEF_TGID)
/* All following 0 means type 0 (kernel space user) */
#define mb_allocate(s)			mb_do_allocate(s,0,THE_MB_USER,MB_DEF_TGID)
#define mb_free(v)				mb_do_free(v,0,THE_MB_USER,MB_DEF_TGID)
#define mb_get(v)				mb_do_get(v,0,THE_MB_USER,MB_DEF_TGID)
#define mb_put(v)				mb_do_put(v,0,THE_MB_USER,MB_DEF_TGID)
#define mb_counter(p)			mb_do_counter(p,0,THE_MB_USER)

unsigned long mb_do_user_to_virt(unsigned long, pid_t);
unsigned long mb_do_user_to_phys(unsigned long, pid_t);
unsigned long mb_do_allocate(unsigned long, unsigned int, char *, pid_t);
int mb_do_free(unsigned long, unsigned int, char *, pid_t);
int mb_do_get(unsigned long, unsigned int, char *, pid_t);
int mb_do_put(unsigned long, unsigned int, char *, pid_t);
int mb_do_counter(unsigned long, unsigned int, char *);

int user_to_prdt(
	unsigned long user, 
	unsigned int size, 
	struct prdt_struct *next,
	unsigned int items);

#endif
