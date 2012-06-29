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

#if !defined(__CARD_H__)
#include "card.h"
#endif
#if !defined(__SETUP_H__)
#include "setup.h"
#endif
#if !defined(__ALLOCT_H__)
#include "alloct.h"
#endif

#include <net.h>

/* added by kevin */
BOOL g_bInit = FALSE;
unsigned int sg_i32GError_status;
/*---------------------  Static Definitions -------------------------*/

/*---------------------  Static Classes  ----------------------------*/

/*---------------------  Static Variables  --------------------------*/

/*---------------------  Static Functions  --------------------------*/

/* static BOOL s_bFind_ConfigGmac(); */
/*---------------------  Export Variables  --------------------------*/
PVOID   g_pvGBufferIndex = (PVOID)GMEMORY_BUFFER_START;

SCmdParaSet sCmdPara;
extern SAdapterOpts sOptions;
/*---------------------  Export Functions  --------------------------*/

int
gmac_startio(bd_t *bd)
{
	int     ii;
	UINT    uTotalAdapterNum = 0;
	int     i32TotalAdapterNum = 0;
	UINT    uAdapterInWork = 0;
	PSAdapterInfo apAdapter[MAX_NET_DEVICE];
	UINT    uSTAdapter, uEDAdapter;
	PSAdapterInfo pAdapter;
	static int gmac_init = 1;
	DWORD isr_status;
	int i;
	BYTE Data;

	if (gmac_init == 0) {
		pAdapter = &sg_aGAdapter[0];
		/* 2009/4/20-s add it for check port status change and re-enable MII auto-polling */
		if (GMACbIsRegBitsOn(pAdapter->dwIoBase, MAC_REG_ISR1, 0x80))
		{
			VNSvOutPortB(0xd8004025, 0x80);
			VNSvOutPortB(0xd8004070, 0x00);
			VNSvOutPortB(0xd8004071, 0x80);
			while(1)
			{
				VNSvInPortB(0xd800406d, &Data);
				if(Data & 0x80)
					break;
			}
			VNSvOutPortB(0xd8004070, 0x80);
		}
		/* 2009/4/20-e add it for check port status change and re-enable MII auto-polling */
		if (GMACbIsRegBitsOn(pAdapter->dwIoBase, MAC_REG_ISR1, 0x20)) {
			MACvReadISR(pAdapter->dwIoBase, &isr_status);
			MACvWriteISR(pAdapter->dwIoBase, isr_status);
			if (!GMACbSafeStop(pAdapter->dwIoBase, pAdapter->byRevId))
				return FALSE;
			GALCvChainRdrMemory(pAdapter);
			GALCvChainTdrMemory(pAdapter, 0); /* chain first data segment */
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
			MACvRxOn(pAdapter->dwIoBase);
			MACvRxQueueRUN(pAdapter->dwIoBase);
			MACvRxQueueWake(pAdapter->dwIoBase);
			MACvTxOn(pAdapter->dwIoBase);
			MACvTxQueueRUN(pAdapter->dwIoBase, 0);
		}
		/* 2009/4/20-s update mac address from ethaddr environment variable */
		for(ii = 0; ii < U_ETHER_ADDR_LEN; ii++)
			VNSvOutPortB(pAdapter->dwIoBase+MAC_REG_PAR+ii,bd->bi_enetaddr[ii]);
		/* 2009/4/20-e update mac address from ethaddr environment variable */
		return 1;
	}
	gmac_init = 0;

	/* 1.Init option & parameter */
	/*sOptions.ulInitCmds = INIT_CMD_FORCE_PHYRST;*/
	sOptions.ulInitCmds = 0;
	sOptions.iRDescNum = 64;
	sOptions.iTDescNum = 64;
	sOptions.bNoInit = FALSE;
	/* set initial value of parameter set */
	sCmdPara.uCardNum = 0;

	/* 2.Confure PCI device configuration space */
	/* s_bFind_ConfigGmac(); */

	/* 3.Init adapter(find adapter) */
	uTotalAdapterNum = GADPuInitAll();

	if (uTotalAdapterNum == 0) {
		printf("Cannot find the adapter or initialization is failed.\n");
		freemem();
		return -1;
	}

	if (uTotalAdapterNum > MAX_NET_DEVICE) {
		printf("Cannot support over %d adapters.\n", MAX_NET_DEVICE);
		freemem();
		return -1;
	}

	/* determine if it's a multi-adapter initialization */
	if (sCmdPara.uCardNum != 0) {
		uSTAdapter = (sCmdPara.uCardNum-1);
		uEDAdapter = uSTAdapter + 1;
	} else {
		uSTAdapter = 0;
		uEDAdapter = uTotalAdapterNum;
	}

	for (ii = uSTAdapter; ii < uEDAdapter; ii++) {
		/* WORD  wEECheckSum; */
		if (GMACbIsRegBitsOn(sg_aGAdapter[ii].dwIoBase, MAC_REG_CR0_SET, CR0_STRT)) {
			/* printf("Adapter %d is already running!\n", ii); */
			/* printf("\n"); */
		}

		/* Check if EEP-less strapping is wrong */
		if (sg_aGAdapter[ii].byRevId >= 0x10) {
			BOOL    bEEPless;

			/* check the EEPROM path strapping, 0 => serial EEPROM */
			/* 1 => EEPROM from shadow */

			if (sg_aGAdapter[ii].byRevId < REV_ID_VT3286_A1) {
				bEEPless = GMACbIsRegBitsOn(
					sg_aGAdapter[ii].dwIoBase,
					MAC_REG_JMPSR1,
					JMPSR1_J_VEESEL
				);
			} else {
				bEEPless = GMACbIsRegBitsOn(
					sg_aGAdapter[ii].dwIoBase,
					MAC_REG_JMPSR1,
					JMPSR1_J_VEESEL
				);
			}

			if ((!bEEPless)) {
				/* it means either customer do the wrong strapping (serial EEP to EEP-less)
				   or BIOS doesn't copy EEPROM content form persistent storage to shadow */
				/* printf("Adapter %d EEP-less strapping check failed!\n", ii); */
				printf("EEP-less strapping = TRUE\n");
				/* freemem(); */
				/* return ; */
			} else {
				if (GMACbIsRegBitsOff(sg_aGAdapter[ii].dwIoBase, MAC_REG_CHIPGSR, 0x10)) {
					printf("Adapter %d EEPROM signature is not correct!\n", ii);
					printf("\n");
					freemem();
					return -1;
				}
			}
		} /* if */
	} /* for */

	/* 4.init MAC, MII, MIB, IRQ */
	i32TotalAdapterNum = uTotalAdapterNum;
	uTotalAdapterNum = 0;

	for (ii = 0; ii < i32TotalAdapterNum; ii++) {
		/* Start driver(create TD/RD and ISR) */
		if (!GADPbBind(&sg_aGAdapter[ii])) {
			printf("Initialize adapter fail.\n");
			freemem();
			return -1;
		}
		apAdapter[uAdapterInWork] = &sg_aGAdapter[ii];
		uAdapterInWork++;
		uTotalAdapterNum++;
	}

	if (uTotalAdapterNum == 0) {
		printf("Cannot find the adapter or initialization is failed.\n");
		freemem();
		return -1;
	}

	/* Wait 2s for connection is stable... */
	/* GMACvTimer0MiniSDelay(pAdapter->dwIoBase, pAdapter->byRevId, 3000); */
	/* set parameter */
	pAdapter = &sg_aGAdapter[0];
	/* 2009/4/20-s update mac address from ethaddr environment variable */
	for(ii = 0; ii < U_ETHER_ADDR_LEN; ii++)
		VNSvOutPortB(pAdapter->dwIoBase+MAC_REG_PAR+ii,bd->bi_enetaddr[ii]);
	/* 2009/4/20-e update mac address from ethaddr environment variable */
	pAdapter->bTxOn = FALSE;
	pAdapter->bRandomPktLength = FALSE;
	pAdapter->bRandomPktData = FALSE;
	pAdapter->bWhenRxCrcErrNoBuf = FALSE;
	pAdapter->bWhenRxDataErrNoBuf = FALSE;
	pAdapter->bIncPktLength = FALSE;
	pAdapter->bSoftwareGenCrc = FALSE;
	pAdapter->bTxSinglePacket = FALSE;
	pAdapter->bTxContFunTest = FALSE;
	/* clear statisic */
	GSTAvClearAllCounter(&pAdapter->scStatistic);
	/* set init rx/tx descriptor address into chip, it will return hw rd/td index to 0 */
	MACvSetRqIndex(pAdapter->dwIoBase, 0);
	GMACvSetTqIndex(pAdapter->dwIoBase, 0, 0);

	/* start ISR, ISR should has enabled, but we want to make sure */
	MACvIntEnable(pAdapter->dwIoBase, IMR_MASK_VALUE);
	MACvStart(pAdapter->dwIoBase);
	/* start rx */
	MACvRxOn(pAdapter->dwIoBase);
	MACvRxQueueRUN(pAdapter->dwIoBase);
	MACvRxQueueWake(pAdapter->dwIoBase);
	/* start tx */
	MACvTxOn(pAdapter->dwIoBase);
	MACvTxQueueRUN(pAdapter->dwIoBase, 0);

	return 1;
}

/*
static BOOL s_bFind_ConfigGmac()
{
	SPciDevice  pciDevice3119;

	// 1.allocate memory space
	NPCIbPCIAlloc(NULL,W_VENDOR_ID,W_DEVICE_ID_3119A,&pciDevice3119);
	return TRUE;
}
*/

int gmac_send(volatile void *packet, int length)
{
	PBYTE     pbyTxDescBuffer;
	UINT      uCurrTdIdx;
	PSTxDesc  pTD;
	PSAdapterInfo pAdapter = &sg_aGAdapter[0];


	uCurrTdIdx = pAdapter->uTxEnqueDescIdx[0];
	pTD = pAdapter->aapTD[0][uCurrTdIdx];

	/* if tx queue is full */
	if (pTD->m_td0TD0.f1Owner == B_OWNED_BY_CHIP)
		return FALSE;

	/* copy buffer */
	pbyTxDescBuffer = (PBYTE)(pAdapter->aapTD[0][uCurrTdIdx]->m_asTxBufs[0].dwTxBufAddrLo);
	MEMvSet((PVOID)pbyTxDescBuffer, 0, 60);
	MEMvCopy((PVOID)pbyTxDescBuffer, (PVOID)packet, length);

	if (length < MIN_PACKET_LEN)
		length = MIN_PACKET_LEN;

	/* Set TD PktLen */
	pTD->m_td0TD0.f14TxPktSize = (WORD)length;

	/* set up data segment */
	pTD->m_td1TD1.f4CMDZ = 2;
	pTD->m_asTxBufs[0].f14TxBufSize = (WORD)length;

	/*
	* Set Qwner Bit and Que Bit
	*/
	/* Get Last TdIdx */
	UINT    uLastTdIdx = (uCurrTdIdx + pAdapter->cbTD - 1) % pAdapter->cbTD;

	/* Set last Td's que bit */
	if (pAdapter->aapTD[0][uLastTdIdx]->m_td0TD0.f1Owner == B_OWNED_BY_CHIP) {
		/* VT3119A0 */
		/* vSetBit((PDWORD)(pAdapter->aapTD[byTxQue][uLastTdIdx])); */
		/* VT3119A1 */
		pAdapter->aapTD[0][uLastTdIdx]->m_asTxBufs[0].f1Que = B_TD_LIST_NOT_END;
	}

	/* set tx descriptors owner to chip */
	pTD->m_td0TD0.f1Owner = B_OWNED_BY_CHIP;

	/* post one TD */
	pAdapter->auTxPktPosted[0]++;

	/* Move EnqueTdIdx to next */
	pAdapter->uTxEnqueDescIdx[0] = (WORD)((uCurrTdIdx + 1) % pAdapter->cbTD);

	/* TX */
	MACvTxOn(pAdapter->dwIoBase);
	MACvTxQueueRUN(pAdapter->dwIoBase, 0);
	MACvTxQueueWake(pAdapter->dwIoBase, 0);

	return 0;
}

int gmac_receive(void)
{
	PSAdapterInfo pAdapter = &sg_aGAdapter[0];
	int length = 0;
	DWORD isr_status;
	UINT        uCurrDescIdx;
	PSRxDesc    pCurrRD;
	BYTE        byRsr0, byRsr1;

	/* 1.store ISR */
	MACvReadISR(pAdapter->dwIoBase, &isr_status);

	/* 2.disable IMR */
	MACvIntDisable(pAdapter->dwIoBase);
	/* 3.clear ISR */
	MACvWriteISR(pAdapter->dwIoBase, isr_status);
	/* 4.handle received packets until owner==chip */
	while (TRUE) {
		uCurrDescIdx = pAdapter->uRxDequeDescIdx;
		pCurrRD = pAdapter->apRD[uCurrDescIdx];
		byRsr0 = pCurrRD->m_rd0RD0.byRSR0;
		byRsr1 = pCurrRD->m_rd0RD0.byRSR1;

		/* if the RD is owned by the chip, we are done. */
		if (pCurrRD->m_rd0RD0.f1Owner == B_OWNED_BY_CHIP)
			break;

		if ((pCurrRD->m_rd0RD0.byRSR1 & RSR1_RXOK)) {
			NetReceive((volatile uchar *)pCurrRD->m_dwRxBufAddrLo, pCurrRD->m_rd0RD0.f14RMBC-4);
			length += (pCurrRD->m_rd0RD0.f14RMBC-4);
		} else {
			printf("receive error status:%02X %02X%\n",
				pCurrRD->m_rd0RD0.byRSR1, pCurrRD->m_rd0RD0.byRSR0);
			/* handler will do later */
		}
		/* reset RD own bit */
		pCurrRD->m_rd0RD0.f1Owner = B_OWNED_BY_CHIP;

		/* increase to next RD */
		pAdapter->uRxDequeDescIdx = (++pAdapter->uRxDequeDescIdx)%pAdapter->cbRD;
	}

	/* 5.enable IMR */
	MACvIntEnable(pAdapter->dwIoBase, IMR_MASK_VALUE);

	return length;
}

void gmac_halt(void)
{
	if (g_bInit)
		GADPbShutdown(&sg_aGAdapter[0]);
	return;
}

void mii_read(char *devname, unsigned char addr,
		unsigned char reg, unsigned short *value)
{
	PSAdapterInfo pAdapter;
	pAdapter = &sg_aGAdapter[0];
	GMIIbReadEmbeded(pAdapter->dwIoBase, pAdapter->byRevId, addr, value);
#if MACDBG
	printf("%02X %02X %04X\n", addr, reg, value);
#endif
}
void mii_write(char *devname, unsigned char addr,
		unsigned char reg, unsigned short value)
{
	PSAdapterInfo pAdapter;
	pAdapter = &sg_aGAdapter[0];
	GMIIbWriteEmbeded(pAdapter->dwIoBase, pAdapter->byRevId, addr, value);
#if MACDBG
	printf("%02X %02X %04X\n", addr, reg, value);
#endif
}
