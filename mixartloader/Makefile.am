# # Process this file with automake to produce Makefile.in.
AUTOMAKE_OPTIONS = 1.3 foreign

AM_CFLAGS = -DDATAPATH=\"$(datadir)/alsa/firmware\"

SUBDIRS = firmware

bin_PROGRAMS = mixartloader

mixartloader_SOURCES = mixartloader.c

EXTRA_DIST = depcomp

alsa-dist: distdir
	@rm -rf ../distdir/mixartloader
	@mkdir -p ../distdir/mixartloader
	@cp -RLpv $(distdir)/* ../distdir/mixartloader
	@rm -rf $(distdir)
