/*
 *  extract_ac3.c
 *
 *  Copyright (C) Aaron Holtzman <aholtzma@ess.engr.uvic.ca> - June 1999
 *
 *  Extracts an AC-3 audio stream from an MPEG-2 system stream
 *  and writes it to stdout
 *
 *  Ideas and bitstream syntax info borrowed from code written 
 *  by Nathan Laredo <laredo@gnu.org>
 *
 *  Multiple track support by Yuqing Deng <deng@tinker.chem.brown.edu>
 * 
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
 *  the Free Software Foundation, 
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/mman.h>


/* Audio track to play */
static unsigned char track_code = 0x80;
static unsigned char track_table[8] = 
{ 
	0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87 
};

#define BUFSIZE 512 /* needs to be as big as biggest header */
static int vobf;
static unsigned char buf[BUFSIZE];
static unsigned char *cur_pos;
static unsigned char *end_pos;

void file_init(char file_name[])
{
	if(file_name[0] == '-' && file_name[1] == '\0')
	{
	   vobf = STDIN_FILENO;
	}
	else if ((vobf = open(file_name, O_RDONLY)) < 0)
	{
		fprintf(stderr,"File not found\n");
		exit(1);
	}
	cur_pos = buf;
	end_pos = buf;
}

inline void increment_position(long x)
{
	if(cur_pos + x <= end_pos)
	{
	   cur_pos += x;
	   if(cur_pos == end_pos)
	   {
		cur_pos = buf;
                end_pos = buf;
	   }
	}
	else
	{
           long size = 0;
           x -= (long)(end_pos - cur_pos);
#ifdef SEEK_PIPES
		if(lseek(vobf, x, SEEK_CUR) < 0)
		{
		   fprintf(stderr, "Error: unexpected end of stream\n");
		   exit(1);
		}
#else
		while(x)
		{
		   size = (x > BUFSIZE) ? BUFSIZE : x;
		   if(read(vobf, buf, size) < size)
		   {
		      fprintf(stderr, "Error: unexpected end of stream\n");
		      exit(1);
		   }
                   x-=size;
		}
#endif
		cur_pos = buf;
                end_pos = buf;
	}
}

inline static void load_next_bytes(long count)
{
   if(cur_pos + count <= end_pos)
      return;
   if(cur_pos + count > buf + BUFSIZE - 1 )
   {
      printf ("No buffer space to read %ld bytes\n", count);
      exit(1);
   }

   count -= (long)(end_pos - cur_pos);
   if(read(vobf, end_pos, count) < count)
   {
      fprintf(stderr, "Error: unexpected end of stream\n");
      exit(1);
   }
   end_pos += count;
}

inline int next_24_bits(long x)
{
   load_next_bytes(3);
	if (cur_pos[0] != ((x >> 16) & 0xff))
		return 0;
	if (cur_pos[1] != ((x >>  8) & 0xff))
		return 0;
	if (cur_pos[2] != ((x      ) & 0xff))
		return 0;

	return 1;
}

inline int next_32_bits(long x)
{
   load_next_bytes(4);
	if (cur_pos[0] != ((x >> 24) & 0xff))
		return 0;
	if (cur_pos[1] != ((x >> 16) & 0xff))
		return 0;
	if (cur_pos[2] != ((x >>  8) & 0xff))
		return 0;
	if (cur_pos[3] != ((x      ) & 0xff))
		return 0;

	return 1;
}

void read_write_next_bytes(long count, int outfd)
{
   long size;
   size = (long)(end_pos - cur_pos);
   if(size > count)
   {
      write(outfd, cur_pos, count);
      cur_pos +=count;
      if(cur_pos == end_pos)
      {
	 cur_pos = buf;
	 end_pos = buf;
      }
      return;
   }
   else if(size > 0)
   {
      write(outfd, cur_pos, size);
   }

   while(count)
   {
      size = (count > BUFSIZE) ? BUFSIZE : count;
      if(read(vobf, buf, size) < size ||
	 write(outfd, buf, size) < size)
      {
	 fprintf(stderr, "Error: unexpected end of stream\n");
      }
      count -= size;
   }
   cur_pos = buf;
   end_pos = buf;
}

void parse_pes(void)
{
	unsigned long data_length;
	unsigned long header_length;

	load_next_bytes(9);

	//The header length is the PES_header_data_length byte plus 6 for the packet
	//start code and packet size, 3 for the PES_header_data_length and two
	//misc bytes, and finally 4 bytes for the mystery AC3 packet tag 
	header_length = cur_pos[8] + 6 + 3 + 4 ;
	data_length =(cur_pos[4]<<8) + cur_pos[5];


	//If we have AC-3 audio then output it
	if(cur_pos[3] == 0xbd)
	{
		load_next_bytes(header_length);
#if 0
		//Debugging printfs
		fprintf(stderr,"start of pes curpos[] = %02x%02x%02x%02x\n",
			cur_pos[0],cur_pos[1],cur_pos[2],cur_pos[3]);
		fprintf(stderr,"header_length = %d data_length = %x\n",
			header_length, data_length);
		fprintf(stderr,"extra crap 0x%02x%02x%02x%02x data size 0x%0lx\n",cur_pos[header_length-4],
			cur_pos[header_length-3],cur_pos[header_length-2],cur_pos[header_length-1],data_length);
#endif

		//Only extract the track we want
		if((cur_pos[header_length-4] == track_code )) 
		{
		   increment_position(header_length);
			read_write_next_bytes(data_length - header_length + 6, STDOUT_FILENO);

		}
		else
		{
	       increment_position(data_length + 6);
		}
	}
	else
	{
	   //The packet size is data_length plus 6 bytes to account for the
	   //packet start code and the data_length itself.
	   increment_position(data_length + 6);
	}
}

void parse_pack(void)
{
	unsigned long skip_length;

	// Deal with the pack header 
	// The first 13 bytes are junk. The fourteenth byte 
	// contains the number of stuff bytes 
	load_next_bytes(14);
	skip_length = cur_pos[13] & 0x7;
	increment_position(14 + skip_length);

	// Deal with the system header if it exists 
	if(next_32_bits(0x000001bb))
	{
	// Bytes 5 and 6 contain the length of the header minus 6
                load_next_bytes(6);
		skip_length = (cur_pos[4] << 8) +  cur_pos[5];
		increment_position(6 + skip_length);
	}

	while(next_24_bits(0x000001) && !next_32_bits(0x000001ba))
	{
		parse_pes();
	}
}

int main(int argc, char *argv[])
{
	int track = 0;

	if (argc < 2) {
		fprintf(stderr, "usage: %s mpeg_stream [track number]\n", argv[0]);
		exit(1);
	}

	if (argc == 3) 
	{
		track = strtol(argv[2], NULL, 0);
		fprintf(stderr,"Extracting track %d\n",track);
	}

	if (track < 0 || track > 7) 
	{
		fprintf(stderr, "Invalid track number: %d\n", track);
		exit(1);
	}

	track_code = track_table[track];



	file_init(argv[1]);

	if(!next_32_bits(0x000001ba))
	{
		fprintf(stderr, "Non-program streams not handled - exiting\n\n");
		exit(1);
	}

	do
	{
		parse_pack();
	} 
	while(next_32_bits(0x000001ba));

	fprintf(stderr,"curpos[] = %x%x%x%x\n",cur_pos[0],cur_pos[1],cur_pos[2],cur_pos[3]);

	if(!next_32_bits(0x000001b9))
	{
		fprintf(stderr, "Error: expected end of stream code\n");
		exit(1);
	}

	if(vobf != STDIN_FILENO) close(vobf);
	return 0;
}		
