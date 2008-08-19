SUBDIRS = sbiload

all:
	@for i in $(SUBDIRS); do cd $$i; echo $$1; ./gitcompile $(GITCOMPILE_ARGS) || exit 1; cd ..; $(MAKE) -C $$i; done

install:
	@for i in $(SUBDIRS); do $(MAKE) -C $$i DESTDIR=$(DESTDIR) install || exit 1; done

alsa-dist:
	@for i in $(SUBDIRS); do $(MAKE) -C $$i alsa-dist || exit 1; done

clean:
	@for i in $(SUBDIRS); do $(MAKE) -C $$i clean || exit 1; done
