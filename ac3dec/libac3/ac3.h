/* 
 *    ac3.h
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
 *
 */

#ifndef AARONS_TYPES
#define AARONS_TYPES
typedef unsigned long long uint_64;
typedef unsigned int   uint_32;
typedef unsigned short uint_16;
typedef unsigned char  uint_8;

typedef signed long long sint_64;
typedef signed int     sint_32;
typedef signed short   sint_16;
typedef signed char    sint_8;
#endif

#define AC3_DOLBY_SURR_ENABLE 0x1
#define AC3_3DNOW_ENABLE      0x2
#define AC3_MMX_ENABLE        0x4
#define AC3_ALTIVEC_ENABLE    0x8

typedef struct ac3_config_s
{
	//Bit flags that enable various things
	uint_32 flags;
	//Callback that points the decoder to new stream data
  void   (*fill_buffer_callback)(uint_8 **, uint_8 **);
	//Number of discrete channels in final output (for downmixing)
	uint_16 num_output_ch;
	//Which channel of a dual mono stream to select
	uint_16 dual_mono_ch_sel;
} ac3_config_t;

typedef struct ac3_frame_s
{
	uint_32 sampling_rate;
	sint_16 *audio_data;
} ac3_frame_t;

void ac3_init(ac3_config_t *config);
ac3_frame_t* ac3_decode_frame(void);
