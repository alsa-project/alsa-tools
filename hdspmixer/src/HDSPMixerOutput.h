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
#ifndef HDSPMixerOutput_H
#define HDSPMixerOutput_H

#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include <FL/fl_draw.H>
#include <sound/hdsp.h>
#include "HDSPMixerFader.h"
#include "HDSPMixerPeak.h"
#include "HDSPMixerGain.h"
#include "HDSPMixerMeter.h"
#include "HDSPMixerOutputData.h"
#include "HDSPMixerWindow.h"
#include "pixmaps.h"

class HDSPMixerFader;
class HDSPMixerGain;
class HDSPMixerPeak;
class HDSPMixerMeter;
class HDSPMixerOutputData;
class HDSPMixerWindow;

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

class HDSPMixerOutput:public Fl_Group
{
private:
    int out_num;
    char **labels;
    HDSPMixerPeak *peak;
    HDSPMixerWindow *basew;    
    void update_child(Fl_Widget& widget);
public:
    HDSPMixerOutputData *data[3][2][8]; /* data[card][mode(ds/ss)][preset number] */
    HDSPMixerFader *fader;
    HDSPMixerGain *gain;
    HDSPMixerMeter *meter;
    HDSPMixerOutput(int x, int y, int w, int h, int out);
    void draw();
    void draw_background();
    void draw_background(int x, int y, int w, int h);
    void setLabels();
};

#endif

