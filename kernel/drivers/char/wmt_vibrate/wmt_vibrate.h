/*++

	Some descriptions of such software. Copyright (c) 2010  WonderMedia Technologies, Inc.

	This program is free software: you can redistribute it and/or modify it under the
	terms of the GNU General Public License as published by the Free Software Foundation,
	either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT
	ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
	PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	You should have received a copy of the GNU General Public License along with
	this program.  If not, see <http://www.gnu.org/licenses/>.

	WonderMedia Technologies, Inc.
	2010-2-26, HangYan, ShenZhen
--*/

#ifndef __WMT_VIBRATE_DRIVER__
#define __WMT_VIBRATE_DRIVER__

#include <linux/types.h>

/*******************io ctrl commands*****************************/

#define WMT_VIBRATE_IOCTL_MAGIC '7'
#define WMT_VIBRATE_ENABLE    _IOW(WMT_VIBRATE_IOCTL_MAGIC, 0, unsigned char)  // 0:disable, 1:enable






#endif

