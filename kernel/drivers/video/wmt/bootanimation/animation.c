/**
 * Author: Aimar Ma <AimarMa@wondermedia.com.cn> 
 *  
 * Show animation during kernel boot stage 
 *  
 *  
 **/

#include <linux/mm.h>
#include <linux/timer.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/version.h>
#include <linux/module.h>
#include <asm/io.h>
#include <linux/proc_fs.h>
#include <linux/ioport.h>
#include "animation.h"
#include "buffer.h"
#include "LzmaDec.h"
#include "../memblock.h"

#include "anim_data.h"


#define ANIM_STOP_PROC_FILE "kernel_animation"

#define     MAX_CLIP_COUNT          6

extern int wmt_getsyspara(char *varname, unsigned char *varval, int *varlen);
//extern void clear_animation_fb(void * p);
//extern void flip_animation_fb(int pingpong);



#define THE_MB_USER "Boot-Animation"

#define DEFAULT_BUF_IMAGES  4       //default buffered image count

#undef THIS_DEBUG
//#define THIS_DEBUG



#ifdef THIS_DEBUG
#define LOG_DBG(fmt,args...)	printk(KERN_INFO "[Boot Animation] " fmt , ## args)
#define LOG_INFO(fmt,args...)	printk(KERN_INFO "[Boot Animation] " fmt, ## args)
#define LOG_ERROR(fmt,args...)	printk(KERN_ERR "[Boot Animation] " fmt , ## args)  
#else
#define LOG_DBG(fmt,args...)	
#define LOG_INFO(fmt,args...)   printk(KERN_INFO "[Boot Animation] " fmt, ## args)
#define LOG_ERROR(fmt,args...)	printk(KERN_ERR "[Boot Animation] " fmt , ## args)  
#endif


// MUST match Windows PC tool. Don't change it.
struct animation_clip_header{
	int  xres;
	int  yres;
    int  linesize;
	unsigned char x_mode;
	unsigned char y_mode;
	short x_offset;
	short y_offset;
	unsigned char repeat;
	unsigned char reserved;
	int  interval;
	int  image_count;
	int  data_len;
};

// MUST match Windows PC tool. Don't change it.
struct file_header {
	int maigc;
	unsigned short version;
	unsigned char clip_count;
	unsigned char color_format;
    unsigned int  file_len;
};



struct play_context {
    struct animation_clip_header *clip;
    int xpos;           //  top postion
    int ypos;           //  left postion

    volatile int  play_thread;
    animation_buffer buf;
};


//  globe value to stop the animation loop
static volatile int g_logo_stop = 0;
static struct       animation_fb_info fb;

static void *SzAlloc(void *p, size_t size)
{
    void * add = (void *)mb_alloc(size);
    LOG_DBG("alloc: size %d, add = %p \n", size, add);
    return add;
}

static void SzFree(void *p, void *address) {
    if (address != 0) {
        LOG_DBG("free: address = %p \n", address);
        mb_free((int)address);
    }
}

static ISzAlloc g_Alloc = { SzAlloc, SzFree };


static int show_frame(struct play_context *ctx, unsigned char *data)
{
    unsigned char * dest;
    int linesize = fb.width * (fb.color_fmt + 1) * 2;
    int i = 0;
    struct animation_clip_header *clip = ctx->clip;
    if (g_logo_stop)
		return 0;

    dest = fb.addr;
    
//    printk(KERN_INFO "dest = 0x%p src = 0x%p\n", dest, data);

    if(data) {
        LOG_DBG("dest %p, data %p (%d,%d) (%dx%d) linesize(%d,%d)", dest, data, 
                ctx->xpos, ctx->ypos, clip->xres, clip->yres, clip->linesize, linesize);

        dest += ctx->ypos * linesize;
        dest += ctx->xpos * (fb.color_fmt + 1) * 2;
        
        for (i = 0; i <  clip->yres; i++) {
            memcpy(dest, data, clip->xres * (fb.color_fmt + 1) * 2);
            dest += linesize;
            data += clip->linesize;
        }
    }
 
    LOG_DBG("show_frame data %p, fb.addr %p\n", data, fb.addr);
    return 0;
}

static int decompress(struct play_context * ctx, unsigned char *src, unsigned int src_len)
{
	SRes res = 0;
	CLzmaDec state;
    size_t inPos = 0;
    unsigned char * inBuf;
    SizeT inProcessed;


	// 1)  read LZMA properties (5 bytes) and uncompressed size (8 bytes, little-endian) to header
	UInt64 unpackSize = 0;
	int i;

	unsigned char * header = src;
	for (i = 0; i < 8; i++)
		unpackSize += (UInt64)header[LZMA_PROPS_SIZE + i] << (i * 8);

	//  2) Allocate CLzmaDec structures (state + dictionary) using LZMA properties


	LzmaDec_Construct(&state);
    RINOK(LzmaDec_Allocate(&state, header, LZMA_PROPS_SIZE, &g_Alloc));
	if (res != SZ_OK)
		return res;

	//  3) Init LzmaDec structure before any new LZMA stream. And call LzmaDec_DecodeToBuf in loop
	LzmaDec_Init(&state);


	inBuf = header + LZMA_PROPS_SIZE + 8;

	for (;;)
	{
		unsigned int outSize;
		unsigned char * outBuf = animation_buffer_get_writable(&ctx->buf, &outSize);

		unsigned int frame_size = outSize;
		unsigned int decoded = 0;
		ELzmaFinishMode finishMode = LZMA_FINISH_ANY;
		ELzmaStatus status;
		while(1) {
            inProcessed = src_len - LZMA_PROPS_SIZE - 8 - inPos;

			res = LzmaDec_DecodeToBuf(&state, outBuf + frame_size - outSize, &outSize,
				inBuf + inPos, &inProcessed, finishMode, &status);

			inPos += inProcessed;
			decoded += outSize;
            unpackSize -= outSize;
			outSize = frame_size - decoded;

            LOG_DBG("Decoded %d bytes, inPos = %d\n", decoded, inPos);

			if(res != SZ_OK)
				break;

			if (outSize == 0)
				break;
		}

		animation_buffer_write_finish(&ctx->buf, outBuf);

		if (res != SZ_OK || unpackSize == 0 || g_logo_stop)
            break;
	}

    //  4) decompress finished, do clean job
    LzmaDec_Free(&state, &g_Alloc);
	return res;
}

static int animation_play(void * arg)
{
    unsigned char * data;
    static int not_first_play;
    struct play_context *ctx = (struct play_context *)arg;

    LOG_DBG( "animation_play thread start...\n");
    
    if(!not_first_play) {
        msleep(500);        // sleep a while to wait deocde few frames
//        clear_animation_fb(fb.addr);
        not_first_play = 1;
    }
    

    //  try to get a valid frame and show it
    while(!g_logo_stop) {
        data = animation_buffer_get_readable(&ctx->buf);
        if(data) {
            show_frame(ctx, data);
            animation_buffer_read_finish(&ctx->buf, data);
        }
        else {
            if( ctx->buf.eof )       //no data and reach eof
                break;
            LOG_DBG("animation_buffer_get_readable return NULL\n");
        }

        if(g_logo_stop)
            break;

        //else 
        msleep(ctx->clip->interval);
    }

    LOG_DBG( "animation_play thread End\n");
    animation_buffer_stop(&ctx->buf);
    ctx->play_thread = 0;
    return 0;
}


static void decode_clip(struct animation_clip_header *clip, unsigned char * data, int index)
{
	//	start timer for animation playback
    struct play_context ctx;
    int buf_images;

    LOG_DBG("Start playing clip %d, %dx%d, linesize %d, image %d, data_len %d\n", 
            index, clip->xres, clip->yres, clip->linesize, clip->image_count, clip->data_len);

    ctx.clip = clip;
    
    //  init the decompress buffer
	if (clip->repeat == 0) {
        buf_images = DEFAULT_BUF_IMAGES;
        if(buf_images > clip->image_count)
            buf_images = clip->image_count;
	}
	else {
        //  for the repeat clip, alloc a big memory to store all the frames
		buf_images = clip->image_count;
	}

    if( 0 != animation_buffer_init(&ctx.buf, clip->linesize * clip->yres, buf_images, &g_Alloc)){
        LOG_ERROR("Can't init animation buffer %dx%d\n", clip->linesize * clip->yres, buf_images);
        return;
    }


    ctx.xpos = clip->x_mode * (fb.width  / 2  - clip->xres / 2) + clip->x_offset;
    ctx.ypos = clip->y_mode * (fb.height / 2  - clip->yres / 2) + clip->y_offset;

    kthread_run(animation_play, &ctx, "wmt-boot-play");
    ctx.play_thread = 1;

	LOG_DBG("Start Decompressing ... \n");
    decompress(&ctx, data, clip->data_len);

    if (clip->repeat) {
		while (!g_logo_stop) {
			//  Fake decompress for REPEAT mode. (Only decompress the clip once so we can save more CPU)
			unsigned int outSize;
			unsigned char * outBuf;

            outBuf = animation_buffer_get_writable(&ctx.buf, &outSize);
			animation_buffer_write_finish(&ctx.buf, outBuf);
        }
	}

    LOG_DBG("Decompress finished!\n");
    ctx.buf.eof = 1;
    //wait the play thread exit
    while(ctx.play_thread) {
        msleep(10);
    }

    LOG_DBG("Play clip %d finished\n",  index);
	animation_buffer_release(&ctx.buf, &g_Alloc);
}


static int animation_main(void * arg)
{
    unsigned char * clip;
    int i;

    struct file_header *header = (struct file_header *)arg;
    int clip_count = header->clip_count;
	struct animation_clip_header clip_headers[MAX_CLIP_COUNT];
    unsigned char  *             clip_datas[MAX_CLIP_COUNT];

    if( clip_count > MAX_CLIP_COUNT)
        clip_count  = MAX_CLIP_COUNT;
    LOG_DBG( "animation_main thread start, clip_cout = %d\n", clip_count);


	clip = (unsigned char *)(header + 1);
    for (i = 0; i< clip_count; i++){
        memcpy(&clip_headers[i], clip, sizeof(struct animation_clip_header));
        clip += sizeof(struct animation_clip_header);
        clip_datas[i] = clip;
		clip += clip_headers[i].data_len;
	}

    LOG_DBG( "Found %d clip(s)\n", clip_count);

	for (i = 0; i < clip_count; i++) {
        if (!g_logo_stop)
            decode_clip(&clip_headers[i], clip_datas[i], i);
	}


    g_Alloc.Free(&g_Alloc, arg);
    LOG_DBG( "animation_main thread finished \n");
	return 0;
}


static int stop_proc_write( struct file   *file,
                           const char    *buffer,
                           unsigned long count,
                           void          *data )
{
    /* 
    char value[20];
    int len = count;
    if( len >= sizeof(value))
        len = sizeof(value) - 1;

    if(copy_from_user(value, buffer, len))
        return -EFAULT;

    value[len] = '\0';

    LOG_DBG("procfile_write get %s\n", value);
    */

    //anything will stop the boot animation
    animation_stop();

    return count; // discard other chars
}

struct proc_dir_entry *stop_proc_file;

static struct proc_dir_entry * create_stop_proc_file(void)
{
    stop_proc_file = create_proc_entry(ANIM_STOP_PROC_FILE, 0644, NULL);

    if( stop_proc_file != NULL )
        stop_proc_file->write_proc = stop_proc_write;
    else
        LOG_ERROR("Can not create /proc/%s file", ANIM_STOP_PROC_FILE);
    return stop_proc_file;
}

int animation_start(struct animation_fb_info *info)
{
    struct file_header *header;
    unsigned char * buffer;

    const void * animation_data = anim_data;
    if (animation_data == NULL) 
        return -1;

    if( !create_stop_proc_file() )
        return -1;

    header = (struct file_header *)animation_data;

    if (header->maigc != 0x12344321) {
        LOG_ERROR ("It's not a valid Animation file at 0x%p, first 4 bytes: 0x%x\n", animation_data, header->maigc);
        return -1;
    }

    buffer = g_Alloc.Alloc(&g_Alloc, header->file_len);
    if(!buffer) {
        LOG_ERROR ("Can't alloc enough memory, length %d\n", header->file_len);
        return -1;
    }

    memcpy(&fb, info, sizeof(fb));

    //copy it to the new buffer and start the play thread
    memcpy(buffer, header, header->file_len);
    g_logo_stop = 0;

    
    LOG_DBG("Start animation_main thread ...\n");
    kthread_run(animation_main, buffer, "wmt-boot-anim");
    return 0;
}

int animation_stop(void)
{
    LOG_INFO("animation_stop\n");
    g_logo_stop = 1;

    if( stop_proc_file ) {
        remove_proc_entry(ANIM_STOP_PROC_FILE,stop_proc_file);
        stop_proc_file = NULL;
    }

    return 0;
}

EXPORT_SYMBOL(animation_start);
EXPORT_SYMBOL(animation_stop);


