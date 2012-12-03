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


#ifndef __MII_H__
#define __MII_H__

#if !defined(__TTYPE_H__)
#include "ttype.h"
#endif

/*---------------------  Export Definitions -------------------------*/

#define MAX_PHY_DEVICE          32      /* max. # of PHY in a MII bus */

/*
 * Registers in the MII (offset unit is WORD)
 */
#define MII_REG_BMCR        0x00    /* Mode Control Register */
#define MII_REG_BMSR        0x01    /* Mode Status Register */
#define MII_REG_PHYID1      0x02    /* OUI */
#define MII_REG_PHYID2      0x03    /* OUI + Module ID + REV ID */
#define MII_REG_ANAR        0x04    /* Auto-Negotiation Advertisement Register */
#define MII_REG_ANLPAR      0x05    /* Auto-Negotiation Link Partner Ability Register */
#define MII_REG_ANER        0x06    /* Auto-Negotiation Expansion Register */
#define MII_REG_ANNPTR      0x07    /* Auto-Negotiation Next-Page Transmit Register */
#define MII_REG_ANLPNP      0x08    /* Auto-Negotiation Link Partner Next-Page Receive Register */
#define MII_REG_GCR         0x09    /* 1000BASE-T Control Register */
#define MII_REG_GSR         0x0A    /* 1000BASE-T Status Register */
#define MII_REG_GSER        0x0F    /* 1000BASE-T Status Extension Register #1 */
#define MII_REG_TXSER       0x10    /* 100BASE-TX Status Extension Register */
#define MII_REG_GSER2       0x11    /* 1000BASE-T Status Extension Register #2 */
#define MII_REG_BYPASS      0x12    /* Bypass Control Register */
#define MII_REG_RECR        0x13    /* Receive Error Counter Register */
#define MII_REG_FCSCR       0x14    /* False Carrier Sense Counter Register */
#define MII_REG_DCR         0x15    /* Disconnect Counter Register */
#define MII_REG_TCSR        0x16    /* 10BASE-T Control & Status Register */
#define MII_REG_EPCR1       0x17    /* Extended PHY Control Register #1 */
#define MII_REG_EPCR2       0x18    /* Extended PHY Control Register #2 */
#define MII_REG_IMR         0x19    /* Interrupt Mask Register */
#define MII_RGE_ISR         0x1A    /* Interrupt Status Register */
#define MII_REG_LED         0x1B    /* Parallel LED Control Register */
#define MII_REG_AUXCSR      0x1C    /* Auxiliary Control & Status Register */
#define MII_REG_DELAY       0x1D    /* Delay Skew Status Register */

/* Marvell 88E1000/88E1000S */
#define MII_REG_PSCR        0x10    /* PHY specific control register */

/* ICPLUS IP1001 */
#define MII_REG_PSCSR       0x10    /* PHY Specific Control & Status Register */
#define MII_REG_DBGCR       0x13    /* PHY Debug Control Register */
#define MII_REG_PSCR2       0x14    /* PHY Specific Control Register2 */
#define MII_REG_ADJUST      0x1E    /* PHY adjustment */

/*
 * Bits in the BMCR register
 */
#define BMCR_RESET          0x8000
#define BMCR_LBK            0x4000
#define BMCR_SPEED100       0x2000
#define BMCR_AUTO           0x1000
#define BMCR_PD             0x0800
#define BMCR_ISO            0x0400
#define BMCR_REAUTO         0x0200
#define BMCR_FDX            0x0100
#define BMCR_SPEED1G        0x0040

/*
 * Bits in the BMSR register
 */
#define BMSR_AUTOCM         0x0020
#define BMSR_LNK            0x0004

/*
 * Bits in the ANAR register
 */
#define ANAR_ASMDIR         0x0800      /* Asymmetric PAUSE support */
#define ANAR_PAUSE          0x0400      /* Symmetric PAUSE Support */
#define ANAR_T4             0x0200
#define ANAR_100FD          0x0100
#define ANAR_100            0x0080
#define ANAR_10FD           0x0040
#define ANAR_10             0x0020

/*
 * Bits in the ANLPAR register
 */
#define ANLPAR_ASMDIR       0x0800      /* Asymmetric PAUSE support */
#define ANLPAR_PAUSE        0x0400      /* Symmetric PAUSE Support */
#define ANLPAR_T4           0x0200
#define ANLPAR_100FD        0x0100
#define ANLPAR_100          0x0080
#define ANLPAR_10FD         0x0040
#define ANLPAR_10           0x0020

/*
 * Bits in the GCR register
 */
#define GCR_1000FD      0x0200      /* PHY is 1000-T Full-duplex capable */
#define GCR_1000        0x0100      /* PHY is 1000-T Half-duplex capable */

/*
 * Bits in the GSR register
 */
#define GSR_1000FD      0x0800      /* LP PHY is 1000-T Full-duplex capable */
#define GSR_1000        0x0400      /* LP PHY is 1000-T Half-duplex capable */


/*
 * Bits in the TCSR register (16h)
 */
#define TCSR_LINKDIS        0x8000  /* Link Disable */
#define TCSR_JABDIS         0x4000  /* Jabber Detect Disable */
#define TCSR_ECHODIS        0x2000  /* Disable 10BASE-T/100BASE-TX Echo Mode */
#define TCSR_SQEDIS         0x1000  /* SQE Disable Mode */
#define TCSR_SQCTRL1        0x0800  /* Squelch Control */
#define TCSR_SQCTRL0        0x0400
#define TCSR_EOF            0x0100  /* EOF Error Detected */
#define TCSR_TDIS           0x0080  /* 10BASE-T Disconnect State */
#define TCSR_TLS            0x0040  /* 10BASE-T Link Status */

/*
 * Bits in the PLEDCR register for CICADA
 */
#define LED_LNK10_ON        0x8000
#define LED_LNK10_DIS       0x4000
#define LED_LNK100_ON       0x2000
#define LED_LNK100_DIS      0x1000
#define LED_LNK1000_ON      0x0800
#define LED_LNK1000_DIS     0x0400
#define LED_DPX_ON          0x0200
#define LED_DPX_DIS         0x0100
#define LED_ACT_ON          0x0080
#define LED_ACT_DIS         0x0040
#define LED_LALBE           0x0004	    /* Link/Activity LED Blink Enable */

/*
 * Bits in the AUXCSR (0x1C) register for CICADA
 */
#define AUXCSR_MDPPS        0x0004


/* Marvell 88E1000/88E1000S Bits in the PHY specific control register (10h) */
#define PSCR_ACRSTX         0x0800      /* Assert CRS on Transmit */

/* Bits in the PSCSR register for ICPLUS IP1001 */
#define PSCSR_FORCELK       0x2000
#define PSCSR_EXTLPBK       0x8000

/* Bits in the DBGCR register for ICPLUS IP1001 */
#define DBGCR_BPNWAY        0x0200      /* Bypass NWAY */

/* Bits in the PSCR2 register for ICPLUS IP1001 */
#define PSCR2_ACEN          0x0004      /* Enable auto MDI/MDIX */
#define PSCR2_MDIXEN        0x0200      /* MDIX enable */
#define PSCR2_APSON         0x0800      /* Activate auto power saving (APS) mode */

/* Loopback mode */
#define MII_LB_NONE         0x00
#define MII_LB_INTERNAL     0x01
#define MII_LB_ISO          0x02        /* isolate endec/twister */

/* Loopback mode */
#define MII_SPD_1000        0x00
#define MII_SPD_100         0x01
#define MII_SPD_10          0x02

/*
 * Company ID
 */
#define CID_REV_ID_MASK_OFF 0xFFFFFFF0UL    /* the last 4-bit is revision id, */
/*                                             we don't care it */

#define CID_CICADA_CIS8201    0x000FC410UL    /* OUI = 00-03-F1 , 0x000F C410 */
#define CID_CICADA_CIS3216I   0x000FC610UL    /* OUI = 00-03-?? , 0x000F C610 */
#define CID_CICADA_CIS3216I64 0x000FC600UL    /* OUI = 00-03-?? , 0x000F C600 */
#define CID_MARVELL_1000      0x01410C50UL    /* OUI = 00-50-43 , 0x0141 0C50 */
#define CID_MARVELL_1000S     0x01410C40UL    /* OUI = 00-50-43, */
#define CID_ICPLAUS_IP1001    0x02430D90UL

#define CB_MAX_COUNT_AUTO_COMPLETE      (0x1244)
/*                                      AUTO-NEGO complete, polling time out count */
/*                                      about 6 sec. */

/*---------------------  Export Types  ------------------------------*/
typedef struct tagSMiiCICADA {
	WORD    w100SER;                    /* 0x10 (WORD) 100BASE-TX Status Extension */
	WORD    w1000SER2;                  /* 0x11 (WORD) 1000BASE-T Status Extension #2 */
	WORD    wBCR;                       /* 0x12 (WORD) Bypass Control */
	WORD    wRECR;                      /* 0x13 (WORD) Receive Error Counter */
	WORD    wFCSC;                      /* 0x14 (WORD) False Carrier Sense Counter */
	WORD    wDCC;                       /* 0x15 (WORD) Disconnect Counter */
	WORD    w10CSR;                     /* 0x16 (WORD) 10BASE-T Control & Status */
	WORD    wEPHYCR1;                   /* 0x17 (WORD) Extended PHY Control #1 */
	WORD    wEPHYCR2;                   /* 0x18 (WORD) Extended PHY Control #2 */
	WORD    wIMR;                       /* 0x19 (WORD) Interrupt Mask */
	WORD    wISR;                       /* 0x1A (WORD) Interrupt Status */
	WORD    wPLEDCR;                    /* 0x1B (WORD) Parallel LED Control */
	WORD    wAUXCSR;                    /* 0x1C (WORD) Auxiliary Control & Status */
	WORD    wDSSR;                      /* 0x1D (WORD) Delay Skew Status */
	WORD    wReserved[2];
} SMiiCICADA;

typedef struct tagSMiiMARVELL {
	WORD    wPHYCR;                     /* 0x10 (WORD) PHY Specific Control */
	WORD    wPHYSR;                     /* 0x11 (WORD) PHY Specific Status */
	WORD    wIMR;                       /* 0x12 (WORD) Interrupt Enable */
	WORD    wISR;                       /* 0x13 (WORD) Interrupt Status */
	WORD    wEPHYCR;                    /* 0x14 (WORD) Extended PHY Specific Control */
	WORD    wRECR;                      /* 0x15 (WORD) Receive Error Counter */
	WORD    wReserved1[2];
	WORD    wPLEDCR;                    /* 0x18 (WORD) Parallel LED Control */
	WORD    wReserved2[7];
} SMiiMARVELL;

/*
 * MII registers
 */
typedef struct tagSMiiReg {
	WORD    wBMCR;                      /* 0x00 (WORD) Basic Mode Control Register */
	WORD    wBMSR;                      /* 0x01 (WORD) Basic Mode Status Register */
	WORD    wPHYIDR1;                   /* 0x02 (WORD) PHY Identifier Register #1 */
	WORD    wPHYIDR2;                   /* 0x03 (WORD) PHY Identifier Register #2 */
	WORD    wANAR;                      /* 0x04 (WORD) Auto-negotiation Advertisement Register */
	WORD    wANLPAR;                    /* 0x05 (WORD) Auto-negotiation Link Partner Ability Register */
	WORD    wANER;                      /* 0x06 (WORD) Auto-negotiation Expansion Register */
	WORD    wANNPTR;                    /* 0x07 (WORD) Auto-negotiation Next Page Transmit Register */
	WORD    wANLPNPR;                   /* 0x08 (WORD) Auto-negotiation Link Partner Next Page Register */
	WORD    w1000CR;                    /* 0x09 (WORD) 1000BASE-T Control Register */
	WORD    w1000SR;                    /* 0x0A (WORD) 1000BASE-T Status Register */
	WORD    awReserved1[4];
	WORD    w1000SER;                   /* 0x0F (WORD) 1000BASE-T Status Extension Register */

	union   {                           /* 0x10 ~ 0x1F Vendor Specific */
		SMiiCICADA    sMiiCicada;
		SMiiMARVELL   sMiiMarvell;
	};
} SMiiReg, DEF * PSMiiReg;

/*---------------------  Export Macros ------------------------------*/

#define MIIvRegBitsOn(dwIoBase, byRevId, byMiiAddr, wBits)                		\
do {                                                                      		\
	WORD wOrgData;                                                            	\
	GMIIbReadEmbeded(dwIoBase, byRevId, byMiiAddr, &wOrgData);                	\
	GMIIbWriteEmbeded(dwIoBase, byRevId, byMiiAddr, (WORD)(wOrgData | wBits));	\
} while (0)

#define MIIvRegBitsOff(dwIoBase, byRevId, byMiiAddr, wBits)                    		\
do {                                                                           		\
		WORD wOrgData;                                                               	\
		GMIIbReadEmbeded(dwIoBase, byRevId, byMiiAddr, &wOrgData);                   	\
		GMIIbWriteEmbeded(dwIoBase, byRevId, byMiiAddr, (WORD)(wOrgData & (~wBits)));	\
} while (0)

#define MIIvReadPhyCmrId(dwIoBase, byRevId, pdwPhyCmrId)                        		\
do {                                                                            		\
		GMIIbReadEmbeded(dwIoBase, byRevId, MII_REG_PHYID2, (PWORD)pdwPhyCmrId);      	\
		GMIIbReadEmbeded(dwIoBase, byRevId, MII_REG_PHYID1, ((PWORD)pdwPhyCmrId) + 1);	\
} while (0)

#define MIIvSetAutoNegotiationOn(dwIoBase, byRevId)                     		\
do {                                                                    		\
	MIIvRegBitsOn(dwIoBase, byRevId, MII_REG_BMCR, BMCR_AUTO | BMCR_REAUTO);	\
} while (0)

#define MIIvSetAutoNegotiationOff(dwIoBase, byRevId)       		\
do {                                                       		\
	MIIvRegBitsOff(dwIoBase, byRevId, MII_REG_BMCR, BMCR_AUTO);	\
} while (0)

#define MIIvSetResetOn(dwIoBase, byRevId)                  		\
do {                                                       		\
	MIIvRegBitsOn(dwIoBase, byRevId, MII_REG_BMCR, BMCR_RESET);	\
} while (0)

/*---------------------  Export Classes  ----------------------------*/

/*---------------------  Export Variables  --------------------------*/

/*---------------------  Export Functions  --------------------------*/
#ifdef __cplusplus
extern "C" {                            /* Assume C declarations for C++ */
#endif /* __cplusplus */

BOOL GMIIbReadEmbeded(DWORD dwIoBase, BYTE byRevId, BYTE byMiiAddr, PWORD pwData);
BOOL GMIIbWriteEmbeded(DWORD dwIoBase, BYTE byRevId, BYTE byMiiAddr, WORD wData);

BOOL GMIIbIsRegBitsOn(DWORD dwIoBase, BYTE byRevId, BYTE byMiiAddr, WORD wTestBits);
BOOL GMIIbIsRegBitsOff(DWORD dwIoBase, BYTE byRevId, BYTE byMiiAddr, WORD wTestBits);

BOOL GMIIbReadAllRegs(DWORD dwIoBase, BYTE byRevId, PSMiiReg pMiiRegs);

VOID GMIIvWaitForNwayCompleted(DWORD dwIoBase, BYTE byRevId);

VOID GMIIvSetLoopbackMode(DWORD dwIoBase, BYTE byRevId, BYTE byLoopbackMode);
VOID GMIIvSetDuplexMode(DWORD dwIoBase, BYTE byRevId, BOOL bFullDuplexOn);
VOID GMIIvSetSpeedMode(DWORD dwIoBase, BYTE byRevId, BYTE bySpeed);

BOOL GMIIbIsAutoNegotiationOn(DWORD dwIoBase, BYTE byRevId);
/* BOOL MIIbIsInFullDuplexMode(DWORD dwIoBase, BYTE byRevId); */

VOID GMIIvScanAllPhy(DWORD dwIoBase, BYTE byRevId, PBYTE pbyPhyId, PUINT puTotalPhyNum);
VOID GMIIvSetActiveForcedPhy(DWORD dwIoBase, BYTE byRevId, PBYTE pbyPhyId, UINT uTotalPhyNum, BYTE byPhyId);
BOOL GMIIbSetActiveLinkedPhy(DWORD dwIoBase, BYTE byRevId, PBYTE pbyPhyId, UINT uTotalPhyNum);

BOOL GMIIbSafeSoftwareReset(DWORD dwIoBase, BYTE byRevId);
VOID GMIIvInitialize(DWORD dwIoBase, BYTE byRevId, DWORD dwPhyCmrId);

#ifdef __cplusplus
}                                       /* End of extern "C" { */
#endif /* __cplusplus */

#endif /* __MII_H__ */
