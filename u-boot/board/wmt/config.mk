#
# (C) Copyright 2002-2004
# Gary Jennejohn, DENX Software Engineering, <gj@denx.de>
# David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
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
# WMT project EVB (ARM920T-Like)
#
# Suppose WMT has 1 bank of 64 MB DDR-SDRAM
#
# 0000'0000 to 0400'0000
#
# Linux-Kernel is expected to be at 0000'8000, entry 0000'8000
# optionally with a ramdisk at 0200'0000
#
# we load ourself to 03F8'0000
#
#
TEXT_BASE = 0x03F80000
