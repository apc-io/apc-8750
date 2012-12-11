#
# Makefile for the linux kernel.
#

# Common support
obj-y   := generic.o irq.o board.o wmt_clk.o dma.o perf.o #time.o
obj-m   := 
obj-n   :=
obj-    :=

# LEDs support
led-y   := #leds.o leds-wmt.o
obj-$(CONFIG_LEDS)				+= $(led-y)
obj-$(CONFIG_PM)                                += pm.o pm_cpai.o
obj-$(CONFIG_WMT_REGMON)			+= regmon.o

