/*
 *   HDSPConf
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
#include "HC_PrefSyncRef.h"

void pref_sync_ref_cb(Fl_Widget *w, void *arg)
{
    int ref, err;
    char card_name[6];
    snd_ctl_elem_value_t *ctl;
    snd_ctl_elem_id_t *id;
    snd_ctl_t *handle;
    HC_PrefSyncRef *psr = (HC_PrefSyncRef *)arg;
    HC_CardPane *pane = (HC_CardPane *)(psr->parent());
    Fl_Round_Button *source = (Fl_Round_Button *)w;
    if (source == psr->word_clock) {
	ref = 0;
    } else if (source == psr->adat_sync) {
	ref = 1;
    } else if (source == psr->spdif) {
	ref = 2;
    } else if (source == psr->adat1) {
	ref = 3;
    } else if (source == psr->adat2) {
	ref = 4;
    } else if (source == psr->adat3) {
	ref = 5;
    }
    snprintf(card_name, 6, "hw:%i", pane->alsa_index);
    snd_ctl_elem_value_alloca(&ctl);
    snd_ctl_elem_id_alloca(&id);
    snd_ctl_elem_id_set_name(id, "Preferred Sync Reference");
    snd_ctl_elem_id_set_numid(id, 0);
    snd_ctl_elem_id_set_interface(id, SND_CTL_ELEM_IFACE_HWDEP);
    snd_ctl_elem_id_set_device(id, 0);
    snd_ctl_elem_id_set_subdevice(id, 0);
    snd_ctl_elem_id_set_index(id, 0);
    snd_ctl_elem_value_set_id(ctl, id);
    snd_ctl_elem_value_set_enumerated(ctl, 0, ref);
    if ((err = snd_ctl_open(&handle, card_name, SND_CTL_NONBLOCK)) < 0) {
	fprintf(stderr, "Error opening ctl interface on card %s\n", card_name); 
	return;
    }
    if ((err = snd_ctl_elem_write(handle, ctl)) < 0) {
	fprintf(stderr, "Error accessing ctl interface on card %s\n", card_name); 
	return;
    }
    snd_ctl_close(handle);
}


HC_PrefSyncRef::HC_PrefSyncRef(int x, int y, int w, int h):Fl_Group(x, y, w, h, "Pref. Sync Ref")
{
	int i = 0;
	int v_step;
	if (((HC_CardPane *)parent())->type == MULTIFACE) {
	    v_step = (int)(h/4.0f);
	} else {
	    v_step = (int)(h/6.0f);
	}
	source = 0;
	box(FL_ENGRAVED_FRAME);;
	label("Pref. Sync Ref");
	labelsize(10);
	align(FL_ALIGN_TOP|FL_ALIGN_LEFT);
	word_clock = new Fl_Round_Button(x+15, y+v_step*i++, w-30, v_step, "Word Clock");
	word_clock->callback(pref_sync_ref_cb, (void *)this);
	adat_sync = new Fl_Round_Button(x+15, y+v_step*i++, w-30, v_step, "ADAT Sync");
	adat_sync->callback(pref_sync_ref_cb, (void *)this);
	spdif = new Fl_Round_Button(x+15, y+v_step*i++, w-30, v_step, "SPDIF In");
	spdif->callback(pref_sync_ref_cb, (void *)this);
	adat1 = new Fl_Round_Button(x+15, y+v_step*i++, w-30, v_step, "ADAT1 In");
	adat1->callback(pref_sync_ref_cb, (void *)this);
	if (((HC_CardPane *)parent())->type != MULTIFACE) {
	    adat2 = new Fl_Round_Button(x+15, y+v_step*i++, w-30, v_step, "ADAT2 In");
	    adat2->labelsize(10);
	    adat2->type(FL_RADIO_BUTTON);
	    adat2->callback(pref_sync_ref_cb, (void *)this);
	    adat3 = new Fl_Round_Button(x+15, y+v_step*i++, w-30, v_step, "ADAT3 In");
	    adat3->labelsize(10);
	    adat3->type(FL_RADIO_BUTTON);
	    adat3->callback(pref_sync_ref_cb, (void *)this);
	}	
	adat1->labelsize(10);
	adat1->type(FL_RADIO_BUTTON);
	spdif->labelsize(10);
	spdif->type(FL_RADIO_BUTTON);
	word_clock->labelsize(10);
	word_clock->type(FL_RADIO_BUTTON);
	adat_sync->labelsize(10);
	adat_sync->type(FL_RADIO_BUTTON);
	end();	
}

void HC_PrefSyncRef::setRef(int r)
{
	if (r >= 0 && r < children()) {
	    if (((Fl_Round_Button *)child(r))->value() != 1)
		((Fl_Round_Button *)child(r))->setonly();
	}
}

