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


#ifndef __TETHER_H__
#define __TETHER_H__

#if !defined(__TTYPE_H__)
#include "ttype.h"
#endif

/*---------------------  Export Definitions -------------------------*/
/*
 * constants
 */
#define U_ETHER_ADDR_LEN    6           /* Ethernet address length */
#define U_TYPE_LEN          2
#define U_CRC_LEN           4
#define U_HEADER_LEN        (U_ETHER_ADDR_LEN * 2 + U_TYPE_LEN)
#define U_ETHER_ADDR_STR_LEN (U_ETHER_ADDR_LEN * 2 + 1) /* Ethernet address string length */
#define MIN_DATA_LEN        46          /* min data length */
#define MAX_DATA_LEN        1500        /* max data length */

#define MIN_PACKET_LEN      (MIN_DATA_LEN + U_HEADER_LEN) /* 60, min total packet length (tx)   */
#define MAX_PACKET_LEN      (MAX_DATA_LEN + U_HEADER_LEN) /* 1514, max total packet length (tx) */
#define MAX_JUMBO_PKT_LEN   9040

#define MAX_LOOKAHEAD_SIZE  MAX_PACKET_LEN

#define U_MULTI_ADDR_LEN    8           /* multicast address length */

/*
 * wType field in the SEthernetHeader
 *
 * NOTE....
 *   in network byte order, high byte is going first
 */
#define TYPE_PKT_IP         0x0008
#define TYPE_PKT_ARP        0x0608
#define TYPE_PKT_RARP       0x3580
#define TYPE_PKT_IPX        0x3781

#define TYPE_PKT_PING_M_REQ 0x1180      /* master reguest */
#define TYPE_PKT_PING_S_GNT 0x2280      /* slave grant */
#define TYPE_PKT_PING_M     0x7780      /* pingpong master packet */
#define TYPE_PKT_PING_S     0x8880      /* pingpong slave packet */
#define TYPE_PKT_DISCONN_M  0x9980      /* pingpong master disconnect packet */
#define TYPE_PKT_WOL_M_REQ  0x3380      /* WOL waker request */
#define TYPE_PKT_WOL_S_GNT  0x4480      /* WOL sleeper grant */

/*
 * wFrameCtl field in the S802_11Header
 *
 * NOTE....
 *   in network byte order, high byte is going first
 */

#define FC_TODS             0x0100
#define FC_FROMDS           0x0200
#define FC_MOREFRAG         0x0400
#define FC_RETRY            0x0800
#define TYPE_802_11_DATA    0x0008
#define TYPE_802_11_MASK    0x000C
#define TYPE_802_11_RTS     0x00B4

#define TYPE_802_11_CONTROL 0x0004
#define TYPE_802_11_MGMT    0x0000

#define U_WMAC_HEADER_LEN   24
/*---------------------  Export Types  ------------------------------*/

/*
 * Ethernet packet
 */
typedef struct tagSEthernetHeader {
    BYTE    abyDstAddr[U_ETHER_ADDR_LEN];
    BYTE    abySrcAddr[U_ETHER_ADDR_LEN];
    WORD    wType;
} SEthernetHeader, DEF * PSEthernetHeader;

/*
 * 802_3 packet
 */
typedef struct tagS802_3Header {
    BYTE    abyDstAddr[U_ETHER_ADDR_LEN];
    BYTE    abySrcAddr[U_ETHER_ADDR_LEN];
    WORD    wLen;
} S802_3Header, DEF * PS802_3Header;

typedef struct tagS802_2LCCHeader {
    BYTE    abyDstAddr[U_ETHER_ADDR_LEN];
    BYTE    abySrcAddr[U_ETHER_ADDR_LEN];
    WORD    wLen;
    BYTE    byDSAP;
    BYTE    bySSAP;
    BYTE    byCtrl;
} S802_2LCCHeader, DEF * PS802_2LCCHeader;

typedef struct tagS802_3SNAPHeader {
    BYTE    abyDstAddr[U_ETHER_ADDR_LEN];
    BYTE    abySrcAddr[U_ETHER_ADDR_LEN];
    WORD    wLen;
    BYTE    byDSAP;
    BYTE    bySSAP;
    BYTE    byCtrl;
    BYTE    Org[3];
    WORD    wType;
} S802_2SNAPHeader, DEF * PS802_2SNAPHeader;

typedef struct tagMACCtrlFrameHeader {
    BYTE    abyDstAddr[U_ETHER_ADDR_LEN];
    BYTE    abySrcAddr[U_ETHER_ADDR_LEN];
    WORD    wType;
    WORD    wOpCode;
    WORD    wPauseTimer;
    BYTE    abyReserved[42];
} SMACCtrlFrameHeader, DEF * PSMACCtrlFrameHeader;

/*
 * 802_11 packet
 */
typedef struct tagS802_11Header {
    WORD    wFrameCtl;
    WORD    wDurationID;
    BYTE    abyAddr1[U_ETHER_ADDR_LEN];
    BYTE    abyAddr2[U_ETHER_ADDR_LEN];
    BYTE    abyAddr3[U_ETHER_ADDR_LEN];
    WORD    wSeqCtl;
#ifdef DS2DS
    BYTE    abyAddr4[U_ETHER_ADDR_LEN];
#endif
    DWORD   dwIV;
} S802_11Header, DEF * PS802_11Header;

/*---------------------  Export Macros ------------------------------*/
/* Frame type macro */

#define IS_MULTICAST_ADDRESS(pbyEtherAddr)          \
    ((*(PBYTE)(pbyEtherAddr) & 0x01) == 1)

#define IS_BROADCAST_ADDRESS(pbyEtherAddr) (        \
    (*(PDWORD)(pbyEtherAddr) == 0xFFFFFFFFL) &&     \
    (*(PWORD)((PBYTE)(pbyEtherAddr) + 4) == 0xFFFF) \
)

#define IS_NULL_ADDRESS(pbyEtherAddr) (             \
    (*(PDWORD)(pbyEtherAddr) == 0L) &&              \
    (*(PWORD)((PBYTE)(pbyEtherAddr) + 4) == 0)      \
)

#define IS_ETH_ADDRESS_EQUAL(pbyAddr1, pbyAddr2) (  \
    (*(PDWORD)(pbyAddr1) == *(PDWORD)(pbyAddr2)) && \
    (*(PWORD)((PBYTE)(pbyAddr1) + 4) ==             \
    *(PWORD)((PBYTE)(pbyAddr2) + 4))                \
)

/*---------------------  Export Classes  ----------------------------*/

/*---------------------  Export Variables  --------------------------*/

/*---------------------  Export Functions  --------------------------*/
#ifdef __cplusplus
extern "C" {                            /* Assume C declarations for C++ */
#endif /* __cplusplus */

BYTE ETHbyGetHashIndexByCrc32(PBYTE pbyMultiAddr);
/* BYTE ETHbyGetHashIndexByCrc(PBYTE pbyMultiAddr); */
BOOL ETHbIsBufferCrc32Ok(PBYTE pbyBuffer, UINT cbFrameLength);

#ifdef __cplusplus
}                                       /* End of extern "C" { */
#endif /* __cplusplus */

#endif /* __TETHER_H__ */
