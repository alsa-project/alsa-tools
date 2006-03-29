AM_CFLAGS = @ENVY24CONTROL_CFLAGS@
bin_PROGRAMS = envy24control
man_MANS = envy24control.1
envy24control_SOURCES = envy24control.c envy24control.h levelmeters.c midi.c \
                        mixer.c patchbay.c hardware.c driverevents.c volume.c \
			profiles.c profiles.h midi.h config.c config.h
envy24control_LDFLAGS = @ENVY24CONTROL_LIBS@
EXTRA_DIST = envy24control.1 depcomp configure.in-gtk1 configure.in-gtk2 \
	     strstr_icase_blank.c new_process.c README.profiles
AUTOMAKE_OPTIONS = foreign

alsa-dist: distdir
	@rm -rf ../distdir/envy24control
	@mkdir -p ../distdir/envy24control
	@cp -RLpv $(distdir)/* ../distdir/envy24control
	@rm -rf $(distdir)
