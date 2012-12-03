#
# (C) Copyright 2002
# Gary Jennejohn, DENX Software Engineering, <gj@denx.de>
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

PLATFORM_RELFLAGS += -fno-strict-aliasing  -fno-common -ffixed-r8 \
	-msoft-float

PLATFORM_CPPFLAGS += -march=armv6
# =========================================================================
#
# Supply options according to compiler version
#
# =========================================================================
PLATFORM_CPPFLAGS +=$(call cc-option,-mapcs-32)
PLATFORM_RELFLAGS +=$(call cc-option,-mshort-load-bytes,$(call cc-option,-malignment-traps,))
