#
# (C) Copyright 2000, 2001, 2002
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

LIB	= lib$(SOC).a

OBJS	= interrupts.o 	\
		serial.o 	\
          	ether.o  	\
          	zde.o  	\
	   	upcix.o 	\
          	pcinet.o  	\
           	gmii.o  	\
           	gcard.o 	\
             	gmib.o 	\
      		gisr.o 	\
        	galloct.o 	\
        	gmac.o  	\
	 	tconvert.o 	\
		gadapter.o 	\
   	mem.o      \
   	tcrc.o \
   	vpci.o   \
		gmacif.o \
		dma.o \
		api-loader.o \
		mmc.o
#            proto.o
#          mac.o    \
#          macif.o  \
#          adapter.o\
#          alloct.o \
#          mii.o    \
#          vpci.o   \
#          mem.o
#proco.o 	\
#tcrc.o \

all:	.depend $(LIB)

$(LIB):	$(OBJS)
	$(AR) crv $@ $(OBJS)

#########################################################################

.depend:	Makefile $(OBJS:.o=.c)
		$(CC) -M $(CFLAGS) $(OBJS:.o=.c)  > $@

sinclude .depend

#########################################################################
