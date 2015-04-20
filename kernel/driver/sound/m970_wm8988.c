#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/regulator/consumer.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/jack.h>

#include "../codecs/wm8988.h"
#include "s3c-dma.h"
#include "s3c64xx-i2s-v4.h"

#include <linux/io.h>
#include <mach/map.h>
#include <mach/regs-clock.h>
#include <mach/regs-audss.h>
#include <plat/gpio-cfg.h>
#include <mach/gpio.h>

#define AUDIO_DEBUG
#define USE_HP_SWITCH

#ifdef AUDIO_DEBUG
#define audio_dbg(fmt, arg ...)		printk(KERN_INFO fmt, ## arg)
#else
#define audio_dbg(fmt, arg ...)
#endif

#define I2S_NUM		0

#ifdef CONFIG_FM2018
extern int fm2018_poweron(void);
extern int fm2018_poweroff(void);
#else
static inline int fm2018_poweron(void) {return 0;}
static inline int fm2018_poweroff(void) {return 0;}
#endif

extern int mid_add_controls(struct snd_soc_codec *);
extern struct snd_soc_platform s3c_dma_wrapper;
extern const struct snd_kcontrol_new s5p_idma_control;
static struct snd_soc_card m970_card;

static struct platform_device *m970_snd_device;
static struct regulator *mic_regulator;

#if !defined(USE_HP_SWITCH)
static struct snd_soc_jack hp_jack;

static struct snd_soc_jack_pin m970_jack_pins[] = {
	{
		.pin	= "HeadPhone",
		.mask	= SND_JACK_HEADPHONE,
		.invert	= 0,
	},
};

static struct snd_soc_jack_gpio m970_jack_gpios[] = {
	{
		.gpio	= S5P_EXT_INT2(6),
		.name	= "HeadPhone detect",
		.report	= SND_JACK_HEADPHONE,
		.debounce_time	= 500,
		.invert	= 1,
	},
};
#endif

static int m970_audio_event(struct snd_soc_dapm_widget *widget, struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_card *card = widget->codec->socdev->card;
	struct snd_soc_dai_link *dai_link = card->dai_link;

	if (SND_SOC_DAPM_EVENT_ON(event)) {

		if (strcmp(widget->name, "HeadPhone") == 0) {
		//	audio_dbg("Enable HeadPhone\n");
		} else if ((strcmp(widget->name, "Speaker") == 0)) {
			audio_dbg("Enable Speaker\n");
			gpio_direction_output(S5PV210_GPJ3(1), 1);
			s3c_gpio_slp_cfgpin(S5PV210_GPJ3(1), 1);

		} else if (strcmp(widget->name, "Mic") == 0) {
			audio_dbg("Enable Mic\n");
			fm2018_poweron();
			if (mic_regulator)
				regulator_enable(mic_regulator);
		} else if (strcmp(widget->name, "FM") == 0) {
			audio_dbg("Enable FM\n");
			dai_link->ignore_suspend = 1;
		}

	} else if (SND_SOC_DAPM_EVENT_OFF(event)) {

		if (strcmp(widget->name, "HeadPhone") == 0) {
		//	audio_dbg("Disable HeadPhone\n");
		}

		if ((strcmp(widget->name, "Speaker") == 0)) {
			audio_dbg("Disable Speaker\n");
			gpio_direction_output(S5PV210_GPJ3(1), 0);
			s3c_gpio_slp_cfgpin(S5PV210_GPJ3(1), 0);
		}

		if (strcmp(widget->name, "Mic") == 0) {
			audio_dbg("Disable Mic\n");
			if (mic_regulator)
				regulator_disable(mic_regulator);
			fm2018_poweroff();
		} else if (strcmp(widget->name, "FM") == 0) {
			audio_dbg("Disable FM\n");
			dai_link->ignore_suspend = 0;
		}
	}

	return 0;
}

static int get_speaker_status(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.enumerated.item[0] =
		gpio_get_value(S5PV210_GPJ3(1));

	return 0;
}

static int set_speaker_status(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *ucontrol)
{
	int status_new;

	status_new = !!ucontrol->value.enumerated.item[0];

	gpio_direction_output(S5PV210_GPJ3(1), status_new);

	audio_dbg("Force %s Speaker\n", status_new ? "Enable" : "Disable");

	return 1;
}

static const struct snd_kcontrol_new m970_controls[] = {
	SOC_SINGLE_BOOL_EXT("Force Speaker Switch", 0,
			get_speaker_status, set_speaker_status),
};

static const struct snd_kcontrol_new speaker_switch_control =
	SOC_DAPM_SINGLE("Switch", 4, 0, 1, 0);

static const struct snd_soc_dapm_widget m970_dapm_widgets[] = {
	SND_SOC_DAPM_HP("HeadPhone", m970_audio_event),
	SND_SOC_DAPM_SPK("Speaker", m970_audio_event),
	SND_SOC_DAPM_SWITCH("SPK", SND_SOC_NOPM, 0, 0, &speaker_switch_control),
	SND_SOC_DAPM_MIXER("OUT2 Mixer", SND_SOC_NOPM, 0, 0, NULL, 0),
	SND_SOC_DAPM_MIC("Mic", m970_audio_event),
	SND_SOC_DAPM_LINE("FM", m970_audio_event),
};

static const struct snd_soc_dapm_route audio_map[] = {
	{"HeadPhone",	NULL,		"LOUT1"},
	{"HeadPhone",	NULL,		"ROUT1"},
	{"OUT2 Mixer",	NULL,		"LOUT2"},
	{"OUT2 Mixer",	NULL,		"ROUT2"},
	{"SPK",		"Switch",	"OUT2 Mixer"},
	{"Speaker",	NULL,		"SPK"},

	{"LINPUT2",	NULL,		"Mic"},
	{"RINPUT2",	NULL,		"Mic"},
	{"LINPUT1",	NULL,		"FM"},
	{"RINPUT1",	NULL,		"FM"},
};

static int set_epll_rate(unsigned long rate)
{
	struct clk *fout_epll;

	fout_epll = clk_get(NULL, "fout_epll");
	if (IS_ERR(fout_epll)) {
		printk(KERN_ERR "%s: failed to get fout_epll\n", __func__);
		return -ENOENT;
	}

	if (rate == clk_get_rate(fout_epll))
		goto out;

	clk_disable(fout_epll);
	clk_set_rate(fout_epll, rate);
	clk_enable(fout_epll);

out:
	clk_put(fout_epll);

	return 0;
}

static int m970_hw_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->dai->cpu_dai;
	struct snd_soc_dai *codec_dai = rtd->dai->codec_dai;
	unsigned int rclk, psr=1;
	int bfs, rfs, ret;
	unsigned long epll_out_rate;
	unsigned int bit_per_sample, sample_rate;

	switch (params_format(params)) {
	case SNDRV_PCM_FORMAT_U24:
	case SNDRV_PCM_FORMAT_S24:
		bfs = 48;//rfs = {384fs, 768fs}
		bit_per_sample = 24;
		break;
	case SNDRV_PCM_FORMAT_U16_LE:
	case SNDRV_PCM_FORMAT_S16_LE:
		bfs = 32;//rfs = { 256fs, 384fs, 512fs, 768fs}
		bit_per_sample = 16;
		break;
	default:
		return -EINVAL;
	}

	sample_rate = params_rate(params);

	/*
	 * WM8988's MCLK only support
	 * {12.2880MHz,24.576MHz,11.2896MHz,22.5792MHz,18.432MHz,36.864MHz,16.9344MHz,33.8688MHz,12MHz,24MHz},
	 * and sample rate must be
	 * {8kHz, 11.025kHz,12kHz, 16kHz, 22.05kHz, 24kHz, 32kHz, 44.1kHz, 48kHz, 88.2kHz and 96kHz}.
	 * We set root clock to an available minimum value to consume less power.
	 */
	switch (sample_rate) {
	case 8000:
	case 11025:
	case 12000:
		//even rfs=768, we cannot get the rclk(sample_rate * rfs) supported by WM8988
		//so we change sample_rate to a larger value
		printk("%s: resample: %u --> 44100\n", __func__, sample_rate);
		sample_rate = 44100;

		if (bfs == 48)
			rfs = 384;//rclk = 16.9344 MHz
		else
			rfs = 256;//rclk = 11.2896 MHz
		break;
	case 16000:
		rfs = 768;//rclk = 12.2880 MHz
		break;
	case 22050:
		if (bfs == 48)
			rfs = 768;//rclk = 16.9344 MHz
		else
			rfs = 512;//rclk = 11.2896 MHz
		break;
	case 24000:
		if (bfs == 48)
			rfs = 768;//rclk = 18.4320 MHz
		else
			rfs = 512;//rclk = 12.2880 MHz
		break;
	case 32000:
		rfs = 384;//rclk = 12.2880 MHz
		break;
	case 44100:
		if (bfs == 48)
			rfs = 384;//rclk = 16.9344 MHz
		else
			rfs = 256;//rclk = 11.2896 MHz
		break;
	case 48000:
		if (bfs == 48)
			rfs = 384;//rclk = 18.4320 MHz
		else
			rfs = 256;//rclk = 12.2880 MHz
		break;
	case 64000:
		audio_dbg("Warning: Sample rate(%u) not supported by WM8988.\n",sample_rate);
		rfs = 384;//rclk = 24.5760 MHz
		break;
	case 88200:
		if (bfs == 48)
			rfs = 384;//rclk = 33.8688 MHz
		else
			rfs = 256;//rclk = 22.5792 MHz
		break;
	case 96000:
		if (bfs == 48)
			rfs = 384;//rclk = 36.8640 MHz
		else
			rfs = 256;//rclk = 24.5760 MHz
		break;
	default:
		printk("Sample rate(%u) not yet supported !\n",sample_rate);
		return -EINVAL;
	}

	rclk = sample_rate * rfs;

	switch (rclk) {
	case 4096000:
	case 5644800:
	case 6144000:
	case 8467200:
	case 9216000:
		psr = 8;
		break;
	case 8192000:
	case 11289600:
	case 12288000:
	case 16934400:
	case 18432000:
		psr = 4;
		break;
	case 22579200:
	case 24576000:
	case 33868800:
	case 36864000:
		psr = 2;
		break;
	case 67737600:
	case 73728000:
		psr = 1;
		break;
	default:
		printk("Not yet supported!\n");
		return -EINVAL;
	}

	printk("IIS Audio: %uBits Stereo %uHz\n", bit_per_sample, sample_rate);

	epll_out_rate = rclk * psr;

	/* Set EPLL clock rate */
	ret = set_epll_rate(epll_out_rate);
	if (ret < 0)
		return ret;

	ret = snd_soc_dai_set_sysclk(codec_dai, WM8988_SYSCLK, rclk, 0);
	if (ret < 0)
		return ret;

	ret = snd_soc_dai_set_sysclk(cpu_dai, S3C64XX_CLKSRC_CDCLK,
					0, SND_SOC_CLOCK_OUT);
	if (ret < 0)
		return ret;

	/* We use MUX for basic ops in SoC-Master mode */
	ret = snd_soc_dai_set_sysclk(cpu_dai, S3C64XX_CLKSRC_MUX,
					0, SND_SOC_CLOCK_IN);
	if (ret < 0)
		return ret;

	ret = snd_soc_dai_set_clkdiv(cpu_dai, S3C_I2SV2_DIV_PRESCALER, psr-1);
	if (ret < 0)
		return ret;

	ret = snd_soc_dai_set_clkdiv(cpu_dai, S3C_I2SV2_DIV_BCLK, bfs);
	if (ret < 0)
		return ret;

	ret = snd_soc_dai_set_clkdiv(cpu_dai, S3C_I2SV2_DIV_RCLK, rfs);
	if (ret < 0)
		return ret;

	return 0;
}

static struct snd_soc_ops m970_ops = {
	.hw_params	= m970_hw_params,
};

static int m970_wm8988_init(struct snd_soc_codec *codec)
{
	int ret;

	audio_dbg("%s\n",__func__);

	//add our widgets
	snd_soc_dapm_new_controls(codec, m970_dapm_widgets,
			ARRAY_SIZE(m970_dapm_widgets));

	//add our control
	snd_soc_add_controls(codec, m970_controls, 
			ARRAY_SIZE(m970_controls));

	//setup audio path
	snd_soc_dapm_add_routes(codec, audio_map,
				ARRAY_SIZE(audio_map));

	//create any new dapm widgets
	snd_soc_dapm_new_widgets(codec);

	//add route controls for android 2.3
	mid_add_controls(codec);

	//signal a DAPM event
	snd_soc_dapm_sync(codec);

	/* Set the Codec DAI configureation */
	ret = snd_soc_dai_set_fmt(&wm8988_dai, SND_SOC_DAIFMT_I2S
				| SND_SOC_DAIFMT_NB_NF
				| SND_SOC_DAIFMT_CBS_CFS);
	if (ret < 0)
		goto err;

	/* Set the cpu DAI configuration */
	ret = snd_soc_dai_set_fmt(&s3c64xx_i2s_v4_dai[I2S_NUM], SND_SOC_DAIFMT_I2S
				| SND_SOC_DAIFMT_NB_NF
				| SND_SOC_DAIFMT_CBS_CFS);
	if (ret < 0)
		goto err;

	/* Set WM8988 sysclk */
	ret = snd_soc_dai_set_sysclk(&wm8988_dai, WM8988_SYSCLK, 11289600, 0);
	if (ret < 0)
		goto err;

#if !defined(USE_HP_SWITCH)
	ret = snd_soc_jack_new(&m970_card, "HeadPhone Jack",
			SND_JACK_HEADPHONE, &hp_jack);
	if (ret < 0)
		goto err;

	ret = snd_soc_jack_add_pins(&hp_jack, ARRAY_SIZE(m970_jack_pins),
			m970_jack_pins);
	if (ret < 0)
		goto err;

	ret = snd_soc_jack_add_gpios(&hp_jack, ARRAY_SIZE(m970_jack_gpios),
			m970_jack_gpios);
	if (ret < 0)
		goto err;
#endif
/*
	printk("############# clk test ##############\n");
	printk("S5P_CLK_SRC0=0x%08x\n",__raw_readl(S5P_CLK_SRC0));
	printk("S5P_CLK_SRC6=0x%08x\n",__raw_readl(S5P_CLK_SRC6));
	printk("S5P_CLK_DIV6=0x%08x\n",__raw_readl(S5P_CLK_DIV6));
	printk("S5P_CLKGATE_IP3=0x%08x\n",__raw_readl(S5P_CLKGATE_IP3));
*/
	return 0;
err:
	return ret;
}

static struct snd_soc_dai_link m970_dai[] = {
	{
		.name		= "wm8988",
		.stream_name	= "WM8988",
		.cpu_dai	= &s3c64xx_i2s_v4_dai[I2S_NUM],
		.codec_dai	= &wm8988_dai,
		.init		= m970_wm8988_init,
		.ops		= &m970_ops,
	},
};

static struct snd_soc_card m970_card = {
	.name		= "M970",
	.platform	= &s3c_dma_wrapper,
	.dai_link	= m970_dai,
	.num_links	= ARRAY_SIZE(m970_dai),
};

static struct snd_soc_device m970_snd_devdata = {
	.card		= &m970_card,
	.codec_dev	= &soc_codec_dev_wm8988,
};

static int __init m970_audio_init(void)
{
	int ret;
	u32 val;

	audio_dbg("%s\n",__func__);

#if (I2S_NUM==0)
	/* We use I2SCLK for rate generation, so set EPLLout as
	 * the parent of I2SCLK.
	 */
	val = readl(S5P_CLKSRC_AUDSS);
	val &= ~(0x3<<2);//Main CLK
	val |= (1<<0);//FOUT_EPLL
	writel(val, S5P_CLKSRC_AUDSS);

	val = readl(S5P_CLKGATE_AUDSS);
	val |= (0x7f<<0);
	writel(val, S5P_CLKGATE_AUDSS);
#else
	val = __raw_readl(S5P_CLK_SRC0);
	val |= 1<<8;//MUXEPLL --> FOUTEPLL
	__raw_writel(val, S5P_CLK_SRC0);

	val = __raw_readl(S5P_CLK_SRC6);
	val &= ~0xfff;
	val |= 0x777;//MUXAUDIO 0,1,2 --> SCLKEPLL 0,1,2
	__raw_writel(val, S5P_CLK_SRC6);

	val = __raw_readl(S5P_CLK_DIV6);
	val &= ~0xfff;//AUDIO_RATIO --> 0
	__raw_writel(val, S5P_CLK_DIV6);

	val = __raw_readl(S5P_CLKGATE_IP3);
	val |= (1<<4) | (1<<5) | (1<<6);//pass CLK_I2S
	__raw_writel(val, S5P_CLKGATE_IP3);
#endif

	m970_snd_device = platform_device_alloc("soc-audio", 0);
	if (!m970_snd_device)
		return -ENOMEM;

	platform_set_drvdata(m970_snd_device, &m970_snd_devdata);
	m970_snd_devdata.dev = &m970_snd_device->dev;
	ret = platform_device_add(m970_snd_device);
	if (ret) {
		platform_device_put(m970_snd_device);
		return ret;
	}

	//set HeadPhone detect pin to interrupt
	s3c_gpio_setpull(S5P_EXT_INT2(6), S3C_GPIO_PULL_UP);
	s3c_gpio_cfgpin(S5P_EXT_INT2(6), S3C_GPIO_SFN(0xf));

	//request gpio for speaker enable pin
	gpio_request(S5PV210_GPJ3(1), "speaker-en");
	s3c_gpio_setpull(S5PV210_GPJ3(1), S3C_GPIO_PULL_DOWN);
	s3c_gpio_cfgpin(S5PV210_GPJ3(1), S3C_GPIO_OUTPUT);
	gpio_direction_output(S5PV210_GPJ3(1), 0);

	mic_regulator = regulator_get(NULL, "vmic");
	if (IS_ERR(mic_regulator)) {
		ret = PTR_ERR(mic_regulator);
		audio_dbg("Failed to get vmic regulator.\n");
		mic_regulator = NULL;
	}

	return ret;
}
static void __exit m970_audio_exit(void)
{
	regulator_put(mic_regulator);
	gpio_free(S5PV210_GPJ3(1));
	platform_device_unregister(m970_snd_device);
}
module_init(m970_audio_init);
module_exit(m970_audio_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("liangxiaoju <liangxiaoju@szboeye.com>");
MODULE_DESCRIPTION("ALSA SoC M970 WM8988");

