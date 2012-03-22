SOURCES = main-gtk.c sysfs-pin-configs.c apply-changes.c

build:
	$(shell sed 's/\\/\\\\/g;s/"/\\"/g;s/^/"/;s/$$/\\n"/;' < README > README.generated.h)
	gcc -g -O0 -Wall -o hda-jack-retask $(SOURCES) $(shell pkg-config --cflags --libs gtk+-3.0) 

clean:
	-rm hda-jack-retask
	-rm README.generated.h

install:
	install -Dm755 hda-jack-retask $(DESTDIR)/usr/bin/hda-jack-retask
	install -Dm644 README $(DESTDIR)/usr/share/doc/hda-jack-retask/README

uninstall:
	-rm $(DESTDIR)/usr/bin/hda-jack-retask

.PHONY: build install uninstall
