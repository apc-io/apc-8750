/*++
linux/include/asm-arm/arch-wmt/wmt_mmap.h

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


#ifndef __WMT_MMAP_H
#define __WMT_MMAP_H

/**
 * WMT Memory Map for Physical Address 0xD8000000 will be mapped to 
 * Virtual Address 0xFE000000
 */
#define WMT_MMAP_OFFSET                       0

#define EXTERNAL_AHB_BRIDGE_BASE_ADDR           0xB0000000
#define INTERNAL_AHB_SLAVES_BASE_ADDR           (0xD8000000 + WMT_MMAP_OFFSET)
#define INTERNAL_APB_SLAVES_BASE_ADDR           (0xD8100000 + WMT_MMAP_OFFSET)

/**
 *  Internal AHB Slaves Memory Address Map
 */
#define MEMORY_CTRL_V3_CFG_BASE_ADDR            (0xD8000000 + WMT_MMAP_OFFSET)  /* 1K , 8/16/32 RW */
#define MEMORY_CTRL_V4_CFG_BASE_ADDR            (0xD8000400 + WMT_MMAP_OFFSET)  /* 1K , 8/16/32 RW */
#define DMA_CTRL0_V3_CFG_BASE_ADDR              (0xD8001000 + WMT_MMAP_OFFSET)  /* 1K , 8/16/32 RW */
#define DMA_CTRL1_V3_CFG_BASE_ADDR              (0xD8001400 + WMT_MMAP_OFFSET)  /* 1K , 8/16/32 RW */
#define DMA_CTRL_V4_CFG_BASE_ADDR               (0xD8001800 + WMT_MMAP_OFFSET)  /* 1K , 8/16/32 RW */
#define PICTOR_DMA_CTRL_CFG_BASE_ADDR           (0xD8001C00 + WMT_MMAP_OFFSET)  /* 1K , 8/16/32 RW */
#define SF_MEM_CTRL_CFG_BASE_ADDR               (0xD8002000 + WMT_MMAP_OFFSET)  /* 1K , 8/16/32 RW */
#define LPC_MEM_CTRL_CFG_BASE_ADDR              (0xD8003000 + WMT_MMAP_OFFSET)  /* 1K , 8/16/32 RW */
#define SPI_MEM_CTRL_CFG_BASE_ADDR              (0xD8003000 + WMT_MMAP_OFFSET)  /* 1K , 8/16/32 RW */

#define ETHERNET_MAC_0_CFG_BASE_ADDR            (0xD8004000 + WMT_MMAP_OFFSET)  /* 1K , 8/16/32 RW */

#define ETHERNET_MAC_1_CFG_BASE_ADDR            (0xD8005000 + WMT_MMAP_OFFSET)  /* 1K , 8/16/32 RW */

#define SECURITY_ENGINE_CFG_BASE_ADDR           (0xD8006000 + WMT_MMAP_OFFSET)  /* 1K , 8/16/32 RW */
#define SECURITY_ENGINE_CFG_EXTENT_BASE_ADDR    (0xD8006400 + WMT_MMAP_OFFSET)  /* 3K , 8/16/32 RW */
#define USB20_HOST_CFG_BASE_ADDR                (0xD8007000 + WMT_MMAP_OFFSET)  /* 2K , 8/16/32 RW */
#define USB20_HOST_DEVICE_CFG_BASE_ADDR         (0xD8007800 + WMT_MMAP_OFFSET)  /* 2K , 8/16/32 RW */
#define PATA_CTRL_CFG_BASE_ADDR                 (0xD8008000 + WMT_MMAP_OFFSET)  /* 2K , 8/16/32 RW */
#define PS2_CFG_BASE_ADDR                       (0xD8008800 + WMT_MMAP_OFFSET)  /* 1K , 8/16/32 RW */
#define USB20_HOST_CFG_EXTENT_BASE_ADDR         (0xD8008C00 + WMT_MMAP_OFFSET)  /* 1K , 8/16/32 RW */
#define NF_CTRL_CFG_BASE_ADDR                   (0xD8009000 + WMT_MMAP_OFFSET)  /* 1K , 8/16/32 RW */
#define NOR_CTRL_CFG_BASE_ADDR                  (0xD8009400 + WMT_MMAP_OFFSET)  /* 1K , 8/16/32 RW */
#define USB20_DEVICE_CFG_BASE_ADDR              (0xD8009800 + WMT_MMAP_OFFSET)  /* 2K , 8/16/32 RW */
#define SD0_SDIO_MMC_BASE_ADDR                  (0xD800A000 + WMT_MMAP_OFFSET)  /* 1K , 8/16/32 RW */
#define SD1_SDIO_MMC_BASE_ADDR                  (0xD800A400 + WMT_MMAP_OFFSET)  /* 1K , 8/16/32 RW */

#define MS_CTRL_CFG_BASE_ADDR                   (0xD800B000 + WMT_MMAP_OFFSET)  /* 1K , 8/16/32 RW */
#define XD_CTRL_CFG_BASE_ADDR                   (0xD800B400 + WMT_MMAP_OFFSET)  /* 1K , 8/16/32 RW */

#define CF_CTRL_CFG_BASE_ADDR                   (0xD800C000 + WMT_MMAP_OFFSET)  /* 1K , 8/16/32 RW */

#define SATA_CTRL_CFG_BASE_ADDR                 (0xD800D000 + WMT_MMAP_OFFSET)  /* 2K , 8/16/32 RW */

#define XOR_CTRL_CFG_BASE_ADDR                  (0xD800E000 + WMT_MMAP_OFFSET)  /* 1K , 8/16/32 RW */
#define LCD_CTRL_CFG_BASE_ADDR                  (0xD800E400 + WMT_MMAP_OFFSET)  /* 1K , 8/16/32 RW */

#define ASYNC_APB_BRIDGE_BASE_ADDR              (0xD802FC00 + WMT_MMAP_OFFSET)  /* 1K , 8/16/32 RW */
#define LPC_TPM_CFG_BASE_ADDR                   (0xD8030000 + WMT_MMAP_OFFSET)  /* 64K , 8/16/32 RW */
#define LPC_SUPERIO_CFG_BASE_ADDR               (0xD8040000 + WMT_MMAP_OFFSET)  /* 64K , 8/16/32 RW */

#define VPU_BASE_ADDR                           (0xD8050100 + WMT_MMAP_OFFSET)  /* 256 , 8/16/32 RW */
#define VPU_BASE2_ADDR                          (0xD8050200 + WMT_MMAP_OFFSET)  /* 256 , 8/16/32 RW */
#define SPU1_BASE_ADDR                          (0xD8050100 + WMT_MMAP_OFFSET)  /* 256 , 8/16/32 RW */
#define SPU2_BASE_ADDR                          (0xD8050200 + WMT_MMAP_OFFSET)  /* 256 , 8/16/32 RW */
#define GOVM_BASE_ADDR                          (0xD8050300 + WMT_MMAP_OFFSET)  /* 256 , 8/16/32 RW */
#define GE1_BASE_ADDR                           (0xD8050400 + WMT_MMAP_OFFSET)  /* 256 , 8/16/32 RW */
#define GE2_BASE_ADDR                           (0xD8050500 + WMT_MMAP_OFFSET)  /* 256 , 8/16/32 RW */
#define GE3_BASE_ADDR                           (0xD8050600 + WMT_MMAP_OFFSET)  /* 256 , 8/16/32 RW */
#define DISP_BASE_ADDR                          (0xD8050700 + WMT_MMAP_OFFSET)  /* 256 , 8/16/32 RW */
#define GOVRH_BASE1_ADDR                        (0xD8050800 + WMT_MMAP_OFFSET)  /* 256 , 8/16/32 RW */
#define GOVRH_BASE2_ADDR                        (0xD8050900 + WMT_MMAP_OFFSET)  /* 256 , 8/16/32 RW */
#define VID_BASE_ADDR                           (0xD8050A00 + WMT_MMAP_OFFSET)  /* 256 , 8/16/32 RW */
#define HDTV_CTRL_BASE_ADDR                     (0xD8050B00 + WMT_MMAP_OFFSET)  /* 256 , 8/16/32 RW */
#define GOVW_BASE_ADDR                          (0xD8050C00 + WMT_MMAP_OFFSET)  /* 256 , 8/16/32 RW */
#define SCL_BASE_ADDR                           (0xD8050D00 + WMT_MMAP_OFFSET)  /* 256 , 8/16/32 RW */
#define SCL_BASE2_ADDR                          (0xD8050000 + WMT_MMAP_OFFSET)  /* 256 , 8/16/32 RW */
#define DISP2_BASE_ADDR                         (0xD8050E00 + WMT_MMAP_OFFSET)  /* 256 , 8/16/32 RW */
#define VPP_BASE_ADDR                           (0xD8050F00 + WMT_MMAP_OFFSET)  /* 256 , 8/16/32 RW */
#define LVDS_BASE_ADDR							(0xD8051000 + WMT_MMAP_OFFSET)  /* 256 , 8/16/32 RW */

#define HDMI_TRANSMITTE_BASE_ADDR               (0xD8060000 + WMT_MMAP_OFFSET)  /* 64K , 8/16/32 RW */
#define HDMI_CP_BASE_ADDR                          (0xD8070000 + WMT_MMAP_OFFSET)  /* 64K , 8/16/32 RW */

#define USB2_OTG_CFG_BASE_ADDR                  (0xD80E4000 + WMT_MMAP_OFFSET)  /* 16K , 8/16/32 RW */

#define AUDREGF_BASE_ADDR                       (0xD80ED800 + WMT_MMAP_OFFSET)  /* 1K , 8/16/32 RW */
#define PART_OF_AUDREGF_BASE_ADDR               (0xD80EDC00 + WMT_MMAP_OFFSET)  /* 1K , 8/16/32 RW */
#define DSS_MBOX_BASE_ADDR                      (0xD80EE000 + WMT_MMAP_OFFSET)  /* 1K , 8/16/32 RW */
#define DSS_PERM_BASE_ADDR                      (0xD80EE400 + WMT_MMAP_OFFSET)  /* 1K , 8/16/32 RW */

#define VDU_BASE_ADDR                           (0xD80F0000 + WMT_MMAP_OFFSET)  /* 1K , 8/16/32 RW */
#define SCRCNT_BASE_ADDR                        (0xD80F0400 + WMT_MMAP_OFFSET)  /* 256 , 8/16/32 RW */

#define VLDBUF_BASE_ADDR                        (0xD80F1000 + WMT_MMAP_OFFSET)  /* 1K , 8/16/32 RW */
#define IQ_BASE_ADDR                            (0xD80F1400 + WMT_MMAP_OFFSET)  /* 1K , 8/16/32 RW */
#define IDCT_BASE_ADDR                          (0xD80F1800 + WMT_MMAP_OFFSET)  /* 1K , 8/16/32 RW */

#define STREAMIN_BASE_ADDR                      (0xD80F2000 + WMT_MMAP_OFFSET)  /* 1K , 8/16/32 RW */

#define MSVD_BASE_ADDR                          (0xD80F3000 + WMT_MMAP_OFFSET)  /* 1K , 8/16/32 RW */
#define AVBO_FIFO_BASE_ADDR                     (0xD80F3400 + WMT_MMAP_OFFSET)  /* 1K , 8/16/32 RW */
#define SPU_FIFO_BASE_ADDR                      (0xD80F3800 + WMT_MMAP_OFFSET)  /* 1K , 8/16/32 RW */

#define TSIN0_BASE_ADDR                         (0xD80F4000 + WMT_MMAP_OFFSET)  /* 4K , 8/16/32 RW */
#define TSIN1_BASE_ADDR                         (0xD80F5000 + WMT_MMAP_OFFSET)  /* 4K , 8/16/32 RW */
#define TSIN2_BASE_ADDR                         (0xD80F6000 + WMT_MMAP_OFFSET)  /* 4K , 8/16/32 RW */
#define TSOUT_BASE_ADDR                         (0xD80F7000 + WMT_MMAP_OFFSET)  /* 4K , 8/16/32 RW */
#define H264_DECODER_BASE_ADDR                  (0xD80F8000 + WMT_MMAP_OFFSET)  /* 16K , 8/16/32 RW */

#define CNM_BIT_BASE_ADDR                       (0xD80FC000 + WMT_MMAP_OFFSET)  /* 4K , 8/16/32 RW */

#define JPEG_DECODER_BASE_ADDR                  (0xD80FE000 + WMT_MMAP_OFFSET)  /* 4K , 8/16/32 RW */
#define JPEG_ENCODER_BASE_ADDR                  (0xD80FF000 + WMT_MMAP_OFFSET)  /* 4K , 8/16/32 RW */

/**
 *  Internal APB Slaves Memory Address Map
 */
#define RTC_BASE_ADDR                           (0xD8100000 + WMT_MMAP_OFFSET)  /* 64K  */
#define GPIO_BASE_ADDR                          (0xD8110000 + WMT_MMAP_OFFSET)  /* 64K  */
#define SYSTEM_CFG_CTRL_BASE_ADDR               (0xD8120000 + WMT_MMAP_OFFSET)  /* 64K  */
#define PM_CTRL_BASE_ADDR                       (0xD8130000 + WMT_MMAP_OFFSET)  /* 64K  */
#define INTERRUPT0_CTRL_BASE_ADDR               (0xD8140000 + WMT_MMAP_OFFSET)  /* 64K  */
#define INTERRUPT1_CTRL_BASE_ADDR               (0xD8150000 + WMT_MMAP_OFFSET)  /* 64K  */

#define AUDIO_CODEC_BASE_ADDR                   (0xD81F0000 + WMT_MMAP_OFFSET)  /* 64K  */
#define UART0_BASE_ADDR                         (0xD8200000 + WMT_MMAP_OFFSET)  /* 64K  */
#define UART1_BASE_ADDR                         (0xD82b0000 + WMT_MMAP_OFFSET)  /* 64K  */
#define UART2_BASE_ADDR                         (0xD8210000 + WMT_MMAP_OFFSET)  /* 64K  */
#define UART3_BASE_ADDR                         (0xD82c0000 + WMT_MMAP_OFFSET)  /* 64K  */
#define UART4_BASE_ADDR                         (0xD8370000 + WMT_MMAP_OFFSET)  /* 64K  */
#define UART5_BASE_ADDR                         (0xD8380000 + WMT_MMAP_OFFSET)  /* 64K  */
#define PWM0_BASE_ADDR                          (0xD8220000 + WMT_MMAP_OFFSET)  /* 64K  */

#define SPI0_BASE_ADDR                          (0xD8240000 + WMT_MMAP_OFFSET)  /* 64K  */
#define SPI1_BASE_ADDR                          (0xD8250000 + WMT_MMAP_OFFSET)  /* 64K  */
#define SPI2_BASE_ADDR                          (0xD82A0000 + WMT_MMAP_OFFSET)  /* 64K  */
#define KPAD_BASE_ADDR                          (0xD8260000 + WMT_MMAP_OFFSET)  /* 64K  */
#define CIR_BASE_ADDR                           (0xD8270000 + WMT_MMAP_OFFSET)  /* 64K  */
#define I2C0_BASE_ADDR                          (0xD8280000 + WMT_MMAP_OFFSET)  /* 64K  */
#define I2C1_BASE_ADDR                          (0xD8320000 + WMT_MMAP_OFFSET)  /* 64K  */
#define PCM_BASE_ADDR                           (0xD82D0000 + WMT_MMAP_OFFSET)  /* 64K  */
#define AC97_BASE_ADDR                          (0xD8290000 + WMT_MMAP_OFFSET)  /* 64K  */

#define AHB_ACCESS_MONITOR0_BASE_ADDR           (0xD82E0000 + WMT_MMAP_OFFSET)
#define AHB_ACCESS_MONITOR1_BASE_ADDR           (0xD82F0000 + WMT_MMAP_OFFSET)
#define AHB_ACCESS_MONITOR2_BASE_ADDR           (0xD8300000 + WMT_MMAP_OFFSET)
#define AHB_ACCESS_MONITOR3_BASE_ADDR           (0xD8310000 + WMT_MMAP_OFFSET)

#define ADC_BASE_ADDR                           (0xD8340000 + WMT_MMAP_OFFSET)  /* 64K  */
#define ROTARY_DETECTOR_BASE_ADDR               (0xD8350000 + WMT_MMAP_OFFSET)  /* 64K  */
#define SMART_CARD_INTERFACE_BASE_ADDR          (0xD8360000 + WMT_MMAP_OFFSET)  /* 64K  */
#define POWER_MOS_BASE_ADDR                     (0xD8390000 + WMT_MMAP_OFFSET)  /* 64K  */
// check
#define MEMORY_CTRL_CFG_BASE_ADDR               MEMORY_CTRL_V3_CFG_BASE_ADDR
#define DMA_CTRL_CFG_BASE_ADDR                  DMA_CTRL_V4_CFG_BASE_ADDR
#define LPC_CTRL_CFG_BASE_ADDR                  LPC_SUPERIO_CFG_BASE_ADDR
#define HDMI1_BASE_ADDR                         (0xD806C000 + WMT_MMAP_OFFSET)
#define HDMI2_BASE_ADDR                         (0xD8070000 + WMT_MMAP_OFFSET)
#define GOVR_BASE_ADDR                          (0xD8050B00 + WMT_MMAP_OFFSET)
#define INTERRUPT_CTRL_BASE_ADDR                INTERRUPT0_CTRL_BASE_ADDR
#define SPI_BASE_ADDR                           SPI0_BASE_ADDR
#define I2C_BASE_ADDR                           I2C0_BASE_ADDR
#define I2S_BASE_ADDR                           AUDREGF_BASE_ADDR


/* WMT Memory Map for Physical Address*/
#define UART0_PHY_BASE_ADDR                     0xD8200000  /* 64K  */
#define UART1_PHY_BASE_ADDR                     0xD82b0000  /* 64K  */

#endif	/* __WMT_MMAP_H */
