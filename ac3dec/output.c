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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <alloca.h>
#include <string.h>
#include <unistd.h>
#include <alsa/asoundlib.h>

typedef signed short sint_16;
typedef unsigned int uint_32;

#include "output.h"

static output_t out_config;
static snd_pcm_t *pcm;

/*
 * open the audio device for writing to
 */
int output_open(output_t *output)
{
	const char *pcm_name = output->pcm_name;
	char devstr[128];
	snd_pcm_hw_params_t *params;
	unsigned int rate, buffer_time, period_time, tmp;
	snd_pcm_format_t format = output->bits == 16 ? SND_PCM_FORMAT_S16 : SND_PCM_FORMAT_U8;
	int err, step;
	snd_pcm_hw_params_alloca(&params);

	out_config = *output;

	/*
	 * Open the device driver
	 */
	if (pcm_name == NULL) {
		switch (output->channels) {
		case 1:
		case 2:
			if (output->spdif != SPDIF_NONE) {
				unsigned char s[4];
				if (output->spdif == SPDIF_PRO) {
					s[0] = (IEC958_AES0_PROFESSIONAL |
					        IEC958_AES0_NONAUDIO |
						IEC958_AES0_PRO_EMPHASIS_NONE |
						IEC958_AES0_PRO_FS_48000);
					s[1] = (IEC958_AES1_PRO_MODE_NOTID |
						IEC958_AES1_PRO_USERBITS_NOTID);
					s[2] = IEC958_AES2_PRO_WORDLEN_NOTID;
					s[3] = 0;
				} else {
					s[0] = IEC958_AES0_CON_EMPHASIS_NONE;
					if (output->spdif == SPDIF_CON)
						s[0] |= IEC958_AES0_NONAUDIO;
					s[1] = (IEC958_AES1_CON_ORIGINAL |
						IEC958_AES1_CON_PCM_CODER);
					s[2] = 0;
					s[3] = IEC958_AES3_CON_FS_48000;
				}
				sprintf(devstr, "plug:iec958:{AES0 0x%x AES1 0x%x AES2 0x%x AES3 0x%x", s[0], s[1], s[2], s[3]);
				if (out_config.card)
					sprintf(devstr + strlen(devstr), " CARD %s", out_config.card);
				strcat(devstr, "}");
				format = SND_PCM_FORMAT_S16_LE;
			} else {
				if (out_config.card)
					sprintf(devstr, "plughw:%s", out_config.card);
				else
					sprintf(devstr, "default");
			}
			break;
		case 4:
			strcpy(devstr, "plug:surround40");
			if (out_config.card)
				sprintf(devstr + strlen(devstr), ":{CARD %s}", out_config.card);
			break;
		case 6:
			strcpy(devstr, "plug:surround51");
			if (out_config.card)
				sprintf(devstr + strlen(devstr), ":{CARD %s}", out_config.card);
			break;
		default:
			fprintf(stderr, "%d channels are not supported\n", output->channels);
			return -EINVAL;
		}
		pcm_name = devstr;
	}
	if (!output->quiet)
		fprintf(stdout, "Using PCM device '%s'\n", pcm_name);
	if ((err = snd_pcm_open(&pcm, pcm_name, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
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
		fprintf(stderr, "Access type not available\n");
		goto __close;
	}
	err = snd_pcm_hw_params_set_format(pcm, params, format);
	if (err < 0) {
		fprintf(stderr, "Sample format non available\n");
		goto __close;
	}
	err = snd_pcm_hw_params_set_channels(pcm, params, output->channels);
	if (err < 0) {
		fprintf(stderr, "Channels count non available\n");
		goto __close;
	}
	rate = output->rate;
	err = snd_pcm_hw_params_set_rate_near(pcm, params, &rate, 0);
	if (err < 0) {
		fprintf(stderr, "Rate not available\n");
		goto __close;
	}
	buffer_time = 500000;
	err = snd_pcm_hw_params_set_buffer_time_near(pcm, params, &buffer_time, 0);
	if (err < 0) {
		fprintf(stderr, "Buffer time not available\n");
		goto __close;
	}
	step = 2;
	period_time = 10000 * 2;
	do {
		period_time /= 2;
		tmp = period_time;
		err = snd_pcm_hw_params_set_period_time_near(pcm, params, &tmp, 0);
		if (err < 0) {
			fprintf(stderr, "Period time not available\n");
			goto __close;
		}
		if (tmp == period_time) {
			period_time /= 3;
			tmp = period_time;
			err = snd_pcm_hw_params_set_period_time_near(pcm, params, &tmp, 0);
			if (tmp == period_time)
				period_time = 10000 * 2;
		}
	} while (buffer_time == period_time && period_time > 10000);
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
int output_play(sint_16* output_samples, uint_32 num_frames)
{
	snd_pcm_sframes_t res = 0;

	do {
		if (res == -EPIPE)		/* underrun */
			res = snd_pcm_prepare(pcm);
		else if (res == -ESTRPIPE) {	/* suspend */
			while ((res = snd_pcm_resume(pcm)) == -EAGAIN)
				sleep(1);
			if (res < 0)
				res = snd_pcm_prepare(pcm);
		}
		res = res < 0 ? res : snd_pcm_writei(pcm, (void *)output_samples, num_frames);
		if (res > 0) {
			output_samples += out_config.channels * res;
			num_frames -= res;
		}
	} while (res == -EPIPE || num_frames > 0);
	if (res < 0)
		fprintf(stderr, "writei returned error: %s\n", snd_strerror(res));
	return res < 0 ? (int)res : 0;
}


void
output_close(void)
{
	snd_pcm_close(pcm);
}
