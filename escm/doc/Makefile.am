# doc/Makefile.am
# $Id$
# Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>

#
# Requires GNU Make to generate manpages and html documents.
# So type `make MAKE=gmake' in some environments.
#

ESCM = ../src/aescm
POD2MAN = pod2man
POD2HTML = pod2html
SUFFIXES = .1 .html .epod .escm .pod

man_MANS = aescm.1 2aescm.1

HTML_FILES = readme.html gimmick.html interpreter.html

$(ESCM) :
	cd ../src/ && make

config-h.scm : ../src/config.h
	$(AWK) -f config-h.awk $< > $@

%.html : %.escm config-h.scm
	LANG=C && $(ESCM) -e "(define *date* \"`date -u`\")" -e "(define *pwd* \"`pwd`\")" -e "(load (string-append *pwd* \"/\" \"$(top_srcdir)/doc/helper.scm\"))" -o $@ $<

%.1 : %.pod
	$(POD2HTML) $< > $*.html
	$(POD2MAN) -c "" -r "" $< > $@

%.pod : %.epod
	$(ESCM) -e "(define *pwd* \"`pwd`\")" -e "(load (string-append *pwd* \"/\" \"/doc/helper.scm\"))" $< > $@

EXTRA_DIST = config-h.awk helper.scm
EXTRA_DIST += readme.escm gimmick.escm interpreter.escm
EXTRA_DIST += aescm.epod 2aescm.epod

CLEANFILES = config-h.scm

noinst_DATA = $(HTML_FILES)

clean-local:
	-rm *~
	-rm *.pod

# end of doc/Makefile.am

