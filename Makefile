VERSION = 1.0.11rc5
TOP = .
SUBDIRS = ac3dec as10k1 envy24control hdsploader hdspconf hdspmixer \
	mixartloader pcxhrloader rmedigicontrol sb16_csp seq sscape_ctl us428control \
	usx2yloader vxloader echomixer ld10k1 qlo10k1

all:
	@for i in $(SUBDIRS); do cd $(TOP)/$$i; ./cvscompile $(CVSCOMPILE_ARGS); cd ..; make -C $$i; done

install:
	@for i in $(SUBDIRS); do make -C $$i DESTDIR=$(DESTDIR) install; done

alsa-dist:
	@echo $(VERSION) >> $(TOP)/version
	@mkdir -p $(TOP)/distdir
	@for i in $(SUBDIRS); do cd $(TOP)/$$i; ./cvscompile $(CVSCOMPILE_ARGS); cd ..; make -C $$i alsa-dist; done
	@mv distdir alsa-tools-$(VERSION)
	@tar --create --verbose --file=- alsa-tools-$(VERSION) | bzip2 -c -9 > alsa-tools-$(VERSION).tar.bz2
	@mv alsa-tools-$(VERSION) distdir

clean:
	rm -rf *~ distdir
	@for i in $(SUBDIRS); do make -C $$i clean; done
