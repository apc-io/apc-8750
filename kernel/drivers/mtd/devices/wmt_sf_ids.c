/*++ 
Copyright (c) 2010 WonderMedia Technologies, Inc.

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software 
Foundation, either version 2 of the License, or (at your option) any later 
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details. You
should have received a copy of the GNU General Public License along with this
program. If not, see http://www.gnu.org/licenses/>.

WonderMedia Technologies, Inc.
10F, 529, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C.
--*/

#include <linux/module.h>
#include "wmt_sf.h"

/*
*	Chip ID list
*
*	Name. ID code, pagesize, chipsize in MegaByte, eraseblock size,
*	options
*
*	Pagesize; 0, 256, 512
*	0	get this information from the extended chip ID
+	256	256 Byte page size
*	512	512 Byte page size
*/
struct wm_sf_dev_t sf_ids[] = {
	/* EON */
	{SF_IDALL(EON_MANUFACT, EON_25P16_ID), (2*1024)},
	{SF_IDALL(EON_MANUFACT, EON_25P64_ID), (8*1024)},
	{SF_IDALL(EON_MANUFACT, EON_25F40_ID), 512},
	{SF_IDALL(EON_MANUFACT, EON_25F16_ID), (2*1024)},
	{SF_IDALL(EON_MANUFACT, EON_25Q64_ID), (8*1024)},
	/* NUMONYX */
	{SF_IDALL(NUMONYX_MANUFACT, NX_25P16_ID), (2*1024)},
	{SF_IDALL(NUMONYX_MANUFACT, NX_25P64_ID), (8*1024)},
	/* MXIC */
	{SF_IDALL(MXIC_MANUFACT, MX_L512_ID), 64},
	{SF_IDALL(MXIC_MANUFACT, MX_L1605D_ID), (2*1024)},
	{SF_IDALL(MXIC_MANUFACT, MX_L3205D_ID), (4*1024)},
	{SF_IDALL(MXIC_MANUFACT, MX_L6405D_ID), (8*1024)},
	{SF_IDALL(MXIC_MANUFACT, MX_L1635D_ID), (2*1024)},
	{SF_IDALL(MXIC_MANUFACT, MX_L3235D_ID), (4*1024)},
	{SF_IDALL(MXIC_MANUFACT, MX_L12805D_ID), (16*1024)},
	/* SPANSION */
	{SF_IDALL(SPANSION_MANUFACT, SPAN_FL016A_ID), (2*1024)},
	{SF_IDALL(SPANSION_MANUFACT, SPAN_FL064A_ID), (8*1024)},
	/* SST */
	{SF_IDALL(SST_MANUFACT, SST_VF016B_ID), (2*1024)},
	/*WinBond*/
	{SF_IDALL(WB_MANUFACT, WB_X16A_ID), (2*1024)},
	{SF_IDALL(WB_MANUFACT, WB_X32_ID), (4*1024)},
	{SF_IDALL(WB_MANUFACT, WB_X64_ID), (8*1024)},
	{SF_IDALL(WB_MANUFACT, WB_X64_25Q64_ID), (8*1024)},
	{SF_IDALL(WB_MANUFACT, WB_X128_ID), (16*1024)},
	/* ATMEL */
	{SF_IDALL(ATMEL_MANUF, AT_25DF041A_ID), 512},
	/* GD */
	{SF_IDALL(GD_MANUF, GD_25Q40_ID), 512},
	{SF_IDALL(GD_MANUF, GD_25Q128_ID), 16*1024},
	{0, }
};
EXPORT_SYMBOL(sf_ids);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("SF device IDs");
