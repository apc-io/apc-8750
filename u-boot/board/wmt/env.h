/*++ 
Copyright (c) 2010 WonderMedia Technologies, Inc.

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software 
Foundation, either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details. You
should have received a copy of the GNU General Public License along with this
program. If not, see http://www.gnu.org/licenses/>.

WonderMedia Technologies, Inc.
10F, 529, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C.
--*/

#ifndef __ENV_H__
#define __ENV_H__


struct env_para_def {
	char *value;
	unsigned int npos;
	unsigned int ppos;
	unsigned int size;
};

unsigned int char2int(char c);
int env_cmp_name(unsigned int pos, char *buf, char *name);
unsigned int sf_env_crc32(unsigned char *buf);
int env_find_env(char *name, unsigned char *buf, struct env_para_def *para);
unsigned int env_find_next_pos(unsigned int pos, char *buf);
unsigned int env_find_para_pos(unsigned int pos, char *buf);
int env_read_para1(char *name, struct env_para_def *para);
int env_write_para(char *name, char *para, unsigned int value);
unsigned char *sf_env_init(unsigned int *pos);
unsigned int env_get_para_value(char *buf);
unsigned int env_get_env_value(char *name);

#endif
