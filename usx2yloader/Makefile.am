# # Process this file with automake to produce Makefile.in.
AUTOMAKE_OPTIONS = 1.3 foreign

AM_CFLAGS = -DDATAPATH=\"$(datadir)/alsa/firmware\"

SUBDIRS = firmware

bin_PROGRAMS = usx2yloader

usx2yloader_SOURCES = usx2yloader.c

EXTRA_DIST = depcomp tascam_fw.usermap tascam_fw tascam_fpga

hotplug_files = tascam_fw.usermap tascam_fw tascam_fpga
hotplugdir = /etc/hotplug/usb
hotplug_SCRIPTS = $(hotplug_files)


alsa-dist: distdir
	@rm -rf ../distdir/usx2yloader
	@mkdir -p ../distdir/usx2yloader
	@cp -RLpv $(distdir)/* ../distdir/usx2yloader
	@rm -rf $(distdir)
