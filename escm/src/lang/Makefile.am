# src/lang/Makefile.am
# $Id$
# Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>

# Do not forget to add a new config file.
LANG_FILES = awk csh ocaml octave perl python ruby scm sh tcl

if POLYGLOT
langdir = $(pkgdatadir)/lang
lang_DATA = $(LANG_FILES)
endif
EXTRA_DIST = $(LANG_FILES)

uninstall-local :
	-rmdir $(pkgdatadir)/lang

# end of src/lang/Makefile.am
