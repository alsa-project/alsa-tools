/*****************************************************************************
   patchbay.c - patchbay/router code
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

#define SPDIF_PLAYBACK_ROUTE_NAME	"IEC958 Playback Route"
#define ANALOG_PLAYBACK_ROUTE_NAME	"H/W Playback Route"

#define toggle_set(widget, state) \
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), state);

static int stream_active[10];

static int is_active(GtkWidget *widget)
{
	return GTK_TOGGLE_BUTTON(widget)->active ? 1 : 0;
}

static int get_toggle_index(int stream)
{
	int err, out;
	snd_ctl_elem_value_t *val;

	stream--;
	if (stream < 0 || stream > 9) {
		g_print("get_toggle_index (1)\n");
		return 0;
	}
	snd_ctl_elem_value_alloca(&val);
	snd_ctl_elem_value_set_interface(val, SND_CTL_ELEM_IFACE_MIXER);
	if (stream >= 8) {
		snd_ctl_elem_value_set_name(val, SPDIF_PLAYBACK_ROUTE_NAME);
		snd_ctl_elem_value_set_index(val, stream - 8);
	} else {
		snd_ctl_elem_value_set_name(val, ANALOG_PLAYBACK_ROUTE_NAME);
		snd_ctl_elem_value_set_index(val, stream);
	}
	if ((err = snd_ctl_elem_read(ctl, val)) < 0)
		return 0;
	out = snd_ctl_elem_value_get_enumerated(val, 0);
	if (out >= 11) {
		if (stream >= 8 || stream < 2)
			return 1; /* digital mixer */
	} else if (out >= 9)
		return out - 9 + 2; /* spdif left (=2) / right (=3) */
	else if (out >= 1)
		return out + 3; /* analog (4-) */

	return 0; /* pcm */
}

void patchbay_update(void)
{
	int stream, tidx;

	for (stream = 1; stream <= 10; stream++) {
		if (stream_active[stream - 1]) {
			tidx = get_toggle_index(stream);
			toggle_set(router_radio[stream - 1][tidx], TRUE);
		}
	}
}

static void set_routes(int stream, int idx)
{
	int err;
	unsigned int out;
	snd_ctl_elem_value_t *val;

	stream--;
	if (stream < 0 || stream > 9) {
		g_print("set_routes (1)\n");
		return;
	}
	if (! stream_active[stream])
		return;
	out = 0;
	if (idx == 1)
		out = 11;
	else if (idx == 2 || idx == 3)	/* S/PDIF left & right */
		out = idx + 7; /* 9-10 */
	else if (idx >= 4) /* analog */
		out = idx - 3; /* 1-8 */

	snd_ctl_elem_value_alloca(&val);
	snd_ctl_elem_value_set_interface(val, SND_CTL_ELEM_IFACE_MIXER);
	if (stream >= 8) {
		snd_ctl_elem_value_set_name(val, SPDIF_PLAYBACK_ROUTE_NAME);
		snd_ctl_elem_value_set_index(val, stream - 8);
	} else {
		snd_ctl_elem_value_set_name(val, ANALOG_PLAYBACK_ROUTE_NAME);
		snd_ctl_elem_value_set_index(val, stream);
	}

	snd_ctl_elem_value_set_enumerated(val, 0, out);
	if ((err = snd_ctl_elem_write(ctl, val)) < 0)
		g_print("Multi track route write error: %s\n", snd_strerror(err));
}

void patchbay_toggled(GtkWidget *togglebutton, gpointer data)
{
	int stream = (long)data >> 16;
	int what = (long)data & 0xffff;

	if (is_active(togglebutton))
		set_routes(stream, what);
}

int patchbay_stream_is_active(int stream)
{
	return stream_active[stream - 1];
}

void patchbay_init(void)
{
	int i;
	snd_ctl_elem_value_t *val;

	snd_ctl_elem_value_alloca(&val);
	snd_ctl_elem_value_set_interface(val, SND_CTL_ELEM_IFACE_MIXER);
	snd_ctl_elem_value_set_name(val, ANALOG_PLAYBACK_ROUTE_NAME);
	for (i = 0; i < 8; i++) {
		snd_ctl_elem_value_set_numid(val, 0);
		snd_ctl_elem_value_set_index(val, i);
		if (snd_ctl_elem_read(ctl, val) < 0)
			continue;

		stream_active[i] = 1;
	}
	snd_ctl_elem_value_set_name(val, SPDIF_PLAYBACK_ROUTE_NAME);
	for (i = 0; i < 2; i++) {
		snd_ctl_elem_value_set_numid(val, 0);
		snd_ctl_elem_value_set_index(val, i);
 		if (snd_ctl_elem_read(ctl, val) < 0)
			continue;
		stream_active[i + 8] = 1;
	}
}

void patchbay_postinit(void)
{
	patchbay_update();
}
