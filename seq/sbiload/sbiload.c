/*
 *  ALSA sequencer SBI FM instrument loader
 *  Copyright (c) 2000 Uros Bizjak <uros@kss-loka.si>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#include <errno.h>
#include <getopt.h>
#include <alsa/sound/ainstr_fm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <alsa/asoundlib.h>
#include <sys/time.h>
#include <unistd.h>

/*
 * Load a SBI FM instrument patch
 *	sbiload [-p client:port] [-4] [-l] [-v level] instfile drumfile
 *
 *	-p client:port  - An ALSA client and port number to load instrument to
 *      -4              - four operators file type
 *	-l              - List possible output ports that could be used
 *	-v level        - Verbose level
 */

typedef struct sbi_header
{
  char key[4];
  char name[32];
}
sbi_header_t;

typedef struct sbi_inst
{
  sbi_header_t header;
#define DATA_LEN_2OP	16
#define DATA_LEN_4OP	24
  char data[DATA_LEN_4OP];
}
sbi_inst_t;

/* offsets for SBI params */
#define AM_VIB		0
#define KSL_LEVEL	2
#define ATTACK_DECAY	4
#define SUSTAIN_RELEASE	6
#define WAVE_SELECT	8

/* offset for SBI instrument */
#define CONNECTION	10
#define OFFSET_4OP	11

/* offsets in sbi_header.name for SBI extensions */
#define ECHO_DELAY	25
#define ECHO_ATTEN	26
#define CHORUS_SPREAD	27
#define TRNSPS		28
#define FIX_DUR		29
#define MODES		30
#define FIX_KEY		31

/* Options for the command */
#define HAS_ARG 1
static struct option long_opts[] = {
  {"port", HAS_ARG, NULL, 'p'},
  {"opl3", 0, NULL, '4'},
  {"list", 0, NULL, 'l'},
  {"verbose", HAS_ARG, NULL, 'v'},
  {"version", 0, NULL, 'V'},
  {0, 0, 0, 0},
};

/* Number of elements in an array */
#define NELEM(a) ( sizeof(a)/sizeof((a)[0]) )

#define ADDR_PARTS 4		/* Number of part in a port description addr 1:2:3:4 */
#define SEP ", \t"		/* Separators for port description */

#define SBI_FILE_TYPE_2OP 0
#define SBI_FILE_TYPE_4OP 1

/* Default file type */
int file_type = SBI_FILE_TYPE_2OP;

/* Default verbose level */
int verbose = 0;

/* Global declarations */
snd_seq_t *seq_handle;

int seq_client;
int seq_port;
int seq_dest_client;
int seq_dest_port;

/* Function prototypes */
static void show_list ();
static void show_usage ();
static void show_op (fm_instrument_t * fm_instr);

static int load_patch (fm_instrument_t * fm_instr, int bank, int prg, char *name);
static int load_file (int bank, char *filename);
static int parse_portdesc (char *portdesc);

static int init_client ();

static void
ignore_errors (const char *file, int line, const char *function, int err, const char *fmt, ...)
{
  /* ignore */
}

/*
 * Show a list of possible output ports that midi could be sent to.
 */
static void
show_list () {
  snd_seq_client_info_t *cinfo;
  snd_seq_port_info_t *pinfo;

  int client, err;

  snd_lib_error_set_handler (ignore_errors);
  if ((err = snd_seq_open (&seq_handle, "hw", SND_SEQ_OPEN_DUPLEX, 0)) < 0) {
      fprintf (stderr, "Could not open sequencer: %s\n", snd_strerror (err));
      return;
  }

  printf (" Port     %-30.30s    %s\n", "Client name", "Port name");
  snd_seq_client_info_alloca(&cinfo);
  snd_seq_client_info_set_client(cinfo, -1);
  while (snd_seq_query_next_client(seq_handle, cinfo) >= 0) {
      client = snd_seq_client_info_get_client(cinfo);
      snd_seq_port_info_alloca(&pinfo);
      snd_seq_port_info_set_client(pinfo, client);
      snd_seq_port_info_set_port(pinfo, -1);
      while (snd_seq_query_next_port(seq_handle, pinfo) >= 0) {
	  unsigned int cap;

	  cap = (SND_SEQ_PORT_CAP_SUBS_WRITE | SND_SEQ_PORT_CAP_WRITE);
	  if ((snd_seq_port_info_get_capability(pinfo) & cap) == cap) {
	      printf ("%3d:%-3d   %-30.30s    %s\n",
		      client, snd_seq_port_info_get_port(pinfo),
		      snd_seq_client_info_get_name(cinfo),
		      snd_seq_port_info_get_name(pinfo));
	  }
      }
  }
  snd_seq_close (seq_handle);
}

/*
 * Show usage message
 */
static void
show_usage () {
  char **cpp;
  static char *msg[] = {
    "Usage: sbiload [-p client:port] [-4] [-l] [-v level] instfile drumfile",
    "",
    "  -p client:port  - A alsa client and port number to send midi to",
    "  -4              - four operators file type (default = two ops)",
    "  -l              - List possible output ports that could be used",
    "  -v level        - Verbose level (default = 0)",
    "  -V              - Show version",
  };

  for (cpp = msg; cpp < msg + NELEM (msg); cpp++) {
      fprintf (stderr, "%s\n", *cpp);
  }
}

/*
 * Show version
 */
static void
show_version () {
  printf("Version: " VERSION "\n");
}

/*
 * Show instrument FM operators
 */
static void
show_op (fm_instrument_t * fm_instr) {
  int i = 0;

  do {
      printf ("  OP%i: flags: %s %s %s %s\011OP%i: flags: %s %s %s %s\n",
	      i,
	      fm_instr->op[i].am_vib & (1 << 7) ? "AM" : "  ",
	      fm_instr->op[i].am_vib & (1 << 6) ? "VIB" : "   ",
	      fm_instr->op[i].am_vib & (1 << 5) ? "EGT" : "   ",
	      fm_instr->op[i].am_vib & (1 << 4) ? "KSR" : "   ",
	      i + 1,
	      fm_instr->op[i + 1].am_vib & (1 << 7) ? "AM" : "  ",
	      fm_instr->op[i + 1].am_vib & (1 << 6) ? "VIB" : "   ",
	      fm_instr->op[i + 1].am_vib & (1 << 5) ? "EGT" : "   ",
	      fm_instr->op[i + 1].am_vib & (1 << 4) ? "KSR" : "");
      printf ("  OP%i: MULT = 0x%x" "\011\011OP%i: MULT = 0x%x\n",
	      i, fm_instr->op[i].am_vib & 0x0f,
	      i + 1, fm_instr->op[i + 1].am_vib & 0x0f);
      printf ("  OP%i: KSL  = 0x%x  TL = 0x%.2x\011OP%i: KSL  = 0x%x  TL = 0x%.2x\n",
	      i, (fm_instr->op[i].ksl_level >> 6) & 0x03, fm_instr->op[i].ksl_level & 0x3f,
	      i + 1, (fm_instr->op[i + 1].ksl_level >> 6) & 0x03, fm_instr->op[i + 1].ksl_level & 0x3f);
      printf ("  OP%i: AR   = 0x%x  DL = 0x%x\011OP%i: AR   = 0x%x  DL = 0x%x\n",
	      i, (fm_instr->op[i].attack_decay >> 4) & 0x0f, fm_instr->op[i].attack_decay & 0x0f,
	      i + 1, (fm_instr->op[i + 1].attack_decay >> 4) & 0x0f, fm_instr->op[i + 1].attack_decay & 0x0f);
      printf ("  OP%i: SL   = 0x%x  RR = 0x%x\011OP%i: SL   = 0x%x  RR = 0x%x\n",
	      i, (fm_instr->op[i].sustain_release >> 4) & 0x0f, fm_instr->op[i].sustain_release & 0x0f,
	      i + 1, (fm_instr->op[i + 1].sustain_release >> 4) & 0x0f, fm_instr->op[i + 1].sustain_release & 0x0f);
      printf ("  OP%i: WS   = 0x%x\011\011OP%i: WS   = 0x%x\n",
	      i, fm_instr->op[i].wave_select & 0x07,
	      i + 1, fm_instr->op[i + 1].wave_select & 0x07);
      printf (" FB = 0x%x,  %s\n",
	      (fm_instr->feedback_connection[i / 2] >> 1) & 0x07,
	      fm_instr->feedback_connection[i / 2] & (1 << 0) ? "parallel" : "serial");
      i += 2;
  }
  while (i == (fm_instr->type == FM_PATCH_OPL3) << 1);

  printf (" Extended data:\n"
	  "  ED = %.3i  EA = %.3i  CS = %.3i  TR = %.3i\n"
	  "  FD = %.3i  MO = %.3i  FK = %.3i\n",
	  fm_instr->echo_delay, fm_instr->echo_atten, fm_instr->chorus_spread, fm_instr->trnsps,
	  fm_instr->fix_dur, fm_instr->modes, fm_instr->fix_key);
}

/*
 * Check the result of the previous instr event
 */
static int
check_result (int evtype) {
  snd_seq_event_t *pev;
  int err;

  for (;;) {
      if ((err = snd_seq_event_input (seq_handle, &pev)) < 0) {
	  fprintf (stderr, "Unable to read event input: %s\n",
		   snd_strerror (err));
	  return -ENXIO;
      }
      if (pev->type == SND_SEQ_EVENT_RESULT && pev->data.result.event == evtype)
	break;
      snd_seq_free_event (pev);
  }
  err = pev->data.result.result;
  snd_seq_free_event (pev);

  return err;
}

/*
 * Send patch to destination port
 */
static int
load_patch (fm_instrument_t * fm_instr, int bank, int prg, char *name) {
  snd_instr_header_t *put;
  snd_seq_instr_t id;
  snd_seq_event_t ev;

  size_t size;
  int err;

  if (verbose > 1) {
    printf ("%.3i: [OPL%i] %s\n", prg, fm_instr->type == FM_PATCH_OPL3 ? 3 : 2, name);
    show_op (fm_instr);
  }

  if ((err = snd_instr_fm_convert_to_stream (fm_instr, name, &put, &size)) < 0) {
    fprintf (stderr, "Unable to convert instrument %.3i to stream: %s\n",
	     prg, snd_strerror (err));
    return -1;
  }
  memset(&id, 0, sizeof(id));
  id.std = SND_SEQ_INSTR_TYPE2_OPL2_3;
  id.prg = prg;
  id.bank = bank;
  snd_instr_header_set_id(put, &id);

  /* build event */
  memset (&ev, 0, sizeof (ev));
  ev.source.client = seq_client;
  ev.source.port = seq_port;
  ev.dest.client = seq_dest_client;
  ev.dest.port = seq_dest_port;

  ev.flags = SND_SEQ_EVENT_LENGTH_VARUSR;
  ev.queue = SND_SEQ_QUEUE_DIRECT;

__again:
  ev.type = SND_SEQ_EVENT_INSTR_PUT;
  ev.data.ext.len = size;
  ev.data.ext.ptr = put;

  if ((err = snd_seq_event_output (seq_handle, &ev)) < 0) {
    fprintf (stderr, "Unable to write an instrument %.3i put event: %s\n",
	     prg, snd_strerror (err));
    return -1;
  }

  if ((err = snd_seq_drain_output (seq_handle)) < 0) {
    fprintf (stderr, "Unable to write instrument %.3i data: %s\n", prg,
	     snd_strerror (err));
    return -1;
  }

  err = check_result (SND_SEQ_EVENT_INSTR_PUT);
  if (err >= 0) {
    if (verbose)
      printf ("Loaded instrument %.3i, bank %.3i: %s\n", prg, bank, name);
    return 0;
  } else if (err == -EBUSY) {
    snd_instr_header_t *remove;

    snd_instr_header_alloca(&remove);
    snd_instr_header_set_cmd(remove, SND_SEQ_INSTR_FREE_CMD_SINGLE);
    snd_instr_header_set_id(remove, snd_instr_header_get_id(put));

    /* remove instrument */
    ev.type = SND_SEQ_EVENT_INSTR_FREE;
    ev.data.ext.len = snd_instr_header_sizeof();
    ev.data.ext.ptr = remove;

    if ((err = snd_seq_event_output (seq_handle, &ev)) < 0) {
      fprintf (stderr, "Unable to write an instrument %.3i free event: %s\n",
	       prg, snd_strerror (err));
      return -1;
    }

    if ((err = snd_seq_drain_output (seq_handle)) < 0) {
      fprintf (stderr, "Unable to write instrument %.3i data: %s\n", prg,
	       snd_strerror (err));
      return -1;
    }

    if ((err = check_result (SND_SEQ_EVENT_INSTR_FREE)) < 0) {
      fprintf (stderr, "Instrument %.3i, bank %.3i - free error: %s\n",
	       prg, bank, snd_strerror (err));
      return -1;
    }
    goto __again;
  }

  fprintf (stderr, "Instrument %.3i, bank %.3i - put error: %s\n",
	   prg, bank, snd_strerror (err));
  return -1;
}

/*
 * Parse standard .sb or .o3 file
 */
static void
load_sb (int bank, int fd) {
  int len, i;
  int prg;

  sbi_inst_t sbi_instr;
  fm_instrument_t fm_instr;
  int fm_instr_type;

  len = (file_type == SBI_FILE_TYPE_4OP) ? DATA_LEN_4OP : DATA_LEN_2OP;
  for (prg = 0;; prg++) {
    if (read (fd, &sbi_instr.header, sizeof (sbi_header_t)) < (ssize_t)sizeof (sbi_header_t))
      break;

    if (!strncmp (sbi_instr.header.key, "SBI\032", 4) || !strncmp (sbi_instr.header.key, "2OP\032", 4)) {
      fm_instr_type = FM_PATCH_OPL2;
    } else if (!strncmp (sbi_instr.header.key, "4OP\032", 4)) {
      fm_instr_type = FM_PATCH_OPL3;
    } else {
      fm_instr_type = 0;
      if (verbose)
	printf ("%.3i: wrong instrument key!\n", prg);
    }

    if (read (fd, &sbi_instr.data, len) < len)
      break;

    if (fm_instr_type == 0)
      continue;

    memset (&fm_instr, 0, sizeof (fm_instr));
    fm_instr.type = fm_instr_type;

    for (i = 0; i < 2; i++) {
      fm_instr.op[i].am_vib = sbi_instr.data[AM_VIB + i];
      fm_instr.op[i].ksl_level = sbi_instr.data[KSL_LEVEL + i];
      fm_instr.op[i].attack_decay = sbi_instr.data[ATTACK_DECAY + i];
      fm_instr.op[i].sustain_release = sbi_instr.data[SUSTAIN_RELEASE + i];
      fm_instr.op[i].wave_select = sbi_instr.data[WAVE_SELECT + i];
    }
    fm_instr.feedback_connection[0] = sbi_instr.data[CONNECTION];

    if (fm_instr_type == FM_PATCH_OPL3) {
      for (i = 0; i < 2; i++) {
	fm_instr.op[i + 2].am_vib = sbi_instr.data[OFFSET_4OP + AM_VIB + i];
	fm_instr.op[i + 2].ksl_level = sbi_instr.data[OFFSET_4OP + KSL_LEVEL + i];
	fm_instr.op[i + 2].attack_decay = sbi_instr.data[OFFSET_4OP + ATTACK_DECAY + i];
	fm_instr.op[i + 2].sustain_release = sbi_instr.data[OFFSET_4OP + SUSTAIN_RELEASE + i];
	fm_instr.op[i + 2].wave_select = sbi_instr.data[OFFSET_4OP + WAVE_SELECT + i];
      }
      fm_instr.feedback_connection[1] = sbi_instr.data[OFFSET_4OP + CONNECTION];
    }

    fm_instr.echo_delay = sbi_instr.header.name[ECHO_DELAY];
    fm_instr.echo_atten = sbi_instr.header.name[ECHO_ATTEN];
    fm_instr.chorus_spread = sbi_instr.header.name[CHORUS_SPREAD];
    fm_instr.trnsps = sbi_instr.header.name[TRNSPS];
    fm_instr.fix_dur = sbi_instr.header.name[FIX_DUR];
    fm_instr.modes = sbi_instr.header.name[MODES];
    fm_instr.fix_key = sbi_instr.header.name[FIX_KEY];

    if (load_patch (&fm_instr, bank, prg, sbi_instr.header.name) < 0)
      break;
  }
  return;
}

/*
 * Load file
 */
static int
load_file (int bank, char *filename) {
  int fd;

  fd = open (filename, O_RDONLY);
  if (fd == -1) {
      perror (filename);
      return -1;
  }

  load_sb(bank, fd);

  close (fd);
  return 0;
}

/*
 * Parse port description
 */
static int
parse_portdesc (char *portdesc) {
  char *astr;
  char *cp;
  int a[ADDR_PARTS];
  int count;

  if (portdesc == NULL)
    return -1;

  for (astr = strtok (portdesc, SEP); astr; astr = strtok (NULL, SEP)) {
    for (cp = astr, count = 0; cp && *cp; cp++) {
      if (count < ADDR_PARTS)
	a[count++] = atoi (cp);
      cp = strchr (cp, ':');
      if (cp == NULL)
	break;
    }

    if (count == 2) {
      seq_dest_client = a[0];
      seq_dest_port = a[1];
    } else {
      printf ("Addresses in %d parts not supported yet\n", count);
      return -1;
    }
  }
  return 0;
}

/*
 * Open sequencer, create client port and
 * subscribe client to destination port
 */
static int
init_client () {
  char name[64];
  snd_seq_port_subscribe_t *sub;
  snd_seq_addr_t addr;
  int err;

  if ((err = snd_seq_open (&seq_handle, "hw", SND_SEQ_OPEN_DUPLEX, 0)) < 0) {
    fprintf (stderr, "Could not open sequencer: %s\n", snd_strerror (err));
    return -1;
  }

  seq_client = snd_seq_client_id (seq_handle);
  if (seq_client < 0) {
    snd_seq_close (seq_handle);
    fprintf (stderr, "Unable to determine my client number: %s\n",
	     snd_strerror (err));
    return -1;
  }

  sprintf (name, "sbiload - %i", getpid ());
  if ((err = snd_seq_set_client_name (seq_handle, name)) < 0) {
    snd_seq_close (seq_handle);
    fprintf (stderr, "Unable to set client info: %s\n",
	     snd_strerror (err));
    return -1;
  }

  if ((seq_port = snd_seq_create_simple_port (seq_handle, "Output",
					      SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_WRITE,
					      SND_SEQ_PORT_TYPE_SPECIFIC)) < 0) {
    snd_seq_close (seq_handle);
    fprintf (stderr, "Unable to create a client port: %s\n",
	     snd_strerror (seq_port));
    return -1;
  }

  snd_seq_port_subscribe_alloca(&sub);
  addr.client = seq_client;
  addr.port = seq_port;
  snd_seq_port_subscribe_set_sender(sub, &addr);
  addr.client = seq_dest_client;
  addr.port = seq_dest_port;
  snd_seq_port_subscribe_set_dest(sub, &addr);
  snd_seq_port_subscribe_set_exclusive(sub, 1);

  if ((err = snd_seq_subscribe_port (seq_handle, sub)) < 0) {
    snd_seq_close (seq_handle);
    fprintf (stderr, "Unable to subscribe destination port: %s\n",
	     snd_strerror (errno));
    return -1;
  }

  return 0;
}

/*
 * Unsubscribe client from destination port
 * and close sequencer
 */
static void
finish_client ()
{
  snd_seq_port_subscribe_t *sub;
  snd_seq_addr_t addr;
  int err;

  snd_seq_port_subscribe_alloca(&sub);
  addr.client = seq_client;
  addr.port = seq_port;
  snd_seq_port_subscribe_set_sender(sub, &addr);
  addr.client = seq_dest_client;
  addr.port = seq_dest_port;
  snd_seq_port_subscribe_set_dest(sub, &addr);
  if ((err = snd_seq_unsubscribe_port (seq_handle, sub)) < 0) {
    fprintf (stderr, "Unable to unsubscribe destination port: %s\n",
	     snd_strerror (errno));
  }
  snd_seq_close (seq_handle);
}

/*
 * Load a .SBI FM instrument patch
 *   sbiload [-p client:port] [-l] [-v level] instfile drumfile
 *
 *   -p, --port=client:port  - An ALSA client and port number to use
 *   -4  --opl3              - four operators file type
 *   -l, --list              - List possible output ports that could be used
 *   -v, --verbose=level     - Verbose level
 */
int
main (int argc, char **argv) {
  char opts[NELEM (long_opts) * 2 + 1];
  char *portdesc;
  char *cp;
  int c;
  struct option *op;

  /* Build up the short option string */
  cp = opts;
  for (op = long_opts; op < &long_opts[NELEM (long_opts)]; op++) {
    *cp++ = op->val;
    if (op->has_arg)
      *cp++ = ':';
  }

  portdesc = NULL;

  /* Deal with the options */
  for (;;) {
    c = getopt_long (argc, argv, opts, long_opts, NULL);
    if (c == -1)
      break;

    switch (c) {
    case 'p':
      portdesc = optarg;
      break;
    case '4':
      file_type = SBI_FILE_TYPE_4OP;
      break;
    case 'v':
      verbose = atoi (optarg);
      break;
    case 'V':
      show_version();
      exit (1);
    case 'l':
      show_list ();
      exit (0);
    case '?':
      show_usage ();
      exit (1);
    }
  }

  if (portdesc == NULL) {
    portdesc = getenv ("ALSA_OUT_PORT");
    if (portdesc == NULL) {
      fprintf (stderr, "No client/port specified.\n"
	       "You must supply one with the -p option or with the\n"
	       "environment variable ALSA_OUT_PORT\n");
      exit (1);
    }
  }

  /* Parse port description to dest_client and dest_port */
  if (parse_portdesc (portdesc) < 0) {
    return 1;
  }

  /* Initialize client and subscribe to destination port */
  if (init_client () < 0) {
    return 1;
  }

  /* Process instrument and drum file */
  if (optind < argc) {
    if (load_file (0, argv[optind++]) < 0) {
      finish_client();
      return 1;
    }
  } else {
    fprintf(stderr, "Warning: instrument file was not specified\n");
  }
  if (optind < argc) {
    if (load_file (128, argv[optind]) < 0) {
      finish_client();
      return 1;
    }
  } else {
    fprintf(stderr, "Warning: drum file was not specified\n");
  }

  /* Unsubscribe destination port and close client */
  finish_client();

  return 0;
}
