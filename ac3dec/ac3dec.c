/* 
 *   ac3dec.c
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <errno.h>

#include "libac3/ac3.h"
#include "output.h"

#define CHUNK_SIZE 2047
uint_8 buf[CHUNK_SIZE];
FILE *in_file;
 
void fill_buffer(uint_8 **start,uint_8 **end)
{
	uint_32 bytes_read;

	*start = buf;

	bytes_read = fread(*start,1,CHUNK_SIZE,in_file);

	//FIXME hack...
	if(bytes_read < CHUNK_SIZE)
		exit(0);

	*end= *start + bytes_read;
}

int main(int argc,char *argv[])
{
	ac3_frame_t *ac3_frame;
	ac3_config_t ac3_config;
	int idx, channels = 2;


	/* If we get an argument then use it as a filename... otherwise use
	 * stdin */
	idx = 1;
	if (idx < argc && !strcmp(argv[idx], "-4")) {
		channels = 4; idx++;
	} else if (idx < argc && !strcmp(argv[idx], "-6")) {
		channels = 6; idx++;
	}
	if (idx < argc && argv[idx] != NULL) {	
		in_file = fopen(argv[idx],"r");	
		if(!in_file)
		{
			fprintf(stderr,"%s - Couldn't open file %s\n",strerror(errno),argv[1]);
			exit(1);
		}
	}
	else
		in_file = stdin;

	ac3_config.fill_buffer_callback = fill_buffer;
	ac3_config.num_output_ch = channels;
	ac3_config.flags = 0;

	ac3_init(&ac3_config);
	
	ac3_frame = ac3_decode_frame();
	output_open(16,ac3_frame->sampling_rate,channels);

	do
	{
		//Send the samples to the output device 
		output_play(ac3_frame->audio_data, 256 * 6);
	}
	while((ac3_frame = ac3_decode_frame()));

	output_close();
	fclose(in_file);
	return 0;
}
