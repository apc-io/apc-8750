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

#if !defined(__UPCI_H__)
#include "upci.h"
#endif
#if !defined(__MAC_H__)
#include "mac.h"
#endif
#if !defined(__PCINET_H__)
#include "pcinet.h"
#endif
#if !defined(__TBIT_H__)
#include "tbit.h"
#endif

/*---------------------  Static Definitions -------------------------*/

/*---------------------  Static Classes  ----------------------------*/

/*---------------------  Static Variables  --------------------------*/

/*---------------------  Static Functions  --------------------------*/

/*---------------------  Export Variables  --------------------------*/
int g_IO_point = PCI_base_addr;
int g_Mem_point = PCI_base_addr + PCI_io_range;

BOOL NPCIbIsRegBitsOn(HANDLE hHandle, WORD wBusDevFunId, BYTE byRegOffset, BYTE byBits)
{
	BYTE byOrgData;

	PCIvReadConfigB(hHandle, wBusDevFunId, byRegOffset, &byOrgData);
	return BITbIsAllBitsOn(byOrgData, byBits);
}

BOOL NPCIbIsRegBitsOff(HANDLE hHandle, WORD wBusDevFunId, BYTE byRegOffset, BYTE byBits)
{
	BYTE byOrgData;

	PCIvReadConfigB(hHandle, wBusDevFunId, byRegOffset, &byOrgData);
	return BITbIsAllBitsOff(byOrgData, byBits);
}

void NPCIvRegBitsOn(HANDLE hHandle, WORD wBusDevFunId, BYTE byRegOffset, BYTE byBits)
{
	BYTE byOrgData;

	PCIvReadConfigB(hHandle, wBusDevFunId, byRegOffset, &byOrgData);
	PCIvWriteConfigB(hHandle, wBusDevFunId, byRegOffset, (BYTE)(byOrgData | byBits));
}

void NPCIvRegBitsOff(HANDLE hHandle, WORD wBusDevFunId, BYTE byRegOffset, BYTE byBits)
{
	BYTE byOrgData;

	PCIvReadConfigB(hHandle, wBusDevFunId, byRegOffset, &byOrgData);
	PCIvWriteConfigB(hHandle, wBusDevFunId, byRegOffset, (BYTE)(byOrgData & ~byBits));
}

void NPCIvReadManyConfigSpace(HANDLE hHandle, WORD wBusDevFunId, BYTE byRegOffset,
	PBYTE pbyBuffer, WORD wCount)
{
	PCIvReadConfigManyBytes(hHandle, wBusDevFunId, byRegOffset, pbyBuffer, wCount);
}

void NPCIvReadRevisionId(HANDLE hHandle, WORD wBusDevFunId, PBYTE pbyRevId)
{
	PCIvReadConfigB(hHandle, wBusDevFunId, PCI_REG_REV_ID, pbyRevId);
}

/*
 * Description: Read PCI information by Vendor's id and Device's id
 *
 * Parameters:
 *
 * Return Value:
 *
 */
BOOL NPCIbReadDeviceInfo(HANDLE hHandle, WORD wVendorID, WORD wDeviceID, PSPciDevice pPciDevs)
{
	BYTE    byBusNum;
	BYTE    bySlotNum;
	BYTE    byFuncNum;
	BYTE    byRevId;
	DWORD   dwIoBase;
	DWORD   dwIoSpaceRange;
	BYTE    byIrqNo;
	DWORD   dwDevVenID = MAKEDWORD(wVendorID, wDeviceID);

	/* init device number to 0 */
	pPciDevs->wDeviceNum = 0;
	byBusNum = 0;
	bySlotNum = 0;
	byFuncNum = 0;
	/* get each Bus/Device/Function no. by vendor & device id */
	while (PCIbFindConfigDeviceInfo(dwDevVenID, &byBusNum, &bySlotNum, &byFuncNum, &byRevId,
	&dwIoBase, &dwIoSpaceRange, &byIrqNo)) {
		pPciDevs->awBusDevFunID[pPciDevs->wDeviceNum] =
			MAKE_BDF_TO_WORD(byBusNum, bySlotNum, byFuncNum);
		(pPciDevs->wDeviceNum)++;

		/* increase bySlotNum or byFuncNum to do the next search */
		/* by checking PCI header type bit 7 */
		if (NPCIbIsRegBitsOn(
			hHandle,
			MAKE_BDF_TO_WORD(byBusNum, bySlotNum, 0),
			PCI_REG_HDR_TYPE,
			0x80)
		)
			byFuncNum++;
		else
			bySlotNum++;
	}

	return TRUE;
}

void NPCIvInitialize(HANDLE hHandle, WORD wBusDevFunId, BYTE byRevId)
{
	/* PATCH.... turn this on to avoid retry forever */
	NPCIvRegBitsOn(hHandle, wBusDevFunId, PCI_REG_MODE2, MODE2_PCEROPT);
	/* PATCH.... */
	/* for some legacy BIOS and OS don't open BusM */
	/* bit in PCI configuration space. So, turn it on. */
	NPCIvRegBitsOn(hHandle, wBusDevFunId, PCI_REG_COMMAND, COMMAND_BUSM);
	/* PATCH.... turn this on to detect MII coding error */
	/* NPCIvRegBitsOn(hHandle, wBusDevFunId, PCI_REG_MODE3, MODE3_MIION); */
}

/* add by kevin */
/* search all pci devices matched vender & device ID */
BOOL NPCIbPCIAlloc(HANDLE hHandle, WORD wVendorID, WORD wDeviceID, PSPciDevice pPciDevs)
{
	BYTE    byBusNum;
	BYTE    bySlotNum;
	BYTE    byFuncNum;
	BYTE    byRevId;
	DWORD   dwIoBase;
	DWORD   dwIoSpaceRange;
	BYTE    byIrqNo;
	DWORD   dwDevVenID = MAKEDWORD(wVendorID, wDeviceID);
	int i;
	WORD command;
	DWORD space_size;

	/* init device number to 0 */
	pPciDevs->wDeviceNum = 0;
	byBusNum = 0;
	bySlotNum = 0;
	byFuncNum = 0;
	/* get each Bus/Device/Function no. by vendor & device id */
	while (PCIbFindConfigDeviceInfo(dwDevVenID, &byBusNum, &bySlotNum, &byFuncNum, &byRevId,
	&dwIoBase, &dwIoSpaceRange, &byIrqNo)) {
		pPciDevs->awBusDevFunID[pPciDevs->wDeviceNum] =
			MAKE_BDF_TO_WORD(byBusNum, bySlotNum, byFuncNum);
		/* disable IO/Mem accesss */
		PCIXvReadW(pPciDevs->awBusDevFunID[pPciDevs->wDeviceNum], PCI_REG_COMMAND, &command);
		command = (~0x3)&command;
		PCIXvWriteW(pPciDevs->awBusDevFunID[pPciDevs->wDeviceNum], PCI_REG_COMMAND, command);

		/* allocate IO or Memory space */
		for (i = 0; i < 6; i++) {
			/* judge whether size equals to 0 */
			PCIXvWriteD(
				pPciDevs->awBusDevFunID[pPciDevs->wDeviceNum],
				PCI_REG_BAR0+i*4,
				PCI_invalid
			);
			PCIXvReadD(pPciDevs->awBusDevFunID[pPciDevs->wDeviceNum],
				PCI_REG_BAR0+i*4,
				&space_size
			);

			if (space_size != 0) {
				if (space_size & PCI_IO_ENABLE) {
					/* assign IO space */
					/* assume io space doesn't exceed 256 bytes */
					PCIXvWriteD(
						pPciDevs->awBusDevFunID[pPciDevs->wDeviceNum],
						PCI_REG_BAR0+i*4,
						g_IO_point
					);
					g_IO_point += PCI_io_page;
				} else {
					/* assign memory space */
					/* assume mem space doesn't exceed 4k bytes */
					PCIXvWriteD(
						pPciDevs->awBusDevFunID[pPciDevs->wDeviceNum],
						PCI_REG_BAR0+i*4,
						g_Mem_point
					);
					g_Mem_point += PCI_memory_page;
				}
			}
		}
		/* 2.assign PCI slot interrupt */
		/* slot  8-a:44 */
		/* slot  9-a:45 */
		/* slot 10-a:46 */
		/* slot 11-a:47 */
		if (bySlotNum == 8)
			PCIXvWriteB(pPciDevs->awBusDevFunID[pPciDevs->wDeviceNum], PCI_REG_INT_LINE, 44);
		else if (bySlotNum == 9)
			PCIXvWriteB(pPciDevs->awBusDevFunID[pPciDevs->wDeviceNum], PCI_REG_INT_LINE, 45);
		else if (bySlotNum == 0xa)
			PCIXvWriteB(pPciDevs->awBusDevFunID[pPciDevs->wDeviceNum], PCI_REG_INT_LINE, 46);
		else if (bySlotNum == 0xb)
			PCIXvWriteB(pPciDevs->awBusDevFunID[pPciDevs->wDeviceNum], PCI_REG_INT_LINE, 47);
		else
			PCIXvWriteB(pPciDevs->awBusDevFunID[pPciDevs->wDeviceNum], PCI_REG_INT_LINE, 44);

		/* enable IO/Mem accesss and PCI master */
		PCIXvReadW(pPciDevs->awBusDevFunID[pPciDevs->wDeviceNum], PCI_REG_COMMAND, &command);
		command |= (0x7);
		PCIXvWriteW(pPciDevs->awBusDevFunID[pPciDevs->wDeviceNum], PCI_REG_COMMAND, command);

		pPciDevs->wDeviceNum++;
		/* increase bySlotNum or byFuncNum to do the next search */
		/* by checking PCI header type bit 7 */
		if (NPCIbIsRegBitsOn(
			hHandle,
			MAKE_BDF_TO_WORD(byBusNum, bySlotNum, 0),
			PCI_REG_HDR_TYPE,
			0x80)
		)
			byFuncNum++;
		else
			bySlotNum++;
	}
	/* printf("Richard DBG3\n"); */
	return TRUE;
}

void pciconfigurationdump(WORD wBusDevFunId)
{
	unsigned long s_c;

	PCIXvReadD(wBusDevFunId, 0, &s_c);
	printf("pci ID:%8x\n", s_c);
	PCIXvReadD(wBusDevFunId, 4, &s_c);
	printf("pci status and command:%8x\n", s_c);
	PCIXvReadD(wBusDevFunId, 16, &s_c);
	printf("pci io space:%8x\n", s_c);
	PCIXvReadD(wBusDevFunId, 20, &s_c);
	printf("pci mem spae:%8x\n", s_c);
	return;
}
