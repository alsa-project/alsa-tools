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
#include "HDSPMixerLoopback.h"

HDSPMixerLoopback::HDSPMixerLoopback(int x, int y, int idx):Fl_Widget(x, y, 34, 15)
{
    basew = (HDSPMixerWindow *)window();
    index = idx;
}

void HDSPMixerLoopback::draw()
{
    if (_loopback == 1)
	fl_draw_pixmap(loopback_xpm, x(), y());
}

int HDSPMixerLoopback::get()
{
    auto const card { basew->cards[basew->current_card] };

    if (card->supportsLoopback() != 0)
	return -1;

    if (index >= card->max_channels)
	return -1;

    int err;
    snd_ctl_elem_value_t *elemval;
    snd_ctl_elem_id_t * elemid;
    snd_ctl_t *handle;
    snd_ctl_elem_value_alloca(&elemval);
    snd_ctl_elem_id_alloca(&elemid);
    char const * const name = basew->cards[basew->current_card]->name;
    if ((err = snd_ctl_open(&handle, name, SND_CTL_NONBLOCK)) < 0) {
	fprintf(stderr, "Error accessing ctl interface on card %s\n.", name);
	return -1; 
    }
    
    snd_ctl_elem_id_set_name(elemid, "Output Loopback");
    snd_ctl_elem_id_set_interface(elemid, SND_CTL_ELEM_IFACE_HWDEP);
    snd_ctl_elem_id_set_index(elemid, index);
    snd_ctl_elem_value_set_id(elemval, elemid);
    if ((err = snd_ctl_elem_read(handle, elemval)) < 0)
	fprintf(stderr, "cannot read loopback: %d\n", err);
    else
	_loopback = snd_ctl_elem_value_get_integer(elemval, 0);

    snd_ctl_close(handle);

    return _loopback;
}

void HDSPMixerLoopback::set(int l)
{
    auto const card { basew->cards[basew->current_card] };

    if (card->supportsLoopback() != 0)
	return;

    if (index >= card->max_channels)
	return;

    if (l != _loopback) {
	int err;
	
	snd_ctl_elem_id_t *id;
	snd_ctl_elem_value_t *ctl;
	snd_ctl_t *handle;

	snd_ctl_elem_value_alloca(&ctl);
	snd_ctl_elem_id_alloca(&id);
	snd_ctl_elem_id_set_name(id, "Output Loopback");
	snd_ctl_elem_id_set_interface(id, SND_CTL_ELEM_IFACE_HWDEP);
	snd_ctl_elem_id_set_device(id, 0);
	snd_ctl_elem_id_set_index(id, index);
	snd_ctl_elem_value_set_id(ctl, id);

	if ((err = snd_ctl_open(
			&handle, basew->cards[basew->current_card]->name, SND_CTL_NONBLOCK)) < 0) {
	    fprintf(stderr, "Alsa error 1: %s\n", snd_strerror(err));
	    return;
	}

	snd_ctl_elem_value_set_integer(ctl, 0, l);
	if ((err = snd_ctl_elem_write(handle, ctl)) < 0) {
	    fprintf(stderr, "Alsa error 2: %s\n", snd_strerror(err));
	    snd_ctl_close(handle);
	    return;
	}

	_loopback = l;
	
	snd_ctl_close(handle);

	redraw();
    }
}

int HDSPMixerLoopback::handle(int e)
{
    int button3 = Fl::event_button3();
    switch (e) {
	case FL_PUSH:
	    set(!_loopback);
	    if (button3)
		relative->set(_loopback);
	    basew->checkState();
	    redraw();
	    return 1;
	default:
	    return Fl_Widget::handle(e);
    }	 
}

