/*
 * Controller for Tascam US-X2Y
 *
 * Copyright (c) 2003 by Karsten Wiese <annabellesgarden@yahoo.de>
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
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <poll.h>
#include <alsa/asoundlib.h>
#include "Cus428_ctls.h"
#include "Cus428State.h"


#define PROGNAME		"us428control"
#define SND_USX2Y_LOADER_ID	"USX2Y Loader"

/* max. number of cards (shouldn't be in the public header?) */
#define SND_CARDS	8

int verbose = 1;


static void error(const char *fmt, ...)
{
	if (verbose) {
		va_list ap;
		va_start(ap, fmt);
		fprintf(stderr, "%s: ", PROGNAME);
		vfprintf(stderr, fmt, ap);
		va_end(ap);
	}
}


static void usage(void)
{
	printf("Tascam US-428 Contol\n");
	printf("version %s\n", VERSION);
	printf("usage: "PROGNAME" [-v verbosity_level 0..2] [-c card] [-D device] [-u usb-device]\n");
}
/*
 * check the name id of the given hwdep handle
 */
static int check_hwinfo(snd_hwdep_t *hw, const char *id, const char* usb_dev_name)
{
	snd_hwdep_info_t *info;
	int err;

	snd_hwdep_info_alloca(&info);
	if ((err = snd_hwdep_info(hw, info)) < 0)
		return err;
	if (strcmp(snd_hwdep_info_get_id(info), id))
		return -ENODEV;
	if (usb_dev_name) 
		if (strcmp(snd_hwdep_info_get_name(info), usb_dev_name))
			return -ENODEV;

	return 0; /* ok */
}

int US428Control(const char* DevName)
{
	snd_hwdep_t		*hw;
	int			err;
	unsigned int		idx, dsps, loaded;
	us428ctls_sharedmem_t	*us428ctls_sharedmem;
	struct pollfd		pfds;

	if ((err = snd_hwdep_open(&hw, DevName, O_RDWR)) < 0) {
		error("cannot open hwdep %s\n", DevName);
		return err;
	}

	if (check_hwinfo(hw, SND_USX2Y_LOADER_ID, NULL) < 0) {
		error("invalid hwdep %s\n", DevName);
		snd_hwdep_close(hw);
		return -ENODEV;
	}
	snd_hwdep_poll_descriptors(hw, &pfds, 1);
	us428ctls_sharedmem = (us428ctls_sharedmem_t*)mmap(NULL, sizeof(us428ctls_sharedmem_t), PROT_READ|PROT_WRITE, MAP_SHARED, pfds.fd, 0);
	if (us428ctls_sharedmem == MAP_FAILED) {
		perror("mmap failed:");
		return -ENOMEM;
	}
	us428ctls_sharedmem->CtlSnapShotRed = us428ctls_sharedmem->CtlSnapShotLast;
	OneState = new Cus428State(us428ctls_sharedmem);
	while (1) {
		int x = poll(&pfds,1,-1);
		if (verbose > 1 || pfds.revents & (POLLERR|POLLHUP))
			printf("poll returned 0x%X\n", pfds.revents);
		if (pfds.revents & (POLLERR|POLLHUP))
			return -ENXIO;
		int Last = us428ctls_sharedmem->CtlSnapShotLast;
		if (verbose > 1)
			printf("Last is %i\n", Last);
		while (us428ctls_sharedmem->CtlSnapShotRed != Last) {
			int Read = us428ctls_sharedmem->CtlSnapShotRed + 1;
			if (Read >= N_us428_ctl_BUFS)
				Read = 0;
			Cus428_ctls* PCtlSnapShot = ((Cus428_ctls*)(us428ctls_sharedmem->CtlSnapShot)) + Read;
			int DiffAt = us428ctls_sharedmem->CtlSnapShotDiffersAt[Read];
			if (verbose > 1)
				PCtlSnapShot->dump(DiffAt);
			PCtlSnapShot->analyse(((Cus428_ctls*)(us428ctls_sharedmem->CtlSnapShot))[us428ctls_sharedmem->CtlSnapShotRed], DiffAt );
			us428ctls_sharedmem->CtlSnapShotRed = Read;
		}
	}
}

int main (int argc, char *argv[])
{
	int c;
	int card = -1;
	char	*device_name = NULL,
		*usb_device_name = getenv("DEVICE");
	char name[64];

	while ((c = getopt(argc, argv, "c:D:u:v:")) != -1) {
		switch (c) {
		case 'c':
			card = atoi(optarg);
			break;
		case 'D':
			device_name = optarg;
			break;
		case 'u':
			usb_device_name = optarg;
			break;
		case 'v':
			verbose = atoi(optarg);
			break;
		default:
			usage();
			return 1;
		}
	}

	if (usb_device_name) {
		snd_hwdep_t *hw = NULL;
		for (c = 0; c < SND_CARDS; c++) {
			sprintf(name, "hw:%d", c);
			if ((0 <= snd_hwdep_open(&hw, name, O_RDWR))
			    && (0 <= check_hwinfo(hw, SND_USX2Y_LOADER_ID, usb_device_name))
			    && (0 <= snd_hwdep_close(hw))){
				card = c;
				break;
			}
		}
	}
	if (device_name) {
		return US428Control(device_name) != 0;
	}
	if (card >= 0) {
		sprintf(name, "hw:%d", card);
		return US428Control(name) != 0;
	}

	/* probe the all cards */
	for (c = 0; c < SND_CARDS; c++) {
		verbose--;
		sprintf(name, "hw:%d", c);
		if (! US428Control(name))
			card = c;
	}
	if (card < 0) {
		fprintf(stderr, PROGNAME ": no US-X2Y-compatible cards found\n");
		return 1;
	}
	return 0;
}

