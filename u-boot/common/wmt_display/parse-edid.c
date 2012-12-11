/*++ 
 * linux/drivers/video/wmt/parse-edid.c
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
 
#define PARSE_EDID_C
// #define DEBUG

#include "vpp.h"
#include "edid.h"
#include "hdmi.h"

const unsigned char edid_v1_header[] = { 
    0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00 };

const unsigned char edid_v1_descriptor_flag[] = { 0x00, 0x00 };

#define COMBINE_HI_8LO( hi, lo ) \
        ( (((unsigned)hi) << 8) | (unsigned)lo )

#define COMBINE_HI_4LO( hi, lo ) \
        ( (((unsigned)hi) << 4) | (unsigned)lo )

#define UPPER_NIBBLE( x ) \
        (((128|64|32|16) & (x)) >> 4)

#define LOWER_NIBBLE( x ) \
        ((1|2|4|8) & (x))

#define MONITOR_NAME            0xfc
#define MONITOR_LIMITS          0xfd
#define UNKNOWN_DESCRIPTOR      -1
#define DETAILED_TIMING_BLOCK   -2

edid_timing_t edid_establish_timing[] = {
	{ 800, 600, 60 }, { 800, 600, 56 }, { 640, 480, 75 }, { 640, 480, 72 }, { 640, 480, 67 }, { 640, 480, 60 }, 
	{ 720, 400, 88 }, { 720, 400, 70 }, { 1280, 1024, 75 }, { 1024, 768, 75 }, { 1024, 768, 70 }, { 1024, 768, 60 }, 
	{ 1024, 768, 87 }, { 832, 624, 75 }, { 800, 600, 75 }, { 800, 600, 72 }, { 1152, 870, 75 }
};
edid_info_t edid_info;

static int block_type( unsigned char * block )
{
    if ( !memcmp( edid_v1_descriptor_flag, block, 2 ) ) {
//        DBGMSG("# Block type: 2:%x 3:%x\n", block[2], block[3]);

        /* descriptor */
        if ( block[ 2 ] != 0 )
	        return UNKNOWN_DESCRIPTOR;
        return block[ 3 ];
    } 
    /* detailed timing block */
    return DETAILED_TIMING_BLOCK;
} /* End of block_type() */

static char * get_vendor_sign( unsigned char * block, char *sign)
{
//    static char sign[4];
    unsigned short h;

  /*
     08h	WORD	big-endian manufacturer ID (see #00136)
		    bits 14-10: first letter (01h='A', 02h='B', etc.)
		    bits 9-5: second letter
		    bits 4-0: third letter
  */
    h = COMBINE_HI_8LO(block[0], block[1]);
    sign[0] = ((h>>10) & 0x1f) + 'A' - 1;
    sign[1] = ((h>>5) & 0x1f) + 'A' - 1;
    sign[2] = (h & 0x1f) + 'A' - 1;
    sign[3] = 0;
    
    return sign;
} /* End of get_vendor_sign() */

static char * get_monitor_name( unsigned char * block )
{
    #define DESCRIPTOR_DATA         5

    unsigned char *ptr = block + DESCRIPTOR_DATA;
    static char name[ 13 ];
    unsigned i;


    for( i = 0; i < 13; i++, ptr++ ) {
        if ( *ptr == 0xa ) {
	        name[ i ] = 0;
	        return name;
	    }
        name[ i ] = *ptr;
    }
    return name;
} /* End of get_monitor_name() */

static int parse_timing_description( unsigned char* dtd )
{
    #define PIXEL_CLOCK_LO     (unsigned)dtd[ 0 ]
    #define PIXEL_CLOCK_HI     (unsigned)dtd[ 1 ]
    #define PIXEL_CLOCK        (COMBINE_HI_8LO( PIXEL_CLOCK_HI,PIXEL_CLOCK_LO )*10000)
    #define H_ACTIVE_LO        (unsigned)dtd[ 2 ]
    #define H_BLANKING_LO      (unsigned)dtd[ 3 ]
    #define H_ACTIVE_HI        UPPER_NIBBLE( (unsigned)dtd[ 4 ] )
    #define H_ACTIVE           COMBINE_HI_8LO( H_ACTIVE_HI, H_ACTIVE_LO )
    #define H_BLANKING_HI      LOWER_NIBBLE( (unsigned)dtd[ 4 ] )
    #define H_BLANKING         COMBINE_HI_8LO( H_BLANKING_HI, H_BLANKING_LO )
    #define V_ACTIVE_LO        (unsigned)dtd[ 5 ]
    #define V_BLANKING_LO      (unsigned)dtd[ 6 ]
    #define V_ACTIVE_HI        UPPER_NIBBLE( (unsigned)dtd[ 7 ] )
    #define V_ACTIVE           COMBINE_HI_8LO( V_ACTIVE_HI, V_ACTIVE_LO )
    #define V_BLANKING_HI      LOWER_NIBBLE( (unsigned)dtd[ 7 ] )
    #define V_BLANKING         COMBINE_HI_8LO( V_BLANKING_HI, V_BLANKING_LO )
    #define H_SYNC_OFFSET_LO   (unsigned)dtd[ 8 ]
    #define H_SYNC_WIDTH_LO    (unsigned)dtd[ 9 ]
    #define V_SYNC_OFFSET_LO   UPPER_NIBBLE( (unsigned)dtd[ 10 ] )
    #define V_SYNC_WIDTH_LO    LOWER_NIBBLE( (unsigned)dtd[ 10 ] )
    #define V_SYNC_WIDTH_HI    ((unsigned)dtd[ 11 ] & (1|2))
    #define V_SYNC_OFFSET_HI   (((unsigned)dtd[ 11 ] & (4|8)) >> 2)
    #define H_SYNC_WIDTH_HI    (((unsigned)dtd[ 11 ] & (16|32)) >> 4)
    #define H_SYNC_OFFSET_HI   (((unsigned)dtd[ 11 ] & (64|128)) >> 6)
    #define V_SYNC_WIDTH       COMBINE_HI_4LO( V_SYNC_WIDTH_HI, V_SYNC_WIDTH_LO )
    #define V_SYNC_OFFSET      COMBINE_HI_4LO( V_SYNC_OFFSET_HI, V_SYNC_OFFSET_LO )
    #define H_SYNC_WIDTH       COMBINE_HI_4LO( H_SYNC_WIDTH_HI, H_SYNC_WIDTH_LO )
    #define H_SYNC_OFFSET      COMBINE_HI_4LO( H_SYNC_OFFSET_HI, H_SYNC_OFFSET_LO )
    #define H_SIZE_LO          (unsigned)dtd[ 12 ]
    #define V_SIZE_LO          (unsigned)dtd[ 13 ]
    #define H_SIZE_HI          UPPER_NIBBLE( (unsigned)dtd[ 14 ] )
    #define V_SIZE_HI          LOWER_NIBBLE( (unsigned)dtd[ 14 ] )
    #define H_SIZE             COMBINE_HI_8LO( H_SIZE_HI, H_SIZE_LO )
    #define V_SIZE             COMBINE_HI_8LO( V_SIZE_HI, V_SIZE_LO )
    #define H_BORDER           (unsigned)dtd[ 15 ]
    #define V_BORDER           (unsigned)dtd[ 16 ]
    #define FLAGS              (unsigned)dtd[ 17 ]
    #define INTERLACED         (FLAGS&128)
    #define SYNC_TYPE	   (FLAGS&3<<3)  /* bits 4,3 */
    #define SYNC_SEPARATE	   (3<<3)
    #define HSYNC_POSITIVE	   (FLAGS & 4)
    #define VSYNC_POSITIVE     (FLAGS & 2)

    int htotal, vtotal;
    
    htotal = H_ACTIVE + H_BLANKING;
    vtotal = V_ACTIVE + V_BLANKING;
  
    DBGMSG( "\tMode \"%dx%d\"", H_ACTIVE, V_ACTIVE );
	DBGMSG("\n");
    DBGMSG( "# vfreq %dHz, hfreq %dkHz\n",
	        PIXEL_CLOCK/(vtotal*htotal),
	        PIXEL_CLOCK/(htotal*1000));
    DBGMSG( "\tDotClock\t%d\n", PIXEL_CLOCK/1000000 );
    DBGMSG( "\tHTimings\t%u %u %u %u\n", H_ACTIVE,
	      H_ACTIVE+H_SYNC_OFFSET, 
	      H_ACTIVE+H_SYNC_OFFSET+H_SYNC_WIDTH,
	      htotal );

    DBGMSG( "\tVTimings\t%u %u %u %u\n", V_ACTIVE,
	    V_ACTIVE+V_SYNC_OFFSET,
	    V_ACTIVE+V_SYNC_OFFSET+V_SYNC_WIDTH,
	    vtotal );

    if ( INTERLACED || (SYNC_TYPE == SYNC_SEPARATE)) {
        DBGMSG( "Flags\t%s\"%sHSync\" \"%sVSync\"\n",
	    INTERLACED ? "\"Interlace\" ": "",
	    HSYNC_POSITIVE ? "+": "-",
	    VSYNC_POSITIVE ? "+": "-");
    }

    DBGMSG( "EndMode\n" );

	{
		int i;
		vpp_timing_t *t;
		
		for(i=0;i<4;i++){
			t = &edid_info.detail_timing[i];
			if( t->pixel_clock == 0 ){
				int fps;

				t->pixel_clock = PIXEL_CLOCK;

				t->hfp = H_SYNC_OFFSET;
				t->hsync = H_SYNC_WIDTH;
				t->hpixel = H_ACTIVE;
				t->hbp = H_BLANKING - (H_SYNC_WIDTH+H_SYNC_OFFSET);

				t->vfp = V_SYNC_OFFSET;
				t->vsync = V_SYNC_WIDTH;
				t->vpixel = V_ACTIVE;
				t->vbp = V_BLANKING - (V_SYNC_WIDTH+V_SYNC_OFFSET);

				fps = PIXEL_CLOCK/(vtotal*htotal);
				if( fps == 59 ) 
					fps = 60;
				t->option = VPP_SET_OPT_FPS(t->option,fps);
				t->option |= INTERLACED? VPP_OPT_INTERLACE:0;
				t->option |= HSYNC_POSITIVE? (VPP_DVO_SYNC_POLAR_HI+VPP_VGA_HSYNC_POLAR_HI):0;
				t->option |= VSYNC_POSITIVE? (VPP_DVO_VSYNC_POLAR_HI+VPP_VGA_VSYNC_POLAR_HI):0;

				if( vout_check_ratio_16_9(H_ACTIVE,V_ACTIVE) ){
					edid_info.option |= EDID_OPT_16_9;
				}
				break;
			}
		}
	}
    return 0;
} /* End of parse_timing_description() */

static int parse_dpms_capabilities(unsigned char flags)
{
    #define DPMS_ACTIVE_OFF		(1 << 5)
    #define DPMS_SUSPEND		(1 << 6)
    #define DPMS_STANDBY		(1 << 7)

    DBGMSG("# DPMS capabilities: Active off:%s  Suspend:%s  Standby:%s\n\n",
            (flags & DPMS_ACTIVE_OFF) ? "yes" : "no",
            (flags & DPMS_SUSPEND)    ? "yes" : "no",
            (flags & DPMS_STANDBY)    ? "yes" : "no");
    return 0;
} /* End of parse_dpms_capabilities() */

static int parse_monitor_limits( unsigned char * block )
{
    #define V_MIN_RATE              block[ 5 ]
    #define V_MAX_RATE              block[ 6 ]
    #define H_MIN_RATE              block[ 7 ]
    #define H_MAX_RATE              block[ 8 ]
    #define MAX_PIXEL_CLOCK         (((int)block[ 9 ]) * 10)
    #define GTF_SUPPORT             block[10]

    DBGMSG( "\tHorizontal Frequency: %u-%u Hz\n", H_MIN_RATE, H_MAX_RATE );
    DBGMSG( "\tVertical   Frequency: %u-%u kHz\n", V_MIN_RATE, V_MAX_RATE );
    if ( MAX_PIXEL_CLOCK == 10*0xff ){
        DBGMSG( "\t# Max dot clock not given\n" );
    }
    else {
        DBGMSG( "\t# Max dot clock (video bandwidth) %u MHz\n", (int)MAX_PIXEL_CLOCK );
		edid_info.pixel_clock_limit = MAX_PIXEL_CLOCK;
    }

    if ( GTF_SUPPORT ) {
        DBGMSG( "\t# EDID version 3 GTF given: contact author\n" );
    }
    return 0;
} /* End of parse_monitor_limits() */

static int get_established_timing( unsigned char * edid )
{
    unsigned char time_1, time_2;
    
    time_1 = edid[ESTABLISHED_TIMING_I];
    time_2 = edid[ESTABLISHED_TIMING_II];
	edid_info.establish_timing = time_1 + (time_2 << 8);

    /*--------------------------------------------------------------------------
        35: ESTABLISHED TIMING I
            bit 7-0: 720กั400@70 Hz, 720กั400@88 Hz, 640กั480@60 Hz, 640กั480@67 Hz,
                     640กั480@72 Hz, 640กั480@75 Hz, 800กั600@56 Hz, 800กั600@60 Hz
    --------------------------------------------------------------------------*/
    DBGMSG("Established Timimgs I:  0x%x\n", time_1);
    if( time_1 & 0x80 )
        DBGMSG("     \t%- dx%d@%dHz\n", 720, 400, 70);
    if( time_1 & 0x40 )
        DBGMSG("     \t%- dx%d@%dHz\n", 720, 400, 88);
    if( time_1 & 0x20 )
        DBGMSG("     \t%- dx%d@%dHz\n", 640, 480, 60);
    if( time_1 & 0x10 )
        DBGMSG("     \t%- dx%d@%dHz\n", 640, 480, 67);
    if( time_1 & 0x08 )
        DBGMSG("     \t%- dx%d@%dHz\n", 640, 480, 72);
    if( time_1 & 0x04 )
        DBGMSG("     \t%- dx%d@%dHz\n", 640, 480, 75);
    if( time_1 & 0x02 )
        DBGMSG("     \t%- dx%d@%dHz\n", 800, 600, 56);
    if( time_1 & 0x01 )
        DBGMSG("     \t%- dx%d@%dHz\n", 800, 600, 60);

    /*--------------------------------------------------------------------------
        36: ESTABLISHED TIMING II
            bit 7-0: 800กั600@72 Hz, 800กั600@75 Hz, 832กั624@75 Hz, 1024กั768@87 Hz (Interlaced),
                     1024กั768@60 Hz, 1024กั768@70 Hz, 1024กั768@75 Hz, 1280กั1024@75 Hz
    --------------------------------------------------------------------------*/
    DBGMSG("Established Timimgs II: 0x%x\n", time_2);
    if( time_2 & 0x80 )
        DBGMSG("     \t%- dx%d@%dHz\n", 800, 600, 72);
    if( time_2 & 0x40 )
        DBGMSG("     \t%- dx%d@%dHz\n", 800, 600, 75);
    if( time_2 & 0x20 )
        DBGMSG("     \t%- dx%d@%dHz\n", 832, 624, 75);
    if( time_2 & 0x10 )
        DBGMSG("     \t%- dx%d@%dHz (Interlace)\n", 1024, 768, 87);
    if( time_2 & 0x08 )
        DBGMSG("     \t%- dx%d@%dHz\n", 1024, 768, 60);
    if( time_2 & 0x04 )
        DBGMSG("     \t%- dx%d@%dHz\n", 1024, 768, 70);
    if( time_2 & 0x02 )
        DBGMSG("     \t%- dx%d@%dHz\n", 1024, 768, 75);
    if( time_2 & 0x01 )
        DBGMSG("     \t%- dx%d@%dHz\n", 1280, 1024, 75);
    
    return 0;
} /* End of get_established_timing() */

static int get_standard_timing( unsigned char * edid )
{
    unsigned char *ptr = edid +STANDARD_TIMING_IDENTIFICATION_START;
    int h_res, v_res, v_freq;
    int byte_1, byte_2, aspect, i;

    /*--------------------------------------------------------------------------
        First byte
            Horizontal resolution.  Multiply by 8, then add 248 for actual value.
        Second byte
            bit 7-6: Aspect ratio. Actual vertical resolution depends on horizontal 
            resolution.
            00=16:10, 01=4:3, 10=5:4, 11=16:9 (00=1:1 prior to v1.3)
            bit 5-0: Vertical frequency. Add 60 to get actual value.
    --------------------------------------------------------------------------*/  
    DBGMSG("Standard Timing Identification \n");
    for(i=0; i< STANDARD_TIMING_IDENTIFICATION_SIZE/2; i++ ) {
        byte_1 = *ptr++;
        byte_2 = *ptr++;
        if( (byte_1 == 0x01) && (byte_2 == 0x01) )
            break;
        h_res = (byte_1 * 8) + 248;
        aspect = byte_2 & 0xC0;
        switch(aspect) {
			default:
            case 0x00:
                v_res = h_res * 10/16;
                break;
            case 0x40:
                v_res = h_res * 3/4;
                break;
            case 0x80:
                v_res = h_res * 4/5;
                break;
            case 0xC0:
                v_res = h_res * 9/16;
                break;
        }
        v_freq = (byte_2 & 0x1F) + 60;
        DBGMSG("Standard Timing: \t%dx%d@%dHz\n", h_res, v_res, v_freq);
		edid_info.standard_timing[i].resx = h_res;
		edid_info.standard_timing[i].resy = v_res;
		edid_info.standard_timing[i].freq = v_freq;
    }    
    return 0;
} /* End of get_standard_timing() */

void edid_dump(unsigned char *edid)
{
	int i;
	
	DPRINT("===================== EDID BlOCK =====================");
	for(i=0;i<128;i++){
		if( (i%16)==0 ) DPRINT("\n");
		DPRINT("%02x ",edid[i]);
	}
	DPRINT("\n");
	DPRINT("======================================================\n");
}

int edid_checksum(unsigned char *edid,int len)
{
	unsigned char checksum = 0;
	int i;
	
    for( i = 0; i < len; i++ )
        checksum += edid[ i ];

	if( checksum ){
		// edid_dump(edid);
	}
	return checksum;
}

int edid_parse_v1( unsigned char * edid )
{
    unsigned char * block;
    unsigned char checksum = 0;
    char *monitor_name = 0;
    char  monitor_alt_name[100];
    char  vendor_sign[4];
    int   i, ret = 0;

    for( i = 0; i < EDID_LENGTH; i++ )
        checksum += edid[ i ];

    if ( checksum != 0 ) {
        DPRINT("*E* EDID checksum failed - data is corrupt\n" );
        ret = -1;
		goto parse_end;
    }  

    if ( memcmp( edid+EDID_HEADER, edid_v1_header, EDID_HEADER_END+1 ) ) {
        DBGMSG("*E* first bytes don't match EDID version 1 header\n");
        ret = -1;
		goto parse_end;
    }

#ifdef DEBUG
	edid_dump(edid);
#endif
	
    DBGMSG("[EDID] EDID version:  %d.%d\n", (int)edid[EDID_STRUCT_VERSION],(int)edid[EDID_STRUCT_REVISION] );

    get_vendor_sign( edid + ID_MANUFACTURER_NAME,(char *) &vendor_sign ); 

    /*--------------------------------------------------------------------------
        Parse Monitor name
    --------------------------------------------------------------------------*/
    block = edid + DETAILED_TIMING_DESCRIPTIONS_START;
    for( i = 0; i < NO_DETAILED_TIMING_DESCRIPTIONS; i++,
	     block += DETAILED_TIMING_DESCRIPTION_SIZE ) {
        if ( block_type( block ) == MONITOR_NAME ) {
	        monitor_name = get_monitor_name( block );
	        break;
	    }
    }

    if (!monitor_name) {
        /* Stupid djgpp hasn't snDBGMSG so we have to hack something together */
        if(strlen(vendor_sign) + 10 > sizeof(monitor_alt_name))
            vendor_sign[3] = 0;
    
        sprintf(monitor_alt_name, "%s:%02x%02x",
	            vendor_sign, edid[ID_MODEL], edid[ID_MODEL+1]) ;
        monitor_name = monitor_alt_name;
    }

    DBGMSG( "Identifier \"%s\"\n", monitor_name );
    DBGMSG( "VendorName \"%s\"\n", vendor_sign );
    DBGMSG( "ModelName  \"%s\"\n",  monitor_name );

    parse_dpms_capabilities(edid[DPMS_FLAGS]);

    /*--------------------------------------------------------------------------
        Parse ESTABLISHED TIMING I and II
    --------------------------------------------------------------------------*/
    get_established_timing( edid );

    /*--------------------------------------------------------------------------
        Parse STANDARD TIMING IDENTIFICATION
    --------------------------------------------------------------------------*/
    get_standard_timing( edid );

    block = edid + DETAILED_TIMING_DESCRIPTIONS_START;
    for( i = 0; i < NO_DETAILED_TIMING_DESCRIPTIONS; i++,
	     block += DETAILED_TIMING_DESCRIPTION_SIZE ) {
        if ( block_type( block ) == MONITOR_LIMITS )
	        parse_monitor_limits( block );
    }

    block = edid + DETAILED_TIMING_DESCRIPTIONS_START;
    for( i = 0; i < NO_DETAILED_TIMING_DESCRIPTIONS; i++,
	     block += DETAILED_TIMING_DESCRIPTION_SIZE ) {
        if ( block_type( block ) == DETAILED_TIMING_BLOCK )
	        parse_timing_description( block );
    }
parse_end:
  	return ret;
}

int edid_parse_CEA(unsigned char *edid)
{
	unsigned char *block;
	unsigned char checksum = 0;
	int i,len;
	vpp_timing_t *p;
	unsigned int fps;
	
	if((edid[0]!=0x2) || (edid[1]!=0x3)){
		return -1;
	}

    for( i = 0; i < EDID_LENGTH; i++ )
        checksum += edid[ i ];

    if ( checksum != 0 ) {
        DPRINT("*E* CEA EDID checksum failed - data is corrupt\n" );
        return -1;
    }  

#ifdef DEBUG
	edid_dump(edid);
#endif
	
    DBGMSG("[EDID] CEA EDID Version %d.%d\n",edid[0],edid[1]);

	edid_info.option = (edid[3] & 0xF0) | EDID_OPT_VALID;
	DBGMSG("\t %s support 422\n", (edid[3] & 0x10)? "":"no" );
	DBGMSG("\t %s support 444\n", (edid[3] & 0x20)? "":"no" );
	DBGMSG("\t %s support audio\n", (edid[3] & 0x40)? "":"no" );
	DBGMSG("\t %s support underscan\n", (edid[3] & 0x80)? "":"no" );

	block = edid + 4;
	do {
		len = block[0] & 0x1F;
		switch(((block[0] & 0xE0)>>5)){
			case 1:	// Audio Data Block
#ifdef DEBUG			
				DBGMSG("Audio Data Block\n");
				switch( ((block[1] & 0x78) >> 3) ){
					default:
					case 0: // reserved
					case 15:// reserved
						DBGMSG("\t Reserved Audio Fmt\n"); break;
					case 1: // LPCM
						DBGMSG("\t Audio Fmt LPCM\n"); break;
					case 2: // AC3
						DBGMSG("\t Audio Fmt AC3\n"); break;
					case 3: // MPEG1
						DBGMSG("\t Audio Fmt MPEG1\n"); break;
					case 4: // MP3
						DBGMSG("\t Audio Fmt MP3\n"); break;
					case 5: // MPEG2
						DBGMSG("\t Audio Fmt MPEG2\n"); break;
					case 6: // AAC
						DBGMSG("\t Audio Fmt AAC\n"); break;
					case 7: // DTS
						DBGMSG("\t Audio Fmt DTS\n"); break;
					case 8: // ATRAC
						DBGMSG("\t Audio Fmt ATRAC\n"); break;
					case 9: // One bit audio
						DBGMSG("\t Audio Fmt ONE BIT AUDIO\n"); break;
					case 10:// Dolby
						DBGMSG("\t Audio Fmt DOLBY\n"); break;
					case 11:// DTS-HD
						DBGMSG("\t Audio Fmt DTS-HD\n"); break;
					case 12:// MAT (MLP)
						DBGMSG("\t Audio Fmt MAT\n"); break;
					case 13:// DST
						DBGMSG("\t Audio Fmt DST\n"); break;
					case 14:// WMA Pro
						DBGMSG("\t Audio Fmt WMA+\n"); break;
				}
				
				DBGMSG("\t Max channel %d\n", (block[1] & 0x7)+1 );				
				DBGMSG("\t %s support 32 KHz\n", (block[2] & 0x1)? "":"no" );
				DBGMSG("\t %s support 44 KHz\n", (block[2] & 0x2)? "":"no" );
				DBGMSG("\t %s support 48 KHz\n", (block[2] & 0x4)? "":"no" );
				DBGMSG("\t %s support 88 KHz\n", (block[2] & 0x8)? "":"no" );
				DBGMSG("\t %s support 96 KHz\n", (block[2] & 0x10)? "":"no" );
				DBGMSG("\t %s support 176 KHz\n", (block[2] & 0x20)? "":"no" );
				DBGMSG("\t %s support 192 KHz\n", (block[2] & 0x40)? "":"no" );
				DBGMSG("\t %s support 16 bit\n", (block[3] & 0x1)? "":"no" );
				DBGMSG("\t %s support 20 bit\n", (block[3] & 0x2)? "":"no" );
				DBGMSG("\t %s support 24 bit\n", (block[3] & 0x4)? "":"no" );
#endif
				break;
			case 2:	// Video Data Block
				DBGMSG("Video Data Block\n");
				for(i=0;i<len;i++){
					unsigned int vic;

					DBGMSG("\t support VIC %d\n",block[1+i]);
					vic = block[1+i] & 0x7F;
					edid_info.cea_vic[vic/8] |= (0x1 << (vic%8));
				}
				break;
			case 3: // Vendor Spec Data Block
				DBGMSG("Vendor Spec Data Block\n");
				if( len < 5 ) break; // min size
				if( (block[1]==0x03) && (block[2]==0x0C) && (block[3]==0x0)){	// IEEE Registration Identifier 0x000C03
					edid_info.option |= EDID_OPT_HDMI;
					DBGMSG("\t support HDMI\n");
				}
				DBGMSG("\t Source Physical Addr %d.%d.%d.%d\n",(block[4]&0xF0)>>4,block[4]&0x0F,(block[5]&0xF0)>>4,block[5]&0x0F);
				edid_info.hdmi_phy_addr = (block[4] << 8) + block[5];
				if( len < 6 ) break;
				DBGMSG("\t %s support AI\n", (block[6] & 0x80)? "":"no" );
				DBGMSG("\t %s support 30 bits/pixel(10 bits/color)\n", (block[6] & 0x40)? "":"no" );
				DBGMSG("\t %s support 36 bits/pixel(12 bits/color)\n", (block[6] & 0x20)? "":"no" );
				DBGMSG("\t %s support 48 bits/pixel(16 bits/color)\n", (block[6] & 0x10)? "":"no" );
				DBGMSG("\t %s support YUV444 in Deep Color mode\n", (block[6] & 0x08)? "":"no" );
				DBGMSG("\t %s support DVI dual-link\n", (block[6] & 0x01)? "":"no" );
				if( len < 7 ) break;
				DBGMSG("\t Max TMDS Clock %d\n", block[7] );
				if( len < 8 ) break;
				if( block[8] & BIT(5) ){
					DBGMSG("\t HDMI Video present\n");
				}
				DBGMSG("\t Max TMDS Clock %d\n", block[7] );
#ifdef CONFIG_HDMI_3D
				if( len < 13 ) break;
				if( block[13] & BIT(7) ){
					int index = 13;
						
					DBGMSG("\t 3D present\n");
					edid_info.option |= EDID_OPT_3D;
					DBGMSG("\t 3D Multi present %d\n",(block[index]&0x60)>>5);
					DBGMSG("\t HDMI_XX_LEN %d,HDMI_3D_LEN %d\n",(block[index+1]&0xE0)>>5,(block[index+1]&0x1F));
					// if( HDMI_XX_LEN > 0 ) Refer to HDMI Spec Ver 1.4a
					index += ((block[index+1]&0xE0)>>5);
					
					switch((block[13]&0x60)>>5){
						case 0:
							ALL_15...0 and 3D_MASK_15...0 not present.
							break;
						case 1:
							ALL_15...0 is present
							break;
						case 2:
							break;
						case 3:
							break;
					}
					
				}				
#endif
				break;
			case 4:	// Speaker Allocation Data Block
				DBGMSG("Speaker Allocation Data Block\n");
				break;
			case 5:	// VESA DTC Data Block
				DBGMSG("VESA DTC Data Block\n");
				break;
			case 7:	// Use Extended Tag
				DBGMSG("Use Extended Tag\n");
				break;
			case 0:	// Reserved
			default:
				len = 0;
				break;
		}
		block += (1+len);
	} while(len);

	block = edid + edid[2];
	len = (128 - edid[2]) / 18;
	for(i=0; i<len; i++, block += 18 ){
		p = &edid_info.cea_timing[i];
		if( (p->pixel_clock = ((block[1]<<8)+block[0])*10000) == 0 ){
			break;
		}
		p->hpixel = ((block[4]&0xF0)<<4)+block[2];
		p->hbp = ((block[4]&0x0F)<<8)+block[3];		// h porch
		p->vpixel = ((block[7]&0xF0)<<4)+block[5];
		p->vbp = ((block[7]&0x0F)<<8)+block[6];		// v porch
		fps = p->pixel_clock / ((p->hpixel+p->hbp)*(p->vpixel+p->vbp));
		if( fps == 59 ) fps = 60;
		p->hfp = ((block[11]&0xC0)<<2)+block[8];
		p->hsync = ((block[11]&0x30)<<4)+block[9];
		p->hbp = p->hbp - p->hfp - p->hsync;
		p->vfp = ((block[11]&0x0C)<<2)+((block[10]&0xF0)>>4);
		p->vsync = ((block[11]&0x03)<<4)+(block[10]&0x0F);
		p->vbp = p->vbp - p->vfp - p->vsync;
		p->option = VPP_SET_OPT_FPS(p->option,fps);
		p->option |= (block[17]&0x80)? VPP_OPT_INTERLACE:0;
		p->option |= (block[17]&0x04)? VPP_DVO_VSYNC_POLAR_HI:0;
		p->option |= (block[17]&0x02)? VPP_DVO_SYNC_POLAR_HI:0;
		DBGMSG("\t%dx%d%s@%d,clk %d\n",p->hpixel,p->vpixel,(block[17]&0x80)?"I":"P",fps,p->pixel_clock);
		DBGMSG("\t\tH bp %d,sync %d,fp %d\n",p->hbp,p->hsync,p->hfp);
		DBGMSG("\t\tV bp %d,sync %d,fp %d\n",p->vbp,p->vsync,p->vfp);
	}
	return 0;
}

int edid_parse(unsigned char *edid)
{
	int ext_cnt = 0;

	if( edid == 0 )
		return 0;

	memset(&edid_info,0,sizeof(edid_info_t));
	if( edid_parse_v1(edid) == 0 ){
		ext_cnt = edid[0x7E];
		if( ext_cnt >= EDID_BLOCK_MAX ){
			DPRINT("[EDID] *W* ext block cnt %d\n",ext_cnt);
			ext_cnt = EDID_BLOCK_MAX - 1;
		}
		edid_info.option = EDID_OPT_VALID;
	}

	while( ext_cnt ){
		edid += 128;
		ext_cnt--;
		if( edid_parse_CEA(edid) == 0 ){
			continue;
		}
		
		DPRINT("*W* not support EDID\n");
		edid_dump(edid);
	}
	return edid_info.option;
}

int edid_find_support(unsigned int resx,unsigned int resy,int freq,vpp_timing_t **timing)
{
	int ret;
	int i;

	ret = 0;
	*timing = 0;

	// find cea timing
	for(i=0;i<6;i++){
		unsigned int option;
		unsigned int vpixel;

		if( edid_info.cea_timing[i].pixel_clock == 0 )
			continue;
		option = VPP_GET_OPT_FPS(edid_info.cea_timing[i].option);
		vpixel = edid_info.cea_timing[i].vpixel;
		if( edid_info.cea_timing[i].option & VPP_OPT_INTERLACE ){
			option |= EDID_TMR_INTERLACE;
			vpixel *= 2;
		}
		if( (resx == edid_info.cea_timing[i].hpixel) && (resy == vpixel) ){
			if( freq == option ){
				*timing = &edid_info.cea_timing[i];
				ret = 4;
				goto find_end;
			}
		}
	}

	// find vic timing
	for(i=0;i<64;i++){
		int vic;
		int interlace;
		
		if( (edid_info.cea_vic[i/8] & (0x1 << (i%8))) == 0 ){
			continue;
		}

		if( i >= HDMI_VIDEO_CODE_MAX )
			continue;

		vic = i;
		if( (resx == hdmi_vic_info[vic].resx) && (resy == hdmi_vic_info[vic].resy) ){
			if( (freq & EDID_TMR_FREQ) != hdmi_vic_info[vic].freq ){
				continue;
			}
			interlace = (freq & EDID_TMR_INTERLACE)? HDMI_VIC_INTERLACE:HDMI_VIC_PROGRESS;
			if( (hdmi_vic_info[vic].option & HDMI_VIC_INTERLACE) == interlace ){
				ret = 5;
				goto find_end;
			}
		}
	}

	// find detail timing
	for(i=0;i<4;i++){
		unsigned int option;
		unsigned int vpixel;

		if( edid_info.detail_timing[i].pixel_clock == 0 )
			continue;
		option = VPP_GET_OPT_FPS(edid_info.detail_timing[i].option);
		vpixel = edid_info.detail_timing[i].vpixel;
		if( edid_info.detail_timing[i].option & VPP_OPT_INTERLACE ){
			option |= EDID_TMR_INTERLACE;
			vpixel *= 2;
		}
		if( (resx == edid_info.detail_timing[i].hpixel) && (resy == edid_info.detail_timing[i].vpixel) ){
			if( freq == option ){
				*timing = &edid_info.detail_timing[i];
				ret = 3;
				goto find_end;
			}
		}
	}

	// find established timing
	if( edid_info.establish_timing ){
		for(i=0;i<17;i++){
			if( edid_info.establish_timing & (0x1 << i) ){
				if( (resx == edid_establish_timing[i].resx) && (resy == edid_establish_timing[i].resy) ){
					if( freq == edid_establish_timing[i].freq ){
						ret = 1;
						goto find_end;
					}
				}
			}
		}
	}

	// find standard timing
	for(i=0;i<8;i++){
		if( edid_info.standard_timing[i].resx == 0 )
			continue;
		if( (resx == edid_info.standard_timing[i].resx) && (resy == edid_info.standard_timing[i].resy) ){
			if( freq == edid_info.standard_timing[i].freq ){
				ret = 2;
				goto find_end;
			}
		}
	}
find_end:
#if 0
	if( (resx == 1920) && (resy==1080) && !(freq & EDID_TMR_INTERLACE) ){
		ret = 0;
	}
#endif
//	printk("[EDID] %s support %dx%d@%d%s(ret %d)\n",(ret)? "":"No",resx,resy,freq & EDID_TMR_FREQ,(freq & EDID_TMR_INTERLACE)?"I":"P",ret);
	return ret;
}

int edid_parse_option(unsigned char *edid)
{
	int option = 0;
	unsigned char *block;
	unsigned char checksum = 0;
	int i,len;
	int ext_cnt;

	if( edid == 0 ){
        DBGMSG("*E* buf pointer invalid\n");
		return 0;
	}

    for( i = 0; i < EDID_LENGTH; i++ )
        checksum += edid[ i ];
    if ( checksum != 0 ) {
        DPRINT("*E* EDID checksum failed - data is corrupt\n" );
    }  
	
    if ( memcmp( edid+EDID_HEADER, edid_v1_header, EDID_HEADER_END+1 ) ) {
        DBGMSG("*E* first bytes don't match EDID version 1 header\n");
		edid_dump(edid);
		return 0;
    }
	ext_cnt = edid[0x7E];
	if( ext_cnt >= EDID_BLOCK_MAX ){
		DPRINT("[EDID] *W* ext block cnt %d\n",ext_cnt);
		ext_cnt = EDID_BLOCK_MAX - 1;
	}

	while( ext_cnt ){
		edid += 128;
		ext_cnt--;
		
		if((edid[0]!=0x2) || (edid[1]!=0x3)){
			continue;
		}

	    for( i = 0; i < EDID_LENGTH; i++ )
	        checksum += edid[ i ];

	    if ( checksum != 0 ) {
	        DBGMSG("*E* CEA EDID checksum failed - data is corrupt\n" );
//	        continue;
	    }  

		option = (edid[3] & 0xF0) | EDID_OPT_VALID;
		DBGMSG("\t %s support 422\n", (edid[3] & 0x10)? "":"no" );
		DBGMSG("\t %s support 444\n", (edid[3] & 0x20)? "":"no" );
		DBGMSG("\t %s support audio\n", (edid[3] & 0x40)? "":"no" );
		DBGMSG("\t %s support underscan\n", (edid[3] & 0x80)? "":"no" );

		block = edid + 4;
		do {
			len = block[0] & 0x1F;
			switch(((block[0] & 0xE0)>>5)){
				case 1:	// Audio Data Block
					DBGMSG("Audio Data Block\n");
					DBGMSG("\t Max channel %d\n", (edid[1] & 0x7)+1 );				
					DBGMSG("\t %s support 32 KHz\n", (edid[2] & 0x1)? "":"no" );
					DBGMSG("\t %s support 44 KHz\n", (edid[2] & 0x2)? "":"no" );
					DBGMSG("\t %s support 48 KHz\n", (edid[2] & 0x4)? "":"no" );
					DBGMSG("\t %s support 88 KHz\n", (edid[2] & 0x8)? "":"no" );
					DBGMSG("\t %s support 96 KHz\n", (edid[2] & 0x10)? "":"no" );
					DBGMSG("\t %s support 176 KHz\n", (edid[2] & 0x20)? "":"no" );
					DBGMSG("\t %s support 192 KHz\n", (edid[2] & 0x40)? "":"no" );
					DBGMSG("\t %s support 16 bit\n", (edid[3] & 0x1)? "":"no" );
					DBGMSG("\t %s support 20 bit\n", (edid[3] & 0x2)? "":"no" );
					DBGMSG("\t %s support 24 bit\n", (edid[3] & 0x4)? "":"no" );
					break;
				case 2:	// Video Data Block
					DBGMSG("Video Data Block\n");
					break;
				case 3: // Vendor Spec Data Block
					DBGMSG("Vendor Spec Data Block\n");
					if( (block[1]==0x03) && (block[2]==0x0C) && (block[3]==0x0)){	// IEEE Registration Identifier 0x000C03
						option |= EDID_OPT_HDMI;
						DBGMSG("\t support HDMI\n");
					}
					// DPRINT("\t Source Physical Addr %d.%d.%d.%d\n",(block[4]&0xF0)>>4,block[4]&0x0F,(block[5]&0xF0)>>4,block[5]&0x0F);
					edid_info.hdmi_phy_addr = (block[4] << 8) + block[5];
					break;
				case 4:	// Speaker Allocation Data Block
					DBGMSG("Speaker Allocation Data Block\n");
					break;
				case 5:	// VESA DTC Data Block
					DBGMSG("VESA DTC Data Block\n");
					break;
				case 7:	// Use Extended Tag
					DBGMSG("Use Extended Tag\n");
					break;
				case 0:	// Reserved
				default:
					len = 0;
					break;
			}
			block += (1+len);
		} while(len);
	}
	return option;
}

int edid_check_block(char *edid)
{
	unsigned char checksum = 0;
	int i;

	if( edid == 0 ){
        DBGMSG("*E* buf pointer invalid\n");
		return 1;
	}

    for( i = 0; i < EDID_LENGTH; i++ )
        checksum += edid[ i ];
    if ( checksum != 0 ) {
        DPRINT("*E* EDID checksum failed - data is corrupt\n" );
		return 1;
    }  
	return 0;
}

unsigned int edid_get_hdmi_phy_addr(void)
{
	return edid_info.hdmi_phy_addr;
}

