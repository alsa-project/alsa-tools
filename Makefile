VERSION = 0.9.6
TOP = .
SUBDIRS = ac3dec as10k1 envy24control hdsploader hdspconf \
	mixartloader rmedigicontrol sb16_csp seq sscape_ctl vxloader

all:
	@for i in $(SUBDIRS); do cd $(TOP)/$$i; ./cvscompile; cd ..; make -C $$i; done

alsa-dist:
	@echo $(VERSION) >> $(TOP)/version
	@mkdir -p $(TOP)/distdir
	@for i in $(SUBDIRS); do cd $(TOP)/$$i; ./cvscompile; cd ..; make -C $$i alsa-dist; done
	@mv distdir alsa-tools-$(VERSION)
	@tar cjf alsa-tools-$(VERSION).tar.bz2 alsa-tools-$(VERSION)
	@mv alsa-tools-$(VERSION) distdir

clean:
	rm -rf *~ distdir
	@for i in $(SUBDIRS); do make -C $$i clean; done
