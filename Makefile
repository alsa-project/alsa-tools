VERSION = 0.9.0beta10
TOP = .
SUBDIRS = ac3dec as10k1 envy24control sb16_csp seq

all:
	@for i in $(SUBDIRS); do cd $(TOP)/$$i; ./cvscompile; cd ..; make -C $$i; done

alsa-dist:
	@echo $(VERSION) >> $(TOP)/version
	@mkdir -p $(TOP)/distdir
	@for i in $(SUBDIRS); do cd $(TOP)/$$i; ./cvscompile; cd ..; make -C $$i alsa-dist; done
	@mv distdir alsa-tools-$(VERSION)
	@tar cIf alsa-tools-$(VERSION).tar.bz2 alsa-tools-$(VERSION)
	@mv alsa-tools-$(VERSION) distdir

clean:
	rm -rf *~ distdir
	@for i in $(SUBDIRS); do make -C $$i clean; done
