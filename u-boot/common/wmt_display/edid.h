/*++ 
 * linux/drivers/video/wmt/edid.h
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

#ifndef EDID_H
#define EDID_H

#define EDID_BLOCK_MAX		4

/*------------------------------------------------------------------------------
    Following definitions, please refer spec of EDID. You may refer it on 
    http://en.wikipedia.org/wiki/EDID#EDID_1.3_data_format   
------------------------------------------------------------------------------*/

#define EDID_LENGTH                             0x80

/*------------------------------------------------------------------------------
  Offset 00-19: HEADER INFORMATION
------------------------------------------------------------------------------*/
/*  00¡V07: Header information "00h FFh FFh FFh FFh FFh FFh 00h" */  
#define EDID_HEADER                             0x00
#define EDID_HEADER_END                         0x07

/*  08¡V09: Manufacturer ID. These IDs are assigned by Microsoft. 
         "00001=A¡¨; ¡§00010=B¡¨; ... ¡§11010=Z¡¨. Bit 7 (at address 08h) is 0, the first
         character (letter) is located at bits 6 ¡÷ 2 (at address 08h), the second character
         (letter) is located at bits 1 & 0 (at address 08h) and bits 7 ¡÷ 5 (at address 09h),
         and the third character (letter) is located at bits 4 ¡÷ 0 (at address 09h).
*/         
#define ID_MANUFACTURER_NAME                    0x08
#define ID_MANUFACTURER_NAME_END                0x09

/*  10¡V11: Product ID Code (stored as LSB first). Assigned by manufacturer */
#define ID_MODEL				                0x0a

/*  12¡V15: 32-bit Serial Number. No requirement for the format. Usually stored as LSB first. In
         order to maintain compatibility with previous requirements the field should set at
         least one byte of the field to be non-zero if an ASCII serial number descriptor is
         provided in the detailed timing section.
*/         
#define ID_SERIAL_NUMBER			            0x0c

/*  16: Week of Manufacture. This varies by manufacturer. One way is to count January 1-7 as
      week 1, January 8-15 as week 2 and so on. Some count based on the week number
      (Sunday-Saturday). Valid range is 1-54.
    17: Year of Manufacture. Add 1990 to the value for actual year. */
#define MANUFACTURE_WEEK			            0x10
#define MANUFACTURE_YEAR			            0x11

/*  18: EDID Version Number. "01h"
    19: EDID Revision Number "03h"  */    
#define EDID_STRUCT_VERSION                     0x12
#define EDID_STRUCT_REVISION                    0x13

/*------------------------------------------------------------------------------
  Offset 20-24: BASIC DISPLAY PARAMETERS
------------------------------------------------------------------------------*/


#define DPMS_FLAGS				                0x18
#define ESTABLISHED_TIMING_I                    0x23
#define ESTABLISHED_TIMING_II                   0x24
#define MANUFACTURERS_TIMINGS                   0x25

#define STANDARD_TIMING_IDENTIFICATION_START    0x26
#define STANDARD_TIMING_IDENTIFICATION_SIZE     16

#define DETAILED_TIMING_DESCRIPTIONS_START      0x36
#define DETAILED_TIMING_DESCRIPTION_SIZE        18
#define NO_DETAILED_TIMING_DESCRIPTIONS         4

#define DETAILED_TIMING_DESCRIPTION_1           0x36
#define DETAILED_TIMING_DESCRIPTION_2           0x48
#define DETAILED_TIMING_DESCRIPTION_3           0x5a
#define DETAILED_TIMING_DESCRIPTION_4           0x6c

#define EDID_TMR_INTERLACE	BIT(31)
#define EDID_TMR_FREQ		0xFF
typedef struct {
	unsigned int resx;
	unsigned int resy;
	int freq;	// EDID_TMR_XXX
} edid_timing_t;

/* EDID option flag */
#define EDID_OPT_VALID		0x01
#define EDID_OPT_YUV422		0x10
#define EDID_OPT_YUV444		0x20
#define EDID_OPT_AUDIO		0x40
#define EDID_OPT_UNDERSCAN	0x80
#define EDID_OPT_HDMI		0x100
#define EDID_OPT_3D			BIT(9)
#define EDID_OPT_16_9		BIT(10)

typedef struct {
	unsigned int establish_timing;
	edid_timing_t standard_timing[8];
	vpp_timing_t detail_timing[4];
	vpp_timing_t cea_timing[6];
	char cea_vic[8];
	unsigned int pixel_clock_limit;
	unsigned int option;
	unsigned int hdmi_phy_addr;
} edid_info_t;

extern edid_info_t edid_info;
extern int edid_msg_enable;
extern int edid_parse( unsigned char * edid );
extern int edid_find_support(unsigned int resx,unsigned int resy,int freq,vpp_timing_t **timing);
extern void edid_dump(unsigned char *edid);
extern int edid_parse_option(unsigned char *edid);
extern int edid_checksum(unsigned char *edid,int len);
extern unsigned int edid_get_hdmi_phy_addr(void);

#endif

