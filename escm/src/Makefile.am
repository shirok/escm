# src/Makefile.am
# $Id$
# Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>

# Library
lib_LTLIBRARIES = libescm.la
include_HEADERS = escm.h
libescm_la_SOURCES = escm.c cgi.c $(include_HEADERS)
libescm_la_LDFLAGS = -version-info @LIB_VERSION@

# Programs
EXTRA_PROGRAMS = aescm aescm-handler

# filter tool
bin_PROGRAMS = @ESCM@
aescm_SOURCES = filter.c meta_arg.c meta_arg.h
aescm_LDADD = libescm.la @GIMMICK_OBJ@
aescm_DEPENDENCIES = @GIMMICK_OBJ@
EXTRA_aescm_SOURCES = lang.c
if GIMMICK
AM_CFLAGS = -DESCM_LANG_DIR=\"$(datadir)/lang\"
endif
EXTRA_DIST = lang/

# action handler
cgidir = @CGIDIR@
cgi_PROGRAMS = @ESCM_HANDLER@
aescm_handler_SOURCES = handler.c
aescm_handler_LDADD = libescm.la

install-data-local :
if GIMMICK
	-$(INSTALL) -d $(pkgdatadir)/lang
	$(INSTALL_DATA) lang/* $(pkgdatadir)/lang
endif

# end of src/Makefile.am
