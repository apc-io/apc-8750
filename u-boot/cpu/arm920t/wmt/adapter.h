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


#ifndef __ADAPTER_H__
#define __ADAPTER_H__

#if !defined(__TTYPE_H__)
#include "ttype.h"
#endif
#if !defined(__TETHER_H__)
#include "tether.h"
#endif
#if !defined(__UPC_H__)
#include "upc.h"
#endif
#if !defined(__PCINET_H__)
#include "pcinet.h"
#endif
#if !defined(__MIB_H__)
#include "mib.h"
#endif
#if !defined(__DESC_H__)
#include "desc.h"
#endif

/*---------------------  Export Definitions -------------------------*/

/*
 * constants
 */
#define W_VENDOR_ID        0x1106  /* vendor ID */
#define W_DEVICE_ID_3119A  0x3119  /* device ID */

/*
 * Adapter Information
 */
typedef struct tagSAdapterInfo {
    UINT    cbTotalAdapterNum;
    UINT    uAdapterIndex;                              /* device index */

    /* got from PCI regs */
    WORD    wBusDevFunID;
    WORD    wDeviceId;
    BYTE    byRevId;                                    /* The Revision Number of this adapter */
    DWORD   dwIoBase;
    DWORD   dwMemBase;
    DWORD   dwIoMapBase;
    BYTE    byIrqLevel;                                 /* IRQ number */
    DWORD   dwBootRomBase;                              /* boot ROM base address */
    BYTE    byPMRegOffset;                              /* the content of Cap_Ptr */

    /* got from MAC regs (loaded from EEPROM) */
    BYTE    byBootRomSize;                              /* boot ROM size */
    BYTE    byPhyId;
    BYTE    abyEtherAddr[U_ETHER_ADDR_LEN];
    TCHAR   aszEtherAddrStr[U_ETHER_ADDR_STR_LEN];

    /* got from MII regs (updated by Link-Change) */
    DWORD   dwPhyCmrId;                                 /* Company/Module/Revision ID of the PHY */
    BOOL    bLinkPass;                                  /* current link status */
    BOOL    bSpeed100M;                                 /* current speed status 100/10 */
    BOOL    bSpeed1G;                                   /* current speed status 1G */
    BOOL    bFullDuplex;                                /* current duplex status */
    UINT    uConnectionType;                            /* for media connect type parameter */

    /* for TD/RD alignment */
    DWORD   dwCacheLineSize;                            /* 64 bytes in 3119 */

    /* for RDR */
    UINT            cbRD;                               /* number of total RD */
    PSRxDesc *apRD;                                     /* RD pointer array */
    SAllocMap       amRxDescRing;                       /* RD ring SAllocMap */
    SAllocMap       amRxDescBuf;                        /* RD databuf SAllocMap */
    UINT            uRxDequeDescIdx;                    /* curr RD index */
    UINT            iRxPktHandled;                      /* rx packets has been handled */
    DWORD           dwRxFifoAddr;                       /* Rx Fifo Address Tracing */
    DWORD           dwRxFifoSize;

    /* multi-RD receive */
    UINT            idxRxPktStartDesc;                  /* if multi-RD, this is the starting RD */
    UINT            cbToBeFreeRD;                       /* Record the number of used RD not freed yet */

    /* for TDR */
    UINT            cbTD;                               /* number of total TD for each TD queue */
    PSTxDesc * aapTD[CB_TD_RING_NUM];                    /* TD pointer array for each TD queue */
    SAllocMap       aamTxDescRing[CB_TD_RING_NUM];      /* TD ring SAllocMap for each TD queue */
    SAllocMap *aaamTxDescBufPool[CB_TD_RING_NUM];       /* TD databuf SAllocMap array for each TD queue */

    UINT            aidxTxPktStartDesc[CB_TD_RING_NUM]; /* if multi-TD, this is the starting TD */
    UINT            auTxPktPosted[CB_TD_RING_NUM];      /* tx packets has posted for each TD queue */
    UINT            cbTxAbortRetry;

    UINT            uTxEnqueDescIdx[CB_TD_RING_NUM];    /* Record current TD index for sending */
    UINT            uTxDequeDescIdx[CB_TD_RING_NUM];    /* Record current TD index for ISR */
    DWORD           dwTxFifoAddr[CB_TD_RING_NUM];       /* Tx Fifo Address Tracing */
    DWORD           dwTxFifoSize;

    /* for statistic */
    SStatCounter    scStatistic;                        /* RD, IRQ statisics */
    SStatCounter    ascStatistic[CB_TD_RING_NUM];       /* TD statisics for each TD queue */
    SHWMibCounter   sHWMibCounter;
    SMib2Counter    mibCounter;
    SRmonCounter    rmonCounter;

    /* for IRQ */
    BYTE    byIntNum;                                   /* INT number mapped from IRQ#, Add by Eric */
    WORD    wOriPmSelector;                             /* Add by Eric */
    DWORD   dwOriPmOffset;                              /* Add by Eric */
    WORD    wOriRmSegment;                              /* Add by Eric */
    WORD    wOriRmOffset;                               /* Add by Eric */
    WORD    wBaseSelector;                              /* Add by Eric */
    void    (*pisrOrgIsr)(void);
    void    (*pisrAdapterIsr)(void);

    BOOL    bSharedIrqSupport;                          /* support shared hardware IRQ */

    /* for packet driver interface */
    int     iIntNo;                                     /* soft intr vector */
    WORD    wRcvTypeLen;
    BYTE    abyRcvType[20];
    INTERRUPT(DEF * pisrReceiver)(void);

    /* for pingpong test */
    void    (DEF * pisrPptReceiver)(void);

    /* for Diag parameters */
    BOOL    bTxOn;                              /* Tx ON */
    BOOL    bTxContFunTest;                     /* for functional test (Tx Continuous - functional test) */
    BOOL    bTxContPerfTest;                    /* for performance test (Tx Continuous - performance test) */
    BOOL    bTxSinglePacket;                    /* Tx no burst packets */
    BOOL    bRandomPktLength;                   /* random packet length */
    BOOL    bRandomPktData;                     /* random packet data */
    BOOL    bIncreasePktData;                   /* [1.05] increase packet data */
    BYTE    byDataIncreased;                    /* [1.05] used for increase packet data */
    BOOL    bSoftwareGenCrc;                    /* determine whether sw-gen-crc */
    BOOL    bSoftwareCheckCrc;                  /* determine whether sw-check-crc */
    UINT    uTxGapTime;                         /* uTxGapTime is a fixed setting */
    UINT    uTxGapTimeShadow;                   /* uTxGapTimeShadow will decrease as time by */
    BOOL    bWhenRxDataErrNoBuf;                /* if Rx data error NOT release owner */
    BOOL    bIncPktLength;                      /* increase packet length */
    UINT    cbPktSize;                          /* fixed Tx packet size */
    BOOL    bWhenRxDataErrOutPort0x80;          /* if Rx data error output port 0x80 0xFF */
    BOOL    bWhenTxErrOutPort0x80;              /* if Tx error output port 0x80 0xAA */
    BOOL    bWhenRxCrcErrNoBuf;                 /* if Rx CRC error NOT release owner */
    UINT    uTxPktSize;                         /* Tx packet size if bIncPktLength = TRUE */

    LONG    lTxPacketNum;                       /* Tx packet number */
    LONG    lTxPacketNumShadow;                 /* Tx packet number will decrease as time by */
    UINT    uTxDataSegsPerPacket;               /* how many data segments in per Tx desc(packet) */
    BOOL    bRandDatSegsPerPacket;              /* random data segments in per Tx desc(packet) */
    BOOL    b8021pqTag;                         /* random 802.1pq Tag */
    BOOL    bRandomDestEthernet;                /* random Dest Ethernet Addr */

    /* for misc */
    BOOL    bRefreshScreen;                     /* refresh screen to update counter value */
    int     idxRxErrorDesc;                     /* index for rx data error RD */
    PBYTE   pbyTmpBuff;                         /* tmp buffer */

    /* It's reserved for adding new structure members of this structure */
    LPVOID          lpvUserDef;

    /* Eric, check CRC */
    BOOL    bShowLbPacketComp;
    BOOL    bFuncReTx;
    UINT    uReTxDelay;
    BYTE    byTimerDebug;

    /* For script */
    PVOID   mempool;

    /* For TD/RD Access Buffer Function */
    WORD    wAccessBufOffset;
    BYTE    abyFillBufValue[8];

    /* Adaptive Interrupt */
    BYTE    byIntHoldTimer;
    BYTE    byTxSupThr;
    BYTE    byRxSupThr;

    /* Flow Ctrl */
    BYTE    byFlowCtrlHiThr;
    BYTE    byFlowCtrlLoThr;
} SAdapterInfo, DEF * PSAdapterInfo;

typedef struct _adapter_option {
    int     iTdRingNum;
    int     iRDescNum;
    int     iTDescNum;
    BYTE    byRevId;
    BOOL    bNoInit;
    ULONG   ulInitCmds;
} SAdapterOpts, DEF * PSAdapterOpts;

#define INIT_CMD_NONE               0
#define INIT_CMD_FORCE_PHYRST       0x00000001UL

#define W_MAX_TIMEOUT               0x0FFFU

/*---------------------  Export Classes  ----------------------------*/

/*---------------------  Export Variables  --------------------------*/
extern PSAdapterInfo    sg_aGAdapter;
extern SAdapterOpts     sOptions;
extern PBYTE            sg_abyRandomTable;
/*---------------------  Export Functions  --------------------------*/
#ifdef __cplusplus
extern "C" {                            /* Assume C declarations for C++ */
#endif /* __cplusplus */

UINT GADPuInitAll(void);
BOOL GADPbInitOne(unsigned char bySlotNum, PSAdapterInfo *ppAdapter);
BOOL GADPbBind(PSAdapterInfo pAdapter);
BOOL GADPbShutdown(PSAdapterInfo pAdapter);
BOOL GADPbInitialize(PSAdapterInfo pAdapter);

#ifdef __cplusplus
}                                       /* End of extern "C" { */
#endif /* __cplusplus */

#endif /* __ADAPTER_H__ */
