/*
 *
 *  output.h
 *
 *  Based on original code by Angus Mackay (amackay@gus.ml.org)
 *
 *	Copyright (C) Aaron Holtzman - May 1999
 *
 *  This file is part of ac3dec, a free Dolby AC-3 stream decoder.
 *	
 *  ac3dec is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  ac3dec is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *
 */

typedef struct {
	const char *pcm_name;
	int bits;
	int rate;
	int channels;
	int spdif: 1;
	int quiet: 1;
} output_t;

int output_open(output_t *output);
int output_play(sint_16* output_samples, uint_32 num_bytes);
void output_close(void);
