# src/lang/Makefile.am
# $Id$
# Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>

if POLYGLOT
langdir = $(pkgdatadir)/lang
lang_DATA = @LANG_FILES@
endif
EXTRA_DIST = @LANG_FILES@

uninstall-local :
	-rmdir $(pkgdatadir)/lang

# end of src/lang/Makefile.am
