/*++ 
 * linux/drivers/video/wmt/hdmi.c
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

#define HDMI_C
// #define DEBUG
/*----------------------- DEPENDENCE -----------------------------------------*/
#include "hdmi.h"
#include "vout.h"

/*----------------------- PRIVATE MACRO --------------------------------------*/

/*----------------------- PRIVATE CONSTANTS ----------------------------------*/
/* #define HDMI_XXXX    1     *//*Example*/
//#define CONFIG_HDMI_INFOFRAME_DISABLE
//#define CONFIG_HDMI_EDID_DISABLE

#define HDMI_I2C_FREQ	80000
/*----------------------- PRIVATE TYPE  --------------------------------------*/
/* typedef  xxxx hdmi_xxx_t; *//*Example*/
typedef enum {
	HDMI_FIFO_SLOT_AVI = 0,
	HDMI_FIFO_SLOT_AUDIO = 1,
	HDMI_FIFO_SLOT_CONTROL = 2,
	HDMI_FIFO_SLOT_MAX = 15
} hdmi_fifo_slot_t;

/*----------EXPORTED PRIVATE VARIABLES are defined in hdmi.h  -------------*/
/*----------------------- INTERNAL PRIVATE VARIABLES - -----------------------*/
/* int  hdmi_xxx;        *//*Example*/

/*--------------------- INTERNAL PRIVATE FUNCTIONS ---------------------------*/
/* void hdmi_xxx(void); *//*Example*/

/*----------------------- Function Body --------------------------------------*/
/*---------------------------- HDMI COMMON API -------------------------------*/
unsigned int hdmi_reg32_write(U32 offset, U32 mask, U32 shift, U32 val)
{
	unsigned int new_val;

	new_val = (vppif_reg32_in(offset) & ~(mask)) | (((val) << (shift)) & mask);
	if( offset == REG_HDMI_GENERAL_CTRL ){
		new_val &= ~( BIT24 | BIT25 | BIT26 );
		new_val |= g_vpp.hdmi_ctrl;
		
	}
	vppif_reg32_out(offset,new_val);
	return new_val;
}

unsigned char hdmi_ecc(unsigned char *buf,int bit_cnt)
{
	#define HDMI_CRC_LEN	9
	
	int crc[HDMI_CRC_LEN],crc_o[HDMI_CRC_LEN];
	int i,j;
	int input,result,result_rev = 0;

	for(i=0;i<HDMI_CRC_LEN;i++){
		crc[i] = 0;
	}

	for(i=0;i<bit_cnt;i++){
		for(j=0;j<HDMI_CRC_LEN;j++){
			crc_o[j] = crc[j];
		}
		input = (buf[i/8] & (1<<(i%8)))? 1:0;
		crc[0] = crc_o[7] ^ input;
		crc[1] = crc_o[0];
		crc[2] = crc_o[1];
		crc[3] = crc_o[2];
		crc[4] = crc_o[3];
		crc[5] = crc_o[4];
		crc[6] = crc_o[5] ^ crc_o[7] ^ input;
		crc[7] = crc_o[6] ^ crc_o[7] ^ input;
		crc[8] = crc_o[7];

		result     = 0;
		result_rev = 0;
		for (j=0;j<HDMI_CRC_LEN-1;j++){
			result     += (crc[j]<<j);
			result_rev += (crc[j]<<(HDMI_CRC_LEN-2-j));
		}
	}

	
	return result_rev;	
}

unsigned char hdmi_checksum(unsigned char *header,unsigned char *buf,int cnt)
{
	unsigned char sum;
	int i;

	for(i=0,sum=0;i<cnt;i++){
		sum += buf[i];
	}
	for(i=0;i<3;i++){
		sum += header[i];
	}
	return (0 - sum);
}

#ifdef WMT_FTBLK_HDMI
/*---------------------------- HDMI HAL --------------------------------------*/
void hdmi_set_enable(vpp_flag_t enable)
{
	hdmi_reg32_write(HDMI_ENABLE,enable);
	lvds_set_power_down((enable)?0:1);
//	lvds_set_enable((enable)? 0:1);
}

void hdmi_set_dvi_enable(vpp_flag_t enable)
{
	hdmi_reg32_write(HDMI_DVI_MODE_ENABLE,enable);
}

void hdmi_set_sync_low_active(vpp_flag_t hsync,vpp_flag_t vsync)
{
	hdmi_reg32_write(HDMI_HSYNC_LOW_ACTIVE,hsync);
	hdmi_reg32_write(HDMI_VSYNC_LOW_ACTIVE,vsync);
}

void hdmi_set_output_colfmt(vdo_color_fmt colfmt)
{
	unsigned int val;

	switch(colfmt){
		default:
		case VDO_COL_FMT_ARGB:
			val = 0;
			break;
		case VDO_COL_FMT_YUV444:
			val = 1;
			break;
		case VDO_COL_FMT_YUV422H:
		case VDO_COL_FMT_YUV422V:
			val = 2;
			break;
	}
	hdmi_reg32_write(HDMI_CONVERT_YUV422,(val==2)?1:0);
	hdmi_reg32_write(HDMI_OUTPUT_FORMAT,val);
}

vdo_color_fmt hdmi_get_output_colfmt(void)
{
	unsigned int val;

	val = vppif_reg32_read(HDMI_OUTPUT_FORMAT);
	switch(val){
		default:
		case 0: return VDO_COL_FMT_ARGB;
		case 1: return VDO_COL_FMT_YUV444;
		case 2: return VDO_COL_FMT_YUV422H;
	}
	return VDO_COL_FMT_ARGB;
}

int hdmi_get_plugin(void)
{
	return vppif_reg32_read(HDMI_HOTPLUG_IN);
}

int hdmi_get_plug_status(void)
{
	int reg;

	reg = vppif_reg32_in(REG_HDMI_HOTPLUG_DETECT);
	return (reg & 0x3000000);
}

void hdmi_clear_plug_status(void)
{
	vppif_reg32_write(HDMI_HOTPLUG_IN_STS,1);
	vppif_reg32_write(HDMI_HOTPLUG_OUT_STS,1);	
}

void hdmi_enable_plugin(int enable)
{
	vppif_reg32_write(HDMI_HOTPLUG_OUT_INT,enable);
	vppif_reg32_write(HDMI_HOTPLUG_IN_INT,enable);
}

void hdmi_write_fifo(hdmi_fifo_slot_t no,unsigned int *buf,int cnt)
{
	int i;

	if( no > HDMI_FIFO_SLOT_MAX ) return;
#ifdef DEBUG
{
	char *ptr;

	
	ptr = (char *) buf;
	
	
}	
#endif

	vppif_reg32_out(REG_HDMI_FIFO_CTRL,(no << 8));
	cnt = (cnt+3)/4;
	for(i=0;i<cnt;i++){
		vppif_reg32_out(REG_HDMI_WR_FIFO_ADDR+4*i,buf[i]);
	}

	vppif_reg32_write(HDMI_INFOFRAME_WR_STROBE,1);
}

void hdmi_read_fifo(hdmi_fifo_slot_t no,unsigned int *buf,int cnt)
{
	int i;
	
	if( no > HDMI_FIFO_SLOT_MAX ) return;
	
	vppif_reg32_out(REG_HDMI_FIFO_CTRL,(no << 8));	
	vppif_reg32_write(HDMI_INFOFRAME_RD_STROBE,1);
	for(i=0;i<cnt;i++){
		buf[i] = vppif_reg32_in(REG_HDMI_RD_FIFO_ADDR+4*i);
	}
}

#define HDMI_DDC_OUT	
#define HDMI_DDC_DELAY 			1
#define HDMI_DDC_CHK_DELAY		1
#define HDMI_DDC_CTRL_DELAY		1

#define HDMI_STATUS_START		BIT16
#define HDMI_STATUS_STOP		BIT17
#define HDMI_STATUS_WR_AVAIL	BIT18
#define HDMI_STATUS_CP_USE		BIT19
#define HDMI_STATUS_SW_READ		BIT25
int hdmi_DDC_check_status(unsigned int checkbits,int condition)
{
    int status = 1;
    unsigned int i = 0, maxloop;

	maxloop = 500 / HDMI_DDC_CHK_DELAY;
	udelay(HDMI_DDC_DELAY);

	if( condition ){ // wait 1 --> 0
		while((vppif_reg32_in(REG_HDMI_I2C_CTRL2) & checkbits) && (i<maxloop)){
			udelay(HDMI_DDC_CHK_DELAY);
            if(++i == maxloop) status = 0;
		}
	}
	else { // wait 0 --> 1
		while(!(vppif_reg32_in(REG_HDMI_I2C_CTRL2) & checkbits) && (i<maxloop)){
			udelay(HDMI_DDC_CHK_DELAY);
            if(++i == maxloop) status = 0;
		}
	}
	
	if( (status == 0) && (checkbits != HDMI_STATUS_SW_READ) ){
	// if( status == 0 ){
		unsigned int reg;

//		vpp_set_dbg_gpio(10,0);
		reg = vppif_reg32_in(REG_HDMI_I2C_CTRL2);
		
			
//		vpp_set_dbg_gpio(10,1);
	}
	
    return status;
}

void hdmi_DDC_set_freq(unsigned int hz)
{
	unsigned int clock;
	unsigned int div;

	clock = 25000000*15/100;	// RTC clock source 
	div = clock / hz;
	
	vppif_reg32_write(HDMI_I2C_CLK_DIVIDER,div);

}

void hdmi_DDC_reset(void)
{
	vppif_reg32_write(HDMI_I2C_SW_RESET,1);
	udelay(1);
	vppif_reg32_write(HDMI_I2C_SW_RESET,0);	
}

int hdmi_DDC_read_func(char addr,int index,char *buf,int length)
{
    int status = 1;
    unsigned int i = 0;
	int err_cnt = 0;



#ifdef CONFIG_HDMI_EDID_DISABLE
	return status;
#endif

	hdmi_DDC_set_freq(g_vpp.hdmi_i2c_freq);
//	vppif_reg32_write(HDMI_I2C_ENABLE,1);

	// enhanced DDC read
	if( index >= 256 ){
		vppif_reg32_write(REG_HDMI_I2C_CTRL2,BIT18+BIT16,16,0x5);	// sw start, write data avail
		udelay(HDMI_DDC_CTRL_DELAY);
		status = hdmi_DDC_check_status(HDMI_STATUS_START+HDMI_STATUS_WR_AVAIL, 1);	// wait start & wr data avail
		if( status == 0 ){

			err_cnt++;
			goto ddc_read_fail;
		}

		// Slave address
		vppif_reg32_write(HDMI_WR_DATA,0x60);
		vppif_reg32_write(HDMI_WR_DATA_AVAIL,1);				// write data avail
		udelay(HDMI_DDC_CTRL_DELAY);		
		status = hdmi_DDC_check_status(HDMI_STATUS_WR_AVAIL, 1);// wait wr data avail
		if( status == 0 ){
			
			err_cnt++;
			goto ddc_read_fail;			
		}

		// Offset
		vppif_reg32_write(HDMI_WR_DATA,0x1);
		vppif_reg32_write(HDMI_WR_DATA_AVAIL,1);				// write data avail
		udelay(HDMI_DDC_CTRL_DELAY);		
		status = hdmi_DDC_check_status(HDMI_STATUS_WR_AVAIL, 1);// wait wr data avail
		if( status == 0 ){
			
			err_cnt++;
			goto ddc_read_fail;			
		}
		index -= 256;
	}

	// START
#ifdef HDMI_DDC_OUT
	vppif_reg32_out(REG_HDMI_I2C_CTRL2,0x50000);
#else
//	vppif_reg32_write(HDMI_SW_START_REQ,1);					// sw start
//	vppif_reg32_write(HDMI_WR_DATA_AVAIL,1);				// write data avail
	vppif_reg32_write(REG_HDMI_I2C_CTRL2,BIT18+BIT16,16,0x5);	// sw start, write data avail	
#endif
	udelay(HDMI_DDC_CTRL_DELAY);
	status = hdmi_DDC_check_status(HDMI_STATUS_START+HDMI_STATUS_WR_AVAIL, 1);	// wait start & wr data avail
	if( status == 0 ){
		
		err_cnt++;
		goto ddc_read_fail;		
	}

	// Slave address
#ifdef HDMI_DDC_OUT
	vppif_reg32_out(REG_HDMI_I2C_CTRL2,0x400A0);
#else
	vppif_reg32_write(HDMI_WR_DATA,addr);
	udelay(HDMI_DDC_DELAY);
	while( vppif_reg32_read(HDMI_WR_DATA) != addr);
	udelay(HDMI_DDC_DELAY);
	vppif_reg32_write(HDMI_WR_DATA_AVAIL,1);				// write data avail
#endif
	udelay(HDMI_DDC_CTRL_DELAY);
	status = hdmi_DDC_check_status(HDMI_STATUS_WR_AVAIL, 1);// wait wr data avail
	if( status == 0 ){
		
		err_cnt++;
		goto ddc_read_fail;		
	}

	// Offset
#ifdef HDMI_DDC_OUT
{
	unsigned int reg;

	reg = 0x40000;
	reg |= index;

	vppif_reg32_out(REG_HDMI_I2C_CTRL2,reg);
}
#else
	vppif_reg32_write(HDMI_WR_DATA,index);
	udelay(HDMI_DDC_DELAY);
	while( vppif_reg32_read(HDMI_WR_DATA) != index);
	udelay(HDMI_DDC_DELAY);
	vppif_reg32_write(HDMI_WR_DATA_AVAIL,1);				// write data avail
#endif
	udelay(HDMI_DDC_CTRL_DELAY);
	status = hdmi_DDC_check_status(HDMI_STATUS_WR_AVAIL, 1);// wait wr data avail
	if( status == 0 ){
		
		err_cnt++;
		goto ddc_read_fail;		
	}

//	vppif_reg32_write(HDMI_WR_DATA,addr+1);

    // START
#ifdef HDMI_DDC_OUT
	vppif_reg32_out(REG_HDMI_I2C_CTRL2,0x50000);
#else
//	vppif_reg32_write(HDMI_SW_START_REQ,1);					// sw start
//	vppif_reg32_write(HDMI_WR_DATA_AVAIL,1);				// write data avail
	vppif_reg32_write(REG_HDMI_I2C_CTRL2,BIT18+BIT16,16,0x5);	// sw start, write data avail	
#endif
	udelay(HDMI_DDC_CTRL_DELAY);
	status = hdmi_DDC_check_status(HDMI_STATUS_START+HDMI_STATUS_WR_AVAIL, 1);		// wait start & wr data avail
	if( status == 0 ){
		
		err_cnt++;
		goto ddc_read_fail;
	}

    // Slave Address + 1
#ifdef HDMI_DDC_OUT
	vppif_reg32_out(REG_HDMI_I2C_CTRL2,0x400A1);
#else
	vppif_reg32_write(HDMI_WR_DATA,addr+1);
	udelay(HDMI_DDC_DELAY);
	while( vppif_reg32_read(HDMI_WR_DATA) != (addr+1));
	udelay(HDMI_DDC_DELAY);
	vppif_reg32_write(HDMI_WR_DATA_AVAIL,1);				// write data avail
#endif
	udelay(HDMI_DDC_CTRL_DELAY);
	status = hdmi_DDC_check_status(HDMI_STATUS_WR_AVAIL, 1);// wait wr data avail
	if( status == 0 ){
		
		err_cnt++;
		goto ddc_read_fail;		
	}

    // Read Data
    for(i = 0; i < length; i++)
    {
#ifdef HDMI_DDC_OUT
		vppif_reg32_out(REG_HDMI_I2C_CTRL2,0x40000);
#else
		vppif_reg32_write(HDMI_WR_DATA_AVAIL,1);				// write data avail
		// hdmi_reg32_write(HDMI_WR_DATA_AVAIL,1);
#endif
		udelay(HDMI_DDC_CTRL_DELAY);
		status = hdmi_DDC_check_status(HDMI_STATUS_WR_AVAIL, 1);// wait wr data avail
		if( status == 0 ){
			
			err_cnt++;
			goto ddc_read_fail;
			// break;
		}

		status = hdmi_DDC_check_status(HDMI_STATUS_SW_READ, 0);	// wait sw read not set
		if( status == 0 ){
			
			if( i == 0 ){
				err_cnt++;
//				goto ddc_read_fail;
			}
			else {
				g_vpp.dbg_hdmi_ddc_read_err++;
			}
			goto ddc_read_fail;
			// break;
		}

        *buf++ = vppif_reg32_read(HDMI_RD_DATA);
		udelay(HDMI_DDC_DELAY);
#ifdef HDMI_DDC_OUT
		vppif_reg32_out(REG_HDMI_I2C_CTRL2,0x2000000);
#else
		vppif_reg32_write(HDMI_SW_READ,1);
#endif
		udelay(HDMI_DDC_DELAY);
    }

    // STOP
//	vppif_reg32_write(HDMI_SW_STOP_REQ,1);					// sw stop
//	vppif_reg32_write(HDMI_WR_DATA_AVAIL,1);				// write data avail
	vppif_reg32_write(REG_HDMI_I2C_CTRL2,BIT18+BIT17,17,3);	// sw stop, write data avail
	udelay(HDMI_DDC_CTRL_DELAY);
	status = hdmi_DDC_check_status(HDMI_STATUS_STOP+HDMI_STATUS_WR_AVAIL+HDMI_STATUS_CP_USE, 1);	// wait start & wr data avail 
	if( status == 0 ){
		
		err_cnt++;
		goto ddc_read_fail;
	}
	udelay(HDMI_DDC_DELAY);

ddc_read_fail:
	if( err_cnt ){
		
		g_vpp.dbg_hdmi_ddc_ctrl_err++;
	}
	else {
		
	}
    return (err_cnt)? 1:0;
}

int hdmi_DDC_read(char addr,int index,char *buf,int length)
{
	int retry = 3;
	do {
		if( hdmi_DDC_read_func(addr,index,buf,length) == 0 ){
			break;
		}
		hdmi_DDC_reset();
		
		retry--;
	} while(retry);
	return 0;
}

void hdmi_audio_enable(vpp_flag_t enable)
{
	vppif_reg32_write(HDMI_AUD_ENABLE,enable);
}

void hdmi_audio_mute(vpp_flag_t enable)
{
	vppif_reg32_write(HDMI_AUD_MUTE,enable);
}

/*----------------------- HDMI API --------------------------------------*/
void hdmi_write_packet(unsigned int header,unsigned char *packet,int cnt)
{
	unsigned char buf[36];
	int i;
	hdmi_fifo_slot_t no;

#ifdef CONFIG_HDMI_INFOFRAME_DISABLE
	return;
#endif
	memcpy(&buf[0],&header,3);
	buf[3] = hdmi_ecc((unsigned char *)&header,24);
	for( i=0; i<cnt/7; i++ ){
		memcpy(&buf[4+8*i],&packet[7*i],7);
		buf[11+8*i] = hdmi_ecc(&packet[7*i],56);
	}

	switch(header & 0xFF){
		case HDMI_PACKET_INFOFRAME_AVI:
			no = HDMI_FIFO_SLOT_AVI;
			break;
		case HDMI_PACKET_INFOFRAME_AUDIO:
			no = HDMI_FIFO_SLOT_AUDIO;
			break;
		default:
			no = HDMI_FIFO_SLOT_CONTROL;
			break;
	}
	hdmi_write_fifo(no,(unsigned int *)buf,(4+8*(cnt/7)));
}

void hdmi_tx_null_packet(void)
{
	hdmi_write_packet(HDMI_PACKET_NULL,0,0);
}

void hdmi_tx_general_control_packet(int mute)
{
	unsigned char buf[7];
	memset(buf,0x0,7);
	buf[0] = (mute)? 0x01:0x10;
	buf[1] = HDMI_COLOR_DEPTH_24 | ( HDMI_PHASE_4 << 4);
	hdmi_write_packet(HDMI_PACKET_GENERAL_CTRL,buf,7);
}

void hdmi_tx_avi_infoframe_packet(vdo_color_fmt colfmt,hdmi_video_code_t vic)
{
	unsigned int header;
	unsigned char buf[28];
	unsigned char temp;

	memset(buf,0x0,28);
	header = HDMI_PACKET_INFOFRAME_AVI + (0x2 << 8) + (0x0d << 16);
	buf[1] = HDMI_SI_NO_DATA + (HDMI_BI_V_H_VALID << 2) + (HDMI_AF_INFO_NO_DATA << 4);
	switch( colfmt ){
		case VDO_COL_FMT_YUV422H:
		case VDO_COL_FMT_YUV422V:
			temp = HDMI_OUTPUT_YUV422;
			break;
		case VDO_COL_FMT_YUV444:
			temp = HDMI_OUTPUT_YUV444;
			break;
		case VDO_COL_FMT_ARGB:
		default:
			temp = HDMI_OUTPUT_RGB;
			break;
	}
	buf[1] += (temp << 5);
	buf[2] = HDMI_ASPECT_RATIO_PIC + (HDMI_PIC_ASPECT_16_9 << 4) + (HDMI_COLORIMETRY_ITU709 << 6);
	buf[3] = 0x84;	
	buf[4] = vic;
	switch( vic ){
		case HDMI_1440x480i60_16x9:
		case HDMI_1440x576i50_16x9:
			buf[5] = HDMI_PIXEL_REP_2;
			break;
		default:
			buf[5] = HDMI_PIXEL_REP_NO;
			break;
	}
	buf[0] = hdmi_checksum((unsigned char *)&header,buf,28);
	hdmi_write_packet(header,buf,28);
}

void hdmi_tx_audio_infoframe_packet(int channel,int freq)
{
	unsigned int header;
	unsigned char buf[28];

	memset(buf,0x0,28);
	header = HDMI_PACKET_INFOFRAME_AUDIO + (0x1 << 8) + (0x0a << 16);
	buf[1] = (channel-1) + (HDMI_AUD_TYPE_REF_STM << 4);
	buf[2] = 0x0;	// HDMI_AUD_SAMPLE_24 + (freq << 2);
	buf[3] = 0x00;
	buf[4] = 0x0;
	buf[5] = 0x0;	// 0 db
	buf[0] = hdmi_checksum((unsigned char *)&header,buf,28);
	hdmi_write_packet(header,buf,28);
}

unsigned int hdmi_pixel_clock;
void hdmi_set_audio_n_cts(unsigned int freq)
{
	unsigned int n,cts;

	n = 128 * freq / 1000;
#if 1
	cts = hdmi_pixel_clock / 1000;
#else
	cts = vpp_get_base_clock(VPP_MOD_GOVRH) / 1000;
#endif
	vppif_reg32_write(HDMI_AUD_N_20BITS,n);
	vppif_reg32_write(HDMI_AUD_ACR_RATIO,cts-2);	
#if 1	// auto detect CTS
	vppif_reg32_write(HDMI_AUD_CTS_SELECT,0);
	cts = 0;
#else
	vppif_reg32_write(HDMI_AUD_CTS_SELECT,1);
#endif
	vppif_reg32_write(HDMI_AUD_CTS_LOW_12BITS,cts & 0xFFF);
	vppif_reg32_write(HDMI_AUD_CTS_HI_8BITS,(cts & 0xFF000)>>12);

	
}
void hdmi_config_audio(vout_audio_t *info)
{
	unsigned int freq;

	g_vpp.hdmi_audio_channel = info->channel;
	g_vpp.hdmi_audio_freq = info->sample_rate;

	hdmi_tx_audio_infoframe_packet(info->channel-1,info->sample_rate);
	hdmi_audio_enable(VPP_FLAG_DISABLE);
	vppif_reg32_write(HDMI_AUD_LAYOUT,(info->channel==8)? 1:0);
	vppif_reg32_write(HDMI_AUD_2CH_ECO,1);

	switch(info->sample_rate){
		case 32000: freq = 0x3; break;
		case 44100: freq = 0x0; break;
		case 88200: freq = 0x8; break;
		case 176400: freq = 0xC; break;
		default:
		case 48000: freq = 0x2; break;
		case 96000: freq = 0xA; break;
		case 192000: freq = 0xE; break;
		case 768000: freq = 0x9; break;
	}
	vppif_reg32_out(REG_HDMI_AUD_CHAN_STATUS0,(freq << 24) + 0x4);
	vppif_reg32_out(REG_HDMI_AUD_CHAN_STATUS1,0x0);
	vppif_reg32_out(REG_HDMI_AUD_CHAN_STATUS2,0xb);
	vppif_reg32_out(REG_HDMI_AUD_CHAN_STATUS3,0x0);
	vppif_reg32_out(REG_HDMI_AUD_CHAN_STATUS4,0x0);
	vppif_reg32_out(REG_HDMI_AUD_CHAN_STATUS5,0x0);

	hdmi_set_audio_n_cts(info->sample_rate);
	vppif_reg32_write(HDMI_AUD_ACR_ENABLE,VPP_FLAG_ENABLE);	// 
	vppif_reg32_write(HDMI_AUD_AIPCLK_RATE,0);
	vppif_reg32_out(AUDREGF_BASE_ADDR+0x18c,0x76543210);
	hdmi_audio_enable(VPP_FLAG_ENABLE);
}

void hdmi_config_video(hdmi_info_t *info)
{
	hdmi_set_output_colfmt(info->outfmt);
	hdmi_tx_avi_infoframe_packet(info->outfmt,info->vic);
}

void hdmi_set_option(unsigned int option)
{
	vdo_color_fmt colfmt;
	int temp;

	hdmi_set_dvi_enable((option & EDID_OPT_HDMI)? VPP_FLAG_DISABLE:VPP_FLAG_ENABLE);
	hdmi_audio_enable((option & EDID_OPT_AUDIO)? VPP_FLAG_ENABLE:VPP_FLAG_DISABLE);
	
	colfmt = hdmi_get_output_colfmt();
	switch( colfmt ){
		case VDO_COL_FMT_YUV422H:
			temp = option & EDID_OPT_YUV422;
			break;
		case VDO_COL_FMT_YUV444:
			temp = option & EDID_OPT_YUV444;
			break;
		default:
			temp = 1;
			break;
	}
	if( temp == 0 ){
		hdmi_set_output_colfmt(VDO_COL_FMT_ARGB);
		
	}
	
}

void hdmi_config(hdmi_info_t *info)
{
	vout_audio_t audio_info;
	int h_porch;
	int delay_cfg;

	vppif_reg32_write(HDMI_CP_ENABLE,0);
	vppif_reg32_write(HDMI_INFOFRAME_SELECT,0);
	vppif_reg32_write(HDMI_INFOFRAME_FIFO1_RDY,0);
	hdmi_config_video(info);

	h_porch = vppif_reg32_read(GOVRH_H_ALLPXL) - vppif_reg32_read(GOVRH_ACTPX_END); // fp
	delay_cfg = 48 - h_porch;
	if( delay_cfg <= 0 ) delay_cfg = 1;
	h_porch = vppif_reg32_read(GOVRH_ACTPX_BG);	// bp
	h_porch = (h_porch - (delay_cfg+1) - 26) / 32;
	if( h_porch <= 0 ) h_porch = 1;
	if( h_porch >= 8 ) h_porch = 0;
	hdmi_reg32_write(HDMI_CP_DELAY,delay_cfg);
	vppif_reg32_write(HDMI_HORIZ_BLANK_MAX_PCK,h_porch);
	

	audio_info.fmt = 16;
	audio_info.channel = info->channel;
	audio_info.sample_rate = info->freq;
	hdmi_config_audio(&audio_info);

	vppif_reg32_write(HDMI_INFOFRAME_FIFO1_ADDR,0);
	vppif_reg32_write(HDMI_INFOFRAME_FIFO1_LEN,1);
	vppif_reg32_write(HDMI_INFOFRAME_FIFO1_RDY,1);

	hdmi_set_option(info->option);
}

/*----------------------- Module API --------------------------------------*/
void hdmi_set_cp_enable(vpp_flag_t enable)
{
	if( !g_vpp.hdmi_cp_enable ){
		enable = 0;
	}

	if( hdmi_cp )
		hdmi_cp->enable(enable);
}

int hdmi_check_cp_int(void)
{
	unsigned int reg;
	int ret = 0;

	reg = vppif_reg32_in(REG_HDMI_CP_INT);
	if( reg & BIT25 ){
		ret = 2;

	}
	
	if( reg & BIT26 ){
		
		ret = 1;
	}
	
	if( reg & BIT27 ){
		
		ret = 1;
	}
	
	if( reg & BIT28 ){
		int sts;

		sts = (reg & 0xE0000000) >> 29;
		ret = 1;
		switch( sts ){
			case 0x0:
				
				break;
			case 0x1:
				
				break;
			case 0x2:
				
				break;
			case 0x3:
				
				break;
			case 0x4:
				
				break;
			case 0x5:
				
				break;
			case 0x6:
				
				break;
			default:
			case 0x7:
				
				ret = 0;
				break;
		}
	}
	vppif_reg32_out(REG_HDMI_CP_INT,reg);	// clear int status
	return ret;
}

void hdmi_get_bksv(unsigned int *bksv)
{
	vppif_reg32_write(HDMI_KSV_LIST_AVAIL,1);
	bksv[0] = vppif_reg32_in(REG_HDMI_CP_BKSV1);
	bksv[1] = vppif_reg32_in(REG_HDMI_CP_BKSV2) & 0xFF;
	vppif_reg32_write(HDMI_KSV_LIST_AVAIL,0);
}

int hdmi_check_cp_dev_cnt(void)
{
	int ret = 0;

	if( vppif_reg32_read(HDMI_CP_ENABLE) ){
		if( vppif_reg32_read(HDMI_CP_REPEATER) ){
			if( vppif_reg32_read(HDMI_CP_BSTATUS_HDMI_MODE) ){	// check BStatus ready
				if( vppif_reg32_read(HDMI_CP_DEV_CNT) == 0 ){
					ret = 1;
				}
			}
		}
	}
	return ret;
}

int hdmi_check_plugin(int hotplug)
{
#ifdef CFG_LOADER
	//fan
	return 1;
#else
	int plugin;
	int option = 0;

	plugin = hdmi_get_plugin();
	hdmi_clear_plug_status();
	if( hotplug ){
		hdmi_set_enable(plugin);
		if( plugin ){
#ifdef __KERNEL__
			unsigned char *buf;
			buf = vout_get_edid(VOUT_HDMI);
			option = edid_parse_option(buf);
#else
			option = 0xFFFFFFFF;
#endif
#ifdef CONFIG_VPP_DEMO
			option |= (EDID_OPT_HDMI + EDID_OPT_AUDIO);
#endif
		}
		hdmi_set_option(option);
		hdmi_set_cp_enable((g_vpp.hdmi_cp_enable && plugin)? VPP_FLAG_ENABLE:VPP_FLAG_DISABLE);
#ifdef CONFIG_WMT_CEC
		wmt_cec_hotplug_notify(plugin);
#endif
	}
	
	return plugin;
#endif
}

void hdmi_reg_dump(void)
{

}

#ifdef CONFIG_PM
static unsigned int *hdmi_pm_bk;
static unsigned int hdmi_pm_enable;
static unsigned int hdmi_cp_pm_enable;
void hdmi_suspend(int sts)
{
	switch( sts ){
		case 0:	// disable module
			hdmi_pm_enable = vppif_reg32_read(HDMI_ENABLE);
			hdmi_reg32_write(HDMI_ENABLE,0);
			hdmi_cp_pm_enable = vppif_reg32_read(HDMI_CP_ENABLE);
			vppif_reg32_write(HDMI_CP_ENABLE,0);
			break;
		case 1: // disable tg
			break;
		case 2:	// backup register
			hdmi_pm_bk = vpp_backup_reg(REG_HDMI_BEGIN,(REG_HDMI_END-REG_HDMI_BEGIN));
			break;
		default:
			break;
	}
}

void hdmi_resume(int sts)
{
	switch( sts ){
		case 0:	// restore register
			vpp_restore_reg(REG_HDMI_BEGIN,(REG_HDMI_END-REG_HDMI_BEGIN),hdmi_pm_bk);
			hdmi_pm_bk = 0;
			break;
		case 1:	// enable module
			hdmi_reg32_write(HDMI_ENABLE,hdmi_pm_enable);
			vppif_reg32_write(HDMI_CP_ENABLE,hdmi_cp_pm_enable);
			break;
		case 2: // enable tg
			break;
		default:
			break;
	}
}
#else
#define hdmi_suspend NULL
#define hdmi_resume NULL
#endif

void hdmi_set_cypher(int func)
{
	if( func == 0 )
		return;
	
	g_vpp.cypher_func = (void *) func;
	if( g_vpp.hdmi_init ){
		if( hdmi_cp )
			hdmi_cp->init();
	}
}
EXPORT_SYMBOL(hdmi_set_cypher);

void hdmi_init(void)
{
	hdmi_pixel_clock = vpp_get_base_clock(VPP_MOD_GOVRH);
	g_vpp.hdmi_i2c_freq = HDMI_I2C_FREQ;
	g_vpp.hdmi_i2c_udelay = 0;
	g_vpp.hdmi_audio_channel = 2;
	g_vpp.hdmi_audio_freq = 48000;
	g_vpp.hdmi_ctrl = 0x1000000;
	g_vpp.hdmi_audio_pb4 = 0x0;
	g_vpp.hdmi_audio_pb1 = 0x0;

	if( (g_vpp.govrh_preinit) && (vppif_reg32_read(HDMI_ENABLE))){
		
	}
	else {
		vppif_reg32_write(LVDS_VBG_SEL,2);
		vppif_reg32_write(LVDS_DRV_PDMODE,1);
		hdmi_set_enable(VPP_FLAG_DISABLE);
		hdmi_set_dvi_enable(VPP_FLAG_DISABLE);
		vppif_reg32_write(HDMI_CIPHER_1_1,0);

		vppif_reg32_write(HDMI_INFOFRAME_SRAM_ENABLE,1);
		vppif_reg32_write(HDMI_INFOFRAME_SELECT,0);
		vppif_reg32_write(HDMI_INFOFRAME_FIFO1_RDY,0);

		vppif_reg32_out(HDMI_BASE_ADDR+0x3ec,0x0);
		vppif_reg32_out(HDMI_BASE_ADDR+0x3e8,0x0);

	//	vppif_reg32_write(HDMI_AUD_LAYOUT,1);
		hdmi_DDC_reset();
		hdmi_DDC_set_freq(g_vpp.hdmi_i2c_freq);
		vppif_reg32_write(HDMI_I2C_ENABLE,1);
	}
	g_vpp.hdmi_init = 1;
	if( g_vpp.cypher_func ){
		if( hdmi_cp ) 
			hdmi_cp->init();		
	}
}
#endif /* WMT_FTBLK_HDMI */

