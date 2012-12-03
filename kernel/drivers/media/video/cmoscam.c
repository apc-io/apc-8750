 /*++ 
 * linux/drivers/media/video/cmoscam.c
 * WonderMedia v4l cmos device driver
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

#include "cmoscam.h"

static int video_nr = -1; /* /dev/videoN, -1 for autodetect */

static const struct v4l2_ioctl_ops cmos_ioctl_ops = {
	.vidioc_querycap    		    = cmos_cam_querycap,
	.vidioc_enum_input     		    = cmos_cam_enum_input,
	.vidioc_g_input      		    = cmos_cam_g_input,
	.vidioc_s_input      		    = cmos_cam_s_input,
	.vidioc_s_std 				    = cmos_cam_s_std,
	.vidioc_reqbufs			    = cmos_cam_reqbufs,
	.vidioc_querybuf		    	    = cmos_cam_querybuf,
	.vidioc_qbuf			    	    = cmos_cam_qbuf,
	.vidioc_dqbuf			   	    = cmos_cam_dqbuf,
	.vidioc_streamon		   	    = cmos_cam_streamon,
	.vidioc_streamoff		   	    = cmos_cam_streamoff,
	.vidioc_enum_fmt_vid_cap 	    = cmos_cam_enum_fmt_cap,
	.vidioc_g_fmt_vid_cap 		    = cmos_cam_g_fmt_cap,
	.vidioc_s_fmt_vid_cap  		    = cmos_cam_s_fmt_cap,
	.vidioc_try_fmt_vid_cap  	    = cmos_cam_try_fmt_cap,
	.vidioc_queryctrl 			    = cmos_cam_queryctrl,
	.vidioc_s_ctrl       			    = cmos_cam_s_ctrl,
	.vidioc_g_ctrl       			    = cmos_cam_g_ctrl,
};

static const struct v4l2_file_operations cmos_cam_fops = {
	.owner = THIS_MODULE,
	.open = cmos_cam_open,
	.release = cmos_cam_release,
	.ioctl = video_ioctl2,
	.mmap = cmos_cam_mmap,

};

struct video_device  cmos_cam = {
	.name =  "cmos_cam",
	.fops = &cmos_cam_fops,
	.ioctl_ops = &cmos_ioctl_ops,
	.release = video_device_release,
	.tvnorms = V4L2_STD_NTSC | V4L2_STD_PAL | V4L2_STD_SECAM,
	.minor = -1
};


static int __init cmos_cam_init(void)
{
	int ret;

	ret = video_register_device(&cmos_cam, VFL_TYPE_GRABBER, video_nr);
	if (ret < 0)
		printk(KERN_ERR "WonderMedia CMOS camera register failed\n");
	else
		printk(KERN_ERR "WonderMedia CMOS camera register OK \n");


	return ret;
}

static void __exit cmos_cam_exit(void)
{
	video_unregister_device(&cmos_cam);
}

module_init(cmos_cam_init);
module_exit(cmos_cam_exit);

MODULE_DESCRIPTION("WonderMedia CMOS Camera for ARM SoC");
MODULE_AUTHOR("WonderMedia Technologies, Inc.");
MODULE_LICENSE("Dual BSD/GPL");

