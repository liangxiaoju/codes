#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include "asoundlib.h"
#include "module.h"
#include "event.h"
#include "thread.h"

#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT  0x20746d66
#define ID_DATA 0x61746164

#define FORMAT_PCM	1

#define DEFAULT_VOLUME	90

module_t module_info_sym;
modwin_t *modwin;

struct riff_wave_header {
    uint32_t riff_id;
    uint32_t riff_sz;
    uint32_t wave_id;
};

struct chunk_header {
    uint32_t id;
    uint32_t sz;
};

struct chunk_fmt {
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
};

struct wav_header {
    uint32_t riff_id;
    uint32_t riff_sz;
    uint32_t riff_fmt;
    uint32_t fmt_id;
    uint32_t fmt_sz;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
    uint32_t data_id;
    uint32_t data_sz;
};

int capturing = 1;

void sigint_handler(int sig)
{
    capturing = 0;
}

unsigned int capture_sample(FILE *file, unsigned int card, unsigned int device,
                            unsigned int channels, unsigned int rate,
                            unsigned int bits, unsigned int period_size,
                            unsigned int period_count)
{
    struct pcm_config config;
    struct pcm *pcm;
    char *buffer;
    unsigned int size;
    unsigned int bytes_read = 0;

    config.channels = channels;
    config.rate = rate;
    config.period_size = period_size;
    config.period_count = period_count;
    if (bits == 32)
        config.format = PCM_FORMAT_S32_LE;
    else if (bits == 16)
        config.format = PCM_FORMAT_S16_LE;
    config.start_threshold = 0;
    config.stop_threshold = 0;
    config.silence_threshold = 0;

    pcm = pcm_open(card, device, PCM_IN, &config);
    if (!pcm || !pcm_is_ready(pcm)) {
        fprintf(stderr, "Unable to open PCM device (%s)\n",
                pcm_get_error(pcm));
        return 0;
    }

    size = pcm_get_buffer_size(pcm);
    buffer = malloc(size);
    if (!buffer) {
        fprintf(stderr, "Unable to allocate %d bytes\n", size);
        free(buffer);
        pcm_close(pcm);
        return 0;
    }

    while (capturing && !pcm_read(pcm, buffer, size)) {
        if (fwrite(buffer, 1, size, file) != size) {
            fprintf(stderr,"Error capturing sample\n");
            break;
        }
        bytes_read += size;
    }

    free(buffer);
    pcm_close(pcm);
    return bytes_read / ((bits / 8) * channels);
}

int capture_wave(const char *filename)
{
	FILE *file;
	struct wav_header header;
	unsigned int card = 0;
	unsigned int device = 0;
	unsigned int channels = 2;
	unsigned int rate = 44100;
	unsigned int bits = 16;
	unsigned int frames;
	unsigned int period_size = 1024;
    unsigned int period_count = 4;

    file = fopen(filename, "wb");
    if (!file)
        return 1;

    header.riff_id = ID_RIFF;
    header.riff_sz = 0;
    header.riff_fmt = ID_WAVE;
    header.fmt_id = ID_FMT;
    header.fmt_sz = 16;
    header.audio_format = FORMAT_PCM;
    header.num_channels = channels;
    header.sample_rate = rate;
    header.byte_rate = (header.bits_per_sample / 8) * channels * rate;
    header.block_align = channels * (header.bits_per_sample / 8);
    header.bits_per_sample = bits;
    header.data_id = ID_DATA;

    /* leave enough room for header */
    fseek(file, sizeof(struct wav_header), SEEK_SET);

    frames = capture_sample(file, card, device, header.num_channels,
                            header.sample_rate, header.bits_per_sample,
                            period_size, period_count);

    /* write header now all information is known */
    header.data_sz = frames * header.block_align;
    fseek(file, 0, SEEK_SET);
    fwrite(&header, sizeof(struct wav_header), 1, file);

    fclose(file);

    return 0;
}

void play_sample(FILE *file, unsigned int card, unsigned int device, unsigned int channels,
                 unsigned int rate, unsigned int bits, unsigned int period_size,
                 unsigned int period_count)
{
    struct pcm_config config;
    struct pcm *pcm;
    char *buffer;
    int size;
    int num_read;

    config.channels = channels;
    config.rate = rate;
    config.period_size = period_size;
    config.period_count = period_count;
    if (bits == 32)
        config.format = PCM_FORMAT_S32_LE;
    else if (bits == 16)
        config.format = PCM_FORMAT_S16_LE;
    config.start_threshold = 0;
    config.stop_threshold = 0;
    config.silence_threshold = 0;

    pcm = pcm_open(card, device, PCM_OUT, &config);
    if (!pcm || !pcm_is_ready(pcm)) {
        fprintf(stderr, "Unable to open PCM device %u (%s)\n",
                device, pcm_get_error(pcm));
        return;
    }

    size = pcm_frames_to_bytes(pcm, pcm_get_buffer_size(pcm));
    buffer = malloc(size);
    if (!buffer) {
        fprintf(stderr, "Unable to allocate %d bytes\n", size);
        free(buffer);
        pcm_close(pcm);
        return;
    }

    do {
        num_read = fread(buffer, 1, size, file);
        if (num_read > 0) {
            if (pcm_write(pcm, buffer, num_read)) {
                fprintf(stderr, "Error playing sample\n");
                break;
            }
        }
    } while (num_read > 0);

    free(buffer);
    pcm_close(pcm);
}

int play_wave(const char *filename)
{
	FILE *file;
    struct riff_wave_header riff_wave_header;
    struct chunk_header chunk_header;
    struct chunk_fmt chunk_fmt;
    unsigned int device = 0;
    unsigned int card = 0;
    unsigned int period_size = 1024;
    unsigned int period_count = 4;
    int more_chunks = 1;

	file = fopen(filename, "rb");
	if (!file)
		return -1;

    fread(&riff_wave_header, sizeof(riff_wave_header), 1, file);
    if ((riff_wave_header.riff_id != ID_RIFF) ||
        (riff_wave_header.wave_id != ID_WAVE)) {
        fclose(file);
        return -1;
    }

    do {
        fread(&chunk_header, sizeof(chunk_header), 1, file);

        switch (chunk_header.id) {
        case ID_FMT:
            fread(&chunk_fmt, sizeof(chunk_fmt), 1, file);
            /* If the format header is larger, skip the rest */
            if (chunk_header.sz > sizeof(chunk_fmt))
                fseek(file, chunk_header.sz - sizeof(chunk_fmt), SEEK_CUR);
            break;
        case ID_DATA:
            /* Stop looking for chunks */
            more_chunks = 0;
            break;
        default:
            /* Unknown chunk, skip bytes */
            fseek(file, chunk_header.sz, SEEK_CUR);
        }
    } while (more_chunks);

    play_sample(file, card, device, chunk_fmt.num_channels, chunk_fmt.sample_rate,
                chunk_fmt.bits_per_sample, period_size, period_count);

    fclose(file);

    return 0;
}

int setSoundVolume(int value)
{
	struct mixer *mixer;
	struct mixer_ctl *ctl;
	int card = 0;
	unsigned int num_values, i;
	int min, max;

	mixer = mixer_open(card);
	if (!mixer)
		return -1;

#if __arm__
	ctl = mixer_get_ctl_by_name(mixer, "Headphone Volume");
#else
	ctl = mixer_get_ctl_by_name(mixer, "Headphone Playback Volume");
#endif
	num_values = mixer_ctl_get_num_values(ctl);
	min = mixer_ctl_get_range_min(ctl);
	max = mixer_ctl_get_range_max(ctl);

	value = value * (max - min) / 100;
	for (i = 0; i < num_values; i++) {
		if (mixer_ctl_set_value(ctl, i, value))
			return -1;
	}

	mixer_close(mixer);

	return 0;
}

void playloop(void)
{
	int ret;
	setSoundVolume(DEFAULT_VOLUME);
	do {
		ret = play_wave("/usr/share/sounds/sound.wav");
	} while (1);
	exit(ret);
}

int selection_playback(CDKSCREEN *screen)
{
	CDKLABEL *label;
	int event;
	char *mesg[] = { "<C>Playing Sound ...", "<C>Hit any key to stop." };
	pid_t pid;

	if ((pid=fork()) == 0)
		playloop();

	window_lock();
	label = newCDKLabel(screen, CENTER, CENTER, mesg, 2, TRUE, FALSE);
	drawCDKLabel(label, TRUE);
	window_unlock();

	do {
		event = get_event(modwin);
	} while (!event);

	window_lock();
	destroyCDKLabel(label);
	window_unlock();

	kill(pid, SIGTERM);
	waitpid(pid, 0, 0);

	return 0;
}

void captureloop(void)
{
	int ret;
    signal(SIGINT, sigint_handler);
	ret = capture_wave("/tmp/sound.wav");
	exit(ret);
}

void playbackloop(void)
{
	int ret;
	setSoundVolume(DEFAULT_VOLUME);
	do {
		ret = play_wave("/tmp/sound.wav");
	} while (1);
	exit(ret);
}

int selection_capture(CDKSCREEN *screen)
{
	CDKLABEL *label;
	char *mesg[5];
	int event;
	pid_t pid;

	if ((pid = fork()) == 0) {
		captureloop();
	}

	mesg[0] = "<C>Recording Sound ...";
	mesg[1] = "<C>Say something";
	mesg[2] = "<C>Hit any key to stop";
	window_lock();
	label = newCDKLabel(screen, CENTER, CENTER, mesg, 3, TRUE, FALSE);
	drawCDKLabel(label, TRUE);
	window_unlock();

	do {
		event = get_event(modwin);
	} while (!event);

	window_lock();
	destroyCDKLabel(label);
	window_unlock();

	kill(pid, SIGINT);
	waitpid(pid, 0, 0);

	if ((pid = fork()) == 0) {
		playbackloop();
	}

	mesg[0] = "<C>Playback ...";
	mesg[1] = "<C>Hit any key to stop";
	window_lock();
	label = newCDKLabel(screen, CENTER, CENTER, mesg, 2, TRUE, FALSE);
	drawCDKLabel(label, TRUE);
	window_unlock();

	do {
		event = get_event(modwin);
	} while (!event);

	window_lock();
	destroyCDKLabel(label);
	window_unlock();

	kill(pid, SIGTERM);
	waitpid(pid, 0, 0);

	return 0;
}

int selection_volume(CDKSCREEN *screen)
{
	CDKUSLIDER *uslider;
	pid_t pid;
	int lines, cols;
	int curr = DEFAULT_VOLUME, high = 100, low = 0, inc = 10;
	int event;

	if ((pid = fork()) == 0) {
		playloop();
	}

	getmaxyx(screen->window, lines, cols);
	window_lock();
	uslider = newCDKUSlider(screen, CENTER, CENTER,
			"<C>Volume", "Current Value: ",
			A_REVERSE | ' ', cols -25,
			curr, low, high, inc, inc*2, TRUE, TRUE);
	drawCDKUSlider(uslider, TRUE);
	window_unlock();

	while (1) {
		event = get_event(modwin);
		if (!event)
			continue;

		if (event == KEY_UP || event == KEY_RIGHT) {
			curr = curr >= high ? high : curr+inc;
		} else if (event == KEY_DOWN || event == KEY_LEFT) {
			curr = curr <= low ? low : curr-inc;
		} else {
			break;
		}
		setSoundVolume(curr);
		window_lock();
		setCDKUSliderValue(uslider, curr);
		drawCDKUSlider(uslider, TRUE);
		window_unlock();
	}
	window_lock();
	destroyCDKUSlider(uslider);
	window_unlock();

	kill(pid, SIGTERM);
	waitpid(pid, 0, 0);

	return 0;
}

void *sound_handler(void *arg)
{
	modwin = arg;
//	boxwin_t *boxwin = to_boxwin(modwin);
	WINDOW *win = modwin->win;
	CDKSCREEN  *screen;
	CDKSELECTION *selection;
	char *list[] = {" "};
	int lines, cols;
	int iselect, event = 0;
	char *itemList[] = {"playback", "capture", "volume"};

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	window_lock();
	screen = initCDKScreen(win);
	getmaxyx(win, lines, cols);

	selection = newCDKSelection(
			screen, CENTER, CENTER, LEFT, lines, cols,
			NULL,
			(CDK_CSTRING2)itemList, 3,
			(CDK_CSTRING2)list, 1,
			A_REVERSE, TRUE, FALSE);
	window_unlock();

	for (;;) {

		window_lock();
		drawCDKSelection(selection, TRUE);
		window_unlock();

		do {
			event = get_event(modwin);
		} while (!event);

		if (event == EVENT_EXIT)
			break;

		window_lock();
		injectCDKSelection(selection, event);
		window_unlock();

		if (event == 0xa) {

			window_lock();
			eraseCDKSelection(selection);
			box(win, 0, 0);
			wrefresh(win);
			iselect = getCDKSelectionCurrent(selection);
			window_unlock();

			switch (iselect) {
			case 0:
				selection_playback(screen);
				break;
			case 1:
				selection_capture(screen);
				break;
			case 2:
				selection_volume(screen);
				break;
			default:
				break;
			}
		}

		if (event == 0x1b) {
			//eraseCDKSelection(selection);
			//eraseCDKScreen(screen);
			exit_subwin(modwin);
		}
	}

	window_lock();
	destroyCDKSelection(selection);
	destroyCDKScreen(screen);
	window_unlock();

	pthread_exit(NULL);
}

module_t module_info_sym = {
	.name = "sound",
	.handler = sound_handler,
};
