# src/lang/Makefile.am
# $Id$
# Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>

# Do not forget to add a new config file.
LANG_FILES = awk csh lisp lisp-cmucl perl python ruby scm-scm sh tcl
# FIFO_FILES = scm-fifo

langdir = $(pkgdatadir)/lang
lang_DATA = $(LANG_FILES) scm

EXTRA_DIST = $(LANG_FILES) $(FIFO_FILES) scm.in

uninstall-local :
	-rmdir $(pkgdatadir)/lang

# end of src/lang/Makefile.am
