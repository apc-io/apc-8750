#
# Makefile for the video capture/playback device drivers.
#
g_wmt_v4l2-objs    += wmt-vid.o
obj-$(CONFIG_WMT_V4L2) += g_wmt_v4l2.o


g_wmt_v4l2_cmos-objs    += cmos/cmos-dev.o cmos/wmt-cmos.o cmos/cmos-dev-ov.o cmos/cmos-dev-gc.o cmos/cmos-dev-siv.o
obj-$(CONFIG_WMT_V4L2_CMOS) += g_wmt_v4l2_cmos.o


g_wmt_v4l2_tv_decoder-objs    += tv_decoder/tv-device.o tv_decoder/wmt-tv-decoder.o 
obj-$(CONFIG_WMT_V4L2_TV_DECODER) += g_wmt_v4l2_tv_decoder.o




