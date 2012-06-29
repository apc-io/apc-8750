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

#if !defined(__TBIT_H__)
#include "tbit.h"
#endif
#if !defined(__CARD_H__)
#include "card.h"
#endif
#if !defined(__ADAPTER_H__)
#include "adapter.h"
#endif
#if !defined(__ISR_H__)
#include "gisr.h"
#endif

/*---------------------  Static Definitions -------------------------*/

/*---------------------  Static Classes  ----------------------------*/

/*---------------------  Static Variables  --------------------------*/

/*---------------------  Static Functions  --------------------------*/

void static s_ISRvPacketReceive(PSAdapterInfo pAdapter);
void static s_ISRvPacketTransmit(PSAdapterInfo pAdapter, BYTE byTxQue);
/*---------------------  Export Variables  --------------------------*/
extern SAdapterOpts  sOptions;
/*---------------------  Export Functions  --------------------------*/
void ISRvIsrForNetwork_Card0(void)
{
	GISRvIsrForNetwork(&sg_aGAdapter[0]);
}

void ISRvIsrForNetwork_Card1(void)
{
	GISRvIsrForNetwork(&sg_aGAdapter[1]);
}

void ISRvIsrForNetwork_Card2(void)
{
	GISRvIsrForNetwork(&sg_aGAdapter[2]);
}

void ISRvIsrForNetwork_Card3(void)
{
	GISRvIsrForNetwork(&sg_aGAdapter[3]);
}

/*
 * Description: This routine is called when MAC interrupt system
 *
 *          NOTE : when into this routine something should be aware
 *                  1) interrupt has been disabled.
 *                  2) inside a critical section (time)
 *                  3) stack owned by unknown application
 *
 * Parameters:
 *
 */
/* Note:In vpost, irq is not enanle after entering ISR ,so it is so called non-nest interrupt */
void GISRvIsrForNetwork(PSAdapterInfo pAdapter)
{
	DWORD   dwOrgIMR;
	DWORD   dwOrgISR;
	WORD    wOrgESR;

	DWORD   dwIoBase;
	BYTE    byIrqLevel;


	/* led 1 */
	VNSvOutPortD(0xD811005C, 1);
	/* get IoBase address and IRQ level */
	dwIoBase = pAdapter->dwIoBase;
	byIrqLevel = pAdapter->byIrqLevel;

	/* preserve original IMR value */
	VNSvInPortD(dwIoBase + MAC_REG_IMR, &dwOrgIMR);

	/* read current ISR */
	MACvReadISR(dwIoBase, &dwOrgISR);

	/* read RXE_SR, TXE_SR */
	VNSvInPortW(dwIoBase + MAC_REG_TXE_SR, &wOrgESR);

	/*
	 * (1). if this is not our device's IRQ, then call original ISR and exit.
	 * (2). if this is our device's IRQ, then handle it and exit.
	 *      we do not call original ISR. Because if another device also invoke
	 *      the IRQ at the same time, then the IRQ will be invoked again after
	 *      we had exited this ISR routine.
	 */

	/* if is not our device's interrupt, */
	if ((dwOrgISR & dwOrgIMR) == 0)
		return;

	/* mask off IMR, prevent MAC's other interrupt to reentry */
	MACvIntDisable(dwIoBase);

	/* write 1s to clear TXESR and RXESR */
	VNSvOutPortW(dwIoBase + MAC_REG_TXE_SR, wOrgESR);

	/* write 1s to clear ISR active interrupts */
	MACvWriteISR(dwIoBase, dwOrgISR);

	/* prevent All 1's ISR */
	if (dwOrgISR == 0xFFFFFFFFUL)
		goto IntDone;

	/* SERVICE RD/TD */
	/* handle rx */
	s_ISRvPacketReceive(pAdapter);
	/* led 2 */
	VNSvOutPortD(0xD811005C, 2);

	/* handle tx */
	s_ISRvPacketTransmit(pAdapter, 0);
	/* led 3 */
	VNSvOutPortD(0xD811005C, 4);

	/* Must do this after doing rx/tx, cause ISR bit is slower */
	/* than RD/TD write back */
	/* update ISR counter */
	GSTAvUpdateIsrStatCounter(&pAdapter->scStatistic, dwOrgISR, wOrgESR);

	/* Read HWMIBCounter */
	if (BITbIsBitOn(dwOrgISR, ISR_MIBFI))
		GSTAvUpdateHWMIBCounter(pAdapter->dwIoBase, &pAdapter->sHWMibCounter);

	/*
	 * ERROR HANDLING
	 */

	/* if link change */
	if (BITbIsBitOn(dwOrgISR, ISR_SRCI)) {
		/* Enable MII Auto-Polling */
		GMACvEnableMiiAutoPoll(dwIoBase);

		/* check the cable link status */
		/* link OK */
		pAdapter->bLinkPass = GMACbIsCableLinkOk(dwIoBase);

		if (pAdapter->bLinkPass) {
			/* each time int invoked, this counter will plus 1 */
			pAdapter->scStatistic.dwIntLinkUp++;

			/* update duplex status */
			pAdapter->bFullDuplex = GMACbIsInFullDuplexMode(pAdapter->dwIoBase);

			/* update speed status */
			pAdapter->bSpeed1G = GMACbIsIn1GMode(pAdapter->dwIoBase, pAdapter->byRevId);
			pAdapter->bSpeed100M = GMACbIsIn100MMode(dwIoBase, pAdapter->byRevId);

			/* set flow control capability according to PHYSR0 register */
			if (GMIIbIsAutoNegotiationOn(pAdapter->dwIoBase, pAdapter->byRevId)) {
				/* enable/disable RX flow control */
				if (GMACbIsRegBitsOn(pAdapter->dwIoBase, MAC_REG_PHYSR0, PHYSR0_RXFLC))
					VNSvOutPortB(pAdapter->dwIoBase + MAC_REG_CR2_SET, CR2_FDXRFCEN);
				else
					VNSvOutPortB(pAdapter->dwIoBase + MAC_REG_CR2_CLR, CR2_FDXRFCEN);

				/* enable/disable TX flow control */
				if (GMACbIsRegBitsOn(pAdapter->dwIoBase, MAC_REG_PHYSR0, PHYSR0_TXFLC))
					VNSvOutPortB(pAdapter->dwIoBase + MAC_REG_CR2_SET, CR2_FDXTFCEN);
				else
					VNSvOutPortB(pAdapter->dwIoBase + MAC_REG_CR2_CLR, CR2_FDXTFCEN);
			}

			/* This is patch for VT3119A1                                      */
			/* Use TCR_TB2BDIS to prevent 1c_Reset failure in helf-duplex mode */
			/* Only use this in 10HD and 100HD                                 */
			if (pAdapter->byRevId < REV_ID_VT3216_A0) {
				if (!pAdapter->bSpeed1G) {
					if (pAdapter->bFullDuplex)
						MACvRegBitsOff(pAdapter->dwIoBase, MAC_REG_TCR, TCR_TB2BDIS);
					else
						MACvRegBitsOn(pAdapter->dwIoBase, MAC_REG_TCR, TCR_TB2BDIS);
				}
			}

			/* This is patch for VT3119/VT3216 */
			/* Patch HW MIB TXSQEErr Bug, this should only function in 10HD */
			if (pAdapter->byRevId < REV_ID_VT3284_A0) {
				if ((!pAdapter->bSpeed1G) &&
				(!pAdapter->bSpeed100M) &&
				(!pAdapter->bFullDuplex)) {
					/* Enable count TXSQEErr in 10HD */
					MACvRegBitsOff(pAdapter->dwIoBase, MAC_REG_TESTCFG, 0x80);
				} else {
					/* Disable count TXSQEErr in 10FD, 100HD, 100FD, 1G */
					MACvRegBitsOn(pAdapter->dwIoBase, MAC_REG_TESTCFG, 0x80);
				}
			}

		} else {
			/* link fail */
			/* each time int invoked, this counter will plus 1 */
			pAdapter->scStatistic.dwIntLinkDown++;
		}
	}

	if (BITbIsBitOn(dwOrgISR, ISR_LSTEI)) {
		/*
		 * if Rx no buffer error, don't do software reset
		 * 1. turn on owner bit in all Rx descriptor
		 * 2. point current Rx to first descriptor
		 * 3. Rx on
		 * 4. RX WAK
		 */
		/*
		for (ii = 0; ii < pAdapter->cbRD; ii++)
			pAdapter->apRD[ii]->m_rd0RD0.f1Owner = 1;
		MACvSetRxDescBaseLo32(pAdapter->dwIoBase, pAdapter->amRxDescRing.dwPAddr);
		pAdapter->uRxDequeDescIdx = pAdapter->idxRxPktStartDesc = 0;
		PCAvDelayByIO(200);
		MACvRxOn(pAdapter->dwIoBase);
		*/
	}

	/* if Rx/Tx DMA stall */
	if (BITbIsBitOn(dwOrgISR, ISR_RXSTLI)) {
		/*
		// TBD....
		// rx off -> sw reset -> start
		// reset the adapter
		if (pAdapter->byRevId < REV_ID_VT3065_A)
			MACbSafeSoftwareReset(pAdapter->dwIoBase, pAdapter->byRevId);
		// rx was shutdowned, turn on rx
		MACvRxOn(pAdapter->dwIoBase);
		MACvTransmit(pAdapter->dwIoBase);
		*/
	}

	if (BITbIsBitOn(dwOrgISR, ISR_TXSTLI)) {
		/*
		if (BITbIsBitOn(ptdCurr->m_td0TD0.byTSR1, TSR1_TBUFF)) {
			// chip may be shutdowned, so re-start it
			MACvTxOn(pAdapter->dwIoBase);
			MACvTxQueueWake(pAdapter->dwIoBase,(BYTE)(0x40>>byTxQue));
			// re-assert TDMD
			MACvTransmit(pAdapter->dwIoBase);
		}
		*/
	}

	/* TXESR */
	if (BITbIsBitOn((BYTE)(wOrgESR & 0x000F), TXESR_TDSTR))
		pAdapter->bTxOn = FALSE;
	if (BITbIsBitOn((BYTE)(wOrgESR & 0x000F), TXESR_TDRBS))
		pAdapter->bTxOn = FALSE;
	if (BITbIsBitOn((BYTE)(wOrgESR & 0x000F), TXESR_TDWBS))
		pAdapter->bTxOn = FALSE;
	if (BITbIsBitOn((BYTE)(wOrgESR & 0x000F), TXESR_TFDBS))
		pAdapter->bTxOn = FALSE;

	/* RXESR */
	if (BITbIsBitOn((BYTE)((wOrgESR & 0x00F0) >> 8), RXESR_RDSTR))
		;
	if (BITbIsBitOn((BYTE)((wOrgESR & 0x00F0) >> 8), RXESR_RDRBS))
		;
	if (BITbIsBitOn((BYTE)((wOrgESR & 0x00F0) >> 8), RXESR_RDWBS))
		;
	if (BITbIsBitOn((BYTE)((wOrgESR & 0x00F0) >> 8), RXESR_RFDBS))
		;

IntDone:
	/*
	 * disable CPU's int to prevent reentry: if next statement enable
	 * MAC's int, and MAC happen to invoke int will cause reentry,
	 * so disable CPU's int here, let RETI to enable CPU's int
	 */
	/* PCAvIntDisable(); */

	/* restore original IMR value */
	MACvIntEnable(dwIoBase, dwOrgIMR);
	/* led off */
	VNSvOutPortD(0xD811005C, 0);
}

void static s_ISRvPacketReceive(PSAdapterInfo pAdapter)
{
	void (CALLBACK * pisrCallBack)(PSAdapterInfo, PBYTE, UINT);

	UINT        uCurrDescIdx;
	PSRxDesc    pCurrRD;
	int         ii;
	UINT        uu;

	BYTE        byRsr0, byRsr1;
	UINT        uRxFreeRD = 0;
	UINT        uRxFreeRDIdx = pAdapter->uRxDequeDescIdx;
	UINT        FrameSize;

	DWORD       dwRxFifoLength;
	UINT        FrameSizeWithCRC;

	PBYTE   pbyRxBufAddr ;
	PBYTE   pbyHwCRC32 ;
	DWORD   dwHwCRC32 ;
	DWORD   dwSwCRC32 ;

	/* added by kevin to not upload to uper layer */
	pisrCallBack = NULL;

	if (pAdapter->byRevId >= REV_ID_VT3216_A0)
		dwRxFifoLength = 48896; /* 48*1024 - 32*8 */
	else
		dwRxFifoLength = 32512; /* 32*1024 - 32*8 */

	while (TRUE) {
		uCurrDescIdx = pAdapter->uRxDequeDescIdx;
		pCurrRD = pAdapter->apRD[uCurrDescIdx];
		byRsr0 = pCurrRD->m_rd0RD0.byRSR0;
		byRsr1 = pCurrRD->m_rd0RD0.byRSR1;
		FrameSizeWithCRC = pCurrRD->m_rd0RD0.f14RMBC;

		/* if the RD is owned by the chip, we are done. */
		if (pCurrRD->m_rd0RD0.f1Owner == B_OWNED_BY_CHIP)
			break;

		/* expected both STP and EDP are zero as single RD receive case */
		/* else it's multiple RD receive case */
		/* [STP, EDP] = [0,0] single RD received */
		/* [STP, EDP] = [1,0] first RD in multiple RD received */
		/* [STP, EDP] = [1,1] middle RD in multiple RD received */
		/* [STP, EDP] = [0,1] last RD in multiple RD received */
		if (BITbIsBitOn(byRsr1, (RSR1_STP | RSR1_EDP))) {
			/* Drop this RD, move to next RD */
			uRxFreeRD++;
			ADD_ONE_WITH_WRAP_AROUND(pAdapter->uRxDequeDescIdx, pAdapter->cbRD);

			/* [1.14], Calculate Rx Fifo Address */
			pAdapter->dwRxFifoAddr += FrameSizeWithCRC;
			if (pAdapter->dwRxFifoAddr >= dwRxFifoLength)
				pAdapter->dwRxFifoAddr -= dwRxFifoLength;

			/* branch code to while(TRUE) */
			continue;
		}

		/* increase handled rx counter */
		pAdapter->iRxPktHandled++;

		/* get the frame size */
		FrameSize = pCurrRD->m_rd0RD0.f14RMBC;

		/* DBG: Get hardware and software-generated CRC32 */
		pbyRxBufAddr = (PBYTE)pAdapter->amRxDescBuf.dwVAddr + uCurrDescIdx*CB_MAX_BUF_SIZE;
		pbyHwCRC32 = pbyRxBufAddr + (FrameSize-4);
		dwHwCRC32 = *((PDWORD)pbyHwCRC32);
		dwSwCRC32 = 0;

		if (pAdapter->bSoftwareCheckCrc) {
			/* TBD */
			/* calculate crc by software */
			PCAvIntDisable();
			dwSwCRC32 = CRCdwGetCrc32(pbyRxBufAddr, FrameSize - 4);
			PCAvIntEnable();
		} /* end if (pAdapter->bSoftwareCheckCrc) */


		/*
		 * handle this RD
		 */
		/* update receive statistic counter */
		GSTAvUpdateRDStatCounterEx(
			&pAdapter->scStatistic,
			pCurrRD,
			pbyRxBufAddr,
			FrameSize,
			dwHwCRC32,
			dwSwCRC32
		);

		if (pAdapter->bWhenRxDataErrOutPort0x80 && GMACbIsInLoopbackMode(pAdapter->dwIoBase)) {
			if (dwHwCRC32 != dwSwCRC32) {
				/* Stop TX if CRC32 inconsistent */
				pAdapter->bTxOn = FALSE;
				pAdapter->bShowLbPacketComp = TRUE;
			}
		}


		/*
		* RX ERROR
		*/
		if (BITbIsBitOff(byRsr1, RSR1_RXOK)) {
			if (BITbIsBitOn(byRsr0, RSR0_CRC) && pAdapter->bWhenRxCrcErrNoBuf) {
				/* goto IPR_NoReleaseOwner; */
				/* Stop RX */
				GMACbSafeRxOff(pAdapter->dwIoBase, pAdapter->byRevId);
				break;
			}

			/* Drop this RD, move to next RD */
			uRxFreeRD++;
			ADD_ONE_WITH_WRAP_AROUND(pAdapter->uRxDequeDescIdx, pAdapter->cbRD);

			/* [1.16], Calculate Rx Fifo Address */
			pAdapter->dwRxFifoAddr += FrameSizeWithCRC;
			/*
			if (pAdapter->dwRxFifoAddr >= pAdapter->dwRxFifoSize)
				pAdapter->dwRxFifoAddr -= pAdapter->dwRxFifoSize;
			*/
			if (pAdapter->dwRxFifoAddr >= dwRxFifoLength)
				pAdapter->dwRxFifoAddr -= dwRxFifoLength;

			/* branch code to while(TRUE) */
			continue;
		}


		/*
		 * RX OK
		 */

		/* for pingpong: copy buffer to upper layer buffer */
		/*because we don't need,we mark the function */
		/*
		pisrCallBack = (void (CALLBACK *)(PSAdapterInfo, PBYTE, UINT))pAdapter->pisrPptReceiver;
		if (pisrCallBack != NULL) {
			(*pisrCallBack)(
				pAdapter,
				pbyRxBufAddr,
				FrameSize
			);
		}
		*/

		/* Release this RD */
		uRxFreeRD++;
		ADD_ONE_WITH_WRAP_AROUND(pAdapter->uRxDequeDescIdx, pAdapter->cbRD);

		/* [1.16], Calculate Rx Fifo Address */
		pAdapter->dwRxFifoAddr += FrameSizeWithCRC;
		/*
		if (pAdapter->dwRxFifoAddr >= pAdapter->dwRxFifoSize) {
			pAdapter->dwRxFifoAddr -= pAdapter->dwRxFifoSize;
		}
		*/
		if (pAdapter->dwRxFifoAddr >= dwRxFifoLength)
			pAdapter->dwRxFifoAddr -= dwRxFifoLength;
	} /* end while (TRUE) */


	/* batch append RD here (4X RDs appended each time) */
	/* count the difference between uRxDequeDescIdx and uRxAppendDescIdx */
	/* why we should release RD wit 4 multiple */
	if ((pAdapter->cbToBeFreeRD + uRxFreeRD) >= 4) {
		/* calculate the number of RD to be free, it must be 4X */
		ii = (pAdapter->cbToBeFreeRD + uRxFreeRD) - ((pAdapter->cbToBeFreeRD + uRxFreeRD) % 4);
		/* decrease the RD index */
		SUB_ONE_WITH_WRAP_AROUND(uRxFreeRDIdx, pAdapter->cbRD);
		/* calculate the end RD index to be free in this time */
		ADD_N_WITH_WRAP_AROUND(uRxFreeRDIdx, (ii - pAdapter->cbToBeFreeRD), pAdapter->cbRD);

		for (uu = 0; uu < ii; uu++) {
			pAdapter->apRD[uRxFreeRDIdx]->m_rd0RD0.f1Owner = B_OWNED_BY_CHIP;
			SUB_ONE_WITH_WRAP_AROUND(uRxFreeRDIdx, pAdapter->cbRD);
		}

		/* update RXRDU in FlowCR0 */
		VNSvOutPortB(pAdapter->dwIoBase + MAC_REG_RBRDU, (BYTE)ii);

		/* calculate to-be-free RD for next time */
		pAdapter->cbToBeFreeRD = (pAdapter->cbToBeFreeRD + uRxFreeRD) % 4;
	} else {
		/* calculate to-be-free RD for next time */
		pAdapter->cbToBeFreeRD += uRxFreeRD;
	}
}

void static s_ISRvPacketTransmit(PSAdapterInfo pAdapter, BYTE byTxQue)
{
	PSTxDesc    ptdCurr;
	int         ii;
	WORD        wPktSize;
	BOOL        bIsTxISR = FALSE;
	UINT        uCurrTdIdx;

	DWORD       dwTxFifoLength;
	PBYTE       pbyBuffer;

	if (pAdapter->byRevId >= REV_ID_VT3216_A0)
		dwTxFifoLength = 16384; /* 16*1024 */
	else
		dwTxFifoLength = 10240; /* 10*1024 */

	while (TRUE) {
		uCurrTdIdx = pAdapter->uTxDequeDescIdx[byTxQue];
		ptdCurr = pAdapter->aapTD[byTxQue][uCurrTdIdx];

		/* If the ring is empty or the current descriptor */
		/* is not owned by the system, we are done. */
		if (ptdCurr->m_td0TD0.f1Owner == B_OWNED_BY_CHIP)
			break;

		if (pAdapter->auTxPktPosted[byTxQue] == 0)
			break;

		/* it's Interrupt for Tx */
		bIsTxISR = TRUE;

		/* if there is a number limit to tx */
		/* pAdapter->lTxPacketNum==0 means, tx number is un-limited */
		if (pAdapter->lTxPacketNum != 0) {
			/* decrease lTxPacketNumShadow */
			pAdapter->lTxPacketNumShadow--;

			/* if exceed the tx number */
			if (pAdapter->lTxPacketNumShadow == 0)
				pAdapter->bTxOn = FALSE;
		}

		/* calculate the total buffer size */
		wPktSize = 0;
		for (ii = 0; ii < ptdCurr->m_td1TD1.f4CMDZ - 1; ii++)
			wPktSize += ptdCurr->m_asTxBufs[ii].f14TxBufSize;

		/*
		 * handle this TD
		 */

		/* update transmit statistic counter */
		GSTAvUpdateTDStatCounterEx(
			&pAdapter->ascStatistic[byTxQue],
			&pAdapter->mibCounter,
			ptdCurr,
			(PBYTE)(pAdapter->aapTD[byTxQue][uCurrTdIdx]->m_asTxBufs[0].dwTxBufAddrLo),
			/* wPktSize */
			ptdCurr->m_asTxBufs[0].f14TxBufSize + ptdCurr->m_asTxBufs[1].f14TxBufSize +
			ptdCurr->m_asTxBufs[2].f14TxBufSize + ptdCurr->m_asTxBufs[3].f14TxBufSize +
			ptdCurr->m_asTxBufs[4].f14TxBufSize + ptdCurr->m_asTxBufs[5].f14TxBufSize +
			ptdCurr->m_asTxBufs[6].f14TxBufSize
		);

		if (byTxQue == 0) { /* default TD queue */
			GSTAvUpdateTDStatCounterTq0(
				&pAdapter->scStatistic,  /* for diag tool counter display */
				ptdCurr,
				(PBYTE)(pAdapter->aapTD[byTxQue][uCurrTdIdx]->m_asTxBufs[0].dwTxBufAddrLo),
				/* wPktSize */
				ptdCurr->m_asTxBufs[0].f14TxBufSize + ptdCurr->m_asTxBufs[1].f14TxBufSize +
				ptdCurr->m_asTxBufs[2].f14TxBufSize + ptdCurr->m_asTxBufs[3].f14TxBufSize +
				ptdCurr->m_asTxBufs[4].f14TxBufSize + ptdCurr->m_asTxBufs[5].f14TxBufSize +
				ptdCurr->m_asTxBufs[6].f14TxBufSize
			);
		}

		/*
		 * TX OK
		 */
		if (BITbIsBitOff(ptdCurr->m_td0TD0.byTSR1, TSR1_TERR)) {
			/* decrease posted tx packet counter */
			pAdapter->auTxPktPosted[byTxQue]--;
		} else {
			/*
			 * TX ERROR HANDLING
			 */

			if (pAdapter->bWhenTxErrOutPort0x80) {
				PCBvOutPortB(0x80, 0xAA);
				/* PCBvOutPortB(0x80, 0x00); */

				pAdapter->bTxOn = FALSE;
			}

			/* (1). tx abort */

			/* tx abort don't do retry */
			if (BITbIsBitOn(ptdCurr->m_td0TD0.byTSR0, TSR0_ABT)) {
				/* chip may be shutdowned, so re-start it */
				MACvTxOn(pAdapter->dwIoBase);
				MACvTxQueueRUN(pAdapter->dwIoBase, byTxQue);
				MACvTxQueueWake(pAdapter->dwIoBase, byTxQue);
			}

			/* decrease posted tx packet counter */
			pAdapter->auTxPktPosted[byTxQue]--;
		} /* end Tx error handling */

		/* clear td status */
		ptdCurr->m_td0TD0.byTSR0 = 0;
		ptdCurr->m_td0TD0.byTSR1 = 0;
		/* VT3119A1 must clear Quebit, MAC doesn't write back QueBit */
		ptdCurr->m_asTxBufs[0].f1Que = 0;

		/* move DequeTdIdx to next */
		pAdapter->uTxDequeDescIdx[byTxQue] = (uCurrTdIdx + 1) % pAdapter->cbTD;

		/* [1.14], Calculate Tx Fifo Address */
		pAdapter->dwTxFifoAddr[byTxQue] += (
			8 + ptdCurr->m_asTxBufs[0].f14TxBufSize + ptdCurr->m_asTxBufs[1].f14TxBufSize +
			ptdCurr->m_asTxBufs[2].f14TxBufSize + ptdCurr->m_asTxBufs[3].f14TxBufSize +
			ptdCurr->m_asTxBufs[4].f14TxBufSize + ptdCurr->m_asTxBufs[5].f14TxBufSize +
			ptdCurr->m_asTxBufs[6].f14TxBufSize
		);

		if (pAdapter->dwTxFifoAddr[byTxQue] >= dwTxFifoLength)
			pAdapter->dwTxFifoAddr[byTxQue] -= dwTxFifoLength;

		/* for specific purpose we add owner bit automatically */
		if (pAdapter->bTxContPerfTest &&
		pAdapter->bTxOn &&
		!pAdapter->bTxSinglePacket) {

			/* Set this TD */
			UINT    uTxPktSize;
			if (pAdapter->bRandomPktLength) {
				/* TBD */
				UINT    uRandomPktSize = MAX_PACKET_LEN ; /* RANDOM(MAX_PACKET_LEN) + 1; */

				/* ??? */
				/* In loopback test, hardware wouldn't generate 4-byte CRC,
				 * software make the minumum TX buffer size to be 64 bytes
				 * to avoid generating runt packet
				 */
				/*
				if (uRandomPktSize < (MIN_PACKET_LEN + 4)) {
				// TBD.... + CRC 4 bytes
				uRandomPktSize = MIN_PACKET_LEN + 4;
				}
				*/
				if (pAdapter->bSoftwareGenCrc) {
					if (uRandomPktSize < (MIN_PACKET_LEN + 4))
						uRandomPktSize = MIN_PACKET_LEN + 4;
				} else {
					if (uRandomPktSize < MIN_PACKET_LEN)
						uRandomPktSize = MIN_PACKET_LEN;
				}
				/*
				if (uRandomPktSize > MAX_PACKET_LEN) {
				uRandomPktSize = MAX_PACKET_LEN;
				}
				*/
				uTxPktSize = uRandomPktSize;
			} else if (pAdapter->bIncPktLength) {
				uTxPktSize = pAdapter->uTxPktSize;

				/* increase packet length */
				pAdapter->uTxPktSize++;
				if (pAdapter->bSoftwareGenCrc) {
					if (pAdapter->uTxPktSize > (MAX_PACKET_LEN + 4))
						pAdapter->uTxPktSize = MIN_PACKET_LEN + 4;
				} else {
					if (pAdapter->uTxPktSize > MAX_PACKET_LEN)
						pAdapter->uTxPktSize = MIN_PACKET_LEN;
				}
			} else
				uTxPktSize = pAdapter->cbPktSize;

			/* [1.12] */
			pbyBuffer = (PBYTE)(ptdCurr->m_asTxBufs[0].dwTxBufAddrLo);

			if (pAdapter->bRandomPktData) {
				UINT    kk;
				UINT    uPosition;

				/* RANDOM DATA */
				/* Get random position in Random Table */
				/* TBD */
				uPosition = 3072 - uTxPktSize;/* RANDOM((3072 - uTxPktSize)); */
				for (kk = 0; kk < uTxPktSize; kk++)
					*(pbyBuffer + kk) = *((PBYTE)sg_abyRandomTable + uPosition + kk);
			}

			if (pAdapter->bSoftwareGenCrc) {
				PDWORD  pdwCRC = (PDWORD)(pbyBuffer + uTxPktSize - 4);
				DWORD   dwCRC;
				PCAvIntDisable();
				dwCRC = CRCdwGetCrc32(pbyBuffer, uTxPktSize - 4);
				*pdwCRC = dwCRC;
				PCAvIntEnable();
			}

			ptdCurr->m_td0TD0.f14TxPktSize = (WORD)uTxPktSize;

			/* Calculate each segment's buffer size */
			UINT    uSegNum;
			UINT    i;
			if (pAdapter->bRandDatSegsPerPacket) {
				/* TBD */
				uSegNum = CB_MAX_SEG_PER_PKT+1 ;/* RANDOM(CB_MAX_SEG_PER_PKT) + 1; */
			} else
			uSegNum = pAdapter->uTxDataSegsPerPacket;

			if (uSegNum > 1) {
				WORD    wTempBufSize;

				/* The BufLen of first Buf is 14 */
				ptdCurr->m_asTxBufs[0].f14TxBufSize = 14;

				/* The rest */
				/* Equal the buffer length */
				wTempBufSize = (WORD)((uTxPktSize - 14) / (uSegNum - 1));
				for (i = 1; i < uSegNum; i++) {
					if (i == (uSegNum - 1)) {
						ptdCurr->m_asTxBufs[i].f14TxBufSize =
							(WORD)((uTxPktSize - 14) -
							(wTempBufSize*(uSegNum - 2)));
					} else
						ptdCurr->m_asTxBufs[i].f14TxBufSize = wTempBufSize;
					ptdCurr->m_asTxBufs[i].dwTxBufAddrLo =
						ptdCurr->m_asTxBufs[i-1].dwTxBufAddrLo +
						ptdCurr->m_asTxBufs[i-1].f14TxBufSize;
				} /* end for */

				for ( ; i < CB_MAX_SEG_PER_PKT; i++) {
					ptdCurr->m_asTxBufs[i].f14TxBufSize = 0;
					ptdCurr->m_asTxBufs[i].dwTxBufAddrLo = 0;
				}
			} else {
				/* uSegNum = 1 */
				/* only one descriptor; it occupies the whole packet size */
				ptdCurr->m_asTxBufs[0].f14TxBufSize = (WORD)uTxPktSize;
				for (i = 1; i < CB_MAX_SEG_PER_PKT; i++) {
					ptdCurr->m_asTxBufs[i].f14TxBufSize = 0;
					ptdCurr->m_asTxBufs[i].dwTxBufAddrLo = 0;
				}
			} /* end if (uSegNum > 1) */
			ptdCurr->m_td1TD1.f4CMDZ = (BYTE)(uSegNum + 1);
			ptdCurr->m_td0TD0.f1Owner = B_OWNED_BY_CHIP;

			pAdapter->auTxPktPosted[byTxQue]++;

			/* Get last TD Idx */
			UINT    uLastTdIdx = (uCurrTdIdx - 1 + pAdapter->cbTD) % pAdapter->cbTD;
			if (pAdapter->aapTD[byTxQue][uLastTdIdx]->m_td0TD0.f1Owner == B_OWNED_BY_CHIP) {
				/* VT3119A0 */
				/* vSetBit((PDWORD)(pAdapter->aapTD[byTxQue][wLastTdIdx])); */
				/* VT3119A1 */
				pAdapter->aapTD[byTxQue][uLastTdIdx]->m_asTxBufs[0].f1Que = B_TD_LIST_NOT_END;
			}
		} /* end if */

	} /* end while (TRUE) */

	if (pAdapter->bTxContPerfTest &&
	pAdapter->bTxOn &&
	!pAdapter->bTxSinglePacket)
		MACvTxQueueWake(pAdapter->dwIoBase, byTxQue);
}

VOID
PCAvIntEnable(
    void
    )
{
	asm volatile ("MRS r0, CPSR\n");
	asm volatile ("BIC r0, r0, #0x80\n");
	asm volatile ("MSR CPSR_c, r0\n");
}

VOID
PCAvIntDisable(
    void
    )
{
	asm volatile ("MRS r0, CPSR\n");
	asm volatile ("ORR r0, r0, #0x80\n");
	asm volatile ("MSR CPSR_c, r0\n");
}
