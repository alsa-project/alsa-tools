/*
 *  Copyright (c) by Jaroslav Kysela <perex@suse.cz>
 *
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <stdio.h>
#include <errno.h>
#include <sys/asoundlib.h>

typedef signed short sint_16;
typedef unsigned int uint_32;

#include "output.h"

static int pcm_channels;
static snd_pcm_t *pcm;

/*
 * open the audio device for writing to
 */
int output_open(int bits, int rate, int channels)
{
	char devstr[128];
	int card, dev;
	snd_pcm_hw_params_t *params;
	snd_pcm_sw_params_t *swparams;
	snd_pcm_sframes_t buffer_time;
	snd_pcm_sframes_t period_time;
	int err;
	snd_pcm_hw_params_alloca(&params);
	snd_pcm_sw_params_alloca(&swparams);

	pcm_channels = channels;

	/*
	 * Open the device driver
	 */
	card = snd_defaults_pcm_card();
	dev = snd_defaults_pcm_device();
	if (card < 0 || dev < 0) {
		fprintf(stderr, "defaults are not set\n");
		return -ENODEV;
	}
	switch (channels) {
	case 1:
	case 2:
		sprintf(devstr, "plug:%d,%d", card, dev);
		break;
	case 4:
		sprintf(devstr, "surround40:%d,%d", card, dev);
		break;
	case 6:
		sprintf(devstr, "surround51:%d,%d", card, dev);
		break;
	default:
		fprintf(stderr, "%d channels are not supported\n", channels);
		return -EINVAL;
	}
	if ((err = snd_pcm_open(&pcm, devstr, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
		fprintf(stderr, "snd_pcm_open: %s\n", snd_strerror(err));
		return err;
	}
	err = snd_pcm_hw_params_any(pcm, params);
	if (err < 0) {
		fprintf(stderr, "Broken configuration for this PCM: no configurations available");
		goto __close;
	}
	/* set interleaved access */
	err = snd_pcm_hw_params_set_access(pcm, params,
					   SND_PCM_ACCESS_RW_INTERLEAVED);
	if (err < 0) {
		fprintf(stderr, "Access type not available");
		goto __close;
	}
	err = snd_pcm_hw_params_set_format(pcm, params, bits == 16 ? SND_PCM_FORMAT_S16_LE : SND_PCM_FORMAT_U8);
	if (err < 0) {
		fprintf(stderr, "Sample format non available");
		goto __close;
	}
	err = snd_pcm_hw_params_set_channels(pcm, params, channels);
	if (err < 0) {
		fprintf(stderr, "Channels count non available");
		goto __close;
	}
	err = snd_pcm_hw_params_set_rate_near(pcm, params, rate, 0);
	if (err < 0) {
		fprintf(stderr, "Rate not available");
		goto __close;
	}
	buffer_time = snd_pcm_hw_params_set_buffer_time_near(pcm, params,
							     500000, 0);
	if (buffer_time < 0) {
		fprintf(stderr, "Buffer time not available");
		goto __close;
	}
	period_time = snd_pcm_hw_params_set_period_time_near(pcm, params,
							     100000, 0);
	if (period_time < 0) {
		fprintf(stderr, "Period time not available");
		goto __close;
	}
	if (buffer_time == period_time) {
		fprintf(stderr, "Buffer time and period time match, could not use\n");
		goto __close;
	}
	if ((err = snd_pcm_hw_params(pcm, params)) < 0) {
		fprintf(stderr, "PCM hw_params failed: %s\n", snd_strerror(err));
		goto __close;
	}

	return 0;
	
      __close:
      	snd_pcm_close(pcm);
      	pcm = NULL;
      	return err;
}

/*
 * play the sample to the already opened file descriptor
 */
void output_play(sint_16* output_samples, uint_32 num_frames)
{
	snd_pcm_sframes_t res;

	res = snd_pcm_writei(pcm, (void *)output_samples, num_frames);
	if (res < 0)
		fprintf(stderr, "writei returned error: %s\n", snd_strerror(res));
	else if (res != num_frames)
		fprintf(stderr, "writei retured %li (expected %li)\n", res, (long)(num_frames));
}


void
output_close(void)
{
	snd_pcm_close(pcm);
}
