#
# Makefile for the video capture/playback device drivers.
#

g_wmt_vd_com-objs    = wmt-vd.o VpuApiFunc.o VpuApi.o wmt-msvd.o
g_wmt_lock-objs      = wmt-lock.o
g_wmt_vd_jdec-objs   = jpeg/hw-jdec.o jpeg/wmt-jdec.o
g_wmt_vd_mpeg2-objs  = mpeg2/wmt-mpeg2.o
g_wmt_vd_mpeg4-objs  = mpeg4/wmt-mpeg4.o
g_wmt_vd_divx-objs   = divx/wmt-divx.o
g_wmt_vd_h263-objs   = h263/wmt-h263.o
g_wmt_vd_h264-objs   = h264/wmt-h264.o
g_wmt_vd_vc1-objs    = vc1/wmt-vc1.o
g_wmt_vd_rv-objs     = rv/wmt-rv.o
g_wmt_vdma-objs      = vdma/wmt-vdma.o


obj-$(CONFIG_WMT_VIDEO_DECODE) += g_wmt_vd_com.o
obj-$(CONFIG_WMT_VIDEO_LOCK) += g_wmt_lock.o
obj-$(CONFIG_WMT_VIDEO_DECODE_JPEG) += g_wmt_vd_jdec.o
obj-$(CONFIG_WMT_VIDEO_DECODE_MPEG2) += g_wmt_vd_mpeg2.o
obj-$(CONFIG_WMT_VIDEO_DECODE_MPEG4) += g_wmt_vd_mpeg4.o
obj-$(CONFIG_WMT_VIDEO_DECODE_DIVX) += g_wmt_vd_divx.o
obj-$(CONFIG_WMT_VIDEO_DECODE_H263) += g_wmt_vd_h263.o
obj-$(CONFIG_WMT_VIDEO_DECODE_H264) += g_wmt_vd_h264.o
obj-$(CONFIG_WMT_VIDEO_DECODE_VC1) += g_wmt_vd_vc1.o
obj-$(CONFIG_WMT_VIDEO_DECODE_RV) += g_wmt_vd_rv.o
obj-$(CONFIG_WMT_VIDEO_DMA) += g_wmt_vdma.o

