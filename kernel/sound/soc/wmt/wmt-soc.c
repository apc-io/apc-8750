/*++ 
 * linux/sound/soc/wmt/wmt-soc.c
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


#include <linux/clk.h>
#include <linux/platform_device.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/hwdep.h>


#include <asm/mach-types.h>
#include <mach/hardware.h>
#include <mach/gpio.h>

#include "wmt-soc.h"
#include "wmt-pcm.h"
#include "../codecs/wmt_vt1602.h"
#include "../codecs/wmt_hwdac.h"
#include "../codecs/wm8900.h"
#include "../codecs/vt1603.h"
extern int wmt_getsyspara(char *varname, unsigned char *varval, int *varlen);
char wmt_codec_name[80];
char wmt_dai_name[80];
char wmt_rate[10];
int WFD_flag = 0;

/*
 * Debug
 */
#define AUDIO_NAME "WMT_SOC"

//#define WMT_SOC_DEBUG 1
//#define WMT_SOC_DEBUG_DETAIL 1

#ifdef WMT_SOC_DEBUG
#define DPRINTK(format, arg...) \
	printk(KERN_INFO AUDIO_NAME ": " format "\n" , ## arg)
#else
#define DPRINTK(format, arg...) do {} while (0)
#endif

#ifdef WMT_SOC_DEBUG_DETAIL
#define DBG_DETAIL(format, arg...) \
	printk(KERN_INFO AUDIO_NAME ": [%s]" format "\n" , __FUNCTION__, ## arg)
#else
#define DBG_DETAIL(format, arg...) do {} while (0)
#endif

#define err(format, arg...) \
	printk(KERN_ERR AUDIO_NAME ": " format "\n" , ## arg)
#define info(format, arg...) \
	printk(KERN_INFO AUDIO_NAME ": " format "\n" , ## arg)
#define warn(format, arg...) \
	printk(KERN_WARNING AUDIO_NAME ": " format "\n" , ## arg)

#define WMT_I2S_RATES		(SNDRV_PCM_RATE_44100 | \
				 SNDRV_PCM_RATE_22050 | \
				 SNDRV_PCM_RATE_11025 | \
				 SNDRV_PCM_RATE_48000 | \
				 SNDRV_PCM_RATE_96000 | \
				 SNDRV_PCM_RATE_88200 | \
				 SNDRV_PCM_RATE_32000 | \
				 SNDRV_PCM_RATE_8000 | \
				 SNDRV_PCM_RATE_16000 | \
				 SNDRV_PCM_RATE_KNOT)


static int wmt_soc_startup(struct snd_pcm_substream *substream)
{
	DBG_DETAIL();
	
	return 0;
}

static void wmt_soc_shutdown(struct snd_pcm_substream *substream)
{
	DBG_DETAIL();
}

static int wmt_soc_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->dai->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->dai->cpu_dai;
	int err = 0;

	DBG_DETAIL();
	
	if (!strcmp(wmt_dai_name, "i2s")) {
		if (strcmp(wmt_codec_name, "hwdac")) {
			/* Set codec DAI configuration */
			err = snd_soc_dai_set_fmt(codec_dai, SND_SOC_DAIFMT_CBS_CFS |
					 SND_SOC_DAIFMT_I2S);
		}
		if (err < 0)
			return err;

		/* Set cpu DAI configuration for I2S */
		err = snd_soc_dai_set_fmt(cpu_dai, SND_SOC_DAIFMT_I2S);
	}
	else {
		/* Set cpu DAI configuration for AC97 */
		err = snd_soc_dai_set_fmt(cpu_dai, SND_SOC_DAIFMT_AC97);
	}

	if (err < 0)
		return err;

	if ((!strcmp(wmt_dai_name, "i2s")) &&
		((!strcmp(wmt_codec_name, "vt1602")) || (!strcmp(wmt_codec_name, "vt1603")))) {
		/* Set the codec system clock for DAC and ADC */
		if (!(params_rate(params) % 11025))
			err = snd_soc_dai_set_sysclk(codec_dai, 0, 11289600,
						    SND_SOC_CLOCK_IN);
		else
			err = snd_soc_dai_set_sysclk(codec_dai, 0, 12288000,
						    SND_SOC_CLOCK_IN);
	}

	return err;
}

static struct snd_soc_ops wmt_soc_ops = {
	.startup = wmt_soc_startup,
	.hw_params = wmt_soc_hw_params,
	.shutdown = wmt_soc_shutdown,
};

static int wmt_hwdep_open(struct snd_hwdep *hw, struct file *file)
{
	/*info("wmt_hwdep_open");*/

	if ((file->f_flags & O_RDWR) && (WFD_flag)) {
		return -EBUSY;
	}
	else if (file->f_flags & O_SYNC) {
		WFD_flag = 1;
	}	
	
	return 0;
}

static int wmt_hwdep_release(struct snd_hwdep *hw, struct file *file)
{
	/*info("wmt_hwdep_release");*/
	WFD_flag = 0;
	return 0;
}

static int wmt_hwdep_mmap(struct snd_hwdep *hw, struct file *file, struct vm_area_struct *vma)
{
	vma->vm_flags |= VM_IO | VM_RESERVED;
	vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
			
	if (remap_pfn_range(vma, vma->vm_start, (vma->vm_pgoff),
			     vma->vm_end - vma->vm_start, vma->vm_page_prot)) {
       	err("*E* remap page range failed: vm_pgoff=0x%x ", (unsigned int)vma->vm_pgoff);
       	return -EAGAIN;    
    }

    return 0;
}

static int wmt_hwdep_ioctl(struct snd_hwdep *hw, struct file *file, unsigned int cmd, unsigned long arg)
{
	int *value;
	WFDStrmInfo_t *info;
	
	switch (cmd) {
	case WMT_SOC_IOCTL_HDMI:
		value = (int __user *)arg;
		
		if (*value > 1) {
			err("Not supported status for HDMI Audio %d", *value);
			return 0;
		}	
		wmt_i2s_dac0_ctrl(*value);
		return 0;
		
	case WMT_SOC_IOCTL_WFD_START:
		return copy_to_user( (void *)arg, (const void __user *) wmt_pcm_wfd_get_buf(), sizeof(unsigned int));
		
	case WMT_SOC_IOCTL_GET_STRM:
		info = (WFDStrmInfo_t *)wmt_pcm_wfd_get_strm((WFDStrmInfo_t *)arg);
		return __put_user((int)info, (unsigned int __user *) arg);

	case WMT_SOC_IOCTL_WFD_STOP:
		wmt_pcm_wfd_stop();
		return 0;
	}
	
	err("Not supported ioctl for WMT-HWDEP");
	return -ENOIOCTLCMD;
}

static void wmt_soc_hwdep_new(struct snd_soc_codec *codec)
{
	struct snd_hwdep *hwdep;
	
	DBG_DETAIL();

	if (snd_hwdep_new(codec->card, "WMT-HWDEP", 0, &hwdep) < 0) {
		err("create WMT-HWDEP fail");
		return;
	}

	sprintf(hwdep->name, "WMT-HWDEP %d", 0);

	info("create %s success", hwdep->name);
	
	hwdep->iface = SNDRV_HWDEP_IFACE_WMT;
	hwdep->ops.open = wmt_hwdep_open;
	hwdep->ops.ioctl = wmt_hwdep_ioctl;
	hwdep->ops.release = wmt_hwdep_release;
	hwdep->ops.mmap = wmt_hwdep_mmap;

	return;
}

static int wmt_soc_dai_init(struct snd_soc_codec *codec)
{
	DBG_DETAIL();

	wmt_soc_hwdep_new(codec);

	return 0;
}

/* Digital audio interface glue - connects codec <--> CPU */
static struct snd_soc_dai_link wmt_dai = {
	.name = "WMT_SOC_AUDIO",
	.stream_name = "WMT_SOC_AUDIO",
	.init = wmt_soc_dai_init,
	.ops = &wmt_soc_ops,
};

static int wmt_suspend_post(struct platform_device *pdev, pm_message_t state)
{
	DBG_DETAIL();

	if (!strcmp(wmt_dai_name, "i2s")) {
		/* Disable BIT15:I2S clock, BIT4:ARFP clock, BIT3:ARF clock */
		PMCEU_VAL &= ~(BIT15 | BIT4 | BIT3);
	}

	return 0;
}

static int wmt_resume_pre(struct platform_device *pdev)
{
	//unsigned int clock = 0;
	
	DBG_DETAIL();
	
	if (!strcmp(wmt_dai_name, "i2s")) {
		/*
		 *  Enable MCLK before VT1602 codec enable,
		 *  otherwise the codec will be disabled.
		 */
		/* set to 11.288MHz */
		auto_pll_divisor(DEV_I2S, CLK_ENABLE , 0, 0);
		auto_pll_divisor(DEV_I2S, SET_PLLDIV, 1, 11288);
		/*clock = auto_pll_divisor(DEV_I2S, GET_FREQ , 0, 0);
		info("%s : clock=%d \n" , __func__, clock);*/

		/* Enable BIT4:ARFP clock, BIT3:ARF clock */
		PMCEU_VAL |= (BIT4 | BIT3);

		/* Enable BIT2:AUD clock */
		PMCE3_VAL |= BIT2;

		/* disable GPIO and Pull Down mode */
		GPIO_CTRL_GP10_I2S_BYTE_VAL &= ~0xFF;
		GPIO_CTRL_GP27_SPDIF_BYTE_VAL &= ~(BIT0 | BIT1);

		GPIO_PULL_EN_GP10_I2S_BYTE_VAL &= ~0xFF;
		GPIO_PULL_EN_GP27_SPDIF_BYTE_VAL &= ~(BIT0 | BIT1);

		/* set to i2s in mode: 2ch input, 2ch output */
		GPIO_PIN_SHARING_SEL_4BYTE_VAL &= ~(BIT0);
		GPIO_PIN_SHARING_SEL_4BYTE_VAL |= (BIT2 | BIT3);
	}

	return 0;
}

/* Audio machine driver */
static struct snd_soc_card snd_soc_machine_wmt = {
	.name = "WMT_SOC",
	.platform = &wmt_soc_platform,
	.dai_link = &wmt_dai,
	.num_links = 1,
	.suspend_post = wmt_suspend_post,
	.resume_pre = wmt_resume_pre,
};

/* Audio private data */
static struct vt1602_setup_data wmt_soc_codec_setup = {
	.i2c_bus = 1,
	.i2c_address = 0x1a,
};

/* Audio subsystem */
static struct snd_soc_device wmt_snd_devdata = {
	.card = &snd_soc_machine_wmt,
};

static struct platform_device *wmt_snd_device;

static int __init wmt_soc_init(void)
{
	int err;
	char buf[80];
	char varname[80];
	int varlen = 80;
	int i = 0;
	unsigned int wmt_i2s_rate = 0;

	DBG_DETAIL();

	/* Read u-boot parameter to decide wmt_dai_name and wmt_codec_name */
	strcpy(varname, "wmt.audio.i2s");
	if (wmt_getsyspara(varname, buf, &varlen) != 0) {
		strcpy(wmt_dai_name, "null");	
		strcpy(wmt_codec_name, "null");
	}
	else {
		strcpy(wmt_dai_name, "i2s");	
	}

	if (strcmp(wmt_dai_name, "null")) {
		for (i = 0; i < 80; ++i) {
			if (buf[i] == ':')
				break;
			else
				wmt_codec_name[i] = buf[i];
		}
	}
	else {
#ifdef CONFIG_SND_WMT_SOC_I2S
		strcpy(wmt_dai_name, "i2s");	
#endif
#ifdef CONFIG_I2S_HW_DAC
		strcpy(wmt_codec_name, "hwdac");
#endif
#ifdef CONFIG_I2S_CODEC_VT1602
		strcpy(wmt_codec_name, "vt1602");
#endif
#ifdef CONFIG_I2S_CODEC_VT1603
		strcpy(wmt_codec_name, "vt1603");
#endif
#ifdef CONFIG_I2S_CODEC_WM8900
		strcpy(wmt_codec_name, "wm8900");
#endif
	}

	info("dai_name=%s, codec_name=%s", wmt_dai_name, wmt_codec_name);

	/* Read u-boot parameter to decide which sample rate we're supported */
	memset(buf, 0, sizeof(buf));
	strcpy(varname, "wmt.audio.rate");
	if (wmt_getsyspara(varname, buf, &varlen) != 0) {
		strcpy(wmt_rate, "single");	
	}
	else {
		for (i = 0; i < 10; ++i) {
			if (buf[i] == ':')
				break;
			else
				wmt_rate[i] = buf[i];
		}
	}

	if ((strcmp(wmt_rate, "all")) && (strcmp(wmt_rate, "single"))) {
		info("wmt.audio.rate should be all or single, set to single and 48K as default");
		strcpy(wmt_rate, "single");
	}

	if (!strcmp(wmt_rate, "single")) {
		sscanf(buf, "single:%d", &wmt_i2s_rate);

		if ((!wmt_i2s_rate) || ((wmt_i2s_rate != 48000) && (wmt_i2s_rate != 44100))) {
			info("Wrong format for wmt.audio.rate, set to 48K");
			wmt_i2s_rate = 48000;
		}
		
		info("%s, wmt_i2s_rate=%d", wmt_rate, wmt_i2s_rate);

		if (wmt_i2s_rate == 48000)
			wmt_i2s_rate = SNDRV_PCM_RATE_48000;
		else
			wmt_i2s_rate = SNDRV_PCM_RATE_44100;
	}
	
	/* Plug-in dai and codec function depend on wmt_dai_name and wmt_codec_name */
	i = 0;
	if (!strcmp(wmt_dai_name, "i2s")) {
#ifdef CONFIG_SND_WMT_SOC_I2S
		if (!strcmp(wmt_rate, "all")) {
			wmt_i2s_dai.playback.rates = WMT_I2S_RATES;
			wmt_i2s_dai.capture.rates = WMT_I2S_RATES;
		}
		else {
			wmt_i2s_dai.playback.rates = wmt_i2s_rate;
			wmt_i2s_dai.capture.rates = wmt_i2s_rate;
		}
		
		wmt_dai.cpu_dai = &wmt_i2s_dai;
		i++;
#endif
	}
	
	if (!i) {
		strcpy(wmt_dai_name, "null");	
	}

	i = 0;

	if (!strcmp(wmt_codec_name, "vt1602")) {
#ifdef CONFIG_I2S_CODEC_VT1602		
		wmt_dai.codec_dai = &vt1602_dai;
		wmt_snd_devdata.codec_dev = &soc_codec_dev_vt1602;
		wmt_snd_devdata.codec_data = &wmt_soc_codec_setup;
		i++;
#endif		
	}
	else if (!strcmp(wmt_codec_name, "wm8900")) {
#ifdef CONFIG_I2S_CODEC_WM8900
		wmt_dai.codec_dai = &wm8900_dai;
		wmt_snd_devdata.codec_dev = &soc_codec_dev_wm8900;
		i++;
#endif		
	}	
	else if (!strcmp(wmt_codec_name, "hwdac")) {
#ifdef CONFIG_I2S_HW_DAC		
		wmt_dai.codec_dai = &hwdac_dai;
		wmt_snd_devdata.codec_dev = &soc_codec_dev_hwdac;
		wmt_snd_devdata.codec_data = &wmt_soc_codec_setup;
		i++;
#endif		
	}
	else if (!strcmp(wmt_codec_name, "vt1603")) {
#ifdef CONFIG_I2S_CODEC_VT1603
		wmt_dai.codec_dai = &vt1603_dai;
		wmt_snd_devdata.codec_dev = &soc_codec_dev_vt1603;
		i++;
#endif
	}
	
	if (!i) {
		strcpy(wmt_codec_name, "null");
	}

	if ((!strcmp(wmt_dai_name, "null")) || (!strcmp(wmt_codec_name, "null"))) {
		info("dai or codec name not matched!");
		return 0;
	}

	/* Doing register process after plug-in */
	wmt_snd_device = platform_device_alloc("soc-audio", -1);
	if (!wmt_snd_device)
		return -ENOMEM;

	platform_set_drvdata(wmt_snd_device, &wmt_snd_devdata);
	wmt_snd_devdata.dev = &wmt_snd_device->dev;
	
	*(unsigned int *)wmt_dai.cpu_dai->private_data = 1;
	
	err = platform_device_add(wmt_snd_device);
	if (err)
		goto err1;

	return 0;
err1:
	platform_device_put(wmt_snd_device);

	return err;

}

static void __exit wmt_soc_exit(void)
{
	DBG_DETAIL();
	
	platform_device_unregister(wmt_snd_device);
}

module_init(wmt_soc_init);
module_exit(wmt_soc_exit);

MODULE_AUTHOR("WonderMedia Technologies, Inc.");
MODULE_DESCRIPTION("WMT [ALSA SoC] driver");
MODULE_LICENSE("GPL");

