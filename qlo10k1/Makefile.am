SUBDIRS = src

EXTRA_DIST = COPYING configure.in README NEWS AUTHORS TODO INSTALL ChangeLog

alsa-dist: distdir
	@rm -rf ../distdir/ld10k1
	@mkdir -p ../distdir/ld10k1
	@cp -RLpv $(distdir)/* ../distdir/ld10k1
	@rm -rf $(distdir)
