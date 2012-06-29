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

/* #include <conio.h> */

/* #if !defined(__UPC_H__) */
/* #include "upc.h" */
/* #endif */
#if !defined(__UASSERT_H__)
#include "uassert.h"
#endif
/* #if !defined(__TMACRO_H__) */
/* #include "tmacro.h" */
/* #endif */
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
#define CB_DELAY_MII_LB_STABLE      (1000)

/*---------------------  Static Classes  ----------------------------*/

/*---------------------  Static Variables  --------------------------*/

/*---------------------  Static Functions  --------------------------*/

/*---------------------  Export Variables  --------------------------*/

/*---------------------  Export Functions  --------------------------*/

/*
 * Description: Read a word from MII, by embeded programming
 *
 * Parameters:
 *  In:
 *		dwIoBase	- I/O base address
 *		byRevId     - chip revision id
 *		byMiiAddr	- address of register in MII
 *  Out:
 *		pwData		- data read
 *
 * Return Value: TRUE if succeeded; FALSE if failed.
 *
 */
BOOL GMIIbReadEmbeded(DWORD dwIoBase, BYTE byRevId, BYTE byMiiAddr, PWORD pwData)
{
	byRevId = byRevId;
	WORD    ww;

	/* disable MIICR_MAUTO, so that mii addr can be set normally */
	GMACvDisableMiiAutoPoll(dwIoBase);

	/* MII reg offset */
	VNSvOutPortB(dwIoBase + MAC_REG_MIIADR, byMiiAddr);

	/* turn on MIICR_RCMD */
	MACvRegBitsOn(dwIoBase, MAC_REG_MIICR, MIICR_RCMD);

	/* W_MAX_TIMEOUT is the timeout period */
	for (ww = 0; ww < W_MAX_TIMEOUT; ww++) {
		udelay(1000);
		if (BITbIsBitOff(GMACbyReadMIICR(dwIoBase), MIICR_RCMD))
			break;
	}

	/* get MII data */
	VNSvInPortW(dwIoBase + MAC_REG_MIIDATA, pwData);

	GMACvEnableMiiAutoPoll(dwIoBase);

	if (ww == W_MAX_TIMEOUT)
		return FALSE;

	return TRUE;
}

/*
 * Description: Write a word to MII, by embeded programming
 *
 * Parameters:
 *  In:
 *		dwIoBase	- I/O base address
 *		byRevId     - chip revision id
 *		byMiiAddr	- address of register in MII
 *		wData		- data to write
 *
 * Return Value: TRUE if succeeded; FALSE if failed.
 *
 */
BOOL GMIIbWriteEmbeded(DWORD dwIoBase, BYTE byRevId, BYTE byMiiAddr, WORD wData)
{
	byRevId = byRevId;
	WORD    ww;

	/* disable MIICR_MAUTO, so that mii addr can be set normally */
	GMACvDisableMiiAutoPoll(dwIoBase);

	/* MII reg offset */
	VNSvOutPortB(dwIoBase + MAC_REG_MIIADR, byMiiAddr);

	/* set MII data */
	VNSvOutPortW(dwIoBase + MAC_REG_MIIDATA, wData);

	/* turn on MIICR_WCMD */
	MACvRegBitsOn(dwIoBase, MAC_REG_MIICR, MIICR_WCMD);

	/* W_MAX_TIMEOUT is the timeout period */
	for (ww = 0; ww < W_MAX_TIMEOUT; ww++) {
		udelay(1000);
		if (BITbIsBitOff(GMACbyReadMIICR(dwIoBase), MIICR_WCMD))
			break;
	}

	GMACvEnableMiiAutoPoll(dwIoBase);

	if (ww == W_MAX_TIMEOUT)
		return FALSE;

	return TRUE;
}

BOOL GMIIbIsRegBitsOn(DWORD dwIoBase, BYTE byRevId, BYTE byMiiAddr, WORD wTestBits)
{
	static WORD wOrgData;

	GMIIbReadEmbeded(dwIoBase, byRevId, byMiiAddr, &wOrgData);
	return BITbIsAllBitsOn(wOrgData, wTestBits);
}

BOOL GMIIbIsRegBitsOff(DWORD dwIoBase, BYTE byRevId, BYTE byMiiAddr, WORD wTestBits)
{
	static WORD wOrgData;

	GMIIbReadEmbeded(dwIoBase, byRevId, byMiiAddr, &wOrgData);
	return BITbIsAllBitsOff(wOrgData, wTestBits);
}

BOOL GMIIbReadAllRegs(DWORD dwIoBase, BYTE byRevId, PSMiiReg pMiiRegs)
{
	UINT    ii;
	PWORD   pwData;
	UINT    cbWords;

	pwData = (PWORD)pMiiRegs;
	cbWords = sizeof(SMiiReg) / sizeof(WORD);

	/* ii = Reg Address */
	for (ii = 0; ii < cbWords; ii++) {
		if (!GMIIbReadEmbeded(dwIoBase, byRevId, (BYTE)ii, pwData))
			return FALSE;
		pwData++;
	}

	return TRUE;
}

VOID GMIIvWaitForNwayCompleted(DWORD dwIoBase, BYTE byRevId)
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
		if (GMIIbIsRegBitsOff(dwIoBase, byRevId, MII_REG_BMSR, BMSR_AUTOCM))
			break;
	}
	/* wait for BMSR_AUTOCM to on */
	/* NOTE.... read BMSR_AUTOCM bit will also clear BMSR_LNK bit */
	for (uu = 0; uu < CB_MAX_COUNT_AUTO_COMPLETE; uu++) {
		/* if AUTO-NEGO completed, then go out this loop */
		/* otherwise, count down to time out */
		if (GMIIbIsRegBitsOn(dwIoBase, byRevId, MII_REG_BMSR, BMSR_AUTOCM))
			break;
	}
}

VOID GMIIvSetLoopbackMode(DWORD dwIoBase, BYTE byRevId, BYTE byLoopbackMode)
{
	/* handle AUTO-NEGO */
	switch (byLoopbackMode) {
	case MII_LB_NONE:
		if(GMIIbIsRegBitsOn(dwIoBase, byRevId, MII_REG_BMCR, BMCR_LBK)) {
			/* turn off MII loopback */
			MIIvRegBitsOff(dwIoBase, byRevId, MII_REG_BMCR, BMCR_LBK);
			/* must turn on AUTO-NEGO after turn off BMCR_LBK, otherwise */
			/* the AUTO-NEGO process will never end in some PHY (e.g. ESI...) */
			MIIvSetAutoNegotiationOn(dwIoBase, byRevId);
		}/* else do not thing */
		break;

	case MII_LB_INTERNAL:
		MIIvSetAutoNegotiationOff(dwIoBase, byRevId);
		MIIvRegBitsOn(dwIoBase, byRevId, MII_REG_BMCR, BMCR_LBK);
		/* select 1000M */
		MIIvRegBitsOn(dwIoBase, byRevId, MII_REG_BMCR, BMCR_SPEED1G);
		MIIvRegBitsOff(dwIoBase, byRevId, MII_REG_BMCR, BMCR_SPEED100);
		break;

	case MII_LB_ISO:
		/* we achieve isolation by do loopback in MII not in Transceiver */
		MIIvSetAutoNegotiationOff(dwIoBase, byRevId);
		MIIvRegBitsOn(dwIoBase, byRevId, MII_REG_BMCR, BMCR_LBK);
		/* select 1000M */
		MIIvRegBitsOn(dwIoBase, byRevId, MII_REG_BMCR, BMCR_SPEED1G);
		MIIvRegBitsOff(dwIoBase, byRevId, MII_REG_BMCR, BMCR_SPEED100);
		break;

	default:
		DBG_ASSERT(FALSE);
		break;
	}

	/* wait for MII loopback to stable */
	PCAvDelayByIO(CB_DELAY_MII_LB_STABLE);
}

VOID GMIIvSetDuplexMode(DWORD dwIoBase, BYTE byRevId, BOOL bFullDuplexOn)
{
	/* when force duplex mode, */
	/* AUTO-NEGO should off */
	MIIvSetAutoNegotiationOff(dwIoBase, byRevId);

	if (bFullDuplexOn)
		MIIvRegBitsOn(dwIoBase, byRevId, MII_REG_BMCR, BMCR_FDX);
	else
		MIIvRegBitsOff(dwIoBase, byRevId, MII_REG_BMCR, BMCR_FDX);
}

VOID GMIIvSetSpeedMode(DWORD dwIoBase, BYTE byRevId, BYTE bySpeed)
{
	/* when force speed mode, */
	/* AUTO-NEGO should off */
	MIIvSetAutoNegotiationOff(dwIoBase, byRevId);

	switch (bySpeed) {
	case MII_SPD_1000:
		MIIvRegBitsOn(dwIoBase, byRevId, MII_REG_BMCR, BMCR_SPEED1G);
		MIIvRegBitsOff(dwIoBase, byRevId, MII_REG_BMCR, BMCR_SPEED100);
		break;
	case MII_SPD_100:
		MIIvRegBitsOff(dwIoBase, byRevId, MII_REG_BMCR, BMCR_SPEED1G);
		MIIvRegBitsOn(dwIoBase, byRevId, MII_REG_BMCR, BMCR_SPEED100);
		break;
	case MII_SPD_10:
		MIIvRegBitsOff(dwIoBase, byRevId, MII_REG_BMCR, BMCR_SPEED1G);
		MIIvRegBitsOff(dwIoBase, byRevId, MII_REG_BMCR, BMCR_SPEED100);
		break;

	default:
		DBG_ASSERT(FALSE);
		break;
	}
}

BOOL GMIIbIsAutoNegotiationOn(DWORD dwIoBase, BYTE byRevId)
{
	return GMIIbIsRegBitsOn(dwIoBase, byRevId, MII_REG_BMCR, BMCR_AUTO);
}

/*
BOOL MIIbIsInFullDuplexMode(DWORD dwIoBase, BYTE byRevId)
{
	// if in AUTO-NEGO mode
	if (byRevId>=REV_ID_VT3106J_A0)
		return MACbIsRegBitsOn(dwIoBase, MAC_REG_MIISR, MIISR_N_FDX);

	if (MIIbIsAutoNegotiationOn(dwIoBase, byRevId)) {

		// if my TX_FD on, check both TX_FD
		if (MIIbIsRegBitsOn(dwIoBase, byRevId, MII_REG_ANAR, ANAR_TXFD)) {
			// partner's TX_FD
			if (MIIbIsRegBitsOn(dwIoBase, byRevId, MII_REG_ANLPAR, ANLPAR_TXFD))
				return TRUE;
		}

		// if my T4 on, check both T4
		if (MIIbIsRegBitsOn(dwIoBase, byRevId, MII_REG_ANAR, ANAR_T4)) {
			// partner's T4
			if (MIIbIsRegBitsOn(dwIoBase, byRevId, MII_REG_ANLPAR, ANLPAR_T4))
				return FALSE;
		}

		// if my TX_HD on, check both TX_HD
		if (MIIbIsRegBitsOn(dwIoBase, byRevId, MII_REG_ANAR, ANAR_TX)) {
			// partner's TX_HD
			if (MIIbIsRegBitsOn(dwIoBase, byRevId, MII_REG_ANLPAR, ANLPAR_TX))
				return FALSE;
		}

		// if my 10_FD on, check both 10_FD
		if (MIIbIsRegBitsOn(dwIoBase, byRevId, MII_REG_ANAR, ANAR_10FD)) {
			// partner's 10_FD
			if (MIIbIsRegBitsOn(dwIoBase, byRevId, MII_REG_ANLPAR, ANLPAR_10FD))
				return TRUE;
		}

		// if all above is not, then it would be 10_HD or no link
		// both case will be half duplex
		return FALSE;
	} else {
		// if in force mode
		if (MIIbIsRegBitsOff(dwIoBase, byRevId, MII_REG_BMCR, BMCR_FDX))
			return FALSE;
	}

	return TRUE;
}
*/

BOOL GMIIbSafeSoftwareReset(DWORD dwIoBase, BYTE byRevId)
{
	WORD    ww;

	/* turn on reset only, do not write other bits */
	GMIIbWriteEmbeded(dwIoBase, byRevId, MII_REG_BMCR, BMCR_RESET);

	/* polling till MII reset complete */
	/* W_MAX_TIMEOUT is the timeout period */
	for (ww = 0; ww < W_MAX_TIMEOUT; ww++) {
		if (GMIIbIsRegBitsOff(dwIoBase, byRevId, MII_REG_BMCR, BMCR_RESET))
			break;
	}

	if (ww == W_MAX_TIMEOUT)
		return FALSE;

	return TRUE;
}

/*
 * Description: Scan how many PHY in MII bus, and put PYH id
 *              in the pbyPhyId array
 *
 * Parameters:
 *  In:
 *		dwIoBase    - I/O base address
 *		byRevId     - chip revision id
 *  Out:
 *		puTotalPhyNum   - total # of PHY
 *		pbyPhyId        - PHY id array
 *
 *                        NOTE: size of pbyPhyId should larger than MAX_PHY_DEVICE
 *                              otherwise, memory will corrupt
 *
 * Return Value: TRUE if succeeded; FALSE if failed.
 *
 */
VOID GMIIvScanAllPhy(DWORD dwIoBase, BYTE byRevId, PBYTE pbyPhyId, PUINT puTotalPhyNum)
{
	BYTE    byOrgMIIADR;
	BYTE    byOrgMIICR;
	DWORD   dwPhyCmrId;
	UINT    uPhyArrayIdx;
	BYTE    byPhyId, byOrgPhyId;

	/* keep original MIICR value */
	VNSvInPortB(dwIoBase + MAC_REG_MIIADR, &byOrgMIIADR);
	VNSvInPortB(dwIoBase + MAC_REG_MIICR, &byOrgMIICR);

	/* disable MIICR_MAUTO, so that mii addr can be set normally */
	GMACvDisableMiiAutoPoll(dwIoBase);

	/* keep original phy ID */
	byOrgPhyId = GMACbyGetPhyId(dwIoBase);

	/* scan all PHY */
	uPhyArrayIdx = 0;
	/* PHI id 0 is not a valid id */
	for (byPhyId = 1; byPhyId < MAX_PHY_DEVICE; byPhyId++) {
		GMACvSetPhyId(dwIoBase, byRevId, byPhyId);

		/* detect if PHY exist? */
		MIIvReadPhyCmrId(dwIoBase, byRevId, &dwPhyCmrId);
		if ((dwPhyCmrId != 0xFFFFFFFFUL) && (dwPhyCmrId != 0UL)) {
			pbyPhyId[uPhyArrayIdx] = byPhyId;
			uPhyArrayIdx++;
		}
	}
	*puTotalPhyNum = uPhyArrayIdx;

	/* restore original phy id */
	GMACvSetPhyId(dwIoBase, byRevId, byOrgPhyId);

	/* restore MIICR */
	VNSvOutPortB(dwIoBase + MAC_REG_MIIADR, byOrgMIIADR);
	VNSvOutPortB(dwIoBase + MAC_REG_MIICR, byOrgMIICR);
}

VOID GMIIvSetActiveForcedPhy(DWORD dwIoBase, BYTE byRevId, PBYTE pbyPhyId, UINT uTotalPhyNum, BYTE byPhyId)
{
	UINT    uu;
	BYTE    byOrgMIIADR;
	BYTE    byOrgMIICR;

	/* keep original MIICR value */
	VNSvInPortB(dwIoBase + MAC_REG_MIIADR, &byOrgMIIADR);
	VNSvInPortB(dwIoBase + MAC_REG_MIICR, &byOrgMIICR);

	/* disable MIICR_MAUTO, so that mii addr can be set normally */
	GMACvDisableMiiAutoPoll(dwIoBase);

	/* iso all PHY */
	for (uu = 0; uu < uTotalPhyNum; uu++) {
		GMACvSetPhyId(dwIoBase, byRevId, pbyPhyId[uu]);
		MIIvRegBitsOn(dwIoBase, byRevId, MII_REG_BMCR, BMCR_ISO);
	}

	/* finally, set correct PHY id */
	GMACvSetPhyId(dwIoBase, byRevId, byPhyId);
	/* active this PHY */
	MIIvRegBitsOff(dwIoBase, byRevId, MII_REG_BMCR, BMCR_ISO);

	/* restore MIICR */
	VNSvOutPortB(dwIoBase + MAC_REG_MIIADR, byOrgMIIADR);
	VNSvOutPortB(dwIoBase + MAC_REG_MIICR, byOrgMIICR);
}

/* if no PHY linked, return FALSE */
BOOL GMIIbSetActiveLinkedPhy(DWORD dwIoBase, BYTE byRevId, PBYTE pbyPhyId, UINT uTotalPhyNum)
{
	BOOL    bRetVal = FALSE;
	UINT    uu;
	BYTE    byActPhyId, byOrgPhyId;
	BYTE    byOrgMIIADR;
	BYTE    byOrgMIICR;

	/* keep original MIICR value */
	VNSvInPortB(dwIoBase + MAC_REG_MIIADR, &byOrgMIIADR);
	VNSvInPortB(dwIoBase + MAC_REG_MIICR, &byOrgMIICR);

	/* disable MIICR_MAUTO, so that mii addr can be set normally */
	GMACvDisableMiiAutoPoll(dwIoBase);

	byOrgPhyId = GMACbyGetPhyId(dwIoBase);

	for (uu = 0; uu < uTotalPhyNum; uu++) {
		GMACvSetPhyId(dwIoBase, byRevId, pbyPhyId[uu]);

		if (GMIIbIsRegBitsOn(dwIoBase, byRevId, MII_REG_BMSR, BMSR_LNK)) {

			byActPhyId = pbyPhyId[uu];

			/* iso all PHY */
			for (uu = 0; uu < uTotalPhyNum; uu++) {
				GMACvSetPhyId(dwIoBase, byRevId, pbyPhyId[uu]);
				MIIvRegBitsOn(dwIoBase, byRevId, MII_REG_BMCR, BMCR_ISO);
			}

			/* finally, set correct PHY id */
			GMACvSetPhyId(dwIoBase, byRevId, byActPhyId);
			/* active this PHY */
			MIIvRegBitsOff(dwIoBase, byRevId, MII_REG_BMCR, BMCR_ISO);
			bRetVal = TRUE;
			break;
		}
	}

	if (!bRetVal)
		/* if no link, set original PHY id */
		GMACvSetPhyId(dwIoBase, byRevId, byOrgPhyId);

	/* restore MIICR */
	VNSvOutPortB(dwIoBase + MAC_REG_MIIADR, byOrgMIIADR);
	VNSvOutPortB(dwIoBase + MAC_REG_MIICR, byOrgMIICR);
	return bRetVal;
}

VOID GMIIvInitialize(DWORD dwIoBase, BYTE byRevId, DWORD dwPhyCmrId)
{
	WORD    wOrgData;

	/* PHY company/module, don't care revision id */
	dwPhyCmrId &= CID_REV_ID_MASK_OFF;

	switch (dwPhyCmrId) {
	case CID_CICADA_CIS8201:
		/* NOTICE!! For CICADA CS8201 PHY */
		/* Turn on AUX_MODE (bit 2) in MII AUX register (offset 1Ch) */
		MIIvRegBitsOn(dwIoBase, byRevId, MII_REG_AUXCSR, AUXCSR_MDPPS);

		/* Turn on bit 2 in offset 0x1B, this patch code only adopt to CIS8201,
		   not for integrated PHY in VT3216 */
		MIIvRegBitsOn(dwIoBase, byRevId, MII_REG_LED, LED_LALBE);

		break;

	case CID_CICADA_CIS3216I:
	case CID_CICADA_CIS3216I64:
		break;

	case CID_MARVELL_1000:
	case CID_MARVELL_1000S:
		/* Assert CRS on Transmit */
		MIIvRegBitsOn(dwIoBase, byRevId, MII_REG_PSCR, PSCR_ACRSTX);
		break;

	default:
		break;
	}

	/* if ISO is on, turn it off */
	GMIIbReadEmbeded(dwIoBase, byRevId, MII_REG_BMCR, &wOrgData);
	if (BITbIsBitOn(wOrgData, BMCR_ISO)) {
		wOrgData &= ~BMCR_ISO;
		GMIIbWriteEmbeded(dwIoBase, byRevId, MII_REG_BMCR, wOrgData);
	}
}
