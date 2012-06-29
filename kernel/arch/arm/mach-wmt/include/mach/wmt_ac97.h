/*++
linux/include/asm-arm/arch-wmt/wmt_ac97.h

Copyright (c) 2008  WonderMedia Technologies, Inc.

This program is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software Foundation,
either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with
this program.  If not, see <http://www.gnu.org/licenses/>.

WonderMedia Technologies, Inc.
10F, 529, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C.
--*/

/* Be sure that virtual mapping is defined right */
#ifndef __ASM_ARCH_HARDWARE_H
#error "You must include hardware.h, not vt8500_ac97.h"
#endif

#ifndef __VT8500_AC97_H
#define __VT8500_AC97_H

/******************************************************************************
 *
 *  AC'97 Base Address.
 *
 ******************************************************************************/
#ifdef __AC97_BASE
#error  "__AC97_BASE has already been defined in another file."
#endif
#if 0
#ifdef AC97_BASE_ADDR                       /* From vt8420.mmap.h */
#define __AC97_BASE      AC97_BASE_ADDR
#else
#define __AC97_BASE      0xF8430000         /* 64K */
#endif
#else
#define __AC97_BASE      AC97_BASE_ADDR     /*From vt8610_mmap.h	 */
#endif

/******************************************************************************
 *
 * VT8500 AC'97 controller registers preface.
 *
 * NOTE:
 *      Following abbreviations are come from ac97_vt8500 spec, but not used
 *      in actual operaion, I list them here in order to help users to compare
 *      with the ac97_vt8500 spec.
 *
 * ACCR         AC97 Controller Control Register.
 *
 * ACSR         AC97 Controller Status Register.
 *
 * CSLR         CODEC Command Status Lock Register.
 *
 * CCR          CODEC Command Register.
 *
 * CSDR         CODEC Status Data Register.
 *
 * PTCR         PCM Tx FIFO Control Register.
 *
 * PTSR         PCM Tx FIFO Status Register.
 *
 * PRCR         PCM Rx FIFO Control Register.
 *
 * PRSR         PCM Rx FIFO Status Register.
 *
 * MCR          Microphone FIFO Control Register.
 *
 * MSR          Microphone FIFO Status Register.
 *
 * PTFIFO       PCM Stereo Tx FIFO.
 *
 * PTFIFOA      PCM Stereo Tx FIFO Alias.
 *
 * PRFIFO       PCM Stereo Rx FIFO.
 *
 * PRFIFOA      PCM Stereo Rx FIFO Alias.
 *
 * MFIFO        Microphone FIFO.
 *
 * MFIFOA       Microphone FIFO Alias.
 *
 ******************************************************************************/
/******************************************************************************
 * VT8500 AC'97 controller registers overview.
 *
 * Registers Abbreviations:
 *
 * ACGC_REG     AC97 Controller Global Control Register (ACCR).
 *
 * ACGS_REG     AC97 Controller Global Status Register (ACSR).
 *
 * CCSL_REG     CODEC Command Status Lock Register (CSLR).
 *
 * CCMD_REG     CODEC Command Register (CCR).
 *
 * CRSD_REG     CODEC Read Status Data Register (CSDR).
 *
 * PTFC_REG     PCM Tx FIFO Control Register (PTCR).
 *
 * PTFS_REG     PCM Tx FIFO Status Register (PTSR).
 *
 * PRFC_REG     PCM Rx FIFO Control Register (PRCR).
 *
 * PRFS_REG     PCM Rx FIFO Status Register (PRSR).
 *
 * MIFC_REG     Microphone FIFO Control Register (MCR).
 *
 * MIFS_REG     Microphone FIFO Status Register (MSR).
 *
 * PTFP_REG     PCM Tx FIFO Write Port start entry Register (PTFIFO).
 *
 * PRFP_REG     PCM Rx FIFO Read Port start entry Register (PRFIFO).
 *
 * MIRP_REG     Microphone FIFO Read Port start entry Register (MFIFO).
 *
 ******************************************************************************/
/******************************************************************************
 *
 * Address constant for each register.
 *
 ******************************************************************************/
#define	ACGC_ADDR       (__AC97_BASE + 0x0000)
#define	ACGS_ADDR       (__AC97_BASE + 0x0004)
#define	CCSL_ADDR       (__AC97_BASE + 0x0008)
#define	CCMD_ADDR       (__AC97_BASE + 0x000C)
#define	CRSD_ADDR       (__AC97_BASE + 0x0010)
/* 16'h0014-16'h001F Reserved (Read-only, all zeros) */
#define	PTFC_ADDR       (__AC97_BASE + 0x0020)
#define	PTFS_ADDR       (__AC97_BASE + 0x0024)
#define	PRFC_ADDR       (__AC97_BASE + 0x0028)
#define	PRFS_ADDR       (__AC97_BASE + 0x002C)
#define	MIFC_ADDR       (__AC97_BASE + 0x0030)
#define	MIFS_ADDR       (__AC97_BASE + 0x0034)
/* 16'h0038-16'h007F Reserved (Read-only, all zeros) */
#define	PTFP_ADDR       (__AC97_BASE + 0x0080)
/* 16'h0084-16'h00BF is PCM Tx FIFO Alias. */
#define	PRFP_ADDR       (__AC97_BASE + 0x00C0)
/* 16'h00C4-16'h00FF is PCM Rx FIFO Alias. */
#define	MIRP_ADDR       (__AC97_BASE + 0x0100)
/* 16'h0104-16'h013F is Microphone FIFO Alias. */
/* 16'h0140-16'hFFFF Reserved (Read-only, all zeros) */

/******************************************************************************
 *
 * Register pointer.
 *
 ******************************************************************************/
#define	ACGC_REG        (REG32_PTR(ACGC_ADDR))
#define	ACGS_REG        (REG32_PTR(ACGS_ADDR))
#define	CCSL_REG        (REG32_PTR(CCSL_ADDR))
#define	CCMD_REG        (REG32_PTR(CCMD_ADDR))
#define	CRSD_REG        (REG32_PTR(CRSD_ADDR))
#define	PTFC_REG        (REG32_PTR(PTFC_ADDR))
#define	PTFS_REG        (REG32_PTR(PTFS_ADDR))
#define	PRFC_REG        (REG32_PTR(PRFC_ADDR))
#define	PRFS_REG        (REG32_PTR(PRFS_ADDR))
#define	MIFC_REG        (REG32_PTR(MIFC_ADDR))
#define	MIFS_REG        (REG32_PTR(MIFS_ADDR))
#define	PTFP_REG        (REG32_PTR(PTFP_ADDR))
#define	PRFP_REG        (REG32_PTR(PRFP_ADDR))
#define	MIRP_REG        (REG32_PTR(MIRP_ADDR))

/******************************************************************************
 *
 * Register value.
 *
 ******************************************************************************/
#define	ACGC_VAL        (REG32_VAL(ACGC_ADDR))
#define	ACGS_VAL        (REG32_VAL(ACGS_ADDR))
#define	CCSL_VAL        (REG32_VAL(CCSL_ADDR))
#define	CCMD_VAL        (REG32_VAL(CCMD_ADDR))
#define	CRSD_VAL        (REG32_VAL(CRSD_ADDR))
#define	PTFC_VAL        (REG32_VAL(PTFC_ADDR))
#define	PTFS_VAL        (REG32_VAL(PTFS_ADDR))
#define	PRFC_VAL        (REG32_VAL(PRFC_ADDR))
#define	PRFS_VAL        (REG32_VAL(PRFS_ADDR))
#define	MIFC_VAL        (REG32_VAL(MIFC_ADDR))
#define	MIFS_VAL        (REG32_VAL(MIFS_ADDR))
#define	PTFP_VAL        (REG32_VAL(PTFP_ADDR))
#define	PRFP_VAL        (REG32_VAL(PRFP_ADDR))
#define	MIRP_VAL        (REG32_VAL(MIRP_ADDR))

/******************************************************************************
 *
 * ACGC_REG     AC97 Controller Global Control Register bits functions.
 *
 ******************************************************************************/
#define ACGC_ACR        BIT0            /* AC97 Cold Reset                   */
#define ACGC_AWR        BIT1            /* AC97 Warm Reset                   */
#define ACGC_CWDIE      BIT2            /* CODEC Write Done Interrupt Enable */
#define ACGC_CRDIE      BIT3            /* CODEC Read Done Interrupt Enable  */
#define ACGC_CRIE       BIT4            /* CODEC Ready Interrupt Enable      */
/* Bits 5-31: Reserved */

/******************************************************************************
 *
 * ACGS_REG     AC97 Controller Global Status Register bits definitions.
 *
 ******************************************************************************/
#define ACGS_CRDY       BIT0            /* CODEC Ready                  */
#define ACGS_CWD        BIT1            /* CODEC Write Done (w1c)       */
#define ACGS_CRD        BIT2            /* CODEC Read Done (w1c)        */
#define ACGS_CST        BIT3            /* CODEC Status Timeout (w1c)   */
#define ACGS_MFIA       BIT4            /* Mic FIFO Interrupt Active    */
#define ACGS_PRFIA      BIT5            /* PCM Rx FIFO Interrupt Active */
#define ACGS_PTFIA      BIT6            /* PCM Tx FIFO Interrupt Active */
#define ACGS_CPD        BIT7            /* CODEC Power Down             */
#define ACGS_W1CMASK    (BIT1 | BIT2 | BIT3) /* Write one to clear bits */
/* Bits 8-31: Reserved */

/******************************************************************************
 *
 * CCSL_REG     CODEC Command Status Lock Register bits definitions.
 *
 ******************************************************************************/
#define CCSL_CSLB       BIT0            /* CODEC Software Lock Bit */
/* Bits 1-31: Reserved */

/******************************************************************************
 *
 * CCMD_REG     CODEC Command Register bits functions.
 *
 ******************************************************************************/
#define CCMD_CCAMASK    0x7F            /* CODEC Command Address bits 0-6      */
#define CCMD_CCA(x)     ((x) & CCMD_CCAMASK)
#define CCMD_CCRW       BIT7            /* CODEC Command Read_nWrite           */
/* Bits 8-15: Reserved */
#define CCMD_CWCDMASK   0xFFFF          /* CODEC Write Command Data bits 16-31 */
#define CCMD_CWCD(x)    (((x) & CCMD_CWCDMASK) << 16)

/******************************************************************************
 *
 * CRSD_REG     CODEC Read Status Data Register bits definitions.
 *
 ******************************************************************************/
#define CRSD_CSDMASK    0xFFFF          /* CODEC Status Data bits 0-15 */
/* Bits 16-31: Reserved */

/******************************************************************************
 *
 * PTFC_REG     PCM Tx FIFO Control Register bits functions.
 *
 ******************************************************************************/
#define PTFC_PTFEN      BIT0            /* PCM Tx FIFO Enable                          */
#define PTFC_PTFEIE     BIT1            /* PCM Tx FIFO Empty Interrupt Enable          */
#define PTFC_PTFAIE     BIT2            /* PCM Tx FIFO Almost Empty Interrupt Enable   */
#define PTFC_PTFADE     BIT3            /* PCM Tx FIFO Almost Empty DMA Request Enable */
#define PTFC_PTFUIE     BIT4            /* PCM Tx FIFO Underrun Interrupt Enable       */
#define PTFC_PTFOIE     BIT5            /* PCM Tx FIFO Overrun Interrupt Enable        */
/* Bits 6-7: Reserved */
#define PTFC_PTFTMASK   (BIT8 | BIT9 | BIT10 | BIT11)   /* PCM Tx FIFO Threshold       */
#define PTFC_PTFT(x)    (((x) << 8) & PTFC_PTFTMASK)    /* x = [0-16] valid 0 - Threshold = 16 */
#define PTFC_PTF8M     BIT12            /* PCM Tx FIFO 8-bit Mode                      */
#define PTFC_PTFMM     BIT13            /* PCM Tx FIFO Mono Mode                       */
/* Bits 14-31: Reserved */

/******************************************************************************
 *
 * PTFS_REG     PCM Tx FIFO Status Register bits definitions.
 *
 ******************************************************************************/
#define PTFS_PTFE       BIT0            /* PCM Tx FIFO Empty          */
#define PTFS_PTFA       BIT1            /* PCM Tx FIFO Almost Empty   */
#define PTFS_PTFUE      BIT2            /* PCM Tx FIFO Underrun Error */
#define PTFS_PTFOE      BIT3            /* PCM Tx FIFO Overrun Error  */
#define PTFS_W1CMASK    (BIT2 | BIT3)   /* Write one to clear bits    */
/* Bits 4-31: Reserved */

/******************************************************************************
 *
 * PRFC_REG     PCM Rx FIFO Control Register bits functions.
 *
 ******************************************************************************/
#define PRFC_PRFEN      BIT0            /* PCM Rx FIFO Enable                         */
#define PRFC_PRFFIE     BIT1            /* PCM Rx FIFO Full Interrupt Enable          */
#define PRFC_PRFAIE     BIT2            /* PCM Rx FIFO Alomost Full Interrupt Enable  */
#define PRFC_PRFADE     BIT3            /* PCM Rx FIFO Almost Full DMA Request Enable */
#define PRFC_PRFUIE     BIT4            /* PCM Rx FIFO Underrun Interrupt Enable      */
#define PRFC_PRFOIE     BIT5            /* PCM Rx FIFO Overrun Interrupt Enable       */
/* Bits 6-7: Reserved */
#define PRFC_PRFTMASK   (BIT8 | BIT9 | BIT10 | BIT11)   /* PCM Rx FIFO Threshold      */
#define PRFC_PRFT(x)    (((x) << 8) & PRFC_PRFTMASK)    /* x = [0-16] valid 0 - Threshold = 16 */
#define PRFC_PRF8M     BIT12            /* PCM Rx FIFO 8-bit Mode                      */
#define PRFC_PRFMM     BIT13            /* PCM Rx FIFO Mono Mode                       */
/* Bits 14-31: Reserved */

/******************************************************************************
 *
 * PRFS_REG     PCM Rx FIFO Status Register bits definitions.
 *
 ******************************************************************************/
#define PRFS_PRFF       BIT0            /* PCM Rx FIFO Full           */
#define PRFS_PRFA       BIT1            /* PCM Rx FIFO Almost Full    */
#define PRFS_PRFUE      BIT2            /* PCM Rx FIFO Underrun Error */
#define PRFS_PRFOE      BIT3            /* PCM Rx FIFO Overrun Error  */
#define PRFS_W1CMASK    (BIT2 | BIT3)   /* Write one to clear bits    */
/* Bits 4-31: Reserved */

/******************************************************************************
 *
 * MIFC_REG     Microphone FIFO Control Register bits functions.
 *
 ******************************************************************************/
#define MIFC_MFEN       BIT0            /* Mic FIFO Enable                       */
#define MIFC_MFFIE      BIT1            /* Mic FIFO Full Interrupt Enable        */
#define MIFC_MFAIE      BIT2            /* Mic FIFO Almost Full Interrupt Enable */
#define MIFC_MFADE      BIT3            /* Mic Almost Full DMA Request Enable    */
#define MIFC_MFUIE      BIT4            /* Mic FIFO Underrun Interrupt Enable    */
#define MIFC_MFOIE      BIT5            /* Mic FIFO Overrun Interrupt Enable     */
/* Bits 6-7: Reserved */
#define MIFC_MFTMASK    (BIT8 | BIT9 | BIT10 | BIT11)   /* Mic FIFO Threshold    */
#define MIFC_MFT(x)     (((x) << 8) & MIFC_MFTMASK)     /* x = [0-16] valid 0 - Threshold = 16 */
#define MIFC_MF8M       BIT12           /* Mic FIFO 8-bit Mode                   */
/* Bits 13-31: Reserved */

/******************************************************************************
 *
 * MIFS_REG     Microphone FIFO Status Register bits definitions.
 *
 ******************************************************************************/
#define MIFS_MFF        BIT0            /* Mic FIFO Full           */
#define MIFS_MFA        BIT1            /* Mic FIFO Almost Full    */
#define MIFS_MFUE       BIT2            /* Mic FIFO Underrun Error */
#define MIFS_MFOE       BIT3            /* Mic FIFO Overrun Error  */
#define MIFS_W1CMASK    (BIT2 | BIT3)   /* Write one to clear bits */
/* Bits 4-31: Reserved */

/******************************************************************************
 *
 * CODEC define section.
 *
 ******************************************************************************/
/*#include "ucb1400.h" */

#endif	/* __VT8500_AC97_H */
