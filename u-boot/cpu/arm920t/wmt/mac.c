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


#if !defined(__UPC_H__)
#include "upc.h"
#endif
#if !defined(__TBIT_H__)
#include "tbit.h"
#endif
#if !defined(__MII_H__)
#include "mii.h"
#endif
#if !defined(__MAC_H__)
#include "mac.h"
#endif

/*---------------------  Static Definitions -------------------------*/
#define CB_DELAY_NM9346        (10)        /* 10ms */
#define CB_DELAY_SOFT_RESET    (50)        /* 50ms */

/*---------------------  Static Classes  ----------------------------*/

/*---------------------  Static Variables  --------------------------*/

/*---------------------  Static Functions  --------------------------*/

/*---------------------  Export Variables  --------------------------*/

/*---------------------  Export Functions  --------------------------*/
BOOL
MACbIsCableLinkOk(
    DWORD   dwIoBase
    )
{
    return MACbIsRegBitsOff(dwIoBase, MAC_REG_MIISR, MIISR_LNKFL);
}

BOOL
MACbIsIn100MMode(
	DWORD   dwIoBase,
	BYTE    byRevId
	)
{
	UINT ii;

	for (ii = 0; ii < 30; ii++)
		PCAvDelayByIO(CB_DELAY_LOOP_WAIT);

	/* NOTE.... */
	/* when link fail, MAC will report in 100M mode, */
	/* but NS DP83840A will report in 10M mode, DAVICOM DM9101 depends */
	return MIIbIsIn100MMode(dwIoBase, byRevId);
}

VOID
MACvIsInFullDuplexMode(
	PBOOL   pbFullDup,
	DWORD   dwIoBase
	)
{
	*pbFullDup = MACbIsRegBitsOn(dwIoBase, MAC_REG_CR1, CR1_FDX);
}

BOOL
MACbIsRegBitsOff(
	DWORD   dwIoBase,
	BYTE    byRegOfs,
	BYTE    byTestBits
	)
{
	BYTE byData;

	VNSvInPortB(dwIoBase + byRegOfs, &byData);
	return BITbIsAllBitsOff(byData, byTestBits);
}

BOOL
MACbIsRegBitsOn(
	DWORD   dwIoBase,
	BYTE    byRegOfs,
	BYTE    byTestBits
	)
{
	BYTE byData;

	VNSvInPortB(dwIoBase + byRegOfs, &byData);
	return BITbIsAllBitsOn(byData, byTestBits);
}

BOOL
MACbSafeSoftwareReset(
	DWORD   dwIoBase,
	BYTE    byRevId
	)
{
	BYTE    abyTmpRegData[0x100];
	BOOL    bRetVal;

	/* PATCH.... */
	/* save some important register's value, then do */
	/* reset, then restore register's value */

	/* save MAC context */
	MACvSaveContext(dwIoBase, byRevId, abyTmpRegData);
	/* do reset */
	bRetVal = MACbSoftwareReset(dwIoBase, byRevId);
	/* restore MAC context, except CR0 */
	MACvRestoreContext(dwIoBase, byRevId, abyTmpRegData);

	return bRetVal;
}

BOOL
MACbStop(
	DWORD   dwIoBase
	)
{
	WORD    ww;
	BYTE    byData;

	VNSvOutPortB(dwIoBase + MAC_REG_CR0, CR0_STOP);

	/* W_MAX_TIMEOUT is the timeout period */
	for (ww = 0; ww < W_MAX_TIMEOUT; ww++) {
		VNSvInPortB(dwIoBase + MAC_REG_CR0, &byData);
		if (BITbIsAllBitsOff(byData, CR0_TXON | CR0_RXON))
			break;
	}
	if (ww == W_MAX_TIMEOUT)
		return FALSE;

	return TRUE;
}

BOOL
MACbSafeStop(
	DWORD   dwIoBase,
	BYTE    byRevId
	)
{
	if (!MACbSafeTxOff(dwIoBase, byRevId))
		return FALSE;
	if (!MACbSafeRxOff(dwIoBase, byRevId))
		return FALSE;
	if (!MACbStop(dwIoBase))
		MACbSafeSoftwareReset(dwIoBase, byRevId);

	return TRUE;
}

BOOL
MACbSafeRxOff(
	DWORD   dwIoBase,
	BYTE    byRevId
	)
{
	WORD    ww;
	BYTE    byData;

	/* try to safe shutdown RX */
	MACvSetLoopbackMode(dwIoBase, MAC_LB_INTERNAL);
	MACvRegBitsOff(dwIoBase, MAC_REG_CR0, CR0_RXON);
	/* safe delay time */
	PCAvDelayByIO(CB_DELAY_SOFT_RESET);
	/* W_MAX_TIMEOUT is the timeout period */
	for (ww = 0; ww < W_MAX_TIMEOUT; ww++) {
		VNSvInPortB(dwIoBase + MAC_REG_CR0, &byData);
		if (BITbIsBitOff(byData, CR0_RXON))
			break;
	}
	if (ww == W_MAX_TIMEOUT) {
		/* enter FIFO test mode to issue RX reject */
		VNSvOutPortB(dwIoBase + MAC_REG_GFTEST, 0x01);
		VNSvOutPortB(dwIoBase + MAC_REG_RFTCMD, 0x08);
		PCAvDelayByIO(CB_DELAY_SOFT_RESET);
		VNSvOutPortB(dwIoBase + MAC_REG_GFTEST, 0x00);
	}

	MACvSetLoopbackMode(dwIoBase, MAC_LB_NONE);
	return TRUE;
}

BOOL
MACbSafeTxOff(
	DWORD   dwIoBase,
	BYTE    byRevId
	)
{
	WORD    ww;
	BYTE    byData;

	/* try to safe shutdown TX */
	MACvRegBitsOff(dwIoBase, MAC_REG_CR0, CR0_TXON);
	/* safe delay time */
	PCAvDelayByIO(CB_DELAY_SOFT_RESET);
	/* W_MAX_TIMEOUT is the timeout period */
	for (ww = 0; ww < W_MAX_TIMEOUT; ww++) {
		VNSvInPortB(dwIoBase + MAC_REG_CR0, &byData);
		if (BITbIsBitOff(byData, CR0_TXON))
			break;
	}
	if (ww == W_MAX_TIMEOUT)
		return FALSE;

	return TRUE;
}

BOOL
MACbSoftwareReset(
	DWORD   dwIoBase,
	BYTE    byRevId
	)
{
	WORD    ww;
	BYTE    byData;

	/* turn on CR1_SFRST */
	MACvRegBitsOn(dwIoBase, MAC_REG_CR1, CR1_SFRST);

	/* polling till software reset complete */
	/* W_MAX_TIMEOUT is the timeout period */
	for (ww = 0; ww < W_MAX_TIMEOUT; ww++) {
		VNSvInPortB(dwIoBase + MAC_REG_CR1, &byData);
		if (BITbIsBitOff(byData, CR1_SFRST))
			break;
	}
	if (ww == W_MAX_TIMEOUT) {
		/* turn on force reset */
		MACvRegBitsOn(dwIoBase, MAC_REG_MISC_CR1, MISC_CR1_FORSRST);
		/* delay 2ms */
		MACvTimer0MiniSDelay(dwIoBase, byRevId, 2);
	}

	return TRUE;
}

BYTE
MACbyReadEECSR(
	DWORD   dwIoBase
	)
{
	BYTE byData;

	VNSvInPortB(dwIoBase + MAC_REG_EECSR, &byData);
	return byData;
}

BYTE
MACbyReadMIICR(
	DWORD   dwIoBase
	)
{
	BYTE byData;

	VNSvInPortB(dwIoBase + MAC_REG_MIICR, &byData);
	return byData;
}

BYTE
MACbyReadMultiAddr(
	DWORD   dwIoBase,
	UINT    uByteIdx
	)
{
	BYTE byData;

	VNSvInPortB(dwIoBase + MAC_REG_MAR + uByteIdx, &byData);
	return byData;
}

VOID
MACvWriteMultiAddr(
	DWORD   dwIoBase,
	UINT    uByteIdx,
	BYTE    byData
	)
{
	VNSvOutPortB(dwIoBase + MAC_REG_MAR + uByteIdx, byData);
}

VOID
MACvInitialize(
	DWORD   dwIoBase,
	BYTE    byRevId
	)
{
	/* Clear sticky bits */
	MACvClearStckDS(dwIoBase);
	/* Disable force PME-enable */
	VNSvOutPortB(dwIoBase + MAC_REG_WOLCG_CLR, WOLCFG_PME_OVR);
	/* Disable power-event config bit */
	MACvPwrEvntDisable(dwIoBase);
	/* Clear power status */
	VNSvOutPortB(dwIoBase + MAC_REG_PWRCSR_CLR, 0xFF);
	VNSvOutPortB(dwIoBase + MAC_REG_PWRCSR1_CLR, 0x03);

	/* Do reset */
	MACbSoftwareReset(dwIoBase, byRevId);
	/* For AUTOLD be effect in VT3043, safe delay time */
	PCAvDelayByIO(CB_DELAY_SOFT_RESET);

	/*
	// Issue AUTOLD in EECSR to reload eeprom
	MACvRegBitsOn(dwIoBase, MAC_REG_EECSR, EECSR_AUTOLD);

	// Wait until EEPROM loading complete
	while (TRUE) {
		BYTE    byData;

		VNSvInPortB(dwIoBase + MAC_REG_EECSR, &byData);
		if (BITbIsBitOff(byData, EECSR_AUTOLD))
			break;
	}
	*/

	/* For 3065D, EEPROM reloaded will cause bit 0 in MAC_REG_CFGA turned on. */
	/* It makes MAC receive magic packet automatically. So, driver turn it off. */
	MACvRegBitsOff(dwIoBase, MAC_REG_CFGA, CFGA_LED0S0);

	/* Set rx-FIFO/tx-FIFO/DMA threshold */
	/* Set rx threshold, 64 bytes */
	MACvSetRxThreshold(dwIoBase, 0);
	/* Set tx threshold, 128 bytes */
	MACvSetTxThreshold(dwIoBase, 0);
	/* Set DMA length, 16 DWORDs = 64 bytes */
	MACvSetDmaLength(dwIoBase, 0);

	/* kevin wonder:Enable queue packet? or disable? */
	MACvRegBitsOff(dwIoBase, MAC_REG_CFGB, CFGB_QPKTDIS);
	/* kevin wonder:Suspend-well accept broadcast, multicast */
	VNSvOutPortB(dwIoBase + MAC_REG_WOLCG_SET, WOLCFG_SAM | WOLCFG_SAB);

	/* kevin wonder :Back off algorithm use original IEEE standard */
	MACvRegBitsOff(dwIoBase, MAC_REG_CFGD, CFGD_CRADOM | CFGD_CAP | CFGD_MBA | CFGD_BAKOPT);

	/* Set packet filter */
	/* Receive directed and broadcast address */
	MACvSetPacketFilter(dwIoBase, PKT_TYPE_DIRECTED | PKT_TYPE_BROADCAST);

	/* enable MIICR_MAUTO */
	MACvEnableMiiAutoPoll(dwIoBase);
	/* macinit */

	/* printf("mac init ok!\n"); */
}

VOID
MACvIntDisable(
	DWORD   dwIoBase,
	BYTE    byRevId
	)
{
	VNSvOutPortW(dwIoBase + MAC_REG_IMR, 0x0000);
	VNSvOutPortB(dwIoBase + MAC_REG_MIMR, 0x00);
}

VOID
MACvIntEnable(
	DWORD   dwIoBase,
	BYTE    byRevId,
	DWORD   dwMask
	)
{
	VNSvOutPortW(dwIoBase + MAC_REG_IMR, LOWORD(dwMask));
	VNSvOutPortB(dwIoBase + MAC_REG_MIMR, (BYTE)(HIWORD(dwMask)));
}

VOID
MACvEnableMiiAutoPoll(
	DWORD   dwIoBase
	)
{
	WORD    ww;
	BYTE    byData;

	VNSvOutPortB(dwIoBase + MAC_REG_MIICR, 0);
	VNSvOutPortB(dwIoBase + MAC_REG_MIIAD, 0x01);
	VNSvOutPortB(dwIoBase + MAC_REG_MIICR, MIICR_MAUTO);

	/* as soon as MDONE is on, MAUTO is really started */
	for (ww = 0; ww < W_MAX_TIMEOUT; ww++) {
		VNSvInPortB(dwIoBase + MAC_REG_MIIAD, &byData);
		if (BITbIsBitOn(byData, MIIAD_MDONE))
			break;
	}

	MACvRegBitsOn(dwIoBase, MAC_REG_MIIAD, MIIAD_MSRCEN);
}

VOID
MACvSafeDisableMiiAutoPoll(
    DWORD   dwIoBase,
    BYTE    byRevId
    )
{
	WORD    ww;
	BYTE    byData;

	/* turn off MAUTO */
	VNSvOutPortB(dwIoBase + MAC_REG_MIICR, 0);

	/* as soon as MIDLE is on, MAUTO is really stoped */
	for (ww = 0; ww < W_MAX_TIMEOUT; ww++) {
		VNSvInPortB(dwIoBase + MAC_REG_MIIAD, &byData);
		if (BITbIsBitOn(byData, MIIAD_MIDLE))
			break;
	}
}

VOID
MACvReadISR(
	DWORD   dwIoBase,
	BYTE    byRevId,
	PDWORD  pdwValue
	)
{
	WORD wData;
	BYTE byData;

	VNSvInPortW(dwIoBase + MAC_REG_ISR, &wData);
	VNSvInPortB(dwIoBase + MAC_REG_MISR, &byData);
	*pdwValue = MAKEDWORD(wData, MAKEWORD(byData, 0x00));
}

VOID
MACvWriteISR(
	DWORD   dwIoBase,
	BYTE    byRevId,
	DWORD   dwValue
	)
{
	/* NOTE.... must write MISR before ISR, otherwise GENI will not */
	/* be cleared */
	VNSvOutPortB(dwIoBase + MAC_REG_MISR, (BYTE)(HIWORD(dwValue)));
	VNSvOutPortW(dwIoBase + MAC_REG_ISR, LOWORD(dwValue));
}

VOID
MACvRestoreContext(
	DWORD   dwIoBase,
	BYTE    byRevId,
	PBYTE   pbyCxtBuf
	)
{
	int         ii;

	/* restore RCR,TCR,CR,ISR,IMR... */
	for (ii = MAC_REG_PAR; ii < MAC_REG_CUR_RD_ADDR; ii++) {
		/* except CR0, because we don't want to start chip now */
		if (ii != MAC_REG_CR0)
			VNSvOutPortB(dwIoBase + ii, *(pbyCxtBuf + ii));
	}

	/* restore CURR_RX_DESC_ADDR, CURR_TX_DESC_ADDR */
	for (ii = MAC_REG_CUR_RD_ADDR; ii < MAC_REG_CUR_TD_ADDR + 4; ii += 4)
		VNSvOutPortD(dwIoBase + ii, *(PDWORD)(pbyCxtBuf + ii));

	/* restore MIIAD... */
	for (ii = MAC_REG_MPHY; ii < MAC_REG_CNTR_MPA; ii++)
		VNSvOutPortB(dwIoBase + ii, *(pbyCxtBuf + ii));

	/* restore WOLCR, PWCFG, TestReg, WOLCG */
	for (ii = MAC_REG_WOLCR_SET; ii < MAC_REG_WOLCR_CLR; ii++)
		VNSvOutPortB(dwIoBase + ii, *(pbyCxtBuf + ii));
	/* restore pattern, mask */
	for (ii = MAC_REG_PATRN_CRC0; ii < MAC_REG_BYTEMSK3_3 + 4; ii += 4)
		VNSvOutPortD(dwIoBase + ii, *((PDWORD)(pbyCxtBuf + ii)));
}

VOID
MACvSaveContext(
	DWORD   dwIoBase,
	BYTE    byRevId,
	PBYTE   pbyCxtBuf
	)
{
	int         ii;

	/* save RCR,TCR,CR,ISR,IMR... */
	for (ii = MAC_REG_PAR; ii < MAC_REG_CUR_RD_ADDR; ii++)
		VNSvInPortB(dwIoBase + ii, (PBYTE)(pbyCxtBuf + ii));

	/* save CURR_RX_DESC_ADDR, CURR_TX_DESC_ADDR */
	for (ii = MAC_REG_CUR_RD_ADDR; ii < MAC_REG_CUR_TD_ADDR + 4; ii += 4)
		VNSvInPortD(dwIoBase + ii, (PDWORD)(pbyCxtBuf + ii));

	/* save MIIAD... */
	for (ii = MAC_REG_MPHY; ii < MAC_REG_CNTR_MPA; ii++)
		VNSvInPortB(dwIoBase + ii, (PBYTE)(pbyCxtBuf + ii));

	/* save WOLCR, PWCFG, TestReg, WOLCG */
	for (ii = MAC_REG_WOLCR_SET; ii < MAC_REG_WOLCR_CLR; ii++)
		VNSvInPortB(dwIoBase + ii, (PBYTE)(pbyCxtBuf + ii));
	/* save pattern, mask */
	for (ii = MAC_REG_PATRN_CRC0; ii < MAC_REG_BYTEMSK3_3 + 4; ii += 4)
		VNSvInPortD(dwIoBase + ii, (PDWORD)(pbyCxtBuf + ii));
}

VOID
MACvSetDmaLength(
	DWORD   dwIoBase,
	BYTE    byDmaLength
	)
{
	BYTE byOrgValue;

	/* set BCR0 */
	VNSvInPortB(dwIoBase + MAC_REG_BCR0, &byOrgValue);
	byOrgValue = (BYTE)((byOrgValue & 0xF8) | byDmaLength);
	VNSvOutPortB(dwIoBase + MAC_REG_BCR0, byOrgValue);
}

VOID
MACvSetDuplexMode(
	DWORD   dwIoBase,
	BYTE    byRevId,
	BOOL    bFullDuplexOn
	)
{
	if (bFullDuplexOn)
		MACvRegBitsOn(dwIoBase, MAC_REG_CR1, CR1_FDX);
	else
		MACvRegBitsOff(dwIoBase, MAC_REG_CR1, CR1_FDX);

	if (bFullDuplexOn)
		VNSvOutPortB(dwIoBase + MAC_REG_WOLCG_SET, WOLCFG_SFDX);
	else
		VNSvOutPortB(dwIoBase + MAC_REG_WOLCG_CLR, WOLCFG_SFDX);
}

VOID
MACvSetLoopbackMode(
	DWORD   dwIoBase,
	BYTE    byLoopbackMode
	)
{
	BYTE byOrgValue;

	/* set TCR */
	VNSvInPortB(dwIoBase + MAC_REG_TCR, &byOrgValue);
	byOrgValue = (BYTE)((byOrgValue & 0xF9) | (byLoopbackMode << 1));
	VNSvOutPortB(dwIoBase + MAC_REG_TCR, byOrgValue);
}

VOID
MACvSetPacketFilter(
	DWORD   dwIoBase,
	WORD    wFilterType
	)
{
	BYTE    byOldRCR;
	BYTE    byNewRCR = 0;

	/* if only in DIRECTED mode, multicast-address will set to zero, */
	/* but if other mode exist (e.g. PROMISCUOUS), multicast-address */
	/* will be open */
	if (BITbIsBitOn(wFilterType, PKT_TYPE_DIRECTED)) {
		/* set multicast address to accept none */
		VNSvOutPortD(dwIoBase + MAC_REG_MAR, 0L);
		VNSvOutPortD(dwIoBase + MAC_REG_MAR + sizeof(DWORD), 0L);
	}

	if (BITbIsAnyBitsOn(wFilterType, PKT_TYPE_PROMISCUOUS | PKT_TYPE_ALL_MULTICAST)) {
		/* set multicast address to accept all */
		VNSvOutPortD(dwIoBase + MAC_REG_MAR, 0xFFFFFFFFL);
		VNSvOutPortD(dwIoBase + MAC_REG_MAR + sizeof(DWORD), 0xFFFFFFFFL);
	}

	if (BITbIsBitOn(wFilterType, PKT_TYPE_PROMISCUOUS))
		byNewRCR |= (RCR_PROM | RCR_AM | RCR_AB);

	if (BITbIsAnyBitsOn(wFilterType, PKT_TYPE_MULTICAST | PKT_TYPE_ALL_MULTICAST))
		byNewRCR |= RCR_AM;

	if (BITbIsBitOn(wFilterType, PKT_TYPE_BROADCAST))
		byNewRCR |= RCR_AB;

	if (BITbIsBitOn(wFilterType, PKT_TYPE_RUNT))
		byNewRCR |= RCR_AR;

	if (BITbIsBitOn(wFilterType, PKT_TYPE_ERROR))
		byNewRCR |= RCR_SEP;

	VNSvInPortB(dwIoBase + MAC_REG_RCR,  &byOldRCR);
	if (byNewRCR != (byOldRCR & 0x1F)) {
		/* Modify the Receive Command Register */
		byNewRCR |= (BYTE)(byOldRCR & 0xE0);
		VNSvOutPortB(dwIoBase + MAC_REG_RCR, byNewRCR);
	}
}

VOID
MACvSetRxThreshold(
	DWORD   dwIoBase,
	BYTE    byThreshold
	)
{
	BYTE byOrgValue;

	/* set BCR0 */
	VNSvInPortB(dwIoBase + MAC_REG_BCR0, &byOrgValue);
	byOrgValue = (BYTE)((byOrgValue & 0xC7) | (byThreshold << 3));
	VNSvOutPortB(dwIoBase + MAC_REG_BCR0, byOrgValue);
	/* set RCR */
	VNSvInPortB(dwIoBase + MAC_REG_RCR, &byOrgValue);
	byOrgValue = (BYTE)((byOrgValue & 0x1F) | (byThreshold << 5));
	VNSvOutPortB(dwIoBase + MAC_REG_RCR, byOrgValue);
}

VOID
MACvGetTxThreshold(
	DWORD   dwIoBase,
	PBYTE   pbyThreshold
	)
{
	/* first, get BCR1 */
	VNSvInPortB(dwIoBase + MAC_REG_BCR1, pbyThreshold);
	*pbyThreshold = (BYTE)((*pbyThreshold >> 3) & 0x07);

	/* second, if BCR1 is zero, we get TCR */
	if (*pbyThreshold == 0) {
		VNSvInPortB(dwIoBase + MAC_REG_TCR, pbyThreshold);
		*pbyThreshold >>= 5;
	}
}

VOID
MACvSetTxThreshold(
	DWORD   dwIoBase,
	BYTE    byThreshold
	)
{
	BYTE byOrgValue;

	/* set BCR1 */
	VNSvInPortB(dwIoBase + MAC_REG_BCR1, &byOrgValue);
	byOrgValue = (BYTE)((byOrgValue & 0xC7) | (byThreshold << 3));
	VNSvOutPortB(dwIoBase + MAC_REG_BCR1, byOrgValue);
	/* set TCR */
	VNSvInPortB(dwIoBase + MAC_REG_TCR, &byOrgValue);
	byOrgValue = (BYTE)((byOrgValue & 0x1F) | (byThreshold << 5));
	VNSvOutPortB(dwIoBase + MAC_REG_TCR, byOrgValue);
}

VOID
MACvTimer0MicroSDelay(
	DWORD   dwIoBase,
	BYTE    byRevId,
	UINT    udelay
	)
{
	BYTE    byData;

	MACvRegBitsOn(dwIoBase, MAC_REG_MISC_CR0, MISC_CR0_TM0US);

	/* set delay time to udelay, unit is micro-second */
	VNSvOutPortW(dwIoBase + MAC_REG_SOFT_TIMER0, (WORD)udelay);
	/* delay time */
	MACvRegBitsOn(dwIoBase, MAC_REG_MISC_CR0, MISC_CR0_TIMER0_EN);
	MACvRegBitsOff(dwIoBase, MAC_REG_MISC_CR0, MISC_CR0_TIMER0_SUSPEND);

	while (TRUE) {
		VNSvInPortB(dwIoBase + MAC_REG_MISC_CR0, &byData);
		if (BITbIsBitOn(byData, MISC_CR0_TIMER0_SUSPEND))
			break;
	}
}

VOID
MACvTimer0MiniSDelay(
	DWORD   dwIoBase,
	BYTE    byRevId,
	UINT    udelay
	)
{
	BYTE    byData;

	MACvRegBitsOff(dwIoBase, MAC_REG_MISC_CR0, MISC_CR0_TM0US);

	/* set delay time to udelay, unit is mini-second */
	VNSvOutPortW(dwIoBase + MAC_REG_SOFT_TIMER0, (WORD)udelay);

	/* delay time */
	MACvRegBitsOn(dwIoBase, MAC_REG_MISC_CR0, MISC_CR0_TIMER0_EN);
	MACvRegBitsOff(dwIoBase, MAC_REG_MISC_CR0, MISC_CR0_TIMER0_SUSPEND);

	while (TRUE) {
		VNSvInPortB(dwIoBase + MAC_REG_MISC_CR0, &byData);
		if (BITbIsBitOn(byData, MISC_CR0_TIMER0_SUSPEND))
			break;
	}
}

VOID
MACvWriteEECSR(
	DWORD   dwIoBase,
	BYTE    byValue,
	BYTE    byRevId
	)
{
	VNSvOutPortB(dwIoBase + MAC_REG_EECSR, byValue);

	/* delay 10 micro-seconds using MAC hardware timer */
	MACvTimer0MicroSDelay(dwIoBase, byRevId, 10);
}

BYTE
MACbyGetPhyId(
	DWORD   dwIoBase
	)
{
	BYTE byOrgValue;

	VNSvInPortB(dwIoBase + MAC_REG_MPHY, &byOrgValue);
	return (BYTE)(byOrgValue & 0x1F);
}
