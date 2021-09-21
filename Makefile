# Makefile to build class 'granular_synth' for Pure Data.
# Needs Makefile.pdlibbuilder as helper makefile for platform-dependent build
# settings and rules.

# library name
# add folder shortcut examples:
# INCLUDES = -I../includes
# TESTING = testing

lib.name = granular_synth

# input source file (class name == source file basename)
class.sources = pd_granular_synth~.c
pd_granular_synth~.class.sources += c_granular_synth.c
pd_granular_synth~.class.sources += grain.c
pd_granular_synth~.class.sources += vas_mem.c
pd_granular_synth~.class.sources += envelope.c

# Hiermit weiteresource files hinzufuegen
# all extra files to be included in binary distribution of the library
datafiles = amen_break.wav

# include Makefile.pdlibbuilder from submodule directory 'pd-lib-builder'
PDLIBBUILDER_DIR=pd-lib-builder/

CC += $(INCLUDES)
# CC +=  -mavx -DVAS_USE_AVX

include $(PDLIBBUILDER_DIR)/Makefile.pdlibbuilder




