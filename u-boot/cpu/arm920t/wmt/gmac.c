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
#if !defined(__TBIT_H__)
#include "tbit.h"
#endif
#if !defined(__MAC_H__)
#include "mac.h"
#endif
#if !defined(_MACIF_H_)
#include "macif.h"
#endif

/*---------------------  Static Definitions -------------------------*/
#define CB_DELAY_NM9346         (10)        /* 10ms */
#define CB_DELAY_SOFT_RESET     (50)        /* 50ms */

/*---------------------  Static Classes  ----------------------------*/

/*---------------------  Static Variables  --------------------------*/

/*---------------------  Static Functions  --------------------------*/

/*---------------------  Export Variables  --------------------------*/

/*---------------------  Export Functions  --------------------------*/

VOID GMACvReadAllRegs(DWORD dwIoBase, PSMacReg pMacRegs, BYTE byRevId)
{
	int ii;
	PBYTE pbyData = (PBYTE)pMacRegs;

	for (ii = 0; ii < MAC_REG_DEC_BASE_HI; ii++) {
		VNSvInPortB(dwIoBase + ii, pbyData);
		pbyData++;
	}

	for ( ; ii < MAC_REG_ISR_CTL; ii += sizeof(DWORD), pbyData += sizeof(DWORD))
		VNSvInPortD(dwIoBase + ii, (PDWORD)pbyData);

	for ( ; ii < MAC_REG_RDBASE_LO; ii++, pbyData++)
		VNSvInPortB(dwIoBase + ii, pbyData);

	for ( ; ii < MAC_REG_CAMADDR; ii += sizeof(DWORD), pbyData += sizeof(DWORD))
		VNSvInPortD(dwIoBase + ii, (PDWORD)pbyData);

	for ( ; ii < MAC_REG_MIBREAD; ii++, pbyData++)
		VNSvInPortB(dwIoBase + ii, pbyData);

	ii += 4;
	pbyData += 4; /* skip MIB_READ_PORT, because it may have side effect. */

	for ( ; ii < MAC_REG_WOLCR0_SET; ii++, pbyData++)
		VNSvInPortB(dwIoBase + ii, pbyData);

	for ( ; ii < MAC_REG_BYTEMSK3_3 + 4; ii += sizeof(DWORD), pbyData += sizeof(DWORD))
		VNSvInPortD(dwIoBase + ii, (PDWORD)pbyData);

	return;
}

BOOL GMACbIsRegBitsOn(DWORD dwIoBase, BYTE byRegOfs, BYTE byTestBits)
{
	BYTE    byData;
	VNSvInPortB(dwIoBase + byRegOfs, &byData);
	return BITbIsAllBitsOn(byData, byTestBits);
}

BOOL GMACbIsRegBitsOff(DWORD dwIoBase, BYTE byRegOfs, BYTE byTestBits)
{
	BYTE byData;
	VNSvInPortB(dwIoBase + byRegOfs, &byData);
	return BITbIsAllBitsOff(byData, byTestBits);
}

BYTE GMACbyReadEECSR(DWORD dwIoBase)
{
	BYTE byData;

	VNSvInPortB(dwIoBase + MAC_REG_EECSR, &byData);
	/* PCAvDelayByIO(CB_DELAY_NM9346); */
	return byData;
}

VOID GMACvWriteEECSR(DWORD dwIoBase, BYTE byValue)
{
	VNSvOutPortB(dwIoBase + MAC_REG_EECSR, byValue);
	/* delay 1 ms using MAC hardware timer */
	GMACvTimer0MiniSDelay(dwIoBase, REV_ID_VT3119_A0, 1);
}

/*
void MACvWriteEECSR (DWORD dwIoBase, BYTE byValue)
{
	VNSvOutPortB(dwIoBase + MAC_REG_EECSR, byValue);
	PCAvDelayByIO(CB_DELAY_NM9346);
}
*/

BYTE GMACbyReadMIICR(DWORD dwIoBase)
{
	BYTE byData;

	VNSvInPortB(dwIoBase + MAC_REG_MIICR, &byData);
	return byData;
}

VOID GMACvWriteMIICR(DWORD dwIoBase, BYTE byValue)
{
	VNSvOutPortB(dwIoBase + MAC_REG_MIICR, byValue);
}


BYTE GMACbyReadMultiAddr(DWORD dwIoBase, UINT uByteIdx)
{
	BYTE byData;

	VNSvInPortB(dwIoBase + MAC_REG_MAR + uByteIdx, &byData);
	return byData;
}


VOID GMACvWriteMultiAddr(DWORD dwIoBase, UINT uByteIdx, BYTE byData)
{
	VNSvOutPortB(dwIoBase + MAC_REG_MAR + uByteIdx, byData);
}


BOOL GMACbIsIntDisable(DWORD dwIoBase, BYTE byRevId)
{
	DWORD dwData;

	VNSvInPortD(dwIoBase + MAC_REG_IMR, &dwData);
	if (dwData != 0x00000000UL)
		return FALSE;

	return TRUE;
}

/*
 * Description: Set this hash index into multicast address register
 *              bit
 *
 * Parameters:
 *
 */
VOID GMACvSetMultiAddrByHash(DWORD dwIoBase, BYTE byHashIdx)
{
	UINT uByteIdx;
	BYTE byBitMask;
	BYTE byOrgValue;
	BYTE byOrgCAMCR, byData;

	/* modify CAMCR to select MAR regs */
	VNSvInPortB(dwIoBase + MAC_REG_CAMCR, &byOrgCAMCR);
	byData = (BYTE)(byOrgCAMCR & ~(CAMCR_PS1|CAMCR_PS0));
	VNSvOutPortB(dwIoBase + MAC_REG_CAMCR, byData);

	/* calculate byte position */
	uByteIdx = byHashIdx / 8;
	DBG_ASSERT(uByteIdx < 8);
	/* calculate bit position */
	byBitMask = 1;
	byBitMask <<= (byHashIdx % 8);
	/* turn on the bit */
	byOrgValue = GMACbyReadMultiAddr(dwIoBase, uByteIdx);
	GMACvWriteMultiAddr(dwIoBase, uByteIdx, (BYTE)(byOrgValue | byBitMask));

	/* restore to original CAMCR */
	VNSvOutPortB(dwIoBase + MAC_REG_CAMCR, byOrgCAMCR);
}

/*
 * Description: Reset this hash index into multicast address register
 *              bit
 *
 * Parameters:
 *
 */
VOID GMACvResetMultiAddrByHash(DWORD dwIoBase, BYTE byHashIdx)
{
	UINT uByteIdx;
	BYTE byBitMask;
	BYTE byOrgValue;
	BYTE byOrgCAMCR, byData;

	/* modify CAMCR to select MAR regs */
	VNSvInPortB(dwIoBase + MAC_REG_CAMCR, &byOrgCAMCR);
	byData = (BYTE)(byOrgCAMCR & ~(CAMCR_PS1|CAMCR_PS0));
	VNSvOutPortB(dwIoBase + MAC_REG_CAMCR, byData);

	/* calculate byte position */
	uByteIdx = byHashIdx / 8;
	DBG_ASSERT(uByteIdx < 8);
	/* calculate bit position */
	byBitMask = 1;
	byBitMask <<= (byHashIdx % 8);
	/* turn off the bit */
	byOrgValue = GMACbyReadMultiAddr(dwIoBase, uByteIdx);
	GMACvWriteMultiAddr(dwIoBase, uByteIdx, (BYTE)(byOrgValue & (~byBitMask)));

	/* restore to original CAMCR */
	VNSvOutPortB(dwIoBase + MAC_REG_CAMCR, byOrgCAMCR);
}

BYTE GMACbyGetBootRomSize(DWORD dwIoBase)
{
	BYTE byOrgValue;

	VNSvInPortB(dwIoBase + MAC_REG_CFGC, &byOrgValue);
	return (BYTE)(byOrgValue & 0x07);
}

VOID GMACvSetPhyId(DWORD dwIoBase, BYTE byRevId, BYTE byPhyId)
{
	BYTE    byData;

	/* set PHY address */
	VNSvInPortB(dwIoBase + MAC_REG_MIICFG, &byData);
	byData = (BYTE)((byData & 0xE0) | (byPhyId & 0x1F));
	VNSvOutPortB(dwIoBase + MAC_REG_MIICFG, byData);
}

BYTE GMACbyGetPhyId(DWORD dwIoBase)
{
	BYTE byOrgValue;

	VNSvInPortB(dwIoBase + MAC_REG_MIICFG, &byOrgValue);
	return (BYTE)(byOrgValue & 0x1F);
}

VOID GMACvSetRxThreshold(DWORD dwIoBase, BYTE byThreshold)
{
	BYTE byOrgValue;

	DBG_ASSERT(byThreshold < 4);

	/* set MCFG0 */
	VNSvInPortB(dwIoBase + MAC_REG_MCFG0, &byOrgValue);
	byOrgValue = (BYTE)((byOrgValue & ~(MCFG0_RFT1|MCFG0_RFT0)) | (byThreshold << 4));
	VNSvOutPortB(dwIoBase + MAC_REG_MCFG0, byOrgValue);
}

VOID GMACvGetRxThreshold(DWORD dwIoBase, PBYTE pbyThreshold)
{
	/* first, get MCFG0 */
	VNSvInPortB(dwIoBase + MAC_REG_MCFG0, pbyThreshold);
	*pbyThreshold = (BYTE)((*pbyThreshold >> 4) & 0x03);
}

VOID GMACvSetDmaLength(DWORD dwIoBase, BYTE byDmaLength)
{
	BYTE byOrgValue;

	DBG_ASSERT(byDmaLength < 8);

	/* set DCFG0 */
	VNSvInPortB(dwIoBase + MAC_REG_DCFG0, &byOrgValue);
	byOrgValue = (BYTE)((byOrgValue & 0xF8) | byDmaLength);
	VNSvOutPortB(dwIoBase + MAC_REG_DCFG0, byOrgValue);
}

VOID GMACvGetDmaLength(DWORD dwIoBase, PBYTE pbyDmaLength)
{
	/* get DCFG0 */
	VNSvInPortB(dwIoBase + MAC_REG_DCFG0, pbyDmaLength);
	*pbyDmaLength &= 0x07;
}

VOID GMACvSetLoopbackMode(DWORD dwIoBase, BYTE byLoopbackMode)
{
	BYTE byOrgValue;

	DBG_ASSERT(byLoopbackMode < 3);

	/* set TCR */
	VNSvInPortB(dwIoBase + MAC_REG_TCR, &byOrgValue);
	byOrgValue = (BYTE)((byOrgValue & 0xFC) | byLoopbackMode);
	VNSvOutPortB(dwIoBase + MAC_REG_TCR, byOrgValue);
}

BOOL GMACbIsInLoopbackMode(DWORD dwIoBase)
{
	BYTE byOrgValue;

	VNSvInPortB(dwIoBase + MAC_REG_TCR, &byOrgValue);
	if (BITbIsAnyBitsOn(byOrgValue, TCR_LB1 | TCR_LB0))
		return TRUE;

	return FALSE;
}

BOOL GMACbIsIn1GMode(DWORD dwIoBase, BYTE byRevId)
{
	UINT ii;

	for (ii = 0; ii < 30; ii++)
		PCAvDelayByIO(CB_DELAY_LOOP_WAIT);

	/* NOTE.... */
	/* when link fail, MAC will report in 1G mode, */
	return GMACbIsRegBitsOn(dwIoBase, MAC_REG_PHYSR0, PHYSR0_SPDG);
}

BOOL GMACbIsIn100MMode(DWORD dwIoBase, BYTE byRevId)
{
	UINT ii;

	for (ii = 0; ii < 30; ii++)
		PCAvDelayByIO(CB_DELAY_LOOP_WAIT);

	/* NOTE.... */
	/* when link fail, MAC will report in 1G mode, */
	return GMACbIsRegBitsOff(dwIoBase, MAC_REG_PHYSR0, PHYSR0_SPDG) &&
		GMACbIsRegBitsOff(dwIoBase, MAC_REG_PHYSR0, PHYSR0_SPD10);
}

BOOL GMACbIsInFullDuplexMode(DWORD dwIoBase)
{
	return GMACbIsRegBitsOn(dwIoBase, MAC_REG_PHYSR0, PHYSR0_FDPX);
}

BOOL GMACbIsCableLinkOk(DWORD dwIoBase)
{
	return GMACbIsRegBitsOn(dwIoBase, MAC_REG_PHYSR0, PHYSR0_LINKGD);
}

VOID GMACvDisableMiiAutoPoll(DWORD dwIoBase)
{
	WORD    ww;
	BYTE    byData;

	/* turn off MAUTO */
	VNSvOutPortB(dwIoBase + MAC_REG_MIICR, 0);

	/* as soon as MIDLE is on, MAUTO is really stoped */
	for (ww = 0; ww < W_MAX_TIMEOUT; ww++) {
		udelay(1000);
		VNSvInPortB(dwIoBase + MAC_REG_MIISR, &byData);
		if (BITbIsBitOn(byData, MIISR_MIIDL))
			break;
	}
}

/*
 * NOTE....
 * first, disable MIICR_MAUTO, then
 * set MII reg offset to BMSR == 0x01
 * must set offset before re-enable MIICR_MAUTO
 */
VOID GMACvEnableMiiAutoPoll(DWORD dwIoBase)
{
	WORD    ww;
	BYTE    byData;

	VNSvOutPortB(dwIoBase + MAC_REG_MIICR, 0);

	/* polling once before MAUTO is turned on */
	VNSvOutPortB(dwIoBase + MAC_REG_MIIADR, MIIADR_SWMPL);

	/* as soon as MIIDL is on, polling is really completed */
	for (ww = 0; ww < W_MAX_TIMEOUT; ww++) {
		udelay(1000);
		VNSvInPortB(dwIoBase + MAC_REG_MIISR, &byData);
		if (BITbIsBitOn(byData, MIISR_MIIDL))
			break;
	}

	/* Turn on MAUTO */
	VNSvOutPortB(dwIoBase + MAC_REG_MIICR, MIICR_MAUTO);

	/* as soon as MIIDL is off, MAUTO is really started */
	for (ww = 0; ww < W_MAX_TIMEOUT; ww++) {
		VNSvInPortB(dwIoBase + MAC_REG_MIISR, &byData);
		if (BITbIsBitOff(byData, MIISR_MIIDL))
			break;
	}
}

VOID GMACvSetPacketFilter(DWORD dwIoBase, WORD wFilterType)
{
	BYTE    byOldRCR;
	BYTE    byNewRCR = 0;
	BYTE    byOrgCAMCR, byData;


	/* modify CAMCR to select MAR regs */
	VNSvInPortB(dwIoBase + MAC_REG_CAMCR, &byOrgCAMCR);
	byData = (BYTE)(byOrgCAMCR & ~(CAMCR_PS1|CAMCR_PS0));
	VNSvOutPortB(dwIoBase + MAC_REG_CAMCR, byData);

	/* if only in DIRECTED mode, multicast-address will set to zero, */
	/* but if other mode exist (e.g. PROMISCUOUS), multicast-address */
	/* will be open */
	if (BITbIsBitOn(wFilterType, PKT_TYPE_DIRECTED)) {
		/* set multicast address to accept none */
		VNSvOutPortD(dwIoBase + MAC_REG_MAR, 0L);
		/* PCAvDelayByIO(1); */
		VNSvOutPortD(dwIoBase + MAC_REG_MAR + sizeof(DWORD), 0L);
		/* PCAvDelayByIO(1); */
	}

	if (BITbIsAnyBitsOn(wFilterType, PKT_TYPE_PROMISCUOUS | PKT_TYPE_ALL_MULTICAST)) {
		/* set multicast address to accept all */
		VNSvOutPortD(dwIoBase + MAC_REG_MAR, 0xFFFFFFFFL);
		/* PCAvDelayByIO(1); */
		VNSvOutPortD(dwIoBase + MAC_REG_MAR + sizeof(DWORD), 0xFFFFFFFFL);
		/* PCAvDelayByIO(1); */
	}

	/* restore to original CAMCR */
	VNSvOutPortB(dwIoBase + MAC_REG_CAMCR, byOrgCAMCR);


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

VOID GMACvSaveContext(DWORD dwIoBase, BYTE byRevId, PBYTE pbyCxtBuf)
{
	int         ii;

	/* save RCR,TCR,CR,ISR,IMR... */
	for (ii = MAC_REG_PAR; ii < MAC_REG_DEC_BASE_HI; ii++) {
		if (ii >= MAC_REG_CR0_CLR && ii <= MAC_REG_CR3_CLR)
			continue;
		VNSvInPortB(dwIoBase + ii, (PBYTE)(pbyCxtBuf + ii));
	}

	/* save MAC_REG_DEC_BASE_HI... */
	for (ii = MAC_REG_DEC_BASE_HI; ii < MAC_REG_ISR_CTL; ii += 4)
		VNSvInPortD(dwIoBase + ii, (PDWORD)(pbyCxtBuf + ii));

	/* save ISR_CTL,ISR,IMR,TDCSR,RDCSR.. */
	for (ii = MAC_REG_ISR_CTL; ii < MAC_REG_TDCSR_CLR; ii++)
		VNSvInPortB(dwIoBase + ii, (PBYTE)(pbyCxtBuf + ii));

	/* save MAC_REG_RDBASE_LO,MAC_REG_TDBASE_LO... */
	for (ii = MAC_REG_RDBASE_LO; ii < MAC_REG_FIFO_TEST0; ii += 4)
		VNSvInPortD(dwIoBase + ii, (PDWORD)(pbyCxtBuf + ii));

	/* save MIICFG... */
	for (ii = MAC_REG_MIICFG; ii < MAC_REG_TBIST; ii++)
		VNSvInPortB(dwIoBase + ii, (PBYTE)(pbyCxtBuf + ii));

	/* save GHIPGSR, WOLCR, PWCFG, TestReg, WOLCG  ... */
	for (ii = MAC_REG_CHIPGSR; ii < MAC_REG_WOLCR0_CLR; ii++)
		VNSvInPortB(dwIoBase + ii, (PBYTE)(pbyCxtBuf + ii));

	/* save pattern, mask */
	for (ii = MAC_REG_PATRN_CRC0; ii < MAC_REG_BYTEMSK3_3 + 4; ii += 4)
		VNSvInPortD(dwIoBase + ii, (PDWORD)(pbyCxtBuf + ii));
}

VOID GMACvRestoreContext(DWORD dwIoBase, BYTE byRevId, PBYTE pbyCxtBuf)
{
	int         ii;

	/* restore RCR,TCR,CR,ISR,IMR... */
	for (ii = MAC_REG_PAR; ii < MAC_REG_DEC_BASE_HI; ii++) {
		if (ii >= MAC_REG_CR0_CLR && ii <= MAC_REG_CR3_CLR)
			continue;
		/* except CR0, because we don't want to start chip now */
		if (ii != MAC_REG_CR0_SET)
			VNSvOutPortB(dwIoBase + ii, *(pbyCxtBuf + ii));
	}

	/* restore MAC_REG_DEC_BASE_HI... */
	for (ii = MAC_REG_DEC_BASE_HI; ii < MAC_REG_ISR_CTL; ii += 4) {
		VNSvOutPortD(dwIoBase + ii, *(PDWORD)(pbyCxtBuf + ii));
		/* PCAvDelayByIO(1); */
	}

	/* restore ISR_CTL,ISR,IMR,TDCSR,RDCSR.. */
	for (ii = MAC_REG_ISR_CTL; ii < MAC_REG_TDCSR_CLR; ii++)
		VNSvOutPortB(dwIoBase + ii, *(pbyCxtBuf + ii));

	/* restore MAC_REG_RDBASE_LO,MAC_REG_TDBASE_LO... */
	for (ii = MAC_REG_RDBASE_LO; ii < MAC_REG_FIFO_TEST0; ii += 4) {
		VNSvOutPortD(dwIoBase + ii, *(PDWORD)(pbyCxtBuf + ii));
		/* PCAvDelayByIO(1); */
	}

	/* restore MIICFG... */
	for (ii = MAC_REG_MIICFG; ii < MAC_REG_TBIST; ii++)
		VNSvOutPortB(dwIoBase + ii, *(pbyCxtBuf + ii));

	/* restore GHIPGSR, WOLCR, PWCFG, TestReg, WOLCG  ... */
	for (ii = MAC_REG_CHIPGSR; ii < MAC_REG_WOLCR0_CLR; ii++)
		VNSvOutPortB(dwIoBase + ii, *(pbyCxtBuf + ii));

	/* restore pattern, mask */
	for (ii = MAC_REG_PATRN_CRC0; ii < MAC_REG_BYTEMSK3_3 + 4; ii += 4) {
		VNSvOutPortD(dwIoBase + ii, *(PDWORD)(pbyCxtBuf + ii));
		/* PCAvDelayByIO(1); */
	}
}

VOID GMACvTimer0MiniSDelay(DWORD dwIoBase, BYTE byRevId, UINT udelay)
{
	BYTE    byData;

	/* Disable Timer0 Interrupt */
	MACvRegBitsOff(dwIoBase, MAC_REG_IMR + 2, (BYTE)(ISR_TMR0I >> 16));

	/* Set resolution to mini second */
	MACvRegBitsOff(dwIoBase, MAC_REG_CHIPGCR, CHIPGCR_TM0US);

	/* set delay time to udelay, unit is mini-second */
	VNSvOutPortW(dwIoBase + MAC_REG_SOFT_TIMER0, (WORD)udelay);

	/* enable timer0 */
	VNSvOutPortB(dwIoBase + MAC_REG_CR1_SET, CR1_TM0EN);

	/* wait for TM0EN self clear */
	while (TRUE) {
		if (GMACbIsRegBitsOff(dwIoBase, MAC_REG_CR1_SET, CR1_TM0EN)) {
			/* clear TMR0I */
			VNSvInPortB(dwIoBase + MAC_REG_ISR + 2, &byData);
			VNSvOutPortB(dwIoBase + MAC_REG_ISR + 2, byData);
			break;
		}
	}

	/* Enable Timer0 Interrupt */
	MACvRegBitsOn(dwIoBase, MAC_REG_IMR + 2, (BYTE)(ISR_TMR0I >> 16));
}

VOID GMACvTimer0MicroSDelay(DWORD dwIoBase, BYTE byRevId, UINT udelay)
{
	BYTE    byData;

	/* Notice!!! */
	/* Disable Timer0 Interrupt */
	MACvRegBitsOff(dwIoBase, MAC_REG_IMR + 2, (BYTE)(ISR_TMR0I >> 16));

	/* Set resolution to micro second */
	MACvRegBitsOn(dwIoBase, MAC_REG_CHIPGCR, CHIPGCR_TM0US);

	/* set delay time to udelay, unit is micro-second */
	VNSvOutPortW(dwIoBase + MAC_REG_SOFT_TIMER0, (WORD)udelay);

	/* enable timer0 */
	VNSvOutPortB(dwIoBase + MAC_REG_CR1_SET, CR1_TM0EN);

	/* wait for TM0EN self clear */
	while (TRUE) {
		/* Method 1 -> OK, and safe */
		if (GMACbIsRegBitsOff(dwIoBase, MAC_REG_CR1_SET, CR1_TM0EN)) {
			/* clear TMR0I */
			VNSvInPortB(dwIoBase + MAC_REG_ISR + 2, &byData);
			VNSvOutPortB(dwIoBase + MAC_REG_ISR + 2, byData);
			break;
		}

		/*
		// Method 2 -> OK, but not safe
		VNSvInPortB(dwIoBase + MAC_REG_ISR + 2, &byData);
		if (BITbIsBitOn(byData, (BYTE)(ISR_TMR0I >> 16))) {
			VNSvOutPortB(dwIoBase + MAC_REG_ISR + 2, byData);
			break;
		}
		*/
		/*
		// Method 3 -> OK, but not safe
		if (MACbIsRegBitsOn(dwIoBase, MAC_REG_ISR + 2, (BYTE)(ISR_TMR0I >> 16))) {
			// clear TMR0I
			VNSvOutPortB(dwIoBase + MAC_REG_ISR + 2, (BYTE)(ISR_TMR0I >> 16));
			break;
		}
		*/
	}

	/* Notice !!! */
	/* Enable Timer0 Interrupt */
	MACvRegBitsOn(dwIoBase, MAC_REG_IMR + 2, (BYTE)(ISR_TMR0I >> 16));
}

BOOL GMACbSoftwareReset(DWORD dwIoBase, BYTE byRevId)
{
	WORD    ww;

	/* turn on CR1_SFRST */
	MACvRegBitsOn(dwIoBase, MAC_REG_CR1_SET, CR1_SFRST);

	/* polling till software reset complete */
	/* W_MAX_TIMEOUT is the timeout period */
	for (ww = 0; ww < W_MAX_TIMEOUT; ww++) {
		if (GMACbIsRegBitsOff(dwIoBase, MAC_REG_CR1_SET, (BYTE)CR1_SFRST))
			break;
	}

	if (ww == W_MAX_TIMEOUT) {
		/* turn on force reset */
		MACvRegBitsOn(dwIoBase, MAC_REG_CR3_SET, CR3_FORSRST);
		/* delay 2ms */
		GMACvTimer0MiniSDelay(dwIoBase, byRevId, 2);
	}
	return TRUE;
}

BOOL GMACbSafeSoftwareReset(DWORD dwIoBase, BYTE byRevId)
{
	BYTE    abyTmpRegData[0x100];
	BOOL    bRetVal;

	/* PATCH.... */
	/* save some important register's value, then do */
	/* reset, then restore register's value */

	/* save MAC context */
	GMACvSaveContext(dwIoBase, byRevId, abyTmpRegData);
	/* do reset */
	bRetVal = GMACbSoftwareReset(dwIoBase, byRevId);
	/* restore MAC context, except CR0 */
	GMACvRestoreContext(dwIoBase, byRevId, abyTmpRegData);

	return bRetVal;
}

BOOL GMACbSafeRxOff(DWORD dwIoBase, BYTE byRevId)
{
	WORD    ww;

	/* clear RUN Rx */
	VNSvOutPortB(dwIoBase + MAC_REG_RDCSR_CLR, TRDCSR_RUN);

	MACvRxOff(dwIoBase);

	/* safe delay time */
	PCAvDelayByIO(CB_DELAY_SOFT_RESET);
	/* W_MAX_TIMEOUT is the timeout period */
	for (ww = 0; ww < W_MAX_TIMEOUT; ww++) {
		if (GMACbIsRegBitsOff(dwIoBase, MAC_REG_CR0_SET, (BYTE)CR0_RXON))
			break;
	}

	if (ww == W_MAX_TIMEOUT)
		return FALSE;

	return TRUE;
}

BOOL GMACbSafeTxOff(DWORD dwIoBase, BYTE byRevId)
{
	WORD    ww;

	/* clear RUN Tx */
	VNSvOutPortW(dwIoBase + MAC_REG_TDCSR_CLR, 0x1111);

	/* try to safe shutdown TX */
	MACvTxOff(dwIoBase);

	/* safe delay time */
	PCAvDelayByIO(CB_DELAY_SOFT_RESET);
	/* W_MAX_TIMEOUT is the timeout period */
	for (ww = 0; ww < W_MAX_TIMEOUT; ww++) {
		if (GMACbIsRegBitsOff(dwIoBase, MAC_REG_CR0_SET, (BYTE)CR0_TXON))
			break;
	}
	if (ww == W_MAX_TIMEOUT)
		return FALSE;

	return TRUE;
}

BOOL GMACbStop(DWORD dwIoBase)
{
	WORD    ww;

	VNSvOutPortB(dwIoBase + MAC_REG_CR0_SET, CR0_STOP);

	/* W_MAX_TIMEOUT is the timeout period */
	for (ww = 0; ww < W_MAX_TIMEOUT; ww++) {
		if (GMACbIsRegBitsOff(dwIoBase, MAC_REG_CR0_SET, (BYTE)(CR0_TXON | CR0_RXON)))
			break;
	}
	if (ww == W_MAX_TIMEOUT)
		return FALSE;

	return TRUE;
}

BOOL GMACbSafeStop(DWORD dwIoBase, BYTE byRevId)
{
	if (!GMACbSafeTxOff(dwIoBase, byRevId))
		return FALSE;
	if (!GMACbSafeRxOff(dwIoBase, byRevId))
		return FALSE;
	if (!GMACbStop(dwIoBase))
		return FALSE;

	return TRUE;
}

BOOL GMACbShutdown(DWORD dwIoBase, BYTE byRevId)
{
	/* disable MAC IMR */
	MACvIntDisable(dwIoBase);
	/* disable MII auto-poll */
	GMACvDisableMiiAutoPoll(dwIoBase);
	/* stop the adapter */
	if (!GMACbSafeStop(dwIoBase, byRevId))
		return FALSE;

	return TRUE;
}

int GMACiWriteVee(DWORD dwIoBase, unsigned char i8Addr, unsigned short i16Data)
{
	int i32Data;
	i8Addr = i8Addr & 0xf;
	i32Data = i16Data|(i8Addr<<16);
	VPCIvWriteD(dwIoBase, Gmac_PCI_VEE_WRITE, i32Data);
	return 1;
}

VOID GMACLoadVEE(DWORD dwIoBase)
{
	unsigned char gmac[32] = {
		0, 0x02, 0x2a, 0xdd, 0x44, 0xb0, 0x7, 0x0, 0x10,
		0x01, 0x6, 0x11, 0x19, 0x31, 0x6, 0x11, 0x1f, 0x10,
		0x0, 0x0, 0x00, 0x01, 0x3, 0x8, 0x01, 0x10, 0x13,
		0x10, 0x40, 0x30, 0x73, 0x21
	};
	/* unsigned char check; */
	DWORD i32Data;
	int ii = 0;

	/* fill virtual eeprom */
	for (ii = 0; ii < 16; ii++)
		GMACiWriteVee(dwIoBase, ii, *(((unsigned short *)gmac)+ii));

	/* enable read process */
	VPCIvReadD(dwIoBase, Gmac_PCI_VEE_READ, &i32Data);
	i32Data = i32Data | 0x08000000;
	VPCIvWriteD(dwIoBase, Gmac_PCI_VEE_READ, i32Data);
	/* set SEEPR */
	VPCIvReadD(dwIoBase, Gmac_PCI_VEE_READ, &i32Data);
	i32Data = i32Data|0x01000000;
	VPCIvWriteD(dwIoBase, Gmac_PCI_VEE_READ, i32Data);
	MACvRegBitsOn(dwIoBase, MAC_REG_EECSR, EECSR_RELOAD);
	/* wait SEELD */
	while (1) {
		VPCIvReadD(dwIoBase, Gmac_PCI_VEE_READ, &i32Data);
		if (i32Data & 0x02000000)
			break;
	}
	/* clean SEEPR */
	VPCIvReadD(dwIoBase, Gmac_PCI_VEE_READ, &i32Data);
	i32Data = i32Data & ~0x01000000;
	VPCIvWriteD(dwIoBase, Gmac_PCI_VEE_READ, i32Data);
	/* disable read process */
	VPCIvReadD(dwIoBase, Gmac_PCI_VEE_READ, &i32Data);
	i32Data = i32Data & ~0x08000000;
	VPCIvWriteD(dwIoBase, Gmac_PCI_VEE_READ, i32Data);
}

VOID GMACvInitialize(PSAdapterInfo pAdapter, DWORD dwIoBase, BYTE byRevId)
{
	BYTE    byTemp;
	BYTE check;

	/* clear sticky bits */
	MACvClearStckDS(dwIoBase);

	/* disable force PME-enable */

	VNSvOutPortB(dwIoBase + MAC_REG_WOLCFG_CLR, WOLCFG_PMEOVR);
	/* disable power-event config bit */

	MACvPwrEvntDisable(dwIoBase);

	/* clear power status */
	VNSvOutPortW(dwIoBase + MAC_REG_WOLSR0_CLR, 0xFFFF);

	/* do reset */
	GMACbSoftwareReset(dwIoBase, byRevId);

	/* for AUTOLD be effect, safe delay time */
	PCAvDelayByIO(CB_DELAY_SOFT_RESET);

	VNSvInPortB(dwIoBase + MAC_REG_JMPSR1, &check);

	/* VNSvInPortB(pAdapter->dwIoBase+Gmac_Jumper_Strapping3, &check) */
	/* if (!(check & JMPSR1_J_VEESEL))
		GMACLoadVEE(dwIoBase);
	else { */
	if (check & JMPSR1_J_VEESEL) {
		/* issue RELOAD in EECSR to reload eeprom */
		MACvRegBitsOn(dwIoBase, MAC_REG_EECSR, EECSR_RELOAD);

		/* wait until EEPROM loading complete */
		while (TRUE) {
			if (GMACbIsRegBitsOff(dwIoBase, MAC_REG_EECSR, EECSR_RELOAD))
				break;
		}
	}

	/* EEPROM reloaded will cause bit 0 in MAC_REG_CFGA turned on. */
	/* it makes MAC receive magic packet automatically. So, driver turn it off. */
	MACvRegBitsOff(dwIoBase, MAC_REG_CFGA, CFGA_PACPI);

	/* set rx-FIFO/DMA threshold */
	/* set rx threshold, 128 bytes */
	/*GMACvSetRxThreshold(dwIoBase, 3);*/

	/* set DMA length, 16 DWORDs = 64 bytes */
	/*GMACvSetDmaLength(dwIoBase, 1);*/

	/* suspend-well accept broadcast, multicast */
	VNSvOutPortB(dwIoBase + MAC_REG_WOLCFG_SET, WOLCFG_SAM | WOLCFG_SAB);

	/* back off algorithm use original IEEE standard */
	MACvRegBitsOff(dwIoBase, MAC_REG_CFGB, CFGB_CRANDOM | CFGB_CAP | CFGB_MBA | CFGB_BAKOPT);

	/* set packet filter */
	/* receive directed and broadcast address */
	GMACvSetPacketFilter(dwIoBase, PKT_TYPE_DIRECTED | PKT_TYPE_BROADCAST);

	/* Eric */
#if defined(__USE_GMASK1__)
	VNSvOutPortD(dwIoBase + MAC_REG_IMR, IMR_MASK_VALUE);
#else
	/* Turn on GenIntMask1 */
	VNSvOutPortB(dwIoBase + MAC_REG_CR3_SET, CR3_GINTMSK1);
#endif
#if 0
	VNSvInPortB(dwIoBase, &byTemp);
	printf("address0 = %x\n ", byTemp);
	VNSvInPortB(dwIoBase + 1, &byTemp);
	printf("address1 = %x\n ", byTemp);
	VNSvInPortB(dwIoBase + 2, &byTemp);
	printf("address2 = %x\n ", byTemp);
	VNSvInPortB(dwIoBase + 3, &byTemp);
	printf("address3 = %x\n ", byTemp);
	VNSvInPortB(dwIoBase + 4, &byTemp);
	printf("address4 = %x\n ", byTemp);
	VNSvInPortB(dwIoBase + 5, &byTemp);
	printf("address6 = %x\n ", byTemp);
#endif
	/* Adaptive Interrupt: Init is disabled */

	/* Select page to interrupt hold timer */
	VNSvInPortB(dwIoBase + MAC_REG_CAMCR, &byTemp);
	byTemp &= ~(CAMCR_PS0 | CAMCR_PS1);
	VNSvOutPortB(dwIoBase + MAC_REG_CAMCR, byTemp);
	/* Set Interrupt hold timer = 0 */
	VNSvOutPortB(dwIoBase + MAC_REG_ISR_HOTMR, 0x00);

	/* Select Page to Tx-sup threshold */
	VNSvInPortB(dwIoBase + MAC_REG_CAMCR, &byTemp);
	byTemp = (BYTE)((byTemp | CAMCR_PS0) & ~CAMCR_PS1);
	VNSvOutPortB(dwIoBase + MAC_REG_CAMCR, byTemp);
	/* Set Tx interrupt suppression threshold = 0 */
	VNSvOutPortB(dwIoBase + MAC_REG_ISR_TSUPTHR, 0x00);

	/* Select Page to Rx-sup threshold */
	VNSvInPortB(dwIoBase + MAC_REG_CAMCR, &byTemp);
	byTemp = (BYTE)((byTemp | CAMCR_PS1) & ~CAMCR_PS0);
	VNSvOutPortB(dwIoBase + MAC_REG_CAMCR, byTemp);
	/* Set Rx interrupt suppression threshold = 0 */
	VNSvOutPortB(dwIoBase + MAC_REG_ISR_RSUPTHR, 0x00);

	/* Select page to interrupt hold timer */
	VNSvInPortB(dwIoBase + MAC_REG_CAMCR, &byTemp);
	byTemp &= ~(CAMCR_PS0 | CAMCR_PS1);
	VNSvOutPortB(dwIoBase + MAC_REG_CAMCR, byTemp);

	/* enable MIICR_MAUTO */
	GMACvEnableMiiAutoPoll(dwIoBase);
}

VOID GMACvSetVCAM(DWORD dwIoBase, BYTE byAddress, WORD wVID)
{
	BYTE    byOrgCAMCR, byData;

	/* modify CAMCR to select CAM DATA regs */
	VNSvInPortB(dwIoBase + MAC_REG_CAMCR, &byOrgCAMCR);
	byData = (BYTE)((byOrgCAMCR | CAMCR_PS1) & ~CAMCR_PS0);
	VNSvOutPortB(dwIoBase + MAC_REG_CAMCR, byData);

	/* enable/select VCAM */
	VNSvOutPortB(dwIoBase + MAC_REG_CAMADDR, (BYTE)(CAMADDR_CAMEN | CAMADDR_VCAMSL | byAddress));

	/* set VCAM data */
	VNSvOutPortW(dwIoBase + MAC_REG_CAM, wVID);

	/* issue write command */
	MACvRegBitsOn(dwIoBase, MAC_REG_CAMCR, CAMCR_CAMWR);

	/* Wait for CAMWR self clear */
	while (TRUE) {
		if (GMACbIsRegBitsOff(dwIoBase, MAC_REG_CAMCR, CAMCR_CAMWR))
			break;
	}

	/* disable CAMEN */
	VNSvOutPortB(dwIoBase + MAC_REG_CAMADDR, 0);

	/* restore to original CAMCR */
	VNSvOutPortB(dwIoBase + MAC_REG_CAMCR, byOrgCAMCR);
}

VOID GMACvSetMCAM(DWORD dwIoBase, BYTE byAddress, PBYTE pbyData)
{
	BYTE    byOrgCAMCR, byData;
	/* DWORD   dwData; */

	/* modify CAMCR to select CAM DATA regs */
	VNSvInPortB(dwIoBase + MAC_REG_CAMCR, &byOrgCAMCR);
	byData = (BYTE)((byOrgCAMCR | CAMCR_PS1) & ~CAMCR_PS0);
	VNSvOutPortB(dwIoBase + MAC_REG_CAMCR, byData);

	/* enable/select MCAM */
	VNSvOutPortB(dwIoBase + MAC_REG_CAMADDR, (BYTE)(CAMADDR_CAMEN | byAddress));

	/* set MCAM Data */
	VNSvOutPortD(dwIoBase + MAC_REG_CAM, *(PDWORD)pbyData);
	/* VNSvInPortD(dwIoBase + MAC_REG_CAM, &dwData); */
	VNSvOutPortW(dwIoBase + MAC_REG_CAM + 4, *(PWORD)(pbyData + 4));
	/* VNSvInPortW(dwIoBase + MAC_REG_CAM + 4, (PWORD)(&dwData)); */

	/* issue write command */
	MACvRegBitsOn(dwIoBase, MAC_REG_CAMCR, CAMCR_CAMWR);

	/* Wait for CAMWR self clear */
	while (TRUE) {
		if (GMACbIsRegBitsOff(dwIoBase, MAC_REG_CAMCR, CAMCR_CAMWR))
			break;
	}

	/* disable CAMEN */
	VNSvOutPortB(dwIoBase + MAC_REG_CAMADDR, 0);

	/* restore to original CAMCR */
	VNSvOutPortB(dwIoBase + MAC_REG_CAMCR, byOrgCAMCR);
}

VOID GMACvGetVCAM(DWORD dwIoBase, BYTE byAddress, PWORD pwData)
{
	BYTE    byOrgCAMCR, byData;

	/* modify CAMCR to select CAM DATA regs */
	VNSvInPortB(dwIoBase + MAC_REG_CAMCR, &byOrgCAMCR);
	byData = (BYTE)((byOrgCAMCR | CAMCR_PS1) & ~CAMCR_PS0);
	VNSvOutPortB(dwIoBase + MAC_REG_CAMCR, byData);

	/* enable/select VCAM */
	VNSvOutPortB(dwIoBase + MAC_REG_CAMADDR, (BYTE)(CAMADDR_CAMEN | CAMADDR_VCAMSL | byAddress));

	/* issue read command */
	MACvRegBitsOn(dwIoBase, MAC_REG_CAMCR, CAMCR_CAMRD);

	/* Wait for CAMRD self clear */
	while (TRUE) {
		if (GMACbIsRegBitsOff(dwIoBase, MAC_REG_CAMCR, CAMCR_CAMRD))
			break;
	}

	/* read VID CAM data */
	VNSvInPortW(dwIoBase + MAC_REG_CAM, pwData);

	/* disable CAMEN */
	VNSvOutPortB(dwIoBase + MAC_REG_CAMADDR, 0);

	/* restore to original CAMCR */
	VNSvOutPortB(dwIoBase + MAC_REG_CAMCR, byOrgCAMCR);
}

VOID GMACvGetMCAM(DWORD dwIoBase, BYTE byAddress, PBYTE pbyData)
{
	BYTE    byOrgCAMCR, byData;

	/* modify CAMCR to select CAM DATA regs */
	VNSvInPortB(dwIoBase + MAC_REG_CAMCR, &byOrgCAMCR);
	byData = (BYTE)((byOrgCAMCR | CAMCR_PS1) & ~CAMCR_PS0);
	VNSvOutPortB(dwIoBase + MAC_REG_CAMCR, byData);

	/* enable/select MCAM */
	VNSvOutPortB(dwIoBase + MAC_REG_CAMADDR, (BYTE)(CAMADDR_CAMEN | byAddress));

	/* issue read command */
	MACvRegBitsOn(dwIoBase, MAC_REG_CAMCR, CAMCR_CAMRD);

	/* Wait for CAMRD self clear */
	while (TRUE) {
		if (GMACbIsRegBitsOff(dwIoBase, MAC_REG_CAMCR, CAMCR_CAMRD))
			break;
	}

	VNSvInPortD(dwIoBase + MAC_REG_CAM, (PDWORD)pbyData);
	VNSvInPortW(dwIoBase + MAC_REG_CAM + 4, (PWORD)(pbyData + 4));

	/* disable CAMEN */
	VNSvOutPortB(dwIoBase + MAC_REG_CAMADDR, 0);

	/* restore to original CAMCR */
	VNSvOutPortB(dwIoBase + MAC_REG_CAMCR, byOrgCAMCR);
}

VOID GMACvSetVCAMMask(DWORD dwIoBase, PBYTE pbyMask)
{
	BYTE    byOrgCAMCR, byData;
	/* DWORD   dwData; */

	/* modify CAMCR to select CAM MASK regs */
	VNSvInPortB(dwIoBase + MAC_REG_CAMCR, &byOrgCAMCR);
	byData = (BYTE)((byOrgCAMCR & ~CAMCR_PS1) | CAMCR_PS0);
	VNSvOutPortB(dwIoBase + MAC_REG_CAMCR, byData);

	/* Select VCAM Mask */
	VNSvOutPortB(dwIoBase + MAC_REG_CAMADDR, CAMADDR_VCAMSL);

	VNSvOutPortD(dwIoBase + MAC_REG_CAM, *(PDWORD)pbyMask);
	/* VNSvInPortD(dwIoBase + MAC_REG_CAM, &dwData); */
	VNSvOutPortD(dwIoBase + MAC_REG_CAM + 4, *(PDWORD)(pbyMask + 4));
	/* VNSvInPortD(dwIoBase + MAC_REG_CAM + 4, &dwData); */

	/* restore to original CAMCR */
	VNSvOutPortB(dwIoBase + MAC_REG_CAMCR, byOrgCAMCR);
}

VOID GMACvSetMCAMMask(DWORD dwIoBase, PBYTE pbyMask)
{
	BYTE    byOrgCAMCR, byData;
	/* DWORD   dwData; */

	/* modify CAMCR to select CAM MASK regs */
	VNSvInPortB(dwIoBase + MAC_REG_CAMCR, &byOrgCAMCR);
	byData = (BYTE)((byOrgCAMCR & ~CAMCR_PS1) | CAMCR_PS0);
	VNSvOutPortB(dwIoBase + MAC_REG_CAMCR, byData);

	/* Select MCAM Mask */
	VNSvOutPortB(dwIoBase + MAC_REG_CAMADDR, 0);

	VNSvOutPortD(dwIoBase + MAC_REG_CAM, *(PDWORD)pbyMask);
	/* VNSvInPortD(dwIoBase + MAC_REG_CAM, &dwData); */
	VNSvOutPortD(dwIoBase + MAC_REG_CAM + 4, *((PDWORD)(pbyMask + 4)));
	/* VNSvInPortD(dwIoBase + MAC_REG_CAM + 4, &dwData); */

	/* restore to original CAMCR */
	VNSvOutPortB(dwIoBase + MAC_REG_CAMCR, byOrgCAMCR);
}

VOID GMACvGetVCAMMask(DWORD dwIoBase, PBYTE pbyMask)
{
	BYTE    byOrgCAMCR, byData;

	/* modify CAMCR to select CAM MASK regs */
	VNSvInPortB(dwIoBase + MAC_REG_CAMCR, &byOrgCAMCR);
	byData = (BYTE)((byOrgCAMCR & ~CAMCR_PS1) | CAMCR_PS0);
	VNSvOutPortB(dwIoBase + MAC_REG_CAMCR, byData);

	/* select VCAM Mask */
	VNSvOutPortB(dwIoBase + MAC_REG_CAMADDR, CAMADDR_VCAMSL);

	VNSvInPortD(dwIoBase + MAC_REG_CAM, (PDWORD)pbyMask);
	VNSvInPortD(dwIoBase + MAC_REG_CAM + 4, (PDWORD)(pbyMask + 4));

	/* restore to original CAMCR */
	VNSvOutPortB(dwIoBase + MAC_REG_CAMCR, byOrgCAMCR);
}

VOID GMACvGetMCAMMask(DWORD dwIoBase, PBYTE pbyMask)
{
	BYTE    byOrgCAMCR, byData;

	/* modify CAMCR to select CAM MASK regs */
	VNSvInPortB(dwIoBase + MAC_REG_CAMCR, &byOrgCAMCR);
	byData = (BYTE)((byOrgCAMCR & ~CAMCR_PS1) | CAMCR_PS0);
	VNSvOutPortB(dwIoBase + MAC_REG_CAMCR, byData);

	/* Select MCAM Mask */
	VNSvOutPortB(dwIoBase + MAC_REG_CAMADDR, 0);

	/* select CAM and read CAM mask data (don't need to set CAMEN) */
	VNSvOutPortB(dwIoBase + MAC_REG_CAMADDR, 0);
	VNSvInPortD(dwIoBase + MAC_REG_CAM, (PDWORD)pbyMask);
	VNSvInPortD(dwIoBase + MAC_REG_CAM + 4, (PDWORD)(pbyMask + 4));

	/* restore to original CAMCR */
	VNSvOutPortB(dwIoBase + MAC_REG_CAMCR, byOrgCAMCR);
}

VOID GMACvSetVCAMMaskByBR(DWORD dwIoBase, PBYTE pbyMask)
{
	BYTE    byOrgCAMCR, byData;

	/* modify CAMCR to select CAM MASK regs */
	VNSvInPortB(dwIoBase + MAC_REG_CAMCR, &byOrgCAMCR);
	byData = (BYTE)((byOrgCAMCR & ~CAMCR_PS1) | CAMCR_PS0);
	VNSvOutPortB(dwIoBase + MAC_REG_CAMCR, byData);

	/* Select VCAM Mask */
	VNSvOutPortB(dwIoBase + MAC_REG_CAMADDR, CAMADDR_VCAMSL);

	UINT    uu;
	for (uu = 0; uu < 8; uu++) {
		VNSvOutPortB(dwIoBase + MAC_REG_CAM + uu, *(pbyMask + uu));
		/* PATCH...Read back for daley */
		/* VNSvInPortB(dwIoBase + MAC_REG_CAM + uu, &byData); */
	}

	/* restore to original CAMCR */
	VNSvOutPortB(dwIoBase + MAC_REG_CAMCR, byOrgCAMCR);
}

VOID GMACvSetMCAMMaskByBR(DWORD dwIoBase, PBYTE pbyMask)
{
	BYTE    byOrgCAMCR, byData;

	/* modify CAMCR to select CAM MASK regs */
	VNSvInPortB(dwIoBase + MAC_REG_CAMCR, &byOrgCAMCR);
	byData = (BYTE)((byOrgCAMCR & ~CAMCR_PS1) | CAMCR_PS0);
	VNSvOutPortB(dwIoBase + MAC_REG_CAMCR, byData);

	/* Select MCAM Mask */
	VNSvOutPortB(dwIoBase + MAC_REG_CAMADDR, 0);

	UINT    uu;
	for (uu = 0; uu < 8; uu++) {
		VNSvOutPortB(dwIoBase + MAC_REG_CAM + uu, *(pbyMask + uu));
		/* PATCH...Read back for delay */
		/* VNSvInPortB(dwIoBase + MAC_REG_CAM + uu, &byData); */
	}

	/* restore to original CAMCR */
	VNSvOutPortB(dwIoBase + MAC_REG_CAMCR, byOrgCAMCR);
}

VOID GMACvGetVCAMMaskByBR(DWORD dwIoBase, PBYTE pbyMask)
{
	BYTE    byOrgCAMCR, byData;

	/* modify CAMCR to select CAM MASK regs */
	VNSvInPortB(dwIoBase + MAC_REG_CAMCR, &byOrgCAMCR);
	byData = (BYTE)((byOrgCAMCR & ~CAMCR_PS1) | CAMCR_PS0);
	VNSvOutPortB(dwIoBase + MAC_REG_CAMCR, byData);

	/* select VCAM Mask */
	VNSvOutPortB(dwIoBase + MAC_REG_CAMADDR, CAMADDR_VCAMSL);

	UINT    uu;
	for (uu = 0; uu < 8; uu++)
		VNSvInPortB(dwIoBase + MAC_REG_CAM + uu, (pbyMask + uu));

	/* restore to original CAMCR */
	VNSvOutPortB(dwIoBase + MAC_REG_CAMCR, byOrgCAMCR);
}

VOID GMACvGetMCAMMaskByBR(DWORD dwIoBase, PBYTE pbyMask)
{
	BYTE    byOrgCAMCR, byData;

	/* modify CAMCR to select CAM MASK regs */
	VNSvInPortB(dwIoBase + MAC_REG_CAMCR, &byOrgCAMCR);
	byData = (BYTE)((byOrgCAMCR & ~CAMCR_PS1) | CAMCR_PS0);
	VNSvOutPortB(dwIoBase + MAC_REG_CAMCR, byData);

	/* Select MCAM Mask */
	VNSvOutPortB(dwIoBase + MAC_REG_CAMADDR, 0);

	UINT    uu;
	for (uu = 0; uu < 8; uu++)
		VNSvInPortB(dwIoBase + MAC_REG_CAM + uu, (pbyMask + uu));

	/* restore to original CAMCR */
	VNSvOutPortB(dwIoBase + MAC_REG_CAMCR, byOrgCAMCR);
}

VOID GMACvSetTqIndex(DWORD dwIoBase, BYTE byTxQue, WORD wTdIdx)
{
	BYTE    byData;

	/* Clear RUN */
	VNSvOutPortB(dwIoBase + MAC_REG_TDCSR_CLR + (byTxQue / 2), (BYTE)(TRDCSR_RUN << ((byTxQue % 2)*4)));

	/* Wait for RUN clear */
	while (TRUE) {
		VNSvInPortB(dwIoBase + MAC_REG_TDCSR_SET + (byTxQue / 2), &byData);
		if (BITbIsBitOff(byData, TRDCSR_RUN << ((byTxQue % 2)*4)))
			break;
	}

	/* Set TdIdx */
	VNSvOutPortW(dwIoBase + MAC_REG_TDINDX + byTxQue*2, wTdIdx);
	/* Set RUN */
	VNSvOutPortB(dwIoBase + MAC_REG_TDCSR_SET + (byTxQue / 2), (BYTE)(TRDCSR_RUN << ((byTxQue % 2)*4)));
}

void dumpmac(int iobase)
{
	int i = 0;
	DWORD data;
	for (i = 0; i < 0x80; i += 4) {
		VNSvInPortD(iobase+i, &data);
		printf("mac %8x:%8x\n", i, data);
	}
	return;
}
