/* 
 *  imdct_test.c
 *
 *	Aaron Holtzman - May 1999
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include "ac3.h"
#include "ac3_internal.h"
#include "imdct.h"

static stream_samples_t samples;
static bsi_t bsi;
static audblk_t audblk;

int main(void)
{
	int i;

	samples[0][20] = 0.4;
	samples[0][40] = 0.4;
	samples[0][30] = 1.0;


	imdct_init();
	bsi.nfchans = 1;

	imdct(&bsi,&audblk,samples);


	for(i=0;i<256;i++)
		printf("%1.8f\n",samples[0][i]);
	
	return 0;

}
