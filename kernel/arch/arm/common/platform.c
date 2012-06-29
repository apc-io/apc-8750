/*
 *  linux/arch/arm/common/platform.c
 *
 *	Copyright (c) 2008 WonderMedia Technologies, Inc.
 *
 *	This program is free software: you can redistribute it and/or modify it under the
 *	terms of the GNU General Public License as published by the Free Software Foundation,
 *	either version 2 of the License, or (at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful, but WITHOUT
 *	ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 *	PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *	You should have received a copy of the GNU General Public License along with
 *	this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *	WonderMedia Technologies, Inc.
 *	10F, 529, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C.
 */

#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/init.h>

int __init platform_add_device(struct platform_device *dev)
{
	int i;

	for (i = 0; i < dev->num_resources; i++) {
		struct resource *r = &dev->resource[i];

		// r->name = dev->dev.bus_id;

		if (r->flags & IORESOURCE_MEM &&
		    request_resource(&iomem_resource, r)) {
			printk(KERN_ERR
			       "%s%d: failed to claim resource %d\n",
			       dev->name, dev->id, i);
			break;
		}
	}
	if (i == dev->num_resources)
		platform_device_register(dev);
	return 0;
}
