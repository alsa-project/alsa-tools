/*****************************************************************************
   driverevents.c - Events from the driver processing
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

static void control_value(snd_ctl_t *handle, void *private_data, snd_ctl_element_id_t *id)
{
	if (id->iface == SND_CTL_ELEMENT_IFACE_PCM) {
		if (!strcmp(id->name, "Multi Track Route")) {
			patchbay_update();
			return;
		}
		if (!strcmp(id->name, "Multi Track S/PDIF Master")) {
			master_clock_update();
			return;
		}
		if (!strcmp(id->name, "Word Clock Sync")) {
			master_clock_update();
			return;
		}
		if (!strcmp(id->name, "Multi Track Volume Rate")) {
			volume_change_rate_update();
			return;
		}
		if (!strcmp(id->name, "S/PDIF Input Optical")) {
			spdif_input_update();
			return;
		}
		if (!strcmp(id->name, "Delta S/PDIF Output Defaults")) {
			spdif_output_update();
			return;
		}
	}
	if (id->iface == SND_CTL_ELEMENT_IFACE_MIXER) {
		if (!strcmp(id->name, "Multi Playback Volume")) {
			mixer_update_stream(id->index + 1, 1, 0);
			return;
		}
		if (!strcmp(id->name, "Multi Capture Volume")) {
			mixer_update_stream(id->index + 11, 1, 0);
			return;
		}	
		if (!strcmp(id->name, "Multi Playback Switch")) {
			mixer_update_stream(id->index + 1, 0, 1);
			return;
		}
		if (!strcmp(id->name, "Multi Capture Switch")) {
			mixer_update_stream(id->index + 11, 0, 1);
			return;
		}
	}
}

static snd_ctl_callbacks_t control_callbacks = {
	private_data: NULL,
	rebuild: NULL,		/* FIXME!! */
	value: control_value,
	change: NULL,
	add: NULL,
	remove: NULL,
	reserved: { NULL, }
};

void control_input_callback(gpointer data, gint source, GdkInputCondition condition)
{
	snd_ctl_read(card_ctl, &control_callbacks);
}
