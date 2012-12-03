 /*++ 
 * linux/drivers/media/video/cmoscam.h
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

#include <linux/module.h>
#include <linux/version.h>
#include <linux/videodev2.h>
#ifdef CONFIG_VIDEO_V4L1_COMPAT
#include <linux/videodev.h>
#endif
#include <media/v4l2-common.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-device.h>



extern int tv_decoder_open (struct inode *, struct file *);
extern int tv_decoder_release (struct file *);
extern ssize_t tv_decoder_read(struct file *, char __user *, size_t, loff_t *);
extern int tv_decoder_mmap (struct file *, struct vm_area_struct *);
extern unsigned int tv_decoder_poll (struct file *, struct poll_table_struct *);
extern int tv_decoder_ioctl (struct inode *, struct file *, unsigned int, unsigned long);
extern long tv_decoder_compat_ioctl (struct file *, unsigned int, unsigned long);

extern int tv_decoder_querycap(struct file *file, void *fh,
			     struct v4l2_capability *cap);
extern int tv_decoder_cropcap(struct file *file, void *__fh,
					struct v4l2_cropcap *cropcap);

extern int tv_decoder_enum_fmt_cap(struct file *file, void *fh,
				 struct v4l2_fmtdesc *f);
extern int tv_decoder_g_fmt_cap(struct file *file, void *fh,
			      struct v4l2_format *f);
extern int tv_decoder_try_fmt_cap(struct file *file, void *fh,
				struct v4l2_format *f);
extern int tv_decoder_s_fmt_cap(struct file *file, void *fh,
			      struct v4l2_format *f);
extern int tv_decoder_reqbufs(struct file *file, void *fh,
			    struct v4l2_requestbuffers *b);
extern int tv_decoder_querybuf(struct file *file, void *fh,
			     struct v4l2_buffer *b);
extern int tv_decoder_qbuf(struct file *file, void *fh,
			 struct v4l2_buffer *b);
extern int tv_decoder_dqbuf(struct file *file, void *fh,
			  struct v4l2_buffer *b);
extern int tv_decoder_s_std(struct file *file, void *fh, v4l2_std_id a);
extern int tv_decoder_enum_input(struct file *file, void *fh,
			       struct v4l2_input *inp);
extern int tv_decoder_g_input(struct file *file, void *fh, unsigned int *i);
extern int tv_decoder_s_input(struct file *file, void *fh, unsigned int i);
extern int tv_decoder_queryctrl(struct file *file, void *fh,
			      struct v4l2_queryctrl *a);
extern int tv_decoder_g_ctrl(struct file *file, void *fh, struct v4l2_control *a);
extern int tv_decoder_s_ctrl(struct file *file, void *fh, struct v4l2_control *a);
extern int tv_decoder_streamon(struct file *file, void *fh, enum v4l2_buf_type i);
extern int tv_decoder_streamoff(struct file *file, void *fh,
			      enum v4l2_buf_type i);

