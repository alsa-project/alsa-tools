SUBDIRS = src pixmaps

alsa-dist: distdir
	@rm -rf ../distdir/hdspmixer
	@mkdir -p ../distdir/hdspmixer
	@cp -RLpv $(distdir)/* ../distdir/hdspmixer
	@rm -rf $(distdir)
