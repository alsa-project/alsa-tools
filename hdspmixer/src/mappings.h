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

#ifndef mappings_H
#define mappings_H

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
    1, 3, 5, 7, 8, 9, 10, 11, 12, 13, 14, 15
};

static char meter_map_h9632_ds[12] = {
    0, 1, 2, 3, 8, 9, 10, 11, 12, 13, 14, 15
};

static char channel_map_h9632_qs[8] = {
    8, 9, 10, 11, 12, 13, 14, 15
};

#endif

