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


#ifndef __MAC_H__
#define __MAC_H__

#if !defined(__TTYPE_H__)
#include "ttype.h"
#endif
#if !defined(__TMACRO_H__)
#include "tmacro.h"
#endif
#if !defined(__UPC_H__)
#include "upc.h"
#endif

#include "adapter.h"



/*---------------------  Export Definitions -------------------------*/
/*
 * Registers in the MAC
 */
#define MAC_REG_PAR         0x00        /* physical address */
#define MAC_REG_RCR         0x06
#define MAC_REG_TCR         0x07
#define MAC_REG_CR0_SET     0x08
#define MAC_REG_CR1_SET     0x09
#define MAC_REG_CR2_SET     0x0A
#define MAC_REG_CR3_SET     0x0B
#define MAC_REG_CR0_CLR     0x0C
#define MAC_REG_CR1_CLR     0x0D
#define MAC_REG_CR2_CLR     0x0E
#define MAC_REG_CR3_CLR     0x0F
#define MAC_REG_MAR         0x10
#define MAC_REG_CAM         0x10
#define MAC_REG_DEC_BASE_HI 0x18
#define MAC_REG_DBF_BASE_HI 0x1C
#define MAC_REG_ISR_CTL     0x20
#define MAC_REG_ISR_HOTMR   0x20
#define MAC_REG_ISR_TSUPTHR 0x20
#define MAC_REG_ISR_RSUPTHR 0x20
#define MAC_REG_ISR_CTL1    0x21
#define MAC_REG_TXE_SR      0x22
#define MAC_REG_RXE_SR      0x23
#define MAC_REG_ISR         0x24
#define MAC_REG_ISR0        0x24
#define MAC_REG_ISR1        0x25
#define MAC_REG_ISR2        0x26
#define MAC_REG_ISR3        0x27
#define MAC_REG_IMR         0x28
#define MAC_REG_IMR0        0x28
#define MAC_REG_IMR1        0x29
#define MAC_REG_IMR2        0x2A
#define MAC_REG_IMR3        0x2B
#define MAC_REG_TDCSR_SET   0x30
#define MAC_REG_RDCSR_SET   0x32
#define MAC_REG_TDCSR_CLR   0x34
#define MAC_REG_RDCSR_CLR   0x36
#define MAC_REG_RDBASE_LO   0x38
#define MAC_REG_RDINDX      0x3C
#define MAC_REG_TQETMR      0x3D        /* for vt3216 */
#define MAC_REG_RQETMR      0x3E        /* for vt3216 */
#define MAC_REG_TDBASE_LO   0x40
#define MAC_REG_RDCSIZE     0x50
#define MAC_REG_TDCSIZE     0x52
#define MAC_REG_TDINDX      0x54
#define MAC_REG_TDIDX0      0x54
#define MAC_REG_TDIDX1      0x56
#define MAC_REG_TDIDX2      0x58
#define MAC_REG_TDIDX3      0x5A
#define MAC_REG_PAUSE_TIMER 0x5C
#define MAC_REG_RBRDU       0x5E
#define MAC_REG_FIFO_TEST0  0x60
#define MAC_REG_FIFO_TEST1  0x64
#define MAC_REG_CAMADDR     0x68
#define MAC_REG_CAMCR       0x69
#define MAC_REG_GFTEST      0x6A
#define MAC_REG_FTSTCMD     0x6B
#define MAC_REG_MIICFG      0x6C
#define MAC_REG_MIISR       0x6D
#define MAC_REG_PHYSR0      0x6E
#define MAC_REG_PHYSR1      0x6F
#define MAC_REG_MIICR       0x70
#define MAC_REG_MIIADR      0x71
#define MAC_REG_MIIDATA     0x72
#define MAC_REG_SOFT_TIMER0 0x74
#define MAC_REG_SOFT_TIMER1 0x76
#define MAC_REG_CFGA        0x78
#define MAC_REG_CFGB        0x79
#define MAC_REG_CFGC        0x7A
#define MAC_REG_CFGD        0x7B
#define MAC_REG_DCFG0       0x7C
#define MAC_REG_DCFG1       0x7D
#define MAC_REG_MCFG0       0x7E
#define MAC_REG_MCFG1       0x7F

#define MAC_REG_TBIST       0x80
#define MAC_REG_RBIST       0x81
#define MAC_REG_BISTCMD     0x80    /* for vt3216/vt3286 */
#define MAC_REG_BISTSR      0x81    /* for vt3216/vt3286 */
#define MAC_REG_PMCC        0x82
#define MAC_REG_STICKHW     0x83
#define MAC_REG_MIBCR       0x84
#define MAC_REG_EERSV       0x85
#define MAC_REG_REVID       0x86
#define MAC_REG_MIBREAD     0x88
#define MAC_REG_BPMA        0x8C
#define MAC_REG_SPIMA       0x8C
#define MAC_REG_EEWR_DATA   0x8C
#define MAC_REG_SPIMA_HI    0x8E
#define MAC_REG_BPMD_WR     0x8F
#define MAC_REG_SPID_WR     0x8F
#define MAC_REG_BPCMD       0x90
#define MAC_REG_SPICMD      0x90
#define MAC_REG_BPMD_RD     0x91
#define MAC_REG_SPID_RD     0x91
#define MAC_REG_EECHKSUM    0x92
#define MAC_REG_EECSR       0x93
#define MAC_REG_EERD_DATA   0x94
#define MAC_REG_EADDR       0x96
#define MAC_REG_EMBCMD      0x97
#define MAC_REG_JMPSR0      0x98
#define MAC_REG_JMPSR1      0x99
#define MAC_REG_JMPSR2      0x9A
#define MAC_REG_JMPSR3      0x9B
#define MAC_REG_CHIPGSR     0x9C
#define MAC_REG_TESTCFG     0x9D
#define MAC_REG_DEBUG       0x9E
#define MAC_REG_CHIPGCR     0x9F
#define MAC_REG_WOLCR0_SET  0xA0
#define MAC_REG_WOLCR1_SET  0xA1
#define MAC_REG_PWCFG_SET   0xA2
#define MAC_REG_WOLCFG_SET  0xA3
#define MAC_REG_WOLCR0_CLR  0xA4
#define MAC_REG_WOLCR1_CLR  0xA5
#define MAC_REG_PWCFG_CLR   0xA6
#define MAC_REG_WOLCFG_CLR  0xA7
#define MAC_REG_WOLSR0_SET  0xA8
#define MAC_REG_WOLSR1_SET  0xA9
#define MAC_REG_WOLSR0_CLR  0xAC
#define MAC_REG_WOLSR1_CLR  0xAD
#define MAC_REG_PATRN_CRC0  0xB0
#define MAC_REG_PATRN_CRC1  0xB2
#define MAC_REG_PATRN_CRC2  0xB4
#define MAC_REG_PATRN_CRC3  0xB6
#define MAC_REG_PATRN_CRC4  0xB8
#define MAC_REG_PATRN_CRC5  0xBA
#define MAC_REG_PATRN_CRC6  0xBC
#define MAC_REG_PATRN_CRC7  0xBE
#define MAC_REG_BYTEMSK0_0  0xC0
#define MAC_REG_BYTEMSK0_1  0xC4
#define MAC_REG_BYTEMSK0_2  0xC8
#define MAC_REG_BYTEMSK0_3  0xCC
#define MAC_REG_BYTEMSK1_0  0xD0
#define MAC_REG_BYTEMSK1_1  0xD4
#define MAC_REG_BYTEMSK1_2  0xD8
#define MAC_REG_BYTEMSK1_3  0xDC
#define MAC_REG_BYTEMSK2_0  0xE0
#define MAC_REG_BYTEMSK2_1  0xE4
#define MAC_REG_BYTEMSK2_2  0xE8
#define MAC_REG_BYTEMSK2_3  0xEC
#define MAC_REG_BYTEMSK3_0  0xF0
#define MAC_REG_BYTEMSK3_1  0xF4
#define MAC_REG_BYTEMSK3_2  0xF8
#define MAC_REG_BYTEMSK3_3  0xFC


/*
 * Bits in the RCR register
 */
#define RCR_AS              0x80
#define RCR_AP              0x40
#define RCR_AL              0x20
#define RCR_PROM            0x10
#define RCR_AB              0x08
#define RCR_AM              0x04
#define RCR_AR              0x02
#define RCR_SEP             0x01

/*
 * Bits in the TCR register
 */
#define TCR_TB2BDIS         0x80
#define TCR_COLTMC1         0x08
#define TCR_COLTMC0         0x04
#define TCR_LB1             0x02        /* loopback[1] */
#define TCR_LB0             0x01        /* loopback[0] */

/*
 * Bits in the CR0 register
 */
#define CR0_TXON            0x08
#define CR0_RXON            0x04
#define CR0_STOP            0x02        /* stop MAC, default = 1 */
#define CR0_STRT            0x01        /* start MAC */

/*
 * Bits in the CR1 register
 */
#define CR1_SFRST           0x80        /* software reset */
#define CR1_TM1EN           0x40
#define CR1_TM0EN           0x20
#define CR1_DPOLL           0x08        /* disable rx/tx auto polling */
#define CR1_DISAU           0x01

/*
 * Bits in the CR2 register
 */
#define CR2_XONEN           0x80
#define CR2_FDXTFCEN        0x40        /* full-duplex TX flow control enable */
#define CR2_FDXRFCEN        0x20        /* full-duplex RX flow control enable */
#define CR2_HDXFCEN         0x10        /* half-duplex flow control enable */
#define CR2_XHITH1          0x08        /* TX XON high threshold 1 */
#define CR2_XHITH0          0x04        /* TX XON high threshold 0 */
#define CR2_XLTH1           0x02        /* TX pause frame low threshold 1 */
#define CR2_XLTH0           0x01        /* TX pause frame low threshold 0 */

/*
 * Bits in the CR3 register
 */
#define CR3_GSPRST          0x80
#define CR3_FORSRST         0x40
#define CR3_FPHYRST         0x20
#define CR3_DIAG            0x10
#define CR3_INTPCTL         0x04
#define CR3_GINTMSK1        0x02
#define CR3_GINTMSK0        0x01


/*
 * Bits in the ISR_CTL1 register
 */
#define ISRCTL1_UDPINT          0x80
#define ISRCTL1_TSUPDIS         0x40
#define ISRCTL1_RSUPDIS         0x20
#define ISRCTL1_PMSK1           0x10
#define ISRCTL1_PMSK0           0x08
#define ISRCTL1_INTPD           0x04
#define ISRCTL1_HCRLD           0x02
#define ISRCTL1_SCRLD           0x01

/*
 * Bits in the TXE_SR register
 */
#define TXESR_TFDBS           0x08
#define TXESR_TDWBS           0x04
#define TXESR_TDRBS           0x02
#define TXESR_TDSTR           0x01

/*
 * Bits in the RXE_SR register
 */
#define RXESR_RFDBS           0x08
#define RXESR_RDWBS           0x04
#define RXESR_RDRBS           0x02
#define RXESR_RDSTR           0x01

/*
 * Bits in the ISR register
 */
#define ISR_ISR3            0x80000000UL
#define ISR_ISR2            0x40000000UL
#define ISR_ISR1            0x20000000UL
#define ISR_ISR0            0x10000000UL
#define ISR_TXSTLI          0x02000000UL
#define ISR_RXSTLI          0x01000000UL
#define ISR_HFLD            0x00800000UL
#define ISR_UDPI            0x00400000UL
#define ISR_MIBFI           0x00200000UL
#define ISR_SHDNI           0x00100000UL
#define ISR_PHYI            0x00080000UL
#define ISR_PWEI            0x00040000UL
#define ISR_TMR1I           0x00020000UL
#define ISR_TMR0I           0x00010000UL
#define ISR_SRCI            0x00008000UL
#define ISR_LSTPEI          0x00004000UL
#define ISR_LSTEI           0x00002000UL
#define ISR_OVFI            0x00001000UL
#define ISR_FLONI           0x00000800UL
#define ISR_RACEI           0x00000400UL
#define ISR_TXWB1I          0x00000200UL
#define ISR_TXWB0I          0x00000100UL
#define ISR_PTX3I           0x00000080UL
#define ISR_PTX2I           0x00000040UL
#define ISR_PTX1I           0x00000020UL
#define ISR_PTX0I           0x00000010UL
#define ISR_PTXI            0x00000008UL
#define ISR_PRXI            0x00000004UL
#define ISR_PPTXI           0x00000002UL
#define ISR_PPRXI           0x00000001UL

/*
 * Bits in the IMR register
 */
#define IMR_UDPIM           0x00400000UL
#define IMR_MIBFIM          0x00200000UL
#define IMR_SHDNIM          0x00100000UL
#define IMR_PHYIM           0x00080000UL
#define IMR_PWEIM           0x00040000UL
#define IMR_TMR1IM          0x00020000UL
#define IMR_TMR0IM          0x00010000UL
#define IMR_SRCIM           0x00008000UL
#define IMR_LSTPEIM         0x00004000UL
#define IMR_LSTEIM          0x00002000UL
#define IMR_OVFIM           0x00001000UL
#define IMR_FLONIM          0x00000800UL
#define IMR_RACEIM          0x00000400UL
#define IMR_TXWB1IM         0x00000200UL
#define IMR_TXWB0IM         0x00000100UL
#define IMR_PTX3IM          0x00000080UL
#define IMR_PTX2IM          0x00000040UL
#define IMR_PTX1IM          0x00000020UL
#define IMR_PTX0IM          0x00000010UL
#define IMR_PTXIM           0x00000008UL
#define IMR_PRXIM           0x00000004UL
#define IMR_PPTXIM          0x00000002UL
#define IMR_PPRXIM          0x00000001UL



/*
 * Bits in the TDCSR0/1, RDCSR0 register
 */
#define TRDCSR_DEAD           0x0008
#define TRDCSR_WAK            0x0004
#define TRDCSR_ACT            0x0002
#define TRDCSR_RUN            0x0001

/*
 * Bits in the CAMADDR register
 */
#define CAMADDR_CAMEN       0x80
#define CAMADDR_VCAMSL      0x40


/*
 * Bits in the CAMCR register
 */
#define CAMCR_PS1           0x80
#define CAMCR_PS0           0x40
#define CAMCR_AITRPKT       0x20
#define CAMCR_AITR16        0x10
#define CAMCR_CAMRD         0x08
#define CAMCR_CAMWR         0x04

/*
 * Bits in the GFTEST register
 */
#define GFTEST_FFSTAS       0x10
#define GFTEST_FIFOSEL      0x04
#define GFTEST_FIFODUMP     0x02
#define GFTEST_FIFOTEST     0x01

/*
 * Bits in the FTSTCMD register
 */
#define FTSTCMD_RRSTPTR     0x10
#define FTSTCMD_TRSTPTR     0x10
#define FTSTCMD_RXPUSH      0x08
#define FTSTCMD_TXPOP       0x08
#define FTSTCMD_RXBAKOUT    0x04
#define FTSTCMD_TXCOLABT    0x04
#define FTSTCMD_RXEOF       0x02
#define FTSTCMD_TXDONE      0x02
#define FTSTCMD_RXSOF       0x01
#define FTSTCMD_TXRETRY     0x01

/*
 * Bits in the MIICFG register
 */
#define MIICFG_MPO1         0x80
#define MIICFG_MPO0         0x40
#define MIICFG_MFDC         0x20

/*
 * Bits in the MIISR register
 */
#define MIISR_MIIDL         0x80

/*
 * Bits in the PHYSR0 (0x6E) register
 */
#define PHYSR0_PHYRST       0x80
#define PHYSR0_LINKGD       0x40
#define PHYSR0_FDPX         0x10
#define PHYSR0_SPDG         0x08
#define PHYSR0_SPD10        0x04
#define PHYSR0_RXFLC        0x02
#define PHYSR0_TXFLC        0x01

/*
 * Bits in the PHYSR1 (0x6F) register
 */
#define PHYSR1_PHYTBI       0x01

/*
 * Bits in the MIICR (0x70) register
 */
#define MIICR_MAUTO         0x80
#define MIICR_RCMD          0x40
#define MIICR_WCMD          0x20
#define MIICR_MDPM          0x10
#define MIICR_MOUT          0x08
#define MIICR_MDO           0x04
#define MIICR_MDI           0x02
#define MIICR_MDC           0x01

/*
 * Bits in the MIIADR (0x71) register
 */
#define MIIADR_SWMPL        0x80

/*
 * Bits in the CFGA (0x78) register
 */
#define CFGA_PHYLEDS1       0x20        /* for vt3216 */
#define CFGA_PHYLEDS0       0x10        /* for vt3216 */
#define CFGA_PMHCTG         0x08
#define CFGA_GPIO1PD        0x04
#define CFGA_ABSHDN         0x02
#define CFGA_PACPI          0x01

/*
 * Bits in the CFGB (0x79) register
 */
#define CFGB_GTCKOPT        0x80
#define CFGB_MIIOPT         0x40
#define CFGB_CRSEOPT        0x20
#define CFGB_OFSET          0x10
#define CFGB_CRANDOM        0x08
#define CFGB_CAP            0x04
#define CFGB_MBA            0x02
#define CFGB_BAKOPT         0x01

/*
 * Bits in the CFGC (0x7A) register
 */
#define CFGC_EELOAD         0x80
#define CFGC_BROPT          0x40
#define CFGC_DLYEN          0x20
#define CFGC_DTSEL          0x10
#define CFGC_BTSEL          0x08
#define CFGC_BPS2           0x04        /* bootrom select[2] */
#define CFGC_BPS1           0x02        /* bootrom select[1] */
#define CFGC_BPS0           0x01        /* bootrom select[0] */

/*
 * Bits in the CFGD (0x7B) register
 */
#define CFGD_IODIS          0x80
#define CFGD_MSLVDACEN      0x40
#define CFGD_CFGDACEN       0x20
#define CFGD_PCI64EN        0x10
#define CFGD_HTMRL4         0x08

/*
 * Bits in the DCFG1 (0x7D) register
 */
#define DCFG1_XMWI          0x80
#define DCFG1_XMRM          0x40
#define DCFG1_XMRL          0x20
#define DCFG1_PERDIS        0x10
#define DCFG1_MRWAIT        0x04
#define DCFG1_MWWAIT        0x02
#define DCFG1_LATMEN        0x01

/*
 * Bits in the MCFG0 (0x7E) register
 */
#define MCFG0_RXARB         0x80
#define MCFG0_RFT1          0x20
#define MCFG0_RFT0          0x10
#define MCFG0_LOWTHOPT      0x08
#define MCFG0_PQEN          0x04
#define MCFG0_RTGOPT        0x02
#define MCFG0_VIDFR         0x01

/*
 * Bits in the MCFG1 (0x7F) register
 */
#define MCFG1_TXARB         0x80
#define MCFG1_TXQBK1        0x08
#define MCFG1_TXQBK0        0x04
#define MCFG1_TXQNOBK       0x02
#define MCFG1_SNAPOPT       0x01

/*
 * Bits in the TBIST (0x80) register
 * Bist in the RBIST (0x81) register
 */
#define BIST_SR             0x80
#define BIST_GO             0x40
#define BIST_ER             0x10
#define BIST_PT1            0x08
#define BIST_PT0            0x04
#define BIST_TMD            0x02
#define BIST_EN             0x01

/*
 * Bits in the BISTCMD (0x80) register, for vt3216/vt3286
 */
#define BISTCMD_EADDRX      0x80
#define BISTCMD_BITMAP      0x80        /* for vt3286 */
#define BISTCMD_EADDTX      0x40
#define BISTCMD_RFBISTGO    0x40        /* for vt3286 */
#define BISTCMD_EFSM        0x20
#define BISTCMD_TFBISTGO    0x20        /* for vt3286 */
#define BISTCMD_BSERRX      0x10
#define BISTCMD_BSERTX      0x08
#define BISTCMD_STERRFZ     0x04
#define BISTCMD_STERREN     0x02
#define BISTCMD_BISTEN      0x01

/*
 * Bits in the BISTSR  (0x81) register, for vt3216/vt3286
 */
#define BISTSR_ERCNT3       0x80
#define BISTSR_ERCNT2       0x40
#define BISTSR_ERCNT1       0x20
#define BISTSR_ERCNT0       0x10
#define BISTSR_BRXL_GO      0x08
#define BISTSR_TFOK         0x08        /* for vt3286 */
#define BISTSR_BRXH_GO      0x04
#define BISTSR_RFOK         0x04        /* for vt3286 */
#define BISTSR_BTX_GO       0x02
#define BISTSR_TFDONE       0x02        /* for vt3286 */
#define BISTSR_MBISTDONE    0x01
#define BISTSR_RFDONE       0x01        /* for vt3286 */

/*
 * Bits in the PMCC  register
 */
#define PMCC_DSI            0x80
#define PMCC_D2_DIS         0x40
#define PMCC_D1_DIS         0x20
#define PMCC_D3C_EN         0x10
#define PMCC_D3H_EN         0x08
#define PMCC_D2_EN          0x04
#define PMCC_D1_EN          0x02
#define PMCC_D0_EN          0x01

/*
 * Bits in STICKHW
 */
#define STICKHW_SWPTAG      0x10
#define STICKHW_WOLSR       0x08
#define STICKHW_WOLEN       0x04
#define STICKHW_DS1         0x02  /* R/W by software/cfg cycle */
#define STICKHW_DS0         0x01  /* suspend well DS write port */

/*
 * Bits in the MIBCR register
 */
#define MIBCR_MIBISTOK      0x80
#define MIBCR_MIBISTGO      0x40
#define MIBCR_MIBINC        0x20
#define MIBCR_MIBHI         0x10
#define MIBCR_MIBFRZ        0x08
#define MIBCR_MIBFLSH       0x04
#define MIBCR_MPTRINI       0x02
#define MIBCR_MIBCLR        0x01

/*
 * Bits in the EERSV register
 */
#define EERSV_BOOT_RPL       ((BYTE) 0x01) /* Boot method selection for VT3119 */

#define EERSV_BOOT_MASK      ((BYTE) 0x06)
#define EERSV_BOOT_INT19     ((BYTE) 0x00)
#define EERSV_BOOT_INT18     ((BYTE) 0x02)
#define EERSV_BOOT_LOCAL     ((BYTE) 0x04)
#define EERSV_BOOT_BEV       ((BYTE) 0x06)


/*
 * Bits in BPCMD
 */
#define BPCMD_BPDNE         0x80
#define BPCMD_EBPWR         0x02
#define BPCMD_EBPRD         0x01

/*
 * Bits in SPICMD
 */
#define SPICMD_BPDNE        0x80
#define SPICMD_EWEN         0x08
#define SPICMD_EWDIS        0x04
#define SPICMD_EBPWR        0x02
#define SPICMD_EBPRD        0x01

/*
 * Bits in the EECSR register
 */
#define EECSR_SPIDPM        0x80        /* SPI direct programming */
#define EECSR_EMBP          0x40        /* eeprom embeded programming */
#define EECSR_RELOAD        0x20        /* eeprom content reload */
#define EECSR_DPM           0x10        /* eeprom direct programming */
#define EECSR_ECS           0x08        /* eeprom/SPI CS pin */
#define EECSR_ECK           0x04        /* eeprom/SPI CK pin */
#define EECSR_EDI           0x02        /* eeprom/SPI DI pin */
#define EECSR_EDO           0x01        /* eeprom/SPI DO pin */

/*
 * Bits in the EMBCMD register
 */
#define EMBCMD_EDONE        0x80
#define EMBCMD_EWDIS        0x08
#define EMBCMD_EWEN         0x04
#define EMBCMD_EWR          0x02
#define EMBCMD_ERD          0x01

/*
 * Bits in the JMPSR1 register for VT3286
 */
#define JMPSR1_J_VEESEL     0x04        /* Strapping status of virtual eeprom */
#define JMPSR1_J_SPI        0x01        /* Strapping status of SPI and Legacy BROM */

/*
 * Bits in the JMPSR3 register for VT3286
 */
#define JMPSR3_BOND_QFP     0x20        /* Bonding option, 0:1 = QFN-64(9*9):PQFP-128(14*20) */

/*
 * Bits in CHIPGCR register
 */
#define CHIPGCR_FCGMII         0x80
#define CHIPGCR_FCFDX          0x40
#define CHIPGCR_FCRESV         0x20
#define CHIPGCR_FCMODE         0x10
#define CHIPGCR_LPSOPT         0x08
#define CHIPGCR_TM1US          0x04
#define CHIPGCR_TM0US          0x02
#define CHIPGCR_PHYINTEN       0x01


/*
 * Bits in WOLCR0
 */
#define WOLCR0_MSWOLEN7          0x80    /* enable pattern match filtering */
#define WOLCR0_MSWOLEN6          0x40
#define WOLCR0_MSWOLEN5          0x20
#define WOLCR0_MSWOLEN4          0x10
#define WOLCR0_MSWOLEN3          0x08
#define WOLCR0_MSWOLEN2          0x04
#define WOLCR0_MSWOLEN1          0x02
#define WOLCR0_MSWOLEN0          0x01


/*
 * Bits in WOLCR1
 */
#define WOLCR1_LINKOFF_EN      0x08    /* link off detected enable */
#define WOLCR1_LINKON_EN       0x04    /* link on detected enable */
#define WOLCR1_MAGIC_EN        0x02    /* magic packet filter enable */
#define WOLCR1_UNICAST_EN      0x01    /* unicast filter enable */


/*
 * Bits in PWCFG
 */
#define PWCFG_PHYPWOPT          0x80    /* internal MII I/F timing */
#define PWCFG_PCISTICK          0x40    /* PCI sticky R/W enable */
#define PWCFG_WOLTYPE           0x20    /* pulse(1) or button (0) */
#define PWCFG_LEGCY_WOL         0x10
#define PWCFG_PMCSR_PME_SR      0x08
#define PWCFG_PMCSR_PME_EN      0x04    /* control by PCISTICK */
#define PWCFG_LEGACY_WOLSR      0x02    /* Legacy WOL_SR shadow */
#define PWCFG_LEGACY_WOLEN      0x01    /* Legacy WOL_EN shadow */

/*
 * Bits in WOLCFG
 */
#define WOLCFG_PMEOVR           0x80    /* for legacy use, force PMEEN always */
#define WOLCFG_SAM              0x20    /* accept multicast case reset, default=0 */
#define WOLCFG_SAB              0x10    /* accept broadcast case reset, default=0 */
#define WOLCFG_SMIIACC          0x08    /* ?? */
#define WOLCFG_SGENWH           0x02
#define WOLCFG_PHYINTEN         0x01    /* 0:PHYINT trigger enable, 1:use internal MII */
/*                                         to report status change */

/*
 * Bits in WOLSR1
 */
#define WOLSR1_LINKOFF_INT      0x08
#define WOLSR1_LINKON_INT       0x04
#define WOLSR1_MAGIC_INT        0x02
#define WOLSR1_UNICAST_INT      0x01



/* Ethernet address filter type */
#define PKT_TYPE_NONE               0x0000  /* turn off receiver */
#define PKT_TYPE_DIRECTED           0x0001  /* obselete, directed address is always accepted */
#define PKT_TYPE_MULTICAST          0x0002
#define PKT_TYPE_ALL_MULTICAST      0x0004
#define PKT_TYPE_BROADCAST          0x0008
#define PKT_TYPE_PROMISCUOUS        0x0020
#define PKT_TYPE_LONG               0x2000  /* NOTE.... the definition of LONG is >2048 bytes in our chip */
#define PKT_TYPE_RUNT               0x4000
#define PKT_TYPE_ERROR              0x8000  /* accept error packets, e.g. CRC error */


/* Loopback mode */
#define MAC_LB_NONE         0x00
#define MAC_LB_INTERNAL     0x01
#define MAC_LB_EXTERNAL     0x02



/* enabled mask value of irq */
#if defined(_SIM)
#define IMR_MASK_VALUE      0x037BFFFFUL    /* initial value of IMR */
#else
#define IMR_MASK_VALUE      0x037BFFFFUL    /* initial value of IMR */
/*                                             ignore MIBFI,RACEI to */
/*                                             reduce intr. frequency */
/*                                             NOTE.... do not enable NoBuf int mask at NDIS driver */
/*                                             when (1) NoBuf -> RxThreshold = SF */
/*                                                  (2) OK    -> RxThreshold = original value */
#endif


/*
 * revision id
 */
#define REV_ID_VT3119_A0   0x00
#define REV_ID_VT3119_A1   0x01
#define REV_ID_VT3216_A0   0x10
#define REV_ID_VT3284_A0   0x20
#define REV_ID_VT3286_A1   0x80

/* wait time within loop */
#define CB_DELAY_LOOP_WAIT  10          /* 10ms */
#define CB_DELAY_MII_STABLE 660

/* max time out delay time */
#define W_MAX_TIMEOUT       0x0FFFU


#define CAMSIZE               64        /* CAM size, 0~63 */
#define VCAMSIZE              64        /* VCAM size, 0~63 */
#define MCAMSIZE              64        /* MCAM size, 0~63 */

/*---------------------  Export Types  ------------------------------*/
/*
 * MAC registers
 */
typedef struct tagSMacReg3119 {
	BYTE    abyPAR[6];                  /* 0x00 */
	BYTE    byRCR;
	BYTE    byTCR;

	BYTE    byCR0Set;                   /* 0x08 */
	BYTE    byCR1Set;
	BYTE    byCR2Set;
	BYTE    byCR3Set;

	BYTE    byCR0Clr;                   /* 0x0C */
	BYTE    byCR1Clr;
	BYTE    byCR2Clr;
	BYTE    byCR3Clr;

	BYTE    abyMARCAM[8];               /* 0x10 */

	DWORD   dwDecBaseHi;                /* 0x18 */
	WORD    wDbfBaseHi;                 /* 0x1C */
	WORD    wReserved_1E;

	BYTE    byISRCTL0;                  /* 0x20 */
	BYTE    byISRCTL1;
	BYTE    byTXESR;
	BYTE    byRXESR;

	DWORD   dwISR;                      /* 0x24 */
	DWORD   dwIMR;

	DWORD   dwTDStatusPort;             /* 0x2C */

	WORD    wTDCSRSet;                  /* 0x30 */
	BYTE    byRDCSR0Set;
	BYTE    byReserved_33;
	WORD    wTDCSRClr;
	BYTE    byRDCSR0Clr;
	BYTE    byReserved_37;

	DWORD   dwRDBase0Lo;                /* 0x38 */
	WORD    wRDIdx;                     /* 0x3C */
	WORD    wReserved_3E;

	DWORD   dwTDBase0Lo;                /* 0x40 */
	DWORD   dwTDBase1Lo;                /* 0x44 */
	DWORD   dwTDBase2Lo;                /* 0x48 */
	DWORD   dwTDBase3Lo;                /* 0x4C */

	WORD    wRDCSize;                   /* 0x50 */
	WORD    wTDCSize;                   /* 0x52 */
	WORD    wTDIdx0;                    /* 0x54 */
	WORD    wTDIdx1;                    /* 0x56 */
	WORD    wTDIdx2;                    /* 0x58 */
	WORD    wTDIdx3;                    /* 0x5A */
	WORD    wTxPauseTimer;              /* 0x5C */
	WORD    wRBRDU;                     /* 0x5E */

	DWORD   dwFIFOTest0;                /* 0x60 */
	DWORD   dwFIFOTest1;                /* 0x64 */

	BYTE    byCAMADDR;                  /* 0x68 */
	BYTE    byCAMCR;                    /* 0x69 */
	BYTE    byGFTEST;                   /* 0x6A */
	BYTE    byFTSTCMD;                  /* 0x6B */

	BYTE    byMIICFG;                   /* 0x6C */
	BYTE    byMIISR;
	BYTE    byPHYSR0;
	BYTE    byPHYSR1;
	BYTE    byMIICR;
	BYTE    byMIIADR;
	WORD    wMIIDATA;

	WORD    wSoftTimer0;                /* 0x74 */
	WORD    wSoftTimer1;

	BYTE    byCFGA;                     /* 0x78 */
	BYTE    byCFGB;
	BYTE    byCFGC;
	BYTE    byCFGD;

	BYTE    byDCFG0;                    /* 0x7C */
	BYTE    byDCFG1;
	BYTE    byMCFG0;
	BYTE    byMCFG1;

	BYTE    byTBIST;                    /* 0x80 */
	BYTE    byRBIST;
	BYTE    byPMCPORT;
	BYTE    bySTICKHW;

	BYTE    byMIBCR;                    /* 0x84 */
	BYTE    byReserved_85;
	BYTE    byRevID;
	BYTE    byPORSTS;

	DWORD   dwMIBRead;                  /* 0x88 */
	union {
		WORD    wBPMA;
		WORD    wEEWrData;
	};
	BYTE    byReserved_8E;
	BYTE    byBPMDWr;
	BYTE    byBPCMD;
	BYTE    byBPMDRd;

	BYTE    byEECHKSUM;                /* 0x92 */
	BYTE    byEECSR;

	WORD    wEERdData;                 /* 0x94 */
	BYTE    byEADDR;
	BYTE    byEMBCMD;


	BYTE    byJMPSR0;                  /* 0x98 */
	BYTE    byJMPSR1;
	BYTE    byJMPSR2;
	BYTE    byJMPSR3;
	BYTE    byCHIPGSR;                 /* 0x9C */
	BYTE    byTESTCFG;
	BYTE    byDEBUG;
	BYTE    byCHIPGCR;

	BYTE    byWOLCR0Set;               /* 0xA0 */
	BYTE    byWOLCR1Set;
	BYTE    byPWCFGSet;
	BYTE    byWOLCFGSet;

	BYTE    byWOLCR0Clr;               /* 0xA4 */
	BYTE    byWOLCR1Clr;
	BYTE    byPWCFGClr;
	BYTE    byWOLCFGClr;

	BYTE    byWOLSR0Set;               /* 0xA8 */
	BYTE    byWOLSR1Set;
	WORD    wReserved_AA;

	BYTE    byWOLSR0Clr;               /* 0xAC */
	BYTE    byWOLSR1Clr;
	WORD    wReserved_AE;

	WORD    wPatternCRC0;              /* 0xB0 */
	WORD    wPatternCRC1;
	WORD    wPatternCRC2;
	WORD    wPatternCRC3;
	WORD    wPatternCRC4;
	WORD    wPatternCRC5;
	WORD    wPatternCRC6;
	WORD    wPatternCRC7;

	DWORD   dwByteMask00;              /* 0xC0 */
	DWORD   dwByteMask01;
	DWORD   dwByteMask02;
	DWORD   dwByteMask03;
	DWORD   dwByteMask10;
	DWORD   dwByteMask11;
	DWORD   dwByteMask12;
	DWORD   dwByteMask13;
	DWORD   dwByteMask20;
	DWORD   dwByteMask21;
	DWORD   dwByteMask22;
	DWORD   dwByteMask23;
	DWORD   dwByteMask30;
	DWORD   dwByteMask31;
	DWORD   dwByteMask32;
	DWORD   dwByteMask33;

} SMacReg, SMacReg3119, DEF * PSMacReg, DEF * PSMacReg3119;




/*---------------------  Export Macros ------------------------------*/

#define MACvRegBitsOn(dwIoBase, byRegOfs, byBits)                		\
do {                                                                		\
	BYTE byData;                                                     	\
	VNSvInPortB(dwIoBase + byRegOfs, &byData);                       	\
	VNSvOutPortB(dwIoBase + byRegOfs, byData | (BYTE)(byBits));      	\
} while (0)


#define MACvRegBitsOff(dwIoBase, byRegOfs, byBits)                		\
do {                                                                 		\
	BYTE byData;                                                      	\
	VNSvInPortB(dwIoBase + byRegOfs, &byData);                        	\
	VNSvOutPortB(dwIoBase + byRegOfs, byData & (BYTE)~(byBits));      	\
} while (0)

/* set/get rx/tx common descriptor base hi[63:32] address */
#define MACvSetDescBaseHi32(dwIoBase, dwDescBaseHi)            			\
do {                                                               		\
	VNSvOutPortD(dwIoBase + MAC_REG_DEC_BASE_HI, dwDescBaseHi);     	\
} while (0)
#define MACvGetDescBaseHi32(dwIoBase, pdwDescBaseHi)               		\
do {                                                                  		\
	VNSvInPortD(dwIoBase + MAC_REG_DEC_BASE_HI, pdwDescBaseHi);        	\
} while (0)

/* set/get rx/tx common data buf base hi[63:48] address */
#define MACvSetDbfBaseHi16(dwIoBase, wDbfBaseHi)                  		\
do {                                                                 		\
	VNSvOutPortW(dwIoBase + MAC_REG_DBF_BASE_HI, wDbfBaseHi);         	\
} while (0)

#define MACvGetDbfBaseHi16(dwIoBase, pwDbfBaseHi)                		\
do {                                                                		\
	VNSvInPortW(dwIoBase + MAC_REG_DBF_BASE_HI, pwDbfBaseHi);        	\
} while (0)

/* set/get rx descriptor base low[31:6] address */
#define MACvSetRxDescBaseLo32(dwIoBase, dwRxDescBase)            		\
do {                                                                		\
	VNSvOutPortD(dwIoBase + MAC_REG_RDBASE_LO, dwRxDescBase);        	\
} while (0)

#define MACvGetRxDescBaseLo32(dwIoBase, pdwRxDescBase)             		\
do {                                                                  		\
	VNSvInPortD(dwIoBase + MAC_REG_RDBASE_LO, (PDWORD)pdwRxDescBase);  	\
} while (0)

/* set/get tx descriptor base low[31:6] address */
#define MACvSetTxDescBaseLo32(dwIoBase, dwTxDescBase, byTxQue)       		\
do {                                                                    		\
	VNSvOutPortD(dwIoBase + MAC_REG_TDBASE_LO + byTxQue*4, dwTxDescBase);	\
} while (0)

#define MACvGetTxDescBaseLo32(dwIoBase, pdwTxDescBase, byTxQue)              		\
do {                                                                            		\
	VNSvInPortD(dwIoBase + MAC_REG_TDBASE_LO + byTxQue*4, (PDWORD)pdwTxDescBase);	\
} while (0)

/* set/get rx queue index */
#define MACvSetRqIndex(dwIoBase, wRdIdx)                      		\
do {                                                             		\
	VNSvOutPortW(dwIoBase + MAC_REG_RDINDX, wRdIdx);              	\
} while (0)

#define MACvGetRqIndex(dwIoBase, pwRdIdx)                       		\
do {                                                               		\
	VNSvInPortW(dwIoBase + MAC_REG_RDINDX, (PWORD)pwRdIdx);      			\
} while (0)

/* set/get tx queue index */
/*
#define MACvSetTqIndex(dwIoBase, wTdIdx, byTxQue)                         \
{                                                                       \
    VNSvOutPortW(dwIoBase + MAC_REG_TDCSR_CLR + (byTxQue / 2), TRDCSR_RUN << ((byTxQue % 2)*4));\
    VNSvOutPortW(dwIoBase + MAC_REG_TDINDX + byTxQue*2, wTdIdx);          \
    VNSvOutPortW(dwIoBase + MAC_REG_TDCSR_SET + (byTxQue / 2), TRDCSR_RUN << ((byTxQue % 2)*4));\
}
*/
#define MACvGetTqIndex(dwIoBase, pwTdIdx, byTxQue)                        \
do {                                                                       \
	VNSvInPortW(dwIoBase + MAC_REG_TDINDX + byTxQue*2, (PWORD)pwTdIdx);   \
} while (0)

/* set/get rx queue size */
#define MACvSetRqSize(dwIoBase, wRdSize)                             \
do {                                                                    \
	VNSvOutPortW(dwIoBase + MAC_REG_RDCSIZE, wRdSize);               \
} while (0)

#define MACvGetRqSize(dwIoBase, pwRdSize)                            \
do {                                                                    \
	VNSvInPortW(dwIoBase + MAC_REG_RDCSIZE, (PWORD)pwRdSize);        \
} while (0)

/* set/get tx queue size */
#define MACvSetTqSize(dwIoBase, wTdSize)                             \
do {                                                                    \
	VNSvOutPortW(dwIoBase + MAC_REG_TDCSIZE, wTdSize);               \
} while (0)

#define MACvGetTqSize(dwIoBase, pwTdSize)                            \
do {                                                                    \
	VNSvInPortW(dwIoBase + MAC_REG_TDCSIZE, (PWORD)pwTdSize);        \
} while (0)

#define MACvReadEtherAddress(dwIoBase, pbyEtherAddr)               		\
do {                                                                  		\
	VNSvInPortB(dwIoBase + MAC_REG_PAR, (PBYTE)pbyEtherAddr);          	\
	VNSvInPortB(dwIoBase + MAC_REG_PAR + 1, (PBYTE)(pbyEtherAddr + 1));	\
	VNSvInPortB(dwIoBase + MAC_REG_PAR+2, (PBYTE)(pbyEtherAddr+2));    	\
	VNSvInPortB(dwIoBase + MAC_REG_PAR + 3, (PBYTE)(pbyEtherAddr + 3));	\
	VNSvInPortB(dwIoBase + MAC_REG_PAR+4, (PBYTE)(pbyEtherAddr+4));    	\
	VNSvInPortB(dwIoBase + MAC_REG_PAR + 5, (PBYTE)(pbyEtherAddr + 5));	\
} while (0)


#define MACvWriteEtherAddress(dwIoBase, pbyEtherAddr)                                          \
do {                                                                                              \
	VNSvOutPortD(dwIoBase + MAC_REG_PAR, *((PDWORD)pbyEtherAddr));                                 \
	VNSvOutPortW(dwIoBase + MAC_REG_PAR + sizeof(DWORD), *((PWORD)(pbyEtherAddr + sizeof(DWORD))));\
} while (0)


/* do not write 1 to ISR_UDPINT_SET */
#define MACvClearISR(dwIoBase, byRevId)                              \
do {                                                                    \
	MACvWriteISR(dwIoBase, 0x003FFFFFUL);                   \
} while (0)


#define MACvStart(dwIoBase)                                          \
do {                                                                    \
	VNSvOutPortB(dwIoBase + MAC_REG_CR0_CLR, CR0_STOP);              \
	VNSvOutPortB(dwIoBase + MAC_REG_CR0_SET, CR0_STRT);              \
	VNSvOutPortB(dwIoBase + MAC_REG_CR0_SET, CR0_RXON | CR0_TXON);  \
} while (0)

#define MACvRxOn(dwIoBase)                                  \
do {                                                           \
	VNSvOutPortB(dwIoBase + MAC_REG_CR0_CLR, CR0_STOP);     \
	VNSvOutPortB(dwIoBase + MAC_REG_CR0_SET, CR0_STRT);     \
	VNSvOutPortB(dwIoBase + MAC_REG_CR0_SET, CR0_RXON);     \
} while (0)

#define MACvRxOff(dwIoBase)                                 \
do {                                                           \
	VNSvOutPortB(dwIoBase + MAC_REG_CR0_CLR, CR0_RXON);     \
} while (0)

#define MACvTxOn(dwIoBase)                                  \
do {                                                           \
	VNSvOutPortB(dwIoBase + MAC_REG_CR0_CLR, CR0_STOP);     \
	VNSvOutPortB(dwIoBase + MAC_REG_CR0_SET, CR0_STRT);     \
	VNSvOutPortB(dwIoBase + MAC_REG_CR0_SET, CR0_TXON);     \
} while (0)

#define MACvTxOff(dwIoBase)                                 \
do {                                                           \
	VNSvOutPortB(dwIoBase + MAC_REG_CR0_CLR, CR0_TXON);     \
} while (0)

#define MACvRxQueueRUN(dwIoBase)                                    \
do {                                                                   \
	VNSvOutPortB(dwIoBase + MAC_REG_RDCSR_SET, TRDCSR_RUN);         \
} while (0)

#define MACvRxQueueRUNOff(dwIoBase)                                 \
do {                                                                   \
	VNSvOutPortB(dwIoBase + MAC_REG_RDCSR_CLR, TRDCSR_RUN);         \
} while (0)

#define MACvTxQueueRUN(dwIoBase, byTxQue)                                  \
do {                                                                          \
	WORD    wBitMask = TRDCSR_RUN;                                         \
	VNSvOutPortW(dwIoBase + MAC_REG_TDCSR_SET, (WORD)(wBitMask << (byTxQue * 4))); \
} while (0)

#define MACvTxQueueRUNOff(dwIoBase, byTxQue)                               \
do {                                                                          \
	WORD    wBitMask = TRDCSR_RUN;                                         \
	VNSvOutPortW(dwIoBase + MAC_REG_TDCSR_CLR, (WORD)(wBitMask << (byTxQue * 4))); \
} while (0)

#define MACvRxQueueWake(dwIoBase)                                   \
do {                                                                   \
	VNSvOutPortB(dwIoBase + MAC_REG_RDCSR_SET, TRDCSR_WAK);         \
} while (0)

#define MACvTxQueueWake(dwIoBase, byTxQue)                                 \
do {                                                                          \
	WORD    wBitMask = TRDCSR_WAK;                                         \
	VNSvOutPortW(dwIoBase + MAC_REG_TDCSR_SET, (WORD)(wBitMask << (byTxQue * 4))); \
} while (0)

#define MACvMisc(dwIoBase)                                          \
do {                                                                   \
	VNSvOutPortB(dwIoBase + MAC_REG_CR3_SET, CR3_GINTMSK1);         \
} while (0)

#define MACvClearStckDS(dwIoBase)                                    \
do {                                                                    \
	BYTE byOrgValue;                                                 \
	VNSvInPortB(dwIoBase + MAC_REG_STICKHW, &byOrgValue);            \
	byOrgValue = byOrgValue & (BYTE)0xFC;                            \
	VNSvOutPortB(dwIoBase + MAC_REG_STICKHW, byOrgValue);            \
} while (0)

#define MACvPwrEvntDisable(dwIoBase)                                 \
do {                                                                    \
	VNSvOutPortB(dwIoBase + MAC_REG_WOLCR0_CLR, 0xFF);               \
	VNSvOutPortB(dwIoBase + MAC_REG_WOLCR1_CLR, 0xFF);               \
} while (0)

#define MACvReadISR(dwIoBase, pdwValue)                              \
do {                                                                    \
	VNSvInPortD(dwIoBase + MAC_REG_ISR, pdwValue);                   \
} while (0)

#define MACvWriteISR(dwIoBase, dwValue)                              \
do {                                                                    \
	VNSvOutPortD(dwIoBase + MAC_REG_ISR, dwValue);                   \
} while (0)

#if defined(__USE_GMASK1__)

#define MACvIntEnable(dwIoBase, dwMask)                       \
do {                                                             \
	VNSvOutPortB(dwIoBase + MAC_REG_CR3_SET, CR3_GINTMSK1);   \
} while (0)

#define MACvIntDisable(dwIoBase)                              \
do {                                                             \
	VNSvOutPortB(dwIoBase + MAC_REG_CR3_CLR, CR3_GINTMSK1);   \
} while (0)

#else /* NOT (__USE_GMASK1__) */

#define MACvIntEnable(dwIoBase, dwMask)             \
do {                                                   \
	VNSvOutPortD(dwIoBase + MAC_REG_IMR, dwMask);   \
} while (0)

#define MACvIntDisable(dwIoBase)                        \
do {                                                       \
	VNSvOutPortD(dwIoBase + MAC_REG_IMR, 0x00000000UL); \
} while (0)

#endif /* end (__USE_GMASK1__) */


/*---------------------  Export Classes  ----------------------------*/

/*---------------------  Export Variables  --------------------------*/

/*---------------------  Export Functions  --------------------------*/
#ifdef __cplusplus
extern "C" {                            /* Assume C declarations for C++ */
#endif /* __cplusplus */

VOID GMACvReadAllRegs(DWORD dwIoBase, PSMacReg pMacRegs, BYTE byRevId);

BOOL GMACbIsRegBitsOn(DWORD dwIoBase, BYTE byRegOfs, BYTE byTestBits);
BOOL GMACbIsRegBitsOff(DWORD dwIoBase, BYTE byRegOfs, BYTE byTestBits);

BYTE GMACbyReadEECSR(DWORD dwIoBase);
VOID GMACvWriteEECSR(DWORD dwIoBase, BYTE byValue);
BYTE GMACbyReadMIICR(DWORD dwIoBase);
VOID GMACvWriteMIICR(DWORD dwIoBase, BYTE byValue);
BYTE GMACbyReadMultiAddr(DWORD dwIoBase, UINT uByteIdx);
VOID GMACvWriteMultiAddr(DWORD dwIoBase, UINT uByteIdx, BYTE byData);

BOOL GMACbIsIntDisable(DWORD dwIoBase, BYTE byRevId);

VOID GMACvSetMultiAddrByHash(DWORD dwIoBase, BYTE byHashIdx);
VOID GMACvResetMultiAddrByHash(DWORD dwIoBase, BYTE byHashIdx);

BYTE GMACbyGetBootRomSize(DWORD dwIoBase);

VOID GMACvSetPhyId(DWORD dwIoBase, BYTE byRevId, BYTE byPhyId);
BYTE GMACbyGetPhyId(DWORD dwIoBase);

VOID GMACvSetRxThreshold(DWORD dwIoBase, BYTE byThreshold);
VOID GMACvGetRxThreshold(DWORD dwIoBase, PBYTE pbyThreshold);

VOID GMACvSetDmaLength(DWORD dwIoBase, BYTE byDmaLength);
VOID GMACvGetDmaLength(DWORD dwIoBase, PBYTE pbyDmaLength);

VOID GMACvSetLoopbackMode(DWORD dwIoBase, BYTE byLoopbackMode);
BOOL GMACbIsInLoopbackMode(DWORD dwIoBase);

BOOL GMACbIsIn1GMode(DWORD dwIoBase, BYTE byRevId);
BOOL GMACbIsIn100MMode(DWORD dwIoBase, BYTE byRevId);
BOOL GMACbIsInFullDuplexMode(DWORD dwIoBase);
BOOL GMACbIsCableLinkOk(DWORD dwIoBase);

VOID GMACvDisableMiiAutoPoll(DWORD dwIoBase);
VOID GMACvEnableMiiAutoPoll(DWORD dwIoBase);

VOID GMACvSetPacketFilter(DWORD dwIoBase, WORD wFilterType);

VOID GMACvSaveContext(DWORD dwIoBase, BYTE byRevId, PBYTE pbyCxtBuf);
VOID GMACvRestoreContext(DWORD dwIoBase, BYTE byRevId, PBYTE pbyCxtBuf);

VOID GMACvTimer0MiniSDelay(DWORD dwIoBase, BYTE byRevId, UINT udelay);
VOID GMACvTimer0MicroSDelay(DWORD dwIoBase, BYTE byRevId, UINT udelay);

BOOL GMACbSoftwareReset(DWORD dwIoBase, BYTE byRevId);
BOOL GMACbSafeSoftwareReset(DWORD dwIoBase, BYTE byRevId);

BOOL GMACbSafeRxOff(DWORD dwIoBase, BYTE byRevId);
BOOL GMACbSafeTxOff(DWORD dwIoBase, BYTE byRevId);
BOOL GMACbStop(DWORD dwIoBase);
BOOL GMACbSafeStop(DWORD dwIoBase, BYTE byRevId);
BOOL GMACbShutdown(DWORD dwIoBase, BYTE byRevId);
VOID GMACvInitialize(PSAdapterInfo pAdapter, DWORD dwIoBase, BYTE byRevId);

VOID GMACvSetVCAM(DWORD dwIoBase, BYTE byAddress, WORD wVID);
VOID GMACvSetMCAM(DWORD dwIoBase, BYTE byAddress, PBYTE pbyData);
VOID GMACvGetVCAM(DWORD dwIoBase, BYTE byAddress, PWORD pwData);
VOID GMACvGetMCAM(DWORD dwIoBase, BYTE byAddress, PBYTE pbyData);
VOID GMACvSetVCAMMask(DWORD dwIoBase, PBYTE pbyMask);
VOID GMACvSetMCAMMask(DWORD dwIoBase, PBYTE pbyMask);
VOID GMACvGetVCAMMask(DWORD dwIoBase, PBYTE pbyMask);
VOID GMACvGetMCAMMask(DWORD dwIoBase, PBYTE pbyMask);

VOID GMACvSetVCAMMaskByBR(DWORD dwIoBase, PBYTE pbyMask);
VOID GMACvSetMCAMMaskByBR(DWORD dwIoBase, PBYTE pbyMask);
VOID GMACvGetVCAMMaskByBR(DWORD dwIoBase, PBYTE pbyMask);
VOID GMACvGetMCAMMaskByBR(DWORD dwIoBase, PBYTE pbyMask);
VOID GMACvSetTqIndex(DWORD dwIoBase, BYTE byTxQue, WORD wTdIdx);

void dumpmac(int iobase);

VOID VPCIvWriteD(DWORD dwIoBase, BYTE byRegOffset, DWORD dwData);
VOID VPCIvReadD(DWORD dwIoBase, BYTE byRegOffset, PDWORD pdwData);

#ifdef __cplusplus
}                                       /* End of extern "C" { */
#endif /* __cplusplus */

#endif /* __MAC_H__ */
