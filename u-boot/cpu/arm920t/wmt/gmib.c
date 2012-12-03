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

#if !defined(__MAC_H__)
#include "mac.h"
#endif
#if !defined(__TBIT_H__)
#include "tbit.h"
#endif
#if !defined(__MIB_H__)
#include "mib.h"
#endif

/*---------------------  Static Definitions -------------------------*/

/*---------------------  Static Classes  ----------------------------*/

/*---------------------  Static Variables  --------------------------*/

/*---------------------  Static Functions  --------------------------*/

/*---------------------  Export Variables  --------------------------*/

/*---------------------  Export Functions  --------------------------*/

VOID GSTAvClearAllCounter(PSStatCounter pStatistic)
{
	/* set memory to zero */
	MEMvSet(pStatistic, 0, sizeof(SStatCounter));
}

VOID GSTAvClearAllSwMibCounter(PSMib2Counter pMib2Counter)
{
	/* set memory to zero */
	/* Not work ??? */
	/*ZERO_MEMORY(pMib2Counter, sizeof(PSMib2Counter)); */

	pMib2Counter->ifOutOctets = 0;
	pMib2Counter->ifOutNUcastPkts = 0;
	pMib2Counter->ifOutUcastPkts = 0;
	pMib2Counter->ifOutDiscards = 0;
	pMib2Counter->ifOutErrors = 0;
}

VOID GSTAvUpdateEsrStatCounter(PSStatCounter pStatistic, BYTE byEsr, BOOL bTx)
{
	if (bTx) {
		if (BITbIsBitOn(byEsr, TXESR_TDSTR))
			pStatistic->dwTxEsrTDStrErr++;
		if (BITbIsBitOn(byEsr, TXESR_TDRBS))
			pStatistic->dwTxEsrTDRBusErr++;
		if (BITbIsBitOn(byEsr, TXESR_TDWBS))
			pStatistic->dwTxEsrTDWBusErr++;
		if (BITbIsBitOn(byEsr, TXESR_TFDBS))
			pStatistic->dwTxEsrDmaBusErr++;
	} else {
		if (BITbIsBitOn(byEsr, RXESR_RDSTR))
			pStatistic->dwRxEsrRDStrErr++;
		if (BITbIsBitOn(byEsr, RXESR_RDRBS))
			pStatistic->dwRxEsrRDRBusErr++;
		if (BITbIsBitOn(byEsr, RXESR_RDWBS))
			pStatistic->dwRxEsrRDWBusErr++;
		if (BITbIsBitOn(byEsr, RXESR_RFDBS))
			pStatistic->dwRxEsrDmaBusErr++;
	}
}

VOID GSTAvUpdateIsrStatCounter(PSStatCounter pStatistic, DWORD dwIsr, WORD wEsr)
{
	/*
	 * ABNORMAL interrupt
	 * not any IMR bit invoke irq
	 */
	if (dwIsr == 0) {
		pStatistic->dwIsrUnknown++;
		return;
	}

	if (BITbIsBitOn(dwIsr, ISR_PRXI)) {
		pStatistic->dwIsrRxOK++;
		/* PATCH.... */
		/* if rx OK, reset packet-race counter */
		pStatistic->dwIsrContinuePktRace = 0;
		/* if rx OK, reset no-buffer counter */
		pStatistic->dwIsrContinueNoBuf = 0;
	}
	if (BITbIsBitOn(dwIsr, ISR_PTXI))
		pStatistic->dwIsrTxOK++;

	if (BITbIsBitOn(dwIsr, ISR_PPRXI))
		pStatistic->dwIsrPRxOK++;
	if (BITbIsBitOn(dwIsr, ISR_PPTXI))
		pStatistic->dwIsrPTxOK++;

	if (BITbIsBitOn(dwIsr, ISR_TXWB0I))
		pStatistic->dwIsrTxWB0I++;
	if (BITbIsBitOn(dwIsr, ISR_TXWB1I))
		pStatistic->dwIsrTxWB1I++;

	if (BITbIsBitOn(dwIsr, ISR_RACEI)) {
		pStatistic->dwIsrRxPktRace++;
		/* PATCH.... */
		/* if irq continuous invoked by packet-race, plus counter */
		pStatistic->dwIsrContinuePktRace++;
	}
	if (BITbIsBitOn(dwIsr, ISR_LSTEI)) {
		pStatistic->dwIsrRxNoBuf++;
		/* PATCH.... */
		/* if irq continuous invoked by no-buffer, plus counter */
		pStatistic->dwIsrContinueNoBuf++;
	}

	if (BITbIsBitOn(dwIsr, ISR_OVFI))
		pStatistic->dwIsrRxFifoOvfl++;
	if (BITbIsBitOn(dwIsr, ISR_FLONI))
		pStatistic->dwIsrRxFlowOn++;

	if (BITbIsBitOn(dwIsr, ISR_LSTPEI))
		pStatistic->dwIsrRxNoBufP++;
	if (BITbIsBitOn(dwIsr, ISR_SRCI))
		pStatistic->dwIsrLinkStatusChg++;

	/* update counters for MISR */
	if (BITbIsBitOn(dwIsr, ISR_TMR0I))
		pStatistic->dwMisrSoftTimer0++;
	if (BITbIsBitOn(dwIsr, ISR_TMR1I))
		pStatistic->dwMisrSoftTimer1++;
	if (BITbIsBitOn(dwIsr, ISR_PWEI))
		pStatistic->dwMisrPWE++;
	if (BITbIsBitOn(dwIsr, ISR_PHYI))
		pStatistic->dwMisrPhyStatChg++;
	if (BITbIsBitOn(dwIsr, ISR_SHDNI))
		pStatistic->dwMisrShutDown++;
	if (BITbIsBitOn(dwIsr, ISR_MIBFI))
		pStatistic->dwMisrMibOvfl++;
	if (BITbIsBitOn(dwIsr, ISR_UDPI))
		pStatistic->dwMisrUserDef++;

	if (BITbIsBitOn(dwIsr, ISR_TXSTLI)) {
		pStatistic->dwMisrTxDmaErr++;
		GSTAvUpdateEsrStatCounter(pStatistic, (BYTE)wEsr, TRUE);
	}
	if (BITbIsBitOn(dwIsr, ISR_RXSTLI)) {
		pStatistic->dwMisrRxDmaErr++;
		GSTAvUpdateEsrStatCounter(pStatistic, (BYTE)(wEsr>>8), FALSE);
	}
}

VOID GSTAvUpdateRDStatCounter(
	PSStatCounter   pStatistic,
	PSRxDesc        prdCurr,
	PBYTE           pbyBuffer,
	UINT            cbFrameLength
	)
{
	pbyBuffer = pbyBuffer;
	BYTE    byRSR0 = prdCurr->m_rd0RD0.byRSR0;
	BYTE    byRSR1 = prdCurr->m_rd0RD0.byRSR1;

	/* increase rx packet count */
	pStatistic->dwRsrRxPacket++;
	pStatistic->dwRsrRxOctet += cbFrameLength;

	if (BITbIsBitOn(byRSR0, RSR0_CRC))
		pStatistic->dwRsrCRCErr++;
	if (BITbIsBitOn(byRSR0, RSR0_FAE))
		pStatistic->dwRsrFrmAlgnErr++;
	if (BITbIsBitOn(byRSR0, RSR0_CE))
		pStatistic->dwRsrCheckSumErr++;
	if (BITbIsBitOn(byRSR0, RSR0_RL))
		pStatistic->dwRsrLengthErr++;
	if (BITbIsBitOn(byRSR0, RSR0_RXER))
		pStatistic->dwRsrPCSSymErr++;
	if (BITbIsBitOn(byRSR0, RSR0_SNTAG))
		pStatistic->dwRsrSNTAG++;
	if (BITbIsBitOn(byRSR0, RSR0_DETAG))
		pStatistic->dwRsrDETAG++;

	if (BITbIsBitOn(byRSR1, RSR1_RXOK))
		pStatistic->dwRsrOK++;

	if (BITbIsBitOn(byRSR1, RSR1_VTAG))
		pStatistic->dwRsrRxTagFrame = (DWORD)(prdCurr->m_rd1RD1.wPQTAG);
	else
		pStatistic->dwRsrRxTagFrame = 0x0;

	if (BITbIsBitOn(byRSR1, RSR0_VIDM))
		pStatistic->dwRsrVIDMiss++;

	if (BITbIsBitOn(byRSR1, RSR1_VTAG))
		pStatistic->dwRsrRxTag++;

	if (BITbIsBitOn(byRSR1, RSR1_PFT))
		pStatistic->dwRsrPerfectMatch++;

	if (cbFrameLength >= U_ETHER_ADDR_LEN) {
		/*
		if (IS_BROADCAST_ADDRESS(pbyBuffer))
		pStatistic->dwRsrBroadcast++;
		else if (IS_MULTICAST_ADDRESS(pbyBuffer))
		pStatistic->dwRsrMulticast++;
		else
		pStatistic->dwRsrDirected++;
		*/
		if (BITbIsBitOn(byRSR1, RSR1_BAR))
			pStatistic->dwRsrBroadcast++;
		if (BITbIsBitOn(byRSR1, RSR1_MAR))
			pStatistic->dwRsrMulticast++;
		if (BITbIsBitOn(byRSR1, RSR1_PHY))
			pStatistic->dwRsrDirected++;
	}

	if (cbFrameLength == MIN_PACKET_LEN + 4)
		pStatistic->dwRsrRxFrmLen64++;
	else if ((65 <= cbFrameLength) && (cbFrameLength <= 127))
		pStatistic->dwRsrRxFrmLen65_127++;
	else if ((128 <= cbFrameLength) && (cbFrameLength <= 255))
		pStatistic->dwRsrRxFrmLen128_255++;
	else if ((256 <= cbFrameLength) && (cbFrameLength <= 511))
		pStatistic->dwRsrRxFrmLen256_511++;
	else if ((512 <= cbFrameLength) && (cbFrameLength <= 1023))
		pStatistic->dwRsrRxFrmLen512_1023++;
	else if ((1024 <= cbFrameLength) && (cbFrameLength <= MAX_PACKET_LEN + 4))
		pStatistic->dwRsrRxFrmLen1024_1518++;
}

VOID GSTAvUpdateRDStatCounterEx(
	PSStatCounter   pStatistic,
	PSRxDesc        prdCurr,
	PBYTE           pbyBuffer,
	UINT            cbFrameLength,
	DWORD           dwHwCRC32,
	DWORD           dwSwCRC32
	)
{
	GSTAvUpdateRDStatCounter(pStatistic, prdCurr, pbyBuffer, cbFrameLength);

	/* rx length */
	pStatistic->dwCntRxFrmLength = cbFrameLength;
	/* Rx Hardware generate CRC32 */
	pStatistic->dwHwCRC32 = dwHwCRC32;
	/* Rx Software generate CRC32 */
	pStatistic->dwSwCRC32 = dwSwCRC32;

	/* DRV32.LIB [1.05] */
	if ((dwHwCRC32 != dwSwCRC32) &&
	(dwSwCRC32 != 0x00000000UL))
		pStatistic->dwCntRxDataErr++;

	/* rx pattern, we just see 16 bytes for sample */
	MEMvCopy((PVOID)pStatistic->abyCntRxPattern, (PVOID)pbyBuffer, 16);
}

VOID GSTAvUpdateTDStatCounter(
	PSStatCounter   pStatistic,
	PSMib2Counter   pMib2Counter,
	PSTxDesc        ptdCurr,
	PBYTE           pbyBuffer,
	UINT            cbFrameLength
	)
{
	BYTE    byTSR0 = ptdCurr->m_td0TD0.byTSR0;
	BYTE    byTSR1 = ptdCurr->m_td0TD0.byTSR1;

	/* increase tx packet count */
	pStatistic->dwTsrTxPacket++;
	pStatistic->dwTsrTxOctet += cbFrameLength;
	pMib2Counter->ifOutOctets += cbFrameLength;

	if (BITbIsBitOn(byTSR0, TSR0_COLS)) {
		pStatistic->dwTsrCollision++;
		pStatistic->dwTsrTotalColRetry += (byTSR0 & 0x0F);

		if ((byTSR0 & 0x0F) == 1)
			pStatistic->dwTsrOnceCollision++;
		else
			pStatistic->dwTsrMoreThanOnceCollision++;
	}

	if (BITbIsBitOff(byTSR1, TSR1_TERR | TSR1_LNKFL | TSR1_CRS | TSR1_CDH))
		pStatistic->dwTsrOK++;
	else {
		if (BITbIsBitOn(byTSR0, TSR0_ABT))
			pStatistic->dwTsrAbort++;
		if (BITbIsBitOn(byTSR0, TSR0_OWC))
			pStatistic->dwTsrLateCollision++;
		if (BITbIsBitOn(byTSR0, TSR0_OWT))
			pStatistic->dwTsrJumboAbort++;

		if (BITbIsBitOn(byTSR1, TSR1_CDH))
			pStatistic->dwTsrHeartBeat++;
		if (BITbIsBitOn(byTSR1, TSR1_CRS))
			pStatistic->dwTsrCarrierLost++;
		if (BITbIsBitOn(byTSR1, TSR1_LNKFL))
			pStatistic->dwTsrLNKFL++;
		if (BITbIsBitOn(byTSR1, TSR1_SHDN))
			pStatistic->dwTsrShutDown++;
		if (BITbIsBitOn(byTSR1, TSR1_TERR))
			pStatistic->dwTsrErr++;
	}

	if (cbFrameLength >= U_ETHER_ADDR_LEN) {
		if (IS_BROADCAST_ADDRESS(pbyBuffer)) {
			pStatistic->dwTsrBroadcast++;
			pMib2Counter->ifOutNUcastPkts++;
		} else if (IS_MULTICAST_ADDRESS(pbyBuffer)) {
			pStatistic->dwTsrMulticast++;
			pMib2Counter->ifOutNUcastPkts++;
		} else {
			pStatistic->dwTsrDirected++;
			pMib2Counter->ifOutUcastPkts++;
		}
	}
}

VOID GSTAvUpdateTDStatCounterEx(
	PSStatCounter   pStatistic,
	PSMib2Counter   pMib2Counter,
	PSTxDesc        ptdCurr,
	PBYTE           pbyBuffer,
	DWORD           cbFrameLength
	)
{
	UINT   uPktLength = (UINT)cbFrameLength;

	GSTAvUpdateTDStatCounter(pStatistic, pMib2Counter, ptdCurr, pbyBuffer, uPktLength);

	/* tx length */
	pStatistic->dwCntTxBufLength = uPktLength;
	/* tx pattern, we just see 16 bytes for sample */
	MEMvCopy((PVOID)pStatistic->abyCntTxPattern, (PVOID)pbyBuffer, 16);
}

VOID GSTAvUpdateTDStatCounterTq0(
	PSStatCounter   pStatistic,
	PSTxDesc        ptdCurr,
	PBYTE           pbyBuffer,
	UINT            cbFrameLength
	)
{
	BYTE    byTSR0 = ptdCurr->m_td0TD0.byTSR0;
	BYTE    byTSR1 = ptdCurr->m_td0TD0.byTSR1;
	UINT    uPktLength = (UINT)cbFrameLength;

	/* tx length */
	pStatistic->dwCntTxBufLength = uPktLength;
	/* tx pattern, we just see 16 bytes for sample */
	MEMvCopy((PVOID)pStatistic->abyCntTxPattern, (PVOID)pbyBuffer, 16);

	/* increase tx packet count */
	pStatistic->dwTsrTxPacket++;
	pStatistic->dwTsrTxOctet += cbFrameLength;

	if (BITbIsBitOn(byTSR0, TSR0_COLS)) {
		pStatistic->dwTsrCollision++;
		pStatistic->dwTsrTotalColRetry += (byTSR0 & 0x0F);

		if ((byTSR0 & 0x0F) == 1)
			pStatistic->dwTsrOnceCollision++;
		else
			pStatistic->dwTsrMoreThanOnceCollision++;
	}

	if (BITbIsBitOff(byTSR1, TSR1_TERR | TSR1_LNKFL | TSR1_CRS | TSR1_CDH))
		pStatistic->dwTsrOK++;
	else {
		if (BITbIsBitOn(byTSR0, TSR0_ABT))
			pStatistic->dwTsrAbort++;
		if (BITbIsBitOn(byTSR0, TSR0_OWC))
			pStatistic->dwTsrLateCollision++;
		if (BITbIsBitOn(byTSR0, TSR0_OWT))
			pStatistic->dwTsrJumboAbort++;

		if (BITbIsBitOn(byTSR1, TSR1_CDH))
			pStatistic->dwTsrHeartBeat++;
		if (BITbIsBitOn(byTSR1, TSR1_CRS))
			pStatistic->dwTsrCarrierLost++;
		if (BITbIsBitOn(byTSR1, TSR1_LNKFL))
			pStatistic->dwTsrLNKFL++;
		if (BITbIsBitOn(byTSR1, TSR1_SHDN))
			pStatistic->dwTsrShutDown++;
		if (BITbIsBitOn(byTSR1, TSR1_TERR))
			pStatistic->dwTsrErr++;
	}

	if (cbFrameLength >= U_ETHER_ADDR_LEN) {
		if (IS_BROADCAST_ADDRESS(pbyBuffer))
			pStatistic->dwTsrBroadcast++;
		else if (IS_MULTICAST_ADDRESS(pbyBuffer))
			pStatistic->dwTsrMulticast++;
		else
			pStatistic->dwTsrDirected++;
	}
}

VOID GSTAvClearHWMIBCounter(DWORD dwIoBase)
{
	MACvRegBitsOn(dwIoBase, MAC_REG_MIBCR, MIBCR_MIBFRZ);
	MACvRegBitsOn(dwIoBase, MAC_REG_MIBCR, MIBCR_MIBCLR);
	/* wait until MIB counter clear done */
	while (TRUE) {
		if (GMACbIsRegBitsOff(dwIoBase, MAC_REG_MIBCR, MIBCR_MIBCLR))
			break;
	}
}

VOID GSTAvEnableHWMIBCounter(DWORD dwIoBase)
{
	MACvRegBitsOff(dwIoBase, MAC_REG_MIBCR, MIBCR_MIBFRZ);
}

VOID GSTAvDisableHWMIBCounter(DWORD dwIoBase)
{
	MACvRegBitsOn(dwIoBase, MAC_REG_MIBCR, MIBCR_MIBFRZ);
}

VOID GSTAvUpdateHWMIBCounter(DWORD dwIoBase, PSHWMibCounter psHWCounters)
{
	DWORD   dwTmp = 0;
	PDWORD  pdwTmp = (PDWORD)psHWCounters;
	int ii;
	/* flush real-time counter value to MIB SRAM */
	MACvRegBitsOn(dwIoBase, MAC_REG_MIBCR, MIBCR_MIBFLSH);

	/* wait until flush done */
	while (TRUE) {
		if (GMACbIsRegBitsOff(dwIoBase, MAC_REG_MIBCR, MIBCR_MIBFLSH))
			break;
	}

	/* return MIB pointer to 0 */
	MACvRegBitsOn(dwIoBase, MAC_REG_MIBCR, MIBCR_MPTRINI);

	for (ii = 0; ii < MAX_HW_MIB_COUNTER; ii++, pdwTmp++) {
		VNSvInPortD(dwIoBase + MAC_REG_MIBREAD, &dwTmp);
		*pdwTmp += dwTmp & 0x00FFFFFFUL;
	}
}
