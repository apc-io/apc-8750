/*++
linux/include/asm-arm/arch-wmt/ac97.h

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

#ifndef __AC97_CODEC
#define __AC97_CODEC

/*
 * AC'97 registers set structure.
 */
struct ac97_regs_s {
	unsigned int volatile ACGC;
	unsigned int volatile ACGS;
	unsigned int volatile CCSL;
	unsigned int volatile CCMD;
	unsigned int volatile CRSD;
	unsigned int volatile REV0[(0x0020-0x0014)/4];  /* 3 words reserved  */
	unsigned int volatile PTFC;
	unsigned int volatile PTFS;
	unsigned int volatile PRFC;
	unsigned int volatile PRFS;
	unsigned int volatile MIFC;
	unsigned int volatile MIFS;
	/*
	 * Due to issue of read FIFO, following section should not use.
	 */
/*        unsigned int volatile REV1[(0x0080-0x0038)/4];   18 words reserved */ 
/*        unsigned int volatile PTFP[(0x00C0-0x0080)/4];   16 words FIFO     */ 
/*        unsigned int volatile PRFP[(0x0100-0x00C0)/4];   16 words FIFO     */ 
/*        unsigned int volatile MIRP[(0x0140-0x0100)/4];   16 words FIFO     */ 
};

/*
 * AC'97 interrupt event counters.
 */
struct ac97_ints_s {
	/*
	 * Global Status.
	 */
	unsigned int crdy;      /* CODEC Ready Interrupt */
	unsigned int cwd;       /* CODEC Write Done Interrupt */
	unsigned int crd;       /* CODEC Read Done Interrupt */
	unsigned int cst;       /* CODEC Status Timeout */

	/*
	 * PCM Tx FIFO Status.
	 */
	unsigned int ptfe;      /* PCM Tx FIFO Empty */
	unsigned int ptfa;      /* PCM Tx FIFO Almost Empty */
	unsigned int ptfue;     /* PCM Tx FIFO Underrun Error */
	unsigned int ptfoe;     /* PCM Tx FIFO Overrun Error */

	/*
	 * PCM Rx FIFO Status.
	 */
	unsigned int prff;      /* PCM Rx FIFO Full */
	unsigned int prfa;      /* PCM Rx FIFO Almost Full */
	unsigned int prfue;     /* PCM Rx FIFO Underrun Error */
	unsigned int prfoe;     /* PCM Rx FIFO Overrun Error */

	/*
	 * Mic FIFO Status.
	 */
	unsigned int mff;       /* Mic FIFO Full */
	unsigned int mfa;       /* Mic FIFO Almost Full */
	unsigned int mfue;      /* Mic FIFO Underrun Error */
	unsigned int mfoe;      /* Mic FIFO Overrun Error */

};

typedef struct ac97_s {
	/*
	 * AC'97 Controller regsiter set.
	 */
	struct ac97_regs_s *const regs;    /* Register set             */

	/*
	 * Interrupt status counters.
	 */
	struct ac97_ints_s ints;

	/*
	 * AC'97 Controller info.
	 */
	const unsigned int irq;      /* AC'97 controller irq     */
	unsigned int ref;            /* AC'97 reference counter  */

	/*
	 * Basic handlers.
	 */
	void (*init)(void);
	void (*exit)(void);

} ac97_t;

/*
 * CODEC status flags.
 */
#define CODEC_INITED            (1 << 0)

struct codec_api_s {
	void	    (*release)(void);
	void	    (*volume)(unsigned short val, unsigned short mute);
	void	    (*bass)(int val);
	void	    (*treble)(int val);
	void	    (*line)(unsigned short val, unsigned short mute);
	void	    (*mic_wr)(unsigned short val, unsigned short mute);
	void	    (*recsrc_wr)(unsigned int val);
	void	    (*mic_rd)(unsigned short *val);
	void	    (*recsrc_rd)(unsigned short *val);
	void	    (*set_adc)(unsigned short val);
	void	    (*set_dac)(unsigned short val);
	void	    (*init)(void);
	int		    (*read_id)(u16 *data1, u16 *data2);
	int		    (*enable)(u16 *data);
	int		    (*disable)(u16 *data);
	void	    (*reg_backup)(void);
	void	    (*reg_restore)(void);
};

struct codec_ops_s {
	/* AC-link status */
	int         (*aclink)(void);
	/* Generic operations */
	int	        (*init)(void);
	void	    (*exit)(void);
	int	        (*enable)(void);
	int	        (*disable)(void);
};

struct codec_s {
	struct codec_ops_s *ops;
	struct codec_api_s *api;
	/* IDs */
	unsigned short  id1;
	unsigned short  id2;
	/* audio generic properties */
	unsigned short  volume;
	unsigned short  bass;
	unsigned short  treble;
	unsigned short  line;
	unsigned short  mic;
	/* modified counter for mixer_ioctl. */
	unsigned int    mod;
	/* IRQ number. */
	unsigned int    irq;
	unsigned int 	recsrc;
	/* Codec State */
	unsigned char	active;
};

extern struct codec_s *codec_attach(void);
extern void codec_detach(struct codec_s *codec);
extern void ac97_reg_backup(void);
extern void ac97_reg_restore(void);


#endif  /* __AC97_CODEC */
