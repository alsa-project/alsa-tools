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
#define HDSPMIXER_DEFINE_SELECTOR_LABELS
#include "HDSPMixerSelector.h"

HDSPMixerSelector::HDSPMixerSelector(int x, int y, int w, int h):Fl_Menu_(x, y, w, h)
{
    max_dest = 0;
    selected = 0;
    basew = (HDSPMixerWindow *)window();
    setLabels();
    textfont(FL_HELVETICA);
    textsize(8);
    textcolor(FL_BLACK);
}

void HDSPMixerSelector::draw() {
    fl_color(FL_WHITE);
    fl_font(FL_HELVETICA, 8);
    fl_draw((char *)mvalue()->label(), x(), y(), w(), h(), FL_ALIGN_CENTER);
}

int HDSPMixerSelector::handle(int e) {
    const Fl_Menu_Item *item;
    int xpos = Fl::event_x()-x();
    int ypos = Fl::event_y()-y();
    switch(e) {
	case FL_PUSH:
	    for (int i = 0; i < max_dest; i++) {
		if (((HDSPMixerIOMixer *)parent())->fader->pos[i] != 0) {
		    mode(i, FL_MENU_TOGGLE|FL_MENU_VALUE);
		} else {
		    mode(i, FL_MENU_TOGGLE);
		}
	    }    
	    if ((item = (menu()->popup(x(), y()+h(), 0, 0, this))) != NULL) {
		value(item);
		selected = value();
		if (basew->inputs->buttons->view->submix) {
		    basew->inputs->buttons->view->submix_value = value();
		    for (int i = 0; i < HDSP_MAX_CHANNELS; i++) {
			basew->inputs->strips[i]->targets->value(value());
			basew->inputs->strips[i]->targets->redraw();
			basew->playbacks->strips[i]->targets->value(value());
			basew->playbacks->strips[i]->targets->redraw();
			basew->inputs->strips[i]->fader->dest = value();
			basew->inputs->strips[i]->fader->redraw();
			basew->inputs->strips[i]->fader->sendGain();
			basew->playbacks->strips[i]->fader->dest = value();
			basew->playbacks->strips[i]->fader->redraw();
			basew->playbacks->strips[i]->fader->sendGain();
			basew->inputs->strips[i]->pan->dest = value();
			basew->inputs->strips[i]->pan->redraw();
			basew->playbacks->strips[i]->pan->dest = value();
			basew->playbacks->strips[i]->pan->redraw();
		    }
		} else {
		    ((HDSPMixerIOMixer *)parent())->fader->dest = value();
		    ((HDSPMixerIOMixer *)parent())->fader->redraw();
		    ((HDSPMixerIOMixer *)parent())->pan->dest = value();
		    ((HDSPMixerIOMixer *)parent())->pan->redraw();
		    ((HDSPMixerIOMixer *)parent())->fader->sendGain();
		}
		redraw();
	    }
	    basew->checkState();
	    return 1;
	default:
	    return Fl_Menu_::handle(e);
    }
}

void HDSPMixerSelector::setLabels()
{
    HDSP_IO_Type type;
    hdsp_9632_aeb_t *aeb;
    int sm;
    clear();
    type = basew->cards[basew->current_card]->type;
    aeb = &basew->cards[basew->current_card]->h9632_aeb;
    sm = basew->cards[basew->current_card]->speed_mode;
    if (type == Multiface) {
	switch (sm) {
	case 0:
	    max_dest = 10;
	    destinations = destinations_mf_ss;
	    break;
	case 1:
	    max_dest = 8;
	    destinations = destinations_mf_ds;
	    break;
	case 2:
	    /* should never happen */
	    break;
	}
    } else if (type == Digiface) {
	switch (sm) {
	case 0:
	    max_dest = 14;
	    destinations = destinations_df_ss;
	    break;
	case 1:
	    max_dest = 8;
	    destinations = destinations_df_ds;
	    break;
	case 2:
	    /* should never happen */
	    break;
	}
    } else if (type == H9652) {
	switch (sm) {
	case 0:
	    max_dest = 13;
	    destinations = destinations_h9652_ss;
	    break;
	case 1:
	    max_dest = 7;
	    destinations = destinations_h9652_ds;
	    break;
	case 2:
	    /* should never happen */
	    break;
	}
    } else if (type == H9632) {
	switch (sm) {
	case 0:
	    max_dest = 6 + (aeb->aebo ? 2 : 0);
	    destinations = destinations_h9632_ss;
	    break;
	case 1:
	    max_dest = 4 + (aeb->aebo ? 2 : 0);
	    destinations = destinations_h9632_ds;
	    break;
	case 2:
	    max_dest = 2 + (aeb->aebo ? 2 : 0);
	    destinations = destinations_h9632_qs;
	    break;
	}
    }
    
    for (int i = 0; i < max_dest; ++i) {
	add(destinations[i], 0, 0, 0, FL_MENU_TOGGLE);
    }
    value(0);
}

