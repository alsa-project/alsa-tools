/*****************************************************************************
   envy24control.c - Env24 chipset (ICE1712) control utility
   midi controller code
   (c) 2004 by Dirk Jagdmann <doj@cubic.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
******************************************************************************/

#include <string.h>
#include <alsa/asoundlib.h>
#include "midi.h"
#include <gtk/gtk.h>
#include <stdint.h>

static snd_seq_t *seq=0;
static int client, clientId, port, ch;
static char *portname=0, *appname=0;
static int nofeedback=-1;
static int maxstreams=0;
static uint8_t currentvalues[128];

void midi_maxstreams(int m)
{
  maxstreams=(m-1)*2;
}

int midi_close()
{
  int i=0;
  if(seq)
    i=snd_seq_close(seq);

  seq=0;
  client=port=0;
  if(portname)
    free(portname), portname=0;
  if(appname)
    free(appname), appname=0;

  return i;
}

static void do_controller(int c, int v)
{
  snd_seq_event_t ev;
  snd_seq_ev_clear(&ev);
  snd_seq_ev_set_source(&ev, port);
  snd_seq_ev_set_subs(&ev);
  snd_seq_ev_set_direct(&ev);
  snd_seq_ev_set_controller(&ev,ch,c,v);
  snd_seq_event_output(seq, &ev);
  snd_seq_drain_output(seq);
}

int midi_controller(int c, int v)
{
  if(c==nofeedback) return 0;
  if(c<0 || c>127) return 0;

  v*=127; v/=96;
  if(v<0) v=0;
  if(v>127) v=127;
  currentvalues[c]=v;
  do_controller(c,v);
  return 0;
}

int midi_init(char *appname, int channel)
{
  snd_seq_client_info_t *clientinfo;
  int npfd;
  struct pollfd *pfd;

  if(seq)
    return 0;

  ch=channel;

  if(snd_seq_open(&seq, "default", SND_SEQ_OPEN_DUPLEX, SND_SEQ_NONBLOCK) < 0)
    {
      g_warning("could not init ALSA sequencer\n");
      seq=0;
      return -1;
    }

  snd_seq_set_client_name(seq, appname);
  snd_seq_client_info_alloca(&clientinfo);
  snd_seq_get_client_info (seq, clientinfo);
  client=snd_seq_client_info_get_client(clientinfo);
  clientId = snd_seq_client_id(seq);

  portname=g_strdup_printf("%s Mixer Control", appname);
  port=snd_seq_create_simple_port(seq, portname,
				  SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE|SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ,
				  SND_SEQ_PORT_TYPE_APPLICATION);
  if(port < 0)
    {
      g_warning("could not create ALSA sequencer port\n");
      midi_close();
      return -1;
    }

  npfd=snd_seq_poll_descriptors_count(seq, POLLIN);
  if(npfd<=0)
    {
      g_warning("could not get number of ALSA sequencer poll descriptors\n");
      midi_close();
      return -1;
    }

  pfd=(struct pollfd*)alloca(npfd * sizeof(struct pollfd));
  if(pfd==0)
    {
      g_warning("could not alloc memory for ALSA sequencer poll descriptors\n");
      midi_close();
      return -1;
    }
  if(snd_seq_poll_descriptors(seq, pfd, npfd, POLLIN) != npfd)
    {
      g_warning("number of returned poll desc is not equal of request poll desc\n");
      midi_close();
      return -1;
    }

  return pfd[0].fd;
}

void mixer_adjust(GtkAdjustment *adj, gpointer data);

void midi_process(gpointer data, gint source, GdkInputCondition condition)
{
  snd_seq_event_t *ev;
  static GtkAdjustment *adj=0;
  if(!adj)
    adj=(GtkAdjustment*) gtk_adjustment_new(0, 0, 96, 1, 1, 10);

  do
    {
      snd_seq_event_input(seq, &ev);
      if(!ev) continue;
      switch(ev->type)
	{
	case SND_SEQ_EVENT_CONTROLLER:
#if 0
	  fprintf(stderr, "Channel %02d: Controller %03d: Value:%d\n",
		  ev->data.control.channel, ev->data.control.param, ev->data.control.value);
#endif
	  if(ev->data.control.channel == ch && ev->data.control.param < maxstreams)
	    {
	      int stream=ev->data.control.param+1;
	      long data=((stream/2)<<16)|(stream&1);
	      int v=ev->data.control.value; v*=96; v/=127;
	      gtk_adjustment_set_value(adj, 96-v);
	      nofeedback=ev->data.control.param;
	      mixer_adjust(adj, (gpointer)data);
	      nofeedback=-1;
	    }
	  break;

	case SND_SEQ_EVENT_PORT_SUBSCRIBED:
#if 0
	  fprintf(stderr, "event subscribed send.client:%i dest.client:%i clientId:%i\n",
		  (int)ev->data.connect.sender.client, (int)ev->data.connect.dest.client, clientId);
#endif
	  if(ev->data.connect.dest.client!=clientId)
	    {
	      int i;
	      for(i=0; i<sizeof(currentvalues); ++i)
		do_controller(i, currentvalues[i]);
	    }
	  break;
	}
      snd_seq_free_event(ev);
    }
  while (snd_seq_event_input_pending(seq, 0) > 0);
}
