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

extern int input_channels, output_channels, spdif_channels;

static int is_active(GtkWidget *widget)
{
	return GTK_TOGGLE_BUTTON(widget)->active ? 1 : 0;
}

void mixer_update_stream(int stream, int vol_flag, int sw_flag)
{
	int err;
	
	if (vol_flag) {
		snd_ctl_elem_value_t *vol;
		int v[2];
		snd_ctl_elem_value_alloca(&vol);
		snd_ctl_elem_value_set_interface(vol, SND_CTL_ELEM_IFACE_MIXER);
		snd_ctl_elem_value_set_name(vol, stream <= 10 ? "Multi Playback Volume" : "Multi Capture Volume");
		snd_ctl_elem_value_set_index(vol, (stream - 1) % 10);
		if ((err = snd_ctl_elem_read(ctl, vol)) < 0)
			g_print("Unable to read multi playback volume: %s\n", snd_strerror(err));
		v[0] = snd_ctl_elem_value_get_integer(vol, 0);
		v[1] = snd_ctl_elem_value_get_integer(vol, 1);
		if (v[0] != v[1])
			toggle_set(mixer_stereo_toggle[stream-1], FALSE);
		gtk_adjustment_set_value(GTK_ADJUSTMENT(mixer_adj[stream-1][0]), 96 - v[0]);
		gtk_adjustment_set_value(GTK_ADJUSTMENT(mixer_adj[stream-1][1]), 96 - v[1]);
	}
	if (sw_flag) {
		snd_ctl_elem_value_t *sw;
		int v[2];
		snd_ctl_elem_value_alloca(&sw);
		snd_ctl_elem_value_set_interface(sw, SND_CTL_ELEM_IFACE_MIXER);
		snd_ctl_elem_value_set_name(sw, stream <= 10 ? "Multi Playback Switch" : "Multi Capture Switch");
		snd_ctl_elem_value_set_index(sw, (stream - 1) % 10);
		if ((err = snd_ctl_elem_read(ctl, sw)) < 0)
			g_print("Unable to read multi playback switch: %s\n", snd_strerror(err));
		v[0] = snd_ctl_elem_value_get_boolean(sw, 0);
		v[1] = snd_ctl_elem_value_get_boolean(sw, 1);
		if (v[0] != v[1])
			toggle_set(mixer_stereo_toggle[stream-1], FALSE);
		toggle_set(mixer_mute_toggle[stream-1][0], !v[0] ? TRUE : FALSE);
		toggle_set(mixer_mute_toggle[stream-1][1], !v[1] ? TRUE : FALSE);
	}
}

static void set_switch1(int stream, int left, int right)
{
	snd_ctl_elem_value_t *sw;
	int err, changed = 0;
	
	snd_ctl_elem_value_alloca(&sw);
	snd_ctl_elem_value_set_interface(sw, SND_CTL_ELEM_IFACE_MIXER);
	snd_ctl_elem_value_set_name(sw, stream <= 10 ? "Multi Playback Switch" : "Multi Capture Switch");
	snd_ctl_elem_value_set_index(sw, (stream - 1) % 10);
	if ((err = snd_ctl_elem_read(ctl, sw)) < 0)
		g_print("Unable to read multi switch: %s\n", snd_strerror(err));
	if (left >= 0 && left != snd_ctl_elem_value_get_boolean(sw, 0)) {
		snd_ctl_elem_value_set_boolean(sw, 0, left);
		changed = 1;
	}
	if (right >= 0 && right != snd_ctl_elem_value_get_boolean(sw, 1)) {
		snd_ctl_elem_value_set_boolean(sw, 1, right);
		changed = 1;
	}
	if (changed) {
		err = snd_ctl_elem_write(ctl, sw);
		if (err < 0)
			g_print("Unable to write multi switch: %s\n", snd_strerror(err));
	}
}

void mixer_toggled_mute(GtkWidget *togglebutton, gpointer data)
{
	int stream = (long)data >> 16;
	int button = (long)data & 1;
	int stereo = is_active(mixer_stereo_toggle[stream-1]) ? 1 : 0;
	int mute;
	int vol[2] = { -1, -1 };
	
	mute = is_active(mixer_mute_toggle[stream-1][button]);
	vol[button] = !mute;
	if (stereo) {
		toggle_set(mixer_mute_toggle[stream-1][button^1], mute);
		vol[button^1] = !mute;
	}
	set_switch1(stream, vol[0], vol[1]);
}

static void set_volume1(int stream, int left, int right)
{
	snd_ctl_elem_value_t *vol;
	int change = 0;
	int err;
	
	snd_ctl_elem_value_alloca(&vol);
	snd_ctl_elem_value_set_interface(vol, SND_CTL_ELEM_IFACE_MIXER);
	snd_ctl_elem_value_set_name(vol, stream <= 10 ? "Multi Playback Volume" : "Multi Capture Volume");
	snd_ctl_elem_value_set_index(vol, (stream - 1) % 10);
	if ((err = snd_ctl_elem_read(ctl, vol)) < 0)
		g_print("Unable to read multi volume: %s\n", snd_strerror(err));
	if (left >= 0) {
		change |= (snd_ctl_elem_value_get_integer(vol, 0) != left);
		snd_ctl_elem_value_set_integer(vol, 0, left);
	}
	if (right >= 0) {
		change |= (snd_ctl_elem_value_get_integer(vol, 1) != right);
		snd_ctl_elem_value_set_integer(vol, 1, right);
	}
	if (change) {
		if ((err = snd_ctl_elem_write(ctl, vol)) < 0 && err != -EBUSY)
			g_print("Unable to write multi volume: %s\n", snd_strerror(err));
	}
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

	for (stream = 1; stream <= output_channels; stream++)
		mixer_update_stream(stream, 1, 1);
	for (stream = 11; stream <= input_channels + 10; stream++)
		mixer_update_stream(stream, 1, 1);
	for (stream = 19; stream <= spdif_channels + 18; stream++)
		mixer_update_stream(stream, 1, 1);
}
