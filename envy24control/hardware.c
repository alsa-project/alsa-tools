/*****************************************************************************
   hardware.c - Hardware Settings
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

static snd_ctl_elem_value_t *spdif_master;
static snd_ctl_elem_value_t *word_clock_sync;
static snd_ctl_elem_value_t *volume_rate;
static snd_ctl_elem_value_t *spdif_input;
static snd_ctl_elem_value_t *spdif_output;

#define toggle_set(widget, state) \
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), state);

static int is_active(GtkWidget *widget)
{
	return GTK_TOGGLE_BUTTON(widget)->active ? 1 : 0;
}

void master_clock_update(void)
{
	int err;
	
	if ((err = snd_ctl_elem_read(ctl, spdif_master)) < 0)
		g_print("Unable to read S/PDIF master state: %s\n", snd_strerror(err));
	if (card_eeprom.subvendor == ICE1712_SUBDEVICE_DELTA1010) {
		if ((err = snd_ctl_elem_read(ctl, word_clock_sync)) < 0)
			g_print("Unable to read word clock sync selection: %s\n", snd_strerror(err));
	}
	if (snd_ctl_elem_value_get_boolean(spdif_master, 0)) {
		if (snd_ctl_elem_value_get_boolean(word_clock_sync, 0)) {
			toggle_set(hw_master_clock_word_radio, TRUE);
		} else {
			toggle_set(hw_master_clock_spdif_radio, TRUE);
		}
	} else {
		toggle_set(hw_master_clock_xtal_radio, TRUE);
	}
	master_clock_status_timeout_callback(NULL);
}

static void master_clock_spdif_master(int on)
{
	int err;

	snd_ctl_elem_value_set_boolean(spdif_master, 0, on ? 1 : 0);
	if ((err = snd_ctl_elem_write(ctl, spdif_master)) < 0)
		g_print("Unable to write S/PDIF master state: %s\n", snd_strerror(err));
}

static void master_clock_word_select(int on)
{
	int err;

	if (card_eeprom.subvendor != ICE1712_SUBDEVICE_DELTA1010)
		return;
	snd_ctl_elem_value_set_boolean(word_clock_sync, 0, on ? 1 : 0);
	if ((err = snd_ctl_elem_write(ctl, word_clock_sync)) < 0)
		g_print("Unable to write word clock sync selection: %s\n", snd_strerror(err));
}

void master_clock_toggled(GtkWidget *togglebutton, gpointer data)
{
	char *what = (char *) data;

	if (!is_active(togglebutton))
		return;
	if (!strcmp(what, "Xtal")) {
		master_clock_spdif_master(0);
	} else if (!strcmp(what, "SPDIF")) {
		master_clock_spdif_master(1);
		master_clock_word_select(0);
	} else if (!strcmp(what, "WordClock")) {
		master_clock_spdif_master(1);
		master_clock_word_select(1);
	} else {
		g_print("master_clock_toggled: %s ???\n", what);
	}
}

gint master_clock_status_timeout_callback(gpointer data)
{
	snd_ctl_elem_value_t *sw;
	int err;
	
	if (card_eeprom.subvendor != ICE1712_SUBDEVICE_DELTA1010)
		return FALSE;
	snd_ctl_elem_value_alloca(&sw);
	snd_ctl_elem_value_set_interface(sw, SND_CTL_ELEM_IFACE_PCM);
	snd_ctl_elem_value_set_name(sw, "Word Clock Status");
	if ((err = snd_ctl_elem_read(ctl, sw)) < 0)
		g_print("Unable to determine word clock status: %s\n", snd_strerror(err));
	gtk_label_set_text(GTK_LABEL(hw_master_clock_status_label),
			   snd_ctl_elem_value_get_boolean(sw, 0) ? "Locked" : "No signal");
	return TRUE;
}

void volume_change_rate_update(void)
{
	int err;
	
	if ((err = snd_ctl_elem_read(ctl, volume_rate)) < 0)
		g_print("Unable to read volume change rate: %s\n", snd_strerror(err));
	gtk_adjustment_set_value(GTK_ADJUSTMENT(hw_volume_change_adj),
				 snd_ctl_elem_value_get_integer(volume_rate, 0));
}

void volume_change_rate_adj(GtkAdjustment *adj, gpointer data)
{
	int err;
	
	snd_ctl_elem_value_set_integer(volume_rate, 0, adj->value);
	if ((err = snd_ctl_elem_write(ctl, volume_rate)) < 0)
		g_print("Unable to write volume change rate: %s\n", snd_strerror(err));
}

void spdif_output_update(void)
{
	int err, val;
	
	if (card_eeprom.subvendor == ICE1712_SUBDEVICE_DELTA44)
		return;
	if ((err = snd_ctl_elem_read(ctl, spdif_output)) < 0)
		g_print("Unable to read Delta S/PDIF output state: %s\n", snd_strerror(err));
	val = snd_ctl_elem_value_get_integer(spdif_output, 0);
	if (val & 1) {		/* consumer */
		toggle_set(hw_spdif_consumer_radio, TRUE);
		if (val & 8) {
			toggle_set(hw_consumer_copyright_on_radio, TRUE);
		} else {
			toggle_set(hw_consumer_copyright_off_radio, TRUE);
		}
		if (val & 0x10) {
			toggle_set(hw_consumer_emphasis_none_radio, TRUE);
		} else {
			toggle_set(hw_consumer_emphasis_5015_radio, TRUE);
		}
		switch (val & 0x60) {
		case 0x00: toggle_set(hw_consumer_category_dat_radio, TRUE); break;
		case 0x20: toggle_set(hw_consumer_category_pcm_radio, TRUE); break;
		case 0x40: toggle_set(hw_consumer_category_cd_radio, TRUE); break;
		case 0x60: toggle_set(hw_consumer_category_general_radio, TRUE); break;
		}
		if (val & 0x80) {
			toggle_set(hw_consumer_copy_1st_radio, TRUE);
		} else {
			toggle_set(hw_consumer_copy_original_radio, TRUE);
		}
	} else {
		toggle_set(hw_spdif_professional_radio, TRUE);
		if (val & 2) {
			toggle_set(hw_spdif_profi_audio_radio, TRUE);
		} else {
			toggle_set(hw_spdif_profi_nonaudio_radio, TRUE);
		}
		switch (val & 0x60) {
		case 0x00: toggle_set(hw_profi_emphasis_ccitt_radio, TRUE); break;
		case 0x20: toggle_set(hw_profi_emphasis_none_radio, TRUE); break;
		case 0x40: toggle_set(hw_profi_emphasis_5015_radio, TRUE); break;
		case 0x60: toggle_set(hw_profi_emphasis_notid_radio, TRUE); break;
		}
		if (val & 0x80) {
			toggle_set(hw_profi_stream_notid_radio, TRUE);
		} else {
			toggle_set(hw_profi_stream_stereo_radio, TRUE);
		}
	}
}

static void spdif_output_write(void)
{
	int err;

	if ((err = snd_ctl_elem_write(ctl, spdif_output)) < 0)
		g_print("Unable to write Delta S/PDIF Output Defaults: %s\n", snd_strerror(err));
}

void profi_data_toggled(GtkWidget *togglebutton, gpointer data)
{
	char *str = (char *)data;
	int val = snd_ctl_elem_value_get_integer(spdif_output, 0);
	
	if (!is_active(togglebutton))
		return;
	if (val & 1)
		return;
	if (!strcmp(str, "Audio")) {
		val |= 0x02;
	} else if (!strcmp(str, "Non-audio")) {
		val &= ~0x02;
	}
	snd_ctl_elem_value_set_integer(spdif_output, 0, val);
	spdif_output_write();
}

void profi_stream_toggled(GtkWidget *togglebutton, gpointer data)
{
	char *str = (char *)data;
	int val = snd_ctl_elem_value_get_integer(spdif_output, 0);
	
	if (!is_active(togglebutton))
		return;
	if (val & 1)
		return;
	if (!strcmp(str, "NOTID")) {
		val |= 0x80;
	} else if (!strcmp(str, "Stereo")) {
		val &= ~0x80;
	}
	snd_ctl_elem_value_set_integer(spdif_output, 0, val);
	spdif_output_write();
}

void profi_emphasis_toggled(GtkWidget *togglebutton, gpointer data)
{
	char *str = (char *)data;
	int val = snd_ctl_elem_value_get_integer(spdif_output, 0);
	
	if (!is_active(togglebutton))
		return;
	if (val & 1)
		return;
	if (!strcmp(str, "CCITT")) {
		val &= ~0x60;
	} else if (!strcmp(str, "No")) {
		val &= ~0x60;
		val |= 0x20;
	} else if (!strcmp(str, "5015")) {
		val &= ~0x60;
		val |= 0x40;
	} else if (!strcmp(str, "NOTID")) {
		val |= 0x60;
	}
	snd_ctl_elem_value_set_integer(spdif_output, 0, val);
	spdif_output_write();
}

void consumer_copyright_toggled(GtkWidget *togglebutton, gpointer data)
{
	char *str = (char *)data;
	int val = snd_ctl_elem_value_get_integer(spdif_output, 0);
	
	if (!is_active(togglebutton))
		return;
	if (!(val & 1))
		return;
	if (!strcmp(str, "Copyright")) {
		val |= 0x08;
	} else if (!strcmp(str, "Permitted")) {
		val &= ~0x08;
	}
	snd_ctl_elem_value_set_integer(spdif_output, 0, val);
	spdif_output_write();
}

void consumer_copy_toggled(GtkWidget *togglebutton, gpointer data)
{
	char *str = (char *)data;
	int val = snd_ctl_elem_value_get_integer(spdif_output, 0);
	
	if (!is_active(togglebutton))
		return;
	if (!(val & 1))
		return;
	if (!strcmp(str, "1st")) {
		val |= 0x80;
	} else if (!strcmp(str, "Original")) {
		val &= ~0x80;
	}
	snd_ctl_elem_value_set_integer(spdif_output, 0, val);
	spdif_output_write();
}

void consumer_emphasis_toggled(GtkWidget *togglebutton, gpointer data)
{
	char *str = (char *)data;
	int val = snd_ctl_elem_value_get_integer(spdif_output, 0);
	
	if (!is_active(togglebutton))
		return;
	if (!(val & 1))
		return;
	if (!strcmp(str, "No")) {
		val |= 0x10;
	} else if (!strcmp(str, "5015")) {
		val &= ~0x10;
	}
	snd_ctl_elem_value_set_integer(spdif_output, 0, val);
	spdif_output_write();
}

void consumer_category_toggled(GtkWidget *togglebutton, gpointer data)
{
	char *str = (char *)data;
	int val = snd_ctl_elem_value_get_integer(spdif_output, 0);
	
	if (!is_active(togglebutton))
		return;
	if (!(val & 1))
		return;
	if (!strcmp(str, "DAT")) {
		val &= ~0x60;
	} else if (!strcmp(str, "PCM")) {
		val &= ~0x60;
		val |= 0x20;
	} else if (!strcmp(str, "CD")) {
		val &= ~0x60;
		val |= 0x40;
	} else if (!strcmp(str, "General")) {
		val |= 0x60;
	}
	snd_ctl_elem_value_set_integer(spdif_output, 0, val);
	spdif_output_write();
}

void spdif_output_toggled(GtkWidget *togglebutton, gpointer data)
{
	char *str = (char *)data;
	int page;

	if (is_active(togglebutton)) {
		if (!strcmp(str, "Professional")) {
			if (snd_ctl_elem_value_get_integer(spdif_output, 0) & 0x01) {
				/* default setup: audio, no emphasis */
				snd_ctl_elem_value_set_integer(spdif_output, 0, 0x22);
			}
			page = 0;
		} else {
			if (!(snd_ctl_elem_value_get_integer(spdif_output, 0) & 0x01)) {
				/* default setup: no emphasis, PCM encoder */
				snd_ctl_elem_value_set_integer(spdif_output, 0, 0x31);
			}
			page = 1;
		}
		spdif_output_write();
		gtk_notebook_set_page(GTK_NOTEBOOK(hw_spdif_output_notebook), page);
		spdif_output_update();
	}
}

void spdif_input_update(void)
{
	int err;
	
	if (card_eeprom.subvendor != ICE1712_SUBDEVICE_DELTADIO2496)
		return;
	if ((err = snd_ctl_elem_read(ctl, spdif_input)) < 0)
		g_print("Unable to read S/PDIF input switch: %s\n", snd_strerror(err));
	if (snd_ctl_elem_value_get_boolean(spdif_input, 0)) {
		toggle_set(hw_spdif_input_optical_radio, TRUE);
	} else {
		toggle_set(hw_spdif_input_coaxial_radio, TRUE);
	}
}

void spdif_input_toggled(GtkWidget *togglebutton, gpointer data)
{
	int err;
	char *str = (char *)data;
	
	if (!is_active(togglebutton))
		return;
	if (!strcmp(str, "Optical"))
		snd_ctl_elem_value_set_boolean(spdif_input, 0, 1);
	else
		snd_ctl_elem_value_set_boolean(spdif_input, 0, 0);
	if ((err = snd_ctl_elem_write(ctl, spdif_input)) < 0)
		g_print("Unable to write S/PDIF input switch: %s\n", snd_strerror(err));
}

void hardware_init(void)
{
	if (snd_ctl_elem_value_malloc(&spdif_master) < 0 ||
	    snd_ctl_elem_value_malloc(&word_clock_sync) < 0 ||
	    snd_ctl_elem_value_malloc(&volume_rate) < 0 ||
	    snd_ctl_elem_value_malloc(&spdif_input) < 0 ||
	    snd_ctl_elem_value_malloc(&spdif_output) < 0) {
		g_print("Cannot allocate memory\n");
		exit(1);
	}

	snd_ctl_elem_value_set_interface(spdif_master, SND_CTL_ELEM_IFACE_PCM);
	snd_ctl_elem_value_set_name(spdif_master, "Multi Track IEC958 Master");

	snd_ctl_elem_value_set_interface(word_clock_sync, SND_CTL_ELEM_IFACE_PCM);
	snd_ctl_elem_value_set_name(word_clock_sync, "Word Clock Sync");

	snd_ctl_elem_value_set_interface(volume_rate, SND_CTL_ELEM_IFACE_MIXER);
	snd_ctl_elem_value_set_name(volume_rate, "Multi Track Volume Rate");

	snd_ctl_elem_value_set_interface(spdif_input, SND_CTL_ELEM_IFACE_PCM);
	snd_ctl_elem_value_set_name(spdif_input, "IEC958 Input Optical");

	snd_ctl_elem_value_set_interface(spdif_output, SND_CTL_ELEM_IFACE_PCM);
	snd_ctl_elem_value_set_name(spdif_output, "IEC958 Playback Default");
}

void hardware_postinit(void)
{
	master_clock_update();
	volume_change_rate_update();
	spdif_input_update();
	spdif_output_update();
}
