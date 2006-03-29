AM_CFLAGS = @ECHOMIXER_CFLAGS@
bin_PROGRAMS = echomixer
man_MANS = 
echomixer_SOURCES = echomixer.c 
echomixer_LDFLAGS = @ECHOMIXER_LIBS@
EXTRA_DIST = configure.in-gtk1 configure.in-gtk2
AUTOMAKE_OPTIONS = foreign

alsa-dist: distdir
	@rm -rf ../distdir/echomixer
	@mkdir -p ../distdir/echomixer
	@cp -RLpv $(distdir)/* ../distdir/echomixer
	@rm -rf $(distdir)
