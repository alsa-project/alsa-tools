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

static output_t out_config;
static snd_pcm_t *pcm;

/*
 * open the audio device for writing to
 */
int output_open(output_t *output)
{
	const char *pcm_name = output->pcm_name;
	char devstr[128];
	int card, dev;
	snd_pcm_hw_params_t *params;
	snd_pcm_sw_params_t *swparams;
	snd_pcm_sframes_t buffer_time;
	snd_pcm_sframes_t period_time, tmp;
	int err, step;
	snd_pcm_hw_params_alloca(&params);
	snd_pcm_sw_params_alloca(&swparams);

	out_config = *output;

	/*
	 * Open the device driver
	 */
	if (pcm_name == NULL) {
		card = snd_defaults_pcm_card();
		dev = snd_defaults_pcm_device();
		if (card < 0 || dev < 0) {
			fprintf(stderr, "defaults are not set\n");
			return -ENODEV;
		}
		switch (output->channels) {
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
			fprintf(stderr, "%d channels are not supported\n", output->channels);
			return -EINVAL;
		}
		pcm_name = devstr;
	}
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
		fprintf(stderr, "Access type not available");
		goto __close;
	}
	err = snd_pcm_hw_params_set_format(pcm, params, output->bits == 16 ? SND_PCM_FORMAT_S16_LE : SND_PCM_FORMAT_U8);
	if (err < 0) {
		fprintf(stderr, "Sample format non available");
		goto __close;
	}
	err = snd_pcm_hw_params_set_channels(pcm, params, output->channels);
	if (err < 0) {
		fprintf(stderr, "Channels count non available");
		goto __close;
	}
	err = snd_pcm_hw_params_set_rate_near(pcm, params, output->rate, 0);
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
	step = 2;
	period_time = 10000 * 2;
	do {
		period_time /= 2;
		tmp = snd_pcm_hw_params_set_period_time_near(pcm, params,
							     period_time, 0);
		if (tmp == period_time) {
			period_time /= 3;
			tmp = snd_pcm_hw_params_set_period_time_near(pcm, params,
								     period_time, 0);
			if (tmp == period_time)
				period_time = 10000 * 2;
		}
		if (period_time < 0) {
			fprintf(stderr, "Period time not available");
			goto __close;
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

	if (output->spdif != SPDIF_NONE) {
		snd_pcm_info_t *info;
		snd_ctl_elem_value_t *ctl;
		snd_ctl_t *ctl_handle;
		char ctl_name[12];
		int ctl_card;
		snd_aes_iec958_t spdif;

		memset(&spdif, 0, sizeof(spdif));
		if (output->spdif == SPDIF_PRO) {
			spdif.status[0] = (IEC958_AES0_PROFESSIONAL |
					   IEC958_AES0_NONAUDIO |
					   IEC958_AES0_PRO_EMPHASIS_NONE |
					   IEC958_AES0_PRO_FS_48000);
			spdif.status[1] = (IEC958_AES1_PRO_MODE_NOTID |
					   IEC958_AES1_PRO_USERBITS_NOTID);
			spdif.status[2] = IEC958_AES2_PRO_WORDLEN_NOTID;
			spdif.status[3] = 0;
		} else {
			spdif.status[0] = (IEC958_AES0_NONAUDIO |
					   IEC958_AES0_CON_EMPHASIS_NONE);
			spdif.status[1] = (IEC958_AES1_CON_ORIGINAL |
					   IEC958_AES1_CON_PCM_CODER);
			spdif.status[2] = 0;
			spdif.status[3] = IEC958_AES3_CON_FS_48000;
		}
		snd_pcm_info_alloca(&info);
		if ((err = snd_pcm_info(pcm, info)) < 0) {
			fprintf(stderr, "pcm info error: %s", snd_strerror(err));
			goto __close;
		}
		snd_ctl_elem_value_alloca(&ctl);
		snd_ctl_elem_value_set_interface(ctl, SND_CTL_ELEM_IFACE_PCM);
		snd_ctl_elem_value_set_device(ctl, snd_pcm_info_get_device(info));
		snd_ctl_elem_value_set_subdevice(ctl, snd_pcm_info_get_subdevice(info));
		snd_ctl_elem_value_set_name(ctl, SND_CTL_NAME_IEC958("",PLAYBACK,PCM_STREAM));
		snd_ctl_elem_value_set_iec958(ctl, &spdif);
		ctl_card = snd_pcm_info_get_card(info);
		if (ctl_card < 0) {
			fprintf(stderr, "Unable to setup the IEC958 (S/PDIF) interface - PCM has no assigned card\n");
			goto __diga_end;
		}
		sprintf(ctl_name, "hw:%d", ctl_card);
		if ((err = snd_ctl_open(&ctl_handle, ctl_name, 0)) < 0) {
			fprintf(stderr, "Unable to open the control interface '%s': %s\n", ctl_name, snd_strerror(err));
			goto __diga_end;
		}
		if ((err = snd_ctl_elem_write(ctl_handle, ctl)) < 0) {
			fprintf(stderr, "Unable to update the IEC958 control: %s\n", snd_strerror(err));
			goto __diga_end;
		}
		snd_ctl_close(ctl_handle);
	      __diga_end:
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
		if (res == -EPIPE)
			res = snd_pcm_prepare(pcm);
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
