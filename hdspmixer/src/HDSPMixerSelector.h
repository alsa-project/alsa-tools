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

#pragma interface
#ifndef HDSPMixerSelector_H
#define HDSPMixerSelector_H

#include <stdio.h>
#include <FL/Fl.H>
#include <FL/Fl_Menu_.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/fl_draw.H>
#include <sound/hdsp.h>
#include "HDSPMixerWindow.h"
#include "HDSPMixerIOMixer.h"
#include "defines.h"

class HDSPMixerWindow;
class HDSPMixerIOMixer;

static char *destinations_mf_ss[10] = {
    "AN 1+2", "AN 3+4", "AN 5+6", "AN 7+8",
    "A 1+2", "A 3+4", "A 5+6", "A 7+8",
    "SPDIF", "Analog"
};

static char *destinations_mf_ds[8] = {
    "AN 1+2", "AN 3+4", "AN 5+6", "AN 7+8",
    "A 1+2", "A 3+4",
    "SPDIF", "Analog"
};

static char *destinations_df_ss[14] = {
    "A1 1+2", "A1 3+4", "A1 5+6", "A1 7+8",
    "A2 1+2", "A2 3+4", "A2 5+6", "A2 7+8",
    "A3 1+2", "A3 3+4", "A3 5+6", "A3 7+8",
    "SPDIF", "Analog"
};

static char *destinations_df_ds[8] = {
    "A1 1+2", "A1 3+4",
    "A2 1+2", "A2 3+4",
    "A3 1+2", "A3 3+4",
    "SPDIF", "Analog"
};

static char *destinations_h9652_ss[13] = {
    "A1 1+2", "A1 3+4", "A1 5+6", "A1 7+8",
    "A2 1+2", "A2 3+4", "A2 5+6", "A2 7+8",
    "A3 1+2", "A3 3+4", "A3 5+6", "A3 7+8",
    "SPDIF"
};

static char *destinations_h9652_ds[7] = {
    "A1 1+2", "A1 3+4",
    "A2 1+2", "A2 3+4",
    "A3 1+2", "A3 3+4",
    "SPDIF"
};

static char *destinations_h9632_ss[8] = {
    "A 1+2", "A 3+4", "A 5+6", "A 7+8",
    "SPDIF", "AN 1+2", "AN 3+4", "AN 5+6"
};

static char *destinations_h9632_ds[6] = {
    "A 1+2", "A 3+4",
    "SPDIF", "AN 1+2", "AN 3+4", "AN 5+6"    
};

static char *destinations_h9632_qs[4] = {
    "SPDIF", "AN 1+2", "AN 3+4", "AN 5+6"    
};

class HDSPMixerSelector:public Fl_Menu_
{
private:
    char **destinations;
    HDSPMixerWindow *basew;
public:
    int max_dest;
    int selected;
    HDSPMixerSelector(int x, int y, int w, int h);
    void draw();
    int handle(int e);
    void select(int element);
    void setLabels();
};

#endif

