TOP = .
include $(TOP)/make.conf

CFLAGS = $(COPTS) $(INCLUDES)
LIBS = $(ASOUND)

OBJS = cspctl.o


# Build all the targets.
#
all: $(OBJS)
	$(CC) -o cspctl $(CFLAGS) $(OBJS) $(LIBS)
	
install: all
	$(INSTALL) cspctl $(INSTDIR)

# Remove all object files and exectutables
#
clean:
	rm -f cspctl $(OBJS) core a.out

# Remove all generated and built files.
#
clobber: clean

cspctl.o : cspctl.c
