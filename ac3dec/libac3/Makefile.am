# # Process this file with automake to produce Makefile.in.
AUTOMAKE_OPTIONS = 1.3 foreign

CFLAGS = -Wall -Werror -O1 -g

noinst_LIBRARIES = libac3.a

libac3_a_SOURCES = bitstream.c decode.c imdct.c coeff.c exponent.c stats.c\
	bit_allocate.c  parse.c crc.c debug.c rematrix.c downmix.c dither.c \
	sanity_check.c

noinst_HEADERS = ac3_internal.h bitstream.h decode.h imdct.h ac3.h stats.h coeff.h\
	exponent.h bit_allocate.h parse.h crc.h debug.h rematrix.h downmix.h dither.h \
	sanity_check.h

