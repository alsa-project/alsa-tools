AM_CFLAGS = @GTK_CFLAGS@
bin_PROGRAMS = envy24control
man_MANS = envy24control.1
envy24control_SOURCES = envy24control.c envy24control.h levelmeters.c \
                        mixer.c patchbay.c hardware.c driverevents.c volume.c
envy24control_LDFLAGS = @GTK_LIBS@
EXTRA_DIST = cvscompile envy24control.1 depcomp

alsa-dist: distdir
	@rm -rf ../distdir/envy24control
	@mkdir -p ../distdir/envy24control
	@cp -RLpv $(distdir)/* ../distdir/envy24control
	@rm -rf $(distdir)
