# doc/Makefile.am
# $Id$
# Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>

ESCM = ../src/aescm
POD2MAN = pod2man
POD2HTML = pod2html
SUFFIXES = .html .epod .escm .pod .1

man_MANS = aescm.1 2aescm.1

HTML_FILES = gimmick.html interpreter.html

all : $(man_MANS) $(HTML_FILES)
	LANG=C && $(ESCM) -e "(define *pwd* \"`pwd`\")" -e "(define *date* \"`date -u`\")" readme.escm > readme.html
	 w3m -dump readme.html > ../README

$(ESCM) :
	cd ../src/ && make

.epod.1 :
	$(ESCM) -e "(define *pwd* \"`pwd`\")" $< > $*.pod
	$(POD2HTML) $*.pod > $*.html
	$(POD2MAN) -c "" -r "" $*.pod > $@

.escm.html :
	$(ESCM) -e "(define *pwd* \"`pwd`\")" $< > $@

EXTRA_DIST = config-h.awk helper.scm
EXTRA_DIST += readme.escm gimmick.escm interpreter.escm
EXTRA_DIST += aescm.epod 2aescm.epod

CLEANFILES = config-h.scm

noinst_DATA = $(HTML_FILES)

clean-local:
	-rm *~
	-rm *.1
	-rm *.pod
	-rm *.html

# end of doc/Makefile.am

