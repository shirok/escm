# doc/Makefile.am
# $Id$
# Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>

ESCM = ../src/aescm

SUFFIXES = .escm .html .epod .pod

man_MANS = aescm.1 2aescm.1

HTML_FILES = readme.html gimmick.html interpreter.html

$(ESCM) :
	cd ../src/ && make

config-h.scm : ../src/config.h
	$(AWK) -f $(top_srcdir)/doc/config-h.awk $< > $@

%.html : %.escm config-h.scm
	-LANG=C && $(ESCM) -e "(define *date* \"`date -u`\")" -e "(define *pwd* \"`pwd`\")" -e "(load (string-append *pwd* \"/\" \"$(top_srcdir)/doc/helper.scm\"))" -o $@ $<

%.pod : %.epod config-h.scm
	$(ESCM) -e "(define *pwd* \"`pwd`\")" -e "(load (string-append *pwd* \"/\" \"$(top_srcdir)/doc/helper.scm\"))" -o $@ $<

%.1 : %.pod
	-$(POD2HTML) $< > $*.html
	-$(POD2MAN) -c "" -r "" $< > $@

EXTRA_DIST = config-h.awk helper.scm
EXTRA_DIST += readme.escm gimmick.escm interpreter.escm
EXTRA_DIST += aescm.epod 2aescm.epod

CLEANFILES = config-h.scm

noinst_DATA = $(HTML_FILES)

clean-local:
	-rm *~
	-rm *.html
	-rm *.1

# end of doc/Makefile.am

