/*****************************************************************************
   envy24control.c - Env24 chipset (ICE1712) control utility
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
#define _GNU_SOURCE
#include <getopt.h>

ice1712_eeprom_t card_eeprom;
snd_ctl_t *ctl;

GtkWidget *window;

GtkWidget *mixer_mix_drawing;
GtkWidget *mixer_clear_peaks_button;
GtkWidget *mixer_drawing[20];
GtkObject *mixer_adj[20][2];
GtkWidget *mixer_vscale[20][2];
GtkWidget *mixer_mute_toggle[20][2];
GtkWidget *mixer_stereo_toggle[20];

GtkWidget *router_radio[10][12];

GtkWidget *hw_master_clock_xtal_radio;
GtkWidget *hw_master_clock_spdif_radio;
GtkWidget *hw_master_clock_word_radio;
GtkWidget *hw_master_clock_status_label;

GtkObject *hw_volume_change_adj;
GtkWidget *hw_volume_change_spin;

GtkWidget *hw_spdif_profi_nonaudio_radio;
GtkWidget *hw_spdif_profi_audio_radio;

GtkWidget *hw_profi_stream_stereo_radio;
GtkWidget *hw_profi_stream_notid_radio;

GtkWidget *hw_profi_emphasis_none_radio;
GtkWidget *hw_profi_emphasis_5015_radio;
GtkWidget *hw_profi_emphasis_ccitt_radio;
GtkWidget *hw_profi_emphasis_notid_radio;

GtkWidget *hw_consumer_copyright_on_radio;
GtkWidget *hw_consumer_copyright_off_radio;

GtkWidget *hw_consumer_copy_1st_radio;
GtkWidget *hw_consumer_copy_original_radio;

GtkWidget *hw_consumer_emphasis_none_radio;
GtkWidget *hw_consumer_emphasis_5015_radio;

GtkWidget *hw_consumer_category_dat_radio;
GtkWidget *hw_consumer_category_pcm_radio;
GtkWidget *hw_consumer_category_cd_radio;
GtkWidget *hw_consumer_category_general_radio;

GtkWidget *hw_spdif_professional_radio;
GtkWidget *hw_spdif_consumer_radio;
GtkWidget *hw_spdif_output_notebook;

GtkWidget *hw_spdif_input_coaxial_radio;
GtkWidget *hw_spdif_input_optical_radio;

GtkObject *av_dac_volume_adj[10];
GtkObject *av_adc_volume_adj[10];
GtkLabel *av_dac_volume_label[10];
GtkLabel *av_adc_volume_label[10];
GtkWidget *av_dac_sense_radio[10][4];
GtkWidget *av_adc_sense_radio[10][4];


static void create_mixer_frame(GtkWidget *fixed, int stream)
{
	GtkWidget *frame;
	GtkWidget *fixed1;
	GtkObject *adj;
	GtkWidget *vscale;
	GtkWidget *drawing;
	GtkWidget *label;
	GtkWidget *toggle;
	GtkWidget *hseparator;
	char str[64], drawname[32];

	if (stream <= 10) {
		sprintf(str, "PCM Out %i", stream);
	} else if (stream <= 18) {
		sprintf(str, "H/W In %i", stream - 10);
	} else if (stream == 19) {
		strcpy(str, "S/PDIF In L");
	} else if (stream == 20) {
		strcpy(str, "S/PDIF In R");
	} else {
		strcpy(str, "???");
	}

	frame = gtk_frame_new(str);
	gtk_widget_show(frame);
	gtk_fixed_put(GTK_FIXED(fixed), frame, 2 + (stream - 1) * 102, 2);
	gtk_widget_set_uposition(frame, 2 + (stream - 1) * 102, 2);
	gtk_widget_set_usize(frame, 98, 288);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);

	fixed1 = gtk_fixed_new();
	gtk_widget_show(fixed1);
	gtk_container_add(GTK_CONTAINER(frame), fixed1);	

	adj = gtk_adjustment_new(96, 0, 96, 1, 16, 0);
	mixer_adj[stream-1][0] = adj;
	vscale = gtk_vscale_new(GTK_ADJUSTMENT(adj));
	mixer_vscale[stream-1][0] = vscale;
        gtk_widget_show(vscale);
	gtk_fixed_put(GTK_FIXED(fixed1), vscale, 8, 8);
	gtk_widget_set_uposition(vscale, 7, 8);
	gtk_widget_set_usize(vscale, 18, 168);	
	gtk_scale_set_value_pos(GTK_SCALE(vscale), GTK_POS_BOTTOM);
	gtk_scale_set_digits(GTK_SCALE(vscale), 0);
	gtk_signal_connect(GTK_OBJECT(adj), "value_changed",
			   GTK_SIGNAL_FUNC(mixer_adjust), (gpointer)((stream << 16) + 0));

	drawing = gtk_drawing_area_new();
	mixer_drawing[stream-1] = drawing;
	sprintf(drawname, "Mixer%i", stream);
	gtk_widget_set_name(drawing, drawname);
	gtk_widget_show(drawing);
	gtk_signal_connect(GTK_OBJECT(drawing), "expose_event",
			   (GtkSignalFunc)level_meters_expose_event, NULL);
	gtk_signal_connect(GTK_OBJECT(drawing), "configure_event",
			   (GtkSignalFunc)level_meters_configure_event, NULL);
	gtk_widget_set_events(drawing, GDK_EXPOSURE_MASK);
	gtk_fixed_put(GTK_FIXED(fixed1), drawing, 24, 9);
	gtk_widget_set_uposition(drawing, 24, 9);
	gtk_widget_set_usize(drawing, 45, 152);	

	adj = gtk_adjustment_new(96, 0, 96, 1, 16, 0);
	mixer_adj[stream-1][1] = adj;
	vscale = gtk_vscale_new(GTK_ADJUSTMENT(adj));
	mixer_vscale[stream-1][1] = vscale;
        gtk_widget_show(vscale);
	gtk_fixed_put(GTK_FIXED(fixed1), vscale, 70, 8);
	gtk_widget_set_uposition(vscale, 69, 8);
	gtk_widget_set_usize(vscale, 18, 168);	
	gtk_scale_set_value_pos(GTK_SCALE(vscale), GTK_POS_BOTTOM);
	gtk_scale_set_digits(GTK_SCALE(vscale), 0);
	gtk_signal_connect(GTK_OBJECT(adj), "value_changed",
			   GTK_SIGNAL_FUNC(mixer_adjust), (gpointer)((stream << 16) + 1));

        label = gtk_label_new("Left");
        gtk_widget_show(label);
	gtk_fixed_put(GTK_FIXED(fixed1), label, 0, 182);
	gtk_widget_set_uposition(label, 0, 182);
	gtk_widget_set_usize(label, 41, 16);	

        label = gtk_label_new("Right");
        gtk_widget_show(label);
	gtk_fixed_put(GTK_FIXED(fixed1), label, 45, 182);
	gtk_widget_set_uposition(label, 45, 182);
	gtk_widget_set_usize(label, 41, 16);

	toggle = gtk_toggle_button_new_with_label("Mute");
	mixer_mute_toggle[stream-1][0] = toggle;
	gtk_widget_show(toggle);
	gtk_fixed_put(GTK_FIXED(fixed1), toggle, 8, 202);
	gtk_widget_set_uposition(toggle, 8, 202);
	gtk_widget_set_usize(toggle, 36, 22);	
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle), TRUE);
	gtk_signal_connect(GTK_OBJECT(toggle), "toggled",
			   (GtkSignalFunc)mixer_toggled_mute, (gpointer)((stream << 16) + 0));
	
	toggle = gtk_toggle_button_new_with_label("Mute");
	mixer_mute_toggle[stream-1][1] = toggle;
	gtk_widget_show(toggle);
	gtk_fixed_put(GTK_FIXED(fixed1), toggle, 48, 202);
	gtk_widget_set_uposition(toggle, 48, 202);
	gtk_widget_set_usize(toggle, 36, 22);	
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle), TRUE);
	gtk_signal_connect(GTK_OBJECT(toggle), "toggled",
			   (GtkSignalFunc)mixer_toggled_mute, (gpointer)((stream << 16) + 1));
	
	hseparator = gtk_hseparator_new();
	gtk_widget_show(hseparator);
	gtk_fixed_put(GTK_FIXED(fixed1), hseparator, 0, 222);
	gtk_widget_set_uposition(hseparator, 0, 222);
	gtk_widget_set_usize(hseparator, 92, 16);

	toggle = gtk_toggle_button_new_with_label("Stereo Gang");
	mixer_stereo_toggle[stream-1] = toggle;
	gtk_widget_show(toggle);
	gtk_fixed_put(GTK_FIXED(fixed1), toggle, 3, 235);
	gtk_widget_set_uposition(toggle, 3, 235);
	gtk_widget_set_usize(toggle, 88, 32);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle), TRUE);
}

static void create_mixer(GtkWidget *main, GtkWidget *notebook, int page)
{
	GtkWidget *label;
	GtkWidget *hpaned;
	GtkWidget *frame;
	GtkWidget *fixed;
	GtkWidget *drawing;
	GtkWidget *hseparator;
	GtkWidget *button;
	GtkWidget *scrollwin;
	GtkWidget *viewport;
	int stream;

	hpaned = gtk_hpaned_new();
	gtk_widget_show(hpaned);
	gtk_container_add(GTK_CONTAINER(notebook), hpaned);
	gtk_paned_set_gutter_size(GTK_PANED(hpaned), 4);
	gtk_paned_set_position(GTK_PANED(hpaned), 108);
        label = gtk_label_new("Monitor Mixer");
        gtk_widget_show(label);
	gtk_notebook_set_tab_label(GTK_NOTEBOOK(notebook), gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), page), label);

	/* create digital mixer frame */
	frame = gtk_frame_new("Digital Mixer");
	gtk_widget_show(frame);
	gtk_container_add(GTK_CONTAINER(hpaned), frame);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);

	/* create controls in the digital mixer frame */
	fixed = gtk_fixed_new();
	gtk_widget_show(fixed);
	gtk_container_add(GTK_CONTAINER(frame), fixed);	

	drawing = gtk_drawing_area_new();
	mixer_mix_drawing = drawing;
	gtk_widget_set_name(drawing, "DigitalMixer");
	gtk_signal_connect(GTK_OBJECT(drawing), "expose_event",
			   (GtkSignalFunc)level_meters_expose_event, NULL);
	gtk_signal_connect(GTK_OBJECT(drawing), "configure_event",
			   (GtkSignalFunc)level_meters_configure_event, NULL);
	gtk_widget_set_events(drawing, GDK_EXPOSURE_MASK);
	gtk_widget_show(drawing);
	gtk_fixed_put(GTK_FIXED(fixed), drawing, 4, 2);
	gtk_widget_set_uposition(drawing, 4, 2);
	gtk_widget_set_usize(drawing, 98, 226);	

        label = gtk_label_new("Left");
        gtk_widget_show(label);
	gtk_fixed_put(GTK_FIXED(fixed), label, 0, 232);
	gtk_widget_set_uposition(label, 0, 232);
	gtk_widget_set_usize(label, 34, 16);	

        label = gtk_label_new("Right");
        gtk_widget_show(label);
	gtk_fixed_put(GTK_FIXED(fixed), label, 60, 232);
	gtk_widget_set_uposition(label, 60, 232);
	gtk_widget_set_usize(label, 34, 16);

	hseparator = gtk_hseparator_new();
	gtk_widget_show(hseparator);
	gtk_fixed_put(GTK_FIXED(fixed), hseparator, 0, 244);
	gtk_widget_set_uposition(hseparator, 0, 244);
	gtk_widget_set_usize(hseparator, 104, 16);

	button = gtk_button_new_with_label("Reset Peaks");
	mixer_clear_peaks_button = button;
	gtk_widget_show(button);
	gtk_fixed_put(GTK_FIXED(fixed), button, 3, 256);
	gtk_widget_set_uposition(button, 3, 256);
	gtk_widget_set_usize(button, 98, 35);	
	gtk_signal_connect(GTK_OBJECT(button), "clicked",
			   GTK_SIGNAL_FUNC(level_meters_reset_peaks), NULL);

	/* build scrolling area */
	scrollwin = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_show(scrollwin);
	gtk_container_add(GTK_CONTAINER(hpaned), scrollwin);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin), GTK_POLICY_ALWAYS, GTK_POLICY_NEVER);

	viewport = gtk_viewport_new(NULL, NULL);
	gtk_widget_show(viewport);
	gtk_container_add(GTK_CONTAINER(scrollwin), viewport);

	fixed = gtk_fixed_new();
	gtk_widget_show(fixed);
	gtk_container_add(GTK_CONTAINER(viewport), fixed);	
	
	for (stream = 1; stream <= 20; stream++)
		create_mixer_frame(fixed, stream);
}

static void create_router_frame(GtkWidget *fixed, int stream, int pos)
{
	GtkWidget *frame;
	GtkWidget *fixed1;
	GtkWidget *radio;
	GtkWidget *hseparator;
	GSList *group = NULL;
	char str[64], str1[64];
	int idx;
	static char *table[10] = {
		"S/PDIF In L",
		"S/PDIF In R",
		"H/W In 1",
		"H/W In 2",
		"H/W In 3",
		"H/W In 4",
		"H/W In 5",
		"H/W In 6",
		"H/W In 7",
		"H/W In 8"
	};

	if (stream <= 8) {
		sprintf(str, "H/W Out %i (%s)", stream, stream & 1 ? "L" : "R");
	} else if (stream == 9) {
		strcpy(str, "S/PDIF Out L");
	} else if (stream == 10) {
		strcpy(str, "S/PDIF Out R");
	} else {
		strcpy(str, "???");
	}
	sprintf(str1, "PCM Out %i", stream);

	frame = gtk_frame_new(str);
	gtk_widget_show(frame);
	gtk_fixed_put(GTK_FIXED(fixed), frame, 2 + pos * 101, 2);
	gtk_widget_set_uposition(frame, 2 + pos * 101, 2);
	gtk_widget_set_usize(frame, 98, 284);

	fixed1 = gtk_fixed_new();
	gtk_widget_show(fixed1);
	gtk_container_add(GTK_CONTAINER(frame), fixed1);	

	radio = gtk_radio_button_new_with_label(group, str1);
	router_radio[stream-1][0] = radio;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radio));
	gtk_widget_show(radio);
	gtk_fixed_put(GTK_FIXED(fixed1), radio, 0, 0);
	gtk_widget_set_uposition(radio, 0, 0);
	gtk_widget_set_usize(radio, 93, 24);
	gtk_signal_connect(GTK_OBJECT(radio), "toggled",
			   (GtkSignalFunc)patchbay_toggled, (gpointer)((stream << 16) + 0));

	hseparator = gtk_hseparator_new();
	gtk_widget_show(hseparator);
	gtk_fixed_put(GTK_FIXED(fixed1), hseparator, 0, 21);
	gtk_widget_set_uposition(hseparator, 0, 21);
	gtk_widget_set_usize(hseparator, 94, 16);

	if (stream == 1 || stream == 2 || stream == 9 || stream == 10) {
		radio = gtk_radio_button_new_with_label(group, stream & 1 ? "Digital Mix L" : "Digital Mix R");
		router_radio[stream-1][1] = radio;
		group = gtk_radio_button_group(GTK_RADIO_BUTTON(radio));
		gtk_widget_show(radio);
		gtk_fixed_put(GTK_FIXED(fixed1), radio, 0, 32);
		gtk_widget_set_uposition(radio, 0, 32);
		gtk_widget_set_usize(radio, 93, 24);
		gtk_signal_connect(GTK_OBJECT(radio), "toggled",
				   (GtkSignalFunc)patchbay_toggled, (gpointer)((stream << 16) + 1));
	}

	hseparator = gtk_hseparator_new();
	gtk_widget_show(hseparator);
	gtk_fixed_put(GTK_FIXED(fixed1), hseparator, 0, 52);
	gtk_widget_set_uposition(hseparator, 0, 52);
	gtk_widget_set_usize(hseparator, 94, 16);

	for (idx = 0; idx < 10; idx++) {
		radio = gtk_radio_button_new_with_label(group, table[idx]);
		router_radio[stream-1][2+idx] = radio;
		group = gtk_radio_button_group(GTK_RADIO_BUTTON(radio));
		gtk_widget_show(radio);
		gtk_fixed_put(GTK_FIXED(fixed1), radio, 0, 64 + (idx * 20));
		gtk_widget_set_uposition(radio, 0, 64 + (idx * 20));
		gtk_widget_set_usize(radio, 93, 24);
		gtk_signal_connect(GTK_OBJECT(radio), "toggled",
				   (GtkSignalFunc)patchbay_toggled, (gpointer)((stream << 16) + 2 + idx));
	}
}

static void create_router(GtkWidget *main, GtkWidget *notebook, int page)
{
	GtkWidget *label;
	GtkWidget *scrollwin;
	GtkWidget *viewport;
	GtkWidget *fixed;
	int stream, pos;

	scrollwin = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_show(scrollwin);
	gtk_container_add(GTK_CONTAINER(notebook), scrollwin);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin), GTK_POLICY_ALWAYS, GTK_POLICY_NEVER);
        label = gtk_label_new("Patchbay / Router");
        gtk_widget_show(label);
	gtk_notebook_set_tab_label(GTK_NOTEBOOK(notebook), gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), page), label);
	viewport = gtk_viewport_new(NULL, NULL);
	gtk_widget_show(viewport);
	gtk_container_add(GTK_CONTAINER(scrollwin), viewport);
	fixed = gtk_fixed_new();
	gtk_widget_show(fixed);
	gtk_container_add(GTK_CONTAINER(viewport), fixed);	

	pos = 0;
	for (stream = 1; stream <= 10; stream++) {
		if (patchbay_stream_is_active(stream))
			create_router_frame(fixed, stream, pos++);
	}
}

static void create_master_clock(GtkWidget *fixed)
{
	GtkWidget *frame;
	GtkWidget *fixed1;
	GtkWidget *radio;
	GtkWidget *viewport;
	GtkWidget *fixed2;
	GtkWidget *label;
	GSList *group = NULL;

	frame = gtk_frame_new("Master Clock");
	gtk_widget_show(frame);
	gtk_fixed_put(GTK_FIXED(fixed), frame, 8, 8);
	gtk_widget_set_uposition(frame, 8, 8);
	gtk_widget_set_usize(frame, 111, 125);

	fixed1 = gtk_fixed_new();
	gtk_widget_show(fixed1);
	gtk_container_add(GTK_CONTAINER(frame), fixed1);	

	radio = gtk_radio_button_new_with_label(group, "Internal Xtal");
	hw_master_clock_xtal_radio = radio;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radio));
	gtk_widget_show(radio);
	gtk_fixed_put(GTK_FIXED(fixed1), radio, 8, 0);
	gtk_widget_set_uposition(radio, 8, 0);
	gtk_widget_set_usize(radio, 92, 24);
	gtk_signal_connect(GTK_OBJECT(radio), "toggled",
			   (GtkSignalFunc)master_clock_toggled, (gpointer)"Xtal");

	radio = gtk_radio_button_new_with_label(group, "S/PDIF In");
	hw_master_clock_spdif_radio = radio;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radio));
	gtk_widget_show(radio);
	gtk_fixed_put(GTK_FIXED(fixed1), radio, 8, 24);
	gtk_widget_set_uposition(radio, 8, 24);
	gtk_widget_set_usize(radio, 92, 24);
	gtk_signal_connect(GTK_OBJECT(radio), "toggled",
			   (GtkSignalFunc)master_clock_toggled, (gpointer)"SPDIF");

	if (card_eeprom.subvendor != ICE1712_SUBDEVICE_DELTA1010)
		return;

	radio = gtk_radio_button_new_with_label(group, "Word Clock");
	hw_master_clock_word_radio = radio;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radio));
	gtk_widget_show(radio);
	gtk_fixed_put(GTK_FIXED(fixed1), radio, 8, 48);
	gtk_widget_set_uposition(radio, 8, 48);
	gtk_widget_set_usize(radio, 92, 24);
	gtk_signal_connect(GTK_OBJECT(radio), "toggled",
			   (GtkSignalFunc)master_clock_toggled, (gpointer)"WordClock");

	viewport = gtk_viewport_new(NULL, NULL);
	gtk_widget_show(viewport);
	gtk_fixed_put(GTK_FIXED(fixed1), viewport, 9, 75);
	gtk_widget_set_uposition(viewport, 9, 75);
	gtk_widget_set_usize(viewport, 90, 26);

	fixed2 = gtk_fixed_new();
	gtk_widget_show(fixed2);
	gtk_container_add(GTK_CONTAINER(viewport), fixed2);	

        label = gtk_label_new("Locked");
        hw_master_clock_status_label = label;
        gtk_widget_show(label);
	gtk_fixed_put(GTK_FIXED(fixed2), label, 0, 3);
	gtk_widget_set_uposition(label, 0, 3);
	gtk_widget_set_usize(label, 86, 16);
}

static void create_volume_change(GtkWidget *fixed)
{
	GtkWidget *frame;
	GtkWidget *fixed1;
	GtkObject *adj;
	GtkWidget *spin;
	GtkWidget *label;

	frame = gtk_frame_new("Volume Change");
	gtk_widget_show(frame);
	gtk_fixed_put(GTK_FIXED(fixed), frame, 8, 144);
	gtk_widget_set_uposition(frame, 8, 144);
	gtk_widget_set_usize(frame, 111, 132);

	fixed1 = gtk_fixed_new();
	gtk_widget_show(fixed1);
	gtk_container_add(GTK_CONTAINER(frame), fixed1);	

	adj = gtk_adjustment_new(16, 0, 255, 1, 10, 10);
	hw_volume_change_adj = adj;
	spin = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 1, 0);
	hw_volume_change_spin = spin;
	gtk_widget_show(spin);
	gtk_fixed_put(GTK_FIXED(fixed1), spin, 48, 40);
	gtk_widget_set_uposition(spin, 48, 40);
	gtk_widget_set_usize(spin, 53, 22);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(spin), TRUE);
	gtk_signal_connect(GTK_OBJECT(adj), "value_changed",
			   GTK_SIGNAL_FUNC(volume_change_rate_adj), NULL);
	
        label = gtk_label_new("Rate:");
        gtk_widget_show(label);
	gtk_fixed_put(GTK_FIXED(fixed1), label, 0, 3);
	gtk_widget_set_uposition(label, 0, 42);
	gtk_widget_set_usize(label, 41, 16);
}

static void create_spdif_output_settings_profi_data(GtkWidget *fixed)
{
	GtkWidget *frame;
	GtkWidget *fixed1;
	GtkWidget *radio;
	GSList *group = NULL;

	frame = gtk_frame_new("Data Mode");
	gtk_widget_show(frame);
	gtk_fixed_put(GTK_FIXED(fixed), frame, 10, 10);
	gtk_widget_set_uposition(frame, 10, 10);
	gtk_widget_set_usize(frame, 100, 81);

	fixed1 = gtk_fixed_new();
	gtk_widget_show(fixed1);
	gtk_container_add(GTK_CONTAINER(frame), fixed1);	

	radio = gtk_radio_button_new_with_label(group, "Non-audio");
	hw_spdif_profi_nonaudio_radio = radio;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radio));
	gtk_widget_show(radio);
	gtk_fixed_put(GTK_FIXED(fixed1), radio, 8, 8);
	gtk_widget_set_uposition(radio, 8, 8);
	gtk_widget_set_usize(radio, 84, 24);
	gtk_signal_connect(GTK_OBJECT(radio), "toggled",
			   (GtkSignalFunc)profi_data_toggled, (gpointer)"Non-audio");

	radio = gtk_radio_button_new_with_label(group, "Audio");
	hw_spdif_profi_audio_radio = radio;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radio));
	gtk_widget_show(radio);
	gtk_fixed_put(GTK_FIXED(fixed1), radio, 8, 32);
	gtk_widget_set_uposition(radio, 8, 32);
	gtk_widget_set_usize(radio, 84, 24);
	gtk_signal_connect(GTK_OBJECT(radio), "toggled",
			   (GtkSignalFunc)profi_data_toggled, (gpointer)"Audio");
}

static void create_spdif_output_settings_profi_stream(GtkWidget *fixed)
{
	GtkWidget *frame;
	GtkWidget *fixed1;
	GtkWidget *radio;
	GSList *group = NULL;

	frame = gtk_frame_new("Stream");
	gtk_widget_show(frame);
	gtk_fixed_put(GTK_FIXED(fixed), frame, 120, 10);
	gtk_widget_set_uposition(frame, 120, 10);
	gtk_widget_set_usize(frame, 116, 81);

	fixed1 = gtk_fixed_new();
	gtk_widget_show(fixed1);
	gtk_container_add(GTK_CONTAINER(frame), fixed1);	

	radio = gtk_radio_button_new_with_label(group, "Stereophonic");
	hw_profi_stream_stereo_radio = radio;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radio));
	gtk_widget_show(radio);
	gtk_fixed_put(GTK_FIXED(fixed1), radio, 8, 8);
	gtk_widget_set_uposition(radio, 8, 8);
	gtk_widget_set_usize(radio, 96, 24);
	gtk_signal_connect(GTK_OBJECT(radio), "toggled",
			   (GtkSignalFunc)profi_stream_toggled, (gpointer)"Stereo");

	radio = gtk_radio_button_new_with_label(group, "Not indicated");
	hw_profi_stream_notid_radio = radio;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radio));
	gtk_widget_show(radio);
	gtk_fixed_put(GTK_FIXED(fixed1), radio, 8, 32);
	gtk_widget_set_uposition(radio, 8, 32);
	gtk_widget_set_usize(radio, 96, 24);
	gtk_signal_connect(GTK_OBJECT(radio), "toggled",
			   (GtkSignalFunc)profi_stream_toggled, (gpointer)"NOTID");
}

static void create_spdif_output_settings_profi_emphasis(GtkWidget *fixed)
{
	GtkWidget *frame;
	GtkWidget *fixed1;
	GtkWidget *radio;
	GSList *group = NULL;

	frame = gtk_frame_new("Emphasis");
	gtk_widget_show(frame);
	gtk_fixed_put(GTK_FIXED(fixed), frame, 10, 10);
	gtk_widget_set_uposition(frame, 246, 10);
	gtk_widget_set_usize(frame, 114, 131);

	fixed1 = gtk_fixed_new();
	gtk_widget_show(fixed1);
	gtk_container_add(GTK_CONTAINER(frame), fixed1);	

	radio = gtk_radio_button_new_with_label(group, "No emphasis");
	hw_profi_emphasis_none_radio = radio;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radio));
	gtk_widget_show(radio);
	gtk_fixed_put(GTK_FIXED(fixed1), radio, 8, 8);
	gtk_widget_set_uposition(radio, 8, 8);
	gtk_widget_set_usize(radio, 94, 24);
	gtk_signal_connect(GTK_OBJECT(radio), "toggled",
			   (GtkSignalFunc)profi_emphasis_toggled, (gpointer)"No");

	radio = gtk_radio_button_new_with_label(group, "50/15us");
	hw_profi_emphasis_5015_radio = radio;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radio));
	gtk_widget_show(radio);
	gtk_fixed_put(GTK_FIXED(fixed1), radio, 8, 32);
	gtk_widget_set_uposition(radio, 8, 32);
	gtk_widget_set_usize(radio, 94, 24);
	gtk_signal_connect(GTK_OBJECT(radio), "toggled",
			   (GtkSignalFunc)profi_emphasis_toggled, (gpointer)"5015");

	radio = gtk_radio_button_new_with_label(group, "CCITT J.17");
	hw_profi_emphasis_ccitt_radio = radio;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radio));
	gtk_widget_show(radio);
	gtk_fixed_put(GTK_FIXED(fixed1), radio, 8, 56);
	gtk_widget_set_uposition(radio, 8, 56);
	gtk_widget_set_usize(radio, 94, 24);
	gtk_signal_connect(GTK_OBJECT(radio), "toggled",
			   (GtkSignalFunc)profi_emphasis_toggled, (gpointer)"CCITT");

	radio = gtk_radio_button_new_with_label(group, "Not indicated");
	hw_profi_emphasis_notid_radio = radio;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radio));
	gtk_widget_show(radio);
	gtk_fixed_put(GTK_FIXED(fixed1), radio, 8, 80);
	gtk_widget_set_uposition(radio, 8, 80);
	gtk_widget_set_usize(radio, 94, 24);
	gtk_signal_connect(GTK_OBJECT(radio), "toggled",
			   (GtkSignalFunc)profi_emphasis_toggled, (gpointer)"NOTID");
}

static void create_spdif_output_settings_profi(GtkWidget *notebook, int page)
{
	GtkWidget *fixed;
	GtkWidget *label;

	fixed = gtk_fixed_new();
	gtk_widget_show(fixed);
	gtk_container_add(GTK_CONTAINER(notebook), fixed);
        label = gtk_label_new("Professional");
        gtk_widget_show(label);
	gtk_notebook_set_tab_label(GTK_NOTEBOOK(notebook), gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), page), label);

	create_spdif_output_settings_profi_data(fixed);
	create_spdif_output_settings_profi_stream(fixed);
	create_spdif_output_settings_profi_emphasis(fixed);
}

static void create_spdif_output_settings_consumer_copyright(GtkWidget *fixed)
{
	GtkWidget *frame;
	GtkWidget *fixed1;
	GtkWidget *radio;
	GSList *group = NULL;

	frame = gtk_frame_new("Copyright");
	gtk_widget_show(frame);
	gtk_fixed_put(GTK_FIXED(fixed), frame, 10, 10);
	gtk_widget_set_uposition(frame, 10, 10);
	gtk_widget_set_usize(frame, 124, 79);

	fixed1 = gtk_fixed_new();
	gtk_widget_show(fixed1);
	gtk_container_add(GTK_CONTAINER(frame), fixed1);

	radio = gtk_radio_button_new_with_label(group, "Copyrighted");
	hw_consumer_copyright_on_radio = radio;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radio));
	gtk_widget_show(radio);
	gtk_fixed_put(GTK_FIXED(fixed1), radio, 8, 8);
	gtk_widget_set_uposition(radio, 8, 8);
	gtk_widget_set_usize(radio, 106, 24);
	gtk_signal_connect(GTK_OBJECT(radio), "toggled",
			   (GtkSignalFunc)consumer_copyright_toggled, (gpointer)"Copyright");

	radio = gtk_radio_button_new_with_label(group, "Copy permitted");
	hw_consumer_copyright_off_radio = radio;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radio));
	gtk_widget_show(radio);
	gtk_fixed_put(GTK_FIXED(fixed1), radio, 8, 32);
	gtk_widget_set_uposition(radio, 8, 32);
	gtk_widget_set_usize(radio, 106, 24);
	gtk_signal_connect(GTK_OBJECT(radio), "toggled",
			   (GtkSignalFunc)consumer_copyright_toggled, (gpointer)"Permitted");
}

static void create_spdif_output_settings_consumer_copy(GtkWidget *fixed)
{
	GtkWidget *frame;
	GtkWidget *fixed1;
	GtkWidget *radio;
	GSList *group = NULL;

	frame = gtk_frame_new("Copy");
	gtk_widget_show(frame);
	gtk_fixed_put(GTK_FIXED(fixed), frame, 10, 96);
	gtk_widget_set_uposition(frame, 10, 96);
	gtk_widget_set_usize(frame, 124, 79);

	fixed1 = gtk_fixed_new();
	gtk_widget_show(fixed1);
	gtk_container_add(GTK_CONTAINER(frame), fixed1);

	radio = gtk_radio_button_new_with_label(group, "1-st generation");
	hw_consumer_copy_1st_radio = radio;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radio));
	gtk_widget_show(radio);
	gtk_fixed_put(GTK_FIXED(fixed1), radio, 8, 8);
	gtk_widget_set_uposition(radio, 8, 8);
	gtk_widget_set_usize(radio, 106, 24);
	gtk_signal_connect(GTK_OBJECT(radio), "toggled",
			   (GtkSignalFunc)consumer_copy_toggled, (gpointer)"1st");

	radio = gtk_radio_button_new_with_label(group, "Original");
	hw_consumer_copy_original_radio = radio;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radio));
	gtk_widget_show(radio);
	gtk_fixed_put(GTK_FIXED(fixed1), radio, 8, 32);
	gtk_widget_set_uposition(radio, 8, 32);
	gtk_widget_set_usize(radio, 106, 24);
	gtk_signal_connect(GTK_OBJECT(radio), "toggled",
			   (GtkSignalFunc)consumer_copy_toggled, (gpointer)"Original");
}

static void create_spdif_output_settings_consumer_emphasis(GtkWidget *fixed)
{
	GtkWidget *frame;
	GtkWidget *fixed1;
	GtkWidget *radio;
	GSList *group = NULL;

	frame = gtk_frame_new("Emphasis");
	gtk_widget_show(frame);
	gtk_fixed_put(GTK_FIXED(fixed), frame, 144, 10);
	gtk_widget_set_uposition(frame, 144, 10);
	gtk_widget_set_usize(frame, 130, 80);

	fixed1 = gtk_fixed_new();
	gtk_widget_show(fixed1);
	gtk_container_add(GTK_CONTAINER(frame), fixed1);

	radio = gtk_radio_button_new_with_label(group, "No emphasis");
	hw_consumer_emphasis_none_radio = radio;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radio));
	gtk_widget_show(radio);
	gtk_fixed_put(GTK_FIXED(fixed1), radio, 8, 8);
	gtk_widget_set_uposition(radio, 8, 8);
	gtk_widget_set_usize(radio, 92, 24);
	gtk_signal_connect(GTK_OBJECT(radio), "toggled",
			   (GtkSignalFunc)consumer_emphasis_toggled, (gpointer)"No");

	radio = gtk_radio_button_new_with_label(group, "50/15us");
	hw_consumer_emphasis_5015_radio = radio;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radio));
	gtk_widget_show(radio);
	gtk_fixed_put(GTK_FIXED(fixed1), radio, 8, 32);
	gtk_widget_set_uposition(radio, 8, 32);
	gtk_widget_set_usize(radio, 92, 24);
	gtk_signal_connect(GTK_OBJECT(radio), "toggled",
			   (GtkSignalFunc)consumer_emphasis_toggled, (gpointer)"5015");
}

static void create_spdif_output_settings_consumer_category(GtkWidget *fixed)
{
	GtkWidget *frame;
	GtkWidget *fixed1;
	GtkWidget *radio;
	GSList *group = NULL;

	frame = gtk_frame_new("Category");
	gtk_widget_show(frame);
	gtk_fixed_put(GTK_FIXED(fixed), frame, 283, 10);
	gtk_widget_set_uposition(frame, 283, 10);
	gtk_widget_set_usize(frame, 130, 126);

	fixed1 = gtk_fixed_new();
	gtk_widget_show(fixed1);
	gtk_container_add(GTK_CONTAINER(frame), fixed1);	

	radio = gtk_radio_button_new_with_label(group, "DAT");
	hw_consumer_category_dat_radio = radio;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radio));
	gtk_widget_show(radio);
	gtk_fixed_put(GTK_FIXED(fixed1), radio, 8, 8);
	gtk_widget_set_uposition(radio, 8, 8);
	gtk_widget_set_usize(radio, 102, 24);
	gtk_signal_connect(GTK_OBJECT(radio), "toggled",
			   (GtkSignalFunc)consumer_category_toggled, (gpointer)"DAT");

	radio = gtk_radio_button_new_with_label(group, "PCM encoder");
	hw_consumer_category_pcm_radio = radio;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radio));
	gtk_widget_show(radio);
	gtk_fixed_put(GTK_FIXED(fixed1), radio, 8, 32);
	gtk_widget_set_uposition(radio, 8, 32);
	gtk_widget_set_usize(radio, 102, 24);
	gtk_signal_connect(GTK_OBJECT(radio), "toggled",
			   (GtkSignalFunc)consumer_category_toggled, (gpointer)"PCM");

	radio = gtk_radio_button_new_with_label(group, "CD (ICE-908)");
	hw_consumer_category_cd_radio = radio;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radio));
	gtk_widget_show(radio);
	gtk_fixed_put(GTK_FIXED(fixed1), radio, 8, 56);
	gtk_widget_set_uposition(radio, 8, 56);
	gtk_widget_set_usize(radio, 102, 24);
	gtk_signal_connect(GTK_OBJECT(radio), "toggled",
			   (GtkSignalFunc)consumer_category_toggled, (gpointer)"CD");

	radio = gtk_radio_button_new_with_label(group, "General");
	hw_consumer_category_general_radio = radio;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radio));
	gtk_widget_show(radio);
	gtk_fixed_put(GTK_FIXED(fixed1), radio, 8, 80);
	gtk_widget_set_uposition(radio, 8, 80);
	gtk_widget_set_usize(radio, 102, 24);
	gtk_signal_connect(GTK_OBJECT(radio), "toggled",
			   (GtkSignalFunc)consumer_category_toggled, (gpointer)"General");
}

static void create_spdif_output_settings_consumer(GtkWidget *notebook, int page)
{
	GtkWidget *fixed;
	GtkWidget *label;

	fixed = gtk_fixed_new();
	gtk_widget_show(fixed);
	gtk_container_add(GTK_CONTAINER(notebook), fixed);
	label = gtk_label_new("Consumer");
        gtk_widget_show(label);
	gtk_notebook_set_tab_label(GTK_NOTEBOOK(notebook), gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), page), label);

	create_spdif_output_settings_consumer_copyright(fixed);
	create_spdif_output_settings_consumer_copy(fixed);
	create_spdif_output_settings_consumer_emphasis(fixed);
	create_spdif_output_settings_consumer_category(fixed);
}

static void create_spdif_output_settings(GtkWidget *fixed)
{
	GtkWidget *frame;
	GtkWidget *fixed1;
	GtkWidget *radio;
	GtkWidget *notebook;
	GSList *group = NULL;

	frame = gtk_frame_new("S/PDIF Output Settings");
	gtk_widget_show(frame);
	gtk_fixed_put(GTK_FIXED(fixed), frame, 128, 8);
	gtk_widget_set_uposition(frame, 128, 8);
	gtk_widget_set_usize(frame, 442, 268);

	fixed1 = gtk_fixed_new();
	gtk_widget_show(fixed1);
	gtk_container_add(GTK_CONTAINER(frame), fixed1);	

	radio = gtk_radio_button_new_with_label(group, "Professional");
	hw_spdif_professional_radio = radio;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radio));
	gtk_widget_show(radio);
	gtk_fixed_put(GTK_FIXED(fixed1), radio, 24, 4);
	gtk_widget_set_uposition(radio, 24, 4);
	gtk_widget_set_usize(radio, 92, 24);
	gtk_signal_connect(GTK_OBJECT(radio), "toggled",
			   (GtkSignalFunc)spdif_output_toggled, (gpointer)"Professional");

	radio = gtk_radio_button_new_with_label(group, "Consumer");
	hw_spdif_consumer_radio = radio;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radio));
	gtk_widget_show(radio);
	gtk_fixed_put(GTK_FIXED(fixed1), radio, 128, 4);
	gtk_widget_set_uposition(radio, 128, 4);
	gtk_widget_set_usize(radio, 92, 24);
	gtk_signal_connect(GTK_OBJECT(radio), "toggled",
			   (GtkSignalFunc)spdif_output_toggled, (gpointer)"Consumer");

	notebook = gtk_notebook_new();
	hw_spdif_output_notebook = notebook;
	gtk_widget_show(notebook);
	gtk_fixed_put(GTK_FIXED(fixed1), notebook, 5, 31);
	gtk_widget_set_uposition(notebook, 5, 31);
	gtk_widget_set_usize(notebook, 427, 215);

	create_spdif_output_settings_profi(notebook, 0);
	create_spdif_output_settings_consumer(notebook, 1);
}

static void create_spdif_input_select(GtkWidget *fixed)
{
	GtkWidget *frame;
	GtkWidget *fixed1;
	GtkWidget *radio;
	GSList *group = NULL;
	int hide = 1;

	if (card_eeprom.subvendor == ICE1712_SUBDEVICE_DELTADIO2496)
		hide = 0;

	frame = gtk_frame_new("S/PDIF Input");
	gtk_widget_show(frame);
	gtk_fixed_put(GTK_FIXED(fixed), frame, 579, 8);
	gtk_widget_set_uposition(frame, 579, 8);
	gtk_widget_set_usize(frame, 98, 79);

	fixed1 = gtk_fixed_new();
	gtk_widget_show(fixed1);
	gtk_container_add(GTK_CONTAINER(frame), fixed1);	

	radio = gtk_radio_button_new_with_label(group, "Coaxial");
	hw_spdif_input_coaxial_radio = radio;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radio));
	gtk_widget_show(radio);
	gtk_fixed_put(GTK_FIXED(fixed1), radio, 8, 8);
	gtk_widget_set_uposition(radio, 8, 8);
	gtk_widget_set_usize(radio, 82, 24);
	gtk_signal_connect(GTK_OBJECT(radio), "toggled",
			   (GtkSignalFunc)spdif_input_toggled, (gpointer)"Coaxial");

	radio = gtk_radio_button_new_with_label(group, "Optical");
	hw_spdif_input_optical_radio = radio;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radio));
	gtk_widget_show(radio);
	gtk_fixed_put(GTK_FIXED(fixed1), radio, 8, 32);
	gtk_widget_set_uposition(radio, 8, 32);
	gtk_widget_set_usize(radio, 82, 24);
	gtk_signal_connect(GTK_OBJECT(radio), "toggled",
			   (GtkSignalFunc)spdif_input_toggled, (gpointer)"Optical");

	if (hide)
		gtk_widget_hide_all(frame);
}

static void create_hardware(GtkWidget *main, GtkWidget *notebook, int page)
{
	GtkWidget *label;
	GtkWidget *fixed;

	fixed = gtk_fixed_new();
	gtk_widget_show(fixed);
	gtk_container_add(GTK_CONTAINER(notebook), fixed);
        label = gtk_label_new("Hardware Settings");
        gtk_widget_show(label);
	gtk_notebook_set_tab_label(GTK_NOTEBOOK(notebook), gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), page), label);

	create_master_clock(fixed);
	create_volume_change(fixed);
	create_spdif_output_settings(fixed);
	create_spdif_input_select(fixed);
}

static void create_about(GtkWidget *main, GtkWidget *notebook, int page)
{
	GtkWidget *label;
	GtkWidget *fixed;

	fixed = gtk_fixed_new();
	gtk_widget_show(fixed);
	gtk_container_add(GTK_CONTAINER(notebook), fixed);
        label = gtk_label_new("About");
        gtk_widget_show(label);
	gtk_notebook_set_tab_label(GTK_NOTEBOOK(notebook), gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), page), label);

	/* create first line */
	label = gtk_label_new("Envy24 Control Utility " VERSION);
        gtk_widget_show(label);
	gtk_fixed_put(GTK_FIXED(fixed), label, 0, 72);
	gtk_widget_set_uposition(label, 0, 72);
	gtk_widget_set_usize(label, 736, 16);

	/* create second line */
	label = gtk_label_new("A GTK Tool for Envy24 PCI Audio Chip");
        gtk_widget_show(label);
	gtk_fixed_put(GTK_FIXED(fixed), label, 0, 104);
	gtk_widget_set_uposition(label, 0, 104);
	gtk_widget_set_usize(label, 736, 16);

	/* create third line */
	label = gtk_label_new("Copyright (c) 2000 by Jaroslav Kysela <perex@suse.cz>");
        gtk_widget_show(label);
	gtk_fixed_put(GTK_FIXED(fixed), label, 0, 152);
	gtk_widget_set_uposition(label, 0, 152);
	gtk_widget_set_usize(label, 736, 16);
}

static void create_analog_volume(GtkWidget *main, GtkWidget *notebook, int page)
{
	GtkWidget *label;
	GtkWidget *fixed;
	GtkWidget *fixed1;
	GtkWidget *frame;
	GtkObject *adj;
	GtkWidget *vscale;
	GtkWidget *radio;
	GSList *group;
	GtkWidget *scrollwin;
	GtkWidget *viewport;
	int i, j;

	scrollwin = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_show(scrollwin);
	gtk_container_add(GTK_CONTAINER(notebook), scrollwin);

        label = gtk_label_new("Analog Volume");
        gtk_widget_show(label);
	gtk_notebook_set_tab_label(GTK_NOTEBOOK(notebook), gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), page), label);

	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin), GTK_POLICY_ALWAYS, GTK_POLICY_NEVER);
	viewport = gtk_viewport_new(NULL, NULL);
	gtk_widget_show(viewport);
	gtk_container_add(GTK_CONTAINER(scrollwin), viewport);

	fixed = gtk_fixed_new();
	gtk_widget_show(fixed);
	gtk_container_add(GTK_CONTAINER(viewport), fixed);	

	/* create DAC */
	for (i = 0; i < envy_dac_volumes(); i++) {
		char name[32];
		sprintf(name, "DAC %d", i);
		frame = gtk_frame_new(name);
		gtk_widget_show(frame);
		gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
		gtk_fixed_put(GTK_FIXED(fixed), frame, 2 + i * 74, 2);
		gtk_widget_set_uposition(frame, 2 + i * 74, 2);
		gtk_widget_set_usize(frame, 70, 288);

		fixed1 = gtk_fixed_new();
		gtk_widget_show(fixed1);
		gtk_container_add(GTK_CONTAINER(frame), fixed1);	

		adj = gtk_adjustment_new(0, -127, 0, 1, 16, 0);
		av_dac_volume_adj[i] = adj;
		vscale = gtk_vscale_new(GTK_ADJUSTMENT(adj));
		gtk_scale_set_draw_value(GTK_SCALE(vscale), FALSE);
		gtk_widget_show(vscale);
		gtk_fixed_put(GTK_FIXED(fixed1), vscale, 2, 2);
		gtk_widget_set_usize(vscale, 66, 180);
		gtk_scale_set_value_pos(GTK_SCALE(vscale), GTK_POS_BOTTOM);
		gtk_scale_set_digits(GTK_SCALE(vscale), 0);
		gtk_signal_connect(GTK_OBJECT(adj), "value_changed",
				   GTK_SIGNAL_FUNC(dac_volume_adjust), (gpointer)(i));

	        label = gtk_label_new("100 (-000dB)");
	        av_dac_volume_label[i] = (GtkLabel *)label;
	        gtk_widget_show(label);
		gtk_fixed_put(GTK_FIXED(fixed1), label, 0, 182);
		gtk_widget_set_uposition(label, 0, 182);
		gtk_widget_set_usize(label, 66, 16);

		if (i >= envy_dac_senses())
			continue;
		group = NULL;
		for (j = 0; j < envy_dac_sense_items(); j++) {
			radio = gtk_radio_button_new_with_label(group, envy_dac_sense_enum_name(j));
			av_dac_sense_radio[i][j] = radio;
			gtk_widget_show(radio);
			gtk_signal_connect(GTK_OBJECT(radio), "toggled",
					   (GtkSignalFunc)dac_sense_toggled, (gpointer)((i << 8) + j));
			gtk_fixed_put(GTK_FIXED(fixed1), radio, 2, 202 + j * 20);
			gtk_widget_set_usize(radio, 66, 24);
			group = gtk_radio_button_group(GTK_RADIO_BUTTON(radio));
		}
	}

	/* create ADC */
	for (i = 0; i < envy_adc_volumes(); i++) {
		char name[32];
		sprintf(name, "ADC %d", i);
		frame = gtk_frame_new(name);
		gtk_widget_show(frame);
		gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
		gtk_fixed_put(GTK_FIXED(fixed), frame, 2 + (i + envy_dac_volumes()) * 74, 2);
		gtk_widget_set_uposition(frame, 2 + (i + envy_dac_volumes()) * 74, 2);
		gtk_widget_set_usize(frame, 70, 288);

		fixed1 = gtk_fixed_new();
		gtk_widget_show(fixed1);
		gtk_container_add(GTK_CONTAINER(frame), fixed1);	

		adj = gtk_adjustment_new(0, -127, 0, 1, 16, 0);
		av_adc_volume_adj[i] = adj;
		vscale = gtk_vscale_new(GTK_ADJUSTMENT(adj));
		gtk_scale_set_draw_value(GTK_SCALE(vscale), FALSE);
		gtk_widget_show(vscale);
		gtk_fixed_put(GTK_FIXED(fixed1), vscale, 2, 2);
		gtk_widget_set_usize(vscale, 66, 180);
		gtk_scale_set_value_pos(GTK_SCALE(vscale), GTK_POS_BOTTOM);
		gtk_scale_set_digits(GTK_SCALE(vscale), 0);
		gtk_signal_connect(GTK_OBJECT(adj), "value_changed",
				   GTK_SIGNAL_FUNC(adc_volume_adjust), (gpointer)(i));

	        label = gtk_label_new("100 (-000dB)");
	        av_adc_volume_label[i] = (GtkLabel *)label;
	        gtk_widget_show(label);
		gtk_fixed_put(GTK_FIXED(fixed1), label, 0, 182);
		gtk_widget_set_uposition(label, 0, 182);
		gtk_widget_set_usize(label, 66, 16);

		if (i >= envy_adc_senses())
			continue;
		group = NULL;
		for (j = 0; j < envy_adc_sense_items(); j++) {
			radio = gtk_radio_button_new_with_label(group, envy_adc_sense_enum_name(j));
			av_adc_sense_radio[i][j] = radio;
			gtk_widget_show(radio);
			gtk_signal_connect(GTK_OBJECT(radio), "toggled",
					   (GtkSignalFunc)adc_sense_toggled, (gpointer)((i << 8) + j));
			gtk_fixed_put(GTK_FIXED(fixed1), radio, 2, 202 + j * 20);
			gtk_widget_set_usize(radio, 66, 24);
			group = gtk_radio_button_group(GTK_RADIO_BUTTON(radio));
		}
	}
}


static void usage(void)
{
	fprintf(stderr, "usage: envy24control [-c card#] [-D control-name]\n");
}

int main(int argc, char **argv)
{
        GtkWidget *notebook;
        char *name, tmpname[8], title[128];
	int i, c, err;
	snd_ctl_card_info_t *hw_info;
	snd_ctl_elem_value_t *val;
	int npfds;
	struct pollfd *pfds;
	int page;
	static struct option long_options[] = {
		{"device", 1, 0, 'D'},
		{"card", 1, 0, 'c'},
	};

	snd_ctl_card_info_alloca(&hw_info);
	snd_ctl_elem_value_alloca(&val);

	/* Go through gtk initialization */
        gtk_init(&argc, &argv);

	name = "hw:0";
	while ((c = getopt_long(argc, argv, "D:c:", long_options, NULL)) != -1) {
		switch (c) {
		case 'c':
			i = atoi(optarg);
			if (i < 0 || i >= 8) {
				fprintf(stderr, "envy24control: invalid card number %d\n", i);
				exit(1);
			}
			sprintf(tmpname, "hw:%d", i);
			name = tmpname;
			break;
		case 'D':
			name = optarg;
			break;
		default:
			usage();
			exit(1);
			break;
		}
	}

	if ((err = snd_ctl_open(&ctl, name, 0)) < 0) {
		fprintf(stderr, "snd_ctl_open: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}
	if ((err = snd_ctl_card_info(ctl, hw_info)) < 0) {
		fprintf(stderr, "snd_ctl_card_info: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}
	if (strcmp(snd_ctl_card_info_get_driver(hw_info), "ICE1712")) {
		fprintf(stderr, "invalid card type (driver is %s)\n", snd_ctl_card_info_get_driver(hw_info));
		exit(EXIT_FAILURE);
	}

	snd_ctl_elem_value_set_interface(val, SND_CTL_ELEM_IFACE_CARD);
	snd_ctl_elem_value_set_name(val, "ICE1712 EEPROM");
	if ((err = snd_ctl_elem_read(ctl, val)) < 0) {
		fprintf(stderr, "Unable to read EEPROM contents: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}
	memcpy(&card_eeprom, snd_ctl_elem_value_get_bytes(val), 32);

	/* Initialize code */
	level_meters_init();
	patchbay_init();
	hardware_init();
	analog_volume_init();

        /* Make the title */
        sprintf(title, "Envy24 Control Utility %s (%s)", VERSION, snd_ctl_card_info_get_longname(hw_info));

        /* Create the main window */
        window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title(GTK_WINDOW(window), title);
        gtk_signal_connect(GTK_OBJECT (window), "delete_event", 
                           (GtkSignalFunc) gtk_main_quit, NULL);
        signal(SIGINT, (void *)gtk_main_quit);
        gtk_widget_set_usize(window, 740, 340);
        gtk_window_set_policy(GTK_WINDOW (window), FALSE, FALSE, FALSE);
        gtk_widget_realize(window);

        /* Create the notebook */
        notebook = gtk_notebook_new();
        gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_TOP);
        gtk_widget_show(notebook);
	gtk_container_add(GTK_CONTAINER(window), notebook);

	page = 0;
	create_mixer(window, notebook, page++);
	create_router(window, notebook, page++);
	create_hardware(window, notebook, page++);
	if (envy_analog_volume_available())
		create_analog_volume(window, notebook, page++);
	create_about(window, notebook, page++);

	npfds = snd_ctl_poll_descriptors_count(ctl);
	if (npfds > 0) {
		pfds = alloca(sizeof(*pfds) * npfds);
		npfds = snd_ctl_poll_descriptors(ctl, pfds, npfds);
		for (i = 0; i < npfds; i++)
			gdk_input_add(pfds[i].fd,
				      GDK_INPUT_READ,
				      control_input_callback,
				      ctl);
		snd_ctl_subscribe_events(ctl, 1);
	}
	gtk_timeout_add(40, level_meters_timeout_callback, NULL);
	gtk_timeout_add(100, master_clock_status_timeout_callback, NULL);

	level_meters_postinit();
	mixer_postinit();
	patchbay_postinit();
	hardware_postinit();
	analog_volume_postinit();

	gtk_widget_show(window);
	gtk_main();

	snd_ctl_close(ctl);

	return EXIT_SUCCESS;
}
