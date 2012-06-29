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

#if !defined(__UPCI_H__)
#include "upci.h"
#endif
#if !defined(__CARD_H__)
#include "card.h"
#endif
#if !defined(__GISR_H__)
#include "gisr.h"
#endif
#if !defined(__VPCI_H__)
#include "vpci.h"
#endif
#if !defined(__TCONVERT_H__)
#include "tconvert.h"
#endif
#if !defined(__ALLOCT_H__)
#include "alloct.h"
#endif

#include "macif.h"

/*---------------------  Static Definitions -------------------------*/

/*---------------------  Static Classes  ----------------------------*/

/*---------------------  Static Variables  --------------------------*/

/*---------------------  Static Functions  --------------------------*/
static void s_vInit(
    PSAdapterInfo   pAdapter,
    UINT            uTotalNum,
    UINT            uIdx,
    PISR            pisrNetIsr,
    WORD            wBusDevFunID
    );
/*---------------------  Export Variables  --------------------------*/
PSAdapterInfo   sg_aGAdapter;
SAdapterOpts    sOptions = {0, 0, 0, 0, 0, 0};
PBYTE           sg_abyRandomTable;

/*---------------------  Export Functions  --------------------------*/
/* Init all adapter's AdapterInfo data structure */
UINT GADPuInitAll(void)
{
	PISR    apisrNetProc[MAX_NET_DEVICE] = {
						(PISR)ISRvIsrForNetwork_Card0,
						(PISR)ISRvIsrForNetwork_Card1,
						(PISR)ISRvIsrForNetwork_Card2,
						(PISR)ISRvIsrForNetwork_Card3
					};

	SPciDevice  pciDevice3119;
	SPciDevice  pciDevice3286;

	int         ii;
	U16         u16IdxPciDev;
	U16         u16TotalPciDev;

	/* read pci info */
	/*
	NPCIbReadDeviceInfo(NULL, W_VENDOR_ID, W_DEVICE_ID_3119A, &pciDevice3119);
	NPCIbReadDeviceInfo(NULL, W_VENDOR_ID, 0x3286, &pciDevice3286);
	*/
	memset(&pciDevice3119, 0, sizeof(SPciDevice));
	memset(&pciDevice3286, 0, sizeof(SPciDevice));
	pciDevice3119.wDeviceNum = CB_MAX_NET_DEVICE;
	pciDevice3286.wDeviceNum = 0;

	u16TotalPciDev = (U16)(pciDevice3119.wDeviceNum + pciDevice3286.wDeviceNum);

	if (u16TotalPciDev == 0)
		return 0;

	sg_aGAdapter = (PSAdapterInfo)malloc(sizeof(SAdapterInfo)*u16TotalPciDev);
	if (sg_aGAdapter == NULL) {
		printf("Memory allocation error %s, %d\n", __FILE__, __LINE__);
		freemem();
		return 0;
	}
	memset(sg_aGAdapter, 0, sizeof(SAdapterInfo)*u16TotalPciDev);

	/* set adapter info */
	u16IdxPciDev = 0;
	for (ii = 0; ii < pciDevice3119.wDeviceNum; ii++, u16IdxPciDev++) {
		s_vInit(
			&sg_aGAdapter[u16IdxPciDev],
			u16TotalPciDev,
			u16IdxPciDev,
			apisrNetProc[u16IdxPciDev],
			pciDevice3119.awBusDevFunID[ii]
		);
	}

	for (ii = 0; ii < pciDevice3286.wDeviceNum; ii++, u16IdxPciDev++) {
		s_vInit(
			&sg_aGAdapter[u16IdxPciDev],
			u16TotalPciDev,
			u16IdxPciDev,
			apisrNetProc[u16IdxPciDev],
			pciDevice3286.awBusDevFunID[ii]
		);
	}

	return u16TotalPciDev;
}

/*
 * Description:
 *
 * Parameters:
 *  In:
 *      bySlotNum   - specify which slot number to init,
 *                      0xFF means the first one adapter to init.
 *  Out:
 *      pAdapter    - pointer of adapter which initialized,
 *                      return NULL if init failed.
 *
 * Return Value: TRUE if succeeded; FALSE if failed.
 *
 */
BOOL GADPbInitOne(U8 bySlotNum, PSAdapterInfo *ppAdapter)
{
	PISR    apisrNetProc[MAX_NET_DEVICE] = {
						(PISR)ISRvIsrForNetwork_Card0,
						(PISR)ISRvIsrForNetwork_Card1
					};

	SPciDevice  pciDevice3119;
	/* SPciDevice  pciDevice3286; */

	int         ii;
	U16         u16IdxPciDev;
	U16         u16TotalPciDev;

	*ppAdapter = NULL;

	/* read pci info */
	NPCIbReadDeviceInfo(NULL, W_VENDOR_ID, W_DEVICE_ID_3119A, &pciDevice3119);
	/* NPCIbReadDeviceInfo(NULL, W_VENDOR_ID, 0x3286, &pciDevice3286); */

	u16TotalPciDev = (U16)(pciDevice3119.wDeviceNum);

	/* if no adapter */
	if (u16TotalPciDev == 0)
		return FALSE;

	sg_aGAdapter = (PSAdapterInfo) malloc(sizeof(SAdapterInfo));
	if (sg_aGAdapter == NULL) {
		printf("Memory allocation error %s, %d\n", __FILE__, __LINE__);
		return FALSE;
		/* exit(0); */
	}

	memset(sg_aGAdapter, 0, sizeof(SAdapterInfo));

	/* init first one adapter */
	if (bySlotNum == 0xFF) {
		s_vInit(&sg_aGAdapter[0], u16TotalPciDev, 0,
		apisrNetProc[0], pciDevice3119.awBusDevFunID[0]);
		*ppAdapter = &sg_aGAdapter[0];
		return TRUE;
	} else {
		/* init adapter by slot number specified */
		u16IdxPciDev = 0;
		for (ii = 0; ii < pciDevice3119.wDeviceNum; ii++, u16IdxPciDev++) {
			if (bySlotNum == GET_DEVID(pciDevice3119.awBusDevFunID[ii])) {
				s_vInit(
					&sg_aGAdapter[u16IdxPciDev],
					u16TotalPciDev,
					u16IdxPciDev,
					apisrNetProc[u16IdxPciDev],
					pciDevice3119.awBusDevFunID[ii]
				);
				*ppAdapter = &sg_aGAdapter[u16IdxPciDev];
				return TRUE;
			}
		}
	}

	return FALSE;
}

static BOOL s_bDynaAllocBuf(PSAdapterInfo pAdapter)
{
	int i;

	/* Allocate RD pointer array */
	pAdapter->apRD = (PSRxDesc *)malloc(sizeof(PSRxDesc)*pAdapter->cbRD);

	if (pAdapter->apRD == NULL)
		return FALSE;

	/* Allocate TD pointer array & DescBuf SAllocMap array for every TD ring. */
	/* we allocate 7 SAllocMap for 7 data buffer pool. */
	for (i = 0; i < CB_TD_RING_NUM; i++) {
		pAdapter->aapTD[i] = (PSTxDesc *)malloc(sizeof(PSTxDesc)*pAdapter->cbTD);
		pAdapter->aaamTxDescBufPool[i] = (PSAllocMap)malloc(sizeof(SAllocMap)*CB_MAX_SEG_PER_PKT);

		if ((pAdapter->aapTD[i] == NULL) || (pAdapter->aaamTxDescBufPool[i] == NULL))
			return FALSE;

		memset(pAdapter->aaamTxDescBufPool[i], 0, sizeof(SAllocMap)*CB_MAX_SEG_PER_PKT);
	}

	return TRUE;
}

/* Init pAdapter data structure */
static void s_vInit(
	PSAdapterInfo   pAdapter,
	UINT            uTotalNum,
	UINT            uIdx,
	PISR            pisrNetIsr,
	WORD            wBusDevFunID
)
{
	/* allocate new buffer dynamically. */
	pAdapter->cbTotalAdapterNum = uTotalNum;
	pAdapter->uAdapterIndex = uIdx;
	pAdapter->pisrAdapterIsr = pisrNetIsr;

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

	/* Get Device ID */
	VPCIvReadW(pAdapter->dwIoBase, PCI_REG_DEVICE_ID, &pAdapter->wDeviceId);
	/* MacDump( MACDBG_INFO, ("Device ID:%04X\n", pAdapter->wDevId)); */

	/* Get Revision ID */
	VPCIvReadB(pAdapter->dwIoBase, PCI_REG_REV_ID, &pAdapter->byRevId);
	/* MacDump( MACDBG_INFO, ("Revision ID:%02X\n", pAdapter->byRevId)); */

	/* Save IRQ number */
	switch (uIdx) {
	case 0:
		pAdapter->byIrqLevel = IRQ_ETH0;
		break;

	case 1:
		pAdapter->byIrqLevel = IRQ_ETH0;
		break;

	default:
		break;
	}

	if (sOptions.byRevId != 0)
		pAdapter->byRevId = sOptions.byRevId;

	pAdapter->cbRD = (sOptions.iRDescNum) ? sOptions.iRDescNum : CB_INIT_RD_NUM;
	pAdapter->cbTD = (sOptions.iTDescNum) ? sOptions.iTDescNum : CB_INIT_TD_NUM;

	VPCIvReadD(pAdapter->dwIoBase, PCI_REG_EXP_ROM_BAR, &pAdapter->dwBootRomBase);

	/* PCIvReadConfigB(NULL, pAdapter->wBusDevFunID, PCI_REG_INT_LINE, &pAdapter->byIrqLevel); */
	/* PCIvReadConfigD(NULL, pAdapter->wBusDevFunID, PCI_REG_EXP_ROM_BAR, &pAdapter->dwBootRomBase); */

	pAdapter->byIntNum = (BYTE)((pAdapter->byIrqLevel <= 7) ?
		(pAdapter->byIrqLevel + 0x08) : (pAdapter->byIrqLevel + 0x68));

	/* Get the offset of PM Capability and save it */
	VPCIvReadB(pAdapter->dwIoBase, PCI_REG_CAP, &pAdapter->byPMRegOffset);

	/* Get Boot ROM Size */
	pAdapter->byBootRomSize = GMACbyGetBootRomSize(pAdapter->dwIoBase);

	/* Get PHY ID */
	pAdapter->byPhyId = GMACbyGetPhyId(pAdapter->dwIoBase);

	/* get Ethernet address and the string */
	MACvReadEtherAddress(pAdapter->dwIoBase, pAdapter->abyEtherAddr);
	CVTvBy6ToStr12(pAdapter->abyEtherAddr, pAdapter->aszEtherAddrStr);

	/*
	* INIT VALUE
	*/
	/* allocate RD/TD poiter array & DescBuf array data structure */
	if (!s_bDynaAllocBuf(pAdapter)) {
		printf("s_bDynaAllocBuf() can'nt allocate buffer");
		freemem();
		return ;
	}
	/* for TD/RD alignment */
	pAdapter->dwCacheLineSize = sizeof(DWORD) * 16; /* 64 bytes */

	/* init pAdapter->bSharedIrqSupport */
	pAdapter->bSharedIrqSupport = TRUE;

	/* set connection type to MEDIA_AUTO by default */
	pAdapter->uConnectionType = MEDIA_AUTO;

	/* init default data segment number per packet in monitor mode */
	pAdapter->uTxDataSegsPerPacket = CB_MIN_SEG_PER_PKT;

	/* init default Tx packet size */
	pAdapter->uTxPktSize = MAX_PACKET_LEN;/* MIN_PACKET_LEN + 4; */
	pAdapter->cbPktSize = MAX_PACKET_LEN;
	/* init MIB2 counter */
	strcpy(pAdapter->mibCounter.ifDescr, " Gigabit Ether");

	pAdapter->mibCounter.ifType = ETHERNETCSMACD;
	pAdapter->mibCounter.ifMtu = MAX_DATA_LEN;
	MEMvCopy((PVOID)pAdapter->mibCounter.ifPhysAddress, (PVOID)pAdapter->abyEtherAddr, U_ETHER_ADDR_LEN);
	pAdapter->mibCounter.ifAdminStatus = UP;

	/* init RMON counter */
	pAdapter->rmonCounter.etherStatsStatus = VALID;

	/* background timer send as default */
	pAdapter->bTxContFunTest = TRUE;

	/* init packet driver */
	pAdapter->wRcvTypeLen = 0;
	pAdapter->pisrReceiver = NULL;
}

BOOL GADPbInitialize(PSAdapterInfo pAdapter)
{
	int i;
	/* write-clear PME status bit */
	BYTE byOrgData, byPMRegOffset;
	/* fill default value */
	GALCvSetDefaultRD(pAdapter);
	GALCvSetDefaultTD(pAdapter, 0); /* initial first data segment */
	/* init PCI */
	/* NPCIvInitialize(NULL, pAdapter->wBusDevFunID, pAdapter->byRevId); */
	VPCIvInitialize(pAdapter->dwIoBase, pAdapter->byRevId);

	VPCIvReadB(pAdapter->dwIoBase, PCI_REG_CAP, &byPMRegOffset);
	VPCIvReadB(pAdapter->dwIoBase, (BYTE)(byPMRegOffset + PM_PMCSR1), &byOrgData);
	VPCIvWriteB(pAdapter->dwIoBase, (BYTE)(byPMRegOffset + PM_PMCSR1),
		(BYTE)(PMCSR1_PME_STATUS | byOrgData));
	/*
	PCIvReadConfigB(NULL, pAdapter->wBusDevFunID, PCI_REG_CAP, &byPMRegOffset);
	PCIvReadConfigB(NULL, pAdapter->wBusDevFunID, (BYTE)(byPMRegOffset + PM_PMCSR1), &byOrgData);
	PCIvWriteConfigB(NULL, pAdapter->wBusDevFunID, (BYTE)(byPMRegOffset + PM_PMCSR1),
		(BYTE)(PMCSR1_PME_STATUS | byOrgData));
	*/
	if (!sOptions.bNoInit) {

		/* init MAC */
		GMACvInitialize(pAdapter, pAdapter->dwIoBase, pAdapter->byRevId);
		/* set address filter to accept any packet */
		/* 2009/4/20 remove it for tftp with checksum bad message */
		/* GMACvSetPacketFilter(pAdapter->dwIoBase, PKT_TYPE_PROMISCUOUS); */

		/* Eric */
		/* Turn On RCR_SEP to receive CRC error packet */
		MACvRegBitsOn(pAdapter->dwIoBase, MAC_REG_RCR, RCR_SEP);

		/* detect PHY is from which company */
		/* MIIvReadPhyCmrId(pAdapter->dwIoBase, pAdapter->byRevId, &pAdapter->dwPhyCmrId); */

		if (sOptions.ulInitCmds & INIT_CMD_FORCE_PHYRST) {
			/* if it's cicada PHY, force PHY reset to avoid 2-card test link fail */

			if ((pAdapter->dwPhyCmrId & CID_REV_ID_MASK_OFF) == CID_CICADA_CIS8201 ||
			(pAdapter->dwPhyCmrId & CID_REV_ID_MASK_OFF) == CID_CICADA_CIS3216I ||
			(pAdapter->dwPhyCmrId & CID_REV_ID_MASK_OFF) == CID_CICADA_CIS3216I64) {
				/* force PHY reset on */
				MACvRegBitsOn(pAdapter->dwIoBase, MAC_REG_CR3_SET, CR3_FPHYRST);
				/* delay 300ms */
				GMACvTimer0MiniSDelay(pAdapter->dwIoBase, pAdapter->byRevId, 300);
				/* force PHY reset off */
				MACvRegBitsOn(pAdapter->dwIoBase, MAC_REG_CR3_CLR, CR3_FPHYRST);
				/* delay 50ms */
				GMACvTimer0MiniSDelay(pAdapter->dwIoBase, pAdapter->byRevId, 50);
			} else if ((pAdapter->dwPhyCmrId & CID_REV_ID_MASK_OFF) == CID_ICPLAUS_IP1001) {
				/* force PHY reset on */
				MACvRegBitsOn(pAdapter->dwIoBase, MAC_REG_CR3_SET, CR3_FPHYRST);
				/* delay 300ms */
				GMACvTimer0MiniSDelay(pAdapter->dwIoBase, pAdapter->byRevId, 300);
				/* force PHY reset off */
				MACvRegBitsOn(pAdapter->dwIoBase, MAC_REG_CR3_CLR, CR3_FPHYRST);
				/* delay 50ms */
				GMACvTimer0MiniSDelay(pAdapter->dwIoBase, pAdapter->byRevId, 50);
			} else {
			/* force PHY reset on */
			MACvRegBitsOn(pAdapter->dwIoBase, MAC_REG_CR3_SET, CR3_FPHYRST);
			/* delay 300ms */
			GMACvTimer0MiniSDelay(pAdapter->dwIoBase, pAdapter->byRevId, 300);
			/* force PHY reset off */
			MACvRegBitsOn(pAdapter->dwIoBase, MAC_REG_CR3_CLR, CR3_FPHYRST);
			/* delay 50ms */
			GMACvTimer0MiniSDelay(pAdapter->dwIoBase, pAdapter->byRevId, 50);
			}
		}

		/* init MII */
		GMIIvInitialize(pAdapter->dwIoBase, pAdapter->byRevId, pAdapter->dwPhyCmrId);

		/* set media mode */
		GCARDvSetLoopbackMode(pAdapter->dwIoBase, pAdapter->byRevId, CARD_LB_NONE);
		GCARDvSetMediaLinkMode(pAdapter->dwIoBase, pAdapter->byRevId, pAdapter->uConnectionType);

		/* set up flow-control, MIB counter, and tagging initials */
		VNSvOutPortB(pAdapter->dwIoBase + MAC_REG_RBRDU, (BYTE)pAdapter->cbRD);

		/* set MIB counter near full condition to 0x00C00000 */
		MACvRegBitsOn(pAdapter->dwIoBase, MAC_REG_MIBCR, MIBCR_MIBHI);

		GSTAvClearHWMIBCounter(pAdapter->dwIoBase);
		GSTAvEnableHWMIBCounter(pAdapter->dwIoBase);

		MACvRegBitsOff(pAdapter->dwIoBase, MAC_REG_MCFG0, MCFG0_PQEN);
	}

	/* set init rx/tx descriptor address into chip */
	MACvSetDescBaseHi32(pAdapter->dwIoBase, 0UL);
	MACvSetDbfBaseHi16(pAdapter->dwIoBase, 0U);
	MACvSetRxDescBaseLo32(pAdapter->dwIoBase, pAdapter->amRxDescRing.dwPAddr);
	MACvSetRqIndex(pAdapter->dwIoBase, 0);
	MACvSetRqSize(pAdapter->dwIoBase, (WORD)(pAdapter->cbRD - 1));

	for (i = 0; i < CB_TD_RING_NUM; i++) {
		MACvSetTxDescBaseLo32(pAdapter->dwIoBase, pAdapter->aamTxDescRing[i].dwPAddr, i);
		GMACvSetTqIndex(pAdapter->dwIoBase, (BYTE)i, 0);
	}

	MACvSetTqSize(pAdapter->dwIoBase, (WORD)(pAdapter->cbTD - 1));

	/* set current rx/tx descriptor index to 0 */
	pAdapter->uRxDequeDescIdx = 0;
	for (i = 0; i < CB_TD_RING_NUM; i++) {
		pAdapter->uTxEnqueDescIdx[i] = 0;
		pAdapter->uTxDequeDescIdx[i] = 0;
	}
	pAdapter->idxRxPktStartDesc = 0;
	/*
	// Set Random VCAM

	WORD    wInitVID;
	WORD    wGetVID;
	int     ii;
	for (i = 0; i < VCAMSIZE; i++) {
		while (TRUE) {
			wInitVID = (WORD)(rand() & 0x0FFF);

			if (i > 0) {
				for (ii = 0; ii < i; ii++) {
					GMACvGetVCAM(pAdapter->dwIoBase, (BYTE)ii, &wGetVID);
					if (wInitVID == wGetVID)
						break;
				}
				if (i == ii)
					break;
			} else
				break;
		} // end while

		GMACvSetVCAM(pAdapter->dwIoBase, (BYTE)i, wInitVID);
	}
	*/
	/* Set Random MCAM */
	/*
	BYTE    abyInitMCAM[6];
	BYTE    abyGetMCAM[6];
	UINT    uMCamIdx;
	for (i = 0; i < MCAMSIZE; i++) {
		while (TRUE) {
			// Gen One MCAM
			for (uMCamIdx = 0; uMCamIdx < 6; uMCamIdx++)
				abyInitMCAM[uMCamIdx] = (BYTE)rand();

			if (i > 0) {
				for (ii = 0; ii < i; ii++) {
					GMACvGetMCAM(pAdapter->dwIoBase, (BYTE)ii, abyGetMCAM);
					// If they are the same??
					if ( (*(PDWORD)(abyInitMCAM) == *(PDWORD)(abyGetMCAM)) &&
					(*(PWORD)(abyInitMCAM + 4) == *(PWORD)(abyGetMCAM + 4)) )
						break;
				}
				if (i == ii)
					break;
			} else
				break;
		} // end while

		GMACvSetMCAM(pAdapter->dwIoBase, (BYTE)i, abyInitMCAM);
	}
	*/
	return TRUE;
}

BOOL GADPbBind(PSAdapterInfo pAdapter)
{
	/* build RDR/TDR */
	if (!GALCbAllocateRdrMemory(pAdapter))
		return FALSE;

	if (!GALCbAllocateTdrMemory(pAdapter))
		return FALSE;

	GALCvChainRdrMemory(pAdapter);
	GALCvChainTdrMemory(pAdapter, 0); /* chain first data segment */

	if (!GADPbInitialize(pAdapter))
		return FALSE;

	/* Hook Interrupt Service Routine on IRQ */
	/* set_irq_handlers(pAdapter->byIrqLevel, pAdapter->pisrAdapterIsr); */

	/* enable Interrupt Mask in PIC */
	/* unmask_interrupt(pAdapter->byIrqLevel); */
	/* must clear it */
	MACvClearISR(pAdapter->dwIoBase, pAdapter->byRevId);
	/* start ISR */
	MACvIntEnable(pAdapter->dwIoBase, IMR_MASK_VALUE);

	/* wait MAUTO to poll twice, then MIISR_LNKFL will */
	/* be correct status */
	PCAvDelayByIO(CB_DELAY_MII_STABLE * 2);

	/* update link status */
	GMACvTimer0MiniSDelay(pAdapter->dwIoBase, pAdapter->byRevId, 2000);
	pAdapter->bLinkPass = GMACbIsCableLinkOk(pAdapter->dwIoBase);

	if (pAdapter->bLinkPass) {
		/* update duplex status */
		/* NOTE.... here we don't call MIIbIsInFullDuplexMode(), because */
		/* we won't turn on/off MAUTO */
		pAdapter->bFullDuplex = GMACbIsInFullDuplexMode(pAdapter->dwIoBase);
		/* update speed status */
		pAdapter->bSpeed1G = GMACbIsIn1GMode(pAdapter->dwIoBase, pAdapter->byRevId);
		pAdapter->bSpeed100M = GMACbIsIn100MMode(pAdapter->dwIoBase, pAdapter->byRevId);

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

		/* This is patch for VT3119A1 */
		/* Use TCR_TB2BDIS to prevent 1c_Reset failure in helf-duplex mode */
		/* Only use this in 10HD and 100HD */
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
			if ((!pAdapter->bSpeed1G) && (!pAdapter->bSpeed100M) && (!pAdapter->bFullDuplex)) {
				/* Enable count TXSQEErr in 10HD */
				MACvRegBitsOff(pAdapter->dwIoBase, MAC_REG_TESTCFG, 0x80);
			} else {
				/* Disable count TXSQEErr in 10FD, 100HD, 100FD, 1G */
				MACvRegBitsOn(pAdapter->dwIoBase, MAC_REG_TESTCFG, 0x80);
			}
		}

	}

	/*pAdapter->pbyTmpBuff = (PBYTE)malloc(CB_MAX_BUF_SIZE);*/

#if defined(_RUN_SCRIPT)
	pAdapter->mempool = malloc(U_STACKSIZE);
#endif

	return TRUE;
}

BOOL GADPbShutdown(PSAdapterInfo pAdapter)
{
	/* stop chip */
	if (!GMACbSafeStop(pAdapter->dwIoBase, pAdapter->byRevId))
		return FALSE;

	/* stop MAC INT */
	MACvIntDisable(pAdapter->dwIoBase);

	/* stop ISR */
	/* mask_interrupt(pAdapter->byIrqLevel); */

	/* disconnect irq and restore org ISR */
	/* unset_irq_handlers(pAdapter->byIrqLevel); */

	/* destroy RDR/TDR */
	GALCvFreeRdrMemory(pAdapter);
	GALCvFreeTdrMemory(pAdapter);

	if (pAdapter->pbyTmpBuff != NULL) {
		MEMvFree(pAdapter->pbyTmpBuff, CB_MAX_BUF_SIZE);
		pAdapter->pbyTmpBuff = NULL;
	}

	return TRUE;
}
