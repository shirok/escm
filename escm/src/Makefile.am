# src/Makefile.am
# $Id$
# Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>

# Program
bin_PROGRAMS = aescm
aescm_SOURCES = filter.c meta_arg.c meta_arg.h escm.c escm.h cgi.c
aescm_LDADD = @GIMMICK_OBJ@
aescm_DEPENDENCIES = @GIMMICK_OBJ@
EXTRA_aescm_SOURCES = lang.c
if GIMMICK
AM_CFLAGS = -DESCM_LANG_DIR=\"$(datadir)/lang\"
endif
EXTRA_DIST = lang/

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
