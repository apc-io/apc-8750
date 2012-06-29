/*++ 
Copyright (c) 2010 WonderMedia Technologies, Inc.

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software 
Foundation, either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details. You
should have received a copy of the GNU General Public License along with this
program. If not, see http://www.gnu.org/licenses/>.

WonderMedia Technologies, Inc.
10F, 529, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C.
--*/


#if !defined(__MEM_H__)
#include "mem.h"
#endif
#if !defined(__CARD_H__)
#include "card.h"
#endif
#if !defined(__ALLOCT_H__)
#include "alloct.h"
#endif
#include "macif.h"
#if !defined(__DBG_H__)
#include "dbg.h"
#endif

/*---------------------  Static Definitions -------------------------*/

/*---------------------  Static Classes  ----------------------------*/

/*---------------------  Static Variables  --------------------------*/

/*---------------------  Static Functions  --------------------------*/

/*---------------------  Export Variables  --------------------------*/

/*---------------------  Export Functions  --------------------------*/

BOOL
ALCbAllocateRdrMemory(
    PSAdapterInfo pAdapter
    )
{
	int     jj;

	/* Allocate the Receive Descriptor ring */
	/* (Physically contiguous and non cached) */
	pAdapter->amRxDescRing.dwRawSize = sizeof(SRxDesc) * pAdapter->cbRD + pAdapter->dwCacheLineSize;
	pAdapter->amRxDescRing.dwSize = sizeof(SRxDesc) * pAdapter->cbRD;
	/* allocate raw RD : add cacheline */
	MEMvAllocateShared(&pAdapter->amRxDescRing);
	MacDump(MACDBG_INFO, ("[ALCbAllocateRdrMemory] - pAdapter->amRxDescRing.dwRawVAddr : %p, size : %d\n",
		(PVOID)pAdapter->amRxDescRing.dwRawVAddr, pAdapter->amRxDescRing.dwRawSize));

	if (pAdapter->amRxDescRing.dwRawVAddr == 0)
		return FALSE;

	/* descriptor MUST aligned by 4 dword */
	MEMvAlign(&pAdapter->amRxDescRing, (UINT)pAdapter->dwCacheLineSize);

	for (jj = 0; jj < pAdapter->cbRD; jj++) {
		pAdapter->aamRxDescBuf[jj].dwRawSize = g_sOptions.uiBuffsize + pAdapter->dwCacheLineSize;
		pAdapter->aamRxDescBuf[jj].dwSize = g_sOptions.uiBuffsize;
		/* allocate Rx Buffer raw:add cacheline */
		MEMvAllocateShared(&pAdapter->aamRxDescBuf[jj]);
		if (pAdapter->aamRxDescBuf[jj].dwRawVAddr == 0) {
			printf("allocate RX Buffer failed\n ");
			return FALSE;
		}
		MEMvAlign(&pAdapter->aamRxDescBuf[jj], (UINT)pAdapter->dwCacheLineSize);
	}

	return TRUE;
}


BOOL
ALCbAllocateTdrMemory(
    PSAdapterInfo pAdapter
    )
{
	/* Allocate the transmit descriptor ring */
	/* (Physically contiguous and non cached) */
	pAdapter->amTxDescRing.dwRawSize = sizeof(STxDesc) * pAdapter->cbTD + pAdapter->dwCacheLineSize;
	pAdapter->amTxDescRing.dwSize = sizeof(STxDesc) * pAdapter->cbTD;
	MEMvAllocateShared(&pAdapter->amTxDescRing);
	MacDump(MACDBG_INFO, ("[ALCbAllocateTdrMemory] - pAdapter->amTxDescRing.dwRawVAddr : %p, size : %d\n",
		(PVOID)pAdapter->amTxDescRing.dwRawVAddr, pAdapter->amTxDescRing.dwRawSize));

	if (pAdapter->amTxDescRing.dwRawVAddr == 0)
		return FALSE;

	/* Descriptor MUST aligned by 4 dword */
	MEMvAlign(&pAdapter->amTxDescRing, (UINT)pAdapter->dwCacheLineSize);

	if (!ALCbAllocateTdrBuffer(pAdapter, pAdapter->aamTxDescBuf, pAdapter->cbTD)) {
		printf("allocte TX buffer failed\n");
		return FALSE;
	}
	return TRUE;
}

BOOL
ALCbAllocateTdrBuffer(
    PSAdapterInfo pAdapter,
    SAllocMap asMaps[],
    UINT    cbTD
    )
{
	int     ii;


	for (ii = 0; ii < cbTD; ii++) {
		asMaps[ii].dwRawSize = g_sOptions.uiBuffsize + pAdapter->dwCacheLineSize;
		asMaps[ii].dwSize = g_sOptions.uiBuffsize;

		MEMvAllocateShared(&asMaps[ii]);

		if (asMaps[ii].dwRawVAddr == 0)
			return FALSE;

		/* If allocated Tx buffers are double-word alignment, make it byte-alignment */
		if (asMaps[ii].dwRawVAddr % sizeof(DWORD) == 0) {
			asMaps[ii].dwVAddr = asMaps[ii].dwRawVAddr + (ii % 4);
			asMaps[ii].dwPAddr = LODWORD(asMaps[ii].qwRawPAddr) + (ii % 4);
		} else {
			asMaps[ii].dwVAddr = asMaps[ii].dwRawVAddr;
			asMaps[ii].dwPAddr = LODWORD(asMaps[ii].qwRawPAddr);
		}
	}

	return TRUE;
}

VOID
ALCvChainRdrMemory(
    PSAdapterInfo pAdapter
    )
{
	int     ii;
	PSRxDesc    pRD;

	/* Initialize the receive descriptor poniter array and */
	/* each individual receive descriptor */
	pRD = (PSRxDesc)pAdapter->amRxDescRing.dwVAddr;

	for (ii = 0; ii < pAdapter->cbRD; ii++) {
		pAdapter->apRD[ii] = pRD;

		/* Init descriptor value */
		pRD->m_rd0RD0.f15FrameLen = 0;
		pRD->m_rd0RD0.f1Owner = B_OWNED_BY_CHIP;
		pRD->m_rd1RD1.f15RxBufLen = CB_MAX_BUF_SIZE;

		/* Set default RD buffer size to 512 bytes if 3065 (for multi-Rx descs per packet) */
		pRD->m_rd1RD1.f15RxBufLen = CB_BUF_SIZE_65;

		pRD->m_dwRxBufferAddr = pAdapter->aamRxDescBuf[ii].dwPAddr;
		if (ii == (pAdapter->cbRD - 1))
			pRD->m_dwRxNextDescAddr = pAdapter->amRxDescRing.dwPAddr;
		else
			pRD->m_dwRxNextDescAddr = pAdapter->amRxDescRing.dwPAddr + sizeof(SRxDesc) * (ii + 1);

		pRD = (PSRxDesc)(pAdapter->amRxDescRing.dwVAddr + sizeof(SRxDesc) * (ii + 1));
	}
}

VOID
ALCvChainTdrMemory(
    PSAdapterInfo pAdapter
    )
{
	int     ii;
	PSTxDesc    pTD;

	/* Initialize the tx desc ring */
	/* Initialize the Transmit Descriptor pointer array */
	pTD = (PSTxDesc)pAdapter->amTxDescRing.dwVAddr;

	for (ii = 0; ii < pAdapter->cbTD; ii++) {
		pAdapter->apTD[ii] = pTD;
		/* Init descriptor value */
		pTD->m_td0TD0.f1Owner = B_OWNED_BY_HOST;
		/* modified by kevin:length =0 */
		pTD->m_td1TD1.f15TxBufLen = 0;
		pTD->m_td1TD1.f1Chain = 1;
		pTD->m_td1TD1.byTCR = TCR_IC | TCR_EDP | TCR_STP;

		pTD->m_dwTxBufferAddr = pAdapter->aamTxDescBuf[ii].dwPAddr;
		if (ii == (pAdapter->cbTD - 1))
			pTD->m_dwTxNextDescAddr = pAdapter->amTxDescRing.dwPAddr;
		else
			pTD->m_dwTxNextDescAddr = pAdapter->amTxDescRing.dwPAddr + sizeof(STxDesc) * (ii + 1);

		pTD = (PSTxDesc)(pAdapter->amTxDescRing.dwVAddr + sizeof(STxDesc) * (ii + 1));
	}

	/* Init adapter's sturcture also, init value = MAX_PACKET_LEN */
	pAdapter->uTxPktSize = MAX_PACKET_LEN;

}

VOID
ALCvFreeRdrMemory(
    PSAdapterInfo pAdapter
    )
{
	int     ii;

	/* Free the receive descriptor ring. */
	if (pAdapter->amRxDescRing.dwRawVAddr != 0)
		pAdapter->amRxDescRing.dwRawVAddr = 0;

	/* Free the Receive buffers */
	for (ii = 0; ii < pAdapter->cbRD; ii++) {
		if (pAdapter->aamRxDescBuf[ii].dwRawVAddr != 0)
			pAdapter->aamRxDescBuf[ii].dwRawVAddr = 0;
	}
}

VOID
ALCvFreeTdrMemory(
    PSAdapterInfo pAdapter
    )
{
	int     ii;

	/* Free the transmit descriptor ring */
	if (pAdapter->amTxDescRing.dwRawVAddr != 0)
		pAdapter->amTxDescRing.dwRawVAddr = 0;

	/* Free the Transmit Buffers */
	for (ii = 0; ii < pAdapter->cbTD; ii++) {
		if (pAdapter->aamTxDescBuf[ii].dwRawVAddr != 0)
			pAdapter->aamTxDescBuf[ii].dwRawVAddr = 0;
	}
}

VOID
ALCvSetDefaultRD(
    PSAdapterInfo pAdapter
    )
{
	/* set init RD value */
	ALCvChainRdrMemory(pAdapter);
}

VOID
ALCvSetDefaultTD(
    PSAdapterInfo pAdapter
    )
{
	int     ii;
	int     jj;
	PBYTE pbyBuf;

	DWORD adwPattern[4] = {
		0xFFFFFFFFL, 0x00000000L,
		0xAAAAAAAAL, 0x55555555L
	};

	/* Set init TD value */
	ALCvChainTdrMemory(pAdapter);

	/* Set tx descriptors owner to host(0) */
	/* Set IC = 1, EDP = 1, STP = 1 */
	for (ii = pAdapter->cbTD - 1; ii >= 0; ii--) {
		pbyBuf = (PBYTE)pAdapter->aamTxDescBuf[ii].dwVAddr;

		/* Packet length maybe multiples of 16 */
		jj = 0;
		while (jj < pAdapter->apTD[ii]->m_td1TD1.f15TxBufLen) {
			if (jj < pAdapter->apTD[ii]->m_td1TD1.f15TxBufLen - 8) {
				*(PDWORD)(pbyBuf + jj) = adwPattern[ii % 4];
				*(PDWORD)(pbyBuf + jj + 4) = adwPattern[ii % 4];
				jj += 8;
			} else {
				*(PBYTE)(pbyBuf + jj)  = (BYTE)adwPattern[ii % 4];
				jj++;
			}
		}
	}
}
