INCLUDES = -I../libac3
noinst_PROGRAMS = imdct_test dither_test
#noinst_PROGRAMS = bitstream_test imdct_test imdct_timing uncouple_timing\
	#crc_timing
noinst_HEADERS = timing.h

#bitstream_test_SOURCES = bitstream_test.c
#bitstream_test_LDADD = ../bitstream.o ../crc.o
#
dither_test_SOURCES = dither_test.c
dither_test_LDADD = ../libac3/dither.o 

imdct_test_SOURCES = imdct_test.c
imdct_test_LDADD = ../libac3/imdct.o -lm
#
#imdct_timing_SOURCES = imdct_timing.c timing.c
#imdct_timing_LDADD = ../imdct.o -lm
#
#uncouple_timing_SOURCES = uncouple_timing.c timing.c
#uncouple_timing_LDADD = ../uncouple.o ../dither.o -lm
#
#crc_timing_SOURCES = crc_timing.c timing.c
#crc_timing_LDADD = ../crc.o -lm

check:
	#rm -f bitstream_test.out
	#./bitstream_test > bitstream_test.out
	#diff bitstream_test.out bitstream_test.out.ref
	#rm -f imdct_test.out
	#./imdct_test > imdct_test.out
	#diff imdct_test.out imdct_test.out.ref
	rm -f dither_test.out
	./dither_test > dither_test.out
	diff dither_test.out dither_test.out.ref

clean:
	rm -f *.out
