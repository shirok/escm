# src/Makefile.am
# $Id$
# Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>

# Library
noinst_LTLIBRARIES = libescm.la
# lib_LTLIBRARIES = libescm.la
# include_HEADERS = escm.h
libescm_la_SOURCES = escm.c cgi.c escm.h
# libescm_la_SOURCES = escm.c cgi.c $(include_HEADERS)
libescm_la_LDFLAGS = -version-info @LIB_VERSION@ -static
# libescm_la_LDFLAGS = -version-info @LIB_VERSION@

# Program
bin_PROGRAMS = aescm
aescm_SOURCES = filter.c meta_arg.c meta_arg.h
aescm_LDADD = libescm.la @GIMMICK_OBJ@
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
