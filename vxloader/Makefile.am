# # Process this file with automake to produce Makefile.in.
AUTOMAKE_OPTIONS = 1.3 foreign

CFLAGS = @CFLAGS@ -DDATAPATH=\"$(datadir)/vxloader\"

SUBDIRS = firmware

bin_PROGRAMS = vxloader

vxloader_SOURCES = vxloader.c

EXTRA_DIST = depcomp

alsa-dist: distdir
	@rm -rf ../distdir/vxloader
	@mkdir -p ../distdir/vxloader
	@cp -RLpv $(distdir)/* ../distdir/vxloader
	@rm -rf $(distdir)
