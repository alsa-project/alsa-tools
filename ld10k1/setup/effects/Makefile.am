EXTRA_DIST = README emu_constants.asm \
	simple.asm vol_master.asm tone.asm \
	vol_2.asm output.asm switch_2.asm \
	sto51.asm switch_6.asm copy_2.asm \
	prologic.asm fxbus.asm


dist_effects_DATA = simple.emu10k1 vol_master.emu10k1 tone.emu10k1 \
	vol_2.emu10k1 output.emu10k1 switch_2.emu10k1 \
	sto51.emu10k1 switch_6.emu10k1 copy_2.emu10k1 \
	prologic.emu10k1 fxbus.emu10k1

CLEANFILES = $(dist_effects_DATA)

SUFFIXEC = .asm .emu10k1

.asm.emu10k1:
	as10k1 -o $@ $<
