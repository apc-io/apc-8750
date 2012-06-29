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


/*#ifndef _GLOBAL_H_
#include "global.h"
#endif
#ifndef _EXTVARS_H_
#include "extvars.h"
#endif*/

#if !defined(__CARD_H__)
#include "card.h"
#endif
#if !defined(__ADAPTER_H__)
#include "adapter.h"
#endif
#if !defined(__DIAG_H__)
/* #include "diag.h" */
#endif
#include "macif.h"
#if !defined(__DBG_H__)
#include "dbg.h"
#endif

/* #include "emacisr.h" */
#include <net.h>
#if MACDBG
ULONG MacDebugLevel = MACDBG_INFO | MACDBG_ISR;
#endif

/*---------------------  Static Definitions -------------------------*/

/*---------------------  Static Classes  ----------------------------*/

/*---------------------  Static Variables  --------------------------*/

/*---------------------  Static Functions  --------------------------*/
/* static BOOL s_bDiagnoseTest(PSAdapterInfo pAdapter, int idxItem, unsigned long *pcbPass, */
/* unsigned long *pcbFail, WORD wTestCond, BOOL bShowMsg); */


/*---------------------  Export Variables  --------------------------*/
/* PVOID   g_pvBufferIndex = (PVOID)MEMORY_BUFFER_START; */

/*---------------------  Export Functions  --------------------------*/
/* for inform allocation 0f TD RD & BUFFER have done!! */
BOOL g_bInit = FALSE;
static void register_dump(void)
{
	int i;
	DWORD state;
	printf("resister dump-------------------------------------------\n");
	for (i = 0; i < 32; i++) {
		VNSvInPortD(sg_aAdapter->dwIoBase+4*i, &state);
		printf("%08X\n", state);
	}
	printf("--------------------------------------------------------\n");
	return;
}

static void TD_RD_dump(void)
{
  int i;
	printf("TD dump-------------------------------------------------\n");
	for (i = 0; i < sg_aAdapter->cbTD; i++) {
		printf("TSR0:%02X ", sg_aAdapter->apTD[i]->m_td0TD0.byTSR0);
		printf("TSR1:%02X ", sg_aAdapter->apTD[i]->m_td0TD0.byTSR1);
		printf("OWNER:%02X\n", sg_aAdapter->apTD[i]->m_td0TD0.f1Owner);
		printf("Buffer length:%d ", sg_aAdapter->apTD[i]->m_td1TD1.f15TxBufLen);
		printf("Chain:%02X ", sg_aAdapter->apTD[i]->m_td1TD1.f1Chain);
		printf("TCR:%02X\n", sg_aAdapter->apTD[i]->m_td1TD1.byTCR);
		printf("Bufferaddress:%08X\n", sg_aAdapter->apTD[i]->m_dwTxBufferAddr);
		printf("NextTD:%08X\n", sg_aAdapter->apTD[i]->m_dwTxNextDescAddr);
	}
	printf("RD dump-------------------------------------------------\n");
	for (i = 0; i < sg_aAdapter->cbTD; i++) {
		printf("RSR0:%02X ", sg_aAdapter->apRD[i]->m_rd0RD0.byRSR0);
		printf("RSR1:%02X ", sg_aAdapter->apRD[i]->m_rd0RD0.byRSR1);
		printf("receive length:%d ", sg_aAdapter->apRD[i]->m_rd0RD0.f15FrameLen);
		printf("OWNER:%02X\n", sg_aAdapter->apRD[i]->m_rd0RD0.f1Owner);
		printf("Buffer length:%d\n", sg_aAdapter->apRD[i]->m_rd1RD1.f15RxBufLen);
		printf("Bufferaddress:%08X\n", sg_aAdapter->apRD[i]->m_dwRxBufferAddr);
		printf("NextRD:%08X\n", sg_aAdapter->apRD[i]->m_dwRxNextDescAddr););
	}
	printf("--------------------------------------------------------\n");
	return;
}
int mac_startio(bd_t *bd)
{
	int     ii, jj;
	UINT    uTotalAdapterNum = 0;
	UINT    uAdapterInWork = 0;

	PSAdapterInfo pAdapter;
	PSAdapterInfo apAdapter[CB_MAX_NET_DEVICE];

	/* INIT ADAPTER: */
	/* 1.allocate adapter structure */
	/* 2.Get info from NIC and store to adapter structure */
	uTotalAdapterNum = ADPuInitAll();
#if MACDBG
	printf("mac_startio - TotalAdapterNum=%d\n", uTotalAdapterNum);
#endif
	if (uTotalAdapterNum == 0) {
		printf("Cannot find the adapter \n");
		return 1;
	}

	uTotalAdapterNum = 0;

	for (ii = 0; ii < CB_MAX_NET_DEVICE; ii++) {
		if ((&sg_aAdapter[ii])->wDevId == W_DEVICE_ID_3106A ||
		(&sg_aAdapter[ii])->wDevId == W_DEVICE_ID_3053A) {
			/* Start driver: */
			/* 1.allocte TD/RD & buffer then init them */
			/* 2.MAC init */
			/* 3.enable INT */
			/* 4.Get status from NIC and store to adapter structure */
			if (!ADPbBind(&sg_aAdapter[ii])) {
				printf("Initialization of adapter[%d]'s TD RD or Init of MAC%d  fail.\n",
					ii, ii);
				continue;
			}
			apAdapter[uAdapterInWork] = &sg_aAdapter[ii];
			uAdapterInWork++;
			uTotalAdapterNum++;
#if MACDBG
			printf("| %d) 10/100 Ethernet MAC%d                |\n", ii, ii);
			printf("|     DevID:%04X  RevID:%02X               |\n",
				(&sg_aAdapter[ii])->wDevId, (&sg_aAdapter[ii])->byRevId);
#endif
		}
	}

	if (uTotalAdapterNum == 0) {
		printf("initialization is failed.\n");
		return 1;
	}

	for (ii = 0; ii < CB_MAX_NET_DEVICE; ii++) {
		/* init mac address from bd */
		if (ii == 0) {
			for (jj = 0; jj < U_ETHER_ADDR_LEN; jj++)
				VNSvOutPortB(apAdapter[ii]->dwIoBase + MAC_REG_PAR + jj, bd->bi_enetaddr[jj]);
		}
		/* Start mac TX/RX */
		MACvStart(apAdapter[ii]->dwIoBase);
	}
#if MACDBG
	printf("Initialization done..\n");
	register_dump();
	TD_RD_dump();
#endif
	g_bInit = TRUE;
	return 0;
}

static VOID s_vSetupData_TD(
	PSAdapterInfo pAdapter, int entry, int length, volatile void *packet
	)
{
	int      jj;
	PBYTE   pbyBuf;

	pbyBuf = (PBYTE)pAdapter->apTD[entry]->m_dwTxBufferAddr;
	jj = 0;
	while (jj < length) {
		*(PBYTE)(pbyBuf+jj)  = *((PBYTE)(packet+jj));
		jj++;
	}
	/* fill TD */
	pAdapter->apTD[entry]->m_td1TD1.f15TxBufLen = length >= 60 ? length : 60;
	pAdapter->apTD[entry]->m_td1TD1.f1Chain = 1;
	pAdapter->apTD[entry]->m_td1TD1.byTCR |= TCR_EDP|TCR_STP|TCR_IC;
	pAdapter->apTD[entry]->m_td0TD0.f1Owner = B_OWNED_BY_CHIP;
	return;
}

int mac_send(volatile void *packet, int length)
{
	int entry, dirty;
	/* use mac0 to send */
	entry = sg_aAdapter->idxTxCurDesc;
	dirty = sg_aAdapter->idxTxdirtyDesc;

	#if MACDBG
	printf("enter mac_send\n");
	#endif

    /* because of no interrupt, check TD status when sending begins */
    /* 1.check TD's ownership */
	if (sg_aAdapter->apTD[entry]->m_td0TD0.f1Owner == B_OWNED_BY_CHIP) {
		MACvRegBitsOn(sg_aAdapter->dwIoBase, (1 << 7), MAC_REG_TQWK);
		MACvTransmit(sg_aAdapter->dwIoBase);
		printf("TD overflow and let DMA send first\n");

		#if MACDBG
		TD_RD_dump();
		#endif
		return 1;
	}

    /* 2.clear current descriptor */
    sg_aAdapter->apTD[entry]->m_td0TD0.byTSR0 = 0;
    sg_aAdapter->apTD[entry]->m_td0TD0.byTSR1 = 0;
    sg_aAdapter->apTD[entry]->m_td0TD0.f12VID = 0;
    sg_aAdapter->apTD[entry]->m_td0TD0.f4Prioity = 0;

	/* 3.prepare data:memcopy */
	s_vSetupData_TD(sg_aAdapter, entry, length, packet);
	/* printf("init TD and data\n"); */

	/* 4.jump to next TD */
	sg_aAdapter->idxTxCurDesc = (++sg_aAdapter->idxTxCurDesc) % sg_aAdapter->cbTD;

	#if MACDBG
    printf("jump to next TD=%d\n", sg_aAdapter->idxTxCurDesc);
	#endif

	/* 5.trigger TDMD1& enable queue */
	MACvRegBitsOn(sg_aAdapter->dwIoBase, (1 << 7), MAC_REG_TQWK);
	MACvTransmit(sg_aAdapter->dwIoBase);

	#if MACDBG
	printf("data sent!\n", sg_aAdapter->idxTxCurDesc);
	#endif
	/* 6.error handling :checking TD if ABT, UND */

	return 0;
}

int mac_receive(void)
{
	PSRxDesc pRD;
	int length = 0;
	DWORD isr_status;
	/* 1.store ISR */
	MACvReadISR(sg_aAdapter->dwIoBase, sg_aAdapter->byRevId, &isr_status);
	/* 2.disable IMR */
	MACvIntDisable(sg_aAdapter->dwIoBase, sg_aAdapter->byRevId);
	/* 3clear ISR */
	MACvWriteISR(sg_aAdapter->dwIoBase, sg_aAdapter->byRevId, isr_status);

	/* 4.handle ISR */
	/* 5.handle received packets until owner==chip */
	/* printf("enter mac_receive\n"); */
	while (TRUE) {
		pRD = sg_aAdapter->apRD[sg_aAdapter->idxRxCurDesc];
		/* a.check RD status */
		/* if owner==chip break; */
		if (pRD->m_rd0RD0.f1Owner == B_OWNED_BY_CHIP) {
			/* printf("receive done \n"); */
			break;
		}
		/* if ok, net_receive() */
		if (pRD->m_rd0RD0.byRSR1&RSR1_RXOK) {
			NetReceive((volatile uchar *)pRD->m_dwRxBufferAddr, pRD->m_rd0RD0.f15FrameLen-4);
			length += (pRD->m_rd0RD0.f15FrameLen-4);

#if MACDBG
			printf("receive ok\n");
#endif
		} else{
			/* else, error handling */
			printf("receive error status:%02X %02X%\n",
				pRD->m_rd0RD0.byRSR1,
				pRD->m_rd0RD0.byRSR0);
			/* handler will do later */
		}

		/* b.set own==chip */
		pRD->m_rd0RD0.f1Owner = B_OWNED_BY_CHIP;

		/* c.increase to next RD */
		sg_aAdapter->idxRxCurDesc = (++sg_aAdapter->idxRxCurDesc)%sg_aAdapter->cbRD;

		#if MACDBG
		printf("jump to next RD =%d\n", sg_aAdapter->idxRxCurDesc);
		#endif
	}
	/* 6.enable IMR */
	MACvIntEnable(sg_aAdapter->dwIoBase, sg_aAdapter->byRevId, IMR_MASK_VALUE);
	return length;
}

void mac_halt(void)
{
#if MACDBG
	printf("mac halt\n");
#endif
	if (g_bInit)
		ADPbShutdown(sg_aAdapter);
  return;
}

void mii_read(char *devname, unsigned char addr,
		unsigned char reg, unsigned short *value)
{
    SAdapterInfo Adapter;
	Adapter.dwIoBase = BA_MAC0;
    Adapter.byPhyId = MACbyGetPhyId(Adapter.dwIoBase);
	MIIbReadEmbedded(Adapter.dwIoBase, Adapter.byPhyId, addr, value);
	#if MACDBG
	printf("%02X %02X %04X\n", addr, reg, *value);
	#endif
}

void mii_write(char *devname, unsigned char addr,
		unsigned char reg, unsigned short value)
{
    SAdapterInfo Adapter;
	Adapter.dwIoBase = BA_MAC0;
	Adapter.byPhyId = MACbyGetPhyId(Adapter.dwIoBase);
	MIIbWriteEmbedded(Adapter.dwIoBase, Adapter.byPhyId, addr, value);
	#if MACDBG
	printf("%02X %02X %04X\n", addr, reg, value);
	#endif
}
