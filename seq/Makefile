SUBDIRS = sbiload

all:
	@for i in $(SUBDIRS); do cd $$i; ./cvscompile $(CVSCOMPILE_ARGS); cd ..; $(MAKE) -C $$i; done	

install:
	@for i in $(SUBDIRS); do $(MAKE) -C $$i DESTDIR=$(DESTDIR) install; done	

alsa-dist:
	@for i in $(SUBDIRS); do $(MAKE) -C $$i alsa-dist; done

clean:
	@for i in $(SUBDIRS); do $(MAKE) -C $$i clean; done
