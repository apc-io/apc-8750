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


/* modified by kevin */
/*#ifndef _GLOBAL_H_
#include "global.h"
#endif
#ifndef _EXTVARS_H_
#include "extvars.h"
#endif*/

#if !defined(__UPC_H__)
#include "upc.h"
#endif
#if !defined(__CARD_H__)
#include "card.h"
#endif
#if !defined(__EMACISR_H__)
/* #include "emacisr.h" */
#endif
#if !defined(__ALLOCT_H__)
#include "alloct.h"
#endif
#if !defined(__VPCI_H__)
#include "vpci.h"
#endif
#if !defined(__MAC_H__)
#include "mac.h"
#endif
#if !defined(__ADAPTER_H__)
#include "adapter.h"
#endif
#include "macif.h"
#if !defined(__DBG_H__)
#include "dbg.h"
#endif

/*---------------------  Static Definitions -------------------------*/

/*---------------------  Static Classes  ----------------------------*/

/*---------------------  Static Variables  --------------------------*/

/*---------------------  Static Functions  --------------------------*/
static void s_vInit(PSAdapterInfo pAdapter, UINT uTotalNum, UINT uIdx);

/*---------------------  Export Variables  --------------------------*/
PSAdapterInfo    sg_aAdapter;
SAdapterOpts     g_sOptions = {0, 0, 0, 0, 0, 0};

/*---------------------  Export Functions  --------------------------*/
BOOL
ADPbBind(
    PSAdapterInfo pAdapter
    )
{
	/* build RDR/TDR */
	if (!ALCbAllocateRdrMemory(pAdapter)) {
		printf("allocate Buffer or RD fail!\n");
		return FALSE;
	}
	if (!ALCbAllocateTdrMemory(pAdapter)) {
		printf("allocate Buffer or TD fail!\n");
		return FALSE;
	}

	ALCvChainRdrMemory(pAdapter);
	ALCvChainTdrMemory(pAdapter);
	/* Mac init & mii init */
	if (!ADPbInitializeEx(pAdapter)) {
		printf("init ex fail\n");
		return FALSE;
	}
	/* Hook Interrupt Service Routine on IRQ */
	/* set_irq_handlers(pAdapter->byIrqLevel, pAdapter->pvAdapterIsr); */

	/* Enable Interrupt Mask */
	/* unmask_interrupt(pAdapter->byIrqLevel); */

	/* MacDump(MACDBG_INFO, ("[ADPbBind] - Hook ISR on IRQ%d\n", pAdapter->byIrqLevel)); */

	/* Start ISR */
	MACvIntEnable(pAdapter->dwIoBase, pAdapter->byRevId, IMR_MASK_VALUE);

	/* Wait MAUTO to poll twice, then MIISR_LNKFL will be correct status */
	/* kevin???? */
	PCAvDelayByIO(CB_DELAY_MII_STABLE * 20000);
	/* Update link status */
	pAdapter->bLinkPass = MACbIsCableLinkOk(pAdapter->dwIoBase);
	if (pAdapter->bLinkPass) {
		MacDump(MACDBG_INFO, ("[ADPbBind]- Link Pass\n"));

		/* update duplex status */
		/* NOTE.... here we don't call MIIbIsInFullDuplexMode(), because */
		/* we won't turn on/off MAUTO */
		MACvIsInFullDuplexMode(&pAdapter->bFullDuplex, pAdapter->dwIoBase);
		/* Update speed status */
		pAdapter->bSpeed100M = MACbIsIn100MMode(pAdapter->dwIoBase, pAdapter->byRevId);
	} else
		MacDump(MACDBG_INFO, ("[ADPbBind]- Fail\n"));

	pAdapter->pbyTmpBuff = (PBYTE)mALLOc(CB_MAX_BUF_SIZE);
	MacDump(MACDBG_INFO, ("[ADPbBind]- Exit!\n"));

	return TRUE;
}

static BOOL ADPbDynaAllocBuf(
    PSAdapterInfo pAdapter
    )
{
	/* Allocate RD/TD pointer array & DescBuf SAllocMap array. */
	/* modified by kevin:mALLOc supplied by u-boot */
	pAdapter->apRD = (PSRxDesc *)mALLOc(sizeof(PSRxDesc) * pAdapter->cbRD);
	MacDump(MACDBG_INFO, ("[ADPbDynaAllocBuf] - pAdapter->apRD : %p, size : %d\n", pAdapter->apRD,
		sizeof(PSRxDesc)*pAdapter->cbRD));
	pAdapter->aamRxDescBuf = (PSAllocMap)mALLOc(sizeof(SAllocMap) * pAdapter->cbRD);
	MacDump(MACDBG_INFO, ("[ADPbDynaAllocBuf] - pAdapter->aamRxDescBuf : %p, size : %d\n",
		pAdapter->aamRxDescBuf, sizeof(SAllocMap) * pAdapter->cbRD));

	pAdapter->apTD = (PSTxDesc *)mALLOc(sizeof(PSTxDesc) * pAdapter->cbTD);
	MacDump(MACDBG_INFO, ("[ADPbDynaAllocBuf] - pAdapter->apTD : %p, size : %d\n", pAdapter->apTD,
		sizeof(PSTxDesc)*pAdapter->cbTD));
	pAdapter->aamTxDescBuf = (PSAllocMap)mALLOc(sizeof(SAllocMap) * pAdapter->cbTD);
	MacDump(MACDBG_INFO, ("[ADPbDynaAllocBuf] - pAdapter->aamTxDescBuf : %p, size : %d\n",
		pAdapter->aamTxDescBuf, sizeof(SAllocMap) * pAdapter->cbTD));

	if ((pAdapter->apRD == NULL) || (pAdapter->apTD == NULL) ||
	(pAdapter->aamTxDescBuf == NULL) || (pAdapter->aamRxDescBuf == NULL))
		return FALSE;

	return TRUE;
}

BOOL
ADPbInitializeEx(
    PSAdapterInfo pAdapter
    )
{
	BYTE    byOrgData, byPMRegOffset;

	/* Fill default value */
	/* marked by kevin:repeat again */
	/* ALCvSetDefaultRD(pAdapter); */
	/* ALCvSetDefaultTD(pAdapter); */

	/* Init PCI */
	VPCIvInitialize(pAdapter->dwIoBase, pAdapter->byRevId);

	/* Write-clear PME status bit */
	VPCIvReadB(pAdapter->dwIoBase, PCI_REG_CAP, &byPMRegOffset);
	VPCIvReadB(pAdapter->dwIoBase, (BYTE)(byPMRegOffset + PM_PMCSR1), &byOrgData);
	VPCIvWriteB(pAdapter->dwIoBase, (BYTE)(byPMRegOffset + PM_PMCSR1),
		(BYTE)(PMCSR1_PME_STATUS | byOrgData));

	if (!g_sOptions.bNoInit) {
		/* Init MAC */
		MACvInitialize(pAdapter->dwIoBase, pAdapter->byRevId);

		/* PATCH.... */
		/* PCI speed is too slow on some mainboard will cause */
		/* tx fifo underflow, so set tx threshold to SF */
		/* set tx threshold, SF */
		/* Masked by Ray for WMT */
		/* MACvSetTxThreshold(pAdapter->dwIoBase, 7); */

		/* Set address filter to accept any packet */
		/* MACvSetPacketFilter(pAdapter->dwIoBase, PKT_TYPE_PROMISCUOUS); */

		/* Detect PHY is from which company */
		MIIvReadPhyCmrId(pAdapter->dwIoBase, pAdapter->byRevId, &pAdapter->dwPhyCmrId);
		/* Init MII */
		MIIvInitialize(pAdapter->dwIoBase, pAdapter->byRevId, pAdapter->dwPhyCmrId);

		CARDvSetLoopbackMode(pAdapter->dwIoBase, pAdapter->byRevId, CARD_LB_NONE);
		/* set mii NWAY or AUTO */
		CARDvSetMediaLinkMode(pAdapter->dwIoBase, pAdapter->byRevId, pAdapter->uConnectionType);

		if (pAdapter->uConnectionType == MEDIA_AUTO) {
			/* Restart AUTO-NEGO to make sure AUTO-NEGO is correct */
			/* Modified by Ray, Not sure if this instruction is wrong. */
			/* MIIvSetResetOn(pAdapter->dwIoBase, pAdapter->byRevId); */
			MIIvSetReAuto(pAdapter->dwIoBase, pAdapter->byRevId);
		}

		VNSvOutPortB(pAdapter->dwIoBase + MAC_REG_FLOWCR0, (BYTE)pAdapter->cbRD);

	}

	/* Set init rx/tx descriptor address into chip */
	MACvSetCurrRxDescAddr(pAdapter->dwIoBase, pAdapter->amRxDescRing.dwPAddr);
	MACvSetCurrTxDescAddr(pAdapter->dwIoBase, pAdapter->amTxDescRing.dwPAddr);

	/* Set current rx/tx descriptor index to 0 */
	pAdapter->idxRxCurDesc = 0;
	pAdapter->idxTxCurDesc = 0;
	/* added by kevin for checking serviced TD RD status */
	pAdapter->idxRxdirtyDesc = 0;
	pAdapter->idxTxdirtyDesc = 0;
	pAdapter->idxRxPktStartDesc = 0;

	return TRUE;
}

BOOL
ADPbShutdown(
    PSAdapterInfo pAdapter
    )
{
	/* Stop chip */
	if (!MACbSafeStop(pAdapter->dwIoBase, pAdapter->byRevId))
		return FALSE;

	/* Stop ISR */
	MACvIntDisable(pAdapter->dwIoBase, pAdapter->byRevId);

	/* Take off Interrupt Service Routine */
	/* unset_irq_handlers(pAdapter->byIrqLevel); */

	/* Disable Interrupt */
	/* mask_interrupt(pAdapter->byIrqLevel); */

	MacDump(MACDBG_INFO, ("[ADPbShutdown] - Take off ISR on IRQ%d\n", pAdapter->byIrqLevel));

	/* Destroy RDR/TDR */
	/* ALCvFreeRdrMemory(pAdapter); */
	/* ALCvFreeTdrMemory(pAdapter); */

	if (pAdapter->pbyTmpBuff != NULL)
		pAdapter->pbyTmpBuff = NULL;

	return TRUE;
}

UINT
ADPuInitAll(
    void
    )
{
	int     ii;
	UINT    uTotalDev = 0;

	/* modified by kevin:mALLOc supplied by u-boot */
	if (!g_bInit) {
		sg_aAdapter = (PSAdapterInfo)mALLOc(sizeof(SAdapterInfo) * CB_MAX_NET_DEVICE);

		if (sg_aAdapter != NULL) {
			/* modifiey by kevin:memset supplied by u-boot */
			memset(sg_aAdapter, 0, sizeof(SAdapterInfo) * CB_MAX_NET_DEVICE);
		} else {
			return 0;
		}
	}

	for (ii = 0; ii < CB_MAX_NET_DEVICE; ii++) {
		s_vInit(&sg_aAdapter[ii], CB_MAX_NET_DEVICE, ii);

		if ((&sg_aAdapter[ii])->wDevId == W_DEVICE_ID_3106A ||
			(&sg_aAdapter[ii])->wDevId == W_DEVICE_ID_3053A)
			uTotalDev++;
	}

	return uTotalDev;
}

VOID
CARDvSetLoopbackMode(
    DWORD   dwIoBase,
    BYTE    byRevId,
    WORD    wLoopbackMode
    )
{
	switch (wLoopbackMode) {
	case CARD_LB_NONE:
	case CARD_LB_MAC:
	case CARD_LB_MII:
		break;
	default:
		/* DBG_ASSERT(FALSE); */
		break;
	}

	/* set MAC loopback */
	MACvSetLoopbackMode(dwIoBase, LOBYTE(wLoopbackMode));
	/* set MII loopback */
	MIIvSetLoopbackMode(dwIoBase, byRevId, HIBYTE(wLoopbackMode));
}

VOID
CARDvSetMediaLinkMode(
    DWORD   dwIoBase,
    BYTE    byRevId,
    UINT    uConnectionType
    )
{
	DWORD   dwData;

	MIIbReadEmbedded(dwIoBase, byRevId, MII_REG_PHYID2, (PWORD)&dwData);
	MIIbReadEmbedded(dwIoBase, byRevId, MII_REG_PHYID1, (PWORD)&dwData + 1);

	/* If connection type is AUTO */
	if (uConnectionType == MEDIA_AUTO) {
		/* Set duplex mode of MAC according to duplex mode of MII */
		if (MIIbIsInFullDuplexMode(dwIoBase, byRevId))
			MACvSetDuplexMode(dwIoBase, byRevId, TRUE);
		else
			MACvSetDuplexMode(dwIoBase, byRevId, FALSE);

		MIIvRegBitsOn(dwIoBase, byRevId, MII_REG_ANAR, ANAR_TXFD|ANAR_TX|ANAR_10FD|ANAR_10);
		/* not force */
		MIIvRegBitsOff(dwIoBase, byRevId, MII_REG_MODCFG, 1);

		/* enable AUTO-NEGO mode */
		MIIvSetAutoNegotiationOn(dwIoBase, byRevId);
	} else {
		/* If not, then */
		/* 1). Turn off AUTO-NEGO */
		/* 2). Set USER-FORCED speed (10/100) & duplex (half/full) mode of MII and MAC */

		/* For 3065/3106, Nway-force */
		WORD    wANAR;

		MIIvSetAutoNegotiationOff(dwIoBase, byRevId);
		MIIbReadEmbedded(dwIoBase, byRevId, MII_REG_ANAR, &wANAR);

		wANAR &= (~(ANAR_TXFD|ANAR_TX|ANAR_10FD|ANAR_10));

		switch (uConnectionType) {
		case MEDIA_100M_FULL:
			wANAR |= ANAR_TXFD;
			MACvSetDuplexMode(dwIoBase, byRevId, TRUE);
			break;

		case MEDIA_100M_HALF:
			wANAR |= ANAR_TX;
			MACvSetDuplexMode(dwIoBase, byRevId, FALSE);
			break;

		case MEDIA_10M_FULL:
			wANAR |= ANAR_10FD;
			MACvSetDuplexMode(dwIoBase, byRevId, TRUE);
			break;

		case MEDIA_10M_HALF:
			wANAR |= ANAR_10;
			MACvSetDuplexMode(dwIoBase, byRevId, FALSE);
			break;

		default:
			break;
		}

		MIIbWriteEmbedded(dwIoBase, byRevId, MII_REG_ANAR, wANAR);
		/* force */
		MIIvRegBitsOn(dwIoBase, byRevId, MII_REG_MODCFG, 1);

		/* Enable AUTO-NEGO mode */
		MIIvSetAutoNegotiationOn(dwIoBase, byRevId);
	}
}

static void s_vInit(
    PSAdapterInfo pAdapter,
    UINT    uTotalNum,
    UINT    uIdx
    )
{
	int     ii;
	unsigned char check;

	pAdapter->cbTotalAdapterNum = uTotalNum;
	pAdapter->uAdapterIndex = uIdx;

	/* Save Memory Mapped IO base address */
	switch (uIdx) {
	case 0:
		pAdapter->dwIoBase = BA_MAC0;
		break;

	case 1:
		pAdapter->dwIoBase = BA_MAC1;
		break;

	default:
		break;
	}
	MacDump(MACDBG_INFO, ("Memory mapped IO base address:%08X\n", pAdapter->dwIoBase));

	/* check vee oe pee */
	VNSvInPortB(pAdapter->dwIoBase+0x77, &check)
	if (check & 0x40) {
		/* Issue AUTOLD in EECSR to reload eeprom to ensure right data from eeprom */
		MACvRegBitsOn(pAdapter->dwIoBase, MAC_REG_EECSR, EECSR_AUTOLD);
		/* set VEELD */
		VNSvOutPortB(pAdapter->dwIoBase+PCI_Configuration_Space_Offset+VMSTS, 0x1);
		/* wait until VEELD is set */
		ii = 0;
		while (1) {
			VNSvInPortB(pAdapter->dwIoBase+PCI_Configuration_Space_Offset+VMSTS, &check);
			if (check&0x2 || ii == MaxTimeOut)
				break;
			ii++;
		}
		/* clear VEELD */
		VNSvOutPortB(pAdapter->dwIoBase+PCI_Configuration_Space_Offset+VMSTS, 0x0);
	} else {
		/* Issue AUTOLD in EECSR to reload eeprom to ensure right data from eeprom */
		MACvRegBitsOn(pAdapter->dwIoBase, MAC_REG_EECSR, EECSR_AUTOLD);
		/* Wait until EEPROM loading complete */
		while (TRUE) {
			BYTE    byData;
			VNSvInPortB(pAdapter->dwIoBase + MAC_REG_EECSR, &byData);
			if (BITbIsBitOff(byData, EECSR_AUTOLD))
				break;
		}
	}

	/* Get Device ID from PCI configuration space */
	VPCIvReadW(pAdapter->dwIoBase, PCI_REG_DEVICE_ID, &pAdapter->wDevId);
	MacDump(MACDBG_INFO, ("Device ID:%04X\n", pAdapter->wDevId));

	if (pAdapter->wDevId != W_DEVICE_ID_3106A && pAdapter->wDevId != W_DEVICE_ID_3053A)
		return;

	/* Get Revision ID from PCI configuration */
	VPCIvReadB(pAdapter->dwIoBase, PCI_REG_REV_ID, &pAdapter->byRevId);
	MacDump(MACDBG_INFO, ("Revision ID:%02X\n", pAdapter->byRevId));

	/* Clear sticky bits */
	if (g_sOptions.ulInitCmds & INIT_CMD_CLEAR_STICKHW)
		MACvClearStckDS(pAdapter->dwIoBase);

	if (g_sOptions.byRevId != 0)
		pAdapter->byRevId = g_sOptions.byRevId;

	/* Set RD,TD number for this adapter now */
	/* For 3065, 3106J, 3206 */
	pAdapter->cbRD = (g_sOptions.iRDescNum) ? g_sOptions.iRDescNum : CB_INIT_RD_NUM;
	pAdapter->cbTD = (g_sOptions.iTDescNum) ? g_sOptions.iTDescNum : CB_INIT_TD_NUM;

	/* Save IRQ number */
	switch (uIdx) {
	case 0:
		pAdapter->byIrqLevel = IRQ_ETH0;
		/* pAdapter->pvAdapterIsr = ISRvIsrForMAC0; */
		break;

	case 1:
		pAdapter->byIrqLevel = IRQ_ETH1;
		/* pAdapter->pvAdapterIsr = ISRvIsrForMAC1; */
		break;

	default:
		break;
	}

	g_sOptions.uiBuffsize = (g_sOptions.uiBuffsize) ? g_sOptions.uiBuffsize : CB_MAX_BUF_SIZE;

	/* Get the offset of PM Capability and save it from pci configuration  space */
	VPCIvReadB(pAdapter->dwIoBase, PCI_REG_CAP, &pAdapter->byPMRegOffset);

	/* Get PHY address */
	pAdapter->byPhyId = MACbyGetPhyId(pAdapter->dwIoBase);
	MacDump(MACDBG_INFO, ("PHY Address:%02X\n", pAdapter->byPhyId));

	/* Get Ethernet address */
	for (ii = 0; ii < U_ETHER_ADDR_LEN; ii++)
		VNSvInPortB(pAdapter->dwIoBase + MAC_REG_PAR, pAdapter->abyEtherAddr + ii);

	/* Allocate RD/TD poiter array & DescBuf array data structure */
	if (!g_bInit) {
		if (!ADPbDynaAllocBuf(pAdapter)) {
			printf("ADPbDynaAllocBuf() can't allocate buffer.\n");
			return;
		}
	}

	pAdapter->dwCacheLineSize = sizeof(DWORD) * 4;

	/* Set default value of connection type to MEDIA_AUTO */
	pAdapter->uConnectionType = MEDIA_AUTO;

	/* Init default descriptor number per packet in monitor mode */
	pAdapter->uTxDescNumPerPacket = 1;

	/* Background timer send as default */
	pAdapter->bTxContFunTest = TRUE;
}
