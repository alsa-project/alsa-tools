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
#include "HDSPMixerOutput.h"

static char *labels_mf_ss[20] = {
    "AN 1", "AN 2", "AN 3", "AN 4", "AN 5", "AN 6", "AN 7", "AN 8",
    "A 1", "A 2", "A 3", "A 4", "A 5", "A 6", "A 7", "A 8",
    "SP.L", "SP.R", "AN.L", "AN.R"
};

static char *labels_mf_ds[16] = {
    "AN 1", "AN 2", "AN 3", "AN 4", "AN 5", "AN 6", "AN 7", "AN 8",
    "A 1", "A 2", "A 3", "A 4",
    "SP.L", "SP.R", "AN.L", "AN.R"
};

static char *labels_df_ss[28] = {
    "A1 1", "A1 2", "A1 3", "A1 4", "A1 5", "A1 6", "A1 7", "A1 8",
    "A2 1", "A2 2", "A2 3", "A2 4", "A2 5", "A2 6", "A2 7", "A2 8",
    "A3 1", "A3 2", "A3 3", "A3 4", "A3 5", "A3 6", "A3 7", "A3 8",
    "SP.L", "SP.R", "AN.L", "AN.R"
};

static char *labels_df_ds[16] = {
    "A1 1", "A1 2", "A1 3", "A1 4",
    "A2 1", "A2 2", "A2 3", "A2 4",
    "A3 1", "A3 2", "A3 3", "A3 4",
    "SP.L", "SP.R", "AN.L", "AN.R"
};

static char *labels_9632_ss[16] = {
    "A 1", "A 2", "A 3", "A 4", "A 5", "A 6", "A 7", "A 8",
    "SP.L", "SP.R", "AN 1", "AN 2", "AN 3", "AN 4", "AN 5", "AN 6"
};

static char *labels_9632_ds[12] = {
    "A 1", "A 2", "A 3", "A 4",
    "SP.L", "SP.R", "AN 1", "AN 2", "AN 3", "AN 4", "AN 5", "AN 6"
};

static char *labels_9632_qs[8] = {
    "SP.L", "SP.R", "AN 1", "AN 2", "AN 3", "AN 4", "AN 5", "AN 6"
};

HDSPMixerOutput::HDSPMixerOutput(int x, int y, int w, int h, int num):Fl_Group(x, y, w, h)
{

    for (int j = 0; j < 3; ++j) {
	for (int i = 0; i < 8; ++i) {
	    data[j][0][i] = new HDSPMixerOutputData();
	    data[j][1][i] = new HDSPMixerOutputData();
	    data[j][2][i] = new HDSPMixerOutputData();
	}
    }
    if (num%2) {
	p_output_xpm = output_r_xpm;
    } else {
	p_output_xpm = output_xpm;
    }
    basew = (HDSPMixerWindow *)window();
    setLabels();
    out_num = num;
    fader = new HDSPMixerFader(x+4, y+19, 65535.0, out_num+1, 2);
    peak = new HDSPMixerPeak(x+3, y+4, 0);
    gain = new HDSPMixerGain(x+3, y+175, 0);
    meter = new HDSPMixerMeter(x+20, y+27, false, peak);
    end();
}

void HDSPMixerOutput::setLabels()
{
    HDSP_IO_Type type = basew->cards[basew->current_card]->type;
    int sm = basew->cards[basew->current_card]->speed_mode;
    if (type == H9632) {
	switch (sm) {
	case 0:
	    labels = labels_9632_ss;
	    break;
	case 1:
	    labels = labels_9632_ds;
	    break;	
	case 2:
	    labels = labels_9632_qs;
	    break;
	}
    } else if (type == Multiface) {
	if (sm)
	    labels = labels_mf_ds;
	else 
	    labels = labels_mf_ss;
    } else {
	if (sm)
	    labels = labels_df_ds;
	else
	    labels = labels_df_ss;
    }
}

void HDSPMixerOutput::draw_background()
{
    draw_background(x(), y(), w(), h());
}

void HDSPMixerOutput::draw_background(int xpos, int ypos, int w, int h)
{
    fl_push_clip(xpos, ypos, w, h);
    fl_draw_pixmap(p_output_xpm, x(), y());
    fl_pop_clip();    
}

void HDSPMixerOutput::draw()
{
    Fl_Widget*const* a = array();
    if (damage() & ~FL_DAMAGE_CHILD) {
	draw_background();
	fl_color(FL_BLACK);
	fl_font(FL_HELVETICA, 8);
	fl_draw(labels[out_num], x()+4, y()+193, 27, 9, FL_ALIGN_CENTER);
	for (int i=children(); i--;) {
	    Fl_Widget& o = **a++;
	    draw_child(o);
	}
    } else {
	for (int i=children(); i--;) update_child(**a++);
    }
}

void HDSPMixerOutput::update_child(Fl_Widget& widget) {
    if (widget.damage() && widget.visible() && widget.type() < FL_WINDOW && fl_not_clipped(widget.x(), widget.y(), widget.w(), widget.h())) {
	if ((HDSPMixerMeter *)&widget == meter) {
	    ((HDSPMixerMeter *)&widget)->fine_draw = 1;
	} else {
	    draw_background(widget.x(), widget.y(), widget.w(), widget.h());
	}
	widget.draw();
	widget.clear_damage();
    }
}

