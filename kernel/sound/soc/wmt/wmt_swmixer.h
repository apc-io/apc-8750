/*++ 
 * linux/sound/soc/wmt/wmt_swmixer.h
 * WonderMedia I2S audio driver for ALSA
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


typedef struct float_data {
    unsigned long frac : 23;
	unsigned long exp : 8;
	unsigned long sign : 1;
} float_data_t;
 
void wmt_sw_u2s(int fmt, char *buffer, unsigned int chunksize);
void wmt_pcm_fmt_trans(int fmt, int channel, char *src_buf, char *dst_buf, unsigned int chunksize);

