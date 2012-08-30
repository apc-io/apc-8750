/*++ 
 * linux/drivers/video/wmt/sil902x.c
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

#define SIL902X_C
/*--------------------------------------------------------------------* 
SiI902x Software customization for PC platform.
Based on :
	Video input configuration=
		24 bit RGB in + Hsync + Vsync + DE + PCLK 
		rising edge is the 1st clock after DE=High
	Audio input configuration=
		SPDIF, word size, sample frequency all refer to stream header

	SiI902x i2c device address=0x72. ( CI2CA pin=LOW) 
//================== i2c routine =========================
SiI902x i2c max speed is 100KHz.
A version max speed is 400KHz

Data = I2C_ReadByte(TX_SLAVE_ADDR, RegOffset);
I2C_WriteByte(TPI_BASE_ADDR, RegOffset, Data);
I2C_WriteBlock(TPI_BASE_ADDR, TPI_Offset, pData, NBytes);
I2C_ReadBlock(TPI_BASE_ADDR, TPI_Offset, pData, NBytes);

------------------------------------------------------------------------------*/
// #define DEBUG
/*----------------------- DEPENDENCE -----------------------------------------*/
#include "../vout.h"

/*----------------------- PRIVATE MACRO --------------------------------------*/
/* #define  SIL902X_XXXX  xxxx    *//*Example*/
#define SIL902X_CONTENT_PROTECT
#define SIL902X_CP_POLL
#ifdef SIL902X_CP_POLL
#define SIL902X_CP_POLL_LINK_STATUS
#define SIL902X_CP_POLL_CP_STATE
#endif

//#define SIL902X_PLUG_POLL

/*----------------------- PRIVATE CONSTANTS ----------------------------------*/
/* #define SIL902X_XXXX    1     *//*Example*/
#define SIL902X_ADDR 	0x72

#define EXTENDED_LINK_PROTECTION_MASK		(0x80)
#define EXTENDED_LINK_PROTECTION_NONE		(0x00)
#define EXTENDED_LINK_PROTECTION_SECURE		(0x80)

#define LOCAL_LINK_PROTECTION_MASK			(0x40)
#define LOCAL_LINK_PROTECTION_NONE			(0x00)
#define LOCAL_LINK_PROTECTION_SECURE		(0x40)

/*----------------------- PRIVATE TYPE  --------------------------------------*/
/* typedef  xxxx sil902x_xxx_t; *//*Example*/
typedef enum {
	VMODE_640x480,
	VMODE_480p60,
	VMODE_480i60,
	VMODE_576p50,
	VMODE_576i50,
	VMODE_800x600,
	VMODE_1024x768,
	VMODE_720p50,
	VMODE_720p60,
	VMODE_1280x1024,
	VMODE_1400x1050,
	VMODE_1600x1200,
	VMODE_1680x1050,
	VMODE_1080i50,
	VMODE_1080p50,
	VMODE_1080i60,
	VMODE_1080p60,
	VMODE_1920x1200,
	VMODE_MAX
} sil902x_mode_t;

typedef enum {
	SIL902X_PWR_NORMAL,		// D0 state = TMDS TX On, audio codec On
	SIL902X_PWR_STANDBY,	// D2 state = TMDS Tx Off, audio codec Off, i2c still alive
	SIL902X_PWR_D3HOT,		// D3 state = HPD event can generate int
	SIL902X_PWR_D3COLD,		// D3 state = lowest power , HPD & RSEN CAN'T generate int
} sil902x_pwr_mode_t;

/*----------EXPORTED PRIVATE VARIABLES are defined in sil902x.h  -------------*/
/*----------------------- INTERNAL PRIVATE VARIABLES - -----------------------*/
/* int  sil902x_xxx;        *//*Example*/
sil902x_mode_t sil902x_videoMode;
int sil902x_colfmt;
int sil902x_datawidth;
int sil902x_audio_fmt = 24;
int sil902x_audio_freq = 48000;
int sil902x_audio_channel = 2;
int sil902x_hdmi_edid = 1;
int sil902x_cp_start = 0;
unsigned int sil902x_cp_linkProtectionLevel;

/*--------------------- INTERNAL PRIVATE FUNCTIONS ---------------------------*/
/* void sil902x_xxx(void); *//*Example*/

static char ReadByte(unsigned int index)
{
	unsigned char temp;

	vpp_i2c_read(SIL902X_ADDR,index,&temp,1);
	return temp;
}

static void WriteByte(unsigned int index,char data)
{
	vpp_i2c_write(SIL902X_ADDR,index,(unsigned char *)&data,1);
}

static void WriteBlock(unsigned int index,char *buf,int len)
{
	vpp_i2c_write(SIL902X_ADDR,index,(unsigned char *)buf,len);
}

void ReadModifyWrite(unsigned int index,char mask,char data)
{
    char reg;

    reg = ReadByte(index);
    reg &= ~mask;
	reg |= (data & mask);
    WriteByte(index,reg);
}

/*----------------------- Function Body --------------------------------------*/
#ifdef SIL902X_CONTENT_PROTECT
int sil902x_CP_check_supported(void)
{
    char reg;
	int support;

	support = 1;

	// Check Device ID
    reg = ReadByte(0x30);
    if (reg != 0x12){
    	support = 0;
	}

	if( ReadByte(0x36) == 0x90 ){
		if( ReadByte(0x37) == 0x22 ){
			if( ReadByte(0x38) == 0xA0 ){
				if( ReadByte(0x39) == 0x00 ){
					if( ReadByte(0x3A) == 0x00 ){
						support = 0;
					}
				}
			}
		}
	}
	return support;
}

int sil902x_CP_check_AKSV(void)
{
    char B_Data[5];
    int NumOfOnes = 0;
    int i;
    int j;

    for (i=0; i < 5; i++){
		B_Data[i] = ReadByte(0x36 + i);
        for (j=0; j < 8; j++){
            if (B_Data[i] & 0x01){
                NumOfOnes++;
            }
            B_Data[i] >>= 1;
        }
     }
     if (NumOfOnes != 20)
        return 0;

    return 1;
}

#ifdef READKSV
int sil902x_CP_IsRepeater(void)
{
    char reg;

	DBGMSG((">>IsRepeater()\n"));

    reg = ReadByte(0x29);
    if (reg & 0x8)
        return 1;
    return 0;           // not a repeater
}

int sil902x_CP_get_KSV(void)
{
	char temp;
    int KeyCount;
    char KSV_Array[128];

	do {
		WriteByte(0xBC,0x1);
		WriteByte(0xBD,0x25);
		temp = ReadByte(0xBE);
	} while( temp > 0x70 );

	ReadModifyWrite(0x1A,0x6,0x6);

	DBGMSG("[SIL902X] GetKSV()\n");
	vpp_i2c_read(0x74,0x41,&temp,1);
    KeyCount = (temp & 0x7F) * 5;
	if (KeyCount != 0){
		vpp_i2c_read(0x74,0x43,KSV_Array,KeyCount);
	}
#if 1
{
	int i;
	DBGMSG("KeyCount = %d\n", (int) KeyCount);
	for (i=0; i<KeyCount; i++){
		DPRINT("KSV[%d] = %x\n", (int) i, (int) KSV_Array[i]);
	}
}
#endif
	temp = ReadByte(0x1A);
	ReadModifyWrite(0x1A,0x8,0x0);
	return 1;
}
#endif

void sil902x_CP_get_bksv(char *bksv)
{
	int i;
	for(i=0;i<5;i++){
		bksv[i] = ReadByte(0x2B+i);
	}
}

void sil902x_CP_init(void)
{
//	HDCP_TxSupports = FALSE;
	sil902x_cp_start = 0;
	sil902x_cp_linkProtectionLevel = EXTENDED_LINK_PROTECTION_NONE | LOCAL_LINK_PROTECTION_NONE;

	// This is TX-related... need only be done once.
	if( !sil902x_CP_check_supported() ){
		DPRINT("[SIL902X] not support CP\n");
		return;
	}

	// This is TX-related... need only be done once.
    if (!sil902x_CP_check_AKSV())
    {
    	DPRINT("[SIL902X] illegal AKSV\n");
        return;
    }

#ifndef SIL902X_CP_POLL_LINK_STATUS
	ReadModifyWrite(0x3C, 0x20, 0x20); // turn on SECURITY_CHANGE_EVENT & HDCP_CHANGE_EVENT interrupt
#endif
#ifndef SIL902X_CP_POLL_CP_STATE
	ReadModifyWrite(0x3C, 0x80, 0x80); // turn on SECURITY_CHANGE_EVENT & HDCP_CHANGE_EVENT interrupt
#endif

	// Both conditions above must pass or authentication will never be attempted. 
	sil902x_cp_start = 1;

	DBGMSG("[SIL902X] HDCP init\n");
}

void sil902x_CP_enable(int enable)
{
	DBGMSG("[SIL902X] HDCP enable %d\n",enable);

	if( enable ){
		WriteByte(0x2A,0x1);
		sil902x_cp_start = 1;
	}
	else {
		ReadModifyWrite(0x1A,0x8,0x8);	// turn on AV_MUTE_MUTED
		WriteByte(0x2A,0x0);

		sil902x_cp_start = 0;
		sil902x_cp_linkProtectionLevel = EXTENDED_LINK_PROTECTION_NONE | LOCAL_LINK_PROTECTION_NONE;
	}
}

void sil902x_TMDS_enable(int enable)
{
	if( enable ){
		ReadModifyWrite(0x1A,0x10,0x0);	// turn off TMDS_OUTPUT_CONTROL_ACTIVE
	//	tmdsPoweredUp = TRUE;
	}
	else {
		ReadModifyWrite(0x1A,0x18,0x18); // turn on TMDS_OUTPUT_CONTROL_POWER_DOWN & AV_MUTE_MUTED
	//	tmdsPoweredUp = FALSE;
	}
}

void sil902x_CP_restart(void)
{
	DBGMSG("HDCP -> Restart\n");

	sil902x_TMDS_enable(0);
	sil902x_CP_enable(0);
	sil902x_TMDS_enable(1);
}

int sil902x_CP_check_status(char InterruptStatusImage)
{
	char reg;
	unsigned int NewLinkProtectionLevel;
#ifdef READKSV
	char RiCnt;
#endif
	int ret = 0; // 1:auto ok,-1:cp fail

//	DPRINT("[SIL902X] CP check status 0x%x\n",InterruptStatusImage);

	// Check if Link Status has changed:
	if (InterruptStatusImage & 0x20){
		DBGMSG("HDCP -> ");

		reg = ReadByte(0x29);
		reg &= 0x30;

#ifdef SIL902X_CP_POLL_LINK_STATUS
		WriteByte(0x3D,0x20);	// clear SECURITY_CHANGE_EVENT bit
#endif
		switch (reg){
			case 0x00: // LINK_STATUS_NORMAL
				DBGMSG("Link = Normal\n");
				break;
			case 0x10: // LINK_STATUS_LINK_LOST
				DBGMSG("Link = Lost\n");
				sil902x_CP_restart();
				ret = -1;
				break;
			case 0x20: // LINK_STATUS_RENEGOTIATION_REQ
				DBGMSG("Link = Renegotiation Required\n");
				sil902x_CP_enable(0);
				sil902x_CP_enable(1);
				ret = -1;
				break;
			case 0x30: // LINK_STATUS_LINK_SUSPENDED
				DBGMSG("Link = Suspended\n");
				sil902x_CP_enable(1);
				break;
		}
	}

	// Check if CP state has changed:
	if (InterruptStatusImage & 0x80){
		reg = ReadByte(0x29);
		NewLinkProtectionLevel = reg & (EXTENDED_LINK_PROTECTION_MASK | LOCAL_LINK_PROTECTION_MASK);
		if (NewLinkProtectionLevel != sil902x_cp_linkProtectionLevel){
			DBGMSG("HDCP -> ");

			sil902x_cp_linkProtectionLevel = NewLinkProtectionLevel;

			switch (sil902x_cp_linkProtectionLevel){
				case (EXTENDED_LINK_PROTECTION_NONE | LOCAL_LINK_PROTECTION_NONE):
					DBGMSG("Protection = None\n");
					sil902x_CP_restart();
					ret = -1;
					break;

				case LOCAL_LINK_PROTECTION_SECURE:
					ReadModifyWrite(0x1A,0x8,0x0);
					DBGMSG("Protection = Local, Video Unmuted\n");
					ret = 1;
					break;

				case (EXTENDED_LINK_PROTECTION_SECURE | LOCAL_LINK_PROTECTION_SECURE):
					DBGMSG("Protection = Extended\n");
#ifdef READKSV
					if (sil902x_CP_IsRepeater()){
						RiCnt = ReadIndexedRegister(INDEXED_PAGE_0, 0x25);
						while (RiCnt > 0x70){  // Frame 112
							RiCnt = ReadIndexedRegister(INDEXED_PAGE_0, 0x25);
						}
						ReadModifyWriteTPI(TPI_SYSTEM_CONTROL_DATA_REG, 0x06, 0x06);
						sil902x_CP_get_KSV();
						RiCnt = ReadByteTPI(TPI_SYSTEM_CONTROL_DATA_REG);
						ReadModifyWriteTPI(TPI_SYSTEM_CONTROL_DATA_REG, 0x08, 0x00);
					}
#endif
					break;
				default:
					DBGMSG("Protection = Extended but not Local?\n");
					sil902x_CP_restart();
					ret = -1;
					break;
			}
		}
#ifdef SIL902X_CP_POLL_CP_STATE
		WriteByte(0x3D,0x80);	// clear HDCP_CHANGE_EVENT bit
#endif
	}
	return ret;
}
#endif

/*!*************************************************************************
* sil902x_enable_access()
* 
* Private Function by Sam Shen, 2011/05/26
*/
/*!
* \brief	To enable access SiI902x regs, need to write Reg0xC7=00 after power on.
*		
* \retval  None
*/ 
void sil902x_enable_access(void)
{
	char reg;

	reg = ReadByte(0x1B);	// dummy read
	WriteByte(0xC7,0x0);
	WriteByte(0x1E,0x0);	// wakeup to D0 state
	
	WriteByte(0xBC,0x1);	// internal page 2
	WriteByte(0xBD,0x82);	// index reg 0x82
	reg = ReadByte(0xBE);
	reg |= 0x1;
	WriteByte(0xBE,reg);	// backdoor turn on internal source termination
} /* End of sil902x_enable_access */

/*!*************************************************************************
* sil902x_read_EDID()
* 
* Private Function by Sam Shen, 2011/05/26
*/
/*!
* \brief	read EDID
*		
* \retval  None
*/
void sil902x_do_read_edid(int index,unsigned char *buf)
{
	char int_bk;
	char reg;

//	DPRINT("[SIL902X] read edid\n");

	WriteByte(0xC7,0x0);	// access backdoor

	WriteByte(0xBC,0x0);	// program back door page 0
	WriteByte(0xBD,0xF5);
	WriteByte(0xBE,0x0);	// prevent DDC NoACK lock the i2c

	int_bk = ReadByte(0x3C);
	WriteByte(0x3C,(int_bk & 0xFE));	// disable HPD & RSEN
	reg = ReadByte(0x1A);
	WriteByte(0x1A,(reg | 0x4));		// request DDC access
	mdelay(1);
	WriteByte(0x1A,(reg | 0x6));		// lock i2c bypass to DDC
	mdelay(1);

	if( index < 0x100 ){
		vpp_i2c_read(0xA0,index,buf,128);
	}
	else {
		vpp_i2c_enhanced_ddc_read(0xA0,index-0x100,buf,128);
	}
	WriteByte(0x1A,(reg & 0xF9));			// clear "request DDC access" bit, stop i2c bypass to DDC, switch i2c to 902x regs bank
	while((ReadByte(0x1A) & 0x6) != 0 ){
		int cnt = 0;

		if( cnt > 10000 ){
			DPRINT("[SIL902X] wait switch i2c\n");
		}
		
	}	// wait switch i2c to 902x regs bank
	WriteByte(0x3C,int_bk);
}

int sil902x_read_edid(char *buf)
{
	sil902x_do_read_edid(0,buf);
	if( buf[0x7E] >= 1 ){
		sil902x_do_read_edid(0x80,&buf[128]);
	}
	if( buf[0x7E] >= 2 ){
		sil902x_do_read_edid(0x100,&buf[256]);
	}
	if( buf[0x7E] >= 3 ){
		sil902x_do_read_edid(0x180,&buf[384]);
	}
	return 0;
} /* End of sil902x_read_EDID */

/*!*************************************************************************
* sil902x_get_vmode()
* 
* Private Function by Sam Shen, 2011/05/26
*/
/*!
* \brief	get video mode
*		
* \retval  video mode
*/ 
sil902x_mode_t sil902x_get_vmode(vout_info_t *info)
{
	sil902x_mode_t mode;
	unsigned int pixclock;

	pixclock = info->timing.pixel_clock;
	mode = VMODE_MAX;
	switch( info->resx ){
		case 640:
    		mode = VMODE_640x480;
			break;
		case 720:
			if( info->resy == 480 ){
				if( pixclock == 27000000 ){
					mode = VMODE_480i60;
				}
				else {
					mode = VMODE_480p60;
				}
			}
			if( info->resy == 576 ){
				if( pixclock == 27000000 ){
				    mode = VMODE_576p50;
				}
				else {
				    mode = VMODE_576i50;
				}
			}
			break;
		case 800:
			if( info->resy == 600 ){
				mode = VMODE_800x600;
			}
			break;
		case 1024:
			if( info->resy == 768 ){
				mode = VMODE_1024x768;
			}
			break;
		case 1280:
			if( info->resy == 720 ){
				if( pixclock == 74250050 ){
					mode = VMODE_720p50;
				}
				else {
					mode = VMODE_720p60;
				}
			}
			if( info->resy == 1024 ){
				mode = VMODE_1280x1024;
			}
			break;
		case 1400:
			if( info->resy == 1050 ){
				mode = VMODE_1400x1050;
			}
			break;
		case 1600:
			if( info->resy == 1200 ){
				mode = VMODE_1600x1200;
			}
			break;
		case 1680:
			if( info->resy == 1050 ){
				mode = VMODE_1680x1050;
			}
			break;
    	case 1920:
			if( info->resy == 1080 ){			
				switch( pixclock ){
					case 74250060:
					    mode = VMODE_1080i60;
						break;
				    case 74250050:
				    	mode = VMODE_1080i50;
						break;
					case 148500000:
				    	mode = VMODE_1080p60;
						break;
					case 148500050:
				    	mode = VMODE_1080p50;
						break;
				}
			}
			if( info->resy == 1200 ){
				mode = VMODE_1920x1200;
			}
			break;
		default:
			break;
	}

	if( mode == VMODE_MAX ){
		DPRINT("[sil902x] *E* not support %dx%d\n",info->resx,info->resy);
	}
	return mode;
} /* End of sil902x_get_vmode */

/*!*************************************************************************
* sil902x_get_audio()
* 
* Private Function by Sam Shen, 2011/05/26
*/
/*!
* \brief	get audio mode
*		
* \retval  audio mode
*/ 
char sil902x_get_audio(void)
{
	char Data;

	switch( sil902x_audio_freq ){
		case 32000: Data = 0x08; break;
		case 44100: Data = 0x10; break;
		case 48000: Data = 0x18; break;
		case 88200: Data = 0x20; break;
		case 96000: Data = 0x28; break;
		case 176400: Data = 0x30; break;
		case 192000: Data = 0x38; break;
		default: Data = 0x0; break;
	}

	switch( sil902x_audio_fmt ){
		case 16: Data |= 0x40; break;
		case 20: Data |= 0x80; break;
		case 24: Data |= 0xC0; break;
		default: break;
	}
	return Data;
} /* End of sil902x_get_audio */

/*!*************************************************************************
* sil902x_active()
* 
* Private Function by Sam Shen, 2011/05/26
*/
/*!
* \brief	active device
*		
* \retval  None
*/ 
void sil902x_active(sil902x_mode_t videoMode)
{
	const char audioInfoFrame[15]={0xc2,0x84,0x01,0x0A,0x71,0,0,0,0,0,0,0,0,0,0};	
	const char videoKRHV[VMODE_MAX][8] = {
		{0xd5, 0x09, 0x70, 0x17, 0x20, 0x03, 0x0d, 0x02}, // VMODE_640x480
		{0x8c, 0x0a, 0x70, 0x17, 0x5a, 0x03, 0x0d, 0x02}, // VMODE_480p60
		{0x8c, 0x0a, 0x70, 0x17, 0xB4, 0x06, 0x06, 0x01}, // VMODE_480i60
		{0x8c, 0x0a, 0x88, 0x13, 0x60, 0x03, 0x71, 0x02}, // VMODE_576p50
		{0x8c, 0x0a, 0x88, 0x13, 0xC0, 0x06, 0x38, 0x01}, // VMODE_576i50
		{0xa0, 0x0f, 0x70, 0x17, 0x20, 0x04, 0x74, 0x02}, // VMODE_800x600
		{0xe0, 0x15, 0x70, 0x17, 0x20, 0x05, 0x20, 0x03}, // VMODE_1024x768
		{0x01, 0x1d, 0x88, 0x13, 0xbc, 0x07, 0xee, 0x02}, // VMODE_720p50
		{0x01, 0x1d, 0x70, 0x17, 0x72, 0x06, 0xee, 0x02}, // VMODE_720p60
		{0x30, 0x2a, 0x70, 0x17, 0x98, 0x06, 0x2a, 0x04}, // VMODE_1280x1024
		{0x93, 0x2f, 0x70, 0x17, 0x48, 0x07, 0x41, 0x04}, // VMODE_1400x1050
		{0x48, 0x3f, 0x70, 0x17, 0x70, 0x08, 0xe2, 0x04}, // VMODE_1600x1200
		{0x7c, 0x2e, 0x70, 0x17, 0x30, 0x07, 0x38, 0x04}, // VMODE_1680x1050
		{0x01, 0x1d, 0xC4, 0x09, 0x50, 0x0A, 0x32, 0x02}, // VMODE_1080i50
		{0x02, 0x3a, 0x88, 0x13, 0x50, 0x0A, 0x65, 0x04}, // VMODE_1080p50
		{0x01, 0x1d, 0xb8, 0x0b, 0x98, 0x08, 0x32, 0x02}, // VMODE_1080i60
		{0x02, 0x3a, 0x70, 0x17, 0x98, 0x08, 0x65, 0x04}, // VMODE_1080p60
		{0x28, 0x3C, 0x70, 0x17, 0x20, 0x08, 0xd3, 0x04}, // VMODE_1920x1200
	};
	const char aviInfoFrame[VMODE_MAX+1][14] = {
		{ 0x5E, 0x00, 0x10, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // VMODE_640x480
		{ 0x73, 0x0d, 0x68, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // VMODE_480p60
		{ 0x50, 0x00, 0x18, 0x00, 0x06, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // VMODE_480i60
		{ 0x64, 0x0d, 0x68, 0x00, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // VMODE_576p50
		{ 0x41, 0x00, 0x18, 0x00, 0x15, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // VMODE_576i50
		{ 0xd1, 0x0e, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // VMODE_800x600 - un-recognized mode
		{ 0xd1, 0x0e, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // VMODE_1024x768 - un-recognized mode
		{ 0x23, 0x0d, 0xa8, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // VMODE_720p50
		{ 0x32, 0x0d, 0xa8, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // VMODE_720p60
		{ 0xd1, 0x0e, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // VMODE_1280x1024 - un-recognized mode
		{ 0xd1, 0x0e, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // VMODE_1400x1050 - un-recognized mode
		{ 0xd1, 0x0e, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // VMODE_1600x1200 - un-recognized mode
		{ 0xd1, 0x0e, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // VMODE_1680x1050 - un-recognized mode
		{ 0x22, 0x0d, 0xa8, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // VMODE_1080i50
		{ 0x17, 0x0d, 0xa8, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // VMODE_1080p50
		{ 0x31, 0x0d, 0xa8, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // VMODE_1080i60
		{ 0x27, 0x0c, 0xa8, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // VMODE_1080p60
		{ 0xd1, 0x0e, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // VMODE_1920x1200 - un-recognized mode
		{ 0xd1, 0x0e, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // VMODE_MAX - DVI or un-recognized mode
	};
	char reg;	

	sil902x_enable_access();
	if( videoMode >= VMODE_MAX ){
		DPRINT("[SIL902x] no support\n");
		return;
	}
	WriteBlock(0x0,(char *)&videoKRHV[videoMode][0],8);

#ifdef SIL902X_CONTENT_PROTECT
	sil902x_CP_enable(0);
#endif

	if(sil902x_hdmi_edid){
		WriteByte(0x1A,0x11);				// Turn off TMDS , HDMI mode
		WriteByte(0x26,0x50);				// audio in=SPDIF, mute, refer header 
		WriteByte(0x28,0);					// Set speaker =FL+FR
		WriteByte(0x25,0x03);				// Don't check audio stream 
		WriteByte(0x27,sil902x_get_audio());// Audio size, sampling frequency, channel number all refer to header 

		// I2S config
		WriteByte(0x1F,0x80);
		WriteByte(0x20,0x90);

		reg = (vpp_get_hdmi_spdif())? 0x40:0x80;	// bit7:6	 01:SPDIF, 10:I2S
		WriteByte(0x26,reg);				// SPDIF, un-mute, refer header 
 
		// Following 3 instruction only for 9022/9024 non-A version, that re-set layout=2 channel audio after unMute audio
		WriteByte(0xBC,0x02);	
		WriteByte(0xBD,0x2F);
		reg = ReadByte(0xBE);
		WriteByte(0xBE,reg & 0xFD);		// Layout=2 channel
		WriteByte(0x1A,0x01);			// Turn on TMDS , encoder format=HDMI
		WriteBlock(0xBF,(char *)audioInfoFrame, 15); // enable & send audio infoframe every frame , 
	}
	else {	// DVI
		WriteByte(0x1A,0x00);			// Turn on TMDS , encoder format=DVI
		videoMode = VMODE_MAX;
	}
	
	reg = (sil902x_datawidth)? 0x70:0x50;		// bit5: 0-12bit, 1-24bit
	WriteByte(0x08,reg);				// 12 bit video in, falling edge, no repeat ; 

	reg = sil902x_colfmt;
	WriteByte(0x09,reg);				// video in=RGB 00b, YUV444 01b, YUV 422 10b
	WriteByte(0x0A,reg);				// video out=RGB 00b, YUV444 01b, YUV 422 10b

	reg = aviInfoFrame[videoMode][1] & 0x9F;
	switch( sil902x_colfmt ){
		default:
		case 0x0:	// RGB 00
			break;
		case 0x1:	// 444
			reg |= (0x2 << 5);
			break;
		case 0x2:	// 422
			reg |= (0x1 << 5);
			break;
	}

	{	// calculate check sum
		unsigned char sum;
		int i;

		sum = 0x91 + reg;
		for(i=2;i<14;i++){
			sum += aviInfoFrame[videoMode][i];
		}
		WriteByte(0x0C,(0-sum));
	}
	WriteByte(0x0D,reg);
	WriteBlock(0x0E,(char *)&aviInfoFrame[videoMode][2], 12);	// send AVI infoframe 

} /* End of sil902x_active */

/*!*************************************************************************
* sil902x_set_power_mode()
* 
* Private Function by Sam Shen, 2011/05/26
*/
/*!
* \brief	set power mode
*		
* \retval  None
*/ 
void sil902x_set_power_mode(sil902x_pwr_mode_t mode)
{
	char reg;

	switch( mode ){
		case SIL902X_PWR_NORMAL:
			reg = ReadByte(0x1B );			// Dummy read, clear unfinished i2c cycle if any.
			reg = ReadByte(0x1E);
			WriteByte(0x1E, reg & 0xFC);	// Reg1E[1:0]=00 binary =D0 mode
			reg = ReadByte(0x1A);
			WriteByte(0x1E,reg & 0xEF) ;	// Reg1A[4]=0=enable TMDS
			break;
		case SIL902X_PWR_STANDBY:
			reg = ReadByte(0x1E);
			WriteByte(0x1E,(reg & 0xFC) | 0x2);	// Reg 0x1E [1:0] = 10 standby mode
			break;
		case SIL902X_PWR_D3HOT:
			WriteByte(0x3C,0x0);	//Disable HPD & RSEN
			WriteByte(0x3D,0xFF);	//Clear HPD & RSEN pending int
			WriteByte(0x3C,0x1);	//Enable HPD 
										
			reg = ReadByte(0x1E);	//Reg1E[1:0]=11 binary =D3 hot mode
			WriteByte(0x1E, (reg & 0xFC) | 03);	
			// Go D3(i2c stop working) hot mode <-- Write this Data will get NoAck, 
			// A dummy read to device 0x72 , get NoAck can confirm it's in D3 state
			// HPD can generate int to MCU. MCU HW reset# SiI902x back to normal operation mode
			break;
		case SIL902X_PWR_D3COLD:
			reg = ReadByte(0x1E);				//Reg1E[2]=1 =declare D3 cold mode
			WriteByte(0x1E,(reg & 0xFC) | 04);
			WriteByte(0x1E,(reg & 0xFC) | 07);	//Go D3 cold mode <-- Write this Data Will get NoAck, but reduce to lowest power 
			// A dummy read to device 0x72 , get NoAck can confirm it's in D3 state			
			break;
		default:
			break;
	}
} /* End of sil902x_set_power_mode */

/*!*************************************************************************
* sil902x_set_interrupt()
* 
* Private Function by Sam Shen, 2011/05/26
*/
/*!
* \brief	enable interrupt
*		
* \retval  None
*/ 
void sil902x_set_interrupt(int enable)
{
	char reg;
	
	reg = ReadByte(0x3C);
	reg = (enable)? (reg | 0x1):(reg & ~0x1);
	WriteByte(0x3C,reg);	
} /* End of sil902x_set_interrupt */

/*!*************************************************************************
* sil902x_clr_interrupt()
* 
* Private Function by Sam Shen, 2011/05/26
*/
/*!
* \brief	clear interrupt status
*		
* \retval  None
*/ 
int sil902x_clr_interrupt(void)
{
	char reg;
	
	reg = ReadByte(0x3D);				//Reg3C[0]=1  Enable HotPlug interrupt	
#if 1
	WriteByte(0x3D,reg);
#else
	if (reg & 0x01){
		WriteByte(0x3D, reg | 0x1 ) ;	//clear pending HotPlug interrupt
		return 1;
	}
#endif
	return 0;
} /* End of sil902x_clr_interrupt */

/*!*************************************************************************
* sil902x_mute_audio()
* 
* Private Function by Sam Shen, 2011/05/26
*/
/*!
* \brief	audio mute or unmute
*		When mute audio, Audio source should keep sending MCLK, BCLK, WS signal. Just stop SD data.
*		If change MCLK, BCLK,WS, or change video mode, should call active to umMute.
* \retval  None
*/ 
void sil902x_mute_audio(int enable)
{
	char reg;
	
	if( enable ){
		reg = ReadByte(0x26);
		WriteByte(0x26,reg | 0x10);
	}
	else {
		char audioInfoFrame[]={0xc2,0x84,0x01,0x0A,0x71,0,0,0,0,0,0,0,0,0,0};
	
		reg = ReadByte(0x26);	
		WriteByte(0x26, reg & 0xEF ) ;	
		// For non-A version, always need to re-set layout = 0 after unMute audio
		// For A version , don't need following 4 instructions
		WriteByte(0xBC,0x02);				// internal page 2
		WriteByte(0xBD,0x2F);				// index reg 0x2F
		reg = ReadByte(0xBE);
		WriteByte(0xBE,reg & 0xFD);			// Layout=2 channel
		WriteBlock(0xBF, audioInfoFrame, 15);// enable & send audio infoframe every frame , 
	}
} /* End of sil902x_mute_audio */

/*!*************************************************************************
* sil902x_check_plugin()
* 
* Private Function by Sam Shen, 2011/05/26
*/
/*!
* \brief	check device plugin status
*		
* \retval  None
*/ 
int sil902x_check_plugin(int hotplug)
{
	int plugin = 0;
	unsigned char *buf;
	int option = 0; 

//	sil902x_clr_interrupt();
	plugin = (ReadByte(0x3D) & 0x04)? 1:0;
	if( hotplug ){
		if (plugin){
			buf = (unsigned char *)vout_get_edid(VOUT_DVO2HDMI);
			option = edid_parse_option(buf);
			sil902x_hdmi_edid = (option & EDID_OPT_HDMI)? 1:0;
			
			sil902x_active( sil902x_videoMode) ;
		}
		else {
#ifdef SIL902X_CONTENT_PROTECT
			sil902x_CP_enable(0);
#endif
			sil902x_set_power_mode(SIL902X_PWR_STANDBY);
		}
		DPRINT("[SIL902X] HDMI plug%s,option 0x%x\n",(plugin)?"in":"out",option);
	}
	vout_set_int_type(2);
	return plugin;
} /* End of sil902x_check_plugin */

int sil902x_interrupt(void)
{
	int plugin = 0;
	char status;
	vout_t *vo;

	vo = vout_get_info(VOUT_DVO2HDMI);

	status = ReadByte(0x3D);
	plugin = (status & 0x04)? 1:0;
	if(!(status & ReadByte(0x3C)))
		return plugin;

	DPRINT("[SIL902X] interrupt 0x%x\n",status);

#ifdef SIL902X_CP_POLL_LINK_STATUS
	status &= ~0x20;
#endif
#ifdef SIL902X_CP_POLL_CP_STATE
	status &= ~0x80;
#endif
	WriteByte(0x3D,status);
	if( status & 0x1 ){
		plugin = sil902x_check_plugin(1);
	}

#ifdef SIL902X_CONTENT_PROTECT
	if( plugin ){
		if ((sil902x_cp_linkProtectionLevel == (EXTENDED_LINK_PROTECTION_NONE | LOCAL_LINK_PROTECTION_NONE)) 
				&& (sil902x_cp_start == 0)){
			char reg;
				
			reg = ReadByte(0x29);
			DBGMSG("[SIL902X] HDCP detect %d\n",(reg&0x2));
			if (reg & 0x2){   // Is CP avaialable
				sil902x_CP_enable(1);
			}
		}
	}
	else {
		vo->status &= ~VPP_VOUT_STS_CONTENT_PROTECT;
	}
#ifndef SIL902X_CP_POLL
	if( status & 0xA0 ){
		switch( sil902x_CP_check_status(status) ){
			case 1: // cp ok
				sil902x_CP_get_bksv(&g_vpp.hdmi_bksv[0]);
				vo->status |= VPP_VOUT_STS_CONTENT_PROTECT;
				break;
			case -1: // cp fail
				vo->status &= ~VPP_VOUT_STS_CONTENT_PROTECT;
				break;
			default:
				break;
		}
	}
#endif
#endif
//	status = ReadByte(0x3D);
//	DPRINT("[SIL902X] End interrupt 0x%x\n",status);
	return plugin;
}

void sil902x_poll(void)
{
	vout_t *vo;

	vo = vout_get_info(VOUT_DVO2HDMI);
	
#ifdef SIL902X_PLUG_POLL
{
	int plugin;
	
	if( ReadByte(0x3D) & 0x1 ){
		WriteByte(0x3D,0x1);
		plugin = sil902x_check_plugin(1);
		govrh_set_dvo_enable(plugin);
		vout_change_status(vo,VOCTL_CHKPLUG,0,plugin);
#ifdef CONFIG_WMT_CEC
		wmt_cec_hotplug_notify(plugin);
#endif
	}
	else {
		plugin = (ReadByte(0x3D) & 0x04)? 1:0;
	}

#ifdef SIL902X_CONTENT_PROTECT
	if( plugin ){
		if ((sil902x_cp_linkProtectionLevel == (EXTENDED_LINK_PROTECTION_NONE | LOCAL_LINK_PROTECTION_NONE)) 
				&& (sil902x_cp_start == 0)){
			char reg;
				
			reg = ReadByte(0x29);
			DBGMSG("[SIL902X] HDCP detect %d\n",(reg&0x2));
			if (reg & 0x2){   // Is CP avaialable
				sil902x_CP_enable(1);
			}
		}
	}
	else {
		vo->status &= ~VPP_VOUT_STS_CONTENT_PROTECT;
	}
#endif
}
#endif

#ifdef SIL902X_CP_POLL
{
	char status;

	vo = vout_get_info(VOUT_DVO2HDMI);
	if( !(vo->status & VPP_VOUT_STS_PLUGIN) ){
		return;
	}
	status = ReadByte(0x3D) & 0xA0;
#ifndef SIL902X_CP_POLL_LINK_STATUS
	status &= ~0x20;
#endif
#ifndef SIL902X_CP_POLL_CP_STATE
	status &= ~0x80;
#endif
	switch( sil902x_CP_check_status(status) ){
		case 1: // cp ok
			sil902x_CP_get_bksv((char *)&g_vpp.hdmi_bksv[0]);
			vo->status |= VPP_VOUT_STS_CONTENT_PROTECT;
			break;
		case -1: // cp fail
			vo->status &= ~VPP_VOUT_STS_CONTENT_PROTECT;
			break;
		default:
			break;
	}
}
#endif
}

/*!*************************************************************************
* sil902x_set_power_down()
* 
* Private Function by Sam Shen, 2011/05/26
*/
/*!
* \brief	set power down
*		
* \retval  None
*/ 
void sil902x_set_power_down(int enable)
{
	return;
	if( enable ){
		sil902x_set_power_mode(SIL902X_PWR_D3COLD);
	}
	else {
		sil902x_set_power_mode(SIL902X_PWR_NORMAL);
	}
} /* End of sil902x_set_power_down */

/*!*************************************************************************
* sil902x_set_mode()
* 
* Private Function by Sam Shen, 2011/05/26
*/
/*!
* \brief	set mode arguments
*		
* \retval  None
*/ 
int sil902x_set_mode(unsigned int *option)
{
	DBGMSG("option %d,%d\n",option[0],option[1]);

	sil902x_datawidth = (option[1] & 0x1)? 1:0;		// 1-24bit,0-12bit
	switch(option[0]){
		case VDO_COL_FMT_ARGB:
			sil902x_colfmt = 0;
			break;
		case VDO_COL_FMT_YUV444:
			sil902x_colfmt = 0x1;
			break;
		case VDO_COL_FMT_YUV422H:
			sil902x_colfmt = 0x2;
			break;
		default:
			return -1;
	}
	return 0;
} /* End of sil902x_set_mode */

/*!*************************************************************************
* sil902x_config()
* 
* Private Function by Sam Shen, 2011/05/26
*/
/*!
* \brief	config device
*		
* \retval  None
*/ 
int sil902x_config(vout_info_t *info)
{
	sil902x_videoMode = sil902x_get_vmode(info);
	DBGMSG("mode %d\n",sil902x_videoMode);
	sil902x_active(sil902x_videoMode);
	return 0;
} /* End of sil902x_config */

/*!*************************************************************************
* sil902x_set_audio()
* 
* Private Function by Sam Shen, 2011/05/26
*/
/*!
* \brief	clear interrupt status
*		
* \retval  None
*/ 
int sil902x_set_audio(vout_audio_t *arg)
{
	char reg;

	sil902x_audio_freq = arg->sample_rate;
	sil902x_audio_channel = arg->channel;
	sil902x_audio_fmt = arg->fmt;

	WriteByte(0x27,sil902x_get_audio());		// Audio size, sampling frequency, channel number all refer to header 
		
	reg = (vpp_get_hdmi_spdif())? 0x40:0x80; 	// bit7:6	 01:SPDIF, 10:I2S
	WriteByte(0x26,reg); 						// SPDIF, un-mute, refer header 
	
//	sil902x_active( sil902x_videoMode) ;
//	printk("[SIL902X] set audio(freq %d)\n",arg->sample_rate);
	return 0;
} /* End of sil902x_set_audio */

//================== Check SiI902x device ID =======================
int sil902x_init(void)
{
	char reg = 0xFF;

	sil902x_enable_access();
	reg = ReadByte(0x1B);
	if( reg != 0xB0 ){	// SiI9022 or 9024 chip.
		return -1;
	}
#ifndef SIL902X_PLUG_POLL
	sil902x_set_interrupt(1);
#endif
#ifdef SIL902X_CONTENT_PROTECT
	sil902x_CP_init();
#endif
	DPRINT("[SIL902X] HDMI ext device\n");
	return 0;
}
/*----------------------- vout device plugin --------------------------------------*/
vout_dev_ops_t sil902x_vout_dev_ops = {
	.mode = VOUT_DVO2HDMI,

	.init = sil902x_init,
	.set_power_down = sil902x_set_power_down,
	.set_mode = sil902x_set_mode,
	.config = sil902x_config,
	.check_plugin = sil902x_check_plugin,
	.get_edid = sil902x_read_edid,
	.set_audio = sil902x_set_audio,
	.interrupt = sil902x_interrupt,
	.poll = sil902x_poll,
};

int sil902x_module_init(void)
{	
	vout_device_register(&sil902x_vout_dev_ops);
	return 0;
} /* End of sil902x_module_init */
module_init(sil902x_module_init);
/*--------------------End of Function Body -----------------------------------*/
#undef SIL902X_C

