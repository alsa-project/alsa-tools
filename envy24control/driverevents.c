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

void control_input_callback(gpointer data, gint source, GdkInputCondition condition)
{
	snd_ctl_t *ctl = (snd_ctl_t *)data;
	snd_ctl_event_t *ev;
	const char *name;
	int index;

	snd_ctl_event_alloca(&ev);
	if (snd_ctl_read(ctl, ev) < 0)
		return;
	name = snd_ctl_event_elem_get_name(ev);
	index = snd_ctl_event_elem_get_index(ev);
	switch (snd_ctl_event_elem_get_interface(ev)) {
	case SND_CTL_ELEM_IFACE_PCM:
		if (!strcmp(name, "Multi Track Route"))
			patchbay_update();
		else if (!strcmp(name, "Multi Track S/PDIF Master"))
			master_clock_update();
		else if (!strcmp(name, "Word Clock Sync"))
			master_clock_update();
		else if (!strcmp(name, "Multi Track Volume Rate"))
			volume_change_rate_update();
		else if (!strcmp(name, "S/PDIF Input Optical"))
			spdif_input_update();
		else if (!strcmp(name, "Delta S/PDIF Output Defaults"))
			spdif_output_update();
		break;
	case SND_CTL_ELEM_IFACE_MIXER:
		if (!strcmp(name, "Multi Playback Volume"))
			mixer_update_stream(index + 1, 1, 0);
		else if (!strcmp(name, "Multi Capture Volume"))
			mixer_update_stream(index + 11, 1, 0);
		else if (!strcmp(name, "Multi Playback Switch"))
			mixer_update_stream(index + 1, 0, 1);
		else if (!strcmp(name, "Multi Capture Switch"))
			mixer_update_stream(index + 11, 0, 1);
		break;
	default:
		break;
	}
}

