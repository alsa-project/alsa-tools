# # Process this file with automake to produce Makefile.in.
AUTOMAKE_OPTIONS = 1.3 foreign

CFLAGS = -Wall -Werror -O3 -g

bin_PROGRAMS = ac3dec 

ac3dec_LDADD= -L./libac3 -lac3

noinst_HEADERS = output.h
ac3dec_SOURCES = ac3dec.c output.c ac3spdif.c
ac3dec_DEPENDENCIES = libac3/libac3.a


EXTRA_DIST = README Changelog TODO plot_spectrum.m autogen.sh depcomp

SUBDIRS = libac3 tools test .

alsa-dist: distdir
	@rm -rf ../distdir/ac3dec
	@mkdir -p ../distdir/ac3dec
	@cp -RLpv $(distdir)/* ../distdir/ac3dec
	@rm -rf $(distdir)
