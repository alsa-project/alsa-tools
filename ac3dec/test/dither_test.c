/* 
 *  dither_test.c
 *
 *	Aaron Holtzman - May 1999
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include "ac3.h"
#include "ac3_internal.h"
#include "dither.h"

#include <sys/time.h>
#include <unistd.h>

int main(void)
{
	int i,j,foo;
	struct timeval start,end;

	/*
	for(i=0;i < 65538 ;i++)
		//printf("%04x\n",dither_gen());
		printf("%f\n",((sint_16)dither_gen())/ 32768.0);
	printf("\n");
	*/



	for(j=0;j < 10 ;j++)
	{
		gettimeofday(&start,0);
		for(i=0;i < 10000 ;i++)
		{
			foo = dither_gen();
		}
		gettimeofday(&end,0);
		printf("%f us\n",((end.tv_sec - start.tv_sec) * 1000000 + 
				(end.tv_usec - start.tv_usec))/10000.0);
	}

}
