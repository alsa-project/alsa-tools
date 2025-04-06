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
		exit(EXIT_FAILURE);
	}
	return result;
}

static void redraw_meters(int idx, int width, int height, int level1, int level2, GtkSnapshot *snapshot)
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
	GdkRGBA black = { 0, 0, 0, 1 };

	// g_print("segs_on1 = %i (%i), segs_on2 = %i (%i)\n", segs_on1, level1, segs_on2, level2);
	gtk_snapshot_append_color(snapshot, &black, &GRAPHENE_RECT_INIT(0, 0, width, height));
	for (seg = 0; seg < green_segments; seg++) {
		gtk_snapshot_append_color(snapshot,
					  segs_on1 > 0 ? penGreenLight : penGreenShadow,
					  &GRAPHENE_RECT_INIT(
						  6, 3 + ((segments - seg - 1) * 4),
						  segment_width,
						  3));
		if (stereo) {
			gtk_snapshot_append_color(snapshot,
						  segs_on2 > 0 ? penGreenLight : penGreenShadow,
						  &GRAPHENE_RECT_INIT(
							  2 + (width / 2),
							  3 + ((segments - seg - 1) * 4),
							  segment_width,
							  3));
		}
		segs_on1--;
		segs_on2--;
	}
	for (; seg < green_segments + orange_segments; seg++) {
		gtk_snapshot_append_color(snapshot,
					  segs_on1 > 0 ? penOrangeLight : penOrangeShadow,
					  &GRAPHENE_RECT_INIT(
						  6, 3 + ((segments - seg - 1) * 4),
						  segment_width,
						  3));
		if (stereo) {
			gtk_snapshot_append_color(snapshot,
						  segs_on2 > 0 ? penOrangeLight : penOrangeShadow,
						  &GRAPHENE_RECT_INIT(
							  2 + (width / 2),
							  3 + ((segments - seg - 1) * 4),
							  segment_width,
							  3));
		}
		segs_on1--;
		segs_on2--;
	}
	for (; seg < segments; seg++) {
		gtk_snapshot_append_color(snapshot,
					  segs_on1 > 0 ? penRedLight : penRedShadow,
					  &GRAPHENE_RECT_INIT(
						  6, 3 + ((segments - seg - 1) * 4),
						  segment_width,
						  3));
		if (stereo) {
			gtk_snapshot_append_color(snapshot,
						  segs_on2 > 0 ? penRedLight : penRedShadow,
						  &GRAPHENE_RECT_INIT(
							  2 + (width / 2),
							  3 + ((segments - seg - 1) * 4),
							  segment_width,
							  3));
		}
		segs_on1--;
		segs_on2--;
	}
}

#define ENVY_TYPE_LEVEL_METER envy_level_meter_get_type()
G_DECLARE_FINAL_TYPE(EnvyLevelMeter, envy_level_meter, ENVY, LEVEL_METER, GtkWidget)

struct _EnvyLevelMeter
{
	GtkWidget parent_instance;
};

G_DEFINE_FINAL_TYPE(EnvyLevelMeter, envy_level_meter, GTK_TYPE_WIDGET)

static void envy_level_meter_snapshot(GtkWidget *widget, GtkSnapshot *snapshot)
{
	int idx = get_index(gtk_widget_get_name(widget));
	int l1, l2;
	
	get_levels(idx, &l1, &l2);
	redraw_meters(idx, gtk_widget_get_width(widget), gtk_widget_get_height(widget), l1, l2, snapshot);
}

static void envy_level_meter_class_init(EnvyLevelMeterClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
	widget_class->snapshot = envy_level_meter_snapshot;
}

static void envy_level_meter_init(EnvyLevelMeter *self)
{
}

GtkWidget *envy_level_meter_new(void)
{
	EnvyLevelMeter *self = g_object_new(ENVY_TYPE_LEVEL_METER, NULL);
	return GTK_WIDGET(self);
}

static void update_meter(int idx)
{
	GtkWidget *widget = idx == 0 ? mixer_mix_drawing : mixer_drawing[idx-1];
	gtk_widget_queue_draw(widget);
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
