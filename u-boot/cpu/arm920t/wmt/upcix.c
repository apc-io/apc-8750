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

#if !defined(__UASSERT_H__)
#include "uassert.h"
#endif
#if !defined(__UPC_H__)
#include "upc.h"
#endif

#if !defined(__TMACRO_H__)
#include "tmacro.h"
#endif
#if !defined(__TPCI_H__)
#include "tpci.h"
#endif
#if !defined(__UPCIX_H__)
#include "upcix.h"
#endif
#if !defined(__TBIT_H__)
#include "tbit.h"
#endif

/*---------------------  Static Definitions -------------------------*/

/* PCI configuration space IO port */
#define PCI_INDEX               0xC0000CF8
#define PCI_DATA                0xC0000CFC

/*---------------------  Static Classes  ----------------------------*/

/*---------------------  Static Variables  --------------------------*/

/*---------------------  Static Functions  --------------------------*/

/*---------------------  Export Variables  --------------------------*/

/*---------------------  Import Functions  --------------------------*/

VOID PCIXvReadB(WORD wBusDevFunId, BYTE byRegOffset, PBYTE pbyData)
{
	DWORD   dwIndex;

	dwIndex = wBusDevFunId;
	dwIndex = 0x80000000L | (dwIndex << 8) | (byRegOffset & 0xFC);

	PCBvOutPortD(PCI_INDEX, dwIndex);
	PCBvInPortB(PCI_DATA + (byRegOffset % 4), pbyData);
}

VOID PCIXvReadW(WORD wBusDevFunId, BYTE byRegOffset, PWORD pwData)
{
	DWORD   dwIndex;

	/* must word alignment */
	DBG_ASSERT((byRegOffset % 2) == 0);

	dwIndex = wBusDevFunId;
	dwIndex = 0x80000000L | (dwIndex << 8) | (byRegOffset & 0xFC);

	PCBvOutPortD(PCI_INDEX, dwIndex);
	PCBvInPortW(PCI_DATA + (byRegOffset % 4), pwData);
}

VOID PCIXvReadD(WORD wBusDevFunId, BYTE byRegOffset, PDWORD pdwData)
{
	DWORD   dwIndex;

	/* must dword alignment */
	DBG_ASSERT((byRegOffset % 4) == 0);

	dwIndex = wBusDevFunId;
	dwIndex = 0x80000000L | (dwIndex << 8) | (byRegOffset & 0xFC);

	PCBvOutPortD(PCI_INDEX, dwIndex);
	PCBvInPortD(PCI_DATA, pdwData);
}

VOID PCIXvWriteB(WORD wBusDevFunId, BYTE byRegOffset, BYTE byData)
{
	DWORD   dwIndex;

	dwIndex = wBusDevFunId;
	dwIndex = 0x80000000L | (dwIndex << 8) | (byRegOffset & 0xFC);

	PCBvOutPortD(PCI_INDEX, dwIndex);
	PCBvOutPortB(PCI_DATA + (byRegOffset % 4), byData);
}

VOID PCIXvWriteW(WORD wBusDevFunId, BYTE byRegOffset, WORD wData)
{
	DWORD   dwIndex;

	/* must word alignment */
	DBG_ASSERT((byRegOffset % 2) == 0);

	dwIndex = wBusDevFunId;
	dwIndex = 0x80000000L | (dwIndex << 8) | (byRegOffset & 0xFC);

	PCBvOutPortD(PCI_INDEX, dwIndex);
	PCBvOutPortW(PCI_DATA + (byRegOffset % 4), wData);
}

VOID PCIXvWriteD(WORD wBusDevFunId, BYTE byRegOffset, DWORD dwData)
{
	DWORD   dwIndex;

	/* must dword alignment */
	DBG_ASSERT((byRegOffset % 4) == 0);

	dwIndex = wBusDevFunId;
	dwIndex = 0x80000000L | (dwIndex << 8) | (byRegOffset & 0xFC);

	PCBvOutPortD(PCI_INDEX, dwIndex);
	PCBvOutPortD(PCI_DATA, dwData);
}

VOID PCIXvReadManyBytes(WORD wBusDevFunId, BYTE byRegOffset, PBYTE pbyBuffer, WORD wCount)
{
	WORD    wIdx;
	PBYTE   pbyTmpBuf = pbyBuffer;

	for (wIdx = 0; wIdx < wCount; wIdx++, pbyTmpBuf++)
		PCIXvReadB(wBusDevFunId, (BYTE)(byRegOffset + wIdx), pbyTmpBuf);
}

BOOL PCIXbFindDeviceInfo(DWORD dwDevVenID, PBYTE pbyBusNum,
		PBYTE pbySlotNum, PBYTE pbyFuncNum, PBYTE pbyRevId, PDWORD pdwIoBase,
		PDWORD pdwIoSpaceRange, PBYTE pbyIrqNo)
{
	DWORD       dwCurrDevVenID;
	BYTE        byBusNum = *pbyBusNum;
	BYTE        bySlotNum = *pbySlotNum;
	BYTE        byFuncNum = *pbyFuncNum;
	BOOL        bMultiFunc;
	BYTE        byData;
	int         i;

	for (i = byBusNum; i < MAX_PCI_BUS; i++) {
		byBusNum = (BYTE)i;

		for ( ; bySlotNum < MAX_PCI_DEVICE; bySlotNum++) {
			PCIXvReadB(MAKE_BDF_TO_WORD(byBusNum, bySlotNum, 0), PCI_REG_HDR_TYPE, &byData);
			bMultiFunc = BITbIsAllBitsOn(byData, 0x80);

			for ( ; byFuncNum < ((bMultiFunc) ? 8 : 1); byFuncNum++) {
				PCIXvReadD(MAKE_BDF_TO_WORD(byBusNum, bySlotNum, byFuncNum),
				PCI_REG_VENDOR_ID, &dwCurrDevVenID);

				/* is this our device? */
				if (dwCurrDevVenID == dwDevVenID) {
					PCIXvReadB(MAKE_BDF_TO_WORD(byBusNum, bySlotNum, byFuncNum),
					PCI_REG_REV_ID, pbyRevId);
					PCIXvReadD(MAKE_BDF_TO_WORD(byBusNum, bySlotNum, byFuncNum),
					PCI_REG_BAR0, pdwIoBase);
					*pdwIoBase = (*pdwIoBase) & 0xFFFFFFFEUL;

					/* IO space range: */
					/* 43:0x80(128B), 65:0x100(256B) */
					if (*pbyRevId < 0x40)
						*pdwIoSpaceRange = 0x00000080L;
					else
						*pdwIoSpaceRange = 0x00000100L;

					PCIXvReadB(MAKE_BDF_TO_WORD(byBusNum, bySlotNum, byFuncNum),
					PCI_REG_INT_LINE, pbyIrqNo);
					*pbyBusNum = byBusNum;
					*pbySlotNum = bySlotNum;
					*pbyFuncNum = byFuncNum;
					return TRUE;
				}
			}
			byFuncNum = 0;
		}
		bySlotNum = 0;
	}

	return FALSE;
}
