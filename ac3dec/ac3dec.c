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
#include <getopt.h>
#include <sys/errno.h>
#include <sys/signal.h>
#include <errno.h>
#include "config.h"

#include "libac3/ac3.h"
#include "output.h"

void
init_spdif(void);
int
output_spdif(uint_8 *data_start, uint_8 *data_end, int quiet);

static int end_flag = 0;
static int quiet = 0;

static void help(void)
{
	printf("Usage: ac3dec <options> [file] [[file]] ...\n");
	printf("\nAvailable options:\n");
	printf("  -h,--help         this help\n");
	printf("  -v,--version      print version of this program\n");
	printf("  -D,--device=NAME  select PCM by NAME\n");
	printf("  -4,--4ch	    four channels mode\n");
	printf("  -6,--6ch	    six channels mode\n");
	printf("  -C,--iec958c      raw IEC958 (S/PDIF) consumer mode\n");
	printf("  -P,--iec958p      raw IEC958 (S/PDIF) professional mode\n");
	printf("  -q,--quit         quit mode\n");
}

#define CHUNK_SIZE 2047
uint_8 buf[CHUNK_SIZE];
FILE *in_file;
 
ssize_t fill_buffer(uint_8 **start,uint_8 **end)
{
	uint_32 bytes_read;

	*start = buf;

	bytes_read = fread(*start,1,CHUNK_SIZE,in_file);

	if (feof(in_file) || ferror(in_file) || bytes_read < CHUNK_SIZE) {
		end_flag = 1;
		return EOF;
	}

	*end = *start + bytes_read;
	return bytes_read;
}

static void ac3dec_signal_handler(int signal)
{
	if (!quiet)
		fprintf(stderr, "Aborted...\n");
	// it's important to close the PCM handle(s), because
	// some driver settings have to be recovered
	output_close();
	fclose(in_file);
	exit(EXIT_FAILURE);
}

int main(int argc,char *argv[])
{
	struct option long_option[] =
	{
		{"help", 0, NULL, 'h'},
		{"version", 0, NULL, 'v'},
		{"device", 1, NULL, 'D'},
		{"4ch", 0, NULL, '4'},
		{"6ch", 0, NULL, '6'},
		{"iec958c", 0, NULL, 'C'},
		{"spdif", 0, NULL, 'C'},
		{"iec958p", 0, NULL, 'P'},
		{"quit", 0, NULL, 'q'},
		{NULL, 0, NULL, 0},
	};
	ac3_config_t ac3_config;
	output_t out_config;
	int morehelp, loop = 0;

	bzero(&ac3_config, sizeof(ac3_config));
	ac3_config.fill_buffer_callback = fill_buffer;
	ac3_config.num_output_ch = 2;
	ac3_config.flags = 0;
	bzero(&out_config, sizeof(out_config));
	out_config.pcm_name = NULL;
	out_config.bits = 16;
	out_config.rate = 48000;
	out_config.channels = 2;
	out_config.spdif = SPDIF_NONE;

	morehelp = 0;
	while (1) {
		int c;

		if ((c = getopt_long(argc, argv, "hvD:46CPq", long_option, NULL)) < 0)
			break;
		switch (c) {
		case 'h':
			morehelp++;
			break;
		case 'v':
			printf("ac3dec version " VERSION "\n");
			return 1;
		case 'D':
			out_config.pcm_name = optarg;
			break;
		case '4':
			if (out_config.spdif == SPDIF_NONE)
				ac3_config.num_output_ch = 4;
			break;
		case '6':
			if (out_config.spdif == SPDIF_NONE)
				ac3_config.num_output_ch = 6;
			break;
		case 'C':
			ac3_config.num_output_ch = 2;
			out_config.spdif = SPDIF_CON;
			break;
		case 'P':
			ac3_config.num_output_ch = 2;
			out_config.spdif = SPDIF_PRO;
			break;
		case 'q':
			ac3_config.flags |= AC3_QUIET;
			out_config.quiet = 1;
			quiet = 1;
			break;
		default:
			fprintf(stderr, "\07Invalid switch or option needs an argument.\n");
			morehelp++;
		}
	}
	out_config.channels = ac3_config.num_output_ch;
	if (morehelp) {
		help();
		return 1;
	}

	while (1) {
		if (argc - optind <= 0) {
			if (loop || end_flag)
				break;
			in_file = stdin;
		} else {
			in_file = fopen(argv[optind],"r");	
			if(!in_file) {
				fprintf(stderr,"%s - Couldn't open file %s\n",strerror(errno),argv[optind]);
				exit(EXIT_FAILURE);
			}
			optind++;
			loop++;
		}
		if (out_config.spdif == SPDIF_NONE) {
			ac3_frame_t *ac3_frame;
			ac3_init(&ac3_config);
			ac3_frame = ac3_decode_frame();
			if (output_open(&out_config) < 0) {
				fprintf(stderr, "Output open failed\n");
				exit(EXIT_FAILURE);
			}
			signal(SIGINT, ac3dec_signal_handler);
			signal(SIGTERM, ac3dec_signal_handler);
			signal(SIGABRT, ac3dec_signal_handler);
			do {
				//Send the samples to the output device 
				output_play(ac3_frame->audio_data, 256 * 6);
			} while((ac3_frame = ac3_decode_frame()));
		} else {
			uint_8 *start, *end;
			init_spdif();
			if (output_open(&out_config) < 0) {
				fprintf(stderr, "Output open failed\n");
				exit(EXIT_FAILURE);
			}
			signal(SIGINT, ac3dec_signal_handler);
			signal(SIGTERM, ac3dec_signal_handler);
			signal(SIGABRT, ac3dec_signal_handler);
			while (fill_buffer(&start, &end) > 0)
				if (output_spdif(start, end, quiet) < 0)
					break;
		}
		output_close();
		fclose(in_file);
	}
	return EXIT_SUCCESS;
}
