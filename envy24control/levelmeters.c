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

static GdkRGBA *penGreenShadow[21] = { NULL, };
static GdkRGBA *penGreenLight[21] = { NULL, };
static GdkRGBA *penOrangeShadow[21] = { NULL, };
static GdkRGBA *penOrangeLight[21] = { NULL, };
static GdkRGBA *penRedShadow[21] = { NULL, };
static GdkRGBA *penRedLight[21] = { NULL, };
static GdkPixbuf *pixmap[21] = { NULL, };
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

static GdkRGBA *get_pen(int idx, int nRed, int nGreen, int nBlue)
{
    GdkRGBA *c = g_malloc(sizeof(GdkRGBA));
    c->red = nRed / 255.0;
    c->green = nGreen / 255.0;
    c->blue = nBlue / 255.0;
    c->alpha = 1.0; // Fully opaque
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

static void redraw_meters(int idx, int width, int height, int level1, int level2)
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

    GdkPixbuf *pixbuf = pixmap[idx];
    // Create a Cairo surface from the GdkPixbuf
    cairo_surface_t *surface = gdk_cairo_surface_create_from_pixbuf(pixbuf, 0, NULL);
    // Create a Cairo context from the surface
    cairo_t *cr = cairo_create(surface);

	// g_print("segs_on1 = %i (%i), segs_on2 = %i (%i)\n", segs_on1, level1, segs_on2, level2);
	for (seg = 0; seg < green_segments; seg++) {
		// Set the drawing color
		if (segs_on1 > 0) {
		    cairo_set_source_rgb(cr, penGreenLight[idx]->red,
		                             penGreenLight[idx]->green,
		                             penGreenLight[idx]->blue);
		} else {
		    cairo_set_source_rgb(cr, penGreenShadow[idx]->red,
		                             penGreenShadow[idx]->green,
		                             penGreenShadow[idx]->blue);
		}

		// Set the rectangle and draw
		cairo_rectangle(cr, 6, 3 + ((segments - seg - 1) * 4), segment_width, 3);
		cairo_fill(cr);


		if (stereo)
		{
			// Set the drawing color
			if (segs_on2 > 0) {
			    cairo_set_source_rgb(cr, penGreenLight[idx]->red,
			                             penGreenLight[idx]->green,
			                             penGreenLight[idx]->blue);
			} else {
			    cairo_set_source_rgb(cr, penGreenShadow[idx]->red,
			                             penGreenShadow[idx]->green,
			                             penGreenShadow[idx]->blue);
			}

			// Set the rectangle and draw
			cairo_rectangle(cr, 2 + (width / 2), 3 + ((segments - seg - 1) * 4), segment_width, 3);
			cairo_fill(cr);
		}
		segs_on1--;
		segs_on2--;

	}
	for (seg = green_segments; seg < green_segments + orange_segments; seg++) {
		// Set the drawing color
		if (segs_on1 > 0) {
		    cairo_set_source_rgb(cr, penOrangeLight[idx]->red,
		                             penOrangeLight[idx]->green,
		                             penOrangeLight[idx]->blue);
		} else {
		    cairo_set_source_rgb(cr, penOrangeShadow[idx]->red,
		                             penOrangeShadow[idx]->green,
		                             penOrangeShadow[idx]->blue);
		}

		// Set the rectangle and draw
		cairo_rectangle(cr, 6, 3 + ((segments - seg - 1) * 4), segment_width, 3);
		cairo_fill(cr);

		if (stereo)
		{

			// Set the drawing color
			if (segs_on2 > 0) {
			    cairo_set_source_rgb(cr, penOrangeLight[idx]->red,
			                             penOrangeLight[idx]->green,
			                             penOrangeLight[idx]->blue);
			} else {
			    cairo_set_source_rgb(cr, penOrangeShadow[idx]->red,
			                             penOrangeShadow[idx]->green,
			                             penOrangeShadow[idx]->blue);
			}

			// Set the rectangle and draw
			cairo_rectangle(cr, 2 + (width / 2), 3 + ((segments - seg - 1) * 4), segment_width, 3);
			cairo_fill(cr);
		}

		segs_on1--;
		segs_on2--;
	}
	for (seg = green_segments + orange_segments; seg < segments; seg++) {

		// Set the drawing color
		if (segs_on1 > 0) {
		    cairo_set_source_rgb(cr, penRedLight[idx]->red,
		                             penRedLight[idx]->green,
		                             penRedLight[idx]->blue);
		} else {
		    cairo_set_source_rgb(cr, penRedShadow[idx]->red,
		                             penRedShadow[idx]->green,
		                             penRedShadow[idx]->blue);
		}

		// Set the rectangle and draw
		cairo_rectangle(cr, 6, 3 + ((segments - seg - 1) * 4), segment_width, 3);
		cairo_fill(cr);

		if (stereo)
		{
			// Set the drawing color based on segs_on2
			if (segs_on2 > 0) {
			    cairo_set_source_rgb(cr, penRedLight[idx]->red,
			                             penRedLight[idx]->green,
			                             penRedLight[idx]->blue);
			} else {
			    cairo_set_source_rgb(cr, penRedShadow[idx]->red,
			                             penRedShadow[idx]->green,
			                             penRedShadow[idx]->blue);
			}

			// Set the rectangle and draw
			cairo_rectangle(cr, 2 + (width / 2), 3 + ((segments - seg - 1) * 4), segment_width, 3);
			cairo_fill(cr);
		}

		segs_on1--;
		segs_on2--;
	}
	// Clean up the Cairo context
	cairo_destroy(cr);
}

gint level_meters_configure_event(GtkWidget *widget, GdkEventConfigure *event)
{
	int idx = get_index(gtk_widget_get_name(widget));

	if (pixmap[idx] != NULL)
		g_object_unref(pixmap[idx]);

	cairo_surface_t *surface = gdk_window_create_similar_surface(gtk_widget_get_window(widget),
	    CAIRO_FORMAT_ARGB32, gtk_widget_get_allocated_width(widget), gtk_widget_get_allocated_height(widget));
	// Create a GdkPixbuf from the cairo surface
	pixmap[idx] = gdk_pixbuf_get_from_surface(surface, 0, 0, gtk_widget_get_allocated_width(widget), gtk_widget_get_allocated_height(widget));

	penGreenShadow[idx] = get_pen(idx, 0, 0x77ff, 0);
	penGreenLight[idx] = get_pen(idx, 0, 0xffff, 0);
	penOrangeShadow[idx] = get_pen(idx, 0xddff, 0x55ff, 0);
	penOrangeLight[idx] = get_pen(idx, 0xffff, 0x99ff, 0);
	penRedShadow[idx] = get_pen(idx, 0xaaff, 0, 0);
	penRedLight[idx] = get_pen(idx, 0xffff, 0, 0);

	// Get the window for the widget
	GdkWindow *window = gtk_widget_get_window(widget);
	// Begin drawing on the window (this replaces gdk_cairo_create)
	GdkDrawingContext *context = gdk_window_begin_draw_frame(window, NULL);
	// Get the Cairo context from the drawing context
	cairo_t *cr = gdk_drawing_context_get_cairo_context(context);
	//cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(widget)); //DEPRECATED
	GtkAllocation allocation;
	gtk_widget_get_allocation(widget, &allocation);

	// Set the source color (black in this case)
	cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);  // RGB for black
	// Create the rectangle with the specified dimensions
	cairo_rectangle(cr, 0, 0, allocation.width, allocation.height);
	// Fill the rectangle (similar to the "TRUE" parameter for filled in gdk_draw_rectangle)
	cairo_fill(cr);

	// g_print("configure: %i:%i\n", allocation.width, allocation.height);
	redraw_meters(idx, allocation.width, allocation.height, 0, 0);

    gdk_window_end_draw_frame(window, NULL); // End the draw frame

	// Don't forget to manage memory and clean up
	cairo_surface_destroy(surface);

	return TRUE;
}

gint level_meters_expose_event(GtkWidget *widget, GdkEventExpose *event)
{
	int idx = get_index(gtk_widget_get_name(widget));
	int l1, l2;

	get_levels(idx, &l1, &l2);
	GtkAllocation allocation;
	gtk_widget_get_allocation(widget, &allocation);
	redraw_meters(idx, allocation.width, allocation.height, l1, l2);

	// Get the window for the widget
	GdkWindow *window = gtk_widget_get_window(widget);
	// Begin drawing on the window (this replaces gdk_cairo_create)
	GdkDrawingContext *context = gdk_window_begin_draw_frame(window, NULL);
	// Get the Cairo context from the drawing context
	cairo_t *cr = gdk_drawing_context_get_cairo_context(context);
	// Convert GdkPixbuf to Cairo surface
	cairo_surface_t *surface = gdk_cairo_surface_create_from_pixbuf(pixmap[idx], 0, gtk_widget_get_window(widget));
	// Set the source surface to the Cairo context
	cairo_set_source_surface(cr, surface, event->area.x, event->area.y);
	// Define the area to draw and fill it
	cairo_rectangle(cr, event->area.x, event->area.y, event->area.width, event->area.height);
	cairo_fill(cr);
	// Clean up the Cairo surface
	cairo_surface_destroy(surface);
	// End the drawing frame
	gdk_window_end_draw_frame(window, context);

	return FALSE;
}

//TODO: Reduce/Remove/Refactor repeated code
gint level_meters_timeout_callback(gpointer data)
{
	GtkWidget *widget;
	int idx, l1, l2;

	update_peak_switch();
	for (idx = 0; idx <= pcm_output_channels; idx++) {
		get_levels(idx, &l1, &l2);
		widget = idx == 0 ? mixer_mix_drawing : mixer_drawing[idx-1];
		if (gtk_widget_get_visible(widget) && (pixmap[idx] != NULL)) {
			GtkAllocation allocation;
			gtk_widget_get_allocation(widget, &allocation);
			redraw_meters(idx, allocation.width, allocation.height, l1, l2);

			// Get the window for the widget
			GdkWindow *window = gtk_widget_get_window(widget);
			// Begin drawing on the window
			GdkDrawingContext *context = gdk_window_begin_draw_frame(window, NULL);
			// Get the Cairo context from the drawing context
			cairo_t *cr = gdk_drawing_context_get_cairo_context(context);
			// Create a Cairo surface from the pixmap
			cairo_surface_t *surface = gdk_cairo_surface_create_from_pixbuf(pixmap[idx], 0, window);
			// Set the source surface to the pixmap (Cairo surface)
			cairo_set_source_surface(cr, surface, 0, 0);
			// Define the area to draw (same as the width and height of the widget)
			cairo_rectangle(cr, 0, 0, allocation.width, allocation.height);
			cairo_fill(cr); // Fill the rectangle with the pixmap
			// Clean up the Cairo surface
			cairo_surface_destroy(surface);
			// End the drawing frame
			gdk_window_end_draw_frame(window, context);
		}
	}
	if (view_spdif_playback) {
		for (idx = MAX_PCM_OUTPUT_CHANNELS + 1; idx <= MAX_OUTPUT_CHANNELS + spdif_channels; idx++) {
			get_levels(idx, &l1, &l2);
			widget = idx == 0 ? mixer_mix_drawing : mixer_drawing[idx-1];
			if (gtk_widget_get_visible(widget) && (pixmap[idx] != NULL)) {
				GtkAllocation allocation;
				gtk_widget_get_allocation(widget, &allocation);
				redraw_meters(idx, allocation.width, allocation.height, l1, l2);

			// Get the window for the widget
			GdkWindow *window = gtk_widget_get_window(widget);
			// Begin drawing on the window
			GdkDrawingContext *context = gdk_window_begin_draw_frame(window, NULL);
			// Get the Cairo context from the drawing context
			cairo_t *cr = gdk_drawing_context_get_cairo_context(context);
			// Create a Cairo surface from the pixmap
			cairo_surface_t *surface = gdk_cairo_surface_create_from_pixbuf(pixmap[idx], 0, window);
			// Set the source surface to the pixmap (Cairo surface)
			cairo_set_source_surface(cr, surface, 0, 0);
			// Define the area to draw (same as the width and height of the widget)
			cairo_rectangle(cr, 0, 0, allocation.width, allocation.height);
			cairo_fill(cr); // Fill the rectangle with the pixmap
			// Clean up the Cairo surface
			cairo_surface_destroy(surface);
			// End the drawing frame
			gdk_window_end_draw_frame(window, context);
			}
		}
	}
	for (idx = MAX_PCM_OUTPUT_CHANNELS + MAX_SPDIF_CHANNELS + 1; idx <= input_channels + MAX_PCM_OUTPUT_CHANNELS + MAX_SPDIF_CHANNELS; idx++) {
		get_levels(idx, &l1, &l2);
		widget = idx == 0 ? mixer_mix_drawing : mixer_drawing[idx-1];
		if (gtk_widget_get_visible(widget) && (pixmap[idx] != NULL)) {
			GtkAllocation allocation;
			gtk_widget_get_allocation(widget, &allocation);
			redraw_meters(idx, allocation.width, allocation.height, l1, l2);

			// Get the window for the widget
			GdkWindow *window = gtk_widget_get_window(widget);
			// Begin drawing on the window
			GdkDrawingContext *context = gdk_window_begin_draw_frame(window, NULL);
			// Get the Cairo context from the drawing context
			cairo_t *cr = gdk_drawing_context_get_cairo_context(context);
			// Create a Cairo surface from the pixmap
			cairo_surface_t *surface = gdk_cairo_surface_create_from_pixbuf(pixmap[idx], 0, window);
			// Set the source surface to the pixmap (Cairo surface)
			cairo_set_source_surface(cr, surface, 0, 0);
			// Define the area to draw (same as the width and height of the widget)
			cairo_rectangle(cr, 0, 0, allocation.width, allocation.height);
			cairo_fill(cr); // Fill the rectangle with the pixmap
			// Clean up the Cairo surface
			cairo_surface_destroy(surface);
			// End the drawing frame
			gdk_window_end_draw_frame(window, context);
		}
	}
	for (idx = MAX_PCM_OUTPUT_CHANNELS + MAX_SPDIF_CHANNELS + MAX_INPUT_CHANNELS + 1; \
		    idx <= spdif_channels + MAX_PCM_OUTPUT_CHANNELS + MAX_SPDIF_CHANNELS + MAX_INPUT_CHANNELS; idx++) {
		get_levels(idx, &l1, &l2);
		widget = idx == 0 ? mixer_mix_drawing : mixer_drawing[idx-1];
		if (gtk_widget_get_visible(widget) && (pixmap[idx] != NULL)) {
			GtkAllocation allocation;
			gtk_widget_get_allocation(widget, &allocation);
			redraw_meters(idx, allocation.width, allocation.height, l1, l2);

			// Get the window for the widget
			GdkWindow *window = gtk_widget_get_window(widget);
			// Begin drawing on the window
			GdkDrawingContext *context = gdk_window_begin_draw_frame(window, NULL);
			// Get the Cairo context from the drawing context
			cairo_t *cr = gdk_drawing_context_get_cairo_context(context);
			// Create a Cairo surface from the pixmap
			cairo_surface_t *surface = gdk_cairo_surface_create_from_pixbuf(pixmap[idx], 0, window);
			// Set the source surface to the pixmap (Cairo surface)
			cairo_set_source_surface(cr, surface, 0, 0);
			// Define the area to draw (same as the width and height of the widget)
			cairo_rectangle(cr, 0, 0, allocation.width, allocation.height);
			cairo_fill(cr); // Fill the rectangle with the pixmap
			// Clean up the Cairo surface
			cairo_surface_destroy(surface);
			// End the drawing frame
			gdk_window_end_draw_frame(window, context);
		}
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
}

void level_meters_postinit(void)
{
	level_meters_timeout_callback(NULL);
}
