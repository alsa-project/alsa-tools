/*
 *   HDSPMixer
 *    
 *   Copyright (C) 2003 Thomas Charbonnel (thomas@undata.org)
 *    
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#pragma implementation
#include "HDSPMixerCard.h"

static char channel_map_df_ss[26] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
    18, 19, 20, 21, 22, 23, 24, 25
};

static char channel_map_mf_ss[26] = {
    0, 1, 2, 3, 4, 5, 6, 7,
    16, 17, 18, 19, 20, 21, 22, 23, 
    24, 25,
    -1, -1, -1, -1, -1, -1, -1, -1
};

static char meter_map_ds[26] = {
    0, 1, 2, 3, 8, 9, 10, 11, 16, 17, 18, 19, 
    24, 25,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

static char channel_map_ds[26] = {
    1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 
    24, 25,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

static char dest_map_mf_ss[10] = {
    0, 2, 4, 6, 16, 18, 20, 22, 24, 26 
};

static char dest_map_ds[8] = {
    0, 2, 8, 10, 16, 18, 24, 26 
};

static char dest_map_df_ss[14] = {
    0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26 
};

static char dest_map_h9652_ss[13] = {
    0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24 
};

static char dest_map_h9652_ds[7] = {
    0, 2, 8, 10, 16, 18, 24 
};

static char dest_map_h9632_ss[8] = {
    0, 2, 4, 6, 8, 10, 12, 14
};

static char dest_map_h9632_ds[6] = {
    0, 2, 8, 10, 12, 14
};

static char dest_map_h9632_qs[4] = {
    8, 10, 12, 14
};

static char channel_map_h9632_ss[16] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
};

static char channel_map_h9632_ds[12] = {
    0, 1, 2, 3, 8, 9, 10, 11, 12, 13, 14, 15
};

static char channel_map_h9632_qs[8] = {
    8, 9, 10, 11, 12, 13, 14, 15
};

static void alsactl_cb(snd_async_handler_t *handler)
{
    int err, clock_value;
    snd_ctl_t *ctl;
    snd_ctl_event_t *event;
    snd_ctl_elem_value_t *elemval;
    snd_ctl_elem_id_t *elemid;
    HDSPMixerCard *card;
    
    card = (HDSPMixerCard *)snd_async_handler_get_callback_private(handler);
    
    snd_ctl_elem_value_alloca(&elemval);
    snd_ctl_elem_id_alloca(&elemid);
    
    ctl = snd_async_handler_get_ctl(handler);
    
    if ((err = snd_ctl_nonblock(ctl, 1))) {
	printf("Error setting non blocking mode for card %s\n", card->name);
	return;
    }
    
    snd_ctl_event_malloc(&event);    
    
    while ((err = snd_ctl_read(ctl, event)) > 0) {
	if (snd_ctl_event_elem_get_numid(event) == 11 && (card == card->basew->cards[card->basew->current_card])) {
	    /* We have a clock change and are the focused card */
	    snd_ctl_event_elem_get_id(event, elemid);
	    snd_ctl_elem_value_set_id(elemval, elemid);
	    if ((err = snd_ctl_elem_read(ctl, elemval)) < 0) {
		fprintf(stderr, "Error reading snd_ctl_elem_t\n");
		snd_ctl_event_free(event);
		return;
	    }
	    clock_value = snd_ctl_elem_value_get_enumerated(elemval, 0);
	    if (clock_value == 0) {
		int new_speed = card->getAutosyncSpeed();
		if (new_speed >= 0 && new_speed != card->speed_mode) card->setMode(new_speed);
	    }
	    if (clock_value > 3 && clock_value < 7 && card->speed_mode != 1) {
		card->setMode(1);
	    } else if (clock_value < 4 && card->speed_mode != 0) {
		card->setMode(0);
	    } else if (clock_value > 6 && card->speed_mode != 2) {
		card->setMode(2);
	    }
	}
	snd_ctl_event_clear(event);
    }
    
    snd_ctl_event_free(event);
}

int HDSPMixerCard::getAutosyncSpeed()
{
    int err, rate;
    snd_ctl_elem_value_t *elemval;
    snd_ctl_elem_id_t * elemid;
    snd_ctl_t *handle;
    snd_ctl_elem_value_alloca(&elemval);
    snd_ctl_elem_id_alloca(&elemid);
    if ((err = snd_ctl_open(&handle, name, SND_CTL_NONBLOCK)) < 0) {
	fprintf(stderr, "Error accessing ctl interface on card %s\n.", name);
	return -1; 
    }
    
    snd_ctl_elem_id_set_name(elemid, "System Sample Rate");
    snd_ctl_elem_id_set_interface(elemid, SND_CTL_ELEM_IFACE_MIXER);
    snd_ctl_elem_id_set_index(elemid, 0);
    snd_ctl_elem_value_set_id(elemval, elemid);
    if (snd_ctl_elem_read(handle, elemval) < 0) {
	snd_ctl_elem_id_set_interface(elemid, SND_CTL_ELEM_IFACE_HWDEP);
	snd_ctl_elem_value_set_id(elemval, elemid);
	snd_ctl_elem_read(handle, elemval);
    }
    rate = snd_ctl_elem_value_get_integer(elemval, 0);

    snd_ctl_close(handle);

    if (rate > 96000) {
	return 2;
    } else if (rate > 48000) {
	return 1;
    }
    return 0;
}

int HDSPMixerCard::getSpeed()
{
    int err, val;
    snd_ctl_elem_value_t *elemval;
    snd_ctl_elem_id_t * elemid;
    snd_ctl_t *handle;
    snd_ctl_elem_value_alloca(&elemval);
    snd_ctl_elem_id_alloca(&elemid);
    if ((err = snd_ctl_open(&handle, name, SND_CTL_NONBLOCK)) < 0) {
	fprintf(stderr, "Error accessing ctl interface on card %s\n.", name);
	return -1; 
    }
    snd_ctl_elem_id_set_name(elemid, "Sample Clock Source");
    snd_ctl_elem_id_set_interface(elemid, SND_CTL_ELEM_IFACE_MIXER);
    snd_ctl_elem_id_set_index(elemid, 0);
    snd_ctl_elem_value_set_id(elemval, elemid);
    if (snd_ctl_elem_read(handle, elemval) < 0) {
	snd_ctl_elem_id_set_interface(elemid, SND_CTL_ELEM_IFACE_PCM);
	snd_ctl_elem_value_set_id(elemval, elemid);
	snd_ctl_elem_read(handle, elemval);
    }
    val = snd_ctl_elem_value_get_enumerated(elemval, 0);
    snd_ctl_close(handle);
    switch (val) {
    case 0:
	/* Autosync mode : We need to determine sample rate */
	return getAutosyncSpeed();
	break;
    case 1:
    case 2:
    case 3:
	/* SR <= 48000 - normal speed */
	return 0;
    case 4:
    case 5:
    case 6:
	/* SR > 48000 Hz - double speed */
	return 1;
    case 7:
    case 8:
    case 9:
	/* SR > 96000 Hz - quad speed */
	return 2;    
    default:
	/* Should never happen */
	return 0;
    }
    return 0;    
}

HDSPMixerCard::HDSPMixerCard(HDSP_IO_Type cardtype, int id)
{
    type = cardtype;
    card_id = id;
    snprintf(name, 6, "hw:%i", card_id);
    h9632_aeb.aebi = 0;
    h9632_aeb.aebo = 0;
    if (type == H9632) {
	getAeb();
	playbacks_offset = 16;
    } else {
	playbacks_offset = 26;
    }
    speed_mode = getSpeed();
    if (speed_mode < 0) {
	fprintf(stderr, "Error trying to determine speed mode for card %s, exiting.\n", name);
	exit(EXIT_FAILURE);
    }
    
    /* Set channels and mappings */
    adjustSettings();
        
    basew = NULL;
}

void HDSPMixerCard::getAeb() {
    int err;
    snd_hwdep_t *hw;
    snd_hwdep_info_t *info;
    snd_hwdep_info_alloca(&info);
    if ((err = snd_hwdep_open(&hw, name, SND_HWDEP_OPEN_DUPLEX)) != 0) {
	fprintf(stderr, "Error opening hwdep device on card %s.\n", name);
	return; 
    }
    if ((err = snd_hwdep_ioctl(hw, SNDRV_HDSP_IOCTL_GET_9632_AEB, &h9632_aeb)) < 0) {
	fprintf(stderr, "Hwdep ioctl error on card %s : %s.\n", name, snd_strerror(err));
	snd_hwdep_close(hw);
	return; 
    }
    snd_hwdep_close(hw);
}

void HDSPMixerCard::adjustSettings() {
    if (type == Multiface) {
	switch (speed_mode) {
	case 0:
	    channels = 18;
	    channel_map = channel_map_mf_ss;
	    dest_map = dest_map_mf_ss;
	    meter_map = channel_map_mf_ss;
	    lineouts = 2;
	    break;
	case 1:
	    channels = 14;
	    channel_map = meter_map_ds;
	    dest_map = dest_map_ds;
	    meter_map = meter_map_ds;
	    lineouts = 2;
	    break;
	case 2:
	    /* should never happen */
	    break;
	}
    } else if (type == Digiface) {
	switch (speed_mode) {
	case 0:
	    channels = 26;
	    channel_map = channel_map_df_ss;
	    dest_map = dest_map_df_ss;
	    meter_map = channel_map_df_ss;
	    lineouts = 2;
	    break;
	case 1:
	    channels = 14;
	    channel_map = channel_map_ds;
	    dest_map = dest_map_ds;
	    meter_map = meter_map_ds;
	    lineouts = 2;
	    break;
	case 2:
	    /* should never happen */
	    break;
	}
    } else if (type == H9652) {
	switch (speed_mode) {
	case 0:
	    channels = 26;
	    channel_map = channel_map_df_ss;
	    dest_map = dest_map_h9652_ss;
	    meter_map = channel_map_df_ss;
	    lineouts = 0;
	    break;
	case 1:
	    channels = 14;
	    channel_map = channel_map_ds;
	    dest_map = dest_map_h9652_ds;
	    meter_map = meter_map_ds;
	    lineouts = 0;
	    break;
	case 2:
	    /* should never happen */
	    break;
	}
    } else if (type == H9632) {
	switch (speed_mode) {
	case 0:
	    channels = 12 + ((h9632_aeb.aebi || h9632_aeb.aebo) ? 4 : 0);
	    channel_map = channel_map_h9632_ss;
	    dest_map = dest_map_h9632_ss;
	    meter_map = channel_map_h9632_ss;
	    lineouts = 0;
	    break;
	case 1:
	    channels = 8 + ((h9632_aeb.aebi || h9632_aeb.aebo) ? 4 : 0);
	    channel_map = channel_map_h9632_ds;
	    dest_map = dest_map_h9632_ds;
	    meter_map = channel_map_h9632_ds;
	    lineouts = 0;
	    break;
	case 2:
	    channels = 4 + ((h9632_aeb.aebi || h9632_aeb.aebo) ? 4 : 0);
	    channel_map = channel_map_h9632_qs;
	    dest_map = dest_map_h9632_qs;
	    meter_map = channel_map_h9632_qs;
	    lineouts = 0;
	    break;
	}
    }
    window_width = (channels+2)*STRIP_WIDTH;
    window_height = FULLSTRIP_HEIGHT*2+SMALLSTRIP_HEIGHT+MENU_HEIGHT;
} 

void HDSPMixerCard::setMode(int mode)
{
    speed_mode = mode;
    adjustSettings();
    actualizeStrips();

    for (int i = 0; i < channels; ++i) {
	basew->inputs->strips[i]->targets->setLabels();
	basew->playbacks->strips[i]->targets->setLabels();
	basew->outputs->strips[i]->setLabels();
    }
    for (int i = channels; i < channels+lineouts; ++i) {
	basew->outputs->strips[i]->setLabels();    
    }
    if (h9632_aeb.aebo && !h9632_aeb.aebi) {
	basew->inputs->empty_aebi[0]->position(STRIP_WIDTH*(channels-4), basew->inputs->empty_aebi[0]->y());
	basew->inputs->empty_aebi[1]->position(STRIP_WIDTH*(channels-2), basew->inputs->empty_aebi[1]->y());
    } else if (h9632_aeb.aebi && !h9632_aeb.aebo) {
	basew->playbacks->empty_aebo[0]->position(STRIP_WIDTH*(channels-4), basew->playbacks->empty_aebo[0]->y());
	basew->playbacks->empty_aebo[1]->position(STRIP_WIDTH*(channels-2), basew->playbacks->empty_aebo[1]->y());
	basew->outputs->empty_aebo[0]->position(STRIP_WIDTH*(channels-4), basew->outputs->empty_aebo[0]->y());
	basew->outputs->empty_aebo[1]->position(STRIP_WIDTH*(channels-2), basew->outputs->empty_aebo[1]->y());
    }
    basew->inputs->buttons->position(STRIP_WIDTH*channels, basew->inputs->buttons->y());
    basew->inputs->init_sizes();
    basew->playbacks->empty->position(STRIP_WIDTH*channels, basew->playbacks->empty->y());
    basew->playbacks->init_sizes();
    basew->outputs->empty->position(STRIP_WIDTH*(channels+lineouts), basew->outputs->empty->y());    
    basew->outputs->init_sizes();
    basew->inputs->size(window_width, basew->inputs->h());
    basew->playbacks->size(window_width, basew->playbacks->h());
    basew->outputs->size(window_width, basew->outputs->h());
    basew->scroll->init_sizes();
    ((Fl_Widget *)(basew->menubar))->size(window_width, basew->menubar->h());
    basew->size_range(MIN_WIDTH, MIN_HEIGHT, window_width, window_height);
    basew->resize(basew->x(), basew->y(), window_width, basew->h());
    basew->reorder();
    basew->resetMixer();
    basew->inputs->buttons->presets->preset_change(1);
}

void HDSPMixerCard::actualizeStrips()
{
    for (int i = 0; i < HDSP_MAX_CHANNELS; ++i) {
	if (i < channels) {
	    basew->inputs->strips[i]->show();
	    basew->playbacks->strips[i]->show();
	    basew->outputs->strips[i]->show();
	} else {
	    basew->inputs->strips[i]->hide();
	    basew->playbacks->strips[i]->hide();
	    basew->outputs->strips[i]->hide();
	}
    }
    for (int i = channels; i < channels+2; ++i) {
	if (i < channels+lineouts) {
	    basew->outputs->strips[i]->show();
	} else {
	    basew->outputs->strips[i]->hide();
	}
    }
    if (h9632_aeb.aebi && !h9632_aeb.aebo) {
	for (int i = 0; i < 2; ++i) {
	    basew->inputs->empty_aebi[i]->hide();
	    basew->playbacks->empty_aebo[i]->show();
	    basew->outputs->empty_aebo[i]->show();
	}
	for (int i = channels-4; i < channels; ++i) {
	    basew->playbacks->strips[i]->hide();
	    basew->outputs->strips[i]->hide();
	}
    } else if (h9632_aeb.aebo && !h9632_aeb.aebi) { 
	for (int i = 0; i < 2; ++i) {
	    basew->inputs->empty_aebi[i]->show();
	    basew->playbacks->empty_aebo[i]->hide();
	    basew->outputs->empty_aebo[i]->hide();
	}        
	for (int i = channels-4; i < channels; ++i) {
	    basew->inputs->strips[i]->hide();
	}
    } else {
	for (int i = 0; i < 2; ++i) {
	    basew->inputs->empty_aebi[i]->hide();
	    basew->playbacks->empty_aebo[i]->hide();
	    basew->outputs->empty_aebo[i]->hide();
	}
    }
    if (type != H9652 && type != H9632) basew->outputs->empty->hide();
}

int HDSPMixerCard::initializeCard(HDSPMixerWindow *w)
{
    int err;
    if ((err = snd_ctl_open(&cb_handle, name, SND_CTL_NONBLOCK)) < 0) {
	fprintf(stderr, "Error opening ctl interface for card %s - exiting\n", name);
	exit(EXIT_FAILURE);
    }
    if ((err = snd_async_add_ctl_handler(&cb_handler, cb_handle, alsactl_cb, this)) < 0) {
	fprintf(stderr, "Error registering async ctl callback for card %s - exiting\n", name);
	exit(EXIT_FAILURE);
    }
    if ((err = snd_ctl_subscribe_events(cb_handle, 1)) < 0) {
	fprintf(stderr, "Error subscribing to ctl events for card %s - exiting\n", name);
	exit(EXIT_FAILURE);
    }
    basew = w;
    actualizeStrips();
    return 0;
}

