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

#if !defined(__ALLOCT_H__)
#include "alloct.h"
#endif
#if !defined(__MAC_H__)
#include "mac.h"
#endif

/*---------------------  Static Definitions -------------------------*/

/*---------------------  Static Classes  ----------------------------*/

/*---------------------  Static Variables  --------------------------*/

/*---------------------  Static Functions  --------------------------*/

static BOOL ALCbAllocateTdrBufferPool(PSAdapterInfo pAdapter, SAllocMap asMaps[], UINT cbTD, int iSegNo)
{
	/* asMaps[iSegNo].dwRawSize = CB_MAX_BUF_SIZE*cbTD + pAdapter->dwCacheLineSize; */
	/* asMaps[iSegNo].dwSize    = CB_MAX_BUF_SIZE*cbTD; */
	asMaps[iSegNo].dwRawSize = MAX_PACKET_LEN*cbTD + pAdapter->dwCacheLineSize;
	asMaps[iSegNo].dwSize    = MAX_PACKET_LEN*cbTD;
	GMEMvAllocateShared(&asMaps[iSegNo]);

	if (asMaps[iSegNo].dwRawVAddr == 0)
		return FALSE;

	/* make data buf 4 dwords alignment for convenience */
	/* MEMvAlign(&asMaps[iSegNo], 16); */

	/* Tx buffers, make it byte-alignment */
	if (asMaps[iSegNo].dwRawVAddr % 2 != 0) {
		asMaps[iSegNo].dwVAddr = asMaps[iSegNo].dwRawVAddr + 1;
		asMaps[iSegNo].dwPAddr = LODWORD(asMaps[iSegNo].qwRawPAddr) + 1;
	} else {
		asMaps[iSegNo].dwVAddr = asMaps[iSegNo].dwRawVAddr;
		asMaps[iSegNo].dwPAddr = LODWORD(asMaps[iSegNo].qwRawPAddr);
	}

	return TRUE;
}

/*---------------------  Export Variables  --------------------------*/
PVOID
GMEMvAllocate(
	UINT    uCount
	)
{
	PVOID   pvMemAddr;

	pvMemAddr = g_pvGBufferIndex;
	g_pvGBufferIndex += uCount;
	return pvMemAddr;
}
VOID
GMEMvAllocateShared(
	PSAllocMap pamMem
	)
{
	/* printf("pamMem->dwRawVAddr1 = %x\n",pamMem->dwRawVAddr); */
	pamMem->dwRawVAddr = (DWORD)GMEMvAllocate((size_t)pamMem->dwRawSize * (size_t)sizeof(BYTE));
	/* If allocation failed, virtual/phisical address == NULL */
	if ((PVOID)pamMem->dwRawVAddr == NULL) {
		LODWORD(pamMem->qwRawPAddr) = 0;
		HIDWORD(pamMem->qwRawPAddr) = 0;
		return;
	}
	/*
	else {
		memset((PVOID)pamMem->dwRawVAddr, 0, (size_t)pamMem->dwRawSize * (size_t)sizeof(BYTE));
	}
	*/
	LODWORD(pamMem->qwRawPAddr) = pamMem->dwRawVAddr;
	HIDWORD(pamMem->qwRawPAddr) = 0;
}

BOOL GALCbAllocateRdrMemory(PSAdapterInfo pAdapter)
{
	/* Allocate the Receive Descriptor ring */
	/* (Physically contiguous and non cached) */
	pAdapter->amRxDescRing.dwRawSize = sizeof(SRxDesc) * pAdapter->cbRD + pAdapter->dwCacheLineSize;
	pAdapter->amRxDescRing.dwSize = sizeof(SRxDesc) * pAdapter->cbRD;
	GMEMvAllocateShared(&pAdapter->amRxDescRing);
	if (pAdapter->amRxDescRing.dwRawVAddr == 0)
		return FALSE;

	/* descriptor MUST aligned by 16 dword */
	MEMvAlign(&pAdapter->amRxDescRing, (UINT)pAdapter->dwCacheLineSize);
	/* allocate contiguous data buffers for RD ring */
	pAdapter->amRxDescBuf.dwRawSize = CB_MAX_BUF_SIZE*pAdapter->cbRD + pAdapter->dwCacheLineSize;
	pAdapter->amRxDescBuf.dwSize    = CB_MAX_BUF_SIZE*pAdapter->cbRD;

	GMEMvAllocateShared(&pAdapter->amRxDescBuf);

	if (pAdapter->amRxDescBuf.dwRawVAddr == 0)
		return FALSE;

	/* make data buf 4 dwords alignment for convenience */
	MEMvAlign(&pAdapter->amRxDescBuf, 16);
	/* MEMvAlign(&pAdapter->amRxDescBuf, 1);  //byte alignment */

	return TRUE;
}

BOOL GALCbAllocateTdSegMemory(PSAdapterInfo pAdapter, int iSegNo)
{
	int i;

	/* for 4 TD rings */
	for (i = 0; i < CB_TD_RING_NUM; i++) {
		if (!ALCbAllocateTdrBufferPool(pAdapter, pAdapter->aaamTxDescBufPool[i],
		pAdapter->cbTD, iSegNo))
			return FALSE;
	}

	return TRUE;
}

BOOL GALCbAllocateTdrMemory(PSAdapterInfo pAdapter)
{
	int i;

	/* for 3119 allocating 4 TD rings */
	for (i = 0; i < CB_TD_RING_NUM; i++) {
		/* Allocate the transmit descriptor ring */
		/* (Physically contiguous and non cached) */
		pAdapter->aamTxDescRing[i].dwRawSize =
			sizeof(STxDesc) * pAdapter->cbTD + pAdapter->dwCacheLineSize;
		pAdapter->aamTxDescRing[i].dwSize = sizeof(STxDesc) * pAdapter->cbTD;

		GMEMvAllocateShared(&pAdapter->aamTxDescRing[i]);

		if (pAdapter->aamTxDescRing[i].dwRawVAddr == 0)
			return FALSE;

		/* descriptor MUST aligned by 16 dword */
		MEMvAlign(&pAdapter->aamTxDescRing[i], (UINT)pAdapter->dwCacheLineSize);

		/* for 3119, only allocate first data segment buffer pool for each TD queue */
		if (!GALCbAllocateTdSegMemory(pAdapter, 0))
			return FALSE;
	}

	return TRUE;
}

void GALCvChainRdrMemory(PSAdapterInfo pAdapter)
{
	int ii;
	PSRxDesc    pRD;
	DWORD       dwBufPAddr;

	/* initialize the receive descriptor poniter array and */
	/* each individual receive descriptor */
	pRD = (PSRxDesc)pAdapter->amRxDescRing.dwVAddr;
	dwBufPAddr = pAdapter->amRxDescBuf.dwPAddr;

	for (ii = 0; ii < pAdapter->cbRD; ii++) {
		pAdapter->apRD[ii] = pRD;

		/* init descriptor value */
		pRD->m_rd0RD0.f1Owner = B_OWNED_BY_CHIP;
		pRD->m_f15RxBufSize = (WORD)CB_MAX_BUF_SIZE;
		pRD->m_wRxBufAddrHi = LOWORD(HIDWORD(pAdapter->amRxDescBuf.qwRawPAddr));
		pRD->m_dwRxBufAddrLo = dwBufPAddr;
		pRD->m_f1IntCtlEn = 1;  /* enable RIC */

		/* (PBYTE)pRD += sizeof(SRxDesc); */
		pRD = (PSRxDesc)(pAdapter->amRxDescRing.dwVAddr + sizeof(SRxDesc)*(ii+1));
		dwBufPAddr = pAdapter->amRxDescBuf.dwPAddr + CB_MAX_BUF_SIZE*(ii+1);
	}
}

void dumprd(PSAdapterInfo pAdapter)
{
	return;
}

void GALCvChainTdrMemory(PSAdapterInfo pAdapter, int iSegNo)
{
	int i, ii;
	PSTxDesc    pTD;
	/* DWORD       dwBufPAddr; */

	/* For VT3119 4 TDs */
	/* Initialize the Transmit Descriptor pointer array & */
	/* the tx desc ring */
	for (i = 0; i < CB_TD_RING_NUM; i++) {
		pTD = (PSTxDesc)pAdapter->aamTxDescRing[i].dwVAddr;
		/* dwBufPAddr = pAdapter->aaamTxDescBufPool[i][iSegNo].dwPAddr; */

		/* set tx descriptors owner to host(0) */
		/* set TCPLS_SOF = 1, TCPLS_EOF = 1 (normal packet) */
		for (ii = 0; ii < pAdapter->cbTD; ii++) {
			pAdapter->aapTD[i][ii] = pTD;

			/* init descriptor value */
			/* init Owner & Que bit */
			pTD->m_td0TD0.f1Owner = B_OWNED_BY_HOST;
			pTD->m_asTxBufs[0].f1Que = B_TD_LIST_END;

			pTD->m_td1TD1.byTCR0 = TCR0_TIC;

			pTD->m_td0TD0.f14TxPktSize = MAX_PACKET_LEN;
			pTD->m_td1TD1.f2TCPLS = 0x03;

			/* set up data segment */
			pTD->m_td1TD1.f4CMDZ = (BYTE)(iSegNo + 2);
			pTD->m_asTxBufs[iSegNo].f14TxBufSize = MAX_PACKET_LEN;
			pTD->m_asTxBufs[iSegNo].wTxBufAddrHi =
			LOWORD(HIDWORD(pAdapter->aaamTxDescBufPool[i][iSegNo].qwRawPAddr));
			/* pTD->m_asTxBufs[iSegNo].dwTxBufAddrLo =
			pAdapter->aaamTxDescBufPool[i][iSegNo].dwPAddr + CB_MAX_BUF_SIZE*ii + (ii % 16); */
			pTD->m_asTxBufs[iSegNo].dwTxBufAddrLo =
			pAdapter->aaamTxDescBufPool[i][iSegNo].dwPAddr + MAX_PACKET_LEN*ii + (ii % 16);

			/* (PBYTE)pTD += sizeof(STxDesc); */
			pTD = (PSTxDesc)(pAdapter->aamTxDescRing[i].dwVAddr + sizeof(STxDesc)*(ii+1));
		}
		/* printf("addr0 = %x\n",pAdapter->aapTD[0][0]->m_asTxBufs[0].dwTxBufAddrLo); */
		/* printf("addr1 = %x\n",pAdapter->aapTD[0][1]->m_asTxBufs[0].dwTxBufAddrLo); */
	}

	/* init adapter's sturcture also, init value = MAX_PACKET_LEN */
	pAdapter->uTxPktSize = MAX_PACKET_LEN;

}

void dumptd(PSAdapterInfo pAdapter)
{
	int data;
	int i;
	int ii;
	for (ii = 0; ii < 32; ii++) {
		for (i = 0; i < 0x10; i += 4) {
			VNSvInPortD(pAdapter->aamTxDescRing[0].dwVAddr+i+ii*sizeof(STxDesc), &data);
			printf("TD %4x:%8x\n", i, data);
		}
	}
	return;
}

void GALCvFreeRdrMemory(PSAdapterInfo pAdapter)
{
	/* Free the receive descriptor ring. */
	if (pAdapter->amRxDescRing.dwRawVAddr != 0) {
		MEMvFreeShared(&pAdapter->amRxDescRing);
		pAdapter->amRxDescRing.dwRawVAddr = 0;
	}

	/* Free the Receive buffer pool */
	if (pAdapter->amRxDescBuf.dwRawVAddr != 0) {
		MEMvFreeShared(&pAdapter->amRxDescBuf);
		pAdapter->amRxDescBuf.dwRawVAddr = 0;
	}
}

void GALCvFreeTdrMemory(PSAdapterInfo pAdapter)
{
	int i, ii;

	/* Free all TD Rings */
	for (i = 0; i < CB_TD_RING_NUM; i++) {
		if (pAdapter->aamTxDescRing[i].dwRawVAddr != 0) {
			MEMvFreeShared(&pAdapter->aamTxDescRing[i]);
			pAdapter->aamTxDescRing[i].dwRawVAddr = 0;
		}
	}

	/* Free the Transmit Buffers */
	for (i = 0; i < CB_TD_RING_NUM; i++) {
		for (ii = 0; ii < CB_MAX_SEG_PER_PKT; ii++) {
			if (pAdapter->aaamTxDescBufPool[i][ii].dwRawVAddr != 0) {
				MEMvFreeShared(&pAdapter->aaamTxDescBufPool[i][ii]);
				pAdapter->aaamTxDescBufPool[i][ii].dwRawVAddr = 0;
			}
		}
	}
}

void GALCvFreeTdSegMemory(PSAdapterInfo pAdapter, int iSegNo)
{
	int i, ii;

	PSTxDesc    pTD;

	/* Free the Transmit Buffers for one segment */
	for (i = 0; i < CB_TD_RING_NUM; i++) {
		pTD = (PSTxDesc)pAdapter->aamTxDescRing[i].dwVAddr;

		if (pAdapter->aaamTxDescBufPool[i][iSegNo].dwRawVAddr != 0) {
			MEMvFreeShared(&pAdapter->aaamTxDescBufPool[i][iSegNo]);
			pAdapter->aaamTxDescBufPool[i][iSegNo].dwRawVAddr = 0;
		}

		for (ii = 0; ii < pAdapter->cbTD; ii++) {
			pTD->m_asTxBufs[iSegNo].f14TxBufSize = 0;
			pTD->m_asTxBufs[iSegNo].wTxBufAddrHi = 0;
			pTD->m_asTxBufs[iSegNo].dwTxBufAddrLo = 0;
			pTD = (PSTxDesc)(pAdapter->aamTxDescRing[i].dwVAddr + sizeof(STxDesc)*(ii+1));
		}
	}
}

void GALCvSetAllRdOwnerToMac(PSAdapterInfo pAdapter)
{
	int     ii;

	for (ii = pAdapter->cbRD - 1; ii >= 0; ii--)
		pAdapter->apRD[ii]->m_rd0RD0.f1Owner = B_OWNED_BY_CHIP;
}

void GALCvSetAllTdOwnerToMac(PSAdapterInfo pAdapter)
{
	int     i, ii;

	for (i = 0; i < CB_TD_RING_NUM; i++)
		for (ii = pAdapter->cbTD - 1; ii >= 0; ii--)
			pAdapter->aapTD[i][ii]->m_td0TD0.f1Owner = B_OWNED_BY_CHIP;
}

void GALCvSetDefaultRD(PSAdapterInfo pAdapter)
{
	/* set init RD value */
	GALCvChainRdrMemory(pAdapter);
}

void GALCvSetDefaultTD(PSAdapterInfo pAdapter, int iSegNo)
{
	/* set init TD value */
	GALCvChainTdrMemory(pAdapter, iSegNo);   /*chain first data segment */
}
