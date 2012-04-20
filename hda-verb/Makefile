VERSION = 0.4

CC = gcc
CFLAGS = -Wall -O2 -g

hda-verb: hda-verb.o
	$(CC) -o $@ hda-verb.o

clean:
	rm -f hda-verb *.o

dist:
	cd ..; mv hda-verb hda-verb-$(VERSION); \
	tar cfz hda-verb-$(VERSION).tar.gz --exclude='.git*' hda-verb-$(VERSION); \
	mv hda-verb-$(VERSION) hda-verb
