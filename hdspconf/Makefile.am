EXTRA_DIST = gitcompile depcomp
SUBDIRS = src pixmaps desktop

alsa-dist: distdir
	@rm -rf ../distdir/hdspconf
	@mkdir -p ../distdir/hdspconf
	@cp -RLpv $(distdir)/* ../distdir/hdspconf
	@rm -rf $(distdir)
