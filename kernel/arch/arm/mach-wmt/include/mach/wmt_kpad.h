/*++
linux/include/asm-arm/arch-wmt/wmt_lpad.h

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
#error "You must include hardware.h, not wmt_kpad.h"
#endif

#ifndef __WMT_KPAD_H
#define __WMT_KPAD_H

/*=============================================================================
 *
 *Define the register access macros.
 *
 *Note: Current policy in standalone program is using register as a pointer.
 *
 *=============================================================================*/

/*=============================================================================
 *
 * WMT Keypad Base Address.
 *
 *=============================================================================*/
#ifdef __KPAD_BASE
#error "__KPAD_BASE has already been defined in another file."
#else
    #define    __KPAD_BASE    KPAD_BASE_ADDR /* 64K */
#endif
/*=============================================================================
 *
 * WMT Keypad control registers.
 *
 * Registers Abbreviations:
 *
 * KPMCR_REG    Keypad Matrix Control Register.
 *
 * KPDCR_REG    Keypad Direct Input Control Register.
 *
 * KPICR_REG    Keypad Invert Input Control Register.
 *
 * KPSTR_REG    Keypad STatus Register.
 *
 * KPMAR_REG    Keypad Matrix Primary Key Automatic Scan Register.
 *
 * KPDSR_REG    Keypad Direct Input Key Scan Register.
 *
 * KPMMR_REG    Keypad Matrix Manual Key Scan Register.
 *
 * KPRIR_REG    Keypad Row Input Register.
 *
 * KPMR0_REG    Keypad Matrix Multiple Keys Scan Register 0.
 *
 * KPMR1_REG    Keypad Matrix Multiple Keys Scan Register 1.
 *
 * KPMR2_REG    Keypad Matrix Multiple Keys Scan Register 2.
 *
 * KPMR3_REG    Keypad Matrix Multiple Keys Scan Register 3.
 *
 * KPMIR_REG    Keypad Matrix Debounce and Scan Interval Register.
 *
 * KPDIR_REG    Keypad Direct Input Debounce Interval Register.
 *
 *=============================================================================*/
/*=============================================================================
 *
 * Address constant for each register.
 *
 *=============================================================================*/
#define    KPMCR_ADDR		(__KPAD_BASE + 0x00)
#define    KPDCR_ADDR		(__KPAD_BASE + 0x04)
#define    KPICR_ADDR		(__KPAD_BASE + 0x08)
#define    KPSTR_ADDR		(__KPAD_BASE + 0x0C)
#define    KPMAR_ADDR		(__KPAD_BASE + 0x10)
#define    KPDSR_ADDR		(__KPAD_BASE + 0x14)
#define    KPMMR_ADDR		(__KPAD_BASE + 0x18)
#define    KPRIR_ADDR		(__KPAD_BASE + 0x1C)
#define    KPMR0_ADDR		(__KPAD_BASE + 0x20)
#define    KPMR1_ADDR		(__KPAD_BASE + 0x24)
#define    KPMR2_ADDR		(__KPAD_BASE + 0x28)
#define    KPMR3_ADDR		(__KPAD_BASE + 0x2C)
#define    KPMIR_ADDR		(__KPAD_BASE + 0x30)
#define    KPDIR_ADDR		(__KPAD_BASE + 0x34)


/*=============================================================================
 *
 * Register pointer.
 *
 *=============================================================================*/
#define    KPMCR_REG       (REG32_PTR(KPMCR_ADDR))
#define    KPDCR_REG       (REG32_PTR(KPDCR_ADDR))
#define    KPICR_REG       (REG32_PTR(KPICR_ADDR))
#define    KPSTR_REG       (REG32_PTR(KPSTR_ADDR))
#define    KPMAR_REG       (REG32_PTR(KPMAR_ADDR))
#define    KPDSR_REG       (REG32_PTR(KPDSR_ADDR))
#define    KPMMR_REG       (REG32_PTR(KPMMR_ADDR))
#define    KPRIR_REG       (REG32_PTR(KPRIR_ADDR))
#define    KPMR0_REG       (REG32_PTR(KPMR0_ADDR))
#define    KPMR1_REG       (REG32_PTR(KPMR1_ADDR))
#define    KPMR2_REG       (REG32_PTR(KPMR2_ADDR))
#define    KPMR3_REG       (REG32_PTR(KPMR3_ADDR))
#define    KPMIR_REG       (REG32_PTR(KPMIR_ADDR))
#define    KPDIR_REG       (REG32_PTR(KPDIR_ADDR))

/*=============================================================================
 *
 * Register value.
 *
 *=============================================================================*/
#define    KPMCR_VAL       (REG32_VAL(KPMCR_ADDR))
#define    KPDCR_VAL       (REG32_VAL(KPDCR_ADDR))
#define    KPICR_VAL       (REG32_VAL(KPICR_ADDR))
#define    KPSTR_VAL       (REG32_VAL(KPSTR_ADDR))
#define    KPMAR_VAL       (REG32_VAL(KPMAR_ADDR))
#define    KPDSR_VAL       (REG32_VAL(KPDSR_ADDR))
#define    KPMMR_VAL       (REG32_VAL(KPMMR_ADDR))
#define    KPRIR_VAL       (REG32_VAL(KPRIR_ADDR))
#define    KPMR0_VAL       (REG32_VAL(KPMR0_ADDR))
#define    KPMR1_VAL       (REG32_VAL(KPMR1_ADDR))
#define    KPMR2_VAL       (REG32_VAL(KPMR2_ADDR))
#define    KPMR3_VAL       (REG32_VAL(KPMR3_ADDR))
#define    KPMIR_VAL       (REG32_VAL(KPMIR_ADDR))
#define    KPDIR_VAL       (REG32_VAL(KPDIR_ADDR))

/*=============================================================================
 *
 *    16' h0038-16' hFFFF Reserved (Read-only, all zeros)
 *
 *=============================================================================*/

/*=============================================================================
 *
 * KPMCR_REG    Keypad Matrix Control Register.
 *
 *=============================================================================*/
#define KPMCR_EN			BIT0		/* Keypad Matrix Enable bit. */
#define KPMCR_IEN			BIT1		/* Keypad Matrix Interrupt Request Enable bit. */
#define KPMCR_AS			BIT2		/* Keypad Matrix Automatic Scan bit. */
#define KPMCR_ASA			BIT3		/* Keypad Matrix Automatic Scan on Activity bit. */
#define KPMCR_IMK			BIT4		/* Keypad Matrix Ignore Multiple Key-press bit. */
#define KPMCR_COLMASK		0x0700		/* Keypad Matrix Column Number bits. */
#define KPMCR_ROWMASK		0x7000		/* Keypad Matrix Row Number bits. */
#define KPMCR_MSMASK		(0xFF << 16)	/* Manual Keypad Matrix Scan Output signals */
#define KPMCR_COL(x)		(((x) <<  8) & KPMCR_COLMASK)
#define KPMCR_ROW(x)		(((x) << 12) & KPMCR_ROWMASK)
#define KPMCR_MS(x)			(((x) << 16) & KPMCR_MSMASK)

/*=============================================================================
 *
 * KPDCR_REG    Keypad Direct Input Control Register.
 *
 *=============================================================================*/
#define KPDCR_EN			BIT0			/* Direct Input Enable */
#define KPDCR_IEN			BIT1			/* Direct Input Interrupt Request Enable */
#define KPDCR_ASA			BIT3			/* Direct Input Automatic Scan on Activity */
#define KPDCR_IMK			BIT4			/* Direct Input Ignore Muiltiple Key-press */
#define KPDCR_DENMASK		(0xFF << 16)	/* Direct Input Enable bit[0:7] */
#define KPDCR_DEN(x)		(((x) << 16) & KPDCR_DENMASK)

/*=============================================================================
 *
 * KPICR_REG    Keypad Invert Input Control Register.
 *
 *=============================================================================*/
#define KPICR_IRIMASK		(0xFF << 16)	/* Invert Row input signals */
#define KPICR_IRI(x)		(((x) << 16) & KPICR_IRIMASK)

/*=============================================================================
 *
 * KPSTR_REG    Keypad Status Register.
 *
 *=============================================================================*/
#define KPSTR_MDA			BIT0		/* Keypad Matrix Manual Debounce Active Key bit. */
#define KPSTR_ASA			BIT1		/* Keypad Matrix Automatic Scan on Activity bit. */
#define KPSTR_ASC			BIT2		/* Keypad Matrix Automatic Scan Completed bit.   */
#define KPSTR_DIA			BIT3		/* Keypad Direct Input Active bit.               */
#define KPSTR_MASK			0xF

/*=============================================================================
 *
 * KPMAR_REG    Keypad Matrix Primary Key Automatic Scan Register.
 *
 *=============================================================================*/
#define KPMAR_COLMASK		(BIT0 | BIT1 | BIT2)
#define KPMAR_ROWMASK		(BIT4 | BIT5 | BIT6)
#define KPMAR_KEYMASK		(BIT29 | BIT30)
#define KPMAR_KEYSHIFT		29
#define KPMAR_KEY(reg)		(((reg) & KPMAR_KEYMASK) >> KPMAR_KEYSHIFT)
#define KPMAR_NOKEY			0x0		/* Bit[29:30] no key pressed.  */
#define KPMAR_ONEKEY		0x1		/* Bit[29:30] one key pressed. */
#define KPMAR_MULTIKEYS		0x2		/* Bit[29:30] multiple keys pressed.*/
/* Notice that 0x3 is also multikeys */
#define KPMAR_VALID			BIT31

/*
 * Keypad Direct Input Key Scan Register
 */
#define Dir_Input			(BIT0|BIT1|BIT2|BIT3|BIT4|BIT5|BIT6|BIT7)
#define Dir_Vaild_Scan		(BIT31)

/*
 * Keypad Manual Matrix Key Scan Register
 */

/*
 * Keypad Row Input Register
 */
#define Row_Input			(BIT0|BIT1|BIT2|BIT3|BIT4|BIT5|BIT6|BIT7)

/*=============================================================================
 *
 * KPMR0_REG    Keypad Matrix Multiple Key Scan Register 0.
 * KPMR1_REG    Keypad Matrix Multiple Key Scan Register 1.
 * KPMR2_REG    Keypad Matrix Multiple Key Scan Register 2.
 * KPMR3_REG    Keypad Matrix Multiple Key Scan Register 3.
 *
 *=============================================================================*/
#define KPMRX_VALID			BIT31
#define KPMRX_EVENMASK		0xFF			/* Even Column Row Input Active bits. */
#define KPMRX_ODDMASK		(0xFF << 16)	/* Odd Column Row Input Active bits.  */

/*=============================================================================
 *
 * KPMIR_REG    Keypad Matrix Debounce and Scan Interval Register.
 *
 *=============================================================================*/
#define KPMIR_DIMASK		0x0FFF			/* Matrix debounce interval mask */
#define KPMIR_SIMASK		(0xFF << 16)	/* Keypaf scan interval mask */
#define KPMIR_DI(x)			((x) & KPMIR_DIMASK)
#define KPMIR_SI(x)			(((x) << 16) & KPMIR_SIMASK)

/*=============================================================================
 *
 * KPDIR_REG    Keypad Direct Debounce and Scan Interval Register.
 *
 *=============================================================================*/
#define KPDIR_DIMASK		0x0FFF			/* Direct input debounce interval mask */
#define KPDIR_DI(x)			((x) & KPDIR_DIMASK)

/*=============================================================================
 *
 * Feature Supported (Keypad Module)
 *
 *=============================================================================*/
/*
#define AutoScna		(MIE | MAS | MASA)
#define ManualScan		(MIE & (~MAS) & (~MASA))
#define DirectScan		((~MAS) & (~MASA))
#define Col3xRow4		(Col3 | Row4)
#define Col4xRow4		(Col4 | Row4)
*/
#endif

