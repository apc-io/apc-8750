/*++
linux/include/asm-arm/arch-wmt/wmt_i2s.h

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
#error "You must include hardware.h, not wmt_i2s.h"
#endif

#ifndef _WMT_I2S_H_
#define _WMT_I2S_H_


/******************************************************************************
 *
 * Address constant for each register.
 *
 ******************************************************************************/
#define	DACCFG_ADDR       		(I2S_BASE_ADDR + 0x0040)
#define HDACKGEN_ADDR				(I2S_BASE_ADDR + 0x0070)
#define ADCCFG_ADDR				(I2S_BASE_ADDR + 0x0080)
#define AADCFEN_ADDR			(I2S_BASE_ADDR + 0x008C)
#define AADCFSTATE_ADDR			(I2S_BASE_ADDR + 0x0090)
#define DGOCFG_ADDR			(I2S_BASE_ADDR + 0x00C0)
#define ADGICFG_ADDR			(I2S_BASE_ADDR + 0x00D0)
#define ASMPFCFG_ADDR			(I2S_BASE_ADDR + 0x0180)
#define ASMPFRDY_ADDR			(I2S_BASE_ADDR + 0x0184)
#define ASMPFBS_TYPE_ADDR		(I2S_BASE_ADDR + 0x0188)
#define ASMPFCHCFG_ADDR			(I2S_BASE_ADDR + 0x0194)
#define AUDPRFRST_ADDR			(I2S_BASE_ADDR + 0x0244)
#define AADCFOBDOUT_DMA_ADDR	(I2S_BASE_ADDR + 0x0300)
#define ASMPFDP_DMA_ADDR		(I2S_BASE_ADDR + 0x0360)
#define DZDRQ8_CFG_ADDR			(I2S_BASE_ADDR + 0x03A0)
#define DZDRQ9_CFG_ADDR			(I2S_BASE_ADDR + 0x03A4)
#define DZDRQA_CFG_ADDR			(I2S_BASE_ADDR + 0x03A8)
#define DGOCS0A_ADDR			(I2S_BASE_ADDR + 0x0114)
#define DGOCS1A_ADDR			(I2S_BASE_ADDR + 0x012C)


/******************************************************************************
 *
 * Register pointer.
 *
 ******************************************************************************/
#define	DACCFG_REG        		(REG32_PTR(DACCFG_ADDR))
#define	HDACKGEN_REG        		(REG32_PTR(HDACKGEN_ADDR))
#define	ADCCFG_REG        		(REG32_PTR(ADCCFG_ADDR))
#define	AADCFEN_REG        		(REG32_PTR(AADCFEN_ADDR))
#define	AADCFSTATE_REG        	(REG32_PTR(AADCFSTATE_ADDR))
#define	DGOCFG_REG        		(REG32_PTR(DGOCFG_ADDR))
#define	ADGICFG_REG        		(REG32_PTR(ADGICFG_ADDR))
#define	ASMPFCFG_REG        	(REG32_PTR(ASMPFCFG_ADDR))
#define	ASMPFRDY_REG        	(REG32_PTR(ASMPFRDY_ADDR))
#define	ASMPFBS_TYPE_REG        (REG32_PTR(ASMPFBS_TYPE_ADDR))
#define	ASMPFCHCFG_REG        	(REG32_PTR(ASMPFCHCFG_ADDR))
#define	AUDPRFRST_REG        	(REG32_PTR(AUDPRFRST_ADDR))
#define	AADCFOBDOUT_DMA_REG		(REG32_PTR(AADCFOBDOUT_DMA_ADDR))
#define	ASMPFDP_DMA_REG			(REG32_PTR(ASMPFDP_DMA_ADDR))
#define	DZDRQ8_CFG_REG			(REG32_PTR(DZDRQ8_CFG_ADDR))
#define	DZDRQ9_CFG_REG			(REG32_PTR(DZDRQ9_CFG_ADDR))
#define	DZDRQA_CFG_REG			(REG32_PTR(DZDRQA_CFG_ADDR))
#define	DGOCS0A_REG			(REG32_PTR(DGOCS0A_ADDR))
#define	DGOCS1A_REG			(REG32_PTR(DGOCS1A_ADDR))


/******************************************************************************
 *
 * Register value.
 *
 ******************************************************************************/
#define	DACCFG_VAL        		(REG32_VAL(DACCFG_ADDR))
#define	HDACKGEN_VAL        		(REG32_VAL(HDACKGEN_ADDR))
#define	ADCCFG_VAL        		(REG32_VAL(ADCCFG_ADDR))
#define	AADCFEN_VAL        		(REG32_VAL(AADCFEN_ADDR))
#define	AADCFSTATE_VAL     		(REG32_VAL(AADCFSTATE_ADDR))
#define	DGOCFG_VAL     		(REG32_VAL(DGOCFG_ADDR))
#define	ADGICFG_VAL     		(REG32_VAL(ADGICFG_ADDR))
#define	ASMPFCFG_VAL        	(REG32_VAL(ASMPFCFG_ADDR))
#define	ASMPFRDY_VAL        	(REG32_VAL(ASMPFRDY_ADDR))
#define	ASMPFBS_TYPE_VAL        (REG32_VAL(ASMPFBS_TYPE_ADDR))
#define	ASMPFCHCFG_VAL        	(REG32_VAL(ASMPFCHCFG_ADDR))
#define	AUDPRFRST_VAL        	(REG32_VAL(AUDPRFRST_ADDR))
#define	AADCFOBDOUT_DMA_VAL		(REG32_VAL(AADCFOBDOUT_DMA_ADDR))
#define	ASMPFDP_DMA_VAL			(REG32_VAL(ASMPFDP_DMA_ADDR))
#define	DZDRQ8_CFG_VAL			(REG32_VAL(DZDRQ8_CFG_ADDR))
#define	DZDRQ9_CFG_VAL			(REG32_VAL(DZDRQ9_CFG_ADDR))
#define	DZDRQA_CFG_VAL			(REG32_VAL(DZDRQA_CFG_ADDR))
#define	DGOCS0A_VAL			(REG32_VAL(DGOCS0A_ADDR))
#define	DGOCS1A_VAL			(REG32_VAL(DGOCS1A_ADDR))


/******************************************************************************
 *
 * 
 *
 ******************************************************************************/
#define DACITF_ENABLE				BIT22			/* DAC interface enable */
#define ADCITF_ENABLE				BIT23			/* ADC interface enable */
#define ASMPF_ENABLE				BIT6			/* sample FIFO enable */
#define ASMPF16_ENABLE				BIT7			/* sample FIFO 16-bits enable */
#define AADCF_ENABLE				BIT0			/* ADC FIFO enable */
#define AADCF16_ENABLE				BIT1			/* ADC FIFO 16-bits enable */
#define DGOITF_ENABLE				BIT7			/* ADGO(SPDIF-out) interface enable */
#define ADGIF16_ENABLE				BIT14			/* ADGI FIFO 16-bits enable */
#define ADGIITF_ENABLE				BIT1			/* ADGI(SPDIF-in) interface enable */
#define ADGI_EXTRACTOR_ENABLE		BIT0			/* ADGI-Extractor enable */

#define ASMPF_RESET				BIT1			/* sample FIFO reset */
#define DACITF_RESET				BIT2			/* DAC interface reset */
#define ADCITF_RESET				BIT3			/* ADC interface & ADC FIFO reset */
#define DGOITF_RESET				BIT4			/* SPDIF out reset */
#define ADGI_EXTRACTOR_RESET		BIT5			/* SPDIF in reset */
#define ADGI_FIFO_RESET			BIT6			/* SPDIF in reset */

#define HDACKGEN_ENABLE			BIT4			/* HDAudio Reference Clock enable */


struct i2s_ints_s {
	/* Tx FIFO Status. */
	unsigned int tfoe;      /* Tx FIFO Overrun Error */
	unsigned int tfue;      /* Tx FIFO Underrun Error */
	unsigned int tfa;       /* Tx FIFO Almost Empty */
	unsigned int tfe;       /* Tx FIFO Empty */

	/* Tx FIFO Status. */
	unsigned int rfoe;      /* Tx FIFO Overrun Error */
	unsigned int rfue;      /* Tx FIFO Underrun Error */
	unsigned int rfa;       /* Tx FIFO Almost Full */
	unsigned int rff;       /* Tx FIFO Full */

};

struct i2s_s {
	/* Interrupt status counters.*/
	struct i2s_ints_s             ints;
	/* I2S Controller info. */
	const unsigned int      irq;            /* I2S controller irq*/
	unsigned int            ref;            /* I2S reference counter*/
	unsigned int			channels;
	int						format;
	unsigned int			fragment_sz;
	unsigned int			rate;
	/* Basic handlers.*/
	void                    (*init)(int mode);
	void                    (*exit)(void);
};

#endif /* __WMT_I2S_H */
