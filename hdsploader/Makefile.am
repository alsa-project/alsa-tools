# # Process this file with automake to produce Makefile.in.

SUBDIRS = firmware

bin_PROGRAMS = hdsploader

hdsploader_SOURCES = hdsploader.c

EXTRA_DIST = depcomp

alsa-dist: distdir
	@rm -rf ../distdir/hdsploader
	@mkdir -p ../distdir/hdsploader
	@cp -RLpv $(distdir)/* ../distdir/hdsploader
	@rm -rf $(distdir)
