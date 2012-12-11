/*++ 
 * linux/drivers/input/rmtctl/oem-dev-table.h
 * WonderMedia input remote control driver
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
#ifndef OEM_DEV_TABLE_H
/* To assert that only one occurrence is included */
#define OEM_DEV_TABLE_H

#ifdef OEM_DEV_TABLE_H
    #define EXTERN
#else
    #define EXTERN   extern
#endif /* ifdef OEM_DEV_TABLE_H */


#define RMCTL_WMT_1
#define RMCTL_WMT_2
#define RMCTL_TV_BOX


EXTERN struct rmt_dev {
	char *vendor_name;
	int vender_id;
	unsigned int key_codes[128];
};


EXTERN struct rmt_dev  rmt_dev_tbl[ ] = {
   #ifdef RMCTL_WMT_1
	{
		.vendor_name = "WMT_1",
		.vender_id = 0x00ff,
		.key_codes = {
			[0] = KEY_POWER,
			[1] = KEY_RESERVED,
			[2] = KEY_RESERVED,
			[3] = KEY_MUTE,
			[4] = KEY_CLEAR,
			[5] = KEY_UP,
			[6] = KEY_ESC,
			[7] = KEY_SCREEN, /* P/N */
			[8] = KEY_LEFT,
			[9] = KEY_ENTER,
			[10] = KEY_RIGHT,
			[11] = KEY_SETUP,
			[12] = KEY_F1,
			[13] = KEY_DOWN,
			[14] = KEY_F2,
			[15] = KEY_STOP,
			[16] = KEY_1,
			[17] = KEY_2,
			[18] = KEY_3,
			[19] = KEY_TIME,
			[20] = KEY_4,
			[21] = KEY_5,
			[22] = KEY_6,
			[23] = KEY_PLAYPAUSE,
			[24] = KEY_7,
			[25] = KEY_8,
			[26] = KEY_9,
			[27] = KEY_VOLUMEUP,
			[28] = KEY_0,
			[29] = KEY_BACK,
			[30] = KEY_FORWARD,
			[31] = KEY_VOLUMEDOWN
		},
	},
    #endif
    #ifdef RMCTL_WMT_2
	{
		.vendor_name = "WMT_2",
		.vender_id = 0x40bf,
		.key_codes = {
			[0] = KEY_RESERVED, /* KARAOKE */
			[1] = KEY_RESERVED, /* FUN- */
			[2] = KEY_RESERVED, /* ANGLE */
			[3] = KEY_VOLUMEDOWN, 
			[4] = KEY_CLEAR, 
			[5] = KEY_0, 
			[6] = KEY_F1, /* DIGEST */
			[7] = KEY_ZOOM, 
			[8] = KEY_7,
			[9] = KEY_8,
			[10] = KEY_NEXT,
			[11] = KEY_VOLUMEUP, 
			[12] = KEY_4,
			[13] = KEY_5,
			[14] = KEY_POWER,
			[15] = KEY_MUTE,
			[16] = KEY_1,
			[17] = KEY_2,
			[18] = KEY_SUBTITLE,
			[19] = KEY_RESERVED, /* RETURN */
			[20] = KEY_RECORD,
			[21] = KEY_RESERVED, /* STEP */
			[22] = KEY_RESERVED, /* A-B */
			[23] = KEY_RESERVED, /* STEP B*/
			[24] = KEY_BACK,
			[25] = KEY_PLAY,
			[26] = KEY_EJECTCD, 
			[27] = KEY_RESERVED, /* FF */
			[28] = KEY_LEFT,
			[29] = KEY_DOWN,
			[30] = KEY_F2, /* Menu/PBC */
			[31] = KEY_PLAYPAUSE, /* SF */
			/* Keycode 32 - 63 are invalid. */
			[64] = KEY_AUDIO,
			[65] = KEY_SETUP,
			[66] = KEY_RESERVED, /* FUN+ */
			[67] = KEY_RESERVED, /* MARK */
			[68] = KEY_UP,
			[69] = KEY_RESERVED, /* +10 */
			[70] = KEY_RESERVED, /* INVALID */
			[71] = KEY_RESERVED, /* SURR */
			[72] = KEY_RIGHT,
			[73] = KEY_9,
			[74] = KEY_RESERVED, /* INVALID */
			[75] = KEY_RESERVED, /* VOCAL */
			[76] = KEY_TV,
			[77] = KEY_6,
			[78] = KEY_RESERVED, /* INVALID */
			[79] = KEY_PROGRAM, /* PROG */
			[80] = KEY_RESERVED, /* DISPLAY */
			[81] = KEY_3,
			[82] = KEY_RESERVED, /* INVALID */
			[83] = KEY_RESERVED, /* INVALID */
			[84] = KEY_GOTO,
			[85] = KEY_PREVIOUS, /* Prev/ASV- */
			[86] = KEY_RESERVED, /* INVALID */
			[87] = KEY_RESERVED, /* INVALID */
			[88] = KEY_RESERVED, /* Repeat */
			[89] = KEY_STOP,
			[90] = KEY_RESERVED, /* INVALID */
			[91] = KEY_RESERVED, /* INVALID */
			[92] = KEY_ENTER,
			[93] = KEY_TITLE
		},
	},
	#endif
	#ifdef RMCTL_TV_BOX
	{
		.vendor_name = "TV_BOX",
		.vender_id = 0x2fd,
		.key_codes = {
			[0x57] = KEY_END,      /* power down */
			[0x56] = KEY_VOLUMEDOWN, /* vol- */
			[0x14] = KEY_VOLUMEUP,   /* vol+ */
			[0x53] = KEY_HOME,       /* home */
			[0x11] = KEY_MENU,       /* menu */
			[0x10] = KEY_BACK,       /* back */
			[0x4b] = KEY_ZOOMOUT,    /* zoom out */
			[0x08] = KEY_ZOOMIN,     /* zoom in */
			[0x0d] = KEY_UP,         /* up */
			[0x4e] = KEY_LEFT,       /* left */
			[0x19] = KEY_REPLY,      /* OK */
			[0x0c] = KEY_RIGHT,      /* right */
			[0x4f] = KEY_DOWN,       /* down */
			[0x09] = KEY_PAGEUP,     /* page up */
			[0x47] = KEY_REWIND,     /* rewind */
			[0x05] = KEY_PAGEDOWN,   /* page down */
			[0x04] = KEY_FASTFORWARD /* forward */
		},
	},
	#endif
};
#undef EXTERN

#endif
