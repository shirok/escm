# doc/Makefile.am
# $Id$
# Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>

all :

ESCM = aescm
POD2MAN = pod2man
HTMLDUMP = w3m -dump
SUFFIXES = .1 .html .epod .escm

man_MANS = aescm.1

.epod.1 :
	$(ESCM) -e "(define *builddir* \"$(top_builddir)\")" -e "(define *srcdir* \"$(top_srcdir)\")" $< > $*.pod
	$(POD2MAN) -c "" -r "" $*.pod > $@

htmldir = $(prefix)/share/doc/aescm
html_DATA = gimmick.html interpreter.html readme.html install.html

.escm.html :
	LANG=C && $(ESCM) -e "(define *date* \"`date -u`\")" -e "(define *builddir* \"$(top_builddir)\")" -e "(define *srcdir* \"$(top_srcdir)\")" -e "(define *language* 'en)" $< > $@

document : $(man_MANS) $(html_DATA)
	$(HTMLDUMP) readme.html > ../README
	$(HTMLDUMP) install.html > ../INSTALL

EXTRA_DIST = $(man_MANS:.1=.epod) $(html_DATA:.html=.escm)
EXTRA_DIST += $(man_MANS) $(html_DATA)

clean-local:
	-rm *~
	-rm *.tmp
	-rm *.pod

# end of doc/Makefile.am

