bin_PROGRAMS = as10k1
as10k1_SOURCES = as10k1.c parse.c assemble.c macro.c
EXTRA_DIST = cvscompile output.doc examples/Makefile examples/*.asm

dsp:
	$(MAKE) -C examples

clean:
	rm -rf .deps *~
	$(MAKE) -C examples clean
