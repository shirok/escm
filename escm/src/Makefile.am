# src/Makefile.am
# $Id$
# Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>

SUBDIRS = lang

# Program
BUILT_SOURCES = deflang.c
CLEANFILES = deflang.c

deflang.c : lang.awk lang/@DEFAULT_LANG@ 
	$(AWK) -f lang.awk lang/@DEFAULT_LANG@ > $@

bin_PROGRAMS = aescm 2aescm
aescm_SOURCES  = escm.c escm.h misc.c misc.h fork.c $(BUILT_SOURCES)
aescm_SOURCES += filter.c
aescm_SOURCES += cmdline.c
aescm_SOURCES += lang.awk

2aescm_SOURCES = classic.c

# other language support
aescm_LDADD = @POLYGLOT_OBJ@
aescm_DEPENDENCIES = @POLYGLOT_OBJ@
EXTRA_aescm_SOURCES = lang.c

if POLYGLOT
pkgdata_DATA = lang.awk
endif

DEFS = -DESCM_LANG_DIR=\"$(pkgdatadir)/lang/\" -DLOCALEDIR=\"$(datadir)/locale\" -DESCM_PATH=\"$(bindir)/$(PACKAGE)\" @DEFS@

# end of src/Makefile.am
