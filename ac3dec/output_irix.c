/*
 *
 *  output_irix.c
 *    
 *      Copyright (C) Aaron Holtzman - May 1999
 *      Port to IRIX by Jim Miller, SGI - Nov 1999
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
 *
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>

#include <audio.h>


typedef signed short sint_16;
typedef unsigned int uint_32;
#include "output.h"

static int init = 0;
static ALport alport = 0;
static ALconfig alconfig = 0;
static int bytesPerWord = 1;
static int nChannels = 2;


/*
 * open the audio device for writing to
 */
int output_open(int bits, int rate, int channels)
{
  ALpv params[2];
  int  dev = AL_DEFAULT_OUTPUT;
  int  wsize = AL_SAMPLE_16;

  nChannels = channels;

  if (!init) {
    init = 1;
    alconfig = alNewConfig();

    if (alSetQueueSize(alconfig, BUFFER_SIZE) < 0) {
        fprintf(stderr, "alSetQueueSize failed: %s\n",
                alGetErrorString(oserror()));
        return 0;
    }

    if (alSetChannels(alconfig, channels) < 0) {
        fprintf(stderr, "alSetChannels(%d) failed: %s\n",
                channels, alGetErrorString(oserror()));
        return 0;
    }

    if (alSetDevice(alconfig, dev) < 0) {
        fprintf(stderr, "alSetDevice failed: %s\n",
                        alGetErrorString(oserror()));
        return 0;
    }

    if (alSetSampFmt(alconfig, AL_SAMPFMT_TWOSCOMP) < 0) {
        fprintf(stderr, "alSetSampFmt failed: %s\n",
                        alGetErrorString(oserror()));
        return 0;
    }

    alport = alOpenPort("AC3Decode", "w", 0);
    if (!alport) {
        fprintf(stderr, "alOpenPort failed: %s\n",
                        alGetErrorString(oserror()));
        return 0;
    }

    switch (bits) {
        case 8:         
                bytesPerWord = 1;
                wsize = AL_SAMPLE_8;
                break;
        case 16: 
                bytesPerWord = 2;
                wsize = AL_SAMPLE_16;
                break;
        case 24:
                bytesPerWord = 4;
                wsize = AL_SAMPLE_24;
                break;
        default:
                printf("Irix audio: unsupported bit with %d\n", bits);
                break;
    }

    if (alSetWidth(alconfig, wsize) < 0) {
        fprintf(stderr, "alSetWidth failed: %s\n", alGetErrorString(oserror()));
        return 0;
    }
        
    params[0].param = AL_RATE;
    params[0].value.ll = alDoubleToFixed((double)rate);
    params[1].param = AL_MASTER_CLOCK;
    params[1].value.i = AL_CRYSTAL_MCLK_TYPE;
    if ( alSetParams(dev, params, 1) < 0) {
        printf("alSetParams() failed: %s\n", alGetErrorString(oserror()));
        return 0;
    }
  }

	printf("I've synced the IRIX code with the mainline blindly.\n Let me know if it works.\n");

  return 1;
}

/*
 * play the sample to the already opened file descriptor
 */

void output_play(sint_16* output_samples, uint_32 num_bytes)
{
	alWriteFrames(alport, output_samples, 6 * 256); 
}

void
output_close(void)
{
  alClosePort(alport);
  alFreeConfig(alconfig);
  alport = 0;
  alconfig = 0;
  init = 0;
}
