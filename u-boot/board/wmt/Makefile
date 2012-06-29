#
# See file CREDITS for list of people who contributed to this
# project.
#
# Some descriptions of such software. Copyright (c) 2008 WonderMedia Technologies, Inc.
#
# This program is free software: you can redistribute it and/or modify it under the
# terms of the GNU General Public License as published by the Free Software Foundation,
# either version 2 of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.  See the GNU General Public License for more details.
# You should have received a copy of the GNU General Public License along with
# this program.  If not, see <http://www.gnu.org/licenses/>.
#
# WonderMedia Technologies, Inc.
# 10F, 529, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C.
#

include $(TOPDIR)/config.mk

LIB	= lib$(BOARD).a

OBJS	:= wmt.o flash.o main.o \
	flash/spi_flash.o flash/nand_flash.o \
	flash/nor_flash_8bit.o  flash/nor_flash_16bit.o \
	wmt_clk.o env.o ehci-hcd.o usb_uhci.o wmt_i2c.o wmt_i2c_1.o
SOBJS	:= lowlevel_init.o

$(LIB):	$(OBJS) $(SOBJS)
	$(AR) crv $@ $^

clean:
	rm -f $(SOBJS) $(OBJS)

distclean:	clean
	rm -f $(LIB) core *.bak .depend

#########################################################################

.depend:	Makefile $(SOBJS:.o=.S) $(OBJS:.o=.c)
		$(CC) -M $(CPPFLAGS) $(SOBJS:.o=.S) $(OBJS:.o=.c) > $@

-include .depend

#########################################################################
