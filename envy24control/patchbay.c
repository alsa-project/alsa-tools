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

static snd_ctl_element_t routes;

#define toggle_set(widget, state) \
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), state);

static int is_active(GtkWidget *widget)
{
	return GTK_TOGGLE_BUTTON(widget)->active ? 1 : 0;
}

static int get_toggle_index(int stream)
{
	unsigned short psdout = routes.value.bytes.data[0] |
				(routes.value.bytes.data[1] << 8);
	unsigned short spdout = routes.value.bytes.data[2] |
				(routes.value.bytes.data[3] << 8);
	unsigned int capture = routes.value.bytes.data[4] |
			       (routes.value.bytes.data[5] << 8) |
			       (routes.value.bytes.data[6] << 16) |
			       (routes.value.bytes.data[7] << 24);
	int right = (stream - 1) & 1;
	int source = (stream - 1) >> 1;

	stream--;
	if (stream < 0 || stream > 9) {
		g_print("get_toggle_index (1)\n");
		return 0;
	}
	if (stream < 8) {	/* SPDOUT */
		int psdout_shift = (source << 1) + (right ? 8 : 0);
		int capture_shift = (source << 3) + (right ? 4 : 0);
		int setup = (psdout >> psdout_shift) & 3;
		int csetup = (capture >> capture_shift) & 15;
		
		switch (setup) {
		case 0:					/* PCM Out */
			return 0;
		case 1:					/* digital mixer */
			if (stream == 0 || stream == 1)
				return 1;
			return 0;
		case 2:
			return (csetup & 7) + 4;
		case 3:
			if (csetup & 8)
				return 3;		/* S/PDIF right */
			return 2;			/* S/PDIF left */
		}
	} else {	/* SPDOUT */
		int spdout_shift = right ? 2 : 0;
		int spdout_shift1 = right ? 12 : 8;
		int setup = (spdout >> spdout_shift) & 3;
		int setup1 = (spdout >> spdout_shift1) & 15;
		
		switch (setup) {
		case 0:					/* PCM Out */
			return 0;
		case 1:					/* digital mixer */
			if (stream == 0 || stream == 1)
				return 1;
			return 0;
		case 2:
			return (setup1 & 7) + 4;
		case 3:
			if (setup1 & 8)
				return 3;		/* S/PDIF right */
			return 2;			/* S/PDIF left */
		}
	}
	return 0;
}

void patchbay_update(void)
{
	int stream, tidx, err;

	if ((err = snd_ctl_element_read(card_ctl, &routes)) < 0) {
		g_print("Multi track routes read error: %s\n", snd_strerror(err));
		return;
	}
	for (stream = 1; stream <= 10; stream++) {
		tidx = get_toggle_index(stream);
		toggle_set(router_radio[stream - 1][tidx], TRUE);
	}
}

static void set_routes(int stream, int idx)
{
	unsigned short psdout = routes.value.bytes.data[0] |
				(routes.value.bytes.data[1] << 8);
	unsigned short spdout = routes.value.bytes.data[2] |
				(routes.value.bytes.data[3] << 8);
	unsigned int capture = routes.value.bytes.data[4] |
			       (routes.value.bytes.data[5] << 8) |
			       (routes.value.bytes.data[6] << 16) |
			       (routes.value.bytes.data[7] << 24);
	int right = (stream - 1) & 1;
	int source = (stream - 1) >> 1;
	int err;

	stream--;
	if (stream < 0 || stream > 9) {
		g_print("set_routes (1)\n");
		return;
	}
	if (stream < 8) {	/* SPDOUT */
		int psdout_shift = (source << 1) + (right ? 8 : 0);
		int capture_shift = (source << 3) + (right ? 4 : 0);
		psdout &= ~(3 << psdout_shift);
		if (idx == 0) {				/* PCM Out */
			/* nothing */ ;
		} else if (idx == 1) {			/* digital mixer */
			if (stream == 0 || stream == 1)
				psdout |= 1 << psdout_shift;
		} else if (idx == 2 || idx == 3) {	/* S/PDIF left & right */
			psdout |= 3 << psdout_shift;
			capture &= ~(1 << (capture_shift + 3));
			capture |= (idx - 2) << (capture_shift + 3);
		} else {
			psdout |= 2 << psdout_shift;
			capture &= ~(7 << capture_shift);
			capture |= ((idx - 4) & 7) << capture_shift;
		}
	} else {	/* SPDOUT */
		int spdout_shift = right ? 2 : 0;
		int spdout_shift1 = right ? 12 : 8;
		spdout &= ~(3 << spdout_shift);
		if (idx == 0) {				/* PCM Out 9 & 10 */
			/* nothing */ ;
		} else if (idx == 1) {			/* digital mixer */
			spdout |= 1 << spdout_shift;
		} else if (idx == 2 || idx == 3) {	/* S/PDIF left & right */
			spdout |= 3 << spdout_shift;
			spdout &= ~(1 << (spdout_shift1 + 3));
			spdout |= (idx - 2) << (spdout_shift1 + 3);
		} else {
			spdout |= 2 << spdout_shift;
			spdout &= ~(7 << spdout_shift1);
			spdout |= ((idx - 4) & 7) << spdout_shift1;
		}
	}
	routes.value.bytes.data[0] = psdout & 0xff;
	routes.value.bytes.data[1] = (psdout >> 8) & 0xff;
	routes.value.bytes.data[2] = spdout & 0xff;
	routes.value.bytes.data[3] = (spdout >> 8) & 0xff;
	routes.value.bytes.data[4] = capture & 0xff;
	routes.value.bytes.data[5] = (capture >> 8) & 0xff;
	routes.value.bytes.data[6] = (capture >> 16) & 0xff;
	routes.value.bytes.data[7] = (capture >> 24) & 0xff;
	// g_print("psdout = 0x%x, spdout = 0x%x, capture = 0x%x\n", psdout, spdout, capture);
	if ((err = snd_ctl_element_write(card_ctl, &routes)) < 0)
		g_print("Multi track route write error: %s\n", snd_strerror(err));
}

void patchbay_toggled(GtkWidget *togglebutton, gpointer data)
{
	int stream = (long)data >> 16;
	int what = (long)data & 0xffff;

	if (is_active(togglebutton))
		set_routes(stream, what);
}

void patchbay_init(void)
{
	memset(&routes, 0, sizeof(routes));
	routes.id.iface = SND_CTL_ELEMENT_IFACE_MIXER;
	strcpy(routes.id.name, "Multi Track Route");
}

void patchbay_postinit(void)
{
	patchbay_update();
}
