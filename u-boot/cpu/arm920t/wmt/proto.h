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

#ifndef __PROTO_H__
#define __PROTO_H__

#if !defined(__TTYPE_H__)
#include "ttype.h"
#endif
#if !defined(__ADAPTER_H__)
#include "adapter.h"
#endif

#define	HW_8021PQ_TAG	    TCR0_VETAG
#define	HW_IP_CHECKSUM 	    TCR0_IPCK
#define HW_UDP_CHECKSUM     TCR0_UDPCK
#define HW_TCP_CHECKSUM     TCR0_TCPCK

/*---------------------  Export Definitions -------------------------*/
/*
 * Protocol buffer ring
 */
typedef struct tagSProtocolBuffer {

    UINT    uProtoIndex;                /* protocol index */

    int     idxRxBufHead;
    int     idxRxBufTail;
    PBYTE   apbyRxBuffer[CB_MAX_RD_NUM];
    UINT    auRxBufLength[CB_MAX_RD_NUM];
    BOOL    abRxBufOccupied[CB_MAX_RD_NUM];

    /* for pingpong test */
    BYTE    abyDstAddr[U_ETHER_ADDR_LEN];
    BYTE    abySrcAddr[U_ETHER_ADDR_LEN];
    DWORD   dwSerialNo;
    BYTE    byDataSeed;

} SProtocolBuffer, DEF * PSProtocolBuffer;


/*---------------------  Export Classes  ----------------------------*/

/*---------------------  Export Variables  --------------------------*/
extern SProtocolBuffer sg_aGProBuf[MAX_NET_DEVICE];

/*---------------------  Export Functions  --------------------------*/
#ifdef __cplusplus
extern "C" {                            /* Assume C declarations for C++ */
#endif /* __cplusplus */


BOOL GPRObDriverSend(PSAdapterInfo pAdapter, PBYTE pbyPacket, UINT uPktLen, BYTE byTxQue);
BOOL GPRObDriverSendEx(PSAdapterInfo pAdapter, BYTE byTxQue, WORD wVID,
		BYTE byPri, BYTE byHwOpt, PBYTE pbyPacket, UINT uPktLen);
BOOL GPRObProtocolReceive(PSAdapterInfo pAdapter, PBYTE *ppbyPacket, PUINT puPktLen);
VOID GPROvProtocolReturnPacket(PSAdapterInfo pAdapter);

BOOL GPRObProtocolConstruct(PSAdapterInfo pAdapter);
VOID GPROvProtocolDestruct(PSAdapterInfo pAdapter);

VOID GPROvPrepareTxPacketEx(PSAdapterInfo pAdapter, BYTE byTxQue, UINT uIdx, UINT uNumSeg);

VOID GPROvDoTransmitSinglePacket(PSAdapterInfo pAdapter, BYTE byTxQue);

VOID GPROvDoTransmitEx(PSAdapterInfo pAdapter, BYTE byTxQue);

/* VOID vSetBit(PDWORD pdwTD0); */

#ifdef __cplusplus
}                                       /* End of extern "C" { */
#endif /* __cplusplus */

#endif /* __PROTO_H__ */



