/* 
 *  bitstream.h
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

//My new and improved vego-matic endian swapping routine
//(stolen from the kernel)
#ifdef WORDS_BIGENDIAN

#	define swab32(x) (x)

#else

#	if defined (__i386__)

#	define swab32(x) __i386_swab32(x)
	static inline const uint_32 __i386_swab32(uint_32 x)
	{
		__asm__("bswap %0" : "=r" (x) : "0" (x));
		return x;
	}

#	else

#	define swab32(x)\
((((uint_8*)&x)[0] << 24) | (((uint_8*)&x)[1] << 16) |  \
 (((uint_8*)&x)[2] << 8)  | (((uint_8*)&x)[3]))

#	endif
#endif

extern uint_32 bits_left;
extern uint_32 current_word;

void bitstream_init(ssize_t(*fill_function)(uint_8**,uint_8**));

uint_8 bitstream_get_byte(void);

uint_8 *bitstream_get_buffer_start(void);
void bitstream_buffer_frame(uint_32 frame_size);

uint_32 bitstream_get_bh(uint_32 num_bits);

static inline uint_32 
bitstream_get(uint_32 num_bits)
{
	uint_32 result;
	
	if(num_bits < bits_left)
	{
		result = (current_word << (32 - bits_left)) >> (32 - num_bits);
		bits_left -= num_bits;
		return result;
	}

	return bitstream_get_bh(num_bits);
}

