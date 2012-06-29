#
# Makefile for vt1603 touchscreen core.
#

ifeq ($(CONFIG_VT1603_TS_SPI), y)
		obj-$(CONFIG_WMT_VT1603A_TOUCHSCREEN) += vt1603a_mt_spi.o
endif

ifeq ($(CONFIG_VT1603_TS_I2C), y)
		obj-$(CONFIG_WMT_VT1603A_TOUCHSCREEN) += vt1603a_mt_i2c.o
endif
