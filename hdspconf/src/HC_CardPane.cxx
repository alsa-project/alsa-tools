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
#include "HC_CardPane.h"

HC_CardPane::HC_CardPane(int alsa_idx, int idx, int t):Fl_Group(PANE_X, PANE_Y, PANE_W, PANE_H)
{
    alsa_index = alsa_idx;
    index = idx;
    type = t;
    snprintf(name, 7, "Card %d", index+1);
    label(name);
    labelsize(10);

    sync_ref = new HC_PrefSyncRef(x()+6, y()+20, 112, 120);
    sync_check = new HC_SyncCheck(x()+6, y()+156, 112, 100);

    spdif_in = new HC_SpdifIn(x()+124, y()+20, 112, 60);
    spdif_out = new HC_SpdifOut(x()+124, y()+96, 112, 80);
    spdif_freq = new HC_SpdifFreq(x()+124, y()+192, 112, 20);

    clock_source = new HC_ClockSource(x()+242, y()+20, 112, 140);
    autosync_ref = new HC_AutoSyncRef(x()+242, y()+176, 112, 40);
    system_clock = new HC_SystemClock(x()+242, y()+232, 112, 40);

    end();
}

