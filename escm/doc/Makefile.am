# doc/Makefile.am
# $Id$

noinst_DATA = interpreters.html readme.html shebang.html todo.html
noinst_DATA += config.scm
EXTRA_DIST = interpreters.escm readme.escm shebang.escm todo.escm
EXTRA_DIST += helper.scm aescm.css aescm.epod
CLEANFILES = $(noinst_DATA) manual.html

man_MANS = aescm.1

SUFFIXES = .1 .pod .html .escm .epod

ESCM = ../src/aescm

config.scm : ../src/config.h
	awk -f ../awk/config-h.awk $< > $@

%.pod : %.epod config.scm
	$(ESCM) $< > $@

%.1 : %.pod
	pod2man -c "" $< > $@
	pod2html $< > manual.html

%.html : %.escm config.scm
	$(ESCM) $< > $@

../README : readme.html config.scm
	$(LYNX) -dump $< > $@

# end of doc/Makefile.am
