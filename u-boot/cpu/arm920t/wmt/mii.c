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
#if !defined(__MAC_H__)
#include "mac.h"
#endif
#if !defined(__MII_H__)
#include "mii.h"
#endif

/*---------------------  Static Definitions -------------------------*/

/*---------------------  Static Classes  ----------------------------*/

/*---------------------  Static Variables  --------------------------*/

/*---------------------  Static Functions  --------------------------*/

/*---------------------  Export Variables  --------------------------*/

/*---------------------  Export Functions  --------------------------*/

BOOL
MIIbIsAutoNegotiationOn(
	DWORD   dwIoBase,
	BYTE    byRevId
	)
{
	return MIIbIsRegBitsOn(dwIoBase, byRevId, MII_REG_BMCR, BMCR_AUTO);
}

BOOL
MIIbIsIn100MMode(
	DWORD   dwIoBase,
	BYTE    byRevId
	)
{
	if (MIIbIsAutoNegotiationOn(dwIoBase, byRevId)) {
		/* If my TX_FD on, check both TX_FD */
		if (MIIbIsRegBitsOn(dwIoBase, byRevId, MII_REG_ANAR, ANAR_TXFD)) {
			/* partner's TX_FD */
			if (MIIbIsRegBitsOn(dwIoBase, byRevId, MII_REG_ANLPAR, ANLPAR_TXFD))
				return TRUE;
		}

		/* If my T4 on, check both T4 */
		if (MIIbIsRegBitsOn(dwIoBase, byRevId, MII_REG_ANAR, ANAR_T4)) {
			/* partner's T4 */
			if (MIIbIsRegBitsOn(dwIoBase, byRevId, MII_REG_ANLPAR, ANLPAR_T4))
				return TRUE;
		}

		/* If my TX_HD on, check both TX_HD */
		if (MIIbIsRegBitsOn(dwIoBase, byRevId, MII_REG_ANAR, ANAR_TX)) {
			/* partner's TX_HD */
			if (MIIbIsRegBitsOn(dwIoBase, byRevId, MII_REG_ANLPAR, ANLPAR_TX))
				return TRUE;
		}

		/* If my 10_FD on, check both 10_FD */
		if (MIIbIsRegBitsOn(dwIoBase, byRevId, MII_REG_ANAR, ANAR_10FD)) {
			/* partner's 10_FD */
			if (MIIbIsRegBitsOn(dwIoBase, byRevId, MII_REG_ANLPAR, ANLPAR_10FD))
				return FALSE;
		}

		/* If all above is not, then it would be 10_HD or no link */
		/* both case will be 10 Mb */
		return FALSE;
	} else {
		/* If in force mode */
		if (MIIbIsRegBitsOff(dwIoBase, byRevId, MII_REG_BMCR, BMCR_SPEED))
			return FALSE;
	}

	return TRUE;
}

BOOL
MIIbIsInFullDuplexMode(
	DWORD   dwIoBase,
	BYTE    byRevId
	)
{
	if (MIIbIsAutoNegotiationOn(dwIoBase, byRevId)) {
		/* if my TX_FD on, check both TX_FD */
		if (MIIbIsRegBitsOn(dwIoBase, byRevId, MII_REG_ANAR, ANAR_TXFD)) {
			/* partner's TX_FD */
			if (MIIbIsRegBitsOn(dwIoBase, byRevId, MII_REG_ANLPAR, ANLPAR_TXFD))
				return TRUE;
		}

		/* if my T4 on, check both T4 */
		if (MIIbIsRegBitsOn(dwIoBase, byRevId, MII_REG_ANAR, ANAR_T4)) {
			/* partner's T4 */
			if (MIIbIsRegBitsOn(dwIoBase, byRevId, MII_REG_ANLPAR, ANLPAR_T4))
				return FALSE;
		}

		/* if my TX_HD on, check both TX_HD */
		if (MIIbIsRegBitsOn(dwIoBase, byRevId, MII_REG_ANAR, ANAR_TX)) {
			/* partner's TX_HD */
			if (MIIbIsRegBitsOn(dwIoBase, byRevId, MII_REG_ANLPAR, ANLPAR_TX))
				return FALSE;
		}

		/* if my 10_FD on, check both 10_FD */
		if (MIIbIsRegBitsOn(dwIoBase, byRevId, MII_REG_ANAR, ANAR_10FD)) {
			/* partner's 10_FD */
			if (MIIbIsRegBitsOn(dwIoBase, byRevId, MII_REG_ANLPAR, ANLPAR_10FD))
				return TRUE;
		}

		/* if all above is not, then it would be 10_HD or no link */
		/* both case will be half duplex */
		return FALSE;
	} else {
		/* if in force mode */
		if (MIIbIsRegBitsOff(dwIoBase, byRevId, MII_REG_BMCR, BMCR_FDX))
			return FALSE;
	}

	return TRUE;
}

BOOL
MIIbIsRegBitsOff(
	DWORD   dwIoBase,
	BYTE    byRevId,
	BYTE    byMiiAddr,
	WORD    wTestBits
	)
{
	static WORD wOrgData;

	MIIbReadEmbedded(dwIoBase, byRevId, byMiiAddr, &wOrgData);
	return BITbIsAllBitsOff(wOrgData, wTestBits);
}

BOOL
MIIbIsRegBitsOn(
	DWORD   dwIoBase,
	BYTE    byRevId,
	BYTE    byMiiAddr,
	WORD    wTestBits
	)
{
	static WORD wOrgData;

	MIIbReadEmbedded(dwIoBase, byRevId, byMiiAddr, &wOrgData);
	return BITbIsAllBitsOn(wOrgData, wTestBits);
}


BOOL
MIIbReadEmbedded(
	DWORD   dwIoBase,
	BYTE    byRevId,
	BYTE    byMiiAddr,
	PWORD   pwData
	)
{
	WORD    ww;

	/* Disable MIICR_MAUTO, so that mii addr can be set normally */
	MACvSafeDisableMiiAutoPoll(dwIoBase, byRevId);

	/* MII reg offset */
	VNSvOutPortB(dwIoBase + MAC_REG_MIIAD, byMiiAddr);

	/* Turn on MIICR_RCMD */
	MACvRegBitsOn(dwIoBase, MAC_REG_MIICR, MIICR_RCMD);
	/* W_MAX_TIMEOUT is the timeout period */
	for (ww = 0; ww < W_MAX_TIMEOUT; ww++) {
		if (BITbIsBitOff(MACbyReadMIICR(dwIoBase), MIICR_RCMD))
			break;
	}

	/* get MII data */
	VNSvInPortW(dwIoBase + MAC_REG_MIIDATA, pwData);

	MACvEnableMiiAutoPoll(dwIoBase);

	if (ww == W_MAX_TIMEOUT)
		return FALSE;

	return TRUE;
}

BOOL
MIIbWriteEmbedded(
	DWORD   dwIoBase,
	BYTE    byRevId,
	BYTE    byMiiAddr,
	WORD    wData
	)
{
	WORD    ww;

	/* disable MIICR_MAUTO, so that mii addr can be set normally */
	MACvSafeDisableMiiAutoPoll(dwIoBase, byRevId);

	/* MII reg offset */
	VNSvOutPortB(dwIoBase + MAC_REG_MIIAD, byMiiAddr);
	/* set MII data */
	VNSvOutPortW(dwIoBase + MAC_REG_MIIDATA, wData);

	/* turn on MIICR_WCMD */
	MACvRegBitsOn(dwIoBase, MAC_REG_MIICR, MIICR_WCMD);
	/* W_MAX_TIMEOUT is the timeout period */
	for (ww = 0; ww < W_MAX_TIMEOUT; ww++) {
		if (BITbIsBitOff(MACbyReadMIICR(dwIoBase), MIICR_WCMD))
			break;
	}

	MACvEnableMiiAutoPoll(dwIoBase);

	if (ww == W_MAX_TIMEOUT)
		return FALSE;

	return TRUE;
}

VOID
MIIvInitialize(
	DWORD   dwIoBase,
	BYTE    byRevId,
	DWORD   dwPhyCmrId
	)
{
	WORD    wOrgData;

	/* PHY company/module, don't care revision id */
	dwPhyCmrId &= CID_REV_ID_MASK_OFF;

	switch (dwPhyCmrId) {
	case CID_NS:
	case CID_MYSON:
		/* for MYSON turn on LED to indicate full-duplex in 10M mode */
		MIIvRegBitsOn(dwIoBase, byRevId, MII_REG_PCR, PCR_LED4MODE);
		break;

	case CID_ESI:
		/* for ESI, set TX/ACT LED option to ACT */
		MIIvRegBitsOn(dwIoBase, byRevId, MII_REG_PCSR, PCSR_LEDON4ACT);
		break;

	default:
		break;
	}

	/* if ISO is on, turn it off */
	MIIbReadEmbedded(dwIoBase, byRevId, MII_REG_BMCR, &wOrgData);
	if (BITbIsBitOn(wOrgData, BMCR_ISO)) {
		wOrgData &= ~BMCR_ISO;
		MIIbWriteEmbedded(dwIoBase, byRevId, MII_REG_BMCR, wOrgData);
	}
}

VOID
MIIvSetLoopbackMode(
	DWORD   dwIoBase,
	BYTE    byRevId,
	BYTE    byLoopbackMode
	)
{
	/* handle AUTO-NEGO */
	switch (byLoopbackMode) {
	case MII_LB_NONE:
		/* turn off MII loopback */
		MIIvRegBitsOff(dwIoBase, byRevId, MII_REG_BMCR, BMCR_LBK);
		/* must turn on AUTO-NEGO after turn off BMCR_LBK, otherwise */
		/* the AUTO-NEGO process will never end in some PHY (e.g. ESI...) */
		MIIvSetAutoNegotiationOn(dwIoBase, byRevId);
		break;

	case MII_LB_INTERNAL:
		MIIvSetAutoNegotiationOff(dwIoBase, byRevId);
		MIIvRegBitsOn(dwIoBase, byRevId, MII_REG_BMCR, BMCR_LBK);
		/* select 100M */
		MIIvRegBitsOn(dwIoBase, byRevId, MII_REG_BMCR, BMCR_SPEED);
		break;

	case MII_LB_ISO:
		/* we achieve isolation by do loopback in MII not in Transceiver */
		MIIvSetAutoNegotiationOff(dwIoBase, byRevId);
		MIIvRegBitsOn(dwIoBase, byRevId, MII_REG_BMCR, BMCR_LBK);
		/* select 100M */
		MIIvRegBitsOn(dwIoBase, byRevId, MII_REG_BMCR, BMCR_SPEED);
		break;

	default:
		break;
	}

	/* wait for MII loopback to stable */
	PCAvDelayByIO(CB_DELAY_MII_LB_STABLE);
}

VOID
MIIvWaitForNwayCompleted(
	DWORD   dwIoBase,
	BYTE    byRevId
	)
{
	UINT    uu;

	/* delay about 8 sec.                                        */
	/* wait for AUTO-NEGO to complete (may never complete if     */
	/* no link), after that then, we can get correct link status */
	/* NOTE: because I have no timer, so looping is used,        */
	/*       if timer is available in your OS, use timer         */
	/*       in stead of looping */

	/* wait for BMSR_AUTOCM to off */
	for (uu = 0; uu < CB_MAX_COUNT_AUTO_COMPLETE; uu++) {
		if (MIIbIsRegBitsOff(dwIoBase, byRevId, MII_REG_BMSR, BMSR_AUTOCM))
			break;
	}
	/* wait for BMSR_AUTOCM to on */
	/* NOTE.... read BMSR_AUTOCM bit will also clear BMSR_LNK bit */
	for (uu = 0; uu < CB_MAX_COUNT_AUTO_COMPLETE; uu++) {
		/* if AUTO-NEGO completed, then go out this loop */
		/* otherwise, count down to time out */
		if (MIIbIsRegBitsOn(dwIoBase, byRevId, MII_REG_BMSR, BMSR_AUTOCM))
			break;
	}
}
