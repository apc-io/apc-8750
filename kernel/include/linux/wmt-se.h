/*++ 
 * WonderMedia cipher driver 
 *
 * Copyright c 2012  WonderMedia  Technologies, Inc.
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

#ifndef WMT_SE_H
/* To assert that only one occurrence is included */
#define WMT_SE_H


typedef int (*SE_FPTR)(unsigned char *buf_in, unsigned char *buf_out, int buf_len);

extern SE_FPTR wmt_se_aes;
extern SE_FPTR wmt_se_sha;


#endif /* ifndef WMT_SE_H */

/*=== END wmt-se.h ==========================================================*/
