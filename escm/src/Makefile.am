# src/Makefile.am
# $Id$
# Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>

SUBDIRS = lang

# Program
BUILT_SOURCES = deflang.c
CLEANFILES = deflang.c langconf

noinst_PROGRAMS = langconf
langconf_SOURCES = langconf.c escm.c escm.h cmdline.c cmdline.h lang.c misc.c misc.h

deflang.c : langconf lang/@DEFAULT_LANG@ 
	./langconf ./lang/@DEFAULT_LANG@ > $@

bin_PROGRAMS = aescm 2aescm
aescm_SOURCES  = escm.c escm.h misc.c misc.h $(BUILT_SOURCES)
aescm_SOURCES += filter.c
aescm_SOURCES += fork.c fork.h
aescm_SOURCES += cmdline.c cmdline.h
aescm_SOURCES += lang.awk

2aescm_SOURCES = classic.c

# other language support
aescm_LDADD = @POLYGLOT_OBJ@
aescm_DEPENDENCIES = @POLYGLOT_OBJ@
EXTRA_aescm_SOURCES = lang.c

DEFS = -DESCM_LANG_DIR=\"$(pkgdatadir)/lang/\" -DESCM_PATH=\"$(bindir)/$(PACKAGE)\" -DXERROR=cgi_error @DEFS@

# end of src/Makefile.am
