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

static snd_ctl_elem_value_t *internal_clock;
static snd_ctl_elem_value_t *word_clock_sync;
static snd_ctl_elem_value_t *rate_locking;
static snd_ctl_elem_value_t *rate_reset;
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
	int err, rate;
	
	if ((err = snd_ctl_elem_read(ctl, internal_clock)) < 0)
		g_print("Unable to read Internal Clock state: %s\n", snd_strerror(err));
	if (card_eeprom.subvendor == ICE1712_SUBDEVICE_DELTA1010) {
		if ((err = snd_ctl_elem_read(ctl, word_clock_sync)) < 0)
			g_print("Unable to read word clock sync selection: %s\n", snd_strerror(err));
	}
	if (snd_ctl_elem_value_get_enumerated(internal_clock, 0) == 13) {
		if (snd_ctl_elem_value_get_boolean(word_clock_sync, 0)) {
			toggle_set(hw_master_clock_word_radio, TRUE);
		} else {
			toggle_set(hw_master_clock_spdif_radio, TRUE);
		}
	} else {
//		toggle_set(hw_master_clock_xtal_radio, TRUE);
		rate = snd_ctl_elem_value_get_enumerated(internal_clock, 0);
		switch (rate) {
		case 5: toggle_set(hw_master_clock_xtal_22050, TRUE); break;
		case 7: toggle_set(hw_master_clock_xtal_32000, TRUE); break;
		case 8: toggle_set(hw_master_clock_xtal_44100, TRUE); break;
		case 9: toggle_set(hw_master_clock_xtal_48000, TRUE); break;
		case 11: toggle_set(hw_master_clock_xtal_88200, TRUE); break;
		case 12: toggle_set(hw_master_clock_xtal_96000, TRUE); break;
		default:
			    g_print("Error in rate: %d\n", rate);
			    break;
		}
	}
	internal_clock_status_timeout_callback(NULL);
	master_clock_status_timeout_callback(NULL);
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

static void internal_clock_set(int xrate)
{
	int err;

	master_clock_word_select(0);
	snd_ctl_elem_value_set_enumerated(internal_clock, 0, xrate);
	if ((err = snd_ctl_elem_write(ctl, internal_clock)) < 0)
		g_print("Unable to write internal clock rate: %s\n", snd_strerror(err));
}

void internal_clock_toggled(GtkWidget *togglebutton, gpointer data)
{
	char *what = (char *) data;

	if (!is_active(togglebutton))
		return;
	if (!strcmp(what, "22050")) {
		internal_clock_set(5);
	} else if (!strcmp(what, "32000")) {
		internal_clock_set(7);
	} else if (!strcmp(what, "44100")) {
		internal_clock_set(8);
	} else if (!strcmp(what, "48000")) {
		internal_clock_set(9);
	} else if (!strcmp(what, "88200")) {
		internal_clock_set(11);
	} else if (!strcmp(what, "96000")) {
		internal_clock_set(12);
	} else if (!strcmp(what, "SPDIF")) {
		internal_clock_set(13);
	} else if (!strcmp(what, "WordClock")) {
		master_clock_word_select(1);
	} else {
		g_print("internal_clock_toggled: %s ???\n", what);
	}
}

static int is_rate_locked(void)
{
	int err;
	
	if ((err = snd_ctl_elem_read(ctl, rate_locking)) < 0)
		g_print("Unable to read rate locking state: %s\n", snd_strerror(err));
	return snd_ctl_elem_value_get_boolean(rate_locking, 0) ? 1 : 0;
}

static int is_rate_reset(void)
{
	int err;
	
	if ((err = snd_ctl_elem_read(ctl, rate_reset)) < 0)
		g_print("Unable to read rate reset state: %s\n", snd_strerror(err));
	return snd_ctl_elem_value_get_boolean(rate_reset, 0) ? 1 : 0;
}

static inline int is_update_needed(void)
{
	return (is_rate_locked() || !is_rate_reset());
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

gint internal_clock_status_timeout_callback(gpointer data)
{
	int err, rate, need_update;
	char *label;
	
	if ((err = snd_ctl_elem_read(ctl, internal_clock)) < 0)
		g_print("Unable to read Internal Clock state: %s\n", snd_strerror(err));
	if (card_eeprom.subvendor == ICE1712_SUBDEVICE_DELTA1010) {
		if ((err = snd_ctl_elem_read(ctl, word_clock_sync)) < 0)
			g_print("Unable to read word clock sync selection: %s\n", snd_strerror(err));
	}
	need_update = is_update_needed() ? 1 : 0;
	if (snd_ctl_elem_value_get_enumerated(internal_clock, 0) == 13) {
		if (snd_ctl_elem_value_get_boolean(word_clock_sync, 0)) {
			label = "Word Clock";
		} else {
			label = "S/PDIF";
		}
	} else {
//		toggle_set(hw_master_clock_xtal_radio, TRUE);
		rate = snd_ctl_elem_value_get_enumerated(internal_clock, 0);
//		g_print("Rate: %d need_update: %d\n", rate, need_update); // for debug
		switch (rate) {
		case 0: label = "8000"; break;
		case 1: label = "9600"; break;
		case 2: label = "11025"; break;
		case 3: label = "12000"; break;
		case 4: label = "16000"; break;
		case 5: label = "22050";
			if (need_update)
			toggle_set(hw_master_clock_xtal_22050, TRUE); break;
		case 6: label = "24000"; break;
		case 7: label = "32000";
			if (need_update)
			toggle_set(hw_master_clock_xtal_32000, TRUE); break;
		case 8: label = "44100";
			if (need_update)
			toggle_set(hw_master_clock_xtal_44100, TRUE); break;
		case 9: label = "48000";
			if (need_update)
			toggle_set(hw_master_clock_xtal_48000, TRUE); break;
		case 10: label = "64000"; break;
		case 11: label = "88200";
			if (need_update)
			toggle_set(hw_master_clock_xtal_88200, TRUE); break;
		case 12: label = "96000";
			if (need_update)
			toggle_set(hw_master_clock_xtal_96000, TRUE); break;
		default:
			    label = "ERROR";
			    g_print("Error in rate: %d\n", rate);
			    break;
		}
	}
	gtk_label_set_text(GTK_LABEL(hw_master_clock_actual_rate_label), label);
	return TRUE;
}

void rate_locking_update(void)
{
	int err;
	
	if ((err = snd_ctl_elem_read(ctl, rate_locking)) < 0)
		g_print("Unable to read rate locking state: %s\n", snd_strerror(err));
	if (snd_ctl_elem_value_get_boolean(rate_locking, 0))
			toggle_set(hw_rate_locking_check, TRUE);
}

void rate_reset_update(void)
{
	int err;
	
	if ((err = snd_ctl_elem_read(ctl, rate_reset)) < 0)
		g_print("Unable to read rate reset state: %s\n", snd_strerror(err));
	if (snd_ctl_elem_value_get_boolean(rate_reset, 0))
			toggle_set(hw_rate_reset_check, TRUE);
}

static void rate_locking_set(int on)
{
	int err;

	snd_ctl_elem_value_set_boolean(rate_locking, 0, on ? 1 : 0);
	if ((err = snd_ctl_elem_write(ctl, rate_locking)) < 0)
		g_print("Unable to write rate locking state: %s\n", snd_strerror(err));
}

static void rate_reset_set(int on)
{
	int err;

	snd_ctl_elem_value_set_boolean(rate_reset, 0, on ? 1 : 0);
	if ((err = snd_ctl_elem_write(ctl, rate_reset)) < 0)
		g_print("Unable to write rate reset state: %s\n", snd_strerror(err));
}

void rate_locking_toggled(GtkWidget *togglebutton, gpointer data)
{
	char *what = (char *) data;

	if (!is_active(togglebutton)) {
		rate_locking_set(0);
		return;
	}
	if (!strcmp(what, "locked")) {
		rate_locking_set(1);
		internal_clock_status_timeout_callback(NULL);
	} else {
		g_print("rate_locking_toggled: %s ???\n", what);
	}
}

void rate_reset_toggled(GtkWidget *togglebutton, gpointer data)
{
	char *what = (char *) data;

	if (!is_active(togglebutton)) {
		rate_reset_set(0);
		internal_clock_status_timeout_callback(NULL);
		return;
	}
	if (!strcmp(what, "reset")) {
		rate_reset_set(1);
	} else {
		g_print("rate_reset_toggled: %s ???\n", what);
	}
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
	int err;
	snd_aes_iec958_t iec958;
	
	if ((err = snd_ctl_elem_read(ctl, spdif_output)) < 0) {
		if (err == -ENOENT)
			return;
		g_print("Unable to read Delta S/PDIF output state: %s\n", snd_strerror(err));
	}
	snd_ctl_elem_value_get_iec958(spdif_output, &iec958);
	if (!(iec958.status[0] & IEC958_AES0_PROFESSIONAL)) {		/* consumer */
		toggle_set(hw_spdif_consumer_radio, TRUE);
		if (iec958.status[0] & IEC958_AES0_CON_NOT_COPYRIGHT) {
			toggle_set(hw_consumer_copyright_off_radio, TRUE);
		} else {
			toggle_set(hw_consumer_copyright_on_radio, TRUE);
		}
		if ((iec958.status[0] & IEC958_AES0_CON_EMPHASIS) != IEC958_AES0_CON_EMPHASIS_5015) {
			toggle_set(hw_consumer_emphasis_none_radio, TRUE);
		} else {
			toggle_set(hw_consumer_emphasis_5015_radio, TRUE);
		}
		switch (iec958.status[1] & IEC958_AES1_CON_CATEGORY) {
		case IEC958_AES1_CON_MAGNETIC_ID: toggle_set(hw_consumer_category_dat_radio, TRUE); break;
		case IEC958_AES1_CON_DIGDIGCONV_ID: toggle_set(hw_consumer_category_pcm_radio, TRUE); break;
		case IEC958_AES1_CON_GENERAL: toggle_set(hw_consumer_category_general_radio, TRUE); break;
		case IEC958_AES1_CON_LASEROPT_ID:
		default: toggle_set(hw_consumer_category_cd_radio, TRUE); break;
		}
		if (iec958.status[1] & IEC958_AES1_CON_ORIGINAL) {
			toggle_set(hw_consumer_copy_original_radio, TRUE);
		} else {
			toggle_set(hw_consumer_copy_1st_radio, TRUE);
		}
	} else {
		toggle_set(hw_spdif_professional_radio, TRUE);
		if (!(iec958.status[0] & IEC958_AES0_NONAUDIO)) {
			toggle_set(hw_spdif_profi_audio_radio, TRUE);
		} else {
			toggle_set(hw_spdif_profi_nonaudio_radio, TRUE);
		}
		switch (iec958.status[0] & IEC958_AES0_PRO_EMPHASIS) {
		case IEC958_AES0_PRO_EMPHASIS_CCITT: toggle_set(hw_profi_emphasis_ccitt_radio, TRUE); break;
		case IEC958_AES0_PRO_EMPHASIS_NONE: toggle_set(hw_profi_emphasis_none_radio, TRUE); break;
		case IEC958_AES0_PRO_EMPHASIS_5015: toggle_set(hw_profi_emphasis_5015_radio, TRUE); break;
		case IEC958_AES0_PRO_EMPHASIS_NOTID:
		default: toggle_set(hw_profi_emphasis_notid_radio, TRUE); break;
		}
		if ((iec958.status[1] & IEC958_AES1_PRO_MODE) == IEC958_AES1_PRO_MODE_STEREOPHONIC) {
			toggle_set(hw_profi_stream_stereo_radio, TRUE);
		} else {
			toggle_set(hw_profi_stream_notid_radio, TRUE);
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
	snd_aes_iec958_t iec958;

	snd_ctl_elem_value_get_iec958(spdif_output, &iec958);
	if (!is_active(togglebutton))
		return;
	if (!(iec958.status[0] & IEC958_AES0_PROFESSIONAL))
		return;
	if (!strcmp(str, "Audio")) {
		iec958.status[0] &= ~IEC958_AES0_NONAUDIO;
	} else if (!strcmp(str, "Non-audio")) {
		iec958.status[0] |= IEC958_AES0_NONAUDIO;
	}
	snd_ctl_elem_value_set_iec958(spdif_output, &iec958);
	spdif_output_write();
}

void profi_stream_toggled(GtkWidget *togglebutton, gpointer data)
{
	char *str = (char *)data;
	snd_aes_iec958_t iec958;

	if (!is_active(togglebutton))
		return;
	snd_ctl_elem_value_get_iec958(spdif_output, &iec958);
	if (!(iec958.status[0] & IEC958_AES0_PROFESSIONAL))
		return;
	iec958.status[1] &= ~IEC958_AES1_PRO_MODE;
	if (!strcmp(str, "NOTID")) {
		iec958.status[0] |= IEC958_AES1_PRO_MODE_STEREOPHONIC;
	} else if (!strcmp(str, "Stereo")) {
		iec958.status[0] |= IEC958_AES1_PRO_MODE_NOTID;
	}
	snd_ctl_elem_value_set_iec958(spdif_output, &iec958);
	spdif_output_write();
}

void profi_emphasis_toggled(GtkWidget *togglebutton, gpointer data)
{
	char *str = (char *)data;
	snd_aes_iec958_t iec958;

	snd_ctl_elem_value_get_iec958(spdif_output, &iec958);
	if (!is_active(togglebutton))
		return;
	if (!(iec958.status[0] & IEC958_AES0_PROFESSIONAL))
		return;
	iec958.status[0] &= ~IEC958_AES0_PRO_EMPHASIS;
	if (!strcmp(str, "CCITT")) {
		iec958.status[0] |= IEC958_AES0_PRO_EMPHASIS_CCITT;
	} else if (!strcmp(str, "No")) {
		iec958.status[0] |= IEC958_AES0_PRO_EMPHASIS_NONE;
	} else if (!strcmp(str, "5015")) {
		iec958.status[0] |= IEC958_AES0_PRO_EMPHASIS_5015;
	} else if (!strcmp(str, "NOTID")) {
		iec958.status[0] |= IEC958_AES0_PRO_EMPHASIS_NOTID;
	}
	snd_ctl_elem_value_set_iec958(spdif_output, &iec958);
	spdif_output_write();
}

void consumer_copyright_toggled(GtkWidget *togglebutton, gpointer data)
{
	char *str = (char *)data;
	snd_aes_iec958_t iec958;

	snd_ctl_elem_value_get_iec958(spdif_output, &iec958);	
	if (!is_active(togglebutton))
		return;
	if (iec958.status[0] & IEC958_AES0_PROFESSIONAL)
		return;
	if (!strcmp(str, "Copyright")) {
		iec958.status[0] &= ~IEC958_AES0_CON_NOT_COPYRIGHT;
	} else if (!strcmp(str, "Permitted")) {
		iec958.status[1] |= IEC958_AES0_CON_NOT_COPYRIGHT;
	}
	snd_ctl_elem_value_set_iec958(spdif_output, &iec958);
	spdif_output_write();
}

void consumer_copy_toggled(GtkWidget *togglebutton, gpointer data)
{
	char *str = (char *)data;
	snd_aes_iec958_t iec958;

	snd_ctl_elem_value_get_iec958(spdif_output, &iec958);	
	if (!is_active(togglebutton))
		return;
	if (iec958.status[0] & IEC958_AES0_PROFESSIONAL)
		return;
	if (!strcmp(str, "1st")) {
		iec958.status[0] |= IEC958_AES1_CON_ORIGINAL;
	} else if (!strcmp(str, "Original")) {
		iec958.status[1] &= ~IEC958_AES1_CON_ORIGINAL;
	}
	snd_ctl_elem_value_set_iec958(spdif_output, &iec958);
	spdif_output_write();
}

void consumer_emphasis_toggled(GtkWidget *togglebutton, gpointer data)
{
	char *str = (char *)data;
	snd_aes_iec958_t iec958;

	snd_ctl_elem_value_get_iec958(spdif_output, &iec958);	
	if (!is_active(togglebutton))
		return;
	if (iec958.status[0] & IEC958_AES0_PROFESSIONAL)
		return;
	iec958.status[0] &= ~IEC958_AES0_CON_EMPHASIS;
	if (!strcmp(str, "No")) {
		iec958.status[0] |= IEC958_AES0_CON_EMPHASIS_NONE;
	} else if (!strcmp(str, "5015")) {
		iec958.status[1] |= ~IEC958_AES0_CON_EMPHASIS_5015;
	}
	snd_ctl_elem_value_set_iec958(spdif_output, &iec958);
	spdif_output_write();
}

void consumer_category_toggled(GtkWidget *togglebutton, gpointer data)
{
	char *str = (char *)data;
	snd_aes_iec958_t iec958;

	snd_ctl_elem_value_get_iec958(spdif_output, &iec958);	
	if (!is_active(togglebutton))
		return;
	if (iec958.status[0] & IEC958_AES0_PROFESSIONAL)
		return;
	iec958.status[0] &= ~IEC958_AES1_CON_CATEGORY;
	if (!strcmp(str, "DAT")) {
		iec958.status[0] |= IEC958_AES1_CON_DAT;
	} else if (!strcmp(str, "PCM")) {
		iec958.status[0] |= IEC958_AES1_CON_PCM_CODER;
	} else if (!strcmp(str, "CD")) {
		iec958.status[0] |= IEC958_AES1_CON_IEC908_CD;
	} else if (!strcmp(str, "General")) {
		iec958.status[0] |= IEC958_AES1_CON_GENERAL;
	}
	snd_ctl_elem_value_set_iec958(spdif_output, &iec958);
	spdif_output_write();
}

void spdif_output_toggled(GtkWidget *togglebutton, gpointer data)
{
	char *str = (char *)data;
	snd_aes_iec958_t iec958;
	int page;

	if (is_active(togglebutton)) {
		snd_ctl_elem_value_get_iec958(spdif_output, &iec958);
		if (!strcmp(str, "Professional")) {
			if (!(iec958.status[0] & IEC958_AES0_PROFESSIONAL)) {
				/* default setup: audio, no emphasis */
				memset(&iec958, 0, sizeof(iec958));
				iec958.status[0] = IEC958_AES0_PROFESSIONAL | IEC958_AES0_PRO_EMPHASIS_NONE | IEC958_AES0_PRO_FS_48000;
				iec958.status[1] = IEC958_AES1_PRO_MODE_STEREOPHONIC;
				snd_ctl_elem_value_set_iec958(spdif_output, &iec958);
			}
			page = 0;
		} else {
			if (iec958.status[0] & IEC958_AES0_PROFESSIONAL) {
				/* default setup: no emphasis, PCM encoder */
				memset(&iec958, 0, sizeof(iec958));
				iec958.status[0] = IEC958_AES0_CON_EMPHASIS_NONE;
				iec958.status[1] = IEC958_AES1_CON_PCM_CODER | IEC958_AES1_CON_ORIGINAL;
				iec958.status[3] = IEC958_AES3_CON_FS_48000;
				snd_ctl_elem_value_set_iec958(spdif_output, &iec958);
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
	if (snd_ctl_elem_value_malloc(&internal_clock) < 0 ||
	    snd_ctl_elem_value_malloc(&word_clock_sync) < 0 ||
	    snd_ctl_elem_value_malloc(&rate_locking) < 0 ||
	    snd_ctl_elem_value_malloc(&rate_reset) < 0 ||
	    snd_ctl_elem_value_malloc(&volume_rate) < 0 ||
	    snd_ctl_elem_value_malloc(&spdif_input) < 0 ||
	    snd_ctl_elem_value_malloc(&spdif_output) < 0) {
		g_print("Cannot allocate memory\n");
		exit(1);
	}

	snd_ctl_elem_value_set_interface(internal_clock, SND_CTL_ELEM_IFACE_MIXER);
	snd_ctl_elem_value_set_name(internal_clock, "Multi Track Internal Clock");

	snd_ctl_elem_value_set_interface(word_clock_sync, SND_CTL_ELEM_IFACE_PCM);
	snd_ctl_elem_value_set_name(word_clock_sync, "Word Clock Sync");

	snd_ctl_elem_value_set_interface(rate_locking, SND_CTL_ELEM_IFACE_MIXER);
	snd_ctl_elem_value_set_name(rate_locking, "Multi Track Rate Locking");

	snd_ctl_elem_value_set_interface(rate_reset, SND_CTL_ELEM_IFACE_MIXER);
	snd_ctl_elem_value_set_name(rate_reset, "Multi Track Rate Reset");

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
	rate_locking_update();
	rate_reset_update();
	volume_change_rate_update();
	spdif_input_update();
	spdif_output_update();
}
