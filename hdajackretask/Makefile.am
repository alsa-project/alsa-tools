EXTRA_DIST = gitcompile README
AM_CFLAGS = @GTK3_CFLAGS@
bin_PROGRAMS = hdajackretask
man_MANS =
hdajackretask_SOURCES = main-gtk.c sysfs-pin-configs.c apply-changes.c 
hdajackretask_LDADD = @GTK3_LIBS@

BUILT_SOURCES = README.generated.h
nodist_hdajackretask_SOURCES = README.generated.h
CLEANFILES = README.generated.h

README.generated.h:
	sed 's/\\/\\\\/g;s/"/\\"/g;s/^/"/;s/$$/\\n"/;' README >$@

alsa-dist: distdir
	@rm -rf ../distdir/hdajackretask
	@mkdir -p ../distdir/hdajackretask
	@cp -RLpv $(distdir)/* ../distdir/hdajackretask
	@rm -rf $(distdir)

