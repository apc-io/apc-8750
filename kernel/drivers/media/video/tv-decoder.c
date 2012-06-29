/*++ 
 * linux/drivers/media/video/tv-decoder.c
 * WonderMedia v4l tv decoder device driver
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




#include "tv-decoder.h"

//static struct video_device tv_decoder;
static int video_nr = 1; /* /dev/videoN, -1 for autodetect */
#if 0
static struct file_operations tv_decoder_fops = {
	.owner          = THIS_MODULE,
	.open           = tv_decoder_open, //video_exclusive_open,
	.release        = tv_decoder_release, //video_exclusive_release,
	.ioctl          = video_ioctl2, /* V4L2 ioctl handler */
	/*
	.read           = tv_decoder_read,
	*/
	.mmap           = tv_decoder_mmap,
	.llseek         = no_llseek,
};

static struct video_device tv_decoder = {
	.name           = "tv_decoder",
	.type           = VID_TYPE_CAPTURE,
	.hardware       = 0,
	.fops           = &tv_decoder_fops,
	.minor          = -1,
	.release        = video_device_release,
	.debug          = 0,
	.vidioc_querycap      = tv_decoder_querycap,
	.vidioc_enum_fmt_cap  = tv_decoder_enum_fmt_cap,
	.vidioc_g_fmt_cap     = tv_decoder_g_fmt_cap,
	.vidioc_try_fmt_cap   = tv_decoder_try_fmt_cap,
	.vidioc_s_fmt_cap     = tv_decoder_s_fmt_cap,
	.vidioc_reqbufs       = tv_decoder_reqbufs,
	.vidioc_querybuf      = tv_decoder_querybuf,
	.vidioc_qbuf          = tv_decoder_qbuf,
	.vidioc_dqbuf         = tv_decoder_dqbuf,
	.vidioc_s_std         = tv_decoder_s_std,
	.vidioc_enum_input    = tv_decoder_enum_input,
	.vidioc_g_input       = tv_decoder_g_input,
	.vidioc_s_input       = tv_decoder_s_input,
	.vidioc_queryctrl     = tv_decoder_queryctrl,
	.vidioc_g_ctrl        = tv_decoder_g_ctrl,
	.vidioc_s_ctrl        = tv_decoder_s_ctrl,
	.vidioc_streamon      = tv_decoder_streamon,
	.vidioc_streamoff     = tv_decoder_streamoff,
};

static int __init tv_decoder_init(void)
{
	int ret;
       printk("tv_decoder_init() \n");

	ret = video_register_device(&tv_decoder, VFL_TYPE_GRABBER, video_nr);
	if (ret < 0)
		printk(KERN_ERR "WonderMedia TV_DECODER camera register failed\n");

	return ret;
}
 
static void __exit tv_decoder_exit(void)
{
	video_unregister_device(&tv_decoder);
}

module_init(tv_decoder_init);
module_exit(tv_decoder_exit);

MODULE_DESCRIPTION("WonderMedia TV_DECODER  for ARM SoC");
MODULE_AUTHOR("WonderMedia Technologies, Inc.");
MODULE_LICENSE("Dual BSD/GPL");
#endif
static const struct v4l2_ioctl_ops tv_decoder_ioctl_ops = {
	.vidioc_querycap    		    = tv_decoder_querycap,
	.vidioc_enum_input     		    = tv_decoder_enum_input,
	.vidioc_g_input      		    = tv_decoder_g_input,
	.vidioc_s_input      		    = tv_decoder_s_input,
	.vidioc_s_std 				    = tv_decoder_s_std,
	.vidioc_reqbufs			    = tv_decoder_reqbufs,
	.vidioc_querybuf		    	    = tv_decoder_querybuf,
	.vidioc_qbuf			    	    = tv_decoder_qbuf,
	.vidioc_dqbuf			   	    = tv_decoder_dqbuf,
	.vidioc_streamon		   	    = tv_decoder_streamon,
	.vidioc_streamoff		   	    = tv_decoder_streamoff,
	.vidioc_enum_fmt_vid_cap 	    = tv_decoder_enum_fmt_cap,
	.vidioc_g_fmt_vid_cap 		    = tv_decoder_g_fmt_cap,
	.vidioc_s_fmt_vid_cap  		    = tv_decoder_s_fmt_cap,
	.vidioc_try_fmt_vid_cap  	    = tv_decoder_try_fmt_cap,
	.vidioc_queryctrl 			    = tv_decoder_queryctrl,
	.vidioc_s_ctrl       			    = tv_decoder_s_ctrl,
	.vidioc_g_ctrl       			    = tv_decoder_g_ctrl,
};

static const struct v4l2_file_operations tv_decoder_fops = {
	.owner = THIS_MODULE,
	.open = tv_decoder_open,
	.release = tv_decoder_release,
	.ioctl = video_ioctl2,
	.mmap = tv_decoder_mmap,

};

struct video_device  tv_decoder = {
	.name =  "tv_decoder",
	.fops = &tv_decoder_fops,
	.ioctl_ops = &tv_decoder_ioctl_ops,
	.release = video_device_release,
	.tvnorms = V4L2_STD_NTSC | V4L2_STD_PAL | V4L2_STD_SECAM,
	.minor = -1
};


static int __init tv_decoder_init(void)
{
	int ret;

	ret = video_register_device(&tv_decoder, VFL_TYPE_GRABBER, video_nr);
	if (ret < 0)
		printk(KERN_ERR "WonderMedia CMOS camera register failed\n");
	else
		printk(KERN_ERR "WonderMedia CMOS camera register OK \n");


	return ret;
}

static void __exit tv_decoder_exit(void)
{
	video_unregister_device(&tv_decoder);
}

module_init(tv_decoder_init);
module_exit(tv_decoder_exit);

MODULE_DESCRIPTION("WonderMedia CMOS Camera for ARM SoC");
MODULE_AUTHOR("WonderMedia Technologies, Inc.");
MODULE_LICENSE("Dual BSD/GPL");
