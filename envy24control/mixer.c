/*****************************************************************************
   mixer.c - mixer code
   Copyright (C) 2000 by Jaroslav Kysela <perex@suse.cz>
   
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
******************************************************************************/

#include "envy24control.h"

#define toggle_set(widget, state) \
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), state);

static int is_active(GtkWidget *widget)
{
	return GTK_TOGGLE_BUTTON(widget)->active ? 1 : 0;
}

void mixer_update_stream(int stream, int vol_flag, int sw_flag)
{
	snd_control_t vol, sw;
	int err;
	
	if (vol_flag) {
		memset(&vol, 0, sizeof(vol));
		vol.id.iface = SND_CONTROL_IFACE_MIXER;
		strcpy(vol.id.name, stream <= 10 ? "Multi Playback Volume" : "Multi Capture Volume");
		vol.id.index = (stream - 1) % 10;
		if ((err = snd_ctl_cread(card_ctl, &vol)) < 0)
			g_print("Unable to read multi playback volume: %s\n", snd_strerror(err));
		gtk_adjustment_set_value(GTK_ADJUSTMENT(mixer_adj[stream-1][0]), 96 - vol.value.integer.value[0]);
		gtk_adjustment_set_value(GTK_ADJUSTMENT(mixer_adj[stream-1][1]), 96 - vol.value.integer.value[1]);
		if (vol.value.integer.value[0] != vol.value.integer.value[1])
			toggle_set(mixer_stereo_toggle[stream-1], FALSE);
	}
	if (sw_flag) {
		memset(&sw, 0, sizeof(sw));
		sw.id.iface = SND_CONTROL_IFACE_MIXER;
		strcpy(sw.id.name, stream <= 10 ? "Multi Playback Switch" : "Multi Capture Switch");
		sw.id.index = (stream - 1) % 10;
		if ((err = snd_ctl_cread(card_ctl, &sw)) < 0)
			g_print("Unable to read multi playback switch: %s\n", snd_strerror(err));
		toggle_set(mixer_solo_toggle[stream-1][0], sw.value.integer.value[0] ? TRUE : FALSE);
		toggle_set(mixer_solo_toggle[stream-1][1], sw.value.integer.value[1] ? TRUE : FALSE);
		toggle_set(mixer_mute_toggle[stream-1][0], !sw.value.integer.value[0] ? TRUE : FALSE);
		toggle_set(mixer_mute_toggle[stream-1][1], !sw.value.integer.value[1] ? TRUE : FALSE);
		if (sw.value.integer.value[0] != sw.value.integer.value[1])
			toggle_set(mixer_stereo_toggle[stream-1], FALSE);
	}
}

static void set_switch1(int stream, int left, int right)
{
	snd_control_t sw;
	int err;
	
	memset(&sw, 0, sizeof(sw));
	sw.id.iface = SND_CONTROL_IFACE_MIXER;
	strcpy(sw.id.name, stream <= 10 ? "Multi Playback Switch" : "Multi Capture Switch");
	sw.id.index = (stream - 1) % 10;
	if ((err = snd_ctl_cread(card_ctl, &sw)) < 0)
		g_print("Unable to read multi switch: %s\n", snd_strerror(err));
	if (left >= 0)
		sw.value.integer.value[0] = left != 0;
	if (right >= 0)
		sw.value.integer.value[1] = right != 0;
	if ((err = snd_ctl_cwrite(card_ctl, &sw)) < 0 && err != -EBUSY)
		g_print("Unable to write multi switch: %s\n", snd_strerror(err));
}

void mixer_toggled_solo(GtkWidget *togglebutton, gpointer data)
{
	int stream = (long)data >> 16;
	int button = (long)data & 1;
	int stereo = is_active(mixer_stereo_toggle[stream-1]) ? 1 : 0;
	int vol[2] = { -1, -1 };

	if (is_active(togglebutton)) {
		if (is_active(mixer_mute_toggle[stream-1][button]))
			toggle_set(mixer_mute_toggle[stream-1][button], FALSE);
		vol[button] = 1;
		if (stereo) {
			if (!is_active(mixer_solo_toggle[stream-1][button ^ 1]))
				toggle_set(mixer_solo_toggle[stream-1][button ^ 1], TRUE);
			if (is_active(mixer_mute_toggle[stream-1][button ^ 1]))
				toggle_set(mixer_mute_toggle[stream-1][button ^ 1], FALSE);
			vol[button ^ 1] = 1;
		}
	} else {
		if (!is_active(mixer_mute_toggle[stream-1][button]))
			toggle_set(mixer_mute_toggle[stream-1][button], TRUE);
		vol[button] = 0;
		if (stereo) {
			if (is_active(mixer_solo_toggle[stream-1][button ^ 1]))
				toggle_set(mixer_solo_toggle[stream-1][button ^ 1], FALSE);
			if (!is_active(mixer_mute_toggle[stream-1][button ^ 1]))
				toggle_set(mixer_mute_toggle[stream-1][button ^ 1], TRUE);
			vol[button ^ 1] = 0;
		}
	}
	set_switch1(stream, vol[0], vol[1]);
}

void mixer_toggled_mute(GtkWidget *togglebutton, gpointer data)
{
	int stream = (long)data >> 16;
	int button = (long)data & 1;
	int stereo = is_active(mixer_stereo_toggle[stream-1]) ? 1 : 0;
	int vol[2] = { -1, -1 };

	if (is_active(togglebutton)) {
		if (is_active(mixer_solo_toggle[stream-1][button]))
			toggle_set(mixer_solo_toggle[stream-1][button], FALSE);
		vol[button] = 0;
		if (stereo) {
			if (!is_active(mixer_mute_toggle[stream-1][button ^ 1]))
				toggle_set(mixer_mute_toggle[stream-1][button ^ 1], TRUE);
			if (is_active(mixer_solo_toggle[stream-1][button ^ 1]))
				toggle_set(mixer_solo_toggle[stream-1][button ^ 1], FALSE);
			vol[button ^ 1] = 0;
		}
	} else {
		if (!is_active(mixer_solo_toggle[stream-1][button]))
			toggle_set(mixer_solo_toggle[stream-1][button], TRUE);
		vol[button] = 1;
		if (stereo) {
			if (is_active(mixer_mute_toggle[stream-1][button ^ 1]))
				toggle_set(mixer_mute_toggle[stream-1][button ^ 1], FALSE);
			if (!is_active(mixer_solo_toggle[stream-1][button ^ 1]))
				toggle_set(mixer_solo_toggle[stream-1][button ^ 1], TRUE);
			vol[button ^ 1] = 1;
		}
	}
	set_switch1(stream, vol[0], vol[1]);
}

static void set_volume1(int stream, int left, int right)
{
	snd_control_t vol;
	int err;
	
	memset(&vol, 0, sizeof(vol));
	vol.id.iface = SND_CONTROL_IFACE_MIXER;
	strcpy(vol.id.name, stream <= 10 ? "Multi Playback Volume" : "Multi Capture Volume");
	vol.id.index = (stream - 1) % 10;
	if ((err = snd_ctl_cread(card_ctl, &vol)) < 0)
		g_print("Unable to read multi volume: %s\n", snd_strerror(err));
	if (left >= 0)
		vol.value.integer.value[0] = left;
	if (right >= 0)
		vol.value.integer.value[1] = right;
	if ((err = snd_ctl_cwrite(card_ctl, &vol)) < 0 && err != -EBUSY)
		g_print("Unable to write multi volume: %s\n", snd_strerror(err));
}

void mixer_adjust(GtkAdjustment *adj, gpointer data)
{
	int stream = (long)data >> 16;
	int button = (long)data & 1;
	int stereo = is_active(mixer_stereo_toggle[stream-1]) ? 1 : 0;
	int vol[2] = { -1, -1 };
	
	vol[button] = 96 - adj->value;
	if (stereo) {
		gtk_adjustment_set_value(GTK_ADJUSTMENT(mixer_adj[stream-1][button ^ 1]), adj->value);
		vol[button ^ 1] = 96 - adj->value;
	}
	set_volume1(stream, vol[0], vol[1]);
}

void mixer_postinit(void)
{
	int stream;

	for (stream = 1; stream <= 20; stream++)
		mixer_update_stream(stream, 1, 1);
}
