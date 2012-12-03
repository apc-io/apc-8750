#
# Makefile for vt1603 battery core.
#

ifeq ($(CONFIG_VT1603_BAT_SPI), y)
		obj-$(CONFIG_WMT_VT1603_BATTERY) += vt1603_bat_spi.o
endif

ifeq ($(CONFIG_VT1603_BAT_I2C), y)
		obj-$(CONFIG_WMT_VT1603_BATTERY) += vt1603_bat_i2c.o
endif

