/* 
 *  bitstream.c
 *
 *	Copyright (C) Aaron Holtzman - Dec 1999
 *
 *  This file is part of ac3dec, a free AC-3 audio decoder
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ac3.h"
#include "ac3_internal.h"
#include "bitstream.h"

#define BUFFER_SIZE 4096

static uint_8 buffer[BUFFER_SIZE];

static uint_8 *buffer_start, *buffer_end;
static uint_8 *chunk_start, *chunk_end;

uint_32 bits_left;
uint_32 current_word;

ssize_t (*bitstream_fill_buffer)(uint_8**,uint_8**);

int bitstream_get_byte(void)
{
	if(chunk_start == chunk_end)
		if (bitstream_fill_buffer(&chunk_start,&chunk_end) <= 0)
			return EOF;

	return (*chunk_start++);
}

uint_8 *bitstream_get_buffer_start(void)
{
	return buffer_start;
}

int
bitstream_buffer_frame(uint_32 frame_size)
{
  uint_32 bytes_read;
  uint_32 num_bytes;

  bytes_read = 0;

  do
  {
    if(chunk_start > chunk_end)
			printf("argh!\n");
    if(chunk_start == chunk_end)
      if (bitstream_fill_buffer(&chunk_start,&chunk_end) <= 0)
      	return EOF;

    num_bytes = chunk_end - chunk_start;

    if(bytes_read + num_bytes > frame_size)
      num_bytes = frame_size - bytes_read;

    memcpy(&buffer[bytes_read], chunk_start, num_bytes);

    bytes_read += num_bytes;
    chunk_start += num_bytes;
  }
  while (bytes_read != frame_size);

  buffer_start = buffer;
  buffer_end   = buffer + frame_size;

  bits_left = 0;
  return 0;
}


static inline void
bitstream_fill_current()
{
	current_word = *((uint_32*)buffer_start)++;
	current_word = swab32(current_word);
}

//
// The fast paths for _get is in the
// bitstream.h header file so it can be inlined.
//
// The "bottom half" of this routine is suffixed _bh
//
// -ah
//

uint_32
bitstream_get_bh(uint_32 num_bits)
{
	uint_32 result;

	num_bits -= bits_left;
	result = (current_word << (32 - bits_left)) >> (32 - bits_left);

	bitstream_fill_current();

	if(num_bits != 0)
		result = (result << num_bits) | (current_word >> (32 - num_bits));
	
	bits_left = 32 - num_bits;

	return result;
}

void
bitstream_init(ssize_t(*fill_function)(uint_8**,uint_8**))
{
	// Setup the buffer fill callback 
	bitstream_fill_buffer = fill_function;
}
