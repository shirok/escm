# src/Makefile.am
# $Id$
# Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>

# Program
BUILT_SOURCES = scm.c
CLEANFILES = scm.c
DEFAULT_LANG = scm

bin_PROGRAMS = aescm
aescm_SOURCES = escm.c escm.h cgi.c fork.c $(BUILT_SOURCES)
aescm_SOURCES += filter.c meta_arg.c meta_arg.h 
aescm_LDADD = @GIMMICK_OBJ@
aescm_DEPENDENCIES = @GIMMICK_OBJ@
EXTRA_aescm_SOURCES = lang.c
if GIMMICK
AM_CFLAGS = -DESCM_LANG_DIR=\"$(pkgdatadir)/lang/\"
endif
EXTRA_DIST = lang.awk lang/

scm.c : lang/$(DEFAULT_LANG) lang.awk
	$(AWK) -f lang.awk $< > $@

install-data-local :
if GIMMICK
	-$(INSTALL) -d $(pkgdatadir)/lang
	$(INSTALL_DATA) lang/[a-z]* $(pkgdatadir)/lang
endif

uninstall-local :
if GIMMICK
	-rm -r $(pkgdatadir)/lang
endif
# end of src/Makefile.am
