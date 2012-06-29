/*++ 
 * linux/sound/soc/wmt/wmt-pcm.h
 * WonderMedia audio driver for ALSA
 *
 * Copyright c 2010  WonderMedia  Technologies, Inc.
 *
 * This program is free software: you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation, either version 2 of the License, or 
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License for more details. 
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * WonderMedia Technologies, Inc.
 * 4F, 533, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C
--*/


#ifndef __OMAP_PCM_H__
#define __OMAP_PCM_H__

struct wmt_pcm_dma_data {
	char		*name;		/* stream identifier */
	int		dma_req;	/* DMA request line */
	unsigned long	port_addr;	/* transmit/receive register */
	struct dma_device_cfg_s *dma_cfg;
};

typedef struct WFDStrmInfo {
	unsigned int req_sz;
	unsigned int avail_sz;
	unsigned int buf_offset;
} WFDStrmInfo_t;


extern struct snd_soc_platform wmt_soc_platform;
extern unsigned int wmt_pcm_wfd_get_buf(void);
extern void wmt_pcm_wfd_stop(void);
extern int wmt_pcm_wfd_get_strm(WFDStrmInfo_t *info);


#endif
