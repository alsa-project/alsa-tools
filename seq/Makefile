SUBDIRS = sbiload

all:
	@for i in $(SUBDIRS); do cd $$i; ./cvscompile; cd ..; $(MAKE) -C $$i; done	

alsa-dist:
	@for i in $(SUBDIRS); do $(MAKE) -C $$i alsa-dist; done

clean:
	@for i in $(SUBDIRS); do $(MAKE) -C $$i clean; done
