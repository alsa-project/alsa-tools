/*
 * Firmware Loader for Digigram VX soundcards
 *
 * Copyright (c) 2003 by Takashi Iwai <tiwai@suse.de>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdarg.h>
#include <alsa/asoundlib.h>

#include <sound/vx_load.h>

#define PROGNAME	"vxload"


/* directory containing the firmware binaries */
#ifndef DATAPATH
#define DATAPATH	"/usr/share/vxloader"
#endif

/* firmware index file */
#define INDEX_FILE	DATAPATH "/index"

/* max. number of cards (shouldn't be in the public header?) */
#define SND_CARDS	8


static int verbose;

static void usage(void)
{
	printf("Boot loader for Digigram VX cards\n");
	printf("version %s\n", VERSION);
	printf("usage: vxloader [-c card] [-D device]\n");
}

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


/*
 * read a xilinx bitstream file
 */
static int read_xilinx_image(struct snd_vx_image *img, const char *fname)
{
	FILE *fp;
	char buf[256];
	int data, c, idx, length;
	char *p;

	if ((fp = fopen(fname, "r")) == NULL) {
		error("cannot open %s\n", fname);
		return -ENODEV;
	}

	strcpy(img->name, fname);

	c = 0;
	data = 0;
	idx = 0;
	length = 0;
	while (fgets(buf, sizeof(buf), fp)) {
		if (strncmp(buf, "Bits:", 5) == 0) {
			for (p = buf + 5; *p && isspace(*p); p++)
				;
			if (! *p) {
				error("corrupted file %s in Bits line\n", fname);
				fclose(fp);
				return -EINVAL;
			}
			length = atoi(p);
			length /= 8;
			if (length <= 0) {
				error("corrupted file %s, detected length = %d\n", fname, length);
				fclose(fp);
				return -EINVAL;
			}
			img->length = length;
			img->image = malloc(length);
			if (! img->image) {
				error("cannot alloc %d bytes\n", length);
				fclose(fp);
				return -ENOMEM;
			}
			continue;
		}
		if (buf[0] != '0' && buf[1] != '1')
			continue;
		if (length <= 0) {
			error("corrupted file %s, starting without Bits line\n", fname);
			fclose(fp);
			return -EINVAL;
		}
		for (p = buf; *p == '0' || *p == '1'; p++) {
			data |= (*p - '0') << c;
			c++;
			if (c >= 8) {
				img->image[idx] = data;
				data = 0;
				c = 0;
				idx++;
				if (idx >= length)
					break;
			}
		}
	}
	if (c)
		img->image[idx++] = data;
	if (idx != length) {
		error("length doesn't match: %d != %d\n", idx, length);
		fclose(fp);
		return -EINVAL;
	}

	fclose(fp);
	return 0;
}

/*
 * read a binary image file
 */
static int read_boot_image(struct snd_vx_image *img, const char *fname)
{
	struct stat st;
	int fd;

	strcpy(img->name, fname);
	if (stat(fname, &st) < 0) {
		error("cannot call stat %s\n", fname);
		return -ENODEV;
	}
	img->length = st.st_size;
	if (img->length == 0) {
		error("zero file size %s\n", fname);
		return -EINVAL;
	}

	img->image = malloc(img->length);
	if (! img->image) {
		error("cannot malloc %d bytes\n", img->length);
		return -ENOMEM;
	}

	fd = open(fname, O_RDONLY);
	if (fd < 0) {
		error("cannot open %s\n", fname);
		return -ENODEV;
	}
	if (read(fd, img->image, img->length) != (ssize_t)img->length) {
		error("cannot read %d bytes from %s\n", img->length, fname);
		close(fd);
		return -EINVAL;
	}

	close(fd);
	return 0;
}


/*
 * parse the index file and get the file to read from the key
 */

#define MAX_PATH	128

static int get_file_name(const char *key, const char *type, char *fname)
{
	FILE *fp;
	char buf[128];
	char temp[32], *p;
	int len;

	if ((fp = fopen(INDEX_FILE, "r")) == NULL) {
		error("cannot open the index file %s\n", INDEX_FILE);
		return -ENODEV;
	}

	sprintf(temp, "%s.%s", key, type);
	len = strlen(temp);

	while (fgets(buf, sizeof(buf), fp)) {
		if (strncmp(buf, temp, len))
			continue;
		for (p = buf + len; *p && isspace(*p); p++)
			;
		if (*p == '/') {
			strncpy(fname, p, MAX_PATH);
		} else {
			snprintf(fname, MAX_PATH, "%s/%s", DATAPATH, p);
		}
		fname[MAX_PATH-1] = 0;
		/* chop the last linefeed */
		for (p = fname; *p && *p != '\n'; p++)
			;
		*p = 0;
		fclose(fp);
		return 0;
	}
	fclose(fp);
	error("cannot find the file entry for %s\n", temp);
	return -ENODEV;
}


/*
 * read the firmware binaries
 */
static int read_firmware(int type, const char *key, struct snd_vx_loader *xilinx, struct snd_vx_loader *dsp)
{
	int err;
	char fname[MAX_PATH];

	if (type >= VX_TYPE_VXPOCKET) {
		if (get_file_name(key, "boot", fname) < 0)
			return -EINVAL;
		err = read_boot_image(&xilinx->boot, fname);
		if (err < 0)
			return err;
	}

	if (get_file_name(key, "xilinx", fname) < 0)
		return -EINVAL;
	err = read_xilinx_image(&xilinx->binary, fname);
	if (err < 0)
		return err;

	if (get_file_name(key, "dspboot", fname) < 0)
		return -EINVAL;
	err = read_boot_image(&dsp->boot, fname);
	if (err < 0)
		return err;

	if (get_file_name(key, "dspimage", fname) < 0)
		return -EINVAL;
	err = read_boot_image(&dsp->binary, fname);
	return err;
}


/*
 * load the firmware binaries
 */
static int vx_boot(const char *devname)
{
	snd_hwdep_t *hw;
	const char *key;
	int err;
	struct snd_vx_version version;
	struct snd_vx_loader xilinx, dsp;

	if ((err = snd_hwdep_open(&hw, devname, O_RDWR)) < 0) {
		error("cannot open hwdep %s\n", devname);
		return err;
	}

	/* get the current status */
	if ((err = snd_hwdep_ioctl(hw, SND_VX_HWDEP_IOCTL_VERSION, &version)) < 0) {
		error("cannot get version for %s\n", devname);
		return err;
	}

	switch (version.type) {
	case VX_TYPE_BOARD:
		key = "board";
		break;
	case VX_TYPE_V2:
	case VX_TYPE_MIC:
		key = "vx222";
		break;
	case VX_TYPE_VXPOCKET:
		key = "vxpocket";
		break;
	case VX_TYPE_VXP440:
		key = "vxp440";
		break;
	default:
		error("invalid VX board type %d\n", version.type);
		return -EINVAL;
	}

	memset(&xilinx, 0, sizeof(xilinx));
	memset(&dsp, 0, sizeof(dsp));
	
	if ((err = read_firmware(version.type, key, &xilinx, &dsp)) < 0)
		return err;

	//fprintf(stderr, "loading xilinx..\n");
	if (! (version.status & VX_STAT_XILINX_LOADED) &&
	    (err = snd_hwdep_ioctl(hw, SND_VX_HWDEP_IOCTL_LOAD_XILINX, &xilinx)) < 0) {
		error("cannot load xilinx\n");
		return err;
	}
	//fprintf(stderr, "loading dsp..\n");
	if (! (version.status & VX_STAT_DSP_LOADED) &&
	    (err = snd_hwdep_ioctl(hw, SND_VX_HWDEP_IOCTL_LOAD_DSP, &dsp)) < 0) {
		error("cannot load DSP\n");
		return err;
	}
	//fprintf(stderr, "starting devices..\n");
	if (! (version.status & VX_STAT_DEVICE_INIT) &&
	    (err = snd_hwdep_ioctl(hw, SND_VX_HWDEP_IOCTL_INIT_DEVICE, 0)) < 0) {
		error("cannot initialize devices\n");
		return err;
	}
	snd_hwdep_close(hw);
	return 0;
}


int main(int argc, char **argv)
{
	int c;
	int card = -1;
	char *device_name = NULL;
	char name[64];

	while ((c = getopt(argc, argv, "c:D:")) != -1) {
		switch (c) {
		case 'c':
			card = atoi(optarg);
			break;
		case 'D':
			device_name = optarg;
			break;
		default:
			usage();
			return 1;
		}
	}

	if (device_name) {
		verbose = 1;
		return vx_boot(device_name) != 0;
	}
	if (card >= 0) {
		sprintf(name, "hw:%d", card);
		verbose = 1;
		return vx_boot(name) != 0;
	}

	/* probe the cards until found */
	for (c = 0; c < SND_CARDS; c++) {
		sprintf(name, "hw:%d", c);
		if (! vx_boot(name))
			return 0;
	}
	fprintf(stderr, PROGNAME ": no VX-compatible cards found\n");
	return 1;
}
