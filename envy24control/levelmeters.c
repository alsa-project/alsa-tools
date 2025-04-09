/*****************************************************************************
   levelmeters.c - Stereo level meters
   Copyright (C) 2000 by Jaroslav Kysela <perex@perex.cz>
   
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
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
******************************************************************************/

#include "envy24control.h"

static GdkRGBA *penGreenShadow = NULL;
static GdkRGBA *penGreenLight = NULL;
static GdkRGBA *penOrangeShadow = NULL;
static GdkRGBA *penOrangeLight = NULL;
static GdkRGBA *penRedShadow = NULL;
static GdkRGBA *penRedLight = NULL;
static int level[22] = { 0 };
static snd_ctl_elem_value_t *peaks;

extern int input_channels, output_channels, pcm_output_channels, spdif_channels, view_spdif_playback;

static void update_peak_switch(void)
{
	int err;

	if ((err = snd_ctl_elem_read(ctl, peaks)) < 0)
		g_print("Unable to read peaks: %s\n", snd_strerror(err));
}

static void get_levels(int idx, int *l1, int *l2)
{
	*l1 = *l2 = 0;
	
	if (idx == 0) {
		*l1 = snd_ctl_elem_value_get_integer(peaks, 20);
		*l2 = snd_ctl_elem_value_get_integer(peaks, 21);
	} else {
		*l1 = *l2 = snd_ctl_elem_value_get_integer(peaks, idx - 1);
	}
}

static GdkRGBA *get_pen(int nRed, int nGreen, int nBlue)
{
	GdkRGBA *c;
	
	c = (GdkRGBA *)g_malloc(sizeof(GdkRGBA));
	c->red = nRed / 65535.0;
	c->green = nGreen / 65535.0;
	c->blue = nBlue / 65535.0;
	c->alpha = 1.0;
	return c;
}

static int get_index(const gchar *name)
{
	int result;

	if (!strcmp(name, "DigitalMixer"))
		return 0;
	result = atoi(name + 5);
	if (result < 1 || result > 20) {
		g_print("Wrong drawing area ID: %s\n", name);
		gtk_main_quit();
	}
	return result;
}

static void redraw_meters(int idx, int width, int height, int level1, int level2, cairo_t *cr)
{
	int stereo = idx == 0;
	int segment_width = stereo ? (width / 2) - 8 : width - 12;
	int segments = (height - 6) / 4;
	int green_segments = (segments / 4) * 3;
	int red_segments = 2;
	int orange_segments = segments - green_segments - red_segments;
	int seg;
	int segs_on1 = ((segments * level1) + 128) / 255;
	int segs_on2 = ((segments * level2) + 128) / 255;
	int end_seg;
	GdkRectangle clip;

	// g_print("segs_on1 = %i (%i), segs_on2 = %i (%i)\n", segs_on1, level1, segs_on2, level2);
	cairo_rectangle(cr, 0, 0, width, height);
	cairo_fill(cr);

	gdk_cairo_get_clip_rectangle(cr, &clip);
	seg = segments - (clip.y + clip.height) / 4;
	if (seg < 0)
		seg = 0;
	segs_on1 -= seg;
	segs_on2 -= seg;
	end_seg = segments - (clip.y - 2) / 4;

	for (; seg < green_segments && seg < end_seg; seg++) {
		gdk_cairo_set_source_rgba(cr,
					  segs_on1 > 0 ? penGreenLight : penGreenShadow);
		cairo_rectangle(cr,
				6, 3 + ((segments - seg - 1) * 4),
				segment_width,
				3);
		cairo_fill(cr);
		if (stereo) {
			gdk_cairo_set_source_rgba(cr,
						  segs_on2 > 0 ? penGreenLight : penGreenShadow);
			cairo_rectangle(cr,
					2 + (width / 2),
					3 + ((segments - seg - 1) * 4),
					segment_width,
					3);
			cairo_fill(cr);
                }
		segs_on1--;
		segs_on2--;
	}
	for (; seg < green_segments + orange_segments && seg < end_seg; seg++) {
		gdk_cairo_set_source_rgba(cr,
					  segs_on1 > 0 ? penOrangeLight : penOrangeShadow);
		cairo_rectangle(cr,
				6, 3 + ((segments - seg - 1) * 4),
				segment_width,
				3);
		cairo_fill(cr);
		if (stereo) {
			gdk_cairo_set_source_rgba(cr,
						  segs_on2 > 0 ? penOrangeLight : penOrangeShadow);
			cairo_rectangle(cr,
					2 + (width / 2),
					3 + ((segments - seg - 1) * 4),
					segment_width,
					3);
			cairo_fill(cr);
		}
		segs_on1--;
		segs_on2--;
	}
	for (; seg < segments && seg < end_seg; seg++) {
		gdk_cairo_set_source_rgba(cr,
					  segs_on1 > 0 ? penRedLight : penRedShadow);
		cairo_rectangle(cr,
				6, 3 + ((segments - seg - 1) * 4),
				segment_width,
				3);
		cairo_fill(cr);
		if (stereo) {
			gdk_cairo_set_source_rgba(cr,
						  segs_on2 > 0 ? penRedLight : penRedShadow);
			cairo_rectangle(cr,
					2 + (width / 2),
					3 + ((segments - seg - 1) * 4),
					segment_width,
					3);
			cairo_fill(cr);
		}
		segs_on1--;
		segs_on2--;
	}
}

gboolean level_meters_draw_callback(GtkWidget *widget, cairo_t *cr, gpointer data)
{
	int idx = get_index(gtk_widget_get_name(widget));
	int l1, l2;
	
	get_levels(idx, &l1, &l2);
	redraw_meters(idx, gtk_widget_get_allocated_width(widget), gtk_widget_get_allocated_height(widget), l1, l2, cr);
	return FALSE;
}

static void update_meter(int idx)
{
	int stereo = idx == 0;
	GtkWidget *widget = stereo ? mixer_mix_drawing : mixer_drawing[idx - 1];
	int width = gtk_widget_get_allocated_width(widget);
	int height = gtk_widget_get_allocated_height(widget);
	int segments = (height - 6) / 4;
	int level_idx = stereo ? 20 : idx - 1;
	int l1, l2, segs_on, old_segs_on, h;

	get_levels(idx, &l1, &l2);
	segs_on = ((segments * l1) + 128) / 255;
	old_segs_on = ((segments * level[level_idx]) + 128) / 255;
	h = abs(old_segs_on - segs_on);
	level[level_idx] = l1;

	if (h > 0) {
		int y = segments - MAX(old_segs_on, segs_on);
		gtk_widget_queue_draw_area(widget,
					   6, 4 * y + 3,
					   stereo ? (width / 2) - 8 : width - 12,
					   4 * h - 1);
	}

	if (stereo) {
		level_idx++;
		segs_on = ((segments * l2) + 128) / 255;
		old_segs_on = ((segments * level[level_idx]) + 128) / 255;
		h = abs(old_segs_on - segs_on);
		level[level_idx] = l2;

		if (h > 0) {
			int y = segments - MAX(old_segs_on, segs_on);
			gtk_widget_queue_draw_area(widget,
						   2 + (width / 2), 4 * y + 3,
						   (width / 2) - 8,
						   4 * h - 1);
		}
	}
}

gint level_meters_timeout_callback(gpointer data)
{
	int idx;

	update_peak_switch();
	for (idx = 0; idx <= pcm_output_channels; idx++) {
		update_meter(idx);
	}
	if (view_spdif_playback) {
		for (idx = MAX_PCM_OUTPUT_CHANNELS + 1; idx <= MAX_OUTPUT_CHANNELS + spdif_channels; idx++) {
			update_meter(idx);
		}
	}
	for (idx = MAX_PCM_OUTPUT_CHANNELS + MAX_SPDIF_CHANNELS + 1; idx <= input_channels + MAX_PCM_OUTPUT_CHANNELS + MAX_SPDIF_CHANNELS; idx++) {
		update_meter(idx);
	}
	for (idx = MAX_PCM_OUTPUT_CHANNELS + MAX_SPDIF_CHANNELS + MAX_INPUT_CHANNELS + 1; \
		    idx <= spdif_channels + MAX_PCM_OUTPUT_CHANNELS + MAX_SPDIF_CHANNELS + MAX_INPUT_CHANNELS; idx++) {
		update_meter(idx);
	}
	return TRUE;
}

void level_meters_reset_peaks(GtkButton *button, gpointer data)
{
}

void level_meters_init(void)
{
	int err;

	snd_ctl_elem_value_malloc(&peaks);
	snd_ctl_elem_value_set_interface(peaks, SND_CTL_ELEM_IFACE_PCM);
	snd_ctl_elem_value_set_name(peaks, "Multi Track Peak");
	if ((err = snd_ctl_elem_read(ctl, peaks)) < 0)
		/* older ALSA driver, using MIXER type */
		snd_ctl_elem_value_set_interface(peaks,
			SND_CTL_ELEM_IFACE_MIXER);

	penGreenShadow = get_pen(0, 0x77ff, 0);
	penGreenLight = get_pen(0, 0xffff, 0);
	penOrangeShadow = get_pen(0xddff, 0x55ff, 0);
	penOrangeLight = get_pen(0xffff, 0x99ff, 0);
	penRedShadow = get_pen(0xaaff, 0, 0);
	penRedLight = get_pen(0xffff, 0, 0);
}

void level_meters_postinit(void)
{
	level_meters_timeout_callback(NULL);
}
