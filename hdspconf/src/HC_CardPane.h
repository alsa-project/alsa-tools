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

#pragma interface
#ifndef HC_CardPane_H
#define HC_CardPane_H

#include <stdio.h>
#include <FL/Fl_Group.H>
#include "HC_SyncCheck.h"
#include "HC_SpdifFreq.h"
#include "HC_AutoSyncRef.h"
#include "HC_SystemClock.h"
#include "HC_ClockSource.h"
#include "HC_SpdifIn.h"
#include "HC_SpdifOut.h"
#include "HC_PrefSyncRef.h"
#include "defines.h"

class HC_SyncCheck;
class HC_SpdifFreq;
class HC_AutoSyncRef;
class HC_SystemClock;
class HC_ClockSource;
class HC_SpdifIn;
class HC_SpdifOut;
class HC_PrefSyncRef;

class HC_CardPane:public Fl_Group
{
public:
    HC_CardPane(int alsa_idx, int idx, int t);    
    HC_SyncCheck *sync_check;
    HC_SpdifFreq *spdif_freq;
    HC_AutoSyncRef *autosync_ref;
    HC_SystemClock *system_clock;
    HC_ClockSource *clock_source;
    HC_SpdifIn *spdif_in;
    HC_SpdifOut *spdif_out;
    HC_PrefSyncRef *sync_ref;
    int index;
    int alsa_index;
    int type;
private:
    char name[7];
};

#endif

