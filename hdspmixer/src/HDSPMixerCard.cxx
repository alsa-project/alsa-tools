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
		if (new_speed >= 0 && new_speed != card->double_speed) card->setMode(new_speed);
	    }
	    if (clock_value > 3 && !card->double_speed) {
		card->setMode(1);
	    } else if (clock_value < 4 && card->double_speed) {
		card->setMode(0);
	    }
	}
	snd_ctl_event_clear(event);
    }
    
    snd_ctl_event_free(event);
}

int HDSPMixerCard::getAutosyncSpeed()
{
    /*  FIXME : this is over simplistic, there are lots of crooked cases
	It should always be possible to do what one wants executing the 
	proper sequence of actions, though.
    */

    int err, external_rate;
    snd_ctl_elem_value_t *elemval;
    snd_ctl_elem_id_t * elemid;
    snd_ctl_t *handle;
    snd_ctl_elem_value_alloca(&elemval);
    snd_ctl_elem_id_alloca(&elemid);
    if ((err = snd_ctl_open(&handle, name, SND_CTL_NONBLOCK)) < 0) {
	fprintf(stderr, "Error accessing ctl interface on card %s\n.", name);
	return -1; 
    }
    
    snd_ctl_elem_id_set_name(elemid, "External Rate");
    snd_ctl_elem_id_set_numid(elemid, 17);
    snd_ctl_elem_id_set_interface(elemid, SND_CTL_ELEM_IFACE_PCM);
    snd_ctl_elem_id_set_device(elemid, 0);
    snd_ctl_elem_id_set_subdevice(elemid, 0);
    snd_ctl_elem_id_set_index(elemid, 0);
    snd_ctl_elem_value_set_id(elemval, elemid);
    snd_ctl_elem_read(handle, elemval);
    external_rate = snd_ctl_elem_value_get_enumerated(elemval, 0);

    snd_ctl_close(handle);

    if (external_rate > 2 && external_rate < 6) {
	return 1;
    } else if (external_rate < 2) {
	return 0;
    }
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
    snd_ctl_elem_id_set_numid(elemid, 11);
    snd_ctl_elem_id_set_interface(elemid, SND_CTL_ELEM_IFACE_PCM);
    snd_ctl_elem_id_set_device(elemid, 0);
    snd_ctl_elem_id_set_subdevice(elemid, 0);
    snd_ctl_elem_id_set_index(elemid, 0);
    snd_ctl_elem_value_set_id(elemval, elemid);
    snd_ctl_elem_read(handle, elemval);
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
	/* SR > 48000 kHz - double speed */
	return 1;
	break;
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
    double_speed = getSpeed();
    if (double_speed < 0) {
	fprintf(stderr, "Error trying to determine speed mode for card %s, exiting.\n", name);
	exit(EXIT_FAILURE);
    }
    
    /* Set channels and mappings */
    adjustSettings();
        
    basew = NULL;
}


void HDSPMixerCard::adjustSettings() {
    if (type == Multiface && !double_speed) {
	channels = 18;
	channel_map = channel_map_mf_ss;
	dest_map = dest_map_mf_ss;
	meter_map = channel_map_mf_ss;
	lineouts = 2;
    } else if (type == Multiface && double_speed) {
	channels = 14;
	/* FIXME : this is a workaround because the driver is wrong */
	channel_map = meter_map_ds;
	dest_map = dest_map_ds;
	meter_map = meter_map_ds;
	lineouts = 2;
    } else if (type == Digiface && !double_speed) {
	channels = 26;
	channel_map = channel_map_df_ss;
	dest_map = dest_map_df_ss;
	meter_map = channel_map_df_ss;
	lineouts = 2;
    } else if (type == Digiface && double_speed) {
	channels = 14;
	channel_map = channel_map_ds;
	dest_map = dest_map_ds;
	meter_map = meter_map_ds;
	lineouts = 2;
    } else if (type == H9652 && !double_speed) {
	channels = 26;
	channel_map = channel_map_df_ss;
	dest_map = dest_map_h9652_ss;
	meter_map = channel_map_df_ss;
	lineouts = 0;
    } else if (type == H9652 && double_speed) {
	channels = 14;
	channel_map = channel_map_ds;
	dest_map = dest_map_h9652_ds;
	meter_map = meter_map_ds;
	lineouts = 0;
    } 
    window_width = (channels+2)*STRIP_WIDTH;
    window_height = FULLSTRIP_HEIGHT*2+SMALLSTRIP_HEIGHT+MENU_HEIGHT;
} 

void HDSPMixerCard::setMode(int mode)
{
    double_speed = mode;
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
    for (int i = channels; i < channels+lineouts; ++i) {
	basew->outputs->strips[i]->show();
    }
    if (type != H9652) basew->outputs->empty->hide();
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

