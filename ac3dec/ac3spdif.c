/***************************************************************************/
/*  This code has been fully inspired from various place                   */
/*  I will give their name later. May they be bless .... It works   	   */
/*                                                                         */
/*	For the moment test it.                                            */   
/*									   */
/* 27-08-00 -- Ze'ev Maor -- fixed recovery from flase syncword detection  */
/* 								           */
/* 24-08-00 -- Ze'ev Maor -- Modified for integrtion with DXR3-OMS-plugin  */  
/***************************************************************************/
									   
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>

#include <libac3/ac3.h>
#include <libac3/ac3_internal.h>
#include <libac3/parse.h>
#include <libac3/crc.h>

#include "output.h"

void swab(const void*, void*, size_t);

#define BLOCK_SIZE 6144

static unsigned char buf[BLOCK_SIZE];
static uint32_t sbuffer_size;
static syncinfo_t syncinfo;
static char *sbuffer;
static int done_banner;

static uint32_t
buffer_syncframe(syncinfo_t *syncinfo, uint8_t **start, uint8_t *end)
{
  uint8_t *cur = *start;
  uint16_t syncword = syncinfo->syncword;
  uint32_t ret = 0;
 
  //
  // Find an ac3 sync frame.
  //
  while(syncword != 0x0b77)
    {
      if(cur >= end)
	goto done;
      syncword = (syncword << 8) + *cur++;
    }
 
  //need the next 3 bytes to decide how big the frame is
  while(sbuffer_size < 3)
    {
      if(cur >= end)
	goto done;
 
      sbuffer[sbuffer_size++] = *cur++;
    }
                                                                                    
  parse_syncinfo_data(syncinfo,sbuffer);
 
  while(sbuffer_size < syncinfo->frame_size * 2 - 2)
    {
      if(cur >= end)
	goto done;
 
      sbuffer[sbuffer_size++] = *cur++;
    }
 
  crc_init();
  crc_process_frame(sbuffer,syncinfo->frame_size * 2 - 2);
	
  if(!crc_validate())
    {
      //error_flag = 1;
      fprintf(stderr,"** CRC failed - skipping frame **\n");
      syncword = 0xffff;
      sbuffer_size = 0;
      bzero(buf,BLOCK_SIZE);

      goto done;
    }                                                                           
  //
  //if we got to this point, we found a valid ac3 frame to decode
  //
 
  //reset the syncword for next time
  syncword = 0xffff;
  sbuffer_size = 0;
  ret = 1;
 
 done:
  syncinfo->syncword = syncword;
  *start = cur;
  return ret;
}                                                                                  

void
init_spdif(void)
{
  sbuffer_size = 0;
  sbuffer = &buf[10];
  done_banner = 0;
}

int
output_spdif_zero(int frames)
{
  int res;

  buf[0] = 0x72; buf[1] = 0xf8;  // spdif syncword
  buf[2] = 0x1f; buf[3] = 0x4e;  // ..............
  buf[4] = 0x00;		 // null frame (no data) 
  buf[5] = 7 << 5;		 // stream = 7
  buf[6] = 0x00; buf[7] = 0x00;  // frame size
  memset(&buf[8], 0, BLOCK_SIZE - 8);
  while (frames-- > 0) {
    res = output_play((short *)buf, BLOCK_SIZE / 2 / 2);	/* 2 channels, 16-bit samples */
    if (res < 0)
      return res;
  }
  return 0;
}

int
output_spdif_leadin(void)
{
  memset(buf, 0, 8);
  return output_play((short *)buf, 8);
}

int
output_spdif(uint8_t *data_start, uint8_t *data_end, int quiet)
{
  int ret = 0, res;
  
  while(buffer_syncframe(&syncinfo, &data_start, data_end))
    {
      buf[0] = 0x72; buf[1] = 0xf8;	// spdif syncword
      buf[2] = 0x1f; buf[3] = 0x4e;	// ..............
      buf[4] = 0x01;			// AC3 data
      buf[5] = buf[13] & 7;		// bsmod, stream = 0
      buf[6] = (syncinfo.frame_size * 16) & 0xff;
      buf[7] = ((syncinfo.frame_size * 16) >> 8) & 0xff;
      buf[8] = 0x77; buf[9] = 0x0b;	// AC3 syncwork
      
      if (!done_banner && !quiet) {
        fprintf(stdout,"AC3 Stream ");
	fprintf(stdout,"%2.1f KHz",syncinfo.sampling_rate * 1e-3);
        fprintf(stdout,"%4d kbps",syncinfo.bit_rate);
        fprintf(stdout,"\n");
        done_banner = 1;
      }
      
#ifndef _a_b_c_d_e_f /* WORDS_BIGENDIAN */
      // extract_ac3 seems to write swabbed data
      swab(&buf[10], &buf[10], syncinfo.frame_size * 2 - 2);
#endif
      res = output_play((short *)buf, BLOCK_SIZE / 2 / 2);	/* 2 channels, 16-bit samples */
      ret = ret < 0 ? ret : res;
      bzero(buf,BLOCK_SIZE);
    }
  return ret;
}
