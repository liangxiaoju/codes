#include <linux/slab.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include "../codecs/wm8988.h"

#define AUDIO_DEBUG

#ifdef AUDIO_DEBUG
#define audio_dbg(fmt, arg ...)		printk(KERN_INFO fmt, ## arg)
#else
#define audio_dbg(fmt, arg ...)
#endif

struct mid_kctls {
	struct snd_kcontrol *hp_pb_vol;
	struct snd_kcontrol *spk_pb_vol;
	struct snd_kcontrol *lmixer_pb_sw;
	struct snd_kcontrol *rmixer_pb_sw;
	struct snd_kcontrol *lline_mux;
	struct snd_kcontrol *rline_mux;
	struct snd_kcontrol *lmixer_lbypass_sw;
	struct snd_kcontrol *lmixer_lbypass_vol;
	struct snd_kcontrol *rmixer_rbypass_sw;
	struct snd_kcontrol *rmixer_rbypass_vol;
	struct snd_kcontrol *rmixer_lbypass_sw;
	struct snd_kcontrol *rmixer_lbypass_vol;
	struct snd_kcontrol *diff_mux;
	struct snd_kcontrol *lpga_mux;
	struct snd_kcontrol *rpga_mux;
	struct snd_kcontrol *ladc_mux;
	struct snd_kcontrol *radc_mux;
	struct snd_kcontrol *cp_sw;
	struct snd_kcontrol *cp_vol;
	struct snd_kcontrol *cp_dvol;
	struct snd_kcontrol *spk_sw;
};

static struct mid_kctls *mid_kctls;

static void __set_hp_volume(struct snd_soc_codec *codec, unsigned int volume)
{
	//'Output 1 Playback Volume'
	struct snd_kcontrol *kctl = mid_kctls->hp_pb_vol;
	struct snd_ctl_elem_value uctl = {{0},};

	uctl.value.integer.value[0] = volume;
	uctl.value.integer.value[1] = volume;
	kctl->put(kctl, &uctl);
	snd_ctl_notify(codec->card, SNDRV_CTL_EVENT_MASK_VALUE, &kctl->id);
}

static void __set_spk_volume(struct snd_soc_codec *codec, unsigned int volume)
{
	//'Output 2 Playback Volume'
	struct snd_kcontrol *kctl = mid_kctls->spk_pb_vol;
	struct snd_ctl_elem_value uctl = {{0},};

	uctl.value.integer.value[0] = volume;
	uctl.value.integer.value[1] = volume;
	kctl->put(kctl, &uctl);
	snd_ctl_notify(codec->card, SNDRV_CTL_EVENT_MASK_VALUE, &kctl->id);
}

static void __set_spk_switch(struct snd_soc_codec *codec, unsigned int sw)
{
	//'SPK Switch' sw
	struct snd_kcontrol *kctl = mid_kctls->spk_sw;
	struct snd_ctl_elem_value uctl = {{0},};

	uctl.value.integer.value[0] = !!sw;
	kctl->put(kctl, &uctl);
	snd_ctl_notify(codec->card, SNDRV_CTL_EVENT_MASK_VALUE, &kctl->id);
}

static void __set_path_playback_on(struct snd_soc_codec *codec)
{
	struct snd_kcontrol *kctl;
	struct snd_ctl_elem_value uctl = {{0},};

	//'Left Mixer Playback Switch' = on
	kctl = mid_kctls->lmixer_pb_sw;
	uctl.value.integer.value[0] = 1;
	kctl->put(kctl, &uctl);
	snd_ctl_notify(codec->card, SNDRV_CTL_EVENT_MASK_VALUE, &kctl->id);

	//'Right Mixer Playback Switch' = on
	kctl = mid_kctls->rmixer_pb_sw;
	uctl.value.integer.value[0] = 1;
	kctl->put(kctl, &uctl);
	snd_ctl_notify(codec->card, SNDRV_CTL_EVENT_MASK_VALUE, &kctl->id);
}

static void __set_path_playback_off(struct snd_soc_codec *codec)
{
	struct snd_kcontrol *kctl;
	struct snd_ctl_elem_value uctl = {{0},};

	//'Left Mixer Playback Switch' = off
	kctl = mid_kctls->lmixer_pb_sw;
	uctl.value.integer.value[0] = 0;
	kctl->put(kctl, &uctl);
	snd_ctl_notify(codec->card, SNDRV_CTL_EVENT_MASK_VALUE, &kctl->id);

	//'Right Mixer Playback Switch' = off
	kctl = mid_kctls->rmixer_pb_sw;
	uctl.value.integer.value[0] = 0;
	kctl->put(kctl, &uctl);
	snd_ctl_notify(codec->card, SNDRV_CTL_EVENT_MASK_VALUE, &kctl->id);
}

static void set_path_playback_spk(struct snd_soc_codec *codec)
{
	audio_dbg("%s\n", __func__);
	__set_spk_switch(codec, 0);
	__set_hp_volume(codec, 0);
	__set_spk_volume(codec, 0);
	__set_path_playback_on(codec);
	__set_spk_volume(codec, 121);
	__set_spk_switch(codec, 1);
}

static void set_path_playback_hp(struct snd_soc_codec *codec)
{
	audio_dbg("%s\n", __func__);
	__set_spk_switch(codec, 0);
	__set_hp_volume(codec, 0);
	__set_spk_volume(codec, 0);
	__set_path_playback_on(codec);
	__set_hp_volume(codec, 121);
}

static void set_path_playback_both(struct snd_soc_codec *codec)
{
	audio_dbg("%s\n", __func__);
	__set_spk_switch(codec, 0);
	__set_hp_volume(codec, 0);
	__set_spk_volume(codec, 0);
	__set_path_playback_on(codec);
	__set_hp_volume(codec, 121);
	__set_spk_volume(codec, 121);
	__set_spk_switch(codec, 1);
}

static void __set_path_playback_fm_on(struct snd_soc_codec *codec)
{
	struct snd_kcontrol *kctl;
	struct snd_ctl_elem_value uctl = {{0},};

	//'Left Line Mux' = 'Line 1'
	kctl = mid_kctls->lline_mux;
	uctl.value.enumerated.item[0] = 0;
	kctl->put(kctl, &uctl);
	snd_ctl_notify(codec->card, SNDRV_CTL_EVENT_MASK_VALUE, &kctl->id);

	//'Right Line Mux' = 'Line 1'
	kctl = mid_kctls->rline_mux;
	uctl.value.enumerated.item[0] = 0;
	kctl->put(kctl, &uctl);
	snd_ctl_notify(codec->card, SNDRV_CTL_EVENT_MASK_VALUE, &kctl->id);

	//'Left Mixer Left Bypass Switch' = on
	kctl = mid_kctls->lmixer_lbypass_sw;
	uctl.value.integer.value[0] = 1;
	kctl->put(kctl, &uctl);
	snd_ctl_notify(codec->card, SNDRV_CTL_EVENT_MASK_VALUE, &kctl->id);

	//'Left Mixer Left Bypass Volume' = 7
	kctl = mid_kctls->lmixer_lbypass_vol;
	uctl.value.integer.value[0] = 7;
	kctl->put(kctl, &uctl);
	snd_ctl_notify(codec->card, SNDRV_CTL_EVENT_MASK_VALUE, &kctl->id);

	//'Right Mixer Right Bypass Switch' = on
	kctl = mid_kctls->rmixer_rbypass_sw;
	uctl.value.integer.value[0] = 1;
	kctl->put(kctl, &uctl);
	snd_ctl_notify(codec->card, SNDRV_CTL_EVENT_MASK_VALUE, &kctl->id);

	//'Right Mixer Right Bypass Volume' = 7
	kctl = mid_kctls->rmixer_rbypass_vol;
	uctl.value.integer.value[0] = 7;
	kctl->put(kctl, &uctl);
	snd_ctl_notify(codec->card, SNDRV_CTL_EVENT_MASK_VALUE, &kctl->id);
}

static void __set_path_playback_fm_off(struct snd_soc_codec *codec)
{
	struct snd_kcontrol *kctl;
	struct snd_ctl_elem_value uctl = {{0},};

	//'Left Mixer Left Bypass Switch' = off
	kctl = mid_kctls->lmixer_lbypass_sw;
	uctl.value.integer.value[0] = 0;
	kctl->put(kctl, &uctl);
	snd_ctl_notify(codec->card, SNDRV_CTL_EVENT_MASK_VALUE, &kctl->id);

	//'Left Mixer Left Bypass Volume' = 0
	kctl = mid_kctls->lmixer_lbypass_vol;
	uctl.value.integer.value[0] = 0;
	kctl->put(kctl, &uctl);
	snd_ctl_notify(codec->card, SNDRV_CTL_EVENT_MASK_VALUE, &kctl->id);

	//'Right Mixer Right Bypass Switch' = off
	kctl = mid_kctls->rmixer_rbypass_sw;
	uctl.value.integer.value[0] = 0;
	kctl->put(kctl, &uctl);
	snd_ctl_notify(codec->card, SNDRV_CTL_EVENT_MASK_VALUE, &kctl->id);

	//'Right Mixer Right Bypass Volume' = 0
	kctl = mid_kctls->rmixer_rbypass_vol;
	uctl.value.integer.value[0] = 0;
	kctl->put(kctl, &uctl);
	snd_ctl_notify(codec->card, SNDRV_CTL_EVENT_MASK_VALUE, &kctl->id);
}

static void set_path_playback_fm_spk(struct snd_soc_codec *codec)
{
	audio_dbg("%s\n", __func__);
	__set_spk_switch(codec, 0);
	__set_hp_volume(codec, 0);
	__set_spk_volume(codec, 0);
	__set_path_playback_fm_on(codec);
	__set_spk_volume(codec, 121);
	__set_spk_switch(codec, 1);
}

static void set_path_playback_fm_hp(struct snd_soc_codec *codec)
{
	audio_dbg("%s\n", __func__);
	__set_spk_switch(codec, 0);
	__set_hp_volume(codec, 0);
	__set_spk_volume(codec, 0);
	__set_path_playback_fm_on(codec);
	__set_hp_volume(codec, 121);
}

static void set_path_playback_fm_both(struct snd_soc_codec *codec)
{
	audio_dbg("%s\n", __func__);
	__set_spk_switch(codec, 0);
	__set_hp_volume(codec, 0);
	__set_spk_volume(codec, 0);
	__set_path_playback_fm_on(codec);
	__set_hp_volume(codec, 121);
	__set_spk_volume(codec, 121);
	__set_spk_switch(codec, 1);
}

static void __set_path_playback_mic_on(struct snd_soc_codec *codec)
{
	struct snd_kcontrol *kctl;
	struct snd_ctl_elem_value uctl = {{0},};

	//'Differential Mux' 'Line 2'
	kctl = mid_kctls->diff_mux;
	uctl.value.enumerated.item[0] = 1;
	kctl->put(kctl, &uctl);
	snd_ctl_notify(codec->card, SNDRV_CTL_EVENT_MASK_VALUE, &kctl->id);

	//'Left Line Mux' 'Differential'
	kctl = mid_kctls->lline_mux;
	uctl.value.enumerated.item[0] = 3;
	kctl->put(kctl, &uctl);
	snd_ctl_notify(codec->card, SNDRV_CTL_EVENT_MASK_VALUE, &kctl->id);

	//'Right Line Mux' 'Differential'
	kctl = mid_kctls->rline_mux;
	uctl.value.enumerated.item[0] = 3;
	kctl->put(kctl, &uctl);
	snd_ctl_notify(codec->card, SNDRV_CTL_EVENT_MASK_VALUE, &kctl->id);

	//'Left Mixer Left Bypass Switch' on
	kctl = mid_kctls->lmixer_lbypass_sw;
	uctl.value.integer.value[0] = 1;
	kctl->put(kctl, &uctl);
	snd_ctl_notify(codec->card, SNDRV_CTL_EVENT_MASK_VALUE, &kctl->id);

	//'Left Mixer Left Bypass Volume' 7
	kctl = mid_kctls->lmixer_lbypass_vol;
	uctl.value.integer.value[0] = 7;
	kctl->put(kctl, &uctl);
	snd_ctl_notify(codec->card, SNDRV_CTL_EVENT_MASK_VALUE, &kctl->id);

	//'Right Mixer Left Bypass Switch' on
	kctl = mid_kctls->rmixer_lbypass_sw;
	uctl.value.integer.value[0] = 1;
	kctl->put(kctl, &uctl);
	snd_ctl_notify(codec->card, SNDRV_CTL_EVENT_MASK_VALUE, &kctl->id);

	//'Right Mixer Left Bypass Volume' 7
	kctl = mid_kctls->rmixer_lbypass_vol;
	uctl.value.integer.value[0] = 7;
	kctl->put(kctl, &uctl);
	snd_ctl_notify(codec->card, SNDRV_CTL_EVENT_MASK_VALUE, &kctl->id);
}

static void __set_path_playback_mic_off(struct snd_soc_codec *codec)
{
	struct snd_kcontrol *kctl;
	struct snd_ctl_elem_value uctl = {{0},};

	//'Left Mixer Left Bypass Switch' = off
	kctl = mid_kctls->lmixer_lbypass_sw;
	uctl.value.integer.value[0] = 0;
	kctl->put(kctl, &uctl);
	snd_ctl_notify(codec->card, SNDRV_CTL_EVENT_MASK_VALUE, &kctl->id);

	//'Left Mixer Left Bypass Volume' = 0
	kctl = mid_kctls->lmixer_lbypass_vol;
	uctl.value.integer.value[0] = 0;
	kctl->put(kctl, &uctl);
	snd_ctl_notify(codec->card, SNDRV_CTL_EVENT_MASK_VALUE, &kctl->id);

	//'Right Mixer Left Bypass Switch' = off
	kctl = mid_kctls->rmixer_lbypass_sw;
	uctl.value.integer.value[0] = 0;
	kctl->put(kctl, &uctl);
	snd_ctl_notify(codec->card, SNDRV_CTL_EVENT_MASK_VALUE, &kctl->id);

	//'Right Mixer Left Bypass Volume' = 0
	kctl = mid_kctls->rmixer_lbypass_vol;
	uctl.value.integer.value[0] = 0;
	kctl->put(kctl, &uctl);
	snd_ctl_notify(codec->card, SNDRV_CTL_EVENT_MASK_VALUE, &kctl->id);
}

static void set_path_playback_mic_spk(struct snd_soc_codec *codec)
{
	__set_spk_switch(codec, 0);
	__set_hp_volume(codec, 0);
	__set_spk_volume(codec, 0);
	__set_path_playback_mic_on(codec);
	__set_spk_volume(codec, 121);
	__set_spk_switch(codec, 1);
}

static void set_path_playback_mic_hp(struct snd_soc_codec *codec)
{
	__set_spk_switch(codec, 0);
	__set_hp_volume(codec, 0);
	__set_spk_volume(codec, 0);
	__set_path_playback_mic_on(codec);
	__set_hp_volume(codec, 121);
}

static void set_path_playback_mic_both(struct snd_soc_codec *codec)
{
	__set_spk_switch(codec, 0);
	__set_hp_volume(codec, 0);
	__set_spk_volume(codec, 0);
	__set_path_playback_mic_on(codec);
	__set_hp_volume(codec, 121);
	__set_spk_volume(codec, 121);
	__set_spk_switch(codec, 1);
}

static void set_path_playback_all_off(struct snd_soc_codec *codec)
{
	__set_spk_switch(codec, 0);
	__set_hp_volume(codec, 0);
	__set_spk_volume(codec, 0);
	__set_path_playback_off(codec);
	__set_path_playback_fm_off(codec);
	__set_path_playback_mic_off(codec);
}

enum PATH_PLAYBACK_ID {
	PATH_PLAYBACK_OFF = 0,
	PATH_PLAYBACK_SPK,
	PATH_PLAYBACK_HP,
	PATH_PLAYBACK_BOTH,
	PATH_PLAYBACK_FM_SPK,
	PATH_PLAYBACK_FM_HP,
	PATH_PLAYBACK_FM_BOTH,
	PATH_PLAYBACK_MIC_SPK,
	PATH_PLAYBACK_MIC_HP,
	PATH_PLAYBACK_MIC_BOTH,
	PATH_PLAYBACK_MAX,
};

static const char *playback_paths[] = {
	[PATH_PLAYBACK_OFF]		= "OFF",
	[PATH_PLAYBACK_SPK]		= "SPK",
	[PATH_PLAYBACK_HP]		= "HP",
	[PATH_PLAYBACK_BOTH]		= "SPK_HP",
	[PATH_PLAYBACK_FM_SPK]		= "FM_SPK",
	[PATH_PLAYBACK_FM_HP]		= "FM_HP",
	[PATH_PLAYBACK_FM_BOTH]		= "FM_SPK_HP",
	[PATH_PLAYBACK_MIC_SPK]		= "MIC_SPK",
	[PATH_PLAYBACK_MIC_HP]		= "MIC_HP",
	[PATH_PLAYBACK_MIC_BOTH]	= "MIC_SPK_HP",
};

typedef void (set_playback_path_t)(struct snd_soc_codec *);

static set_playback_path_t *set_pb_path[] = {
	[PATH_PLAYBACK_OFF]		= set_path_playback_all_off,
	[PATH_PLAYBACK_SPK]		= set_path_playback_spk,
	[PATH_PLAYBACK_HP]		= set_path_playback_hp,
	[PATH_PLAYBACK_BOTH]		= set_path_playback_both,
	[PATH_PLAYBACK_FM_SPK]		= set_path_playback_fm_spk,
	[PATH_PLAYBACK_FM_HP]		= set_path_playback_fm_hp,
	[PATH_PLAYBACK_FM_BOTH]		= set_path_playback_fm_both,
	[PATH_PLAYBACK_MIC_SPK]		= set_path_playback_mic_spk,
	[PATH_PLAYBACK_MIC_HP]		= set_path_playback_mic_hp,
	[PATH_PLAYBACK_MIC_BOTH]	= set_path_playback_mic_both,
};

static long playback_path_id = 0;
static int get_playback_path(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *ucontrol)
{
	int ret = 0;

	ucontrol->value.enumerated.item[0] = playback_path_id;

	return ret;
}

static int set_playback_path(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *ucontrol)
{
	int ret = 0;
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);

	ret = ucontrol->value.enumerated.item[0];
	audio_dbg("%s: %d\n", __func__, ret);

	if (ret == playback_path_id)
		return 0;

	switch (ret) {
	case 0 ... (PATH_PLAYBACK_MAX - 1):
		set_pb_path[ret](codec);
		break;
	default:
		break;
	}

	playback_path_id = ret;

	return 1;
}

static void __set_path_capture_on(struct snd_soc_codec *codec)
{
	struct snd_kcontrol *kctl;
	struct snd_ctl_elem_value uctl = {{0},};

	//'Left PGA Mux' 'Differential'
	kctl = mid_kctls->lpga_mux;
	uctl.value.enumerated.item[0] = 2;
	kctl->put(kctl, &uctl);
	snd_ctl_notify(codec->card, SNDRV_CTL_EVENT_MASK_VALUE, &kctl->id);

	//'Right PGA Mux' 'Differential'
	kctl = mid_kctls->rpga_mux;
	uctl.value.enumerated.item[0] = 2;
	kctl->put(kctl, &uctl);
	snd_ctl_notify(codec->card, SNDRV_CTL_EVENT_MASK_VALUE, &kctl->id);

	//'Left ADC Mux' 'Digital Mono'
	kctl = mid_kctls->ladc_mux;
	uctl.value.enumerated.item[0] = 3;
	kctl->put(kctl, &uctl);
	snd_ctl_notify(codec->card, SNDRV_CTL_EVENT_MASK_VALUE, &kctl->id);

	//'Right ADC Mux' 'Digital Mono'
	kctl = mid_kctls->radc_mux;
	uctl.value.enumerated.item[0] = 3;
	kctl->put(kctl, &uctl);
	snd_ctl_notify(codec->card, SNDRV_CTL_EVENT_MASK_VALUE, &kctl->id);

	//'Capture Switch' on
	kctl = mid_kctls->cp_sw;
	uctl.value.integer.value[0] = 1;
	uctl.value.integer.value[1] = 1;
	kctl->put(kctl, &uctl);
	snd_ctl_notify(codec->card, SNDRV_CTL_EVENT_MASK_VALUE, &kctl->id);

	//'Capture Volume' 63
	kctl = mid_kctls->cp_vol;
	uctl.value.integer.value[0] = 63;
	uctl.value.integer.value[1] = 63;
	kctl->put(kctl, &uctl);
	snd_ctl_notify(codec->card, SNDRV_CTL_EVENT_MASK_VALUE, &kctl->id);

	//'Capture Digital Volume' 230
	kctl = mid_kctls->cp_dvol;
	uctl.value.integer.value[0] = 230;
	uctl.value.integer.value[1] = 230;
	kctl->put(kctl, &uctl);
	snd_ctl_notify(codec->card, SNDRV_CTL_EVENT_MASK_VALUE, &kctl->id);
}

static void set_path_capture_off(struct snd_soc_codec *codec)
{
	struct snd_kcontrol *kctl;
	struct snd_ctl_elem_value uctl = {{0},};

	//'Capture Switch' off
	kctl = mid_kctls->cp_sw;
	uctl.value.integer.value[0] = 0;
	uctl.value.integer.value[1] = 0;
	kctl->put(kctl, &uctl);
	snd_ctl_notify(codec->card, SNDRV_CTL_EVENT_MASK_VALUE, &kctl->id);
}

static void set_path_capture_fm(struct snd_soc_codec *codec)
{
	struct snd_kcontrol *kctl;
	struct snd_ctl_elem_value uctl = {{0},};

	//'Differential Mux' 'Line 1'
	kctl = mid_kctls->diff_mux;
	uctl.value.enumerated.item[0] = 0;
	kctl->put(kctl, &uctl);
	snd_ctl_notify(codec->card, SNDRV_CTL_EVENT_MASK_VALUE, &kctl->id);

	__set_path_capture_on(codec);
}

static void set_path_capture_mic(struct snd_soc_codec *codec)
{
	struct snd_kcontrol *kctl;
	struct snd_ctl_elem_value uctl = {{0},};

	//'Differential Mux' 'Line 2'
	kctl = mid_kctls->diff_mux;
	uctl.value.enumerated.item[0] = 1;
	kctl->put(kctl, &uctl);
	snd_ctl_notify(codec->card, SNDRV_CTL_EVENT_MASK_VALUE, &kctl->id);

	__set_path_capture_on(codec);
}

enum PATH_CAPTURE_ID {
	PATH_CAPTURE_OFF = 0,
	PATH_CAPTURE_FM,
	PATH_CAPTURE_MIC,
	PATH_CAPTURE_MAX,
};

static const char *capture_paths[] = {
	[PATH_CAPTURE_OFF]		= "OFF",
	[PATH_CAPTURE_FM]		= "FM",
	[PATH_CAPTURE_MIC]		= "MIC",
};

typedef void (set_capture_path_t)(struct snd_soc_codec *);

static set_capture_path_t *set_cp_path[] = {
	[PATH_CAPTURE_OFF]		= set_path_capture_off,
	[PATH_CAPTURE_FM]		= set_path_capture_fm,
	[PATH_CAPTURE_MIC]		= set_path_capture_mic,
};

static long capture_path_id = 0;
static int get_capture_path(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *ucontrol)
{
	int ret = 0;

	ucontrol->value.enumerated.item[0] = capture_path_id;

	return ret;
}

static int set_capture_path(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *ucontrol)
{
	int ret = 0;
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);

	ret = ucontrol->value.enumerated.item[0];
	audio_dbg("%s: %d\n", __func__, ret);

	if (ret == capture_path_id)
		return 0;

	switch (ret) {
	case 0 ... (PATH_CAPTURE_MAX - 1):
		set_cp_path[ret](codec);
		break;
	default:
		break;
	}

	capture_path_id = ret;

	return 1;
}

static const struct soc_enum path_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(playback_paths), playback_paths),
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(capture_paths), capture_paths),
};

static const struct snd_kcontrol_new paths_control[] = {
	SOC_ENUM_EXT("Playback Path", path_enum[0], get_playback_path, set_playback_path),
	SOC_ENUM_EXT("Capture Path", path_enum[1], get_capture_path, set_capture_path),
};

static struct snd_kcontrol *
find_kctl_by_name(struct snd_soc_codec *codec,
			const char *str)
{
	struct snd_ctl_elem_id elem_id;
	struct snd_kcontrol *kctl;

	memset(&elem_id, 0, sizeof(elem_id));
	elem_id.iface = SNDRV_CTL_ELEM_IFACE_MIXER;
	strcpy(elem_id.name, str);
	kctl = snd_ctl_find_id(codec->card, &elem_id);
	if (kctl == NULL)
		printk(KERN_ERR "Failed to find kcontrol: %s\n", str);

	return kctl;
}

int mid_add_controls(struct snd_soc_codec *codec)
{
	//add paths control
	snd_soc_add_controls(codec, paths_control, ARRAY_SIZE(paths_control));

	mid_kctls = kzalloc(sizeof(struct mid_kctls), GFP_KERNEL);
	if (!mid_kctls)
		return -ENOMEM;

	//find the kcontrols we need
	mid_kctls->hp_pb_vol =
		find_kctl_by_name(codec, "Output 1 Playback Volume");
	mid_kctls->spk_pb_vol =
		find_kctl_by_name(codec, "Output 2 Playback Volume");
	mid_kctls->lmixer_pb_sw =
		find_kctl_by_name(codec, "Left Mixer Playback Switch");
	mid_kctls->rmixer_pb_sw =
		find_kctl_by_name(codec, "Right Mixer Playback Switch");
	mid_kctls->lline_mux =
		find_kctl_by_name(codec, "Left Line Mux");
	mid_kctls->rline_mux =
		find_kctl_by_name(codec, "Right Line Mux");
	mid_kctls->lmixer_lbypass_sw =
		find_kctl_by_name(codec, "Left Mixer Left Bypass Switch");
	mid_kctls->lmixer_lbypass_vol =
		find_kctl_by_name(codec, "Left Mixer Left Bypass Volume");
	mid_kctls->rmixer_rbypass_sw =
		find_kctl_by_name(codec, "Right Mixer Right Bypass Switch");
	mid_kctls->rmixer_rbypass_vol =
		find_kctl_by_name(codec, "Right Mixer Right Bypass Volume");
	mid_kctls->rmixer_lbypass_sw =
		find_kctl_by_name(codec, "Right Mixer Left Bypass Switch");
	mid_kctls->rmixer_lbypass_vol =
		find_kctl_by_name(codec, "Right Mixer Left Bypass Volume");
	mid_kctls->diff_mux =
		find_kctl_by_name(codec, "Differential Mux");
	mid_kctls->lpga_mux =
		find_kctl_by_name(codec, "Left PGA Mux");
	mid_kctls->rpga_mux =
		find_kctl_by_name(codec, "Right PGA Mux");
	mid_kctls->ladc_mux =
		find_kctl_by_name(codec, "Left ADC Mux");
	mid_kctls->radc_mux =
		find_kctl_by_name(codec, "Right ADC Mux");
	mid_kctls->cp_sw =
		find_kctl_by_name(codec, "Capture Switch");
	mid_kctls->cp_vol =
		find_kctl_by_name(codec, "Capture Volume");
	mid_kctls->cp_dvol =
		find_kctl_by_name(codec, "Capture Digital Volume");
	mid_kctls->spk_sw =
		find_kctl_by_name(codec, "SPK Switch");

	if (mid_kctls->hp_pb_vol && mid_kctls->spk_pb_vol
		&& mid_kctls->lmixer_pb_sw && mid_kctls->rmixer_pb_sw 
		&& mid_kctls->lline_mux && mid_kctls->rline_mux
		&& mid_kctls->lmixer_lbypass_sw && mid_kctls->lmixer_lbypass_vol
		&& mid_kctls->rmixer_rbypass_sw && mid_kctls->rmixer_rbypass_vol
		&& mid_kctls->rmixer_lbypass_sw && mid_kctls->rmixer_lbypass_vol
		&& mid_kctls->diff_mux && mid_kctls->lpga_mux
		&& mid_kctls->rpga_mux && mid_kctls->ladc_mux
		&& mid_kctls->radc_mux && mid_kctls->cp_sw
		&& mid_kctls->cp_vol && mid_kctls->cp_dvol
		&& mid_kctls->spk_sw)

		return 0;
	else {
		audio_dbg("Oh, My God ! Failed to find some kcontrols we need.\n");
		return -ENODEV;
	}
}

/*
 * "Add route controls for lazy Android"
 */

MODULE_AUTHOR("liangxiaoju <liangxiaoju@szboeye.com>");

