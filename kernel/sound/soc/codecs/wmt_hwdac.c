/*++ 
 * linux/sound/soc/codecs/wmt_hwdac.c
 * WonderMedia I2S audio driver for ALSA
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


#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/initval.h>

#include "wmt_hwdac.h"

#define AUDIO_NAME "HW_DAC"

/*
 * Debug
 */

//#define WMT_HWDAC_DEBUG 1
//#define WMT_HWDAC_DEBUG_DETAIL 1

#ifdef WMT_HWDAC_DEBUG
#define DPRINTK(format, arg...) \
	printk(KERN_INFO AUDIO_NAME ": " format "\n" , ## arg)
#else
#define DPRINTK(format, arg...) do {} while (0)
#endif

#ifdef WMT_HWDAC_DEBUG_DETAIL
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

extern int wmt_getsyspara(char *varname, unsigned char *varval, int *varlen);


static int hwdac_pcm_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params, struct snd_soc_dai *dai)
{
	DBG_DETAIL();
	
	return 0;
}


#define HWDAC_RATES (SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_11025 |\
		SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_22050 | SNDRV_PCM_RATE_32000 | \
		SNDRV_PCM_RATE_44100 | SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_88200 | \
		SNDRV_PCM_RATE_96000)

#define HWDEC_FORMATS (SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S20_3LE |\
	SNDRV_PCM_FMTBIT_S24_LE | SNDRV_PCM_FMTBIT_U8 | SNDRV_PCM_FMTBIT_FLOAT)

static struct snd_soc_dai_ops hwdac_dai_ops = {
	.hw_params	= hwdac_pcm_hw_params,
};

struct snd_soc_dai hwdac_dai = {
	.name = "HWDEC",
	.playback = {
		.stream_name = "Playback",
		.channels_min = 1,
		.channels_max = 2,
		.rates = HWDAC_RATES,
		.formats = HWDEC_FORMATS,},
	.ops = &hwdac_dai_ops,
};
EXPORT_SYMBOL_GPL(hwdac_dai);


/*
 * initialise the hwdac driver
 * register the mixer and dsp interfaces with the kernel
 */
static int wmt_hwdac_init(struct snd_soc_device *socdev)
{
	struct snd_soc_codec *codec = socdev->card->codec;
	int ret = 0;

	DBG_DETAIL();

	codec->name = "HWDAC";
	codec->owner = THIS_MODULE;
	codec->dai = &hwdac_dai;
	codec->num_dai = 1;
	
	/* register pcms */
	ret = snd_soc_new_pcms(socdev, SNDRV_DEFAULT_IDX1, SNDRV_DEFAULT_STR1);
	if (ret < 0) {
		printk(KERN_ERR "hwdac: failed to create pcms\n");
		goto pcm_err;
	}

	ret = snd_soc_init_card(socdev);
	if (ret < 0) {
		printk(KERN_ERR "hwdac: failed to register card\n");
		goto card_err;
	}
	return ret;

card_err:
	snd_soc_free_pcms(socdev);
	
pcm_err:
	return ret;
}


static int hwdac_probe(struct platform_device *pdev)
{
	struct snd_soc_device *socdev = platform_get_drvdata(pdev);
	struct snd_soc_codec *codec;
	int ret = 0;

	DBG_DETAIL();
	
	codec = kzalloc(sizeof(struct snd_soc_codec), GFP_KERNEL);
	if (codec == NULL)
		return -ENOMEM;

	socdev->card->codec = codec;
	mutex_init(&codec->mutex);
	INIT_LIST_HEAD(&codec->dapm_widgets);
	INIT_LIST_HEAD(&codec->dapm_paths);

	wmt_hwdac_init(socdev);

	return ret;
}

/* power down chip */
static int hwdac_remove(struct platform_device *pdev)
{
	struct snd_soc_device *socdev = platform_get_drvdata(pdev);
	struct snd_soc_codec *codec = socdev->card->codec;

	DBG_DETAIL();
	
	snd_soc_free_pcms(socdev);

	kfree(codec);

	return 0;
}

struct snd_soc_codec_device soc_codec_dev_hwdac = {
	.probe = 	hwdac_probe,
	.remove = 	hwdac_remove,
};

EXPORT_SYMBOL_GPL(soc_codec_dev_hwdac);

static int __init hwdac_modinit(void)
{
	char buf[80];
	int varlen = 80;
	int ret = 0;
	char codec_name[5];
	
	ret = wmt_getsyspara("wmt.audio.i2s", buf, &varlen);
	
	if (ret == 0) {
		sscanf(buf, "%5s", codec_name);

		if (strcmp(codec_name, "hwdac")) {
			info("hwdac string not found");
			return -EINVAL;
		}
	}
	
	return snd_soc_register_dai(&hwdac_dai);
}
module_init(hwdac_modinit);

static void __exit hwdac_exit(void)
{
	snd_soc_unregister_dai(&hwdac_dai);
}
module_exit(hwdac_exit);

MODULE_DESCRIPTION("WMT [ALSA SoC] driver");
MODULE_AUTHOR("WonderMedia Technologies, Inc.");
MODULE_LICENSE("GPL");
