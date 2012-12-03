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


#ifndef __DESC_H__
#define __DESC_H__

#if !defined(__TTYPE_H__)
#include "ttype.h"
#endif
#if !defined(__TMEM_H__)
#include "tmem.h"
#endif




/*---------------------  Export Definitions -------------------------*/

#define B_OWNED_BY_CHIP     1
#define B_OWNED_BY_HOST     0
#define B_TD_LIST_NOT_END   1
#define B_TD_LIST_END       0

/*
 * Bits in the RSR0 register
 */
#define RSR0_DETAG          0x80
#define RSR0_SNTAG          0x40
#define RSR0_RXER           0x20
#define RSR0_RL             0x10
#define RSR0_CE             0x08
#define RSR0_FAE            0x04
#define RSR0_CRC            0x02
#define RSR0_VIDM           0x01

/*
 * Bits in the RSR1 register
 */
#define RSR1_RXOK           0x80        /* rx OK */
#define RSR1_PFT            0x40        /* Perfect filtering address match */
#define RSR1_MAR            0x20        /* MAC accept multicast address packet */
#define RSR1_BAR            0x10        /* MAC accept broadcast address packet */
#define RSR1_PHY            0x08        /* MAC accept physical address packet */
#define RSR1_VTAG           0x04        /* 802.1p/1q tagging packet indicator */
#define RSR1_STP            0x02        /* start of packet */
#define RSR1_EDP            0x01        /* end of packet */

/*
 * Bits in the CSM register
 */
#define CSM_IPOK            0x40        /* IP Checkusm validatiaon ok */
#define CSM_TUPOK           0x20        /* TCP/UDP Checkusm validatiaon ok */
#define CSM_FRAG            0x10        /* Fragment IP datagram */
#define CSM_IPKT            0x04        /* Received an IP packet */
#define CSM_TCPKT           0x02        /* Received a TCP packet */
#define CSM_UDPKT           0x01        /* Received a UDP packet */

/*
 * Bits in the TSR0 register
 */
#define TSR0_ABT            0x80        /* Tx abort because of excessive collision */
#define TSR0_OWT            0x40        /* Jumbo frame Tx abort */
#define TSR0_OWC            0x20        /* Out of window collision */
#define TSR0_COLS           0x10        /* experience collision in this transmit event */
#define TSR0_NCR3           0x08        /* collision retry counter[3] */
#define TSR0_NCR2           0x04        /* collision retry counter[2] */
#define TSR0_NCR1           0x02        /* collision retry counter[1] */
#define TSR0_NCR0           0x01        /* collision retry counter[0] */

/*
 * Bits in the TSR1 register
 */
#define TSR1_TERR           0x80
#define TSR1_FDX            0x40        /* current transaction is serviced by full duplex mode */
#define TSR1_GMII           0x20        /* current transaction is serviced by GMII mode */
#define TSR1_LNKFL          0x10        /* packet serviced during link down */
#define TSR1_SHDN           0x04        /* shutdown case */
#define TSR1_CRS            0x02        /* carrier sense lost */
#define TSR1_CDH            0x01        /* AQE test fail (CD heartbeat) */

/*
 * Bits in the TCR0 register
 */
#define TCR0_TIC            0x80        /* assert interrupt immediately while descriptor has been */
/*                                         send complete                                          */
#define TCR0_PIC            0x40        /* priority interrupt request, INA# is issued over adaptive */
/*                                         interrupt scheme                                         */
#define TCR0_VETAG          0x20        /* enable VLAN tag */
#define TCR0_IPCK           0x10        /* request IP  checksum calculation. */
#define TCR0_UDPCK          0x08        /* request UDP checksum calculation. */
#define TCR0_TCPCK          0x04        /* request TCP checksum calculation. */
#define TCR0_JMBO           0x02        /* indicate a jumbo packet in GMAC side */
#define TCR0_CRC            0x01        /* disable CRC generation */


/* max transmit or receive buffer size */
/* #define CB_MAX_BUF_SIZE    16320UL      max buffer size */
/* #define CB_MAX_BUF_SIZE     9040UL      max buffer size             */

#define CB_MAX_BUF_SIZE     2048UL      /*max buffer size             */
/*                                         NOTE: must be multiple of 4 */

#define CB_MAX_RD_NUM       512         /* MAX # of RD */
#define CB_MAX_TD_NUM       256         /* MAX # of TD */

#define CB_INIT_RD_NUM_3119 128         /* init # of RD, for setup VT3119 */
#define CB_INIT_TD_NUM_3119 64	        /* init # of TD, for setup VT3119 */

#define CB_INIT_RD_NUM      128         /* init # of RD, for setup default */
#define CB_INIT_TD_NUM      64          /* init # of TD, for setup default */

/* for 3119 */
#define CB_TD_RING_NUM      1           /* # of TD rings. */
#define CB_MAX_SEG_PER_PKT  7           /* max data seg per packet (Tx) */
#define CB_MIN_SEG_PER_PKT  1           /* min data seg per packet (Tx) */


/* if collisions excess 15 times , tx will abort, and */
/* if tx fifo underflow, tx will fail */
/* we should try to resend it */
#define CB_MAX_TX_ABORT_RETRY   3


/*---------------------  Export Types  ------------------------------*/

/*
 * receive descriptor
 */
typedef struct tagRDES0 {
    BYTE    byRSR0;
    BYTE    byRSR1;

    WORD    f14RMBC : 14;           /* receive packet length */
    WORD    f1Resv  : 1;
    WORD    f1Owner : 1;
} SRDES0;

typedef struct tagRDES1 {
    WORD    wPQTAG;
    BYTE    byCSM;
    BYTE    byIPKT;
} SRDES1;

typedef struct tagSRxDesc {
    SRDES0  m_rd0RD0;
    SRDES1  m_rd1RD1;
    DWORD   m_dwRxBufAddrLo;
    WORD    m_wRxBufAddrHi;

    WORD    m_f15RxBufSize : 15;
    WORD    m_f1IntCtlEn   : 1;
} SRxDesc, DEF * PSRxDesc;

typedef const SRxDesc DEF * PCSRxDesc;


/*
 * transmit descriptor
 */
typedef struct tagTDES0 {
    BYTE    byTSR0;
    BYTE    byTSR1;

    WORD    f14TxPktSize : 14;
    WORD    f1Reserved   : 1;
    WORD    f1Owner      : 1;
} STDES0;

typedef struct tagTDES1 {
    WORD    f12VID       : 12;
    WORD    f1CFI        : 1;
    WORD    f3Prioity    : 3;

    BYTE    byTCR0;

    BYTE    f2TCPLS      : 2;
    BYTE    f2Reserved   : 2;
    BYTE    f4CMDZ       : 4;
} STDES1;

typedef struct tagTDBUF {
    DWORD   dwTxBufAddrLo;
    WORD    wTxBufAddrHi;

    WORD    f14TxBufSize : 14;
    WORD    f1Reserved : 1;
    WORD    f1Que : 1;

} STDBUF;

typedef struct tagSTxDesc {
    STDES0  m_td0TD0;
    STDES1  m_td1TD1;
    STDBUF  m_asTxBufs[7];
} STxDesc, DEF * PSTxDesc;

typedef const STxDesc DEF * PCSTxDesc;


/*---------------------  Export Macros ------------------------------*/

/*---------------------  Export Classes  ----------------------------*/

/*---------------------  Export Variables  --------------------------*/

/*---------------------  Export Functions  --------------------------*/


#endif /* __DESC_H__ */

