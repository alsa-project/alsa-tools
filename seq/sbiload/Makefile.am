# # Process this file with automake to produce Makefile.in.
AUTOMAKE_OPTIONS = 1.3 foreign

CFLAGS = -Wall -g

bin_PROGRAMS = sbiload
#man_MANS = sbiload.1

sbiload_SOURCES = sbiload.c

EXTRA_DIST = README COPYING

alsa-dist: distdir
	@rm -rf ../../distdir/seq/sbiload
	@mkdir -p ../../distdir/seq/sbiload
	@cp -RLpv $(distdir)/* ../../distdir/seq/sbiload
	@rm -rf $(distdir)
