/*
 *
 *  downmix.c
 *    
 *	Copyright (C) Aaron Holtzman - Sept 1999
 *
 *	Originally based on code by Yuqing Deng.
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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "ac3.h"
#include "ac3_internal.h"


#include "decode.h"
#include "downmix.h"
#include "debug.h"


//Pre-scaled downmix coefficients
static float cmixlev_lut[4] = { 0.2928, 0.2468, 0.2071, 0.2468 };
static float smixlev_lut[4] = { 0.2928, 0.2071, 0.0   , 0.2071 };

static void 
downmix_3f_2r_to_2ch(bsi_t* bsi, stream_samples_t samples,sint_16 *s16_samples)
{
	uint_32 j;
	float right_tmp;
	float left_tmp;
	float clev,slev;
	float *centre = 0, *left = 0, *right = 0, *left_sur = 0, *right_sur = 0;

	left      = samples[0];
	centre    = samples[1];
	right     = samples[2];
	left_sur  = samples[3];
	right_sur = samples[4];

	clev = cmixlev_lut[bsi->cmixlev];
	slev = smixlev_lut[bsi->surmixlev];

	for (j = 0; j < 256; j++) 
	{
		left_tmp = 0.4142f * *left++ + clev * *centre   + slev * *left_sur++;
		right_tmp= 0.4142f * *right++ + clev * *centre++ + slev * *right_sur++;

		s16_samples[j * 2 ]    = (sint_16) (left_tmp  * 32767.0f);
		s16_samples[j * 2 + 1] = (sint_16) (right_tmp * 32767.0f);
		// printf("[0] = %.6f, [1] = %.6f\n", left_tmp, right_tmp);
	}
}

static void 
downmix_3f_2r_to_4ch(bsi_t* bsi, stream_samples_t samples,sint_16 *s16_samples)
{
	uint_32 j;
	float right_tmp, left_tmp, rear_right_tmp, rear_left_tmp;
	float clev,slev;
	float *centre = 0, *left = 0, *right = 0, *left_sur = 0, *right_sur = 0;

	left      = samples[0];
	centre    = samples[1];
	right     = samples[2];
	left_sur  = samples[3];
	right_sur = samples[4];

	clev = cmixlev_lut[bsi->cmixlev];
	slev = smixlev_lut[bsi->surmixlev];

	for (j = 0; j < 256; j++) 
	{
		left_tmp = 0.4142f * *left++  + clev * *centre;
		right_tmp= 0.4142f * *right++ + clev * *centre++;
		rear_left_tmp = 0.4142f * *left_sur++;
		rear_right_tmp = 0.4142f * *right_sur++;

		s16_samples[j * 4 ]    = (sint_16) (left_tmp  * 32767.0f);
		s16_samples[j * 4 + 1] = (sint_16) (right_tmp * 32767.0f);
		s16_samples[j * 4 + 2] = (sint_16) (rear_left_tmp  * 32767.0f);
		s16_samples[j * 4 + 3] = (sint_16) (rear_right_tmp * 32767.0f);
#if 0
		printf("[0] = %.6f, [1] = %.6f, [2] = %.6f, [3] = %.6f\n",
				left_tmp, right_tmp, rear_left_tmp, rear_right_tmp);
#endif
	}
}

static void 
downmix_3f_2r_to_6ch(bsi_t* bsi, stream_samples_t samples,sint_16 *s16_samples)
{
	uint_32 j;
	float right_tmp, centre_tmp, left_tmp, rear_right_tmp, rear_left_tmp, lfe_tmp;
	float clev,slev;
	float *centre = 0, *left = 0, *right = 0, *left_sur = 0, *right_sur = 0, *lfe = 0;

	left      = samples[0];
	centre    = samples[1];
	right     = samples[2];
	left_sur  = samples[3];
	right_sur = samples[4];
	lfe	  = samples[5];

	clev = cmixlev_lut[bsi->cmixlev];
	slev = smixlev_lut[bsi->surmixlev];

	for (j = 0; j < 256; j++) 
	{
		left_tmp = 0.4142f * *left++;
		right_tmp= 0.4142f * *right++;
		centre_tmp = 0.4142f * *centre++;
		rear_left_tmp = 0.4142f * *left_sur++;
		rear_right_tmp = 0.4142f * *right_sur++;
		lfe_tmp = bsi->lfeon ? 0.4142f * *lfe++ : (float)0.0;

		s16_samples[j * 6 ]    = (sint_16) (left_tmp  * 32767.0f);
		s16_samples[j * 6 + 1] = (sint_16) (right_tmp * 32767.0f);
		s16_samples[j * 6 + 2] = (sint_16) (rear_left_tmp * 32767.0f);
		s16_samples[j * 6 + 3] = (sint_16) (rear_right_tmp * 32767.0f);
		s16_samples[j * 6 + 4] = (sint_16) (centre_tmp  * 32767.0f);
		s16_samples[j * 6 + 5] = (sint_16) (lfe_tmp * 32767.0f);
#if 0
		printf("[0] = %.6f, [1] = %.6f, [2] = %.6f, [3] = %.6f, [4] = %.6f, [5] = %.6f\n",
				left_tmp, right_tmp, rear_left_tmp, rear_right_tmp,
				centre_tmp, lfe_tmp);
#endif
	}
}

static void
downmix_2f_2r_to_2ch(bsi_t* bsi, stream_samples_t samples,sint_16 *s16_samples)
{
	uint_32 j;
	float right_tmp;
	float left_tmp;
	float slev;
	float *left = 0, *right = 0, *left_sur = 0, *right_sur = 0;

	left      = samples[0];
	right     = samples[1];
	left_sur  = samples[2];
	right_sur = samples[3];

	slev = smixlev_lut[bsi->surmixlev];

	for (j = 0; j < 256; j++) 
	{
		left_tmp = 0.4142f * *left++  + slev * *left_sur++;
		right_tmp= 0.4142f * *right++ + slev * *right_sur++;

		s16_samples[j * 2 ]    = (sint_16) (left_tmp  * 32767.0f);
		s16_samples[j * 2 + 1] = (sint_16) (right_tmp * 32767.0f);
	}
}

static void
downmix_3f_1r_to_2ch(bsi_t* bsi, stream_samples_t samples,sint_16 *s16_samples)
{
	uint_32 j;
	float right_tmp;
	float left_tmp;
	float clev,slev;
	float *centre = 0, *left = 0, *right = 0, *sur = 0;

	left      = samples[0];
	centre    = samples[1];
	right     = samples[2];
	//Mono surround
	sur = samples[3];

	clev = cmixlev_lut[bsi->cmixlev];
	slev = smixlev_lut[bsi->surmixlev];

	for (j = 0; j < 256; j++) 
	{
		left_tmp = 0.4142f * *left++  + clev * *centre++ + slev * *sur;
		right_tmp= 0.4142f * *right++ + clev * *centre   + slev * *sur++;

		s16_samples[j * 2 ]    = (sint_16) (left_tmp  * 32767.0f);
		s16_samples[j * 2 + 1] = (sint_16) (right_tmp * 32767.0f);
	}
}


static void
downmix_2f_1r_to_2ch(bsi_t* bsi, stream_samples_t samples,sint_16 *s16_samples)
{
	uint_32 j;
	float right_tmp;
	float left_tmp;
	float slev;
	float *left = 0, *right = 0, *sur = 0;

	left      = samples[0];
	right     = samples[1];
	//Mono surround
	sur = samples[2];

	slev = smixlev_lut[bsi->surmixlev];

	for (j = 0; j < 256; j++) 
	{
		left_tmp = 0.4142f * *left++  + slev * *sur;
		right_tmp= 0.4142f * *right++ + slev * *sur++;

		s16_samples[j * 2 ]    = (sint_16) (left_tmp  * 32767.0f);
		s16_samples[j * 2 + 1] = (sint_16) (right_tmp * 32767.0f);
	}
}

static void
downmix_3f_0r_to_2ch(bsi_t* bsi, stream_samples_t samples,sint_16 *s16_samples)
{
	uint_32 j;
	float right_tmp;
	float left_tmp;
	float clev;
	float *centre = 0, *left = 0, *right = 0;

	left      = samples[0];
	centre    = samples[1];
	right     = samples[2];

	clev = cmixlev_lut[bsi->cmixlev];

	for (j = 0; j < 256; j++) 
	{
		left_tmp = 0.4142f * *left++  + clev * *centre; 
		right_tmp= 0.4142f * *right++ + clev * *centre++;   

		s16_samples[j * 2 ]    = (sint_16) (left_tmp  * 32767.0f);
		s16_samples[j * 2 + 1] = (sint_16) (right_tmp * 32767.0f);
	}
}

static void
downmix_2f_0r_to_6ch(bsi_t* bsi, stream_samples_t samples,sint_16 *s16_samples)
{
	uint_32 j;
	float *left = 0, *right = 0;

	left      = samples[0];
	right     = samples[1];

	for (j = 0; j < 256; j++) 
	{
		s16_samples[j * 6 ]    = (sint_16) (*left++  * 32767.0f);
		s16_samples[j * 6 + 1] = (sint_16) (*right++ * 32767.0f);
	} //FIXME enable output on surround channels, too.
}

static void
downmix_2f_0r_to_2ch(bsi_t* bsi, stream_samples_t samples,sint_16 *s16_samples)
{
	uint_32 j;
	float *left = 0, *right = 0;

	left      = samples[0];
	right     = samples[1];

	for (j = 0; j < 256; j++) 
	{
		s16_samples[j * 2 ]    = (sint_16) (*left++  * 32767.0f);
		s16_samples[j * 2 + 1] = (sint_16) (*right++ * 32767.0f);
	}
}

static void
downmix_1f_0r_to_2ch(float *centre,sint_16 *s16_samples)
{
	uint_32 j;
	float tmp;

	//Mono program!

	for (j = 0; j < 256; j++) 
	{
		tmp =  32767.0f * 0.7071f * *centre++;

		s16_samples[j * 2 ] = s16_samples[j * 2 + 1] = (sint_16) tmp;
	}
}

//
// Downmix into 2 or 4 channels  (4 ch isn't in quite yet)
//
// The downmix function names have the following format
//
// downmix_Xf_Yr_to_[2|4|6]ch[_dolby]
//
// where X        = number of front channels
//       Y        = number of rear channels
//       [2|4]    = number of output channels
//       [_dolby] = with or without dolby surround mix
//

void downmix(bsi_t* bsi, stream_samples_t samples,sint_16 *s16_samples)
{
	if(bsi->acmod > 7)
		dprintf("(downmix) invalid acmod number\n"); 

	//
	//There are two main cases, with or without Dolby Surround
	//
	if(ac3_config.flags & AC3_DOLBY_SURR_ENABLE)
	{
		fprintf(stderr,"Dolby Surround Mixes not currently enabled\n");
		exit(1);
	}

	//Non-Dolby surround downmixes
	switch(bsi->acmod)
	{
		// 3/2
		case 7:
			switch (ac3_config.num_output_ch) {
			case 2:
				downmix_3f_2r_to_2ch(bsi,samples,s16_samples);
				break;
			case 4:
				downmix_3f_2r_to_4ch(bsi,samples,s16_samples);
				break;
			case 6:
				downmix_3f_2r_to_6ch(bsi,samples,s16_samples);
				break;
			default:
				fprintf(stderr,"unsupported 3/2 channels %d\n", ac3_config.num_output_ch);
				exit(1);
			}
		break;

		// 2/2
		case 6:
			if (ac3_config.num_output_ch != 2) {
				fprintf(stderr,"unsupported 2/2 channels %d\n", ac3_config.num_output_ch);
				exit(1);
			}
			downmix_2f_2r_to_2ch(bsi,samples,s16_samples);
		break;

		// 3/1
		case 5:
			if (ac3_config.num_output_ch != 2) {
				fprintf(stderr,"unsupported 3/1 channels %d\n", ac3_config.num_output_ch);
				exit(1);
			}
			downmix_3f_1r_to_2ch(bsi,samples,s16_samples);
		break;

		// 2/1
		case 4:
			if (ac3_config.num_output_ch != 2) {
				fprintf(stderr,"unsupported 2/1 channels %d\n", ac3_config.num_output_ch);
				exit(1);
			}
			downmix_2f_1r_to_2ch(bsi,samples,s16_samples);
		break;

		// 3/0
		case 3:
			if (ac3_config.num_output_ch != 2) {
				fprintf(stderr,"unsupported 3/0 channels %d\n", ac3_config.num_output_ch);
				exit(1);
			}
			downmix_3f_0r_to_2ch(bsi,samples,s16_samples);
		break;

		// 2/0 - 2f_0r_to_6ch not really, but allows -D pcm.surround51:1 with 2/0 and 3/2 input (VDR e.g.)
		case 2:
			switch (ac3_config.num_output_ch) {
			case 2:
				downmix_2f_0r_to_2ch(bsi,samples,s16_samples);
				break;
			case 6:
				downmix_2f_0r_to_6ch(bsi,samples,s16_samples);
				break;
			default:
				fprintf(stderr,"unsupported 2/0 channels %d\n", ac3_config.num_output_ch);
				exit(1);
			}
		break;

		// 1/0
		case 1:
			if (ac3_config.num_output_ch != 2) {
				fprintf(stderr,"unsupported 1/0 channels %d\n", ac3_config.num_output_ch);
				exit(1);
			}
			downmix_1f_0r_to_2ch(samples[0],s16_samples);
		break;

		// 1+1
		case 0:
			if (ac3_config.num_output_ch != 2) {
				fprintf(stderr,"unsupported 1/1 channels %d\n", ac3_config.num_output_ch);
				exit(1);
			}
			downmix_1f_0r_to_2ch(samples[ac3_config.dual_mono_ch_sel],s16_samples);
		break;
	}
}



#if 0 

	//the dolby mixes lay here for the time being
	switch(bsi->acmod)
	{
		// 3/2
		case 7:
			left      = samples[0];
			centre    = samples[1];
			right     = samples[2];
			left_sur  = samples[3];
			right_sur = samples[4];

			for (j = 0; j < 256; j++) 
			{
				right_tmp = 0.2265f * *left_sur++ + 0.2265f * *right_sur++;
				left_tmp  = -1 * right_tmp;
				right_tmp += 0.3204f * *right++ + 0.2265f * *centre;
				left_tmp  += 0.3204f * *left++  + 0.2265f * *centre++;

				samples[1][j] = right_tmp;
				samples[0][j] = left_tmp;
			}

		break;

		// 2/2
		case 6:
			left      = samples[0];
			right     = samples[1];
			left_sur  = samples[2];
			right_sur = samples[3];

			for (j = 0; j < 256; j++) 
			{
				right_tmp = 0.2265f * *left_sur++ + 0.2265f * *right_sur++;
				left_tmp  = -1 * right_tmp;
				right_tmp += 0.3204f * *right++;
				left_tmp  += 0.3204f * *left++ ;

				samples[1][j] = right_tmp;
				samples[0][j] = left_tmp;
			}
		break;

		// 3/1
		case 5:
			left      = samples[0];
			centre    = samples[1];
			right     = samples[2];
			//Mono surround
			right_sur = samples[3];

			for (j = 0; j < 256; j++) 
			{
				right_tmp =  0.2265f * *right_sur++;
				left_tmp  = -1 * right_tmp;
				right_tmp += 0.3204f * *right++ + 0.2265f * *centre;
				left_tmp  += 0.3204f * *left++  + 0.2265f * *centre++;

				samples[1][j] = right_tmp;
				samples[0][j] = left_tmp;
			}
		break;

		// 2/1
		case 4:
			left      = samples[0];
			right     = samples[1];
			//Mono surround
			right_sur = samples[2];

			for (j = 0; j < 256; j++) 
			{
				right_tmp =  0.2265f * *right_sur++;
				left_tmp  = -1 * right_tmp;
				right_tmp += 0.3204f * *right++; 
				left_tmp  += 0.3204f * *left++;

				samples[1][j] = right_tmp;
				samples[0][j] = left_tmp;
			}
		break;

		// 3/0
		case 3:
			left      = samples[0];
			centre    = samples[1];
			right     = samples[2];

			for (j = 0; j < 256; j++) 
			{
				right_tmp = 0.3204f * *right++ + 0.2265f * *centre;
				left_tmp  = 0.3204f * *left++  + 0.2265f * *centre++;

				samples[1][j] = right_tmp;
				samples[0][j] = left_tmp;
			}
		break;

		// 2/0
		case 2:
		//Do nothing!
		break;

		// 1/0
		case 1:
			//Mono program!
			right = samples[0];

			for (j = 0; j < 256; j++) 
			{
				right_tmp = 0.7071f * *right++;

				samples[1][j] = right_tmp;
				samples[0][j] = right_tmp;
			}
			
		break;

		// 1+1
		case 0:
			//Dual mono, output selected by user
			right = samples[ac3_config.dual_mono_ch_sel];

			for (j = 0; j < 256; j++) 
			{
				right_tmp = 0.7071f * *right++;

				samples[1][j] = right_tmp;
				samples[0][j] = right_tmp;
			}
		break;
#endif
