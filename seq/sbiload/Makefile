TOP = .
include $(TOP)/make.conf

CFLAGS = $(COPTS) $(INCLUDES)
LIBS = $(ASOUND)

OBJS = sbiload.o


# Build all the targets.
#
all: $(OBJS)
	$(CC) -o sbiload $(CFLAGS) $(OBJS) $(LIBS)
	
install: all
	$(INSTALL) sbiload $(INSTDIR)

# Remove all object files and exectutables
#
clean:
	rm -f sbiload $(OBJS) core a.out

# Remove all generated and built files.
#
clobber: clean

sbiload.o : sbiload.c
